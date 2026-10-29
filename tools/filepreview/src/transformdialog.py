"""Editor dialog for the filename transform code."""

import traceback

from PySide6.QtCore import Qt
from PySide6.QtGui import QFontDatabase, QKeySequence, QTextCursor
from PySide6.QtWidgets import (
    QLabel, QVBoxLayout, QFileDialog, QPlainTextEdit, QDialog, QDialogButtonBox, QMenuBar,
    QMessageBox,
)

from codeedit import CodeEdit, PythonHighlighter
from transform import compile_transform, TRANSFORM_TEMPLATE


TRANSFORM_HELP = '''\
<p>The code must define <code>transform(path)</code>. It is called with a <code>pathlib.Path</code>
for every file and directory in the list (never for ".."), and its return value decides
how the entry is shown:</p>
<table cellspacing="0" cellpadding="3" style="margin-left: 16px">
<tr><td><code>True</code></td><td>show the original name</td></tr>
<tr><td><code>False</code></td><td>hide the entry</td></tr>
<tr><td>non-empty <code>str</code></td><td>show this text instead of the name</td></tr>
<tr><td>anything else, or an exception&nbsp;&nbsp;</td>
    <td>show the original name in pink; hover over it to see why</td></tr>
</table>
<p>The wildcard/regex filter then matches the shown text.</p>
<p><b>Apply</b> checks the code and turns on <b>Transform &gt; Enable</b>;
<b>OK</b> also closes this window. <b>Save &amp; Apply</b> also writes the code to the
loaded file. The applied code is remembered between runs.</p>
<p>Sorting by name uses the shown text; <b>Sort &gt; Transform Errors Last</b> puts pink
entries at the end.</p>
<p>The packaged filepreview.exe can only import the Python standard library, PySide6, Pygments and python-magic.
A transform that never returns freezes the app.</p>'''


class TransformDialog(QDialog):
    def __init__(self, parent, code, filePath, onApply):
        """onApply(code, filePath) installs code; it is only called once code compiles."""
        super().__init__(parent)
        self.onApply = onApply
        self.filePath = filePath
        self.resize(700, 450)

        self.editor = CodeEdit()
        self.editor.setFont(QFontDatabase.systemFont(QFontDatabase.FixedFont))
        self.editor.setLineWrapMode(QPlainTextEdit.NoWrap)
        self.editor.setPlainText(code)
        self.highlighter = PythonHighlighter(self.editor.document())
        self.editor.document().modificationChanged.connect(self.setWindowModified)
        self.errorLabel = QLabel()
        self.errorLabel.setStyleSheet("color: #c00000;")
        self.errorLabel.setWordWrap(True)
        self.errorLabel.setTextInteractionFlags(Qt.TextSelectableByMouse)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Apply | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        buttons.button(QDialogButtonBox.Apply).clicked.connect(self.apply)
        buttons.addButton("&Save && Apply", QDialogButtonBox.ApplyRole).clicked.connect(self.saveAndApply)

        layout = QVBoxLayout(self)
        layout.setMenuBar(self.createMenuBar())
        layout.addWidget(self.editor, 1)
        layout.addWidget(self.errorLabel)
        layout.addWidget(buttons)
        self.updateTitle()

    def createMenuBar(self):
        bar = QMenuBar(self)
        ed = self.editor

        openMenu = bar.addMenu("&Open")
        openMenu.addAction("&Load...", QKeySequence.Open, self.load)
        openMenu.addAction("&Save && Apply", QKeySequence.Save, self.saveAndApply)
        openMenu.addAction("Save &As && Apply...", QKeySequence.SaveAs, lambda: self.saveAndApply(askPath=True))
        openMenu.addSeparator()
        openMenu.addAction("&Restore Defaults", lambda: self.replaceText(TRANSFORM_TEMPLATE))

        editMenu = bar.addMenu("&Edit")
        undo = editMenu.addAction("&Undo", QKeySequence.Undo, ed.undo)
        redo = editMenu.addAction("&Redo", QKeySequence.Redo, ed.redo)
        editMenu.addSeparator()
        cut = editMenu.addAction("Cu&t", QKeySequence.Cut, ed.cut)
        copy = editMenu.addAction("&Copy", QKeySequence.Copy, ed.copy)
        editMenu.addAction("&Paste", QKeySequence.Paste, ed.paste)
        editMenu.addSeparator()
        editMenu.addAction("Select &All", QKeySequence.SelectAll, ed.selectAll)
        for act, signal in ((undo, ed.undoAvailable), (redo, ed.redoAvailable),
                            (cut, ed.copyAvailable), (copy, ed.copyAvailable)):
            act.setEnabled(False)
            signal.connect(act.setEnabled)

        helpMenu = bar.addMenu("&Help")
        helpMenu.addAction("Filename &Transform Help", QKeySequence.HelpContents, self.showHelp)
        return bar

    def showHelp(self):
        box = QMessageBox(QMessageBox.NoIcon, "Filename Transform Help", TRANSFORM_HELP, parent=self)
        box.setTextFormat(Qt.RichText)
        box.exec()

    def updateTitle(self):
        self.setWindowTitle("Filename Transform" + (f" - {self.filePath}" if self.filePath else "") + "[*]")

    def replaceText(self, text):
        # edit through a cursor instead of setPlainText() so Ctrl+Z can undo it
        cursor = self.editor.textCursor()
        cursor.select(QTextCursor.Document)
        cursor.insertText(text)
        cursor.movePosition(QTextCursor.Start)
        self.editor.setTextCursor(cursor)

    def showError(self, text):
        self.errorLabel.setText(text)

    def load(self):
        path, _ = QFileDialog.getOpenFileName(self, "Load Filename Transform", self.filePath,
                                              "Python files (*.py);;All files (*)")
        if not path:
            return
        try:
            with open(path, encoding="utf-8") as f:
                text = f.read()
        except (OSError, UnicodeDecodeError) as exc:
            self.showError(f"cannot load {path}: {exc}")
            return
        self.replaceText(text)
        self.editor.document().setModified(False)
        self.filePath = path
        self.updateTitle()
        self.errorLabel.clear()

    def apply(self):
        code = self.editor.toPlainText()
        try:
            compile_transform(code)
        except Exception as exc:
            # full form keeps the caret under a syntax error
            self.showError("".join(traceback.format_exception_only(type(exc), exc)).strip())
            return False
        self.errorLabel.clear()
        self.onApply(code, self.filePath)
        return True

    def saveAndApply(self, askPath=False):
        path = self.filePath
        if askPath or not path:
            path, _ = QFileDialog.getSaveFileName(self, "Save Filename Transform", path or "transform.py",
                                                  "Python files (*.py);;All files (*)")
            if not path:
                return
        try:
            compile_transform(self.editor.toPlainText())
        except Exception:
            return self.apply()  # shows the error; nothing is saved
        try:
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write(self.editor.toPlainText())
        except OSError as exc:
            self.showError(f"cannot save {path}: {exc}")
            return
        self.editor.document().setModified(False)
        self.filePath = path
        self.updateTitle()
        self.apply()

    def accept(self):
        if self.apply():
            super().accept()
