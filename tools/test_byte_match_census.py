#!/usr/bin/env python3
"""Pin the census's unit construction, especially what it REFUSES.

The census writes into `.opensmacx/byte-match.csv`, which is a SHARED ledger:
`agent_brief`, `decomp_status`, `writeback` and `classify_casts` all read or
write it. A refusal here is therefore not a row this tool skipped, it is a
verdict every other tool inherits - so a refusal that is really a gap in this
file's own reader is a false demotion published to the whole project.

That happened. Measured 2026-08-14: one census run turned 606 rows other tools
had scored BYTE_EXACT into REFUSED, 1,727 in total, all with the reason
"extract does not end in a closing brace" - because a FILE-mode recovery has
no body to extract. `verify_recovered_function` had already learned FILE mode
for its own path and this file had not, which is the same split that produced
the FILE-marker bug and the `_catalog_facts` drift.

The tests below are the mechanical form of that lesson: they do not check that
FILE-mode rows are scored WELL, only that this file never again turns one into
a refusal on account of its own extractor.
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match_census as tool  # noqa: E402


class FileModeTests(unittest.TestCase):
    def setUp(self):
        tool.file_mode_units.cache_clear()
        self.units = tool.file_mode_units()

    def test_the_tree_has_file_mode_recoveries_to_get_wrong(self):
        # Without this the two tests below pass vacuously the day the last
        # FILE-mode recovery is rewritten, and the regression comes back
        # unobserved. This repository has published a vacuous proof before.
        self.assertGreater(len(self.units), 100)

    def test_a_file_mode_unit_is_the_file_and_is_not_refused(self):
        address = sorted(self.units)[0]
        text, refusal = tool.build_unit(
            address, {"name": "", "size": ""}, f"{self.units[address]}:1",
            {}, {}, {}, None)
        self.assertEqual("", refusal)
        self.assertIsNotNone(text)
        # The FILE is the translation unit: it carries its own annotation,
        # typedefs and declarations, and nothing is prepended to it.
        self.assertEqual(
            (tool.REPO_ROOT / self.units[address]).read_text(errors="ignore"),
            text)

    def test_no_file_mode_row_refuses_for_want_of_a_closing_brace(self):
        """The exact false demotion, over the whole population.

        `build_unit` is cheap for these - a dictionary hit and a file read -
        so this can afford to be exhaustive rather than sample one row.
        """
        for address, path in sorted(self.units.items())[:400]:
            _, refusal = tool.build_unit(
                address, {"name": "", "size": ""}, f"{path}:1",
                {}, {}, {}, None)
            self.assertNotIn("closing brace", refusal,
                             f"0x{address:08X} in {path}")

    def test_a_body_mode_row_still_goes_through_the_extractor(self):
        # The fix must not swallow the ordinary path: a row with no FILE
        # annotation has a body to extract and a location to extract it from,
        # and an unusable location still has to refuse.
        _, refusal = tool.build_unit(
            0xDEADBEEF, {"name": "", "size": ""}, "", {}, {}, {}, None)
        self.assertEqual("no source_locations; not censusable", refusal)


if __name__ == "__main__":
    unittest.main()
