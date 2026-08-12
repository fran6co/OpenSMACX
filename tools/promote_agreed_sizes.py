#!/usr/bin/env python3
"""Turn an agreed size into a `static_assert`, once the compiler confirms it.

`derive_agreed_sizes.py` finds the classes where the IDA member table and the
observed access bound land on the same number. That agreement passes
`derive_class_layout.py --score-csv` at 24 right and 0 wrong, which no other
source in this tree has managed - but it is still agreement, not proof.

This is what makes it proof. `static_assert(sizeof(X) == N)` is compiled
against the REAL declaration under cl 12.00.8168, so a class whose size the two
sources got wrong fails the build instead of being believed. Only the
assertions that compile are kept; the rest are reported and dropped.

That is also why this is worth doing rather than leaving the sizes in a CSV.
A pinned size is what `class_layouts.pinned_layouts()` reads, so a class that
gains one becomes a candidate for `verify_class_layouts.py`, and a class that
passes THAT is handed to an agent with real members instead of an opaque shell.
The assertion is the gate between a hypothesis and a layout an agent may rely
on.

IT IS ALSO THE ONE IRREVERSIBLE STEP. Everything else this campaign added -
declarations, names, padded extents - is checked downstream by objdiff, and a
mistake costs attempts. A `static_assert` is believed by everything downstream
and checked by nothing, so it gets the compiler's answer and not a source's.

HOW MUCH THE COMPILE ACTUALLY PROVES, stated plainly. For a class whose tail
was padded out to the access bound by `extend_short_classes.py`, its declared
size IS the access bound, and the agreed size is the access bound too - so the
assertion passes by construction and confirms nothing. The evidence for those
is the AGREEMENT and its control (24 right, 0 wrong against the classes pinned
before any of this), not the compile.

The compile still earns its place: it is the only thing that can catch a class
whose declaration was arrived at independently and disagrees. It caught exactly
one - `Sound`, where the two sources agreed on 0x54 and the real class is not
that size. Without it that number would have been written into `src/` as a
proof.
"""

from __future__ import annotations

import argparse
import atexit
import csv
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match as bm  # noqa: E402
import class_layouts  # noqa: E402
import derive_class_layout  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
AGREED = REPO_ROOT / "docs" / "recovery" / "agreed-class-sizes.csv"
GENERATED = "hypothesis_layouts.h"


def agreed_sizes(path: Path = None) -> dict:
    found = {}
    path = path or AGREED
    if not path.is_file():
        return found
    with path.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            try:
                found[row["class"]] = int(row["size"], 16)
            except (ValueError, KeyError, TypeError):
                continue
    return found


def declaring_header() -> dict:
    home = {}
    for header in sorted(SRC.glob("*.h")):
        for name, _, _ in class_layouts.class_bodies(
                header.read_text(errors="ignore")):
            home.setdefault(name, header.name)
    return home


def confirmed(candidates: dict, home: dict) -> tuple:
    """(sizes the compiler agrees with, sizes it refuses).

    One translation unit per class, exactly as `verify_class_layouts.py` does,
    so one refusal does not hide the rest.
    """
    work = Path(tempfile.mkdtemp())
    # Removed when the process exits rather than left behind: these
    # probes ran often enough to leave 664 abandoned directories in
    # build/ before anyone counted them. `atexit` rather than a
    # `finally` so an unhandled exception cleans up too.
    atexit.register(shutil.rmtree, work, True)
    environment = bm.wine_environment()
    environment["INCLUDE"] += ";Z:" + str(SRC.resolve()).replace("/", "\\")
    units = {}
    for index, name in enumerate(sorted(candidates)):
        stem = f"p{index:04d}"
        (work / f"{stem}.cpp").write_text(
            f'#include "stdafx.h"\n#include "{home[name]}"\n'
            f"static_assert(sizeof({name}) == 0x{candidates[name]:X},"
            f' "size");\n'
            f"int probe{index}() {{ return 0; }}\n")
        units[stem] = name
    (work / "cl.rsp").write_text(
        "/c /GR- /GX\n" + "\n".join(f"{stem}.cpp" for stem in units) + "\n")
    subprocess.run(["wine", str(bm.VC6_CL), "/nologo", "@cl.rsp"], cwd=work,
                   env=environment, capture_output=True, text=True)
    good = {name for stem, name in units.items()
            if (work / f"{stem}.obj").is_file()}
    return good, set(units.values()) - good


def insert(text: str, name: str, size: int) -> str:
    """Put the assertion straight after the class's closing brace.

    Where `src/` already spells it - see `buffer.h` - and where anything
    scraping `static_assert(sizeof(X) == N)` out of a header will find it.
    """
    for head in class_layouts.CLASS_HEAD.finditer(text):
        if head.group("name") != name:
            continue
        depth, index = 1, head.end()
        while index < len(text) and depth:
            if text[index] == "{":
                depth += 1
            elif text[index] == "}":
                depth -= 1
            index += 1
        end = text.find("\n", index)
        if end < 0:
            end = len(text)
        return (text[:end + 1]
                + f"\nstatic_assert(sizeof({name}) == 0x{size:X},\n"
                  f'              "{name} layout must match the original '
                  f'executable");\n'
                + text[end + 1:])
    return text


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--apply", action="store_true",
                        help="write the headers (default: report only)")
    # Any source that has already passed derive_class_layout.py --score-csv.
    # The compile below is what actually admits each size, so a second source
    # needs no second mechanism - only its own CSV.
    parser.add_argument("--sizes", type=Path, default=AGREED,
                        help="the scored size CSV to promote from "
                             "(default: the two-source agreement)")
    args = parser.parse_args(argv)

    reason = bm.available()
    if reason:
        print(f"SKIP: {reason}. This compiles the assertion for real.")
        return 0

    sizes = agreed_sizes(args.sizes)
    if not sizes:
        print(f"SKIP: {args.sizes} is absent.")
        return 0
    pinned = derive_class_layout.load_pinned()
    home = declaring_header()
    # THE GENERATED HEADER IS EXCLUDED, for two reasons and the second is the
    # one that matters. Its classes are regenerated from the IDB member table,
    # so an assertion written there is wiped on the next run - and worse, it
    # would be a TAUTOLOGY: the class is declared from the members that sum to
    # the agreed size, so `sizeof(X) == S` proves the emitter and not the
    # layout. A size is only worth pinning against a declaration that was
    # arrived at independently.
    candidates = {name: size for name, size in sizes.items()
                  if name not in pinned and name in home
                  and home[name] != GENERATED}
    if not candidates:
        print("nothing to promote: every agreed size is already pinned.")
        return 0

    good, refused = confirmed(candidates, home)
    print(f"{len(candidates)} candidate(s); the compiler confirms {len(good)} "
          f"and refuses {len(refused)}")
    for name in sorted(refused):
        print(f"   refused {name}: sizeof is not 0x{candidates[name]:X}, so "
              f"the two sources agreed on a wrong number")

    by_header = {}
    for name in sorted(good):
        by_header.setdefault(home[name], []).append(name)
        print(f"   {name:22} sizeof == 0x{candidates[name]:X}")

    if args.apply:
        for header, names in by_header.items():
            path = SRC / header
            text = path.read_text()
            for name in names:
                text = insert(text, name, candidates[name])
            path.write_text(text)
        print(f"\npinned {len(good)} class(es) across {len(by_header)} "
              f"header(s)")
    else:
        print("\nre-run with --apply to write them")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
