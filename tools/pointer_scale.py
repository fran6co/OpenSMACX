#!/usr/bin/env -S uv run python
r"""`reinterpret_cast<int *>(parent + 0xcc)` reads a DIFFERENT OBJECT.

`parent` is a `Win *`, so `parent + 0xcc` advances 0xcc whole Win objects -
about 900KB - not 0xcc bytes. The cast then reads there. The correct form
puts the cast FIRST, because a cast binds tighter than `+`:

    *reinterpret_cast<int *>(reinterpret_cast<char *>(parent) + 0xcc)   ok
    *reinterpret_cast<int *>(parent + 0xcc)                             wrong

Found 2026-08-25 in `Win::remove_parent_dialog`, where every one of eight
accesses to the parent's node list was scaled by sizeof(Win). It measured
12 of 61 instructions and nobody had asked why - a byte comparison cannot
tell a wrong displacement from a right one, it only sees that the numbers
differ.

SCOPED PER FUNCTION, and that is the whole difficulty. A name is `char *`
in one body and a `Win *` in another - win.cpp has both a `char *w` and an
`int w` - so a file-wide match reports the safe ones. This walks each
function, collects the pointers DECLARED there, and only then looks for the
cast-of-a-sum shape.

    uv run tools/pointer_scale.py            # the census
    uv run tools/pointer_scale.py --check    # exit 1 if any site exists
"""
from __future__ import annotations

import pathlib
import re
import sys

REPO = pathlib.Path(__file__).resolve().parent.parent

DEFN = re.compile(r"^[\w:][\w:\*&<>,\s]*?\b\w+\s*\([^;{)]*\)\s*\{")
# the cast wraps the SUM - that is the defect. A cast on the VARIABLE
# (`(char *)parent + 0xdc`) binds first and is byte arithmetic, which is
# correct however raw it looks.
SUM = re.compile(r"(?:reinterpret_cast<[^>]*\*[^>]*>|\(\s*[\w:]+\s*\*+\s*\))"
                 r"\s*\(\s*(\w+)\s*\+\s*0x[0-9A-Fa-f]{2,}\s*\)")
# a pointer to a CLASS: the scaling is only wrong when sizeof is not 1
CLASS_PTR = re.compile(r"\b([A-Z]\w+)\s*\*\s*(?:const\s+)?(\w+)\s*[=;,)]")
BYTE_PTR = re.compile(r"\b(?:char|void|unsigned char|uint8_t|int8_t)\s*"
                      r"\*\s*(?:const\s+)?(\w+)\s*[=;,)]")


def _code(lines) -> str:
    """The function's text with `//` tails and `/* */` blocks removed."""
    text = "\n".join(lines)
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return "\n".join(l.split("//", 1)[0] for l in text.splitlines())


def census():
    hits = []
    for path in sorted(REPO.glob("src/*.cpp")) + sorted(REPO.glob("src/*.h")):
        lines = path.read_text(errors="replace").splitlines()
        i = 0
        while i < len(lines):
            if not DEFN.match(lines[i]):
                i += 1
                continue
            j = i + 1
            while j < len(lines) and not lines[j].startswith("}"):
                j += 1
            body = _code(lines[i:j + 1])
            typed = {m.group(2) for m in CLASS_PTR.finditer(body)}
            bytes_ = {m.group(1) for m in BYTE_PTR.finditer(body)}
            for m in SUM.finditer(body):
                if m.group(1) in typed and m.group(1) not in bytes_:
                    hits.append((path.name, i + 1, m.group(0).strip()))
            i = j + 1
    return hits


def main() -> int:
    hits = census()
    for name, line, text in hits:
        print(f"  {name}: function at line {line}\n      {text[:78]}")
    print(f"{len(hits)} cast-of-a-sum on a class pointer"
          f"{' - each reads a different object' if hits else ''}")
    return 1 if hits and "--check" in sys.argv else 0


if __name__ == "__main__":
    sys.exit(main())
