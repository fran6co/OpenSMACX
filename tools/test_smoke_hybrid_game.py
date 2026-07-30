#!/usr/bin/env python3

import re
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from owned_wine_prefix import claim_owned_wine_prefix, prepare_owned_wine_prefix
from movie_skip import (
    configure_intro_movie_skip,
    restore_intro_movie_config,
)
from smoke_hybrid_game import (
    analyze_diagnostics,
    parse_generated_verdicts,
    prepare_runtime_oracle_result_path,
    validate_runtime_oracles,
    validate_smoke,
)
from setup_game import stage_pracx

# Every suite src/runtime_oracle.cpp registers, phase one then deferred. The
# gate must expect all of them by name; a suite the gate does not name is a
# suite the gate stops checking, which is how the generated-signatures hole
# stayed open.
REGISTERED_SUITES = (
    "constructor", "basebutton", "scroll", "win", "sprite", "buffer",
    "graphicwin", "stringstruct", "spying",
    "basebutton-release", "sprite-release", "buffer-release",
    "generated-signatures",
)


class AnalyzeDiagnosticsTests(unittest.TestCase):
    SUCCESSFUL_LOADS = """
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\terranx_hybrid.exe" at 00400000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\OpenSMACX.dll" at 7A1B0000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\prax.dll" at 7A0B0000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\DDRAW.dll" at 7AD00000: builtin
"""

    def test_accepts_expected_modules_and_rendering(self):
        result = analyze_diagnostics(
            self.SUCCESSFUL_LOADS
            + "00d4:fixme:ddraw:ddraw_surface1_Flip Ignoring flags 0x1.\n",
            "terranx_hybrid.exe")
        self.assertEqual(result["fatal_lines"], [])
        self.assertEqual(result["loader_failure_lines"], [])
        self.assertEqual(result["missing_markers"], [])
        self.assertTrue(result["rendering_started"])

    def test_rejects_native_ddraw(self):
        result = analyze_diagnostics("""
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\terranx_hybrid.exe" at 00400000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\OpenSMACX.dll" at 7A1B0000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\prax.dll" at 7A0B0000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\DDRAW.dll" at 7AD00000: native
""", "terranx_hybrid.exe")
        self.assertIn("builtin_ddraw", result["missing_markers"])

    def test_rejects_builtin_ddraw_loaded_by_another_loader_context(self):
        result = analyze_diagnostics("""
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\terranx_hybrid.exe" at 00400000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\OpenSMACX.dll" at 7A1B0000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\prax.dll" at 7A0B0000: native
00d4:trace:loaddll:build_module Loaded L"Z:\\game\\DDRAW.dll" at 7AD00000: native
00e8:trace:loaddll:build_module Loaded L"C:\\windows\\system32\\DDRAW.dll" at 7AD00000: builtin
""", "terranx_hybrid.exe")
        self.assertIn("builtin_ddraw", result["missing_markers"])

    def test_detects_unhandled_exception(self):
        result = analyze_diagnostics(
            "err:seh:NtRaiseException Unhandled exception code c0000409",
            "terranx_hybrid.exe")
        self.assertEqual(len(result["fatal_lines"]), 1)

    def test_accepts_surviving_hybrid_without_flip_trace(self):
        result = analyze_diagnostics(
            self.SUCCESSFUL_LOADS, "terranx_hybrid.exe")
        self.assertEqual(validate_smoke(result, [123]), "process_survival")

    def test_rejects_missing_flip_when_game_exited(self):
        result = analyze_diagnostics(
            self.SUCCESSFUL_LOADS, "terranx_hybrid.exe")
        with self.assertRaisesRegex(RuntimeError, "did not survive"):
            validate_smoke(result, [])

    def test_rejects_failure_text_that_mentions_required_modules(self):
        result = analyze_diagnostics("""
OpenSMACX.dll failed to load
prax.dll module not found
DDRAW.dll builtin load failed
terranx_hybrid.exe could not load imports
""", "terranx_hybrid.exe")
        self.assertEqual(
            result["missing_markers"],
            ["executable", "opensmacx", "pracx", "builtin_ddraw"])
        self.assertGreaterEqual(len(result["loader_failure_lines"]), 3)
        with self.assertRaisesRegex(RuntimeError, "load failure"):
            validate_smoke(result, [123])

    def test_requires_passing_runtime_oracles(self):
        with tempfile.TemporaryDirectory() as directory:
            result = Path(directory) / "runtime-oracle.txt"
            with self.assertRaisesRegex(RuntimeError, "did not produce"):
                validate_runtime_oracles(result)
            result.write_text("scroll failed\n", encoding="ascii")
            with self.assertRaisesRegex(RuntimeError, "suites failed: scroll"):
                validate_runtime_oracles(result)
            result.write_text("scroll passed\n", encoding="ascii")
            with self.assertRaisesRegex(RuntimeError, "all passed"):
                validate_runtime_oracles(result)
            result.write_text("all passed\n", encoding="ascii")
            with self.assertRaisesRegex(RuntimeError, "no suites"):
                validate_runtime_oracles(result)
            result.write_text(
                "scroll passed\ncheckbox passed\nall passed\n",
                encoding="ascii")
            self.assertEqual(
                validate_runtime_oracles(result),
                {"scroll": "passed", "checkbox": "passed"})

    def test_a_result_missing_an_EXPECTED_suite_is_refused(self):
        with tempfile.TemporaryDirectory() as directory:
            result = Path(directory) / "runtime-oracle.txt"
            result.write_text("scroll passed\nall passed\n", encoding="ascii")
            # Terminator present, suite absent: the exact shape the gate passed
            # on for the 108 generated oracles.
            with self.assertRaisesRegex(
                    RuntimeError, "never reported: generated-signatures"):
                validate_runtime_oracles(
                    result, ["scroll", "generated-signatures"])
            self.assertEqual(
                validate_runtime_oracles(result, ["scroll"]),
                {"scroll": "passed"})

    def test_THE_LAST_GREEN_GATES_OWN_RESULT_FILE_is_now_refused(self):
        # The positive control for this whole change, and it is not synthetic:
        # this is the file the last passing gate wrote, byte for byte. It lists
        # twelve suites and `all passed`, and `generated-signatures` - 108
        # oracles, zero verdict lines in the log beside it - is not among them.
        # If this ever passes again, the check has been defeated.
        text = (
            "constructor passed\nbasebutton passed\nscroll passed\n"
            "win passed\nsprite passed\nbuffer passed\ngraphicwin passed\n"
            "stringstruct passed\nspying passed\nbasebutton-release passed\n"
            "sprite-release passed\nbuffer-release passed\nall passed\n")
        with tempfile.TemporaryDirectory() as directory:
            result = Path(directory) / "runtime-oracle.txt"
            result.write_text(text, encoding="ascii")
            with self.assertRaisesRegex(
                    RuntimeError, "never reported: generated-signatures"):
                validate_runtime_oracles(result, REGISTERED_SUITES)

    def test_a_result_ending_in_DEFERRED_PENDING_is_refused(self):
        with tempfile.TemporaryDirectory() as directory:
            result = Path(directory) / "runtime-oracle.txt"
            result.write_text("scroll passed\ndeferred pending\n",
                              encoding="ascii")
            with self.assertRaisesRegex(
                    RuntimeError, "deferred suites never ran"):
                validate_runtime_oracles(result)

    def test_a_result_ending_in_REPORT_OVERFLOW_is_refused(self):
        with tempfile.TemporaryDirectory() as directory:
            result = Path(directory) / "runtime-oracle.txt"
            result.write_text("scroll passed\nreport-overflow\n",
                              encoding="ascii")
            with self.assertRaisesRegex(RuntimeError, "overflowed its buffer"):
                validate_runtime_oracles(result)

    def test_generated_verdict_lines_are_parsed_out_of_the_log(self):
        log = (
            "  running ?main_caption@MapWin@@QAEXXZ\n"
            "GENERATED-ORACLE-VERDICT: 0x0046FB10 PASS ?main_caption@MapWin\n"
            "GENERATED-ORACLE-VERDICT: 0x004456A0 INCONCLUSIVE-no-effect ?x\n"
            "unrelated line\n")
        self.assertEqual(
            parse_generated_verdicts(log),
            {0x0046FB10: "PASS",
             0x004456A0: "INCONCLUSIVE-no-effect"})

    def test_a_verdict_state_tally_counts_each_state(self):
        # The shape recorded into the report JSON on every run, passing or not.
        log = "\n".join(
            f"GENERATED-ORACLE-VERDICT: 0x004{i:05X} "
            + ("PASS" if i < 2 else "INCONCLUSIVE-no-effect") + " ?f"
            for i in range(17))
        verdicts = parse_generated_verdicts(log)
        states = {}
        for state in verdicts.values():
            states[state] = states.get(state, 0) + 1
        self.assertEqual(states, {"PASS": 2, "INCONCLUSIVE-no-effect": 15})

    def test_no_verdict_lines_parses_to_nothing_rather_than_failing(self):
        # The state the tree is in today: the suite never ran, so the log has
        # none. That must read as zero verdicts, which --expect-verdicts then
        # refuses, rather than as a parse error somewhere else.
        self.assertEqual(parse_generated_verdicts("err:sync:something\n"), {})

    def test_the_EXPECTED_suite_set_matches_the_one_the_DLL_registers(self):
        # The drift guard. If someone registers a suite in runtime_oracle.cpp
        # and does not add it to the gate's expectation, the gate silently stops
        # checking it - which is how this defect existed in the first place.
        source = (Path(__file__).resolve().parent.parent / "src"
                  / "runtime_oracle.cpp").read_text(encoding="utf-8")
        registered = re.findall(r'\{"([a-z-]+)", run_\w+\}', source)
        self.assertEqual(sorted(registered), sorted(REGISTERED_SUITES))
        cmake = (Path(__file__).resolve().parent.parent
                 / "CMakeLists.txt").read_text(encoding="utf-8")
        expected = re.findall(r'--expect-oracle-suite=([a-z-]+)', cmake)
        self.assertEqual(sorted(expected), sorted(REGISTERED_SUITES))

    def test_runtime_oracle_result_rejects_symlink_without_removing_target(self):
        build_root = Path(__file__).resolve().parent.parent / "build"
        with tempfile.TemporaryDirectory(dir=build_root) as directory:
            root = Path(directory)
            target = root / "target.txt"
            target.write_text("preserved\n", encoding="ascii")
            link = root / "result.txt"
            link.symlink_to(target)
            with self.assertRaisesRegex(RuntimeError, "must not contain symlinks"):
                prepare_runtime_oracle_result_path(link)
            self.assertEqual(target.read_text(encoding="ascii"), "preserved\n")

    def test_runtime_oracle_result_rejects_nonlocal_path(self):
        result = (Path(__file__).resolve().parent.parent.parent
                  / "outside-runtime-oracle-result.txt")
        with self.assertRaisesRegex(RuntimeError, "inside the ignored"):
            prepare_runtime_oracle_result_path(result)

    def test_rejects_wine_import_error_after_successful_load_records(self):
        result = analyze_diagnostics(
            self.SUCCESSFUL_LOADS
            + "00d4:err:module:import_dll Library OpenSMACX.dll not found\n",
            "terranx_hybrid.exe")
        self.assertEqual(result["missing_markers"], [])
        with self.assertRaisesRegex(RuntimeError, "load failure"):
            validate_smoke(result, [123])

    def test_ignores_optional_loader_fallback_failure(self):
        result = analyze_diagnostics(
            self.SUCCESSFUL_LOADS
            + "00d4:warn:vulkan: failed to load optional Vulkan driver; using OpenGL\n",
            "terranx_hybrid.exe")
        self.assertEqual(result["loader_failure_lines"], [])

    def test_ignores_import_error_from_another_loader_context(self):
        result = analyze_diagnostics(
            self.SUCCESSFUL_LOADS
            + "00e8:err:module:import_dll Library child-only.dll not found\n",
            "terranx_hybrid.exe")
        self.assertEqual(result["loader_failure_lines"], [])

    def test_ignores_optional_module_error_from_game_context(self):
        result = analyze_diagnostics(
            self.SUCCESSFUL_LOADS
            + "00d4:err:module:load_dll Failed to load optional graphics-helper.dll\n",
            "terranx_hybrid.exe")
        self.assertEqual(result["loader_failure_lines"], [])

    def test_owned_prefix_rejects_an_unmarked_existing_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            prefix = Path(directory).resolve() / "prefix"
            prefix.mkdir()
            with self.assertRaisesRegex(RuntimeError, "unmarked"):
                claim_owned_wine_prefix(prefix)

    def test_owned_prefix_rejects_a_symlinked_ancestor(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            target = root / "target"
            target.mkdir()
            link = root / "link"
            link.symlink_to(target, target_is_directory=True)
            with self.assertRaisesRegex(RuntimeError, "must not contain symlinks"):
                claim_owned_wine_prefix(link / "prefix")

    @patch("owned_wine_prefix.subprocess.run")
    @patch("owned_wine_prefix.wine_prefix_tools")
    def test_owned_prefix_rejects_symlinked_initialization_marker(self, tools, run):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            tools.return_value = (root / "wineboot", root / "wineserver")
            prefix = root / "prefix"
            claim_owned_wine_prefix(prefix)
            (prefix / ".opensmacx-test-initialized").symlink_to(root / "outside")
            with self.assertRaisesRegex(RuntimeError, "not a regular file"):
                prepare_owned_wine_prefix(prefix, "wine")
            self.assertFalse((root / "outside").exists())
            run.assert_called()

    def test_pracx_staging_preflights_all_destinations(self):
        build = Path(__file__).resolve().parent.parent / "build"
        with tempfile.TemporaryDirectory(dir=build) as directory:
            output = Path(directory)
            support = output / "__support" / "app"
            support.mkdir(parents=True)
            original = output / "terranx.exe"
            original.write_bytes(b"original")
            (support / "terranx_PRACX.exe").write_bytes(b"pracx")
            (support / "terran_PRACX.exe").write_bytes(b"pracx-base")
            (support / "prax.dll").write_bytes(b"support")
            (output / "terran.exe").mkdir()

            with self.assertRaisesRegex(RuntimeError, "not a regular file"):
                stage_pracx(output)

            self.assertEqual(original.read_bytes(), b"original")
            self.assertFalse((output / "terranx_original.exe").exists())
            self.assertFalse((output / "prax.dll").exists())

    @patch("owned_wine_prefix.subprocess.run")
    @patch("owned_wine_prefix.wine_prefix_tools")
    def test_owned_prefix_is_initialized_once_and_stopped(self, tools, run):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            wineboot = root / "wineboot"
            wineserver = root / "wineserver"
            tools.return_value = (wineboot, wineserver)
            prefix = root / "prefix"

            prepare_owned_wine_prefix(prefix, "wine")
            self.assertTrue((prefix / ".opensmacx-test-owned").is_file())
            self.assertTrue((prefix / ".opensmacx-test-initialized").is_file())
            self.assertTrue(
                (prefix / ".opensmacx-test-crash-dialog-disabled").is_file())
            self.assertEqual(
                [call.args[0][1] for call in run.call_args_list],
                ["--init", "reg", "-k", "-w"],
            )
            # A faulting test binary must print its backtrace to stderr rather
            # than block the runner on winedbg's modal dialog.
            registry = next(call.args[0] for call in run.call_args_list
                            if call.args[0][1] == "reg")
            self.assertEqual(
                registry[1:], ["reg", "add", r"HKCU\Software\Wine\WineDbg",
                               "/v", "ShowCrashDialog", "/t", "REG_DWORD",
                               "/d", "0", "/f"])

            run.reset_mock()
            prepare_owned_wine_prefix(prefix, "wine")
            self.assertEqual(
                [call.args[0][1] for call in run.call_args_list],
                ["-k", "-w"],
            )

    def test_intro_movie_skip_replaces_and_restores_pracx_command(self):
        with tempfile.TemporaryDirectory() as directory:
            game_dir = Path(directory)
            ini_path = game_dir / "Alpha Centauri.ini"
            original = b"[Game]\r\nFoo=Bar\r\n[PRACX]\r\nMoviePlayerCommand=old\r\n"
            ini_path.write_bytes(original)

            config = configure_intro_movie_skip(game_dir, "token")

            configured = ini_path.read_text(encoding="ascii")
            self.assertIn("MoviePlayerCommand=cmd.exe /c .\\.opensmacx-movie-skip-token.bat",
                          configured)
            self.assertEqual(config[2].read_bytes(), b"@exit /b 0\r\n")
            restore_intro_movie_config(*config)
            self.assertEqual(ini_path.read_bytes(), original)
            self.assertFalse(config[2].exists())

    def test_intro_movie_skip_removes_created_configuration(self):
        with tempfile.TemporaryDirectory() as directory:
            game_dir = Path(directory)
            config = configure_intro_movie_skip(game_dir, "token")

            self.assertIn(b"[PRACX]", config[0].read_bytes())
            restore_intro_movie_config(*config)
            self.assertFalse(config[0].exists())

    def test_intro_movie_skip_rejects_symlinked_configuration(self):
        with tempfile.TemporaryDirectory() as directory:
            game_dir = Path(directory)
            target = game_dir / "target.ini"
            target.write_text("[PRACX]\n", encoding="ascii")
            (game_dir / "Alpha Centauri.ini").symlink_to(target)
            with self.assertRaisesRegex(RuntimeError, "invalid game configuration"):
                configure_intro_movie_skip(game_dir, "token")


if __name__ == "__main__":
    unittest.main()
