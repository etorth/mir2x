"""Preview pane: images, APNG animation and text."""

import os

from PySide6.QtCore import Qt, QTimer, QRect
from PySide6.QtGui import QImage, QPainter, QPixmap, QColor, QFontDatabase
from PySide6.QtWidgets import (
    QWidget, QLabel, QCheckBox, QPushButton, QHBoxLayout, QVBoxLayout, QPlainTextEdit,
    QStackedWidget, QSpinBox,
)

import apng
from entries import human_size


PNG_EXTS = {".png", ".apng"}
IMAGE_EXTS = {".jpg", ".jpeg", ".bmp", ".gif", ".webp", ".tga", ".ico"}
TEXT_PREVIEW_BYTES = 64 * 1024


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
