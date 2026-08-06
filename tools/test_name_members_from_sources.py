#!/usr/bin/env python3
"""Unit tests for the placeholder-naming pass.

A rename here cannot move a byte, which is what makes it safe. These tests
guard the two ways that could stop being true: matching on offset without
matching on width, and adopting a name that is itself a placeholder.
"""

import collections
import unittest

import name_members_from_sources as namer


class WidthTest(unittest.TestCase):
    def test_known_scalars(self):
        self.assertEqual(namer.declared_width("uint8_t", "", ""), 1)
        self.assertEqual(namer.declared_width("uint16_t", "", ""), 2)
        self.assertEqual(namer.declared_width("uint32_t", "", ""), 4)

    def test_a_pointer_is_four_bytes_on_this_target(self):
        self.assertEqual(namer.declared_width("Buffer", "*", ""), 4)

    def test_an_array_multiplies(self):
        self.assertEqual(namer.declared_width("uint32_t", "", "[4]"), 16)
        self.assertEqual(namer.declared_width("uint8_t", "", "[0x10]"), 16)

    def test_an_unknown_type_returns_zero_rather_than_a_guess(self):
        # 0 means "do not rename". Guessing four would turn the
        # offset-and-width match into an offset-only match, which is the whole
        # check.
        self.assertEqual(namer.declared_width("SomeClass", "", ""), 0)
        self.assertEqual(namer.declared_width("uint32_t", "", "[COUNT]"), 0)


class PlaceholderTest(unittest.TestCase):
    def test_both_trees_notations_are_refused(self):
        # IDA writes field_9E8. Thinker writes the member's TYPE followed by
        # its offset, so adopting `dwordC` would replace one tree's
        # placeholder with another's.
        for name in ("field_9E8", "unk1", "gap0", "pad", "dwordC", "dword10",
                     "word4", "byte8", ""):
            self.assertTrue(namer.PLACEHOLDER_NAME.match(name), name)

    def test_real_names_survive(self):
        for name in ("iFlags", "poOwner", "lineHeight", "wordCount",
                     "byteOrder", "dwordAlignedTotal"):
            self.assertFalse(namer.PLACEHOLDER_NAME.match(name), name)


class PlanTest(unittest.TestCase):
    def source(self, **members):
        return {"S": members}

    def test_a_matching_offset_and_width_renames(self):
        text = "class S {\n  uint32_t field_8_;\n};"
        renames, skipped = namer.plan(text, {"S": {8: ("iFlags", 4, "Thinker")}})
        self.assertEqual([(old, new) for old, new, _, _ in renames],
                         [("field_8_", "iFlags_")])
        self.assertEqual(skipped, [])

    def test_a_width_disagreement_refuses(self):
        """The IDB saying 16 bytes where src/ declares 4 is a disagreement
        about the layout, not a naming opportunity."""
        text = "class S {\n  uint32_t field_8_;\n};"
        renames, skipped = namer.plan(text, {"S": {8: ("thing", 16, "the IDB")}})
        self.assertEqual(renames, [])
        self.assertEqual(len(skipped), 1)
        self.assertIn("16 byte(s)", skipped[0][1])

    def test_an_unsizeable_declaration_refuses(self):
        text = "class S {\n  SomeClass field_8_;\n};"
        renames, skipped = namer.plan(text, {"S": {8: ("thing", 4, "the IDB")}})
        self.assertEqual(renames, [])
        self.assertEqual(len(skipped), 1)

    def test_an_offset_no_member_sits_at_is_left_alone(self):
        text = "class S {\n  uint32_t field_8_;\n};"
        renames, _ = namer.plan(text, {"S": {0x40: ("thing", 4, "the IDB")}})
        self.assertEqual(renames, [])

    def test_a_name_already_in_the_class_is_made_unique(self):
        text = "class S {\n  uint32_t iFlags_;\n  uint32_t field_8_;\n};"
        renames, _ = namer.plan(text, {"S": {8: ("iFlags", 4, "Thinker")}})
        self.assertEqual(renames[0][1], "iFlags__")


class OrphanTest(unittest.TestCase):
    def test_a_source_member_src_has_no_member_at_is_reported(self):
        """Either the source's table is truncated - the IDB accumulates
        offsets, so one missing member shifts every later one - or src/'s
        layout is wrong. Both want a person, not this tool."""
        text = "class S {\n  uint32_t field_0_;\n};"
        found = namer.orphans(text, {"S": {0x40: ("thing", 4, "the IDB")}})
        self.assertEqual([(name, offset) for name, offset, _, _ in found],
                         [("S", 0x40)])

    def test_an_offset_src_declares_a_real_member_at_is_not_an_orphan(self):
        text = "class S {\n  uint32_t field_0_;\n  uint32_t named_;\n};"
        found = namer.orphans(text, {"S": {0x0: ("thing", 4, "the IDB")}})
        self.assertEqual(found, [])


if __name__ == "__main__":
    unittest.main()
