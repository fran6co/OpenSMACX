"""Synthetic coordinator-call accounting, not evidence of live recovery."""

from copy import deepcopy
import json
from pathlib import Path
import subprocess

import pytest

from match_recovery.coordination import DecisionBudget, SerialCoordinationRequired, coordinate
from match_recovery.driver import drive, working_state
from match_recovery.investigation_round import authorize
from match_recovery.locking import file_lock
from match_recovery.tasks import TaskStore
from test_campaign_parallel_resume import diagnostic_result, retained_batch
from test_coordination import answer, discover, setup
from test_dispatch import batch
from test_source_configuration import configured
from test_task_source import git, project


def command(name):
    return {"argv": [name], "timeout": 10}


def driver_config(task):
    return {"schema_version": 1, "target": task.target,
            "context": command("context"), "check": command("check")}


def role(name):
    return {"author": name, "command": command(name)}


@pytest.mark.parametrize("limit", [0, -1, 101, True, "3", None])
def test_budget_requires_a_valid_bounded_integer(limit):
    with pytest.raises(ValueError):
        DecisionBudget(limit)


def test_exhaustion_prevents_diagnostics_and_provider_without_mutating_policy(setup):
    root, store, task, _, delegation = setup
    discover(setup)
    delegation["max_decisions"] = 1
    original = deepcopy(delegation)
    budget = DecisionBudget(1)
    assert (budget.used, budget.remaining) == (0, 1)
    calls = []

    def execute(cmd, current, name, payload):
        calls.append(cmd.argv[0])
        if cmd.argv[0] == "context":
            assert budget.used == 0
            return 0, {}
        if cmd.argv[0] == "check":
            assert budget.used == 0
            return 1, {"result": "fail"}
        assert cmd.argv == ("coordinate",) and budget.used == 1
        return 0, answer(payload, "defer")

    before = working_state(root), store.decisions(task.task_id), store.scope(task.task_id)
    first = coordinate(root, task.task_id, delegation, execute=execute,
                       diagnostic_configuration=driver_config(task), _budget=budget)
    assert first["action"] == "defer" and not first["published"]
    assert calls == ["context", "check", "coordinate"]
    second = coordinate(root, task.task_id, delegation,
                        execute=lambda *args: pytest.fail("Exhaustion must skip all commands"),
                        diagnostic_configuration=driver_config(task), _budget=budget)
    assert second["action"] == "budget_exhausted"
    assert second["published"] is False and second["ready_for_collection"] is False
    assert (budget.used, budget.remaining) == (1, 0)
    assert delegation == original
    assert (working_state(root), store.decisions(task.task_id), store.scope(task.task_id)) == before


@pytest.mark.parametrize("outcome", ["rejected", "unavailable", "invalid", "stale_response", "timeout"])
def test_every_invoked_coordinator_attempt_is_charged_once(setup, outcome):
    root, store, task, _, delegation = setup
    discover(setup)
    budget = DecisionBudget(delegation["max_decisions"])
    before = working_state(root), store.decisions(task.task_id)
    calls = []

    def execute(cmd, current, name, payload):
        calls.append(payload)
        assert budget.used == 1
        if outcome == "unavailable": return 3, {"error": "synthetic unavailable"}
        if outcome == "invalid": return 0, {"not": "a coordinator response"}
        if outcome == "timeout": raise subprocess.TimeoutExpired("synthetic coordinator", 1)
        return 0, answer(payload, "expand", subjects=[], editable_files=[],
                         **({"snapshot": "f" * 64} if outcome == "stale_response" else {}))

    if outcome in ("invalid", "stale_response", "timeout"):
        with pytest.raises(subprocess.TimeoutExpired if outcome == "timeout" else ValueError):
            coordinate(root, task.task_id, delegation, execute=execute, _budget=budget)
    else:
        report = coordinate(root, task.task_id, delegation, execute=execute, _budget=budget)
        assert report["action"] == ("proposal_rejected" if outcome == "rejected" else "unavailable")
        assert not report["published"] and not report["ready_for_collection"]
    assert len(calls) == 1 and (budget.used, budget.remaining) == (1, 2)
    assert (working_state(root), store.decisions(task.task_id)) == before


@pytest.mark.parametrize("damage", ["budget_limit", "policy", "diagnostic_source"])
def test_failures_before_coordinator_invocation_do_not_charge_a_call(setup, damage):
    root, store, task, _, delegation = setup
    discover(setup)
    budget = DecisionBudget(2 if damage == "budget_limit" else delegation["max_decisions"])
    if damage == "policy": delegation["author"] = "not-the-coordinator"
    calls = []

    def execute(cmd, current, name, payload):
        assert cmd.argv[0] in ("context", "check"), "Unsafe preflight reached coordinator"
        calls.append(cmd.argv[0])
        path = root / "code/owner.hpp"
        path.write_text(path.read_text() + "// preserved illegal diagnostic mutation\n")
        return 0, {}

    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, execute=execute,
                   diagnostic_configuration=driver_config(task), _budget=budget)
    assert budget.used == 0
    assert bool(calls) == (damage == "diagnostic_source")
    assert len(store.decisions(task.task_id)) == 1
    if damage == "diagnostic_source":
        assert "illegal diagnostic mutation" in (root / "code/owner.hpp").read_text()


@pytest.mark.parametrize("limit", [1, 2, 3])
def test_restricted_serial_handoff_shares_actual_call_budget_without_replaying_proposal(setup, limit):
    root, store, task, caller, delegation = setup
    discover(setup)
    delegation["max_decisions"] = limit
    original = deepcopy(delegation)
    budget = DecisionBudget(limit)
    calls, snapshots = [], []

    def execute(cmd, current, name, payload):
        kind = cmd.argv[0]
        calls.append(kind)
        if kind == "context": return 0, {}
        if kind == "check": return 1, {"result": "fail"}
        if kind == "worker": return 0, {"discoveries": []}
        assert kind == "coordinate", "No review or acceptance is available for a pending finding"
        snapshots.append(payload["snapshot"])
        assert budget.used == calls.count("coordinate")
        action = "expand" if calls.count("coordinate") <= 2 else "investigate"
        return 0, answer(payload, action, editable_files=["code/caller.cc"] if action == "expand" else [])

    before = working_state(root), store.decisions(task.task_id), store.scope(task.task_id)
    with pytest.raises(SerialCoordinationRequired):
        authorize(root, task.task_id, delegation, execute=execute, _budget=budget)
    assert (budget.used, budget.remaining) == (1, limit - 1)
    assert (working_state(root), store.decisions(task.task_id), store.scope(task.task_id)) == before
    report = drive(root, task.task_id, driver_config(task), role("worker"), iterations=1,
                   reviewer_configuration=role("reviewer"), coordinator_configuration=delegation,
                   execute=execute, _coordination_budget=budget)
    assert calls.count("coordinate") == limit == budget.used
    assert budget.remaining == 0 and report["coordinator_decisions"] == limit
    assert report["stop_reason"] == ("no_source_progress" if limit == 3 else "coordination_budget_exhausted")
    assert calls.count("worker") == (1 if limit == 3 else 0)
    assert not report["ready_for_collection"] and delegation == original
    assert store.read(task.task_id) == task
    assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
    if limit > 1:
        assert snapshots[0] != snapshots[1]  # Fresh serial request, not replayed preplanning response.
        assert caller.entity_id in store.scope(task.task_id)[0]


def test_investigation_grant_does_not_embed_or_change_shared_budget_authority(setup):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    budget = DecisionBudget(delegation["max_decisions"])
    before = working_state(root), deepcopy(delegation)
    report = authorize(root, task.task_id, delegation, _budget=budget,
                       execute=lambda *args: (0, answer(args[-1], "investigate")))
    grant = report["authorization"]
    assert report["status"] == "authorized" and (budget.used, budget.remaining) == (1, 2)
    assert set(grant) == {"schema_version", "kind", "request_id", "root", "task", "author", "epoch",
                          "delegation_sha256", "investigations"}
    assert (working_state(root), delegation) == before
    assert len(store.decisions(task.task_id)) == 1


@pytest.mark.parametrize("limit", [2, 3])
def test_partial_investigation_coverage_counts_rejected_attempt_not_just_grants(setup, limit):
    root, store, task, caller, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    store.discover(task.task_id, request_id="outside", author="worker", hypothesis="Known outside impact",
                   affects=[caller.entity_id], evidence=["Synthetic caller evidence"], expected_revision=1)
    delegation["max_decisions"] = limit
    budget = DecisionBudget(limit)
    before = working_state(root), store.decisions(task.task_id), store.scope(task.task_id)
    calls = []

    def execute(cmd, current, name, payload):
        calls.append(payload)
        assert budget.used == len(calls)
        if len(calls) == 1: return 0, answer(payload, "investigate")
        assert payload["investigating"] == ["layout"]
        return 0, answer(payload, "incorporate" if len(calls) == 2 else "investigate_local",
                         discovery_id="outside")

    report = authorize(root, task.task_id, delegation, execute=execute, _budget=budget)
    assert len(calls) == budget.used == limit and budget.remaining == 0
    if limit == 2:
        assert report["status"] == "deferred" and "authorization" not in report
    else:
        assert report["status"] == "authorized"
        assert [row["discovery_id"] for row in report["authorization"]["investigations"]] == ["layout", "outside"]
    assert (working_state(root), store.decisions(task.task_id), store.scope(task.task_id)) == before


def test_resume_round_charges_separate_task_counters_and_serial_cannot_reset_them(retained_batch):
    from match_recovery.campaign import _resume_round

    root, plan, profiles, directory, revision = retained_batch
    budgets = {row["task"]: DecisionBudget(1) for row in plan["jobs"]}
    before, original_profiles = working_state(root), deepcopy(profiles)
    calls = []

    def execute(cmd, child, name, payload):
        result = diagnostic_result(cmd, child, name, payload, profiles, calls)
        if result is not None: return result
        assert budgets[name].used == 1
        return 0, answer(payload, "expand", discovery_id="pending", subjects=[], editable_files=["code/owner.hpp"])

    with file_lock(root / "build/recovery/driver.lock", timeout=0) as lease:
        report, decisions, deferred = _resume_round(
            root, plan, profiles, directory, 0, revision, execute=execute,
            fresh=lambda: None, announce=lambda *args, **kwargs: None, lease=lease,
            coordination_budgets=budgets)
    assert report is None and not deferred
    assert [row["status"] for row in decisions] == ["serial_coordination_required"] * 2
    assert calls == [(name, stage) for name in ("owner", "other") for stage in ("context", "check", "coordinate")]
    for row in plan["jobs"]:
        child, name = Path(row["root"]), row["task"]
        store = TaskStore(child)
        child_before = working_state(child), store.decisions(name)
        result = drive(child, name, driver_config(store.read(name)), role("worker"), iterations=1,
                       coordinator_configuration=profiles["coordinator"], _coordination_budget=budgets[name],
                       execute=lambda *args: pytest.fail("Serial fallback cannot reset preplanning budget"))
        assert result["stop_reason"] == "coordination_budget_exhausted"
        assert result["coordinator_decisions"] == 1 and result["worker_rounds"] == 0
        assert (working_state(child), store.decisions(name)) == child_before
    assert working_state(root) == before and profiles == original_profiles
    assert not list((root / "build/signals").iterdir())


def test_native_campaign_preserves_clean_unresolved_worker_when_preplanning_spends_serial_budget(setup, monkeypatch):
    import match_recovery.campaign as campaign

    root, store, task, _, delegation = setup
    discover(setup)
    delegation["max_decisions"] = 1
    source_config = json.loads((root / "recovery/project.json").read_text())
    source_config["compiler"] = {"environment": ["TEST_COMPILER_SLOT"]}
    (root / "recovery/project.json").write_text(json.dumps(source_config))
    slot = root / "build/budget-slot"
    slot.mkdir(parents=True)
    profiles = {
        "planner": {"schema_version": 1, "author": "coordinator", "command": command("planner"),
                    "max_entities": 10, "max_editable_files": 4, "commit_decision": True,
                    "evidence": ["Synthetic planner authority"]},
        "driver": {**driver_config(task), "acceptance_scope": "change",
                   "obligations": command("obligations"), "migration_check": command("migration_check")},
        "worker": role("worker"), "reviewer": role("reviewer"), "coordinator": delegation,
        "checkpoint": {"author": "coordinator", "message": "Synthetic checkpoint", "allow_migration": False},
        "resources": {"schema_version": 1, "slots": [{"name": "budget-slot", "directory": str(slot),
                       "environment": {"TEST_COMPILER_SLOT": "{resource}"}}]},
    }
    for name, value in profiles.items():
        (root / f"recovery/{name}.json").write_text(json.dumps(value))
    configuration = {"schema_version": 1, "author": "coordinator", "destination_branch": "refs/heads/work",
                     **{name: f"recovery/{name}.json" for name in profiles}, "max_batches": 1, "max_parallel": 1,
                     "iterations": 1, "timeout": 10, "evidence": ["Synthetic retained budget control"],
                     "publication": {"enabled": True, "allow_migration": False,
                                     "message": "Synthetic publication", "cleanup": True}}
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic clean unresolved campaign assignment")
    monkeypatch.setattr(campaign, "select_task", lambda *args, **kwargs: {"action": "resume", "task_id": "owner"})
    native_prepare = campaign.prepare_worker

    def prepare(current, name, authorization, **kwargs):
        # Exercise native checkout/assignment creation, not a real compiler.
        result = native_prepare(current, name, {**authorization, "prepare": False},
                                **{**kwargs, "resources": None})
        return {**result, "compiler_prepared": True}

    monkeypatch.setattr(campaign, "prepare_worker", prepare)
    monkeypatch.setattr(campaign, "prepare_project", lambda *args: {"result": "pass", "published": True})
    monkeypatch.setattr(campaign, "dispatch", lambda *args, **kwargs: pytest.fail("No exhausted worker may dispatch"))
    for name in ("prepare_integration", "publish_integration", "_remove_integrated", "retire"):
        monkeypatch.setattr(campaign, name, lambda *args, **kwargs: pytest.fail("Unresolved worker must remain intact"))

    class Stopped(RuntimeError):
        pass

    def stop_before_dispatch(event):
        if event["stage"] == "dispatch": raise Stopped("Synthetic interruption before any worker")

    with pytest.raises(Stopped):
        campaign.campaign(root, configuration, progress=stop_before_dispatch,
                          execute=lambda *args: pytest.fail("No provider needed for fixed synthetic task selection"))
    directory, = (root / "build/recovery/campaigns").iterdir()
    child = directory / "batch-0-worker-0"
    assert child.is_dir() and not git(child, "status", "--porcelain")
    before = working_state(root), working_state(child)
    frozen = {path: path.read_bytes() for path in directory.glob("*.json")}
    profile_bytes = {root / value: (root / value).read_bytes() for key, value in configuration.items() if key in profiles}
    calls, seen_budgets = [], []
    native_drive = campaign.drive

    def serial_drive(*args, **kwargs):
        budget = kwargs["_coordination_budget"]
        seen_budgets.append(budget)
        assert (budget.used, budget.remaining) == (1, 0)
        return native_drive(*args, **kwargs)

    monkeypatch.setattr(campaign, "drive", serial_drive)

    def execute(cmd, current, name, payload):
        calls.append(cmd.argv[0])
        if cmd.argv[0] == "context": return 0, {}
        if cmd.argv[0] == "check": return 1, {"result": "fail"}
        assert cmd.argv == ("coordinate",), "Exhausted serial fallback must not reach worker/reviewer/acceptance"
        return 0, answer(payload, "expand", editable_files=["code/caller.cc"])

    report = campaign.campaign(root, configuration, resume=directory, execute=execute)
    assert calls == ["context", "check", "coordinate"] and len(seen_budgets) == 1
    assert report["stop_reason"] == "replanning_required"
    assert not report["ready_for_collection"] and not report["full_project_accepted"]
    current_batch, = report["batches"]
    assert current_batch["pending"][0]["task"] == "owner"
    assert current_batch["pending"][0]["reason"] == "coordination_budget_exhausted"
    assert not current_batch["integrations"]
    assert (working_state(root), working_state(child)) == before
    assert TaskStore(child).read("owner") == task
    assert TaskStore(child).decisions("owner") == store.decisions("owner")
    assert all(path.read_bytes() == contents for path, contents in {**frozen, **profile_bytes}.items())
    assert not list(directory.glob("*retired-owner.json"))
