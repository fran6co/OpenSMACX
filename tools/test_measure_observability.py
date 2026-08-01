#!/usr/bin/env python3
"""The prober must be able to say OBSERVED, or it measures nothing.

A tool that answers "does anything watch this function" is worthless in one
specific way: if it can only ever answer UNOBSERVED, it looks like a dramatic
finding and is a broken instrument. That is the failure this file exists to
prevent, and it is the same shape as the checks that could not fail.

Validated against two functions whose full sweeps are known, which is the part
no unit test can do: ?repair_phase@@YAXH@Z kills 118 of 126 mutants and the
prober calls it OBSERVED; ?social_ai@@YAXHHHHHPAH@Z kills 0 of 709 and the
prober calls it UNOBSERVED. Those live in docs/RECOVERY_ECONOMICS.md rather
than here because they need a build.
"""
import unittest

import measure_observability as prober


class MaximalMutantTests(unittest.TestCase):
    class Fn:
        def __init__(self, start, end):
            self.address, self.start, self.end = "00400000", start, end

    def test_a_drop_mutant_is_excluded(self):
        # Drops shorten the file, so composing them line-wise would misalign
        # every later replacement and produce a wreck nobody designed.
        lines = ["void f() {", "    int a = 5;", "    call();", "}"]
        by_line = prober.maximal_mutant(lines, self.Fn(1, 3))
        for index, text in by_line.items():
            self.assertLess(index, len(lines))
            self.assertNotEqual(lines[index], text)

    def test_every_perturbable_line_gets_exactly_one_replacement(self):
        lines = ["void f() {", "    int a = 5;", "    int b = 7;", "}"]
        by_line = prober.maximal_mutant(lines, self.Fn(1, 3))
        self.assertEqual(sorted(by_line), [1, 2])

    def test_a_body_with_nothing_to_perturb_yields_nothing(self):
        # Returning an empty plan must not be confused with a wreck that
        # survived; the caller skips these rather than scoring them.
        lines = ["void f() {", "    call();", "}"]
        self.assertFalse(prober.maximal_mutant(lines, self.Fn(1, 2)))


class WreckCandidateTests(unittest.TestCase):
    def test_candidates_shrink_and_are_largest_first(self):
        lines = ["a", "b", "c", "d", "e"]
        by_line = {0: "A", 1: "B", 2: "C", 3: "D"}
        sizes = [count for _, count in prober.wreck_candidates(lines, by_line)]
        self.assertEqual([4, 2, 1], sizes)

    def test_the_first_candidate_wrecks_every_planned_line(self):
        lines = ["a", "b", "c"]
        by_line = {0: "A", 2: "C"}
        wrecked, count = next(prober.wreck_candidates(lines, by_line))
        self.assertEqual(["A", "b", "C"], wrecked)
        self.assertEqual(2, count)

    def test_it_terminates_on_a_single_line(self):
        # A generator that never stops halving would hang the whole run.
        got = list(prober.wreck_candidates(["a"], {0: "A"}))
        self.assertEqual(1, len(got))

    def test_an_empty_plan_yields_no_candidates(self):
        self.assertEqual([], list(prober.wreck_candidates(["a"], {})))

    def test_every_candidate_leaves_the_file_the_same_length(self):
        # Length drift is what disqualified drop mutants; the composed wrecks
        # must not reintroduce it.
        lines = list("abcdefgh")
        by_line = {i: lines[i].upper() for i in range(8)}
        for wrecked, _ in prober.wreck_candidates(lines, by_line):
            self.assertEqual(len(lines), len(wrecked))


class ClassifyTests(unittest.TestCase):
    """The OBSERVED/UNOBSERVED decision itself, which had no test at all.

    It was rewritten to stop counting a crash as coverage - the change that
    moved 9 functions out of OBSERVED and corrected a published number - and
    mutating `done.returncode == 0` still survived every test. The decision that
    produces the census had nothing pinning it.
    """

    class Harness:
        test, build_dir = "suite", "/build"

    def run_with(self, returncode=0, output="", timeout=False):
        import subprocess
        from unittest import mock

        def fake(*args, **kwargs):
            if timeout:
                raise subprocess.TimeoutExpired(cmd="ctest", timeout=1)
            return subprocess.CompletedProcess(
                args=[], returncode=returncode, stdout=output, stderr="")

        with mock.patch("subprocess.run", fake):
            return prober.classify(self.Harness(), 5)

    def test_a_passing_suite_means_nothing_observes_it(self):
        self.assertEqual(prober.UNOBSERVED, self.run_with(returncode=0))

    def test_an_assertion_failure_is_the_only_thing_that_counts_as_observed(self):
        self.assertEqual(prober.OBSERVED,
                         self.run_with(returncode=1, output="2 failure(s)\n"))

    def test_a_fault_is_not_an_observation(self):
        # The defect: a wrecked body that segfaults proves the code RUNS, and
        # was being counted as proof something watches it.
        self.assertEqual(
            prober.CRASHED,
            self.run_with(returncode=1,
                          output="err:seh:NtRaiseException code c0000005\n"))

    def test_a_hung_suite_is_not_an_observation_either(self):
        # And must not abort the census, which it did for all 224 functions.
        self.assertEqual(prober.CRASHED, self.run_with(timeout=True))


class VerdictVocabularyTests(unittest.TestCase):
    def test_no_compile_is_not_one_of_the_measured_verdicts(self):
        # Folding NO-COMPILE into either verdict would report an unmeasurable
        # function as a finding. Two of the first five tried did not compile.
        self.assertNotIn(prober.NO_COMPILE, (prober.OBSERVED, prober.UNOBSERVED))
        self.assertNotIn(prober.STALE, (prober.OBSERVED, prober.UNOBSERVED))


if __name__ == "__main__":
    unittest.main()
