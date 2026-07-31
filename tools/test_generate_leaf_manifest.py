#!/usr/bin/env python3
"""Tests for tools/generate_leaf_manifest.py."""

import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import generate_leaf_manifest as gen

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LEAF_DIR = os.path.join(REPO, "tests", "leaf")


def family_file(directory, family, body):
    path = os.path.join(directory, f"{family}_tests.cpp")
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(f'#include "leaf_shared.h"\n\n'
                     f'#define LEAF_FAMILY "{family}"\n\n{body}')
    return path


class CountingTests(unittest.TestCase):
    def test_counts_only_lines_that_start_with_leaf_case(self):
        with tempfile.TemporaryDirectory() as directory:
            path = family_file(directory, "alpha", "\n".join([
                "LEAF_CASE(0, test_a);",
                "LEAF_CASE(1, test_b);",
                "    LEAF_CASE(2, test_indented);",
                "// LEAF_CASE(3, test_commented);",
                "LEAF_CASE(LEAF_APPEND, test_c);",
            ]) + "\n")
            self.assertEqual(gen.rows_for([path]), [("alpha", 3)])

    def test_indented_registration_is_not_counted(self):
        # An `#if 0` around a LEAF_CASE leaves the text in the file.  The
        # counter deliberately does not try to understand the preprocessor;
        # the run-time count check is what catches a text/registration
        # mismatch, and it can only do that if this stays literal.
        with tempfile.TemporaryDirectory() as directory:
            path = family_file(directory, "alpha",
                               "#if 0\nLEAF_CASE(0, test_dead);\n#endif\n")
            self.assertEqual(gen.rows_for([path]), [("alpha", 1)])

    def test_rows_are_sorted_by_family(self):
        with tempfile.TemporaryDirectory() as directory:
            zulu = family_file(directory, "zulu", "LEAF_CASE(0, test_z);\n")
            alpha = family_file(directory, "alpha", "LEAF_CASE(1, test_a);\n")
            self.assertEqual(gen.rows_for([zulu, alpha]),
                             [("alpha", 1), ("zulu", 1)])

    def test_family_with_no_cases_still_gets_a_row(self):
        with tempfile.TemporaryDirectory() as directory:
            path = family_file(directory, "alpha", "\n")
            self.assertEqual(gen.rows_for([path]), [("alpha", 0)])


class RejectionTests(unittest.TestCase):
    def test_missing_leaf_family_define_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "alpha_tests.cpp")
            with open(path, "w", encoding="utf-8") as handle:
                handle.write("LEAF_CASE(0, test_a);\n")
            with self.assertRaises(gen.ManifestError):
                gen.rows_for([path])

    def test_family_name_must_match_file_name(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "alpha_tests.cpp")
            with open(path, "w", encoding="utf-8") as handle:
                handle.write('#define LEAF_FAMILY "beta"\n'
                             'LEAF_CASE(0, test_a);\n')
            with self.assertRaises(gen.ManifestError):
                gen.rows_for([path])

    def test_two_files_cannot_share_a_family(self):
        with tempfile.TemporaryDirectory() as directory:
            one = family_file(directory, "alpha", "LEAF_CASE(0, test_a);\n")
            nested = os.path.join(directory, "nested")
            os.makedirs(nested)
            two = family_file(nested, "alpha", "LEAF_CASE(1, test_b);\n")
            with self.assertRaises(gen.ManifestError):
                gen.rows_for([one, two])

    def test_bad_file_suffix_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "alpha.cpp")
            with open(path, "w", encoding="utf-8") as handle:
                handle.write('#define LEAF_FAMILY "alpha"\n')
            with self.assertRaises(gen.ManifestError):
                gen.rows_for([path])


class OutputTests(unittest.TestCase):
    def test_render_emits_one_macro_per_family(self):
        text = gen.render([("alpha", 3), ("beta", 0)])
        self.assertIn("LEAF_FAMILY_COUNT(alpha, 3)\n", text)
        self.assertIn("LEAF_FAMILY_COUNT(beta, 0)\n", text)

    def test_write_if_changed_leaves_an_identical_file_alone(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "out", "manifest.h")
            self.assertTrue(gen.write_if_changed(path, "hello"))
            self.assertFalse(gen.write_if_changed(path, "hello"))
            self.assertTrue(gen.write_if_changed(path, "goodbye"))

    def test_main_writes_the_manifest(self):
        with tempfile.TemporaryDirectory() as directory:
            path = family_file(directory, "alpha", "LEAF_CASE(0, test_a);\n")
            output = os.path.join(directory, "gen", "manifest.h")
            self.assertEqual(gen.main(["--output", output, path]), 0)
            with open(output, encoding="utf-8") as handle:
                self.assertIn("LEAF_FAMILY_COUNT(alpha, 1)", handle.read())

    def test_main_reports_a_bad_family_without_writing(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "alpha_tests.cpp")
            with open(path, "w", encoding="utf-8") as handle:
                handle.write("LEAF_CASE(0, test_a);\n")
            output = os.path.join(directory, "manifest.h")
            self.assertEqual(gen.main(["--output", output, path]), 2)
            self.assertFalse(os.path.exists(output))


class RealTreeTests(unittest.TestCase):
    """The counts the build will actually use, against the checked-in tree."""

    def test_the_real_leaf_tree_totals_the_baseline_case_count(self):
        sources = sorted(
            os.path.join(LEAF_DIR, name)
            for name in os.listdir(LEAF_DIR)
            if name.endswith(gen.FILE_SUFFIX))
        rows = gen.rows_for(sources)
        self.assertEqual(len(rows), len(sources))
        self.assertEqual(sum(count for _, count in rows), 229)


if __name__ == "__main__":
    unittest.main()
