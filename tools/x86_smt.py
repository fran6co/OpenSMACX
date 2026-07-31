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


# ---------------------------------------------------------------------------
# Machine state and memory
#
# Memory is one flat Array(BV32, BV8). Byte-addressed rather than word-indexed
# because x86 is: a 32-bit store followed by an 8-bit load one byte along has to
# see the second byte of what was stored, and a word-indexed model either
# cannot express that or has to special-case it. Little-endian is written out
# explicitly for the same reason - it is a property of the machine, not of the
# solver, and burying it in a helper is how an encoding ends up big-endian in
# one place and little-endian in another.
# ---------------------------------------------------------------------------

REGISTERS = ("eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi")


class State:
    """Registers, flags and memory, as Z3 terms."""

    def __init__(self, prefix: str = ""):
        self.regs = {name: z3.BitVec(f"{prefix}{name}", 32)
                     for name in REGISTERS}
        self.flags = z3.BitVec(f"{prefix}eflags", 32)
        self.mem = z3.Array(f"{prefix}mem", z3.BitVecSort(32),
                            z3.BitVecSort(8))

    def copy(self) -> "State":
        clone = State.__new__(State)
        clone.regs = dict(self.regs)
        clone.flags = self.flags
        clone.mem = self.mem
        return clone


def load(mem, address, width: int):
    """Little-endian read of `width` bits, returned zero-extended to 32."""
    if width % 8:
        raise ValueError(f"width {width} is not a whole number of bytes")
    value = None
    for index in range(width // 8):
        byte = z3.Select(mem, address + z3.BitVecVal(index, 32))
        value = byte if value is None else z3.Concat(byte, value)
    return z3.ZeroExt(32 - width, value)


def store(mem, address, value, width: int):
    """Little-endian write of the low `width` bits. Returns the new memory."""
    if width % 8:
        raise ValueError(f"width {width} is not a whole number of bytes")
    for index in range(width // 8):
        byte = z3.Extract(index * 8 + 7, index * 8, value)
        mem = z3.Store(mem, address + z3.BitVecVal(index, 32), byte)
    return mem


# ---------------------------------------------------------------------------
# Decoding a straight-line block into a formula
#
# One pass over capstone's decode, folding each instruction into the state. No
# loops and no branches yet: the recovered bodies this targets have a median of
# six instructions, and a straight run is what a leaf looks like.
#
# An instruction this does not model raises rather than being skipped. A
# skipped instruction produces a formula about a DIFFERENT program, and the
# prover would then answer a question nobody asked - the single most dangerous
# thing an encoder like this can do.
# ---------------------------------------------------------------------------

SUB_REGISTERS = {
    "al": ("eax", 0, 8), "cl": ("ecx", 0, 8), "dl": ("edx", 0, 8),
    "bl": ("ebx", 0, 8), "ah": ("eax", 8, 8), "ch": ("ecx", 8, 8),
    "dh": ("edx", 8, 8), "bh": ("ebx", 8, 8),
    "ax": ("eax", 0, 16), "cx": ("ecx", 0, 16), "dx": ("edx", 0, 16),
    "bx": ("ebx", 0, 16), "sp": ("esp", 0, 16), "bp": ("ebp", 0, 16),
    "si": ("esi", 0, 16), "di": ("edi", 0, 16),
}


class Unsupported(Exception):
    """An instruction the encoder does not model. Never silently skipped."""


def read_register(state: State, name: str):
    if name in state.regs:
        return state.regs[name]
    if name in SUB_REGISTERS:
        whole, shift, width = SUB_REGISTERS[name]
        value = z3.LShR(state.regs[whole], z3.BitVecVal(shift, 32))
        return value & z3.BitVecVal(mask_of(width), 32)
    raise Unsupported(f"register {name}")


def write_register(state: State, name: str, value):
    """Writing a sub-register PRESERVES the bits around it.

    `mov al, 0` leaves the top 24 bits of EAX alone. An encoding that writes the
    whole register instead is wrong in a way that disappears whenever the
    caller happens not to read those bits, which is most of the time.
    """
    if name in state.regs:
        state.regs[name] = value
        return
    if name not in SUB_REGISTERS:
        raise Unsupported(f"register {name}")
    whole, shift, width = SUB_REGISTERS[name]
    keep = z3.BitVecVal(~(mask_of(width) << shift) & 0xFFFFFFFF, 32)
    piece = (value & z3.BitVecVal(mask_of(width), 32)) << z3.BitVecVal(shift, 32)
    state.regs[whole] = (state.regs[whole] & keep) | piece


def effective_address(state: State, instruction, operand):
    """base + index*scale + displacement, exactly as the machine computes it.

    Register names come from the INSTRUCTION, because capstone gives a memory
    operand register IDs rather than names and resolving them any other way is
    a guess about its numbering.
    """
    total = z3.BitVecVal(operand.mem.disp & 0xFFFFFFFF, 32)
    if operand.mem.base:
        total = total + read_register(
            state, instruction.reg_name(operand.mem.base))
    if operand.mem.index:
        total = total + read_register(
            state, instruction.reg_name(operand.mem.index)) * \
            z3.BitVecVal(operand.mem.scale, 32)
    return total


def operand_width(instruction, operand) -> int:
    return operand.size * 8


def read_operand(state: State, instruction, operand):
    kind = operand.type
    if kind == 1:                                    # X86_OP_REG
        return read_register(state, instruction.reg_name(operand.reg))
    if kind == 2:                                    # X86_OP_IMM
        return z3.BitVecVal(operand.imm & 0xFFFFFFFF, 32)
    if kind == 3:                                    # X86_OP_MEM
        return load(state.mem, effective_address(state, instruction, operand),
                    operand_width(instruction, operand))
    raise Unsupported(f"operand type {kind}")


def write_operand(state: State, instruction, operand, value):
    kind = operand.type
    if kind == 1:
        write_register(state, instruction.reg_name(operand.reg), value)
        return
    if kind == 3:
        state.mem = store(state.mem, effective_address(state, instruction, operand), value,
                          operand_width(instruction, operand))
        return
    raise Unsupported(f"cannot write operand type {kind}")


# mnemonic -> the operation `evaluate` knows it by.
BINARY = {"add": "add", "adc": "adc", "sub": "sub", "sbb": "sbb",
          "and": "and", "or": "or", "xor": "xor", "shl": "shl", "sal": "shl",
          "shr": "shr", "sar": "sar", "imul": "imul"}
UNARY = {"inc": "inc", "dec": "dec", "neg": "neg", "not": "not"}


def step(state: State, instruction) -> State:
    """Fold one instruction into the state. Raises on anything unmodelled."""
    name = instruction.mnemonic
    operands = instruction.operands

    if name == "nop":
        return state
    if name == "mov":
        write_operand(state, instruction, operands[0],
                      read_operand(state, instruction, operands[1]))
        return state
    if name == "lea":
        # The ADDRESS, not what is at it. Encoding lea as a load is a classic
        # way to make a formula that is wrong only when the address happens to
        # be mapped.
        write_register(state, instruction.reg_name(operands[0].reg),
                       effective_address(state, instruction, operands[1]))
        return state
    if name in ("movzx", "movsx"):
        source = read_operand(state, instruction, operands[1])
        width = operand_width(instruction, operands[1])
        if name == "movzx":
            value = source & z3.BitVecVal(mask_of(width), 32)
        else:
            value = _sext(source, width)
        write_operand(state, instruction, operands[0], value)
        return state
    if name in BINARY:
        width = operand_width(instruction, operands[0])
        left = read_operand(state, instruction, operands[0])
        right = read_operand(state, instruction, operands[1])
        result, state.flags = evaluate(BINARY[name], width, left, right,
                                       state.flags)
        write_operand(state, instruction, operands[0], result)
        return state
    if name in UNARY:
        width = operand_width(instruction, operands[0])
        value = read_operand(state, instruction, operands[0])
        result, state.flags = evaluate(UNARY[name], width, value, value,
                                       state.flags)
        write_operand(state, instruction, operands[0], result)
        return state
    if name.startswith("set") and len(name) > 3:
        # setcc writes ONE byte and leaves the rest of the register alone,
        # which write_register already handles; the value is the condition
        # itself, so the two share a table and cannot drift apart.
        value = z3.If(condition(name[3:], state.flags),
                      z3.BitVecVal(1, 32), z3.BitVecVal(0, 32))
        write_operand(state, instruction, operands[0], value)
        return state
    if name == "cmp":
        width = operand_width(instruction, operands[0])
        left = read_operand(state, instruction, operands[0])
        right = read_operand(state, instruction, operands[1])
        # CMP is SUB that keeps its flags and throws the result away.
        _, state.flags = evaluate("sub", width, left, right, state.flags)
        return state
    if name == "test":
        width = operand_width(instruction, operands[0])
        left = read_operand(state, instruction, operands[0])
        right = read_operand(state, instruction, operands[1])
        _, state.flags = evaluate("and", width, left, right, state.flags)
        return state
    if name == "call":
        operand = operands[0]
        if operand.type == 2:                       # X86_OP_IMM, a direct call
            target = z3.BitVecVal(operand.imm & 0xFFFFFFFF, 32)
        else:
            # Indirect and virtual calls are NOT excluded: the target is read
            # like any other operand, so two sides computing the same pointer
            # get the same callee term and two sides computing different ones
            # do not. That distinction is the point.
            target = read_operand(state, instruction, operand)
        return apply_call(state, target)
    if name == "push":
        value = read_operand(state, instruction, operands[0])
        state.regs["esp"] = state.regs["esp"] - z3.BitVecVal(4, 32)
        state.mem = store(state.mem, state.regs["esp"], value, 32)
        return state
    if name == "pop":
        value = load(state.mem, state.regs["esp"], 32)
        state.regs["esp"] = state.regs["esp"] + z3.BitVecVal(4, 32)
        write_operand(state, instruction, operands[0], value)
        return state
    raise Unsupported(f"{name} {instruction.op_str}")


def encode_function(code: bytes, address: int, state: State,
                    limit: int = 64) -> State:
    """Every path from entry to `ret`, merged with ITE.

    The recovered bodies this targets have a median of six instructions and p90
    of three branches, so enumerating paths is cheaper than any cleverness and
    needs no invariants. `limit` bounds the recursion against a decode that
    runs away rather than against real control flow.

    A LOOP RAISES. Bounding one silently would produce a formula about a
    program that runs a fixed number of iterations, which is a different
    program from the one being proved - and it would be a formula the prover
    would happily discharge.
    """
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    stream = {item.address: item for item in engine.disasm(code, address)}
    if not stream:
        raise Unsupported("nothing decoded")

    def walk(at: int, current: State, seen: frozenset, depth: int) -> State:
        while True:
            if depth > limit:
                raise Unsupported("path longer than the limit")
            item = stream.get(at)
            if item is None:
                raise Unsupported(f"control leaves the block at {at:#x}")
            if at in seen:
                raise Unsupported(f"loop back to {at:#x}")
            seen = seen | {at}
            name = item.mnemonic
            if name == "ret":
                return current
            if name == "jmp":
                target = item.operands[0]
                if target.type != 2:          # X86_OP_IMM
                    raise Unsupported("indirect jmp")
                at = target.imm
                depth += 1
                continue
            if name.startswith("j"):
                target = item.operands[0]
                if target.type != 2:
                    raise Unsupported("indirect branch")
                when = condition(name[1:], current.flags)
                # Both sides explored from the SAME state, then merged. The
                # copies matter: `step` mutates, so sharing one state would
                # let the taken path's writes leak into the untaken one.
                taken = walk(target.imm, current.copy(), seen, depth + 1)
                fell = walk(item.address + item.size, current.copy(), seen,
                            depth + 1)
                return merge(taken, fell, when)
            current = step(current, item)
            at = item.address + item.size
            depth += 1

    return walk(address, state, frozenset(), 0)


def encode_block(code: bytes, address: int, state: State) -> State:
    """Fold a straight-line block into the state, stopping at `ret`.

    A `ret` ends the block. Anything else that transfers control raises: this
    encoder models no branches yet, and quietly falling through a jump would
    produce a formula about a path the program does not take.
    """
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    for instruction in engine.disasm(code, address):
        if instruction.mnemonic == "ret":
            return state
        if instruction.group(1) or instruction.mnemonic.startswith("j"):
            raise Unsupported(f"control flow: {instruction.mnemonic}")
        state = step(state, instruction)
    return state


# ---------------------------------------------------------------------------
# Conditions, branches and path merging
#
# The recovered bodies this targets have a median of six instructions and p90
# of three branches, with no loops - so every path from entry to `ret` is
# enumerated and the results merged with ITE. No unrolling and no invariants,
# because there is nothing to unroll.
#
# A LOOP IS REFUSED, not bounded. Silently unrolling one would produce a
# formula about a program that runs a fixed number of iterations, which is a
# different program from the one being proved.
# ---------------------------------------------------------------------------

def flag_set(flags, bit: int):
    return (flags & z3.BitVecVal(bit, 32)) != z3.BitVecVal(0, 32)


def condition(name: str, flags):
    """The predicate a `j<cc>` or `set<cc>` tests, as a Z3 Bool.

    Spelled out per mnemonic rather than derived, because the signed and
    unsigned families read almost alike and differ in exactly the place that
    matters: `jl` is SF != OF and `jb` is CF, and swapping them produces a
    formula that agrees with the program on every non-negative input.
    """
    cf, pf, zf = flag_set(flags, CF), flag_set(flags, PF), flag_set(flags, ZF)
    sf, of = flag_set(flags, SF), flag_set(flags, OF)
    table = {
        "o": of, "no": z3.Not(of),
        "b": cf, "c": cf, "nae": cf,
        "ae": z3.Not(cf), "nb": z3.Not(cf), "nc": z3.Not(cf),
        "e": zf, "z": zf,
        "ne": z3.Not(zf), "nz": z3.Not(zf),
        "be": z3.Or(cf, zf), "na": z3.Or(cf, zf),
        "a": z3.And(z3.Not(cf), z3.Not(zf)),
        "nbe": z3.And(z3.Not(cf), z3.Not(zf)),
        "s": sf, "ns": z3.Not(sf),
        "p": pf, "pe": pf, "np": z3.Not(pf), "po": z3.Not(pf),
        "l": sf != of, "nge": sf != of,
        "ge": sf == of, "nl": sf == of,
        "le": z3.Or(zf, sf != of), "ng": z3.Or(zf, sf != of),
        "g": z3.And(z3.Not(zf), sf == of),
        "nle": z3.And(z3.Not(zf), sf == of),
    }
    if name not in table:
        raise Unsupported(f"condition {name}")
    return table[name]


def merge(taken: State, not_taken: State, when) -> State:
    """One state from two, selected by the path condition."""
    merged = State.__new__(State)
    merged.regs = {name: z3.If(when, taken.regs[name], not_taken.regs[name])
                   for name in REGISTERS}
    merged.flags = z3.If(when, taken.flags, not_taken.flags)
    merged.mem = z3.If(when, taken.mem, not_taken.mem)
    return merged


# ---------------------------------------------------------------------------
# Calls
#
# A callee is UNINTERPRETED: whatever it does, it does the same thing to both
# sides. That is the whole reason this works, and it is worth stating exactly,
# because it is easy to mistake for a weakness.
#
# The model does not have to be ACCURATE. It has to be the SAME FUNCTION on
# both sides, applied to arguments the two sides can be shown to agree on. If
# the recovered body reaches the same call target with the same reachable
# memory, the callee's contribution to both final states is literally the same
# term and cancels. A more faithful model would not prove more; an
# INCONSISTENT one would prove less, or worse, prove something false.
#
# `target` is a BitVec rather than a constant, so an indirect or virtual call
# is not a special case: two sides calling through the same computed pointer
# get the same term, and two sides computing DIFFERENT pointers do not - which
# is exactly the distinction a proof needs to make.
#
# WHAT THE CALLEE MAY SEE is canonicalised first. Everything at or above ESP is
# reachable by it; everything below is this frame's private space, which the
# callee cannot name because it has not been given a pointer to it. Passing raw
# memory instead would make the callee's result depend on the caller's dead
# locals, and the two sides lay their frames out differently - so a true
# equivalence would fail to prove for a reason that has nothing to do with the
# program.
# ---------------------------------------------------------------------------

MEMORY_SORT = z3.ArraySort(z3.BitVecSort(32), z3.BitVecSort(8))
CALLEE_MEMORY = z3.Function("callee_memory", z3.BitVecSort(32), MEMORY_SORT,
                            z3.BitVecSort(32), MEMORY_SORT)
CALLEE_RESULT = z3.Function("callee_result", z3.BitVecSort(32), MEMORY_SORT,
                            z3.BitVecSort(32), z3.BitVecSort(32))
CALLEE_SECOND = z3.Function("callee_second", z3.BitVecSort(32), MEMORY_SORT,
                            z3.BitVecSort(32), z3.BitVecSort(32))
CALLEE_FLAGS = z3.Function("callee_flags", z3.BitVecSort(32), MEMORY_SORT,
                           z3.BitVecSort(32), z3.BitVecSort(32))


def canonical(mem, esp):
    """What a callee can see.

    NO LAMBDA, and that is a decidability decision rather than a modelling one.
    Canonicalising with `Lambda(a, If(a >= esp, mem[a], 0))` is the more
    faithful statement - the private region is exactly `addr < esp` - but Z3
    answers `unknown` on it, reporting "incomplete (theory array)". An
    undecidable obligation discharges nothing, and worse, an `unknown` read as
    an `unsat` looks exactly like a proof.

    So the callee is given the memory itself and the ESP that bounds it, and
    the bound does its work as an ARGUMENT rather than as a rewritten array.
    Two sides that agree on memory and on ESP still get the same term, which is
    what the cancellation needs.

    What this gives up: if the two sides differ BELOW ESP - dead locals in
    frames they lay out differently - their callee terms differ and a true
    equivalence may fail to prove. That is the safe direction, and it is left
    to be measured rather than pre-emptied, since no such case has been seen
    yet.
    """
    return mem


def apply_call(state: State, target) -> State:
    """Fold an uninterpreted callee into the state.

    ESP is left where it was: the call pushes a return address and the callee's
    `ret` pops it. A stdcall callee also pops its arguments, which this does not
    model - and does not need to, because the same unmodelled adjustment lands
    on both sides and cancels. Only a difference between the sides can survive.
    """
    esp = state.regs["esp"]
    visible = canonical(state.mem, esp)
    after = State.__new__(State)
    after.regs = dict(state.regs)
    # Callee-saved by the cdecl/stdcall/thiscall ABIs this image uses: EBX,
    # ESI, EDI and EBP come back unchanged, and treating them as clobbered
    # would lose every proof whose body reads one after a call.
    after.regs["eax"] = CALLEE_RESULT(target, visible, esp)
    after.regs["edx"] = CALLEE_SECOND(target, visible, esp)
    after.regs["ecx"] = CALLEE_SECOND(target + z3.BitVecVal(1, 32), visible, esp)
    after.flags = CALLEE_FLAGS(target, visible, esp)
    # The callee may write anything, INCLUDING below ESP, and that costs
    # nothing: 32-bit x86 has no red zone, so bytes below ESP are scratch that
    # no correct caller reads after a call. Locals live between ESP and EBP,
    # which is above ESP and therefore preserved or not by the callee exactly
    # as the uninterpreted function says.
    #
    # Splicing the two regions with a Lambda was tried and reverted: it is the
    # more precise statement and it makes Z3 answer `unknown`.
    after.mem = CALLEE_MEMORY(target, visible, esp)
    return after


# ---------------------------------------------------------------------------
# The obligation
#
# Two bodies are equivalent when, started from IDENTICAL state, they agree on
# what a caller can observe:
#
#   the projected return value   EAX, plus EDX for a 64-bit return
#   every byte at or above ESP0  which is everything the caller can still name
#
# The private region is exactly `addr < ESP0`. That is exact rather than
# conservative, and it is why the two sides laying their frames out completely
# differently costs nothing: those bytes are dead the moment either returns.
#
# `forall a >= ESP0` is posed as a FREE variable rather than a z3.ForAll. To
# prove a universal you refute its negation on an arbitrary witness, and the
# free form stays in the decidable fragment - the ForAll form is exactly the
# shape that made Z3 answer `unknown` for the callee canonicalisation.
# ---------------------------------------------------------------------------

PROVED = "PROVED"
REFUTED = "REFUTED"
UNKNOWN = "UNKNOWN"


def equivalence_query(before: State, left: State, right: State,
                      returns_value: bool = True, wide_return: bool = False):
    """The NEGATION of equivalence: satisfiable exactly when they differ."""
    probe = z3.BitVec("obligation_probe", 32)
    differences = []
    if returns_value:
        differences.append(left.regs["eax"] != right.regs["eax"])
        if wide_return:
            differences.append(left.regs["edx"] != right.regs["edx"])
    differences.append(z3.And(
        z3.UGE(probe, before.regs["esp"]),
        z3.Select(left.mem, probe) != z3.Select(right.mem, probe)))
    return z3.Or(*differences)


def prove_equivalent(original: bytes, recovered: bytes, address: int = 0x1000,
                     returns_value: bool = True, wide_return: bool = False,
                     timeout_ms: int = 60000, recovered_address: int = None,
                     receiver_disjoint: bool = True):
    """(verdict, detail) for two bodies started from identical state.

    Returns UNKNOWN rather than guessing, and the caller must treat it as a
    non-result. An `unknown` folded into a boolean reads as a PROOF, which is
    the single most dangerous thing this module could do - it has already
    happened once here, to a check written as `== unsat`.
    """
    before = State()
    left_start, right_start = before.copy(), before.copy()
    try:
        left = encode_function(original, address, left_start)
        # The recovered body is compiled at its own base - section-relative in
        # an object file, nothing to do with the original's canonical address -
        # so its branch targets are only meaningful against that base.
        right = encode_function(recovered,
                                address if recovered_address is None
                                else recovered_address, right_start)
    except Unsupported as reason:
        return UNKNOWN, f"not encodable: {reason}"

    solver = z3.Solver()
    solver.set("timeout", timeout_ms)
    # A STACK POINTER THAT CANNOT WRAP. Without this the solver is free to put
    # ESP0 within a few bytes of zero, where `esp0 - 4` is UNSIGNED-GREATER
    # than esp0 - so a `push` writing the caller's own frame slot lands
    # "above ESP0" and shows up as an observable difference. That produced a
    # spurious counterexample on ?UNK3@TutWin, whose two bodies are plainly
    # equivalent and differ only in that the original builds a frame.
    #
    # The bound is a precondition on the machine, not a weakening of the
    # obligation: no thread in this program runs with a stack pointer in the
    # first 64 KiB or the last, and a proof under it is a proof for every stack
    # that actually exists.
    esp0 = before.regs["esp"]
    solver.add(z3.UGT(esp0, z3.BitVecVal(0x00010000, 32)))
    solver.add(z3.ULT(esp0, z3.BitVecVal(0xF0000000, 32)))
    # THE RECEIVER IS NOT THE ARGUMENT AREA, and without saying so every
    # __thiscall method refutes. Left free, the solver lays the object on top
    # of the caller's own pushed arguments - measured on
    # ?set_text_color@Buffer, where it chose ecx = 0xb04001a8 against
    # esp0 = 0xb04006f4 so that `this + 0x56c` and `[esp0 + 0x10]` are the same
    # bytes. Under that aliasing the two bodies really do differ: the original
    # reads all four arguments interleaved with its stores, ours reads each one
    # immediately before storing it, so a store that clobbers a not-yet-read
    # argument is visible in one and not the other.
    #
    # No real call does this - the arguments a caller pushes are not inside the
    # object it is calling a method on - so it is stated as a precondition
    # rather than left to be rediscovered as fourteen refutations.
    #
    # THE LIMIT, because it is an assumption and not a derivation: the window
    # is a fixed 64 KiB rather than the object's real extent, which is not
    # known here. A stack-allocated receiver genuinely close to ESP is
    # therefore excluded from the theorem rather than proved about, so this
    # narrows what a PROOF covers and can never manufacture one.
    if receiver_disjoint:
        # ONE wrapped difference, bounded on BOTH sides. Written first as
        # `UGT(d, 64K) or UGT(-d, 64K)` it was vacuous: for an object just
        # BELOW the stack, d is a huge unsigned number and the first disjunct
        # is trivially true, so the precondition permitted precisely the
        # aliasing it existed to exclude. The two-sided form says what was
        # meant - far above AND far below.
        distance = before.regs["ecx"] - esp0
        solver.add(z3.UGT(distance, z3.BitVecVal(0x00010000, 32)))
        solver.add(z3.ULT(distance, z3.BitVecVal(0xFFFF0000, 32)))
    solver.add(equivalence_query(before, left, right, returns_value,
                                 wide_return))
    result = solver.check()
    if result == z3.unsat:
        return PROVED, None
    if result == z3.sat:
        # A counterexample on a body believed correct is the most valuable
        # outcome available, so it is returned rather than summarised away.
        return REFUTED, solver.model()
    return UNKNOWN, solver.reason_unknown()
