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

import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match_census as tool  # noqa: E402
import emit_translation_unit as emit  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent


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


class BodyDefinedClassTests(unittest.TestCase):
    """A class the BODY defines is not the scaffold's to define.

    The same split this file's docstring is about, one path over. `writeback`
    passes the body to `emit` and then runs the two collision filters over the
    scaffolding; this file did neither, so the gate compiled a different unit
    from the one the agent measured. A body carrying its own `VCall` shim with
    typed slots - which agents write constantly, because the generated one
    declares every slot nullary - compiled under `--body`/`--dir` and died
    here with `C2011: 'VCall' : 'class' type redefinition`.

    Measured 2026-08-14: fixing it moved 28 committed bodies out of
    NO_COMPILE with nothing moving the other way.
    """

    @classmethod
    def setUpClass(cls):
        # CENSUS-RECIPE ROWS ONLY. `decomp_status` builds a "writeback"
        # annotation through `writeback.build_unit` and only a "census" one
        # through this file, so sampling every body-mode row would test this
        # builder against addresses the gate never hands it - a check that
        # can fail for a path it does not guard, and pass while the path it
        # does guard is broken.
        import annotation_scan
        import byte_match
        import pefile
        cls.functions = emit.load_functions()
        cls.derived, cls.callees = emit.load_derived(), emit.load_callees()
        cls.pe = pefile.PE(str(byte_match.DEFAULT_EXE), fast_load=True)
        # A BODY DEFINING A CLASS IS NOT YET A COLLISION. The first version of
        # this test sampled those and passed with the filter deleted, because
        # none of the twelve it happened to draw defined a class the SCAFFOLD
        # also defines - a check that cannot fail, which is the shape this
        # repository has published before. So the sample is built from the
        # collision itself: scaffold and body defining one name.
        cls.collisions = []
        for annotation in annotation_scan.scan_tree():
            if annotation.recipe != "census" or len(cls.collisions) >= 6:
                continue
            row = cls.functions.get(annotation.address)
            if row is None:
                continue
            location = f"{annotation.path}:{annotation.line}"
            try:
                body = tool.extract_body(location)
            except (ValueError, OSError):
                continue
            defined = emit.classes_defined_in(body)
            if not defined:
                continue
            try:
                scaffolding = emit.emit(annotation.address, cls.functions,
                                        cls.derived, cls.callees, cls.pe,
                                        scaffolding_only=True, body=body)
            except emit.Unsettled:
                continue
            shared = defined & emit.classes_defined_in(scaffolding)
            if shared:
                cls.collisions.append((annotation.address, row, location, shared))

    def test_the_tree_has_a_real_collision_to_get_wrong(self):
        # Non-vacuity, proved rather than assumed: at least one committed body
        # defines a class its own scaffold also defines. Without this the test
        # below passes the day the population no longer contains the shape.
        self.assertGreater(len(self.collisions), 2)

    def test_no_unit_defines_a_class_its_own_body_defines(self):
        for address, row, location, shared in self.collisions:
            text, refusal = tool.build_unit(
                address, row, location, self.functions, self.derived,
                self.callees, self.pe)
            self.assertTrue(text, f"0x{address:08X} refused: {refusal}")
            for name in shared:
                self.assertEqual(
                    1, len(re.findall(
                        rf"^(?:class|struct)\s+{re.escape(name)}\s*(?::[^{{;]*)?"
                        r"\{", text, re.M)),
                    f"0x{address:08X}: {name} is not defined exactly once in "
                    f"the unit; the body defines it, so a scaffold or preamble "
                    f"definition beside it is C2011 and kills the unit")


if __name__ == "__main__":
    unittest.main()
