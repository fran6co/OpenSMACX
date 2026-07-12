#!/usr/bin/env python3

import unittest

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


if __name__ == "__main__":
    unittest.main()
