#!/usr/bin/env python3
"""Pin the parts of the byte-match experiment that run without the compiler.

CI has no Visual C++ 6.0 and no Wine, so nothing here compiles anything. What
it pins is everything that could quietly turn a mismatch into a match: the
COFF reader that decides which bytes are "the function", the normalisation
that decides which differences are allowed to vanish, and the skip path that
must announce itself rather than pass silently.

The comparator's own honesty is checked here in both directions - it must say
match for identical input and mismatch for a one-mnemonic edit - because the
live positive control in the tool can only run when something matched.
"""

from __future__ import annotations

import os
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import msvc6_byte_match as tool  # noqa: E402


def build_coff(functions, extra_symbols=()):
    """A minimal i386 COFF object: [(section name, code, symbol name)]."""
    header_size = 20 + 40 * len(functions)
    raw = b""
    sections = []
    for name, code, _ in functions:
        sections.append((name, len(code), header_size + len(raw)))
        raw += code

    symbols = b""
    count = 0
    for index, (_, _, symbol) in enumerate(functions):
        symbols += (symbol.encode().ljust(8, b"\0")[:8]
                    + struct.pack("<IhHBB", 0, index + 1, 0x20, 2, 0))
        count += 1
    for symbol, section, storage in extra_symbols:
        symbols += (symbol.encode().ljust(8, b"\0")[:8]
                    + struct.pack("<IhHBB", 0, section, 0x20, storage, 0))
        count += 1

    body = header_size + len(raw)
    head = struct.pack("<HHIIIHH", 0x14C, len(functions), 0, body, count, 0, 0)
    table = b""
    for name, size, pointer in sections:
        table += (name.encode().ljust(8, b"\0")[:8]
                  + struct.pack("<IIII", 0, 0, size, pointer)
                  + struct.pack("<IIHHI", 0, 0, 0, 0, 0x60501020))
    return head + table + raw + symbols + struct.pack("<I", 4)


class CoffReaderTests(unittest.TestCase):
    def test_it_returns_the_single_external_text_symbol(self):
        obj = build_coff([(".text", b"\x33\xc0\xc3", "?f@@YAHXZ")])
        self.assertEqual(tool.coff_function(obj), b"\x33\xc0\xc3")

    def test_it_strips_int3_and_nop_alignment_padding(self):
        # A COMDAT is padded to alignment. Counting the padding as code would
        # make every rebuilt body longer than its original by an amount that
        # depends only on where the function happened to end.
        obj = build_coff([(".text", b"\x33\xc0\xc3\xcc\xcc\xcc", "?f@@YAHXZ")])
        self.assertEqual(tool.coff_function(obj), b"\x33\xc0\xc3")
        obj = build_coff([(".text", b"\x33\xc0\xc3\x90\x90", "?f@@YAHXZ")])
        self.assertEqual(tool.coff_function(obj), b"\x33\xc0\xc3")

    def test_it_refuses_an_object_holding_two_functions(self):
        # One function per translation unit is the whole reason the reader can
        # be this simple. If a unit ever grows a second definition the tool has
        # to stop, not pick one and compare the wrong body.
        obj = build_coff([(".text", b"\xc3", "?f@@YAHXZ"),
                          (".text", b"\xc3", "?g@@YAHXZ")])
        with self.assertRaises(ValueError):
            tool.coff_function(obj)

    def test_it_ignores_static_and_non_text_symbols(self):
        obj = build_coff([(".text", b"\x33\xc0\xc3", "?f@@YAHXZ")],
                         extra_symbols=[(".text", 1, 3), ("@comp.id", -1, 3)])
        self.assertEqual(tool.coff_function(obj), b"\x33\xc0\xc3")

    def test_it_refuses_a_non_i386_object(self):
        obj = bytearray(build_coff([(".text", b"\xc3", "?f@@YAHXZ")]))
        obj[0:2] = struct.pack("<H", 0x8664)
        with self.assertRaises(ValueError):
            tool.coff_function(bytes(obj))


class NormalisationTests(unittest.TestCase):
    def test_operands_are_discarded_so_register_choice_cannot_mismatch(self):
        # mov eax, 1 and mov ebx, 2 must reduce to the same thing: a fresh
        # compile has no reason to pick the original's registers, and its
        # immediates cannot carry the original's addresses.
        self.assertEqual(tool.mnemonics(b"\xb8\x01\x00\x00\x00"),
                         tool.mnemonics(b"\xbb\x02\x00\x00\x00"))

    def test_nothing_else_is_merged(self):
        # lea is not mov, add is not sub, je is not jne. Each of these is a
        # difference an over-eager normaliser would be tempted to erase, and
        # erasing any of them would let a wrong body pass.
        self.assertNotEqual(tool.mnemonics(b"\x8d\x04\x80"),   # lea eax,[eax+eax*4]
                            tool.mnemonics(b"\x8b\xc1"))       # mov eax, ecx
        self.assertNotEqual(tool.mnemonics(b"\x03\xc1"),       # add eax, ecx
                            tool.mnemonics(b"\x2b\xc1"))       # sub eax, ecx
        self.assertNotEqual(tool.mnemonics(b"\x74\x0a"),       # je
                            tool.mnemonics(b"\x75\x0a"))       # jne
        self.assertNotEqual(tool.mnemonics(b"\x40"),           # inc eax
                            tool.mnemonics(b"\x83\xc0\x01"))   # add eax, 1

    def test_the_ret_pop_count_is_a_known_blind_spot(self):
        # ret 8 and ret reduce alike, so arity does not show up in the verdict.
        # It is pinned rather than fixed: the byte lengths are reported next to
        # every result, and this is what makes that reporting load-bearing.
        self.assertEqual(tool.mnemonics(b"\xc2\x08\x00"), tool.mnemonics(b"\xc3"))


class ComparatorTests(unittest.TestCase):
    def test_identical_sequences_match(self):
        self.assertEqual(tool.compare(["push", "ret"], ["push", "ret"]),
                         (True, ""))

    def test_a_one_mnemonic_edit_is_reported_with_its_position(self):
        matched, note = tool.compare(["push", "add", "ret"],
                                     ["push", "sub", "ret"])
        self.assertFalse(matched)
        self.assertIn("#1", note)
        self.assertIn("add", note)
        self.assertIn("sub", note)

    def test_a_prefix_is_not_a_match(self):
        matched, note = tool.compare(["push", "mov", "ret"], ["push", "mov"])
        self.assertFalse(matched)
        self.assertIn("<end>", note)

    def test_a_leading_frame_setup_diverges_at_zero(self):
        # This is the shape the pre-registered /O2 run produced five times
        # over: the original pushes ebp and an /Oy build does not.
        matched, note = tool.compare(["push", "mov", "ret"], ["mov", "ret"])
        self.assertFalse(matched)
        self.assertTrue(note.startswith("#0"))


class SpanTests(unittest.TestCase):
    def test_size_is_the_sum_of_spans_not_end_minus_address(self):
        row = {"address": "0x00401D80", "size": "1568",
               "body_ranges": "0x00401D80-0x0040230F;0x006504E0-0x00650571"}
        spans = tool.body_spans(row)
        self.assertEqual(spans, [(0x00401D80, 0x0040230F),
                                 (0x006504E0, 0x00650571)])
        self.assertEqual(sum(high - low for low, high in spans), 1568)

    def test_a_missing_range_falls_back_to_address_plus_size(self):
        row = {"address": "0x00401000", "size": "16", "body_ranges": ""}
        self.assertEqual(tool.body_spans(row), [(0x00401000, 0x00401010)])


class BodyExtractionTests(unittest.TestCase):
    def test_it_reopens_a_body_that_starts_inside_its_doc_comment(self):
        # functions.csv points at the "Original Offset" line, which is inside
        # the comment. The extract therefore opens with an unmatched `*/`, and
        # feeding that to a compiler is a syntax error, not a mismatch.
        with tempfile.TemporaryDirectory() as work:
            path = Path(work) / "sample.cpp"
            path.write_text("/*\nOriginal Offset: 00401000\nStatus: Complete\n"
                            "*/\nint f(int a) {\n    return a + 1;\n}\n"
                            "int g() { return 0; }\n")
            saved = tool.REPO_ROOT
            tool.REPO_ROOT = Path(work)
            try:
                body = tool.extract_body("sample.cpp:2")
            finally:
                tool.REPO_ROOT = saved
            self.assertTrue(body.startswith("/*\nOriginal Offset"))
            self.assertIn("return a + 1;", body)
            self.assertNotIn("int g()", body)

    def test_it_stops_at_the_matching_brace(self):
        with tempfile.TemporaryDirectory() as work:
            path = Path(work) / "sample.cpp"
            path.write_text("int f(int a) {\n    if (a) {\n        return 1;\n"
                            "    }\n    return 0;\n}\nint g() { return 2; }\n")
            saved = tool.REPO_ROOT
            tool.REPO_ROOT = Path(work)
            try:
                body = tool.extract_body("sample.cpp:1")
            finally:
                tool.REPO_ROOT = saved
            self.assertIn("return 0;", body)
            self.assertNotIn("int g()", body)


class CatalogueTests(unittest.TestCase):
    """The five cases must still be what the pre-registration allowed."""

    def setUp(self):
        if not tool.FUNCTIONS_CSV.is_file():
            self.skipTest("docs/recovery/functions.csv is absent")
        self.rows = tool.load_rows()

    def test_every_case_is_a_source_complete_leaf(self):
        for case in tool.CASES:
            row = self.rows[case.address]
            self.assertEqual(row["recovery_state"], "source_complete",
                             f"0x{case.address:08X}")
            self.assertEqual(row["call_target_count"], "0",
                             f"0x{case.address:08X} is not a leaf")

    def test_every_case_still_points_at_its_recovered_body(self):
        for case in tool.CASES:
            self.assertEqual(self.rows[case.address]["source_locations"],
                             case.source, f"0x{case.address:08X} moved")

    def test_every_case_carries_a_perturbation_that_is_present(self):
        # A positive control that silently matches nothing is the failure mode
        # this whole experiment exists to avoid.
        for case in tool.CASES:
            old, _ = case.perturb
            self.assertTrue(old, f"0x{case.address:08X} has no perturbation")
            self.assertIn(old, tool.extract_body(case.source),
                          f"0x{case.address:08X} perturbation is stale")

    def test_the_pre_registered_flags_are_recorded_verbatim(self):
        self.assertEqual(tool.PREREGISTERED_FLAGS, "/c /O2 /Gy /GR-")
        self.assertEqual(tool.MEASURED_FLAGS, "/c /O2 /Gy /GR- /Oy-")


class SkipTests(unittest.TestCase):
    def test_it_names_what_is_missing_rather_than_passing_quietly(self):
        saved = tool.VC6_CL
        tool.VC6_CL = Path(os.devnull) / "CL.EXE"
        try:
            self.assertIn("CL.EXE", tool.available())
        finally:
            tool.VC6_CL = saved


if __name__ == "__main__":
    unittest.main()
