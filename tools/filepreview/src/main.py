#!/usr/bin/env python3
"""Simple file previewer: preview on the left, sorted file list on the right.

Usage: python3 main.py [directory]

The preview depends on the file type, detected from the content by libmagic:
PNG/APNG and other images, PDFs (all pages, rendered as they scroll into view),
Word .docx documents, rendered Markdown, source code highlighted by Pygments,
plain text, or a hex dump. File types are pluggable, see preview/registry.py.
Config > Preview Type shows the current file as another type; Config > Preview
Options has settings for the current type (image background, code theme, text
encoding, ...), saved between runs.

Requires: pip install PySide6 pygments python-magic-bin mammoth
(python-magic-bin bundles libmagic for Windows; on Linux use python-magic.)

Keys: Ctrl+O open directory, Up/Down move selection, Space play/pause animation,
      Left/Right step animation frames, F toggle fit-to-window,
      Enter or double-click enter a directory (shown with the checkbox next to the filter).

Transform > Enable runs a user-defined transform(path: pathlib.Path)
on every entry before the wildcard/regex filter; it can rename or hide entries.
It is edited via Transform > Edit... and saved between runs.
"""

import os
import sys

from PySide6.QtCore import Qt, QEvent, QSettings
from PySide6.QtGui import QActionGroup, QKeySequence, QIcon
from PySide6.QtWidgets import QApplication, QMainWindow, QSplitter, QFileDialog

from entries import FILTER_REGEX, FILTER_WILDCARD, SORT_METHODS
from filelist import FileListPanel
from preview import PreviewPanel
from transform import TRANSFORM_TEMPLATE
from transformdialog import TransformDialog


SETTING_TRANSFORM_CODE = "transform/code"
SETTING_TRANSFORM_FILE = "transform/file"


def resource_path(name):
    """Locate a bundled data file, both from source and in a PyInstaller build."""
    base = getattr(sys, "_MEIPASS", os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base, name)


class MainWindow(QMainWindow):
    def __init__(self, directory):
        super().__init__()
        self.setWindowTitle("File Preview")
        self.resize(1100, 700)

        self.preview = PreviewPanel()
        self.files = FileListPanel(self.onFileSelected, self.statusBar().showMessage)
        self.settings = QSettings("mir2x", "filepreview")
        self.files.setTransformCode(self.settings.value(SETTING_TRANSFORM_CODE, TRANSFORM_TEMPLATE))

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

        configMenu = bar.addMenu("&Config")
        typeMenu = configMenu.addMenu("Preview &Type")
        typeMenu.aboutToShow.connect(lambda: self.preview.fillTypeMenu(typeMenu))
        optionsMenu = configMenu.addMenu("Preview &Options")
        optionsMenu.aboutToShow.connect(lambda: self.preview.fillOptionsMenu(optionsMenu))

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
        errorsLast = sortMenu.addAction("Sin&k Failed Transforms")
        errorsLast.setCheckable(True)
        errorsLast.setChecked(self.files.errorsLast)
        errorsLast.setToolTip("List entries the filename transform failed on after all the others, "
                              "sorted separately; off: sort them together with the others")
        errorsLast.toggled.connect(self.files.setErrorsLast)
        sortMenu.setToolTipsVisible(True)

        filterMenu = bar.addMenu("&Filter")
        filterGroup = QActionGroup(self)
        for mode in (FILTER_WILDCARD, FILTER_REGEX):
            act = filterMenu.addAction(mode)
            act.setCheckable(True)
            act.setChecked(mode == self.files.filterMode)
            act.triggered.connect(lambda _checked, m=mode: self.files.setFilterMode(m))
            filterGroup.addAction(act)

        transformMenu = bar.addMenu("&Transform")
        self.transformAction = transformMenu.addAction("&Enable")
        self.transformAction.setCheckable(True)
        self.transformAction.setToolTip("Rename or hide entries with Python code before the wildcard/regex filter")
        self.transformAction.toggled.connect(self.files.setTransformEnabled)
        transformMenu.addAction("&Edit...", self.editTransform)
        transformMenu.setToolTipsVisible(True)

    def editTransform(self):
        TransformDialog(self, self.files.transformCode, self.settings.value(SETTING_TRANSFORM_FILE, ""),
                        self.applyTransform).exec()

    def applyTransform(self, code, filePath):
        self.settings.setValue(SETTING_TRANSFORM_CODE, code)
        self.settings.setValue(SETTING_TRANSFORM_FILE, filePath)
        self.files.setTransformCode(code)
        self.transformAction.setChecked(True)

    def openDirectory(self):
        d = QFileDialog.getExistingDirectory(self, "Open Directory", self.files.directory)
        if d:
            self.files.setDirectory(d)

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
                self.preview.imagePane.togglePlay()
                return True
            if key == Qt.Key_Left:
                self.preview.imagePane.stepFrame(-1)
                return True
            if key == Qt.Key_Right:
                self.preview.imagePane.stepFrame(1)
                return True
            if key == Qt.Key_F:
                self.preview.imagePane.fitCheck.toggle()
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
