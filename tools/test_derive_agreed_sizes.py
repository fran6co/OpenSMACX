#!/usr/bin/env python3
"""Unit tests for the two-source size agreement.

The agreement is only worth anything because its control holds, so the tests
are about what may enter it and what the control does when it does not.
"""

import unittest

import derive_agreed_sizes as agreed


class AgreementTest(unittest.TestCase):
    def setUp(self):
        self._bounds, self._totals = agreed.access_bounds, agreed.idb_totals

    def tearDown(self):
        agreed.access_bounds, agreed.idb_totals = self._bounds, self._totals

    def fake(self, bounds, totals):
        agreed.access_bounds = lambda: bounds
        agreed.idb_totals = lambda: totals

    def test_exact_agreement_is_admitted(self):
        self.fake({"S": 0x40}, {"S": 0x40})
        self.assertEqual(agreed.agreed(), {"S": 0x40})

    def test_the_idb_total_is_aligned_up_first(self):
        """Its table ends at the last member, not at `sizeof`, and MSVC rounds
        a class up to its alignment - Buffer sums to 0x585 and really is
        0x588. Comparing the raw sum makes every 4-byte-aligned class look like
        a disagreement."""
        self.fake({"Buffer": 0x588}, {"Buffer": 0x585})
        self.assertEqual(agreed.agreed(), {"Buffer": 0x588})

    def test_a_real_disagreement_is_refused(self):
        # PullDown: the IDB records one member of 0xa14, the code reaches
        # 0xf40. Alignment cannot close 0x52C bytes.
        self.fake({"PullDown": 0xF40}, {"PullDown": 0xA14})
        self.assertEqual(agreed.agreed(), {})

    def test_a_class_only_one_source_knows_is_refused(self):
        self.fake({"S": 0x40}, {})
        self.assertEqual(agreed.agreed(), {})
        self.fake({}, {"S": 0x40})
        self.assertEqual(agreed.agreed(), {})

    def test_zero_is_not_an_agreement(self):
        self.fake({"S": 0}, {"S": 0})
        self.assertEqual(agreed.agreed(), {})


class ControlTest(unittest.TestCase):
    def setUp(self):
        self._pinned = agreed.derive_class_layout.load_pinned

    def tearDown(self):
        agreed.derive_class_layout.load_pinned = self._pinned

    def test_a_matching_pinned_size_counts_as_right(self):
        agreed.derive_class_layout.load_pinned = lambda: {"S": 0x40}
        right, wrong = agreed.control({"S": 0x40})
        self.assertEqual((right, wrong), (["S"], []))

    def test_a_mismatching_pinned_size_is_reported_wrong(self):
        agreed.derive_class_layout.load_pinned = lambda: {"S": 0x40}
        right, wrong = agreed.control({"S": 0x44})
        self.assertEqual(right, [])
        self.assertEqual(wrong, [("S", 0x44, 0x40)])

    def test_an_unpinned_class_is_neither(self):
        agreed.derive_class_layout.load_pinned = lambda: {}
        right, wrong = agreed.control({"S": 0x40})
        self.assertEqual((right, wrong), ([], []))


class RenderTest(unittest.TestCase):
    def test_sorted_with_a_header_and_hex_sizes(self):
        text = agreed.render({"Win": 0x444, "Buffer": 0x588})
        rows = text.splitlines()
        self.assertEqual(rows[0], "class,size,evidence")
        self.assertEqual([row.split(",")[0] for row in rows[1:]],
                         ["Buffer", "Win"])
        self.assertIn("0x588", rows[1])


if __name__ == "__main__":
    unittest.main()
