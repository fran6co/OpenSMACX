#!/usr/bin/env python3

import argparse
import os
from pathlib import Path
import subprocess

from owned_wine_prefix import prepare_owned_wine_prefix, stop_owned_wine_prefix
from wine_runtime import find_wine


KEEP_OWNED_PREFIX_ENV = "OPENSMACX_KEEP_OWNED_WINE_PREFIX_RUNNING"


def main():
    parser = argparse.ArgumentParser(
        description="Run a Windows test executable locally")
    parser.add_argument("--executable", required=True)
    parser.add_argument("--wine")
    parser.add_argument("--wine-prefix")
    # Everything after `--` goes to the test binary untouched. Without this a
    # test that takes a flag can only be run by hand, which is how
    # recovery-leaf-tests' `--reverse` came to be documented as "the regression
    # test" for an ordering bug while nothing in CI ever ran it.
    parser.add_argument("test_args", nargs="*", metavar="-- ARG...")
    args = parser.parse_args()

    executable = Path(args.executable).expanduser().resolve()
    if not executable.is_file():
        parser.error(f"test executable not found: {executable}")

    if os.name == "nt":
        raise SystemExit(
            subprocess.run([str(executable), *args.test_args]).returncode)
    if not args.wine_prefix:
        parser.error("a dedicated --wine-prefix is required")

    wine_prefix = Path(args.wine_prefix).expanduser().absolute()
    wine = find_wine(args.wine)
    prepare_owned_wine_prefix(wine_prefix, wine)
    environment = os.environ.copy()
    environment["WINEPREFIX"] = str(wine_prefix)
    keep_prefix_running = os.environ.get(KEEP_OWNED_PREFIX_ENV) == "1"
    try:
        result = subprocess.run([wine, str(executable), *args.test_args],
                                env=environment).returncode
    finally:
        if not keep_prefix_running:
            stop_owned_wine_prefix(wine_prefix, wine)
    raise SystemExit(result)


if __name__ == "__main__":
    main()
