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
    "size_t": 4, "ptrdiff_t": 4, "intptr_t": 4, "uintptr_t": 4,
    # Win32 value structs, by their documented layouts.
    "PALETTEENTRY": 4,      # 4 BYTEs
    "RGBQUAD": 4,           # 4 BYTEs
    "BITMAPINFOHEADER": 40,
    "LOGPEN": 16,           # UINT + POINT + COLORREF
    "SIZE": 8,
    # CRITICAL_SECTION is the one Win32 struct here that is NOT a handful of
    # DWORDs by inspection: on x86 it is DebugInfo, LockCount, RecursionCount,
    # OwningThread, LockSemaphore, SpinCount - six 4-byte fields, 0x18. It
    # stopped Ambience's walk, and a stopped walk reads as "no anchor", which
    # is why a missing size here costs a whole class.
    "CRITICAL_SECTION": 24,
    "FILETIME": 8, "LARGE_INTEGER": 8, "ULARGE_INTEGER": 8,
    # The remaining pointer-width and fixed-width typedefs. Every _PTR type is
    # 4 on x86; BYTE/WORD are the sizes their names promise.
    "UINT_PTR": 4, "INT_PTR": 4, "LONG_PTR": 4, "ULONG_PTR": 4,
    "DWORD_PTR": 4, "LRESULT": 4, "HRESULT": 4, "HBRUSH": 4, "HPEN": 4,
    "HMODULE": 4, "HGLOBAL": 4, "HLOCAL": 4, "HKEY": 4, "HRSRC": 4,
    "WNDPROC": 4, "FARPROC": 4, "LPRECT": 4, "LPPOINT": 4, "LPWORD": 4,
    "LPDWORD": 4, "LPBYTE": 4, "ULONG": 4, "BYTE": 1, "CHAR": 1, "TCHAR": 1,
    "WORD": 2, "USHORT": 2, "SHORT": 2, "ATOM": 2, "INT": 4, "UCHAR": 1,
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
    r"([A-Za-z_][\w:]*(?:\s+[A-Za-z_][\w:]*)?)\s+(\*?)(\w+)\s*(\[([^\]]+)\])?\s*;"
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


def extent(text: str) -> int | None:
    r"""The element count of an array declarator, or None.

    NOT JUST A DECIMAL LITERAL. This tree sizes its opaque spans by
    arithmetic - `uint8_t dialogs_[0x2274 - 0x21D0];` in basepop.h,
    `uint8_t flatButtonsA_[7 * 0xB4C];` in reportif.h - and reading only
    `\d+` made those arrays ONE element long, which threw every later
    member off by the whole span. BasePop was reported as a layout
    DISAGREEMENT on the strength of it; the layout was right and the reader
    was wrong.

    Constant folding only: hex/decimal literals with + - * and parentheses.
    Anything with an identifier in it returns None, which stops the walk
    honestly instead of guessing.
    """
    body = text.strip()
    if not body:
        return None
    if not re.fullmatch(r"[0-9A-Fa-fx\s+\-*()]+", body):
        return None
    try:
        value = eval(body, {"__builtins__": {}}, {})   # noqa: S307 - digits only
    except Exception:
        return None
    return value if isinstance(value, int) and value > 0 else None


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


def packed(text: str, cls: str) -> bool:
    """Is `cls` declared while a `#pragma pack` is in effect?

    THE ALIGNMENT MODEL BELOW IS ONLY TRUE AT THE DEFAULT PACKING. Under
    `#pragma pack(1)` the compiler inserts no padding at all, so every offset
    this walk derives for such a class would be wrong in the one direction
    that looks plausible. caviar.h packs a three-member group precisely so
    `camera_` can sit at the unaligned 0xA5 the image uses; a walk that
    "helpfully" rounded that to 0xA8 would hand name_offsets the wrong member.
    Refusing is the honest answer - the map is not derivable here.
    """
    depth = 0
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("#pragma pack"):
            arg = s[s.index("(") + 1:s.rindex(")")].strip() if "(" in s else ""
            if arg in ("", "pop"):
                depth = max(0, depth - 1)
            elif arg.startswith("push") or arg.isdigit():
                depth += 1
        # WORD BOUNDARY, not a prefix. `"struct CaviarCamera".startswith(
        # "struct Caviar")` is true, and without the boundary this refused
        # `Caviar` - which is declared at line 55, well outside the pack
        # region - because the packed struct three lines up shares its prefix.
        # Same defect as matching a member declaration by suffix; a name test
        # that can match a LONGER name is not a name test.
        if depth and re.match(rf"(class|struct)\s+{re.escape(cls)}\b", s):
            return True
    return False


def derive(header: Path, cls: str, extra: dict[str, int] | None = None):
    text = header.read_text(errors="replace")
    if packed(text, cls):
        print(f"  ...refusing `{cls}`: declared under #pragma pack, where this "
              f"walk's alignment model does not hold", file=sys.stderr)
        return {}, [], 0, False, None
    body = class_body(text, cls)
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
        # ALIGNMENT IS PART OF THE COMPARISON, not just part of the placement.
        # This padding used to happen further down, AFTER the disagreement
        # check below, so a correctly-annotated member that follows a narrow
        # one was compared against an unpadded offset and reported as a
        # LAYOUT FINDING. Flic is the case: `uint8_t field_0_` at 0x0 leaves
        # the walk at 0x1, `Buffer buffer_` is annotated 0x4 and IS at 0x4,
        # and the check called the header wrong while the very next lines
        # placed it correctly. Pad first, then compare.
        align = min(4 if star else (SIZES.get(ty) if extra is None
                                    else extra.get(ty, SIZES.get(ty))) or 1, 4)
        if anchored and offset is not None and align > 1 and offset % align:
            offset += align - (offset % align)
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
        n = extent(count) if count else 1
        if n is None:
            print(f"  ...stopping at `{ty} {name}[{count}]`: extent is not a "
                  f"constant", file=sys.stderr)
            break
        if not star and ty in FIELDS and n == 1:
            for fname, delta in FIELDS[ty]:
                table[f"{offset + delta:#x}"] = f"{name}.{fname}"
        else:
            table[f"{offset:#x}"] = name
        offset += size * n
    # THE END OF THE WALK IS EVIDENCE, not a byproduct. A class whose last
    # member ends exactly on a size derived elsewhere corroborates that size;
    # one that falls short has an undeclared tail. Callers that only want the
    # map ignore this, but `sizeof_from_embeds.py` cannot do its job without
    # it - and reporting the last member's OFFSET instead of its END is a
    # whole member's worth of false shortfall.
    return table, disagreements, seen, bool(body), offset


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
    ap.add_argument("--check", action="store_true",
                    help="every derivable class agrees with its own names and "
                         "annotations; the gate's invariant, and it must be 0")
    args = ap.parse_args()

    if args.check:
        # WHY THIS IS A GATE CHECK AND NOT A THING SOMEONE RE-RUNS.
        # `name_offsets.py` rewrites `self + 0xNN` into a member name using
        # the map this file derives, so a walk that quietly shifts by four
        # bytes renames every access after it to the WRONG member - and the
        # bytes cannot see it, because the compiled offset is identical
        # either way. Two edits on 2026-08-26 changed this walk (alignment
        # moved above the comparison, packed classes became refusals), and
        # nothing in the gate would have noticed if either had been wrong.
        #
        # The invariant is self-verifying and needs no list to maintain: a
        # member NAMED `field_NN_` states its own offset, so the walk landing
        # on it is a proof, and every annotation is a second one.
        import re as _re
        known = sizes(args.root)
        checked = wrong = 0
        clashes: list[str] = []
        for header in sorted(args.root.glob("*.h")):
            text = header.read_text(errors="replace")
            for m in _re.finditer(r"^(?:class|struct) (\w+)\s*(?::[^{]*)?\{",
                                  text, _re.M):
                cls = m.group(1)
                try:
                    table, bad, _seen, found, _end = derive(header, cls, known)
                except Exception:
                    continue
                if not found:
                    continue
                for name, stated, walked in bad:
                    clashes.append(f"  {header.name} {cls}::{name}: header says "
                                   f"{stated:#x}, walk says {walked:#x}")
                    wrong += 1
                for off, name in table.items():
                    nm = _re.fullmatch(r"field_([0-9A-Fa-f]+)_", name)
                    if not nm:
                        continue
                    checked += 1
                    if int(nm.group(1), 16) != int(off, 16):
                        clashes.append(f"  {header.name} {cls}::{name} walked to "
                                       f"{off} - its own name says "
                                       f"{int(nm.group(1), 16):#x}")
                        wrong += 1
        for line in clashes[:40]:
            print(line, file=sys.stderr)
        if len(clashes) > 40:
            print(f"  ...and {len(clashes) - 40} more", file=sys.stderr)
        # A DISTINCT VERDICT ON FAILURE, because "0 disagree" and
        # "3 disagree" differ by one character and the harness that proves
        # this check can fail matches on TEXT. A green line that a red line
        # is a substring of is how a control passes without proving anything.
        if wrong:
            print(f"FAILED: an offset walk disagrees with the tree's own "
                  f"names - {wrong} of {checked} self-naming member(s)")
            return 1
        print(f"{checked} self-naming member(s) checked against the walk, "
              f"0 disagree")
        return 0

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
                    table, _bad, seen, found, _end = derive(header, cls, known)
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

    table, bad, seen, found, _end = derive(args.header, args.cls, sizes(args.root))
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
