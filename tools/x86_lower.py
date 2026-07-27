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
    if mem.segment != x86.X86_REG_INVALID:
        # fs:/gs: is the thread block: SEH chains and TLS. Modelling it needs a
        # per-thread region, which is phase 4, so refuse rather than silently
        # lower it as a flat-image access and corrupt the image instead.
        raise Unsupported("segment-relative memory")
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
        return f"opensmacx_mem{width}({memory_address(operand.mem)})"
    raise Unsupported(f"operand type {operand.type}")


def write_operand(operand, value: str) -> str:
    if operand.type == X86_OP_REG:
        return write_register(operand.reg, value)
    if operand.type == X86_OP_MEM:
        width = operand.size * 8
        if width not in (8, 16, 32):
            raise Unsupported(f"{operand.size}-byte memory operand")
        return f"opensmacx_store{width}({memory_address(operand.mem)}, {value});"
    raise Unsupported(f"write to operand type {operand.type}")


def sign_extend(value: str, from_bits: int) -> str:
    if from_bits == 32:
        return value
    return (f"static_cast<uint32_t>(static_cast<int32_t>("
            f"static_cast<int{from_bits}_t>({value})))")


def lower(instruction, label_for) -> list[str]:
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
        return _lower(instruction, label_for)
    except Unsupported as reason:
        text = f"{instruction.mnemonic} {instruction.op_str}".strip()
        raise Unsupported(
            f"{instruction.address:#010x}: {text} ({reason})") from None


def _lower(instruction, label_for) -> list[str]:
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

    if mnemonic == "xchg" and len(operands) == 2:
        return [f"{{ const uint32_t swap = {read_operand(operands[0])};",
                f"  {write_operand(operands[0], read_operand(operands[1]))}",
                f"  {write_operand(operands[1], 'swap')} }}"]

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
             "lea", "push", "pop", "inc", "dec", "neg", "not", "imul", "cdq",
             "cwde", "jmp", "call", "ret", "leave", "xchg"}
    names |= set(ARITH) | set(SHIFT)
    names |= {f"set{name}" for name in CONDITIONS}
    names |= {f"cmov{name}" for name in CONDITIONS}
    names |= {f"j{name}" for name in CONDITIONS}
    return names
