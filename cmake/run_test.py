#!/usr/bin/env python3
# mir2x CTest wrapper - invoked by every test registered via mir2x_add_unit_test()/
# mir2x_add_integration_test() (see cmake/Mir2xTest.cmake), not meant to be run standalone
# as part of the test suite itself, but it's a plain script, so nothing stops you from
# using it directly to (re)generate a gold file (see --update-gold below).
#
# runs the given command; if --gold is given, diffs the command's stdout against that file
# (printing a unified diff on mismatch) and fails if they differ or if the gold file doesn't
# exist yet. CTest only looks at this wrapper's own exit code, so this is what actually
# turns a stdout mismatch into a CTest-visible test failure.
#
# usage (as wired up by cmake/Mir2xTest.cmake, one per CTest test):
#   run_test.py --name <test-name> [--gold <path>] -- <command> [args...]
#
# to (re)create a gold file from a test's current actual output:
#   run_test.py --update-gold --gold <path> -- <command> [args...]

import argparse
import difflib
import os
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--name", default="test", help="test name, used only in messages")
    parser.add_argument("--gold", help="path to expected stdout; omit to skip the diff and only check exit code")
    parser.add_argument("--update-gold", action="store_true",
            help="instead of comparing, overwrite --gold with this run's actual stdout")
    parser.add_argument("command", nargs=argparse.REMAINDER, help="command to run, after --")
    args = parser.parse_args()

    command = args.command
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        parser.error("no command given (expected: -- <command> [args...])")

    try:
        proc = subprocess.run(command, capture_output=True, text=True, timeout=600)
    except Exception as e:
        print(f"[{args.name}] failed to launch: {e}", file=sys.stderr)
        return 1

    if proc.stdout:
        sys.stdout.write(proc.stdout)
    if proc.stderr:
        sys.stderr.write(proc.stderr)

    if proc.returncode != 0:
        print(f"[{args.name}] exited with code {proc.returncode}", file=sys.stderr)
        return proc.returncode

    if not args.gold:
        return 0

    if args.update_gold:
        with open(args.gold, "w") as f:
            f.write(proc.stdout)
        print(f"[{args.name}] wrote {args.gold}", file=sys.stderr)
        return 0

    if not os.path.exists(args.gold):
        print(f"[{args.name}] gold file not found: {args.gold}", file=sys.stderr)
        print(f"[{args.name}] (rerun with --update-gold to create it from this run's output)", file=sys.stderr)
        return 1

    with open(args.gold, "r") as f:
        expected = f.read()

    if expected != proc.stdout:
        print(f"[{args.name}] stdout differs from {args.gold}", file=sys.stderr)
        diff = difflib.unified_diff(
            expected.splitlines(keepends=True),
            proc.stdout.splitlines(keepends=True),
            fromfile="gold", tofile="actual")
        sys.stderr.writelines(diff)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
