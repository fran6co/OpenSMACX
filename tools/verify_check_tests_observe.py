#!/usr/bin/env python3
"""Mutate each check, and require its own tests to notice.

THIS IS THE ONE DIAGNOSTIC THAT WORKED. Three adversarial audits found 3, then
5, then 8 defects in this project's checks, with a flat 3-in-5 rate of fixes
that carried a new defect. Reading code found the first two rounds' worth. What
found the third round's vacuous tests was different: kill the tool and see
whether its suite still passes.

  test_verify_checks_can_fail.py asserted a property by inspecting main().__doc__
  - which is None - and stayed green under four of five single-edit mutations of
  the code it claimed to cover.

  test_measure_observability.py compared four string literals; folding
  NO-COMPILE into OBSERVED left all nine tests green.

Neither is visible by reading, both are obvious under mutation, and both were
written by someone actively trying to avoid exactly this. So the technique
belongs in the gate rather than in an audit that happens when someone thinks to
run one.

WHAT IT DOES. For each check tool with a test file, perturb the TOOL and run the
TOOL'S OWN TESTS. A mutant the tests still pass is a piece of the check nothing
observes - the same argument mutate_and_verify makes about recovered C++, turned
on the instruments themselves.

WHY IT IS AFFORDABLE HERE where the C++ sweep is not: no build. A mutant costs
one `python -m unittest` of a single module, tens of milliseconds, so the whole
population is a gate test rather than an eight-hour campaign.

A SURVIVOR IS NOT AUTOMATICALLY A BUG. Perturbing a log message or a help string
changes nothing a test should assert on. The threshold is therefore a RATE, per
tool, recorded and ratcheted - not zero.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import mutate_and_verify as mutation  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS = REPO_ROOT / "tools"


def prose_lines(text):
    """Line numbers holding a docstring or a comment, which are not behaviour.

    The mutation operators were written for C++ and happily perturb the digits
    in `2026-08-01` inside a docstring. Those mutants change nothing a test
    could observe, so counting them as survivors reports every suite as blind -
    the first run of this tool scored 15/15 survived on a tool whose tests are
    perfectly adequate. A survivor has to be a survivor of CODE.
    """
    import ast
    skip = set()
    for index, line in enumerate(text.splitlines(), 1):
        if line.lstrip().startswith("#"):
            skip.add(index)
    try:
        tree = ast.parse(text)
    except SyntaxError:
        return skip
    for node in ast.walk(tree):
        if not isinstance(node, (ast.Module, ast.ClassDef, ast.FunctionDef,
                                 ast.AsyncFunctionDef)):
            continue
        body = getattr(node, "body", None)
        if not body:
            continue
        first = body[0]
        if (isinstance(first, ast.Expr)
                and isinstance(first.value, ast.Constant)
                and isinstance(first.value.value, str)):
            skip.update(range(first.lineno, (first.end_lineno or first.lineno) + 1))
    return skip


def mutants_for(path, limit):
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines()
    prose = prose_lines(text)
    whole = mutation.Function(address="00000000", start=0, end=len(lines))
    out = []
    for mutant in mutation.build_mutants(lines, whole):
        if len(mutant.lines) != len(lines):
            continue                     # a drop; line-wise composition only
        if mutant.line_number in prose:
            continue
        out.append(mutant)
        if len(out) >= limit:
            break
    return lines, out


def run_tests(module, timeout):
    done = subprocess.run(
        [sys.executable, "-m", "unittest", module],
        cwd=str(TOOLS), capture_output=True, text=True, timeout=timeout)
    return done.returncode == 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--tools", nargs="*",
                        help="check tools to mutate; default is every "
                             "verify_*/audit_* with a test file")
    parser.add_argument("--limit", type=int, default=40,
                        help="mutants per tool")
    parser.add_argument("--timeout", type=float, default=120.0)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--report", type=Path)
    arguments = parser.parse_args()

    if arguments.tools:
        candidates = [TOOLS / name for name in arguments.tools]
    else:
        candidates = sorted(
            path for path in TOOLS.glob("*.py")
            if (path.name.startswith(("verify_", "audit_"))
                and (TOOLS / f"test_{path.name}").is_file()))
    if not candidates:
        print("check-tests-observe: no check tool has a test file, so this "
              "run measured NOTHING", file=sys.stderr)
        return 1

    results = {}
    for path in candidates:
        module = f"test_{path.stem}"
        original = path.read_text(encoding="utf-8")
        if not run_tests(module, arguments.timeout):
            print(f"check-tests-observe: {module} does not pass on the "
                  f"unmutated tool, so nothing can be measured from it",
                  file=sys.stderr)
            return 1
        _, mutants = mutants_for(path, arguments.limit)
        killed = survived = broken = 0
        examples = []
        try:
            for mutant in mutants:
                path.write_text("\n".join(mutant.lines) + "\n", encoding="utf-8")
                try:
                    compile(path.read_text(encoding="utf-8"), str(path), "exec")
                except SyntaxError:
                    broken += 1
                    continue
                if run_tests(module, arguments.timeout):
                    survived += 1
                    if len(examples) < 3:
                        examples.append(
                            f"line {mutant.line_number}: {mutant.description}")
                else:
                    killed += 1
        finally:
            path.write_text(original, encoding="utf-8")
        measured = killed + survived
        results[path.name] = {
            "killed": killed, "survived": survived, "uncompilable": broken,
            "rate": round(survived / measured, 3) if measured else None,
            "examples": examples}
        print(f"{path.name:44} killed {killed:3} survived {survived:3} "
              f"({'-' if not measured else f'{100*survived/measured:.0f}%'})",
              flush=True)

    total_killed = sum(r["killed"] for r in results.values())
    total_survived = sum(r["survived"] for r in results.values())
    measured = total_killed + total_survived
    if not measured:
        print("check-tests-observe: not one mutant was measured, so this run "
              "proves NOTHING about any check's tests", file=sys.stderr)
        return 1
    print(f"\ncheck-tests-observe: {total_killed}/{measured} mutants killed, "
          f"{total_survived} survived ({100*total_survived/measured:.0f}%)")

    if arguments.report:
        arguments.report.write_text(
            json.dumps(results, indent=1, sort_keys=True) + "\n",
            encoding="utf-8")

    if arguments.baseline and arguments.baseline.is_file():
        recorded = json.loads(arguments.baseline.read_text(encoding="utf-8"))
        cap = recorded.get("max_survived")
        if cap is not None and total_survived > cap:
            print(f"check-tests-observe: survivors rose from {cap} to "
                  f"{total_survived}. A check's tests observe less of it than "
                  f"they did.", file=sys.stderr)
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
