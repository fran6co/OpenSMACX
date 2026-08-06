#!/usr/bin/env python3
"""Unit tests for size promotion.

The compiler decides which sizes get promoted, and that part needs wine. What
is tested here is the text surgery: where the assertion lands, and that it
lands in the shape everything else in this tree scrapes for.
"""

import re
import unittest

import class_layouts
import promote_agreed_sizes as promoter


class InsertTest(unittest.TestCase):
    def test_the_assertion_follows_the_class(self):
        text = "class Buffer {\n  uint32_t a_;\n};\n"
        out = promoter.insert(text, "Buffer", 0x588)
        self.assertLess(out.index("};"), out.index("static_assert"))
        self.assertIn("sizeof(Buffer) == 0x588", out)

    def test_the_shape_is_the_one_pinned_layouts_reads(self):
        """`class_layouts.pinned_layouts()` decides what an agent may rely on
        by scraping this exact spelling. An assertion it cannot parse pins
        nothing while looking like it does."""
        out = promoter.insert("class S {\n};\n", "S", 0x40)
        self.assertEqual(class_layouts.SIZE_ASSERT.findall(out),
                         [("S", "0x40")])

    def test_the_right_class_is_chosen(self):
        text = "class A {\n};\nclass B {\n};\n"
        out = promoter.insert(text, "B", 0x10)
        before = out.split("class B {", 1)[0]
        self.assertNotIn("static_assert", before)

    def test_a_nested_brace_does_not_end_the_class(self):
        text = ("class A {\n"
                "  void m() { if (x) { y(); } }\n"
                "  uint32_t a_;\n"
                "};\n")
        out = promoter.insert(text, "A", 0x4)
        self.assertLess(out.index("uint32_t a_;"), out.index("static_assert"))

    def test_an_unknown_class_leaves_the_text_alone(self):
        text = "class A {\n};\n"
        self.assertEqual(promoter.insert(text, "Missing", 0x4), text)

    def test_sizes_are_written_as_hex(self):
        out = promoter.insert("class A {\n};\n", "A", 0x1B394)
        self.assertIn("0x1B394", out)
        self.assertNotIn("111508", out)


if __name__ == "__main__":
    unittest.main()
