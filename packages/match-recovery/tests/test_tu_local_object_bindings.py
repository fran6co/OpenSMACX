"""TU-qualified static-data references never turn into section/global aliases."""
from __future__ import annotations

import hashlib
import struct
from dataclasses import replace

import pytest

from match_recovery.application_binary import check_compiled
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Result, Span
from match_recovery.object_source import local_object_targets, parse_objects
from match_recovery.pe_coff import Coff, PeImage
from test_binary import pe_fixture


PINNED_TARGET = 0x401100
FUNCTION = 0x401000


def _section(name, data, relocations, flags):
    return (struct.pack("<8sIIIIIIHHI", name.encode().ljust(8, b"\0"), 0, 0,
                        len(data), 0, 0, 0, len(relocations), 0, flags),
            b"".join(struct.pack("<IIH", *row) for row in relocations))


def _auxiliary(length, relocations, number, selection, parent=None):
    return struct.pack("<IHHIHB", length, relocations, 0, 0,
                       number if parent is None else parent, selection) + bytes(3)


def data_object(*, target="_hidden", target_value=0, target_section=2,
                target_type=0, target_storage=3, target_aux=0,
                extra_symbols=(), references=None, bss=False,
                data_comdat=False, data_associative=False):
    """One function COMDAT plus an ordinary data section and named COFF records."""
    code = b"\xa1\0\0\0\0\xc3" + b"\xcc" * 10
    data = b"" if bss else bytes(16)
    specs = [("_caller", 0, 1, 0x20, 2, 0),
             (target, target_value, target_section, target_type, target_storage, target_aux),
             *extra_symbols]
    # Section symbols each consume one auxiliary record; ordinary symbols are
    # indexed after those records and their own optional auxiliary entries.
    indices = {}
    records = []
    index = 0
    data_selection = 5 if data_associative else 2 if data_comdat else 0
    data_parent = 1 if data_associative else None
    for section_name, section_no, section_data, relocs, selection, parent in (
            (".text$A", 1, code, references or (), 2, None),
            (".bss" if bss else ".data", 2, data, (), data_selection, data_parent)):
        indices[section_name] = index
        records.append((section_name, 0, section_no, 0, 3, 1,
                        _auxiliary(16 if section_no == 2 and bss else len(section_data),
                                   len(relocs), section_no, selection, parent)))
        index += 2
    for name, value, section, kind, storage, aux_count in specs:
        if name not in indices:
            indices[name] = index
        records.append((name, value, section, kind, storage, aux_count, bytes(18 * aux_count)))
        index += 1 + aux_count

    relocs = []
    for offset, symbol_name, kind in references or ():
        relocs.append((offset, indices[symbol_name], kind))
    text_table, text_reloc_bytes = _section(".text$A", code, relocs, 0x60501020)
    data_flags = 0xC0300080 if bss else 0xC0300040
    if data_comdat or data_associative:
        data_flags |= 0x1000
    data_table, data_reloc_bytes = _section(".bss" if bss else ".data", data, (), data_flags)
    header_size = 20 + 2 * 40
    text_at = header_size
    data_at = text_at + len(code)
    reloc_at = data_at + len(data)
    symbol_at = reloc_at + len(text_reloc_bytes)
    text_table = bytearray(text_table)
    data_table = bytearray(data_table)
    struct.pack_into("<I", text_table, 20, text_at)
    struct.pack_into("<I", text_table, 24, reloc_at if relocs else 0)
    if bss:
        struct.pack_into("<I", data_table, 16, 16)
        struct.pack_into("<I", data_table, 20, 0)
    else:
        struct.pack_into("<I", data_table, 20, data_at)
    struct.pack_into("<I", data_table, 24, 0)
    symbol_records = []
    for name, value, section, kind, storage, aux_count, aux in records:
        symbol_records.append(name.encode().ljust(8, b"\0")
                              + struct.pack("<IhHBB", value, section, kind, storage, aux_count)
                              + aux)
    symbol_bytes = b"".join(symbol_records)
    symbol_count = index
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbol_at, symbol_count, 0, 0)
    return (header + bytes(text_table) + bytes(data_table) + code + data
            + text_reloc_bytes + symbol_bytes + struct.pack("<I", 4))


def _image(target=PINNED_TARGET):
    raw = bytearray(pe_fixture(b"\xa1" + struct.pack("<I", target) + b"\xc3",
                               absolute_site=1))
    pin = hashlib.sha256(raw).hexdigest()
    return PeImage(bytes(raw), pin), pin


def _objects(pin, text="// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
             *, unit="src/owner.cpp", obj=None):
    bindings = parse_objects(text, unit, pin, address_space="pe:va")
    objects = {unit: Coff(obj if obj is not None else
                          data_object(references=((1, "_hidden", 6),)))}
    return bindings, objects


def _function(pin, *, unit="src/owner.cpp"):
    return FunctionBinding(Origin(pin, "pe:va", FUNCTION),
                          (Span(FUNCTION, FUNCTION + 6),), "_caller", unit)


def _target_rows(bindings, objects):
    targets, reports = local_object_targets(bindings, objects)
    return targets, reports


def test_same_translation_unit_static_data_resolves_without_section_placement():
    image, pin = _image()
    bindings, objects = _objects(pin)
    targets, reports = _target_rows(bindings, objects)
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    assert reports
    assert reports[0]["definition"] == "pass"
    assert reports[0]["linkage"] == "translation_unit_local"
    assert reports[0]["extent"] == "unbounded"
    rows = check_objects([_function(pin)], objects, image, local_data_targets=targets)
    assert rows[0]["result"] == Result.PASS


def test_wrong_local_address_fails_and_neighbor_in_same_data_section_stays_unknown():
    image, pin = _image()
    binding = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    objects = {"src/owner.cpp": Coff(data_object(references=((1, "_hidden", 6),)))}
    wrong = {("src/owner.cpp", "_hidden"): PINNED_TARGET + 4}
    row, = check_objects([_function(pin)], objects, image, local_data_targets=wrong)
    assert row["result"] == Result.FAIL

    # The neighboring static symbol shares .data with the mapped object, but
    # neither that section nor the neighbor inherits the mapped identity.
    neighbor_obj = Coff(data_object(target="_hidden", extra_symbols=(
        ("_nearby", 8, 2, 0, 3, 0),), references=((1, "_nearby", 6),)))
    targets, _ = _target_rows(binding, {"src/owner.cpp": neighbor_obj})
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    row, = check_objects([_function(pin)], {"src/owner.cpp": neighbor_obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN


def test_same_symbol_in_another_tu_does_not_leak_and_missing_units_are_unknown():
    image, pin = _image()
    binding = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    owner = Coff(data_object(target="_hidden"))
    caller = Coff(data_object(target="_hidden", target_section=0, target_storage=2,
                              references=((1, "_hidden", 6),)))
    objects = {"src/owner.cpp": owner, "src/caller.cpp": caller}
    targets, _ = _target_rows(binding, objects)
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    row, = check_objects([_function(pin, unit="src/caller.cpp")], objects, image,
                         local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN

    wrong_unit = {("src/missing.cpp", "_hidden"): PINNED_TARGET}
    owner_with_reference = Coff(data_object(references=((1, "_hidden", 6),)))
    row, = check_objects([_function(pin)], {"src/owner.cpp": owner_with_reference}, image,
                         local_data_targets=wrong_unit)
    assert row["result"] != Result.PASS
    row, = check_objects([_function(pin)], {}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN


@pytest.mark.parametrize("damage", ["duplicate", "code", "section_symbol", "auxiliary",
                                     "common", "undefined", "global"])
def test_only_one_static_ordinary_data_definition_is_admitted(damage):
    image, pin = _image()
    if damage == "duplicate":
        raw = data_object(target="_hidden", extra_symbols=(
            ("_hidden", 4, 2, 0, 3, 0),))
    elif damage == "code":
        raw = data_object(target="_hidden", target_value=0, target_section=1,
                          target_type=0)
    elif damage == "section_symbol":
        raw = data_object()
        binding = parse_objects("// RECOVERY_OBJECT: v4 0x00401100 _legacy target .data\n",
                                "src/owner.cpp", pin, address_space="pe:va")
        targets, reports = _target_rows(binding, {"src/owner.cpp": Coff(raw)})
        assert targets == {}
        assert reports and reports[0]["definition"] == Result.UNKNOWN
        return
    elif damage == "auxiliary":
        raw = data_object(target="_hidden", target_aux=1)
    elif damage == "common":
        raw = data_object(target="_hidden", target_value=4, target_section=0,
                          target_storage=2)
    elif damage == "undefined":
        raw = data_object(target="_hidden", target_value=0, target_section=0,
                          target_storage=2)
    else:
        raw = data_object(target="_hidden", target_storage=2)
    binding = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    targets, reports = _target_rows(binding, {"src/owner.cpp": Coff(raw)})
    assert targets == {}
    assert reports and reports[0]["definition"] == Result.UNKNOWN
    if damage in ("duplicate", "code"):
        # An explicit local map does not make a duplicate or code-section
        # symbol valid. In particular, the code symbol would otherwise inherit
        # the function COMDAT's placement and spuriously resolve this operand.
        image_for_symbol = image
        if damage == "code":
            image_for_symbol, pin = _image(FUNCTION)
        obj = Coff(raw)
        row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image_for_symbol,
                             local_data_targets={("src/owner.cpp", "_hidden"): PINNED_TARGET})
        assert row["result"] != Result.PASS


def test_offset_static_symbol_preserves_original_relocation_addend():
    image, pin = _image(PINNED_TARGET + 8)
    binding = parse_objects("// RECOVERY_OBJECT: v4 0x00401100 _legacy target _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    raw = data_object(target="_hidden", target_value=4,
                      references=((1, "_hidden", 6),))
    targets, _ = _target_rows(binding, {"src/owner.cpp": Coff(raw)})
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    # The COFF static symbol's value (4) and relocation addend (8) remain
    # independent contributions; resolving to object start yields target+8.
    # Candidate immediate contains addend 8 for this focused control.
    obj = Coff(data_object(target="_hidden", target_value=4,
                           references=((1, "_hidden", 6),)))
    code = bytearray(obj.sections[0].data)
    code[1:5] = struct.pack("<I", 8)
    obj.sections = (replace(obj.sections[0], data=bytes(code)), *obj.sections[1:])
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.PASS
    code[1:5] = struct.pack("<I", 12)
    obj.sections = (replace(obj.sections[0], data=bytes(code)), *obj.sections[1:])
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.FAIL


def test_global_external_resolution_is_unchanged_and_not_promoted_from_local_map():
    image, pin = _image(PINNED_TARGET)
    binding = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    # A separate global reference remains governed by explicit global targets.
    obj = Coff(data_object(target="_hidden", references=((1, "_global", 6),),
                           extra_symbols=(("_global", 0, 0, 0, 2, 0),)))
    local_targets, _ = _target_rows(binding, {"src/owner.cpp": obj})
    assert local_targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         external_targets={"_global": PINNED_TARGET},
                         local_data_targets=local_targets)
    assert row["result"] == Result.PASS
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets=local_targets)
    assert row["result"] == Result.UNKNOWN


def test_bss_static_definition_resolves_from_declared_section_size_without_rawbytes():
    image, pin = _image()
    bindings = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                             "src/owner.cpp", pin, address_space="pe:va")
    obj = Coff(data_object(bss=True, references=((1, "_hidden", 6),)))
    assert obj.sections[1].data == b""
    assert obj.section_sizes[1] == 16
    targets, reports = _target_rows(bindings, {"src/owner.cpp": obj})
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    assert reports[0]["definition"] == "pass"
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.PASS


@pytest.mark.parametrize("damage", ["function_type", "executable", "mixed_data_kinds",
                                     "past_section", "empty_section", "weak"])
def test_invalid_physical_local_target_cannot_borrow_section_or_global_fallback(damage):
    image, pin = _image()
    obj = Coff(data_object(references=((1, "_hidden", 6),)))
    symbol, = (s for s in obj.symbols.values() if s.name == "_hidden")
    if damage == "function_type":
        obj.symbols[symbol.index] = replace(symbol, type=0x20)
    elif damage == "executable":
        obj.sections = (obj.sections[0], replace(obj.sections[1], flags=0xE0300040))
    elif damage == "mixed_data_kinds":
        obj.sections = (obj.sections[0], replace(obj.sections[1], flags=0xC03000C0))
    elif damage == "past_section":
        obj.symbols[symbol.index] = replace(symbol, value=16)
    elif damage == "empty_section":
        obj.section_sizes = (obj.section_sizes[0], 0)
    else:
        obj.symbols[symbol.index] = replace(symbol, section=0, storage=105)
    bindings = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                             "src/owner.cpp", pin, address_space="pe:va")
    targets, reports = _target_rows(bindings, {"src/owner.cpp": obj})
    assert targets == {} and reports[0]["definition"] == Result.UNKNOWN
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets={("src/owner.cpp", "_hidden"): PINNED_TARGET},
                         external_targets={"_hidden": PINNED_TARGET},
                         section_bases={("src/owner.cpp", 2): PINNED_TARGET})
    assert row["result"] != Result.PASS


def test_unbound_static_in_another_tu_does_not_borrow_the_same_named_binding():
    image, pin = _image()
    bindings, objects = _objects(pin)
    objects["src/other.cpp"] = Coff(data_object(references=((1, "_hidden", 6),)))
    targets, _ = _target_rows(bindings, objects)
    row, = check_objects([_function(pin, unit="src/other.cpp")], objects, image,
                         local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN


@pytest.mark.parametrize("associative", [False, True])
def test_comdat_and_associative_data_definitions_are_not_local_object_targets(associative):
    image, pin = _image()
    del image
    bindings = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                             "src/owner.cpp", pin, address_space="pe:va")
    obj = Coff(data_object(data_comdat=not associative,
                           data_associative=associative))
    targets, _reports = _target_rows(bindings, {"src/owner.cpp": obj})
    assert targets == {}


@pytest.mark.parametrize("text", [
    "// RECOVERY_OBJECT: v2 0x00401100 0x00401104 _hidden 0x00401100\n",
    "// RECOVERY_OBJECT: v3 0x00401100 _member member _parent field\n",
    ("// RECOVERY_OBJECT: v1 0x00401100 _parent\n"
     "// RECOVERY_OBJECT: v3 0x00401104 _member member _parent field\n"),
])
def test_bounded_member_and_member_group_bindings_are_not_local_object_targets(text):
    image, pin = _image()
    del image
    bindings = parse_objects(text, "src/owner.cpp", pin, address_space="pe:va")
    obj = Coff(data_object(target="_parent" if "_parent" in text else "_hidden"))
    targets, _reports = _target_rows(bindings, {"src/owner.cpp": obj})
    assert targets == {}


@pytest.mark.parametrize("mapping", [
    {"_hidden": PINNED_TARGET},
    {("src/owner.cpp", "_hidden"): True},
    {("src/owner.cpp", "_hidden"): -1},
    {("src/owner.cpp", "_hidden"): 0x100000000},
    {("src/owner.cpp", "_hidden", "extra"): PINNED_TARGET},
])
def test_local_data_map_rejects_malformed_keys_and_values(mapping):
    image, pin = _image()
    objects = {"src/owner.cpp": Coff(data_object(references=((1, "_hidden", 6),)))}
    with pytest.raises(ValueError, match="invalid TU-qualified local data target"):
        check_objects([_function(pin)], objects, image, local_data_targets=mapping)


def test_check_compiled_pipeline_uses_source_local_object_bindings():
    image, pin = _image()
    source_objects, objects = _objects(pin)
    function_rows, object_rows, *_ = check_compiled(
        [_function(pin)], source_objects, (), objects, image, {})
    assert function_rows[0]["result"] == Result.PASS
    assert object_rows and object_rows[0]["definition"] == "pass"
    assert object_rows[0]["linkage"] == "translation_unit_local"
