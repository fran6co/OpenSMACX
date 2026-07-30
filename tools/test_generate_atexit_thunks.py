#!/usr/bin/env python3
"""Unit tests for the opaque-storage path in generate_atexit_thunks.

Every byte pattern here is synthesised at invented addresses rather than
copied out of the pinned executable: the decoders are what is under test, and
a committed fixture must not be derived from original bytes.
"""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import generate_adjustor_thunks as adjustor  # noqa: E402
import generate_atexit_thunks as gen  # noqa: E402
import generator_support as support  # noqa: E402


def mov_ecx(value: int) -> bytes:
    return b"\xb9" + (value & 0xFFFFFFFF).to_bytes(4, "little")


def rel32(opcode: int, origin: int, target: int) -> bytes:
    """A 5-byte E8/E9 whose displacement lands on `target` from `origin`."""
    return bytes([opcode]) + ((target - (origin + 5))
                              & 0xFFFFFFFF).to_bytes(4, "little")


def teardown_thunk(address: int, storage: int, target: int,
                   tail: bool = True, padding: bytes = b"") -> bytes:
    call = rel32(0xE9 if tail else 0xE8, address + 5, target)
    return mov_ecx(storage) + call + (b"" if tail else b"\xc3") + padding


class TestEntryExtent(unittest.TestCase):
    def test_defers_to_the_adjustor_generator(self):
        self.assertIs(gen.adjustor_module(), adjustor)
        self.assertIs(gen.adjustor_module().entry_extent,
                      adjustor.entry_extent)

    def test_uses_end_address_not_size_for_a_split_body(self):
        # `size` sums both spans of a split function; only the entry span is
        # a contiguous run a decoder may read.
        split = {"address": "0x00401000", "end_address": "0x00401010",
                 "size": "64"}
        self.assertEqual(gen.entry_extent(split), 0x10)

    def test_falls_back_to_size_without_an_end_address(self):
        self.assertEqual(gen.entry_extent({"size": "24"}), 24)


class TestDecodeOpaqueTeardownRaw(unittest.TestCase):
    def test_reads_storage_and_target_from_a_tail_jump(self):
        data = teardown_thunk(0x00440F20, 0x0073ACD8, 0x00444DB0)
        self.assertEqual(len(data), 10)
        self.assertEqual(
            gen.decode_opaque_teardown_raw(data, 0x00440F20),
            (0x0073ACD8, 0x00444DB0))

    def test_reads_the_call_and_return_spelling_too(self):
        data = teardown_thunk(0x00500000, 0x00900000, 0x00501234, tail=False)
        self.assertEqual(len(data), 11)
        self.assertEqual(gen.decode_opaque_teardown_raw(data, 0x00500000),
                         (0x00900000, 0x00501234))

    def test_reads_a_backward_displacement(self):
        data = teardown_thunk(0x00500000, 0x00900000, 0x00400000)
        self.assertEqual(gen.decode_opaque_teardown_raw(data, 0x00500000),
                         (0x00900000, 0x00400000))

    def test_ignores_alignment_padding(self):
        for pad in (b"\x90\x90", b"\xcc\xcc\xcc"):
            data = teardown_thunk(0x00440F20, 0x0073ACD8, 0x00444DB0,
                                  padding=pad)
            self.assertEqual(
                gen.decode_opaque_teardown_raw(data, 0x00440F20),
                (0x0073ACD8, 0x00444DB0))

    def test_rejects_a_body_that_is_not_the_template(self):
        address = 0x00440F20
        cases = {
            "no mov ecx": b"\x33\xc0" + rel32(0xE9, address + 2, 0x444DB0),
            "truncated": mov_ecx(0x73ACD8) + b"\xe9\x86",
            "not a call or jump": mov_ecx(0x73ACD8) + b"\xc3" * 5,
            "call without a return":
                mov_ecx(0x73ACD8) + rel32(0xE8, address + 5, 0x444DB0),
            "trailing instruction":
                teardown_thunk(address, 0x73ACD8, 0x444DB0) + b"\x50\xc3",
        }
        for label, data in cases.items():
            with self.subTest(label):
                self.assertIsNone(
                    gen.decode_opaque_teardown_raw(data, address))


class TestSeamName(unittest.TestCase):
    def test_spellings_match_the_adjustor_generator(self):
        # Same spelling is what makes an address already bound in
        # deleting_thunks.h resolve to the same symbol and be reused.
        for mangled in ("??1DiploWin@@QAE@XZ", "?close@Caviar@@QAEXXZ",
                        "j_??1Ambience@@QAE@XZ"):
            with self.subTest(mangled):
                self.assertEqual(gen.seam_name(mangled),
                                 adjustor.target_symbol(mangled))

    def test_does_not_spell_a_scalar_deleting_destructor(self):
        # `??_G` takes a flag and returns a pointer, so it can never be the
        # nullary `void(__thiscall)(void *)` an opaque teardown seam is. Not
        # naming it is what makes such a target a named leftover rather than
        # a seam with the wrong signature.
        self.assertEqual(gen.seam_name("??_GConsole@@UAEPAXI@Z"), "")

    def test_names_destructors_constructors_and_methods(self):
        self.assertEqual(gen.seam_name("??1DiploWin@@QAE@XZ"),
                         "DiploWinDtorTarget")
        self.assertEqual(gen.seam_name("j_??1Ambience@@QAE@XZ"),
                         "AmbienceDtorTarget")
        self.assertEqual(gen.seam_name("??0MapWin@@QAE@H@Z"),
                         "MapWinCtorTarget")
        self.assertEqual(gen.seam_name("?close@Caviar@@QAEXXZ"),
                         "CaviarCloseTarget")
        self.assertEqual(gen.seam_name("??1Wave_In_Device@@QAE@XZ"),
                         "Wave_In_DeviceDtorTarget")

    def test_returns_empty_for_a_name_it_cannot_spell(self):
        self.assertEqual(gen.seam_name("_atexit"), "")
        self.assertEqual(gen.seam_name("??_M@YGXPAXIHP6EX0@Z@Z"), "")


class TestLoadBindings(unittest.TestCase):
    def write(self, directory, name, text):
        (Path(directory) / name).write_text(text)

    def test_reads_address_commented_seams_and_honours_exclude(self):
        with tempfile.TemporaryDirectory() as directory:
            self.write(directory, "deleting_thunks.h",
                       "extern func_deleting_dtor *BaseWinDtorTarget;"
                       "   // 0x00420A70\n")
            self.write(directory, "atexit_thunks.h",
                       "extern func_thiscall_teardown *TimeDtorTarget;"
                       "   // 0x00616200\n")
            self.write(directory, "unrelated.h",
                       "extern int *NotASeam;\n"
                       "// extern func_deleting_dtor *Commented;   // 0x1\n")
            by_address, by_name = gen.load_bindings(directory)
            self.assertEqual(
                by_address[0x00420A70],
                ("BaseWinDtorTarget", "func_deleting_dtor",
                 "deleting_thunks.h"))
            self.assertIn(0x00616200, by_address)
            self.assertEqual(by_name["BaseWinDtorTarget"][0], 0x00420A70)
            self.assertEqual(len(by_address), 2)

            excluded, _ = gen.load_bindings(directory,
                                            exclude=("atexit_thunks.h",))
            self.assertNotIn(0x00616200, excluded)

    def test_refuses_one_address_bound_under_two_names(self):
        with tempfile.TemporaryDirectory() as directory:
            self.write(directory, "a_thunks.h",
                       "extern func_deleting_dtor *One;   // 0x00420A70\n")
            self.write(directory, "b_thunks.h",
                       "extern func_deleting_dtor *Two;   // 0x00420A70\n")
            with self.assertRaises(SystemExit) as raised:
                gen.load_bindings(directory)
            self.assertIn("0x00420A70", str(raised.exception))


class TestResolveSeams(unittest.TestCase):
    def bindings(self, by_address=None):
        by_address = by_address or {}
        by_name = {entry[0]: (address, entry[2])
                   for address, entry in by_address.items()}
        return by_address, by_name

    def test_defines_new_seams_in_address_order(self):
        needs = {0x00616200: ("??1Time@@QAE@XZ", gen.OPAQUE_NULLARY),
                 0x00472020: ("??1MessageWin@@QAE@XZ", gen.OPAQUE_NULLARY)}
        seams, declare, includes = gen.resolve_seams(needs, self.bindings())
        self.assertEqual([address for address, _n, _t in declare],
                         [0x00472020, 0x00616200])
        self.assertEqual(seams[0x00616200], "TimeDtorTarget")
        self.assertEqual(includes, [])

    def test_reuses_an_address_already_bound_elsewhere(self):
        needs = {0x00420A70: ("??1BaseWin@@QAE@XZ", gen.OPAQUE_NULLARY)}
        bindings = self.bindings({
            0x00420A70: ("BaseWinDtorTarget", "func_deleting_dtor",
                         "deleting_thunks.h")})
        seams, declare, includes = gen.resolve_seams(needs, bindings)
        self.assertEqual(seams, {0x00420A70: "BaseWinDtorTarget"})
        self.assertEqual(declare, [])
        self.assertEqual(includes, ["deleting_thunks.h"])

    def test_reuse_keeps_a_foreign_name_rather_than_the_preferred_one(self):
        needs = {0x00420A70: ("??1BaseWin@@QAE@XZ", gen.OPAQUE_NULLARY)}
        bindings = self.bindings({
            0x00420A70: ("SomeOtherName", "func_thiscall_teardown",
                         "deleting_thunks.h")})
        seams, declare, _includes = gen.resolve_seams(needs, bindings)
        self.assertEqual(seams, {0x00420A70: "SomeOtherName"})
        self.assertEqual(declare, [])

    def test_two_thunks_onto_one_address_share_a_single_seam(self):
        needs = {0x005FE2E0: ("??1Palette@@QAE@XZ", gen.OPAQUE_NULLARY)}
        seams, declare, _includes = gen.resolve_seams(needs, self.bindings())
        self.assertEqual(len(declare), 1)
        self.assertEqual(seams[0x005FE2E0], "PaletteDtorTarget")

    def test_refuses_a_name_already_bound_to_a_different_address(self):
        needs = {0x00616200: ("??1Time@@QAE@XZ", gen.OPAQUE_NULLARY)}
        bindings = self.bindings({
            0x00999999: ("TimeDtorTarget", "func_deleting_dtor",
                         "deleting_thunks.h")})
        with self.assertRaises(SystemExit) as raised:
            gen.resolve_seams(needs, bindings)
        self.assertIn("TimeDtorTarget", str(raised.exception))
        self.assertIn("0x00999999", str(raised.exception))

    def test_refuses_two_addresses_that_want_one_new_name(self):
        needs = {0x00100000: ("??1Time@@QAE@XZ", gen.OPAQUE_NULLARY),
                 0x00200000: ("??1Time@@QAE@XZ", gen.OPAQUE_NULLARY)}
        with self.assertRaises(SystemExit) as raised:
            gen.resolve_seams(needs, self.bindings())
        self.assertIn("0x00100000", str(raised.exception))
        self.assertIn("0x00200000", str(raised.exception))

    def test_refuses_reuse_through_a_non_generated_header(self):
        needs = {0x0060E740: ("??1CheckBox@@QAE@XZ", gen.OPAQUE_NULLARY)}
        bindings = self.bindings({
            0x0060E740: ("DialogsCheckBoxDestructor", "func_thiscall_teardown",
                         "dialogs.h")})
        with self.assertRaises(SystemExit) as raised:
            gen.resolve_seams(needs, bindings)
        self.assertIn("dialogs.h", str(raised.exception))

    def test_refuses_reuse_at_an_incompatible_type(self):
        needs = {0x00420A70: ("??1BaseWin@@QAE@XZ", gen.OPAQUE_NULLARY)}
        bindings = self.bindings({
            0x00420A70: ("BaseWinScalarDeleteTarget", "func_adjustor_p_u",
                         "adjustor_thunks.h")})
        with self.assertRaises(SystemExit) as raised:
            gen.resolve_seams(needs, bindings)
        self.assertIn("func_adjustor_p_u", str(raised.exception))

    def test_accepts_the_compatible_nullary_typedef_spellings(self):
        for typedef in sorted(gen.NULLARY_TYPEDEFS):
            with self.subTest(typedef):
                bindings = self.bindings({
                    0x00420A70: ("Bound", typedef, "deleting_thunks.h")})
                seams, _d, _i = gen.resolve_seams(
                    {0x00420A70: ("??1BaseWin@@QAE@XZ", gen.OPAQUE_NULLARY)},
                    bindings)
                self.assertEqual(seams[0x00420A70], "Bound")

    def test_refuses_a_target_it_cannot_name(self):
        with self.assertRaises(SystemExit):
            gen.resolve_seams({0x1: ("_atexit", gen.OPAQUE_NULLARY)},
                              self.bindings())


class TestMergeBindings(unittest.TestCase):
    def test_folds_in_seams_the_scanner_cannot_see(self):
        scanned = ({}, {})
        merged = gen.merge_bindings(scanned, {
            0x00456010: ("FactionArtElementTeardown", gen.OPAQUE_NULLARY,
                         "atexit_thunks.h")})
        self.assertEqual(merged[0][0x00456010][0], "FactionArtElementTeardown")
        self.assertEqual(merged[1]["FactionArtElementTeardown"][0], 0x00456010)

    def test_the_scan_wins_when_both_agree(self):
        scanned = ({0x1: ("Same", "func_deleting_dtor", "deleting_thunks.h")},
                   {"Same": (0x1, "deleting_thunks.h")})
        merged = gen.merge_bindings(
            scanned, {0x1: ("Same", gen.OPAQUE_NULLARY, "atexit_thunks.h")})
        self.assertEqual(merged[0][0x1][2], "deleting_thunks.h")

    def test_refuses_a_disagreement(self):
        scanned = ({0x1: ("One", "func_deleting_dtor", "deleting_thunks.h")},
                   {"One": (0x1, "deleting_thunks.h")})
        with self.assertRaises(SystemExit):
            gen.merge_bindings(
                scanned, {0x1: ("Two", gen.OPAQUE_NULLARY, "atexit_thunks.h")})

    def test_does_not_mutate_its_input(self):
        scanned = ({}, {})
        gen.merge_bindings(scanned, {0x1: ("A", gen.OPAQUE_NULLARY, "h")})
        self.assertEqual(scanned, ({}, {}))


class TestFormatting(unittest.TestCase):
    def test_wrapped_call_stays_on_one_line_when_it_fits(self):
        self.assertEqual(
            gen.wrapped_call("    ", "Seam", ["a", "b"]),
            ["    Seam(a, b);"])

    def test_wrapped_call_wraps_under_the_open_paren(self):
        lines = gen.wrapped_call("    ", "VectorCtorIterator", [
            gen.storage_literal(0x0078E978), "0x65C", "8",
            "FactionArtCtorTarget", "FactionArtElementTeardown"])
        self.assertEqual(len(lines), 2)
        self.assertTrue(all(len(line) <= 79 for line in lines))
        self.assertTrue(lines[1].startswith(" " * len("    VectorCtorIterator(")))
        self.assertTrue(lines[-1].endswith(");"))
        self.assertNotIn(" \n", "\n".join(lines))

    def test_wrapped_call_never_leaves_trailing_whitespace(self):
        lines = gen.wrapped_call("    ", "S" * 60, ["x" * 30, "y" * 30])
        for line in lines:
            self.assertEqual(line, line.rstrip())

    def test_storage_literal_is_a_padded_hex_cast(self):
        self.assertEqual(gen.storage_literal(0x73ACD8),
                         "reinterpret_cast<void *>(0x0073ACD8)")


class TestRowPartitioning(unittest.TestCase):
    def test_typed_and_opaque_split_and_preserve_order(self):
        rows = [{"address": 1}, {"address": 2, "opaque": True},
                {"address": 3}]
        self.assertEqual([r["address"] for r in gen.typed(rows)], [1, 3])
        self.assertEqual([r["address"] for r in gen.opaque(rows)], [2])

    def test_kind_of_is_none_for_an_opaque_row(self):
        self.assertIsNone(gen.kind_of({"opaque": True}))

    def test_opaque_needs_is_keyed_on_the_target_address(self):
        rows = [{"opaque": True, "target_address": 0x10,
                 "target_name": "??1A@@QAE@XZ"},
                {"opaque": True, "target_address": 0x10,
                 "target_name": "??1A@@QAE@XZ"},
                {"target_name": "??1Wave@@QAE@XZ"}]
        self.assertEqual(gen.opaque_needs(rows),
                         {0x10: ("??1A@@QAE@XZ", gen.OPAQUE_NULLARY)})


class TestRenderSeamBlock(unittest.TestCase):
    def test_emits_one_extern_per_seam_with_its_address(self):
        lines = gen.render_seam_block(
            [(0x00472020, "MessageWinDtorTarget", gen.OPAQUE_NULLARY)],
            ["// note"])
        self.assertEqual(lines, [
            "// note",
            "extern func_thiscall_teardown *MessageWinDtorTarget;"
            "   // 0x00472020"])
        # The comment is what load_bindings reads back for the dedupe, so the
        # shared scan must be able to parse what this generator emits.
        self.assertIsNotNone(support.SEAM_BINDING_RE.match(lines[-1]))


if __name__ == "__main__":
    unittest.main()
