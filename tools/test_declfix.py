#!/usr/bin/env python3

import unittest

from declfix import CRT_SIGNATURES, decode_signature, fix_declarations


class DecodeSignatureTest(unittest.TestCase):
    def test_thiscall_method_with_unsigned_long(self):
        # K must come back as unsigned long: VC6 mangles unsigned int as I.
        self.assertEqual(("int", ["unsigned long"]),
                         decode_signature("?pid_2_idx@AlphaNet@@QAEHK@Z"))

    def test_void_method_taking_nothing(self):
        self.assertEqual(("void", []),
                         decode_signature("?close@StringStruct@@QAEXXZ"))

    def test_free_cdecl_function_with_pointer_param(self):
        # D is plain char (measured: f_char(char*) mangles to PAD).
        self.assertEqual(("void", ["int", "int", "char *"]),
                         decode_signature("?message_chat@@YAXHHPAD@Z"))

    def test_pointer_to_pointer(self):
        self.assertEqual(("void", ["char * *"]),
                         decode_signature("?f@@YAXPAPAD@Z"))

    def test_unsigned_char_pointer(self):
        self.assertEqual(("void", ["unsigned char *"]),
                         decode_signature("?f@@YAXPAE@Z"))

    def test_rejects_c_linkage_names(self):
        self.assertIsNone(decode_signature("_free"))

    def test_decodes_a_struct_pointer(self):
        # Used to be out of scope, and the emitter then fell back to the
        # prototype's spelling of the type - which is where `player_data` was
        # emitted for `PlayerData` and the symbol stopped matching.
        self.assertEqual(("void", ["Spot *"]),
                         decode_signature("?f@@YAXPAUSpot@@@Z"))

    def test_the_qualifier_chain_closes_at_the_first_double_at(self):
        # `PAUSprite@@` carries a `@@` of its own, so splitting on the LAST
        # one read the argument list as the class qualifier and returned None
        # for every signature with a struct parameter.
        self.assertEqual(("void", ["Sprite *", "GraphicWin *", "int"]),
                         decode_signature(
                             "?draw@@YAXPAUSprite@@PAUGraphicWin@@H@Z"))

    def test_a_class_key_decodes_the_same_as_a_struct_key(self):
        # The emitted unit declares every one of them `struct`.
        self.assertEqual(decode_signature("?f@@YAXPAUSpot@@@Z"),
                         decode_signature("?f@@YAXPAVSpot@@@Z"))

    def test_const_survives_the_pointer(self):
        # 25 rows are string arguments the catalogue spells `PBD`; dropping
        # the `B` emitted `char *` where the target holds `const char *`.
        self.assertEqual(("int", ["const char *"]),
                         decode_signature("?f@@YAHPBD@Z"))

    def test_a_const_pointer_is_not_a_pointer_to_const(self):
        # Measured against VC6: `Palette *const` gives `QAV1@` and
        # `const Palette *` gives `PBV1@`. Decoding `Q` as a plain pointer
        # emitted `PAV1@`, which pairs with neither.
        self.assertEqual(("int", ["Palette *const"]),
                         decode_signature("?__as@Palette@@QAEHQAUPalette@@@Z"))

    def test_a_function_pointer_parameter_decodes(self):
        # Giving up on this returned None for the WHOLE signature, so every
        # OTHER argument kept the prototype's spelling - `char *` where the
        # name says `PBD`, which is a different symbol. 15 rows.
        self.assertEqual(
            ("int", ["const char *", "const char *", "int (__cdecl *)()"]),
            decode_signature("?popp@@YAHPBDPBDP6AHXZ@Z"))

    def test_a_function_pointer_keeps_its_own_convention_and_arguments(self):
        self.assertEqual(
            ("void", ["void (__cdecl *)(char *)", "int"]),
            decode_signature("?tech_heck@@YAXP6AXPAD@ZH@Z"))
        self.assertEqual(
            ("void", ["int (__stdcall *)(char *, char *)", "int"]),
            decode_signature("?f@@YAXP6GHPAD0@ZH@Z"))

    def test_a_back_reference_repeats_the_earlier_argument(self):
        self.assertEqual(("int", ["const char *", "const char *", "int"]),
                         decode_signature("?POP2@@YAHPBD0H@Z"))

    def test_the_return_type_takes_no_back_reference_slot(self):
        # Slot 0 is the first ARGUMENT. Counting the return type would make
        # `0` resolve to the wrong type in every signature that returns one.
        self.assertEqual(("int *", ["char *", "char *"]),
                         decode_signature("?f@@YAPAHPAD0@Z"))

    def test_a_back_reference_with_no_slot_is_refused(self):
        self.assertIsNone(decode_signature("?f@@YAXH0@Z"))


class FixDeclarationsTest(unittest.TestCase):
    def test_rewrites_method_parameter_spelling(self):
        text = "class AlphaNet { public:\n    int pid_2_idx(unsigned int);\n};"
        fixed = fix_declarations(
            text, [{"name": "?pid_2_idx@AlphaNet@@QAEHK@Z"}])
        self.assertIn("int pid_2_idx(unsigned long);", fixed)

    def test_rewrites_free_function_and_preserves_indent(self):
        text = "void message_chat(int, int, int8*);"
        fixed = fix_declarations(text, [{"name": "?message_chat@@YAXHHPAD@Z"}])
        self.assertEqual("void message_chat(int, int, char *);", fixed)

    def test_arity_mismatch_leaves_the_declaration_alone(self):
        text = "int pid_2_idx(unsigned int, int);"
        fixed = fix_declarations(
            text, [{"name": "?pid_2_idx@AlphaNet@@QAEHK@Z"}])
        self.assertEqual(text, fixed)

    def test_c_linkage_crt_callee_gets_extern_c_and_signature(self):
        text = "int __cdecl _free();"
        fixed = fix_declarations(text, [{"name": "_free"}])
        self.assertEqual(f'extern "C" {CRT_SIGNATURES["_free"]};', fixed)

    def test_c_linkage_unknown_callee_keeps_shape_but_fixes_linkage(self):
        text = "int __cdecl mystery(int);"
        fixed = fix_declarations(text, [{"name": "mystery"}])
        self.assertEqual('extern "C" int __cdecl mystery(int);', fixed)

    def test_nameless_row_is_skipped(self):
        text = "int pid_2_idx(unsigned int);"
        self.assertEqual(text, fix_declarations(text, [{"name": ""}]))


class DuplicateMemberTest(unittest.TestCase):
    """Rewriting is per-callee, so two overloads can land on the same text.

    C++ calls that `error C2535: member function already defined or declared`
    and refuses the whole unit. It surfaced when the emitter began decoding
    types out of mangled names: `?do_menu@Popup@@...PAD...` and its `PBD`
    sibling both became `(char *, int, int)`.
    """

    def test_an_exact_twin_inside_a_struct_is_dropped(self):
        text = ("struct Popup {\n"
                "    int do_menu(char *, int, int);\n"
                "    int do_menu(char *, int, int);\n"
                "};")
        self.assertEqual(
            "struct Popup {\n    int do_menu(char *, int, int);\n};",
            fix_declarations(text, []))

    def test_a_class_body_is_deduplicated_as_well_as_a_struct(self):
        # A type with methods is emitted `class X { public:`, which matches
        # neither the `struct ` prefix nor the trailing `{` this used to look
        # for - so the C2535 it exists to prevent came back for every one of
        # the 152 classes.
        text = ("class Popup { public:\n"
                "    int do_menu(char *, int, int);\n"
                "    int do_menu(char *, int, int);\n"
                "};")
        self.assertEqual(
            "class Popup { public:\n    int do_menu(char *, int, int);\n};",
            fix_declarations(text, []))

    def test_a_real_overload_set_survives(self):
        text = ("struct Popup {\n"
                "    int do_menu(char *, int, int);\n"
                "    int do_menu(int);\n"
                "};")
        self.assertEqual(text, fix_declarations(text, []))

    def test_two_structs_do_not_share_a_seen_set(self):
        # The same member name in two classes is not a duplicate.
        text = ("struct A {\n    void close();\n};\n"
                "struct B {\n    void close();\n};")
        self.assertEqual(text, fix_declarations(text, []))

    def test_declarations_outside_any_struct_are_untouched(self):
        text = "int f(int);\nint f(int);"
        self.assertEqual(text, fix_declarations(text, []))


if __name__ == "__main__":
    unittest.main()
