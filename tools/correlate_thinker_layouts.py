#!/usr/bin/env python3
"""Extract struct-layout and global-address hypotheses from Thinker headers.

The Thinker mod's engine headers describe packed struct layouts and absolute
global addresses reverse engineered from the same binary. Under the repository
policy they are hypothesis sources only: this tool reduces fetched local
copies to two ignored CSVs of offsets and addresses so the layout campaign can
consume leads without any header text entering the repository. Every lead
still requires independent verification against the hash-bound canonical
executable before source, tests, or metadata are committed.
"""

import argparse
import csv
from pathlib import Path
import re
import sys

from local_artifact import require_local_artifact_path


REPO_ROOT = Path(__file__).resolve().parent.parent
# `static_assert(sizeof(X) == N)` - the tree's own statement that a size was
# checked against the image. Only these are used to place a derived struct's
# members, deliberately: an unproved base size would put every member of the
# derived struct at a wrong offset, which is the failure this whole file is
# careful about.
SIZE_ASSERT_RE = re.compile(
    r"static_assert\(\s*sizeof\(\s*(\w+)\s*\)\s*==\s*(0[xX][0-9a-fA-F]+|\d+)")
DEFAULT_SOURCE_DIR = (
    REPO_ROOT / ".opensmacx" / "external-analysis" / "sources" / "thinker")
DEFAULT_OUTPUT_DIR = REPO_ROOT / ".opensmacx" / "external-analysis"

# `struct X {` and `struct X : Base {`.
#
# THE BASE LIST WAS NOT OPTIONAL HERE, AND EVERY DERIVED STRUCT WAS DROPPED
# WHOLE. Thinker's headers declare 121 structs and this read 93; the 16 it
# missed - GraphicWin : Win, BasePop : GraphicWin, BaseWindow : GraphicWin,
# BaseButton : GraphicWin and the rest - are the inheritance spine of exactly
# the classes carrying the most unnamed members, and they took 1,070 member
# lines with them.
#
# `struct std::hash<Point>` does not match and must not: `\w+` cannot span the
# `::`, so the qualified name falls out on its own.
STRUCT_RE = re.compile(
    r"^\s*struct\s+(?P<name>\w+)\s*"
    r"(?::\s*(?:public\s+|protected\s+|private\s+)?(?P<base>\w+)\s*)?\{")
# Thinker names an unknown member after its own ABSOLUTE offset, the same way
# `src/` does. That makes its placeholders a free control on the arithmetic
# below: `struct BasePop : GraphicWin` opens with `int field_A14;`, and 0xA14
# is sizeof(GraphicWin). If a computed offset and the name disagree, the
# computation is wrong and the rest of the struct is withheld.
SELF_NAMED_RE = re.compile(r"^field_(?P<offset>[0-9A-Fa-f]+)$")
FIELD_RE = re.compile(
    r"^\s*(?P<type>[A-Za-z_]\w*(?:\s*\*+)?)\s+"
    r"(?P<name>\w+)\s*(?:\[(?P<count>\w+)\])?\s*;")
ADDRESS_RE = re.compile(r"0x0{0,2}[0-9A-Fa-f]{6,8}\b")
BINDING_RE = re.compile(
    r"^\s*\w[\w\s\*&:<>,]*?[\s\*&](?P<name>\w+)\s*="
    r"[^;]*?(?P<address>0x[0-9A-Fa-f]{6,8})\b")

PRIMITIVE_SIZES = {
    "int8_t": 1, "uint8_t": 1, "char": 1, "bool": 1,
    "int16_t": 2, "uint16_t": 2, "short": 2,
    "int32_t": 4, "uint32_t": 4, "int": 4, "float": 4, "long": 4,
    "int64_t": 8, "uint64_t": 8, "double": 8,
    # The Win32 spellings Thinker uses. Fixed by the platform ABI on a 32-bit
    # target rather than inferred, and worth having because ONE unreadable
    # member stops a whole struct: `BYTE`, `WORD`, `RECT` and `LPSTR` between
    # them blocked BasePop, Console, Buffer and Win, which hold some of the
    # largest runs of unnamed members in the tree.
    "BYTE": 1, "CHAR": 1, "UCHAR": 1,
    "WORD": 2, "SHORT": 2, "USHORT": 2, "WCHAR": 2,
    "DWORD": 4, "LONG": 4, "ULONG": 4, "UINT": 4, "INT": 4, "BOOL": 4,
    "COLORREF": 4, "FLOAT": 4, "LPARAM": 4, "WPARAM": 4, "LRESULT": 4,
    "LPSTR": 4, "LPCSTR": 4, "LPVOID": 4, "HANDLE": 4,
    "HDC": 4, "HBITMAP": 4, "HWND": 4, "HFONT": 4, "HINSTANCE": 4,
    "HRGN": 4, "HPEN": 4, "HBRUSH": 4, "HPALETTE": 4, "HMENU": 4,
    "HICON": 4, "HCURSOR": 4,
    "POINT": 8, "SIZE": 8, "RECT": 16,
}


def field_size(type_name, count, known_sizes=None):
    """Bytes a declaration occupies, or None when this cannot say.

    A member whose type is a class the tree has PROVED the size of is exactly
    as sizeable as a primitive, and refusing it stopped whole structs one
    member in: `GraphicWin` opens `Buffer oCanvas;` and `BasePop` has
    `Heap heap;`, so the accumulator gave up after one and five fields
    respectively.

    Thinker's own placeholder names confirm both sizes independently. Its next
    field after `Buffer oCanvas` is `field_9CC`, and 0x444 + 0x588 is 0x9CC;
    after `Heap heap` comes `field_A3C`, and 0xA28 + 0x14 is 0xA3C. Those are
    the pinned sizes of `Buffer` and `Heap` arrived at from the other side.
    """
    known_sizes = known_sizes or {}
    if "*" in type_name:
        base = 4  # 32-bit target pointers
    elif type_name in PRIMITIVE_SIZES:
        base = PRIMITIVE_SIZES[type_name]
    elif type_name in known_sizes:
        base = known_sizes[type_name]
    else:
        return None
    if count is None:
        return base
    try:
        return base * int(count, 0)
    except ValueError:
        return None


def drop_name_collisions(rows, pinned):
    """Remove structs whose computed size contradicts a pinned one.

    Thinker names its own types, and where a name means something different
    from `src/`'s type of that name, every offset it carries is about a
    different object. `docs/EXCLUSIONS.md` records the pattern - its `Buffer`
    is 0x1c against a real 0x588 - and the computed size is the cheapest
    detector, because it needs no judgement.

    Live example, and the reason this is a filter rather than a warning:
    Thinker's `Spot` is `RECT rect; int type; int position;` at 0x18, which is
    what `src/spot.h` calls `Spot::SpotInternal`; what `src/` calls `Spot` is
    Thinker's `SpotList`, and both agree that one is 0xC. Same name, different
    type, and adopting the offsets would have described the wrong object.
    """
    computed = {row["struct"]: row["offset"] for row in rows
                if row["field"] == "(sizeof)" and row["offset"]}
    collided = {name for name, size in computed.items()
                if name in pinned and int(size, 16) != pinned[name]}
    kept = [row for row in rows if row["struct"] not in collided]
    return kept, sorted(collided)


def pinned_sizes() -> dict:
    """{class: size} for every class `src/` asserts the size of."""
    found = {}
    for header in sorted((REPO_ROOT / "src").glob("*.h")):
        for name, raw in SIZE_ASSERT_RE.findall(
                header.read_text(errors="ignore")):
            found[name] = int(raw, 0)
    return found


def parse_struct_layouts(text, source_name, base_sizes=None):
    """Walk packed struct bodies accumulating byte offsets for primitive
    fields; offsets become unknown after the first unsized field.

    A DERIVED STRUCT'S OWN MEMBERS DO NOT START AT ZERO. They start after the
    base subobject, so the accumulator opens at `sizeof(base)` and the base has
    to be a size this tree has PROVED - `base_sizes` is the pinned
    `static_assert(sizeof(X) == N)` set, not another hypothesis. Accumulating
    from zero instead would report every member of `BasePop` 0xa14 bytes low,
    which is the same failure as reading past an unparsable declaration and
    forty times the size of it.

    Where the base's size is not pinned, the members are still emitted - their
    NAMES are worth having - and carry no offset.
    """
    base_sizes = base_sizes or {}
    rows = []
    struct_name = None
    offset = 0
    known = True
    for line_number, line in enumerate(text.splitlines(), 1):
        if struct_name is None:
            match = STRUCT_RE.match(line)
            if match:
                struct_name = match.group("name")
                base = match.group("base")
                if base is None:
                    offset, known = 0, True
                else:
                    offset = base_sizes.get(base, 0)
                    known = base in base_sizes
            continue
        if re.match(r"^\s*\};", line):
            rows.append({
                "source": source_name, "line": line_number,
                "struct": struct_name, "field": "(sizeof)",
                "offset": f"0x{offset:X}" if known else "",
                "size": "",
            })
            struct_name = None
            continue
        match = FIELD_RE.match(line)
        if not match:
            # A DECLARATION THIS REGEX CANNOT READ IS NOT A LINE TO SKIP. It
            # occupies storage, so every offset after it moves, and skipping it
            # silently reports the REST of the struct too low by its width.
            #
            # Measured on Thinker's own Buffer, which declares
            #
            #     int (__cdecl *pfcnScrollText)(char *, int, int, int);
            #
            # at 0x8. FIELD_RE does not match a function pointer, so `iFlags`
            # came out at 0x18 against a true 0x1C. The struct's own names give
            # it away: the fields Thinker calls dwordC, dword10 and dword14
            # were reported at 0x8, 0xC and 0x10.
            #
            # Anything ending in `;` inside a struct body is a member, so from
            # here on the honest answer is that the offset is unknown - which
            # this function already knows how to say.
            if re.sub(r"//.*", "", line).strip().endswith(";"):
                known = False
            continue
        size = field_size(match.group("type").strip(), match.group("count"),
                          base_sizes)
        # THE FREE CONTROL. Thinker names an unknown member after its own
        # absolute offset, so where it does, it is stating the answer this
        # function is computing. A disagreement means the accumulator is wrong
        # - a base size, a member width, a declaration read past - and the
        # honest response is to stop answering for this struct rather than to
        # publish a number its own source contradicts.
        self_named = SELF_NAMED_RE.match(match.group("name"))
        if known and self_named and int(self_named.group("offset"), 16) != offset:
            known = False
        rows.append({
            "source": source_name, "line": line_number,
            "struct": struct_name, "field": match.group("name"),
            "offset": f"0x{offset:X}" if known else "",
            "size": size if size is not None else "",
        })
        if size is None:
            known = False
        elif known:
            offset += size
    return rows


def parse_address_bindings(text, source_name):
    rows = []
    for line_number, line in enumerate(text.splitlines(), 1):
        match = BINDING_RE.match(line)
        if match:
            rows.append({
                "source": source_name, "line": line_number,
                "name": match.group("name"),
                "address": f"0x{int(match.group('address'), 16):08X}",
            })
    return rows


def write_rows(path, fieldnames, rows):
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser(
        description="Reduce Thinker headers to local layout/address hypotheses")
    parser.add_argument("--source-dir", type=Path, default=DEFAULT_SOURCE_DIR)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    args = parser.parse_args()

    output_dir = require_local_artifact_path(
        args.output_dir, "hypothesis output directory")
    source_dir = args.source_dir.resolve()
    if not source_dir.is_dir():
        raise SystemExit(
            f"fetched Thinker sources not found: {source_dir}; run "
            "tools/fetch_external_analysis.py first")

    layout_rows = []
    binding_rows = []
    base_sizes = pinned_sizes()
    for source in sorted(source_dir.rglob("*")):
        if source.suffix not in (".h", ".cpp"):
            continue
        text = source.read_text(encoding="utf-8", errors="replace")
        name = source.relative_to(source_dir).as_posix()
        if source.suffix == ".h":
            layout_rows.extend(
                parse_struct_layouts(text, name, base_sizes))
        binding_rows.extend(parse_address_bindings(text, name))

    layout_rows, collided = drop_name_collisions(layout_rows, base_sizes)

    output_dir.mkdir(parents=True, exist_ok=True)
    write_rows(output_dir / "thinker-layout-hypotheses.csv",
               ["source", "line", "struct", "field", "offset", "size"],
               layout_rows)
    write_rows(output_dir / "thinker-address-hypotheses.csv",
               ["source", "line", "name", "address"],
               binding_rows)
    structs = len({row["struct"] for row in layout_rows})
    print(
        f"correlate-thinker-layouts: {structs} structs, "
        f"{len(layout_rows)} field rows, {len(binding_rows)} address rows")
    if collided:
        print(f"    dropped {len(collided)} struct(s) whose computed size "
              f"contradicts a pinned one, so the name means something else "
              f"here: {', '.join(collided)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
