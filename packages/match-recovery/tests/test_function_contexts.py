"""Declared context providers are object identities, not linker winners."""

import struct

import pytest

from match_recovery.binary import AbsoluteConstant
from match_recovery.function_contexts import (ContextFunctionTarget,
                                               qualify_context_targets,
                                               referenced_context_units)
from match_recovery.model import Origin, Result
from match_recovery.pe_coff import Coff
from test_function_fragments import associated_parent_coff


PIN = "0" * 64


def coff_object(*, definitions=("_f",), storage=2, kind=32, references=(), comdat=True):
    """One bounded .text COMDAT plus optional exact physical relocations."""
    code = b"\x31\xc0\xc3" + b"\xcc" * 13
    relocation_at = 60 + len(code)
    symbol_at = relocation_at + 10 * len(references)

    def symbol(name, value=0, section=1, symbol_kind=0, symbol_storage=3, aux=0):
        return name.encode("ascii").ljust(8, b"\0") + struct.pack(
            "<IhHBB", value, section, symbol_kind, symbol_storage, aux)

    # Section definition, its auxiliary record, then definitions and relocation
    # targets. Relocation indices count auxiliary records physically.
    records = [symbol(".text", aux=1),
               struct.pack("<IHHIHB3x", len(code), len(references), 0, 0, 0,
                           2 if comdat else 0)]
    for name in definitions:
        records.append(symbol(name, symbol_kind=kind, symbol_storage=storage))
    reference_indices = []
    for name, reference_storage, _relocation_kind in references:
        reference_indices.append(len(records))
        records.append(symbol(name, section=0, symbol_storage=reference_storage))

    flags = 0x60501020 if comdat else 0x60500020
    section = (b".text\0\0\0" + struct.pack(
        "<IIIIIIHHI", 0, 0, len(code), 60, relocation_at if references else 0,
        0, len(references), 0, flags))
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbol_at,
                         len(records), 0, 0)
    relocations = b"".join(struct.pack("<IIH", 0, index, relocation_kind)
                           for index, (_name, _storage, relocation_kind)
                           in zip(reference_indices, references))
    return Coff(header + section + code + relocations + b"".join(records) + struct.pack("<I", 4))


def coff_with_data_references(section_name, references):
    """Two-section object with relocations in a non-code data/metadata section."""
    code = b"\x31\xc0\xc3" + b"\xcc" * 13
    data = bytes(4 * len(references))
    data_at = 100 + len(code)
    relocation_at = data_at + len(data)
    symbol_at = relocation_at + 10 * len(references)

    def symbol(name, value=0, section=1, kind=0, storage=3, aux=0):
        return name.encode("ascii").ljust(8, b"\0") + struct.pack(
            "<IhHBB", value, section, kind, storage, aux)

    records = [symbol(".text", aux=1),
               struct.pack("<IHHIHB3x", len(code), 0, 0, 0, 0, 2),
               symbol("_caller", kind=32, storage=2)]
    reference_indices = []
    for name, storage, _kind in references:
        reference_indices.append(len(records))
        records.append(symbol(name, section=0, storage=storage))

    def section(name, size, raw_at, reloc_at, reloc_count, flags):
        return name.encode("ascii").ljust(8, b"\0") + struct.pack(
            "<IIIIIIHHI", 0, 0, size, raw_at, reloc_at, 0, reloc_count, 0, flags)

    headers = (section(".text", len(code), 100, 0, 0, 0x60501020)
               + section(section_name, len(data), data_at, relocation_at,
                         len(references), 0x40300040))
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbol_at,
                         len(records), 0, 0)
    relocations = b"".join(struct.pack("<IIH", offset * 4, index, kind)
                           for offset, (index, (_name, _storage, kind))
                           in enumerate(zip(reference_indices, references)))
    return Coff(header + headers + code + data + relocations + b"".join(records)
                + struct.pack("<I", 4))


def context_target(*, symbol="_f", unit="engine/context.cc", source_unit="engine/body.cc",
                   address=0x401000):
    return ContextFunctionTarget(Origin(PIN, "pe:va", address), symbol, unit, source_unit)


def test_declared_context_qualifies_only_object_definition_and_preserves_plain_targets():
    ordinary = AbsoluteConstant(0x1234)
    context = context_target()
    resolved, reports = qualify_context_targets({"plain": ordinary, "_f": context},
                                                 {context.unit: coff_object()})
    assert resolved == {"plain": ordinary, "_f": 0x401000}
    assert resolved["plain"] is ordinary
    assert reports == [{
        "symbol": "_f", "unit": "engine/context.cc", "source_unit": "engine/body.cc",
        "origin": context.origin.entity_id, "result": Result.PASS,
        "evidence": "object_level_definition_identity", "linked_selection": False,
        "retained_equivalence": False,
    }]


def test_missing_named_context_does_not_borrow_another_objects_provider():
    target = context_target(unit="engine/declared.cc")
    resolved, reports = qualify_context_targets({"_f": target},
                                                 {"engine/other.cc": coff_object()})
    assert resolved == {}
    assert reports[0]["result"] == Result.UNKNOWN
    assert reports[0]["reason"] == "declared context object is missing"
    assert reports[0]["linked_selection"] is False


@pytest.mark.parametrize("obj,reason", [
    (coff_object(storage=3), "not external COFF storage"),
    (coff_object(kind=0), "expected one defined COFF function"),
    (coff_object(definitions=("_f", "_f")), "expected one defined COFF function"),
    (coff_object(definitions=(), references=(("_f", 2, 6),)),
     "expected one defined COFF function"),
    (coff_object(references=(("_u", 2, 7),)), "unsupported i386 relocation kind"),
    (coff_object(comdat=False), "code COMDAT"),
    (Coff(associated_parent_coff()), "not an associative COMDAT child"),
])
def test_nonexternal_nonfunction_duplicate_undefined_and_noncomdat_targets_remain_unknown(obj, reason):
    target = context_target()
    resolved, reports = qualify_context_targets({"_f": target}, {target.unit: obj})
    assert resolved == {}
    assert reports[0]["result"] == Result.UNKNOWN
    assert reason in reports[0]["reason"]
    assert reports[0]["linked_selection"] is False
    assert reports[0]["retained_equivalence"] is False


@pytest.mark.parametrize("unit", ["", ".", "../outside.cc", "a\\b.cc", "/absolute.cc"])
def test_context_paths_must_be_canonical_relative_paths(unit):
    with pytest.raises(ValueError):
        context_target(unit=unit)


def test_target_key_must_match_exact_decorated_symbol():
    target = context_target()
    resolved, reports = qualify_context_targets({"_other": target}, {target.unit: coff_object()})
    assert resolved == {}
    assert reports[0]["result"] == Result.UNKNOWN


@pytest.mark.parametrize("offset,symbol_index", [(13, 3), (999, 3)])
def test_malformed_physical_relocation_cannot_publish_a_context_target(offset, symbol_index):
    obj = coff_object(references=(("_u", 2, 6),))
    raw = bytearray(obj.raw_data)
    struct.pack_into("<II", raw, 76, offset, symbol_index)
    target = context_target()
    resolved, reports = qualify_context_targets({"_f": target}, {target.unit: Coff(bytes(raw))})
    assert resolved == {}
    assert reports[0]["result"] == Result.UNKNOWN


def test_referenced_context_units_follow_only_real_external_code_relocations():
    target = context_target(unit="engine/context.cc")
    caller = coff_object(definitions=("_caller",), references=(
        ("_f", 2, 6),       # external relocation: context unit is required
        ("_f", 3, 6),       # same spelling, static target: cannot borrow
        ("_f", 2, 0),       # explicit no-op: not a reference
    ))
    assert referenced_context_units({"_f": target}, {"engine/caller.cc": caller}) == {
        "engine/context.cc"
    }
    assert referenced_context_units({"_f": target}, {}) == set()


@pytest.mark.parametrize(("section_name", "expected"), [
    (".rdata", {"engine/context.cc"}),
    (".debug$S", set()),
    (".drectve", set()),
])
def test_referenced_context_units_follow_runtime_data_but_skip_metadata(section_name, expected):
    target = context_target(unit="engine/context.cc")
    obj = coff_with_data_references(section_name, (("_f", 2, 6),))
    assert referenced_context_units({"_f": target}, {"engine/holder.cc": obj}) == expected
