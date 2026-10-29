"""Source code highlighted with Pygments (skipped if Pygments is unavailable)."""

from PySide6.QtGui import QColor, QFont, QTextCharFormat

from .base import Previewer, truncated_note
from .views import TextView

try:
    from pygments.lexers import get_lexer_for_filename, get_lexer_for_mimetype
    from pygments.lexers.special import TextLexer
    from pygments.styles import get_style_by_name
    from pygments.util import ClassNotFound
except ImportError:
    get_lexer_for_filename = None

STYLE = "default"


def find_lexer(f, **options):
    """Lexer by file name (more specific: libmagic calls C++ "text/x-c"),
    else by the detected type (e.g. extensionless "#!/usr/bin/env python3")."""
    if get_lexer_for_filename is None:
        return None
    for find, key in ((get_lexer_for_filename, f.name), (get_lexer_for_mimetype, f.mime)):
        try:
            lexer = find(key, **options)
        except ClassNotFound:
            continue
        if not isinstance(lexer, TextLexer):
            return lexer
    return None


class CodePreviewer(Previewer):
    def __init__(self):
        self.formats = {}

    def match(self, f):
        return f.isText() and find_lexer(f) is not None

    def show(self, f, panel):
        lexer = find_lexer(f, stripnl=False, ensurenl=False)
        text, truncated = f.readText()

        runs = []
        for ttype, value in lexer.get_tokens(text):
            fmt = self.charFormat(ttype)
            if runs and runs[-1][1] is fmt:
                runs[-1][0].append(value)
            else:
                runs.append(([value], fmt))
        panel.view(TextView).setRuns(("".join(parts), fmt) for parts, fmt in runs)
        return [lexer.name] + truncated_note(truncated)

    def charFormat(self, ttype):
        fmt = self.formats.get(ttype)
        if fmt is None:
            style = get_style_by_name(STYLE).style_for_token(ttype)
            fmt = QTextCharFormat()
            if style["color"]:
                fmt.setForeground(QColor("#" + style["color"]))
            if style["bgcolor"]:
                fmt.setBackground(QColor("#" + style["bgcolor"]))
            if style["bold"]:
                fmt.setFontWeight(QFont.Bold)
            if style["italic"]:
                fmt.setFontItalic(True)
            if style["underline"]:
                fmt.setFontUnderline(True)
            self.formats[ttype] = fmt
        return fmt
