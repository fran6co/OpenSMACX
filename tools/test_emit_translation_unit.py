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
        with self.assertRaises(tool.Unsettled):
            tool.Signature(row(prototype=""), {})


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


if __name__ == "__main__":
    unittest.main()
