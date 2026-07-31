#!/usr/bin/env python3

import argparse
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import batch_decompile
import run_ghidra_analysis


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


class LocatorTests(unittest.TestCase):
    """Cover run_ghidra_analysis's launcher discovery.

    These live beside the batch-decompile tests on purpose: batch_decompile
    imports both functions, this file is the registered ctest that exercises
    them, and until now nothing in the suite executed either one.
    """

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(
            dir=batch_decompile.REPO_ROOT / "build")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def make_shim(self, name):
        """A symlink to a binary called `snap`, as /snap/bin really is."""
        snap = self.root / "snap"
        snap.write_text("#!/bin/sh\n", encoding="utf-8")
        shim_dir = self.root / "bin"
        shim_dir.mkdir(exist_ok=True)
        shim = shim_dir / name
        shim.symlink_to(snap)
        return shim, snap

    def test_a_snap_shim_becomes_snap_run_and_is_never_resolved(self):
        shim, snap = self.make_shim("ghidra.analyzeHeadless")
        self.assertEqual(
            "ghidra.analyzeHeadless", run_ghidra_analysis.snap_shim_app(shim))
        found = run_ghidra_analysis.locate_analyze_headless(str(shim))
        self.assertEqual(["snap", "run", "ghidra.analyzeHeadless"], found)
        # The dangerous failure this guards: resolving the shim yields the
        # snapd binary, which would then be run with Ghidra's arguments and
        # no error at all.
        self.assertNotIn(str(snap), found)

    def test_a_real_executable_is_returned_as_a_one_element_argv(self):
        launcher = self.root / "analyzeHeadless"
        launcher.write_text("#!/bin/sh\n", encoding="utf-8")
        self.assertEqual(
            [str(launcher.resolve())],
            run_ghidra_analysis.locate_analyze_headless(str(launcher)))

    def test_a_ghidra_home_directory_is_searched_for_the_launcher(self):
        support = self.root / "home" / "support"
        support.mkdir(parents=True)
        launcher = support / "analyzeHeadless"
        launcher.write_text("#!/bin/sh\n", encoding="utf-8")
        self.assertEqual(
            [str(launcher)],
            run_ghidra_analysis.locate_analyze_headless(str(self.root / "home")))

    def test_an_empty_ghidra_home_raises_instead_of_returning_nothing(self):
        empty = self.root / "empty"
        empty.mkdir()
        with self.assertRaises(RuntimeError):
            run_ghidra_analysis.locate_analyze_headless(str(empty))

    def test_a_snap_shim_on_PATH_is_not_executed_directly(self):
        shim, snap = self.make_shim("analyzeHeadless")
        with mock.patch.object(run_ghidra_analysis.shutil, "which",
                               side_effect=lambda name: str(shim)
                               if name == "analyzeHeadless" else None):
            found = run_ghidra_analysis.locate_analyze_headless(None)
        self.assertEqual(["snap", "run", "analyzeHeadless"], found)
        self.assertNotIn(str(snap), found)

    def test_snap_is_only_offered_when_the_snap_is_actually_installed(self):
        with mock.patch.object(run_ghidra_analysis.shutil, "which",
                               return_value=None), \
                mock.patch.object(run_ghidra_analysis, "snap_shim_app",
                                  return_value=None):
            self.assertIsNone(run_ghidra_analysis.locate_snap_analyze_headless())
            with self.assertRaises(RuntimeError):
                run_ghidra_analysis.locate_analyze_headless(None)

    def test_configure_java_leaves_the_snap_alone(self):
        # Measured: `snap run ghidra.analyzeHeadless` runs its own JDK 21 with
        # JAVA_HOME unset and with JAVA_HOME=/nonexistent/jdk alike, so this
        # must add nothing on a host whose only Ghidra is the snap.
        environment = {}
        self.assertIsNone(run_ghidra_analysis.configure_java(environment))
        self.assertNotIn("JAVA_HOME", environment)
        preset = {"JAVA_HOME": "/keep/me"}
        run_ghidra_analysis.configure_java(preset)
        self.assertEqual("/keep/me", preset["JAVA_HOME"])


class MultiWordLauncherTests(unittest.TestCase):
    """Every call site must splice the argv prefix, not stringify it."""

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(
            dir=batch_decompile.REPO_ROOT / "build")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.launcher = ["snap", "run", "ghidra.analyzeHeadless"]

    def test_run_headless_puts_all_three_words_first(self):
        output = self.root / "cache"
        output.mkdir()

        def write_manifest(command, **kwargs):
            self.captured = command
            (output / batch_decompile.RUN_MANIFEST_NAME).write_text(
                json.dumps({"0x00401000": {"status": "ok", "file": "a.c"}}),
                encoding="utf-8")

        arguments = argparse.Namespace(
            ghidra_home=None, project_dir=self.root / "projects", timeout=120)
        with mock.patch.object(batch_decompile, "locate_analyze_headless",
                               return_value=self.launcher), \
                mock.patch.object(batch_decompile.subprocess, "run",
                                  side_effect=write_manifest):
            results = batch_decompile.run_headless(
                ["0x00401000"], output, arguments)
        self.assertEqual(self.launcher, self.captured[:3])
        self.assertEqual(str(self.root / "projects"), self.captured[3])
        self.assertEqual("ok", results["0x00401000"]["status"])

    def test_bootstrap_puts_all_three_words_first_and_keeps_the_project(self):
        exe = self.root / "terranx_original.exe"
        exe.write_bytes(b"MZ")
        project_dir = self.root / "projects"

        def make_project(command, **kwargs):
            self.captured = command
            batch_decompile.project_file(project_dir).write_text(
                "gpr", encoding="utf-8")

        arguments = argparse.Namespace(
            ghidra_home=None, project_dir=project_dir, exe=exe, max_cpu=4)
        with mock.patch.object(batch_decompile, "locate_analyze_headless",
                               return_value=self.launcher), \
                mock.patch.object(batch_decompile.subprocess, "run",
                                  side_effect=make_project):
            batch_decompile.bootstrap_project(arguments)
        self.assertEqual(self.launcher, self.captured[:3])
        self.assertIn("-import", self.captured)
        # The whole point of the persistent project: a later -process run
        # attaches to it, so the import must not delete what it made.
        self.assertNotIn("-deleteProject", self.captured)

    def test_bootstrap_refuses_when_the_import_produced_no_project(self):
        exe = self.root / "terranx_original.exe"
        exe.write_bytes(b"MZ")
        arguments = argparse.Namespace(
            ghidra_home=None, project_dir=self.root / "projects", exe=exe,
            max_cpu=1)
        with mock.patch.object(batch_decompile, "locate_analyze_headless",
                               return_value=self.launcher), \
                mock.patch.object(batch_decompile.subprocess, "run"):
            with self.assertRaises(RuntimeError):
                batch_decompile.bootstrap_project(arguments)

    def test_bootstrap_refuses_a_missing_executable(self):
        arguments = argparse.Namespace(
            ghidra_home=None, project_dir=self.root / "projects",
            exe=self.root / "absent.exe", max_cpu=1)
        with self.assertRaises(RuntimeError):
            batch_decompile.bootstrap_project(arguments)


class BootstrapGateTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(
            dir=batch_decompile.REPO_ROOT / "build")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.addresses = self.root / "addresses.txt"
        self.addresses.write_text("0x00401000\n", encoding="utf-8")
        self.project_dir = self.root / "projects"

    def argv(self, *extra):
        return ["batch_decompile.py",
                "--addresses", str(self.addresses),
                "--output-dir", str(self.root / "cache"),
                "--project-dir", str(self.project_dir), *extra]

    def test_project_file_is_named_after_the_persistent_project(self):
        self.assertEqual(
            f"{batch_decompile.PROJECT_NAME}.gpr",
            batch_decompile.project_file(self.project_dir).name)
        self.assertFalse(batch_decompile.project_exists(self.project_dir))
        self.project_dir.mkdir()
        batch_decompile.project_file(self.project_dir).write_text(
            "gpr", encoding="utf-8")
        self.assertTrue(batch_decompile.project_exists(self.project_dir))

    def test_a_missing_project_refuses_instead_of_attaching(self):
        with mock.patch.object(batch_decompile, "run_headless") as headless, \
                mock.patch.object(batch_decompile,
                                  "bootstrap_project") as bootstrap, \
                mock.patch("sys.argv", self.argv()):
            result = batch_decompile.main()
        self.assertEqual(1, result)
        headless.assert_not_called()
        bootstrap.assert_not_called()

    def test_bootstrap_runs_the_import_before_attaching(self):
        with mock.patch.object(batch_decompile, "run_headless",
                               return_value={}) as headless, \
                mock.patch.object(batch_decompile,
                                  "bootstrap_project") as bootstrap, \
                mock.patch("sys.argv", self.argv("--bootstrap")):
            result = batch_decompile.main()
        self.assertEqual(0, result)
        bootstrap.assert_called_once()
        headless.assert_called_once()

    def test_an_existing_project_is_not_reimported(self):
        self.project_dir.mkdir()
        batch_decompile.project_file(self.project_dir).write_text(
            "gpr", encoding="utf-8")
        with mock.patch.object(batch_decompile, "run_headless",
                               return_value={}) as headless, \
                mock.patch.object(batch_decompile,
                                  "bootstrap_project") as bootstrap, \
                mock.patch("sys.argv", self.argv("--bootstrap")):
            result = batch_decompile.main()
        self.assertEqual(0, result)
        bootstrap.assert_not_called()
        headless.assert_called_once()


if __name__ == "__main__":
    unittest.main()
