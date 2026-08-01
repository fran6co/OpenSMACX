#!/usr/bin/env python3
"""Refuse a tools/test_*.py that CMake never executes.

A Python tool test runs only because CMakeLists.txt names it. Writing the file
is not enough, and nothing about an unregistered one looks wrong: it imports,
it passes when run by hand, it sits in tools/ beside 61 others that do run, and
the gate reports a clean sweep without it. Four active test files sat
unregistered here before anyone noticed, and the count has been wrong in the
other direction too - this file exists because the author of a new check has to
remember a second, unrelated edit, and that is exactly the kind of rule that
should not depend on memory.

TWO FORMS COUNT AS REGISTERED, and telling them apart is the point:

  1. `opensmacx_add_python_tool_test(<name> <file>)`, the normal path.
  2. Executed directly as `COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}" <file>`,
     which three of them are - they gate a generator by running its tests
     immediately before it.

A file named ONLY in a `DEPENDS` list is NOT registered. That is the trap this
check is really for: DEPENDS makes CMake rebuild when the test changes, so the
reference looks like registration and reads like registration, while the test
never executes. Grepping for the filename would call that a pass.
"""
import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

HELPER_CALL = re.compile(
    r"opensmacx_add_python_tool_test\(\s*\S+\s+(test_\S+?\.py)\s*\)")
# `COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"` followed by the script it runs,
# which CMake style puts on the next line.
PYTHON_COMMAND = re.compile(
    r"COMMAND\s+\"\$\{OPENSMACX_PYTHON_EXECUTABLE\}\"\s*\n\s*"
    r"\"[^\"]*?/tools/(test_\S+?\.py)\"")


def executed_tests(cmake_text):
    return (set(HELPER_CALL.findall(cmake_text))
            | set(PYTHON_COMMAND.findall(cmake_text)))


def mentioned_tests(cmake_text):
    return set(re.findall(r"/tools/(test_\S+?\.py)", cmake_text))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--cmake", type=Path,
                        default=REPO_ROOT / "CMakeLists.txt")
    parser.add_argument("--tools", type=Path, default=REPO_ROOT / "tools")
    parser.add_argument("--floor", type=int, default=50,
                        help="refuse to report clean over fewer test files "
                             "than this; a wrong --tools finds nothing")
    arguments = parser.parse_args()

    if not arguments.cmake.is_file():
        print(f"tool-test-registration: {arguments.cmake} is absent, so this "
              f"check verified NOTHING", file=sys.stderr)
        return 1
    if not arguments.tools.is_dir():
        print(f"tool-test-registration: {arguments.tools} is not a directory, "
              f"so this check verified NOTHING", file=sys.stderr)
        return 1

    cmake_text = arguments.cmake.read_text(encoding="utf-8")
    present = sorted(path.name for path in arguments.tools.glob("test_*.py"))
    if len(present) < arguments.floor:
        print(f"tool-test-registration: found only {len(present)} test files "
              f"in {arguments.tools}, below the floor of {arguments.floor}. "
              f"Refusing to report clean over a directory this tool may have "
              f"failed to read.", file=sys.stderr)
        return 1

    executed = executed_tests(cmake_text)
    if not executed:
        print("tool-test-registration: parsed ZERO registrations out of "
              f"{arguments.cmake}. The file's shape has changed and this "
              "check can no longer see registrations at all.", file=sys.stderr)
        return 1

    mentioned = mentioned_tests(cmake_text)
    unregistered = [name for name in present if name not in executed]
    if unregistered:
        print(f"tool-test-registration: {len(unregistered)} test file(s) are "
              f"never executed by CMake, so they DO NOT RUN:", file=sys.stderr)
        for name in unregistered:
            if name in mentioned:
                print(f"    {name}  - named in CMakeLists.txt but only as a "
                      f"dependency, never as a COMMAND", file=sys.stderr)
            else:
                print(f"    {name}  - not named in CMakeLists.txt at all",
                      file=sys.stderr)
        print("  Add `opensmacx_add_python_tool_test(<name> <file>)`.",
              file=sys.stderr)
        return 1

    stale = sorted(name for name in executed
                   if not (arguments.tools / name).is_file())
    if stale:
        print(f"tool-test-registration: CMake runs {len(stale)} test file(s) "
              f"that do not exist: {', '.join(stale)}", file=sys.stderr)
        return 1

    print(f"tool-test-registration: {len(present)} test files, all executed "
          f"by CMake")
    return 0


if __name__ == "__main__":
    sys.exit(main())
