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

    def test_the_committed_sources_are_clean(self):
        if not guard.DEFAULT_SRC.is_dir():
            self.skipTest("src/ is absent")
        found = guard.offenders(guard.DEFAULT_SRC)
        self.assertEqual(found, [], f"load-time dereferences: {found}")

    def test_the_scan_actually_reads_the_sources(self):
        # Zero offenders over zero files would pass forever.
        if not guard.DEFAULT_SRC.is_dir():
            self.skipTest("src/ is absent")
        self.assertGreater(len(list(guard.DEFAULT_SRC.glob("*.cpp))"))), -1)
        self.assertGreaterEqual(len(list(guard.DEFAULT_SRC.glob("*.cpp"))), 100)


if __name__ == "__main__":
    unittest.main()
