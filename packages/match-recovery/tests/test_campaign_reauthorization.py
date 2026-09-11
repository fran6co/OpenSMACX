"""Reauthorize preserved work through the campaign, not by rewriting its ledgers."""

from copy import deepcopy
from dataclasses import replace
import json

import pytest

import match_recovery.campaign as native
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.model import Origin, digest
from match_recovery.tasks import TaskStore
from test_campaign import choose, configured_campaign, compiler_project, project
from test_campaign_resume import stopped
from test_task_source import git


@pytest.fixture
def preserved(configured_campaign):
    root, config = configured_campaign
    config["max_batches"] = 1
    config["coordinator"] = "recovery/coordinator.json"
    previous = {"schema_version": 1, "author": "coordinator", "subjects": [], "editable_files": [],
                "max_decisions": 1, "evidence": ["Original bounded coordination"],
                "command": json.loads((root / "recovery/planner.json").read_text())["command"]}
    (root / config["coordinator"]).write_text(json.dumps(previous))
    git(root, "add", "recovery/coordinator.json")
    git(root, "commit", "-m", "Initial coordinator authority")
    root, config, directory, child = stopped((root, config))
    path = child / "code/model.h"
    path.write_text(path.read_text() + "// Preserved structural candidate\n")
    return root, config, directory, child, previous


def authorization(preserved, *, subjects=(), files=()):
    root, config, directory, child, previous = preserved
    coordinator = {**previous, "max_decisions": 2, "subjects": list(subjects), "editable_files": list(files),
                   "evidence": ["Authorize the next bounded investigation without resetting its baseline"]}
    (root / config["coordinator"]).write_text(json.dumps(coordinator))
    git(root, "add", "recovery/coordinator.json")
    git(root, "commit", "-m", "Explicit revised coordinator delegation")
    manifest = json.loads((directory / "run.json").read_text())
    return {"schema_version": 1, "author": config["author"], "run_sha256": digest(manifest),
            "batch_sha256": digest(json.loads((directory / "batch-0-workers.json").read_text())),
            "previous_profiles_sha256": manifest["profiles_sha256"], "previous_coordinator": previous,
            "coordinator": coordinator, "destination_revision": git(root, "rev-parse", "HEAD"),
            "evidence": ["Test-owned explicit reauthorization for the remaining bounded campaign"]}


def test_reauthorization_resumes_actual_wip_without_rewriting_assignments(preserved, monkeypatch):
    root, config, directory, child, previous = preserved
    token = authorization(preserved)
    manifest = (directory / "run.json").read_bytes()
    assignment = (directory / "batch-0-workers.json").read_bytes()
    before = working_state(child)
    baseline = TaskStore(child).read("owner").base_revision
    calls = []
    def drive(path, name, *args, **kwargs):
        assert path == child and name == "owner"
        assert kwargs["coordinator_configuration"] == token["coordinator"]
        calls.append(name)
        return {"stop_reason": "no_source_progress"}  # Not a passing recovery receipt.
    monkeypatch.setattr(native, "drive", drive)
    report = native.campaign(root, config, resume=directory, coordinator_authorization=token, execute=choose)
    assert report["stop_reason"] == "replanning_required" and not report["full_project_accepted"]
    assert calls == ["owner"] and working_state(child) == before
    assert TaskStore(child).read("owner").base_revision == baseline
    assert (directory / "run.json").read_bytes() == manifest
    assert (directory / "batch-0-workers.json").read_bytes() == assignment
    records = list(directory.glob("coordinator-authorization-*.json"))
    assert len(records) == 1 and json.loads(records[0].read_text()) == token
    # Neither a saved result nor an already recorded permission replaces a run.
    (directory / "report.json").write_text('{"full_project_accepted": true}')
    native.campaign(root, config, resume=directory, execute=choose)
    native.campaign(root, config, resume=directory, coordinator_authorization=token, execute=choose)
    assert calls == ["owner"] * 3 and len(list(directory.glob("coordinator-authorization-*.json"))) == 1
    assert working_state(child) == before


def test_reauthorized_discovery_expands_through_native_coordinator_then_resumes(preserved, monkeypatch):
    from match_recovery.coordination import coordinate
    root, config, directory, child, previous = preserved
    store = TaskStore(child)
    task = store.read("owner")
    original = task.subjects[0]
    dependency = Origin(original.image_sha256, original.address_space, 0x401010)
    store.discover("owner", request_id="caller-dependency", author="worker",
                   hypothesis="The owner recovery needs the caller's matching declaration",
                   affects=[dependency.entity_id], evidence=["source:code/caller.cc"], expected_revision=0)
    token = authorization(preserved, subjects=[dependency.entity_id], files=["code/caller.cc"])
    source_before = (child / "code/model.h").read_bytes()
    events = []
    def drive(path, name, *args, **kwargs):
        if dependency.entity_id in TaskStore(path).scope(name)[0]:
            return {"stop_reason": "pending_discovery"}
        def expand(command, current, task_name, request):
            return 0, {"snapshot": request["snapshot"], "action": "expand", "discovery_id": "caller-dependency",
                       "subjects": [dependency.entity_id], "editable_files": ["code/caller.cc"],
                       "evidence": ["source:controlled cross-owner relationship"]}
        events.append(coordinate(path, name, kwargs["coordinator_configuration"], execute=expand))
        return {"stop_reason": "pending_discovery"}
    monkeypatch.setattr(native, "drive", drive)
    first = native.campaign(root, config, resume=directory, coordinator_authorization=token, execute=choose)
    assert first["stop_reason"] == "replanning_required" and events[0]["published"]
    assert dependency.entity_id in store.scope("owner")[0] and "code/caller.cc" in store.scope("owner")[1]
    assert store.status("owner", store.scope("owner")[0])["pending_discoveries"]
    assert store.read("owner").base_revision == task.base_revision
    # An exact permission retry does not newly grant the scope it already gave.
    second = native.campaign(root, config, resume=directory, coordinator_authorization=token, execute=choose)
    assert second["stop_reason"] == "replanning_required" and len(events) == 1
    assert (child / "code/model.h").read_bytes() == source_before
    assert len(list(directory.glob("coordinator-authorization-*.json"))) == 1


def test_cleaned_retired_assignment_does_not_require_recreating_its_worker(preserved, monkeypatch):
    root, config, directory, child, previous = preserved
    # This fixture's only edit is test-owned WIP; restore its known source bytes
    # before using the ordinary native no-checkpoint retirement/cleanup path.
    (child / "code/model.h").write_bytes((root / "code/model.h").read_bytes())
    monkeypatch.setattr(native, "drive", lambda *a, **k: {"stop_reason": "no_source_progress"})
    first = native.campaign(root, config, resume=directory, execute=choose)
    assert first["stop_reason"] == "batch_budget_exhausted" and not child.exists()
    token = authorization(preserved)
    before = (directory / "batch-0-retired-owner.json").read_bytes()
    monkeypatch.setattr(native, "drive", lambda *a, **k: pytest.fail("retired worker restarted"))
    resumed = native.campaign(root, config, resume=directory, coordinator_authorization=token, execute=choose)
    assert resumed["stop_reason"] == "batch_budget_exhausted" and not resumed["full_project_accepted"]
    assert not child.exists() and (directory / "batch-0-retired-owner.json").read_bytes() == before
    assert len(list(directory.glob("coordinator-authorization-*.json"))) == 1


def test_cli_passes_bound_permission_to_native_resume(preserved, monkeypatch, capsys):
    from match_recovery.cli import main
    root, config, directory, child, previous = preserved
    token = authorization(preserved)
    path = root / "recovery/reauthorization.json"
    path.write_text(json.dumps(token))
    (root / "recovery/campaign.json").write_text(json.dumps(config))
    def called(actual_root, actual_config, **kwargs):
        assert actual_root == root and actual_config == config
        assert kwargs["resume"] == directory and kwargs["coordinator_authorization"] == token
        return {"stop_reason": "replanning_required", "unresolved_work": ["preserved WIP"]}
    monkeypatch.setattr(native, "campaign", called)
    assert main(["campaign", "--root", str(root), "--config", "recovery/campaign.json", "--resume", str(directory),
                 "--coordinator-authorization", "recovery/reauthorization.json"]) == 3
    assert "replanning_required" in capsys.readouterr().out


@pytest.mark.parametrize("damage", ["leased", "outside", "baseline", "missing", "staged"])
def test_reauthorization_preserves_unsafe_worker_without_publishing_permission(preserved, monkeypatch, damage):
    root, config, directory, child, previous = preserved
    token = authorization(preserved)
    if damage == "outside":
        path = child / "code/caller.cc"
        path.write_text(path.read_text() + "// Unapproved work\n")
    elif damage == "baseline":
        path = child / "recovery/tasks/owner.json"
        task = json.loads(path.read_text())
        task["base_revision"] = "0" * 40
        path.write_text(json.dumps(task))
    elif damage == "missing":
        (child / "code/model.h").unlink()
        git(child, "add", "code/model.h")  # Partial publication is never repaired by reauthorization.
    elif damage == "staged":
        git(child, "add", "code/model.h")
    before = working_state(child)
    monkeypatch.setattr(native, "drive", lambda *a, **k: pytest.fail("unsafe worker ran"))
    if damage == "leased":
        with file_lock(child / "build/recovery/driver.lock"):
            with pytest.raises(TimeoutError):
                native.campaign(root, config, resume=directory, coordinator_authorization=token, execute=choose)
    else:
        with pytest.raises(ValueError):
            native.campaign(root, config, resume=directory, coordinator_authorization=token, execute=choose)
    assert working_state(child) == before
    assert not list(directory.glob("coordinator-authorization-*.json"))


@pytest.mark.parametrize("owner", ["assigned", "other"])
def test_reauthorization_cannot_grant_already_owned_source(preserved, monkeypatch, owner):
    root, config, directory, child, previous = preserved
    store = TaskStore(root)
    task = store.read("owner")
    if owner == "other":
        original = task.subjects[0]
        task = replace(task, task_id="other", subjects=(Origin(
            original.image_sha256, original.address_space, original.address + 0x100),),
            editable_files=("code/caller.cc",))
        store.create(task)
        git(root, "add", "recovery/tasks/other.json")
        git(root, "commit", "-m", "Reserve another source owner")
    token = authorization(preserved, subjects=[task.subjects[0].entity_id], files=task.editable_files)
    before = working_state(child)
    monkeypatch.setattr(native, "drive", lambda *a, **k: pytest.fail("overlapping worker ran"))
    with pytest.raises(ValueError, match="overlaps existing task ownership"):
        native.campaign(root, config, resume=directory, coordinator_authorization=token, execute=choose)
    assert working_state(child) == before
    assert not list(directory.glob("coordinator-authorization-*.json"))


def test_shared_grants_exclude_a_siblings_newly_acquired_scope(preserved):
    root, config, directory, child, previous = preserved
    task = TaskStore(root).read("owner")
    original = task.subjects[0]
    acquired = Origin(original.image_sha256, original.address_space, original.address + 0x100)
    sibling = replace(task, task_id="sibling", subjects=(acquired,), editable_files=("code/caller.cc",))
    TaskStore(child).create(sibling)
    profile = {**previous, "subjects": [original.entity_id, acquired.entity_id],
               "editable_files": ["code/model.h", "code/caller.cc"]}
    before = deepcopy(profile)
    narrowed = native._scoped_coordinator(root, profile, "owner", [{"task": "sibling", "root": str(child)}])
    assert narrowed["subjects"] == [original.entity_id] and narrowed["editable_files"] == ["code/model.h"]
    assert profile == before
    # Collected peer ownership remains excluded after its checkout is gone.
    TaskStore(root).create(sibling)
    assert native._scoped_coordinator(root, profile, "owner", []) == narrowed
