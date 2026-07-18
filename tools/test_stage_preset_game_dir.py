#!/usr/bin/env python3

import os
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import stage_preset_game_dir


class StagePresetGameDirTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(
            dir=stage_preset_game_dir.REPO_ROOT / "build")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.source = self.root / "master"
        self.destination = self.root / "staged"
        (self.source / "data").mkdir(parents=True)
        (self.source / "terranx.exe").write_bytes(b"legacy")
        (self.source / "data" / "alpha.txt").write_bytes(b"alpha-data")

    def run_main(self):
        argv = ["stage_preset_game_dir.py",
                "--source", str(self.source),
                "--destination", str(self.destination)]
        with mock.patch("sys.argv", argv):
            return stage_preset_game_dir.main()

    def test_mirrors_master_files(self):
        self.assertEqual(self.run_main(), 0)
        self.assertEqual(
            (self.destination / "terranx.exe").read_bytes(), b"legacy")
        self.assertEqual(
            (self.destination / "data" / "alpha.txt").read_bytes(),
            b"alpha-data")

    def test_skips_unchanged_and_updates_changed(self):
        self.run_main()
        copied, skipped, _ = stage_preset_game_dir.mirror(
            self.source, self.destination)
        self.assertEqual(copied, 0)
        self.assertEqual(skipped, 2)
        (self.source / "data" / "alpha.txt").write_bytes(b"alpha-data-two")
        copied, skipped, _ = stage_preset_game_dir.mirror(
            self.source, self.destination)
        self.assertEqual(copied, 1)
        self.assertEqual(
            (self.destination / "data" / "alpha.txt").read_bytes(),
            b"alpha-data-two")

    def test_excludes_patcher_owned_outputs(self):
        (self.source / "terranx_hybrid.exe").write_bytes(b"patched")
        (self.source / "OpenSMACX.dll").write_bytes(b"dll")
        (self.source / ".OpenSMACX.dll-previous-abc").write_bytes(b"tmp")
        self.assertEqual(self.run_main(), 0)
        self.assertFalse((self.destination / "terranx_hybrid.exe").exists())
        self.assertFalse((self.destination / "OpenSMACX.dll").exists())
        self.assertFalse(
            (self.destination / ".OpenSMACX.dll-previous-abc").exists())

    def test_never_deletes_destination_only_files(self):
        self.run_main()
        save = self.destination / "saves" / "auto.sav"
        save.parent.mkdir()
        save.write_bytes(b"game-written")
        self.assertEqual(self.run_main(), 0)
        self.assertEqual(save.read_bytes(), b"game-written")

    def test_rejects_overlapping_directories(self):
        argv = ["stage_preset_game_dir.py",
                "--source", str(self.source),
                "--destination", str(self.source / "nested")]
        with mock.patch("sys.argv", argv):
            with self.assertRaises(SystemExit):
                stage_preset_game_dir.main()

    def test_timestamp_drift_triggers_copy(self):
        self.run_main()
        target = self.source / "terranx.exe"
        stat = target.stat()
        os.utime(target, (stat.st_atime, stat.st_mtime + 5))
        copied, _, _ = stage_preset_game_dir.mirror(
            self.source, self.destination)
        self.assertEqual(copied, 1)


if __name__ == "__main__":
    unittest.main()
