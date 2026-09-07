"""Synthetic initial storage: declared extents, actual types and target identity."""

from dataclasses import replace
import hashlib
import struct

import pytest

from match_recovery.codeview_data import data_layout, storage_size
from match_recovery.model import Origin, Span
from match_recovery.object_data import check_initial_data, referenced_initial_object_units
from match_recovery.object_source import ObjectBinding, parse_objects
from match_recovery.pe_coff import Coff, PeImage, Symbol
from test_binary import pe_fixture
from test_codeview import record


def data_fixture(value=bytes(4), *, type_index=0x74, types=b"", relocations=(), bss=None,
                 definition=b"_g", reference=b"_h"):
    assert 0 < len(definition) <= 8 and 0 < len(reference) <= 8
    debug = struct.pack("<I", 2) + record(0x1008, struct.pack("<IIH", type_index, 0, 0) + b"\x01g")
    parts = [(b".data", value, relocations, 0xC0300040, len(value)) if bss is None else
             (b".bss", b"", (), 0xC0300080, bss),
             (b".debug$T", struct.pack("<I", 2) + types, (), 0x40000040, 4 + len(types)),
             (b".debug$S", debug, ((12, 0, 11), (16, 0, 10)), 0x40000040, len(debug))]
    tables, chunks, offset = [], [], 20 + 40 * len(parts)
    for name, raw, rels, flags, size in parts:
        relbytes = b"".join(struct.pack("<IIH", *r) for r in rels)
        tables.append(struct.pack("<8sIIIIIIHHI", name, 0, 0, size, offset if raw else 0,
                                  offset + len(raw) if rels else 0, 0, len(rels), 0, flags))
        chunks.append(raw + relbytes)
        offset += len(raw) + len(relbytes)
    symbols = definition.ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0, 2, 0)
    symbols += reference.ljust(8, b"\0") + struct.pack("<IhHBB", 0, 0, 0, 2, 0)
    return struct.pack("<HHIIIHH", 0x14C, len(parts), 0, offset, 2, 0, 0) + b"".join(tables + chunks) + symbols + struct.pack("<I", 4)


def data_image(value=bytes(4), absolute=None):
    raw = bytearray(pe_fixture(value, absolute_site=absolute))
    struct.pack_into("<I", raw, 0x178 + 36, 0xC0000040)
    return bytes(raw)


def proof(original=bytes(4), obj=None, *, absolute=None, target=None, span_size=None):
    raw = data_image(original, absolute)
    image = PeImage(raw, hashlib.sha256(raw).hexdigest())
    origin = Origin(hashlib.sha256(raw).hexdigest(), "pe:va", 0x401000)
    binding = ObjectBinding(origin, "_g", "code/global.cc", 1,
                            Span(0x401000, 0x401000 + (span_size or len(original))), origin)
    row, = check_initial_data((binding,), {binding.unit: obj or Coff(data_fixture())}, image, target or {})
    return row


@pytest.mark.parametrize("type_index,value", [(0x70, b"\x07"), (0x21, b"\x34\x12"), (0x74, b"\x07\0\0\0"),
                                            (0x41, bytes(8)), (0x403, bytes(4))])
def test_width_is_measured_not_assumed_four_bytes(type_index, value):
    obj = Coff(data_fixture(value, type_index=type_index))
    assert data_layout(obj, "_g")["size"] == len(value)
    assert proof(value, obj)["content"] == "pass"


def test_narrow_object_never_consumes_its_neighbors_bytes():
    obj = Coff(data_fixture(b"\0\xff\xff\xff", type_index=0x70))
    row = proof(b"\0", obj)
    assert row["content"] == "pass" and row["compiler_storage"]["size"] == 1
    assert proof(bytes(4), obj)["proof"]["boundary"] == "fail"


def test_dropped_initial_value_is_a_counterexample_even_with_valid_definition():
    row = proof(b"\x09\0\0\0", Coff(data_fixture(bss=4)))
    assert row["definition"] == "pass" and row["content"] == "fail"
    assert row["proof"]["differing_offsets"] == [0]
    assert row["lifetime"] == "unknown"


def test_pointer_initializer_needs_the_right_declared_target_and_relocation():
    original = struct.pack("<I", 0x402004)
    obj = Coff(data_fixture(struct.pack("<I", 4), type_index=0x403, relocations=((0, 1, 6),)))
    assert proof(original, obj, absolute=0, target={"_h": 0x402000})["content"] == "pass"
    assert proof(original, obj, absolute=0, target={"_h": 0x403000})["proof"]["references"] == "fail"
    assert proof(original, obj, absolute=0)["content"] == "unknown"
    hardcoded = Coff(data_fixture(original, type_index=0x403))
    row = proof(original, hardcoded, absolute=0)
    assert row["proof"]["bytes"] == "pass" and row["proof"]["references"] == "fail"


@pytest.mark.parametrize("storage,value,section", [(3, 0, 0), (105, 0, 0), (2, 4, 0), (2, 0, -1)])
def test_local_weak_common_and_absolute_symbols_do_not_borrow_external_binding(storage, value, section):
    obj = Coff(data_fixture(relocations=((0, 1, 6),)))
    obj.symbols[1] = replace(obj.symbols[1], storage=storage, value=value, section=section)
    assert proof(struct.pack("<I", 0x402000), obj, absolute=0, target={"_h": 0x402000})["content"] == "unknown"


@pytest.mark.parametrize("change", ["missing_types", "missing_symbols", "wrong_pair", "addend", "duplicate", "short_record", "overlap"])
def test_missing_or_inconsistent_compiler_storage_is_unknown(change):
    obj = Coff(data_fixture())
    if change == "missing_types":
        obj.sections = (obj.sections[0], replace(obj.sections[1], name=".debug$P"), obj.sections[2])
    elif change == "missing_symbols":
        obj.sections = (*obj.sections[:2], replace(obj.sections[2], relocations=()))
    elif change == "wrong_pair":
        obj.sections = (*obj.sections[:2], replace(obj.sections[2], relocations=((12, 0, 6), (16, 0, 10))))
    elif change == "addend":
        data = bytearray(obj.sections[2].data)
        data[12] = 1
        obj.sections = (*obj.sections[:2], replace(obj.sections[2], data=bytes(data)))
    elif change == "duplicate":
        obj.sections = (*obj.sections, obj.sections[2])
    elif change == "short_record":
        obj.sections = (*obj.sections[:2], replace(obj.sections[2], data=obj.sections[2].data[:-1]))
    else:
        obj.symbols[2] = Symbol(2, "_alias", 0, 1, 0, 2)
    row = proof(obj=obj)
    assert row["content"] == "unknown" and "reason" in row


def test_arrays_use_type_extent_and_invalid_or_recursive_types_are_refused():
    types = record(0x1003, struct.pack("<IIH", 0x74, 0x74, 12) + b"\0")
    obj = Coff(data_fixture(bytes(12), type_index=0x1000, types=types))
    assert data_layout(obj, "_g")["size"] == 12
    assert proof(bytes(12), obj)["content"] == "pass"
    for leaf, body in [(0x1001, struct.pack("<IH", 0x1000, 0)), (0x1002, struct.pack("<II", 0x74, 0x6A)),
                       (0x1003, struct.pack("<IIH", 0x74, 0x74, 3) + b"\0")]:
        obj = Coff(data_fixture(type_index=0x1000, types=record(leaf, body)))
        assert proof(obj=obj)["content"] == "unknown"


@pytest.mark.parametrize("index", [0, 0x100, 0x503, 0x603, 0x900, 0x9999])
def test_unknown_primitives_and_far_or_64_bit_pointer_modes_are_not_four_bytes(index):
    with pytest.raises(ValueError):
        storage_size(index, [])


def test_virtual_zero_fill_is_not_out_of_image_and_file_tail_can_cross_into_it():
    raw = bytearray(data_image())
    struct.pack_into("<I", raw, 0x180, 1024)
    image = PeImage(bytes(raw), hashlib.sha256(raw).hexdigest())
    assert image.read_initial(Span(0x401200, 0x401204)) == bytes(4)
    assert image.read_initial(Span(0x4011FE, 0x401202)) == bytes(4)
    with pytest.raises(ValueError):
        image.read_initial(Span(0x4013FE, 0x401402))


def test_bounded_object_grammar_and_moving_source_keep_original_interpretation():
    item, = parse_objects("// RECOVERY_OBJECT: v2 0x1000 0x1004 _g 0x2000\n", "code/a.cc", "a" * 64, address_space="custom:address")
    assert item.span == Span(0x1000, 0x1004)
    assert replace(item, unit="elsewhere/b.cpp", line=20).interpretation_id == item.interpretation_id
    for annotation in ("v2 0x1000 0x1000 _g 0x2000", "v2 0x1000 _g", "v1 0x1000 0x1004 _g 0x2000"):
        with pytest.raises(ValueError):
            parse_objects("// RECOVERY_OBJECT: " + annotation, "code/a.cc", "a" * 64, address_space="custom:address")


def test_initializer_dependency_selection_ignores_other_data_and_function_references():
    origin = Origin("a" * 64, "pe:va", 0x401000)
    binding = ObjectBinding(origin, "_g", "code/a.cc", 1, Span(0x401000, 0x401004), origin)
    target = ObjectBinding(replace(origin, address=0x402000), "_h", "code/b.cc", 1)
    obj = Coff(data_fixture(bytes(8), relocations=((4, 1, 6),)))
    obj.sections = (*obj.sections, replace(obj.sections[0], name=".text", flags=0x60000020, relocations=((0, 1, 6),)))
    assert referenced_initial_object_units((binding,), (binding, target), {binding.unit: obj}) == frozenset()
    obj.sections = (replace(obj.sections[0], relocations=((0, 1, 6),)), *obj.sections[1:])
    assert referenced_initial_object_units((binding,), (binding, target), {binding.unit: obj}) == frozenset({target.unit})
    unbounded = replace(binding, span=None, evidence=None)
    assert referenced_initial_object_units((unbounded,), (unbounded, target), {binding.unit: obj}) == frozenset()


@pytest.mark.parametrize("damage", [None, "missing", "duplicate", "other_namespace", "other_kind", "malformed_forward"])
def test_class_array_resolves_only_one_complete_same_stream_type(damage):
    forward = record(0x1004, struct.pack("<HHIIIH", 0, 0x80, 0, 0, 0,
                                      4 if damage == "malformed_forward" else 0) + b"\x06A::Box")
    fields = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x74, 0) + b"\x01n")
    name = b"B::Box" if damage == "other_namespace" else b"A::Box"
    definition = record(0x1005 if damage == "other_kind" else 0x1004,
                        struct.pack("<HHIIIH", 1, 0, 0x1001, 0, 0, 4) + bytes([len(name)]) + name)
    if damage == "missing":
        definition = record(0x1001, struct.pack("<IH", 0x74, 0))
    array = record(0x1003, struct.pack("<IIH", 0x1000, 0x74, 12) + b"\0")
    types = forward + fields + definition + array + (definition if damage == "duplicate" else b"")
    obj = Coff(data_fixture(bytes(12), type_index=0x1003, types=types))
    assert proof(bytes(12), obj)["content"] == ("pass" if damage is None else "unknown")
