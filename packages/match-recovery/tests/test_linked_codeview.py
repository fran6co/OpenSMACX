"""Physical associated debug ownership and exact module procedure scopes."""

from dataclasses import replace
import struct

import pytest

from match_recovery.linked_codeview import source_procedure, module_procedures, procedure_correspondence
from match_recovery.codeview_procedures import decode_procedure
from match_recovery.pe_coff import Coff
from test_codeview_procedures import procedure, record, S_END, S_GPROC32_ST


def object_bytes(payload=None, *, relocations=((32, 2, 11), (36, 2, 10))):
    payload = procedure() if payload is None else payload
    code = b"\x90" * 18 + b"\xc3" + b"\xcc" * 13
    reloc_at = 100 + len(code) + len(payload)
    symbol_at = reloc_at + 10 * len(relocations)
    symbol = lambda name, section, kind, storage, aux: name.ljust(8, b"\0") + struct.pack(
        "<IhHBB", 0, section, kind, storage, aux)
    auxiliary = lambda size, count, parent, selection: struct.pack("<IHHIHB3x", size, count, 0, 0, parent, selection)
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbol_at, 5, 0, 0)
    section = lambda name, size, at, reloc, count, flags: name.ljust(8, b"\0") + struct.pack(
        "<IIIIIIHHI", 0, 0, size, at, reloc, 0, count, 0, flags)
    return (header + section(b".text", len(code), 100, 0, 0, 0x60501020)
            + section(b".debug$S", len(payload), 132, reloc_at, len(relocations), 0x42101048)
            + code + payload + b"".join(struct.pack("<IIH", *row) for row in relocations)
            + symbol(b".text", 1, 0, 3, 1) + auxiliary(len(code), 0, 0, 1)
            + symbol(b"_f", 1, 32, 2, 0)
            + symbol(b".debug$S", 2, 0, 3, 1) + auxiliary(len(payload), len(relocations), 1, 5)
            + struct.pack("<I", 4))


def source(raw=None):
    obj = Coff(object_bytes() if raw is None else raw)
    return source_procedure(obj, 2, obj.function("_f"))


def test_complete_child_is_typed_without_accepting_machine_code_relocation_kinds():
    assert source().name == "Owner::add"
    obj = Coff(object_bytes())
    with pytest.raises(ValueError, match="unsupported i386 relocation kind"):
        obj.section_subject(2)


@pytest.mark.parametrize("relocations", [(), ((32, 2, 6), (36, 2, 10)),
    ((32, 2, 11),), ((32, 2, 11), (36, 0, 10)), ((31, 2, 11), (36, 2, 10)),
    ((32, 2, 11), (35, 2, 10)), ((36, 2, 10), (32, 2, 11))])
def test_only_exact_root_section_relative_pair_can_bind_debug(relocations):
    with pytest.raises(ValueError):
        source(object_bytes(relocations=relocations))


@pytest.mark.parametrize("field,value", [("parent", 1), ("end", 87), ("next_", 8),
    ("offset", 4), ("section", 1), ("length", 17), ("length", 33)])
def test_scope_addends_and_nonpadding_extents_remain_obligations(field, value):
    with pytest.raises(ValueError):
        source(object_bytes(procedure(**{field: value})))


@pytest.mark.parametrize("damage", ["flags", "parent", "alias", "incoming", "auxiliary", "nested"])
def test_physical_ownership_damage_cannot_be_discarded_as_debug(damage):
    obj = Coff(object_bytes())
    if damage == "flags":
        obj.sections = (obj.sections[0], replace(obj.sections[1], flags=0x42101040))
    elif damage == "parent":
        obj.associations.clear()
    elif damage == "alias":
        obj.symbols[8] = replace(obj.symbols[3], index=8, name="alias")
    elif damage == "incoming":
        obj.sections = (replace(obj.sections[0], relocations=((0, 3, 6),)), obj.sections[1])
    elif damage == "auxiliary":
        raw = bytearray(obj.auxiliary[3]); raw[6] = 1; obj.auxiliary[3] = bytes(raw)
    else:
        obj.associations[1] = 2
    with pytest.raises(ValueError):
        source_procedure(obj, 2, obj.symbols[2])


def module(**options):
    raw = procedure(alignment=4, **options)
    final = 4 + len(raw) - 4
    raw = bytearray(raw)
    struct.pack_into("<I", raw, 8, final)
    return b"\x02\0\0\0" + bytes(raw)


def test_module_end_links_select_complete_records_not_byte_patterns():
    raw = module(section=1, offset=0x20)
    parsed, = module_procedures(raw)
    assert parsed.end_record_offset == len(raw) - 4
    assert parsed.section == 1 and parsed.offset == 0x20


@pytest.mark.parametrize("cut", range(96))
def test_every_partial_module_refuses(cut):
    # A signature-only stream is valid empty evidence, not a procedure proof.
    if cut == 4:
        assert module_procedures(module()[:cut]) == ()
    else:
        with pytest.raises(ValueError):
            module_procedures(module()[:cut])


@pytest.mark.parametrize("offset,value", [(8, 4), (12, 5), (12, 8), (16, 4)])
def test_bad_parent_end_or_next_link_refuses(offset, value):
    raw = bytearray(module()); struct.pack_into("<I", raw, offset, value)
    with pytest.raises(ValueError):
        module_procedures(bytes(raw))


def test_unsupported_record_inside_procedure_is_not_skipped():
    raw = bytearray(module())
    struct.pack_into("<H", raw, 4 + 52 + 2, 0x1013)
    with pytest.raises(ValueError, match="unsupported"):
        module_procedures(bytes(raw))


def test_nested_procedure_cannot_borrow_outer_end():
    outer = procedure(alignment=4, locals_=[])[:-4]
    inner = procedure(alignment=4, locals_=[])[:-4]
    raw = bytearray(b"\x02\0\0\0" + outer + inner + record(S_END) + record(S_END))
    struct.pack_into("<I", raw, 12, len(raw) - 4)
    struct.pack_into("<I", raw, 4 + len(outer) + 8, len(raw) - 8)
    with pytest.raises(ValueError):
        module_procedures(bytes(raw))


def corresponding():
    local = (0x1006, struct.pack("<iI", 8, 0x74) + b"\x05input")
    source = decode_procedure(procedure(type_index=0x74, locals_=[local]))
    target, = module_procedures(module(type_index=0x74, locals_=[local], section=1))
    return source, target


def test_correspondence_retains_all_nonplacement_contents_after_scope_validation():
    source, target = corresponding()
    result = procedure_correspondence(source, target, {}, {})
    assert result["name"] == "Owner::add" and result["length"] == 19
    assert result["locals"][0]["frame_offset"] == 8
    assert result["types"]["roots"] == [{"primitive": 0x74}, {"primitive": 0x74}]
    assert not {"result", "accepted", "discarded"} & result.keys()


@pytest.mark.parametrize("field,value", [("kind", 0x100A), ("length", 18), ("debug_start", 2),
    ("debug_end", 14), ("flags", 3), ("name", "Other::add"), ("type_index", 0x75)])
def test_any_procedure_field_or_type_change_refuses(field, value):
    source, target = corresponding()
    with pytest.raises(ValueError):
        procedure_correspondence(source, replace(target, **{field: value}), {}, {})


@pytest.mark.parametrize("field,value", [("kind", 0x1001), ("name", "other"), ("frame_offset", 12),
    ("register", 18), ("type_index", 0x75)])
def test_any_local_field_or_type_change_refuses(field, value):
    source, target = corresponding()
    target = replace(target, locals=(replace(target.locals[0], **{field: value}),))
    with pytest.raises(ValueError):
        procedure_correspondence(source, target, {}, {})


@pytest.mark.parametrize("count", [0, 2])
def test_added_or_removed_local_is_not_ignored(count):
    source, target = corresponding()
    with pytest.raises(ValueError):
        procedure_correspondence(source, replace(target, locals=target.locals * count), {}, {})
