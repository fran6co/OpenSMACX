#!/usr/bin/env python3
"""Unit tests for the Thinker header reducer.

One property matters more than the rest: an offset this tool reports must be
right or absent. A wrong offset is worse than no offset, because a member
renamed or declared at it looks like knowledge.
"""

import unittest

import correlate_thinker_layouts as reducer


def offsets(rows):
    return {row["field"]: row["offset"] for row in rows}


class UnreadableDeclarationTest(unittest.TestCase):
    """The regression that motivated this file."""

    def test_a_function_pointer_makes_later_offsets_unknown(self):
        # Thinker's own Buffer declares
        #     int (__cdecl *pfcnScrollText)(char *, int, int, int);
        # at 0x8. FIELD_RE cannot read it. Skipping the line - which is what
        # this tool used to do - reported every later member 4 bytes low, and
        # the struct's own names gave it away: the fields Thinker calls dwordC,
        # dword10 and dword14 came out at 0x8, 0xC and 0x10.
        text = "\n".join([
            "struct Buffer {",
            "    void* vtable;",
            "    Win* poOwner;",
            "    int (__cdecl *pfcnScrollText)(char* s, int x, int y, int n);",
            "    int dwordC;",
            "    int dword10;",
            "};",
        ])
        found = offsets(reducer.parse_struct_layouts(text, "engine_win.h"))
        self.assertEqual(found["vtable"], "0x0")
        self.assertEqual(found["poOwner"], "0x4")
        self.assertEqual(found["dwordC"], "")
        self.assertEqual(found["dword10"], "")

    def test_the_struct_size_is_withheld_too(self):
        text = "\n".join([
            "struct S {",
            "    int a;",
            "    int (*fn)(int);",
            "    int b;",
            "};",
        ])
        rows = reducer.parse_struct_layouts(text, "s.h")
        size_row = [row for row in rows if row["field"] == "(sizeof)"][0]
        self.assertEqual(size_row["offset"], "")

    def test_an_unsized_type_still_stops_the_accumulation(self):
        # The behaviour that already worked, kept under test beside the one
        # that did not.
        text = "\n".join([
            "struct S {",
            "    int a;",
            "    SomeClass b;",
            "    int c;",
            "};",
        ])
        found = offsets(reducer.parse_struct_layouts(text, "s.h"))
        self.assertEqual(found["a"], "0x0")
        self.assertEqual(found["c"], "")


class OrdinaryLayoutTest(unittest.TestCase):
    def test_primitives_accumulate(self):
        text = "\n".join([
            "struct S {",
            "    char a;",
            "    char b;",
            "    int16_t c;",
            "    int d;",
            "};",
        ])
        found = offsets(reducer.parse_struct_layouts(text, "s.h"))
        self.assertEqual([found[k] for k in "abcd"],
                         ["0x0", "0x1", "0x2", "0x4"])

    def test_arrays_multiply(self):
        text = "struct S {\n    int a[4];\n    int b;\n};"
        found = offsets(reducer.parse_struct_layouts(text, "s.h"))
        self.assertEqual(found["b"], "0x10")

    def test_pointers_are_four_bytes(self):
        text = "struct S {\n    char* a;\n    int b;\n};"
        found = offsets(reducer.parse_struct_layouts(text, "s.h"))
        self.assertEqual(found["b"], "0x4")

    def test_a_comment_line_is_not_a_member(self):
        text = "\n".join([
            "struct S {",
            "    int a;",
            "    // just a note;",
            "    int b;",
            "};",
        ])
        found = offsets(reducer.parse_struct_layouts(text, "s.h"))
        self.assertEqual(found["b"], "0x4")

    def test_a_blank_line_is_not_a_member(self):
        text = "struct S {\n    int a;\n\n    int b;\n};"
        found = offsets(reducer.parse_struct_layouts(text, "s.h"))
        self.assertEqual(found["b"], "0x4")


if __name__ == "__main__":
    unittest.main()
