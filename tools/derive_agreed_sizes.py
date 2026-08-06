#!/usr/bin/env python3
"""Sizes two independent sources agree on, which is enough to pin one.

`docs/ORACLE_SESSIONS.md` said what the shape had to be and then measured that
it did not happen: *"A class whose IDB sum equals its next-global upper bound
is pinned exactly by two independent sources agreeing, and that is the shape
the campaign should use"* - followed by **0 of the 40**, because the upper
bound measures to the next constructed global and is far looser than the
object.

The shape was right and the second source was wrong. Pair the IDB's member-sum
with the ACCESS LOWER BOUND instead - the furthest byte a class's own thiscall
methods reach through `this` - and the two meet often, because both are trying
to describe the same object rather than the gap to the next one.

They are independent in the way that matters. One is a table somebody typed
into IDA; the other is read out of the instruction stream. Their failure modes
do not overlap: the IDB under-reports when a member was never entered, and the
access bound under-reports when a method never touches the tail. Nothing makes
them agree on a WRONG size, and measured against the classes whose size is
already pinned, nothing does:

    24 right, 0 wrong, 16 not claimed

`derive_class_layout.py --score-csv` - the gate that refused Thinker at 5 right
and 7 wrong, and the IDB alone at 31 and 2 - accepts this. It is the first
source in this tree that has passed it.

WHAT AGREEMENT IS NOT. It is not proof, and this file does not pretend
otherwise: it is two sources that under-report in different circumstances
landing on the same number. The `--check` control is what stands behind it, and
it must stay at zero wrong. Promotion to a `static_assert` compiles the
assertion against the real declaration, so a class whose size this gets wrong
fails the build rather than being believed.
"""

from __future__ import annotations

import argparse
import collections
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import derive_class_layout  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
BOUNDS = REPO_ROOT / "docs" / "recovery" / "access-lower-bounds.csv"
IDB_MEMBERS = REPO_ROOT / "docs" / "recovery" / "idb-members.csv"
OUTPUT = REPO_ROOT / "docs" / "recovery" / "agreed-class-sizes.csv"


def access_bounds() -> dict:
    found = {}
    if not BOUNDS.is_file():
        return found
    with BOUNDS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            try:
                found[row["class"]] = int(row["lower_bound"], 16)
            except (ValueError, KeyError, TypeError):
                continue
    return found


def idb_totals() -> dict:
    """{class: where its member table ends}."""
    totals = collections.defaultdict(int)
    if not IDB_MEMBERS.is_file():
        return totals
    with IDB_MEMBERS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            try:
                end = int(row["offset"], 16) + int(row["size"])
            except (ValueError, KeyError, TypeError):
                continue
            totals[row["class"]] = max(totals[row["class"]], end)
    return totals


def agreed() -> dict:
    """{class: size} where both sources land on the same number.

    The IDB's total is ALIGNED UP first. Its table ends at the last member,
    not at `sizeof`, and MSVC rounds a class up to its alignment - `Buffer`
    sums to 0x585 and really is 0x588. Comparing the raw sum would make every
    4-byte-aligned class look like a disagreement.
    """
    bounds, totals = access_bounds(), idb_totals()
    found = {}
    for name, total in totals.items():
        aligned = (total + 3) & ~3
        if bounds.get(name) == aligned and aligned > 0:
            found[name] = aligned
    return found


def render(sizes: dict) -> str:
    import io

    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\n")
    writer.writerow(["class", "size", "evidence"])
    for name in sorted(sizes):
        writer.writerow([
            name, f"0x{sizes[name]:X}",
            "the IDB member table and the observed access bound agree"])
    return buffer.getvalue()


def control(sizes: dict) -> tuple:
    """(right, wrong) against the classes whose size `src/` already pins."""
    pinned = derive_class_layout.load_pinned()
    right = [n for n in sizes if n in pinned and sizes[n] == pinned[n]]
    wrong = [(n, sizes[n], pinned[n]) for n in sizes
             if n in pinned and sizes[n] != pinned[n]]
    return right, wrong


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed CSV is stale or the "
                             "control regresses")
    args = parser.parse_args(argv)

    sizes = agreed()
    if not sizes:
        print("SKIP: neither input table is present.")
        return 0
    text = render(sizes)
    right, wrong = control(sizes)

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        if wrong:
            print(f"the control regressed: {len(wrong)} wrong",
                  file=sys.stderr)
            for name, claimed, true in wrong:
                print(f"    {name}: agreed 0x{claimed:X}, real 0x{true:X}",
                      file=sys.stderr)
            return 1
        print(f"agreed class sizes: {len(sizes)} classes, control "
              f"{len(right)} right / 0 wrong (up to date)")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    pinned = derive_class_layout.load_pinned()
    fresh = [n for n in sizes if n not in pinned]
    print(f"{len(sizes)} class(es) where both sources agree -> {args.out}")
    print(f"    control: {len(right)} right, {len(wrong)} wrong")
    for name, claimed, true in wrong:
        print(f"        WRONG {name}: agreed 0x{claimed:X}, real 0x{true:X}")
    print(f"    {len(fresh)} of them have no pinned size yet")
    return 1 if wrong else 0


if __name__ == "__main__":
    raise SystemExit(main())
