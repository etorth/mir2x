"""The preview panel: picks a previewer for the selected file and shows its view."""

from PySide6.QtCore import Qt, QSize
from PySide6.QtWidgets import QWidget, QLabel, QVBoxLayout, QStackedWidget

from entries import human_size
from .base import FileInfo
from .registry import PREVIEWERS
from .views import ImagePane


FIELD_SEP = " | "


def elide_fields(metrics, fields, width):
    """Join fields with FIELD_SEP, shortening the longest ones with "..." to fit width."""
    fields = list(fields)
    for _ in fields:
        line = FIELD_SEP.join(fields)
        excess = metrics.horizontalAdvance(line) - width
        if excess <= 0:
            return line
        i = max(range(len(fields)), key=lambda k: metrics.horizontalAdvance(fields[k]))
        fields[i] = metrics.elidedText(fields[i], Qt.ElideRight, max(0, metrics.horizontalAdvance(fields[i]) - excess))
    return metrics.elidedText(FIELD_SEP.join(fields), Qt.ElideRight, width)


class ElidedLabel(QLabel):
    """A label that elides its lines with "..." to fit, instead of widening its window."""

    def __init__(self):
        super().__init__()
        self.setTextFormat(Qt.PlainText)  # widths are measured as plain text; file names aren't HTML
        self.lines = []

    def setText(self, text):
        self.setLines([[line] for line in text.split("\n")])

    def setFields(self, fields):
        """One line of fields; the longest fields are shortened first, the others stay whole."""
        self.setLines([fields])

    def setLines(self, lines):
        self.lines = lines
        self.setToolTip("\n".join(FIELD_SEP.join(fields) for fields in lines))
        self.elide()

    def elide(self):
        metrics, width = self.fontMetrics(), self.contentsRect().width()
        super().setText("\n".join(elide_fields(metrics, fields, width) for fields in self.lines))

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.elide()

    def minimumSizeHint(self):
        return QSize(0, super().minimumSizeHint().height())


class PreviewPanel(QWidget):
    def __init__(self):
        super().__init__()
        self.stack = QStackedWidget()
        self.views = {}
        self.imagePane = self.view(ImagePane)  # created up front for the View menu options

        self.info = ElidedLabel()
        self.info.setTextInteractionFlags(Qt.TextSelectableByMouse)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.stack, 1)
        layout.addWidget(self.info)

    def view(self, cls):
        """Return the shared instance of view class cls, made current."""
        view = self.views.get(cls)
        if view is None:
            view = self.views[cls] = cls()
            self.stack.addWidget(view)
        self.stack.setCurrentWidget(view)
        return view

    def clear(self, message=""):
        for view in self.views.values():
            view.clear()
        self.stack.setCurrentWidget(self.imagePane)
        self.info.setText(message)

    def showFile(self, path):
        self.clear()
        try:
            f = FileInfo.load(path)
            previewer = next(p for p in PREVIEWERS if p.match(f))
            fields = previewer.show(f, self)
        except Exception as e:  # a broken file or previewer must not take the app down
            self.clear("%s\nError: %s" % (path, e))
            return
        if f.isDir:
            self.info.setFields([f.path] + fields)
        else:
            self.info.setFields([f.name] + fields + [human_size(f.size)])
