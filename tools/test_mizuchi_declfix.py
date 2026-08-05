#!/usr/bin/env python3

import unittest

from mizuchi_declfix import CRT_SIGNATURES, decode_signature, fix_declarations


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

    def test_bails_on_struct_pointers(self):
        # Out of scope for the MVP: the honest mismatch stays visible.
        self.assertIsNone(decode_signature("?f@@YAXPAUSpot@@@Z"))


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
