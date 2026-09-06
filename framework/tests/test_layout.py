import struct

import pytest

from match_recovery.adapters.opensmacx_layout import verify_layout
from match_recovery.adapters.vc6_layout import Cursor, class_layout, direct_members
from match_recovery.layout_source import read_members
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
    assert class_layout(obj, "Widget") == {"class": "Widget", "size": 16, "members": {"slot_": 4}}


@pytest.mark.parametrize("blob", [debug_types(tail=b"\xff\x7f"), debug_types(16),
                                  debug_types(duplicate=True), debug_types()[:-1],
                                  b"\x01\0\0\0" + debug_types()[4:]])
def test_partial_unsupported_or_ambiguous_layout_is_not_proof(blob):
    with pytest.raises(ValueError):
        class_layout(Coff(coff(blob)), "Widget")


def test_pch_and_missing_class_are_refused():
    with pytest.raises(ValueError, match="standalone"):
        class_layout(Coff(coff(section=b".debug$P")), "Widget")
    with pytest.raises(ValueError, match="one complete"):
        class_layout(Coff(coff()), "Absent")


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


def test_duplicate_or_malformed_original_expectations_are_refused(project):
    root, task = prepare(project)
    text = (root / "src/widget.h").read_text()
    (root / "src/widget.h").write_text(text + text)
    with pytest.raises(ValueError, match="duplicate"):
        read_members(root, task.subjects[0].image_sha256)
    (root / "src/widget.h").write_text("// RECOVERY_MEMBER: broken\n")
    with pytest.raises(ValueError, match="invalid"):
        read_members(root, task.subjects[0].image_sha256)
