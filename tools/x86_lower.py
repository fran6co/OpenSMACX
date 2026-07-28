#!/usr/bin/env python3
"""Lower decoded x86 instructions to explicit-CPU-state C++.

The micro-pilot in `static_recompile_pilot.py` lowers by matching exact operand
shapes: EAX specifically, `[ebp+8]` specifically, an immediate specifically.
That is the right shape for proving one function and the wrong shape for
724,814 of them - it has one branch per (instruction, operand pattern) pair,
so its size grows with the corpus rather than with the instruction set.

This module inverts that. Operands are read and written through two generic
functions that handle any register, any `base + index*scale + disp` memory
form, and any immediate at any width; each mnemonic then names an operation
over values rather than over addressing modes. The result grows with the
instruction set, which is finite and small: 40 mnemonics cover 99.44% of every
instruction in the image.

Two decisions here are worth stating because they are easy to get quietly
wrong.

**Sub-register writes are merges, not assignments.** Writing AL leaves the top
24 bits of EAX intact and writing AX leaves the top 16 intact, while on this
architecture a 32-bit write is a whole-register replacement. Lowering `mov al,
0` as `eax = 0` is the single most destructive mistake available here and it
would pass any test that only looks at AL.

**Flags are computed from the operation's own inputs.** They cannot be derived
from the result alone: overflow needs both operands' signs, and the carry of a
subtract is a comparison of the inputs. Every ALU lowering therefore passes its
inputs to the flag helper rather than reconstructing them afterwards.
"""

from __future__ import annotations

from capstone import x86
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

# The eight 32-bit general registers, as fields of the state struct.
FULL = {
    x86.X86_REG_EAX: "eax", x86.X86_REG_EBX: "ebx",
    x86.X86_REG_ECX: "ecx", x86.X86_REG_EDX: "edx",
    x86.X86_REG_ESI: "esi", x86.X86_REG_EDI: "edi",
    x86.X86_REG_EBP: "ebp", x86.X86_REG_ESP: "esp",
}

# 16-bit views. A write here preserves the upper half.
HALF = {
    x86.X86_REG_AX: "eax", x86.X86_REG_BX: "ebx",
    x86.X86_REG_CX: "ecx", x86.X86_REG_DX: "edx",
    x86.X86_REG_SI: "esi", x86.X86_REG_DI: "edi",
    x86.X86_REG_BP: "ebp", x86.X86_REG_SP: "esp",
}

# 8-bit views. The `high` ones are bits 8..15, which is why a single "low byte"
# helper is not enough and the shift has to be part of the operand model.
LOW8 = {
    x86.X86_REG_AL: "eax", x86.X86_REG_BL: "ebx",
    x86.X86_REG_CL: "ecx", x86.X86_REG_DL: "edx",
}
HIGH8 = {
    x86.X86_REG_AH: "eax", x86.X86_REG_BH: "ebx",
    x86.X86_REG_CH: "ecx", x86.X86_REG_DH: "edx",
}

CONDITIONS = {
    "e": "opensmacx_zf(s)", "z": "opensmacx_zf(s)",
    "ne": "!opensmacx_zf(s)", "nz": "!opensmacx_zf(s)",
    "s": "opensmacx_sf(s)", "ns": "!opensmacx_sf(s)",
    "o": "opensmacx_of(s)", "no": "!opensmacx_of(s)",
    "b": "opensmacx_cf(s)", "nae": "opensmacx_cf(s)", "c": "opensmacx_cf(s)",
    "ae": "!opensmacx_cf(s)", "nb": "!opensmacx_cf(s)", "nc": "!opensmacx_cf(s)",
    "be": "(opensmacx_cf(s) || opensmacx_zf(s))",
    "a": "(!opensmacx_cf(s) && !opensmacx_zf(s))",
    "l": "(opensmacx_sf(s) != opensmacx_of(s))",
    "ge": "(opensmacx_sf(s) == opensmacx_of(s))",
    "le": "(opensmacx_zf(s) || opensmacx_sf(s) != opensmacx_of(s))",
    "g": "(!opensmacx_zf(s) && opensmacx_sf(s) == opensmacx_of(s))",
    "p": "opensmacx_pf(s)", "pe": "opensmacx_pf(s)",
    "np": "!opensmacx_pf(s)", "po": "!opensmacx_pf(s)",
}

# Mnemonic -> (helper, writes_result). A false `writes_result` is cmp/test:
# same arithmetic, flags only, destination untouched.
ARITH = {
    "add": ("opensmacx_add", True), "sub": ("opensmacx_sub", True),
    "adc": ("opensmacx_adc", True), "sbb": ("opensmacx_sbb", True),
    "and": ("opensmacx_and", True), "or": ("opensmacx_or", True),
    "xor": ("opensmacx_xor", True),
    "cmp": ("opensmacx_sub", False), "test": ("opensmacx_and", False),
}

SHIFT = {"shl": "opensmacx_shl", "sal": "opensmacx_shl",
         "shr": "opensmacx_shr", "sar": "opensmacx_sar",
         "rol": "opensmacx_rol", "ror": "opensmacx_ror"}

# The double shifts, which have no 8-bit form and a third operand. They keep
# their own table because they share nothing with SHIFT above but the word
# "shift": the helper takes a second VALUE, not just a count.
DOUBLE_SHIFT = {"shld": "opensmacx_shld", "shrd": "opensmacx_shrd"}

# The one-byte string opcodes: MOVS, CMPS, STOS, LODS and SCAS, at both widths.
# The OPCODE decides membership, never the mnemonic - `movsd` is also the SSE
# scalar double move and `movsx` is not a string operation at all, so a
# name-based test lowers two unrelated instructions as a dword string copy.
STRING_OPCODES = frozenset({0xA4, 0xA5, 0xA6, 0xA7, 0xAA, 0xAB,
                            0xAC, 0xAD, 0xAE, 0xAF})

# The mnemonic's last letter is its width, at any repeat prefix.
STRING_WIDTHS = {"b": 8, "w": 16, "d": 32}

# Every byte that can precede an opcode. Used to find the repeat prefix in the
# ENCODING, which is the only place it is reliably reported - see the string
# rule in `_lower` for the case that forces this.
PREFIX_BYTES = frozenset({0x26, 0x2E, 0x36, 0x3E, 0x64, 0x65,
                          0x66, 0x67, 0xF0, 0xF2, 0xF3})


class Unsupported(Exception):
    """Raised for an instruction this module does not lower yet.

    Carrying the address and text matters: the caller turns it into a trap that
    names the exact instruction, so an unlowered body reports what it needs
    instead of failing anonymously.
    """


def register_view(reg: int) -> tuple[str, int, int]:
    """(state field, width in bits, bit offset) for a register operand."""
    if reg in FULL:
        return FULL[reg], 32, 0
    if reg in HALF:
        return HALF[reg], 16, 0
    if reg in LOW8:
        return LOW8[reg], 8, 0
    if reg in HIGH8:
        return HIGH8[reg], 8, 8
    raise Unsupported(f"register id {reg}")


def read_register(reg: int) -> str:
    field, width, shift = register_view(reg)
    if width == 32:
        return f"s.{field}"
    mask = (1 << width) - 1
    if shift:
        return f"((s.{field} >> {shift}) & {mask:#x}U)"
    return f"(s.{field} & {mask:#x}U)"


def write_register(reg: int, value: str) -> str:
    """A sub-register write MERGES; only a 32-bit write replaces."""
    field, width, shift = register_view(reg)
    if width == 32:
        return f"s.{field} = {value};"
    mask = ((1 << width) - 1) << shift
    return (f"s.{field} = (s.{field} & {~mask & 0xFFFFFFFF:#010x}U)"
            f" | (({value} << {shift}) & {mask:#010x}U);")


def stack_step(operand) -> int:
    """Bytes a PUSH or POP of this operand moves ESP, which is its width."""
    if operand.size not in (2, 4):
        raise Unsupported(f"{operand.size}-byte stack operand")
    return operand.size


def segment_displacement(mem):
    """The thread-block displacement for an fs:-relative operand, else None.

    Segment-prefixed memory is a DIFFERENT ADDRESS SPACE, not an offset into
    the flat image, so it does not go through `memory_address` at all - see
    `tools/lifted_tls.h` for what conflating the two corrupts.

    Measured over the LIFT SCOPE of terranx.exe - the 5,673 catalogued rows the
    lift plans, `external_library` excluded - by disassembling every body:
    1,632 fs:-prefixed operands in 393 functions, and the shape is unanimous.
    Every one is a `mov`, 32-bit, a bare `fs:[0]` with displacement 0 and no
    base or index: the SEH registration chain head. (315 further operands carry
    an `es:` prefix, but all 315 are the implicit destination of a string
    instruction and have no displacement of their own.) So the shape below is
    the whole segment surface of this image, and every other shape still
    refuses rather than guessing.

    The count used to read 1,330 over 389 functions. The SHAPE claim - which is
    the load-bearing half, because it is why one branch can cover the surface -
    was right, but the tally was not, and it had been copied into
    lifted_oracle_plan.py and lifted_oracle.cpp as "the entire fs: surface".
    Re-derive it with a disassembly of the catalogued bodies, not by hand.
    """
    if mem.segment == x86.X86_REG_INVALID:
        return None
    if mem.segment != x86.X86_REG_FS:
        raise Unsupported("segment-relative memory")
    if mem.base != x86.X86_REG_INVALID or mem.index != x86.X86_REG_INVALID:
        # `fs:[eax]` would be a COMPUTED thread-block offset. It appears zero
        # times here, and lowering it blind would index the thread block with a
        # register that was meant for some other address space.
        raise Unsupported("fs-relative memory with a register term")
    return f"{mem.disp & 0xFFFFFFFF:#010x}U"


def memory_address(mem) -> str:
    """`base + index*scale + disp`, as an expression over the state."""
    if mem.base in HALF or mem.index in HALF:
        # A 0x67 prefix makes the address 16-bit: `[bx + si]` wraps within 64K
        # and is taken from the segment base, so lowering it as 32-bit
        # arithmetic over the low halves of EBX and ESI computes an address
        # that is right only while both stay small. Refuse it for the same
        # reason as segment-relative memory below - modelling it needs the
        # segment, which is phase 4.
        raise Unsupported("16-bit address size")
    if segment_displacement(mem) is not None:
        # The thread block has no flat address, so there is nothing to return:
        # `opensmacx_at` would turn fs:[0] into `opensmacx_image - 0x00400000`.
        # `read_operand` and `write_operand` route segment operands to the TIB
        # helpers before reaching here, so this guards the other callers - the
        # ones that want an ADDRESS rather than an access - instead of refusing
        # fs: itself.
        raise Unsupported("segment-relative memory used as a flat address")
    terms = []
    if mem.base != x86.X86_REG_INVALID:
        terms.append(read_register(mem.base))
    if mem.index != x86.X86_REG_INVALID:
        index = read_register(mem.index)
        terms.append(index if mem.scale == 1 else f"({index} * {mem.scale}U)")
    if mem.disp or not terms:
        terms.append(f"{mem.disp & 0xFFFFFFFF:#010x}U")
    return " + ".join(terms)


def read_operand(operand) -> str:
    if operand.type == X86_OP_REG:
        return read_register(operand.reg)
    if operand.type == X86_OP_IMM:
        # Narrow to the operand's own width, the same invariant `read_register`
        # keeps: everything handed to an N-bit helper is already N bits wide.
        # Capstone reports the sign-extended imm8 encodings as negative, so a
        # blanket 32-bit mask would give `cmp ax, -1` a right operand of
        # 0xffffffff while `cmp ax, 0xffff` - the identical operation, one
        # encoding longer - gets 0x0000ffff.
        width = operand.size * 8 if operand.size in (1, 2) else 32
        return f"{operand.imm & ((1 << width) - 1):#010x}U"
    if operand.type == X86_OP_MEM:
        width = operand.size * 8
        if width not in (8, 16, 32):
            raise Unsupported(f"{operand.size}-byte memory operand")
        displacement = segment_displacement(operand.mem)
        if displacement is not None:
            return f"opensmacx_fs{width}({displacement})"
        return f"opensmacx_mem{width}({memory_address(operand.mem)})"
    raise Unsupported(f"operand type {operand.type}")


def write_operand(operand, value: str) -> str:
    if operand.type == X86_OP_REG:
        return write_register(operand.reg, value)
    if operand.type == X86_OP_MEM:
        width = operand.size * 8
        if width not in (8, 16, 32):
            raise Unsupported(f"{operand.size}-byte memory operand")
        displacement = segment_displacement(operand.mem)
        if displacement is not None:
            return f"opensmacx_store_fs{width}({displacement}, {value});"
        return f"opensmacx_store{width}({memory_address(operand.mem)}, {value});"
    raise Unsupported(f"write to operand type {operand.type}")


def sign_extend(value: str, from_bits: int) -> str:
    if from_bits == 32:
        return value
    return (f"static_cast<uint32_t>(static_cast<int32_t>("
            f"static_cast<int{from_bits}_t>({value})))")


# ---------------------------------------------------------------------------
# x87
#
# The x87 is a second machine: eight registers addressed RELATIVE to a
# top-of-stack index that the instructions themselves move. None of it fits the
# operand model above - there is no state field to write and no width to mask -
# so every rule here is a call into `lifted_x87.h`, and the only thing lowered
# locally is which ST index, or which address.
#
# Capstone's operand FORM carries the encoding asymmetry, which is what lets
# these rules be this short. `fmul st(3)` with one operand is the D8 row and
# means ST(0) *= ST(3); `fmul st(3), st(0)` with two is the DC row and means
# ST(3) *= ST(0). And DC E0+i disassembles as FSUBR where D8 E0+i disassembles
# as FSUB - the rows are not parallel - so the mnemonic arriving here is
# already the true operation rather than the opcode's position in a table.
# ---------------------------------------------------------------------------

X87_OPS = {"fadd": "OpensmacxX87Add", "fmul": "OpensmacxX87Mul",
           "fsub": "OpensmacxX87Sub", "fsubr": "OpensmacxX87Subr",
           "fdiv": "OpensmacxX87Div", "fdivr": "OpensmacxX87Divr"}

# The popping forms and the integer-memory forms are the same six operations
# reached by a different encoding, so they map onto the same six names.
X87_POP_OPS = {"faddp": "fadd", "fmulp": "fmul", "fsubp": "fsub",
               "fsubrp": "fsubr", "fdivp": "fdiv", "fdivrp": "fdivr"}
X87_INTEGER_OPS = {"fiadd": "fadd", "fimul": "fmul", "fisub": "fsub",
                   "fisubr": "fsubr", "fidiv": "fdiv", "fidivr": "fdivr"}

X87_NULLARY = {
    "fchs": "opensmacx_x87_chs();", "fabs": "opensmacx_x87_abs();",
    "fsqrt": "opensmacx_x87_sqrt();", "frndint": "opensmacx_x87_rndint();",
    "fsin": "opensmacx_x87_sin();", "fcos": "opensmacx_x87_cos();",
    "fpatan": "opensmacx_x87_patan();", "ftst": "opensmacx_x87_ftst();",
    "fninit": "opensmacx_x87_fninit();", "fnclex": "opensmacx_x87_fnclex();",
    "fld1": "opensmacx_x87_fld1();", "fldz": "opensmacx_x87_fldz();",
    "fldpi": "opensmacx_x87_fldpi();", "fldl2e": "opensmacx_x87_fldl2e();",
    "fldl2t": "opensmacx_x87_fldl2t();", "fldlg2": "opensmacx_x87_fldlg2();",
    "fldln2": "opensmacx_x87_fldln2();",
    # FWAIT only waits for a pending unmasked exception, and this image masks
    # every one of them, so there is nothing to model. 81 in the image.
    "wait": "", "fwait": "",
}

ST_REGISTERS = {getattr(x86, f"X86_REG_ST{index}"): index for index in range(8)}


def x87_index(operand) -> int:
    if operand.type != X86_OP_REG or operand.reg not in ST_REGISTERS:
        raise Unsupported("x87 operand is not a stack register")
    return ST_REGISTERS[operand.reg]


def x87_value(operand, integer: bool = False) -> str:
    """The operand's VALUE as an x87 register value - every binary form."""
    if operand.type == X86_OP_REG:
        if integer:
            raise Unsupported("integer x87 form with a register operand")
        return f"opensmacx_x87_get({x87_index(operand)}U)"
    if operand.type != X86_OP_MEM:
        raise Unsupported(f"x87 operand type {operand.type}")
    width = operand.size * 8
    allowed = (16, 32, 64) if integer else (32, 64, 80)
    if width not in allowed:
        raise Unsupported(f"{operand.size}-byte x87 memory operand")
    prefix = "imem" if integer else "mem"
    return f"opensmacx_x87_{prefix}{width}({memory_address(operand.mem)})"


def x87_address(operand) -> str:
    if operand.type != X86_OP_MEM:
        raise Unsupported("x87 form needs a memory operand")
    return memory_address(operand.mem)


def _lower_x87(instruction) -> list[str]:
    mnemonic = instruction.mnemonic
    operands = instruction.operands

    if mnemonic in X87_NULLARY and not operands:
        statement = X87_NULLARY[mnemonic]
        return [statement] if statement else []

    if mnemonic == "fcompp" and not operands:
        return ["opensmacx_x87_fcom(opensmacx_x87_get(1U), 2U);"]

    if mnemonic == "fnstsw" and len(operands) == 1:
        # The register form writes AX, and a 16-bit write is a MERGE - so it
        # goes through `write_operand` rather than assigning `s.eax`, exactly
        # like every other sub-register destination in this module.
        if operands[0].type == X86_OP_REG:
            return [write_operand(operands[0], "opensmacx_x87_status_word()")]
        return [f"opensmacx_x87_fnstsw_mem({x87_address(operands[0])});"]

    if mnemonic in ("fldcw", "fnstcw") and len(operands) == 1:
        return [f"opensmacx_x87_{mnemonic}({x87_address(operands[0])});"]

    if mnemonic == "fxch":
        # Capstone spells this `fxch st(0), st(i)`; the no-operand encoding
        # means ST(1).
        if len(operands) == 2:
            index = x87_index(operands[1])
        elif len(operands) == 1:
            index = x87_index(operands[0])
        else:
            index = 1
        return [f"opensmacx_x87_fxch({index}U);"]

    if mnemonic == "fld" and len(operands) == 1:
        if operands[0].type == X86_OP_REG:
            return [f"opensmacx_x87_fld_st({x87_index(operands[0])}U);"]
        width = operands[0].size * 8
        if width not in (32, 64, 80):
            raise Unsupported(f"{operands[0].size}-byte fld operand")
        return [f"opensmacx_x87_fld{width}({x87_address(operands[0])});"]

    if mnemonic == "fild" and len(operands) == 1:
        width = operands[0].size * 8
        if width not in (16, 32, 64):
            raise Unsupported(f"{operands[0].size}-byte fild operand")
        return [f"opensmacx_x87_fild{width}({x87_address(operands[0])});"]

    if mnemonic in ("fst", "fstp") and len(operands) == 1:
        pop = "true" if mnemonic == "fstp" else "false"
        if operands[0].type == X86_OP_REG:
            return [f"opensmacx_x87_fst_st({x87_index(operands[0])}U, {pop});"]
        width = operands[0].size * 8
        if width not in (32, 64, 80):
            raise Unsupported(f"{operands[0].size}-byte {mnemonic} operand")
        return [f"opensmacx_x87_fst{width}("
                f"{x87_address(operands[0])}, {pop});"]

    if mnemonic in ("fist", "fistp") and len(operands) == 1:
        pop = "true" if mnemonic == "fistp" else "false"
        width = operands[0].size * 8
        if width not in (16, 32, 64):
            raise Unsupported(f"{operands[0].size}-byte {mnemonic} operand")
        return [f"opensmacx_x87_fist{width}("
                f"{x87_address(operands[0])}, {pop});"]

    if mnemonic == "ffree" and len(operands) == 1:
        return [f"opensmacx_x87_ffree({x87_index(operands[0])}U, false);"]

    if mnemonic in ("fcom", "fcomp", "ficom", "ficomp") and len(operands) == 1:
        pops = 1 if mnemonic.endswith("p") else 0
        integer = mnemonic.startswith("fi")
        return [f"opensmacx_x87_fcom({x87_value(operands[0], integer)},"
                f" {pops}U);"]

    if mnemonic in X87_OPS and len(operands) in (1, 2):
        operation = X87_OPS[mnemonic]
        if len(operands) == 1:
            # ST(0) <- ST(0) op source. Both the memory forms and the D8 row.
            return [f"opensmacx_x87_binary_st0({operation},"
                    f" {x87_value(operands[0])});"]
        # The DC row: ST(i) <- ST(i) op ST(0). The second operand is always
        # ST(0) in this encoding; refusing anything else means a future
        # capstone that spells the D8 row with two operands becomes a trap
        # rather than a silently reversed destination.
        if x87_index(operands[1]) != 0:
            raise Unsupported("two-operand x87 form whose source is not ST(0)")
        return [f"opensmacx_x87_binary_sti({operation},"
                f" {x87_index(operands[0])}U, false);"]

    if mnemonic in X87_POP_OPS:
        operation = X87_OPS[X87_POP_OPS[mnemonic]]
        # The same guard the two-operand branch above carries, and for the same
        # reason: `faddp st(2), st(0)` is the only shape capstone emits today,
        # so a second operand that is not ST(0) is a disassembler this rule was
        # not written against and must trap rather than pick an index.
        if len(operands) == 2 and x87_index(operands[1]) != 0:
            raise Unsupported("popping x87 form whose source is not ST(0)")
        index = x87_index(operands[0]) if operands else 1
        return [f"opensmacx_x87_binary_sti({operation}, {index}U, true);"]

    if mnemonic in X87_INTEGER_OPS and len(operands) == 1:
        operation = X87_OPS[X87_INTEGER_OPS[mnemonic]]
        return [f"opensmacx_x87_binary_st0({operation},"
                f" {x87_value(operands[0], True)});"]

    raise Unsupported("no lowering for this x87 mnemonic")


def lower(instruction, label_for, case_targets=None) -> list[str]:
    """Statements implementing one instruction.

    `label_for(address)` returns a local label when the address is a branch
    target inside this same function, and None when it is not - control leaving
    the function is a different lowering (a dispatch) from control staying in
    it (a goto).

    Every refusal is re-raised carrying the address and text. The operand model
    refuses from three levels down and knows only what it was looking at -
    "register id 50", "segment-relative memory" - which names nothing among
    724,814 instructions; the trap the caller emits needs the instruction.
    """
    try:
        return _lower(instruction, label_for, case_targets)
    except Unsupported as reason:
        text = f"{instruction.mnemonic} {instruction.op_str}".strip()
        raise Unsupported(
            f"{instruction.address:#010x}: {text} ({reason})") from None


def _lower(instruction, label_for, case_targets=None) -> list[str]:
    mnemonic = instruction.mnemonic
    operands = instruction.operands
    width = operands[0].size * 8 if operands else 32

    if mnemonic in ("nop", "hint_nop", "fnop"):
        return []

    if mnemonic == "mov" and len(operands) == 2:
        return [write_operand(operands[0], read_operand(operands[1]))]

    if mnemonic in ("movsx", "movsxd") and len(operands) == 2:
        source_bits = operands[1].size * 8
        return [write_operand(
            operands[0], sign_extend(read_operand(operands[1]), source_bits))]

    if mnemonic == "movzx" and len(operands) == 2:
        return [write_operand(operands[0], read_operand(operands[1]))]

    if mnemonic == "lea" and len(operands) == 2:
        # LEA computes the address and never dereferences, which is why it is
        # the one memory-form instruction that must NOT go through mem32.
        return [write_operand(operands[0], memory_address(operands[1].mem))]

    if mnemonic == "push" and len(operands) == 1:
        # The width is the OPERAND's, not a constant 4: `push ax` under the
        # 0x66 prefix moves ESP by two and writes two bytes, and hardcoding
        # four both stores the wrong number of bytes and leaves the stack
        # misaligned against every later access. Capstone reports `push imm`
        # as a 4-byte operand in 32-bit mode, so only the explicit 16-bit
        # forms take the narrow path.
        step = stack_step(operands[0])
        # The source is read with the ESP the instruction started with: `push
        # esp` stores the pre-decrement pointer, and `push [esp+4]` addresses
        # the slot above the one about to be written. Writing to `s.esp - N`
        # and adjusting afterwards is the mirror of the `pop` case below, and
        # gets both without a temporary.
        return [f"opensmacx_store{step * 8}(s.esp - {step}U,"
                f" {read_operand(operands[0])});",
                f"s.esp -= {step}U;"]

    if mnemonic == "pop" and len(operands) == 1:
        # Read before adjusting: `pop esp` is legal and must load the stored
        # value, not the incremented pointer. The width is the operand's, for
        # the same reason as push.
        step = stack_step(operands[0])
        return [f"{{ const uint32_t popped = opensmacx_mem{step * 8}(s.esp);",
                f"  s.esp += {step}U;",
                f"  {write_operand(operands[0], 'popped')} }}"]

    if mnemonic in ARITH and len(operands) == 2:
        helper, writes = ARITH[mnemonic]
        left, right = read_operand(operands[0]), read_operand(operands[1])
        # An 8- or 16-bit immediate is sign-extended to the operand width
        # before the operation, so `add eax, -1` is not `add eax, 0xFF`.
        if operands[1].type == X86_OP_IMM and operands[1].size < operands[0].size:
            right = sign_extend(right, operands[1].size * 8)
        call = f"{helper}{width}(s, {left}, {right})"
        return [write_operand(operands[0], call)] if writes else [f"(void){call};"]

    if mnemonic in ("inc", "dec") and len(operands) == 1:
        # INC and DEC deliberately preserve CF. Lowering them as add/sub 1
        # would clobber it, and the bug only shows through a later carry-based
        # branch, a long way from here.
        helper = "opensmacx_inc" if mnemonic == "inc" else "opensmacx_dec"
        return [write_operand(
            operands[0], f"{helper}{width}(s, {read_operand(operands[0])})")]

    if mnemonic in ("neg", "not") and len(operands) == 1:
        helper = "opensmacx_neg" if mnemonic == "neg" else "opensmacx_not"
        return [write_operand(
            operands[0], f"{helper}{width}(s, {read_operand(operands[0])})")]

    if mnemonic in DOUBLE_SHIFT and len(operands) == 3:
        # SHLD/SHRD. Capstone reports them in Intel order - (destination,
        # source, count) - with the count either an imm8 or CL, and it is the
        # FIRST operand that is read AND written, which is what separates them
        # from the other three-operand instruction here, the IMUL below, whose
        # destination is write-only. Reading operand 1 as the destination
        # would shift the wrong register by the right amount.
        helper = DOUBLE_SHIFT[mnemonic]
        if width not in (16, 32):
            # There is no 8-bit double shift on x86, so any other width is a
            # decode this rule was not written against.
            raise Unsupported(f"{width}-bit double shift")
        return [write_operand(
            operands[0],
            f"{helper}{width}(s, {read_operand(operands[0])},"
            f" {read_operand(operands[1])}, {read_operand(operands[2])})")]

    if mnemonic in SHIFT and len(operands) == 2:
        helper = SHIFT[mnemonic]
        return [write_operand(operands[0],
                              f"{helper}{width}(s, {read_operand(operands[0])},"
                              f" {read_operand(operands[1])})")]

    if mnemonic == "imul":
        if len(operands) == 1:
            return [f"opensmacx_imul1_{width}(s, {read_operand(operands[0])});"]
        if len(operands) == 2:
            return [write_operand(
                operands[0],
                f"opensmacx_imul{width}(s, {read_operand(operands[0])},"
                f" {read_operand(operands[1])})")]
        if len(operands) == 3:
            return [write_operand(
                operands[0],
                f"opensmacx_imul{width}(s, {read_operand(operands[1])},"
                f" {read_operand(operands[2])})")]

    if mnemonic == "mul" and len(operands) == 1:
        return [f"opensmacx_mul1_{width}(s, {read_operand(operands[0])});"]

    if mnemonic in ("div", "idiv") and len(operands) == 1:
        # The address travels with the call because a bad divide is a #DE, and
        # a trap that cannot name the instruction is a trap nobody can act on.
        return [f"opensmacx_{mnemonic}1_{width}(s, {read_operand(operands[0])},"
                f" {instruction.address:#010x}U);"]

    if mnemonic == "cdq":
        return ["s.edx = (s.eax & 0x80000000U) ? 0xFFFFFFFFU : 0U;"]
    if mnemonic == "cwde":
        return [write_register(x86.X86_REG_EAX,
                               sign_extend("(s.eax & 0xffffU)", 16))]

    if mnemonic.startswith("set") and len(operands) == 1:
        condition = CONDITIONS.get(mnemonic[3:])
        if condition:
            return [write_operand(operands[0], f"({condition} ? 1U : 0U)")]

    if mnemonic.startswith("cmov") and len(operands) == 2:
        condition = CONDITIONS.get(mnemonic[4:])
        if condition:
            return [f"if ({condition}) {{ "
                    f"{write_operand(operands[0], read_operand(operands[1]))} }}"]

    if mnemonic == "jmp" and len(operands) == 1:
        if operands[0].type == X86_OP_IMM:
            label = label_for(operands[0].imm)
            if label:
                return [f"goto {label};"]
            # A jump out of the function is a tail call: transfer, then return
            # whatever it left in the state.
            return [f"opensmacx_dispatch({operands[0].imm:#010x}U)(s);",
                    "return;"]
        # A switch. `case_targets` is the table the caller recovered for THIS
        # jump, in index order, so case k is the arm for index k - which is
        # what `[reg*4 + T]` means. Every arm is interior to this function, so
        # each is a goto; the dispatch below stays as the default, because it
        # holds function STARTS only and would trap on all of them. Falling out
        # of the switch is therefore an out-of-range index failing loudly,
        # exactly as it does today, instead of running on into the next case.
        if case_targets and operands[0].type == X86_OP_MEM:
            lines = [f"switch ({read_register(operands[0].mem.index)}) {{"]
            for case, target in enumerate(case_targets):
                label = label_for(target)
                if label is None:
                    # The recovered target is not a labelled instruction start
                    # in this function. Emitting `goto` to nothing would not
                    # compile and guessing would be worse, so refuse the whole
                    # instruction and let it become a trap that names it.
                    raise Unsupported(
                        f"case {case} target {target:#010x} has no label")
                lines.append(f"case {case}U: goto {label};")
            lines.append("}")
            return lines + [
                f"opensmacx_dispatch({read_operand(operands[0])})(s);",
                "return;"]
        return [f"opensmacx_dispatch({read_operand(operands[0])})(s);",
                "return;"]

    if mnemonic.startswith("j") and len(operands) == 1:
        condition = CONDITIONS.get(mnemonic[1:])
        if condition and operands[0].type == X86_OP_IMM:
            label = label_for(operands[0].imm)
            if label:
                return [f"if ({condition}) goto {label};"]
            return [f"if ({condition}) {{ "
                    f"opensmacx_dispatch({operands[0].imm:#010x}U)(s); return; }}"]

    if mnemonic == "call" and len(operands) == 1:
        # The return address is pushed for real. Code that reads it back off
        # the stack - and this image does, for the thunks that adjust `this` -
        # sees the value the original would have seen.
        retaddr = f"{instruction.address + instruction.size:#010x}U"
        if operands[0].type == X86_OP_IMM:
            # A literal target cannot observe ESP, so the order is free here.
            return ["s.esp -= 4U;",
                    f"opensmacx_store32(s.esp, {retaddr});",
                    f"opensmacx_dispatch({operands[0].imm:#010x}U)(s);"]
        # An indirect target CAN observe ESP: `call [esp+4]` names a slot
        # relative to the pre-push pointer, and `call esp` reads the pointer
        # itself. Materialise the target first, exactly as `push` reads its
        # source first: after the decrement, `[esp+4]` names the slot four
        # bytes BELOW the one meant - a word that is still a plausible
        # pointer, so the call lands in whatever function that word happens to
        # name instead of failing where the mistake is.
        return [f"{{ const uint32_t target = {read_operand(operands[0])};",
                f"  opensmacx_store32(s.esp - 4U, {retaddr});",
                "  s.esp -= 4U;",
                "  opensmacx_dispatch(target)(s); }"]

    if mnemonic == "ret":
        pop = operands[0].imm if operands else 0
        return [f"s.esp += {4 + pop}U;", "return;"]

    if mnemonic == "leave":
        return ["s.esp = s.ebp;",
                "s.ebp = opensmacx_mem32(s.esp);",
                "s.esp += 4U;"]

    if mnemonic in ("cld", "std"):
        return [f"opensmacx_{mnemonic}(s);"]

    if instruction.opcode[0] in STRING_OPCODES:
        # A string operation has no operands to lower: ESI, EDI, ECX and the
        # accumulator are implicit, and the direction comes from DF. The whole
        # instruction is therefore its NAME plus its repeat prefix.
        if 0x67 in instruction.prefix:
            # The address-size prefix substitutes CX/SI/DI for their 32-bit
            # selves, which wraps at 64K within a segment - refused for the
            # same reason as 16-bit addressing above.
            raise Unsupported("16-bit address size")
        if instruction.prefix[1] in (0x64, 0x65):
            # An fs:/gs: override on the SOURCE puts it in the thread block,
            # which the operand model routes to separate accessors for every
            # other instruction and which the implicit-operand form would
            # otherwise smuggle past. cs:/ss:/ds:/es: are deliberately NOT
            # refused here: this is a flat model, those four have the same
            # base, and the asymmetry with `memory_address` is that decision
            # rather than an oversight.
            raise Unsupported("segment-relative string source")
        words = mnemonic.split()
        base, suffix = words[-1][:-1], words[-1][-1:]
        # The repeat comes from the ENCODING, not the printed name. Capstone
        # reports `f2 a5` as a bare "movsd" with the prefix array cleared - it
        # consumes the F2 but does not report it, because the mnemonic collides
        # with the SSE movsd - so trusting the name there lowers a whole-buffer
        # copy as a single element. It also prints F2 on a moving op as
        # "repne", for which no helper exists and none should: on MOVS/STOS/
        # LODS the hardware treats F2 exactly as REP, which is what the F2
        # group of the differential checks against the real instruction.
        found = None
        for byte in instruction.bytes:
            if byte in (0xF2, 0xF3):
                found = byte
            elif byte not in PREFIX_BYTES:
                break
        comparing = base in ("scas", "cmps")
        if found == 0xF3:
            repeat = "repe_" if comparing else "rep_"
        elif found == 0xF2:
            repeat = "repne_" if comparing else "rep_"
        else:
            repeat = ""
        if suffix in STRING_WIDTHS:
            return [f"opensmacx_{repeat}{base}{STRING_WIDTHS[suffix]}(s);"]

    if mnemonic == "xchg" and len(operands) == 2:
        return [f"{{ const uint32_t swap = {read_operand(operands[0])};",
                f"  {write_operand(operands[0], read_operand(operands[1]))}",
                f"  {write_operand(operands[1], 'swap')} }}"]

    # Everything left beginning with `f` is x87, plus FWAIT which spells
    # itself `wait`. No integer mnemonic in 32-bit x86 starts with `f`.
    if mnemonic.startswith("f") or mnemonic in ("wait", "fwait"):
        return _lower_x87(instruction)

    raise Unsupported("no lowering for this mnemonic")


def candidate_mnemonics() -> set[str]:
    """Mnemonics `lower` has a rule for - an UPPER BOUND, not coverage.

    Membership here means only that the mnemonic reaches a lowering rule. It
    does not mean the instruction lowers: the operand model refuses segment
    overrides, 16-bit addressing, control registers and 16-bit stack widths,
    all of which arrive under mnemonics in this set. `mov` is in it, and `mov
    eax, fs:[0]` is refused.

    A coverage number keyed on this set therefore overstates the lift, and by
    an amount no test of the set can measure. The only honest coverage metric
    is to call `lower` on every decoded instruction and count the refusals,
    which is what `lift_whole_image.py` does; this set exists as a cheap
    pre-filter for tools that want to triage a mnemonic histogram without
    decoding operands.
    """
    names = {"nop", "hint_nop", "fnop", "mov", "movsx", "movsxd", "movzx",
             "lea", "push", "pop", "inc", "dec", "neg", "not", "imul", "mul",
             "div", "idiv", "cdq",
             "cwde", "jmp", "call", "ret", "leave", "xchg"}
    names |= set(ARITH) | set(SHIFT) | set(DOUBLE_SHIFT)
    names |= {"cld", "std"}
    names |= {f"{repeat}{base}{suffix}"
              for repeat in ("", "rep ", "repe ", "repne ")
              for base in ("movs", "stos", "lods", "scas", "cmps")
              for suffix in STRING_WIDTHS}
    names |= set(X87_OPS) | set(X87_POP_OPS) | set(X87_INTEGER_OPS)
    names |= set(X87_NULLARY)
    names |= {"fcompp", "fnstsw", "fldcw", "fnstcw", "fxch", "fld", "fild",
              "fst", "fstp", "fist", "fistp", "ffree", "fcom", "fcomp",
              "ficom", "ficomp"}
    names |= {f"set{name}" for name in CONDITIONS}
    names |= {f"cmov{name}" for name in CONDITIONS}
    names |= {f"j{name}" for name in CONDITIONS}
    return names
