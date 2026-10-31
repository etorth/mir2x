"""Reusable preview widgets. A previewer gets one with panel.view(cls);
each view class needs a clear() method."""

from PySide6.QtCore import QTimer, QRect
from PySide6.QtGui import QPainter, QPalette, QPixmap, QColor, QFontDatabase, QTextCharFormat, QTextCursor
from PySide6.QtWidgets import (
    QWidget, QLabel, QCheckBox, QPushButton, QHBoxLayout, QVBoxLayout, QPlainTextEdit, QSpinBox,
)


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


class ImagePane(QWidget):
    """ImageView with fit/zoom controls; plays a list of (QImage, delay_ms)
    frames, with animation controls when there is more than one."""

    def __init__(self):
        super().__init__()
        self.frames = []
        self.index = 0
        self.timer = QTimer(self)
        self.timer.setSingleShot(True)
        self.timer.timeout.connect(self.nextFrame)

        self.imageView = ImageView()

        self.playButton = QPushButton("Pause")
        self.playButton.clicked.connect(self.togglePlay)
        self.prevButton = QPushButton("<")
        self.prevButton.clicked.connect(lambda: self.stepFrame(-1))
        self.nextButton = QPushButton(">")
        self.nextButton.clicked.connect(lambda: self.stepFrame(1))
        self.frameLabel = QLabel()
        # 1-based frame range the animation loops over
        self.firstSpin = QSpinBox()
        self.firstSpin.setPrefix("Frame ")
        self.firstSpin.setToolTip("First frame played")
        self.lastSpin = QSpinBox()
        self.lastSpin.setPrefix("Frame ")
        self.lastSpin.setToolTip("Last frame played")
        self.firstSpin.valueChanged.connect(self.firstChanged)
        self.lastSpin.valueChanged.connect(self.lastChanged)
        self.animControls = (self.prevButton, self.playButton, self.nextButton, self.frameLabel,
                             self.firstSpin, QLabel("to"), self.lastSpin)

        self.fitCheck = QCheckBox("Fit")
        self.fitCheck.setChecked(True)
        self.fitCheck.toggled.connect(self.updateZoom)
        self.zoomSpin = ZoomSpinBox()
        self.zoomSpin.setEnabled(False)
        self.zoomSpin.valueChanged.connect(self.updateZoom)

        controls = QHBoxLayout()
        for w in self.animControls:
            controls.addWidget(w)
        controls.addStretch()
        controls.addWidget(self.fitCheck)
        controls.addWidget(self.zoomSpin)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.imageView, 1)
        layout.addLayout(controls)
        self.showAnimControls(False)

    def clear(self):
        self.timer.stop()
        self.frames = []
        self.imageView.setImage(None)
        self.showAnimControls(False)

    def setFrames(self, frames):
        """Show [(QImage, delay_ms)]; more than one frame starts playing."""
        self.clear()
        self.frames = frames
        for spin, value in ((self.firstSpin, 1), (self.lastSpin, len(frames))):
            spin.blockSignals(True)
            spin.setRange(1, max(1, len(frames)))
            spin.setValue(value)
            spin.blockSignals(False)
        self.setFrame(0)
        if self.isAnimated():
            self.showAnimControls(True)
            self.playButton.setText("Pause")
            self.timer.start(self.frames[0][1])

    def updateZoom(self):
        self.imageView.fit = self.fitCheck.isChecked()
        self.zoomSpin.setEnabled(not self.imageView.fit)
        self.imageView.zoom = self.zoomSpin.zoom()
        self.imageView.update()

    def showAnimControls(self, show):
        for w in self.animControls:
            w.setVisible(show)

    def frameRange(self):
        """0-based (first, last) frames to play."""
        return self.firstSpin.value() - 1, self.lastSpin.value() - 1

    def firstChanged(self, value):
        if self.lastSpin.value() < value:
            self.lastSpin.setValue(value)
        self.clampFrame()

    def lastChanged(self, value):
        if self.firstSpin.value() > value:
            self.firstSpin.setValue(value)
        self.clampFrame()

    def clampFrame(self):
        first, last = self.frameRange()
        self.setFrame(self.index if first <= self.index <= last else first)

    def setFrame(self, index):
        """Show frame index, wrapped into the First..Last range."""
        if not self.frames:
            return
        first, last = self.frameRange()
        self.index = first + (index - first) % (last - first + 1)
        self.imageView.setImage(self.frames[self.index][0])
        if self.isAnimated():
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


def set_colors(widget, background=None, text=None, link=None):
    """Recolor a text widget's page; no arguments restore the application palette."""
    palette = QPalette()
    if background is not None:
        palette.setColor(QPalette.Base, QColor(background))
        palette.setColor(QPalette.Text, QColor(text))
        if link is not None:
            palette.setColor(QPalette.Link, QColor(link))
    widget.setPalette(palette)


DARK_PAGE = ("#1e1e1e", "#d4d4d4", "#4ea1ff")  # background, text, link


def recolor_links(doc, color):
    """Qt bakes the application palette's link color into loaded Markdown/HTML: replace it."""
    fmt = QTextCharFormat()
    fmt.setForeground(QColor(color))
    cursor = QTextCursor(doc)
    block = doc.begin()
    while block.isValid():
        it = block.begin()
        while not it.atEnd():
            fragment = it.fragment()
            if fragment.charFormat().isAnchor():
                cursor.setPosition(fragment.position())
                cursor.setPosition(fragment.position() + fragment.length(), QTextCursor.KeepAnchor)
                cursor.mergeCharFormat(fmt)
            it += 1
        block = block.next()


class TextView(QPlainTextEdit):
    """Read-only monospace text, plain or as formatted runs."""

    def __init__(self):
        super().__init__()
        self.setReadOnly(True)
        self.setUndoRedoEnabled(False)
        self.setFont(QFontDatabase.systemFont(QFontDatabase.FixedFont))
        self.clear()

    def clear(self):
        super().clear()
        # setPlainText() uses the cursor's format, which setRuns() left at a highlight color
        self.setCurrentCharFormat(QTextCharFormat())
        self.setWordWrap(False)
        set_colors(self)

    def setWordWrap(self, wrap):
        self.setLineWrapMode(QPlainTextEdit.WidgetWidth if wrap else QPlainTextEdit.NoWrap)

    def setRuns(self, runs):
        """Show text given as [(str, QTextCharFormat)]."""
        super().clear()
        cursor = QTextCursor(self.document())
        cursor.beginEditBlock()
        for text, fmt in runs:
            cursor.insertText(text, fmt)
        cursor.endEditBlock()
        self.moveCursor(QTextCursor.Start)
