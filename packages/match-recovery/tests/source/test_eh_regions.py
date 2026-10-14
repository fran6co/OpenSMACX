"""Synthetic, hermetic controls for the narrow VC6 EH-region owner binder."""

import hashlib
import struct

import pytest

from match_recovery.pe_coff import PeImage
from match_recovery.source.eh_regions import (
    EHRegionRole,
    bind_eh_regions,
)


BASE = 0x00400000
ROOT = "?recover@Owner@@QAEHXZ"
SPANS = ((BASE + 0x1000, BASE + 0x1016),
         (BASE + 0x1100, BASE + 0x1115))


def _pe(*, damage=None, cold_size=21, short_cleanup_jump=False):
    """Build a small PE32 with relocated root/handler/FuncInfo pointers."""
    text = bytearray(0x200)
    cold = BASE + 0x1100
    func_info = BASE + 0x2000
    cleanup_target = cold + 0x20 if short_cleanup_jump else BASE + 0x11F0
    handler_target = BASE + 0x11F8

    text[0:2] = b"\x6a\xff"
    text[2] = 0x68
    handler_offset = cold_size - 10
    struct.pack_into("<I", text, 3, cold + handler_offset)
    text[7:13] = b"\x64\xa1\0\0\0\0"
    text[13] = 0x50
    text[14:21] = b"\x64\x89\x25\0\0\0\0"
    text[21] = 0xC3
    handler_offset = cold_size - 10
    lea = (b"\x8d\x4d\xf0" if handler_offset == 8
           else b"\x8d\x8d\xf0\xff\xff\xff")
    clean_jump = 0x100 + len(lea)
    jump_size = 2 if short_cleanup_jump else 5
    handler_start = clean_jump + jump_size
    text[0x100:0x100 + len(lea)] = lea
    if short_cleanup_jump:
        text[clean_jump] = 0xEB
        struct.pack_into("<b", text, clean_jump + 1,
                         cleanup_target - (cold + handler_start - 0x100))
        text[cleanup_target - (BASE + 0x1000)] = 0xC3
    else:
        text[clean_jump] = 0xE9
        struct.pack_into("<i", text, clean_jump + 1,
                         cleanup_target - (cold + handler_start - 0x100))
    text[handler_start] = 0xB8
    struct.pack_into("<I", text, handler_start + 1, func_info)
    text[handler_start + 5] = 0xE9
    struct.pack_into("<i", text, handler_start + 6,
                     handler_target - (cold + cold_size))
    text[0x11F0 - 0x1000] = 0xC3
    text[0x11F8 - 0x1000] = 0xC3

    if damage == "original_handler_edge":
        struct.pack_into("<I", text, 3, cold + handler_offset - 1)
    if damage == "original_funcinfo_edge":
        struct.pack_into("<I", text, 0x10C, func_info + 4)
    if damage == "original_cleanup_shape":
        text[0x100] = 0x90
    if damage == "original_bad_registration_sentinel":
        text[0] = 0x90
    if damage == "original_bad_registration_fs_read":
        text[7] = 0x90
    if damage == "original_bad_registration_fs_write":
        text[14] = 0x90

    info = bytearray(40)
    struct.pack_into("<8I", info, 0, 0x19930520, 1, func_info + 32,
                     0, 0, 0, 0, 0)
    struct.pack_into("<I", info, 32, 0xFFFFFFFF)
    struct.pack_into("<I", info, 36, cold)
    if damage == "original_funcinfo_magic":
        struct.pack_into("<I", info, 0, 0)
    if damage == "original_unwind_action":
        struct.pack_into("<I", info, 36, cold + 1)

    reloc_entries = {
        0x1000: [0x3003, 0x3100 + handler_offset + 1],
        0x2000: [0x3008, 0x3024],
    }
    if damage == "original_missing_root_reloc":
        reloc_entries[0x1000].remove(0x3003)
    if damage == "original_missing_handler_reloc":
        reloc_entries[0x1000].remove(0x310C)
    if damage == "original_missing_funcinfo_reloc":
        reloc_entries[0x2000].remove(0x3008)
    if damage == "original_missing_cleanup_reloc":
        reloc_entries[0x2000].remove(0x3024)
    if damage == "original_extra_funcinfo_reloc":
        reloc_entries[0x2000].append(0x3004)
    if damage == "original_extra_cold_reloc":
        reloc_entries[0x1000].append(0x3100)
    reloc = bytearray()
    for page, entries in reloc_entries.items():
        if len(entries) % 2:
            entries = [*entries, 0]
        block_size = 8 + 2 * len(entries)
        reloc += struct.pack("<II", page, block_size)
        reloc += struct.pack("<" + "H" * len(entries), *entries)

    headers = bytearray(0x400)
    headers[:2] = b"MZ"
    struct.pack_into("<I", headers, 0x3C, 0x80)
    headers[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", headers, 0x84, 0x14C, 3, 0, 0, 0, 224, 0x0102)
    optional = 0x98
    struct.pack_into("<H", headers, optional, 0x10B)
    struct.pack_into("<I", headers, optional + 16, 0x1000)
    struct.pack_into("<I", headers, optional + 28, BASE)
    struct.pack_into("<II", headers, optional + 32, 0x1000, 0x200)
    struct.pack_into("<I", headers, optional + 56, 0x4000)
    struct.pack_into("<I", headers, optional + 60, 0x400)
    struct.pack_into("<H", headers, optional + 68, 3)
    struct.pack_into("<I", headers, optional + 92, 16)
    struct.pack_into("<II", headers, optional + 96 + 5 * 8,
                     0x3000, len(reloc))

    section_table = optional + 224
    sections = [
        (b".text", 0x1000, 0x400, bytes(text), 0x60000020),
        (b".rdata", 0x2000, 0x600, bytes(info), 0x40000040),
        (b".reloc", 0x3000, 0x800, bytes(reloc), 0x40000040),
    ]
    for index, (name, rva, raw, content, flags) in enumerate(sections):
        struct.pack_into("<8sIIIIIIHHI", headers, section_table + index * 40,
                         name.ljust(8, b"\0"), max(len(content), 0x200), rva,
                         0x200, raw, 0, 0, 0, 0, flags)
    data = headers + bytes(0xA00 - len(headers))
    for _name, _rva, raw, content, _flags in sections:
        data[raw:raw + len(content)] = content
    return bytes(data)


def _coff(*, damage=None, cold_size=21):
    """Build a root, one supported-size associative child, and EH data."""
    handler_offset = cold_size - 10
    lea = (bytes.fromhex("8d 4d f0") if handler_offset == 8
           else bytes.fromhex("8d 8d f0 ff ff ff"))
    child_data = lea + b"\xe9\0\0\0\0\xb8\0\0\0\0\xe9\0\0\0\0"
    if damage == "wrong_child_lea":
        child_data = b"\x8d\x0d" + child_data[2:]
    destructor_offset = len(lea) + 1
    info_offset = handler_offset + 1
    runtime_offset = handler_offset + 6
    sections = [
        {"name": ".text", "data": _root_code(damage),
         "flags": 0x60501020, "selection": 2, "parent": 0},
        {"name": ".text$x",
         "data": child_data,
         "flags": 0x60501020, "selection": 5, "parent": 1},
        {"name": ".xdata$x", "data": _eh_data(damage),
         "flags": 0x40401040, "selection": 5, "parent": 1},
    ]
    if damage == "truncated_child":
        sections[1]["data"] = child_data[:-1]
    if damage == "extra_code_child":
        sections.append({"name": ".textra", "data": b"\xc3",
                         "flags": 0x60501020, "selection": 5, "parent": 1})
    if damage == "extra_eh_data":
        sections.append({"name": ".xtraeh", "data": bytes(8),
                         "flags": 0x40401040, "selection": 5, "parent": 1})
    if damage == "debug_code_child":
        sections.append({"name": ".debug$Q", "data": b"\xc3",
                         "flags": 0x60501020, "selection": 5, "parent": 1})

    symbols = []
    indices = {}
    string_table = bytearray(b"\0\0\0\0")

    def name_bytes(name):
        encoded = name.encode("ascii")
        if len(encoded) <= 8:
            return encoded.ljust(8, b"\0")
        offset = len(string_table)
        string_table.extend(encoded + b"\0")
        return b"\0\0\0\0" + struct.pack("<I", offset)

    def add_symbol(key, name, value, section, kind, storage, aux=b""):
        index = len(symbols)
        symbols.append(name_bytes(name) + struct.pack("<IhHBB", value, section,
                                                       kind, storage, 1 if aux else 0))
        if aux:
            assert len(aux) == 18
            symbols.append(aux)
        indices[key] = index

    def section_aux(section):
        relocs = section.get("relocs", ())
        return struct.pack("<IHHIHB3x", len(section["data"]), len(relocs), 0, 0,
                           section["parent"], section["selection"])

    for number, section in enumerate(sections, 1):
        add_symbol(("section", number), section["name"], 0, number, 0, 3,
                   section_aux(section))
    add_symbol("root", ROOT, 0, 1, 0x20, 2)
    if damage == "ambiguous_root":
        add_symbol("root_duplicate", ROOT, 0, 1, 0x20, 2)
    add_symbol("handler", "$Lhandler", handler_offset, 2, 0, 6)
    add_symbol("cleanup", "$Lcleanup", 0, 2, 0, 6)
    add_symbol("info", "$Tinfo", 0, 3, 0, 6 if damage == "bad_info_storage" else 3)
    add_symbol("unwind", "$Tunwind", 32, 3, 0, 6)
    add_symbol("destructor", "??1Thing@@QAE@XZ", 0, 0, 0x20, 2)
    add_symbol("runtime", "___CxxFrameHandler", 0, 0, 0, 2)
    if damage == "handler_offset":
        # A separate local target avoids relying on a generated label spelling.
        add_symbol("wrong_handler", "$Lwrong", 10, 2, 0, 6)

    root_relocs = [(3, "handler" if damage != "handler_offset" else "wrong_handler", 6)]
    child_relocs = [(destructor_offset, "destructor", 20),
                    (info_offset, "info", 6), (runtime_offset, "runtime", 20)]
    data_relocs = [(8, "unwind", 6), (36, "cleanup", 6)]
    if damage == "missing_root_edge":
        root_relocs.clear()
    if damage == "missing_info_edge":
        child_relocs = [item for item in child_relocs if item[0] != info_offset]
    if damage == "missing_destructor_edge":
        child_relocs = [item for item in child_relocs if item[0] != destructor_offset]
    if damage == "missing_runtime_edge":
        child_relocs = [item for item in child_relocs if item[0] != runtime_offset]
    if damage == "extra_child_relocation":
        child_relocs.append((1, "destructor", 6))
    if damage == "wrong_runtime":
        add_symbol("wrong_runtime", "___OtherHandler", 0, 0, 0, 2)
        child_relocs = [(*item[:2], 20) if item[0] == runtime_offset else item for item in child_relocs]
        child_relocs = [(off, "wrong_runtime" if off == runtime_offset else target, kind)
                        for off, target, kind in child_relocs]
    if damage == "wrong_cleanup_edge":
        add_symbol("wrong_cleanup", "$Lwrongcleanup", 1, 2, 0, 6)
        data_relocs = [(off, "wrong_cleanup" if off == 36 else target, kind)
                       for off, target, kind in data_relocs]
    if damage == "bad_destructor":
        add_symbol("not_destructor", "?other@@YAXXZ", 0, 0, 0x20, 2)
        child_relocs = [(off, "not_destructor" if off == 7 else target, kind)
                        for off, target, kind in child_relocs]
    if damage in {"nonzero_relative_addend", "nonzero_runtime_addend"}:
        sections[1]["data"] = bytearray(sections[1]["data"])
        offset = destructor_offset if damage == "nonzero_relative_addend" else runtime_offset
        struct.pack_into("<I", sections[1]["data"], offset, 1)

    sections[0]["relocs"] = root_relocs
    sections[1]["relocs"] = child_relocs
    sections[2]["relocs"] = data_relocs
    if len(sections) > 3:
        sections[3]["relocs"] = []
    if len(sections) > 4:
        sections[4]["relocs"] = []
    if len(sections) > 5:
        sections[5]["relocs"] = []

    # Section auxiliary records include each section's final relocation count.
    for number, section in enumerate(sections, 1):
        aux_index = indices[("section", number)] + 1
        symbols[aux_index] = section_aux(section)

    def make_string_table():
        struct.pack_into("<I", string_table, 0, len(string_table))
        return bytes(string_table)

    n_sections = len(sections)
    headers_size = 20 + n_sections * 40
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
    sym_offset = reloc_cursor
    section_headers = b""
    for index, section in enumerate(sections):
        section_headers += struct.pack(
            "<8sIIIIIIHHI", section["name"].encode().ljust(8, b"\0"), 0, 0,
            len(section["data"]), raw_offsets[index], reloc_offsets[index], 0,
            len(section["relocs"]), 0, section["flags"])
    header = struct.pack("<HHIIIHH", 0x14C, n_sections, 0, sym_offset,
                         len(symbols), 0, 0)
    content = bytearray(header + section_headers)
    for section in sections:
        content.extend(section["data"])
    for section in sections:
        for offset, target, kind in section["relocs"]:
            content.extend(struct.pack("<IIH", offset, indices[target], kind))
    content.extend(b"".join(symbols))
    content.extend(make_string_table())
    return bytes(content)


def _eh_data(damage):
    data = bytearray(40)
    struct.pack_into("<8I", data, 0, 0x19930520, 1, 0, 0, 0, 0, 0, 0)
    struct.pack_into("<I", data, 32, 0xFFFFFFFF)
    if damage == "bad_info_magic":
        struct.pack_into("<I", data, 0, 0)
    if damage == "bad_max_state":
        struct.pack_into("<I", data, 4, 2)
    if damage == "bad_unwind_state":
        struct.pack_into("<I", data, 32, 0)
    return bytes(data)


def _root_code(damage):
    code = bytearray.fromhex(
        "6a ff 68 00 00 00 00 64 a1 00 00 00 00 50 64 89 25 00 00 00 00 c3")
    if damage == "bad_registration_sentinel":
        code[0] = 0x90
    if damage == "bad_registration_fs_read":
        code[7] = 0x90
    if damage == "bad_registration_fs_write":
        code[14] = 0x90
    return bytes(code)


def _bind(*, object_damage=None, image_damage=None, spans=SPANS,
          object_cold_size=21):
    original_size = (spans[1][1] - spans[1][0]
                     if type(spans) is tuple and len(spans) == 2
                     and type(spans[1]) is tuple and len(spans[1]) == 2
                     and spans[1][1] - spans[1][0] in (18, 21) else 21)
    image_data = _pe(damage=image_damage, cold_size=original_size)
    image = PeImage(image_data, hashlib.sha256(image_data).hexdigest())
    return bind_eh_regions(_coff(damage=object_damage, cold_size=object_cold_size), ROOT, image, spans)


def test_binds_only_the_root_and_its_proven_cleanup_handler_bundle():
    pairs = _bind()
    assert len(pairs) == 2
    root, cold = pairs
    assert (root.role, cold.role) == (EHRegionRole.ROOT, EHRegionRole.COLD)
    assert (root.original.start, root.original.end) == SPANS[0]
    assert root.section == root.ownership.root_section
    assert cold.section == cold.ownership.child_section
    assert cold.code == bytes.fromhex(
        "8d 8d f0 ff ff ff e9 00 00 00 00 b8 00 00 00 00 e9 00 00 00 00")
    assert cold.ownership.func_info_section == 3
    assert (cold.ownership.original_handler, cold.ownership.original_cleanup,
            cold.ownership.handler_offset, cold.ownership.cleanup_offset) == (
                SPANS[1][0] + 11, SPANS[1][0], 11, 0)


@pytest.mark.parametrize(("original_size", "object_size"), [(18, 18), (18, 21), (21, 18)])
def test_binds_each_bounded_compact_or_wide_encoding_independently(original_size, object_size):
    spans = (SPANS[0], (SPANS[1][0], SPANS[1][0] + original_size))
    root, cold = _bind(spans=spans, object_cold_size=object_size)
    assert len(cold.code) == object_size
    assert cold.ownership.original_handler_offset == original_size - 10
    assert cold.ownership.handler_offset == object_size - 10
    assert cold.ownership.original_handler == spans[1][0] + original_size - 10


def test_original_18_byte_extent_does_not_admit_short_cleanup_jump():
    spans = (SPANS[0], (SPANS[1][0], SPANS[1][0] + 18))
    data = _pe(cold_size=18, short_cleanup_jump=True)
    image = PeImage(data, hashlib.sha256(data).hexdigest())
    with pytest.raises(ValueError):
        bind_eh_regions(_coff(), ROOT, image, spans)


@pytest.mark.parametrize("damage", [
    "missing_root_edge", "handler_offset", "missing_info_edge", "bad_destructor",
    "wrong_runtime", "wrong_cleanup_edge", "bad_info_magic", "bad_max_state",
    "bad_unwind_state", "bad_info_storage", "extra_code_child", "extra_eh_data", "debug_code_child",
    "bad_registration_sentinel", "bad_registration_fs_read", "bad_registration_fs_write",
    "nonzero_relative_addend", "nonzero_runtime_addend",
    "missing_destructor_edge", "missing_runtime_edge", "extra_child_relocation",
    "wrong_child_lea", "truncated_child",
])
def test_refuses_damaged_or_ambiguous_compiled_eh_ownership(damage):
    with pytest.raises(ValueError):
        _bind(object_damage=damage)


@pytest.mark.parametrize("damage", [
    "original_handler_edge", "original_funcinfo_edge", "original_cleanup_shape",
    "original_funcinfo_magic", "original_unwind_action", "original_missing_root_reloc",
    "original_missing_handler_reloc", "original_missing_funcinfo_reloc",
    "original_missing_cleanup_reloc", "original_extra_funcinfo_reloc",
    "original_extra_cold_reloc", "original_bad_registration_sentinel",
    "original_bad_registration_fs_read", "original_bad_registration_fs_write",
])
def test_refuses_damaged_original_handler_or_unwind_ownership(damage):
    with pytest.raises(ValueError):
        _bind(image_damage=damage)


@pytest.mark.parametrize("spans", [
    (),
    ((SPANS[0][0], SPANS[0][1]),),
    ((SPANS[1][0], SPANS[1][1]), (SPANS[0][0], SPANS[0][1])),
    ((SPANS[0][0], SPANS[1][0] + 1), SPANS[1]),
    ((SPANS[0][0], SPANS[0][0]), SPANS[1]),
    ((SPANS[0][0], SPANS[0][1]), (SPANS[1][0], SPANS[1][1] - 1)),
    ((SPANS[0][0], SPANS[0][1]), (SPANS[1][0], SPANS[1][0] + 17)),
])
def test_refuses_malformed_reordered_overlapping_or_unbounded_spans(spans):
    with pytest.raises(ValueError):
        _bind(spans=spans)
