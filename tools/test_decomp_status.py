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
from unittest import mock
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan  # noqa: E402
import byte_match  # noqa: E402
import decomp_status  # noqa: E402

FIELDS = decomp_status.FIELDS


def make_row(address: str, tier: str, **extra) -> dict:
    # `flag_digest` defaults to the CURRENT one, because that is the ordinary
    # case: a row this tool wrote, under the flag sets it is running now.
    # Every merge assertion below is about that case; the one that is not
    # passes a different digest explicitly.
    row = {field: "" for field in FIELDS}
    row.update({"address": address, "tier": tier, "name": "?x@@YAHXZ",
                "size": "16", "flag_digest": decomp_status.flag_set_digest()})
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

    def test_a_row_from_OTHER_flag_sets_is_not_protected(self):
        """The never-downgrade rule assumes the incumbent came from this tool.

        It did not, on 2026-08-15: `/O2 /Oi-` ran as two extra flag sets,
        wrote sixteen BYTE_EXACT rows, and was reverted - after which those
        rows asserted a match no invocation this tool performs can reproduce,
        against every later run that disagreed. `.opensmacx/byte-match.csv` is
        what agent_brief reads, so they were on their way into a brief.
        """
        stale = make_row("0x00401000", "BYTE_EXACT", flag_digest="deadbeef")
        self.seed([stale])
        fresh = make_row("0x00401000", "MISMATCH")
        total, protected = self.merge({"0x00401000": fresh})
        self.assertEqual(self.read()["0x00401000"]["tier"], "MISMATCH")
        self.assertEqual(protected, {}, "a verdict from flag sets that no "
                                        "longer exist must not be protected")

    def test_the_digest_moves_when_the_flag_sets_do(self):
        before = decomp_status.flag_set_digest()
        with mock.patch.object(decomp_status.byte_match, "FLAG_SETS",
                               decomp_status.byte_match.FLAG_SETS + ("/c /O2 /Oi-",)):
            self.assertNotEqual(decomp_status.flag_set_digest(), before)
        self.assertEqual(decomp_status.flag_set_digest(), before)

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


class UnscoreableDenominator(unittest.TestCase):
    """A wall is not a miss, and a rate that sums them cannot say which.

    145 REFUSED rows are copy-protection and self-modifying spans the project
    has DECIDED not to express; 32 SHARED_TAIL rows are COMDAT-folded spans up
    to thirteen functions claim, where a per-function verdict is not unknown
    but undefined. Neither can ever move, so both sat in the denominator
    understating every rate the project reports about itself.
    """

    def annotation(self, address):
        return annotation_scan.Annotation(
            address=address, mode=annotation_scan.MODE_BODY,
            state=annotation_scan.STATE_IMPLEMENTED, path="src/x.cpp", line=1,
            region="int f() {}\n")

    def summary(self, tiers):
        annotations, outcomes, functions = [], {}, {}
        for index, tier in enumerate(tiers):
            address = 0x00401000 + index * 0x10
            annotations.append(self.annotation(address))
            outcomes[address] = {"tier": tier}
            functions[address] = {"size": "100"}
        out = io.StringIO()
        with redirect_stdout(out):
            decomp_status.summarise(annotations, {}, outcomes, {}, functions)
        return out.getvalue()

    def test_the_denominator_excludes_them(self):
        text = self.summary(["BYTE_EXACT", "MISMATCH", "REFUSED",
                             "SHARED_TAIL"])
        self.assertIn("1/2 BYTE_EXACT", text)
        self.assertIn("100/200 bytes", text)

    def test_they_are_reported_rather_than_dropped(self):
        """Set aside, not swept away - a reader has to be able to see how
        much of the image is wall."""
        text = self.summary(["BYTE_EXACT", "REFUSED", "SHARED_TAIL"])
        self.assertIn("set aside as unscoreable by construction: 2 piece(s), "
                      "200 bytes", text)
        self.assertIn("REFUSED", text)
        self.assertIn("SHARED_TAIL", text)

    def test_a_population_with_no_wall_says_nothing_extra(self):
        text = self.summary(["BYTE_EXACT", "MISMATCH"])
        self.assertIn("1/2 BYTE_EXACT", text)
        self.assertNotIn("set aside", text)

    def test_the_two_tiers_are_named_in_one_place(self):
        """`decomp_status` must not carry a second opinion about which tiers
        can never move; the list lives beside TIER_ORDER."""
        for tier in byte_match.UNSCOREABLE_TIERS:
            self.assertIn(tier, byte_match.TIER_ORDER)


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



class LessonReportTest(unittest.TestCase):
    """The three offline checks that keep a lesson marker honest.

    They are what earns these tokens a place under the rule that state is
    measured and not claimed: a LEVER sits on a body that matched, a RULED-OUT
    on one that has not, and each fails the moment its body moves the other way.
    """

    def annotation(self, **kw):
        return decomp_status.annotation_scan.Annotation(
            address=0x00401000, mode=decomp_status.annotation_scan.MODE_BODY,
            state=kw.pop("state", decomp_status.annotation_scan.STATE_IMPLEMENTED),
            path="src/x.cpp", line=1, **kw)

    def test_ruled_out_on_a_proved_body_fails(self):
        # The refutation case: the spellings were ruled out, and then one
        # worked. The list must be promoted or deleted, not left standing.
        faults = decomp_status.lesson_report(
            [self.annotation(matched=True, ruled_out=("ternary",))])
        self.assertEqual(len(faults), 1)
        self.assertIn("promote", faults[0][1])

    def test_a_lever_without_a_proof_fails(self):
        faults = decomp_status.lesson_report(
            [self.annotation(matched=False, levers=(("jl/jge", "flipped"),))])
        self.assertEqual(len(faults), 1)
        self.assertIn("no BYTE_EXACT", faults[0][1])

    def test_ruled_out_on_a_placeholder_fails(self):
        # You cannot rule a spelling out of a body that does not exist. This is
        # what makes an agent land its best attempt instead of leaving nothing.
        faults = decomp_status.lesson_report(
            [self.annotation(state=decomp_status.annotation_scan.STATE_PLACEHOLDER,
                             ruled_out=("ternary",))])
        self.assertEqual(len(faults), 1)
        self.assertIn("land the attempt", faults[0][1])

    def test_a_lever_with_no_fingerprint_fails(self):
        # The fingerprint is the key the aggregation groups on, so a LEVER
        # without one is a lesson that can never be filed against anything.
        faults = decomp_status.lesson_report(
            [self.annotation(matched=True, region="// LEVER: jl/jge\n")])
        self.assertEqual(len(faults), 1)
        self.assertIn("no fingerprint", faults[0][1])

    def test_the_well_formed_pair_is_silent(self):
        self.assertEqual(decomp_status.lesson_report([
            self.annotation(matched=True, levers=(("jl/jge", "flipped"),)),
            self.annotation(matched=False, ruled_out=("ternary",)),
        ]), [])

    def test_the_committed_tree_is_clean(self):
        from pathlib import Path as _Path
        src = _Path(__file__).resolve().parent.parent / "src"
        if not src.is_dir():
            self.skipTest("no src/ tree")
        self.assertEqual(
            decomp_status.lesson_report(decomp_status.annotation_scan.scan_tree(src)), [])


class CatalogFactsTest(unittest.TestCase):
    """`--work` must not drop what the placeholder was carrying.

    `_catalog_facts` is what `--work` rewrites a placeholder through, and it
    was a second, divergent copy of `project_catalogue.facts` - whose docstring
    claims the two are "one projection, not two". `facts` grew `calls`,
    `indirect` and `notes` when the call graph moved into `src/`; the copy did
    not. Materialising 48 scaffolds for the 2026-08-13 batch therefore deleted
    every call edge and indirect-call-site record those files held: 33 indirect
    sites gone before an agent opened a file.

    It is invisible from the recovery loop - a lost edge does not stop a body
    compiling, or scoring, or banking. It surfaced only because
    `docs/EXCLUSIONS.md` pins the total and `measure_exclusions` re-derives it
    from `src/`, so the count fell 5,159 -> 5,126 and that test went red.
    """

    def row(self):
        return {"name": "?f@@YAXXZ", "size": "16", "prototype": "void f()",
                "caller_count": "4", "call_target_count": "0",
                "_calls": (), "_indirect": (0x00446992, 0x004469A6)}

    def test_the_indirect_sites_survive(self):
        text = decomp_status._catalog_facts(0x00446960, self.row())
        self.assertIn("// indirect", text)
        self.assertIn("0x00446992", text)
        self.assertIn("0x004469A6", text)

    def test_the_call_edges_survive(self):
        row = dict(self.row(), _calls=(0x00401000,))
        text = decomp_status._catalog_facts(0x00446960, row)
        self.assertIn("// calls", text)
        self.assertIn("0x00401000", text)

    def test_it_is_the_same_projection_as_the_catalogue(self):
        # The actual invariant. Two spellings of one projection is what let
        # them drift apart in the first place, so pin equality rather than
        # pinning a list of fields that would drift the same way.
        import project_catalogue
        row = dict(self.row(), _calls=(0x00401000,))
        self.assertEqual(
            decomp_status._catalog_facts(0x00446960, row),
            "".join(l + "\n" for l in project_catalogue.facts(0x00446960, row)))

    def test_a_round_trip_through_the_placeholder_keeps_them(self):
        # The path that actually did the damage: placeholder_text embeds
        # _catalog_facts, and --work rewrites the file through it.
        text = decomp_status.placeholder_text(0x00446960, self.row())
        self.assertIn("// indirect", text)


class DemoteLeversTest(unittest.TestCase):
    """A lever the measurement contradicts is demoted, not argued about.

    `LEVER` names what MADE a body match. Agents keep writing it for "what
    improved this" - three consecutive batches, five annotations in the third -
    and each time the gate went red and a human moved the token by hand. Since
    `--record-matches` already stamps BYTE_EXACT FROM MEASUREMENT, demoting a
    contradicted lever is the same rule pointed the other way.
    """

    def setUp(self):
        self.work = Path(tempfile.mkdtemp())
        self.original_root = decomp_status.REPO_ROOT
        decomp_status.REPO_ROOT = self.work

    def tearDown(self):
        decomp_status.REPO_ROOT = self.original_root
        import shutil
        shutil.rmtree(self.work, ignore_errors=True)

    def write(self, name, marker, lever="// LEVER: flipped the arms\n"):
        path = self.work / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(f"{marker}\n{lever}void f() {{}}\n")
        return name

    def annotation(self, name, matched, levers=(("jl/jge", "flipped"),)):
        return decomp_status.annotation_scan.Annotation(
            address=0x00401000, mode=decomp_status.annotation_scan.MODE_FILE,
            state=decomp_status.annotation_scan.STATE_IMPLEMENTED,
            path=name, line=1, matched=matched, levers=levers)

    def test_a_lever_on_a_mismatch_is_demoted(self):
        name = self.write("a.cpp", "// ORIGINAL: 0x00401000 FILE")
        changed = decomp_status.demote_levers(
            [self.annotation(name, matched=False)],
            {0x00401000: {"tier": "MISMATCH"}})
        self.assertEqual(len(changed), 1)
        text = (self.work / name).read_text()
        self.assertIn("// RULED-OUT: flipped the arms", text)
        self.assertNotIn("// LEVER:", text)

    def test_the_prose_survives_verbatim(self):
        # The observation is real; only the token was wrong. Losing the prose
        # would make the demotion cost more than the fault it fixes.
        name = self.write("b.cpp", "// ORIGINAL: 0x00401000 FILE",
                          "// LEVER: `x < 0x18` emits jge where `x <= 0x17` emits jg\n")
        decomp_status.demote_levers([self.annotation(name, matched=False)],
                                    {0x00401000: {"tier": "MISMATCH"}})
        self.assertIn("`x < 0x18` emits jge where `x <= 0x17` emits jg",
                      (self.work / name).read_text())

    def test_a_lever_on_a_proved_body_is_left_alone(self):
        name = self.write("c.cpp", "// ORIGINAL: 0x00401000 BYTE_EXACT FILE")
        changed = decomp_status.demote_levers(
            [self.annotation(name, matched=True)],
            {0x00401000: {"tier": "BYTE_EXACT"}})
        self.assertEqual(changed, [])
        self.assertIn("// LEVER:", (self.work / name).read_text())

    def test_a_body_this_run_proved_keeps_its_lever(self):
        # record_claims stamps first, but `matched` is still False on the
        # in-memory annotation, so the TIER is what has to save it.
        name = self.write("d.cpp", "// ORIGINAL: 0x00401000 FILE")
        changed = decomp_status.demote_levers(
            [self.annotation(name, matched=False)],
            {0x00401000: {"tier": "BYTE_EXACT"}})
        self.assertEqual(changed, [])
        self.assertIn("// LEVER:", (self.work / name).read_text())

    def test_an_annotation_with_no_lever_is_untouched(self):
        name = self.write("e.cpp", "// ORIGINAL: 0x00401000 FILE",
                          "// RULED-OUT: ternary\n")
        changed = decomp_status.demote_levers(
            [self.annotation(name, matched=False, levers=())],
            {0x00401000: {"tier": "MISMATCH"}})
        self.assertEqual(changed, [])

    def test_it_reports_the_tier_that_contradicted_the_lever(self):
        name = self.write("f.cpp", "// ORIGINAL: 0x00401000 FILE")
        changed = decomp_status.demote_levers(
            [self.annotation(name, matched=False)],
            {0x00401000: {"tier": "SHAPE_EXACT"}})
        self.assertEqual(changed[0][1], "SHAPE_EXACT")


class ScaffoldModeTest(unittest.TestCase):
    """A generated whole-unit scaffold must stay annotated `FILE`.

    Strip that word and the extractor cuts a definition out of a file that is
    all scaffolding: REFUSED ("extract does not end in a closing brace"), or
    NO_COMPILE with the scaffolding's own globals redefined. Measured on
    2026-08-13 across one agent's whole batch of eight - restoring the word
    turned four REFUSED into BYTE_EXACT with no change to any body.

    What makes it worth a check rather than a rule: the agent's own scorer
    cannot see it. `verify_recovered_function` builds the unit itself and never
    reads the marker, so it reports BYTE_EXACT while `--record-matches` reports
    a refusal - and a refusal reads as a bad body, not a bad marker.
    """

    def setUp(self):
        self.work = Path(tempfile.mkdtemp())
        self.original_root = decomp_status.REPO_ROOT
        decomp_status.REPO_ROOT = self.work

    def tearDown(self):
        decomp_status.REPO_ROOT = self.original_root
        import shutil
        shutil.rmtree(self.work, ignore_errors=True)

    def write(self, name, banner=True):
        path = self.work / name
        path.parent.mkdir(parents=True, exist_ok=True)
        head = f"{decomp_status.SCAFFOLD_BANNER}\n" if banner else "// hand written\n"
        path.write_text(head + "int f() { return 0; }\n")
        return name

    def annotation(self, name, mode):
        return decomp_status.annotation_scan.Annotation(
            address=0x00401000, mode=mode,
            state=decomp_status.annotation_scan.STATE_IMPLEMENTED,
            path=name, line=1)

    def test_a_scaffold_in_body_mode_is_a_fault(self):
        name = self.write("src/unrecovered/00401000.cpp")
        faults = decomp_status._scaffold_mode_faults(
            [self.annotation(name, decomp_status.annotation_scan.MODE_BODY)])
        self.assertEqual(len(faults), 1)
        self.assertIn("restore `FILE`", faults[0][1])

    def test_the_same_scaffold_in_file_mode_is_silent(self):
        name = self.write("src/unrecovered/00401000.cpp")
        self.assertEqual(decomp_status._scaffold_mode_faults(
            [self.annotation(name, decomp_status.annotation_scan.MODE_FILE)]), [])

    def test_a_hand_written_file_may_be_body_mode(self):
        # The common, correct case - src/leaf_recoveries.cpp carries hundreds
        # of body-mode annotations. Firing on those would make the check
        # useless the day it landed.
        name = self.write("src/leaf_recoveries.cpp", banner=False)
        self.assertEqual(decomp_status._scaffold_mode_faults(
            [self.annotation(name, decomp_status.annotation_scan.MODE_BODY)]), [])

    def test_a_missing_file_does_not_take_the_check_down(self):
        self.assertEqual(decomp_status._scaffold_mode_faults(
            [self.annotation("src/gone.cpp",
                             decomp_status.annotation_scan.MODE_BODY)]), [])

    def test_it_reaches_lesson_report(self):
        # The routing matters as much as the rule: `lesson_report` is what
        # `--check` calls, and it is called before every early return so a
        # checkout without VC6 still enforces it.
        name = self.write("src/unrecovered/00401000.cpp")
        faults = decomp_status.lesson_report(
            [self.annotation(name, decomp_status.annotation_scan.MODE_BODY)])
        self.assertTrue(any("restore `FILE`" in why for _, why in faults), faults)


if __name__ == "__main__":
    unittest.main()
