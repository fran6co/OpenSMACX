"""Explicit i386 intrinsic frontend behind the existing project compiler wrapper.

VC6 LINK cannot consume this frontend's CodeView records. The selected route
therefore omits debug information, never pretending to provide layout/PDB proof.
Ordinary compilations remain VC6; no source-name heuristic selects this route.
"""

import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

if __package__:
    from .wine_repro import ReproducibleCompile, _path
    from .wine_compile import capture_before, capture_after, stamp
else:
    from wine_repro import ReproducibleCompile, _path
    from wine_compile import capture_before, capture_after, stamp

OPTION = "--frontend=intrinsics-i386-no-debug"
_LINE = re.compile(rb'^\s*#\s+\d+\s+"((?:[^"\\]|\\[\\"])*)"(?:\s+[1-4])*\s*$')
_IMPLICIT = ("CL", "_CL_", "INCLUDE", "CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH",
             "COMPILER_PATH", "GCC_EXEC_PREFIX", "LIBRARY_PATH", "SDKROOT", "LLVM_PROFILE_FILE")
_POLICY = ("--driver-mode=cl", "--no-default-config", "--target=i686-pc-windows-msvc", "/TP",
           "/GS-", "/Zl", "/clang:-fno-addrsig",
           "/clang:-Wno-builtin-macro-redefined", "/clang:-march=i386",
           "/clang:-mgeneral-regs-only")
_STABLE = ('/D__DATE__="Jan  1 1970"', '/D__TIME__="00:00:00"',
           '/D__TIMESTAMP__="Thu Jan  1 00:00:00 1970"')
_SWITCHES = frozenset(("/nologo", "/c", "/TP", "/O1", "/O2", "/Od", "/Os", "/Ot",
    "/Oy", "/Oy-", "/Ob0", "/Ob1", "/Ob2", "/Oi", "/Oi-", "/Gy", "/Gy-",
    "/GX", "/GX-", "/GR", "/GR-", "/GF", "/GF-", "/Gd", "/GZ",
    "/EHsc", "/W0", "/W1", "/W2", "/W3", "/W4", "/Wall", "/w", "/WX", "/WX-",
    "/MT", "/MTd", "/MD", "/MDd"))


def registration(path=None):
    path = path or Path(__file__).with_name("frontends.json")
    value = json.loads(path.read_text())
    if (not isinstance(value, dict) or set(value) != {"schema_version", "intrinsics"}
            or type(value["schema_version"]) is not int or value["schema_version"] != 1
            or not isinstance(value["intrinsics"], dict)
            or set(value["intrinsics"]) != {"enabled", "compiler"}
            or type(value["intrinsics"]["enabled"]) is not bool
            or not isinstance(value["intrinsics"]["compiler"], str)
            or not value["intrinsics"]["compiler"] or "\0" in value["intrinsics"]["compiler"]):
        raise ValueError("invalid explicit intrinsic frontend registration")
    return value["intrinsics"]


def implicit(name):
    name = name.upper()
    return name in _IMPLICIT or name.startswith(("CCC_", "CLANG_", "_CLANG_", "CC_PRINT_"))


def compiler_environment():
    if any(value for name, value in os.environ.items() if implicit(name)):
        raise ValueError("intrinsic frontend refuses implicit compiler/include/config options")
    return {name: value for name, value in os.environ.items() if not implicit(name)}


def explicit_arguments(arguments):
    """Closed input language; backend/config/debug options cannot slip through."""
    result = []
    iterator = iter(arguments)
    for raw in iterator:
        if raw.startswith("--repro-root="):
            result.append(raw)
            continue
        word = "/" + raw[1:] if raw.startswith("-") else raw
        # The declared route deliberately provides no debug/PDB information.
        if word == "/Z7":
            continue
        if word.startswith("/Fd"):
            if word == "/Fd" and not next(iterator, ""):
                raise ValueError("missing ignored program database destination")
            continue
        if word in _SWITCHES or re.fullmatch(r"/(?:Zm|wd|we|wo)\d+", word):
            result.append(word)
            continue
        prefix = next((part for part in ("/FI", "/Fo", "/I", "/D", "/U") if word.startswith(part)), None)
        if prefix is not None:
            value = word[len(prefix):] or next(iterator, "")
            if not value:
                raise ValueError("missing explicit compiler operand")
            if prefix in ("/D", "/U"):
                name = value.split("=", 1)[0]
                if (not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name)
                        or name.startswith(("__", "_M_", "_MSC_")) or name in ("_WIN32", "_WIN64")):
                    raise ValueError("intrinsic target/language predefined macros cannot be overridden")
            result.append(prefix + value)
            continue
        if (not raw.startswith(("-", "@")) and not Path(raw).name.startswith(("-", "@"))
                and Path(raw).suffix.lower() in (".cpp", ".cxx") and Path(raw).is_file()):
            result.append(raw)
            continue
        raise ValueError("unsupported explicit intrinsic compiler argument: " + raw)
    return result


def executable():
    selected = registration()
    if not selected["enabled"]:
        raise ValueError("intrinsic frontend is not registered")
    spelling = shutil.which(selected["compiler"])
    if spelling is None:
        raise ValueError("the selected intrinsic frontend requires clang-cl")
    path = Path(spelling).resolve()
    if not path.is_file():
        raise ValueError("intrinsic compiler must resolve to a regular executable")
    return path


def toolchain_inputs():
    if not registration()["enabled"]:
        return {"files": {}, "directories": {}}
    tool = executable()
    stripper = debug_stripper(tool)
    result = subprocess.run([str(tool), "--driver-mode=gcc", "--no-default-config", "-print-resource-dir"],
                            env=compiler_environment(), capture_output=True, timeout=30)
    if result.returncode or result.stderr:
        raise ValueError("cannot observe intrinsic compiler resource directory")
    directory = Path(os.fsdecode(result.stdout).strip())
    if not directory.is_absolute() or not directory.is_dir():
        raise ValueError("intrinsic resource directory must be absolute and existing")
    headers = directory / "include"
    if not headers.is_dir():
        raise ValueError("intrinsic resource headers are missing")
    return {"files": {"intrinsic-compiler": str(tool), "intrinsic-debug-stripper": str(stripper)},
            "directories": {"intrinsic-headers": str(headers.resolve())}}


def debug_stripper(tool):
    path = (tool.parent / "llvm-objcopy").resolve()
    if not path.is_file():
        raise ValueError("intrinsic frontend requires its companion llvm-objcopy")
    return path


def parse_preprocessed(data, cwd):
    """Parse Clang line metadata, preserving every nonmetadata byte verbatim."""
    text, files = [], set()
    for line in data.splitlines(keepends=True):
        match = _LINE.fullmatch(line)
        if match:
            name = re.sub(rb'\\([\\"])', rb'\1', match[1])
            if name in (b"<built-in>", b"<command line>"):
                continue  # Supplied by the observed executable and explicit argv.
            path = _path(os.fsdecode(name), cwd)
            if not path.is_file():
                raise ValueError("unobserved intrinsic preprocessor input: " + str(path))
            files.add(path)
        elif re.match(rb'^\s*#\s*(?:line\b|\d)', line):
            raise ValueError("unsupported intrinsic line metadata")
        else:
            text.append(line)
    if not files:
        raise ValueError("intrinsic preprocessor supplied no real input files")
    return b"".join(text), files


def compile(arguments, diagnostics, dep_target, dep_file):
    """Execute one explicitly selected compile, with fresh real observations."""
    try:
        environment = compiler_environment()
        arguments = explicit_arguments(arguments)
        repro = ReproducibleCompile.read("cl", arguments, Path.cwd())
        if repro is None:
            raise ValueError("intrinsic frontend requires an explicit reproducible root")
        tool = executable()
        stripper = debug_stripper(tool)

        def run(words, cwd):
            return subprocess.run([str(tool), *_POLICY, *words], cwd=cwd,
                                  env=environment, capture_output=True, timeout=300)

        def preprocess(words, cwd):
            result = run(words, cwd)
            if result.returncode:
                raise ValueError("intrinsic preprocessing failed: " +
                                 (result.stdout + result.stderr).decode(errors="replace")[-4000:])
            return result.stdout

        # Clang does not use VC6's time-seeded namespace nonce. Check fixed
        # clock predefines against the ordinary text instead: active date/time
        # dependence refuses, rather than entering a content-keyed cache.
        effective = repro.verify(preprocess, parse_preprocessed=parse_preprocessed,
                                 stable_options=_STABLE)
        if dep_file:
            repro.write_depfile(dep_file, dep_target or "")
        observed = capture_before(repro, [*_POLICY, *effective], tool) if diagnostics else None
        # This is a compiler pipeline, not an assertion that the final object
        # is Clang's untouched output. Track the postprocessor and this policy
        # alongside the actual primary compiler argv and preprocessing closure.
        extras = {stripper, Path(__file__).resolve(), Path(__file__).with_name("frontends.json").resolve()}
        extra_stamps = {path: stamp(path) for path in extras}
        if observed is not None:
            observed["inputs"].extend({"path": str(path), "before": value}
                                      for path, value in sorted(extra_stamps.items()))
        destination = Path(diagnostics[0]).absolute() if diagnostics else None
        if destination is not None:
            paths = {Path(item["path"]) for item in observed["inputs"]}
            if (destination in paths or destination == Path(observed["output"]["path"])
                    or destination.parent.resolve() != destination.parent
                    or not destination.parent.is_dir() or destination.exists() or destination.is_symlink()):
                raise ValueError("intrinsic observation requires a new canonical separate destination")
        result = run(effective, repro.root)
        primary_output, stages = None, []
        if result.returncode == 0:
            output, = [word[3:] for word in effective if word.startswith("/Fo")]
            primary_output = stamp(output)
            argv = [str(stripper), "--strip-debug", output]
            stripped = subprocess.run(argv, cwd=repro.root, env=environment,
                                      capture_output=True, timeout=60)
            if stripped.returncode:
                raise ValueError("intrinsic debug stripping failed: " +
                                 (stripped.stdout + stripped.stderr).decode(errors="replace")[-4000:])
            stages.append({"tool": str(stripper), "cwd": str(repro.root), "argv": argv,
                           "inputs": [str(path) for path in sorted(extras)],
                           "input_before": primary_output, "output_after": stamp(output),
                           "exit_code": stripped.returncode})
        if any(stamp(path) != value for path, value in extra_stamps.items()):
            raise ValueError("intrinsic postprocessor or policy changed during compilation")
        repro.check_inputs()
        if observed is not None:
            observed = capture_after(observed, result.returncode)
            observed.update(schema_version=2,
                compiler_output={**observed["output"], "after": primary_output}, postprocess=stages)
            with destination.open("x", encoding="utf-8") as stream:
                stream.write(json.dumps(observed, indent=2) + "\n")
        sys.stdout.buffer.write(result.stdout)
        sys.stderr.buffer.write(result.stderr)
        return result.returncode
    except (OSError, ValueError, subprocess.TimeoutExpired) as error:
        print(str(error), file=sys.stderr)
        return 3
