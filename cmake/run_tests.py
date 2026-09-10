#!/usr/bin/env python3
# mir2x test runner - invoked by the `test` CMake target (see cmake/Mir2xTest.cmake), not
# meant to be run standalone, but it's a plain script so nothing stops you from doing so.
#
# reads every *.json descriptor under --manifest-dir (one file per test, written by
# mir2x_add_unit_test()/mir2x_add_integration_test() at configure time), runs each test's
# command, and if the descriptor names a "gold" file, diffs the test's stdout against it.
#
# exit code is 0 iff every test exits successfully and (when a gold file is registered)
# produces byte-for-byte identical stdout; otherwise non-zero.

import argparse
import difflib
import glob
import json
import os
import subprocess
import sys


def run_one(spec, update_gold):
    name = spec["name"]
    command = spec["command"]
    workdir = spec.get("workdir") or None
    gold = spec.get("gold")

    print(f"--- {name}: {' '.join(command)}")
    try:
        proc = subprocess.run(command, cwd=workdir, capture_output=True, text=True, timeout=600)
    except Exception as e:
        print(f"[FAIL] {name}: failed to launch: {e}")
        return False

    if proc.returncode != 0:
        print(f"[FAIL] {name}: exited with code {proc.returncode}")
        if proc.stdout:
            sys.stdout.write(proc.stdout)
        if proc.stderr:
            sys.stderr.write(proc.stderr)
        return False

    if gold:
        if update_gold:
            with open(gold, "w") as f:
                f.write(proc.stdout)
            print(f"[GOLD] {name}: wrote {gold}")
            return True

        if not os.path.exists(gold):
            print(f"[FAIL] {name}: gold file not found: {gold}")
            print(f"       (rerun with --update-gold to create it from this run's output)")
            return False

        with open(gold, "r") as f:
            expected = f.read()

        if expected != proc.stdout:
            print(f"[FAIL] {name}: stdout differs from {gold}")
            diff = difflib.unified_diff(
                expected.splitlines(keepends=True),
                proc.stdout.splitlines(keepends=True),
                fromfile="gold", tofile="actual")
            sys.stdout.writelines(diff)
            return False

    print(f"[PASS] {name}")
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest-dir", required=True, help="directory of per-test *.json descriptors")
    parser.add_argument("--update-gold", action="store_true",
            help="instead of comparing, overwrite each test's gold file with this run's actual stdout")
    args = parser.parse_args()

    manifests = sorted(glob.glob(os.path.join(args.manifest_dir, "*.json")))
    if not manifests:
        print("no tests registered")
        return 0

    passed = 0
    failed_names = []
    for path in manifests:
        with open(path, "r") as f:
            spec = json.load(f)
        if run_one(spec, args.update_gold):
            passed += 1
        else:
            failed_names.append(spec["name"])
        print()

    total = len(manifests)
    print(f"{passed}/{total} tests passed")
    if failed_names:
        print("FAILED: " + ", ".join(failed_names))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
