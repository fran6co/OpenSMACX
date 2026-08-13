#!/usr/bin/env python3
"""Tests for derive_prototypes_from_names.py.

The dangerous failure for this catalogue is a prototype that is WRONG rather
than missing: every consumer of the `prototype` column reads it as certain, so a
mis-rendered argument list would silently change which oracles are generated and
which proofs compare EAX. The tests below therefore pin the rendered SHAPE
byte-for-byte against the shape the catalogue already uses, pin the refusals,
and assert that the committed catalogue still parses under the regex the real
consumer uses.

Nothing here needs `undname` or the executable: the demangler is injected, so
the rendering logic is tested on a machine with neither.
"""

from __future__ import annotations

import csv
import io
import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import derive_prototypes_from_names as prototypes  # noqa: E402


def fake_demangler(table):
    return lambda name: table.get(name, "")


def body(observed, evidence=""):
    """A callee purge read straight off the row's own body."""
    return prototypes.Purge(observed, prototypes.SOURCE_BODY, evidence)


def hopped(observed, evidence=""):
    """The same number, but reached by following a tail jump one hop."""
    return prototypes.Purge(observed, prototypes.SOURCE_TAIL_JUMP, evidence)


class TypeSpellingTests(unittest.TestCase):
    """The demangler's alphabet -> IDA's, which is what the column holds."""

    def test_a_struct_pointer_LOSES_the_struct_keyword(self):
        self.assertEqual("RECT*", prototypes.ida_type("struct RECT *"))

    def test_a_class_pointer_LOSES_the_class_keyword(self):
        self.assertEqual("Font*", prototypes.ida_type("class Font *"))

    def test_a_char_pointer_becomes_int8(self):
        self.assertEqual("int8*", prototypes.ida_type("char *"))

    def test_a_const_char_pointer_ALSO_becomes_int8(self):
        # The recorded column spells both the same way; 21 rows prove it.
        self.assertEqual("int8*", prototypes.ida_type("char const *"))

    def test_an_unsigned_char_pointer_becomes_unsigned_int8(self):
        self.assertEqual("unsigned int8*",
                         prototypes.ida_type("unsigned char *"))

    def test_unsigned_long_narrows_to_unsigned_int(self):
        self.assertEqual("unsigned int", prototypes.ida_type("unsigned long"))

    def test_a_reference_is_written_as_a_pointer(self):
        self.assertEqual("Vector*", prototypes.ida_type("class Vector &"))

    def test_a_double_pointer_keeps_both_stars(self):
        self.assertEqual("Buffer**", prototypes.ida_type("struct Buffer * *"))

    def test_a_function_pointer_with_no_arguments_loses_the_void(self):
        self.assertEqual("int (__cdecl *)()",
                         prototypes.ida_type("int (__cdecl*)(void)"))

    def test_a_function_pointer_keeps_its_arguments_comma_spaced(self):
        self.assertEqual("void (__cdecl *)(int, int)",
                         prototypes.ida_type("void (__cdecl*)(int,int)"))

    def test_a_plain_int_is_left_alone(self):
        self.assertEqual("int", prototypes.ida_type("int"))


class StackSlotTests(unittest.TestCase):

    def test_a_scalar_takes_one_slot(self):
        self.assertEqual([1, 1], prototypes.stack_slots(["int", "int8*"]))

    def test_a_double_takes_two_slots(self):
        self.assertEqual([2], prototypes.stack_slots(["double"]))

    def test_an_int64_takes_two_slots(self):
        self.assertEqual([2], prototypes.stack_slots(["__int64"]))

    def test_a_pointer_to_a_double_takes_ONE_slot(self):
        # The width of the pointee is irrelevant; the dangerous bug is counting
        # a `double *` as two slots and shifting every later argument.
        self.assertEqual([1], prototypes.stack_slots(["double*"]))


class RenderTests(unittest.TestCase):
    """The exact shape: `<ret> (<conv> <MANGLED>)(<params>)`."""

    def derive(self, name, demangled):
        return prototypes.derive_one("0x00401000", name, demangled)

    def test_a_method_renders_this_first_and_explicitly(self):
        one = self.derive(
            "?set_view@Console@@QAEXH@Z",
            "public: void __thiscall Console::set_view(int)")
        self.assertEqual(
            "void (__thiscall ?set_view@Console@@QAEXH@Z)(Console* this, int)",
            one.prototype)

    def test_a_receiver_only_method_has_NO_void_parameter(self):
        # The recorded column has zero rows spelling `(void)` and zero with an
        # empty list; a receiver-only method reads `(Console* this)`.
        one = self.derive("?close@Console@@QAEXXZ",
                          "public: void __thiscall Console::close(void)")
        self.assertEqual(
            "void (__thiscall ?close@Console@@QAEXXZ)(Console* this)",
            one.prototype)

    def test_a_constructor_is_given_a_void_return(self):
        one = self.derive("??0StringStruct@@QAE@H@Z",
                          "public: __thiscall StringStruct::StringStruct(int)")
        self.assertEqual(
            "void (__thiscall ??0StringStruct@@QAE@H@Z)"
            "(StringStruct* this, int)", one.prototype)

    def test_a_free_function_has_no_receiver(self):
        one = self.derive("?terraform_cost@@YAHHHH@Z",
                          "int __cdecl terraform_cost(int,int,int)")
        self.assertEqual(
            "int (__cdecl ?terraform_cost@@YAHHHH@Z)(int, int, int)",
            one.prototype)

    def test_a_static_member_is_scoped_but_takes_NO_receiver(self):
        one = self.derive("??3AlphaMovie@@SAXPAXI@Z",
                          "public: static void __cdecl "
                          "AlphaMovie::operator delete(void *,unsigned int)")
        self.assertFalse(one.takes_this)
        self.assertNotIn("this", one.prototype)

    def test_the_virtual_keyword_is_not_mistaken_for_a_return_type(self):
        one = self.derive(
            "??_GDialogs@@UAEPAXI@Z",
            "public: virtual void * __thiscall "
            "Dialogs::`scalar deleting destructor'(unsigned int)")
        self.assertEqual("void*", one.return_type)
        self.assertEqual(
            "void* (__thiscall ??_GDialogs@@UAEPAXI@Z)"
            "(Dialogs* this, unsigned int)", one.prototype)

    def test_a_stdcall_member_keeps_stdcall_AND_the_receiver(self):
        one = self.derive("?load_tga@Buffer@@QAGHPBD@Z",
                          "public: int __stdcall Buffer::load_tga(char const *)")
        self.assertEqual(
            "int (__stdcall ?load_tga@Buffer@@QAGHPBD@Z)(Buffer* this, int8*)",
            one.prototype)

    def test_the_cv_slot_is_NOT_read_as_the_convention(self):
        # `QAE` is public/instance/__thiscall. Reading the `A` as the
        # convention is the classic mis-parse and would say __cdecl.
        one = self.derive("?close@Console@@QAEXXZ",
                          "public: void __thiscall Console::close(void)")
        self.assertEqual("__thiscall", one.convention)

    def test_a_function_pointer_argument_survives_the_comma_inside_it(self):
        one = self.derive(
            "?init@Time@@QAEXP6AXHH@ZHHHH@Z",
            "public: void __thiscall "
            "Time::init(void (__cdecl*)(int,int),int,int,int,int)")
        self.assertEqual(5, len(one.argument_types))
        self.assertEqual("void (__cdecl *)(int, int)", one.argument_types[0])

    def test_the_receiver_type_comes_from_the_SCOPE_not_the_mangled_class(self):
        # 152 recorded rows disagree with the mangled class; the demangled
        # scope is the authority, so a thunk names the class it belongs to.
        one = self.derive("?on_mouse_move@BaseWin@@QAEXHH@Z",
                          "public: void __thiscall BaseWin::on_mouse_move(int,int)")
        self.assertEqual("BaseWin", one.receiver)


    def test_a_virtual_DESTRUCTOR_does_not_publish_virtual_as_a_return_type(self):
        # `public: virtual __thiscall Foo::~Foo(void)` - the head is exactly
        # `public: virtual`, with no trailing space, so a `^(virtual|static)\s+`
        # strip leaves the word behind and the row publishes
        # `virtual (__thiscall ...)`.
        one = self.derive("??1Foo@@UAE@XZ",
                          "public: virtual __thiscall Foo::~Foo(void)")
        self.assertEqual("void", one.return_type)
        self.assertEqual("void (__thiscall ??1Foo@@UAE@XZ)(Foo* this)",
                         one.prototype)

    def test_a_static_with_no_return_type_is_stripped_the_same_way(self):
        one = self.derive("??1Foo@@SAE@XZ",
                          "public: static __cdecl Foo::~Foo(void)")
        self.assertEqual("void", one.return_type)


class ThunkSpellingTests(unittest.TestCase):
    """The two thunk spellings, and the purge each one implies.

    47 catalogued rows are named `??3<Class>@@SAXPAXI@Z` - a static __cdecl
    `operator delete`, which implies a callee purge of 0 - and every one of
    their bodies is `sub ecx, <imm> ; jmp <the class's own ??_G>` with the
    target executing `ret 4`. 0x00404430 is 11 bytes long, touches no stack
    slot and takes its receiver in ECX. These are adjustor thunks into the
    scalar deleting destructor, and the correct spelling of a thunk implies
    __thiscall with one `unsigned int` argument - a purge of 4, which is what
    the tail jump was measured to execute.

    Derivation has to come FIRST. A rename that landed while these spellings
    were underivable would replace 47 refusals nobody can miss with 47 rows
    that quietly fall out of the published catalogue.
    """

    ADJUSTOR = "??_GAlphaMovie@@WEEE@AEPAXI@Z"
    ADJUSTOR_DEMANGLED = (
        "[thunk]:public: virtual void * __thiscall "
        "AlphaMovie::`scalar deleting destructor'`adjustor{1092}' "
        "(unsigned int)")
    VTORDISP = "??_GPlanWin@@$4PPPPPPPM@A@AEPAXI@Z"
    VTORDISP_DEMANGLED = (
        "[thunk]:public: virtual void * __thiscall "
        "PlanWin::`scalar deleting destructor'`vtordisp{4294967292,0}' "
        "(unsigned int)")

    def test_the_adjustor_thunk_at_0x00404430_implies_the_purge_it_executes(self):
        one = prototypes.derive_one("0x00404430", self.ADJUSTOR,
                                    self.ADJUSTOR_DEMANGLED)
        self.assertIsNotNone(one)
        self.assertEqual("__thiscall", one.convention)
        self.assertEqual("AlphaMovie", one.receiver)
        self.assertEqual(["unsigned int"], one.argument_types)
        # 4, matching the `ret 4` the tail-jump target executes. The `??3`
        # spelling the catalogue carries implies 0 on the same bytes.
        self.assertEqual(4, prototypes.implied_purge(one))

    def test_the_vtordisp_thunk_at_0x0048BF10_implies_the_same_4(self):
        one = prototypes.derive_one("0x0048BF10", self.VTORDISP,
                                    self.VTORDISP_DEMANGLED)
        self.assertIsNotNone(one)
        self.assertEqual("__thiscall", one.convention)
        self.assertEqual("PlanWin", one.receiver)
        self.assertEqual(4, prototypes.implied_purge(one))

    def test_the_thunk_MARKER_is_not_published_as_a_return_type(self):
        # The demangler writes `[thunk]:public: virtual void * ...`, so the
        # anchored `public:` strip matches nothing and the marker, the access
        # and the `virtual` all land in the return type unless the marker goes
        # first. Measured before the fix: `[thunk]:public: virtual void*`.
        one = prototypes.derive_one("0x00404430", self.ADJUSTOR,
                                    self.ADJUSTOR_DEMANGLED)
        self.assertEqual("void*", one.return_type)
        self.assertEqual(
            "void* (__thiscall ??_GAlphaMovie@@WEEE@AEPAXI@Z)"
            "(AlphaMovie* this, unsigned int)", one.prototype)

    def test_the_catalogued_operator_delete_spelling_still_implies_ZERO(self):
        # The contrast that makes the refusal a finding rather than a quirk:
        # same address, same bytes, two names, two different implied purges.
        one = prototypes.derive_one(
            "0x00404430", "??3AlphaMovie@@SAXPAXI@Z",
            "public: static void __cdecl "
            "AlphaMovie::operator delete(void *,unsigned int)")
        self.assertEqual(0, prototypes.implied_purge(one))
        self.assertEqual(
            prototypes.CONTRADICTED,
            prototypes.purge_verdict(one, prototypes.Purge(
                4, prototypes.SOURCE_TAIL_JUMP, "the target executes ret 4")))

    def test_the_thunk_spelling_is_VERIFIED_by_the_same_observation(self):
        one = prototypes.derive_one("0x00404430", self.ADJUSTOR,
                                    self.ADJUSTOR_DEMANGLED)
        self.assertEqual(
            prototypes.VERIFIED_VIA_TAIL_JUMP,
            prototypes.purge_verdict(one, prototypes.Purge(
                4, prototypes.SOURCE_TAIL_JUMP, "the target executes ret 4")))

    def test_a_thunk_row_survives_the_control_that_gates_publication(self):
        # `emitted_control` re-reads each published row from the demangler,
        # independently of `split_infix`. A row that parses one way and reads
        # another way must never ship.
        demangler = {self.ADJUSTOR: self.ADJUSTOR_DEMANGLED,
                     self.VTORDISP: self.VTORDISP_DEMANGLED}
        found = [prototypes.derive_one("0x00404430", self.ADJUSTOR,
                                       self.ADJUSTOR_DEMANGLED),
                 prototypes.derive_one("0x0048BF10", self.VTORDISP,
                                       self.VTORDISP_DEMANGLED)]
        self.assertEqual([], prototypes.emitted_control(found, demangler.get))


class RefusalTests(unittest.TestCase):

    def test_a_TEMPLATE_name_is_refused_because_the_infix_cannot_be_found(self):
        # `split_infix` reads the infix at the FIRST `@@`, which inside a
        # template name falls in the template ARGUMENTS: this one parses as
        # kind `T` (public static) convention `E`, silently dropping the
        # receiver that `QAE` demands. Refusing is the only honest answer.
        self.assertIsNone(prototypes.derive_one(
            "0x00401000", "?f@?$Vec@PAVFoo@@TEvent@@@@QAEXH@Z",
            "public: void __thiscall Vec<class Foo *,enum Event>::f(int)"))

    def test_the_same_name_WITHOUT_the_template_is_still_derived(self):
        # So the refusal above is the template marker doing the work, and not
        # the whole shape having become underivable.
        one = prototypes.derive_one(
            "0x00401000", "?f@Vec@@QAEXH@Z",
            "public: void __thiscall Vec::f(int)")
        self.assertEqual("Vec", one.receiver)

    def test_a_vftable_is_refused_because_it_is_not_a_function(self):
        self.assertIsNone(prototypes.derive_one(
            "0x00401000", "??_7Console@@6B@", "const Console::`vftable'"))

    def test_a_varargs_function_is_refused_rather_than_truncated(self):
        # The caller decides a varargs layout, so there is no argument list to
        # state; emitting the fixed part would understate the ABI.
        self.assertIsNone(prototypes.derive_one(
            "0x00401000", "?parse@@YAXPADZZ",
            "void __cdecl parse(char *,...)"))

    def test_an_unmangled_name_is_refused(self):
        self.assertIsNone(prototypes.derive_one(
            "0x00401000", "sub_401000", ""))

    def test_derive_NEVER_overwrites_a_recorded_prototype(self):
        rows = [{"address": "0x00401000", "name": "?close@Console@@QAEXXZ",
                 "prototype": "void (__thiscall ?close@Console@@QAEXXZ)"
                              "(Console* this)"}]
        found, refused = prototypes.derive(rows, fake_demangler({}))
        self.assertEqual([], found)
        self.assertEqual(1, refused["already recorded"])

    def test_derive_emits_for_a_row_with_no_prototype(self):
        rows = [{"address": "0x00401000", "name": "?close@Console@@QAEXXZ",
                 "prototype": ""}]
        found, _ = prototypes.derive(rows, fake_demangler(
            {"?close@Console@@QAEXXZ":
             "public: void __thiscall Console::close(void)"}))
        self.assertEqual(1, len(found))


class ControlTests(unittest.TestCase):
    """The control has to be able to FAIL, or it certifies nothing."""

    RECORDED = ("void (__thiscall ?close@Console@@QAEXXZ)(Console* this)")
    DEMANGLED = {"?close@Console@@QAEXXZ":
                 "public: void __thiscall Console::close(void)"}

    def rows(self, recorded=None):
        return [{"address": "0x00401000", "name": "?close@Console@@QAEXXZ",
                 "prototype": recorded or self.RECORDED}]

    def test_an_agreeing_row_scores_on_every_axis(self):
        measured = prototypes.control(self.rows(),
                                      fake_demangler(self.DEMANGLED))
        self.assertEqual(1, measured.population)
        for key in prototypes.AGREEMENT_FLOOR:
            self.assertEqual(1, measured.agree[key], key)

    def test_a_convention_disagreement_is_COUNTED_not_swallowed(self):
        measured = prototypes.control(
            self.rows("void (__cdecl ?close@Console@@QAEXXZ)(Console* this)"),
            fake_demangler(self.DEMANGLED))
        self.assertEqual(0, measured.agree["convention"])
        self.assertEqual(1, len(measured.disagreements["convention"]))

    def test_a_dropped_receiver_is_counted_against_the_receiver_axis(self):
        measured = prototypes.control(
            self.rows("void (__thiscall ?close@Console@@QAEXXZ)(int)"),
            fake_demangler(self.DEMANGLED))
        self.assertEqual(0, measured.agree["receiver"])

    def test_an_extra_argument_breaks_the_stack_layout_axis(self):
        measured = prototypes.control(
            self.rows("void (__thiscall ?close@Console@@QAEXXZ)"
                      "(Console* this, int)"),
            fake_demangler(self.DEMANGLED))
        self.assertEqual(0, measured.agree["stack layout"])

    def test_failures_names_every_axis_that_fell_below_its_floor(self):
        # population_floor=0 so this exercises the RATIO axes alone; the
        # population pin has its own tests below.
        measured = prototypes.Control(population_floor=0)
        measured.population = 100
        measured.agree = {key: 0 for key in prototypes.AGREEMENT_FLOOR}
        self.assertEqual(len(prototypes.AGREEMENT_FLOOR),
                         len(measured.failures()))

    def test_failures_is_empty_when_every_axis_is_perfect(self):
        measured = prototypes.Control(population_floor=0)
        measured.population = 100
        measured.agree = {key: 100 for key in prototypes.AGREEMENT_FLOOR}
        self.assertEqual([], measured.failures())

    def test_a_shrinking_population_is_caught_though_every_RATE_holds(self):
        # The hole this closes: every axis above is a ratio, and a ratio cannot
        # see its own denominator fall. Rows silently leaving the control keep
        # all five green on less evidence than was pinned.
        measured = prototypes.Control(population_floor=3213)
        measured.population = 2892          # a 10% loss, rates untouched
        measured.agree = {key: 2892 for key in prototypes.AGREEMENT_FLOOR}
        self.assertEqual([], [f for f in measured.failures()
                              if "population" not in f],
                         "no RATIO axis should complain - that is the point")
        failures = measured.failures()
        self.assertEqual(1, len(failures))
        self.assertIn("control population 2892", failures[0])
        self.assertIn("3213", failures[0])

    def test_the_population_floor_matches_the_pinned_control_size(self):
        # The floor and the documented control size are the same number, so a
        # future re-measurement cannot move one and leave the other behind.
        self.assertEqual(prototypes.CONTROL_POPULATION_FLOOR,
                         prototypes.AGREEMENT_FLOOR["convention"][1])

    def test_a_parameter_NAME_never_counts_as_a_disagreement(self):
        # `int8* sectionID` and `int8*` are the same type; the name is
        # information the mangling never had.
        self.assertEqual("int8*",
                         prototypes.strip_parameter_name("int8* sectionID"))

    def test_unsigned_int_is_not_mistaken_for_a_named_parameter(self):
        self.assertEqual("unsigned int",
                         prototypes.strip_parameter_name("unsigned int"))

    def test_the_inner_this_of_a_function_pointer_is_stripped_as_a_name(self):
        # IDA writes `void (__cdecl *)(int this)`; that `this` is a parameter
        # name on an inner argument, not a receiver.
        self.assertEqual("void (__cdecl *)(int)",
                         prototypes.strip_parameter_name(
                             "void (__cdecl *)(int this)"))


class EmittedControlTests(unittest.TestCase):

    def test_a_clean_row_reports_no_problem(self):
        table = {"?close@Console@@QAEXXZ":
                 "public: void __thiscall Console::close(void)"}
        found, _ = prototypes.derive(
            [{"address": "0x00401000", "name": "?close@Console@@QAEXXZ",
              "prototype": ""}], fake_demangler(table))
        self.assertEqual([], prototypes.emitted_control(
            found, fake_demangler(table)))

    def test_a_convention_that_the_demangler_contradicts_is_REPORTED(self):
        one = prototypes.Derived("0x00401000", "?close@Console@@QAEXXZ",
                                 "__cdecl", "instance", "void", [], "Console")
        problems = prototypes.emitted_control([one], fake_demangler(
            {"?close@Console@@QAEXXZ":
             "public: void __thiscall Console::close(void)"}))
        self.assertEqual(1, len(problems))
        self.assertIn("convention", problems[0])

    def test_an_argument_count_the_demangler_contradicts_is_REPORTED(self):
        one = prototypes.Derived("0x00401000", "?close@Console@@QAEXXZ",
                                 "__thiscall", "instance", "void",
                                 ["int"], "Console")
        problems = prototypes.emitted_control([one], fake_demangler(
            {"?close@Console@@QAEXXZ":
             "public: void __thiscall Console::close(void)"}))
        self.assertEqual(1, len(problems))
        self.assertIn("argument", problems[0])


class DynamicInitialiserTests(unittest.TestCase):
    """The single largest published category, which the docstring denied.

    `derive_one` claimed in prose that a `??__E` dynamic initialiser is refused
    as "not a function". It is not refused, and it should not be: 788 of the
    published rows are `??__E`/`??__F`, 49% of the catalogue, and a dynamic
    initialiser really is a `void __cdecl f(void)`. These pin the OUTPUT, so the
    prose can never drift away from it again without a test saying so.
    """

    def test_a_dynamic_initialiser_IS_derived(self):
        one = prototypes.derive_one(
            "0x00402F10", "??__Eg_MAIN_MENU@@YAXXZ",
            "void __cdecl `dynamic initializer for 'g_MAIN_MENU''(void)")
        self.assertIsNotNone(one)
        self.assertEqual("__cdecl", one.convention)
        self.assertEqual([], one.argument_types)
        self.assertFalse(one.takes_this)

    def test_a_dynamic_atexit_destructor_IS_derived(self):
        one = prototypes.derive_one(
            "0x00402F30", "??__Fg_MAIN_MENU@@YAXXZ",
            "void __cdecl `dynamic atexit destructor for 'g_MAIN_MENU''(void)")
        self.assertIsNotNone(one)
        self.assertEqual("void (__cdecl ??__Fg_MAIN_MENU@@YAXXZ)()",
                         one.prototype)


class PurgeGateTests(unittest.TestCase):
    """The machine-code gate: what the prototype IMPLIES the callee pops.

    Nothing here reads the executable - the observed purge is injected, exactly
    as the demangler is, so the arithmetic and the refusal are testable on a
    machine with no copy of the game.
    """

    def derived(self, name, demangled):
        return prototypes.derive_one("0x00401000", name, demangled)

    def test_a_cdecl_function_implies_a_purge_of_zero(self):
        one = self.derived("?f@@YAXHH@Z", "void __cdecl f(int,int)")
        self.assertEqual(0, prototypes.implied_purge(one))

    def test_a_thiscall_method_charges_its_ARGUMENTS_and_not_this(self):
        one = self.derived("?f@C@@QAEXHH@Z",
                           "public: void __thiscall C::f(int,int)")
        self.assertTrue(one.takes_this)
        self.assertEqual(8, prototypes.implied_purge(one))

    def test_a_STDCALL_method_ALSO_leaves_this_in_ECX(self):
        # The eight-times-repeated bug. `?window_proc@Win@@QAGJPAXIIJ@Z`
        # declares four arguments plus a receiver and executes `ret 0x10`, not
        # `ret 0x14`; all eight QAG bodies in this image agree and none
        # disagrees. Charging `this` to the stack here is what contradicted
        # every one of them.
        one = self.derived("?window_proc@Win@@QAGJPAXIIJ@Z",
                           "public: long __stdcall "
                           "Win::window_proc(void *,unsigned int,"
                           "unsigned int,long)")
        self.assertTrue(one.takes_this)
        self.assertEqual(16, prototypes.implied_purge(one))

    def test_a_receiver_only_stdcall_method_implies_a_purge_of_zero(self):
        one = self.derived("?get_key_window@Win@@QAGHXZ",
                           "public: int __stdcall Win::get_key_window(void)")
        self.assertEqual(0, prototypes.implied_purge(one))

    def test_a_double_argument_is_charged_TWO_slots(self):
        one = self.derived("?f@C@@QAEXN@Z",
                           "public: void __thiscall C::f(double)")
        self.assertEqual(8, prototypes.implied_purge(one))

    def test_a_convention_with_no_modelled_rule_implies_NOTHING(self):
        one = prototypes.Derived("0x00401000", "?f@@YIXH@Z", "__fastcall",
                                 "free", "void", ["int"], None)
        self.assertIsNone(prototypes.implied_purge(one))
        self.assertEqual(prototypes.UNKNOWN,
                         prototypes.purge_verdict(one, body(0)))

    def test_agreement_with_the_bytes_is_VERIFIED(self):
        one = self.derived("?f@C@@QAEXHH@Z",
                           "public: void __thiscall C::f(int,int)")
        self.assertEqual(prototypes.VERIFIED,
                         prototypes.purge_verdict(one, body(8)))

    def test_disagreement_with_the_bytes_is_CONTRADICTED(self):
        one = self.derived("?f@C@@QAEXHH@Z",
                           "public: void __thiscall C::f(int,int)")
        self.assertEqual(prototypes.CONTRADICTED,
                         prototypes.purge_verdict(one, body(16)))

    def test_a_body_that_never_said_is_UNKNOWN_and_never_CONTRADICTED(self):
        one = self.derived("?f@C@@QAEXHH@Z",
                           "public: void __thiscall C::f(int,int)")
        self.assertEqual(prototypes.UNKNOWN,
                         prototypes.purge_verdict(one, prototypes.NO_PURGE))

    # -- a hopped verdict is NOT a direct observation ------------------------

    def test_the_SAME_number_reached_by_a_hop_reads_differently(self):
        # The whole point of the source column. Both of these agree with the
        # prototype; only one of them was read off the row's own body, and a
        # reader must be able to tell which without going back to the bytes.
        one = self.derived("?f@C@@QAEXHH@Z",
                           "public: void __thiscall C::f(int,int)")
        self.assertEqual(prototypes.VERIFIED,
                         prototypes.purge_verdict(one, body(8)))
        self.assertEqual(prototypes.VERIFIED_VIA_TAIL_JUMP,
                         prototypes.purge_verdict(one, hopped(8)))

    def test_a_hopped_verdict_is_never_the_bare_word_verified(self):
        self.assertNotEqual(prototypes.VERIFIED,
                            prototypes.VERIFIED_VIA_TAIL_JUMP)

    def test_a_hopped_CONTRADICTION_is_still_a_contradiction(self):
        one = self.derived("?f@C@@QAEXHH@Z",
                           "public: void __thiscall C::f(int,int)")
        self.assertEqual(prototypes.CONTRADICTED,
                         prototypes.purge_verdict(one, hopped(4)))

    # -- the gate as `derive` applies it ------------------------------------

    ROW = [{"address": "0x00401000", "name": "?f@C@@QAEXHH@Z",
            "prototype": ""}]
    TABLE = {"?f@C@@QAEXHH@Z": "public: void __thiscall C::f(int,int)"}

    def test_a_contradicted_row_is_NOT_published(self):
        found, refused = prototypes.derive(
            self.ROW, fake_demangler(self.TABLE),
            {"0x00401000": body(16, "one ret imm over 1 body span(s)")})
        self.assertEqual([], found)
        self.assertEqual(1, refused[prototypes.PURGE_REFUSAL])

    def test_a_contradicted_row_is_counted_under_ITS_OWN_reason(self):
        _found, refused = prototypes.derive(
            self.ROW, fake_demangler(self.TABLE), {"0x00401000": body(16)})
        self.assertEqual(0, refused["not a function"])
        self.assertEqual(0, refused["unmangled"])

    def test_a_HOPPED_contradiction_is_refused_under_a_SEPARATE_reason(self):
        # A refusal that rests on a hop rests on more indirection than one that
        # rests on the row's own epilogue, and the tally has to say so.
        _found, refused = prototypes.derive(
            self.ROW, fake_demangler(self.TABLE), {"0x00401000": hopped(16)})
        self.assertEqual(1, refused[prototypes.PURGE_REFUSAL_HOP])
        self.assertEqual(0, refused[prototypes.PURGE_REFUSAL])

    def test_an_agreeing_row_is_published_and_marked_verified(self):
        found, refused = prototypes.derive(
            self.ROW, fake_demangler(self.TABLE), {"0x00401000": body(8)})
        self.assertEqual(1, len(found))
        self.assertEqual(prototypes.VERIFIED, found[0].purge)
        self.assertEqual(0, refused[prototypes.PURGE_REFUSAL])

    def test_a_row_agreeing_VIA_A_HOP_carries_the_hopped_word(self):
        found, _ = prototypes.derive(
            self.ROW, fake_demangler(self.TABLE), {"0x00401000": hopped(8)})
        self.assertEqual(prototypes.VERIFIED_VIA_TAIL_JUMP, found[0].purge)

    def test_an_unevidenced_row_is_published_but_marked_UNKNOWN(self):
        # Absence of evidence is not a contradiction, and must never read as
        # agreement either.
        found, _ = prototypes.derive(
            self.ROW, fake_demangler(self.TABLE),
            {"0x00401000": prototypes.NO_PURGE})
        self.assertEqual(1, len(found))
        self.assertEqual(prototypes.UNKNOWN, found[0].purge)

    def test_a_row_the_table_never_mentions_is_UNKNOWN_not_verified(self):
        found, _ = prototypes.derive(self.ROW, fake_demangler(self.TABLE), {})
        self.assertEqual(prototypes.UNKNOWN, found[0].purge)

    def test_deriving_with_no_table_at_all_marks_everything_UNKNOWN(self):
        found, _ = prototypes.derive(self.ROW, fake_demangler(self.TABLE))
        self.assertEqual(prototypes.UNKNOWN, found[0].purge)

    # -- the committed purge catalogue --------------------------------------

    def test_the_purge_catalogue_round_trips_through_its_own_renderer(self):
        table = {"0x00401000": body(12, "one ret imm over 1 body span(s)"),
                 "0x00402000": hopped(4, "one ret imm in the tail-jump target "
                                         "0x00403000 over 1 body span(s)"),
                 "0x00403000": prototypes.Purge(
                     None, prototypes.SOURCE_NONE, "a chain")}
        text = prototypes.render_purges(table)
        self.assertNotIn("\r\n", text)
        path = Path(tempfile.mkdtemp()) / "callee-purge.csv"
        path.write_text(text, encoding="utf-8")
        self.assertEqual(table, prototypes.load_purges(path))

    def test_the_SOURCE_survives_the_round_trip_and_is_not_reset(self):
        table = {"0x00401000": hopped(4, "hopped")}
        path = Path(tempfile.mkdtemp()) / "callee-purge.csv"
        path.write_text(prototypes.render_purges(table), encoding="utf-8")
        self.assertEqual(prototypes.SOURCE_TAIL_JUMP,
                         prototypes.load_purges(path)["0x00401000"].source)

    def test_a_PRE_HOP_catalogue_with_no_source_column_is_REFUSED(self):
        # The dangerous stale state: a three-column file whose numbers would
        # otherwise be read as direct observations of the body.
        path = Path(tempfile.mkdtemp()) / "callee-purge.csv"
        path.write_text("address,purge,evidence\n0x00401000,0x4,one ret imm\n",
                        encoding="utf-8")
        with self.assertRaises(ValueError) as caught:
            prototypes.load_purges(path)
        self.assertIn("no source column", str(caught.exception))

    def test_an_UNKNOWN_row_with_no_source_is_not_an_error(self):
        # There is no number to misread, so there is nothing to refuse.
        path = Path(tempfile.mkdtemp()) / "callee-purge.csv"
        path.write_text("address,purge,source,evidence\n"
                        "0x00401000,unknown,,why\n", encoding="utf-8")
        self.assertIsNone(prototypes.load_purges(path)["0x00401000"].observed)

    def test_an_unknown_purge_is_spelled_out_and_not_left_blank(self):
        text = prototypes.render_purges(
            {"0x00401000": prototypes.Purge(None, prototypes.SOURCE_NONE,
                                            "why")})
        self.assertIn(f",{prototypes.UNKNOWN},", text)

    def test_a_missing_purge_catalogue_is_empty_not_an_error(self):
        self.assertEqual({}, prototypes.load_purges(Path("/nonexistent.csv")))


class Insn:
    """The four fields the hop's rules read off a decoded instruction."""

    def __init__(self, address, size, mnemonic, op_str=""):
        self.address = address
        self.size = size
        self.mnemonic = mnemonic
        self.op_str = op_str


def sequence(*items, start=0x401000):
    """Instructions laid end to end from `start`; each item is (mnem, ops)."""
    out = []
    address = start
    for mnemonic, op_str in items:
        out.append(Insn(address, 2, mnemonic, op_str))
        address += 2
    return out, address


class TailJumpTests(unittest.TestCase):
    """ONE hop, and every condition that makes it a refusal instead.

    Nothing here decodes anything: the rules take a sequence of instructions,
    so they are testable on a machine with no capstone and no executable, which
    is the same bar the rest of this file meets.
    """

    def test_a_direct_tail_jump_names_its_target(self):
        seq, end = sequence(("mov", "ecx, 0x8c6e68"), ("jmp", "0x4bdea0"))
        self.assertEqual(0x4BDEA0, prototypes.tail_jump_target(seq, end))

    def test_a_REGISTER_jump_names_no_target(self):
        seq, end = sequence(("mov", "eax, 4"), ("jmp", "eax"))
        self.assertIsNone(prototypes.tail_jump_target(seq, end))

    def test_a_MEMORY_jump_names_no_target(self):
        seq, end = sequence(("jmp", "dword ptr [0x6d1234]"))
        self.assertIsNone(prototypes.tail_jump_target(seq, end))

    def test_a_CONDITIONAL_branch_is_not_a_terminator(self):
        seq, end = sequence(("test", "eax, eax"), ("je", "0x401020"))
        self.assertIsNone(prototypes.tail_jump_target(seq, end))

    def test_a_CALL_is_not_a_tail_jump(self):
        seq, end = sequence(("call", "0x401020"))
        self.assertIsNone(prototypes.tail_jump_target(seq, end))

    def test_a_sweep_that_STOPS_SHORT_of_the_body_end_names_no_target(self):
        # A linear sweep that desynchronised on embedded data can decode a
        # `jmp` out of the middle of some other instruction; the address it
        # names was never a branch target. The body's last byte is the check.
        seq, end = sequence(("jmp", "0x4bdea0"))
        self.assertIsNone(prototypes.tail_jump_target(seq, end + 6))

    def test_an_empty_sweep_names_no_target(self):
        self.assertIsNone(prototypes.tail_jump_target([], 0x401000))

    # -- stack neutrality, which is what makes the target's purge the thunk's -

    def test_a_mov_only_thunk_provably_leaves_the_stack_pointer_alone(self):
        seq, _ = sequence(("mov", "ecx, 0x8c6e68"))
        self.assertIsNone(prototypes.stack_shift_reason(seq))

    def test_the_sub_ecx_adjustor_shape_leaves_the_stack_pointer_alone(self):
        # The 47 refused rows. ECX is the receiver; ESP is untouched, which is
        # exactly why the target's `ret 4` IS the thunk's purge and the
        # published `__cdecl ... purge 0` is wrong.
        seq, _ = sequence(("sub", "ecx, 0x444"))
        self.assertIsNone(prototypes.stack_shift_reason(seq))

    def test_a_PUSH_before_the_jump_is_a_refusal(self):
        seq, _ = sequence(("push", "0"))
        self.assertIn("moves the stack pointer",
                      prototypes.stack_shift_reason(seq))

    def test_a_CALL_before_the_jump_is_a_refusal(self):
        seq, _ = sequence(("call", "0x436090"))
        self.assertIn("calls out first", prototypes.stack_shift_reason(seq))

    def test_NAMING_esp_in_an_operand_is_a_refusal(self):
        seq, _ = sequence(("mov", "ecx, dword ptr [esp + 4]"))
        self.assertIn("names the stack pointer",
                      prototypes.stack_shift_reason(seq))

    def test_esp_is_matched_as_a_WORD_and_not_inside_another_name(self):
        seq, _ = sequence(("mov", "ecx, dword ptr [esponge]"))
        self.assertIsNone(prototypes.stack_shift_reason(seq))

    # -- reading the ret out of a target ------------------------------------

    def test_a_bare_ret_pops_nothing(self):
        seq, _ = sequence(("ret", ""))
        self.assertEqual({0}, prototypes.ret_immediates(seq))

    def test_a_ret_imm_pops_what_it_says(self):
        seq, _ = sequence(("ret", "4"))
        self.assertEqual({4}, prototypes.ret_immediates(seq))

    def test_two_different_rets_are_TWO_answers_and_not_one(self):
        seq, _ = sequence(("ret", ""), ("ret", "0x10"))
        self.assertEqual({0, 16}, prototypes.ret_immediates(seq))


class HopAgreementTests(unittest.TestCase):
    """The built-in sanity check: a hop that mostly contradicts is a bad hop."""

    def one(self, address="0x00401000"):
        return prototypes.derive_one(
            address, "?f@C@@QAEXHH@Z",
            "public: void __thiscall C::f(int,int)")

    def test_a_hopped_row_that_agrees_is_counted_as_agreement(self):
        agreeing, unscorable, contradictions = prototypes.hop_agreement(
            [self.one()], {"0x00401000": hopped(8)})
        self.assertEqual((1, 0, []), (agreeing, unscorable, contradictions))

    def test_a_hopped_row_that_disagrees_is_counted_as_a_contradiction(self):
        agreeing, _unscorable, contradictions = prototypes.hop_agreement(
            [self.one()], {"0x00401000": hopped(4)})
        self.assertEqual(0, agreeing)
        self.assertEqual(1, len(contradictions))

    def test_a_row_read_off_its_OWN_body_is_not_in_the_hop_population(self):
        # The control has to measure the hop, not the whole catalogue.
        self.assertEqual((0, 0, []), prototypes.hop_agreement(
            [self.one()], {"0x00401000": body(8)}))

    def test_the_pinned_floor_is_a_majority_and_not_a_formality(self):
        agreeing, sample = prototypes.HOP_AGREEMENT_FLOOR
        self.assertGreater(agreeing / sample, 0.5)


class HypothesisTests(unittest.TestCase):
    """The weak source stays weak, and stays separate."""

    ROWS = [{"address": "0x00401520", "name": "sub_401520", "prototype": ""}]
    IDA = [{"address": "0x00401520",
            "type": "int __thiscall(void *Block, char)"}]

    def test_an_argument_count_is_published(self):
        out = prototypes.hypotheses(self.ROWS, self.IDA)
        self.assertEqual(1, len(out))
        self.assertEqual(2, out[0][2])

    def test_the_hypothesis_file_carries_NO_calling_convention_column(self):
        # IDA's convention agrees only 30.3% of the time. Publishing it beside
        # an argument count would let a reader treat it as equally supported.
        self.assertNotIn("convention", prototypes.HYPOTHESIS_FIELDS)

    def test_the_hypothesis_file_carries_NO_prototype_column(self):
        self.assertNotIn("prototype", prototypes.HYPOTHESIS_FIELDS)

    def test_no_convention_reaches_a_CLAIM_column_of_the_hypothesis_file(self):
        """No column a consumer reads as a claim carries a convention token.

        THIS TEST USED TO ASSERT NOTHING. It read
        `assertNotIn("__thiscall(", text.split("evidence")[0])`, and "evidence"
        first occurs in the HEADER - so the string it inspected was the
        24-character `address,name,argument_count,` and no input could ever
        have failed it.

        Written honestly, the property as originally stated is FALSE and should
        be: 581 of the 906 committed rows do carry a convention token, every one
        of them inside `evidence`, because `evidence` is a verbatim quote of
        what IDA guessed and quoting a source exactly is not endorsing it. The
        property that must hold - and that the docstring on `hypotheses` is
        actually claiming - is that the columns a consumer reads as this file's
        own assertion carry none, and that the quote is always attributed to its
        source so it cannot be mistaken for one.
        """
        text = prototypes.render_hypotheses(
            prototypes.hypotheses(self.ROWS, self.IDA))
        rows = list(csv.DictReader(io.StringIO(text)))
        self.assertTrue(rows)
        for row in rows:
            for field in prototypes.HYPOTHESIS_FIELDS:
                if field == "evidence":
                    continue
                self.assertIsNone(
                    prototypes.CONVENTION_TOKEN.search(row[field]),
                    f"{field} carries a calling convention: {row[field]!r}")
            self.assertTrue(row["evidence"].startswith("IDA 9.4 guess_type "),
                            row["evidence"])

    def test_the_evidence_column_really_does_quote_IDAs_convention(self):
        # The counterpart of the test above: it is not passing because nothing
        # in the file ever contains a convention. The quote is there, in the one
        # column that is allowed to hold it.
        text = prototypes.render_hypotheses(
            prototypes.hypotheses(self.ROWS, self.IDA))
        row = next(csv.DictReader(io.StringIO(text)))
        self.assertIsNotNone(
            prototypes.CONVENTION_TOKEN.search(row["evidence"]))

    def test_a_MANGLED_row_never_reaches_the_hypothesis_file(self):
        rows = [{"address": "0x00401520", "name": "?close@Console@@QAEXXZ",
                 "prototype": ""}]
        self.assertEqual([], prototypes.hypotheses(rows, self.IDA))

    def test_a_row_that_already_has_a_prototype_is_not_hypothesised(self):
        rows = [{"address": "0x00401520", "name": "sub_401520",
                 "prototype": "int (__cdecl sub_401520)(int)"}]
        self.assertEqual([], prototypes.hypotheses(rows, self.IDA))

    def test_a_varargs_guess_is_refused(self):
        out = prototypes.hypotheses(
            self.ROWS, [{"address": "0x00401520", "type": "int __cdecl(char *, ...)"}])
        self.assertEqual([], out)

    def test_a_row_IDA_never_saw_is_simply_absent(self):
        self.assertEqual([], prototypes.hypotheses(self.ROWS, []))


class RenderFileTests(unittest.TestCase):

    def test_the_rendered_csv_is_LF_and_starts_with_the_header(self):
        text = prototypes.render([])
        self.assertEqual(",".join(prototypes.FIELDS) + "\n", text)
        self.assertNotIn("\r\n", text)

    def test_rows_are_ordered_by_address(self):
        table = {"?a@C@@QAEXXZ": "public: void __thiscall C::a(void)",
                 "?b@C@@QAEXXZ": "public: void __thiscall C::b(void)"}
        found, _ = prototypes.derive(
            [{"address": "0x00402000", "name": "?b@C@@QAEXXZ", "prototype": ""},
             {"address": "0x00401000", "name": "?a@C@@QAEXXZ", "prototype": ""}],
            fake_demangler(table))
        rows = list(csv.reader(io.StringIO(prototypes.render(found))))
        self.assertEqual(["0x00401000", "0x00402000"],
                         [row[0] for row in rows[1:]])

    def test_a_prototype_containing_a_comma_is_QUOTED(self):
        found, _ = prototypes.derive(
            [{"address": "0x00401000", "name": "?f@C@@QAEXHH@Z",
              "prototype": ""}],
            fake_demangler({"?f@C@@QAEXHH@Z":
                            "public: void __thiscall C::f(int,int)"}))
        self.assertIn('"', prototypes.render(found))


class LoadTests(unittest.TestCase):

    def test_a_missing_catalogue_is_not_an_error(self):
        self.assertEqual({}, prototypes.load(Path("/nonexistent/nope.csv")))


class CatalogueTests(unittest.TestCase):
    """Against the committed files, skipped where they have not been derived."""

    def setUp(self):
        if not prototypes.PROTOTYPES_CSV.is_file():
            self.skipTest("no prototype catalogue derived on this machine")

    def test_the_committed_catalogue_is_LF(self):
        self.assertNotIn(b"\r\n", prototypes.PROTOTYPES_CSV.read_bytes())

    def test_every_committed_prototype_parses_under_the_CONSUMER_regex(self):
        # The consumer moved: `generate_signature_oracles` is retired, and the
        # live reader of this catalogue is emit_translation_unit, which feeds
        # every row through PROTOTYPE_RE and raises Unsettled - no unit, no
        # byte match - on one it cannot read. Asserting against the emitter's
        # regex and not against `prototypes.RECORDED` is deliberate: the
        # emitted control inside the tool already matches its output against
        # RECORDED, so reading it back here would only prove the tool agrees
        # with itself. The two spellings differ (`\S+` vs `[^)]*` for the name
        # slot), which is what makes this a second opinion. Measured today:
        # 1,553 published rows, 0 the emitter cannot parse.
        import emit_translation_unit as emitter
        loaded = prototypes.load()
        self.assertTrue(loaded)
        bad = [address for address, text in loaded.items()
               if not emitter.PROTOTYPE_RE.match(text)]
        self.assertEqual([], bad[:5])

    def test_every_committed_prototype_names_its_own_address_row(self):
        loaded = prototypes.load()
        self.assertTrue(loaded)
        for address, text in loaded.items():
            self.assertTrue(address.startswith("0x"), address)

    def test_the_mangled_name_appears_INSIDE_the_prototype_parens(self):
        shape = re.compile(r"^.+? \(__\w+ (?P<name>\S+)\)\(")
        with prototypes.PROTOTYPES_CSV.open(newline="",
                                            encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                found = shape.match(row["prototype"])
                self.assertIsNotNone(found, row["prototype"])
                self.assertEqual(row["name"], found.group("name"))

    def test_the_catalogue_never_overlaps_a_row_that_already_has_one(self):
        # The two columns must stay disjoint; a derived prototype must never
        # shadow IDA's, which carries parameter names and typedefs. Read the
        # recorded side through `prototypes.load_rows()` - the same call the
        # tool's own `derive` is fed - because functions.csv is gone and the
        # annotations in src/ are the catalogue now. Going straight to a path
        # is what let this test read a file the tool had already stopped
        # reading. The population is asserted first: load_rows() falls back
        # through project_catalogue, and an empty catalogue would make the
        # disjointness below true by having nothing to intersect. Measured
        # today: 3,262 rows carry a recorded prototype, 1,553 are derived, and
        # the two sets share nothing.
        recorded = {row["address"] for row in prototypes.load_rows()
                    if row.get("prototype")}
        self.assertTrue(recorded)
        self.assertEqual(set(), recorded & set(prototypes.load()))

    def test_no_derived_prototype_spells_the_empty_void_parameter_list(self):
        for text in prototypes.load().values():
            self.assertNotIn("(void)", text)


class PurgeCatalogueTests(unittest.TestCase):
    """The gate, replayed against the COMMITTED files and no executable.

    This is the check that keeps the published catalogue honest on a machine
    that cannot read the bytes. It rebuilds the implied purge from the columns
    the file actually publishes - `convention` and `stack_slots` - rather than
    from a `Derived` object, so a bug in the deriving code cannot make this
    agree with itself.
    """

    # The nine addresses an adversarial review found contradicted by the bytes
    # on 2026-07-31. Eight are `QAG` rows that were over-declared by exactly
    # four bytes because the receiver was charged to the stack; they are correct
    # now and must stay published. The ninth declares six arguments and executes
    # `ret 0x10`; it is a real contradiction and must stay refused.
    CORRECTED = ("0x00416580", "0x005D7B40", "0x005D7CA0", "0x005EC690",
                 "0x005EC6F0", "0x005F0650", "0x005F6A50", "0x005F8500")
    REFUSED = "0x0061A760"

    # Three of the 47 `??3<Class>@@SAXPAXI@Z` rows the one hop caught on
    # 2026-07-31. Each is `sub ecx, <imm> ; jmp <??_G scalar deleting
    # destructor>`, the target executes `ret 4`, and each was published as a
    # `__cdecl` taking two stack arguments and purging 0 - so a caller pushed 8
    # and got 4 popped.
    #
    # THE HOP WAS RIGHT AND THE NAME WAS WRONG, which is the resolution these
    # tests now pin instead of the refusal. On 2026-08-13
    # `catalogue_corrections` renamed all 47 to the this-adjusting thunk
    # spellings their bodies carry - `??_GAlphaMovie@@WEEE@AEPAXI@Z`, whose
    # `adjustor{1092}` is the 0x444 the body at 0x00404430 subtracts - so each
    # is now a `__thiscall` taking one `unsigned int`, implying the purge of 4
    # the tail-jump target executes. They are PUBLISHED, and the hop that used
    # to refuse them now confirms them.
    #
    # Asserting the refusal was the right test while the names were wrong: it
    # is what kept the contradiction visible instead of letting 47 rows ship a
    # signature no caller could satisfy. Inverting it now is not weakening the
    # check - the same bytes back both directions, and the row that is STILL
    # refused (`REFUSED`, a different defect) is asserted absent right above.
    CORRECTED_THUNKS = ("0x00404430", "0x004070B0", "0x0062C850")

    # The vtordisp form, the only one of the 47 that is not an 11-byte
    # `sub ecx, imm32` adjustor: 8 bytes, `sub ecx, dword ptr [ecx - 4]`, and
    # `$4PPPPPPPM@A@` demangles to `vtordisp{-4, 0}`. It travels separately
    # because it is the row a grammar that only handles adjustors would drop.
    CORRECTED_VTORDISP = "0x0048BF10"

    def setUp(self):
        if not (prototypes.PROTOTYPES_CSV.is_file()
                and prototypes.PURGE_CSV.is_file()):
            self.skipTest("no catalogue derived on this machine")
        with prototypes.PROTOTYPES_CSV.open(newline="",
                                            encoding="utf-8-sig") as handle:
            self.published = {row["address"]: row
                              for row in csv.DictReader(handle)}
        self.purges = prototypes.load_purges()

    @staticmethod
    def implied(row):
        """The callee purge the PUBLISHED columns imply, or None."""
        slots = sum(int(digit) for digit in row["stack_slots"])
        if row["convention"] == "__cdecl":
            return 0
        if row["convention"] in ("__thiscall", "__stdcall"):
            return 4 * slots
        return None

    def test_the_purge_catalogue_covers_every_published_row(self):
        missing = [a for a in self.published if a not in self.purges]
        self.assertEqual([], missing[:5])

    def test_no_published_row_is_contradicted_by_the_bytes(self):
        bad = []
        for address, row in self.published.items():
            observed = self.purges.get(address, prototypes.NO_PURGE).observed
            implied = self.implied(row)
            if observed is not None and implied is not None \
                    and observed != implied:
                bad.append(f"{address} implies {implied}, body pops {observed}")
        self.assertEqual([], bad[:5])

    def test_the_purge_column_says_verified_only_where_the_bytes_spoke(self):
        expected = {prototypes.SOURCE_BODY: prototypes.VERIFIED,
                    prototypes.SOURCE_TAIL_JUMP:
                        prototypes.VERIFIED_VIA_TAIL_JUMP}
        for address, row in self.published.items():
            purge = self.purges.get(address, prototypes.NO_PURGE)
            spoke = purge.observed is not None and self.implied(row) is not None
            self.assertEqual(
                expected[purge.source] if spoke else prototypes.UNKNOWN,
                row["purge"], address)

    def test_the_purge_column_only_ever_holds_one_of_the_three_words(self):
        self.assertEqual({prototypes.VERIFIED, prototypes.UNKNOWN,
                          prototypes.VERIFIED_VIA_TAIL_JUMP},
                         {row["purge"] for row in self.published.values()})

    def test_a_HOPPED_row_never_reads_as_a_direct_observation(self):
        # The failure this column exists to prevent: a number that took an
        # indirection published under the same word as one read off the body.
        for address, row in self.published.items():
            if self.purges.get(address, prototypes.NO_PURGE).source \
                    == prototypes.SOURCE_TAIL_JUMP:
                self.assertNotEqual(prototypes.VERIFIED, row["purge"], address)

    def test_every_hopped_row_says_so_in_BOTH_files(self):
        # The `source` column and the `purge` word cannot drift apart.
        for address, row in self.published.items():
            hopped_here = row["purge"] == prototypes.VERIFIED_VIA_TAIL_JUMP
            source = self.purges.get(address, prototypes.NO_PURGE).source
            self.assertEqual(hopped_here,
                             source == prototypes.SOURCE_TAIL_JUMP, address)

    def test_the_hop_reaches_a_real_share_of_the_catalogue(self):
        # Below the pinned floor the hop is not worth its indirection; this is
        # the committed-file replay of the check `main` runs against the bytes.
        _agreeing, sample = prototypes.HOP_AGREEMENT_FLOOR
        reached = sum(1 for row in self.published.values()
                      if row["purge"] == prototypes.VERIFIED_VIA_TAIL_JUMP)
        self.assertGreaterEqual(reached, sample * 0.5)

    def test_the_eight_QAG_rows_are_published_AND_purge_verified(self):
        for address in self.CORRECTED:
            row = self.published.get(address)
            self.assertIsNotNone(row, f"{address} vanished from the catalogue")
            self.assertEqual("__stdcall", row["convention"], address)
            self.assertEqual(prototypes.VERIFIED, row["purge"], address)

    def test_the_row_the_bytes_contradict_is_ABSENT(self):
        self.assertNotIn(self.REFUSED, self.published)

    def test_that_refused_row_is_still_in_the_purge_catalogue(self):
        # A refusal has to leave a trace, or the next reader cannot tell a row
        # that was rejected from a row nobody ever looked at.
        self.assertIn(self.REFUSED, self.purges)
        self.assertEqual(16, self.purges[self.REFUSED].observed)

    def test_the_adjustor_thunks_the_HOP_caught_are_PUBLISHED(self):
        # Under the corrected name each derives the signature the bytes show:
        # a `__thiscall` receiving `this` in ECX and one `unsigned int` on the
        # stack, which implies the purge of 4 the tail-jump target executes.
        for address in self.CORRECTED_THUNKS + (self.CORRECTED_VTORDISP,):
            row = self.published.get(address)
            self.assertIsNotNone(row, f"{address} is not published")
            self.assertEqual("__thiscall", row["convention"], address)
            self.assertEqual("1", row["stack_slots"], address)
            self.assertEqual(prototypes.VERIFIED_VIA_TAIL_JUMP,
                             row["purge"], address)
            self.assertTrue(row["name"].startswith("??_G"), row["name"])

    def test_each_of_those_is_still_in_the_catalogue_with_its_source(self):
        for address in self.CORRECTED_THUNKS + (self.CORRECTED_VTORDISP,):
            purge = self.purges.get(address)
            self.assertIsNotNone(purge, address)
            self.assertEqual(4, purge.observed, address)
            self.assertEqual(prototypes.SOURCE_TAIL_JUMP, purge.source, address)

    def test_the_corrected_thunk_names_carry_their_adjustment(self):
        # The name is only right if it round-trips to the constant the body
        # subtracts, so the published spelling is held to it rather than to a
        # prefix. 0x444 == 1092 for every adjustor; the vtordisp spells the
        # field offset it loads through instead.
        for address in self.CORRECTED_THUNKS:
            name = self.published[address]["name"]
            self.assertIn("@@WEEE@AEPAXI@Z", name, address)
        self.assertIn("@@$4PPPPPPPM@A@AEPAXI@Z",
                      self.published[self.CORRECTED_VTORDISP]["name"])

    def test_the_hop_now_contradicts_NOTHING_it_reaches(self):
        # The 47 were the whole of the hop's disagreement. With their names
        # corrected the floor is total agreement, so any hopped row whose
        # published prototype implies a different purge is a NEW finding.
        agreeing, sample = prototypes.HOP_AGREEMENT_FLOOR
        self.assertEqual(agreeing, sample)

    def test_EVERY_static_kind_row_the_hop_reached_is_gone(self):
        # 47 of 47, none spared - the blind spot an earlier review named. Any
        # `??3<Class>@@SAXPAXI@Z` still published would be one of them.
        #
        # This survives the correction unchanged, and for a better reason than
        # before: the spelling is not refused now, it no longer EXISTS. A row
        # reappearing under `??3` would mean `catalogue_corrections` stopped
        # being applied - the exact silent-off failure that channel already had
        # once, when its only call sat inside a branch that could not be taken.
        still_here = [address for address, row in self.published.items()
                      if row["name"].startswith("??3")]
        self.assertEqual([], still_here)

    def test_the_committed_purge_catalogue_is_LF(self):
        self.assertNotIn(b"\r\n", prototypes.PURGE_CSV.read_bytes())

    def test_the_purge_catalogue_header_is_the_declared_one(self):
        with prototypes.PURGE_CSV.open(newline="",
                                       encoding="utf-8-sig") as handle:
            self.assertEqual(list(prototypes.PURGE_FIELDS),
                             next(csv.reader(handle)))

    def test_the_purge_catalogue_carries_no_disassembly(self):
        # Addresses and integers only; the executable's bytes never reach a
        # committed file. The hop widened the vocabulary of this column, so the
        # allow-list is widened with it rather than loosened into a wildcard -
        # and deliberately still admits no mnemonic and no byte.
        allowed = re.compile(
            r"^(one reachable ret imm|\d+ distinct reachable ret imm"
            r"|the only ret imm|no body_ranges"
            r"|no ret and no direct tail jump|a direct tail jump)")
        forbidden = re.compile(r"\b(mov|jmp|sub|push|pop|lea|ecx|eax|esp)\b")
        with prototypes.PURGE_CSV.open(newline="",
                                       encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                self.assertRegex(row["evidence"], allowed)
                self.assertNotRegex(row["evidence"], forbidden)

    def test_a_ret_the_entry_cannot_reach_is_not_evidence(self):
        # The defect this pins: a linear sweep cannot tell a live `ret` from a
        # dead one. `?_JumpToContinuation` ends at `jmp eax` and the epilogue
        # after it is unreachable; `?terminate`'s only `ret` is in an EH
        # funclet. Both numbers happened to be right, which is exactly why
        # calling them "verified" was an overstatement worth refusing.
        with prototypes.PURGE_CSV.open(newline="",
                                       encoding="utf-8-sig") as handle:
            rows = {row["address"]: row for row in csv.DictReader(handle)}
        for address in ("0x00644F45", "0x006492CA"):
            self.assertEqual(prototypes.SOURCE_NONE, rows[address]["source"],
                             address)
            self.assertIn("not reachable from the entry",
                          rows[address]["evidence"], address)

    def test_discarding_a_dead_ret_can_RESOLVE_an_ambiguous_body(self):
        # The same filter that withdraws a claim also earns one. __ArrayUnwind
        # carries two `ret`s - a bare one in its EH unwind funclet and `ret 0x10`
        # on the reachable path - and two distinct immediates used to read as
        # ambiguous. Dropping the unreachable one leaves a single answer.
        with prototypes.PURGE_CSV.open(newline="",
                                       encoding="utf-8-sig") as handle:
            rows = {row["address"]: row for row in csv.DictReader(handle)}
        row = rows["0x00645764"]
        self.assertEqual(prototypes.SOURCE_BODY, row["source"])
        self.assertEqual("0x10", row["observed"] if "observed" in row
                         else row.get("purge", "0x10"))

    def test_reachable_addresses_stops_at_ret_and_at_an_indirect_jump(self):
        # Duck-typed instructions, so this runs where capstone is absent - the
        # same reason the module imports it lazily.
        class Fake:
            def __init__(self, address, size, mnemonic, op_str=""):
                self.address, self.size = address, size
                self.mnemonic, self.op_str = mnemonic, op_str

        # entry -> jmp eax ; then a dead epilogue that a sweep would believe.
        body = [Fake(0x1000, 2, "xor", "eax, eax"),
                Fake(0x1002, 2, "jmp", "eax"),
                Fake(0x1004, 1, "pop", "edi"),
                Fake(0x1005, 3, "ret", "8")]
        reached = prototypes.reachable_addresses(body, 0x1000)
        self.assertEqual({0x1000, 0x1002}, reached)
        self.assertEqual(set(), prototypes.ret_immediates(
            [one for one in body if one.address in reached]))

        # A conditional branch must contribute BOTH edges, or the filter would
        # withhold "verified" from ordinary branching code.
        both = [Fake(0x2000, 2, "je", "0x2006"),
                Fake(0x2002, 3, "mov", "eax, 1"),
                Fake(0x2005, 1, "ret"),
                Fake(0x2006, 3, "ret", "4")]
        reached = prototypes.reachable_addresses(both, 0x2000)
        self.assertEqual({0x2000, 0x2002, 0x2005, 0x2006}, reached)

    def test_the_purge_catalogue_source_column_holds_only_declared_words(self):
        with prototypes.PURGE_CSV.open(newline="",
                                       encoding="utf-8-sig") as handle:
            sources = {row["source"] for row in csv.DictReader(handle)}
        self.assertEqual(set(), sources - {prototypes.SOURCE_BODY,
                                           prototypes.SOURCE_TAIL_JUMP,
                                           prototypes.SOURCE_NONE})

    def test_a_row_with_a_NUMBER_always_names_where_it_came_from(self):
        with prototypes.PURGE_CSV.open(newline="",
                                       encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                if row["purge"] != prototypes.UNKNOWN:
                    self.assertIn(row["source"], (prototypes.SOURCE_BODY,
                                                  prototypes.SOURCE_TAIL_JUMP),
                                  row["address"])


class HypothesisCatalogueTests(unittest.TestCase):

    def setUp(self):
        if not prototypes.HYPOTHESES_CSV.is_file():
            self.skipTest("no hypothesis catalogue derived on this machine")

    def test_the_committed_hypotheses_are_LF(self):
        self.assertNotIn(b"\r\n", prototypes.HYPOTHESES_CSV.read_bytes())

    def test_the_hypotheses_file_never_names_a_MANGLED_row(self):
        with prototypes.HYPOTHESES_CSV.open(newline="",
                                            encoding="utf-8-sig") as handle:
            for row in csv.DictReader(handle):
                self.assertFalse(row["name"].startswith("?"), row["name"])

    def test_the_two_catalogues_share_no_address(self):
        with prototypes.HYPOTHESES_CSV.open(newline="",
                                            encoding="utf-8-sig") as handle:
            weak = {row["address"] for row in csv.DictReader(handle)}
        self.assertEqual(set(), weak & set(prototypes.load()))

    def test_the_hypotheses_header_is_the_declared_one(self):
        with prototypes.HYPOTHESES_CSV.open(newline="",
                                            encoding="utf-8-sig") as handle:
            self.assertEqual(list(prototypes.HYPOTHESIS_FIELDS),
                             next(csv.reader(handle)))




class ThunkAdjustmentGateTests(unittest.TestCase):
    """The half of a thunk name the callee-purge gate cannot falsify.

    `??_GAlphaMovie@@WEEE@AEPAXI@Z` asserts a signature AND a displacement.
    Corrupt the signature and the purge check fires; corrupt the displacement
    from 1092 to 1093 and every published number stays green while the
    catalogue carries a name for a thunk that does not exist. Measured on
    2026-08-13 by doing exactly that to the live correction: the gate went from
    exit 0 to exit 1 with `the name states [1093] and the body applies [1092]`,
    and no floor moved in either run. These are the offline half of that.
    """

    class Fake:
        """The two capstone shapes MSVC emits, with the detail the gate reads."""

        def __init__(self, mnemonic, operands):
            self.mnemonic, self.operands = mnemonic, operands

        @staticmethod
        def reg_name(reg):
            return reg

    class Op:
        def __init__(self, kind, imm=0, reg="", mem=None):
            from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG
            self.type = {"imm": X86_OP_IMM, "reg": X86_OP_REG,
                         "mem": X86_OP_MEM}[kind]
            self.imm, self.reg, self.mem = imm, reg, mem

    class Mem:
        def __init__(self, base, disp, index=0):
            self.base, self.disp, self.index = base, disp, index

    def sub_ecx_imm(self, value):
        return self.Fake("sub", [self.Op("reg", reg="ecx"),
                                 self.Op("imm", imm=value)])

    def sub_ecx_mem(self, disp):
        return self.Fake("sub", [self.Op("reg", reg="ecx"),
                                 self.Op("mem", mem=self.Mem("ecx", disp))])

    def test_an_adjustor_body_reports_the_constant_it_subtracts(self):
        self.assertEqual([0x444], prototypes._applied_adjustment(
            [self.sub_ecx_imm(0x444)]))

    def test_a_vtordisp_body_reports_the_field_and_a_zero_adjustment(self):
        # `sub ecx, [ecx - 4]` with no second `sub` is vtordisp{-4, 0}: the
        # zero is the ABSENCE of a second instruction, not a default.
        self.assertEqual([-4, 0], prototypes._applied_adjustment(
            [self.sub_ecx_mem(-4)]))

    def test_a_vtordisp_with_a_second_subtraction_reports_both(self):
        self.assertEqual([-4, 8], prototypes._applied_adjustment(
            [self.sub_ecx_mem(-4), self.sub_ecx_imm(8)]))

    def test_a_body_that_does_not_touch_ecx_reports_NOTHING(self):
        # Empty, so it can never equal a name's adjustment. A thunk-named
        # function whose body adjusts nothing must be reported, not excused.
        self.assertEqual([], prototypes._applied_adjustment([]))
        self.assertEqual([], prototypes._applied_adjustment(
            [self.Fake("push", [self.Op("reg", reg="esi")])]))
        self.assertEqual([], prototypes._applied_adjustment(
            [self.Fake("sub", [self.Op("reg", reg="eax"),
                               self.Op("imm", imm=4)])]))

    def test_the_two_spellings_of_a_negative_displacement_reconcile(self):
        # The mangled number carries no sign, so `PPPPPPPM@` decodes to
        # 0xFFFFFFFC while capstone reads -4 off the same four bytes. Both are
        # the same displacement and the gate must not call that a disagreement.
        self.assertEqual(prototypes._as_displacements([0xFFFFFFFC, 0]),
                         prototypes._as_displacements([-4, 0]))

    def test_but_a_real_difference_still_survives_that_reconciliation(self):
        # The failure mode of a normalisation is that it makes everything
        # equal. 1092 and 1093 must stay different, and so must 0x444 and a
        # negative number that happens to share its low bits.
        self.assertNotEqual(prototypes._as_displacements([1092]),
                            prototypes._as_displacements([1093]))
        self.assertNotEqual(prototypes._as_displacements([0xFFFFFFFC]),
                            prototypes._as_displacements([0xFFFFFFFC - 1]))

    def test_is_thunk_name_selects_the_thunk_spellings_and_nothing_else(self):
        self.assertEqual("W", prototypes.is_thunk_name(
            "??_GAlphaMovie@@WEEE@AEPAXI@Z"))
        self.assertEqual("$4", prototypes.is_thunk_name(
            "??_GPlanWin@@$4PPPPPPPM@A@AEPAXI@Z"))
        for name in ("??_GAlphaMovie@@UAEPAXI@Z", "??3AlphaMovie@@SAXPAXI@Z",
                     "?f@C@@QAEXXZ", "?f@@YAXXZ", "not_mangled"):
            self.assertEqual("", prototypes.is_thunk_name(name), name)


if __name__ == "__main__":
    unittest.main()
