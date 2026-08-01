#!/usr/bin/env python3
"""The golden must detect change, and must not pretend to prove correctness.

These cover the comparison and the guards, not the derived facts themselves -
the facts are the golden's job, and asserting them here twice would just be the
same numbers written in two places.
"""
import unittest

import verify_recovery_pipeline as pipeline


class DifferenceTests(unittest.TestCase):
    def test_no_change_reports_nothing(self):
        facts = {"0x1": {"name": "f", "size": 10}}
        self.assertEqual([], pipeline.differences(facts, facts))

    def test_a_changed_fact_names_the_function_and_both_values(self):
        was = {"0x1": {"name": "?f@@YAXXZ", "span_bytes": 16430}}
        now = {"0x1": {"name": "?f@@YAXXZ", "span_bytes": 14328}}
        changes = pipeline.differences(was, now)
        self.assertEqual(1, len(changes))
        self.assertIn("?f@@YAXXZ", changes[0])
        self.assertIn("16430", changes[0])
        self.assertIn("14328", changes[0])

    def test_a_slate_entry_appearing_or_vanishing_is_reported(self):
        self.assertIn("new on the slate",
                      pipeline.differences({}, {"0x1": {"name": "f"}})[0])
        self.assertIn("no longer on the slate",
                      pipeline.differences({"0x1": {"name": "f"}}, {})[0])

    def test_a_fact_added_to_an_existing_entry_is_reported(self):
        # Otherwise a new derived fact could be introduced without review.
        changes = pipeline.differences({"0x1": {"name": "f"}},
                                       {"0x1": {"name": "f", "purge": [4]}})
        self.assertEqual(1, len(changes))
        self.assertIn("purge", changes[0])


class SlateTests(unittest.TestCase):
    def test_every_slate_entry_records_why_it_is_there(self):
        # The comment is the only record of which regression each defends; an
        # entry without one is a number nobody can justify keeping.
        for address, why in pipeline.SLATE:
            self.assertTrue(why.strip(), f"{address} has no rationale")
            self.assertGreater(len(why), 20, f"{address}: {why!r} says nothing")

    def test_the_slate_has_no_duplicates(self):
        addresses = [address for address, _ in pipeline.SLATE]
        self.assertEqual(len(addresses), len(set(addresses)))

    def test_purge_reports_unknown_rather_than_zero_when_there_is_no_ret(self):
        # A tail-jumping body has no `ret` of its own. Reporting that as 0 would
        # claim a cdecl arity the bytes never stated, which has produced a wrong
        # prototype in this repository before.
        class NoInstructions:
            def disasm(self, code, start):
                return iter(())
        self.assertIsNone(
            pipeline.purge_bytes(NoInstructions(), lambda a, b: b"\x90",
                                 [(0x1000, 0x1004)]))


if __name__ == "__main__":
    unittest.main()
