#!/usr/bin/env python3

import argparse
import os
from pathlib import Path, PureWindowsPath
import secrets
import subprocess
import sys

from movie_skip import configure_intro_movie_skip, restore_intro_movie_config
from wine_runtime import find_wine


def main():
    parser = argparse.ArgumentParser(description="Run a staged OpenSMACX game")
    parser.add_argument("--game-dir", required=True)
    parser.add_argument("--executable", default="terranx_opensmacx.exe",
                        help="Staged executable filename")
    parser.add_argument("--wine")
    parser.add_argument("--wine-prefix")
    parser.add_argument("--play-intro-movie", action="store_true",
                        help="Leave PRACX's configured movie player enabled")
    args = parser.parse_args()

    game_dir = Path(args.game_dir).expanduser().resolve()
    executable_name = PureWindowsPath(args.executable)
    if (not args.executable or executable_name.drive or executable_name.root or
            len(executable_name.parts) != 1 or ":" in args.executable or
            not args.executable.casefold().endswith(".exe")):
        parser.error(f"invalid executable filename: {args.executable!r}")
    executable = game_dir / args.executable
    if executable.is_symlink():
        parser.error(f"executable must not be a symlink: {executable}")
    if executable.resolve().parent != game_dir:
        parser.error(f"executable is outside the game directory: {executable}")
    if not executable.is_file():
        parser.error(f"patched executable not found: {executable}")

    environment = os.environ.copy()
    if args.wine_prefix:
        environment["WINEPREFIX"] = str(Path(args.wine_prefix).expanduser().resolve())

    movie_config = None
    try:
        if not args.play_intro_movie:
            movie_config = configure_intro_movie_skip(game_dir, secrets.token_hex(16))
        if os.name == "nt":
            command = [str(executable)]
        else:
            wine = find_wine(args.wine)
            wine_path = Path(wine)
            if sys.platform == "darwin" and len(wine_path.parents) >= 5:
                application = wine_path.parents[4]
                if application.suffix == ".app":
                    command = ["open", "-W", "-n", "-a", application.stem]
                    if "WINEPREFIX" in environment:
                        command.extend([
                            "--env", f"WINEPREFIX={environment['WINEPREFIX']}",
                        ])
                    command.extend(["--args", str(executable)])
                else:
                    command = [wine, str(executable)]
            else:
                command = [wine, str(executable)]
        returncode = subprocess.run(command, cwd=str(game_dir), env=environment).returncode
    finally:
        if movie_config is not None:
            restore_intro_movie_config(*movie_config)
    raise SystemExit(returncode)


if __name__ == "__main__":
    main()
