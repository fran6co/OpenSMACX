"""Whole-product layout controls use synthetic CodeView, never game fixtures."""

import base64
import json
import sys

import pytest

from match_recovery.application_layout import check_member
from match_recovery.cli import main
from match_recovery.layout_check import check_layout
from match_recovery.layout_source import parse_members
from test_codeview import coff, debug_types, hierarchy_types
from test_symbol_check import project


@pytest.fixture
def layout_project(project):
    (project / "code/types.h").write_text("// RECOVERY_MEMBER: v1 Widget slot_ 0x4 0x00401000\n")
    return project


def add_unit(root, name="code/g.cc"):
    (root / name).parent.mkdir(parents=True, exist_ok=True)
    (root / name).write_text("void g() {}\n")
    path = root / "build/compile_commands.json"
    entries = json.loads(path.read_text())
    entries.append({**entries[0], "file": name})
    path.write_text(json.dumps(entries))


def test_every_product_tu_and_flag_set_is_compiled_once_for_all_expectations(layout_project):
    add_unit(layout_project)
    path = layout_project / "code/types.h"
    path.write_text("// RECOVERY_MEMBER: v2 Widget owned Base 0x4 0x00401000\n"
                    "// RECOVERY_MEMBER: v3 Widget ptr Base* 0x8 0x00401000\n"
                    "// RECOVERY_BASE: v1 Widget Base 0x0 0x00401000\n")
    original = path.read_bytes()
    calls = []
    def compiler(path, args, flags):
        calls.append((path.name, args, flags))
        return coff(hierarchy_types())
    result = check_layout(layout_project, compiler=compiler)
    assert result["result"] == "pass" and result["counts"] == {"pass": 3}
    assert result["compile_attempts"] == 4 and len(result["observations"]) == 12
    assert calls == [(name, (), (flag,)) for name in ("f.cc", "g.cc") for flag in ("first", "second")]
    assert path.read_bytes() == original and not result["persisted"] and not result["ready_for_collection"]
    assert result["structure_verification"] == "unknown"


@pytest.mark.parametrize("damage", ["later_tu", "later_flags", "failure_and_unknown"])
def test_good_layout_cannot_hide_a_conflicting_definition(layout_project, damage):
    add_unit(layout_project)
    def compiler(path, _, flags):
        if damage == "failure_and_unknown" and path.name == "g.cc":
            return b"truncated"
        bad = path.name == "g.cc" if damage == "later_tu" else flags == ("second",)
        return coff(debug_types(8 if bad else 4))
    result = check_layout(layout_project, compiler=compiler)
    assert result["result"] == "fail" and result["counts"] == {"fail": 1}
    assert any(row["result"] == "fail" for row in result["observations"])


def test_known_wrong_offset_is_still_failure_when_member_type_is_unknown():
    item, = parse_members("// RECOVERY_MEMBER: v3 Widget ptr Base* 0x8 0x00401000",
                          "code/types.h", "a" * 64, address_space="pe:va")
    result = check_member(item, {"members": {"ptr": 4}, "member_types": {}})
    assert result["type_result"] == "unknown" and result["offset_result"] == result["result"] == "fail"


@pytest.mark.parametrize("bad", [None, b"", "object", b"truncated", coff(section=b".debug$P"),
                                  coff(debug_types(duplicate=True))])
def test_unobservable_tu_cannot_borrow_good_layouts_from_its_neighbor(layout_project, bad):
    add_unit(layout_project)
    result = check_layout(layout_project, compiler=lambda path, *_: bad if path.name == "g.cc" else coff())
    assert result["result"] == "unknown" and result["counts"] == {"unknown": 1}
    assert result["errors"]


def test_valid_nonemission_is_reported_but_is_not_an_abi_independence_proof(layout_project):
    add_unit(layout_project)
    absent = coff(debug_types().replace(b"Widget", b"OtherX"))
    result = check_layout(layout_project, compiler=lambda path, *_: absent if path.name == "g.cc" else coff())
    assert result["result"] == "pass" and result["counts"] == {"pass": 1}
    assert len([r for r in result["coverage"] if r["observation"] == "not_emitted"]) == 2
    assert result["structure_verification"] == "unknown"


def test_each_flag_set_needs_a_witness_not_just_one_passing_spelling(layout_project):
    absent = coff(debug_types().replace(b"Widget", b"OtherX"))
    result = check_layout(layout_project, compiler=lambda _, __, flags: absent if flags == ("second",) else coff())
    assert result["result"] == "unknown"
    assert result["expectations"][0]["missing_flag_sets"] == [["second"]]


def test_ragged_profile_trials_keep_absence_separate_from_actual_profile_witness(layout_project):
    add_unit(layout_project)
    config_path = layout_project / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["measurement"]["unit_flag_sets"] = {"code/f.cc": [0], "code/g.cc": [1]}
    config_path.write_text(json.dumps(config))
    calls = []
    absent = coff(debug_types().replace(b"Widget", b"OtherX"))

    def compile(path, _, flags):
        calls.append((path.name, flags))
        return absent if path.name == "g.cc" else coff()

    result = check_layout(layout_project, compiler=compile)
    assert result["result"] == "unknown"
    assert calls == [("f.cc", ("first",)), ("g.cc", ("second",))]
    assert result["expectations"][0]["missing_flag_sets"] == [["second"]]


def test_diagnostic_selection_never_waives_omitted_product_units(layout_project):
    add_unit(layout_project)
    result = check_layout(layout_project, units=["code/f.cc"], compiler=lambda *_: coff())
    assert result["result"] == "unknown" and result["counts"] == {"pass": 1}
    assert result["omitted_units"] == ["code/g.cc"] and result["compile_attempts"] == 2


@pytest.mark.parametrize("name", ["code/seeds/g.cc", "code/g.h", "elsewhere/g.cc"])
def test_unsupported_or_seed_build_input_is_not_silently_ignored(layout_project, name):
    add_unit(layout_project, name)
    result = check_layout(layout_project, compiler=lambda *_: coff())
    assert result["result"] == "unknown" and result["unsupported_build_inputs"] == [name]


def test_seed_expectations_are_hints_not_additional_product_requirements(layout_project):
    path = layout_project / "code/seeds/types.h"
    path.parent.mkdir()
    path.write_text("// RECOVERY_MEMBER: v1 Widget slot_ 0x8 0x00401000\n")
    assert check_layout(layout_project, compiler=lambda *_: coff())["counts"] == {"pass": 1}


def test_external_build_inputs_keep_whole_product_coverage_incomplete(layout_project, tmp_path_factory):
    external = tmp_path_factory.mktemp("external") / "external.cc"
    external.write_text("void external() {}\n")
    database = layout_project / "build/compile_commands.json"
    entries = json.loads(database.read_text())
    entries.append({**entries[0], "file": str(external)})
    database.write_text(json.dumps(entries))
    result = check_layout(layout_project, compiler=lambda *_: coff())
    assert result["result"] == "unknown" and result["counts"] == {"pass": 1}
    assert result["external_build_inputs"] == [str(external)]


def test_changed_compiler_environment_refuses_mixed_layout_evidence(layout_project, monkeypatch):
    def compiler(*_):
        monkeypatch.setenv("PATH", "/deliberately/different/compiler-search")
        return coff()
    with pytest.raises(ValueError, match="changed"):
        check_layout(layout_project, compiler=compiler)


@pytest.mark.parametrize("units", [[], "code/f.cc", ["code/types.h"], ["../f.cc"], ["code/f.cc"] * 2])
def test_invalid_unit_selection_is_refused(layout_project, units):
    with pytest.raises(ValueError):
        check_layout(layout_project, units=units, compiler=lambda *_: coff())


@pytest.mark.parametrize("damage", ["header", "source", "database", "policy", "pin", "new_header"])
def test_changed_inputs_refuse_mixed_evidence(layout_project, damage):
    path = layout_project / {"header": "code/types.h", "source": "code/f.cc",
                             "database": "build/compile_commands.json", "policy": "recovery/project.json",
                             "pin": "code/PIN", "new_header": "code/new.h"}[damage]
    def compiler(*_):
        path.write_bytes((path.read_bytes() if path.exists() else b"") + b"\n")
        return coff()
    with pytest.raises(ValueError, match="changed"):
        check_layout(layout_project, compiler=compiler)


def test_no_expectations_is_unavailable_not_vacuous_pass(project):
    with pytest.raises(ValueError, match="no source-owned"):
        check_layout(project, compiler=lambda *_: coff())


@pytest.mark.parametrize("flag", ["/Yufoo.h", "-Ycfoo.h", "/Fpfoo.pch", "@flags.rsp"])
def test_pch_and_response_files_refuse_standalone_evidence(layout_project, flag):
    def compiler(*_):
        pytest.fail("must refuse before compilation")
    result = check_layout(layout_project, compiler=compiler, command_for=lambda _: (flag,))
    assert result["result"] == "unknown" and result["compile_attempts"] == 0


@pytest.mark.parametrize("offset,expected", [(4, 0), (8, 1)])
def test_installed_cli_uses_local_compiler_contract_and_summary_keeps_failures(layout_project, capsys, offset, expected):
    response = base64.b64encode(coff(debug_types(offset))).decode()
    (layout_project / "recovery/compiler.py").write_text(
        "import json,sys\nr=json.load(sys.stdin)\n"
        "assert r['profile']=='test' and r['source']=='code/f.cc'\n"
        "print(json.dumps({'object_base64':" + repr(response) + "}))\n")
    path = layout_project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["compiler"] = {"command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 10},
                          "inputs": ["recovery/compiler.py"], "environment": []}
    path.write_text(json.dumps(config))
    assert main(["check-layout", "--root", str(layout_project), "--summary"]) == expected
    report = json.loads(capsys.readouterr().out)
    assert len(report["observations"]) == (2 if expected else 0)
    assert "coverage" not in report


def test_cli_returns_three_for_missing_compiler_evidence(layout_project, monkeypatch, capsys):
    import match_recovery.layout_check as module
    monkeypatch.setattr(module, "Compiler", lambda *_, **__: lambda *_: b"truncated")
    assert main(["check-layout", "--root", str(layout_project)]) == 3
    assert json.loads(capsys.readouterr().out)["counts"] == {"unknown": 1}


@pytest.mark.parametrize("available", [True, False])
def test_trial_progress_preserves_coverage_and_verdicts(layout_project, available):
    add_unit(layout_project)
    compiler = lambda *_: coff() if available else b"truncated"
    before = check_layout(layout_project, compiler=compiler)
    events = []
    observed = check_layout(layout_project, compiler=compiler, progress=events.append)
    assert observed == before
    starts = [event for event in events if event["event"] == "trial_start"]
    finishes = [event for event in events if event["event"] == "trial_finish"]
    assert [event["trial"] for event in starts] == [event["trial"] for event in finishes] == [1, 2, 3, 4]
    assert [event["unit"] for event in starts] == ["code/f.cc", "code/f.cc", "code/g.cc", "code/g.cc"]
    assert [event["flags"] for event in starts] == [["first"], ["second"], ["first"], ["second"]]
    assert all(event["total"] == 4 and "result" not in event for event in events)
    assert all(event["elapsed_seconds"] >= 0 for event in finishes)


def test_progress_cannot_hide_input_changes_before_compilation(layout_project):
    path = layout_project / "code/types.h"
    def progress(_):
        path.write_text("preserve concurrent input")
    with pytest.raises(ValueError, match="changed"):
        check_layout(layout_project, compiler=lambda *_: pytest.fail("compiled changed inputs"), progress=progress)
    assert path.read_text() == "preserve concurrent input"


def test_cli_layout_progress_stays_out_of_json(layout_project, monkeypatch, capsys):
    import match_recovery.layout_check as module
    monkeypatch.setattr(module, "Compiler", lambda *_, **__: lambda *_: coff())
    assert main(["check-layout", "--root", str(layout_project), "--summary"]) == 0
    captured = capsys.readouterr()
    assert json.loads(captured.out)["result"] == "pass"
    assert "layout: trial 1/2 started: code/f.cc" in captured.err
    assert "layout: trial 2/2 finished: code/f.cc" in captured.err
