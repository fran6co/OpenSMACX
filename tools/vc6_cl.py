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
import re
import subprocess
import sys
import tempfile
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


def response_tokens(text: str) -> list:
    """The arguments inside a response file, quotes honoured and removed."""
    out, current, quoted = [], [], False
    for character in text:
        if character == '"':
            quoted = not quoted
            continue
        if not quoted and character in " \t\r\n":
            if current:
                out.append("".join(current))
                current = []
            continue
        current.append(character)
    if current:
        out.append("".join(current))
    return out


def expand_responses(arguments: list, depth: int = 0) -> list:
    """Replace every `@file` with the arguments inside it.

    THE WRAPPER ONLY EVER SAW ARGV, and a generator that passes its flags in a
    response file therefore defeated everything this file does. Two failures,
    one cause:

      * `/FI<path>` reached cl untranslated, cl read the leading `/` of the
        POSIX path as the start of another option, and the build stopped on
        `D2004: '/FI' requires an argument` - the exact error PATH_PREFIXES was
        written to prevent, reappearing because the translation never got to
        look at the flag.
      * `--depfile=` and `--deptarget=` were never taken out, so no depfile was
        written and none of them reached the code that writes one. That is the
        silent half: ninja records `#deps 0`, editing a header rebuilds
        nothing, and the build reports success over stale objects.

    Neither showed up under `Unix Makefiles`, which puts flags on the command
    line. Ninja uses response files, so the generator choice decided whether
    the compiler wrapper worked - and only one of the two failures is loud.

    Recursive, because a response file may name another; bounded, because a
    file naming itself would otherwise hang the build.
    """
    if depth > 8:
        return list(arguments)
    out = []
    for argument in arguments:
        if argument.startswith("@"):
            path = Path(argument[1:])
            if path.is_file():
                out.extend(expand_responses(
                    response_tokens(path.read_text(errors="replace")),
                    depth + 1))
                continue
        out.append(argument)
    return out


# Windows caps a command line near 32k and wine enforces it, so a link line
# naming every object can still need a response file - just one this wrapper
# writes itself, after translating, rather than one it cannot read.
COMMAND_LINE_LIMIT = 30000


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


LINE_DIRECTIVE = re.compile(r'^#line\s+\d+\s+"(.*)"', re.M)
# Flags that make no sense for, or actively defeat, a preprocess-only pass.
# `/Yu` is the important one: with the precompiled header in use cl does not
# re-read the headers it covers, so the dependency list would omit exactly the
# headers most likely to change.
DEP_PASS_DROP = ("/Yu", "/Fp", "/Fo", "/Fd", "/c")


def posix_path(text: str) -> str:
    """`Z:\\home\\x\\y.h` back to `/home/x/y.h`.

    The inverse of `windows_path`, and needed only for depfiles: cl reports
    the headers it opened in the drive-letter form wine handed it, and ninja
    compares those strings against the POSIX paths in build.ninja. A path in
    the wrong alphabet is not a mismatch it reports - it is a dependency it
    silently does not have.
    """
    if len(text) > 2 and text[1] == ":":
        text = text[2:]
    return text.replace("\\", "/")


def take_depfile(arguments: list) -> tuple:
    """Pull `--deptarget=` and `--depfile=` out of the argument list.

    CMake's Ninja generator writes `deps = gcc` and `depfile = $DEP_FILE` for
    this compiler, then passes whatever CMAKE_DEPFILE_FLAGS_CXX holds. It held
    nothing, so no depfile was ever written and ninja recorded `#deps 0` for
    every object - meaning a header could be edited and NOTHING rebuilt. Three
    measurements in one session were taken against stale objects before that
    surfaced. cl cannot write a GNU depfile, so the wrapper does it.

    THE FLAG NAMES ARE NOT THE GNU ONES ON PURPOSE. The first attempt used
    `-MD -MT -MF`, and `-MD` is MSVC's multithreaded-DLL RUNTIME switch, which
    CMake passes on every compile: consuming it here stripped the runtime
    model out of the command silently, swapping the CRT the DLL links. `-MT`
    is the static-runtime switch and would collide the same way. These two
    spellings belong to this wrapper and to nothing else.
    """
    kept, target, depfile = [], None, None
    for argument in arguments:
        if argument.startswith("--deptarget="):
            target = argument.split("=", 1)[1]
        elif argument.startswith("--depfile="):
            depfile = argument.split("=", 1)[1]
        else:
            kept.append(argument)
    return kept, target, depfile


def write_depfile(executable, arguments: list, path: str, target: str) -> None:
    """One GNU depfile, from a preprocess-only pass.

    NOT `/showIncludes`: cl 12.00.8168 answers that with
    `D4002: ignoring unknown option` - it arrived in VS2003 - and the silent
    result is an empty dependency list, which is indistinguishable from a file
    that genuinely includes nothing. `/E` is what this compiler has: it emits
    a `#line N "file"` directive for every file it opens, so the set of names
    in those directives IS the dependency set.

    The cost is one extra preprocessor pass per translation unit. On this tree
    that is a few seconds against an eleven-second build, and it buys an
    incremental build that is correct rather than one that quietly compiles
    nothing after a header changes.

    A failure here is deliberately silent: dependency information is an
    optimisation, and refusing to produce an object because the DEPENDENCY
    pass failed would turn a missing nicety into a broken build.
    """
    kept = [a for a in arguments if not a.startswith(DEP_PASS_DROP)]
    finished = subprocess.run(
        ["wine32", str(executable), "/nologo", "/E"] + [translate(a) for a in kept],
        env=environment(), capture_output=True, text=True, errors="replace")
    seen, ordered = set(), []
    for name in LINE_DIRECTIVE.findall(finished.stdout):
        # cl escapes the separators inside the directive, so `Z:\\a\\b.h`.
        resolved = posix_path(name.replace("\\\\", "\\"))
        if resolved not in seen:
            seen.add(resolved)
            ordered.append(resolved)
    body = "".join(f" \\\n  {name}" for name in ordered)
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    Path(path).write_text(f"{target}:{body}\n")


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

    # BEFORE ANYTHING READS THE ARGUMENTS. Both the depfile extraction below
    # and the path translation are blind to whatever sits inside a `@file`.
    arguments = expand_responses(arguments)
    arguments, dep_target, dep_file = take_depfile(arguments)
    if dep_file:
        # Before the real compile, and unconditionally: a depfile is what
        # tells ninja to try again once the header is fixed, so it has to
        # exist even when the compile that follows fails.
        write_depfile(executable, arguments, dep_file, dep_target or "")
    translated = [translate(a) for a in arguments]
    scratch = None
    if sum(len(a) + 1 for a in translated) > COMMAND_LINE_LIMIT:
        scratch = tempfile.NamedTemporaryFile(
            "w", suffix=".rsp", delete=False, encoding="utf-8")
        # Quoted per line: a translated path holds backslashes and may hold
        # spaces, and cl splits an unquoted response file on whitespace.
        scratch.write("\n".join(f'"{a}"' for a in translated) + "\n")
        scratch.close()
        translated = ["@" + windows_path(scratch.name)]
    command = ["wine32", str(executable)] + translated
    finished = subprocess.run(command, env=environment(), capture_output=True,
                              text=True, errors="replace")
    if scratch is not None:
        os.unlink(scratch.name)
    # CL writes the source file name to stdout before any diagnostic. It is
    # noise in a build log and CMake does not need it, but a diagnostic that
    # follows it does, so only the bare echo line is dropped. The
    # `/showIncludes` notes go the same way - they are the depfile's input,
    # not a build log's.
    for line in finished.stdout.splitlines():
        if line.strip() and not line.strip().lower().endswith((".cpp", ".c", ".cxx")):
            print(line)
    if finished.stderr.strip():
        print(finished.stderr, file=sys.stderr, end="")
    return finished.returncode


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
