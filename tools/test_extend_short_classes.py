#!/usr/bin/env python3
"""Unit tests for the short-class extender.

The compiler-backed parts need wine and are exercised by running the tool. What
is unit-tested here is the text surgery and the offset arithmetic, because both
have already been wrong once in ways that compiled.
"""

import unittest

import extend_short_classes as extender


class ExtendTest(unittest.TestCase):
    def test_the_member_lands_inside_the_named_class(self):
        text = "class A {\n  uint32_t a_;\n};\nclass B {\n  uint32_t b_;\n};\n"
        out = extender.extend(text, "B", 0x4, 0x10)
        before, after = out.split("class B {", 1)
        self.assertNotIn("field_4_", before)
        self.assertIn("uint8_t field_4_[0x10];", after)

    def test_a_nested_brace_does_not_end_the_class(self):
        text = ("class A {\n"
                "  void method() { if (x) { y(); } }\n"
                "  uint32_t a_;\n"
                "};\n")
        out = extender.extend(text, "A", 0x4, 0x8)
        # The member must come after the last real member, not after the
        # method body's first closing brace.
        self.assertLess(out.index("uint32_t a_;"),
                        out.index("uint8_t field_4_[0x8];"))
        self.assertLess(out.index("uint8_t field_4_[0x8];"), out.index("};"))

    def test_an_unknown_class_leaves_the_text_alone(self):
        text = "class A {\n  uint32_t a_;\n};\n"
        self.assertEqual(extender.extend(text, "Missing", 0, 4), text)

    def test_the_comment_states_the_reach_not_the_length(self):
        # The reader needs the address the image proves is touched, which is
        # offset + length, not the length on its own.
        out = extender.extend("class A {\n};\n", "A", 0xA14, 0x18)
        self.assertIn("reach 0xA2C", out)

    def test_sizes_are_written_as_hex(self):
        out = extender.extend("class A {\n};\n", "A", 0x2A3C, 0xCBF0)
        self.assertIn("uint8_t field_2A3C_[0xCBF0];", out)


class EmptyClassOffsetTest(unittest.TestCase):
    """An empty class is `sizeof == 1` and holds nothing.

    That byte exists only so two objects have distinct addresses and it
    disappears the moment a real member arrives, so padding from offset 1
    lands the class one byte short - which is what AlphaMenu and DeletionList
    did on the first run.
    """

    @staticmethod
    def placement(declared, bound):
        at = 0 if declared <= 1 else declared
        return at, bound - at

    def test_an_empty_class_pads_from_zero_and_reaches_the_bound(self):
        at, length = self.placement(1, 0xA30)
        self.assertEqual((at, length), (0, 0xA30))
        self.assertEqual(at + length, 0xA30)

    def test_a_non_empty_class_pads_from_its_declared_end(self):
        at, length = self.placement(0xA14, 0xA2C)
        self.assertEqual((at, length), (0xA14, 0x18))
        self.assertEqual(at + length, 0xA2C)


class TailMembersTest(unittest.TestCase):
    """The gate that decides whether the tail gets names or stays opaque."""

    def test_a_boundary_landing_on_the_declared_end_is_accepted(self):
        table = {"S": {0x0: ("base", 0xA14), 0xA14: ("a", 4),
                       0xA18: ("b", 4)}}
        found = extender.tail_members("S", 0xA14, 0xA1C, table)
        self.assertEqual(found, [(0xA14, "a", 4), (0xA18, "b", 4)])

    def test_no_member_at_the_declared_end_is_refused(self):
        """The IDB's offsets accumulate from member sizes, so a member nobody
        entered shifts everything after it. A table that independently starts
        a member at the byte src/ finished at agrees about everything before
        it; one that does not has said nothing this can rely on. NetDaemon is
        the live case, and it is also one of the four whose IDB total the
        access bound contradicts."""
        table = {"S": {0x0: ("base", 0x10), 0x10: ("a", 4)}}
        self.assertEqual(extender.tail_members("S", 0x14, 0x40, table), [])

    def test_an_unknown_class_is_refused(self):
        self.assertEqual(extender.tail_members("Missing", 0, 0x10, {}), [])

    def test_members_past_the_bound_are_not_returned(self):
        table = {"S": {0x0: ("a", 4), 0x4: ("b", 4), 0x8: ("c", 4)}}
        found = extender.tail_members("S", 0x0, 0x8, table)
        self.assertEqual([offset for offset, _, _ in found], [0x0, 0x4])


class TailTextTest(unittest.TestCase):
    def test_without_members_it_is_one_opaque_block(self):
        text = extender.tail_text("S", 0xA14, 0x18, [])
        self.assertIn("uint8_t field_A14_[0x18];", text)

    def test_with_members_the_class_still_ends_where_it_did(self):
        """Splitting the block changes how much of the tail has names, never
        how big it is - the remainder after the last member is padded."""
        members = [(0x10, "a", 4), (0x14, "b", 4)]
        text = extender.tail_text("S", 0x10, 0x20, members)
        self.assertIn("uint32_t a_;", text)
        self.assertIn("uint32_t b_;", text)
        # 0x10 + 0x20 = 0x30, members cover to 0x18, so 0x18 pads 0x18 bytes.
        self.assertIn("uint8_t field_18_[0x18];", text)

    def test_a_hole_between_members_is_padded(self):
        members = [(0x0, "a", 4), (0x10, "b", 4)]
        text = extender.tail_text("S", 0x0, 0x14, members)
        self.assertIn("uint8_t field_4_[0xC];", text)

    def test_a_member_overrunning_the_bound_is_dropped(self):
        members = [(0x0, "a", 4), (0x4, "big", 0x100)]
        text = extender.tail_text("S", 0x0, 0x8, members)
        self.assertIn("uint32_t a_;", text)
        self.assertNotIn("big_", text)
        self.assertIn("uint8_t field_4_[0x4];", text)


if __name__ == "__main__":
    unittest.main()


class StaleNoteTest(unittest.TestCase):
    """A class extended twice must not keep both notes.

    `designwin.h` carried "its own methods reach 0x14210" thirty lines above
    "reach 0x145A4"; twelve headers had such a pair, and the older number in
    each was simply false. Appending was the whole cause.
    """

    CLASS = ("class DLLEXPORT C : Base {\n public:\n  uint32_t a_;  // 0x0\n"
             "};\n")

    def test_extending_twice_leaves_one_note(self):
        once = extender.extend(self.CLASS, "C", 0x10, 0x10)
        twice = extender.extend(once, "C", 0x20, 0x10)
        self.assertEqual(
            twice.count("// Storage the image proves is here"), 1)
        self.assertIn("reach 0x30", twice)
        self.assertNotIn("reach 0x20.", twice)

    def test_the_declarations_under_a_retired_note_survive(self):
        # They are real members later work has named; deleting them would be a
        # layout change wearing a comment fix's clothes.
        once = extender.extend(self.CLASS, "C", 0x10, 0x10)
        named = once.replace("uint8_t field_10_[0x10];",
                             "uint32_t proved_10_;  // 0x10\n"
                             "  uint8_t field_14_[0xC];")
        twice = extender.extend(named, "C", 0x20, 0x10)
        self.assertIn("uint32_t proved_10_;", twice)
        self.assertIn("uint8_t field_14_[0xC];", twice)

    def test_the_first_extension_is_unchanged_by_the_retirement_logic(self):
        once = extender.extend(self.CLASS, "C", 0x10, 0x10)
        self.assertIn("uint32_t a_;", once)
        self.assertIn("uint8_t field_10_[0x10];", once)
        self.assertTrue(once.rstrip().endswith("};"))

    def test_a_note_belonging_to_another_class_is_left_alone(self):
        two = ("class A {\n public:\n"
               "  // Storage the image proves is here: its own methods reach 0x8.\n"
               "  uint8_t field_0_[0x8];\n};\n") + self.CLASS
        out = extender.extend(two, "C", 0x10, 0x10)
        self.assertIn("reach 0x8.", out)
        self.assertEqual(out.count("// Storage the image proves is here"), 2)


class TidyTest(unittest.TestCase):
    """The sweep for notes that were stacked before `extend` stopped stacking
    them. Twelve headers carried a pair, and in every pair the earlier number
    was false."""

    TWO_NOTES = (
        "class DLLEXPORT C : Base {\n public:\n"
        "  // Storage the image proves is here: its own methods reach 0xA38.\n"
        "  // Extent only - this class carries no size assertion, and the bound is a floor.\n"
        "  // 8 member(s) from the IDA database, 1 named; it starts a member at 0xA14.\n"
        "\n"
        "  uint32_t field_A14_;  // 0xA14\n"
        "\n"
        "  // Storage the image proves is here: its own methods reach 0x4614.\n"
        "  // Extent only - this class carries no size assertion, and the bound is a floor.\n"
        "  uint32_t field_A38_;  // 0xA38\n"
        "};\n")

    def test_only_the_last_note_survives(self):
        out, removed = extender.tidy(self.TWO_NOTES)
        self.assertEqual(removed, 1)
        self.assertEqual(out.count("// Storage the image proves is here"), 1)
        self.assertIn("reach 0x4614", out)
        self.assertNotIn("reach 0xA38", out)

    def test_the_declarations_under_the_retired_note_stay(self):
        out, _ = extender.tidy(self.TWO_NOTES)
        self.assertIn("uint32_t field_A14_;", out)
        self.assertIn("uint32_t field_A38_;", out)

    def test_a_single_note_is_left_alone(self):
        one = ("class C {\n public:\n"
               "  // Storage the image proves is here: its own methods reach 0x8.\n"
               "  uint8_t field_0_[0x8];\n};\n")
        out, removed = extender.tidy(one)
        self.assertEqual((out, removed), (one, 0))

    def test_notes_in_two_different_classes_are_not_a_pair(self):
        two = ("class A {\n public:\n"
               "  // Storage the image proves is here: its own methods reach 0x8.\n"
               "  uint8_t a_[0x8];\n};\n"
               "class B {\n public:\n"
               "  // Storage the image proves is here: its own methods reach 0x10.\n"
               "  uint8_t b_[0x10];\n};\n")
        out, removed = extender.tidy(two)
        self.assertEqual(removed, 0)
        self.assertEqual(out, two)

    def test_tidying_twice_changes_nothing_the_second_time(self):
        once, _ = extender.tidy(self.TWO_NOTES)
        twice, removed = extender.tidy(once)
        self.assertEqual((twice, removed), (once, 0))
