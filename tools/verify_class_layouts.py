#!/usr/bin/env python3
"""Prove each extracted class layout against the real one, and record which.

`class_layouts` reads data members out of a header with regexes. That is
enough to be useful and not enough to be trusted: a member it skips - a
nested typedef's type, an anonymous pad - moves every offset after it, and
the recovered body that indexes through the wrong offset still COMPILES. It
fails later, as a byte mismatch nobody traces back to here.

So every candidate is compiled against the real class:

    struct Probe { ...extracted members... };
    static_assert(sizeof(Probe) == sizeof(Buffer), "layout");

and only the ones that survive are written to the verified list the emitter
reads. Measured on the first run: 20 of 25 candidates survived, and the five
that did not (Base, Buffer, Font, ListBox, PlanWin) would each have handed
an agent a layout that is wrong in a way nothing downstream would catch.

Needs VC6 under wine, so it is a generate-and-commit step rather than
something the emitter does per unit.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match as bm  # noqa: E402
import class_layouts  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
VERIFIED = REPO_ROOT / "docs" / "recovery" / "verified-layouts.txt"


def declaring_header() -> dict:
    """{class name: the header that defines it}."""
    home = {}
    for header in sorted(SRC.glob("*.h")):
        for name, _, _ in class_layouts.class_bodies(
                header.read_text(errors="ignore")):
            home.setdefault(name, header.name)
    return home


def verify(candidates: dict) -> tuple:
    """(verified, rejected) - one translation unit per class, compiled."""
    home = declaring_header()
    work = Path(tempfile.mkdtemp())
    environment = bm.wine_environment()
    environment["INCLUDE"] += ";Z:" + str(SRC).replace("/", "\\")

    units = {}
    for index, name in enumerate(sorted(candidates)):
        if name not in home:
            continue
        stem = f"c{index:03d}"
        body = "\n".join(class_layouts.unverified_declaration_for(name))
        (work / f"{stem}.cpp").write_text(
            f'#include "stdafx.h"\n#include "{home[name]}"\n'
            f"struct Probe {{\n{body}\n}};\n"
            f'static_assert(sizeof(Probe) == sizeof({name}), "layout");\n'
            f"int probe{index}() {{ return 0; }}\n")
        units[stem] = name

    (work / "cl.rsp").write_text(
        "/c /GR- /GX\n" + "\n".join(f"{stem}.cpp" for stem in units) + "\n")
    subprocess.run(["wine", str(bm.VC6_CL), "/nologo", "@cl.rsp"], cwd=work,
                   env=environment, capture_output=True, text=True)

    verified = sorted(name for stem, name in units.items()
                      if (work / f"{stem}.obj").is_file())
    rejected = sorted(name for stem, name in units.items()
                      if not (work / f"{stem}.obj").is_file())
    return verified, rejected


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed list is stale")
    # Named so verify_checks_can_fail.py can point --check at a deliberately
    # damaged copy and prove this gate refuses one. A check with no such proof
    # is a check nobody has seen fail.
    parser.add_argument("--verified", type=Path, default=VERIFIED,
                        help="the list to compare against (default: the "
                             "committed one)")
    arguments = parser.parse_args()
    verified_path = arguments.verified

    # Without the compiler every probe fails to build, which reads as "no
    # layout survived" and is indistinguishable from "every layout is wrong".
    # As a CTest gate that would turn a missing toolchain into 23 phantom
    # regressions, so the absence is reported as an absence.
    reason = bm.available()
    if reason:
        print(f"SKIP: {reason}. This needs Visual C++ 6.0 (12.00.8168) under "
              f"Wine.")
        return 0

    candidates = class_layouts.pinned_layouts()
    verified, rejected = verify(candidates)

    text = ("# Class layouts tools/class_layouts.py extracts AND that compile\n"
            "# to the same size as the real class. Regenerate with\n"
            "# tools/verify_class_layouts.py. A layout not listed here is not\n"
            "# handed to an agent; the emitter falls back to an opaque shell.\n"
            + "\n".join(verified) + "\n")

    if arguments.check:
        current = verified_path.read_text() if verified_path.is_file() else ""
        if current != text:
            print("verified-layouts.txt is stale; regenerate it", file=sys.stderr)
            return 1
        print(f"class layouts: {len(verified)} verified (up to date)")
        return 0

    verified_path.parent.mkdir(parents=True, exist_ok=True)
    verified_path.write_text(text)
    print(f"{len(verified)} verified, {len(rejected)} rejected -> {verified_path}")
    for name in rejected:
        print(f"    rejected: {name} (extracted layout is not the real size)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
