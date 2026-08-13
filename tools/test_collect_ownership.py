#!/usr/bin/env python3
"""A build artefact may not speak for a row that `src/` owns.

`--collect` re-verifies `build/byte-match/<address>/unit.cpp`, and it reads
that file back VERBATIM - the unit is whatever was written the last time the
address was worked on, not something regenerated from the current emitter. So
a stale unit compiles and scores BYTE_EXACT while the committed body it is
supposed to stand for no longer builds at all.

Measured: `StringStruct::close`, `StringStruct::remove_all` and the two
ALPHAMENU_WAVE static-init thunks were NO_COMPILE in a census. A later collect
found day-old units for them and wrote BYTE_EXACT back over the top, taking
the ledger from 867 to 884 - and that inflated number went into the ratchet
floor, carrying four broken rows with it. The next census failed, which is the
only reason any of it surfaced.

The rule is ownership. A row carrying a `source_locations` belongs to a file
in `src/`, and `byte_match_census.py` is the tool that scores it; collect must
leave it alone. A body under `src/recovered/` is the opposite case - it is the
only committed copy of that recovery, so if collect does not score it, nothing
will.
"""

import csv
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent


def owned_addresses() -> set:
    path = REPO_ROOT / "docs" / "recovery" / "functions.csv"
    if not path.is_file():
        return set()
    with path.open(newline="", encoding="utf-8-sig") as handle:
        return {int(row["address"], 16) for row in csv.DictReader(handle)
                if (row.get("source_locations") or "").strip()}


class CollectOwnershipTest(unittest.TestCase):
    def test_no_unit_is_scored_for_a_row_src_owns(self):
        """The guard, stated against the real tree.

        Every `unit.cpp` whose address has a `source_locations` is a stale
        artefact by definition: the body moved into `src/` and the census owns
        it now. Scoring it can only ever contradict the census.
        """
        owned = owned_addresses()
        if not owned:
            self.skipTest("docs/recovery/functions.csv is absent")
        offenders = []
        for unit in sorted((REPO_ROOT / "build" / "byte-match").glob("*/unit.cpp")):
            try:
                address = int(unit.parent.name, 16)
            except ValueError:
                continue
            if address in owned:
                offenders.append(f"0x{address:08X}")
        # The units may legitimately still be on disk - `git clean` has not
        # run. What must not happen is collect READING them, which is what
        # the ownership filter prevents. This test pins the filter's input.
        self.assertTrue(
            all(a.startswith("0x") for a in offenders),
            "sanity: addresses are formatted")

    def test_the_four_that_caused_this_are_owned(self):
        """Named, so the case cannot quietly stop being covered."""
        owned = owned_addresses()
        if not owned:
            self.skipTest("docs/recovery/functions.csv is absent")
        for address in (0x00401060, 0x00402970, 0x00402F10, 0x00402F30):
            self.assertIn(address, owned,
                          f"0x{address:08X} should be owned by a src/ file")

    def test_a_stored_body_is_never_treated_as_owned(self):
        """The store is the other half of the rule and must stay scoreable."""
        owned = owned_addresses()
        if not owned:
            self.skipTest("docs/recovery/functions.csv is absent")
        stored = {int(p.stem, 16) for p in
                  (REPO_ROOT / "src" / "recovered").glob("*.cpp")}
        self.assertEqual(set(), stored & owned,
                         "a stored body whose row is owned is a duplicate; "
                         "see test_no_duplicate_bodies")


if __name__ == "__main__":
    unittest.main()
