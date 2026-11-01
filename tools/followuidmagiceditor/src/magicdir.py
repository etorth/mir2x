"""Magic APNG directory used by followuidmagiceditor.

A magic APNG is named by gfxres2apng.encode_magic_name(), its first 8 hex digits
are the sequence ID: gfxID << 4 | direction. Head and tail live in the file name.
Setting them only changes memory, save() renames the changed APNGs, which only
changes their head/tail digits.

head == tail means the gfx is non-rotate. Non-rotate is a per-gfxID editing mode,
derived from the names until it's set: on means all directions have head == tail.
"""

import os
import re
import sys

from PIL import Image
from PySide6.QtGui import QImage

# installed next to this file, or in the source tree
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "gfxres2apng", "src"))
from gfxres2apng import decode_magic_name, encode_magic_name  # noqa: E402

MAGIC_NAME_RE = re.compile(r"^[0-9A-F]{26}\.PNG$")


def seq_id(gfx_id, direction):
    return (gfx_id << 4) | direction


class MagicSeq:
    def __init__(self, path):
        self.path = path
        info = decode_magic_name(os.path.basename(path))
        self.file = info["file"]
        self.image = info["image"]
        self.direction = info["direction"]
        self.dx = info["dx"]
        self.dy = info["dy"]
        self.saved_head = self.head = info["head"]
        self.saved_tail = self.tail = info["tail"]
        self._frames = None

    @property
    def name(self):
        return os.path.basename(self.path)

    @property
    def gfx_id(self):
        return (self.file << 20) | self.image

    @property
    def dirty(self):
        return (self.head, self.tail) != (self.saved_head, self.saved_tail)

    def edited_name(self):
        return encode_magic_name((self.file, self.image, self.direction, self.head), self.dx, self.dy, self.tail)

    def frames(self):
        if self._frames is None:
            self._frames = []
            with Image.open(self.path) as img:
                for i in range(getattr(img, "n_frames", 1)):
                    img.seek(i)
                    rgba = img.convert("RGBA")
                    qimg = QImage(rgba.tobytes(), rgba.width, rgba.height, rgba.width * 4, QImage.Format.Format_RGBA8888)
                    self._frames.append(qimg.copy())
        return self._frames

    def frame(self, index):
        frames = self.frames()
        return frames[index % len(frames)] if frames else None


class MagicDir:
    def __init__(self, path):
        """path is the APNG dir, or a gfxres2apng output root which has texture/magic."""
        path = os.path.abspath(path)
        if os.path.isdir(os.path.join(path, "texture", "magic")):
            path = os.path.join(path, "texture", "magic")

        self.path = path
        self.seqs = {}
        self.reload()

    def reload(self):
        seqs = {}
        for name in sorted(os.listdir(self.path)):
            if not MAGIC_NAME_RE.match(name):
                continue

            key = int(name[:8], 16)
            if key in seqs:
                raise ValueError(f"duplicate magic sequence {key:08X}: {seqs[key].name} and {name}")
            seqs[key] = MagicSeq(os.path.join(self.path, name))

        if not seqs:
            raise ValueError(f"no magic APNG in {self.path}")
        self.seqs = seqs
        self.non_rotate = {}
        self.rotate_tails = {}  # gfxID -> {seq: tail}, tails before non-rotate was turned on

    def get(self, gfx_id, direction):
        return self.seqs.get(seq_id(gfx_id, direction))

    def set_points(self, seq, head=None, tail=None):
        """Edit seq's head/tail in memory, save() writes them."""
        if head is not None:
            seq.head = tuple(head)
        if tail is not None:
            seq.tail = tuple(tail)

    def gfx_seqs(self, gfx_id, dir_count):
        return [seq for seq in (self.get(gfx_id, d) for d in range(dir_count)) if seq]

    def is_non_rotate(self, gfx_id, dir_count):
        if gfx_id in self.non_rotate:
            return self.non_rotate[gfx_id]

        seqs = self.gfx_seqs(gfx_id, dir_count)
        return bool(seqs) and all(seq.head == seq.tail for seq in seqs)

    def set_non_rotate(self, gfx_id, dir_count, on):
        """Turning on sets every tail to its head, turning off restores the tails from before."""
        if on == self.is_non_rotate(gfx_id, dir_count):
            return

        self.non_rotate[gfx_id] = on
        seqs = self.gfx_seqs(gfx_id, dir_count)

        if on:
            self.rotate_tails[gfx_id] = {seq: seq.tail for seq in seqs}
            for seq in seqs:
                seq.tail = seq.head
        else:
            for seq, tail in self.rotate_tails.pop(gfx_id, {}).items():
                seq.tail = tail

    def dirty_seqs(self):
        return [seq for seq in self.seqs.values() if seq.dirty]

    def revert(self):
        for seq in self.dirty_seqs():
            seq.head = seq.saved_head
            seq.tail = seq.saved_tail
        self.non_rotate = {}
        self.rotate_tails = {}

    def save(self):
        """Rename every edited APNG, returns [(old name, new name)].

        Names are checked before any rename, a failed rename leaves the rest unsaved.
        """
        renames = []
        for seq in self.dirty_seqs():
            new_name = seq.edited_name()
            if new_name != seq.name and os.path.exists(os.path.join(self.path, new_name)):
                raise FileExistsError(os.path.join(self.path, new_name))
            renames.append((seq, new_name))

        done = []
        for seq, new_name in renames:
            old_name = seq.name
            if new_name != old_name:
                new_path = os.path.join(self.path, new_name)
                os.rename(seq.path, new_path)
                seq.path = new_path

            seq.saved_head = seq.head
            seq.saved_tail = seq.tail
            done.append((old_name, new_name))
        return done
