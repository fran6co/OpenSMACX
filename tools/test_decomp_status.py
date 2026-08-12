#!/usr/bin/env python3
"""Offline tests for tools/decomp_status.py.

The status tool is the one place the map, the compiler verdicts, and the
ledger meet, so these pin the joins: the cache's invalidation contract, the
ledger's merge law (never truncate, never downgrade BYTE_EXACT), the drift
report's categories, and the SKIP path. No VC6 and no Wine - the measurement
phase itself is exercised by the ratchet and by the pilot, which need both.
"""

from __future__ import annotations

import csv
import io
import json
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan  # noqa: E402
import byte_match  # noqa: E402
import decomp_status  # noqa: E402

FIELDS = decomp_status.FIELDS


def make_row(address: str, tier: str, **extra) -> dict:
    row = {field: "" for field in FIELDS}
    row.update({"address": address, "tier": tier, "name": "?x@@YAHXZ",
                "size": "16"})
    row.update(extra)
    return row


class Cache(unittest.TestCase):

    def test_round_trip(self):
        with tempfile.TemporaryDirectory() as tmp:
            cache_path = Path(tmp) / "cache.json"
            with mock.patch.object(decomp_status, "CACHE_PATH", cache_path):
                decomp_status.save_cache({"abc": {"tier": "BYTE_EXACT"}})
                loaded = decomp_status.load_cache(no_cache=False)
        self.assertEqual(loaded, {"abc": {"tier": "BYTE_EXACT"}})

    def test_no_cache_flag_returns_empty(self):
        with tempfile.TemporaryDirectory() as tmp:
            cache_path = Path(tmp) / "cache.json"
            with mock.patch.object(decomp_status, "CACHE_PATH", cache_path):
                decomp_status.save_cache({"abc": {"tier": "BYTE_EXACT"}})
                self.assertEqual(decomp_status.load_cache(no_cache=True), {})

    def test_version_mismatch_invalidates(self):
        with tempfile.TemporaryDirectory() as tmp:
            cache_path = Path(tmp) / "cache.json"
            cache_path.write_text(json.dumps(
                {"version": "v0-ancient", "entries": {"abc": {}}}))
            with mock.patch.object(decomp_status, "CACHE_PATH", cache_path):
                self.assertEqual(decomp_status.load_cache(no_cache=False), {})

    def test_corrupt_cache_is_empty_not_fatal(self):
        with tempfile.TemporaryDirectory() as tmp:
            cache_path = Path(tmp) / "cache.json"
            cache_path.write_text("{not json")
            with mock.patch.object(decomp_status, "CACHE_PATH", cache_path):
                self.assertEqual(decomp_status.load_cache(no_cache=False), {})

    def test_unit_hash_is_content_keyed(self):
        self.assertEqual(decomp_status.unit_hash("int f() {}"),
                         decomp_status.unit_hash("int f() {}"))
        self.assertNotEqual(decomp_status.unit_hash("int f() {}"),
                            decomp_status.unit_hash("int f() { }"))


class LedgerMerge(unittest.TestCase):

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.ledger = Path(self._tmp.name) / "byte-match.csv"

    def tearDown(self):
        self._tmp.cleanup()

    def merge(self, rows_to_write: dict):
        with mock.patch.object(decomp_status, "LEDGER", self.ledger):
            return decomp_status.merge_ledger(rows_to_write)

    def seed(self, rows: list):
        with self.ledger.open("w", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=FIELDS)
            writer.writeheader()
            for row in rows:
                writer.writerow(row)

    def read(self) -> dict:
        with self.ledger.open(newline="") as handle:
            return {row["address"]: row for row in csv.DictReader(handle)}

    def test_foreign_rows_preserved_verbatim(self):
        foreign = make_row("0x00999999", "MISMATCH", note="not ours")
        self.seed([foreign])
        total, protected = self.merge(
            {"0x00401000": make_row("0x00401000", "BYTE_EXACT")})
        self.assertEqual(total, 2)
        self.assertEqual(protected, {})
        self.assertEqual(self.read()["0x00999999"]["note"], "not ours")

    def test_byte_exact_is_never_downgraded(self):
        incumbent = make_row("0x00401000", "BYTE_EXACT",
                             source_location="old.cpp:1")
        self.seed([incumbent])
        fresh = make_row("0x00401000", "NO_COMPILE",
                         source_location="new.cpp:2")
        total, protected = self.merge({"0x00401000": fresh})
        self.assertEqual(self.read()["0x00401000"]["tier"], "BYTE_EXACT")
        self.assertEqual(self.read()["0x00401000"]["source_location"],
                         "old.cpp:1")
        self.assertIn("0x00401000", protected)

    def test_byte_exact_reproduction_writes(self):
        self.seed([make_row("0x00401000", "BYTE_EXACT")])
        fresh = make_row("0x00401000", "BYTE_EXACT",
                         source_location="new.cpp:2")
        self.merge({"0x00401000": fresh})
        self.assertEqual(self.read()["0x00401000"]["source_location"],
                         "new.cpp:2")

    def test_worse_tiers_overwrite(self):
        self.seed([make_row("0x00401000", "NO_COMPILE")])
        self.merge({"0x00401000": make_row("0x00401000", "BYTE_EXACT")})
        self.assertEqual(self.read()["0x00401000"]["tier"], "BYTE_EXACT")

    def test_write_is_atomic_no_tmp_left(self):
        self.merge({"0x00401000": make_row("0x00401000", "MISMATCH")})
        self.assertFalse(self.ledger.with_suffix(".tmp").exists())
        self.assertTrue(self.ledger.is_file())


class SpanClasses(unittest.TestCase):

    def test_dict_form(self):
        outcome = {"span_classes": {"primary": 1, "eh": 2, "shared": 0,
                                    "selfmod": 0}}
        self.assertEqual(decomp_status.span_class_string(outcome),
                         "primary=1;eh=2;shared=0;selfmod=0")

    def test_missing_is_empty(self):
        self.assertEqual(decomp_status.span_class_string({}), "")

    def test_string_form_passes_through(self):
        outcome = {"span_classes": "primary=1;eh=0;shared=0;selfmod=0"}
        self.assertEqual(decomp_status.span_class_string(outcome),
                         "primary=1;eh=0;shared=0;selfmod=0")


class Drift(unittest.TestCase):

    def annotation(self, address, path="src/x.cpp", line=1):
        return annotation_scan.Annotation(
            address=address, mode=annotation_scan.MODE_BODY,
            state=annotation_scan.STATE_IMPLEMENTED, path=path, line=line,
            region="int f() {}\n")

    def test_categories(self):
        functions = {
            0x00401000: {"recovery_state": "unrecovered",
                         "source_locations": ""},
            0x00401060: {"recovery_state": "source_complete",
                         "source_locations": ""},
            0x00401520: {"recovery_state": "source_complete",
                         "source_locations": "src/old.cpp:9"},
            0x00401560: {"recovery_state": "source_complete",
                         "source_locations": "src/x.cpp:1"},
            # source_complete, no location, but the scan HAS an annotation:
            # the scan can locate it.
            0x00402500: {"recovery_state": "source_complete",
                         "source_locations": ""},
        }
        annotations = [self.annotation(0x00401000),
                       self.annotation(0x00401520),
                       self.annotation(0x00401560),
                       self.annotation(0x00402500, line=40)]
        matched = {a.address: a for a in annotations}
        drift = decomp_status.drift_report(annotations, matched, functions)
        self.assertEqual([a for a, _ann, _s in
                          drift["annotated_not_recovered"]], [0x00401000])
        self.assertEqual(drift["unannotated_complete"], [0x00401060])
        self.assertEqual(drift["locatable_unlocated"], [0x00402500])
        self.assertEqual([(a, was, now) for a, was, now in
                          drift["stale_locations"]],
                         [(0x00401520, "src/old.cpp:9", "src/x.cpp:1")])
        self.assertEqual(drift["duplicates"], {})


class ResolveIntegration(unittest.TestCase):
    """resolve() collapses stacked comments; precedence settles the stores."""

    def annotation(self, address, region, deprecated=False, recipe="census",
                   path="src/x.cpp"):
        return annotation_scan.Annotation(
            address=address, mode=annotation_scan.MODE_BODY,
            state=annotation_scan.STATE_IMPLEMENTED, path=path, line=1,
            deprecated=deprecated, region=region, recipe=recipe)

    def test_stacked_comments_collapse(self):
        first = self.annotation(0x00401000, "/* a */\nint f() {}\n")
        second = self.annotation(0x00401000, "int f() {}\n")
        resolved, conflicts = annotation_scan.resolve([first, second])
        self.assertEqual(resolved, [first])
        self.assertEqual(conflicts, {})

    def test_proved_beats_preserved(self):
        proved = self.annotation(0x00401000, "int f() { return 1; }\n",
                                 deprecated=True, recipe="writeback",
                                 path="src/recovered/00401000.cpp")
        preserved = self.annotation(0x00401000, "// unit\nint f() { return 2; }\n",
                                    deprecated=True, recipe="verbatim",
                                    path="src/recovered/units/00401000.cpp")
        resolved, conflicts = annotation_scan.resolve([preserved, proved])
        self.assertEqual(resolved, [proved])
        self.assertEqual(conflicts, {})

    def test_new_grammar_beats_everything(self):
        new = self.annotation(0x00401000, "int f() { return 3; }\n")
        legacy = self.annotation(0x00401000, "int f() { return 1; }\n",
                                 deprecated=True, recipe="writeback")
        resolved, conflicts = annotation_scan.resolve([legacy, new])
        self.assertEqual(resolved, [new])
        self.assertEqual(conflicts, {})

    def test_proved_beats_preserved_once_both_migrated(self):
        # After the migration both stores carry explicit markers; the tie
        # breaks on what the file IS, not the spelling it used to carry.
        proved = self.annotation(0x006116E0, "int f() { return 1; }\n",
                                 recipe="writeback",
                                 path="src/recovered/006116e0.cpp")
        preserved = self.annotation(0x006116E0, "// unit\nint g() {}\n",
                                    recipe="verbatim",
                                    path="src/recovered/units/006116e0.cpp")
        resolved, conflicts = annotation_scan.resolve([preserved, proved])
        self.assertEqual(resolved, [proved])
        self.assertEqual(conflicts, {})

    def test_tie_is_a_conflict(self):
        left = self.annotation(0x00401000, "int f() { return 1; }\n",
                               path="src/a.cpp")
        right = self.annotation(0x00401000, "int f() { return 2; }\n",
                                path="src/b.cpp")
        resolved, conflicts = annotation_scan.resolve([left, right])
        self.assertEqual(resolved, [])
        self.assertEqual(conflicts[0x00401000], [left, right])


class Placeholders(unittest.TestCase):

    def row(self, state="unrecovered", **extra):
        base = {"recovery_state": state, "name": "?f@@YAHXZ", "size": "16",
                "body_ranges": "", "prototype": "int __cdecl f(void)",
                "caller_count": "2", "call_target_count": "1"}
        base.update(extra)
        return base

    def test_exclusion_grounds(self):
        token, prose = decomp_status.exclusion_for(
            0x006470A6, self.row(state="external_library"))
        self.assertEqual(token, "S1")
        self.assertIn("EXCLUSIONS", prose)
        token, _ = decomp_status.exclusion_for(0x0065A000, self.row())
        self.assertEqual(token, "S2a")
        token, _ = decomp_status.exclusion_for(
            decomp_status.PORT_IO_ADDRESS, self.row())
        self.assertEqual(token, "S3")
        self.assertEqual(decomp_status.exclusion_for(0x00401000, self.row()),
                         ("", ""))

    def test_placeholder_text_carries_marker_and_sentinel(self):
        text = decomp_status.placeholder_text(0x00405C20, self.row())
        self.assertIn("// ORIGINAL: 0x00405C20 FILE", text)
        self.assertIn("// BODY GOES HERE.", text)
        self.assertIn("?f@@YAHXZ", text)

    def test_excluded_text_has_no_sentinel(self):
        text = decomp_status.excluded_text(0x006470A6, self.row(), "S1",
                                           "availability - CRT")
        self.assertIn("ORIGINAL: 0x006470A6 EXCLUDED S1", text)
        self.assertNotIn("BODY GOES HERE", text)

    def test_generate_skips_annotated_and_source_complete(self):
        functions = {
            0x00401000: self.row(),                      # -> placeholder
            0x00401060: self.row(),                      # annotated: skip
            0x00401520: self.row(state="source_complete"),  # skip + report
            0x00401560: self.row(state="external_library"),  # -> EXCLUDED
        }
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "unrecovered"
            with mock.patch.object(decomp_status, "UNRECOVERED_DIR", target):
                buffer = io.StringIO()
                with redirect_stdout(buffer):
                    decomp_status.generate_placeholders(
                        functions, annotated={0x00401060}, force=False)
            written = sorted(p.name for p in target.glob("*.cpp"))
            self.assertEqual(written, ["00401000.cpp", "00401560.cpp"])
            self.assertIn("EXCLUDED S1",
                          (target / "00401560.cpp").read_text())
        self.assertIn("source_complete", buffer.getvalue())

    def test_generate_is_idempotent_without_force(self):
        functions = {0x00401000: self.row()}
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "unrecovered"
            with mock.patch.object(decomp_status, "UNRECOVERED_DIR", target):
                with redirect_stdout(io.StringIO()):
                    decomp_status.generate_placeholders(functions, set(),
                                                        force=False)
                    (target / "00401000.cpp").write_text("// user edits\n")
                    decomp_status.generate_placeholders(functions, set(),
                                                        force=False)
            self.assertEqual((target / "00401000.cpp").read_text(),
                             "// user edits\n")


class Migration(unittest.TestCase):
    """The rewrite is a change of spelling: comment-only, address-preserving."""

    def migrate(self, text: str, rel: str):
        path = Path(rel)
        with mock.patch.object(decomp_status, "REPO_ROOT", Path("/fake")):
            # store_kind only inspects the path's directory names, so a
            # fake root keeps the tests hermetic.
            return decomp_status.plan_file_migration(path, text, "")

    def test_block_marker_rewritten_in_place(self):
        text = ("/*\nPurpose: demo.\nOriginal Offset: 00401640\n*/\n"
                "int f() {\n    return 1;\n}\n")
        new = self.migrate(text, "src/x.cpp")
        self.assertIn("ORIGINAL: 0x00401640", new)
        self.assertNotIn("Original Offset:", new)
        self.assertEqual(annotation_scan._code_only(text),
                         annotation_scan._code_only(new))

    def test_trailing_marker_moves_above_definition(self):
        text = ("void __cdecl random_rand() { reseed(0); } // 00625700\n"
                "int other() {\n    return 2;\n}\n")
        path = Path("src/random.cpp")
        new = decomp_status.plan_file_migration(path, text, "")
        lines = new.splitlines()
        self.assertEqual(lines[0], "// ORIGINAL: 0x00625700")
        self.assertEqual(lines[1], "void __cdecl random_rand() { reseed(0); }")
        self.assertNotIn("00625700", lines[1])
        # The moved marker extracts the SAME body forward.
        annotations = annotation_scan.scan_text(new, path)
        self.assertEqual(len(annotations), 1)
        self.assertFalse(annotations[0].deprecated)
        self.assertIn("reseed(0);", annotations[0].region)

    def test_trailing_multiline_body_moves_above_opening(self):
        text = ("void __cdecl f(int a) {\n    if (a) {\n        g();\n    }\n"
                "} // 00625720\n")
        new = decomp_status.plan_file_migration(Path("src/x.cpp"), text, "")
        lines = new.splitlines()
        self.assertEqual(lines[0], "// ORIGINAL: 0x00625720")
        self.assertEqual(lines[1], "void __cdecl f(int a) {")
        self.assertEqual(lines[-1], "}")

    def test_opening_brace_marker_moves_above_definition(self):
        text = ("int __cdecl f(int a) { // 00585120\n"
                "    return a;\n}\n")
        new = decomp_status.plan_file_migration(Path("src/x.cpp"), text, "")
        lines = new.splitlines()
        self.assertEqual(lines[0], "// ORIGINAL: 0x00585120")
        self.assertEqual(lines[1], "int __cdecl f(int a) {")

    def test_preserved_unit_gains_file_marker(self):
        text = "// PRESERVED UNIT - measured BYTE_EXACT.\nint f() {}\n"
        new = decomp_status.plan_file_migration(
            Path("src/recovered/units/00403b70.cpp"), text, "unit")
        self.assertTrue(new.startswith("// ORIGINAL: 0x00403B70 FILE\n"))
        annotations = annotation_scan.scan_text(
            new, Path("src/recovered/units/00403b70.cpp"))
        self.assertEqual(len(annotations), 1)
        self.assertFalse(annotations[0].deprecated)
        self.assertEqual(annotations[0].mode, annotation_scan.MODE_FILE)

    def test_proved_body_gains_marker_and_keeps_body_semantics(self):
        text = ("// 0x00401000  ??0StringStruct@@QAE@H@Z\n//\n// prose\n\n"
                "StringStruct::StringStruct(int a1) {\n    p[0] = 0;\n}\n")
        path = Path("src/recovered/00401000.cpp")
        new = decomp_status.plan_file_migration(path, text, "")
        self.assertTrue(new.startswith("// ORIGINAL: 0x00401000\n"))
        annotations = annotation_scan.scan_text(new, path)
        self.assertEqual(len(annotations), 1)
        self.assertFalse(annotations[0].deprecated)
        self.assertEqual(annotations[0].recipe, "writeback")
        # Body semantics unchanged: the definition after the header run.
        self.assertIn("p[0] = 0;", annotations[0].region)
        self.assertNotIn("prose", annotations[0].region)

    def test_no_deprecated_spellings_remain_in_src(self):
        # The migration ran once (1,607 files, guards recorded in the
        # commit); the permanent invariant is that nothing regresses back.
        # A legacy spelling reappearing means a writer emitted the old
        # form, and the map is no longer one grammar.
        annotations = annotation_scan.scan_tree()
        deprecated = [a for a in annotations if a.deprecated]
        self.assertEqual(
            deprecated, [],
            f"{len(deprecated)} deprecated annotation(s) remain, first: "
            f"{deprecated[0].location if deprecated else ''}")

    def test_migration_plan_guards_on_synthetic_files(self):
        text = ("/*\nOriginal Offset: 00401640\n*/\n"
                "int f() {\n    return 1;\n}\n")
        new = decomp_status.plan_file_migration(Path("src/x.cpp"), text, "")
        plan = [(Path("src/x.cpp"), text, new)]
        self.assertTrue(decomp_status.check_migration(plan))

    def test_guard_fails_when_code_would_change(self):
        plan = [(Path("src/x.cpp"), "int f() {}\n", "int f() { return 1; }\n")]
        self.assertFalse(decomp_status.check_migration(plan))


class SkipAndStateOnly(unittest.TestCase):
    """The CLI must degrade to SKIP and still report the map."""

    def test_skip_path_reports_map_and_exits_zero(self):
        fake_functions = {0x00401000: {"recovery_state": "unrecovered",
                                       "size": "16", "name": "?f@@YAHXZ"}}
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "x.cpp"
            source.write_text("// ORIGINAL: 0x00401000\n"
                              "int f() {\n    return 1;\n}\n")
            with mock.patch.object(decomp_status.emit, "load_functions",
                                   return_value=fake_functions), \
                 mock.patch.object(decomp_status.byte_match, "available",
                                   return_value="cl.exe is absent"), \
                 mock.patch.object(annotation_scan, "SRC_ROOT", Path(tmp)):
                buffer = io.StringIO()
                with redirect_stdout(buffer):
                    code = decomp_status.main([str(source)])
        self.assertEqual(code, 0)
        self.assertIn("SKIP", buffer.getvalue())
        self.assertIn("implemented", buffer.getvalue())

    def test_state_only_json(self):
        fake_functions = {0x00401000: {"recovery_state": "unrecovered",
                                       "size": "16", "name": "?f@@YAHXZ"}}
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "x.cpp"
            source.write_text("// ORIGINAL: 0x00401000\n"
                              "int f() {\n    return 1;\n}\n")
            with mock.patch.object(decomp_status.emit, "load_functions",
                                   return_value=fake_functions):
                buffer = io.StringIO()
                with redirect_stdout(buffer):
                    code = decomp_status.main([str(source), "--state-only",
                                               "--json"])
        self.assertEqual(code, 0)
        payload = json.loads(buffer.getvalue())
        self.assertEqual(payload["states"]["implemented"]["count"], 1)
        self.assertEqual(payload["states"]["implemented"]["bytes"], 16)



class Ratchet(unittest.TestCase):
    """The BYTE_EXACT claim in `src/` is the floor, so it must be able to fail.

    Proved end to end once by hand as well: changing `Sound::UNK1` from
    `return 11` to `return 12` made `--check` exit 1 and name
    0x004C6430 src/sound.cpp:27.
    """

    def annotation(self, address, matched, path="src/x.cpp", line=3):
        return annotation_scan.Annotation(
            address=address, mode=annotation_scan.MODE_BODY,
            state=annotation_scan.STATE_IMPLEMENTED, path=path, line=line,
            matched=matched)

    def test_a_reproduced_claim_is_not_a_regression(self):
        entries = [self.annotation(0x401000, True)]
        self.assertEqual([], decomp_status.claim_regressions(
            entries, {0x401000: {"tier": "BYTE_EXACT"}}))

    def test_a_lost_claim_is_a_regression(self):
        entries = [self.annotation(0x401000, True)]
        lost = decomp_status.claim_regressions(
            entries, {0x401000: {"tier": "MISMATCH"}})
        self.assertEqual(1, len(lost))
        self.assertEqual("MISMATCH", lost[0][1])

    def test_an_unmeasured_claim_is_a_regression(self):
        # Silence is not success: a claim whose piece this run never measured
        # is exactly as unproved as one that measured worse.
        entries = [self.annotation(0x401000, True)]
        lost = decomp_status.claim_regressions(entries, {})
        self.assertEqual(1, len(lost))
        self.assertEqual("not measured", lost[0][1])

    def test_an_unclaimed_piece_is_never_a_regression(self):
        entries = [self.annotation(0x401000, False)]
        self.assertEqual([], decomp_status.claim_regressions(
            entries, {0x401000: {"tier": "NO_COMPILE"}}))

    def test_recording_adds_the_token_to_the_marker_line(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "src").mkdir()
            target = root / "src" / "x.cpp"
            target.write_text("/*\nPurpose: t\nORIGINAL: 0x00401000\n*/\n"
                              "int f() { return 0; }\n")
            with mock.patch.object(decomp_status, "REPO_ROOT", root):
                written = decomp_status.record_claims(
                    [self.annotation(0x401000, False, line=3)],
                    {0x401000: {"tier": "BYTE_EXACT"}})
            self.assertEqual(1, written)
            self.assertIn("ORIGINAL: 0x00401000 BYTE_EXACT", target.read_text())

    def test_recording_is_idempotent(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "src").mkdir()
            target = root / "src" / "x.cpp"
            target.write_text("// ORIGINAL: 0x00401000 BYTE_EXACT\n")
            with mock.patch.object(decomp_status, "REPO_ROOT", root):
                written = decomp_status.record_claims(
                    [self.annotation(0x401000, True, line=1)],
                    {0x401000: {"tier": "BYTE_EXACT"}})
            self.assertEqual(0, written)
            self.assertEqual(1, target.read_text().count("BYTE_EXACT"))

if __name__ == "__main__":
    unittest.main()
