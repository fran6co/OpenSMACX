#!/usr/bin/env python3

from pathlib import Path
import tempfile
import unittest

import verify_documented_counts as counts


FUNCTIONS = ("address,recovery_state\n"
             "0x1,source_complete\n0x2,source_complete\n"
             "0x3,unrecovered\n0x4,thunk\n0x5,external_library\n"
             "0x6,original_dependency\n0x7,source_in_progress\n")

LINE = ("- Current recovery state: {sc} `source_complete`, 1 "
        "`original_dependency`, 1 `source_in_progress`, 1 `external_library`, "
        "1 `thunk`, {un} `unrecovered`; more prose after.\n")


class DocumentedCountsTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.functions = self.root / "functions.csv"
        self.functions.write_text(FUNCTIONS, encoding="utf-8")

    def doc(self, text):
        path = self.root / "AGENTS.md"
        path.write_text(text, encoding="utf-8")
        return path

    def run_check(self, text):
        import sys
        from unittest import mock
        argv = ["verify_documented_counts.py", "--doc", str(self.doc(text)),
                "--functions", str(self.functions)]
        with mock.patch.object(sys, "argv", argv):
            return counts.main()

    def test_agreeing_counts_pass(self):
        self.assertEqual(self.run_check(LINE.format(sc=2, un=1)), 0)

    def test_a_single_wrong_figure_fails(self):
        self.assertEqual(self.run_check(LINE.format(sc=3, un=1)), 1)

    def test_thousands_separators_are_understood(self):
        # The real line writes 2,573 - a parser that choked would compare
        # nothing and pass.
        big = self.root / "big.csv"
        big.write_text("address,recovery_state\n"
                       + "".join(f"0x{i},source_complete\n" for i in range(1100))
                       + "0x9,unrecovered\n0xa,thunk\n0xb,external_library\n"
                         "0xc,original_dependency\n0xd,source_in_progress\n",
                       encoding="utf-8")
        self.functions = big
        self.assertEqual(self.run_check(LINE.format(sc="1,100", un=1)), 0)
        self.assertEqual(self.run_check(LINE.format(sc="1,101", un=1)), 1)

    def test_a_missing_line_fails_rather_than_passing_vacuously(self):
        self.assertEqual(self.run_check("nothing to see here\n"), 1)

    def test_a_partial_line_fails_rather_than_comparing_a_subset(self):
        # Matching two of six states and calling it clean is how a check
        # survives a rename while verifying nothing.
        self.assertEqual(
            self.run_check("- Current recovery state: 2 `source_complete`, "
                           "1 `unrecovered` and nothing else.\n"), 1)

    def test_the_committed_tree_agrees(self):
        if not counts.DEFAULT_FUNCTIONS.is_file():
            self.skipTest("docs/recovery/functions.csv is absent")
        documented = counts.documented_counts(counts.DEFAULT_DOC)
        self.assertIsNotNone(documented)
        actual = counts.actual_counts(counts.DEFAULT_FUNCTIONS)
        for state, said in documented.items():
            self.assertEqual(said, actual[state], f"AGENTS.md {state}")


if __name__ == "__main__":
    unittest.main()
