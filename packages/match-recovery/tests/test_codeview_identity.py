"""Type-index renumbering is not permission to drop layout/type evidence."""

import struct
from dataclasses import replace

import pytest

from match_recovery.codeview_identity import reference_fields, type_identity
from match_recovery.coff_preservation import compare_outside_functions
from match_recovery.coff_replica import object_signature
from match_recovery.pe_coff import Coff, Symbol
from test_codeview import coff, record


def graph(order=("fields", "owner", "base", "pointer", "args", "method"), *,
          offset=0x1000, member="owned", base="Base", attributes=3, call=4, adjustment=0, field_type="base"):
    indices = {name: index + 0x1000 for index, name in enumerate(order)}
    def index(name):
        return indices[name]
    contents = {
        "fields": record(0x1203, struct.pack("<HHIH", 0x1405, attributes, index(field_type), offset)
                         + bytes([len(member)]) + member.encode()),
        "owner": record(0x1005, struct.pack("<HHIIIH", 1, 0, index("fields"), 0, 0, 0x2000) + b"\x05Owner"),
        "base": record(0x1005, struct.pack("<HHIIIH", 0, 0x80, 0, 0, 0, 0) + bytes([len(base)]) + base.encode()),
        "pointer": record(0x1002, struct.pack("<II", index("base"), 0xA)),
        "args": record(0x1201, struct.pack("<III", 2, index("pointer"), 0x74)),
        "method": record(0x1009, struct.pack("<IIIBBHIi", 0x74, index("base"), index("pointer"),
                                           call, 0, 2, index("args"), adjustment)),
    }
    return Coff(coff(struct.pack("<I", 2) + b"".join(contents[name] for name in order)))


def test_full_type_graph_is_independent_of_emission_order():
    old = graph()
    new = graph(("method", "base", "owner", "args", "fields", "pointer"))
    assert old.sections[0].data != new.sections[0].data
    assert type_identity(old) == type_identity(new)
    assert object_signature(old) == object_signature(new)
    assert compare_outside_functions(old, new, {}, {})["result"] == "pass"


@pytest.mark.parametrize("change", [{"offset": 0x1001}, {"member": "other"}, {"base": "Else"},
                                    {"attributes": 1}, {"call": 0}, {"adjustment": 0x1000},
                                    {"field_type": "pointer"}])
def test_offsets_names_access_conventions_adjustments_and_member_types_remain_protected(change):
    old, new = graph(), graph(**change)
    assert type_identity(old) != type_identity(new)
    assert object_signature(old) != object_signature(new)
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"


@pytest.mark.parametrize("leaf,body,expected", [
    (0x000A, struct.pack("<H", 3) + b"\x00\x00", ()),
    (0x1001, struct.pack("<IH", 0x1000, 1), (2,)),
    (0x1002, struct.pack("<II", 0x1000, 0xA), (2,)),
    (0x1002, struct.pack("<IIIH", 0x1000, 0x4A, 0x1001, 1), (2, 10)),
    (0x1003, struct.pack("<IIH", 0x1000, 0x74, 16) + b"\0", (2, 6)),
    (0x1004, struct.pack("<HHIIIH", 1, 0, 0x1000, 0x1001, 0x1002, 16) + b"\x01C", (6, 10, 14)),
    (0x1006, struct.pack("<HHIH", 1, 0, 0x1000, 16) + b"\x01U", (6,)),
    (0x1007, struct.pack("<HHII", 1, 0, 0x74, 0x1000) + b"\x01E", (6, 10)),
    (0x1008, struct.pack("<IBBHI", 0x74, 0, 0, 2, 0x1000), (2, 10)),
    (0x1009, struct.pack("<IIIBBHIi", 0x74, 0x1000, 0x1001, 4, 0, 2, 0x1002, 0x1003), (2, 6, 10, 18)),
    (0x1201, struct.pack("<III", 2, 0x1000, 0x1001), (6, 10)),
    (0x1204, struct.pack("<II", 1, 0x1000), (6,)),
    (0x1205, struct.pack("<IBB", 0x74, 4, 2), (2,)),
    (0x1206, struct.pack("<HHIIHHI", 4 << 2, 0, 0x1000, 0x1001, 3, 0, 0x1002), (6, 18)),
])
def test_only_declared_reference_fields_are_type_indices(leaf, body, expected):
    data = struct.pack("<H", leaf) + body
    assert tuple(at for at, _ in reference_fields(data)) == expected


@pytest.mark.parametrize("kind,tail,expected", [
    (0x0403, struct.pack("<H", 0x1000) + b"\x01E", ()),
    (0x1400, struct.pack("<IH", 0x1000, 0x1001), (6,)),
    (0x1401, struct.pack("<IIHH", 0x1000, 0x1001, 0x1002, 0x1003), (6, 10)),
    (0x1402, struct.pack("<IIHH", 0x1000, 0x1001, 0x1002, 0x1003), (6, 10)),
    (0x1403, struct.pack("<I", 0x1000) + b"\x01f", (6,)),
    (0x1404, struct.pack("<I", 0x1000), (6,)),
    (0x1405, struct.pack("<IH", 0x1000, 0x1001) + b"\x01m", (6,)),
    (0x1406, struct.pack("<I", 0x1000) + b"\x01m", (6,)),
    (0x1407, struct.pack("<I", 0x1000) + b"\x01f", (6,)),
    (0x1408, struct.pack("<I", 0x1000) + b"\x01T", (6,)),
    (0x1409, struct.pack("<I", 0x1000), (6,)),
    (0x140A, struct.pack("<I", 0x1000), (6,)),
    (0x140B, struct.pack("<I", 0x1000) + b"\x01f", (6,)),
    (0x140C, struct.pack("<II", 0x1000, 0x1001), (6,)),
    (0x140D, struct.pack("<I", 0x1000) + b"\x01T", (6,)),
])
def test_field_reference_positions_never_include_offsets_or_enum_values(kind, tail, expected):
    assert tuple(at for at, _ in reference_fields(struct.pack("<HHH", 0x1203, kind, 0) + tail)) == expected


def test_record_multiplicity_is_not_erased_by_structural_identity():
    one = record(0x1001, struct.pack("<IH", 0x74, 1))
    a, b = (Coff(coff(struct.pack("<I", 2) + one * count)) for count in (1, 2))
    assert len(type_identity(a)) == 1 and len(type_identity(b)) == 2
    assert object_signature(a) != object_signature(b)


@pytest.mark.parametrize("body", [struct.pack("<IH", 0x9999, 1), struct.pack("<IH", 0x1000, 1)])
def test_invalid_reference_or_cycle_never_supplies_normalized_evidence(body):
    obj = Coff(coff(struct.pack("<I", 2) + record(0x1001, body)))
    with pytest.raises(ValueError, match="outside|cyclic"):
        type_identity(obj)


def test_unknown_stream_is_compared_raw_not_dropped():
    a = Coff(coff(struct.pack("<I", 2) + record(0xFFFF, b"opaque-a")))
    b = Coff(coff(struct.pack("<I", 2) + record(0xFFFF, b"opaque-b")))
    with pytest.raises(ValueError, match="unsupported"):
        type_identity(a)
    assert object_signature(a) == object_signature(a)
    assert object_signature(a) != object_signature(b)


def test_type_depth_is_not_a_python_recursion_limit():
    types = [record(0x1001, struct.pack("<IH", 0x74 if i == 0 else 0x1000 + i - 1, 1))
             for i in range(2000)]
    assert len(type_identity(Coff(coff(struct.pack("<I", 2) + b"".join(types))))) == 2000


def test_relocated_type_stream_cannot_use_numbering_normalization():
    old = graph()
    new = graph(("method", "base", "owner", "args", "fields", "pointer"))
    assert type_identity(old) == type_identity(new)
    for obj in (old, new):
        obj.symbols[0] = Symbol(0, "_external", 0, 0, 0, 2)
        obj.sections = (replace(obj.sections[0], relocations=((8, 0, 6),)),)
    # The graph does not carry byte-offset relocation edges; the enclosing
    # object control must retain the raw type bytes in this unsupported case.
    assert object_signature(old) != object_signature(new)


@pytest.mark.parametrize("payload", [
    struct.pack("<H", 0x1001),  # Truncated modifier.
    struct.pack("<HII", 0x1002, 0x74, 0x100A),  # Unsupported pointer attributes.
    struct.pack("<HIH", 0x1001, 0x74, 1) + b"\xf2\xf2",  # Invalid padding.
    struct.pack("<HIH", 0x1001, 0x74, 1) + b"tail",  # Unparsed extension.
])
def test_unsupported_or_malformed_record_never_supplies_partial_evidence(payload):
    obj = Coff(coff(struct.pack("<IH", 2, len(payload)) + payload))
    with pytest.raises(ValueError):
        type_identity(obj)
    assert object_signature(obj) == object_signature(obj)  # Strict raw fallback.


@pytest.mark.parametrize("kind,attributes,prefix,suffix", [
    (0x0403, 0, b"", b"\x01E"),  # Enum constant.
    (0x1400, 3, struct.pack("<I", 0x74), b""),  # Base offset.
    (0x140B, 4 << 2, struct.pack("<I", 0x74), b"\x01f"),  # Virtual slot.
    (0x140C, 0, struct.pack("<I", 0x74), b""),  # Vtable pointer offset.
])
def test_index_like_nonreference_values_remain_exact(kind, attributes, prefix, suffix):
    fmt = "<H" if kind in (0x0403, 0x1400) else "<I"
    def obj(value):
        fields = struct.pack("<HH", kind, attributes) + prefix + struct.pack(fmt, value) + suffix
        return Coff(coff(struct.pack("<I", 2) + record(0x1203, fields)))
    assert type_identity(obj(0x1000)) != type_identity(obj(0x1001))
    assert object_signature(obj(0x1000)) != object_signature(obj(0x1001))
