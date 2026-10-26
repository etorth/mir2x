#!/usr/bin/env python3
"""gfxres2apng: convert per-frame texture PNGs into one APNG per animation sequence.

Usage:
    python3 tools/gfxres2apng/src/gfxres2apng.py --res-path MIR2X_RES --output-dir OUTPUT_DIR [--types monster,hero] --jobs 8

Reads <res-path>/texture/<type>, writes <output-dir>/texture/<type>, for every
type in TEXTURE_TYPES, or the subset given by --types.

Input filename (old per-frame format), 18 chars + ".PNG":

    KKKKKKKK S S DDDD DDDD
      KKKKKKKK: 8-hex key, layout depends on the type, see below
      S S     : sign of dx / dy, '0' means negative
      DDDD    : abs(dx) / abs(dy) in hex

Output filename, hex digits + ".PNG", offsets are always the lowest 24 bits:

      x: bits  0-11, x offset, 12-bit two's complement
      y: bits 12-23, y offset, 12-bit two's complement

monster, npc
    old key: frame(0-4), direction(5-7), motion(8-11), look(12-22), shadow(23)
             see monsterwil2png/npcwil2png, client/src/monster.cpp and clientnpc.cpp
    new name, 14 hex digits:
      d: bits 24-26, direction
      m: bits 27-30, motion
      l: bit  31   , layer, always 0
      s: bit  32   , shadow
      g: bits 33-43, look
         bits 44-55 are zero

hero, hair, helmet, weapon
    old key: frame(0-4), direction(5-7), motion(8-13), id(14-21), gender(22), shadow(23), layer(24)
             hero has all fields, weapon has no layer, hair and helmet have no layer or shadow
             see herowil2png/hairwil2png/helmetwil2png/weaponwil2png and client/src/hero.cpp
    new name, 14 hex digits:
      d: bits 24-26, direction
      m: bits 27-32, motion
      l: bit  33   , layer, weapon uses 1 for the weapon effect
      s: bit  34   , shadow
      g: bit  35   , gender
      i: bits 36-43, dress / hair / helmet / weapon ID
         bits 44-55 are zero

    Weapon effect, the glow drawn over weapon 44, comes from the magic resource,
    MonMagicEx4.wil (file index 7) image index 1900-5815, it uses the same
    anchor and slot layout as the weapon:

        image index = base + motion * 80 + direction * 10 + frame
        base        = 1900 for male, 3900 for female

    It's written as weapon 44, layer 1, no shadow. It also has gfx motion 1
    and 20 which the weapon resource doesn't have, they are kept.

magic
    old key: image index in the WIL file(0-23), file index(24-31), see magicwil2png
    new name, 24 hex digits:
      H: bits 24-35, head x, 12-bit two's complement
         bits 36-47, head y, 12-bit two's complement
      T: bits 48-59, tail x, 12-bit two's complement
         bits 60-71, tail y, 12-bit two's complement
      D: bits 72-75, direction index, [0, gfxDirType)
      I: bits 76-91, image index of the sequence's base gfxID, same for all directions
      F: bits 92-95, file index

    Offsets, head and tail are all relative to the same anchor. The head is the
    hit point of a flying magic, the tail->head vector is the flying direction,
    head == tail means the gfx is isotropic and never rotated. Heads are seeded
    from MagicGfxEntry::targetOffList, tails are zero until set by the editor.

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

    Frames in MAGIC_EXCLUDED_KEY_RANGES are skipped, they are the weapon effect.

equip
    static UI images, not animations, a PNG is already a 1-frame APNG, so each
    file is copied byte-for-byte, only renamed
    new name, 14 hex digits:
      K: bits 24-55, the old 32-bit key, unchanged

selectchar
    old key: frame(0-4), motion(5-8), gender(9), job gfx index(10-12), magic overlay(13), shadow(14)
             see ProcessSelectChar::charGfxBaseID() and ProcessCreateChar::charGfxBaseID()
    new name, same layout as hero, 14 hex digits:
      d: bits 24-26, direction, always 0
      m: bits 27-32, motion
      l: bit  33   , magic overlay
      s: bit  34   , shadow
      g: bit  35   , gender, 1 is male
      i: bits 36-43, job gfx index
         bits 44-55 are zero

    The client draws the shadow and the magic overlay at the body's frame index,
    so every body sequence must be contiguous from frame 0, and its shadow and
    overlay sequences are padded with blank frames to the body's frame count,
    e.g. an overlay with frames 4-19 over a 20-frame body gets blank frames 0-3.

proguse
    old key: 8 hex digits, "KKKKKKKK.PNG", no offset, see client/src/progusedb.hpp
    groups come from proguse_groups_v3.txt (--proguse-groups), one APNG per line:
        anim    FIRST-LAST  DELAY  [name=KEY]    frames FIRST..LAST, DELAY ms per frame
        button  OFF  ON  DOWN  [name=KEY]    3 frames, frame 0/1/2 = off/on/down
    new name, 8 hex digits:
      K: bits 0-31, the group's smallest key, or KEY given by name=KEY
    A key may be in more than one group, every APNG name must be unique.
    Frames are not trimmed, all images of a group must have the same size.
    Every key not in any group is a static image, copied byte-for-byte to
    KKKKKKKK.PNG, a lowercase ".png" extension becomes ".PNG".

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
import re
import shutil
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

TEXTURE_TYPES = ("monster", "npc", "hero", "hair", "helmet", "weapon", "magic", "equip", "selectchar", "proguse")
COPY_TYPES = ("equip",)  # static images, copied unchanged and renamed

# old keys must be below these limits, see the old key layouts above
KEY_LIMITS = {
    "monster": 1 << 24,
    "npc": 1 << 24,
    "hero": 1 << 25,
    "hair": 1 << 23,
    "helmet": 1 << 23,
    "weapon": 1 << 24,
}

HUMAN_LAYER_BIT = 19  # in seqKey, old key bit 24

EFFECT_WEAPON = 44
EFFECT_FILE_INDEX = 7
EFFECT_IMAGE_BEGIN = 1900
EFFECT_IMAGE_END = 5815  # inclusive
EFFECT_GENDER_BASE = {1: 1900, 0: 3900}
EFFECT_MOTION_COUNT = 25  # 2000 slots per gender

MAGIC_HARDCODED_GFX_IDS = (0x0F0000DC, 0x0F000104, 0x0F000105)  # client/src/fixedlocmagic.cpp
MAGIC_EXCLUDED_KEY_RANGES = ((0x0700076C, 0x070016B7),)  # weapon 44 effect
DEFAULT_MAGIC_RECORD = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "common", "src", "magicrecord.inc")
DEFAULT_PROGUSE_GROUPS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "proguse_groups_v3.txt")


def parse_old_name(fileName):
    stem, ext = os.path.splitext(fileName)
    if ext.upper() != ".PNG" or len(stem) != 18 or stem[8] not in "01" or stem[9] not in "01":
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


def encode_creature_name(seqKey, dx, dy):
    # seqKey is the old key >> 5: direction(0-2), motion(3-6), look(7-17), shadow(18)
    value = (
        (to_twos(dx, OFFSET_BITS) << 0)
        | (to_twos(dy, OFFSET_BITS) << 12)
        | (((seqKey >> 0) & 0x07) << 24)
        | (((seqKey >> 3) & 0x0F) << 27)
        | (((seqKey >> 18) & 0x01) << 32)
        | (((seqKey >> 7) & 0x07FF) << 33)
    )
    return f"{value:014X}.PNG"


def decode_creature_name(fileName):
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


def encode_human_name(seqKey, dx, dy):
    # seqKey is the old key >> 5: direction(0-2), motion(3-8), id(9-16), gender(17), shadow(18), layer(19)
    value = (
        (to_twos(dx, OFFSET_BITS) << 0)
        | (to_twos(dy, OFFSET_BITS) << 12)
        | (((seqKey >> 0) & 0x07) << 24)
        | (((seqKey >> 3) & 0x3F) << 27)
        | (((seqKey >> HUMAN_LAYER_BIT) & 0x01) << 33)
        | (((seqKey >> 18) & 0x01) << 34)
        | (((seqKey >> 17) & 0x01) << 35)
        | (((seqKey >> 9) & 0xFF) << 36)
    )
    return f"{value:014X}.PNG"


def decode_human_name(fileName):
    value = int(os.path.splitext(fileName)[0], 16)
    return {
        "dx": from_twos((value >> 0) & 0xFFF, OFFSET_BITS),
        "dy": from_twos((value >> 12) & 0xFFF, OFFSET_BITS),
        "direction": (value >> 24) & 0x07,
        "motion": (value >> 27) & 0x3F,
        "layer": (value >> 33) & 0x01,
        "shadow": (value >> 34) & 0x01,
        "gender": (value >> 35) & 0x01,
        "id": (value >> 36) & 0xFF,
    }


def encode_magic_name(ident, dx, dy, tail=(0, 0)):
    fileIndex, imageIndex, direction, head = ident
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


def decode_magic_name(fileName):
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


def encode_static_name(key, dx, dy):
    if not (0 <= key < (1 << 32)):
        raise ValueError(f"invalid static image key: {key}")
    return f"{(key << 24) | (to_twos(dy, OFFSET_BITS) << 12) | to_twos(dx, OFFSET_BITS):014X}.PNG"


def decode_static_name(fileName):
    value = int(os.path.splitext(fileName)[0], 16)
    return {
        "dx": from_twos((value >> 0) & 0xFFF, OFFSET_BITS),
        "dy": from_twos((value >> 12) & 0xFFF, OFFSET_BITS),
        "key": (value >> 24) & 0xFFFFFFFF,
    }


def encode_proguse_name(key):
    if not (0 <= key < (1 << 32)):
        raise ValueError(f"invalid proguse key: {key}")
    return f"{key:08X}.PNG"


def decode_proguse_name(fileName):
    stem, ext = os.path.splitext(fileName)
    if not re.fullmatch(r"[0-9A-Fa-f]{8}", stem) or ext.upper() != ".PNG":
        raise ValueError(f"invalid proguse filename: {fileName}")
    return int(stem, 16)


NAME_CODECS = {
    "monster": (encode_creature_name, decode_creature_name),
    "npc": (encode_creature_name, decode_creature_name),
    "hero": (encode_human_name, decode_human_name),
    "hair": (encode_human_name, decode_human_name),
    "helmet": (encode_human_name, decode_human_name),
    "weapon": (encode_human_name, decode_human_name),
    "magic": (encode_magic_name, decode_magic_name),
    "equip": (encode_static_name, decode_static_name),
    "selectchar": (encode_human_name, decode_human_name),
    "proguse": (encode_proguse_name, decode_proguse_name),
}


def list_frames(inputDir):
    for fileName in os.listdir(inputDir):
        if fileName.upper().endswith(".PNG"):
            key, dx, dy = parse_old_name(fileName)
            yield key, (os.path.abspath(os.path.join(inputDir, fileName)), dx, dy)


def collect_frame_groups(inputDir, keyLimit):
    """Groups frames by old key >> 5, returns {seqKey: [(path, dx, dy)]} in frame order."""
    groups = defaultdict(list)
    for key, frame in list_frames(inputDir):
        if not (0 <= key < keyLimit):
            raise ValueError(f"frame key {key:08X} out of range: {frame[0]}")
        groups[key >> 5].append((key & 0x1F, frame))

    sequences = {}
    for seqKey, items in groups.items():
        items.sort()
        if [i[0] for i in items] != list(range(len(items))):
            raise RuntimeError(f"{inputDir}: sequence {seqKey:06X}: frames are not contiguous from 0: {[i[0] for i in items]}")
        sequences[seqKey] = [frame for _, frame in items]
    return sequences


def collect_weapon_effect_sequences(magicDir):
    frames = {key & 0xFFFFFF: frame for key, frame in list_frames(magicDir) if key >> 24 == EFFECT_FILE_INDEX and EFFECT_IMAGE_BEGIN <= key & 0xFFFFFF <= EFFECT_IMAGE_END}

    sequences = {}
    used = set()
    for gender, base in EFFECT_GENDER_BASE.items():
        for motion in range(EFFECT_MOTION_COUNT):
            for direction in range(8):
                slot = base + motion * 80 + direction * 10
                found = [f for f in range(10) if slot + f in frames]
                if not found:
                    continue
                if found != list(range(len(found))):
                    raise RuntimeError(f"weapon effect image {slot}: frames are not contiguous from 0: {found}")

                seqKey = (1 << HUMAN_LAYER_BIT) | (gender << 17) | (EFFECT_WEAPON << 9) | (motion << 3) | direction
                sequences[seqKey] = [frames[slot + f] for f in found]
                used.update(slot + f for f in found)

    if missed := sorted(set(frames) - used):
        raise RuntimeError(f"weapon effect images not in any sequence: {missed[:10]}")
    return sequences


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


def build_magic_sequences(frameKeys, entries, hardcodedGfxIDs=MAGIC_HARDCODED_GFX_IDS):
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


def collect_magic_sequences(magicDir, magicRecord):
    frames = {}
    for key, frame in list_frames(magicDir):
        if any(begin <= key <= end for begin, end in MAGIC_EXCLUDED_KEY_RANGES):
            continue
        if key in frames:
            raise RuntimeError(f"duplicated magic frame key {key:08X}: {frames[key][0]}, {frame[0]}")
        frames[key] = frame

    with open(magicRecord, encoding="utf-8") as fp:
        entries = parse_magic_record(fp.read())

    sequences, referencedCount, runCount = build_magic_sequences(frames, entries)
    print(f"gfxres2apng magic: {len(entries)} gfx entries, {referencedCount} referenced sequences, {runCount} unreferenced runs", flush=True)
    return {(*ident, head): [frames[k] for k in keys] for ident, (head, keys) in sequences.items()}


def collect_selectchar_sequences(inputDir):
    """Returns {hero-layout seqKey: [(path, dx, dy) or None]}, None is a blank padding frame."""
    groups = defaultdict(dict)
    for key, frame in list_frames(inputDir):
        if not (0 <= key < (1 << 15)):
            raise ValueError(f"frame key {key:08X} out of range: {frame[0]}")
        groups[key >> 5][key & 0x1F] = frame

    def to_seq_key(oldSeqKey):
        # old seqKey: motion(0-3), gender(4), job(5-7), magic(8), shadow(9)
        motion, gender, job, magic, shadow = oldSeqKey & 0x0F, (oldSeqKey >> 4) & 1, (oldSeqKey >> 5) & 0x07, (oldSeqKey >> 8) & 1, (oldSeqKey >> 9) & 1
        return (magic << HUMAN_LAYER_BIT) | (shadow << 18) | (gender << 17) | (job << 9) | (motion << 3)

    sequences = {}
    for oldSeqKey, frames in groups.items():
        bodyFrames = groups.get(oldSeqKey & 0xFF)
        if not bodyFrames or sorted(bodyFrames) != list(range(len(bodyFrames))):
            raise RuntimeError(f"{inputDir}: body sequence {(oldSeqKey & 0xFF) << 5:08X} must exist with frames contiguous from 0")
        if max(frames) >= len(bodyFrames):
            raise RuntimeError(f"{inputDir}: sequence {oldSeqKey << 5:08X} has frame {max(frames)}, body has {len(bodyFrames)} frames")
        sequences[to_seq_key(oldSeqKey)] = [frames.get(i) for i in range(len(bodyFrames))]
    return sequences


def parse_proguse_groups(text):
    """Returns [(name, [key per frame], delayMs or None)], delayMs is None for buttons."""
    groups = []
    names = {}
    for lineNo, line in enumerate(text.splitlines(), 1):
        tokens = line.split("#", 1)[0].split()
        if not tokens:
            continue

        def fail(msg):
            raise ValueError(f"proguse groups line {lineNo}: {msg}: {line.strip()}")

        def parse_key(token):
            if not re.fullmatch(r"[0-9A-Fa-f]{8}", token):
                fail(f"invalid key {token!r}")
            return int(token, 16)

        name = None
        if tokens[-1].startswith("name="):
            name = parse_key(tokens.pop()[len("name="):])

        if tokens[0] == "anim" and len(tokens) == 3:
            first, sep, last = tokens[1].partition("-")
            if not sep:
                fail("expect FIRST-LAST")
            first, last = parse_key(first), parse_key(last)
            if first > last:
                fail("FIRST > LAST")
            if not re.fullmatch(r"[0-9]+", tokens[2]) or not (0 < int(tokens[2]) < 65536):
                fail("DELAY must be in [1, 65535]")
            keys, delayMs = list(range(first, last + 1)), int(tokens[2])
        elif tokens[0] == "button" and len(tokens) == 4:
            keys, delayMs = [parse_key(t) for t in tokens[1:]], None
        else:
            fail("expect 'anim FIRST-LAST DELAY' or 'button OFF ON DOWN [name=KEY]'")

        name = min(keys) if name is None else name
        if name not in keys:
            fail(f"name {name:08X} is not a key of the group")
        if name in names:
            fail(f"APNG name {name:08X} already used by line {names[name]}")
        names[name] = lineNo
        groups.append((name, keys, delayMs))
    return groups


def collect_proguse_sequences(inputDir, groupsPath):
    """Returns {key: ([path per frame], delayMs or None, isAPNG)}, a non-APNG is a static image copy."""
    files = {}
    for fileName in os.listdir(inputDir):
        key = decode_proguse_name(fileName)
        if key in files:
            raise RuntimeError(f"duplicated proguse key {key:08X}: {files[key]}, {fileName}")
        files[key] = os.path.abspath(os.path.join(inputDir, fileName))

    with open(groupsPath, encoding="utf-8") as fp:
        groups = parse_proguse_groups(fp.read())

    sequences = {}
    grouped = set()
    for name, keys, delayMs in groups:
        missing = [f"{k:08X}" for k in keys if k not in files]
        if missing:
            raise RuntimeError(f"{groupsPath}: group {name:08X}: missing {', '.join(missing)} in {inputDir}")
        sequences[name] = ([files[k] for k in keys], delayMs, True)
        grouped.update(keys)

    for key, path in files.items():
        if key not in grouped:
            sequences[key] = ([path], None, False)
    return sequences


def collect_sequences(resPath, typeName, magicRecord=DEFAULT_MAGIC_RECORD, proguseGroups=DEFAULT_PROGUSE_GROUPS):
    """Returns {ident: [(path, dx, dy)]}, ident is what NAME_CODECS[typeName] encodes.
    proguse is different, see collect_proguse_sequences()."""
    inputDir = os.path.join(resPath, "texture", typeName)
    if typeName == "magic":
        return collect_magic_sequences(inputDir, magicRecord)

    if typeName == "proguse":
        return collect_proguse_sequences(inputDir, proguseGroups)

    if typeName in COPY_TYPES:
        sequences = {}
        for key, frame in list_frames(inputDir):
            if key in sequences:
                raise RuntimeError(f"duplicated {typeName} key {key:08X}: {sequences[key][0][0]}, {frame[0]}")
            sequences[key] = [frame]
        return sequences

    if typeName == "selectchar":
        return collect_selectchar_sequences(inputDir)

    sequences = collect_frame_groups(inputDir, KEY_LIMITS[typeName])
    if typeName == "weapon":
        sequences.update(collect_weapon_effect_sequences(os.path.join(resPath, "texture", "magic")))
    return sequences


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


def compose_frames(frames):
    """frames: [(path, dx, dy) or None] in frame order, None is a blank frame. Returns (images, offX, offY)."""
    loaded = [(load_rgba(f[0]), f[1], f[2]) if f else (None, 0, 0) for f in frames]
    loaded = [(None if img is None or is_placeholder(img, dx, dy) else img, dx, dy) for img, dx, dy in loaded]

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


def verify_apng(typeName, outputPath, frames, offX, offY, ident):
    encode, decode = NAME_CODECS[typeName]
    meta = decode(os.path.basename(outputPath))
    if (meta["dx"], meta["dy"]) != (offX, offY) or encode(ident, meta["dx"], meta["dy"]) != os.path.basename(outputPath):
        raise RuntimeError(f"{outputPath}: filename round-trip failed")

    with Image.open(outputPath) as apng:
        frameCount = getattr(apng, "n_frames", 1)
        if frameCount != len(frames):
            raise RuntimeError(f"{outputPath}: expect {len(frames)} frames, got {frameCount}")

        for index, frame in enumerate(frames):
            apng.seek(index)
            decoded = clear_transparent_rgb(apng.convert("RGBA"))

            if frame is None:
                if decoded.getchannel("A").getbbox():
                    raise RuntimeError(f"{outputPath}: padding frame {index} should be blank")
                continue

            path, dx, dy = frame
            original = clear_transparent_rgb(load_rgba(path))
            if is_placeholder(original, dx, dy):
                if decoded.getchannel("A").getbbox():
                    raise RuntimeError(f"{outputPath}: frame {index} should be blank for placeholder {path}")
                continue

            expected = Image.new("RGBA", original.size, (0, 0, 0, 0))
            region = decoded.crop((dx - offX, dy - offY, dx - offX + original.width, dy - offY + original.height))
            expected.paste(region, (0, 0))

            if expected.tobytes() != original.tobytes():
                raise RuntimeError(f"{outputPath}: frame {index} differs from {path}")


def copy_one(typeName, outputDir, ident, frame, verify):
    path, dx, dy = frame
    outputPath = os.path.join(outputDir, NAME_CODECS[typeName][0](ident, dx, dy))
    shutil.copyfile(path, outputPath)

    if verify:
        meta = NAME_CODECS[typeName][1](os.path.basename(outputPath))
        if (meta["key"], meta["dx"], meta["dy"]) != (ident, dx, dy):
            raise RuntimeError(f"{outputPath}: filename round-trip failed")
        with open(path, "rb") as src, open(outputPath, "rb") as dst:
            if src.read() != dst.read():
                raise RuntimeError(f"{outputPath}: differs from {path}")
    return 1


def optimize_apng(apngopt, outputPath):
    tmpPath = outputPath + ".opt"
    subprocess.run([apngopt, "-z2", "-i15", outputPath, tmpPath], check=True, stdout=subprocess.DEVNULL)
    os.replace(tmpPath, outputPath)


def convert_proguse_one(outputDir, key, paths, delayMs, isAPNG, apngopt, verify):
    outputPath = os.path.join(outputDir, encode_proguse_name(key))
    if not isAPNG:
        shutil.copyfile(paths[0], outputPath)
        if verify:
            with open(paths[0], "rb") as src, open(outputPath, "rb") as dst:
                if src.read() != dst.read():
                    raise RuntimeError(f"{outputPath}: differs from {paths[0]}")
        return 1

    images = [clear_transparent_rgb(load_rgba(path)) for path in paths]
    if len({img.size for img in images}) != 1:
        raise RuntimeError(f"proguse group {key:08X}: images have different sizes: {paths}")

    with open(outputPath, "wb") as fp:
        fp.write(encode_apng(images, delayMs))

    if apngopt:
        optimize_apng(apngopt, outputPath)

    if verify:
        if decode_proguse_name(os.path.basename(outputPath)) != key:
            raise RuntimeError(f"{outputPath}: filename round-trip failed")
        with Image.open(outputPath) as apng:
            if getattr(apng, "n_frames", 1) != len(images):
                raise RuntimeError(f"{outputPath}: expect {len(images)} frames, got {getattr(apng, 'n_frames', 1)}")
            for index, (img, path) in enumerate(zip(images, paths)):
                apng.seek(index)
                decoded = clear_transparent_rgb(apng.convert("RGBA"))
                if decoded.size != img.size or decoded.tobytes() != img.tobytes():
                    raise RuntimeError(f"{outputPath}: frame {index} differs from {path}")
    return len(set(paths))


def convert_one(task):
    typeName, outputDir, ident, frames, delayMs, apngopt, verify = task
    if typeName == "proguse":
        paths, groupDelayMs, isAPNG = frames
        return convert_proguse_one(outputDir, ident, paths, groupDelayMs or delayMs, isAPNG, apngopt, verify)

    if typeName in COPY_TYPES:
        return copy_one(typeName, outputDir, ident, frames[0], verify)

    images, offX, offY = compose_frames(frames)
    if not (OFFSET_MIN <= offX <= OFFSET_MAX and OFFSET_MIN <= offY <= OFFSET_MAX):
        raise RuntimeError(f"{typeName} sequence {ident}: offset ({offX}, {offY}) out of range")

    outputPath = os.path.join(outputDir, NAME_CODECS[typeName][0](ident, offX, offY))
    with open(outputPath, "wb") as fp:
        fp.write(encode_apng(images, delayMs))

    if apngopt:
        optimize_apng(apngopt, outputPath)

    if verify:
        verify_apng(typeName, outputPath, frames, offX, offY, ident)
    return sum(1 for f in frames if f)


def main():
    parser = argparse.ArgumentParser(description="gfxres2apng: convert per-frame texture PNGs into one APNG per animation sequence.")
    parser.add_argument("--res-path", required=True, help="mir2x_res root, reads <res-path>/texture/<type>")
    parser.add_argument("--output-dir", required=True, help="writes <output-dir>/texture/<type>")
    parser.add_argument("--types", default=",".join(TEXTURE_TYPES), help=f"comma separated texture types, default all: {','.join(TEXTURE_TYPES)}")
    parser.add_argument("--magic-record", default=DEFAULT_MAGIC_RECORD, help="path to magicrecord.inc, default common/src/magicrecord.inc")
    parser.add_argument("--proguse-groups", default=DEFAULT_PROGUSE_GROUPS, help="proguse group list, default proguse_groups_v3.txt next to this script")
    parser.add_argument("--delay", type=int, default=100, help="frame delay in ms, default 100, proguse anim groups use their own DELAY")
    parser.add_argument("--jobs", type=int, default=os.cpu_count(), help="parallel workers, default CPU count")
    parser.add_argument("--apngopt", default=None, help="path to apngopt, run 'apngopt -z2 -i15' on every output")
    parser.add_argument("--no-verify", dest="verify", action="store_false", help="skip decoding and comparing outputs")
    args = parser.parse_args()

    types = [t for t in args.types.split(",") if t]
    if not types or any(t not in TEXTURE_TYPES for t in types) or len(set(types)) != len(types):
        parser.error(f"--types must be distinct values of {','.join(TEXTURE_TYPES)}")

    if not (0 < args.delay < 65536):
        parser.error("--delay must be in [1, 65535]")

    if os.path.abspath(args.res_path) == os.path.abspath(args.output_dir):
        parser.error("--output-dir must differ from --res-path")

    with Pool(max(1, args.jobs)) as pool:
        for typeName in types:
            outputDir = os.path.join(args.output_dir, "texture", typeName)
            os.makedirs(outputDir, exist_ok=True)

            sequences = collect_sequences(args.res_path, typeName, args.magic_record, args.proguse_groups)
            tasks = [(typeName, outputDir, ident, frames, args.delay, args.apngopt, args.verify) for ident, frames in sorted(sequences.items())]

            doneSeq = 0
            doneFrame = 0
            for frameCount in pool.imap_unordered(convert_one, tasks, chunksize=8):
                doneSeq += 1
                doneFrame += frameCount
                if doneSeq % 500 == 0 or doneSeq == len(tasks):
                    print(f"gfxres2apng {typeName}: {doneSeq}/{len(tasks)} sequences, {doneFrame} frames", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
