"""Raster images: PNG/APNG via the apng module, everything else via QImageReader."""

from PySide6.QtCore import Qt
from PySide6.QtGui import QImage, QImageReader, QPainter

import apng
from .base import Previewer
from .filetype import UNKNOWN_MIME
from .views import ImagePane

MAX_FRAMES = 1000
MAX_FRAME_BYTES = 512 * 1024 * 1024  # decoded frames of one animation


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


class PngPreviewer(Previewer):
    def match(self, f):
        return f.mime in ("image/png", "image/apng")

    def show(self, f, panel):
        frames, info = load_png_frames(f.path)
        panel.view(ImagePane).setFrames(frames)
        fields = ["APNG" if info.is_animated else "PNG", "%dx%d" % (info.width, info.height)]
        if info.is_animated:
            plays = "infinite" if info.num_plays == 0 else str(info.num_plays)
            fields += ["%d frames" % len(frames), "%d ms" % sum(d for _, d in frames), "loops: %s" % plays]
        return fields


class ImagePreviewer(Previewer):
    """Any type Qt's image plugins read (jpeg, bmp, gif, webp, svg, ...);
    animated GIF/WebP play like APNG."""

    def __init__(self):
        self.mimes = {bytes(m).decode() for m in QImageReader.supportedMimeTypes()} - {"application/pdf"}
        formats = {bytes(fmt).decode().lower() for fmt in QImageReader.supportedImageFormats()} - {"pdf"}
        self.exts = {"." + fmt for fmt in formats}

    def match(self, f):
        # some formats (tga, wbmp) have no signature libmagic knows: trust the extension then
        return f.mime in self.mimes or (f.mime == UNKNOWN_MIME and f.ext in self.exts)

    def show(self, f, panel):
        reader = QImageReader(f.path)
        reader.setAutoTransform(True)
        kind = bytes(reader.format()).decode().upper() or f.ext[1:].upper()
        frames = []
        loaded = 0
        partial = False
        while True:
            img = reader.read()
            if img.isNull():
                break
            frames.append((img, max(reader.nextImageDelay(), 10)))
            loaded += img.sizeInBytes()
            if not reader.supportsAnimation() or reader.imageCount() == len(frames):
                break
            if len(frames) >= MAX_FRAMES or loaded >= MAX_FRAME_BYTES:
                partial = True
                break
        if not frames:
            raise ValueError(reader.errorString())

        panel.view(ImagePane).setFrames(frames)
        img = frames[0][0]
        fields = [kind, "%dx%d" % (img.width(), img.height())]
        if len(frames) > 1:
            fields += ["%s%d frames" % ("first " if partial else "", len(frames)), "%d ms" % sum(d for _, d in frames)]
        return fields
