"""PDF: renders the first page with QtPdf (skipped if QtPdf is unavailable)."""

from PySide6.QtCore import Qt, QSize
from PySide6.QtGui import QImage, QPainter

from .base import Previewer
from .views import ImagePane

try:
    from PySide6.QtPdf import QPdfDocument
except ImportError:
    QPdfDocument = None

RENDER_SCALE = 2.0 * 96 / 72  # 2x screen resolution, page sizes are in points
MAX_RENDER_SIDE = 4096


class PdfPreviewer(Previewer):
    def match(self, f):
        return QPdfDocument is not None and f.mime == "application/pdf"

    def show(self, f, panel):
        doc = QPdfDocument()
        err = doc.load(f.path)
        if err != QPdfDocument.Error.None_:
            raise ValueError("cannot open PDF: %s" % err.name)
        if doc.pageCount() < 1:
            raise ValueError("PDF has no pages")

        count = doc.pageCount()
        page = doc.pagePointSize(0)
        scale = min(RENDER_SCALE, MAX_RENDER_SIDE / max(page.width(), page.height(), 1))
        size = QSize(max(1, round(page.width() * scale)), max(1, round(page.height() * scale)))
        rendered = doc.render(0, size)
        doc.close()
        if rendered.isNull():
            raise ValueError("failed to render PDF page")

        # pages render on a transparent background; put them on paper
        image = QImage(size, QImage.Format_RGB32)
        image.fill(Qt.white)
        p = QPainter(image)
        p.drawImage(0, 0, rendered)
        p.end()

        panel.view(ImagePane).setFrames([(image, 0)])
        return ["PDF", "%d page%s" % (count, "" if count == 1 else "s"),
                "page 1: %gx%g pt" % (round(page.width()), round(page.height()))]
