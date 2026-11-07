#!/usr/bin/env python3
"""Make VC6's CL.EXE usable as a CMake compiler from Linux.

CMake drives a compiler with POSIX paths, and MSVC reads a leading `/` as the
start of an OPTION - `/home/fran6co/...` parses as `/h`, `/o`, `/m`, and the
compile fails in a way that looks nothing like a path problem. Wine already
maps the filesystem at `Z:`, so the whole translation is: absolute POSIX path
-> `Z:\\...` with backslashes, everything else through untouched.

The same wrapper serves LINK.EXE and LIB.EXE, chosen by argv[0] or --tool,
because they need the identical treatment and the identical environment.

Matching needs the configured historical compiler and ABI, not the host's
default compiler. The CL driver banner alone is insufficient: VC6 release and
SP3 can share that driver while using different C1/C1XX/C2 DLLs. The pinned
image contains mixed compiler-build records; see recovery/COMPILER_EVIDENCE.md.
The native toolchain fingerprint includes the actual compiler DLLs and SDK.
This wrapper does not infer a historical compiler assignment or select one
from a function's matching score.
"""

from __future__ import annotations

import os
import json
import re
import signal
import subprocess
import sys
import tempfile
from pathlib import Path

VC6_ROOT = Path(os.environ.get("VC6_ROOT", Path.home() / "opt" / "vc6"))
VC6_PREFIX = Path(os.environ.get("VC6_PREFIX", VC6_ROOT / ".wineprefix"))
TOOLS = {"cl": "CL.EXE", "link": "LINK.EXE", "lib": "LIB.EXE", "ml": "ML.EXE"}
# MASM-dialect sources (user decision, 2026-11-07) go through Microsoft ML
# 6.11d, run under the same Wine prefix as CL. It reproduces the original
# _SELFMOD modules and uV decoder byte for byte, ALIGN fill and section flags
# included; 6.12 and 6.13 fill ALIGN differently. It is a separate install
# beside VC6; ~/opt/masm/<version>/PROVENANCE.md pins it.
ML_ROOT = Path(os.environ.get("ML_ROOT", Path.home() / "opt" / "masm" / "6.11d"))
# The AnimaTek Caviar/VOX library was compiled by Visual C++ 5.0 (CL 11.00.7022),
# not VC6; see recovery/COMPILER_EVIDENCE.md, "Caviar library compiler". Its
# recipes carry the explicit `--compiler=vc5` option, which selects the "cl5"
# tool: VC5's CL.EXE under its own Wine prefix. Only the compiler binaries are
# installed; the SDK/CRT headers stay VC6's. ~/opt/vc5/PROVENANCE.md pins it.
VC5_ROOT = Path(os.environ.get("VC5_ROOT", Path.home() / "opt" / "vc5"))
VC5_PREFIX = Path(os.environ.get("VC5_PREFIX", VC5_ROOT / "wineprefix"))
TOOLS["cl5"] = "CL.EXE"
COMPILER_OPTIONS = {"--compiler=vc5": "cl5"}
ROOT_VARIABLES = {"ml": "ML_ROOT", "cl5": "VC5_ROOT"}


def tool_executable(tool: str) -> Path:
    if tool == "ml":
        return ML_ROOT / "bin" / TOOLS[tool]
    return (VC5_ROOT if tool == "cl5" else VC6_ROOT) / "BIN" / TOOLS[tool]


def take_compiler(tool: str, arguments: list) -> tuple:
    """Remove an explicit `--compiler=` choice and return the selected tool.

    The choice is a recipe option, not an inference: an absent option keeps
    VC6, and it is valid only on a compiler invocation.
    """
    chosen = [word for word in arguments if word.startswith("--compiler=")]
    if not chosen:
        return tool, arguments
    if tool != "cl" or len(chosen) != 1 or chosen[0] not in COMPILER_OPTIONS:
        raise ValueError("unsupported or repeated explicit compiler selection")
    return COMPILER_OPTIONS[chosen[0]], [word for word in arguments if word != chosen[0]]


def preprocessing_diagnostics(stdout: bytes, stderr: bytes) -> str:
    """Bound failed /E output without burying diagnostics in expanded source.

    The compiler client retains the beginning of an error, so real diagnostic
    lines precede any continuation or fallback context. This renders failures
    only: it neither decides success nor changes successful preprocessor bytes.
    """
    diagnostic = re.compile(rb"(?:fatal error|warning|error) [CD][0-9]{4}:")
    lines, context, seen = [], [], set()
    omitted, stderr_context = False, ""
    for stream_number, stream in enumerate((stdout, stderr)):
        continuation = 0
        for match in re.finditer(rb"[^\r\n]+", stream):
            line = match.group()
            if diagnostic.search(line):
                continuation = 2
                if line in seen:
                    continue
                if len(lines) == 16:
                    omitted = True
                    continue
                seen.add(line)
                text = line[:1024].decode(errors="replace")
                lines.append(text + (" [line truncated]" if len(line) > 1024 else ""))
            elif continuation and line.startswith((b" ", b"\t")):
                if len(context) < 2:
                    context.append(line[:256].decode(errors="replace"))
                continuation -= 1
            else:
                continuation = 0
                if stream_number == 1:
                    stderr_context = (stderr_context + "\n" + line[-512:].decode(errors="replace"))[-512:]
    if lines:
        rendered = "\n".join(lines)
        if context:
            rendered += "\nDiagnostic continuation context:\n" + "\n".join(context)
        if stderr_context:
            rendered += "\nOther stderr context:" + stderr_context
        if omitted:
            rendered += "\n[additional diagnostic lines omitted]"
    else:
        rendered = "\n".join(label + stream[-512:].decode(errors="replace")
                             for label, stream in (("stderr tail: ", stderr), ("stdout tail: ", stdout))
                             if stream) or "no compiler output"
    return rendered if len(rendered) <= 4000 else rendered[:3960] + "\n[diagnostic output truncated]"


def run_tool(command: list, *, cwd=None, binary=False, explicit_compiler=False,
             explicit_assembler=False) -> subprocess.CompletedProcess:
    """Wait for the tool, not EOF from background Wine services.

    Wine services can inherit captured pipes and keep them open after CL exits.
    File-backed diagnostics preserve the tool's output and status without
    waiting for those unrelated services or terminating a shared wineserver.
    The caller still owns compilation timeouts and process-group cleanup.
    """
    options = {"mode": "w+b"} if binary else {"mode": "w+", "errors": "replace"}
    child_environment = environment()
    if explicit_compiler:
        child_environment = {key: value for key, value in child_environment.items()
                             if key.upper() not in ("CL", "_CL_")}
        child_environment.update(CL="", _CL_="")
    if explicit_assembler:
        # ML prepends options from its ML variable; the recipe must be argv only.
        child_environment = {key: value for key, value in child_environment.items()
                             if key.upper() != "ML"}
        child_environment.update(ML="")
    with tempfile.TemporaryFile(**options) as stdout, tempfile.TemporaryFile(**options) as stderr:
        finished = subprocess.run(command, env=child_environment, stdout=stdout, stderr=stderr,
                                  **({"cwd": cwd} if cwd is not None else {}))
        stdout.seek(0)
        stderr.seek(0)
        output, errors = stdout.read(), stderr.read()
        if finished.returncode < 0:
            number = -finished.returncode
            try:
                name = signal.Signals(number).name
            except ValueError:
                name = str(number)
            diagnostic = f"Wine tool terminated by signal {name} ({number}); tool execution failed."
            if number == getattr(signal, "SIGSYS", None):
                diagnostic += (
                    " Check sandbox/execution permissions using the approved diagnostic route;"
                    " do not change C++ or bypass a denied permission."
                )
            # Preserve preprocessing bytes and the actual return code. Otherwise
            # SIGSYS can surface only as shell exit 225 with empty diagnostics,
            # indistinguishable to an agent from a broken C++ spelling or recipe.
            newline = b"\n" if binary else "\n"
            suffix = ("\n" if errors and not errors.endswith(newline) else "") + diagnostic + "\n"
            errors += suffix.encode() if binary else suffix
        return subprocess.CompletedProcess(command, finished.returncode, output, errors)


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


def sdk_search_paths(variable: str, default: Path) -> tuple[Path, ...]:
    paths = tuple(Path(part) for part in os.environ.get(variable, str(default)).split(":") if part)
    if any(not path.is_absolute() for path in paths):
        raise ValueError(variable + " requires absolute POSIX directory paths")
    return tuple(path.resolve() for path in paths)


# The tool this process runs; the VC5 compiler keeps its own Wine prefix.
ACTIVE_TOOL = "cl"


def environment() -> dict:
    # LINK can prepend options independently of argv. The captured effective
    # link policy must not omit aliases, forced links or extra input files.
    # Require explicit arguments, including for case-insensitive Windows names;
    # pass empty values so Wine registry defaults cannot add hidden options.
    implicit = [key for key, value in os.environ.items() if key.upper() in {"LINK", "_LINK_"} and value]
    if implicit:
        raise ValueError("implicit linker environment options are unsupported; use explicit arguments: " +
                         ", ".join(sorted(implicit)))
    include = sdk_search_paths("VC6_INCLUDE", VC6_ROOT / "INCLUDE")
    library = sdk_search_paths("VC6_LIB", VC6_ROOT / "LIB")
    return dict(
        {key: value for key, value in os.environ.items() if key.upper() not in {"LINK", "_LINK_"}},
        LINK="", _LINK_="",
        WINEPREFIX=str(VC5_PREFIX if ACTIVE_TOOL == "cl5" else VC6_PREFIX),
        WINEDEBUG=os.environ.get("WINEDEBUG", "-all"),
        INCLUDE=";".join("Z:" + str(part).replace("/", "\\") for part in include),
        LIB=";".join("Z:" + str(part).replace("/", "\\") for part in library),
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


def take_link_diagnostics(tool: str, arguments: list) -> tuple:
    """Capture a configured link transcript without changing compiler argv."""
    paths = [a.split("=", 1)[1] for a in arguments if a.startswith("--link-diagnostics=")]
    if not paths:
        return arguments, None
    if tool != "link" or len(paths) != 1 or not paths[0]:
        raise ValueError("one nonempty --link-diagnostics path is supported only for link")
    if not any(a.upper() in ("/VERBOSE", "-VERBOSE") for a in arguments):
        raise ValueError("link diagnostics require full /VERBOSE, not /VERBOSE:LIB")
    path = Path(paths[0])
    if not path.parent.is_dir() or path.is_symlink() or path.exists() and not path.is_file():
        raise ValueError("link diagnostics require a regular output in an existing directory")
    return [a for a in arguments if not a.startswith("--link-diagnostics=")], path


def write_link_diagnostics(path: Path, text: str) -> None:
    # A new build gets a new path; ordinary incremental builds replace only
    # their configured derived log. Do not expose a partially written report.
    with tempfile.NamedTemporaryFile("w", dir=path.parent, prefix=".link-log-",
                                     encoding="utf-8", delete=False) as scratch:
        temporary = Path(scratch.name)
        try:
            scratch.write(text)
            scratch.flush()
            os.replace(temporary, path)
        finally:
            temporary.unlink(missing_ok=True)


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
    finished = run_tool(
        ["wine32", str(executable), "/nologo", "/E"] + [translate(a) for a in kept])
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


# ML echoes ` Assembling: <file>` per source; diagnostics follow on their own lines.
ASSEMBLER_SUMMARY = re.compile(r"^Assembling: ")
# ML 6.11d refuses source lines longer than 512 characters (A1009). Only
# full-line comments carrying annotations (RECOVERY_BINDING JSON) exceed it.
ASSEMBLER_LINE_LIMIT = 512


def assembler_source_copy(source: Path, directory: Path) -> Path:
    """Copy `source` with every over-long full-line `;` comment reduced to `;`.

    The copy keeps the basename and the line count, so ML's diagnostics still
    name the right file and line. Code lines are never rewritten: an over-long
    instruction is a source error ML must report.
    """
    lines = source.read_bytes().split(b"\n")
    shortened = [b";" + (b"\r" if line.endswith(b"\r") else b"")
                 if len(line.rstrip(b"\r")) > ASSEMBLER_LINE_LIMIT and line.lstrip().startswith(b";")
                 else line for line in lines]
    copy = directory / source.name
    copy.write_bytes(b"\n".join(shortened))
    return copy


def assemble(executable: Path, arguments: list) -> int:
    """Run ML with the CL wrapper's path translation and environment.

    No depfile, compiler frontend, reproducibility root or diagnostics capture:
    those are CL protocols. An assembler source that names one is a build
    configuration error, not something to drop silently. CMake's `-Fo <obj>`
    is joined into ML's `/Fo<obj>`, and `-X` options are spelled `/X` so
    absolute include paths get the same translation as CL's.
    """
    unsupported = [word for word in arguments if word.startswith("--")]
    if unsupported:
        print("unsupported assembler wrapper option: " + " ".join(unsupported), file=sys.stderr)
        return 3
    words, iterator = [], iter(arguments)
    for word in iterator:
        if word.startswith("-") and len(word) > 1:
            word = "/" + word[1:]
        if word in ("/Fo", "/I"):
            word += next(iterator, "")
        words.append(word)
    sources = [index for index, word in enumerate(words)
               if not word.startswith("/") or Path(word).is_file()]
    sources = [index for index in sources if words[index].lower().endswith(".asm")]
    objects = [word[3:] for word in words if word.startswith("/Fo")]
    if len(sources) != 1 or len(objects) != 1:
        print("the assembler wrapper needs exactly one .asm source and one /Fo object",
              file=sys.stderr)
        return 3
    source = Path(words[sources[0]]).resolve()
    output_directory = Path(objects[0]).parent.resolve()
    output_directory.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="ml-source-", dir=output_directory) as temporary:
        words[sources[0]] = str(assembler_source_copy(source, Path(temporary)))
        # The copy lives elsewhere, so name the original directory first: ML
        # searches the source's own directory, then /I directories in order.
        words.insert(0, "/I" + str(source.parent))
        translated = [translate(word) for word in words]
        finished = run_tool(["wine32", str(executable), *translated], explicit_assembler=True)
    for line in finished.stdout.splitlines():
        if line.strip() and not ASSEMBLER_SUMMARY.match(line.strip()):
            print(line)
    if finished.stderr.strip():
        print(finished.stderr, file=sys.stderr, end="")
    return finished.returncode


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

    if tool == "ml":
        executable = tool_executable(tool)
        if not executable.is_file():
            print(f"{executable} not found; set ML_ROOT", file=sys.stderr)
            return 127
        return assemble(executable, expand_responses(arguments))

    # BEFORE ANYTHING READS THE ARGUMENTS. Both the depfile extraction below
    # and the path translation are blind to whatever sits inside a `@file`.
    arguments = expand_responses(arguments)
    # `tool` stays the command family (cl, link, lib) for the option grammar
    # below; the selected compiler only changes the executable and prefix.
    try:
        selected, arguments = take_compiler(tool, arguments)
    except ValueError as error:
        print(str(error), file=sys.stderr)
        return 3
    global ACTIVE_TOOL
    ACTIVE_TOOL = selected
    executable = tool_executable(selected)
    if not executable.is_file():
        print(f"{executable} not found; set {ROOT_VARIABLES.get(selected, 'VC6_ROOT')}",
              file=sys.stderr)
        return 127
    compile_diagnostics = [word.split("=", 1)[1] for word in arguments
                           if word.startswith("--compile-diagnostics=")]
    if compile_diagnostics and (tool != "cl" or len(compile_diagnostics) != 1 or not compile_diagnostics[0]):
        print("one compile diagnostics path is supported only for cl", file=sys.stderr)
        return 3
    arguments = [word for word in arguments if not word.startswith("--compile-diagnostics=")]
    try:
        arguments, link_diagnostics = take_link_diagnostics(tool, arguments)
    except ValueError as error:
        print(str(error), file=sys.stderr)
        return 3
    arguments, dep_target, dep_file = take_depfile(arguments)
    frontends = [word for word in arguments if word.startswith("--frontend=")]
    if frontends:
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        from intrinsics import OPTION, compile as intrinsic_compile
        if tool != "cl" or selected != "cl" or frontends != [OPTION] or link_diagnostics is not None:
            print("unsupported or repeated explicit compiler frontend", file=sys.stderr)
            return 3
        ordinary = [word for word in arguments if word != OPTION]
        from wine_repro import preprocessing_probe
        try:
            probe = preprocessing_probe(tool, ordinary)
            if probe is not None and compile_diagnostics:
                raise ValueError("preprocessing probe cannot publish object evidence")
        except (OSError, ValueError) as error:
            print(str(error), file=sys.stderr)
            return 3
        if probe is None:
            return intrinsic_compile(ordinary, compile_diagnostics, dep_target, dep_file)
        # CMake advertises VC6/MSVC, so IDE compiler-information sources use
        # its macro-probe dialect. The per-source intrinsic override applies
        # to object compilation, not this advertised compiler identity.
        arguments = ordinary
    reproducible = None
    probe_only = False
    if any(word.startswith("--repro-root=") for word in arguments):
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        from wine_repro import ReproducibleCompile, preprocessing_probe, preprocessing_stderr_is_expected
        try:
            probe = preprocessing_probe(tool, arguments)
            if probe is not None:
                if dep_file or dep_target:
                    raise ValueError("preprocessing probe cannot publish object dependencies")
                arguments, probe_only = probe, True
            else:
                reproducible = ReproducibleCompile.read(tool, arguments, Path.cwd())
            def preprocess(words, directory):
                result = run_tool(["wine32", str(executable), *map(translate, words)],
                                  cwd=directory, binary=True, explicit_compiler=True)
                # /E emits a filename echo and may diagnose an explicitly
                # overridden inline option. Only the observed option-bound
                # driver diagnostic is admissible; C++ diagnostics still fail.
                if (result.returncode or not preprocessing_stderr_is_expected(
                        result.stderr, reproducible.source.name, words) or
                        re.search(rb"(?m)^.*(?:fatal error|warning|error) [CD][0-9]{4}\s*:", result.stdout)):
                    raise ValueError("reproducibility preprocessing failed: " +
                                     preprocessing_diagnostics(result.stdout, result.stderr))
                return result.stdout
            if reproducible is not None:
                arguments = reproducible.verify(preprocess)
                if dep_file:
                    reproducible.write_depfile(dep_file, dep_target or "")
        except (OSError, ValueError) as error:
            print(str(error), file=sys.stderr)
            return 3
    elif dep_file:
        # Before the real compile, and unconditionally: a depfile is what
        # tells ninja to try again once the header is fixed, so it has to
        # exist even when the compile that follows fails.
        write_depfile(executable, arguments, dep_file, dep_target or "")
    compile_capture = None
    if compile_diagnostics:
        try:
            sys.path.insert(0, str(Path(__file__).resolve().parent))
            from wine_compile import capture_before as compile_before, capture_after as compile_after
            compile_capture = compile_before(reproducible, arguments, executable)
            destination = Path(compile_diagnostics[0]).absolute()
            observed_paths = {Path(item["path"]) for item in compile_capture["inputs"]}
            if (destination in observed_paths or destination == Path(compile_capture["output"]["path"])
                    or destination.parent.resolve() != destination.parent or destination.is_symlink()
                    or destination.exists()):
                raise ValueError("compile diagnostics require a new canonical file, separate from compiler inputs and outputs")
        except (OSError, ValueError) as error:
            print(str(error), file=sys.stderr)
            return 3
    translated = [translate(a) for a in arguments]
    if compile_capture is not None:
        # Record the actual compiler spellings before optional response-file
        # packaging, not the host-side paths used to locate observed files.
        compile_capture["arguments"] = list(translated)
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
    input_capture = None
    if link_diagnostics is not None:
        # Keep standalone CMake entry points usable without importing project
        # recovery code or the installed framework into the compiler wrapper.
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        from wine_link import capture_before, capture_after
        input_capture = capture_before(arguments, executable,
            [part for part in os.environ.get("VC6_LIB", str(VC6_ROOT / "LIB")).split(":") if part], Path.cwd())
    if reproducible is not None:
        finished = run_tool(command, cwd=reproducible.root, explicit_compiler=True)
    elif probe_only:
        finished = run_tool(command, binary=True, explicit_compiler=True)
    else:
        finished = run_tool(command)
    if scratch is not None:
        os.unlink(scratch.name)
    if probe_only:
        # Preprocessor text is compiler information, not a build log. Preserve
        # blank lines and file-like lines as well as diagnostics for the IDE.
        sys.stdout.buffer.write(finished.stdout)
        sys.stderr.buffer.write(finished.stderr)
        return finished.returncode
    if reproducible is not None:
        try:
            reproducible.check_inputs()
        except (OSError, ValueError) as error:
            print(str(error), file=sys.stderr)
            return 3
    if compile_capture is not None:
        try:
            # Publish only after the logical include-path bindings survived the
            # final reproducibility check, not just their resolved file bytes.
            # Exclusive creation cannot overwrite raced-in source or old evidence.
            # Interrupted partial JSON is diagnostic debris, never valid evidence.
            with destination.open("x", encoding="utf-8") as stream:
                stream.write(json.dumps(compile_after(compile_capture, finished.returncode), indent=2) + "\n")
        except (OSError, ValueError) as error:
            print(f"cannot capture compile diagnostics: {error}", file=sys.stderr)
            return 3
    if link_diagnostics is not None:
        try:
            write_link_diagnostics(link_diagnostics, finished.stdout)
            evidence = capture_after(input_capture, finished.stdout)
            write_link_diagnostics(link_diagnostics.with_suffix(".inputs.json"), json.dumps(evidence, indent=2) + "\n")
        except OSError as error:
            print(f"cannot capture link diagnostics: {error}", file=sys.stderr)
            return 3
    # CL writes the source file name to stdout before any diagnostic. It is
    # noise in a build log and CMake does not need it, but a diagnostic that
    # follows it does, so only the bare echo line is dropped. The
    # `/showIncludes` notes go the same way - they are the depfile's input,
    # not a build log's.
    for line in finished.stdout.splitlines():
        if link_diagnostics is not None and finished.returncode == 0:
            # Keep the full transcript as a build artifact, not thousands of
            # lines of routine search/ICF chatter in every build report.
            if not re.search(r"\b(?:warning|error|fatal error) LNK\d+:", line, re.IGNORECASE):
                continue
        if line.strip() and not line.strip().lower().endswith((".cpp", ".c", ".cxx")):
            print(line)
    if finished.stderr.strip():
        print(finished.stderr, file=sys.stderr, end="")
    return finished.returncode


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
