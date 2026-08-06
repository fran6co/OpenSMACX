#!/usr/bin/env python3
"""Give `src/`'s offset-named members the names the sources know.

1,170 of the 2,368 data members declared across `src/*.h` are called
`field_9E8_`, where the hex IS the offset and the name says nothing else -
`Palette` is 257 of 257. The IDA database and the Thinker mod both describe
members at those offsets, and some of what they describe carries a real name.

THIS CANNOT MOVE A BYTE, which is what makes it safe where importing a size is
not. A rename keeps the declared type and therefore the width; the offsets are
already fixed by the surrounding declarations; and 40 of these classes carry
`static_assert(sizeof(X) == N)`, so the build fails outright if the layout
changes. A wrong name here is a misleading identifier. A wrong size would be an
offset no agent can ever match.

THE MATCH IS ON OFFSET AND WIDTH, BOTH. A source member at 0x58 renames
`field_58_` only if the source also agrees it is four bytes wide. Agreeing on
the offset alone would let a `char[4]` rename a `uint32_t` and quietly assert
something about the type that neither source established.

ANYTHING ELSE IS REPORTED, NOT APPLIED. A source member landing on an offset
`src/` has no member at means either the source's table is truncated - the IDB
accumulates offsets, so one missing member shifts every later one - or `src/`'s
layout is wrong. Both want a person, and neither wants this tool guessing.
"""

from __future__ import annotations

import argparse
import collections
import csv
import keyword
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import class_layouts  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
IDB_MEMBERS = REPO_ROOT / "docs" / "recovery" / "idb-members.csv"
THINKER = REPO_ROOT / "docs" / "recovery" / "thinker-members.csv"
BEHAVIOUR = REPO_ROOT / "docs" / "recovery" / "behaviour-member-names.csv"
GENERATED = "hypothesis_layouts.h"

PLACEHOLDER_MEMBER = re.compile(
    r"^(?P<indent>\s*)(?P<type>(?:const\s+)?[A-Za-z_]\w*)\s*(?P<stars>\*+)?\s*"
    r"field_(?P<offset>[0-9A-Fa-f]+)_\s*(?P<array>\[[^\]]*\])?\s*;",
    re.MULTILINE)

# Names that carry no information. Beyond IDA's own `field_9E8` there is
# Thinker's notation for the same idea: `dwordC`, `dword10`, `dword14` are the
# member's TYPE followed by its offset, and adopting one would replace `src/`'s
# placeholder with a second tree's placeholder. The hex tail must be the whole
# remainder, so `wordCount` and `byteOrder` are still real names.
PLACEHOLDER_NAME = re.compile(
    r"^(field_[0-9A-Fa-f]+|unk\w*|gap\w*|pad\w*|_?\d+"
    r"|(?:dword|qword|word|byte)[0-9A-Fa-f]*|)$")
IDENTIFIER = re.compile(r"^[A-Za-z_]\w*$")

WIDTH = {
    "bool": 1, "char": 1, "int8_t": 1, "uint8_t": 1,
    "short": 2, "int16_t": 2, "uint16_t": 2, "wchar_t": 2,
    "int": 4, "long": 4, "float": 4, "int32_t": 4, "uint32_t": 4,
    "double": 8, "int64_t": 8, "uint64_t": 8,
}


def declared_width(type_name: str, stars: str, array: str) -> int:
    """Bytes a declaration occupies, or 0 when this tool cannot say.

    0 means "do not rename", never "assume four". A width guessed wrong turns
    the offset-and-width match into an offset-only match, which is the check
    this tool exists to make.
    """
    if stars:
        base = 4  # 32-bit target
    else:
        base = WIDTH.get(type_name.replace("const", "").strip(), 0)
    if not base:
        return 0
    if not array:
        return base
    count = array.strip("[]").strip()
    try:
        return base * int(count, 0)
    except ValueError:
        return 0


def source_members() -> dict:
    """{class: {offset: (name, size, source)}} with a real name, both sources.

    The IDB is read first and Thinker second, so where both name the same
    offset Thinker wins: its offsets are explicit in its headers rather than
    accumulated from member sizes, and its field meanings have been validated
    for years by the mod working.
    """
    found = collections.defaultdict(dict)
    for path, columns, label in (
            (IDB_MEMBERS, ("class", "offset", "name", "size"), "the IDB"),
            (THINKER, ("struct", "offset", "field", "size"), "Thinker"),
            # Last, so it wins. A behaviour-derived name is read out of the
            # image rather than out of somebody's notes, and it reproduced
            # Wave's pitch_, reverb_mix_ and ms_length_ - three names this tree
            # had already recovered by hand - exactly.
            (BEHAVIOUR, ("class", "offset", "name", "size"), "the image")):
        if not path.is_file():
            continue
        klass, offset_column, name_column, size_column = columns
        with path.open(newline="", encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                name = (row.get(name_column) or "").strip()
                if PLACEHOLDER_NAME.match(name) or not IDENTIFIER.match(name):
                    continue
                if keyword.iskeyword(name):
                    continue
                try:
                    offset = int(row[offset_column], 16)
                    size = int(row[size_column])
                except (ValueError, KeyError, TypeError):
                    continue
                found[row[klass]][offset] = (name, size, label)
    return found


def unique(name: str, taken: set) -> str:
    candidate = name if name.endswith("_") else f"{name}_"
    while candidate in taken:
        candidate = f"{candidate}_"
    return candidate


def plan(text: str, known: dict) -> tuple:
    """(renames, skipped) for one header's text.

    renames: [(old, new, offset, source)] ; skipped: [(reason, detail)]
    """
    renames, skipped = [], []
    for name, _, body in class_layouts.class_bodies(text):
        members = known.get(name)
        if not members:
            continue
        # The WHOLE identifier, trailing underscore included. Capturing
        # `iFlags` out of `iFlags_;` and then testing the candidate `iFlags_`
        # against it never matches, so the collision check silently passed and
        # two members could be given the same name.
        taken = set(re.findall(r"\b(\w+)\s*(?:\[[^\]]*\])?\s*;", body))
        for match in PLACEHOLDER_MEMBER.finditer(body):
            offset = int(match.group("offset"), 16)
            candidate = members.get(offset)
            if not candidate:
                continue
            new_name, size, label = candidate
            width = declared_width(match.group("type"),
                                   match.group("stars") or "",
                                   match.group("array") or "")
            if not width:
                skipped.append((f"{name}.field_{offset:X}_",
                                f"cannot size `{match.group('type')}`"))
                continue
            if width != size:
                skipped.append((
                    f"{name}.field_{offset:X}_",
                    f"{label} says {size} byte(s), src/ declares {width}"))
                continue
            final = unique(new_name, taken)
            taken.add(final)
            renames.append((f"field_{offset:X}_", final, offset, label))
    return renames, skipped


def orphans(text: str, known: dict) -> list:
    """Source members on offsets `src/` declares no member at.

    Reported and never applied. Either the source table is truncated - the
    IDB's offsets accumulate, so one member nobody entered shifts every later
    one - or `src/`'s layout is wrong. Both want a person.
    """
    out = []
    for name, _, body in class_layouts.class_bodies(text):
        members = known.get(name)
        if not members:
            continue
        declared = {int(found.group("offset"), 16)
                    for found in PLACEHOLDER_MEMBER.finditer(body)}
        real = {int(found, 16) for found in
                re.findall(r"\bfield_([0-9A-Fa-f]+)_", body)}
        for offset, (member, _, label) in sorted(members.items()):
            if offset not in declared and offset not in real:
                out.append((name, offset, member, label))
    return out


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--apply", action="store_true",
                        help="write the headers (default: report only)")
    parser.add_argument("--show-orphans", action="store_true",
                        help="list source members on offsets src/ has none at")
    args = parser.parse_args(argv)

    known = source_members()
    if not known:
        print("SKIP: neither member table is present.")
        return 0

    applied = skipped_total = 0
    for header in sorted(SRC.glob("*.h")):
        if header.name == GENERATED:
            continue
        text = header.read_text(errors="ignore")
        renames, skipped = plan(text, known)
        skipped_total += len(skipped)
        for reason, detail in skipped:
            print(f"   skipped {reason}: {detail}")
        if not renames:
            continue
        updated = text
        for old, new, offset, label in renames:
            # Word-bounded: `field_5_` must not match inside `field_50_`.
            updated = re.sub(rf"\b{re.escape(old)}\b", new, updated)
            print(f"   {header.name}: {old} -> {new} "
                  f"(0x{offset:X}, from {label})")
        applied += len(renames)
        if args.apply:
            header.write_text(updated)

    if args.show_orphans:
        for header in sorted(SRC.glob("*.h")):
            if header.name == GENERATED:
                continue
            for name, offset, member, label in orphans(
                    header.read_text(errors="ignore"), known):
                print(f"   orphan {name} 0x{offset:X} {member} ({label}) - "
                      f"src/ declares no member there")

    verb = "renamed" if args.apply else "would rename"
    print(f"\n{applied} placeholder(s) {verb}, {skipped_total} refused")
    if not args.apply and applied:
        print("re-run with --apply to write them")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
