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

THE OTHER DIRECTION, `--to-header`. Homing brings bodies whose parameters
someone HAS named while the declaration still reads `a1, a2, a3` - win.h
declared `OnRButtonDown(void *a1, long a2, int a3, int a4, unsigned int a5)`
against a definition spelling `(void *hwnd, long flags, int x, int y,
unsigned int keys)`. This copies the definition's names onto the
declaration, and it is safer than the forward direction rather than merely
as safe: a parameter name in a DECLARATION emits nothing at all, so there
is no body to clash with and nothing to re-measure.

    uv run tools/name_params.py src/win.h src/win.cpp --to-header --apply
"""
from __future__ import annotations

import pathlib
import re
import sys

SCAFFOLD = re.compile(r"^a\d+$")
# `int on_nc_hittest(int x, int y);` inside a class body
# `__cdecl` AND FRIENDS HAVE TO BE SKIPPED EXPLICITLY. Without them the
# name group swallowed the calling convention: win.h's file-scope
# `Win *__cdecl get_mouse_window_recurse(Win *window, int *x, int *y);`
# parsed as a function CALLED `__cdecl`, so its three good names never
# reached the definition and 51 `aN` mentions stayed put.
CONV = r"(?:__cdecl|__stdcall|__fastcall|__thiscall)"
DECL = re.compile(r"^\s*(?:virtual\s+|static\s+|friend\s+|extern\s+)*"
                  r"[\w:]+[\s\*&]+(?:" + CONV + r"\s+)?(\w+)\s*"
                  r"\(([^;{)]*)\)\s*(?:const\s*)?[;{]")
DEFN = re.compile(r"^[\w:][\w:\*&<>,\s]*?(?:" + CONV + r"\s+)?"
                  r"\b(?:(\w+)::)?(\w+)\s*\(([^;{)]*)\)\s*\{")


def _strip_comments(text: str) -> str:
    """The code, with `//` tails and `/* */` blocks removed."""
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return "\n".join(line.split("//", 1)[0] for line in text.splitlines())


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
        # KEYED ON (name, arity), not name alone. Keeping only the first
        # declaration meant an overload set gave up all but one member:
        # win.h declares `init` twice, at six and nine parameters, and the
        # nine-parameter definition was rejected on arity against the
        # six-parameter names. Two overloads of the same arity still cannot
        # be told apart here, so the first still wins for those.
        best.setdefault((m.group(1), len(names)), names)
    return best


def joined(lines: list[str]):
    """Each logical signature on one line.

    A DEFINITION THAT WRAPS IS STILL A DEFINITION. The regexes match a
    single line, so `void Win::window_line_raw(int x2, int y2, int x1,\n
    int y1, ...)` never parsed and its seven good names never reached the
    header - seven of the `aN` this tool exists to remove, invisible to it.
    Joins a line with the ones after it until its parentheses balance.
    """
    i = 0
    while i < len(lines):
        line = lines[i].rstrip("\n")
        if line.lstrip().startswith("//"):
            i += 1
            continue
        depth = line.count("(") - line.count(")")
        j = i
        while depth > 0 and j + 1 < len(lines) and j - i < 6:
            j += 1
            nxt = lines[j].rstrip("\n")
            line += " " + nxt.strip()
            depth += nxt.count("(") - nxt.count(")")
        yield i, j, line
        i = j + 1


def defn_names(source: pathlib.Path) -> dict[tuple[str, int], list[str]]:
    """Real parameter names taken from DEFINITIONS, keyed like header_names."""
    best: dict[tuple[str, int], list[str]] = {}
    for _i, _j, line in joined(source.read_text(errors="replace").splitlines(True)):
        m = DEFN.match(line)
        if not m:
            continue
        names = params(m.group(3))
        if not names or any(SCAFFOLD.match(n) for n in names):
            continue
        best.setdefault((m.group(2), len(names)), names)
    return best


def retype_decl(line: str, want: list[str]) -> str | None:
    """`line` with its parameter identifiers replaced by `want`."""
    m = DECL.match(line)
    if not m:
        return None
    inner = m.group(2)
    parts = inner.split(",")
    if len(parts) != len(want):
        return None
    out = []
    for part, name in zip(parts, want):
        hit = re.search(r"(\w+)(\s*(?:\[\s*\])?)$", part.rstrip())
        if not hit:
            return None
        out.append(part.rstrip()[:hit.start(1)] + name + hit.group(2))
    start = m.start(2)
    return line[:start] + ",".join(out) + line[m.end(2):]


def to_header(header: pathlib.Path, source: pathlib.Path, apply: bool) -> int:
    table = defn_names(source)
    lines = header.read_text(errors="replace").splitlines(True)
    named = 0
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
        named += sum(1 for h, w in zip(have, merged) if h != w)
        print(f"  {m.group(1)}: {', '.join(f'{h}->{w}' for h, w in zip(have, merged) if h != w)}")
    if apply:
        header.write_text("".join(lines))
    print(f"{named} declaration parameter(s) named from {source.name}"
          f"{'' if apply else '  (dry run; pass --apply)'}")
    return 0


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    header, source = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    apply = "--apply" in sys.argv
    if "--to-header" in sys.argv:
        return to_header(header, source, apply)
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
        want = table.get((name, len(have or [])))
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
        # THE CLASH TEST MUST NOT READ PROSE. Checking the raw body counted
        # a comment as a use: get_mouse_window_recurse was skipped because
        # one line says "the window's four render buffers", and its three
        # good names never reached it. Same defect compiler_work had, in a
        # different tool - strip comments before asking.
        code = _strip_comments(body)
        clash = [w for _, w in pairs if re.search(rf"\b{w}\b", code)]
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
