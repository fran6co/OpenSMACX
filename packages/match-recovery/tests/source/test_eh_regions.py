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
TWO_SPANS = (SPANS[0], (BASE + 0x1100, BASE + 0x1120))


def _pe(*, damage=None, cold_size=21, short_cleanup_jump=False, mode="default"):
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
    two_actions = cold_size == 32 or mode == "lea_pair"
    if mode == "lea_pair":
        widths = {26: (3, 3), 29: (3, 6), 32: (6, 6)}[cold_size]
        cursor = 0x100
        starts = []
        for width, target in zip(widths, (BASE + 0x11F0, BASE + 0x11E0)):
            starts.append(cursor - 0x100)
            lea = b"\x8d\x4d\xf0" if width == 3 else b"\x8d\x8d\xf0\xff\xff\xff"
            text[cursor:cursor + width] = lea
            jump = cursor + width
            text[jump] = 0xE9
            struct.pack_into("<i", text, jump + 1,
                             target - (cold + (jump - 0x100) + 5))
            cursor = jump + 5
        assert starts == [0, widths[0] + 5]
        text[cursor] = 0xB8
        struct.pack_into("<I", text, cursor + 1, func_info)
        text[cursor + 5] = 0xE9
        struct.pack_into("<i", text, cursor + 6,
                         handler_target - (cold + cold_size))
    elif cold_size == 32:
        text[0x100:0x116] = bytes.fromhex(
            "8b 4d f0 e9 00 00 00 00 8b 4d f0 81 c1 4c 14 00 00 e9 00 00 00 00")
        struct.pack_into("<i", text, 0x104, (BASE + 0x11F0) - (cold + 8))
        struct.pack_into("<i", text, 0x112, (BASE + 0x11E0) - (cold + 22))
        text[0x116] = 0xB8
        struct.pack_into("<I", text, 0x117, func_info)
        text[0x11B] = 0xE9
        struct.pack_into("<i", text, 0x11C, handler_target - (cold + cold_size))
    else:
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

    info = bytearray(48 if two_actions else 40)
    struct.pack_into("<8I", info, 0, 0x19930520, 2 if two_actions else 1, func_info + 32,
                     0, 0, 0, 0, 0)
    struct.pack_into("<I", info, 32, 0xFFFFFFFF)
    struct.pack_into("<I", info, 36, cold)
    if two_actions:
        second_start = 8 if mode == "default" else {26: 8, 29: 8, 32: 11}[cold_size]
        struct.pack_into("<II", info, 40, 0, cold + second_start)
    if damage == "original_funcinfo_magic":
        struct.pack_into("<I", info, 0, 0)
    if damage == "original_unwind_action":
        struct.pack_into("<I", info, 36, cold + 1)
    if damage == "original_second_map_wrong_state":
        struct.pack_into("<I", info, 40, 2)
    if damage == "original_second_map_wrong_action":
        struct.pack_into("<I", info, 44, cold + 7)
    if damage == "original_second_map_duplicate_action":
        struct.pack_into("<I", info, 44, cold)
    if damage == "original_second_map_gap":
        struct.pack_into("<I", info, 44, cold + second_start + 1)
    if damage == "original_wrong_state_count":
        struct.pack_into("<I", info, 4, 3)
    if damage == "original_second_action_shape":
        text[0x10B] = 0x83
    if damage == "original_second_action_jump":
        text[0x111] = 0xEB
    if damage == "original_handler_prefix" and mode == "lea_pair":
        offset = 0x100 + handler_offset
        text[offset:offset + 5] = b"\x66\xb8\0\0\x90"
    if damage == "original_cross_action_branch" and mode == "lea_pair":
        first_width = {26: 3, 29: 3, 32: 6}[cold_size]
        first_jump = cold + first_width
        second_start = first_width + 5
        struct.pack_into("<i", text, 0x100 + first_width + 1,
                         cold + second_start - (first_jump + 5))

    reloc_entries = {
        0x1000: [0x3003, 0x3100 + handler_offset + 1],
        0x2000: [0x3008, 0x3024] + ([0x302C] if two_actions else []),
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
    if damage == "original_missing_second_cleanup_reloc":
        reloc_entries[0x2000].remove(0x302C)
    if damage == "original_extra_second_cleanup_reloc":
        reloc_entries[0x2000].append(0x3028)
    if damage == "original_missing_second_map_reloc":
        reloc_entries[0x2000].remove(0x302C)
    if damage == "original_extra_second_map_reloc":
        reloc_entries[0x2000].append(0x3028)
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


def _coff(*, damage=None, cold_size=21, mode="default"):
    """Build a root, one supported-size associative child, and EH data."""
    handler_offset = cold_size - 10
    two_actions = cold_size == 32 or mode == "lea_pair"
    if mode == "lea_pair":
        widths = {26: (3, 3), 29: (3, 6), 32: (6, 6)}[cold_size]
        child = bytearray()
        destructor_offsets = []
        action_starts = []
        for width in widths:
            action_starts.append(len(child))
            child.extend(b"\x8d\x4d\xf0" if width == 3 else b"\x8d\x8d\xf0\xff\xff\xff")
            destructor_offsets.append(len(child) + 1)
            child.extend(b"\xe9\0\0\0\0")
        handler_offset = len(child)
        child.extend(b"\xb8\0\0\0\0\xe9\0\0\0\0")
        child_data = bytes(child)
        destructor_offsets = tuple(destructor_offsets)
        action_starts = tuple(action_starts)
    elif cold_size == 32:
        child_data = bytes.fromhex(
            "8b 4d f0 e9 00 00 00 00 8b 4d f0 81 c1 4c 14 00 00 e9 00 00 00 00"
            "b8 00 00 00 00 e9 00 00 00 00")
        lea_size = 3
        destructor_offsets = (4, 18)
        action_starts = (0, 8)
    else:
        lea = (bytes.fromhex("8d 4d f0") if handler_offset == 8
               else bytes.fromhex("8d 8d f0 ff ff ff"))
        child_data = lea + b"\xe9\0\0\0\0\xb8\0\0\0\0\xe9\0\0\0\0"
        lea_size = len(lea)
        destructor_offsets = (lea_size + 1,)
        action_starts = (0,)
    if damage == "wrong_child_lea":
        child_data = b"\x8d\x0d" + child_data[2:]
    if damage == "second_action_shape" and cold_size == 32:
        child_data = bytearray(child_data)
        child_data[11] = 0x83
        child_data = bytes(child_data)
    if damage == "second_action_jump" and cold_size == 32:
        child_data = bytearray(child_data)
        child_data[17] = 0xEB
        child_data = bytes(child_data)
    if damage == "malformed_handler_prefix" and mode == "lea_pair":
        child_data = bytearray(child_data)
        child_data[handler_offset:handler_offset + 5] = b"\x66\xb8\0\0\x90"
        child_data = bytes(child_data)
    destructor_offset = destructor_offsets[0]
    info_offset = handler_offset + 1
    runtime_offset = handler_offset + 6
    if damage == "func_info_nonzero_addend":
        child_data = bytearray(child_data)
        struct.pack_into("<I", child_data, info_offset, 1)
        child_data = bytes(child_data)
    sections = [
        {"name": ".text", "data": _root_code(damage),
         "flags": 0x60501020, "selection": 2, "parent": 0},
        {"name": ".text$x",
         "data": child_data,
         "flags": 0x60501020, "selection": 5, "parent": 1},
        {"name": ".xdata$x", "data": _eh_data(damage, two_actions=two_actions),
         "flags": 0x40401040, "selection": 5, "parent": 1},
    ]
    if two_actions:
        sections.append({"name": ".text$l", "data": b"\xc3",
                         "flags": 0x60501020, "selection": 2, "parent": 0})
        if damage == "local_target_writable":
            sections[3]["flags"] |= 0x80000000
        if damage == "local_target_root_owned":
            sections[3]["parent"] = 1
            sections[3]["selection"] = 5
        if damage == "local_target_executable_debug_child":
            sections.append({"name": ".debug$F", "data": b"\xc3",
                             "flags": 0x60501020, "selection": 5, "parent": 4})
        if damage in {"local_target_debug_child", "local_target_debug_child_and_cold_reference"}:
            sections.append({"name": ".debug$F", "data": bytes(4),
                             "flags": 0x40401040, "selection": 5, "parent": 4})
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
    if two_actions:
        external_second = damage in {
            "second_external_destructor", "second_external_wrong_name",
            "second_external_nonzero_value", "second_external_nonfunction",
        }
        if damage == "second_external_wrong_name":
            local_name = "??_DLocal@@QAEXXZ"
        elif external_second:
            local_name = "??1Time@@QAE@XZ"
        else:
            local_name = "?WrongLocal@@QAEXXZ" if damage == "local_target_wrong_name" else "??_DLocal@@QAEXXZ"
        local_type = 0 if damage in {"local_target_not_function", "second_external_nonfunction"} else 0x20
        local_section = (0 if external_second else
                         1 if damage == "local_target_root_owned" else 4)
        local_value = 1 if damage == "second_external_nonzero_value" else 0
        if damage == "second_external_defined":
            local_name = "??1Time@@QAE@XZ"
        add_symbol("local_destructor", local_name, local_value, local_section, local_type, 2)
        if damage == "local_target_overlapping_owner":
            add_symbol("local_duplicate", "??_DSecond@@QAEXXZ", 0, 4, 0x20, 2)
        if damage == "local_target_overlapping_static_owner":
            add_symbol("local_static_duplicate", "?Static@@YAXXZ", 0, 4, 0x20, 3)
    add_symbol("runtime", "___CxxFrameHandler", 0, 0, 0, 2)
    if damage == "handler_offset":
        # A separate local target avoids relying on a generated label spelling.
        add_symbol("wrong_handler", "$Lwrong", 10, 2, 0, 6)

    root_relocs = [(3, "handler" if damage != "handler_offset" else "wrong_handler", 6)]
    child_relocs = [(destructor_offset, "destructor", 20)]
    if two_actions:
        child_relocs.append((destructor_offsets[1], "local_destructor", 20))
    child_relocs.extend([(info_offset, "info", 6), (runtime_offset, "runtime", 20)])
    data_relocs = [(8, "unwind", 6), (36, "cleanup", 6)]
    if two_actions:
        cleanup2_value = (0 if damage == "second_map_duplicate_action" else
                          4 if damage == "second_map_wrong_target_offset" else action_starts[1])
        add_symbol("cleanup2", "$Lcleanup2", cleanup2_value, 2, 0, 6)
        data_relocs.append((44, "cleanup2", 6))
    if damage == "missing_root_edge":
        root_relocs.clear()
    if damage == "missing_info_edge":
        child_relocs = [item for item in child_relocs if item[0] != info_offset]
    if damage == "missing_destructor_edge":
        child_relocs = [item for item in child_relocs if item[0] != destructor_offset]
    if damage == "missing_second_action_relocation":
        child_relocs = [item for item in child_relocs if item[0] != 18]
    if damage == "extra_second_action_relocation":
        child_relocs.append((1, "local_destructor", 20))
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
    if damage == "local_target_nonzero_addend":
        sections[1]["data"] = bytearray(sections[1]["data"])
        struct.pack_into("<I", sections[1]["data"], 18, 1)
    if damage == "local_target_references_root":
        sections[3]["data"] = bytes.fromhex("e8 00 00 00 00 c3")
        sections[3]["relocs"] = [(1, "root", 20)]
    if damage == "local_target_debug_child_and_cold_reference":
        sections[3]["data"] = bytes.fromhex("e8 00 00 00 00 c3")
        sections[3]["relocs"] = [(1, "handler", 20)]
    if damage == "local_target_overlapping_relocations":
        sections[3]["relocs"] = [(1, "root", 20), (2, "root", 20)]
    if damage == "local_target_out_of_bounds_relocation":
        sections[3]["relocs"] = [(3, "root", 20)]

    sections[0]["relocs"] = root_relocs
    sections[1]["relocs"] = child_relocs
    sections[2]["relocs"] = data_relocs
    if len(sections) > 3:
        sections[3].setdefault("relocs", [])
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


def _eh_data(damage, *, two_actions=False):
    data = bytearray(48 if two_actions else 40)
    struct.pack_into("<8I", data, 0, 0x19930520, 2 if two_actions else 1,
                     0, 0, 0, 0, 0, 0)
    struct.pack_into("<I", data, 32, 0xFFFFFFFF)
    if damage == "bad_info_magic":
        struct.pack_into("<I", data, 0, 0)
    if damage == "bad_max_state":
        struct.pack_into("<I", data, 4, 2)
    if damage == "bad_unwind_state":
        struct.pack_into("<I", data, 32, 0)
    if two_actions:
        struct.pack_into("<II", data, 40, 0, 0)
        if damage == "second_map_wrong_state":
            struct.pack_into("<I", data, 40, 2)
        if damage == "second_map_wrong_action":
            struct.pack_into("<I", data, 44, 7)
        if damage == "second_map_nonzero_addend":
            struct.pack_into("<I", data, 44, 1)
        if damage == "second_map_duplicate_action":
            struct.pack_into("<I", data, 44, 0)
        if damage == "wrong_state_count":
            struct.pack_into("<I", data, 4, 3)
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
          object_cold_size=21, mode="default"):
    original_size = (spans[1][1] - spans[1][0]
                     if type(spans) is tuple and len(spans) == 2
                     and type(spans[1]) is tuple and len(spans[1]) == 2
                     and spans[1][1] - spans[1][0] in (18, 21, 26, 29, 32) else 21)
    image_data = _pe(damage=image_damage, cold_size=original_size, mode=mode)
    image = PeImage(image_data, hashlib.sha256(image_data).hexdigest())
    return bind_eh_regions(_coff(damage=object_damage, cold_size=object_cold_size, mode=mode), ROOT, image, spans)


def _linear_fixture(original_widths, object_widths=None, *, original_damage=None,
                    object_damage=None, truncate_original=False,
                    original_predecessors=None, object_predecessors=None,
                    return_artifacts=False):
    """Build complete independent PE/COFF unwind maps with supported actions."""
    if object_widths is None:
        object_widths = original_widths
    if original_predecessors is None:
        original_predecessors = tuple(
            0xFFFFFFFF if index == 0 else index - 1
            for index in range(len(original_widths)))
    if object_predecessors is None:
        object_predecessors = tuple(
            0xFFFFFFFF if index == 0 else index - 1
            for index in range(len(object_widths)))

    def action_bytes(width, target=None, pc=0):
        result = bytearray(b"\x8b\x4d\xf0")
        if width == 11:
            result.extend(b"\x83\xc1\x04")
        elif width == 14:
            result.extend(b"\x81\xc1\x4c\x14\x00\x00")
        elif width != 8:
            raise ValueError("fixture action width must be 8, 11, or 14")
        jump = len(result)
        result.extend(b"\xe9\0\0\0\0")
        if target is not None:
            struct.pack_into("<i", result, jump + 1, target - (pc + len(result)))
        return bytes(result), jump + 1

    def layout(widths):
        code, starts, branches = bytearray(), [], []
        for index, width in enumerate(widths):
            starts.append(len(code))
            action, branch = action_bytes(width, BASE + 0x1300 + 0x10 * index,
                                          BASE + 0x1100 + len(code))
            branches.append(len(code) + branch)
            code.extend(action)
        handler = len(code)
        info = BASE + 0x2000
        runtime = BASE + 0x13F0
        code.extend(b"\xb8" + struct.pack("<I", info) + b"\xe9")
        code.extend(struct.pack("<i", runtime - (BASE + 0x1100 + len(code) + 4)))
        return bytes(code), tuple(starts), tuple(branches), handler

    original_code, orig_starts, orig_branches, orig_handler = layout(original_widths)
    object_code, obj_starts, obj_branches, obj_handler = layout(object_widths)
    orig_cold = BASE + 0x1100
    info_address = BASE + 0x2000
    text = bytearray(0x400)
    text[:22] = bytes.fromhex("6a ff 68 00 00 00 00 64 a1 00 00 00 00 50 64 89 25 00 00 00 00 c3")
    struct.pack_into("<I", text, 3, orig_cold + orig_handler)
    if original_damage == "extra_root_cold_edge":
        text[22:28] = b"\xa1" + struct.pack("<I", orig_cold) + b"\xc3"
    elif original_damage == "direct_root_cold_edge":
        text[22] = 0xE9
        struct.pack_into("<i", text, 23, orig_cold - (BASE + 0x1000 + 27))
    text[0x100:0x100 + len(original_code)] = original_code
    info = bytearray(32 + 8 * len(original_widths))
    struct.pack_into("<8I", info, 0, 0x19930520, len(original_widths), info_address + 32,
                     0, 0, 0, 0, 0)
    for index, start in enumerate(orig_starts):
        struct.pack_into("<II", info, 32 + 8 * index,
                         original_predecessors[index],
                         orig_cold + start)
    if original_damage == "state":
        struct.pack_into("<I", info, 32 + 8 * (len(orig_starts) - 1), 9)
    elif original_damage == "action":
        struct.pack_into("<I", info, 36 + 8 * (len(orig_starts) - 1),
                         orig_cold + orig_starts[-1] + 1)

    root_relocs = [0x3003, 0x3100 + orig_handler + 1]
    if original_damage == "extra_root_cold_edge":
        root_relocs.append(0x3017)
    reloc_pages = {0x1000: root_relocs,
                   0x2000: [0x3008, *(0x3024 + 8 * i for i in range(len(original_widths)))]}
    if original_damage == "missing_reloc":
        reloc_pages[0x2000].pop()
    reloc = bytearray()
    for page, entries in reloc_pages.items():
        padded = entries if len(entries) % 2 == 0 else [*entries, 0]
        reloc.extend(struct.pack("<II", page, 8 + 2 * len(padded)))
        reloc.extend(struct.pack("<" + "H" * len(padded), *padded))
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
    struct.pack_into("<II", headers, optional + 96 + 5 * 8, 0x3000, len(reloc))
    sec_table = optional + 224
    rdata_size = len(info) - 1 if original_damage == "metadata_truncated" else len(info)
    sections = [(b".text", 0x1000, 0x400, bytes(text), 0x60000020),
                (b".rdata", 0x2000, 0x800, bytes(info[:rdata_size]), 0x40000040),
                (b".reloc", 0x3000, 0xC00, bytes(reloc), 0x40000040)]
    for i, (name, rva, raw, content, flags) in enumerate(sections):
        virtual_size = max(len(content), 0x200)
        raw_size = max(len(content), 0x200)
        if name == b".rdata" and original_damage == "metadata_truncated":
            virtual_size = raw_size = len(info) - 1
        struct.pack_into("<8sIIIIIIHHI", headers, sec_table + 40 * i,
                         name.ljust(8, b"\0"), virtual_size, rva,
                         raw_size, raw, 0, 0, 0, 0, flags)
    pe = headers + bytes(0xE00 - len(headers))
    for _name, _rva, raw, content, _flags in sections:
        pe[raw:raw + len(content)] = content
    image_bytes = bytes(pe)
    image = PeImage(image_bytes, hashlib.sha256(image_bytes).hexdigest())
    end = orig_cold + len(original_code) - (1 if truncate_original else 0)
    root_extra = (6 if original_damage == "extra_root_cold_edge" else
                  5 if original_damage == "direct_root_cold_edge" else 0)
    spans = ((BASE + 0x1000, BASE + 0x1016 + root_extra), (orig_cold, end))

    # Assemble an associated root, cold child, and complete FuncInfo COMDAT.
    data = bytearray(32 + 8 * len(object_widths))
    struct.pack_into("<8I", data, 0, 0x19930520, len(object_widths), 0, 0, 0, 0, 0, 0)
    for index in range(len(object_widths)):
        struct.pack_into("<II", data, 32 + 8 * index,
                         object_predecessors[index], 0)
    if object_damage == "state":
        struct.pack_into("<I", data, 32 + 8 * (len(obj_starts) - 1), 9)
    if object_damage == "extra_reloc":
        pass
    object_code = bytearray(object_code)
    for offset in (*obj_branches, obj_handler + 1, obj_handler + 6):
        struct.pack_into("<I", object_code, offset, 0)
    object_code = bytes(object_code)
    root_code = bytes.fromhex("6a ff 68 00 00 00 00 64 a1 00 00 00 00 50 64 89 25 00 00 00 00 c3")
    sections = [
        {"name": ".text", "data": root_code, "flags": 0x60501020,
         "selection": 2, "parent": 0, "relocs": [(3, "handler", 6)]},
        {"name": ".text$x", "data": object_code, "flags": 0x60501020,
         "selection": 5, "parent": 1, "relocs": []},
        {"name": ".xdata$x", "data": bytes(data), "flags": 0x40401040,
         "selection": 5, "parent": 1, "relocs": []},
    ]
    symbols, indices, strings = [], {}, bytearray(b"\0\0\0\0")

    def symbol_name(name):
        encoded = name.encode("ascii")
        if len(encoded) <= 8:
            return encoded.ljust(8, b"\0")
        offset = len(strings)
        strings.extend(encoded + b"\0")
        return b"\0\0\0\0" + struct.pack("<I", offset)

    def add(key, name, value, section, kind, storage, aux=None):
        indices[key] = len(symbols)
        symbols.append(symbol_name(name) + struct.pack("<IhHBB", value, section, kind,
                                                       storage, int(aux is not None)))
        if aux is not None:
            symbols.append(aux)

    for number, section in enumerate(sections, 1):
        aux = struct.pack("<IHHIHB3x", len(section["data"]), len(section["relocs"]),
                          0, 0, section["parent"], section["selection"])
        add(("section", number), section["name"], 0, number, 0, 3, aux)
    add("root", ROOT, 0, 1, 0x20, 2)
    add("handler", "$Lhandler", obj_handler, 2, 0, 6)
    if object_damage == "extra_root_child_edge":
        add("other_child", "$Lotherchild", 0, 2, 0, 6)
    for i, start in enumerate(obj_starts):
        add(("cleanup", i), f"$Lcleanup{i}",
            start + (1 if object_damage == "map_target" and i == len(obj_starts) - 1 else 0),
            2, 0, 6)
    add("info", "$Tinfo", 0, 3, 0, 3)
    add("unwind", "$Tunwind", 32, 3, 0, 6)
    for i in range(len(obj_starts)):
        destructor = ("?NotDestructor@@YAXXZ" if object_damage == "late_destructor"
                      and i == len(obj_starts) - 1 else f"??1Thing{i}@@QAE@XZ")
        add(("dtor", i), destructor, 0, 0, 0x20, 2)
    add("runtime", "___CxxFrameHandler", 0, 0, 0, 2)
    child_relocs = [(offset, ("dtor", i), 20) for i, offset in enumerate(obj_branches)]
    child_relocs.extend([(obj_handler + 1, "info", 6), (obj_handler + 6, "runtime", 20)])
    info_relocs = [(8, "unwind", 6)] + [
        (36 + 8 * i, ("cleanup", i), 6) for i in range(len(obj_starts))]
    if object_damage == "extra_reloc":
        child_relocs.append((1, ("dtor", 0), 20))
    if object_damage == "extra_root_child_edge":
        sections[0]["relocs"].append((7, "other_child", 6))
    sections[1]["relocs"], sections[2]["relocs"] = child_relocs, info_relocs
    # Refresh section auxiliaries after final relocation inventories are known.
    for number, section in enumerate(sections, 1):
        symbols[indices[("section", number)] + 1] = struct.pack(
            "<IHHIHB3x", len(section["data"]), len(section["relocs"]), 0, 0,
            section["parent"], section["selection"])
    headers_size = 20 + 40 * len(sections)
    raw_cursor = headers_size
    raw_offsets = []
    for section in sections:
        raw_offsets.append(raw_cursor)
        raw_cursor += len(section["data"])
    rel_cursor = raw_cursor
    rel_offsets = []
    for section in sections:
        rel_offsets.append(rel_cursor if section["relocs"] else 0)
        rel_cursor += 10 * len(section["relocs"])
    headers_coff = b"".join(struct.pack(
        "<8sIIIIIIHHI", section["name"].encode().ljust(8, b"\0"), 0, 0,
        len(section["data"]), raw_offsets[i], rel_offsets[i], 0,
        len(section["relocs"]), 0, section["flags"])
        for i, section in enumerate(sections))
    raw_coff = bytearray(struct.pack("<HHIIIHH", 0x14C, len(sections), 0,
                                     rel_cursor + 0, len(symbols), 0, 0) + headers_coff)
    for section in sections:
        raw_coff.extend(section["data"])
    for section in sections:
        for offset, target, kind in section["relocs"]:
            raw_coff.extend(struct.pack("<IIH", offset, indices[target], kind))
    symbol_start = len(raw_coff)
    struct.pack_into("<I", strings, 0, len(strings))
    raw_coff.extend(b"".join(symbols))
    struct.pack_into("<I", raw_coff, 8, symbol_start)
    raw_coff.extend(strings)
    if return_artifacts:
        return image_bytes, bytes(raw_coff), spans
    return bind_eh_regions(bytes(raw_coff), ROOT, image, spans)


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


@pytest.mark.parametrize("metadata", [None, "local_target_debug_child"])
def test_binds_two_action_funcinfo_map_and_exposes_both_actions(metadata):
    root, cold = _bind(spans=TWO_SPANS, object_cold_size=32, object_damage=metadata)
    assert root.ownership.cleanup_offsets == (0, 8)
    assert root.ownership.original_cleanups == (TWO_SPANS[1][0], TWO_SPANS[1][0] + 8)
    assert root.ownership.original_cleanup == TWO_SPANS[1][0]
    assert root.ownership.original_cleanup_targets == (BASE + 0x11F0, BASE + 0x11E0)
    assert cold.ownership.handler_offset == 22
    assert cold.code[:22] == bytes.fromhex(
        "8b 4d f0 e9 00 00 00 00 8b 4d f0 81 c1 4c 14 00 00 e9 00 00 00 00")


@pytest.mark.parametrize("widths,expected_extent", [
    ((14,), 24),
    ((8, 14, 11), 43),
    ((8, 14, 14), 46),
    ((8, 11, 8, 14), 51),
])
def test_binds_complete_linear_cleanup_chains_from_bounded_vc6_shapes(widths, expected_extent):
    root, cold = _linear_fixture(widths)
    assert len(cold.code) == expected_extent
    assert root.ownership.cleanup_offsets == tuple(
        sum(widths[:index]) for index in range(len(widths)))
    assert root.ownership.original_cleanups == tuple(
        BASE + 0x1100 + sum(widths[:index]) for index in range(len(widths)))
    assert len(root.ownership.cleanup_offsets) == len(widths)


def test_original_and_compiled_linear_action_inventories_are_independent():
    original = (8, 14, 8)
    compiled = (14,)
    root, cold = _linear_fixture(original, compiled)
    assert root.ownership.original_cleanups == (BASE + 0x1100, BASE + 0x1108,
                                                BASE + 0x1116)
    assert root.ownership.cleanup_offsets == (0,)
    assert len(root.ownership.original_cleanups) == 3
    assert len(cold.code) == 24
    assert root.ownership.original_predecessors == (0xFFFFFFFF, 0, 1)
    assert root.ownership.compiled_predecessors == (0xFFFFFFFF,)


def test_binds_backward_branched_predecessor_graphs_independently():
    original = (0xFFFFFFFF, 0, 0, 1)
    compiled = (0xFFFFFFFF, 0, 0, 1)
    root, cold = _linear_fixture(
        (8, 8, 8, 8), original_predecessors=original,
        object_predecessors=compiled)
    assert root.ownership.original_predecessors == original
    assert cold.ownership.original_predecessors == original
    assert root.ownership.compiled_predecessors == compiled
    assert cold.ownership.compiled_predecessors == compiled


@pytest.mark.parametrize("bad_graph", [
    (0xFFFFFFFF, 1, 0),  # self/forward edge at state 1
    (0xFFFFFFFF, 2, 1),  # forward edge at state 1 (would close a cycle)
    (0xFFFFFFFF, 0, 9),  # out-of-range predecessor
])
def test_rejects_original_non_backward_predecessor_graph(bad_graph):
    with pytest.raises(ValueError, match="predecessor"):
        _linear_fixture((8, 8, 8), original_predecessors=bad_graph)


@pytest.mark.parametrize("bad_graph", [
    (0xFFFFFFFF, 1, 0),
    (0xFFFFFFFF, 2, 1),
    (0xFFFFFFFF, 0, 9),
])
def test_rejects_compiled_non_backward_predecessor_graph(bad_graph):
    with pytest.raises(ValueError, match="predecessor"):
        _linear_fixture((8, 8, 8), object_predecessors=bad_graph)


@pytest.mark.parametrize("damage", ["state", "action", "missing_reloc"])
def test_rejects_corrupt_original_later_linear_actions(damage):
    with pytest.raises(ValueError):
        _linear_fixture((8, 14, 11), original_damage=damage)


@pytest.mark.parametrize("damage", ["state", "map_target", "extra_reloc",
                                    "extra_root_child_edge", "late_destructor"])
def test_rejects_corrupt_compiled_later_linear_actions(damage):
    with pytest.raises(ValueError):
        _linear_fixture((8, 14, 11), object_damage=damage)


def test_rejects_truncated_original_cold_extent_and_incomplete_action_table():
    with pytest.raises(ValueError):
        _linear_fixture((8, 14, 11), truncate_original=True)


def test_rejects_truncated_original_funcinfo_metadata():
    with pytest.raises(ValueError):
        _linear_fixture((8, 14, 11), original_damage="metadata_truncated")


def test_rejects_extra_original_root_absolute_edge_into_cold_child():
    with pytest.raises(ValueError, match="extra absolute edge"):
        _linear_fixture((8, 14, 11), original_damage="extra_root_cold_edge")


def test_rejects_original_direct_relative_edge_into_cold_child():
    with pytest.raises(ValueError, match="direct relative edge"):
        _linear_fixture((8, 14, 11), original_damage="direct_root_cold_edge")


def test_rejects_original_cleanup_count_above_explicit_resource_cap():
    with pytest.raises(ValueError, match="action cap"):
        _linear_fixture((8,) * 17)


def test_rejects_compiled_cleanup_count_above_explicit_resource_cap():
    with pytest.raises(ValueError, match="action cap"):
        _linear_fixture((8,), object_widths=(8,) * 17)


def test_accepts_maximum_supported_linear_action_count():
    root, cold = _linear_fixture((8,) * 16)
    assert len(root.ownership.original_cleanups) == 16
    assert len(cold.ownership.cleanup_offsets) == 16
    assert len(cold.code) == 16 * 8 + 10


def test_binds_second_action_to_undefined_external_destructor():
    root, cold = _bind(spans=TWO_SPANS, object_cold_size=32,
                       object_damage="second_external_destructor")
    assert root.ownership.cleanup_offsets == (0, 8)
    assert cold.ownership.original_cleanups == (TWO_SPANS[1][0], TWO_SPANS[1][0] + 8)


@pytest.mark.parametrize(("original_size", "object_size", "expected_second"), [
    (26, 26, 8), (29, 29, 8), (32, 32, 11),
    (26, 29, 8), (32, 26, 8),
])
def test_binds_map_bounded_two_lea_cleanup_pairs_independently(
        original_size, object_size, expected_second):
    spans = (SPANS[0], (SPANS[1][0], SPANS[1][0] + original_size))
    root, cold = _bind(spans=spans, object_cold_size=object_size, mode="lea_pair")
    assert root.ownership.original_cleanups == (spans[1][0], spans[1][0] + (8 if original_size != 32 else 11))
    assert root.ownership.cleanup_offsets == (0, expected_second)
    assert len(cold.code) == object_size
    assert cold.ownership.original_handler_offset == original_size - 10
    assert cold.ownership.handler_offset == object_size - 10


def test_original_action_inventory_is_independent_of_compiled_action_inventory():
    root, cold = _bind(spans=TWO_SPANS, object_cold_size=21)
    assert root.ownership.original_cleanups == (TWO_SPANS[1][0], TWO_SPANS[1][0] + 8)
    assert root.ownership.original_cleanup_targets == (BASE + 0x11F0, BASE + 0x11E0)
    assert root.ownership.cleanup_offsets == (0,)
    assert len(cold.code) == 21


@pytest.mark.parametrize("damage", [
    "second_map_wrong_state", "second_map_wrong_action", "second_map_duplicate_action",
    "second_map_wrong_target_offset", "second_map_nonzero_addend",
    "wrong_state_count", "missing_second_action_relocation",
    "extra_second_action_relocation", "second_action_shape", "second_action_jump",
    "original_second_map_wrong_state", "original_second_map_wrong_action",
    "original_second_map_duplicate_action", "original_wrong_state_count",
    "original_second_map_gap",
    "original_second_action_shape", "original_second_action_jump",
    "local_target_nonzero_addend", "local_target_wrong_name", "local_target_not_function",
    "local_target_overlapping_owner", "local_target_writable", "local_target_root_owned",
    "local_target_overlapping_static_owner", "local_target_executable_debug_child",
    "local_target_references_root",
    "local_target_debug_child_and_cold_reference",
    "local_target_overlapping_relocations", "local_target_out_of_bounds_relocation",
    "bad_registration_sentinel", "bad_registration_fs_read", "bad_registration_fs_write",
    "second_external_wrong_name", "second_external_nonzero_value",
    "second_external_nonfunction", "second_external_defined",
    "func_info_nonzero_addend",
])
def test_refuses_damaged_two_action_ownership_graph(damage):
    with pytest.raises(ValueError):
        _bind(spans=TWO_SPANS, object_cold_size=32,
              object_damage=damage if not damage.startswith("original_") else None,
              image_damage=damage if damage.startswith("original_") else None)


@pytest.mark.parametrize("damage", ["malformed_handler_prefix"])
def test_refuses_noncanonical_compiled_two_action_handler(damage):
    with pytest.raises(ValueError):
        _bind(spans=(SPANS[0], (SPANS[1][0], SPANS[1][0] + 29)),
              object_cold_size=29, mode="lea_pair", object_damage=damage)


def test_refuses_original_cross_action_branch():
    spans = (SPANS[0], (SPANS[1][0], SPANS[1][0] + 29))
    with pytest.raises(ValueError):
        _bind(spans=spans, object_cold_size=29, mode="lea_pair",
              image_damage="original_cross_action_branch")


def test_refuses_noncanonical_original_two_action_handler():
    spans = (SPANS[0], (SPANS[1][0], SPANS[1][0] + 29))
    with pytest.raises(ValueError):
        _bind(spans=spans, object_cold_size=29, mode="lea_pair",
              image_damage="original_handler_prefix")


@pytest.mark.parametrize("damage", [
    "original_missing_second_cleanup_reloc", "original_extra_second_cleanup_reloc",
])
def test_refuses_missing_or_extra_original_second_action_relocations(damage):
    with pytest.raises(ValueError):
        _bind(spans=TWO_SPANS, object_cold_size=32, image_damage=damage)


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
    "func_info_nonzero_addend",
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
