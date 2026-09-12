"""Real parsed PE import-slot controls, not inferred decoration fixtures."""

from dataclasses import replace
import struct

import pytest

from match_recovery.linked_imports import import_slots, resolve_import_slot
from match_recovery.msvc_link_map import Public
from match_recovery.pe_coff import Symbol
from test_imports import image_imports, pe, with_second_import
from test_binary import pe_fixture


def data_image(**kwargs):
    raw = bytearray(image_imports(**kwargs))
    # The shared import-parser fixture places all bytes in a code section.
    # This helper exercises the additional linked-slot storage boundary.
    struct.pack_into("<I", raw, 0x178 + 36, 0xC0000040)
    return bytes(raw)


def subject():
    return Symbol(1, "arbitrary_pointer", 0, 0, 0x20, 2)


def placement():
    return Public("arbitrary_pointer", 1, 0x160, 0x401160, False, "import.obj")


@pytest.mark.parametrize("lookup,bound", [(True, False), (True, True), (False, False)])
def test_identity_comes_from_lookup_not_symbol_or_prebound_address(lookup, bound):
    slots = import_slots(pe(data_image(lookup=lookup, bound=bound)))
    assert resolve_import_slot(subject(), placement(), 0, slots) == {
        "module": "demo.dll", "name": "Paint", "ordinal": None,
        "address": 0x401160, "symbol": "arbitrary_pointer"}


def test_ordinal_import_is_not_an_invented_export_name():
    slots = import_slots(pe(data_image(ordinal=True)))
    assert resolve_import_slot(subject(), placement(), 0, slots)["ordinal"] == 7
    assert resolve_import_slot(subject(), placement(), 0, slots)["name"] is None


def test_unrecognized_address_is_not_guessed_from_import_spelling():
    slots = import_slots(pe(data_image()))
    assert resolve_import_slot(replace(subject(), name="__imp_Paint"),
                               replace(placement(), address=0x401164), 0, slots) is None


@pytest.mark.parametrize("changes", [{"storage": 3}, {"storage": 105}, {"section": 1},
                                     {"section": -1}, {"value": 4}])
def test_nonordinary_symbols_cannot_borrow_import_pointer_role(changes):
    with pytest.raises(ValueError, match="ordinary undefined external"):
        resolve_import_slot(replace(subject(), **changes), placement(), 0, import_slots(pe(data_image())))


@pytest.mark.parametrize("changes", [{"function": True}, {"segment": 0}, {"segment": 2},
                                     {"symbol": "another_pointer"}])
def test_placement_must_be_the_same_nonfunction_symbol_and_segment(changes):
    with pytest.raises(ValueError, match="non-function map placement"):
        resolve_import_slot(subject(), replace(placement(), **changes), 0, import_slots(pe(data_image())))


@pytest.mark.parametrize("addend", [-4, -1, 1, 4, 2**32, False, True, 0.0, None])
def test_pointer_addends_cannot_escape_or_offset_the_loader_slot(addend):
    with pytest.raises(ValueError, match="zero addend"):
        resolve_import_slot(subject(), placement(), addend, import_slots(pe(data_image())))


@pytest.mark.parametrize("flags", [0x80000040, 0x60000040, 0x40000020, 0xE0000020])
def test_import_storage_must_be_readable_and_nonexecutable(flags):
    raw = bytearray(data_image())
    struct.pack_into("<I", raw, 0x178 + 36, flags)
    with pytest.raises(ValueError, match="readable non-executable"):
        import_slots(pe(bytes(raw)))


@pytest.mark.parametrize("site", [0x115D, 0x115E, 0x115F, 0x1160, 0x1161, 0x1162, 0x1163])
def test_base_relocations_cannot_overlap_any_byte_of_the_slot(site):
    raw = bytearray(data_image())
    # The fixture's existing HIGHLOW entry follows its block header at 0x300.
    struct.pack_into("<H", raw, 0x308, 0x3000 | (site & 0xFFF))
    with pytest.raises(ValueError, match="relocation"):
        import_slots(pe(bytes(raw)))


def test_duplicate_export_identity_in_separate_slots_remains_distinct():
    raw = with_second_import(data_image(), "DEMO.dll")
    slots = import_slots(pe(raw))
    assert set(slots) == {0x401160, 0x401210}
    other = replace(placement(), address=0x401210, offset=0x210)
    assert resolve_import_slot(subject(), other, 0, slots)["address"] == 0x401210


def test_malformed_imports_do_not_return_partial_indexes():
    with pytest.raises(ValueError, match="bound.*lookup"):
        import_slots(pe(data_image(lookup=False, bound=True)))


def test_import_parser_refuses_overlapping_slots():
    raw = bytearray(with_second_import(data_image(bound=True), "OTHER.dll"))
    struct.pack_into("<I", raw, 0x334 + 4, 123)
    struct.pack_into("<I", raw, 0x334 + 16, 0x1162)
    with pytest.raises(ValueError, match="overlapping.*import"):
        import_slots(pe(bytes(raw)))


def test_returned_identity_is_not_a_mutable_alias_of_the_index():
    slots = import_slots(pe(data_image()))
    result = resolve_import_slot(subject(), placement(), 0, slots)
    result["module"] = "different.dll"
    assert slots[0x401160]["module"] == "demo.dll"


def test_absent_imports_are_an_empty_index():
    assert import_slots(pe(pe_fixture())) == {}


def test_readonly_slot_is_supported_without_inferring_loader_write_policy():
    raw = bytearray(data_image())
    struct.pack_into("<I", raw, 0x178 + 36, 0x40000040)
    slots = import_slots(pe(bytes(raw)))
    assert resolve_import_slot(replace(subject(), type=0), placement(), 0, slots)["name"] == "Paint"


def test_unbacked_iat_cannot_borrow_lookup_table_identity():
    raw = bytearray(data_image())
    struct.pack_into("<I", raw, 0x320 + 16, 0x1200)
    with pytest.raises(ValueError, match="backed"):
        import_slots(pe(bytes(raw)))


def test_slot_straddling_a_raw_section_boundary_refuses():
    raw = bytearray(data_image(bound=True))
    struct.pack_into("<I", raw, 0x320 + 16, 0x11FE)
    with pytest.raises(ValueError, match="backed"):
        import_slots(pe(bytes(raw)))
