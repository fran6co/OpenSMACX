#!/usr/bin/env python3
"""Reading a class's vtable off its constructor.

Three rules carry this derivation, and each of them exists because the simpler
version was tried against the image and gave a wrong answer. They are pinned
here on synthetic stores, and the whole tree is measured by the tool's own
`--check`.
"""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import derive_class_vtables as vtables  # noqa: E402


class SharedSpanTests(unittest.TestCase):
    """A span two rows claim is COMDAT-folded and belongs to neither.

    `??0Sound@@`, `??0Wave@@`, `??0Ambience@@`, `??0VoiceRx@@` and
    `??0VoiceTx@@` all list 0x004C8450-0x004C8457 - seven bytes installing a
    shared base's vptr. Scanning it once per claimant made six classes appear
    to share 0x0066E444, and that was the derivation's only collision.
    """

    def rows(self):
        return {
            0x4C6080: {"body_ranges": "0x004C6080-0x004C611A;0x004C8450-0x004C8457"},
            0x4C66E0: {"body_ranges": "0x004C66E0-0x004C67B6;0x004C8450-0x004C8457"},
            0x4C8410: {"body_ranges": "0x004C8410-0x004C8442"},
        }

    def test_a_span_two_rows_claim_is_shared(self):
        self.assertEqual({(0x4C8450, 0x4C8457)},
                         vtables.shared_spans(self.rows()))

    def test_a_span_one_row_claims_is_not(self):
        self.assertNotIn((0x4C8410, 0x4C8442),
                         vtables.shared_spans(self.rows()))

    def test_one_range_written_two_ways_is_still_one_range(self):
        """Keyed on the TEXT, a different hex case reads as two ranges, so
        neither is disowned and the folded tail's vptr store goes to both."""
        self.assertEqual({(0x4C8450, 0x4C8457)}, vtables.shared_spans({
            1: {"body_ranges": "0x004C8450-0x004C8457"},
            2: {"body_ranges": "0x004c8450-0x004c8457"}}))

    def test_a_tree_with_no_sharing_has_none(self):
        self.assertEqual(set(), vtables.shared_spans(
            {1: {"body_ranges": "0x00401000-0x00401010"}}))


class ClassificationTests(unittest.TestCase):
    """What a vptr at a non-zero offset can and cannot be shown to be."""

    def derivation(self):
        found = vtables.Derivation()
        found.vtable = {
            ("AlphaMovie", 0x0): 0x669458,
            ("AlphaMovie", 0x444): 0x669450,
            ("Win", 0x0): 0x66FDD0,
            ("BasePop", 0x0): 0x66C000,
            ("BasePop", 0x2154): 0x66FDD0,
        }
        return found

    def test_offset_zero_is_the_class_itself(self):
        self.assertEqual("own primary",
                         self.derivation().classify("AlphaMovie", 0))

    def test_another_class_s_primary_is_a_subobject(self):
        """`??0BasePop@@` writes Win's vtable at +0x2154, so there is an
        inlined Win constructor there - a base or an embedded member."""
        self.assertEqual("subobject",
                         self.derivation().classify("BasePop", 0x2154))

    def test_a_vtable_no_class_claims_is_left_unattributed(self):
        """NOT called "the class's second vtable". No constructor stores it at
        offset 0, so it is either a multiple-inheritance secondary of this
        class or an embedded member of a class the catalogue does not name a
        constructor for, and this evidence cannot separate them."""
        self.assertEqual("unattributed",
                         self.derivation().classify("AlphaMovie", 0x444))

    def test_the_distance_from_the_primary_is_reported(self):
        # MSVC emits a class's vtable group adjacently; a hint, not a verdict.
        self.assertEqual(-8, self.derivation().distance("AlphaMovie", 0x444))

    def test_a_class_with_no_primary_has_no_distance(self):
        found = vtables.Derivation()
        found.vtable = {("Orphan", 0x10): 0x669000}
        self.assertIsNone(found.distance("Orphan", 0x10))


class CollisionTests(unittest.TestCase):
    """Distinct classes have distinct vtables - the property the retired
    `vtables.csv` violated 15 ways for one address."""

    def test_two_classes_claiming_one_vtable_is_a_collision(self):
        found = vtables.Derivation()
        found.vtable = {("Sound", 0): 0x66E444, ("Wave", 0): 0x66E444}
        self.assertEqual({0x66E444: ["Sound", "Wave"]}, found.collisions())

    def test_one_class_with_several_vtables_is_not(self):
        found = vtables.Derivation()
        found.vtable = {("AlphaMovie", 0): 0x669458,
                        ("AlphaMovie", 0x444): 0x669450}
        self.assertEqual({}, found.collisions())


class TreeTests(unittest.TestCase):
    """The derivation over the real catalogue and the real image."""

    @classmethod
    def setUpClass(cls):
        if not vtables.DEFAULT_EXE.is_file():
            raise unittest.SkipTest("the pinned executable is absent")
        cls.found = vtables.derive()

    def test_no_two_classes_share_a_primary_vtable(self):
        self.assertEqual({}, self.found.collisions())

    def test_the_class_count_has_not_fallen(self):
        self.assertGreaterEqual(len(self.found.classes), vtables.CLASS_FLOOR)

    def test_every_primary_is_a_distinct_address(self):
        primary = self.found.primary()
        self.assertEqual(len(primary), len(set(primary.values())))

    def test_the_retired_ledger_is_not_reproduced(self):
        """0x006698C4 is the address `vtables.csv` gave to 15 classes. If the
        derivation ever hands it to more than one, the defect that made that
        file unusable has come back."""
        owners = [name for name, address in self.found.primary().items()
                  if address == 0x6698C4]
        self.assertLessEqual(len(owners), 1, owners)


if __name__ == "__main__":
    unittest.main()
