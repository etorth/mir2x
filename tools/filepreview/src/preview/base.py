"""Base class for previewers and the file description they work on."""

import os
from dataclasses import dataclass

from .filetype import detect, text_codec
from .options import TEXT_ENCODING

HEAD_BYTES = 8192
TEXT_PREVIEW_BYTES = 64 * 1024


@dataclass
class FileInfo:
    path: str
    name: str
    ext: str            # lower case, with the dot: ".png"
    isDir: bool
    size: int
    head: bytes         # first HEAD_BYTES bytes; empty for directories
    mime: str           # detected from the content by libmagic, e.g. "image/png", "text/x-c"
    charset: str        # "binary" for non-text data, else e.g. "us-ascii", "utf-8", "utf-16le"
    description: str    # libmagic's description, e.g. "PE32+ executable (GUI) x86-64"; may be ""

    @classmethod
    def load(cls, path):
        name = os.path.basename(path) or path
        if os.path.isdir(path):
            return cls(path, name, "", True, 0, b"", "inode/directory", "binary", "directory")
        size = os.path.getsize(path)
        with open(path, "rb") as f:
            head = f.read(HEAD_BYTES)
        mime, charset, description = detect(name, head)
        return cls(path, name, os.path.splitext(name)[1].lower(), False, size, head, mime, charset, description)

    def isText(self):
        return self.charset != "binary"

    def readText(self, limit=TEXT_PREVIEW_BYTES):
        """Return (text, truncated): the first limit bytes, decoded by the detected charset."""
        with open(self.path, "rb") as f:
            data = f.read(limit)
        text = data.decode(text_codec(self.charset, TEXT_ENCODING.value()), errors="replace")
        return text.removeprefix("\ufeff"), self.size > limit


class Previewer:
    """Shows one kind of file in the preview panel.

    To support a new file type, subclass this, implement match() and show(),
    and add an instance to PREVIEWERS in registry.py.
    """

    name = ""     # shown in Config > Preview Type
    options = ()  # options.Option list shown in Config > Preview Options; read them in show()

    def match(self, f):
        """Return True if this previewer handles FileInfo f, normally by its
        detected type (f.mime, f.isText()). Keep it cheap: it runs for every
        selected file."""
        raise NotImplementedError

    def show(self, f, panel):
        """Show f in a view from panel.view(SomeViewClass). Return the info
        fields shown after the file name, e.g. ["PNG", "32x32"]; the panel
        appends the file size. Raise an exception to show an error instead."""
        raise NotImplementedError


def truncated_note(truncated, limit=TEXT_PREVIEW_BYTES):
    return ["first %d KB shown" % (limit // 1024)] if truncated else []
