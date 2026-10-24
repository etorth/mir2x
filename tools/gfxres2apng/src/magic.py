#!/usr/bin/env python3
"""gfxres2apng magic resource converter: one APNG per magic gfx sequence.

Usage:
    python3 tools/gfxres2apng/src/magic.py INPUT_DIR OUTPUT_DIR --jobs 8

Input filename (old per-frame format), 18 chars + ".PNG":

    KKKKKKKK S S DDDD DDDD
      KKKKKKKK: 8-hex key, see tools/magicwil2png
                image index in the WIL file(0-23), file index(24-31)
      S S     : sign of dx / dy, '0' means negative
      DDDD    : abs(dx) / abs(dy) in hex

Output filename (one APNG per sequence), 24 hex digits + ".PNG", 96-bit value:

    F IIII D TTT TTT HHH HHH YYY XXX

      X: bits  0-11, x offset, 12-bit two's complement
      Y: bits 12-23, y offset, 12-bit two's complement
      H: bits 24-35, head x, 12-bit two's complement
         bits 36-47, head y, 12-bit two's complement
      T: bits 48-59, tail x, 12-bit two's complement
         bits 60-71, tail y, 12-bit two's complement
      D: bits 72-75, direction index, [0, gfxDirType)
      I: bits 76-91, image index of the sequence's base gfxID, same for all directions
      F: bits 92-95, file index

Offsets, head and tail are all relative to the same anchor, the client draws
the APNG at anchor + (x, y). The head is the hit point of a flying magic, the
tail->head vector is the flying direction. head == tail means the gfx is
isotropic and never rotated. Heads are seeded from MagicGfxEntry::targetOffList,
tails are zero until set by the editor.

Sequences:

    1. Every MagicGfxEntry in common/src/magicrecord.inc, every direction d:
       frames start at gfxID + d * gfxIDCount, the APNG keeps all contiguous
       frames up to gfxIDCount, MagicGfxEntry::frameCount selects the prefix
       the magic uses. Entries sharing a sequence keep the longest one.
       Named by (file, image index of gfxID, d).
    2. Single images hardcoded in client/src/fixedlocmagic.cpp.
    3. Frames not used by the above, magicwil2png skips empty WIL slots, so
       each contiguous run of image indices is one sequence, named by its
       first frame with direction 0.

All frames are aligned on one canvas, the transparent margin shared by all
frames is trimmed, so one x/y offset applies to every frame.

Placeholder frames, fully transparent or with an offset out of the 12-bit
range, stay in the APNG as blank frames, their size and offset are ignored
when computing the canvas.
"""

import argparse
import io
import os
import re
import struct
import subprocess
import sys
import zlib
from multiprocessing import Pool

from PIL import Image

OFFSET_BITS = 12
OFFSET_MIN = -(1 << (OFFSET_BITS - 1))
OFFSET_MAX = (1 << (OFFSET_BITS - 1)) - 1
PNG_SIG = b"\x89PNG\r\n\x1a\n"

HARDCODED_GFX_IDS = (0x0F0000DC, 0x0F000104, 0x0F000105)  # client/src/fixedlocmagic.cpp
DEFAULT_MAGIC_RECORD = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "common", "src", "magicrecord.inc")


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
        raise ValueError(f"value {value} doesn't fit in {bits}-bit two's complement")
    return value & ((1 << bits) - 1)


def from_twos(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def encode_new_name(ident, head, tail, dx, dy):
    fileIndex, imageIndex, direction = ident
    if not (0 <= fileIndex < 16 and 0 <= imageIndex < 65536 and 0 <= direction < 16):
        raise ValueError(f"invalid magic sequence identity: {ident}")

    value = (
        (to_twos(dx, OFFSET_BITS) << 0)
        | (to_twos(dy, OFFSET_BITS) << 12)
        | (to_twos(head[0], OFFSET_BITS) << 24)
        | (to_twos(head[1], OFFSET_BITS) << 36)
        | (to_twos(tail[0], OFFSET_BITS) << 48)
        | (to_twos(tail[1], OFFSET_BITS) << 60)
        | (direction << 72)
        | (imageIndex << 76)
        | (fileIndex << 92)
    )
    return f"{value:024X}.PNG"


def decode_new_name(fileName):
    value = int(os.path.splitext(fileName)[0], 16)
    return {
        "dx": from_twos((value >> 0) & 0xFFF, OFFSET_BITS),
        "dy": from_twos((value >> 12) & 0xFFF, OFFSET_BITS),
        "head": (from_twos((value >> 24) & 0xFFF, OFFSET_BITS), from_twos((value >> 36) & 0xFFF, OFFSET_BITS)),
        "tail": (from_twos((value >> 48) & 0xFFF, OFFSET_BITS), from_twos((value >> 60) & 0xFFF, OFFSET_BITS)),
        "direction": (value >> 72) & 0x0F,
        "image": (value >> 76) & 0xFFFF,
        "file": (value >> 92) & 0x0F,
    }


def matching_brace(text, begin):
    depth = 0
    for pos in range(begin, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return pos
    raise ValueError(f"unbalanced brace at offset {begin}")


def enclosing_block(text, pos):
    depth = 0
    for begin in range(pos, -1, -1):
        if text[begin] == "}":
            depth += 1
        elif text[begin] == "{":
            if depth == 0:
                return begin, matching_brace(text, begin) + 1
            depth -= 1
    raise ValueError(f"no enclosing block at offset {pos}")


def top_level(block):
    # drop nested {...} so fields of nested structs don't match
    out = []
    depth = 0
    for c in block[1:-1]:
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        elif depth == 0:
            out.append(c)
    return "".join(out)


def parse_magic_record(text):
    """Returns [{gfxID, frameCount, stride, dirType, heads, line}] for every MagicGfxEntry with a gfxID."""
    text = re.sub(r"//[^\n]*", "", text)
    entries = []
    for match in re.finditer(r"\.gfxID\s*=", text):
        begin, end = enclosing_block(text, match.start())
        block = text[begin:end]
        fields = top_level(block)
        line = text.count("\n", 0, match.start()) + 1

        def int_field(name, default):
            found = re.search(r"\." + name + r"\s*=\s*([^,\s]+)", fields)
            return int(found.group(1), 0) if found else default

        gfxID = int_field("gfxID", None)
        frameCount = int_field("frameCount", 0)
        stride = int_field("gfxIDCount", frameCount)
        dirType = int_field("gfxDirType", 1)

        if not (0 < frameCount <= stride and 0 < dirType <= 16):
            raise ValueError(f"magicrecord.inc line {line}: invalid frameCount {frameCount}, gfxIDCount {stride}, gfxDirType {dirType}")

        heads = None
        if targetOff := re.search(r"\.targetOffList\s*\{", block):
            listBegin = targetOff.end() - 1
            body = block[listBegin:matching_brace(block, listBegin) + 1]
            heads = [(int(x), int(y)) for x, y in re.findall(r"\{\s*(-?\d+)\s*,\s*(-?\d+)\s*\}", body)]
            if len(heads) != dirType:
                raise ValueError(f"magicrecord.inc line {line}: targetOffList has {len(heads)} entries, expect {dirType}")

        entries.append({"gfxID": gfxID, "frameCount": frameCount, "stride": stride, "dirType": dirType, "heads": heads, "line": line})
    return entries


def build_sequences(frameKeys, entries, hardcodedGfxIDs=HARDCODED_GFX_IDS):
    """Returns ({(file, image, direction): (head, [old keys])}, referencedCount, runCount)."""
    sequences = {}

    def add(ident, head, keys, origin):
        # entries sharing a sequence may use different prefixes of it, keep the longest
        oldHead, oldKeys = sequences.get(ident, (head, keys))
        shorter, longer = sorted((oldKeys, keys), key=len)
        if oldHead != head or longer[:len(shorter)] != shorter:
            raise RuntimeError(f"{origin}: sequence {ident} conflicts with another definition")
        sequences[ident] = (head, longer)

    entries = list(entries) + [{"gfxID": k, "frameCount": 1, "stride": 1, "dirType": 1, "heads": None, "line": "hardcoded"} for k in hardcodedGfxIDs]
    for entry in entries:
        for direction in range(entry["dirType"]):
            start = entry["gfxID"] + direction * entry["stride"]
            count = 0
            while count < entry["stride"] and start + count in frameKeys:
                count += 1

            origin = f"magicrecord.inc line {entry['line']}, gfxID {entry['gfxID']:08X}, direction {direction}"
            if count < entry["frameCount"]:
                raise RuntimeError(f"{origin}: needs {entry['frameCount']} frames from {start:08X}, found {count}")

            head = entry["heads"][direction] if entry["heads"] else (0, 0)
            add((entry["gfxID"] >> 24, entry["gfxID"] & 0xFFFFFF, direction), head, list(range(start, start + count)), origin)

    referencedCount = len(sequences)
    covered = {key for _, keys in sequences.values() for key in keys}

    runs = []
    for key in sorted(set(frameKeys) - covered):
        if runs and key == runs[-1][-1] + 1:
            runs[-1].append(key)
        else:
            runs.append([key])

    for run in runs:
        add((run[0] >> 24, run[0] & 0xFFFFFF, 0), (0, 0), run, f"unreferenced run {run[0]:08X}")
    return sequences, referencedCount, len(runs)


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


def verify_apng(inputDir, outputPath, frames, offX, offY, ident, head):
    meta = decode_new_name(os.path.basename(outputPath))
    if os.path.basename(outputPath) != encode_new_name(ident, head, (0, 0), offX, offY) or (meta["file"], meta["image"], meta["direction"], meta["head"], meta["dx"], meta["dy"]) != (*ident, head, offX, offY):
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
    inputDir, outputDir, ident, head, frames, delayMs, apngopt, verify = task
    images, offX, offY = compose_frames(inputDir, frames)
    if not (OFFSET_MIN <= offX <= OFFSET_MAX and OFFSET_MIN <= offY <= OFFSET_MAX):
        raise RuntimeError(f"sequence {ident}: offset ({offX}, {offY}) out of range")

    outputPath = os.path.join(outputDir, encode_new_name(ident, head, (0, 0), offX, offY))
    with open(outputPath, "wb") as fp:
        fp.write(encode_apng(images, delayMs))

    if apngopt:
        tmpPath = outputPath + ".opt"
        subprocess.run([apngopt, "-z2", "-i15", outputPath, tmpPath], check=True, stdout=subprocess.DEVNULL)
        os.replace(tmpPath, outputPath)

    if verify:
        verify_apng(inputDir, outputPath, frames, offX, offY, ident, head)
    return len(frames)


def collect_frames(inputDir):
    frames = {}
    for fileName in os.listdir(inputDir):
        if not fileName.upper().endswith(".PNG"):
            continue
        key, dx, dy = parse_old_name(fileName)
        if key in frames:
            raise RuntimeError(f"duplicated frame key {key:08X}: {frames[key][0]}, {fileName}")
        frames[key] = (fileName, dx, dy)
    return frames


def main():
    parser = argparse.ArgumentParser(description="gfxres2apng: convert per-frame magic PNGs into one APNG per magic gfx sequence.")
    parser.add_argument("input_dir", help="directory of per-frame PNGs, e.g. mir2x_res/texture/magic")
    parser.add_argument("output_dir", help="directory to write APNGs into")
    parser.add_argument("--magic-record", default=DEFAULT_MAGIC_RECORD, help="path to magicrecord.inc, default common/src/magicrecord.inc")
    parser.add_argument("--delay", type=int, default=100, help="frame delay in ms, default 100")
    parser.add_argument("--jobs", type=int, default=os.cpu_count(), help="parallel workers, default CPU count")
    parser.add_argument("--apngopt", default=None, help="path to apngopt, run 'apngopt -z2 -i15' on every output")
    parser.add_argument("--no-verify", dest="verify", action="store_false", help="skip decoding and comparing outputs")
    args = parser.parse_args()

    if not (0 < args.delay < 65536):
        parser.error("--delay must be in [1, 65535]")

    if os.path.abspath(args.input_dir) == os.path.abspath(args.output_dir):
        parser.error("output_dir must differ from input_dir")

    with open(args.magic_record, encoding="utf-8") as fp:
        entries = parse_magic_record(fp.read())

    frames = collect_frames(args.input_dir)
    sequences, referencedCount, runCount = build_sequences(frames, entries)
    print(f"gfxres2apng magic: {len(entries)} gfx entries, {referencedCount} referenced sequences, {runCount} unreferenced runs", flush=True)

    os.makedirs(args.output_dir, exist_ok=True)
    tasks = [(args.input_dir, args.output_dir, ident, head, [frames[k] for k in keys], args.delay, args.apngopt, args.verify) for ident, (head, keys) in sorted(sequences.items())]

    doneSeq = 0
    doneFrame = 0
    with Pool(max(1, args.jobs)) as pool:
        for frameCount in pool.imap_unordered(convert_one, tasks, chunksize=8):
            doneSeq += 1
            doneFrame += frameCount
            if doneSeq % 200 == 0 or doneSeq == len(tasks):
                print(f"gfxres2apng magic: {doneSeq}/{len(tasks)} sequences, {doneFrame} frames", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
