#!/usr/bin/env python3
"""Unit tests for the base-edge scorer.

The claim comes from Thinker; the verdict comes from the image. What is tested
here is the scoring and the reading of the result, since the disassembly needs
the pinned executable and is exercised by running the tool.

`AGENTS.md` records why an unchecked edge is dangerous: FileWin, StatusWin,
InfoWin and BattleWin were each declared as deriving from the class their
constructor calls into, when in every case it is a MEMBER at a nonzero offset.
Declaring a member as a base puts it at offset zero and moves every field after
it, so `refuted` has to be a verdict this can reach.
"""

import tempfile
import unittest
from pathlib import Path

import derive_base_edges as edges


class RenderTest(unittest.TestCase):
    def test_sorted_with_a_header(self):
        text = edges.render([
            ("Scroll", "GraphicWin", "agreed", "e"),
            ("BasePop", "GraphicWin", "agreed", "e"),
        ])
        rows = text.splitlines()
        self.assertEqual(rows[0], "derived,base,verdict,evidence")
        self.assertEqual([row.split(",")[0] for row in rows[1:]],
                         ["BasePop", "Scroll"])

    def test_the_evidence_survives_the_csv(self):
        text = edges.render([("A", "B", "agreed", "builds a B at 0x00401000")])
        self.assertIn("builds a B at 0x00401000", text)


class AgreedTest(unittest.TestCase):
    """Only confirmed edges may be acted on; the other verdicts must not leak
    into anything that emits a declaration."""

    def setUp(self):
        self.path = Path(tempfile.mkdtemp()) / "base-edges.csv"

    def test_only_agreed_rows_are_returned(self):
        self.path.write_text(edges.render([
            ("Good", "Base", "agreed", "e"),
            ("Bad", "Other", "refuted", "it is a member at 0x30"),
            ("Quiet", "Third", "unchecked", "no constructor"),
        ]))
        self.assertEqual(edges.agreed(self.path), {"Good": "Base"})

    def test_a_missing_file_is_empty_rather_than_an_error(self):
        self.assertEqual(edges.agreed(self.path / "absent.csv"), {})


class VerdictVocabularyTest(unittest.TestCase):
    def test_the_verdicts_are_the_ones_agreed_reads(self):
        # `agreed()` filters on this exact string; a rename in one place and
        # not the other would silently return nothing.
        self.assertEqual(edges.AGREED, "agreed")
        self.assertEqual(edges.UNCHECKED, "unchecked")


if __name__ == "__main__":
    unittest.main()
