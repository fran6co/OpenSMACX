#!/usr/bin/env python3
"""Name the fields inside a block of unknown storage, from the image.

A class recovered this far usually still ends in a slab: `uint8_t
field_101F4_[0xB1A0];`, standing for "something is here and nobody has said
what". The cost is not cosmetic. A recovered body that reaches into a slab has
to write

    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x101F4) = a1;

where a declared member would let it write `field_101F4_ = a1;`, and those casts
are not a style problem - they are this file's input, showing up in source form.

WHERE THE FIELDS COME FROM. `verify_member_offsets.py` collects every
`[this + N]` access a class's own thiscall methods make, with its WIDTH. An
access inside a slab is a proof that a scalar of that width starts there: the
instruction reads or writes those bytes as one value. 79 of them land inside
`Datalink`'s tail alone. This turns each into a declaration and leaves the bytes
nothing touches as smaller slabs between them.

WHAT IT WILL NOT SPLIT. Only storage named in this tree's own unknown-storage
idiom - `field_<hex>_`, `unmapped_`, `derived_tail_`. A byte array called
`graphicWin_` or `scroll_` is an embedded OBJECT held as bytes on purpose, and
an access inside it belongs to that object's layout rather than to this class;
splitting one would invent members for a class that already has them elsewhere.
Generated headers are skipped too: an edit there survives until the generator
next runs.

THE CHECK IS THE COMPILER, NOT THE ARITHMETIC. Every rewrite preserves the
member's total extent by construction, but "by construction" is what everything
that has ever been wrong here also claimed. So `--apply` re-probes each edited
class and requires `sizeof` to be UNCHANGED, reverting the file if it moved -
alignment is the way this fails, since a `uint32_t` the tree places at an odd
offset is slid forward by the compiler and takes every later member with it.
"""

from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import verify_member_offsets as offsets  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
# Written by tools/emit_hypothesis_layouts.py; edits here last until it runs.
GENERATED = {"hypothesis_layouts.h"}

# The tree's own spelling for "storage of known extent and unknown content".
# Anything else that happens to be a byte array is an embedded object.
UNKNOWN_STORAGE = re.compile(
    r"^(?:field|unmapped)_(?:[0-9A-Fa-f]+_)?$|^derived_(?:tail|storage)_$")
BYTE_ARRAY = re.compile(r"\b(?:uint8_t|unsigned char|char|BYTE)\b")
SCALAR_FOR = {1: "uint8_t", 2: "uint16_t", 4: "uint32_t"}


def slabs(layouts: dict, sizes: dict) -> dict:
    """{class: [(member, offset, size)]} - the storage this may split."""
    found = {}
    for klass, rows in layouts.items():
        mine = [(name, offset, size) for name, offset, size, spelled in rows
                if "::" not in name and size >= 8
                and BYTE_ARRAY.search(spelled or "")
                and UNKNOWN_STORAGE.match(name)]
        if mine:
            found[klass] = sorted(mine, key=lambda row: row[1])
    return found


def partition(start: int, size: int, touched: dict, typed: dict) -> list:
    """[(offset, width, type)] covering [start, start+size) exactly.

    A width-0 entry is a gap: bytes nothing reaches, kept as a smaller slab.
    An access whose scalar would not be ALIGNED at its offset is dropped
    rather than declared - the compiler would slide it forward and move every
    member after it, and a member the tree cannot spell is better left inside
    a slab than spelled wrong.
    """
    end = start + size
    proved = {}
    for offset, width in sorted(touched):
        if not start <= offset < end or offset + width > end:
            continue
        if offset == start and width == size:
            continue                       # the slab read whole; says nothing
        if width not in SCALAR_FOR and width != 8:
            continue
        if offset % min(width, 4):
            continue                       # would be slid forward; skip it
        proved[offset] = max(proved.get(offset, 0), width)

    out, cursor = [], start
    for offset in sorted(proved):
        if offset < cursor:
            continue                       # already inside a wider member
        width = proved[offset]
        if offset + width > end:
            continue
        if offset > cursor:
            out.append((cursor, 0, ""))
        spelling = SCALAR_FOR.get(width)
        if width == 8:
            # Eight bytes is only ever a `double` here, and only when x87 said
            # so; two dwords otherwise, which the gap logic handles.
            spelling = "double" if (offset, 8) in typed else None
            if spelling is None:
                continue
        out.append((offset, width, spelling))
        cursor = offset + width
    if cursor < end:
        out.append((cursor, 0, ""))
    return [(offset, width, spelling) for offset, width, spelling in out
            if width or True]


def declarations(pieces: list, end: int, typed: dict) -> list:
    """The replacement lines, in this tree's `field_<HEX>_` house style."""
    lines = []
    for index, (offset, width, spelling) in enumerate(pieces):
        stop = pieces[index + 1][0] if index + 1 < len(pieces) else end
        if width:
            kind = spelling
            if width == 4 and (offset, 4) in typed:
                kind = "float"
            lines.append(f"  {kind} field_{offset:X}_;  // 0x{offset:X}")
        else:
            span = stop - offset
            if span <= 0:
                continue
            lines.append(f"  uint8_t field_{offset:X}_[0x{span:X}];"
                         f"  // 0x{offset:X}")
    return lines


DECLARATION = ("^[ \t]*(?:public|private|protected)?[ \t]*:?[ \t]*"
               r"(?:uint8_t|unsigned char|char|BYTE)[ \t]+%s[ \t]*"
               r"\[[^\]]*\][ \t]*;.*$")


def rewrite(text: str, member: str, lines: list) -> str:
    """Replace one slab declaration with the lines that split it."""
    pattern = re.compile(DECLARATION % re.escape(member), re.M)
    found = pattern.search(text)
    if not found:
        return text
    return text[:found.start()] + "\n".join(lines) + text[found.end():]


def plan(layouts: dict, sizes: dict, observed: dict, typed: dict,
         home: dict) -> dict:
    """{header: [(class, member, [lines], how many fields named)]}."""
    work = collections.defaultdict(list)
    for klass, members in sorted(slabs(layouts, sizes).items()):
        header = home.get(klass)
        if not header or header in GENERATED:
            continue
        for member, offset, size in members:
            pieces = partition(offset, size, observed.get(klass, {}),
                               typed.get(klass, {}))
            named = sum(1 for _, width, _ in pieces if width)
            if not named:
                continue
            lines = declarations(pieces, offset + size, typed.get(klass, {}))
            work[header].append((klass, member, lines, named))
    return work


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true",
                        help="rewrite the headers (default is to report)")
    parser.add_argument("--class", dest="only", help="one class only")
    parser.add_argument("--src", type=Path, default=SRC)
    args = parser.parse_args(argv)

    import byte_match as bm

    reason = bm.available()
    if reason:
        print(f"SKIP: {reason}")
        return 0

    observed, typed = offsets.load_accesses()
    if not observed:
        print(f"SKIP: {offsets.ACCESSES_CSV.name} is absent; run "
              "verify_member_offsets.py --publish first")
        return 0

    layouts, sizes, refused = offsets.declared(args.src)
    for line in refused:
        print(f"  refused {line}")
    home = {klass: header
            for klass, (header, _, _) in offsets.classes(args.src).items()}

    work = plan(layouts, sizes, observed, typed, home)
    if args.only:
        work = {header: [row for row in rows if row[0] == args.only]
                for header, rows in work.items()}
        work = {header: rows for header, rows in work.items() if rows}

    total = sum(named for rows in work.values() for *_, named in rows)
    print(f"{total} field(s) the image proves, in "
          f"{sum(len(rows) for rows in work.values())} slab(s) across "
          f"{len(work)} header(s)")
    for header, rows in sorted(work.items()):
        for klass, member, lines, named in rows:
            print(f"  {header:<20} {klass}.{member}: {named} field(s)")

    if not args.apply:
        return 0

    before = {klass: sizes.get(klass) for klass in
              {row[0] for rows in work.values() for row in rows}}
    originals = {}
    for header, rows in sorted(work.items()):
        path = args.src / header
        originals[header] = text = path.read_text()
        for klass, member, lines, _named in rows:
            text = rewrite(text, member, lines)
        path.write_text(text)

    # THE COMPILER DECIDES. Alignment is how this fails, and it fails quietly:
    # a scalar the tree places where the ABI would not slides forward and takes
    # every member after it with it, which no arithmetic in this file would
    # notice.
    after, sizes_after, refused_after = offsets.declared(args.src)
    moved = [(klass, before[klass], sizes_after.get(klass))
             for klass in before if sizes_after.get(klass) != before[klass]]
    if moved or refused_after:
        for header, text in originals.items():
            (args.src / header).write_text(text)
        for klass, was, now in moved:
            print(f"REVERTED: sizeof({klass}) moved "
                  f"0x{was:X} -> "
                  f"{'?' if now is None else f'0x{now:X}'}")
        for line in refused_after:
            print(f"REVERTED: {line}")
        return 1
    print(f"applied; every edited class kept its size "
          f"({len(before)} classes checked)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
