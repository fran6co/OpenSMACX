#!/usr/bin/env python3
"""Unit tests for the frontier, the waves and the leaf queue.

The property worth pinning is the one that was WRONG for as long as this tool
existed: `functions.csv` does not know about `src/recovered/`, so 252 of the 563
leaves it printed were already proved and `0x00401000` sat fifth in the queue
with a BYTE_EXACT claim on it. A queue that hands an agent finished work is
worse than a short queue, because the agent cannot tell.

The join matters in BOTH directions and both are tested: a proved address must
leave the queue, and it must also count as RESOLVED for its callers, which is
what lets the frontier grow rather than merely shrink.
"""

import io
import unittest
from contextlib import redirect_stdout
from pathlib import Path

import recovery_frontier as frontier


def inventory(*rows):
    """rows of (address, size, state, callers) -> the shape `load` returns."""
    return {address: {"size": size, "name": f"fn_{address:x}", "state": state,
                      "kind": "game", "callers": callers, "targets": 0}
            for address, size, state, callers in rows}


class LeafQueueTest(unittest.TestCase):
    def test_a_leaf_is_unrecovered_with_no_callees(self):
        inv = inventory((0x1000, 10, "unrecovered", 3),
                        (0x2000, 20, "source_complete", 1))
        queue = frontier.leaf_queue(inv, {})
        self.assertEqual([a for a, _ in queue], [0x1000])

    def test_ranked_by_callers_then_size(self):
        inv = inventory((0x1000, 90, "unrecovered", 1),
                        (0x2000, 10, "unrecovered", 5),
                        (0x3000, 50, "unrecovered", 5))
        queue = frontier.leaf_queue(inv, {})
        self.assertEqual([a for a, _ in queue], [0x2000, 0x3000, 0x1000])

    def test_a_function_with_a_callee_is_not_a_leaf(self):
        inv = inventory((0x1000, 10, "unrecovered", 1))
        self.assertEqual(frontier.leaf_queue(inv, {0x1000: {0x9000}}), [])


class WaveTest(unittest.TestCase):
    def test_a_resolved_callee_puts_its_caller_on_the_frontier(self):
        inv = inventory((0x1000, 10, "unrecovered", 0),
                        (0x2000, 10, "source_complete", 1))
        waves, pending = frontier.waves(inv, {0x1000: {0x2000}})
        self.assertIn(0x1000, waves[0])
        self.assertEqual(pending, set())

    def test_an_unrecovered_callee_blocks_its_caller(self):
        inv = inventory((0x1000, 10, "unrecovered", 0),
                        (0x2000, 10, "unrecovered", 1))
        waves, _ = frontier.waves(inv, {0x1000: {0x2000}})
        self.assertNotIn(0x1000, waves[0])
        self.assertIn(0x2000, waves[0])


class ClaimedFilterTest(unittest.TestCase):
    """The defect this tool shipped with, in both of its directions."""

    def setUp(self):
        self.load = frontier.load
        self.claimed = frontier.claimed_addresses

    def tearDown(self):
        frontier.load = self.load
        frontier.claimed_addresses = self.claimed

    def drive(self, inv, callees, claimed, extra=()):
        frontier.load = lambda f, c: (inv, callees)
        frontier.claimed_addresses = lambda src=None: claimed
        out = io.StringIO()
        with redirect_stdout(out):
            frontier.main(["--queue", "50", *extra])
        return out.getvalue()

    def test_a_proved_address_leaves_the_queue(self):
        inv = inventory((0x401000, 88, "unrecovered", 12),
                        (0x402000, 40, "unrecovered", 2))
        text = self.drive(inv, {}, {0x401000})
        self.assertNotIn("0x00401000", text)
        self.assertIn("0x00402000", text)

    def test_a_proved_address_also_resolves_its_callers(self):
        # The half a subtraction would miss: promoting a proved callee UNBLOCKS
        # the functions that call it, so the frontier grows as well as shrinks.
        inv = inventory((0x401000, 88, "unrecovered", 0),
                        (0x402000, 40, "unrecovered", 1))
        text = self.drive(inv, {0x401000: {0x402000}}, {0x402000})
        self.assertIn("RECOVERABLE FRONTIER: 1 functions", text)

    def test_include_claimed_restores_the_old_behaviour(self):
        inv = inventory((0x401000, 88, "unrecovered", 12))
        text = self.drive(inv, {}, {0x401000}, extra=["--include-claimed"])
        self.assertIn("0x00401000", text)

    def test_the_filter_is_reported_not_silent(self):
        # Silently shrinking a published queue would read as progress.
        inv = inventory((0x401000, 88, "unrecovered", 12))
        self.assertIn("already proves", self.drive(inv, {}, {0x401000}))

    def test_max_size_bounds_the_queue(self):
        inv = inventory((0x401000, 500, "unrecovered", 9),
                        (0x402000, 40, "unrecovered", 9))
        text = self.drive(inv, {}, set(), extra=["--max-size", "128"])
        self.assertNotIn("0x00401000", text)
        self.assertIn("0x00402000", text)


class ClaimedAddressesTest(unittest.TestCase):
    def test_it_reads_the_map_not_source_locations(self):
        """Stored bodies deliberately carry no `source_locations`, so a reader
        keyed on the catalogue cannot see them. This must read the annotation."""
        found = frontier.claimed_addresses()
        if not found:
            self.skipTest("no src/ tree to scan")
        self.assertIn(0x00401000, found)

    def test_a_missing_tree_is_empty_not_an_error(self):
        self.assertEqual(frontier.claimed_addresses(Path("/nonexistent")), set())


if __name__ == "__main__":
    unittest.main()
