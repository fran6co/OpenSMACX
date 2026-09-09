"""Campaign refusal and scheduling controls, not recovery acceptance proofs."""

from copy import deepcopy
import json
from pathlib import Path
import sys

import pytest

import match_recovery.campaign as implementation
from match_recovery.campaign import campaign, _source_key
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.resources import INHERITED
from match_recovery.tasks import TaskStore
from test_preservation import project
from test_task_source import git
from test_worker_worktree import compiler_project


@pytest.fixture
def configured_campaign(compiler_project):
    root, resources = compiler_project
    command = lambda code: {"argv": [sys.executable, "-c", code], "timeout": 5}
    planner = {"schema_version": 1, "author": "coordinator", "command": command("print('{}'); raise SystemExit(3)"),
               "max_entities": 10, "max_editable_files": 4, "commit_decision": True,
               "evidence": ["Authorize synthetic bounded planning"]}
    driver = {"schema_version": 1, "target": "portable", "acceptance_scope": "change",
              "context": command("print('{}')"), "check": command("print('{}'); raise SystemExit(1)"),
              "obligations": command("print('{}')"), "migration_check": command("print('{}'); raise SystemExit(3)")}
    config = {"schema_version": 1, "author": "coordinator", "destination_branch": "refs/heads/work",
              **{key: "recovery/" + key + ".json" for key in ("planner", "driver", "worker", "reviewer", "resources", "checkpoint")},
              "max_batches": 2, "max_parallel": 1, "iterations": 1, "timeout": 10,
              "publication": {"enabled": True, "allow_migration": False, "message": "Synthetic publication", "cleanup": True},
              "evidence": ["Authorize test-owned campaign"]}
    for name, value in (("planner", planner), ("driver", driver),
                        ("worker", {"author": "worker", "command": command("print('{\"discoveries\": []}')")}),
                        ("reviewer", {"author": "reviewer", "command": command("raise SystemExit(3)")}),
                        ("checkpoint", {"author": "coordinator", "message": "Synthetic checkpoint", "allow_migration": False})):
        (root / ("recovery/" + name + ".json")).write_text(json.dumps(value))
    git(root, "config", "user.name", "Campaign control")
    git(root, "config", "user.email", "campaign@example.invalid")
    git(root, "add", "recovery")
    git(root, "commit", "-m", "Authorize synthetic campaign commands")
    return root, config


def choose(command, root, name, payload):
    existing = [row for row in payload["tasks"] if row["eligible"]]
    return 0, {"snapshot": payload["snapshot"], "action": "resume" if existing else "defer",
               "task_id": existing[0]["task"]["task_id"] if existing else "", "group": "", "intent": "",
               "editable_files": [], "evidence": ["Inspect synthetic existing task eligibility"]}


@pytest.mark.parametrize("damage", ["permission", "migration", "cleanup", "parallel", "batches", "iterations", "timeout",
                                    "author", "branch", "evidence", "unknown", "path", "shared_role", "full_acceptance"])
def test_invalid_policy_refuses_before_planner_or_worker_mutation(configured_campaign, damage):
    root, original = configured_campaign
    config = deepcopy(original)
    if damage == "permission": config["publication"]["enabled"] = False
    if damage == "migration": config["publication"]["allow_migration"] = 1
    if damage == "cleanup": config["publication"]["cleanup"] = 1
    if damage == "parallel": config["max_parallel"] = 2
    if damage == "batches": config["max_batches"] = True
    if damage == "iterations": config["iterations"] = 0
    if damage == "timeout": config["timeout"] = 86401
    if damage == "author": config["author"] = "worker"
    if damage == "branch": config["destination_branch"] = "HEAD"
    if damage == "evidence": config["evidence"] = []
    if damage == "unknown": config["force"] = True
    if damage == "path": config["planner"] = "../planner.json"
    if damage == "shared_role": config["reviewer"] = config["worker"]
    if damage == "full_acceptance":
        path = root / config["driver"]
        path.write_text(json.dumps({**json.loads(path.read_text()), "acceptance_scope": "full_project"}))
    before = working_state(root)
    with pytest.raises(ValueError):
        campaign(root, config, execute=lambda *args: pytest.fail("invalid campaign must not invoke agents"))
    assert working_state(root) == before
    assert not (root / "build/recovery/campaigns").exists()


@pytest.mark.parametrize("profile", ["planner", "driver", "worker", "reviewer", "checkpoint", "resources", "coordinator", "migration"])
def test_profile_arrays_are_refused_before_mutation(configured_campaign, profile):
    root, config = configured_campaign
    config[profile] = "recovery/" + profile + ".json"
    (root / config[profile]).write_text("[]")
    before = working_state(root)
    with pytest.raises(ValueError, match="JSON object"):
        campaign(root, config, execute=choose)
    assert working_state(root) == before
    assert not (root / "build/recovery/campaigns").exists()


@pytest.mark.parametrize("damage", ["dirty", "staged", "branch", "operation", "inherited", "leased"])
def test_unready_destination_preserves_all_work(configured_campaign, monkeypatch, damage):
    root, config = configured_campaign
    if damage in ("dirty", "staged"):
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// user work\n")
        if damage == "staged": git(root, "add", "code/owner.cc")
    if damage == "branch": config["destination_branch"] = "refs/heads/somewhere-else"
    if damage == "operation": (root / ".git/MERGE_HEAD").write_text(git(root, "rev-parse", "HEAD"))
    if damage == "inherited": monkeypatch.setenv(INHERITED, "1")
    before = working_state(root)
    if damage == "leased":
        with file_lock(root / "build/recovery/driver.lock"):
            with pytest.raises(TimeoutError): campaign(root, config, execute=choose)
    else:
        with pytest.raises(ValueError): campaign(root, config, execute=choose)
    assert working_state(root) == before
    assert git(root, "worktree", "list", "--porcelain").count("worktree ") == 1


def test_no_checkpoint_defers_snapshot_then_selects_again_without_acceptance(configured_campaign, monkeypatch):
    root, config = configured_campaign
    before = working_state(root)
    # Exercise native setup, locks, selection and cleanup. The dispatch double
    # represents a clean failed/no-op process, not a successful recovery.
    def no_op(root, plan, **kwargs):
        return {"coordinator_changed": False, "batch_completed": False,
                "jobs": [{"timed_out": False, "scope_intact": True, "returncode": 3} for row in plan["jobs"]]}
    monkeypatch.setattr(implementation, "dispatch", no_op)
    report = campaign(root, config, execute=choose)
    assert report["stop_reason"] == "planning_defer" and len(report["batches"]) == 2
    assert report["unresolved_work"][0]["reason"] == "no_worker_checkpoint"
    assert not report["full_project_accepted"] and not report["batches"][0]["integrations"]
    assert working_state(root) == before
    assert git(root, "worktree", "list", "--porcelain").count("worktree ") == 1
    assert TaskStore(root).read("owner").base_revision


@pytest.mark.parametrize("damage", ["timeout", "scope", "source", "destination"])
def test_interrupted_or_uncommitted_work_never_integrates_or_disappears(configured_campaign, monkeypatch, damage):
    root, config = configured_campaign
    def failed(root, plan, **kwargs):
        child = Path(plan["jobs"][0]["root"])
        if damage == "source":
            path = child / "code/owner.cc"
            path.write_text(path.read_text() + "// preserve structural WIP\n")
        return {"coordinator_changed": damage == "destination", "batch_completed": False,
                "jobs": [{"timed_out": damage == "timeout", "scope_intact": damage != "scope", "returncode": 3}]}
    monkeypatch.setattr(implementation, "dispatch", failed)
    monkeypatch.setattr(implementation, "prepare_integration", lambda *args, **kwargs: pytest.fail("must not integrate"))
    report = campaign(root, config, execute=choose)
    assert report["stop_reason"] == ("coordinator_changed" if damage == "destination" else "replanning_required")
    child = Path(report["batches"][0]["workers"][0]["worktree"])
    assert child.is_dir() and not report["full_project_accepted"]
    if damage == "source": assert "structural WIP" in (child / "code/owner.cc").read_text()


def test_source_attempt_key_ignores_git_and_cache_but_changes_with_source(configured_campaign):
    root, config = configured_campaign
    before = _source_key(root)
    git(root, "commit", "--allow-empty", "-m", "Not source progress")
    (root / "build/status.json").write_text('{"accepted":true}')
    assert _source_key(root) == before
    path = root / "code/model.h"
    path.write_text(path.read_text() + "// new ownership evidence\n")
    assert _source_key(root) != before


def test_coordinator_changes_between_stages_are_not_adopted(configured_campaign):
    root, config = configured_campaign
    def progress(event):
        if event["stage"] == "prepare":
            path = root / "code/model.h"
            path.write_text(path.read_text() + "// concurrent change\n")
    with pytest.raises(ValueError, match="destination changed"):
        campaign(root, config, execute=choose, progress=progress)
    assert "concurrent change" in (root / "code/model.h").read_text()
    assert git(root, "worktree", "list", "--porcelain").count("worktree ") == 1


def test_native_failed_driver_is_not_an_integration_receipt(configured_campaign):
    root, config = configured_campaign
    before = working_state(root)
    report = campaign(root, config, execute=choose)
    job, = report["batches"][0]["dispatch"]["jobs"]
    assert job["returncode"] == 3 and not job["timed_out"] and job["scope_intact"]
    assert not report["batches"][0]["integrations"] and report["unresolved_work"]
    assert not report["full_project_accepted"] and working_state(root) == before


def test_installed_campaign_reports_progress_on_stderr_and_does_not_accept_deferral(configured_campaign, capsys):
    from match_recovery.cli import main
    root, config = configured_campaign
    (root / "build/campaign.json").write_text(json.dumps(config))
    before = working_state(root)
    # The configured planner deliberately returns unavailable, not a verdict.
    assert main(["campaign", "--root", str(root), "--config", "build/campaign.json"]) == 3
    output = capsys.readouterr()
    report = json.loads(output.out)
    assert report["stop_reason"] == "planning_unavailable" and not report["full_project_accepted"]
    assert json.loads(output.err.splitlines()[0])["stage"] == "select"
    assert working_state(root) == before
