#!/usr/bin/env python3
"""gfxres2apng monster resource converter: one APNG per motion sequence.

Usage:
    python3 tools/gfxres2apng/src/monster.py INPUT_DIR OUTPUT_DIR --jobs 8

Input filename (old per-frame format), 18 chars + ".PNG":

    KKKKKKKK S S DDDD DDDD
      KKKKKKKK: 8-hex key, see MonsterFrameGfxSeq::gfxID()
                frame(0-4), direction(5-7), motion(8-11), look(12-22), shadow(23)
      S S     : sign of dx / dy, '0' means negative
      DDDD    : abs(dx) / abs(dy) in hex

Output filename (one APNG per motion), 14 hex digits + ".PNG", 56-bit value:

    0b_0000_0000_0000_gggg_gggg_gggs_lmmm_mddd_yyyy_yyyy_yyyy_xxxx_xxxx_xxxx

      x: bits  0-11, x offset, 12-bit two's complement
      y: bits 12-23, y offset, 12-bit two's complement
      d: bits 24-26, direction
      m: bits 27-30, motion
      l: bit  31   , layer, always 0 for monsters
      s: bit  32   , shadow
      g: bits 33-43, look

The frame index is no longer in the filename, it's the APNG frame order.
All frames are aligned on one canvas, the transparent margin shared by all
frames is trimmed, so one x/y offset applies to every frame.
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
    dx = int(stem[10:14], 16) * (1 if stem[8] != "0" else -1)
    dy = int(stem[14:18], 16) * (1 if stem[9] != "0" else -1)
    return key, dx, dy


def to_twos(value, bits):
    if not (-(1 << (bits - 1)) <= value < (1 << (bits - 1))):
        raise ValueError(f"offset {value} doesn't fit in {bits}-bit two's complement")
    return value & ((1 << bits) - 1)


def from_twos(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def encode_new_name(seqKey, layer, dx, dy):
    # seqKey is the old key >> 5: direction(0-2), motion(3-6), look(7-17), shadow(18)
    direction = (seqKey >> 0) & 0x07
    motion = (seqKey >> 3) & 0x0F
    look = (seqKey >> 7) & 0x07FF
    shadow = (seqKey >> 18) & 0x01

    value = (
        (to_twos(dx, OFFSET_BITS) << 0)
        | (to_twos(dy, OFFSET_BITS) << 12)
        | (direction << 24)
        | (motion << 27)
        | ((layer & 0x01) << 31)
        | (shadow << 32)
        | (look << 33)
    )
    return f"{value:014X}.PNG"


def decode_new_name(fileName):
    value = int(os.path.splitext(fileName)[0], 16)
    return {
        "dx": from_twos((value >> 0) & 0xFFF, OFFSET_BITS),
        "dy": from_twos((value >> 12) & 0xFFF, OFFSET_BITS),
        "direction": (value >> 24) & 0x07,
        "motion": (value >> 27) & 0x0F,
        "layer": (value >> 31) & 0x01,
        "shadow": (value >> 32) & 0x01,
        "look": (value >> 33) & 0x07FF,
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


def compose_frames(inputDir, frames):
    """frames: [(fileName, dx, dy)] in frame order. Returns (images, offX, offY)."""
    loaded = [(load_rgba(os.path.join(inputDir, f)), dx, dy) for f, dx, dy in frames]

    x0 = min(dx for _, dx, _ in loaded)
    y0 = min(dy for _, _, dy in loaded)
    x1 = max(dx + img.width for img, dx, _ in loaded)
    y1 = max(dy + img.height for img, _, dy in loaded)

    canvases = []
    bbox = None
    for img, dx, dy in loaded:
        canvas = Image.new("RGBA", (x1 - x0, y1 - y0), (0, 0, 0, 0))
        canvas.paste(img, (dx - x0, dy - y0))
        canvases.append(canvas)

        if box := canvas.getchannel("A").getbbox():
            bbox = box if bbox is None else (min(bbox[0], box[0]), min(bbox[1], box[1]), max(bbox[2], box[2]), max(bbox[3], box[3]))

    if bbox is None:
        bbox = (0, 0, 1, 1)  # all frames fully transparent, keep a 1x1 canvas

    images = [clear_transparent_rgb(c.crop(bbox)) for c in canvases]
    return images, x0 + bbox[0], y0 + bbox[1]


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
    expectedName = encode_new_name(seqKey, 0, offX, offY)
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

    outputPath = os.path.join(outputDir, encode_new_name(seqKey, 0, offX, offY))
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
    parser = argparse.ArgumentParser(description="gfxres2apng: convert per-frame monster PNGs into one APNG per motion.")
    parser.add_argument("input_dir", help="directory of per-frame PNGs, e.g. mir2x_res/texture/monster")
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
                print(f"gfxres2apng monster: {doneSeq}/{len(tasks)} sequences, {doneFrame} frames", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
