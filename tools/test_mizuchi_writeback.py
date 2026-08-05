#!/usr/bin/env python3
"""Pin the splice, because a wrong one is invisible in a diff review.

The writeback edits `src/` from a line number in a catalogue. Every way it can
go wrong produces a file that still looks plausible: a doc comment swallowed
into the body, a `}` left orphaned below the replacement, a `Status: Complete`
annotation deleted so `recovery_state` silently regresses on the next
inventory. None of those fail a compile of the CENSUS unit, which is
scaffolding rather than the real file, so the BYTE_EXACT re-check in the tool
would pass and the tree would be broken anyway.

The line-shift tests matter for the same reason and are worse to debug: a
catalogued location that is off by three still extracts A body, just the wrong
one, and the census would then report a mismatch on a function whose source
never changed.

No Wine and no VC6 here. The verification step is patched out; what is pinned
is the editing, the shifting and the restore-on-refusal, which is the half that
runs everywhere and the half that mutates the tree.
"""

from __future__ import annotations

import csv
import json
import sys
import textwrap
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import mizuchi_writeback as tool  # noqa: E402

SOURCE = textwrap.dedent("""\
    #include "stdafx.h"

    /*
    Purpose: Set both coordinates.
    Original Offset: 00601B80
    Status: Complete
    */
    void BasePop::set_loc(int x, int y) {
        loc_a_ = x;
    }

    /*
    Purpose: A later function whose line number must move.
    Status: Complete
    */
    int BasePop::later() {
        return 1;
    }
    """)

# `set_loc`'s doc comment opens on line 3; `later`'s on line 12.
FIRST = 0x00601B80
SECOND = 0x00601C00


class WritebackTest(unittest.TestCase):
    def setUp(self):
        self.root = Path(self.enterContext(
            __import__("tempfile").TemporaryDirectory()))
        (self.root / "src").mkdir()
        self.source = self.root / "src" / "basepop.cpp"
        self.source.write_text(SOURCE)

        recovery = self.root / "docs" / "recovery"
        recovery.mkdir(parents=True)
        self.functions_csv = recovery / "functions.csv"
        self.write_csv(self.functions_csv, "source_locations",
                       [("0x00601B80", "src/basepop.cpp:3"),
                        ("0x00601C00", "src/basepop.cpp:12"),
                        ("0x00700000", "src/other.cpp:12")])
        self.ledger_csv = recovery / "byte-match.csv"
        self.write_csv(self.ledger_csv, "source_location",
                       [("0x00601B80", "src/basepop.cpp:3"),
                        ("0x00601C00", "src/basepop.cpp:12")])
        (self.root / "mizuchi").mkdir()
        self.source_map = self.root / "mizuchi" / "source-map.json"
        # `indent=2` plus a trailing newline is what `emit_asm_dumps.py`
        # writes; matching it here is what makes the restore-on-refusal test
        # able to observe a byte-identical revert rather than a reformat.
        self.source_map.write_text(json.dumps([
            {"file": "src/basepop.cpp", "line": 3, "name": "set_loc"},
            {"file": "src/basepop.cpp", "line": 12, "name": "later"},
        ], indent=2) + "\n")

        self.functions = {
            FIRST: {"name": "?set_loc@BasePop@@QAEXHH@Z",
                    "source_locations": "src/basepop.cpp:3"},
            SECOND: {"name": "?later@BasePop@@QAEHXZ",
                     "source_locations": "src/basepop.cpp:12"},
        }
        self.enterContext(mock.patch.object(census, "REPO_ROOT", self.root))
        self.enterContext(mock.patch.object(tool, "FUNCTIONS_CSV", self.functions_csv))
        self.enterContext(mock.patch.object(tool, "LEDGER_CSV", self.ledger_csv))
        self.enterContext(mock.patch.object(tool, "SOURCE_MAP", self.source_map))
        self.enterContext(mock.patch.object(tool, "REPO_ROOT", self.root))
        self.enterContext(mock.patch.object(emit, "load_functions",
                                            lambda: self.functions))

    @staticmethod
    def write_csv(path, field, rows):
        with path.open("w", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=("address", field, "name"))
            writer.writeheader()
            for address, location in rows:
                writer.writerow({"address": address, field: location, "name": ""})

    @staticmethod
    def locations(path, field):
        with path.open(newline="") as handle:
            return {row["address"]: row[field] for row in csv.DictReader(handle)}

    def writeback(self, code, tier="BYTE_EXACT", target=hex(FIRST)):
        with mock.patch.object(tool, "verify", return_value={"tier": tier}):
            return tool.writeback(target, code)

    def test_replaces_the_definition_and_keeps_the_doc_comment(self):
        result = self.writeback("void BasePop::set_loc(int x, int y) {\n"
                                "    loc_a_ = x;\n    loc_b_ = y;\n}\n")
        text = self.source.read_text()
        self.assertIn("Status: Complete", text)
        self.assertIn("Purpose: Set both coordinates.", text)
        self.assertIn("loc_b_ = y;", text)
        # The old body is gone rather than merely followed by the new one.
        self.assertEqual(text.count("void BasePop::set_loc"), 1)
        self.assertEqual(text.count("loc_a_ = x;"), 1)
        # And the untouched neighbour survives intact.
        self.assertIn("int BasePop::later() {", text)
        self.assertEqual(result["line_delta"], 1)
        self.assertTrue(result["verified"])

    def test_a_longer_body_shifts_only_later_locations_in_the_same_file(self):
        self.writeback("void BasePop::set_loc(int x, int y) {\n"
                       "    loc_a_ = x;\n    loc_b_ = y;\n}\n")
        functions = self.locations(self.functions_csv, "source_locations")
        self.assertEqual(functions["0x00601B80"], "src/basepop.cpp:3")
        self.assertEqual(functions["0x00601C00"], "src/basepop.cpp:13")
        self.assertEqual(functions["0x00700000"], "src/other.cpp:12")
        ledger = self.locations(self.ledger_csv, "source_location")
        self.assertEqual(ledger["0x00601C00"], "src/basepop.cpp:13")
        entries = json.loads(self.source_map.read_text())
        self.assertEqual([e["line"] for e in entries], [3, 13])
        # The shifted line still opens the neighbour's doc comment.
        lines = self.source.read_text().splitlines()
        self.assertEqual(lines[12].strip(), "/*")

    def test_a_shorter_body_shifts_the_other_way(self):
        self.writeback("void BasePop::set_loc(int x, int y) { loc_a_ = x; }\n")
        functions = self.locations(self.functions_csv, "source_locations")
        self.assertEqual(functions["0x00601C00"], "src/basepop.cpp:10")
        lines = self.source.read_text().splitlines()
        self.assertEqual(lines[9].strip(), "/*")

    def test_a_failed_verification_restores_every_file(self):
        before = (self.source.read_text(), self.functions_csv.read_text(),
                  self.ledger_csv.read_text(), self.source_map.read_text())
        with self.assertRaises(tool.Refused) as caught:
            self.writeback("void BasePop::set_loc(int x, int y) {\n"
                           "    loc_a_ = x;\n    loc_b_ = y;\n}\n",
                           tier="MISMATCH")
        self.assertIn("MISMATCH", str(caught.exception))
        after = (self.source.read_text(), self.functions_csv.read_text(),
                 self.ledger_csv.read_text(), self.source_map.read_text())
        self.assertEqual(before, after)

    def test_a_function_with_no_src_location_is_refused_untouched(self):
        self.functions[FIRST]["source_locations"] = ""
        before = self.source.read_text()
        with self.assertRaises(tool.Refused) as caught:
            self.writeback("void BasePop::set_loc(int, int) {}\n")
        self.assertIn("never been placed", str(caught.exception))
        self.assertEqual(self.source.read_text(), before)

    def test_an_empty_submission_is_refused(self):
        with self.assertRaises(tool.Refused):
            self.writeback("   \n\n")

    def test_resolves_a_mangled_name_as_well_as_an_address(self):
        result = self.writeback("void BasePop::set_loc(int x, int y) {\n}\n",
                                target="?set_loc@BasePop@@QAEXHH@Z")
        self.assertEqual(result["address"], "0x00601B80")


class SplitDefinitionTest(unittest.TestCase):
    def test_definition_starts_after_the_doc_comment(self):
        span = ["/*", "Status: Complete", "*/", "void f() {", "}"]
        self.assertEqual(tool.split_definition(span), 3)

    def test_a_span_with_no_comment_starts_at_zero(self):
        self.assertEqual(tool.split_definition(["void f() {", "}"]), 0)

    def test_a_comment_close_inside_the_body_does_not_move_the_boundary(self):
        # A `*/` after the opening brace belongs to the body. Letting it count
        # would splice away real code and leave the file still compiling.
        span = ["void f() {", "    /* inner */", "    return;", "}"]
        self.assertEqual(tool.split_definition(span), 0)


if __name__ == "__main__":
    unittest.main()
