#!/usr/bin/env python3
"""`source_locations` must point at the body it names.

The line number is where `byte_match_census.extract_body` starts reading, and
nothing kept it in step with the file. Insert a line above a body and every
location below it is off by one.

What made this survive so long is that it USUALLY still works: the location
points into a doc comment several lines tall, so a drift of one or two still
lands inside it and the right body still comes out. Measured over the
catalogue, 1,592 locations had drifted and 619 had drifted far enough to
extract the wrong text - a run of global definitions, in the case that
surfaced it, reported as `g_ALPHAMENU_WAVE : undeclared identifier` for a body
that never mentions it.

A NO_COMPILE caused by a bad pointer is indistinguishable from a NO_COMPILE
caused by a broken body, and only one of them is worth an agent's turn. So the
invariant is checked rather than trusted.
"""

import csv
import tempfile
import unittest
from pathlib import Path

import repair_source_locations as tool

REPO_ROOT = Path(__file__).resolve().parent.parent


def body(address, purpose="does a thing"):
    return ["/*", f"Purpose: {purpose}", f"Original Offset: {address}",
            "Status: Complete", "*/", "void f() {}", ""]


class OffsetIndexTest(unittest.TestCase):
    def setUp(self):
        self.holder = tempfile.TemporaryDirectory()
        self.path = Path(self.holder.name) / "owner.cpp"

    def tearDown(self):
        self.holder.cleanup()

    def test_it_finds_every_offset_line(self):
        self.path.write_text("\n".join(
            ['#include "stdafx.h"', ""] + body("00401000") + body("00402000")))
        self.assertEqual({0x401000: 5, 0x402000: 12},
                         tool.offset_index(self.path))

    def test_a_commented_offset_line_still_counts(self):
        self.path.write_text("// Original Offset: 00401000\n")
        self.assertEqual({0x401000: 1}, tool.offset_index(self.path))

    def test_the_first_of_a_duplicated_offset_wins(self):
        """Two bodies claiming one address is `test_no_duplicate_bodies`'s
        complaint to make. Preferring the last one here would hide it behind a
        location that happens to work."""
        self.path.write_text("\n".join(body("00401000") + body("00401000")))
        self.assertEqual({0x401000: 3}, tool.offset_index(self.path))

    def test_a_line_that_only_mentions_an_offset_is_not_one(self):
        self.path.write_text("// see Original Offset: 00401000 for details\n")
        self.assertEqual({}, tool.offset_index(self.path))


class SurveyTest(unittest.TestCase):
    def setUp(self):
        self.holder = tempfile.TemporaryDirectory()
        self.root = Path(self.holder.name)
        self.previous = tool.REPO_ROOT
        tool.REPO_ROOT = self.root
        (self.root / "src").mkdir()
        (self.root / "src" / "owner.cpp").write_text(
            "\n".join(['#include "stdafx.h"', ""] + body("00401000")))

    def tearDown(self):
        tool.REPO_ROOT = self.previous
        self.holder.cleanup()

    def row(self, line):
        return {"address": "0x00401000", "source_locations": f"src/owner.cpp:{line}"}

    def test_a_correct_location_needs_no_repair(self):
        repairs, unfindable = tool.survey([self.row(5)])
        self.assertEqual(([], []), (repairs, unfindable))

    def test_a_drifted_location_is_repaired_to_the_offset_line(self):
        repairs, unfindable = tool.survey([self.row(3)])
        self.assertEqual([], unfindable)
        self.assertEqual("src/owner.cpp:5", repairs[0][2])
        self.assertEqual(2, repairs[0][3], "the reported shift")

    def test_a_body_with_no_offset_line_is_reported_not_moved(self):
        (self.root / "src" / "other.cpp").write_text("void f() {}\n")
        rows = [{"address": "0x00409999",
                 "source_locations": "src/other.cpp:1"}]
        repairs, unfindable = tool.survey(rows)
        self.assertEqual([], repairs)
        self.assertEqual(1, len(unfindable))

    def test_a_missing_file_is_reported_not_crashed_on(self):
        rows = [{"address": "0x00401000",
                 "source_locations": "src/gone.cpp:5"}]
        repairs, unfindable = tool.survey(rows)
        self.assertEqual([], repairs)
        self.assertIn("absent", unfindable[0][2])

    def test_extra_locations_after_the_first_are_left_alone(self):
        rows = [{"address": "0x00401000",
                 "source_locations": "src/owner.cpp:3;src/elsewhere.cpp:9"}]
        repairs, _ = tool.survey(rows)
        self.assertEqual("src/owner.cpp:5", repairs[0][2])


class CatalogueIsCurrentTest(unittest.TestCase):
    """The gate. This is the one that fails when someone inserts a line."""

    def test_no_committed_location_has_drifted(self):
        functions = REPO_ROOT / "docs" / "recovery" / "functions.csv"
        if not functions.is_file():
            self.skipTest("docs/recovery/functions.csv is absent")
        with functions.open(newline="", encoding="utf-8-sig") as handle:
            rows = list(csv.DictReader(handle))
        repairs, _ = tool.survey(rows)
        self.assertEqual(
            [], [f"{row['address']} {was} -> {now}"
                 for row, was, now, _ in repairs[:20]],
            "run tools/repair_source_locations.py --apply")


if __name__ == "__main__":
    unittest.main()
