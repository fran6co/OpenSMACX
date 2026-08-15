#!/usr/bin/env python3
"""Which member offsets does this class's own code touch, and what are they called?

TWO FUNCTIONS WERE DEFERRED IN ONE BATCH BECAUSE OF A PROSE CAVEAT. An agent
holding `InfoWin::setup_text` (0x00456300, 8,333 bytes) read `src/infowin.h`
saying "the layout is not established and nothing pins its sizeof", concluded
"every offset used would be a guess", and stopped. Measured afterwards: all 30
offsets that InfoWin's methods touch are already declared, named fields in that
header. Nothing needed guessing. The same batch lost a second function to the
same shape of misreading in `src/dialogs.h`.

THE CAVEAT AND THE OFFSETS ARE DIFFERENT FACTS. "sizeof is not pinned" and
"nobody knows what field_838_ MEANS" are both true and both worth saying. They
say nothing about WHERE the fields are, because that was never a hypothesis:

    member-accesses.csv    read off `[ecx + N]` in the class's own methods.
                           The compiler emitted those displacements from the
                           real declaration, so they are evidence of the same
                           kind as a jump table or a destructor receiver.
    a declared layout      a proposal about bytes nobody has watched being
                           used, which is why `class_section` names it a
                           hypothesis and withholds it.

`agent_brief.class_section` withholds offsets on purpose, and for the second
kind that is correct - a wrong offset compiles and fails later as a byte
mismatch nobody traces back. This module carries the FIRST kind, which the
brief restates for jump tables and frame objects already and had no reason to
withhold here.

THE NAME IS THE OFFSET. Every field in this tree is spelled `field_<HEX>_` at
that hex offset, so an agent needs no table to convert one to the other - it
needs to be told the convention holds. `verify_member_offsets --check-names`
is what makes that safe to say: it compares each name against the offset the
compiler actually assigned, so a field whose name has drifted off its slot
fails the gate instead of silently handing agents a wrong byte.

AN UNDECLARED OFFSET IS THE FINDING WORTH HAVING. When the image touches a
byte no member declares, that is a real gap in the header, it is exactly what
the STRUCTURE channel is for, and it is the one case here where an agent must
not invent a spelling.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
# `uint32_t field_838_;` / `uint8_t field_B9_[0x77F];` - the name states the
# offset, which is the convention `--check-names` holds the tree to.
FIELD = re.compile(r"\bfield_([0-9A-Fa-f]+)_\s*(?:\[[^\]]*\])?\s*;")
# `class DLLEXPORT InfoWin {` / `class Popup : ... {`
CLASS = re.compile(r"^\s*(?:class|struct)\s+(?:DLLEXPORT\s+)?(\w+)\b", re.M)


def observed(klass: str) -> list:
    """[(offset, width, evidence)] the image proves this class's code touches."""
    import verify_member_offsets

    found, _ = verify_member_offsets.load_accesses()
    return sorted((offset, width, evidence)
                  for (offset, width), evidence in found.get(klass, {}).items())


def header_for(klass: str, src: Path = SRC) -> Path | None:
    """The header declaring this class, found by reading rather than guessing.

    `src/<lowercase>.h` is right often enough to be tempting and wrong often
    enough to matter - Popup lives in basepop.h.
    """
    for path in sorted(src.glob("*.h")):
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        if any(name == klass for name in CLASS.findall(text)):
            return path
    return None


def declared_offsets(path: Path) -> set:
    """Every offset a `field_<HEX>_` name in this header claims."""
    if path is None or not path.is_file():
        return set()
    text = path.read_text(encoding="utf-8", errors="replace")
    return {int(hexed, 16) for hexed in FIELD.findall(text)}


def render(klass: str, src: Path = SRC) -> str:
    """The member map for the brief, or "" when the image proves nothing."""
    rows = observed(klass)
    if not rows:
        return ""
    path = header_for(klass, src)
    claimed = declared_offsets(path)
    where = f"src/{path.name}" if path else "no header found"
    lines = ["", f"# The member offsets {klass}'s own code touches", "",
             "READ FROM THE IMAGE, not proposed. Each line below is a real",
             f"`[ecx + N]` access inside a {klass} method, so the compiler",
             "emitted that displacement from the real declaration and it",
             "cannot be wrong about where the field sits.",
             "",
             f"THIS IS NOT THE SAME CLAIM AS THE LAYOUT IN {where}. A header",
             "note saying a layout is unestablished, or that nothing pins",
             "sizeof, is about how BIG the class is and what its fields MEAN.",
             "It is not a statement that these offsets are unknown, and two",
             "functions were deferred in one batch for reading it that way.",
             "",
             "The name states the offset: the field at 0x838 is `field_838_`,",
             "and `verify_member_offsets --check-names` fails the gate if any",
             "name has drifted off its slot. So you can spell any offset here",
             "without looking it up.",
             ""]
    missing = []
    for offset, width, evidence in rows:
        mark = "" if offset in claimed else "   <-- NOT DECLARED"
        if offset not in claimed:
            missing.append(offset)
        lines.append(f"    0x{offset:04X}  {width} byte(s)   field_{offset:X}_"
                     f"{mark}      {evidence}")
    if missing:
        lines += ["",
                  f"  {len(missing)} offset(s) above have no member declaring",
                  f"  them in {where}. That is a real gap in the header rather",
                  "  than something for you to spell around: report it on the",
                  "  STRUCTURE channel and use the byte array that covers it."]
    return "\n".join(lines)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("classes", nargs="+", help="class names, e.g. InfoWin")
    parser.add_argument("--src", type=Path, default=SRC)
    arguments = parser.parse_args(argv)
    for klass in arguments.classes:
        text = render(klass, arguments.src)
        print(text or f"the image proves no member access in {klass}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
