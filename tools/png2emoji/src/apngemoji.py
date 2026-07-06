#!/usr/bin/env python3

import argparse
import re
import subprocess
import sys
from pathlib import Path


# needs apngasm from: https://github.com/apngasm/apngasm
# the legacy apngasm looks buggy and can not setup correct frame delay
#
# but the build of apngasm/apngasm is not easier
# this script also support --apngasm-python which can be pip installed
#
# also after APNG generated, we can use apngopt to optimize the APNG file size:
# https://github.com/jindongyi011039/apngopt.git


def natural_sort_key(path):
    return [int(part) if part.isdigit() else part for part in re.split(r"(\d+)", path.name)]


def find_frames(index, input_dir):
    frames_by_index = {}
    for frame_path in sorted(input_dir.glob(f"{index}_*.png"), key=natural_sort_key):
        match = re.fullmatch(rf"{index}_(\d+)\.png", frame_path.name)
        if not match:
            print(f"Warning: index {index} has unexpected frame file name: {frame_path}", file=sys.stderr)
            continue

        frames_by_index[int(match.group(1))] = frame_path

    return frames_by_index


def print_frame_table(index, input_dir, output_path, frames_by_index):
    print(f"creating {output_path}, found frames:")
    for frame_index in range(1, max(frames_by_index) + 1):
        expected_frame_path = input_dir / f"{index}_{frame_index}.png"
        if frame_index in frames_by_index:
            print(f"{frames_by_index[frame_index]} found")
        else:
            print(f"{expected_frame_path} missing")


def get_frame_delay_ms(fps):
    return round(1000 / fps)


def get_output_filename(output_filename_format, index):
    try:
        output_filename = output_filename_format % index
    except (TypeError, ValueError) as exc:
        raise ValueError(f"invalid --output-filename-format {output_filename_format!r}: {exc}") from exc

    if not output_filename or output_filename in (".", "..") or Path(output_filename).name != output_filename or "\\" in output_filename:
        raise ValueError(f"invalid --output-filename-format {output_filename_format!r}: result must be a filename, not a path")

    return output_filename


def generate_apng_with_cli(frames, output_path, frame_delay_ms, apngasm_path):
    result = subprocess.run(
        [
            str(apngasm_path),
            "-o",
            str(output_path),
            *[str(frame) for frame in frames],
            "-F",
            "-d",
            str(frame_delay_ms),
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        if result.stdout:
            print(result.stdout, end="")
        if result.stderr:
            print(result.stderr, end="", file=sys.stderr)
        result.check_returncode()


def generate_apng_with_python(frames, output_path, frame_delay_ms):
    from apngasm_python.apngasm import APNGAsmBinder

    apngasm = APNGAsmBinder()
    try:
        for frame in frames:
            apngasm.add_frame_from_file(file_path=str(frame), delay_num=frame_delay_ms, delay_den=1000)
        apngasm.assemble(str(output_path))
    finally:
        apngasm.reset()


def generate_apng(index, input_dir, output_dir, output_filename_format, fps, apngasm_path, use_apngasm_python):
    frames_by_index = find_frames(index, input_dir)
    frames = [frames_by_index[frame_index] for frame_index in sorted(frames_by_index)]
    output_path = output_dir / get_output_filename(output_filename_format, index)

    print_frame_table(index, input_dir, output_path, frames_by_index)

    frame_delay_ms = get_frame_delay_ms(fps)
    if use_apngasm_python:
        generate_apng_with_python(frames, output_path, frame_delay_ms)
    else:
        generate_apng_with_cli(frames, output_path, frame_delay_ms, apngasm_path)
    print("done")


def parse_args():
    parser = argparse.ArgumentParser(description="Generate APNG emoji files from indexed PNG frames.")
    parser.add_argument("--input-dir", required=True, type=Path, help="Directory containing <index>_<frame>.png files.")
    parser.add_argument("--output-dir", required=True, type=Path, help="Directory to write generated APNG files.")
    parser.add_argument("--fps", type=float, default=5, help="Animation frames per second. Defaults to 5.")
    parser.add_argument("--apngasm-path", default="apngasm", help="Path to apngasm. Defaults to apngasm from PATH.")
    parser.add_argument("--apngasm-python", action="store_true", help="Use the apngasm-python package instead of apngasm CLI.")
    parser.add_argument("--max-index", type=int, default=1000, help="Maximum emoji index to scan. Defaults to 1000.")
    parser.add_argument(
        "--output-filename-format",
        default="%d.png",
        help='Printf-style output filename format using the emoji index. Defaults to "%%d.png".',
    )
    return parser.parse_args()


def main():
    args = parse_args()

    if not args.input_dir.is_dir():
        print(f"Error: {args.input_dir} is not a directory", file=sys.stderr)
        return 1

    if args.fps <= 0:
        print(f"Error: --fps must be greater than 0, got {args.fps}", file=sys.stderr)
        return 1

    frame_delay_ms = get_frame_delay_ms(args.fps)
    if frame_delay_ms <= 0:
        print(f"Error: --fps is too high; computed frame delay is {frame_delay_ms} ms", file=sys.stderr)
        return 1

    if args.max_index < 0:
        print(f"Error: --max-index must be greater than or equal to 0, got {args.max_index}", file=sys.stderr)
        return 1

    try:
        get_output_filename(args.output_filename_format, 0)
    except ValueError as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    args.output_dir.mkdir(parents=True, exist_ok=True)

    for index in range(args.max_index + 1):
        if (args.input_dir / f"{index}_1.png").is_file():
            generate_apng(
                index,
                args.input_dir,
                args.output_dir,
                args.output_filename_format,
                args.fps,
                args.apngasm_path,
                args.apngasm_python,
            )

    return 0


if __name__ == "__main__":
    sys.exit(main())
