#!/usr/bin/env python3
"""Pin the layout extraction, especially where it must REFUSE.

A layout that is wrong is worse than no layout: the offsets move, and a
recovered body indexing through the wrong one still compiles. It fails later
as a byte mismatch nobody traces back here. So the tests that matter are the
ones about refusing.
"""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import class_layouts as tool  # noqa: E402


class MembersTest(unittest.TestCase):
    def test_plain_members_are_read_in_order(self):
        body = ("\n public:\n  void method(int);\n private:\n"
                "  uint32_t first_;\n  Entry *second_;\n  int third_;\n")
        self.assertEqual(
            [("uint32_t", "first_", ""), ("Entry *", "second_", ""),
             ("int", "third_", "")],
            tool.members_of(body))

    def test_the_star_binds_to_the_name_in_this_tree(self):
        # `Entry *head_`, not `Entry* head_`. Matching only the second
        # spelling found nothing at all and reported 11 classes instead of 25.
        self.assertEqual([("Entry *", "head_", "")],
                         tool.members_of("  Entry *head_;\n"))

    def test_an_array_keeps_its_bound(self):
        self.assertEqual([("int", "grid_", "[4]")],
                         tool.members_of("  int grid_[4];\n"))

    def test_every_array_dimension_is_kept(self):
        # A single `\[...\]?` left `Sprite base[6][4];` unparsed, and an
        # unparsed statement refuses the WHOLE class: FactionArt, PlayerData,
        # Trace and VOX_Matrix were all refused on one regex.
        self.assertEqual([("float", "values", "[3][3]")],
                         tool.members_of("  float values[3][3];\n"))
        self.assertEqual([("char", "saved_queue_name", "[8][24]")],
                         tool.members_of("  char saved_queue_name[8][24];\n"))

    def test_a_virtual_refuses_the_whole_class(self):
        # A vtable pointer this cannot see sits at offset 0 and moves every
        # member after it.
        self.assertIsNone(tool.members_of("  virtual void f();\n  int a_;\n"))

    def test_a_union_refuses_the_whole_class(self):
        self.assertIsNone(tool.members_of("  union { int a; };\n"))

    def test_anything_unrecognised_refuses(self):
        self.assertIsNone(tool.members_of("  int a_ : 3;\n"))

    def test_a_function_pointer_member_is_counted_as_a_pointer(self):
        # It IS storage. What must never happen is it being SKIPPED - it
        # contains a `(`, so the method test would drop it and every offset
        # after it would move, and sizeof only catches that when padding does
        # not absorb the four bytes.
        #
        # It used to refuse the whole class instead. That was safe and became
        # expensive: `Time` is pinned at 0x28 and holds two of these, so it
        # was refused outright, and with it BaseButton, FlatButton, Scroll,
        # MainInterface and everything downstream. Four bytes is four bytes.
        # `void *` is the spelling because the emitter writes members as
        # `<type> <name>;` and a function pointer does not fit that shape.
        self.assertEqual([("void *", "callback", "")],
                         tool.members_of("  void (*callback)(int);\n"))

    def test_a_function_pointer_does_not_displace_the_member_after_it(self):
        # The property the old refusal was protecting, stated directly.
        self.assertEqual(
            [("int", "a_", ""), ("void *", "cb_", ""), ("int", "b_", "")],
            tool.members_of("  int a_;\n  void (__cdecl *cb_)(int);\n"
                            "  int b_;\n"))

    def test_a_nested_typedef_of_a_scalar_is_resolved(self):
        # `typedef int32_t Dib;` inside Buffer, then `Dib dib_[256];`. Before
        # this the alias was an unknown type and Buffer refused - taking
        # GraphicWin and the eight classes holding one by value with it.
        self.assertEqual(
            [("int32_t", "dib_", "[256]")],
            tool.members_of("  typedef int32_t Dib;\n  Dib dib_[256];\n"))

    def test_a_nested_typedef_of_an_unknown_type_still_refuses(self):
        # Only aliases of types this file already knows are resolved. An alias
        # for something unknown is still unknown, and guessing its width is
        # the one failure mode that silently moves every later offset.
        self.assertIsNone(
            tool.members_of("  typedef Whatever Alias;\n  Alias member_;\n"))

    def test_a_win32_struct_may_be_held_by_value(self):
        # RECT is sixteen bytes of four LONGs and cannot be spelled as a
        # primitive, so the emitter defines it instead. Holding one refused
        # Win, Buffer, Scroll and MainInterface before that.
        self.assertEqual([("RECT", "outer_rect_", "")],
                         tool.members_of("  RECT outer_rect_;\n"))
        self.assertEqual(
            ["    long left;", "    long top;",
             "    long right;", "    long bottom;"],
            tool.declaration_for("RECT"))

    def test_a_method_taking_a_function_pointer_still_parses(self):
        # The difference is the identifier before the first paren.
        self.assertEqual(
            [("int", "a_", "")],
            tool.members_of("  void init(void (*cb)(int));\n  int a_;\n"))

    def test_methods_and_access_labels_are_not_members(self):
        self.assertEqual([], tool.members_of(" public:\n  void f(int);\n"))


class VerificationGateTest(unittest.TestCase):
    """Only a PROVED layout reaches an agent."""

    def test_an_unverified_class_supplies_nothing(self):
        self.assertEqual([], tool.declaration_for("NotAClassAnywhere"))

    def test_the_verified_list_is_a_subset_of_what_is_extracted(self):
        extracted = set(tool.pinned_layouts())
        self.assertTrue(tool.verified_names() <= extracted,
                        "the verified list names a class no longer extracted; "
                        "regenerate it with tools/verify_class_layouts.py")

    def test_every_verified_class_supplies_members(self):
        for name in tool.verified_names():
            self.assertTrue(tool.declaration_for(name), name)

    def test_a_scalar_is_never_forward_declared(self):
        # `struct uint32_t;` is `error C2371: redefinition; different basic
        # types`, and it took out every unit that supplied a layout.
        for name in tool.verified_names():
            self.assertFalse(tool.referenced_types(name) & tool.SCALAR, name)


class WindowsTypeAgreementTest(unittest.TestCase):
    """The two hand-synced Win32 lists must name the same types.

    `class_layouts.WINDOWS_TYPEDEF` decides whether a by-value member is
    ALLOWED; `emit_translation_unit.NOT_A_STRUCT` decides how the emitter
    SPELLS it. A name in the first and not the second is permitted into a
    layout and then forward-declared as a struct, which is
    `C2079: uses undefined struct` on every unit that holds one.

    That is not hypothetical. HDC was missing from the second list for as long
    as Buffer was unreadable and nothing noticed, because no unit ever held an
    `HDC` by value. The moment Buffer became extractable it cost 72 byte-exact
    bodies in a single census.
    """

    def test_every_allowed_typedef_has_a_spelling(self):
        import emit_translation_unit
        missing = sorted(set(tool.WINDOWS_TYPEDEF)
                         - set(emit_translation_unit.NOT_A_STRUCT))
        self.assertEqual([], missing,
                         "these are allowed by value but the emitter has no "
                         "spelling for them, so it will forward-declare them "
                         "as structs")


if __name__ == "__main__":
    unittest.main()
