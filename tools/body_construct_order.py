#!/usr/bin/env -S uv run python
"""Constructors that call a base `construct()` in the BODY, behind their members.

C++ runs member constructors BEFORE the constructor body. So a constructor
written

    Scroll::Scroll() {
        GraphicWin::construct();      // <- cannot run before the members
        ...
    }

emits the base construction AFTER every member with a non-trivial constructor,
and if the image calls it FIRST the two can never agree - the call targets
match and the call ORDER does not. Scroll sat at 21 of 83 for this reason, and
the marker's own RULED-OUT had diagnosed it correctly before concluding the fix
needed raw storage and placement-new. It does not: base subobjects are
constructed before members, so moving the call into a base uses C++'s own
ordering rule and leaves the layout alone. 21 -> 81 of 83.

The reorder was worth 1 instruction by itself. What it bought was VISIBILITY:
two more levers underneath were unreachable until the order was right, and the
larger of them was worth 55. That is the reason to care about this list even
where the ordering fix alone looks small.

MEASURED ON ALL SEVEN, 2026-08-22, AND THE RESULT IS NOT "APPLY IT EVERYWHERE".
The image order was verified for each - every one of them really does
`call 0x005D4CF0` before its first `lea ecx, [esi + N]` - and the fix still went
three different ways:

    AlphaMovie   15/24  -> 24/24   BYTE_EXACT
    BaseWin     104/144 -> 104/144, compiled 153 -> 151
    StringBox     6/47  -> 6/47,   compiled  49 -> 46
    PickWin      21/101 -> 16/101  WORSE
    ReportWin    14/87  -> 10/87   WORSE
    SocialWin    16/121 -> 16/121, compiled 133 -> 134

A CORRECT ORDERING FIX CAN LOWER THE SCORE, and that is the thing to know
before running this list. It moves the first divergence EARLIER, so a body whose
remaining problems are structural measures worse while being more faithful.
Scroll is the counter-case: +1 from the reorder, then +59 from two levers it
made reachable. PickWin and ReportWin have no such lever waiting - their vtable
constants already fold, and they are 134 and 149 compiled instructions against
101 and 87 - so the reorder was reverted there and the finding written onto
their markers instead of being re-derived later.

Use this list to pick bodies for a DEDICATED pass. Do not sweep it.

A row here is a CANDIDATE, not a defect. It matters only when the class also
has members whose constructors do something - which is what the second column
reports - and `osmx measure` is what settles it.

    uv run tools/body_construct_order.py
    uv run tools/body_construct_order.py --with-members   # only the ones that can bite

Exit 0 always: a worklist, not a gate.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

# `Base::construct(` at the top level of a constructor body.
CONSTRUCT_CALL = re.compile(r"^\s*(\w+)::construct\s*\(")
# `Class::Class(` - a constructor definition, not a declaration.
CTOR_DEF = re.compile(r"^(\w+)::(\1)\s*\([^;]*\)\s*(?::[^{]*)?\{")
CLASS_HEAD = re.compile(r"^\s*class\s+(\w+)\s*(?::\s*([^{]*))?\{")
# A member whose type is a class this tree declares - i.e. one that can have a
# constructor of its own. Pointers do not count: `Foo *p;` constructs nothing.
MEMBER = re.compile(
    r"^\s+(?!return|public|private|protected|friend|typedef|using|static)"
    r"([A-Z]\w+)\s+(\w+)\s*(?:\[[^\]]*\])?\s*;")


def declared_classes() -> set[str]:
    names = set()
    for path in SRC.glob("*.h"):
        for line in path.read_text(errors="replace").splitlines():
            head = CLASS_HEAD.match(line)
            if head:
                names.add(head.group(1))
    return names


def members_with_ctors(cls: str, known: set[str]) -> list[str]:
    """Members whose type is a class - the ones VC6 constructs before the body."""
    found = []
    for path in SRC.glob("*.h"):
        text = path.read_text(errors="replace")
        match = re.search(rf"^\s*class\s+{re.escape(cls)}\s*(?::[^{{]*)?\{{",
                          text, re.M)
        if not match:
            continue
        depth, started = 0, False
        for line in text[match.start():].splitlines():
            depth += line.count("{") - line.count("}")
            started = started or "{" in line
            member = MEMBER.match(line)
            if member and member.group(1) in known and "(" not in line:
                found.append(f"{member.group(1)} {member.group(2)}")
            if started and depth <= 0:
                break
        break
    return found


if __name__ == "__main__":
    known = declared_classes()
    only_biting = "--with-members" in sys.argv

    rows = []
    for path in sorted(SRC.glob("*.cpp")):
        lines = path.read_text(errors="replace").splitlines()
        current, depth = None, 0
        for line in lines:
            ctor = CTOR_DEF.match(line)
            if ctor:
                current, depth = ctor.group(1), line.count("{") - line.count("}")
                continue
            if current is None:
                continue
            call = CONSTRUCT_CALL.match(line)
            if call and call.group(1) != current:
                members = members_with_ctors(current, known)
                rows.append((current, call.group(1), path.name, members))
                current = None
                continue
            depth += line.count("{") - line.count("}")
            if depth <= 0:
                current = None

    shown = 0
    for cls, base, where, members in sorted(rows):
        if only_biting and not members:
            continue
        shown += 1
        mark = "BITES" if members else "  -  "
        print(f"  {mark}  {cls}::{cls} calls {base}::construct() in its body "
              f"({where})")
        if members:
            print(f"          constructed BEFORE the body: {', '.join(members)}")

    print(f"\n{len(rows)} constructor(s) call a base construct() in the body; "
          f"{sum(1 for r in rows if r[3])} have members that make the order "
          f"observable")
    if not only_biting:
        print("  --with-members lists only those")
