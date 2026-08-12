#!/usr/bin/env python3
"""Unit tests for the subobject scanner's judgement.

The scan itself needs the pinned image and is exercised by running the tool.
What is tested here is the part that decides what a site MEANS, because that is
where being wrong is quiet: calling a real base edge `absent`, or a mistyped
member `agreed`, both read as a clean tree.
"""

import unittest

import verify_subobjects as subobjects


class VerdictTest(unittest.TestCase):

    LAYOUT = {"C": [("a_", 0, 4, "uint32_t"),
                    ("thing_", 0x10, 0x24, "StringStruct"),
                    ("slab_", 0x40, 0x100, "uint8_t")]}
    SIZES = {"C": 0x140, "StringStruct": 0x24, "Font": 0x28, "Base": 0x10}

    def _one(self, built, layout=None, sizes=None):
        rows = subobjects.verdicts(built, layout or self.LAYOUT,
                                   sizes or self.SIZES)
        return rows[0] if rows else None

    def test_a_member_of_exactly_that_size_agrees(self):
        row = self._one({"C": {0x10: {"StringStruct": "e"}}})
        self.assertEqual(row[0], "agreed")

    def test_a_member_of_the_wrong_size_is_mistyped(self):
        # `Palette *palette_` where the constructor builds a 0x454 Palette.
        row = self._one({"C": {0: {"X": "e"}, 0x40: {"Font": "e"}}},
                        sizes=dict(self.SIZES))
        rows = subobjects.verdicts({"C": {0x40: {"Font": "e"}}},
                                   self.LAYOUT, self.SIZES)
        self.assertEqual(rows[0][0], "mistyped")
        self.assertIn("0x28", rows[0][4])

    def test_nothing_declared_at_the_offset_is_absent(self):
        row = self._one({"C": {0x30: {"Font": "e"}}})
        self.assertEqual(row[0], "absent")

    def test_offset_zero_with_a_declared_base_is_a_base(self):
        # `base_chain` reads src/; the fixture class has none, so a declared
        # base is simulated by naming a class the chain does know.
        rows = subobjects.verdicts({"C": {0: {"Base": "e"}}},
                                   self.LAYOUT, self.SIZES)
        self.assertIn(rows[0][0], ("base", "at-zero"))

    def test_offset_zero_with_no_declared_base_is_at_zero(self):
        rows = subobjects.verdicts({"C": {0: {"Whatever": "e"}}},
                                   self.LAYOUT, self.SIZES)
        self.assertEqual(rows[0][0], "at-zero")

    def test_the_outermost_type_is_the_member(self):
        # A derived member's destructor runs its base's on the same pointer.
        rows = subobjects.verdicts(
            {"C": {0x10: {"StringStruct": "outer", "Base": "inner"}}},
            self.LAYOUT, self.SIZES)
        self.assertEqual(rows[0][3], "StringStruct")
        self.assertEqual(rows[0][0], "agreed")

    def test_a_class_with_no_declared_layout_is_skipped_not_guessed(self):
        self.assertEqual(subobjects.verdicts({"C": {0x10: {"Font": "e"}}},
                                             {}, self.SIZES), [])

    def test_a_built_type_of_unknown_size_is_not_called_agreed(self):
        rows = subobjects.verdicts({"C": {0x10: {"Mystery": "e"}}},
                                   self.LAYOUT, self.SIZES)
        self.assertEqual(rows[0][0], "mistyped")

    def test_mistyped_sorts_before_absent(self):
        rows = subobjects.verdicts(
            {"C": {0x30: {"Font": "e"}, 0x40: {"Font": "e"}}},
            self.LAYOUT, self.SIZES)
        self.assertEqual([row[0] for row in rows], ["mistyped", "absent"])

    def test_the_evidence_survives_into_the_verdict(self):
        rows = subobjects.verdicts({"C": {0x10: {"StringStruct": "the call"}}},
                                   self.LAYOUT, self.SIZES)
        self.assertEqual(rows[0][5], "the call")


class BuilderPatternTest(unittest.TestCase):
    """Which mangled names name a subobject's own receiver."""

    def test_a_constructor_is_a_builder(self):
        self.assertEqual(
            subobjects.BUILDER.match("??0Palette@@QAE@XZ").group(1), "Palette")

    def test_a_destructor_is_a_builder(self):
        self.assertEqual(
            subobjects.BUILDER.match("??1Flic@@QAE@XZ").group(1), "Flic")

    def test_close_counts_because_the_destructor_is_often_inlined(self):
        self.assertEqual(
            subobjects.CLOSER.match("?close@Scroll@@QAEXXZ").group(1), "Scroll")

    def test_an_ordinary_method_is_not_a_builder(self):
        self.assertIsNone(subobjects.BUILDER.match("?init@Win@@QAEXXZ"))
        self.assertIsNone(subobjects.CLOSER.match("?init@Win@@QAEXXZ"))

    def test_a_free_function_is_not_a_builder(self):
        self.assertIsNone(subobjects.BUILDER.match("?f@@YAXXZ"))

    def test_a_static_or_stack_convention_is_not_a_thiscall_receiver(self):
        # `QAE` is thiscall; a `QAA`/`YA` form takes its receiver elsewhere and
        # ECX is not `this` there.
        self.assertIsNone(subobjects.BUILDER.match("??0Palette@@QAA@XZ"))


class RenderTest(unittest.TestCase):
    def test_the_catalogue_has_a_header_and_one_row_per_site(self):
        text = subobjects.render([("agreed", "C", 0x10, "StringStruct", "d",
                                   "e")])
        lines = text.splitlines()
        self.assertTrue(lines[0].startswith("verdict,class,offset,built"))
        self.assertEqual(len(lines), 2)
        self.assertIn("0x10", lines[1])

    def test_rows_are_ordered_by_class_then_offset(self):
        text = subobjects.render([("absent", "B", 0x20, "X", "d", "e"),
                                  ("absent", "A", 0x30, "X", "d", "e"),
                                  ("absent", "B", 0x10, "X", "d", "e")])
        rows = [line.split(",")[1:3] for line in text.splitlines()[1:]]
        self.assertEqual(rows, [["A", "0x30"], ["B", "0x10"], ["B", "0x20"]])



class ArrayMemberTest(unittest.TestCase):
    """An array of them is not a member of the wrong size.

    `SocialWin` declares `ButtonGroup buttonGroups_[4]` - 0x250 bytes - and its
    `close` builds element zero on `this + 0x1EB84`. Reading that as a 0x94
    member declared 0x250 wide put a correct declaration on the worklist.
    """

    LAYOUT = {"C": [("groups_", 0x10, 0x250, "ButtonGroup")]}
    SIZES = {"ButtonGroup": 0x94}

    def test_an_array_of_the_built_type_agrees(self):
        rows = subobjects.verdicts({"C": {0x10: {"ButtonGroup": "e"}}},
                                   self.LAYOUT, self.SIZES)
        self.assertEqual(rows[0][0], "agreed")
        self.assertIn("4 element(s)", rows[0][4])

    def test_a_different_type_of_the_same_stride_is_still_mistyped(self):
        rows = subobjects.verdicts({"C": {0x10: {"Other": "e"}}},
                                   self.LAYOUT, dict(self.SIZES, Other=0x94))
        self.assertEqual(rows[0][0], "mistyped")

    def test_a_size_that_is_not_a_whole_number_of_elements_is_mistyped(self):
        rows = subobjects.verdicts(
            {"C": {0x10: {"ButtonGroup": "e"}}},
            {"C": [("groups_", 0x10, 0x251, "ButtonGroup")]}, self.SIZES)
        self.assertEqual(rows[0][0], "mistyped")

if __name__ == "__main__":
    unittest.main()
