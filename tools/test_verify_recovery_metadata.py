#!/usr/bin/env python3

import csv
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import recovery_metrics
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

    def test_export_manifest_excludes_downstream_inputs_and_outputs(self):
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        lines = verify_recovery_metadata.export_manifest_lines(idb)
        labels = [line.split(":", 1)[0] for line in lines]
        self.assertIn("tools/export_recovery_inventory.py", labels)
        self.assertIn("tools/verify_recovery_metadata.py", labels)
        self.assertNotIn("src/OpenSMACX.def", labels)
        self.assertNotIn("docs/recovery-overrides.csv", labels)
        self.assertNotIn("source_metadata", labels)
        self.assertNotIn("tools/correlate_recovery_analyses.py", labels)
        self.assertNotIn("docs/recovery-binding-classifications.csv", labels)
        self.assertNotIn("docs/recovery/functions.csv", labels)

    def write_export_outputs(self, verify_dir, caller_count="3"):
        # `size` is here because the byte-weighted block is derived from it.
        # Without it every figure in that block is zero, and a refresh that
        # recomputed the block from the wrong rows would still produce zeroes
        # and still compare equal.
        (verify_dir / "functions.csv").write_text(
            "address,size,name,binary_kind,source_locations,source_statuses,"
            "redirect_exports,original_dependencies,recovery_state,priority,"
            "notes,caller_count\n"
            f"0x00401000,300,first,game,,,,,unrecovered,,,{caller_count}\n",
            encoding="utf-8")
        (verify_dir / "callgraph.json").write_text(
            json.dumps({
                "format_version": 1,
                "edges": [],
                "caller_count": caller_count,
            }, sort_keys=True)
            + "\n", encoding="utf-8")
        (verify_dir / "summary.json").write_text(json.dumps({
            "format_version": 1,
            "inputs": {
                "idb": "test.idb",
                "idb_sha256": "abc",
                "original_input_path": "terranx.exe",
                "original_input_sha256": "def",
                "definition": "src/OpenSMACX.def",
                "source_directory": "src",
                "overrides": "docs/recovery-overrides.csv",
            },
            "functions": {
                "total": 1,
                "by_binary_kind": {"game": 1},
                "by_recovery_state": {"unrecovered": 1},
                "with_prototypes": 0,
                "with_comments": 0,
            },
            "source_annotations": {},
            "redirects": {},
            "source_bindings": {},
            "call_graph": {"edges": 0, "caller_count": caller_count},
        }, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    def make_export_checkpoint(self):
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        verify_dir = self.root / "verify"
        verify_dir.mkdir()
        self.write_export_outputs(verify_dir)
        verify_recovery_metadata.write_export_checkpoint(idb, verify_dir)
        self.assertTrue(
            verify_recovery_metadata.export_checkpoint_matches(
                idb, verify_dir))
        return idb, verify_dir

    def test_regeneration_reuses_completed_canonical_export(self):
        idb, verify_dir = self.make_export_checkpoint()
        with mock.patch.object(
                verify_recovery_metadata, "refresh_export_metadata") as refresh, \
                mock.patch("subprocess.run") as run:
            verify_recovery_metadata.run_regeneration(idb, verify_dir)
        refresh.assert_called_once_with(verify_dir)
        self.assertEqual(run.call_count, 1)
        command = run.call_args.args[0]
        self.assertEqual(Path(command[1]), verify_recovery_metadata.CORRELATE_TOOL)

    def test_metadata_only_export_drift_reuses_binary_checkpoint(self):
        idb, verify_dir = self.make_export_checkpoint()
        functions = verify_dir / "functions.csv"
        functions.write_text(
            functions.read_text(encoding="utf-8").replace(
                "0x00401000,first,game,,,,,unrecovered,,,3",
                "0x00401000,first,game,src/first.cpp:10,Complete,,,"
                "source_complete,,,3"),
            encoding="utf-8")
        self.assertTrue(
            verify_recovery_metadata.export_checkpoint_matches(
                idb, verify_dir))
        with mock.patch.object(
                verify_recovery_metadata, "refresh_export_metadata") as refresh, \
                mock.patch("subprocess.run") as run:
            verify_recovery_metadata.run_regeneration(idb, verify_dir)
        refresh.assert_called_once_with(verify_dir)
        self.assertEqual(run.call_count, 1)

    def test_refresh_reapplies_all_source_derived_metadata(self):
        idb, verify_dir = self.make_export_checkpoint()
        annotations = {
            0x00401000: [{
                "location": "src/first.cpp:7",
                "status": "Complete",
            }],
        }
        bindings = [{
            "symbol": "FirstOriginal",
            "address": 0x00401000,
            "kind": "function",
            "type": "func_first *",
            "location": "src/bindings.cpp:9",
        }]
        redirects = [{
            "name": "first",
            "location": "src/OpenSMACX.def:11",
        }]
        overrides = {
            0x00401000: {
                "recovery_state": "",
                "priority": "7",
                "notes": "next batch",
            },
        }
        with mock.patch(
                "export_recovery_inventory.load_source_annotations",
                return_value=annotations), mock.patch(
                "export_recovery_inventory.load_source_bindings",
                return_value=bindings), mock.patch(
                "export_recovery_inventory.load_redirects",
                return_value=redirects), mock.patch(
                "export_recovery_inventory.load_overrides",
                return_value=overrides):
            verify_recovery_metadata.refresh_export_metadata(verify_dir)

        with (verify_dir / "functions.csv").open(
                newline="", encoding="utf-8") as file:
            row = next(iter(csv.DictReader(file)))
        self.assertEqual(row["source_locations"], "src/first.cpp:7")
        self.assertEqual(row["source_statuses"], "Complete")
        self.assertEqual(row["redirect_exports"], "first")
        self.assertEqual(row["original_dependencies"], "FirstOriginal")
        self.assertEqual(row["recovery_state"], "source_complete")
        self.assertEqual(row["priority"], "7")
        self.assertEqual(row["notes"], "next batch")
        summary = json.loads(
            (verify_dir / "summary.json").read_text(encoding="utf-8"))
        self.assertEqual(
            summary["functions"]["by_recovery_state"],
            {"source_complete": 1})
        # The byte-weighted block must be recomputed here, not carried over.
        # This is the path most runs take - the checkpoint is reused and the
        # IDB is never reopened - and `recovery_state` has just changed under
        # it, so a stale block would publish a debt describing the tree as it
        # was before the recovery landed. The row is 300 bytes and is now
        # source_complete, so the debt is zero and the scope is not.
        block = summary["functions"]["bytes"]
        self.assertEqual(block["denominator"]["bytes"], 300)
        self.assertEqual(block["machine_carried"]["bytes"], 0)
        self.assertEqual(block["recovered"]["bytes"], 300)
        self.assertEqual(
            block,
            recovery_metrics.bytes_block([{
                "size": "300", "recovery_state": "source_complete",
                "binary_kind": "game"}]))
        self.assertEqual(summary["source_annotations"], {
            "annotations": 1,
            "matched_function_starts": 1,
            "unique_addresses": 1,
            "unmatched_addresses": [],
        })
        self.assertEqual(summary["redirects"], {
            "definitions": 1,
            "exact_name_matches": 1,
            "unmatched": [],
        })
        self.assertEqual(
            summary["source_bindings"]["items"][0]["function"], "first")
        self.assertTrue(
            verify_recovery_metadata.export_checkpoint_matches(
                idb, verify_dir))

    def test_refresh_counts_a_PROVEN_function_as_proven(self):
        # The refresh path is the one most runs take, and it rebuilds
        # summary.json's byte-weighted block. If it does not thread the proven
        # set through, every checkpoint-reusing run publishes
        # proven_recovered = 0 and silently moves the proven bytes back into
        # unproven_recovered - the exact figure the project promises only ever
        # goes down for a real reason. Measured on the real tree 2026-07-30:
        # a line-shifting src edit triggered refresh, and 53 proven functions
        # (6,086 B) became zero while proven.csv still held all 53 rows.
        idb, verify_dir = self.make_export_checkpoint()
        annotations = {
            0x00401000: [{
                "location": "src/first.cpp:7",
                "status": "Complete",
            }],
        }
        with mock.patch(
                "export_recovery_inventory.load_source_annotations",
                return_value=annotations), mock.patch(
                "export_recovery_inventory.load_source_bindings",
                return_value=[]), mock.patch(
                "export_recovery_inventory.load_redirects",
                return_value=[]), mock.patch(
                "export_recovery_inventory.load_overrides",
                return_value={}), mock.patch(
                "export_recovery_inventory.load_proven_addresses",
                return_value={0x00401000}):
            verify_recovery_metadata.refresh_export_metadata(verify_dir)

        summary = json.loads(
            (verify_dir / "summary.json").read_text(encoding="utf-8"))
        block = summary["functions"]["bytes"]
        self.assertEqual(block["proven_recovered"]["functions"], 1)
        self.assertEqual(block["proven_recovered"]["bytes"], 300)
        self.assertEqual(block["unproven_recovered"]["functions"], 0)
        self.assertEqual(block["unproven_recovered"]["bytes"], 0)

    def test_modified_export_output_forces_canonical_export(self):
        idb, verify_dir = self.make_export_checkpoint()
        (verify_dir / "functions.csv").write_text(
            "drift\n", encoding="utf-8")

        def simulate(command, check):
            self.assertTrue(check)
            if Path(command[1]) == verify_recovery_metadata.EXPORT_TOOL:
                self.write_export_outputs(verify_dir, caller_count="4")

        with mock.patch("subprocess.run", side_effect=simulate) as run:
            verify_recovery_metadata.run_regeneration(idb, verify_dir)
        self.assertEqual(
            [Path(call.args[0][1]) for call in run.call_args_list],
            [verify_recovery_metadata.EXPORT_TOOL,
             verify_recovery_metadata.CORRELATE_TOOL])
        self.assertTrue(
            verify_recovery_metadata.export_checkpoint_matches(
                idb, verify_dir))

    def test_force_reruns_completed_canonical_export(self):
        idb, verify_dir = self.make_export_checkpoint()

        def simulate(command, check):
            self.assertTrue(check)
            if Path(command[1]) == verify_recovery_metadata.EXPORT_TOOL:
                self.write_export_outputs(verify_dir, caller_count="5")

        with mock.patch("subprocess.run", side_effect=simulate) as run:
            verify_recovery_metadata.run_regeneration(
                idb, verify_dir, force=True)
        self.assertEqual(
            [Path(call.args[0][1]) for call in run.call_args_list],
            [verify_recovery_metadata.EXPORT_TOOL,
             verify_recovery_metadata.CORRELATE_TOOL])

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

    def test_promote_copies_drifted_outputs_and_still_compares(self):
        # A recovery leaves the committed metadata stale by construction, so
        # promotion must both copy the regenerated bytes over and keep the
        # comparison that certifies determinism.
        verify_dir = self.root / "verify"
        verify_dir.mkdir()
        committed_dir = self.root / "docs" / "recovery"
        committed_dir.mkdir(parents=True)
        for name in verify_recovery_metadata.COMPARED_OUTPUTS:
            (verify_dir / name).write_bytes(b"regenerated-" + name.encode())
            (committed_dir / name).write_bytes(b"stale")
        with mock.patch.object(
                verify_recovery_metadata, "REPO_ROOT", self.root):
            promoted = verify_recovery_metadata.promote_outputs(verify_dir)
            # Promotion reports every file it moved, and the comparison the
            # gate relies on now passes against what was just written.
            self.assertEqual(
                sorted(promoted),
                sorted(verify_recovery_metadata.COMPARED_OUTPUTS))
            verify_recovery_metadata.compare_outputs(verify_dir)
            for name in verify_recovery_metadata.COMPARED_OUTPUTS:
                self.assertEqual((committed_dir / name).read_bytes(),
                                 b"regenerated-" + name.encode())
            # Re-promoting a current tree copies nothing.
            self.assertEqual(
                verify_recovery_metadata.promote_outputs(verify_dir), [])

    def test_promote_raises_when_regeneration_is_incomplete(self):
        # Promotion must never invent a committed file from a run that did not
        # produce one; that would launder a broken export into the repo.
        verify_dir = self.root / "verify"
        verify_dir.mkdir()
        (self.root / "docs" / "recovery").mkdir(parents=True)
        with mock.patch.object(
                verify_recovery_metadata, "REPO_ROOT", self.root):
            with self.assertRaises(RuntimeError):
                verify_recovery_metadata.promote_outputs(verify_dir)

    def test_promote_rekeys_the_stamp_after_copying(self):
        # The committed outputs are hashed into the cache key, so a stamp
        # written before promotion would certify a tree that no longer exists
        # and the next run would miss.
        idb = self.root / "test.idb"
        idb.write_bytes(b"idb-bytes")
        verify_dir = self.root / "verify"
        cache_dir = self.root / "cache"
        stale_key = hashlib.sha256(
            ("\n".join(verify_recovery_metadata.manifest_lines(idb)) + "\n")
            .encode("utf-8")).hexdigest()
        argv = ["verify_recovery_metadata.py",
                "--idb", str(idb),
                "--verify-dir", str(verify_dir),
                "--cache-dir", str(cache_dir),
                "--promote"]
        calls = []

        def fake_manifest(_path):
            calls.append(None)
            return [f"manifest-revision-{len(calls)}"]

        with mock.patch.object(verify_recovery_metadata, "run_regeneration"), \
                mock.patch.object(verify_recovery_metadata, "compare_outputs"), \
                mock.patch.object(verify_recovery_metadata, "promote_outputs",
                                  return_value=["functions.csv"]), \
                mock.patch.object(verify_recovery_metadata, "manifest_lines",
                                  side_effect=fake_manifest), \
                mock.patch("sys.argv", argv):
            self.assertEqual(verify_recovery_metadata.main(), 0)
        # The manifest was recomputed after the copy, and the stamp on disk is
        # the post-promotion one rather than the key derived before it.
        self.assertEqual(len(calls), 2)
        stamps = [path.name for path in cache_dir.glob("*.stamp")]
        self.assertEqual(len(stamps), 1)
        self.assertNotIn(f"{stale_key}.stamp", stamps)
        self.assertEqual(
            (cache_dir / stamps[0]).read_text(encoding="utf-8"),
            "manifest-revision-2\n")

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




class ManifestClosureTests(unittest.TestCase):
    """The manifest must cover everything the generation reads.

    This is the test that should have existed four defects ago. Each of them was
    the same shape - an input that changes a committed output was absent from
    STATIC_INPUTS, so editing it left the stamp valid and the gate reported
    success over stale data - and each was found by noticing a wrong number:

      * analysis-summary.json recorded a hash of a summary.json it no longer
        matched, because the exporter and the correlator write in that order and
        only the first had been re-run;
      * proven.csv was not an input, so demoting two markers left summary.json
        claiming 54 proofs while proven.csv said 53, stamped "verified";
      * the reused-export refresh path rebuilt the byte block WITHOUT the proven
        set, publishing proven_recovered as zero;
      * recovery_metrics.py - the module that DECIDES the block - was not an
        input, so adding the unproven_by_shape split reported "cached".

    Reviewing the list by hand is what failed four times, so these two tests
    derive it instead.
    """

    ENTRY_POINTS = ("export_recovery_inventory", "correlate_recovery_analyses",
                    "verify_recovery_metadata")

    def _closure(self):
        """Every tools/ module the generation imports, transitively."""
        import ast

        tools = Path(__file__).resolve().parent
        local = {path.stem for path in tools.glob("*.py")}
        seen, queue = set(), list(self.ENTRY_POINTS)
        while queue:
            name = queue.pop()
            if name in seen or name not in local:
                continue
            seen.add(name)
            tree = ast.parse((tools / f"{name}.py").read_text())
            for node in ast.walk(tree):
                if isinstance(node, ast.Import):
                    queue.extend(alias.name.split(".")[0]
                                 for alias in node.names)
                elif isinstance(node, ast.ImportFrom) and node.module:
                    queue.append(node.module.split(".")[0])
        return seen

    def test_every_module_the_generation_IMPORTS_is_an_input(self):
        declared = {path.name for path in verify_recovery_metadata.STATIC_INPUTS}
        missing = sorted(f"{name}.py" for name in self._closure()
                         if f"{name}.py" not in declared)
        self.assertEqual(
            [], missing,
            "these modules run during generation but are not in STATIC_INPUTS, "
            "so editing one leaves the stamp valid over stale output")

    def test_every_docs_FILE_the_generation_names_is_an_input_or_an_output(self):
        """Data inputs, found by introspection rather than by reading the source.

        A path constant is collected whatever expression built it - REPO_ROOT /
        ... or Path(__file__).parent.parent / ... - which a regex over the source
        does not manage, and two of the four defects above were exactly a data
        file nobody had listed.
        """
        import importlib

        repo = Path(__file__).resolve().parent.parent
        docs = repo / "docs"
        declared = {path.resolve() for path in verify_recovery_metadata.STATIC_INPUTS}
        outputs = {(repo / "docs" / "recovery" / name).resolve()
                   for name in verify_recovery_metadata.COMPARED_OUTPUTS}
        named = {}
        for name in sorted(self._closure()):
            module = importlib.import_module(name)
            for attribute in dir(module):
                value = getattr(module, attribute)
                items = value if isinstance(value, (list, tuple)) else [value]
                for item in items:
                    if not isinstance(item, Path):
                        continue
                    resolved = item.resolve()
                    # Directories are where output goes, not inputs.
                    if docs in resolved.parents and not resolved.is_dir():
                        named.setdefault(resolved, f"{name}.{attribute}")
        missing = sorted(
            f"{path.relative_to(repo).as_posix()} ({named[path]})"
            for path in named
            if path not in declared and path not in outputs)
        self.assertEqual(
            [], missing,
            "these docs/ files are named by the generation but are neither "
            "inputs nor declared outputs")


if __name__ == "__main__":
    unittest.main()
