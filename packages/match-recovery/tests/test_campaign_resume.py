"""Resume routing and refusal controls; no doubled verdict grants acceptance."""

import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import time

import pytest

import match_recovery.campaign as implementation
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.resources import ACTIVE
from match_recovery.tasks import TaskStore
from test_campaign import choose, configured_campaign, compiler_project, project
from test_task_source import git
from test_processes import _assert_stopped


class Interrupted(RuntimeError):
    pass


def stopped(configured_campaign, stage="dispatch"):
    root, config = configured_campaign
    def progress(event):
        if event["stage"] == stage:
            raise Interrupted("controlled stop before starting a process")
    with pytest.raises(Interrupted):
        implementation.campaign(root, config, execute=choose, progress=progress)
    directory, = (root / "build/recovery/campaigns").iterdir()
    return root, config, directory, directory / "batch-0-worker-0"


def refuse_drive(monkeypatch, inspect=None):
    def refused(path, *args, **kwargs):
        if inspect: inspect(path)
        return {"stop_reason": "no_source_progress"}
    monkeypatch.setattr(implementation, "drive", refused)


@pytest.mark.parametrize("stage", ["prepare", "dispatch"])
def test_resume_reuses_assignment_and_baseline_without_accepting_reports(configured_campaign, monkeypatch, stage):
    root, config, directory, child = stopped(configured_campaign, stage)
    base = TaskStore(root).read("owner").base_revision
    refuse_drive(monkeypatch)
    (directory / "report.json").write_text('{"full_project_accepted":true}')
    report = implementation.campaign(root, config, resume=directory, execute=choose)
    worker, = report["batches"][0]["workers"]
    assert worker["worktree"] == str(child) and worker["task_base_revision"] == base
    assert worker["worker_created"] == (stage == "prepare")
    assert report["batches"][0]["dispatch"] is None and not report["full_project_accepted"]
    assert report["stop_reason"] == "planning_defer" and report["unresolved_work"]
    assert not child.exists() and not git(root, "branch", "--list", "recovery/campaign-*")


def test_resume_preserves_uncommitted_structural_work(configured_campaign, monkeypatch):
    root, config, directory, child = stopped(configured_campaign)
    path = child / "code/model.h"
    path.write_text(path.read_text() + "// keep structural WIP\n")
    before = working_state(child)
    refuse_drive(monkeypatch, inspect=lambda current: assert_wip(current))
    def assert_wip(current):
        assert current == child and "structural WIP" in path.read_text()
    report = implementation.campaign(root, config, resume=directory, execute=choose)
    assert report["stop_reason"] == "replanning_required"
    assert report["unresolved_work"][0]["reason"] == "uncommitted_worker_progress"
    assert working_state(child) == before and path.exists()


def test_fresh_invocation_cannot_duplicate_a_preserved_worker(configured_campaign):
    root, config, directory, child = stopped(configured_campaign)
    before = working_state(child)
    with pytest.raises(ValueError, match="--resume"):
        implementation.campaign(root, config, execute=choose)
    assert list((root / "build/recovery/campaigns").iterdir()) == [directory]
    assert working_state(child) == before


@pytest.mark.parametrize("damage", ["leased", "quarantined", "policy", "staged", "branch", "outside"])
def test_resume_refuses_unsafe_workers_before_running_agents(configured_campaign, monkeypatch, damage):
    root, config, directory, child = stopped(configured_campaign)
    if damage in ("policy", "staged", "outside"):
        path = child / ("code/model.h" if damage == "staged" else "recovery/worker.json" if damage == "policy" else "code/caller.cc")
        path.write_text(path.read_text() + "\n")
        if damage == "staged": git(child, "add", "code/model.h")
    if damage == "branch": git(child, "branch", "-m", "unexpected-worker")
    slot = root / "build/compiler-slot"
    if damage == "quarantined": (slot / ACTIVE).write_text("old marker is not proof of descendant termination")
    before = working_state(child)
    monkeypatch.setattr(implementation, "drive", lambda *args, **kwargs: pytest.fail("unsafe worker must not run"))
    if damage == "leased":
        with file_lock(child / "build/recovery/driver.lock"):
            with pytest.raises(TimeoutError): implementation.campaign(root, config, resume=directory, execute=choose)
    else:
        with pytest.raises((ValueError, TimeoutError)):
            implementation.campaign(root, config, resume=directory, execute=choose)
    assert working_state(child) == before
    if damage == "quarantined": assert (slot / ACTIVE).exists()


@pytest.mark.parametrize("damage", ["run", "profile", "job_path", "branch", "task", "extra", "symlink"])
def test_changed_resume_choices_cannot_redirect_work(configured_campaign, monkeypatch, damage):
    root, config, directory, child = stopped(configured_campaign)
    path = directory / "batch-0-workers.json"
    value = json.loads(path.read_text())
    if damage == "run":
        path = directory / "run.json"
        value = json.loads(path.read_text())
        value["root"] = str(root.parent)
    if damage == "profile":
        path = root / "recovery/worker.json"
        value = json.loads(path.read_text())
        value["command"]["timeout"] += 1
    if damage == "job_path": value["plan"]["jobs"][0]["root"] = str(root)
    if damage == "branch": value["branches"][0] = "work"
    if damage == "task": value["plan"]["jobs"][0]["task"] = "../owner"
    if damage == "extra": value["accepted"] = True
    path.write_text(json.dumps(value))
    if damage == "profile":
        git(root, "add", "recovery/worker.json")
        git(root, "commit", "-m", "Change profile without resumption authority")
    if damage == "symlink":
        target = directory / "saved.json"
        path.rename(target)
        path.symlink_to(target.name)
    before = working_state(child)
    with pytest.raises(ValueError): implementation.campaign(root, config, resume=directory, execute=choose)
    assert working_state(child) == before


def test_retirement_does_not_hide_new_wip_when_cleanup_is_disabled(configured_campaign, monkeypatch):
    root, config = configured_campaign
    config["publication"]["cleanup"] = False
    root, config, directory, child = stopped(configured_campaign)
    refuse_drive(monkeypatch)
    implementation.campaign(root, config, resume=directory, execute=choose)
    path = child / "code/model.h"
    path.write_text(path.read_text() + "// late work\n")
    with pytest.raises(ValueError, match="dirty"):
        implementation.campaign(root, config, resume=directory, execute=choose)
    assert "late work" in path.read_text()


def test_resume_finishes_interrupted_branch_cleanup_without_replaying_checks(configured_campaign, monkeypatch):
    root, config, directory, child = stopped(configured_campaign)
    refuse_drive(monkeypatch)
    original = implementation._remove_integrated
    def interrupted(root, child, branch, directory):
        git(root, "worktree", "remove", str(child))
        raise Interrupted("removed clean checkout but retained merged branch")
    monkeypatch.setattr(implementation, "_remove_integrated", interrupted)
    with pytest.raises(Interrupted): implementation.campaign(root, config, resume=directory, execute=choose)
    assert not child.exists() and git(root, "branch", "--list", "recovery/campaign-*")
    monkeypatch.setattr(implementation, "_remove_integrated", original)
    monkeypatch.setattr(implementation, "drive", lambda *args, **kwargs: pytest.fail("retained history is not another worker run"))
    report = implementation.campaign(root, config, resume=directory, execute=choose)
    assert not report["batches"][0]["integrations"] and not report["full_project_accepted"]
    assert not git(root, "branch", "--list", "recovery/campaign-*")


@pytest.mark.skipif(sys.platform != "linux", reason="actual supervisor death and descendant liveness checks")
def test_dead_supervisor_does_not_authorize_restarting_its_still_live_worker(configured_campaign, monkeypatch):
    root, config = configured_campaign
    ready, release = root / "build/ready", root / "build/release"
    planner = json.loads((root / "recovery/planner.json").read_text())
    planner["command"] = {"argv": [sys.executable, "-c",
        "import json,sys; p=json.load(sys.stdin); print(json.dumps(dict(snapshot=p['snapshot'],action='resume',"
        "task_id='owner',group='',intent='',editable_files=[],evidence=['test-owned source task'])))"], "timeout": 5}
    (root / "recovery/planner.json").write_text(json.dumps(planner))
    driver = json.loads((root / "recovery/driver.json").read_text())
    code = ("import json,os,sys,time; from pathlib import Path; json.load(sys.stdin); "
            f"Path({str(ready)!r}).write_text(str(os.getpid()));\n"
            f"while not Path({str(release)!r}).exists(): time.sleep(.01)\nprint('{{}}')")
    driver["context"] = {"argv": [sys.executable, "-c", code], "timeout": 20}
    (root / "recovery/driver.json").write_text(json.dumps(driver))
    (root / "recovery/campaign.json").write_text(json.dumps(config))
    git(root, "add", "recovery")
    git(root, "commit", "-m", "Native supervisor-death control")
    output, errors = root / "build/supervisor.out", root / "build/supervisor.err"
    worker_pid = None
    with output.open("wb") as stdout, errors.open("wb") as stderr:
        supervisor = subprocess.Popen([sys.executable, "-I", "-m", "match_recovery", "campaign", "--root", str(root),
                                       "--config", "recovery/campaign.json"], stdout=stdout, stderr=stderr,
                                      start_new_session=True)
        try:
            deadline = time.monotonic() + 10
            while not ready.exists() and supervisor.poll() is None and time.monotonic() < deadline:
                time.sleep(.02)
            assert ready.exists(), errors.read_text()
            worker_pid, = [event["pid"] for line in errors.read_text().splitlines()
                           if (event := json.loads(line)).get("event") == "started"]
            supervisor.kill()
            assert supervisor.wait(timeout=5) == -signal.SIGKILL
            directory, = (root / "build/recovery/campaigns").iterdir()
            os.kill(worker_pid, 0)  # The assigned worker is actually still alive.
            with pytest.raises(TimeoutError):
                implementation.campaign(root, config, resume=directory, execute=choose)
            release.write_text("let the controlled worker terminate normally")
            _assert_stopped(worker_pid)
            assert not (root / "build/compiler-slot" / ACTIVE).exists()
            refuse_drive(monkeypatch)
            result = implementation.campaign(root, config, resume=directory, execute=choose)
            assert result["stop_reason"] == "planning_defer" and not result["full_project_accepted"]
            assert git(root, "worktree", "list", "--porcelain").count("worktree ") == 1
        finally:
            release.write_text("release only test-owned command")
            if supervisor.poll() is None:
                supervisor.kill()
            supervisor.wait(timeout=5)
            if worker_pid is not None:
                try:
                    os.kill(worker_pid, signal.SIGTERM)
                except ProcessLookupError:
                    pass
                _assert_stopped(worker_pid)
