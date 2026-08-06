#!/usr/bin/env python3
"""Read class member tables out of the IDA database, without IDA.

IDA Free 9.4 cannot open this database at all - it is a 32-bit `.idb` and the
free build refuses it - so `File > Produce file > Create C header file` is not
available and no route that begins inside IDA can run here. `python-idb` reads
the container directly, which is what this does, and it supersedes the two
export scripts that had to be run from IDA's own script menu: this yields
member NAMES and OFFSETS where they yielded a name and a total size.

THE FORMAT, measured rather than looked up:

  * A class name resolves to a netnode in the 0xFF000000+ struct space -
    `Console` gives 0xff034e43 - while a FUNCTION name resolves to a netnode
    whose id is its address. Lookups for invented names raise, so the
    distinction is real and not an artifact of the reader.
  * `$ structs` is NOT an index of them. It lists 68 entries, nearly all
    Windows SDK types, and none of the game classes; the game classes are
    reachable only by name. So candidates come from `src/`, from the mangled
    class scopes in `functions.csv`, and from the Thinker hypothesis CSV.
  * Under tag `M` the member table is IDA packed dwords: a header word, a
    member count, then five-tuples of
    `(member netnode id, ?, size, flags, ?)`. Blobs longer than 1024 bytes are
    split across successive `M` entries and must be concatenated in index
    order - `Win` arrives in three pieces.
  * The header word is IDA's `SF_ALIGN` field, bits 7..10, holding alignment
    as a power of two. `Sprite` is 0x1110, decoding to align 4, which is
    exactly the 0x29 -> 0x2c it needs.
  * OFFSETS ARE NOT RECORDED. They accumulate from the member sizes, so one
    member missing from the table shifts every member after it. That is not
    hypothetical: see `size` below.
  * Each member's own netnode carries its name under tag `N` as
    `Class.member`, and its TYPE under tag `S` at index 0x3000 - `HFONT` for
    `Font.fontObj`. Only the named-type encoding is decoded here; anything
    else is left blank rather than guessed at, and the size still carries the
    layout.

WHAT THIS IS NOT GOOD FOR. Scored against the 40 classes whose size is pinned
in `src/`, the accumulated totals read 33 right, 2 wrong, 5 silent - and the
two wrong ones are DATA, not parsing. `PullDown` records ONE member of 0xa14
against a true 0xf40, members that whoever built the database never entered;
`Console` parses 145 of 145 and still sums 4 short. No parser recovers a
member that was never typed in, and nothing distinguishes the classes with no
pinned size from that case.

So the `size` column here is a hypothesis and must never become a
`static_assert` without `derive_class_layout.py --score-csv` at zero wrong.
The `name` and `type` columns are a different matter: `functions.csv` holds
4,821 mangled function symbols and ZERO data symbols, because MSVC mangling
never names an instance data member, so nothing in the binary competes with
these. See `docs/EXCLUSIONS.md` for the bar by where the data lands.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import class_layouts  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_IDB = (REPO_ROOT / ".opensmacx" / "analysis" /
               "terranx_ORIG_200_v3_7.5.SP3.idb")
SRC = REPO_ROOT / "src"
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
THINKER = (REPO_ROOT / ".opensmacx" / "external-analysis" /
           "thinker-layout-hypotheses.csv")
OUTPUT = REPO_ROOT / "docs" / "recovery" / "idb-members.csv"

SUPPORTED_IDB_HASHES = {
    "6ffdcf2d6644f2c1b19c218d3b1b293b4e442d56b8cf1f537b0403608ff866fa",
}

# The class a member function belongs to. An ordinary method spells
# `?name@Class@@`; a SPECIAL name has no name segment at all - `??0Buffer@@` is
# `??`, the operator code, then the class. The scope must be NON-EMPTY, which
# is what refuses a free function: a lazy `[~\w@]*?@(\w+)@@` reads
# `?f@@YAXPAUGraphicWin@@@Z` as a class called `YAXPAUGraphicWin`. Here that
# only cost wasted netnode lookups for names no database holds, but it is the
# same defect and it is not worth leaving in three places.
SCOPE_RE = re.compile(
    r"^(?:\?\?(?:_[A-Z]|[0-9A-Z])|\?[\w_]+@)([\w_]+)(?:@[\w_]+)*@@")

# Names that carry no information: IDA's own auto-generated ones, in the same
# shape `src/` uses for a member it has an offset for and nothing else.
PLACEHOLDER = re.compile(r"^(field_[0-9A-Fa-f]+|unk\w*|gap\w*|pad\w*|_?\d+|)$")

STRUCT_SPACE = 0xFF000000
TYPE_SUPVAL = 0x3000
NAMED_TYPE = 0x3D  # BT_COMPLEX | BTMT_TYPEDEF: a length-prefixed type name.


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def unpack(buffer: bytes, index: int) -> tuple:
    """IDA's packed dword: (value, index just past it).

    One byte below 0x80, two below 0xC0, four below 0xE0, otherwise a 0xFF
    prefix and four raw big-endian bytes.
    """
    lead = buffer[index]
    if lead < 0x80:
        return lead, index + 1
    if lead < 0xC0:
        return ((lead & 0x3F) << 8) | buffer[index + 1], index + 2
    if lead < 0xE0:
        return (((lead & 0x1F) << 24) | (buffer[index + 1] << 16)
                | (buffer[index + 2] << 8) | buffer[index + 3]), index + 4
    return int.from_bytes(buffer[index + 1:index + 5], "big"), index + 5


def named_type(blob: bytes) -> str:
    """The type name, when the encoding is one this reader is sure of.

    IDA's type encoding is a whole language and only its named-type form is
    decoded: 0x3D, a length byte, then the name. Everything else returns "" so
    the SIZE column carries the layout on its own. A half-decoded type that
    reads plausibly is worse than no type at all.

    WHAT IS LEFT ON THE TABLE, measured over all 13,078 members:

        lead 0x3d  12,646   named type - but see below
        lead 0x07     193   BT_INT, no width beyond what `size` already says
        lead 0x1b     122   array of ...
        lead 0x0a      81   pointer to ...
        lead 0x32      27   / 0x03 8 / 0x09 1

    Almost all of the 0x3d are not names. The pascal string starts with `#`,
    which is IDA's ORDINAL marker, and the rest is a packed ordinal into the
    local type library - `= 04 23 81 7C 00` is ordinal 380. Only 6 members in
    the whole database spell their type out (HFONT, PVOID, HRGN, UINT_PTR,
    uint32_t), and those are the ones this returns.

    Resolving the ordinals would need the local type library, and it is not
    reachable here: there is no `$ tils`, `$ types` or `$ tinfo` netnode, and
    `$ structs` holds 233 Windows SDK types that do not include the game
    classes. python-idb's own `StructMember.get_type()` does not resolve them
    either - it raises UnicodeDecodeError on the ordinal byte.

    It costs less than it looks. The member NAME and SIZE together already
    identify the interesting cases: `BaseButton.graphicWin` is 0xa14, which is
    GraphicWin, and `AlphaMenu.buffer` is 0x588, which is Buffer. A by-value
    member of a known class is recognisable without decoding anything.
    """
    if len(blob) < 3 or blob[0] != NAMED_TYPE:
        return ""
    # The length byte COUNTS THE TERMINATING NUL: `HFONT` arrives as
    # `= \x06 H F O N T \x00`. Slicing 6 bytes and asking isidentifier() of
    # "HFONT\0" answers no, which silently dropped every type this decodes.
    length = blob[1]
    if length == 0 or len(blob) < 2 + length:
        return ""
    try:
        name = blob[2:2 + length].decode("ascii").rstrip("\x00")
    except UnicodeDecodeError:
        return ""
    return name if name.isidentifier() else ""


def candidates() -> list:
    """Every name worth asking the database about.

    `$ structs` is not an index of the game classes, so this is assembled from
    the three places a class name is already known.
    """
    names = set()
    for header in sorted(SRC.glob("*.h")):
        for name, _, _ in class_layouts.class_bodies(
                header.read_text(errors="ignore")):
            names.add(name)
    if FUNCTIONS.is_file():
        with FUNCTIONS.open(newline="", encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                match = SCOPE_RE.match(row.get("name") or "")
                if match:
                    names.add(match.group(1))
    if THINKER.is_file():
        with THINKER.open(newline="", encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                if row.get("struct"):
                    names.add(row["struct"])
    names.discard("")
    return sorted(names)


def members_of(db, netnode, name: str) -> list:
    """[(offset, member name, size, type)] for one class, or []."""
    import idb.netnode

    try:
        node = idb.netnode.Netnode(db, name)
        blob = b"".join(
            entry.value for entry in
            sorted(node.get_tag_entries(tag="M"),
                   key=lambda entry: entry.parsed_key.index))
    except Exception:
        return []
    if not blob:
        return []

    index = 0
    _header, index = unpack(blob, index)
    count, index = unpack(blob, index)

    found, offset = [], 0
    for _ in range(count):
        try:
            member_id, index = unpack(blob, index)
            _unused, index = unpack(blob, index)
            size, index = unpack(blob, index)
            _flags, index = unpack(blob, index)
            _tail, index = unpack(blob, index)
        except IndexError:
            # A truncated table is reported by what it yielded, not padded out
            # to the promised count.
            break
        member_name, member_type = "", ""
        try:
            member = idb.netnode.Netnode(db, STRUCT_SPACE | member_id)
            member_name = member.name().split(".", 1)[-1]
            member_type = named_type(member.supval(TYPE_SUPVAL))
        except Exception:
            pass
        found.append((offset, member_name, size, member_type))
        offset += size
    return found


def alignment(db, name: str) -> int:
    """`SF_ALIGN`, bits 7..10 of the header word, as a power of two."""
    import idb.netnode

    try:
        node = idb.netnode.Netnode(db, name)
        blob = b"".join(
            entry.value for entry in
            sorted(node.get_tag_entries(tag="M"),
                   key=lambda entry: entry.parsed_key.index))
        header, _ = unpack(blob, 0)
    except Exception:
        return 1
    return 1 << ((header >> 7) & 0xF)


def collect(idb_path: Path) -> list:
    import idb

    rows = []
    with idb.from_file(str(idb_path)) as db:
        for name in candidates():
            members = members_of(db, None, name)
            if not members:
                continue
            for offset, member_name, size, member_type in members:
                rows.append({
                    "class": name,
                    "offset": f"0x{offset:X}",
                    "name": member_name,
                    "size": size,
                    "type": member_type,
                })
    return rows


def render(rows: list) -> str:
    import io

    buffer = io.StringIO()
    writer = csv.DictWriter(
        buffer, fieldnames=["class", "offset", "name", "size", "type"],
        lineterminator="\n")
    writer.writeheader()
    for row in sorted(rows, key=lambda row: (row["class"],
                                             int(row["offset"], 16))):
        writer.writerow(row)
    return buffer.getvalue()


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--idb", type=Path, default=DEFAULT_IDB)
    parser.add_argument("--out", type=Path, default=OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed CSV is stale")
    args = parser.parse_args(argv)

    if not args.idb.is_file():
        print(f"SKIP: {args.idb} is absent. This reads the canonical IDA "
              f"database, which is never committed.")
        return 0
    try:
        import idb  # noqa: F401
    except ImportError:
        print("SKIP: python-idb is not installed (tools/requirements.txt).")
        return 0

    digest = sha256(args.idb)
    if digest not in SUPPORTED_IDB_HASHES:
        print(f"error: {args.idb} hashes {digest}, which is not the pinned "
              f"database. Member offsets from a different analysis are not "
              f"comparable with this tree's.", file=sys.stderr)
        return 2

    text = render(collect(args.idb))

    if args.check:
        current = args.out.read_text() if args.out.is_file() else ""
        if current != text:
            print(f"{args.out.name} is stale; regenerate it", file=sys.stderr)
            return 1
        classes = len({line.split(",", 1)[0] for line in
                       text.splitlines()[1:]})
        print(f"idb members: {len(text.splitlines()) - 1} members across "
              f"{classes} classes (up to date)")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    lines = text.splitlines()[1:]
    classes = {line.split(",", 1)[0] for line in lines}
    # Counting a `field_9E8_` as "named" is the conflation that made the IDB
    # look like a naming oracle: it defines a struct for 54 of the 73 blocked
    # classes, but most of what it defines is placeholders of its own.
    named = sum(1 for line in lines if not PLACEHOLDER.match(line.split(",")[2]))
    print(f"{len(lines)} members across {len(classes)} classes; "
          f"{named} carry a real name, {len(lines) - named} are the database's "
          f"own placeholders -> {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
