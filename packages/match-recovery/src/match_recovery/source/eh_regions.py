"""Narrow owner proof for VC6's disjoint x86 C++-EH code regions.

This binds the observed single-cleanup layout without assigning generated
symbols by spelling or section order. It proves a root -> handler -> FuncInfo
-> unwind cleanup chain independently in the pinned image and one COFF object.
It returns bounded bytes for a caller to compare; it grants no match verdict.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum
import struct

from capstone import CS_ARCH_X86, CS_MODE_32, Cs, x86

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
_EH_BUNDLE_SIZE = 40
_COLD_SIZES = {18: 8, 21: 11}
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
    if cold.end - cold.start not in _COLD_SIZES:
        raise ValueError("cold EH region must use one of the two bounded VC6 extents")
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


def _image_registration_edge(image: PeImage, primary: Span, code: bytes,
                             target: int) -> None:
    instructions = _decoded(code, primary.start)
    site = _registration_push(instructions, target)
    if site is None:
        raise ValueError("original root lacks one complete relocated EH registration chain")
    offset = site - primary.start
    sites = image.absolute_sites(primary)
    if offset not in sites:
        raise ValueError("original handler PUSH lacks a PE HIGHLOW relocation")


def _exec_target(image: PeImage, target: int, *, outside: tuple[Span, ...]) -> None:
    if any(span.start <= target < span.end for span in outside):
        raise ValueError("EH branch unexpectedly targets an EH region")
    _image_section(image, Span(target, target + 1), code=True)


def _original_eh(image: PeImage, primary: Span, cold: Span):
    main = image.read(primary)
    cold_code = image.read(cold)
    handler_offset = cold.end - cold.start - 10
    if _COLD_SIZES.get(cold.end - cold.start) != handler_offset:
        raise ValueError("original cold EH region has an unsupported extent")
    handler = cold.start + handler_offset
    _image_section(image, primary, code=True)
    _image_section(image, cold, code=True)
    _image_registration_edge(image, primary, main, handler)

    clean = _decoded(cold_code[:handler_offset], cold.start)
    if (len(clean) != 2 or clean[0].mnemonic != "lea"
            or len(clean[0].operands) != 2
            or clean[0].operands[0].type != x86.X86_OP_REG
            or clean[0].operands[0].reg != x86.X86_REG_ECX
            or clean[0].operands[1].type != x86.X86_OP_MEM
            or clean[0].operands[1].mem.base != x86.X86_REG_EBP
            or clean[0].operands[1].mem.index != 0
            or clean[0].size not in (3, 6)
            or clean[0].encoding.disp_size != clean[0].size - 2
            or clean[1].mnemonic != "jmp"
            or len(clean[1].operands) != 1
            or clean[1].operands[0].type != x86.X86_OP_IMM):
        raise ValueError("original unwind action is not a bounded frame cleanup stub")
    cleanup_target = clean[1].operands[0].imm & 0xFFFFFFFF
    _exec_target(image, cleanup_target, outside=(primary, cold))

    handler_code = cold_code[handler_offset:]
    handler_ins = _decoded(handler_code, handler)
    if (len(handler_ins) != 2 or handler_ins[0].mnemonic != "mov"
            or len(handler_ins[0].operands) != 2
            or handler_ins[0].operands[0].type != x86.X86_OP_REG
            or handler_ins[0].operands[0].reg != x86.X86_REG_EAX
            or handler_ins[0].operands[1].type != x86.X86_OP_IMM
            or handler_ins[1].mnemonic != "jmp"
            or len(handler_ins[1].operands) != 1
            or handler_ins[1].operands[0].type != x86.X86_OP_IMM):
        raise ValueError("original cold handler is not a FuncInfo runtime stub")
    validated_offset, _destructor_offset, _info_offset, _runtime_offset = _cold_layout(cold_code)
    if validated_offset != handler_offset:
        raise ValueError("original cold region does not use a supported cleanup/handler encoding")
    func_info = handler_ins[0].operands[1].imm & 0xFFFFFFFF
    runtime_target = handler_ins[1].operands[0].imm & 0xFFFFFFFF
    _exec_target(image, runtime_target, outside=(primary, cold))

    fi_span = Span(func_info, func_info + _EH_BUNDLE_SIZE)
    _image_section(image, fi_span, code=False)
    fi_data = image.read(fi_span)
    fields = struct.unpack("<10I", fi_data)
    (magic, max_state, unwind_map, try_count, try_map,
     ip_count, ip_map, es_types, to_state, action) = fields
    if (magic != _FUNCINFO_MAGIC or max_state != 1
            or unwind_map != func_info + 32
            or any((try_count, try_map, ip_count, ip_map, es_types))
            or to_state != 0xFFFFFFFF or action != cold.start):
        raise ValueError("original FuncInfo is not the supported one-state cleanup layout")
    if image.absolute_sites(Span(func_info, func_info + 12)) != frozenset({8}):
        raise ValueError("original FuncInfo unwind-map pointer lacks its exact HIGHLOW site")
    if image.absolute_sites(Span(unwind_map, unwind_map + 8)) != frozenset({4}):
        raise ValueError("original unwind cleanup pointer lacks its exact HIGHLOW site")
    if image.absolute_sites(Span(handler, cold.end)) != frozenset({1}):
        raise ValueError("original handler FuncInfo pointer lacks its exact HIGHLOW site")
    if image.absolute_sites(fi_span) != frozenset({8, 36}):
        raise ValueError("original FuncInfo has unexpected HIGHLOW sites in its full 40-byte bundle")
    if image.absolute_sites(cold) != frozenset({handler_offset + 1}):
        raise ValueError("original cold region has unexpected HIGHLOW sites outside the handler pointer")
    return handler_offset, handler, func_info, cold.start, cleanup_target, runtime_target


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


def _cold_layout(code: bytes) -> tuple[int, int, int, int]:
    """Validate the two supported cold encodings and return key byte offsets."""
    instructions = _decoded(code, 0)
    if (len(instructions) != 4 or instructions[0].mnemonic != "lea"
            or len(instructions[0].operands) != 2
            or instructions[0].operands[0].type != x86.X86_OP_REG
            or instructions[0].operands[0].reg != x86.X86_REG_ECX
            or instructions[0].operands[1].type != x86.X86_OP_MEM
            or instructions[0].operands[1].mem.base != x86.X86_REG_EBP
            or instructions[0].operands[1].mem.index != 0
            or instructions[0].size not in (3, 6)
            or instructions[0].encoding.disp_size != instructions[0].size - 2
            or instructions[1].mnemonic != "jmp"
            or instructions[1].size != 5
            or len(instructions[1].operands) != 1
            or instructions[1].operands[0].type != x86.X86_OP_IMM
            or instructions[2].mnemonic != "mov"
            or instructions[2].size != 5
            or len(instructions[2].operands) != 2
            or instructions[2].operands[0].type != x86.X86_OP_REG
            or instructions[2].operands[0].reg != x86.X86_REG_EAX
            or instructions[2].operands[1].type != x86.X86_OP_IMM
            or instructions[3].mnemonic != "jmp"
            or instructions[3].size != 5
            or len(instructions[3].operands) != 1
            or instructions[3].operands[0].type != x86.X86_OP_IMM):
        raise ValueError("EH child does not contain one supported cleanup then handler stub")
    handler_offset = instructions[2].address
    return handler_offset, instructions[1].address + 1, handler_offset + 1, handler_offset + 6


def _object_push_edge(obj: Coff, root: Symbol, child: int, handler_offset: int) -> None:
    _section, code, relocations, _alignment = _relocations(obj, root.section)
    instructions = _decoded(code, 0)
    expected = _registration_push(instructions, 0)
    if expected is None:
        raise ValueError("COFF root lacks one complete EH registration chain")
    edges = []
    for item in relocations:
        if item.kind is not RelocationKind.ABSOLUTE32:
            continue
        target = obj.symbols[int(item.target)]
        if target.section == child and target.value == handler_offset:
            addend, = struct.unpack_from("<I", code, item.offset)
            edges.append((item.offset, addend))
    if edges != [(expected, 0)]:
        raise ValueError("COFF root lacks one exact relocated PUSH to child handler offset")


def _object_child_and_info(obj: Coff, root: Symbol, child: int):
    child_section, child_code, child_relocs, child_alignment = _relocations(obj, child)
    handler_offset, destructor_offset, info_offset, runtime_offset = _cold_layout(child_code)
    if len(child_code) != handler_offset + 10:
        raise ValueError("COFF EH child has an unsupported bounded extent")
    expected_relocations = {destructor_offset, info_offset, runtime_offset}
    if {item.offset for item in child_relocs} != expected_relocations:
        raise ValueError("COFF EH child has missing or unaccounted relocations")
    destructor = _reloc_at(obj, child, destructor_offset, RelocationKind.RELATIVE32)
    if (destructor.section != 0 or destructor.storage != 2
            or not destructor.name.startswith("??1") or not destructor.type & 0x20):
        raise ValueError("COFF cleanup stub does not transfer to an external destructor")
    func_info_symbol = _reloc_at(obj, child, info_offset, RelocationKind.ABSOLUTE32)
    runtime = _reloc_at(obj, child, runtime_offset, RelocationKind.RELATIVE32)
    if (runtime.section != 0 or runtime.storage != 2
            or runtime.name != _EH_RUNTIME):
        raise ValueError("COFF handler stub does not transfer to the C++ EH runtime")
    if (struct.unpack_from("<I", child_code, destructor_offset)[0] != 0
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
    if (len(data_section.data) != _EH_BUNDLE_SIZE
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

    fields = struct.unpack("<10I", data_section.data)
    magic, max_state, raw_unwind, try_count, try_map, ip_count, ip_map, es_types, to_state, raw_action = fields
    if (magic != _FUNCINFO_MAGIC or max_state != 1 or raw_unwind != 0
            or any((try_count, try_map, ip_count, ip_map, es_types))
            or to_state != 0xFFFFFFFF or raw_action != 0):
        raise ValueError("COFF FuncInfo does not have the supported one-state layout")
    unwind_symbol = _reloc_at(obj, data_section_number, 8, RelocationKind.ABSOLUTE32)
    cleanup_symbol = _reloc_at(obj, data_section_number, 36, RelocationKind.ABSOLUTE32)
    all_data_relocs = data_section.relocations
    if {(offset, kind) for offset, _index, kind in all_data_relocs} != {(8, 6), (36, 6)}:
        raise ValueError("COFF FuncInfo contains missing or unsupported relocations")
    if (unwind_symbol.section != data_section_number or unwind_symbol.value != 32
            or cleanup_symbol.section != child or cleanup_symbol.value != 0):
        raise ValueError("COFF unwind map does not own exactly the child cleanup entry")
    return (child_section, child_code, child_relocs, child_alignment,
            data_section_number, runtime.name, handler_offset)


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
     original_cleanup, _cleanup_target, original_runtime) = _original_eh(
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
    handler_offset, _destructor_offset, _info_offset, _runtime_offset = _cold_layout(child_code)
    _object_push_edge(obj, root, child, handler_offset)
    (_child_section, child_code, child_relocs, child_alignment,
     info_section, runtime_symbol, object_handler_offset) = _object_child_and_info(obj, root, child)
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
        original_runtime_target=original_runtime,
        compiled_runtime_symbol=runtime_symbol,
    )
    return (
        EHRegionPair(EHRegionRole.ROOT, primary, root.section, root_code,
                     root_relocs, root_alignment, evidence),
        EHRegionPair(EHRegionRole.COLD, cold, child,
                     child_code, child_relocs, child_alignment, evidence),
    )
