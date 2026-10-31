"""CSV/TSV files shown as a table."""

import csv
import io
import itertools

from PySide6.QtCore import Qt, QAbstractTableModel, QModelIndex
from PySide6.QtWidgets import QTableView

from .base import Previewer, truncated_note
from .options import Option, TEXT_ENCODING

CSV_BYTES = 1024 * 1024
MAX_ROWS = 10000
MAX_COLUMN_WIDTH = 400
CSV_MIMES = {"text/csv", "application/csv", "text/tab-separated-values"}
DEFAULT_DELIMITERS = {".csv": ",", ".tsv": "\t", ".tab": "\t"}
DELIMITER_NAMES = {",": "CSV", "\t": "TSV"}
HEADER_ROW = Option("csv/headerRow", "First Row Is Header", True)


class RowsModel(QAbstractTableModel):
    def __init__(self, header, rows):
        super().__init__()
        self.header = header
        self.rows = rows
        self.columns = max(itertools.chain([len(header)], (len(r) for r in rows)))

    def rowCount(self, parent=QModelIndex()):
        return 0 if parent.isValid() else len(self.rows)

    def columnCount(self, parent=QModelIndex()):
        return 0 if parent.isValid() else self.columns

    def data(self, index, role=Qt.DisplayRole):
        if role in (Qt.DisplayRole, Qt.ToolTipRole):
            row = self.rows[index.row()]
            return row[index.column()] if index.column() < len(row) else None
        return None

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role != Qt.DisplayRole:
            return None
        if orientation == Qt.Horizontal and section < len(self.header):
            return self.header[section]
        return str(section + 1)


class TableView(QTableView):
    def __init__(self):
        super().__init__()
        self.setWordWrap(False)
        self.horizontalHeader().setResizeContentsPrecision(100)  # measure 100 rows, not 1000
        self.verticalHeader().setDefaultSectionSize(self.fontMetrics().height() + 6)
        self.rowsModel = None  # the view doesn't own its model: keep it alive

    def clear(self):
        self.setModel(None)
        self.rowsModel = None

    def setRows(self, header, rows):
        self.rowsModel = RowsModel(header, rows)
        self.setModel(self.rowsModel)
        self.resizeColumnsToContents()
        for column in range(self.rowsModel.columns):
            self.setColumnWidth(column, min(self.columnWidth(column), MAX_COLUMN_WIDTH))


class CsvPreviewer(Previewer):
    name = "Table"
    options = (HEADER_ROW, TEXT_ENCODING)

    def match(self, f):
        return f.isText() and (f.mime in CSV_MIMES or f.ext in DEFAULT_DELIMITERS)

    def show(self, f, panel):
        text, truncated = f.readText(CSV_BYTES)
        if truncated and "\n" in text:
            text = text[:text.rindex("\n") + 1]  # drop the cut-off last line
        delimiter = DEFAULT_DELIMITERS.get(f.ext, ",")
        if delimiter == ",":  # .csv may also mean ";" (European locales) or another delimiter
            try:
                delimiter = csv.Sniffer().sniff(text[:64 * 1024], delimiters=",;\t|").delimiter
            except csv.Error:  # one column, or too irregular to tell
                pass
        rows = [r for r in csv.reader(io.StringIO(text, newline=""), delimiter=delimiter) if r]
        shownRows = rows[:MAX_ROWS + 1]
        header = shownRows.pop(0) if HEADER_ROW.value() and shownRows else []
        shownRows = shownRows[:MAX_ROWS]
        view = panel.view(TableView)
        view.setRows(header, shownRows)

        nRows, nColumns = len(shownRows), view.rowsModel.columns
        fields = [DELIMITER_NAMES.get(delimiter, "delimiter %r" % delimiter),
                  "%d row%s x %d column%s" % (nRows, "" if nRows == 1 else "s",
                                               nColumns, "" if nColumns == 1 else "s")]
        if len(rows) - bool(header) > MAX_ROWS:
            fields.append("first %d rows shown" % MAX_ROWS)
        return fields + truncated_note(truncated, CSV_BYTES)
