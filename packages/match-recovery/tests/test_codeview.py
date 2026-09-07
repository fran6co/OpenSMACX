"""CodeView parser and annotation contracts, using synthetic object bytes only."""
import struct
import pytest
from match_recovery.codeview import ClassNotEmitted, Cursor, class_layout, direct_members
from match_recovery.layout_source import parse_members
from match_recovery.pe_coff import Coff

def record(leaf, body):
    data = struct.pack("<H", leaf) + body
    return struct.pack("<H", len(data)) + data



def debug_types(offset=4, *, tail=b"", duplicate=False):
    fields = struct.pack("<HHIH", 0x1405, 0, 0x74, offset) + b"\x05slot_" + tail
    definition = struct.pack("<HHIIIH", 1, 0, 0x1000, 0, 0, 16) + b"\x06Widget"
    cls = record(0x1005, definition)
    return struct.pack("<I", 2) + record(0x1203, fields) + cls + (cls if duplicate else b"")



def coff(blob=None, *, section=b".debug$T"):
    if blob is None:
        blob = debug_types()
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, 60 + len(blob), 0, 0, 0)
    table = struct.pack("<8sIIIIIIHHI", section, 0, 0, len(blob), 60, 0, 0, 0, 0, 0x40000040)
    return header + table + blob + struct.pack("<I", 4)



def test_strict_standalone_class_layout():
    obj = Coff(coff())
    assert obj.sections[0].name == ".debug$T"
    assert class_layout(obj, "Widget") == {"class": "Widget", "size": 16, "members": {"slot_": 4},
                                          "bases": [], "class_value_members": {},
                                          "member_type_kinds": {"slot_": "non_class_value"},
                                          "member_types": {"slot_": {"kind": "non_class_value"}}}



def hierarchy_types(*, virtual=False, base_index=0x1002, base_offset=0):
    if virtual:
        base = struct.pack("<HHIIHH", 0x1401, 0, base_index, 0x1003, 4, 2)
    else:
        base = struct.pack("<HHIH", 0x1400, 0, base_index, base_offset)
    value = struct.pack("<HHIH", 0x1405, 0, 0x1002, 4) + b"\x05owned"
    pointer = struct.pack("<HHIH", 0x1405, 0, 0x1003, 8) + b"\x03ptr"
    fields = record(0x1203, base + value + pointer)
    definition = record(0x1005, struct.pack("<HHIIIH", 3, 0, 0x1000, 0, 0, 16) + b"\x06Widget")
    forward = record(0x1005, struct.pack("<HHIIIH", 0, 0x80, 0, 0, 0, 0) + b"\x04Base")
    ptr_type = record(0x1002, struct.pack("<II", 0x1002, 0xA))
    return struct.pack("<I", 2) + fields + definition + forward + ptr_type



def test_nonvirtual_base_and_class_value_are_not_confused_with_pointer_members():
    layout = class_layout(Coff(coff(hierarchy_types())), "Widget")
    assert layout["bases"] == [{"type_index": 0x1002, "virtual": False, "offset": 0, "class": "Base"}]
    assert layout["class_value_members"] == {"owned": "Base"}
    assert layout["members"] == {"owned": 4, "ptr": 8}
    assert layout["member_type_kinds"] == {"owned": "class_value", "ptr": "pointer"}



def modified_member_types(target=0x1002, attributes=1, *, body=None):
    blob = hierarchy_types().replace(struct.pack("<HHIH", 0x1405, 0, 0x1002, 4),
                                     struct.pack("<HHIH", 0x1405, 0, 0x1004, 4))
    return blob + record(0x1001, struct.pack("<IH", target, attributes) if body is None else body)



@pytest.mark.parametrize("attributes", [0, 1, 2, 3])
def test_qualified_class_values_remain_embedded_objects(attributes):
    layout = class_layout(Coff(coff(modified_member_types(attributes=attributes))), "Widget")
    assert layout["class_value_members"] == {"owned": "Base"}
    assert layout["member_type_kinds"]["owned"] == "class_value"



def test_qualified_pointer_is_not_unwrapped_into_owned_class():
    layout = class_layout(Coff(coff(modified_member_types(target=0x1003))), "Widget")
    assert "owned" not in layout["class_value_members"]
    assert layout["member_type_kinds"]["owned"] == "pointer"



@pytest.mark.parametrize("options", [{"target": 0x1004}, {"target": 0x9999}, {"attributes": 0x80},
                                     {"body": b"\x02\x10"}, {"body": struct.pack("<IH", 0x1002, 1) + b"bad"}])
def test_cyclic_truncated_or_unsupported_modifiers_are_unknown(options):
    layout = class_layout(Coff(coff(modified_member_types(**options))), "Widget")
    assert "owned" not in layout["class_value_members"]
    assert layout["member_type_kinds"]["owned"] == "unknown"



@pytest.mark.parametrize("text", ["v1 Widget slot_ Base 0x4 0x00401000", "v2 Widget slot_ 0x4 0x00401000"])
def test_member_annotation_version_must_agree_with_type_field(text):
    with pytest.raises(ValueError, match="version/type"):
        parse_members("// RECOVERY_MEMBER: " + text, "src/widget.h", "a" * 64, address_space="pe:va")



def test_virtual_base_metadata_never_invents_a_static_base_offset():
    base, = class_layout(Coff(coff(hierarchy_types(virtual=True))), "Widget")["bases"]
    assert base["class"] == "Base" and base["virtual"]
    assert base["offset"] is None and base["vbptr_offset"] == 4 and base["vbtable_index"] == 2



@pytest.mark.parametrize("options", [{"base_index": 0x9999}, {"base_index": 0x1003}, {"base_offset": 16}])
def test_invalid_base_references_or_extent_do_not_yield_partial_hierarchy(options):
    with pytest.raises(ValueError, match="base"):
        class_layout(Coff(coff(hierarchy_types(**options))), "Widget")



@pytest.mark.parametrize("blob", [debug_types(tail=b"\xff\x7f"), debug_types(16),
                                  debug_types(duplicate=True), debug_types()[:-1],
                                  b"\x01\0\0\0" + debug_types()[4:]])
def test_partial_unsupported_or_ambiguous_layout_is_not_proof(blob):
    with pytest.raises(ValueError):
        class_layout(Coff(coff(blob)), "Widget")



def test_pch_and_missing_class_are_refused():
    with pytest.raises(ValueError, match="standalone"):
        class_layout(Coff(coff(section=b".debug$P")), "Widget")
    with pytest.raises(ClassNotEmitted, match="none emitted"):
        class_layout(Coff(coff()), "Absent")



def test_ambiguous_or_malformed_types_are_not_reported_as_absent():
    for blob in (debug_types(duplicate=True), debug_types()[:-1]):
        with pytest.raises(ValueError) as failure:
            class_layout(Coff(coff(blob)), "Widget")
        assert not isinstance(failure.value, ClassNotEmitted)



def test_unknown_numeric_is_not_zero_and_truncated_names_are_not_accepted():
    with pytest.raises(ValueError, match="numeric"):
        Cursor(b"\xff\x80").numeric()
    with pytest.raises(ValueError, match="truncated"):
        Cursor(b"\x06abc").name()
    with pytest.raises(ValueError, match="field leaf"):
        direct_members(struct.pack("<HHI", 0x1404, 0, 0x1000))
