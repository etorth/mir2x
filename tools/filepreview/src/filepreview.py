#!/usr/bin/env python3
"""Simple file previewer: preview on the left, sorted file list on the right.

Usage: python3 filepreview.py [directory]

Keys: Ctrl+O open directory, Up/Down move selection, Space play/pause animation,
      Left/Right step animation frames, F toggle fit-to-window.
"""

import fnmatch
import os
import re
import sys

from PySide6.QtCore import Qt, QTimer, QRect, QEvent, QMimeData, QUrl
from PySide6.QtGui import (
    QImage, QPainter, QPixmap, QColor, QFontDatabase, QActionGroup, QKeySequence, QIcon,
)
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QSplitter, QListWidget, QListWidgetItem, QMenu,
    QLabel, QCheckBox, QLineEdit, QPushButton, QHBoxLayout, QVBoxLayout,
    QFileDialog, QPlainTextEdit, QStackedWidget, QSpinBox,
)

import apng

PNG_EXTS = {".png", ".apng"}
IMAGE_EXTS = {".jpg", ".jpeg", ".bmp", ".gif", ".webp", ".tga", ".ico"}
TEXT_PREVIEW_BYTES = 64 * 1024


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


# name -> key(entry); entries are (name, size, mtime) tuples
SORT_METHODS = {
    "Name (natural)": lambda e: natural_key(e[0]),
    "Name (hex)":     lambda e: hex_name_key(e[0]),
    "Name":           lambda e: e[0].lower(),
    "Extension":      lambda e: (os.path.splitext(e[0])[1].lower(), natural_key(e[0])),
    "Size":           lambda e: (e[1], natural_key(e[0])),
    "Modified time":  lambda e: (e[2], natural_key(e[0])),
}


FILTER_WILDCARD = "Wildcard"
FILTER_REGEX = "Regex"


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


def list_files(path, sort_name, descending, name_filter="", mode=FILTER_WILDCARD):
    pattern = compile_filter(name_filter, mode)
    entries = []
    try:
        for e in os.scandir(path):
            if e.is_file():
                st = e.stat()
                entries.append((e.name, st.st_size, st.st_mtime))
    except OSError:
        return []
    if pattern is not None:
        entries = [e for e in entries if pattern.search(e[0])]
    entries.sort(key=SORT_METHODS[sort_name], reverse=descending)
    return entries


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
        self.zoomSpin = QSpinBox()
        self.zoomSpin.setRange(1, 16)
        self.zoomSpin.setSuffix("x")
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
        self.imageView.zoom = self.zoomSpin.value()
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
            self.showAnimControls(True)
            self.playButton.setText("Pause")
        self.info.setText(text)
        self.setFrame(0)
        if len(self.frames) > 1:
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


class FileListPanel(QWidget):
    def __init__(self, onSelect, onStatus):
        super().__init__()
        self.onSelect = onSelect
        self.onStatus = onStatus
        self.directory = ""
        self.sortName = next(iter(SORT_METHODS))
        self.descending = False
        self.filterMode = FILTER_WILDCARD

        self.filterEdit = QLineEdit()
        self.filterEdit.textChanged.connect(self.refresh)
        self.updateFilterHint()

        self.list = QListWidget()
        self.list.setFont(QFontDatabase.systemFont(QFontDatabase.FixedFont))
        self.list.currentItemChanged.connect(self.itemChanged)
        self.list.setContextMenuPolicy(Qt.CustomContextMenu)
        self.list.customContextMenuRequested.connect(self.showContextMenu)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.filterEdit)
        layout.addWidget(self.list, 1)

    def setDirectory(self, path):
        path = os.path.abspath(os.path.expanduser(path))
        if not os.path.isdir(path):
            return
        self.directory = path
        self.refresh()

    def setSort(self, name):
        self.sortName = name
        self.refresh()

    def setDescending(self, descending):
        self.descending = descending
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
        current = self.list.currentItem().text() if self.list.currentItem() else None
        error = None
        try:
            entries = list_files(self.directory, self.sortName, self.descending,
                                 self.filterEdit.text(), self.filterMode)
        except re.error as exc:
            entries = []
            error = "Invalid regex: %s" % exc
        self.list.blockSignals(True)
        self.list.clear()
        row = 0
        for i, (name, size, _) in enumerate(entries):
            item = QListWidgetItem(name)
            item.setToolTip(human_size(size))
            self.list.addItem(item)
            if name == current:
                row = i
        self.list.blockSignals(False)
        if error:
            self.onStatus(error)
        elif not entries:
            self.onStatus("No files")
        else:
            self.onStatus("%d file%s" % (len(entries), "" if len(entries) == 1 else "s"))
        if entries:
            self.list.setCurrentRow(row)
            self.itemChanged(self.list.currentItem())
        else:
            self.onSelect(None)
        if not self.filterEdit.hasFocus():
            self.list.setFocus()

    def itemChanged(self, item, _prev=None):
        self.onSelect(os.path.join(self.directory, item.text()) if item else None)

    def showContextMenu(self, pos):
        item = self.list.itemAt(pos)
        if item is None:
            return
        name = item.text()
        path = os.path.join(self.directory, name)
        menu = QMenu(self)
        menu.addAction("Copy file", lambda: self.copyFile(path))
        menu.addAction("Copy filename", lambda: QApplication.clipboard().setText(name))
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

        filterMenu = bar.addMenu("&Filter")
        filterGroup = QActionGroup(self)
        for mode in (FILTER_WILDCARD, FILTER_REGEX):
            act = filterMenu.addAction(mode)
            act.setCheckable(True)
            act.setChecked(mode == self.files.filterMode)
            act.triggered.connect(lambda _checked, m=mode: self.files.setFilterMode(m))
            filterGroup.addAction(act)

    def openDirectory(self):
        d = QFileDialog.getExistingDirectory(self, "Open Directory", self.files.directory)
        if d:
            self.files.setDirectory(d)

    def setClearBackground(self, clear):
        self.preview.imageView.clearBackground = clear
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
