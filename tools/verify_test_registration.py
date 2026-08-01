#!/usr/bin/env python3
"""Refuse a test function that is defined and never registered.

tests/recovery_gameplay_tests.cpp self-registers its cases: each `void test_x()`
is followed by `GAMEPLAY_CASE(test_x);`, and main() walks the registry. That
removed the central call list, which was the one merge conflict in this file
where a careless resolution is SILENT - drop a line and the test stops running
while everything remaining still passes.

Self-registration moves the failure rather than removing it: now the way to
lose a test is to write it and forget the GAMEPLAY_CASE line. Nothing else can
see that. The compiler is happy, every assertion still passes, and the
assertion counter simply reads lower than it would have - which nobody notices
without a before-and-after.

So this is the guard that makes the registry safe, and it is the same check
tools/generate_leaf_manifest.py grew for tests/leaf/*.cpp on the same day, for
the same reason.

The reverse direction is deliberately not checked: a GAMEPLAY_CASE naming a
function that does not exist fails to compile, so the compiler covers it.
"""
import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SUITE = REPO_ROOT / "tests" / "recovery_gameplay_tests.cpp"

DEFINITION_RE = re.compile(r"^void (test_[A-Za-z0-9_]+)\s*\(\s*\)", re.MULTILINE)
REGISTRATION_RE = re.compile(r"^GAMEPLAY_CASE\((test_[A-Za-z0-9_]+)\)", re.MULTILINE)
# The old shape. If it comes back, the registry has been bypassed.
DIRECT_CALL_RE = re.compile(r"^    (test_[A-Za-z0-9_]+)\(\);$", re.MULTILINE)


def audit(path):
    text = path.read_text(encoding="utf-8")
    defined = DEFINITION_RE.findall(text)
    registered = set(REGISTRATION_RE.findall(text))
    main_at = text.find("\nint main()")
    direct = DIRECT_CALL_RE.findall(text[main_at:]) if main_at >= 0 else []
    return defined, registered, direct


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--suite", type=Path, default=DEFAULT_SUITE)
    parser.add_argument("--floor", type=int, default=50,
                        help="refuse to report clean over fewer cases than this")
    arguments = parser.parse_args()

    if not arguments.suite.is_file():
        print(f"test-registration: {arguments.suite} is absent, so this check "
              "verified NOTHING", file=sys.stderr)
        return 1

    defined, registered, direct = audit(arguments.suite)
    if len(defined) < arguments.floor:
        print(f"test-registration: only {len(defined)} test functions found in "
              f"{arguments.suite.name}, below the floor of {arguments.floor}. "
              "Refusing to report clean over a file this tool may have failed "
              "to parse.", file=sys.stderr)
        return 1

    unregistered = [name for name in defined if name not in registered]
    if unregistered:
        print(f"test-registration: {len(unregistered)} test(s) in "
              f"{arguments.suite.name} are defined and never registered, so "
              "they DO NOT RUN.\n  Add `GAMEPLAY_CASE(<name>);` after each:",
              file=sys.stderr)
        for name in unregistered:
            print(f"    {name}", file=sys.stderr)
        return 1

    if direct:
        print(f"test-registration: {len(direct)} case(s) are called directly "
              "from main() again. That list is what self-registration removed - "
              "it is\n  the merge conflict where dropping a line silently drops "
              "a test.", file=sys.stderr)
        for name in direct[:5]:
            print(f"    {name}", file=sys.stderr)
        return 1

    print(f"test-registration: {len(defined)} cases, all registered, none "
          "called directly from main()")
    return 0


if __name__ == "__main__":
    sys.exit(main())
