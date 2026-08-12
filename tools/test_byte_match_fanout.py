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


# RatchetTest and BaselineTest are gone with the thing they tested. The floor
# was two constants in this file compared against docs/recovery/byte-match.csv;
# it is now the `BYTE_EXACT` claims in `src/`, checked by
# `tools/decomp_status.py --check` and covered by `tools/test_decomp_status.py`
# plus a damage case in `verify_checks_can_fail.py` that the constant form
# never had.


if __name__ == "__main__":
    unittest.main()
