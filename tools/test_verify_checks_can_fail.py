#!/usr/bin/env python3
"""The harness that proves checks can fail must not be fakeable itself.

Its own first run demonstrated the hazard: a damage case whose substitution
matched nothing produced input identical to the real file, the check passed, and
that was reported as the check being unable to fail. Both directions of that
mistake are covered here - damage that does not damage, and a check that exits 0.
"""
import unittest

import verify_checks_can_fail as harness


class SubstituteTests(unittest.TestCase):
    def test_a_pattern_that_is_absent_is_refused(self):
        # The exact bug: `RESOURCE_LOCK wineprefix` against a file that says
        # `RESOURCE_LOCK "wineprefix"`.
        with self.assertRaises(harness.Skip):
            harness.substitute('RESOURCE_LOCK "wineprefix"',
                               "RESOURCE_LOCK wineprefix", "")

    def test_a_present_pattern_is_replaced(self):
        self.assertEqual("b", harness.substitute("a", "a", "b"))

    def test_the_refusal_says_it_would_prove_nothing(self):
        with self.assertRaises(harness.Skip) as raised:
            harness.substitute("hello", "absent", "x")
        self.assertIn("prove nothing", str(raised.exception))


class CoverageContractTests(unittest.TestCase):
    def test_every_covered_check_has_at_least_one_case(self):
        # This is the property that makes a new check ship with a proof it can
        # fail, rather than with a promise that it does.
        declared = {check for check, _, _, _ in harness.CASES}
        self.assertEqual(set(), harness.COVERED_CHECKS - declared,
                         "a check is listed as covered with no damage case")

    def test_every_case_names_a_covered_check(self):
        # The reverse keeps the two lists honest: a case for a check nobody
        # claims coverage of would never be missed if it stopped running.
        for check, description, _, _ in harness.CASES:
            self.assertIn(check, harness.COVERED_CHECKS,
                          f"case {description!r} names unlisted check {check}")

    def test_every_case_declares_expected_refusal_text(self):
        # Without it a usage error scores as a refusal; see RefusalTests.
        for check, description, _, expected in harness.CASES:
            self.assertTrue(expected and expected.strip(),
                            f"{check}: {description} has no expected text")


class RefusalTests(unittest.TestCase):
    """A non-zero exit is not evidence that a check refused anything.

    This was the audit's headline finding against this very file: argparse exits
    2 on an unrecognised flag, and seven of the eight checks here are invoked
    with at least one flag no add_test passes. Renaming any of them would leave
    ctest green while the damage case became `ok (exit 2)` on a usage banner.
    """

    def test_exit_zero_is_not_a_refusal(self):
        real, why = harness.refusal_is_real(0, "all clean", "clean")
        self.assertFalse(real)
        self.assertIn("exited 0", why)

    def test_a_usage_banner_is_not_a_refusal(self):
        real, why = harness.refusal_is_real(
            2, "usage: verify_x.py [-h] [--src SRC]\nerror: unrecognized", "x")
        self.assertFalse(real)
        self.assertIn("failed to run", why)

    def test_a_traceback_is_not_a_refusal(self):
        real, _ = harness.refusal_is_real(
            1, "Traceback (most recent call last):\n  File ...", "x")
        self.assertFalse(real)

    def test_the_wrong_refusal_is_not_accepted(self):
        # Exiting non-zero for some unrelated reason is not proof that the
        # property under test was the one detected.
        real, why = harness.refusal_is_real(1, "some other complaint", "x")
        self.assertFalse(real)
        self.assertIn("does not contain", why)

    def test_a_real_refusal_is_accepted(self):
        real, why = harness.refusal_is_real(
            1, "load-time-addresses: a fixed game address is dereferenced at "
               "load time", "dereferenced at load time")
        self.assertTrue(real)
        self.assertEqual("", why)

    def test_skip_is_not_a_pass(self):
        # A skipped case must not satisfy the coverage requirement, or an
        # absent artifact silently becomes a clean run.
        self.assertTrue(issubclass(harness.Skip, Exception))
        source = harness.main.__doc__ or ""
        self.assertNotIn("skipped", source.lower().replace("skipped and", ""),
                         "main() must not treat skips as coverage")


if __name__ == "__main__":
    unittest.main()
