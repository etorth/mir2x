"""Source code highlighted with Pygments (skipped if Pygments is unavailable)."""

from PySide6.QtGui import QColor, QFont, QTextCharFormat

from .base import Previewer, truncated_note
from .options import Option, TEXT_ENCODING, WORD_WRAP
from .views import TextView, set_colors

try:
    from pygments.lexers import get_lexer_for_filename, get_lexer_for_mimetype, guess_lexer
    from pygments.lexers.special import TextLexer
    from pygments.styles import get_all_styles, get_style_by_name
    from pygments.token import Token
    from pygments.util import ClassNotFound
except ImportError:
    get_lexer_for_filename = None

THEME = Option("code/theme", "Theme", "default",
               tuple((s, s) for s in sorted(get_all_styles())) if get_lexer_for_filename else ())


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
    name = "Code"
    options = (THEME, WORD_WRAP, TEXT_ENCODING)

    def __init__(self):
        self.formats = {}
        self.style = None

    def match(self, f):
        return f.isText() and find_lexer(f) is not None

    def show(self, f, panel):
        if get_lexer_for_filename is None:
            raise ValueError("Pygments is not installed")
        text, truncated = f.readText()
        # chosen in Config > Preview Type for a file it doesn't recognize: guess from the content
        lexer = find_lexer(f, stripnl=False, ensurenl=False) or guess_lexer(text, stripnl=False, ensurenl=False)
        style = get_style_by_name(THEME.value())
        if style is not self.style:
            self.style, self.formats = style, {}

        runs = []
        for ttype, value in lexer.get_tokens(text):
            fmt = self.charFormat(ttype)
            if runs and runs[-1][1] is fmt:
                runs[-1][0].append(value)
            else:
                runs.append(([value], fmt))
        view = panel.view(TextView)
        view.setRuns(("".join(parts), fmt) for parts, fmt in runs)
        view.setWordWrap(WORD_WRAP.value())
        background = QColor(style.background_color or "#ffffff")
        color = style.style_for_token(Token)["color"]
        set_colors(view, background, "#" + color if color else "black" if background.lightness() > 128 else "white")
        return [lexer.name] + truncated_note(truncated)

    def charFormat(self, ttype):
        fmt = self.formats.get(ttype)
        if fmt is None:
            style = self.style.style_for_token(ttype)
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
