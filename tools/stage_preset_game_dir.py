#!/usr/bin/env python3
"""Mirror the shared master game directory into a per-preset staging copy.

The master directory under `.opensmacx/game` holds the extracted proprietary
install plus the hash-pinned input executables and stays read-only during
verification batches. Each build preset stages its own mirrored copy so Debug
and Release batches can patch, launch, and smoke-test concurrently without
overwriting each other's `terranx_hybrid.exe` and `OpenSMACX.dll`. Those two
patcher-owned outputs are never mirrored, files the game writes into a staged
copy are never deleted, and unchanged files are skipped by size and timestamp.
On Darwin the initial copy uses APFS clones so the 700 MB mirror costs no
duplicated blocks.
"""

import argparse
from pathlib import Path
import shutil
import subprocess
import sys

from local_artifact import require_local_artifact_path


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SOURCE = REPO_ROOT / ".opensmacx" / "game"

PATCHER_OWNED_NAMES = ("terranx_hybrid.exe", "OpenSMACX.dll")
PATCHER_TEMPORARY_PREFIXES = tuple(
    f".{name}" for name in PATCHER_OWNED_NAMES)


def is_patcher_owned(name):
    if name in PATCHER_OWNED_NAMES:
        return True
    return any(name.startswith(prefix) for prefix in PATCHER_TEMPORARY_PREFIXES)


def clone_file(source, destination):
    destination.parent.mkdir(parents=True, exist_ok=True)
    if sys.platform == "darwin":
        result = subprocess.run(
            ["cp", "-c", str(source), str(destination)],
            capture_output=True)
        if result.returncode == 0:
            return
    shutil.copy2(source, destination)


def needs_copy(source, destination):
    if not destination.is_file():
        return True
    source_stat = source.stat()
    destination_stat = destination.stat()
    if source_stat.st_size != destination_stat.st_size:
        return True
    return abs(source_stat.st_mtime - destination_stat.st_mtime) > 1.0


def mirror(source_dir, destination_dir):
    copied = 0
    skipped = 0
    excluded = 0
    for source in sorted(source_dir.rglob("*")):
        if source.is_symlink() or not source.is_file():
            continue
        relative = source.relative_to(source_dir)
        if is_patcher_owned(relative.name):
            excluded += 1
            continue
        destination = destination_dir / relative
        if needs_copy(source, destination):
            clone_file(source, destination)
            copied += 1
        else:
            skipped += 1
    return copied, skipped, excluded


def main():
    parser = argparse.ArgumentParser(
        description="Mirror the master game directory into a preset staging copy")
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--destination", type=Path, required=True)
    args = parser.parse_args()

    source_dir = args.source.resolve()
    if not source_dir.is_dir():
        raise SystemExit(f"master game directory not found: {source_dir}")
    destination_dir = require_local_artifact_path(
        args.destination, "staged game directory")
    if (destination_dir == source_dir
            or destination_dir in source_dir.parents
            or source_dir in destination_dir.parents):
        raise SystemExit(
            "staged game directory must not overlap the master directory")

    destination_dir.mkdir(parents=True, exist_ok=True)
    copied, skipped, excluded = mirror(source_dir, destination_dir)
    print(
        f"stage-preset-game: copied {copied}, unchanged {skipped}, "
        f"patcher-owned excluded {excluded} -> {destination_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
