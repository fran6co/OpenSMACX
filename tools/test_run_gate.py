#!/usr/bin/env python3

"""The gate wrapper's own failure modes.

A wrapper that runs the gate is only useful if a failing lane cannot be
mistaken for a passing one, and if concurrency is a choice rather than a
requirement - a machine that cannot afford two lanes still has to be able to
gate.
"""

import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import run_gate


class FakeProcess:
    """Exits after `polls` observations, so a lane's own elapsed can be seen."""

    def __init__(self, returncode, output, log, polls=0):
        self.returncode = returncode
        self._output = output
        self._log = log
        self._polls = polls

    def _exit(self):
        self._log.write(self._output)
        return self.returncode

    def wait(self):
        return self._exit()

    def poll(self):
        if self._polls > 0:
            self._polls -= 1
            return None
        return self._exit()


class RunGateTest(unittest.TestCase):
    def setUp(self):
        # These tests are about LANE SCHEDULING. Configuring a build directory
        # is a separate concern with its own case below, and letting it run
        # here would shell out to cmake from a unit test.
        self._configure = run_gate.Lane.configure
        run_gate.Lane.configure = lambda self: None

    def tearDown(self):
        run_gate.Lane.configure = self._configure

    def _run(self, argv, outcomes):
        """outcomes: lane dir name -> (returncode, log text). Returns (rc, stdout)."""
        starts = []

        def fake_popen(command, cwd, stdout, stderr, **_ignored):
            preset = Path(command[command.index("--build") + 1]).name
            starts.append(preset)
            returncode, output = outcomes[preset]
            return FakeProcess(returncode, output, stdout)

        with tempfile.TemporaryDirectory() as directory:
            argv = ["run_gate.py", "--log-dir", directory] + argv
            with mock.patch("sys.argv", argv), \
                    mock.patch.object(subprocess, "Popen", fake_popen), \
                    mock.patch.object(run_gate, "tracked_manifest",
                                      lambda _repository: {}), \
                    mock.patch("sys.stdout", new_callable=_Capture) as out:
                code = run_gate.main()
            return code, out.text, starts

    def test_both_lanes_pass(self):
        code, text, starts = self._run([], {
            "vc6-debug": (0, "100% tests passed out of 59\n"),
            "vc6-release": (0, "100% tests passed out of 59\n"),
        })
        self.assertEqual(0, code)
        self.assertEqual(("vc6-debug", "vc6-release"), tuple(starts))
        self.assertIn("GATE PASSED", text)
        # The verdict has to be visible without opening the logs.
        self.assertEqual(2, text.count("100% tests passed out of 59"))

    def test_one_failing_lane_fails_the_gate_and_is_named(self):
        code, text, _ = self._run([], {
            "vc6-debug": (0, "100% tests passed out of 59\n"),
            "vc6-release": (8, "The following tests FAILED:\n"
                                      "\t3 - recovery-gameplay-tests\n"),
        })
        self.assertEqual(1, code)
        self.assertIn("GATE FAILED: Release", text)
        self.assertNotIn("GATE PASSED", text)

    def test_a_lane_that_produced_no_verdict_still_shows_its_tail(self):
        # A build failure never reaches CTest, so there is no "tests passed"
        # line to echo. Silence there would read as success.
        code, text, _ = self._run([], {
            "vc6-debug": (2, "ninja: build stopped: subcommand failed.\n"),
            "vc6-release": (0, "100% tests passed out of 59\n"),
        })
        self.assertEqual(1, code)
        self.assertIn("ninja: build stopped", text)

    def test_one_lane_alone(self):
        code, text, starts = self._run(["--build-type", "Debug"], {
            "vc6-debug": (0, "100% tests passed out of 59\n"),
        })
        self.assertEqual(0, code)
        self.assertEqual(["vc6-debug"], starts)
        self.assertIn("1 lane(s) serially", text)

    def test_serial_mode_starts_the_second_lane_after_the_first(self):
        order = []

        def fake_popen(command, cwd, stdout, stderr, **_ignored):
            preset = Path(command[command.index("--build") + 1]).name
            order.append(f"start {preset}")
            process = FakeProcess(0, "100% tests passed out of 59\n", stdout)
            wait = process.wait

            def logged():
                order.append(f"wait {preset}")
                return wait()

            process.wait = logged
            return process

        with tempfile.TemporaryDirectory() as directory:
            argv = ["run_gate.py", "--serial", "--log-dir", directory]
            with mock.patch("sys.argv", argv), \
                    mock.patch.object(subprocess, "Popen", fake_popen), \
                    mock.patch.object(run_gate, "tracked_manifest",
                                      lambda _repository: {}), \
                    mock.patch("sys.stdout", new_callable=_Capture):
                self.assertEqual(0, run_gate.main())
        self.assertEqual(["start vc6-debug", "wait vc6-debug",
                          "start vc6-release", "wait vc6-release"],
                         order)

    def test_concurrent_mode_starts_both_lanes_before_either_finishes(self):
        order = []

        def fake_popen(command, cwd, stdout, stderr, **_ignored):
            preset = Path(command[command.index("--build") + 1]).name
            order.append(f"start {preset}")
            process = FakeProcess(0, "100% tests passed out of 59\n", stdout,
                                  polls=1)
            exit_ = process._exit

            def logged():
                order.append(f"done {preset}")
                return exit_()

            process._exit = logged
            return process

        with tempfile.TemporaryDirectory() as directory:
            argv = ["run_gate.py", "--log-dir", directory]
            with mock.patch("sys.argv", argv), \
                    mock.patch.object(subprocess, "Popen", fake_popen), \
                    mock.patch.object(run_gate, "tracked_manifest",
                                      lambda _repository: {}), \
                    mock.patch.object(run_gate.time, "sleep", lambda _: None), \
                    mock.patch("sys.stdout", new_callable=_Capture):
                self.assertEqual(0, run_gate.main())
        self.assertEqual(["start vc6-debug", "start vc6-release",
                          "done vc6-debug", "done vc6-release"],
                         order)

    def test_a_lane_is_timed_when_it_ends_not_when_it_is_reaped(self):
        """The debug lane exits first; its elapsed must not include the wait
        for the release lane. Two lanes reporting the same elapsed is the
        signature of `a.wait(); b.wait()` and hides the cost of concurrency."""
        clock = [0.0]

        def fake_popen(command, cwd, stdout, stderr, **_ignored):
            preset = Path(command[command.index("--build") + 1]).name
            polls = 1 if preset == "vc6-debug" else 5
            return FakeProcess(0, "100% tests passed out of 59\n", stdout,
                               polls=polls)

        lanes = []
        original_report = run_gate.report
        with tempfile.TemporaryDirectory() as directory:
            argv = ["run_gate.py", "--log-dir", directory]
            with mock.patch("sys.argv", argv), \
                    mock.patch.object(subprocess, "Popen", fake_popen), \
                    mock.patch.object(run_gate, "tracked_manifest",
                                      lambda _repository: {}), \
                    mock.patch.object(run_gate.time, "sleep",
                                      lambda seconds: clock.__setitem__(
                                          0, clock[0] + seconds)), \
                    mock.patch.object(run_gate.time, "monotonic",
                                      lambda: clock[0]), \
                    mock.patch.object(run_gate, "report",
                                      lambda lane: (lanes.append(
                                          (lane.directory.name, lane.elapsed)),
                                          original_report(lane))[1]), \
                    mock.patch("sys.stdout", new_callable=_Capture):
                self.assertEqual(0, run_gate.main())
        elapsed = dict(lanes)
        self.assertLess(elapsed["vc6-debug"],
                        elapsed["vc6-release"])

    def test_the_default_target_does_not_promote_metadata(self):
        # promote-recovery-metadata writes docs/recovery/ in the SOURCE tree,
        # which is the one thing two concurrent lanes must not both do. The
        # gate target's outputs are all under the preset's binary dir.
        lane = run_gate.Lane("Debug", run_gate.TARGET, ".", ".")
        self.assertEqual(
            ["cmake", "--build", str(lane.directory),
             "--target", "verify-recovery-batch"], lane.command)


class _Capture:
    def __init__(self):
        self.text = ""

    def write(self, data):
        self.text += data
        return len(data)

    def flush(self):
        pass


class TreeWriteDetectionTests(unittest.TestCase):
    """The defect restores what it wrote, so only mtime can see it."""

    def test_an_unchanged_tree_is_silent(self):
        manifest = {"src/a.cpp": (1, 10), "docs/recovery/functions.csv": (2, 20)}
        self.assertEqual(run_gate.report_tree_writes(manifest, dict(manifest)), 0)

    def test_metadata_written_during_the_run_fails(self):
        before = {"docs/recovery/functions.csv": (2, 20)}
        after = {"docs/recovery/functions.csv": (3, 20)}   # restored, mtime moved
        self.assertEqual(run_gate.report_tree_writes(before, after), 1)

    def test_same_size_restored_content_is_still_caught(self):
        # A content hash would see nothing here; that is the whole point.
        before = {"docs/recovery/summary.json": (100, 512)}
        after = {"docs/recovery/summary.json": (200, 512)}
        self.assertEqual(run_gate.report_tree_writes(before, after), 1)

    def test_a_source_edit_warns_but_does_not_fail(self):
        # Another agent editing src/ mid-gate is normal here, and failing on it
        # would make the gate flaky - worse than no check.
        before = {"src/veh.cpp": (1, 10)}
        after = {"src/veh.cpp": (2, 11)}
        self.assertEqual(run_gate.report_tree_writes(before, after), 0)

    def test_a_new_untracked_file_is_not_a_write(self):
        before = {"src/a.cpp": (1, 10)}
        after = {"src/a.cpp": (1, 10), "build/x.o": (5, 5)}
        self.assertEqual(run_gate.report_tree_writes(before, after), 0)

    def test_an_unlistable_tree_says_so_instead_of_reporting_clean(self):
        # Returning an empty manifest would compare nothing with nothing and
        # report a clean tree forever.
        self.assertEqual(run_gate.report_tree_writes(None, {"a": (1, 1)}), 0)


class ConfigureTest(unittest.TestCase):
    """A lane configures its own directory, and never re-configures one.

    The re-configure case is the one that matters. CMAKE_CXX_FLAGS is a cache
    variable, so a directory configured with the wrong flags cannot be
    repaired by configuring it again - `/Zm1000` survived that way and failed
    137 objects with C1060 long after the fix had landed. If this ever starts
    reconfiguring, that failure becomes silent again.
    """

    def setUp(self):
        self.work = Path(tempfile.mkdtemp())
        self.calls = []
        self.run = run_gate.subprocess.run
        run_gate.subprocess.run = lambda *a, **k: self.calls.append(a[0])

    def tearDown(self):
        run_gate.subprocess.run = self.run

    def test_it_configures_a_directory_that_does_not_exist(self):
        lane = run_gate.Lane("Debug", run_gate.TARGET, self.work, self.work)
        lane.configure()
        self.assertEqual(len(self.calls), 1)
        self.assertIn("-DCMAKE_BUILD_TYPE=Debug", self.calls[0])
        self.assertIn(str(lane.directory), self.calls[0])

    def test_it_leaves_an_existing_cache_alone(self):
        lane = run_gate.Lane("Release", run_gate.TARGET, self.work, self.work)
        lane.directory.mkdir(parents=True)
        (lane.directory / "CMakeCache.txt").write_text("")
        lane.configure()
        self.assertEqual(self.calls, [],
                         "a configured lane must not be reconfigured")

    def test_the_two_lanes_use_separate_directories(self):
        one = run_gate.Lane("Debug", run_gate.TARGET, self.work, self.work)
        two = run_gate.Lane("Release", run_gate.TARGET, self.work, self.work)
        self.assertNotEqual(one.directory, two.directory)
        self.assertNotEqual(one.log, two.log)


if __name__ == "__main__":
    unittest.main()
