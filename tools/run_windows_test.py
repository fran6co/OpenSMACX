#!/usr/bin/env python3

import argparse
import os
from pathlib import Path
import subprocess

from owned_wine_prefix import prepare_owned_wine_prefix, stop_owned_wine_prefix
from wine_runtime import find_wine


def main():
    parser = argparse.ArgumentParser(
        description="Run a Windows test executable locally")
    parser.add_argument("--executable", required=True)
    parser.add_argument("--wine")
    parser.add_argument("--wine-prefix")
    args = parser.parse_args()

    executable = Path(args.executable).expanduser().resolve()
    if not executable.is_file():
        parser.error(f"test executable not found: {executable}")

    if os.name == "nt":
        raise SystemExit(subprocess.run([str(executable)]).returncode)
    if not args.wine_prefix:
        parser.error("a dedicated --wine-prefix is required")

    wine_prefix = Path(args.wine_prefix).expanduser().absolute()
    wine = find_wine(args.wine)
    prepare_owned_wine_prefix(wine_prefix, wine)
    environment = os.environ.copy()
    environment["WINEPREFIX"] = str(wine_prefix)
    try:
        result = subprocess.run([wine, str(executable)], env=environment).returncode
    finally:
        stop_owned_wine_prefix(wine_prefix, wine)
    raise SystemExit(result)


if __name__ == "__main__":
    main()
