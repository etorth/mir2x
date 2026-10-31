"""Archives: ZIP (and ZIP-based files such as .jar, .apk or .xlsx), TAR and
compressed TAR; shows the list of entries, nothing is extracted."""

import datetime
import locale
import sys
import tarfile
import zipfile

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QTreeWidget, QTreeWidgetItem

from entries import human_size, natural_key
from .base import Previewer
from .options import TEXT_ENCODING

MAX_ENTRIES = 10000
ZIP_SIGNATURES = (b"PK\x03\x04", b"PK\x05\x06")  # first entry; empty archive
COMPRESSIONS = {
    "application/gzip": "gzip",
    "application/x-gzip": "gzip",
    "application/x-bzip2": "bzip2",
    "application/x-xz": "xz",
}
COMPRESSED_TAR_EXTS = (".tar.gz", ".tgz", ".tar.bz2", ".tbz", ".tbz2", ".tar.xz", ".txz")
COLUMNS = ("Name", "Size", "Packed", "Modified")


def zip_name_encoding():
    """Codec for ZIP entry names without the UTF-8 flag: the Non-UTF-8 Text Encoding
    option; its "system" default is the OEM code page Windows' own ZIP tool uses."""
    encoding = TEXT_ENCODING.value()
    if encoding != "system":
        return encoding
    if sys.platform == "win32":
        import ctypes
        return "cp%d" % ctypes.windll.kernel32.GetOEMCP()
    return locale.getencoding()


class ArchiveItem(QTreeWidgetItem):
    """Sorts by sortKeys, one per column, kept in Python: big ints or natural_key
    lists may not survive a round trip through Qt item data."""

    def __init__(self, texts, sortKeys):
        super().__init__(texts)
        self.sortKeys = sortKeys

    def __lt__(self, other):
        column = self.treeWidget().sortColumn()
        return self.sortKeys[column] < other.sortKeys[column]


class ArchiveView(QTreeWidget):
    def __init__(self):
        super().__init__()
        self.setHeaderLabels(COLUMNS)
        self.setRootIsDecorated(False)
        self.setUniformRowHeights(True)
        self.setSortingEnabled(True)
        self.sortByColumn(0, Qt.AscendingOrder)

    def setEntries(self, entries, showPacked):
        """entries: [(name, isDir, size, packed, mtime: datetime or None)]."""
        self.setSortingEnabled(False)  # sorting on every insert is slow
        self.clear()
        items = []
        for name, isDir, size, packed, mtime in entries:
            item = ArchiveItem([name,
                                "" if isDir else human_size(size),
                                "" if isDir else human_size(packed),
                                mtime.strftime("%Y-%m-%d %H:%M") if mtime else ""],
                               (natural_key(name), size, packed, mtime or datetime.datetime.min))
            item.setTextAlignment(1, Qt.AlignRight | Qt.AlignVCenter)
            item.setTextAlignment(2, Qt.AlignRight | Qt.AlignVCenter)
            items.append(item)
        self.addTopLevelItems(items)
        self.setColumnHidden(2, not showPacked)
        self.setSortingEnabled(True)
        self.resizeColumnToContents(0)
        self.setColumnWidth(0, min(self.columnWidth(0), 600))
        for column in (1, 2, 3):
            self.resizeColumnToContents(column)


def zip_entries(path):
    try:
        archive = zipfile.ZipFile(path, metadata_encoding=zip_name_encoding())
    except UnicodeDecodeError:  # names aren't in that encoding after all
        archive = zipfile.ZipFile(path)
    with archive:
        infos = archive.infolist()
    entries = []
    for info in infos[:MAX_ENTRIES]:
        try:
            mtime = datetime.datetime(*info.date_time)
        except ValueError:
            mtime = None
        entries.append((info.filename, info.is_dir(), info.file_size, info.compress_size, mtime))
    return entries, len(infos)


def tar_entries(path):
    """Reads at most MAX_ENTRIES + 1 headers: a compressed TAR is decompressed up to there."""
    entries = []
    with tarfile.open(path) as archive:
        while len(entries) <= MAX_ENTRIES and (info := archive.next()) is not None:
            name = info.name + ("/" if info.isdir() else "")
            if info.issym() or info.islnk():
                name += " -> " + info.linkname
            try:
                mtime = datetime.datetime.fromtimestamp(info.mtime) if info.mtime > 0 else None
            except (OverflowError, OSError, ValueError):
                mtime = None
            entries.append((name, info.isdir(), info.size, info.size, mtime))
    return entries[:MAX_ENTRIES], len(entries)


class ArchivePreviewer(Previewer):
    name = "Archive"
    options = (TEXT_ENCODING,)

    def isTar(self, f):
        if f.mime == "application/x-tar" or f.head[257:262] == b"ustar":
            return True
        return f.mime in COMPRESSIONS and f.name.lower().endswith(COMPRESSED_TAR_EXTS)

    def match(self, f):
        return f.head[:4] in ZIP_SIGNATURES or self.isTar(f)

    def show(self, f, panel):
        if f.head[:4] in ZIP_SIGNATURES:
            entries, total = zip_entries(f.path)
            kind = (f.description or "ZIP archive").split(",")[0]
        else:
            entries, total = tar_entries(f.path)
            kind = "TAR archive" + (", %s" % COMPRESSIONS[f.mime] if f.mime in COMPRESSIONS else "")
        panel.view(ArchiveView).setEntries(entries, f.head[:4] in ZIP_SIGNATURES)

        nDirs = sum(isDir for _, isDir, *_ in entries)
        nFiles = len(entries) - nDirs
        fields = [kind, "%d file%s" % (nFiles, "" if nFiles == 1 else "s")]
        if nDirs:
            fields[-1] += ", %d dir%s" % (nDirs, "" if nDirs == 1 else "s")
        if total > len(entries):
            fields.append("first %d entries shown" % len(entries))
        fields.append("%s unpacked" % human_size(sum(size for _, isDir, size, *_ in entries if not isDir)))
        return fields
