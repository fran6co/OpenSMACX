"""Strict byte/reference proof for explicitly declared adjacent binary data.

Associated switch tables remain outside function source spans. This adapter
forms a comparison envelope only from those independently recorded spans and
never treats a compiler label or a trailing section extent as their identity.
"""

from __future__ import annotations

import hashlib
import struct
from dataclasses import asdict
from typing import Mapping, Sequence

from capstone import (
    CS_ARCH_X86, CS_GRP_BRANCH_RELATIVE, CS_GRP_CALL, CS_GRP_INT, CS_GRP_IRET, CS_GRP_RET,
    CS_MODE_32, Cs, x86,
)

from .associated_source import AssociatedDataBinding, validate_associated
from .binary import AbsoluteConstant, BoundedTarget, CompilerArtifact, Relocation, RelocationKind, compare_bound_code
from .callee_preservation import callee_preserves
from .components import relative_path
from .linked_fpo import validate_fpo_object
from .model import Origin, Result, Span
from .pe_coff import Coff, PeImage, Symbol


_ENGINE = Cs(CS_ARCH_X86, CS_MODE_32)
_ENGINE.detail = True
_FILL = frozenset((0x90, 0xCC))
_GPR32 = frozenset((x86.X86_REG_EAX, x86.X86_REG_EBX, x86.X86_REG_ECX,
                    x86.X86_REG_EDX, x86.X86_REG_ESI, x86.X86_REG_EDI,
                    x86.X86_REG_EBP, x86.X86_REG_ESP))
_SAFE_X87_MNEMONICS = frozenset(("fild", "fsub", "fsubr", "fld", "fld1", "fmul", "fstp"))
_REGISTER_FAMILIES = {
    "al": "eax", "ah": "eax", "ax": "eax", "eax": "eax",
    "bl": "ebx", "bh": "ebx", "bx": "ebx", "ebx": "ebx",
    "cl": "ecx", "ch": "ecx", "cx": "ecx", "ecx": "ecx",
    "dl": "edx", "dh": "edx", "dx": "edx", "edx": "edx",
    "sil": "esi", "si": "esi", "esi": "esi",
    "dil": "edi", "di": "edi", "edi": "edi",
    "bpl": "ebp", "bp": "ebp", "ebp": "ebp",
    "spl": "esp", "sp": "esp", "esp": "esp",
}


def _decode_exact(image: PeImage, span: Span):
    code = image.read(span)
    instructions = list(_ENGINE.disasm(code, span.start))
    cursor = span.start
    for instruction in instructions:
        if instruction.address != cursor:
            raise ValueError("source body contains undecoded bytes")
        cursor += instruction.size
    if cursor != span.end or not instructions:
        raise ValueError("source body is not completely decoded")
    last = instructions[-1]
    if not (last.mnemonic.startswith("ret") or last.id == x86.X86_INS_JMP):
        raise ValueError("source body must end at a decoded return or unconditional jump")
    return code, instructions


def _is_canonical_lea_identity_padding(code: bytes) -> bool:
    """Recognize only one unprefixed three-byte LEA r32,[r32+disp8(0)]."""
    if type(code) is not bytes or len(code) != 3:
        return False
    instructions = list(_ENGINE.disasm(code, 0))
    if len(instructions) != 1:
        return False
    instruction, = instructions
    if (instruction.address != 0 or instruction.size != 3
            or instruction.id != x86.X86_INS_LEA
            or instruction.addr_size != 4
            or instruction.disp_size != 1
            or instruction.disp != 0
            or instruction.rex != 0
            or any(instruction.prefix)
            or instruction.eflags != 0
            or len(instruction.operands) != 2):
        return False
    destination, address = instruction.operands
    if (destination.type != x86.X86_OP_REG or destination.size != 4
            or destination.reg not in _GPR32
            or address.type != x86.X86_OP_MEM or address.size != 4):
        return False
    memory = address.mem
    return (memory.base == destination.reg
            and memory.base in _GPR32
            and memory.index == 0
            and memory.scale == 1
            and memory.disp == 0
            and memory.segment == 0)


def _is_canonical_hotpatch_padding(code: bytes) -> bool:
    """Recognize only the pinned two-byte MOV EDI,EDI alignment spelling."""
    return type(code) is bytes and code == b"\x8b\xff"


def _is_intergroup_hotpatch_gap(original: bytes, candidate: bytes, left: int, right: int,
                                selector_ends: set[int], table_starts: set[int]) -> bool:
    """Accept the exact hotpatch pair only between an adjacent map and table."""
    return (_is_canonical_hotpatch_padding(original)
            and candidate == original
            and left in selector_ends and right in table_starts
            and left % 4 == 2 and right % 4 == 0)


def _is_intergroup_lea_identity_gap(original: bytes, candidate: bytes, left: int, right: int,
                                    selector_ends: set[int], table_starts: set[int]) -> bool:
    """Accept one exact three-byte LEA identity only between a map and the next table."""
    return (len(original) == 3
            and _is_canonical_lea_identity_padding(original)
            and candidate == original
            and left in selector_ends and right in table_starts
            and right - left == 3 and right % 4 == 0)


def _symbol_target(obj: Coff, section_number: int, section_data: bytes, offset: int):
    """Resolve one physical ABS32 field to a local same-section symbol."""
    rows = [(site, index, kind) for site, index, kind in obj.sections[section_number - 1].relocations
            if site == offset]
    if len(rows) != 1:
        raise ValueError("associated operand/cell requires exactly one COFF relocation")
    _, symbol_index, kind = rows[0]
    if kind != 6 or offset + 4 > len(section_data):
        raise ValueError("associated operand/cell requires an in-bounds ABS32 relocation")
    symbol = obj.symbols[symbol_index]
    if symbol.section != section_number or symbol.storage != 6 or symbol.type != 0:
        raise ValueError("associated operand/cell target is not a local same-section symbol")
    addend = struct.unpack_from("<i", section_data, offset)[0]
    resolved = symbol.value + addend
    if not 0 <= resolved < len(section_data):
        raise ValueError("associated relocation resolves outside its candidate COMDAT")
    return symbol, addend, resolved


def _instruction_at(instructions, address: int):
    return next((instruction for instruction in instructions if instruction.address == address), None)


def _family(instruction, register: int) -> str:
    name = instruction.reg_name(register)
    if name not in _REGISTER_FAMILIES:
        raise ValueError("associated dispatch uses an unsupported register family")
    return _REGISTER_FAMILIES[name]


def _access_families(instruction) -> set[str]:
    try:
        _, writes = instruction.regs_access()
    except Exception as error:
        raise ValueError("cannot establish associated dispatch register writes") from error
    families = set()
    for register in writes:
        name = instruction.reg_name(register)
        if name in ("eflags", "rflags", "eip", "ip"):
            continue
        if (instruction.mnemonic in _SAFE_X87_MNEMONICS
                and (name == "fpsw" or name.startswith("st("))):
            continue
        families.add(_family(instruction, register))
    return families


def _branch_target(instruction) -> int | None:
    if not instruction.group(x86.X86_GRP_JUMP):
        return None
    if len(instruction.operands) == 1 and instruction.operands[0].type == x86.X86_OP_IMM:
        return instruction.operands[0].imm
    return None


def _incoming_direct_target(instruction) -> int | None:
    """An explicit incoming edge can be a branch, LOOP-family edge or CALL.

    CALL remains a fallthrough instruction for jump-only consumers. Here its
    callee entry can bypass another dispatch's guard just as a branch can.
    """
    if not (instruction.group(x86.X86_GRP_JUMP)
            or instruction.group(CS_GRP_BRANCH_RELATIVE)
            or instruction.group(CS_GRP_CALL)):
        return None
    if (instruction.group(CS_GRP_CALL) and len(instruction.operands) == 2
            and all(operand.type == x86.X86_OP_IMM for operand in instruction.operands)):
        raise ValueError("guard proof requires unsupported segment-context for far direct CALL")
    if len(instruction.operands) == 1 and instruction.operands[0].type == x86.X86_OP_IMM:
        return instruction.operands[0].imm
    return None


def _writes_integer_flags(instruction) -> bool:
    # Capstone's x86 `eflags` field is a union with x87 condition/status bits.
    # Only these individually reviewed non-flag-writing x87 operations are
    # admitted in a guard corridor; FCOMI/FUCOMI and unknown x87 forms refuse.
    if instruction.mnemonic in _SAFE_X87_MNEMONICS:
        return False
    return bool(instruction.eflags)


def _corridor_control_transfer(instruction) -> bool:
    """Reject control transfers that can invalidate the guard dataflow."""
    return (instruction.group(x86.X86_GRP_JUMP)
            or any(instruction.group(group)
                   for group in (CS_GRP_BRANCH_RELATIVE, CS_GRP_CALL, CS_GRP_RET, CS_GRP_INT, CS_GRP_IRET)))


def _guard_for_dispatch(instructions, dispatch, span: Span, *,
                        indirect_targets: frozenset[int] | None = None):
    """Prove one straight-line unsigned guard reaches this indexed jump.

    The supported envelope is intentionally narrow: `cmp reg,imm`, a direct
    JA/JAE to the default arm, no flag-writing/control-transfer instruction
    before that branch, and no alternate direct branch/call into either protected corridor.
    With no indirect-edge witness, retain the conservative legacy rule and
    reject every other indirect JMP. The associated-code caller may supply
    the complete table-derived target set only after independently validating
    every cell and proving that every indirect JMP has a declared table.
    """
    # A linear decoded CFG cannot prove an overlapping instruction stream.
    # This includes the guard branch itself and calls into instruction bytes,
    # not merely incoming edges whose target equals a decoded start. External
    # callees lie outside this parent body and retain the existing contract.
    starts = {item.address for item in instructions}
    if any(target is not None and span.start <= target < span.end
           and target not in starts
           for item in instructions
           for target in (_incoming_direct_target(item),)):
        raise ValueError("direct control target is not a decoded parent instruction boundary")
    choices = []
    for branch in instructions:
        if (branch.address >= dispatch.address or branch.id not in
                (x86.X86_INS_JA, x86.X86_INS_JAE)):
            continue
        target = _branch_target(branch)
        if target is None or not span.start <= target < span.end:
            continue
        if branch.address + branch.size <= target <= dispatch.address:
            continue
        branch_index = instructions.index(branch)
        for compare in reversed(instructions[:branch_index]):
            if compare.id != x86.X86_INS_CMP or len(compare.operands) != 2:
                continue
            left, right = compare.operands
            if (left.type != x86.X86_OP_REG or right.type != x86.X86_OP_IMM
                    or left.size != 4 or not 0 <= right.imm <= 0xFFFFFFFF):
                continue
            compare_index = instructions.index(compare)
            if compare.address >= branch.address:
                continue
            corridor = instructions[compare_index + 1:branch_index]
            if any(_writes_integer_flags(item) or _corridor_control_transfer(item)
                   for item in corridor):
                continue
            guarded = _family(compare, left.reg)
            if any(guarded in _access_families(item) for item in corridor):
                continue
            guard_corridor_start = compare.address + compare.size
            if any((other is not branch and _incoming_direct_target(other) is not None
                    and guard_corridor_start <= _incoming_direct_target(other) <= branch.address)
                   for other in instructions):
                continue
            if (indirect_targets is not None
                    and any(guard_corridor_start <= target <= branch.address
                            for target in indirect_targets)):
                continue
            path = instructions[branch_index + 1:instructions.index(dispatch)]
            if any(_corridor_control_transfer(item) for item in path):
                continue
            # Any other direct edge into the fallthrough corridor would bypass
            # the dominating range check. The accepted branch itself exits it.
            corridor_start = branch.address + branch.size
            if any((other is not branch and _incoming_direct_target(other) is not None
                    and corridor_start <= _incoming_direct_target(other) <= dispatch.address)
                   for other in instructions):
                continue
            if indirect_targets is None:
                # Preserve the original helper contract for callers that do
                # not supply a complete, independently proven edge set.
                if any(other is not dispatch and other.group(x86.X86_GRP_JUMP)
                       and _branch_target(other) is None for other in instructions):
                    continue
            elif any(corridor_start <= target <= dispatch.address
                     for target in indirect_targets):
                continue
            max_value = right.imm - (1 if branch.id == x86.X86_INS_JAE else 0)
            count = max_value + 1
            if count <= 0 or count > 256:
                continue
            choices.append((compare, branch, guarded, count, target))
            break
    if len(choices) != 1:
        raise ValueError("dispatch extent lacks one unambiguous unsigned dominating guard")
    choice = choices[0]
    if choice[4] not in {item.address for item in instructions}:
        raise ValueError("unsigned guard default target is not an instruction boundary")
    return choice


# Instructions whose only memory effect is an explicit operand. Every other
# instruction (PUSH, CALL, string forms, ...) is assumed to write memory.
_EXPLICIT_MEMORY_IDS = frozenset((
    x86.X86_INS_MOV, x86.X86_INS_MOVZX, x86.X86_INS_MOVSX, x86.X86_INS_LEA,
    x86.X86_INS_CMP, x86.X86_INS_TEST, x86.X86_INS_ADD, x86.X86_INS_SUB,
    x86.X86_INS_AND, x86.X86_INS_OR, x86.X86_INS_XOR, x86.X86_INS_SHL,
    x86.X86_INS_SHR, x86.X86_INS_SAR, x86.X86_INS_IMUL, x86.X86_INS_INC,
    x86.X86_INS_DEC, x86.X86_INS_NEG, x86.X86_INS_NOT, x86.X86_INS_CDQ,
    x86.X86_INS_NOP,
))
# Unsigned/equality conditions: (taken relation, fallthrough relation) of
# `cmp left, right`. Signed conditions give no unsigned bound and are ignored.
_UNSIGNED_EDGE_RELATIONS = {
    x86.X86_INS_JA: (">", "<="), x86.X86_INS_JAE: (">=", "<"),
    x86.X86_INS_JB: ("<", ">="), x86.X86_INS_JBE: ("<=", ">"),
    x86.X86_INS_JE: ("==", "!="), x86.X86_INS_JNE: ("!=", "=="),
}
_PATH_PROOF_STATE_LIMIT = 50000


def _value_location(instruction, operand):
    """A tracked 32-bit value location: a GPR family or an EBP-relative slot."""
    if operand.type == x86.X86_OP_REG and operand.size == 4:
        try:
            return ("reg", _family(instruction, operand.reg))
        except ValueError:
            return None
    if operand.type == x86.X86_OP_MEM and operand.size == 4:
        memory = operand.mem
        if (memory.base == x86.X86_REG_EBP and not memory.index
                and memory.segment == x86.X86_REG_INVALID):
            return ("slot", memory.disp)
    return None


def _may_write_memory(instruction) -> bool:
    if instruction.id in (x86.X86_INS_CMP, x86.X86_INS_TEST):
        return False
    if instruction.group(x86.X86_GRP_JUMP) and not instruction.group(CS_GRP_CALL):
        return False
    if instruction.id not in _EXPLICIT_MEMORY_IDS:
        return True
    return bool(instruction.operands) and instruction.operands[0].type == x86.X86_OP_MEM


def _kills(instruction, location) -> bool:
    """Whether executing `instruction` can change the value held at `location`."""
    if any(instruction.group(group) for group in (CS_GRP_CALL, CS_GRP_INT, CS_GRP_IRET, CS_GRP_RET)):
        return True
    try:
        written = _access_families(instruction)
    except ValueError:
        return True
    if location[0] == "reg":
        return location[1] in written
    return _may_write_memory(instruction) or "ebp" in written


def _backward_step(instruction, locations: frozenset):
    """Locations holding a value before `instruction`, given those after it.

    Returns ``("constant", value)`` when the instruction itself defines one
    of the locations with a 32-bit constant; otherwise the predecessor set.
    """
    operands = instruction.operands
    destination = (_value_location(instruction, operands[0])
                   if len(operands) == 2 and operands[0].type == x86.X86_OP_REG else None)
    if destination is not None and destination in locations:
        if instruction.id == x86.X86_INS_MOV and operands[1].type == x86.X86_OP_IMM:
            return ("constant", operands[1].imm & 0xFFFFFFFF)
        if (instruction.id == x86.X86_INS_XOR and operands[1].type == x86.X86_OP_REG
                and operands[1].reg == operands[0].reg):
            return ("constant", 0)
    before = frozenset(location for location in locations if not _kills(instruction, location))
    if (destination is not None and destination in locations
            and instruction.id == x86.X86_INS_MOV):
        source = _value_location(instruction, operands[1])
        if source is not None:
            before |= {source}
    return before


def _incoming_edges(instructions, span: Span, indirect_targets: frozenset[int]):
    """Every modeled predecessor edge, by instruction position."""
    positions = {item.address: position for position, item in enumerate(instructions)}
    edges = {position: [] for position in range(len(instructions))}
    for position, item in enumerate(instructions):
        following = position + 1 if position + 1 < len(instructions) else None
        target = _incoming_direct_target(item)
        if target is not None and span.start <= target < span.end and target not in positions:
            raise ValueError("direct control target is not a decoded parent instruction boundary")
        if item.group(CS_GRP_CALL):
            if target is not None and span.start <= target < span.end:
                edges[positions[target]].append((position, "call"))
            if following is not None:
                edges[following].append((position, "fallthrough"))
        elif item.group(CS_GRP_RET) or item.group(CS_GRP_IRET):
            continue
        elif item.id == x86.X86_INS_JMP:
            if target is None:
                if any(address not in positions for address in indirect_targets):
                    raise ValueError("switch target is not a decoded parent instruction boundary")
                for address in indirect_targets:
                    edges[positions[address]].append((position, "indirect"))
            elif span.start <= target < span.end:
                edges[positions[target]].append((position, "taken"))
        elif item.group(x86.X86_GRP_JUMP) or item.group(CS_GRP_BRANCH_RELATIVE):
            if target is None:
                raise ValueError("path-bounded index proof refuses an indirect conditional transfer")
            if span.start <= target < span.end:
                edges[positions[target]].append((position, "taken"))
            if following is not None:
                edges[following].append((position, "fallthrough"))
        elif following is not None:
            edges[following].append((position, "fallthrough"))
    return edges


def _edge_bounds(instructions, edges, position: int, kind: str):
    """Unsigned facts a conditional edge establishes about `cmp` locations.

    Returns (location, bound) pairs meaning location <= bound after the
    branch, where bound is ("imm", value) or ("location", location).
    """
    branch = instructions[position]
    relations = _UNSIGNED_EDGE_RELATIONS.get(branch.id)
    if relations is None or kind not in ("taken", "fallthrough"):
        return []
    if _branch_target(branch) == branch.address + branch.size:
        return []
    relation = relations[0 if kind == "taken" else 1]
    cursor = position - 1
    while cursor >= 0 and not _writes_integer_flags(instructions[cursor]):
        if _corridor_control_transfer(instructions[cursor]):
            return []
        cursor -= 1
    if cursor < 0:
        return []
    compare = instructions[cursor]
    # The flags reaching the branch must come from this compare on every path.
    if any(edges[inner] != [(inner - 1, "fallthrough")] for inner in range(cursor + 1, position + 1)):
        return []
    if compare.id != x86.X86_INS_CMP or len(compare.operands) != 2:
        return []
    left = _value_location(compare, compare.operands[0])
    if left is None:
        return []
    if compare.operands[1].type == x86.X86_OP_IMM:
        right = ("imm", compare.operands[1].imm & 0xFFFFFFFF)
    else:
        location = _value_location(compare, compare.operands[1])
        if location is None:
            return []
        right = ("location", location)
    corridor = instructions[cursor + 1:position]
    for location in (left, right[1] if right[0] == "location" else None):
        if location is not None and any(_kills(item, location) for item in corridor):
            return []
    if relation == "==" and right[0] == "location":
        # Equal values bound each other in both directions.
        return [(left, right), (right[1], ("location", left))]
    if relation in ("<=", "=="):
        return [(left, right)]
    if relation == "<":
        if right[0] == "imm":
            return [(left, ("imm", right[1] - 1))] if right[1] else []
        return [(left, right)]
    if relation in (">=", ">") and right[0] == "location":
        return [(right[1], ("location", left))]
    return []


def _is_eh_registration_access(instruction) -> bool:
    """`mov r32, fs:[0]` or `mov fs:[0], r32`: the VC6 EH list head only."""
    if instruction.id != x86.X86_INS_MOV or len(instruction.operands) != 2:
        return False
    memory = [operand for operand in instruction.operands if operand.type == x86.X86_OP_MEM]
    registers = [operand for operand in instruction.operands if operand.type == x86.X86_OP_REG]
    if len(memory) != 1 or len(registers) != 1 or registers[0].size != 4:
        return False
    address = memory[0].mem
    return (memory[0].size == 4 and address.segment == x86.X86_REG_FS
            and not address.base and not address.index and address.disp == 0)


def _call_step(instruction, locations: frozenset, image: PeImage | None, preserved: dict,
               resolved: dict | None = None):
    """Locations still holding their value after a direct CALL.

    Only registers that `callee_preserves` proves the callee hands back
    survive; EBP slots and every register of an unprovable call are killed.
    """
    target = _incoming_direct_target(instruction)
    if image is None or target is None or instruction.bytes[0] != 0xE8:
        return frozenset()
    kept = set()
    for location in locations:
        if location[0] != "reg" or location[1] == "esp":
            continue
        key = (target, location[1])
        if key not in preserved:
            try:
                virtual_calls = callee_preserves(image, target, location[1])
                preserved[key] = None
                if virtual_calls and resolved is not None:
                    resolved[key] = virtual_calls
            except ValueError as error:
                preserved[key] = str(error)
        if preserved[key] is None:
            kept.add(location)
    return frozenset(kept)


def _path_bounded_dispatch(instructions, dispatch, index_family: str, span: Span,
                           indirect_targets: frozenset[int], *,
                           image: PeImage | None = None, eh_frame: bool = False) -> dict:
    """Bound a dispatch index on every modeled backward path.

    Each path from the indexed JMP back towards the entry tracks the set of
    registers/EBP slots that hold the index (or, after a register-register
    unsigned compare, an upper bound of it; an equality edge `cmp a,b` keeps
    both compared locations, since each then bounds the other). A path is
    proved by a 32-bit constant definition of that set or by an
    unsigned/equality branch edge whose `cmp` constrains a location equal
    to it. A path that returns to a state already being searched (a case
    arm that re-enters the dispatch) adds no new execution: every real
    path is finite and starts at the entry. Reaching the function
    entry, a call edge into the body, or losing every tracked location
    refuses. Non-explicit memory writes kill every location they can affect.
    Calls kill every EBP slot; with the pinned `image`, a direct `call rel32`
    keeps exactly the tracked registers that `callee_preserves` proves the
    callee hands back unchanged (no callee-saved convention is assumed).

    Segment-relative operands refuse, except that with `eh_frame` (the caller
    proved a cleanup-only VC6 C++ EH frame: no try blocks, hence no catch
    continuation into the body) the `fs:[0]` registration moves are admitted.
    """
    if any(operand.type == x86.X86_OP_MEM and operand.mem.segment != x86.X86_REG_INVALID
           and not (eh_frame and _is_eh_registration_access(item))
           for item in instructions for operand in item.operands):
        raise ValueError("path-bounded index proof refuses segment-relative (EH/SEH) frames")
    preserved: dict = {}
    resolved: dict = {}

    def step(item, locations):
        if item.group(CS_GRP_CALL):
            return _call_step(item, locations, image, preserved, resolved)
        return _backward_step(item, locations)

    edges = _incoming_edges(instructions, span, indirect_targets)
    start = instructions.index(dispatch)
    stack = [(start, frozenset({("reg", index_family)}), frozenset())]
    seen = set()
    bounds = set()

    def proved(tracked, facts):
        matches = [bound for locations, bound in facts if locations & tracked]
        return min(matches) if matches else None

    while stack:
        state = stack.pop()
        if state in seen:
            continue
        seen.add(state)
        if len(seen) > _PATH_PROOF_STATE_LIMIT:
            raise ValueError("path-bounded index proof exceeds its search bound")
        position, tracked, facts = state
        if instructions[position].address == span.start:
            raise ValueError("dispatch index reaches the function entry without a bound")
        for predecessor, kind in edges[position]:
            if kind == "call":
                raise ValueError("direct call enters a dispatch index path")
            item = instructions[predecessor]
            edge_tracked, edge_facts = tracked, set(facts)
            edge_pairs = _edge_bounds(instructions, edges, predecessor, kind)
            equal = {(location, bound[1]) for location, bound in edge_pairs
                     if bound[0] == "location"
                     and (bound[1], ("location", location)) in edge_pairs}
            for location, bound in edge_pairs:
                if bound[0] == "imm":
                    edge_facts.add((frozenset({location}), bound[1]))
                elif (location, bound[1]) in equal:
                    # index <= location == other: both hold an upper bound
                    # of the index, so both stay tracked.
                    if location in tracked:
                        edge_tracked = edge_tracked | {bound[1]}
                elif location in edge_tracked:
                    # index <= location <= bound: bound the compared value instead.
                    edge_tracked = frozenset({bound[1]})
            value = proved(edge_tracked, edge_facts)
            if value is not None:
                bounds.add((item.address, kind, value))
                continue
            tracked_step = step(item, edge_tracked)
            if type(tracked_step) is tuple:
                bounds.add((item.address, "constant", tracked_step[1]))
                continue
            if not tracked_step:
                target = _incoming_direct_target(item) if item.group(CS_GRP_CALL) else None
                unproved = sorted({reason for (callee, _), reason in preserved.items()
                                   if callee == target and reason})
                detail = f" ({'; '.join(unproved)})" if unproved else ""
                raise ValueError(f"dispatch index is not bounded on the path through {item.address:#x}{detail}")
            next_facts = set()
            for locations, bound in edge_facts:
                carried = step(item, locations)
                if type(carried) is not tuple and carried:
                    next_facts.add((carried, bound))
            value = proved(tracked_step, next_facts)
            if value is not None:
                bounds.add((item.address, "fact", value))
                continue
            stack.append((predecessor, tracked_step, frozenset(next_facts)))
    if not bounds:
        raise ValueError("dispatch index has no modeled incoming path")
    witness = {"kind": "path-bounded", "maximum_index": max(value for _, _, value in bounds),
               "bounds": [{"address": address, "edge": kind, "maximum": value}
                          for address, kind, value in sorted(bounds)]}
    carried = sorted(key for key, reason in preserved.items() if reason is None)
    if carried:
        witness["preserving_callees"] = [
            {"callee": callee, "register": register,
             **({"virtual_calls": resolved[callee, register]} if (callee, register) in resolved else {})}
            for callee, register in carried]
    return witness


def _prove_dispatch_extent(instructions, body: Span, image: PeImage,
                           table_binding: AssociatedDataBinding,
                           table_instruction, table_operand,
                           selector_binding: AssociatedDataBinding | None,
                           selector_instruction, selector_operand, *,
                           indirect_targets: frozenset[int] | None = None,
                           path_proof: bool = False, eh_frame: bool = False) -> dict:
    """Derive one table/map extent from its guard and preserved index flow.

    With `path_proof`, a direct-index table without one dominating guard may
    instead be bounded on every incoming path (see `_path_bounded_dispatch`).
    """
    index_register = table_operand.mem.index
    if not index_register:
        raise ValueError("switch target dispatch lacks an index register")
    index_family = _family(table_instruction, index_register)
    table_count = (table_binding.span.end - table_binding.span.start) // 4
    try:
        compare, branch, guard_family, count, default_target = _guard_for_dispatch(
            instructions, table_instruction, body,
            indirect_targets=indirect_targets)
    except ValueError as error:
        if not path_proof or selector_binding is not None or indirect_targets is None:
            raise
        try:
            guard = _path_bounded_dispatch(instructions, table_instruction, index_family,
                                           body, indirect_targets, image=image,
                                           eh_frame=eh_frame)
        except ValueError as path_error:
            raise ValueError(f"{error}; {path_error}") from path_error
        if guard["maximum_index"] + 1 != table_count:
            raise ValueError("direct switch table disagrees with its path-bounded index domain")
        return {"guard": guard, "target_count": table_count, "selector_count": 0,
                "selector_offset": None, "index_register": index_family,
                "mode": "direct-index"}
    branch_pos = instructions.index(branch)
    dispatch_pos = instructions.index(table_instruction)
    if selector_binding is None:
        if table_count != count or index_family != guard_family:
            raise ValueError("direct switch table disagrees with its guard-derived index domain")
        path = instructions[branch_pos + 1:dispatch_pos]
        if any(index_family in _access_families(item) for item in path):
            raise ValueError("direct switch index changes between guard and dispatch")
        return {"guard": {"compare": compare.address, "branch": branch.address,
                          "maximum_index": count - 1, "default": default_target,
                          "condition": branch.mnemonic},
                "target_count": table_count, "selector_count": 0,
                "selector_offset": None, "index_register": index_family,
                "mode": "direct-index"}
    if selector_instruction is None or selector_operand is None:
        raise ValueError("selector interpretation lacks its original load instruction")
    selector_length = selector_binding.span.end - selector_binding.span.start
    if selector_length != count:
        raise ValueError("declared selector map length disagrees with unsigned guard extent")
    selector_index = selector_operand.mem.index or selector_operand.mem.base
    if not selector_index or _family(selector_instruction, selector_index) != guard_family:
        raise ValueError("selector map is not indexed by the guarded range register")
    if bool(selector_operand.mem.base) == bool(selector_operand.mem.index):
        raise ValueError("selector map address must use exactly the guarded index register")

    selector_index_instructions = [item for item in instructions
                                    if item.address == selector_instruction.address]
    if len(selector_index_instructions) != 1:
        raise ValueError("selector instruction is not unique in decoded body")
    if selector_instruction.id == x86.X86_INS_MOVZX:
        if (len(selector_instruction.operands) != 2
                or selector_instruction.operands[0].type != x86.X86_OP_REG
                or selector_instruction.operands[0].size != 4
                or _family(selector_instruction, selector_instruction.operands[0].reg) != index_family):
            raise ValueError("MOVZX selector does not produce the jump-table index register")
        index_start = _family(selector_instruction, selector_instruction.operands[0].reg)
        selector_pos = instructions.index(selector_instruction)
        if not branch_pos < selector_pos < dispatch_pos:
            raise ValueError("selector load is outside the guarded dispatch path")
        for item in instructions[branch_pos + 1:selector_pos]:
            if guard_family in _access_families(item):
                raise ValueError("guarded selector register changes before map lookup")
        for item in instructions[selector_pos + 1:dispatch_pos]:
            if index_start in _access_families(item):
                raise ValueError("switch index changes between selector load and dispatch")
    elif selector_instruction.id == x86.X86_INS_MOV:
        if (len(selector_instruction.operands) != 2
                or selector_instruction.operands[0].type != x86.X86_OP_REG
                or selector_instruction.operands[0].size != 1
                or selector_instruction.reg_name(selector_instruction.operands[0].reg)
                not in ("al", "bl", "cl", "dl")
                or _family(selector_instruction, selector_instruction.operands[0].reg) != index_family):
            raise ValueError("byte selector load does not write the jump-table index low byte")
        selector_pos = instructions.index(selector_instruction)
        if not branch_pos < selector_pos < dispatch_pos:
            raise ValueError("byte selector load is outside the guarded dispatch path")
        if any(guard_family in _access_families(item)
               for item in instructions[branch_pos + 1:selector_pos]):
            raise ValueError("guarded range register changes before selector lookup")
        zeroers = []
        for position in range(branch_pos + 1, selector_pos):
            item = instructions[position]
            if (item.id == x86.X86_INS_XOR and len(item.operands) == 2
                    and all(op.type == x86.X86_OP_REG for op in item.operands)
                    and _family(item, item.operands[0].reg) == index_family
                    and _family(item, item.operands[1].reg) == index_family
                    and item.operands[0].size == 4 and item.operands[1].size == 4):
                zeroers.append(position)
        if len(zeroers) != 1:
            raise ValueError("low-byte selector load lacks one dominating full-register zero")
        zero_pos = zeroers[0]
        if any(index_family in _access_families(item)
               for item in instructions[zero_pos + 1:selector_pos]):
            raise ValueError("selector index is modified after its zeroing instruction")
        if any(index_family in _access_families(item)
               for item in instructions[selector_pos + 1:dispatch_pos]):
            raise ValueError("switch index changes between low-byte load and dispatch")
    else:
        raise ValueError("unsupported selector-map load form")

    selector_bytes = image.read(selector_binding.span)
    if not selector_bytes or max(selector_bytes) + 1 != table_count:
        raise ValueError("declared target table length disagrees with selector-map domain")
    if any(value >= table_count for value in selector_bytes):
        raise ValueError("selector map contains an index outside the guarded table domain")
    selector_start = selector_binding.span.start - body.start
    return {"guard": {"compare": compare.address, "branch": branch.address,
                      "maximum_index": count - 1, "default": default_target,
                      "condition": branch.mnemonic},
            "target_count": table_count, "selector_count": selector_length,
            "selector_offset": selector_start, "index_register": index_family,
            "mode": "mapped-index"}


def _memory_operand(instruction, kind: str):
    if kind == "switch_targets":
        if instruction.id != x86.X86_INS_JMP or not instruction.operands:
            raise ValueError("switch_targets evidence must anchor an indexed indirect jmp")
        operand = instruction.operands[0]
    else:
        if instruction.id not in (x86.X86_INS_MOV, x86.X86_INS_MOVZX) or len(instruction.operands) < 2:
            raise ValueError("switch_selector evidence must anchor a byte load")
        operand = instruction.operands[1]
    if operand.type != x86.X86_OP_MEM or operand.size != (4 if kind == "switch_targets" else 1):
        raise ValueError("associated-data anchor has an unsupported memory operand")
    memory = operand.mem
    if kind == "switch_targets":
        if (not memory.index or memory.base or memory.scale != 4):
            raise ValueError("switch_targets requires an absolute indexed dword-jump operand")
    elif (not (memory.base or memory.index) or memory.scale != 1):
        raise ValueError("switch_selector requires a register-indexed byte operand")
    return operand


def _indexed_jump_tables(instructions):
    """Return every original absolute indexed-dword JMP that needs evidence.

    This deliberately recognizes only the same narrow operand shape accepted
    by ``_memory_operand``. It prevents a caller from omitting a whole group
    while still proving the groups that remain in the annotation population.
    """
    found = set()
    for instruction in instructions:
        if instruction.id != x86.X86_INS_JMP or len(instruction.operands) != 1:
            continue
        operand = instruction.operands[0]
        if (operand.type == x86.X86_OP_MEM and operand.size == 4
                and not operand.mem.base and operand.mem.index
                and operand.mem.scale == 4 and instruction.disp_size == 4):
            found.add(instruction.address)
    return found


def _selector_dispatch(instructions, selector):
    """Return the indexed JMP that one original byte selector load feeds.

    VC6 emits a sparse switch as ``movzx r32, byte [map+guarded]`` (or a
    zeroed ``mov r8, byte [map+guarded]``) followed in straight line by
    ``jmp [table+4*r32]``. The load feeds a dispatch only when its absolute
    single-register byte operand writes the jump's index register, the first
    later control transfer is that indexed JMP, and nothing in between touches
    the index register. Anything else returns None; pairing never guesses.
    """
    if (selector.id not in (x86.X86_INS_MOV, x86.X86_INS_MOVZX)
            or len(selector.operands) != 2 or selector.disp_size != 4):
        return None
    destination, source = selector.operands
    if (destination.type != x86.X86_OP_REG or source.type != x86.X86_OP_MEM
            or source.size != 1 or source.mem.scale != 1 or source.mem.segment
            or bool(source.mem.base) == bool(source.mem.index)):
        return None
    if selector.id == x86.X86_INS_MOVZX and destination.size != 4:
        return None
    if (selector.id == x86.X86_INS_MOV
            and selector.reg_name(destination.reg) not in ("al", "bl", "cl", "dl")):
        return None
    family = _family(selector, destination.reg)
    try:
        position = instructions.index(selector)
    except ValueError:
        return None
    for item in instructions[position + 1:]:
        if _corridor_control_transfer(item):
            if item.address not in _indexed_jump_tables((item,)):
                return None
            index = item.operands[0].mem.index
            return item if _family(item, index) == family else None
        if family in _access_families(item):
            return None
    return None


def _selector_loads(instructions):
    """Return every original byte load that feeds an indexed dispatch JMP."""
    return {instruction.address for instruction in instructions
            if _selector_dispatch(instructions, instruction) is not None}


def _anchor_field(image: PeImage, instruction, operand, data_start: int) -> int:
    if operand.mem.disp != data_start or instruction.disp_size != 4:
        raise ValueError("original evidence operand does not point to its declared data start")
    field = instruction.address + instruction.disp_offset
    sites = image.absolute_sites(Span(field, field + 4))
    if sites != frozenset({0}):
        raise ValueError("original associated-data anchor lacks exactly one PE HIGHLOW relocation")
    return field


def _candidate_anchor(obj: Coff, section_number: int, section_data: bytes,
                      candidate_instruction) -> int:
    if candidate_instruction.disp_size != 4:
        raise ValueError("candidate associated-data operand has no four-byte displacement")
    field = candidate_instruction.address + candidate_instruction.disp_offset
    # Resolve the physical operand now, but leave equality to the ordinary
    # relocation-aware comparator below. That yields a measured FAIL (rather
    # than unsupported UNKNOWN) when a candidate points at the wrong byte.
    _symbol_target(obj, section_number, section_data, field)
    return field


def _binding_witness(binding: AssociatedDataBinding) -> dict:
    return {
        "kind": binding.kind,
        "span": {"start": binding.span.start, "end": binding.span.end},
        "evidence": {"image_sha256": binding.evidence.image_sha256,
                     "address_space": binding.evidence.address_space,
                     "address": binding.evidence.address},
        "unit": binding.unit,
        "line": binding.line,
    }


def compare_associated_code(obj: Coff, subject: Symbol, span: Span, image: PeImage,
                            bindings: Sequence[AssociatedDataBinding],
                            targets: Mapping[str, int | AbsoluteConstant | BoundedTarget | CompilerArtifact],
                            *, eh_tail: int | None = None, eh_cold: Span | None = None) -> dict:
    """Compare one root code COMDAT and explicitly declared adjacent data.

    `span` remains the independently mapped function body. The comparison
    envelope ends at the last declared data span; it does not revise source
    function identity or establish final-link selection/runtime behavior.
    Unsupported evidence raises ValueError so callers preserve UNKNOWN.

    `eh_tail` names the caller's separately mapped `.text$x` EH region of
    this same function. It admits exactly that one associative code child and
    exactly one `.xdata$x` FuncInfo child; the caller compares the tail
    itself. Switch data still belongs only to the primary root body.
    `eh_cold` is that tail's original span; when the pinned image proves it a
    cleanup-only VC6 FuncInfo frame (no try blocks), the path-bounded index
    proof may run over the EH-registering body.
    """
    if type(obj) is not Coff or type(subject) is not Symbol or type(image) is not PeImage:
        raise ValueError("associated proof requires native Coff, Symbol, and PeImage values")
    if type(span) is not Span:
        raise ValueError("associated proof requires one exact source body span")
    Span.__post_init__(span)
    if type(bindings) not in (tuple, list) or not bindings:
        raise ValueError("associated proof requires a nonempty binding sequence")
    if not isinstance(targets, Mapping):
        raise ValueError("associated proof requires the ordinary target mapping")
    pin = hashlib.sha256(image.data).hexdigest()
    if not (0 <= span.start < span.end <= 0xFFFFFFFF):
        raise ValueError("source body span is outside i386 address space")
    if (subject.section <= 0 or subject.section > len(obj.sections) or subject.value != 0
            or not subject.type & 0x20 or subject.storage not in (2, 3)):
        raise ValueError("subject is not a unique function entry at a root COMDAT offset zero")
    section_number = subject.section
    section = obj.sections[section_number - 1]
    if not (section.flags & 0x20 and section.flags & 0x1000):
        raise ValueError("associated proof requires a root code COMDAT")
    if section_number in obj.associations:
        raise ValueError("associated proof refuses an associative root COMDAT")
    associated_children = [child for child, parent in obj.associations.items()
                           if parent == section_number]
    if eh_tail is not None:
        tails = [child for child in associated_children if obj.sections[child - 1].name == ".text$x"]
        funcinfo = [child for child in associated_children if obj.sections[child - 1].name == ".xdata$x"]
        if (type(eh_tail) is not int or tails != [eh_tail] or len(funcinfo) != 1
                or obj.code_sections(subject) != (section_number, eh_tail)
                or obj.sections[eh_tail - 1].flags != 0x60501020
                or obj.sections[funcinfo[0] - 1].flags != 0x40401040
                or any(child in obj.associations.values() for child in (eh_tail, funcinfo[0]))):
            raise ValueError("associated proof requires one exact .text$x EH tail and .xdata$x FuncInfo child")
        eh_children = {eh_tail, funcinfo[0]}
    else:
        eh_children = set()
    debug_witnesses = []
    fpo_children = []
    proved_children = set(eh_children)
    for child in associated_children:
        name = obj.sections[child - 1].name
        if child in eh_children:
            continue
        if name == ".debug$F":
            fpo_children.append(child)
        elif name == ".debug$S":
            from .linked_codeview import source_procedure
            procedure = source_procedure(obj, child, subject, local_addresses=True, eh_tail=eh_tail)
            debug_procedure = asdict(procedure)
            for record in debug_procedure["records"]:
                record["padding"] = record["padding"].hex()
            debug_witnesses.append({"section": child, "association_parent": section_number,
                                    "format": "codeview5_procedure", "procedure": debug_procedure})
            proved_children.add(child)
        else:
            raise ValueError("associated proof refuses an unclassified child section")
    fpo_named_sections = {
        number for number, child in enumerate(obj.sections, 1)
        if number != section_number and (
            child.name == ".debug$F"
            or any(symbol.section == number and symbol.name == ".debug$F"
                   for symbol in obj.symbols.values()))
    }
    fpo_references = set()
    for number in fpo_named_sections:
        child = obj.sections[number - 1]
        if any(target in obj.symbols and
               (target == subject.index or obj.symbols[target].name == subject.name)
               for _, target, kind in child.relocations if kind != 0):
            fpo_references.add(number)
    if fpo_references != set(fpo_children):
        raise ValueError("root-referencing FPO metadata lacks one exact associative child")
    fpo_witness = None
    if fpo_children:
        if len(fpo_children) != 1:
            raise ValueError("associated proof supports only one exact FPO metadata child")
        fpo_witness = validate_fpo_object(obj, fpo_children[0], subject, source_codeview=True,
                                          proved_siblings=frozenset(proved_children))
    entries = [symbol for symbol in obj.symbols.values()
               if symbol.section == section_number and symbol.type & 0x20 and symbol.storage in (2, 3)]
    if len(entries) != 1 or entries[0] != subject:
        raise ValueError("associated proof requires exactly one function entry in the root COMDAT")
    if obj.function(subject.name) != subject:
        raise ValueError("associated proof subject is not the unique COFF function")

    body_code, body_instructions = _decode_exact(image, span)
    owner = None
    unit = None
    normalized = []
    evidence_addresses = set()
    for binding in bindings:
        if type(binding) is not AssociatedDataBinding:
            raise ValueError("associated records must use the exact native binding type")
        AssociatedDataBinding.__post_init__(binding)
        if type(binding.owner) is not Origin or type(binding.evidence) is not Origin or type(binding.span) is not Span:
            raise ValueError("associated binding contains an unsupported nested value")
        Origin.__post_init__(binding.owner)
        Origin.__post_init__(binding.evidence)
        Span.__post_init__(binding.span)
        if (binding.owner.image_sha256 != pin or binding.owner.address_space != "pe:va"
                or binding.owner.address != span.start):
            raise ValueError("associated data owner does not identify the mapped function entry")
        if (binding.evidence.image_sha256 != pin or binding.evidence.address_space != "pe:va"
                or not span.start <= binding.evidence.address < span.end):
            raise ValueError("associated evidence is outside the pinned parent body")
        if binding.evidence.address in evidence_addresses:
            raise ValueError("associated dispatch evidence is ambiguous or duplicated")
        evidence_addresses.add(binding.evidence.address)
        if binding.kind not in ("switch_targets", "switch_selector"):
            raise ValueError("unsupported associated-data kind")
        if binding.kind == "switch_targets" and (binding.span.end - binding.span.start) % 4:
            raise ValueError("switch target table extent must be a nonempty dword sequence")
        if not span.end <= binding.span.start < binding.span.end <= 0xFFFFFFFF:
            raise ValueError("associated data must follow the source body within i386 address space")
        relative_path(binding.unit)
        owner = binding.owner if owner is None else owner
        unit = binding.unit if unit is None else unit
        if binding.owner != owner or binding.unit != unit:
            raise ValueError("associated records disagree on their parent owner/source unit")
        instruction = _instruction_at(body_instructions, binding.evidence.address)
        if instruction is None:
            raise ValueError("associated evidence does not identify a decoded instruction start")
        operand = _memory_operand(instruction, binding.kind)
        _anchor_field(image, instruction, operand, binding.span.start)
        normalized.append((binding, instruction, operand))
    validate_associated(tuple(binding for binding, _, _ in normalized))

    ordered = sorted((binding.span for binding, _, _ in normalized))
    if any(left.end > right.start for left, right in zip(ordered, ordered[1:])):
        raise ValueError("associated data spans overlap")
    selector_rows = [row for row in normalized if row[0].kind == "switch_selector"]
    table_rows = [row for row in normalized if row[0].kind == "switch_targets"]
    if not table_rows:
        raise ValueError("associated dispatch requires at least one target table")
    if len({row[0].span for row in normalized}) != len(normalized):
        raise ValueError("associated dispatch data span is ambiguous or duplicated")

    # Existing v1 records carry no group identifier. Each declared selector
    # is paired with the target table its own original load feeds (dataflow
    # into the next indexed JMP, see `_selector_dispatch`), and the pair must
    # be corroborated by layout: the map immediately follows that table.
    # Tables whose JMP no selector load feeds are direct dispatches and are
    # proved independently from their own guard. A population may mix both
    # kinds, but every selector-fed JMP in the original body must declare
    # its map, so omitting a map never turns a mapped index into a direct one.
    tables_by_jump = {row[1].address: row for row in table_rows}
    selector_by_table = {}
    for selector_row in sorted(selector_rows, key=lambda row: row[0].span.start):
        selector_binding, selector_instruction, _ = selector_row
        dispatch = _selector_dispatch(body_instructions, selector_instruction)
        if dispatch is None:
            raise ValueError("orphan switch selector map does not feed one indexed jump")
        table_row = tables_by_jump.get(dispatch.address)
        if table_row is None:
            raise ValueError("switch selector map feeds an undeclared target table")
        if table_row[1].address in selector_by_table:
            raise ValueError("switch target table has ambiguous selector maps")
        if selector_binding.span.start != table_row[0].span.end:
            raise ValueError("switch selector map does not immediately follow the target table it feeds")
        selector_by_table[table_row[1].address] = selector_row
    dispatch_rows = [(table_row, selector_by_table.get(table_row[1].address))
                     for table_row in sorted(table_rows, key=lambda row: row[0].span.start)]
    table_evidence = {row[0][0].evidence.address for row in dispatch_rows}
    if _indexed_jump_tables(body_instructions) != table_evidence:
        raise ValueError("associated target tables do not cover every indexed indirect jump")
    indirect_jumps = {instruction.address for instruction in body_instructions
                       if instruction.group(x86.X86_GRP_JUMP)
                       and _branch_target(instruction) is None}
    if indirect_jumps != table_evidence:
        raise ValueError("parent body contains an unmodeled indirect JMP")
    if _selector_loads(body_instructions) != {row[1].address for row in selector_rows}:
        raise ValueError("associated selector maps do not cover every selector-indexed jump")
    for table_row, _selector_row in dispatch_rows:
        table_binding = table_row[0]
        if table_binding.span.start < span.end or table_binding.span.start % 4:
            raise ValueError("switch target table must begin after the body at a DWORD-aligned address")
    # Establish the complete indirect-edge set from every declared table
    # before considering any guard. All destinations must be relocated,
    # bounded by the mapped body and decoded instruction starts.
    body_instruction_addresses = {instruction.address for instruction in body_instructions}
    indirect_targets = set()
    for table_row, _selector_row in dispatch_rows:
        table_binding = table_row[0]
        target_count = (table_binding.span.end - table_binding.span.start) // 4
        for index in range(target_count):
            cell = table_binding.span.start + index * 4
            cell_span = Span(cell, cell + 4)
            if image.absolute_sites(cell_span) != frozenset({0}):
                raise ValueError("switch target cell lacks exactly one original PE HIGHLOW relocation")
            target_address = struct.unpack("<I", image.read(cell_span))[0]
            if not span.start <= target_address < span.end:
                raise ValueError("original switch target escapes the independently bounded parent code")
            if target_address not in body_instruction_addresses:
                raise ValueError("original switch target is not a decoded parent instruction boundary")
            indirect_targets.add(target_address)
    envelope = Span(span.start, max([span.end, *(item.end for item in ordered)]))
    # The FPO record describes the candidate's own code/data extent. When it
    # differs from the original envelope the candidate body has another size:
    # a measured mismatch for the comparator below, never a PASS.
    fpo_extent_error = None
    if fpo_witness is not None and fpo_witness["code_size"] != envelope.end - envelope.start:
        fpo_extent_error = {"kind": "fpo_extent", "evidence": envelope.start,
                            "candidate_offset": fpo_witness["code_size"],
                            "reason": "FPO metadata extent differs from the full declared code/data envelope"}
    original = image.read(envelope)
    containing_sections = [(index, region, size) for index, (region, _, size) in enumerate(image.sections)
                           if region.start <= envelope.start and envelope.end <= region.start + size]
    if (len(containing_sections) != 1
            or not image.section_flags[containing_sections[0][0]] & 0x20
            or not image.section_flags[containing_sections[0][0]] & 0x20000000):
        raise ValueError("original code/data envelope must be file-backed in one executable PE code section")
    section_base = span.start
    candidate, relocations, alignment = obj.section_subject(section_number)
    # Every byte between code/data regions is explicit alignment fill. It is
    # included in the ordinary strict comparison and is never silently skipped.
    boundaries = [span.end, *(point for data_span in ordered for point in (data_span.start, data_span.end))]
    boundaries = sorted(set(boundaries))
    gap_anchor_errors = []
    for left, right in zip(boundaries, boundaries[1:]):
        in_data = any(data_span.start <= left and right <= data_span.end for data_span in ordered)
        if in_data:
            continue
        if left < span.end:
            continue
        gap = Span(left, right)
        original_gap = image.read(gap)
        candidate_offset = gap.start - section_base
        candidate_gap = candidate[candidate_offset:candidate_offset + (gap.end - gap.start)]
        first_table = min(row[0].span.start for row in table_rows)
        switch_alignment = (left == span.end and right == first_table
                            and _is_canonical_hotpatch_padding(original_gap)
                            and span.end % 4 == 2 and first_table % 4 == 0)
        # Classify the original gap from the original alone. A candidate that
        # spells or places this gap differently is a byte mismatch reported by
        # the strict envelope comparison below, never a refused measurement.
        intergroup_switch_alignment = _is_intergroup_hotpatch_gap(
            original_gap, original_gap, left, right,
            {row[0].span.end for row in selector_rows},
            {row[0].span.start for row in table_rows})
        intergroup_lea_alignment = _is_intergroup_lea_identity_gap(
            original_gap, original_gap, left, right,
            {row[0].span.end for row in selector_rows},
            {row[0].span.start for row in table_rows})
        lea_identity_alignment = (
            left == span.end and right == first_table
            and len(original_gap) == 3
            and (span.end + len(original_gap)) % 4 == 0
            and first_table % 4 == 0
            and _is_canonical_lea_identity_padding(original_gap)
        ) or intergroup_lea_alignment
        if candidate_gap == original_gap and lea_identity_alignment:
            # The candidate is classified only when it preserves the exact
            # original instruction bytes. If it differs, keep the original
            # gap bounded here and let the strict envelope comparison report
            # the candidate byte mismatch below.
            lea_identity_alignment = _is_canonical_lea_identity_padding(candidate_gap)
        ordinary_alignment = (left == span.end and right == first_table
                              and len(original_gap) <= 3
                              and (span.end + len(original_gap)) % 4 == 0
                              and first_table % 4 == 0)
        intergroup_alignment = (
            left >= span.end and right > left and right - left <= 3
            and left != span.end and right % 4 == 0
            and bool(original_gap) and all(byte in _FILL for byte in original_gap)
        )
        if (not original_gap or (any(byte not in _FILL for byte in original_gap)
                                 and not switch_alignment and not intergroup_switch_alignment
                                 and not lea_identity_alignment)
                or (not ordinary_alignment and not intergroup_alignment and not switch_alignment
                    and not intergroup_switch_alignment
                    and not lea_identity_alignment)
                ):
            raise ValueError("unaccounted code/data gap is not a supported alignment form")
        if candidate_gap and (candidate_gap != original_gap
                              or (any(byte not in _FILL for byte in candidate_gap)
                                  and not switch_alignment and not intergroup_switch_alignment
                                  and not lea_identity_alignment)):
            # The strict whole-envelope comparison reports this candidate
            # mutation as a byte mismatch; do not reinterpret it as a gap.
            pass
        if image.absolute_sites(gap):
            raise ValueError("alignment gap contains an original HIGHLOW relocation")
        if any(relocation.offset < candidate_offset + len(candidate_gap)
               and candidate_offset < relocation.offset + 4 for relocation in relocations):
            # The original gap is proved fill above; a candidate relocation
            # there means the compiled layout differs, a measured mismatch.
            gap_anchor_errors.append({
                "kind": "alignment_gap", "evidence": gap.start, "candidate_offset": candidate_offset,
                "reason": "alignment gap contains a candidate COFF relocation"})

    # Independently derive root-section symbol placement. A caller-supplied
    # local target may confirm this placement but cannot override it.
    resolved_targets = dict(targets)
    for symbol in obj.symbols.values():
        if symbol.section != section_number:
            continue
        key = str(symbol.index)
        address = section_base + symbol.value
        supplied = resolved_targets.get(key)
        if supplied is not None and (type(supplied) is not int or supplied != address):
            raise ValueError("caller target conflicts with independently placed root-COMDAT symbol")
        resolved_targets[key] = address

    section_relocations = obj.sections[section_number - 1].relocations
    # The path proof models every in-body edge except address-taken labels.
    # An EH tail or an absolute body address inside the body (catch/SEH
    # continuations, label tables) would add unmodeled predecessors.
    try:
        address_taken = any(
            span.start <= struct.unpack("<I", image.read(Span(span.start + site, span.start + site + 4)))[0] < span.end
            for site in image.absolute_sites(span))
    except ValueError:
        address_taken = True
    # A cleanup-only C++ EH frame never resumes inside the body: without try
    # blocks the frame handler only runs the cold unwind actions and keeps
    # unwinding. The narrow original EH proof establishes exactly that frame
    # (registration chain, FuncInfo without try/IP/ES tables, no root edge
    # into the cold region, no cleanup edge into the body).
    eh_frame = False
    if eh_tail is not None and eh_cold is not None:
        from .source.eh_regions import original_eh_action_count
        try:
            original_eh_action_count(image, ((span.start, span.end), (eh_cold.start, eh_cold.end)))
            eh_frame = True
        except ValueError:
            eh_frame = False
    dispatch_witnesses = []
    for table_row, selector_row in dispatch_rows:
        witness = _prove_dispatch_extent(
            body_instructions, span, image,
            table_row[0], table_row[1], table_row[2],
            *(selector_row if selector_row is not None else (None, None, None)),
            indirect_targets=frozenset(indirect_targets),
            path_proof=(eh_tail is None or eh_frame) and not address_taken,
            eh_frame=eh_frame)
        dispatch_witnesses.append(witness)

    def report(proof, *, candidate_layout_unproven=False):
        witnesses = [_binding_witness(binding) for binding, _, _ in normalized]
        result = {
            **proof,
            "source_body_span": {"start": span.start, "end": span.end},
            "associated_data": witnesses,
            "comparison_span": {"start": envelope.start, "end": envelope.end},
            "associated_metadata": fpo_witness,
            "source_debug_metadata": debug_witnesses,
            "original_metadata": Result.UNKNOWN,
            "linked_selection": Result.UNKNOWN,
            "runtime": Result.UNKNOWN,
            "limitations": ["proves bounded object bytes/references only; not final linked selection or runtime behavior"],
        }
        result["dispatch_extents"] = dispatch_witnesses
        # Retain the singular report field for consumers of the original v1 API.
        if len(dispatch_witnesses) == 1:
            result["dispatch_extent"] = dispatch_witnesses[0]
        if candidate_layout_unproven:
            result["candidate_anchors"] = Result.UNKNOWN
            result["limitations"].append(
                "candidate associated-cell layout remains unproved after a measured boundary failure")
        return result

    # In the newly supported all-direct multi-table domain, a complete
    # physical candidate can already contradict the fixed source envelope.
    # Let the ordinary comparator validate every physical relocation and
    # report that conclusive boundary FAIL before looking for cells at the
    # original offsets. This admits no shifted layout or candidate-cell proof.
    # Short candidates and all existing singleton/selector paths retain their
    # original validation/refusal order; every possible PASS still checks cells.
    if len(table_rows) > 1 and not selector_rows and len(candidate) >= len(original):
        boundary_probe = compare_bound_code(
            envelope, original, candidate, relocations, resolved_targets,
            original_absolute_sites=image.absolute_sites(envelope), alignment=alignment)
        if boundary_probe["boundary"] is Result.FAIL:
            return report(boundary_probe, candidate_layout_unproven=True)

    candidate_anchor_errors = [fpo_extent_error] if fpo_extent_error is not None else []
    candidate_anchor_errors.extend(gap_anchor_errors)
    for binding, original_instruction, original_operand in normalized:
        # A missing/changed candidate instruction or operand is a body
        # mismatch. When it still has the original shape, verify the actual
        # COFF relocation at that field and let compare_bound_code establish
        # target equality using its own symbol/addend bytes.
        candidate_instruction = _instruction_at(
            list(_ENGINE.disasm(candidate[:span.end - span.start], 0)),
            binding.evidence.address - span.start)
        if candidate_instruction is not None:
            try:
                _memory_operand(candidate_instruction, binding.kind)
                _candidate_anchor(obj, section_number, candidate, candidate_instruction)
            except ValueError as error:
                candidate_anchor_errors.append({
                    "kind": binding.kind, "evidence": binding.evidence.address,
                    "candidate_offset": binding.evidence.address - span.start,
                    "reason": str(error)})
        else:
            candidate_anchor_errors.append({
                "kind": binding.kind, "evidence": binding.evidence.address,
                "candidate_offset": binding.evidence.address - span.start,
                "reason": "candidate anchor is not a decoded instruction start"})
        if binding.kind == "switch_targets":
            count = (binding.span.end - binding.span.start) // 4
            for index in range(count):
                cell = binding.span.start + index * 4
                original_cell = image.read(Span(cell, cell + 4))
                if image.absolute_sites(Span(cell, cell + 4)) != frozenset({0}):
                    raise ValueError("switch target cell lacks exactly one original PE HIGHLOW relocation")
                target_address = struct.unpack("<I", original_cell)[0]
                if not span.start <= target_address < span.end:
                    raise ValueError("original switch target escapes the independently bounded parent code")
                if target_address not in {item.address for item in body_instructions}:
                    raise ValueError("original switch target is not a decoded parent instruction boundary")
                cell_offset = cell - section_base
                try:
                    _symbol_target(obj, section_number, candidate, cell_offset)
                except ValueError as error:
                    # The original cell is proved above; a candidate without
                    # one local relocation at the same offset has a shifted
                    # or different table, which is a measured body mismatch.
                    candidate_anchor_errors.append({
                        "kind": "switch_target_cell", "evidence": cell,
                        "candidate_offset": cell_offset, "reason": str(error)})
                    break
        else:
            data = image.read(binding.span)
            if image.absolute_sites(binding.span):
                raise ValueError("switch selector bytes must have no original HIGHLOW relocations")
            start = binding.span.start - section_base
            end = binding.span.end - section_base
            if any(offset < end and start < offset + 4 for offset, _, _ in section_relocations):
                # Original selector bytes are relocation-free (checked above);
                # candidate code or data over them is a shifted layout.
                candidate_anchor_errors.append({
                    "kind": "switch_selector", "evidence": binding.span.start, "candidate_offset": start,
                    "reason": "switch selector bytes must have no candidate COFF relocations"})
            if candidate[start:end] != data:
                # Preserve a measured data mismatch as FAIL through the normal
                # comparator below; this branch only ensures the selector is
                # not interpreted via any relocation.
                pass

    proof = compare_bound_code(envelope, original, candidate, relocations, resolved_targets,
                               original_absolute_sites=image.absolute_sites(envelope),
                               alignment=alignment)
    # Equal relocated addresses alone do not establish the required physical
    # local-anchor evidence. Older code discarded these errors and could PASS
    # an external/incorrect anchor whose addend happened to reproduce the VA.
    if candidate_anchor_errors and proof["result"] is Result.PASS:
        details = "; ".join(
            f"{row['kind']} at {row['evidence']:#x}: {row['reason']}"
            for row in candidate_anchor_errors)
        raise ValueError("candidate associated-data anchor proof is unsupported: " + details)
    result = report(proof)
    if candidate_anchor_errors:
        result["candidate_anchors"] = Result.UNKNOWN
        result["unsupported_candidate_anchors"] = candidate_anchor_errors
        if result["references"] is Result.PASS:
            result["references"] = Result.UNKNOWN
    return result
