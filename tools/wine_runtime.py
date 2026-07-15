#!/usr/bin/env python3

import os
from pathlib import Path
import shutil
import sys


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
