#!/usr/bin/env python3

import os
from pathlib import Path
import shutil
import subprocess
import sys


def disable_crash_dialog(wine, environment):
    """Route crash backtraces to stderr instead of winedbg's modal dialog.

    Without this, a fault inside the hybrid image blocks on a GUI dialog that
    no log captures, so the faulting address never reaches a diagnostics file.
    ShowCrashDialog=0 makes winedbg dump the same backtrace to stderr, which
    every launcher here already redirects into its log.
    """
    subprocess.run(
        [str(wine), "reg", "add", r"HKCU\Software\Wine\WineDbg",
         "/v", "ShowCrashDialog", "/t", "REG_DWORD", "/d", "0", "/f"],
        env=environment, check=False,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


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
