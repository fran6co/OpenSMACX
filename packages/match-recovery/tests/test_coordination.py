"""Delegation controls over native source snapshots and durable task decisions."""

import json
import subprocess
import sys

import pytest

from match_recovery.coordination import coordinate, policy
from match_recovery.driver import drive
from match_recovery.model import Origin
from match_recovery.tasks import Task, TaskStore
from test_source_configuration import configured
from test_task_source import git, project


@pytest.fixture
def setup(project):
    root = project
    caller = Origin("a" * 64, "custom:offset", 32)
    (root / "code/caller.cc").write_text('// ORIGINAL: 0x00000020 _caller 0x00000020-0x00000028\nvoid caller() {}\n')
    (root / ".gitignore").write_text("build/\n")
    git(root, "add", ".")
    git(root, "commit", "-m", "Caller evidence")
    task = Task("owner", "custom", "coordinator", "Recover ownership", git(root, "rev-parse", "HEAD"),
                (Origin("a" * 64, "custom:offset", 16),), ("code/owner.cc",), ("seed",))
    store = TaskStore(root)
    store.create(task)
    delegation = {"schema_version": 1, "author": "coordinator",
                  "command": {"argv": ["coordinate"], "timeout": 10},
                  "subjects": [caller.entity_id], "editable_files": ["code/caller.cc", "code/owner.hpp"],
                  "evidence": ["Authorize investigation of the known caller and shared header"], "max_decisions": 3}
    (root / "recovery/coordinator.json").write_text(json.dumps(delegation))
    git(root, "add", ".")
    git(root, "commit", "-m", "Delegated task")
    return root, store, task, caller, delegation


def discover(setup, subject=None):
    root, store, task, caller, _ = setup
    store.discover(task.task_id, request_id="layout", author="worker", hypothesis="The caller shares the owner header",
                   affects=[subject or caller.entity_id], evidence=["code/caller.cc"], expected_revision=0)


def answer(payload, action="expand", **overrides):
    return {"snapshot": payload["snapshot"], "action": action, "discovery_id": "layout",
            "subjects": payload["delegation"]["subjects"] if action == "expand" else [],
            "editable_files": [], "evidence": ["Shared source model inspected; this is not acceptance"], **overrides}


def test_native_expansion_and_resolution_preserve_source_baseline_and_obligations(setup):
    root, store, task, caller, delegation = setup
    discover(setup)
    before = (root / "code/owner.cc").read_bytes()
    head = git(root, "rev-parse", "HEAD")
    seen = []
    def execute(command, root, name, request):
        seen.append(request)
        assert request["mapped_records"][0]["origin"]["address"] == 32
        return 0, answer(request, "expand" if len(seen) == 1 else "incorporate")
    assert coordinate(root, task.task_id, delegation, execute=execute)["action"] == "expand"
    assert store.scope(task.task_id) == (frozenset([task.subjects[0].entity_id, caller.entity_id]), frozenset(["code/owner.cc"]))
    assert coordinate(root, task.task_id, delegation, execute=execute)["action"] == "incorporate"
    assert seen[0]["snapshot"] != seen[1]["snapshot"]
    assert len(store.decisions(task.task_id)) == 3
    assert store.read(task.task_id).base_revision == task.base_revision
    assert (root / "code/owner.cc").read_bytes() == before
    assert git(root, "rev-parse", "HEAD") == head


@pytest.mark.parametrize("damage", ["scope", "stale", "discovery", "no_progress", "policy_file", "unknown", "source_write", "git_write", "exception_write", "reject_undelegated"])
def test_coordinator_refuses_unsafe_decisions_without_rolling_back_work(setup, damage):
    root, store, task, caller, delegation = setup
    unknown = "entity-" + "b" * 64
    discover(setup, unknown if damage == "unknown" else None)
    if damage == "policy_file":
        delegation["editable_files"] = ["recovery/project.json"]
    if damage == "reject_undelegated":
        delegation["subjects"] = []
    def execute(command, root, name, payload):
        response = answer(payload)
        if damage == "reject_undelegated": response = answer(payload, "reject")
        if damage == "scope": response["editable_files"] = ["code/owner.cc"]
        if damage == "stale": response["snapshot"] = "f" * 64
        if damage == "discovery": response["discovery_id"] = "not-pending"
        if damage == "no_progress": response["subjects"] = []
        if damage == "policy_file": response["editable_files"] = ["recovery/project.json"]
        if damage in ("source_write", "exception_write"):
            (root / "code/owner.cc").write_text("preserved unauthorized edit")
        if damage == "exception_write": raise subprocess.TimeoutExpired("coordinator", 1)
        if damage == "git_write":
            (root / "extra").write_text("unauthorized staging")
            git(root, "add", "extra")
        return 0, response
    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, execute=execute)
    assert len(store.decisions(task.task_id)) == 1
    assert store.read(task.task_id).base_revision == task.base_revision
    if damage in ("source_write", "exception_write"):
        assert (root / "code/owner.cc").read_text() == "preserved unauthorized edit"


@pytest.mark.parametrize("action", ["defer", "unavailable"])
def test_deferral_and_unavailability_preserve_pending_discovery(setup, action):
    root, store, task, _, delegation = setup
    discover(setup)
    result = coordinate(root, task.task_id, delegation, execute=lambda *args:
                        (3, {"error": "quota"}) if action == "unavailable" else (0, answer(args[-1], "defer")))
    assert result["action"] == action and not result["published"] and not result["ready_for_collection"]
    assert len(store.decisions(task.task_id)) == 1


def test_investigation_preserves_pending_history_and_needs_existing_scope(setup):
    root, store, task, caller, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    before = store.decisions(task.task_id)
    result = coordinate(root, task.task_id, delegation, execute=lambda *args: (0, answer(args[-1], "investigate")))
    assert result["action"] == "investigate" and not result["published"]
    assert result["discovery_id"] == "layout" and len(result["snapshot"]) == 64
    assert store.decisions(task.task_id) == before
    assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
    store.discover(task.task_id, request_id="outside", author="worker", hypothesis="Caller needs repair",
                   affects=[caller.entity_id], evidence=["caller"], expected_revision=1)
    with pytest.raises(ValueError, match="expand current task scope"):
        coordinate(root, task.task_id, delegation, execute=lambda *args:
                   (0, answer(args[-1], "investigate", discovery_id="outside")))


@pytest.mark.parametrize("outcome", ["resolve", "pending", "no_progress", "no_progress_change", "defer", "budget", "multiple", "multiple_allowed"])
def test_driver_investigates_before_resolution_but_never_accepts_pending_findings(setup, outcome):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    if outcome in ("multiple", "multiple_allowed"):
        store.discover(task.task_id, request_id="second", author="worker", hypothesis="Second in-scope concern",
                       affects=[task.subjects[0].entity_id], evidence=["owner"], expected_revision=1)
    original_decisions = store.decisions(task.task_id)
    delegation["max_decisions"] = 1 if outcome == "budget" else 6
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10},
              "check": {"argv": ["check"], "timeout": 10},
              "build": {"argv": ["build"], "timeout": 10},
              "acceptance": {"argv": ["acceptance"], "timeout": 10}}
    if outcome == "no_progress_change":
        config.update(acceptance_scope="change", obligations={"argv": ["inventory"], "timeout": 10},
                      migration_check={"argv": ["migration"], "timeout": 10})
    roles = {role: {"author": role, "command": {"argv": [role], "timeout": 10}} for role in ("worker", "reviewer")}
    calls, snapshots = [], []
    def execute(command, root, name, payload):
        role = command.argv[0]
        calls.append(role)
        if role == "coordinate":
            snapshots.append(payload["snapshot"])
            pending = [item for item in payload["pending_discoveries"]
                       if item["discovery_id"] not in payload.get("investigating", [])]
            discovery = pending[0]["discovery_id"]
            action = ("defer" if outcome == "defer" or outcome == "multiple" and discovery == "second" else
                      "incorporate" if outcome == "resolve" and "worker" in calls else "investigate")
            return 0, answer(payload, action, discovery_id=discovery)
        if role == "context": return 0, {}
        if role == "inventory": return 0, {"obligations": []}
        if role in ("check", "build"): return 0, {"result": "pass"}  # Even passing checks cannot waive pending work.
        if role == "worker":
            assert len(payload["context"]["discovery_investigations"]) == (2 if outcome == "multiple_allowed" else 1)
            assert payload["context"]["pending_discoveries"][0]["discovery_id"] == "layout"
            assert store.decisions(task.task_id) == original_decisions
            if outcome not in ("no_progress", "no_progress_change"):
                path = root / "code/owner.cc"
                path.write_text(path.read_text() + "// investigated repair\n")
            return 0, {"discoveries": []}
        assert outcome == "resolve" and not store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
        if role == "reviewer":
            return 0, {"snapshot": payload["snapshot"], "decision": "approve", "evidence": ["source reviewed"], "feedback": "check independently"}
        assert role == "acceptance"
        return 3, {"result": "unknown"}  # Deliberately incomplete; invocation ordering is the subject here.
    # Do not invoke the separate full acceptance contract in the resolved case.
    if outcome == "resolve": del config["acceptance"]
    result = drive(root, task.task_id, config, roles["worker"], iterations=1,
                   reviewer_configuration=roles["reviewer"], coordinator_configuration=delegation, execute=execute)
    assert not result["ready_for_collection"]
    assert store.read(task.task_id).base_revision == task.base_revision
    assert calls.count("worker") == (0 if outcome in ("defer", "multiple") else 1)
    assert ("reviewer" in calls) == (outcome == "resolve")
    assert "acceptance" not in calls
    if outcome == "resolve":
        assert calls == ["context", "check", "coordinate", "context", "check", "build", "worker",
                         "context", "check", "coordinate", "context", "check", "build", "reviewer"]
        assert len(store.decisions(task.task_id)) == 2 and snapshots[0] != snapshots[1]
    else:
        assert store.decisions(task.task_id) == original_decisions
    if outcome == "pending":
        assert calls.count("build") == 2
        assert calls.count("check") == calls.count("build") + calls.count("coordinate") == 4
        assert result["stop_reason"] == "evaluate_structural_discoveries" and result["check_is_current"]
    if outcome == "budget": assert result["stop_reason"] == "coordination_budget_exhausted"
    if outcome in ("no_progress", "no_progress_change"): assert result["stop_reason"] == "no_source_progress"


def test_native_agent_bridge_keeps_full_pending_context_when_selecting_next_investigation(setup):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    store.discover(task.task_id, request_id="second", author="worker", hypothesis="Second concern",
                   affects=[task.subjects[0].entity_id], evidence=["owner"], expected_revision=1)
    script = ("import json,sys; from pathlib import Path; "
              "p=json.loads(Path(sys.argv[1]).read_text()); sys.stdin.read(); "
              "assert len(p['pending_discoveries']) == 2 and p['investigating'] == ['layout']; "
              "print(json.dumps(dict(snapshot=p['snapshot'],action='investigate',discovery_id='second',"
              "subjects=[],editable_files=[],evidence=['inspect current owner in existing scope'])))")
    role = {"command": {"argv": [sys.executable, "-c", script, "{request}"], "timeout": 10},
            "response": "stdout", "instructions": []}
    (root / "recovery/agents.json").write_text(json.dumps({"schema_version": 1, "artifacts": "build/agents",
                                                          "worker": role, "reviewer": role, "coordinator": role}))
    delegation["command"] = {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", "coordinator"], "timeout": 20}
    report = coordinate(root, task.task_id, delegation, investigating=["layout"])
    assert report["action"] == "investigate" and report["discovery_id"] == "second" and not report["published"]
    assert len(store.decisions(task.task_id)) == 2


def test_verification_only_does_not_investigate_or_accept_pending_discoveries(setup):
    root, store, task, _, _ = setup
    discover(setup, task.subjects[0].entity_id)
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10}, "check": {"argv": ["check"], "timeout": 10}}
    role = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}
    result = drive(root, task.task_id, config, role, iterations=1, verify_only=True,
                   execute=lambda *args: pytest.fail("pending discovery must block verification-only dispatch"))
    assert result["worker_rounds"] == 0 and not result["ready_for_collection"]
    assert result["stop_reason"] == "evaluate_structural_discoveries"


def test_investigation_authorization_refuses_intervening_source_change(setup, monkeypatch):
    import match_recovery.coordination as implementation
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    original = implementation.coordinate
    def changed_after_authorization(*args, **kwargs):
        response = original(*args, **kwargs)
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// concurrent edit preserved\n")
        return response
    monkeypatch.setattr(implementation, "coordinate", changed_after_authorization)
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10}, "check": {"argv": ["check"], "timeout": 10}}
    role = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}
    def execute(command, root, name, payload):
        if command.argv[0] == "context": return 0, {}
        if command.argv[0] == "check": return 0, {"result": "pass"}
        assert command.argv[0] == "coordinate"
        return 0, answer(payload, "investigate")
    with pytest.raises(ValueError, match="source changed after discovery investigation authorization"):
        drive(root, task.task_id, config, role, iterations=1, coordinator_configuration=delegation, execute=execute)
    assert "concurrent edit preserved" in (root / "code/owner.cc").read_text()
    assert len(store.decisions(task.task_id)) == 1


def test_resuming_pending_investigation_needs_fresh_coordinator_authorization(setup):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10}, "check": {"argv": ["check"], "timeout": 10}}
    role = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}
    resumed, workers, decisions = False, [], []
    def execute(command, root, name, payload):
        name = command.argv[0]
        if name == "coordinate":
            assert not payload.get("investigating")
            decisions.append(payload)
            return 0, answer(payload, "defer" if resumed else "investigate")
        if name == "context": return 0, {}
        if name == "check": return 0, {"result": "pass"}
        assert name == "worker" and not resumed
        workers.append(payload)
        return 0, {"discoveries": []}
    first = drive(root, task.task_id, config, role, iterations=1, coordinator_configuration=delegation, execute=execute)
    assert first["stop_reason"] == "no_source_progress"
    resumed = True
    second = drive(root, task.task_id, config, role, iterations=1, coordinator_configuration=delegation, execute=execute)
    assert second["stop_reason"] == "coordination_defer" and second["worker_rounds"] == 0
    assert len(workers) == 1 and len(decisions) == 2 and len(store.decisions(task.task_id)) == 1


@pytest.mark.parametrize("budget", [1, 3])
def test_driver_coordinates_without_resetting_worker_budget_or_baseline(setup, budget):
    root, store, task, caller, delegation = setup
    delegation["max_decisions"] = budget
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10}, "check": {"argv": ["check"], "timeout": 10}}
    roles = {role: {"author": role, "command": {"argv": [role], "timeout": 10}} for role in ("worker", "reviewer")}
    calls = []
    def execute(command, root, name, payload):
        role = command.argv[0]
        calls.append(role)
        if role == "context": return 0, {"scope": sorted(store.scope(task.task_id)[0])}
        if role == "check":
            passed = "// fixed" in (root / "code/owner.cc").read_text()
            return (0 if passed else 1), {"result": "pass" if passed else "fail"}
        if role == "worker":
            assert payload["round"] == 1
            path = root / "code/owner.cc"
            path.write_text(path.read_text() + "// fixed\n")
            return 0, {"discoveries": [{"hypothesis": "Shared caller", "affects": [caller.entity_id], "evidence": ["code/caller.cc"]}]}
        if role == "coordinate":
            discovery = payload["pending_discoveries"][0]["discovery_id"]
            return 0, answer(payload, "expand" if calls.count(role) == 1 else "incorporate", discovery_id=discovery)
        assert caller.entity_id in payload["context"]["scope"]
        return 0, {"snapshot": payload["snapshot"], "decision": "approve", "evidence": ["independent control"], "feedback": "native acceptance still required"}
    result = drive(root, task.task_id, config, roles["worker"], iterations=1,
                   reviewer_configuration=roles["reviewer"], coordinator_configuration=delegation, execute=execute)
    assert result["worker_rounds"] == 1 and not result["ready_for_collection"]
    assert result["stop_reason"] == ("coordination_budget_exhausted" if budget == 1 else "full_acceptance_gate_required")
    assert calls.count("worker") == 1
    assert store.read(task.task_id).base_revision == task.base_revision


@pytest.mark.parametrize("change", [{"author": "worker"}, {"subjects": ["unknown"]}, {"editable_files": ["../escape"]},
                                    {"max_decisions": True}, {"evidence": []}, {"unknown": 1}])
def test_delegation_requires_explicit_valid_authority(setup, change):
    _, _, task, _, delegation = setup
    with pytest.raises(ValueError):
        policy({**delegation, **change}, task)


def test_installed_agent_bridge_runs_a_readonly_coordinator_subprocess(setup):
    root, store, task, _, delegation = setup
    discover(setup)
    script = ("import json,sys; from pathlib import Path; "
              "p=json.loads(Path(sys.argv[1]).read_text()); sys.stdin.read(); "
              "print(json.dumps(dict(snapshot=p['snapshot'],action='expand',discovery_id='layout',"
              "subjects=p['delegation']['subjects'],editable_files=[],evidence=['source caller inspected'])))")
    role = {"command": {"argv": [sys.executable, "-c", script, "{request}"], "timeout": 10},
            "response": "stdout", "instructions": []}
    (root / "recovery/agents.json").write_text(json.dumps({"schema_version": 1, "artifacts": "build/agents",
                                                          "worker": role, "reviewer": role, "coordinator": role}))
    delegation["command"] = {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", "coordinator"], "timeout": 20}
    report = coordinate(root, task.task_id, delegation)
    assert report["action"] == "expand" and report["published"]
    assert len(store.decisions(task.task_id)) == 2
    assert not report["ready_for_collection"]


def test_local_investigation_keeps_cross_owner_finding_and_scope_unchanged(setup):
    root, store, task, caller, delegation = setup
    discover(setup)
    delegation.update(subjects=[], editable_files=[])
    before = store.decisions(task.task_id), store.scope(task.task_id)
    source = {path: (root / path).read_bytes() for path in ("code/owner.cc", "code/caller.cc")}
    head = git(root, "rev-parse", "HEAD")

    def execute(command, root, name, payload):
        assert payload["subjects"] == [task.subjects[0].entity_id]
        assert payload["editable_files"] == ["code/owner.cc"]
        assert payload["pending_discoveries"][0]["affects"] == [caller.entity_id]
        assert payload["mapped_records"][0]["entity_id"] == caller.entity_id
        return 0, answer(payload, "investigate_local")

    report = coordinate(root, task.task_id, delegation, execute=execute)
    assert report["action"] == "investigate_local"
    assert report["discovery_id"] == "layout" and len(report["snapshot"]) == 64
    assert not report["published"] and not report["ready_for_collection"]
    assert (store.decisions(task.task_id), store.scope(task.task_id)) == before
    status = store.status(task.task_id, store.scope(task.task_id)[0])
    assert status["scope_expansion_required"] == [caller.entity_id]
    assert [item["discovery_id"] for item in status["pending_discoveries"]] == ["layout"]
    assert store.read(task.task_id) == task
    assert git(root, "rev-parse", "HEAD") == head
    assert all((root / path).read_bytes() == content for path, content in source.items())


@pytest.mark.parametrize("damage", ["unknown", "subjects", "editable_files", "stale", "source_write", "exception_write"])
def test_local_investigation_does_not_relax_binding_scope_or_freshness_controls(setup, damage):
    root, store, task, caller, delegation = setup
    discover(setup, "entity-" + "b" * 64 if damage == "unknown" else None)
    delegation.update(subjects=[], editable_files=[])
    before = store.decisions(task.task_id), store.scope(task.task_id)

    def execute(command, root, name, payload):
        response = answer(payload, "investigate_local")
        if damage == "subjects": response["subjects"] = [caller.entity_id]
        if damage == "editable_files": response["editable_files"] = ["code/caller.cc"]
        if damage == "stale": response["snapshot"] = "f" * 64
        if damage in ("source_write", "exception_write"):
            (root / "code/caller.cc").write_text("// preserve unauthorized coordinator work\n")
        if damage == "exception_write": raise subprocess.TimeoutExpired("coordinator", 1)
        return 0, response

    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, execute=execute)
    assert (store.decisions(task.task_id), store.scope(task.task_id)) == before
    assert store.read(task.task_id) == task
    if damage in ("source_write", "exception_write"):
        assert (root / "code/caller.cc").read_text() == "// preserve unauthorized coordinator work\n"


@pytest.mark.parametrize("outcome", ["pending", "no_progress", "outside_edit", "second_deferred"])
@pytest.mark.parametrize("acceptance_scope", ["change", "full_project"])
def test_driver_local_investigation_preserves_all_findings_and_cannot_accept(setup, outcome, acceptance_scope):
    root, store, task, caller, delegation = setup
    discover(setup)
    store.discover(task.task_id, request_id="second", author="worker", hypothesis="Owner still needs structural evidence",
                   affects=[task.subjects[0].entity_id], evidence=["code/owner.cc"], expected_revision=1)
    delegation.update(subjects=[], editable_files=[], max_decisions=6)
    decisions, scope = store.decisions(task.task_id), store.scope(task.task_id)
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10},
              "check": {"argv": ["check"], "timeout": 10},
              "build": {"argv": ["build"], "timeout": 10},
              "acceptance_scope": acceptance_scope,
              "acceptance": {"argv": ["acceptance"], "timeout": 10}}
    if acceptance_scope == "change":
        config.update(obligations={"argv": ["inventory"], "timeout": 10},
                      migration_check={"argv": ["migration"], "timeout": 10})
    roles = {role: {"author": role, "command": {"argv": [role], "timeout": 10}} for role in ("worker", "reviewer")}
    calls = []

    def execute(command, root, name, payload):
        role = command.argv[0]
        calls.append(role)
        if role == "coordinate":
            assert {item["discovery_id"] for item in payload["pending_discoveries"]} == {"layout", "second"}
            pending = [item for item in payload["pending_discoveries"]
                       if item["discovery_id"] not in payload.get("investigating", [])]
            discovery = pending[0]["discovery_id"]
            action = ("defer" if outcome == "second_deferred" and discovery == "second" else
                      "investigate_local" if discovery == "layout" else "investigate")
            return 0, answer(payload, action, discovery_id=discovery)
        if role == "context": return 0, {}
        if role == "inventory": return 0, {"obligations": []}
        if role in ("check", "build"): return 0, {"result": "pass"}
        assert role == "worker", "Pending cross-owner findings must prevent review and either acceptance route"
        assert payload["editable_files"] == ["code/owner.cc"]
        context = payload["context"]
        assert {item["discovery_id"] for item in context["pending_discoveries"]} == {"layout", "second"}
        assert {item["action"] for item in context["discovery_investigations"]} == {"investigate_local", "investigate"}
        assert (store.decisions(task.task_id), store.scope(task.task_id)) == (decisions, scope)
        if outcome != "no_progress":
            path = root / ("code/caller.cc" if outcome == "outside_edit" else "code/owner.cc")
            path.write_text(path.read_text() + "// preserve investigated work\n")
        return 0, {"discoveries": []}

    def run():
        return drive(root, task.task_id, config, roles["worker"], iterations=1,
                     reviewer_configuration=roles["reviewer"], coordinator_configuration=delegation, execute=execute)

    if outcome == "outside_edit":
        with pytest.raises(ValueError, match="worker exceeded editing scope"):
            run()
        assert "preserve investigated work" in (root / "code/caller.cc").read_text()
    else:
        report = run()
        assert not report["ready_for_collection"]
        expected = {"pending": "evaluate_structural_discoveries", "no_progress": "no_source_progress",
                    "second_deferred": "coordination_defer"}
        assert report["stop_reason"] == expected[outcome]
    assert calls.count("worker") == (0 if outcome == "second_deferred" else 1)
    assert not {"reviewer", "acceptance", "migration"} & set(calls)
    assert (store.decisions(task.task_id), store.scope(task.task_id)) == (decisions, scope)
    assert store.read(task.task_id) == task
    status = store.status(task.task_id, scope[0])
    assert status["scope_expansion_required"] == [caller.entity_id]
    assert len(status["pending_discoveries"]) == 2


def test_local_investigation_refuses_source_change_after_authorization(setup, monkeypatch):
    import match_recovery.coordination as implementation
    root, store, task, _, delegation = setup
    discover(setup)
    delegation.update(subjects=[], editable_files=[])
    original = implementation.coordinate

    def changed_after_authorization(*args, **kwargs):
        response = original(*args, **kwargs)
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// concurrent edit preserved\n")
        return response

    monkeypatch.setattr(implementation, "coordinate", changed_after_authorization)
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10}, "check": {"argv": ["check"], "timeout": 10}}
    role = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}

    def execute(command, root, name, payload):
        if command.argv[0] == "context": return 0, {}
        if command.argv[0] == "check": return 0, {"result": "pass"}
        assert command.argv[0] == "coordinate"
        return 0, answer(payload, "investigate_local")

    with pytest.raises(ValueError, match="source changed after discovery investigation authorization"):
        drive(root, task.task_id, config, role, iterations=1, coordinator_configuration=delegation, execute=execute)
    assert "concurrent edit preserved" in (root / "code/owner.cc").read_text()
    assert len(store.decisions(task.task_id)) == 1


def test_native_bridge_validates_local_action_and_keeps_cross_owner_investigation_visible(setup):
    root, store, task, caller, delegation = setup
    discover(setup)
    store.discover(task.task_id, request_id="second", author="worker", hypothesis="Another local concern",
                   affects=[task.subjects[0].entity_id], evidence=["code/owner.cc"], expected_revision=1)
    delegation.update(subjects=[], editable_files=[])
    script = ("import json,sys; from pathlib import Path; "
              "p=json.loads(Path(sys.argv[1]).read_text()); s=json.loads(Path(sys.argv[2]).read_text()); sys.stdin.read(); "
              "assert 'investigate_local' in s['properties']['action']['enum']; "
              "assert len(p['pending_discoveries']) == 2; "
              "assert p['pending_discoveries'][0]['affects'][0] not in p['subjects']; "
              "assert p.get('investigating',[]) in ([],['layout']); "
              "first=not p.get('investigating'); "
              "print(json.dumps(dict(snapshot=p['snapshot'],action='investigate_local' if first else 'investigate',"
              "discovery_id='layout' if first else 'second',subjects=[],editable_files=[],"
              "evidence=['Investigate authorized owner only; retain external dependency finding'])))")
    role = {"command": {"argv": [sys.executable, "-c", script, "{request}", "{schema}"], "timeout": 10},
            "response": "stdout", "instructions": []}
    (root / "recovery/agents.json").write_text(json.dumps({"schema_version": 1, "artifacts": "build/agents",
                                                          "worker": role, "reviewer": role, "coordinator": role}))
    delegation["command"] = {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", "coordinator"], "timeout": 20}
    decisions, scope = store.decisions(task.task_id), store.scope(task.task_id)
    first = coordinate(root, task.task_id, delegation)
    assert first["action"] == "investigate_local" and first["discovery_id"] == "layout" and not first["published"]
    second = coordinate(root, task.task_id, delegation, investigating=["layout"])
    assert second["action"] == "investigate" and second["discovery_id"] == "second" and not second["published"]
    assert (store.decisions(task.task_id), store.scope(task.task_id)) == (decisions, scope)
    assert store.status(task.task_id, scope[0])["scope_expansion_required"] == [caller.entity_id]
