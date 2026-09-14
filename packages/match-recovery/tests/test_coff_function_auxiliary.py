"""Physical standard COFF function auxiliaries, not link-time selection proof."""

from dataclasses import replace
import struct

import pytest

from match_recovery.coff_comdat import comdat_key
from match_recovery.pe_coff import Coff, CoffRecords


def fixture(*, functions=1, lines=True):
    count = functions
    raw_start = 20 + count * 40
    line_start = raw_start + count * 16
    symbol_start = line_start + count * (12 if lines else 0)
    raw = bytearray(symbol_start)
    struct.pack_into("<HHIIIHH", raw, 0, 0x14C, count, 0, symbol_start, count * 9, 0, 0)
    records = []

    def symbol(name, value, section, kind, storage, aux=0):
        return name.ljust(8, b"\0") + struct.pack("<IhHBB", value, section, kind, storage, aux)

    for i in range(count):
        section, base = i + 1, i * 9
        ptr = line_start + i * 12 if lines else 0
        header = 20 + i * 40
        raw[header:header + 8] = b".text\0\0\0"
        struct.pack_into("<IIIIIIHHI", raw, header + 8, 0, 0, 16,
                         raw_start + i * 16, 0, ptr, 0, 2 if lines else 0, 0x60501020)
        raw[raw_start + i * 16:raw_start + (i + 1) * 16] = b"\x31\xc0\xc3" + b"\xcc" * 13
        if lines:
            struct.pack_into("<IHIH", raw, ptr, base + 2, 0, 1, 1)
        records.extend([
            symbol(b".text", 0, section, 0, 3, 1),
            struct.pack("<IHHIHB3x", 16, 0, 2 if lines else 0, 0, 0, 1),
            symbol(f"_f{i}".encode(), 0, section, 32, 2, 1),
            struct.pack("<IIIIH", base + 4, 3, ptr, base + 11 if i + 1 < count else 0, 0),
            symbol(b".bf", 0, section, 0, 101, 1),
            struct.pack("<IH6xIH", 0, 7, base + 13 if i + 1 < count else 0, 0),
            symbol(b".lf", 2 if lines else 0, section, 0, 101),
            symbol(b".ef", 3, section, 0, 101, 1),
            struct.pack("<IH6xIH", 0, 8, 0, 0),
        ])
    return bytes(raw) + b"".join(records) + struct.pack("<I", 4)


def mutate_symbol(data, index, offset, fmt, value):
    data = bytearray(data)
    start = struct.unpack_from("<I", data, 8)[0]
    struct.pack_into(fmt, data, start + index * 18 + offset, value)
    return bytes(data)


@pytest.mark.parametrize("functions", [1, 2, 4])
@pytest.mark.parametrize("lines", [False, True])
def test_standard_function_auxiliary_binds_original_symbol_without_changing_extent(functions, lines):
    raw = fixture(functions=functions, lines=lines)
    obj = Coff(raw)
    assert obj.raw_data is raw
    for i in range(functions):
        assert comdat_key(obj, i + 1) == obj.symbols[i * 9 + 2]
        assert len(obj.subject(obj.symbols[i * 9 + 2])[0]) == 16


@pytest.mark.parametrize("index,offset,fmt,value", [
    (3, 0, "<I", 0), (3, 0, "<I", 3), (3, 0, "<I", 7),
    (3, 4, "<I", 0), (3, 4, "<I", 17),
    (3, 8, "<I", 0), (3, 8, "<I", 77),
    (3, 12, "<I", 2), (3, 12, "<I", 4), (3, 16, "<H", 1),
    (4, 8, "<I", 1), (4, 14, "<H", 32), (4, 16, "<B", 3),
    (5, 0, "<I", 1), (5, 4, "<H", 0), (5, 6, "<H", 1),
    (5, 12, "<I", 4), (5, 16, "<H", 1),
    (7, 8, "<I", 2), (7, 16, "<B", 3),
    (8, 0, "<I", 1), (8, 4, "<H", 0), (8, 6, "<H", 1),
    (8, 12, "<I", 4), (8, 16, "<H", 1),
    (2, 14, "<H", 0),
])
def test_function_auxiliary_and_markers_cannot_borrow_wrong_fields(index, offset, fmt, value):
    with pytest.raises(ValueError):
        comdat_key(Coff(mutate_symbol(fixture(), index, offset, fmt, value)), 1)


@pytest.mark.parametrize("index,offset,value", [(3, 12, 0), (3, 12, 13),
    (3, 12, 20), (5, 12, 0), (5, 12, 11), (12, 0, 4)])
def test_next_function_and_begin_links_are_exact_not_any_real_symbol(index, offset, value):
    with pytest.raises(ValueError):
        comdat_key(Coff(mutate_symbol(fixture(functions=2), index, offset, "<I", value)), 1)


@pytest.mark.parametrize("damage", ["section_pointer", "section_count", "definition_count",
    "first_index", "first_line", "other_function", "outside_code", "truncated", "header_alias",
    "raw_alias", "symbol_alias", "other_table_alias"])
def test_physical_line_table_damages_refuse(damage):
    raw = bytearray(fixture(functions=2))
    ptr = struct.unpack_from("<I", raw, 48)[0]
    start = struct.unpack_from("<I", raw, 8)[0]
    if damage == "section_pointer": struct.pack_into("<I", raw, 48, ptr + 6)
    elif damage == "section_count": struct.pack_into("<H", raw, 54, 1)
    elif damage == "definition_count": struct.pack_into("<H", raw, start + 18 + 6, 1)
    elif damage == "first_index": struct.pack_into("<I", raw, ptr, 3)
    elif damage == "first_line": struct.pack_into("<H", raw, ptr + 4, 1)
    elif damage == "other_function": struct.pack_into("<H", raw, ptr + 10, 0)
    elif damage == "outside_code": struct.pack_into("<I", raw, ptr + 6, 3)
    else:
        target = {"truncated": len(raw) - 6, "header_alias": 40,
                  "raw_alias": 100, "symbol_alias": start,
                  "other_table_alias": ptr + 12}[damage]
        struct.pack_into("<I", raw, 48, target)
        struct.pack_into("<I", raw, start + 3 * 18 + 8, target)
    with pytest.raises(ValueError):
        comdat_key(Coff(bytes(raw)), 1)


@pytest.mark.parametrize("damage", ["raw", "auxiliary", "removed_auxiliary", "section", "symbol", "bool", "mutable"])
def test_original_bytes_and_mutable_parsed_metadata_must_remain_coherent(damage):
    obj = Coff(fixture())
    if damage == "raw": obj.raw_data = fixture(lines=False)
    elif damage == "auxiliary": obj.auxiliary[3] = bytes(18)
    elif damage == "removed_auxiliary": del obj.auxiliary[2]
    elif damage == "section": obj.sections = (replace(obj.sections[0], data=bytes(16)),)
    elif damage == "symbol": obj.symbols[6] = replace(obj.symbols[6], value=1)
    elif damage == "bool": obj.symbols[4] = replace(obj.symbols[4], value=False)
    elif damage == "mutable": obj.raw_data = bytearray(obj.raw_data)
    with pytest.raises(ValueError, match="physical COFF"):
        comdat_key(obj, 1)


def test_record_parser_snapshots_mutable_byte_input():
    raw = bytearray(fixture())
    obj = CoffRecords(raw)
    before = obj.raw_data
    raw[0] = 0
    assert type(before) is bytes and obj.raw_data == fixture()
