#!/usr/bin/env python3

import os
from pathlib import Path
import subprocess

from wine_runtime import disable_crash_dialog, find_wine


OWNER_MARKER = ".opensmacx-test-owned"
INITIALIZED_MARKER = ".opensmacx-test-initialized"
CRASH_DIALOG_MARKER = ".opensmacx-test-crash-dialog-disabled"
OWNER_CONTENT = "OpenSMACX dedicated runtime test prefix\n"


def validate_owned_wine_prefix(prefix):
    prefix = prefix.expanduser().absolute()
    if prefix.resolve(strict=False) != prefix:
        raise RuntimeError(
            f"test Wine prefix must not contain symlinks: {prefix}")
    marker = prefix / OWNER_MARKER
    if prefix.exists():
        if not prefix.is_dir():
            raise RuntimeError(f"test Wine prefix is not a directory: {prefix}")
        if (marker.is_symlink() or not marker.is_file()
                or marker.read_text(encoding="ascii") != OWNER_CONTENT):
            raise RuntimeError(
                f"refusing to own unmarked existing Wine prefix: {prefix}")
    return prefix


def claim_owned_wine_prefix(prefix):
    prefix = validate_owned_wine_prefix(prefix)
    if prefix.exists():
        return False
    prefix.mkdir(parents=True)
    (prefix / OWNER_MARKER).write_text(OWNER_CONTENT, encoding="ascii")
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
        if initialized.is_symlink() or (
                initialized.exists() and not initialized.is_file()):
            raise RuntimeError(
                f"test Wine initialization marker is not a regular file: {initialized}")
        if not initialized.is_file():
            environment = prefix_environment(prefix)
            subprocess.run(
                [str(wineboot), "--init"], env=environment, check=True,
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            initialized.write_text("initialized\n", encoding="ascii")
        # A test binary that faults - which is exactly what a killed mutant or
        # a genuine regression does - otherwise blocks on winedbg's modal
        # dialog until the runner times out, and the backtrace never reaches
        # any log. Its own marker, because prefixes initialized before this
        # existed still need the value written once.
        crash_dialog = prefix / CRASH_DIALOG_MARKER
        if crash_dialog.is_symlink() or (
                crash_dialog.exists() and not crash_dialog.is_file()):
            raise RuntimeError(
                f"test Wine crash-dialog marker is not a regular file: {crash_dialog}")
        if not crash_dialog.is_file():
            # Beside wineboot, the same way wine_prefix_tools derives it, so a
            # caller that redirected that lookup stays redirected here.
            disable_crash_dialog(
                wineboot.with_name("wine"), prefix_environment(prefix))
            crash_dialog.write_text("disabled\n", encoding="ascii")
    finally:
        stop_owned_wine_prefix(prefix, wine)
