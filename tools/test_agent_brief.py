#!/usr/bin/env python3
"""Unit tests for the per-function agent brief.

The brief exists to stop an agent spending its budget finding things: the
first coordinated run cost 79,111 tokens and 20 tool calls on a sixteen-byte
function, nearly all of it opening files. So what is tested is that the brief
is SELF-CONTAINED and that it says only what applies.
"""

import io
import shutil
import sys
import tempfile
from contextlib import redirect_stderr, redirect_stdout
import unittest
from pathlib import Path

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
        agent_brief.reset_cache()   # the stub must not outlive itself
        agent_brief.ledger_row = lambda a, tier="", note="": {
            "size": "16", "tier": "MISMATCH",
            "note": "#2: original 'jl' vs rebuilt 'jge'"}
        agent_brief.disassembly = lambda a: "```asm\n0x00401000 ret\n```"
        self.landing = agent_brief.landing_path
        agent_brief.landing_path = lambda a, src=None: "src/unrecovered/x.cpp"

    def tearDown(self):
        agent_brief.verifier.committed_body = self.committed
        agent_brief.emit.load_functions = self.functions
        agent_brief.reset_cache()
        agent_brief.ledger_row = self.row
        agent_brief.disassembly = self.disasm
        agent_brief.landing_path = self.landing

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

    def test_it_names_the_one_file_the_body_has_to_land_in(self):
        # The rule used to be "Write only to /tmp", which contradicts the loop
        # it is part of: the coordinator banks with `--record-matches`, and
        # that measures the SOURCE TREE. Under the old wording a whole batch
        # could prove bodies in /tmp and bank zero, and it would have read as
        # agents failing rather than as the brief being wrong.
        text = agent_brief.brief(0x401000)
        self.assertIn("src/unrecovered/x.cpp", text)
        self.assertNotIn("Write only to /tmp", text)

    def test_it_says_why_landing_is_not_optional(self):
        self.assertIn("banks nothing", agent_brief.brief(0x401000))

    def test_the_scorer_is_spelled_with_a_python_that_can_run_it(self):
        # It was printed bare. `verify_recovered_function.py` is not
        # executable and its shebang is the system python3, so the one command
        # the brief exists to hand over exits 126 as printed and 1 under
        # python3 - and each agent spent turns finding the venv instead.
        text = agent_brief.brief(0x401000)
        self.assertNotIn("\n    tools/verify_recovered_function.py", text)
        self.assertIn(f"{agent_brief.interpreter()} "
                      f"tools/verify_recovered_function.py", text)

    def test_the_interpreter_is_the_one_running_this(self):
        # Derived, not written down: a spelled-out path is right on one
        # machine, which is how the write-scope rule came to carry somebody's
        # home directory.
        self.assertTrue(
            agent_brief.interpreter().endswith(Path(sys.executable).name))

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
        #
        # 6,000 held from the day this landed until 2026-08-13, when the
        # write-scope rule went from one line to three. That is not prose
        # creep: "Write only to /tmp" contradicted the loop the brief is part
        # of, and the replacement has to name the file and say why landing is
        # not optional. Measured 6,038 after; 6,100 leaves the same ~1% the
        # original bound did, which is a reword and not a section.
        self.assertLess(len(agent_brief.brief(0x401000)), 6100)


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
        agent_brief.reset_cache()   # the stub must not outlive itself
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
        agent_brief.reset_cache()
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


class BatchModeTest(unittest.TestCase):
    """`--addresses` must be the same projection as the single-address path.

    It exists only to pay the catalogue, annotation-index and PE loads ONCE:
    a single brief costs ~24 s and almost all of it is those three loads, so
    72 briefs one process at a time was half an hour of re-reading the same
    data. The marginal cost through batch mode is about a second.

    A faster second spelling of the same output is exactly how
    `_catalog_facts` drifted from `project_catalogue.facts` and silently
    dropped every call edge, so this asserts BYTE equality, not similarity.
    """

    def setUp(self):
        self.work = Path(tempfile.mkdtemp())
        self.original = dict(agent_brief._CACHE)
        agent_brief._CACHE.clear()
        self.stubs = {
            "committed": agent_brief.verifier.committed_body,
            "row": agent_brief.ledger_row,
            "disasm": agent_brief.disassembly,
            "landing": agent_brief.landing_path,
        }
        agent_brief.verifier.committed_body = lambda a: (None, "nope")
        agent_brief.ledger_row = lambda a, tier="", note="": {"tier": tier}
        agent_brief.disassembly = lambda a: f"```asm\n0x{a:08X} ret\n```"
        agent_brief.landing_path = lambda a, src=None: "src/unrecovered/x.cpp"

    def tearDown(self):
        agent_brief.verifier.committed_body = self.stubs["committed"]
        agent_brief.ledger_row = self.stubs["row"]
        agent_brief.disassembly = self.stubs["disasm"]
        agent_brief.landing_path = self.stubs["landing"]
        agent_brief._CACHE.clear()
        agent_brief._CACHE.update(self.original)
        shutil.rmtree(self.work, ignore_errors=True)

    def run_batch(self, addresses):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            status = agent_brief.main(
                ["--addresses", addresses, "--out-dir", str(self.work),
                 "--tier", "PLACEHOLDER"])
        return status, out.getvalue() + err.getvalue()

    def test_it_writes_one_brief_per_address(self):
        status, output = self.run_batch("0x401000,0x402000,0x403000")
        self.assertEqual(status, 0)
        self.assertEqual(len(list(self.work.glob("*.md"))), 3)
        self.assertIn("wrote 3 brief(s)", output)

    def test_each_brief_is_byte_identical_to_the_single_address_path(self):
        self.run_batch("0x401000")
        out = io.StringIO()
        with redirect_stdout(out):
            agent_brief.main(["0x401000", "--tier", "PLACEHOLDER"])
        self.assertEqual((self.work / "0x00401000.md").read_text(),
                         out.getvalue())

    def test_one_bad_address_does_not_take_the_batch_down(self):
        # A batch is 70+ addresses assembled by a coordinator. Losing all of
        # them to one unscaffoldable function would be the expensive failure.
        original = agent_brief.brief
        def flaky(address, tier="", note=""):
            if address == 0x402000:
                raise RuntimeError("no scaffolding")
            return original(address, tier, note)
        agent_brief.brief = flaky
        try:
            status, output = self.run_batch("0x401000,0x402000,0x403000")
        finally:
            agent_brief.brief = original
        self.assertEqual(status, 1)                    # visibly partial
        self.assertEqual(len(list(self.work.glob("*.md"))), 2)
        self.assertIn("0x00402000", output)

    def test_the_cache_is_not_shared_across_a_changed_tree(self):
        # `_cached` is process-local by design: the coordinator edits src/
        # between runs, and a persisted cache would hand an agent work that is
        # already done.
        self.assertFalse(hasattr(agent_brief, "CACHE_FILE"))


class ArityHypothesisTest(unittest.TestCase):
    """IDA's argument count, for the functions no mangled name describes.

    274 placeholders are `sub_*` with no prototype, 217 of them already have an
    evidenced `argument_count` in prototype-hypotheses.csv, and it reached
    nothing - so the scaffold handed out a nullary contract and the agent
    rebuilt the arity from `[esp+N]` reads. Batch 4 turned ~24% of its
    addresses into signature proposals, which is that gap priced.
    """

    def setUp(self):
        self.work = Path(tempfile.mkdtemp())
        self.original = agent_brief.HYPOTHESES_CSV
        agent_brief.reset_cache()

    def tearDown(self):
        agent_brief.HYPOTHESES_CSV = self.original
        agent_brief.reset_cache()
        shutil.rmtree(self.work, ignore_errors=True)

    def csv_with(self, rows):
        path = self.work / "hyp.csv"
        path.write_text("address,name,argument_count,evidence\n" + rows)
        agent_brief.HYPOTHESES_CSV = path
        agent_brief.reset_cache()
        return path

    def test_a_hypothesis_is_shown_with_its_evidence(self):
        self.csv_with('0x00401D80,sub_401d80,1,"IDA 9.4 guess_type int __stdcall(void *Block)"\n')
        text = agent_brief.arity_hypothesis(0x00401D80)
        self.assertIn("IDA guessed 1 argument", text)
        self.assertIn("guess_type int __stdcall(void *Block)", text)

    def test_it_is_labelled_a_hypothesis_not_a_contract(self):
        # One source. I checked whether Ghidra could corroborate and it cannot:
        # `undefined FUN_xxxxxxxx()` with convention unknown. Presenting a lone
        # guess as a second record is how a plausible wrong body gets written.
        self.csv_with('0x00401D80,sub_401d80,3,"IDA 9.4 guess_type int(int,int,int)"\n')
        text = agent_brief.arity_hypothesis(0x00401D80)
        self.assertIn("hypothesis from one tool", text)
        self.assertIn("no cross-check", text)

    def test_a_zero_argument_hypothesis_does_not_fire(self):
        # It agrees with the fallback the scaffold already emitted, so it adds
        # nothing and would only dilute the brief.
        self.csv_with('0x00402DD0,sub_402dd0,0,"IDA 9.4 guess_type int()"\n')
        self.assertEqual(agent_brief.arity_hypothesis(0x00402DD0), "")

    def test_an_address_with_no_hypothesis_gets_nothing(self):
        self.csv_with('0x00401D80,sub_401d80,1,"x"\n')
        self.assertEqual(agent_brief.arity_hypothesis(0x00999999), "")

    def test_a_missing_csv_does_not_take_the_brief_down(self):
        agent_brief.HYPOTHESES_CSV = self.work / "absent.csv"
        agent_brief.reset_cache()
        self.assertEqual(agent_brief.arity_hypothesis(0x00401D80), "")

    def test_a_malformed_count_is_skipped_rather_than_raising(self):
        self.csv_with('0x00401D80,sub_401d80,,"no count"\n')
        self.assertEqual(agent_brief.arity_hypothesis(0x00401D80), "")

    def test_it_warns_that_a_thiscall_receiver_is_not_an_argument(self):
        # The specific way this hypothesis misleads: IDA counts `this` for a
        # thiscall it guessed as stdcall, and an agent adding a stack param for
        # it writes a body with one argument too many.
        self.csv_with('0x00401D80,sub_401d80,2,"IDA 9.4 guess_type int __thiscall(void *Block, char)"\n')
        self.assertIn("is NOT one of", agent_brief.arity_hypothesis(0x00401D80))


class ThunkSectionTest(unittest.TestCase):
    """A one-jump body is a thunk, and the operand says which kind.

    Batch 2 measured thunks at 12 of 38 matches, with four agents each
    rediscovering the shim-class lever unaided, and three agents separately
    proving that the import-stub shape cannot be expressed at all. Both halves
    are worth carrying: one turns a search into a single candidate, the other
    stops a search that cannot succeed.
    """

    def tail(self):
        return "```asm\n0x00404B00  jmp 0x00404b40\n```"

    def stub(self):
        return "```asm\n0x00644DC0  jmp dword ptr [0x00669258]\n```"

    def test_a_tail_jump_gets_the_shim_lever(self):
        text = agent_brief.thunk_section(self.tail())
        self.assertIn("shim class", text)
        self.assertIn("reinterpret_cast", text)

    def test_an_import_stub_is_named_as_a_wall(self):
        text = agent_brief.thunk_section(self.stub())
        self.assertIn("known wall", text)
        self.assertIn("will not tail-call", text)

    def test_the_two_shapes_do_not_get_each_other_s_advice(self):
        # The expensive confusion: telling an agent to keep searching on the
        # shape three agents already proved inexpressible.
        self.assertNotIn("known wall", agent_brief.thunk_section(self.tail()))
        self.assertNotIn("shim class", agent_brief.thunk_section(self.stub()))

    def test_the_stub_section_still_says_what_DOES_work(self):
        # An import reached by `call` is expressible through an IAT cast. A
        # wall notice that suppressed that too would cost the neighbouring win.
        self.assertIn("IAT slot", agent_brief.thunk_section(self.stub()))

    def test_an_ordinary_body_gets_neither(self):
        self.assertEqual(agent_brief.thunk_section(
            "```asm\n0x00401000  mov eax, ecx\n0x00401002  ret\n```"), "")

    def test_a_multi_instruction_body_ending_in_jmp_is_not_a_thunk(self):
        # A tail call at the END of real work is not a thunk, and the shim
        # advice would be wrong about the whole body.
        self.assertEqual(agent_brief.thunk_section(
            "```asm\n0x00401000  mov eax, ecx\n0x00401002  jmp 0x00402000\n```"),
            "")

    def test_it_survives_a_body_with_no_instructions(self):
        self.assertEqual(agent_brief.thunk_section("```asm\n\n```"), "")


class LifecycleSectionTest(unittest.TestCase):
    """The store-order rule fires on constructors and destructors only.

    It is keyed on the mangled name because the divergence it produces carries
    nothing to key on: declaration order scores MNEMONIC_ONLY, where every
    mnemonic matches and only operands differ, so there is no note and
    `targeted_rules` reports "no fingerprint matched this divergence".
    """

    def test_a_constructor_gets_the_rule(self):
        self.assertIn("STORE ORDER",
                      agent_brief.lifecycle_section("??0Font@@QAE@XZ"))

    def test_a_destructor_gets_it_too(self):
        self.assertIn("STORE ORDER",
                      agent_brief.lifecycle_section("??1Filemap@@QAE@XZ"))

    def test_an_ordinary_method_does_not(self):
        self.assertEqual(
            agent_brief.lifecycle_section("?get_state_id@CheckBox@@QAEHH@Z"), "")

    def test_a_decompiler_stub_name_does_not(self):
        self.assertEqual(agent_brief.lifecycle_section("sub_63d450"), "")

    def test_it_says_why_an_init_list_cannot_express_it(self):
        # The mechanism is the load-bearing half. Without it an agent tries a
        # reordered member-init list, which C++ silently runs in declaration
        # order anyway, and concludes the lever does not work.
        text = agent_brief.lifecycle_section("??0Time@@QAE@XZ")
        self.assertIn("declaration order", text)
        self.assertIn("MNEMONIC_ONLY", text)


class LandingPathTest(unittest.TestCase):
    """Where a body has to land is LOOKED UP, never composed.

    `src/unrecovered/<hex>.cpp` is the usual home and is not the rule. Of the
    48 addresses prepared for the 2026-08-13 batch, one already lived under
    `src/recovered/`, and a composed path would have sent that agent to a file
    the coordinator does not measure - which banks zero while every command in
    the agent's transcript exits 0.
    """

    def setUp(self):
        self.work = Path(tempfile.mkdtemp())

    def tearDown(self):
        shutil.rmtree(self.work, ignore_errors=True)

    def write(self, relative, address):
        path = self.work / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(f"// ORIGINAL: 0x{address:08X} FILE\n"
                        f"void f() {{ /* BODY GOES HERE. */ }}\n")
        return path

    def test_it_finds_the_file_that_claims_the_address(self):
        self.write("unrecovered/00401000.cpp", 0x401000)
        found = agent_brief.landing_path(0x401000, src=self.work)
        self.assertTrue(found.endswith("unrecovered/00401000.cpp"), found)

    def test_a_body_outside_the_usual_directory_is_still_found(self):
        # The case that breaks composition: same address, different home.
        self.write("recovered/00401000.cpp", 0x401000)
        found = agent_brief.landing_path(0x401000, src=self.work)
        self.assertTrue(found.endswith("recovered/00401000.cpp"), found)

    def test_an_unclaimed_address_returns_empty_rather_than_a_guess(self):
        self.write("unrecovered/00401000.cpp", 0x401000)
        self.assertEqual(agent_brief.landing_path(0x402000, src=self.work), "")

    def test_the_brief_says_so_rather_than_naming_a_file_that_does_not_exist(self):
        # An invented path is worse than none: the agent creates it, every
        # command it runs succeeds, and the coordinator measures a placeholder.
        original = agent_brief.landing_path
        agent_brief.landing_path = lambda a, src=None: ""
        try:
            text = agent_brief.brief(0x401000, tier="MISMATCH", note="probe")
        finally:
            agent_brief.landing_path = original
        self.assertIn("NONE", text)
        self.assertIn("nowhere to land it", text)


if __name__ == "__main__":
    unittest.main()
