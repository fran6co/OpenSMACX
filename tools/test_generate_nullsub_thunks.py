#!/usr/bin/env python3
"""Unit tests for the do-nothing-leaf generator.

The decoder here accepts exactly four bodies and must reject everything else,
because it is the only check standing between the catalogue and 56 unattended
emissions. Nothing downstream can tell a mis-accepted body apart: a function
that reads a field before returning looks identical from the outside to one
that does not.
"""
import unittest

import generate_nullsub_thunks as generator


class DecodeTest(unittest.TestCase):
    def test_reads_a_bare_return(self):
        self.assertEqual((0, False), generator.decode(b"\xc3", 0x400000))

    def test_reads_a_popping_return(self):
        self.assertEqual((8, False),
                         generator.decode(b"\xc2\x08\x00", 0x400000))

    def test_reads_a_zeroing_return(self):
        self.assertEqual((0, True), generator.decode(b"\x33\xc0\xc3", 0x400000))

    def test_reads_a_zeroing_popping_return(self):
        self.assertEqual((8, True),
                         generator.decode(b"\x33\xc0\xc2\x08\x00", 0x400000))

    def test_declines_a_body_that_reads_something(self):
        # mov eax,[ecx]; ret - one load away from a no-op, and emitting it as
        # one would silently drop the read.
        self.assertIsNone(generator.decode(b"\x8b\x01\xc3", 0x400000))

    def test_declines_a_body_that_writes_something(self):
        # mov byte ptr [ecx+4], 1; ret
        self.assertIsNone(
            generator.decode(b"\xc6\x41\x04\x01\xc3", 0x400000))

    def test_declines_a_constant_other_than_zero(self):
        # mov eax, 1; ret belongs to the constant-return scan, not here; this
        # generator only knows how to answer 0.
        self.assertIsNone(
            generator.decode(b"\xb8\x01\x00\x00\x00\xc3", 0x400000))

    def test_declines_a_branch(self):
        self.assertIsNone(generator.decode(b"\x85\xc9\x74\x01\xc3", 0x400000))

    def test_declines_empty_bytes(self):
        self.assertIsNone(generator.decode(b"", 0x400000))

    def test_declines_a_zeroing_with_no_return(self):
        self.assertIsNone(generator.decode(b"\x33\xc0", 0x400000))


class SymbolTest(unittest.TestCase):
    def test_names_an_unnamed_leaf_by_address(self):
        # IDA's nullsub_N and sub_N numbering is not stable across analyses,
        # so the address is what the emitted symbol is keyed on.
        self.assertEqual(
            "nullsub_0044c820",
            generator.symbol_for({"name": "nullsub_3", "address": "0x0044C820"}))

    def test_names_a_sub_by_address_too(self):
        self.assertEqual(
            "nullsub_00406b30",
            generator.symbol_for({"name": "sub_406b30",
                                  "address": "0x00406B30"}))

    def test_keeps_a_descriptive_name(self):
        self.assertEqual(
            "mapwin_nullsub1",
            generator.symbol_for({"name": "mapwin_nullsub1",
                                  "address": "0x00471160"}))


if __name__ == "__main__":
    unittest.main()
