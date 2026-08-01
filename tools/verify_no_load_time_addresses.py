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

# A file-scope statement (column 0) whose initialiser dereferences a literal
# game address. Both spellings the codebase uses:
#     T *p = *reinterpret_cast<T **>(0x00669304);
#     T *p = *(T **)0x00669304;
# `*` before the cast is what makes it a LOAD, and a load is what faults.
PATTERNS = (
    re.compile(r"^[A-Za-z_][^;{}\n]*=\s*\*\s*reinterpret_cast\s*<[^;]*?"
               r"0x00[0-9A-Fa-f]{6}[^;]*;", re.MULTILINE | re.DOTALL),
    re.compile(r"^[A-Za-z_][^;{}\n]*=\s*\*\s*\([^)]*\*\s*\)\s*"
               r"0x00[0-9A-Fa-f]{6}[^;]*;", re.MULTILINE | re.DOTALL),
)


def offenders(source_dir):
    found = []
    for path in sorted(Path(source_dir).glob("*.cpp")):
        text = path.read_text(encoding="utf-8", errors="replace")
        for pattern in PATTERNS:
            for match in pattern.finditer(text):
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
    sources = list(arguments.src.glob("*.cpp"))
    if not sources:
        print(f"load-time-addresses: no .cpp under {arguments.src}, so this "
              "check verified NOTHING", file=sys.stderr)
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
