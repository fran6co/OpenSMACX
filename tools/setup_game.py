#!/usr/bin/env python3

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys

from local_artifact import require_local_artifact_path
from owned_wine_prefix import validate_owned_wine_prefix
from wine_runtime import find_wine


SUPPORTED_INSTALLERS = {
    # GOG Planetary Pack 1.1 with PRACX/DDraw (77244).
    "80c820ca601d1eb13e07650022e61d7edef09e2afe211b14ee4a17107d492c8b",
}


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def extract_installer(installer, output):
    innoextract = shutil.which("innoextract")
    if not innoextract:
        raise RuntimeError("innoextract was not found in PATH")
    if output.exists() and any(output.iterdir()):
        raise RuntimeError(
            f"output directory is not empty: {output}; use --reuse to keep it")

    output.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [innoextract, "--extract", "--output-dir", str(output), str(installer)],
        check=True,
    )


def stage_pracx(output):
    support = require_local_artifact_path(
        output / "__support" / "app", "installer support directory")
    original = require_local_artifact_path(
        output / "terranx.exe", "game executable")
    pracx = require_local_artifact_path(
        support / "terranx_PRACX.exe", "PRACX executable")
    terran_pracx = require_local_artifact_path(
        support / "terran_PRACX.exe", "PRACX executable")
    if (not original.is_file() or not pracx.is_file()
            or not terran_pracx.is_file()):
        raise RuntimeError("the extracted installer does not contain the expected game executables")

    original_copy = require_local_artifact_path(
        output / "terranx_original.exe", "original game executable")
    terran = require_local_artifact_path(
        output / "terran.exe", "game executable")
    destinations = [original_copy, original, terran]
    support_copies = []
    for source in support.iterdir():
        if source.is_symlink():
            raise RuntimeError(f"PRACX support source must not be a symlink: {source}")
        if source.is_file() and not source.name.startswith(("terranx_", "terran_")):
            destination = require_local_artifact_path(
                output / source.name, "PRACX support file")
            support_copies.append((source, destination))
            destinations.append(destination)
    for destination in destinations:
        if destination.exists() and not destination.is_file():
            raise RuntimeError(
                f"game destination is not a regular file: {destination}")

    if not original_copy.exists():
        shutil.copy2(original, original_copy)
    for source, destination in support_copies:
        shutil.copy2(source, destination)
    shutil.copy2(pracx, original)
    shutil.copy2(terran_pracx, terran)


def main():
    parser = argparse.ArgumentParser(
        description="Extract a local GOG installation and apply OpenSMACX")
    parser.add_argument("--installer", required=True)
    parser.add_argument("--dll", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--wine")
    parser.add_argument("--wine-prefix",
                        help="Dedicated marker-protected Wine prefix for ImportAdder")
    parser.add_argument("--reuse", action="store_true",
                        help="Reuse an already extracted output directory")
    parser.add_argument("--without-pracx", action="store_true",
                        help="Patch the original executable without bundled PRACX files")
    args = parser.parse_args()

    installer = Path(args.installer).expanduser().resolve()
    dll = Path(args.dll).expanduser().resolve()
    try:
        output = require_local_artifact_path(args.output, "game output")
    except RuntimeError as error:
        parser.error(str(error))
    if not installer.is_file():
        parser.error(f"installer not found: {installer}")
    if not dll.is_file():
        parser.error(f"OpenSMACX DLL not found: {dll}")

    wine = None
    wine_prefix = None
    if os.name != "nt":
        if not args.wine_prefix:
            parser.error("a dedicated --wine-prefix is required")
        try:
            wine_prefix = require_local_artifact_path(
                args.wine_prefix, "Wine prefix")
            wine_prefix = validate_owned_wine_prefix(wine_prefix)
        except RuntimeError as error:
            parser.error(str(error))
        wine = find_wine(args.wine)

    installer_hash = sha256(installer)
    if installer_hash not in SUPPORTED_INSTALLERS:
        parser.error(f"unsupported installer SHA-256: {installer_hash}")

    reuse_original = require_local_artifact_path(
        output / "terranx_original.exe", "original game executable")
    if not args.reuse or not reuse_original.is_file():
        extract_installer(installer, output)
    if not args.without_pracx:
        stage_pracx(output)

    source_exe = require_local_artifact_path(
        output / "terranx.exe", "game executable")
    if not source_exe.is_file():
        raise RuntimeError(f"terranx.exe was not extracted to {output}")

    patcher = Path(__file__).resolve().with_name("opensmac_patcher.py")
    command = [
        sys.executable,
        str(patcher),
        "--exe", str(source_exe),
        "--dll", str(dll),
        "--output", str(output),
    ]
    if os.name != "nt":
        command.extend([
            "--wine", wine,
            "--wine-prefix", str(wine_prefix),
        ])
    subprocess.run(command, check=True)

    print(f"Playable game staged at {output / 'terranx_opensmacx.exe'}")


if __name__ == "__main__":
    main()
