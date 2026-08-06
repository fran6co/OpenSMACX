#!/usr/bin/env python3
"""Unit tests for the Yitzi note reader.

The source is prose, so the parsing is the risk: a line read wrongly gives a
size for the wrong class, and the mapping from its plural English names to this
tree's class names is written by hand. The compile in
`promote_agreed_sizes.py` is what makes a wrong pairing a build failure rather
than a wrong pin, but these keep the reading itself honest.
"""

import tempfile
import unittest
from pathlib import Path

import correlate_yitzi_notes as yitzi


def notes(text):
    path = Path(tempfile.mkdtemp()) / "variables.txt"
    path.write_text(text)
    return path


class SizeReadingTest(unittest.TestCase):
    def test_the_three_spellings_the_notes_actually_use(self):
        path = notes("\n".join([
            "Factions: Size 20CC.  Faction 0 would have:",
            "Bases: Size is 134, begins at 97D040",
            "Units: At 952828, size 34",
        ]))
        self.assertEqual(yitzi.sizes(path),
                         {"Faction": 0x20CC, "Base": 0x134, "Veh": 0x34})

    def test_sizes_are_hexadecimal(self):
        # "size 34" is 0x34, not 34. The notes are written in hex throughout,
        # and Base at 0x134 agrees with both the pinned size and the IDB.
        self.assertEqual(yitzi.sizes(notes("Units: At 952828, size 34"))["Veh"],
                         0x34)

    def test_a_record_with_no_mapping_is_ignored(self):
        # Unmapped names must not be guessed at - a wrong pairing would pin
        # the wrong class.
        self.assertEqual(yitzi.sizes(notes("Widgets: Size 40")), {})

    def test_a_line_without_a_size_is_ignored(self):
        self.assertEqual(yitzi.sizes(notes("Bases: begins at 97D040")), {})

    def test_a_missing_file_is_empty_rather_than_an_error(self):
        self.assertEqual(yitzi.sizes(Path("/nonexistent/variables.txt")), {})


class ControlTest(unittest.TestCase):
    def setUp(self):
        self.pinned = yitzi.thinker.pinned_sizes

    def tearDown(self):
        yitzi.thinker.pinned_sizes = self.pinned

    def test_agreement_with_a_pinned_size_counts_as_right(self):
        yitzi.thinker.pinned_sizes = lambda: {"Base": 0x134}
        right, wrong = yitzi.control({"Base": 0x134})
        self.assertEqual((right, wrong), (["Base"], []))

    def test_disagreement_is_reported(self):
        yitzi.thinker.pinned_sizes = lambda: {"Base": 0x134}
        right, wrong = yitzi.control({"Base": 0x140})
        self.assertEqual(wrong, [("Base", 0x140, 0x134)])

    def test_an_unpinned_class_is_neither(self):
        yitzi.thinker.pinned_sizes = lambda: {}
        self.assertEqual(yitzi.control({"Veh": 0x34}), ([], []))


class RenderTest(unittest.TestCase):
    def test_sorted_with_a_header_and_hex_sizes(self):
        text = yitzi.render({"Veh": 0x34, "Base": 0x134})
        rows = text.splitlines()
        self.assertEqual(rows[0], "class,size,evidence")
        self.assertEqual([row.split(",")[0] for row in rows[1:]],
                         ["Base", "Veh"])
        self.assertIn("0x134", rows[1])


if __name__ == "__main__":
    unittest.main()
