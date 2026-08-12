#!/usr/bin/env python3
"""Unit tests for the tool-reachability check.

The check exists because the obvious version of this audit is wrong, and it was
wrong here first: rooting reachability at `CMakeLists.txt` alone called
`mizuchi_context.py` dead, and `mizuchi.yaml` invokes it. So the tests that
matter are the ones about the ROOT SET - that a non-CMake entry point counts,
and that a test file does not.
"""

import io
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import verify_tool_reachability as checker  # noqa: E402


class TreeBuilder:
    """A miniature repository: entry points at the root, scripts in tools/."""

    def __init__(self, name):
        self.root = Path(name)
        (self.root / "tools").mkdir()
        (self.root / "docs").mkdir()
        (self.root / "docs" / "recovery").mkdir()
        (self.root / "CMakeLists.txt").write_text("")

    def tool(self, stem, body=""):
        (self.root / "tools" / f"{stem}.py").write_text(body)

    def entry(self, name, text):
        (self.root / name).write_text(text)


class RootSetTest(unittest.TestCase):

    def test_a_tool_named_by_cmake_is_reached(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha")
            tree.entry("CMakeLists.txt", "tools/alpha.py")
            stale, unreachable, reached = checker.audit(tree.root)
            self.assertEqual((stale, unreachable, reached), ([], [], 1))

    def test_a_tool_named_only_by_a_yaml_harness_is_reached(self):
        # The defect this whole check was written after: mizuchi.yaml is an
        # entry point in a language CMake has never heard of.
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha")
            tree.entry("mizuchi.yaml", "getContextScript: tools/alpha.py")
            stale, unreachable, _ = checker.audit(tree.root)
            self.assertEqual((stale, unreachable), ([], []))

    def test_a_tool_named_only_by_a_doc_is_reached(self):
        # A tool a human runs by hand is reachable exactly when something
        # tells a human to run it. That is the rule, not a loophole.
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha")
            (tree.root / "docs" / "TOOLS.md").write_text("run tools/alpha.py")
            stale, unreachable, _ = checker.audit(tree.root)
            self.assertEqual((stale, unreachable), ([], []))

    def test_a_tool_reached_only_through_an_import_counts(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha", "import beta\n")
            tree.tool("beta")
            tree.entry("CMakeLists.txt", "tools/alpha.py")
            stale, unreachable, reached = checker.audit(tree.root)
            self.assertEqual((stale, unreachable, reached), ([], [], 2))

    def test_an_import_chain_of_three_is_followed(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha", "import beta\n")
            tree.tool("beta", "from gamma import thing\n")
            tree.tool("gamma")
            tree.entry("CMakeLists.txt", "tools/alpha.py")
            _, unreachable, reached = checker.audit(tree.root)
            self.assertEqual((unreachable, reached), ([], 3))

    def test_a_TEST_FILE_IS_NOT_A_ROOT(self):
        # A tool that exists only to be tested is unreachable, and saying so
        # is the point: thirteen tools were in exactly that state.
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha")
            tree.tool("test_alpha", "import alpha\n")
            tree.entry("CMakeLists.txt", "test_alpha.py")
            _, unreachable, _ = checker.audit(tree.root)
            self.assertEqual(unreachable, ["alpha"])

    def test_a_test_file_is_never_itself_reported_unreachable(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("test_alpha")
            _, unreachable, _ = checker.audit(tree.root)
            self.assertEqual(unreachable, [])


class FailureTest(unittest.TestCase):

    def test_a_tool_nothing_names_is_unreachable(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("orphan")
            _, unreachable, _ = checker.audit(tree.root)
            self.assertEqual(unreachable, ["orphan"])

    def test_an_entry_point_naming_a_missing_tool_is_stale(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.entry("CMakeLists.txt", "tools/gone.py")
            stale, _, _ = checker.audit(tree.root)
            self.assertEqual(stale, ["gone.py"])

    def test_the_bare_backticked_spelling_counts_when_the_file_exists(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha")
            (tree.root / "docs" / "TOOLS.md").write_text("run `alpha.py`")
            _, unreachable, _ = checker.audit(tree.root)
            self.assertEqual(unreachable, [])

    def test_a_bare_name_that_is_not_a_tool_is_not_reported_stale(self):
        # Prose mentions `setup.py` and `conftest.py` all the time; only the
        # qualified spelling can be stale, because only it claims a path.
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            (tree.root / "docs" / "TOOLS.md").write_text("unlike `setup.py`")
            stale, _, _ = checker.audit(tree.root)
            self.assertEqual(stale, [])


class GateTest(unittest.TestCase):
    """The gate's own invocation - the argv CMake passes."""

    def _run(self, root):
        buffer = io.StringIO()
        with redirect_stdout(buffer):
            code = checker.main(["--root", str(root)])
        return code, buffer.getvalue()

    def test_a_clean_tree_passes(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha")
            tree.entry("CMakeLists.txt", "tools/alpha.py")
            code, output = self._run(tree.root)
            self.assertEqual(code, 0, output)

    def test_an_orphan_tool_FAILS_the_gate(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha")
            tree.entry("CMakeLists.txt", "tools/alpha.py")
            tree.tool("orphan")
            code, output = self._run(tree.root)
            self.assertEqual(code, 1, output)
            self.assertIn("UNREACHABLE", output)

    def test_a_stale_reference_FAILS_the_gate(self):
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            tree.tool("alpha")
            tree.entry("CMakeLists.txt", "tools/alpha.py tools/gone.py")
            code, output = self._run(tree.root)
            self.assertEqual(code, 1, output)
            self.assertIn("STALE", output)

    def test_a_tree_with_no_tools_at_all_FAILS_rather_than_passing(self):
        # Nothing reached means nothing proved, which is the vacuity trap this
        # project keeps falling into.
        with tempfile.TemporaryDirectory() as name:
            tree = TreeBuilder(name)
            code, output = self._run(tree.root)
            self.assertEqual(code, 1, output)
            self.assertIn("proves nothing", output)


if __name__ == "__main__":
    unittest.main()
