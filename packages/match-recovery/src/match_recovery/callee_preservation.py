"""Per-callee proof that a direct callee preserves one 32-bit register.

The path-bounded switch proof (see `associated_binary`) may carry a tracked
register value across a direct `call rel32` only when this module proves,
from the callee's own pinned image bytes, that the callee hands that register
back unchanged. No calling-convention (callee-saved) assumption is made.

A callee preserves register R when, on every state reachable from its entry:

* R is never written while it still holds the caller's value, or
* R is first saved by `push R` and every return happens after the matching
  `pop R` restored it from that exact stack slot.

Stack depth is tracked exactly relative to the callee entry: PUSH/POP,
ADD/SUB ESP,imm, the EBP frame (`mov ebp,esp`, `mov esp,ebp`,
`lea esp,[ebp+d]`, `leave`) and the stack effect of nested direct calls. A
nested callee's effect is the one RET immediate shared by every return
reachable in its own bytes (direct jumps and path-bounded jump tables); any
unresolved code or non-standard ESP write there leaves it unknown. That
nested callee's internal balance is its encoded return contract and is not
re-proved. The one non-standard stack helper admitted is the VC6 stack probe
(`_STACK_PROBE`), recognized only by exact unrelocated byte equality; its
effect (ESP lowered by the caller's EAX, which must be a tracked
`mov eax,imm` constant) is derived below. Indirect calls and unknown effects
leave ESP unknown until an EBP restore re-anchors it.

The save slot must stay live (ESP never rises above it while R is
clobbered) and no fixed-displacement ESP/EBP-relative store may overlap it;
such a store from an unknown frame base refuses. While R holds the caller's
value every nested direct call must itself be proved preserving R
(recursively, depth-bounded; recursion refuses). Once R is clobbered under a
live save, nested calls need only a stack effect.

Indirect jumps are followed only through `jmp [r*4+table]` whose index is
bounded on the path by an unsigned `cmp r,imm` edge (optionally through one
byte selector map); every table cell must carry a HIGHLOW relocation.

Everything else refuses: undecodable or non-executable bytes, indirect calls
while R holds the caller's value (imports included), unbounded indirect
jumps, interrupts and other unsupported transfers, a restore at an unknown or
wrong stack depth, recursion and the search/depth bounds. Stores through
computed pointers (including indexed frame addresses) are outside this model.
"""

from __future__ import annotations

from capstone import (
    CS_AC_WRITE, CS_ARCH_X86, CS_GRP_BRANCH_RELATIVE, CS_GRP_CALL, CS_GRP_INT, CS_GRP_IRET,
    CS_GRP_JUMP, CS_GRP_RET, CS_MODE_32, Cs, x86,
)

from .model import Span
from .pe_coff import PeImage


_ENGINE = Cs(CS_ARCH_X86, CS_MODE_32)
_ENGINE.detail = True
_CODE = 0x00000020
_EXECUTE = 0x20000000
PRESERVATION_DEPTH_LIMIT = 8
PRESERVATION_STATE_LIMIT = 20000
_FAMILIES = {
    "al": "eax", "ah": "eax", "ax": "eax", "eax": "eax",
    "bl": "ebx", "bh": "ebx", "bx": "ebx", "ebx": "ebx",
    "cl": "ecx", "ch": "ecx", "cx": "ecx", "ecx": "ecx",
    "dl": "edx", "dh": "edx", "dx": "edx", "edx": "edx",
    "si": "esi", "esi": "esi", "di": "edi", "edi": "edi",
    "bp": "ebp", "ebp": "ebp", "sp": "esp", "esp": "esp",
}
_PRESERVABLE = frozenset(("eax", "ebx", "ecx", "edx", "esi", "edi", "ebp"))
_ORIGINAL = "original"
_UNSUPPORTED = frozenset(("hlt", "ud2", "into", "sysenter", "syscall", "enter",
                          "lcall", "ljmp", "retf", "iretd"))
# VC6 `_chkstk` (position independent, no relocations):
#   push ecx; cmp eax,1000h; lea ecx,[esp+8]; jb last
#   probe: sub ecx,1000h; sub eax,1000h; test [ecx],eax; cmp eax,1000h; jae probe
#   last: sub ecx,eax; mov eax,esp; test [ecx],eax; mov esp,ecx
#         mov ecx,[eax]; mov eax,[eax+4]; push eax; ret
# With E the caller's ESP before the CALL and N its EAX: `lea ecx,[esp+8]`
# is E, the loop lowers ECX and EAX in step, so `mov esp,ecx` sets ESP = E-N;
# ECX and the return address are reloaded from the old frame and
# `push eax; ret` returns with ESP = E-N. Only EAX and flags change.
_STACK_PROBE = bytes.fromhex(
    "513d001000008d4c2408721481e9001000002d0010000085013d0010000073ec"
    "2bc88bc485018be18b088b400450c3")
_STACK_PROBE_PRESERVES = frozenset(("ebx", "ecx", "edx", "esi", "edi", "ebp"))
_PROBE = "probe"


class _Refusal(ValueError):
    pass


class _ContextRefusal(_Refusal):
    """A depth/cycle refusal that depends on the query's call chain."""


class _Context:
    """Decoded-instruction, verdict and stack-effect caches for one image."""

    def __init__(self, image: PeImage):
        self.image = image
        self.decoded: dict[int, object] = {}
        self.verdicts: dict[tuple[int, str], str] = {}
        self.effects: dict[int, int | str | None] = {}
        self.taint = 0  # count of depth/call-chain refusals met so far

    def _code_region(self, address: int):
        matches = [(region, size, flags) for (region, _, size), flags
                   in zip(self.image.sections, self.image.section_flags)
                   if region.start <= address < region.start + size]
        if len(matches) != 1:
            raise _Refusal(f"callee code {address:#x} is not file-backed image storage")
        region, size, flags = matches[0]
        if not flags & _CODE or not flags & _EXECUTE:
            raise _Refusal(f"callee code {address:#x} is not executable image code")
        return region, size

    def decode(self, address: int):
        cached = self.decoded.get(address)
        if cached is not None:
            return cached
        region, size = self._code_region(address)
        end = min(address + 15, region.start + size)
        decoded = next(_ENGINE.disasm(self.image.read(Span(address, end)), address, 1), None)
        if decoded is None:
            raise _Refusal(f"callee code {address:#x} does not decode")
        self.decoded[address] = decoded
        return decoded

    def is_stack_probe(self, entry: int) -> bool:
        try:
            region, size = self._code_region(entry)
        except _Refusal:
            return False
        end = entry + len(_STACK_PROBE)
        if end > region.start + size:
            return False
        return (self.image.read(Span(entry, end)) == _STACK_PROBE
                and not any(entry <= site < end for site in self.image.relocations))

    def table_targets(self, instruction, bounds: frozenset) -> tuple[int, ...]:
        """Targets of `jmp [r*4 + table]` whose index is bounded on this path."""
        operands = instruction.operands
        if (instruction.id != x86.X86_INS_JMP or len(operands) != 1
                or operands[0].type != x86.X86_OP_MEM or operands[0].size != 4):
            raise _Refusal(f"callee reaches an unbounded indirect jump at {instruction.address:#x}")
        memory = operands[0].mem
        maximum = dict(bounds).get(_family(instruction, memory.index)) if memory.index else None
        if (memory.base or memory.scale != 4 or memory.segment != x86.X86_REG_INVALID
                or maximum is None or maximum > _MAX_TABLE_INDEX):
            raise _Refusal(f"callee reaches an unbounded indirect jump at {instruction.address:#x}")
        table = memory.disp & 0xFFFFFFFF
        targets = []
        for index in range(maximum + 1):
            cell = table + 4 * index
            if cell not in self.image.relocations:
                raise _Refusal(f"callee jump table cell {cell:#x} lacks a HIGHLOW relocation")
            targets.append(int.from_bytes(self.image.read(Span(cell, cell + 4)), "little"))
        return tuple(targets)


_CONTEXTS: dict[int, tuple[PeImage, _Context]] = {}


def _context(image: PeImage) -> _Context:
    cached = _CONTEXTS.get(id(image))
    if cached is not None and cached[0] is image:
        return cached[1]
    if len(_CONTEXTS) > 8:
        _CONTEXTS.clear()
    context = _Context(image)
    _CONTEXTS[id(image)] = (image, context)
    return context


def _family(instruction, register: int) -> str | None:
    return _FAMILIES.get(instruction.reg_name(register))


def _written_families(instruction) -> set[str]:
    try:
        _, writes = instruction.regs_access()
    except Exception as error:
        raise _Refusal(f"cannot establish register writes at {instruction.address:#x}") from error
    families = set()
    for register in writes:
        family = _family(instruction, register)
        if family is not None:
            families.add(family)
    return families


def _direct_target(instruction) -> int | None:
    operands = instruction.operands
    if len(operands) == 1 and operands[0].type == x86.X86_OP_IMM:
        return operands[0].imm & 0xFFFFFFFF
    return None


def _frame_store_overlaps(instruction, esp: int | None, ebp: int | None, slot: int) -> str | None:
    """Why an explicit ESP/EBP-relative store may overlap the save slot, if it may."""
    for operand in instruction.operands:
        if operand.type != x86.X86_OP_MEM or not operand.access & CS_AC_WRITE:
            continue
        memory = operand.mem
        base = _family(instruction, memory.base) if memory.base else None
        if base not in ("esp", "ebp") or memory.index:
            # Indexed and other computed addresses are pointer stores,
            # outside this model like every store through a computed pointer.
            continue
        if memory.segment != x86.X86_REG_INVALID:
            return "stores through a segment-relative frame address"
        anchor = esp if base == "esp" else ebp
        if anchor is None:
            return f"stores relative to an unknown {base} depth"
        start = anchor + memory.disp
        if start < slot + 4 and slot < start + operand.size:
            return "overwrites the save slot"
    return None


def _stack_step(instruction, esp: int | None, ebp: int | None) -> tuple[int | None, int | None]:
    """Track ESP/EBP (relative to callee entry) across one non-control instruction."""
    mnemonic = instruction.mnemonic
    operands = instruction.operands
    written = _written_families(instruction)
    new_esp, new_ebp = esp, ebp
    if mnemonic == "leave":
        return (None if ebp is None else ebp + 4), None
    if mnemonic in ("push", "pop") and len(operands) == 1:
        size = operands[0].size if mnemonic == "pop" else -operands[0].size
        new_esp = None if esp is None else esp + size
        if mnemonic == "pop" and "esp" in {_family(instruction, operands[0].reg)
                                           if operands[0].type == x86.X86_OP_REG else None}:
            new_esp = None
    elif mnemonic in ("pushf", "popf", "pushfd", "popfd", "pushal", "pushad", "popal", "popad"):
        size = {"pushf": 2, "popf": 2, "pushfd": 4, "popfd": 4}.get(mnemonic, 32)
        new_esp = None if esp is None else esp + (size if mnemonic.startswith("pop") else -size)
    elif "esp" in written:
        new_esp = None
        if (len(operands) == 2 and operands[0].type == x86.X86_OP_REG
                and _family(instruction, operands[0].reg) == "esp" and operands[0].size == 4):
            source = operands[1]
            if mnemonic in ("add", "sub") and source.type == x86.X86_OP_IMM and esp is not None:
                new_esp = esp + (source.imm if mnemonic == "add" else -source.imm)
            elif (mnemonic == "mov" and source.type == x86.X86_OP_REG
                  and _family(instruction, source.reg) == "ebp"):
                new_esp = ebp
            elif (mnemonic == "lea" and source.type == x86.X86_OP_MEM and source.mem.base
                  and _family(instruction, source.mem.base) == "ebp"
                  and not source.mem.index and ebp is not None):
                new_esp = ebp + source.mem.disp
    if "ebp" in written:
        new_ebp = None
        if (mnemonic == "mov" and len(operands) == 2
                and operands[0].type == x86.X86_OP_REG and operands[1].type == x86.X86_OP_REG
                and _family(instruction, operands[0].reg) == "ebp"
                and _family(instruction, operands[1].reg) == "esp"):
            new_ebp = esp
    return new_esp, new_ebp


_MAX_TABLE_INDEX = 255
_BOUND_RELATIONS = {
    # (taken bound, fallthrough bound) for `cmp r, imm`; "imm" or "imm-1".
    x86.X86_INS_JA: (None, 0), x86.X86_INS_JAE: (None, -1),
    x86.X86_INS_JB: (-1, None), x86.X86_INS_JBE: (0, None),
    x86.X86_INS_JE: (0, None), x86.X86_INS_JNE: (None, 0),
}


def _with_bound(bounds: frozenset, family: str, maximum: int) -> frozenset:
    current = dict(bounds)
    if family in current:
        maximum = min(maximum, current[family])
    current[family] = maximum
    return frozenset(current.items())


def _edge_bounds(instruction, flags, bounds: frozenset) -> tuple[frozenset, frozenset]:
    """Unsigned upper bounds a conditional edge establishes from `cmp r32, imm`."""
    relation = _BOUND_RELATIONS.get(instruction.id)
    if flags is None or relation is None:
        return bounds, bounds
    family, immediate = flags
    result = []
    for adjustment in relation:
        if adjustment is None or immediate + adjustment < 0:
            result.append(bounds)
        else:
            result.append(_with_bound(bounds, family, immediate + adjustment))
    return result[0], result[1]


def _selector_bound(context: _Context, instruction, current: dict) -> int | None:
    """Bound of a byte loaded from a selector map indexed by a bounded register.

    Admits `movzx r32, byte [idx+map]` and `mov r8low, byte [idx+map]` when
    the destination register was bounded by 0xFF (upper bytes zero). The map
    bytes are read from the pinned image and must carry no relocation.
    """
    operands = instruction.operands
    if len(operands) != 2 or operands[0].type != x86.X86_OP_REG or operands[1].type != x86.X86_OP_MEM:
        return None
    source = operands[1]
    memory = source.mem
    if source.size != 1 or memory.segment != x86.X86_REG_INVALID:
        return None
    registers = [register for register in (memory.base, memory.index) if register]
    if len(registers) != 1 or (memory.index and memory.scale != 1):
        return None
    maximum = current.get(_family(instruction, registers[0]))
    if maximum is None or maximum > _MAX_TABLE_INDEX:
        return None
    destination = instruction.reg_name(operands[0].reg)
    if instruction.mnemonic == "movzx" and operands[0].size == 4:
        pass
    elif (instruction.mnemonic == "mov" and destination in ("al", "bl", "cl", "dl")
          and current.get(_family(instruction, operands[0].reg), 0x100) <= 0xFF):
        pass
    else:
        return None
    start = memory.disp & 0xFFFFFFFF
    end = start + maximum + 1
    if any(site in context.image.relocations for site in range(start - 3, end)):
        return None
    try:
        return max(context.image.read(Span(start, end)))
    except ValueError:
        return None


def _value_step(instruction, flags, bounds: frozenset, context: _Context | None = None):
    """Carry the `cmp r32, imm` flag source and per-register unsigned bounds."""
    operands = instruction.operands
    written = _written_families(instruction)
    current = dict(bounds)
    selected = None if context is None else _selector_bound(context, instruction, current)
    copied = None
    if (instruction.mnemonic == "mov" and len(operands) == 2
            and operands[0].type == x86.X86_OP_REG and operands[0].size == 4
            and operands[1].type == x86.X86_OP_REG and operands[1].size == 4):
        copied = current.get(_family(instruction, operands[1].reg))
    zeroed = (instruction.mnemonic == "xor" and len(operands) == 2
              and operands[0].type == x86.X86_OP_REG and operands[1].type == x86.X86_OP_REG
              and operands[0].reg == operands[1].reg and operands[0].size == 4)
    for family in written:
        current.pop(family, None)
    if copied is not None:
        current[_family(instruction, operands[0].reg)] = copied
    if zeroed:
        current[_family(instruction, operands[0].reg)] = 0
    if selected is not None:
        current[_family(instruction, operands[0].reg)] = selected
    if (instruction.mnemonic == "cmp" and len(operands) == 2
            and operands[0].type == x86.X86_OP_REG and operands[0].size == 4
            and operands[1].type == x86.X86_OP_IMM):
        family = _family(instruction, operands[0].reg)
        next_flags = None if family == "esp" else (family, operands[1].imm & 0xFFFFFFFF)
    elif instruction.eflags or written & ({flags[0]} if flags else set()):
        next_flags = None
    else:
        next_flags = flags
    return next_flags, frozenset(current.items())


def _eax_constant(instruction, eax: int | None) -> int | None:
    """The tracked `mov eax, imm32` constant after one instruction."""
    operands = instruction.operands
    if (instruction.mnemonic == "mov" and len(operands) == 2
            and operands[0].type == x86.X86_OP_REG and operands[0].reg == x86.X86_REG_EAX
            and operands[1].type == x86.X86_OP_IMM):
        return operands[1].imm & 0xFFFFFFFF
    return None if "eax" in _written_families(instruction) else eax


def _analyse(context: _Context, entry: int, family: str, depth: int,
             active: frozenset[int]) -> None:
    """Prove `family` preserved on every reachable state of one callee.

    The register status is ORIGINAL (the register holds the caller's value
    and no save is live) or (slot, clobbered) after `push R` stored the
    caller's value at the ESP-relative `slot`.
    """
    if depth > PRESERVATION_DEPTH_LIMIT:
        raise _ContextRefusal(f"callee {entry:#x} exceeds the preservation depth bound")
    if entry in active:
        raise _ContextRefusal(f"callee {entry:#x} is recursive; preservation is not proved")
    active = active | {entry}
    stack = [(entry, 0, None, None, _ORIGINAL, None, frozenset())]
    seen = set()
    returns = 0
    while stack:
        state = stack.pop()
        if state in seen:
            continue
        seen.add(state)
        if len(seen) > PRESERVATION_STATE_LIMIT:
            raise _Refusal(f"callee {entry:#x} exceeds the preservation search bound")
        address, esp, ebp, eax, status, flags, bounds = state
        instruction = context.decode(address)
        following = address + instruction.size
        if (instruction.group(CS_GRP_INT) or instruction.group(CS_GRP_IRET)
                or instruction.mnemonic in _UNSUPPORTED):
            raise _Refusal(f"callee {entry:#x} reaches an unsupported transfer at {address:#x}")
        if any(operand.type == x86.X86_OP_MEM and operand.mem.segment == x86.X86_REG_SS
               for operand in instruction.operands):
            raise _Refusal(f"callee {entry:#x} uses an explicit stack segment at {address:#x}")
        saved = status != _ORIGINAL
        overlap = _frame_store_overlaps(instruction, esp, ebp, status[0]) if saved else None
        if overlap:
            raise _Refusal(f"callee {entry:#x} {overlap} while {family} is saved, at {address:#x}")
        if instruction.group(CS_GRP_RET):
            if saved and status[1]:
                raise _Refusal(f"callee {entry:#x} returns at {address:#x} without restoring {family}")
            returns += 1
            continue
        if instruction.group(CS_GRP_CALL):
            target = _direct_target(instruction)
            next_status = status
            if target is None or instruction.bytes[0] != 0xE8:
                if not saved:
                    raise _Refusal(f"callee {entry:#x} makes an unprovable call at {address:#x}")
                effect = None
                if saved:
                    next_status = (status[0], True)
            else:
                effect = _effect(context, target)
                if not (saved and status[1]):
                    try:
                        _verdict(context, target, family, depth + 1, active)
                    except _Refusal:
                        if not saved:
                            raise
                        # The save is live; a nested clobber only means the
                        # POP must restore the register.
                        next_status = (status[0], True)
            if effect == _PROBE:
                next_esp = None if esp is None or eax is None else esp - eax
            else:
                next_esp = None if esp is None or effect is None else esp + effect
            stack.append((following, next_esp, ebp, None, next_status, None, frozenset()))
            continue
        if instruction.group(CS_GRP_JUMP) or instruction.group(CS_GRP_BRANCH_RELATIVE):
            target = _direct_target(instruction)
            if target is None:
                for target in context.table_targets(instruction, bounds):
                    stack.append((target, esp, ebp, eax, status, None, bounds))
                continue
            taken, fallthrough = _edge_bounds(instruction, flags, bounds)
            stack.append((target, esp, ebp, eax, status, flags, taken))
            if instruction.id != x86.X86_INS_JMP:
                stack.append((following, esp, ebp, eax, status, flags, fallthrough))
            continue
        next_status = status
        operands = instruction.operands
        register_operand = (len(operands) == 1 and operands[0].type == x86.X86_OP_REG
                            and _family(instruction, operands[0].reg) == family
                            and operands[0].size == 4)
        if instruction.mnemonic == "push" and register_operand:
            if not saved and esp is not None:
                next_status = (esp - 4, False)
        elif instruction.mnemonic == "pop" and register_operand:
            if not saved:
                raise _Refusal(f"callee {entry:#x} overwrites {family} at {address:#x}")
            if esp is None:
                raise _Refusal(f"callee {entry:#x} restores {family} at an unknown stack depth")
            next_status = _ORIGINAL if esp == status[0] else (status[0], True)
        elif family in _written_families(instruction):
            if not saved:
                raise _Refusal(f"callee {entry:#x} overwrites {family} at {address:#x}")
            next_status = (status[0], True)
        next_esp, next_ebp = _stack_step(instruction, esp, ebp)
        if next_status != _ORIGINAL and next_esp is not None and next_esp > next_status[0]:
            if next_status[1]:
                raise _Refusal(f"callee {entry:#x} releases its saved {family} slot at {address:#x}")
            next_status = _ORIGINAL  # released while the register still holds the value
        next_flags, next_bounds = _value_step(instruction, flags, bounds, context)
        stack.append((following, next_esp, next_ebp, _eax_constant(instruction, eax), next_status,
                      next_flags, next_bounds))
    if not returns:
        raise _Refusal(f"callee {entry:#x} has no reachable return")


def _standard_stack_write(instruction) -> bool:
    """ESP writes whose effect `_stack_step` derives exactly from the frame."""
    if instruction.mnemonic in ("push", "pop", "leave", "pushf", "popf", "pushfd", "popfd",
                                "pushal", "pushad", "popal", "popad"):
        return not (instruction.mnemonic == "pop" and instruction.operands
                    and instruction.operands[0].type == x86.X86_OP_REG
                    and _family(instruction, instruction.operands[0].reg) == "esp")
    operands = instruction.operands
    if (len(operands) != 2 or operands[0].type != x86.X86_OP_REG
            or _family(instruction, operands[0].reg) != "esp" or operands[0].size != 4):
        return False
    source = operands[1]
    return ((instruction.mnemonic in ("add", "sub") and source.type == x86.X86_OP_IMM)
            or (instruction.mnemonic == "mov" and source.type == x86.X86_OP_REG
                and _family(instruction, source.reg) == "ebp")
            or (instruction.mnemonic == "lea" and source.type == x86.X86_OP_MEM
                and bool(source.mem.base) and _family(instruction, source.mem.base) == "ebp"
                and not source.mem.index))


def _effect(context: _Context, entry: int) -> int | str | None:
    """A nested callee's declared stack effect, `_PROBE`, or None (unknown).

    The declared effect is the one RET immediate shared by every return
    reachable from `entry` (fallthrough, direct jumps and path-bounded jump
    tables; calls fall through). It is unknown when any reachable code is
    unresolved or writes ESP other than by the standard frame forms. The
    nested callee's own internal balance is its encoded contract and is not
    re-proved; the stack probe is the only admitted non-standard helper.
    """
    if entry in context.effects:
        return context.effects[entry]
    if context.is_stack_probe(entry):
        context.effects[entry] = _PROBE
        return _PROBE
    effect = None
    pending = [(entry, None, frozenset())]
    seen = set()
    immediates = set()
    try:
        while pending:
            state = pending.pop()
            if state in seen:
                continue
            seen.add(state)
            if len(seen) > PRESERVATION_STATE_LIMIT:
                raise _Refusal("stack-effect scan exceeds its bound")
            address, flags, bounds = state
            instruction = context.decode(address)
            following = address + instruction.size
            if (instruction.group(CS_GRP_INT) or instruction.group(CS_GRP_IRET)
                    or instruction.mnemonic in _UNSUPPORTED):
                raise _Refusal("unsupported transfer")
            if instruction.group(CS_GRP_RET):
                immediates.add(instruction.operands[0].imm if instruction.operands else 0)
                continue
            if instruction.group(CS_GRP_CALL):
                pending.append((following, None, frozenset()))
                continue
            if instruction.group(CS_GRP_JUMP) or instruction.group(CS_GRP_BRANCH_RELATIVE):
                target = _direct_target(instruction)
                if target is None:
                    pending.extend((item, None, bounds)
                                   for item in context.table_targets(instruction, bounds))
                    continue
                taken, fallthrough = _edge_bounds(instruction, flags, bounds)
                pending.append((target, flags, taken))
                if instruction.id != x86.X86_INS_JMP:
                    pending.append((following, flags, fallthrough))
                continue
            if "esp" in _written_families(instruction) and not _standard_stack_write(instruction):
                raise _Refusal("non-standard stack pointer write")
            pending.append((following, *_value_step(instruction, flags, bounds, context)))
        if len(immediates) == 1:
            effect = immediates.pop()
    except _Refusal:
        effect = None
    context.effects[entry] = effect
    return effect


def _verdict(context: _Context, entry: int, family: str, depth: int,
             active: frozenset[int]) -> None:
    key = (entry, family)
    cached = context.verdicts.get(key)
    if isinstance(cached, str) and cached != "proved":
        raise _Refusal(cached)
    if cached is not None:
        return
    if context.is_stack_probe(entry):
        if family not in _STACK_PROBE_PRESERVES:
            context.verdicts[key] = f"stack probe {entry:#x} overwrites {family}"
            raise _Refusal(context.verdicts[key])
        context.verdicts[key] = "proved"
        return
    taint = context.taint
    try:
        _analyse(context, entry, family, depth, active)
    except _ContextRefusal:
        context.taint += 1
        raise
    except _Refusal as error:
        if context.taint == taint:
            context.verdicts[key] = str(error)
        raise
    context.verdicts[key] = "proved"


def callee_preserves(image: PeImage, entry: int, family: str) -> None:
    """Raise ValueError unless the callee at `entry` provably preserves `family`."""
    if type(image) is not PeImage:
        raise ValueError("callee preservation requires a pinned parsed PE image")
    if family not in _PRESERVABLE:
        raise ValueError(f"callee preservation does not model register {family!r}")
    if type(entry) is not int or not 0 <= entry <= 0xFFFFFFFF:
        raise ValueError("callee entry is outside i386 address space")
    _verdict(_context(image), entry, family, 0, frozenset())
