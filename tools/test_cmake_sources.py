#!/usr/bin/env python3
"""Tests for tools/cmake_sources.py."""

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import cmake_sources as tool  # noqa: E402


class DiscoveryTests(unittest.TestCase):

    def workspace(self):
        root = Path(tempfile.mkdtemp())
        self.addCleanup(__import__("shutil").rmtree, root, True)
        return root

    def test_the_root_file_comes_first(self):
        # Callers concatenate, and CMake's own order is root-then-subdirectory.
        # Sorting alone would put `tests/CMakeLists.txt` before the root one on
        # any tree whose top-level directory sorts after "tests".
        root = self.workspace()
        (root / "CMakeLists.txt").write_text("root\n")
        (root / "aaa").mkdir()
        (root / "aaa" / "CMakeLists.txt").write_text("aaa\n")
        found = tool.cmake_files(root)
        self.assertEqual(found[0], root / "CMakeLists.txt")
        self.assertEqual(len(found), 2)

    def test_a_subdirectory_file_is_found(self):
        # THE WHOLE POINT. Every `add_test` moved into tests/CMakeLists.txt on
        # 2026-08-15, and three checks reading only the root file went blind.
        root = self.workspace()
        (root / "CMakeLists.txt").write_text("root\n")
        (root / "tests").mkdir()
        (root / "tests" / "CMakeLists.txt").write_text("add_test(NAME x)\n")
        self.assertIn("add_test(NAME x)", tool.cmake_text(root))

    def test_build_directories_are_not_read(self):
        # CMake copies CMakeLists.txt into its own scratch under build/, and
        # counting those double-counts every registration.
        root = self.workspace()
        (root / "CMakeLists.txt").write_text("root\n")
        for name in ("build", "build-lean", ".git", "__pycache__"):
            (root / name).mkdir()
            (root / name / "CMakeLists.txt").write_text("GENERATED COPY\n")
        self.assertEqual(tool.cmake_files(root), [root / "CMakeLists.txt"])
        self.assertNotIn("GENERATED COPY", tool.cmake_text(root))

    def test_a_tree_with_no_cmakelists_is_an_error(self):
        # The floor. An empty list downstream reads as "the build registers
        # nothing", which is the vacuous pass this module exists to prevent.
        with self.assertRaises(SystemExit):
            tool.cmake_files(self.workspace())

    def test_the_text_names_the_file_each_part_came_from(self):
        root = self.workspace()
        (root / "CMakeLists.txt").write_text("root\n")
        (root / "tests").mkdir()
        (root / "tests" / "CMakeLists.txt").write_text("sub\n")
        self.assertIn("tests/CMakeLists.txt", tool.cmake_text(root))

    def test_the_real_project_has_both_of_its_files(self):
        # Pins the live tree, not a fixture: if the split is undone or moved,
        # this says so rather than passing over whatever is left.
        names = [path.name for path in tool.cmake_files()]
        self.assertEqual(names, ["CMakeLists.txt"] * len(names))
        relative = [str(path.relative_to(tool.REPO_ROOT))
                    for path in tool.cmake_files()]
        self.assertIn("CMakeLists.txt", relative)
        self.assertIn("tests/CMakeLists.txt", relative)


if __name__ == "__main__":
    unittest.main()
