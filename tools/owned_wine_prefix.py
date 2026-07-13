#!/usr/bin/env python3

import os
from pathlib import Path
import subprocess

from setup_game import find_wine


OWNER_MARKER = ".opensmacx-test-owned"
INITIALIZED_MARKER = ".opensmacx-test-initialized"
OWNER_CONTENT = "OpenSMACX dedicated runtime test prefix\n"


def claim_owned_wine_prefix(prefix):
    if prefix.is_symlink():
        raise RuntimeError(f"test Wine prefix must not be a symlink: {prefix}")
    marker = prefix / OWNER_MARKER
    if prefix.exists():
        if not prefix.is_dir():
            raise RuntimeError(f"test Wine prefix is not a directory: {prefix}")
        if (marker.is_symlink() or not marker.is_file()
                or marker.read_text(encoding="ascii") != OWNER_CONTENT):
            raise RuntimeError(
                f"refusing to own unmarked existing Wine prefix: {prefix}")
        return False
    prefix.mkdir(parents=True)
    marker.write_text(OWNER_CONTENT, encoding="ascii")
    return True


def wine_prefix_tools(wine):
    wine_path = Path(find_wine(wine))
    wineboot = wine_path.with_name("wineboot")
    wineserver = wine_path.with_name("wineserver")
    if not wineboot.is_file() or not wineserver.is_file():
        raise RuntimeError(
            f"Wine installation lacks wineboot or wineserver beside {wine_path}")
    return wineboot, wineserver


def prefix_environment(prefix):
    environment = os.environ.copy()
    environment["WINEPREFIX"] = str(prefix)
    return environment


def stop_owned_wine_prefix(prefix, wine):
    claim_owned_wine_prefix(prefix)
    _, wineserver = wine_prefix_tools(wine)
    environment = prefix_environment(prefix)
    subprocess.run(
        [str(wineserver), "-k"], env=environment, check=False,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(
        [str(wineserver), "-w"], env=environment, check=True,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def prepare_owned_wine_prefix(prefix, wine):
    claim_owned_wine_prefix(prefix)
    wineboot, _ = wine_prefix_tools(wine)
    initialized = prefix / INITIALIZED_MARKER
    try:
        if not initialized.is_file():
            environment = prefix_environment(prefix)
            subprocess.run(
                [str(wineboot), "--init"], env=environment, check=True,
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            initialized.write_text("initialized\n", encoding="ascii")
    finally:
        stop_owned_wine_prefix(prefix, wine)
