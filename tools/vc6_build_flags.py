"""The flags CMake really compiles a source with, read from the build itself.

WHY THIS EXISTS. `decomp_status.py` measures a body in its own translation
unit whenever the scaffold cannot build one, and that only works if the file
compiles - which means giving it the include path and forced includes the
build gives it. The approximation it had was `byte_match.seed_context`:
copy the file's own directory beside it. That resolves `#include "win.h"`
and nothing else, so `src/basewin.cpp` came back
`C1083: Cannot open include file: 'cstdint'` - supplied by
`cmake/vc6-include`, which CMakeLists.txt adds and nothing else knew about.

WHY NOT COPY THE TWO FLAGS INTO THE TOOL. Because that is a second copy of
CMakeLists.txt's decisions, and a copy goes stale the first time an include
directory or a define is added - silently, since the only symptom is a
NO_COMPILE that looks like a bad body. CMake already writes the answer down;
this reads it.

AND CMAKE IS RE-RUN FIRST, for the same reason. A `compile_commands.json`
older than the CMakeLists.txt that generated it describes a build that no
longer exists, and every consumer of it would be measuring against flags
nobody uses. Staleness is checked against the files that decide the flags,
and a reconfigure is 0.1 s.

WHAT IS TAKEN AND WHAT IS NOT. Includes, defines and forced includes: the
flags that decide whether the file PARSES. Not the optimisation flags, not
the runtime library, not `/Z7` - the verdict is best-of-four over
`byte_match.FLAG_SETS` and the build is a debug build, so taking its `/Od`
would measure something the shipped image is not. Not the PCH flags either:
`/Yu` and `/Fp` point at a precompiled header built for a different object
directory, and the `/FI` that goes with it is the PCH's own.
"""
from __future__ import annotations

import functools
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = Path(os.environ.get("OPENSMACX_BUILD_DIR",
                                str(REPO_ROOT / "build")))
COMPILE_COMMANDS = BUILD_DIR / "compile_commands.json"

# The files that decide what a compile command contains. A change to any of
# them makes an existing compile_commands.json a description of the past.
FLAG_SOURCES = (
    REPO_ROOT / "CMakeLists.txt",
    REPO_ROOT / "cmake" / "toolchains" / "vc6.cmake",
    REPO_ROOT / "cmake" / "vc6-flag-overrides.cmake",
)

# Kept: what decides whether the file parses.
KEEP_PREFIXES = ("-I", "/I", "-D", "/D")
# Dropped even though they start with a kept prefix or look harmless.
PCH_MARKERS = ("cmake_pch",)


def _stale() -> bool:
    if not COMPILE_COMMANDS.is_file():
        return True
    written = COMPILE_COMMANDS.stat().st_mtime
    return any(path.is_file() and path.stat().st_mtime > written
               for path in FLAG_SOURCES)


@functools.lru_cache(maxsize=1)
def refresh() -> str:
    """Re-run CMake if the flags on disk are older than the files that set
    them. Returns "" on success, or the reason it could not.

    Once per process, and only when something actually asks for flags - a
    run that never reaches the translation-unit fallback pays nothing.
    """
    if not _stale():
        return ""
    if not FLAG_SOURCES[0].is_file():
        return f"no {FLAG_SOURCES[0]}"
    if shutil.which("cmake") is None:
        return "cmake is not on PATH"
    done = subprocess.run(
        ["cmake", "-S", str(REPO_ROOT), "-B", str(BUILD_DIR)],
        capture_output=True, text=True)
    if done.returncode != 0:
        return (f"cmake -S . -B {BUILD_DIR.name} failed:\n"
                f"{done.stdout}{done.stderr}".strip())
    return ""


@functools.lru_cache(maxsize=1)
def _table() -> dict:
    """{resolved source path: [flag, ...]}, empty when there is no build."""
    reason = refresh()
    if reason:
        print(f"vc6_build_flags: {reason}; falling back to the file's own "
              f"directory alone", file=sys.stderr)
    if not COMPILE_COMMANDS.is_file():
        return {}
    try:
        entries = json.loads(COMPILE_COMMANDS.read_text())
    except (OSError, ValueError) as error:
        print(f"vc6_build_flags: {COMPILE_COMMANDS} unreadable: {error}",
              file=sys.stderr)
        return {}
    table = {}
    for entry in entries:
        source = entry.get("file")
        command = entry.get("command")
        if not source or not command:
            continue
        table[str(Path(source).resolve())] = _wanted(command.split())
    return table


def _wanted(words: list) -> list:
    """The parse-deciding flags out of one command line, ready for CL.EXE.

    TWO TRANSLATIONS, AND BOTH ARE THE BUILD'S, NOT THIS FILE'S. CMake writes
    include directories as `-I` and definitions as `/D` in the same command,
    because it is generating for a compiler it has been told is MSVC through
    a POSIX wrapper; VC6 takes the slash form. And the paths are POSIX, which
    `CL.EXE` under Wine reads as options - `D2004: '/FI' requires an
    argument`, which is the same failure `tools/vc6_cl.py` was given
    `PATH_PREFIXES` to prevent. That translator is reused rather than
    restated: the build's compiles go through it, so these compile the same
    way by construction.
    """
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import vc6_cl

    kept = []
    for word in words:
        if any(marker in word for marker in PCH_MARKERS):
            continue
        if word.startswith(("-I", "-D", "-FI")):
            word = "/" + word[1:]
        if word.startswith(("/I", "/D")) and len(word) > 2 or \
                word.startswith("/FI") and len(word) > 3:
            kept.append(vc6_cl.translate(word))
    return kept


def flags_for(origin) -> list:
    """The extra flags for compiling `origin`, or [] if the build has none.

    [] covers everything that is not a build input - every scaffolded unit,
    every file under `src/unrecovered/` and `src/recovered/units/` - which is
    most of what this tree compiles, and those are self-contained by design.
    """
    if origin is None:
        return []
    return _table().get(str(Path(origin).resolve()), [])


def signature(origin) -> str:
    """A hashable stand-in for `flags_for`, for grouping units into batches."""
    return "\x1f".join(flags_for(origin))


def main() -> int:
    reason = refresh()
    if reason:
        print(reason, file=sys.stderr)
        return 1
    table = _table()
    print(f"{len(table)} source(s) in {COMPILE_COMMANDS}")
    for source in sorted(table)[:5]:
        print(f"  {Path(source).name}: {' '.join(table[source])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
