#!/usr/bin/env -S uv run python
r"""One image address, two names - and two different pieces of storage.

The tree names an image global two ways:

    static int *const WinHighlighted = (int *)0x009B7A6C;   a BINDING
    extern Win *WinZOrderWindow;  // 0x009B7A6C             a real GLOBAL

Those are not two spellings of one variable. The binding reads THAT ADDRESS;
the global is storage this binary allocates somewhere else. Code using one
cannot see writes through the other, and nothing in the byte ratchet
notices, because each body is internally consistent.

Fifteen of these were created in one afternoon (2026-08-25) by a naming pass
whose duplicate check read only the binding spelling and not the annotated
one. They were found by accident - `get_mouse_window` loads 0x9b7acc where
the image loads 0x9b7ac0 - which is not a way to find the other thirty-odd.

RATCHETED, not zeroed: the tree has pre-existing pairs that belong to other
classes' passes. The count may only fall.

    uv run tools/duplicate_globals.py            # the census
    uv run tools/duplicate_globals.py --check    # exit 1 if it grew
"""
from __future__ import annotations

import collections
import pathlib
import re
import sys

REPO = pathlib.Path(__file__).resolve().parent.parent

# Measured 2026-08-25 after consolidating Win's fifteen.
# 27 -> 26 (2026-08-29, init_thunks batch): the managed globals became real
# objects defined in init_thunks.cpp, which retired the address-pointer
# bindings and the one doubled name they carried.
# 26 -> 25 (2026-08-29, lifecycle-retirement batch): TeardownObject009BEAE8
# became g_BLANK_SPRITE in sprite.cpp, collapsing the last doubled name.
# 25 -> 23 (2026-08-29, binding-conversion batch): FontTable took over
# g_FONTS (font.cpp, 0x0093FC58) and TxtIndex took over TxtIndexGlobal
# (textindex.cpp, 0x009B7D08) - the object carries the binding's public
# name, so each pair is one name again.
CEILING = 16

BINDING = re.compile(r"^\s*(?:static\s+)?[\w:]+[\s\*&]+const\s+(\w+)\s*=\s*"
                     r"\(?\s*\([\w:\s\*]+\)\s*0x(00[0-9A-Fa-f]{6})")
ANNOTATED = re.compile(r"^\s*(?:extern\s+)?[\w:]+[\s\*&]+(\w+)\s*"
                       r"(?:\[[^\]]*\])?\s*;\s*//\s*0x(00[0-9A-Fa-f]{6})\b")


def census():
    names = collections.defaultdict(set)
    files = collections.defaultdict(set)
    for path in sorted(REPO.glob("src/*.h")) + sorted(REPO.glob("src/*.cpp")):
        for line in path.read_text(errors="replace").splitlines():
            for rx in (BINDING, ANNOTATED):
                found = rx.match(line)
                if found:
                    names[found.group(2).lower()].add(found.group(1))
                    files[found.group(2).lower()].add(path.name)
    return ({a: n for a, n in names.items() if len(n) > 1}, files)


def main() -> int:
    dups, files = census()
    check = "--check" in sys.argv
    if not check:
        for addr, group in sorted(dups.items()):
            print(f"  0x{addr.upper()}  {', '.join(sorted(group))}"
                  f"   in {', '.join(sorted(files[addr]))}")
    if len(dups) > CEILING:
        print(f"DUPLICATE GLOBALS GREW: {len(dups)} addresses carry more than "
              f"one name, above the ceiling of {CEILING} - one of them is "
              f"storage this binary allocates and the other is the image's "
              f"address, and they are not the same memory")
        return 1
    if len(dups) < CEILING:
        print(f"duplicate globals down: {len(dups)}, below the ceiling of "
              f"{CEILING} - set the ceiling to {len(dups)} in this commit")
        return 1 if check else 0
    print(f"duplicate globals: {len(dups)} address(es) with two names, "
          f"ceiling exact")
    return 0


if __name__ == "__main__":
    sys.exit(main())
