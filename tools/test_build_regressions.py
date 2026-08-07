#!/usr/bin/env python3
"""Unit tests for the build-regression debt ledger.

The point of this file is that giving up a proof to make the tree compile is
ALLOWED but never invisible. So the tests are about what the validator refuses:
a row that records no real loss, or records one without saying what could not
be reconciled, would let the debt vanish into a number.
"""

import unittest

import build_regressions as tool


def row(**overrides):
    base = {"address": "0x00401000", "name": "?f@C@@QAEXXZ",
            "was_tier": "BYTE_EXACT", "now_tier": "MISMATCH",
            "reason": "the body reaches a field the header declares under a "
                      "different name",
            "recorded_by": "vc6-build"}
    base.update(overrides)
    return base


class ValidationTest(unittest.TestCase):
    def test_a_well_formed_row_passes(self):
        tool.validate([row()])

    def test_a_row_that_lost_nothing_is_refused(self):
        # If it was never a proof, giving it up costs nothing and the ratchet
        # should not forgive anything for it.
        with self.assertRaises(tool.Invalid):
            tool.validate([row(was_tier="MISMATCH")])

    def test_a_row_still_holding_a_proof_is_refused(self):
        # Nothing was lost, so nothing is owed - and forgiving it would let
        # the floor drift down for free.
        with self.assertRaises(tool.Invalid):
            tool.validate([row(now_tier="BYTE_EXACT")])

    def test_a_missing_reason_is_refused(self):
        with self.assertRaises(tool.Invalid):
            tool.validate([row(reason="")])

    def test_it_did_not_compile_is_not_a_reason(self):
        # The whole file is about things that did not compile. What has to be
        # recorded is what specifically could not be reconciled, so the next
        # person starts where this one stopped.
        with self.assertRaises(tool.Invalid):
            tool.validate([row(reason="did not compile")])

    def test_a_decimal_address_is_refused(self):
        with self.assertRaises(tool.Invalid):
            tool.validate([row(address="4198400")])

    def test_an_unexpected_column_is_refused(self):
        bad = row()
        bad["excuse"] = "busy"
        with self.assertRaises(tool.Invalid):
            tool.validate([bad])


class OwedTest(unittest.TestCase):
    def test_an_empty_ledger_owes_nothing(self):
        original = tool.load
        tool.load = lambda: []
        try:
            self.assertEqual(tool.owed()[0], 0)
        finally:
            tool.load = original

    def test_the_committed_ledger_is_valid(self):
        tool.validate(tool.load())


if __name__ == "__main__":
    unittest.main()
