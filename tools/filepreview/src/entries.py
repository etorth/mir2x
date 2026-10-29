"""Directory entries: scanning, wildcard/regex filtering and sorting."""

import fnmatch
import os
import re
from dataclasses import dataclass


def natural_key(s):
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", s)]


def hex_name_key(name):
    stem = os.path.splitext(name)[0]
    if re.fullmatch(r"[0-9a-fA-F]+", stem):
        return (0, int(stem, 16), name.lower(), name)
    return (1, 0, name.lower(), name)


@dataclass
class Entry:
    name: str           # real name on disk
    size: int
    mtime: float
    isDir: bool
    text: str = None    # displayed name; the filename transform may change it
    error: str = None   # set when the filename transform misbehaved for this entry

    def __post_init__(self):
        if self.text is None:
            self.text = self.name


# name -> key(entry); name-based keys use the displayed text
SORT_METHODS = {
    "Name (natural)": lambda e: natural_key(e.text),
    "Name (hex)":     lambda e: hex_name_key(e.text),
    "Name":           lambda e: e.text.lower(),
    "Extension":      lambda e: (os.path.splitext(e.text)[1].lower(), natural_key(e.text)),
    "Size":           lambda e: (e.size, natural_key(e.text)),
    "Modified time":  lambda e: (e.mtime, natural_key(e.text)),
}


def sort_entries(entries, sort_name, descending, errors_last=True):
    """Sort in place, directories before files; with errors_last, entries
    the filename transform failed on go after all others."""
    entries.sort(key=SORT_METHODS[sort_name], reverse=descending)
    # stable sort keeps the order above inside each group
    entries.sort(key=lambda e: (errors_last and e.error is not None, not e.isDir))
    return entries


FILTER_WILDCARD = "Wildcard"
FILTER_REGEX = "Regex"


def compile_filter(name_filter, mode):
    """Return a case-insensitive compiled pattern, or None for no filtering.

    Wildcard: * and ? glob syntax matched against the whole name; text
    without wildcard characters matches anywhere in the name.
    Regex: re.search semantics (use ^ and $ to anchor). Raises re.error.
    """
    if not name_filter:
        return None
    if mode == FILTER_REGEX:
        return re.compile(name_filter, re.IGNORECASE)
    if not any(c in name_filter for c in "*?["):
        name_filter = "*%s*" % name_filter
    return re.compile(fnmatch.translate(name_filter), re.IGNORECASE)


def scan_dir(path, show_dirs=False):
    """Return unsorted [Entry] for the files (and directories) in path."""
    entries = []
    try:
        for e in os.scandir(path):
            if e.is_dir():
                if show_dirs:
                    entries.append(Entry(e.name, 0, e.stat().st_mtime, True))
            elif e.is_file():
                st = e.stat()
                entries.append(Entry(e.name, st.st_size, st.st_mtime, False))
    except OSError:
        return []
    return entries


def filter_entries(entries, name_filter, mode):
    """Wildcard/regex filtering of the shown text; applies to files only so
    directories stay navigable."""
    pattern = compile_filter(name_filter, mode)
    if pattern is None:
        return entries
    return [e for e in entries if e.isDir or pattern.search(e.text)]


def human_size(n):
    if n < 1024:
        return "%d B" % n
    for unit in ("KB", "MB", "GB"):
        n /= 1024.0
        if n < 1024 or unit == "GB":
            return "%.1f %s" % (n, unit)
