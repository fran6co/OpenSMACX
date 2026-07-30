#!/usr/bin/env python3

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
    """Entries go in ascending position where one exists, and at the end of
    the jump table otherwise.

    Order is a tidiness convention here, not a correctness one: the array is
    only iterated, and the committed table is already out of order in two
    dozen places. What must hold is that an entry lands in the jump table and
    never in the call table below it, which holds interior call sites rather
    than function starts."""

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
    static_assert(sizeof(specs) / sizeof(specs[0]) == RedirectCount, "x");
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

    def test_appends_at_the_end_of_the_jump_table(self):
        # An address higher than everything present is ordinary - Cursor's is.
        # It must land at the end of the jump table, never in the call table
        # below, which holds interior call sites rather than function starts.
        text = self._insert(0x00700000, "omega_redirect")
        self.assertEqual([0x00401000, 0x00500000, 0x00600000, 0x00700000],
                         self._addresses(text))
        self.assertIn("0x00700000",
                      text[:text.index("const RedirectSpec call_specs[]")])

    def test_appending_leaves_the_call_table_alone(self):
        text = self._insert(0x00700000, "omega_redirect")
        below = text[text.index("const RedirectSpec call_specs[]"):]
        self.assertNotIn("omega_redirect", below)


class VisibilityTest(unittest.TestCase):
    """The four files agreeing is not enough - the spec table must be able to
    name the symbol. Wiring eighteen redirects to a new header once produced
    eighteen 'not declared in this scope' errors after the tool had reported
    success on every one."""

    def run_against(self, headers, dllmain_text):
        def make(name, text):
            path = mock.Mock()
            path.name = name
            path.read_text.return_value = text
            return path
        paths = [make(name, text) for name, text in headers.items()]
        with mock.patch.object(add_redirect, "SRC_DIR") as directory, \
             mock.patch.object(add_redirect, "DLLMAIN") as dllmain:
            directory.glob.return_value = paths
            dllmain.read_text.return_value = dllmain_text
            dllmain.name = "dllmain.cpp"
            return add_redirect.visible_to_dllmain("beta_redirect")

    def test_accepts_a_symbol_whose_header_is_included(self):
        visible, _ = self.run_against(
            {"ambience.h": "void __fastcall beta_redirect(A *self, void *);"},
            '#include "ambience.h"\n')
        self.assertTrue(visible)

    def test_rejects_a_header_dllmain_does_not_include(self):
        visible, detail = self.run_against(
            {"ambience.h": "void __fastcall beta_redirect(A *self, void *);"},
            '#include "buffer.h"\n')
        self.assertFalse(visible)
        self.assertIn("does not include", detail)

    def test_rejects_a_symbol_no_header_declares(self):
        visible, detail = self.run_against(
            {"buffer.h": "void other_redirect(void);"}, '#include "buffer.h"\n')
        self.assertFalse(visible)
        self.assertIn("no header", detail)


class VerifyTest(unittest.TestCase):
    HEADERS = {"buffer.h": "void __fastcall beta_redirect(A *self, void *);"}

    def verify(self, text, signature="OPENSMACX_SIGNATURE_00500000"):
        def make(name, body):
            path = mock.Mock()
            path.name = name
            path.read_text.return_value = body
            return path
        paths = [make(name, body) for name, body in self.HEADERS.items()]
        with mock.patch.object(add_redirect, "SIGNATURES") as signatures, \
             mock.patch.object(add_redirect, "SRC_DIR") as directory, \
             mock.patch.object(add_redirect, "DLLMAIN") as dllmain:
            signatures.read_text.return_value = signature
            directory.glob.return_value = paths
            dllmain.read_text.return_value = '#include "buffer.h"\n' + text
            dllmain.name = "dllmain.cpp"
            add_redirect.verify(0x00500000, "beta_redirect")

    def test_detects_a_count_that_disagrees_with_the_table(self):
        text = SpecInsertionTest.TABLE.replace(
            "RedirectCount = 3", "RedirectCount = 9")
        with self.assertRaises(ValueError) as caught:
            self.verify(text)
        self.assertIn("RedirectCount", str(caught.exception))

    def test_detects_a_missing_signature_macro(self):
        with self.assertRaises(ValueError) as caught:
            self.verify(SpecInsertionTest.TABLE, signature="no macros here")
        self.assertIn("missing after regeneration", str(caught.exception))

    def test_counts_only_the_jump_table(self):
        # Entries after the call-table marker belong to a different array and
        # must not be counted against RedirectCount.
        self.verify(SpecInsertionTest.TABLE + """\
        {
            0x00650000,
            reinterpret_cast<uintptr_t>(&call_redirect),
            OPENSMACX_SIGNATURE_00650000,
        },
""")                                          # must not raise


if __name__ == "__main__":
    unittest.main()
