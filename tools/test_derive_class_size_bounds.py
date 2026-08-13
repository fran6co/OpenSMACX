#!/usr/bin/env python3
"""Tests for the class-size bound catalogue that stages oracle receivers.

The whole safety argument for this route is directional: a bound ABOVE the true
size wastes a few kilobytes of zero-filled buffer, and a bound BELOW it lets a
method write past the end of its receiver, which compiles, links and corrupts
memory only at run time. So the tests here are almost entirely about the
directions and the refusals, not about the arithmetic.

The derivation itself reads the hash-bound executable and cannot run in the test
suite; what is covered is everything downstream of it, which is the part the
generator consumes.
"""

from __future__ import annotations

import csv
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import derive_class_size_bounds as bounds  # noqa: E402


class LoadTests(unittest.TestCase):
    def load(self, rows):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bounds.csv"
            with path.open("w", newline="", encoding="utf-8") as handle:
                writer = csv.writer(handle, lineterminator="\n")
                writer.writerow(bounds.FIELDS)
                writer.writerows(rows)
            return bounds.load(path)

    def test_a_bound_inside_the_cap_is_offered(self):
        self.assertEqual({"Widget": 0x400},
                         self.load([["Widget", "0x400", "next global"]]))

    def test_a_bound_above_the_cap_is_withheld(self):
        # Three staged buffers of it would cost more BSS than the oracle is
        # worth; MainInterface's real bound is 0x25098, which is 456 KB for one
        # function.
        self.assertEqual({}, self.load([["Huge", hex(bounds.MAX_STAGEABLE + 1),
                                         "next global"]]))

    def test_the_cap_itself_is_allowed(self):
        self.assertEqual({"Edge": bounds.MAX_STAGEABLE},
                         self.load([["Edge", hex(bounds.MAX_STAGEABLE), "x"]]))

    def test_an_absurdly_small_bound_is_withheld(self):
        # Under four bytes cannot be a class with methods, so it is far more
        # likely a mis-derivation than a tiny object - and a too-small bound is
        # the one error that corrupts.
        self.assertEqual({}, self.load([["Tiny", "0x2", "next global"]]))

    def test_a_decimal_bound_is_read(self):
        self.assertEqual({"Widget": 1024},
                         self.load([["Widget", "1024", "next global"]]))

    def test_an_unparseable_row_is_skipped_not_guessed(self):
        found = self.load([["Bad", "not-a-number", "x"],
                           ["Good", "0x40", "x"]])
        self.assertEqual({"Good": 0x40}, found)

    def test_a_missing_catalogue_is_not_an_error(self):
        # The generator must still run where the file has never been derived.
        with tempfile.TemporaryDirectory() as directory:
            self.assertEqual({}, bounds.load(Path(directory) / "absent.csv"))


class CatalogueTests(unittest.TestCase):
    def test_the_committed_catalogue_parses_and_is_all_upper_bounds(self):
        # Guards the shape of the committed file rather than its contents: the
        # contents change whenever the executable's globals move, but every row
        # must always carry a class, a size and the evidence for it.
        if not bounds.BOUNDS_CSV.is_file():
            self.skipTest("no bounds catalogue derived on this machine")
        with bounds.BOUNDS_CSV.open(newline="", encoding="utf-8-sig") as handle:
            rows = list(csv.DictReader(handle))
        self.assertTrue(rows, "the catalogue is empty")
        for row in rows:
            self.assertTrue(row["class"])
            self.assertTrue(row["evidence"],
                            f"{row['class']} carries a size with no evidence")
            self.assertGreaterEqual(int(row["upper_bound"], 0), 4)

    def test_the_committed_catalogue_is_lf(self):
        if not bounds.BOUNDS_CSV.is_file():
            self.skipTest("no bounds catalogue derived on this machine")
        self.assertNotIn(b"\r\n", bounds.BOUNDS_CSV.read_bytes())


# ProvenanceTests stood here: two tests asserting that
# generate_signature_oracles labels a BOUNDED receiver as bounded and never as
# pinned. That generator went with the runtime-oracle route, so both tests have
# been erroring on the import ever since - and the property they guarded (a
# bound must never be presentable as a proof) now has no consumer to guard.
# It comes back with the generator, if the generator comes back.


if __name__ == "__main__":
    unittest.main()
