#!/usr/bin/env python3
"""Refuse a test file that defines a test class after its `unittest.main()`.

`unittest.main()` under `if __name__ == "__main__":` collects the classes
DEFINED SO FAR. Anything below that block is parsed, is valid, is reported by
nothing, and never runs - and the file still exits 0, so the gate that invokes
it goes green over a suite it did not execute.

Measured on 2026-08-13: tools/test_emit_translation_unit.py carried the block at
line 558 with four classes below it - ProvedSubobjectTests, DeclaredBeforeTests,
CrtDeclarationTests, OverloadDeclarationTests. The registered CMake test ran 57
of 73. Sixteen tests, including the ones covering embedded sub-objects and CRT
declaration ordering, had not executed since whenever the block was pasted into
the middle of the file.

WHY THIS IS ITS OWN CHECK rather than a fixed one-line move. Two gates already
watch the test suite and neither could see this: `test-registration` checks that
every case is REGISTERED, and `tool-test-registration` checks that CMake
EXECUTES every test file. Both were green. The file was registered, CMake did
run it, and it did exit 0 - it simply ran two thirds of itself. That is the
house failure mode written down in half a dozen places here: a check that
reports success while verifying nothing.

`unittest discover` does NOT have this behaviour - it imports the module and
finds every class - so the defect is invisible from a discovery run and appears
only under the invocation the gate actually uses. A test that passes one way and
silently shrinks the other is worth catching mechanically.
"""

from __future__ import annotations

import argparse
import ast
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS = REPO_ROOT / "tools"


def main_guard_line(tree: ast.Module) -> int | None:
    """Line of the `if __name__ == "__main__":` block that calls unittest.main.

    Only TOP-LEVEL blocks count. One nested inside a function or a class cannot
    end module execution the way this check is about.
    """
    for node in tree.body:
        if not isinstance(node, ast.If):
            continue
        for inner in ast.walk(node):
            if (isinstance(inner, ast.Call)
                    and isinstance(inner.func, ast.Attribute)
                    and inner.func.attr == "main"):
                return node.lineno
    return None


def stranded(path: Path) -> tuple:
    """(guard line, [(class, test count)]) for classes below the guard."""
    tree = ast.parse(path.read_text(errors="replace"), filename=str(path))
    guard = main_guard_line(tree)
    if guard is None:
        # No guard is fine: `python -m unittest <module>` still runs it, and
        # several files here are libraries of fixtures rather than entry points.
        return None, []
    below = []
    for node in tree.body:
        if isinstance(node, ast.ClassDef) and node.lineno > guard:
            tests = len([f for f in node.body
                         if isinstance(f, (ast.FunctionDef, ast.AsyncFunctionDef))
                         and f.name.startswith("test")])
            below.append((node.name, tests))
    return guard, below


def sweep(root: Path = None) -> list:
    """[(path, guard line, [(class, tests)])] for every offending file."""
    root = root or TOOLS
    found = []
    for path in sorted(root.glob("test_*.py")):
        guard, below = stranded(path)
        if below:
            found.append((path, guard, below))
    return found


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--tools", type=Path, default=TOOLS)
    args = parser.parse_args(argv)

    files = sorted(args.tools.glob("test_*.py"))
    if len(files) < 20:
        # A sweep that matches nothing must not read as a clean sweep.
        print(f"tests-all-run: found only {len(files)} test file(s) under "
              f"{args.tools}; the glob is wrong", file=sys.stderr)
        return 1

    offenders = sweep(args.tools)
    for path, guard, below in offenders:
        total = sum(count for _, count in below)
        print(f"    {path.name}: `unittest.main()` at line {guard}, with "
              f"{len(below)} class(es) / {total} test(s) defined after it",
              file=sys.stderr)
        for name, count in below:
            print(f"        {name} ({count} test(s)) never runs",
                  file=sys.stderr)
    if offenders:
        print(f"{len(offenders)} test file(s) run only part of themselves. Move "
              f"the `if __name__` block to the END of the file.", file=sys.stderr)
        return 1

    print(f"tests-all-run: {len(files)} test file(s), none define a test class "
          f"after their `unittest.main()`")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
