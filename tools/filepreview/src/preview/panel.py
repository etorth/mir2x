"""The preview panel: picks a previewer for the selected file and shows its view."""

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QWidget, QLabel, QVBoxLayout, QStackedWidget

from entries import human_size
from .base import FileInfo
from .registry import PREVIEWERS
from .views import ImagePane


class PreviewPanel(QWidget):
    def __init__(self):
        super().__init__()
        self.stack = QStackedWidget()
        self.views = {}
        self.imagePane = self.view(ImagePane)  # created up front for the View menu options

        self.info = QLabel()
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
            self.info.setText(" | ".join([f.path] + fields))
        else:
            self.info.setText(" | ".join([f.name] + fields + [human_size(f.size)]))
