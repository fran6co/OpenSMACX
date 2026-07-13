#!/usr/bin/env python3

import os
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from run_gameplay_scenario import (
    command_runs_scenario_executable,
    cleanup_owned_processes,
    fatal_diagnostics,
    matching_scenario_process_ids,
    resolve_fixture,
    scenario_executable_path,
    stage_scenario_executable,
    validate_report,
    wine_path,
)


class GameplayScenarioToolTests(unittest.TestCase):
    def test_fixture_must_be_a_regular_file_inside_game_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            game_dir = Path(directory) / "game"
            game_dir.mkdir()
            fixture = game_dir / "scenarios" / "fixture.SC"
            fixture.parent.mkdir()
            fixture.write_bytes(b"fixture")

            self.assertEqual(resolve_fixture(game_dir, "scenarios/fixture.SC"), fixture.resolve())
            with self.assertRaisesRegex(RuntimeError, "outside"):
                resolve_fixture(game_dir, "../fixture.SC")
            with self.assertRaisesRegex(RuntimeError, "not found"):
                resolve_fixture(game_dir, "missing.SC")

    def test_wine_path_uses_the_z_drive(self):
        self.assertEqual(wine_path(Path("/tmp/game/result.json")),
                         "Z:\\tmp\\game\\result.json")

    def test_report_requires_the_requested_success_state(self):
        inspection = {
            "status": "inspected", "turn": 12, "current_faction": 5,
            "vehicle_count": 4,
            "candidate": {"id": 3, "x": 2, "y": 4, "prototype": 1,
                          "triad": 0, "moves": 3},
            "adjacent": [{"x": 3, "y": 5, "occupied": -1, "cost": 3,
                          "ocean": False, "base": False}],
        }
        validate_report(inspection, True)
        execution = {
            "status": "passed", "movement_ordered": True,
            "end_turn_requested": True, "turn": 12, "vehicle": 3,
            "start": [2, 4], "target": [3, 5],
        }
        validate_report(execution, False, 3, 3, 5)
        resolved = dict(execution, movement_resolved=True, order_cleared=True,
                        movement_cost=3)
        validate_report(resolved, False, 3, 3, 5, True)
        with self.assertRaisesRegex(RuntimeError, "resolved movement"):
            validate_report(execution, False, 3, 3, 5, True)
        advanced = dict(resolved, turn_advanced=True, initial_turn=12,
                        advanced_turn=13, mission_year=2173)
        validate_report(advanced, False, 3, 3, 5, True, True)
        with self.assertRaisesRegex(RuntimeError, "turn advancement"):
            validate_report(resolved, False, 3, 3, 5, True, True)
        with self.assertRaisesRegex(RuntimeError, "failed"):
            validate_report({"status": "failed", "error": "load_daemon_failed"}, False)
        with self.assertRaisesRegex(RuntimeError, "expected passing"):
            validate_report(execution, False, 3, 4, 5)
        malformed = dict(execution, vehicle=True, target=[True, False])
        with self.assertRaisesRegex(RuntimeError, "expected passing"):
            validate_report(malformed, False, 1, 1, 0)
        with self.assertRaisesRegex(RuntimeError, "complete inspection"):
            validate_report({"status": "passed"}, True)

    def test_process_ownership_uses_a_tokenized_executable_name(self):
        executable = Path("/game/terranx_hybrid.exe")
        self.assertEqual(
            scenario_executable_path(executable, "abc123"),
            Path("/game/.terranx_hybrid-scenario-abc123.exe"),
        )
        alias = scenario_executable_path(executable, "abc123")
        self.assertTrue(command_runs_scenario_executable(
            f"Z:\\game\\{alias.name} ", alias))
        with tempfile.TemporaryDirectory() as directory:
            launcher = Path(directory) / "Wine App" / "wine"
            launcher.parent.mkdir()
            launcher.touch()
            self.assertTrue(command_runs_scenario_executable(
                f"{launcher} {alias}", alias))
        self.assertFalse(command_runs_scenario_executable(
            f"python inspect.py {alias}", alias))
        self.assertFalse(command_runs_scenario_executable(
            f"/usr/bin/python /tmp/wine {alias}", alias))

    @patch("runtime_process.subprocess.run")
    def test_process_matcher_excludes_current_and_unrelated_pids(self, run):
        executable = Path("/game/.terranx_hybrid-scenario-abc123.exe")
        with tempfile.TemporaryDirectory() as directory:
            launcher = Path(directory) / "wine"
            launcher.touch()
            run.return_value = SimpleNamespace(stdout=(
                f"  42 {launcher} {executable}\n"
                f"{os.getpid()} Z:\\game\\{executable.name}\n"
                f"  77 python inspect.py {executable}\n"))
            self.assertEqual(matching_scenario_process_ids(executable), {42})

    @patch("run_gameplay_scenario.stop_owned_wine_prefix")
    @patch("run_gameplay_scenario.stop_executable_processes", return_value=True)
    def test_cleanup_waits_for_every_alias_process(self, stop_processes, stop_prefix):
        self.assertTrue(cleanup_owned_processes(
            Path("/game/.terranx_hybrid-scenario-abc123.exe"), None,
            "wine", Path("/prefix")))
        stop_prefix.assert_called_once_with(Path("/prefix"), "wine")
        stop_processes.assert_called_once()

    def test_scenario_executable_is_staged_under_its_tokenized_name(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "terranx_hybrid.exe"
            executable.write_bytes(b"hybrid")
            staged = stage_scenario_executable(executable, "abc123")
            try:
                self.assertEqual(staged.read_bytes(), b"hybrid")
                self.assertEqual(staged, scenario_executable_path(executable, "abc123"))
            finally:
                staged.unlink(missing_ok=True)

    def test_fatal_diagnostics_reject_unhandled_exceptions(self):
        self.assertEqual(fatal_diagnostics("normal output\n"), [])
        self.assertEqual(
            fatal_diagnostics("normal output\nwine: Unhandled page fault\n"),
            ["wine: Unhandled page fault"],
        )


if __name__ == "__main__":
    unittest.main()
