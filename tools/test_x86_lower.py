#!/usr/bin/env python3
"""Unit tests for the instruction lowerer.

These test the Python that emits C++, not the C++ runtime the emission calls
into. That split is deliberate: a runtime bug shows up as a wrong answer in one
game function, but a lowerer bug shows up 724,814 times, identically, and every
one of them looks like plausible code.

What is worth asserting here is the handful of places where wrong output is
still *well-formed* output - where the emitted C++ compiles, runs, and quietly
means something else:

* A sub-register write is a merge. `mov al, 0` lowering to `eax = 0` compiles,
  reads correctly if you only ever look at AL, and destroys three bytes of an
  unrelated live value. The tests therefore assert the whole merge expression,
  mask included, not just that AL appears somewhere in it.
* AH/BH/CH/DH live at bits 8..15, so they need a shift on *both* sides. A test
  that only exercises AL cannot tell a correct model from one that treats every
  8-bit name as a low byte.
* LEA is the one memory-form instruction that must not dereference. "It emits
  an assignment" is true of both the right answer and the wrong one, so the
  test asserts the absence of `opensmacx_mem`.
* An 8-bit immediate widened to 32 bits must be sign-extended. `add eax, -1`
  and `add eax, 0xff` are different instructions that differ by one bit in the
  emitted constant.
* PUSH evaluates its source before the stack pointer moves and POP writes its
  destination after; `push esp` and `pop esp` are the two instructions that can
  tell the difference, and both are legal.
* cmp and test are the arithmetic instructions that must not write anything.
  Emitting a write is invisible until some later read of the destination.
* A branch inside the function is a goto and a branch out of it is a dispatch,
  so both directions are driven through a `label_for` that answers for one
  address and refuses another.
* A refusal has to name the instruction that caused it, because the caller
  turns the message into a trap and "register id 50" identifies nothing.
"""
import types
import unittest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone import x86 as capstone_x86

import x86_lower

# Somewhere in the image's text section; the exact value only matters because
# refusals and call return addresses are expected to quote it.
BASE = 0x00401000

DECODER = Cs(CS_ARCH_X86, CS_MODE_32)
DECODER.detail = True


def decode(encoded, address=BASE):
    """The single instruction encoded by `encoded`, a hex string."""
    instructions = list(DECODER.disasm(bytes.fromhex(encoded), address))
    if not instructions:
        raise AssertionError(f"{encoded} did not decode")
    return instructions[0]


def fake_x87(mnemonic, st_indices):
    """An x87 instruction capstone will not produce, for the refusal paths.

    Capstone's `X86Op` has no setters, so a shape it never emits cannot be
    made by mutating a decoded one. Only the fields the lowering reads are
    provided; anything else it touches should be a loud AttributeError rather
    than a plausible default.
    """
    operands = [types.SimpleNamespace(
        type=x86_lower.X86_OP_REG,
        reg=getattr(capstone_x86, f"X86_REG_ST{index}"),
        size=10) for index in st_indices]
    return types.SimpleNamespace(
        mnemonic=mnemonic, op_str="(hand-built)", address=BASE, size=2,
        operands=operands, opcode=(0xDC, 0, 0, 0), prefix=(0, 0, 0, 0),
        bytes=b"\xdc\xe1")


def no_labels(_address):
    """A `label_for` for functions with no branch targets of their own."""
    return None


def lower(encoded, label_for=no_labels, address=BASE, case_targets=None):
    return x86_lower.lower(decode(encoded, address), label_for, case_targets)


def lower_one(encoded, label_for=no_labels, address=BASE):
    statements = lower(encoded, label_for, address)
    if len(statements) != 1:
        raise AssertionError(f"expected one statement, got {statements}")
    return statements[0]


def joined(encoded, label_for=no_labels, address=BASE):
    return "\n".join(lower(encoded, label_for, address))


class SubRegisterWriteTests(unittest.TestCase):
    """Writing part of a register must preserve the rest of it."""

    def test_writing_al_merges_instead_of_assigning(self):
        # mov al, 0 - the top 24 bits of EAX survive.
        self.assertEqual(
            "s.eax = (s.eax & 0xffffff00U) | ((0x00000000U << 0)"
            " & 0x000000ffU);",
            lower_one("b000"))

    def test_writing_al_never_assigns_the_whole_register(self):
        # The failure mode this whole class exists for: `s.eax = 0x0...0U;`
        # is a legal-looking lowering of `mov al, 0` and is catastrophic.
        self.assertNotIn("s.eax = 0x", lower_one("b000"))

    def test_writing_ax_preserves_the_upper_half(self):
        # mov ax, 0
        self.assertEqual(
            "s.eax = (s.eax & 0xffff0000U) | ((0x00000000U << 0)"
            " & 0x0000ffffU);",
            lower_one("66b80000"))

    def test_writing_eax_replaces_the_register(self):
        # mov eax, 0 - a 32-bit write is the one that does not merge.
        self.assertEqual("s.eax = 0x00000000U;", lower_one("b800000000"))

    def test_writing_ah_shifts_into_bits_8_to_15(self):
        # mov ah, 1 - the value is shifted up and the hole is bits 8..15.
        self.assertEqual(
            "s.eax = (s.eax & 0xffff00ffU) | ((0x00000001U << 8)"
            " & 0x0000ff00U);",
            lower_one("b401"))

    def test_reading_ah_shifts_back_down(self):
        # mov al, ah - one instruction that needs the shift on both sides at
        # once, so a model that treats AH as a low byte cannot fake it.
        self.assertEqual(
            "s.eax = (s.eax & 0xffffff00U) | ((((s.eax >> 8) & 0xffU) << 0)"
            " & 0x000000ffU);",
            lower_one("8ac4"))

    def test_high_bytes_of_other_registers_use_their_own_field(self):
        # mov ch, dh - neither operand is in EAX, and both are high bytes.
        self.assertEqual(
            "s.ecx = (s.ecx & 0xffff00ffU) | ((((s.edx >> 8) & 0xffU) << 8)"
            " & 0x0000ff00U);",
            lower_one("8aee"))

    def test_a_low_byte_of_another_register_targets_that_register(self):
        # mov bl, 1 - the field is EBX, not EAX.
        self.assertEqual(
            "s.ebx = (s.ebx & 0xffffff00U) | ((0x00000001U << 0)"
            " & 0x000000ffU);",
            lower_one("b301"))

    def test_an_arithmetic_result_is_merged_too(self):
        # add al, 1 - the merge applies to computed values, not just to
        # immediates, so the ALU path cannot skip it.
        self.assertEqual(
            "s.eax = (s.eax & 0xffffff00U)"
            " | ((opensmacx_add8(s, (s.eax & 0xffU), 0x00000001U) << 0)"
            " & 0x000000ffU);",
            lower_one("0401"))


class LoadEffectiveAddressTests(unittest.TestCase):
    """LEA computes an address; every other memory form dereferences one."""

    def test_lea_does_not_dereference(self):
        # lea eax, [ebx + 4]
        self.assertNotIn("opensmacx_mem", lower_one("8d4304"))

    def test_lea_emits_the_address_arithmetic(self):
        self.assertEqual("s.eax = s.ebx + 0x00000004U;", lower_one("8d4304"))

    def test_lea_keeps_the_index_scale(self):
        # lea eax, [ecx*4 + 4] - the scaled-index form is how the image does
        # array arithmetic, and it must survive without a load.
        statement = lower_one("8d048d04000000")
        self.assertEqual("s.eax = (s.ecx * 4U) + 0x00000004U;", statement)
        self.assertNotIn("opensmacx_mem", statement)

    def test_the_same_address_under_mov_does_dereference(self):
        # mov eax, [ebx + 4] - identical operand shape, opposite treatment.
        # Without this contrast the LEA test above proves nothing.
        self.assertEqual("s.eax = opensmacx_mem32(s.ebx + 0x00000004U);",
                         lower_one("8b4304"))

    def test_narrow_loads_pick_the_matching_accessor(self):
        # movzx eax, byte ptr [eax] loads a byte, not a dword.
        self.assertEqual("s.eax = opensmacx_mem8(s.eax);", lower_one("0fb600"))


class ImmediateWidthTests(unittest.TestCase):
    """A narrow immediate widened to the operand size is signed."""

    def test_a_negative_byte_immediate_reaches_32_bits_as_negative(self):
        # add eax, -1, encoded as the sign-extended imm8 form. Widening it as
        # unsigned would make it `add eax, 255`.
        statement = lower_one("83c0ff")
        self.assertEqual("s.eax = opensmacx_add32(s, s.eax, 0xffffffffU);",
                         statement)
        self.assertNotIn("0x000000ffU", statement)

    def test_a_positive_byte_immediate_is_unchanged(self):
        # add ecx, 1 - the same encoding form, to show the widening is a sign
        # extension and not a blanket set of the high bits.
        self.assertEqual("s.ecx = opensmacx_add32(s, s.ecx, 0x00000001U);",
                         lower_one("83c101"))

    def test_a_byte_operation_keeps_a_byte_immediate(self):
        # cmp al, 0xff - here 0xff really is 255, because the operation is
        # 8-bit. Sign extension must not leak into the narrow case.
        self.assertEqual(
            "(void)opensmacx_sub8(s, (s.eax & 0xffU), 0x000000ffU);",
            lower_one("80f8ff"))

    def test_two_encodings_of_the_same_immediate_lower_alike(self):
        # cmp ax, -1 as the sign-extended imm8 form and as the full imm16 form.
        # They are one instruction with two spellings, so a 16-bit helper must
        # not be able to tell them apart by their right operand.
        self.assertEqual(lower_one("6681f8ffff"), lower_one("6683f8ff"))

    def test_a_word_immediate_stays_within_the_operand_width(self):
        self.assertEqual(
            "(void)opensmacx_sub16(s, (s.eax & 0xffffU), 0x0000ffffU);",
            lower_one("6683f8ff"))

    def test_movsx_sign_extends_through_the_source_width(self):
        # movsx eax, al
        self.assertEqual(
            "s.eax = static_cast<uint32_t>(static_cast<int32_t>("
            "static_cast<int8_t>((s.eax & 0xffU))));",
            lower_one("0fbec0"))

    def test_movzx_does_not_sign_extend(self):
        # movzx eax, al - the mask in the read is the whole extension.
        statement = lower_one("0fb6c0")
        self.assertEqual("s.eax = (s.eax & 0xffU);", statement)
        self.assertNotIn("int8_t", statement)

    def test_cwde_sign_extends_the_low_half_into_all_of_eax(self):
        self.assertEqual(
            "s.eax = static_cast<uint32_t>(static_cast<int32_t>("
            "static_cast<int16_t>((s.eax & 0xffffU))));",
            lower_one("98"))


class StackTests(unittest.TestCase):
    """PUSH and POP move ESP; when they move it decides what they see."""

    def test_push_writes_below_the_stack_pointer_and_then_moves_it(self):
        # push eax
        self.assertEqual(
            ["opensmacx_store32(s.esp - 4U, s.eax);", "s.esp -= 4U;"],
            lower("50"))

    def test_push_esp_stores_the_pointer_it_had_on_entry(self):
        # push esp is legal and pushes the pre-decrement value. Decrementing
        # first and then reading ESP as the source stores a value four lower
        # than the hardware does - and it still compiles, still runs, and only
        # diverges inside whatever reads the slot back.
        statements = lower("54")
        self.assertNotIn("opensmacx_store32(s.esp, s.esp);", statements)
        self.assertEqual("opensmacx_store32(s.esp - 4U, s.esp);",
                         statements[0])

    def test_push_of_a_stack_relative_slot_reads_the_pre_push_address(self):
        # push dword ptr [esp + 4] - the source address is computed with the
        # old ESP, so the emitted read must not follow a decrement.
        statements = lower("ff742404")
        self.assertIn("opensmacx_mem32(s.esp + 0x00000004U)", statements[0])
        self.assertEqual("s.esp -= 4U;", statements[-1])

    def test_pop_reads_the_slot_before_moving_the_pointer(self):
        text = joined("58")
        self.assertLess(text.index("opensmacx_mem32(s.esp)"),
                        text.index("s.esp += 4U;"))

    def test_pop_esp_loads_the_stored_value_not_the_incremented_pointer(self):
        # pop esp - the destination is the pointer being adjusted, so a
        # lowering that increments first overwrites the loaded value with
        # garbage that happens to look like a plausible stack pointer.
        text = joined("5c")
        self.assertIn("s.esp = popped;", text)
        self.assertNotIn("s.esp = s.esp", text)

    def test_a_narrow_push_moves_the_pointer_by_its_own_width(self):
        # push ax - two bytes, not four. A four-byte lowering writes two bytes
        # of an unrelated register into the slot AND leaves ESP two below where
        # the hardware left it, so every later stack access is off by two.
        self.assertEqual(
            ["opensmacx_store16(s.esp - 2U, (s.eax & 0xffffU));",
             "s.esp -= 2U;"],
            lower("6650"))

    def test_a_narrow_pop_reads_its_own_width(self):
        # pop ax - reading a dword here loads two bytes past the slot and the
        # merge then discards exactly the half that was correct.
        self.assertEqual(
            ["{ const uint32_t popped = opensmacx_mem16(s.esp);",
             "  s.esp += 2U;",
             "  s.eax = (s.eax & 0xffff0000U) | ((popped << 0)"
             " & 0x0000ffffU); }"],
            lower("6658"))

    def test_a_push_immediate_stays_four_bytes_wide(self):
        # push 0x10 - the immediate encoding is one byte but the push is not,
        # so a width taken from the literal rather than from the operand would
        # narrow it. The contrast with `push ax` is the whole point.
        self.assertEqual(
            ["opensmacx_store32(s.esp - 4U, 0x00000010U);", "s.esp -= 4U;"],
            lower("6a10"))

    def test_call_pushes_the_return_address_not_the_target(self):
        # call +0x10 from BASE: the return address is BASE+5, the target is
        # BASE+0x15. Encoding a call with displacement zero would make the two
        # coincide and prove nothing.
        text = joined("e810000000")
        self.assertIn("opensmacx_store32(s.esp, 0x00401005U);", text)
        self.assertIn("opensmacx_dispatch(0x00401015U)(s);", text)

    def test_an_indirect_call_reads_its_target_before_the_push(self):
        # call dword ptr [esp + 4] - the slot named is relative to the ESP the
        # instruction started with. Decrementing first turns it into [esp+8]
        # after the adjustment: still a plausible pointer, still dispatched,
        # and the call lands in whatever function that other word names.
        text = joined("ff542404")
        self.assertIn("const uint32_t target ="
                      " opensmacx_mem32(s.esp + 0x00000004U);", text)
        self.assertIn("opensmacx_dispatch(target)(s);", text)
        # The read must precede the only statement that moves ESP.
        self.assertLess(text.index("const uint32_t target"),
                        text.index("s.esp -= 4U;"))

    def test_an_indirect_call_still_pushes_the_return_address(self):
        # Materialising the target must not cost the push: the slot written is
        # below the entry ESP and ESP ends four lower, exactly as for a direct
        # call. `call [esp+4]` is four bytes, so the return address is BASE+4.
        text = joined("ff542404")
        self.assertIn("opensmacx_store32(s.esp - 4U, 0x00401004U);", text)
        self.assertIn("s.esp -= 4U;", text)

    def test_a_direct_call_needs_no_temporary(self):
        # A literal target cannot observe ESP, so the simple form stays.
        self.assertNotIn("target", joined("e810000000"))

    def test_ret_releases_the_return_address(self):
        self.assertEqual(["s.esp += 4U;", "return;"], lower("c3"))

    def test_ret_with_an_immediate_also_releases_the_arguments(self):
        # ret 8 - four bytes of return address plus eight of callee-popped
        # arguments.
        self.assertEqual(["s.esp += 12U;", "return;"], lower("c20800"))


class FlagOnlyTests(unittest.TestCase):
    """cmp and test do the arithmetic and throw the result away."""

    def test_cmp_discards_its_result(self):
        self.assertEqual("(void)opensmacx_sub32(s, s.eax, s.ebx);",
                         lower_one("39d8"))

    def test_cmp_does_not_write_the_destination_register(self):
        statement = lower_one("39d8")
        self.assertNotIn("s.eax =", statement)

    def test_cmp_against_memory_does_not_store(self):
        # cmp dword ptr [ebx], eax - the destination is a memory operand, so
        # the mistake here would be a store rather than an assignment.
        statement = lower_one("3903")
        self.assertNotIn("opensmacx_store", statement)
        self.assertEqual(
            "(void)opensmacx_sub32(s, opensmacx_mem32(s.ebx), s.eax);",
            statement)

    def test_test_discards_its_result(self):
        self.assertEqual("(void)opensmacx_and32(s, s.eax, s.eax);",
                         lower_one("85c0"))

    def test_sub_by_contrast_keeps_its_result(self):
        # Same helper as cmp; only the write distinguishes them.
        self.assertEqual("s.eax = opensmacx_sub32(s, s.eax, s.ebx);",
                         lower_one("29d8"))

    def test_and_by_contrast_keeps_its_result(self):
        self.assertEqual("s.eax = opensmacx_and32(s, s.eax, s.eax);",
                         lower_one("21c0"))


class BranchTests(unittest.TestCase):
    """Control staying in the function and control leaving it differ."""

    @staticmethod
    def only(target):
        """A `label_for` that knows one address inside the function."""
        return lambda address: "L_00401007" if address == 0x00401007 else None

    def test_a_conditional_branch_inside_the_function_is_a_goto(self):
        # je BASE+7, with BASE+7 claimed as a local label.
        self.assertEqual("if (opensmacx_zf(s)) goto L_00401007;",
                         lower_one("7405", self.only(0x00401007)))

    def test_a_conditional_branch_outside_the_function_dispatches(self):
        # The same instruction, with the target disowned. The branch has to
        # leave the function entirely, which means a return as well.
        statement = lower_one("7405", no_labels)
        self.assertEqual(
            "if (opensmacx_zf(s)) { opensmacx_dispatch(0x00401007U)(s);"
            " return; }",
            statement)
        self.assertNotIn("goto", statement)

    def test_an_unconditional_jump_inside_the_function_is_a_goto(self):
        # jmp BASE+7
        self.assertEqual(["goto L_00401007;"],
                         lower("eb05", self.only(0x00401007)))

    def test_an_unconditional_jump_outside_the_function_is_a_tail_call(self):
        self.assertEqual(
            ["opensmacx_dispatch(0x00401007U)(s);", "return;"],
            lower("eb05", no_labels))

    def test_the_label_lookup_is_asked_about_the_absolute_target(self):
        # The lookup keys on image addresses, not on the encoded displacement.
        asked = []
        lower("7405", asked.append)
        self.assertEqual([0x00401007], asked)

    def test_an_unsigned_comparison_branch_uses_the_carry_flag(self):
        # jbe BASE+7 - the condition table is the other half of every cmp, and
        # picking the signed reading here is a silent off-by-one-population bug.
        self.assertEqual(
            "if ((opensmacx_cf(s) || opensmacx_zf(s))) goto L_00401007;",
            lower_one("7605", self.only(0x00401007)))

    def test_a_signed_comparison_branch_uses_sign_against_overflow(self):
        # jle BASE+7
        self.assertEqual(
            "if ((opensmacx_zf(s) || opensmacx_sf(s) != opensmacx_of(s)))"
            " goto L_00401007;",
            lower_one("7e05", self.only(0x00401007)))

    def test_an_indirect_jump_dispatches_on_the_loaded_value(self):
        # jmp dword ptr [eax*4 + 0x401000] - a switch table. There is no
        # immediate to hand to `label_for`, so it must dispatch.
        self.assertEqual(
            ["opensmacx_dispatch(opensmacx_mem32((s.eax * 4U)"
             " + 0x00401000U))(s);", "return;"],
            lower("ff248500104000"))


class RefusalTests(unittest.TestCase):
    """A refusal is turned into a trap, so it has to say which instruction."""

    def test_an_unknown_mnemonic_is_refused(self):
        # bswap eax - no lowering, and no pretending there is one. It stands in
        # for `div ecx`, which used to be the example here and now lowers.
        with self.assertRaises(x86_lower.Unsupported):
            lower("0fc8")

    def test_the_refusal_names_the_address(self):
        with self.assertRaisesRegex(x86_lower.Unsupported, "0x00401000"):
            lower("0fc8")

    def test_the_refusal_names_the_instruction(self):
        with self.assertRaisesRegex(x86_lower.Unsupported, "bswap"):
            lower("0fc8")

    def test_the_address_in_the_refusal_is_the_instruction_s_own(self):
        # Two instructions, one message each: the trap has to point at the
        # right one, not at the start of the function.
        with self.assertRaisesRegex(x86_lower.Unsupported, "0x00402340"):
            lower("0fc8", no_labels, 0x00402340)

    def test_a_non_fs_segment_is_still_refused(self):
        # mov eax, gs:[0]. Only fs: is modelled, because only fs: appears -
        # a gs: operand would be a different thread block on a different
        # platform, and lowering it into the fs: one would silently merge two
        # address spaces.
        with self.assertRaisesRegex(x86_lower.Unsupported, "segment"):
            lower("65a100000000")

    def test_the_segment_refusal_also_names_the_address(self):
        # The refusal comes from inside the operand model rather than from the
        # mnemonic table, and it has to be just as identifiable.
        with self.assertRaisesRegex(x86_lower.Unsupported, "0x00401000"):
            lower("65a100000000")

    def test_fs_relative_memory_with_a_register_term_is_refused(self):
        # mov eax, fs:[ecx]. A COMPUTED thread-block offset: the displacement
        # is no longer the whole address, so the constant-displacement helper
        # would index the block with whatever ECX held.
        with self.assertRaisesRegex(x86_lower.Unsupported, "register term"):
            lower("648b01")

    def test_lea_of_an_fs_operand_is_still_refused(self):
        # lea eax, fs:[0x18]. LEA wants a FLAT address and the thread block has
        # none, so the routing in read_operand must not be mistaken for
        # making fs: generally addressable.
        with self.assertRaisesRegex(x86_lower.Unsupported, "flat address"):
            lower("648d0518000000")

    def test_sixteen_bit_addressing_is_refused(self):
        # mov eax, [bx + si] under a 0x67 prefix. The 16-bit address wraps
        # within 64K and is taken from a segment base, so lowering it as
        # `(s.ebx & 0xffff) + (s.esi & 0xffff)` is right only while the sum
        # stays under 0x10000 - it compiles, and it addresses the flat image.
        with self.assertRaisesRegex(x86_lower.Unsupported, "16-bit address"):
            lower("678b00")

    def test_a_sixteen_bit_index_alone_is_refused(self):
        # [bx + di] - the refusal must key on either half-width component, not
        # only on the base.
        with self.assertRaisesRegex(x86_lower.Unsupported, "16-bit address"):
            lower("678b4100")

    def test_the_thirty_two_bit_form_of_the_same_shape_still_lowers(self):
        # mov eax, [ebx + esi] - without the prefix this is ordinary, so the
        # refusal above is about the address size and not about base+index.
        self.assertEqual("s.eax = opensmacx_mem32(s.ebx + s.esi);",
                         lower_one("8b0433"))

    def test_a_control_register_is_refused_by_address_too(self):
        # mov eax, cr0 - refused by the register table, which on its own only
        # knows a numeric register id.
        with self.assertRaisesRegex(x86_lower.Unsupported, "0x00401000"):
            lower("0f20c0")

    def test_an_unmodelled_x87_form_is_refused(self):
        # fbld tbyte ptr [eax] - packed BCD, which occurs zero times in this
        # image. The `f`-prefix dispatch must not swallow it into a plausible
        # neighbour just because the mnemonic starts with an f.
        with self.assertRaises(x86_lower.Unsupported):
            lower("df20")

    def test_the_sse_movsd_is_not_a_string_copy(self):
        # f2 0f 10 05 - `movsd xmm0, [0x11223344]` shares its MNEMONIC with the
        # dword string move and nothing else. The opcode is what separates
        # them, which is why the string rule tests the opcode.
        with self.assertRaises(x86_lower.Unsupported):
            lower("f20f100544332211")

    def test_a_16_bit_address_size_string_op_is_refused(self):
        # 67 f3 ab - `rep stosd` over DI and CX, which wraps within 64K.
        with self.assertRaises(x86_lower.Unsupported):
            lower("67f3ab")

    def test_a_segment_overridden_string_source_is_refused(self):
        # 65 f3 a4 - `rep movsb` reading gs:[esi], the thread block.
        with self.assertRaises(x86_lower.Unsupported):
            lower("65f3a4")


class MiscellaneousTests(unittest.TestCase):
    def test_nop_emits_nothing(self):
        self.assertEqual([], lower("90"))

    def test_inc_is_not_lowered_as_an_add(self):
        # INC preserves the carry flag and ADD does not, so INC needs its own
        # helper however tempting `add 1` looks.
        statement = lower_one("40")
        self.assertEqual("s.eax = opensmacx_inc32(s, s.eax);", statement)
        self.assertNotIn("add", statement)

    def test_xchg_uses_a_temporary_so_neither_side_is_lost(self):
        self.assertEqual(
            ["{ const uint32_t swap = s.eax;",
             "  s.eax = s.ebx;",
             "  s.ebx = swap; }"],
            lower("87d8"))

    def test_the_candidate_set_covers_the_mnemonics_that_lower(self):
        for encoded, mnemonic in (("90", "nop"), ("39d8", "cmp"),
                                  ("8d4304", "lea"), ("c3", "ret"),
                                  ("87d8", "xchg"), ("99", "cdq")):
            with self.subTest(mnemonic=mnemonic):
                self.assertIn(mnemonic, x86_lower.candidate_mnemonics())
                lower(encoded)
        self.assertNotIn("bswap", x86_lower.candidate_mnemonics())

    def test_the_candidate_set_is_an_upper_bound_not_coverage(self):
        # `mov` is in the set and these three `mov`s are all refused, so any
        # coverage figure computed by counting instructions whose mnemonic is
        # in the set overstates the lift by however many of these the image
        # holds - and no test of the set alone can detect that, because the
        # set is keyed on the mnemonic and the refusals are about operands.
        # The measured number has to come from calling `lower`.
        self.assertIn("mov", x86_lower.candidate_mnemonics())
        for encoded, why in (("65a100000000", "gs segment override"),
                             ("678b00", "16-bit addressing"),
                             ("0f20c0", "control register")):
            with self.subTest(why=why):
                self.assertEqual("mov", decode(encoded).mnemonic)
                with self.assertRaises(x86_lower.Unsupported):
                    lower(encoded)



class MultiplyDivideTests(unittest.TestCase):
    """The one-operand forms, whose register pair is implicit.

    Capstone reports exactly one operand for these, the explicit source; AX /
    DX:AX / EDX:EAX never appear in the operand list, so the lowering cannot
    read them off the instruction and the helper owns them instead. That is
    why these emit a bare statement rather than a `write_operand`.
    """

    def test_mul_passes_only_the_explicit_source(self):
        # mul ecx
        self.assertEqual(["opensmacx_mul1_32(s, s.ecx);"], lower("f7e1"))

    def test_mul_takes_its_width_from_the_operand(self):
        # mul cl / mul cx - the widths that decide which register pair moves.
        self.assertEqual(["opensmacx_mul1_8(s, (s.ecx & 0xffU));"],
                         lower("f6e1"))
        self.assertEqual(["opensmacx_mul1_16(s, (s.ecx & 0xffffU));"],
                         lower("66f7e1"))

    def test_div_carries_the_instruction_address(self):
        # div ecx - a divide can fault, and the trap has to name the divide,
        # not the function, so the address is an argument.
        self.assertEqual(["opensmacx_div1_32(s, s.ecx, 0x00401000U);"],
                         lower("f7f1"))

    def test_idiv_is_a_different_helper_not_a_flag(self):
        # idiv ecx - signed and unsigned divide disagree about the quotient
        # range and about the sign of the remainder, so one helper taking a
        # boolean would be two functions wearing a coat.
        self.assertEqual(["opensmacx_idiv1_32(s, s.ecx, 0x00401000U);"],
                         lower("f7f9"))

    def test_a_memory_source_still_goes_through_the_operand_model(self):
        # idiv dword ptr [0x691e70] - 256 of the image's idivs are this shape.
        self.assertEqual(
            ["opensmacx_idiv1_32(s, opensmacx_mem32(0x00691e70U),"
             " 0x00401000U);"],
            lower("f73d701e6900"))

    def test_the_address_is_the_instruction_s_own(self):
        # A divide part-way into a body must name where it actually is.
        self.assertEqual(["opensmacx_idiv1_32(s, s.ecx, 0x00402340U);"],
                         lower("f7f9", no_labels, 0x00402340))

    def test_the_candidate_set_admits_all_three(self):
        for mnemonic in ("mul", "div", "idiv"):
            with self.subTest(mnemonic=mnemonic):
                self.assertIn(mnemonic, x86_lower.candidate_mnemonics())


class SegmentRelativeTests(unittest.TestCase):
    """fs: is a separate address space, so it gets separate accessors.

    Every one of the image's 1,330 segment-prefixed operands is a bare
    `fs:[0]`, so the shape asserted here is not a sample - it is the whole
    surface. What makes a wrong lowering dangerous is that it is still
    well-formed: `opensmacx_mem32(0)` compiles, and reads four megabytes below
    the image array.
    """

    def test_reading_fs_goes_to_the_thread_block_not_the_image(self):
        # mov eax, fs:[0]
        self.assertEqual(["s.eax = opensmacx_fs32(0x00000000U);"],
                         lower("64a100000000"))

    def test_writing_fs_goes_to_the_thread_block_not_the_image(self):
        # mov fs:[0], esp - the install half of the SEH prologue.
        self.assertEqual(
            ["opensmacx_store_fs32(0x00000000U, s.esp);"],
            lower("64892500000000"))

    def test_the_displacement_is_the_whole_address(self):
        # mov eax, fs:[0x2c] - the TLS array. The displacement is passed
        # through unchanged rather than added to any base, because the block
        # has no base in the flat image.
        self.assertEqual(["s.eax = opensmacx_fs32(0x0000002cU);"],
                         lower("64a12c000000"))

    def test_no_image_helper_appears_in_an_fs_lowering(self):
        # The failure mode is a lowering that is right about the value and
        # wrong about the space, so assert the space explicitly.
        for encoded in ("64a100000000", "64892500000000"):
            with self.subTest(encoded=encoded):
                emitted = " ".join(lower(encoded))
                self.assertNotIn("opensmacx_mem", emitted)
                self.assertNotIn("opensmacx_store32", emitted)


class StringTests(unittest.TestCase):
    def test_a_repeated_string_instruction_is_one_call(self):
        # ESI, EDI and ECX are implicit and the direction comes from DF, so
        # there is nothing to lower but the name.
        self.assertEqual(["opensmacx_rep_movs8(s);"], lower("f3a4"))

    def test_the_mnemonic_suffix_selects_the_element_width(self):
        self.assertEqual(["opensmacx_rep_stos8(s);"], lower("f3aa"))
        self.assertEqual(["opensmacx_rep_stos16(s);"], lower("66f3ab"))
        self.assertEqual(["opensmacx_rep_stos32(s);"], lower("f3ab"))

    def test_repe_and_repne_keep_their_prefixes(self):
        # f3/ae and f2/ae differ only in which way ZF ends the loop; collapsing
        # them to one helper would run a failed search to ECX == 0.
        self.assertEqual(["opensmacx_repe_scas8(s);"], lower("f3ae"))
        self.assertEqual(["opensmacx_repne_scas8(s);"], lower("f2ae"))

    def test_an_unrepeated_string_instruction_has_no_prefix_in_its_name(self):
        self.assertEqual(["opensmacx_movs8(s);"], lower("a4"))
        self.assertEqual(["opensmacx_lods8(s);"], lower("ac"))

    def test_f2_on_a_moving_op_is_a_repeat_not_a_repne(self):
        # `f2 a5` is the case the printed name cannot answer. Capstone reports
        # it as a BARE "movsd" with the prefix array cleared - it consumes the
        # F2 and does not report it, because the mnemonic collides with the SSE
        # movsd - so a lowering that reads the repeat off the name copies four
        # bytes where the hardware copies the whole buffer, and with ECX = 0
        # writes memory the original never touches. On MOVS/STOS/LODS the
        # hardware treats F2 exactly as REP, which is what this asserts.
        self.assertEqual(["opensmacx_rep_movs32(s);"], lower("f2a5"))
        self.assertEqual(["opensmacx_rep_movs8(s);"], lower("f2a4"))
        self.assertEqual(["opensmacx_rep_stos32(s);"], lower("f2ab"))
        self.assertEqual(["opensmacx_rep_lods32(s);"], lower("f2ad"))
        self.assertEqual(["opensmacx_rep_movs16(s);"], lower("66f2a5"))

    def test_f2_on_a_comparing_op_really_is_repne(self):
        # The other half of the same rule: on SCAS and CMPS the two prefixes
        # are genuinely different loops, so the encoding must not be collapsed
        # there.
        self.assertEqual(["opensmacx_repne_cmps8(s);"], lower("f2a6"))
        self.assertEqual(["opensmacx_repe_cmps8(s);"], lower("f3a6"))

    def test_cld_and_std_write_the_direction_flag(self):
        self.assertEqual(["opensmacx_cld(s);"], lower("fc"))
        self.assertEqual(["opensmacx_std(s);"], lower("fd"))


class X87Tests(unittest.TestCase):
    """The x87 is a second machine, and its encoding rows are not parallel.

    The dangerous mistakes here are all well-formed. A reversed subtract
    compiles and returns a number; the wrong ST index compiles and returns a
    number; a `fstp` lowered as `fst` leaves the stack one deep and corrupts
    the NEXT function's register numbering rather than this one's answer.
    """

    def test_the_two_encoding_rows_are_different_operations(self):
        # D8 E1 disassembles as `fsub st(1)` with ONE operand and means
        # ST(0) -= ST(1). DC E1 disassembles as `fsubr st(1), st(0)` with TWO
        # and means ST(1) = ST(0) - ST(1). The rows are swapped in the opcode
        # map, so a lowering that keyed on the opcode's position rather than on
        # capstone's resolved mnemonic would negate every one of them.
        self.assertEqual(
            ["opensmacx_x87_binary_st0(OpensmacxX87Sub,"
             " opensmacx_x87_get(1U));"],
            lower("d8e1"))
        self.assertEqual(
            ["opensmacx_x87_binary_sti(OpensmacxX87Subr, 1U, false);"],
            lower("dce1"))

    def test_the_popping_form_pops(self):
        # de c1 - faddp st(1), st(0). The `true` is the whole difference
        # between a balanced body and one that leaks a stack slot per call.
        self.assertEqual(
            ["opensmacx_x87_binary_sti(OpensmacxX87Add, 1U, true);"],
            lower("dec1"))

    def test_a_memory_operand_selects_the_width_helper(self):
        # d8 00 / dc 00 - fadd dword ptr [eax] and fadd qword ptr [eax]. The
        # width is the OPERAND's, and reading a float as a double reads four
        # bytes past it.
        self.assertEqual(
            ["opensmacx_x87_binary_st0(OpensmacxX87Add,"
             " opensmacx_x87_mem32(s.eax));"],
            lower("d800"))
        self.assertEqual(
            ["opensmacx_x87_binary_st0(OpensmacxX87Add,"
             " opensmacx_x87_mem64(s.eax));"],
            lower("dc00"))

    def test_the_integer_forms_go_through_the_integer_accessors(self):
        # da 00 - fiadd dword ptr [eax]. Same operation, different decode of
        # the operand: `imem` reads an int32, `mem` would read a float.
        self.assertEqual(
            ["opensmacx_x87_binary_st0(OpensmacxX87Add,"
             " opensmacx_x87_imem32(s.eax));"],
            lower("da00"))

    def test_load_and_store_widths(self):
        self.assertEqual(["opensmacx_x87_fld32(s.eax);"], lower("d900"))
        self.assertEqual(["opensmacx_x87_fld64(s.eax);"], lower("dd00"))
        self.assertEqual(["opensmacx_x87_fld80(s.eax);"], lower("db28"))
        self.assertEqual(["opensmacx_x87_fild64(s.eax);"], lower("df28"))
        self.assertEqual(["opensmacx_x87_fst32(s.eax, true);"], lower("d918"))
        self.assertEqual(["opensmacx_x87_fst64(s.eax, false);"], lower("dd10"))
        self.assertEqual(["opensmacx_x87_fist32(s.eax, true);"], lower("db18"))

    def test_fnstsw_ax_is_a_sixteen_bit_merge(self):
        # df e0 - the status word goes into AX, and AX is half of EAX. An
        # assignment to `s.eax` would destroy the top 16 bits, which on this
        # path hold whatever the caller left there.
        self.assertEqual(
            ["s.eax = (s.eax & 0xffff0000U)"
             " | ((opensmacx_x87_status_word() << 0) & 0x0000ffffU);"],
            lower("dfe0"))

    def test_fcompp_compares_against_st1_and_pops_twice(self):
        self.assertEqual(["opensmacx_x87_fcom(opensmacx_x87_get(1U), 2U);"],
                         lower("ded9"))

    def test_fwait_emits_nothing_rather_than_being_refused(self):
        # Every exception is masked in this image, so there is nothing to wait
        # for - but 81 of these appear, and a refusal would be 81 traps.
        self.assertEqual([], lower("9b"))

    def test_the_constants_are_instructions_not_literals(self):
        self.assertEqual(["opensmacx_x87_fldpi();"], lower("d9eb"))

    def test_a_form_whose_source_is_not_st0_is_refused(self):
        # No disassembler emits this today, and capstone's own operands cannot
        # be edited to produce it, so the instruction is built by hand. If a
        # future capstone ever spells the DC or DE row this way, the index
        # these rules would pick is the wrong one - so they must trap rather
        # than silently reverse the destination.
        for mnemonic in ("fsubr", "faddp"):
            with self.subTest(mnemonic=mnemonic):
                with self.assertRaisesRegex(x86_lower.Unsupported,
                                            "not ST\\(0\\)"):
                    x86_lower.lower(
                        fake_x87(mnemonic, [1, 2]), no_labels)


class SwitchTests(unittest.TestCase):
    """A jump through a table is control flow INSIDE the function.

    Lowering it as a dispatch compiles and is counted as a success, and then
    fails the moment it runs: the dispatch table holds function STARTS, and
    every switch arm is an address interior to a function. So the wrong answer
    here is invisible to the refusal histogram - which is exactly why it needs
    a test that asserts the goto rather than the absence of a trap.
    """

    @staticmethod
    def labels(address):
        return f"L_{address:08x}" if address in (0x401100, 0x401200) else None

    def test_the_arms_become_gotos_in_index_order(self):
        # jmp dword ptr [eax*4 + 0x401000]
        self.assertEqual(
            ["switch (s.eax) {",
             "case 0U: goto L_00401100;",
             "case 1U: goto L_00401200;",
             "}",
             "opensmacx_dispatch(opensmacx_mem32((s.eax * 4U)"
             " + 0x00401000U))(s);",
             "return;"],
            lower("ff248500104000", self.labels, BASE,
                  [0x401100, 0x401200]))

    def test_the_index_register_comes_from_the_operand(self):
        # jmp dword ptr [ecx*4 + 0x401000] - the switch must key on the
        # register the ADDRESS uses, not on a fixed one.
        self.assertIn("switch (s.ecx) {",
                      lower("ff248d00104000", self.labels, BASE,
                            [0x401100]))

    def test_an_unlabelled_arm_refuses_the_whole_jump(self):
        # A recovered target that is not an instruction start in this function
        # cannot become a goto. A trap that names the jump beats a body that
        # does not compile, and beats a goto into the middle of an instruction.
        with self.assertRaises(x86_lower.Unsupported):
            lower("ff248500104000", self.labels, BASE, [0x401100, 0x409999])

    def test_without_a_table_it_is_still_a_dispatch(self):
        # The 71 vtable and computed forms have no table, and must be left
        # exactly as they were.
        self.assertEqual(
            ["opensmacx_dispatch(opensmacx_mem32((s.eax * 4U)"
             " + 0x00401000U))(s);", "return;"],
            lower("ff248500104000", self.labels))


if __name__ == "__main__":
    unittest.main()
