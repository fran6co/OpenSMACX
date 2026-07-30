#!/usr/bin/env python3
"""Tests for resolving a canonical address to the bytes our compiler produced.

This tool feeds a prover, so its failure mode is not "finds nothing" but "finds
the wrong body and looks certain about it". Three things here are load-bearing
for that reason: a name that resolves two ways is refused rather than picked, a
symbol's bytes are read from ITS section rather than from `.text`, and the
i686 calling-convention decoration is undone in the one direction that works.
"""

from __future__ import annotations

import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import extract_compiled_recovery as extract  # noqa: E402


class DecorationTests(unittest.TestCase):
    def test_a_fastcall_at_becomes_an_underscore(self):
        # The `@` REPLACES the leading underscore rather than joining it, and
        # this cross-c++filt strips a target underscore itself - so removing
        # the `@` without putting the underscore back yields a name it
        # refuses. That mistake left 1,136 of 2,048 symbols unresolved.
        self.assertEqual("__ZN1A1bEv", extract.undecorate("@_ZN1A1bEv@8"))

    def test_a_stdcall_suffix_comes_off(self):
        self.assertEqual("__ZN1A1bEv", extract.undecorate("__ZN1A1bEv@12"))

    def test_an_undecorated_name_is_unchanged(self):
        self.assertEqual("__ZN1A1bEv", extract.undecorate("__ZN1A1bEv"))

    def test_a_plain_c_symbol_survives(self):
        self.assertEqual("_construct_basewin",
                         extract.undecorate("_construct_basewin"))


class QualifiedNameTests(unittest.TestCase):
    def test_the_parameter_list_comes_off(self):
        self.assertEqual("construct_basewin",
                         extract.qualified_name("construct_basewin()"))

    def test_a_scope_is_kept(self):
        self.assertEqual("AlphaMenu::requested_height",
                         extract.qualified_name("AlphaMenu::requested_height()"))

    def test_a_name_containing_parentheses_keeps_them(self):
        # `A::operator()(int)` is the discriminating case: cutting at the FIRST
        # parenthesis yields `A::operator`, which is not a symbol. An earlier
        # version of this test used `f(void (*)(int, int), char*)`, where the
        # naive split happens to give the right answer too - so it could not
        # tell a correct implementation from a broken one.
        self.assertEqual("A::operator()",
                         extract.qualified_name("A::operator()(int)"))

    def test_a_function_pointer_parameter_does_not_confuse_it(self):
        self.assertEqual(
            "f", extract.qualified_name("f(void (*)(int, int), char*)"))

    def test_a_name_with_no_parameters_is_returned_whole(self):
        self.assertEqual("plain", extract.qualified_name("plain"))


class IndexKeyTests(unittest.TestCase):
    def symbol(self, name, demangled):
        return extract.Symbol(name, demangled, Path("o.obj"), 1, 0)

    def test_a_method_is_findable_by_its_qualified_name(self):
        keys = extract.index_keys(
            self.symbol("__ZN9AlphaMenu16requested_heightEv",
                        "AlphaMenu::requested_height()"))
        self.assertIn("AlphaMenu::requested_height", keys)
        self.assertIn("AlphaMenu::requested_height()", keys)

    def test_a_c_symbol_is_findable_without_its_leading_underscore(self):
        # The redirect table writes `construct_basewin`; the object writes
        # `_construct_basewin`.
        keys = extract.index_keys(
            self.symbol("_construct_basewin", "_construct_basewin"))
        self.assertIn("construct_basewin", keys)


class PaddingTests(unittest.TestCase):
    def test_trailing_alignment_is_dropped(self):
        self.assertEqual(b"\x31\xc0\xc3",
                         extract.trim_padding(b"\x31\xc0\xc3" + b"\x90" * 13))

    def test_int3_padding_is_dropped_too(self):
        self.assertEqual(b"\xc3", extract.trim_padding(b"\xc3\xcc\xcc\xcc"))

    def test_a_nop_inside_the_body_is_kept(self):
        # Only TRAILING bytes are alignment. A 0x90 before a later instruction
        # executes, and dropping it would hand the prover a different program.
        self.assertEqual(b"\x90\x31\xc0\xc3",
                         extract.trim_padding(b"\x90\x31\xc0\xc3\x90"))

    def test_an_all_padding_body_becomes_empty(self):
        self.assertEqual(b"", extract.trim_padding(b"\x90\x90"))


class SectionTests(unittest.TestCase):
    """Bytes come from the symbol's own section, at a section-relative offset."""

    def coff(self, sections: list[bytes]) -> Path:
        # Minimal COFF: header, one section header each, then the raw data.
        count = len(sections)
        header = struct.pack("<HHIIIHH", 0x14C, count, 0, 0, 0, 0, 0)
        table_size = count * 40
        cursor = len(header) + table_size
        table, blob = b"", b""
        for index, data in enumerate(sections):
            name = f".s{index}".encode().ljust(8, b"\0")
            table += name + struct.pack("<IIIIIIHHI", 0, 0, len(data), cursor,
                                        0, 0, 0, 0, 0)
            blob += data
            cursor += len(data)
        path = Path(self.directory.name) / "t.obj"
        path.write_bytes(header + table + blob)
        return path

    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)

    def test_each_section_is_read_at_its_own_pointer(self):
        path = self.coff([b"AAAA", b"BBBBBB"])
        found = extract.section_data(path)
        self.assertEqual(b"AAAA", found[1][0])
        self.assertEqual(b"BBBBBB", found[2][0])

    def test_a_body_ends_at_the_next_symbol_in_the_same_section(self):
        first = extract.Symbol("a", "a", Path("o"), 1, 0)
        second = extract.Symbol("b", "b", Path("o"), 1, 3)
        code = extract.body_of(first, {1: (b"\x01\x02\x03\x04\x05", 5)},
                               [first, second])
        self.assertEqual(b"\x01\x02\x03", code)

    def test_a_symbol_in_another_section_does_not_end_this_body(self):
        # A COMDAT method at offset 3 of a DIFFERENT section must not truncate
        # a body in section 1. Getting this wrong yields short bodies that
        # still disassemble, which is a wrong answer and not a missing one.
        first = extract.Symbol("a", "a", Path("o"), 1, 0)
        elsewhere = extract.Symbol("b", "b", Path("o"), 2, 3)
        code = extract.body_of(first, {1: (b"\x01\x02\x03\x04\xc3", 5)},
                               [first, elsewhere])
        self.assertEqual(b"\x01\x02\x03\x04\xc3", code)

    def test_the_last_symbol_runs_to_the_end_of_its_section(self):
        only = extract.Symbol("a", "a", Path("o"), 1, 2)
        code = extract.body_of(only, {1: (b"\x00\x00\x31\xc0\xc3", 5)}, [only])
        self.assertEqual(b"\x31\xc0\xc3", code)


class RedirectTableTests(unittest.TestCase):
    def test_an_entry_maps_its_address_to_the_installed_symbol(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "dllmain.cpp"
            path.write_text(
                "    const RedirectSpec specs[] = {\n"
                "        {\n            0x00402F10,\n"
                "            reinterpret_cast<uintptr_t>(&construct_alphamenu_wave),\n"
                "            OPENSMACX_SIGNATURE_00402F10,\n        },\n    };\n")
            found = extract.redirect_targets(path)
        self.assertEqual({0x00402F10: "construct_alphamenu_wave"}, found)

    def test_a_qualified_target_keeps_its_scope(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "dllmain.cpp"
            path.write_text(
                "{ 0x00403610, reinterpret_cast<uintptr_t>(&Alpha::method), X },")
            self.assertEqual({0x00403610: "Alpha::method"},
                             extract.redirect_targets(path))


if __name__ == "__main__":
    unittest.main()
