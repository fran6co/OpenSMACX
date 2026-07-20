#!/usr/bin/env python3

import argparse
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


class StatementDependenceTest(unittest.TestCase):
    def test_independent_stores_are_equivalent_mutants(self):
        # Distinct lvalues with constant right-hand sides cannot observe order.
        self.assertFalse(
            mutate_and_verify.statements_interact("ordered[0] = 0;", "ordered[1] = 0;"))
        self.assertFalse(
            mutate_and_verify.statements_interact("self->a = 1;", "self->b = 2;"))

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


class HarnessClassificationTest(unittest.TestCase):
    """The three outcomes must stay distinct: a mutant that never compiled is
    not evidence that the suite observes anything."""

    def _run(self, build_ok, test_passes):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "sample.cpp"
            source.write_text(SOURCE)
            args = argparse.Namespace(
                source=str(source), build_dir=directory, target="t", test="t",
                address=[], operator=["constant"], limit=1, timeout=60)
            # The trailing True is the rebuild main() performs after restoring
            # the original source in its finally block.
            with mock.patch.object(mutate_and_verify.Harness, "build",
                                   side_effect=list(build_ok) + [True]), \
                 mock.patch.object(mutate_and_verify.Harness, "check",
                                   side_effect=test_passes), \
                 mock.patch("sys.argv", ["mutate_and_verify", str(source)]), \
                 mock.patch("argparse.ArgumentParser.parse_args", return_value=args):
                code = mutate_and_verify.main()
            return code, source.read_text()

    PASSED = mutate_and_verify.PASSED
    FAILED = mutate_and_verify.FAILED
    TIMEOUT = mutate_and_verify.TIMEOUT

    def test_survivor_reports_failure_exit_code(self):
        # baseline build, baseline test, mutant build, mutant test(passes)
        code, _ = self._run([True, True], [self.PASSED, self.PASSED])
        self.assertEqual(1, code)

    def test_killed_mutant_exits_clean(self):
        code, _ = self._run([True, True], [self.PASSED, self.FAILED])
        self.assertEqual(0, code)

    def test_hung_mutant_counts_as_killed_not_as_a_crash(self):
        # A perturbation that hangs the suite has been observed by it. Letting
        # the timeout propagate aborted a whole run mid-sweep.
        code, _ = self._run([True, True], [self.PASSED, self.TIMEOUT])
        self.assertEqual(0, code)

    def test_uncompilable_mutant_is_not_counted_as_killed(self):
        code, _ = self._run([True, False, True], [self.PASSED])
        self.assertEqual(0, code)

    def test_failing_baseline_aborts(self):
        code, _ = self._run([True], [self.FAILED])
        self.assertEqual(2, code)

    def test_source_is_restored_after_run(self):
        _, restored = self._run([True, True], [self.PASSED, self.FAILED])
        self.assertEqual(SOURCE, restored)

    def test_source_is_restored_when_baseline_fails(self):
        _, restored = self._run([True], [self.FAILED])
        self.assertEqual(SOURCE, restored)


if __name__ == "__main__":
    unittest.main()
