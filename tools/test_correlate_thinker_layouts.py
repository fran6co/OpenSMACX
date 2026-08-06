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


class DerivedStructTest(unittest.TestCase):
    """A base list used to make the whole struct invisible.

    `STRUCT_RE` required `{` immediately after the name, so all 16 of
    Thinker's derived structs were skipped whole - 1,070 member lines, and
    they are the inheritance spine: GraphicWin : Win, BasePop : GraphicWin,
    BaseWindow : GraphicWin.
    """

    def test_a_derived_struct_is_read_at_all(self):
        text = "struct BasePop : GraphicWin {\n    int a;\n};"
        rows = reducer.parse_struct_layouts(text, "h", {"GraphicWin": 0xA14})
        self.assertEqual([row["field"] for row in rows], ["a", "(sizeof)"])

    def test_members_start_after_the_base_subobject(self):
        """Accumulating from zero would put every BasePop member 0xa14 bytes
        low. Thinker's own first member is literally called `field_A14`."""
        text = "struct BasePop : GraphicWin {\n    int field_A14;\n};"
        rows = reducer.parse_struct_layouts(text, "h", {"GraphicWin": 0xA14})
        self.assertEqual(offsets(rows)["field_A14"], "0xA14")

    def test_an_unpinned_base_withholds_offsets_but_keeps_names(self):
        text = "struct AlphaNet : Net {\n    int iSessions;\n};"
        rows = reducer.parse_struct_layouts(text, "h", {})
        self.assertEqual(offsets(rows)["iSessions"], "")
        self.assertIn("iSessions", offsets(rows))

    def test_access_specifiers_are_accepted(self):
        text = "struct D : public B {\n    int a;\n};"
        rows = reducer.parse_struct_layouts(text, "h", {"B": 0x10})
        self.assertEqual(offsets(rows)["a"], "0x10")

    def test_a_qualified_template_name_is_not_a_struct(self):
        # `struct std::hash<Point> {` must not be read as a struct called
        # `std` deriving from `hash<Point>`.
        text = "struct std::hash<Point> {\n    int a;\n};"
        self.assertEqual(reducer.parse_struct_layouts(text, "h", {}), [])


class SelfNamedControlTest(unittest.TestCase):
    """Thinker names an unknown member after its own absolute offset, so it
    states the answer this parser computes. Disagreement means the
    accumulator is wrong."""

    def test_a_disagreeing_self_named_field_stops_the_offsets(self):
        text = "struct S {\n    int a;\n    int field_99;\n    int b;\n};"
        found = offsets(reducer.parse_struct_layouts(text, "h", {}))
        self.assertEqual(found["a"], "0x0")
        self.assertEqual(found["field_99"], "")
        self.assertEqual(found["b"], "")

    def test_an_agreeing_self_named_field_passes_through(self):
        text = "struct S {\n    int a;\n    int field_4;\n    int b;\n};"
        found = offsets(reducer.parse_struct_layouts(text, "h", {}))
        self.assertEqual([found[k] for k in ("a", "field_4", "b")],
                         ["0x0", "0x4", "0x8"])


class KnownSizedMemberTest(unittest.TestCase):
    """One unreadable member stops a whole struct, and a member whose type is
    a class the tree has PROVED the size of is as sizeable as a primitive.
    GraphicWin opens `Buffer oCanvas;` and stopped after one field."""

    def test_a_pinned_class_member_carries_the_accumulator(self):
        text = "struct GraphicWin : Win {\n    Buffer oCanvas;\n    int a;\n};"
        found = offsets(reducer.parse_struct_layouts(
            text, "h", {"Win": 0x444, "Buffer": 0x588}))
        # 0x444 + 0x588 == 0x9CC, which is what Thinker calls its next field.
        self.assertEqual(found["a"], "0x9CC")

    def test_an_unpinned_class_member_still_abstains(self):
        text = "struct S {\n    Mystery m;\n    int a;\n};"
        self.assertEqual(offsets(reducer.parse_struct_layouts(text, "h", {}))["a"], "")

    def test_win32_typedefs_are_sized_by_the_platform_abi(self):
        text = ("struct S {\n    BYTE b;\n    WORD w;\n    RECT r;\n"
                "    LPSTR p;\n    int a;\n};")
        found = offsets(reducer.parse_struct_layouts(text, "h", {}))
        # 1 + 2 -> aligned by the packed walk: 0, 1, 3, 0x13, 0x17
        self.assertEqual(found["a"], "0x17")


class NameCollisionTest(unittest.TestCase):
    """Thinker names its own types. Where a name means a different type, every
    offset it carries describes a different object."""

    def test_a_contradicted_size_drops_the_whole_struct(self):
        # Thinker's `Spot` is RECT+int+int = 0x18, which src/ calls
        # Spot::SpotInternal; what src/ calls Spot is Thinker's SpotList.
        rows = [{"struct": "Spot", "field": "rect", "offset": "0x0"},
                {"struct": "Spot", "field": "(sizeof)", "offset": "0x18"},
                {"struct": "Heap", "field": "(sizeof)", "offset": "0x14"}]
        kept, collided = reducer.drop_name_collisions(
            rows, {"Spot": 0xC, "Heap": 0x14})
        self.assertEqual(collided, ["Spot"])
        self.assertEqual({row["struct"] for row in kept}, {"Heap"})

    def test_an_agreeing_size_is_kept(self):
        rows = [{"struct": "Heap", "field": "(sizeof)", "offset": "0x14"}]
        kept, collided = reducer.drop_name_collisions(rows, {"Heap": 0x14})
        self.assertEqual((len(kept), collided), (1, []))

    def test_a_struct_src_does_not_pin_is_not_judged(self):
        rows = [{"struct": "Unknown", "field": "(sizeof)", "offset": "0x40"}]
        kept, collided = reducer.drop_name_collisions(rows, {})
        self.assertEqual((len(kept), collided), (1, []))


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
