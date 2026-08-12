#!/usr/bin/env python3
"""Refuse a `${CMAKE_CURRENT_SOURCE_DIR}/...` path that is not in the tree.

A retirement that deletes a tool leaves its CMake references behind, and CMake
does not complain: `add_custom_target` resolves nothing at configure time, and a
target outside `all` is never built, so a `COMMAND` naming a script that does
not exist configures clean and passes every test run. It fails only when
somebody finally invokes that target, which for a retired route is never.

That is how this tree ended up with twenty-one live `COMMAND` and `DEPENDS`
lines pointing at paths like `prepare_hybrid_image (retired)` - a placeholder
spelling produced by rewriting tool names in place instead of removing the
blocks - alongside `DEPENDS` on `docs/recovery/functions.csv`, deleted when the
map moved into `src/`. Both survived a full configure and a 173-step build.

WHAT THIS CHECKS. Every `${CMAKE_CURRENT_SOURCE_DIR}/<path>` in CMakeLists.txt
names a file or directory that exists. That is the whole rule, and it is
mechanical rather than a list somebody maintains: a reference is either
resolvable or it is not.

WHAT IT DELIBERATELY DOES NOT CHECK. Paths built from variables
(`${OPENSMACX_LEGACY_EXE}`) are not resolved. They are configure-time values,
frequently point into ignored local artifact directories that a clean checkout
genuinely lacks, and guessing at them would make this check refuse a healthy
tree. The literal-path case is the one that caught real defects.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CMAKELISTS = REPO_ROOT / "CMakeLists.txt"

# Only literal paths: a `${...}` anywhere in the tail means it is computed.
REFERENCE_RE = re.compile(r'"\$\{CMAKE_CURRENT_SOURCE_DIR\}/([^"$]+)"')

# A comment line documents a retired route rather than invoking it, and the
# names in it are prose. Stripping them is what keeps the check from demanding
# that history be deleted along with the code.
COMMENT_RE = re.compile(r"(?<!\\)#.*$")


def references(text: str) -> list:
    """[(line number, path)] for every literal source-dir path outside a
    comment."""
    found = []
    for number, line in enumerate(text.splitlines(), start=1):
        for match in REFERENCE_RE.finditer(COMMENT_RE.sub("", line)):
            found.append((number, match.group(1)))
    return found


def missing(root: Path, text: str) -> list:
    """[(line, path)] for the references that do not resolve."""
    return [(number, path) for number, path in references(text)
            if not (root / path).exists()]


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--cmakelists", type=Path, default=CMAKELISTS)
    parser.add_argument("--root", type=Path, default=None)
    parser.add_argument("--check", action="store_true",
                        help="exit non-zero if any reference is unresolvable")
    args = parser.parse_args(argv)

    root = args.root or args.cmakelists.resolve().parent
    text = args.cmakelists.read_text(errors="replace")
    total = references(text)
    gone = missing(root, text)

    for number, path in gone:
        print(f"    {args.cmakelists.name}:{number}: {path} does not exist",
              file=sys.stderr)
    if gone:
        print(f"{len(gone)} of {len(total)} CMake source path(s) name nothing "
              f"in the tree", file=sys.stderr)
        return 1 if args.check else 1

    print(f"cmake-paths: {len(total)} literal source path(s), all resolve")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
