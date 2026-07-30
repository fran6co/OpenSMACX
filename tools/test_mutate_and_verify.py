#!/usr/bin/env python3

import argparse
import os
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import mutate_and_verify


SOURCE = """\
#include "stdafx.h"

/*
Purpose: Clear all vector components.
Original Offset: 006343D0
Status: Complete
*/
void Vector::close() {
    volatile uint32_t *ordered = values_;
    ordered[0] = 0;
    ordered[1] = 0;
}

/*
Purpose: Compare a bound.
Original Offset: 006343E0
Status: Complete
*/
int Vector::at_limit() {
    // The bound below is 3, chosen to match the legacy disassembly at 0x28.
    if (count_ >= 3) {
        return 1;
    }
    return 0;
}
"""


class ParseFunctionsTest(unittest.TestCase):
    def test_locates_each_annotated_body(self):
        functions = mutate_and_verify.parse_functions(SOURCE.splitlines(keepends=True))
        self.assertEqual(["006343D0", "006343E0"], [f.address for f in functions])

    def test_body_excludes_signature_and_closing_brace(self):
        lines = SOURCE.splitlines(keepends=True)
        first = mutate_and_verify.parse_functions(lines)[0]
        body = "".join(lines[first.start:first.end])
        self.assertIn("ordered[0] = 0;", body)
        self.assertNotIn("void Vector::close()", body)
        self.assertNotIn("\n}", body)

    def test_ignores_files_without_offset_annotations(self):
        self.assertEqual([], mutate_and_verify.parse_functions(["int main() {\n", "}\n"]))


class ComparisonPatternTest(unittest.TestCase):
    def _first_match(self, text):
        for pattern, target in mutate_and_verify.COMPARISONS:
            match = pattern.search(text)
            if match:
                return match.group(0), target
        return None

    def test_does_not_fire_inside_arrow_or_shift(self):
        # These produced uncompilable noise before the patterns were anchored.
        self.assertIsNone(self._first_match("a->b = c;"))
        self.assertIsNone(self._first_match("x = y >> 2;"))
        self.assertIsNone(self._first_match("x = y << 2;"))

    def test_rewrites_real_comparisons(self):
        self.assertEqual(("<", "<="), self._first_match("if (a < b) {"))
        self.assertEqual((">=", ">"), self._first_match("if (a >= b) {"))
        self.assertEqual(("==", "!="), self._first_match("if (p == q) {"))


class SimpleStatementTest(unittest.TestCase):
    """`*` leads both block-comment continuations and dereference stores.
    Treating both as comments silently skipped every write through a pointer
    parameter, so an out-parameter function could report full coverage with
    none of its stores ever mutated."""

    def test_dereference_assignments_are_statements(self):
        self.assertTrue(mutate_and_verify.is_simple_statement("    *x += 1;"))
        self.assertTrue(mutate_and_verify.is_simple_statement("    *x -= 5;"))
        self.assertTrue(mutate_and_verify.is_simple_statement(
            "    *reinterpret_cast<int *>(p) = 0;"))

    def test_block_comment_continuations_are_not_statements(self):
        self.assertFalse(mutate_and_verify.is_simple_statement(" * a comment;"))

    def test_empty_abi_barrier_is_not_a_behavioral_statement(self):
        self.assertFalse(mutate_and_verify.is_simple_statement(
            '    __asm__ __volatile__("" : : "a"(this) : "memory");'))

    def test_control_flow_is_still_excluded(self):
        self.assertFalse(mutate_and_verify.is_simple_statement("    return 3;"))


class SplitAssignmentTest(unittest.TestCase):
    """A subscript containing whitespace (`ordered[0x04 / 4]`) is not a
    declaration and must never have its type-specifier stripping applied to
    it. `.split()[-1]` on the raw target used to grab the last whitespace
    token - `4]` - which collapsed every offset with the same divisor onto
    one fake identifier and made distinct array stores look like a
    write-after-write on a single lvalue."""

    def test_distinct_offsets_with_internal_whitespace_stay_distinct(self):
        first = mutate_and_verify.split_assignment("ordered[0x04 / 4] = 0;")[0]
        second = mutate_and_verify.split_assignment("ordered[0x0C / 4] = 0;")[0]
        self.assertNotEqual(first, second)
        self.assertEqual("ordered[0x04/4]", first)

    def test_member_access_target_is_kept_verbatim(self):
        self.assertEqual(
            "self->field", mutate_and_verify.split_assignment("self->field = 1;")[0])

    def test_declaration_specifier_still_stripped_without_brackets(self):
        self.assertEqual("b", mutate_and_verify.split_assignment("int b = a;")[0])
        self.assertEqual(
            "p", mutate_and_verify.split_assignment("uint32_t *p = q;")[0])


class StatementDependenceTest(unittest.TestCase):
    def test_independent_stores_are_equivalent_mutants(self):
        # Distinct lvalues with constant right-hand sides cannot observe order.
        self.assertFalse(
            mutate_and_verify.statements_interact("ordered[0] = 0;", "ordered[1] = 0;"))
        self.assertFalse(
            mutate_and_verify.statements_interact("self->a = 1;", "self->b = 2;"))

    def test_distinct_offsets_with_internal_whitespace_are_equivalent(self):
        # The regression case: before the split_assignment fix these collapsed
        # onto the same fake target and were (accidentally, and for every such
        # pair in the codebase) reported as a coverage-hole swap.
        self.assertFalse(mutate_and_verify.statements_interact(
            "ordered[0x04 / 4] = 0;", "ordered[0x0C / 4] = 0;"))

    def test_independent_declarations_are_equivalent_mutants(self):
        self.assertFalse(mutate_and_verify.statements_interact(
            "StringStructEntry *next = current_->next;",
            "uint32_t position = current_position_ + 1U;"))

    def test_write_after_write_interacts(self):
        self.assertTrue(
            mutate_and_verify.statements_interact("ordered[0] = 0;", "ordered[0] = 1;"))

    def test_read_after_write_interacts(self):
        self.assertTrue(mutate_and_verify.statements_interact("a = 1;", "b = a;"))
        self.assertTrue(
            mutate_and_verify.statements_interact("int a = 1;", "int b = a;"))
        self.assertTrue(
            mutate_and_verify.statements_interact("uint32_t *p = q;", "uint32_t *r = p;"))

    def test_opaque_calls_are_kept(self):
        self.assertTrue(mutate_and_verify.statements_interact(
            "current_ = next;", "memcpy(&a, &b, 4);"))


class BuildMutantsTest(unittest.TestCase):
    def _mutants(self, address):
        lines = SOURCE.splitlines(keepends=True)
        function = next(f for f in mutate_and_verify.parse_functions(lines)
                        if f.address == address)
        return lines, mutate_and_verify.build_mutants(lines, function)

    def test_emits_expected_operators(self):
        _, mutants = self._mutants("006343D0")
        self.assertEqual({"drop-statement", "constant"},
                         {m.operator for m in mutants})

    def test_skips_equivalent_swap_of_independent_stores(self):
        _, mutants = self._mutants("006343D0")
        self.assertEqual([], [m for m in mutants if m.operator == "swap-adjacent"])

    def test_skips_comment_lines(self):
        # A literal inside a `//` comment (`0x28` here) has no compiled
        # effect, so mutating it would always survive - not a coverage hole,
        # just a wasted build. Confirmed against real cases in font_recovery.cpp
        # and sprite.cpp where such comments produced only noise.
        _, mutants = self._mutants("006343E0")
        self.assertFalse(
            any("0x28" in m.description for m in mutants),
            "a comment-only literal must not be mutated")

    def test_mutant_lines_differ_from_original_by_construction(self):
        lines, mutants = self._mutants("006343D0")
        for mutant in mutants:
            self.assertNotEqual(lines, mutant.lines, mutant.description)

    def test_comparison_operator_reaches_guarded_bound(self):
        _, mutants = self._mutants("006343E0")
        self.assertIn("comparison", {m.operator for m in mutants})

    def test_control_flow_lines_are_not_dropped(self):
        _, mutants = self._mutants("006343E0")
        dropped = [m.description for m in mutants if m.operator == "drop-statement"]
        self.assertFalse([d for d in dropped if "return" in d])

    def test_declaration_used_later_yields_no_drop_or_swap_mutant(self):
        # These two operators can only produce a `no compile` on such a line:
        # the use would precede the declaration. Each still costs a full
        # rebuild and a Wine run, and - worse - the wasted mutants sum into a
        # `no compile` total that lets `killed n/n` read as full coverage when
        # statement order was never tested at all.
        source = SOURCE.replace(
            "    ordered[0] = 0;\n",
            "    const uint32_t width = ordered[1];\n"
            "    const uint32_t height = ordered[2];\n"
            "    ordered[0] = width + height;\n")
        lines = source.splitlines(keepends=True)
        function = next(f for f in mutate_and_verify.parse_functions(lines)
                        if f.address == "006343D0")
        mutants = mutate_and_verify.build_mutants(lines, function)
        structural = [m for m in mutants
                      if m.operator in ("drop-statement", "swap-adjacent")
                      and ("const uint32_t width" in m.description
                           or "const uint32_t height" in m.description)]
        self.assertEqual(
            [], [m.description for m in structural],
            "a declaration whose name is used later cannot be dropped or "
            "swapped and still compile")
        # The `constant` operator is unaffected and SHOULD still mutate the
        # literal inside that same declaration: `ordered[1]` -> `ordered[0]`
        # compiles and is a real perturbation. Suppressing by line rather than
        # by operator would throw that away.
        self.assertTrue(
            any(m.operator == "constant" and "const uint32_t width" in m.description
                for m in mutants))

    def test_declaration_not_used_later_is_still_droppable(self):
        # The predicate must key on the name actually being READ later, not on
        # the line looking like a declaration - otherwise it would suppress
        # genuine coverage holes, which is the opposite failure.
        source = SOURCE.replace(
            "    ordered[0] = 0;\n",
            "    const uint32_t unused = 7;\n    ordered[0] = 0;\n")
        lines = source.splitlines(keepends=True)
        function = next(f for f in mutate_and_verify.parse_functions(lines)
                        if f.address == "006343D0")
        mutants = mutate_and_verify.build_mutants(lines, function)
        self.assertTrue(
            any("unused" in m.description and m.operator == "drop-statement"
                for m in mutants),
            "an unread declaration is a real, compilable coverage probe")

    def test_declares_name_used_later_matches_the_house_style(self):
        lines = [
            "    const uint32_t left = rect->left;\n",
            "    int x = 0;\n",
            "    x = left + 1;\n",
        ]
        self.assertTrue(
            mutate_and_verify.declares_name_used_later(lines, 0, len(lines)))
        self.assertTrue(
            mutate_and_verify.declares_name_used_later(lines, 1, len(lines)))
        # A plain store declares nothing.
        self.assertFalse(
            mutate_and_verify.declares_name_used_later(lines, 2, len(lines)))

    def test_skips_equivalent_and_invalid_divided_index_mutants(self):
        source = SOURCE.replace(
            "ordered[0] = 0;",
            "ordered[0x000 / 4] = 0;")
        lines = source.splitlines(keepends=True)
        function = mutate_and_verify.parse_functions(lines)[0]
        constants = [
            mutant for mutant in mutate_and_verify.build_mutants(lines, function)
            if mutant.operator == "constant"
            and "ordered[0x000 / 4] = 0;" in mutant.description]
        self.assertEqual(1, len(constants))
        self.assertIn("ordered[0x000 / 4] = 1;", "".join(constants[0].lines))

    def test_mutates_suffixed_literals_and_keeps_the_suffix(self):
        # `1U` used to be exempt entirely: the `(?![\w.])` guard saw the `U`
        # as a word character and refused the match, so suffixed constants -
        # including the edge arithmetic in Buffer::box - were never perturbed.
        # The suffix must survive into the replacement so the mutant keeps the
        # original expression's type and still compiles.
        source = SOURCE.replace(
            "ordered[0] = 0;",
            "ordered[0] = value + 1U;")
        lines = source.splitlines(keepends=True)
        function = mutate_and_verify.parse_functions(lines)[0]
        constants = [
            mutant for mutant in mutate_and_verify.build_mutants(lines, function)
            if mutant.operator == "constant"
            and "value + 1U" in mutant.description]
        mutated_sources = ["".join(mutant.lines) for mutant in constants]
        self.assertTrue(any("value + 0U;" in source
                            for source in mutated_sources))
        self.assertFalse(any("value + 0;" in source
                             for source in mutated_sources))

    def test_mutates_repeated_literals_by_occurrence(self):
        lines, mutants = self._mutants("006343D0")
        constants = [
            mutant for mutant in mutants
            if mutant.operator == "constant"
            and "ordered[0] = 0;" in mutant.description]
        mutated_sources = ["".join(mutant.lines) for mutant in constants]
        self.assertTrue(any("ordered[1] = 0;" in source
                            for source in mutated_sources))
        self.assertTrue(any("ordered[0] = 1;" in source
                            for source in mutated_sources))
        self.assertTrue(all(source != "".join(lines) for source in mutated_sources))


class HarnessCommandTest(unittest.TestCase):
    def test_ctest_refuses_to_pass_on_zero_matched_tests(self):
        # `ctest -R` with a no-match pattern exits zero; without
        # --no-tests=error a misspelled --test silently turns every mutant
        # into a survivor.
        args = argparse.Namespace(build_dir=".", target="t", test="t",
                                  timeout=60)
        harness = mutate_and_verify.Harness(args)
        captured = {}

        def fake_run(command, cwd, timeout):
            captured["command"] = command
            return mutate_and_verify.PASSED

        harness._run = fake_run
        harness.check()
        self.assertIn("--no-tests=error", captured["command"])

    def test_owned_wine_prefix_reuse_is_opt_in_and_cleaned(self):
        args = argparse.Namespace(
            build_dir=".", target="t", test="t", timeout=60,
            reuse_owned_wine_prefix=True)
        harness = mutate_and_verify.Harness(args)
        environments = []

        def fake_run(command, cwd, timeout):
            environments.append(os.environ.get(
                mutate_and_verify.KEEP_OWNED_PREFIX_ENV))
            return mutate_and_verify.PASSED

        harness._run = fake_run
        harness.check()
        harness.cleanup()
        self.assertEqual(["1", None], environments)
        self.assertNotIn(
            mutate_and_verify.KEEP_OWNED_PREFIX_ENV, os.environ)


class HarnessClassificationTest(unittest.TestCase):
    """The three outcomes must stay distinct: a mutant that never compiled is
    not evidence that the suite observes anything."""

    def _run(self, build_ok, test_passes, confirm=0):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "sample.cpp"
            source.write_text(SOURCE)
            # The artifact has to exist: main() refuses a baseline that produced
            # no binary, because a staleness check over a file that is never
            # there compares None with None and answers "fresh" every time.
            (Path(directory) / "t.exe").write_bytes(b"MZ")
            args = argparse.Namespace(
                source=str(source), build_dir=directory, target="t", test="t",
                address=[], operator=["constant"], limit=1, timeout=60,
                artifact=None, confirm_survivors=confirm)
            # The trailing PASSED is the rebuild main() performs after restoring
            # the original source in its finally block.
            with mock.patch.object(mutate_and_verify.Harness, "build",
                                   side_effect=list(build_ok)
                                   + [mutate_and_verify.PASSED]), \
                 mock.patch.object(mutate_and_verify.Harness, "check",
                                   side_effect=test_passes), \
                 mock.patch("sys.argv", ["mutate_and_verify", str(source)]), \
                 mock.patch("argparse.ArgumentParser.parse_args", return_value=args):
                code = mutate_and_verify.main()
            return code, source.read_text()

    PASSED = mutate_and_verify.PASSED
    FAILED = mutate_and_verify.FAILED
    TIMEOUT = mutate_and_verify.TIMEOUT
    STALE = mutate_and_verify.STALE

    def _harness(self, directory, artifact=None):
        args = argparse.Namespace(
            source="x.cpp", build_dir=directory, target="t", test="t",
            timeout=60, artifact=artifact)
        return mutate_and_verify.Harness(args)

    def test_a_survivor_confirmed_by_a_RE_RUN_is_still_a_survivor(self):
        # baseline build, baseline check, mutant build, then the mutant check
        # plus two confirmations, all passing.
        code, _ = self._run([self.PASSED, self.PASSED],
                            [self.PASSED, self.PASSED, self.PASSED, self.PASSED],
                            confirm=2)
        self.assertEqual(1, code)

    def test_a_survivor_the_RE_RUN_kills_is_a_kill_not_a_coverage_hole(self):
        # The measured flake: three sweeps kill a mutant and one lets it
        # through, always in that direction. One re-observation removes it, so
        # the run must exit clean rather than report a hole that is not there.
        code, _ = self._run([self.PASSED, self.PASSED],
                            [self.PASSED, self.PASSED, self.FAILED],
                            confirm=2)
        self.assertEqual(0, code)

    def test_confirmation_stops_at_the_first_kill(self):
        # Cheapness is the reason this is affordable: it must not keep re-running
        # after it already has its answer.
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / "t.exe").write_bytes(b"MZ")
            harness = self._harness(directory)
            with mock.patch.object(
                    harness, "check",
                    side_effect=[mutate_and_verify.FAILED]) as check:
                self.assertEqual(self.FAILED, harness.confirm_survivor(5))
            self.assertEqual(1, check.call_count)

    def test_confirmation_can_be_disabled(self):
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / "t.exe").write_bytes(b"MZ")
            harness = self._harness(directory)
            with mock.patch.object(harness, "check") as check:
                self.assertEqual(self.PASSED, harness.confirm_survivor(0))
            check.assert_not_called()

    def test_a_build_that_exits_zero_without_replacing_the_binary_is_STALE(self):
        # The link in the chain of evidence nothing used to check: source ->
        # binary -> suite. A build that reports success and leaves the binary
        # alone means the suite measured the PREVIOUS mutant.
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / "t.exe").write_bytes(b"MZ")
            harness = self._harness(directory)
            with mock.patch.object(harness, "_run",
                                   return_value=mutate_and_verify.PASSED):
                self.assertEqual(self.STALE, harness.build())

    def test_a_build_that_replaces_the_binary_is_PASSED(self):
        with tempfile.TemporaryDirectory() as directory:
            artifact = Path(directory) / "t.exe"
            artifact.write_bytes(b"MZ")
            harness = self._harness(directory)

            def touch(*_args, **_kwargs):
                artifact.write_bytes(b"MZ-rebuilt-and-longer")
                return mutate_and_verify.PASSED

            with mock.patch.object(harness, "_run", side_effect=touch):
                self.assertEqual(self.PASSED, harness.build())

    def test_a_replaced_INODE_counts_as_fresh(self):
        # A toolchain that writes a temporary and renames it over the target can
        # land identical mtime and size; reading that as STALE would report a
        # real measurement as unmeasured.
        with tempfile.TemporaryDirectory() as directory:
            artifact = Path(directory) / "t.exe"
            artifact.write_bytes(b"MZ")
            stat = artifact.stat()
            harness = self._harness(directory)

            def rename_over(*_args, **_kwargs):
                other = Path(directory) / "t.exe.new"
                other.write_bytes(b"MZ")
                os.utime(other, ns=(stat.st_atime_ns, stat.st_mtime_ns))
                other.replace(artifact)
                return mutate_and_verify.PASSED

            with mock.patch.object(harness, "_run", side_effect=rename_over):
                after = artifact.stat()
                self.assertEqual(stat.st_size, after.st_size)
                self.assertEqual(self.PASSED, harness.build())

    def test_a_build_that_deletes_the_binary_is_STALE_not_PASSED(self):
        with tempfile.TemporaryDirectory() as directory:
            artifact = Path(directory) / "t.exe"
            artifact.write_bytes(b"MZ")
            harness = self._harness(directory)

            def remove(*_args, **_kwargs):
                artifact.unlink()
                return mutate_and_verify.PASSED

            with mock.patch.object(harness, "_run", side_effect=remove):
                self.assertEqual(self.STALE, harness.build())

    def test_an_explicit_artifact_path_overrides_the_default(self):
        with tempfile.TemporaryDirectory() as directory:
            elsewhere = Path(directory) / "sub" / "other.exe"
            harness = self._harness(directory, artifact=str(elsewhere))
            self.assertEqual(elsewhere.resolve(), harness.artifact)

    def test_a_STALE_mutant_is_neither_killed_nor_survived(self):
        # It must not read as a kill (which would claim coverage that was never
        # measured) nor as a survivor (which would claim a coverage hole that may
        # not exist). It fails the run either way, so `N/N killed` cannot be
        # printed over a sweep that skipped some of its own mutants.
        code, _ = self._run([self.PASSED, self.STALE, self.PASSED],
                            [self.PASSED])
        self.assertEqual(1, code)

    def test_a_missing_artifact_after_the_baseline_is_refused(self):
        # Without this the staleness check compares None with None on every
        # mutant and silently answers "fresh" - the check would be inert and
        # nothing would say so.
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "sample.cpp"
            source.write_text(SOURCE)
            args = argparse.Namespace(
                source=str(source), build_dir=directory, target="t", test="t",
                address=[], operator=["constant"], limit=1, timeout=60,
                artifact=None)
            with mock.patch.object(mutate_and_verify.Harness, "build",
                                   return_value=mutate_and_verify.PASSED), \
                 mock.patch.object(mutate_and_verify.Harness, "check",
                                   return_value=mutate_and_verify.PASSED), \
                 mock.patch("sys.argv", ["mutate_and_verify", str(source)]), \
                 mock.patch("argparse.ArgumentParser.parse_args",
                            return_value=args):
                self.assertEqual(2, mutate_and_verify.main())

    def test_a_TIMEOUT_reaps_the_processes_still_holding_the_binary(self):
        # subprocess's timeout kills ctest and nothing under it, so the wine
        # process running the test binary outlives it and can still hold the
        # file the next mutant needs to link over.
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / "t.exe").write_bytes(b"MZ")
            harness = self._harness(directory)
            with mock.patch.object(harness, "_run",
                                   return_value=mutate_and_verify.TIMEOUT), \
                 mock.patch.object(mutate_and_verify,
                                   "stop_executable_processes",
                                   return_value=True) as reap:
                self.assertEqual(self.TIMEOUT, harness.check())
            reap.assert_called_once_with(harness.artifact)

    def test_a_PASSING_check_reaps_nothing(self):
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / "t.exe").write_bytes(b"MZ")
            harness = self._harness(directory)
            with mock.patch.object(harness, "_run",
                                   return_value=mutate_and_verify.PASSED), \
                 mock.patch.object(mutate_and_verify,
                                   "stop_executable_processes") as reap:
                self.assertEqual(self.PASSED, harness.check())
            reap.assert_not_called()

    def test_survivor_reports_failure_exit_code(self):
        # baseline build, baseline test, mutant build, mutant test(passes)
        code, _ = self._run([self.PASSED, self.PASSED],
                            [self.PASSED, self.PASSED])
        self.assertEqual(1, code)

    def test_killed_mutant_exits_clean(self):
        code, _ = self._run([self.PASSED, self.PASSED],
                            [self.PASSED, self.FAILED])
        self.assertEqual(0, code)

    def test_hung_mutant_counts_as_killed_not_as_a_crash(self):
        # A perturbation that hangs the suite has been observed by it. Letting
        # the timeout propagate aborted a whole run mid-sweep.
        code, _ = self._run([self.PASSED, self.PASSED],
                            [self.PASSED, self.TIMEOUT])
        self.assertEqual(0, code)

    def test_uncompilable_mutant_is_not_counted_as_killed(self):
        code, _ = self._run([self.PASSED, self.FAILED, self.PASSED],
                            [self.PASSED])
        self.assertEqual(0, code)

    def test_failing_baseline_aborts(self):
        code, _ = self._run([self.PASSED], [self.FAILED])
        self.assertEqual(2, code)

    def test_source_is_restored_after_run(self):
        _, restored = self._run([self.PASSED, self.PASSED],
                                [self.PASSED, self.FAILED])
        self.assertEqual(SOURCE, restored)

    def test_source_is_restored_when_baseline_fails(self):
        _, restored = self._run([True], [self.FAILED])
        self.assertEqual(SOURCE, restored)


class IdenticalStatementSwapTests(unittest.TestCase):
    """Swapping a statement with its identical neighbour is the program."""

    def test_two_identical_calls_do_not_interact(self):
        # Four recovered bubble-dismiss handlers are literally
        #     Win::clear_bubble_text(); Win::clear_bubble_text();
        # and each reported a swap survivor that no fixture could ever close,
        # because the swapped file is byte-identical to the original.
        self.assertFalse(mutate_and_verify.statements_interact(
            "    ::Win::clear_bubble_text();",
            "    ::Win::clear_bubble_text();"))

    def test_indentation_alone_does_not_make_them_differ(self):
        self.assertFalse(mutate_and_verify.statements_interact(
            "  foo();", "        foo();"))

    def test_two_different_calls_still_interact(self):
        # The general rule is unchanged: an opaque call may have side effects,
        # so a genuine pair is still worth a build.
        self.assertTrue(mutate_and_verify.statements_interact("    foo();", "    bar();"))

    def test_a_write_after_write_still_interacts(self):
        self.assertTrue(mutate_and_verify.statements_interact(
            "    x_ = 1;", "    x_ = 2;"))


if __name__ == "__main__":
    unittest.main()
