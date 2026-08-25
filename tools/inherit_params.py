#!/usr/bin/env python3
"""Give an override the parameter names its BASE already declares.

An override cannot invent its own meaning for an argument: if
`Win::on_mouse_move(int x, int y, unsigned int keys, int from_parent)` is
named, then every `on_mouse_move` with that arity is the same four things.
This tree has that shape everywhere - `on_scrolled`, `on_left_down`,
`attach`, `show` - and `name_params.py` cannot reach it, because it only
moves names WITHIN one header/source pair.

    uv run tools/inherit_params.py            # dry run
    uv run tools/inherit_params.py --apply

KEYED ON (method name, arity), like name_params, and for the same reason:
an overload set otherwise gives up all but one member. Two overloads of the
same arity still cannot be told apart, so the first named one wins and the
tool says so rather than guessing between them.

REFUSALS: a declaration whose names are already real is never touched; a
donor whose own names are scaffold is not a donor; and a method whose
arities disagree across the tree is skipped entirely, because that is a
signature question and not a naming one.
"""
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from name_params import DECL, SCAFFOLD, params, retype_decl  # noqa: E402


def donors(root: Path) -> dict[tuple[str, int], list[str]]:
    """(method, arity) -> real parameter names, from any header that has them."""
    best: dict[tuple[str, int], list[str]] = {}
    for header in sorted(root.glob("*.h")):
        for line in header.read_text(errors="replace").split("\n"):
            if line.lstrip().startswith("//"):
                continue
            m = DECL.match(line)
            if not m:
                continue
            names = params(m.group(2))
            if not names or any(SCAFFOLD.match(n) for n in names):
                continue
            best.setdefault((m.group(1), len(names)), names)
    return best


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", type=Path, default=Path("src"))
    ap.add_argument("--apply", action="store_true")
    args = ap.parse_args()

    table = donors(args.root)
    named = collections.Counter()
    for header in sorted(args.root.glob("*.h")):
        lines = header.read_text(errors="replace").split("\n")
        touched = False
        for i, line in enumerate(lines):
            if line.lstrip().startswith("//"):
                continue
            m = DECL.match(line)
            if not m:
                continue
            have = params(m.group(2))
            if not have or not any(SCAFFOLD.match(h) for h in have):
                continue
            want = table.get((m.group(1), len(have)))
            if not want:
                continue
            merged = [w if SCAFFOLD.match(h) and not SCAFFOLD.match(w) else h
                      for h, w in zip(have, want)]
            if merged == have:
                continue
            new = retype_decl(line, merged)
            if new is None or new == line:
                continue
            lines[i] = new
            touched = True
            named[header.name] += sum(1 for h, w in zip(have, merged) if h != w)
        if touched and args.apply:
            header.write_text("\n".join(lines))
    for name, n in named.most_common(12):
        print(f"  {n:4d}  {name}")
    print(f"{sum(named.values())} parameter(s) inherited from a base declaration"
          f"{'' if args.apply else '  (dry run; pass --apply)'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
