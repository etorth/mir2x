"""File list pane: filter box and the list of entries."""

import os
import re

from PySide6.QtCore import Qt, QMimeData, QUrl
from PySide6.QtGui import QColor, QFontDatabase
from PySide6.QtWidgets import (
    QApplication, QCheckBox, QWidget, QListWidget, QListWidgetItem, QMenu, QLineEdit, QHBoxLayout, QVBoxLayout,
    QStyledItemDelegate, QStyle,
)

from entries import (
    filter_entries, FILTER_REGEX, FILTER_WILDCARD, human_size, scan_dir, sort_entries, SORT_METHODS,
)
from transform import apply_transform, compile_transform, format_error, TRANSFORM_TEMPLATE


PARENT_DIR = ".."
ROLE_NAME = Qt.UserRole        # real entry name; display text adds "/" to dirs
ROLE_IS_DIR = Qt.UserRole + 1
ROLE_TRANSFORM_ERROR = Qt.UserRole + 2

TRANSFORM_ERROR_BG = QColor(255, 210, 220)
TRANSFORM_ERROR_MARK = QColor(220, 40, 90)


class EntryDelegate(QStyledItemDelegate):
    """Keeps transform-error entries recognizable while selected: the selection
    highlight hides the pink background, so draw a marker bar on the left."""

    def paint(self, painter, option, index):
        super().paint(painter, option, index)
        if index.data(ROLE_TRANSFORM_ERROR) and option.state & QStyle.State_Selected:
            r = option.rect
            painter.fillRect(r.x(), r.y(), 4, r.height(), TRANSFORM_ERROR_MARK)


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
        self.errorsLast = True
        self.selectName = None  # entry to select on the next refresh
        self.transformEnabled = False
        self.transformCode = TRANSFORM_TEMPLATE
        self.transformFunc = compile_transform(TRANSFORM_TEMPLATE)
        self.transformError = None

        self.filterEdit = QLineEdit()
        self.filterEdit.textChanged.connect(self.refresh)
        self.updateFilterHint()
        self.dirCheck = QCheckBox()
        self.dirCheck.setToolTip("Show directories; Enter or double-click opens one")
        self.dirCheck.toggled.connect(self.setShowDirs)

        self.list = QListWidget()
        self.list.setItemDelegate(EntryDelegate(self.list))
        self.list.setFont(QFontDatabase.systemFont(QFontDatabase.FixedFont))
        self.list.currentItemChanged.connect(self.itemChanged)
        self.list.itemActivated.connect(self.itemActivated)  # double-click or Enter
        self.list.setContextMenuPolicy(Qt.CustomContextMenu)
        self.list.customContextMenuRequested.connect(self.showContextMenu)

        filterRow = QHBoxLayout()
        filterRow.addWidget(self.filterEdit, 1)
        filterRow.addWidget(self.dirCheck)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addLayout(filterRow)
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
        sort_entries(entries, self.sortName, self.descending, self.errorsLast)

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
