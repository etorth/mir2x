"""Plain-text code editor with line numbers and Python syntax highlighting."""

import builtins
import keyword
import re

from PySide6.QtCore import Qt
from PySide6.QtGui import QPainter, QColor, QPalette, QFont, QSyntaxHighlighter, QTextCharFormat
from PySide6.QtWidgets import QWidget, QPlainTextEdit


class PythonHighlighter(QSyntaxHighlighter):
    TOKEN_RE = re.compile(r"""
        (?P<comment>\#.*)
      | (?P<tstring>[rRbBuUfF]{0,2}(?:'''|\"\"\"))
      | (?P<string>[rRbBuUfF]{0,2}(?:"(?:[^"\\]|\\.)*"?|'(?:[^'\\]|\\.)*'?))
      | (?P<decorator>^\s*@[\w.]+)
      | (?P<number>\b(?:0[xXoObB][\da-fA-F_]+|\d[\d_]*(?:\.[\d_]*)?(?:[eE][+-]?\d+)?[jJ]?)\b)
      | (?P<word>[A-Za-z_]\w*)
    """, re.VERBOSE)
    # block state of a line that ends inside a triple-quoted string
    TRIPLE_STATES = {"'''": 1, '"""': 2}
    BUILTINS = frozenset(n for n in dir(builtins) if not n.startswith("_"))

    def __init__(self, document):
        super().__init__(document)

        def fmt(color, bold=False, italic=False):
            f = QTextCharFormat()
            f.setForeground(QColor(color))
            if bold:
                f.setFontWeight(QFont.Bold)
            f.setFontItalic(italic)
            return f

        self.formats = {
            "keyword": fmt("#0000c0", bold=True),
            "builtin": fmt("#7a3e9d"),
            "self": fmt("#9a4f00"),
            "defname": fmt("#00627a", bold=True),
            "string": fmt("#a31515"),
            "comment": fmt("#008000", italic=True),
            "number": fmt("#098658"),
            "decorator": fmt("#808000"),
        }

    def closeString(self, text, start, bodyStart, quote):
        """Highlight a triple-quoted string from start; return its end, or -1 if it continues on the next line."""
        end = bodyStart
        while (end := text.find(quote, end)) >= 0:
            slashes = end - len(text[:end].rstrip("\\"))
            if slashes % 2 == 0:  # an odd run of backslashes escapes the quote
                end += 3
                self.setFormat(start, end - start, self.formats["string"])
                return end
            end += 1
        self.setFormat(start, len(text) - start, self.formats["string"])
        self.setCurrentBlockState(self.TRIPLE_STATES[quote])
        return -1

    def highlightBlock(self, text):
        self.setCurrentBlockState(0)
        pos = 0
        for quote, state in self.TRIPLE_STATES.items():
            if self.previousBlockState() == state:
                pos = self.closeString(text, 0, 0, quote)
                if pos < 0:
                    return

        prevWord = None
        while (m := self.TOKEN_RE.search(text, pos)):
            kind, start, pos = m.lastgroup, m.start(), m.end()
            if kind == "tstring":
                pos = self.closeString(text, start, pos, m.group()[-3:])
                if pos < 0:
                    return
                prevWord = None
                continue
            if kind == "word":
                word = m.group()
                if prevWord in ("def", "class"):
                    kind = "defname"
                elif keyword.iskeyword(word):
                    kind = "keyword"
                elif word in ("self", "cls"):
                    kind = "self"
                elif word in self.BUILTINS:
                    kind = "builtin"
                else:
                    kind = None
                prevWord = word
            else:
                prevWord = None
            if kind:
                self.setFormat(start, pos - start, self.formats[kind])


class LineNumberArea(QWidget):
    def __init__(self, editor):
        super().__init__(editor)
        self.editor = editor

    def paintEvent(self, event):
        self.editor.paintLineNumbers(event)


class CodeEdit(QPlainTextEdit):
    """Plain text code editor with a line-number gutter; Tab inserts 4 spaces."""

    def __init__(self):
        super().__init__()
        self.lineNumbers = LineNumberArea(self)
        self.blockCountChanged.connect(self.updateGutterWidth)
        self.updateRequest.connect(self.updateGutter)
        self.cursorPositionChanged.connect(self.lineNumbers.update)  # bold current line
        self.updateGutterWidth()

    def gutterWidth(self):
        digits = max(2, len(str(self.blockCount())))
        return 12 + self.fontMetrics().horizontalAdvance("9") * digits

    def updateGutterWidth(self):
        self.setViewportMargins(self.gutterWidth(), 0, 0, 0)

    def updateGutter(self, rect, dy):
        if dy:
            self.lineNumbers.scroll(0, dy)
        else:
            self.lineNumbers.update(0, rect.y(), self.lineNumbers.width(), rect.height())

    def setFont(self, font):
        super().setFont(font)
        self.updateGutterWidth()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        r = self.contentsRect()
        self.lineNumbers.setGeometry(r.left(), r.top(), self.gutterWidth(), r.height())

    def paintLineNumbers(self, event):
        p = QPainter(self.lineNumbers)
        p.fillRect(event.rect(), self.palette().color(QPalette.Window))
        p.setPen(self.palette().color(QPalette.PlaceholderText))
        current = self.textCursor().blockNumber()
        block = self.firstVisibleBlock()
        top = round(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())
        width = self.lineNumbers.width() - 6
        height = self.fontMetrics().height()
        while block.isValid() and top <= event.rect().bottom():
            bottom = top + round(self.blockBoundingRect(block).height())
            if block.isVisible() and bottom >= event.rect().top():
                font = p.font()
                font.setBold(block.blockNumber() == current)
                p.setFont(font)
                p.drawText(0, top, width, height, Qt.AlignRight, str(block.blockNumber() + 1))
            block = block.next()
            top = bottom

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Tab and event.modifiers() == Qt.NoModifier:
            self.insertPlainText("    ")
            return
        super().keyPressEvent(event)
