#!/usr/bin/env python3

"""Every Wine-backed CTest must hold the `wineprefix` RESOURCE_LOCK.

The seven tests that go through `tools/run_windows_test.py` all drive the one
marker-protected prefix, and that runner stops the prefix when it finishes -
so two of them running at once means one killing the other's wineserver
mid-test. Measured on the debug preset: serial `ctest` passed 58/58 in 67.9 s,
plain `ctest -j8` finished in 45.6 s and failed recovery-leaf-tests; an earlier
lane failed three of them. CTest never schedules two holders of the same
RESOURCE_LOCK together, which is what makes -j honest here.

The reason this check exists rather than a comment: a wine-backed test that is
MISSING from the lock list does not fail, it flakes, and it flakes in a
different test each run. The list in CMakeLists.txt is therefore checked
against the GENERATED CTestTestfile.cmake - what CTest will really run - so
adding an eighth Wine-backed test without locking it fails the gate
immediately.
"""

import argparse
from pathlib import Path
import re
import sys


RUNNER = "run_windows_test.py"
LOCK = "wineprefix"
ADD_TEST = re.compile(r'^add_test\(\s*"([^"]+)"(.*)$')
PROPERTIES = re.compile(r'^set_tests_properties\(\s*"([^"]+)"\s+PROPERTIES(.*)$')


def parse(text):
    """(tests needing the lock, tests holding it).

    Line-oriented on purpose: CTestTestfile.cmake is generated, one statement
    per line, and a real CMake parser here would be a second thing to be wrong.
    """
    wine_backed = []
    locked = set()
    for line in text.splitlines():
        line = line.strip()
        match = ADD_TEST.match(line)
        if match:
            name, rest = match.groups()
            if _invokes(rest):
                wine_backed.append(name)
            continue
        match = PROPERTIES.match(line)
        if match:
            name, rest = match.groups()
            if re.search(r"RESOURCE_LOCK\s+\"?" + LOCK + r"\"?", rest):
                locked.add(name)
    return wine_backed, locked


def _invokes(arguments):
    """True when the test command RUNS the Wine runner.

    `tools/test_run_windows_test.py` is a host-side unit test whose own path
    ends in the runner's name; it takes no --executable and must not be
    locked, or the suite serialises a test that needs nothing.
    """
    if f"/{RUNNER}" not in arguments:
        return False
    return "--wine-prefix" in arguments


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ctest-file", required=True,
                        help="generated CTestTestfile.cmake to check")
    # The two cases the old code could not tell apart, separated by making the
    # caller state which one it is in. A native Windows build really
    # does have zero Wine-backed tests; a cross build has seven, and zero there
    # means this check stopped matching rather than that the locks are fine.
    #
    # Deliberately "at least" and not "exactly": blinding shows up as a count
    # BELOW the expectation, while legitimately adding an eighth Wine test must
    # not turn the gate red for a number that was only ever a floor.
    parser.add_argument("--expect-at-least", type=int, default=0,
                        help="fail if fewer than this many Wine-backed tests "
                             "are found; 0 means a build that may have none")
    arguments = parser.parse_args()

    path = Path(arguments.ctest_file)
    if not path.is_file():
        print(f"error: {path} does not exist", file=sys.stderr)
        return 2
    wine_backed, locked = parse(path.read_text(encoding="utf-8"))
    if len(wine_backed) < arguments.expect_at_least:
        print(f"error: found {len(wine_backed)} test(s) running {RUNNER} in "
              f"{path}, expected at least {arguments.expect_at_least}. Either "
              f"a Wine-backed test was removed, or this check has been blinded "
              f"- it matches on the literal strings '/{RUNNER}' and "
              f"'--wine-prefix', and renaming either makes it see nothing while "
              f"still reporting success. If the removal is intended, lower "
              f"--expect-at-least where it is registered.", file=sys.stderr)
        return 1
    if not wine_backed:
        # Reachable only when the caller said zero is legitimate.
        print(f"no test in {path} runs {RUNNER}; this check verified NOTHING")
        return 0
    missing = [name for name in wine_backed if name not in locked]
    for name in sorted(missing):
        print(f"error: {name} runs {RUNNER} without RESOURCE_LOCK {LOCK}; "
              f"under `ctest -j` it will race the other Wine tests for the "
              f"single owned prefix and fail intermittently", file=sys.stderr)
    print(f"{len(wine_backed)} Wine-backed test(s), "
          f"{len(wine_backed) - len(missing)} holding RESOURCE_LOCK {LOCK}")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
