"""The preview panel: picks a previewer for the selected file and shows its view."""

from PySide6.QtCore import Qt, QSize
from PySide6.QtWidgets import QWidget, QLabel, QMenu, QVBoxLayout, QStackedWidget

from entries import human_size
from .base import FileInfo
from .basic import DirectoryPreviewer
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
        self.imagePane = self.view(ImagePane)  # created up front for the animation keys
        self.path = None        # the file shown
        self.detected = None    # previewer picked by its type
        self.previewer = None   # previewer in use: detected or chosen in Config > Preview Type
        self.overrides = {}     # path -> previewer chosen for it, for this session

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
        self.path = self.detected = self.previewer = None
        self.clearViews(message)

    def clearViews(self, message=""):
        for view in self.views.values():
            view.clear()
        self.stack.setCurrentWidget(self.imagePane)
        self.info.setText(message)

    def showFile(self, path):
        self.clear()
        self.path = path
        try:
            f = FileInfo.load(path)
            self.detected = next(p for p in PREVIEWERS if p.match(f))
            self.previewer = self.overrides.get(path, self.detected)
            fields = self.previewer.show(f, self)
        except Exception as e:  # a broken file or previewer must not take the app down
            self.clearViews("%s\nError: %s" % (path, e))
            return
        if f.isDir:
            self.info.setFields([f.path] + fields)
        else:
            self.info.setFields([f.name] + fields + [human_size(f.size)])

    def reshow(self):
        if self.path is not None:
            self.showFile(self.path)

    def setPreviewType(self, previewer):
        """Show the current file with previewer, or None for the detected one."""
        if previewer is None:
            self.overrides.pop(self.path, None)
        else:
            self.overrides[self.path] = previewer
        self.reshow()

    def fillTypeMenu(self, menu):
        """Config > Preview Type: the detected type or any other, for the current file."""
        menu.clear()
        if self.detected is None or isinstance(self.detected, DirectoryPreviewer):
            menu.addAction("(no file)").setEnabled(False)
            return
        forced = self.overrides.get(self.path)
        choices = [("Auto (%s)" % self.detected.name, None)]
        choices += [(p.name, p) for p in PREVIEWERS if not isinstance(p, DirectoryPreviewer)]
        for label, previewer in choices:
            action = menu.addAction(label, lambda p=previewer: self.setPreviewType(p))
            action.setCheckable(True)
            action.setChecked(previewer is forced)
            if previewer is None:
                menu.addSeparator()

    def fillOptionsMenu(self, menu):
        """Config > Preview Options: the options of the previewer in use."""
        menu.clear()
        for sub in menu.findChildren(QMenu, options=Qt.FindDirectChildrenOnly):
            sub.deleteLater()  # clear() keeps the submenus addMenu() created
        options = self.previewer.options if self.previewer else ()
        if not options:
            menu.addAction("(none for this file type)").setEnabled(False)
        for option in options:
            if not isinstance(option.default, bool):
                sub = menu.addMenu(option.label)
                current = option.value()
                for value, label in option.choices:
                    action = sub.addAction(label, lambda o=option, v=value: self.setOption(o, v))
                    action.setCheckable(True)
                    action.setChecked(value == current)
            else:
                action = menu.addAction(option.label)
                action.setCheckable(True)
                action.setChecked(option.value())
                action.toggled.connect(lambda checked, o=option: self.setOption(o, checked))

    def setOption(self, option, value):
        option.setValue(value)
        self.reshow()
