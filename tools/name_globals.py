#!/usr/bin/env -S uv run python
r"""Turn `g_009b7a50` into a name, mechanically and without moving a byte.

WHY THIS IS A TOOL AND NOT A SED. `compiler_work`'s "anonymous fixed-address
global" ceiling counts EVERY MENTION of a `g_<address>` identifier, because
the name is the defect: it tells the reader nothing at any of its use sites.
Homing imports these from the artifact archives by the hundred - win.cpp
alone arrived carrying 674 - and the ceiling is what forces them to be
imported NAMED.

THE ONE RULE THAT MAKES THIS SAFE. A RENAME is byte-neutral: an identifier
is not observable in the object file, so `g_009b7a50` -> `WinBubbleActive`
cannot cost a claim and needs no re-measurement. REPOINTING a use at a
differently-TYPED binding is not neutral and must be measured, because the
type decides whether the binding folds to its immediate. Both were done by
hand on 2026-08-25 and only the second one broke anything:

  - 0x0066927C as `GetDCFn *const` compiled `mov eax, dword ptr [0]` +
    `call [eax]` where the image has one `call dword ptr [0x66927c]`.
  - 0x009B6630 as `void **const` did the same in Win::redraw while folding
    in two other bodies, which is what made it look like a per-body
    register-allocation quirk rather than the type.

Both fold as `int *const` with the cast moved to the point of use. So this
tool renames and REFUSES to retype; when a rename would collide with an
existing binding for the same address, it says so and leaves the decision -
delete which one, and re-measure - to a person.

    uv run tools/name_globals.py src/win.cpp             # the evidence sheet
    uv run tools/name_globals.py src/win.cpp --map m.json # apply {addr: name}
"""
from __future__ import annotations

import argparse
import collections
import json
import pathlib
import re
import sys

REPO = pathlib.Path(__file__).resolve().parent.parent
ANON = re.compile(r"\bg_(00[0-9a-f]{4,6})\b")
BINDING = re.compile(r"^\s*(?:static\s+)?[\w:\s]*\*+\s*const\s+(\w+)\s*=\s*"
                     r"\(?\s*\([\w:\s\*]+\)\s*0x(00[0-9A-Fa-f]{6})")


def tree_files():
    for p in sorted(REPO.glob("src/*.cpp")) + sorted(REPO.glob("src/*.h")):
        yield p


def named_addresses() -> dict[str, set[str]]:
    """address -> the non-anonymous names already bound to it."""
    out = collections.defaultdict(set)
    for p in tree_files():
        for line in p.read_text(errors="replace").splitlines():
            m = BINDING.match(line)
            if m and not m.group(1).startswith("g_00"):
                out[m.group(2).lower()].add(m.group(1))
    return out


def sheet(target: pathlib.Path) -> int:
    text = target.read_text(errors="replace")
    counts = collections.Counter(ANON.findall(text))
    lines = text.splitlines()
    known = named_addresses()
    print(f"{sum(counts.values())} mention(s), {len(counts)} address(es) "
          f"in {target.name}")
    for addr, n in counts.most_common():
        clash = known.get(addr)
        tag = f"  ALREADY NAMED {sorted(clash)}" if clash else ""
        print(f"\n  0x{addr.upper()}  x{n}{tag}")
        shown = 0
        for line in lines:
            if f"g_{addr}" in line and not line.lstrip().startswith("//"):
                print(f"      {line.strip()[:104]}")
                shown += 1
                if shown == 3:
                    break
    return 0


def apply(target: pathlib.Path, mapping: dict) -> int:
    known = named_addresses()
    bad = [a for a in mapping if a.lower() in known]
    if bad:
        for a in bad:
            print(f"REFUSED 0x{a.upper()}: already bound to "
                  f"{sorted(known[a.lower()])} - delete one binding by hand "
                  f"and re-measure; a retype is not byte-neutral")
        return 1
    changed = 0
    for p in tree_files():
        text = original = p.read_text(errors="replace")
        for addr, name in mapping.items():
            text = re.sub(rf"\bg_{addr.lower()}\b", name, text)
        if text != original:
            p.write_text(text)
            changed += 1
    print(f"renamed {len(mapping)} address(es) across {changed} file(s); "
          f"identifiers only, so no body needs re-measuring")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("file", type=pathlib.Path)
    ap.add_argument("--map", type=pathlib.Path,
                    help='JSON {"009b7a50": "WinBubbleActive"}')
    args = ap.parse_args()
    if args.map:
        return apply(args.file, json.loads(args.map.read_text()))
    return sheet(args.file)


if __name__ == "__main__":
    sys.exit(main())
