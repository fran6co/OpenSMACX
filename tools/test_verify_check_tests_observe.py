#!/usr/bin/env python3
"""The instrument that measures every other check had no tests of its own.

Every survivor rate this project publishes comes out of
verify_check_tests_observe.py, and until today nothing tested it. Its own
default sweep is why nobody noticed: candidates were chosen by
`name.startswith(("verify_", "audit_"))` AND `test_<tool>.py exists`, so a tool
with no tests was silently skipped - and this tool has no tests, so it excluded
itself from the census it produces.

Four defects were found the moment it was looked at, and all four are pinned
below:

  * the prefix tuple was a hand-maintained list. Measured 2026-08-13: CMake
    registers 29 gate checks and the tuple could see 16 of them. The 13 invisible
    ones are every `derive_*`, `emit_*`, `export_*`, `classify_*`, `correlate_*`
    and `measure_*` check plus decomp_status - and they did not show as `not
    measured`, they did not show at all;
  * the comparison operator `break`s after the first match, so
    `if (x >= lo && x <= hi)` perturbs ONE bound and which one depends on the
    order of COMPARISONS. 347 lines in src/ carry a second comparison nothing
    ever touched;
  * the census was written once, after the whole loop, so interrupting a sweep
    that costs over an hour discarded everything measured;
  * the mutant cap was a `break` in line order, so every rate for a long tool
    described the TOP of the file - verify_recovery_abi's first 40 eligible
    mutants are lines 45-258 of 2,608 - and a truncated run printed exactly what
    a complete run printed.
"""
import argparse
import contextlib
import io
import json
import tempfile
import textwrap
import unittest
from pathlib import Path

import verify_check_tests_observe as observer


def mutants_of(lines, operator=None):
    """Run the shipped generator over a whole synthetic body."""
    whole = observer.Function(address="00000000", start=0, end=len(lines))
    return [mutant for mutant in observer.build_mutants(lines, whole)
            if operator is None or mutant.operator == operator]


class ComparisonOperatorTests(unittest.TestCase):
    """Every comparison on the line, not whichever one COMPARISONS lists first.

    A range check has two edges and an off-by-one lives on either. Under the
    `break` the second edge of every range test in the tree was unmutated, so a
    harness reporting "all mutants killed" had never touched it.
    """

    def test_a_line_with_two_comparisons_yields_two_mutants(self):
        # THE DEFECT, in one case. `>=` matched first and `<=` was never reached.
        lines = ["void f(int x) {",
                 "    if (x >= 3 && x <= 9) { g(); }",
                 "}"]
        mutants = mutants_of(lines, "comparison")
        self.assertEqual(
            2, len(mutants),
            f"one bound of a two-sided range went unmutated: "
            f"{[mutant.description for mutant in mutants]}")

    def test_both_bounds_are_the_ones_perturbed(self):
        lines = ["    if (x >= 3 && x <= 9) { g(); }"]
        bodies = [mutant.lines[0] for mutant in mutants_of(lines, "comparison")]
        self.assertIn("    if (x > 3 && x <= 9) { g(); }", bodies)
        self.assertIn("    if (x >= 3 && x < 9) { g(); }", bodies)

    def test_each_mutant_changes_exactly_one_operator(self):
        # Rewriting both at once would be a single coarser mutant, and a suite
        # that catches the pair says nothing about either edge alone.
        lines = ["    if (x >= 3 && x <= 9) { g(); }"]
        for mutant in mutants_of(lines, "comparison"):
            self.assertNotEqual(lines[0], mutant.lines[0])
            differences = sum(1 for original, mutated
                              in zip(lines[0].split("&&"),
                                     mutant.lines[0].split("&&"))
                              if original != mutated)
            self.assertEqual(1, differences, mutant.description)

    def test_three_comparisons_on_one_line_yield_three_mutants(self):
        lines = ["    if (a < b && c > d && e == f) { g(); }"]
        self.assertEqual(3, len(mutants_of(lines, "comparison")))

    def test_the_mutants_are_emitted_in_source_order(self):
        # Ordering by offset rather than by COMPARISONS position is what makes a
        # survivor list readable against the source it came from.
        lines = ["    if (x == 1 && y <= 2 && z > 3) { g(); }"]
        found = [mutant.description.split("`")[1]
                 for mutant in mutants_of(lines, "comparison")]
        self.assertEqual(["==", "<=", ">"], found)

    def test_a_single_comparison_still_yields_exactly_one(self):
        lines = ["    if (x < y) { g(); }"]
        self.assertEqual(1, len(mutants_of(lines, "comparison")))

    def test_no_comparison_yields_none(self):
        self.assertEqual([], mutants_of(["    x = y + 1;"], "comparison"))

    def test_template_angle_brackets_are_still_not_comparisons(self):
        # Emitting every occurrence must not resurrect the template rewrites the
        # blanking exists to suppress; only the real `<` should survive.
        lines = ["    if (static_cast<int>(a) < b) { g(); }"]
        found = [mutant.description.split("`")[1]
                 for mutant in mutants_of(lines, "comparison")]
        self.assertEqual(["<"], found)

    def test_a_comparison_inside_a_trailing_comment_is_not_mutated(self):
        # The comment compiles to nothing, so its operators are phantoms that
        # survive every suite and read as coverage holes.
        lines = ["    x = 1;  // set when a >= b"]
        self.assertEqual([], mutants_of(lines, "comparison"))


class SamplingTests(unittest.TestCase):
    """The cap must sample the file, not its first pages."""

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def write_tool(self, count):
        """A module with `count` mutable integer literals, widely spread."""
        body = "\n".join(f"def f{index}(x):\n    return x + {index + 3}\n"
                         for index in range(count))
        path = self.root / "tool_under_test.py"
        path.write_text(body, encoding="utf-8")
        return path

    def test_an_uncapped_run_measures_every_eligible_mutant(self):
        path = self.write_tool(30)
        _, sampled, eligible = observer.mutants_for(path, 0)
        self.assertEqual(len(sampled), eligible)
        self.assertGreater(eligible, 30)

    def test_a_capped_run_returns_exactly_the_cap(self):
        path = self.write_tool(30)
        _, sampled, eligible = observer.mutants_for(path, 12)
        self.assertEqual(12, len(sampled))
        self.assertGreater(eligible, 12)

    def test_a_capped_run_reaches_the_end_of_the_file(self):
        # THE DEFECT. Under the old `break` the sample stopped at the tenth
        # mutant, which for any real check tool is still in its imports.
        path = self.write_tool(40)
        lines, sampled, _ = observer.mutants_for(path, 10)
        last = max(mutant.line_number for mutant in sampled)
        self.assertGreater(last, len(lines) * 0.7,
                           f"a 10-mutant sample of a {len(lines)}-line file "
                           f"reached only line {last}; it is measuring the top "
                           f"of the file, not the file")

    def test_the_sample_spans_the_whole_file(self):
        path = self.write_tool(40)
        lines, sampled, _ = observer.mutants_for(path, 10)
        numbers = [mutant.line_number for mutant in sampled]
        self.assertLess(min(numbers), len(lines) * 0.2)
        self.assertGreater(max(numbers), len(lines) * 0.7)

    def test_the_sample_never_repeats_a_mutant(self):
        # A stride computed with integer division would collide and quietly
        # measure the same mutant several times, inflating agreement.
        path = self.write_tool(40)
        _, sampled, _ = observer.mutants_for(path, 17)
        self.assertEqual(17, len({id(mutant) for mutant in sampled}))

    def test_a_cap_above_the_eligible_count_is_not_padded(self):
        path = self.write_tool(3)
        _, sampled, eligible = observer.mutants_for(path, 500)
        self.assertEqual(eligible, len(sampled))

    def test_the_eligible_count_is_reported_alongside_the_sample(self):
        # Without it a truncated run is indistinguishable from a census in every
        # artifact it produces, which is how "50% survivors" over 2.2% of a tool
        # got published as a property of the tool.
        path = self.write_tool(30)
        _, sampled, eligible = observer.mutants_for(path, 5)
        self.assertEqual(5, len(sampled))
        self.assertGreater(eligible, len(sampled))


class CandidateTests(unittest.TestCase):
    """Which tools the census covers when nobody passes --tools."""

    def candidates(self):
        # The shipped derivation, not a re-implementation of it. Recomputing the
        # rule here would hide the defect, because a test that restates the code
        # cannot disagree with it.
        return [path.name for path in observer.candidate_tools()]

    def test_every_check_cmake_registers_is_a_candidate(self):
        # THE DEFECT. `verify_*`/`audit_*` could see 16 of 29.
        names = self.candidates()
        for script in observer.registered_check_scripts().values():
            if (observer.TOOLS / script).is_file():
                self.assertIn(script, names)

    def test_the_families_the_prefix_tuple_could_not_see_are_swept(self):
        # Named individually so the diff says which check came back, and so a
        # regression that drops one family is not hidden by the other five.
        names = self.candidates()
        for script in ("derive_class_size_bounds.py", "emit_hypothesis_layouts.py",
                       "export_idb_members.py", "classify_casts.py",
                       "correlate_yitzi_notes.py", "measure_exclusions.py",
                       "decomp_status.py"):
            self.assertIn(script, names)

    def test_the_prefix_families_remain_a_floor(self):
        # A registered-only population would drop the checks that are checks
        # without being registered, which is the mirror of the original defect.
        names = self.candidates()
        for path in observer.TOOLS.glob("*.py"):
            if path.name.startswith(observer.CHECK_PREFIXES):
                self.assertIn(path.name, names)

    def test_a_tool_with_no_test_file_is_still_a_candidate(self):
        # It is not unmeasurable, it is 100% unobserved - and skipping it is how
        # this very file came not to exist.
        self.assertIn("verify_check_tests_observe.py", self.candidates())

    def test_an_unreadable_cmake_never_shrinks_the_population(self):
        # A broken parse must not quietly sweep less while printing the same
        # shape - the defect one level up from the one this tool hunts.
        noise = io.StringIO()
        with contextlib.redirect_stderr(noise):
            fallback = observer.candidate_tools("/nonexistent/CMakeLists.txt")
        names = [path.name for path in fallback]
        self.assertGreaterEqual(len(names), 18)
        self.assertIn("verify_recovery_abi.py", names)
        self.assertIn("FLOOR", noise.getvalue())

    def test_a_broken_scan_does_not_kill_the_sweep(self):
        # gate_checks raises SystemExit, which is a BaseException - so the
        # obvious `except Exception` would take the whole run down instead of
        # falling back.
        with tempfile.TemporaryDirectory() as directory:
            stub = Path(directory) / "CMakeLists.txt"
            stub.write_text('add_test(NAME only-one COMMAND "tools/x.py")\n',
                            encoding="utf-8")
            with contextlib.redirect_stderr(io.StringIO()):
                fallback = observer.candidate_tools(stub)
        self.assertGreaterEqual(len(fallback), 18)

    def test_a_registered_check_resolves_to_the_script_it_runs(self):
        scripts = observer.registered_check_scripts()
        self.assertEqual("verify_recovery_abi.py", scripts["recovery-abi"])
        self.assertEqual("audit_export_signedness.py",
                         scripts["export-signedness-audit"])
        self.assertEqual("measure_exclusions.py", scripts["exclusions-current"])

    def test_every_registered_check_resolves_to_some_script(self):
        # A name the population knows and this cannot resolve is a check that
        # vanishes from the census - the failure mode being fixed, one layer in.
        import verify_checks_can_fail as coverage
        self.assertEqual(set(), set(coverage.gate_checks())
                         - set(observer.registered_check_scripts()))

    def test_a_unit_suite_is_not_mistaken_for_the_check(self):
        # Some gate blocks run `test_<tool>.py` immediately before the tool;
        # picking the test file would mutate the suite instead of the check.
        for script in observer.registered_check_scripts().values():
            self.assertFalse(script.startswith("test_"), script)


class ProseTests(unittest.TestCase):
    """A survivor has to be a survivor of code."""

    def test_a_docstring_is_not_mutable_code(self):
        text = textwrap.dedent('''\
            """A note dated 2026-08-01 with a 512 in it."""
            VALUE = 7
            ''')
        self.assertIn(1, observer.prose_lines(text))

    def test_a_comment_is_not_mutable_code(self):
        self.assertIn(1, observer.prose_lines("# 42 things\nVALUE = 7\n"))

    def test_real_code_is_mutable(self):
        self.assertNotIn(2, observer.prose_lines("# note\nVALUE = 7\n"))

    def test_a_function_docstring_is_skipped_across_all_its_lines(self):
        text = textwrap.dedent('''\
            def f():
                """line one 111
                line two 222
                """
                return 5
            ''')
        prose = observer.prose_lines(text)
        for line in (2, 3, 4):
            self.assertIn(line, prose)
        self.assertNotIn(5, prose)

    def test_unparseable_source_still_skips_comments(self):
        # build_mutants runs on mutated text, which can be syntactically broken;
        # falling over here would abandon the sweep.
        self.assertIn(1, observer.prose_lines("# a note\ndef f(:\n"))


class SuiteRunTests(unittest.TestCase):
    def test_a_hanging_suite_counts_as_detection_not_as_an_abort(self):
        # Letting TimeoutExpired propagate cost a completed run over 14 tools
        # once - every result lost to one hung subprocess.
        self.assertFalse(observer.run_tests("test_zzz_no_such_module", 0.001))

    def test_a_failing_suite_is_reported_as_failing(self):
        self.assertFalse(observer.run_tests("test_zzz_no_such_module", 30))


class ReportWriteTests(unittest.TestCase):
    """A census costs over an hour; it must not be all-or-nothing."""

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.path = Path(self.temporary.name) / "report.json"

    def read(self):
        return json.loads(self.path.read_text(encoding="utf-8"))

    def test_a_report_is_written_where_none_existed(self):
        observer.write_report(self.path, {"a.py": {"survived": 1}})
        self.assertEqual({"a.py": {"survived": 1}}, self.read())

    def test_a_later_tool_does_not_erase_an_earlier_one(self):
        # This is what makes writing after every tool safe rather than
        # destructive, and what makes a single-tool re-measure cheap.
        observer.write_report(self.path, {"a.py": {"survived": 1}})
        observer.write_report(self.path, {"b.py": {"survived": 2}})
        self.assertEqual({"a.py", "b.py"}, set(self.read()))

    def test_re_measuring_a_tool_replaces_its_entry(self):
        observer.write_report(self.path, {"a.py": {"survived": 9}})
        observer.write_report(self.path, {"a.py": {"survived": 1}})
        self.assertEqual(1, self.read()["a.py"]["survived"])

    def test_an_unreadable_report_is_replaced_rather_than_aborting(self):
        # Losing a corrupt file is bad; losing the run that would replace it is
        # worse.
        self.path.write_text("{ not json", encoding="utf-8")
        observer.write_report(self.path, {"a.py": {"survived": 1}})
        self.assertEqual({"a.py"}, set(self.read()))

    def test_no_report_path_is_not_an_error(self):
        observer.write_report(None, {"a.py": {"survived": 1}})


class InterruptedSweepTests(unittest.TestCase):
    """A sweep that stops part-way must leave what it measured on disk.

    THE DEFECT. The write happened once, after the loop, so a run that stopped -
    for any reason, including the operator realising the remaining tools cannot
    change the answer - discarded every result it had paid for. Demonstrated
    2026-08-02: seventeen tools, thirty minutes, every result present in the log
    and none of it in the JSON. The log is not the artifact.

    The stop is staged with a second tool whose own suite fails unmutated, which
    is the sweep's existing "nothing can be measured from it" exit.
    """

    ALPHA = "def value():\n    return 7\n"
    ALPHA_TEST = textwrap.dedent("""\
        import unittest
        import tool_alpha


        class AlphaTests(unittest.TestCase):
            def test_value(self):
                self.assertEqual(7, tool_alpha.value())
        """)
    OMEGA = "def value():\n    return 9\n"
    OMEGA_TEST = textwrap.dedent("""\
        import unittest


        class OmegaTests(unittest.TestCase):
            def test_this_suite_is_red(self):
                self.fail("staged: this suite does not pass unmutated")
        """)

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        for name, text in (("tool_alpha.py", self.ALPHA),
                           ("test_tool_alpha.py", self.ALPHA_TEST),
                           ("tool_omega.py", self.OMEGA),
                           ("test_tool_omega.py", self.OMEGA_TEST)):
            (self.root / name).write_text(text, encoding="utf-8")
        self.saved = observer.TOOLS
        observer.TOOLS = self.root
        self.addCleanup(self.restore)
        self.report = self.root / "census.json"

    def restore(self):
        observer.TOOLS = self.saved

    def run_sweep(self):
        arguments = argparse.Namespace(limit=2, timeout=60.0, report=self.report,
                                       baseline=None)
        candidates = [self.root / "tool_alpha.py", self.root / "tool_omega.py"]
        with contextlib.redirect_stdout(io.StringIO()) as out, \
                contextlib.redirect_stderr(io.StringIO()):
            status = observer.sweep(candidates, arguments)
        return status, out.getvalue()

    def census(self):
        # Named refusal rather than a bare FileNotFoundError, so a regression
        # says what went wrong instead of only that something did.
        self.assertTrue(self.report.is_file(),
                        "the sweep stopped and wrote no census at all; every "
                        "tool it had already paid for is lost")
        return json.loads(self.report.read_text(encoding="utf-8"))

    def test_the_sweep_stops_where_it_was_staged_to_stop(self):
        # The control: if it ran to completion the census below would prove
        # nothing about interruption.
        status, printed = self.run_sweep()
        self.assertEqual(1, status)
        self.assertIn("tool_alpha.py", printed)

    def test_the_finished_tool_is_on_disk_after_the_stop(self):
        # THE PROPERTY. Under the end-of-loop write this file does not exist.
        self.run_sweep()
        census = self.census()
        self.assertIn("tool_alpha.py", census)
        self.assertGreater(census["tool_alpha.py"]["sampled"], 0)

    def test_the_unreached_tool_is_absent_rather_than_reported_empty(self):
        # A zero row for a tool nobody measured would read as full coverage.
        self.run_sweep()
        self.assertNotIn("tool_omega.py", self.census())

    def test_the_census_records_how_much_of_the_tool_was_looked_at(self):
        self.run_sweep()
        entry = self.census()["tool_alpha.py"]
        self.assertGreaterEqual(entry["eligible"], entry["sampled"])
        self.assertIsNotNone(entry["line_span"])

    def test_the_tool_is_restored_after_the_sweep(self):
        # It rewrites files in tools/; a stranded mutant is read by everything
        # that comes next.
        self.run_sweep()
        self.assertEqual(self.ALPHA,
                         (self.root / "tool_alpha.py").read_text(encoding="utf-8"))

    def test_a_check_with_no_test_file_is_counted_not_skipped(self):
        # Deriving candidates from the gate brings in checks nobody has written
        # tests for; aborting on the first would make the fix unusable, and
        # skipping it hides a coverage of exactly zero.
        (self.root / "test_tool_alpha.py").unlink()
        arguments = argparse.Namespace(limit=2, timeout=60.0, report=self.report,
                                       baseline=None)
        with contextlib.redirect_stdout(io.StringIO()) as out, \
                contextlib.redirect_stderr(io.StringIO()):
            observer.sweep([self.root / "tool_alpha.py"], arguments)
        self.assertIn("NO TEST FILE", out.getvalue())
        entry = self.census()["tool_alpha.py"]
        self.assertTrue(entry["no_test_file"])
        self.assertEqual(0, entry["killed"])


class BytecodeTests(unittest.TestCase):
    def test_a_cached_compile_is_discarded(self):
        # A mutated .pyc left behind failed a unit test against source that was
        # byte-identical to a green commit, and the gate went red for half an
        # hour on an artifact pretending to be a defect.
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            module = root / "victim.py"
            module.write_text("VALUE = 1\n", encoding="utf-8")
            cache = root / "__pycache__"
            cache.mkdir()
            stale = cache / "victim.cpython-312.pyc"
            stale.write_bytes(b"not really bytecode")
            observer.discard_bytecode(module)
            self.assertFalse(stale.exists())

    def test_discarding_is_safe_when_nothing_is_cached(self):
        with tempfile.TemporaryDirectory() as directory:
            module = Path(directory) / "victim.py"
            module.write_text("VALUE = 1\n", encoding="utf-8")
            observer.discard_bytecode(module)


if __name__ == "__main__":
    unittest.main()
