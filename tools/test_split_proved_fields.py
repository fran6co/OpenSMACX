#!/usr/bin/env python3
"""Unit tests for the slab splitter.

The property everything else depends on is EXTENT PRESERVATION: whatever this
emits in place of `uint8_t field_X_[N];` must occupy exactly N bytes at exactly
the same offsets, or every member after it moves and every recovered body that
indexes past it is wrong. `--apply` re-probes with the compiler and reverts on a
size change, which is the real guard; these tests are what makes a revert rare
rather than the normal outcome, and they cover the two ways the arithmetic can
be wrong without the compiler catching it - a gap dropped at the end, and an
overlapping access counted twice.
"""

import tempfile
import unittest
from pathlib import Path

import split_proved_fields as splitter


def extent(pieces, start, size):
    """Total bytes the emitted pieces account for."""
    total, end = 0, start + size
    for index, (offset, width, _) in enumerate(pieces):
        stop = pieces[index + 1][0] if index + 1 < len(pieces) else end
        total += width or (stop - offset)
    return total


class PartitionTest(unittest.TestCase):
    def test_a_proved_dword_becomes_a_scalar_and_the_rest_stays_a_slab(self):
        pieces = splitter.partition(0x100, 0x10, {(0x104, 4): "e"}, {})
        self.assertEqual(pieces, [(0x100, 0, ""), (0x104, 4, "uint32_t"),
                                  (0x108, 0, "")])

    def test_the_pieces_cover_the_slab_exactly(self):
        for touched in ({(0x104, 4): "e"},
                        {(0x100, 4): "e", (0x10C, 4): "e"},
                        {(0x102, 2): "e", (0x108, 1): "e"},
                        {}):
            pieces = splitter.partition(0x100, 0x10, touched, {})
            self.assertEqual(extent(pieces, 0x100, 0x10), 0x10, touched)

    def test_a_slab_read_whole_says_nothing_about_its_contents(self):
        # `memset(this + 0x100, 0, 0x10)` reaches every byte and names none.
        pieces = splitter.partition(0x100, 0x10, {(0x100, 0x10): "e"}, {})
        self.assertEqual([width for _, width, _ in pieces], [0])

    def test_an_unaligned_dword_is_left_inside_the_slab(self):
        # The compiler would slide it to the next multiple of four and take
        # every later member with it. Better unnamed than moved.
        pieces = splitter.partition(0x100, 0x10, {(0x102, 4): "e"}, {})
        self.assertEqual([width for _, width, _ in pieces], [0])

    def test_a_wider_access_swallows_a_narrower_one_inside_it(self):
        pieces = splitter.partition(0x100, 0x10,
                                    {(0x100, 4): "e", (0x101, 1): "e"}, {})
        named = [(offset, width) for offset, width, _ in pieces if width]
        self.assertEqual(named, [(0x100, 4)])
        self.assertEqual(extent(pieces, 0x100, 0x10), 0x10)

    def test_the_widest_access_at_one_offset_wins(self):
        pieces = splitter.partition(0x100, 0x10,
                                    {(0x100, 1): "e", (0x100, 4): "e"}, {})
        self.assertEqual([(o, w) for o, w, _ in pieces if w], [(0x100, 4)])

    def test_an_access_running_past_the_slab_end_is_not_declared(self):
        pieces = splitter.partition(0x100, 0x10, {(0x10E, 4): "e"}, {})
        self.assertEqual([width for _, width, _ in pieces], [0])
        self.assertEqual(extent(pieces, 0x100, 0x10), 0x10)

    def test_an_access_outside_the_slab_is_ignored(self):
        pieces = splitter.partition(0x100, 0x10, {(0x200, 4): "e"}, {})
        self.assertEqual([width for _, width, _ in pieces], [0])

    def test_a_byte_and_a_word_get_their_own_widths(self):
        pieces = splitter.partition(0x100, 0x10,
                                    {(0x100, 1): "e", (0x102, 2): "e"}, {})
        named = [(offset, width, kind) for offset, width, kind in pieces
                 if width]
        self.assertEqual(named, [(0x100, 1, "uint8_t"), (0x102, 2, "uint16_t")])

    def test_eight_bytes_is_a_double_only_when_x87_said_so(self):
        plain = splitter.partition(0x100, 0x10, {(0x100, 8): "e"}, {})
        self.assertEqual([width for _, width, _ in plain], [0])
        typed = splitter.partition(0x100, 0x10, {(0x100, 8): "e"},
                                   {(0x100, 8): "fstp qword"})
        self.assertEqual([(o, w, k) for o, w, k in typed if w],
                         [(0x100, 8, "double")])


class DeclarationTest(unittest.TestCase):
    def test_the_house_style_is_type_name_and_offset_comment(self):
        lines = splitter.declarations([(0x104, 4, "uint32_t")], 0x108, {})
        self.assertEqual(lines, ["  uint32_t field_104_;  // 0x104"])

    def test_a_gap_is_a_smaller_slab_of_the_right_length(self):
        lines = splitter.declarations([(0x100, 0, "")], 0x110, {})
        self.assertEqual(lines, ["  uint8_t field_100_[0x10];  // 0x100"])

    def test_an_x87_dword_is_declared_float_rather_than_uint32(self):
        lines = splitter.declarations([(0x100, 4, "uint32_t")], 0x104,
                                      {(0x100, 4): "fld dword"})
        self.assertIn("float field_100_;", lines[0])

    def test_a_zero_length_gap_emits_nothing(self):
        self.assertEqual(splitter.declarations([(0x100, 0, "")], 0x100, {}), [])


class SlabSelectionTest(unittest.TestCase):
    """Only this tree's unknown-storage idiom. A byte array called
    `graphicWin_` is an embedded object held as bytes on purpose, and the
    accesses inside it belong to GraphicWin's layout, not to this class."""

    def test_an_unknown_storage_slab_is_selected(self):
        found = splitter.slabs(
            {"C": [("field_100_", 0x100, 0x20, "uint8_t")]}, {"C": 0x120})
        self.assertEqual(found["C"], [("field_100_", 0x100, 0x20)])

    def test_an_embedded_object_is_not_selected(self):
        found = splitter.slabs(
            {"C": [("graphicWin_", 0, 0xA14, "uint8_t")]}, {"C": 0xA14})
        self.assertEqual(found, {})

    def test_the_unmapped_spellings_are_selected(self):
        for name in ("unmapped_", "unmapped_0_", "unmapped_A14_",
                     "derived_tail_", "derived_storage_", "field_0_"):
            found = splitter.slabs({"C": [(name, 0, 0x20, "uint8_t")]},
                                   {"C": 0x20})
            self.assertIn("C", found, name)

    def test_a_named_field_that_is_not_storage_is_not_selected(self):
        for name in ("name_", "file_name_", "data_", "base_storage_",
                     "buffer_", "scroll_"):
            found = splitter.slabs({"C": [(name, 0, 0x20, "uint8_t")]},
                                   {"C": 0x20})
            self.assertEqual(found, {}, name)

    def test_an_inherited_member_belongs_to_its_own_class(self):
        # Splitting it here would declare the field twice, once per derived
        # class, and only one of them would be the class that owns it.
        found = splitter.slabs(
            {"C": [("Base::field_100_", 0x100, 0x20, "uint8_t")]}, {"C": 0x120})
        self.assertEqual(found, {})

    def test_a_non_byte_member_is_not_a_slab(self):
        found = splitter.slabs({"C": [("field_100_", 0x100, 0x20, "RECT")]},
                               {"C": 0x120})
        self.assertEqual(found, {})

    def test_storage_under_eight_bytes_is_left_alone(self):
        found = splitter.slabs({"C": [("field_100_", 0x100, 4, "uint8_t")]},
                               {"C": 0x104})
        self.assertEqual(found, {})


class RewriteTest(unittest.TestCase):
    def test_the_declaration_line_is_replaced_in_place(self):
        text = ("class C {\n public:\n  uint32_t a_;  // 0x0\n"
                "  uint8_t field_4_[0x8];  // 0x4\n  uint32_t z_;  // 0xC\n};\n")
        out = splitter.rewrite(text, "field_4_",
                               ["  uint32_t field_4_;  // 0x4",
                                "  uint32_t field_8_;  // 0x8"])
        self.assertIn("uint32_t field_4_;", out)
        self.assertIn("uint32_t field_8_;", out)
        self.assertNotIn("field_4_[0x8]", out)
        self.assertIn("uint32_t z_;", out)

    def test_a_member_that_is_not_there_leaves_the_text_alone(self):
        text = "class C { uint32_t a_; };\n"
        self.assertEqual(splitter.rewrite(text, "field_4_", ["x"]), text)

    def test_a_similarly_named_member_is_not_hit(self):
        text = "  uint8_t field_40_[0x8];  // 0x40\n"
        self.assertEqual(splitter.rewrite(text, "field_4_", ["x"]), text)

    def test_an_access_label_on_the_same_line_does_not_hide_it(self):
        text = " private: uint8_t field_4_[0x8];  // 0x4\n"
        out = splitter.rewrite(text, "field_4_", ["  uint32_t field_4_;"])
        self.assertIn("uint32_t field_4_;", out)


class PlanTest(unittest.TestCase):
    def test_a_generated_header_is_never_planned(self):
        # An edit there lasts until emit_hypothesis_layouts.py next runs.
        work = splitter.plan({"C": [("field_0_", 0, 0x10, "uint8_t")]},
                             {"C": 0x10}, {"C": {(4, 4): "e"}}, {},
                             {"C": "hypothesis_layouts.h"})
        self.assertEqual(work, {})

    def test_a_slab_the_image_never_reaches_into_is_not_planned(self):
        work = splitter.plan({"C": [("field_0_", 0, 0x10, "uint8_t")]},
                             {"C": 0x10}, {"C": {}}, {}, {"C": "c.h"})
        self.assertEqual(work, {})

    def test_a_planned_slab_carries_its_lines_and_its_count(self):
        work = splitter.plan({"C": [("field_0_", 0, 0x10, "uint8_t")]},
                             {"C": 0x10}, {"C": {(4, 4): "e"}}, {},
                             {"C": "c.h"})
        (klass, member, lines, named), = work["c.h"]
        self.assertEqual((klass, member, named), ("C", "field_0_", 1))
        self.assertTrue(any("uint32_t field_4_;" in line for line in lines))


if __name__ == "__main__":
    unittest.main()
