#!/usr/bin/env python3
"""Make VC6's CL.EXE usable as a CMake compiler from Linux.

CMake drives a compiler with POSIX paths, and MSVC reads a leading `/` as the
start of an OPTION - `/home/fran6co/...` parses as `/h`, `/o`, `/m`, and the
compile fails in a way that looks nothing like a path problem. Wine already
maps the filesystem at `Z:`, so the whole translation is: absolute POSIX path
-> `Z:\\...` with backslashes, everything else through untouched.

The same wrapper serves LINK.EXE and LIB.EXE, chosen by argv[0] or --tool,
because they need the identical treatment and the identical environment.

Why a wrapper rather than a native build: this project is a matching
decompilation, so the ONE compiler whose output is meaningful is the one that
built the original image - cl 12.00.8168. Every other toolchain can only tell
you whether the code is valid, never whether it is right.
"""

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

VC6_ROOT = Path(os.environ.get("VC6_ROOT", Path.home() / "opt" / "vc6"))
VC6_PREFIX = Path(os.environ.get("VC6_PREFIX", VC6_ROOT / ".wineprefix"))
TOOLS = {"cl": "CL.EXE", "link": "LINK.EXE", "lib": "LIB.EXE"}


def windows_path(text: str) -> str:
    """`/home/x/y.cpp` -> `Z:\\home\\x\\y.cpp`; anything else unchanged.

    Only ABSOLUTE paths are translated. A relative path already works - wine
    resolves it against the working directory - and rewriting one would break
    the response files CMake writes with paths relative to the build tree.
    """
    if not text.startswith("/"):
        return text
    # `/nologo`, `/O2`, `/D_WIN32` also start with a slash. Telling an option
    # from a path by SHAPE cannot be done - `/c` and a file called `/c` look
    # the same - so the filesystem is asked instead: it is a path when it
    # exists, or when the directory it would be created in exists (an output
    # file that is not there yet).
    if Path(text).exists() or Path(text).parent.is_dir() and "/" in text[1:]:
        return "Z:" + text.replace("/", "\\")
    return text


# Options CMake glues an absolute path straight onto. `/FI`, `/Yc` and `/Yu`
# are the precompiled-header trio; without them the path went through
# untranslated, VC6 read its leading `/` as the start of another option, and
# the build stopped on `D2004: '/FI' requires an argument` at the very first
# object.
PATH_PREFIXES = ("/Fo", "/Fe", "/Fd", "/Fp", "/Fa", "/FI", "/Yc", "/Yu",
                 "/I", "/OUT:", "/out:", "/IMPLIB:", "/PDB:", "/LIBPATH:")


def translate(argument: str) -> str:
    """Translate a bare path, or the path inside `/Fo<path>` and friends."""
    # LONGEST FIRST. `/I` is a prefix of `/IMPLIB:`, so in source order it
    # matched first and handed `MPLIB:/tmp/...` to the translator, which sees
    # no leading slash and passes it through - the import library path reached
    # LINK untranslated every time.
    for prefix in sorted(PATH_PREFIXES, key=len, reverse=True):
        if argument.startswith(prefix):
            return prefix + windows_path(argument[len(prefix):])
    if argument.startswith("@"):
        return "@" + windows_path(argument[1:])
    return windows_path(argument)


def environment() -> dict:
    include = os.environ.get("VC6_INCLUDE", str(VC6_ROOT / "INCLUDE"))
    library = os.environ.get("VC6_LIB", str(VC6_ROOT / "LIB"))
    return dict(
        os.environ,
        WINEPREFIX=str(VC6_PREFIX),
        WINEDEBUG=os.environ.get("WINEDEBUG", "-all"),
        INCLUDE=";".join("Z:" + part.replace("/", "\\")
                         for part in include.split(":") if part),
        LIB=";".join("Z:" + part.replace("/", "\\")
                     for part in library.split(":") if part),
    )


def main(argv: list) -> int:
    tool = "cl"
    arguments = argv[1:]
    if arguments and arguments[0].startswith("--tool="):
        tool = arguments[0].split("=", 1)[1]
        arguments = arguments[1:]
    else:
        stem = Path(argv[0]).stem.lower()
        for name in TOOLS:
            if stem.endswith(name):
                tool = name
                break

    executable = VC6_ROOT / "BIN" / TOOLS[tool]
    if not executable.is_file():
        print(f"{executable} not found; set VC6_ROOT", file=sys.stderr)
        return 127

    command = ["wine", str(executable)] + [translate(a) for a in arguments]
    finished = subprocess.run(command, env=environment(), capture_output=True,
                              text=True, errors="replace")
    # CL writes the source file name to stdout before any diagnostic. It is
    # noise in a build log and CMake does not need it, but a diagnostic that
    # follows it does, so only the bare echo line is dropped.
    for line in finished.stdout.splitlines():
        if line.strip() and not line.strip().lower().endswith((".cpp", ".c", ".cxx")):
            print(line)
    if finished.stderr.strip():
        print(finished.stderr, file=sys.stderr, end="")
    return finished.returncode


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
