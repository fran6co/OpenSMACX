"""Source-owned TU-local member groups bind only to their real static parent."""
from __future__ import annotations

import hashlib
import struct

import pytest

from match_recovery.application_binary import check_compiled
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Result, Span
from match_recovery.object_source import local_object_targets, parse_objects
from match_recovery.pe_coff import Coff, PeImage
from test_binary import pe_fixture
from test_codeview import record


FUNCTION = 0x401000
PARENT = 0x401100
UNIT = "src/owner.cpp"


def _section(name, data, relocs=(), flags=0x40000040):
    return (name.encode().ljust(8, b"\0")
            + struct.pack("<IIIIIIHHI", 0, 0, len(data), 0, 0, 0,
                          len(relocs), 0, flags))


def _section_aux(length, relocations, number, selection):
    return struct.pack("<IHHIHB", length, relocations, 0, 0, number, selection) + bytes(3)


def _nested_parent_types(*, parent_size=20, duplicate_storage=False):
    """Parent::storage_.bytes_[1] and Parent::tail_, with real CodeView types."""
    array = record(0x1003, struct.pack("<IIH", 0x20, 0x75, 8) + b"\0")
    storage_fields = (struct.pack("<HHIH", 0x1405, 0, 0x1000, 0)
                      + b"\x06bytes_")
    storage = record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1001, 0, 0, 8)
                     + b"\x07Storage")
    parent_fields = (struct.pack("<HHIH", 0x1405, 0, 0x1002, 4)
                     + b"\x08storage_"
                     + struct.pack("<HHIH", 0x1405, 0, 0x74, 16)
                     + b"\x05tail_")
    count = 2
    if duplicate_storage:
        parent_fields += struct.pack("<HHIH", 0x1405, 0, 0x1002, 8) + b"\x08storage_"
        count += 1
    parent = record(0x1005, struct.pack("<HHIIIH", count, 0, 0x1003, 0, 0,
                                         parent_size) + b"\x06Parent")
    return struct.pack("<I", 2) + array + record(0x1203, storage_fields) + storage \
        + record(0x1203, parent_fields) + parent


def _coff(parent_base=PARENT, *, parent_storage=3, parent_size=20,
          debug_types=True, debug_symbols=True, debug_parent_relocs=True,
          wrong_debug_relocs=False, duplicate_storage=False,
          parent_symbol="_parent"):
    # The PE has the resolved absolute address. The COFF immediate is the
    # relocation addend; the TU-local symbol mapping supplies the parent base.
    code = b"\xa1" + struct.pack("<I", 0) + b"\xc3" + b"\xcc" * 10
    data = bytes(20)
    types = _nested_parent_types(parent_size=parent_size,
                                 duplicate_storage=duplicate_storage)
    symbol_stream = (struct.pack("<I", 2)
                     + record(0x1007, struct.pack("<IIH", 0x1004, 0, 0)
                              + bytes([len(parent_symbol)]) + parent_symbol.encode()))
    sections = [
        (".text$A", code, ((1, 3, 6),), 0x60501020),
        (".data", data, (), 0xC0300040),
    ]
    if debug_types:
        sections.append((".debug$T", types, (), 0x40000040))
    if debug_symbols:
        relocs = (((12, 2, 11), (16, 2, 10)) if wrong_debug_relocs else
                  ((12, 3, 11), (16, 3, 10)) if debug_parent_relocs else ())
        sections.append((".debug$S", symbol_stream, relocs, 0x40000040))

    symbols = (
        b".text$A\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 1)
        + _section_aux(len(code), 1, 1, 2)
        + b"_caller\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
        + parent_symbol.encode().ljust(8, b"\0")
        + struct.pack("<IhHBB", 0, 2, 0, parent_storage, 0)
    )
    header_size = 20 + 40 * len(sections)
    cursor = header_size
    raw_rows = []
    for name, raw, relocs, flags in sections:
        raw_at = cursor if raw else 0
        raw_rows.append((name, raw, relocs, flags, raw_at))
        cursor += len(raw)
    section_headers = []
    reloc_parts = []
    for name, raw, relocs, flags, raw_at in raw_rows:
        reloc_at = cursor if relocs else 0
        reloc_bytes = b"".join(struct.pack("<IIH", *reloc) for reloc in relocs)
        reloc_parts.append(reloc_bytes)
        cursor += len(reloc_bytes)
        section_headers.append(
            name.encode().ljust(8, b"\0")
            + struct.pack("<IIIIIIHHI", 0, 0, len(raw), raw_at, reloc_at,
                          0, len(relocs), 0, flags))

    # COFF symbol indexes are stable: caller 0, local parent 1. The debug
    # records use SECTION and SECREL relocations against that exact parent.
    # The text section symbol plus its auxiliary occupies two COFF table
    # indices; caller and parent follow at indexes 2 and 3 respectively.
    header = struct.pack("<HHIIIHH", 0x14C, len(sections), 0, cursor, 4, 0, 0)
    return Coff(header + b"".join(section_headers)
                + b"".join(row[1] for row in raw_rows)
                + b"".join(reloc_parts) + symbols + struct.pack("<I", 4))


def _image(target=PARENT):
    raw = pe_fixture(b"\xa1" + struct.pack("<I", target) + b"\xc3",
                     absolute_site=1)
    pin = hashlib.sha256(raw).hexdigest()
    return PeImage(raw, pin), pin


def _member_bindings(pin, *, unit=UNIT, base=PARENT, second_base=None,
                     anchor_base=None, bounded=False):
    second_base = base if second_base is None else second_base
    rows = [f"// RECOVERY_OBJECT: v3 0x{base + 5:08X} _nested_byte member "
            "_parent storage_.bytes_[1]",
            f"// RECOVERY_OBJECT: v3 0x{second_base + 16:08X} _tail member "
            "_parent tail_"]
    if bounded:
        rows.append(f"// RECOVERY_OBJECT: v3 0x{base + 5:08X} "
                    f"0x{base + 6:08X} _bounded 0x00401500 member _parent "
                    "storage_.bytes_[1]")
    if anchor_base is not None:
        rows.append(f"// RECOVERY_OBJECT: v1 0x{anchor_base:08X} _parent")
    return parse_objects("\n".join(rows) + "\n", unit, pin, address_space="pe:va")


def _function(pin, *, unit=UNIT):
    return FunctionBinding(Origin(pin, "pe:va", FUNCTION),
                           (Span(FUNCTION, FUNCTION + 6),), "_caller", unit)


def _target_rows(bindings, objects):
    return local_object_targets(bindings, objects)


def test_nested_member_group_derives_static_parent_and_resolves_actual_code_reference():
    image, pin = _image()
    bindings = _member_bindings(pin)
    obj = _coff()
    targets, reports = _target_rows(bindings, {UNIT: obj})

    assert targets == {(UNIT, "_parent"): PARENT}
    assert {(row["symbol"], row["member_path"], row["member_offset"],
             row["original_parent"], row["parent_layout_size"])
            for row in reports} == {
                ("_nested_byte", "storage_.bytes_[1]", 5, "0x00401100", 20),
                ("_tail", "tail_", 16, "0x00401100", 20),
            }
    assert all(row["definition"] == Result.PASS
               and row["linkage"] == "translation_unit_local"
               and row["extent"] == "unbounded" for row in reports)

    # Exercise the same public pipeline used for compiled candidates. The
    # function's absolute target resolves through this member group's parent.
    function_rows, *_ = check_compiled(
        [_function(pin)], bindings, (), {UNIT: obj}, image, {})
    assert function_rows[0]["result"] == Result.PASS

    wrong_target, = check_objects(
        [_function(pin)], {UNIT: obj}, image,
        local_data_targets={(UNIT, "_parent"): PARENT + 4})
    assert wrong_target["result"] == Result.FAIL


def test_consistent_direct_parent_anchor_confirms_group_base_and_reference():
    image, pin = _image()
    bindings = _member_bindings(pin, anchor_base=PARENT)
    obj = _coff()
    targets, reports = _target_rows(bindings, {UNIT: obj})

    assert targets == {(UNIT, "_parent"): PARENT}
    assert len(reports) == 3
    assert all(row["definition"] == Result.PASS for row in reports)
    row, = check_objects([_function(pin)], {UNIT: obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.PASS


@pytest.mark.parametrize("damage", ["missing_types", "missing_debug_symbol",
                                     "wrong_parent_relocation", "nonstatic_parent",
                                     "missing_path", "parent_too_small",
                                     "duplicate_member", "missing_owner"])
def test_member_target_requires_one_fresh_local_parent_and_complete_codeview_path(damage):
    image, pin = _image()
    bindings = _member_bindings(pin)
    obj = _coff(debug_types=damage != "missing_types",
                debug_symbols=damage != "missing_debug_symbol",
                wrong_debug_relocs=damage == "wrong_parent_relocation",
                debug_parent_relocs=damage != "wrong_parent_relocation",
                parent_storage=2 if damage == "nonstatic_parent" else 3,
                parent_size=4 if damage == "parent_too_small" else 20,
                duplicate_storage=damage == "duplicate_member")
    if damage == "missing_path":
        bindings = parse_objects(
            "// RECOVERY_OBJECT: v3 0x00401105 _bad member _parent "
            "storage_.absent_\n", UNIT, pin, address_space="pe:va")
    objects = {} if damage == "missing_owner" else {UNIT: obj}
    targets, reports = _target_rows(bindings, objects)

    assert targets == {}
    assert reports and all(row["definition"] == Result.UNKNOWN for row in reports)
    if damage == "missing_owner":
        row, = check_objects([_function(pin)], objects, image,
                             local_data_targets={(UNIT, "_parent"): PARENT})
        assert row["result"] != Result.PASS


def test_all_group_members_must_imply_one_base_and_direct_anchor_must_agree():
    _image_obj, pin = _image()
    inconsistent = _member_bindings(pin, second_base=PARENT + 1)
    targets, reports = _target_rows(inconsistent, {UNIT: _coff()})
    assert targets == {} and all(row["definition"] == Result.UNKNOWN for row in reports)

    anchored = _member_bindings(pin, base=PARENT + 0x20, anchor_base=PARENT)
    targets, reports = _target_rows(anchored, {UNIT: _coff()})
    assert targets == {} and all(row["definition"] == Result.UNKNOWN for row in reports)

    _image_obj, other_pin = _image(PARENT + 0x100)
    member_domain = _member_bindings(other_pin)
    anchor_domain = parse_objects(
        f"// RECOVERY_OBJECT: v1 0x{PARENT:08X} _parent\n",
        UNIT, pin, address_space="pe:va")
    with pytest.raises(ValueError, match="different original address domains"):
        _target_rows(anchor_domain + member_domain, {UNIT: _coff()})


@pytest.mark.parametrize("address", [0x00000004, 0xFFFFFFF5])
def test_member_derived_parent_range_must_fit_the_32_bit_address_domain(address):
    _image_obj, pin = _image()
    binding, = parse_objects(
        f"// RECOVERY_OBJECT: v3 0x{address:08X} _edge member "
        "_parent storage_.bytes_[1]\n", UNIT, pin, address_space="pe:va")
    targets, reports = _target_rows((binding,), {UNIT: _coff()})

    assert targets == {}
    assert reports[0]["definition"] == Result.UNKNOWN


def test_bounded_member_group_stays_outside_unbounded_local_target_proof():
    _image_obj, pin = _image()
    bindings = _member_bindings(pin, bounded=True)
    targets, reports = _target_rows(bindings, {UNIT: _coff()})
    assert targets == {}
    assert reports and all(row["definition"] == Result.UNKNOWN for row in reports)


def test_local_member_parent_namespace_does_not_leak_between_translation_units():
    image, pin = _image()
    first = _member_bindings(pin, unit="src/first.cpp")
    # The same static spelling physically exists in second.cpp, but source
    # facts only bind first.cpp. Neither the COFF name nor another TU's local
    # map entry qualifies second.cpp's reference.
    objects = {"src/first.cpp": _coff(), "src/second.cpp": _coff()}
    targets, _reports = _target_rows(first, objects)
    row, = check_objects([_function(pin, unit="src/second.cpp")], objects,
                         image, local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN


def test_local_member_target_is_tu_qualified_and_never_promoted_to_global():
    image, pin = _image()
    bindings = _member_bindings(pin)
    targets, _reports = _target_rows(bindings, {UNIT: _coff()})
    assert targets == {(UNIT, "_parent"): PARENT}
    row, = check_objects([_function(pin)], {UNIT: _coff()}, image,
                         external_targets={"_parent": PARENT})
    assert row["result"] == Result.UNKNOWN
    row, = check_objects([_function(pin)], {UNIT: _coff()}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.PASS
    row, = check_objects([_function(pin)], {UNIT: _coff()}, image,
                         local_data_targets={("src/other.cpp", "_parent"): PARENT})
    assert row["result"] == Result.UNKNOWN
