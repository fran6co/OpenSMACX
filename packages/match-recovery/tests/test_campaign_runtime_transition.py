"""Run-bound runtime changes preserve stopped source, assignments and policy."""

from copy import deepcopy
import json
from pathlib import Path

import pytest

import match_recovery.campaign as native
from match_recovery import framework_runtime as runtime
from match_recovery.campaign_choices import integration_choice, retire, worker_choice
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.model import digest
from match_recovery.records import publish_json, read_json
from match_recovery.tasks import TaskStore
from test_campaign import choose, configured_campaign, compiler_project, project
from test_campaign_resume import stopped
from test_task_source import git


@pytest.fixture
def transition(configured_campaign, monkeypatch):
    root, config = configured_campaign
    config["max_batches"] = 1
    config["publication"]["cleanup"] = False
    root, config, directory, child = stopped((root, config))
    source = child / "code/model.h"
    source.write_text(source.read_text() + "// Keep unfinished structural candidate\n")
    binding = root / "build/runtime-binding.json"
    binding.write_text('{"test": "runtime validation is exercised separately"}\n')
    state = {"fingerprint": "e" * 64, "binding": str(binding)}
    monkeypatch.setattr(runtime, "fingerprint", lambda: state["fingerprint"])
    monkeypatch.setattr(runtime, "export_binding", lambda: state["binding"] if state["fingerprint"] else None)
    token = {"schema_version": 1, "author": config["author"],
             "run_sha256": digest(read_json(directory / "run.json")),
             "batch_sha256": digest(read_json(directory / "batch-0-workers.json")),
             "profiles_sha256": digest(native._policy(root, config)),
             "destination_revision": git(root, "rev-parse", "HEAD"),
             "runtime_binding": str(binding), "runtime_sha256": state["fingerprint"],
             "evidence": ["Explicitly upgrade only the installed native framework for this retained run"]}
    return root, config, directory, child, state, token


def forbid_work(monkeypatch):
    for name in ("prepare_project", "prepare_worker", "drive", "dispatch"):
        monkeypatch.setattr(native, name, lambda *args, **kwargs:
                            pytest.fail("Runtime refusal must happen before preparation or provider execution"))


def resume(transition, **kwargs):
    root, config, directory, _, _, _ = transition
    return native.campaign(root, config, resume=directory, execute=choose, **kwargs)


@pytest.mark.parametrize("damage", ["missing", "run", "batch", "runtime", "profiles", "extra"])
def test_runtime_permission_refuses_before_preparation(transition, monkeypatch, damage):
    root, config, directory, child, state, original = transition
    token = deepcopy(original)
    if damage == "missing":
        token = None
    elif damage == "extra":
        token["upgrade_project_compiler"] = True
    else:
        key = {"run": "run_sha256", "batch": "batch_sha256", "runtime": "runtime_sha256",
               "profiles": "profiles_sha256"}[damage]
        token[key] = "f" * 64
    before = working_state(root), working_state(child)
    forbid_work(monkeypatch)
    with pytest.raises(ValueError):
        resume(transition, runtime_authorization=token)
    assert (working_state(root), working_state(child)) == before
    assert not (directory / "runtime-authorization.json").exists()


def test_runtime_transition_waits_for_stopped_worker_lease(transition, monkeypatch):
    root, config, directory, child, _, token = transition
    before = working_state(child)
    forbid_work(monkeypatch)
    with file_lock(child / "build/recovery/driver.lock", timeout=0):
        with pytest.raises(TimeoutError):
            resume(transition, runtime_authorization=token)
    assert working_state(child) == before
    assert not (directory / "runtime-authorization.json").exists()


def test_runtime_transition_freezes_existing_candidate_lease(transition, monkeypatch):
    root, config, directory, child, _, token = transition
    git(child, "add", "code/model.h")
    git(child, "commit", "-m", "Preserve synthetic unaccepted candidate")
    context = {"author": config["author"], "source_revision": git(child, "rev-parse", "HEAD"),
               "target_revision": git(root, "rev-parse", "HEAD"), "evidence": ["Synthetic pending integration"]}
    choice = integration_choice(directory, 0, "owner", context)
    (directory / "batch-0-integration-owner.json").write_text(json.dumps(choice))
    candidate = Path(choice["worktree"])
    git(root, "worktree", "add", "-b", choice["branch"], str(candidate), context["source_revision"])
    before = working_state(child), working_state(candidate)
    forbid_work(monkeypatch)
    with file_lock(candidate / "build/recovery/driver.lock", timeout=0):
        with pytest.raises(TimeoutError):
            resume(transition, runtime_authorization=token)
    assert (working_state(child), working_state(candidate)) == before
    assert not (directory / "runtime-authorization.json").exists()


def installed_boundary(monkeypatch, transition, *, during_dispatch=None):
    root, config, directory, child, state, token = transition
    calls = []

    def prepare(path):
        assert path == child and read_json(directory / "runtime-authorization.json") == token
        assert runtime.export_binding() == state["binding"]
        calls.append("prepare")
        return {"result": "pass", "published": True}

    def dispatch(current, plan, **kwargs):
        assert current == root and runtime.fingerprint() == token["runtime_sha256"]
        assert runtime.export_binding() == token["runtime_binding"]
        assert plan == read_json(directory / "batch-0-workers.json")["plan"]
        assert [row["root"] for row in plan["jobs"]] == [str(child)]
        calls.append("dispatch")
        if during_dispatch:
            during_dispatch()
        return {"coordinator_changed": False, "batch_completed": False,
                "verification": "not_measured_by_dispatch", "ready_for_collection": False,
                "jobs": [{"task": row["task"], "root": row["root"], "timed_out": False,
                          "scope_intact": True, "returncode": 3} for row in plan["jobs"]]}

    monkeypatch.setattr(native, "prepare_project", prepare)
    monkeypatch.setattr(native, "dispatch", dispatch)
    monkeypatch.setattr(native, "drive", lambda *args, **kwargs:
                        pytest.fail("Bound parallel resume must not duplicate a worker in serial"))
    return calls


def test_bound_resume_preserves_assignments_profiles_baselines_and_cpp_wip(transition, monkeypatch):
    root, config, directory, child, _, token = transition
    before = working_state(root), working_state(child)
    task = TaskStore(child).read("owner")
    choices = {name: (directory / name).read_bytes() for name in ("run.json", "batch-0-workers.json")}
    calls = installed_boundary(monkeypatch, transition)
    for options in ({"runtime_authorization": token}, {}, {"runtime_authorization": token}):
        report = resume(transition, **options)
        assert report["stop_reason"] == "replanning_required" and not report["full_project_accepted"]
        assert report["batches"][0]["dispatch"] is not None
        assert (working_state(root), working_state(child)) == before
        assert TaskStore(child).read("owner") == task
        assert {name: (directory / name).read_bytes() for name in choices} == choices
    assert calls == ["prepare", "dispatch"] * 3
    assert read_json(directory / "runtime-authorization.json") == token


@pytest.mark.parametrize("damage", ["inactive", "different_runtime", "different_binding", "altered_grant"])
def test_preserved_runtime_requires_same_binding_before_repreparation(transition, monkeypatch, damage):
    root, config, directory, child, state, token = transition
    installed_boundary(monkeypatch, transition)
    resume(transition, runtime_authorization=token)
    if damage == "inactive":
        state["fingerprint"] = None
    elif damage == "different_runtime":
        state["fingerprint"] = "f" * 64
    elif damage == "different_binding":
        state["binding"] = str(root / "build/different-binding.json")
    else:
        altered = {**token, "runtime_sha256": "f" * 64}
        (directory / "runtime-authorization.json").write_text(json.dumps(altered))
    before = working_state(root), working_state(child)
    forbid_work(monkeypatch)
    with pytest.raises(ValueError):
        resume(transition)
    assert (working_state(root), working_state(child)) == before


def test_runtime_grant_artifact_is_fresh_after_dispatch_callback(transition, monkeypatch):
    _, _, directory, child, _, token = transition
    before = working_state(child)

    def alter():
        value = read_json(directory / "runtime-authorization.json")
        value["evidence"].append("Concurrent runtime authorization rewrite")
        (directory / "runtime-authorization.json").write_text(json.dumps(value))

    calls = installed_boundary(monkeypatch, transition, during_dispatch=alter)
    with pytest.raises(ValueError):
        resume(transition, runtime_authorization=token)
    assert calls == ["prepare", "dispatch"] and working_state(child) == before
    assert "Concurrent runtime authorization rewrite" in read_json(directory / "runtime-authorization.json")["evidence"]


def test_runtime_authorization_cli_reaches_campaign_without_changing_profiles(transition, monkeypatch, capsys):
    from match_recovery.cli import main

    root, config, directory, _, _, token = transition
    permission = root / "build/runtime-permission.json"
    permission.write_text(json.dumps(token))
    (root / "build/campaign-config.json").write_text(json.dumps(config))

    def called(current, configuration, **kwargs):
        assert current == root and configuration == config
        assert kwargs["runtime_authorization"] == token and kwargs["resume"] == directory
        return {"stop_reason": "replanning_required", "unresolved_work": ["Retained candidate"]}

    monkeypatch.setattr(native, "campaign", called)
    assert main(["campaign", "--root", str(root), "--config", "build/campaign-config.json",
                 "--resume", str(directory), "--runtime-authorization", str(permission)]) == 3
    assert "replanning_required" in capsys.readouterr().out


def test_same_head_branch_rename_during_runtime_publication_is_not_frozen(transition, monkeypatch):
    from match_recovery import campaign_runtime

    root, config, directory, child, _, token = transition
    before = working_state(child)
    original = campaign_runtime.authority

    def rename(*args, **kwargs):
        result = original(*args, **kwargs)
        if kwargs.get("publish"):
            git(child, "branch", "-m", "runtime-concurrent-rename")
            assert working_state(child) == before  # Same HEAD/index/files is insufficient.
        return result

    monkeypatch.setattr(campaign_runtime, "authority", rename)
    forbid_work(monkeypatch)
    with pytest.raises(ValueError):
        resume(transition, runtime_authorization=token)
    assert working_state(child) == before
    assert git(child, "symbolic-ref", "--short", "HEAD") == "runtime-concurrent-rename"
    assert read_json(directory / "runtime-authorization.json") == token  # Preserved decision is not acceptance.


def test_runtime_transition_reuses_earlier_retained_batch_reservation_lease(transition, monkeypatch):
    root, config, directory, older, state, token = transition
    # The first assignment is clean and retired, but intentionally retained.
    (older / "code/model.h").write_bytes((root / "code/model.h").read_bytes())
    revision = git(root, "rev-parse", "HEAD")
    retire(directory, 0, "owner", git(older, "rev-parse", "HEAD"), revision, reason="no_checkpoint")
    latest = worker_choice(directory, 1, revision, ["owner"], config)
    publish_json(directory / "batch-1-workers.json", latest)
    child = Path(latest["plan"]["jobs"][0]["root"])
    git(root, "worktree", "add", "-b", latest["branches"][0], str(child), revision)
    source = child / "code/model.h"
    source.write_text(source.read_text() + "// Keep second assignment WIP\n")
    token = {**token, "batch_sha256": digest(latest)}
    current = root, config, directory, child, state, token
    before = working_state(root), working_state(older), working_state(child)
    calls = []

    def prepare(path):
        assert path == child and read_json(directory / "runtime-authorization.json") == token
        with pytest.raises(TimeoutError):
            with file_lock(older / "build/recovery/driver.lock", timeout=0):
                pytest.fail("Retained historical assignment lost its reservation lease")
        calls.append("prepare")
        return {"result": "pass", "published": True}

    def dispatch(path, plan, **kwargs):
        assert path == root and plan == latest["plan"]
        calls.append("dispatch")
        return {"coordinator_changed": False, "jobs": [{"task": "owner", "root": str(child),
                "scope_intact": True, "timed_out": False, "returncode": 3}]}

    monkeypatch.setattr(native, "prepare_project", prepare)
    monkeypatch.setattr(native, "dispatch", dispatch)
    report = resume(current, runtime_authorization=token)
    assert calls == ["prepare", "dispatch"]
    assert report["stop_reason"] == "replanning_required" and not report["full_project_accepted"]
    assert (working_state(root), working_state(older), working_state(child)) == before
