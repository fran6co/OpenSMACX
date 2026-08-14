#!/usr/bin/env python3
"""What counts as a fact about a function, and what is only prose beside one.

`src/` IS the catalogue. Every consumer - the emitter, the census, the
frontier, the prototype derivation - reads a function's name, size, spans and
call edges out of the comment block under its `ORIGINAL:` marker. So the rule
for "which line is a fact" is not a formatting nicety: it decides what the
tree believes about the binary.

It was loose enough to read a sentence. A RULED-OUT continuation line is
`//            calls need, so this is landed FILE-mode with ...`, the pattern
allowed any indentation before the key, and the last match won - so
0x00402DD0's single call edge became that sentence, silently, and the
catalogue reported the function calls nothing.
"""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan  # noqa: E402
import project_catalogue as catalogue  # noqa: E402


class StampedTests(unittest.TestCase):

    def stamp(self, *lines):
        return catalogue.stamped(["// ORIGINAL: 0x00402DD0 FILE", *lines], 0)

    def test_the_block_is_read(self):
        found = self.stamp("// name      sub_402dd0",
                           "// size      279 bytes",
                           "// spans     0x00402DD0-0x00402ED2",
                           "// calls     0x00402970")
        self.assertEqual("sub_402dd0", found["name"])
        self.assertEqual("0x00402970", found["calls"])

    def test_prose_that_starts_with_a_key_is_not_a_fact(self):
        """0x00402DD0, verbatim. The sentence is a RULED-OUT continuation."""
        found = self.stamp(
            "// calls     0x00402970",
            "// RULED-OUT: the vtable-patch teardown is more than the two",
            "//            calls need, so this is landed FILE-mode with edits")
        self.assertEqual("0x00402970", found["calls"])

    def test_prose_above_the_block_does_not_win_either(self):
        """0x0041A7E0 has its RULED-OUT note ABOVE the facts, so taking the
        first match blind to indentation moves the defect rather than fixing
        it. The key has to be at column 3, where `facts` writes it."""
        found = self.stamp(
            "// RULED-OUT: the multiply is expressed as the branchless",
            "//            calls as the branchless cdq/xor/sub sequence",
            "// name      sub_41a7e0",
            "// calls     0x004EC3B0 0x005D5A70")
        self.assertEqual("0x004EC3B0 0x005D5A70", found["calls"])

    def test_a_second_spelling_never_overwrites_the_first(self):
        found = self.stamp("// kind      game", "// kind      thunk")
        self.assertEqual("game", found["kind"])

    def test_an_empty_value_is_present_and_empty(self):
        """`// prototype` with the trailing space stripped is how 71 rows
        spell "no recorded prototype". Absent and empty must not differ, or a
        whitespace edit reads as a fact going missing."""
        self.assertEqual("", self.stamp("// prototype")["prototype"])
        self.assertEqual("", self.stamp("// prototype ")["prototype"])

    def test_a_wrapped_edge_list_is_one_value(self):
        """0x0041E790 carries 16 call targets over three lines. Reading only
        the first said 6, and the batch that merely re-flowed the line read as
        an agent deleting ten edges."""
        found = self.stamp("// calls     0x0040C850 0x0040E360 0x0040F0F0",
                           "//           0x004103B0 0x00412AA0",
                           "//           0x0045C290")
        self.assertEqual(6, len(found["calls"].split()))

    def test_a_multiplicity_survives_the_wrap(self):
        found = self.stamp("// calls     0x005CEB12 (16x)",
                           "//           0x00644EF2")
        self.assertEqual("0x005CEB12 (16x) 0x00644EF2", found["calls"])

    def test_prose_at_the_value_column_is_not_a_continuation(self):
        """Alignment alone admits a sentence. 0x004C2360 has a RULED-OUT
        continuation that lands on exactly the value column."""
        found = self.stamp(
            "// calls     0x005D4510",
            "//           `ret` and one path returns with eax = the last a6.")
        self.assertEqual("0x005D4510", found["calls"])

    def test_an_address_at_the_wrong_column_is_not_a_continuation(self):
        found = self.stamp("// calls     0x005D4510",
                           "//                    0x00644EF2")
        self.assertEqual("0x005D4510", found["calls"])

    def test_only_an_edge_list_continues(self):
        """`name`, `size` and `prototype` are never wrapped, so a hex-looking
        line under one of them is something else."""
        found = self.stamp("// prototype void (__cdecl f)(int)",
                           "//           0x00644EF2")
        self.assertEqual("void (__cdecl f)(int)", found["prototype"])

    def test_the_run_ends_at_the_first_line_of_code(self):
        found = self.stamp("// name      sub_402dd0",
                           "void sub_402dd0() {}",
                           "// calls     0x00402970")
        self.assertNotIn("calls", found)


class KeepProseTests(unittest.TestCase):
    """What `--apply` removes before writing a fresh block under a marker."""

    def test_the_fact_block_goes(self):
        self.assertEqual([], catalogue.keep_prose(
            ["// name      f\n", "// calls     0x00401000\n"]))

    def test_a_wrapped_edge_list_goes_with_it(self):
        """Left behind, the continuation reads as part of the FRESH `// calls`
        line stamped above it, and the addresses are said twice."""
        self.assertEqual([], catalogue.keep_prose(
            ["// calls     0x00401000 0x00402000\n",
             "//           0x00403000\n"]))

    def test_prose_stays_in_order(self):
        self.assertEqual(["// RULED-OUT: the vtable-patch teardown\n",
                          "//            calls need, so this is FILE-mode\n"],
                         catalogue.keep_prose(
                             ["// name      f\n",
                              "// RULED-OUT: the vtable-patch teardown\n",
                              "//            calls need, so this is FILE-mode\n"]))


class TreeTests(unittest.TestCase):
    """The rule held to the tree it reads, not to fixtures alone."""

    @classmethod
    def setUpClass(cls):
        cls.resolved, _ = annotation_scan.resolve(annotation_scan.scan_tree())
        cls.rows = catalogue.from_source()

    def test_every_annotation_reaches_the_catalogue(self):
        """A marker whose fact block is gone is not a row with holes - it is
        no row at all, because `from_source` skips an annotation `stamped`
        reads nothing from. Three files lost their whole block in batch 9 and
        the addresses simply left the catalogue; the loss was found months
        later by bisecting an unrelated ratchet, which is not a mechanism."""
        lost = [hex(one.address) for one in self.resolved
                if one.address not in self.rows]
        self.assertEqual([], lost)

    def test_every_row_carries_the_facts_its_consumers_read(self):
        without = {key: [hex(address) for address, row in self.rows.items()
                         if not row.get(field)]
                   for key, field in (("name", "name"), ("size", "size"),
                                      ("spans", "body_ranges"))}
        self.assertEqual({"name": [], "size": [], "spans": []}, without)


if __name__ == "__main__":
    unittest.main()
