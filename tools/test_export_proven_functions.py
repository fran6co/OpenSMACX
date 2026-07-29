#!/usr/bin/env python3
"""Checks for the proven-function inventory.

This file decides what counts as evidence that a recovered function is correct,
so its dangerous failure is publishing a proof that is not one. A number that
overstates coverage is worse than no number: it retires the very question it
was added to keep open.

So most of these tests are about what it REFUSES to publish.
"""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import export_proven_functions as exporter

CATALOGUE = {
    0x00400000: {"address": "0x00400000", "name": "?a@@QAEXXZ", "size": "100",
                 "recovery_state": "source_complete"},
    0x00400100: {"address": "0x00400100", "name": "?b@@QAEXXZ", "size": "200",
                 "recovery_state": "source_complete"},
    0x00400200: {"address": "0x00400200", "name": "sub_400200", "size": "400",
                 "recovery_state": "unrecovered"},
}


class MarkerTests(unittest.TestCase):
    """Proofs are DECLARED. Inference from syntax was tried and was wrong."""

    def markers(self, text: str):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory)
            (source / "thing_oracle.cpp").write_text(text)
            return exporter.marker_addresses(source)

    def test_a_marker_declares_a_proof(self):
        found = self.markers("// PROVEN-AGAINST-ORIGINAL: 0x00400000\n")
        self.assertEqual({0x00400000: "thing_oracle.cpp"}, found)

    def test_a_cast_without_a_marker_is_not_a_proof(self):
        # `reinterpret_cast<Fn>(0x005D4E40U)` is how an oracle CALLS the
        # original, and scraping that pattern was the first thing tried. It
        # picked up 0x697020 and 0x9b8de0, which are a vtable and a global -
        # cast to a pointer type, never called.
        self.assertEqual({}, self.markers(
            "auto f = reinterpret_cast<Fn>(0x00400000U);\n"))

    def test_a_bare_address_is_not_a_proof(self):
        self.assertEqual({}, self.markers("value = 0x00400000U;\n"))

    def test_only_oracle_sources_are_read(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory)
            (source / "helper.cpp").write_text(
                "// PROVEN-AGAINST-ORIGINAL: 0x00400000\n")
            self.assertEqual({}, exporter.marker_addresses(source))


class IslandListTests(unittest.TestCase):
    """The island mechanism's address list lives in CMakeLists.txt."""

    def test_requested_addresses_are_read_from_the_oracle_target(self):
        text = ("extract_legacy_leaves.py\n"
                '  --output "${OPENSMACX_RECOVERY_ORACLE_DIR}"\n'
                "  --max-size 1024\n"
                "  --address 0x004BA830\n"
                "  --address 0x004E2610\n"
                ")\n")
        self.assertEqual({0x004BA830, 0x004E2610},
                         exporter.island_addresses(text))

    def test_a_DIFFERENT_extractor_invocation_is_not_counted(self):
        # This is the bug that shipped. Four custom commands run
        # extract_legacy_leaves.py; only one is the recovery oracle. Matching
        # on the tool name swept up 0x004C5460 from the static-recompile
        # target and published a proof for a function the oracle never builds.
        text = ("extract_legacy_leaves.py\n"
                '  --output "${OPENSMACX_RECOVERY_ORACLE_DIR}"\n'
                "  --address 0x004BA830\n"
                ")\n"
                "extract_legacy_leaves.py\n"
                '  --output "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_DIR}"\n'
                "  --address 0x004C5460\n"
                ")\n")
        found = exporter.island_addresses(text)
        self.assertEqual({0x004BA830}, found)
        self.assertNotIn(0x004C5460, found)

    def test_the_published_list_matches_what_the_oracle_actually_builds(self):
        # The catch that found the bug: the extractor's manifest had 32
        # entries against the 33 this function reported. The manifest is a
        # build artefact so it cannot be an input here, but when it is present
        # the two must agree exactly.
        manifest = (Path(__file__).resolve().parent.parent / "build" /
                    "mingw-i686-release" / "recovery-oracles" / "manifest.json")
        if not manifest.exists():
            self.skipTest("recovery-oracles not built on this host")
        import json
        built = {int(item["address"], 16)
                 for item in json.loads(manifest.read_text())["functions"]}
        cmake = (Path(__file__).resolve().parent.parent / "CMakeLists.txt")
        self.assertEqual(built, exporter.island_addresses(cmake.read_text()))


class RefusalTests(unittest.TestCase):
    """A proof over the wrong kind of function is not a proof."""

    def test_an_oracle_over_an_UNRECOVERED_function_is_refused(self):
        # Most legacy-leaf islands are exactly this: they stand in for code
        # nobody has written. An oracle over one proves the island matches the
        # original, which says nothing about recovered source.
        problems = exporter.verify(CATALOGUE, {0x00400200}, {})
        self.assertEqual(1, len(problems))
        self.assertIn("not source_complete", problems[0])

    def test_an_address_that_is_not_a_function_start_is_refused(self):
        problems = exporter.verify(CATALOGUE, set(), {0x00401234: "x.cpp"})
        self.assertEqual(1, len(problems))
        self.assertIn("not a catalogued function start", problems[0])

    def test_a_refused_address_is_never_published(self):
        rows = exporter.build_rows(CATALOGUE, {0x00400200}, {0x00401234: "x"})
        self.assertEqual([], rows)

    def test_a_valid_proof_is_published(self):
        rows = exporter.build_rows(CATALOGUE, {0x00400000}, {})
        self.assertEqual(1, len(rows))
        self.assertEqual("0x00400000", rows[0]["address"])
        self.assertEqual("legacy_leaf_island", rows[0]["mechanism"])
        self.assertEqual("100", rows[0]["size"])

    def test_both_mechanisms_are_recorded_when_both_apply(self):
        rows = exporter.build_rows(
            CATALOGUE, {0x00400000}, {0x00400000: "win_oracle.cpp"})
        self.assertEqual(1, len(rows))
        self.assertEqual("legacy_leaf_island+hybrid_runtime",
                         rows[0]["mechanism"])
        self.assertIn("win_oracle.cpp", rows[0]["evidence"])

    def test_the_same_function_is_never_counted_twice(self):
        rows = exporter.build_rows(
            CATALOGUE, {0x00400000, 0x00400100}, {0x00400000: "a.cpp"})
        self.assertEqual(2, len(rows))
        self.assertEqual(len({row["address"] for row in rows}), len(rows))


class PublishedFileTests(unittest.TestCase):
    def test_the_committed_inventory_is_current(self):
        """`docs/recovery/proven.csv` must match its inputs.

        It feeds a published percentage, so a stale copy overstates or
        understates coverage silently.
        """
        self.assertEqual(0, exporter.main(["--check"]))


if __name__ == "__main__":
    unittest.main()
