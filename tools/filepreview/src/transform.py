"""User-defined filename transform: compiling the code and applying it to entries."""

import pathlib
import reprlib
import traceback


TRANSFORM_TEMPLATE = '''\
import pathlib


def transform(path: pathlib.Path):
    # Called for every file and directory in the list (never for "..").
    #   True           -> show the original name
    #   False          -> hide the entry
    #   non-empty str  -> show this text instead of the name
    # Anything else, or an exception, shows the original name in pink.
    return True
'''


TRANSFORM_FILENAME = "<filename transform>"


def compile_transform(code):
    """Run code and return the function it defines as `transform`; raises on error."""
    namespace = {"__name__": "filename_transform"}
    exec(compile(code, TRANSFORM_FILENAME, "exec"), namespace)
    func = namespace.get("transform")
    if not callable(func):
        raise ValueError("the code must define a function named 'transform'")
    return func


def format_error(exc):
    """One-line description; includes the line number inside the transform code."""
    text = "%s: %s" % (type(exc).__name__, exc)
    frames = [fr for fr in traceback.extract_tb(exc.__traceback__) if fr.filename == TRANSFORM_FILENAME]
    if frames:
        text += " (line %d)" % frames[-1].lineno
    return text


def apply_transform(entries, directory, func):
    """Return the entries func keeps, with text/error updated."""
    kept = []
    for e in entries:
        try:
            result = func(pathlib.Path(directory, e.name))
        except Exception as exc:
            e.error = "transform raised " + format_error(exc)
        else:
            if result is False:
                continue
            if isinstance(result, str) and result:
                e.text = result
            elif result is not True:
                e.error = "transform returned %s; expected True, False or a non-empty str" % reprlib.repr(result)
        kept.append(e)
    return kept
