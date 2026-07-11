#!/usr/bin/env python3

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys


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


def find_wine(value):
    if value:
        candidate = Path(value).expanduser()
        if candidate.is_file():
            return str(candidate.resolve())
        resolved = shutil.which(value)
        if resolved:
            return resolved
        raise RuntimeError(f"Wine executable not found: {value}")

    configured = os.environ.get("WINE")
    if configured:
        return find_wine(configured)

    resolved = shutil.which("wine")
    if resolved:
        return resolved

    if sys.platform == "darwin":
        for application in ("Wine Staging", "Wine Stable", "Wine Devel"):
            candidate = Path(
                f"/Applications/{application}.app/Contents/Resources/wine/bin/wine")
            if candidate.is_file():
                return str(candidate)

    raise RuntimeError("Wine was not found; install it or pass --wine")


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
    support = output / "__support" / "app"
    original = output / "terranx.exe"
    pracx = support / "terranx_PRACX.exe"
    if not original.is_file() or not pracx.is_file():
        raise RuntimeError("the extracted installer does not contain the expected game executables")

    original_copy = output / "terranx_original.exe"
    if not original_copy.exists():
        shutil.copy2(original, original_copy)

    for source in support.iterdir():
        if source.is_file() and not source.name.startswith(("terranx_", "terran_")):
            shutil.copy2(source, output / source.name)
    shutil.copy2(pracx, original)
    shutil.copy2(support / "terran_PRACX.exe", output / "terran.exe")


def main():
    parser = argparse.ArgumentParser(
        description="Extract a local GOG installation and apply OpenSMACX")
    parser.add_argument("--installer", required=True)
    parser.add_argument("--dll", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--wine")
    parser.add_argument("--reuse", action="store_true",
                        help="Reuse an already extracted output directory")
    parser.add_argument("--without-pracx", action="store_true",
                        help="Patch the original executable without bundled PRACX files")
    args = parser.parse_args()

    installer = Path(args.installer).expanduser().resolve()
    dll = Path(args.dll).expanduser().resolve()
    output = Path(args.output).expanduser().resolve()
    if not installer.is_file():
        parser.error(f"installer not found: {installer}")
    if not dll.is_file():
        parser.error(f"OpenSMACX DLL not found: {dll}")

    installer_hash = sha256(installer)
    if installer_hash not in SUPPORTED_INSTALLERS:
        parser.error(f"unsupported installer SHA-256: {installer_hash}")

    if not args.reuse or not (output / "terranx_original.exe").is_file():
        extract_installer(installer, output)
    if not args.without_pracx:
        stage_pracx(output)

    source_exe = output / "terranx.exe"
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
        command.extend(["--wine", find_wine(args.wine)])
    subprocess.run(command, check=True)

    print(f"Playable game staged at {output / 'terranx_opensmacx.exe'}")


if __name__ == "__main__":
    main()
