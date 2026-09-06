import struct

import pytest

from match_recovery.adapters.opensmacx_layout import verify_layout
from match_recovery.adapters.vc6_layout import ClassNotEmitted, Cursor, class_layout, direct_members
from match_recovery.cli import main
from match_recovery.layout_source import parse_bases, parse_members, read_bases, read_members
from match_recovery.pe_coff import Coff
from match_recovery.tasks import TaskStore
from test_obligations import project


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
                                          "member_type_kinds": {"slot_": "non_class_value"}}


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
    ptr_type = record(0x1002, struct.pack("<II", 0x1002, 0))
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


@pytest.mark.parametrize("member, expected_type, offset, expected", [
    ("owned", "Base", 4, "pass"), ("owned", "Other", 4, "fail"),
    ("owned", "Base", 8, "fail"), ("ptr", "Base", 8, "fail"),
])
def test_typed_member_requires_both_offset_and_embedded_class(project, member, expected_type, offset, expected):
    root, task = prepare(project)
    (root / "src/widget.h").write_text(f"// RECOVERY_MEMBER: v2 Widget {member} {expected_type} 0x{offset:X} 0x00401000\n")
    report = verify_layout(root, task.task_id, compiler=lambda *_: coff(hierarchy_types()), command_for=lambda _: ("compiler",))
    assert report["result"] == expected
    assert all(row["result"] == expected for row in report["members"])
    assert report["structure_verification"] == "unknown"


def test_scalar_at_correct_offset_cannot_replace_embedded_object(project):
    root, task = prepare(project)
    (root / "src/widget.h").write_text("// RECOVERY_MEMBER: v2 Widget slot_ Base 0x4 0x00401000\n")
    report = verify_layout(root, task.task_id, compiler=lambda *_: coff(), command_for=lambda _: ("compiler",))
    assert report["result"] == "fail"


def test_array_at_correct_offset_cannot_replace_single_embedded_object(project):
    root, task = prepare(project)
    (root / "src/widget.h").write_text("// RECOVERY_MEMBER: v2 Widget ptr Base 0x8 0x00401000\n")
    pointer_record = record(0x1002, struct.pack("<II", 0x1002, 0))
    array_record = record(0x1003, struct.pack("<IIH", 0x1002, 0x74, 4) + b"\0")
    blob = hierarchy_types().replace(pointer_record, array_record)
    report = verify_layout(root, task.task_id, compiler=lambda *_: coff(blob), command_for=lambda _: ("compiler",))
    assert report["result"] == "fail"
    assert all(row["member_type_kind"] == "array" for row in report["members"])


def test_unsupported_member_type_remains_unknown(project):
    root, task = prepare(project)
    (root / "src/widget.h").write_text("// RECOVERY_MEMBER: v2 Widget slot_ Base 0x4 0x00401000\n")
    blob = debug_types().replace(struct.pack("<I", 0x74), struct.pack("<I", 0x9999))
    report = verify_layout(root, task.task_id, compiler=lambda *_: coff(blob), command_for=lambda _: ("compiler",))
    assert report["result"] == "unknown"


@pytest.mark.parametrize("text", ["v1 Widget slot_ Base 0x4 0x00401000", "v2 Widget slot_ 0x4 0x00401000"])
def test_member_annotation_version_must_agree_with_type_field(text):
    with pytest.raises(ValueError, match="version/type"):
        parse_members("// RECOVERY_MEMBER: " + text, "src/widget.h", "a" * 64)


@pytest.mark.parametrize("blob, expected", [(hierarchy_types(), "pass"),
                                           (hierarchy_types(base_offset=8), "fail"),
                                           (hierarchy_types(virtual=True), "fail"),
                                           (debug_types(), "fail"),
                                           (debug_types().replace(b"Widget", b"Absent"), "unknown")])
def test_declared_base_cannot_be_replaced_by_flattened_fields(project, blob, expected):
    root, task = prepare(project)
    (root / "src/widget.h").write_text("// RECOVERY_BASE: v1 Widget Base 0x0 0x00401000\n")
    report = verify_layout(root, task.task_id, compiler=lambda *_: coff(blob), command_for=lambda _: ("compiler",))
    assert report["result"] == expected
    assert len(report["bases"]) == 2 and report["members"] == []
    assert all(row["result"] == expected for row in report["bases"])
    assert report["structure_verification"] == "unknown"


def test_base_grammar_supports_qualified_names_and_refuses_duplicate_or_malformed_input(project):
    root, task = project
    text = "// RECOVERY_BASE: v1 UI::Widget UI::Base 0x4 0x00401000\n"
    row, = parse_bases(text, "src/widget.h", task.subjects[0].image_sha256)
    assert row.class_name == "UI::Widget" and row.base == "UI::Base" and row.offset == 4
    (root / "src/widget.h").write_text(text * 2)
    with pytest.raises(ValueError, match="duplicate original base"):
        read_bases(root, task.subjects[0].image_sha256)
    with pytest.raises(ValueError, match="invalid original base"):
        parse_bases("// RECOVERY_BASE: unsupported\n", "src/widget.h", task.subjects[0].image_sha256)
    assert parse_bases(text, "src/unrecovered/seed.cpp", task.subjects[0].image_sha256) == ()


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


def prepare(project):
    root, task = project
    (root / "src/widget.h").write_text("// RECOVERY_MEMBER: v1 Widget slot_ 0x4 0x00401000\n")
    store = TaskStore(root)
    store.expand(task.task_id, request_id="layout", author=task.author, subjects=(),
                 editable_files=("src/widget.h",), evidence=("original member evidence",), expected_revision=0)
    return root, task


def test_source_expectations_are_checked_against_every_flag_set(project):
    root, task = prepare(project)
    calls = []
    def compiler(source, command, flags):
        calls.append(flags)
        assert "/Z7" in flags
        return coff()
    report = verify_layout(root, task.task_id, compiler=compiler, command_for=lambda _: ("compiler",))
    assert report["result"] == "pass" and len(calls) == 2
    assert report["structure_verification"] == "unknown" and not report["scope_complete"]
    assert not report["persisted"]


def test_layout_regression_cannot_hide_behind_one_passing_configuration(project):
    root, task = prepare(project)
    report = verify_layout(root, task.task_id,
                           compiler=lambda _, command, flags: coff(debug_types(8 if "/Oy-" in flags else 4)),
                           command_for=lambda _: ("compiler",))
    assert report["result"] == "fail"


def test_pch_command_is_refused_before_compilation(project):
    root, task = prepare(project)
    def forbidden(*_):
        pytest.fail("PCH compilation must not run")
    report = verify_layout(root, task.task_id, compiler=forbidden, command_for=lambda _: ("compiler", "/Yupch.h"))
    assert report["result"] == "unknown" and report["compile_attempts"] == 0


def test_missing_expectations_are_not_a_vacuous_structure_pass(project):
    root, task = project
    report = verify_layout(root, task.task_id, compiler=lambda *_: coff(), command_for=lambda _: ("compiler",))
    assert report["result"] == "unknown" and report["compile_attempts"] == 0


def test_method_hint_exposes_class_before_any_layout_expectation_exists(project):
    root, task = project
    path = root / "src/f.cpp"
    path.write_text(path.read_text().replace("?f@@YAXXZ", "?method@Widget@@QAEXXZ"))
    report = verify_layout(root, task.task_id, compiler=lambda *_: coff(), command_for=lambda _: ("compiler",))
    assert report["compile_attempts"] == 2
    assert report["result"] == "unknown" and report["members"] == []
    assert len(report["coverage"]) == 2
    for observation in report["coverage"]:
        assert observation["basis"] == "mapped_method_hint"
        assert observation["compiler_layout"]["class"] == "Widget"
        assert observation["hint_entities"] == [task.subjects[0].entity_id]
    assert report["structure_verification"] == "unknown"


def test_missing_method_hint_does_not_change_declared_offset_verdict(project):
    root, task = prepare(project)
    path = root / "src/f.cpp"
    path.write_text(path.read_text().replace("?f@@YAXXZ", "?method@Other@@QAEXXZ"))
    report = verify_layout(root, task.task_id, compiler=lambda *_: coff(), command_for=lambda _: ("compiler",))
    assert report["result"] == "pass"  # Only declared Widget offsets are requirements.
    absent = [row for row in report["coverage"] if row["class_name"] == "Other"]
    assert len(absent) == 2 and all(row["observation"] == "not_emitted" for row in absent)
    assert all(row["basis"] == "mapped_method_hint" for row in absent)


@pytest.mark.parametrize("where", ["seed", "outside_scope"])
def test_unrelated_or_seed_class_names_do_not_trigger_product_compilation(project, where):
    root, task = project
    directory = root / ("src/unrecovered" if where == "seed" else "src")
    directory.mkdir(exist_ok=True)
    text = (root / "src/f.cpp").read_text().replace("?f@@YAXXZ", "?method@Widget@@QAEXXZ")
    if where == "outside_scope":
        text = text.replace("00401000", "00402000")
    (directory / "other.cpp").write_text(text)
    report = verify_layout(root, task.task_id, compiler=lambda *_: pytest.fail("unexpected compilation"),
                           command_for=lambda _: ("compiler",))
    assert report["compile_attempts"] == 0 and report["coverage"] == []


def test_duplicate_or_malformed_original_expectations_are_refused(project):
    root, task = prepare(project)
    text = (root / "src/widget.h").read_text()
    (root / "src/widget.h").write_text(text + text)
    with pytest.raises(ValueError, match="duplicate"):
        read_members(root, task.subjects[0].image_sha256)
    (root / "src/widget.h").write_text("// RECOVERY_MEMBER: broken\n")
    with pytest.raises(ValueError, match="invalid"):
        read_members(root, task.subjects[0].image_sha256)


@pytest.mark.parametrize("other, state", [(debug_types().replace(b"Widget", b"Absent"), "not_emitted"),
                                         (debug_types()[:-1], "unmeasurable")])
def test_coverage_distinguishes_missing_types_without_waiving_checks(project, other, state):
    root, task = prepare(project)
    store = TaskStore(root)
    (root / "src/other.cpp").write_text("// synthetic extra task TU\n")
    store.expand(task.task_id, request_id="extra-unit", author=task.author, subjects=(),
                 editable_files=("src/other.cpp",), evidence=("synthetic scope",), expected_revision=1)
    report = verify_layout(root, task.task_id,
                           compiler=lambda path, *_: coff(other if path.name == "other.cpp" else debug_types()),
                           command_for=lambda _: ("compiler",))
    assert report["result"] == "unknown"
    assert report["counts"] == {"pass": 2, "fail": 0, "unknown": 2}
    missing = [row for row in report["coverage"] if row["unit"] == "src/other.cpp"]
    assert len(missing) == 2 and all(row["observation"] == state for row in missing)
    assert all(row["next_action"] for row in missing)


@pytest.mark.parametrize("result, code", [("pass", 0), ("fail", 1), ("unknown", 3)])
def test_layout_summary_preserves_result_and_coverage(monkeypatch, capsys, result, code):
    import json

    monkeypatch.setattr("match_recovery.adapters.opensmacx_layout.verify_layout",
                        lambda *_: {"result": result, "members": ["verbose observations"],
                                    "coverage": ["required coverage"], "structure_verification": "unknown"})
    assert main(["tasks", "check-layout", "fixture", "--summary"]) == code
    report = json.loads(capsys.readouterr().out)
    assert report["coverage"] == ["required coverage"] and "members" not in report
    assert report["structure_verification"] == "unknown"
