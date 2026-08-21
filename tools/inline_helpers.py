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
# `(?!\*/)` rejects only a line that STARTS with `*/`, and this tree closes
# some blocks with a leading space - so the skip ran from the licence header
# down to the first marker and took every `#include` with it. `(?!.*\*/)`
# rejects the close wherever it sits on the line.
HELPER = re.compile(
    r"/\*\n(?:(?!.*\*/)[^\n]*\n)*?Original Offset: n/a\n"
    r"(?:(?!.*\*/)[^\n]*\n){0,4}\*/\n"
    r"(?P<sig>[\w:*&<>,\s]+?\b(?P<name>\w+)\s*\((?P<params>[^;{)]*)\))\s*\{\n"
    r"(?P<body>    return [^;]*;\n)\}\n")


# Words that are never a name a header must supply.
KEYWORDS = frozenset("""void bool char short int long float double signed
unsigned const volatile struct class union enum static inline return if else
for while do switch case default break continue sizeof true false nullptr
new delete this operator typedef namespace using template typename public
private protected virtual explicit friend""".split())


def _names(body: str) -> set[str]:
    """The identifiers a body needs, with comments and keywords removed.

    Scanning the raw text asked headers to supply `false`, `TODO` and `MP` -
    two of those live in prose, and refusing on them dropped real candidates.
    """
    code = re.sub(r"/\*.*?\*/|//[^\n]*", " ", body, flags=re.S)
    return {w for w in re.findall(r"[A-Za-z_]\w*", code)
            if w not in KEYWORDS}


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
    # WITHOUT COMMENTS. Every prose block in this tree names the functions it
    # discusses, so `bitmask` "appeared in" map.h's scope while being declared
    # nowhere near it - and the move compiled everywhere except the one file
    # that needed it.
    text = re.sub(r"/\*.*?\*/|//[^\n]*", " ", files[header], flags=re.S)
    for name in re.findall(r'#include\s+"([^"]+)"', text):
        text += visible(SRC / name, files, seen)
    return text


def _append(header_text: str, block: str) -> str:
    """`block` at the very end of the header, inside an include guard only.

    "Before the last `#endif`" is wrong: time.h is `#pragma once` and has an
    `#endif` at line 159 closing a `static_assert` guard, with declarations
    after it - so the block landed above names it needed. The `#endif` is only
    the file's guard when nothing but blank lines follows it.
    """
    end = header_text.rfind("#endif")
    if end >= 0 and not header_text[end:].splitlines()[1:] :
        return header_text[:end] + block + "\n" + header_text[end:]
    if end >= 0 and not any(line.strip()
                            for line in header_text[end:].splitlines()[1:]):
        return header_text[:end] + block + "\n" + header_text[end:]
    return header_text.rstrip("\n") + "\n\n" + block


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
            unknown = sorted(w for w in _names(match.group("body"))
                             if w not in scope)
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


EXISTING = re.compile(
    r"(?:^//[^\n]*\n)*^(?:MEASURED )?inline "
    r"[\w:*&<>,\s]+?\b(?P<ident>\w+)\s*\([^;{)]*\)\s*\{\n"
    r"(?:    [^\n]*\n)+?\}\n", re.M)


CATALOGUED = re.compile(
    r"/\*\n(?:(?!.*\*/)[^\n]*\n)*?// ORIGINAL: (?P<address>0x[0-9A-F]+) "
    r"(?P<name>\?\w+@@Y\S*)[^\n]*\n"
    r"(?:(?!.*\*/)[^\n]*\n)*?// size\s+(?P<size>\d+) bytes\n"
    r"(?:(?!.*\*/)[^\n]*\n)*?\*/\n"
    r"(?P<sig>[\w:*&<>,\s]+?\b(?P<ident>\w+)\s*\([^;{)]*\))\s*\{\n"
    r"(?P<body>(?:    [^\n]*\n)+?)\}\n")


def measured(apply: bool, limit: int) -> int:
    """Move small CATALOGUED free helpers to their headers as `MEASURED inline`.

    Different from `main` above, which moves helpers the image has no body for
    at all. These DO have a body - and the image also writes them out at some
    call sites: `osmx calls 0x00532B70` reports rebuild_base_bits making no
    calls while its source calls `bit_set` and `owner_set` twice each.

    Both at once is what `MEASURED inline` gives: in the header so callers can
    inline it, `__declspec(dllexport)` so the standalone body is still emitted
    and its own claim still measures.

    SAFE FOR THE CALLERS THAT REALLY CALL, because the flag search is per
    function and includes `/Ob0`: a caller the image does not inline into
    scores its best under inline expansion off. That is why this can be done in
    bulk instead of one caller at a time.
    """
    files = {path: path.read_text()
             for path in sorted(SRC.glob("*.[ch]*"))
             if path.suffix in (".c", ".h", ".cpp", ".hpp")}
    moved = 0
    pending: dict[Path, list] = {}
    for path in [p for p in files if p.suffix == ".cpp"]:
        header = path.with_suffix(".h")
        if header not in files:
            continue
        for match in list(CATALOGUED.finditer(files[path])):
            if int(match.group("size")) > limit:
                continue
            sig, ident = match.group("sig").strip(), match.group("ident")
            declaration = re.compile(rf"^{re.escape(sig)}\s*;[^\n]*\n", re.M)
            if not declaration.search(files[header]):
                print(f"  - {ident}: {header.name} does not declare it as "
                      f"`{sig};`")
                continue
            scope = visible(header, files)
            unknown = sorted(w for w in _names(match.group("body"))
                             if w not in scope)
            if unknown:
                print(f"  - {ident}: {header.name} cannot see "
                      f"{', '.join(unknown[:3])}")
                continue
            block = match.group(0)
            keep = block[:block.index("*/\n") + 3]
            inline = keep + (f"MEASURED inline {sig} {{\n"
                             f"{match.group('body')}}}\n")
            if re.search(rf"^(?:MEASURED )?inline [^\n]*\b{ident}\s*\(",
                         files[header], re.M):
                print(f"  - {ident}: {header.name} already defines it inline")
                continue
            files[header] = declaration.sub("", files[header])
            pending.setdefault(header, []).append((ident, inline))
            files[path] = files[path].replace(
                block,
                keep + f"// BODY IN {header.name}, as `MEASURED inline`: the "
                       f"image writes it out at\n// some call sites and calls "
                       f"it at others, and a .cpp definition is only ever\n"
                       f"// one of those. The marker stays here because that "
                       f"is where the catalogue\n// reads it.\n\n")
            print(f"  + {ident} ({match.group('address')}, "
                  f"{match.group('size')}B) -> {header.name}")
            moved += 1

    # IN DEPENDENCY ORDER, AND THAT INCLUDES THE BLOCKS ALREADY THERE. These
    # helpers call each other - `cursor_dist` reads `x_dist` - and an earlier
    # run's block sits above this run's, so sorting only what is new leaves
    # half of them above their own callees. Every inline block is lifted out
    # of the header and re-appended in one ordered region.
    for header, blocks in pending.items():
        for existing in list(EXISTING.finditer(files[header])):
            blocks.append((existing.group("ident"), existing.group(0)))
            files[header] = files[header].replace(existing.group(0), "")
        # KEYED BY POSITION, NOT BY NAME. `swap` has an `int *` overload and a
        # `uint8_t *` one; keying the visit set by identifier placed the first
        # and silently dropped the second, taking its marker with it.
        ordered, placed = [], set()

        def emit(index: int, depth: int = 0) -> None:
            if index in placed or depth > 12:
                return
            placed.add(index)
            ident, inline = blocks[index]
            for other in range(len(blocks)):
                if other == index:
                    continue
                if re.search(rf"(?<![\w:])\b{blocks[other][0]}\s*\(",
                             inline):
                    emit(other, depth + 1)
            ordered.append(inline)

        for index in range(len(blocks)):
            emit(index)
        for inline in ordered:
            files[header] = _append(files[header], inline)
    if apply:
        for path, body in files.items():
            if body != path.read_text():
                path.write_text(body)
    return moved


if __name__ == "__main__":
    if "--measured" in sys.argv:
        cap = 96
        for arg in sys.argv:
            if arg.startswith("--limit="):
                cap = int(arg.split("=")[1])
        count = measured("--apply" in sys.argv, cap)
        print(f"{count} catalogued helper(s) "
              f"{'moved' if '--apply' in sys.argv else 'movable'}")
        raise SystemExit(0)
    count = main("--apply" in sys.argv)
    print(f"{count} helper(s) {'moved' if '--apply' in sys.argv else 'movable'}")
