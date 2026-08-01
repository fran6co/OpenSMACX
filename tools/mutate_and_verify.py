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

SHARDING, because this is the slowest thing in a recovery. A sweep is one
rebuild-and-run per mutant, strictly serial, and mutant count scales with
function size -- measured at roughly one per 18-22 bytes, so ~90 for a 1.5 KB
body and ~103 for a 2.2 KB one. At ~3.5 s each that is five to six minutes per
pass, and hardening a new test runs several passes. It dominates.

The mutants are INDEPENDENT of one another. They are serial only because they
all write the same source file and the same binary. `--shard I/N` splits them,
so N workers can sweep one function together. Measured 2026-07-31 on
?stack_veh@@YAHHH@Z, 88 mutants:

    serial, one build directory        313 s
    four shards, four lanes             99 s     3.16x, 79% efficiency

with byte-identical verdicts -- the same 75 killed, the same 2 survivors.

EACH WORKER NEEDS ITS OWN SOURCE TREE. This script edits the source in place
and reverts afterwards, so shards sharing a tree would overwrite each other's
mutations and score nonsense. A lane costs about seven seconds to build and
2 GB of disk:

    git archive HEAD | tar -x -C "$S" --one-top-level=lane$i
    mkdir -p "$S/lane$i/.opensmacx"
    cp -al <repo>/.opensmacx/game "$S/lane$i/.opensmacx/game"   # hard links
    cd "$S/lane$i" && cmake -S . -B build -G Ninja \
        --toolchain cmake/toolchains/mingw-i686.cmake
    cmake --build build --target <suite> -j4

Hard links, not a symlink: the build refuses a symlinked OPENSMACX_GAME_DIR and
refuses one outside the lane's own ignored root, and hard links satisfy both at
no cost. The Wine prefix is already ${CMAKE_CURRENT_BINARY_DIR}/wineprefix, so
per-lane build directories isolate it for free.

A SHARD IS NOT A SWEEP, and the run says so on every invocation. Each shard
prints its own tally, and `survived 0` from one shard looks exactly like a clean
full sweep. All N shards must run and their survivors be unioned before a
function has been swept; exit status from one shard means only that its own
share was killed.

Disk binds before CPU: at ~2 GB per lane, four lanes is a comfortable default
and the per-mutant cost had risen only 3.5 s -> 4.5 s at that width, so it was
not yet saturated.
"""

from __future__ import annotations

import argparse
import re
import signal
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from runtime_process import stop_executable_processes  # noqa: E402

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
    # Mutants whose build succeeded without replacing the binary. Counted
    # separately and never folded into killed or survived, for the same reason
    # `uncompilable` is: they are evidence of nothing.
    stale: list[Mutant] = field(default_factory=list)
    # Mutants the suite let through once and then killed on a re-run. Counted so
    # the flake RATE is published rather than absorbed: these are the mutants
    # that made two runs of an identical tree disagree.
    flaky: list[Mutant] = field(default_factory=list)


def parse_shard(value: str | None) -> tuple[int, int] | None:
    """`I/N` -> (I-1, N), or None. Raises SystemExit on anything malformed.

    ONE-BASED ON THE COMMAND LINE, zero-based internally. `--shard 1/4` is the
    first of four, because a caller writing a loop over `1 2 3 4` and getting
    silent nonsense from `--shard 4/4` would have no way to notice: every shard
    reports its own clean-looking tally, so an off-by-one costs coverage without
    costing a green run. Malformed input is fatal for the same reason - there is
    no safe default when the alternative is silently sweeping a quarter of a
    function and reading it as the whole.
    """
    if value is None:
        return None
    try:
        index, count = (int(part) for part in value.split("/", 1))
    except ValueError:
        raise SystemExit(f"error: --shard wants I/N, got {value!r}")
    if count < 1 or not 1 <= index <= count:
        raise SystemExit(f"error: --shard {value} is out of range; "
                         f"I must be 1..N and N at least 1")
    return index - 1, count


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


DECLARATION = re.compile(
    r"^\s*(?:const\s+|static\s+|volatile\s+|constexpr\s+|auto\s+)*"
    r"[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*(?:=|\{|\()")


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
        comparable = TEMPLATE_ANGLE.sub(lambda m: " " * len(m.group(0)),
                                        code_line)
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
                and not structural
                and not declares_name_used_later(lines, index + 1, function.end)
                and is_simple_statement(lines[index + 1])
                and statements_interact(line, lines[index + 1])):
            emit(index + 1, "swap-adjacent", f"swap `{stripped}` with next",
                 lines[:index] + [lines[index + 1], line] + lines[index + 2:])

    return mutants


PASSED, FAILED, TIMEOUT = "passed", "failed", "timeout"
# A fourth build outcome, and it is not a kind of failure. STALE means the build
# reported success and yet the test binary on disk did not change, so whatever
# CTest ran afterwards was the PREVIOUS mutant's executable. That result is
# UNMEASURED - neither killed nor survived - and it is the leading suspect for
# full-file sweeps reporting different coverage holes on identical trees.
STALE = "stale"


class Harness:
    def __init__(self, args: argparse.Namespace) -> None:
        self.build_dir = Path(args.build_dir).resolve()
        self.target = args.target
        self.test = args.test
        self.timeout = args.timeout
        artifact = getattr(args, "artifact", None)
        self.artifact = (Path(artifact).resolve() if artifact
                         else self.build_dir / f"{self.target}.exe")
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

    def _artifact_identity(self):
        """(mtime, size, inode), or None when the binary is not there.

        Inode as well as mtime and size: a toolchain that writes to a temporary
        and renames it over the target can leave mtime and size unchanged while
        genuinely replacing the file, and reading that as STALE would report a
        real measurement as unmeasured.
        """
        try:
            stat = self.artifact.stat()
        except OSError:
            return None
        return (stat.st_mtime_ns, stat.st_size, stat.st_ino)

    def build(self) -> str:
        """PASSED, FAILED, or STALE -- a build can succeed and change nothing.

        The sweep's whole chain of evidence is "this source produced this
        binary, which the suite then ran". Nothing checked the middle link. If
        the link fails while the build still exits zero, CTest runs the previous
        mutant's executable and the harness records whatever that does - a false
        SURVIVED if the previous mutant was equivalent, a false killed if not.
        """
        before = self._artifact_identity()
        status = self._run(["cmake", "--build", str(self.build_dir),
                            "--target", self.target], Path.cwd(),
                           self.timeout)
        if status != PASSED:
            return FAILED
        after = self._artifact_identity()
        if after is None:
            return STALE
        return PASSED if before is None or after != before else STALE

    def check(self) -> str:
        """PASSED, FAILED, or TIMEOUT -- a hung mutant is not a crashed run.

        --no-tests=error is load-bearing: `ctest -R` with a pattern that
        matches nothing exits zero, which would make every mutant of a
        misspelled --test "survive" and report the whole file as one giant
        coverage hole.

        There is deliberately no owned-prefix reuse here. The opt-in that used
        to keep the Wine session alive between mutants made this call FOUR
        TIMES SLOWER (measured 1.33 s against 0.34 s), because the retained
        wineserver and its service processes hold the write end of CTest's
        output pipe and CTest reads that pipe to EOF. See
        tools/run_windows_test.py.
        """
        # -FS excludes the build-freshness setup fixture. CTest would otherwise
        # pull it in automatically for any selected test that requires it, and
        # a fixture failure turns the test into `***Not Run` - which this
        # harness would read as the mutant being KILLED. A false kill is the
        # one outcome worse than a false survivor, and the sweep already has a
        # stronger check of its own: it compares the artifact's identity before
        # and after the build and reports STALE when the binary was not
        # replaced.
        status = self._run(
            ["ctest", "--no-tests=error", "-FS", "build_fresh",
             "-R", self.test],
            self.build_dir, self.test_timeout)
        if status == TIMEOUT:
            # subprocess's timeout kills CTest and nothing else, so the
            # wine process running the test binary outlives it. Unlike
            # smoke_hybrid_game.py, this harness had no reaping step at
            # all - so an orphan could still hold the executable while the
            # next mutant tried to link over it. Reap the ones we own, by
            # the artifact path, never a global shutdown.
            self.reap()
        return status

    def reap(self) -> bool:
        """Stop processes still running THIS harness's test binary.

        Matched on the artifact path, so a sibling agent's copy of the same
        executable elsewhere is not touched, and never a `wineserver -k` over
        anything but what this harness started.
        """
        try:
            return stop_executable_processes(self.artifact)
        except Exception as error:                      # noqa: BLE001
            print(f"warning: could not reap {self.artifact.name}: {error}",
                  flush=True)
            return False

    def confirm_survivor(self, attempts: int) -> str:
        """Re-run a mutant the suite just let through, without rebuilding it.

        A survivor is the claim this harness makes that costs the most - it says
        some behaviour of a recovery is unobserved - and it is also the outcome
        that has been measured to be unreliable. Four sweeps over an identical
        tree (361 mutants each, 2026-07-30) disagreed on 5 of 361 indices, 1.39%,
        and every single disagreement was in the SAME DIRECTION: three runs
        killed the mutant and one let it through. Different mutants flipped in
        different runs, so it is a low-rate flake and not a set of weak
        fixtures. All five perturb a memory offset or a pointer guard, where
        whether the perturbation is observable depends on fixture memory the test
        does not pin.
        `STALE` was zero across all four runs, which retires the stale-binary
        mechanism `docs/HANDOVER.md` had recorded as the likely cause.

        Because the flake only ever manufactures survivors, re-observing a
        survivor is enough to remove it, and it is nearly free: the binary is
        already built, so a confirmation costs one test run, and there are
        usually a couple of dozen survivors in a sweep.
        """
        for _ in range(max(0, attempts)):
            status = self.check()
            if status != PASSED:
                return status
        return PASSED

    def calibrate(self, elapsed: float) -> None:
        """Allow a generous multiple of the clean runtime before calling a hang."""
        self.test_timeout = max(30.0, elapsed * 10)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", help="recovered source file to mutate")
    parser.add_argument("--build-dir", default="build/mingw-i686-debug")
    parser.add_argument("--shard", metavar="I/N",
                        help="run only mutants whose index %% N == I-1, so N "
                             "workers in N SEPARATE SOURCE TREES can sweep one "
                             "function together. Each worker needs its own tree "
                             "and its own --build-dir: this script mutates the "
                             "source in place, so shards sharing a tree would "
                             "overwrite each other. See the module docstring.")
    parser.add_argument("--target", default="recovery-leaf-tests")
    parser.add_argument("--test", default="recovery-leaf-tests")
    parser.add_argument("--address", action="append", default=[],
                        help="restrict to these Original Offset values (repeatable)")
    parser.add_argument("--operator", action="append", default=[],
                        help="restrict to these mutation operators (repeatable)")
    parser.add_argument("--limit", type=int, default=0, help="cap mutants (0 = all)")
    parser.add_argument("--timeout", type=int, default=600)
    parser.add_argument(
        "--confirm-survivors", type=int, default=2, metavar="N",
        help=("re-run a surviving mutant N more times before reporting it as a "
              "coverage hole (0 disables). Costs one test run per survivor and "
              "removes the measured 1.39%% false-survivor rate"))
    parser.add_argument(
        "--artifact",
        help=("the test binary the build must replace for a mutant to count as "
              "measured; defaults to <build-dir>/<target>.exe"))
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
    # getattr, not args.shard: callers that build a Namespace directly (the
    # test suite does) predate this flag, and a new optional argument must not
    # break them.
    shard = parse_shard(getattr(args, "shard", None))
    if shard:
        index, count = shard
        whole = len(mutants)
        mutants = [m for n, m in enumerate(mutants) if n % count == index]
        print(f"SHARD {index + 1} OF {count}: {len(mutants)} of this "
              f"function's {whole} mutant(s). THIS RUN IS NOT A SWEEP - its "
              f"'survived 0' means only that no mutant IN THIS SHARD survived. "
              f"All {count} shards must be run and their survivors unioned "
              f"before the function has been swept.")

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
        if harness.build() == FAILED:
            print("error: baseline build failed; fix the tree first", file=sys.stderr)
            return 2
        # The baseline is the one build allowed to be STALE - an already-current
        # tree legitimately rebuilds nothing - but the artifact has to EXIST, or
        # every later staleness check is comparing None against None and
        # silently answers "fresh".
        if not harness.artifact.is_file():
            print(f"error: {harness.artifact} does not exist after the baseline "
                  f"build; pass --artifact if the target writes elsewhere",
                  file=sys.stderr)
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
            built = harness.build()
            if built == FAILED:
                result.uncompilable.append(mutant)
                print(f"{label} -- skipped (no compile)", flush=True)
                continue
            if built == STALE:
                result.stale.append(mutant)
                print(f"{label} -- STALE: the build reported success without "
                      f"replacing {harness.artifact.name}; this mutant is "
                      f"UNMEASURED", flush=True)
                continue
            status = harness.check()
            if status == PASSED and args.confirm_survivors > 0:
                # Re-observe before claiming a coverage hole. Measured flake
                # rate 1.39% per sweep, always in this direction.
                status = harness.confirm_survivor(args.confirm_survivors)
                if status != PASSED:
                    result.flaky.append(mutant)
                    result.killed.append(mutant)
                    print(f"{label} -- killed on re-run (the suite let it "
                          f"through once: FLAKY)", flush=True)
                    continue
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

    valid = len(result.killed) + len(result.survived)
    print("\n" + "=" * 72)
    print(f"measured against `{args.test}` only -- a survivor here is unobserved "
          f"by\nthat suite, not necessarily by every gate (the in-process "
          f"runtime\noracle runs separately, under the hybrid smoke target).")
    print("-" * 72)
    print(f"killed      {len(result.killed)}/{valid}")
    print(f"survived    {len(result.survived)}")
    print(f"no compile  {len(result.uncompilable)} (proves nothing)")
    if result.stale:
        print(f"STALE       {len(result.stale)} (UNMEASURED - the build "
              f"reported success\n            without replacing the binary, so "
              f"the suite ran the\n            previous mutant. These are not "
              f"survivors and not kills.)")
    # Per operator, because the totals hide the case that matters most: an
    # operator that generated no compilable mutant at all tested NOTHING, and
    # summed into `killed n/n` that is indistinguishable from having tested
    # everything. GraphicWin::init read as fully covered on exactly that
    # confusion until the swap operator was re-run alone (commit 2aa199f).
    operators = sorted({mutant.operator for mutant in mutants})
    if operators:
        print("-" * 72)
        for operator in operators:
            counted = lambda bucket: sum(
                1 for mutant in bucket if mutant.operator == operator)
            killed, survived = counted(result.killed), counted(result.survived)
            note = ""
            if killed + survived == 0:
                note = ("   <- generated no compilable mutant: this property "
                        "is UNTESTED,\n" + " " * 8 + "not tested-and-equivalent")
            print(f"  {operator:<16} killed {killed:<5} survived {survived:<5} "
                  f"no compile {counted(result.uncompilable)}{note}")
    if result.hung:
        print(f"  of the killed, {len(result.hung)} hung rather than failed an assertion")
    if result.flaky:
        print(f"  of the killed, {len(result.flaky)} were let through ONCE and "
              f"killed on a re-run.\n  Those are the mutants that make two "
              f"sweeps of one tree disagree; without\n  --confirm-survivors "
              f"they would have been reported as coverage holes.")
    if result.survived:
        print("\nCOVERAGE HOLES -- these perturbations are not observed:")
        for mutant in result.survived:
            print(f"  {mutant.address}:{mutant.line_number} [{mutant.operator}] "
                  f"{mutant.description}")
    print("=" * 72)
    # Zero measured mutants is NO SIGNAL, and must not exit 0. An empty sweep
    # prints `killed 0/0`, an empty survivor list and a clean exit, which reads
    # exactly like total coverage - the same confusion the per-operator block
    # above exists to break, one level up. A body whose only statement is a
    # guard clause carries no literal and no comparison, so nothing is
    # generated; HANDOVER.md already calls that "no signal, not a clean sweep",
    # and the commit that follows says "swept, 0 survivors".
    measured = len(result.killed) + len(result.survived)
    if measured == 0:
        if shard:
            # Legitimate: N above the mutant count leaves some shards empty.
            # Still not a pass - the sweep is unmeasured until all N have run
            # and their survivors are unioned.
            print("NO SIGNAL: this shard measured no mutant. That is expected "
                  "when N\nexceeds the mutant count, but it is not coverage. "
                  "The function is\nunswept until every shard has run.")
        else:
            print("NO SIGNAL: not one mutant was measured, so this run proves "
                  "NOTHING\nabout the body. `killed 0/0` is not a clean sweep. "
                  "Check that the\noperators can see this function at all.")
        return 1
    # A STALE mutant fails the run as surely as a survivor does. Not because the
    # recovery is wrong - nothing was learned about it either way - but because
    # "N/N killed" must never be printable over a sweep that silently skipped
    # some of its own mutants.
    return 1 if result.survived or result.stale else 0


if __name__ == "__main__":
    sys.exit(main())
