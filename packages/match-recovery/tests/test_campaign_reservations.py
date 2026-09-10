"""Campaign/external-worktree boundaries using only synthetic repositories."""

from dataclasses import replace
import json
from pathlib import Path

import pytest

import match_recovery.campaign as implementation
from match_recovery.driver import working_state
from match_recovery.model import Origin
from match_recovery.tasks import TaskStore
from test_campaign import choose, configured_campaign, compiler_project, project
from test_campaign_resume import Interrupted, stopped
from test_task_source import git


def commit(root, message):
    git(root, "add", "recovery")
    git(root, "commit", "-m", message)


def another_task(root, name="parked"):
    store = TaskStore(root)
    owner = store.read("owner")
    task = replace(owner, task_id=name, subjects=(Origin("a" * 64, "pe:va", 0x401010),),
                   editable_files=("code/caller.cc",))
    store.create(task)
    commit(root, "Authorize separate synthetic caller task")
    return task


def parked(root, *, task="owner", declared=True):
    child = root / "build/manual-worker"
    if declared:
        profile = root / "recovery/planner.json"
        policy = json.loads(profile.read_text())
        policy["reserved_worktrees"] = [{"path": str(child), "tasks": [task],
                                          "evidence": ["Preserve separately authorized synthetic WIP"]}]
        profile.write_text(json.dumps(policy))
        commit(root, "Park external task without copying its scope")
    git(root, "worktree", "add", "-b", "manual-worker", str(child))
    return child


def forbid_execution(monkeypatch):
    def forbidden(*args, **kwargs):
        pytest.fail("reserved work must refuse before preparation, dispatch or drive")
    for name in ("prepare_worker", "_existing_worker", "dispatch", "drive", "prepare_integration"):
        monkeypatch.setattr(implementation, name, forbidden)


@pytest.mark.parametrize("kind", ["dirty", "committed", "committed-reversal", "staged-reversal"])
def test_unaccounted_external_wip_refuses_before_campaign_agents(configured_campaign, monkeypatch, kind):
    root, config = configured_campaign
    child = parked(root, declared=False)
    path = child / "code/owner.cc"
    original = path.read_bytes()
    path.write_bytes(original + b"// external recovery work\n")
    if kind != "dirty":
        git(child, "add", "code/owner.cc")
    if kind in ("committed", "committed-reversal"):
        git(child, "commit", "-m", "Preserve unmerged synthetic work")
    if kind.endswith("reversal"):
        path.write_bytes(original)
    before, external = working_state(root), working_state(child)
    forbid_execution(monkeypatch)
    with pytest.raises(ValueError, match="unreserved recovery worktree"):
        implementation.campaign(root, config, execute=lambda *args: pytest.fail("planner must not run"))
    assert working_state(root) == before and working_state(child) == external


@pytest.mark.parametrize("force_resume", [False, True])
def test_parked_scope_makes_existing_task_ineligible(configured_campaign, monkeypatch, force_resume):
    root, config = configured_campaign
    child = parked(root)
    path = child / "code/owner.cc"
    path.write_text(path.read_text() + "// preserved manual work\n")
    before, external = working_state(root), working_state(child)
    requests = []
    def planner(command, current, name, payload):
        requests.append(payload)
        owner, = [row for row in payload["tasks"] if row["task"]["task_id"] == "owner"]
        assert owner["eligible"] is False
        assert payload["reservations"][0]["tasks"] == ["owner"]
        if force_resume:
            return 0, {"snapshot": payload["snapshot"], "action": "resume", "task_id": "owner",
                       "group": "", "intent": "", "editable_files": [], "evidence": ["Try reserved task"]}
        return choose(command, current, name, payload)
    forbid_execution(monkeypatch)
    if force_resume:
        with pytest.raises(ValueError, match="within delegated authority"):
            implementation.campaign(root, config, execute=planner)
    else:
        report = implementation.campaign(root, config, execute=planner)
        assert report["stop_reason"] == "planning_defer" and report["batches"][0]["workers"] == []
    assert len(requests) == 1
    assert working_state(root) == before and working_state(child) == external


@pytest.mark.parametrize("dimension", ["subject", "file"])
def test_resume_rederives_parked_expansion_before_preparing_saved_assignment(configured_campaign, monkeypatch, dimension):
    root, config = configured_campaign
    another_task(root)
    external = parked(root, task="parked")
    root, config, directory, worker = stopped(configured_campaign)
    store = TaskStore(external)
    store.expand("parked", request_id="late-parked-scope", author="coordinator", expected_revision=0,
                 subjects=TaskStore(root).read("owner").subjects if dimension == "subject" else (),
                 editable_files=["code/model.h"] if dimension == "file" else (),
                 evidence=["Coupled ownership discovered while campaign was stopped"])
    before, parked_before, worker_before = working_state(root), working_state(external), working_state(worker)
    forbid_execution(monkeypatch)
    with pytest.raises(ValueError, match="overlaps a reserved worktree"):
        implementation.campaign(root, config, resume=directory,
                                execute=lambda *args: pytest.fail("resumed agent must not run"))
    assert working_state(root) == before
    assert working_state(external) == parked_before and working_state(worker) == worker_before


@pytest.mark.parametrize("dimension", ["subject", "file"])
def test_reserved_coordinator_expansion_refuses_before_any_decision(configured_campaign, monkeypatch, dimension):
    root, config = configured_campaign
    config["coordinator"] = "recovery/coordinator.json"
    delegation = {"schema_version": 1, "author": "coordinator",
                  "command": json.loads((root / "recovery/planner.json").read_text())["command"],
                  "subjects": [TaskStore(root).read("owner").subjects[0].entity_id] if dimension == "subject" else [],
                  "editable_files": ["code/model.h"] if dimension == "file" else [],
                  "evidence": ["Test delegated overlap refusal"], "max_decisions": 2}
    (root / config["coordinator"]).write_text(json.dumps(delegation))
    child = parked(root)  # Commits both profiles before creating the parked checkout.
    before, external = working_state(root), working_state(child)
    forbid_execution(monkeypatch)
    with pytest.raises(ValueError, match="overlaps a reserved worktree"):
        implementation.campaign(root, config, execute=lambda *args: pytest.fail("no decision agent may run"))
    assert working_state(root) == before and working_state(child) == external
    assert TaskStore(root).decisions("owner") == TaskStore(child).decisions("owner") == ()


def test_later_batch_resume_does_not_hide_new_wip_in_older_retired_worker(configured_campaign, monkeypatch):
    root, config = configured_campaign
    another_task(root, "second")
    config["publication"]["cleanup"] = False
    def no_op(current, plan, **kwargs):
        return {"coordinator_changed": False, "batch_completed": False,
                "jobs": [{"timed_out": False, "scope_intact": True, "returncode": 3} for row in plan["jobs"]]}
    monkeypatch.setattr(implementation, "dispatch", no_op)
    def progress(event):
        if event["stage"] == "dispatch" and event["batch"] == 1:
            raise Interrupted("retain batch zero, pause batch one")
    with pytest.raises(Interrupted):
        implementation.campaign(root, config, execute=choose, progress=progress)
    directory, = (root / "build/recovery/campaigns").iterdir()
    older, latest = directory / "batch-0-worker-0", directory / "batch-1-worker-0"
    assert older.is_dir() and latest.is_dir()
    assert (directory / "batch-0-retired-owner.json").is_file()
    assert json.loads((directory / "batch-1-workers.json").read_text())["plan"]["jobs"][0]["task"] == "second"
    path = older / "code/owner.cc"
    path.write_text(path.read_text() + "// late uncollected work in retired assignment\n")
    before, old_before, latest_before = working_state(root), working_state(older), working_state(latest)
    forbid_execution(monkeypatch)
    with pytest.raises(ValueError, match="unreserved recovery worktree"):
        implementation.campaign(root, config, resume=directory,
                                execute=lambda *args: pytest.fail("do not resume over older WIP"))
    assert working_state(root) == before
    assert working_state(older) == old_before and working_state(latest) == latest_before


@pytest.mark.parametrize("kind", ["source", "history"])
@pytest.mark.parametrize("cleanup", [False, True])
def test_ignored_recovery_work_in_latest_retired_assignment_refuses_resume_and_cleanup(
        configured_campaign, monkeypatch, kind, cleanup):
    root, config = configured_campaign
    config["publication"]["cleanup"] = cleanup
    root, config, directory, child = stopped(configured_campaign)
    monkeypatch.setattr(implementation, "drive", lambda *args, **kwargs: {"stop_reason": "no_source_progress"})
    remove = implementation._remove_integrated
    if cleanup:
        def stop_before_cleanup(*args, **kwargs):
            raise Interrupted("retirement recorded; no checkout removed")
        monkeypatch.setattr(implementation, "_remove_integrated", stop_before_cleanup)
        with pytest.raises(Interrupted):
            implementation.campaign(root, config, resume=directory, execute=choose)
        monkeypatch.setattr(implementation, "_remove_integrated", remove)
    else:
        report = implementation.campaign(root, config, resume=directory, execute=choose)
        assert report["stop_reason"] == "planning_defer"
    assert (directory / "batch-0-retired-owner.json").is_file() and child.is_dir()
    # The task's source revision is already on the destination. Ignored source
    # or decisions must nevertheless prevent reusing/cleaning this assignment.
    assert git(child, "rev-parse", "HEAD") == git(root, "rev-parse", "HEAD")
    tracked_before = working_state(child)
    exclude = root / ".git/info/exclude"
    excluded = "code/late.cc" if kind == "source" else "recovery/history/"
    exclude.write_text(exclude.read_text() + "\n" + excluded + "\n")
    if kind == "source":
        path = child / "code/late.cc"
        path.write_text('// ORIGINAL: 0x00401010 _h 0x00401010-0x00401016\n'
                        'int h() { return 9; } // preserve ignored recovery source\n')
    else:
        store = TaskStore(child)
        store.expand("owner", request_id="ignored-retired-scope", author="coordinator", expected_revision=0,
                     subjects=(), editable_files=["code/caller.cc"],
                     evidence=["Preserve late coupled scope; not an acceptance decision"])
        path, = (child / "recovery/history").rglob("*.json")
    evidence = path.read_bytes()
    assert git(child, "check-ignore", str(path))
    assert working_state(child) == tracked_before  # Ordinary Git state misses this input.
    root_before = working_state(root)
    monkeypatch.setattr(implementation, "drive", lambda *args, **kwargs: pytest.fail("retired WIP must not drive"))
    try:
        with pytest.raises(ValueError):
            implementation.campaign(root, config, resume=directory, execute=choose)
    finally:
        assert child.is_dir() and path.is_file(), "cleanup discarded ignored recovery work"
        assert path.read_bytes() == evidence
        assert working_state(root) == root_before


def test_fresh_no_checkpoint_worker_keeps_ignored_source_as_uncommitted_progress(configured_campaign, monkeypatch):
    root, config = configured_campaign
    assert config["publication"]["cleanup"] is True
    created = []
    source = b"int unfinished_recovery() { return 9; } // retain this ignored source\n"
    def ignored_progress(current, plan, **kwargs):
        child = Path(plan["jobs"][0]["root"])
        before = working_state(child)
        exclude = root / ".git/info/exclude"
        exclude.write_text(exclude.read_text() + "\ncode/late.cc\n")
        path = child / "code/late.cc"
        path.write_bytes(source)
        created.append(path)
        assert git(child, "check-ignore", str(path))
        assert working_state(child) == before
        return {"coordinator_changed": False, "batch_completed": False,
                "jobs": [{"timed_out": False, "scope_intact": True, "returncode": 3}]}
    monkeypatch.setattr(implementation, "dispatch", ignored_progress)
    monkeypatch.setattr(implementation, "prepare_integration", lambda *args, **kwargs: pytest.fail("WIP must not integrate"))
    before = working_state(root)
    report = implementation.campaign(root, config, execute=choose)
    path, = created
    assert path.is_file(), "no-checkpoint cleanup discarded ignored recovery source"
    assert path.read_bytes() == source
    assert report["stop_reason"] == "replanning_required"
    assert report["unresolved_work"][0]["reason"] == "uncommitted_worker_progress"
    assert not list(Path(report["artifacts"]).glob("batch-*-retired-*.json"))
    assert working_state(root) == before
