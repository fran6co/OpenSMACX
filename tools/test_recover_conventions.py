#!/usr/bin/env python3
"""Unit tests for calling-convention recovery.

Everything here guards a place where a WRONG answer still looks like a right
one, because that is the only kind of bug this module can have:

* The infix. `QAE` is public/instance, cv-none, __thiscall. Reading the slot
  after the kind char as the convention gives `A` = __cdecl, which is a legal
  convention, a legal answer, and wrong for 2,695 functions - it reports the
  entire image as __cdecl and nothing crashes. So the tests pin `QAE` against
  `QAA` AND pin a free `Y` and a static `S` in the same shape, because those
  two genuinely have no cv slot and a parser that "fixes" the member case by
  always skipping one char breaks them instead.

* Argument widths. A `double` is two stack dwords. Counting it as one shifts
  every later slot's pointer-ness by four bytes, which is exactly the kind of
  error that produces a plausible mask and a useless seed.

* Refusal. Varargs and by-value classes must return None rather than a guess.
  A guessed width is silently wrong for every slot after it.

* `this` on the stack. A `__cdecl` member's `this` occupies stack slot 0, so
  its pointer mask must have bit 0 set and every declared argument must move up
  one slot. A `__thiscall`'s `this` must NOT appear in the mask at all. These
  two are one bit apart and both produce a well-formed mask.

* Callee-pop arithmetic. `__thiscall` pops arguments but never `this`;
  `__stdcall` on a member pops both. Getting that backwards makes the
  body-versus-name check accuse correct rows and excuse wrong ones.

* The bare `ret` ambiguity. `__thiscall` with no arguments and `__cdecl` with
  no arguments BOTH emit `ret 0`. A checker that resolves that from the pop
  byte alone reports a disagreement on thousands of correct functions, so the
  test asserts that a bare `ret` with ECX read first is accepted as thiscall
  and that the same body without the ECX read is flagged.
"""
import csv
import tempfile
import unittest
from pathlib import Path

import recover_conventions as rc


class InfixTest(unittest.TestCase):
    def test_public_instance_thiscall(self):
        self.assertEqual(("Q", "E"), rc.split_infix("?close@S@@QAEXXZ"))

    def test_public_instance_cdecl(self):
        self.assertEqual(("Q", "A"), rc.split_infix("?close@S@@QAAXXZ"))

    def test_public_instance_stdcall(self):
        self.assertEqual(("Q", "G"), rc.split_infix("?get@Win@@QAGHXZ"))

    def test_free_function_has_no_cv_slot(self):
        # `Y` then the convention directly. A parser that always skips a cv
        # slot reads the return type here and returns None or nonsense.
        self.assertEqual(("Y", "A"), rc.split_infix("?f@@YAXXZ"))

    def test_static_member_has_no_cv_slot(self):
        self.assertEqual(("S", "A"), rc.split_infix("?init@Time@@SAHXZ"))

    def test_virtual_member_has_a_cv_slot(self):
        self.assertEqual(("U", "E"), rc.split_infix("?draw@Win@@UAEXXZ"))

    def test_const_this_qualifier_is_skipped(self):
        # `QBE`: cv slot B = const this. The convention is still E.
        self.assertEqual(("Q", "E"), rc.split_infix("?size@S@@QBEHXZ"))

    def test_data_decoration_is_not_a_function(self):
        self.assertIsNone(rc.split_infix("??_7Win@@6B@"))

    def test_unmangled_name_is_not_a_function(self):
        self.assertIsNone(rc.split_infix("sub_401000"))

    def test_a_TEMPLATE_name_is_refused_rather_than_read_at_the_wrong_at_at(self):
        # The infix is located at the FIRST `@@`, which in a template name
        # falls inside the template ARGUMENTS. Here that reads kind `T`
        # (public STATIC) and convention `E`, so the receiver a `QAE` instance
        # method must have is silently dropped and nothing downstream can tell
        # the result from a real static.
        self.assertIsNone(rc.split_infix("?f@?$Vec@PAVFoo@@TEvent@@@@QAEXH@Z"))

    def test_the_same_method_WITHOUT_template_arguments_still_parses(self):
        # So the refusal above is the template marker, not the shape.
        self.assertEqual(("Q", "E"), rc.split_infix("?f@Vec@@QAEXH@Z"))

    def test_both_real_template_names_in_the_catalogue_are_refused(self):
        for name in (
                "?underflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@"
                "MAEHXZ_0",
                "?Reinitialize@?$StructuredWorkStealingQueue@"
                "V_UnrealizedChore@details@Concurrency@@"
                "V_CriticalNonReentrantLock@23@@details@Concurrency@@QAEXXZ"):
            self.assertIsNone(rc.split_infix(name), name)

    def test_kind_and_convention_tables_agree_with_the_infix(self):
        kind_char, conv_char = rc.split_infix("?f@C@@QAAXXZ")
        self.assertEqual("instance", rc.KIND[kind_char][1])
        self.assertEqual("__cdecl", rc.CONV[conv_char])


class ArgumentTest(unittest.TestCase):
    def test_void_is_no_slots(self):
        self.assertEqual([], rc.arg_slots(["void"]))

    def test_pointer_is_one_pointer_slot(self):
        self.assertEqual([True], rc.arg_slots(["struct RECT *"]))

    def test_reference_is_a_pointer_slot(self):
        self.assertEqual([True], rc.arg_slots(["int &"]))

    def test_int_is_one_value_slot(self):
        self.assertEqual([False], rc.arg_slots(["int"]))

    def test_double_occupies_two_slots(self):
        self.assertEqual([False, False], rc.arg_slots(["double"]))

    def test_double_shifts_a_later_pointer(self):
        # The bug this exists for: counting the double as one dword puts the
        # pointer in slot 1 instead of slot 2.
        self.assertEqual([False, False, True],
                         rc.arg_slots(["double", "char *"]))

    def test_varargs_refuses(self):
        self.assertIsNone(rc.arg_slots(["char *", "..."]))

    def test_class_by_value_refuses(self):
        self.assertIsNone(rc.arg_slots(["class StringStruct"]))

    def test_mixed_list(self):
        self.assertEqual([True, False, True],
                         rc.arg_slots(["struct Font *", "int", "struct Win *"]))


class DemangledArgsTest(unittest.TestCase):
    def test_simple(self):
        self.assertEqual(
            "int,int",
            rc.demangled_args("public: void __thiscall S::f(int,int)"))

    def test_function_pointer_parameter_does_not_end_the_list(self):
        # The nested `(` `)` of a function-pointer parameter closes before the
        # real one; a scanner without a depth counter stops at the wrong paren
        # and loses every argument after it.
        text = ("void __stdcall __ArrayUnwind(void *,unsigned int,int,"
                "void (__thiscall*)(void *))")
        self.assertEqual(
            "void *,unsigned int,int,void (__thiscall*)(void *)",
            rc.demangled_args(text))

    def test_split_respects_nesting(self):
        self.assertEqual(
            ["void *", "unsigned int", "int", "void (__thiscall*)(void *)"],
            rc.split_args("void *,unsigned int,int,"
                          "void (__thiscall*)(void *)"))

    def test_no_parentheses(self):
        self.assertIsNone(rc.demangled_args("const Win::`vftable'"))

    def test_a_pointer_to_ARRAY_return_does_not_invent_a_parameter(self):
        # `int (* __cdecl grid_row(void))[8]`. The FIRST `(` opens the RETURN
        # type here, not the argument list: scanning from it yields
        # `* __cdecl grid_row(void)`, which the caller splits into one phantom
        # parameter, so a function taking nothing reads as taking something.
        self.assertEqual(
            "void", rc.demangled_args("int (* __cdecl grid_row(void))[8]"))

    def test_that_phantom_parameter_really_would_have_been_produced(self):
        # The counterpart: the old scan-from-the-first-paren rule, spelled out,
        # so this test fails if the shape above ever stops being dangerous.
        text = "int (* __cdecl grid_row(void))[8]"
        self.assertNotEqual(text.find("("), text.find("(", text.find("__cdecl")))

    def test_a_signature_with_no_convention_word_is_refused(self):
        # Not guessed at from whichever paren happens to come first.
        self.assertIsNone(rc.demangled_args("Win::Win(int)"))


def signature(name, demangled, kind, convention, slots):
    return rc.Signature("0x00400000", name, demangled, kind, convention, slots)


class SignatureTest(unittest.TestCase):
    def test_thiscall_keeps_this_out_of_the_stack_slots(self):
        s = signature("?f@C@@QAEXPAH@Z", "", "instance", "__thiscall", [True])
        self.assertEqual("ecx", s.this_location)
        self.assertEqual([True], s.stack_slots)
        self.assertEqual(0b1, s.pointer_mask)

    def test_cdecl_member_puts_this_in_slot_zero(self):
        s = signature("?f@C@@QAAXPAH@Z", "", "instance", "__cdecl", [True])
        self.assertEqual("stack", s.this_location)
        self.assertEqual([True, True], s.stack_slots)
        self.assertEqual(0b11, s.pointer_mask)

    def test_cdecl_member_shifts_a_value_argument_up_one_slot(self):
        # `this` is slot 0, so the int lands in slot 1 and the mask is 0b01.
        s = signature("?f@C@@QAAXH@Z", "", "instance", "__cdecl", [False])
        self.assertEqual([True, False], s.stack_slots)
        self.assertEqual(0b1, s.pointer_mask)

    def test_free_function_has_no_this(self):
        s = signature("?f@@YAXPAH@Z", "", "free", "__cdecl", [True])
        self.assertEqual("none", s.this_location)
        self.assertEqual([True], s.stack_slots)

    def test_static_member_has_no_this(self):
        s = signature("?f@C@@SAXPAH@Z", "", "static", "__cdecl", [True])
        self.assertEqual("none", s.this_location)
        self.assertEqual([True], s.stack_slots)

    def test_unknown_arguments_produce_no_mask(self):
        s = signature("?f@C@@QAAXZZ", "", "instance", "__cdecl", None)
        self.assertIsNone(s.stack_slots)
        self.assertEqual(0, s.pointer_mask)


class CalleePopTest(unittest.TestCase):
    def test_cdecl_pops_nothing(self):
        s = signature("", "", "instance", "__cdecl", [True, False])
        self.assertEqual(0, s.callee_pop_bytes)

    def test_thiscall_pops_arguments_but_not_this(self):
        s = signature("", "", "instance", "__thiscall", [True, False])
        self.assertEqual(8, s.callee_pop_bytes)

    def test_thiscall_with_no_arguments_pops_nothing(self):
        s = signature("", "", "instance", "__thiscall", [])
        self.assertEqual(0, s.callee_pop_bytes)

    def test_stdcall_member_pops_this_as_well(self):
        s = signature("", "", "instance", "__stdcall", [True, False])
        self.assertEqual(12, s.callee_pop_bytes)

    def test_stdcall_free_function_pops_only_arguments(self):
        s = signature("", "", "free", "__stdcall", [True, False])
        self.assertEqual(8, s.callee_pop_bytes)


class AgreementTest(unittest.TestCase):
    def test_thiscall_with_matching_pop_agrees(self):
        s = signature("", "", "instance", "__thiscall", [False])
        self.assertEqual("agree", rc.agreement(s, rc.BodyEvidence([4], True, 9)))

    def test_zero_argument_thiscall_reading_ecx_agrees(self):
        # The bare-`ret` ambiguity: this MUST NOT be reported as a __cdecl.
        s = signature("", "", "instance", "__thiscall", [])
        self.assertEqual("agree", rc.agreement(s, rc.BodyEvidence([0], True, 9)))

    def test_zero_argument_thiscall_never_reading_ecx_is_flagged(self):
        s = signature("", "", "instance", "__thiscall", [])
        self.assertIn("never reads ECX",
                      rc.agreement(s, rc.BodyEvidence([0], False, 9)))

    def test_one_instruction_body_is_not_flagged_for_ignoring_ecx(self):
        # A `ret`-only stub cannot read ECX and is not evidence of anything.
        s = signature("", "", "instance", "__thiscall", [])
        self.assertEqual("agree", rc.agreement(s, rc.BodyEvidence([0], False, 1)))

    def test_declared_arity_short_of_the_pop_disagrees(self):
        # ?on_redraw@Win@@QAEHXZ: declared no arguments, body is `ret 8`.
        s = signature("", "", "instance", "__thiscall", [])
        self.assertEqual("callee-pop 8 but the name declares 0",
                         rc.agreement(s, rc.BodyEvidence([8], False, 2)))

    def test_declared_arity_beyond_the_pop_disagrees(self):
        s = signature("", "", "instance", "__thiscall", [True] * 6)
        self.assertEqual("callee-pop 16 but the name declares 24",
                         rc.agreement(s, rc.BodyEvidence([16], True, 300)))

    def test_cdecl_with_a_callee_pop_disagrees(self):
        s = signature("", "", "instance", "__cdecl", [False])
        self.assertEqual("callee-pop 4 but the name declares 0",
                         rc.agreement(s, rc.BodyEvidence([4], False, 9)))

    def test_mixed_return_immediates_are_unknown(self):
        s = signature("", "", "instance", "__thiscall", [False])
        self.assertEqual("unknown",
                         rc.agreement(s, rc.BodyEvidence([0, 4], True, 9)))

    def test_unknown_arguments_are_unknown(self):
        s = signature("", "", "instance", "__cdecl", None)
        self.assertEqual("unknown",
                         rc.agreement(s, rc.BodyEvidence([0], True, 9)))

    def test_tail_jump_body_is_unknown(self):
        s = signature("", "", "instance", "__thiscall", [])
        self.assertEqual("unknown", rc.agreement(s, rc.BodyEvidence([], True, 9)))


class ResolveThisTest(unittest.TestCase):
    """Body first. The access char in this catalogue is not trustworthy: 139
    names say `public instance` for functions the machine code and the repo's
    own headers both say are static."""

    def test_thiscall_this_is_in_ecx(self):
        s = signature("", "", "instance", "__thiscall", [False])
        self.assertEqual("ecx", rc.resolve_this_location(
            s, rc.BodyEvidence([4], True, 9)))

    def test_free_function_has_no_this(self):
        s = signature("", "", "free", "__cdecl", [True])
        self.assertEqual("none", rc.resolve_this_location(
            s, rc.BodyEvidence([0], False, 9)))

    def test_stdcall_popping_only_the_arguments_is_static(self):
        # ?window_proc@Win@@QAGJPAXIIJ@Z: four declared arguments, `ret 16`.
        # A pushed `this` would make it `ret 20`.
        s = signature("", "", "instance", "__stdcall", [True, False, False,
                                                        False])
        self.assertEqual("none", rc.resolve_this_location(
            s, rc.BodyEvidence([16], False, 40)))

    def test_stdcall_popping_one_word_more_is_refused_not_called_a_member(self):
        # ?adjust_menus@Win@@QAGHPAX@Z at 0x005F0540 declares ONE argument and
        # pops 8 - 4*(1+1), exactly the shape a pushed `this` makes. It is the
        # ONE of the fourteen __stdcall names that does; the other thirteen pop
        # 4*args, measured against the image. Its body reads [ESP+4] and passes
        # it to an import as a window handle, so slot 0 is the first REAL
        # argument and the name simply under-declares the arity. Claiming
        # "stack" here would put a phantom `this` in slot 0 and shift every
        # argument up one. The fixture is that function's own shape; it used to
        # be a four-argument body popping 20, which is the same algebra but
        # describes nothing in the catalogue.
        s = signature("", "", "instance", "__stdcall", [True])
        self.assertEqual("unresolved", rc.resolve_this_location(
            s, rc.BodyEvidence([8], False, 40)))

    def test_stdcall_with_an_unexplainable_pop_is_unresolved(self):
        s = signature("", "", "instance", "__stdcall", [False])
        self.assertEqual("unresolved", rc.resolve_this_location(
            s, rc.BodyEvidence([12], False, 40)))

    def test_cdecl_member_cannot_be_resolved_from_the_body(self):
        # The caller pops under either layout, so the body is silent and the
        # tool must refuse rather than invent a slot.
        s = signature("", "", "instance", "__cdecl", [True])
        self.assertEqual("unresolved", rc.resolve_this_location(
            s, rc.BodyEvidence([0], False, 9)))

    def test_unresolved_produces_no_slots_and_no_mask(self):
        s = signature("", "", "instance", "__cdecl", [True])
        self.assertIsNone(s.stack_slots_for("unresolved"))
        self.assertEqual(0, s.pointer_mask_for("unresolved"))

    def test_resolved_static_layout_drops_the_phantom_this_slot(self):
        # The whole point: under 'none' the declared pointer stays in slot 0,
        # under 'stack' it is pushed to slot 1. One bit apart, both well formed.
        s = signature("", "", "instance", "__cdecl", [True])
        self.assertEqual(0b1, s.pointer_mask_for("none"))
        self.assertEqual(0b11, s.pointer_mask_for("stack"))

    def test_missing_body_leaves_stdcall_unresolved(self):
        s = signature("", "", "instance", "__stdcall", [False])
        self.assertEqual("unresolved", rc.resolve_this_location(s, None))


class RecoverTest(unittest.TestCase):
    """The whole path, over rows shaped like the catalogue's."""

    ROWS = [
        {"address": "0x00401000", "size": "88",
         "name": "??0StringStruct@@QAE@H@Z"},
        {"address": "0x00604900", "size": "39",
         "name": "?set_def_string_color@BasePop@@QAAXHHHH@Z"},
        {"address": "0x005EC690", "size": "81",
         "name": "?get_hdc@Win@@QAGPAUHDC__@@XZ"},
        {"address": "0x00616880", "size": "14",
         "name": "?init_class@Time@@SAHXZ"},
        {"address": "0x00401058", "size": "16", "name": "sub_401058"},
        {"address": "0x00401070", "size": "16",
         "name": "??__Efoo@@YAXXZ"},
    ]

    def setUp(self):
        import undname
        self.signatures, self.skipped = rc.recover(self.ROWS, undname.undname)
        self.by_address = {s.address: s for s in self.signatures}

    def test_only_real_functions_survive(self):
        self.assertEqual(4, len(self.signatures))
        self.assertEqual(1, self.skipped["no function infix"])
        self.assertEqual(1, self.skipped["dynamic initialiser"])

    def test_thiscall_constructor(self):
        s = self.by_address["0x00401000"]
        self.assertEqual("__thiscall", s.convention)
        self.assertEqual("ecx", s.this_location)
        self.assertEqual([False], s.stack_slots)

    def test_cdecl_member_takes_this_on_the_stack(self):
        s = self.by_address["0x00604900"]
        self.assertEqual("__cdecl", s.convention)
        self.assertEqual("stack", s.this_location)
        self.assertEqual([True, False, False, False, False], s.stack_slots)
        self.assertEqual(0b00001, s.pointer_mask)

    def test_stdcall_member(self):
        s = self.by_address["0x005EC690"]
        self.assertEqual("__stdcall", s.convention)
        self.assertEqual("stack", s.this_location)
        self.assertEqual(4, s.callee_pop_bytes)

    def test_static_member_has_no_this_on_the_stack(self):
        s = self.by_address["0x00616880"]
        self.assertEqual("static", s.kind)
        self.assertEqual("none", s.this_location)
        self.assertEqual([], s.stack_slots)
        self.assertEqual(0, s.callee_pop_bytes)


class LoadRowsTests(unittest.TestCase):
    """Where the catalogue comes from, and the exact shape `main` assumes.

    `load_rows` used to default to docs/recovery/functions.csv, which is
    DELETED - every CLI mode died on FileNotFoundError before printing a line.
    The default is now `emit_translation_unit.load_functions()`, i.e. the
    `ORIGINAL:` annotations in src/, and what these pin is not "it returned
    something" but the three column facts the rest of the module indexes on
    without a fallback: `main` does `int(r["size"])` bare and
    `r["address"].lower()`, `recover` does `row["name"]`. A store that came
    back with an int address, or one row with an empty size, would take the
    tool down again in a way a row count cannot see.
    """

    @classmethod
    def setUpClass(cls):
        cls.rows = rc.load_rows()

    def test_the_default_store_is_src_and_it_is_not_empty(self):
        # Population asserted first: every per-row check below is vacuously
        # true over zero rows, which is how a broken loader passes a suite.
        self.assertGreater(len(self.rows), 5000)

    def test_every_row_carries_the_columns_main_indexes_bare(self):
        for row in self.rows:
            self.assertTrue(row["name"], row["address"])
            self.assertTrue(row["address"].startswith("0x"), row["address"])
            self.assertRegex(row["size"], r"^\d+$")

    def test_an_explicit_path_still_wins(self):
        # The parameter survives only so a regenerated export can be read back
        # and compared against src/. If it silently returned the src rows the
        # comparison would be with itself - the vacuous shape this tree has
        # been bitten by - so the file's own contents must come out.
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "export.csv"
            with path.open("w", newline="") as handle:
                writer = csv.DictWriter(handle, ["address", "name", "size"])
                writer.writeheader()
                writer.writerow({"address": "0x00401000", "size": "88",
                                 "name": "??0StringStruct@@QAE@H@Z"})
            self.assertEqual([{"address": "0x00401000", "size": "88",
                               "name": "??0StringStruct@@QAE@H@Z"}],
                             rc.load_rows(path))

    def test_a_missing_explicit_path_is_an_error_not_a_silent_fallback(self):
        # Asking for a file that is not there and getting 6,000 src rows back
        # would report a comparison that never happened.
        with tempfile.TemporaryDirectory() as folder:
            with self.assertRaises(OSError):
                rc.load_rows(Path(folder) / "nope.csv")


class FakeInstruction:
    def __init__(self, mnemonic):
        self.mnemonic = mnemonic


class PushRunTests(unittest.TestCase):
    """The one piece of `--call-sites` that is arithmetic rather than I/O.

    The call-site evidence used to live in an ad-hoc script and an artifact
    under build/, and the number it produced - "eight targets, three call sites
    on ?flip" - was out by more than an order of magnitude against the
    executable (71 targets; ?flip alone has 40). Nothing could re-derive it, so
    nothing caught it. The scan is now in the tool; this is the part of it that
    can be tested without the user's copy of the game.

    What matters is that a run it cannot read comes back SHORT, never long: a
    zero run is reported as inconclusive, and inventing a longer one is how a
    site would be made to agree with whatever was hoped for.
    """

    @staticmethod
    def seq(*mnemonics):
        return [FakeInstruction(m) for m in mnemonics]

    def test_the_run_stops_at_the_first_non_push(self):
        seq = self.seq("push", "push", "mov", "push", "push", "push", "call")
        self.assertEqual(3, rc.count_pushes_before(seq, 6))

    def test_a_call_with_no_pushes_in_front_reads_as_zero(self):
        # Arguments written with `mov [esp+8], eax` leave no run at all. Zero
        # is the honest answer and the caller scores it inconclusive; a reader
        # that walked past the mov would report a push count the code does not
        # make.
        seq = self.seq("mov", "mov", "call")
        self.assertEqual(0, rc.count_pushes_before(seq, 2))

    def test_the_run_does_not_walk_off_the_front_of_the_body(self):
        seq = self.seq("push", "push", "call")
        self.assertEqual(2, rc.count_pushes_before(seq, 2))

    def test_the_call_itself_is_not_counted(self):
        # Off by one here adds a phantom argument to every site, which is
        # exactly the "pushed this" signal the scan exists to look for.
        seq = self.seq("push", "call", "push")
        self.assertEqual(1, rc.count_pushes_before(seq, 1))

    def test_pushes_after_the_call_belong_to_the_next_call(self):
        seq = self.seq("push", "call", "push", "push", "call")
        self.assertEqual(2, rc.count_pushes_before(seq, 4))


if __name__ == "__main__":
    unittest.main()
