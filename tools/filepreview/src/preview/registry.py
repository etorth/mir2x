"""The ordered list of previewers; the first whose match() accepts a file shows it.

Files are matched by their content type (FileInfo.mime, FileInfo.charset),
detected by libmagic in filetype.py, not by their extension.

To add a file type: write a Previewer subclass (see base.py; image.py, pdf.py
and code.py are examples), import it here and insert an instance before the
more generic previewers. Explicit imports also keep PyInstaller bundling it.
"""

from .archive import ArchivePreviewer
from .basic import DirectoryPreviewer, TextPreviewer, HexPreviewer
from .code import CodePreviewer
from .docx import DocxPreviewer
from .font import FontPreviewer
from .image import PngPreviewer, ImagePreviewer
from .markdown import MarkdownPreviewer
from .media import MediaPreviewer
from .pdf import PdfPreviewer
from .table import CsvPreviewer

PREVIEWERS = [
    DirectoryPreviewer(),
    PdfPreviewer(),
    DocxPreviewer(),
    ArchivePreviewer(),  # after DocxPreviewer: a .docx is a ZIP file too
    PngPreviewer(),
    ImagePreviewer(),
    FontPreviewer(),
    MediaPreviewer(),
    MarkdownPreviewer(),  # before CodePreviewer, which would highlight it as source
    CsvPreviewer(),
    CodePreviewer(),
    TextPreviewer(),
    HexPreviewer(),  # matches everything, keep last
]
