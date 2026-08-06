#!/usr/bin/env python3
"""Grow the classes the image proves are bigger than `src/` declares.

`derive_access_bounds.py` reads the largest `[ecx+N]` a class's own thiscall
methods reach. That is a hard lower bound: the object has to contain the byte
the code writes. Nineteen classes in `src/` are declared smaller than their
own bound, and some by a lot - `BaseWin` declares 0xa14 bytes, which is exactly
its base and nothing of its own, while the image reaches 0x40d20.

A class that is too short is not a cosmetic problem. Every recovered method
that indexes into the missing tail is writing outside the object, and every
`sizeof` taken of it - by `operator new` in its own constructor, by an array of
them - is wrong. So the tail is declared as what it is: storage of known extent
and unknown content, named for its own offset in the tree's `field_<hex>_`
idiom.

THE SIZE IS ASKED OF THE COMPILER, NOT COMPUTED. `class_layouts`' regexes read
members well enough to be useful and not well enough to total them - bases,
alignment and anything they skip all move the answer. So the declared size is
bisected with `static_assert(sizeof(X) >= N)` probes under the real compiler,
which is exact by construction.

AND THE PROBE IS CONTROLLED. A probe that fails to build scores identically to
one whose assertion failed, so every class is first compiled with a trivially
true bound. Only a class that compiles at `>= 1` and fails at `>= bound` is
short. Without that control the first run of this measurement reported all 94
classes as too small, including `Win`, which is exactly its bound.

WHAT THIS DOES NOT CLAIM. The bound is a floor, not a size, so the extended
class is still a lower bound and gets no `static_assert`. It is bigger than it
was and still may be smaller than the real thing.
"""

from __future__ import annotations

import argparse
import csv
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match as bm  # noqa: E402
import class_layouts  # noqa: E402
import emit_hypothesis_layouts as hypothesis  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
BOUNDS = REPO_ROOT / "docs" / "recovery" / "access-lower-bounds.csv"
IDB_MEMBERS = REPO_ROOT / "docs" / "recovery" / "idb-members.csv"
GENERATED = "hypothesis_layouts.h"


def idb_members() -> dict:
    """{class: {offset: (name, size)}} from the committed member table."""
    found = {}
    if not IDB_MEMBERS.is_file():
        return found
    with IDB_MEMBERS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            try:
                offset, size = int(row["offset"], 16), int(row["size"])
            except (ValueError, KeyError, TypeError):
                continue
            found.setdefault(row["class"], {})[offset] = (row["name"], size)
    return found


def tail_members(name: str, at: int, bound: int, members: dict) -> list:
    """[(offset, name, size)] to declare between `at` and `bound`, or [].

    THE GATE IS THAT THE IDB'S OWN BOUNDARY LANDS ON `at`. Its offsets are not
    recorded, they accumulate from member sizes, so a member nobody entered
    shifts everything after it. A table that independently starts a member at
    exactly the byte `src/` finished at is agreeing with `src/` about
    everything before that point, which is the only check available here.

    Refusing on that basis is not theoretical: `NetDaemon` has no member at its
    declared end and is refused - and it is also one of the four classes whose
    IDB total `derive_access_bounds.py --falsify` contradicts. Two independent
    checks reject the same table.
    """
    table = members.get(name)
    if not table or at not in table:
        return []
    return [(offset, table[offset][0], table[offset][1])
            for offset in sorted(table) if at <= offset < bound]


def lower_bounds() -> dict:
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


def declaring_header() -> dict:
    home = {}
    for header in sorted(SRC.glob("*.h")):
        if header.name == GENERATED:
            continue
        for name, _, _ in class_layouts.class_bodies(
                header.read_text(errors="ignore")):
            home.setdefault(name, header.name)
    return home


def probe(tests: dict, home: dict, tag: str) -> set:
    """{name} whose `sizeof` satisfies its threshold, compiled for real."""
    if not tests:
        return set()
    work = Path(tempfile.mkdtemp())
    environment = bm.wine_environment()
    environment["INCLUDE"] += ";Z:" + str(SRC.resolve()).replace("/", "\\")
    units = {}
    for index, (name, threshold) in enumerate(sorted(tests.items())):
        stem = f"{tag}{index:04d}"
        (work / f"{stem}.cpp").write_text(
            f'#include "stdafx.h"\n#include "{home[name]}"\n'
            f'static_assert(sizeof({name}) >= {threshold}, "size");\n'
            f"int probe{index}() {{ return 0; }}\n")
        units[stem] = name
    # The response file names SOURCES. Listing bare stems compiles nothing and
    # reads as every assertion failing - which is how the first run of this
    # measurement concluded all 94 classes were too small.
    (work / "cl.rsp").write_text(
        "/c /GR- /GX\n" + "\n".join(f"{stem}.cpp" for stem in units) + "\n")
    subprocess.run(["wine", str(bm.VC6_CL), "/nologo", "@cl.rsp"], cwd=work,
                   env=environment, capture_output=True, text=True)
    return {name for stem, name in units.items()
            if (work / f"{stem}.obj").is_file()}


def declared_sizes(names: list, home: dict, ceiling: dict) -> dict:
    """Exact `sizeof` for each name, by bisection under the real compiler."""
    low = {name: 1 for name in names}
    high = {name: max(ceiling[name], 1) for name in names}
    for round_ in range(32):
        tests = {name: (low[name] + high[name] + 1) // 2
                 for name in names if low[name] < high[name]}
        if not tests:
            break
        passing = probe(tests, home, f"b{round_}_")
        for name, threshold in tests.items():
            if name in passing:
                low[name] = threshold
            else:
                high[name] = threshold - 1
    return low


def tail_text(name: str, at: int, length: int, members: list) -> str:
    """The declarations to append: the IDB's members, else one byte array.

    Whatever the IDB covers, the class still ends at `at + length`, so the
    remainder after its last member is padded. The total is the same either
    way - this changes how much of the tail has names, not how big it is.
    """
    reach = at + length
    head = (f"\n  // Storage the image proves is here: its own methods "
            f"reach 0x{reach:X}.\n"
            f"  // Extent only - this class carries no size assertion, and "
            f"the bound is a floor.\n")
    if not members:
        return (head + f"  uint8_t field_{at:X}_[0x{length:X}];\n")

    named = sum(1 for _, member, _ in members
                if not hypothesis.PLACEHOLDER.match(member))
    lines = [head.rstrip("\n"),
             f"  // {len(members)} member(s) from the IDA database, {named} "
             f"named; it starts a member at 0x{at:X}, which is where src/ "
             f"ends.",
             ""]
    taken = set()
    cursor = at
    for offset, member, size in members:
        if offset < cursor or size <= 0:
            continue
        if offset > cursor:
            lines.append(f"  uint8_t field_{cursor:X}_[0x{offset - cursor:X}];"
                         f"  // 0x{cursor:X}")
        if offset + size > reach:
            break
        lines.append(f"  {hypothesis.storage(size, hypothesis.member_name(member, offset, taken))}"
                     f"  // 0x{offset:X}")
        cursor = offset + size
    if cursor < reach:
        lines.append(f"  uint8_t field_{cursor:X}_[0x{reach - cursor:X}];"
                     f"  // 0x{cursor:X}")
    return "\n".join(lines) + "\n"


def extend(text: str, name: str, at: int, length: int, members=()) -> str:
    """Append the tail just before `name`'s closing brace."""
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
        close = index - 1
        return (text[:close] + tail_text(name, at, length, list(members))
                + text[close:])
    return text


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--apply", action="store_true",
                        help="write the headers (default: report only)")
    args = parser.parse_args(argv)

    reason = bm.available()
    if reason:
        print(f"SKIP: {reason}. This asks the real compiler for `sizeof`.")
        return 0

    bounds = lower_bounds()
    home = declaring_header()
    candidates = {name: bound for name, bound in bounds.items()
                  if name in home}
    if not candidates:
        print("SKIP: no class has both a bound and a declaration.")
        return 0

    # The control: a class that will not compile at all must not be reported
    # as too small.
    compiles = probe({name: 1 for name in candidates}, home, "c")
    short = sorted(compiles - probe(candidates, home, "r"))
    print(f"{len(candidates)} class(es) bounded and declared; "
          f"{len(compiles)} compile; {len(short)} declared too small")
    if not short:
        return 0

    sizes = declared_sizes(short, home, candidates)
    members = idb_members()
    by_header = {}
    for name in short:
        # AN EMPTY CLASS IS `sizeof == 1` AND HOLDS NOTHING. That byte exists
        # only so two objects have distinct addresses, and it disappears the
        # moment a real member arrives - so padding from offset 1 lands the
        # class one byte short of the target, which is exactly what AlphaMenu
        # and DeletionList did on the first run.
        at = 0 if sizes[name] <= 1 else sizes[name]
        gap = candidates[name] - at
        if gap <= 0:
            continue
        tail = tail_members(name, at, candidates[name], members)
        by_header.setdefault(home[name], []).append((name, at, gap, tail))
        detail = (f"{len(tail)} IDB member(s)" if tail
                  else "opaque - the IDB starts no member there")
        print(f"   {name:22} declared 0x{sizes[name]:X}, "
              f"proven >= 0x{candidates[name]:X}, adding 0x{gap:X}: {detail}")

    if args.apply:
        for header, additions in by_header.items():
            path = SRC / header
            text = path.read_text()
            for name, at, length, tail in additions:
                text = extend(text, name, at, length, tail)
            path.write_text(text)
        print(f"\nextended {sum(len(v) for v in by_header.values())} class(es) "
              f"across {len(by_header)} header(s)")
    else:
        print("\nre-run with --apply to write them")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
