"""Native checkpoint orchestration with real history/build and synthetic COFF.

The compiler is a controlled test double; this is not a live agent/game pilot.
Restoration, layout, outside-code comparison and admission policy are real.
"""

from dataclasses import asdict, replace
import io
import json
import sys

import pytest

from match_recovery import migration_check as native
from match_recovery.cli import main
from match_recovery.driver import drive, working_state
from match_recovery.migration_protocol import admit, extend_inventory
from match_recovery.model import Dimension, Obligation
from match_recovery.preservation import check_preservation
from match_recovery.restoration import measure_restoration
from match_recovery.reviews import ReviewStore
from match_recovery.source_obligations import task_obligations
from match_recovery.tasks import TaskStore
from test_application_commands import project, combined_object
from test_codeview import debug_types
from test_task_source import git


class Compiler:
    def __init__(self, root):
        self.root = root

    def __call__(self, source, arguments, flags):
        return self.overlay(source, {}, arguments, flags)

    def overlay(self, source, replacements, arguments, flags):
        contents = {name: replacements.get(name, (self.root / name).read_bytes())
                    for name in ("code/unit.cc", "code/unit.hpp")}
        for data in contents.values():
            if data.startswith(b"#error "):
                raise ValueError(data.splitlines()[0].decode())
        obj = bytearray(combined_object(debug_types(8 if b"wrong-layout" in contents["code/unit.cc"] else 4)))
        if b"regression" in contents["code/unit.cc"]:
            obj[100] = 0xCC
        return bytes(obj)


@pytest.fixture
def migration(project, monkeypatch):
    root = project
    store = TaskStore(root)
    task = store.read("owner")
    (root / "recovery/tasks/owner.json").unlink()
    (root / ".gitignore").write_text("build/\n")
    path = root / "code/unit.cc"
    path.write_text(path.read_text().replace("0x00401001\n", "0x00401001 BYTE_EXACT\n"))
    (root / "recovery/audits.json").write_text(json.dumps({"schema_version": 1, "audits": {}}))
    (root / "recovery/build.json").write_text(json.dumps({
        "schema_version": 1, "artifacts": "build/products", "input_roots": ["code"], "inputs": [], "environment": [],
        "commands": [{"argv": [sys.executable, "-c", "from pathlib import Path; import sys; Path(sys.argv[1], 'result').write_bytes(b'synthetic product')", "{build}"], "timeout": 10}],
        "outputs": [{"path": "result", "format": "file"}]}))
    config = root / "recovery/project.json"
    value = json.loads(config.read_text())
    value.update(build="recovery/build.json", audits="recovery/audits.json")
    config.write_text(json.dumps(value))
    git(root, "init", "--initial-branch=work")
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic native migration baseline")
    task = replace(task, base_revision=git(root, "rev-parse", "HEAD"))
    store.create(task)
    git(root, "add", ".")
    git(root, "commit", "-m", "Authorize owner")
    # These independent checkers have their own tests. Keep orchestration
    # refusal injection separate from the real bounded compiler/history checks.
    for name in ("check_source", "check_mappings", "check_claims"):
        monkeypatch.setattr(native, name, lambda *_: {"result": "pass"})
    monkeypatch.setattr(native, "measure_restoration", lambda root, name, image, **kwargs:
                        measure_restoration(root, name, image, compiler=Compiler(root), **kwargs))
    monkeypatch.setattr(native, "check_preservation", lambda root, name: check_preservation(root, name, compiler=Compiler(root)))
    return root


def request(root):
    task = TaskStore(root).read("owner")
    inventory = task_obligations(root, "owner")
    policy = {}
    snapshot = ReviewStore(root, "owner").snapshot(working_state(root), policy)
    review = {"kind": "review", "author": "worker", "reviewer": "independent", "snapshot": snapshot,
              "decision": "approve", "evidence": ["source:synthetic reviewed change"], "feedback": "Checkpoint, not completion"}
    return {"task": task.to_dict(), "snapshot": review["snapshot"], "review": review,
            "review_policy": policy,
            "protected": inventory["obligations"], "interpretations": inventory["interpretations"],
            "permission": {"migration_id": "layout", "author": task.author, "evidence": ["source:synthetic permission"],
                           "allowed": [{"entity_id": task.subjects[0].entity_id, "dimension": "bytes"}]}}


def measure(root, payload=None):
    return native.measure_migration(root, "owner", root / "original.exe", payload or request(root))


def admission(root, payload, report):
    scope, _ = TaskStore(root).scope("owner")
    protected = {}
    extend_inventory(protected, {"obligations": payload["protected"]}, scope)
    return admit(payload["permission"], tuple(protected.values()), scope, payload["review"], payload["snapshot"], 0, report)


def test_native_measured_checkpoint_tracks_regression_then_restores_same_obligations(migration):
    path = migration / "code/unit.cc"
    before = path.read_text()
    path.write_text(before + "// regression\n")
    payload = request(migration)
    checkpoint = measure(migration, payload)
    assert checkpoint["build"] == checkpoint["structure"] == checkpoint["references"] == checkpoint["outside_scope"] == "pass"
    tracked = admission(migration, payload, checkpoint)
    assert tracked.exit_code == 2 and len(tracked.tracked) == 1
    assert "BYTE_EXACT" in path.read_text() and "regression" in path.read_text()
    path.write_text(before)
    restored = measure(migration)
    assert admission(migration, request(migration), restored).exit_code == 0
    assert set(restored["current"]) == set(checkpoint["current"])
    assert TaskStore(migration).read("owner").base_revision == payload["task"]["base_revision"]


@pytest.mark.parametrize("scope", ["change", "full_project"])
def test_driver_uses_native_checkpoint_then_resumes_without_resetting_baseline(migration, scope):
    task = TaskStore(migration).read("owner")
    permission = request(migration)["permission"]
    path = migration / "code/unit.cc"
    before = path.read_text()
    path.write_text(before + "// regression\n")
    calls = []

    def command(name):
        return {"argv": [name], "timeout": 10}

    config = {"schema_version": 1, "target": task.target, "acceptance_scope": scope,
              **{name: command(name) for name in ("context", "check", "obligations", "migration_check")}}

    def execute(command, root, name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "context":
            return 0, {}
        if stage == "obligations":
            return 0, task_obligations(root, name)
        if stage == "check":
            report = native.measure_restoration(root, name, root / "original.exe")
            return {"pass": 0, "fail": 1, "unknown": 3}[report["result"]], report
        if stage == "worker":
            # Preserve the proposed model and its outstanding byte regression.
            path.write_text(path.read_text() + "// reviewed spelling candidate\n")
            return 0, {"discoveries": []}
        if stage == "review":
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["source:synthetic bounded member requirement"],
                       "feedback": "Synthetic checkpoint review, not complete structure proof"}
        assert stage == "migration_check"
        return 0, native.measure_migration(root, name, root / "original.exe", payload)

    def run():
        return drive(migration, task.task_id, config, {"author": "worker", "command": command("worker")},
                     iterations=1, reviewer_configuration={"author": "independent", "command": command("review")},
                     migration_configuration=permission, execute=execute)

    first = run()
    assert first["stop_reason"] == "migration_admitted" and first["migration"]["exit_code"] == 2
    assert len(first["migration"]["tracked"]) == 1 and not first["ready_for_collection"]
    assert "regression" in path.read_text() and "BYTE_EXACT" in path.read_text()
    git(migration, "add", ".")
    git(migration, "commit", "-m", "Synthetic reviewed structural checkpoint")
    path.write_text(before)
    calls.clear()
    second = run()
    assert second["stop_reason"] == ("change_accepted" if scope == "change" else "full_acceptance_gate_required")
    assert second["migration"]["exit_code"] == 0 and not second["migration"]["tracked"]
    assert not second["ready_for_collection"] and "worker" not in calls
    assert "migration_check" in calls
    assert TaskStore(migration).read("owner").base_revision == task.base_revision


@pytest.mark.parametrize("name", ["check_source", "check_mappings", "check_claims", "check_preservation"])
@pytest.mark.parametrize("result", ["fail", "unknown"])
def test_independent_prerequisites_cannot_be_waived(migration, monkeypatch, name, result):
    monkeypatch.setattr(native, name, lambda *_: {"result": result})
    payload = request(migration)
    report = measure(migration, payload)
    assert report["outside_scope"] == result
    with pytest.raises(ValueError, match="outside-scope preservation"):
        admission(migration, payload, report)


def test_wrong_layout_cannot_be_admitted_by_review(migration):
    path = migration / "code/unit.cc"
    path.write_text(path.read_text() + "// wrong-layout\n")
    assert measure(migration)["structure"] == "fail"


def test_changed_build_policy_cannot_borrow_baseline_preservation(migration):
    path = migration / "recovery/build.json"
    value = json.loads(path.read_text())
    value["commands"][0]["argv"][2] += "; pass"
    path.write_text(json.dumps(value))
    assert measure(migration)["outside_scope"] == "unknown"


def test_changed_measurement_compiler_cannot_borrow_baseline_preservation(migration):
    # This helper is deliberately not an application-build input in the fixture.
    path = migration / "recovery/compile.py"
    path.write_text(path.read_text() + "\n# changed compiler interaction\n")
    assert measure(migration)["outside_scope"] == "unknown"


def test_failed_build_prevents_compiler_checks(migration, monkeypatch):
    monkeypatch.setattr(native, "check_build", lambda *_: {"result": "fail"})
    monkeypatch.setattr(native, "measure_restoration", lambda *a, **k: pytest.fail("compiled after failed build"))
    report = measure(migration)
    assert report["build"] == "fail" and report["measurements"] == []


def test_explicit_complete_structure_obligation_is_not_replaced_by_bounded_layout(migration):
    payload = request(migration)
    task = TaskStore(migration).read("owner")
    full = Obligation(task.subjects[0].entity_id, Dimension.STRUCTURE, "independent whole-model requirement")
    payload["protected"].append({**asdict(full), "obligation_id": full.obligation_id})
    report = measure(migration, payload)
    row, = [row for row in report["measurements"] if row["obligation_id"] == full.obligation_id]
    assert row["result"] == "unknown" and report["structure"] == "pass"
    with pytest.raises(ValueError, match="not admissible"):
        admission(migration, payload, report)


def test_matching_permission_cannot_waive_declared_structure(migration):
    payload = request(migration)
    payload["permission"]["allowed"][0]["dimension"] = "declared_structure"
    with pytest.raises(ValueError, match="only scoped matching regressions"):
        measure(migration, payload)


def test_stale_review_is_refused_before_build(migration, monkeypatch):
    payload = request(migration)
    payload["review"]["snapshot"] = "d" * 64
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built for stale review"))
    with pytest.raises(ValueError, match="current independent approval"):
        measure(migration, payload)


@pytest.mark.parametrize("damage", ["source", "review-policy"])
def test_stale_reviewed_inputs_cannot_admit_a_migration(migration, monkeypatch, damage):
    payload = request(migration)
    if damage == "source":
        path = migration / "code/unit.cc"
        path.write_text(path.read_text() + "// edited after independent review\n")
    else:
        payload["review_policy"]["changed"] = "policy"
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built for stale reviewed inputs"))
    with pytest.raises(ValueError, match="reviewed source snapshot"):
        measure(migration, payload)


def test_native_cli_consumes_driver_request_without_project_dispatch(migration, monkeypatch, capsys):
    monkeypatch.setattr(sys, "stdin", io.StringIO(json.dumps(request(migration))))
    assert main(["tasks", "check-migration", "owner", "--root", str(migration), "--image", "original.exe"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["build"] == report["structure"] == report["outside_scope"] == "pass"
    assert "ready_for_collection" not in report  # Measurement envelope, not admission.


@pytest.mark.parametrize("damage", ["image", "source", "history"])
def test_changes_between_prerequisites_refuse_checkpoint(migration, monkeypatch, damage):
    def mutate(*_):
        if damage == "history":
            git(migration, "commit", "--allow-empty", "-m", "Concurrent history change")
        else:
            path = migration / ("original.exe" if damage == "image" else "code/unit.cc")
            path.write_bytes(path.read_bytes() + b"changed")
        return {"result": "pass"}
    monkeypatch.setattr(native, "check_mappings", mutate)
    with pytest.raises(ValueError, match="changed|pinned original"):
        measure(migration)
