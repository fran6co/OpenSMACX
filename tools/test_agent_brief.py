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
        agent_brief.ledger_row = lambda a, tier="", note="": {
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
        #
        # Matched with whitespace collapsed. Pinning "NOT ONE\nBYTE" with the
        # newline inside it pinned where the paragraph HAPPENED to wrap, which
        # is not the claim being guarded: rewording earlier in the same
        # sentence failed this test while the fact was still stated, and the
        # only way to pass was to break the line mid-phrase.
        text = " ".join(agent_brief.brief(0x401000).split())
        self.assertIn("NOT ONE BYTE", text)

    def test_it_stays_small(self):
        # The whole point. A brief that grows into a manual recreates the
        # problem it was written to solve.
        self.assertLess(len(agent_brief.brief(0x401000)), 6000)


class FreshRecoveryTest(unittest.TestCase):
    """278 of the remaining leaves under 64 bytes have NO committed body.

    The census still gives those rows a tier, because it scores every
    catalogued function and an unrecovered one compiles to an empty scaffold.
    Left alone the brief handed an agent a blank body, the `committed_body`
    error string as a source location, and a paragraph of register-allocation
    advice about a stub that does not exist.
    """

    def setUp(self):
        self.committed = agent_brief.verifier.committed_body
        self.functions = agent_brief.emit.load_functions
        self.row = agent_brief.ledger_row
        self.disasm = agent_brief.disassembly
        self.section = agent_brief.prompt_section
        agent_brief.verifier.committed_body = lambda a: (
            None, "0x00401000 has no source_locations")
        agent_brief.emit.load_functions = lambda: {0x401000: {"name": "f"}}
        agent_brief.ledger_row = lambda a, tier="", note="": {
            "size": "26", "tier": "MISMATCH",
            "note": "#0: original 'push' vs rebuilt 'xor'"}
        agent_brief.disassembly = lambda a: "```asm\n0x00401000 ret\n```"
        agent_brief.prompt_section = lambda a, h: (
            "## Contract\n\nint C::m(int a) {" if h == "Contract"
            else "## Ghidra\n\nvoid FUN_401000(void) {}")

    def tearDown(self):
        agent_brief.verifier.committed_body = self.committed
        agent_brief.emit.load_functions = self.functions
        agent_brief.ledger_row = self.row
        agent_brief.disassembly = self.disasm
        agent_brief.prompt_section = self.section

    def test_it_says_nothing_is_recovered_rather_than_showing_a_blank_body(self):
        text = agent_brief.brief(0x401000)
        self.assertIn("nothing recovered yet", text)
        self.assertNotIn("The body as it stands", text)

    def test_it_does_not_leak_the_lookup_error_as_a_source_location(self):
        self.assertNotIn("has no source_locations",
                         agent_brief.brief(0x401000))

    def test_the_empty_scaffold_divergence_is_disowned_not_explained(self):
        # `#0 push vs xor` against an unrecovered row is the shape of a
        # function that does not exist. Explaining it sends the agent after
        # a register-allocation problem it does not have.
        text = agent_brief.brief(0x401000)
        self.assertIn("IGNORE IT", text)
        self.assertNotIn("REGISTER ALLOCATION", text)

    def test_it_carries_the_definition_head_and_the_decompiler_guess(self):
        text = agent_brief.brief(0x401000)
        self.assertIn("int C::m(int a) {", text)
        self.assertIn("FUN_401000", text)

    def test_a_missing_contract_does_not_take_the_brief_down(self):
        # The Contract used to be scraped out of a gitignored prompt folder, so
        # its absence was normal and the brief printed "run
        # disassembly.py". It is derived from the image now, so an
        # empty section means the scaffolding refused - the brief must still
        # carry the disassembly and the loop rather than failing.
        agent_brief.prompt_section = lambda a, h: ""
        text = agent_brief.brief(0x401000)
        self.assertIn("# Your loop", text)
        self.assertIn("verify_recovered_function.py", text)


class RealBriefSizeTest(unittest.TestCase):
    """The size guard above measures a one-line mock body, no real
    disassembly and a name with no placeholders in it, so it never sees the
    three sections that only exist on real data. This measures a real one -
    the thing that is actually sent."""

    def test_a_real_brief_stays_small(self):
        # The verdict is PASSED IN rather than read from a gitignored ledger,
        # which is the whole point of the change this test now guards: the
        # brief has to work in a worktree that has never generated one.
        if not agent_brief.verifier.byte_match.DEFAULT_EXE.is_file():
            self.skipTest("the pinned executable is absent")
        text = agent_brief.brief(0x005E3630, tier="MISMATCH",
                                 note="#3: original 'jl' vs rebuilt 'jge'")
        body, _ = agent_brief.verifier.committed_body(0x005E3630)

        # NON-VACUITY FIRST, because the absent version of this brief is the
        # small one. The bound here was calibrated at 6,035 bytes on
        # 2026-08-12 while `committed_body` was returning None for this
        # address - functions.csv gave the row two source_locations joined by
        # " | " and the reader opened the pair as a single path - so the "real
        # brief" being measured was the fresh-recovery one, with no body, no
        # divergence rule and no naming section. src/ became the catalogue
        # (185dd977), the lookup works, and the same call is now 8,036 bytes
        # with NOT ONE byte added to the prose: measured at eedcb9de and at
        # HEAD through the mock above, both 5,863. A size guard a body-less
        # brief can satisfy is measuring the brief that is not sent.
        self.assertIsNotNone(body, "the fixture address is not recovered any "
                                   "more, so this measures the wrong brief")
        self.assertIn("# The body as it stands", text)
        self.assertIn(body.strip(), text)

        # Then bound what the TOOL owns. The other two inputs are as large as
        # the function is, and a total is mostly them: over twenty sampled
        # recovered functions the brief ran 6,230 to 22,267 bytes, the top one
        # being this same prose around a 12,453-byte disassembly and a
        # 3,946-byte body (0x004346A0). So an absolute total never bounded the
        # prose - it bounded this one leaf staying short, and it moved when
        # the leaf did. The prose itself barely moves: 5,840-5,880 bytes on
        # the sixteen whose names are already good, 6,547-6,584 on the five
        # that also earn the naming section. 7,000 leaves 6%, which is a
        # paragraph - enough that rewording passes and adding a section does
        # not.
        asm = text.split("```asm\n", 1)[1].split("```", 1)[0]
        prose = len(text) - len(body.strip()) - len(asm)
        self.assertLess(prose, 7000)


if __name__ == "__main__":
    unittest.main()
