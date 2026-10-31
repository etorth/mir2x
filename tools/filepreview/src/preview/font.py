"""Fonts (TrueType, OpenType, collections, WOFF): family names and sample text."""

import struct

from PySide6.QtCore import Qt
from PySide6.QtGui import QFont, QFontDatabase, QRawFont
from PySide6.QtWidgets import QGridLayout, QLabel, QScrollArea, QWidget

from .base import Previewer

# leading bytes -> kind; Qt can't load WOFF2, so it stays a hex dump
FONT_KINDS = {
    b"\x00\x01\x00\x00": "TrueType font",
    b"true": "TrueType font",
    b"OTTO": "OpenType font",
    b"ttcf": "Font collection",
    b"wOFF": "WOFF font",
}
FONT_EXTS = {".ttf", ".otf", ".ttc", ".otc", ".woff"}
SAMPLE_SIZES = (10, 14, 20, 28, 40, 56)
CHARSET_LINES = (
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "abcdefghijklmnopqrstuvwxyz",
    "0123456789 !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
)
PANGRAM = "The quick brown fox jumps over the lazy dog"
CJK_SAMPLE = "\u6c38\u548c\u4e5d\u5e74 \u5929\u5730\u4eba \u5c71\u6c34\u65e5\u6708"


class FontView(QScrollArea):
    """Sample text in the fonts of one font file, registered with Qt while shown."""

    def __init__(self):
        super().__init__()
        self.setWidgetResizable(True)
        self.fontIds = []
        self.clear()

    def clear(self):
        self.setWidget(QWidget())  # deletes the labels drawn in the old fonts
        for fontId in self.fontIds:
            QFontDatabase.removeApplicationFont(fontId)
        self.fontIds = []

    def showFont(self, data):
        """Register font file data and show its families; returns (families, style name)."""
        self.clear()
        fontId = QFontDatabase.addApplicationFontFromData(data)
        if fontId < 0:
            raise ValueError("Qt can't load this font")
        self.fontIds.append(fontId)
        families = QFontDatabase.applicationFontFamilies(fontId)
        # the style of the file's (first) font: QFont(family) alone could pick an installed
        # sibling, e.g. Arial Regular for arialbd.ttf
        raw = QRawFont(data, 12)
        style = raw.styleName() if raw.isValid() else ""
        if raw.isValid() and not style:  # e.g. arialbd.ttf: no style name, but weight 700
            probe = QFont()
            probe.setWeight(QFont.Weight(raw.weight()))
            probe.setStyle(raw.style())
            style = QFontDatabase.styleString(probe).replace("Normal", "Regular")
        cjk = raw.isValid() and all(raw.supportsCharacter(c) for c in CJK_SAMPLE if c != " ")

        page = QWidget()
        grid = QGridLayout(page)
        grid.setColumnStretch(1, 1)
        row = 0

        def add(text, font, sizeLabel=""):
            nonlocal row
            label = QLabel(text)
            label.setTextFormat(Qt.PlainText)
            label.setTextInteractionFlags(Qt.TextSelectableByMouse)
            label.setFont(font)
            grid.addWidget(QLabel(sizeLabel), row, 0, Qt.AlignRight | Qt.AlignVCenter)
            grid.addWidget(label, row, 1)
            row += 1

        for family in families:
            title = QFont(self.font())
            title.setBold(True)
            title.setPointSize(title.pointSize() + 2)
            add("%s %s" % (family, style), title)
            font = QFont(family)
            if raw.isValid():
                font.setWeight(QFont.Weight(raw.weight()))
                font.setStyle(raw.style())
            font.setPointSize(20)
            lines = CHARSET_LINES + ((CJK_SAMPLE,) if cjk else ())
            for line in lines:
                add(line, font)
            for size in SAMPLE_SIZES:
                font = QFont(font)
                font.setPointSize(size)
                add(PANGRAM + (" " + CJK_SAMPLE if cjk else ""), font, "%d pt" % size)
            grid.setRowMinimumHeight(row, 16)
            row += 1
        grid.setRowStretch(row, 1)
        self.setWidget(page)
        return families, style


class FontPreviewer(Previewer):
    name = "Font"

    def match(self, f):
        return f.head[:4] in FONT_KINDS and (
            "font" in f.mime or "opentype" in f.mime or "Font" in f.description or f.ext in FONT_EXTS)

    def show(self, f, panel):
        with open(f.path, "rb") as file:
            data = file.read()
        families, style = panel.view(FontView).showFont(data)
        fields = [FONT_KINDS[f.head[:4]]]
        if f.head[:4] == b"ttcf" and len(f.head) >= 12:
            fields.append("%d fonts" % struct.unpack(">I", f.head[8:12])[0])
        return fields + [", ".join(families)] + ([style] if style else [])
