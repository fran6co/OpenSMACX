#!/usr/bin/env python3
"""Unit tests for the harness that tests the harness.

`lifted_oracle_mutate.py` is the project's only evidence that an oracle PASS
means anything: it damages a lifted function on purpose and checks the oracle
says so. Everything downstream of it - the pass rate, the coverage headline -
inherits its credibility, so a bug here does not produce a wrong number, it
produces a *confident* wrong number. It has already had one: the verdict was
scraped from the end-of-run summary, which cannot tell
INCONCLUSIVE-lifted-looped-forever (the mutant WAS detected) from
INCONCLUSIVE-original-faulted (says nothing at all), and a detected mutant was
scored MISSED.

Nothing here runs Wine. Every test drives the pure logic that surrounds the
subprocess call, because that is where all three known defects lived.

What is worth asserting, and why each one is a place a wrong answer still looks
like a right one:

* THE CLASSIFIER MUST BE TOTAL AND LOUD. `verdict not in CAUGHT` therefore
  MISSED is a default that turns every verdict string the script has not been
  taught into an accusation against the oracle - and a newly added verdict is
  exactly when nobody re-reads the classifier. So the verdict strings are
  extracted from `oracle_verdict_name` in tools/lifted_oracle.cpp and every one
  of them must appear in the table; an unseen string must raise.
* THE TWO INCONCLUSIVES MUST DISAGREE. `INCONCLUSIVE-lifted-budget` means the
  mutation stopped the lifted body terminating: a detection. Every
  ORIGINAL-side inconclusive means the case never became a comparison: no
  evidence, in either direction. One answer for both is the original bug, and
  a test that only checks "the classifier returns a string" cannot see it.
* A MUTATION THAT MATCHES NOTHING IS UNTESTED, NOT EQUIVALENT. If a regex stops
  matching - because the lowerer renamed an emitted helper, say - the mutation
  kind silently vanishes from the run and the score stays clean. So each
  mutation is applied to C++ produced by the REAL lowerer, `x86_lower.lower`,
  on real encoded x86: if the emitted shape drifts, these tests fail rather
  than the property quietly disappearing. Writing the fixtures by hand from the
  regexes would have proved only that the regexes match themselves.
* AND IT MUST ALSO CHANGE SOMETHING. A replacement that puts back identical
  text is worse than a no-match: the mutant compiles to the byte-identical
  function, the oracle rightly says PASS, and the run records a MISS against an
  oracle that was handed nothing to find.
* THE CANARY MUST NOT BE A SECOND MUTATION. It only prepends a trap; if it also
  removed the statement, "reached" would be measuring a different program from
  the one the mutant runs.
* BODY-SPAN MUST CUT THE RIGHT FUNCTION. Shards hold many functions and the
  mutation is applied to the sliced body, so a span that runs to the wrong
  closing brace damages a NEIGHBOUR - and the oracle then reports PASS for a
  function whose text is untouched.
"""
import contextlib
import io
import re
import sys
import tempfile
import unittest
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

import lifted_oracle_mutate as mutate
import x86_lower

BASE = 0x00401000
ORACLE_SOURCE = Path(__file__).resolve().parent / "lifted_oracle.cpp"

DECODER = Cs(CS_ARCH_X86, CS_MODE_32)
DECODER.detail = True


def lowered(encoded, address=BASE, branch=False):
    """The C++ the real lowerer emits for one encoded x86 instruction."""
    instructions = list(DECODER.disasm(bytes.fromhex(encoded), address))
    if not instructions:
        raise AssertionError(f"{encoded} did not decode")
    label_for = (lambda target: f"L_{target:08x}") if branch else (lambda _: None)
    return x86_lower.lower(instructions[0], label_for, None)


def shard_text(functions):
    """A shard laid out exactly as `lift_whole_image.write_shards` lays one out.

    Same banner comment, same signature, same four-space indent, same trailing
    `opensmacx_trap` and closing brace - because `body_span` finds its function
    by that signature and its end by that brace, and a fixture that only
    resembles the generator would be testing a shape nothing produces.
    """
    parts = ['#include "lifted_runtime.h"\n\n']
    for address, name, statements in functions:
        parts.append(
            f"// {name}\n"
            f"// {address:#010x}-{address + 0x20:#010x}  32 bytes, "
            f"{len(statements)} instructions\n"
            f"void lifted_{address:08x}(OpensmacxStaticRecompileState &s) {{\n")
        parts.append("\n".join("    " + statement for statement in statements))
        parts.append(f"\n    opensmacx_trap({address:#010x}U,"
                     f" \"fell off the end of the body\");\n}}\n\n")
    return "".join(parts)


def body_of(text, address):
    span = mutate.body_span(text, address)
    if span is None:
        raise AssertionError(f"no body for {address:#010x}")
    return text[span[0]:span[1]]


# One real x86 instruction per mutation, keyed by the mutation's own regex
# source, plus what the mutated text must and must not contain afterwards. The
# expectations are written from the mutation's NAME - "flip a branch condition"
# has to produce the opposite condition - so a replacement that changes the
# right line in the wrong way still fails.
FIXTURES = {
    r"if \(opensmacx_zf\(s\)\) goto": dict(
        encoded="7410", branch=True,            # je short
        expect=["if (!opensmacx_zf(s)) goto L_00401012;"],
        forbid=["if (opensmacx_zf(s)) goto"]),
    r"if \(!opensmacx_zf\(s\)\) goto": dict(
        encoded="7510", branch=True,            # jne short
        expect=["if (opensmacx_zf(s)) goto L_00401012;"],
        forbid=["if (!opensmacx_zf(s)) goto"]),
    r"^(\s*)opensmacx_store32\(": dict(
        encoded="8903",                         # mov dword ptr [ebx], eax
        expect=["\n    if (false) opensmacx_store32(s.ebx, s.eax);"],
        forbid=["\n    opensmacx_store32("]),
    r"s\.esi = opensmacx_mem32": dict(
        encoded="8b33",                         # mov esi, dword ptr [ebx]
        expect=["s.edi = opensmacx_mem32(s.ebx);"],
        forbid=["s.esi = opensmacx_mem32"]),
    r"\+ 0x00000008U\)": dict(
        encoded="8b4308",                       # mov eax, dword ptr [ebx + 8]
        expect=["s.eax = opensmacx_mem32(s.ebx + 0x0000000cU);"],
        forbid=["+ 0x00000008U)"]),
    r", 0x00000001U\);": dict(
        encoded="83c001",                       # add eax, 1
        expect=["s.eax = opensmacx_add32(s, s.eax, 0x00000002U);"],
        forbid=[", 0x00000001U);"]),
    r"opensmacx_sar32": dict(
        encoded="d1f8",                         # sar eax, 1
        expect=["opensmacx_shr32(s, s.eax, 0x00000001U);"],
        forbid=["opensmacx_sar32"]),
    r"^(\s*)\(void\)opensmacx_sub32\(": dict(
        encoded="39d8",                         # cmp eax, ebx
        expect=["\n    if (false) (void)opensmacx_sub32(s, s.eax, s.ebx);"],
        forbid=["\n    (void)opensmacx_sub32("]),
    r"opensmacx_x87_binary_st0\(OpensmacxX87Mul,": dict(
        encoded="d8c9",                         # fmul st(1)
        expect=["opensmacx_x87_binary_st0(OpensmacxX87Add,"],
        forbid=["OpensmacxX87Mul"]),
    r"^(\s*)opensmacx_x87_fld32\(": dict(
        encoded="d945f4",                       # fld dword ptr [ebp - 0xc]
        expect=["\n    if (false) opensmacx_x87_fld32(s.ebp + 0xfffffff4U);"],
        forbid=["\n    opensmacx_x87_fld32("]),
    r"opensmacx_x87_fst32\(s\.ebp \+ 0xfffffff4U": dict(
        encoded="d955f4",                       # fst dword ptr [ebp - 0xc]
        expect=["opensmacx_x87_fst32(s.ebp + 0xfffffff0U, false);"],
        forbid=["opensmacx_x87_fst32(s.ebp + 0xfffffff4U"]),
    r"opensmacx_mem8\(": dict(
        encoded="0fb600",                       # movzx eax, byte ptr [eax]
        expect=["s.eax = opensmacx_mem16(s.eax);"],
        forbid=["opensmacx_mem8("]),
}


def oracle_verdict_names():
    """Every string `oracle_verdict_name` in the C++ oracle can return.

    Read out of the committed source rather than copied here, so that adding a
    verdict to the oracle fails this file until somebody decides, deliberately,
    whether the new verdict is a detection.
    """
    text = ORACLE_SOURCE.read_text(encoding="utf-8", errors="replace")
    start = text.find("const char *oracle_verdict_name(")
    if start < 0:
        raise AssertionError(f"oracle_verdict_name is not in {ORACLE_SOURCE}; "
                             f"this test has lost track of the oracle")
    end = text.find("\n}\n", start)
    names = set(re.findall(r'return\s+"([^"]+)";', text[start:end]))
    # A silent extraction failure would make every assertion below vacuous.
    if len(names) < 8:
        raise AssertionError(f"only found {sorted(names)} in oracle_verdict_name")
    return names


DRIVER_SOURCE = Path(__file__).resolve().parent / "lifted_oracle_main.cpp"


def report_verdict_literals():
    """Verdict strings the driver writes into a report row DIRECTLY.

    `oracle_verdict_name` is not the only writer. The driver emits a bare
    "SKIP" row for every function the plan disqualifies, and that string never
    passes through the verdict enum - so scanning only `oracle_verdict_name`
    left a second writer entirely uncovered, and `classify("SKIP", ...)` used
    to raise UnknownVerdict on it.

    The row format is `address<TAB>verdict<TAB>...`, so a literal verdict is
    whatever sits between the first two tabs of a report format string.
    """
    text = DRIVER_SOURCE.read_text(encoding="utf-8", errors="replace")
    literals = set()
    for line in re.findall(r'fprintf\(report,\s*\n?\s*"([^"]+)"', text):
        fields = line.split("\\t")
        if len(fields) < 2 or fields[0] != "%#010x":
            continue
        if "%" in fields[1]:            # a runtime verdict, not a literal
            continue
        literals.add(fields[1])
    if not literals:
        raise AssertionError(
            f"found no literal verdict rows in {DRIVER_SOURCE}; this test has "
            f"lost track of the driver's report writer")
    return literals


class VerdictClassificationTests(unittest.TestCase):
    """Every verdict the oracle can utter maps to exactly one answer."""

    def test_every_verdict_the_oracle_can_emit_is_in_the_table(self):
        missing = oracle_verdict_names() - set(mutate.VERDICT_MEANING)
        self.assertEqual(set(), missing,
                         "the oracle can print these and the mutation runner "
                         "would silently score them MISSED")

    def test_every_verdict_the_driver_writes_directly_is_in_the_table_too(self):
        """`oracle_verdict_name` is not the only writer of a verdict column.

        The driver prints a bare "SKIP" row for every plan-disqualified
        function without going through the enum, so a scan of
        `oracle_verdict_name` alone declared the cross-language contract
        satisfied while a second writer went uncovered. The guarantee this
        file advertises - "adding a verdict to the C++ fails the Python" -
        only holds if both writers are read.
        """
        literals = report_verdict_literals()
        self.assertIn("SKIP", literals,
                      "the driver no longer writes a literal SKIP row; if the "
                      "row shape changed, update the extractor")
        missing = literals - set(mutate.VERDICT_MEANING)
        self.assertEqual(set(), missing,
                         "the driver writes these verdict strings and the "
                         "mutation runner would raise UnknownVerdict on them")

    def test_a_skipped_function_is_no_evidence_about_the_mutant(self):
        # The oracle never ran the function, so the mutant was never judged.
        # The alternative - raising - would abort a whole run on a row that
        # simply says "not looked at".
        self.assertEqual("unusable", mutate.classify("SKIP", True))
        self.assertNotIn("SKIP", mutate.CAUGHT)

    def test_every_verdict_the_oracle_can_emit_classifies(self):
        for verdict in sorted(oracle_verdict_names()):
            with self.subTest(verdict=verdict):
                self.assertIn(mutate.classify(verdict, True),
                              {"caught", "missed", "unreached", "unusable"})

    def test_an_unseen_verdict_raises_instead_of_being_bucketed(self):
        # The shape of the next oracle change: a new FAIL- flavour. Defaulting
        # it to MISSED would report a hole in the oracle that does not exist.
        with self.assertRaises(mutate.UnknownVerdict) as caught:
            mutate.classify("FAIL-lifted-desynchronised", True)
        self.assertIn("FAIL-lifted-desynchronised", str(caught.exception))

    def test_the_two_inconclusive_kinds_get_different_answers(self):
        # THE bug, in one assertion. Both printed as `INCONCLUSIVE 1`.
        self.assertNotEqual(
            mutate.classify("INCONCLUSIVE-lifted-budget", True),
            mutate.classify("INCONCLUSIVE-original-fault", True))

    def test_a_lifted_side_inconclusive_is_a_detection(self):
        # The mutation stopped the lifted body terminating, or walked it out of
        # its own span. Either way the oracle noticed.
        self.assertEqual("caught",
                         mutate.classify("INCONCLUSIVE-lifted-budget", True))
        self.assertEqual("caught",
                         mutate.classify("INCONCLUSIVE-lifted-out-of-span", True))

    def test_an_original_side_inconclusive_is_never_a_miss(self):
        # The original faulted, or timed out, or leaned on the unmodellable top
        # page. The mutant was never judged, so it cannot be evidence against
        # the oracle - whatever the canary said.
        for verdict in ("INCONCLUSIVE-original-fault",
                        "INCONCLUSIVE-original-timeout",
                        "INCONCLUSIVE-original-top-page"):
            for reached in (True, False, None):
                with self.subTest(verdict=verdict, reached=reached):
                    self.assertEqual("unusable",
                                     mutate.classify(verdict, reached))

    def test_a_plain_fail_and_a_lifted_fault_are_both_detections(self):
        self.assertEqual("caught", mutate.classify("FAIL", True))
        self.assertEqual("caught", mutate.classify("FAIL-lifted-fault", True))

    def test_a_trap_is_a_detection_because_the_canary_relies_on_it(self):
        # `reached` is computed as `run_oracle(...) in CAUGHT`, and the canary
        # is an opensmacx_trap. If SKIP-trap stopped counting as a detection,
        # every canary would report unreached and every genuine MISS would be
        # filed as a coverage gap.
        self.assertEqual("caught", mutate.classify("SKIP-trap", True))
        self.assertIn("SKIP-trap", mutate.CAUGHT)

    def test_a_pass_on_a_reached_line_is_a_miss(self):
        self.assertEqual("missed", mutate.classify("PASS", True))

    def test_a_pass_on_a_line_no_seed_reaches_is_not_a_miss(self):
        # A coverage figure, not an oracle figure.
        self.assertEqual("unreached", mutate.classify("PASS", False))

    def test_a_pass_with_no_canary_answer_is_still_a_miss(self):
        # `reached is None` means the canary build never ran. Treating "we did
        # not check" as "not reached" would let real misses be excused.
        self.assertEqual("missed", mutate.classify("PASS", None))

    def test_a_mutant_that_did_not_compile_is_not_evidence(self):
        self.assertEqual("unusable", mutate.classify("DID-NOT-COMPILE", True))

    def test_an_oracle_that_wrote_no_report_is_not_evidence(self):
        # No row means the oracle never answered. Scoring it MISSED invents a
        # hole out of a crashed subprocess.
        self.assertEqual("unusable", mutate.classify("NO-VERDICT", True))
        self.assertEqual("unusable", mutate.classify("UNRUN", True))

    def test_the_caught_set_is_derived_from_the_table(self):
        # Two hand-maintained lists of detections is one too many.
        self.assertEqual(
            {name for name, meaning in mutate.VERDICT_MEANING.items()
             if meaning == "detected"},
            mutate.CAUGHT)

    def test_pass_is_the_only_verdict_that_can_ever_be_a_miss(self):
        for verdict in mutate.VERDICT_MEANING:
            if verdict == "PASS":
                continue
            with self.subTest(verdict=verdict):
                self.assertNotEqual("missed", mutate.classify(verdict, True))


class ReportReadingTests(unittest.TestCase):
    """The verdict comes from the row, not from the prose around it."""

    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.addCleanup(self.dir.cleanup)
        self.report = Path(self.dir.name) / "mutate_one.tsv"

    def test_the_verdict_is_the_second_column_of_the_row(self):
        self.report.write_text(
            "address\tverdict\tcases\tcompared\tdetail\tname\n"
            "0x00401000\tFAIL\t16\t15\t\t??0StringStruct@@QAE@H@Z\n")
        self.assertEqual("FAIL", mutate.verdict_from_report(self.report))

    def test_a_summary_in_the_same_file_cannot_change_the_answer(self):
        # The original defect: the summary says INCONCLUSIVE, the row says the
        # lifted side blew its budget, and only the row knows which side it was.
        self.report.write_text(
            "address\tverdict\tcases\tcompared\tdetail\tname\n"
            "0x00401000\tINCONCLUSIVE-lifted-budget\t16\t0\t\t?f@@YAXXZ\n"
            "\nPASSED 0\nINCONCLUSIVE 1\nFAILED (compare) 0\n")
        self.assertEqual("INCONCLUSIVE-lifted-budget",
                         mutate.verdict_from_report(self.report))
        self.assertEqual("caught", mutate.classify(
            mutate.verdict_from_report(self.report), True))

    def test_a_missing_report_is_no_verdict(self):
        self.assertEqual("NO-VERDICT", mutate.verdict_from_report(self.report))

    def test_a_header_only_report_is_no_verdict(self):
        # The oracle started, wrote its header, and died. That is not a PASS.
        self.report.write_text("address\tverdict\tcases\tcompared\tdetail\tname\n")
        self.assertEqual("NO-VERDICT", mutate.verdict_from_report(self.report))

    def test_the_first_row_is_the_answer_when_a_report_holds_several(self):
        """Pinned convention, not a proven-correct choice.

        `run_oracle` deletes the report and re-runs the oracle with `--only`,
        so in production exactly one row comes back and `rows[0]` versus
        `rows[-1]` is an equivalent mutation - it was measured to be, and it
        survived the suite. That is precisely why it is worth pinning: the day
        `run_oracle` grows `--list` (which it arguably should - see
        `test_run_oracle_argv_does_not_carry_the_plan`), the report gains SKIP
        rows for every other plan entry and the two stop agreeing silently.

        If you change which row wins, this test SHOULD fail - update it, do
        not delete it.
        """
        self.report.write_text(
            "address\tverdict\tcases\tcompared\tdetail\tname\n"
            "0x00401000\tFAIL\t16\t16\t\t?subject@@YAXXZ\n"
            "0x00402000\tSKIP\t0\t0\treaches an import\t?other@@YAXXZ\n")
        self.assertEqual("FAIL", mutate.verdict_from_report(self.report))

    def test_a_non_row_line_before_the_data_cannot_become_the_verdict(self):
        # Rows are recognised by their 0x prefix, so a resumed-sweep banner or
        # the header itself must not be read as a verdict string.
        self.report.write_text(
            "address\tverdict\tcases\tcompared\tdetail\tname\n"
            "-- resuming --\n"
            "0x00401000\tPASS\t16\t16\t\t?subject@@YAXXZ\n")
        self.assertEqual("PASS", mutate.verdict_from_report(self.report))


class OracleInvocationTests(unittest.TestCase):
    """The argv `run_oracle` builds, because it sets the comparison rules.

    `run_oracle` was dismissed in an earlier round as "a thin subprocess
    wrapper" not worth testing. It is not thin: the flags it passes decide how
    strictly the mutant is judged, and one of them is missing.
    """

    def argv(self, address=0x00401000) -> list:
        recorded = {}

        class Result:
            returncode = 0
            stdout = stderr = ""

        def fake_run(command, **kwargs):
            recorded["command"] = command
            return Result()

        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        old = (mutate.subprocess.run, mutate.ORACLE)
        mutate.ORACLE = Path(tmp.name)
        mutate.subprocess.run = fake_run
        try:
            mutate.run_oracle(address)
        finally:
            mutate.subprocess.run, mutate.ORACLE = old
        return recorded["command"]

    def test_the_oracle_is_asked_for_one_function_and_all_the_cases(self):
        command = self.argv(0x00559210)
        self.assertIn("--only", command)
        self.assertEqual("0x00559210", command[command.index("--only") + 1])
        self.assertIn("--cases", command)
        self.assertEqual(str(mutate.CASES),
                         command[command.index("--cases") + 1])
        self.assertIn("--no-blame", command)

    def test_the_case_count_flag_follows_the_module_setting(self):
        # `--cases 4` is how the A/B against the legacy seed generator is run;
        # a hard-coded 16 here would make that switch a no-op.
        old = mutate.CASES
        mutate.CASES = 4
        try:
            command = self.argv()
            self.assertEqual("4", command[command.index("--cases") + 1])
        finally:
            mutate.CASES = old

    def test_a_stale_report_cannot_answer_for_the_current_mutant(self):
        """The report is deleted BEFORE the oracle runs.

        If it is not, and the oracle then dies without writing, the previous
        mutant's verdict is read as this one's - so a run in which the oracle
        crashed on every function after the first would report that first
        function's verdict eight times over, and score it as evidence. The
        `NO-VERDICT` -> `unusable` path exists precisely to stop that, and it
        only works if the stale file is gone.
        """
        class Result:
            returncode = 0
            stdout = stderr = ""

        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        stale = Path(tmp.name) / "mutate_one.tsv"
        stale.write_text(
            "address\tverdict\tcases\tcompared\tdetail\tname\n"
            "0x00401000\tFAIL\t16\t16\t\t?previous@@YAXXZ\n")
        old = (mutate.subprocess.run, mutate.ORACLE)
        mutate.ORACLE = Path(tmp.name)
        # The oracle runs and writes nothing at all - it crashed.
        mutate.subprocess.run = lambda command, **kwargs: Result()
        try:
            self.assertEqual("NO-VERDICT", mutate.run_oracle(0x00402000))
        finally:
            mutate.subprocess.run, mutate.ORACLE = old
        self.assertEqual("unusable", mutate.classify("NO-VERDICT", True))

    def test_run_oracle_argv_does_not_carry_the_plan(self):
        """DEFECT, pinned: no `--list`, so the mutant is judged more strictly
        than the sweep that declared the function PASS.

        `tools/lifted_oracle_main.cpp` states the requirement outright - the
        synthetic `--only` entry "must still carry this function's `undef=`
        token, or the one-function reproduction of a whole-plan FAIL would
        compare a flag the plan run does not" - and the `undef=` lookup sits
        inside `if (list_path)`. `lifted_oracle_sweep.sh` does pass
        `--list "$PLAN"`; this does not. Every mutant is therefore compared on
        flags the plan run masks, which biases `caught` UPWARD: the runner can
        credit the oracle with a detection that the real sweep would never
        make.

        Present at HEAD, so this is not a regression - but nothing said so,
        and an unasserted defect is how this file's numbers went wrong before.
        Fixing it changes the score, which is a decision with its own
        measurement, not a refactor. If you add `--list`, this test SHOULD
        fail - update it, do not delete it.
        """
        command = self.argv()
        self.assertNotIn("--list", command)


class MutationRegexTests(unittest.TestCase):
    """Each mutation matches the construct it names, in real emitted C++."""

    def test_every_mutation_has_a_fixture(self):
        # A new mutation with no fixture is a mutation nobody has watched land.
        self.assertEqual(
            {pattern.pattern for _, pattern, _ in mutate.MUTATIONS},
            set(FIXTURES))

    def test_every_fixture_still_names_a_live_mutation(self):
        self.assertEqual(
            set(FIXTURES),
            {pattern.pattern for _, pattern, _ in mutate.MUTATIONS})

    def test_each_mutation_applies_to_the_lowerer_s_own_output(self):
        for name, pattern, replacement in mutate.MUTATIONS:
            fixture = FIXTURES[pattern.pattern]
            with self.subTest(mutation=name, pattern=pattern.pattern):
                statements = lowered(fixture["encoded"],
                                     branch=fixture.get("branch", False))
                text = shard_text([(BASE, "?fixture@@YAXXZ", statements)])
                body = body_of(text, BASE)
                applied = mutate.mutate_body(pattern, replacement, body)
                self.assertIsNotNone(
                    applied,
                    f"{name} matches nothing in the C++ the lowerer emits for "
                    f"{fixture['encoded']}: {statements}. The property is "
                    f"UNTESTED, not equivalent.")
                mutated, _ = applied
                self.assertNotEqual(body, mutated)
                for fragment in fixture["expect"]:
                    self.assertIn(fragment, mutated)
                for fragment in fixture["forbid"]:
                    self.assertNotIn(fragment, mutated)

    def test_a_mutation_leaves_the_rest_of_the_body_alone(self):
        # count=1: one deliberate corruption per mutant, or a FAIL says nothing
        # about which change caused it.
        statements = (lowered("8903") + lowered("8903") + lowered("83c001"))
        body = body_of(shard_text([(BASE, "?two@@YAXXZ", statements)]), BASE)
        pattern = re.compile(r"^(\s*)opensmacx_store32\(", re.M)
        mutated, _ = mutate.mutate_body(pattern, r"\1if (false) opensmacx_store32(",
                                        body)
        self.assertEqual(1, mutated.count("if (false)"))
        self.assertIn("s.eax = opensmacx_add32(s, s.eax, 0x00000001U);", mutated)

    def test_a_mutation_that_matches_nothing_is_untested(self):
        # Not "equivalent", not "clean": untested. `None` is what the runner
        # turns into an UNTESTED line and a non-zero exit.
        body = body_of(shard_text([(BASE, "?f@@YAXXZ", lowered("89d1"))]), BASE)
        for name, pattern, replacement in mutate.MUTATIONS:
            with self.subTest(mutation=name):
                self.assertIsNone(mutate.mutate_body(pattern, replacement, body))

    def test_a_mutation_that_changes_nothing_is_untested(self):
        # It matched, so `subn` reports count=1 and the old code carried on to
        # build a byte-identical mutant, get a correct PASS, and record a MISS.
        body = body_of(shard_text([(BASE, "?f@@YAXXZ", lowered("d1f8"))]), BASE)
        self.assertIsNotNone(mutate.mutate_body(
            re.compile("opensmacx_sar32"), "opensmacx_shr32", body))
        self.assertIsNone(mutate.mutate_body(
            re.compile("opensmacx_sar32"), "opensmacx_sar32", body))

    def test_the_canary_only_adds_a_trap(self):
        # The canary measures reachability of the mutated line. If it also
        # deleted the statement it would be a second mutation, and `reached`
        # would be an answer about a different program.
        for name, pattern, replacement in mutate.MUTATIONS:
            fixture = FIXTURES[pattern.pattern]
            with self.subTest(mutation=name):
                statements = lowered(fixture["encoded"],
                                     branch=fixture.get("branch", False))
                body = body_of(shard_text([(BASE, "?f@@YAXXZ", statements)]), BASE)
                _, canary = mutate.mutate_body(pattern, replacement, body)
                self.assertIn(mutate.CANARY, canary)
                self.assertEqual(body, canary.replace(mutate.CANARY, "", 1))
                self.assertEqual(1, canary.count(mutate.CANARY))


class BodySpanTests(unittest.TestCase):
    """The slice must cut the named function and only the named function."""

    def setUp(self):
        self.second = BASE + 0x40
        self.text = shard_text([
            (BASE, "?first@@YAXXZ", lowered("8903")),
            (self.second, "?second@@YAXXZ", lowered("8b33") + lowered("d1f8")),
        ])

    def test_the_span_selects_the_named_function(self):
        body = body_of(self.text, self.second)
        self.assertIn("s.esi = opensmacx_mem32(s.ebx);", body)
        self.assertIn("void lifted_00401040(", body)

    def test_the_span_excludes_the_other_function(self):
        # A span that ran to the LAST closing brace would still contain the
        # right function, still compile, and let a mutation land in a
        # neighbour whose verdict nobody is reading.
        body = body_of(self.text, self.second)
        self.assertNotIn("opensmacx_store32", body)
        self.assertNotIn("void lifted_00401000(", body)

    def test_the_first_function_stops_before_the_second(self):
        body = body_of(self.text, BASE)
        self.assertIn("opensmacx_store32(s.ebx, s.eax);", body)
        self.assertNotIn("s.esi = opensmacx_mem32", body)
        self.assertNotIn("void lifted_00401040(", body)

    def test_head_and_tail_rebuild_the_file_byte_for_byte(self):
        # The runner writes back head + mutated + tail, so anything the span
        # arithmetic drops is deleted from a real shard.
        start, end = mutate.body_span(self.text, self.second)
        self.assertEqual(self.text,
                         self.text[:start] + self.text[start:end] + self.text[end:])

    def test_an_absent_function_has_no_span(self):
        self.assertIsNone(mutate.body_span(self.text, 0x00999999))

    def test_a_near_miss_address_is_not_close_enough(self):
        # lifted_00401040 must not be found by asking for 0x00401004.
        self.assertIsNone(mutate.body_span(self.text, BASE + 4))

    def test_an_unterminated_body_has_no_span(self):
        truncated = self.text[:self.text.find("void lifted_00401040") + 60]
        self.assertIsNone(mutate.body_span(truncated, self.second))


class TargetSelectionTests(unittest.TestCase):
    """Which function each mutation lands on, and when it lands on none."""

    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.addCleanup(self.dir.cleanup)
        root = Path(self.dir.name)
        self.decoy = BASE
        self.carrier = BASE + 0x40
        self.other_carrier = BASE + 0x80
        self.shard = root / "lifted_000.cpp"
        self.shard.write_text(shard_text([
            (self.decoy, "?decoy@@YAXXZ", lowered("89d1")),
            (self.carrier, "?carrier@@YAXXZ", lowered("8903")),
            (self.other_carrier, "?other@@YAXXZ", lowered("8903")),
        ]), encoding="utf-8")
        self.candidates = [(self.decoy, "?decoy@@YAXXZ"),
                           (self.carrier, "?carrier@@YAXXZ"),
                           (self.other_carrier, "?other@@YAXXZ")]
        self.store = next((name, pattern, replacement)
                          for name, pattern, replacement in mutate.MUTATIONS
                          if name == "drop a store")

    def locate(self, address):
        return self.shard

    def find(self, spent=(), min_size=0, mutation=None):
        _, pattern, replacement = mutation or self.store
        return mutate.find_target(pattern, replacement, self.candidates,
                                  set(spent), min_size, locate=self.locate)

    def test_a_function_without_the_construct_is_passed_over(self):
        target = self.find()
        self.assertIsNotNone(target)
        self.assertEqual(self.carrier, target.address)
        self.assertEqual("?carrier@@YAXXZ", target.symbol)

    def test_a_spent_function_is_not_mutated_twice(self):
        # Six of eight mutants on one function is a measurement of that
        # function, not of the oracle.
        target = self.find(spent=[self.carrier])
        self.assertEqual(self.other_carrier, target.address)

    def test_no_candidate_carrying_the_construct_means_no_target(self):
        target = self.find(spent=[self.carrier, self.other_carrier])
        self.assertIsNone(target)

    def test_a_shard_that_cannot_be_located_is_skipped(self):
        target = mutate.find_target(
            self.store[1], self.store[2], self.candidates, set(), 0,
            locate=lambda address: self.shard if address == self.other_carrier
            else None)
        self.assertEqual(self.other_carrier, target.address)

    def test_min_size_rejects_bodies_with_too_few_statements(self):
        # The median passing function is 34 bytes; without this the draw is
        # dominated by bodies too small to touch memory at all.
        self.assertIsNone(self.find(min_size=99))
        self.assertIsNotNone(self.find(min_size=1))

    def test_the_mutation_is_confined_to_the_chosen_function(self):
        target = self.find()
        rebuilt = target.text(target.mutated)
        self.assertEqual(1, rebuilt.count("if (false)"))
        self.assertIn("void lifted_00401080(", rebuilt)
        self.assertIn("s.ecx = s.edx;", rebuilt)
        self.assertEqual(len(rebuilt),
                         len(self.shard.read_text()) + len("if (false) "))

    def test_the_untouched_variant_is_the_original_file(self):
        target = self.find()
        self.assertEqual(self.shard.read_text(), target.text(target.body))


class ScoringTests(unittest.TestCase):
    """A run is only clean if every attempted mutation produced evidence."""

    def test_a_miss_fails_the_run(self):
        self.assertNotEqual(0, mutate.exit_code(caught=7, missed=1, untested=0))

    def test_all_caught_passes(self):
        self.assertEqual(0, mutate.exit_code(caught=8, missed=0, untested=0))

    def test_an_untested_mutation_fails_the_run(self):
        # The whole point: a mutation kind that matched nothing used to leave a
        # clean score and a zero exit behind it.
        self.assertNotEqual(0, mutate.exit_code(caught=8, missed=0, untested=1))

    def test_a_run_that_caught_nothing_at_all_fails(self):
        # Every mutant unusable or unreached is not evidence of a good oracle.
        self.assertNotEqual(0, mutate.exit_code(caught=0, missed=0, untested=0))


# ---------------------------------------------------------------------------
# The driver itself.
#
# An earlier round excused `main()` from testing on the grounds that running it
# "requires Wine, the mingw cross-compiler and a full shard rebuild per mutant,
# minutes each". That is false, and the cost of believing it was ten uncaught
# mutations - eight of which move a published number, including the one that
# turns the exact regression this script was written to fix back off:
#
#   untested.append(...) -> pass            EXIT 1  -> EXIT 0
#   reached = ... in CAUGHT -> not in       missed 4 -> missed 0
#   canary sub count=1 dropped              unreached 4 -> missed 1, unreached 3
#   "detected" -> caught made canary-gated  caught 4 -> caught 0
#   parts[1] == "PASS" -> !=                mutants 4 -> mutants 0
#   spent.add(...) -> pass                  mutants 4 -> mutants 5
#   the shard restore -> pass               corrupt build tree survives the run
#   random.shuffle -> pass                  targets revert to address order
#
# `run_oracle` and `rebuild` are module-level names, so replacing them makes a
# whole run take milliseconds. Everything else - passing_functions, shard_of,
# find_target, mutate_body, classify, exit_code, the shard writes and the
# restore - is the real code.
#
# The decision the script exists to make is SPLIT across main() and the
# extracted helpers: `classify` is handed `reached` and `exit_code` is handed
# `untested`, and nothing tested who computes either. These tests test that.
# ---------------------------------------------------------------------------

# One body, carrying eight of the twelve mutation constructs. The four it does
# not carry (the inverted branch and the three x87 shapes) are what makes the
# UNTESTED path fire without any extra scaffolding.
DRIVER_BODY = [
    "s.eax = opensmacx_mem32(s, s.ebp + 0x00000008U);",
    "s.esi = opensmacx_mem32(s, s.ebx);",
    "opensmacx_store32(s, s.edi, s.eax);",
    "s.eax = opensmacx_sar32(s, s.eax, 0x00000001U);",
    "s.ecx = opensmacx_mem8(s, s.edx);",
    "if (opensmacx_zf(s)) goto L_00401020;",
    "(void)opensmacx_sub32(s, s.eax, s.ecx);",
]
MATCHING_KINDS = 8      # ...of len(mutate.MUTATIONS)
DRIVER_FUNCTIONS = 16

# ...and one that carries all twelve, so the "no kind went untested" side of
# the exit code can be reached at all.
FULL_BODY = DRIVER_BODY + [
    "if (!opensmacx_zf(s)) goto L_00401024;",
    "opensmacx_x87_binary_st0(OpensmacxX87Mul, s.ebx);",
    "opensmacx_x87_fld32(s.ebp + 0xffffffe8U);",
    "opensmacx_x87_fst32(s.ebp + 0xfffffff4U);",
]


def statements_in_body(body) -> int:
    """The `;` count `--min-size` actually sees for a body of `body`.

    `body_span` runs to the closing brace, so the generated trailing
    `opensmacx_trap` is inside the span and counts. Computing it here rather
    than hard-coding a number keeps the boundary test honest if the generated
    epilogue ever changes.
    """
    text = shard_text([(BASE, "?probe@@YAXXZ", body)])
    return body_of(text, BASE).count(";")


class DriverRun:
    """One scripted `main()` run and everything observable about it."""

    def __init__(self, code, out, shard, pristine, oracle_calls, canary_texts,
                 mutated_texts, addresses):
        self.code = code
        self.out = out
        self.shard = shard
        self.pristine = pristine
        self.oracle_calls = oracle_calls
        self.canary_texts = canary_texts
        self.mutated_texts = mutated_texts
        self.addresses = addresses

    def score(self) -> dict:
        """The `mutants N: caught .., missed ..` line, parsed back out."""
        for line in self.out.splitlines():
            if line.startswith("mutants "):
                head, rest = line.split(":", 1)
                out = {"mutants": int(head.split()[1])}
                for piece in rest.split(","):
                    key, value = piece.split()
                    out[key] = int(value)
                return out
        raise AssertionError(f"no score line in:\n{self.out}")

    def untested_kinds(self) -> list:
        return [line.split("UNTESTED", 1)[1].strip()
                for line in self.out.splitlines() if "UNTESTED" in line]


class DriverTests(unittest.TestCase):
    """`main()` end to end, with the two subprocess calls scripted."""

    #: canary verdict, mutant verdict -> what the run should conclude
    MODES = {
        # the canary trap fired, and the mutant was then detected
        "detects": ("SKIP-trap", "FAIL"),
        # the trap fired, so the line runs - and the oracle still said PASS
        "blind": ("SKIP-trap", "PASS"),
        # the trap never fired: no seed reaches the line
        "unreached": ("PASS", "PASS"),
        # the original side could not be run; says nothing either way
        "no-evidence": ("SKIP-trap", "INCONCLUSIVE-original-fault"),
        # detected even though the canary said nothing - a FAIL is a FAIL
        "detects-without-canary": ("PASS", "FAIL"),
    }

    def drive(self, mode="detects", *, argv=(), verdicts=("PASS",) * 8,
              body=None, functions=DRIVER_FUNCTIONS, rebuild=True) -> DriverRun:
        canary_verdict, mutant_verdict = self.MODES[mode]
        body = DRIVER_BODY if body is None else body
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        root = Path(tmp.name)
        lifted, oracle = root / "lifted", root / "oracle"
        lifted.mkdir()
        oracle.mkdir()

        addresses = [BASE + 0x100 * i for i in range(functions)]
        shard = lifted / "lifted_000.cpp"
        shard.write_text(shard_text(
            [(a, f"?f{i}@@YAXXZ", body) for i, a in enumerate(addresses)]),
            encoding="utf-8")
        pristine = shard.read_bytes()

        rows = ["address\tverdict\tcases\tcompared\tdetail\tname"]
        for i, address in enumerate(addresses):
            rows.append(f"{address:#010x}\t{verdicts[i % len(verdicts)]}"
                        f"\t16\t16\t\t?f{i}@@YAXXZ")
        (oracle / "report.tsv").write_text("\n".join(rows) + "\n",
                                           encoding="utf-8")

        oracle_calls, canary_texts, mutated_texts = [], [], []

        def fake_run_oracle(address):
            text = shard.read_text(encoding="utf-8")
            is_canary = mutate.CANARY in text
            oracle_calls.append((address, "canary" if is_canary else "mutant"))
            if is_canary:
                canary_texts.append(text)
                return canary_verdict
            mutated_texts.append(text)
            return mutant_verdict

        old = (mutate.LIFTED, mutate.ORACLE, mutate.REPORT,
               mutate.run_oracle, mutate.rebuild, sys.argv, mutate.CASES)
        mutate.LIFTED, mutate.ORACLE = lifted, oracle
        mutate.REPORT = oracle / "report.tsv"
        mutate.run_oracle = fake_run_oracle
        mutate.rebuild = lambda _shard: rebuild
        sys.argv = ["lifted_oracle_mutate.py", *argv]
        captured = io.StringIO()
        try:
            with contextlib.redirect_stdout(captured):
                code = mutate.main()
        finally:
            (mutate.LIFTED, mutate.ORACLE, mutate.REPORT, mutate.run_oracle,
             mutate.rebuild, sys.argv, mutate.CASES) = old
        return DriverRun(code, captured.getvalue(), shard, pristine,
                         oracle_calls, canary_texts, mutated_texts, addresses)

    # -- the score ----------------------------------------------------------

    def test_a_detecting_oracle_scores_every_mutant_caught(self):
        run = self.drive("detects")
        self.assertEqual(MATCHING_KINDS, run.score()["mutants"])
        self.assertEqual(MATCHING_KINDS, run.score()["caught"])
        self.assertEqual(0, run.score()["missed"])

    def test_a_detection_is_a_detection_whatever_the_canary_said(self):
        """A FAIL is evidence on its own; `reached` is only consulted for PASS.

        Gating "caught" on the canary turns a fully detecting run into
        `caught 0, unreached 8` - a total inversion of the headline, produced
        by a plausible-looking edit to a function whose own docstring says the
        canary is "consulted only when the oracle reported agreement".
        """
        run = self.drive("detects-without-canary")
        self.assertEqual(MATCHING_KINDS, run.score()["caught"])
        self.assertEqual(0, run.score()["unreached"])
        # ...and the same claim at the unit level, which nothing asserted:
        # every detection classifies as caught for all three canary answers.
        for verdict in sorted(mutate.CAUGHT):
            for reached in (True, False, None):
                with self.subTest(verdict=verdict, reached=reached):
                    self.assertEqual("caught",
                                     mutate.classify(verdict, reached))

    def test_a_blind_oracle_scores_every_mutant_missed(self):
        """The canary fired, so the line runs, so a PASS is a hole.

        This is the run the script exists to produce. Inverting the canary
        test - `run_oracle(...) not in CAUGHT` - turns all eight misses into
        unreached and reports a clean oracle.
        """
        run = self.drive("blind")
        self.assertEqual(MATCHING_KINDS, run.score()["missed"])
        self.assertEqual(0, run.score()["unreached"])
        self.assertEqual(0, run.score()["caught"])
        self.assertNotEqual(0, run.code)

    def test_a_line_no_seed_reaches_is_not_scored_against_the_oracle(self):
        run = self.drive("unreached")
        self.assertEqual(MATCHING_KINDS, run.score()["unreached"])
        self.assertEqual(0, run.score()["missed"])
        self.assertIn("no seed", run.out)

    def test_an_original_side_inconclusive_is_scored_unusable_by_the_run(self):
        run = self.drive("no-evidence")
        self.assertEqual(MATCHING_KINDS, run.score()["unusable"])
        self.assertEqual(0, run.score()["missed"])
        self.assertEqual(0, run.score()["caught"])

    def test_the_four_buckets_are_reachable_and_distinct(self):
        seen = {mode: self.drive(mode).score() for mode in self.MODES}
        self.assertEqual(MATCHING_KINDS, seen["detects"]["caught"])
        self.assertEqual(MATCHING_KINDS, seen["blind"]["missed"])
        self.assertEqual(MATCHING_KINDS, seen["unreached"]["unreached"])
        self.assertEqual(MATCHING_KINDS, seen["no-evidence"]["unusable"])

    # -- UNTESTED, and the exit code ----------------------------------------

    def test_a_mutation_kind_that_matched_nothing_is_reported_and_fails_the_run(self):
        """The regression this script was written to fix, at the driver level.

        `exit_code` is unit-tested with `untested=1`, but nothing checked that
        `main()` ever puts anything IN that list: replacing the one
        `untested.append(...)` with `pass` left a fully green run - `mutants 8:
        caught 8`, EXIT 0 - while four of the twelve mutation kinds had never
        been put to the oracle at all.
        """
        run = self.drive("detects", argv=["--count", "99"])
        kinds = run.untested_kinds()
        self.assertEqual(len(mutate.MUTATIONS) - MATCHING_KINDS, len(kinds))
        self.assertNotEqual(0, run.code, "an untested kind must fail the run")
        self.assertIn("This is not a pass", run.out)
        # Each untested line names the mutation and its regex, so the reader
        # can tell which property went unexercised.
        for kind in kinds:
            self.assertIn("/", kind)

    def test_the_untested_kinds_are_the_ones_the_body_does_not_carry(self):
        run = self.drive("detects", argv=["--count", "99"])
        reported = " ".join(run.untested_kinds())
        for missing in ("opensmacx_x87_fld32", "opensmacx_x87_fst32",
                        "OpensmacxX87Mul", r"if \(!opensmacx_zf"):
            with self.subTest(missing=missing):
                self.assertIn(missing, reported)

    def test_a_run_where_every_kind_applies_succeeds(self):
        """The other side of the exit code, so it is not simply always 1.

        Every one of the twelve mutation kinds finds a home in FULL_BODY, so
        nothing is untested and every mutant is caught: exit 0. Without this
        the untested assertions above would pass against a run that can only
        ever fail.
        """
        run = self.drive("detects", body=FULL_BODY,
                         argv=["--count", str(len(mutate.MUTATIONS))])
        self.assertEqual([], run.untested_kinds())
        self.assertEqual(len(mutate.MUTATIONS), run.score()["mutants"])
        self.assertEqual(len(mutate.MUTATIONS), run.score()["caught"])
        self.assertEqual(0, run.code)

    def test_running_out_of_count_is_not_the_same_as_untested(self):
        """"--count ran out" must not be reported as a property never tested.

        One is a budget the operator set; the other is a claim the run makes
        about the oracle. Pooling them would make every short run look like a
        gap in the mutation set.
        """
        run = self.drive("detects", body=FULL_BODY, argv=["--count", "3"])
        self.assertEqual([], run.untested_kinds())
        self.assertIn("not attempted", run.out)
        self.assertEqual(3, run.score()["mutants"])
        self.assertEqual(0, run.code)

    def test_a_run_that_catches_nothing_fails_even_with_no_misses(self):
        run = self.drive("no-evidence", argv=["--count", str(MATCHING_KINDS)])
        self.assertEqual(0, run.score()["caught"])
        self.assertNotEqual(0, run.code)

    # -- who becomes a candidate --------------------------------------------

    def test_only_passing_functions_are_mutated(self):
        """A FAIL is already failing; mutating it proves nothing.

        Inverting the filter takes the run to `mutants 0` while still exiting
        non-zero for other reasons, so the score line is what pins it.
        """
        verdicts = ("PASS", "FAIL", "SKIP", "PASS",
                    "INCONCLUSIVE-original-fault", "PASS", "SKIP-trap", "PASS")
        run = self.drive("detects", verdicts=verdicts, argv=["--count", "99"])
        passing = {address for i, address in enumerate(run.addresses)
                   if verdicts[i % len(verdicts)] == "PASS"}
        chosen = {address for address, _ in run.oracle_calls}
        self.assertTrue(chosen, "nothing was mutated at all")
        self.assertTrue(chosen <= passing,
                        f"mutated a non-PASS function: {chosen - passing}")
        # ...and the non-PASS rows really were in the report, so this is not
        # passing because the filter had nothing to reject.
        self.assertLess(len(passing), len(run.addresses))

    def test_no_function_is_mutated_twice(self):
        """Six of eight mutants landing on one function measures that function.

        `spent` is what stops it. Dropping it lets the same address absorb
        several mutation kinds and inflates `mutants` past the number of
        distinct functions actually examined.
        """
        run = self.drive("detects", argv=["--count", "99"])
        chosen = [address for address, phase in run.oracle_calls
                  if phase == "mutant"]
        self.assertEqual(len(chosen), len(set(chosen)))
        self.assertEqual(MATCHING_KINDS, len(chosen))

    def test_targets_are_drawn_shuffled_not_in_report_order(self):
        """Address order is what made three oracle-level defects invisible.

        Every function here carries every construct, so which one each kind
        lands on is decided by the shuffle alone. Without it the run walks the
        report from the top and mutates f0, f1, f2 ... in order.
        """
        run = self.drive("detects", argv=["--count", "99"])
        chosen = [address for address, phase in run.oracle_calls
                  if phase == "mutant"]
        self.assertEqual(sorted(chosen), chosen and sorted(set(chosen)))
        self.assertNotEqual(run.addresses[:len(chosen)], chosen,
                            "targets came out in report order - not shuffled")

    def test_the_same_seed_draws_the_same_targets_and_a_different_one_does_not(self):
        def draw(seed):
            run = self.drive("detects", argv=["--count", "99", "--seed", seed])
            return [a for a, phase in run.oracle_calls if phase == "mutant"]
        self.assertEqual(draw("11"), draw("11"))
        self.assertNotEqual(draw("11"), draw("12"))

    def test_min_size_is_a_minimum_and_not_a_strict_greater_than(self):
        """`--min-size N` must accept a body of exactly N statements.

        `<` -> `<=` makes the flag silently mean "more than N", which quietly
        shrinks the candidate pool by exactly the bodies at the boundary.
        """
        size = statements_in_body(DRIVER_BODY)
        exact = self.drive("detects", argv=["--count", "1",
                                            "--min-size", str(size)])
        self.assertEqual(1, exact.score()["mutants"],
                         f"a body of exactly {size} statements was rejected")
        too_big = self.drive("detects", argv=["--count", "1",
                                              "--min-size", str(size + 1)])
        self.assertEqual(0, too_big.score()["mutants"])

    # -- what the run leaves behind -----------------------------------------

    def test_the_shard_is_byte_identical_when_the_run_finishes(self):
        """A mutation runner that leaves damage behind poisons everything after.

        The restore is one `write_bytes`; dropping it leaves the last mutant
        compiled into the tree, and every later oracle result is then measuring
        a corrupted lift.
        """
        run = self.drive("detects", argv=["--count", "99"])
        self.assertEqual(run.pristine, run.shard.read_bytes())
        self.assertNotIn(mutate.CANARY, run.shard.read_text(encoding="utf-8"))

    def test_the_shard_is_restored_even_when_nothing_compiles(self):
        run = self.drive("detects", argv=["--count", "99"], rebuild=False)
        self.assertEqual(run.pristine, run.shard.read_bytes())
        # Nothing built, so nothing was judged: no misses may be invented.
        self.assertEqual(0, run.score()["missed"])
        self.assertEqual(MATCHING_KINDS, run.score()["unusable"])

    def test_the_canary_plants_exactly_one_trap(self):
        """One trap on the mutated line, not one on every matching line.

        Dropping `count=1` from the canary substitution sprays traps over the
        whole body, so the canary answers "some line here is reached" instead
        of "THIS line is reached" - and misses and coverage gaps get shuffled
        between buckets on that basis.
        """
        run = self.drive("detects", argv=["--count", "99"])
        self.assertEqual(MATCHING_KINDS, len(run.canary_texts))
        for text in run.canary_texts:
            self.assertEqual(1, text.count(mutate.CANARY))

    def test_the_canary_traps_one_line_when_the_body_repeats_the_construct(self):
        """The body above matches each regex once, so `count=1` is invisible.

        A real lowered body repeats its constructs constantly - three stores,
        four `sar`s - and the canary must land on the SAME single line the
        mutant changes. Spray traps over all of them and `reached` answers
        "some line matching this regex runs", which silently reclassifies
        misses as coverage gaps and back.
        """
        repeated = [
            "opensmacx_store32(s, s.edi, s.eax);",
            "opensmacx_store32(s, s.esi, s.ecx);",
            "opensmacx_store32(s, s.ebx, s.edx);",
        ]
        run = self.drive("detects", body=repeated, argv=["--count", "1"])
        self.assertEqual(1, run.score()["mutants"])
        self.assertEqual(1, len(run.canary_texts))
        self.assertEqual(1, run.canary_texts[0].count(mutate.CANARY))
        # ...and the mutant changed exactly one of the three, too.
        self.assertEqual(1, run.mutated_texts[0].count("if (false)"))

    def test_the_canary_and_the_mutant_land_on_the_same_line(self):
        """Otherwise `reached` describes a different statement from the mutant.

        The canary's trap and the mutant's `if (false)` must sit on the same
        store. Comparing their positions is the only way to see it: both
        bodies contain exactly one edit either way round.
        """
        repeated = [
            "opensmacx_store32(s, s.edi, s.eax);",
            "opensmacx_store32(s, s.esi, s.ecx);",
            "opensmacx_store32(s, s.ebx, s.edx);",
        ]
        run = self.drive("detects", body=repeated, argv=["--count", "1"])
        canary_line = next(
            i for i, line in enumerate(run.canary_texts[0].splitlines())
            if mutate.CANARY.strip() in line)
        mutant_line = next(
            i for i, line in enumerate(run.mutated_texts[0].splitlines())
            if "if (false)" in line)
        self.assertEqual(canary_line, mutant_line)

    def test_the_canary_and_the_mutant_are_different_builds(self):
        # Two oracle runs per mutant, canary first. A run that fed the oracle
        # the canary text twice would report every line reached.
        run = self.drive("detects", argv=["--count", "99"])
        phases = [phase for _, phase in run.oracle_calls]
        self.assertEqual(["canary", "mutant"] * MATCHING_KINDS, phases)
        for text in run.mutated_texts:
            self.assertNotIn(mutate.CANARY, text)

    def test_the_report_is_required_before_anything_is_mutated(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        old = (mutate.REPORT, sys.argv)
        mutate.REPORT = Path(tmp.name) / "absent.tsv"
        sys.argv = ["lifted_oracle_mutate.py"]
        try:
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(2, mutate.main())
        finally:
            mutate.REPORT, sys.argv = old


if __name__ == "__main__":
    unittest.main()
