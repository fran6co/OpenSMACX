"""Run-bound runtime decisions never replace campaign manifests or acceptance."""

from copy import deepcopy
import json
import os

import pytest

from match_recovery import campaign_runtime, framework_runtime
from match_recovery.campaign_choices import coordinator_authority, manifest, worker_choice
from match_recovery.campaign_runtime import FILENAME, authority
from match_recovery.model import Origin, digest
from match_recovery.records import publish_json, read_json
from match_recovery.tasks import Task, TaskStore
from test_source_configuration import configured
from test_task_source import git, project


@pytest.fixture
def run(project, monkeypatch):
    root = project
    base = git(root, "rev-parse", "HEAD")
    TaskStore(root).create(Task("owner", "test", "coordinator", "Preserve ownership", base,
                                (Origin("a" * 64, "custom:offset", 16),), ("code/owner.cc",), ("source",)))
    git(root, "add", "recovery/tasks")
    git(root, "commit", "-m", "Test-owned task")
    revision = git(root, "rev-parse", "HEAD")
    config = {"author": "coordinator", "driver": "recovery/driver.json", "worker": "recovery/worker.json",
              "reviewer": "recovery/reviewer.json", "checkpoint": "recovery/checkpoint.json",
              "coordinator": "recovery/coordinator.json", "resources": "recovery/resources.json",
              "iterations": 2, "max_parallel": 1, "timeout": 30, "evidence": ["Test campaign"]}
    profiles = {"worker": {"author": "worker"}, "coordinator": {
        "schema_version": 1, "author": "coordinator", "subjects": [], "editable_files": [],
        "command": {"argv": ["test-coordinator"], "timeout": 5}, "max_decisions": 1, "evidence": ["original"]}}
    directory = root / "build/recovery/campaigns/run-controlled"
    directory.mkdir(parents=True)
    publish_json(directory / "run.json", manifest(root, config, profiles, revision))
    batch = worker_choice(directory, 0, revision, ["owner"], config)
    publish_json(directory / "batch-0-workers.json", batch)
    binding = root / "build/runtime-binding.json"
    binding.write_text('{"test":"runtime boundary is mocked, not an installation"}')
    state = {"fingerprint": "d" * 64, "binding": str(binding)}
    monkeypatch.setattr(framework_runtime, "fingerprint", lambda: state["fingerprint"])
    monkeypatch.setattr(framework_runtime, "export_binding", lambda: state["binding"])
    token = {"schema_version": 1, "author": "coordinator", "run_sha256": digest(read_json(directory / "run.json")),
             "batch_sha256": digest(batch), "profiles_sha256": digest(profiles), "destination_revision": revision,
             "runtime_binding": str(binding), "runtime_sha256": state["fingerprint"], "evidence": ["Explicit runtime selection"]}
    return root, directory, config, profiles, token, state


def test_legacy_inactive_run_needs_no_runtime_decision_even_before_assignment(run):
    root, directory, config, profiles, token, state = run
    state.update(fingerprint=None, binding=None)
    (directory / "batch-0-workers.json").unlink()
    assert authority(root, directory, config, profiles) is None
    assert not (directory / FILENAME).exists()


def test_explicit_frozen_publication_is_required_then_exact_retry_is_idempotent(run):
    root, directory, config, profiles, token, state = run
    before = {path.name: path.read_bytes() for path in directory.iterdir()}
    with pytest.raises(ValueError, match="explicit runtime authorization"):
        authority(root, directory, config, profiles)
    with pytest.raises(ValueError, match="frozen publication"):
        authority(root, directory, config, profiles, token)
    assert not (directory / FILENAME).exists()
    assert authority(root, directory, config, profiles, token, publish=True) == token
    published = (directory / FILENAME).read_bytes()
    assert authority(root, directory, config, profiles) == token
    assert authority(root, directory, config, profiles, token, publish=True) == token
    assert (directory / FILENAME).read_bytes() == published
    assert all((directory / name).read_bytes() == value for name, value in before.items())
    assert set(directory.iterdir()) == {directory / name for name in [*before, FILENAME]}


@pytest.mark.parametrize("damage", ["inactive", "different-runtime", "different-path", "changed-token"])
def test_published_runtime_cannot_be_removed_or_replaced(run, damage):
    root, directory, config, profiles, token, state = run
    authority(root, directory, config, profiles, token, publish=True)
    before = (directory / FILENAME).read_bytes()
    supplied = None
    if damage == "inactive": state.update(fingerprint=None, binding=None)
    elif damage == "different-runtime": state["fingerprint"] = "e" * 64
    elif damage == "different-path": state["binding"] += "-other"
    else: supplied = {**token, "evidence": ["Replacement selection"]}
    with pytest.raises(ValueError):
        authority(root, directory, config, profiles, supplied, publish=True)
    assert (directory / FILENAME).read_bytes() == before


@pytest.mark.parametrize("field,value", [
    ("schema_version", True), ("schema_version", 2), ("extra", "permission"),
    ("author", "worker"), ("run_sha256", "a" * 64), ("batch_sha256", "b" * 64),
    ("profiles_sha256", "c" * 64), ("runtime_sha256", "e" * 64), ("runtime_binding", "relative.json"),
    ("destination_revision", "HEAD"), ("evidence", []), ("evidence", ["same", "same"]),
    ("evidence", [{}]), ("runtime_sha256", "invalid"),
])
def test_malformed_or_unbound_new_authorization_is_not_published(run, field, value):
    root, directory, config, profiles, token, state = run
    with pytest.raises(ValueError):
        authority(root, directory, config, profiles, {**token, field: value}, publish=True)
    assert not (directory / FILENAME).exists()


def test_new_decision_requires_current_destination_but_published_decision_survives_progress(run):
    root, directory, config, profiles, token, state = run
    git(root, "commit", "--allow-empty", "-m", "Ordinary destination progress")
    with pytest.raises(ValueError, match="latest assignment"):
        authority(root, directory, config, profiles, token, publish=True)
    token["destination_revision"] = git(root, "rev-parse", "HEAD")
    authority(root, directory, config, profiles, token, publish=True)
    git(root, "commit", "--allow-empty", "-m", "Further destination progress")
    later = worker_choice(directory, 1, git(root, "rev-parse", "HEAD"), ["owner"], config)
    publish_json(directory / "batch-1-workers.json", later)
    assert authority(root, directory, config, profiles, token, publish=True) == token


def test_new_decision_requires_latest_assignment(run):
    root, directory, config, profiles, token, state = run
    later = worker_choice(directory, 1, git(root, "rev-parse", "HEAD"), ["owner"], config)
    publish_json(directory / "batch-1-workers.json", later)
    with pytest.raises(ValueError, match="latest assignment"):
        authority(root, directory, config, profiles, token, publish=True)
    assert not (directory / FILENAME).exists()


def test_runtime_grant_survives_separate_valid_coordinator_scope_transition(run):
    root, directory, config, profiles, token, state = run
    authority(root, directory, config, profiles, token, publish=True)
    updated = deepcopy(profiles)
    updated["coordinator"]["max_decisions"] = 2
    updated["coordinator"]["evidence"] = ["Bounded additional coordination"]
    decision = {"schema_version": 1, "author": "coordinator", "run_sha256": token["run_sha256"],
                "batch_sha256": token["batch_sha256"], "previous_profiles_sha256": digest(profiles),
                "previous_coordinator": profiles["coordinator"], "coordinator": updated["coordinator"],
                "destination_revision": token["destination_revision"], "evidence": ["Separate coordinator delegation"]}
    coordinator_authority(root, directory, config, updated, decision, publish=True)
    assert authority(root, directory, config, updated) == token
    updated["worker"]["author"] = "changed-worker"
    with pytest.raises(ValueError):
        authority(root, directory, config, updated)


@pytest.mark.parametrize("damage", ["symlink", "hardlink", "duplicate-json-key", "run-alias"])
def test_saved_authority_and_run_paths_require_unaliased_strict_artifacts(run, damage):
    root, directory, config, profiles, token, state = run
    authority(root, directory, config, profiles, token, publish=True)
    path = directory / FILENAME
    if damage == "duplicate-json-key":
        path.write_text('{"author":"unexpected",' + path.read_text().lstrip()[1:])
    elif damage == "run-alias":
        alias = directory.parent / "alias"
        alias.symlink_to(directory, target_is_directory=True)
        directory = alias
    else:
        other = directory / "retained-copy.json"
        if damage == "symlink":
            other.write_bytes(path.read_bytes())
            path.unlink()
            path.symlink_to(other)
        else:
            os.link(path, other)
    with pytest.raises(ValueError):
        authority(root, directory, config, profiles)


def test_runtime_change_during_publication_refuses_but_preserves_published_decision(run, monkeypatch):
    root, directory, config, profiles, token, state = run
    def changed(path, value):
        publish_json(path, value)
        state["fingerprint"] = "e" * 64
    monkeypatch.setattr(campaign_runtime, "publish_json", changed)
    with pytest.raises(ValueError, match="runtime changed"):
        authority(root, directory, config, profiles, token, publish=True)
    assert read_json(directory / FILENAME) == token

