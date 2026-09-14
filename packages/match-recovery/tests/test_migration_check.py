"""Native checkpoint orchestration with real history/build and synthetic COFF.

The compiler is a controlled test double; this is not a live agent/game pilot.
Restoration, layout, outside-code comparison and admission policy are real.
Positive admission protocol controls explicitly double the missing qualified
linked preservation; ordinary physical compiler evidence cannot supply it.
"""

from contextlib import contextmanager
from copy import deepcopy
from dataclasses import asdict, replace
import io
import json
from pathlib import Path
import sys
from tempfile import TemporaryDirectory
from types import SimpleNamespace

import pytest

from match_recovery import migration_check as native
from match_recovery.cli import main
from match_recovery.driver import drive, working_state
from match_recovery.migration_protocol import admit, extend_inventory
from match_recovery.model import Dimension, Obligation, Origin
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
    monkeypatch.setattr(native, "check_preservation", lambda root, name, **kwargs:
                        check_preservation(root, name, compiler=Compiler(root), **kwargs))
    return root


def request(root, *, generated_changes=None):
    task = TaskStore(root).read("owner")
    inventory = task_obligations(root, "owner")
    permission = {"migration_id": "layout", "author": task.author, "evidence": ["source:synthetic permission"],
                  "allowed": [{"entity_id": task.subjects[0].entity_id, "dimension": "bytes"}]}
    if generated_changes is not None:
        permission["generated_changes"] = generated_changes
    policy = {"migration": permission}
    snapshot = ReviewStore(root, "owner").snapshot(working_state(root), policy)
    review = {"kind": "review", "author": "worker", "reviewer": "independent", "snapshot": snapshot,
              "decision": "approve", "evidence": ["source:synthetic reviewed change"], "feedback": "Checkpoint, not completion"}
    return {"task": task.to_dict(), "snapshot": review["snapshot"], "review": review,
            "review_policy": policy,
            "protected": inventory["obligations"], "interpretations": inventory["interpretations"],
            "permission": permission}


def measure(root, payload=None):
    return native.measure_migration(root, "owner", root / "original.exe", payload or request(root))


def admission(root, payload, report):
    scope, _ = TaskStore(root).scope("owner")
    protected = {}
    extend_inventory(protected, {"obligations": payload["protected"]}, scope)
    return admit(payload["permission"], tuple(protected.values()), scope, payload["review"], payload["snapshot"], 0, report)


@pytest.fixture
def qualified_preservation_protocol(migration, monkeypatch):
    """Explicit complete-prerequisite double for admission protocol tests only.

    This is not evidence that the physical compiler comparison proves linking.
    The ordinary migration fixture and native producer remain unqualified.
    """
    preserve = native.check_preservation

    def qualified(*args, **kwargs):
        return {**preserve(*args, **kwargs), "outside_scope_verification": "pass",
                "test_fixture": "synthetic qualified linked prerequisite; not a physical proof"}

    monkeypatch.setattr(native, "check_preservation", qualified)


def test_admission_protocol_tracks_regression_then_restores_same_obligations(migration, qualified_preservation_protocol):
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


def test_physical_compiled_residual_pass_cannot_supply_linked_acceptance(migration, monkeypatch):
    path = migration / "code/unit.cc"
    path.write_text(path.read_text() + "// Ordinary scoped body spelling with unchanged build policy.\n")
    preserve = native.check_preservation
    observed = []

    def physical(*args, **kwargs):
        # Real bounded COFF comparison and damage controls, not a fabricated
        # passing preservation report. No linked-selection evidence is supplied.
        report = preserve(*args, **kwargs)
        observed.append(report)
        return report

    monkeypatch.setattr(native, "check_preservation", physical)
    payload = request(migration)
    before = working_state(migration)
    report = measure(migration, payload)
    assert len(observed) == 1 and observed[0]["result"] == "pass"
    assert observed[0]["outside_scope_verification"] == "unknown" and not observed[0]["scope_complete"]
    assert report["build"] == report["structure"] == report["references"] == "pass"
    assert report["outside_scope"] == "unknown"
    diagnostic, = [row for row in report["diagnostics"] if row["check"] == "linked-preservation"]
    assert diagnostic["result"] == "unknown"
    assert '"compiled_residual": "pass"' in diagnostic["evidence"][0]
    assert "baseline/current selection" in diagnostic["evidence"][0]
    with pytest.raises(ValueError, match="outside-scope preservation"):
        admission(migration, payload, report)
    assert working_state(migration) == before


@pytest.mark.parametrize("qualified", [None, "unknown", "not_measured", False])
def test_absent_or_invalid_linked_qualification_never_inherits_compiled_pass(migration, monkeypatch, qualified):
    report = {"result": "pass"}
    if qualified is not None:
        report["outside_scope_verification"] = qualified
    monkeypatch.setattr(native, "check_preservation", lambda *args, **kwargs: report)
    payload = request(migration)
    measured = measure(migration, payload)
    assert measured["outside_scope"] == "unknown"
    with pytest.raises(ValueError, match="outside-scope preservation"):
        admission(migration, payload, measured)


def test_native_verification_only_driver_cannot_publish_with_compiled_evidence_alone(migration):
    task = TaskStore(migration).read("owner")
    before = working_state(migration)
    calls = []

    def command(name):
        return {"argv": [name], "timeout": 10}

    config = {"schema_version": 1, "target": task.target, "acceptance_scope": "change",
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
        if stage == "review":
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["Independent approval cannot establish linked selection"], "feedback": "Reviewed scoped source"}
        assert stage == "migration_check", "Verification-only run must never invoke the worker"
        return 0, native.measure_migration(root, name, root / "original.exe", payload)

    report = drive(migration, task.task_id, config, {"author": "worker", "command": command("worker")},
                   iterations=1, verify_only=True,
                   reviewer_configuration={"author": "independent", "command": command("review")},
                   checkpoint_configuration={"author": task.author, "message": "Must not publish", "allow_migration": False},
                   execute=execute)
    assert report["stop_reason"] == "change_unknown" and not report["ready_for_collection"]
    assert not report.get("ready_for_checkpoint") and "worker" not in calls
    assert calls.count("migration_check") == 1
    assert working_state(migration)["head"] == before["head"]


@pytest.mark.parametrize("scope", ["change", "full_project"])
def test_driver_checkpoint_protocol_resumes_without_resetting_baseline(migration, qualified_preservation_protocol, scope):
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
def test_independent_prerequisites_cannot_be_waived(migration, qualified_preservation_protocol, monkeypatch, name, result):
    monkeypatch.setattr(native, name, lambda *_: {"result": result, "outside_scope_verification": "pass"})
    payload = request(migration)
    report = measure(migration, payload)
    assert report["outside_scope"] == result
    with pytest.raises(ValueError, match="outside-scope preservation"):
        admission(migration, payload, report)


def test_wrong_layout_cannot_be_admitted_by_review(migration):
    path = migration / "code/unit.cc"
    path.write_text(path.read_text() + "// wrong-layout\n")
    assert measure(migration)["structure"] == "fail"


@pytest.mark.parametrize("outcome", ["fail", "unknown"])
def test_refused_preservation_retains_actionable_native_diagnostics(migration, monkeypatch, outcome):
    monkeypatch.setattr(native, "check_preservation", lambda *_: {
        "result": outcome, "units": [{"unit": "code/unit.cc", "reason": "unscoped function changed",
                                       "symbol_changes": [{"symbol": "outside_helper", "change": "added"}]}]})
    payload = request(migration)
    report = measure(migration, payload)
    row = next(row for row in report["diagnostics"] if row["check"] == "preservation")
    assert row["result"] == outcome
    assert "outside_helper" in row["evidence"][0] and "code/unit.cc" in row["evidence"][0]
    with pytest.raises(ValueError, match="outside-scope preservation"):
        admission(migration, payload, report)


def test_failed_build_diagnostics_survive_early_return(migration, monkeypatch):
    monkeypatch.setattr(native, "check_build", lambda *_: {
        "result": "fail", "commands": [{"stderr": "undefined reference to Owner::add"}]})
    report = measure(migration)
    assert report["build"] == "fail" and report["measurements"] == []
    assert "undefined reference to Owner::add" in next(
        row for row in report["diagnostics"] if row["check"] == "build")["evidence"][0]


@pytest.mark.parametrize("damage", ["generator", "preparation", "added_input", "removed_input", "source"])
def test_preparation_policy_is_protected_independently_of_application_build_inputs(migration, damage):
    from match_recovery.build_check import Build
    from match_recovery.measurement import Measurement
    root = migration
    path = root / "recovery/project.json"
    project = json.loads(path.read_text())
    project["preparation"] = "recovery/preparation.json"
    path.write_text(json.dumps(project))
    (root / "generator-data").mkdir()
    (root / "generator-data/model.txt").write_text("stable recipe inputs\n")
    (root / "recovery/generator.py").write_text("# configured generator\n")
    (root / "recovery/preparation.json").write_text(json.dumps({
        "schema_version": 1, "artifacts": "build/preparation", "input_roots": ["code", "generator-data"],
        "inputs": ["recovery/generator.py"], "environment": [],
        "commands": [{"argv": [sys.executable, "recovery/generator.py", "{build}"], "timeout": 10}],
        "outputs": [{"path": "compile_commands.json", "format": "compilation-database"}]}))
    git(root, "add", ".")
    git(root, "commit", "-m", "Declare generator before authorizing a new task")
    task = replace(TaskStore(root).read("owner"), task_id="prepared", base_revision=git(root, "rev-parse", "HEAD"))
    assert native.unchanged_build_policy(root, task, Measurement.read(root), Build.read(root))
    if damage == "generator":
        (root / "recovery/generator.py").write_text("# different generator\n")
    elif damage == "preparation":
        path = root / "recovery/preparation.json"
        path.write_text(path.read_text() + "\n")
    elif damage == "added_input":
        (root / "generator-data/new.txt").write_text("new input\n")
    elif damage == "removed_input":
        (root / "generator-data/model.txt").unlink()
    else:
        (root / "code/unit.cc").write_text((root / "code/unit.cc").read_text() + "// authorized source edit\n")
    assert native.unchanged_build_policy(root, task, Measurement.read(root), Build.read(root)) == (damage == "source")


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


def live_graph(migration, monkeypatch, *, outcome="unknown", reusable_build=True,
               enter_refusal=False, close_refusal=False, enter_mutate=None):
    """Synthetic lifetime only; native graph/build producers have separate tests."""
    calls, sessions = [], []
    monkeypatch.setattr(native, "build_policy_changes", lambda *a, **k: {
        "unchanged": False, "graph_only": True, "changed_paths": ["BuildGraph.txt"]})

    @contextmanager
    def graph(root, name, *, integration):
        assert root == migration and name == "owner" and integration is None
        calls.append("enter")
        if enter_refusal:
            raise ValueError("baseline context unavailable")
        with TemporaryDirectory(prefix="migration-live-graph-") as temporary:
            baseline, current = Path(temporary) / "baseline", Path(temporary) / "current"
            baseline.mkdir()
            current.mkdir()
            active = True

            def fresh():
                assert active and baseline.is_dir() and current.is_dir()

            report = {"result": outcome, "commands": {"result": "pass"}, "linked_effects": "unknown"}
            if reusable_build:
                report["current_build"] = {"result": "pass", "test_fixture": "live context producer fixture"}
            session = SimpleNamespace(report=report, check_current=fresh,
                                      baseline=SimpleNamespace(root=baseline, check_current=fresh),
                                      current=SimpleNamespace(root=current, check_current=fresh))
            sessions.append(session)
            try:
                if enter_mutate is not None:
                    enter_mutate()
                yield session
            finally:
                fresh()
                calls.append("close")
                active = False
                if close_refusal:
                    raise ValueError("graph context close refused")

    monkeypatch.setattr(native, "application_graph", graph)
    return calls, sessions


def test_native_graph_prerequisite_reuses_current_build_without_waiving_linked_effects(migration, monkeypatch):
    calls, sessions = live_graph(migration, monkeypatch)
    task = TaskStore(migration).read("owner")
    monkeypatch.setattr(native, "check_build", lambda *a: pytest.fail("duplicated current application build"))
    restoration = native.measure_restoration

    def restore(*args, **kwargs):
        assert calls == ["enter"]
        sessions[0].baseline.check_current()
        sessions[0].current.check_current()
        calls.append("restoration")
        return restoration(*args, **kwargs)

    def preserve(root, name, *, application):
        assert root == migration and name == "owner" and application is sessions[0]
        assert calls == ["enter", "restoration"]
        application.baseline.check_current()
        application.current.check_current()
        calls.append("preservation")
        return {"result": "pass", "outside_scope_verification": "pass",
                "test_fixture": "consumer orchestration, not producer evidence"}

    monkeypatch.setattr(native, "measure_restoration", restore)
    monkeypatch.setattr(native, "check_preservation", preserve)
    payload = request(migration)
    report = measure(migration, payload)
    assert calls == ["enter", "restoration", "preservation", "close"] and report["build"] == "pass"
    assert not sessions[0].baseline.root.exists() and not sessions[0].current.root.exists()
    assert report["outside_scope"] == "unknown" and report["measurements"]
    assert any(row["check"] == "build-graph" and '"linked_effects": "unknown"' in row["evidence"][0]
               for row in report["diagnostics"])
    with pytest.raises(ValueError, match="preservation"):
        admission(migration, payload, report)
    assert TaskStore(migration).read("owner") == task


def test_refused_graph_context_still_checks_current_application_without_granting_acceptance(migration, monkeypatch):
    calls, sessions = live_graph(migration, monkeypatch, enter_refusal=True)
    monkeypatch.setattr(native, "check_build", lambda *a: calls.append("current-build") or {"result": "pass"})
    report = measure(migration)
    assert calls == ["enter", "current-build"] and not sessions and report["build"] == "pass"
    assert report["outside_scope"] == "unknown"
    assert "baseline context unavailable" in next(row for row in report["diagnostics"]
                                                  if row["check"] == "build-graph")["evidence"][0]


@pytest.mark.parametrize("changed_policy", [False, True])
def test_unchanged_or_non_graph_policy_never_invokes_graph_exception_route(migration, monkeypatch, changed_policy):
    if changed_policy:
        path = migration / "recovery/compile.py"
        path.write_text(path.read_text() + "\n# changed compiler policy\n")
    monkeypatch.setattr(native, "application_graph", lambda *a, **k: pytest.fail("ineligible graph route"))
    report = measure(migration)
    assert report["outside_scope"] == "unknown"


def test_ordinary_native_compile_link_policy_enters_paired_builds(migration, monkeypatch):
    # Route/lifetime control only. No synthetic graph result qualifies native
    # linked preservation; physical application producers have separate tests.
    calls, sessions = live_graph(migration, monkeypatch, enter_refusal=True)
    monkeypatch.setattr(native, "build_policy_changes", lambda *a, **k: {
        "unchanged": True, "graph_only": False, "changed_paths": []})
    build = native.Build.read(migration)
    monkeypatch.setattr(native.Build, "read", lambda *a, **k: replace(build, compile_link={}))
    report = measure(migration)
    assert calls == ["enter"] and not sessions
    assert report["outside_scope"] == "unknown"
    assert any(row["check"] == "build-graph" for row in report["diagnostics"])


def test_graph_prerequisite_cannot_return_after_mutating_reviewed_source(migration, monkeypatch):
    def mutate():
        path = migration / "code/unit.cc"
        path.write_text(path.read_text() + "// preserve concurrent source change\n")

    calls, sessions = live_graph(migration, monkeypatch, enter_mutate=mutate)
    with pytest.raises(ValueError, match="changed"):
        measure(migration)
    assert calls == ["enter", "close"] and not sessions[0].baseline.root.exists()
    assert "preserve concurrent source change" in (migration / "code/unit.cc").read_text()


def test_graph_context_close_refusal_prevents_returning_an_earlier_report(migration, monkeypatch):
    calls, sessions = live_graph(migration, monkeypatch, close_refusal=True)
    monkeypatch.setattr(native, "check_preservation", lambda *a, **k: {"result": "pass"})
    monkeypatch.setattr(native, "check_build", lambda *a: pytest.fail("unnecessary current build"))
    with pytest.raises(ValueError, match="graph context close refused"):
        measure(migration)
    assert calls == ["enter", "close"]
    assert not sessions[0].baseline.root.exists() and not sessions[0].current.root.exists()


@pytest.mark.parametrize("stage", ["restoration", "preservation"])
def test_graph_lifetime_refuses_concurrent_changes_during_scoped_checks(migration, monkeypatch, stage):
    calls, sessions = live_graph(migration, monkeypatch)
    restoration = native.measure_restoration
    source = migration / "code/unit.cc"

    def mutate():
        source.write_bytes(source.read_bytes() + b"// preserve changed recovery input\n")

    def restore(*args, **kwargs):
        sessions[0].check_current()
        calls.append("restoration")
        report = restoration(*args, **kwargs)
        if stage == "restoration":
            mutate()
        return report

    def preserve(*args, application, **kwargs):
        assert application is sessions[0]
        application.check_current()
        calls.append("preservation")
        assert stage == "preservation", "preservation ran after refused restoration epoch"
        mutate()
        return {"result": "pass"}

    monkeypatch.setattr(native, "measure_restoration", restore)
    monkeypatch.setattr(native, "check_preservation", preserve)
    with pytest.raises(ValueError, match="changed"):
        measure(migration)
    assert calls == ["enter", "restoration", *(["preservation"] if stage == "preservation" else []), "close"]
    assert source.read_bytes().endswith(b"// preserve changed recovery input\n")
    assert not sessions[0].baseline.root.exists() and not sessions[0].current.root.exists()


@pytest.mark.parametrize("built", ["pass", "fail"])
def test_failed_graph_without_reusable_build_still_checks_current_application(migration, monkeypatch, built):
    calls, sessions = live_graph(migration, monkeypatch, outcome="fail", reusable_build=False)
    monkeypatch.setattr(native, "check_build", lambda *a: calls.append("current-build") or {"result": built})

    def preserve(*args, application, **kwargs):
        assert built == "pass" and application is sessions[0]
        application.check_current()
        calls.append("preservation")
        return {"result": "pass"}

    monkeypatch.setattr(native, "check_preservation", preserve)
    report = measure(migration)
    assert report["build"] == built
    assert report["outside_scope"] == ("fail" if built == "pass" else "unknown")
    assert calls == ["enter", "current-build", *(["preservation"] if built == "pass" else []), "close"]
    assert not sessions[0].baseline.root.exists() and not sessions[0].current.root.exists()


def test_definite_live_graph_failure_is_not_downgraded_by_unknown_compiled_preservation(migration, monkeypatch):
    live_graph(migration, monkeypatch, outcome="fail")
    monkeypatch.setattr(native, "check_preservation", lambda *a, **k: {"result": "unknown"})
    payload = request(migration)
    report = measure(migration, payload)
    assert report["outside_scope"] == "fail"
    with pytest.raises(ValueError, match="outside-scope preservation"):
        admission(migration, payload, report)


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


@pytest.mark.parametrize("damage", ["missing", "substituted", "evidence"])
def test_permission_must_be_the_exact_reviewed_policy(migration, monkeypatch, damage):
    payload = request(migration)
    if damage == "missing":
        del payload["review_policy"]["migration"]
    else:
        payload["permission"] = deepcopy(payload["permission"])
        if damage == "substituted":
            payload["permission"]["allowed"] = []  # Valid, but never reviewed.
        else:
            payload["permission"]["evidence"] = ["different coordinator rationale"]
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built for unreviewed permission"))
    monkeypatch.setattr(native, "check_source", lambda *_: pytest.fail("checked unreviewed permission"))
    with pytest.raises(ValueError, match="permission must exactly match"):
        measure(migration, payload)


def generated_request(root):
    task = TaskStore(root).read("owner")
    return request(root, generated_changes=[{
        "entity_id": task.subjects[0].entity_id, "unit": "code/unit.cc", "version": "current",
        "symbol": "_$E21", "offset": 0, "evidence": ["source:explicit scoped lifecycle change"]}])


@pytest.mark.parametrize("role", ["author", "reviewer"])
def test_generated_permission_requires_independent_coordinator_without_integration(migration, monkeypatch, role):
    payload = generated_request(migration)
    assert "integration" not in payload["review_policy"]
    payload["review"][role] = TaskStore(migration).read("owner").author
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built for self-approved generated change"))
    monkeypatch.setattr(native, "check_source", lambda *_: pytest.fail("checked self-approved generated change"))
    with pytest.raises(ValueError, match="generated-change coordinator must be independent"):
        measure(migration, payload)


def test_generated_permission_policy_binding_preserves_json_types(migration, monkeypatch):
    payload = generated_request(migration)
    payload["review_policy"] = deepcopy(payload["review_policy"])
    # Python's equality treats these as equal; the reviewed decision does not.
    payload["review_policy"]["migration"]["generated_changes"][0]["offset"] = False
    assert payload["review_policy"]["migration"] == payload["permission"]
    monkeypatch.setattr(native, "check_source", lambda *_: pytest.fail("checked substituted decision type"))
    with pytest.raises(ValueError, match="permission must exactly match"):
        measure(migration, payload)


def test_changing_both_permission_envelopes_still_requires_fresh_review(migration, monkeypatch):
    payload = generated_request(migration)
    payload["permission"]["generated_changes"][0]["offset"] = 4
    assert payload["review_policy"]["migration"] == payload["permission"]
    monkeypatch.setattr(native, "check_source", lambda *_: pytest.fail("checked stale generated permission"))
    with pytest.raises(ValueError, match="reviewed source snapshot"):
        measure(migration, payload)


def test_independent_generated_permission_reaches_fresh_native_checks_without_granting_ownership(migration, monkeypatch):
    payload = generated_request(migration)

    class ReachedNativeChecks(Exception):
        pass

    def reached(*_):
        raise ReachedNativeChecks

    # Stop before any compiler checks. A syntactically valid, reviewed decision
    # authorizes no residual subtraction by itself; native evidence still runs.
    monkeypatch.setattr(native, "check_source", reached)
    with pytest.raises(ReachedNativeChecks):
        measure(migration, payload)


def test_stale_review_is_refused_before_build(migration, monkeypatch):
    payload = request(migration)
    payload["review"]["snapshot"] = "d" * 64
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built for stale review"))
    with pytest.raises(ValueError, match="current independent approval"):
        measure(migration, payload)


@pytest.mark.parametrize("outside", [False, True])
@pytest.mark.parametrize("cli", [False, True])
def test_pending_discovery_cannot_be_waived_by_direct_approval(migration, monkeypatch, capsys, outside, cli):
    store = TaskStore(migration)
    subject = store.read("owner").subjects[0]
    if outside:
        subject = Origin(subject.image_sha256, subject.address_space, subject.address + 0x100)
    store.discover("owner", request_id="unresolved-model", author="worker",
                   hypothesis="The changed caller may depend on a different lifetime model",
                   affects=[subject.entity_id], evidence=["source:unresolved relationship"], expected_revision=0)
    payload = request(migration)  # Fresh approval still cannot resolve a finding.
    before = working_state(migration)
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built with unresolved discovery"))
    monkeypatch.setattr(native, "measure_restoration", lambda *a, **k: pytest.fail("measured unresolved discovery"))
    if cli:
        monkeypatch.setattr(sys, "stdin", io.StringIO(json.dumps(payload)))
        assert main(["tasks", "check-migration", "owner", "--root", str(migration), "--image", "original.exe"]) == 3
        assert "unresolved-model" in capsys.readouterr().err
    else:
        with pytest.raises(ValueError, match="pending discoveries: unresolved-model"):
            measure(migration, payload)
    assert working_state(migration) == before
    assert len(store.status("owner", store.scope("owner")[0])["pending_discoveries"]) == 1


@pytest.mark.parametrize("disposition", ["incorporated", "rejected"])
def test_resolved_discovery_requires_fresh_review_then_native_checks(migration, disposition):
    store = TaskStore(migration)
    task = store.read("owner")
    store.discover("owner", request_id="model", author="worker", hypothesis="Check the member model",
                   affects=[task.subjects[0].entity_id], evidence=["source:bounded model"], expected_revision=0)
    stale = request(migration)
    store.resolve("owner", discovery_id="model", request_id="resolve-model", author=task.author,
                  disposition=disposition, source_evidence=["source:coordinator disposition"], expected_revision=1)
    with pytest.raises(ValueError, match="reviewed source snapshot"):
        measure(migration, stale)
    payload = request(migration)
    report = measure(migration, payload)
    assert report["build"] == report["structure"] == report["references"] == "pass"
    assert report["outside_scope"] == "unknown"
    with pytest.raises(ValueError, match="outside-scope preservation"):
        admission(migration, payload, report)
    assert store.read("owner").base_revision == task.base_revision


def test_discovery_during_native_verification_invalidates_review(migration, monkeypatch):
    store = TaskStore(migration)
    task = store.read("owner")

    def discover(*_):
        store.discover("owner", request_id="late-model", author="worker", hypothesis="New dependency",
                       affects=[task.subjects[0].entity_id], evidence=["source:late finding"], expected_revision=0)
        return {"result": "pass"}

    monkeypatch.setattr(native, "check_mappings", discover)
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built after task mutation"))
    with pytest.raises(ValueError, match="reviewed source snapshot|inputs changed"):
        measure(migration)


def test_other_tasks_pending_work_does_not_establish_a_change_dependency(migration, qualified_preservation_protocol):
    store = TaskStore(migration)
    task = store.read("owner")
    subject = task.subjects[0]
    other = replace(task, task_id="other", subjects=(Origin(
        subject.image_sha256, subject.address_space, subject.address + 0x100),))
    store.create(other)
    store.discover("other", request_id="other-work", author="worker", hypothesis="Unfinished other owner",
                   affects=[other.subjects[0].entity_id], evidence=["source:separate work"], expected_revision=0)
    payload = request(migration)
    assert admission(migration, payload, measure(migration, payload)).exit_code == 0
    assert len(store.status("other", store.scope("other")[0])["pending_discoveries"]) == 1


def test_ignored_resolution_cannot_bypass_review_freshness(migration, monkeypatch):
    path = migration / ".gitignore"
    path.write_text(path.read_text() + "recovery/history/\n")
    store = TaskStore(migration)
    task = store.read("owner")
    store.discover("owner", request_id="model", author="worker", hypothesis="Changed model",
                   affects=[task.subjects[0].entity_id], evidence=["source:pending model"], expected_revision=0)
    payload = request(migration)
    before = working_state(migration)
    store.resolve("owner", discovery_id="model", request_id="resolve-model", author=task.author,
                  disposition="rejected", source_evidence=["source:unreviewed disposition"], expected_revision=1)
    assert working_state(migration) == before  # Review token alone misses ignored decisions.
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built with unreviewed decisions"))
    with pytest.raises(ValueError, match="decisions must participate in the review snapshot"):
        measure(migration, payload)


@pytest.mark.parametrize("kind", ["task", "journal", "event"])
def test_symlinked_decisions_are_not_reviewed_bytes(migration, monkeypatch, kind):
    store = TaskStore(migration)
    task = store.read("owner")
    store.discover("owner", request_id="model", author="worker", hypothesis="Changed model",
                   affects=[task.subjects[0].entity_id], evidence=["source:model"], expected_revision=0)
    store.resolve("owner", discovery_id="model", request_id="resolve-model", author=task.author,
                  disposition="rejected", source_evidence=["source:disposition"], expected_revision=1)
    journal = store.journal.stream_directory("task:owner")
    path = (migration / "recovery/tasks/owner.json" if kind == "task" else
            journal if kind == "journal" else journal / "00000002.json")
    target = migration / "build/decision-target"
    path.rename(target)
    path.symlink_to(target, target_is_directory=kind == "journal")
    payload = request(migration)
    monkeypatch.setattr(native, "check_build", lambda *_: pytest.fail("built with unreviewed target bytes"))
    with pytest.raises(ValueError, match="canonical regular task decision files"):
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
    assert report["build"] == report["structure"] == "pass"
    assert report["outside_scope"] == "unknown"
    assert any(row["check"] == "linked-preservation" for row in report["diagnostics"])
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
