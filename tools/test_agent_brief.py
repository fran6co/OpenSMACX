#!/usr/bin/env python3
"""Unit tests for the per-function agent brief.

The brief exists to stop an agent spending its budget finding things: the
first coordinated run cost 79,111 tokens and 20 tool calls on a sixteen-byte
function, nearly all of it opening files. So what is tested is that the brief
is SELF-CONTAINED and that it says only what applies.
"""

import unittest

import agent_brief


class TargetedRulesTest(unittest.TestCase):
    """Only the rules that bear on the divergence actually seen. Everything an
    agent reads and does not use is budget it cannot spend on the answer."""

    def test_a_branch_diff_gets_the_polarity_rule_only(self):
        advice = agent_brief.targeted_rules("#2: original 'jl' vs rebuilt 'jge'")
        self.assertIn("BRANCH POLARITY", advice)
        self.assertNotIn("STRENGTH REDUCTION", advice)

    def test_an_unsigned_branch_gets_the_signedness_rule(self):
        advice = agent_brief.targeted_rules("#3: original 'jle' vs rebuilt 'jbe'")
        self.assertIn("SIGNEDNESS", advice)

    def test_a_shift_gets_strength_reduction(self):
        advice = agent_brief.targeted_rules("#1: original 'sar' vs rebuilt 'and'")
        self.assertIn("STRENGTH REDUCTION", advice)
        self.assertNotIn("SIGNEDNESS", advice)

    def test_register_allocation_is_flagged_as_the_hard_class(self):
        advice = agent_brief.targeted_rules("#0: original 'push' vs rebuilt 'mov'")
        self.assertIn("REGISTER ALLOCATION", advice)
        self.assertIn("refuted", advice)

    def test_an_unrecognised_divergence_says_so_rather_than_guessing(self):
        advice = agent_brief.targeted_rules("#4: original 'fldz' vs rebuilt 'wat'")
        self.assertIn("No fingerprint matched", advice)

    def test_the_measured_counter_example_is_carried(self):
        # The ternary rule did NOT hold on 0x005E3650 and flipping the `if`
        # did. An agent told only the general rule repeats that dead end.
        advice = agent_brief.targeted_rules("original 'jl' vs rebuilt 'jge'")
        self.assertIn("ternary did NOT help", advice)


class SelfContainmentTest(unittest.TestCase):
    def setUp(self):
        self.committed = agent_brief.verifier.committed_body
        self.functions = agent_brief.emit.load_functions
        self.row = agent_brief.ledger_row
        self.disasm = agent_brief.disassembly
        agent_brief.verifier.committed_body = lambda a: (
            "int f() { return 1; }", "src/x.cpp:10")
        agent_brief.emit.load_functions = lambda: {0x401000: {"name": "f"}}
        agent_brief.ledger_row = lambda a: {
            "size": "16", "tier": "MISMATCH",
            "note": "#2: original 'jl' vs rebuilt 'jge'"}
        agent_brief.disassembly = lambda a: "```asm\n0x00401000 ret\n```"

    def tearDown(self):
        agent_brief.verifier.committed_body = self.committed
        agent_brief.emit.load_functions = self.functions
        agent_brief.ledger_row = self.row
        agent_brief.disassembly = self.disasm

    def test_it_carries_everything_the_agent_needs(self):
        text = agent_brief.brief(0x401000)
        for needed in ("0x00401000",                 # which function
                       "MISMATCH",                    # what to beat
                       "0x00401000 ret",              # the disassembly
                       "int f() { return 1; }",       # the current body
                       "verify_recovered_function",   # how to score it
                       "--against-committed",         # and not regress
                       "__asm"):                      # what is forbidden
            self.assertIn(needed, text, needed)

    def test_it_forbids_touching_the_repository(self):
        text = agent_brief.brief(0x401000)
        self.assertIn("Write only to /tmp", text)

    def test_it_asks_for_readability_not_only_byte_exactness(self):
        self.assertIn("Readability counts", agent_brief.brief(0x401000))

    def test_it_stays_small(self):
        # The whole point. A brief that grows into a manual recreates the
        # problem it was written to solve.
        self.assertLess(len(agent_brief.brief(0x401000)), 6000)


if __name__ == "__main__":
    unittest.main()
