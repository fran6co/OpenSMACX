"""Narrow owner proof for VC6's disjoint x86 C++-EH code regions.

This binds bounded VC6 destructor-cleanup chains without assigning
generated symbols by section order. It proves a root -> handler -> FuncInfo ->
unwind cleanup chain in the pinned image and one COFF object. It returns
bounded bytes for a caller to compare; it grants no match verdict.
"""

from __future__ import annotations

from dataclasses import dataclass, replace
from enum import StrEnum
import struct
from typing import Mapping

from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_GRP_JUMP, CS_MODE_32, Cs, x86

from ..binary import Relocation, RelocationKind
from ..model import Span
from ..pe_coff import Coff, PeImage, Symbol
from .eh_actions import (EHActionFact, EHActionKind, EHTransferFact,
                         EHTransferRole, resolve_transfer)


_CODE = 0x00000020
_INITIALIZED_DATA = 0x00000040
_EXECUTE = 0x20000000
_READ = 0x40000000
_WRITE = 0x80000000
_COMDAT = 0x00001000
_FUNCINFO_MAGIC = 0x19930520
_FUNCINFO_HEADER_SIZE = 32
_MAX_CLEANUP_ACTIONS = 64
_MAX_ACTION_SIZE = 34
_HANDLER_STUB_SIZE = 10
_MAX_COLD_SIZE = _MAX_CLEANUP_ACTIONS * _MAX_ACTION_SIZE + _HANDLER_STUB_SIZE
_MAX_LOCAL_EH_DEPTH = 4
_MAX_LOCAL_EH_ROOTS = 16
_EH_RUNTIME = "___CxxFrameHandler"

_ENGINE = Cs(CS_ARCH_X86, CS_MODE_32)
_ENGINE.detail = True


class EHRegionRole(StrEnum):
    ROOT = "root"
    COLD = "cold_cleanup_and_handler"


@dataclass(frozen=True)
class EHOwnershipEvidence:
    """The validated graph joining image spans to this COFF root/child."""

    root_section: int
    child_section: int
    func_info_section: int
    handler_offset: int
    original_handler_offset: int
    cleanup_offset: int
    original_handler: int
    original_func_info: int
    original_cleanup: int
    cleanup_offsets: tuple[int, ...]
    original_cleanups: tuple[int, ...]
    original_predecessors: tuple[int, ...]
    compiled_predecessors: tuple[int, ...]
    original_cleanup_targets: tuple[int, ...]
    original_runtime_target: int
    compiled_runtime_symbol: str
    original_actions: tuple[EHActionFact, ...]
    compiled_actions: tuple[EHActionFact, ...]


@dataclass(frozen=True)
class EHRegionPair:
    """One exact original span and its physically owned COFF code section."""

    role: EHRegionRole
    original: Span
    section: int
    code: bytes
    relocations: tuple[Relocation, ...]
    alignment: int
    ownership: EHOwnershipEvidence


def eh_action_semantic_difference(original: EHActionFact,
                                 compiled: EHActionFact,
                                 index: int) -> tuple[str, str] | None:
    """Shared bounded semantic check used by both EH comparison paths."""
    if original.kind is not compiled.kind:
        return "ordinary", f"valid EH action kind differs at index {index}"
    if original.kind is EHActionKind.ARRAY_DESTRUCTOR:
        original_facts = (original.element_count, original.element_size,
                          original.frame_displacement, original.base_adjustment,
                          original.scratch_register)
        compiled_facts = (compiled.element_count, compiled.element_size,
                          compiled.frame_displacement, compiled.base_adjustment,
                          compiled.scratch_register)
        if original_facts != compiled_facts:
            return ("ordinary",
                    f"valid EH array-cleanup semantics differ at index {index}: "
                    f"original={original_facts!r}, compiled={compiled_facts!r}")
    elif original.kind in (EHActionKind.FRAME_TRANSFER,
                           EHActionKind.FRAME_SLOT_CALL_CLEANUP):
        original_facts = (original.frame_displacement, original.base_adjustment,
                          original.scratch_register)
        compiled_facts = (compiled.frame_displacement, compiled.base_adjustment,
                          compiled.scratch_register)
        if original_facts != compiled_facts:
            label = ("frame-transfer" if original.kind is EHActionKind.FRAME_TRANSFER
                     else "frame-local cleanup")
            return ("frame-transfer" if original.kind is EHActionKind.FRAME_TRANSFER
                    else "ordinary",
                    f"valid EH {label} semantics differ at index {index}: "
                    f"original={original_facts!r}, compiled={compiled_facts!r}")
    return None


def _image_section(image: PeImage, span: Span, *, code: bool) -> int:
    matches = [index for index, (region, _offset, size) in enumerate(image.sections)
               if region.start <= span.start and span.end <= region.start + size]
    if len(matches) != 1:
        raise ValueError("EH region is not uniquely backed by one PE section")
    index = matches[0]
    flags = image.section_flags[index]
    if code:
        if not flags & _CODE or not flags & _EXECUTE or flags & _WRITE:
            raise ValueError("EH code region is not in bounded executable image storage")
    elif (flags & (_CODE | _EXECUTE | _WRITE)
          or not flags & (_INITIALIZED_DATA | 0x00000080)
          or not flags & _READ):
        raise ValueError("FuncInfo is not in bounded read-only image data")
    return index


def _checked_spans(spans: tuple[tuple[int, int], ...]) -> tuple[Span, Span]:
    if type(spans) is not tuple or len(spans) != 2:
        raise ValueError("VC6 EH binding requires exactly primary and cold spans")
    result = []
    for pair in spans:
        if (type(pair) is not tuple or len(pair) != 2
                or any(type(value) is not int for value in pair)):
            raise ValueError("EH span bounds must be integer pairs")
        low, high = pair
        if not 0 <= low < high < 2**32:
            raise ValueError("EH span is empty or outside i386 address space")
        result.append(Span(low, high))
    primary, cold = result
    if primary.start >= cold.start or primary.end > cold.start:
        raise ValueError("EH spans are reordered or overlap")
    cold_size = cold.end - cold.start
    if not _HANDLER_STUB_SIZE < cold_size <= _MAX_COLD_SIZE:
        raise ValueError("cold EH region exceeds the bounded VC6 extent")
    return primary, cold


def _decoded(code: bytes, base: int):
    instructions = list(_ENGINE.disasm(code, base))
    if sum(item.size for item in instructions) != len(code):
        raise ValueError("EH instruction region contains undecoded bytes")
    return instructions


def _registration_push(instructions, target: int) -> int | None:
    """Return the handler immediate for one bounded VC6 EH setup schedule."""
    def fs_load(item):
        return (item.mnemonic == "mov" and len(item.operands) == 2
                and _is_reg(item.operands[0], x86.X86_REG_EAX)
                and item.operands[1].type == x86.X86_OP_MEM
                and item.operands[1].size == 4
                and item.operands[1].mem.segment == x86.X86_REG_FS
                and item.operands[1].mem.base == 0
                and item.operands[1].mem.index == 0
                and item.operands[1].mem.disp == 0)

    def sentinel(item):
        return (item.mnemonic == "push" and len(item.operands) == 1
                and item.operands[0].type == x86.X86_OP_IMM
                and item.operands[0].size == 4 and item.operands[0].imm == -1
                and item.encoding.imm_size in (1, 4))

    def handler_push(item):
        return (item.mnemonic == "push" and len(item.operands) == 1
                and item.operands[0].type == x86.X86_OP_IMM
                and item.operands[0].size == 4 and item.operands[0].imm == target
                and item.encoding.imm_size == 4)

    def push_eax(item):
        return (item.mnemonic == "push" and len(item.operands) == 1
                and _is_reg(item.operands[0], x86.X86_REG_EAX))

    def fs_install(item):
        return (item.mnemonic == "mov" and len(item.operands) == 2
                and item.operands[0].type == x86.X86_OP_MEM
                and item.operands[0].size == 4
                and item.operands[0].mem.segment == x86.X86_REG_FS
                and item.operands[0].mem.base == 0
                and item.operands[0].mem.index == 0
                and item.operands[0].mem.disp == 0
                and _is_reg(item.operands[1], x86.X86_REG_ESP))

    def permitted_saved_eax_move(item):
        if (item.mnemonic != "mov" or len(item.operands) != 2
                or not _is_reg(item.operands[0], x86.X86_REG_EAX)):
            return False
        source = item.operands[1]
        if source.type == x86.X86_OP_IMM:
            return source.size == 4 and item.encoding.imm_size == 4
        return (source.type == x86.X86_OP_MEM and source.size == 4
                and source.mem.segment == 0
                and source.mem.base == x86.X86_REG_ESP
                and source.mem.index == 0 and source.mem.disp == 0x14)

    schedules = (
        ("sentinel-handler-load", (sentinel, handler_push, fs_load, push_eax)),
        ("load-sentinel-handler", (fs_load, sentinel, handler_push, push_eax)),
        ("sentinel-load-handler", (sentinel, fs_load, handler_push, push_eax)),
    )
    matches = []
    for index in range(len(instructions)):
        for _name, prefix in schedules:
            for saved_move in (False, True):
                width = len(prefix) + int(saved_move) + 1
                candidate = instructions[index:index + width]
                if len(candidate) != width:
                    continue
                if not all(check(item) for check, item in zip(prefix, candidate)):
                    continue
                cursor = len(prefix)
                if saved_move:
                    if not permitted_saved_eax_move(candidate[cursor]):
                        continue
                    cursor += 1
                if not fs_install(candidate[cursor]):
                    continue
                handler = candidate[1] if _name == "sentinel-handler-load" else candidate[2]
                matches.append(handler.address + handler.encoding.imm_offset)
    if len(matches) != 1:
        return None
    return matches[0]


def _image_registration_edge(image: PeImage, primary: Span, cold: Span,
                             code: bytes, target: int) -> None:
    instructions = _decoded(code, primary.start)
    site = _registration_push(instructions, target)
    if site is None:
        raise ValueError("original root lacks one complete relocated EH registration chain")
    for instruction in instructions:
        if not (instruction.group(CS_GRP_CALL) or instruction.group(CS_GRP_JUMP)):
            continue
        if any(operand.type == x86.X86_OP_IMM
               and cold.start <= (operand.imm & 0xFFFFFFFF) < cold.end
               for operand in instruction.operands):
            raise ValueError("original root has a direct relative edge into its EH child")
    offset = site - primary.start
    sites = image.absolute_sites(primary)
    if offset not in sites:
        raise ValueError("original handler PUSH lacks a PE HIGHLOW relocation")
    for other in sites - {offset}:
        value, = struct.unpack("<I", image.read(Span(primary.start + other,
                                                      primary.start + other + 4)))
        if cold.start <= value < cold.end:
            raise ValueError("original root has an extra absolute edge into its EH child")


def _exec_target(image: PeImage, target: int, *, outside: tuple[Span, ...]) -> None:
    if any(span.start <= target < span.end for span in outside):
        raise ValueError("EH branch unexpectedly targets an EH region")
    _image_section(image, Span(target, target + 1), code=True)


def _is_reg(operand, register: int, size: int = 4) -> bool:
    return (operand.type == x86.X86_OP_REG and operand.reg == register
            and operand.size == size)


def _is_imm_push(instruction) -> int | None:
    if instruction.mnemonic != "push" or len(instruction.operands) != 1:
        return None
    operand = instruction.operands[0]
    if operand.type != x86.X86_OP_IMM:
        return None
    if not ((instruction.bytes[0] == 0x6A and instruction.encoding.imm_size == 1)
            or (instruction.bytes[0] == 0x68 and instruction.encoding.imm_size == 4)):
        return None
    return operand.imm if 0 < operand.imm < 2**31 else None


def _frame_array_load(instruction) -> int | None:
    if (instruction.mnemonic != "mov" or len(instruction.operands) != 2
            or not _is_reg(instruction.operands[0], x86.X86_REG_EAX)
            or instruction.operands[1].type != x86.X86_OP_MEM
            or instruction.operands[1].size != 4):
        return None
    memory = instruction.operands[1].mem
    if memory.base != x86.X86_REG_EBP or memory.index != 0 or memory.segment != 0:
        return None
    if (instruction.size == 3 and instruction.bytes[:2] == b"\x8b\x45"
            and instruction.encoding.disp_size == 1):
        return memory.disp
    if (instruction.size == 6 and instruction.bytes[:2] == b"\x8b\x85"
            and instruction.encoding.disp_size == 4):
        return memory.disp
    return None


def _array_add(instruction) -> int | None:
    if (instruction.mnemonic != "add" or len(instruction.operands) != 2
            or not _is_reg(instruction.operands[0], x86.X86_REG_EAX)
            or instruction.operands[1].type != x86.X86_OP_IMM):
        return None
    if (instruction.size == 3 and instruction.bytes[:2] == b"\x83\xc0"
            and instruction.encoding.imm_size == 1):
        return instruction.operands[1].imm
    if (instruction.size == 6 and instruction.bytes[:2] == b"\x81\xc0"
            and instruction.encoding.imm_size == 4):
        return instruction.operands[1].imm
    if (instruction.size == 5 and instruction.bytes[:1] == b"\x05"
            and instruction.encoding.imm_size == 4):
        return instruction.operands[1].imm
    return None


def _cleanup_action(instructions, *, code_base: int = 0) -> EHActionFact:
    """Decode one complete bounded action into typed transfer facts."""
    if not instructions:
        raise ValueError("EH action is empty")
    start = instructions[0].address - code_base
    end = instructions[-1].address + instructions[-1].size - code_base

    if len(instructions) == 8:
        callback, count_push, size_push, frame_load, adjustment, base_push, call, ret = instructions
        count = _is_imm_push(count_push)
        element_size = _is_imm_push(size_push)
        displacement = _frame_array_load(frame_load)
        base_adjustment = _array_add(adjustment)
        if (callback.mnemonic == "push" and callback.bytes[:1] == b"\x68"
                and len(callback.operands) == 1
                and callback.operands[0].type == x86.X86_OP_IMM
                and callback.size == 5 and callback.encoding.imm_offset == 1
                and callback.encoding.imm_size == 4
                and count is not None and element_size is not None
                and displacement is not None and base_adjustment is not None
                and base_push.mnemonic == "push" and len(base_push.operands) == 1
                and _is_reg(base_push.operands[0], x86.X86_REG_EAX)
                and base_push.size == 1 and base_push.bytes == b"\x50"
                and call.mnemonic == "call" and call.bytes[:1] == b"\xe8"
                and call.size == 5 and call.encoding.imm_offset == 1
                and len(call.operands) == 1
                and call.operands[0].type == x86.X86_OP_IMM
                and call.encoding.imm_size == 4 and ret.mnemonic == "ret"
                and ret.bytes == b"\xc3"):
            transfers = (
                EHTransferFact(EHTransferRole.CALLBACK,
                               callback.address + callback.encoding.imm_offset - code_base,
                               4, "ABS32", None,
                               callback.operands[0].imm & 0xFFFFFFFF, None),
                EHTransferFact(EHTransferRole.HELPER,
                               call.address + call.encoding.imm_offset - code_base,
                               4, "REL32", None, call.operands[0].imm & 0xFFFFFFFF, None),
            )
            return EHActionFact(EHActionKind.ARRAY_DESTRUCTOR, start, end,
                                transfers, count, element_size, displacement,
                                base_adjustment, "eax")

    if len(instructions) == 5:
        frame_load, argument_push, call, argument_pop, ret = instructions
        displacement = None
        if (frame_load.mnemonic == "mov" and len(frame_load.operands) == 2
                and _is_reg(frame_load.operands[0], x86.X86_REG_EAX)
                and frame_load.operands[1].type == x86.X86_OP_MEM
                and frame_load.operands[1].size == 4):
            memory = frame_load.operands[1].mem
            if (memory.base == x86.X86_REG_EBP and memory.index == 0
                    and memory.segment == 0):
                if (frame_load.size == 3 and frame_load.bytes[:2] == b"\x8b\x45"
                        and frame_load.encoding.disp_size == 1):
                    displacement = memory.disp
                elif (frame_load.size == 6 and frame_load.bytes[:2] == b"\x8b\x85"
                      and frame_load.encoding.disp_size == 4):
                    displacement = memory.disp
        if (displacement is not None and displacement != 0 and displacement % 4 == 0
                and argument_push.mnemonic == "push"
                and len(argument_push.operands) == 1
                and _is_reg(argument_push.operands[0], x86.X86_REG_EAX)
                and argument_push.bytes == b"\x50"
                and call.mnemonic == "call" and call.bytes[:1] == b"\xe8"
                and call.size == 5 and call.encoding.imm_offset == 1
                and call.encoding.imm_size == 4 and len(call.operands) == 1
                and call.operands[0].type == x86.X86_OP_IMM
                and argument_pop.mnemonic == "pop"
                and len(argument_pop.operands) == 1
                and _is_reg(argument_pop.operands[0], x86.X86_REG_ECX)
                and argument_pop.bytes == b"\x59"
                and ret.mnemonic == "ret" and ret.bytes == b"\xc3"):
            transfer = EHTransferFact(
                EHTransferRole.CLEANUP,
                call.address + call.encoding.imm_offset - code_base,
                4, "REL32", None, call.operands[0].imm & 0xFFFFFFFF, None)
            return EHActionFact(
                EHActionKind.FRAME_SLOT_CALL_CLEANUP, start, end, (transfer,),
                frame_displacement=displacement, base_adjustment=0,
                scratch_register="eax")

    target_instruction = None
    first = instructions[0]
    frame_displacement = None
    base_adjustment = 0
    scratch_register = "ecx"
    if (len(instructions) == 2 and first.mnemonic == "lea"
            and len(first.operands) == 2
            and _is_reg(first.operands[0], x86.X86_REG_ECX)
            and first.operands[1].type == x86.X86_OP_MEM
            and first.operands[1].mem.base == x86.X86_REG_EBP
            and first.operands[1].mem.index == 0
            and first.operands[1].mem.segment == 0
            and first.operands[1].size == 4 and first.size in (3, 6)
            and first.bytes[:2] == (b"\x8d\x4d" if first.size == 3 else b"\x8d\x8d")
            and first.encoding.disp_size == first.size - 2):
        target_instruction = instructions[1]
        frame_displacement = first.operands[1].mem.disp
    elif (len(instructions) in (2, 3) and first.mnemonic == "mov"
          and len(first.operands) == 2
          and _is_reg(first.operands[0], x86.X86_REG_ECX)
          and first.operands[1].type == x86.X86_OP_MEM
          and first.operands[1].size == 4
          and first.operands[1].mem.base == x86.X86_REG_EBP
          and first.operands[1].mem.index == 0
          and first.operands[1].mem.segment == 0
          and first.size == 3 and first.bytes[:2] == b"\x8b\x4d"
          and first.encoding.disp_size == 1):
        target_instruction = instructions[-1]
        frame_displacement = first.operands[1].mem.disp
        if len(instructions) == 3:
            adjustment = instructions[1]
            ecx_adjustment = (
                len(adjustment.operands) == 2
                and _is_reg(adjustment.operands[0], x86.X86_REG_ECX)
                and adjustment.operands[1].type == x86.X86_OP_IMM
            )
            add_form = (
                adjustment.mnemonic == "add"
                and ((adjustment.size == 3 and adjustment.bytes[:2] == b"\x83\xc1"
                      and adjustment.encoding.imm_size == 1)
                     or (adjustment.size == 6 and adjustment.bytes[:2] == b"\x81\xc1"
                         and adjustment.encoding.imm_size == 4))
            )
            sub_form = (
                adjustment.mnemonic == "sub"
                and ((adjustment.size == 3 and adjustment.bytes[:2] == b"\x83\xe9"
                      and adjustment.encoding.imm_size == 1)
                     or (adjustment.size == 6 and adjustment.bytes[:2] == b"\x81\xe9"
                         and adjustment.encoding.imm_size == 4))
            )
            if not (ecx_adjustment and (add_form or sub_form)):
                target_instruction = None
            else:
                immediate = adjustment.operands[1].imm
                if adjustment.encoding.imm_size == 1:
                    immediate = ((immediate & 0xFF) ^ 0x80) - 0x80
                else:
                    immediate = ((immediate & 0xFFFFFFFF) ^ 0x80000000) - 0x80000000
                base_adjustment = immediate if adjustment.mnemonic == "add" else -immediate
    if (target_instruction is None or target_instruction.mnemonic != "jmp"
            or target_instruction.size != 5 or target_instruction.bytes[:1] != b"\xe9"
            or len(target_instruction.operands) != 1
            or target_instruction.operands[0].type != x86.X86_OP_IMM
            or target_instruction.encoding.imm_offset != 1
            or target_instruction.encoding.imm_size != 4):
        raise ValueError("unwind map does not bound a supported complete EH action")
    transfer = EHTransferFact(
        EHTransferRole.CLEANUP,
        target_instruction.address + target_instruction.encoding.imm_offset - code_base,
        4, "REL32", None, target_instruction.operands[0].imm & 0xFFFFFFFF, None)
    return EHActionFact(EHActionKind.FRAME_TRANSFER, start, end, (transfer,),
                        frame_displacement=frame_displacement,
                        base_adjustment=base_adjustment,
                        scratch_register=scratch_register)


def _handler_stub(code: bytes, base: int) -> tuple[int, int, int]:
    """Return handler offset, FuncInfo immediate offset, and runtime target."""
    if len(code) <= _HANDLER_STUB_SIZE:
        raise ValueError("EH child is shorter than a cleanup plus handler stub")
    handler_offset = len(code) - _HANDLER_STUB_SIZE
    handler = code[handler_offset:]
    instructions = _decoded(handler, base + handler_offset)
    if (len(instructions) != 2 or instructions[0].mnemonic != "mov"
            or len(instructions[0].operands) != 2
            or instructions[0].operands[0].type != x86.X86_OP_REG
            or instructions[0].operands[0].reg != x86.X86_REG_EAX
            or instructions[0].size != 5 or instructions[0].bytes[0] != 0xB8
            or instructions[0].encoding.imm_offset != 1
            or instructions[0].encoding.imm_size != 4
            or instructions[0].operands[1].type != x86.X86_OP_IMM
            or instructions[1].mnemonic != "jmp" or instructions[1].size != 5
            or instructions[1].bytes[0] != 0xE9
            or instructions[1].encoding.imm_offset != 1
            or instructions[1].encoding.imm_size != 4
            or len(instructions[1].operands) != 1
            or instructions[1].operands[0].type != x86.X86_OP_IMM):
        raise ValueError("EH child does not end in the bounded FuncInfo runtime stub")
    return (handler_offset,
            handler_offset + instructions[0].encoding.imm_offset,
            instructions[1].operands[0].imm & 0xFFFFFFFF)


def _unwind_map(data: bytes, count: int, *, base: int,
                first_action: int) -> tuple[tuple[int, ...], tuple[int, ...]]:
    """Validate a bounded backward-only unwind graph and its action starts."""
    if not 1 <= count <= _MAX_CLEANUP_ACTIONS:
        raise ValueError("EH cleanup action count exceeds the bounded resource cap")
    if len(data) != count * 8:
        raise ValueError("EH unwind map has an incomplete action table")
    entries = struct.unpack("<" + "I" * (count * 2), data)
    actions, predecessors = [], []
    for index in range(count):
        predecessor, action = entries[index * 2:index * 2 + 2]
        if predecessor != 0xFFFFFFFF and predecessor >= index:
            raise ValueError("EH unwind predecessor is not a sentinel or earlier state")
        if (action < base or (index == 0 and action != first_action)
                or (actions and action <= actions[-1])):
            raise ValueError("EH unwind actions are not strictly ordered from the first cleanup")
        predecessors.append(predecessor)
        actions.append(action)
    return tuple(actions), tuple(predecessors)


def _action_targets(code: bytes, base: int, starts: tuple[int, ...],
                    handler_offset: int) -> tuple[tuple[int, ...], tuple[EHActionFact, ...]]:
    if not starts or len(starts) > _MAX_CLEANUP_ACTIONS or starts[0] != 0:
        raise ValueError("EH cleanup action starts exceed bounds or omit the first action")
    if any(start < 0 or start >= handler_offset for start in starts):
        raise ValueError("EH cleanup action start lies outside the cold action region")
    if any(left >= right for left, right in zip(starts, starts[1:])):
        raise ValueError("EH cleanup action starts are duplicate or unordered")
    boundaries = (*starts, handler_offset)
    targets, actions = [], []
    for start, end in zip(boundaries, boundaries[1:]):
        instructions = _decoded(code[start:end], base + start)
        action = _cleanup_action(instructions, code_base=base)
        if (action.start != start or action.end != end
                or action.end - action.start > _MAX_ACTION_SIZE):
            raise ValueError("EH action does not exactly cover its mapped boundary")
        primary_transfer = (next((item for item in action.transfers
                                  if item.role is EHTransferRole.CLEANUP), None)
                            if action.kind in (EHActionKind.FRAME_TRANSFER,
                                               EHActionKind.FRAME_SLOT_CALL_CLEANUP) else
                            next((item for item in action.transfers
                                  if item.role is EHTransferRole.HELPER), None))
        if primary_transfer is None or not isinstance(primary_transfer.target, int):
            raise ValueError("original EH action lacks its decoded control target")
        target = primary_transfer.target
        targets.append(target)
        actions.append(action)
    return tuple(targets), tuple(actions)


def _original_eh(image: PeImage, primary: Span, cold: Span):
    main = image.read(primary)
    cold_code = image.read(cold)
    handler_offset, info_offset, runtime_target = _handler_stub(cold_code, cold.start)
    handler = cold.start + handler_offset
    _image_section(image, primary, code=True)
    _image_section(image, cold, code=True)
    _image_registration_edge(image, primary, cold, main, handler)
    handler_ins = _decoded(cold_code[handler_offset:], handler)
    func_info = handler_ins[0].operands[1].imm & 0xFFFFFFFF
    _exec_target(image, runtime_target, outside=(primary, cold))

    header = image.read(Span(func_info, func_info + _FUNCINFO_HEADER_SIZE))
    (magic, max_state, unwind_map, try_count, try_map,
     ip_count, ip_map, es_types) = struct.unpack("<8I", header)
    if not 1 <= max_state <= _MAX_CLEANUP_ACTIONS:
        raise ValueError("original FuncInfo exceeds the bounded cleanup action cap")
    fi_span = Span(func_info, func_info + _FUNCINFO_HEADER_SIZE + 8 * max_state)
    _image_section(image, fi_span, code=False)
    fi_data = image.read(fi_span)
    if (magic != _FUNCINFO_MAGIC or unwind_map != func_info + _FUNCINFO_HEADER_SIZE
            or any((try_count, try_map, ip_count, ip_map, es_types))):
        raise ValueError("original FuncInfo is not a supported bounded cleanup layout")
    map_span = Span(unwind_map, unwind_map + 8 * max_state)
    map_data = image.read(map_span)
    original_cleanups, original_predecessors = _unwind_map(
        map_data, max_state, base=cold.start, first_action=cold.start)
    if any(action >= handler for action in original_cleanups):
        raise ValueError("original unwind action starts at or beyond its handler")
    original_action_starts = original_cleanups
    cleanup_targets, original_actions = _action_targets(
        cold_code, cold.start,
        tuple(action - cold.start for action in original_action_starts),
        handler_offset)
    for target in cleanup_targets:
        _exec_target(image, target, outside=(primary, cold))
    callback_sites = set()
    original_actions = tuple(
        replace(action, transfers=tuple(
            replace(transfer, relocation_kind=("HIGHLOW"
                     if transfer.role is EHTransferRole.CALLBACK else None))
            for transfer in action.transfers))
        for action in original_actions)
    for action in original_actions:
        for transfer in action.transfers:
            if transfer.role is EHTransferRole.CALLBACK:
                if not isinstance(transfer.target, int):
                    raise ValueError("original array cleanup lacks its absolute callback target")
                _exec_target(image, transfer.target, outside=(primary, cold))
                callback_sites.add(transfer.operand_offset)
    if image.absolute_sites(Span(func_info, func_info + 12)) != frozenset({8}):
        raise ValueError("original FuncInfo unwind-map pointer lacks its exact HIGHLOW site")
    expected_map_sites = frozenset(4 + 8 * index for index in range(max_state))
    if image.absolute_sites(map_span) != expected_map_sites:
        raise ValueError("original unwind cleanup pointer lacks its exact HIGHLOW site")
    if image.absolute_sites(Span(handler, cold.end)) != frozenset({info_offset - handler_offset}):
        raise ValueError("original handler FuncInfo pointer lacks its exact HIGHLOW site")
    expected_fi_sites = frozenset({8} | {36 + 8 * index for index in range(max_state)})
    if image.absolute_sites(fi_span) != expected_fi_sites:
        raise ValueError("original FuncInfo has unexpected HIGHLOW sites in its bounded bundle")
    expected_cold_sites = callback_sites | {info_offset}
    if image.absolute_sites(cold) != frozenset(expected_cold_sites):
        raise ValueError("original cold region has unexpected HIGHLOW sites outside the handler pointer")
    return (handler_offset, handler, func_info, cold.start, tuple(cleanup_targets),
            runtime_target, original_action_starts, original_predecessors,
            tuple(original_actions))


def _associated_root(obj: Coff, section: int, root: int) -> bool:
    current = section
    seen = set()
    while current in obj.associations:
        if current in seen:
            return False
        seen.add(current)
        current = obj.associations[current]
    return current == root


def _relocations(obj: Coff, section: int):
    raw_section = obj.sections[section - 1]
    code, relocations, alignment = obj.section_subject(section)
    if len(code) != obj.section_sizes[section - 1]:
        raise ValueError("COFF EH section is not fully raw-backed")
    # COFF's section_subject rejects unsupported kinds; also reject overlaps
    # and fields that extend past this independently bounded section.
    occupied = set()
    for item in relocations:
        if item.offset < 0 or item.offset + 4 > len(code):
            raise ValueError("COFF EH relocation crosses its section boundary")
        field = set(range(item.offset, item.offset + 4))
        if occupied & field:
            raise ValueError("COFF EH relocations overlap")
        occupied.update(field)
    return raw_section, code, relocations, alignment


def _reloc_at(obj: Coff, section: int, offset: int, kind: RelocationKind) -> Symbol:
    rows = [(at, index, coff_kind)
            for at, index, coff_kind in obj.sections[section - 1].relocations
            if at == offset]
    wanted_kind = 6 if kind is RelocationKind.ABSOLUTE32 else 20
    if len(rows) != 1 or rows[0][2] != wanted_kind:
        raise ValueError("EH ownership edge lacks its unique supported COFF relocation")
    return obj.symbols[rows[0][1]]


def _validate_local_destructor(obj: Coff, symbol: Symbol, root: int, child: int, *,
                               forbidden_sections: frozenset[int] = frozenset(),
                               walk: _LocalEHWalk | None = None) -> None:
    """Admit a bounded independent local ??_D function, without semantic claims."""
    if walk is not None:
        _bind_local_eh_object(obj, walk)
    section_number = symbol.section
    if (section_number <= 0 or not symbol.name.startswith("??_D")
            or section_number in (root, child)
            or section_number in obj.associations
            or section_number > len(obj.sections)):
        raise ValueError("COFF local cleanup target is not an independent ??_D function")
    section = obj.sections[section_number - 1]
    associated_children = [number for number, parent in obj.associations.items()
                           if parent == section_number]
    for number in associated_children:
        metadata = obj.sections[number - 1]
        if (not metadata.name.startswith(".debug$")
                or metadata.flags & (_CODE | _EXECUTE | _WRITE)
                or not metadata.flags & (_INITIALIZED_DATA | 0x00000080)
                or not metadata.flags & _READ):
            raise ValueError("COFF local cleanup target has non-metadata associated ownership")
    if (not section.flags & _CODE or not section.flags & _EXECUTE
            or section.flags & _WRITE or not section.data or symbol.value != 0
            or len(section.data) != obj.section_sizes[section_number - 1]):
        raise ValueError("COFF local cleanup target lacks a bounded executable function section")
    same_section = [candidate for candidate in obj.symbols.values()
                    if candidate.section == section_number and candidate.storage in (2, 3)
                    and candidate.type & 0x20]
    if same_section != [symbol]:
        raise ValueError("COFF local cleanup target is not uniquely owned by one function symbol")
    for _offset, target_index, _kind in section.relocations:
        if obj.symbols[target_index].section in ({root, child} | forbidden_sections):
            raise ValueError("COFF local cleanup target has an ambiguous root/EH relocation")
    _raw, code, _helper_relocations, _alignment = _relocations(obj, section_number)
    if len(code) != obj.section_sizes[section_number - 1]:
        raise ValueError("COFF local cleanup target is not fully raw-backed")
    _decoded(code, 0)
    if walk is not None and walk.frames:
        frame = walk.frames[-1]
        frame.function_identities.add(_symbol_proof_identity(symbol))
        _record_proof_section(obj, walk, section_number, forbidden_refs=True)
        frame.owned_sections.update(associated_children)


def _symbol_proof_identity(symbol: Symbol) -> tuple[object, ...]:
    return (symbol.section, symbol.name, symbol.value, symbol.type, symbol.storage)


def _section_proof_facts(obj: Coff, number: int) -> tuple[object, ...]:
    """Bind all validated physical section/edge facts to this call-local COFF."""
    section = obj.sections[number - 1]
    edges = tuple(
        (offset, target_index, kind,
         _symbol_proof_identity(obj.symbols[target_index]))
        for offset, target_index, kind in section.relocations
    )
    return (number, section.name, section.flags, obj.section_sizes[number - 1],
            section.data, edges, obj.associations.get(number))


@dataclass(frozen=True)
class _LocalEHProofSummary:
    """Immutable, fully validated subtree proof scoped to one Coff instance."""

    obj: Coff
    root_identity: tuple[object, ...]
    function_identities: frozenset[tuple[object, ...]]
    owned_sections: frozenset[int]
    forbidden_refs: frozenset[int]
    section_facts: tuple[tuple[object, ...], ...]
    height: int


@dataclass
class _LocalEHProofFrame:
    """Private post-order accumulator; published only after full validation."""

    root_identity: tuple[object, ...]
    function_identities: set[tuple[object, ...]]
    owned_sections: set[int]
    forbidden_refs: set[int]
    height: int = 0


def _debug_relocation_bounds(obj: Coff, number: int) -> None:
    """Validate debug storage writes, without interpreting code or CV identity."""
    section = obj.sections[number - 1]
    if len(section.data) != obj.section_sizes[number - 1]:
        raise ValueError("COFF local destructor debug metadata is not fully raw-backed")
    occupied = set()
    # Debug SECTION and SECREL are not executable DIR32/REL32 operands.
    # A no-op relocation has no write; unknown kinds do not acquire a width.
    # DIR32NB is accepted only for a directly associated .debug$F child,
    # where it describes a bounded RVA metadata write (not executable code).
    widths = {0: 0, 10: 2, 11: 4}
    if section.name == ".debug$F":
        widths[7] = 4
    for offset, target, kind in section.relocations:
        width = widths.get(kind)
        if (width is None or target not in obj.symbols or offset < 0
                or offset + width > len(section.data)):
            raise ValueError("COFF local destructor has unsupported debug relocation bounds")
        field = set(range(offset, offset + width))
        if occupied & field:
            raise ValueError("COFF local destructor debug relocations overlap")
        occupied.update(field)


@dataclass
class _LocalEHWalk:
    """Bounded active paths and context-specific proofs for local ??1 roots."""

    active: set[int]
    complete: set[tuple[int, frozenset[int], int]]
    roots: int = 1
    obj: Coff | None = None
    summaries: dict[tuple[object, ...], _LocalEHProofSummary] | None = None
    frames: list[_LocalEHProofFrame] | None = None


def _bind_local_eh_object(obj: Coff, walk: _LocalEHWalk) -> None:
    """Keep context keys and summaries scoped to one parsed object."""
    if walk.obj is None:
        walk.obj = obj
    elif walk.obj is not obj:
        raise ValueError("COFF local ??1 walk cannot mix object instances")


def _record_proof_section(obj: Coff, walk: _LocalEHWalk, number: int, *,
                          forbidden_refs: bool = False) -> None:
    if not walk.frames:
        return
    frame = walk.frames[-1]
    frame.owned_sections.add(number)
    if forbidden_refs:
        section = obj.sections[number - 1]
        frame.forbidden_refs.update(
            obj.symbols[target_index].section
            for _offset, target_index, _kind in section.relocations
        )


def _merge_proof_summary(frame: _LocalEHProofFrame,
                         summary: _LocalEHProofSummary) -> None:
    frame.function_identities.update(summary.function_identities)
    frame.owned_sections.update(summary.owned_sections)
    frame.forbidden_refs.update(summary.forbidden_refs)
    frame.height = max(frame.height, summary.height + 1)


def _validate_local_complete_destructor(
        obj: Coff, symbol: Symbol, parent_root: int, parent_child: int,
        walk: _LocalEHWalk, depth: int) -> _LocalEHProofSummary:
    """Prove a local ??1 function and any directly owned EH subtree."""
    _bind_local_eh_object(obj, walk)
    section_number = symbol.section
    if (section_number <= 0 or not symbol.name.startswith("??1")
            or section_number in (parent_root, parent_child)
            or section_number > len(obj.sections)
            or section_number in obj.associations or symbol.value != 0):
        raise ValueError("COFF local ??1 target is not an independent function entry")
    if section_number in walk.active:
        raise ValueError("COFF local ??1 EH ownership contains a cycle")
    section = obj.sections[section_number - 1]
    if (not section.flags & _CODE or not section.flags & _EXECUTE
            or section.flags & _WRITE or not section.flags & _COMDAT
            or not section.data or len(section.data) != obj.section_sizes[section_number - 1]):
        raise ValueError("COFF local ??1 target lacks a bounded executable COMDAT")
    same_section = [candidate for candidate in obj.symbols.values()
                    if candidate.section == section_number and candidate.storage in (2, 3)
                    and candidate.type & 0x20]
    if same_section != [symbol]:
        raise ValueError("COFF local ??1 target is not uniquely owned by one function symbol")
    if obj.function(symbol.name) != symbol:
        raise ValueError("COFF local ??1 target name is absent or ambiguous")
    _raw, code, relocations, _alignment = _relocations(obj, section_number)
    _decoded(code, 0)
    if any(obj.symbols[int(item.target)].section in (*walk.active, parent_child)
           for item in relocations):
        raise ValueError("COFF local ??1 target references an active EH owner")
    if depth > _MAX_LOCAL_EH_DEPTH:
        raise ValueError("COFF local ??1 EH ownership exceeds its bounded traversal budget")
    proof_key = (section_number, frozenset(walk.active), depth)
    if proof_key in walk.complete:
        summary = (walk.summaries or {}).get(_symbol_proof_identity(symbol))
        if summary is not None and summary.obj is obj:
            if walk.frames:
                _merge_proof_summary(walk.frames[-1], summary)
            return summary
        # Exact-context completion without its immutable summary is not
        # sufficient evidence for a caller that needs to compose a proof.
    summary = (walk.summaries or {}).get(_symbol_proof_identity(symbol))
    forbidden = frozenset(walk.active | {parent_child})
    if (summary is not None and summary.obj is obj
            and not (summary.owned_sections & forbidden)
            and not (summary.forbidden_refs & forbidden)
            and depth + summary.height <= _MAX_LOCAL_EH_DEPTH):
        if walk.frames:
            _merge_proof_summary(walk.frames[-1], summary)
        return summary
    if walk.roots >= _MAX_LOCAL_EH_ROOTS:
        raise ValueError("COFF local ??1 EH ownership exceeds its bounded traversal budget")

    direct_children = [number for number, parent in obj.associations.items()
                       if parent == section_number]
    owned_descendants = [number for number in obj.associations
                         if _associated_root(obj, number, section_number)]
    if set(owned_descendants) != set(direct_children):
        raise ValueError("COFF local ??1 target has unsupported nested COMDAT descendants")
    code_children, data_children, unexplained_children = [], [], []
    for number in direct_children:
        child = obj.sections[number - 1]
        if child.name.startswith(".debug$"):
            if (child.flags & (_CODE | _EXECUTE | _WRITE)
                    or not child.flags & (_INITIALIZED_DATA | 0x00000080)
                    or not child.flags & _READ):
                raise ValueError("COFF local ??1 target has unsupported debug ownership")
            _debug_relocation_bounds(obj, number)
        elif child.flags & _CODE:
            code_children.append(number)
        elif child.flags & (_INITIALIZED_DATA | 0x00000080):
            data_children.append(number)
        else:
            unexplained_children.append(number)
    if len(code_children) > 1:
        raise ValueError("COFF local ??1 target has ambiguous associated EH code children")
    if tuple(obj.code_sections(symbol)) != (section_number, *code_children):
        raise ValueError("COFF local ??1 target has hidden associated code descendants")
    if unexplained_children or len(data_children) > 1:
        raise ValueError("COFF local ??1 target has unowned associated data")
    if not code_children and data_children:
        raise ValueError("COFF local ??1 leaf has unsupported associated data")
    if not code_children:
        instructions = _decoded(code, 0)
        if any(operand.type == x86.X86_OP_MEM
               and operand.mem.segment == x86.X86_REG_FS
               for instruction in instructions for operand in instruction.operands):
            raise ValueError("COFF local ??1 leaf has an unsupported FS registration or access")

    frame = _LocalEHProofFrame(
        _symbol_proof_identity(symbol), {_symbol_proof_identity(symbol)},
        {section_number, *direct_children}, set(),
    )
    frame.forbidden_refs.update(obj.symbols[int(item.target)].section
                                for item in relocations)
    if walk.frames is None:
        walk.frames = []
    walk.frames.append(frame)
    walk.active.add(section_number)
    walk.roots += 1
    try:
        if code_children:
            child_number = code_children[0]
            _child, child_code, _relocs, _alignment = _relocations(obj, child_number)
            child_section = obj.sections[child_number - 1]
            if (not child_section.flags & _CODE or not child_section.flags & _EXECUTE
                    or child_section.flags & _WRITE
                    or not child_section.flags & _COMDAT):
                raise ValueError("COFF local ??1 EH child lacks executable COMDAT ownership")
            handler_offset, _actions, _info, _runtime, _starts = _cold_layout(child_code)
            _object_push_edge(obj, symbol, child_number, handler_offset)
            if child_number in walk.active:
                raise ValueError("COFF local ??1 EH child is already active")
            walk.active.add(child_number)
            try:
                (_section, _code, _relocs, _alignment, info_number, _runtime,
                 _handler, _dtors, _starts, _preds) = _object_child_and_info(
                     obj, symbol, child_number, walk=walk, depth=depth + 1)
                _info_section, _info_code, _info_relocs, _info_alignment = _relocations(
                    obj, info_number)
                if (obj.associations.get(info_number) != section_number
                        or data_children != [info_number]):
                    raise ValueError("COFF local ??1 target does not own exactly its EH FuncInfo")
                frame.owned_sections.add(info_number)
            finally:
                walk.active.remove(child_number)
    finally:
        walk.active.remove(section_number)
        if walk.frames and walk.frames[-1] is frame:
            walk.frames.pop()
    facts = tuple(_section_proof_facts(obj, number)
                  for number in sorted(frame.owned_sections))
    summary = _LocalEHProofSummary(
        obj, frame.root_identity, frozenset(frame.function_identities),
        frozenset(frame.owned_sections), frozenset(frame.forbidden_refs),
        facts, frame.height,
    )
    if walk.summaries is None:
        walk.summaries = {}
    walk.summaries[frame.root_identity] = summary
    walk.complete.add(proof_key)
    if walk.frames:
        _merge_proof_summary(walk.frames[-1], summary)
    return summary


def _cold_layout(code: bytes) -> tuple[int, tuple[EHActionFact, ...], int, int, tuple[int, ...]]:
    """Parse bounded adjacent actions followed by one handler stub."""
    if not _HANDLER_STUB_SIZE < len(code) <= _MAX_COLD_SIZE:
        raise ValueError("COFF EH child exceeds the bounded cold extent")
    handler_offset, info_offset, runtime_target = _handler_stub(code, 0)
    instructions = _decoded(code[:handler_offset], 0)
    starts, actions = [], []
    cursor = 0
    index = 0
    while index < len(instructions):
        if len(starts) >= _MAX_CLEANUP_ACTIONS:
            raise ValueError("COFF EH child exceeds the bounded cleanup action cap")
        instruction = instructions[index]
        action = None
        consumed = 0
        if instruction.bytes[:1] == b"\x68":
            if index + 8 <= len(instructions):
                candidate = instructions[index:index + 8]
                try:
                    parsed = _cleanup_action(candidate, code_base=0)
                except ValueError:
                    parsed = None
                if parsed is not None and parsed.kind is EHActionKind.ARRAY_DESTRUCTOR:
                    action, consumed = parsed, 8
        else:
            for length in (2, 3, 5):
                if index + length > len(instructions):
                    continue
                try:
                    parsed = _cleanup_action(instructions[index:index + length], code_base=0)
                except ValueError:
                    continue
                if parsed.kind in (EHActionKind.FRAME_TRANSFER,
                                   EHActionKind.FRAME_SLOT_CALL_CLEANUP):
                    if action is not None:
                        raise ValueError("COFF EH action has ambiguous bounded decoding")
                    action, consumed = parsed, length
        if action is None or consumed == 0 or action.start != cursor:
            raise ValueError("COFF EH child has an unsupported or incomplete cleanup action")
        if action.end - action.start > _MAX_ACTION_SIZE:
            raise ValueError("COFF EH action exceeds its bounded byte budget")
        starts.append(action.start)
        actions.append(action)
        cursor = action.end
        index += consumed
    if cursor != handler_offset or not actions:
        raise ValueError("COFF EH child has incomplete cleanup bytes before its handler")
    return (handler_offset, tuple(actions), info_offset,
            handler_offset + 6, tuple(starts))


def _object_push_edge(obj: Coff, root: Symbol, child: int, handler_offset: int) -> None:
    _section, code, relocations, _alignment = _relocations(obj, root.section)
    instructions = _decoded(code, 0)
    expected = _registration_push(instructions, 0)
    if expected is None:
        raise ValueError("COFF root lacks one complete EH registration chain")
    edges = []
    for item in relocations:
        target = obj.symbols[int(item.target)]
        if target.section == child:
            if item.kind is not RelocationKind.ABSOLUTE32 or target.value != handler_offset:
                raise ValueError("COFF root has an extra or malformed relocation to its EH child")
            addend, = struct.unpack_from("<I", code, item.offset)
            edges.append((item.offset, addend))
    if edges != [(expected, 0)]:
        raise ValueError("COFF root lacks one exact relocated PUSH to child handler offset")


def _object_child_and_info(obj: Coff, root: Symbol, child: int, *,
                           walk: _LocalEHWalk, depth: int):
    child_section, child_code, child_relocs, child_alignment = _relocations(obj, child)
    _record_proof_section(obj, walk, child)
    handler_offset, action_shapes, info_offset, runtime_offset, action_starts = _cold_layout(child_code)
    if (len(child_code) != handler_offset + _HANDLER_STUB_SIZE
            or len(child_code) > _MAX_COLD_SIZE):
        raise ValueError("COFF EH child has an unsupported bounded extent")
    expected_relocations = {info_offset: 6, runtime_offset: 20}
    for action in action_shapes:
        for transfer in action.transfers:
            expected_relocations[transfer.operand_offset] = (
                6 if transfer.role is EHTransferRole.CALLBACK else 20)
    actual_relocations = [(item.offset, 6 if item.kind is RelocationKind.ABSOLUTE32 else 20)
                          for item in child_relocs]
    if (len(actual_relocations) != len(expected_relocations)
            or set(actual_relocations) != set(expected_relocations.items())):
        raise ValueError("COFF EH child has missing or unaccounted relocations")
    compiled_actions = []
    for action_index, action in enumerate(action_shapes):
        resolved = action
        for transfer in action.transfers:
            raw_addend, = struct.unpack_from("<I", child_code, transfer.operand_offset)
            if transfer.role is EHTransferRole.CALLBACK:
                symbol = _reloc_at(obj, child, transfer.operand_offset,
                                   RelocationKind.ABSOLUTE32)
                if raw_addend != 0 or symbol.storage != 2 or not symbol.type & 0x20:
                    raise ValueError("COFF vector callback lacks a zero-addend function relocation")
                if symbol.section == 0:
                    if symbol.value != 0 or not symbol.name.startswith("??1"):
                        raise ValueError("COFF vector callback is not an external destructor")
                elif symbol.name.startswith("??1"):
                    _validate_local_complete_destructor(
                        obj, symbol, root.section, child, walk, depth)
                else:
                    raise ValueError("COFF vector callback is not a local ordinary destructor")
                resolved = resolve_transfer(
                    resolved, role=transfer.role, relocation_kind="DIR32",
                    target=symbol.name, addend=raw_addend)
            elif transfer.role is EHTransferRole.HELPER:
                symbol = _reloc_at(obj, child, transfer.operand_offset,
                                   RelocationKind.RELATIVE32)
                if (raw_addend != 0 or symbol.section != 0 or symbol.value != 0
                        or symbol.storage != 2 or not symbol.type & 0x20
                        or symbol.name != "??_M@YGXPAXIHP6EX0@Z@Z"):
                    raise ValueError("COFF vector cleanup does not call the supported runtime helper")
                resolved = resolve_transfer(
                    resolved, role=transfer.role, relocation_kind="REL32",
                    target=symbol.name, addend=raw_addend)
            elif action.kind is EHActionKind.FRAME_SLOT_CALL_CLEANUP:
                symbol = _reloc_at(obj, child, transfer.operand_offset,
                                   RelocationKind.RELATIVE32)
                named_symbols = [candidate for candidate in obj.symbols.values()
                                 if candidate.name == symbol.name]
                if (transfer.role is not EHTransferRole.CLEANUP
                        or raw_addend != 0 or symbol.section != 0
                        or symbol.value != 0 or symbol.storage != 2
                        or not symbol.name or not symbol.type & 0x20
                        or symbol.index in obj.auxiliary or len(named_symbols) != 1):
                    raise ValueError(
                        "COFF frame-local cleanup lacks one unambiguous external function relocation")
                resolved = resolve_transfer(
                    resolved, role=transfer.role, relocation_kind="REL32",
                    target=symbol.name, addend=raw_addend)
            else:
                symbol = _reloc_at(obj, child, transfer.operand_offset,
                                   RelocationKind.RELATIVE32)
                if symbol.storage != 2 or not symbol.type & 0x20:
                    raise ValueError("COFF cleanup stub does not transfer to a function symbol")
                if symbol.section == 0:
                    if symbol.value != 0 or not symbol.name.startswith("??1"):
                        raise ValueError("COFF cleanup does not transfer to an undefined external destructor")
                elif symbol.name.startswith("??1"):
                    _validate_local_complete_destructor(
                        obj, symbol, root.section, child, walk, depth)
                elif action_index > 0:
                    _validate_local_destructor(obj, symbol, root.section, child,
                                               forbidden_sections=frozenset(walk.active),
                                               walk=walk)
                else:
                    raise ValueError("COFF first local cleanup is not a complete ??1 destructor")
                if raw_addend != 0:
                    raise ValueError("COFF cleanup branch has a nonzero addend")
                resolved = resolve_transfer(
                    resolved, role=transfer.role, relocation_kind="REL32",
                    target=symbol.name, addend=raw_addend)
        compiled_actions.append(resolved)
    func_info_symbol = _reloc_at(obj, child, info_offset, RelocationKind.ABSOLUTE32)
    if struct.unpack_from("<I", child_code, info_offset)[0] != 0:
        raise ValueError("COFF handler FuncInfo pointer has a nonzero addend")
    runtime = _reloc_at(obj, child, runtime_offset, RelocationKind.RELATIVE32)
    if (runtime.section != 0 or runtime.storage != 2
            or runtime.name != _EH_RUNTIME):
        raise ValueError("COFF handler stub does not transfer to the C++ EH runtime")
    if struct.unpack_from("<I", child_code, runtime_offset)[0] != 0:
        raise ValueError("COFF cold branch has a nonzero runtime addend")

    data_section_number = func_info_symbol.section
    if (not 1 <= data_section_number <= len(obj.sections)
            or func_info_symbol.value != 0
            or not _associated_root(obj, data_section_number, root.section)
            or obj.associations.get(data_section_number) != root.section):
        raise ValueError("COFF FuncInfo is not an associated root-owned data section")
    data_section = obj.sections[data_section_number - 1]
    data_flags = data_section.flags
    action_count = len(action_shapes)
    if (len(data_section.data) != _FUNCINFO_HEADER_SIZE + 8 * action_count
            or data_section.flags & (_CODE | _EXECUTE | _WRITE)
            or not data_flags & _INITIALIZED_DATA or not data_flags & _READ
            or not data_flags & _COMDAT
            or func_info_symbol.storage != 3 or func_info_symbol.type != 0):
        raise ValueError("COFF FuncInfo data section has unsupported bounds or permissions")

    # There must be one EH data child. Associated compiler debug records are
    # metadata, not generated EH data; any other associated data is ambiguous.
    data_children = []
    unsupported_children = []
    for number, parent in obj.associations.items():
        if not _associated_root(obj, number, root.section):
            continue
        section = obj.sections[number - 1]
        if section.name.startswith(".debug$"):
            if section.flags & (_CODE | _EXECUTE):
                unsupported_children.append(number)
            continue
        if section.flags & _CODE:
            if number != child:
                unsupported_children.append(number)
        elif section.flags & (_INITIALIZED_DATA | 0x00000080):
            data_children.append(number)
        else:
            unsupported_children.append(number)
    if data_children != [data_section_number] or unsupported_children:
        raise ValueError("COFF root has missing, extra, or ambiguous associated EH data")
    _record_proof_section(obj, walk, data_section_number)

    fields = struct.unpack("<8I", data_section.data[:_FUNCINFO_HEADER_SIZE])
    magic, max_state, raw_unwind, try_count, try_map, ip_count, ip_map, es_types = fields
    if (magic != _FUNCINFO_MAGIC or max_state != action_count or raw_unwind != 0
            or any((try_count, try_map, ip_count, ip_map, es_types))):
        raise ValueError("COFF FuncInfo does not have a supported bounded cleanup layout")
    unwind_symbol = _reloc_at(obj, data_section_number, 8, RelocationKind.ABSOLUTE32)
    map_entries = struct.unpack_from("<" + "I" * (2 * action_count),
                                     data_section.data, _FUNCINFO_HEADER_SIZE)
    compiled_predecessors = []
    for index in range(action_count):
        predecessor, action_addend = map_entries[index * 2:index * 2 + 2]
        if ((predecessor != 0xFFFFFFFF and predecessor >= index)
                or action_addend != 0):
            raise ValueError("COFF unwind predecessor is not a sentinel or earlier state")
        compiled_predecessors.append(predecessor)
    cleanup_symbols = [
        _reloc_at(obj, data_section_number, _FUNCINFO_HEADER_SIZE + 4 + 8 * index,
                  RelocationKind.ABSOLUTE32)
        for index in range(action_count)
    ]
    all_data_relocs = data_section.relocations
    expected_data_relocs = {(8, 6)} | {
        (_FUNCINFO_HEADER_SIZE + 4 + 8 * index, 6)
        for index in range(action_count)
    }
    if {(offset, kind) for offset, _index, kind in all_data_relocs} != expected_data_relocs:
        raise ValueError("COFF FuncInfo contains missing or unsupported relocations")
    if (unwind_symbol.section != data_section_number
            or unwind_symbol.value != _FUNCINFO_HEADER_SIZE
            or any(symbol.section != child or symbol.value != action_starts[index]
                   for index, symbol in enumerate(cleanup_symbols))):
        raise ValueError("COFF unwind map does not own exactly the child cleanup entry")
    return (child_section, child_code, child_relocs, child_alignment,
            data_section_number, runtime.name, handler_offset,
            tuple(compiled_actions), action_starts, tuple(compiled_predecessors))


def bind_eh_regions(raw_object: bytes, emitted_symbol: str, original_image: PeImage,
                    spans: tuple[tuple[int, int], ...]) -> tuple[EHRegionPair, EHRegionPair]:
    """Bind the supported original primary/cold spans to rooted COFF sections.

    Unsupported or ambiguous ownership raises ``ValueError``. The returned
    code and relocation records are complete physical section subjects, not a
    comparison verdict, linker-selection proof, or reference-target proof.
    """
    if type(raw_object) is not bytes or not raw_object:
        raise ValueError("EH binding requires nonempty COFF object bytes")
    if type(emitted_symbol) is not str or not emitted_symbol:
        raise ValueError("EH binding requires an exact emitted root symbol")
    if type(original_image) is not PeImage:
        raise ValueError("EH binding requires a pinned parsed PE image")
    primary, cold = _checked_spans(spans)
    (original_handler_offset, original_handler, original_func_info,
     original_cleanup, original_cleanup_targets, original_runtime,
     original_action_starts, original_predecessors, original_actions) = _original_eh(
         original_image, primary, cold)

    obj = Coff(raw_object)
    root = obj.function(emitted_symbol)
    if root.section in obj.associations:
        raise ValueError("COFF EH root is itself an associated child")
    root_flags = obj.sections[root.section - 1].flags
    if not root_flags & _CODE or not root_flags & _EXECUTE or root_flags & _WRITE:
        raise ValueError("COFF EH root is not bounded executable code")
    code_sections = obj.code_sections(root)
    if len(code_sections) != 2 or code_sections[0] != root.section:
        raise ValueError("COFF EH root must own exactly one associated code child")
    child = code_sections[1]
    if obj.associations.get(child) != root.section:
        raise ValueError("COFF EH code child must be directly associated with its root")
    child_flags = obj.sections[child - 1].flags
    if not child_flags & _CODE or not child_flags & _EXECUTE or child_flags & _WRITE:
        raise ValueError("COFF EH child is not bounded executable code")

    _child_section, child_code, _child_relocs, _child_alignment = _relocations(obj, child)
    handler_offset, _action_shapes, _info_offset, _runtime_offset, _action_starts = _cold_layout(child_code)
    _object_push_edge(obj, root, child, handler_offset)
    walk = _LocalEHWalk(active={root.section, child}, complete=set(), obj=obj)
    (_child_section, child_code, child_relocs, child_alignment,
     info_section, runtime_symbol, object_handler_offset,
     compiled_actions, compiled_action_starts,
     compiled_predecessors) = _object_child_and_info(
         obj, root, child, walk=walk, depth=0)
    if handler_offset != object_handler_offset:
        raise ValueError("COFF EH handler offset changed during ownership validation")

    root_section, root_code, root_relocs, root_alignment = _relocations(obj, root.section)
    evidence = EHOwnershipEvidence(
        root_section=root.section,
        child_section=child,
        func_info_section=info_section,
        handler_offset=handler_offset,
        original_handler_offset=original_handler_offset,
        cleanup_offset=0,
        original_handler=original_handler,
        original_func_info=original_func_info,
        original_cleanup=original_cleanup,
        cleanup_offsets=compiled_action_starts,
        original_cleanups=original_action_starts,
        original_predecessors=original_predecessors,
        compiled_predecessors=compiled_predecessors,
        original_cleanup_targets=original_cleanup_targets,
        original_runtime_target=original_runtime,
        compiled_runtime_symbol=runtime_symbol,
        original_actions=original_actions,
        compiled_actions=compiled_actions,
    )
    return (
        EHRegionPair(EHRegionRole.ROOT, primary, root.section, root_code,
                     root_relocs, root_alignment, evidence),
        EHRegionPair(EHRegionRole.COLD, cold, child,
                     child_code, child_relocs, child_alignment, evidence),
    )


def eh_funcinfo_reference_target(raw_object: bytes, emitted_symbol: str,
                                 original_image: PeImage,
                                 spans: tuple[tuple[int, int], ...],
                                 resolved_targets: Mapping[str, int]) -> tuple[int, int]:
    """Prove one exact COFF local FuncInfo symbol against this EH owner.

    This is deliberately narrower than ``bind_eh_regions``: it additionally
    requires matching action topology/shape and independently resolved
    transfer targets. It returns the exact symbol-table index used by this
    owner's handler relocation and its pinned FuncInfo VA. It does not bind
    other local symbols in the section or infer linker selection.
    """
    root_region, cold_region = bind_eh_regions(
        raw_object, emitted_symbol, original_image, spans)
    if not isinstance(resolved_targets, Mapping):
        raise ValueError("EH reference proof requires current-object resolved targets")
    ownership = root_region.ownership
    if ownership.original_predecessors != ownership.compiled_predecessors:
        raise ValueError("EH FuncInfo reference has changed predecessor topology")
    if len(ownership.original_actions) != len(ownership.compiled_actions):
        raise ValueError("EH FuncInfo reference has changed action count")

    obj = Coff(raw_object)

    def target_for_relocation(section: int, offset: int, kind: int) -> tuple[int, Symbol]:
        found = [(index, coff_kind) for at, index, coff_kind
                 in obj.sections[section - 1].relocations if at == offset]
        if len(found) != 1 or found[0][1] != kind:
            raise ValueError("EH target lacks its exact unique relocation")
        index, _coff_kind = found[0]
        return index, obj.symbols[index]

    for index, (original_action, compiled_action) in enumerate(zip(
            ownership.original_actions, ownership.compiled_actions, strict=True)):
        if original_action.kind not in (EHActionKind.ARRAY_DESTRUCTOR,
                                        EHActionKind.FRAME_TRANSFER,
                                        EHActionKind.FRAME_SLOT_CALL_CLEANUP):
            raise ValueError("EH FuncInfo reference has an unsupported action kind")
        difference = eh_action_semantic_difference(
            original_action, compiled_action, index)
        if difference is not None:
            raise ValueError(difference[1])
        if len(original_action.transfers) != len(compiled_action.transfers):
            raise ValueError(f"EH FuncInfo reference has changed transfer count at {index}")
        for original_transfer, compiled_transfer in zip(
                original_action.transfers, compiled_action.transfers, strict=True):
            if (original_transfer.role is not compiled_transfer.role
                    or type(original_transfer.target) is not int
                    or compiled_transfer.addend != 0
                    or compiled_transfer.relocation_kind not in ("DIR32", "REL32")):
                raise ValueError("EH FuncInfo reference has an unresolved or changed transfer")
            coff_kind = 6 if compiled_transfer.relocation_kind == "DIR32" else 20
            symbol_index, _symbol = target_for_relocation(
                ownership.child_section, compiled_transfer.operand_offset, coff_kind)
            resolved_target = resolved_targets.get(str(symbol_index))
            if (type(resolved_target) is not int
                    or resolved_target != original_transfer.target):
                raise ValueError("EH cleanup transfer does not resolve to its pinned target")

    # Bind the runtime edge too; a valid-looking local FuncInfo is not enough
    # if the associated handler calls a different runtime entry.
    _handler, _actions, info_offset, runtime_offset, _starts = _cold_layout(cold_region.code)
    runtime_index, _runtime_symbol = target_for_relocation(
        ownership.child_section, runtime_offset, 20)
    runtime_target = resolved_targets.get(str(runtime_index))
    if (type(runtime_target) is not int
            or runtime_target != ownership.original_runtime_target):
        raise ValueError("EH handler runtime does not resolve to its pinned target")

    func_info_index, func_info_symbol = target_for_relocation(
        ownership.child_section, info_offset, 6)
    if (func_info_symbol.section != ownership.func_info_section
            or func_info_symbol.value != 0 or func_info_symbol.storage != 3
            or func_info_symbol.type != 0
            or obj.associations.get(func_info_symbol.section) != ownership.root_section):
        raise ValueError("EH handler target is not this root's exact local FuncInfo definition")
    return func_info_index, ownership.original_func_info
