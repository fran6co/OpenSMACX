#!/usr/bin/env python3
"""Offline tests for tools/src_declarations.py.

The module feeds the scaffolding preamble every recovered body is measured in,
so its failure modes are the ones that matter here:

  * emitting a WRONG declaration would change what is being measured, which is
    worse than emitting none - the census would report a tier for a unit that
    is not the recovery;
  * emitting a declaration for a name the scaffolding already carries would
    turn a unit that compiles today into a redefinition error;
  * missing the file-scope helpers a `.cpp` keeps in an anonymous namespace
    would leave the whole population it exists for untouched.

No VC6 and no Wine: every case is a string in, a string out.
"""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import src_declarations as sd  # noqa: E402


def index_of(text: str) -> dict:
    """The index for one synthetic file, without touching src/."""
    out, rejected = {}, set()
    sd._index_text(text, "synthetic.h", out, rejected)
    for name in rejected:
        out.pop(name, None)
    return out


class Statements(unittest.TestCase):

    def test_class_body_is_opaque(self):
        found = index_of("class Foo { int member_; void method(); };")
        self.assertIn("Foo", found)
        self.assertNotIn("member_", found)
        self.assertNotIn("method", found)

    def test_function_body_is_opaque(self):
        found = index_of("int helper(int a) { int local = 0; return local; }")
        self.assertIn("helper", found)
        self.assertNotIn("local", found)

    def test_anonymous_namespace_is_transparent(self):
        # The reason this module found 26 names in src/sound.cpp and none of
        # the four its bodies call: the helpers live in `namespace { }`.
        found = index_of("namespace {\nint forward(int a, int b) { return a; }\n}\n")
        self.assertIn("forward", found)
        self.assertEqual(found["forward"].text, "int forward(int a, int b);")

    def test_named_namespace_is_transparent(self):
        found = index_of("namespace detail {\nint inner(void) { return 0; }\n}\n")
        self.assertIn("inner", found)

    def test_extern_c_block_is_transparent(self):
        found = index_of('extern "C" {\nint exported(int a);\n}\n')
        self.assertIn("exported", found)


class Declarations(unittest.TestCase):

    def test_function_definition_becomes_a_prototype(self):
        found = index_of("Buffer *make(int a, char *b) { return 0; }")
        self.assertEqual(found["make"].text, "Buffer * make(int a, char *b);")
        self.assertEqual(found["make"].kind, "function")

    def test_static_is_dropped_so_no_definition_is_promised(self):
        # `static int f(int);` with no body is a promise the unit cannot keep;
        # the census compiles with /c and never links, but VC6 still refuses.
        found = index_of("static int quiet(int a) { return a; }")
        self.assertEqual(found["quiet"].text, "int quiet(int a);")

    def test_variable_definition_becomes_extern(self):
        found = index_of("Veh *Vehs;")
        self.assertEqual(found["Vehs"].text, "extern Veh * Vehs;")

    def test_array_variable_keeps_its_bound(self):
        found = index_of("int Table[16];")
        self.assertEqual(found["Table"].text, "extern int Table[16];")

    def test_function_pointer_typedef_is_verbatim(self):
        found = index_of("typedef void (__cdecl *MenuProc)(int);")
        self.assertEqual(found["MenuProc"].kind, "typedef")
        self.assertEqual(found["MenuProc"].text,
                         "typedef void (__cdecl *MenuProc)(int);")

    def test_member_pointer_typedef_is_verbatim(self):
        found = index_of("typedef int (OriginalObject::*sound_arg)(int a1);")
        self.assertEqual(found["sound_arg"].kind, "typedef")

    def test_class_offers_only_a_forward_declaration(self):
        # The DEFINITION is the emitter's business and its layout is gated;
        # handing one out here would route around that gate.
        found = index_of("class DLLEXPORT Sound { int volume_; };")
        self.assertEqual(found["Sound"].text, "class Sound;")

    def test_method_definition_out_of_line_is_not_a_free_function(self):
        found = index_of("void Sound::fade(unsigned long a1) { ; }")
        self.assertEqual(found, {})

    def test_control_flow_is_not_a_declaration(self):
        self.assertEqual(index_of("if (x) { y(); }"), {})

    def test_conflicting_declarations_are_refused(self):
        found = index_of("int both(int a);\nchar *both(char *b);\n")
        self.assertNotIn("both", found)

    def test_forward_declaration_does_not_beat_the_definition(self):
        found = index_of("class Foo;\nclass Foo { int a_; };\n")
        self.assertEqual(found["Foo"].text, "class Foo;")


class ParameterTypes(unittest.TestCase):

    def test_parameter_name_is_not_a_type(self):
        # Reading `vtable_offset` as a type refused every named-parameter
        # declaration in the tree, because no such type is declared anywhere.
        self.assertEqual(sd._param_type("int vtable_offset"), "int")
        self.assertEqual(sd._param_type("Sound *self"), "Sound *")
        self.assertEqual(sd._param_type("int"), "int")
        self.assertEqual(sd._param_type("char *"), "char *")

    def test_by_value_class_parameter_is_reported(self):
        self.assertEqual(sd._by_value_types("Heap heap, int n"), {"Heap"})

    def test_pointer_parameter_is_not_by_value(self):
        self.assertEqual(sd._by_value_types("Heap *heap"), set())


class MemberTypes(unittest.TestCase):

    def test_pointer_member_type_is_reported_separately(self):
        by_value, pointed = sd._member_types([
            "    Spot spot_;",
            "    const BITMAPINFO *bitmap_info_;",
            "    Win *children_[150];",
            "    void *opaque_;",
            "    uint32_t plain_;",
        ])
        self.assertEqual(by_value, {"Spot"})
        self.assertEqual(pointed, {"BITMAPINFO", "Win"})

    def test_a_pointer_still_needs_the_name_declared(self):
        # `??1TextureStore@@QAE@XZ` was byte-exact and became NO_COMPILE the
        # first time definitions were emitted, on exactly this: `Buffer` holds
        # `const BITMAPINFO *`, nothing asked for BITMAPINFO, and VC6 stopped
        # at `syntax error : missing ';' before '*'`.
        _, pointed = sd._member_types(["    const BITMAPINFO *bitmap_info_;"])
        self.assertIn("BITMAPINFO", pointed)


class ForBody(unittest.TestCase):

    def setUp(self):
        self.tree = tempfile.TemporaryDirectory()
        self.src = Path(self.tree.name)
        (self.src / "helpers.h").write_text(
            "typedef void (__cdecl *Hook)(int);\n"
            "class Widget { int a_; };\n"
            "int compute(int a, int b);\n"
            "Widget *TheWidget;\n"
            "Hook InstalledHook;\n"
            "int by_value(Widget w);\n")
        sd.index.cache_clear()
        sd.file_scope.cache_clear()
        self.addCleanup(sd.index.cache_clear)
        self.addCleanup(sd.file_scope.cache_clear)
        self.addCleanup(self.tree.cleanup)

    def test_emits_only_what_the_body_names(self):
        text = sd.for_body("int f() { return compute(1, 2); }", "", src=self.src)
        self.assertIn("int compute(int a, int b);", text)
        self.assertNotIn("TheWidget", text)

    def test_skips_names_the_scaffolding_already_declares(self):
        # This is the property that makes the change inert for units that
        # compile today: they declare every name they use, so nothing is added
        # and their text - and therefore their cached verdict - is unchanged.
        text = sd.for_body("int f() { return compute(1, 2); }",
                           "int compute(int, int);", src=self.src)
        self.assertEqual(text, "")

    def test_type_a_declaration_needs_is_emitted_first(self):
        text = sd.for_body("void f() { TheWidget = 0; }", "", src=self.src)
        lines = [line for line in text.splitlines() if line]
        self.assertEqual(lines[0], "class Widget;")
        self.assertIn("extern Widget * TheWidget;", lines)

    def test_typedef_a_declaration_needs_is_emitted(self):
        text = sd.for_body("void f() { InstalledHook = 0; }", "", src=self.src)
        self.assertIn("typedef void (__cdecl *Hook)(int);", text)
        self.assertIn("extern Hook InstalledHook;", text)

    def test_by_value_class_parameter_refuses_the_whole_declaration(self):
        # A forward declaration cannot satisfy a by-value parameter, and
        # emitting one anyway trades C2065 for C2079 - a different error on
        # the same failing unit, plus a wrong claim in the preamble.
        text = sd.for_body("int f() { return by_value(0); }", "", src=self.src)
        self.assertNotIn("by_value", text)

    def test_file_scope_helper_beats_the_header_index(self):
        cpp = self.src / "unit.cpp"
        cpp.write_text("namespace {\nint local_only(int a) { return a; }\n}\n")
        text = sd.for_body("int f() { return local_only(1); }", "",
                           source_path=cpp, src=self.src)
        self.assertIn("int local_only(int a);", text)

    def test_a_shared_dependency_is_defined_once(self):
        # `RECT` was defined once as Buffer's dependency and again as Win's,
        # and the unit died on C2011 redefinition - a unit that compiled
        # before definitions were emitted at all. The preamble has to track
        # what IT has written, not only what the scaffolding wrote.
        import class_layouts
        text = sd.for_body("void f() { Buffer b; Win w; }", "")
        if "struct RECT {" not in text and "class RECT {" not in text:
            self.skipTest("this tree does not supply RECT to these classes")
        self.assertEqual(text.count("struct RECT {") + text.count("class RECT {"), 1)

    def test_unknown_name_is_simply_absent(self):
        text = sd.for_body("int f() { return mystery(1); }", "", src=self.src)
        self.assertEqual(text, "")


if __name__ == "__main__":
    unittest.main()
