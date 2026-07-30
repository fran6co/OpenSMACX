#!/usr/bin/env python3
"""Unit tests for the opaque-storage path in generate_init_thunks.

Every byte pattern here is synthesised at invented addresses rather than
copied out of the pinned executable: the decoders are what is under test, and
a committed fixture must not be derived from original bytes.
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import generate_adjustor_thunks as adjustor  # noqa: E402
import generate_atexit_thunks as atexit_gen  # noqa: E402
import generate_init_thunks as gen  # noqa: E402


def push(value: int) -> bytes:
    if -128 <= value <= 127:
        return bytes([0x6A, value & 0xFF])
    return b"\x68" + (value & 0xFFFFFFFF).to_bytes(4, "little")


def mov_ecx(value: int) -> bytes:
    return b"\xb9" + (value & 0xFFFFFFFF).to_bytes(4, "little")


def call(origin: int, target: int) -> bytes:
    return b"\xe8" + ((target - (origin + 5)) & 0xFFFFFFFF).to_bytes(
        4, "little")


ATEXIT = 0x00645398


def init_thunk(address, storage, target, arguments=(), registered=0x00401000,
               registrar=ATEXIT, padding=b""):
    """The whole ??__E grammar, assembled at `address`."""
    body = b"".join(push(value) for value in reversed(list(arguments)))
    if storage is not None:
        body += mov_ecx(storage)
    body += call(address + len(body), target)
    body += push(registered)
    body += call(address + len(body), registrar)
    return body + b"\x59\xc3" + padding


class TestImports(unittest.TestCase):
    def test_uses_the_adjustor_generator_entry_extent(self):
        self.assertIs(gen.entry_extent, adjustor.entry_extent)
        self.assertIs(gen.callee_pop, adjustor.callee_pop)

    def test_entry_extent_prefers_end_address_over_size(self):
        self.assertEqual(
            gen.entry_extent({"address": "0x00401000",
                              "end_address": "0x00401016", "size": "64"}),
            0x16)


class TestDecodeOpaqueInitRaw(unittest.TestCase):
    def test_reads_a_nullary_scalar_initializer(self):
        address = 0x004083B0
        data = init_thunk(address, 0x006A7628, 0x00408490,
                          registered=0x004083D0)
        self.assertEqual(len(data), 22)
        self.assertEqual(
            gen.decode_opaque_init_raw(data, address),
            ([], 0x006A7628, 0x00408490, 0x004083D0, ATEXIT))

    def test_reads_one_int_argument(self):
        address = 0x00462080
        data = init_thunk(address, 0x007D4060, 0x004626E0, arguments=[1],
                          registered=0x004620A0)
        self.assertEqual(len(data), 24)
        self.assertEqual(
            gen.decode_opaque_init_raw(data, address),
            ([1], 0x007D4060, 0x004626E0, 0x004620A0, ATEXIT))

    def test_reads_three_arguments_in_source_order(self):
        address = 0x0047A770
        data = init_thunk(address, 0x00805338, 0x0047ACF0,
                          arguments=[4096, 8, 0], registered=0x0047A790)
        arguments, storage, target, registered, registrar = \
            gen.decode_opaque_init_raw(data, address)
        # Pushes run right to left, so the decoder must hand them back the
        # way the call site reads: a reversed list here would silently swap
        # a size for a flag.
        self.assertEqual(arguments, [4096, 8, 0])
        self.assertEqual((storage, target, registered, registrar),
                         (0x00805338, 0x0047ACF0, 0x0047A790, ATEXIT))

    def test_sign_extends_a_byte_push(self):
        address = 0x0047A7A0
        data = init_thunk(address, 0x007FFF80, 0x0047ACF0,
                          arguments=[-5, 40, 1], registered=0x0047A7C0)
        self.assertEqual(gen.decode_opaque_init_raw(data, address)[0],
                         [-5, 40, 1])

    def test_reads_a_high_imm32_as_unsigned(self):
        address = 0x00401000
        data = init_thunk(address, 0x00900000, 0x00402000,
                          arguments=[0x80000000])
        self.assertEqual(gen.decode_opaque_init_raw(data, address)[0],
                         [0x80000000])

    def test_reads_the_array_form_which_has_no_mov_ecx(self):
        address = 0x0044CC50
        data = init_thunk(
            address, None, 0x006457C2,
            arguments=[0x0078E978, 0x65C, 8, 0x00455F10, 0x00456010],
            registered=0x0044CC80)
        arguments, storage, target, registered, _registrar = \
            gen.decode_opaque_init_raw(data, address)
        self.assertIsNone(storage)
        self.assertEqual(arguments,
                         [0x0078E978, 0x65C, 8, 0x00455F10, 0x00456010])
        self.assertEqual((target, registered), (0x006457C2, 0x0044CC80))

    def test_ignores_alignment_padding(self):
        address = 0x00401000
        for pad in (b"\x90", b"\xcc\xcc\xcc\xcc"):
            data = init_thunk(address, 0x00900000, 0x00402000, padding=pad)
            self.assertIsNotNone(gen.decode_opaque_init_raw(data, address))

    def test_rejects_bodies_that_are_not_the_template(self):
        address = 0x00401000
        whole = init_thunk(address, 0x00900000, 0x00402000)
        cases = {
            "no registration": mov_ecx(0x900000) + call(address + 5,
                                                        0x402000) + b"\xc3",
            "truncated": whole[:-1],
            "extra instruction": whole + b"\x90\x50",
            "registration is not a push imm32":
                mov_ecx(0x900000) + call(address + 5, 0x402000)
                + push(8) + call(address + 12, ATEXIT) + b"\x59\xc3",
            "tail jump": mov_ecx(0x900000) + b"\xe9\x00\x00\x00\x00",
        }
        for label, data in cases.items():
            with self.subTest(label):
                self.assertIsNone(gen.decode_opaque_init_raw(data, address))


class TestOpaqueCtorSignatures(unittest.TestCase):
    def test_accepts_nullary_and_all_int_constructors(self):
        self.assertEqual(gen.opaque_ctor_arity("??0BaseWin@@QAE@XZ"), 0)
        self.assertEqual(gen.opaque_ctor_arity("??0MapWin@@QAE@H@Z"), 1)
        self.assertEqual(gen.opaque_ctor_arity("??0NetMsg@@QAE@HHH@Z"), 3)

    def test_rejects_a_signature_a_pushed_literal_would_not_type(self):
        # A pointer or a float in the slot pushes the same four bytes an int
        # does; the mangled name is the only thing that says which, so an
        # unmodelled spelling is left alone rather than assumed to be int.
        for mangled in ("??0Sprite@@QAE@PAUBuffer@@@Z",
                        "??0Wave@@QAE@M@Z",
                        "??1BaseWin@@QAE@XZ",
                        "??0Console@@QAE@HPAX@Z",
                        "??_L@YGXPAXIHP6EX0@Z1@Z"):
            with self.subTest(mangled):
                self.assertIsNone(gen.opaque_ctor_arity(mangled))

    def test_typedef_names_track_arity(self):
        self.assertEqual(gen.opaque_ctor_typedef(0), gen.OPAQUE_NULLARY)
        self.assertEqual(gen.opaque_ctor_typedef(1), "func_opaque_ctor_i")
        self.assertEqual(gen.opaque_ctor_typedef(3), "func_opaque_ctor_iii")

    def test_renders_only_the_arities_in_use_and_wraps_at_79(self):
        lines = gen.render_ctor_typedefs({0, 1, 3})
        self.assertTrue(all(len(line) <= 79 for line in lines))
        self.assertTrue(all(line == line.rstrip() for line in lines))
        text = "\n".join(lines)
        self.assertIn("func_opaque_ctor_i)(void *object, int a0);", text)
        self.assertIn("int a2);", text)
        # Arity zero rides func_thiscall_teardown and needs no typedef.
        self.assertNotIn(gen.OPAQUE_NULLARY, text)

    def test_renders_nothing_when_every_opaque_ctor_is_nullary(self):
        self.assertEqual(gen.render_ctor_typedefs({0}), [])


class TestOpaqueNeeds(unittest.TestCase):
    def test_a_scalar_row_needs_one_seam_at_its_ctor_arity(self):
        rows = [{"opaque": True, "target_name": "??0MapWin@@QAE@H@Z",
                 "target_address": 0x004626E0, "arity": 1}]
        self.assertEqual(gen.opaque_needs(rows),
                         {0x004626E0: ("??0MapWin@@QAE@H@Z",
                                       "func_opaque_ctor_i")})

    def test_an_array_row_needs_both_element_seams(self):
        rows = [{"opaque": True, "target_name": gen.VECTOR_CTOR_NAME,
                 "ctor_name": "??0FactionArt@@QAE@XZ",
                 "ctor_address": 0x00455F10,
                 "dtor_name": "??1FactionArt@@QAE@XZ",
                 "dtor_address": 0x00456010}]
        self.assertEqual(gen.opaque_needs(rows), {
            0x00455F10: ("??0FactionArt@@QAE@XZ", gen.OPAQUE_NULLARY),
            0x00456010: ("??1FactionArt@@QAE@XZ", gen.OPAQUE_NULLARY)})

    def test_typed_rows_need_nothing(self):
        self.assertEqual(gen.opaque_needs([{"target_name": "??0Wave@@QAE@XZ"}]),
                         {})


class TestTableBindings(unittest.TestCase):
    def test_hands_over_the_element_seams_the_scanner_cannot_see(self):
        rows = [{"target_name": gen.VECTOR_CTOR_NAME,
                 "ctor_name": "??0Sprite@@QAE@XZ", "ctor_address": 0x1000,
                 "dtor_name": "??1Sprite@@QAE@XZ", "dtor_address": 0x2000}]
        f_rows = [{"target_name": atexit_gen.VECTOR_DTOR_NAME,
                   "teardown_name": "??1FactionArt@@QAE@XZ",
                   "teardown_address": 0x00456010}]
        table = gen.table_bindings(rows, f_rows)
        self.assertEqual(table[0x1000][0], "SpriteElementCtor")
        self.assertEqual(table[0x2000][0], "SpriteElementTeardown")
        self.assertEqual(table[0x00456010][0], "FactionArtElementTeardown")

    def test_an_opaque_row_contributes_no_table_binding(self):
        rows = [{"opaque": True, "target_name": gen.VECTOR_CTOR_NAME,
                 "ctor_name": "??0FactionArt@@QAE@XZ",
                 "ctor_address": 0x00455F10,
                 "dtor_name": "??1FactionArt@@QAE@XZ",
                 "dtor_address": 0x00456010}]
        self.assertEqual(gen.table_bindings(rows, []), {})

    def test_the_factionart_teardown_resolves_to_a_reuse_not_a_new_seam(self):
        # The one address both generators want. A second name for it would be
        # a duplicate definition at link time.
        rows = [{"opaque": True, "target_name": gen.VECTOR_CTOR_NAME,
                 "ctor_name": "??0FactionArt@@QAE@XZ",
                 "ctor_address": 0x00455F10,
                 "dtor_name": "??1FactionArt@@QAE@XZ",
                 "dtor_address": 0x00456010}]
        f_rows = [{"target_name": atexit_gen.VECTOR_DTOR_NAME,
                   "teardown_name": "??1FactionArt@@QAE@XZ",
                   "teardown_address": 0x00456010}]
        bindings = atexit_gen.merge_bindings(({}, {}),
                                             gen.table_bindings(rows, f_rows))
        seams, declare, _includes = atexit_gen.resolve_seams(
            gen.opaque_needs(rows), bindings)
        self.assertEqual(seams[0x00456010], "FactionArtElementTeardown")
        self.assertEqual([name for _a, name, _t in declare],
                         ["FactionArtCtorTarget"])


class TestRowPartitioning(unittest.TestCase):
    def test_typed_and_opaque_split_and_preserve_order(self):
        rows = [{"address": 1, "target_name": "??0Wave@@QAE@XZ"},
                {"address": 2, "opaque": True},
                {"address": 3, "target_name": "??0Wave@@QAE@XZ"}]
        self.assertEqual([r["address"] for r in gen.typed(rows)], [1, 3])
        self.assertEqual([r["address"] for r in gen.opaque(rows)], [2])

    def test_kind_of_is_none_for_an_opaque_row(self):
        self.assertIsNone(gen.kind_of({"opaque": True}))

    def test_opaque_scalars_group_by_arity_in_address_order(self):
        rows = [{"opaque": True, "target_name": "??0A@@QAE@H@Z", "arity": 1,
                 "address": 2},
                {"opaque": True, "target_name": "??0B@@QAE@XZ", "arity": 0,
                 "address": 1},
                {"opaque": True, "target_name": gen.VECTOR_CTOR_NAME}]
        grouped = gen.opaque_scalars(rows)
        self.assertEqual(list(grouped), [0, 1])
        self.assertEqual([r["address"] for r in grouped[0]], [1])


class TestRenderedOpaqueTest(unittest.TestCase):
    def rows(self):
        return [
            {"opaque": True, "address": 0x004083B0, "global_name": "g_BASEWIN",
             "global_address": 0x006A7628, "target_name": "??0BaseWin@@QAE@XZ",
             "target_address": 0x00408490, "arity": 0, "arguments": [],
             "registered": 0x004083D0, "seam": "BaseWinCtorTarget"},
            {"opaque": True, "address": 0x0047A770,
             "global_name": "g_NETMSG1", "global_address": 0x00805338,
             "target_name": "??0NetMsg@@QAE@HHH@Z",
             "target_address": 0x0047ACF0, "arity": 3,
             "arguments": [4096, 8, 0], "registered": 0x0047A790,
             "seam": "NetMsgCtorTarget"},
        ]

    def test_tables_carry_the_storage_address_and_every_literal(self):
        text = "\n".join(gen.render_opaque_tables(self.rows()))
        self.assertIn("reinterpret_cast<void *>(0x006A7628),", text)
        self.assertIn("reinterpret_cast<void *>(0x00805338), 4096, 8, 0,",
                      text)
        self.assertIn("&NetMsgCtorTarget,", text)
        self.assertIn("0x0047A790U", text)

    def test_rendered_lines_fit_and_carry_no_trailing_whitespace(self):
        lines = (gen.render_opaque_tables(self.rows())
                 + gen.render_opaque_observers(self.rows())
                 + gen.render_opaque_loops(self.rows()))
        for line in lines:
            self.assertLessEqual(len(line), 79, line)
            self.assertEqual(line, line.rstrip(), repr(line))

    def test_each_arity_gets_its_own_observer_and_loop(self):
        observers = "\n".join(gen.render_opaque_observers(self.rows()))
        self.assertIn("observe_opaque_ctor_iii(void *object, int a0, int a1,",
                      observers)
        loops = "\n".join(gen.render_opaque_loops(self.rows()))
        # The nullary loop reuses the existing recorder; the arity-3 loop must
        # check every literal, or a swapped pair would pass.
        self.assertIn("*entry.ctor_slot = &observe_init_ctor;", loops)
        self.assertIn("expect(g_init_opaque_args[2] == entry.a2);", loops)
        self.assertIn("expect(g_init_constructed == entry.storage);", loops)

    def test_no_opaque_rows_renders_nothing(self):
        typed_only = [{"target_name": "??0Wave@@QAE@XZ"}]
        self.assertEqual(gen.render_opaque_tables(typed_only), [])
        self.assertEqual(gen.render_opaque_observers(typed_only), [])
        self.assertEqual(gen.render_opaque_loops(typed_only), [])


class TestSourceOwnedGlobals(unittest.TestCase):
    """A global a hand-written file defines must be referenced, not redefined.

    src/strings.cpp defines `Strings *StringTable` and this generator used to
    emit a second definition of it into src/init_thunks.cpp. That only ever
    linked because the DLL passes -Wl,--allow-multiple-definition, which also
    suppresses genuinely accidental duplicates, so the flag could not be
    removed while the duplicate stood.
    """

    def rows(self):
        return [
            # The source-owned one: strings.cpp already defines it.
            {"address": 0x00616880, "global_name": "StringTable",
             "global_address": 0x009B90D8,
             "target_name": "??0Strings@@QAE@XZ",
             "target_address": 0x006168D0, "registered": 0x006168C0},
            # A generator-owned one, so every assertion below has a control:
            # the same render call must still declare and define this.
            {"address": 0x00616980, "global_name": "g_OTHER_STRINGS",
             "global_address": 0x009B9200,
             "target_name": "??0Strings@@QAE@XZ",
             "target_address": 0x006168D0, "registered": 0x006169C0},
        ]

    def test_the_header_declares_neither_a_definition_nor_an_extern(self):
        header = gen.render_header(self.rows(), {})
        self.assertNotIn("extern Strings *StringTable;", header)
        self.assertIn("extern Strings *g_OTHER_STRINGS;", header)

    def test_the_source_does_not_define_a_source_owned_global(self):
        source = gen.render_source(self.rows(), {}, 0x00645398)
        self.assertNotIn(
            "Strings *StringTable = (Strings *)0x009B90D8;", source)
        self.assertIn(
            "Strings *g_OTHER_STRINGS = (Strings *)0x009B9200;", source)

    def test_the_thunk_still_constructs_through_the_reused_global(self):
        # Skipping the definition must not skip the initializer: the thunk
        # body still names StringTable, now resolving to strings.cpp's object.
        source = gen.render_source(self.rows(), {}, 0x00645398)
        self.assertIn("    StringsInitCtor(StringTable);", source)
        self.assertIn("void __cdecl construct_stringtable() {", source)

    def test_the_pinned_address_is_the_one_strings_cpp_uses(self):
        address, header = gen.SOURCE_OWNED_GLOBALS["StringTable"]
        self.assertEqual(0x009B90D8, address)
        self.assertEqual("strings.h", header)
        definition = (Path(__file__).resolve().parent.parent / "src" /
                      "strings.cpp").read_text(encoding="utf-8")
        self.assertIn(
            f"Strings *StringTable = (Strings *)0x{address:08X};", definition)


class TestRenderedOpaqueSource(unittest.TestCase):
    def test_a_scalar_body_calls_the_seam_on_a_literal_address(self):
        body = atexit_gen.wrapped_call(
            "    ", "DiploWinCtorTarget",
            [atexit_gen.storage_literal(0x0073ACD8)])
        self.assertEqual(
            body,
            ["    DiploWinCtorTarget(reinterpret_cast<void *>(0x0073ACD8));"])
        self.assertLessEqual(len(body[0]), 79)


if __name__ == "__main__":
    unittest.main()
