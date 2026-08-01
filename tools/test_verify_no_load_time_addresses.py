#!/usr/bin/env python3

from pathlib import Path
import tempfile
import unittest

import verify_no_load_time_addresses as guard


class LoadTimeAddressTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def write(self, body):
        (self.root / "a.cpp").write_text(body, encoding="utf-8")
        return guard.offenders(self.root)

    def test_the_historical_defect_is_caught(self):
        found = self.write(
            "func_x *GraphicWinInvalidateRect =\n"
            "    *reinterpret_cast<func_x **>(0x00669304);\n")
        self.assertEqual(len(found), 1)

    def test_the_c_style_spelling_is_caught(self):
        self.assertEqual(len(self.write("int *p = *(int **)0x00669304;\n")), 1)

    # THE THREE SPELLINGS THIS GUARD USED TO MISS. It required `0x00` plus
    # exactly six digits at column zero, so it caught one of four ways the tree
    # already writes the construct and reported clean over the rest.
    def test_an_unpadded_address_is_caught(self):
        self.assertEqual(
            len(self.write("int *p = *reinterpret_cast<int **>(0x669304);\n")), 1)

    def test_an_unpadded_c_style_address_is_caught(self):
        self.assertEqual(len(self.write("int *p = *(int **)0x669304;\n")), 1)

    def test_a_declaration_indented_inside_a_namespace_is_caught(self):
        # File scope, but not column zero - which is every declaration in the
        # anonymous namespaces src/dllmain.cpp and the oracles are built from.
        found = self.write(
            "namespace {\n"
            "  int *p = *reinterpret_cast<int **>(0x00669304);\n"
            "}\n")
        self.assertEqual(len(found), 1)

    def test_a_header_is_scanned(self):
        # offenders() globbed *.cpp only, so 129 headers were never read.
        (self.root / "b.h").write_text(
            "inline int *p = *reinterpret_cast<int **>(0x00669304);\n",
            encoding="utf-8")
        self.assertEqual(len(guard.offenders(self.root)), 1)

    def test_a_small_literal_is_a_field_offset_not_an_address(self):
        # Loosening the digit count alone fires on src/console.cpp:330, where
        # `+ 0x1DD70` is a struct displacement. Below the PE ImageBase it is not
        # a game address and cannot fault at load time.
        self.assertEqual(self.write(
            "struct S { int *q = *(int **)0x1DD70; };\n"), [])

    def test_a_function_body_at_namespace_depth_is_still_allowed(self):
        # Allowing indentation must not flag the remedy this tool recommends:
        # resolve the address on first use, inside the function.
        self.assertEqual(self.write(
            "namespace {\n"
            "  void resolve() {\n"
            "    int *p = *reinterpret_cast<int **>(0x00669304);\n"
            "    (void)p;\n"
            "  }\n"
            "}\n"), [])

    def test_a_brace_inside_a_comment_does_not_open_a_scope(self):
        # The comments in src/ carry disassembly with unbalanced braces; if they
        # counted, everything after one would look like function scope and the
        # guard would go quiet.
        found = self.write(
            "// the original does { push ebp\n"
            "int *p = *reinterpret_cast<int **>(0x00669304);\n")
        self.assertEqual(len(found), 1)

    # The three shapes that must NOT fire; a guard that flags these would be
    # unusable, because the codebase is built on fixed addresses.
    def test_taking_the_address_without_loading_is_allowed(self):
        # No leading `*`: this stores the pointer, it does not read through it.
        self.assertEqual(
            self.write("func_x *Target = (func_x *)0x005ED240;\n"), [])

    def test_a_dereference_inside_a_function_is_allowed(self):
        self.assertEqual(self.write(
            "void f() {\n"
            "    p = *reinterpret_cast<func_x **>(0x00669304);\n"
            "}\n"), [])

    def test_an_indented_continuation_is_not_file_scope(self):
        self.assertEqual(self.write(
            "void f() {\n"
            "    int *q =\n"
            "        *(int **)0x00669304;\n"
            "}\n"), [])

    def test_a_small_literal_at_FILE_scope_is_not_an_address(self):
        # The existing small-literal test puts it in a struct body, which the
        # scope check skips for its own reasons - so it never exercises
        # GAME_ADDRESS_FLOOR at all, and mutating the floor to 0 survived. At
        # file scope only the floor can reject this.
        self.assertEqual(
            self.write("int *offset = *(int **)0x1DD70;\n"), [])

    def test_the_floor_admits_the_image_base_itself(self):
        # 0x00400000 IS a game address - the PE ImageBase. A floor nudged up by
        # one would silently stop rejecting loads from the very first page.
        found = self.write("int *p = *(int **)0x00400000;\n")
        self.assertEqual(len(found), 1)

    def test_the_floor_rejects_the_byte_below_the_image_base(self):
        # And nudged down by one, the floor starts flagging displacements that
        # are not addresses at all.
        self.assertEqual(self.write("int *p = *(int **)0x3FFFFF;\n"), [])

    def test_the_committed_sources_are_clean(self):
        if not guard.DEFAULT_SRC.is_dir():
            self.skipTest("src/ is absent")
        found = guard.offenders(guard.DEFAULT_SRC)
        self.assertEqual(found, [], f"load-time dereferences: {found}")

    def test_the_scan_actually_reads_the_sources(self):
        # Zero offenders over zero files would pass forever.
        if not guard.DEFAULT_SRC.is_dir():
            self.skipTest("src/ is absent")
        # The line that used to be here globbed `*.cpp))` - a pattern matching
        # nothing - and asserted its length was greater than -1, which is true
        # of every list. A vacuous assertion guarding the non-vacuity check.
        self.assertGreaterEqual(len(list(guard.DEFAULT_SRC.glob("*.cpp"))), 100)
        self.assertGreaterEqual(len(list(guard.DEFAULT_SRC.glob("*.h"))), 50)


if __name__ == "__main__":
    unittest.main()
