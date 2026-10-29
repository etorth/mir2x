#!/usr/bin/env python3
"""Simple file previewer: preview on the left, sorted file list on the right.

Usage: python3 filepreview.py [directory]

Keys: Ctrl+O open directory, Up/Down move selection, Space play/pause animation,
      Left/Right step animation frames, F toggle fit-to-window,
      Enter or double-click enter a directory (View > Show Directory).

Transform > Enable runs a user-defined transform(path: pathlib.Path)
on every entry before the wildcard/regex filter; it can rename or hide entries.
It is edited via Transform > Edit... and saved between runs.
"""

import builtins
import fnmatch
import keyword
import os
import pathlib
import re
import reprlib
import sys
import traceback
from dataclasses import dataclass

from PySide6.QtCore import Qt, QTimer, QRect, QEvent, QMimeData, QUrl, QSettings
from PySide6.QtGui import (
    QImage, QPainter, QPixmap, QColor, QFontDatabase, QActionGroup, QKeySequence, QIcon, QPalette, QTextCursor,
    QFont, QSyntaxHighlighter, QTextCharFormat,
)
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QSplitter, QListWidget, QListWidgetItem, QMenu,
    QLabel, QCheckBox, QLineEdit, QPushButton, QHBoxLayout, QVBoxLayout,
    QFileDialog, QPlainTextEdit, QStackedWidget, QSpinBox,
    QDialog, QDialogButtonBox, QStyledItemDelegate, QStyle, QMenuBar, QMessageBox,
)

import apng

PNG_EXTS = {".png", ".apng"}
IMAGE_EXTS = {".jpg", ".jpeg", ".bmp", ".gif", ".webp", ".tga", ".ico"}
TEXT_PREVIEW_BYTES = 64 * 1024

PARENT_DIR = ".."
ROLE_NAME = Qt.UserRole        # real entry name; display text adds "/" to dirs
ROLE_IS_DIR = Qt.UserRole + 1
ROLE_TRANSFORM_ERROR = Qt.UserRole + 2

TRANSFORM_ERROR_BG = QColor(255, 210, 220)
TRANSFORM_ERROR_MARK = QColor(220, 40, 90)

SETTING_TRANSFORM_CODE = "transform/code"
SETTING_TRANSFORM_FILE = "transform/file"


def resource_path(name):
    """Locate a bundled data file, both from source and in a PyInstaller build."""
    base = getattr(sys, "_MEIPASS", os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base, name)


# ---------------------------------------------------------------------------
# sort methods
# ---------------------------------------------------------------------------

def natural_key(s):
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", s)]


def hex_name_key(name):
    stem = os.path.splitext(name)[0]
    if re.fullmatch(r"[0-9a-fA-F]+", stem):
        return (0, int(stem, 16), name.lower(), name)
    return (1, 0, name.lower(), name)


@dataclass
class Entry:
    name: str           # real name on disk
    size: int
    mtime: float
    isDir: bool
    text: str = None    # displayed name; the filename transform may change it
    error: str = None   # set when the filename transform misbehaved for this entry

    def __post_init__(self):
        if self.text is None:
            self.text = self.name


# name -> key(entry); name-based keys use the displayed text
SORT_METHODS = {
    "Name (natural)": lambda e: natural_key(e.text),
    "Name (hex)":     lambda e: hex_name_key(e.text),
    "Name":           lambda e: e.text.lower(),
    "Extension":      lambda e: (os.path.splitext(e.text)[1].lower(), natural_key(e.text)),
    "Size":           lambda e: (e.size, natural_key(e.text)),
    "Modified time":  lambda e: (e.mtime, natural_key(e.text)),
}


def sort_entries(entries, sort_name, descending, mix_dirs=False, errors_last=True):
    """Sort in place. Unless mix_dirs, directories go before files; with
    errors_last, entries the filename transform failed on go after all others."""
    entries.sort(key=SORT_METHODS[sort_name], reverse=descending)
    # stable sort keeps the order above inside each group
    entries.sort(key=lambda e: (errors_last and e.error is not None, not mix_dirs and not e.isDir))
    return entries


FILTER_WILDCARD = "Wildcard"
FILTER_REGEX = "Regex"

TRANSFORM_TEMPLATE = '''\
import pathlib


def transform(path: pathlib.Path):
    # Called for every file and directory in the list (never for "..").
    #   True           -> show the original name
    #   False          -> hide the entry
    #   non-empty str  -> show this text instead of the name
    # Anything else, or an exception, shows the original name in pink.
    return True
'''

TRANSFORM_HELP = '''\
<p>The code must define <code>transform(path)</code>. It is called with a <code>pathlib.Path</code>
for every file and directory in the list (never for ".."), and its return value decides
how the entry is shown:</p>
<table cellspacing="0" cellpadding="3" style="margin-left: 16px">
<tr><td><code>True</code></td><td>show the original name</td></tr>
<tr><td><code>False</code></td><td>hide the entry</td></tr>
<tr><td>non-empty <code>str</code></td><td>show this text instead of the name</td></tr>
<tr><td>anything else, or an exception&nbsp;&nbsp;</td>
    <td>show the original name in pink; hover over it to see why</td></tr>
</table>
<p>The wildcard/regex filter then matches the shown text.</p>
<p><b>Apply</b> checks the code and turns on <b>Transform &gt; Enable</b>;
<b>OK</b> also closes this window. <b>Save &amp; Apply</b> also writes the code to the
loaded file. The applied code is remembered between runs.</p>
<p>Sorting by name uses the shown text; <b>Sort &gt; Transform Errors Last</b> puts pink
entries at the end.</p>
<p>The packaged filepreview.exe can only import the Python standard library and PySide6.
A transform that never returns freezes the app.</p>'''

TRANSFORM_FILENAME = "<filename transform>"


def compile_transform(code):
    """Run code and return the function it defines as `transform`; raises on error."""
    namespace = {"__name__": "filename_transform"}
    exec(compile(code, TRANSFORM_FILENAME, "exec"), namespace)
    func = namespace.get("transform")
    if not callable(func):
        raise ValueError("the code must define a function named 'transform'")
    return func


def format_error(exc):
    """One-line description; includes the line number inside the transform code."""
    text = "%s: %s" % (type(exc).__name__, exc)
    frames = [fr for fr in traceback.extract_tb(exc.__traceback__) if fr.filename == TRANSFORM_FILENAME]
    if frames:
        text += " (line %d)" % frames[-1].lineno
    return text


def apply_transform(entries, directory, func):
    """Return the entries func keeps, with text/error updated."""
    kept = []
    for e in entries:
        try:
            result = func(pathlib.Path(directory, e.name))
        except Exception as exc:
            e.error = "transform raised " + format_error(exc)
        else:
            if result is False:
                continue
            if isinstance(result, str) and result:
                e.text = result
            elif result is not True:
                e.error = "transform returned %s; expected True, False or a non-empty str" % reprlib.repr(result)
        kept.append(e)
    return kept


def compile_filter(name_filter, mode):
    """Return a case-insensitive compiled pattern, or None for no filtering.

    Wildcard: * and ? glob syntax matched against the whole name; text
    without wildcard characters matches anywhere in the name.
    Regex: re.search semantics (use ^ and $ to anchor). Raises re.error.
    """
    if not name_filter:
        return None
    if mode == FILTER_REGEX:
        return re.compile(name_filter, re.IGNORECASE)
    if not any(c in name_filter for c in "*?["):
        name_filter = "*%s*" % name_filter
    return re.compile(fnmatch.translate(name_filter), re.IGNORECASE)


def scan_dir(path, show_dirs=False):
    """Return unsorted [Entry] for the files (and directories) in path."""
    entries = []
    try:
        for e in os.scandir(path):
            if e.is_dir():
                if show_dirs:
                    entries.append(Entry(e.name, 0, e.stat().st_mtime, True))
            elif e.is_file():
                st = e.stat()
                entries.append(Entry(e.name, st.st_size, st.st_mtime, False))
    except OSError:
        return []
    return entries


def filter_entries(entries, name_filter, mode):
    """Wildcard/regex filtering of the shown text; applies to files only so
    directories stay navigable."""
    pattern = compile_filter(name_filter, mode)
    if pattern is None:
        return entries
    return [e for e in entries if e.isDir or pattern.search(e.text)]


def human_size(n):
    if n < 1024:
        return "%d B" % n
    for unit in ("KB", "MB", "GB"):
        n /= 1024.0
        if n < 1024 or unit == "GB":
            return "%.1f %s" % (n, unit)


# ---------------------------------------------------------------------------
# APNG compositing
# ---------------------------------------------------------------------------

def load_png_frames(path):
    """Return (frames, info): frames is [(QImage, delay_ms)], fully composited."""
    with open(path, "rb") as f:
        data = f.read()
    info = apng.parse_png(data)

    if not info.is_animated:
        img = QImage.fromData(data, "PNG")
        if img.isNull():
            raise apng.PNGError("failed to decode PNG")
        return [(img, 0)], info

    canvas = QImage(info.width, info.height, QImage.Format_ARGB32_Premultiplied)
    canvas.fill(Qt.transparent)
    frames = []
    for fr in info.frames:
        img = QImage.fromData(fr.png, "PNG")
        if img.isNull():
            raise apng.PNGError("failed to decode APNG frame")
        saved = canvas.copy() if fr.dispose_op == apng.DISPOSE_PREVIOUS else None

        p = QPainter(canvas)
        p.setCompositionMode(QPainter.CompositionMode_SourceOver if fr.blend_op == apng.BLEND_OVER
                             else QPainter.CompositionMode_Source)
        p.drawImage(fr.x, fr.y, img)
        p.end()

        frames.append((canvas.copy(), fr.delay_ms if fr.delay_ms > 0 else 100))

        if fr.dispose_op == apng.DISPOSE_BACKGROUND:
            p = QPainter(canvas)
            p.setCompositionMode(QPainter.CompositionMode_Source)
            p.fillRect(fr.x, fr.y, fr.width, fr.height, Qt.transparent)
            p.end()
        elif saved is not None:
            canvas = saved
    return frames, info


# ---------------------------------------------------------------------------
# widgets
# ---------------------------------------------------------------------------

ZOOM_OUT_LEVELS = [16, 8, 4, 2]
ZOOM_IN_MAX = 16


class ZoomSpinBox(QSpinBox):
    """Steps through 1/16x .. 1/2x, 1x .. 16x; value() is an index, zoom() the factor."""

    LEVELS = [(1.0 / d, "1/%dx" % d) for d in ZOOM_OUT_LEVELS] + \
             [(float(m), "%dx" % m) for m in range(1, ZOOM_IN_MAX + 1)]

    def __init__(self):
        super().__init__()
        self.setRange(0, len(self.LEVELS) - 1)
        self.setValue(len(ZOOM_OUT_LEVELS))  # 1x
        self.lineEdit().setReadOnly(True)  # step with arrows/wheel only

    def zoom(self):
        return self.LEVELS[self.value()][0]

    def textFromValue(self, value):
        return self.LEVELS[value][1]

    def valueFromText(self, text):
        labels = [label for _, label in self.LEVELS]
        return labels.index(text) if text in labels else self.value()

class ImageView(QWidget):
    """Draws an image centered on a checkerboard (or clear background);
    fit-to-window or fixed zoom."""

    def __init__(self):
        super().__init__()
        self.setMinimumSize(200, 200)
        self.image = None
        self.fit = True
        self.zoom = 1
        self.clearBackground = False
        self.drawBorder = False
        self.checker = QPixmap(16, 16)
        self.checker.fill(QColor(190, 190, 190))
        p = QPainter(self.checker)
        p.fillRect(0, 0, 8, 8, QColor(235, 235, 235))
        p.fillRect(8, 8, 8, 8, QColor(235, 235, 235))
        p.end()

    def setImage(self, image):
        self.image = image
        self.update()

    def paintEvent(self, event):
        p = QPainter(self)
        p.fillRect(self.rect(), QColor(60, 60, 60))
        if self.image is None or self.image.isNull():
            return

        iw, ih = self.image.width(), self.image.height()
        if self.fit:
            scale = min(self.width() / iw, self.height() / ih)
            if scale >= 1:
                scale = int(scale)  # integer upscale keeps pixel art crisp
        else:
            scale = self.zoom
        w, h = max(1, int(iw * scale)), max(1, int(ih * scale))
        rect = QRect((self.width() - w) // 2, (self.height() - h) // 2, w, h)

        if not self.clearBackground:
            p.drawTiledPixmap(rect, self.checker)
        p.setRenderHint(QPainter.SmoothPixmapTransform, scale < 1)
        p.drawImage(rect, self.image)
        if self.drawBorder:
            # 1px box just outside the image; filled strips instead of drawRect
            # because a pen stroke lands off by half a pixel on high-DPI screens
            color = QColor(255, 0, 255)
            x, y, w, h = rect.x(), rect.y(), rect.width(), rect.height()
            p.fillRect(x - 1, y - 1, w + 2, 1, color)
            p.fillRect(x - 1, y + h, w + 2, 1, color)
            p.fillRect(x - 1, y, 1, h, color)
            p.fillRect(x + w, y, 1, h, color)


class PreviewPanel(QWidget):
    def __init__(self):
        super().__init__()
        self.frames = []
        self.index = 0
        self.timer = QTimer(self)
        self.timer.setSingleShot(True)
        self.timer.timeout.connect(self.nextFrame)

        self.imageView = ImageView()
        self.textView = QPlainTextEdit()
        self.textView.setReadOnly(True)
        self.textView.setLineWrapMode(QPlainTextEdit.NoWrap)
        self.textView.setFont(QFontDatabase.systemFont(QFontDatabase.FixedFont))
        self.stack = QStackedWidget()
        self.stack.addWidget(self.imageView)
        self.stack.addWidget(self.textView)

        self.info = QLabel()
        self.info.setTextInteractionFlags(Qt.TextSelectableByMouse)

        self.playButton = QPushButton("Pause")
        self.playButton.clicked.connect(self.togglePlay)
        self.prevButton = QPushButton("<")
        self.prevButton.clicked.connect(lambda: self.stepFrame(-1))
        self.nextButton = QPushButton(">")
        self.nextButton.clicked.connect(lambda: self.stepFrame(1))
        self.frameLabel = QLabel()

        self.fitCheck = QCheckBox("Fit")
        self.fitCheck.setChecked(True)
        self.fitCheck.toggled.connect(self.updateZoom)
        self.zoomSpin = ZoomSpinBox()
        self.zoomSpin.setEnabled(False)
        self.zoomSpin.valueChanged.connect(self.updateZoom)

        controls = QHBoxLayout()
        for w in (self.prevButton, self.playButton, self.nextButton, self.frameLabel):
            controls.addWidget(w)
        controls.addStretch()
        controls.addWidget(self.fitCheck)
        controls.addWidget(self.zoomSpin)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.stack, 1)
        layout.addLayout(controls)
        layout.addWidget(self.info)
        self.showAnimControls(False)

    def updateZoom(self):
        self.imageView.fit = self.fitCheck.isChecked()
        self.zoomSpin.setEnabled(not self.imageView.fit)
        self.imageView.zoom = self.zoomSpin.zoom()
        self.imageView.update()

    def showAnimControls(self, show):
        for w in (self.prevButton, self.playButton, self.nextButton, self.frameLabel):
            w.setVisible(show)

    def clear(self, message=""):
        self.timer.stop()
        self.frames = []
        self.imageView.setImage(None)
        self.textView.clear()
        self.stack.setCurrentWidget(self.imageView)
        self.showAnimControls(False)
        self.info.setText(message)

    def showFile(self, path):
        self.clear()
        if os.path.isdir(path):
            self.info.setText("%s | directory" % path)
            return
        ext = os.path.splitext(path)[1].lower()
        try:
            size = os.path.getsize(path)
            if ext in PNG_EXTS or self.isPNG(path):
                self.showPNG(path, size)
            elif ext in IMAGE_EXTS:
                self.showImage(path, size)
            else:
                self.showText(path, size)
        except (OSError, apng.PNGError) as e:
            self.clear("%s\nError: %s" % (path, e))

    @staticmethod
    def isPNG(path):
        with open(path, "rb") as f:
            return f.read(8) == apng.PNG_SIGNATURE

    def showPNG(self, path, size):
        self.frames, info = load_png_frames(path)
        kind = "APNG" if info.is_animated else "PNG"
        text = "%s | %s | %dx%d | %s" % (os.path.basename(path), kind, info.width, info.height, human_size(size))
        if info.is_animated:
            total = sum(d for _, d in self.frames)
            plays = "infinite" if info.num_plays == 0 else str(info.num_plays)
            text += " | %d frames | %d ms | loops: %s" % (len(self.frames), total, plays)
        self.info.setText(text)
        self.setFrame(0)
        if len(self.frames) > 1:
            self.showAnimControls(True)
            self.playButton.setText("Pause")
            self.timer.start(self.frames[0][1])

    def showImage(self, path, size):
        img = QImage(path)
        if img.isNull():
            raise apng.PNGError("unsupported or corrupted image")
        self.frames = [(img, 0)]
        self.setFrame(0)
        self.info.setText("%s | %dx%d | %s" % (os.path.basename(path), img.width(), img.height(), human_size(size)))

    def showText(self, path, size):
        with open(path, "rb") as f:
            data = f.read(TEXT_PREVIEW_BYTES)
        if b"\0" in data:
            lines = []
            for off in range(0, min(len(data), 4096), 16):
                chunk = data[off:off + 16]
                hexs = " ".join("%02x" % b for b in chunk)
                asc = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
                lines.append("%08x  %-47s  %s" % (off, hexs, asc))
            self.textView.setPlainText("\n".join(lines))
            kind = "binary (hex dump)"
        else:
            self.textView.setPlainText(data.decode("utf-8", errors="replace"))
            kind = "text"
        self.stack.setCurrentWidget(self.textView)
        self.info.setText("%s | %s | %s" % (os.path.basename(path), kind, human_size(size)))

    def setFrame(self, index):
        if not self.frames:
            return
        self.index = index % len(self.frames)
        self.imageView.setImage(self.frames[self.index][0])
        if len(self.frames) > 1:
            self.frameLabel.setText("%d / %d (%d ms)" % (self.index + 1, len(self.frames), self.frames[self.index][1]))

    def nextFrame(self):
        self.setFrame(self.index + 1)
        self.timer.start(self.frames[self.index][1])

    def isAnimated(self):
        return len(self.frames) > 1

    def togglePlay(self):
        if not self.isAnimated():
            return
        if self.timer.isActive():
            self.timer.stop()
            self.playButton.setText("Play")
        else:
            self.timer.start(self.frames[self.index][1])
            self.playButton.setText("Pause")

    def stepFrame(self, delta):
        if not self.isAnimated():
            return
        self.timer.stop()
        self.playButton.setText("Play")
        self.setFrame(self.index + delta)


class EntryDelegate(QStyledItemDelegate):
    """Keeps transform-error entries recognizable while selected: the selection
    highlight hides the pink background, so draw a marker bar on the left."""

    def paint(self, painter, option, index):
        super().paint(painter, option, index)
        if index.data(ROLE_TRANSFORM_ERROR) and option.state & QStyle.State_Selected:
            r = option.rect
            painter.fillRect(r.x(), r.y(), 4, r.height(), TRANSFORM_ERROR_MARK)


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


class TransformDialog(QDialog):
    def __init__(self, parent, code, filePath, onApply):
        """onApply(code, filePath) installs code; it is only called once code compiles."""
        super().__init__(parent)
        self.onApply = onApply
        self.filePath = filePath
        self.resize(700, 450)

        self.editor = CodeEdit()
        self.editor.setFont(QFontDatabase.systemFont(QFontDatabase.FixedFont))
        self.editor.setLineWrapMode(QPlainTextEdit.NoWrap)
        self.editor.setPlainText(code)
        self.highlighter = PythonHighlighter(self.editor.document())
        self.editor.document().modificationChanged.connect(self.setWindowModified)
        self.errorLabel = QLabel()
        self.errorLabel.setStyleSheet("color: #c00000;")
        self.errorLabel.setWordWrap(True)
        self.errorLabel.setTextInteractionFlags(Qt.TextSelectableByMouse)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Apply | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        buttons.button(QDialogButtonBox.Apply).clicked.connect(self.apply)
        buttons.addButton("&Save && Apply", QDialogButtonBox.ApplyRole).clicked.connect(self.saveAndApply)

        layout = QVBoxLayout(self)
        layout.setMenuBar(self.createMenuBar())
        layout.addWidget(self.editor, 1)
        layout.addWidget(self.errorLabel)
        layout.addWidget(buttons)
        self.updateTitle()

    def createMenuBar(self):
        bar = QMenuBar(self)
        ed = self.editor

        openMenu = bar.addMenu("&Open")
        openMenu.addAction("&Load...", QKeySequence.Open, self.load)
        openMenu.addAction("&Save && Apply", QKeySequence.Save, self.saveAndApply)
        openMenu.addAction("Save &As && Apply...", QKeySequence.SaveAs, lambda: self.saveAndApply(askPath=True))
        openMenu.addSeparator()
        openMenu.addAction("&Restore Defaults", lambda: self.replaceText(TRANSFORM_TEMPLATE))

        editMenu = bar.addMenu("&Edit")
        undo = editMenu.addAction("&Undo", QKeySequence.Undo, ed.undo)
        redo = editMenu.addAction("&Redo", QKeySequence.Redo, ed.redo)
        editMenu.addSeparator()
        cut = editMenu.addAction("Cu&t", QKeySequence.Cut, ed.cut)
        copy = editMenu.addAction("&Copy", QKeySequence.Copy, ed.copy)
        editMenu.addAction("&Paste", QKeySequence.Paste, ed.paste)
        editMenu.addSeparator()
        editMenu.addAction("Select &All", QKeySequence.SelectAll, ed.selectAll)
        for act, signal in ((undo, ed.undoAvailable), (redo, ed.redoAvailable),
                            (cut, ed.copyAvailable), (copy, ed.copyAvailable)):
            act.setEnabled(False)
            signal.connect(act.setEnabled)

        helpMenu = bar.addMenu("&Help")
        helpMenu.addAction("Filename &Transform Help", QKeySequence.HelpContents, self.showHelp)
        return bar

    def showHelp(self):
        box = QMessageBox(QMessageBox.NoIcon, "Filename Transform Help", TRANSFORM_HELP, parent=self)
        box.setTextFormat(Qt.RichText)
        box.exec()

    def updateTitle(self):
        self.setWindowTitle("Filename Transform" + (f" - {self.filePath}" if self.filePath else "") + "[*]")

    def replaceText(self, text):
        # edit through a cursor instead of setPlainText() so Ctrl+Z can undo it
        cursor = self.editor.textCursor()
        cursor.select(QTextCursor.Document)
        cursor.insertText(text)
        cursor.movePosition(QTextCursor.Start)
        self.editor.setTextCursor(cursor)

    def showError(self, text):
        self.errorLabel.setText(text)

    def load(self):
        path, _ = QFileDialog.getOpenFileName(self, "Load Filename Transform", self.filePath,
                                              "Python files (*.py);;All files (*)")
        if not path:
            return
        try:
            with open(path, encoding="utf-8") as f:
                text = f.read()
        except (OSError, UnicodeDecodeError) as exc:
            self.showError(f"cannot load {path}: {exc}")
            return
        self.replaceText(text)
        self.editor.document().setModified(False)
        self.filePath = path
        self.updateTitle()
        self.errorLabel.clear()

    def apply(self):
        code = self.editor.toPlainText()
        try:
            compile_transform(code)
        except Exception as exc:
            # full form keeps the caret under a syntax error
            self.showError("".join(traceback.format_exception_only(type(exc), exc)).strip())
            return False
        self.errorLabel.clear()
        self.onApply(code, self.filePath)
        return True

    def saveAndApply(self, askPath=False):
        path = self.filePath
        if askPath or not path:
            path, _ = QFileDialog.getSaveFileName(self, "Save Filename Transform", path or "transform.py",
                                                  "Python files (*.py);;All files (*)")
            if not path:
                return
        try:
            compile_transform(self.editor.toPlainText())
        except Exception:
            return self.apply()  # shows the error; nothing is saved
        try:
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write(self.editor.toPlainText())
        except OSError as exc:
            self.showError(f"cannot save {path}: {exc}")
            return
        self.editor.document().setModified(False)
        self.filePath = path
        self.updateTitle()
        self.apply()

    def accept(self):
        if self.apply():
            super().accept()


class FileListPanel(QWidget):
    def __init__(self, onSelect, onStatus):
        super().__init__()
        self.onSelect = onSelect
        self.onStatus = onStatus
        self.directory = ""
        self.sortName = next(iter(SORT_METHODS))
        self.descending = False
        self.filterMode = FILTER_WILDCARD
        self.showDirs = False
        self.mixDirs = False
        self.errorsLast = True
        self.selectName = None  # entry to select on the next refresh
        self.transformEnabled = False
        self.transformCode = TRANSFORM_TEMPLATE
        self.transformFunc = compile_transform(TRANSFORM_TEMPLATE)
        self.transformError = None

        self.filterEdit = QLineEdit()
        self.filterEdit.textChanged.connect(self.refresh)
        self.updateFilterHint()

        self.list = QListWidget()
        self.list.setItemDelegate(EntryDelegate(self.list))
        self.list.setFont(QFontDatabase.systemFont(QFontDatabase.FixedFont))
        self.list.currentItemChanged.connect(self.itemChanged)
        self.list.itemActivated.connect(self.itemActivated)  # double-click or Enter
        self.list.setContextMenuPolicy(Qt.CustomContextMenu)
        self.list.customContextMenuRequested.connect(self.showContextMenu)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.filterEdit)
        layout.addWidget(self.list, 1)

    def setDirectory(self, path, select=None):
        path = os.path.abspath(os.path.expanduser(path))
        if not os.path.isdir(path):
            return
        self.directory = path
        self.selectName = select
        self.refresh()

    def setShowDirs(self, show):
        self.showDirs = show
        self.refresh()

    def setSort(self, name):
        self.sortName = name
        self.refresh()

    def setDescending(self, descending):
        self.descending = descending
        self.refresh()

    def setMixDirs(self, mix):
        self.mixDirs = mix
        self.refresh()

    def setErrorsLast(self, last):
        self.errorsLast = last
        self.refresh()

    def setTransformCode(self, code):
        """Install filename transform code; returns an error message or None.
        Broken code is kept (so it can be edited) and transforms nothing."""
        self.transformCode = code
        try:
            self.transformFunc, self.transformError = compile_transform(code), None
        except Exception as exc:
            self.transformFunc, self.transformError = None, "Filename transform: " + format_error(exc)
        if self.transformEnabled:
            self.refresh()
        return self.transformError

    def setTransformEnabled(self, enabled):
        self.transformEnabled = enabled
        self.refresh()

    def setFilterMode(self, mode):
        self.filterMode = mode
        self.updateFilterHint()
        self.refresh()

    def updateFilterHint(self):
        if self.filterMode == FILTER_REGEX:
            self.filterEdit.setPlaceholderText("regex filter, e.g. ^[0-9a-f]+\\.png$")
        else:
            self.filterEdit.setPlaceholderText("wildcard filter, e.g. *.png")

    def refresh(self):
        if self.selectName is not None:
            current, self.selectName = self.selectName, None
        else:
            current = self.list.currentItem().data(ROLE_NAME) if self.list.currentItem() else None
        error = None
        entries = scan_dir(self.directory, self.showDirs)
        if self.transformEnabled:
            if self.transformFunc is None:
                error = self.transformError
            else:
                entries = apply_transform(entries, self.directory, self.transformFunc)
        try:
            entries = filter_entries(entries, self.filterEdit.text(), self.filterMode)
        except re.error as exc:
            entries = []
            error = "Invalid regex: %s" % exc
        sort_entries(entries, self.sortName, self.descending, self.mixDirs, self.errorsLast)

        self.list.blockSignals(True)
        self.list.clear()
        if self.showDirs and os.path.dirname(self.directory) != self.directory:
            item = QListWidgetItem("..")
            item.setData(ROLE_NAME, PARENT_DIR)
            item.setData(ROLE_IS_DIR, True)
            item.setToolTip("parent directory")
            self.list.addItem(item)
        # default to the first real entry rather than ".."
        row = 1 if self.list.count() and entries and current != PARENT_DIR else 0
        for e in entries:
            item = QListWidgetItem(e.text + "/" if e.isDir else e.text)
            item.setData(ROLE_NAME, e.name)
            item.setData(ROLE_IS_DIR, e.isDir)
            tip = [e.name] if e.text != e.name else []
            tip.append(e.error or ("directory" if e.isDir else human_size(e.size)))
            item.setToolTip("\n".join(tip))
            if e.error:
                item.setData(ROLE_TRANSFORM_ERROR, True)
                item.setBackground(TRANSFORM_ERROR_BG)
                item.setForeground(QColor(Qt.black))  # stay readable in dark mode
            if e.name == current:
                row = self.list.count()
            self.list.addItem(item)
        self.list.blockSignals(False)

        nDirs = sum(e.isDir for e in entries)
        nFiles = len(entries) - nDirs
        nErrors = sum(e.error is not None for e in entries)
        if error:
            self.onStatus(error)
        elif not entries:
            self.onStatus("No files")
        else:
            status = "%d file%s" % (nFiles, "" if nFiles == 1 else "s")
            if nDirs:
                status += ", %d dir%s" % (nDirs, "" if nDirs == 1 else "s")
            if nErrors:
                status += ", %d transform error%s" % (nErrors, "" if nErrors == 1 else "s")
            self.onStatus(status)
        if self.list.count():
            self.list.setCurrentRow(row)
            self.itemChanged(self.list.currentItem())
        else:
            self.onSelect(None)
        if not self.filterEdit.hasFocus():
            self.list.setFocus()

    def itemPath(self, item):
        return os.path.normpath(os.path.join(self.directory, item.data(ROLE_NAME)))

    def itemChanged(self, item, _prev=None):
        self.onSelect(self.itemPath(item) if item else None)

    def itemActivated(self, item):
        if not item.data(ROLE_IS_DIR):
            return
        if item.data(ROLE_NAME) == PARENT_DIR:
            # land on the directory we just left
            self.setDirectory(os.path.dirname(self.directory), os.path.basename(self.directory))
        else:
            self.setDirectory(self.itemPath(item))

    def showContextMenu(self, pos):
        item = self.list.itemAt(pos)
        if item is None or item.data(ROLE_NAME) == PARENT_DIR:
            return
        name = item.data(ROLE_NAME)
        path = self.itemPath(item)
        kind = "directory" if item.data(ROLE_IS_DIR) else "file"
        menu = QMenu(self)
        menu.addAction("Copy %s" % kind, lambda: self.copyFile(path))
        menu.addAction("Copy %sname" % ("directory " if kind == "directory" else "file"),
                       lambda: QApplication.clipboard().setText(name))
        menu.exec(self.list.viewport().mapToGlobal(pos))

    @staticmethod
    def copyFile(path):
        # file URL on the clipboard lets Explorer paste the file itself
        mime = QMimeData()
        mime.setUrls([QUrl.fromLocalFile(path)])
        QApplication.clipboard().setMimeData(mime)


class MainWindow(QMainWindow):
    def __init__(self, directory):
        super().__init__()
        self.setWindowTitle("File Preview")
        self.resize(1100, 700)

        self.preview = PreviewPanel()
        self.files = FileListPanel(self.onFileSelected, self.statusBar().showMessage)
        self.settings = QSettings("mir2x", "filepreview")
        self.files.setTransformCode(self.settings.value(SETTING_TRANSFORM_CODE, TRANSFORM_TEMPLATE))

        splitter = QSplitter()
        splitter.addWidget(self.preview)
        splitter.addWidget(self.files)
        splitter.setStretchFactor(0, 3)
        splitter.setStretchFactor(1, 1)
        splitter.setSizes([800, 300])
        self.setCentralWidget(splitter)
        self.createMenus()

        self.files.list.installEventFilter(self)
        self.files.setDirectory(directory)

    def createMenus(self):
        bar = self.menuBar()

        openMenu = bar.addMenu("&Open")
        openDir = openMenu.addAction("Open &Directory...", self.openDirectory)
        openDir.setShortcut(QKeySequence.Open)

        viewMenu = bar.addMenu("&View")
        clearBg = viewMenu.addAction("&Clear Background")
        clearBg.setCheckable(True)
        clearBg.toggled.connect(self.setClearBackground)
        border = viewMenu.addAction("Draw &Border")
        border.setCheckable(True)
        border.toggled.connect(self.setDrawBorder)
        showDirs = viewMenu.addAction("Show &Directory")
        showDirs.setCheckable(True)
        showDirs.toggled.connect(self.files.setShowDirs)

        sortMenu = bar.addMenu("&Sort")
        sortGroup = QActionGroup(self)
        for name in SORT_METHODS:
            act = sortMenu.addAction(name)
            act.setCheckable(True)
            act.setChecked(name == self.files.sortName)
            act.triggered.connect(lambda _checked, n=name: self.files.setSort(n))
            sortGroup.addAction(act)
        sortMenu.addSeparator()
        desc = sortMenu.addAction("&Descending")
        desc.setCheckable(True)
        desc.toggled.connect(self.files.setDescending)
        mixDirs = sortMenu.addAction("Sort &Including Directory")
        mixDirs.setCheckable(True)
        mixDirs.setToolTip("Sort directories together with files instead of listing them first")
        mixDirs.toggled.connect(self.files.setMixDirs)
        errorsLast = sortMenu.addAction("Transform &Errors Last")
        errorsLast.setCheckable(True)
        errorsLast.setChecked(self.files.errorsLast)
        errorsLast.toggled.connect(self.files.setErrorsLast)
        sortMenu.setToolTipsVisible(True)

        filterMenu = bar.addMenu("&Filter")
        filterGroup = QActionGroup(self)
        for mode in (FILTER_WILDCARD, FILTER_REGEX):
            act = filterMenu.addAction(mode)
            act.setCheckable(True)
            act.setChecked(mode == self.files.filterMode)
            act.triggered.connect(lambda _checked, m=mode: self.files.setFilterMode(m))
            filterGroup.addAction(act)

        transformMenu = bar.addMenu("&Transform")
        self.transformAction = transformMenu.addAction("&Enable")
        self.transformAction.setCheckable(True)
        self.transformAction.setToolTip("Rename or hide entries with Python code before the wildcard/regex filter")
        self.transformAction.toggled.connect(self.files.setTransformEnabled)
        transformMenu.addAction("&Edit...", self.editTransform)
        transformMenu.setToolTipsVisible(True)

    def editTransform(self):
        TransformDialog(self, self.files.transformCode, self.settings.value(SETTING_TRANSFORM_FILE, ""),
                        self.applyTransform).exec()

    def applyTransform(self, code, filePath):
        self.settings.setValue(SETTING_TRANSFORM_CODE, code)
        self.settings.setValue(SETTING_TRANSFORM_FILE, filePath)
        self.files.setTransformCode(code)
        self.transformAction.setChecked(True)

    def openDirectory(self):
        d = QFileDialog.getExistingDirectory(self, "Open Directory", self.files.directory)
        if d:
            self.files.setDirectory(d)

    def setClearBackground(self, clear):
        self.preview.imageView.clearBackground = clear
        self.preview.imageView.update()

    def setDrawBorder(self, draw):
        self.preview.imageView.drawBorder = draw
        self.preview.imageView.update()

    def onFileSelected(self, path):
        if path is None:
            self.preview.clear("No file")
        else:
            self.preview.showFile(path)
            self.setWindowTitle("File Preview - %s" % path)

    def eventFilter(self, obj, event):
        # Up/Down are handled by the list itself; intercept preview keys here
        if event.type() == QEvent.KeyPress and event.modifiers() == Qt.NoModifier:
            key = event.key()
            if key == Qt.Key_Space:
                self.preview.togglePlay()
                return True
            if key == Qt.Key_Left:
                self.preview.stepFrame(-1)
                return True
            if key == Qt.Key_Right:
                self.preview.stepFrame(1)
                return True
            if key == Qt.Key_F:
                self.preview.fitCheck.toggle()
                return True
        return super().eventFilter(obj, event)


def main():
    if sys.platform == "win32":
        # own taskbar identity so Windows shows our icon instead of python.exe's
        import ctypes
        ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID("mir2x.filepreview")
    app = QApplication(sys.argv)
    app.setWindowIcon(QIcon(resource_path("favicon.ico")))
    win = MainWindow(sys.argv[1] if len(sys.argv) > 1 else os.getcwd())
    win.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
