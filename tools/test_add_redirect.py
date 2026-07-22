#!/usr/bin/env python3

import re
import unittest
from unittest import mock

import add_redirect


class ParseAddressTest(unittest.TestCase):
    def test_accepts_a_code_address(self):
        self.assertEqual(0x005EE130, add_redirect.parse_address("0x005EE130"))

    def test_rejects_an_address_outside_the_image(self):
        # A data address wired as a redirect would patch a jump over
        # something that is not a function prologue.
        with self.assertRaises(ValueError):
            add_redirect.parse_address("0x009B7B28")
        with self.assertRaises(ValueError):
            add_redirect.parse_address("0x00100000")


class SpecInsertionTest(unittest.TestCase):
    """The table is kept sorted by address. A misplaced entry compiles and
    runs, so nothing downstream catches it - which is why insertion position
    is computed rather than appended."""

    TABLE = """\
constexpr size_t RedirectCount = 3;
    const RedirectSpec specs[] = {
        {
            0x00401000,
            reinterpret_cast<uintptr_t>(&alpha_redirect),
            OPENSMACX_SIGNATURE_00401000,
        },
        {
            0x00500000,
            reinterpret_cast<uintptr_t>(&beta_redirect),
            OPENSMACX_SIGNATURE_00500000,
        },
        {
            0x00600000,
            reinterpret_cast<uintptr_t>(&gamma_redirect),
            OPENSMACX_SIGNATURE_00600000,
        },
    };
const RedirectSpec call_specs[] = {
"""

    def _insert(self, address, symbol):
        written = {}
        with mock.patch.object(add_redirect, "DLLMAIN") as dllmain:
            dllmain.read_text.return_value = self.TABLE
            dllmain.name = "dllmain.cpp"
            dllmain.write_text.side_effect = lambda text: written.setdefault("text", text)
            add_redirect.add_spec(address, symbol)
        return written["text"]

    def _addresses(self, text):
        cutoff = text.index("const RedirectSpec call_specs[]")
        return [int(m.group(1), 16)
                for m in add_redirect.SPEC_RE.finditer(text[:cutoff])]

    def test_inserts_in_sorted_position(self):
        text = self._insert(0x00550000, "delta_redirect")
        self.assertEqual([0x00401000, 0x00500000, 0x00550000, 0x00600000],
                         self._addresses(text))

    def test_inserts_before_everything_when_lowest(self):
        text = self._insert(0x00400500, "delta_redirect")
        self.assertEqual(0x00400500, self._addresses(text)[0])

    def test_increments_the_count(self):
        text = self._insert(0x00550000, "delta_redirect")
        self.assertEqual("4", add_redirect.COUNT_RE.search(text).group(1))

    def test_rejects_an_address_already_present(self):
        with self.assertRaises(ValueError):
            self._insert(0x00500000, "beta_redirect")

    def test_refuses_to_append_past_the_end(self):
        # Sorting after every entry means the address belongs after the jump
        # table, which is where the call table starts - appending blindly
        # would put it in the wrong table.
        with self.assertRaises(ValueError):
            self._insert(0x00700000, "omega_redirect")


class VerifyTest(unittest.TestCase):
    def test_detects_a_count_that_disagrees_with_the_table(self):
        text = SpecInsertionTest.TABLE.replace(
            "RedirectCount = 3", "RedirectCount = 9")
        with mock.patch.object(add_redirect, "SIGNATURES") as signatures, \
             mock.patch.object(add_redirect, "DLLMAIN") as dllmain:
            signatures.read_text.return_value = "OPENSMACX_SIGNATURE_00500000"
            dllmain.read_text.return_value = text
            with self.assertRaises(ValueError) as caught:
                add_redirect.verify(0x00500000)
        self.assertIn("RedirectCount", str(caught.exception))

    def test_detects_a_missing_signature_macro(self):
        with mock.patch.object(add_redirect, "SIGNATURES") as signatures:
            signatures.read_text.return_value = "no macros here"
            with self.assertRaises(ValueError) as caught:
                add_redirect.verify(0x00500000)
        self.assertIn("missing after regeneration", str(caught.exception))

    def test_counts_only_the_jump_table(self):
        # Entries after the call-table marker belong to a different array and
        # must not be counted against RedirectCount.
        text = SpecInsertionTest.TABLE + """\
        {
            0x00650000,
            reinterpret_cast<uintptr_t>(&call_redirect),
            OPENSMACX_SIGNATURE_00650000,
        },
"""
        with mock.patch.object(add_redirect, "SIGNATURES") as signatures, \
             mock.patch.object(add_redirect, "DLLMAIN") as dllmain:
            signatures.read_text.return_value = "OPENSMACX_SIGNATURE_00500000"
            dllmain.read_text.return_value = text
            add_redirect.verify(0x00500000)   # must not raise


if __name__ == "__main__":
    unittest.main()
