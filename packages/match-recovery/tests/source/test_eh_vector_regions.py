"""Serialized public-interface controls for bounded vector EH actions."""

import hashlib
import struct
from pathlib import Path

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source.asm import compare_subject, subject_asm
from match_recovery.source.eh_actions import EHActionKind, EHTransferRole
from match_recovery.source.eh_regions import bind_eh_regions
from match_recovery.source.reader import read_text


BASE = 0x00400000
ROOT_NAME = "?array_owner@@YAXXZ"
ROOT_SPAN = (BASE + 0x1000, BASE + 0x1016)
COLD_VA = BASE + 0x1100
RDATA_VA = BASE + 0x4000
HELPER_VA = BASE + 0x1F20
CALLBACK_VA = BASE + 0x1F30
TAIL_VA = BASE + 0x1F00
RUNTIME_VA = BASE + 0x1F10


def _array_action(*, count=3, stride=12, frame_disp=-16, base_adjust=0x234,
                  count_width=1, stride_width=1, frame_width=1, add_width=4,
                  add_form="accumulator",
                  callback=CALLBACK_VA, helper=HELPER_VA, damage=None):
    """Encode the bounded push/push/push/load/add/push/call/ret action."""
    code = bytearray(b"\x68" + struct.pack("<I", callback))
    callback_site = 1
    for value, width in ((count, count_width), (stride, stride_width)):
        if width == 1:
            if not 0 < value <= 127:
                raise ValueError("imm8 fixture values must be positive signed bytes")
            code.extend((0x6A, value))
        elif width == 4:
            code.extend(b"\x68" + struct.pack("<I", value))
        else:
            raise ValueError("push immediate width must be 1 or 4")
    if frame_width == 1:
        if not -128 <= frame_disp <= 127:
            raise ValueError("disp8 fixture is out of range")
        code.extend((0x8B, 0x45, frame_disp & 0xFF))
    elif frame_width == 4:
        code.extend(b"\x8B\x85" + struct.pack("<i", frame_disp))
    else:
        raise ValueError("frame displacement width must be 1 or 4")
    if add_width == 1:
        if not -128 <= base_adjust <= 127:
            raise ValueError("imm8 fixture adjustment is out of range")
        code.extend((0x83, 0xC0, base_adjust & 0xFF))
    elif add_width == 4 and add_form == "accumulator":
        code.extend(b"\x05" + struct.pack("<I", base_adjust))
    elif add_width == 4 and add_form == "group":
        code.extend(b"\x81\xC0" + struct.pack("<I", base_adjust))
    else:
        raise ValueError("base adjustment width must be 1 or 4")
    code.append(0x50)  # push eax
    call_start = len(code)
    code.append(0xE8)
    code.extend(b"\0\0\0\0")
    code.append(0xC3)
    helper_operand = call_start + 1
    if damage == "call_to_jump":
        code[call_start] = 0xE9
    elif damage == "missing_ret":
        code.pop()
    elif damage == "wrong_register":
        code[-(1 + 5 + 1)] = 0x51  # push ecx
    elif damage == "wrong_load_base":
        load = 5 + (2 if count_width == 1 else 5) + (2 if stride_width == 1 else 5)
        code[load + 1] = 0x46  # mov eax,[esi+disp8]
    elif damage == "wrong_add_register":
        # Both supported add encodings have their EAX destination in ModRM.
        add_size = (3 if add_width == 1 else
                    5 if add_form == "accumulator" else 6)
        add_at = call_start - (1 + add_size)
        code[add_at + 1] = 0xC1  # destination ECX
    elif damage == "trailing_nop":
        code.append(0x90)
    return bytes(code), callback_site, helper_operand


def _tail_action(width=8, *, target=TAIL_VA, damage=None):
    if width == 8:
        code = bytearray(b"\x8D\x4D\xF0\xE9\0\0\0\0")
        operand = 4
    elif width == 11:
        code = bytearray(b"\x8B\x4D\xF0\x83\xC1\x04\xE9\0\0\0\0")
        operand = 7
    elif width == 14:
        code = bytearray(b"\x8B\x4D\xF0\x81\xC1\x4C\x14\0\0\xE9\0\0\0\0")
        operand = 10
    else:
        raise ValueError("tail action width must be 8, 11, or 14")
    if damage == "short_jump":
        at = operand - 1
        code[at] = 0xEB
        del code[at + 2:at + 5]
    elif damage == "call_instead_of_jump":
        code[operand - 1] = 0xE8
    return bytes(code), operand


def _build(actions, *, object_array=None, original_array=None,
           object_damage=None, original_damage=None, map_damage=None,
           object_map_damage=None, original_callback_sites=True,
           original_reloc_damage=None, object_reloc_damage=None):
    """Serialize a PE32 and a three-section COFF object for public APIs."""
    action_specs = []
    for index, spec in enumerate(actions):
        if spec[0] == "array":
            params = dict(spec[1])
            orig_params = dict(params)
            obj_params = dict(params)
            if index == original_array and isinstance(original_damage, dict):
                orig_params.update(original_damage)
            if index == object_array and object_damage:
                obj_params.update(object_damage)
            action_specs.append(("array", orig_params, obj_params))
        else:
            action_specs.append(("tail", spec[1], spec[1]))

    original_actions, object_actions = [], []
    for kind, original_spec, object_spec in action_specs:
        if kind == "array":
            original_actions.append(_array_action(**original_spec))
            object_actions.append(_array_action(**object_spec))
        else:
            original_actions.append(_tail_action(original_spec))
            object_actions.append(_tail_action(object_spec))

    original_starts, object_starts = [], []
    original_cold, object_cold = bytearray(), bytearray()
    object_child_relocs = []
    base_sites = [0x1003]
    for index, ((kind, original_spec, object_spec), orig_action, obj_action) in enumerate(
            zip(action_specs, original_actions, object_actions)):
        original_starts.append(len(original_cold))
        object_starts.append(len(object_cold))
        orig_bytes = bytearray(orig_action[0])
        obj_bytes = bytearray(obj_action[0])
        if kind == "array":
            cb_offset, helper_offset = orig_action[1:]
            obj_cb_offset, obj_helper_offset = obj_action[1:]
            helper_next = COLD_VA + len(original_cold) + helper_offset + 4
            helper_target = original_spec.get("helper", HELPER_VA)
            struct.pack_into("<i", orig_bytes, helper_offset, helper_target - helper_next)
            struct.pack_into("<I", obj_bytes, obj_cb_offset, 0)
            struct.pack_into("<i", obj_bytes, obj_helper_offset, 0)
            object_child_relocs.extend([
                (len(object_cold) + obj_cb_offset, "callback", 6),
                (len(object_cold) + obj_helper_offset, "helper", 20),
            ])
            if original_callback_sites:
                base_sites.append(0x1100 + len(original_cold) + cb_offset)
            else:
                struct.pack_into("<I", orig_bytes, cb_offset, 0)
            # Positive callback values are the measured executable address;
            # malformed source-pointer mutations are built by callers below.
        else:
            tail_operand = orig_action[1]
            rel_next = COLD_VA + len(original_cold) + tail_operand + 4
            struct.pack_into("<i", orig_bytes, tail_operand, TAIL_VA - rel_next)
            obj_operand = obj_action[1]
            struct.pack_into("<i", obj_bytes, obj_operand, 0)
            object_child_relocs.append((len(object_cold) + obj_operand, "destructor", 20))
        original_cold.extend(orig_bytes)
        object_cold.extend(obj_bytes)

    original_handler_offset = len(original_cold)
    object_handler_offset = len(object_cold)
    original_info_at = RDATA_VA
    original_runtime_next = COLD_VA + original_handler_offset + 10
    object_cold.extend(b"\xB8\0\0\0\0\xE9\0\0\0\0")
    original_cold.extend(b"\xB8" + struct.pack("<I", original_info_at) + b"\xE9")
    original_cold.extend(struct.pack("<i", RUNTIME_VA - original_runtime_next))
    struct.pack_into("<I", object_cold, object_handler_offset + 1, 0)
    struct.pack_into("<i", object_cold, object_handler_offset + 6, 0)
    object_child_relocs.extend([
        (object_handler_offset + 1, "info", 6),
        (object_handler_offset + 6, "runtime", 20),
    ])
    if object_reloc_damage == "missing_callback":
        object_child_relocs = [row for row in object_child_relocs if row[1] != "callback"]
    elif object_reloc_damage == "callback_wrong_kind":
        object_child_relocs = [(at, target, 20 if target == "callback" else kind)
                               for at, target, kind in object_child_relocs]
    elif object_reloc_damage == "missing_helper":
        object_child_relocs = [row for row in object_child_relocs if row[1] != "helper"]
    elif object_reloc_damage == "helper_wrong_kind":
        object_child_relocs = [(at, target, 6 if target == "helper" else kind)
                               for at, target, kind in object_child_relocs]
    elif object_reloc_damage == "callback_nonfunction":
        object_child_relocs = [(at, "nonfunction" if target == "callback" else target, kind)
                               for at, target, kind in object_child_relocs]
    elif object_reloc_damage == "helper_nonfunction":
        object_child_relocs = [(at, "nonfunction" if target == "helper" else target, kind)
                               for at, target, kind in object_child_relocs]
    elif object_reloc_damage == "callback_other_function":
        object_child_relocs = [(at, "other_callback" if target == "callback" else target, kind)
                               for at, target, kind in object_child_relocs]
    elif object_reloc_damage == "nonzero_callback_addend":
        vector = next((i for i, row in enumerate(object_child_relocs)
                      if row[1] == "callback"), None)
        if vector is not None:
            struct.pack_into("<I", object_cold, object_child_relocs[vector][0], 1)
    elif object_reloc_damage == "nonzero_helper_addend":
        vector = next((i for i, row in enumerate(object_child_relocs)
                      if row[1] == "helper"), None)
        if vector is not None:
            struct.pack_into("<I", object_cold, object_child_relocs[vector][0], 1)
    elif object_reloc_damage == "extra_relocation":
        object_child_relocs.append((2, "destructor", 20))

    # PE root entry: complete VC6 FS registration with a relocated handler.
    root_code = bytearray.fromhex("6a ff 68 00 00 00 00 64 a1 00 00 00 00 50 64 89 25 00 00 00 00 c3")
    handler_va = COLD_VA + original_handler_offset
    struct.pack_into("<I", root_code, 3, handler_va)
    object_root_code = bytearray(root_code)
    struct.pack_into("<I", object_root_code, 3, 0)
    if original_damage == "bad_root":
        root_code[0] = 0x90

    # Original FuncInfo and state/action table; COFF stores corresponding
    # relocated zero-addend references in its associated .xdata COMDAT.
    n = len(actions)
    info_size = 32 + 8 * n
    original_info = bytearray(info_size)
    struct.pack_into("<8I", original_info, 0, 0x19930520, n,
                     original_info_at + 32, 0, 0, 0, 0, 0)
    object_info = bytearray(info_size)
    struct.pack_into("<8I", object_info, 0, 0x19930520, n, 0, 0, 0, 0, 0, 0)
    info_relocs = [(8, "unwind", 6)]
    for index, (o_start, c_start) in enumerate(zip(original_starts, object_starts)):
        predecessor = 0xFFFFFFFF if index == 0 else index - 1
        struct.pack_into("<II", original_info, 32 + 8 * index,
                         predecessor, COLD_VA + o_start)
        struct.pack_into("<II", object_info, 32 + 8 * index, predecessor, 0)
        info_relocs.append((36 + 8 * index, f"action{index}", 6))
    if map_damage == "bad_predecessor":
        struct.pack_into("<I", original_info, 32 + 8, 1)
    elif map_damage == "bad_action":
        struct.pack_into("<I", original_info, 36, COLD_VA + 1)
    if object_map_damage == "bad_predecessor":
        struct.pack_into("<I", object_info, 32 + 8, 1)
    elif object_map_damage == "bad_action_addend":
        struct.pack_into("<I", object_info, 36, 1)

    # Serialize COFF. Section-definition auxiliary records establish the exact
    # root -> cold/FuncInfo associations (not section order/name heuristics).
    sections = [
        {"name": ".text", "data": bytes(object_root_code), "flags": 0x60501020,
         "parent": 0, "selection": 2, "relocs": [(3, "handler", 6)]},
        {"name": ".text$x", "data": bytes(object_cold), "flags": 0x60501020,
         "parent": 1, "selection": 5, "relocs": object_child_relocs},
        {"name": ".xdata$x", "data": bytes(object_info), "flags": 0x40401040,
         "parent": 1, "selection": 5, "relocs": info_relocs},
    ]
    symbols, indices, strings = [], {}, bytearray(b"\0\0\0\0")

    def enc_name(value):
        raw = value.encode("ascii")
        if len(raw) <= 8:
            return raw.ljust(8, b"\0")
        offset = len(strings)
        strings.extend(raw + b"\0")
        return b"\0\0\0\0" + struct.pack("<I", offset)

    def add(key, name, value, section, kind, storage, aux=None):
        indices[key] = len(symbols)
        symbols.append(enc_name(name) + struct.pack("<IhHBB", value, section,
                                                    kind, storage, int(aux is not None)))
        if aux is not None:
            symbols.append(aux)

    for number, section in enumerate(sections, 1):
        aux = struct.pack("<IHHIHB3x", len(section["data"]), len(section["relocs"]),
                          0, 0, section["parent"], section["selection"])
        add(("section", number), section["name"], 0, number, 0, 3, aux)
    add("root", ROOT_NAME, 0, 1, 0x20, 2)
    add("handler", "$Lhandler", object_handler_offset, 2, 0, 6)
    for index, offset in enumerate(object_starts):
        add(f"action{index}", f"$Laction{index}", offset, 2, 0, 6)
    add("info", "$Tinfo", 0, 3, 0, 3)
    add("unwind", "$Tunwind", 32, 3, 0, 6)
    add("destructor", "??1TailElement@@QAE@XZ", 0, 0, 0x20, 2)
    add("callback", "??1Element@@QAE@XZ", 0, 0, 0x20, 2)
    add("other_callback", "??1OtherElement@@QAE@XZ", 0, 0, 0x20, 2)
    add("nonfunction", "?callback_data@@3HA", 0, 0, 0, 2)
    add("helper", "??_M@YGXPAXIHP6EX0@Z@Z", 0, 0, 0x20, 2)
    add("runtime", "___CxxFrameHandler", 0, 0, 0, 2)

    # Refresh section auxiliaries after final relocation counts are known.
    for number, section in enumerate(sections, 1):
        symbols[indices[("section", number)] + 1] = struct.pack(
            "<IHHIHB3x", len(section["data"]), len(section["relocs"]),
            0, 0, section["parent"], section["selection"])
    headers_size = 20 + 40 * len(sections)
    raw_cursor = headers_size
    raw_offsets = []
    for section in sections:
        raw_offsets.append(raw_cursor)
        raw_cursor += len(section["data"])
    reloc_cursor = raw_cursor
    reloc_offsets = []
    for section in sections:
        reloc_offsets.append(reloc_cursor if section["relocs"] else 0)
        reloc_cursor += 10 * len(section["relocs"])
    symbol_offset = reloc_cursor
    coff_headers = b"".join(struct.pack(
        "<8sIIIIIIHHI", section["name"].encode().ljust(8, b"\0"), 0, 0,
        len(section["data"]), raw_offsets[index], reloc_offsets[index], 0,
        len(section["relocs"]), 0, section["flags"])
        for index, section in enumerate(sections))
    coff = bytearray(struct.pack("<HHIIIHH", 0x14C, len(sections), 0,
                                 symbol_offset, len(symbols), 0, 0) + coff_headers)
    for section in sections:
        coff.extend(section["data"])
    for section in sections:
        for offset, target, kind in section["relocs"]:
            coff.extend(struct.pack("<IIH", offset, indices[target], kind))
    struct.pack_into("<I", strings, 0, len(strings))
    coff.extend(b"".join(symbols))
    coff.extend(strings)

    # Lay out a PE32 with executable storage large enough for the 64 x 34-byte
    # upper-bound fixture, separate read-only FuncInfo, and base relocations.
    text_size = 0x2000
    rdata_size = max(0x200, (info_size + 0x1FF) & ~0x1FF)
    text = bytearray(text_size)
    text[:len(root_code)] = root_code
    text[0x100:0x100 + len(original_cold)] = original_cold
    text[0xF00] = text[0xF10] = text[0xF20] = text[0xF30] = 0xC3
    rdata = bytearray(rdata_size)
    rdata[:info_size] = original_info
    base_sites.extend([0x1100 + original_handler_offset + 1,
                       0x4000 + 8])
    base_sites.extend(0x4000 + 36 + 8 * index for index in range(n))
    if original_reloc_damage == "moved_callback_site" and original_array is not None:
        expected_site = 0x1100 + original_starts[original_array] + 1
        base_sites = [site + 1 if site == expected_site else site for site in base_sites]
    grouped = {}
    for rva in base_sites:
        grouped.setdefault(rva & ~0xFFF, []).append(0x3000 | (rva & 0xFFF))
    reloc = bytearray()
    for page, entries in sorted(grouped.items()):
        if len(entries) % 2:
            entries.append(0)
        reloc.extend(struct.pack("<II", page, 8 + 2 * len(entries)))
        reloc.extend(struct.pack("<" + "H" * len(entries), *entries))
    reloc_directory_size = len(reloc)
    reloc_size = max(0x200, (reloc_directory_size + 0x1FF) & ~0x1FF)
    reloc.extend(bytes(reloc_size - len(reloc)))

    pe_headers_size = 0x200
    text_raw, rdata_raw, reloc_raw = pe_headers_size, pe_headers_size + text_size, pe_headers_size + text_size + rdata_size
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
    struct.pack_into("<I", pe, optional + 60, pe_headers_size)
    struct.pack_into("<H", pe, optional + 68, 3)
    struct.pack_into("<I", pe, optional + 92, 16)
    struct.pack_into("<II", pe, optional + 96 + 5 * 8,
                     0x5000, reloc_directory_size)
    section_at = optional + 224
    headers = [
        (b".text", 0x1000, text_size, text_raw, text_size, 0x60000020),
        (b".rdata", 0x4000, rdata_size, rdata_raw, rdata_size, 0x40000040),
        (b".reloc", 0x5000, reloc_size, reloc_raw, reloc_size, 0x42000040),
    ]
    for index, (name, vaddr, vsize, raw, rawsize, flags) in enumerate(headers):
        struct.pack_into("<8sIIIIIIHHI", pe, section_at + 40 * index,
                         name.ljust(8, b"\0"), vsize, vaddr, rawsize, raw,
                         0, 0, 0, 0, flags)
    pe[text_raw:text_raw + len(text)] = text
    pe[rdata_raw:rdata_raw + len(rdata)] = rdata
    pe[reloc_raw:reloc_raw + len(reloc)] = reloc

    image_bytes = bytes(pe)
    image = PeImage(image_bytes, hashlib.sha256(image_bytes).hexdigest())
    cold_span = (COLD_VA, COLD_VA + len(original_cold))
    spans = (ROOT_SPAN, cold_span)
    text_record = (f"// ORIGINAL: 0x{ROOT_SPAN[0]:08X} test-owned "
                   f"0x{ROOT_SPAN[0]:08X}-0x{ROOT_SPAN[1]:08X};"
                   f"0x{cold_span[0]:08X}-0x{cold_span[1]:08X} FILE\n"
                   f"// symbol {ROOT_NAME}\nvoid array_owner() {{}}\n")
    record, = read_text(text_record, "synthetic_vector_eh.cpp")
    return bytes(coff), image, spans, record, tuple(original_starts), tuple(object_starts)


def _measure(fixture, tmp_path):
    obj_data, image, spans, record, _original_starts, _object_starts = fixture
    obj = Coff(obj_data)
    regions = bind_eh_regions(obj_data, ROOT_NAME, image, spans)
    measured = subject_asm(obj_data, record, "serialized-vector-eh")
    image_path = Path(tmp_path) / "synthetic-vector-eh.exe"
    image_path.write_bytes(image.data)
    return obj, regions, compare_subject(record, image_path, measured)


def test_serialized_vector_and_tail_actions_bind_and_compare_publicly(tmp_path):
    fixture = _build([("array", {}), ("tail", 8), ("tail", 14)])
    obj, regions, result = _measure(fixture, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    ownership = regions[0].ownership
    assert len(ownership.original_actions) == len(ownership.compiled_actions) == 3
    original, compiled = ownership.original_actions[0], ownership.compiled_actions[0]
    assert original.kind is compiled.kind is EHActionKind.ARRAY_DESTRUCTOR
    assert (original.start, original.end, compiled.start, compiled.end) == (0, 24, 0, 24)
    assert (original.element_count, original.element_size) == (3, 12)
    assert (compiled.element_count, compiled.element_size) == (3, 12)
    assert original.frame_displacement == compiled.frame_displacement == -16
    assert original.base_adjustment == compiled.base_adjustment == 0x234
    assert original.scratch_register == compiled.scratch_register == "eax"
    assert [fact.role for fact in original.transfers] == [
        EHTransferRole.CALLBACK, EHTransferRole.HELPER]
    assert [fact.role for fact in compiled.transfers] == [
        EHTransferRole.CALLBACK, EHTransferRole.HELPER]
    cb, helper = original.transfers
    assert (cb.operand_offset, cb.width, cb.encoding, cb.relocation_kind,
            cb.target, cb.addend) == (1, 4, "ABS32", "HIGHLOW", CALLBACK_VA, None)
    cb_obj, helper_obj = compiled.transfers
    assert (cb_obj.encoding, cb_obj.relocation_kind, cb_obj.target, cb_obj.addend) == (
        "ABS32", "DIR32", "??1Element@@QAE@XZ", 0)
    assert (helper_obj.encoding, helper_obj.relocation_kind,
            helper_obj.target, helper_obj.addend) == (
                "REL32", "REL32", "??_M@YGXPAXIHP6EX0@Z@Z", 0)
    assert helper.operand_offset == helper_obj.operand_offset
    assert (helper.encoding, helper.relocation_kind, helper.target) == (
        "REL32", None, HELPER_VA)
    # Legacy tail actions remain represented and source-level helper identity
    # stays distinct from physical binding.
    assert [action.kind for action in ownership.compiled_actions[1:]] == [
        EHActionKind.FRAME_TRANSFER, EHActionKind.FRAME_TRANSFER]
    assert len(obj.code_sections(obj.function(ROOT_NAME))) == 2


@pytest.mark.parametrize("values", [
    {"count": 5, "stride": 28, "frame_disp": -24, "base_adjust": 0x5F50},
    {"count": 7, "stride": 4, "frame_disp": -4, "base_adjust": 0x1020,
     "count_width": 4, "stride_width": 4, "frame_width": 4, "add_width": 4},
])
def test_serialized_vector_numeric_variants_are_supported(tmp_path, values):
    fixture = _build([("array", values)])
    _obj, regions, result = _measure(fixture, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    action = regions[0].ownership.compiled_actions[0]
    assert (action.element_count, action.element_size,
            action.frame_displacement, action.base_adjustment) == (
                values["count"], values["stride"], values["frame_disp"],
                values["base_adjust"])


@pytest.mark.parametrize(("field", "value"), [
    ("count", 4), ("stride", 16), ("frame_disp", -20), ("base_adjust", 0x235),
])
def test_valid_object_operand_change_is_mismatch_not_relocation_mask(tmp_path, field, value):
    fixture = _build([("array", {})], object_array=0,
                     object_damage={field: value})
    _obj, _regions, result = _measure(fixture, tmp_path)
    assert result.verdict.value == "MISMATCH", result


@pytest.mark.parametrize("map_damage", ["bad_predecessor", "bad_action"])
def test_malformed_original_map_refuses(map_damage):
    fixture = _build([("array", {}), ("tail", 8)], map_damage=map_damage)
    obj_data, image, spans, _record, _original, _compiled = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(obj_data, ROOT_NAME, image, spans)


@pytest.mark.parametrize("map_damage", ["bad_predecessor", "bad_action_addend"])
def test_malformed_compiled_map_refuses(map_damage):
    fixture = _build([("array", {}), ("tail", 8)], object_map_damage=map_damage)
    obj_data, image, spans, _record, _original, _compiled = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(obj_data, ROOT_NAME, image, spans)


def test_maximum_state_count_and_34_byte_action_fit_real_serialized_sections(tmp_path):
    fixture = _build([("array", {"count": 3, "stride": 12,
                                  "count_width": 4, "stride_width": 4,
                                  "frame_width": 4, "add_width": 4,
                                  "add_form": "group"})] * 64)
    obj_data, image, spans, record, original_starts, object_starts = fixture
    assert len(original_starts) == len(object_starts) == 64
    assert spans[1][1] - spans[1][0] == 64 * 34 + 10
    obj, regions, result = _measure(fixture, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    assert len(regions[0].ownership.compiled_actions) == 64
    assert regions[0].ownership.compiled_actions[0].end == 34
    assert regions[1].original.end <= BASE + 0x3000
    assert len(obj.section_subject(regions[1].section)[0]) == 64 * 34 + 10


def test_twenty_one_state_272_byte_mixed_map_fits_disjoint_pe_storage(tmp_path):
    widths = [8] * 4 + [11] * 6 + [14] * 10
    fixture = _build([("array", {"count": 3, "stride": 12})]
                     + [("tail", width) for width in widths])
    _obj, regions, result = _measure(fixture, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    assert len(regions[0].ownership.original_actions) == 21
    assert regions[1].original.end - regions[1].original.start == 272


@pytest.mark.parametrize("damage", [
    "missing_callback", "callback_wrong_kind", "missing_helper", "helper_wrong_kind",
    "nonzero_callback_addend", "nonzero_helper_addend", "extra_relocation",
    "callback_nonfunction", "helper_nonfunction",
])
def test_malformed_compiled_action_relocations_refuse(damage):
    fixture = _build([("array", {})], object_reloc_damage=damage)
    obj_data, image, spans, _record, _original, _compiled = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(obj_data, ROOT_NAME, image, spans)


@pytest.mark.parametrize("damage", [
    "call_to_jump", "missing_ret", "wrong_register", "wrong_load_base",
    "trailing_nop",
])
def test_malformed_array_action_grammar_refuses(damage):
    fixture = _build([("array", {"damage": damage})])
    obj_data, image, spans, _record, _original, _compiled = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(obj_data, ROOT_NAME, image, spans)


def test_malformed_array_add_destination_refuses():
    fixture = _build([("array", {"add_form": "group", "damage": "wrong_add_register"})])
    obj_data, image, spans, _record, _original, _compiled = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(obj_data, ROOT_NAME, image, spans)


def test_missing_original_callback_highlow_refuses():
    fixture = _build([("array", {})], original_callback_sites=False)
    obj_data, image, spans, _record, _original, _compiled = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(obj_data, ROOT_NAME, image, spans)


def test_moved_original_callback_highlow_refuses():
    fixture = _build([("array", {})], original_array=0,
                     original_reloc_damage="moved_callback_site")
    obj_data, image, spans, _record, _original, _compiled = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(obj_data, ROOT_NAME, image, spans)


@pytest.mark.parametrize("target_field", ["callback", "helper"])
def test_original_composite_target_must_remain_bounded_executable(target_field):
    fixture = _build([("array", {target_field: RDATA_VA})])
    obj_data, image, spans, _record, _original, _compiled = fixture
    with pytest.raises(ValueError):
        bind_eh_regions(obj_data, ROOT_NAME, image, spans)


def test_physical_binding_does_not_claim_callback_symbol_identity():
    # Both callback targets remain valid code. Legacy masked equality is not
    # evidence that the original and candidate target the same symbol.
    fixture = _build([("array", {})], object_reloc_damage="callback_other_function")
    _obj_data, image, spans, _record, _original, _compiled = fixture
    regions = bind_eh_regions(_obj_data, ROOT_NAME, image, spans)
    assert regions[0].ownership.original_actions[0].transfers[0].target == CALLBACK_VA
    assert regions[0].ownership.compiled_actions[0].transfers[0].target == (
        "??1OtherElement@@QAE@XZ")

