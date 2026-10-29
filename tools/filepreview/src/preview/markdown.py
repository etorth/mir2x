"""Markdown rendered by Qt's own Markdown support (QTextDocument, GitHub dialect)."""

import os
import re

from PySide6.QtCore import QUrl
from PySide6.QtGui import QDesktopServices
from PySide6.QtWidgets import QTextBrowser

from .base import Previewer, truncated_note
from .options import Option, TEXT_ENCODING
from .views import DARK_PAGE, recolor_links, set_colors

# libmagic sees Markdown as plain text (or HTML when it embeds tags): use the extension
MARKDOWN_EXTS = {".md", ".markdown", ".mdown", ".mkd", ".mkdn"}
EXTERNAL_SCHEMES = {"http", "https", "mailto"}
DARK = Option("markdown/dark", "Dark Page", False)


class MarkdownView(QTextBrowser):
    """Read-only rendered Markdown; relative images load from the file's directory."""

    def __init__(self):
        super().__init__()
        self.headings = {}
        self.setOpenLinks(False)  # don't navigate away from the previewed file
        self.anchorClicked.connect(self.openLink)

    def setMarkdownFile(self, text, directory):
        self.setSearchPaths([directory])
        self.document().setBaseUrl(QUrl.fromLocalFile(directory + os.sep))
        self.setMarkdown(text)
        self.headings = heading_slugs(self.document())

    def openLink(self, url):
        if url.scheme() in EXTERNAL_SCHEMES:
            QDesktopServices.openUrl(url)
        elif url.hasFragment() and not url.path():
            block = self.headings.get(url.fragment().lower())
            if block is not None:
                top = self.document().documentLayout().blockBoundingRect(block).top()
                self.verticalScrollBar().setValue(int(top))


def heading_slugs(doc):
    """Map GitHub-style heading anchors to blocks; Qt's importer creates no anchors."""
    slugs, block = {}, doc.begin()
    while block.isValid():
        if block.blockFormat().headingLevel() > 0:
            slug = re.sub(r"[^\w\- ]", "", block.text().strip().lower()).replace(" ", "-")
            base, n = slug, 0
            while slug in slugs:
                n += 1
                slug = "%s-%d" % (base, n)
            slugs[slug] = block
        block = block.next()
    return slugs


class MarkdownPreviewer(Previewer):
    name = "Markdown"
    options = (DARK, TEXT_ENCODING)

    def match(self, f):
        return f.isText() and (f.mime == "text/markdown" or f.ext in MARKDOWN_EXTS)

    def show(self, f, panel):
        text, truncated = f.readText()
        view = panel.view(MarkdownView)
        set_colors(view, *(DARK_PAGE if DARK.value() else ()))
        view.setMarkdownFile(text, os.path.dirname(os.path.abspath(f.path)))
        if DARK.value():
            recolor_links(view.document(), DARK_PAGE[2])
        return ["Markdown"] + truncated_note(truncated)
