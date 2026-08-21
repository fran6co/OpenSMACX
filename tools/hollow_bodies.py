#!/usr/bin/env -S uv run python
"""Markers claiming a substantial image body over an empty or trivial source.

`Popup`'s constructor marker claims 87 image bytes and its body is
`Popup() { ; }`. `Scroll`'s was the same, and the consequence was real: the
recovered program never constructed Popup's `Scroll` member at all, while the
image calls `Scroll::Scroll` on `this + 0x3230` every time.

None of these are claimed BYTE_EXACT, so nothing false is being ASSERTED - but
an empty body under an 87-byte marker reads as finished and is not, and the
catalogue counts it among the bodies that exist.

    uv run tools/hollow_bodies.py [--min-bytes N]

EMPTY OR A BARE RETURN, not merely "thin". A ratio test on statement count
against image bytes reports 35 markers and most are correct: `log_say` is one
statement because it delegates to a helper the image INLINES, and one statement
is exactly the right transcription of that. What is never right is a body that
does NO WORK standing in for dozens of image bytes - `Popup() { ; }` under an
87-byte marker, which is why Popup never constructed its `Scroll` member.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read

REPO_ROOT = Path(__file__).resolve().parent.parent
MIN_BYTES = 40


def blanked(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)),
                  text, flags=re.S)
    return re.sub(r"//[^\n]*", lambda m: " " * len(m.group(0)), text)


_CACHE: dict = {}


def _source_of(record):
    """(path, first line) where this record's body actually lives.

    A marker whose body is in a HEADER carries a `body` fact naming it - the
    marker itself stays in the .cpp because a header is not a compilation unit.
    Reading only `location` misses those entirely, which is how
    `Popup() { ; }` under an 87-byte marker went unreported: the marker is at
    popup.cpp:814 and the body is in popup.h.
    """
    if record.body:
        header = REPO_ROOT / record.body
        if header.exists():
            # THE MANGLING DECIDES WHAT TO SEARCH FOR. `??0Popup@@QAE@XZ` is a
            # CONSTRUCTOR - the C++ spelling is `Popup(` - and naively taking
            # everything before the first `@` gives `0Popup`, which matches
            # nothing and silently falls back to line 1 of the header. That is
            # how this tool read the wrong body and reported the wrong answer.
            mangled = record.name or ""
            ctor = re.match(r"\?\?0(\w+)@@", mangled)
            dtor = re.match(r"\?\?1(\w+)@@", mangled)
            method = re.match(r"\?(\w+)@\w+@@", mangled)
            free = re.match(r"\?(\w+)@@", mangled)
            if ctor:
                name = ctor.group(1)
            elif dtor:
                name = "~" + dtor.group(1)
            elif method:
                name = method.group(1)
            elif free:
                name = free.group(1)
            else:
                name = ""
            if name:
                try:
                    for number, line in enumerate(
                            header.read_text(errors="replace").splitlines(), 1):
                        if re.search(rf"\b{re.escape(name)}\s*\(", line):
                            return header, number
                except OSError:
                    pass
            return header, 1
    where = str(record.location)
    path_s, _, line = where.rpartition(":")
    return Path(path_s), int(line)


def statements(record) -> int | None:
    """Non-blank statement lines in the body following a record's marker."""
    path, line = _source_of(record)
    if path not in _CACHE:
        try:
            _CACHE[path] = blanked(path.read_text(errors="replace")).splitlines()
        except OSError:
            _CACHE[path] = []
    lines = _CACHE[path]
    text = "\n".join(lines[line - 1: line + 400])
    start = text.find("{")
    if start < 0:
        return None
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                inner = text[start + 1:i]
                return len([s for s in inner.split("\n") if s.strip()])
    return None


# A bare `;` counts. `Popup() { ; }` is a NULL STATEMENT - the idiom this tree
# uses for "constructor that does nothing" - and it does no work at all, which
# is the whole point of this check. A pattern that only accepted `return`
# missed every one of them.
TRIVIAL = re.compile(r"\s*(;|return\s*(-?\d+|nullptr|false|true)?\s*;)?\s*")


def _trivial(record) -> bool:
    """Is the single statement a bare `return`, doing no work?"""
    path, line = _source_of(record)
    lines = _CACHE.get(path, [])
    text = "\n".join(lines[line - 1: line + 400])
    start = text.find("{")
    if start < 0:
        return False
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return bool(TRIVIAL.fullmatch(text[start + 1:i]))
    return False


if __name__ == "__main__":
    floor = MIN_BYTES
    if "--min-bytes" in sys.argv:
        floor = int(sys.argv[sys.argv.index("--min-bytes") + 1])

    rows = []
    for record in read(REPO_ROOT / "src"):
        where = str(record.path)
        if "/recovered/" in where or "/unrecovered/" in where:
            continue
        if record.byte_exact or record.semantic or not record.size:
            continue
        if record.size < floor:
            continue
        count = statements(record)
        if count is None:
            continue
        if count == 0 or (count == 1 and _trivial(record)):
            rows.append((record.size, count, record))

    rows.sort(key=lambda r: -r[0])
    for size, count, record in rows:
        print(f"  {record.address_hex}  {size:5,}b image, {count:2d} "
              f"statement(s)   {record.path.name:20s} {record.name}")
    print(f"\n{len(rows)} marker(s) with an EMPTY or bare-return body over "
          f">= {floor} image bytes")
