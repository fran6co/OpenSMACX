#!/usr/bin/env python3
"""Temporary build-path shim; compiler interaction lives in recovery/environment."""

from pathlib import Path
from runpy import run_path
import sys

if __name__ == "__main__":
    implementation = Path(__file__).resolve().parents[1] / "recovery/environment/wine.py"
    raise SystemExit(run_path(str(implementation))["main"](sys.argv))
