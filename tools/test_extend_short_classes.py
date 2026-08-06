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


if __name__ == "__main__":
    unittest.main()
