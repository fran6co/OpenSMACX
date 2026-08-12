#!/usr/bin/env python3
"""Unit tests for the "no test writes into the source tree" check.

Every case builds a REAL git repository in a temporary directory and points the
check at it with `--repo`. Pointing it at this repository would make the tests
indistinguishable from the defect they hunt.

The properties worth pinning are the two ways this check could be worthless: it
could report everything (which the first draft did, because running a Python
test writes `__pycache__`), or it could report nothing and still print OK.
"""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import verify_tests_do_not_write as checker

POLITE = ("import unittest\n"
          "class T(unittest.TestCase):\n"
          "    def test_ok(self): self.assertTrue(True)\n")

RUDE = ("import pathlib, unittest\n"
        "ROOT = pathlib.Path(__file__).resolve().parent.parent\n"
        "class T(unittest.TestCase):\n"
        "    def test_writes(self):\n"
        "        (ROOT / 'tracked.txt').write_text('clobbered\\n')\n"
        "        self.assertTrue(True)\n")

DELETER = ("import pathlib, unittest\n"
           "ROOT = pathlib.Path(__file__).resolve().parent.parent\n"
           "class T(unittest.TestCase):\n"
           "    def test_deletes(self):\n"
           "        (ROOT / 'src' / 'proof.cpp').unlink()\n"
           "        self.assertTrue(True)\n")


class RepoCase(unittest.TestCase):
    def setUp(self):
        self.holder = tempfile.TemporaryDirectory()
        self.repo = Path(self.holder.name)
        (self.repo / "tools").mkdir()
        (self.repo / "src").mkdir()
        (self.repo / "tracked.txt").write_text("original\n")
        (self.repo / "src" / "proof.cpp").write_text(
            "// ORIGINAL: 0x00401000 BYTE_EXACT\n")

    def tearDown(self):
        self.holder.cleanup()

    def commit(self):
        for command in (["git", "init", "-q", "."],
                        ["git", "config", "user.email", "t@example.invalid"],
                        ["git", "config", "user.name", "t"],
                        ["git", "add", "-A"],
                        ["git", "commit", "-qm", "base"]):
            subprocess.run(command, cwd=self.repo, capture_output=True)

    def write_test(self, name, text):
        (self.repo / "tools" / name).write_text(text)

    def run_check(self, *extra):
        return checker.main(["--repo", str(self.repo), *extra])


class DetectionTest(RepoCase):
    def test_a_test_that_edits_a_tracked_file_fails_the_check(self):
        self.write_test("test_rude.py", RUDE)
        self.commit()
        self.assertEqual(self.run_check(), 1)

    def test_a_test_that_deletes_a_tracked_file_fails_the_check(self):
        """The deletion is the case that actually happened, and it is the one a
        `git status` glance is least likely to notice."""
        self.write_test("test_deleter.py", DELETER)
        self.commit()
        self.assertEqual(self.run_check(), 1)

    def test_a_well_behaved_test_passes(self):
        self.write_test("test_polite.py", POLITE)
        self.commit()
        self.assertEqual(self.run_check(), 0)

    def test_pycache_alone_is_not_a_violation(self):
        """Running any Python test writes `__pycache__`. Counting untracked
        paths reported EVERY test file as an offender and buried the real one -
        that was the first draft's behaviour, caught by its own damage case."""
        self.write_test("test_polite.py", POLITE)
        self.write_test("test_also_polite.py", POLITE)
        self.commit()
        self.assertEqual(self.run_check(), 0)

    def test_the_offender_is_named_and_the_innocent_is_not(self):
        self.write_test("test_polite.py", POLITE)
        self.write_test("test_rude.py", RUDE)
        self.commit()
        offenders, _, count = checker.sweep(
            self.repo, self.repo / "tools", "test_*.py", 600)
        self.assertEqual(count, 2)
        self.assertEqual([name for name, _ in offenders], ["test_rude.py"])


class RestorationTest(RepoCase):
    def test_a_clobbered_file_is_put_back(self):
        """The check must not leave the damage it found. A file that was clean
        at baseline is restorable to HEAD by definition."""
        self.write_test("test_rude.py", RUDE)
        self.commit()
        self.run_check()
        self.assertEqual((self.repo / "tracked.txt").read_text(), "original\n")

    def test_a_deleted_file_is_put_back(self):
        self.write_test("test_deleter.py", DELETER)
        self.commit()
        self.run_check()
        self.assertTrue((self.repo / "src" / "proof.cpp").is_file())

    def test_a_file_already_modified_is_left_alone(self):
        """Restoring a file the operator had already edited would destroy their
        work - a worse failure than the one being looked for. It is reported as
        UNATTRIBUTABLE instead, and the check still fails."""
        self.write_test("test_rude.py", RUDE)
        self.commit()
        (self.repo / "tracked.txt").write_text("the operator was here\n")
        status = self.run_check()
        self.assertEqual(status, 1)
        self.assertEqual((self.repo / "tracked.txt").read_text(),
                         "clobbered\n")


class VacuityTest(RepoCase):
    """A run that measured nothing must not report clean."""

    def test_no_matching_tests_is_a_failure_not_an_ok(self):
        self.commit()
        self.assertEqual(self.run_check(), 1)

    def test_a_missing_tests_directory_is_a_failure(self):
        self.commit()
        self.assertEqual(
            checker.main(["--repo", str(self.repo),
                          "--tests-dir", str(self.repo / "nope")]), 1)


if __name__ == "__main__":
    unittest.main()
