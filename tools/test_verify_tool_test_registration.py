#!/usr/bin/env python3
"""The DEPENDS case is the one that matters.

A test file named in a `DEPENDS` list is referenced by CMake, so any check built
on `if name in cmake_text` calls it registered. It is not: DEPENDS makes the
build re-run when the file changes, it never executes it. That is precisely the
shape this project keeps finding - a reference that looks like coverage.
"""
import tempfile
import unittest
from pathlib import Path

import verify_tool_test_registration as guard

HELPER = 'opensmacx_add_python_tool_test(some-tests {name})\n'
COMMAND = (
    '    COMMAND "${{OPENSMACX_PYTHON_EXECUTABLE}}"\n'
    '        "${{CMAKE_CURRENT_SOURCE_DIR}}/tools/{name}"\n'
)
DEPENDS = (
    '    DEPENDS\n'
    '        "${{CMAKE_CURRENT_SOURCE_DIR}}/tools/{name}"\n'
)


class ExecutedTestsTests(unittest.TestCase):
    def test_the_helper_registration_counts(self):
        self.assertEqual({"test_a.py"},
                         guard.executed_tests(HELPER.format(name="test_a.py")))

    def test_a_direct_python_command_counts(self):
        # Three test files are run this way, gating a generator immediately
        # before it runs.
        self.assertEqual({"test_b.py"},
                         guard.executed_tests(COMMAND.format(name="test_b.py")))

    def test_a_depends_entry_does_not_count(self):
        text = DEPENDS.format(name="test_c.py")
        self.assertEqual(set(), guard.executed_tests(text))
        # ...but it IS a mention, which is what makes it deceptive.
        self.assertEqual({"test_c.py"}, guard.mentioned_tests(text))


class EndToEndTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.tools = self.root / "tools"
        self.tools.mkdir()
        self.cmake = self.root / "CMakeLists.txt"

    def write_tools(self, count):
        for index in range(count):
            (self.tools / f"test_{index}.py").write_text("", encoding="utf-8")

    def run_guard(self, floor=3):
        import contextlib
        import io
        import sys
        argv = sys.argv
        sys.argv = ["verify_tool_test_registration.py",
                    "--cmake", str(self.cmake), "--tools", str(self.tools),
                    "--floor", str(floor)]
        try:
            with contextlib.redirect_stdout(io.StringIO()), \
                 contextlib.redirect_stderr(io.StringIO()) as errors:
                status = guard.main()
            return status, errors.getvalue()
        finally:
            sys.argv = argv

    def test_all_registered_passes(self):
        self.write_tools(3)
        self.cmake.write_text(
            "".join(HELPER.format(name=f"test_{i}.py") for i in range(3)),
            encoding="utf-8")
        self.assertEqual(0, self.run_guard()[0])

    def test_an_unmentioned_test_fails(self):
        self.write_tools(3)
        self.cmake.write_text(
            "".join(HELPER.format(name=f"test_{i}.py") for i in range(2)),
            encoding="utf-8")
        status, errors = self.run_guard()
        self.assertEqual(1, status)
        self.assertIn("not named in CMakeLists.txt at all", errors)

    def test_a_test_only_in_depends_fails_and_says_so(self):
        self.write_tools(3)
        self.cmake.write_text(
            HELPER.format(name="test_0.py")
            + HELPER.format(name="test_1.py")
            + DEPENDS.format(name="test_2.py"),
            encoding="utf-8")
        status, errors = self.run_guard()
        self.assertEqual(1, status)
        self.assertIn("only as a dependency", errors)

    def test_too_few_files_refuses_rather_than_reporting_clean(self):
        # A wrong --tools finds nothing and every file in it is registered.
        self.write_tools(2)
        self.cmake.write_text(
            "".join(HELPER.format(name=f"test_{i}.py") for i in range(2)),
            encoding="utf-8")
        status, errors = self.run_guard(floor=10)
        self.assertEqual(1, status)
        self.assertIn("below the floor", errors)

    def test_parsing_no_registrations_at_all_refuses(self):
        self.write_tools(3)
        self.cmake.write_text("# the shape changed entirely\n", encoding="utf-8")
        status, errors = self.run_guard()
        self.assertEqual(1, status)
        self.assertIn("ZERO registrations", errors)

    def test_a_registration_naming_a_deleted_file_fails(self):
        self.write_tools(3)
        self.cmake.write_text(
            "".join(HELPER.format(name=f"test_{i}.py") for i in range(3))
            + HELPER.format(name="test_deleted.py"),
            encoding="utf-8")
        status, errors = self.run_guard()
        self.assertEqual(1, status)
        self.assertIn("do not exist", errors)


class DisabledSpellingTests(unittest.TestCase):
    """The spellings CMake accepts for DISABLED, and the group it extracts.

    Mutation-testing this tool against its own suite left 67% of mutants alive,
    and two of the survivors were real: the `1` in `DISABLED\\s+(?:TRUE|ON|1)`
    could be changed to 0 or 2 unnoticed, and `match.group(1)` could become
    `group(0)` unnoticed. The suite only ever exercised `DISABLED TRUE`, so
    every other spelling CMake honours was untested and the name extraction was
    never pinned at all.
    """

    def cmake_with(self, spelling):
        return ("opensmacx_add_python_tool_test(demo-tests test_demo.py)\n"
                f'set_tests_properties("demo-tests" PROPERTIES {spelling})\n')

    def test_every_spelling_cmake_accepts_disables_the_test(self):
        for spelling in ("DISABLED TRUE", "DISABLED ON", "DISABLED 1",
                         "DISABLED true", "DISABLED On"):
            with self.subTest(spelling=spelling):
                self.assertEqual(
                    set(), guard.executed_tests(self.cmake_with(spelling)),
                    f"{spelling} should disable the test")

    def test_disabled_false_does_not_disable_it(self):
        # The reverse matters as much: a check that treats DISABLED FALSE as
        # disabled would hide a test that really does run.
        for spelling in ("DISABLED FALSE", "DISABLED OFF", "DISABLED 0"):
            with self.subTest(spelling=spelling):
                self.assertEqual(
                    {"test_demo.py"},
                    guard.executed_tests(self.cmake_with(spelling)), spelling)

    def test_the_disabled_name_is_taken_from_the_right_capture_group(self):
        # group(0) is the whole match, so a mutation to it silently stops any
        # test from ever being recognised as disabled.
        text = ("opensmacx_add_python_tool_test(alpha-tests test_alpha.py)\n"
                "opensmacx_add_python_tool_test(beta-tests test_beta.py)\n"
                'set_tests_properties("beta-tests" PROPERTIES DISABLED TRUE)\n')
        self.assertEqual({"test_alpha.py"}, guard.executed_tests(text))

    def test_a_property_block_naming_another_test_does_not_disable_this_one(self):
        text = ("opensmacx_add_python_tool_test(alpha-tests test_alpha.py)\n"
                'set_tests_properties("other-tests" PROPERTIES DISABLED TRUE)\n')
        self.assertEqual({"test_alpha.py"}, guard.executed_tests(text))


class CommittedTreeTests(unittest.TestCase):
    def test_the_repository_is_clean(self):
        self.assertEqual([], [
            name for name in
            sorted(p.name for p in (guard.REPO_ROOT / "tools").glob("test_*.py"))
            if name not in guard.executed_tests(
                (guard.REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8"))
        ])


if __name__ == "__main__":
    unittest.main()
