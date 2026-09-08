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
