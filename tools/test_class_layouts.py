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

    def test_multiple_bases_are_modelled_in_declaration_order(self):
        # MSVC lays non-virtual bases out in declaration order at increasing
        # offsets, which is ordinary concatenation. ??0BaseWin@@QAE@XZ builds
        # its GraphicWin on an unadjusted `this` and reaches its SubInterface
        # at 0xA14 == sizeof(GraphicWin), exactly where a second base goes.
        self.assertEqual([("GraphicWin", False), ("SubInterface", False)],
                         tool.bases_of(": GraphicWin, SubInterface"))
        self.assertEqual([("BaseButton", False)],
                         tool.bases_of(": public BaseButton"))
        self.assertEqual([], tool.bases_of(""))
        self.assertEqual([], tool.bases_of(None))

    def test_a_virtual_base_is_flagged_not_refused(self):
        # MSVC places a virtual base AFTER the derived members, behind a
        # vbtable pointer at offset 0 - a position this file can compute, so
        # it is modelled rather than refused. Console and MapWin declare
        # theirs and their pinned sizes hold under cl 12.00.8168.
        self.assertEqual([("Win", True)], tool.bases_of(": virtual Win"))
        self.assertEqual([("GraphicWin", False), ("Win", True)],
                         tool.bases_of(": GraphicWin, virtual Win"))

    def test_a_virtual_base_lands_after_the_derived_members(self):
        # The vbtable pointer opens the object and the base closes it. Getting
        # this order wrong would put every derived offset sizeof(base) too high
        # and still compile.
        layout = tool._with_bases("Console")
        self.assertIsNotNone(layout)
        self.assertEqual(("void *", "__vbptr", ""), layout[0])
        self.assertIn("Console", tool.verified_names())

    def test_an_inherited_virtual_base_is_hoisted_the_whole_way(self):
        # `Console : MapWin` and `MapWin : virtual GraphicWin`. A virtual base
        # belongs to the MOST DERIVED object, so in a Console the GraphicWin
        # closes the Console, not the MapWin subobject inside it - the ctor
        # builds it at this+0x23D94 (0x0050F4A0) and cl 12.00.8168 answers
        # `&((Console *)0)->field_21A6C_` with 0x21A6C.
        #
        # Hoisting one level only put GraphicWin's 0xA14 bytes in the MIDDLE
        # of a Console and every one of its 36 name-encoded members 0xA14 too
        # high. sizeof(Console) is 0x247A8 either way, which is why the size
        # assertion could not see it, and why this checks the ORDER: the own
        # members are the last entries before the trailing base, and there is
        # exactly one vbtable pointer for the two classes that share it.
        flattened, trailing = tool._layout_parts("Console")
        self.assertEqual(["GraphicWin"], list(trailing))
        self.assertEqual(1, sum(1 for member in flattened
                                if member[1] == "__vbptr"))
        start, stop = tool.declared_span("Console")
        self.assertEqual(len(flattened), stop)
        self.assertEqual("field_21A6C_", flattened[start][1])
        self.assertEqual("field_23D90_", flattened[stop - 1][1])

    def test_a_file_scope_function_pointer_typedef_is_four_bytes(self):
        # `Menu` was refused for holding a `MenuProc proc_` - a width that was
        # never in doubt; the extractor simply only knew the IN-CLASS spelling.
        self.assertIn("MenuProc", tool.function_pointer_typedefs())
        self.assertEqual([("void *", "proc_", "")],
                         tool.members_of("  MenuProc proc_;\n"))

    def test_a_pointer_to_member_typedef_is_not_four_bytes(self):
        # An MSVC pointer-to-member is 4 bytes only under single inheritance
        # and 8, 12 or 16 otherwise. This tree declares 114 of them for the
        # original-image seams; sizing one as four would move every offset
        # after it.
        self.assertNotIn("func_thiscall_teardown",
                         tool.function_pointer_typedefs())
        for name in tool.function_pointer_typedefs():
            self.assertNotIn("::", name)

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
