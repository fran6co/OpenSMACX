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
import os
import subprocess
import re
import sys
from dataclasses import dataclass
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))


REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS = REPO_ROOT / "tools"


# THE MUTANT GENERATOR, INLINED. This imported it from mutate_and_verify.py,
# which was deleted with the mutation-testing route on 2026-08-12 - so this
# diagnostic has been dead ever since, and nothing noticed, because it is
# reachable from no gate BY DESIGN and its own import error only surfaces when
# somebody runs it. It was then deleted as an orphan on 2026-08-13 and restored
# the same day. Only  and  were ever used, 102 lines of
# 968, so the dependency is now gone rather than resurrected.


@dataclass
class Function:
    address: str
    start: int  # index of first body line
    end: int    # index one past last body line


@dataclass
class Mutant:
    address: str
    line_number: int
    operator: str
    description: str
    lines: list[str]


INT_LITERAL = re.compile(
    r"(?<![\w.])(0[xX][0-9A-Fa-f]+|\d+)([uUlL]*)(?![\w.])")
COMPARISONS = [
    (re.compile(r"=="), "!="),
    (re.compile(r"!="), "=="),
    (re.compile(r"<="), "<"),
    (re.compile(r">="), ">"),
    (re.compile(r"(?<![<>=!-])<(?![<=])"), "<="),
    (re.compile(r"(?<![<>=!-])>(?![>=])"), ">="),
]
TEMPLATE_ANGLE = re.compile(r"\b\w+<[^<>;]*>")
DECLARATION = re.compile(
    r"^\s*(?:const\s+|static\s+|volatile\s+|constexpr\s+|auto\s+)*"
    r"[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*(?:=|\{|\()")


def is_simple_statement(line: str) -> bool:
    stripped = line.strip()
    if not stripped.endswith(";"):
        return False
    if stripped.startswith('__asm__ __volatile__(""'):
        # This zero-instruction compiler barrier carries no source-observable
        # behavior. It constrains register residue and belongs to the ABI
        # disassembly gate; dropping it can never be killed by a C++ fixture.
        return False
    if stripped.startswith("*"):
        # `*` leads both block-comment continuations (`* like this`) and
        # pointer-dereference assignments (`*x += 1;`). Only the former is a
        # non-statement. Treating both as comments silently skipped every
        # store through a pointer parameter - an out-parameter function could
        # report full coverage having had none of its writes mutated.
        return not stripped.startswith("* ")
    return not stripped.startswith(CONTROL_PREFIXES)

def statements_interact(first: str, second: str) -> bool:
    """True when swapping two statements could change behaviour.

    Statements that write distinct lvalues and read nothing the other writes
    are order-independent, so swapping them yields an *equivalent mutant*: it
    survives every possible suite and would be reported as a coverage hole it
    is not. Only genuine write-after-write or read-after-write pairs are worth
    spending a build on.
    """
    # Swapping a statement with a TEXTUALLY IDENTICAL one reproduces the
    # original file exactly, so the mutant is the program itself and no suite
    # can ever kill it. This is not a heuristic about side effects - it is the
    # observation that the two orderings are the same bytes.
    #
    # The `not first_target` branch below keeps opaque calls on the grounds
    # that they may have side effects, which is right in general and wrong
    # here: two consecutive `Win::clear_bubble_text();` calls are exactly the
    # shape four recovered bubble-dismiss handlers have, and every one of them
    # reported a permanent coverage hole that could not be closed. Reporting
    # those "trains you to ignore the output", which is this file's own stated
    # reason for filtering equivalent swaps at all.
    if first.strip() == second.strip():
        return False
    first_target, first_base, first_rhs = split_assignment(first)
    second_target, second_base, second_rhs = split_assignment(second)
    if not first_target or not second_target:
        return True  # An opaque call may carry side effects; keep it.
    if first_target == second_target:
        return True  # write-after-write on the same lvalue
    word = lambda name, text: bool(name) and re.search(rf"\b{re.escape(name)}\b", text)
    return bool(word(first_base, second_rhs) or word(second_base, first_rhs))

def constant_replacements(literal: str, value: int, suffix: str):
    """The perturbations of one integer literal, widest-blast-radius first.

    ZERO IS NOT ENOUGH, and this is the hole it left. Until 2026-08-01 the only
    mutant was `0` (or `1` for a zero), so "all mutants killed" proved every
    constant was PRESENT AND NON-ZERO - never that it held the right value.
    Measured on a tree whose sweep read 42/42 killed: changing a region penalty
    from `512` to `511` passed the entire suite. That applies to every constant
    in every recovery this harness has certified, and the project's own
    documentation treats a clean sweep as the bar.

    So each literal now also moves by one in each direction. An off-by-one is
    the perturbation a fixture is likeliest to be blind to, because a test built
    from the same reading of the disassembly as the code tends to exercise the
    interior of a range rather than its edge - which is exactly where a wrong
    boundary hides.

    The base is preserved: `0x200` becomes `0x201`, not `513`. Recovered code
    spells bitmasks and offsets in hex, and a decimal mutant of one is harder to
    read in a survivor report than the thing it came from.

    Zero yields `1` only. `-1` would be a sign change rather than an off-by-one,
    and on the unsigned literals this codebase is full of it wraps to a value
    that reads as an unrelated defect.
    """
    hexadecimal = literal[:2] in ("0x", "0X")

    def spell(number):
        return (f"0x{number:X}" if hexadecimal else str(number)) + suffix

    if value == 0:
        return [spell(1)]
    # `1` collapses two of these onto `0`; dedupe in order so the report does
    # not list the same mutant twice.
    ordered = []
    for candidate in (spell(0), spell(value + 1), spell(value - 1)):
        if candidate not in ordered:
            ordered.append(candidate)
    return ordered

def changes_subscript_value(line: str, start: int, end: int,
                            replacement: str) -> bool:
    """Reject constant mutants that leave a simple array index unchanged.

    Recovered code commonly spells byte offsets as `object[0xA4 / 4]`. A
    mechanical zero-to-one mutation of `object[0x000 / 4]` still indexes zero,
    while mutating its divisor to zero creates an invalid expression. Neither
    is a behavioral perturbation worth rebuilding. Constants outside a simple
    positive-integer division remain eligible.
    """
    opening = line.rfind("[", 0, start + 1)
    closing = line.find("]", end)
    if opening < 0 or closing < 0:
        return True
    expression_start = opening + 1
    original_expression = line[expression_start:closing]
    mutated_expression = (
        line[expression_start:start] + replacement + line[end:closing])
    original = INDEX_DIVISION.fullmatch(original_expression)
    mutated = INDEX_DIVISION.fullmatch(mutated_expression)
    if not original or not mutated:
        return True
    original_denominator = int(original.group("denominator"), 0)
    mutated_denominator = int(mutated.group("denominator"), 0)
    if original_denominator == 0 or mutated_denominator == 0:
        return False
    original_index = (
        int(original.group("numerator"), 0) // original_denominator)
    mutated_index = (
        int(mutated.group("numerator"), 0) // mutated_denominator)
    return original_index != mutated_index

def code_extent(line: str) -> int:
    """Index at which the compiled part of `line` ends, before any comment.

    Quote-aware: `"http://host"` and `'/'` both contain a slash pair that is not
    a comment, and treating them as one would silently drop real mutants, which
    is the worse direction to err in.

    A whole-line `//` is already skipped by the caller, but a TRAILING comment
    was not, and its digits and operators compile to nothing - so perturbing
    them yields a mutant that behaves identically to the original, survives
    every test, and reads as a coverage hole. One `// RadiusBase entry 8 is
    (0,0)` on a `for` line produced four such phantoms (commit 2db61f0), and a
    genuine survivor was hiding among them.
    """
    quote = None
    index = 0
    while index < len(line):
        char = line[index]
        if quote is not None:
            if char == "\\":
                index += 2
                continue
            if char == quote:
                quote = None
        elif char in "\"'":
            quote = char
        elif char == "/" and line[index:index + 2] in ("//", "/*"):
            return index
        index += 1
    return len(line)


INDEX_DIVISION = re.compile(
    r"\s*(?P<numerator>0[xX][0-9A-Fa-f]+|\d+)\s*/\s*"
    r"(?P<denominator>0[xX][0-9A-Fa-f]+|\d+)\s*")


CONTROL_PREFIXES = (
    "return", "if", "for", "while", "else", "do", "switch", "case", "break",
    "continue", "goto", "static_assert", "#", "//", "/*", "*", "}", "{",
)


def declares_name_used_later(lines: list[str], index: int, end: int) -> bool:
    """True when this line declares a local that a later line still reads.

    Dropping or reordering such a line cannot compile - the use precedes the
    declaration - so the mutant is guaranteed to be reported as `no compile`
    after a full rebuild and a Wine run that proved nothing. The house
    transcription style leans on `const` locals, which makes that the common
    case rather than the rare one: on the TutWin do_* batch 72 of 128 mutants
    (56%) never compiled, and across all of src/ this predicate flags 2,080 of
    9,782 drop/swap mutants (21%).

    The wasted builds are the smaller cost. The real one is that the summary
    prints `killed 56/56  survived 0`, which reads as full coverage when in
    fact every statement-order mutant failed to compile and the emission order
    was never tested at all. That exact misreading had to be corrected once
    already, on GraphicWin::init in commit 2aa199f. Suppressing these here, and
    reporting per operator below, makes the untested case show up as a zero
    instead of hiding inside the no-compile total.
    """
    match = DECLARATION.match(lines[index])
    if not match:
        return False
    name = match.group(1)
    pattern = re.compile(rf"\b{re.escape(name)}\b")
    return any(pattern.search(lines[later]) for later in range(index + 1, end))


OBJECT_RE_TEMPLATE = r"CMakeFiles/([A-Za-z0-9_.-]+)\.dir/[^ :|]*{name}\.obj"
LINKED_EXE_RE = re.compile(r"^build ([A-Za-z0-9_.-]+)\.exe\s*:", re.MULTILINE)


def build_mutants(lines: list[str], function: Function) -> list[Mutant]:
    """Derive mechanical perturbations of one function body."""
    mutants: list[Mutant] = []

    def emit(number: int, operator: str, description: str, mutated: list[str]) -> None:
        mutants.append(Mutant(function.address, number, operator, description, mutated))

    for index in range(function.start, function.end):
        line = lines[index]
        stripped = line.strip()
        if stripped.startswith("//"):
            # A pure comment line has no compiled effect, so any literal or
            # operator inside it (`// borrowed flag at 0x28 is clear.`) is
            # guaranteed to survive every mutation - not a coverage hole, just
            # a build spent proving a comment doesn't execute.
            continue

        # A line declaring something later lines use cannot be dropped or
        # reordered and still compile, so neither operator below can learn
        # anything from it. See declares_name_used_later.
        structural = declares_name_used_later(lines, index, function.end)

        # 1. Drop a store. Catches assertions that never read the field.
        if is_simple_statement(line) and not structural:
            emit(index + 1, "drop-statement", f"drop `{stripped}`",
                 lines[:index] + lines[index + 1:])

        # Everything a trailing comment contains is invisible to the compiler,
        # so blank it - length-preserving, because every match offset below is
        # used to slice the ORIGINAL line.
        extent = code_extent(line)
        code_line = line[:extent] + " " * (len(line) - extent)

        # 2. Perturb a constant. Catches fixtures driving unobservable state.
        emitted_constant_lines = set()
        for match in INT_LITERAL.finditer(code_line):
            literal = match.group(0)
            try:
                value = int(match.group(1), 0)
            except ValueError:
                # Not every run of digits is a literal this can perturb. `08`
                # in a date is the case that surfaced it - Python rejects a
                # leading-zero decimal under base 0 - and the whole sweep died
                # on one comment rather than skipping one token. A literal that
                # cannot be parsed cannot be meaningfully mutated either way.
                continue
            for replacement in constant_replacements(literal, value,
                                                     match.group(2)):
                if not changes_subscript_value(
                        line, match.start(), match.end(), replacement):
                    continue
                mutated_line = (
                    line[:match.start()] + replacement + line[match.end():])
                if mutated_line == line or mutated_line in emitted_constant_lines:
                    continue
                emitted_constant_lines.add(mutated_line)
                emit(index + 1, "constant",
                     f"`{literal}` -> `{replacement}` in `{stripped}`",
                     lines[:index] + [mutated_line] + lines[index + 1:])

        # 3. Invert a comparison. Catches untested boundaries. Template angle
        #    brackets are not comparisons; rewriting them only burns a build.
        #    The blanking must preserve length, because the match offset is
        #    used to slice the *original* line.
        comparable = TEMPLATE_ANGLE.sub(lambda m: " " * len(m.group(0)),
                                        code_line)
        #    EVERY comparison on the line, not the first one found. This used to
        #    `break` after one match, so `if (x >= lo && x <= hi)` perturbed a
        #    single bound and WHICH bound depended on the order of COMPARISONS
        #    rather than on anything about the code. A range check has two edges
        #    and an off-by-one lives on either. Measured over src/ on 2026-08-02:
        #    347 lines carry more than one comparison, so 347 boundaries in
        #    already-certified recoveries had never been touched by the harness
        #    that certified them. Cost is +393 comparison mutants, x1.15 on this
        #    operator and far less on a whole sweep.
        #
        #    The six patterns are mutually exclusive at any offset - `<` carries
        #    `(?![<=])` and `>` carries `(?![>=])` - so no occurrence is emitted
        #    twice. Sorted by offset purely so the mutant list reads in source
        #    order.
        occurrences = sorted(
            (match.start(), match.end(), match.group(0), target)
            for pattern, target in COMPARISONS
            for match in pattern.finditer(comparable))
        for start, end, found, target in occurrences:
            mutated_line = line[:start] + target + line[end:]
            emit(index + 1, "comparison",
                 f"`{found}` -> `{target}` in `{stripped}`",
                 lines[:index] + [mutated_line] + lines[index + 1:])

        # 4. Swap adjacent stores. Catches unverified write ordering, but only
        #    where the two statements actually interact (see statements_interact).
        if (index + 1 < function.end and is_simple_statement(line)
                and not structural
                and not declares_name_used_later(lines, index + 1, function.end)
                and is_simple_statement(lines[index + 1])
                and statements_interact(line, lines[index + 1])):
            emit(index + 1, "swap-adjacent", f"swap `{stripped}` with next",
                 lines[:index] + [lines[index + 1], line] + lines[index + 2:])

    return mutants


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
    """Mutable code lines, sampled ACROSS the file - never truncated at it.

    THE CAP USED TO BE A `break`, and that quietly turned every published
    survivor rate into a measurement of the TOP of the file. Measured
    2026-08-02: verify_recovery_abi has 1,828 eligible mutants, and the first 40
    are lines 45-258 of 2,608 - so its published "50% survivors" described 10%
    of the tool and none of the logic that decides anything. Six other checks
    were sampled over less than a third of their source the same way, and the
    only reason it was invisible is that a truncated run printed exactly what a
    complete run printed.

    A stride costs the same and samples everywhere. Returns the eligible total
    as well, so the caller can say how much it actually looked at - without that
    number a truncated run is indistinguishable from a census in every artifact
    it produces.
    """
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines()
    prose = prose_lines(text)
    whole = Function(address="00000000", start=0, end=len(lines))
    eligible = [mutant for mutant in build_mutants(lines, whole)
                if len(mutant.lines) == len(lines)   # a drop; line-wise only
                and mutant.line_number not in prose]
    if limit and len(eligible) > limit:
        stride = len(eligible) / limit
        sampled = [eligible[int(index * stride)] for index in range(limit)]
    else:
        sampled = eligible
    return lines, sampled, len(eligible)


def run_tests(module, timeout):
    # PYTHONDONTWRITEBYTECODE, because the alternative bit hard. Each mutant is
    # written to the .py, imported, and reverted - but CPython caches the
    # compiled MUTANT in __pycache__, and a later run whose source mtime and
    # size happen to match reuses it. That left a mutated
    # verify_wine_test_locks.pyc behind after a sweep, its unit test failed
    # against source that was demonstrably correct, and the gate went red on a
    # tree whose .py files were byte-identical to a green commit. Half an hour
    # of a stale artifact pretending to be a defect.
    environment = dict(os.environ, PYTHONDONTWRITEBYTECODE="1")
    try:
        done = subprocess.run(
            [sys.executable, "-m", "unittest", module], env=environment,
            cwd=str(TOOLS), capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        # A mutant that hangs the suite is DETECTED, not a reason to abandon
        # the sweep. Mutating a loop bound in the brace scanner does exactly
        # this, and letting it propagate cost a completed run over 14 tools -
        # every result lost to one hung subprocess, which is the same shape as
        # a sweep that reports a tally for the functions it did not skip.
        return False
    return done.returncode == 0


def discard_bytecode(path):
    """Remove any cached compile of this module, mutated or not."""
    cache = path.parent / "__pycache__"
    for stale in cache.glob(f"{path.stem}.*.pyc"):
        stale.unlink(missing_ok=True)


def write_report(path, results):
    """Merge these results into the report, now, not at the end of the sweep.

    A FULL CENSUS TAKES OVER AN HOUR AND WAS ALL-OR-NOTHING. The write happened
    once, after every tool, so stopping the run - for any reason, including the
    operator realising the remaining tools cannot change the answer - discarded
    everything measured so far. Demonstrated 2026-08-02: seventeen tools, thirty
    minutes, every result present in the log and none of it in the JSON. The log
    is not the artifact; the JSON is.

    Merging rather than replacing is what makes a targeted re-measure of one
    tool cheap, and it is also what makes writing after every tool safe.
    """
    if path is None:
        return
    merged = {}
    if path.is_file():
        try:
            merged = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            # Losing a corrupt report is bad; losing the run that would have
            # replaced it is worse.
            merged = {}
    merged.update(results)
    path.write_text(json.dumps(merged, indent=1, sort_keys=True) + "\n",
                    encoding="utf-8")


CMAKELISTS = REPO_ROOT / "CMakeLists.txt"
CHECK_PREFIXES = ("verify_", "audit_", "measure_")
ADD_TEST = re.compile(r"add_test\(\s*NAME\s+([A-Za-z0-9_-]+)")
TOOL_SCRIPT = re.compile(r"tools/([A-Za-z0-9_]+\.py)")


def registered_check_scripts(cmakelists=None):
    """{gate check name: the tools/*.py it runs}, for the checks CMake registers.

    The POPULATION is `verify_checks_can_fail.gate_checks`, which is the one
    authority on what a registered check is; this only resolves each of its
    names to the script behind it, because that function returns names alone.
    A name it reports and this cannot resolve is a warning on stderr rather
    than a silent omission - a check that vanishes from the census is exactly
    the failure this whole file exists to catch.

    (The block walk below duplicates five lines of gate_checks. It should not
    have to: gate_checks wants to return {name: script}, at which point this
    function is one call. That edit belongs to verify_checks_can_fail.py.)
    """
    # EVERY CMakeLists.txt when no file is named, for the reason spelled out
    # in tools/cmake_sources.py: the `add_test` blocks moved into
    # tests/CMakeLists.txt, gate_checks refused the empty root file, and THIS
    # function caught that refusal and fell back to matching tool names by
    # prefix - still printing a sweep, over a population it no longer derived.
    import cmake_sources
    import verify_checks_can_fail as coverage
    path = Path(cmakelists) if cmakelists else None
    names = coverage.gate_checks(path)
    source = path.read_text(errors="replace") if path else cmake_sources.cmake_text()
    body = re.sub(r"(?<!\\)#.*", "", source)
    found = {}
    for match in ADD_TEST.finditer(body):
        if match.group(1) not in names:
            continue
        index, depth = match.end(), 1
        while index < len(body) and depth:
            depth += (body[index] == "(") - (body[index] == ")")
            index += 1
        # A `test_*.py` in the block is the check's own unit suite, which
        # run_tests already covers; the check itself is the other script.
        scripts = [script for script
                   in TOOL_SCRIPT.findall(body[match.end():index])
                   if not script.startswith("test_")]
        if scripts:
            found[match.group(1)] = scripts[0]
    unresolved = sorted(set(names) - set(found))
    if unresolved:
        print(f"check-tests-observe: {len(unresolved)} registered check(s) "
              f"name no tools/ script this could resolve, so they are outside "
              f"the sweep: {', '.join(unresolved)}", file=sys.stderr)
    return found


def candidate_tools(cmakelists=None):
    """Every check tool to sweep: what CMake registers, PLUS the prefix families.

    THE PREFIX TUPLE WAS ITSELF A HAND-MAINTAINED LIST, which is this project's
    highest-yield tooling defect shape, sitting in the one tool whose subject is
    checks that report success while observing nothing. `verify_*`/`audit_*`
    missed 13 of the 29 checks CMake registers - every `derive_*`, `emit_*`,
    `export_*`, `classify_*`, `correlate_*` and `measure_*` check, plus
    decomp_status - so the census printed a clean shape over 55% of its subject
    and the other 45% did not appear as `not measured`, it did not appear at
    all.

    The gate already knows which scripts are checks, so it is asked. The prefix
    families stay as a FLOOR for tools that are checks without being registered,
    and so that a CMakeLists that cannot be read can never SHRINK the population
    - a broken parse would otherwise quietly sweep less and print the same
    shape, which is the defect one level up.

    NOTE for whoever runs a full sweep: decomp_status.py is now correctly in
    this set, and it writes a shared, unlocked ledger. Mutating it and running
    its unit suite is safe; running its registered gate command against a mutant
    would not be.
    """
    registered = set()
    try:
        # SystemExit alongside Exception, because gate_checks raises it - and it
        # is a BaseException, so the obvious `except Exception` would let a
        # broken CMake scan kill the sweep instead of falling back to the floor.
        registered = set(registered_check_scripts(cmakelists).values())
    except (Exception, SystemExit) as reason:
        print(f"check-tests-observe: the registered checks could not be "
              f"derived from CMake ({reason!r}); falling back to the prefix "
              f"families, which are a FLOOR and not the population",
              file=sys.stderr)
    return sorted(path for path in TOOLS.glob("*.py")
                  if path.name.startswith(CHECK_PREFIXES)
                  or path.name in registered)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--tools", nargs="*",
                        help="check tools to mutate; default is every check "
                             "CMake registers, plus every verify_*/audit_*/"
                             "measure_* in tools/")
    parser.add_argument("--limit", type=int, default=40,
                        help="mutants per tool, sampled by stride across the "
                             "whole file; 0 measures every eligible mutant")
    parser.add_argument("--timeout", type=float, default=120.0)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--report", type=Path)
    arguments = parser.parse_args()

    if arguments.tools:
        candidates = [TOOLS / name for name in arguments.tools]
    else:
        candidates = candidate_tools()
    if not candidates:
        print("check-tests-observe: no check tool was found at all, so this "
              "run measured NOTHING", file=sys.stderr)
        return 1
    return sweep(candidates, arguments)


def sweep(candidates, arguments):
    results = {}
    untested = []
    for path in candidates:
        module = f"test_{path.stem}"
        if not (TOOLS / f"test_{path.name}").is_file():
            # Counted, not skipped. Every mutant survives a suite that does not
            # exist, so this is a measurement of 100% and belongs in the census.
            # Skipping it was also how deriving the candidate set from the gate
            # would otherwise ABORT the run on the first registered check whose
            # tests nobody has written.
            _, _, eligible = mutants_for(path, 0)
            results[path.name] = {
                "killed": 0, "survived": eligible, "uncompilable": 0,
                "rate": 1.0 if eligible else None, "eligible": eligible,
                "sampled": eligible, "line_span": None, "no_test_file": True,
                "examples": []}
            untested.append(path.name)
            print(f"{path.name:44} NO TEST FILE - {eligible} mutant(s), "
                  f"all unobserved by construction", flush=True)
            write_report(arguments.report, results)
            continue
        original = path.read_text(encoding="utf-8")
        if not run_tests(module, arguments.timeout):
            print(f"check-tests-observe: {module} does not pass on the "
                  f"unmutated tool, so nothing can be measured from it",
                  file=sys.stderr)
            return 1
        _, mutants, eligible = mutants_for(path, arguments.limit)
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
            discard_bytecode(path)
        measured = killed + survived
        numbers = [mutant.line_number for mutant in mutants]
        results[path.name] = {
            "killed": killed, "survived": survived, "uncompilable": broken,
            "rate": round(survived / measured, 3) if measured else None,
            "eligible": eligible, "sampled": len(mutants),
            "line_span": [min(numbers), max(numbers)] if numbers else None,
            "examples": examples}
        coverage = ("whole" if len(mutants) == eligible
                    else f"{len(mutants)}/{eligible} sampled")
        print(f"{path.name:44} killed {killed:3} survived {survived:3} "
              f"({'-' if not measured else f'{100*survived/measured:.0f}%'})"
              f"  [{coverage}]", flush=True)
        # Persist after EVERY tool. A tool costs minutes and the census costs
        # over an hour, so losing all of it to one interruption is a worse
        # failure than anything this sweep measures - and interrupting is the
        # right thing to do once the remaining tools cannot change the answer.
        write_report(arguments.report, results)

    total_killed = sum(r["killed"] for r in results.values())
    total_survived = sum(r["survived"] for r in results.values())
    measured = total_killed + total_survived
    if not measured:
        print("check-tests-observe: not one mutant was measured, so this run "
              "proves NOTHING about any check's tests", file=sys.stderr)
        return 1
    print(f"\ncheck-tests-observe: {total_killed}/{measured} mutants killed, "
          f"{total_survived} survived ({100*total_survived/measured:.0f}%)")

    if untested:
        print(f"check-tests-observe: {len(untested)} check tool(s) have NO "
              f"test file at all: {', '.join(sorted(untested))}")

    # NO SILENT CAPS. A truncated run used to print exactly what a complete run
    # printed, which is how "50% survivors" over 2.2% of verify_recovery_abi got
    # published as a property of the tool.
    partial = {name: result for name, result in results.items()
               if result["sampled"] != result["eligible"]}
    if partial:
        print(f"check-tests-observe: {len(partial)} tool(s) were SAMPLED, not "
              f"measured whole. Each rate above is an estimate from a stride "
              f"across the file; re-run with --limit 0 for a census:")
        for name, result in sorted(partial.items()):
            print(f"    {name}: {result['sampled']} of {result['eligible']} "
                  f"mutants")

    write_report(arguments.report, results)

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
