#!/usr/bin/env python3
"""The predicate that decides which processes get killed, previously untested.

`stop_executable_processes` reaps every PID whose command line
`command_runs_scenario_executable` accepts. A false positive there does not
produce a failed test - it kills something outside the build, which is why the
project's standing rule says never to issue a global Wine shutdown or terminate
unrelated processes. The staged executable carries a
`secrets.token_hex(16)` name precisely so this predicate can be exact, and
nothing checked that it is.

The negative cases matter more than the positive ones here, so they come first.
"""
import os
import unittest
from pathlib import Path

import runtime_process

EXE = Path("/tmp/staged/terranx_0123456789abcdef.exe")


class MustNotMatchTests(unittest.TestCase):
    """Every one of these would kill a process the harness does not own."""

    def test_an_unrelated_command_is_not_matched(self):
        self.assertFalse(runtime_process.command_runs_scenario_executable(
            "/usr/bin/firefox --profile /home/user", EXE))

    def test_a_longer_path_sharing_this_prefix_is_not_matched(self):
        # `...abcdef.exe.log` starts with the marker; only a boundary saves it.
        self.assertFalse(runtime_process.command_runs_scenario_executable(
            f"/usr/bin/less {EXE}.log", EXE))

    def test_a_different_token_is_not_matched(self):
        other = Path("/tmp/staged/terranx_fedcba9876543210.exe")
        self.assertFalse(runtime_process.command_runs_scenario_executable(
            f"/usr/bin/wine {other}", EXE))

    def test_a_non_wine_launcher_is_not_matched(self):
        # An editor or a debugger holding the path open is not the game.
        self.assertFalse(runtime_process.command_runs_scenario_executable(
            f"/usr/bin/vim {EXE}", EXE))

    def test_a_relative_launcher_is_not_matched(self):
        # The launcher is stat'ed, so it must be an absolute host path.
        self.assertFalse(runtime_process.command_runs_scenario_executable(
            f"wine {EXE}", EXE))


class MustMatchTests(unittest.TestCase):
    def test_the_bare_executable_is_matched(self):
        self.assertTrue(runtime_process.command_runs_scenario_executable(
            str(EXE), EXE))

    def test_the_executable_with_arguments_is_matched(self):
        self.assertTrue(runtime_process.command_runs_scenario_executable(
            f"{EXE} -windowed", EXE))

    def test_the_wine_z_drive_spelling_is_matched(self):
        windows = "Z:" + str(EXE).replace("/", "\\")
        self.assertTrue(runtime_process.command_runs_scenario_executable(
            windows, EXE))

    def test_case_differences_from_wine_are_tolerated(self):
        self.assertTrue(runtime_process.command_runs_scenario_executable(
            str(EXE).upper(), EXE))


class LauncherPrefixTests(unittest.TestCase):
    """A real wine launcher must be recognised, and it is stat'ed to do it.

    The casefold/slice split is load-bearing: the launcher prefix is sliced out
    of the ORIGINAL string rather than the casefolded one, because on a
    case-sensitive filesystem `/opt/Wine/bin/wine` casefolded does not exist,
    is_file() says no, and the game process goes unreaped. That is invisible on
    macOS and appeared the moment this ran on Linux.
    """

    def test_a_real_wine_binary_prefix_is_matched(self):
        wine = None
        for candidate in ("/usr/bin/wine", "/usr/bin/wine64"):
            if Path(candidate).is_file():
                wine = candidate
                break
        if wine is None:
            self.skipTest("no host wine binary to exercise the launcher path")
        self.assertTrue(runtime_process.command_runs_scenario_executable(
            f"{wine} {EXE} -windowed", EXE))

    def test_a_launcher_that_does_not_exist_is_not_matched(self):
        self.assertFalse(runtime_process.command_runs_scenario_executable(
            f"/nonexistent/bin/wine {EXE}", EXE))


class StagingTests(unittest.TestCase):
    def test_the_staged_name_carries_the_token(self):
        staged = runtime_process.scenario_executable_path(
            Path("/tmp/game/terranx.exe"), "deadbeef")
        self.assertIn("deadbeef", staged.name)
        self.assertEqual(".exe", staged.suffix)

    def test_two_tokens_give_two_names(self):
        # One lane must not reap the other's game; distinct names are how.
        first = runtime_process.scenario_executable_path(
            Path("/tmp/game/terranx.exe"), "aaaa")
        second = runtime_process.scenario_executable_path(
            Path("/tmp/game/terranx.exe"), "bbbb")
        self.assertNotEqual(first, second)

    def test_matching_never_reports_the_current_process(self):
        # matching_scenario_process_ids excludes os.getpid() explicitly; if it
        # stopped doing so the harness could signal itself.
        if os.name == "nt":
            self.skipTest("the scan is POSIX-only")
        self.assertNotIn(os.getpid(),
                         runtime_process.matching_scenario_process_ids(EXE))


if __name__ == "__main__":
    unittest.main()
