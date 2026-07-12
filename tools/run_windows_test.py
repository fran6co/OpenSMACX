#!/usr/bin/env python3

import argparse
import os
from pathlib import Path
import subprocess

from setup_game import find_wine


def main():
    parser = argparse.ArgumentParser(
        description="Run a Windows test executable locally")
    parser.add_argument("--executable", required=True)
    parser.add_argument("--wine")
    args = parser.parse_args()

    executable = Path(args.executable).expanduser().resolve()
    if not executable.is_file():
        parser.error(f"test executable not found: {executable}")

    command = ([str(executable)] if os.name == "nt" else
               [find_wine(args.wine), str(executable)])
    raise SystemExit(subprocess.run(command).returncode)


if __name__ == "__main__":
    main()
