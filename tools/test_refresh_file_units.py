#!/usr/bin/env python3
"""What `refresh_file_units` must not do to a committed recovery.

The tool rewrites the scaffolding half of a landed FILE-mode unit under a body
nobody re-derived. That is the one edit in this tree that can destroy evidence
without looking like it: the body is untouched and the claim still reads
BYTE_EXACT, while the unit around it no longer compiles to the same bytes. So
the properties pinned here are the safety ones - the split never eats the body,
the annotation survives, and a row that did not improve is restored.
"""
from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import refresh_file_units as tool                             # noqa: E402

UNIT = """// ORIGINAL: 0x00406C00 FILE
// name      ??1Popup@@QAE@XZ
// size      836 bytes
// PRESERVED UNIT - measured NO_COMPILE.

class Popup { public:
    ~Popup();
};

Popup::~Popup() {
    return;
}
"""


class SplitUnitTests(unittest.TestCase):
    def test_the_annotation_block_is_the_header(self):
        header, _ = tool.split_unit(UNIT, "Popup")
        self.assertIn("// ORIGINAL: 0x00406C00 FILE", header)
        self.assertIn("// name      ??1Popup@@QAE@XZ", header)
        # The prose below the facts belongs to the header too: it is what a
        # reader of `src/` sees, and the emitter does not reproduce it.
        self.assertIn("// PRESERVED UNIT", header)
        self.assertNotIn("class Popup", header)

    def test_the_body_is_the_last_definition_not_the_first_mention(self):
        # `~Popup();` inside the shell matches the subject's name too, and
        # splitting there would hand the emitter the shell as the body and
        # drop the recovery on the floor.
        _, body = tool.split_unit(UNIT, "Popup")
        self.assertTrue(body.startswith("Popup::~Popup() {"))
        self.assertIn("return;", body)

    def test_a_unit_with_no_definition_is_refused_not_guessed(self):
        declaration_only = "// name      x\nclass Popup { public:\n"\
                           "    ~Popup();\n};\n"
        self.assertEqual(tool.split_unit(declaration_only, "Popup"),
                         (None, None))

    def test_a_renamed_subject_is_refused(self):
        # `src/unrecovered/00633c90.cpp` lands `sub_633c90` as
        # `Sub633c90Host::remove_entry`. Refusing is right: the tool cannot
        # tell which definition is the recovery.
        self.assertEqual(
            tool.split_unit("// name      sub_633c90\n"
                            "int Host::remove_entry(int a) {\n    return a;\n}\n",
                            "sub_633c90"),
            (None, None))


class RatchetTests(unittest.TestCase):
    """A row may go up a tier or go back to what it was. Never down."""

    def test_only_a_strictly_better_tier_is_kept(self):
        for before, after in (("NO_COMPILE", "MISMATCH"),
                              ("MISMATCH", "BYTE_EXACT"),
                              ("NO_COMPILE", "BYTE_EXACT")):
            self.assertGreater(tool.rank(after), tool.rank(before),
                               f"{before} -> {after}")

    def test_equal_or_worse_is_a_regression(self):
        for before, after in (("NO_COMPILE", "NO_COMPILE"),
                              ("BYTE_EXACT", "MISMATCH"),
                              ("MISMATCH", "NO_COMPILE")):
            self.assertLessEqual(tool.rank(after), tool.rank(before),
                                 f"{before} -> {after}")

    def test_an_unscoreable_tier_is_never_an_improvement(self):
        # SHARED_TAIL and REFUSED are not points on this scale, and a row that
        # lands in one has not improved - it has stopped being measurable.
        for tier in ("SHARED_TAIL", "REFUSED", ""):
            self.assertEqual(tool.rank(tier), -1, tier)
            self.assertLess(tool.rank(tier), tool.rank("NO_COMPILE"), tier)

    def test_the_revert_set_is_read_from_the_before_snapshot(self):
        # Re-deriving it from the live ledger drops every row that improved,
        # because those rows are no longer NO_COMPILE. The run still reverted
        # the right files and reported "0 improved, 72 restored" for a run
        # that improved 44.
        was = {"0x00406C00": "NO_COMPILE"}
        self.assertNotEqual(
            [address for address, _, _ in tool.candidates(tiers=was)],
            [address for address, _, _ in tool.candidates(tiers={})])


class AddressSpellingTests(unittest.TestCase):
    def test_one_spelling_for_a_key_used_by_two_readers(self):
        # `f"0x{...:08X}"` and `.upper()` disagree on the `x`, and a dict
        # keyed both ways finds nothing - which reads as "no candidates".
        self.assertEqual(tool.normalise("0x406c00"), "0x00406C00")
        self.assertEqual(tool.normalise("0X00406C00"), "0x00406C00")
        self.assertEqual(tool.normalise("0x00406C00"), "0x00406C00")


if __name__ == "__main__":
    unittest.main()
