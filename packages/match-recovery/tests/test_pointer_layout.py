"""Pointer-target obligations, tested without a game, compiler or private bytes."""

import base64
import json
import struct

import pytest

from match_recovery.application_layout import check_member, verify_layout
from match_recovery.application_check import check_task
from match_recovery.cli import main
from match_recovery.codeview import class_layout
from match_recovery.layout_source import parse_members, read_members
from match_recovery.pe_coff import Coff
from test_application_commands import combined_object, project
from test_codeview import coff, hierarchy_types, record


ANNOTATION = "// RECOVERY_MEMBER: v3 Widget ptr Base* 0x8 0x00401000\n"


def pointer_types(target=0x1002, attributes=0xA, *, body=None, extra=b""):
    original = record(0x1002, struct.pack("<II", 0x1002, 0xA))
    replacement = record(0x1002, struct.pack("<II", target, attributes) if body is None else body)
    return hierarchy_types().replace(original, replacement) + extra


def expectation(text=ANNOTATION):
    return parse_members(text, "code/unit.hpp", "a" * 64, address_space="pe:va")[0]


def test_pointer_expectation_is_distinct_from_an_embedded_value():
    item = expectation()
    assert item.class_type is None and item.class_pointer_type == "Base"
    assert item.offset == 8 and item.evidence.address == 0x401000
    namespaced = expectation(ANNOTATION.replace("Widget ptr Base*", "UI::Widget ptr UI::Base*"))
    assert namespaced.class_name == "UI::Widget" and namespaced.class_pointer_type == "UI::Base"


@pytest.mark.parametrize("text", [
    "v1 Widget ptr Base* 0x8 0x00401000", "v2 Widget ptr Base* 0x8 0x00401000",
    "v3 Widget ptr Base 0x8 0x00401000", "v3 Widget ptr 0x8 0x00401000",
    "v3 Widget ptr Base** 0x8 0x00401000", "v3 Widget ptr Base& 0x8 0x00401000",
    "v3 Widget ptr const Base* 0x8 0x00401000", "v4 Widget ptr Base* 0x8 0x00401000",
])
def test_unsupported_or_version_inconsistent_expectations_are_refused(text):
    with pytest.raises(ValueError, match="invalid original member"):
        expectation("// RECOVERY_MEMBER: " + text)


@pytest.mark.parametrize("attributes", [0xA, 0x10A, 0x20A, 0x40A, 0x60A])
def test_plain_or_cv_qualified_pointer_retains_its_unqualified_class_target(attributes):
    layout = class_layout(Coff(coff(pointer_types(attributes=attributes))), "Widget")
    row = check_member(expectation(), layout)
    assert row["result"] == row["type_result"] == row["offset_result"] == "pass"
    assert row["actual_class_pointer_type"] == "Base" and row["actual_class_type"] is None
    assert row["observed_type"] == {"kind": "pointer", "width": 4,
                                    "target": {"kind": "class_value", "class": "Base"}}


def test_const_pointee_does_not_turn_pointer_storage_into_a_class_value():
    types = pointer_types(target=0x1004, extra=record(0x1001, struct.pack("<IH", 0x1002, 1)))
    layout = class_layout(Coff(coff(types)), "Widget")
    assert check_member(expectation(), layout)["result"] == "pass"
    embedded = expectation(ANNOTATION.replace("v3", "v2").replace("Base*", "Base"))
    assert check_member(embedded, layout)["result"] == "fail"


@pytest.mark.parametrize("types, kind", [
    (pointer_types(attributes=0x2A), "reference"),
    (pointer_types(target=0x74), "pointer"),
    (pointer_types(target=0x1001), "pointer"),  # Widget*, not Base*.
    (pointer_types(target=0x1004, extra=record(0x1002, struct.pack("<II", 0x1002, 0xA))), "pointer"),
    (hierarchy_types().replace(struct.pack("<HHIH", 0x1405, 0, 0x1003, 8),
                               struct.pack("<HHIH", 0x1405, 0, 0x74, 8)), "non_class_value"),
    (hierarchy_types().replace(struct.pack("<HHIH", 0x1405, 0, 0x1003, 8),
                               struct.pack("<HHIH", 0x1405, 0, 0x1002, 8)), "class_value"),
    (hierarchy_types().replace(record(0x1002, struct.pack("<II", 0x1002, 0xA)),
                               record(0x1003, struct.pack("<IIH", 0x1002, 0x74, 4) + b"\0")), "array"),
])
def test_wrong_type_at_the_same_offset_is_a_measured_failure(types, kind):
    layout = class_layout(Coff(coff(types)), "Widget")
    row = check_member(expectation(), layout)
    assert row["offset_result"] == "pass" and row["actual_offset"] == 8
    assert row["result"] == row["type_result"] == "fail"
    assert row["member_type_kind"] == kind


@pytest.mark.parametrize("options", [
    {"body": b""}, {"body": struct.pack("<I", 0x1002)},
    {"body": struct.pack("<II", 0x1002, 0xA) + b"bad"},
    {"attributes": 0}, {"attributes": 0x4A}, {"attributes": 0x6A},
    {"attributes": 0x8A}, {"attributes": 0x100A}, {"attributes": 0x8000000A},
    {"target": 0}, {"target": 0x9999}, {"target": 0x1003},
    {"target": 0x1004, "extra": record(0x1001, struct.pack("<IH", 0x1004, 1))},
    {"target": 0x1004, "extra": record(0x1001, struct.pack("<IH", 0x1002, 0x80))},
])
def test_malformed_cyclic_or_unsupported_pointer_types_never_pass(options):
    layout = class_layout(Coff(coff(pointer_types(**options))), "Widget")
    row = check_member(expectation(), layout)
    assert row["offset_result"] == "pass"
    assert row["result"] == row["type_result"] == "unknown"
    assert row["member_type_kind"] == "unknown"


def test_pointer_storage_must_fit_inside_class_extent():
    types = hierarchy_types().replace(struct.pack("<HHIH", 0x1405, 0, 0x1003, 8),
                                       struct.pack("<HHIH", 0x1405, 0, 0x1003, 14))
    with pytest.raises(ValueError, match="storage outside"):
        class_layout(Coff(coff(types)), "Widget")


def test_all_configurations_are_checked_and_disagreement_is_not_hidden(project):
    (project / "code/unit.hpp").write_text(ANNOTATION)
    config = project / "recovery/project.json"
    value = json.loads(config.read_text())
    value["measurement"]["flag_sets"].append(["-alternate"])
    config.write_text(json.dumps(value))
    def compiler(path, command, flags):
        return coff(pointer_types(attributes=0x2A if "-alternate" in flags else 0xA))
    report = verify_layout(project, "owner", compiler=compiler)
    assert report["result"] == "fail" and report["compile_attempts"] == 2
    assert [row["result"] for row in report["members"]] == ["pass", "fail"]
    assert report["structure_verification"] == "unknown" and not report["scope_complete"]


def test_joint_task_check_reports_wrong_pointer_even_when_function_bytes_pass(project):
    (project / "code/unit.hpp").write_text(ANNOTATION)
    report = check_task(project, "owner", project / "original.exe",
                        compiler=lambda *_: combined_object(pointer_types(target=0x1001)))
    assert report["checks"]["binary"]["result"] == "pass"
    assert report["checks"]["layout"]["result"] == report["result"] == "fail"
    assert report["checks"]["layout"]["members"][0]["actual_class_pointer_type"] == "Widget"
    assert report["compilation"]["attempts"] == 1  # One object supplies both checks.
    assert not report["ready_for_collection"]


def test_native_cli_checks_configured_pointer_expectations(project, capsys):
    (project / "code/unit.hpp").write_text(ANNOTATION)
    obj = base64.b64encode(coff(pointer_types())).decode()
    (project / "recovery/compile.py").write_text("import json; print(" + repr(json.dumps({"object_base64": obj})) + ")\n")
    assert main(["tasks", "check-layout", "owner", "--root", str(project)]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["members"][0]["actual_class_pointer_type"] == "Base"
    assert report["members"][0]["expectation"]["class_pointer_type"] == "Base"
    item, = read_members(project, report["members"][0]["expectation"]["evidence"]["image_sha256"])
    assert item.class_pointer_type == "Base"
    (project / "code/unit.hpp").write_text(ANNOTATION.replace("Base*", "Other*"))
    assert main(["tasks", "check-layout", "owner", "--root", str(project)]) == 1
    assert json.loads(capsys.readouterr().out)["members"][0]["type_result"] == "fail"
