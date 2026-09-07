"""Invocation-local CodeView indexing, with synthetic standalone objects."""

import struct

import pytest

from match_recovery import codeview
from match_recovery.application_layout import verify_layout
from match_recovery.layout_check import check_layout
from match_recovery.model import Origin
from match_recovery.pe_coff import Coff
from match_recovery.tasks import Task, TaskStore
from test_codeview import coff, debug_types, hierarchy_types, record
from test_layout_check import layout_project
from test_symbol_check import project


def sibling_types(*, bad_fields=False, duplicate=False):
    fields = struct.pack("<HHIH", 0x1405, 0, 0x74, 8) + b"\x05slot_"
    if bad_fields:
        fields += b"\xff\x7f"
    definition = record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1002, 0, 0, 16) + b"\x07Sibling")
    return debug_types() + record(0x1203, fields) + definition + (definition if duplicate else b"")


def test_one_stream_parse_serves_present_absent_and_repeated_names(monkeypatch):
    original, calls = codeview._type_records, []
    def observed(obj):
        calls.append(obj)
        return original(obj)
    monkeypatch.setattr(codeview, "_type_records", observed)
    obj = Coff(coff(sibling_types()))
    index = codeview.ClassLayouts(obj)
    assert index.layout("Widget")["members"] == {"slot_": 4}
    assert index.layout("Sibling")["members"] == {"slot_": 8}
    with pytest.raises(codeview.ClassNotEmitted):
        index.layout("Absent")
    assert index.layout("Widget")["members"] == {"slot_": 4}
    assert calls == [obj]


def test_returned_layout_mutations_do_not_change_later_observations():
    index = codeview.ClassLayouts(Coff(coff(hierarchy_types())))
    first = index.layout("Widget")
    first["members"]["owned"] = 123
    first["bases"][0]["offset"] = 123
    first["member_types"]["ptr"]["target"]["class"] = "Wrong"
    again = index.layout("Widget")
    assert again["members"]["owned"] == 4 and again["bases"][0]["offset"] == 0
    assert again["member_types"]["ptr"]["target"]["class"] == "Base"


def test_identical_class_names_in_different_objects_do_not_share_observations():
    first = codeview.ClassLayouts(Coff(coff(debug_types(4))))
    second = codeview.ClassLayouts(Coff(coff(debug_types(8))))
    assert first.layout("Widget")["members"] == {"slot_": 4}
    assert second.layout("Widget")["members"] == {"slot_": 8}
    assert first.layout("Widget")["members"] == {"slot_": 4}


@pytest.mark.parametrize("options", [{"bad_fields": True}, {"duplicate": True}])
def test_unmeasurable_class_does_not_erase_valid_siblings(options):
    index = codeview.ClassLayouts(Coff(coff(sibling_types(**options))))
    for _ in range(2):
        with pytest.raises(ValueError) as failure:
            index.layout("Sibling")
        assert not isinstance(failure.value, codeview.ClassNotEmitted)
        assert index.layout("Widget")["members"] == {"slot_": 4}


@pytest.mark.parametrize("blob", [debug_types()[:-1], b"\x01\0\0\0" + debug_types()[4:],
                                  debug_types() + record(0x1005, b"\0")])
def test_index_never_accepts_a_partial_or_unsupported_type_stream(blob):
    with pytest.raises(ValueError):
        codeview.ClassLayouts(Coff(coff(blob)))


def test_pch_does_not_supply_a_standalone_index():
    with pytest.raises(ValueError, match="standalone"):
        codeview.ClassLayouts(Coff(coff(section=b".debug$P")))


def test_empty_valid_stream_provides_absence_not_an_empty_verified_class():
    index = codeview.ClassLayouts(Coff(coff(struct.pack("<I", 2))))
    with pytest.raises(codeview.ClassNotEmitted):
        index.layout("Widget")


def test_product_layout_check_indexes_each_fresh_object_once(layout_project, monkeypatch):
    path = layout_project / "code/types.h"
    path.write_text(path.read_text() + "// RECOVERY_MEMBER: v1 Sibling slot_ 0x8 0x00401000\n")
    original, calls = codeview._type_records, []
    def observed(obj):
        calls.append(obj)
        return original(obj)
    monkeypatch.setattr(codeview, "_type_records", observed)
    report = check_layout(layout_project, compiler=lambda *_: coff(sibling_types()))
    assert report["result"] == "pass" and report["counts"] == {"pass": 2}
    assert len(calls) == report["compile_attempts"] == 2


def test_task_layout_check_indexes_each_fresh_object_once(layout_project, monkeypatch):
    path = layout_project / "code/types.h"
    path.write_text(path.read_text() + "// RECOVERY_MEMBER: v1 Sibling slot_ 0x8 0x00401000\n")
    TaskStore(layout_project).create(Task("owner", "example", "coordinator", "Recover ownership", "b" * 40,
                                          (Origin("a" * 64, "pe:va", 0x401000),),
                                          ("code/f.cc", "code/types.h"), ("synthetic",)))
    original, calls = codeview._type_records, []
    def observed(obj):
        calls.append(obj)
        return original(obj)
    monkeypatch.setattr(codeview, "_type_records", observed)
    report = verify_layout(layout_project, "owner", compiler=lambda *_: coff(sibling_types()))
    assert report["result"] == "pass" and report["counts"] == {"pass": 4, "fail": 0, "unknown": 0}
    assert len(calls) == report["compile_attempts"] == 2
