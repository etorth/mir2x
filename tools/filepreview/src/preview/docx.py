"""Word .docx: converted to HTML by mammoth and shown in a QTextBrowser (skipped if mammoth is missing).

mammoth keeps the document structure (headings, lists, tables, links, images),
not its exact page layout.
"""

import base64

from PySide6.QtCore import QUrl
from PySide6.QtGui import QDesktopServices, QImage, QTextDocument
from PySide6.QtWidgets import QTextBrowser

from .base import Previewer
from .options import Option
from .views import DARK_PAGE, recolor_links, set_colors

try:
    import mammoth
except ImportError:
    mammoth = None

DOCX_MIME = "application/vnd.openxmlformats-officedocument.wordprocessingml.document"
GENERIC_MIMES = {"application/zip", "application/octet-stream"}  # older libmagic
EXTERNAL_SCHEMES = {"http", "https", "mailto"}
DARK = Option("docx/dark", "Dark Page", False)
# mammoth maps Heading N to <hN>; Word's Title/Subtitle styles would otherwise be plain paragraphs
STYLE_MAP = "p[style-name='Title'] => h1:fresh\np[style-name='Subtitle'] => h2:fresh"


class DocxView(QTextBrowser):
    def __init__(self):
        super().__init__()
        self.setOpenLinks(False)  # don't navigate away from the previewed file
        self.anchorClicked.connect(self.openLink)

    def openLink(self, url):
        if url.scheme() in EXTERNAL_SCHEMES:
            QDesktopServices.openUrl(url)
        elif url.hasFragment():
            self.scrollToAnchor(url.fragment())

    def loadResource(self, kind, url):
        # mammoth embeds images as base64 data: URLs
        if kind == QTextDocument.ImageResource and url.scheme() == "data":
            _header, _sep, data = url.toString(QUrl.FullyEncoded).partition(",")
            return QImage.fromData(base64.b64decode(QUrl.fromPercentEncoding(data.encode())))
        return super().loadResource(kind, url)


class DocxPreviewer(Previewer):
    name = "Word"
    options = (DARK,)

    def match(self, f):
        return mammoth is not None and (f.mime == DOCX_MIME or (f.ext == ".docx" and f.mime in GENERIC_MIMES))

    def show(self, f, panel):
        if mammoth is None:
            raise ValueError("mammoth is not installed")
        with open(f.path, "rb") as file:
            html = mammoth.convert_to_html(file, style_map=STYLE_MAP).value
        # mammoth emits bare <table>; give Qt's renderer visible cell borders
        html = html.replace("<table>", '<table border="1" cellspacing="0" cellpadding="4">')
        view = panel.view(DocxView)
        set_colors(view, *(DARK_PAGE if DARK.value() else ()))
        view.setHtml(html)
        if DARK.value():
            recolor_links(view.document(), DARK_PAGE[2])
        return ["Word document"]
