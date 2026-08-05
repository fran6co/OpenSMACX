#!/usr/bin/env python3
"""Pin the symbol both objects have to carry.

Every expectation about what CL EMITS in here was measured against the real
VC6 under Wine before it was written down - `extern "C"` definitions in each
convention, compiled, and the external `.text` symbol read back out of the
COFF. `_f@0` for a zero-argument __stdcall and 12 rather than 8 for
`(double, int)` are both counter-intuitive enough to be worth pinning, and
neither was guessed.
"""

import unittest

import recovery_symbols as rs


class SpellingTest(unittest.TestCase):
    def test_a_mangled_name_is_left_to_the_cxx_compiler(self):
        found = rs.spelling("?on_key_down@NetWin@@QAEHH@Z")
        self.assertEqual("c++", found.linkage)
        self.assertEqual("", found.identifier)

    def test_a_disassembler_label_implies_no_convention(self):
        # `sub_5e3650` carries no decoration, so it says nothing about how the
        # function is called - the prototype has to.
        found = rs.spelling("sub_5e3650")
        self.assertEqual(("c", "sub_5e3650", "", -1), tuple(found))

    def test_a_stdcall_decoration_is_read_back_whole(self):
        found = rs.spelling("_WinMain@16")
        self.assertEqual(("c", "WinMain", rs.STDCALL, 16), tuple(found))

    def test_a_fastcall_decoration_is_read_back_whole(self):
        found = rs.spelling("@fast@8")
        self.assertEqual(("c", "fast", rs.FASTCALL, 8), tuple(found))

    def test_one_underscore_is_stripped_and_only_one(self):
        # zlib's `_tr_align` is catalogued `__tr_align`. Stripping greedily
        # would spell `tr_align` and emit a symbol one underscore short of
        # every reference to it.
        self.assertEqual("_tr_align", rs.spelling("__tr_align").identifier)
        self.assertEqual("build_tree", rs.spelling("_build_tree").identifier)

    def test_a_name_that_is_not_spellable_yields_no_identifier(self):
        for name in ("PopMenu::delete1", "j_??1Ambience@@QAE@XZ",
                     "_$I10_OUTPUT"):
            self.assertEqual("", rs.spelling(name).identifier, name)


class DecorationTest(unittest.TestCase):
    """Each expectation compiled under VC6 and read back out of the object."""

    def test_cdecl_takes_an_underscore_and_no_count(self):
        self.assertEqual("_f", rs.decorate("f", rs.CDECL, ["int", "int"]))

    def test_stdcall_counts_argument_bytes(self):
        self.assertEqual("_f@8", rs.decorate("f", rs.STDCALL, ["int", "int"]))

    def test_a_zero_argument_stdcall_still_carries_the_count(self):
        self.assertEqual("_f@0", rs.decorate("f", rs.STDCALL, []))

    def test_fastcall_swaps_the_underscore_for_an_at(self):
        self.assertEqual("@f@8", rs.decorate("f", rs.FASTCALL, ["int", "int"]))

    def test_a_double_occupies_two_slots(self):
        self.assertEqual("_f@12",
                         rs.decorate("f", rs.STDCALL, ["double", "int"]))

    def test_a_pointer_occupies_one_slot_whatever_it_points_at(self):
        self.assertEqual("_f@8",
                         rs.decorate("f", rs.STDCALL, ["char *", "int"]))
        self.assertEqual("_f@8",
                         rs.decorate("f", rs.STDCALL, ["double *", "int"]))


class SymbolForTest(unittest.TestCase):
    def test_a_mangled_name_passes_through_untouched(self):
        name = "?on_key_down@NetWin@@QAEHH@Z"
        self.assertEqual(name, rs.symbol_for(name, 0x483310, rs.THISCALL))

    def test_a_label_becomes_a_decorated_c_symbol(self):
        self.assertEqual(
            "_sub_401520@4",
            rs.symbol_for("sub_401520", 0x401520, rs.STDCALL, ["int"],
                          "sub_401520"))

    def test_an_unspellable_name_falls_back_to_its_address(self):
        self.assertEqual(
            "_fn_00401c80@4",
            rs.symbol_for("SessionStruct::SessionStruct", 0x401C80,
                          rs.STDCALL, ["int"]))

    def test_a_decorated_name_reproduces_itself(self):
        # The loop that makes the whole scheme safe: read the decoration off
        # the catalogued name, spell the source from it, decorate again, and
        # land on the string you started with.
        for name, convention, params in (("_WinMain@16", rs.STDCALL,
                                          ["int"] * 4),
                                         ("_build_tree", rs.CDECL, ["int"]),
                                         ("__tr_align", rs.CDECL, [])):
            found = rs.spelling(name)
            self.assertEqual(name, rs.symbol_for(name, 0, convention, params,
                                                 found.identifier), name)


class DisagreementTest(unittest.TestCase):
    def test_a_reproduced_decoration_is_not_a_disagreement(self):
        self.assertEqual("", rs.disagreement("_WinMain@16", "_WinMain@16"))

    def test_a_wrong_arity_against_a_real_decoration_is_reported(self):
        self.assertIn("_WinMain@16",
                      rs.disagreement("_WinMain@16", "_WinMain@8"))

    def test_a_label_can_never_disagree_with_itself(self):
        # `sub_401520` was never a symbol, so `_sub_401520@4` contradicts
        # nothing. Reporting it would bury the 4 real ones in 1,175 lines.
        self.assertEqual("", rs.disagreement("sub_401520", "_sub_401520@4"))

    def test_a_mangled_name_is_never_reported(self):
        self.assertEqual("", rs.disagreement("?f@@YAXXZ", "?f@@YAXXZ"))


class BackReferenceTest(unittest.TestCase):
    """CL never spells the same composite argument type twice.

    The catalogue does: its names come from a demangle/remangle round trip
    that expands the back-references, so 37 target objects held a name the
    compiler will never emit. Every expectation here was read off a real VC6
    object, not from the ABI document.
    """

    def test_a_repeated_pointer_becomes_its_index(self):
        self.assertEqual(
            "?on_adjust_pulldown_pos@AlphaMenu@@QAEXPAH0@Z",
            rs.compress_backrefs(
                "?on_adjust_pulldown_pos@AlphaMenu@@QAEXPAHPAH@Z"))

    def test_the_index_counts_slots_not_positions(self):
        # `PAD` takes slot 0 and `PAH` slot 1, so the repeat is `1`.
        self.assertEqual("?UNK2@Datalink@@QAEXPADPAH1@Z",
                         rs.compress_backrefs("?UNK2@Datalink@@QAEXPADPAHPAH@Z"))

    def test_a_user_defined_type_takes_a_slot_too(self):
        # And its own `@@` must not be mistaken for the end of the qualifier
        # chain, which is why the split is on the FIRST `@@`.
        self.assertEqual(
            "?f@@YAXPAUSprite@@0@Z",
            rs.compress_backrefs("?f@@YAXPAUSprite@@PAUSprite@@@Z"))

    def test_a_primitive_is_never_back_referenced(self):
        for name in ("?on_key_down@NetWin@@QAEHH@Z", "?POP@@YAXHHH@Z"):
            self.assertEqual(name, rs.compress_backrefs(name), name)

    def test_the_return_type_is_not_a_slot(self):
        # `?g@@YAPAHPAH@Z` returns `int *` and takes one; compressing the
        # argument against the RETURN type would emit `?g@@YAPAH0@Z`.
        self.assertEqual("?g@@YAPAHPAH@Z",
                         rs.compress_backrefs("?g@@YAPAHPAH@Z"))

    def test_an_already_compressed_name_is_left_alone(self):
        self.assertEqual("?help_create_link@@YAXPAD00@Z",
                         rs.compress_backrefs("?help_create_link@@YAXPAD00@Z"))

    def test_a_repeated_NAME_becomes_its_index_inside_the_type(self):
        # A second table, separate from the type one: the function's own name
        # is 0, its class 1, then each new type name. `GraphicWin` is already
        # 1 when it turns up as a parameter.
        self.assertEqual(
            "?update@GraphicWin@@QAEXHHHHPAU1@@Z",
            rs.compress_backrefs(
                "?update@GraphicWin@@QAEXHHHHPAUGraphicWin@@@Z"))

    def test_a_new_name_is_written_out_and_takes_the_next_slot(self):
        self.assertEqual(
            "?up2@GraphicWin@@QAEXPAURECT@@PAU1@@Z",
            rs.compress_backrefs(
                "?up2@GraphicWin@@QAEXPAURECT@@PAUGraphicWin@@@Z"))

    def test_the_type_table_wins_over_the_name_table(self):
        # The third argument repeats the FIRST whole type, so it is `0` rather
        # than a name back-reference into `Sprite`.
        self.assertEqual(
            "?f3@@YAXPAUSprite@@PAUGraphicWin@@0@Z",
            rs.compress_backrefs(
                "?f3@@YAXPAUSprite@@PAUGraphicWin@@PAUSprite@@@Z"))

    def test_two_type_slots_are_numbered_independently(self):
        self.assertEqual("?f4@@YAXPAD0PAH1@Z",
                         rs.compress_backrefs("?f4@@YAXPADPADPAHPAH@Z"))

    def test_anything_unparseable_is_returned_unchanged(self):
        for name in ("_sub_401520@4", "sub_401520", "?weird@@YA", ""):
            self.assertEqual(name, rs.compress_backrefs(name), name)


class SubstituteNameTest(unittest.TestCase):
    """14 catalogued names are not C identifiers and never can be.

    `??__Eg_BOOM_BUFFERS1@@YAXXZ` is a dynamic initialiser and `??__F...` an
    atexit thunk, both minted by the compiler for a global it constructs. The
    emitter substitutes `fn_<address>`; the target object kept the original,
    so the two had no name in common and none of them could pair.
    """

    def test_the_name_is_replaced_and_the_types_are_not(self):
        self.assertEqual(
            "?fn_004838b0@@YAXXZ",
            rs.substitute_name("??__Fg_NEWTECHWIN_SPRITES@@YAXXZ",
                               "fn_004838b0"))

    def test_a_class_qualifier_survives(self):
        self.assertEqual(
            "?m_00401000@Buffer@@UAEPAXI@Z",
            rs.substitute_name("??_Gthunk@Buffer@@UAEPAXI@Z", "m_00401000"))

    def test_a_name_that_did_not_change_is_left_alone(self):
        for name in ("?on_key_down@NetWin@@QAEHH@Z", "_sub_401520", ""):
            self.assertEqual(name, rs.substitute_name(name, "on_key_down"))


class UndecorateTest(unittest.TestCase):
    """The way back: Mizuchi hands these tools the symbol, not the label."""

    def test_each_decoration_is_reversed(self):
        self.assertEqual("sub_5e3650", rs.undecorate("_sub_5e3650"))
        self.assertEqual("sub_401520", rs.undecorate("_sub_401520@4"))
        self.assertEqual("fast", rs.undecorate("@fast@8"))

    def test_a_mangled_name_is_not_touched(self):
        self.assertEqual("?f@@YAXXZ", rs.undecorate("?f@@YAXXZ"))

    def test_a_synthesised_identifier_carries_its_address(self):
        self.assertEqual(0x401C80, rs.address_in("_fn_00401c80@4"))
        self.assertEqual(0x4483C0, rs.address_in("_m_004483c0"))

    def test_an_ordinary_symbol_carries_no_address(self):
        self.assertIsNone(rs.address_in("_sub_401520@4"))
        self.assertIsNone(rs.address_in("?f@@YAXXZ"))


class CompilerOwnedNameTest(unittest.TestCase):
    def test_the_intrinsics_that_cannot_be_defined_are_listed(self):
        # Measured: every name in the catalogue that raised C2169 under /O2.
        for name in ("strlen", "memcpy", "abs", "sin", "sqrt"):
            self.assertIn(name, rs.INTRINSIC)

    def test_atexit_is_declared_by_cl_itself(self):
        self.assertIn("atexit", rs.COMPILER_DECLARED)

    def test_the_two_undefinable_subjects_say_why(self):
        self.assertIn("C2733", rs.UNDEFINABLE["atexit"])
        self.assertIn("C2169", rs.UNDEFINABLE["_abnormal_termination"])


if __name__ == "__main__":
    unittest.main()
