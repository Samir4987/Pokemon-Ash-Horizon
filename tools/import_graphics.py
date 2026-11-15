#!/usr/bin/env python3
"""Import graphics from the two user-provided archives into local game assets.

Usage:
  python tools/import_graphics.py "Generation 9 Pack v3.3.8(1).zip" "Downloads(1).zip"

The copyrighted art is deliberately not committed by this script; it is copied
from the archives on the developer's machine into the ignored local assets tree.
"""
from __future__ import annotations

import argparse
import io
import shutil
import tarfile
import zipfile
from pathlib import Path


def find_prefix(names: list[str], suffix: str) -> str:
    matches = [name for name in names if name.endswith(suffix)]
    if not matches:
        raise RuntimeError(f"Could not find {suffix!r} inside Generation 9 Pack")
    return matches[0]


def copy_zip_tree(zf: zipfile.ZipFile, prefix: str, destination: Path) -> int:
    count = 0
    for info in zf.infolist():
        if info.is_dir() or not info.filename.startswith(prefix):
            continue
        relative = info.filename[len(prefix):]
        if not relative or "/" in relative:
            continue
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        with zf.open(info) as source, target.open("wb") as output:
            shutil.copyfileobj(source, output)
        count += 1
    return count


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("generation9_zip", type=Path)
    parser.add_argument("downloads_zip", type=Path)
    parser.add_argument("--output", type=Path, default=Path("assets"))
    args = parser.parse_args()

    output = args.output
    counts: dict[str, int] = {}

    with zipfile.ZipFile(args.generation9_zip) as gen9:
        names = gen9.namelist()
        root = find_prefix(names, "Graphics/Pokemon/Front/")
        root = root[:root.index("Graphics/Pokemon/Front/")]
        base = root + "Graphics/"

        mappings = {
            "Graphics/Pokemon/Front/": output / "pokemon/battle/front",
            "Graphics/Pokemon/Back/": output / "pokemon/battle/back",
            "Graphics/Characters/Followers/": output / "pokemon/followers",
            "Graphics/Characters/Followers shiny/": output / "pokemon/followers_shiny",
            "Graphics/Battle animations/": output / "battle/balls",
        }
        for suffix, target in mappings.items():
            counts[str(target)] = copy_zip_tree(gen9, base + suffix, target)

    with zipfile.ZipFile(args.downloads_zip) as downloads:
        nested = next((n for n in downloads.namelist()
                       if n.lower().endswith("overworld.tar") and
                       not n.endswith("/")), None)
        if not nested:
            raise RuntimeError("Could not find nested overworld.tar in Downloads archive")
        raw_tar = downloads.read(nested)
        with tarfile.open(fileobj=io.BytesIO(raw_tar), mode="r:*") as archive:
            for direction in ("down", "up", "left", "right"):
                target = output / "pokemon/overworld" / direction
                target.mkdir(parents=True, exist_ok=True)
                copied = 0
                prefix = f"pokemon/overworld/{direction}/"
                for member in archive.getmembers():
                    if not member.isfile() or not member.name.startswith(prefix):
                        continue
                    relative = member.name[len(prefix):]
                    if "/" in relative or not relative.lower().endswith(".png"):
                        continue
                    source = archive.extractfile(member)
                    if source is None:
                        continue
                    with source, (target / relative).open("wb") as dest:
                        shutil.copyfileobj(source, dest)
                    copied += 1
                counts[str(target)] = copied

    print("Graphics import complete:")
    for folder, count in counts.items():
        print(f"  {folder}: {count} PNG files")
    print("\nTrainer battle sprites and move-specific battle animations were not "
          "present in the inspected source folders. The engine keeps safe "
          "fallbacks for those missing assets.")
    print("Do not distribute imported artwork unless its source license permits it.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
