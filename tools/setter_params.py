#!/usr/bin/env python3
"""Name a ONE-PARAMETER setter's argument from the method's own name.

`set_volume(int a1)` takes a volume. `set_pitch(int a1)` takes a pitch.
That is evidence already in the tree - the method name - and it is the
single most common shape left in `class_debt`'s "scaffold name" census once
the mechanical copying passes are exhausted.

DELIBERATELY NARROW, because the convention stops being evidence quickly:

  - ONE parameter only. `set_attack(unsigned, unsigned, unsigned)` takes
    three, and the method name says nothing about which is which.
  - a `get_` is named ONLY when its parameter is an OUT-POINTER. For
    `get_listener_xpos(float *a1)` the pointer receives the x, so `x` is
    right; but `get_time(uint32_t a1)` takes a SELECTOR and returns the
    time, so naming that parameter `time` would be exactly backwards. The
    first version proposed it, which is why the rule is written down.
  - `set_`/`get_` prefix only. `play(int)`, `init(char *, unsigned long)`
    and `load(char *, unsigned long)` are NOT named here - "play" does not
    say what its int is, and guessing would be a convention dressed as a
    finding.
  - the parameter must still be scaffold (`aN`); a real name is never
    overwritten.
  - `set_xpos`/`ypos`/`zpos` map to x/y/z, which is the one abbreviation
    this tree uses consistently.

    uv run tools/setter_params.py src/wave.h            # dry run
    uv run tools/setter_params.py src/wave.h --apply
    uv run tools/setter_params.py --all --apply

A declaration parameter emits nothing, so this is byte-neutral; run
name_params afterwards to carry the names onto the definitions.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

DECL = re.compile(
    r"^(\s*(?:virtual\s+|static\s+)?[\w:]+[\s\*&]+)(set|get)_(\w+)"
    r"\(\s*([^;,()]+?\b)(a\d+)\s*\)\s*(const\s*)?;")
AXIS = {"xpos": "x", "ypos": "y", "zpos": "z"}


def rename(path: Path, apply: bool) -> int:
    lines = path.read_text(errors="replace").split("\n")
    n = 0
    for i, line in enumerate(lines):
        m = DECL.match(line)
        if not m:
            continue
        word = m.group(3)
        # A `get_` names its parameter only when that parameter is where the
        # value goes - an out-pointer. Otherwise it is a selector and the
        # method's name describes the RETURN, not the argument.
        if m.group(2) == "get" and "*" not in m.group(4):
            continue
        name = AXIS.get(word, word)
        if not name.isidentifier() or name in ("class", "int", "float"):
            continue
        lines[i] = f"{m.group(1)}{m.group(2)}_{m.group(3)}({m.group(4)}{name})" \
                   f"{' const' if m.group(6) else ''};"
        print(f"  {m.group(2)}_{word}: {m.group(5)} -> {name}")
        n += 1
    if apply and n:
        path.write_text("\n".join(lines))
    return n


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("header", type=Path, nargs="?")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--root", type=Path, default=Path("src"))
    args = ap.parse_args()

    headers = sorted(args.root.glob("*.h")) if args.all else [args.header]
    if not headers or headers == [None]:
        print(__doc__, file=sys.stderr)
        return 2
    total = 0
    for h in headers:
        before = total
        total += rename(h, args.apply)
        if total != before:
            print(f"  ^ {h.name}")
    print(f"{total} setter parameter(s) named from the method name"
          f"{'' if args.apply else '  (dry run; pass --apply)'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
