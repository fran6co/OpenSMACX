#!/usr/bin/env python3
"""The pure parts of holding a header's call-graph facts to the image.

The disassembly itself is exercised by the tool's own run over the whole
catalogue and by its damage case in `verify_checks_can_fail`. What is pinned
here is everything that decided a row WRONG for a reason other than the bytes
- each of which called a correct annotation damaged, or a damaged one correct,
while this was being written.
"""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import verify_call_edges as edges  # noqa: E402


def finding(**kwargs):
    fields = dict(address=0x401000, name="f", omitted=[], unconfirmed=[],
                  uncatalogued=[], targets_said=0, targets_are=0,
                  callers_said=0, callers_are=0)
    fields.update(kwargs)
    return edges.Finding(**fields)


class OwnershipTests(unittest.TestCase):
    """The catalogue records the edge to the callee's ENTRY POINT."""

    def owner(self):
        return edges.ownership({
            0x6463D0: {"body_ranges": "0x006463D0-0x006463E4", "_calls": set()},
            0x6463E4: {"body_ranges": "0x006463E4-0x00646444", "_calls": set()},
        })

    def test_a_mid_function_target_is_the_function_that_contains_it(self):
        """`__CIsin` calls 0x006463ED, nine bytes inside 0x006463E4. Comparing
        raw targets called nine CRT float helpers damaged."""
        self.assertEqual(0x6463E4, self.owner()(0x6463ED))

    def test_an_entry_point_is_itself(self):
        self.assertEqual(0x6463E4, self.owner()(0x6463E4))

    def test_an_address_no_span_covers_is_left_alone(self):
        self.assertEqual(0x700000, self.owner()(0x700000))

    def test_the_end_of_a_span_belongs_to_the_next(self):
        """Spans are half-open: `0x006463D0-0x006463E4` is 20 bytes and the
        byte at 0x006463E4 is the next function's first."""
        self.assertEqual(0x6463E4, self.owner()(0x6463E4))


class CallerFloorTests(unittest.TestCase):
    """`// callers` counts more than this can see, so it is a floor."""

    def test_more_recorded_callers_than_edges_is_normal(self):
        # IDA counted the indirect and data references; a sweep of one
        # function's bytes cannot reproduce them.
        self.assertFalse(finding(callers_said=44, callers_are=43)
                         .undercounts_callers)

    def test_fewer_recorded_callers_than_edges_is_impossible(self):
        self.assertTrue(finding(callers_said=0, callers_are=2)
                        .undercounts_callers)

    def test_an_unrecorded_count_is_left_alone(self):
        """`// callers   ?` is how the catalogue spells "never measured"."""
        self.assertFalse(finding(callers_said=-1, callers_are=16)
                         .undercounts_callers)


class ReflowTests(unittest.TestCase):
    """Rewriting the line has to produce something the reader accepts back."""

    def test_a_short_list_stays_on_one_line(self):
        self.assertEqual(["// calls     0x00401000 0x00402000"],
                         edges.reflow(["0x00401000", "0x00402000"]))

    def test_an_empty_list_is_spelled_none(self):
        self.assertEqual(["// calls     (none)"], edges.reflow([]))

    def test_a_long_list_wraps_to_the_value_column(self):
        written = edges.reflow([f"0x004{n:05X}" for n in range(12)])
        self.assertGreater(len(written), 1)
        for line in written[1:]:
            self.assertEqual(len(edges.HEAD), len(line) - len(line[13:]))
            self.assertTrue(line.startswith(edges.CONTINUATION))
        for line in written:
            self.assertLessEqual(len(line), 78)

    def test_what_it_writes_is_what_stamped_reads_back(self):
        import project_catalogue
        tokens = [f"0x006{n:05X}" for n in range(11)] + ["0x005CEB12 (16x)"]
        written = edges.reflow(tokens)
        read = project_catalogue.stamped(["// ORIGINAL: 0x00401000"] + written, 0)
        self.assertEqual(" ".join(tokens), read["calls"])


if __name__ == "__main__":
    unittest.main()
