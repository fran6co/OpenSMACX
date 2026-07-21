#!/usr/bin/env python3

import hashlib
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import verify_recovery_metadata


class VerifyRecoveryMetadataTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(
            dir=verify_recovery_metadata.REPO_ROOT / "build")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def test_manifest_includes_inputs_and_committed_outputs(self):
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        lines = verify_recovery_metadata.manifest_lines(idb)
        labels = [line.split(":", 1)[0] for line in lines]
        self.assertEqual(labels[0], "python")
        self.assertIn("tools/export_recovery_inventory.py", labels)
        self.assertIn("tools/correlate_recovery_analyses.py", labels)
        self.assertIn("tools/verify_recovery_metadata.py", labels)
        self.assertIn("docs/recovery-overrides.csv", labels)
        self.assertIn("docs/recovery/functions.csv", labels)
        self.assertIn("docs/recovery/priorities.csv", labels)
        self.assertIn("source_metadata", labels)
        self.assertFalse(any(
            label.startswith("src/") and label.endswith(".cpp")
            for label in labels))
        expected = hashlib.sha256(b"idb-bytes").hexdigest()
        idb_line = [line for line in lines if line.endswith(expected)]
        self.assertEqual(len(idb_line), 1)

    def test_source_fingerprint_ignores_behavior_only_edits(self):
        source = self.root / "source"
        source.mkdir()
        implementation = source / "win.cpp"
        implementation.write_text(
            "/* Original Offset: 005ED240\nStatus: Complete\n*/\n"
            "int recovered() { return 1; }\n",
            encoding="utf-8")
        oracle = source / "win_oracle.cpp"
        oracle.write_text("int fixture_count = 1;\n", encoding="utf-8")
        baseline = verify_recovery_metadata.source_metadata_sha256(source)

        implementation.write_text(
            "/* Original Offset: 005ED240\nStatus: Complete\n*/\n"
            "int recovered() { return 2; }\n",
            encoding="utf-8")
        oracle.write_text(
            "int fixture_count = 2;\nint extra_fixture = 3;\n",
            encoding="utf-8")
        self.assertEqual(
            baseline,
            verify_recovery_metadata.source_metadata_sha256(source))

    def test_source_fingerprint_tracks_annotation_edits(self):
        source = self.root / "source"
        source.mkdir()
        implementation = source / "win.cpp"
        implementation.write_text(
            "/* Original Offset: 005ED240\nStatus: Complete\n*/\n"
            "int recovered() { return 1; }\n",
            encoding="utf-8")
        baseline = verify_recovery_metadata.source_metadata_sha256(source)

        implementation.write_text(
            "\n/* Original Offset: 005ED240\nStatus: In progress\n*/\n"
            "int recovered() { return 1; }\n",
            encoding="utf-8")
        self.assertNotEqual(
            baseline,
            verify_recovery_metadata.source_metadata_sha256(source))

    def test_source_fingerprint_tracks_binding_address_and_location(self):
        source = self.root / "source"
        source.mkdir()
        implementation = source / "binding.cpp"
        implementation.write_text(
            "func_target *Target = (func_target *)0x005ED240;\n",
            encoding="utf-8")
        baseline = verify_recovery_metadata.source_metadata_sha256(source)

        implementation.write_text(
            "func_target *Target = (func_target *)0x005ED2D0;\n",
            encoding="utf-8")
        changed_address = verify_recovery_metadata.source_metadata_sha256(source)
        self.assertNotEqual(baseline, changed_address)

        implementation.write_text(
            "\nfunc_target *Target = (func_target *)0x005ED2D0;\n",
            encoding="utf-8")
        self.assertNotEqual(
            changed_address,
            verify_recovery_metadata.source_metadata_sha256(source))

    def test_manifest_changes_when_committed_output_changes(self):
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        baseline = verify_recovery_metadata.manifest_lines(idb)
        functions = (verify_recovery_metadata.REPO_ROOT / "docs" / "recovery" /
                     "functions.csv")
        original = functions.read_bytes()
        try:
            functions.write_bytes(original + b"#drift\n")
            drifted = verify_recovery_metadata.manifest_lines(idb)
        finally:
            functions.write_bytes(original)
        self.assertNotEqual(baseline, drifted)

    def test_missing_input_raises(self):
        with self.assertRaises(RuntimeError):
            verify_recovery_metadata.manifest_lines(self.root / "absent.idb")

    def test_stamp_round_trip_and_prune(self):
        cache = self.root / "cache"
        for index in range(verify_recovery_metadata.CACHE_KEEP_COUNT + 3):
            verify_recovery_metadata.write_stamp(
                cache, f"{index:064x}", f"manifest-{index}\n")
        stamps = list(cache.glob("*.stamp"))
        self.assertEqual(
            len(stamps), verify_recovery_metadata.CACHE_KEEP_COUNT)
        newest = cache / (
            f"{verify_recovery_metadata.CACHE_KEEP_COUNT + 2:064x}.stamp")
        self.assertEqual(
            newest.read_text(encoding="utf-8"),
            f"manifest-{verify_recovery_metadata.CACHE_KEEP_COUNT + 2}\n")

    def test_cached_run_skips_regeneration(self):
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        verify_dir = self.root / "verify"
        cache_dir = self.root / "cache"
        manifest = "\n".join(
            verify_recovery_metadata.manifest_lines(idb)) + "\n"
        key = hashlib.sha256(manifest.encode("utf-8")).hexdigest()
        verify_recovery_metadata.write_stamp(cache_dir, key, manifest)
        argv = ["verify_recovery_metadata.py",
                "--idb", str(idb),
                "--verify-dir", str(verify_dir),
                "--cache-dir", str(cache_dir)]
        with mock.patch.object(verify_recovery_metadata, "run_regeneration") \
                as regenerate, mock.patch("sys.argv", argv):
            result = verify_recovery_metadata.main()
        self.assertEqual(result, 0)
        regenerate.assert_not_called()

    def test_corrupt_stamp_forces_regeneration(self):
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        verify_dir = self.root / "verify"
        cache_dir = self.root / "cache"
        manifest = "\n".join(
            verify_recovery_metadata.manifest_lines(idb)) + "\n"
        key = hashlib.sha256(manifest.encode("utf-8")).hexdigest()
        verify_recovery_metadata.write_stamp(cache_dir, key, "corrupted\n")
        argv = ["verify_recovery_metadata.py",
                "--idb", str(idb),
                "--verify-dir", str(verify_dir),
                "--cache-dir", str(cache_dir)]
        with mock.patch.object(verify_recovery_metadata, "run_regeneration") \
                as regenerate, \
                mock.patch.object(verify_recovery_metadata, "compare_outputs"), \
                mock.patch("sys.argv", argv):
            result = verify_recovery_metadata.main()
        self.assertEqual(result, 0)
        regenerate.assert_called_once()

    def test_force_ignores_matching_stamp(self):
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        verify_dir = self.root / "verify"
        cache_dir = self.root / "cache"
        manifest = "\n".join(
            verify_recovery_metadata.manifest_lines(idb)) + "\n"
        key = hashlib.sha256(manifest.encode("utf-8")).hexdigest()
        verify_recovery_metadata.write_stamp(cache_dir, key, manifest)
        argv = ["verify_recovery_metadata.py",
                "--idb", str(idb),
                "--verify-dir", str(verify_dir),
                "--cache-dir", str(cache_dir),
                "--force"]
        with mock.patch.object(verify_recovery_metadata, "run_regeneration") \
                as regenerate, \
                mock.patch.object(verify_recovery_metadata, "compare_outputs"), \
                mock.patch("sys.argv", argv):
            result = verify_recovery_metadata.main()
        self.assertEqual(result, 0)
        regenerate.assert_called_once()

    def test_verify_dir_containment_enforced(self):
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        outside = Path(tempfile.mkdtemp())
        self.addCleanup(lambda: outside.rmdir())
        argv = ["verify_recovery_metadata.py",
                "--idb", str(idb),
                "--verify-dir", str(outside / "verify"),
                "--cache-dir", str(self.root / "cache")]
        with mock.patch("sys.argv", argv):
            with self.assertRaises(RuntimeError):
                verify_recovery_metadata.main()


if __name__ == "__main__":
    unittest.main()
