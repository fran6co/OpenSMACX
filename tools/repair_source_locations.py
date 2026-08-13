#!/usr/bin/env python3
"""Re-point `source_locations` at the bodies they name.

A location is `path:line`, and the line is where `byte_match_census` starts
reading. Nothing keeps it in step with the file. Insert a line anywhere above a
body - a new `#include`, a comment, one more global - and every location below
it in that file is off by one, silently, forever.

`extract_body` tolerates a small drift by accident: the location points INTO
the doc comment, and a comment is several lines tall, so a shift of one or two
still lands inside it and the body still comes out right. That accident is why
this went unnoticed. Measured across the catalogue: 46 files carry a shift,
most of them +1, and `src/init_thunks.cpp` is off by +14 and
`src/scroll.cpp` by -10. 619 rows drift far enough to extract the WRONG TEXT.

What that costs is worse than a bad line number. The census scores whatever it
extracts and writes the verdict against the address it MEANT to read, so a row
can be scored against a neighbouring function's body, or against a run of
global definitions - which is what `0x00402F10` was doing, reporting
`g_ALPHAMENU_WAVE : undeclared identifier` for a body that does not mention
it. A NO_COMPILE that is really a bad pointer looks exactly like a NO_COMPILE
that is really a broken body, and only one of them is worth an agent's time.

The repair is mechanical because the body carries its own identity: every
committed recovery has an `Original Offset: XXXXXXXX` line in its doc comment,
written by `writeback` and required by `integrate_recovery`. So the
true line is a lookup, not a guess, and a row whose offset line is absent is
reported rather than repositioned.

    tools/repair_source_locations.py            # report
    tools/repair_source_locations.py --apply    # re-point them
    tools/repair_source_locations.py --check    # exit 1 if any have drifted
"""

from __future__ import annotations

import argparse
import collections
import csv
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"

# The source-map grammar (docs/DECOMP_MAP.md) speaks `ORIGINAL: 0x...`; the
# legacy `Original Offset:` spelling is still read for stragglers and tests.
OFFSET_LINE = re.compile(
    r"^\s*(?://\s*)?(?:ORIGINAL:\s*0x|Original Offset:\s*)([0-9A-Fa-f]{8})\s*$")


def offset_index(path: Path) -> dict:
    """{address: line number} for every ORIGINAL marker line in the file."""
    found = {}
    for number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
        match = OFFSET_LINE.match(line)
        if match:
            # FIRST wins. A duplicated offset line means two bodies claim one
            # address, which `test_no_duplicate_bodies` is the right place to
            # complain about; silently preferring the last one here would hide
            # it behind a location that happens to work.
            found.setdefault(int(match.group(1), 16), number)
    return found


def survey(rows: list) -> tuple:
    """(repairs, unfindable) - what has drifted and what cannot be placed."""
    by_file = collections.defaultdict(list)
    for row in rows:
        location = (row.get("source_locations") or "").strip()
        if not location:
            continue
        first = location.split(";")[0]
        path, _, line = first.rpartition(":")
        if not line.isdigit() or not path:
            continue
        by_file[path].append((row, first, int(line)))

    repairs, unfindable = [], []
    for path, entries in sorted(by_file.items()):
        resolved = REPO_ROOT / path
        if not resolved.is_file():
            unfindable += [(row["address"], first, "file is absent")
                           for row, first, _ in entries]
            continue
        index = offset_index(resolved)
        for row, first, recorded in entries:
            address = int(row["address"], 16)
            actual = index.get(address)
            if actual is None:
                unfindable.append((row["address"], first,
                                   "no `Original Offset` line names it"))
            elif actual != recorded:
                repairs.append((row, first, f"{path}:{actual}",
                                actual - recorded))
    return repairs, unfindable


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--apply", action="store_true")
    parser.add_argument("--check", action="store_true",
                        help="exit 1 if any location has drifted")
    # THE CATALOGUE IS A PARAMETER so a caller can be sandboxed. Without it
    # `--apply` always rewrites the committed `functions.csv`, whatever root
    # the caller thinks it is working under - which is how
    # `test_integrate_recovery.py`, running against a temporary directory,
    # re-pointed 37 rows in the real file on every ctest run.
    parser.add_argument("--functions", type=Path, default=FUNCTIONS,
                        help="the catalogue to survey and rewrite "
                             "(default: the committed one)")
    arguments = parser.parse_args(argv)
    catalogue = arguments.functions

    with catalogue.open(newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        fields, rows = reader.fieldnames, list(reader)

    repairs, unfindable = survey(rows)

    if arguments.check:
        if not repairs:
            return 0
        drift = collections.Counter(path.rsplit(":", 1)[0]
                                    for _, path, _, _ in repairs)
        print(f"source-locations: {len(repairs)} have drifted from their "
              f"bodies", file=sys.stderr)
        for path, count in drift.most_common(10):
            print(f"  {count:>4}  {path}", file=sys.stderr)
        print("  run tools/repair_source_locations.py --apply",
              file=sys.stderr)
        return 1

    print(f"{len(repairs)} locations have drifted, "
          f"{len(unfindable)} cannot be placed")
    shifts = collections.Counter(delta for _, _, _, delta in repairs)
    for delta, count in shifts.most_common(8):
        print(f"  {count:>4} rows shifted {delta:+d}")
    for address, first, why in unfindable[:10]:
        print(f"  UNPLACEABLE {address} {first}: {why}")

    if not arguments.apply or not repairs:
        return 0

    for row, first, corrected, _ in repairs:
        rest = (row["source_locations"] or "").split(";")[1:]
        row["source_locations"] = ";".join([corrected] + rest)
    with catalogue.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    print(f"re-pointed {len(repairs)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
