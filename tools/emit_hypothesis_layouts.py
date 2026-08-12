#!/usr/bin/env python3
"""Declare the classes the binary has and `src/` does not.

Classes own unrecovered functions whose agent is handed an opaque shell and
told nothing about the receiver it is writing through. 86 such classes are
declared here, 27 of them owning functions in the image; afterwards **6 classes
owning 19 unrecovered functions** have no declaration anywhere, and for four of
those no source in this tree knows anything at all - not a member, not a size.

A partial declaration is knowledge. The type exists, some members have names
and offsets, and the rest is storage - and none of it can contradict anything,
because none of these classes has a pinned size to contradict.

(An earlier version of this file claimed 51 classes and 213 functions. Those
came from a scope regex that read `?f@@YAXPAUGraphicWin@@@Z` - a free function
taking a `GraphicWin *` - as a class called `YAXPAUGraphicWin`. The counts were
inflated by free functions; the parse is fixed above and the numbers here are
the corrected ones.)

WHAT THIS DOES NOT DO. It writes no `static_assert`. A compile-time size
assertion is believed by everything downstream and checked by nothing, so it
stays behind `derive_class_layout.py --score-csv` at zero wrong, and these
sizes are hypotheses: the IDB's accumulate from member sizes, so a member
nobody entered shifts every member after it - `PullDown` records one member of
0xa14 against a true 0xf40. `class_layouts.pinned_layouts()` reads only classes
carrying that assertion, so nothing here can reach an agent as a proved layout
or slip into `verified-layouts.txt` by verifying against itself.

EVERY MEMBER IS RAW STORAGE, never another class by value. `BaseButton`'s
first member is 0xa14 bytes and that is a `GraphicWin`, but emitting it as one
would make this header's classes depend on each other's definitions and on
emission order. That is not a hypothetical cost: by-value layout members broke
675 units once and alphabetical emission order broke 77 more. A byte array
holds the same offsets and depends on nothing.

WHERE THE MEMBERS COME FROM, strongest first. The IDB for coverage, since it
carries every member of a class or none. Thinker for names wherever it
describes the same offset - its offsets are explicit in its headers rather than
accumulated, and its field meanings have been validated for years by the mod
working. And for a class neither has heard of, the access lower bound: it names
nothing, but a class known to be at least 0x40 bytes is a better receiver than
a type the tree has never declared. Neither external source's text enters the
repository; both are read through committed offset/name CSVs.
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
import derive_base_edges  # noqa: E402


def _catalogue_rows():
    """Every catalogued row, from `src/`.

    `docs/recovery/functions.csv` is deleted: every `ORIGINAL:` annotation
    carries its own name, size, spans, prototype, kind, flags and call
    edges, and `emit.load_functions()` reads them back. This tool opened
    the CSV directly, so it broke the moment the store moved - which is
    how five layout gates went red at once.
    """
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import emit_translation_unit as _emit
    return list(_emit.load_functions().values())

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
IDB_MEMBERS = REPO_ROOT / "docs" / "recovery" / "idb-members.csv"
# The committed reduction, not the ignored working copy under
# .opensmacx/external-analysis/. Both hold the same thing - a struct name, a
# field name, an offset and a size - and the policy that keeps Thinker's TEXT
# out of the repository has never covered extracted offsets and names, which is
# also why functions.csv and ida9-functions.csv are committed. Reading the
# ignored copy would make this generator's output depend on a file a clean
# checkout does not have, so `--check` would fail for everyone but its author.
THINKER = REPO_ROOT / "docs" / "recovery" / "thinker-members.csv"
THINKER_LOCAL = (REPO_ROOT / ".opensmacx" / "external-analysis" /
                 "thinker-layout-hypotheses.csv")
ACCESS_BOUNDS = REPO_ROOT / "docs" / "recovery" / "access-lower-bounds.csv"
# The only member source derived from bodies PROVED byte-identical.
OBSERVATIONS = (REPO_ROOT / "docs" / "recovery"
                / "agent-structure-observations.csv")
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
OUTPUT = SRC / "hypothesis_layouts.h"

# The class a member function belongs to, and nothing for a free function.
# An ordinary method spells `?name@Class@@`; a SPECIAL name has no name segment
# at all - `??0Buffer@@` is `??`, the operator code, then the class.
#
# The scope must be NON-EMPTY, which is what refuses a free function. A lazy
# `[~\w@]*?@(\w+)@@` does not: on `?f@@YAXPAUGraphicWin@@@Z` it consumes `f@`
# and then reads the convention code and the parameter type as a class,
# inventing scopes called `YAXPAUGraphicWin` and `YAHHHHHHPAUCaviar`.
SCOPE_RE = re.compile(
    r"^(?:\?\?(?:_[A-Z]|[0-9A-Z])|\?[\w_]+@)([\w_]+)(?:@[\w_]+)*@@")
PLACEHOLDER = re.compile(r"^(field_[0-9A-Fa-f]+|unk\w*|gap\w*|pad\w*|_?\d+|)$")
IDENTIFIER = re.compile(r"^[A-Za-z_]\w*$")


def declared_in_src() -> set:
    names = set()
    for header in sorted(SRC.glob("*.h")):
        if header.name == OUTPUT.name:
            continue
        for name, _, _ in class_layouts.class_bodies(
                header.read_text(errors="ignore")):
            names.add(name)
    return names


def idb_members() -> dict:
    """{class: [(offset, name, size)]} in offset order."""
    found = collections.defaultdict(list)
    if not IDB_MEMBERS.is_file():
        return found
    with IDB_MEMBERS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            found[row["class"]].append(
                (int(row["offset"], 16), row["name"], int(row["size"])))
    for members in found.values():
        members.sort()
    return found


def thinker_members() -> dict:
    """{struct: {offset: (name, size)}}. Sizes the reducer could not work out
    arrive empty and are kept only for their NAME, never for layout."""
    found = collections.defaultdict(dict)
    if not THINKER.is_file():
        return found
    with THINKER.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            try:
                offset = int(row["offset"], 16)
            except (ValueError, KeyError, TypeError):
                continue
            try:
                size = int(row["size"])
            except (ValueError, KeyError, TypeError):
                size = 0
            found[row["struct"]][offset] = (row["field"], size)
    return found


def regenerate_thinker() -> int:
    """Refresh the committed reduction from the ignored working copy."""
    if not THINKER_LOCAL.is_file():
        print(f"SKIP: {THINKER_LOCAL} is absent; run "
              f"tools/correlate_thinker_layouts.py first.")
        return 0
    with THINKER_LOCAL.open(newline="", encoding="utf-8-sig") as handle:
        rows = list(csv.DictReader(handle))
    rows.sort(key=lambda row: (row["struct"],
                               int(row["offset"], 16) if row["offset"] else 0,
                               row["field"]))
    with THINKER.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle, fieldnames=["struct", "offset", "field", "size"],
            lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow({key: row[key] for key in
                             ("struct", "offset", "field", "size")})
    print(f"{len(rows)} Thinker field(s) -> {THINKER}")
    return 0


def access_bounds() -> dict:
    """{class: the size its own code proves it must be at least}.

    The weakest source here and the only one for a class neither the IDB nor
    Thinker has heard of. It names no member, but a class known to be 0x40
    bytes is still a better receiver than a class the tree has never declared:
    the type exists and its extent is right.
    """
    found = {}
    if not ACCESS_BOUNDS.is_file():
        return found
    with ACCESS_BOUNDS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            try:
                found[row["class"]] = int(row["lower_bound"], 16)
            except (ValueError, KeyError, TypeError):
                continue
    return found


def proved_members() -> dict:
    """{class: {offset: (name, size)}} from bodies proved BYTE_EXACT.

    THE STRONGEST SOURCE HERE, and the last to be wired in. Every other input
    is an outside opinion: the IDB is analysis, Thinker is a mod's reading,
    and the access bounds only say how far the image reaches. These come from
    `agent-structure-observations.csv`, written while recovering a function
    whose body was then proved byte-identical - so the offset is not a
    hypothesis about the layout, it is a fact the comparison would have caught.

    WHY IT MATTERS FOR READABILITY, not only accuracy: a recovered body that
    reaches an UNDECLARED field must write
    `*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x2144) = a1;`
    where a declared one writes `field_2144_ = a1;`. `GAmbience::begin` does
    exactly the latter, and only because this file happens to declare its
    member; `Midi` is one opaque `uint8_t field_0_[0x50]`, so every Midi body
    casts. The casts in recovered code were never a style problem - they are
    this file's gaps, showing up in source form.

    NEGATIVE offsets are skipped. They are real, and they are the most
    interesting rows in that file - `[ecx-0x1c]` means the class is a
    SUBOBJECT of something larger - but a member cannot be declared at a
    negative offset, and inventing the enclosing class from one access would
    be exactly the kind of guess this file exists to avoid.
    """
    found = collections.defaultdict(dict)
    if not OBSERVATIONS.is_file():
        return found
    with OBSERVATIONS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            if row.get("kind") not in ("member", "member-type"):
                continue
            if row.get("status") == "rejected":
                continue
            subject = (row.get("subject") or "").strip()
            offset = (row.get("offset") or "").strip()
            # The subject must NAME a class. Observations are written by hand
            # and several say things like "the class read by sub_5e3630" -
            # true, useful to a reader, and not something a declaration can be
            # emitted for. Those wait until someone identifies the type.
            if not IDENTIFIER.match(subject):
                continue
            if not offset.lower().startswith("0x"):
                continue               # negative, or not an offset at all
            try:
                at = int(offset, 16)
            except ValueError:
                continue
            width = 1 if "one byte" in (row.get("detail") or "") else 4
            found[subject][at] = (f"field_{at:x}_", width)
    return found


def member_name_for(type_name: str) -> str:
    """`SubInterface` -> `sub_interface_`, so the member reads as itself."""
    parts = re.findall(r"[A-Z]+(?![a-z])|[A-Z][a-z0-9]*|[a-z0-9]+", type_name)
    return "_".join(part.lower() for part in parts) + "_"


def proved_subobjects() -> dict:
    """{class: {offset: (member, type)}} for EMBEDDED sub-objects.

    Kept apart from `proved_members` because the two are not the same claim
    and cannot be emitted the same way. A member has a known width; an
    embedded sub-object's size is exactly what nobody knows yet, so it can
    only ever be declared LAST - anything after it would sit at an offset
    this file just invented.

    This is the other half of the reinterpret_cast story. `proved_members`
    already explains why an undeclared FIELD forces a cast; an undeclared
    SUB-OBJECT forces the uglier one, because there is no field to name at
    all:

        ((SubInterface *)((char *)this + 0xA14))->release_iface_mode();

    against what a declaration buys:

        sub_interface_.release_iface_mode();

    Both compile to `lea ecx,[esi+0xa14]; call ...`, so this is readability
    bought at no cost to the proof - the byte comparison cannot tell them
    apart, which is precisely why it is safe to prefer the second.

    The evidence for this one is unusually strong: five distinct classes -
    BaseWin, CouncWin, Datalink, DiploWin, SocialWin - place SubInterface at
    the SAME +0xA14, every one of them established by a body that then
    verified byte-identical. Five classes agreeing on an offset is a shared
    base, and no `src/*.h` declares any inheritance at all. Declaring the
    sub-object per class is what can be justified TODAY; naming the common
    base needs its size, which is still unknown.

    NEGATIVE offsets are skipped for the same reason as in `proved_members`,
    and they mean the same thing: `-0xA14` says this class is itself a
    sub-object of something larger. That is the more interesting fact and
    still not one a declaration can express.
    """
    found = collections.defaultdict(dict)
    if not OBSERVATIONS.is_file():
        return found
    with OBSERVATIONS.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            if row.get("kind") != "embedded":
                continue
            if row.get("status") == "rejected":
                continue
            subject = (row.get("subject") or "").strip()
            offset = (row.get("offset") or "").strip()
            if not IDENTIFIER.match(subject):
                continue
            if not offset.lower().startswith("0x"):
                continue                       # negative, or not an offset
            try:
                at = int(offset, 16)
            except ValueError:
                continue
            # The TYPE is the leading identifier of the detail, which is how
            # these rows are written: "SubInterface sub-object embedded at
            # +0xA14". A row that does not start by naming a type is not
            # actionable - it describes a sub-object nobody has identified.
            #
            # It must LOOK like a class name, which here means starting with a
            # capital. Accepting any leading word turned BattleWin's
            # "a Win sub-object at +8" into a member of type `a`, which
            # compiles to nothing and reads as a finding.
            lead = re.match(r"\s*([A-Z]\w*)", row.get("detail") or "")
            if lead is None or lead.group(1) == subject:
                continue
            found[subject][at] = (member_name_for(lead.group(1)),
                                  lead.group(1))
    return found


def owns_functions() -> collections.Counter:
    owned = collections.Counter()
    # `src/` is the catalogue's store; the export is deleted.
    for row in _catalogue_rows():
        match = SCOPE_RE.match(row.get("name") or "")
        if match:
            owned[match.group(1)] += 1
    return owned


def storage(size: int, name: str) -> str:
    """A member declaration holding exactly `size` bytes and nothing else.

    Never another class by value: see the module docstring. 1, 2 and 4 get the
    integer of that width because that is what the tree writes for a scalar
    member; anything else is a byte array, which is honest about the fact that
    the only thing known here is how much room it takes.
    """
    if size == 1:
        return f"uint8_t {name};"
    if size == 2:
        return f"uint16_t {name};"
    if size == 4:
        return f"uint32_t {name};"
    return f"uint8_t {name}[0x{size:X}];"


def member_name(raw: str, offset: int, taken: set) -> str:
    """The source's name when it says anything, else the tree's own idiom.

    `src/` names a member it has an offset for and nothing else `field_9E8_`,
    where the hex IS the offset, so a placeholder from either source is
    rewritten into that shape rather than carried across as `field_9E8`.
    """
    name = raw.strip()
    if (not IDENTIFIER.match(name) or keyword.iskeyword(name)
            or PLACEHOLDER.match(name)):
        name = f"field_{offset:X}_"
    elif not name.endswith("_"):
        name = f"{name}_"
    while name in taken:
        name = f"{name}_"
    taken.add(name)
    return name


def layout_for(name: str, idb: dict, thinker: dict, bounds=None,
               proved=None) -> tuple:
    """([(offset, name, size)], provenance) - the members to declare.

    The IDB carries every member or none, so it decides the shape. Thinker
    supplies a NAME wherever it describes the same offset and the IDB left a
    placeholder there; where the IDB has nothing at all, Thinker's own prefix
    is used and the gaps between its offsets become padding.
    """
    bound = (bounds or {}).get(name, 0)

    def extended(members: list, source: str) -> tuple:
        """The tail the image proves is there, whatever named it up to now.

        THE ACCESS BOUND OUTRANKS A MEMBER TABLE, in the one direction it
        speaks: a class whose own method writes its 0x36D8th byte has that
        byte, however few members somebody entered into the database. Applying
        it only where nothing was named - which is how this started - left the
        IDB's truncations standing, and `verify_member_offsets.py` reads them
        straight off the instruction stream: `MonuWin` declared 0x366C against
        28 accesses above it, up to `mov dword ptr [esi + 0x36d8], 0x6b`.
        `PullDown` recording one member of 0xA14 against a true 0xF40 is the
        same defect, and this is what closes it for both.
        """
        end = max((offset + size for offset, _, size in members), default=0)
        if bound > end:
            members = members + [(end, "", bound - end)]
            source = f"{source}, extended to the access bound"
        return members, source

    if name in idb:
        members, source = list(idb[name]), "the IDB"
        named = thinker.get(name, {})
        improved = 0
        for index, (offset, member, size) in enumerate(members):
            candidate = named.get(offset)
            if (candidate and PLACEHOLDER.match(member)
                    and not PLACEHOLDER.match(candidate[0])):
                members[index] = (offset, candidate[0], size)
                improved += 1
        if improved:
            source = f"the IDB, {improved} name(s) from Thinker"
        return extended(members, source)

    # Thinker only: explicit offsets, a prefix rather than a whole struct, so
    # the space between two it names has to be declared as padding or every
    # offset after a gap would be wrong.
    members, cursor = [], 0
    for offset in sorted(thinker.get(name, {})):
        member, size = thinker[name][offset]
        if offset < cursor:
            continue
        if offset > cursor:
            members.append((cursor, "", offset - cursor))
        if size <= 0:
            continue
        members.append((offset, member, size))
        cursor = offset + size
    if members:
        return extended(members, "Thinker")

    # Neither the IDB nor Thinker has heard of this class, but a recovered
    # body that was PROVED byte-identical reached into it, and that says where
    # a field is with more authority than either. It says nothing about the
    # ones nobody has touched, so the gaps stay padding and the access bound -
    # if there is one - extends the tail.
    #
    # This is what stops a recovered body writing
    # `*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x2144) = a1;`
    # where `field_2144_ = a1;` would do. `Midi` reaching here as one opaque
    # `uint8_t field_0_[0x50]` is why every Midi body casts.
    proved = (proved or {}).get(name, {})
    bound = (bounds or {}).get(name, 0)
    if proved:
        members, cursor = [], 0
        for offset in sorted(proved):
            member, size = proved[offset]
            if offset < cursor:
                continue
            if offset > cursor:
                members.append((cursor, "", offset - cursor))
            members.append((offset, member, size))
            cursor = offset + size
        if bound > cursor:
            members.append((cursor, "", bound - cursor))
        return members, "bodies proved byte-identical"

    # Nothing names a member, but the class's own code proves how far into it
    # the image reaches. One block of storage, no names.
    if bound > 0:
        return [(0, "", bound)], "its own code, which reaches that far"
    return [], "nothing"


def render(names: list, idb: dict, thinker: dict,
           owned: collections.Counter, bounds=None, bases=None,
           proved=None) -> str:
    lines = [
        "/*",
        " * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.",
        " * Copyright (C) 2013-2021 Brendan Casey",
        " *",
        " * OpenSMACX is free software: you can redistribute it and / or modify",
        " * it under the terms of the GNU General Public License as published by",
        " * the Free Software Foundation, either version 3 of the License, or",
        " * (at your option) any later version.",
        " *",
        " * OpenSMACX is distributed in the hope that it will be useful,",
        " * but WITHOUT ANY WARRANTY; without even the implied warranty of",
        " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
        " * GNU General Public License for more details.",
        " *",
        " * You should have received a copy of the GNU General Public License",
        " * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.",
        " */",
        "#pragma once",
        "",
        "/*",
        " * GENERATED by tools/emit_hypothesis_layouts.py. Do not edit; edit the",
        " * generator, or promote a class out of this file into one of its own.",
        " *",
        " * Classes the binary has and the rest of src/ does not declare. Their",
        " * offsets come from the IDA database and the Thinker mod, and they are",
        " * HYPOTHESES: no member here is proved against the image.",
        " *",
        " * There is deliberately no static_assert. A compile-time size assertion",
        " * is believed by everything downstream and checked by nothing, so it",
        " * needs derive_class_layout.py --score-csv at zero wrong first - and",
        " * these sizes cannot pass it. The IDB's offsets ACCUMULATE from member",
        " * sizes, so one member nobody entered shifts every member after it:",
        " * PullDown records a single member of 0xa14 against a true 0xf40.",
        " *",
        " * Because there is no assertion, class_layouts.pinned_layouts() does not",
        " * read these, so none of them can reach an agent as a proved layout or",
        " * enter verified-layouts.txt by verifying against itself.",
        " *",
        " * Every member is raw storage rather than another class by value.",
        " * BaseButton's first member is 0xa14 bytes and is a GraphicWin, but",
        " * declaring it as one would make these classes depend on each other and",
        " * on emission order - by-value layout members once broke 675 units and",
        " * alphabetical order 77 more. A byte array holds the same offsets and",
        " * depends on nothing.",
        " */",
        "",
        '#include "stdafx.h"',
        "",
    ]

    for name in names:
        members, provenance = layout_for(name, idb, thinker, bounds, proved)
        if not members:
            continue
        total = max((offset + size for offset, _, size in members), default=0)
        functions = owned.get(name, 0)
        # The user's rule for this tree: methods make it a class, data alone
        # makes it a struct. These carry no method declarations, so the binary
        # is what is asked - does anything dispatch on this receiver?
        keyword_ = "class" if functions else "struct"
        real = sum(1 for _, member, _ in members
                   if member and not PLACEHOLDER.match(member))
        # An inheritance edge the image confirms. It changes no byte here -
        # these classes are emitted flat on purpose - but it says what the
        # leading bytes ARE, which is the thing a flat layout cannot.
        base = (bases or {}).get(name)
        if base:
            lines.append(f"/* Derives from {base}: its constructor builds one "
                         f"on an unadjusted `this`. See "
                         f"docs/recovery/base-edges.csv. */")
        lines.append(f"/* 0x{total:X} bytes, {len(members)} member(s), "
                     f"{real} named. From {provenance}."
                     + (f" {functions} function(s) in the image." if functions
                        else "")
                     + " */")
        lines.append(f"{keyword_} {name} {{")
        if keyword_ == "class":
            lines.append(" public:")
        taken = set()
        for offset, member, size in members:
            if size <= 0:
                continue
            lines.append(
                f"  {storage(size, member_name(member, offset, taken))}"
                f"  // 0x{offset:X}")
        lines.append("};")
        lines.append("")
    return "\n".join(lines) + "\n" if lines else ""


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed header is stale")
    parser.add_argument("--refresh-thinker", action="store_true",
                        help="rebuild docs/recovery/thinker-members.csv from "
                             "the ignored working copy")
    args = parser.parse_args(argv)

    if args.refresh_thinker:
        return regenerate_thinker()

    idb, thinker = idb_members(), thinker_members()
    if not idb and not thinker:
        print("SKIP: neither idb-members.csv nor the Thinker hypothesis CSV "
              "is present.")
        return 0

    declared = declared_in_src()
    bounds = access_bounds()
    proved = proved_members()
    names = sorted((set(idb) | set(thinker) | set(bounds) | set(proved))
                   - declared)
    owned = owns_functions()
    text = render(names, idb, thinker, owned, bounds,
                  derive_base_edges.agreed(), proved)

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        print(f"hypothesis layouts: {len(names)} classes (up to date)")
        return 0

    args.out.write_text(text)
    unrecovered = sum(1 for name in names if owned.get(name))
    print(f"{len(names)} classes src/ did not declare -> {args.out}")
    print(f"    {unrecovered} of them own functions in the image")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
