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


class FrameForwardTest(unittest.TestCase):
    def test_accepts_a_parameter_forward_with_literal_padding(self):
        # The synch_veh shape: one incoming parameter placed among literals.
        forward = delegates.analyze_frame_forward(frame(
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
        forward = delegates.analyze_frame_forward(frame(
            ("mov", "eax, dword ptr [ebp + 0xc]"),
            ("mov", "ecx, dword ptr [ebp + 8]"),
            ("push", "eax"),
            ("push", "ecx"),
            ("call", "0x429180"),
            ("pop", "ebp"),
            ("ret", ""),
        ), row())
        self.assertEqual(["param:0", "param:1"],
                         [a.describe() for a in forward.arguments])
        # ecx held a parameter here, not a fixed global, so it is an argument
        # and the receiver is that parameter rather than a singleton.
        self.assertEqual("param:0", forward.receiver)

    def test_records_the_callee_pop_count(self):
        forward = delegates.analyze_frame_forward(frame(
            ("mov", "eax, dword ptr [ebp + 8]"),
            ("push", "eax"),
            ("call", "0x429180"),
            ("pop", "ebp"),
            ("ret", "4"),
        ), row())
        self.assertEqual(4, forward.popped)

    def test_rejects_a_conditional_branch(self):
        # A guard is not a forward; accepting one would drop the guard.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_frame_forward(frame(
                ("test", "eax, eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_a_store_the_forward_would_lose(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_frame_forward(frame(
                ("mov", "dword ptr [ecx + 4], eax"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_a_push_of_a_register_never_loaded(self):
        # `push esi` at entry is a callee-save, not an argument. Treating it as
        # one would invent a parameter that does not exist.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_frame_forward(frame(
                ("push", "esi"),
                ("call", "0x429180"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_an_indirect_call(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_frame_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("call", "dword ptr [eax + 0xe8]"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_work_after_the_call(self):
        # `neg eax / sbb eax, eax / neg eax` normalises the result; a plain
        # forward does not, and emitting one would change the return value.
        with self.assertRaises(delegates.Reject):
            delegates.analyze_frame_forward(frame(
                ("call", "0x600320"),
                ("neg", "eax"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_a_body_with_no_call_at_all(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_frame_forward(frame(
                ("mov", "eax, dword ptr [ebp + 8]"),
                ("pop", "ebp"),
                ("ret", ""),
            ), row())

    def test_rejects_a_missing_prologue(self):
        with self.assertRaises(delegates.Reject):
            delegates.analyze_frame_forward(
                [FakeInstruction("call", "0x429180"),
                 FakeInstruction("ret", ""),
                 FakeInstruction("nop", ""),
                 FakeInstruction("nop", "")], row())


class FrameSlotTest(unittest.TestCase):
    def test_maps_displacements_to_parameter_indices(self):
        self.assertEqual(0, delegates.parse_frame_slot("dword ptr [ebp + 8]"))
        self.assertEqual(1, delegates.parse_frame_slot("dword ptr [ebp + 0xc]"))
        self.assertEqual(2, delegates.parse_frame_slot("dword ptr [ebp + 0x10]"))

    def test_rejects_locals_and_saved_slots(self):
        # [ebp+0] is the saved ebp and [ebp+4] the return address; negative
        # displacements are locals. None of them are parameters.
        self.assertIsNone(delegates.parse_frame_slot("dword ptr [ebp + 4]"))
        self.assertIsNone(delegates.parse_frame_slot("dword ptr [ebp - 8]"))

    def test_rejects_a_misaligned_displacement(self):
        self.assertIsNone(delegates.parse_frame_slot("dword ptr [ebp + 0xa]"))


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
        self.assertEqual("param:0", forward.receiver)

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
        # The array base, element size and count are the leading arguments.
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
        # A dynamic initializer takes none, so a callee-pop means this is
        # something else wearing the same name shape.
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
