#!/usr/bin/env python3

import os
from pathlib import Path
import stat
import subprocess
import tempfile
import unittest
from unittest import mock

import verify_build_freshness as freshness


CTEST_FILE = '''\
add_test(alpha "/usr/bin/python3" "/repo/tools/run_windows_test.py" \
"--executable" "/bd/recovery-leaf-tests.exe")
add_test(beta "/bd/globals-diff-tests.exe")
add_test(gamma "/usr/bin/python3" "/repo/tools/audit.py" "--check")
'''

RERUN_LINE = ("build build.ninja /bd/CTestTestfile.cmake: RERUN_CMAKE "
              "/bd/CMakeFiles/cmake.verify_globs | /repo/CMakeLists.txt "
              "CMakeCache.txt\n")


class ParsingTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def test_executables_are_derived_from_the_ctest_file(self):
        build = self.root / "bd"
        build.mkdir()
        ctest = build / "CTestTestfile.cmake"
        ctest.write_text(CTEST_FILE.replace("/bd/", f"{build}/"),
                         encoding="utf-8")
        found = freshness.ctest_executables(ctest, build)
        self.assertEqual(found, ["globals-diff-tests.exe",
                                 "recovery-leaf-tests.exe"])

    def test_a_python_only_test_contributes_no_executable(self):
        build = self.root / "bd"
        build.mkdir()
        ctest = build / "CTestTestfile.cmake"
        ctest.write_text('add_test(gamma "/usr/bin/python3" "/repo/a.py")\n',
                         encoding="utf-8")
        self.assertEqual(freshness.ctest_executables(ctest, build), [])

    def test_regeneration_inputs_come_from_the_manifest(self):
        build = self.root / "bd"
        build.mkdir()
        (build / "build.ninja").write_text(
            "rule RERUN_CMAKE\n" + RERUN_LINE, encoding="utf-8")
        inputs = freshness.regeneration_inputs(build)
        self.assertEqual(inputs, ["/repo/CMakeLists.txt", "CMakeCache.txt"])

    def test_absent_rerun_edge_is_reported_not_assumed_fine(self):
        build = self.root / "bd"
        build.mkdir()
        (build / "build.ninja").write_text("build all: phony\n",
                                           encoding="utf-8")
        self.assertIsNone(freshness.regeneration_inputs(build))


class StubNinjaTests(unittest.TestCase):
    """A stub ninja lets the three checks be exercised without a real build."""

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.build = self.root / "bd"
        self.build.mkdir()
        (self.build / "build.ninja").write_text(
            "rule RERUN_CMAKE\n"
            f"build build.ninja: RERUN_CMAKE x | {self.root}/CMakeLists.txt\n",
            encoding="utf-8")
        (self.root / "CMakeLists.txt").write_text("x\n", encoding="utf-8")
        os.utime(self.root / "CMakeLists.txt", (0, 0))
        self.ctest = self.build / "CTestTestfile.cmake"
        self.ctest.write_text(
            f'add_test(a "{self.build}/leaf.exe")\n', encoding="utf-8")

    def stub(self, body):
        path = self.root / "ninja-stub"
        path.write_text("#!/bin/sh\n" + body, encoding="utf-8")
        path.chmod(path.stat().st_mode | stat.S_IEXEC)
        return path

    def run_tool(self, ninja):
        argv = ["verify_build_freshness.py",
                "--build-dir", str(self.build),
                "--ctest-file", str(self.ctest),
                "--ninja", str(ninja)]
        with mock.patch("sys.argv", argv):
            return freshness.main()

    def test_a_ninja_that_always_succeeds_is_caught_as_blind(self):
        # THE point of the self-control. A plain `ninja -n` in this project
        # exits 0 without parsing the target list, because file(GLOB
        # CONFIGURE_DEPENDS) leaves build.ninja permanently dirty. A freshness
        # answer from that is worthless, and must not read as "up to date".
        self.assertEqual(self.run_tool(self.stub('exit 0\n')), 1)

    def test_up_to_date_passes(self):
        self.assertEqual(self.run_tool(self.stub(
            'case "$*" in *no-such-target*) exit 1;; esac\n'
            'echo "ninja: no work to do."\n')), 0)

    def test_pending_work_fails_even_though_ninja_exits_zero(self):
        # A dry run with work to do also exits 0, having merely PRINTED the
        # commands. Exit status is not the signal; the phrase is.
        self.assertEqual(self.run_tool(self.stub(
            'case "$*" in *no-such-target*) exit 1;; esac\n'
            'echo "[1/2] Building CXX object foo.obj"\n')), 1)

    def test_no_artifacts_to_check_fails_rather_than_passing_vacuously(self):
        self.ctest.write_text('add_test(a "/usr/bin/python3" "x.py")\n',
                              encoding="utf-8")
        self.assertEqual(self.run_tool(self.stub(
            'case "$*" in *no-such-target*) exit 1;; esac\n'
            'echo "ninja: no work to do."\n')), 1)

    def test_a_configure_input_newer_than_the_manifest_fails(self):
        os.utime(self.root / "CMakeLists.txt", None)   # now, i.e. newest
        self.assertEqual(self.run_tool(self.stub(
            'case "$*" in *no-such-target*) exit 1;; esac\n'
            'echo "ninja: no work to do."\n')), 1)

    def test_missing_manifest_fails(self):
        (self.build / "build.ninja").unlink()
        self.assertEqual(self.run_tool(self.stub('exit 1\n')), 1)


class RealNinjaTests(unittest.TestCase):
    """Against this checkout's own build directories, if they exist."""

    def test_absolute_manifest_path_defeats_the_blind_mode(self):
        repo = Path(freshness.__file__).resolve().parent.parent
        candidates = sorted((repo / "build").glob("*/build.ninja"))
        if not candidates:
            self.skipTest("no configured build directory to query")
        candidate = candidates[0].parent
        blind = subprocess.run(
            ["ninja", "-C", str(candidate), "-n", freshness.IMPOSSIBLE_TARGET],
            capture_output=True, text=True)
        guarded = freshness.ninja("ninja", candidate,
                                  [freshness.IMPOSSIBLE_TARGET])
        # If ninja ever stops being blind, `blind` starts failing too and this
        # assertion is what says so, rather than the tool silently relying on a
        # workaround it no longer needs.
        self.assertEqual(blind.returncode, 0,
                         "plain `ninja -n` no longer swallows unknown targets; "
                         "revisit the absolute-manifest workaround")
        self.assertNotEqual(guarded.returncode, 0,
                            "the absolute manifest path no longer suppresses "
                            "manifest regeneration")


if __name__ == "__main__":
    unittest.main()
