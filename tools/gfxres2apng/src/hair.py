#!/usr/bin/env python3
"""gfxres2apng hair resource converter: one APNG per motion sequence.

Usage:
    python3 tools/gfxres2apng/src/hair.py INPUT_DIR OUTPUT_DIR --jobs 8

Input filename (old per-frame format), 18 chars + ".PNG":

    KKKKKKKK S S DDDD DDDD
      KKKKKKKK: 8-hex key, see hairwil2png and Hero::gfxHairID() in client/src/hero.cpp
                frame(0-4), direction(5-7), motion(8-13), hair(14-21), gender(22)
      S S     : sign of dx / dy, '0' means negative
      DDDD    : abs(dx) / abs(dy) in hex

Output filename (one APNG per motion), 14 hex digits + ".PNG", 56-bit value:

      x: bits  0-11, x offset, 12-bit two's complement
      y: bits 12-23, y offset, 12-bit two's complement
      d: bits 24-26, direction
      m: bits 27-32, motion
         bits 33-34 are zero, same layout as hero.py, hair has no layer or shadow
      g: bit  35   , gender
      i: bits 36-43, hair ID
         bits 44-55 are zero

The frame index is no longer in the filename, it's the APNG frame order.
All frames are aligned on one canvas, the transparent margin shared by all
frames is trimmed, so one x/y offset applies to every frame.
The fixed corner is (-dx, -dy) relative to the APNG's top-left pixel.

Placeholder frames, fully transparent or with an offset out of the 12-bit
range (e.g. hair DIE frame 5 where the body hides the head, 4x2 at +9703/+9775),
stay in the APNG as blank frames, their size and offset are ignored when
computing the canvas.
"""

import argparse
import io
import os
import struct
import subprocess
import sys
import zlib
from collections import defaultdict
from multiprocessing import Pool

from PIL import Image

OFFSET_BITS = 12
OFFSET_MIN = -(1 << (OFFSET_BITS - 1))
OFFSET_MAX = (1 << (OFFSET_BITS - 1)) - 1
PNG_SIG = b"\x89PNG\r\n\x1a\n"


def parse_old_name(fileName):
    stem, ext = os.path.splitext(fileName)
    if ext.upper() != ".PNG" or len(stem) != 18:
        raise ValueError(f"invalid frame filename: {fileName}")

    key = int(stem[0:8], 16)
    if not (0 <= key < (1 << 23)) or stem[8] not in "01" or stem[9] not in "01":
        raise ValueError(f"invalid hair frame filename: {fileName}")
    dx = int(stem[10:14], 16) * (1 if stem[8] != "0" else -1)
    dy = int(stem[14:18], 16) * (1 if stem[9] != "0" else -1)
    return key, dx, dy


def to_twos(value, bits):
    if not (-(1 << (bits - 1)) <= value < (1 << (bits - 1))):
        raise ValueError(f"offset {value} doesn't fit in {bits}-bit two's complement")
    return value & ((1 << bits) - 1)


def from_twos(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def encode_new_name(seqKey, dx, dy):
    # seqKey is the old key >> 5: direction(0-2), motion(3-8), hair(9-16), gender(17)
    direction = (seqKey >> 0) & 0x07
    motion = (seqKey >> 3) & 0x3F
    hair = (seqKey >> 9) & 0xFF
    gender = (seqKey >> 17) & 0x01

    value = (
        (to_twos(dx, OFFSET_BITS) << 0)
        | (to_twos(dy, OFFSET_BITS) << 12)
        | (direction << 24)
        | (motion << 27)
        | (gender << 35)
        | (hair << 36)
    )
    return f"{value:014X}.PNG"


def decode_new_name(fileName):
    value = int(os.path.splitext(fileName)[0], 16)
    return {
        "dx": from_twos((value >> 0) & 0xFFF, OFFSET_BITS),
        "dy": from_twos((value >> 12) & 0xFFF, OFFSET_BITS),
        "direction": (value >> 24) & 0x07,
        "motion": (value >> 27) & 0x3F,
        "gender": (value >> 35) & 0x01,
        "hair": (value >> 36) & 0xFF,
    }


def clear_transparent_rgb(img):
    # pixels with alpha 0 are invisible, zero their RGB for stable compression and comparison
    r, g, b, a = img.split()
    zero = Image.new("L", img.size, 0)
    mask = a.point(lambda v: 255 if v == 0 else 0)
    r.paste(zero, mask=mask)
    g.paste(zero, mask=mask)
    b.paste(zero, mask=mask)
    return Image.merge("RGBA", (r, g, b, a))


def load_rgba(path):
    with Image.open(path) as img:
        return img.convert("RGBA")


def is_placeholder(img, dx, dy):
    # fully transparent frames, or frames with a weird offset, become blank frames
    return img.getchannel("A").getbbox() is None or not (abs(dx) <= OFFSET_MAX and abs(dy) <= OFFSET_MAX)


def compose_frames(inputDir, frames):
    """frames: [(fileName, dx, dy)] in frame order. Returns (images, offX, offY)."""
    loaded = [(load_rgba(os.path.join(inputDir, f)), dx, dy) for f, dx, dy in frames]
    loaded = [(None if is_placeholder(img, dx, dy) else img, dx, dy) for img, dx, dy in loaded]

    # canvas covers visible pixels of non-placeholder frames only
    visible = [(dx + b[0], dy + b[1], dx + b[2], dy + b[3]) for img, dx, dy in loaded if img and (b := img.getchannel("A").getbbox())]
    if not visible:
        return [Image.new("RGBA", (1, 1), (0, 0, 0, 0)) for _ in loaded], 0, 0  # all frames are placeholders

    x0 = min(b[0] for b in visible)
    y0 = min(b[1] for b in visible)
    x1 = max(b[2] for b in visible)
    y1 = max(b[3] for b in visible)

    images = []
    for img, dx, dy in loaded:
        canvas = Image.new("RGBA", (x1 - x0, y1 - y0), (0, 0, 0, 0))
        if img:
            canvas.paste(img, (dx - x0, dy - y0))
        images.append(clear_transparent_rgb(canvas))
    return images, x0, y0


def png_chunk(chunkType, data):
    return struct.pack(">I", len(data)) + chunkType + data + struct.pack(">I", zlib.crc32(chunkType + data) & 0xFFFFFFFF)


def png_chunks(pngBytes):
    if pngBytes[:8] != PNG_SIG:
        raise ValueError("invalid png signature")
    pos = 8
    while pos < len(pngBytes):
        (length,) = struct.unpack(">I", pngBytes[pos:pos + 4])
        chunkType = pngBytes[pos + 4:pos + 8]
        yield chunkType, pngBytes[pos + 8:pos + 8 + length]
        pos += 12 + length


def encode_apng(images, delayMs):
    # Pillow's APNG writer merges identical consecutive frames, which changes the frame count
    # assemble chunks manually, every frame is a full frame with dispose NONE and blend SOURCE
    width, height = images[0].size
    out = [PNG_SIG]
    seq = 0

    for index, img in enumerate(images):
        buf = io.BytesIO()
        img.save(buf, format="PNG", optimize=True)
        chunks = list(png_chunks(buf.getvalue()))

        if index == 0:
            ihdr = next(data for t, data in chunks if t == b"IHDR")
            out.append(png_chunk(b"IHDR", ihdr))
            out.append(png_chunk(b"acTL", struct.pack(">II", len(images), 0)))

        out.append(png_chunk(b"fcTL", struct.pack(">IIIIIHHBB", seq, width, height, 0, 0, delayMs, 1000, 0, 0)))
        seq += 1

        for chunkType, data in chunks:
            if chunkType != b"IDAT":
                continue
            if index == 0:
                out.append(png_chunk(b"IDAT", data))
            else:
                out.append(png_chunk(b"fdAT", struct.pack(">I", seq) + data))
                seq += 1

    out.append(png_chunk(b"IEND", b""))
    return b"".join(out)


def verify_apng(inputDir, outputPath, frames, offX, offY, seqKey):
    meta = decode_new_name(os.path.basename(outputPath))
    expectedName = encode_new_name(seqKey, offX, offY)
    if os.path.basename(outputPath) != expectedName or (meta["dx"], meta["dy"]) != (offX, offY):
        raise RuntimeError(f"{outputPath}: filename round-trip failed")

    with Image.open(outputPath) as apng:
        frameCount = getattr(apng, "n_frames", 1)
        if frameCount != len(frames):
            raise RuntimeError(f"{outputPath}: expect {len(frames)} frames, got {frameCount}")

        for index, (fileName, dx, dy) in enumerate(frames):
            apng.seek(index)
            decoded = clear_transparent_rgb(apng.convert("RGBA"))

            original = clear_transparent_rgb(load_rgba(os.path.join(inputDir, fileName)))
            if is_placeholder(original, dx, dy):
                if decoded.getchannel("A").getbbox():
                    raise RuntimeError(f"{outputPath}: frame {index} should be blank for placeholder {fileName}")
                continue

            expected = Image.new("RGBA", original.size, (0, 0, 0, 0))
            region = decoded.crop((dx - offX, dy - offY, dx - offX + original.width, dy - offY + original.height))
            expected.paste(region, (0, 0))

            if expected.tobytes() != original.tobytes():
                raise RuntimeError(f"{outputPath}: frame {index} differs from {fileName}")


def convert_one(task):
    inputDir, outputDir, seqKey, frames, delayMs, apngopt, verify = task
    images, offX, offY = compose_frames(inputDir, frames)
    if not (OFFSET_MIN <= offX <= OFFSET_MAX and OFFSET_MIN <= offY <= OFFSET_MAX):
        raise RuntimeError(f"sequence {seqKey:06X}: offset ({offX}, {offY}) out of range")

    outputPath = os.path.join(outputDir, encode_new_name(seqKey, offX, offY))
    with open(outputPath, "wb") as fp:
        fp.write(encode_apng(images, delayMs))

    if apngopt:
        tmpPath = outputPath + ".opt"
        subprocess.run([apngopt, "-z2", "-i15", outputPath, tmpPath], check=True, stdout=subprocess.DEVNULL)
        os.replace(tmpPath, outputPath)

    if verify:
        verify_apng(inputDir, outputPath, frames, offX, offY, seqKey)
    return len(frames)


def collect_sequences(inputDir):
    groups = defaultdict(list)
    for fileName in os.listdir(inputDir):
        if not fileName.upper().endswith(".PNG"):
            continue
        key, dx, dy = parse_old_name(fileName)
        groups[key >> 5].append((key & 0x1F, fileName, dx, dy))

    sequences = {}
    for seqKey, items in groups.items():
        items.sort()
        if [i[0] for i in items] != list(range(len(items))):
            raise RuntimeError(f"sequence {seqKey:06X}: frames are not contiguous from 0: {[i[0] for i in items]}")
        sequences[seqKey] = [(f, dx, dy) for _, f, dx, dy in items]
    return sequences


def main():
    parser = argparse.ArgumentParser(description="gfxres2apng: convert per-frame hair PNGs into one APNG per motion.")
    parser.add_argument("input_dir", help="directory of per-frame PNGs, e.g. mir2x_res/texture/hair")
    parser.add_argument("output_dir", help="directory to write APNGs into")
    parser.add_argument("--delay", type=int, default=100, help="frame delay in ms, default 100")
    parser.add_argument("--jobs", type=int, default=os.cpu_count(), help="parallel workers, default CPU count")
    parser.add_argument("--apngopt", default=None, help="path to apngopt, run 'apngopt -z2 -i15' on every output")
    parser.add_argument("--no-verify", dest="verify", action="store_false", help="skip decoding and comparing outputs")
    args = parser.parse_args()

    if not (0 < args.delay < 65536):
        parser.error("--delay must be in [1, 65535]")

    if os.path.abspath(args.input_dir) == os.path.abspath(args.output_dir):
        parser.error("output_dir must differ from input_dir")

    os.makedirs(args.output_dir, exist_ok=True)
    sequences = collect_sequences(args.input_dir)
    tasks = [(args.input_dir, args.output_dir, seqKey, frames, args.delay, args.apngopt, args.verify) for seqKey, frames in sorted(sequences.items())]

    doneSeq = 0
    doneFrame = 0
    with Pool(max(1, args.jobs)) as pool:
        for frameCount in pool.imap_unordered(convert_one, tasks, chunksize=16):
            doneSeq += 1
            doneFrame += frameCount
            if doneSeq % 500 == 0 or doneSeq == len(tasks):
                print(f"gfxres2apng hair: {doneSeq}/{len(tasks)} sequences, {doneFrame} frames", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
