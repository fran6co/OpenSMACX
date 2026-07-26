#!/usr/bin/env python3

import argparse
import os
from pathlib import Path, PureWindowsPath
import secrets
import subprocess
import sys

from movie_skip import configure_intro_movie_skip, restore_intro_movie_config
from wine_runtime import disable_crash_dialog, find_wine


def main():
    parser = argparse.ArgumentParser(description="Run a staged OpenSMACX game")
    parser.add_argument("--game-dir", required=True)
    parser.add_argument("--executable", default="terranx_opensmacx.exe",
                        help="Staged executable filename")
    parser.add_argument("--wine")
    parser.add_argument("--wine-prefix")
    parser.add_argument("--play-intro-movie", action="store_true",
                        help="Leave PRACX's configured movie player enabled")
    parser.add_argument("--log", help="Capture stdout and stderr, including any "
                                      "crash backtrace, to this file")
    args = parser.parse_args()

    log_path = Path(args.log).expanduser().resolve() if args.log else None
    if log_path is not None and not log_path.parent.is_dir():
        parser.error(f"log directory does not exist: {log_path.parent}")

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
    log_file = None
    try:
        if not args.play_intro_movie:
            movie_config = configure_intro_movie_skip(game_dir, secrets.token_hex(16))
        bundle_launch = False
        if os.name == "nt":
            command = [str(executable)]
        else:
            wine = find_wine(args.wine)
            disable_crash_dialog(wine, environment)
            wine_path = Path(wine)
            if sys.platform == "darwin" and len(wine_path.parents) >= 5:
                application = wine_path.parents[4]
                if application.suffix == ".app":
                    bundle_launch = True
                    command = ["open", "-W", "-n", "-a", application.stem]
                    if "WINEPREFIX" in environment:
                        command.extend([
                            "--env", f"WINEPREFIX={environment['WINEPREFIX']}",
                        ])
                    if log_path is not None:
                        command.extend(["--stderr", str(log_path)])
                    command.extend(["--args", str(executable)])
                else:
                    command = [wine, str(executable)]
            else:
                command = [wine, str(executable)]
        # The bundle launch redirects through open's own --stderr; every other
        # path is an ordinary child whose handles we can point at the log.
        if log_path is not None and not bundle_launch:
            log_file = log_path.open("w", encoding="utf-8")
            returncode = subprocess.run(
                command, cwd=str(game_dir), env=environment,
                stdout=log_file, stderr=subprocess.STDOUT).returncode
        else:
            returncode = subprocess.run(
                command, cwd=str(game_dir), env=environment).returncode
    finally:
        if log_file is not None:
            log_file.close()
        if movie_config is not None:
            restore_intro_movie_config(*movie_config)
    raise SystemExit(returncode)


if __name__ == "__main__":
    main()
