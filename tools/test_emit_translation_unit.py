#!/usr/bin/env python3
"""Pin the scaffolding emitter, especially the parts that were wrong.

This tool went from a 38% to a 98% coverage rate over the fan-out population,
and every step of that was a defect rather than a feature. Each is pinned here
so it cannot come back:

  * forward declarations were scoped to the SUBJECT's signature, so a callee
    taking `RECT *` was declared before anything declared `RECT` (38% -> 52%);
  * the skeleton had no `return`, so VC6 raised C4716 on every non-void subject
    and "did it compile" stopped being a usable signal for the scaffolding;
  * the repair loop's C2146 pattern was pinned to `missing ';'` when CL says
    `missing ','` in a parameter list, so it matched nothing and the compile
    rate came back BYTE-IDENTICAL to the run before it - which is how the dead
    code path was found;
  * a mangled name that is not a C identifier was REFUSED, costing 60+ rows,
    when the emitted name cannot affect the comparison at all because nothing
    is ever linked;
  * rows with no prototype anywhere were refused, when 638 of the 679 carry a
    single consistent `ret N` in the body - the arity ground truth.

The compiler is not needed for any of this and CI does not have it.
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import emit_translation_unit as tool  # noqa: E402


def row(address="0x00401000", name="?f@@YAHH@Z", prototype="", size="16",
        body_ranges=""):
    return {"address": address, "name": name, "prototype": prototype,
            "size": size, "body_ranges": body_ranges or f"{address}-0x00401010",
            "recovery_state": "unrecovered"}


class SignatureTests(unittest.TestCase):
    def test_a_thiscall_becomes_a_member_function(self):
        """VC6 rejects the `__thiscall` keyword on a free function outright,
        and 1,708 of 3,063 remaining functions are __thiscall."""
        signature = tool.Signature(
            row(name="?add@StringStruct@@QAEHH@Z",
                prototype="int (__thiscall ?add@StringStruct@@QAEHH@Z)"
                          "(StringStruct* this, int)"), {})
        self.assertTrue(signature.is_method)
        self.assertEqual(signature.klass, "StringStruct")
        self.assertEqual(signature.method, "add")
        self.assertEqual(signature.params, ["int"],
                         "`this` must not survive into the parameter list")

    def test_a_constructor_and_destructor_are_recognised(self):
        constructor = tool.Signature(
            row(name="??0StringStruct@@QAE@H@Z",
                prototype="void (__thiscall ??0StringStruct@@QAE@H@Z)"
                          "(StringStruct* this, int)"), {})
        self.assertEqual(constructor.kind, "ctor")
        destructor = tool.Signature(
            row(name="??1StringStruct@@QAE@XZ",
                prototype="void (__thiscall ??1StringStruct@@QAE@XZ)"
                          "(StringStruct* this)"), {})
        self.assertEqual(destructor.kind, "dtor")
        self.assertEqual(destructor.method, "~StringStruct")

    def test_a_cdecl_free_function_keeps_its_convention(self):
        signature = tool.Signature(
            row(name="?bitmask@@YAXHPAHPAH@Z",
                prototype="void (__cdecl ?bitmask@@YAXHPAHPAH@Z)"
                          "(int, int *, int *)"), {})
        self.assertFalse(signature.is_method)
        self.assertEqual(signature.convention, "__cdecl")
        self.assertEqual(signature.method, "bitmask")

    def test_an_unspellable_name_is_renamed_not_refused(self):
        """`??__Eg_FOO@@YAXXZ` is not a C identifier. Refusing cost 60+ rows.
        Nothing is ever LINKED, so the emitted spelling reaches no
        comparison - only the convention, parameters and return type do."""
        signature = tool.Signature(
            row(address="0x00402F10", name="??__Eg_ALPHAMENU_WAVE@@YAXXZ",
                prototype="void (__cdecl ??__Eg_ALPHAMENU_WAVE@@YAXXZ)()"), {})
        self.assertEqual(signature.method, "fn_00402f10")

    def test_a_prototype_that_does_not_parse_is_refused_not_guessed(self):
        with self.assertRaises(tool.Unsettled):
            tool.Signature(row(prototype="this is not a prototype"), {})

    def test_no_prototype_anywhere_is_refused_without_the_binary(self):
        # A DISASSEMBLER LABEL, which states nothing about the signature. A
        # mangled name states all of it, and is read instead of refused.
        with self.assertRaises(tool.Unsettled):
            tool.Signature(row(name="sub_401000", prototype=""), {})

    def test_a_mangled_name_is_read_when_there_is_no_prototype(self):
        # 48 rows are in this state. The name is what the LINKER wrote from
        # the real declaration; the fallback it replaces could only offer N
        # ints guessed from a `ret`, and cannot tell __stdcall from
        # __thiscall at all because the two write the purge byte identically.
        signature = tool.Signature(
            row(name="?draw@Texture@@QAEXPAUVert@@H@Z", prototype=""), {})
        self.assertEqual("__thiscall", signature.convention)
        self.assertEqual(["Vert *", "int"], signature.params)
        self.assertEqual("Texture", signature.klass)
        self.assertTrue(signature.inferred)

    def test_a_name_carrying_a_type_that_does_not_decode_is_refused(self):
        # A POINTER TO MEMBER (`P8C@@AEXXZ`), which is still out of scope.
        # A function pointer used to be the example here and is not any more.
        with self.assertRaises(tool.Unsettled):
            tool.Signature(row(name="?f@@YAXP8C@@AEXXZ@Z", prototype=""), {})


class LinkageTests(unittest.TestCase):
    """Which linkage the subject gets, which decides the emitted SYMBOL.

    objdiff looks one name up in both objects. A `?`-mangled row reproduces its
    catalogued name through the C++ compiler, so it needs nothing. A row a
    disassembler named has no such loop: emitted with C++ linkage it becomes
    `?sub_5e3650@@YGHH@Z`, which no target object holds, and the run reports a
    missing symbol rather than a diff. That was the state of 1,179 rows.
    """

    def test_a_mangled_row_keeps_cxx_linkage_and_its_own_name(self):
        signature = tool.Signature(
            row(name="?bitmask@@YAXHPAHPAH@Z",
                prototype="void (__cdecl ?bitmask@@YAXHPAHPAH@Z)"
                          "(int, int *, int *)"), {})
        self.assertEqual("c++", signature.linkage)
        # Compressed: CL never spells the same composite argument type twice,
        # so the second `PAH` is written `0`. The catalogued name is the
        # uncompressed form, which the compiler will never emit.
        self.assertEqual("?bitmask@@YAXHPAH0@Z", signature.symbol)

    def test_a_disassembler_label_gets_c_linkage_and_a_decorated_symbol(self):
        signature = tool.Signature(
            row(address="0x005E3650", name="sub_5e3650",
                prototype="int (__stdcall sub_5e3650)(int, int)"), {})
        self.assertEqual("c", signature.linkage)
        self.assertEqual("_sub_5e3650@8", signature.symbol)

    def test_a_decorated_name_reproduces_itself_through_the_source(self):
        signature = tool.Signature(
            row(address="0x0045F950", name="_WinMain@16",
                prototype="int (__stdcall _WinMain@16)"
                          "(int, int, char *, int)"), {})
        self.assertEqual("WinMain", signature.method,
                         "the source spells the UNDECORATED identifier")
        self.assertEqual("_WinMain@16", signature.symbol)

    def test_the_decoration_outranks_an_inferred_convention(self):
        # `_WinMain@16` says __stdcall outright. A prototype read out of a
        # purge byte cannot tell __stdcall from __thiscall, so the name wins.
        signature = tool.Signature(
            row(address="0x0045F950", name="_WinMain@16",
                prototype="int (__cdecl _WinMain@16)(int, int, char *, int)"),
            {})
        self.assertEqual("__stdcall", signature.convention)

    def test_a_thiscall_under_an_undecorated_name_is_refused(self):
        # The unit would be emitted as a method of a class this tool invented,
        # and CL would mangle that class into a symbol no target object can
        # hold. Refusing says so; emitting produces an unmatchable prompt.
        with self.assertRaises(tool.Unsettled):
            tool.Signature(
                row(address="0x004483C0", name="j_??1Ambience@@QAE@XZ",
                    prototype="void (__thiscall j_??1Ambience@@QAE@XZ)"
                              "(Ambience* this)"), {})


class MangledTypeTests(unittest.TestCase):
    """The mangled name outranks the prototype's SPELLING of a type.

    `derive_prototypes_from_names` writes IDA's alphabet on purpose, and that
    alphabet collapses types MSVC keeps apart: `char` and `signed char` both
    become `int8`, `long` and `int` both become `int`. They mangle differently
    - D vs C, J vs H - so a body compiled from the IDA spelling emits a symbol
    no target object holds. 108 of 264 unpairable rows were only this.
    """

    def test_int8_becomes_char_when_the_name_says_D(self):
        signature = tool.Signature(
            row(name="?amovie_project@@YAXPAD@Z",
                prototype="void (__cdecl ?amovie_project@@YAXPAD@Z)(int8*)"),
            {})
        self.assertEqual(["char *"], signature.params)

    def test_int_becomes_unsigned_long_when_the_name_says_K(self):
        signature = tool.Signature(
            row(name="?create_device@Midi_Device@@QAEHK@Z",
                prototype="int (__thiscall ?create_device@Midi_Device@@QAEHK@Z)"
                          "(Midi_Device* this, unsigned int)"), {})
        self.assertEqual(["unsigned long"], signature.params)

    def test_a_disagreeing_arity_leaves_the_prototype_alone(self):
        # `decode_signature` returns None on a user-defined type, and a count
        # that differs means it read a different signature rather than a better
        # spelling of the same one.
        signature = tool.Signature(
            row(name="?f@@YAXPAD@Z",
                prototype="void (__cdecl ?f@@YAXPAD@Z)(int8*, int)"), {})
        self.assertEqual(["int8*", "int"], signature.params)


class MemberFunctionTests(unittest.TestCase):
    def test_a_cdecl_member_is_a_member(self):
        """`QAA` is a public __cdecl MEMBER, and 80 of them are catalogued.

        Keying `is_method` off __thiscall alone sent every one down the
        free-function path, where the mangled name is not an identifier, so it
        was renamed `fn_<address>` and emitted as a free function."""
        signature = tool.Signature(
            row(address="0x00403AF0",
                name="?fill_func@AlphaMenu@@QAAHPAUGraphicWin@@HPAURECT@@@Z",
                prototype="int (__cdecl ?fill_func@AlphaMenu@@QAAHPAUGraphic"
                          "Win@@HPAURECT@@@Z)(AlphaMenu* this, GraphicWin*, "
                          "int, RECT*)"), {})
        self.assertTrue(signature.is_method)
        self.assertEqual("AlphaMenu", signature.klass)
        self.assertEqual("fill_func", signature.method)
        self.assertEqual("__cdecl ", signature.member_convention(),
                         "__cdecl on a member is not the default and has to "
                         "be spelled, or CL mangles QAE where the target "
                         "holds QAA")

    def test_thiscall_never_spells_its_convention(self):
        # VC6 rejects the keyword outright (C4234) and it is the default.
        signature = tool.Signature(
            row(name="?add@StringStruct@@QAEHH@Z",
                prototype="int (__thiscall ?add@StringStruct@@QAEHH@Z)"
                          "(StringStruct* this, int)"), {})
        self.assertEqual("", signature.member_convention())

    def test_a_free_function_with_a_parameter_named_this_stays_free(self):
        # 93 of them exist. Both conditions are required: a class in the
        # mangled name AND a receiver-looking first parameter.
        signature = tool.Signature(
            row(name="?bitmask@@YAXPAH@Z",
                prototype="void (__cdecl ?bitmask@@YAXPAH@Z)(int* this)"), {})
        self.assertFalse(signature.is_method)


class CalleeDeclarationTests(unittest.TestCase):
    def test_an_undecorated_callee_is_declared_extern_c(self):
        # Without it the call site references a C++ mangling of the name while
        # the target object's relocation carries the C decoration, so every
        # call to one of these reads as a diff in an otherwise exact body.
        text = tool.declare_callee(
            row(address="0x005E3650", name="sub_5e3650",
                prototype="int (__stdcall sub_5e3650)(int)"), {})
        self.assertTrue(text.startswith('extern "C" '), text)

    def test_a_mangled_callee_is_not_declared_extern_c(self):
        text = tool.declare_callee(
            row(name="?bitmask@@YAXHPAHPAH@Z",
                prototype="void (__cdecl ?bitmask@@YAXHPAHPAH@Z)(int)"), {})
        self.assertFalse(text.startswith('extern "C" '), text)

    def test_a_name_cl_declares_itself_is_left_alone(self):
        # A second C-linkage declaration of `atexit` is C2733, and it took the
        # whole unit down with it - 9 units, measured.
        self.assertEqual("", tool.declare_callee(
            row(address="0x00645398", name="_atexit",
                prototype="int (__cdecl _atexit)(int)"), {}))


class SplitParamsTests(unittest.TestCase):
    def test_it_splits_at_top_level_only(self):
        self.assertEqual(tool.split_params("int, char *"), ["int", "char *"])
        self.assertEqual(tool.split_params("Map<int,int>, int"),
                         ["Map<int,int>", "int"])
        self.assertEqual(tool.split_params(""), [])


class RepairPatternTests(unittest.TestCase):
    """The regexes must match what CL ACTUALLY prints, verbatim."""

    def test_it_matches_the_comma_wording_a_parameter_list_produces(self):
        text = ("t.cpp(56) : error C2146: syntax error : "
                "missing ',' before identifier 'a1'")
        self.assertEqual(tool.MISSING_SEMICOLON.findall(text),
                         [("56", "a1")],
                         "pinned to ';' this matched nothing and the repair "
                         "silently did nothing")

    def test_it_still_matches_the_semicolon_wording(self):
        text = ("t.cpp(12) : error C2146: syntax error : "
                "missing ';' before identifier 'bar'")
        self.assertEqual(tool.MISSING_SEMICOLON.findall(text), [("12", "bar")])

    def test_it_reads_the_type_named_by_the_direct_diagnostics(self):
        c2061 = "t.cpp(24) : error C2061: syntax error : identifier 'RECT'"
        c2065 = "t.cpp(9) : error C2065: 'Foo' : undeclared identifier"
        self.assertEqual(tool.UNDECLARED[0].findall(c2061), ["RECT"])
        self.assertEqual(tool.UNDECLARED[1].findall(c2065), ["Foo"])

    def test_repair_stops_when_the_compiler_stops_complaining(self):
        calls = []

        def compile_once(text):
            calls.append(text)
            return None
        text, added, reason = tool.repair("unit", compile_once)
        self.assertEqual(added, [])
        self.assertEqual(reason, "")
        self.assertEqual(len(calls), 1)

    def test_repair_declares_what_the_compiler_named(self):
        seen = []

        def compile_once(text):
            seen.append(text)
            if len(seen) == 1:
                return ("t.cpp(3) : error C2061: syntax error : "
                        "identifier 'RECT'")
            return None
        text, added, reason = tool.repair(tool.PRELUDE + "\nbody\n",
                                          compile_once)
        self.assertEqual(added, ["RECT"])
        self.assertIn("struct RECT;", text)
        self.assertEqual(reason, "")

    def test_repair_gives_up_loudly_rather_than_looping(self):
        def compile_once(text):
            return "t.cpp(3) : error C2061: syntax error : identifier 'A'"
        _, _, reason = tool.repair(tool.PRELUDE, compile_once, rounds=2)
        self.assertTrue(reason, "an unfixable unit must say so, not pass")


class VtableShimTests(unittest.TestCase):
    """827 targets (33% of the remaining mass) dispatch indirectly, and VC6
    rejects a free `__thiscall` function pointer, so the only way to emit
    `call [reg+N*4]` is to call the Nth virtual of a shim class."""

    def test_the_requested_slot_lands_at_the_right_index(self):
        text = tool.vtable_shim([3])
        virtuals = [line for line in text.splitlines()
                    if line.strip().startswith("virtual")]
        self.assertEqual(len(virtuals), 4, "slots 0..3 inclusive")
        self.assertIn("slot003", virtuals[3])
        self.assertIn("<-- used", virtuals[3])

    def test_filler_slots_are_present_but_unmarked(self):
        text = tool.vtable_shim([2])
        self.assertIn("slot000", text)
        self.assertNotIn("slot000();  // <-- used", text)

    def test_two_slots_both_land_and_the_class_reaches_the_higher(self):
        text = tool.vtable_shim([1, 5])
        virtuals = [line for line in text.splitlines()
                    if line.strip().startswith("virtual")]
        self.assertEqual(len(virtuals), 6)
        self.assertIn("<-- used", virtuals[1])
        self.assertIn("<-- used", virtuals[5])

    def test_no_slots_emits_nothing(self):
        self.assertEqual(tool.vtable_shim([]), "")

    def test_the_cap_is_high_enough_to_be_worth_having(self):
        """Measured: median highest slot 37, p99 93, and a cap of 128 covers
        804 of the 806 readable bodies. A cap below ~100 would start
        excluding real vtables."""
        self.assertGreaterEqual(tool.VTABLE_SLOT_CAP, 100)


class NamedTypeTests(unittest.TestCase):
    def test_builtins_are_never_forward_declared(self):
        self.assertEqual(tool.named_types("unsigned int"), set())
        self.assertEqual(tool.named_types("const char *"), set())

    def test_a_game_type_is_picked_up(self):
        self.assertEqual(tool.named_types("Buffer *"), {"Buffer"})

    def test_keywords_are_never_declared_as_types(self):
        """Forward-declaring a keyword turns a C2146 into a C2059 and moves
        the failure without fixing it."""
        for word in ("return", "static", "const", "public"):
            self.assertIn(word, tool.KEYWORDS | tool.BUILTIN)


class ScaffoldCompilesTests(unittest.TestCase):
    """The four reasons 154 emitted units would not compile at all.

    Each was read off CL rather than guessed: the units were recompiled one
    at a time and the first error of each grouped by code.
    """

    def test_a_function_pointer_parameter_is_named_inside_its_declarator(self):
        # `int (__cdecl *)() a2` is C2146. 45 units, one per such parameter.
        self.assertEqual("int (__cdecl *a2)()",
                         tool.named_parameter("int (__cdecl *)()", "a2"))
        self.assertEqual("void (__cdecl *a1)(int8*)",
                         tool.named_parameter("void (__cdecl *)(int8*)", "a1"))

    def test_a_pointer_to_a_function_pointer_too(self):
        self.assertEqual("void (__stdcall * *a1)(int)",
                         tool.named_parameter("void (__stdcall * *)(int)",
                                              "a1"))

    def test_an_ordinary_parameter_takes_the_name_after_it(self):
        for text in ("int", "Win*", "char *", "const char *"):
            self.assertEqual(f"{text} a1", tool.named_parameter(text, "a1"))

    def test_an_array_declarator_is_handled_the_same_way(self):
        self.assertEqual("int (*a1)[10]",
                         tool.named_parameter("int (*)[10]", "a1"))

    def test_a_win32_scalar_is_a_typedef_and_never_a_struct(self):
        # `struct HRESULT;` then used by value is C2027, or C2526 when it is
        # a C-linkage return type.
        for name in ("HRESULT", "LPSTR", "LPCSTR", "CHAR", "BOOL", "DWORD"):
            self.assertIn(name, tool.NOT_A_STRUCT, name)

    def test_a_win32_struct_is_left_to_be_forward_declared(self):
        # These DO appear inside catalogued mangled names - `PAURECT@@` 92
        # times - so a typedef would change the symbol.
        for name in ("RECT", "FILE", "_GUID", "PALETTEENTRY", "WINDOWPOS",
                     "RGBQUAD", "DPNAME"):
            self.assertNotIn(name, tool.NOT_A_STRUCT, name)

    def test_size_t_is_never_declared(self):
        # CL declares it itself; `struct size_t;` is C2371, 18 units.
        self.assertIn("size_t", tool.BUILTIN)

    def test_a_one_argument_thiscall_with_no_class_is_fastcall(self):
        # IDA means "the single argument is the register one". VC6 rejects
        # __thiscall on a free function (C4234); __fastcall with one argument
        # uses the same register and cleans the same empty stack, and the two
        # compile to identical bytes.
        signature = tool.Signature(
            row(address="0x005CB050", name="sub_5cb050",
                prototype="int (__thiscall sub_5CB050)(LPSTR pszFileName)"),
            {})
        self.assertEqual("__fastcall", signature.convention)
        self.assertFalse(signature.is_method)
        self.assertEqual("@sub_5cb050@4", signature.symbol)

    def test_two_arguments_is_still_refused(self):
        # __thiscall puts the second on the stack and __fastcall puts it in
        # EDX, so they stop being the same function.
        with self.assertRaises(tool.Unsettled):
            tool.Signature(
                row(name="sub_5cb050",
                    prototype="int (__thiscall sub_5CB050)(char *, int)"), {})


class CollisionRenameTest(unittest.TestCase):
    """Two catalogued rows, one symbol - the loser is renamed, not refused.

    `?get@NetFifo@@QAEHPAXPAIPAHPAI@Z` at 0x00633F70 compresses to
    `?get@NetFifo@@QAEHPAXPAIPAH1@Z`, which 0x00633D90 is catalogued as
    outright: 173 bytes and 229, one symbol. Every tool resolves a subject BY
    symbol, so the loser would have been handed the winner's context, target
    object and assembly.
    """

    def test_the_higher_address_gives_up_the_catalogued_spelling(self):
        self.assertIn(0x00633F70, tool.renamed_for_collision())
        self.assertNotIn(0x00633D90, tool.renamed_for_collision())

    def test_nothing_else_in_the_catalogue_collides(self):
        self.assertEqual({0x00633F70}, set(tool.renamed_for_collision()))


class ClassKeyTests(unittest.TestCase):
    """Which types are emitted `class` and which `struct`.

    The key is half the mangled symbol, and the catalogue cannot settle it:
    six classes disagree with themselves there. Neither can the image - no
    RTTI, no embedded mangled strings. Both objects are ours, so the answer
    only has to be the SAME on both sides, which makes it a naming
    convention: a type with methods is a class, a type that is only data is a
    struct.
    """

    def keys(self, *names):
        return tool.class_keys({index: {"name": name}
                                for index, name in enumerate(names)})

    def test_a_type_with_a_catalogued_method_is_a_class(self):
        self.assertEqual({"Buffer": "class"},
                         self.keys("?draw@Buffer@@QAEHH@Z"))

    def test_a_constructor_names_a_class_too(self):
        self.assertEqual("class", self.keys("??0Sprite@@QAE@XZ")["Sprite"])

    def test_a_compiler_generated_member_still_names_its_class(self):
        # `??_G<...>@Buffer@@` is a scalar deleting destructor, and none of
        # the three ordinary patterns describe its qualifier chain.
        self.assertEqual("class",
                         self.keys("??_Gthunk@Buffer@@UAEPAXI@Z")["Buffer"])

    def test_a_type_only_ever_passed_is_a_struct(self):
        self.assertEqual({"f": "class", "RECT": "struct"},
                         self.keys("?f@f@@QAEXPAURECT@@@Z"))

    def test_every_type_the_catalogue_names_gets_an_answer(self):
        # A type MISSING from the map is declared `struct` by the emitter and
        # left however the catalogue spelled it in the target - the same
        # disagreement in a quieter form.
        keys = self.keys("?f@@YAXPAVSprite@@@Z")
        self.assertEqual({"Sprite": "struct"}, keys)

    def test_the_receiver_wins_over_a_bare_use(self):
        keys = self.keys("?f@@YAXPAUSprite@@@Z", "?draw@Sprite@@QAEXXZ")
        self.assertEqual("class", keys["Sprite"])

    def test_a_class_whose_name_starts_with_a_key_is_not_a_phantom(self):
        # `Vector@@` reads as `V` + `ector` + `@@` to a whole-name scan, which
        # invented classes called `ector`, `ideo` and `nwind` - and each one
        # consumed a name-table slot, shifting every later back-reference.
        self.assertNotIn("ector", self.keys("?f@Vector@@QAEXPAURECT@@@Z"))


if __name__ == "__main__":
    unittest.main()
