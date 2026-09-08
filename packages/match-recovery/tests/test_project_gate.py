"""The package gate owns coverage, sequencing, refusal and input freshness."""

import json
from pathlib import Path
import sys

import pytest

from match_recovery import gate
from match_recovery.cli import main
from test_claim_integrity import configuration
from test_match_check import mapped, project


NATIVE = ("check_source", "check_mappings", "check_claims", "check_build", "check_matches",
          "check_symbols", "check_data", "check_layout")
COMPILED = ("check_matches", "check_symbols", "check_data", "check_layout")


@pytest.fixture
def configured(mapped):
    (mapped / "recovery/gate.json").write_text(json.dumps({
        "schema_version": 1, "checks": {}, "inputs": [], "input_roots": [], "environment": []}))
    (mapped / "recovery/build.json").write_text(json.dumps({
        "schema_version": 1, "artifacts": "build/products",
        "commands": [{"argv": [sys.executable, "-c", "pass", "{build}"], "timeout": 10}],
        "outputs": [{"path": "sample.exe", "format": "file"}],
        "inputs": [], "input_roots": ["engine"], "environment": []}))
    (mapped / "recovery/audits.json").write_text(json.dumps({"schema_version": 1, "audits": {}}))
    configuration(mapped, lambda value: value.update(gate="recovery/gate.json", build="recovery/build.json",
                                                      audits="recovery/audits.json"))
    return mapped


@pytest.fixture
def observed(monkeypatch):
    calls = []
    for name in NATIVE:
        def check(*args, _name=name, **kwargs):
            calls.append((_name, args, kwargs))
            return {"result": "pass"}
        monkeypatch.setattr(gate, name, check)
    return calls


def run(root, **kwargs):
    return gate.check_project(root, Path("original/input.exe"), **kwargs)


def edit_gate(root, update):
    path = root / "recovery/gate.json"
    value = json.loads(path.read_text())
    update(value)
    path.write_text(json.dumps(value))


def command(root, code):
    edit_gate(root, lambda value: value["checks"].update(control={
        "argv": [sys.executable, "-c", code], "timeout": 2}))


def test_all_native_stages_run_even_with_no_project_commands(configured, observed):
    events = []
    report = run(configured, progress=events.append, jobs=2)
    assert [name for name, _, _ in observed] == list(NATIVE)
    assert report["result"] == "pass" and report["exit_code"] == 0
    assert len(report["checks"]) == len(NATIVE) and len(events) == 2 * len(NATIVE)
    assert not report["ready_for_collection"] and not report["persisted"]
    assert observed[0][2]["jobs"] == observed[4][2]["jobs"] == 2


def test_acceptance_can_retain_the_same_native_build(configured, observed):
    assert run(configured, retain_build=True)["result"] == "pass"
    assert next(kwargs for name, _, kwargs in observed if name == "check_build") == {"retain": True}


@pytest.mark.parametrize("name", NATIVE)
@pytest.mark.parametrize("result,code", [("fail", 1), ("unknown", 3)])
def test_each_native_check_participates_in_the_exit_code(configured, observed, monkeypatch, name, result, code):
    monkeypatch.setattr(gate, name, lambda *a, **k: {"result": result})
    report = run(configured)
    assert report["exit_code"] == code and report["result"] == result
    assert len(report["checks"]) == len(NATIVE)


@pytest.mark.parametrize("result", ["fail", "unknown"])
def test_failed_or_unavailable_fresh_build_prevents_compiler_sweeps(configured, observed, monkeypatch, result):
    monkeypatch.setattr(gate, "check_build", lambda *_: {"result": result})
    for name in COMPILED:
        monkeypatch.setattr(gate, name, lambda *a, **k: pytest.fail("compiled after unsuccessful build"))
    report = run(configured)
    assert all(row["result"] == "unknown" for row in report["checks"][-4:])
    assert all("not run" in row["report"]["reason"] for row in report["checks"][-4:])


def test_known_failure_dominates_other_unknown_evidence(configured, observed, monkeypatch):
    monkeypatch.setattr(gate, "check_mappings", lambda *_: {"result": "fail"})
    monkeypatch.setattr(gate, "check_layout", lambda *_: {"result": "unknown"})
    assert run(configured)["exit_code"] == 1


@pytest.mark.parametrize("report", [{}, {"result": "ready"}, None])
def test_missing_or_invalid_checker_reports_cannot_pass(configured, observed, monkeypatch, report):
    monkeypatch.setattr(gate, "check_claims", lambda *_: report)
    result = run(configured)
    assert result["exit_code"] == 3
    assert "supported result" in result["checks"][2]["report"]["reason"]


def test_native_refusal_remains_visible_and_other_checks_still_run(configured, observed, monkeypatch):
    def refuse(*_):
        raise ValueError("missing original evidence")
    monkeypatch.setattr(gate, "check_data", refuse)
    report = run(configured)
    assert report["exit_code"] == 3
    assert report["checks"][-2]["report"]["reason"] == "missing original evidence"
    assert observed[-1][0] == "check_layout"


@pytest.mark.parametrize("code,result", [(0, "pass"), (1, "fail"), (3, "unknown"), (2, "unknown"), (127, "unknown")])
def test_additional_project_commands_preserve_exit_status(configured, observed, code, result):
    command(configured, f"import sys; print('control output'); sys.exit({code})")
    report = run(configured)
    row, = [row for row in report["checks"] if row["name"] == "project:control"]
    assert row["result"] == result and row["report"]["exit_code"] == code
    assert row["report"]["stdout"] == "control output\n"
    assert report["result"] == result


def test_timed_out_project_command_is_unknown(configured, observed):
    command(configured, "import time; time.sleep(10)")
    report = run(configured)
    assert report["exit_code"] == 3
    row, = [row for row in report["checks"] if row["name"] == "project:control"]
    assert row["result"] == "unknown" and "timed out" in row["report"]["reason"]


def test_every_configured_audit_runs_without_a_second_name_list(configured, observed, monkeypatch):
    path = configured / "recovery/audits.json"
    path.write_text(json.dumps({"schema_version": 1, "audits": {"first": {}, "new-audit": {}}}))
    calls = []
    def audit(root, name):
        calls.append(name)
        return {"result": "fail" if name == "new-audit" else "pass"}
    monkeypatch.setattr(gate, "audit_source", audit)
    report = run(configured)
    assert calls == ["first", "new-audit"] and report["exit_code"] == 1


@pytest.mark.parametrize("change", ["source", "policy", "new-file", "missing-input", "image"])
def test_mid_gate_input_changes_stop_the_gate_without_reverting(configured, observed, change):
    target = {"source": "engine/types.hpp", "policy": "recovery/gate.json", "new-file": "new-note.md",
              "missing-input": "engine/types.hpp", "image": "original/input.exe"}[change]
    action = (f"Path({target!r}).unlink()" if change == "missing-input" else
              f"Path({target!r}).write_text('preserve this concurrent edit')")
    command(configured, "from pathlib import Path; " + action)
    report = run(configured)
    assert report["exit_code"] == 3 and report["checks"][-1]["name"] == "freshness"
    assert not any(name == "check_build" for name, _, _ in observed)
    if change != "missing-input":
        assert (configured / target).read_text() == "preserve this concurrent edit"


def test_declared_environment_change_invalidates_gate(configured, observed, monkeypatch):
    edit_gate(configured, lambda value: value.update(environment=["GATE_CONTROL_VALUE"]))
    def mutate(*_):
        monkeypatch.setenv("GATE_CONTROL_VALUE", "changed")
        return {"result": "pass"}
    monkeypatch.setattr(gate, "check_layout", mutate)
    report = run(configured)
    assert report["exit_code"] == 3 and report["checks"][-1]["name"] == "freshness"


@pytest.mark.parametrize("change", ["no-config", "missing-input", "missing-root", "invalid-version", "unknown-field"])
def test_invalid_configuration_refuses_before_any_checks(configured, observed, change):
    if change == "no-config":
        (configured / "recovery/gate.json").unlink()
    elif change == "missing-input":
        edit_gate(configured, lambda value: value.update(inputs=["missing"]))
    elif change == "missing-root":
        edit_gate(configured, lambda value: value.update(input_roots=["missing"]))
    elif change == "invalid-version":
        edit_gate(configured, lambda value: value.update(schema_version=True))
    else:
        edit_gate(configured, lambda value: value.update(skip=["matches"]))
    with pytest.raises((ValueError, OSError)):
        run(configured)
    assert not observed


def test_cli_reports_all_checks_and_emits_progress_separately(configured, observed, capsys):
    assert main(["check", "--root", str(configured), "--image", "original/input.exe", "--summary"]) == 0
    captured = capsys.readouterr()
    report = json.loads(captured.out)
    assert len(report["checks"]) == len(NATIVE) and "layout: pass" in captured.err
    assert not report["ready_for_collection"]


def test_real_native_checks_cannot_call_an_unimplemented_fixture_complete(configured):
    # No mocked stage: real source/history/database/compile checks and the
    # declared build subprocess. That process exits zero but creates no output.
    report = run(configured)
    by_name = {row["name"]: row for row in report["checks"]}
    assert by_name["source"]["result"] == by_name["mappings"]["result"] == "pass"
    assert by_name["build"]["result"] != "pass" and report["exit_code"] != 0
    assert not report["ready_for_collection"]


def preparation(root, commands=None):
    value = json.loads((root / "recovery/build.json").read_text())
    rows = (root / "build/commands.json").read_text()
    value.update(artifacts="build/preparation", outputs=[{"path": "commands.json", "format": "compilation-database"}],
        commands=commands or [{"argv": [sys.executable, "-c",
            "from pathlib import Path; import sys; (Path(sys.argv[1])/'commands.json').write_text(sys.argv[2])",
            "{build}", rows], "timeout": 10}])
    (root / "recovery/prepare.json").write_text(json.dumps(value))
    configuration(root, lambda project: project.update(preparation="recovery/prepare.json"))


def test_preparation_runs_before_measurement_snapshot_and_missing_database_checks(configured, observed):
    preparation(configured)
    (configured / "build/commands.json").unlink()
    report = run(configured)
    assert report["result"] == "pass" and report["checks"][0]["name"] == "preparation"
    assert report["checks"][0]["report"]["published"]
    assert [name for name, _, _ in observed] == list(NATIVE)


@pytest.mark.parametrize("code,expected", [(0, "unknown"), (1, "fail")])
def test_failed_preparation_stops_before_checks_and_cannot_use_old_database(configured, observed, code, expected):
    previous = (configured / "build/commands.json").read_bytes()
    preparation(configured, [{"argv": [sys.executable, "-c", f"raise SystemExit({code})", "{build}"], "timeout": 10}])
    report = run(configured)
    assert report["result"] == expected and len(report["checks"]) == 1
    assert report["checks"][0]["name"] == "preparation" and not observed
    assert (configured / "build/commands.json").read_bytes() == previous


def test_wrong_image_refuses_before_preparation_writes(configured, observed):
    preparation(configured)
    (configured / "original/input.exe").write_bytes(b"wrong executable")
    with pytest.raises(ValueError, match="pinned"):
        run(configured)
    assert not observed and not (configured / "build/preparation").exists()


def test_generated_context_changes_invalidate_standalone_measurements(configured):
    from match_recovery.measurement import Measurement
    from match_recovery.preparation import prepare_project
    preparation(configured)
    report = prepare_project(configured)
    header = configured / report["directory"] / "generated.h"
    header.write_text("original generated context")
    settings = Measurement.read(configured)
    before = settings.snapshot(configured, {})
    header.write_text("changed generated context")
    assert settings.snapshot(configured, {}) != before
    assert "recovery/prepare.json" in before["inputs"]


def test_generated_context_changes_mid_gate_stop_compiler_sweeps(configured, observed, monkeypatch):
    preparation(configured)
    def mutate(*_):
        directory, = (configured / "build/preparation").glob("build-*")
        (directory / "generated.h").write_text("concurrent generated input")
        return {"result": "pass"}
    monkeypatch.setattr(gate, "check_build", mutate)
    report = run(configured)
    assert report["exit_code"] == 3 and report["checks"][-1]["name"] == "freshness"
    assert not any(name in COMPILED for name, _, _ in observed)


def test_whole_gate_forwards_layout_trial_progress(configured, observed, monkeypatch):
    def layout(root, *, progress):
        progress({"event": "trial_start", "unit": "engine/family.cc", "trial": 1, "total": 1, "flags": []})
        progress({"event": "trial_finish", "unit": "engine/family.cc", "trial": 1, "total": 1,
                  "flags": [], "elapsed_seconds": 0.1})
        return {"result": "pass"}
    monkeypatch.setattr(gate, "check_layout", layout)
    events = []
    report = run(configured, progress=events.append)
    assert report["result"] == "pass"
    assert events[-3]["event"] == "trial_start" and events[-3]["check"] == "layout"
    assert events[-2]["event"] == "trial_finish" and events[-1]["result"] == "pass"
