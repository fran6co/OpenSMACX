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
import unittest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

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


def no_labels(_address):
    """A `label_for` for functions with no branch targets of their own."""
    return None


def lower(encoded, label_for=no_labels, address=BASE):
    return x86_lower.lower(decode(encoded, address), label_for)


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
        # div ecx - no lowering, and no pretending there is one.
        with self.assertRaises(x86_lower.Unsupported):
            lower("f7f1")

    def test_the_refusal_names_the_address(self):
        with self.assertRaisesRegex(x86_lower.Unsupported, "0x00401000"):
            lower("f7f1")

    def test_the_refusal_names_the_instruction(self):
        with self.assertRaisesRegex(x86_lower.Unsupported, "div"):
            lower("f7f1")

    def test_the_address_in_the_refusal_is_the_instruction_s_own(self):
        # Two instructions, one message each: the trap has to point at the
        # right one, not at the start of the function.
        with self.assertRaisesRegex(x86_lower.Unsupported, "0x00402340"):
            lower("f7f1", no_labels, 0x00402340)

    def test_segment_relative_memory_is_refused(self):
        # mov eax, fs:[0] - the SEH chain. Lowering it as a flat access would
        # read and write image memory at address 0.
        with self.assertRaisesRegex(x86_lower.Unsupported, "segment"):
            lower("64a100000000")

    def test_the_segment_refusal_also_names_the_address(self):
        # The refusal comes from inside the operand model rather than from the
        # mnemonic table, and it has to be just as identifiable.
        with self.assertRaisesRegex(x86_lower.Unsupported, "0x00401000"):
            lower("64a100000000")

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

    def test_a_floating_point_instruction_is_refused(self):
        # fld dword ptr [eax] - x87 is a later phase, not a silent no-op.
        with self.assertRaises(x86_lower.Unsupported):
            lower("d900")

    def test_a_repeated_string_instruction_is_refused(self):
        # rep movsb - the repeat prefix is a loop; dropping it would copy one
        # byte and call it done.
        with self.assertRaises(x86_lower.Unsupported):
            lower("f3a4")


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
        self.assertNotIn("div", x86_lower.candidate_mnemonics())

    def test_the_candidate_set_is_an_upper_bound_not_coverage(self):
        # `mov` is in the set and these three `mov`s are all refused, so any
        # coverage figure computed by counting instructions whose mnemonic is
        # in the set overstates the lift by however many of these the image
        # holds - and no test of the set alone can detect that, because the
        # set is keyed on the mnemonic and the refusals are about operands.
        # The measured number has to come from calling `lower`.
        self.assertIn("mov", x86_lower.candidate_mnemonics())
        for encoded, why in (("64a100000000", "segment override"),
                             ("678b00", "16-bit addressing"),
                             ("0f20c0", "control register")):
            with self.subTest(why=why):
                self.assertEqual("mov", decode(encoded).mnemonic)
                with self.assertRaises(x86_lower.Unsupported):
                    lower(encoded)


if __name__ == "__main__":
    unittest.main()
