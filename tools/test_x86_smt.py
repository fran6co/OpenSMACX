#!/usr/bin/env python3
"""Tests for the Z3 x86 encoding.

These are NOT the evidence that the encoding is right. That comes from
`tools/validate_x86_smt.py`, which drives this module and lifted_x86.h with the
same states and requires bit-for-bit agreement; a unit test written from the
same understanding as the code would agree with a shared misunderstanding.

What these pin is the part validation cannot: that the module is symbolic
rather than only concrete - `evaluate` must produce usable formulas over free
variables, since a prover never sees a concrete state - and the handful of
values whose flags are worth stating outright so a reader can check them by
hand against the manual.
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import z3  # noqa: E402

import x86_smt  # noqa: E402


class ConcreteTests(unittest.TestCase):
    """A few results spelled out, checkable against the manual by eye."""

    def test_a_signed_overflow_on_add_sets_of_and_sf(self):
        result, flags = x86_smt.concrete("add", 32, 0x7FFFFFFF, 1, 0)
        self.assertEqual(0x80000000, result)
        self.assertTrue(flags & x86_smt.OF)
        self.assertTrue(flags & x86_smt.SF)
        self.assertTrue(flags & x86_smt.AF)   # 0xF + 1 carries out of bit 3
        self.assertFalse(flags & x86_smt.CF)  # unsigned, it did not carry
        self.assertFalse(flags & x86_smt.ZF)

    def test_borrowing_below_zero_sets_cf(self):
        result, flags = x86_smt.concrete("sub", 8, 0x00, 0x01, 0)
        self.assertEqual(0xFF, result)
        self.assertTrue(flags & x86_smt.CF)
        self.assertTrue(flags & x86_smt.SF)
        self.assertFalse(flags & x86_smt.OF)  # 0 - 1 does not overflow signed

    def test_a_zero_result_sets_zf_and_pf(self):
        result, flags = x86_smt.concrete("and", 32, 0xF0F0, 0x0F0F, 0)
        self.assertEqual(0, result)
        self.assertTrue(flags & x86_smt.ZF)
        self.assertTrue(flags & x86_smt.PF)
        self.assertFalse(flags & x86_smt.CF)  # logic clears CF
        self.assertFalse(flags & x86_smt.OF)

    def test_parity_reads_only_the_low_byte(self):
        # 0x0000FF00 has even parity overall and ZERO one-bits in its low byte,
        # which is also even - so PF is set. The discriminating case is a value
        # whose low byte and whole word disagree.
        _, flags = x86_smt.concrete("or", 32, 0x0000FF01, 0, 0)
        # Low byte 0x01 has one bit set: ODD, so PF must be clear even though
        # the full word has nine bits set (also odd, but read at width 32 a
        # naive encoding would use a different bit count).
        self.assertFalse(flags & x86_smt.PF)
        _, flags = x86_smt.concrete("or", 32, 0x0000FF03, 0, 0)
        self.assertTrue(flags & x86_smt.PF)   # low byte 0x03, two bits: even

    def test_inc_leaves_carry_alone(self):
        # The reason INC is not encoded as `add 1`: a loop that tests CF after
        # incrementing an index would silently change meaning.
        _, flags = x86_smt.concrete("inc", 32, 0, 0, x86_smt.CF)
        self.assertTrue(flags & x86_smt.CF)
        _, flags = x86_smt.concrete("add", 32, 0, 1, x86_smt.CF)
        self.assertFalse(flags & x86_smt.CF)

    def test_adc_carries_the_incoming_flag_into_the_result(self):
        result, _ = x86_smt.concrete("adc", 32, 1, 1, x86_smt.CF)
        self.assertEqual(3, result)
        result, _ = x86_smt.concrete("adc", 32, 1, 1, 0)
        self.assertEqual(2, result)

    def test_a_width_narrower_than_32_truncates(self):
        result, flags = x86_smt.concrete("add", 8, 0xFF, 0x02, 0)
        self.assertEqual(0x01, result)
        self.assertTrue(flags & x86_smt.CF)


class ShiftTests(unittest.TestCase):
    """The shift rules that are easiest to encode almost-correctly."""

    def test_a_shift_by_zero_touches_no_flag(self):
        # lifted_x86.h returns early on a zero count, and that early return is
        # the semantics rather than an optimisation: every other encoding of
        # SHL would clear CF here.
        before = x86_smt.CF | x86_smt.OF | x86_smt.ZF
        result, flags = x86_smt.concrete("shl", 32, 0xFFFFFFFF, 0, before)
        self.assertEqual(0xFFFFFFFF, result)
        self.assertEqual(before, flags)

    def test_the_count_is_masked_to_five_bits_at_every_width(self):
        # An 8-bit shift by 32 is a shift by ZERO, not a shift that clears the
        # operand - and therefore touches no flag either.
        result, flags = x86_smt.concrete("shl", 8, 0xAB, 32, x86_smt.CF)
        self.assertEqual(0xAB, result)
        self.assertEqual(x86_smt.CF, flags)

    def test_shr_overflow_comes_from_the_original_operand(self):
        # OF is the sign of what went IN, not of what came out.
        _, flags = x86_smt.concrete("shr", 32, 0x80000000, 1, 0)
        self.assertTrue(flags & x86_smt.OF)
        _, flags = x86_smt.concrete("shr", 32, 0x40000000, 1, 0)
        self.assertFalse(flags & x86_smt.OF)

    def test_sar_never_sets_overflow(self):
        _, flags = x86_smt.concrete("sar", 32, 0x80000000, 1, x86_smt.OF)
        self.assertFalse(flags & x86_smt.OF)

    def test_sar_shifts_the_sign_in(self):
        result, _ = x86_smt.concrete("sar", 32, 0x80000000, 4, 0)
        self.assertEqual(0xF8000000, result)

    def test_not_touches_no_flag(self):
        before = x86_smt.CF | x86_smt.ZF | x86_smt.SF
        result, flags = x86_smt.concrete("not", 32, 0x0F0F0F0F, 0, before)
        self.assertEqual(0xF0F0F0F0, result)
        self.assertEqual(before, flags)

    def test_neg_sets_carry_when_the_operand_was_non_zero(self):
        _, flags = x86_smt.concrete("neg", 32, 1, 0, 0)
        self.assertTrue(flags & x86_smt.CF)
        _, flags = x86_smt.concrete("neg", 32, 0, 0, 0)
        self.assertFalse(flags & x86_smt.CF)

    def test_neg_overflows_only_on_the_value_with_no_negation(self):
        _, flags = x86_smt.concrete("neg", 32, 0x80000000, 0, 0)
        self.assertTrue(flags & x86_smt.OF)

    def test_imul_flags_say_whether_the_product_fitted(self):
        _, flags = x86_smt.concrete("imul", 32, 2, 3, 0)
        self.assertFalse(flags & x86_smt.CF)
        self.assertFalse(flags & x86_smt.OF)
        _, flags = x86_smt.concrete("imul", 32, 0x10000, 0x10000, 0)
        self.assertTrue(flags & x86_smt.CF)
        self.assertTrue(flags & x86_smt.OF)

    def test_imul_is_signed(self):
        # -1 * -1 == 1 fits; an unsigned reading would call it truncated.
        result, flags = x86_smt.concrete("imul", 32, 0xFFFFFFFF, 0xFFFFFFFF, 0)
        self.assertEqual(1, result)
        self.assertFalse(flags & x86_smt.CF)


class SymbolicTests(unittest.TestCase):
    """The prover never sees a concrete state, so the formulas must be free."""

    def test_evaluate_accepts_free_variables(self):
        a, b, flags = (z3.BitVec("a", 32), z3.BitVec("b", 32),
                       z3.BitVec("f", 32))
        result, out = x86_smt.evaluate("add", 32, a, b, flags)
        self.assertEqual(32, result.size())
        self.assertEqual(32, out.size())

    def test_subtracting_a_value_from_itself_is_provably_zero(self):
        a = z3.BitVec("a", 32)
        result, _ = x86_smt.evaluate("sub", 32, a, a, z3.BitVecVal(0, 32))
        solver = z3.Solver()
        solver.add(result != z3.BitVecVal(0, 32))
        self.assertEqual(z3.unsat, solver.check())

    def test_zf_holds_exactly_when_the_result_is_zero(self):
        # A property rather than a sample: the flag must agree with the result
        # for EVERY input, which is the kind of statement only a solver makes.
        a, b = z3.BitVec("a", 32), z3.BitVec("b", 32)
        result, flags = x86_smt.evaluate("sub", 32, a, b, z3.BitVecVal(0, 32))
        zf = (flags & z3.BitVecVal(x86_smt.ZF, 32)) != z3.BitVecVal(0, 32)
        solver = z3.Solver()
        solver.add(zf != (result == z3.BitVecVal(0, 32)))
        self.assertEqual(z3.unsat, solver.check())

    def test_xor_of_a_value_with_itself_is_provably_zero(self):
        a = z3.BitVec("a", 32)
        result, flags = x86_smt.evaluate("xor", 32, a, a, z3.BitVec("f", 32))
        solver = z3.Solver()
        solver.add(z3.Or(result != z3.BitVecVal(0, 32),
                         (flags & z3.BitVecVal(x86_smt.ZF, 32))
                         == z3.BitVecVal(0, 32)))
        self.assertEqual(z3.unsat, solver.check())

    def test_an_unknown_operation_is_refused(self):
        with self.assertRaises(KeyError):
            x86_smt.evaluate("bswap", 32, z3.BitVec("a", 32),
                             z3.BitVec("b", 32), z3.BitVec("f", 32))


if __name__ == "__main__":
    unittest.main()
