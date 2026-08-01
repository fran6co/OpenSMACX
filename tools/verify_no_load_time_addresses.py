#!/usr/bin/env python3
"""Refuse a fixed game address dereferenced at load time.

`src/graphicwin.cpp` held, at file scope:

    func_x *GraphicWinInvalidateRect =
        *reinterpret_cast<func_x **>(0x00669304);

0x00669304 is the game's USER32!InvalidateRect import slot. It is mapped inside
the game and NOT inside the host test executables, where it sits past the end
of the image - so whether the read faulted depended entirely on SizeOfImage.
Measured 2026-08-01: it survived at 0x24F000 and killed the process BEFORE
main() at 0x266000, with

    page fault on read access to 0x00669304 in 32-bit code (0x00504580)

The trigger was adding tests. Any commit that grew either test binary would
have inherited a pre-main crash with no connection to its own change, and the
bisect would land on an innocent recovery.

A file-scope initialiser runs before main(); the same expression inside a
function body runs only when that path is reached, and every such path in a
test is one the fixture chose to drive. So the rule is not "no fixed
addresses" - the codebase is built on them - it is "not dereferenced before
main()". Resolve on first use instead.

A sweep on 2026-08-01 found zero occurrences after the one above was fixed.
Nothing was keeping it at zero.
"""
import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SRC = REPO_ROOT / "src"

# A statement whose initialiser dereferences a literal game address. Both
# spellings the codebase uses:
#     T *p = *reinterpret_cast<T **>(0x00669304);
#     T *p = *(T **)0x00669304;
# `*` before the cast is what makes it a LOAD, and a load is what faults.
#
# THIS USED TO REQUIRE `0x00` FOLLOWED BY EXACTLY SIX DIGITS, AT COLUMN ZERO,
# and so reported clean over three of the four ways the tree already writes it:
# `0x669304` without the padding, and anything indented - which every file-scope
# declaration inside a `namespace {` block is. It also globbed *.cpp only, so
# the headers were never read at all. It was published as keeping the count at
# zero while covering about a quarter of the space.
#
# Two things replace the two accidental constraints, and neither is a looser
# regex:
#
#   - the literal is captured and compared against the PE ImageBase, because
#     "six hex digits" was standing in for "is a game address" and did it badly.
#     Loosening the digit count alone immediately produces a false positive on
#     src/console.cpp:330, where `+ 0x1DD70` is a struct field offset;
#   - indentation is allowed, but only at namespace scope, which is computed
#     rather than guessed from the column. A function body that resolves an
#     address on first use is exactly what this tool's own failure message tells
#     people to do, so flagging it would make the check forbid its own remedy.
GAME_ADDRESS_FLOOR = 0x00400000  # the original's PE ImageBase

PATTERNS = (
    re.compile(r"^[ \t]*[A-Za-z_][^;{}\n]*=\s*\*\s*reinterpret_cast\s*<[^;]*?"
               r"(0x0*[0-9A-Fa-f]{5,8})[^;]*;", re.MULTILINE | re.DOTALL),
    re.compile(r"^[ \t]*[A-Za-z_][^;{}\n]*=\s*\*\s*\([^)]*\*\s*\)\s*"
               r"(0x0*[0-9A-Fa-f]{5,8})[^;]*;", re.MULTILINE | re.DOTALL),
)

SOURCE_GLOBS = ("*.cpp", "*.h")

# `namespace {`, `namespace foo {` and `extern "C" {` do not introduce a scope
# that delays initialisation; every other brace does.
TRANSPARENT_BRACE = re.compile(r'\b(?:namespace\b|extern\s*"C")[^;{}]*$')


def blank_comments_and_strings(text):
    """Same text, same offsets, with comment and literal contents spaced out.

    Brace counting has to ignore a `{` inside a string or a comment, and this
    file is full of both - the disassembly excerpts in the comments alone
    contain unbalanced braces.
    """
    out = list(text)
    index, size = 0, len(text)

    def blank(start, end):
        for position in range(start, min(end, size)):
            if out[position] != "\n":
                out[position] = " "

    while index < size:
        pair = text[index:index + 2]
        if pair == "//":
            end = text.find("\n", index)
            end = size if end < 0 else end
            blank(index, end)
            index = end
        elif pair == "/*":
            end = text.find("*/", index + 2)
            end = size if end < 0 else end + 2
            blank(index, end)
            index = end
        elif text[index] in "\"'":
            quote, end = text[index], index + 1
            while end < size and text[end] != "\n":
                if text[end] == "\\":
                    end += 2
                    continue
                if text[end] == quote:
                    end += 1
                    break
                end += 1
            blank(index, end)
            index = end
        else:
            index += 1
    return "".join(out)


def namespace_scope_regions(text):
    """Offset ranges that are at namespace scope - not inside any function."""
    clean = blank_comments_and_strings(text)
    regions, opened, depth, start = [], [], 0, 0
    for match in re.finditer(r"[{}]", clean):
        position = match.start()
        if clean[position] == "{":
            preceding = clean[max(0, position - 240):position]
            transparent = bool(TRANSPARENT_BRACE.search(preceding))
            opened.append(transparent)
            if not transparent:
                if depth == 0:
                    regions.append((start, position))
                depth += 1
        elif opened:
            if not opened.pop():
                depth = max(0, depth - 1)
                if depth == 0:
                    start = position
    if depth == 0:
        regions.append((start, len(clean)))
    return regions


def offenders(source_dir):
    found = []
    paths = sorted(path for glob in SOURCE_GLOBS
                   for path in Path(source_dir).glob(glob))
    for path in paths:
        text = path.read_text(encoding="utf-8", errors="replace")
        regions = namespace_scope_regions(text)
        for pattern in PATTERNS:
            for match in pattern.finditer(text):
                if int(match.group(1), 16) < GAME_ADDRESS_FLOOR:
                    continue
                if not any(begin <= match.start() < end
                           for begin, end in regions):
                    continue  # inside a function, which is the prescribed fix
                line = text[:match.start()].count("\n") + 1
                found.append((path, line, " ".join(match.group(0).split())[:96]))
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--src", type=Path, default=DEFAULT_SRC)
    arguments = parser.parse_args()

    if not arguments.src.is_dir():
        print(f"load-time-addresses: {arguments.src} is absent, so this check "
              "verified NOTHING", file=sys.stderr)
        return 1
    sources = [path for glob in SOURCE_GLOBS
               for path in arguments.src.glob(glob)]
    if not sources:
        print(f"load-time-addresses: no {' or '.join(SOURCE_GLOBS)} under "
              f"{arguments.src}, so this check verified NOTHING",
              file=sys.stderr)
        return 1

    found = offenders(arguments.src)
    if found:
        print("load-time-addresses: a fixed game address is dereferenced at "
              "load time. In the host test\nbinaries that address is outside "
              "the image, so this faults before main() once the\nbinary grows "
              "past it - and the crash is blamed on whichever commit grew it.\n"
              "Resolve it on first use inside the function instead.",
              file=sys.stderr)
        for path, line, text in found:
            print(f"    {path}:{line}: {text}", file=sys.stderr)
        return 1

    print(f"load-time-addresses: {len(sources)} sources, no fixed address "
          "dereferenced before main()")
    return 0


if __name__ == "__main__":
    sys.exit(main())
