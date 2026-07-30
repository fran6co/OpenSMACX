#!/usr/bin/env python3
"""A Z3 encoding of the x86 semantics `tools/lifted_x86.h` defines.

This is the arithmetic core of the equivalence spike. It exists so a recovered
body and the original's machine code can be compared as FORMULAS rather than by
running them on sampled inputs, which is the only way to reach the population
both existing proof routes structurally cannot: small, relocation-bearing,
call-bearing functions.

IT IS A SECOND SEMANTICS, WHICH IS A SECOND CHANCE TO BE WRONG. An encoding
that errs in a COLLAPSING direction - one that makes two different programs
look equal - proves wrong recoveries correct while looking exactly like
success. Nothing here may be trusted on the grounds that it reads correctly.
`tools/validate_x86_smt.py` drives this and `tools/x86_smt_reference.cpp`, which
is lifted_x86.h itself, with the same random states and requires bit-for-bit
agreement. No proof from this module counts until that passes.

The rules are transcribed from lifted_x86.h rather than from a manual, and
where the two could differ the header wins, because the header is what the
lifted image actually runs. Three of its choices are easy to get subtly wrong
and are called out where they are encoded: PF is the parity of the LOW EIGHT
BITS at every width, AF comes from `(a ^ b ^ result) & 0x10` and not from a
carry computation, and the overflow expressions for add and subtract are
different formulas rather than one with a sign flipped.
"""

from __future__ import annotations

import z3

# static_recompile_runtime.h
CF = 1 << 0
PF = 1 << 2
AF = 1 << 4
ZF = 1 << 6
SF = 1 << 7
OF = 1 << 11
ALU_FLAGS = CF | PF | AF | ZF | SF | OF

WIDTHS = (8, 16, 32)


def mask_of(width: int) -> int:
    return (1 << width) - 1


def sign_of(width: int) -> int:
    return 1 << (width - 1)


def _bit(condition, flag: int):
    """A flag's contribution: `flag` when the condition holds, else zero."""
    return z3.If(condition, z3.BitVecVal(flag, 32), z3.BitVecVal(0, 32))


def parity_flag(result):
    """PF is the parity of the LOW 8 BITS of the result, at EVERY width.

    lifted_x86.h: `__builtin_parity(result & 0xFF) ? 0 : OpensmacxParityFlag`.
    Two traps in one line - the wider bits never participate even for a 32-bit
    operation, and the flag is set on EVEN parity, so the sense is inverted
    relative to the builtin.
    """
    low = z3.Extract(7, 0, result)
    ones = z3.Extract(0, 0, low)
    for bit in range(1, 8):
        ones = ones ^ z3.Extract(bit, bit, low)
    return _bit(ones == z3.BitVecVal(0, 1), PF)


def result_flags(result, width: int):
    """ZF, SF and PF, which every flag-writing operation derives the same way."""
    masked = result & z3.BitVecVal(mask_of(width), 32)
    return (_bit(masked == z3.BitVecVal(0, 32), ZF)
            | _bit((masked & z3.BitVecVal(sign_of(width), 32))
                   != z3.BitVecVal(0, 32), SF)
            | parity_flag(masked))


def _keep(flags):
    """The incoming flags minus everything an ALU operation redefines."""
    return flags & z3.BitVecVal(~ALU_FLAGS & 0xFFFFFFFF, 32)


def add_core(a, b, carry_in, flags, width: int):
    """ADD/ADC. Returns (result, flags)."""
    mask = z3.BitVecVal(mask_of(width), 32)
    sign = z3.BitVecVal(sign_of(width), 32)
    a = a & mask
    b = b & mask
    # One bit wider so the carry out of the top is a real bit rather than a
    # comparison that has to be argued about.
    wide = (z3.ZeroExt(1, z3.Extract(width - 1, 0, a))
            + z3.ZeroExt(1, z3.Extract(width - 1, 0, b))
            + z3.ZeroExt(1, z3.Extract(width - 1, 0, carry_in)))
    result = z3.ZeroExt(32 - width, z3.Extract(width - 1, 0, wide))
    carry = z3.Extract(width, width, wide)
    flags_out = (_keep(flags)
                 | result_flags(result, width)
                 | _bit(carry == z3.BitVecVal(1, 1), CF)
                 | _bit(((a ^ b ^ result) & z3.BitVecVal(0x10, 32))
                        != z3.BitVecVal(0, 32), AF)
                 # Overflow on an add: the operands AGREED in sign and the
                 # result disagrees with them.
                 | _bit((~(a ^ b) & (a ^ result) & sign)
                        != z3.BitVecVal(0, 32), OF))
    return result, flags_out


def sub_core(a, b, borrow_in, flags, width: int):
    """SUB/SBB/CMP. Returns (result, flags)."""
    mask = z3.BitVecVal(mask_of(width), 32)
    sign = z3.BitVecVal(sign_of(width), 32)
    a = a & mask
    b = b & mask
    result = (a - b - borrow_in) & mask
    # lifted_x86.h widens before comparing so that `b + 1` cannot wrap back to
    # zero and turn a borrow into no borrow. Same widening here.
    wide_a = z3.ZeroExt(1, z3.Extract(width - 1, 0, a))
    wide_b = (z3.ZeroExt(1, z3.Extract(width - 1, 0, b))
              + z3.ZeroExt(1, z3.Extract(width - 1, 0, borrow_in)))
    flags_out = (_keep(flags)
                 | result_flags(result, width)
                 | _bit(z3.ULT(wide_a, wide_b), CF)
                 | _bit(((a ^ b ^ result) & z3.BitVecVal(0x10, 32))
                        != z3.BitVecVal(0, 32), AF)
                 # Overflow on a subtract: the operands DISAGREED in sign and
                 # the result disagrees with the minuend. NOT the add formula
                 # with a negation - a different expression.
                 | _bit(((a ^ b) & (a ^ result) & sign)
                        != z3.BitVecVal(0, 32), OF))
    return result, flags_out


def logic_result(result, flags, width: int):
    """AND/OR/XOR/TEST: CF and OF are cleared, AF is undefined and left alone."""
    masked = result & z3.BitVecVal(mask_of(width), 32)
    return masked, _keep(flags) | result_flags(masked, width)


def carry_bit(flags):
    return z3.LShR(flags & z3.BitVecVal(CF, 32), 0) & z3.BitVecVal(1, 32)


def _sext(value, width: int):
    """Sign-extend a width-W value to a full 32-bit one."""
    if width == 32:
        return value
    return z3.SignExt(32 - width, z3.Extract(width - 1, 0, value))


def shift_core(op: str, value, count, flags, width: int):
    """SHL/SHR/SAR. Returns (result, flags).

    THE COUNT IS MASKED TO 5 BITS FIRST, AT EVERY WIDTH, so an 8-bit shift by
    32 is a shift by zero - and a shift by zero touches NO FLAG AT ALL. That
    early return in lifted_x86.h is the semantics and not an optimisation, so
    it is encoded as a guard over the whole flag word rather than folded into
    the individual flag expressions.
    """
    mask = z3.BitVecVal(mask_of(width), 32)
    sign = z3.BitVecVal(sign_of(width), 32)
    value = value & mask
    count = count & z3.BitVecVal(31, 32)
    zero = z3.BitVecVal(0, 32)
    one = z3.BitVecVal(1, 32)

    if op == "shl":
        result = (value << count) & mask
        # CF is the last bit shifted out of the top; past the operand width
        # there is nothing left to shift out.
        shifted = z3.LShR(value, (z3.BitVecVal(width, 32) - count)
                          & z3.BitVecVal(31, 32)) & one
        carry = z3.If(z3.ULE(count, z3.BitVecVal(width, 32)), shifted, zero)
        carried = carry != zero
        changed = (_keep(flags) | result_flags(result, width)
                   | _bit(carried, CF)
                   # Defined architecturally only for count 1, where it is
                   # "the sign changed"; computed uniformly for determinism.
                   | _bit(((result & sign) != zero) != carried, OF))
    elif op == "shr":
        result = z3.LShR(value, count)
        carry = z3.LShR(value, count - one) & one
        changed = (_keep(flags) | result_flags(result, width)
                   | _bit(carry != zero, CF)
                   # For SHR, OF is the sign of the ORIGINAL operand.
                   | _bit((value & sign) != zero, OF))
    elif op == "sar":
        wide = _sext(value, width)
        result = (wide >> count) & mask
        carry = (wide >> (count - one)) & one
        # OF is CLEARED by SAR: an arithmetic right shift cannot change sign.
        changed = (_keep(flags) | result_flags(result, width)
                   | _bit(carry != zero, CF))
    else:
        raise KeyError(op)
    unshifted = value if op != "sar" else value & mask
    return (z3.If(count == zero, unshifted, result),
            z3.If(count == zero, flags, changed))


def imul_core(a, b, flags, width: int):
    """Two-operand IMUL. CF and OF mean "the full product did not fit"."""
    wide = z3.SignExt(64 - width, z3.Extract(width - 1, 0, a)) * \
        z3.SignExt(64 - width, z3.Extract(width - 1, 0, b))
    result = z3.ZeroExt(32 - width, z3.Extract(width - 1, 0, wide))
    back = z3.SignExt(64 - width, z3.Extract(width - 1, 0, result))
    truncated = wide != back
    return result, (_keep(flags) | result_flags(result, width)
                    | _bit(truncated, CF) | _bit(truncated, OF))


# Every operation this module encodes, by the name the reference harness uses,
# so the validator cannot drift out of step with either side.
def evaluate(op: str, width: int, a, b, flags):
    """(result, flags) for `<op><width>`, mirroring the reference harness."""
    zero = z3.BitVecVal(0, 32)
    one = z3.BitVecVal(1, 32)
    if op == "add":
        return add_core(a, b, zero, flags, width)
    if op == "adc":
        return add_core(a, b, carry_bit(flags), flags, width)
    if op == "sub":
        return sub_core(a, b, zero, flags, width)
    if op == "sbb":
        return sub_core(a, b, carry_bit(flags), flags, width)
    if op == "and":
        return logic_result(a & b, flags, width)
    if op == "or":
        return logic_result(a | b, flags, width)
    if op == "xor":
        return logic_result(a ^ b, flags, width)
    if op == "inc":
        # INC and DEC leave CF ALONE. Encoding them as add/sub with a carry of
        # zero would clear it, which is the classic way to get a loop that
        # tests CF after an increment subtly wrong.
        result, changed = add_core(a, one, zero, flags, width)
        return result, (changed & z3.BitVecVal(~CF & 0xFFFFFFFF, 32)) | (
            flags & z3.BitVecVal(CF, 32))
    if op == "dec":
        result, changed = sub_core(a, one, zero, flags, width)
        return result, (changed & z3.BitVecVal(~CF & 0xFFFFFFFF, 32)) | (
            flags & z3.BitVecVal(CF, 32))
    if op == "neg":
        # Exactly `sub` from zero, flags included.
        return sub_core(zero, a, zero, flags, width)
    if op == "not":
        # NOT affects NO flags. Not "sets them from the result" - none.
        return (~a) & z3.BitVecVal(mask_of(width), 32), flags
    if op in ("shl", "shr", "sar"):
        return shift_core(op, a, b, flags, width)
    if op == "imul":
        return imul_core(a, b, flags, width)
    raise KeyError(op)


SUPPORTED = ("add", "adc", "sub", "sbb", "and", "or", "xor", "inc", "dec",
             "neg", "not", "shl", "shr", "sar", "imul")


def concrete(op: str, width: int, a: int, b: int, flags: int) -> tuple[int, int]:
    """Evaluate on concrete inputs, which is what the validator compares.

    Uses Z3's own simplifier rather than a separate interpreter: a hand-written
    evaluator would be a THIRD semantics, and agreeing with it would say
    nothing about the formulas the prover actually sees.
    """
    result, flags_out = evaluate(op, width,
                                 z3.BitVecVal(a, 32), z3.BitVecVal(b, 32),
                                 z3.BitVecVal(flags, 32))
    return (z3.simplify(result).as_long(), z3.simplify(flags_out).as_long())
