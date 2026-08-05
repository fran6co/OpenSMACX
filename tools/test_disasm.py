#!/usr/bin/env python3

import json
import pathlib
import tempfile
import unittest
from unittest import mock

import disasm


FUNCTIONS = {
    0x00618F30: {"name": "?UNK1@Font@@QAEHHHHH@Z", "size": "8"},
    0x005D4510: {"name": "??0Buffer@@QAE@XZ", "size": "530"},
    0x00400000: {"name": "", "size": ""},
    0x005E3650: {"name": "sub_5e3650", "size": "64"},
    0x00401C80: {"name": "SessionStruct::SessionStruct", "size": "96"},
}


class ResolveTest(unittest.TestCase):
    def test_accepts_a_hex_address_and_reports_the_catalogued_size(self):
        address, size = disasm.resolve("0x00618F30", FUNCTIONS)
        self.assertEqual((0x00618F30, 8), (address, size))

    def test_accepts_a_mangled_name(self):
        # Typing the name is how this gets used in practice; requiring the
        # address first would mean a lookup before every disassembly.
        address, _ = disasm.resolve("??0Buffer@@QAE@XZ", FUNCTIONS)
        self.assertEqual(0x005D4510, address)

    def test_accepts_the_recovery_symbol_of_a_disassembler_label(self):
        # Mizuchi's {{functionName}} carries the symbol both objects share,
        # and for these rows that is a decoration of the catalogued label.
        # Without this the context emitter refuses every such prompt.
        for symbol in ("_sub_5e3650", "_sub_5e3650@8", "@sub_5e3650@8"):
            address, _ = disasm.resolve(symbol, FUNCTIONS)
            self.assertEqual(0x005E3650, address, symbol)

    def test_accepts_a_synthesised_identifier_by_its_address(self):
        # `SessionStruct::SessionStruct` is not spellable, so the emitter mints
        # `fn_00401c80` from the address - which is the only way back.
        address, _ = disasm.resolve("_fn_00401c80@4", FUNCTIONS)
        self.assertEqual(0x00401C80, address)

    def test_rejects_something_that_is_neither(self):
        with self.assertRaises(ValueError):
            disasm.resolve("Buffer::Buffer", FUNCTIONS)

    def test_rejects_a_symbol_naming_nothing_catalogued(self):
        with self.assertRaises(ValueError):
            disasm.resolve("_sub_999999@4", FUNCTIONS)
        with self.assertRaises(ValueError):
            disasm.resolve("_fn_00999999", FUNCTIONS)

    def test_reports_no_size_for_an_uncatalogued_address(self):
        address, size = disasm.resolve("0x00777777", FUNCTIONS)
        self.assertEqual((0x00777777, None), (address, size))

    def test_reports_no_size_when_the_row_has_none(self):
        _, size = disasm.resolve("0x00400000", FUNCTIONS)
        self.assertIsNone(size)


class AnnotateTest(unittest.TestCase):
    def test_names_a_call_target(self):
        self.assertIn("??0Buffer@@QAE@XZ",
                      disasm.annotate("call", "0x5d4510", FUNCTIONS))

    def test_names_a_jump_target(self):
        self.assertIn("??0Buffer@@QAE@XZ",
                      disasm.annotate("jmp", "0x5d4510", FUNCTIONS))

    def test_says_nothing_about_a_register_call(self):
        self.assertEqual("", disasm.annotate("call", "eax", FUNCTIONS))

    def test_says_nothing_about_a_non_branch(self):
        self.assertEqual("", disasm.annotate("mov", "0x5d4510", FUNCTIONS))

    def test_says_nothing_about_an_unknown_target(self):
        self.assertEqual("", disasm.annotate("call", "0x999999", FUNCTIONS))


class ReadRangeTest(unittest.TestCase):
    def make_image(self, virtual, raw, size):
        section = mock.Mock()
        section.VirtualAddress = virtual
        section.PointerToRawData = raw
        section.Misc_VirtualSize = size
        section.SizeOfRawData = size
        image = mock.Mock()
        image.OPTIONAL_HEADER.ImageBase = 0x00400000
        image.sections = [section]
        image.__data__ = bytes(range(256)) * 8
        return image

    def test_maps_a_virtual_address_to_file_bytes(self):
        image = self.make_image(virtual=0x1000, raw=0x400, size=0x200)
        data = disasm.read_range(image, 0x00401000, 4)
        self.assertEqual(image.__data__[0x400:0x404], data)

    def test_refuses_an_address_outside_every_section(self):
        # Silently returning empty bytes would look like a zero-length
        # function rather than a bad address.
        image = self.make_image(virtual=0x1000, raw=0x400, size=0x200)
        with self.assertRaises(ValueError):
            disasm.read_range(image, 0x00500000, 4)


class ExactNameWinsTest(unittest.TestCase):
    """`__exit` is a catalogued row AND the decoration of the row `_exit`.

    CL decorates source `exit` to `_exit` and source `_exit` to `__exit`, so
    both are real and distinct. Trying the exact name and the undecorated one
    together as a set let whichever came first in the catalogue win.
    """

    FUNCTIONS = {0x00644DFF: {"address": "0x00644DFF", "size": "17",
                              "name": "_exit"},
                 0x00644E10: {"address": "0x00644E10", "size": "16",
                              "name": "__exit"}}

    def setUp(self):
        disasm.symbol_map.cache_clear()
        self.addCleanup(disasm.symbol_map.cache_clear)

    def test_each_resolves_to_its_own_row(self):
        with tempfile.TemporaryDirectory() as tmp:
            missing = pathlib.Path(tmp) / "absent.json"
            with mock.patch.object(disasm, "SYMBOL_MAP", missing):
                self.assertEqual(0x00644E10,
                                 disasm.resolve("__exit", self.FUNCTIONS)[0])
                self.assertEqual(0x00644DFF,
                                 disasm.resolve("_exit", self.FUNCTIONS)[0])

    def test_a_decoration_with_no_row_of_its_own_still_undecorates(self):
        functions = {0x005E3650: {"address": "0x005E3650", "size": "32",
                                  "name": "sub_5e3650"}}
        with tempfile.TemporaryDirectory() as tmp:
            missing = pathlib.Path(tmp) / "absent.json"
            with mock.patch.object(disasm, "SYMBOL_MAP", missing):
                self.assertEqual(0x005E3650,
                                 disasm.resolve("_sub_5e3650@8", functions)[0])


class SymbolMapResolutionTest(unittest.TestCase):
    """A C++ symbol cannot be undecorated back into its catalogued name.

    `?POP2@@YAHPBD0H@Z` is `?POP2@@YAHPBDPBDH@Z` with its back-references
    compressed and `?init@AlphaMenu@@QAEHPAVWin@@@Z` is the same name with
    the struct key the source declares. 221 of the 2,783 unrecovered rows
    are in that state, and the context emitter refused every one of them
    until `resolve` learned to read the symbol map.
    """

    def setUp(self):
        self.functions = {0x00405140: {"address": "0x00405140", "size": "64",
                                       "name": "?POP2@@YAHPBDPBDH@Z"}}
        disasm.symbol_map.cache_clear()
        self.addCleanup(disasm.symbol_map.cache_clear)

    def test_a_symbol_resolves_through_the_map(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / "symbol-map.json"
            path.write_text(json.dumps(
                {"?POP2@@YAHPBD0H@Z": "build/target-objects/00405140.obj"}))
            with mock.patch.object(disasm, "SYMBOL_MAP", path):
                address, _ = disasm.resolve("?POP2@@YAHPBD0H@Z",
                                            self.functions)
        self.assertEqual(0x00405140, address)

    def test_the_catalogued_name_still_wins_without_a_map(self):
        with tempfile.TemporaryDirectory() as tmp:
            missing = pathlib.Path(tmp) / "absent.json"
            with mock.patch.object(disasm, "SYMBOL_MAP", missing):
                address, _ = disasm.resolve("?POP2@@YAHPBDPBDH@Z",
                                            self.functions)
                self.assertEqual(0x00405140, address)
                with self.assertRaises(ValueError):
                    disasm.resolve("?POP2@@YAHPBD0H@Z", self.functions)


if __name__ == "__main__":
    unittest.main()