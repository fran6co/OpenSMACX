#!/usr/bin/env python3
"""Tests for the boot loop's parsing and its no-op detector.

The detector is the reason this tool exists. Task #32's loop is build-boot-repeat,
and the failure mode is a change that measures identically to no change being read
as progress - which has happened twice in this project, both times caught only by
a number that failed to move. So "the stop address is UNCHANGED" has to be said
out loud, and the two evidence classes must never be summed: a real stop is a
statement about how far the image gets, a surveyed skip returned 0 to its caller
and is only a work item.
"""

from __future__ import annotations

import json
import sys
import tempfile
import unittest
import unittest.mock
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lifted_boot_loop as loop  # noqa: E402


class ParseTests(unittest.TestCase):
    def test_a_real_stop_is_read_as_a_stop(self):
        found = loop.parse("boot: entry 0x0045f950\nBOOT-STOPPED-AT 0x0064531A\n")
        self.assertEqual("0x0064531a", found["stopped_at"])
        self.assertEqual([], found["surveyed"])
        self.assertFalse(found["timed_out"])

    def test_surveyed_lines_are_collected_with_their_names(self):
        found = loop.parse(
            "SURVEY-REACHED 0x0064531a __onexit\n"
            "SURVEY-REACHED 0x0064fc88 __itoa\n")
        self.assertEqual(
            [("0x0064531a", "__onexit"), ("0x0064fc88", "__itoa")],
            found["surveyed"])
        # A surveyed run has no stop address, and inventing one from a survey
        # line would turn a work queue into a claim about the boot.
        self.assertIsNone(found["stopped_at"])

    def test_a_timeout_is_not_reported_as_a_stop(self):
        # Reaching something that loops is a different result from stopping, and
        # collapsing the two would make a hang look like a work item.
        found = loop.parse("boot: entry\nBOOT-TIMED-OUT\n")
        self.assertTrue(found["timed_out"])
        self.assertIsNone(found["stopped_at"])

    def test_the_CRT_range_report_is_parsed(self):
        found = loop.parse(
            "boot: .CRT$XI [0x006826d0,0x006826e4) ran 4, 0 catalogued ??__E, "
            "1 null\n"
            "boot: .CRT$XC [0x00682000,0x006826cc) ran 434, 392 catalogued "
            "??__E, 1 null\n")
        self.assertEqual(2, len(found["ranges"]))
        self.assertEqual("XI", found["ranges"][0]["section"])
        self.assertEqual(4, found["ranges"][0]["ran"])
        # The measurement that matters: XI runs entries the ??__E filter cannot
        # see, which is why the old heuristic missed ___onexitinit entirely.
        self.assertEqual(0, found["ranges"][0]["catalogued"])
        self.assertEqual(434, found["ranges"][1]["ran"])
        self.assertEqual(392, found["ranges"][1]["catalogued"])


class NoOpDetectorTests(unittest.TestCase):
    def _run(self, first: str, second: str):
        with tempfile.TemporaryDirectory() as directory:
            state = Path(directory) / "state.json"
            frontier = Path(directory) / "frontier.tsv"
            outputs = []
            for text in (first, second):
                with unittest.mock.patch.object(
                        loop, "run_boot", return_value=text), \
                        unittest.mock.patch("sys.stdout") as out:
                    loop.main(["--boot", __file__, "--state", str(state),
                               "--frontier", str(frontier)])
                outputs.append("".join(
                    str(call.args[0]) for call in out.write.call_args_list))
            # Read inside the block: the temporary directory is removed on exit,
            # so returning the path would hand back something already deleted.
            remembered = json.loads(state.read_text(encoding="utf-8"))
            return outputs, remembered

    def test_an_UNCHANGED_stop_address_is_called_a_NO_OP(self):
        outputs, _ = self._run("BOOT-STOPPED-AT 0x0064531A\n",
                               "BOOT-STOPPED-AT 0x0064531A\n")
        self.assertIn("UNCHANGED", outputs[1])
        self.assertIn("NO-OP", outputs[1])

    def test_a_MOVED_stop_address_says_where_it_came_from(self):
        outputs, _ = self._run("BOOT-STOPPED-AT 0x0064531A\n",
                               "BOOT-STOPPED-AT 0x0064FC88\n")
        self.assertIn("MOVED", outputs[1])
        self.assertIn("0x0064531a", outputs[1])
        self.assertNotIn("NO-OP", outputs[1])

    def test_the_first_run_claims_neither(self):
        outputs, _ = self._run("BOOT-STOPPED-AT 0x0064531A\n",
                               "BOOT-STOPPED-AT 0x0064531A\n")
        self.assertNotIn("UNCHANGED", outputs[0])
        self.assertNotIn("MOVED", outputs[0])

    def test_the_state_file_remembers_the_stop_address(self):
        _, remembered = self._run("BOOT-STOPPED-AT 0x0064531A\n",
                                  "BOOT-STOPPED-AT 0x0064FC88\n")
        self.assertEqual("0x0064fc88", remembered["stopped_at"])


class SurveyEvidenceTests(unittest.TestCase):
    def test_a_surveyed_run_says_it_is_not_a_boot(self):
        with tempfile.TemporaryDirectory() as directory:
            frontier = Path(directory) / "frontier.tsv"
            with unittest.mock.patch.object(
                    loop, "run_boot",
                    return_value="SURVEY-REACHED 0x0064531a __onexit\n"), \
                    unittest.mock.patch("sys.stdout") as out:
                loop.main(["--boot", __file__,
                           "--state", str(Path(directory) / "s.json"),
                           "--frontier", str(frontier), "--survey"])
            printed = "".join(str(call.args[0])
                              for call in out.write.call_args_list)
            self.assertIn("WORK QUEUE, not a boot", printed)
            rows = frontier.read_text().splitlines()
            self.assertEqual("address\tname", rows[0])
            self.assertEqual("0x0064531a\t__onexit", rows[1])

    def test_duplicate_survey_lines_collapse_to_one_frontier_row(self):
        # One routine called from twenty call sites is one work item.
        with tempfile.TemporaryDirectory() as directory:
            frontier = Path(directory) / "frontier.tsv"
            with unittest.mock.patch.object(
                    loop, "run_boot",
                    return_value="SURVEY-REACHED 0x1 a\n" * 3), \
                    unittest.mock.patch("sys.stdout"):
                loop.main(["--boot", __file__,
                           "--state", str(Path(directory) / "s.json"),
                           "--frontier", str(frontier), "--survey"])
            self.assertEqual(2, len(frontier.read_text().splitlines()))


if __name__ == "__main__":
    unittest.main()
