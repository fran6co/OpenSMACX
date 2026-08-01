#!/usr/bin/env python3
"""Fail when this build directory is out of date, so a green ctest means something.

CTest never builds. Twice this project reported "62/62 passed" against binaries a
failed build had not replaced - once against a DLL that had failed to link. The
operator ran `ctest` after a build error and every executable still on disk from
the previous build ran and passed.

Per-binary freshness would NOT have caught that incident. No test executable links
OpenSMACX.dll - they are all static - so on the day the DLL failed to link, every
test binary was legitimately current. The DLL has to be asked about by name, which
is why --also exists.

Ninja is the only component that knows the real dependency graph, including the
header edges recorded in .ninja_deps, so this asks ninja rather than reimplementing
staleness with mtimes. `-n` runs nothing.

THREE checks, because the obvious one alone is silently vacuous here:

  1. Self-control FIRST. `ninja -n <a target that cannot exist>` must fail. If it
     succeeds, the query is blind and every later answer is worthless.

     It really is blind by default: file(GLOB ... CONFIGURE_DEPENDS) makes CMake
     emit a phony edge whose output never exists, so build.ninja is permanently
     dirty; a plain `ninja -n` then exits 0 after printing "Re-running CMake..."
     WITHOUT EVER PARSING THE TARGET LIST. Naming the manifest by ABSOLUTE path
     matches no node in it, which suppresses that regeneration phase. Measured
     2026-08-01: `ninja -n opensmacx--no-such-target` exits 0, and the same query
     with `-f <abs>/build.ninja` exits 1 with "unknown target".

  2. The artifacts ctest is about to launch, plus --also, must be up to date.
     Ninja prints "no work to do" only when they are. Exit status is NOT the
     signal: a dry run with pending work also exits 0, having merely PRINTED the
     commands it would run.

  3. Because check 1 deliberately suppressed manifest regeneration, compare
     build.ninja against the implicit inputs of its own RERUN_CMAKE edge, read out
     of the manifest rather than listed here - so an input nobody thought of
     (CMakeCache.txt, the toolchain file, a CMake module) cannot be forgotten.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

IMPOSSIBLE_TARGET = "opensmacx--no-such-target--freshness-self-control"


def ninja(program, build_dir, arguments):
    """Always names the manifest absolutely; see check 1 in the module docstring."""
    manifest = (build_dir / "build.ninja").resolve()
    return subprocess.run(
        [str(program), "-C", str(build_dir), "-f", str(manifest), "-n",
         *arguments],
        capture_output=True, text=True)


def ctest_executables(ctest_file, build_dir):
    """The .exe files CTest will launch, derived rather than maintained."""
    if not ctest_file.is_file():
        return []
    text = ctest_file.read_text(encoding="utf-8", errors="replace")
    found = set()
    for match in re.finditer(r'"([^"]*?/([A-Za-z0-9_.-]+\.exe))"', text):
        path = Path(match.group(1))
        try:
            found.add(path.resolve().relative_to(build_dir.resolve()).as_posix())
        except ValueError:
            continue
    return sorted(found)


def regeneration_inputs(build_dir):
    """The implicit inputs of the `build build.ninja: RERUN_CMAKE ...` edge."""
    manifest = build_dir / "build.ninja"
    for line in manifest.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("build build.ninja") and "RERUN_CMAKE" in line:
            if "|" not in line:
                return []
            tail = line.split("|", 1)[1]
            # `||` would start order-only inputs; stop there if present.
            tail = tail.split("||", 1)[0]
            return [one for one in tail.split() if one]
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--ctest-file", type=Path)
    parser.add_argument("--ninja", default="ninja")
    parser.add_argument("--also", action="append", default=[],
                        help="extra artifacts nothing else pulls in - the DLL")
    arguments = parser.parse_args()

    build_dir = arguments.build_dir.resolve()
    if not (build_dir / "build.ninja").is_file():
        print(f"build-freshness: no build.ninja in {build_dir}", file=sys.stderr)
        return 1

    # 1. Self-control. An answer from a blind query is worse than no answer.
    control = ninja(arguments.ninja, build_dir, [IMPOSSIBLE_TARGET])
    if control.returncode == 0:
        print("build-freshness: FAILED - the staleness query is BLIND.\n"
              f"  `ninja -n {IMPOSSIBLE_TARGET}` succeeded, but that target "
              "cannot exist.\n"
              "  Ninja is answering without parsing the target list, so every "
              "freshness\n  answer below it would be vacuous. See check 1 in "
              "this tool's docstring.",
              file=sys.stderr)
        return 1

    # 2. Are the artifacts CTest launches actually current?
    targets = ctest_executables(arguments.ctest_file, build_dir) if \
        arguments.ctest_file else []
    for extra in arguments.also:
        path = Path(extra)
        try:
            targets.append(path.resolve().relative_to(build_dir).as_posix())
        except ValueError:
            targets.append(extra)
    if not targets:
        print("build-freshness: FAILED - no artifacts to check; the ctest file "
              "named none and --also was empty, so this would pass vacuously.",
              file=sys.stderr)
        return 1

    result = ninja(arguments.ninja, build_dir, targets)
    combined = result.stdout + result.stderr
    if result.returncode != 0:
        print("build-freshness: FAILED - ninja could not evaluate the "
              f"artifacts:\n{combined.strip()}", file=sys.stderr)
        return 1
    if "no work to do" not in combined:
        pending = [one for one in combined.splitlines()
                   if one.strip() and not one.startswith("ninja: Entering")]
        print("build-freshness: FAILED - the build is OUT OF DATE. CTest does "
              "not build,\n  so the tests below this would run the PREVIOUS "
              "binaries and report them\n  passing. Build first.\n"
              "  ninja would run:", file=sys.stderr)
        for line in pending[:12]:
            print(f"    {line}", file=sys.stderr)
        if len(pending) > 12:
            print(f"    ... and {len(pending) - 12} more", file=sys.stderr)
        return 1

    # 3. The regeneration edge check 1 deliberately skipped.
    inputs = regeneration_inputs(build_dir)
    if inputs is None:
        print("build-freshness: FAILED - no RERUN_CMAKE edge in build.ninja; "
              "this tool cannot tell whether CMake needs re-running.",
              file=sys.stderr)
        return 1
    manifest_time = (build_dir / "build.ninja").stat().st_mtime
    newer = []
    for one in inputs:
        path = Path(one)
        if not path.is_absolute():
            path = build_dir / path
        try:
            if path.stat().st_mtime > manifest_time:
                newer.append(str(path))
        except OSError:
            continue
    if newer:
        print("build-freshness: FAILED - CMake needs re-running; these are "
              "newer than build.ninja:", file=sys.stderr)
        for one in newer[:8]:
            print(f"    {one}", file=sys.stderr)
        return 1

    print(f"build-freshness: {len(targets)} artifacts up to date, "
          f"{len(inputs)} configure inputs older than build.ninja")
    return 0


if __name__ == "__main__":
    sys.exit(main())
