"""Fallbacks: directories, plain text and a hex dump for anything else."""

from .base import Previewer, truncated_note
from .views import ImagePane, TextView

HEX_DUMP_BYTES = 4096


class DirectoryPreviewer(Previewer):
    def match(self, f):
        return f.isDir

    def show(self, f, panel):
        panel.view(ImagePane)
        return ["directory"]


class TextPreviewer(Previewer):
    def match(self, f):
        return f.isText()

    def show(self, f, panel):
        text, truncated = f.readText()
        panel.view(TextView).setPlainText(text)
        return [f.description or "text"] + truncated_note(truncated)


class HexPreviewer(Previewer):
    def match(self, f):
        return True

    def show(self, f, panel):
        data = f.head[:HEX_DUMP_BYTES]
        lines = []
        for off in range(0, len(data), 16):
            chunk = data[off:off + 16]
            hexs = " ".join("%02x" % b for b in chunk)
            asc = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
            lines.append("%08x  %-47s  %s" % (off, hexs, asc))
        panel.view(TextView).setPlainText("\n".join(lines))
        return [f.description or f.mime, "hex dump"] + truncated_note(f.size > HEX_DUMP_BYTES, HEX_DUMP_BYTES)
