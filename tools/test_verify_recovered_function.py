#!/usr/bin/env python3
"""Unit tests for the single-function verifier.

The compile needs VC6 and is exercised by running the tool. What is tested here
is the contract an agent loop depends on: the exit code, and that a function
with nothing recovered says so instead of reporting a verdict about nothing.
"""

import io
import unittest
from contextlib import redirect_stdout

import verify_recovered_function as verifier


class ExitCodeTest(unittest.TestCase):
    """Exit 0 means BYTE_EXACT and nothing else - the same contract
    `byte_match.py` uses, so this can be a loop condition."""

    def setUp(self):
        self.verify = verifier.writeback.verify
        self.body = verifier.committed_body
        verifier.committed_body = lambda address: ("body", "src/x.cpp:1")

    def tearDown(self):
        verifier.writeback.verify = self.verify
        verifier.committed_body = self.body

    def run_main(self, tier, extra=None):
        verifier.writeback.verify = lambda a, b: {"tier": tier, **(extra or {})}
        out = io.StringIO()
        with redirect_stdout(out):
            status = verifier.main(["0x00401000"])
        return status, out.getvalue()

    def test_byte_exact_exits_zero(self):
        self.assertEqual(self.run_main("BYTE_EXACT")[0], 0)

    def test_every_other_tier_exits_nonzero(self):
        for tier in ("MISMATCH", "MNEMONIC_ONLY", "NO_COMPILE", "REFUSED",
                     "SHAPE_EXACT", "SHARED_TAIL"):
            self.assertEqual(self.run_main(tier)[0], 1, tier)

    def test_the_divergence_is_printed(self):
        _, output = self.run_main(
            "MISMATCH", {"note": "#0: original 'xor' vs rebuilt 'ret 8'"})
        self.assertIn("original 'xor'", output)

    def test_a_refusal_reason_is_printed(self):
        _, output = self.run_main(
            "NO_COMPILE", {"refusal_reason": "error C2065: 'Win'"})
        self.assertIn("C2065", output)

    def test_empty_verdict_fields_are_not_printed(self):
        _, output = self.run_main("BYTE_EXACT", {"note": ""})
        self.assertNotIn("divergence", output)


class NothingRecoveredTest(unittest.TestCase):
    """A function with no committed body must not be scored.

    Reporting a verdict for one would be a statement about a body that does not
    exist; `byte_match_fanout.py --prepare` is the tool for those, and saying so
    is more use than a tier.
    """

    def setUp(self):
        self.body = verifier.committed_body

    def tearDown(self):
        verifier.committed_body = self.body

    def test_it_skips_rather_than_scoring(self):
        verifier.committed_body = lambda address: (
            None, "0x00401000 has no source_locations")
        out = io.StringIO()
        with redirect_stdout(out):
            status = verifier.main(["0x00401000"])
        self.assertEqual(status, 0)
        self.assertIn("SKIP", out.getvalue())


class AddressParsingTest(unittest.TestCase):
    def test_a_non_hex_address_is_a_usage_error(self):
        out = io.StringIO()
        with redirect_stdout(out):
            self.assertEqual(verifier.main(["not-an-address"]), 2)


if __name__ == "__main__":
    unittest.main()
