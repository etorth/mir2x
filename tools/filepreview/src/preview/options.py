"""Persistent preview options, shown in Config > Preview Options for the
previewers that list them in their `options`."""

import locale

from PySide6.QtCore import QSettings

_settings = None


def settings():
    global _settings
    if _settings is None:
        _settings = QSettings("mir2x", "filepreview")
    return _settings


class Option:
    """A toggle (bool default) or a choice among (value, label) pairs, saved as preview/<key>."""

    def __init__(self, key, label, default, choices=()):
        self.key = "preview/" + key
        self.label = label
        self.default = default
        self.choices = choices

    def value(self):
        value = settings().value(self.key, self.default)
        if isinstance(self.default, bool):
            return value in (True, "true")
        return value if any(value == v for v, _ in self.choices) else self.default

    def setValue(self, value):
        settings().setValue(self.key, value)


WORD_WRAP = Option("text/wordWrap", "Word Wrap", False)

# libmagic reports 8-bit text that isn't UTF-8 as iso-8859-1 or unknown-8bit: pick its encoding
TEXT_ENCODING = Option("text/encoding", "Non-UTF-8 Text Encoding", "system", (
    ("system", "System (%s)" % locale.getencoding()),
    ("gbk", "GBK"),
    ("big5", "Big5"),
    ("shift_jis", "Shift-JIS"),
    ("euc_kr", "EUC-KR"),
    ("cp1252", "Windows-1252"),
))
