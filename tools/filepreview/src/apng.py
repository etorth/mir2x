"""Minimal pure-python PNG/APNG parser.

Splits an APNG into standalone PNG byte streams, one per frame, plus the
frame control info needed to composite them. Decoding of the per-frame PNG
data is left to the caller (e.g. QImage.fromData).
"""

import struct
import zlib
from dataclasses import dataclass, field

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

DISPOSE_NONE = 0
DISPOSE_BACKGROUND = 1
DISPOSE_PREVIOUS = 2

BLEND_SOURCE = 0
BLEND_OVER = 1


class PNGError(Exception):
    pass


@dataclass
class APNGFrame:
    png: bytes
    width: int
    height: int
    x: int
    y: int
    delay_ms: int
    dispose_op: int
    blend_op: int


@dataclass
class APNGInfo:
    width: int
    height: int
    num_plays: int = 0
    is_animated: bool = False
    frames: list = field(default_factory=list)


def _chunk(ctype: bytes, data: bytes) -> bytes:
    return struct.pack(">I", len(data)) + ctype + data + struct.pack(">I", zlib.crc32(ctype + data) & 0xFFFFFFFF)


def iter_chunks(data: bytes):
    if not data.startswith(PNG_SIGNATURE):
        raise PNGError("not a PNG file")
    pos = len(PNG_SIGNATURE)
    while pos + 8 <= len(data):
        length, ctype = struct.unpack(">I4s", data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + length]
        if len(body) != length:
            raise PNGError("truncated chunk %r" % ctype)
        yield ctype, body
        pos += 12 + length
        if ctype == b"IEND":
            break


def _delay_ms(num: int, den: int) -> int:
    if den == 0:
        den = 100
    return int(round(num * 1000.0 / den))


def parse_png(data: bytes) -> APNGInfo:
    ihdr = None
    actl = None
    shared = []         # ancillary chunks before first IDAT (PLTE, tRNS, gAMA, ...)
    frames_raw = []     # [fctl_tuple, [data, ...]]
    cur = None
    seen_idat = False
    default_image = []  # IDAT data
    default_is_frame = False

    for ctype, body in iter_chunks(data):
        if ctype == b"IHDR":
            ihdr = body
        elif ctype == b"acTL":
            actl = struct.unpack(">II", body[:8])
        elif ctype == b"fcTL":
            fctl = struct.unpack(">IIIIIHHBB", body[:26])
            cur = [fctl, []]
            frames_raw.append(cur)
            if not seen_idat:
                default_is_frame = True
        elif ctype == b"IDAT":
            seen_idat = True
            default_image.append(body)
            if default_is_frame and cur is not None:
                cur[1].append(body)
        elif ctype == b"fdAT":
            if cur is not None:
                cur[1].append(body[4:])
        elif ctype == b"IEND":
            break
        elif not seen_idat:
            shared.append((ctype, body))

    if ihdr is None:
        raise PNGError("missing IHDR")
    width, height = struct.unpack(">II", ihdr[:8])
    info = APNGInfo(width=width, height=height)

    def build(w, h, parts):
        hdr = struct.pack(">II", w, h) + ihdr[8:]
        out = [PNG_SIGNATURE, _chunk(b"IHDR", hdr)]
        out += [_chunk(t, b) for t, b in shared]
        out += [_chunk(b"IDAT", p) for p in parts]
        out.append(_chunk(b"IEND", b""))
        return b"".join(out)

    frames_raw = [f for f in frames_raw if f[1]]
    if actl is None or not frames_raw:
        info.frames.append(APNGFrame(build(width, height, default_image), width, height, 0, 0, 0, DISPOSE_NONE, BLEND_SOURCE))
        return info

    info.is_animated = True
    info.num_plays = actl[1]
    for i, (fctl, parts) in enumerate(frames_raw):
        _, w, h, x, y, dnum, dden, dispose, blend = fctl
        if i == 0 and dispose == DISPOSE_PREVIOUS:
            dispose = DISPOSE_BACKGROUND
        info.frames.append(APNGFrame(build(w, h, parts), w, h, x, y, _delay_ms(dnum, dden), dispose, blend))
    return info
