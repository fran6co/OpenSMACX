#!/usr/bin/env -S uv run python
"""Classes spelled FLAT that duplicate another class's fields at the same offsets.

A class written `class Wave {` while the image says it IS a Sound cannot
produce the image's code, and the failure is not subtle: constructing a base
whose destructor is non-trivial, then calling something VC6 cannot prove
nothrow, is what emits the SEH frame. With no base declared there is nothing to
protect and no frame. Wave's constructor sat at 3 of 73 and its destructor at
2 of 101 for exactly this reason; declaring `class Wave : public Sound` and
deleting the 13 duplicated fields took them to 23/73 and 44/101, with all
twelve prologue instructions agreeing.

That was found by hand, on one class, after a stale refusal claimed the flat
spelling was protecting something. This looks for the rest of them.

The signal is field DUPLICATION: a baseless class whose leading run of member
declarations repeats another class's leading run, name for name. Nothing here
proves inheritance on its own - a candidate is a place to go measure, and the
layout arithmetic (sizeof(base) + own fields == the pinned size) is what
actually settles it.

    uv run tools/flat_classes.py
    uv run tools/flat_classes.py --min 4     # longer prefixes only

Exit 0 always: this is a worklist, not a gate.

WHAT IT FOUND, AND WHY THAT IS THE USEFUL PART. Run over the whole tree on
2026-08-22 it returns exactly ONE candidate pair: StringList repeats
StringStruct's first seven fields, `virtual_base_abi_word_` among them, and the
arithmetic settles it - sizeof(StringStruct) is 0x24, sizeof(StringList) is
0x30, and 0x24 + 12 is 0x30.

One. That is a real negative result and it should stop anyone re-running this
expecting a harvest: the flat-class defect was very nearly a one-off, so the
SEH-frame family will NOT mostly dissolve this way. The lever with the large
population is the other one - a constructor calling a base `construct()` in its
BODY, where the image calls it before the members are constructed. See
tools/body_construct_order.py.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

CLASS_HEAD = re.compile(r"^\s*(?:class|struct)\s+(\w+)\s*(?::\s*([^{]*))?\{")
# A data member: optional const/mutable, a type, then a name, then `;` or `[`.
# Deliberately NOT trying to be a C++ parser - a method has `(` before the `;`,
# which is the only distinction that matters here.
MEMBER = re.compile(
    r"^\s*(?!public|private|protected|friend|typedef|using|return)"
    r"(?:const\s+|mutable\s+|static\s+|unsigned\s+|signed\s+)*"
    r"[\w:<>]+\s*[*&]?\s*(\w+)\s*(?:\[[^\]]*\])?\s*;")


def classes(path: Path) -> dict[str, tuple[str, list[str]]]:
    """name -> (base-list, ordered field names). Brace-depth tracked, so a
    nested struct does not leak its fields into the enclosing class."""
    out: dict[str, tuple[str, list[str]]] = {}
    lines = path.read_text(errors="replace").splitlines()
    stack: list[tuple[str, str, list[str], int]] = []
    depth = 0
    for line in lines:
        head = CLASS_HEAD.match(line)
        if head and not line.rstrip().endswith(";"):
            stack.append((head.group(1), (head.group(2) or "").strip(), [],
                          depth))
            depth += line.count("{") - line.count("}")
            continue
        opened, closed = line.count("{"), line.count("}")
        if stack and depth - closed < stack[-1][3] + 1 and closed:
            name, base, fields, _ = stack.pop()
            out[name] = (base, fields)
        elif stack and opened == closed:
            member = MEMBER.match(line)
            if member and "(" not in line.split(";")[0]:
                stack[-1][2].append(member.group(1))
        depth += opened - closed
    return out


if __name__ == "__main__":
    minimum = 3
    if "--min" in sys.argv:
        minimum = int(sys.argv[sys.argv.index("--min") + 1])

    catalogue: dict[str, tuple[str, list[str], str]] = {}
    for path in sorted(SRC.glob("*.h")):
        for name, (base, fields) in classes(path).items():
            catalogue[name] = (base, fields, path.name)

    # A PREFIX OF PLACEHOLDER NAMES IS NOT EVIDENCE. `field_0_, field_4_,
    # field_8_` is what every un-named layout starts with, so matching on it
    # pairs each of them with all the others: the first run produced 118
    # candidates, and the top of the list was thirty spellings of that one
    # coincidence. A shared prefix only means something if at least one name in
    # it was chosen by a person - `vtable_storage_`, `volume_`, `chain_prev_`.
    generic = re.compile(r"^field_[0-9A-Fa-f]+_$")
    # hypothesis_layouts.h is EXPLICITLY speculative - layouts nobody has
    # measured yet. Pairing hypotheses with each other says nothing about the
    # image, and it buried the real hits.
    skip_files = {"hypothesis_layouts.h"}

    hits = []
    for name, (base, fields, where) in sorted(catalogue.items()):
        if base or len(fields) < minimum or where in skip_files:
            continue        # already derives, too small, or speculative
        for other, (_, other_fields, other_where) in catalogue.items():
            if other == name or len(other_fields) < minimum:
                continue
            shared = 0
            for mine, theirs in zip(fields, other_fields):
                if mine != theirs:
                    break
                shared += 1
            if other_where in skip_files:
                continue
            if shared >= minimum and any(
                    not generic.match(n) for n in fields[:shared]):
                hits.append((shared, name, where, other, other_where,
                             fields[:shared]))

    hits.sort(key=lambda h: -h[0])
    for shared, name, where, other, other_where, names in hits:
        print(f"  {name} ({where}) repeats {other} ({other_where})'s first "
              f"{shared} field(s)")
        print(f"      {', '.join(names)}")
        print(f"      check:  sizeof({other}) + {name}'s own == its pinned size")

    if not hits:
        print("  no baseless class repeats another's leading fields")
    print(f"\n{len(catalogue):,} class(es) read from {len(list(SRC.glob('*.h'))):,} "
          f"header(s); {len(hits)} candidate(s) at prefix >= {minimum}")
