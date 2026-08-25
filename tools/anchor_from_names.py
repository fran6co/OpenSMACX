#!/usr/bin/env python3
"""Anchor a class layout from its own `field_NN_` member names, and CHECK it.

`header_offsets.py --survey` says 48 classes declare members but carry no
`// 0xNN` anchor, so no offset map can be derived for them - and that blocks
`name_offsets.py`, which is the mechanical remedy for the "raw self-access"
shape. It blocks derived classes too: EditGroup's cleanup needs Dialog's
layout, and Dialog had 56 members and no anchor.

But the names already carry the answer. Dialog declares `field_1C_`,
`field_20_`, `field_24_` ... `field_6C_`, and a member called `field_40_`
is at 0x40 by this tree's own convention.

SELF-VERIFYING, which is the whole point. Annotating the FIRST such member
is a claim; walking the members from there and landing on every other
`field_NN_` at exactly NN is the PROOF. A class whose walk disagrees is not
annotated - the disagreement is a layout finding, and papering over it with
an anchor would hide exactly what the ratchet is for.

    uv run tools/anchor_from_names.py            # what would be anchored
    uv run tools/anchor_from_names.py --apply

WHAT IS LEFT AFTER THIS, measured 2026-08-26: 36 classes still carry no
anchor, and none of them can be reached the same way. A second rule was
considered and DISCARDED against the tree rather than shipped as a comment:
"a class with no base starts its data at 0 (or 4 with a vfptr), and its own
static_assert(sizeof) verifies the walk" - true, and it applies to ZERO of
the 36, because every one of them either has a base or has no size assert.
Those need per-class evidence from the image.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from header_offsets import class_body, derive, sizes  # noqa: E402

FIELD = re.compile(r"^(\s+)([\w:]+)\s+(field_([0-9A-Fa-f]+)_);")


def candidates(header: Path, cls: str):
    """(line index within the file, indent, name, offset) per field_NN_."""
    body = class_body(header.read_text(errors="replace"), cls)
    if not body:
        return []
    text = header.read_text(errors="replace")
    lines = text.split("\n")
    start = next((i for i, l in enumerate(lines)
                  if re.match(r"^class\s+" + cls + r"\b", l)), None)
    if start is None:
        return []
    out = []
    depth = 0
    for i in range(start, len(lines)):
        depth += lines[i].count("{") - lines[i].count("}")
        m = FIELD.match(lines[i])
        if m:
            out.append((i, m.group(1), m.group(3), int(m.group(4), 16)))
        if depth <= 0 and i > start:
            break
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", type=Path, default=Path("src"))
    ap.add_argument("--apply", action="store_true")
    args = ap.parse_args()

    known = sizes(args.root)      # the tree's own static_assert(sizeof) table
    anchored = refused = 0
    for header in sorted(args.root.glob("*.h")):
        text = header.read_text(errors="replace")
        for m in re.finditer(r"^class (\w+)\s*(?::[^{]*)?\{", text, re.M):
            cls = m.group(1)
            try:
                table, _bad, seen, found = derive(header, cls, known)
            except Exception:
                continue
            if not found or not seen or table:
                continue                      # already anchored, or nothing to walk
            fields = candidates(header, cls)
            if len(fields) < 3:
                continue                      # too few names to prove anything
            enough = False
            i, indent, name, off = fields[0]
            lines = text.split("\n")
            lines[i] = lines[i].rstrip() + f"  // {off:#06x}"
            trial = "\n".join(lines)
            header.write_text(trial)
            try:
                table2, bad2, _s, _f = derive(header, cls, known)
                agree = all(table2.get(f"{o:#x}") == n for _i, _ind, n, o in fields
                            if f"{o:#x}" in table2)
                covered = sum(1 for _i, _ind, n, o in fields if f"{o:#x}" in table2)
                # MOST of the names must be reached, not three of sixteen.
                # StringBox passed a bare `>= 3` bar while the walk stopped
                # at an unsized member after its third field - which proves
                # almost nothing about the layout it was being asked to pin.
                enough = covered >= 3 and covered >= 0.8 * len(fields)
            finally:
                if not (agree and not bad2 and enough) or not args.apply:
                    header.write_text(text)
            if agree and not bad2 and enough:
                anchored += 1
                print(f"  ANCHOR {cls:<20} {name} at {off:#06x}; "
                      f"{covered}/{len(fields)} field names verified by the walk")
            else:
                refused += 1
                # TWO DIFFERENT REFUSALS, and calling both "does not
                # reproduce its own names" reported seven of my own tool's
                # missing type sizes as findings about the tree.
                if not agree:
                    why = (f"DISAGREES - a field_NN_ name lands somewhere "
                           f"other than NN")
                elif bad2:
                    why = "an existing // 0xNN annotation disagrees with the walk"
                else:
                    why = (f"only {covered} of {len(fields)} names reached; "
                           f"the walk stops at a member whose size is unknown")
                print(f"  refuse {cls:<20} {why}")
    print(f"\n{anchored} class(es) anchored, {refused} refused"
          f"{'' if args.apply else '  (dry run; pass --apply)'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
