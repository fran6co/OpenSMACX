"""Same-TU witness controls, with synthetic standalone objects only."""

import re
import struct
import json

import pytest

from match_recovery.application_layout import verify_layout
from match_recovery.application_check import check_task
from match_recovery.layout_check import check_layout
from match_recovery.layout_witness import missing_layout_witnesses
from match_recovery.pe_coff import Coff
from test_application_commands import combined_object, project
from test_codeview import coff, debug_types
from test_layout_check import add_unit
from test_symbol_check import project as product_project
from test_restoration import historical


ABSENT = combined_object(debug_types().replace(b"Widget", b"OtherX"))


def probe_object(symbol, *, changed=False, offset=4):
    data = bytearray(combined_object(debug_types(offset)))
    start, = struct.unpack_from("<I", data, 8)
    data[start:start + 8] = struct.pack("<II", 0, 4)
    data[100] = 0x90 if changed else 0xC3
    name = symbol.encode() + b"\0"
    return bytes(data[:-4]) + struct.pack("<I", len(name) + 4) + name


class Compiler:
    def __init__(self, root, *, damage=None, offset=4):
        self.root, self.damage, self.offset, self.calls = root, damage, offset, []

    def __call__(self, *_):
        return ABSENT

    def overlay(self, source, replacements, command, flags):
        self.calls.append((source, replacements, command, flags))
        data, = replacements.values()
        assert len(replacements) == 1 and source.read_bytes() == data[:len(source.read_bytes())]
        if self.damage == "source_change":
            source.write_text("concurrent source edit")
        if data == source.read_bytes():
            return coff() if self.damage == "replica" else ABSENT
        if self.damage == "ignore":
            return ABSENT
        if self.damage == "invisible":
            raise ValueError("class not declared in this TU")
        symbol = "_" + re.search(rb'int (match_recovery_layout_\w+)\(', data)[1].decode()
        changed = data.endswith(b"1; }\n")
        return probe_object(symbol, changed=changed and self.damage != "control",
                            offset=8 if changed and self.damage == "layout" else self.offset)


def run(root, compiler, **kwargs):
    return missing_layout_witnesses(root, "code/unit.cc", ["Widget"], Coff(ABSENT), compiler,
                                    ("argument",), ("flags", "-types"), **kwargs)


def test_witness_preserves_source_and_uses_same_recipe_and_two_controls(project):
    compiler = Compiler(project)
    original = (project / "code/unit.cc").read_bytes()
    result = run(project, compiler)
    row, = result["witnesses"]
    assert result["compile_attempts"] == len(compiler.calls) == 3
    assert row["result"] == row["replica_control"] == row["probe_control"] == "pass"
    assert row["compiler_layout"]["members"] == {"slot_": 4}
    assert all(args == ("argument",) and flags == ("flags", "-types") for _, _, args, flags in compiler.calls)
    assert (project / "code/unit.cc").read_bytes() == original
    assert b"#include" not in compiler.calls[1][1]["code/unit.cc"][len(original):]


@pytest.mark.parametrize("damage", ["replica", "ignore", "invisible", "control", "layout"])
def test_failed_probe_or_control_never_supplies_a_layout(project, damage):
    row, = run(project, Compiler(project, damage=damage))["witnesses"]
    assert row["result"] == "unknown" and row["compiler_layout"] is None


def test_unsupported_overlay_is_explicit_unknown_without_compile(project):
    result = run(project, lambda *_: ABSENT)
    assert result["compile_attempts"] == 0
    assert result["witnesses"][0]["result"] == "unknown"


def test_concurrent_edit_is_not_restored_or_accepted(project):
    result = run(project, Compiler(project, damage="source_change"))
    assert result["witnesses"][0]["result"] == "unknown"
    assert (project / "code/unit.cc").read_text() == "concurrent source edit"


@pytest.mark.parametrize("types", [debug_types(), debug_types(duplicate=True), debug_types(tail=b"\xff\x7f")])
def test_observed_or_unparseable_definition_cannot_be_replaced(project, types):
    compiler = Compiler(project)
    with pytest.raises(ValueError):
        missing_layout_witnesses(project, "code/unit.cc", ["Widget"], Coff(coff(types)), compiler, (), ())
    assert compiler.calls == []


@pytest.mark.parametrize("names", [None, "Widget", [{}], [], ["Widget"] * 2,
                                   ["Widget; void injected()"], ["A<B>"], ["évil"]])
def test_names_cannot_inject_source(project, names):
    with pytest.raises(ValueError):
        missing_layout_witnesses(project, "code/unit.cc", names, Coff(ABSENT), Compiler(project), (), ())


@pytest.mark.parametrize("flag", ["/Yuf.h", "/Ycf.h", "/Fpf.pch", "@args"])
def test_standalone_probe_cannot_borrow_precompiled_context(project, flag):
    with pytest.raises(ValueError, match="standalone"):
        missing_layout_witnesses(project, "code/unit.cc", ["Widget"], Coff(ABSENT), Compiler(project), (), (flag,))


@pytest.mark.parametrize("offset,expected", [(4, "pass"), (8, "fail")])
def test_task_checks_probe_against_source_expectation_not_just_successful_compile(project, offset, expected):
    compiler = Compiler(project, offset=offset)
    assert verify_layout(project, "owner", compiler=compiler)["result"] == "unknown"
    assert compiler.calls == []  # No extra work without explicit opt-in.
    result = verify_layout(project, "owner", compiler=compiler, witness_missing=True)
    assert result["result"] == expected and result["compile_attempts"] == 4
    assert result["coverage"][0]["observation"] == "witnessed"
    assert result["structure_verification"] == "unknown" and not result["scope_complete"]


def test_task_does_not_probe_an_observed_counterexample(project):
    class Observed(Compiler):
        def __call__(self, *_):
            return combined_object(debug_types(8))
    compiler = Observed(project)
    result = verify_layout(project, "owner", compiler=compiler, witness_missing=True)
    assert result["result"] == "fail" and compiler.calls == []


def test_cli_forwards_opt_in_to_native_task_checker(project, monkeypatch, capsys):
    from match_recovery.cli import main
    monkeypatch.setattr("match_recovery.application_layout.Compiler", lambda *_, **__: Compiler(project))
    assert main(["tasks", "check-layout", "owner", "--root", str(project), "--witness-missing"]) == 0


def test_product_keeps_missing_units_and_failed_witnesses_unknown(product_project):
    (product_project / "code/types.h").write_text("// RECOVERY_MEMBER: v1 Widget slot_ 0x4 0x00401000\n")
    add_unit(product_project)
    compiler = Compiler(product_project)
    report = check_layout(product_project, units=["code/f.cc"], compiler=compiler, witness_missing=True)
    assert report["counts"] == {"pass": 1} and report["result"] == "unknown"
    assert report["omitted_units"] == ["code/g.cc"]
    compiler.damage = "control"
    report = check_layout(product_project, compiler=compiler, witness_missing=True)
    assert report["result"] == "unknown" and report["errors"]


def test_joint_check_automatically_probes_missing_layout_without_reusing_control_objects(project):
    compiler = Compiler(project)
    for _ in range(2):
        report = check_task(project, "owner", project / "original.exe", compiler=compiler)
        assert report["checks"]["binary"]["result"] == report["checks"]["layout"]["result"] == "pass"
        assert report["compilation"]["attempts"] == 4
        assert report["compilation"]["overlay_attempts"] == 3
        assert report["compilation"]["reused_objects"] == 1
        assert report["checks"]["layout"]["witnessed_definitions"] == 1
        assert not report["ready_for_collection"] and report["structure_verification"] == "unknown"
    assert len(compiler.calls) == 6  # Fresh controls on every invocation.


@pytest.mark.parametrize("damage", ["replica", "ignore", "invisible", "control", "layout"])
def test_joint_check_never_lends_product_bytes_to_a_failed_overlay_control(project, damage):
    report = check_task(project, "owner", project / "original.exe", compiler=Compiler(project, damage=damage))
    assert report["checks"]["binary"]["result"] == "pass"
    assert report["checks"]["layout"]["result"] == report["result"] == "unknown"


def test_joint_check_without_overlay_capability_still_reports_the_binary_result(project):
    report = check_task(project, "owner", project / "original.exe", compiler=lambda *_: ABSENT)
    assert report["checks"]["binary"]["result"] == "pass"
    assert report["checks"]["layout"]["result"] == "unknown"
    assert report["compilation"]["attempts"] == 1 and report["compilation"]["overlay_attempts"] == 0


@pytest.mark.parametrize("damage", ["header", "image", "task", "policy", "new_header"])
def test_changed_inputs_during_probe_refuse_the_entire_joint_snapshot(project, damage):
    path = project / {"header": "code/unit.hpp", "image": "original.exe",
                      "task": "recovery/tasks/owner.json", "policy": "recovery/project.json",
                      "new_header": "code/new.hpp"}[damage]
    class Changed(Compiler):
        def overlay(self, *args):
            value = super().overlay(*args)
            if damage == "task":
                task = json.loads(path.read_text())
                task["intent"] = "concurrent task decision"
                path.write_text(json.dumps(task))
            else:
                path.write_bytes((path.read_bytes() if path.exists() else b"") + b"\n")
            return value
    with pytest.raises(ValueError, match="changed"):
        check_task(project, "owner", project / "original.exe", compiler=Changed(project))


def test_restoration_consumes_witnesses_without_reclassifying_them_as_product_observations(historical):
    from match_recovery.restoration import measure_restoration
    path = historical / "code/unit.hpp"
    path.write_text(path.read_text().replace("0x00401010", "0x00401000"))
    report = measure_restoration(historical, "owner", historical / "original.exe", compiler=Compiler(historical))
    declared, = [row for row in report["details"] if row["dimension"] == "declared_structure"]
    assert declared["result"] == "pass"
    requirement, = [row for row in report["structural_requirements"] if row["current"]]
    assert requirement["result"] == "pass"
    assert requirement["trials"][0]["layout_basis"] == "same_tu_probe"
    assert not report["ready_for_collection"] and not report["scope_complete"]
