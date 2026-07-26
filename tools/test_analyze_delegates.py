#!/usr/bin/env python3

"""Tests for the forwarder acceptor.

These are written against assembled byte sequences rather than the pinned
executable, so they run without it and so each case states the exact shape it
is about. What matters most here is the rejections: the value of a proven
forwarder is that a generated body needs no human judgement, which is worth
nothing if the acceptor is loose.
"""

import unittest

import analyze_delegates as delegates


class FakeInstruction:
    def __init__(self, mnemonic, op_str, length=1):
        self.mnemonic = mnemonic
        self.op_str = op_str
        self.bytes = b"\x90" * length
        self.address = 0


def row(name="?f@@YAXH@Z", address="0x00401000", size=32):
    return {"name": name, "address": address, "size": str(size)}


def frame(*pairs):
    return [FakeInstruction("push", "ebp"), FakeInstruction("mov", "ebp, esp")] \
        + [FakeInstruction(mnemonic, operands) for mnemonic, operands in pairs]


def flat(*pairs):
    return [FakeInstruction(mnemonic, operands) for mnemonic, operands in pairs]


class NameDecodingTest(unittest.TestCase):
    def test_decodes_free_functions(self):
        # find_constant_returns.declared_arity reads the convention from the
        # member-function position, so every free function decoded as None
        # there; this is the case the local decoder exists for.
        self.assertEqual(("cdecl", 4),
                         delegates.name_convention_and_arity("?f@@YAXH@Z"))
        self.assertEqual(("stdcall", 8),
                         delegates.name_convention_and_arity("?f@@YGXHH@Z"))

    def test_decodes_member_functions(self):
        self.assertEqual(
            ("thiscall", 8),
            delegates.name_convention_and_arity("?exec@Datalink@@QAEXIH@Z"))

    def test_first_separator_wins(self):
        # Struct parameters carry their own `@@`; reading the signature from
        # the last separator would decode the middle of the parameter list.
        decoded = delegates.name_convention_and_arity(
            "?X_pops@@YAHPBDPAUSprite@@HHP6AHXZ@Z")
        self.assertIsNotNone(decoded)
        self.assertEqual("cdecl", decoded[0])

    def test_declines_what_it_cannot_read(self):
        self.assertIsNone(delegates.name_convention_and_arity("sub_401000"))
        self.assertIsNone(delegates.name_convention_and_arity("_free"))


class FrameForwardTest(unittest.TestCase):
    def test_accepts_a_parameter_forward_with_literal_padding(self):
        # The synch_veh shape: one incoming parameter placed among literals.
        forward = delegates.analyze_forward(frame(
            ("mov", "eax, dword ptr [ebp + 8]"),
            ("push", "0x2101"),
            ("push", "eax"),
            ("mov", "ecx, 0x93cd90"),
            ("call", "0x532e00"),
            ("pop", "ebp"),
            ("ret", ""),
        ), row())
        self.assertEqual("frame-forward", forward.form)
        self.assertEqual(0x532E00, forward.target)
        self.assertEqual("global:0x0093CD90", forward.receiver)
        # Pushed right-to-left, so the source order is the reverse.
        self.assertEqual(["param:0", "imm:0x2101"],
                         [a.describe() for a in forward.arguments])

    def test_parameter_indices_come_from_the_frame_displacement(self):
        forward = delegates.analyze_forward(frame(
            ("mov", "eax, dword ptr [ebp + 0xc]"),
            ("mov", "ecx, dword ptr [ebp + 8]"),
            ("push", "eax"),
            ("push", "ecx"),
            ("call", "0x429180"),
            ("pop", "ebp"),
            ("ret", ""),
        ), row(name="?f@@YAXHH@Z"))
        self.assertEqual(["param:0", "param:1"],
                         [a.describe() for a in forward.arguments])
        # ecx held a parameter here, not a fixed global, so the receiver is
        # that parameter rather than a singleton.
        self.assertEqual("param:0", forward.receiver)

    def test_rejects_a_read_past_the_declared_parameters(self):
        # [ebp+0xc] is parameter 1 of a function that declares only one; a
        # forwarder cannot be reading it, whatever else the body looks like.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 0xc]"),
                ("push", "eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row(name="?f@@YAXH@Z"))

    def test_records_the_callee_pop_count(self):
        forward = delegates.analyze_forward(frame(
            ("mov", "eax, dword ptr [ebp + 8]"),
            ("push", "eax"),
            ("call", "0x429180"),
            ("pop", "ebp"),
            ("ret", "4"),
        ), row(name="?f@@YGXH@Z"))
        self.assertEqual(4, forward.popped)

    def test_rejects_a_cdecl_body_that_pops(self):
        # `ret 4` from a declared __cdecl function contradicts the name; one
        # of the two is wrong and neither may be guessed at.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("push", "eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", "4"),
            ), row(name="?f@@YAXH@Z"))

    def test_rejects_a_stdcall_body_popping_the_wrong_count(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("push", "eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", "8"),
            ), row(name="?f@@YGXH@Z"))

    def test_rejects_cleanup_that_does_not_match_the_pushes(self):
        # `add esp, 8` after one push means something else was on the stack;
        # whatever this is, it is not a plain forward.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("push", "eax"),
                ("call", "0x429180"),
                ("add", "esp, 8"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_a_conditional_branch(self):
        # A guard is not a forward; accepting one would drop the guard.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("test", "eax, eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_a_store_the_forward_would_lose(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "dword ptr [ecx + 4], eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_an_indirect_call(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("call", "dword ptr [eax + 0xe8]"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_work_after_the_call(self):
        # `neg eax / sbb eax, eax / neg eax` normalises the result; a plain
        # forward does not, and emitting one would change the return value.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("call", "0x600320"),
                ("neg", "eax"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_a_body_with_no_call_at_all(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())


class CalleeSaveTest(unittest.TestCase):
    def test_accepts_a_saved_register_that_is_restored(self):
        forward = delegates.analyze_forward(flat(
            ("push", "esi"),
            ("mov", "esi, dword ptr [esp + 8]"),
            ("push", "esi"),
            ("call", "0x429180"),
            ("pop", "esi"),
            ("ret", ""),
        ), row())
        self.assertEqual("flat-forward", forward.form)
        self.assertEqual(["param:0"],
                         [a.describe() for a in forward.arguments])

    def test_slot_arithmetic_tracks_the_push_depth(self):
        # After two saves, [esp+0xc] is parameter 0 - the arithmetic Ghidra's
        # output silently gets wrong, and the reason depth is tracked at all.
        forward = delegates.analyze_forward(flat(
            ("push", "esi"),
            ("push", "edi"),
            ("mov", "eax, dword ptr [esp + 0xc]"),
            ("push", "eax"),
            ("call", "0x429180"),
            ("pop", "edi"),
            ("pop", "esi"),
            ("ret", ""),
        ), row())
        self.assertEqual(["param:0"],
                         [a.describe() for a in forward.arguments])

    def test_argument_pushes_shift_later_slots_too(self):
        # One save plus one argument already pushed: parameter 1 has moved to
        # [esp+0x10] by the time the second load happens... but loads happen
        # before pushes here, so drive it the other way: load after an
        # argument push must account for that push as well.
        forward = delegates.analyze_forward(flat(
            ("push", "esi"),
            ("mov", "esi, dword ptr [esp + 0xc]"),
            ("push", "esi"),
            ("mov", "eax, dword ptr [esp + 0xc]"),
            ("push", "eax"),
            ("call", "0x429180"),
            ("pop", "esi"),
            ("ret", ""),
        ), row(name="?f@@YAXHH@Z"))
        # First load: depth 1 -> 0xc is param 1. Second: depth 2 -> param 0.
        self.assertEqual(["param:0", "param:1"],
                         [a.describe() for a in forward.arguments])

    def test_rejects_a_save_that_never_comes_back(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(flat(
                ("push", "esi"),
                ("call", "0x429180"),
                ("ret", ""),
            ), row(name="?f@@YAXXZ"))

    def test_rejects_saves_restored_in_push_order(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(flat(
                ("push", "esi"),
                ("push", "edi"),
                ("call", "0x429180"),
                ("pop", "esi"),
                ("pop", "edi"),
                ("ret", ""),
            ), row(name="?f@@YAXXZ"))


class ThiscallSourceTest(unittest.TestCase):
    def test_this_can_be_pushed_as_an_argument(self):
        # A __thiscall method forwarding itself to a __cdecl helper: ecx is
        # `this` at entry and lands on the stack.
        forward = delegates.analyze_forward(flat(
            ("push", "ecx"),
            ("call", "0x429180"),
            ("ret", ""),
        ), row(name="?m@C@@QAEXXZ"))
        self.assertEqual(["this"], [a.describe() for a in forward.arguments])
        self.assertEqual("this", forward.receiver)

    def test_this_survives_a_register_copy(self):
        forward = delegates.analyze_forward(flat(
            ("mov", "eax, ecx"),
            ("push", "eax"),
            ("call", "0x429180"),
            ("ret", ""),
        ), row(name="?m@C@@QAEXXZ"))
        self.assertEqual(["this"], [a.describe() for a in forward.arguments])

    def test_thiscall_ret_must_match_the_declared_arity(self):
        forward = delegates.analyze_forward(frame(
            ("mov", "eax, dword ptr [ebp + 8]"),
            ("push", "eax"),
            ("call", "0x429180"),
            ("pop", "ebp"),
            ("ret", "4"),
        ), row(name="?m@C@@QAEXH@Z"))
        self.assertEqual(4, forward.popped)
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("push", "eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row(name="?m@C@@QAEXH@Z"))

    def test_cdecl_source_still_rejects_a_bare_ecx_push(self):
        # With no `this`, push ecx at entry pushes garbage; whatever the body
        # is, it is not a forwarder.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(flat(
                ("push", "ecx"),
                ("call", "0x429180"),
                ("ret", ""),
            ), row(name="?f@@YAXXZ"))


class TailAliasTest(unittest.TestCase):
    def test_a_bare_jmp_forwards_every_declared_parameter(self):
        forward = delegates.analyze_forward(flat(
            ("jmp", "0x607040"),
        ), row(name="?m@C@@QAEXHH@Z"))
        self.assertEqual("tail-alias", forward.form)
        self.assertTrue(forward.tail_call)
        self.assertEqual("this", forward.receiver)
        self.assertEqual(["param:0", "param:1"],
                         [a.describe() for a in forward.arguments])

    def test_rejects_an_alias_whose_own_arity_is_unreadable(self):
        # The synthesised call needs the parameter list; without it there is
        # nothing safe to emit.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(flat(
                ("jmp", "0x607040"),
            ), row(name="sub_401000"))

    def test_rejects_a_tail_jump_over_a_live_stack(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(flat(
                ("push", "0x1"),
                ("jmp", "0x607040"),
            ), row(name="?f@@YAXXZ"))


class TargetArityTest(unittest.TestCase):
    def functions(self, name):
        return {0x429180: {"name": name}}

    def test_rejects_a_push_count_the_target_does_not_declare(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("push", "eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row(), self.functions("?g@@YAXHH@Z"))

    def test_rejects_a_cdecl_target_with_no_cleanup(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("push", "eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row(), self.functions("?g@@YAXH@Z"))

    def test_accepts_when_the_numbers_agree(self):
        forward = delegates.analyze_forward(frame(
            ("mov", "eax, dword ptr [ebp + 8]"),
            ("push", "eax"),
            ("call", "0x429180"),
            ("add", "esp, 4"),
            ("pop", "ebp"),
            ("ret", ""),
        ), row(), self.functions("?g@@YAXH@Z"))
        self.assertEqual(4, forward.caller_cleanup)


class StackSlotTest(unittest.TestCase):
    def test_maps_frame_displacements_to_parameter_indices(self):
        self.assertEqual(
            0, delegates.parse_stack_slot("dword ptr [ebp + 8]", True, 0))
        self.assertEqual(
            1, delegates.parse_stack_slot("dword ptr [ebp + 0xc]", True, 0))

    def test_rejects_frame_locals_and_saved_slots(self):
        self.assertIsNone(
            delegates.parse_stack_slot("dword ptr [ebp + 4]", True, 0))
        self.assertIsNone(
            delegates.parse_stack_slot("dword ptr [ebp - 8]", True, 0))

    def test_flat_slots_shift_with_the_depth(self):
        self.assertEqual(
            0, delegates.parse_stack_slot("dword ptr [esp + 4]", False, 0))
        self.assertEqual(
            0, delegates.parse_stack_slot("dword ptr [esp + 8]", False, 1))
        self.assertEqual(
            1, delegates.parse_stack_slot("dword ptr [esp + 0xc]", False, 1))

    def test_flat_slots_below_the_parameters_are_not_parameters(self):
        # [esp] is the return address; anything at or under the tracked depth
        # is stack the function pushed itself.
        self.assertIsNone(
            delegates.parse_stack_slot("dword ptr [esp]", False, 0))
        self.assertIsNone(
            delegates.parse_stack_slot("dword ptr [esp + 4]", False, 1))


class VtableTailTest(unittest.TestCase):
    def test_accepts_republished_vtables_then_a_tail_jump(self):
        forward = delegates.analyze_vtable_tail([
            FakeInstruction("mov", "dword ptr [ecx], 0x670c60"),
            FakeInstruction("mov", "dword ptr [ecx + 0x444], 0x670c58"),
            FakeInstruction("jmp", "0x607040"),
        ], row(name="??1PushButton@@QAE@XZ"))
        self.assertEqual("vtable-tail", forward.form)
        self.assertEqual(0x607040, forward.target)
        self.assertEqual([(0, 0x670C60), (0x444, 0x670C58)],
                         forward.vtable_writes)
        # `this` reaches the base destructor untouched.
        self.assertEqual("this", forward.receiver)

    def test_rejects_a_jump_that_is_not_last(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_vtable_tail([
                FakeInstruction("jmp", "0x607040"),
                FakeInstruction("mov", "dword ptr [ecx], 0x670c60"),
            ], row())

    def test_rejects_a_non_literal_vtable_store(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_vtable_tail([
                FakeInstruction("mov", "dword ptr [ecx], eax"),
                FakeInstruction("jmp", "0x607040"),
            ], row())


class GlobalInitTest(unittest.TestCase):
    def test_accepts_the_scalar_form_with_exit_registration(self):
        forward = delegates.analyze_global_init([
            FakeInstruction("mov", "ecx, 0x6a7090"),
            FakeInstruction("call", "0x4c66e0"),
            FakeInstruction("push", "0x402f30"),
            FakeInstruction("call", "0x645398"),
            FakeInstruction("pop", "ecx"),
            FakeInstruction("ret", ""),
        ], row(name="??__Eg_ALPHAMENU_WAVE@@YAXXZ"))
        self.assertEqual("global-init", forward.form)
        self.assertEqual(0x4C66E0, forward.target)
        self.assertEqual("global:0x006A7090", forward.receiver)
        self.assertEqual([(-1, 0x402F30)], forward.vtable_writes)

    def test_accepts_a_constructor_argument(self):
        forward = delegates.analyze_global_init([
            FakeInstruction("push", "1"),
            FakeInstruction("mov", "ecx, 0x7d4060"),
            FakeInstruction("call", "0x4626e0"),
            FakeInstruction("push", "0x4620a0"),
            FakeInstruction("call", "0x645398"),
            FakeInstruction("pop", "ecx"),
            FakeInstruction("ret", ""),
        ], row(name="??__Eg_MAPWIN@@YAXXZ"))
        self.assertEqual(["imm:0x1"],
                         [a.describe() for a in forward.arguments])

    def test_accepts_the_tail_jump_form_that_registers_nothing(self):
        forward = delegates.analyze_global_init([
            FakeInstruction("mov", "ecx, 0x915068"),
            FakeInstruction("jmp", "0x5d7410"),
        ], row(name="??__Eg_BOOM_BUFFER1@@YAXXZ"))
        self.assertTrue(forward.tail_call)
        self.assertEqual([], forward.vtable_writes)

    def test_accepts_the_array_form(self):
        forward = delegates.analyze_global_init([
            FakeInstruction("push", "0x4c67c0"),
            FakeInstruction("push", "0x4c66e0"),
            FakeInstruction("push", "0x2d"),
            FakeInstruction("push", "0x6c"),
            FakeInstruction("push", "0x74c5f0"),
            FakeInstruction("call", "0x6457c2"),
            FakeInstruction("push", "0x445480"),
            FakeInstruction("call", "0x645398"),
            FakeInstruction("pop", "ecx"),
            FakeInstruction("ret", ""),
        ], row(name="??__Eg_CPU_WAVES@@YAXXZ"))
        self.assertEqual(0x6457C2, forward.target)
        self.assertEqual(
            ["imm:0x74C5F0", "imm:0x6C", "imm:0x2D", "imm:0x4C66E0",
             "imm:0x4C67C0"],
            [a.describe() for a in forward.arguments])

    def test_rejects_a_second_call_with_nothing_registered(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_global_init([
                FakeInstruction("mov", "ecx, 0x6a7090"),
                FakeInstruction("call", "0x4c66e0"),
                FakeInstruction("call", "0x645398"),
                FakeInstruction("ret", ""),
            ], row())

    def test_rejects_an_initializer_that_pops_arguments(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_global_init([
                FakeInstruction("mov", "ecx, 0x6a7090"),
                FakeInstruction("call", "0x4c66e0"),
                FakeInstruction("ret", "4"),
            ], row())


class PaddingTest(unittest.TestCase):
    def test_strips_trailing_alignment(self):
        instructions = [FakeInstruction("ret", ""), FakeInstruction("nop", ""),
                        FakeInstruction("nop", "")]
        self.assertEqual(1, len(delegates.trailing_padding(instructions)))

    def test_keeps_an_empty_result_empty(self):
        self.assertEqual([], delegates.trailing_padding(
            [FakeInstruction("nop", "")]))


if __name__ == "__main__":
    unittest.main()
