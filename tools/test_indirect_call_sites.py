#!/usr/bin/env python3

from pathlib import Path
import unittest

import indirect_call_sites as sites


def decoder():
    import capstone
    return capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


def image(payload, base=0x400000):
    def body(start, end):
        if start < base or start >= base + len(payload):
            return b""
        return payload[start - base:end - base]
    return body


class DetectionTests(unittest.TestCase):
    def scan(self, payload):
        start = 0x400000
        return sites.indirect_sites(image(payload), decoder(),
                                    start, start + len(payload))

    def test_direct_call_is_not_a_site(self):
        # E8 rel32 - the only kind the callgraph records.
        self.assertEqual(self.scan(b"\xE8\x10\x00\x00\x00\xC3"), [])

    def test_call_through_a_register(self):
        found = self.scan(b"\xFF\xD0\xC3")          # call eax
        self.assertEqual(len(found), 1)
        self.assertIn("eax", found[0][1])

    def test_call_through_a_vtable_slot(self):
        found = self.scan(b"\xFF\x52\x64\xC3")      # call [edx+0x64]
        self.assertEqual(len(found), 1)
        self.assertIn("0x64", found[0][1])

    def test_call_through_an_absolute_pointer(self):
        # FF 15 - the import-table shape, `call dword ptr [0x669338]`.
        found = self.scan(b"\xFF\x15\x38\x93\x66\x00\xC3")
        self.assertEqual(len(found), 1)
        self.assertIn("0x669338", found[0][1])

    def test_a_jump_is_not_a_call(self):
        self.assertEqual(self.scan(b"\xFF\xE0\xC3"), [])   # jmp eax

    def test_mixed_body_reports_only_the_indirect_ones(self):
        payload = (b"\xE8\x10\x00\x00\x00"    # direct
                   b"\xFF\xD0"                # call eax
                   b"\xE8\x20\x00\x00\x00"    # direct
                   b"\xFF\x52\x64"            # call [edx+0x64]
                   b"\xC3")
        self.assertEqual(len(self.scan(payload)), 2)

    def test_unmapped_range_yields_nothing_rather_than_raising(self):
        self.assertEqual(
            sites.indirect_sites(image(b"\xFF\xD0"), decoder(),
                                 0x900000, 0x900002),
            [])


class CommittedImageTests(unittest.TestCase):
    """Against the real executable, if it is present."""

    def setUp(self):
        if not sites.DEFAULT_EXE.is_file():
            self.skipTest("the original executable is absent")
        if not sites.DEFAULT_FUNCTIONS.is_file():
            self.skipTest("docs/recovery/functions.csv is absent")
        self.rows = sites.read_rows(sites.DEFAULT_FUNCTIONS)
        self.body, self.decoder = sites.load_image(sites.DEFAULT_EXE)

    def scan(self, address):
        row = self.rows[address]
        return sites.indirect_sites(self.body, self.decoder,
                                    int(row["address"], 16),
                                    int(row["end_address"], 16))

    def test_hline_dispatches_indirectly_despite_zero_callees(self):
        # The whole reason this tool exists: the catalogue calls it a leaf.
        row = self.rows["0x005E10C0"]
        self.assertEqual(row.get("call_target_count"), "0")
        self.assertGreaterEqual(len(self.scan("0x005E10C0")), 1)

    def test_fill_dispatches_indirectly_despite_zero_callees(self):
        self.assertGreaterEqual(len(self.scan("0x005DFCD0")), 1)

    def test_a_genuinely_direct_function_reports_none(self):
        # scan_prototypes was recovered as a real leaf; if this starts
        # reporting sites, the decoder is finding calls that are not there.
        self.assertEqual(self.scan("0x0053A4A0"), [])


if __name__ == "__main__":
    unittest.main()
