#!/usr/bin/env python3
"""No address may have a body in `src/` AND a body in `src/recovered/`.

`src/recovered/` is for recoveries no file in the tree owns yet. A store copy
alongside a `src/` body is not a spare - it is a SECOND ANSWER for one
address, and nothing downstream knows which to believe:

  * `byte_match_census.py` scores the `src/` copy, through `source_locations`;
  * `byte_match_fanout.py --collect` scores the store copy;
  * both write the same ledger row.

So the recorded tier is whichever tool ran last. Three addresses reached that
state and the ratchet read 702, then 699, then 702 again inside one session.
The swing was diagnosed first as a branch regression and then as stale rows,
and it was neither - two different bodies were being measured under one
address. A number that changes depending on which tool touched it last is
worse than a wrong number, because it looks like evidence of something.

`recovery_state` does not catch this: all three were catalogued `unrecovered`
while carrying a `source_locations`. The location is the field that says
whether a file owns the function, so that is what this checks.
"""

import csv
import pathlib
import unittest

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
FUNCTIONS = REPO_ROOT / "docs" / "recovery" / "functions.csv"
STORE = REPO_ROOT / "src" / "recovered"


class NoDuplicateBodiesTest(unittest.TestCase):
    def test_no_address_has_two_bodies(self):
        if not FUNCTIONS.is_file():
            self.skipTest("docs/recovery/functions.csv is absent")
        stored = {path.stem.upper() for path in STORE.glob("*.cpp")}
        duplicated = []
        with FUNCTIONS.open(newline="", encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                if not (row.get("source_locations") or "").strip():
                    continue
                key = f"{int(row['address'], 16):08X}"
                if key in stored:
                    duplicated.append(
                        f"{row['address']} {row.get('name', '')} "
                        f"is both at {row['source_locations'].split(';')[0]} "
                        f"and in src/recovered/{key.lower()}.cpp")
        self.assertEqual([], duplicated, "\n".join(duplicated))

    def test_every_stored_file_is_named_for_a_catalogued_address(self):
        """A store file whose name is not an address in the catalogue is
        unreachable: nothing will ever score it, so it is neither proof nor
        product and would sit there looking like both."""
        if not FUNCTIONS.is_file():
            self.skipTest("docs/recovery/functions.csv is absent")
        with FUNCTIONS.open(newline="", encoding="utf-8-sig") as handle:
            known = {int(row["address"], 16) for row in csv.DictReader(handle)}
        orphans = []
        for path in STORE.glob("*.cpp"):
            try:
                address = int(path.stem, 16)
            except ValueError:
                orphans.append(f"{path.name} is not named for an address")
                continue
            if address not in known:
                orphans.append(f"{path.name} names no catalogued function")
        self.assertEqual([], orphans, "\n".join(orphans))


if __name__ == "__main__":
    unittest.main()
