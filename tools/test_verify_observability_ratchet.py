#!/usr/bin/env python3
"""The ratchet must fail on the three ways coverage silently degrades.

A new recovery landing unmeasured, the unobserved count rising, and the census
being truncated all look identical from the outside: a green gate. Each is
covered here, and `checks-can-fail` damages the committed census end to end.
"""
import contextlib
import csv
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path

import verify_observability_ratchet as ratchet

COLUMNS = ["address", "name", "size", "recovery_state", "binary_kind"]


def census(entries, unobserved, unmeasured=0, min_size=200):
    return {"min_size": min_size, "baseline_unobserved": unobserved,
            "baseline_unmeasured": unmeasured, "functions": entries}


def entry(verdict, size=300):
    return {"name": "?f@@YAXXZ", "size": size, "verdict": verdict, "other": []}


class RatchetTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def run_ratchet(self, entries, unobserved, live, unmeasured=0, floor=2):
        census_path = self.root / "observability.json"
        census_path.write_text(json.dumps(census(entries, unobserved,
                                                 unmeasured)), encoding="utf-8")
        functions = self.root / "functions.csv"
        with functions.open("w", newline="", encoding="utf-8") as handle:
            writer = csv.DictWriter(handle, fieldnames=COLUMNS)
            writer.writeheader()
            for address in live:
                writer.writerow({"address": address, "name": "?f@@YAXXZ",
                                 "size": "300",
                                 "recovery_state": "source_complete",
                                 "binary_kind": "game"})
        argv = sys.argv
        sys.argv = ["verify_observability_ratchet.py",
                    "--census", str(census_path),
                    "--functions", str(functions), "--floor", str(floor)]
        try:
            with contextlib.redirect_stdout(io.StringIO()) as out, \
                 contextlib.redirect_stderr(io.StringIO()) as err:
                status = ratchet.main()
            return status, out.getvalue() + err.getvalue()
        finally:
            sys.argv = argv

    def test_a_matching_census_passes(self):
        entries = {"0x1": entry("OBSERVED"), "0x2": entry("UNOBSERVED")}
        self.assertEqual(0, self.run_ratchet(entries, 1, ["0x1", "0x2"])[0])

    def test_a_new_recovery_absent_from_the_census_fails(self):
        # The case every new recovery hits, and the reason this exists.
        entries = {"0x1": entry("OBSERVED"), "0x2": entry("UNOBSERVED")}
        status, output = self.run_ratchet(entries, 1, ["0x1", "0x2", "0x3"])
        self.assertEqual(1, status)
        self.assertIn("not in the census", output)
        self.assertIn("0x3", output)

    def test_unobserved_rising_fails(self):
        entries = {"0x1": entry("UNOBSERVED"), "0x2": entry("UNOBSERVED")}
        status, output = self.run_ratchet(entries, 1, ["0x1", "0x2"])
        self.assertEqual(1, status)
        self.assertIn("must not grow", output)

    def test_unobserved_falling_demands_the_baseline_be_lowered(self):
        # Otherwise a gain is made once and silently given back later.
        entries = {"0x1": entry("OBSERVED"), "0x2": entry("OBSERVED")}
        status, output = self.run_ratchet(entries, 1, ["0x1", "0x2"])
        self.assertEqual(1, status)
        self.assertIn("lock the gain in", output)

    def test_unmeasured_is_ratcheted_too(self):
        # Otherwise "put it in a multi-target source" becomes the way to land a
        # recovery nothing checks.
        entries = {"0x1": entry("NOT-MEASURED"), "0x2": entry("NOT-MEASURED")}
        status, output = self.run_ratchet(entries, 0, ["0x1", "0x2"],
                                          unmeasured=1)
        self.assertEqual(1, status)
        self.assertIn("must not grow", output)

    def test_a_truncated_census_refuses_rather_than_passing(self):
        entries = {"0x1": entry("OBSERVED")}
        status, output = self.run_ratchet(entries, 0, ["0x1"], floor=50)
        self.assertEqual(1, status)
        self.assertIn("below the floor", output)

    def test_an_absent_census_verifies_nothing(self):
        argv = sys.argv
        sys.argv = ["verify_observability_ratchet.py", "--census", "/nope.json"]
        try:
            with contextlib.redirect_stderr(io.StringIO()) as err:
                status = ratchet.main()
        finally:
            sys.argv = argv
        self.assertEqual(1, status)
        self.assertIn("verified NOTHING", err.getvalue())


class CommittedCensusTests(unittest.TestCase):
    def test_the_committed_census_is_internally_consistent(self):
        if not ratchet.DEFAULT_CENSUS.is_file():
            self.skipTest("no committed census")
        data = json.loads(ratchet.DEFAULT_CENSUS.read_text(encoding="utf-8"))
        counted = sum(1 for e in data["functions"].values()
                      if e["verdict"] == "UNOBSERVED")
        self.assertEqual(data["baseline_unobserved"], counted,
                         "baseline_unobserved disagrees with the entries")


if __name__ == "__main__":
    unittest.main()
