#!/usr/bin/env python3
"""Derive an offset -> member map from a class declaration.

`name_offsets.py` needs a map from `self + 0xNN` to a member name, and until
now that map had to be written by hand - which is the shape this project has
recorded as its highest-yield tooling defect. The header already knows: it
declares the members in order, and it annotates enough of them with their
offset to anchor the walk.

    uv run tools/header_offsets.py src/win.h Win
    uv run tools/header_offsets.py src/win.h Win --json > map.json

A member whose size this cannot know stops the walk rather than guessing
past it - a wrong offset here renames the wrong field, which is worse than
naming nothing.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

# Sizes this tree's headers actually use. Anything else halts the walk.
SIZES = {
    "int": 4, "unsigned int": 4, "uint32_t": 4, "int32_t": 4, "long": 4,
    "unsigned long": 4, "float": 4, "bool": 1, "char": 1, "uint8_t": 1,
    "int8_t": 1, "short": 2, "uint16_t": 2, "int16_t": 2, "double": 8,
    "RECT": 16, "POINT": 8, "WinNodeList": 24,
    # Win32 handle and pointer typedefs. Every one is a pointer on x86, and
    # they are what stopped the walk on Buffer - whose members ARE
    # self-consistent by hand, so a missing size here reads as a layout
    # defect that is not there.
    "LPVOID": 4, "LPSTR": 4, "LPCSTR": 4, "HDC": 4, "HWND": 4, "HBITMAP": 4,
    "HPALETTE": 4, "HRGN": 4, "HCURSOR": 4, "HICON": 4, "HGDIOBJ": 4,
    "HANDLE": 4, "HFONT": 4, "HMENU": 4, "HINSTANCE": 4, "LPARAM": 4,
    "WPARAM": 4, "DWORD": 4, "UINT": 4, "BOOL": 4, "LONG": 4, "COLORREF": 4,
}
# A struct member is expanded so `self + 0x140` names a FIELD, not the struct.
FIELDS = {
    "RECT": [("left", 0), ("top", 4), ("right", 8), ("bottom", 12)],
    "POINT": [("x", 0), ("y", 4)],
    "WinNodeList": [("head_", 0), ("current_", 4), ("tail_", 8),
                    ("count_", 12), ("external_", 16)],
}
MEMBER = re.compile(
    r"^\s{2,}(?!(?:public|private|protected|virtual|static|friend|typedef|using)\b)"
    r"([A-Za-z_][\w:]*(?:\s+[A-Za-z_][\w:]*)?)\s+(\*?)(\w+)\s*(\[(\d+)\])?\s*;"
    r"(?:\s*//\s*(?:Win\+)?(0x[0-9A-Fa-f]+))?")


def class_body(text: str, name: str) -> list[str]:
    m = re.search(r"^class\s+" + name + r"\b[^{]*\{", text, re.M)
    if not m:
        return []
    depth, out, i = 0, [], m.end() - 1
    for line in text[i:].splitlines(True):
        depth += line.count("{") - line.count("}")
        out.append(line)
        if depth <= 0 and len(out) > 1:
            break
    return out


def sizes(root: Path) -> dict[str, int]:
    """Every `static_assert(sizeof(X) == N)` in the tree, as a size table.

    The headers already pin the sizes this walk needs - buffer.h asserts
    sizeof(Buffer) == 0x588, sprite.h sizeof(Sprite) == 0x2C - and reading
    them is what lets the walk pass THROUGH a class-typed member instead of
    stopping there and reporting a layout it never measured.
    """
    out: dict[str, int] = {}
    for header in root.glob("*.h"):
        for name, size in re.findall(
                r"static_assert\(sizeof\((\w+)\)\s*==\s*(0x[0-9A-Fa-f]+|\d+)",
                header.read_text(errors="replace")):
            out[name] = int(size, 0)
    return out


def derive(header: Path, cls: str, extra: dict[str, int] | None = None):
    body = class_body(header.read_text(errors="replace"), cls)
    offset, table, anchored = None, {}, False
    disagreements: list = []
    seen = 0            # members parsed, anchored or not
    for line in body:
        m = MEMBER.match(line.rstrip("\n"))
        if not m:
            continue
        ty, star, name, _arr, count, note = m.groups()
        ty = " ".join(ty.split())
        seen += 1
        if note:
            stated = int(note, 16)
            # AN ANNOTATION THAT DISAGREES WITH THE WALK IS A LAYOUT FINDING,
            # not a place to silently re-anchor. Re-anchoring would absorb
            # exactly the defect this map exists to avoid: a member declared
            # at the wrong size shifts everything after it, and the next
            # annotation would quietly paper over the shift while every
            # offset between the two stayed wrong.
            if anchored and offset is not None and stated != offset:
                print(f"  LAYOUT DISAGREEMENT at `{name}`: the header says "
                      f"{stated:#x}, walking the members from the last "
                      f"annotation gives {offset:#x}", file=sys.stderr)
                disagreements.append((name, stated, offset))
            offset, anchored = stated, True
        if not anchored or offset is None:
            continue
        # THE TREE'S OWN static_assert(sizeof(X)) IS A SIZE TABLE, and
        # `sizes()` has been reading it all along while this line ignored
        # it. A `Buffer buffer_` member stopped the walk even though
        # buffer.h asserts sizeof(Buffer) == 0x588 two files away.
        size = 4 if star else (SIZES.get(ty) if extra is None
                               else extra.get(ty, SIZES.get(ty)))
        if size is None:
            print(f"  ...stopping at `{ty} {name}`: unknown size", file=sys.stderr)
            break
        n = int(count) if count else 1
        if not star and ty in FIELDS and n == 1:
            for fname, delta in FIELDS[ty]:
                table[f"{offset + delta:#x}"] = f"{name}.{fname}"
        else:
            table[f"{offset:#x}"] = name
        offset += size * n
    return table, disagreements, seen, bool(body)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("header", type=Path, nargs="?")
    ap.add_argument("cls", nargs="?")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--survey", action="store_true",
                    help="every class under --root: which can derive a map, "
                         "and which declare members but carry no anchor")
    ap.add_argument("--root", type=Path, default=Path("src"))
    args = ap.parse_args()

    if args.survey:
        # WHY THIS EXISTS. `name_offsets.py` clears the "raw self-access"
        # shape, but only for a class whose header can anchor a walk. Every
        # class WITHOUT an anchor is a blocker, and it blocks its DERIVED
        # classes too: EditGroup's cleanup needs Dialog's layout, and Dialog
        # declares 56 members with no `// 0xNN` on any of them (measured
        # 2026-08-26). Annotating one member whose offset is known unblocks
        # the rest of that class AND everything that inherits from it.
        import re as _re
        known = sizes(args.root)
        can, cannot = 0, []
        for header in sorted(args.root.glob("*.h")):
            text = header.read_text(errors="replace")
            for m in _re.finditer(r"^class (\w+)\s*(?::[^{]*)?\{", text, _re.M):
                cls = m.group(1)
                try:
                    table, _bad, seen, found = derive(header, cls, known)
                except Exception:
                    continue
                if not found or not seen:
                    continue
                if table:
                    can += 1
                else:
                    cannot.append((seen, cls, header.name))
        cannot.sort(reverse=True)
        print(f"{can} class(es) CAN derive an offset map")
        print(f"{len(cannot)} class(es) declare members but carry NO anchor\n")
        print("biggest blockers - members declared, no `// 0xNN` anywhere:")
        for seen, cls, f in cannot[:15]:
            print(f"  {seen:4d}  {cls:<22} {f}")
        return 0 if not cannot else 1

    table, bad, seen, found = derive(args.header, args.cls, sizes(args.root))
    if args.json:
        print(json.dumps(table, indent=2))
    else:
        for off, name in sorted(table.items(), key=lambda kv: int(kv[0], 16)):
            print(f"  {off:>8}  {name}")
        # A ZERO HERE IS TWO DIFFERENT ANSWERS - say which one. "no offsets"
        # reads as "this class is clean" when it usually means the header
        # carries no `// 0xNN` to anchor the walk, and the map is simply
        # unavailable rather than empty.
        if not table:
            if not found:
                print(f"no `class {args.cls}` in {args.header.name}", file=sys.stderr)
            elif not seen:
                print(f"class {args.cls} declares no data members this can parse",
                      file=sys.stderr)
            else:
                print(f"class {args.cls} declares {seen} member(s) but NOT ONE "
                      f"carries a `// 0xNN` offset, so there is nothing to anchor "
                      f"the walk to. Annotate one member whose offset is known "
                      f"and the rest follow.", file=sys.stderr)
            return 2
        print(f"{len(table)} offset(s) derived from {args.header.name} "
              f"({seen} member(s) walked)")
    if bad:
        print(f"{len(bad)} layout disagreement(s) - the map is NOT usable "
              f"until they are resolved", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
