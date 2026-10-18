"""Narrow owner proof for VC6's disjoint x86 C++-EH code regions.

This binds bounded linear VC6 destructor-cleanup chains without assigning
generated symbols by section order. It proves a root -> handler -> FuncInfo ->
unwind cleanup chain in the pinned image and one COFF object. It returns
bounded bytes for a caller to compare; it grants no match verdict.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum
import struct

from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_GRP_JUMP, CS_MODE_32, Cs, x86

from ..binary import Relocation, RelocationKind
from ..model import Span
from ..pe_coff import Coff, PeImage, Symbol


_CODE = 0x00000020
_INITIALIZED_DATA = 0x00000040
_EXECUTE = 0x20000000
_READ = 0x40000000
_WRITE = 0x80000000
_COMDAT = 0x00001000
_FUNCINFO_MAGIC = 0x19930520
_FUNCINFO_HEADER_SIZE = 32
_MAX_CLEANUP_ACTIONS = 16
_MAX_ACTION_SIZE = 14
_HANDLER_STUB_SIZE = 10
_MAX_COLD_SIZE = _MAX_CLEANUP_ACTIONS * _MAX_ACTION_SIZE + _HANDLER_STUB_SIZE
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
    original_cleanup_targets: tuple[int, ...]
    original_runtime_target: int
    compiled_runtime_symbol: str


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
        raise ValueError("cold EH region exceeds the bounded linear VC6 extent")
    return primary, cold


def _decoded(code: bytes, base: int):
    instructions = list(_ENGINE.disasm(code, base))
    if sum(item.size for item in instructions) != len(code):
        raise ValueError("EH instruction region contains undecoded bytes")
    return instructions


def _registration_push(instructions, target: int) -> int | None:
    """Return the handler immediate offset only for the complete EH chain."""
    matches = []
    for index in range(max(0, len(instructions) - 4)):
        push_minus_one, push_handler, read_fs, push_eax, write_fs = instructions[index:index + 5]
        if (push_minus_one.mnemonic != "push" or len(push_minus_one.operands) != 1
                or push_minus_one.operands[0].type != x86.X86_OP_IMM
                or push_minus_one.operands[0].imm != -1
                or push_handler.mnemonic != "push" or len(push_handler.operands) != 1
                or push_handler.operands[0].type != x86.X86_OP_IMM
                or push_handler.operands[0].imm != target
                or push_handler.encoding.imm_size != 4
                or read_fs.mnemonic != "mov" or len(read_fs.operands) != 2
                or read_fs.operands[0].type != x86.X86_OP_REG
                or read_fs.operands[0].reg != x86.X86_REG_EAX
                or read_fs.operands[1].type != x86.X86_OP_MEM
                or read_fs.operands[1].mem.segment != x86.X86_REG_FS
                or read_fs.operands[1].mem.base != 0
                or read_fs.operands[1].mem.index != 0
                or read_fs.operands[1].mem.disp != 0
                or push_eax.mnemonic != "push" or len(push_eax.operands) != 1
                or push_eax.operands[0].type != x86.X86_OP_REG
                or push_eax.operands[0].reg != x86.X86_REG_EAX
                or write_fs.mnemonic != "mov" or len(write_fs.operands) != 2
                or write_fs.operands[0].type != x86.X86_OP_MEM
                or write_fs.operands[0].mem.segment != x86.X86_REG_FS
                or write_fs.operands[0].mem.base != 0
                or write_fs.operands[0].mem.index != 0
                or write_fs.operands[0].mem.disp != 0
                or write_fs.operands[1].type != x86.X86_OP_REG
                or write_fs.operands[1].reg != x86.X86_REG_ESP):
            continue
        matches.append(push_handler.address + push_handler.encoding.imm_offset)
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


def _frame_cleanup_jump(instructions) -> bool:
    """Recognize one complete LEA frame-address setup and long cleanup jump."""
    return (len(instructions) == 2
            and instructions[0].mnemonic == "lea"
            and len(instructions[0].operands) == 2
            and instructions[0].operands[0].type == x86.X86_OP_REG
            and instructions[0].operands[0].reg == x86.X86_REG_ECX
            and instructions[0].operands[0].size == 4
            and instructions[0].operands[1].type == x86.X86_OP_MEM
            and instructions[0].operands[1].mem.base == x86.X86_REG_EBP
            and instructions[0].operands[1].mem.index == 0
            and instructions[0].operands[1].mem.segment == 0
            and instructions[0].operands[1].size == 4
            and instructions[0].size in (3, 6)
            and instructions[0].bytes[:2] == (b"\x8d\x4d" if instructions[0].size == 3
                                               else b"\x8d\x8d")
            and instructions[0].encoding.disp_size == instructions[0].size - 2
            and instructions[1].mnemonic == "jmp"
            and instructions[1].size == 5
            and instructions[1].bytes[0] == 0xE9
            and len(instructions[1].operands) == 1
            and instructions[1].operands[0].type == x86.X86_OP_IMM
            and instructions[1].encoding.imm_offset == 1
            and instructions[1].encoding.imm_size == 4)


def _frame_load(instruction) -> bool:
    return (instruction.mnemonic == "mov" and len(instruction.operands) == 2
            and instruction.operands[0].type == x86.X86_OP_REG
            and instruction.operands[0].reg == x86.X86_REG_ECX
            and instruction.operands[0].size == 4
            and instruction.operands[1].type == x86.X86_OP_MEM
            and instruction.operands[1].size == 4
            and instruction.operands[1].mem.base == x86.X86_REG_EBP
            and instruction.operands[1].mem.index == 0
            and instruction.operands[1].mem.segment == 0
            and instruction.size == 3
            and instruction.bytes[:2] == b"\x8b\x4d"
            and instruction.encoding.disp_size == 1)


def _frame_add(instruction) -> bool:
    return (instruction.mnemonic == "add" and len(instruction.operands) == 2
            and instruction.operands[0].type == x86.X86_OP_REG
            and instruction.operands[0].reg == x86.X86_REG_ECX
            and instruction.operands[1].type == x86.X86_OP_IMM
            and ((instruction.size == 3 and instruction.bytes[:2] == b"\x83\xc1"
                  and instruction.encoding.imm_size == 1)
                 or (instruction.size == 6 and instruction.bytes[:2] == b"\x81\xc1"
                     and instruction.encoding.imm_size == 4)))


def _cleanup_action(instructions) -> int:
    """Return a cleanup JMP operand offset for one complete VC6 action."""
    if _frame_cleanup_jump(instructions):
        return instructions[1].address + instructions[1].encoding.imm_offset
    if (len(instructions) == 2 and _frame_load(instructions[0])
            and instructions[1].mnemonic == "jmp"
            and instructions[1].size == 5 and instructions[1].bytes[0] == 0xE9
            and len(instructions[1].operands) == 1
            and instructions[1].operands[0].type == x86.X86_OP_IMM
            and instructions[1].encoding.imm_offset == 1
            and instructions[1].encoding.imm_size == 4):
        return instructions[1].address + instructions[1].encoding.imm_offset
    if (len(instructions) == 3 and _frame_load(instructions[0])
            and _frame_add(instructions[1])
            and instructions[2].mnemonic == "jmp"
            and instructions[2].size == 5 and instructions[2].bytes[0] == 0xE9
            and len(instructions[2].operands) == 1
            and instructions[2].operands[0].type == x86.X86_OP_IMM
            and instructions[2].encoding.imm_offset == 1
            and instructions[2].encoding.imm_size == 4):
        return instructions[2].address + instructions[2].encoding.imm_offset
    raise ValueError("unwind map does not bound a supported complete frame cleanup action")


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


def _linear_map(data: bytes, count: int, *, base: int,
                first_action: int) -> tuple[int, ...]:
    """Validate a bounded linear unwind map and return its action targets."""
    if not 1 <= count <= _MAX_CLEANUP_ACTIONS:
        raise ValueError("EH cleanup action count exceeds the bounded resource cap")
    if len(data) != count * 8:
        raise ValueError("EH unwind map has an incomplete action table")
    entries = struct.unpack("<" + "I" * (count * 2), data)
    actions = []
    for index in range(count):
        predecessor, action = entries[index * 2:index * 2 + 2]
        expected_state = 0xFFFFFFFF if index == 0 else index - 1
        if predecessor != expected_state:
            raise ValueError("EH unwind states are not the exact linear predecessor chain")
        if (action < base or (index == 0 and action != first_action)
                or (actions and action <= actions[-1])):
            raise ValueError("EH unwind actions are not strictly ordered from the first cleanup")
        actions.append(action)
    return tuple(actions)


def _action_targets(code: bytes, base: int, starts: tuple[int, ...],
                    handler_offset: int) -> tuple[tuple[int, ...], tuple[int, ...]]:
    if not starts or len(starts) > _MAX_CLEANUP_ACTIONS or starts[0] != 0:
        raise ValueError("EH cleanup action starts exceed bounds or omit the first action")
    if any(start < 0 or start >= handler_offset for start in starts):
        raise ValueError("EH cleanup action start lies outside the cold action region")
    if any(left >= right for left, right in zip(starts, starts[1:])):
        raise ValueError("EH cleanup action starts are duplicate or unordered")
    boundaries = (*starts, handler_offset)
    targets, operand_offsets = [], []
    for start, end in zip(boundaries, boundaries[1:]):
        instructions = _decoded(code[start:end], base + start)
        operand = _cleanup_action(instructions)
        jump = instructions[-1]
        target = jump.operands[0].imm & 0xFFFFFFFF
        targets.append(target)
        operand_offsets.append(operand)
    return tuple(targets), tuple(operand_offsets)


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
    original_cleanups = _linear_map(
        map_data, max_state, base=cold.start, first_action=cold.start)
    if any(action >= handler for action in original_cleanups):
        raise ValueError("original unwind action starts at or beyond its handler")
    original_action_starts = original_cleanups
    cleanup_targets, _cleanup_operands = _action_targets(
        cold_code, cold.start,
        tuple(action - cold.start for action in original_action_starts),
        handler_offset)
    for target in cleanup_targets:
        _exec_target(image, target, outside=(primary, cold))
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
    if image.absolute_sites(cold) != frozenset({handler_offset + info_offset - handler_offset}):
        raise ValueError("original cold region has unexpected HIGHLOW sites outside the handler pointer")
    return (handler_offset, handler, func_info, cold.start, tuple(cleanup_targets),
            runtime_target, original_action_starts)


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


def _validate_local_destructor(obj: Coff, symbol: Symbol, root: int, child: int) -> None:
    """Admit a bounded independent local ??_D function, without semantic claims."""
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
        if obj.symbols[target_index].section in (root, child):
            raise ValueError("COFF local cleanup target has an ambiguous root/EH relocation")
    _raw, code, _helper_relocations, _alignment = _relocations(obj, section_number)
    if len(code) != obj.section_sizes[section_number - 1]:
        raise ValueError("COFF local cleanup target is not fully raw-backed")
    _decoded(code, 0)


def _cold_layout(code: bytes) -> tuple[int, tuple[int, ...], int, int, tuple[int, ...]]:
    """Parse complete adjacent cleanup actions followed by one handler stub."""
    if not _HANDLER_STUB_SIZE < len(code) <= _MAX_COLD_SIZE:
        raise ValueError("COFF EH child exceeds the bounded linear cold extent")
    handler_offset, info_offset, runtime_target = _handler_stub(code, 0)
    instructions = _decoded(code[:handler_offset], 0)
    starts, destructor_offsets = [], []
    cursor = 0
    action = []
    for instruction in instructions:
        if instruction.address != cursor:
            raise ValueError("COFF EH child has a gap before its handler")
        action.append(instruction)
        cursor += instruction.size
        if instruction.mnemonic == "jmp" and instruction.size == 5 and instruction.bytes[0] == 0xE9:
            if len(starts) >= _MAX_CLEANUP_ACTIONS:
                raise ValueError("COFF EH child exceeds the bounded cleanup action cap")
            starts.append(action[0].address)
            destructor_offsets.append(_cleanup_action(action))
            action = []
    if cursor != handler_offset or action or not starts:
        raise ValueError("COFF EH child has incomplete cleanup bytes before its handler")
    return (handler_offset, tuple(destructor_offsets), info_offset,
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


def _object_child_and_info(obj: Coff, root: Symbol, child: int):
    child_section, child_code, child_relocs, child_alignment = _relocations(obj, child)
    handler_offset, destructor_offsets, info_offset, runtime_offset, action_starts = _cold_layout(child_code)
    if (len(child_code) != handler_offset + _HANDLER_STUB_SIZE
            or len(child_code) > _MAX_COLD_SIZE):
        raise ValueError("COFF EH child has an unsupported bounded extent")
    expected_relocations = {*destructor_offsets, info_offset, runtime_offset}
    if {item.offset for item in child_relocs} != expected_relocations:
        raise ValueError("COFF EH child has missing or unaccounted relocations")
    destructors = [_reloc_at(obj, child, offset, RelocationKind.RELATIVE32)
                   for offset in destructor_offsets]
    for index, destructor in enumerate(destructors):
        if destructor.storage != 2 or not destructor.type & 0x20:
            raise ValueError("COFF cleanup stub does not transfer to a function symbol")
        if index == 0 and (destructor.section != 0 or destructor.value != 0
                           or not destructor.name.startswith("??1")):
            raise ValueError("COFF first cleanup does not transfer to its external destructor")
        if index > 0:
            if destructor.section == 0:
                if (destructor.value != 0 or not destructor.name.startswith("??1")):
                    raise ValueError("COFF cleanup does not transfer to an undefined external destructor")
            else:
                _validate_local_destructor(obj, destructor, root.section, child)
    func_info_symbol = _reloc_at(obj, child, info_offset, RelocationKind.ABSOLUTE32)
    if struct.unpack_from("<I", child_code, info_offset)[0] != 0:
        raise ValueError("COFF handler FuncInfo pointer has a nonzero addend")
    runtime = _reloc_at(obj, child, runtime_offset, RelocationKind.RELATIVE32)
    if (runtime.section != 0 or runtime.storage != 2
            or runtime.name != _EH_RUNTIME):
        raise ValueError("COFF handler stub does not transfer to the C++ EH runtime")
    if (any(struct.unpack_from("<I", child_code, offset)[0] != 0
            for offset in destructor_offsets)
            or struct.unpack_from("<I", child_code, runtime_offset)[0] != 0):
        raise ValueError("COFF cold branch has a nonzero destructor/runtime addend")

    data_section_number = func_info_symbol.section
    if (not 1 <= data_section_number <= len(obj.sections)
            or func_info_symbol.value != 0
            or not _associated_root(obj, data_section_number, root.section)
            or obj.associations.get(data_section_number) != root.section):
        raise ValueError("COFF FuncInfo is not an associated root-owned data section")
    data_section = obj.sections[data_section_number - 1]
    data_flags = data_section.flags
    action_count = len(destructor_offsets)
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

    fields = struct.unpack("<8I", data_section.data[:_FUNCINFO_HEADER_SIZE])
    magic, max_state, raw_unwind, try_count, try_map, ip_count, ip_map, es_types = fields
    if (magic != _FUNCINFO_MAGIC or max_state != action_count or raw_unwind != 0
            or any((try_count, try_map, ip_count, ip_map, es_types))):
        raise ValueError("COFF FuncInfo does not have a supported bounded cleanup layout")
    unwind_symbol = _reloc_at(obj, data_section_number, 8, RelocationKind.ABSOLUTE32)
    map_entries = struct.unpack_from("<" + "I" * (2 * action_count),
                                     data_section.data, _FUNCINFO_HEADER_SIZE)
    for index in range(action_count):
        predecessor, action_addend = map_entries[index * 2:index * 2 + 2]
        if predecessor != (0xFFFFFFFF if index == 0 else index - 1) or action_addend != 0:
            raise ValueError("COFF unwind map is not the exact linear predecessor chain")
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
            data_section_number, runtime.name, handler_offset, destructor_offsets,
            action_starts)


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
     original_action_starts) = _original_eh(
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
    handler_offset, _destructor_offset, _info_offset, _runtime_offset, _action_starts = _cold_layout(child_code)
    _object_push_edge(obj, root, child, handler_offset)
    (_child_section, child_code, child_relocs, child_alignment,
     info_section, runtime_symbol, object_handler_offset,
     destructor_offsets, compiled_action_starts) = _object_child_and_info(obj, root, child)
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
        original_cleanup_targets=original_cleanup_targets,
        original_runtime_target=original_runtime,
        compiled_runtime_symbol=runtime_symbol,
    )
    return (
        EHRegionPair(EHRegionRole.ROOT, primary, root.section, root_code,
                     root_relocs, root_alignment, evidence),
        EHRegionPair(EHRegionRole.COLD, cold, child,
                     child_code, child_relocs, child_alignment, evidence),
    )
