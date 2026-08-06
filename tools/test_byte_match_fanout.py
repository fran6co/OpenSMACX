#!/usr/bin/env python3
"""Unit tests for the byte-match ledger and its ratchet.

The ratchet is this tree's regression alarm: it is what noticed that restoring
a deleted method cost 115 matched bytes, and what confirmed that growing 19
classes by up to 0x4030C bytes cost nothing. Until this file existed, nothing
tested the alarm itself - and an alarm that cannot fail is worse than none,
because the number it prints is trusted.

`--prepare` and `--collect` drive VC6 under wine and are exercised by running
the tool. What is tested here is the accounting: which tiers count, which
direction is refused, and whether a missing ledger reads as zero progress.
"""

import io
import unittest
from contextlib import redirect_stdout

import byte_match_fanout as fanout


def ledger(*rows):
    """{address: row} in the shape read_ledger returns."""
    return {row["address"]: row for row in rows}


def row(address, tier, size):
    return {"address": address, "tier": tier, "size": str(size)}


class SummariseTest(unittest.TestCase):
    def test_only_byte_exact_counts(self):
        """SHAPE_EXACT means the instruction shape agrees while a non-relocated
        operand differs - the comparator's own controls use a wrong field
        offset and a wrong vtable slot to make that tier fire. Counting it
        would turn the first localised wrong constant into recovery progress,
        which is the exact failure this campaign's layouts could produce."""
        found = fanout.summarise(ledger(
            row("0x1", "BYTE_EXACT", 100),
            row("0x2", "SHAPE_EXACT", 200),
            row("0x3", "MNEMONIC_ONLY", 400),
            row("0x4", "MISMATCH", 800),
            row("0x5", "NO_COMPILE", 1600)))
        self.assertEqual(found, (1, 100))

    def test_bytes_are_summed_not_counted(self):
        found = fanout.summarise(ledger(
            row("0x1", "BYTE_EXACT", 10),
            row("0x2", "BYTE_EXACT", 32)))
        self.assertEqual(found, (2, 42))

    def test_an_empty_ledger_is_zero(self):
        self.assertEqual(fanout.summarise({}), (0, 0))

    def test_a_missing_size_does_not_raise(self):
        found = fanout.summarise(ledger({"address": "0x1",
                                         "tier": "BYTE_EXACT", "size": ""}))
        self.assertEqual(found, (1, 0))


class RatchetTest(unittest.TestCase):
    """Both figures may rise and neither may fall."""

    def setUp(self):
        self.read = fanout.read_ledger
        self.functions = fanout.BASELINE_MATCHED_FUNCTIONS
        self.bytes = fanout.BASELINE_MATCHED_BYTES
        fanout.BASELINE_MATCHED_FUNCTIONS = 10
        fanout.BASELINE_MATCHED_BYTES = 1000

    def tearDown(self):
        fanout.read_ledger = self.read
        fanout.BASELINE_MATCHED_FUNCTIONS = self.functions
        fanout.BASELINE_MATCHED_BYTES = self.bytes

    def at(self, functions, size):
        """A ledger summarising to exactly (functions, size)."""
        rows = [row(f"0x{i}", "BYTE_EXACT", 0) for i in range(functions - 1)]
        rows.append(row(f"0x{functions}", "BYTE_EXACT", size))
        fanout.read_ledger = lambda: ledger(*rows)
        out = io.StringIO()
        with redirect_stdout(out):
            status = fanout.check()
        return status, out.getvalue()

    def test_sitting_exactly_on_the_floor_passes(self):
        status, output = self.at(10, 1000)
        self.assertEqual(status, 0)
        self.assertNotIn("FAIL", output)

    def test_losing_bytes_fails_even_with_more_functions(self):
        """The byte figure is the one that means anything: the exact-match
        population is ordered smallest-first, so the function count is
        dominated by trivial stubs and can rise while real bytes are lost.
        That is exactly what a deleted method looked like."""
        status, output = self.at(20, 900)
        self.assertEqual(status, 1)
        self.assertIn("matched bytes fell", output)

    def test_losing_functions_fails(self):
        status, output = self.at(9, 1000)
        self.assertEqual(status, 1)
        self.assertIn("matched functions fell", output)

    def test_rising_passes_and_says_the_floor_is_stale(self):
        status, output = self.at(11, 1100)
        self.assertEqual(status, 0)
        self.assertIn("11 / 1100", output)

    def test_an_absent_ledger_is_not_a_failure(self):
        fanout.read_ledger = lambda: {}
        out = io.StringIO()
        with redirect_stdout(out):
            status = fanout.check()
        self.assertEqual(status, 0)
        self.assertIn("absent", out.getvalue())


class BaselineTest(unittest.TestCase):
    def test_the_committed_floor_is_not_above_the_committed_ledger(self):
        """The floor and the ledger are committed together, so a floor raised
        without its ledger - or a ledger regenerated without raising the floor
        - is a broken commit rather than a broken tree."""
        found = fanout.summarise(fanout.read_ledger())
        if found == (0, 0):
            self.skipTest("no committed ledger")
        self.assertGreaterEqual(found[0], fanout.BASELINE_MATCHED_FUNCTIONS)
        self.assertGreaterEqual(found[1], fanout.BASELINE_MATCHED_BYTES)


if __name__ == "__main__":
    unittest.main()
