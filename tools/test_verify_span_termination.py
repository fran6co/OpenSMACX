#!/usr/bin/env python3
"""A span is a fact about the ORIGINAL, and `// size` is the same fact twice.

The truncation half of this tool needs the image and is exercised by its
damage case. The size half needs nothing but one annotation read against
itself, and it is the half that catches a span being REMOVED - which the
truncation test cannot see, because what is left of a shortened span decodes
cleanly and ends on a terminator.
"""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import emit_translation_unit as emit  # noqa: E402
import verify_span_termination as spans  # noqa: E402


def row(size, ranges, name="f"):
    return {0x401000: {"name": name, "size": size, "body_ranges": ranges}}


class MismeasuredTests(unittest.TestCase):

    def test_a_span_covering_what_the_size_says_is_silent(self):
        self.assertEqual([], spans.mismeasured(
            row("88", "0x00401000-0x00401058")))

    def test_the_end_is_exclusive(self):
        """0x00401000-0x00401058 is 0x58 bytes, not 0x59. Reading it inclusive
        made all 6,000 rows disagree at once, which is how the convention was
        settled rather than assumed."""
        self.assertEqual(1, len(spans.mismeasured(
            row("89", "0x00401000-0x00401058"))))

    def test_a_second_span_counts_towards_the_size(self):
        """402 rows carry a cold block MSVC outlined to 0x0065xxxx."""
        self.assertEqual([], spans.mismeasured(
            row("1568", "0x00401D80-0x0040230F;0x006504E0-0x00650571")))

    def test_deleting_the_second_span_is_caught(self):
        found = spans.mismeasured(row("1568", "0x00401D80-0x0040230F"))
        self.assertEqual(1, len(found))
        self.assertIn("spans covering 1423", found[0])

    def test_a_row_with_no_spans_is_not_scored(self):
        self.assertEqual([], spans.mismeasured(row("88", "")))

    def test_an_unstated_size_is_not_scored(self):
        self.assertEqual([], spans.mismeasured(
            row("?", "0x00401000-0x00401058")))


class TreeTests(unittest.TestCase):
    """Held to zero because it WAS zero: every one of the 6,000 rows agreed at
    185dd977, the commit where `src/` became the store. 60 disagreed by
    2026-08-14, 39 in one generated file and 20 in another, each carrying one
    size repeated over every annotation in it."""

    def test_no_catalogued_row_states_a_size_its_spans_contradict(self):
        self.assertEqual([], spans.mismeasured(emit.load_functions()))


if __name__ == "__main__":
    unittest.main()
