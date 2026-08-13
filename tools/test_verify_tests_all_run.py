#!/usr/bin/env python3
"""Unit tests for the stranded-test-class check.

The defect is quiet by construction - the offending file still exits 0 - so what
these pin is that the check sees a class below the guard, and that it does NOT
complain about the three shapes that are fine.
"""

import io
import shutil
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

import verify_tests_all_run as checker


class StrandedTest(unittest.TestCase):
    def setUp(self):
        self.work = Path(tempfile.mkdtemp())

    def tearDown(self):
        shutil.rmtree(self.work, ignore_errors=True)

    def write(self, text, name="test_x.py"):
        path = self.work / name
        path.write_text(text)
        return path

    def test_a_class_below_the_guard_is_stranded(self):
        guard, below = checker.stranded(self.write(
            "import unittest\n"
            "class A(unittest.TestCase):\n    def test_a(self): pass\n"
            'if __name__ == "__main__":\n    unittest.main()\n'
            "class B(unittest.TestCase):\n    def test_b(self): pass\n"))
        self.assertEqual(below, [("B", 1)])
        self.assertEqual(guard, 4)

    def test_the_guard_at_the_end_strands_nothing(self):
        _, below = checker.stranded(self.write(
            "import unittest\n"
            "class A(unittest.TestCase):\n    def test_a(self): pass\n"
            'if __name__ == "__main__":\n    unittest.main()\n'))
        self.assertEqual(below, [])

    def test_a_file_with_no_guard_is_not_an_offender(self):
        # `python -m unittest <module>` runs it regardless, and several files
        # here are fixture libraries rather than entry points.
        _, below = checker.stranded(self.write(
            "import unittest\n"
            "class A(unittest.TestCase):\n    def test_a(self): pass\n"))
        self.assertEqual(below, [])

    def test_a_nested_main_call_is_not_the_guard(self):
        # Only a TOP-LEVEL block ends module execution. One inside a function
        # cannot strand anything, and treating it as the guard would report
        # every class after that function.
        _, below = checker.stranded(self.write(
            "import unittest\n"
            "def run():\n"
            '    if __name__ == "__main__":\n        unittest.main()\n'
            "class A(unittest.TestCase):\n    def test_a(self): pass\n"))
        self.assertEqual(below, [])

    def test_the_test_count_is_reported_not_just_the_class(self):
        # The number is what tells a reader how much was not running: 16 is a
        # different problem from 1.
        _, below = checker.stranded(self.write(
            "import unittest\n"
            'if __name__ == "__main__":\n    unittest.main()\n'
            "class B(unittest.TestCase):\n"
            "    def test_one(self): pass\n"
            "    def test_two(self): pass\n"
            "    def helper(self): pass\n"))
        self.assertEqual(below, [("B", 2)])


class SweepTest(unittest.TestCase):
    def setUp(self):
        self.work = Path(tempfile.mkdtemp())

    def tearDown(self):
        shutil.rmtree(self.work, ignore_errors=True)

    def run_main(self):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            status = checker.main(["--tools", str(self.work)])
        return status, out.getvalue() + err.getvalue()

    def populate(self, count, offender=False):
        for index in range(count):
            body = ("import unittest\n"
                    f"class A{index}(unittest.TestCase):\n"
                    "    def test_a(self): pass\n"
                    'if __name__ == "__main__":\n    unittest.main()\n')
            if offender and index == 0:
                body += "class Late(unittest.TestCase):\n    def test_l(self): pass\n"
            (self.work / f"test_{index}.py").write_text(body)

    def test_a_clean_sweep_passes(self):
        self.populate(25)
        self.assertEqual(self.run_main()[0], 0)

    def test_one_offender_fails_and_names_the_class(self):
        self.populate(25, offender=True)
        status, output = self.run_main()
        self.assertEqual(status, 1)
        self.assertIn("Late", output)

    def test_a_glob_that_matches_almost_nothing_refuses(self):
        # The failure this check exists to catch, applied to itself: a sweep
        # over an empty population must not report a clean sweep.
        self.populate(3)
        status, output = self.run_main()
        self.assertEqual(status, 1)
        self.assertIn("glob is wrong", output)


if __name__ == "__main__":
    unittest.main()
