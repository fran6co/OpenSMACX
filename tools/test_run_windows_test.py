#!/usr/bin/env python3

"""The dedicated Wine prefix must be stopped after every test run.

This is a performance guard as much as a hygiene one. The wineserver and the
six service processes it starts (services.exe, two winedevice.exe,
explorer.exe, plugplay.exe, svchost.exe, rpcss.exe) inherit this runner's
stdout and stderr. CTest reads that pipe to EOF, so a session left alive to
save teardown keeps the write end open after the test binary has exited and
CTest waits for it: measured on recovery-leaf-tests, the test process exited
at 0.32 s and the pipe reached EOF at 1.38 s, buying a 1.05 s wait to skip a
0.053 s `wineserver -k`. An opt-in that did exactly that
(OPENSMACX_KEEP_OWNED_WINE_PREFIX_RUNNING) made a 12-mutant sweep take 70.7 s
instead of 45.8 s. If it ever comes back, it must not come back as an
environment variable this runner honours.
"""

import os
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import run_windows_test


RETIRED_KEEP_ENV = "OPENSMACX_KEEP_OWNED_WINE_PREFIX_RUNNING"


class StopOwnedPrefixTest(unittest.TestCase):
    def _run_main(self, returncode):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "suite.exe"
            executable.write_bytes(b"MZ")
            prefix = Path(directory) / "wineprefix"
            argv = ["run_windows_test.py",
                    "--executable", str(executable),
                    "--wine-prefix", str(prefix)]
            stops = []
            with mock.patch("sys.argv", argv), \
                    mock.patch.object(run_windows_test, "find_wine",
                                      return_value="/usr/bin/wine"), \
                    mock.patch.object(run_windows_test,
                                      "prepare_owned_wine_prefix"), \
                    mock.patch.object(
                        run_windows_test, "stop_owned_wine_prefix",
                        side_effect=lambda *args: stops.append(args)), \
                    mock.patch.object(
                        subprocess, "run",
                        return_value=subprocess.CompletedProcess(
                            [], returncode)):
                with self.assertRaises(SystemExit) as raised:
                    run_windows_test.main()
            return raised.exception.code, stops, prefix

    def test_prefix_is_stopped_once_on_success(self):
        code, stops, prefix = self._run_main(0)
        self.assertEqual(0, code)
        self.assertEqual(1, len(stops))
        self.assertEqual(prefix, stops[0][0])

    def test_prefix_is_stopped_once_when_the_suite_fails(self):
        code, stops, _ = self._run_main(3)
        self.assertEqual(3, code)
        self.assertEqual(1, len(stops))

    def test_retired_keep_env_does_not_retain_the_prefix(self):
        with mock.patch.dict(os.environ, {RETIRED_KEEP_ENV: "1"}):
            code, stops, _ = self._run_main(0)
        self.assertEqual(0, code)
        self.assertEqual(1, len(stops))

    def test_runner_does_not_read_the_retired_keep_env(self):
        source = Path(run_windows_test.__file__).read_text(encoding="utf-8")
        self.assertNotIn(RETIRED_KEEP_ENV, source)


if __name__ == "__main__":
    unittest.main()
