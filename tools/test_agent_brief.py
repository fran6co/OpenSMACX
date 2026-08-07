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


class PlaceholderTest(unittest.TestCase):
    """Names the compiler cannot object to, so the brief has to."""

    def test_it_finds_the_address_derived_placeholders(self):
        found = agent_brief.placeholders(
            "sub_5e3630", "uint32_t leaf_005e3630_redirect(void *self) {}")
        self.assertEqual(found, ["sub_5e3630", "leaf_005e3630_redirect"])

    def test_it_finds_decompiler_argument_names(self):
        found = agent_brief.placeholders("f", "int f(int a1, int a2) { int v3; }")
        self.assertEqual(found, ["a1", "a2", "v3"])

    def test_it_does_not_report_the_same_name_twice(self):
        self.assertEqual(agent_brief.placeholders("sub_401000", "sub_401000();"),
                         ["sub_401000"])

    def test_a_real_name_is_not_a_placeholder(self):
        self.assertEqual(
            agent_brief.placeholders(
                "bit_count_signed", "int bit_count_signed(int input) {}"),
            [])

    def test_the_naming_section_is_omitted_when_nothing_needs_renaming(self):
        # A target whose names are already good must not be asked to invent
        # better ones - that is how a good name gets replaced by a fluent one.
        self.assertEqual(
            agent_brief.naming_section("garrison_check", "int garrison_check()"),
            "")

    def test_the_naming_section_appears_when_something_does(self):
        text = agent_brief.naming_section("sub_401000", "void sub_401000() {}")
        self.assertIn("# Naming", text)
        self.assertIn("sub_401000", text)

    def test_it_asks_for_evidence_rather_than_a_plausible_name(self):
        text = agent_brief.naming_section("sub_401000", "void sub_401000() {}")
        self.assertIn("unclear", text)


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

    def test_the_signature_is_declared_changeable(self):
        # It always was - `byte_match.object_code` compares the object's code
        # and never looks a symbol up - but the brief used to forbid it, so
        # every convention and return-type error was out of reach by wording.
        text = agent_brief.brief(0x401000)
        self.assertIn("signature is yours to change", text)
        self.assertNotIn("same signature as the body above", text)

    def test_a_signature_change_is_framed_as_a_catalogue_proposal(self):
        # Scoring it is free; LANDING it is not. A body whose signature
        # disagrees with the catalogue fails at integration on C2371/C2556.
        text = agent_brief.brief(0x401000)
        self.assertIn("C2371", text)
        self.assertIn("PROPOSALS", text)

    def test_the_refuted_signature_lever_is_carried(self):
        # Freeing the signature invites rewriting every fake-thiscall as a
        # real one. Measured on 0x005E3630: it changes nothing.
        self.assertIn("NOT ONE\nBYTE", agent_brief.brief(0x401000))

    def test_it_stays_small(self):
        # The whole point. A brief that grows into a manual recreates the
        # problem it was written to solve.
        self.assertLess(len(agent_brief.brief(0x401000)), 6000)


class RealBriefSizeTest(unittest.TestCase):
    """The size guard above measures a one-line mock body, so it would not
    notice the prose growing until a real brief was twice its budget. This
    measures a real one - the thing that is actually sent."""

    def test_a_real_brief_stays_small(self):
        if not agent_brief.LEDGER.is_file():
            self.skipTest("docs/recovery/byte-match.csv is absent")
        row = agent_brief.ledger_row(0x005E3630)
        if not row:
            self.skipTest("0x005E3630 is not in the ledger")
        self.assertLess(len(agent_brief.brief(0x005E3630)), 8000)


if __name__ == "__main__":
    unittest.main()
