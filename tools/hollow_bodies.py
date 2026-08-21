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
    uv run tools/hollow_bodies.py --stubbed

`--stubbed` finds the SAME defect one step further out: a class whose method is
`{ ; }` in a header while the only marker for its address lives in an artifact.
Those fall between both detectors - this one needs a product-tree marker and
`promotable.py` only reports artifacts that already reproduce - so
`StringBox::StringBox` (266 image bytes) and `MainInterface::MainInterface`
(1,185) were invisible to both while being the two bodies that stop the built
executable reaching `WinMain`.

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


def _cpp_name(mangled: str | None) -> str | None:
    """The C++ spelling a header would use for this mangled symbol."""
    mangled = mangled or ""
    for pattern, build in (
            (r"\?\?0(\w+)@@", lambda m: m.group(1)),
            (r"\?\?1(\w+)@@", lambda m: "~" + m.group(1)),
            (r"\?(\w+)@(\w+)@@", lambda m: m.group(1)),
            (r"\?(\w+)@@", lambda m: m.group(1))):
        found = re.match(pattern, mangled)
        if found:
            return build(found)
    return None


def stubbed(records) -> list:
    """Artifact-only bodies whose product-side definition does nothing.

    An empty inline in a header is not a transcription of a 1,185-byte
    constructor, and when the marker lives only in an artifact NEITHER other
    check sees it: this file's main mode wants a product marker, and
    `promotable.py` wants the artifact to be BYTE_EXACT already.
    """
    product = {r.address for r in records
               if "/recovered/" not in str(r.path)
               and "/unrecovered/" not in str(r.path)}
    headers = {}
    for header in sorted((REPO_ROOT / "src").glob("*.h")):
        try:
            headers[header] = blanked(header.read_text(errors="replace"))
        except OSError:
            continue

    rows = []
    for record in records:
        whole = sum(high - low for low, high in record.image_spans)
        if record.address in product or not whole:
            continue
        where = str(record.path)
        if "/recovered/" not in where and "/unrecovered/" not in where:
            continue
        name = _cpp_name(record.name)
        if not name:
            continue
        # An empty body: `Name() { ; }` or `Name() {}`, with anything for args.
        #
        # The `~` is load-bearing IN BOTH DIRECTIONS. `\bScroll` matches inside
        # `~Scroll`, because there is a word boundary between the tilde and the
        # S - so without the lookbehind a class with a correctly declared
        # constructor and a stubbed DESTRUCTOR reads as a stubbed constructor.
        # `Scroll` was exactly that: `Scroll();` on one line and
        # `~Scroll() { ; }` on the next.
        if name.startswith("~"):
            head = r"~\s*" + re.escape(name[1:])
        else:
            head = r"(?<![~\w])" + re.escape(name)
        empty = re.compile(
            rf"{head}\s*\([^;{{}}]*\)\s*(?:const\s*)?\{{\s*;?\s*\}}")
        for header, text in headers.items():
            if empty.search(text):
                rows.append((whole, record, header))
                break
    return rows


if __name__ == "__main__":
    floor = MIN_BYTES
    if "--min-bytes" in sys.argv:
        floor = int(sys.argv[sys.argv.index("--min-bytes") + 1])

    all_records = read(REPO_ROOT / "src")
    if "--stubbed" in sys.argv:
        found = sorted(stubbed(all_records), key=lambda r: -r[0])
        for size, record, header in found:
            print(f"  {record.address_hex}  {size:5,}b image   "
                  f"{header.name:20s} {record.name}")
            print(f"      transcription in {record.path.relative_to(REPO_ROOT / 'src')}")
        print(f"\n{len(found)} artifact-only body(s) whose product-side "
              f"definition is an empty inline")
        sys.exit(0)

    rows = []
    for record in all_records:
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

    # AND WHETHER THE REAL BODY ALREADY EXISTS. `Midi_Device`'s constructor was
    # `{ ; }` under a 60-byte marker while a BYTE_EXACT transcription of it sat
    # in `src/recovered/units/004c5740.cpp` - so the work was not "write this
    # body" but "promote the one already written". Eight of the nine had one.
    # Promoting is still not copying: the artifact reaches its fields through
    # `reinterpret_cast<char *>(this) + 0x18` and the class usually names them.
    def artifact_for(address: int) -> Path | None:
        stem = f"{address:08x}"
        for where in ("recovered", "recovered/units", "unrecovered"):
            candidate = REPO_ROOT / "src" / where / f"{stem}.cpp"
            if candidate.exists():
                return candidate
        return None

    rows.sort(key=lambda r: -r[0])
    have = 0
    for size, count, record in rows:
        art = artifact_for(record.address)
        if art:
            have += 1
        where = (f"  <- {art.relative_to(REPO_ROOT / 'src')}"
                 if art else "  (no artifact - must be written)")
        print(f"  {record.address_hex}  {size:5,}b image, {count:2d} "
              f"statement(s)   {record.path.name:20s} {record.name}")
        print(f"      {where}")
    print(f"\n{len(rows)} marker(s) with an EMPTY or bare-return body over "
          f">= {floor} image bytes; {have} have a transcription in an artifact "
          f"waiting to be promoted")
