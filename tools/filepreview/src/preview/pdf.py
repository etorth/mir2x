"""PDF: all pages in a scrollable QPdfView (skipped if QtPdf is unavailable).

QPdfView renders only the visible pages, on demand and off the GUI thread, so
big documents open quickly and scrolling renders new pages as they appear.
"""

from .base import Previewer
from .options import Option

try:
    from PySide6.QtPdf import QPdfDocument
    from PySide6.QtPdfWidgets import QPdfView
except ImportError:
    QPdfDocument = QPdfView = None

FIT = Option("pdf/fit", "Fit", "width", (("width", "Page Width"), ("page", "Whole Page")))


if QPdfView is not None:
    class PdfView(QPdfView):
        def __init__(self):
            super().__init__()
            self.setPageMode(QPdfView.PageMode.MultiPage)
            self.setZoomMode(QPdfView.ZoomMode.FitToWidth)

        def load(self, path):
            self.clear()
            doc = QPdfDocument(self)
            err = doc.load(path)
            if err != QPdfDocument.Error.None_:
                doc.deleteLater()
                raise ValueError("cannot open PDF: %s" % err.name)
            self.setDocument(doc)
            self.verticalScrollBar().setValue(0)
            return doc

        def clear(self):
            # a closed QPdfDocument still holds its file open: delete it instead
            doc = self.document()
            if doc is not None:
                self.setDocument(None)
                doc.deleteLater()


class PdfPreviewer(Previewer):
    name = "PDF"
    options = (FIT,)

    def match(self, f):
        return QPdfView is not None and f.mime == "application/pdf"

    def show(self, f, panel):
        if QPdfView is None:
            raise ValueError("QtPdf is not available")
        view = panel.view(PdfView)
        view.setZoomMode(QPdfView.ZoomMode.FitInView if FIT.value() == "page" else QPdfView.ZoomMode.FitToWidth)
        doc = view.load(f.path)
        count, page = doc.pageCount(), doc.pagePointSize(0)
        fields = ["PDF", "%d page%s" % (count, "" if count == 1 else "s")]
        if count:
            fields.append("page 1: %gx%g pt" % (round(page.width()), round(page.height())))
        return fields
