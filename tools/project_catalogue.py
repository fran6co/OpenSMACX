#!/usr/bin/env python3
"""Put the catalogue's facts beside every annotation, so `src/` carries them.

`docs/DECOMP_MAP.md` has said since it was written that "the decompilation is
tracked in exactly one place: the source tree… The CSVs under `docs/recovery/`
are derived views with a retirement path, not the record." That was true of the
MAP - which addresses exist and what state they are in - and false of everything
else about a function. Its name, its size, the spans it occupies and its
prototype lived only in `docs/recovery/functions.csv`, an IDA export, and every
tool that wanted them read the CSV.

Except for placeholders. All 1,863 of those already carry the facts, stamped by
`decomp_status.py --generate-placeholders`:

    // ORIGINAL: 0x00401100 FILE
    // name      ?add@StringStruct@@QAEHH@Z
    // size      1043 bytes
    // spans     0x00401100-0x00401513
    // prototype int (__thiscall ?add@StringStruct@@QAEHH@Z)(StringStruct*, int)
    // callers   21   call targets   3

Measured 2026-08-12: 1,863 of 3,478 annotated files carry that block, and it is
exactly the placeholder third. Product source, `src/recovered/` and
`src/recovered/units/` carry none of it. The projection was built for the
population that needed it and never finished, so the catalogue stayed load
bearing for the 4,108 pieces already recovered.

This finishes it, and then CHECKS it. `--check` compares what `src/` says
against what the export says, in both directions: a stamped block that
disagrees, and an annotation with no block. Once every annotation carries its
own facts, a reader wanting the name of a function can ask `src/`, and the CSV
becomes what the map already is - a derived view.

WHAT IS NOT PROJECTED, and why. `segment`, `comments` and `priority` have no
reader at all (measured: 0 tool references between them, and `priority` is empty
in every one of the 6,000 rows), so copying them into `src/` would carry dead
weight across. `body_ranges` IS projected, because a function's spans are how
the ratchet knows which bytes to compare and 402 functions carry more than one.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan  # noqa: E402
import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS_CSV = REPO_ROOT / "docs" / "recovery" / "functions.csv"

FACT_LINE = re.compile(
    r"^\s*(?://|\*)?\s*(name|size|spans|prototype|callers)\s+(.*?)\s*$")


def catalogue(path: Path = None) -> dict:
    """{address: row} for the CORRECTED catalogue, not the raw export.

    THROUGH `emit.load_functions()`, WHICH APPLIES `catalogue_corrections`. The
    raw CSV is IDA's reconstruction and the tree keeps a hand-curated overlay of
    names the bytes contradict - `?vx_malloc@Caviar@@QAAXK@Z` in the export is
    `SAXK` in `src/`, because the body takes no receiver. Reading the CSV
    directly made the first run of this tool report those corrections as
    disagreements, i.e. it accused `src/` of being wrong about the one thing
    `src/` is right about. The corrected catalogue is what every other tool
    consumes and is what a projection must be compared against.
    """
    if path is not None and path != FUNCTIONS_CSV:
        rows = {}
        with path.open(newline="", encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                rows[int(row["address"], 16)] = row
        return rows
    return emit.load_functions()


def facts(address: int, row: dict) -> list:
    """The comment lines a piece carries about itself.

    Deliberately the same spelling `decomp_status._catalog_facts` already uses
    on placeholders - one projection, not two, or the check below would be
    comparing this tool against itself rather than against the export.
    """
    spans = row.get("body_ranges") or \
        f"0x{address:08X}-0x{address + int(row.get('size') or 0):08X}"
    return [f"// name      {row.get('name', '')}",
            f"// size      {row.get('size', '')} bytes",
            f"// spans     {spans}",
            f"// prototype {row.get('prototype', '')}",
            f"// callers   {row.get('caller_count', '')}   "
            f"call targets   {row.get('call_target_count', '')}"]


def stamped(lines: list, index: int) -> dict:
    """{key: value} already recorded in the comment run after a marker."""
    found = {}
    for line in lines[index + 1:]:
        stripped = line.strip()
        if not (stripped.startswith("//") or stripped.startswith("*")):
            break
        match = FACT_LINE.match(line)
        if match:
            found[match.group(1)] = match.group(2)
    return found


def survey(src: Path, rows: dict) -> tuple:
    """(missing, disagreeing, uncatalogued) over every annotation in `src`."""
    missing, disagreeing, uncatalogued = [], [], []
    for annotation in annotation_scan.scan_tree(src):
        row = rows.get(annotation.address)
        if row is None:
            uncatalogued.append(annotation)
            continue
        path = REPO_ROOT / annotation.path
        try:
            lines = path.read_text(errors="ignore").splitlines()
        except OSError:
            continue
        if not annotation.line:
            continue
        present = stamped(lines, annotation.line - 1)
        if not present:
            missing.append(annotation)
            continue
        want = {}
        for line in facts(annotation.address, row):
            match = FACT_LINE.match(line)
            want[match.group(1)] = match.group(2)
        for key, value in want.items():
            if key in present and present[key] != value:
                disagreeing.append((annotation, key, present[key], value))
    return missing, disagreeing, uncatalogued


def apply(src: Path, rows: dict, missing: list) -> int:
    """Insert the facts under each marker that has none. Returns how many."""
    by_file = {}
    for annotation in missing:
        by_file.setdefault(annotation.path, []).append(annotation)
    changed = 0
    for relative, entries in by_file.items():
        path = REPO_ROOT / relative
        lines = path.read_text(errors="ignore").splitlines(keepends=True)
        # Deepest marker first, so an insertion never moves a later one.
        for annotation in sorted(entries, key=lambda a: -a.line):
            row = rows.get(annotation.address)
            if row is None or not annotation.line:
                continue
            block = [line + "\n" for line in facts(annotation.address, row)]
            lines[annotation.line:annotation.line] = block
            changed += 1
        path.write_text("".join(lines))
    return changed


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--src", type=Path, default=REPO_ROOT / "src")
    parser.add_argument("--functions", type=Path, default=FUNCTIONS_CSV)
    parser.add_argument("--apply", action="store_true",
                        help="stamp the facts onto annotations that lack them")
    parser.add_argument("--check", action="store_true",
                        help="exit 1 if src/ and the export disagree")
    arguments = parser.parse_args(argv)

    rows = catalogue(arguments.functions)
    missing, disagreeing, uncatalogued = survey(arguments.src, rows)
    total = len(annotation_scan.scan_tree(arguments.src))

    print(f"catalogue projection: {total - len(missing)} of {total} "
          f"annotation(s) carry their own facts")

    if disagreeing:
        print(f"  {len(disagreeing)} DISAGREE with the export:", file=sys.stderr)
        for annotation, key, mine, theirs in disagreeing[:10]:
            print(f"    {annotation.location} {key}: src says {mine!r}, "
                  f"the export says {theirs!r}", file=sys.stderr)
    if uncatalogued:
        print(f"  {len(uncatalogued)} annotation(s) name an address the export "
              f"does not have")

    if arguments.apply:
        changed = apply(arguments.src, rows, missing)
        print(f"stamped {changed} annotation(s)")
        return 0

    if arguments.check:
        # A disagreement is always a defect. A MISSING block is one too, but
        # only once the projection is complete - reported either way, and the
        # exit code says which of the two states this tree is in.
        if disagreeing:
            print(f"FAIL: src/ and the export disagree on {len(disagreeing)} "
                  f"fact(s)", file=sys.stderr)
            return 1
        if missing:
            print(f"FAIL: {len(missing)} annotation(s) carry no catalogue "
                  f"facts; run --apply", file=sys.stderr)
            return 1
        if not total:
            print("FAIL: no annotations were scanned, so this proves nothing",
                  file=sys.stderr)
            return 1
        print("OK: every annotation carries its own catalogue facts, and none "
              "disagrees with the export")
        return 0

    print(f"  {len(missing)} carry none; run --apply to stamp them")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
