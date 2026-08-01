#!/usr/bin/env python3
"""A gate check that had no unit tests at all until 2026-08-01.

It guards against a Wine-backed test running without RESOURCE_LOCK, which does
not fail - it flakes, in a different test each run, and the last time that
happened it cost three separate diagnoses. The check itself matches on two
literal strings, so the interesting tests are the ones about what makes it stop
matching.

`checks-can-fail` covers the same ground end-to-end against the generated
CTestTestfile. These cover the branches that file does not happen to contain.
"""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

import verify_wine_test_locks as locks

RUNNER = f'"/repo/tools/{locks.RUNNER}"'


# The generated file quotes both the test name and every argument; the parser
# is line-oriented against exactly that shape, so the fixtures must be too.
def add_test(name, arguments):
    return f'add_test("{name}" "/usr/bin/python" {arguments})\n'


def properties(name, rest):
    return f'set_tests_properties("{name}" PROPERTIES {rest})\n'


class ParseTests(unittest.TestCase):
    def test_a_wine_backed_test_is_recognised(self):
        text = add_test("leaf-tests", f'{RUNNER} "--wine-prefix" "/tmp/p"')
        wine_backed, locked = locks.parse(text)
        self.assertEqual(["leaf-tests"], wine_backed)
        self.assertEqual(set(), locked)

    def test_the_host_side_unit_test_of_the_runner_is_not_wine_backed(self):
        # tools/test_run_windows_test.py's own path ends in the runner's name.
        # Locking it would serialise a test that needs nothing.
        text = add_test("windows-test-runner-tests",
                        '"/repo/tools/test_run_windows_test.py"')
        self.assertEqual(([], set()), locks.parse(text))

    def test_the_lock_is_found_with_or_without_quotes(self):
        # CMake generates the quoted form; hand-written CMakeLists use neither
        # consistently, and a damage case already tripped on this exact detail.
        for spelling in ('RESOURCE_LOCK "wineprefix"', "RESOURCE_LOCK wineprefix"):
            _, locked = locks.parse(properties("t", spelling))
            self.assertEqual({"t"}, locked, spelling)

    def test_a_different_lock_does_not_count(self):
        _, locked = locks.parse(properties("t", 'RESOURCE_LOCK "something"'))
        self.assertEqual(set(), locked)

    def test_running_the_runner_without_a_prefix_is_not_wine_backed(self):
        text = add_test("t", f'{RUNNER} "--executable" "x.exe"')
        self.assertEqual(([], set()), locks.parse(text))


class MainTests(unittest.TestCase):
    def run_main(self, text, *extra):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "CTestTestfile.cmake"
            path.write_text(text, encoding="utf-8")
            argv = sys.argv
            sys.argv = ["verify_wine_test_locks.py", "--ctest-file", str(path),
                        *extra]
            try:
                with contextlib.redirect_stdout(io.StringIO()) as out, \
                     contextlib.redirect_stderr(io.StringIO()) as err:
                    status = locks.main()
                return status, out.getvalue() + err.getvalue()
            finally:
                sys.argv = argv

    LOCKED = (add_test("t", f'{RUNNER} "--wine-prefix" "/tmp/p"')
              + properties("t", 'RESOURCE_LOCK "wineprefix"'))
    UNLOCKED = add_test("t", f'{RUNNER} "--wine-prefix" "/tmp/p"')

    def test_a_locked_wine_test_passes(self):
        self.assertEqual(0, self.run_main(self.LOCKED, "--expect-at-least", "1")[0])

    def test_an_unlocked_wine_test_fails_and_names_it(self):
        status, output = self.run_main(self.UNLOCKED)
        self.assertEqual(1, status)
        self.assertIn("without RESOURCE_LOCK", output)

    def test_matching_nothing_fails_when_tests_are_expected(self):
        # THE DEFECT THIS CHECK SHIPPED WITH. It printed "verified NOTHING" and
        # returned 0, so renaming the runner or the flag turned the check off
        # while CTest reported PASS.
        status, output = self.run_main("", "--expect-at-least", "7")
        self.assertEqual(1, status)
        self.assertIn("blinded", output)

    def test_matching_nothing_is_allowed_when_none_are_expected(self):
        # A native MinGW-on-Windows build legitimately has no Wine-backed test.
        self.assertEqual(0, self.run_main("")[0])

    def test_more_wine_tests_than_expected_is_not_a_failure(self):
        # "At least", deliberately: adding an eighth Wine test must not turn the
        # gate red for a number that was only ever a floor.
        text = self.LOCKED + add_test("u", f'{RUNNER} "--wine-prefix" "/tmp/p"') \
            + properties("u", 'RESOURCE_LOCK "wineprefix"')
        self.assertEqual(0, self.run_main(text, "--expect-at-least", "1")[0])

    def test_an_absent_ctest_file_returns_two(self):
        argv = sys.argv
        sys.argv = ["verify_wine_test_locks.py", "--ctest-file", "/nonexistent"]
        try:
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(2, locks.main())
        finally:
            sys.argv = argv


if __name__ == "__main__":
    unittest.main()
