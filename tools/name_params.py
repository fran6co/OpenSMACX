#!/usr/bin/env -S uv run python
r"""Copy parameter names from a class's HEADER onto its definitions.

WHY. `class_debt`'s "scaffold name" shape counts every `aN` identifier left
in code: a parameter nobody has examined. Homing imports them by the
hundred - win.cpp arrived carrying 645 - and naming them one function at a
time is slow work that mostly re-derives something the tree ALREADY KNOWS.
`win.h` declares `on_mouse_move(int x, int y, unsigned int keys, int dbl)`
while `win.cpp` defines `on_mouse_move(int a1, int a2, unsigned int a3, int
a4)`. The header is the evidence; this copies it onto the definition.

SAFE BY CONSTRUCTION. A parameter rename changes no mangled name and no
emitted byte - only the identifier - so nothing needs re-measuring. The
rename is applied to the WHOLE FUNCTION BODY, so uses follow the parameter.
Refusals, all of them silent skips rather than guesses:

  - the header's own name is scaffold (`a1`, or absent) - nothing to copy;
  - arity disagrees between declaration and definition;
  - the target name is already used inside the body as something else,
    which would merge two distinct identifiers into one.

    uv run tools/name_params.py src/win.h src/win.cpp          # dry run
    uv run tools/name_params.py src/win.h src/win.cpp --apply
"""
from __future__ import annotations

import pathlib
import re
import sys

SCAFFOLD = re.compile(r"^a\d+$")
# `int on_nc_hittest(int x, int y);` inside a class body
DECL = re.compile(r"^\s*(?:virtual\s+|static\s+|friend\s+)*"
                  r"[\w:]+[\s\*&]+(\w+)\s*\(([^;{)]*)\)\s*(?:const\s*)?[;{]")
DEFN = re.compile(r"^[\w:][\w:\*&<>,\s]*?\b(?:(\w+)::)?(\w+)\s*\(([^;{)]*)\)\s*\{")


def params(text: str) -> list[str] | None:
    """The declared NAME of each parameter, or None when unparsable."""
    text = text.strip()
    if not text or text == "void":
        return []
    out = []
    for part in text.split(","):
        part = part.split("=")[0].strip()
        m = re.search(r"(\w+)\s*(?:\[\s*\])?$", part)
        if not m:
            return None
        out.append(m.group(1))
    return out


def header_names(header: pathlib.Path) -> dict[str, list[str]]:
    best: dict[str, list[str]] = {}
    for line in header.read_text(errors="replace").splitlines():
        if line.lstrip().startswith("//"):
            continue
        m = DECL.match(line)
        if not m:
            continue
        names = params(m.group(2))
        if names is None or not names:
            continue
        if any(SCAFFOLD.match(n) for n in names):
            continue
        # an overload set cannot be told apart by name alone; keep the first
        # and let arity disagreement reject the wrong pairing later.
        best.setdefault(m.group(1), names)
    return best


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    header, source = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    apply = "--apply" in sys.argv
    table = header_names(header)
    lines = source.read_text(errors="replace").splitlines(True)

    renamed = skipped = 0
    i = 0
    while i < len(lines):
        m = DEFN.match(lines[i])
        if not m:
            i += 1
            continue
        name = m.group(2)
        have = params(m.group(3))
        want = table.get(name)
        if not have or not want or len(have) != len(want):
            i += 1
            continue
        pairs = [(h, w) for h, w in zip(have, want)
                 if SCAFFOLD.match(h) and not SCAFFOLD.match(w)]
        if not pairs:
            i += 1
            continue
        # the body: to the matching closing brace at column 0
        j = i + 1
        while j < len(lines) and not lines[j].startswith("}"):
            j += 1
        body = "".join(lines[i:j + 1])
        clash = [w for _, w in pairs if re.search(rf"\b{w}\b", body)]
        if clash:
            skipped += 1
            print(f"  skip {name}: `{', '.join(clash)}` already used in the body")
            i = j + 1
            continue
        for h, w in pairs:
            body = re.sub(rf"\b{h}\b", w, body)
        lines[i:j + 1] = [body]
        renamed += len(pairs)
        print(f"  {name}: {', '.join(f'{h}->{w}' for h, w in pairs)}")
        i += 1

    if apply:
        source.write_text("".join(lines))
    print(f"{renamed} parameter(s) named from {header.name}, {skipped} skipped"
          f"{'' if apply else '  (dry run; pass --apply)'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
