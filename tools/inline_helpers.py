#!/usr/bin/env -S uv run python
"""Move tree-invented single-`return` helpers into their headers as `inline`.

WHY. Some helpers in this tree name something the shipped image OPEN-CODES -
`map_loc`, `altitude_at`, `is_human`. They carry `Original Offset: n/a`,
because there is no body in the image to point at. Defined in a `.cpp`, VC6
cannot inline them across translation units, so every caller pays a `call` the
image does not make - and a caller cannot be byte exact while it does.

`map_loc` alone was in the way of a hundred bodies under map.cpp. This finds
the rest.

WHAT IT TAKES. Only a helper whose whole body is one `return` statement, whose
header already declares it, and whose declaration is not inside a class. A
longer body is a judgement about whether VC6 would inline it at all, and this
tool does not make judgements - `try_spellings` is where that goes.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

# THE WHOLE DOC BLOCK, not from "Original Offset" onwards: that line lives
# INSIDE a `/* ... */`, and cutting from there leaves the opening `/*`
# orphaned - which CL reports as "unexpected end of file found in comment"
# hundreds of lines later, nowhere near the edit.
HELPER = re.compile(
    r"/\*\n(?:(?!\*/)[^\n]*\n)*?Original Offset: n/a\n"
    r"(?:(?!\*/)[^\n]*\n){0,4}\*/\n"
    r"(?P<sig>[\w:*&<>,\s]+?\b(?P<name>\w+)\s*\((?P<params>[^;{)]*)\))\s*\{\n"
    r"(?P<body>    return [^;]*;\n)\}\n")


def visible(header: Path, files: dict, seen: set | None = None) -> str:
    """The header's text plus every header it includes, transitively.

    A helper's body can name anything its .cpp could see; the header it moves
    to sees less. `label_get` reads `StringTable`, which alpha.cpp includes and
    alpha.h does not - moved blind, it does not compile, and the error names
    the CALLER rather than the move.
    """
    seen = seen if seen is not None else set()
    if header in seen or header not in files:
        return ""
    seen.add(header)
    text = files[header]
    for name in re.findall(r'#include\s+"([^"]+)"', text):
        text += visible(SRC / name, files, seen)
    return text


def _append(header_text: str, block: str) -> str:
    """`block` at the end of the header, inside its include guard."""
    end = header_text.rfind("#endif")
    if end < 0:
        return header_text.rstrip("\n") + "\n\n" + block
    return header_text[:end] + block + "\n" + header_text[end:]


def main(apply: bool) -> int:
    files = {path: path.read_text()
             for path in sorted(SRC.glob("*.[ch]*"))
             if path.suffix in (".c", ".h", ".cpp", ".hpp")}
    moved = 0
    for path in [p for p in files if p.suffix == ".cpp"]:
        header = path.with_suffix(".h")
        if header not in files:
            continue
        for match in list(HELPER.finditer(files[path])):
            name, sig = match.group("name"), match.group("sig").strip()
            declaration = re.compile(
                rf"^{re.escape(sig)}\s*;[^\n]*\n", re.M)
            if not declaration.search(files[header]):
                print(f"  - {name}: {header.name} does not declare it as "
                      f"`{sig};`")
                continue
            scope = visible(header, files)
            unknown = sorted({
                word for word in re.findall(r"[A-Za-z_]\w*",
                                            match.group("body"))
                if word not in scope and not word.isdigit()})
            if unknown:
                print(f"  - {name}: {header.name} cannot see "
                      f"{', '.join(unknown[:3])}")
                continue
            inline = (f"// INLINE: the image has no {name} - it open-codes "
                      f"what this names.\ninline {sig} {{\n"
                      f"{match.group('body')}}}\n")
            # AT THE END OF THE HEADER, not where the declaration stood. An
            # inline body must see every name it uses, and a helper declared
            # early routinely reads a table declared late - `on_map` sits
            # above `RadiusBaseX` in map.h and would not compile in place.
            files[header] = declaration.sub("", files[header])
            files[header] = _append(files[header], inline)
            files[path] = files[path].replace(match.group(0), "")
            print(f"  + {name} -> {header.name}")
            moved += 1
    if apply:
        for path, body in files.items():
            if body != path.read_text():
                path.write_text(body)
    return moved


if __name__ == "__main__":
    count = main("--apply" in sys.argv)
    print(f"{count} helper(s) {'moved' if '--apply' in sys.argv else 'movable'}")
