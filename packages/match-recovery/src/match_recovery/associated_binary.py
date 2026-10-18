"""Strict byte/reference proof for explicitly declared adjacent binary data.

Associated switch tables remain outside function source spans. This adapter
forms a comparison envelope only from those independently recorded spans and
never treats a compiler label or a trailing section extent as their identity.
"""

from __future__ import annotations

import hashlib
import struct
from typing import Mapping, Sequence

from capstone import (
    CS_ARCH_X86, CS_GRP_CALL, CS_GRP_INT, CS_GRP_IRET, CS_GRP_RET,
    CS_MODE_32, Cs, x86,
)

from .associated_source import AssociatedDataBinding, validate_associated
from .binary import AbsoluteConstant, BoundedTarget, CompilerArtifact, Relocation, RelocationKind, compare_bound_code
from .components import relative_path
from .model import Origin, Result, Span
from .pe_coff import Coff, PeImage, Symbol


_ENGINE = Cs(CS_ARCH_X86, CS_MODE_32)
_ENGINE.detail = True
_FILL = frozenset((0x90, 0xCC))
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
                   for group in (CS_GRP_CALL, CS_GRP_RET, CS_GRP_INT, CS_GRP_IRET)))


def _guard_for_dispatch(instructions, dispatch, span: Span):
    """Prove one straight-line unsigned guard reaches this indexed jump.

    The supported envelope is intentionally narrow: `cmp reg,imm`, a direct
    JA/JAE to the default arm, no flag-writing/control-transfer instruction
    before that branch, and no alternate branch into the fallthrough corridor.
    """
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
            if any((other is not branch and _branch_target(other) is not None
                    and guard_corridor_start <= _branch_target(other) <= branch.address)
                   for other in instructions):
                continue
            path = instructions[branch_index + 1:instructions.index(dispatch)]
            if any(_corridor_control_transfer(item) for item in path):
                continue
            # Any other direct edge into the fallthrough corridor would bypass
            # the dominating range check. The accepted branch itself exits it.
            corridor_start = branch.address + branch.size
            if any((other is not branch and _branch_target(other) is not None
                    and corridor_start <= _branch_target(other) <= dispatch.address)
                   for other in instructions):
                continue
            if any(other is not dispatch and other.group(x86.X86_GRP_JUMP)
                   and _branch_target(other) is None for other in instructions):
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


def _prove_dispatch_extent(instructions, body: Span, image: PeImage,
                           table_binding: AssociatedDataBinding,
                           table_instruction, table_operand,
                           selector_binding: AssociatedDataBinding | None,
                           selector_instruction, selector_operand) -> dict:
    """Derive table/map lengths from a narrow guard and preserved index flow."""
    index_register = table_operand.mem.index
    if not index_register:
        raise ValueError("switch target dispatch lacks an index register")
    index_family = _family(table_instruction, index_register)
    compare, branch, guard_family, count, default_target = _guard_for_dispatch(
        instructions, table_instruction, body)
    table_count = (table_binding.span.end - table_binding.span.start) // 4
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
                            targets: Mapping[str, int | AbsoluteConstant | BoundedTarget | CompilerArtifact]) -> dict:
    """Compare one root code COMDAT and explicitly declared adjacent data.

    `span` remains the independently mapped function body. The comparison
    envelope ends at the last declared data span; it does not revise source
    function identity or establish final-link selection/runtime behavior.
    Unsupported evidence raises ValueError so callers preserve UNKNOWN.
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
    if section_number in obj.associations or any(parent == section_number for parent in obj.associations.values()):
        raise ValueError("associated proof refuses associative COMDAT children, including EH/data companions")
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
    if len(selector_rows) > 1 or len(table_rows) != 1:
        raise ValueError("associated dispatch requires one target table and at most one selector map")
    table_binding = table_rows[0][0]
    selector_binding = selector_rows[0][0] if selector_rows else None
    if table_binding.span.start < span.end or table_binding.span.start % 4:
        raise ValueError("switch target table must begin after the body at a DWORD-aligned address")
    if selector_binding is not None and selector_binding.span.start != table_binding.span.end:
        raise ValueError("switch selector map must immediately follow its target table")
    envelope = Span(span.start, max([span.end, *(item.end for item in ordered)]))
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
        switch_alignment = (left == span.end and right == table_binding.span.start
                            and original_gap == b"\x8b\xff" and len(original_gap) == 2
                            and span.end % 4 == 2 and table_binding.span.start % 4 == 0)
        ordinary_alignment = (left == span.end and right == table_binding.span.start
                              and len(original_gap) <= 3
                              and (span.end + len(original_gap)) % 4 == 0
                              and table_binding.span.start % 4 == 0)
        if (not original_gap or (any(byte not in _FILL for byte in original_gap)
                                 and not switch_alignment)
                or (not ordinary_alignment and not switch_alignment)
                ):
            raise ValueError("unaccounted code/data gap is not identical NOP/INT3 alignment fill")
        if candidate_gap and (candidate_gap != original_gap
                              or (any(byte not in _FILL for byte in candidate_gap)
                                  and not switch_alignment)):
            # The strict whole-envelope comparison reports this candidate
            # mutation as a byte mismatch; do not reinterpret it as a gap.
            pass
        if image.absolute_sites(gap):
            raise ValueError("alignment gap contains an original HIGHLOW relocation")
        if any(relocation.offset < candidate_offset + len(candidate_gap)
               and candidate_offset < relocation.offset + 4 for relocation in relocations):
            raise ValueError("alignment gap contains a candidate COFF relocation")

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
    dispatch_witness = _prove_dispatch_extent(
        body_instructions, span, image,
        table_rows[0][0], table_rows[0][1], table_rows[0][2],
        *(selector_rows[0] if selector_rows else (None, None, None)))
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
            except ValueError:
                # A malformed candidate anchor cannot produce a false pass:
                # original HIGHLOW sites and all candidate relocations are
                # still compared across the complete envelope below.
                pass
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
                _symbol_target(obj, section_number, candidate, cell_offset)
        else:
            data = image.read(binding.span)
            if image.absolute_sites(binding.span):
                raise ValueError("switch selector bytes must have no original HIGHLOW relocations")
            start = binding.span.start - section_base
            end = binding.span.end - section_base
            if any(offset < end and start < offset + 4 for offset, _, _ in section_relocations):
                raise ValueError("switch selector bytes must have no candidate COFF relocations")
            if candidate[start:end] != data:
                # Preserve a measured data mismatch as FAIL through the normal
                # comparator below; this branch only ensures the selector is
                # not interpreted via any relocation.
                pass

    proof = compare_bound_code(envelope, original, candidate, relocations, resolved_targets,
                               original_absolute_sites=image.absolute_sites(envelope),
                               alignment=alignment)
    witnesses = [_binding_witness(binding) for binding, _, _ in normalized]
    return {
        **proof,
        "source_body_span": {"start": span.start, "end": span.end},
        "associated_data": witnesses,
        "comparison_span": {"start": envelope.start, "end": envelope.end},
        "dispatch_extent": dispatch_witness,
        "linked_selection": Result.UNKNOWN,
        "runtime": Result.UNKNOWN,
        "limitations": ["proves bounded object bytes/references only; not final linked selection or runtime behavior"],
    }
