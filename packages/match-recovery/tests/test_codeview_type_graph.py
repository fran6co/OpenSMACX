"""Physical CV5 type graphs: renumbering is not omission or type coalescing."""

import json
import struct

import pytest

from match_recovery.codeview_type_graph import (
    coff_type_records, tpi_type_records, compare_type_records, compare_type_roots,
)
from match_recovery.pe_coff import Coff
from test_codeview import coff


def record(leaf, body):
    raw = struct.pack("<H", leaf) + body
    padding = (-(len(raw) + 2)) % 4
    return raw + bytes(range(0xF0 + padding, 0xF0, -1))


def pointer(target):
    return record(0x1002, struct.pack("<II", target, 0xA))


def procedure(args, *, call=0):
    return record(0x1008, struct.pack("<IBBHI", 0x74, call, 0, 1, args))


def graph(base=0x1000, *, value=0x74, call=0, name=b"field", offset=0, size=4):
    return {
        base: procedure(base + 1, call=call),
        base + 1: record(0x1201, struct.pack("<II", 1, base + 2)),
        base + 2: pointer(base + 3),
        base + 3: record(0x1005, struct.pack("<HHIIIH", 1, 0, base + 4, 0, 0, size) + b"\x05Owner"),
        base + 4: record(0x1203, struct.pack("<HHIH", 0x1405, 3, value, offset) + bytes([len(name)]) + name),
    }


def encode_tpi(records):
    body = b"".join(struct.pack("<H", len(raw)) + raw for raw in records.values())
    header = struct.pack("<IIIIIHHIIiiiiii", 19961031, 56, 0x1000, 0x1000 + len(records),
                         len(body), 4, 0xFFFF, 2, 0x1000, 0, len(records) * 2,
                         len(records) * 2, 0, len(records) * 2, 0)
    return header + body


def test_physical_coff_and_tpi_records_have_exact_correspondence():
    records = graph()
    types = struct.pack("<I", 2) + b"".join(struct.pack("<H", len(raw)) + raw for raw in records.values())
    source = coff_type_records(Coff(coff(types)))
    linked = tpi_type_records(encode_tpi(records))
    canonical = compare_type_roots(source, [0x1000, 0x1002], linked, [0x1000, 0x1002])
    assert len(canonical["nodes"]) == 5
    assert canonical["roots"] == [{"node": 0}, {"node": 1}]
    assert json.loads(json.dumps(canonical)) == canonical
    with pytest.raises(TypeError):
        linked[0x1000] = pointer(0x74)
    with pytest.raises(TypeError):
        source[0x1000] = pointer(0x74)


def test_joint_procedure_and_local_roots_ignore_numbers_but_keep_repetition():
    a, b = graph(), graph(0x2000)
    result = compare_type_roots(a, [0x1000, 0x1002, 0x1002, 0x74], b, [0x2000, 0x2002, 0x2002, 0x74])
    assert result == compare_type_roots(b, [0x2000, 0x2002, 0x2002, 0x74], a, [0x1000, 0x1002, 0x1002, 0x74])
    assert result["roots"][1] == result["roots"][2]
    assert result["roots"][-1] == {"primitive": 0x74}
    assert len(result["nodes"]) == 5


@pytest.mark.parametrize("change", [{"value": 0x75}, {"call": 4}, {"name": b"other"}, {"offset": 1}, {"size": 8}])
def test_member_types_names_offsets_sizes_and_procedure_conventions_survive(change):
    with pytest.raises(ValueError, match="differ"):
        compare_type_records(graph(), 0x1000, graph(0x2000, **change), 0x2000)


def test_local_type_mismatch_cannot_be_hidden_by_matching_procedure():
    with pytest.raises(ValueError, match="primitive"):
        compare_type_roots(graph(), [0x1000, 0x74], graph(0x2000), [0x2000, 0x75])


def test_real_recursive_class_cycle_compares_without_hash_rounds():
    a, b = graph(value=0x1002), graph(0x2000, value=0x2002)
    result = compare_type_records(a, 0x1000, b, 0x2000)
    assert len(result["nodes"]) == 5
    assert result["nodes"][4]["references"][0]["target"] == {"node": 2}


def test_inconsistent_cycle_and_collapsed_type_multiplicity_refuse():
    a, b = graph(value=0x1002), graph(0x2000, value=0x2003)
    with pytest.raises(ValueError, match="pairing"):
        compare_type_records(a, 0x1000, b, 0x2000)
    left = {0x1000: pointer(0x74), 0x1001: pointer(0x74)}
    right = {0x2000: pointer(0x74)}
    for first, roots_a, second, roots_b in [(left, [0x1000, 0x1001], right, [0x2000, 0x2000]),
                                           (right, [0x2000, 0x2000], left, [0x1000, 0x1001])]:
        with pytest.raises(ValueError, match="pairing"):
            compare_type_roots(first, roots_a, second, roots_b)


def test_forward_declaration_never_substitutes_for_definition():
    source, linked = graph(), graph(0x2000)
    linked[0x2003] = record(0x1005, struct.pack("<HHIIIH", 0, 0x80, 0, 0, 0, 0) + b"\x05Owner")
    with pytest.raises(ValueError, match="payload"):
        compare_type_records(source, 0x1000, linked, 0x2000)


@pytest.mark.parametrize("leaf", [0x0013, 0x0014, 0x0016, 0x0200, 0x100E, 0x1200, 0x1501, 0x1509, 0x1515])
def test_unreferenced_index_context_record_cannot_shift_a_root(leaf):
    source, linked = graph(), graph(0x2000)
    source[0x1010] = record(leaf, struct.pack("<I", 0x2000))
    with pytest.raises(ValueError, match="standalone"):
        compare_type_records(source, 0x1000, linked, 0x2000)
    blob = struct.pack("<I", 2) + b"".join(struct.pack("<H", len(raw)) + raw for raw in source.values())
    with pytest.raises(ValueError, match="standalone"):
        coff_type_records(Coff(coff(blob)))


def test_no_recursive_python_stack_limit_or_fixed_depth_cutoff():
    count = 3000
    a = {0x1000 + n: pointer(0x1001 + n if n + 1 < count else 0x1000) for n in range(count)}
    b = {0x5000 + n: pointer(0x5001 + n if n + 1 < count else 0x5000) for n in range(count)}
    result = compare_type_records(a, 0x1000, b, 0x5000)
    assert len(result["nodes"]) == count
    b[0x5000 + count - 1] = pointer(0x5001)
    with pytest.raises(ValueError, match="pairing"):
        compare_type_records(a, 0x1000, b, 0x5000)


@pytest.mark.parametrize("raw", [record(0xFFFF, b""), record(0x100E, b""), b"\x02\x10", pointer(0x1234)])
def test_unsupported_truncated_imported_and_missing_types_are_not_dropped(raw):
    with pytest.raises(ValueError):
        compare_type_records({0x1000: raw}, 0x1000, {0x2000: raw}, 0x2000)


@pytest.mark.parametrize("bad", [True, -1, 0xFFFFFFFF, "4096"])
def test_invalid_indices_refuse(bad):
    with pytest.raises(ValueError):
        compare_type_records(graph(), bad, graph(), bad)


def test_mutable_record_bytes_refuse_and_returned_graph_is_defensive():
    a, b = graph(), graph(0x2000)
    saved = compare_type_records(a, 0x1000, b, 0x2000)
    result = compare_type_records(a, 0x1000, b, 0x2000)
    result["nodes"][0]["references"].clear()
    assert compare_type_records(a, 0x1000, b, 0x2000) == saved
    a[0x1004] = graph(value=0x75)[0x1004]
    with pytest.raises(ValueError):
        compare_type_records(a, 0x1000, b, 0x2000)
    a[0x1000] = bytearray(a[0x1000])
    with pytest.raises(ValueError, match="immutable"):
        compare_type_records(a, 0x1000, b, 0x2000)


@pytest.mark.parametrize("offset,fmt,value", [
    (0, "I", 19990903), (4, "I", 20), (8, "I", 0x2000), (12, "I", 0x1006),
    (16, "I", 0), (20, "H", 2), (22, "H", 6), (24, "I", 4), (28, "I", 10),
    (32, "i", -1), (36, "i", 1), (40, "i", -1), (44, "i", 1), (48, "i", -1),
    (52, "i", -1), (56, "H", 1), (56, "H", 3), (56, "H", 65534),
])
def test_tpi_header_ranges_and_record_extents_fail_closed(offset, fmt, value):
    raw = bytearray(encode_tpi(graph()))
    struct.pack_into("<" + fmt, raw, offset, value)
    with pytest.raises(ValueError):
        tpi_type_records(bytes(raw))


def test_tpi_rejects_uncovered_bytes_dangling_types_and_wrong_count():
    raw = encode_tpi(graph())
    for invalid in (raw + b"tail", raw[:-1], bytearray(raw), b""):
        with pytest.raises(ValueError):
            tpi_type_records(invalid)
    records = graph()
    records[0x1002] = pointer(0x2000)
    with pytest.raises(ValueError, match="outside"):
        tpi_type_records(encode_tpi(records))


def test_empty_tpi_is_not_an_absent_requested_type():
    records = tpi_type_records(encode_tpi({}))
    assert dict(records) == {}
    assert compare_type_records(records, 0x74, records, 0x74)["nodes"] == []
    with pytest.raises(ValueError, match="outside"):
        compare_type_records(records, 0x1000, records, 0x1000)
