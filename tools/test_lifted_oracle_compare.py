#!/usr/bin/env python3
"""Tests for lifted_oracle_compare.

The tool's whole job is to report a direction, so the tests that matter are
the ones that would catch it reporting the WRONG one. A comparator that prints
plausible numbers with the sign backwards is worse than no comparator, because
it is believed.
"""

from __future__ import annotations

import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lifted_oracle_compare as compare  # noqa: E402
import lifted_oracle_summary as summary  # noqa: E402

HEADER = "address\tverdict\tcases\tcompared\tdetail\tname\n"


def write_report(directory: Path, name: str, rows) -> Path:
    """rows: (address, verdict, cases, compared, detail, name)."""
    path = directory / name
    with path.open("w") as handle:
        handle.write(HEADER)
        for row in rows:
            handle.write("\t".join(str(part) for part in row) + "\n")
    return path


# One catalogue for every test: three functions with distinct, obvious sizes so
# a byte total can only come from one combination of them.
CATALOGUE = ("address,size,name,recovery_state,body_ranges\n"
             "0x00401000,1000,big,unrecovered,0x00401000-0x004013e8\n"
             "0x00402000,100,middle,unrecovered,0x00402000-0x00402064\n"
             "0x00403000,10,small,unrecovered,0x00403000-0x0040300a\n")


class CompareTestCase(unittest.TestCase):
    def setUp(self):
        self._directory = tempfile.TemporaryDirectory()
        self.root = Path(self._directory.name)
        self.functions = self.root / "functions.csv"
        self.functions.write_text(CATALOGUE)
        self.addCleanup(self._directory.cleanup)

    def run_compare(self, before_rows, after_rows, *extra):
        before = write_report(self.root, "before.tsv", before_rows)
        after = write_report(self.root, "after.tsv", after_rows)
        captured = io.StringIO()
        with contextlib.redirect_stdout(captured):
            status = compare.main([str(before), str(after),
                                   "--functions", str(self.functions), *extra])
        return status, captured.getvalue()


class DirectionTests(CompareTestCase):
    """`better`/`worse` is the output; a signed delta is not enough."""

    def test_more_agreement_is_better(self):
        _, text = self.run_compare(
            [("0x00401000", "FAIL", 16, 16, "", "big")],
            [("0x00401000", "PASS", 16, 16, "", "big")])
        agreed = next(l for l in text.splitlines()
                      if l.strip().startswith("agreed "))
        self.assertIn("better", agreed)
        self.assertIn("+1000", agreed)

    def test_less_agreement_is_worse(self):
        _, text = self.run_compare(
            [("0x00401000", "PASS", 16, 16, "", "big")],
            [("0x00401000", "FAIL", 16, 16, "", "big")])
        agreed = next(l for l in text.splitlines()
                      if l.strip().startswith("agreed "))
        self.assertIn("worse", agreed)

    def test_a_must_go_down_figure_rising_is_worse(self):
        # The inversion this tool exists for. `never_compared` going UP is a
        # regression, and a comparator that printed "+N better" here would be
        # the dashboard error this project already made once.
        _, text = self.run_compare(
            [("0x00401000", "PASS", 16, 16, "", "big")],
            [("0x00401000", "SKIP", 0, 0, "", "big")])
        line = next(l for l in text.splitlines()
                    if l.strip().startswith("never_compared"))
        self.assertIn("worse", line)
        self.assertIn("must go down", line)

    def test_a_must_go_down_figure_falling_is_better(self):
        _, text = self.run_compare(
            [("0x00401000", "SKIP", 0, 0, "", "big")],
            [("0x00401000", "PASS", 16, 16, "", "big")])
        line = next(l for l in text.splitlines()
                    if l.strip().startswith("never_compared"))
        self.assertIn("better", line)


class FullStrengthTests(CompareTestCase):
    """The two agreement figures move independently and must be printed so."""

    def test_total_can_rise_while_full_strength_falls(self):
        # The exact trade the migration checklist warns about: a large
        # single-seed PASS gained, a small full-strength PASS lost. Reading
        # only `agreed`, this is an improvement; it is not.
        _, text = self.run_compare(
            [("0x00403000", "PASS", 16, 16, "", "small"),
             ("0x00401000", "FAIL", 16, 16, "", "big")],
            [("0x00403000", "FAIL", 16, 16, "", "small"),
             ("0x00401000", "PASS", 16, 1, "", "big")])
        agreed = next(l for l in text.splitlines()
                      if l.strip().startswith("agreed "))
        full = next(l for l in text.splitlines()
                    if l.strip().startswith("agreed_full_strength"))
        self.assertIn("better", agreed)
        self.assertIn("worse", full)

    def test_a_pass_that_loses_a_compared_seed_leaves_full_strength(self):
        _, text = self.run_compare(
            [("0x00402000", "PASS", 16, 16, "", "middle")],
            [("0x00402000", "PASS", 16, 15, "", "middle")])
        self.assertIn("AGREEMENT WEAKENED", text)
        self.assertIn("full-strength -> seeds-incomplete", text)
        full = next(l for l in text.splitlines()
                    if l.strip().startswith("agreed_full_strength"))
        self.assertIn("-100", full)


class RegressionTests(CompareTestCase):
    """Lost agreement is named, listed, and reflected in the exit status."""

    def test_lost_agreement_is_listed_with_its_address(self):
        status, text = self.run_compare(
            [("0x00401000", "PASS", 16, 16, "", "big")],
            [("0x00401000", "INCONCLUSIVE-original-fault", 16, 0, "boom",
              "big")])
        self.assertIn("LOST AGREEMENT", text)
        self.assertIn("0x00401000", text)
        self.assertIn("PASS -> INCONCLUSIVE-original-fault", text)
        self.assertEqual(1, status)

    def test_no_regression_exits_zero(self):
        status, _ = self.run_compare(
            [("0x00401000", "PASS", 16, 16, "", "big")],
            [("0x00401000", "PASS", 16, 16, "", "big")])
        self.assertEqual(0, status)

    def test_a_new_fail_is_called_out_separately(self):
        _, text = self.run_compare(
            [("0x00402000", "PASS", 16, 16, "", "middle")],
            [("0x00402000", "FAIL", 16, 16, "flags ZF differ", "middle")])
        self.assertIn("NEW FAILS", text)
        self.assertIn("flags ZF differ", text)

    def test_gaining_agreement_is_not_a_regression(self):
        status, text = self.run_compare(
            [("0x00401000", "FAIL", 16, 16, "", "big")],
            [("0x00401000", "PASS", 16, 16, "", "big")])
        self.assertEqual(0, status)
        self.assertIn("GAINED AGREEMENT", text)


class AddressSetTests(CompareTestCase):
    """Two reports over different address sets are not silently intersected."""

    def test_a_differing_address_set_is_announced(self):
        _, text = self.run_compare(
            [("0x00401000", "PASS", 16, 16, "", "big"),
             ("0x00402000", "PASS", 16, 16, "", "middle")],
            [("0x00401000", "PASS", 16, 16, "", "big")])
        self.assertIn("ADDRESS SETS DIFFER", text)
        self.assertIn("1 only in before", text)

    def test_identical_sets_say_nothing(self):
        _, text = self.run_compare(
            [("0x00401000", "PASS", 16, 16, "", "big")],
            [("0x00401000", "PASS", 16, 16, "", "big")])
        self.assertNotIn("ADDRESS SETS DIFFER", text)


class AgreementWithTheSummaryTests(unittest.TestCase):
    """The figures must be the summary's, not a second definition of them."""

    def test_the_tier_function_is_the_summarys_own(self):
        # If this ever becomes a local copy, the two tools can disagree about
        # what full-strength means while both look right in isolation.
        self.assertIs(compare.summary.evidence_tier, summary.evidence_tier)

    def test_every_figure_declares_a_direction(self):
        rows = []
        figures = compare.figures(rows)
        self.assertTrue(figures)
        for name, (_, direction) in figures.items():
            self.assertIn(direction,
                          (compare.HIGHER_IS_BETTER, compare.LOWER_IS_BETTER),
                          f"{name} has no stated direction")


if __name__ == "__main__":
    unittest.main()
