"""Installed runtime upgrades append authority without rewriting preserved work."""

import json
import os
from pathlib import Path

import pytest

from match_recovery import campaign_runtime
from match_recovery.campaign_choices import latest_batch, worker_choice
from match_recovery.campaign_runtime import FILENAME, authority
from match_recovery.model import digest
from match_recovery.records import publish_json, read_json
from test_campaign_runtime import configured, project, run
from test_task_source import git


def next_grant(run, previous, number):
    root, directory, config, profiles, _, state = run
    binding = root / f"build/runtime-{number}.json"
    binding.write_text(json.dumps({"test_runtime": number}))
    state.update(binding=str(binding), fingerprint=f"{number:x}" * 64)
    return {**previous, "schema_version": 2, "previous_authorization_sha256": digest(previous),
            "runtime_binding": str(binding), "runtime_sha256": state["fingerprint"],
            "destination_revision": git(root, "rev-parse", "HEAD"), "profiles_sha256": digest(profiles),
            "batch_sha256": digest(latest_batch(root, directory, config)), "evidence": [f"Select installed runtime {number}"]}


def publish_first(run):
    root, directory, config, profiles, first, _ = run
    assert authority(root, directory, config, profiles, first, publish=True) == first
    return first


def test_three_grants_preserve_original_record_and_only_tail_is_effective(run):
    root, directory, config, profiles, first, state = run
    original_inputs = {name: (directory / name).read_bytes() for name in ("run.json", "batch-0-workers.json")}
    publish_first(run)
    first_bytes = (directory / FILENAME).read_bytes()
    second = next_grant(run, first, 2)
    with pytest.raises(ValueError, match="selected installation"):
        authority(root, directory, config, profiles)
    with pytest.raises(ValueError, match="frozen publication"):
        authority(root, directory, config, profiles, second)
    assert not (directory / "runtime-authorization-2.json").exists()
    assert authority(root, directory, config, profiles, second, publish=True) == second
    second_bytes = (directory / "runtime-authorization-2.json").read_bytes()
    third = next_grant(run, second, 3)
    assert authority(root, directory, config, profiles, third, publish=True) == third
    assert authority(root, directory, config, profiles) == third
    assert (directory / FILENAME).read_bytes() == first_bytes
    assert (directory / "runtime-authorization-2.json").read_bytes() == second_bytes
    assert all((directory / name).read_bytes() == content for name, content in original_inputs.items())
    assert sorted(path.name for path in directory.glob("runtime-authorization*.json")) == [
        "runtime-authorization-2.json", "runtime-authorization-3.json", FILENAME]


def test_retired_binding_files_need_not_exist_or_be_probed(run):
    root, directory, config, profiles, first, _ = run
    publish_first(run)
    second = next_grant(run, first, 2)
    authority(root, directory, config, profiles, second, publish=True)
    Path(first["runtime_binding"]).unlink()
    assert authority(root, directory, config, profiles) == second
    # A superseded path becoming an alias must not redirect an execution probe.
    Path(first["runtime_binding"]).symlink_to(root / "missing-old-installation")
    assert authority(root, directory, config, profiles) == second


def test_exact_tail_retry_survives_progress_but_old_grants_cannot_rollback(run):
    root, directory, config, profiles, first, state = run
    publish_first(run)
    second = next_grant(run, first, 2)
    authority(root, directory, config, profiles, second, publish=True)
    before = {path.name: path.read_bytes() for path in directory.glob("runtime-authorization*.json")}
    git(root, "commit", "--allow-empty", "-m", "Normal campaign progress")
    publish_json(directory / "batch-1-workers.json", worker_choice(
        directory, 1, git(root, "rev-parse", "HEAD"), ["owner"], config))
    assert authority(root, directory, config, profiles, second, publish=True) == second
    with pytest.raises(ValueError):
        authority(root, directory, config, profiles, first, publish=True)
    state.update(binding=first["runtime_binding"], fingerprint=first["runtime_sha256"])
    with pytest.raises(ValueError, match="selected installation"):
        authority(root, directory, config, profiles)
    assert all((directory / name).read_bytes() == content for name, content in before.items())


@pytest.mark.parametrize("damage", ["head", "predecessor", "missing-predecessor", "schema", "batch", "profiles", "run", "no-op"])
def test_new_transition_refusals_never_publish_or_rewrite_existing_grants(run, damage):
    root, directory, config, profiles, first, state = run
    publish_first(run)
    before = (directory / FILENAME).read_bytes()
    second = next_grant(run, first, 2)
    if damage == "head":
        git(root, "commit", "--allow-empty", "-m", "Stale runtime decision")
    elif damage == "predecessor": second["previous_authorization_sha256"] = "f" * 64
    elif damage == "missing-predecessor": del second["previous_authorization_sha256"]
    elif damage == "schema": second["schema_version"] = 1
    elif damage == "no-op":
        second.update(runtime_binding=first["runtime_binding"], runtime_sha256=first["runtime_sha256"])
        state.update(binding=first["runtime_binding"], fingerprint=first["runtime_sha256"])
    else: second[{"batch": "batch_sha256", "profiles": "profiles_sha256", "run": "run_sha256"}[damage]] = "f" * 64
    with pytest.raises(ValueError):
        authority(root, directory, config, profiles, second, publish=True)
    assert (directory / FILENAME).read_bytes() == before
    assert not (directory / "runtime-authorization-2.json").exists()


@pytest.mark.parametrize("damage", ["missing-first", "missing-middle", "truncated-tail", "partial-json", "bad-name",
                                    "index-one", "symlink", "hardlink", "changed-predecessor", "duplicate-key"])
def test_partial_or_modified_chains_cannot_silently_select_an_older_runtime(run, damage):
    root, directory, config, profiles, first, state = run
    publish_first(run)
    second = next_grant(run, first, 2)
    authority(root, directory, config, profiles, second, publish=True)
    third = next_grant(run, second, 3)
    authority(root, directory, config, profiles, third, publish=True)
    middle, tail = directory / "runtime-authorization-2.json", directory / "runtime-authorization-3.json"
    if damage == "missing-first": (directory / FILENAME).unlink()
    elif damage == "missing-middle": middle.unlink()
    elif damage == "truncated-tail": tail.unlink()
    elif damage == "partial-json": middle.write_text('{"schema_version":')
    elif damage in {"bad-name", "index-one"}:
        middle.rename(directory / ("runtime-authorization-02.json" if damage == "bad-name" else "runtime-authorization-1.json"))
    elif damage in {"symlink", "hardlink"}:
        copy = directory / "retained-middle.json"
        if damage == "symlink":
            copy.write_bytes(middle.read_bytes())
            middle.unlink()
            middle.symlink_to(copy)
        else: os.link(middle, copy)
    elif damage == "duplicate-key":
        middle.write_text('{"author":"unexpected",' + middle.read_text().lstrip()[1:])
    else:
        changed = read_json(directory / FILENAME)
        changed["evidence"] = ["Rewritten first grant"]
        (directory / FILENAME).write_text(json.dumps(changed))
    with pytest.raises(ValueError):
        authority(root, directory, config, profiles)
    assert not (directory / "runtime-authorization-4.json").exists()


def test_concurrent_prior_grant_mutation_at_append_is_detected_and_preserved(run, monkeypatch):
    root, directory, config, profiles, first, state = run
    publish_first(run)
    second = next_grant(run, first, 2)
    def changed(path, value):
        publish_json(path, value)
        original = read_json(directory / FILENAME)
        original["evidence"] = ["Concurrent prior-grant mutation"]
        (directory / FILENAME).write_text(json.dumps(original))
    monkeypatch.setattr(campaign_runtime, "publish_json", changed)
    with pytest.raises(ValueError, match="chain changed"):
        authority(root, directory, config, profiles, second, publish=True)
    assert read_json(directory / "runtime-authorization-2.json") == second
    assert read_json(directory / FILENAME)["evidence"] == ["Concurrent prior-grant mutation"]

