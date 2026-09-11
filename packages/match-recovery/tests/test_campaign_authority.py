"""Delegation transitions preserve assignments; they never grant acceptance."""

from copy import deepcopy
import json
import os
import sys

import pytest

from match_recovery.campaign_choices import coordinator_authority, load_run, manifest, worker_choice
from match_recovery.model import digest
from match_recovery.records import publish_json
from test_preservation import project
from test_task_source import git


@pytest.fixture
def authority(project):
    root = project
    configuration = {"author": "coordinator", "coordinator": "recovery/coordinator.json",
                     "driver": "recovery/driver.json", "worker": "recovery/worker.json",
                     "reviewer": "recovery/reviewer.json", "checkpoint": "recovery/checkpoint.json",
                     "resources": "recovery/resources.json", "iterations": 1, "max_parallel": 2,
                     "timeout": 20, "evidence": ["Bounded campaign"], "destination_branch": "refs/heads/work"}
    coordinator = {"schema_version": 1, "author": "coordinator",
                   "command": {"argv": [sys.executable, "-c", "pass"], "timeout": 10},
                   "subjects": [], "editable_files": [], "max_decisions": 1,
                   "evidence": ["Original delegation"], "seed_bindings": {"image": "{env:TEST_IMAGE}"}}
    profiles = {"coordinator": coordinator, "worker": {"author": "worker"}}
    directory = root / "build/recovery/campaigns/run-test"
    head = git(root, "rev-parse", "HEAD")
    run = manifest(root, configuration, profiles, head)
    batch = worker_choice(directory, 0, head, ["owner"], configuration)
    publish_json(directory / "run.json", run)
    publish_json(directory / "batch-0-workers.json", batch)
    current = deepcopy(profiles)
    current["coordinator"]["editable_files"] = ["code/caller.cc"]
    token = {"schema_version": 1, "author": "coordinator", "run_sha256": digest(run),
             "batch_sha256": digest(batch), "previous_profiles_sha256": digest(profiles),
             "previous_coordinator": deepcopy(coordinator), "coordinator": deepcopy(current["coordinator"]),
             "destination_revision": head, "evidence": ["Authorize existing-source follow-up"]}
    return root, directory, configuration, profiles, current, token


def test_preview_publish_and_retry_do_not_rewrite_original_choices(authority):
    root, directory, config, original, current, token = authority
    before = {path.name: path.read_bytes() for path in directory.iterdir()}
    assert coordinator_authority(root, directory, config, current) == digest(original)
    with pytest.raises(ValueError, match="profiles"):
        load_run(root, directory.parent, directory, config, current)
    assert load_run(root, directory.parent, directory, config, current, token) == directory
    assert {path.name: path.read_bytes() for path in directory.iterdir()} == before
    assert coordinator_authority(root, directory, config, current, token, publish=True) == digest(current)
    path = directory / "coordinator-authorization-1.json"
    stamp = path.stat().st_mtime_ns
    git(root, "commit", "--allow-empty", "-m", "Destination advanced")
    assert coordinator_authority(root, directory, config, current, token, publish=True) == digest(current)
    assert path.stat().st_mtime_ns == stamp
    assert load_run(root, directory.parent, directory, config, current) == directory
    for name, data in before.items():
        assert (directory / name).read_bytes() == data


def test_chain_applies_to_remainder_and_binds_each_historical_batch(authority):
    root, directory, config, original, current, token = authority
    coordinator_authority(root, directory, config, current, token, publish=True)
    git(root, "commit", "--allow-empty", "-m", "Next assigned batch")
    batch = worker_choice(directory, 1, git(root, "rev-parse", "HEAD"), ["owner"], config)
    publish_json(directory / "batch-1-workers.json", batch)
    assert load_run(root, directory.parent, directory, config, current, token) == directory
    newer = deepcopy(current)
    newer["coordinator"]["max_decisions"] = 2
    next_token = {**token, "previous_profiles_sha256": digest(current),
                  "previous_coordinator": current["coordinator"], "coordinator": newer["coordinator"],
                  "batch_sha256": digest(batch), "destination_revision": git(root, "rev-parse", "HEAD")}
    assert coordinator_authority(root, directory, config, newer, next_token, publish=True) == digest(newer)
    assert load_run(root, directory.parent, directory, config, newer) == directory
    assert len(list(directory.glob("coordinator-authorization-*.json"))) == 2
    with pytest.raises(ValueError):
        coordinator_authority(root, directory, config, newer, token, publish=True)


@pytest.mark.parametrize("damage", ["extra", "schema", "author", "run", "batch", "previous_digest",
                                    "previous_coordinator", "noop", "command", "seed", "policy_author",
                                    "scope", "budget", "evidence", "profile", "configured", "configuration"])
def test_invalid_grants_never_publish(authority, damage):
    root, directory, config, original, current, token = authority
    if damage == "extra": token["accepted"] = True
    if damage == "schema": token["schema_version"] = True
    if damage == "author": token["author"] = "worker"
    if damage == "run": token["run_sha256"] = "a" * 64
    if damage == "batch": token["batch_sha256"] = "a" * 64
    if damage == "previous_digest": token["previous_profiles_sha256"] = "a" * 64
    if damage == "previous_coordinator": token["previous_coordinator"]["max_decisions"] = 3
    if damage == "noop": token["coordinator"] = deepcopy(token["previous_coordinator"])
    if damage == "command": token["coordinator"]["command"]["timeout"] += 1
    if damage == "seed": token["coordinator"]["seed_bindings"]["image"] = "different-image"
    if damage == "policy_author": token["coordinator"]["author"] = "worker"
    if damage == "scope": token["coordinator"]["editable_files"] = ["../outside"]
    if damage == "budget": token["coordinator"]["max_decisions"] = 101
    if damage == "evidence": token["evidence"] = []
    if damage == "profile": current["worker"]["author"] = "other-worker"
    if damage == "configured": current["coordinator"]["max_decisions"] = 3
    if damage == "configuration": config["iterations"] += 1
    with pytest.raises(ValueError):
        coordinator_authority(root, directory, config, current, token, publish=True)
    assert not list(directory.glob("coordinator-authorization-*.json"))


@pytest.mark.parametrize("damage", ["head", "latest_batch", "no_batch", "no_coordinator", "missing_task"])
def test_new_authority_requires_live_assignment_and_current_revision(authority, damage):
    root, directory, config, original, current, token = authority
    if damage == "head": git(root, "commit", "--allow-empty", "-m", "Stale grant")
    if damage == "latest_batch":
        publish_json(directory / "batch-1-workers.json", worker_choice(directory, 1, token["destination_revision"], ["owner"], config))
    if damage == "no_batch": (directory / "batch-0-workers.json").unlink()
    if damage == "no_coordinator": del current["coordinator"]
    if damage == "missing_task": (root / "recovery/tasks/owner.json").unlink()
    with pytest.raises((ValueError, FileNotFoundError)):
        coordinator_authority(root, directory, config, current, token, publish=True)
    assert not list(directory.glob("coordinator-authorization-*.json"))


@pytest.mark.parametrize("damage", ["gap", "zero", "leading_zero", "malformed", "symlink", "hardlink",
                                    "record", "run", "run_schema", "batch", "other_profile"])
def test_saved_chain_is_revalidated_not_replayed(authority, damage):
    root, directory, config, original, current, token = authority
    coordinator_authority(root, directory, config, current, token, publish=True)
    path = directory / "coordinator-authorization-1.json"
    if damage in ("gap", "zero", "leading_zero", "malformed"):
        number = {"gap": "2", "zero": "0", "leading_zero": "01", "malformed": "next"}[damage]
        path.rename(directory / f"coordinator-authorization-{number}.json")
    if damage == "symlink":
        target = directory / "saved-token.json"
        path.rename(target)
        path.symlink_to(target.name)
    if damage == "hardlink": os.link(path, directory / "saved-token.json")
    if damage == "record":
        token["previous_coordinator"]["max_decisions"] = 4
        path.write_text(json.dumps(token))
    if damage == "run":
        value = json.loads((directory / "run.json").read_text())
        value["root"] = str(root.parent)
        (directory / "run.json").write_text(json.dumps(value))
    if damage == "run_schema":
        value = json.loads((directory / "run.json").read_text())
        value["schema_version"] = True
        (directory / "run.json").write_text(json.dumps(value))
    if damage == "batch":
        value = json.loads((directory / "batch-0-workers.json").read_text())
        value["plan"]["jobs"][0]["task"] = "somewhere-else"
        (directory / "batch-0-workers.json").write_text(json.dumps(value))
    if damage == "other_profile": current["worker"]["author"] = "different"
    with pytest.raises((ValueError, FileNotFoundError)):
        load_run(root, directory.parent, directory, config, current)


def test_profileless_legacy_campaign_resume_remains_strict(authority):
    root, directory, config, original, current, token = authority
    assert load_run(root, directory.parent, directory, config, original) == directory
    # No stored full-profile document is needed to authenticate the old digest.
    assert "profiles" not in json.loads((directory / "run.json").read_text())
    assert coordinator_authority(root, directory, config, current, token) == digest(current)


@pytest.mark.parametrize("damage", ["unretained", "before_assignment"])
def test_recorded_destination_must_retain_its_assignment_in_current_history(authority, damage):
    root, directory, config, original, current, token = authority
    if damage == "unretained":
        token["destination_revision"] = git(root, "commit-tree", git(root, "rev-parse", "HEAD^{tree}"),
                                            "-p", token["destination_revision"], "-m", "Not retained in destination")
    else:
        git(root, "commit", "--allow-empty", "-m", "Later assignment")
        batch = worker_choice(directory, 1, git(root, "rev-parse", "HEAD"), ["owner"], config)
        publish_json(directory / "batch-1-workers.json", batch)
        token["batch_sha256"] = digest(batch)
    publish_json(directory / "coordinator-authorization-1.json", token)
    with pytest.raises(ValueError, match="ancestry"):
        load_run(root, directory.parent, directory, config, current)


def test_campaign_without_a_coordinator_still_resumes_but_cannot_add_one(authority):
    root, directory, config, original, current, token = authority
    del config["coordinator"]
    del original["coordinator"]
    path = directory / "run.json"
    run = manifest(root, config, original, token["destination_revision"])
    path.write_text(json.dumps(run))
    assert load_run(root, directory.parent, directory, config, original) == directory
    token["run_sha256"] = digest(run)
    token["previous_profiles_sha256"] = digest(original)
    with pytest.raises(ValueError, match="existing coordinator"):
        coordinator_authority(root, directory, config, current, token, publish=True)
