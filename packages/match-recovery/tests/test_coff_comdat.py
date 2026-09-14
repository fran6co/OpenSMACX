"""Physical key/order metadata cannot be replaced by a convenient symbol name."""

from dataclasses import replace
import struct

import pytest

from match_recovery.coff_comdat import comdat_key, comdat_selection
from match_recovery.pe_coff import Coff, CoffRecords
from test_linked_fpo import object_bytes


def object_with_key(*, selection=2, before=(), after=(), key_type=32, key_value=0,
                    key_storage=2, key_name=b"_f", parent=0):
    raw = bytearray(object_bytes())
    start, count = struct.unpack_from("<II", raw, 8)
    records = [bytes(raw[start + index * 18:start + (index + 1) * 18]) for index in range(count)]
    auxiliary = bytearray(records[1])
    struct.pack_into("<HB", auxiliary, 12, parent, selection)
    records[1] = bytes(auxiliary)
    records[2] = key_name.ljust(8, b"\0") + struct.pack("<IhHBB", key_value, 1, key_type, key_storage, 0)
    # The FPO relocation is not inspected by this parser-only control. Preserve
    # its valid symbol index when inserting section labels before the key.
    records = records[:2] + list(before) + records[2:3] + list(after) + records[3:]
    struct.pack_into("<I", raw, 12, len(records))
    return bytes(raw[:start]) + b"".join(records) + struct.pack("<I", 4)


def symbol(name=b"$label", *, section=1, storage=3):
    return name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, section, 0, storage, 0)


@pytest.mark.parametrize("selection", [1, 2, 3, 4, 6])
@pytest.mark.parametrize("kind", [0, 32])
def test_exact_independent_key_and_policy_are_observations(selection, kind):
    obj = Coff(object_with_key(selection=selection, key_type=kind))
    key = comdat_key(obj, 1)
    assert key == obj.symbols[2] and key.name == "_f" and key.type == kind
    assert comdat_selection(obj, 1) == selection
    assert not hasattr(key, "winner") and not hasattr(key, "result")
    assert comdat_selection(obj, 2) == 5


def test_first_and_second_are_by_section_membership_not_global_adjacency():
    obj = Coff(object_with_key(before=(symbol(b"_undef", section=0, storage=2),)))
    assert comdat_key(obj, 1).index == 3


def test_later_public_does_not_override_the_actual_second_symbol():
    obj = Coff(object_with_key(after=(symbol(b"_other", storage=2),)))
    assert comdat_key(obj, 1).name == "_f"


def test_convenient_later_external_cannot_replace_a_local_second_symbol():
    obj = Coff(object_with_key(before=(symbol(),)))
    with pytest.raises(ValueError, match="second"):
        comdat_key(obj, 1)


@pytest.mark.parametrize("options", [{"key_storage": 3}, {"key_type": 16},
    {"key_value": 1}, {"key_name": b""}, {"key_name": b".text"}, {"parent": 2}])
def test_unsupported_key_forms_and_nonassociative_parent_refuse(options):
    with pytest.raises(ValueError):
        comdat_key(Coff(object_with_key(**options)), 1)


def test_key_before_section_definition_does_not_borrow_later_auxiliary():
    raw = bytearray(object_with_key())
    start, = struct.unpack_from("<I", raw, 8)
    definition = bytes(raw[start:start+36])
    key = bytes(raw[start+36:start+54])
    raw[start:start+54] = key + definition
    struct.pack_into("<I", raw, start - 6, 0)
    obj = Coff(bytes(raw))
    with pytest.raises(ValueError, match="first"):
        comdat_key(obj, 1)


def test_association_cannot_mint_an_independent_external_key():
    obj = Coff(object_with_key())
    with pytest.raises(ValueError, match="independent"):
        comdat_key(obj, 2)


@pytest.mark.parametrize("number", [False, True, 0, -1, 3, 1.0, "1", None])
def test_section_selection_requires_exact_bounded_integer(number):
    with pytest.raises(ValueError):
        comdat_key(Coff(object_with_key()), number)


def test_inventory_parser_does_not_gain_ownership_from_key_helper():
    with pytest.raises(ValueError, match="strict COFF"):
        comdat_key(CoffRecords(object_with_key()), 1)


def test_dictionary_reordering_cannot_change_physical_symbol_order():
    obj = Coff(object_with_key())
    key = comdat_key(obj, 1)
    obj.symbols = dict(reversed(list(obj.symbols.items())))
    assert comdat_key(obj, 1) == key


def test_changed_section_definition_metadata_is_not_reused():
    obj = Coff(object_with_key())
    assert comdat_selection(obj, 1) == 2
    obj.symbols[0] = replace(obj.symbols[0], value=1)
    with pytest.raises(ValueError, match="first"):
        comdat_selection(obj, 1)
