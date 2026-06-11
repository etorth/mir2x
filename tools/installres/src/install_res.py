#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


CLIENT_RESOURCE_PACKS = (
    ("map", "map/mapbin.zsdb"),
    ("sound/seff", "sound/seff.zsdb"),
    ("sound/bgm", "sound/bgm.zsdb"),
    ("emoji", "emoji/emoji.zsdb"),
    ("font", "font/fontex.zsdb"),
    ("texture/proguse", "texture/proguse.zsdb"),
    ("texture/item", "texture/item.zsdb"),
    ("texture/npc", "texture/npc.zsdb"),
    ("texture/weapon", "texture/weapon.zsdb"),
    ("texture/map", "texture/map.zsdb"),
    ("texture/monster", "texture/monster.zsdb"),
    ("texture/helmet", "texture/helmet.zsdb"),
    ("texture/selectchar", "texture/selectchar.zsdb"),
    ("texture/magic", "texture/magic.zsdb"),
    ("texture/hair", "texture/hair.zsdb"),
    ("texture/equip", "texture/equip.zsdb"),
    ("texture/hero", "texture/hero.zsdb"),
)

SERVER_RESOURCE_PACKS = (
    ("map", "map/mapbin.zsdb"),
)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Pack resource directories into .zsdb databases.",
    )
    parser.add_argument(
        "--zsdbmaker",
        required=True,
        help="Path to the zsdbmaker executable.",
    )
    parser.add_argument(
        "--input-dir",
        required=True,
        help="Directory containing unpacked resource files.",
    )
    parser.add_argument(
        "--output-dir",
        required=True,
        help="Directory where packed .zsdb files will be created.",
    )
    install_mode = parser.add_mutually_exclusive_group(required=True)
    install_mode.add_argument(
        "--install-server",
        action="store_true",
        help="Pack server resources.",
    )
    install_mode.add_argument(
        "--install-client",
        action="store_true",
        help="Pack client resources.",
    )
    parser.add_argument(
        "--ignore-zsdbmaker-error",
        action="store_true",
        help="Continue packing if zsdbmaker returns an error for a resource pack.",
    )
    return parser.parse_args()


def resolve_zsdbmaker(zsdbmaker):
    zsdbmaker_path = Path(zsdbmaker)
    if zsdbmaker_path.exists() or zsdbmaker_path.parent != Path("."):
        return str(zsdbmaker_path.resolve())

    resolved = shutil.which(zsdbmaker)
    if resolved is None:
        raise FileNotFoundError(f"can't find zsdbmaker executable: {zsdbmaker}")

    return resolved


def main():
    args = parse_args()

    zsdbmaker = resolve_zsdbmaker(args.zsdbmaker)
    input_dir = Path(args.input_dir).resolve()
    output_dir = Path(args.output_dir).resolve()
    resource_packs = SERVER_RESOURCE_PACKS if args.install_server else CLIENT_RESOURCE_PACKS

    for input_relpath, _ in resource_packs:
        pack_input_dir = input_dir / input_relpath
        if not pack_input_dir.is_dir():
            raise FileNotFoundError(f"missing resource directory: {pack_input_dir}")

    for input_relpath, output_relpath in resource_packs:
        pack_input_dir = input_dir / input_relpath
        pack_output_path = (output_dir / output_relpath).resolve()
        pack_output_path.parent.mkdir(parents=True, exist_ok=True)

        print(f"Packing {pack_input_dir} -> {pack_output_path}")
        result = subprocess.run(
            [
                zsdbmaker,
                f"--input-dir={pack_input_dir}",
                f"--create-db={pack_output_path}",
            ],
            check=False,
        )
        if result.returncode != 0:
            if not args.ignore_zsdbmaker_error:
                raise subprocess.CalledProcessError(result.returncode, result.args)

            print(
                f"Warning: zsdbmaker failed with exit code {result.returncode}: "
                f"{pack_input_dir} -> {pack_output_path}",
                file=sys.stderr,
            )


if __name__ == "__main__":
    main()
