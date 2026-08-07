#!/usr/bin/env python3
"""Unit tests for moving a recovery out of the store into product source.

The dangerous part of this tool is not the append - it is that it edits
`functions.csv` and then has to be able to put it back. A half-applied
integration leaves a row pointing at a body that is not there, which reads as
a different function entirely; that has cost a day before. So what is tested
is the revert, and the catalogue arithmetic that decides where the row points.
"""

import tempfile
import unittest
from pathlib import Path
from unittest import mock

import integrate_recovery as tool


class DocCommentTest(unittest.TestCase):
    """A harvested body has no doc comment, and the catalogue needs one."""

    def test_a_body_without_one_gets_the_required_lines(self):
        text = tool.with_doc_comment(0x401000, "void f() {}\n")
        self.assertIn("Original Offset: 00401000", text)
        self.assertIn("Status: Complete", text)
        self.assertTrue(text.startswith("/*"))

    def test_a_body_that_has_one_is_left_alone(self):
        body = "/*\nPurpose: something real.\nOriginal Offset: 00401000\n*/\nvoid f() {}\n"
        self.assertEqual(tool.with_doc_comment(0x401000, body), body)

    def test_purpose_is_never_invented(self):
        # It is the one line that says what the function is FOR, it cannot be
        # derived from an address, and a generated placeholder would read
        # exactly like a real one.
        self.assertNotIn("Purpose:", tool.with_doc_comment(0x401000, "void f() {}"))

    def test_the_catalogue_line_is_inside_the_comment(self):
        text = tool.with_doc_comment(0x401000, "void f() {}\n")
        lines = text.splitlines()
        index = tool.catalogue_line(text)
        self.assertEqual(lines[index - 1].strip(), "Original Offset: 00401000")
        # and the comment must still be open at that point
        self.assertNotIn("*/", "\n".join(lines[:index]))

    def test_a_body_with_no_offset_line_is_refused(self):
        with self.assertRaises(tool.Refused):
            tool.catalogue_line("void f() {}\n")


class IntegrateTest(unittest.TestCase):
    def setUp(self):
        self.holder = tempfile.TemporaryDirectory()
        root = Path(self.holder.name)
        self.target = root / "owner.cpp"
        self.target.write_text("#include \"stdafx.h\"\n\nvoid other() {}\n")
        self.before = self.target.read_text()

        self.functions = tool.emit.load_functions
        self.verify = tool.writeback.verify
        self.extract = tool.census.extract_body
        self.setloc = tool.set_source_location
        self.stored = tool.stored_body
        self.root = tool.REPO_ROOT
        tool.REPO_ROOT = root
        tool.emit.load_functions = lambda: {
            0x401000: {"name": "?f@C@@QAEXXZ", "source_locations": ""}}
        tool.stored_body = lambda a: "void C::f() {}\n"
        tool.census.extract_body = lambda loc: "void C::f() {}\n"
        self.located = []
        tool.set_source_location = lambda a, loc: self.located.append(loc)

    def tearDown(self):
        tool.emit.load_functions = self.functions
        tool.writeback.verify = self.verify
        tool.census.extract_body = self.extract
        tool.set_source_location = self.setloc
        tool.stored_body = self.stored
        tool.REPO_ROOT = self.root
        self.holder.cleanup()

    def test_a_byte_exact_body_lands_and_is_catalogued(self):
        tool.writeback.verify = lambda a, b: {"tier": "BYTE_EXACT"}
        result = tool.integrate(0x401000, self.target)
        self.assertIn("void C::f() {}", self.target.read_text())
        self.assertEqual(result["source_location"], self.located[-1])
        # 3 lines already in the file, a blank, then `/*` and the offset line.
        self.assertTrue(result["source_location"].endswith(":6"),
                        result["source_location"])

    def test_the_catalogued_line_really_is_the_offset_line(self):
        tool.writeback.verify = lambda a, b: {"tier": "BYTE_EXACT"}
        tool.integrate(0x401000, self.target)
        line = int(self.located[-1].rsplit(":", 1)[1])
        text = self.target.read_text().splitlines()[line - 1]
        self.assertEqual(text.strip(), "Original Offset: 00401000")

    def test_anything_short_of_byte_exact_reverts_the_file(self):
        tool.writeback.verify = lambda a, b: {"tier": "MISMATCH", "note": "x"}
        with self.assertRaises(tool.Refused):
            tool.integrate(0x401000, self.target)
        self.assertEqual(self.target.read_text(), self.before)

    def test_a_failed_integration_puts_the_catalogue_back(self):
        # The file being restored is not enough. A row still pointing into a
        # file whose body was removed names whatever moved into those lines.
        tool.writeback.verify = lambda a, b: {"tier": "NO_COMPILE"}
        with self.assertRaises(tool.Refused):
            tool.integrate(0x401000, self.target)
        self.assertEqual(self.located[-1], "")

    def test_a_row_that_already_has_a_home_is_refused(self):
        tool.emit.load_functions = lambda: {
            0x401000: {"name": "f", "source_locations": "src/a.cpp:10"}}
        with self.assertRaises(tool.Refused):
            tool.integrate(0x401000, self.target)
        self.assertEqual(self.target.read_text(), self.before)

    def test_a_missing_target_is_refused_before_anything_is_read(self):
        with self.assertRaises(tool.Refused):
            tool.integrate(0x401000, Path(self.holder.name) / "nope.cpp")


if __name__ == "__main__":
    unittest.main()
