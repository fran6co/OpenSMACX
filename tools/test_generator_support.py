#!/usr/bin/env python3
"""Unit tests for the helpers the bulk generators share.

The PE fixtures are synthesised section headers at invented addresses, never
bytes copied from the pinned executable: what is under test is the address
arithmetic, and a committed fixture must not be derived from original bytes.
"""

from __future__ import annotations

import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import generator_support as support  # noqa: E402


class FakeSection:
    def __init__(self, virtual_address, virtual_size, raw_size, raw_pointer):
        self.VirtualAddress = virtual_address
        self.Misc_VirtualSize = virtual_size
        self.SizeOfRawData = raw_size
        self.PointerToRawData = raw_pointer


class FakeHeader:
    def __init__(self, base):
        self.ImageBase = base


class FakePE:
    def __init__(self, base, sections, data):
        self.OPTIONAL_HEADER = FakeHeader(base)
        self.sections = sections
        self.__data__ = data


class ReadBytesTest(unittest.TestCase):
    def image(self):
        # .text: 0x400 bytes present in the file at offset 0x200.
        # .data: 0x100 in the file at 0x600, but 0x1000 in memory, so 0xF00
        #        bytes of it exist only once the loader zero-fills them.
        # A third section follows in the FILE right after .data's raw bytes,
        # which is what a naive offset computation would wander into.
        text = FakeSection(0x1000, 0x400, 0x400, 0x200)
        data = FakeSection(0x2000, 0x1000, 0x100, 0x600)
        rsrc = FakeSection(0x4000, 0x100, 0x100, 0x700)
        blob = (b"\x00" * 0x200 + bytes(range(256)) * 4
                + b"\xaa" * 0x100 + b"\xbb" * 0x100)
        return FakePE(0x00400000, [text, data, rsrc], blob)

    def test_reads_bytes_that_exist_in_the_file(self):
        pe = self.image()
        self.assertEqual(bytes(range(8)),
                         support.read_bytes(pe, 0x00401000, 8))

    def test_an_address_in_no_section_reads_empty(self):
        pe = self.image()
        self.assertEqual(b"", support.read_bytes(pe, 0x00900000, 8))

    def test_the_virtual_tail_reads_EMPTY_not_the_next_section(self):
        # The whole point of this module's read_bytes. 0x00402100 is inside
        # .data's virtual extent but past its raw data; the bytes at the file
        # offset a naive computation lands on belong to .rsrc (0xbb here).
        # Handing a decoder those bytes lets it accept a shape that is not
        # there; handing it b"" makes it refuse the row.
        pe = self.image()
        self.assertEqual(b"", support.read_bytes(pe, 0x00402100, 8))
        self.assertNotIn(b"\xbb", support.read_bytes(pe, 0x00402100, 8))

    def test_a_read_is_clamped_to_its_own_section(self):
        # Four bytes before .data's raw data ends: only four come back, not
        # four plus the following section's.
        pe = self.image()
        got = support.read_bytes(pe, 0x00400000 + 0x2000 + 0x100 - 4, 16)
        self.assertEqual(4, len(got))
        self.assertEqual(b"\xaa" * 4, got)

    def test_the_last_byte_with_raw_data_still_reads(self):
        pe = self.image()
        self.assertEqual(
            b"\xaa", support.read_bytes(pe, 0x00402000 + 0xFF, 1))


class NameTest(unittest.TestCase):
    def test_snake_keeps_an_acronym_whole(self):
        self.assertEqual("http_server", support.snake("HTTPServer"))
        self.assertEqual("fx", support.snake("FX"))
        self.assertEqual("uv2_player", support.snake("UV2Player"))
        self.assertEqual("mci_video", support.snake("MCIVideo"))

    def test_snake_converts_the_ordinary_class_names(self):
        self.assertEqual("base_button", support.snake("BaseButton"))
        self.assertEqual("strings_init_ctor", support.snake("StringsInitCtor"))
        self.assertEqual("wave_device", support.snake("Wave_Device"))

    def test_snake_leaves_snake_case_alone(self):
        self.assertEqual("already_snake", support.snake("already_snake"))

    def test_camel_round_trips_the_separators_away(self):
        self.assertEqual("BaseButton", support.camel("base_button"))
        self.assertEqual("AB", support.camel("a__b"))

    def test_identifier_of_global_drops_the_marker_and_lowers(self):
        self.assertEqual("basewin_wave",
                         support.identifier_of_global("g_BASEWIN_WAVE"))
        self.assertEqual("iface_std_popups_top_left_sprite",
                         support.identifier_of_global(
                             "g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE"))

    def test_identifier_of_global_collapses_doubled_underscores(self):
        self.assertEqual("a_b", support.identifier_of_global("g_A__B"))

    def test_identifier_of_global_is_NOT_snake_case(self):
        # These two must never be merged. identifier_of_global runs over seam
        # names as well as global names, and the committed generated source
        # really does spell them run-together - src/init_thunks.cpp defines
        # `ambienceinitctor_default`. Routing that through snake() would
        # rename every generated trampoline.
        self.assertEqual("stringsinitctor",
                         support.identifier_of_global("StringsInitCtor"))
        self.assertEqual("strings_init_ctor", support.snake("StringsInitCtor"))


class SeamScanTest(unittest.TestCase):
    def headers(self, **files):
        directory = Path(tempfile.mkdtemp())
        self.addCleanup(shutil.rmtree, directory)
        for name, text in files.items():
            (directory / name).write_text(text, encoding="utf-8")
        return directory

    def test_it_reads_the_symbol_type_and_address(self):
        directory = self.headers(**{"a.h": "extern func_x *ThingTarget;  // 0x00401000\n"})
        self.assertEqual(
            [("a.h", "ThingTarget", "func_x", 0x00401000)],
            support.scan_seam_bindings(directory))

    def test_trailing_prose_after_the_address_still_counts(self):
        # The bug this scan was written to end. One of the three copies
        # anchored the address to end-of-line, so these two real shapes were
        # invisible to it and their addresses looked unbound.
        directory = self.headers(**{
            "console.h":
                "extern void *ConsoleInfoWin;      "
                "// 0x007AD2A0, the process-wide InfoWin\n",
            "listbox.h":
                "extern uint32_t *ListBoxCloseStaticDefaults;   "
                "// 0x006970E0 [0..3]\n",
        })
        found = {name: address
                 for _, name, _, address in support.scan_seam_bindings(directory)}
        self.assertEqual({"ConsoleInfoWin": 0x007AD2A0,
                          "ListBoxCloseStaticDefaults": 0x006970E0}, found)

    def test_the_star_may_sit_against_either_side(self):
        directory = self.headers(**{
            "a.h": "extern Foo* Attached;  // 0x00401000\n"
                   "extern Foo *Detached;  // 0x00402000\n",
        })
        self.assertEqual(
            {"Attached", "Detached"},
            {name for _, name, _, _ in support.scan_seam_bindings(directory)})

    def test_a_declaration_without_an_address_comment_is_not_a_seam(self):
        # The address is the whole point: seams dedupe on it, so a pointer
        # extern that does not name one cannot participate.
        directory = self.headers(**{"a.h": "extern Foo *Plain;\n"})
        self.assertEqual([], support.scan_seam_bindings(directory))

    def test_exclude_drops_a_generators_own_output(self):
        directory = self.headers(**{
            "mine.h": "extern Foo *Ours;  // 0x00401000\n",
            "other.h": "extern Foo *Theirs;  // 0x00402000\n",
        })
        self.assertEqual(
            ["Theirs"],
            [name for _, name, _, _
             in support.scan_seam_bindings(directory, exclude={"mine.h"})])

    def test_it_finds_the_seams_the_real_tree_declares(self):
        # Guards against a regex tightening that quietly stops seeing the
        # committed bindings: the count only moves when a seam is added or
        # removed on purpose.
        source = Path(__file__).resolve().parent.parent / "src"
        found = support.scan_seam_bindings(source)
        self.assertGreater(len(found), 200)
        by_address = {}
        for header, name, _, address in found:
            if address in by_address and by_address[address] != name:
                self.fail(f"0x{address:08X} is bound as two names: "
                          f"{by_address[address]} and {name} ({header})")
            by_address[address] = name


class LicenseTest(unittest.TestCase):
    def test_it_is_the_block_the_committed_generated_files_open_with(self):
        # The generators previously carried six copies of this, three of them
        # read out of src/vector_teardown.h at import time. Compare against a
        # committed generated file so a change here cannot silently diverge
        # from the tree.
        emitted = (Path(__file__).resolve().parent.parent / "src" /
                   "global_arith.cpp").read_text(encoding="utf-8")
        self.assertTrue(emitted.startswith(support.LICENSE))

    def test_it_ends_with_a_newline_so_a_header_can_follow_it(self):
        self.assertTrue(support.LICENSE.endswith("*/\n"))


if __name__ == "__main__":
    unittest.main()
