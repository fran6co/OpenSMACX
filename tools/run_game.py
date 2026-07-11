#!/usr/bin/env python3

import argparse
import os
from pathlib import Path
import subprocess
import sys

from setup_game import find_wine


def main():
    parser = argparse.ArgumentParser(description="Run a staged OpenSMACX game")
    parser.add_argument("--game-dir", required=True)
    parser.add_argument("--wine")
    parser.add_argument("--wine-prefix")
    args = parser.parse_args()

    game_dir = Path(args.game_dir).expanduser().resolve()
    executable = game_dir / "terranx_opensmacx.exe"
    if not executable.is_file():
        parser.error(f"patched executable not found: {executable}")

    environment = os.environ.copy()
    if args.wine_prefix:
        environment["WINEPREFIX"] = str(Path(args.wine_prefix).expanduser().resolve())

    if os.name == "nt":
        command = [str(executable)]
    else:
        wine = find_wine(args.wine)
        wine_path = Path(wine)
        if sys.platform == "darwin" and len(wine_path.parents) >= 5:
            application = wine_path.parents[4]
            if application.suffix == ".app":
                command = [
                    "open", "-n", "-a", application.stem,
                    "--args", str(executable),
                ]
            else:
                command = [wine, str(executable)]
        else:
            command = [wine, str(executable)]
    raise SystemExit(subprocess.run(command, cwd=str(game_dir), env=environment).returncode)


if __name__ == "__main__":
    main()
