#!/usr/bin/env python3
"""Unit tests for the Yitzi note reader.

The source is prose, so the parsing is the risk: a line read wrongly gives a
size for the wrong class, and the mapping from its plural English names to this
tree's class names is written by hand. The compile in
`promote_agreed_sizes.py` is what makes a wrong pairing a build failure rather
than a wrong pin, but these keep the reading itself honest.
"""

import shutil
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


class MarkerTest(unittest.TestCase):
    """The marker is read off the `static_assert`, so both numbers come from
    one line and a check can compare them without trusting either."""

    def setUp(self):
        self.work = Path(tempfile.mkdtemp())

    def tearDown(self):
        shutil.rmtree(self.work, ignore_errors=True)

    def write(self, text):
        (self.work / "x.h").write_text(text)
        return yitzi.markers(self.work)

    def test_both_numbers_come_off_one_line(self):
        found = self.write('static_assert(sizeof(Base) == 0x134, "m");'
                           '  // yitzi 0x134\n')
        self.assertEqual(found, {"Base": (0x134, 0x134)})

    def test_a_wrapped_assertion_still_matches(self):
        # Three of the four in the tree put the message on the next line, so
        # the marker sits after the comma with no closing paren before it.
        found = self.write("static_assert(sizeof(Veh) == 0x34,  // yitzi 0x34\n"
                           '              "Veh layout");\n')
        self.assertEqual(found, {"Veh": (0x34, 0x34)})

    def test_an_assertion_with_no_marker_is_not_a_correlation(self):
        self.assertEqual(self.write('static_assert(sizeof(Win) == 0x10, "m");'
                                    "\n"), {})

    def test_the_marker_is_not_read_from_a_neighbouring_line(self):
        # An unanchored search would pair this assertion with the comment
        # below it and report an agreement nobody wrote.
        self.assertEqual(self.write('static_assert(sizeof(Win) == 0x10, "m");\n'
                                    "// yitzi 0x10\n"), {})


class CorrelateTest(unittest.TestCase):
    """Three ways to disagree, and all three must be loud.

    The two that matter are asymmetric in how they arise: a note-against-marker
    mismatch means somebody mistyped the correlation, and a
    marker-against-assert mismatch means the class moved underneath a marker
    nobody revisited. The second is the one that happens on its own.
    """

    def test_all_three_agreeing_is_an_agreement(self):
        agreed, wrong, unmarked = yitzi.correlate(
            {"Base": 0x134}, {"Base": (0x134, 0x134)})
        self.assertEqual((agreed, wrong, unmarked), (["Base"], [], []))

    def test_a_marker_against_the_notes_is_wrong(self):
        _, wrong, _ = yitzi.correlate({"Base": 0x134}, {"Base": (0x134, 0x140)})
        self.assertEqual(len(wrong), 1)
        self.assertIn("0x134", wrong[0][1])

    def test_a_marker_against_its_static_assert_is_wrong(self):
        _, wrong, _ = yitzi.correlate({"Veh": 0x34}, {"Veh": (0x38, 0x34)})
        self.assertEqual(len(wrong), 1)
        self.assertIn("0x38", wrong[0][1])

    def test_a_marker_the_notes_never_stated_is_wrong(self):
        # The failure mode that would otherwise be silent: a marker citing a
        # source that says nothing, which reads as evidence and is not.
        _, wrong, _ = yitzi.correlate({}, {"Base": (0x134, 0x134)})
        self.assertEqual(len(wrong), 1)
        self.assertIn("no size", wrong[0][1])

    def test_a_stated_size_the_tree_cannot_pin_is_unmarked_not_wrong(self):
        # Faction: stated by Yitzi, declared nowhere this tree compiles. An
        # open question is not a defect, and failing on it would force
        # somebody to invent a number to make the gate green.
        agreed, wrong, unmarked = yitzi.correlate({"Faction": 0x20CC}, {})
        self.assertEqual((agreed, wrong, unmarked), ([], [], ["Faction"]))


if __name__ == "__main__":
    unittest.main()
