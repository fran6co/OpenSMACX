#!/usr/bin/env python3

import argparse
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import batch_decompile


PRIORITIES_HEADER = (
    "rank,priority,score,address,name,size,caller_count,call_target_count,"
    "recovery_state,binding_category,binding_strategy,"
    "analysis_entry_agreement,ida9_relation,ghidra_relation,prototype\n")


def priorities_row(address, name, size, callers, targets, state="unrecovered"):
    return (f"1,P1,100,{address},{name},{size},{callers},{targets},"
            f"{state},,,2,exact,exact,\n")


class BatchDecompileTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(
            dir=batch_decompile.REPO_ROOT / "build")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def write_priorities(self, rows):
        path = self.root / "priorities.csv"
        path.write_text(PRIORITIES_HEADER + "".join(rows), encoding="utf-8")
        return path

    def make_arguments(self, **overrides):
        defaults = {
            "recovery_state": "unrecovered", "name_prefix": None,
            "max_size": None, "min_callers": None, "max_call_targets": None,
            "limit": None,
        }
        defaults.update(overrides)
        return argparse.Namespace(**defaults)

    def test_selects_by_state_size_and_call_targets(self):
        path = self.write_priorities([
            priorities_row("0x00605840", "?init@Scroll@@", 64, 5, 0),
            priorities_row("0x00605890", "?init_vert@Scroll@@", 4096, 5, 0),
            priorities_row("0x006058D0", "?close@Scroll@@", 64, 5, 3),
            priorities_row("0x00605910", "?done@Scroll@@", 64, 5, 0,
                           state="source_complete"),
        ])
        arguments = self.make_arguments(max_size=1024, max_call_targets=0)
        selected = batch_decompile.select_from_priorities(path, arguments)
        self.assertEqual(selected, ["0x00605840"])

    def test_selects_by_name_prefix_and_limit(self):
        path = self.write_priorities([
            priorities_row("0x0060EA10", "?flip_state_id@CheckBox@@", 312, 1, 0),
            priorities_row("0x0060EB80", "?set_state_id@CheckBox@@", 139, 3, 0),
            priorities_row("0x00605840", "?init@Scroll@@", 64, 5, 0),
        ])
        arguments = self.make_arguments(name_prefix="?", limit=2)
        selected = batch_decompile.select_from_priorities(path, arguments)
        self.assertEqual(selected, ["0x0060EA10", "0x0060EB80"])

    def test_cached_addresses_require_status_and_file(self):
        output = self.root / "cache"
        output.mkdir()
        (output / "0x00605840-init.c").write_text("int f;", encoding="utf-8")
        manifest = {
            "0x00605840": {"status": "ok", "file": "0x00605840-init.c"},
            "0x00605890": {"status": "ok", "file": "missing.c"},
            "0x006058D0": {"status": "error", "error": "timeout"},
        }
        cached = batch_decompile.cached_addresses(manifest, output)
        self.assertEqual(cached, {"0x00605840"})

    def test_manifest_round_trip_tolerates_corruption(self):
        output = self.root / "cache"
        output.mkdir()
        self.assertEqual(batch_decompile.load_manifest(output), {})
        (output / batch_decompile.MANIFEST_NAME).write_text(
            "{broken", encoding="utf-8")
        self.assertEqual(batch_decompile.load_manifest(output), {})
        payload = {"0x00605840": {"status": "ok", "file": "a.c"}}
        (output / batch_decompile.MANIFEST_NAME).write_text(
            json.dumps(payload), encoding="utf-8")
        self.assertEqual(batch_decompile.load_manifest(output), payload)

    def test_normalize_address_formats(self):
        self.assertEqual(
            batch_decompile.normalize_address("0x605840"), "0x00605840")
        self.assertEqual(
            batch_decompile.normalize_address("00605840"), "0x00605840")

    def test_dry_run_skips_headless_invocation(self):
        addresses = self.root / "addresses.txt"
        addresses.write_text("0x00605840\n", encoding="utf-8")
        output = self.root / "cache"
        argv = ["batch_decompile.py",
                "--addresses", str(addresses),
                "--output-dir", str(output),
                "--project-dir", str(self.root / "projects"),
                "--dry-run"]
        with mock.patch.object(batch_decompile, "run_headless") as headless, \
                mock.patch("sys.argv", argv):
            result = batch_decompile.main()
        self.assertEqual(result, 0)
        headless.assert_not_called()

    def test_rejects_output_outside_local_roots(self):
        addresses = self.root / "addresses.txt"
        addresses.write_text("0x00605840\n", encoding="utf-8")
        outside = Path(tempfile.mkdtemp())
        self.addCleanup(lambda: outside.rmdir())
        argv = ["batch_decompile.py",
                "--addresses", str(addresses),
                "--output-dir", str(outside / "cache"),
                "--dry-run"]
        with mock.patch("sys.argv", argv):
            with self.assertRaises(RuntimeError):
                batch_decompile.main()


if __name__ == "__main__":
    unittest.main()
