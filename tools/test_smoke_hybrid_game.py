#!/usr/bin/env python3

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from owned_wine_prefix import claim_owned_wine_prefix, prepare_owned_wine_prefix
from smoke_hybrid_game import analyze_diagnostics


class AnalyzeDiagnosticsTests(unittest.TestCase):
    def test_accepts_expected_modules_and_rendering(self):
        result = analyze_diagnostics("""
Loaded Z:\\game\\terranx_hybrid.exe: native
Loaded Z:\\game\\OpenSMACX.dll: native
Loaded Z:\\game\\prax.dll: native
Loaded Z:\\game\\DDRAW.dll: builtin
fixme:ddraw:ddraw_surface1_Flip Ignoring flags 0x1.
""", "terranx_hybrid.exe")
        self.assertEqual(result["fatal_lines"], [])
        self.assertEqual(result["missing_markers"], [])
        self.assertTrue(result["rendering_started"])

    def test_rejects_native_ddraw(self):
        result = analyze_diagnostics("""
Loaded terranx_hybrid.exe: native
Loaded OpenSMACX.dll: native
Loaded prax.dll: native
Loaded DDRAW.dll: native
""", "terranx_hybrid.exe")
        self.assertIn("builtin_ddraw", result["missing_markers"])

    def test_detects_unhandled_exception(self):
        result = analyze_diagnostics(
            "err:seh:NtRaiseException Unhandled exception code c0000409",
            "terranx_hybrid.exe")
        self.assertEqual(len(result["fatal_lines"]), 1)

    def test_owned_prefix_rejects_an_unmarked_existing_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            prefix = Path(directory) / "prefix"
            prefix.mkdir()
            with self.assertRaisesRegex(RuntimeError, "unmarked"):
                claim_owned_wine_prefix(prefix)

    @patch("owned_wine_prefix.subprocess.run")
    @patch("owned_wine_prefix.wine_prefix_tools")
    def test_owned_prefix_is_initialized_once_and_stopped(self, tools, run):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            wineboot = root / "wineboot"
            wineserver = root / "wineserver"
            tools.return_value = (wineboot, wineserver)
            prefix = root / "prefix"

            prepare_owned_wine_prefix(prefix, "wine")
            self.assertTrue((prefix / ".opensmacx-test-owned").is_file())
            self.assertTrue((prefix / ".opensmacx-test-initialized").is_file())
            self.assertEqual(
                [call.args[0][1] for call in run.call_args_list],
                ["--init", "-k", "-w"],
            )

            run.reset_mock()
            prepare_owned_wine_prefix(prefix, "wine")
            self.assertEqual(
                [call.args[0][1] for call in run.call_args_list],
                ["-k", "-w"],
            )


if __name__ == "__main__":
    unittest.main()
