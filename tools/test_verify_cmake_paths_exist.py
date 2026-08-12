#!/usr/bin/env python3
"""Unit tests for the CMake source-path check.

The defect this was written for is quiet by construction: a retired tool's
CMake references configure clean, and a target outside `all` is never built, so
nothing resolves the path until somebody invokes that target. The tests pin the
two halves that keep it honest - it sees a live reference, and it does not
complain about the two things that legitimately do not resolve.
"""

import io
import shutil
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

import verify_cmake_paths_exist as checker


class ReferenceTest(unittest.TestCase):
    def test_a_literal_source_path_is_a_reference(self):
        found = checker.references(
            '    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/x.py"\n')
        self.assertEqual(found, [(1, "tools/x.py")])

    def test_a_computed_path_is_not_resolved(self):
        # `${OPENSMACX_LEGACY_EXE}` is a configure-time value that usually
        # points into an ignored artifact directory. Resolving it would make
        # the check refuse a healthy clean checkout.
        self.assertEqual(checker.references(
            '    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${OPENSMACX_EXE}"\n'), [])

    def test_a_commented_reference_is_not_a_reference(self):
        # Retirement notes name the tools they retired. Demanding those files
        # exist would mean deleting the history along with the code.
        self.assertEqual(checker.references(
            '# see "${CMAKE_CURRENT_SOURCE_DIR}/gone.py" (retired)\n'), [])

    def test_a_trailing_comment_does_not_hide_the_code_before_it(self):
        found = checker.references(
            '    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/a.py"  # and not b.py\n')
        self.assertEqual(found, [(1, "a.py")])

    def test_the_line_number_is_reported(self):
        found = checker.references(
            '\n\n"${CMAKE_CURRENT_SOURCE_DIR}/tools/x.py"\n')
        self.assertEqual(found[0][0], 3)


class MissingTest(unittest.TestCase):
    def setUp(self):
        self.work = Path(tempfile.mkdtemp())

    def tearDown(self):
        shutil.rmtree(self.work, ignore_errors=True)

    def test_a_path_in_the_tree_resolves(self):
        (self.work / "there.py").write_text("")
        self.assertEqual(checker.missing(
            self.work, '"${CMAKE_CURRENT_SOURCE_DIR}/there.py"\n'), [])

    def test_a_directory_counts_as_present(self):
        (self.work / "src").mkdir()
        self.assertEqual(checker.missing(
            self.work, '"${CMAKE_CURRENT_SOURCE_DIR}/src"\n'), [])

    def test_the_retired_placeholder_spelling_is_caught(self):
        # The exact shape found in the tree on 2026-08-12: a tool name rewritten
        # in place, leaving a live COMMAND naming a file nobody ever created.
        gone = checker.missing(
            self.work,
            '    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/run_game (retired)"\n')
        self.assertEqual(gone, [(1, "run_game (retired)")])

    def test_a_deleted_data_file_is_caught_too(self):
        # `docs/recovery/functions.csv` outlived its deletion in two DEPENDS
        # lists. The defect is the same one; the path just is not a script.
        gone = checker.missing(
            self.work,
            '        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/gone.csv"\n')
        self.assertEqual(len(gone), 1)


class ExitCodeTest(unittest.TestCase):
    """`--check` is a gate, so the exit code is the contract."""

    def setUp(self):
        self.work = Path(tempfile.mkdtemp())
        self.lists = self.work / "CMakeLists.txt"

    def tearDown(self):
        shutil.rmtree(self.work, ignore_errors=True)

    def run_on(self, text):
        self.lists.write_text(text)
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            status = checker.main(["--cmakelists", str(self.lists), "--check"])
        return status, out.getvalue() + err.getvalue()

    def test_a_resolvable_tree_passes(self):
        (self.work / "x.py").write_text("")
        status, _ = self.run_on('"${CMAKE_CURRENT_SOURCE_DIR}/x.py"\n')
        self.assertEqual(status, 0)

    def test_an_unresolvable_reference_fails_and_names_it(self):
        status, output = self.run_on('"${CMAKE_CURRENT_SOURCE_DIR}/no.py"\n')
        self.assertEqual(status, 1)
        self.assertIn("no.py", output)

    def test_a_file_with_no_references_at_all_passes(self):
        # Not a failure: a CMakeLists that names no literal source path has
        # nothing to be wrong about. Reporting a count is what keeps that
        # visible rather than looking like a check that ran.
        status, output = self.run_on("project(x)\n")
        self.assertEqual(status, 0)
        self.assertIn("0 literal source path", output)


if __name__ == "__main__":
    unittest.main()
