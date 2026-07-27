#!/usr/bin/env python3

import unittest
from unittest import mock

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

import generate_deleting_thunks as deleting

ADDRESS = 0x00404420


class Instruction:
    def __init__(self, text):
        self.mnemonic, _, self.op_str = text.partition(" ")
        self.address = 0


def decode(*texts):
    """Fake capstone output; addresses are assigned by the caller."""
    return [Instruction(text) for text in texts]


def row_for(address, extent):
    return {"address": f"0x{address:08X}",
            "end_address": f"0x{address + extent:08X}",
            "size": str(extent)}


def real_decode(data, address):
    return list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(data, address))


def adjust_bytes(address, target, kind="imm32", value=0x444):
    """One Family A encoding, assembled so a test can perturb one field."""
    if kind == "imm32":
        head = b"\x81\xe9" + int(value).to_bytes(4, "little", signed=True)
    elif kind == "imm8":
        head = b"\x83\xe9" + int(value).to_bytes(1, "little", signed=True)
    elif kind == "vtordisp":
        head = b"\x2b\x49" + (-value).to_bytes(1, "little", signed=True)
    else:
        head = kind
    relative = target - (address + len(head) + 5)
    return head + b"\xe9" + relative.to_bytes(4, "little", signed=True)


def scalar_bytes(address, destructor, deleter, frame=True, slot=8, mask=1,
                 skip=9, pop=4, trailer=b""):
    """One Family B encoding, with every field a test may need to perturb.

    The two call offsets are spelled out rather than computed, because they
    are exactly what the generator must not get wrong: the ebp form's guarded
    call sits at +18 with the next instruction at +23, and the frameless
    form's at +16 with the next at +21."""
    if frame:
        prologue = b"\x55\x8b\xec\x56\x8b\xf1"
        guard = b"\xf6\x45" + bytes([slot, mask])
        epilogue = b"\x8b\xc6\x5e\x5d"
        destructor_call, deleter_call = 6, 18
    else:
        prologue = b"\x56\x8b\xf1"
        guard = b"\xf6\x44\x24" + bytes([slot, mask])
        epilogue = b"\x8b\xc6\x5e"
        destructor_call, deleter_call = 3, 16
    body = (prologue
            + b"\xe8" + (destructor - (address + destructor_call + 5))
            .to_bytes(4, "little", signed=True)
            + guard
            + b"\x74" + bytes([skip])
            + b"\x56"
            + b"\xe8" + (deleter - (address + deleter_call + 5))
            .to_bytes(4, "little", signed=True)
            + b"\x83\xc4\x04"
            + epilogue
            + b"\xc2" + pop.to_bytes(2, "little"))
    assert body[destructor_call] == 0xE8 and body[deleter_call] == 0xE8
    return body + trailer


class NameTest(unittest.TestCase):
    """The emitted symbol has to survive the names this family carries.

    Seven rows have no name at all and 47 carry `??3X@@SAXPAXI@Z`, which
    demangles as `static void __cdecl X::operator delete(void *, unsigned)`.
    Emitting that claim would put a caller-cleaned two-parameter free into
    committed source for a body that is a __thiscall receiver fixup."""

    def test_names_a_scalar_deleting_destructor_from_its_class(self):
        self.assertEqual(
            "scalar_delete_alpha_movie",
            deleting.scalar_delete_symbol("??_GAlphaMovie@@UAEPAXI@Z"))
        self.assertEqual("scalar_delete_sub_448390",
                         deleting.scalar_delete_symbol("sub_448390"))

    def test_never_spells_a_fixup_as_operator_delete(self):
        symbol = deleting.adjust_symbol("??3GraphicWin@@SAXPAXI@Z")
        self.assertEqual("adjust_this_graphic_win", symbol)
        self.assertNotIn("delete", symbol)
        self.assertNotIn("operator", symbol)

    def test_names_the_remaining_fixup_spellings(self):
        self.assertEqual("adjust_this_pop_menu_delete2",
                         deleting.adjust_symbol("PopMenu::delete2"))
        self.assertEqual("adjust_this_sub_404420",
                         deleting.adjust_symbol("sub_404420"))

    def test_refuses_a_name_it_cannot_place(self):
        self.assertEqual("", deleting.adjust_symbol("??_Bthunk@@"))
        self.assertEqual("", deleting.scalar_delete_symbol("loc_404420"))

    def test_names_a_seam_from_its_target(self):
        self.assertEqual(
            "AlphaMovieDtorTarget",
            deleting.target_symbol("??1AlphaMovie@@QAE@XZ", 0x404310))
        self.assertEqual(
            "AmbienceDtorTarget",
            deleting.target_symbol("j_??1Ambience@@QAE@XZ", 0x4483C0))
        self.assertEqual(
            "PlanWinScalarDeleteTarget",
            deleting.target_symbol("??_GPlanWin@@UAEPAXI@Z", 0x48BF20))
        self.assertEqual("Sub004C86D0Target",
                         deleting.target_symbol("sub_4c86d0", 0x4C86D0))


class FamilyTest(unittest.TestCase):
    """The extent has to be the template length exactly.

    The extent is the whole of what a decoder may read from the entry, so a
    prologue that matches over a longer body is a different function that
    happens to start the same way."""

    def test_recognises_each_template_at_its_own_length(self):
        self.assertEqual("adjust", deleting.family_of(b"\x81\xe9" + b"\0" * 9))
        self.assertEqual("adjust", deleting.family_of(b"\x83\xe9" + b"\0" * 6))
        self.assertEqual("adjust",
                         deleting.family_of(b"\x2b\x49\xfc" + b"\0" * 5))
        self.assertEqual("scalar", deleting.family_of(
            b"\x55\x8b\xec\x56\x8b\xf1" + b"\0" * 27))
        self.assertEqual("scalar",
                         deleting.family_of(b"\x56\x8b\xf1" + b"\0" * 27))

    def test_rejects_a_matching_prologue_over_a_longer_body(self):
        self.assertIsNone(deleting.family_of(b"\x81\xe9" + b"\0" * 40))
        self.assertIsNone(deleting.family_of(
            b"\x55\x8b\xec\x56\x8b\xf1" + b"\0" * 60))


class RawAdjustTest(unittest.TestCase):
    """The encoding is read a second time, without capstone's renderer.

    The builders require both readings to agree, so this path only earns its
    keep by being at least as strict as the capstone matcher."""

    def test_reads_the_imm32_form(self):
        self.assertEqual(
            ("imm", 0x444, 0x4043A0),
            deleting.decode_raw_adjust(
                adjust_bytes(0x404430, 0x4043A0), 0x404430))

    def test_reads_the_sign_extended_imm8_form(self):
        self.assertEqual(
            ("imm", 0x58, 0x6336D0),
            deleting.decode_raw_adjust(
                adjust_bytes(0x633730, 0x6336D0, "imm8", 0x58), 0x633730))

    def test_reads_the_vtordisp_form(self):
        self.assertEqual(
            ("vtordisp", 4, 0x48BF20),
            deleting.decode_raw_adjust(
                adjust_bytes(0x48BF10, 0x48BF20, "vtordisp", 4), 0x48BF10))

    def test_rejects_a_sub_through_another_register(self):
        # `sub ecx, [eax-4]` is not the template, however it renders.
        self.assertIsNone(deleting.decode_raw_adjust(
            b"\x2b\x48\xfc\xe9\x00\x00\x00\x00", ADDRESS))

    def test_rejects_a_missing_tail_jump(self):
        self.assertIsNone(deleting.decode_raw_adjust(
            b"\x81\xe9\x44\x04\x00\x00\xc3", ADDRESS))

    def test_rejects_anything_after_the_tail_jump(self):
        self.assertIsNone(deleting.decode_raw_adjust(
            adjust_bytes(ADDRESS, 0x4042B0) + b"\x90", ADDRESS))


class AdjustTemplateTest(unittest.TestCase):
    """Anything but the exact template is skipped and named, never guessed."""

    def decode_with(self, *texts):
        with mock.patch.object(deleting.delegates, "decode",
                               return_value=decode(*texts)), \
             mock.patch.object(deleting.delegates, "trailing_padding",
                               side_effect=lambda i: i):
            return deleting.decode_adjust(None, row_for(ADDRESS, 11))

    def test_accepts_the_immediate_form(self):
        self.assertEqual(("imm", 0x444, 0x4043A0),
                         self.decode_with("sub ecx, 0x444", "jmp 0x4043a0"))

    def test_accepts_the_vtordisp_form(self):
        self.assertEqual(
            ("vtordisp", 4, 0x48BF20),
            self.decode_with("sub ecx, dword ptr [ecx - 4]", "jmp 0x48bf20"))

    def test_rejects_a_vtordisp_at_another_displacement(self):
        # The two-instruction body at another displacement is the virtual
        # base adjustor family, which has its own generator.
        self.assertIsInstance(
            self.decode_with("sub ecx, dword ptr [ecx - 8]", "jmp 0x48bf20"),
            str)

    def test_rejects_an_indirect_tail_jump(self):
        self.assertIsInstance(
            self.decode_with("sub ecx, 0x444",
                             "jmp dword ptr [edx + 0x100]"), str)

    def test_rejects_a_body_that_does_more_than_adjust_and_jump(self):
        self.assertIsInstance(
            self.decode_with("sub ecx, 0x444", "mov eax, dword ptr [ecx]",
                             "jmp 0x4043a0"), str)


class RawScalarDeleteTest(unittest.TestCase):
    """The guarded call is the value an off-by-one would silently corrupt.

    0x0064557E, one byte before `operator delete`, is the tail of
    `__alloca_probe` - a plausible-looking function that nothing downstream
    would question. So the byte reader walks the body instruction by
    instruction instead of indexing a constant, and these pin what walking
    has to produce."""

    def test_reads_the_framed_form_and_both_call_offsets(self):
        data = scalar_bytes(0x004043A0, 0x00404310, 0x0064557F)
        self.assertEqual(33, len(data))
        self.assertEqual(0xE8, data[18])
        self.assertEqual(
            ("frame", 0x00404310, 0x0064557F, 1, 4, 0),
            deleting.decode_raw_scalar_delete(data, 0x004043A0))

    def test_reads_the_frameless_form_and_both_call_offsets(self):
        data = scalar_bytes(0x005D7140, 0x005D4DD0, 0x0064557F, frame=False)
        self.assertEqual(30, len(data))
        self.assertEqual(0xE8, data[16])
        self.assertEqual(
            ("frameless", 0x005D4DD0, 0x0064557F, 1, 4, 0),
            deleting.decode_raw_scalar_delete(data, 0x005D7140))

    def test_a_base_one_byte_early_would_name_alloca_probe(self):
        # Pins both offsets and shows what getting them wrong buys: a
        # relative call is resolved against the address of the FOLLOWING
        # instruction, and taking that one byte early lands on 0x0064557E,
        # the tail of `__alloca_probe`, rather than on `operator delete`.
        for frame, opcode, following in ((True, 18, 23), (False, 16, 21)):
            data = scalar_bytes(0x004043A0, 0x00404310, 0x0064557F,
                                frame=frame)
            self.assertEqual(0xE8, data[opcode])
            relative = int.from_bytes(data[opcode + 1:opcode + 5], "little",
                                      signed=True)
            self.assertEqual(0x0064557F, 0x004043A0 + following + relative)
            self.assertEqual(0x0064557E,
                             0x004043A0 + following - 1 + relative)

    def test_derives_the_flag_parameter_from_the_guard_slot(self):
        # Both forms name parameter 0 at slot 8 - the frameless one only
        # because the single `push esi` is accounted for.
        for frame in (True, False):
            data = scalar_bytes(ADDRESS, 0x1000, 0x0064557F, frame=frame)
            self.assertEqual(0, deleting.decode_raw_scalar_delete(
                data, ADDRESS)[5])
            data = scalar_bytes(ADDRESS, 0x1000, 0x0064557F, frame=frame,
                                slot=12)
            self.assertEqual(1, deleting.decode_raw_scalar_delete(
                data, ADDRESS)[5])

    def test_reads_the_pop_count_and_the_mask_rather_than_assuming_them(self):
        data = scalar_bytes(ADDRESS, 0x1000, 0x0064557F, mask=2, pop=8)
        decoded = deleting.decode_raw_scalar_delete(data, ADDRESS)
        self.assertEqual(2, decoded[3])
        self.assertEqual(8, decoded[4])

    def test_rejects_a_branch_that_does_not_skip_exactly_the_free(self):
        # A shorter branch would leave the free running unconditionally and
        # the flag controlling nothing.
        data = scalar_bytes(ADDRESS, 0x1000, 0x0064557F, skip=8)
        self.assertIsNone(deleting.decode_raw_scalar_delete(data, ADDRESS))

    def test_rejects_a_body_with_anything_after_the_return(self):
        data = scalar_bytes(ADDRESS, 0x1000, 0x0064557F, trailer=b"\x90")
        self.assertIsNone(deleting.decode_raw_scalar_delete(data, ADDRESS))

    def test_rejects_a_prologue_that_is_not_this_family(self):
        self.assertIsNone(
            deleting.decode_raw_scalar_delete(b"\x55\x8b\xec" + b"\0" * 30,
                                              ADDRESS))


class ScalarDeleteTemplateTest(unittest.TestCase):
    def decode_with(self, *texts, extent=33):
        instructions = decode(*texts)
        for index, instruction in enumerate(instructions):
            instruction.address = ADDRESS + index
        with mock.patch.object(deleting.delegates, "decode",
                               return_value=instructions), \
             mock.patch.object(deleting.delegates, "trailing_padding",
                               side_effect=lambda i: i):
            return deleting.decode_scalar_delete(None, row_for(ADDRESS,
                                                               extent))

    def framed(self, overrides=None):
        # decode_with numbers the instructions one byte apart, so the branch
        # target that has to land on `mov eax, esi` is ADDRESS + 10.
        texts = ["push ebp", "mov ebp, esp", "push esi", "mov esi, ecx",
                 "call 0x404310", "test byte ptr [ebp + 8], 1",
                 f"je 0x{ADDRESS + 10:x}", "push esi", "call 0x64557f",
                 "add esp, 4", "mov eax, esi", "pop esi", "pop ebp", "ret 4"]
        for index, text in (overrides or {}).items():
            texts[index] = text
        return self.decode_with(*texts)

    def test_accepts_the_framed_form(self):
        self.assertEqual(("frame", 0x404310, 0x64557F, 1, 4, 0), self.framed())

    def test_rejects_an_indirect_destructor_call(self):
        self.assertIsInstance(self.framed({4: "call dword ptr [ecx]"}), str)

    def test_rejects_a_branch_that_lands_off_the_shared_return(self):
        self.assertIsInstance(self.framed({6: f"je 0x{ADDRESS + 12:x}"}), str)

    def test_rejects_a_guard_on_a_register_rather_than_a_parameter(self):
        self.assertIsInstance(self.framed({5: "test al, 1"}), str)

    def test_rejects_a_prologue_that_is_not_this_family(self):
        self.assertIsInstance(
            self.decode_with("push ebp", "mov ebp, esp", "sub esp, 8",
                             "ret 4"), str)


class DecoderAgreementTest(unittest.TestCase):
    """The two readings have to agree, or nothing is emitted.

    Running the real capstone matcher over the same bytes the byte reader
    walks is the only check that the pair is actually redundant rather than
    two spellings of one mistake."""

    def agree_on(self, address, data, decoder, raw_decoder, extent):
        with mock.patch.object(deleting.delegates, "decode",
                               side_effect=lambda pe, a, n: real_decode(data,
                                                                        a)):
            capstone_reading = decoder(None, row_for(address, extent))
        self.assertEqual(capstone_reading, raw_decoder(data, address))
        return capstone_reading

    def test_both_readings_of_a_framed_scalar_delete(self):
        data = scalar_bytes(0x004043A0, 0x00404310, 0x0064557F)
        self.assertEqual(
            ("frame", 0x00404310, 0x0064557F, 1, 4, 0),
            self.agree_on(0x004043A0, data, deleting.decode_scalar_delete,
                          deleting.decode_raw_scalar_delete, 33))

    def test_both_readings_of_a_frameless_scalar_delete(self):
        data = scalar_bytes(0x005D7140, 0x005D4DD0, 0x0064557F, frame=False)
        self.assertEqual(
            ("frameless", 0x005D4DD0, 0x0064557F, 1, 4, 0),
            self.agree_on(0x005D7140, data, deleting.decode_scalar_delete,
                          deleting.decode_raw_scalar_delete, 30))

    def test_both_readings_of_each_adjust_form(self):
        for kind, value, extent in (("imm32", 0x444, 11), ("imm8", 0x58, 8),
                                    ("vtordisp", 4, 8)):
            data = adjust_bytes(ADDRESS, 0x4042B0, kind, value)
            self.assertEqual(
                ("vtordisp" if kind == "vtordisp" else "imm", value,
                 0x4042B0),
                self.agree_on(ADDRESS, data, deleting.decode_adjust,
                              deleting.decode_raw_adjust, extent))


class ArityTest(unittest.TestCase):
    """Arity is the one wrong guess that corrupts a caller's stack.

    Family B proves its own: `ret 4` pops one slot and the guard reads the
    parameter that slot belongs to. A body that disagrees with either is
    skipped and named rather than emitted with a plausible signature."""

    def build(self, decoded):
        row = dict(row_for(0x004043A0, 33), name="??_GAlphaMovie@@UAEPAXI@Z",
                   recovery_state="unrecovered", source_locations="")
        functions = {0x00404310: {"name": "??1AlphaMovie@@QAE@XZ",
                                  "recovery_state": "unrecovered"}}
        with mock.patch.object(deleting, "decode_scalar_delete",
                               return_value=decoded), \
             mock.patch.object(deleting, "decode_raw_scalar_delete",
                               return_value=decoded):
            return deleting.build_scalar(None, functions,
                                         [(0x004043A0, row, b"")])

    def test_accepts_one_popped_slot_guarded_as_parameter_zero(self):
        rows, leftovers, _ = self.build(
            ("frame", 0x00404310, deleting.DELETER_ADDRESS, 1, 4, 0))
        self.assertEqual([], leftovers)
        self.assertEqual("scalar_delete_alpha_movie", rows[0]["symbol"])

    def test_refuses_a_body_that_pops_something_else(self):
        rows, leftovers, _ = self.build(
            ("frame", 0x00404310, deleting.DELETER_ADDRESS, 1, 8, 0))
        self.assertEqual([], rows)
        self.assertIn("pops 8", leftovers[0][2])

    def test_refuses_a_guard_on_a_parameter_the_pop_does_not_cover(self):
        rows, leftovers, _ = self.build(
            ("frame", 0x00404310, deleting.DELETER_ADDRESS, 1, 4, 1))
        self.assertEqual([], rows)
        self.assertIn("parameter 1", leftovers[0][2])

    def test_refuses_a_deleter_that_is_not_the_shared_one(self):
        rows, leftovers, _ = self.build(
            ("frame", 0x00404310, deleting.DELETER_ADDRESS - 1, 1, 4, 0))
        self.assertEqual([], rows)
        self.assertIn("0x0064557E", leftovers[0][2])


class ForwardTest(unittest.TestCase):
    """A Family A row pops nothing, so its contract is entirely the target's.

    The mangled name says two caller-cleaned parameters for 47 of them; only
    `callee_pop` is allowed to decide, and a row it cannot answer for is
    skipped."""

    def setUp(self):
        self.functions = {
            0x004043A0: {"name": "??_GAlphaMovie@@UAEPAXI@Z",
                         "recovery_state": "unrecovered"},
            0x004042B0: {"name": "sub_4042b0",
                         "recovery_state": "unrecovered"},
            0x00406F20: {"name": "??_GScroll@@UAEPAXI@Z",
                         "recovery_state": "source_complete"},
            0x00406880: {"name": "??1FlatButton@@QAE@XZ",
                         "recovery_state": "source_complete"},
        }
        self.emitted = {0x004043A0: {"symbol": "scalar_delete_alpha_movie",
                                     "address": 0x004043A0}}

    def test_calls_a_body_this_run_recovers_rather_than_seaming_it(self):
        spelling, include, seam = deleting.resolve_forward(
            self.functions, 0x004043A0, 4, self.emitted)
        self.assertIsNone(seam)
        self.assertIsNone(include)
        self.assertIn("scalar_delete_alpha_movie", spelling)

    def test_reuses_an_existing_recovery_instead_of_a_second_binding(self):
        spelling, include, seam = deleting.resolve_forward(
            self.functions, 0x00406F20, 4, self.emitted)
        self.assertIsNone(seam)
        self.assertEqual("scroll.h", include)
        self.assertIn("scroll_scalar_dtor_redirect", spelling)

    def test_refuses_an_existing_recovery_whose_pop_disagrees(self):
        self.assertIsInstance(
            deleting.resolve_forward(self.functions, 0x00406F20, 0,
                                     self.emitted), str)

    def test_seams_only_what_nothing_else_covers(self):
        _, _, seam = deleting.resolve_forward(self.functions, 0x004042B0, 0,
                                              self.emitted)
        self.assertEqual(("Sub004042B0Target", 0x004042B0,
                          deleting.FORWARD_TYPES[0]), seam)

    def test_refuses_to_seam_over_a_body_somebody_else_owns(self):
        self.assertIsInstance(
            deleting.resolve_forward(self.functions, 0x00406880, 4,
                                     self.emitted), str)

    def test_refuses_an_uncatalogued_target(self):
        self.assertIsInstance(
            deleting.resolve_forward(self.functions, 0x0060CE50, 4,
                                     self.emitted), str)


class DestructorTest(unittest.TestCase):
    def setUp(self):
        self.functions = {
            0x00404310: {"name": "??1AlphaMovie@@QAE@XZ",
                         "recovery_state": "unrecovered"},
            0x005EBC90: {"name": "??1Win@@QAE@XZ",
                         "recovery_state": "original_dependency"},
            0x004C6120: {"name": "??1Sound@@QAE@XZ",
                         "recovery_state": "source_complete"},
        }

    def test_seams_an_unrecovered_destructor(self):
        spelling, _, seam = deleting.resolve_destructor(self.functions,
                                                        0x00404310)
        self.assertEqual(("AlphaMovieDtorTarget", 0x00404310,
                          deleting.DTOR_TYPE), seam)
        self.assertEqual("AlphaMovieDtorTarget({0})", spelling)

    def test_reuses_the_binding_an_original_dependency_already_has(self):
        spelling, include, seam = deleting.resolve_destructor(self.functions,
                                                              0x005EBC90)
        self.assertIsNone(seam)
        self.assertEqual("graphicwin.h", include)
        self.assertEqual("WinOriginalDestructor({0})", spelling)

    def test_refuses_a_recovered_destructor_it_has_no_spelling_for(self):
        # Left alone rather than seamed: a second fixed-address binding onto
        # a body somebody else owns would quietly compete with theirs.
        self.assertIsInstance(
            deleting.resolve_destructor(self.functions, 0x004C6120), str)

    def test_refuses_a_handled_entry_whose_address_drifted(self):
        functions = {0x00401000: {"name": "??1Win@@QAE@XZ",
                                  "recovery_state": "original_dependency"}}
        with self.assertRaises(SystemExit):
            deleting.resolve_destructor(functions, 0x00401000)


class SeamTableTest(unittest.TestCase):
    def row(self, seam):
        return {"seam": seam}

    def test_shares_one_seam_between_bodies_onto_the_same_target(self):
        rows = [self.row(("PopMenuDtorTarget", 0x421400, "d")),
                self.row(("PopMenuDtorTarget", 0x421400, "d"))]
        self.assertEqual({"PopMenuDtorTarget": (0x421400, "d")},
                         deleting.seam_table(rows))

    def test_ignores_a_body_that_needs_no_seam(self):
        self.assertEqual({}, deleting.seam_table([self.row(None)]))

    def test_refuses_one_name_at_two_addresses(self):
        rows = [self.row(("PopMenuDtorTarget", 0x421400, "d")),
                self.row(("PopMenuDtorTarget", 0x421410, "d"))]
        with self.assertRaises(SystemExit):
            deleting.seam_table(rows)

    def test_refuses_one_address_at_two_signatures(self):
        # Deduping on the name alone would let the same function be reached
        # through two seams the suite could rebind independently.
        rows = [self.row(("PopMenuDtorTarget", 0x421400, "d")),
                self.row(("PopMenuScalarDeleteTarget", 0x421400, "f"))]
        with self.assertRaises(SystemExit):
            deleting.seam_table(rows)


class OwnershipTest(unittest.TestCase):
    """Both shapes are also worn by recoveries this generator does not own.

    93 virtual-base adjustor thunks decode as Family A and four hand-written
    scalar deleting destructors decode as Family B. Emitting those would
    define the same recovery twice; excluding them by recorded source file
    rather than by an address list is what lets a rerun keep re-emitting the
    rows this generator itself recovered."""

    def test_takes_every_unrecovered_row(self):
        self.assertTrue(deleting.owns({"recovery_state": "unrecovered",
                                       "source_locations": ""}))

    def test_leaves_a_recovery_that_lives_in_another_file(self):
        self.assertFalse(deleting.owns(
            {"recovery_state": "source_complete",
             "source_locations": "src/adjustor_thunks.cpp:412"}))

    def test_keeps_re_emitting_its_own_earlier_output(self):
        self.assertTrue(deleting.owns(
            {"recovery_state": "source_complete",
             "source_locations": f"{deleting.GENERATED_SOURCE}:118"}))


class SignatureTest(unittest.TestCase):
    def test_a_four_byte_pop_becomes_one_declared_parameter(self):
        declarations, argument = deleting.parameters_of(
            {"family": "adjust", "pop": 4})
        self.assertEqual(["void *self", "void *", "unsigned int arg0"],
                         declarations)
        self.assertEqual("arg0", argument)

    def test_a_zero_pop_declares_no_stack_parameter(self):
        declarations, argument = deleting.parameters_of(
            {"family": "adjust", "pop": 0})
        self.assertEqual(["void *self", "void *"], declarations)
        self.assertIsNone(argument)

    def test_a_scalar_delete_always_declares_its_proven_flags_word(self):
        declarations, argument = deleting.parameters_of({"family": "scalar"})
        self.assertEqual(["void *self", "void *", "unsigned int arg0"],
                         declarations)
        self.assertEqual("arg0", argument)


class BodyTest(unittest.TestCase):
    """Each body spells its own constants, so the mutation sweep has targets.

    The harness only perturbs functions introduced by an `Original Offset:`
    comment, and it can only perturb literals it can see in the body - a
    shared adjustment table would leave it nothing to change."""

    def adjust_row(self, **overrides):
        row = {"family": "adjust", "address": 0x00404430, "kind": "imm",
               "value": 0x444, "pop": 4, "name": "??3AlphaMovie@@SAXPAXI@Z",
               "target_name": "??_GAlphaMovie@@UAEPAXI@Z",
               "symbol": "adjust_this_alpha_movie",
               "call": "scalar_delete_alpha_movie({0}, nullptr, {1})"}
        row.update(overrides)
        return row

    def test_the_adjustment_is_an_inline_literal(self):
        body = "\n".join(deleting.render_body(self.adjust_row()))
        self.assertIn("Original Offset: 00404430", body)
        self.assertIn("object - 0x444", body)

    def test_the_vtordisp_form_reads_the_word_ahead_of_the_receiver(self):
        body = "\n".join(deleting.render_body(
            self.adjust_row(kind="vtordisp", value=4)))
        self.assertIn("*reinterpret_cast<const int32_t *>(object - 4)", body)
        self.assertIn("object - vtordisp", body)

    def test_the_flag_mask_is_an_inline_literal(self):
        body = "\n".join(deleting.render_body(
            {"family": "scalar", "address": 0x004043A0,
             "name": "??_GAlphaMovie@@UAEPAXI@Z",
             "symbol": "scalar_delete_alpha_movie",
             "call": "AlphaMovieDtorTarget({0})"}))
        self.assertIn("Original Offset: 004043A0", body)
        self.assertIn("if (arg0 & 1) {", body)
        self.assertIn(f"{deleting.DELETER_SYMBOL}(self);", body)

    def test_every_emitted_line_fits_the_column_limit(self):
        for row in (self.adjust_row(),
                    self.adjust_row(value=0x57C0,
                                    symbol="adjust_this_pop_menu_delete3"),
                    self.adjust_row(
                        pop=0, call="Sub004042B0Target({0})",
                        symbol="adjust_this_sub_404420")):
            for line in deleting.render_body(row):
                self.assertLessEqual(len(line), 79, line)
                self.assertEqual(line, line.rstrip())


if __name__ == "__main__":
    unittest.main()
