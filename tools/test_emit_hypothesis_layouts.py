#!/usr/bin/env python3
"""Unit tests for the hypothesis-layout header generator.

The properties tested here are the ones whose failure is SILENT: a member
emitted at the wrong width still compiles, a by-value member still compiles
until something else includes it, and a class that quietly acquires a size
assertion would be believed by everything downstream.
"""

import unittest

import class_layouts
import emit_hypothesis_layouts as emitter


class StorageTest(unittest.TestCase):
    """Every member holds exactly its own size and never another class."""

    def test_scalar_widths(self):
        self.assertEqual(emitter.storage(1, "a_"), "uint8_t a_;")
        self.assertEqual(emitter.storage(2, "a_"), "uint16_t a_;")
        self.assertEqual(emitter.storage(4, "a_"), "uint32_t a_;")

    def test_anything_else_is_a_byte_array_of_that_size(self):
        # 0xa14 is a GraphicWin. Emitting it AS a GraphicWin would make these
        # classes depend on each other's definitions and on emission order;
        # by-value layout members once broke 675 units.
        self.assertEqual(emitter.storage(0xA14, "graphicWin_"),
                         "uint8_t graphicWin_[0xA14];")

    def test_odd_sizes_survive(self):
        self.assertEqual(emitter.storage(3, "a_"), "uint8_t a_[0x3];")
        self.assertEqual(emitter.storage(8, "a_"), "uint8_t a_[0x8];")


class MemberNameTest(unittest.TestCase):
    def test_a_real_name_is_kept_and_given_the_trailing_underscore(self):
        self.assertEqual(emitter.member_name("lineHeight", 0xC, set()),
                         "lineHeight_")

    def test_a_placeholder_is_rewritten_to_carry_its_own_offset(self):
        # src/ spells this field_9E8_, where the hex IS the offset. Carrying
        # the source's `field_9E8` across would put two conventions in one
        # tree for the same thing.
        self.assertEqual(emitter.member_name("field_1", 0x9E8, set()),
                         "field_9E8_")
        self.assertEqual(emitter.member_name("", 0x10, set()), "field_10_")
        self.assertEqual(emitter.member_name("unk3", 0x4, set()), "field_4_")

    def test_a_keyword_is_not_emitted_as_an_identifier(self):
        self.assertEqual(emitter.member_name("class", 0x8, set()), "field_8_")

    def test_a_repeated_name_is_made_unique(self):
        taken = set()
        first = emitter.member_name("count", 0x0, taken)
        second = emitter.member_name("count", 0x4, taken)
        self.assertNotEqual(first, second)


class LayoutTest(unittest.TestCase):
    def test_thinker_gaps_become_padding(self):
        """Thinker declares only the prefix it needs, so the space between two
        offsets it names is storage nobody described - and leaving it out
        would move every member after the gap."""
        thinker = {"S": {0x0: ("a", 4), 0x10: ("b", 4)}}
        members, provenance = emitter.layout_for("S", {}, thinker)
        self.assertEqual(provenance, "Thinker")
        self.assertEqual([(offset, size) for offset, _, size in members],
                         [(0x0, 4), (0x4, 0xC), (0x10, 4)])

    def test_thinker_names_an_offset_the_idb_left_blank(self):
        idb = {"S": [(0x0, "field_0", 4), (0x4, "field_4", 4)]}
        thinker = {"S": {0x4: ("mineralSurplus", 4)}}
        members, provenance = emitter.layout_for("S", idb, thinker)
        self.assertEqual([member for _, member, _ in members],
                         ["field_0", "mineralSurplus"])
        self.assertIn("Thinker", provenance)

    def test_thinker_does_not_overwrite_a_name_the_idb_already_has(self):
        idb = {"S": [(0x0, "lineHeight", 4)]}
        thinker = {"S": {0x0: ("something_else", 4)}}
        members, _ = emitter.layout_for("S", idb, thinker)
        self.assertEqual([member for _, member, _ in members], ["lineHeight"])


class RenderTest(unittest.TestCase):
    def test_no_size_assertion_is_ever_emitted(self):
        """The one property that must not regress. A static_assert here would
        be believed by class_layouts.pinned_layouts(), reach an agent as a
        PROVED layout, and could enter verified-layouts.txt by verifying
        against itself."""
        text = emitter.render(
            ["S"], {"S": [(0x0, "a", 4)]}, {}, emitter.collections.Counter())
        # The banner EXPLAINS why there is no assertion, so searching the whole
        # file finds the explanation and passes for the wrong reason. What
        # matters is the declarations, and `SIZE_ASSERT` is the very regex
        # class_layouts.pinned_layouts() uses to decide what to believe.
        declarations = text.split('#include "stdafx.h"', 1)[1]
        self.assertNotIn("static_assert", declarations)
        self.assertEqual(class_layouts.SIZE_ASSERT.findall(text), [])

    def test_a_class_owning_functions_is_a_class_and_data_is_a_struct(self):
        owned = emitter.collections.Counter({"HasMethods": 3})
        idb = {"HasMethods": [(0x0, "a", 4)], "JustData": [(0x0, "a", 4)]}
        text = emitter.render(["HasMethods", "JustData"], idb, {}, owned)
        self.assertIn("class HasMethods {", text)
        self.assertIn("struct JustData {", text)

    def test_offsets_are_written_beside_every_member(self):
        text = emitter.render(
            ["S"], {"S": [(0x0, "a", 4), (0x4, "b", 0x10)]}, {},
            emitter.collections.Counter())
        self.assertIn("// 0x0", text)
        self.assertIn("// 0x4", text)


if __name__ == "__main__":
    unittest.main()
