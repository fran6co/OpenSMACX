#!/usr/bin/env python3
"""Unit tests for the structural observation log.

The point of the log is that a fact found while recovering one function is
still available when someone touches the same class later. That only works if
the rows stay groupable and stay evidenced, so those are what is tested - plus
the committed file, because a catalogue that has drifted out of its own schema
is worse than no catalogue.
"""

import unittest

import structure_observations as tool


def row(**overrides):
    base = {"address": "0x00401000", "kind": "member", "subject": "Win",
            "offset": "0x40", "detail": "a flag byte",
            "evidence": "mov byte ptr [ecx+0x40], 0", "status": "open"}
    base.update(overrides)
    return base


class ValidationTest(unittest.TestCase):
    def test_a_well_formed_row_passes(self):
        tool.validate([row()])

    def test_an_unknown_kind_is_refused(self):
        # The vocabulary is deliberately small: a kind invented per row stops
        # the log being groupable, which is the only reason it beats prose.
        with self.assertRaises(tool.Invalid):
            tool.validate([row(kind="thing-i-noticed")])

    def test_a_row_without_evidence_is_refused(self):
        # This sits beside catalogues fed from external analysis, whose value
        # is that a reader can tell a measurement from a guess.
        with self.assertRaises(tool.Invalid):
            tool.validate([row(evidence="")])

    def test_a_row_without_a_subject_is_refused(self):
        with self.assertRaises(tool.Invalid):
            tool.validate([row(subject="   ")])

    def test_an_unknown_status_is_refused(self):
        with self.assertRaises(tool.Invalid):
            tool.validate([row(status="maybe")])

    def test_a_negative_offset_is_allowed(self):
        # `[ecx-0x1c]` means the declared class is a SUBOBJECT inside a larger
        # one - inheritance the declaration does not model. That is the most
        # informative observation there is, and the first version of this
        # validator threw it away.
        tool.validate([row(offset="-0x1C")])

    def test_a_decimal_offset_is_refused(self):
        # Every other offset in this repository is hex; one decimal row is
        # how a promotion silently lands a field 42 bytes from where it is.
        with self.assertRaises(tool.Invalid):
            tool.validate([row(offset="64")])

    def test_a_decimal_address_is_refused(self):
        with self.assertRaises(tool.Invalid):
            tool.validate([row(address="4198400")])

    def test_an_empty_address_is_allowed(self):
        # Emitter observations are about the tooling, not about one function.
        tool.validate([row(address="", kind="emitter", offset="")])

    def test_an_unexpected_column_is_refused(self):
        bad = row()
        bad["guess"] = "yes"
        with self.assertRaises(tool.Invalid):
            tool.validate([bad])


class CommittedFileTest(unittest.TestCase):
    def test_the_committed_log_is_valid(self):
        if not tool.OBSERVATIONS.is_file():
            self.skipTest("no observation log yet")
        tool.validate(tool.load())

    def test_every_kind_in_use_is_documented(self):
        if not tool.OBSERVATIONS.is_file():
            self.skipTest("no observation log yet")
        for entry in tool.load():
            self.assertIn(entry["kind"], tool.KINDS)


if __name__ == "__main__":
    unittest.main()
