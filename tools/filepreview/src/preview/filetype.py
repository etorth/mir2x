"""File type detection: libmagic via python-magic, by content rather than name.

On Windows install python-magic-bin (it ships libmagic.dll and magic.mgc);
elsewhere python-magic plus the system libmagic. Without either, the type is
guessed from the file extension.
"""

import locale
import mimetypes

try:
    import magic
except ImportError:  # also raised when the libmagic library itself is missing
    magic = None

UNKNOWN_MIME = "application/octet-stream"

# libmagic names that differ from the registered ones Qt and Pygments use
MIME_ALIASES = {
    "image/x-ms-bmp": "image/bmp",
    "image/x-icon": "image/vnd.microsoft.icon",
    "image/svg": "image/svg+xml",
    "image/x-xpmi": "image/x-xpixmap",
    "image/x-portable-greymap": "image/x-portable-graymap",
}

_magic = None


def _detectors():
    global _magic
    if _magic is None:
        _magic = (magic.Magic(mime=True, mime_encoding=True), magic.Magic())
    return _magic


def detect(name, head):
    """Return (mime, charset, description) for a file name and its leading bytes.
    charset is "binary" for non-text data; description is libmagic's text
    ("PE32+ executable ...") or "" when guessed from the extension."""
    if not head:
        return "application/x-empty", "us-ascii", "empty"
    try:
        typeMagic, descMagic = _detectors()
        mime, _, charset = typeMagic.from_buffer(head).partition("; charset=")
        description = descMagic.from_buffer(head)
    except Exception:  # no python-magic/libmagic, or libmagic failed on this data
        mime = mimetypes.guess_type(name)[0] or UNKNOWN_MIME
        charset = "binary" if b"\0" in head else "utf-8"
        description = ""
    return MIME_ALIASES.get(mime, mime), charset or "binary", description


def text_codec(charset):
    """Python codec to decode text libmagic reported with this charset."""
    charset = charset.lower()
    if charset.startswith(("utf-16", "utf-32")):
        return charset
    if charset in ("iso-8859-1", "unknown-8bit"):
        # 8-bit but not UTF-8: most likely the Windows ANSI code page (e.g. GBK)
        return locale.getencoding()
    return "utf-8"
