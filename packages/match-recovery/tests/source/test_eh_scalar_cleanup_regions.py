"""Serialized controls for bounded frame-slot call cleanup actions."""

import hashlib
import struct

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Result, Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source.asm import compare_subject, subject_asm
from match_recovery.source.eh_actions import EHActionKind, EHTransferRole
from match_recovery.source.eh_regions import (bind_eh_regions,
                                              eh_funcinfo_reference_target)
from match_recovery.source.reader import read_text


BASE = 0x00400000
ROOT_NAME = "?synthetic_owner@@YAXXZ"
ROOT_SPAN = (BASE + 0x1000, BASE + 0x1016)
COLD_VA = BASE + 0x1100
INFO_VA = BASE + 0x4000
FRAME_TARGET = BASE + 0x1F00
CALL_TARGET = BASE + 0x1F10
RUNTIME_TARGET = BASE + 0x1F20
FRAME_SYMBOL = "??1SyntheticElement@@QAE@XZ"
CALL_SYMBOL = "?synthetic_cleanup@@YAXPAX@Z"
RUNTIME_SYMBOL = "___CxxFrameHandler"


def _scalar(displacement=-16):
    """mov eax,[ebp+disp]; push eax; call rel32; pop ecx; ret."""
    if -128 <= displacement <= 127:
        return bytes((0x8B, 0x45, displacement & 0xFF,
                      0x50, 0xE8, 0, 0, 0, 0, 0x59, 0xC3)), 5
    return (b"\x8b\x85" + struct.pack("<i", displacement)
            + b"\x50\xe8\0\0\0\0\x59\xc3", 8)


def _frame_transfer():
    return b"\x8d\x4d\xf0\xe9\0\0\0\0", 4


def _rel32(code, operand, next_va, target):
    struct.pack_into("<i", code, operand, target - next_va)


def _build(*, action_kinds=("scalar",), displacement=-16,
           object_displacement=None,
           original_damage=None, object_damage=None, call_symbol=CALL_SYMBOL):
    """Make independent PE32 and COFF records with synthetic symbols/targets."""
    image_actions = []
    object_actions = []
    for kind in action_kinds:
        image_code, image_operand = (
            _scalar(displacement) if kind == "scalar" else _frame_transfer())
        compiled_slot = displacement if object_displacement is None else object_displacement
        object_code, object_operand = (
            _scalar(compiled_slot) if kind == "scalar" else _frame_transfer())
        image_actions.append((kind, bytearray(image_code), image_operand))
        object_actions.append((kind, bytearray(object_code), object_operand))

    grammar_damage = {
        "wrong_pop", "wrong_return", "indirect_call", "prefix", "trailing_byte",
        "wrong_frame_base", "indexed_frame", "zero_frame", "misaligned_frame",
    }
    if object_damage in grammar_damage:
        image_row = next(row for row in image_actions if row[0] == "scalar")
        object_row = next(row for row in object_actions if row[0] == "scalar")
        image_code, object_code = image_row[1], object_row[1]
        if object_damage == "wrong_pop":
            image_code[-2] = object_code[-2] = 0x58
        elif object_damage == "wrong_return":
            image_code[-1] = object_code[-1] = 0xC2
            image_code.extend(b"\0\0")
            object_code.extend(b"\0\0")
        elif object_damage == "indirect_call":
            image_code[4:9] = object_code[4:9] = b"\xff\xd0\x90\x90\x90"
        elif object_damage == "prefix":
            image_code.insert(0, 0x66)
            object_code.insert(0, 0x66)
        elif object_damage == "trailing_byte":
            image_code.append(0x90)
            object_code.append(0x90)
        elif object_damage == "wrong_frame_base":
            image_code[1] = object_code[1] = 0x46
        elif object_damage == "indexed_frame":
            for code in (image_code, object_code):
                code[1] = 0x44
                code[2] = 0x8D
                code.insert(3, 0xF0)
        elif object_damage == "zero_frame":
            image_code[2] = object_code[2] = 0
        elif object_damage == "misaligned_frame":
            image_code[2] = object_code[2] = 0xEF

    original_starts, object_starts = [], []
    original_cold, object_cold = bytearray(), bytearray()
    object_action_relocs = []
    for kind, image_row, object_row in zip(action_kinds, image_actions, object_actions):
        _kind, image_code, image_operand = image_row
        _kind, object_code, object_operand = object_row
        original_starts.append(len(original_cold))
        object_starts.append(len(object_cold))
        if kind == "scalar":
            _rel32(image_code, image_operand,
                   COLD_VA + len(original_cold) + image_operand + 4,
                   CALL_TARGET)
            object_action_relocs.append((len(object_cold) + object_operand,
                                         "call_target", 20))
        else:
            _rel32(image_code, image_operand,
                   COLD_VA + len(original_cold) + image_operand + 4,
                   FRAME_TARGET)
            object_action_relocs.append((len(object_cold) + object_operand,
                                         "frame_target", 20))
        original_cold.extend(image_code)
        object_cold.extend(object_code)

    handler_offset = len(original_cold)
    object_handler_offset = len(object_cold)
    assert handler_offset == object_handler_offset
    original_cold.extend(b"\xb8" + struct.pack("<I", INFO_VA) + b"\xe9\0\0\0\0")
    _rel32(original_cold, handler_offset + 6, COLD_VA + handler_offset + 10,
           RUNTIME_TARGET)
    object_cold.extend(b"\xb8\0\0\0\0\xe9\0\0\0\0")
    object_action_relocs.extend([
        (handler_offset + 1, "info", 6),
        (handler_offset + 6, "runtime", 20),
    ])
    if object_damage == "handler_info_addend":
        struct.pack_into("<I", object_cold, handler_offset + 1, 1)
    elif object_damage == "wrong_info_field":
        object_action_relocs[-2] = (handler_offset + 1, "unwind", 6)

    root_code = bytearray.fromhex(
        "6a ff 68 00 00 00 00 64 a1 00 00 00 00 50 "
        "64 89 25 00 00 00 00 c3")
    struct.pack_into("<I", root_code, 3, COLD_VA + handler_offset)
    object_root = bytearray(root_code)
    struct.pack_into("<I", object_root, 3, 0)

    count = len(action_kinds)
    info_size = 32 + count * 8
    original_info = bytearray(info_size)
    object_info = bytearray(info_size)
    struct.pack_into("<8I", original_info, 0, 0x19930520, count,
                     INFO_VA + 32, 0, 0, 0, 0, 0)
    struct.pack_into("<8I", object_info, 0, 0x19930520, count,
                     0, 0, 0, 0, 0, 0)
    if object_damage == "wrong_count":
        struct.pack_into("<I", object_info, 4, count + 1)
    data_relocs = [(8, "unwind", 6)]
    for index, start in enumerate(original_starts):
        predecessor = 0xFFFFFFFF if index == 0 else index - 1
        struct.pack_into("<II", original_info, 32 + 8 * index,
                         predecessor, COLD_VA + start)
        struct.pack_into("<II", object_info, 32 + 8 * index, predecessor, 0)
        data_relocs.append((36 + 8 * index, f"action{index}", 6))

    if original_damage == "bad_map_target":
        struct.pack_into("<I", original_info, 36, COLD_VA + 1)
    elif original_damage == "bad_predecessor" and count > 1:
        struct.pack_into("<I", original_info, 40, 1)
    elif original_damage == "bad_unwind_bounds":
        struct.pack_into("<I", original_info, 8, INFO_VA + info_size + 4)
    if object_damage == "bad_map_addend":
        struct.pack_into("<I", object_info, 36, 1)
    elif object_damage == "bad_predecessor" and count > 1:
        struct.pack_into("<I", object_info, 40, 1)

    if object_damage in {"missing_call", "duplicate_call", "wrong_call_kind",
                         "misaligned_call", "call_addend"}:
        call_reloc = next(row for row in object_action_relocs
                          if row[1] == "call_target")
        if object_damage == "missing_call":
            object_action_relocs.remove(call_reloc)
        elif object_damage == "duplicate_call":
            object_action_relocs.append(call_reloc)
        elif object_damage == "wrong_call_kind":
            object_action_relocs[object_action_relocs.index(call_reloc)] = (
                call_reloc[0], call_reloc[1], 6)
        elif object_damage == "misaligned_call":
            object_action_relocs[object_action_relocs.index(call_reloc)] = (
                call_reloc[0] + 1, call_reloc[1], call_reloc[2])
        elif object_damage == "call_addend":
            struct.pack_into("<I", object_cold, call_reloc[0], 1)

    sections = [
        {"name": ".text", "data": bytes(object_root), "flags": 0x60501020,
         "parent": 0, "selection": 2, "relocs": [(3, "handler", 6)]},
        {"name": ".text$x", "data": bytes(object_cold), "flags": 0x60501020,
         "parent": 1, "selection": 5, "relocs": object_action_relocs},
        {"name": ".xdata$x", "data": bytes(object_info), "flags": 0x40401040,
         "parent": 1, "selection": 5, "relocs": data_relocs},
    ]
    if object_damage == "wrong_association":
        sections[1]["parent"] = 0
    if object_damage == "extra_child_relocation":
        sections[1]["relocs"].append((0, "runtime", 20))
    if object_damage == "missing_info_relocation":
        sections[1]["relocs"] = [row for row in sections[1]["relocs"]
                                  if row[0] != handler_offset + 1]
    if object_damage == "missing_runtime_relocation":
        sections[1]["relocs"] = [row for row in sections[1]["relocs"]
                                  if row[0] != handler_offset + 6]
    if object_damage == "missing_map_relocation":
        sections[2]["relocs"] = [row for row in sections[2]["relocs"]
                                  if row[0] != 36]
    if object_damage == "wrong_info_association":
        sections[2]["parent"] = 0
    if object_damage == "missing_root_handler_relocation":
        sections[0]["relocs"] = []
    if object_damage == "wrong_handler_target":
        sections[0]["relocs"] = [(3, "wrong_handler", 6)]

    symbols, indices, strings = [], {}, bytearray(b"\0\0\0\0")

    def add(key, name, value, section, kind, storage, aux=None):
        raw = name.encode("ascii")
        if len(raw) <= 8:
            encoded = raw.ljust(8, b"\0")
        else:
            offset = len(strings)
            strings.extend(raw + b"\0")
            encoded = b"\0\0\0\0" + struct.pack("<I", offset)
        indices[key] = len(symbols)
        symbols.append(encoded + struct.pack("<IhHBB", value, section, kind,
                                             storage, int(aux is not None)))
        if aux is not None:
            symbols.append(aux)

    for number, section in enumerate(sections, 1):
        aux = struct.pack("<IHHIHB3x", len(section["data"]),
                          len(section["relocs"]), 0, 0,
                          section["parent"], section["selection"])
        add(("section", number), section["name"], 0, number, 0, 3, aux)
    add("root", ROOT_NAME, 0, 1, 0x20, 2)
    add("handler", "$Lhandler", handler_offset, 2, 0, 6)
    if object_damage == "wrong_handler_target":
        add("wrong_handler", "$Lwronghandler", handler_offset - 1, 2, 0, 6)
    for index, start in enumerate(object_starts):
        add(f"action{index}", f"$Laction{index}", start, 2, 0, 6)
    add("info", "$Tinfo", 0, 3, 0, 3)
    add("unwind", "$Tunwind", 32, 3, 0, 6)
    add("frame_target", FRAME_SYMBOL, 0, 0, 0x20, 2)
    add("call_target", call_symbol, 0, 0,
        0 if object_damage == "call_not_function" else 0x20, 2)
    add("runtime", RUNTIME_SYMBOL, 0, 0, 0, 2)
    if object_damage == "call_conflicting_alias":
        add("call_alias", call_symbol, 0, 0, 0x20, 2)

    for number, section in enumerate(sections, 1):
        symbols[indices[("section", number)] + 1] = struct.pack(
            "<IHHIHB3x", len(section["data"]), len(section["relocs"]), 0, 0,
            section["parent"], section["selection"])
    raw_offsets, cursor = [], 20 + 40 * len(sections)
    for section in sections:
        raw_offsets.append(cursor)
        cursor += len(section["data"])
    reloc_offsets, reloc_cursor = [], cursor
    for section in sections:
        reloc_offsets.append(reloc_cursor if section["relocs"] else 0)
        reloc_cursor += len(section["relocs"]) * 10
    symbol_offset = reloc_cursor
    headers = b"".join(struct.pack(
        "<8sIIIIIIHHI", section["name"].encode().ljust(8, b"\0"), 0, 0,
        len(section["data"]), raw_offsets[index], reloc_offsets[index], 0,
        len(section["relocs"]), 0, section["flags"])
        for index, section in enumerate(sections))
    coff = bytearray(struct.pack("<HHIIIHH", 0x14C, len(sections), 0,
                                 symbol_offset, len(symbols), 0, 0) + headers)
    for section in sections:
        coff.extend(section["data"])
    for section in sections:
        for offset, key, kind in section["relocs"]:
            coff.extend(struct.pack("<IIH", offset, indices[key], kind))
    struct.pack_into("<I", strings, 0, len(strings))
    coff.extend(b"".join(symbols))
    coff.extend(strings)

    text = bytearray(0x2000)
    text[:len(root_code)] = root_code
    text[0x100:0x100 + len(original_cold)] = original_cold
    text[FRAME_TARGET - (BASE + 0x1000)] = 0xC3
    text[CALL_TARGET - (BASE + 0x1000)] = 0xC3
    text[RUNTIME_TARGET - (BASE + 0x1000)] = 0xC3
    rdata = bytearray(0x200)
    rdata[:len(original_info)] = original_info
    sites = [0x1003, 0x1100 + handler_offset + 1, 0x4008]
    sites.extend(0x4000 + 36 + 8 * index for index in range(count))
    grouped = {}
    for rva in sites:
        grouped.setdefault(rva & ~0xFFF, []).append(0x3000 | (rva & 0xFFF))
    reloc = bytearray()
    for page, entries in sorted(grouped.items()):
        if len(entries) % 2:
            entries.append(0)
        reloc.extend(struct.pack("<II", page, 8 + 2 * len(entries)))
        reloc.extend(struct.pack("<" + "H" * len(entries), *entries))
    reloc_directory_size = len(reloc)
    reloc_size = max(0x200, (len(reloc) + 0x1FF) & ~0x1FF)
    reloc.extend(bytes(reloc_size - len(reloc)))
    text_raw, rdata_raw = 0x200, 0x2200
    reloc_raw = rdata_raw + len(rdata)
    pe = bytearray(reloc_raw + reloc_size)
    pe[:2] = b"MZ"
    struct.pack_into("<I", pe, 0x3C, 0x80)
    pe[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", pe, 0x84, 0x14C, 3, 0, 0, 0, 224, 0x0102)
    optional = 0x98
    struct.pack_into("<H", pe, optional, 0x10B)
    struct.pack_into("<I", pe, optional + 16, 0x1000)
    struct.pack_into("<I", pe, optional + 28, BASE)
    struct.pack_into("<II", pe, optional + 32, 0x1000, 0x200)
    struct.pack_into("<I", pe, optional + 56, 0x6000)
    struct.pack_into("<I", pe, optional + 60, 0x200)
    struct.pack_into("<H", pe, optional + 68, 3)
    struct.pack_into("<I", pe, optional + 92, 16)
    struct.pack_into("<II", pe, optional + 96 + 5 * 8,
                     0x5000, reloc_directory_size)
    section_at = optional + 224
    pe_sections = [
        (b".text", 0x1000, 0x2000, text_raw, 0x2000, 0x60000020, text),
        (b".rdata", 0x4000, 0x200, rdata_raw, 0x200, 0x40000040, rdata),
        (b".reloc", 0x5000, reloc_size, reloc_raw, reloc_size, 0x42000040, reloc),
    ]
    for index, (name, rva, vsize, raw, rawsize, flags, content) in enumerate(pe_sections):
        struct.pack_into("<8sIIIIIIHHI", pe, section_at + 40 * index,
                         name.ljust(8, b"\0"), vsize, rva, rawsize, raw,
                         0, 0, 0, 0, flags)
        pe[raw:raw + len(content)] = content

    if original_damage == "bad_handler_info":
        pe[text_raw + 0x100 + handler_offset + 1] ^= 1
    elif original_damage == "bad_handler_info_bounds":
        struct.pack_into("<I", pe, text_raw + 0x100 + handler_offset + 1,
                         INFO_VA + 0x1000)
    image_bytes = bytes(pe)
    image = PeImage(image_bytes, hashlib.sha256(image_bytes).hexdigest())
    spans = (ROOT_SPAN, (COLD_VA, COLD_VA + len(original_cold)))
    record_text = (f"// ORIGINAL: 0x{ROOT_SPAN[0]:08X} synthetic "
                   f"0x{ROOT_SPAN[0]:08X}-0x{ROOT_SPAN[1]:08X};"
                   f"0x{spans[1][0]:08X}-0x{spans[1][1]:08X} FILE\n"
                   f"// symbol {ROOT_NAME}\nvoid synthetic_owner() {{}}\n")
    record, = read_text(record_text, "synthetic_scalar_eh.cpp")
    return bytes(coff), image, spans, record


def _measure(fixture, tmp_path):
    object_bytes, image, spans, record = fixture
    bound = bind_eh_regions(object_bytes, ROOT_NAME, image, spans)
    listing = subject_asm(object_bytes, record, "synthetic-scalar-eh")
    image_path = tmp_path / "synthetic-scalar-eh.exe"
    image_path.write_bytes(image.data)
    comparison = compare_subject(record, image_path, listing)
    return Coff(object_bytes), bound, comparison


def _indexed_targets(obj, targets):
    return {str(symbol.index): targets[symbol.name]
            for symbol in obj.symbols.values() if symbol.name in targets}


@pytest.mark.parametrize("displacement", [-16, -24, -32, -0x120, 4, 8, 24, 36, 0x120])
def test_scalar_action_binds_complete_pe_coff_and_compares_full_regions(tmp_path,
                                                                        displacement):
    fixture = _build(displacement=displacement)
    obj, regions, result = _measure(fixture, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    ownership = regions[0].ownership
    original, compiled = ownership.original_actions[0], ownership.compiled_actions[0]
    assert original.kind is compiled.kind is EHActionKind.FRAME_SLOT_CALL_CLEANUP
    expected_end = 11 if -128 <= displacement <= 127 else 14
    assert (original.start, original.end) == (compiled.start, compiled.end) == (0, expected_end)
    assert (original.frame_displacement, original.base_adjustment,
            original.scratch_register) == (displacement, 0, "eax")
    assert (compiled.frame_displacement, compiled.base_adjustment,
            compiled.scratch_register) == (displacement, 0, "eax")
    assert len(original.transfers) == len(compiled.transfers) == 1
    original_call, compiled_call = original.transfers[0], compiled.transfers[0]
    assert original_call.role is compiled_call.role is EHTransferRole.CLEANUP
    assert (original_call.encoding, original_call.target) == ("REL32", CALL_TARGET)
    assert (compiled_call.encoding, compiled_call.relocation_kind,
            compiled_call.target, compiled_call.addend) == (
                "REL32", "REL32", CALL_SYMBOL, 0)
    assert len(obj.code_sections(obj.function(ROOT_NAME))) == 2


def test_changed_external_callee_stays_distinct_from_pinned_target(tmp_path):
    changed_symbol = "?different_synthetic_cleanup@@YAXPAX@Z"
    fixture = _build(call_symbol=changed_symbol)
    obj, regions, result = _measure(fixture, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    original, compiled = (regions[0].ownership.original_actions[0],
                         regions[0].ownership.compiled_actions[0])
    assert original.transfers[0].target == CALL_TARGET
    assert compiled.transfers[0].target == changed_symbol
    assert original.transfers[0].target != compiled.transfers[0].target
    assert obj.function(ROOT_NAME) is not None
    _raw, image, spans, _record = fixture
    unit = "synthetic_scalar_eh.cpp"
    binding = FunctionBinding(
        Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", ROOT_SPAN[0]),
        tuple(Span(start, end) for start, end in spans), ROOT_NAME, unit)
    unresolved, = check_objects((binding,), {unit: obj}, image)
    assert unresolved["result"] != Result.PASS
    callee, = (item for item in unresolved["diagnostic"]["emitted_references"]
               if item["symbol"] == changed_symbol)
    assert callee["binding_address"] is None
    wrong_target, = check_objects(
        (binding,), {unit: obj}, image,
        external_targets={changed_symbol: CALL_TARGET + 4})
    assert wrong_target["result"] == Result.FAIL


def test_bound_object_resolves_only_the_proved_local_funcinfo_reference():
    raw, image, spans, _record = _build()
    obj = Coff(raw)
    binding = FunctionBinding(
        Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", ROOT_SPAN[0]),
        tuple(Span(start, end) for start, end in spans), ROOT_NAME,
        "synthetic_scalar_eh.cpp")
    targets = {FRAME_SYMBOL: FRAME_TARGET, CALL_SYMBOL: CALL_TARGET,
               RUNTIME_SYMBOL: RUNTIME_TARGET}
    indexed_targets = _indexed_targets(obj, targets)
    index, target = eh_funcinfo_reference_target(
        raw, ROOT_NAME, image, spans, indexed_targets)
    assert obj.symbols[index].name == "$Tinfo"
    assert target == INFO_VA

    row, = check_objects((binding,), {binding.unit: obj}, image,
                         external_targets=targets)
    assert row["result"] == Result.PASS
    reference, = (item for item in row["diagnostic"]["emitted_references"]
                  if item["symbol"] == "$Tinfo")
    assert reference["binding_address"] == f"0x{INFO_VA:08X}"


@pytest.mark.parametrize(("damage", "expected_reason"), [
    ("wrong_info_field", "COFF FuncInfo is not an associated root-owned data section"),
    ("handler_info_addend", "COFF handler FuncInfo pointer has a nonzero addend"),
    ("bad_predecessor", "COFF unwind predecessor is not a sentinel or earlier state"),
    ("wrong_count", "COFF FuncInfo does not have a supported bounded cleanup layout"),
    ("call_addend", "COFF frame-local cleanup lacks one unambiguous external function relocation"),
])
def test_unowned_or_changed_eh_funcinfo_is_not_promoted(damage, expected_reason):
    raw, image, spans, _record = _build(object_damage=damage,
                                        action_kinds=("scalar", "frame"))
    obj = Coff(raw)
    binding = FunctionBinding(
        Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", ROOT_SPAN[0]),
        tuple(Span(start, end) for start, end in spans), ROOT_NAME,
        "synthetic_scalar_eh.cpp")
    targets = {FRAME_SYMBOL: FRAME_TARGET, CALL_SYMBOL: CALL_TARGET,
               RUNTIME_SYMBOL: RUNTIME_TARGET}
    if damage == "call_addend":
        call_index = next(symbol.index for symbol in obj.symbols.values()
                          if symbol.name == CALL_SYMBOL)
        call_offset, = (offset for offset, symbol_index, _kind
                        in obj.sections[1].relocations if symbol_index == call_index)
        assert struct.unpack_from("<I", obj.sections[1].data, call_offset)[0] == 1
    with pytest.raises(ValueError, match=expected_reason):
        eh_funcinfo_reference_target(
            raw, ROOT_NAME, image, spans, _indexed_targets(obj, targets))
    row, = check_objects((binding,), {binding.unit: obj}, image,
                         external_targets=targets)
    assert row["result"] != Result.PASS
    info_refs = [item for item in row["diagnostic"]["emitted_references"]
                 if item["symbol"] in ("$Tinfo", "$Tunwind")]
    assert info_refs
    assert all(item["binding_address"] == "compiler-artifact" for item in info_refs)


def test_changed_frame_action_semantics_do_not_promote_funcinfo():
    raw, image, spans, _record = _build(object_displacement=-24,
                                        action_kinds=("scalar", "frame"))
    obj = Coff(raw)
    binding = FunctionBinding(
        Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", ROOT_SPAN[0]),
        tuple(Span(start, end) for start, end in spans), ROOT_NAME,
        "synthetic_scalar_eh.cpp")
    targets = {FRAME_SYMBOL: FRAME_TARGET, CALL_SYMBOL: CALL_TARGET,
               RUNTIME_SYMBOL: RUNTIME_TARGET}
    indexed_targets = _indexed_targets(obj, targets)
    with pytest.raises(ValueError, match="semantics differ"):
        eh_funcinfo_reference_target(raw, ROOT_NAME, image, spans, indexed_targets)
    row, = check_objects((binding,), {binding.unit: obj}, image,
                         external_targets=targets)
    assert row["result"] != Result.PASS
    reference, = (item for item in row["diagnostic"]["emitted_references"]
                  if item["symbol"] == "$Tinfo")
    assert reference["binding_address"] == "compiler-artifact"


def test_wrong_funcinfo_association_fails_before_local_target_resolution():
    raw, image, spans, _record = _build(object_damage="wrong_info_association")
    with pytest.raises(ValueError, match="associative COMDAT parent"):
        Coff(raw)


def test_other_function_cannot_reuse_this_owners_local_funcinfo_index():
    raw, image, spans, _record = _build()
    obj = Coff(raw)
    targets = _indexed_targets(obj, {FRAME_SYMBOL: FRAME_TARGET,
                                     CALL_SYMBOL: CALL_TARGET,
                                     RUNTIME_SYMBOL: RUNTIME_TARGET})
    with pytest.raises(ValueError):
        eh_funcinfo_reference_target(
            raw, "?different_owner@@YAXXZ", image, spans, targets)


def test_changed_cleanup_transfer_target_does_not_prove_funcinfo():
    changed_call = "?different_cleanup_target@@YAXPAX@Z"
    raw, image, spans, _record = _build(call_symbol=changed_call)
    obj = Coff(raw)
    binding = FunctionBinding(
        Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", ROOT_SPAN[0]),
        tuple(Span(start, end) for start, end in spans), ROOT_NAME,
        "synthetic_scalar_eh.cpp")
    targets = {FRAME_SYMBOL: FRAME_TARGET, changed_call: CALL_TARGET + 4,
               RUNTIME_SYMBOL: RUNTIME_TARGET}
    indexed_targets = _indexed_targets(obj, targets)
    with pytest.raises(ValueError, match="cleanup transfer"):
        eh_funcinfo_reference_target(raw, ROOT_NAME, image, spans, indexed_targets)
    row, = check_objects((binding,), {binding.unit: obj}, image,
                         external_targets=targets)
    assert row["result"] == Result.FAIL
    reference, = (item for item in row["diagnostic"]["emitted_references"]
                  if item["symbol"] == "$Tinfo")
    assert reference["binding_address"] == "compiler-artifact"


def test_scalar_action_can_share_one_ordered_eh_map_with_frame_transfer(tmp_path):
    _obj, regions, result = _measure(
        _build(action_kinds=("frame", "scalar")), tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    actions = regions[0].ownership.compiled_actions
    assert [action.kind for action in actions] == [
        EHActionKind.FRAME_TRANSFER, EHActionKind.FRAME_SLOT_CALL_CLEANUP]
    assert [(action.start, action.end) for action in actions] == [(0, 8), (8, 19)]
    assert regions[0].ownership.compiled_predecessors == (0xFFFFFFFF, 0)


def test_valid_slot_offset_change_is_a_semantic_region_mismatch(tmp_path):
    _obj, _regions, result = _measure(
        _build(displacement=8, object_displacement=-16), tmp_path)
    assert result.verdict.value == "MISMATCH", result


@pytest.mark.parametrize("displacement", [0, -17, 6])
def test_zero_or_misaligned_frame_slots_refuse(displacement):
    object_bytes, image, spans, _record = _build(displacement=displacement)
    with pytest.raises(ValueError):
        bind_eh_regions(object_bytes, ROOT_NAME, image, spans)


@pytest.mark.parametrize(("damage", "displacement"), [
    ("zero_frame", -16), ("misaligned_frame", -16),
    ("wrong_frame_base", -16), ("indexed_frame", -16),
    ("wrong_pop", -16), ("wrong_return", -16),
    ("indirect_call", -16), ("prefix", -16), ("trailing_byte", -16),
])
def test_malformed_scalar_action_grammar_refuses(damage, displacement):
    object_bytes, image, spans, _record = _build(
        displacement=displacement, object_damage=damage)
    with pytest.raises(ValueError):
        bind_eh_regions(object_bytes, ROOT_NAME, image, spans)


@pytest.mark.parametrize("damage", [
    "zero_frame", "wrong_pop", "wrong_return", "prefix", "trailing_byte",
])
def test_malformed_coff_action_refuses_with_valid_original(damage):
    _valid_object, image, spans, _record = _build()
    malformed_object, _damaged_image, _damaged_spans, _ = _build(object_damage=damage)
    # Keep the original independently valid: refusal must reach the COFF
    # action decoder rather than being satisfied by damaged PE bytes first.
    with pytest.raises(ValueError, match="COFF"):
        bind_eh_regions(malformed_object, ROOT_NAME, image, spans)


@pytest.mark.parametrize("damage", [
    "missing_call", "duplicate_call", "wrong_call_kind", "misaligned_call",
    "call_addend", "call_not_function", "call_conflicting_alias",
    "extra_child_relocation", "wrong_association", "bad_map_addend",
    "bad_predecessor", "missing_info_relocation", "missing_runtime_relocation",
    "missing_map_relocation", "wrong_info_association",
    "missing_root_handler_relocation", "wrong_handler_target",
])
def test_malformed_scalar_coff_binding_refuses(damage):
    fixture = _build(action_kinds=("frame", "scalar") if damage == "bad_predecessor"
                     else ("scalar",), object_damage=damage)
    object_bytes, image, spans, _record = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(object_bytes, ROOT_NAME, image, spans)


@pytest.mark.parametrize("damage", ["bad_map_target", "bad_predecessor",
                                     "bad_handler_info", "bad_handler_info_bounds",
                                     "bad_unwind_bounds"])
def test_malformed_original_eh_bounds_refuse(damage):
    fixture = _build(action_kinds=("frame", "scalar") if damage == "bad_predecessor"
                     else ("scalar",), original_damage=damage)
    object_bytes, image, spans, _record = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(object_bytes, ROOT_NAME, image, spans)
