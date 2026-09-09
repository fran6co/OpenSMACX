"""Synthetic COFF declarations/imports, kept separate from code admission."""

import struct

import pytest

from match_recovery.coff_inventory import inventory_archive, inventory_object, short_import
from match_recovery.pe_coff import Coff, CoffRecords
from test_binary import coff_fixture
from test_toolchain_components import archive


def imported(symbol="_Paint@4", module="demo.dll", *, naming=3, kind=0, hint=7):
    payload = symbol.encode() + b"\0" + module.encode() + b"\0"
    return struct.pack("<HHHHIIHH", 0, 0xFFFF, 0, 0x14C, 123, len(payload), hint, kind | naming << 2) + payload


@pytest.mark.parametrize("kind,expected", [(0, "code"), (1, "data"), (2, "const")])
@pytest.mark.parametrize("naming,name,ordinal", [(0, None, 7), (1, "_Paint@4", None),
                                               (2, "Paint@4", None), (3, "Paint", None)])
def test_import_header_modes_preserve_identity_without_selection(kind, expected, naming, name, ordinal):
    row = short_import(imported(kind=kind, naming=naming))
    assert row["symbol"] == "_Paint@4" and row["module"] == "demo.dll"
    assert row["type"] == expected and row["name"] == name and row["ordinal"] == ordinal
    assert row["hint"] == (None if naming == 0 else 7)
    assert row["selection"] == "unknown"


@pytest.mark.parametrize("symbol,name", [("@Paint@4", "Paint"), ("?Paint@@X", "Paint"),
                                         ("Paint", "Paint"), ("__Paint@4", "_Paint")])
def test_undecoration_removes_at_most_one_prefix(symbol, name):
    assert short_import(imported(symbol))["name"] == name


@pytest.mark.parametrize("offset,value", [(0, 1), (2, 0), (4, 1), (6, 0x8664),
                                         (18, 3), (18, 4 << 2), (18, 1 << 5)])
def test_unsupported_header_fields_do_not_become_import_proof(offset, value):
    raw = bytearray(imported())
    struct.pack_into("<H", raw, offset, value)
    with pytest.raises(ValueError):
        short_import(bytes(raw))
    assert inventory_object(bytes(raw))["result"] == "unknown"


@pytest.mark.parametrize("raw", [b"", b"\0\0\xff\xff", imported()[:19], imported()[:-1],
    imported() + b"x", imported(""), imported(module=""), imported("_"),
    imported("\x01bad"), imported("é"), imported(module="one\0two.dll")])
def test_truncated_ambiguous_or_invalid_import_strings_refuse(raw):
    with pytest.raises(ValueError):
        short_import(raw)
    assert inventory_object(raw)["result"] == "unknown"


def test_size_cannot_hide_an_extra_string_or_trailing_payload():
    for suffix in (b"extra\0", b"\0", b"junk"):
        raw = bytearray(imported() + suffix)
        struct.pack_into("<I", raw, 12, len(raw) - 20)
        assert inventory_object(bytes(raw))["result"] == "unknown"


def common_object(size=4):
    raw = bytearray(coff_fixture())
    symbols = struct.unpack_from("<I", raw, 8)[0]
    struct.pack_into("<I", raw, symbols + 18 + 8, size)
    return bytes(raw)


def descriptor_object():
    # Traditional import-descriptor objects may use a section-named static
    # record without COMDAT auxiliary ownership metadata. This is not code
    # verification evidence, but it does not erase the external symbol table.
    raw = bytearray(common_object())
    symbols, count = struct.unpack_from("<II", raw, 8)
    at = symbols + count * 18
    section_name = b".text\0\0\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 0)
    raw[at:at] = section_name
    struct.pack_into("<I", raw, 12, count + 1)
    return bytes(raw)


def test_record_inventory_does_not_relax_comdat_verification():
    raw = descriptor_object()
    records = CoffRecords(raw)
    assert records.symbols[1].name == "_g" and records.symbols[1].value == 4
    with pytest.raises(ValueError, match="section definition"):
        Coff(raw)
    row = inventory_object(raw)
    assert row["result"] == "pass" and row["format"] == "coff"
    assert row["symbols"][1]["declaration"] == "common"
    with pytest.raises(ValueError):
        Coff(imported())


def optional_descriptor():
    raw = bytearray(descriptor_object())
    header = bytearray(224)
    struct.pack_into("<H", header, 0, 0x10B)
    struct.pack_into("<I", header, 92, 16)
    symbol_offset = struct.unpack_from("<I", raw, 8)[0]
    struct.pack_into("<I", raw, 8, symbol_offset + len(header))
    struct.pack_into("<H", raw, 16, len(header))
    for at in (20 + 20, 20 + 24):
        offset = struct.unpack_from("<I", raw, at)[0]
        if offset:
            struct.pack_into("<I", raw, at, offset + len(header))
    raw[20:20] = header
    return bytes(raw)


def test_import_descriptor_optional_header_is_metadata_not_function_evidence():
    raw = optional_descriptor()
    report = inventory_object(raw)
    assert report["result"] == "pass" and report["optional_header_size"] == 224
    assert report["symbols"][1]["name"] == "_g"
    with pytest.raises(ValueError, match="without an optional header"):
        Coff(raw)


@pytest.mark.parametrize("offset,format,value", [(20, "H", 0x20B), (16, "H", 40), (20 + 92, "I", 17)])
def test_damaged_optional_record_headers_cannot_be_skipped_unchecked(offset, format, value):
    raw = bytearray(optional_descriptor())
    struct.pack_into("<" + format, raw, offset, value)
    assert inventory_object(bytes(raw))["result"] == "unknown"


@pytest.mark.parametrize("section,size,expected", [(0, 0, "undefined"), (0, 8, "common"),
                                                 (1, 0, "defined"), (-1, 20, "absolute")])
def test_external_declarations_are_not_mistaken_for_selected_allocations(section, size, expected):
    raw = bytearray(common_object(size))
    at = struct.unpack_from("<I", raw, 8)[0] + 18
    struct.pack_into("<h", raw, at + 12, section)
    report = inventory_object(bytes(raw))
    assert report["result"] == "pass"
    assert report["symbols"][1]["declaration"] == expected


def test_linker_directives_stay_uninterpreted_in_inventory():
    text = b' /DEFAULTLIB:"runtime" /ALTERNATENAME:_a=_b '
    raw = bytearray(coff_fixture(text))
    raw[20:28] = b".drectve"
    # The fixture fills alignment with INT3; a real directive section has text.
    size = struct.unpack_from("<I", raw, 20 + 16)[0]
    raw[60 + len(text):60 + size] = b" " * (size - len(text))
    report = inventory_object(bytes(raw))
    assert report["result"] == "pass"
    assert report["directives"][0]["interpretation"] == "unknown"
    assert "/ALTERNATENAME" in report["directives"][0]["text"]


def weak_object(mode=3):
    raw = bytearray(coff_fixture())
    symbols, count = struct.unpack_from("<II", raw, 8)
    # _g weakly names _f as its fallback. The weak record owns an auxiliary
    # table slot; it does not create a new defined/common symbol.
    raw[symbols + 18 + 16:symbols + 18 + 18] = bytes([105, 1])
    raw[symbols + count * 18:symbols + count * 18] = struct.pack("<II10x", 0, mode)
    struct.pack_into("<I", raw, 12, count + 1)
    return bytes(raw)


@pytest.mark.parametrize("mode", [1, 2, 3])
def test_weak_fallbacks_are_records_not_selected_definitions(mode):
    report = inventory_object(weak_object(mode))
    assert report["result"] == "pass"
    assert report["symbols"][1]["declaration"] == "weak"
    assert report["symbols"][1]["fallback"] == "_f"
    assert report["symbols"][1]["search_mode"] == mode
    assert inventory_object(weak_object(4))["result"] == "unknown"


def test_unspecified_machine_metadata_never_becomes_i386_function_evidence():
    raw = bytearray(weak_object())
    struct.pack_into("<H", raw, 0, 0)
    report = inventory_object(bytes(raw))
    assert report["result"] == "pass" and report["machine"] == 0
    assert report["symbols"][1]["declaration"] == "weak"
    with pytest.raises(ValueError, match="i386 COFF object"):
        Coff(bytes(raw))


def test_archive_repeated_import_names_are_enumerated_not_selected_or_collapsed():
    raw = archive([("demo.dll/", imported()), ("demo.dll/", imported("_Other@8")),
                   ("defs.obj/", common_object())])
    report = inventory_archive(raw)
    assert report["result"] == "pass"
    assert report["member_count"] == 3 and report["format_counts"] == {"coff": 1, "short_import": 2}
    assert [(row["member"], row["occurrence"]) for row in report["members"]] == [
        ("demo.dll", 0), ("demo.dll", 1), ("defs.obj", 0)]
    assert report["common_requests"] == [{"member": "defs.obj", "occurrence": 0, "symbol": "_g", "size": 4}]
    assert report["allocation_verification"] == report["link_selection"] == "unknown"
    assert not report["persisted"] and not report["ready_for_collection"]


def test_unknown_member_prevents_complete_inventory_without_erasing_known_requests():
    raw = archive([("defs.obj/", common_object()), ("bad.obj/", b"not COFF")])
    report = inventory_archive(raw)
    assert report["result"] == "unknown" and report["common_requests"]
    assert report["unsupported_members"][0]["member"] == "bad.obj"
    assert inventory_archive(archive([]))["result"] == "unknown"


def test_damaged_symbol_table_cannot_yield_a_complete_inventory():
    raw = bytearray(common_object())
    struct.pack_into("<I", raw, 12, 100000)
    assert inventory_object(bytes(raw))["result"] == "unknown"
    assert inventory_object(common_object()[:-1])["result"] == "unknown"


def test_linked_component_report_exposes_inventory_without_admitting_common_data():
    from test_component_link import inputs, verify
    report = verify(*inputs(symbol_damage="common"))
    assert report["result"] == report["archive_inventory"]["result"] == "pass"
    assert report["archive_inventory"]["common_requests"][0]["size"] == 4
    assert report["storage_verification"] == "unknown" and not report["ready_for_collection"]
