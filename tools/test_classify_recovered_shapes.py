#!/usr/bin/env python3
"""Tests for the shape classifier behind `unproven_by_shape`.

The shapes decide how a 188,443-byte figure is split, and a wrong one is not
visible in the total: misclassifying a looping body as straight_line would make
the symbolic route look reachable for bytes it can never discharge. So the
decisions are tested on hand-assembled instruction bytes at invented addresses -
never bytes copied out of the pinned executable, which is the same rule the other
decoder tests here follow.
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402

import classify_recovered_shapes as classifier  # noqa: E402

BASE = 0x00401000


class FakeSection:
    def __init__(self, virtual_address, size, raw_pointer):
        self.VirtualAddress = virtual_address
        self.Misc_VirtualSize = size
        self.SizeOfRawData = size
        self.PointerToRawData = raw_pointer


class FakePE:
    def __init__(self, code):
        self.OPTIONAL_HEADER = type("H", (), {"ImageBase": 0x00400000})()
        self.sections = [FakeSection(0x1000, max(len(code), 1), 0x200)]
        self.__data__ = b"\x00" * 0x200 + code


def row(code_len, locations="src/x.cpp:1"):
    return {
        "address": f"0x{BASE:08X}",
        "name": "?f@@YAXXZ",
        "body_ranges": f"0x{BASE:08X}-0x{BASE + code_len:08X}",
        "source_locations": locations,
    }


def shape_of(code, locations="src/x.cpp:1"):
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    return classifier.body_shape(FakePE(code), md, row(len(code), locations))


class ShapeTests(unittest.TestCase):
    def test_a_straight_line_body_is_straight_line(self):
        # mov eax, 1 / ret
        self.assertEqual("straight_line",
                         shape_of(b"\xb8\x01\x00\x00\x00\xc3"))

    def test_a_FORWARD_branch_is_still_straight_line(self):
        # A forward jump is a diamond, not a loop: one symbolic verification
        # condition covers it with an ITE. Treating any branch as a loop would
        # move most of the bucket to the wrong side.
        # xor eax,eax / jz +1 / nop / ret
        self.assertEqual("straight_line",
                         shape_of(b"\x31\xc0\x74\x01\x90\xc3"))

    def test_a_BACKWARD_branch_is_a_loop(self):
        # nop / jmp -3 (back onto the nop)
        self.assertEqual("loop", shape_of(b"\x90\xeb\xfd\xc3"))

    def test_a_self_jump_is_a_loop(self):
        # jmp -2, onto itself
        self.assertEqual("loop", shape_of(b"\xeb\xfe"))

    def test_the_LOOP_instruction_is_a_loop_even_branching_forward(self):
        # `loop` has no forward form worth relying on; the mnemonic settles it.
        # loop +2 / ret
        self.assertEqual("loop", shape_of(b"\xe2\x02\xc3\xc3"))

    def test_an_x87_body_is_x87_and_that_beats_loop(self):
        # fld1 / nop / jmp -3: both properties present, and x87 must win,
        # because a symbolic route refuses x87 outright rather than unrolling.
        self.assertEqual("x87", shape_of(b"\xd9\xe8\x90\xeb\xfb\xc3"))

    def test_a_SEAM_FORWARDING_body_is_classified_from_its_source_file(self):
        # Beats every byte-level property: whatever the original body does, the
        # recovered twin hands the original address onward, so a differential
        # compares the original with itself. This is the floor.
        self.assertEqual(
            "seam_forwarding",
            shape_of(b"\xb8\x01\x00\x00\x00\xc3",
                     locations="src/atexit_thunks.cpp:210"))
        self.assertEqual(
            "seam_forwarding",
            shape_of(b"\x90\xeb\xfd\xc3", locations="src/init_thunks.cpp:99"))

    def test_a_body_no_decoder_can_read_is_UNDECODABLE_not_guessed(self):
        # An empty read must not silently become straight_line, which would
        # advertise it to a mechanism that cannot see it at all.
        md = Cs(CS_ARCH_X86, CS_MODE_32)
        md.detail = True
        empty = {
            "address": "0x00900000",
            "name": "?f@@YAXXZ",
            "body_ranges": "0x00900000-0x00900010",
            "source_locations": "src/x.cpp:1",
        }
        self.assertEqual("undecodable",
                         classifier.body_shape(FakePE(b"\xc3"), md, empty))

    def test_fs_and_far_are_not_read_as_x87(self):
        # Capstone spells the x87 set f*, and these two collide with it.
        # mov eax, fs:[0]
        self.assertEqual("straight_line",
                         shape_of(b"\x64\xa1\x00\x00\x00\x00\xc3"))


class RangeTests(unittest.TestCase):
    def test_each_span_of_a_split_body_is_read_separately(self):
        # 416 catalogued functions are split. Reading `end - address` in one go
        # runs past the body into whatever follows it - measured at up to 2,102
        # bytes of another function.
        self.assertEqual([(0x401000, 0x401004), (0x650000, 0x650008)],
                         classifier.parse_ranges(
                             "0x00401000-0x00401004;0x00650000-0x00650008"))

    def test_an_empty_range_column_yields_nothing_rather_than_failing(self):
        self.assertEqual([], classifier.parse_ranges(""))
        self.assertEqual([], classifier.parse_ranges(None))


if __name__ == "__main__":
    unittest.main()
