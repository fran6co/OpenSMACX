#!/usr/bin/env python3
"""Every CMakeLists.txt the build reads, found rather than named.

A leaf module on purpose - it imports only the standard library, so any check
can use it without a cycle.

WHY IT EXISTS. Three checks read "the CMakeLists" to learn what the build
registers, and each spelled that `REPO_ROOT / "CMakeLists.txt"`. That was true
for as long as there was one file. On 2026-08-15 everything that is not the
executable moved to `tests/CMakeLists.txt` behind `OPENSMACX_BUILD_TOOLING`,
which is where every `add_test` went with it, and all three checks began
reading a file with no registrations in it:

  * `verify_tool_test_registration` refused, correctly - its floor caught the
    empty parse and said the file's shape had changed.
  * `verify_checks_can_fail.gate_checks` refused the same way, and
  * `verify_check_tests_observe` CAUGHT that refusal and fell back to matching
    tool names by prefix - so it kept printing a sweep, over a population it
    had stopped deriving.

The first two failed loudly and the third degraded quietly, which is the
distribution you would expect and the reason the fix is a discovery rather
than three corrected constants. A `add_subdirectory` that moves registrations
again changes nothing here.

WHAT IS EXCLUDED, and why by name. Build directories hold CMake's own
generated CMakeLists.txt copies - `build/_deps/...` and the CMakeFiles
scratch - and a check that parsed those would count registrations twice and
report checks that no source file asks for.
"""

from __future__ import annotations

from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# Directory names never descended into. `build` covers `build/`, `build-lean/`
# and anything else prefixed that way; the rest are the tool directories that
# vendor their own trees.
SKIP_PREFIXES = ("build", ".", "__pycache__", "node_modules")

# A FLOOR, like every other scan in this directory has. The split made two
# files where there was one, and the shape this module exists to prevent is a
# discovery that quietly finds none - so finding none, or losing the root one,
# is an error rather than an empty list a caller would treat as "nothing
# registered anywhere".
MINIMUM_FILES = 1


def _skipped(path: Path, root: Path) -> bool:
    return any(part.startswith(SKIP_PREFIXES) for part in
               path.relative_to(root).parts[:-1])


def cmake_files(root: Path = None) -> list:
    """Every CMakeLists.txt under `root`, root-first then sorted."""
    root = root or REPO_ROOT
    found = [path for path in root.rglob("CMakeLists.txt")
             if path.is_file() and not _skipped(path, root)]
    top = root / "CMakeLists.txt"
    rest = sorted(path for path in found if path != top)
    ordered = ([top] if top in found else []) + rest
    if len(ordered) < MINIMUM_FILES:
        raise SystemExit(
            f"cmake_sources: found {len(ordered)} CMakeLists.txt under {root}, "
            f"below the floor of {MINIMUM_FILES}. Nothing downstream of this "
            f"can report a population it did not read.")
    return ordered


def cmake_text(root: Path = None) -> str:
    """Every CMakeLists.txt under `root`, concatenated for one parse.

    Concatenation rather than a parse per file because every caller is asking
    a question about the BUILD - "what does it register", "what does it run" -
    and which file a registration is written in is not part of the answer. The
    separator carries the path so a failure can still say where it came from.
    """
    root = root or REPO_ROOT
    parts = []
    for path in cmake_files(root):
        parts.append(f"# ---- {path.relative_to(root)}\n")
        parts.append(path.read_text(errors="replace"))
    return "\n".join(parts)


def main() -> int:
    for path in cmake_files():
        print(path.relative_to(REPO_ROOT))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
