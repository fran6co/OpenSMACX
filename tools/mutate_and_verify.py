#!/usr/bin/env python3
"""Mutation-test recovered functions to prove the suites actually observe them.

AGENTS.md asks that every new suite be shown to fail when the recovery is
perturbed. Applied by hand that check is unreliable: a fixture can drive a
function over state where the recovered behaviour is unobservable (an all-zero
game table, a field the next stage overwrites), and the suite then passes for
reasons that have nothing to do with correctness.

This tool mechanises the check. For each recovered function it derives a set of
source-level mutants, rebuilds, and runs the owning test. A mutant that still
passes is a *coverage hole*: some byte of the recovery is not observed by any
assertion. Compile failures are not evidence either way and are reported
separately -- a mutant that never built has proven nothing.

Exit status is 0 only when every valid mutant was killed.
"""

from __future__ import annotations

import argparse
import os
import re
import signal
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

# Lines that are not safe or not meaningful to drop/reorder wholesale.
CONTROL_PREFIXES = (
    "return", "if", "for", "while", "else", "do", "switch", "case", "break",
    "continue", "goto", "static_assert", "#", "//", "/*", "*", "}", "{",
)

FUNCTION_HEADER = re.compile(r"Original Offset:\s*([0-9A-Fa-f]{6,8})")
# The trailing group captures a C integer suffix. Without it the `(?![\w.])`
# guard treated the `U` of `1U` as a word character and refused the whole
# match, so every suffixed literal in the recovered sources - the `+ 1U` /
# `- 1U` edge arithmetic in Buffer::box, and 97 others across src/ - was
# silently exempt from the constant operator. The suffix is carried into the
# replacement so `1U` becomes `0U` rather than `0`, keeping the expression's
# type and signedness intact.
INT_LITERAL = re.compile(
    r"(?<![\w.])(0[xX][0-9A-Fa-f]+|\d+)([uUlL]*)(?![\w.])")
INDEX_DIVISION = re.compile(
    r"\s*(?P<numerator>0[xX][0-9A-Fa-f]+|\d+)\s*/\s*"
    r"(?P<denominator>0[xX][0-9A-Fa-f]+|\d+)\s*")

# Comparison rewrites. The patterns must not fire inside `->`, `<<`, `>>` or an
# already-matched `<=`/`>=`, all of which produce uncompilable noise rather than
# a meaningful perturbation.
COMPARISONS = [
    (re.compile(r"=="), "!="),
    (re.compile(r"!="), "=="),
    (re.compile(r"<="), "<"),
    (re.compile(r">="), ">"),
    (re.compile(r"(?<![<>=!-])<(?![<=])"), "<="),
    (re.compile(r"(?<![<>=!-])>(?![>=])"), ">="),
]

ASSIGNMENT = re.compile(r"^(?P<target>[^=]+?)\s*(?:[-+*/|&^]|<<|>>)?=(?!=)")

# `static_cast<uint32_t>`, `Fixture<T>` and friends carry angle brackets that
# are not comparisons. Blanking them keeps the comparison operator off lines
# where every rewrite is guaranteed not to compile.
TEMPLATE_ANGLE = re.compile(r"\b\w+<[^<>;]*>")

# A bare increment/decrement: a write to one lvalue with no other reads.
INCREMENT = re.compile(r"^(?:\+\+|--)?(?P<target>[A-Za-z_]\w*(?:\[[^\]]*\])?)(?:\+\+|--)?;$")


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


@dataclass
class Result:
    killed: list[Mutant] = field(default_factory=list)
    survived: list[Mutant] = field(default_factory=list)
    uncompilable: list[Mutant] = field(default_factory=list)
    hung: list[Mutant] = field(default_factory=list)


def parse_functions(lines: list[str]) -> list[Function]:
    """Locate function bodies introduced by an `Original Offset:` comment.

    A body runs from the line after the signature to the first column-zero `}`.
    A signature whose brace line is itself balanced (`{ stmt; }` on one line,
    e.g. `void f() { x = y; }`) has no separable multi-line body: treating its
    next line as the start and scanning for a column-zero `}` would run past
    it into whatever follows - including the next function's doc comment and
    signature - silently fabricating a bogus, oversized span. Such one-liners
    are skipped rather than mis-parsed.
    """
    functions: list[Function] = []
    for index, line in enumerate(lines):
        match = FUNCTION_HEADER.search(line)
        if not match:
            continue
        # Skip past the remainder of the comment block and the signature.
        cursor = index
        while cursor < len(lines) and not lines[cursor].rstrip().endswith("*/"):
            cursor += 1
        cursor += 1
        # The signature may wrap across lines; the body opens at the first `{`.
        while cursor < len(lines) and "{" not in lines[cursor]:
            cursor += 1
        if cursor >= len(lines):
            continue
        brace_line = lines[cursor]
        if brace_line.count("{") == brace_line.count("}"):
            continue  # one-liner: no multi-line body to mutate
        start = cursor + 1
        end = start
        while end < len(lines) and not lines[end].startswith("}"):
            end += 1
        if start < end:
            functions.append(Function(match.group(1).upper(), start, end))
    return functions


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


def split_assignment(line: str) -> tuple[str, str, str]:
    """Return (full lvalue, base identifier, right-hand side) of a statement.

    All three are empty for anything that is not a plain assignment -- a bare
    call, for instance, whose side effects are opaque here.
    """
    stripped = line.strip()
    # `count++;` writes count and reads nothing else. Without this it falls
    # through to the opaque-call branch and every adjacent increment pair looks
    # order-dependent, reporting equivalent mutants as coverage holes.
    increment = INCREMENT.match(stripped)
    if increment:
        target = increment.group("target")
        return target, target, ""
    match = ASSIGNMENT.match(stripped)
    if not match:
        return "", "", ""
    # A subscript or member target (`ordered[0x04 / 4]`, `self->field`) is
    # never a declaration, so it is never carrying a type specifier to strip -
    # keep it verbatim (whitespace collapsed) so distinct offsets stay
    # distinct. `.split()[-1]` on a raw target instead grabs the last
    # whitespace-separated token, which for `ordered[0x04 / 4]` is the
    # meaningless `4]` - every offset with the same divisor then collapses to
    # the same fake identifier, and pairs of genuinely different array stores
    # look like a write-after-write on one lvalue.
    raw_target = match.group("target").strip()
    if any(marker in raw_target for marker in ("[", "->", ".")):
        target = re.sub(r"\s+", "", raw_target)
    else:
        # Only here can a declaration's type specifier precede the name, e.g.
        # `uint32_t *p` or `int b`. Strip it before collapsing whitespace, or
        # `int b` fuses into the single bogus identifier `intb`.
        target = raw_target.split()[-1].lstrip("*&")
        target = re.sub(r"\s+", "", target)
    identifiers = re.findall(r"[A-Za-z_]\w*", target)
    return target, (identifiers[0] if identifiers else ""), line[match.end():]


def statements_interact(first: str, second: str) -> bool:
    """True when swapping two statements could change behaviour.

    Statements that write distinct lvalues and read nothing the other writes
    are order-independent, so swapping them yields an *equivalent mutant*: it
    survives every possible suite and would be reported as a coverage hole it
    is not. Only genuine write-after-write or read-after-write pairs are worth
    spending a build on.
    """
    first_target, first_base, first_rhs = split_assignment(first)
    second_target, second_base, second_rhs = split_assignment(second)
    if not first_target or not second_target:
        return True  # An opaque call may carry side effects; keep it.
    if first_target == second_target:
        return True  # write-after-write on the same lvalue
    word = lambda name, text: bool(name) and re.search(rf"\b{re.escape(name)}\b", text)
    return bool(word(first_base, second_rhs) or word(second_base, first_rhs))


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

        # 1. Drop a store. Catches assertions that never read the field.
        if is_simple_statement(line):
            emit(index + 1, "drop-statement", f"drop `{stripped}`",
                 lines[:index] + lines[index + 1:])

        # 2. Perturb a constant. Catches fixtures driving unobservable state.
        emitted_constant_lines = set()
        for match in INT_LITERAL.finditer(line):
            literal = match.group(0)
            value = int(match.group(1), 0)
            replacement = ("1" if value == 0 else "0") + match.group(2)
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
        comparable = TEMPLATE_ANGLE.sub(lambda m: " " * len(m.group(0)), line)
        for pattern, target in COMPARISONS:
            match = pattern.search(comparable)
            if match:
                mutated_line = line[:match.start()] + target + line[match.end():]
                emit(index + 1, "comparison",
                     f"`{match.group(0)}` -> `{target}` in `{stripped}`",
                     lines[:index] + [mutated_line] + lines[index + 1:])
                break

        # 4. Swap adjacent stores. Catches unverified write ordering, but only
        #    where the two statements actually interact (see statements_interact).
        if (index + 1 < function.end and is_simple_statement(line)
                and is_simple_statement(lines[index + 1])
                and statements_interact(line, lines[index + 1])):
            emit(index + 1, "swap-adjacent", f"swap `{stripped}` with next",
                 lines[:index] + [lines[index + 1], line] + lines[index + 2:])

    return mutants


PASSED, FAILED, TIMEOUT = "passed", "failed", "timeout"
KEEP_OWNED_PREFIX_ENV = "OPENSMACX_KEEP_OWNED_WINE_PREFIX_RUNNING"


class Harness:
    def __init__(self, args: argparse.Namespace) -> None:
        self.build_dir = Path(args.build_dir).resolve()
        self.target = args.target
        self.test = args.test
        self.timeout = args.timeout
        self.reuse_owned_wine_prefix = getattr(
            args, "reuse_owned_wine_prefix", False)
        self.owned_wine_prefix_is_running = False
        # Tightened once the baseline run has been timed. A mutant that hangs
        # is a detection, and waiting the full build timeout for it wastes
        # minutes per occurrence.
        self.test_timeout = args.timeout

    def _run(self, command: list[str], cwd: Path, timeout: float) -> str:
        try:
            result = subprocess.run(command, cwd=cwd, capture_output=True,
                                    text=True, timeout=timeout)
        except subprocess.TimeoutExpired:
            return TIMEOUT
        return PASSED if result.returncode == 0 else FAILED

    def build(self) -> bool:
        return self._run(["cmake", "--build", str(self.build_dir),
                          "--target", self.target], Path.cwd(),
                         self.timeout) == PASSED

    def check(self, cleanup=False) -> str:
        """PASSED, FAILED, or TIMEOUT -- a hung mutant is not a crashed run.

        --no-tests=error is load-bearing: `ctest -R` with a pattern that
        matches nothing exits zero, which would make every mutant of a
        misspelled --test "survive" and report the whole file as one giant
        coverage hole.
        """
        keep_prefix = self.reuse_owned_wine_prefix and not cleanup
        previous = os.environ.get(KEEP_OWNED_PREFIX_ENV)
        if keep_prefix:
            os.environ[KEEP_OWNED_PREFIX_ENV] = "1"
            self.owned_wine_prefix_is_running = True
        else:
            os.environ.pop(KEEP_OWNED_PREFIX_ENV, None)
        try:
            return self._run(
                ["ctest", "--no-tests=error", "-R", self.test],
                self.build_dir, self.test_timeout)
        finally:
            if previous is None:
                os.environ.pop(KEEP_OWNED_PREFIX_ENV, None)
            else:
                os.environ[KEEP_OWNED_PREFIX_ENV] = previous

    def cleanup(self) -> None:
        """Stop a prefix retained by the opt-in fast mutation path.

        A final ordinary CTest invocation uses the same marker-protected
        runner as every other test, so it both checks the restored binary and
        stops only the build's owned prefix.
        """
        if not self.owned_wine_prefix_is_running:
            return
        self.check(cleanup=True)
        self.owned_wine_prefix_is_running = False

    def calibrate(self, elapsed: float) -> None:
        """Allow a generous multiple of the clean runtime before calling a hang."""
        self.test_timeout = max(30.0, elapsed * 10)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", help="recovered source file to mutate")
    parser.add_argument("--build-dir", default="build/mingw-i686-debug")
    parser.add_argument("--target", default="recovery-leaf-tests")
    parser.add_argument("--test", default="recovery-leaf-tests")
    parser.add_argument("--address", action="append", default=[],
                        help="restrict to these Original Offset values (repeatable)")
    parser.add_argument("--operator", action="append", default=[],
                        help="restrict to these mutation operators (repeatable)")
    parser.add_argument("--limit", type=int, default=0, help="cap mutants (0 = all)")
    parser.add_argument("--timeout", type=int, default=600)
    parser.add_argument(
        "--reuse-owned-wine-prefix", action="store_true",
        help=("keep run_windows_test.py's marker-protected Wine prefix alive "
              "between mutants, then stop it after restoring the source"))
    args = parser.parse_args()

    source = Path(args.source).resolve()
    if not source.is_file():
        print(f"error: {source} is not a file", file=sys.stderr)
        return 2

    original = source.read_text()
    lines = original.splitlines(keepends=True)
    functions = parse_functions(lines)
    if args.address:
        wanted = {value.upper().removeprefix("0X") for value in args.address}
        functions = [f for f in functions if f.address.removeprefix("0X") in wanted]
    if not functions:
        print("error: no recovered functions matched", file=sys.stderr)
        return 2

    mutants: list[Mutant] = []
    for function in functions:
        mutants.extend(build_mutants(lines, function))
    if args.operator:
        mutants = [m for m in mutants if m.operator in set(args.operator)]
    if args.limit:
        mutants = mutants[:args.limit]

    harness = Harness(args)

    # Restore the file whatever happens -- an interrupted run must not leave a
    # mutant on disk.
    def restore(*_: object) -> None:
        source.write_text(original)

    signal.signal(signal.SIGINT, lambda *a: (restore(), sys.exit(130)))
    signal.signal(signal.SIGTERM, lambda *a: (restore(), sys.exit(143)))

    result = Result()
    try:
        print(f"baseline: building {args.target}", flush=True)
        if not harness.build():
            print("error: baseline build failed; fix the tree first", file=sys.stderr)
            return 2
        print(f"baseline: running {args.test}", flush=True)
        started = time.monotonic()
        if harness.check() != PASSED:
            print("error: baseline test already fails; every mutant would look "
                  "killed", file=sys.stderr)
            return 2
        harness.calibrate(time.monotonic() - started)
        print(f"baseline clean in {time.monotonic() - started:.1f}s; "
              f"hang threshold {harness.test_timeout:.0f}s", flush=True)

        print(f"\n{len(mutants)} mutants across {len(functions)} function(s)\n",
              flush=True)
        for index, mutant in enumerate(mutants, start=1):
            source.write_text("".join(mutant.lines))
            label = f"[{index}/{len(mutants)}] {mutant.address}:{mutant.line_number} {mutant.operator}"
            if not harness.build():
                result.uncompilable.append(mutant)
                print(f"{label} -- skipped (no compile)", flush=True)
                continue
            status = harness.check()
            if status == PASSED:
                result.survived.append(mutant)
                print(f"{label} -- SURVIVED: {mutant.description}", flush=True)
            elif status == TIMEOUT:
                # The perturbation hung the suite: observed, therefore killed.
                result.killed.append(mutant)
                result.hung.append(mutant)
                print(f"{label} -- killed (hang)", flush=True)
            else:
                result.killed.append(mutant)
                print(f"{label} -- killed", flush=True)
    finally:
        restore()
        harness.build()
        harness.cleanup()

    valid = len(result.killed) + len(result.survived)
    print("\n" + "=" * 72)
    print(f"measured against `{args.test}` only -- a survivor here is unobserved "
          f"by\nthat suite, not necessarily by every gate (the in-process "
          f"runtime\noracle runs separately, under the hybrid smoke target).")
    print("-" * 72)
    print(f"killed      {len(result.killed)}/{valid}")
    print(f"survived    {len(result.survived)}")
    print(f"no compile  {len(result.uncompilable)} (proves nothing)")
    if result.hung:
        print(f"  of the killed, {len(result.hung)} hung rather than failed an assertion")
    if result.survived:
        print("\nCOVERAGE HOLES -- these perturbations are not observed:")
        for mutant in result.survived:
            print(f"  {mutant.address}:{mutant.line_number} [{mutant.operator}] "
                  f"{mutant.description}")
    print("=" * 72)
    return 1 if result.survived else 0


if __name__ == "__main__":
    sys.exit(main())
