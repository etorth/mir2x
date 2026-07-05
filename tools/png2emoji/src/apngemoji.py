#!/usr/bin/env python3

import argparse
import re
import subprocess
import sys
from pathlib import Path


# needs apngasm from: https://github.com/apngasm/apngasm
# the legacy apngasm looks buggy and can not setup correct frame delay
#
# install
#
#       sudo add-apt-repository ppa:zero-tsuki/ppa
#       sudo apt-get update
#       sudo apt-get install apngasm

DEFAULT_MAX_INDEX = 1000


def natural_sort_key(path):
    return [int(part) if part.isdigit() else part for part in re.split(r"(\d+)", path.name)]


def warn_if_frames_are_not_continuous(index, frames):
    expected_frame = 1

    for frame_path in frames:
        match = re.fullmatch(rf"{index}_(\d+)\.png", frame_path.name)
        if not match:
            print(f"Warning: index {index} has unexpected frame file name: {frame_path.name}", file=sys.stderr)
            continue

        frame = int(match.group(1))
        if frame != expected_frame:
            print(
                f"Warning: index {index} frame sequence is not continuous: "
                f"expected {index}_{expected_frame}.png but found {frame_path.name}",
                file=sys.stderr,
            )
            expected_frame = frame

        expected_frame += 1


def generate_apng(index, input_dir, output_dir, fps, apngasm_path):
    frames = sorted(input_dir.glob(f"{index}_*.png"), key=natural_sort_key)
    warn_if_frames_are_not_continuous(index, frames)

    output_path = output_dir / f"out_{index}.png"
    frame_delay = round(1000 / fps)
    subprocess.run(
        [
            str(apngasm_path),
            "-o",
            str(output_path),
            *[str(frame) for frame in frames],
            "-F",
            "-d",
            str(frame_delay),
        ],
        check=True,
    )


def parse_args():
    parser = argparse.ArgumentParser(description="Generate APNG emoji files from indexed PNG frames.")
    parser.add_argument("--input-dir", required=True, type=Path, help="Directory containing <index>_<frame>.png files.")
    parser.add_argument("--output-dir", required=True, type=Path, help="Directory to write generated APNG files.")
    parser.add_argument("--fps", type=float, default=5, help="Animation frames per second. Defaults to 5.")
    parser.add_argument("--apngasm-path", default="apngasm", help="Path to apngasm. Defaults to apngasm from PATH.")
    parser.add_argument("--max-index", type=int, default=DEFAULT_MAX_INDEX, help="Maximum emoji index to scan. Defaults to 1000.")
    return parser.parse_args()


def main():
    args = parse_args()

    if not args.input_dir.is_dir():
        print(f"Error: {args.input_dir} is not a directory", file=sys.stderr)
        return 1

    if args.fps <= 0:
        print(f"Error: --fps must be greater than 0, got {args.fps}", file=sys.stderr)
        return 1

    if args.max_index < 0:
        print(f"Error: --max-index must be greater than or equal to 0, got {args.max_index}", file=sys.stderr)
        return 1

    args.output_dir.mkdir(parents=True, exist_ok=True)

    for index in range(args.max_index + 1):
        if (args.input_dir / f"{index}_1.png").is_file():
            generate_apng(index, args.input_dir, args.output_dir, args.fps, args.apngasm_path)

    return 0


if __name__ == "__main__":
    sys.exit(main())
