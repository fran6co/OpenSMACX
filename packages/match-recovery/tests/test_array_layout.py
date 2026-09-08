"""Bounded class arrays: source interpretations and independent compiler types."""

from dataclasses import replace
import struct

import pytest

from match_recovery.application_check import check_task
from match_recovery.application_layout import check_member, verify_layout
from match_recovery.codeview import class_layout
from match_recovery.layout_source import parse_members
from match_recovery.pe_coff import Coff
from match_recovery.structural_requirements import check_requirements
from test_application_commands import combined_object, project
from test_codeview import coff, hierarchy_types, record
from test_structural_requirements import candidate, inventory


ANNOTATION = "// RECOVERY_MEMBER: v4 Widget ptr Base[2] 0x8 0x8 0x00401000\n"


def expectation(text=ANNOTATION):
    return parse_members(text, "code/unit.hpp", "a" * 64, address_space="pe:va")[0]


def array_types(*, size=8, target=0x1002, complete=True, element_size=4, extra=b"", body=None):
    pointer = record(0x1002, struct.pack("<II", 0x1002, 0xA))
    array = record(0x1003, struct.pack("<IIH", target, 0x74, size) + b"\0" if body is None else body)
    result = hierarchy_types().replace(pointer, array)
    if complete:
        result += record(0x1203, b"")
        result += record(0x1005, struct.pack("<HHIIIH", 0, 0, 0x1004, 0, 0, element_size) + b"\x04Base")
    return result + extra


def observed(**kwargs):
    return class_layout(Coff(coff(array_types(**kwargs))), "Widget")


def test_bounded_array_grammar_and_interpretation_identity():
    item = expectation()
    assert (item.class_array_type, item.array_count, item.array_size, item.offset) == ("Base", 2, 8, 8)
    assert item.class_type is item.class_pointer_type is None
    assert item.interpretation["array_size"] == 8
    assert replace(item, array_count=1).interpretation_id != item.interpretation_id
    assert replace(item, array_size=16).interpretation_id != item.interpretation_id
    namespaced = expectation(ANNOTATION.replace("Widget ptr Base", "UI::Widget ptr UI::Base"))
    assert namespaced.class_array_type == "UI::Base"


@pytest.mark.parametrize("text", [
    "v4 Widget ptr Base[0] 0x8 0x8 0x00401000",
    "v4 Widget ptr Base[02] 0x8 0x8 0x00401000",
    "v4 Widget ptr Base[-2] 0x8 0x8 0x00401000",
    "v4 Widget ptr Base[2] 0x8 0x0 0x00401000",
    "v4 Widget ptr Base[3] 0x8 0x8 0x00401000",
    "v4 Widget ptr Base[2] 0x8 0x00401000",
    "v4 Widget ptr Base[2][2] 0x8 0x8 0x00401000",
    "v4 Widget ptr Base*[2] 0x8 0x8 0x00401000",
    "v2 Widget ptr Base[2] 0x8 0x8 0x00401000",
    "v3 Widget ptr Base* 0x8 0x8 0x00401000",
    "v1 Widget ptr 0x8 0x8 0x00401000",
])
def test_invalid_or_version_inconsistent_arrays_are_refused(text):
    with pytest.raises(ValueError, match="invalid original member"):
        expectation("// RECOVERY_MEMBER: " + text)


def test_array_count_comes_from_complete_element_storage_not_the_member_name():
    row = check_member(expectation(), observed())
    assert row["result"] == row["offset_result"] == row["type_result"] == "pass"
    assert row["array_count_result"] == row["array_size_result"] == "pass"
    assert row["observed_type"] == {"kind": "array", "target": {"kind": "class_value", "class": "Base"},
                                    "size": 8, "element_size": 4, "count": 2}


@pytest.mark.parametrize("options, dimension", [
    ({"size": 4}, "array_size_result"),
    ({"element_size": 8}, "array_count_result"),  # Same total bytes, one larger Base.
    ({"target": 0x74}, "type_result"),  # Same size/count, wrong element type.
])
def test_same_offset_and_partial_type_agreement_do_not_pass(options, dimension):
    row = check_member(expectation(), observed(**options))
    assert row["offset_result"] == "pass" and row[dimension] == row["result"] == "fail"


def test_same_sized_wrong_class_is_not_the_expected_element():
    layout = class_layout(Coff(coff(array_types().replace(b"Base", b"Else"))), "Widget")
    row = check_member(expectation(), layout)
    assert row["array_count_result"] == row["array_size_result"] == "pass"
    assert row["type_result"] == row["result"] == "fail"


def test_single_class_and_pointer_members_cannot_substitute_for_arrays():
    layout = class_layout(Coff(coff(hierarchy_types())), "Widget")
    assert check_member(expectation(), layout)["result"] == "fail"
    assert check_member(replace(expectation(), member="owned", offset=4), layout)["result"] == "fail"


def test_unresolved_forward_element_retains_shape_but_cannot_invent_stride():
    row = check_member(expectation(), observed(complete=False))
    assert row["type_result"] == row["array_size_result"] == "pass"
    assert row["array_count_result"] == row["result"] == "unknown"
    assert row["observed_type"]["element_size"] is None


@pytest.mark.parametrize("options", [
    {"size": 7}, {"size": 0}, {"target": 0x9999}, {"target": 0x1003},
    {"body": b""}, {"body": struct.pack("<IIH", 0x1002, 0x74, 8)},
    {"body": struct.pack("<IIH", 0x1002, 0x74, 8) + b"\0bad"},
])
def test_malformed_or_cyclic_array_records_do_not_supply_partial_proof(options):
    row = check_member(expectation(), observed(**options))
    assert row["offset_result"] == "pass" and row["result"] == "unknown"


def test_array_storage_cannot_extend_past_the_containing_class():
    with pytest.raises(ValueError, match="storage outside"):
        observed(size=12)


def test_array_of_const_class_preserves_unqualified_element_identity():
    types = array_types(target=0x1006, extra=record(0x1001, struct.pack("<IH", 0x1002, 1)))
    assert check_member(expectation(), class_layout(Coff(coff(types)), "Widget"))["result"] == "pass"


def test_all_flag_trials_must_agree_on_array_shape(project):
    import json
    (project / "code/unit.hpp").write_text(ANNOTATION)
    path = project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["flag_sets"].append(["-alternate"])
    path.write_text(json.dumps(config))
    report = verify_layout(project, "owner", compiler=lambda _p, _c, flags:
                           coff(array_types(element_size=8 if "-alternate" in flags else 4)))
    assert [row["result"] for row in report["members"]] == ["pass", "fail"]
    assert report["result"] == "fail"


def test_joint_check_rejects_wrong_array_even_when_function_bytes_pass(project):
    (project / "code/unit.hpp").write_text(ANNOTATION)
    report = check_task(project, "owner", project / "original.exe",
                        compiler=lambda *_: combined_object(array_types(element_size=8)))
    assert report["checks"]["binary"]["result"] == "pass"
    assert report["checks"]["layout"]["result"] == report["result"] == "fail"
    assert not report["ready_for_collection"]


def test_array_change_preserves_its_historical_interpretation_for_review():
    before, after = expectation(), replace(expectation(), array_count=1)
    rows = check_requirements(inventory((before, False), (after, True)), candidate(after))
    old = next(row for row in rows if not row["current"])
    assert old["result"] == "unknown" and old["next_action"] == "review_supersession_or_retraction"
    assert next(row for row in rows if row["current"])["result"] == "pass"
