#!/usr/bin/env python3

from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

import verify_documented_counts as counts

PADDING = "filler paragraph.\n" * 100          # keeps the file over the floor


class RestatementTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def check(self, body):
        doc = self.root / "AGENTS.md"
        doc.write_text(body + PADDING, encoding="utf-8")
        argv = ["verify_documented_counts.py", "--doc", str(doc)]
        with mock.patch.object(sys, "argv", argv):
            return counts.main()

    def test_a_document_with_no_counts_passes(self):
        self.assertEqual(self.check("Run tools/recovery_metrics.py.\n"), 0)

    def test_a_restated_count_fails(self):
        self.assertEqual(
            self.check("- state: 2,573 `source_complete`, 2,787 `unrecovered`.\n"),
            1)

    def test_a_single_restated_count_fails(self):
        self.assertEqual(self.check("there are 28 `thunk` functions.\n"), 1)

    def test_naming_a_state_without_a_number_is_allowed(self):
        # Prose has to be able to talk about the states.
        self.assertEqual(
            self.check("A callee is resolved if it is `source_complete`, "
                       "`external_library` or `thunk`.\n"), 0)

    def test_a_number_near_an_unrelated_backtick_is_allowed(self):
        self.assertEqual(
            self.check("The 2,573 functions live in `docs/recovery/`.\n"), 0)

    def test_a_truncated_document_fails_rather_than_passing(self):
        doc = self.root / "AGENTS.md"
        doc.write_text("tiny\n", encoding="utf-8")
        argv = ["verify_documented_counts.py", "--doc", str(doc)]
        with mock.patch.object(sys, "argv", argv):
            self.assertEqual(counts.main(), 1)

    def test_a_missing_document_fails(self):
        argv = ["verify_documented_counts.py", "--doc",
                str(self.root / "absent.md")]
        with mock.patch.object(sys, "argv", argv):
            self.assertEqual(counts.main(), 1)

    def test_the_committed_agents_md_restates_nothing(self):
        if not counts.DEFAULT_DOC.is_file():
            self.skipTest("AGENTS.md is absent")
        self.assertEqual(counts.restated_counts(counts.DEFAULT_DOC), [])


if __name__ == "__main__":
    unittest.main()
