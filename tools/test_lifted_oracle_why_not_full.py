#!/usr/bin/env python3
"""Tests for the one-seed-short diagnostic's parsing and cohort selection.

Two things here can quietly produce a wrong answer rather than an error. The
first is pairing: a case whose original never returns prints its header and no
verdict, and pairing that header with the NEXT case's verdict would attribute
one case's fault to another. The second is cohort selection: a FAILING function
that happens to be one seed short is not "nearly proven", it is wrong, and
letting it into a cohort with that label is the flattering-reclassification
failure this project has rules against.
"""

from __future__ import annotations

import csv
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lifted_oracle_why_not_full as why  # noqa: E402


class ParseTests(unittest.TestCase):
    def test_a_faulting_case_carries_its_fault_triple(self):
        cases = why.parse_cases(
            "stage: case 0x0052aeb0/12\n"
            "stage:   -> INCONCLUSIVE-original-fault  code=0xc0000005 "
            "eip=0x0052b73f data=0x592dc72c\n")
        self.assertEqual(1, len(cases))
        self.assertEqual(12, cases[0]["case"])
        self.assertEqual("INCONCLUSIVE-original-fault", cases[0]["verdict"])
        self.assertEqual(0xC0000005, cases[0]["fault_code"])
        self.assertEqual(0x0052B73F, cases[0]["fault_eip"])
        self.assertEqual(0x592DC72C, cases[0]["fault_data"])

    def test_a_clean_pass_has_no_fault_fields(self):
        cases = why.parse_cases(
            "stage: case 0x0052aeb0/9\nstage:   -> PASS\n")
        self.assertEqual("PASS", cases[0]["verdict"])
        self.assertEqual(0, cases[0]["fault_code"])
        self.assertEqual(0, cases[0]["fault_data"])

    def test_a_case_with_no_verdict_is_not_credited_to_the_next_one(self):
        # Case 3's original never returned, so it printed no verdict. Pairing
        # its header with case 4's verdict would report a fault against the
        # wrong seed - and would also hide that a case went unanswered.
        cases = why.parse_cases(
            "stage: case 0x1/3\n"
            "stage: case 0x1/4\n"
            "stage:   -> PASS\n")
        self.assertEqual(1, len(cases))
        self.assertEqual(4, cases[0]["case"])

    def test_a_dereference_of_literal_zero_is_read(self):
        # VERBATIM from a real run of 0x00581170. `%#010x` of zero prints
        # `0000000000` with no `0x`, because C only adds the prefix for a
        # non-zero value. A pattern demanding `0x` drops this line - and this
        # is the single most diagnostic line the cohort can produce, since a
        # null dereference is precisely the unconstructed-global signature.
        cases = why.parse_cases(
            "stage: case 0x00581170/0\n"
            "stage:   -> INCONCLUSIVE-original-timeout  code=0x0dead100 "
            "eip=0x00581180 data=0000000000\n")
        self.assertEqual(1, len(cases))
        self.assertEqual(0x0DEAD100, cases[0]["fault_code"])
        self.assertEqual(0x00581180, cases[0]["fault_eip"])
        self.assertEqual(0, cases[0]["fault_data"])
        self.assertEqual([], why.unparsed_verdict_lines(
            "stage:   -> X  code=0x1 eip=0x2 data=0000000000\n"))

    def test_a_verdict_line_that_cannot_be_read_is_reported_not_dropped(self):
        # The failure mode this guards is silence: an unreadable verdict line
        # looks exactly like a case that never ran.
        stray = why.unparsed_verdict_lines(
            "stage:   -> PASS\n"
            "stage:   -> WEIRD code=?? eip=?? data=??\n")
        self.assertEqual(1, len(stray))
        self.assertIn("WEIRD", stray[0])

    def test_interleaved_noise_between_the_two_lines_is_ignored(self):
        cases = why.parse_cases(
            "stage: case 0x1/0\n"
            "  faulting host 0x10000000 -> guest 0x00400000\n"
            "stage:   -> PASS\n")
        self.assertEqual([0], [case["case"] for case in cases])


class RegionTests(unittest.TestCase):
    def test_a_zeroed_pointer_lands_in_the_null_band(self):
        self.assertEqual("near-null", why.region_of(0x0))
        self.assertEqual("near-null", why.region_of(0x48))
        self.assertEqual("near-null", why.region_of(why.NULL_BAND - 1))

    def test_the_image_span_is_recognised(self):
        self.assertEqual("image", why.region_of(why.IMAGE_BEGIN))
        self.assertEqual("image", why.region_of(why.IMAGE_END - 1))

    def test_the_guard_band_is_distinct_from_the_stack(self):
        # These are adjacent and mean opposite things: the guard band is
        # poisoned and never valid, the stack is normal.
        self.assertEqual("stack-guard", why.region_of(why.IMAGE_END))
        self.assertEqual("stack", why.region_of(why.GUARD_END))
        self.assertEqual("stack", why.region_of(why.STACK_END - 1))

    def test_seed_garbage_outside_every_region_is_wild(self):
        self.assertEqual("wild", why.region_of(0x592DC72C))
        self.assertEqual("wild", why.region_of(why.STACK_END))

    def test_the_top_page_is_called_out_separately(self):
        self.assertEqual("top-page", why.region_of(0xFFFF0000))
        self.assertEqual("top-page", why.region_of(0xFFFFFFFF))


class CohortTests(unittest.TestCase):
    def _row(self, address, verdict, cases, compared):
        return {"address": address, "verdict": verdict, "cases": str(cases),
                "compared": str(compared), "name": "n", "detail": ""}

    def test_exactly_one_short_is_selected(self):
        found = why.cohort([self._row("0x1", "PASS", 16, 15)], 1, {})
        self.assertEqual([0x1], [entry["address"] for entry in found])

    def test_two_short_is_not_one_short(self):
        self.assertEqual([], why.cohort([self._row("0x1", "PASS", 16, 14)],
                                        1, {}))

    def test_a_full_strength_function_is_not_in_the_cohort(self):
        self.assertEqual([], why.cohort([self._row("0x1", "PASS", 16, 16)],
                                        1, {}))

    def test_a_FAILING_function_never_enters_a_nearly_proven_cohort(self):
        # The reason this test exists: one seed short of agreement is a
        # completely different statement from disagreeing, and a cohort
        # labelled "nearly full strength" must not contain a defect.
        self.assertEqual([], why.cohort([self._row("0x1", "FAIL", 16, 15)],
                                        1, {}))

    def test_a_function_that_judged_nothing_is_excluded(self):
        # compared == 0 means no seed was judged at all, which is not "nearly"
        # anything.
        self.assertEqual([], why.cohort([self._row("0x1", "PASS", 16, 0)],
                                        16, {}))

    def test_the_cohort_is_ordered_by_size_so_the_prize_reads_first(self):
        rows = [self._row("0x1", "PASS", 16, 15),
                self._row("0x2", "PASS", 16, 15)]
        found = why.cohort(rows, 1, {0x1: ("small", 10), 0x2: ("big", 3970)})
        self.assertEqual([0x2, 0x1], [entry["address"] for entry in found])
        self.assertEqual(3970, found[0]["size"])


class OutputTests(unittest.TestCase):
    def _results(self, cases, compared=15, size=100):
        return [{"address": 0x52AEB0, "name": "f", "size": size,
                 "seeds_short": 1, "cases": 16, "compared": compared,
                 "cases_seen": cases, "timed_out": False}]

    def test_only_the_non_comparable_cases_are_written(self):
        results = self._results([
            {"case": 0, "verdict": "PASS", "fault_code": 0, "fault_eip": 0,
             "fault_data": 0},
            {"case": 1, "verdict": "INCONCLUSIVE-original-fault",
             "fault_code": 0xC0000005, "fault_eip": 0x52B73F,
             "fault_data": 0x48},
        ])
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory) / "one-short.tsv"
            self.assertEqual(1, why.write_rows(out, results))
            raw = out.read_bytes()
            # Every other committed catalogue in this repo is LF; csv defaults
            # to CRLF, which has already cost one --check mismatch here.
            self.assertNotIn(b"\r\n", raw)
            with out.open(newline="", encoding="utf-8") as handle:
                rows = list(csv.DictReader(handle, delimiter="\t"))
            self.assertEqual(1, len(rows))
            self.assertEqual("1", rows[0]["case"])
            self.assertEqual("near-null", rows[0]["region"])

    def test_a_non_fault_verdict_gets_no_region(self):
        results = self._results([
            {"case": 0, "verdict": "INCONCLUSIVE-no-effect", "fault_code": 0,
             "fault_eip": 0, "fault_data": 0}])
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory) / "o.tsv"
            why.write_rows(out, results)
            with out.open(newline="", encoding="utf-8") as handle:
                rows = list(csv.DictReader(handle, delimiter="\t"))
            # region_of(0) would say "near-null", which would be a claim about
            # a fault that never happened.
            self.assertEqual("", rows[0]["region"])


class SummaryTests(unittest.TestCase):
    def _result(self, cases, compared):
        return {"address": 0x1, "name": "f", "size": 100, "seeds_short": 1,
                "cases": 16, "compared": compared, "cases_seen": cases,
                "timed_out": False}

    def test_agreement_between_report_and_replay_is_reported_as_none_missing(self):
        cases = [{"case": 0, "verdict": "PASS", "fault_code": 0,
                  "fault_eip": 0, "fault_data": 0},
                 {"case": 1, "verdict": "INCONCLUSIVE-original-fault",
                  "fault_code": 0xC0000005, "fault_eip": 0x2,
                  "fault_data": 0x592DC72C}]
        found = why.summarise([self._result(cases, compared=1)])
        self.assertEqual([], found["disagreed"])
        self.assertEqual(1, found["verdicts"]["INCONCLUSIVE-original-fault"])
        self.assertEqual(1, found["regions"]["wild"])
        self.assertEqual(100, found["blocked_bytes"]["wild"])

    def test_a_PASS_is_not_counted_as_a_reason_the_function_fell_short(self):
        # The verdict distribution is the headline this tool exists to print,
        # and it is a distribution over the cases that could NOT be judged. A
        # leaked PASS would inflate it with the cases that worked - which is
        # the one direction the number must not be able to move.
        cases = [{"case": 0, "verdict": "PASS", "fault_code": 0,
                  "fault_eip": 0, "fault_data": 0},
                 {"case": 1, "verdict": "INCONCLUSIVE-original-fault",
                  "fault_code": 0xC0000005, "fault_eip": 0x2,
                  "fault_data": 0x592DC72C}]
        found = why.summarise([self._result(cases, compared=1)])
        self.assertNotIn("PASS", found["verdicts"])
        self.assertEqual(1, sum(found["verdicts"].values()))

    def test_a_function_owing_a_reason_it_did_not_give_is_named(self):
        # One seed short, zero reasons recorded. The judged count still agrees
        # with the report, so the other cross-check stays quiet - which is
        # exactly how three functions slipped through the first real run.
        cases = [{"case": 0, "verdict": "PASS", "fault_code": 0,
                  "fault_eip": 0, "fault_data": 0}]
        found = why.summarise([self._result(cases, compared=1)])
        self.assertEqual([], found["disagreed"])
        self.assertEqual([(0x1, 1, 0)], found["unexplained"])

    def test_a_function_that_gave_its_reason_is_not_flagged(self):
        cases = [{"case": 0, "verdict": "PASS", "fault_code": 0,
                  "fault_eip": 0, "fault_data": 0},
                 {"case": 1, "verdict": "INCONCLUSIVE-original-fault",
                  "fault_code": 0xC0000005, "fault_eip": 0x2,
                  "fault_data": 0x0}]
        found = why.summarise([self._result(cases, compared=1)])
        self.assertEqual([], found["unexplained"])
        self.assertEqual(1, found["regions"]["near-null"])

    def test_a_report_that_disagrees_with_the_replay_is_named(self):
        # Neither number is trusted over the other; the disagreement itself is
        # the finding, because the two describe the same binary on the same
        # seeds and cannot both be right.
        cases = [{"case": 0, "verdict": "PASS", "fault_code": 0,
                  "fault_eip": 0, "fault_data": 0}]
        found = why.summarise([self._result(cases, compared=15)])
        self.assertEqual([(0x1, 15, 1)], found["disagreed"])


if __name__ == "__main__":
    unittest.main()
