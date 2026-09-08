"""Unverified isolated candidate creation; no acceptance from a text merge."""

import json
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.driver import working_state
from match_recovery.integration_prepare import prepare_integration
from match_recovery.tasks import TaskStore
from test_preservation import project, check
from test_task_source import git


@pytest.fixture
def inputs(project):
    root = project
    fork = git(root, "rev-parse", "HEAD")
    git(root, "switch", "-c", "incoming")
    path = root / "code/owner.cc"
    path.write_text(path.read_text().replace("return 1", "return 2"))
    git(root, "add", "code/owner.cc")
    git(root, "commit", "-m", "Incoming test-owned recovery")
    source = git(root, "rev-parse", "HEAD")
    git(root, "switch", "-c", "destination", fork)
    path = root / "code/caller.cc"
    path.write_text(path.read_text().replace("return 1", "return 2"))
    git(root, "add", "code/caller.cc")
    git(root, "commit", "-m", "Independent test-owned destination update")
    context = {"author": "coordinator", "source_revision": source, "target_revision": git(root, "rev-parse", "HEAD"),
               "evidence": ["Coordinator selected these two test-owned revisions"]}
    return root, context


def prepare(inputs, **kwargs):
    root, context = inputs
    return prepare_integration(root, "owner", context, worktree=kwargs.pop("worktree", "build/candidate"),
                               branch=kwargs.pop("branch", "candidate"), **kwargs)


def test_preparation_creates_a_separate_raw_checkout_with_both_parents_and_no_acceptance(inputs):
    root, context = inputs
    before = working_state(root)
    result = prepare(inputs)
    candidate = Path(result["worktree"])
    assert result["candidate_created"] and result["stop_reason"] == "candidate_prepared"
    assert result["verification"] == "not_measured" and not result["ready_for_collection"]
    assert git(candidate, "rev-parse", "HEAD^") == context["target_revision"]
    assert git(candidate, "rev-parse", "HEAD^2") == context["source_revision"]
    assert working_state(root) == before
    assert git(root, "rev-parse", "destination") == context["target_revision"]
    assert git(root, "rev-parse", "incoming") == context["source_revision"]
    assert TaskStore(candidate).read("owner") == TaskStore(root).read("owner")
    assert check(candidate, integration=context)["result"] == "pass"
    assert not (candidate / "build/compile_commands.json").exists()  # No borrowed compiler cache.
    message = git(candidate, "show", "-s", "--format=%B", "HEAD")
    assert json.loads(message.split("Integration authorization:\n", 1)[1]) == context


def test_installed_cli_refuses_destination_removal_of_the_task(inputs, tmp_path_factory):
    root, context = inputs
    path = root / "recovery/tasks/owner.json"
    path.unlink()
    git(root, "add", "-u")
    git(root, "commit", "-m", "Destination does not yet have this task")
    context = {**context, "target_revision": git(root, "rev-parse", "HEAD")}
    authorization = tmp_path_factory.mktemp("integration-permission") / "context.json"
    authorization.write_text(json.dumps(context))
    result = subprocess.run([sys.executable, "-m", "match_recovery.cli", "integrate", "prepare", "owner", "--root", str(root),
                             "--context", str(authorization), "--worktree", "build/candidate", "--branch", "candidate"],
                            capture_output=True, timeout=30)
    # A deletion on one side must not silently erase incoming task history,
    # even if Git can text-merge it without a conflict.
    assert result.returncode == 3
    assert not path.exists()


def test_installed_cli_prepares_a_new_incoming_task_on_its_original_destination(inputs, tmp_path_factory):
    root, context = inputs
    task = TaskStore(root).read("owner")
    git(root, "switch", "-c", "first-destination", task.base_revision)
    context = {**context, "target_revision": task.base_revision}
    assert not (root / "recovery/tasks/owner.json").exists()
    authorization = tmp_path_factory.mktemp("integration-first-permission") / "context.json"
    authorization.write_text(json.dumps(context))
    before = working_state(root)
    result = subprocess.run([sys.executable, "-m", "match_recovery.cli", "integrate", "prepare", "owner", "--root", str(root),
                             "--context", str(authorization), "--worktree", "build/candidate", "--branch", "candidate"],
                            capture_output=True, timeout=30)
    assert result.returncode == 0, result.stdout.decode() + result.stderr.decode()
    report = json.loads(result.stdout)
    assert report["candidate_created"] and not report["ready_for_collection"]
    assert TaskStore(Path(report["worktree"])).read("owner") == task
    assert working_state(root) == before


def test_conflicts_return_paths_without_creating_a_worktree_or_moving_inputs(inputs):
    root, context = inputs
    path = root / "code/owner.cc"
    path.write_text(path.read_text().replace("return 1", "return 3"))
    git(root, "add", "code/owner.cc")
    git(root, "commit", "-m", "Conflicting destination spelling")
    context = {**context, "target_revision": git(root, "rev-parse", "HEAD")}
    before = working_state(root)
    result = prepare((root, context))
    assert result["stop_reason"] == "merge_conflicts" and result["conflicts"] == ["code/owner.cc"]
    assert not result["candidate_created"] and not result["ready_for_collection"]
    assert not (root / "build/candidate").exists() and working_state(root) == before
    assert git(root, "branch", "--list", "candidate") == ""


@pytest.mark.parametrize("damage", ["dirty", "staged", "existing_path", "existing_branch", "unignored_path", "git_metadata", "stale_target", "wrong_author"])
def test_refusals_preserve_the_destination_and_existing_work(inputs, damage):
    root, context = inputs
    options = {}
    if damage in ("dirty", "staged"):
        (root / "code/owner.cc").write_text("user work\n")
        if damage == "staged": git(root, "add", "code/owner.cc")
    elif damage == "existing_path":
        (root / "build/candidate").mkdir(parents=True)
        (root / "build/candidate/user.txt").write_text("preserve\n")
    elif damage == "existing_branch": git(root, "branch", "candidate")
    elif damage == "unignored_path": options["worktree"] = "unignored-candidate"
    elif damage == "git_metadata": options["worktree"] = ".git/candidate"
    elif damage == "stale_target": git(root, "commit", "--allow-empty", "-m", "Concurrent destination")
    else: context = {**context, "author": "worker"}
    before = working_state(root)
    branches = git(root, "show-ref")
    with pytest.raises(ValueError):
        prepare((root, context), **options)
    assert working_state(root) == before and git(root, "show-ref") == branches
    if damage == "existing_path": assert (root / "build/candidate/user.txt").read_text() == "preserve\n"


def test_hooks_are_not_executed_and_git_config_is_not_rewritten(inputs):
    root, _ = inputs
    hook = root / ".git/hooks/post-checkout"
    hook.write_text("#!/bin/sh\nexit 87\n")
    hook.chmod(0o755)
    config = (root / ".git/config").read_bytes()
    assert prepare(inputs)["candidate_created"]
    assert (root / ".git/config").read_bytes() == config


def test_active_checkout_filters_are_refused_without_executing_them(inputs):
    root, context = inputs
    (root / ".gitattributes").write_text("code/owner.cc filter=untrusted\n")
    git(root, "add", ".gitattributes")
    git(root, "commit", "-m", "Active checkout filter requires separate resolution")
    context = {**context, "target_revision": git(root, "rev-parse", "HEAD")}
    for field in ("clean", "smudge", "process"):
        git(root, "config", "filter.untrusted." + field, "false")
    git(root, "config", "filter.untrusted.required", "true")
    git(root, "config", "merge.renormalize", "true")
    before = working_state(root)
    with pytest.raises(ValueError, match="active checkout filters"):
        prepare((root, context))
    assert working_state(root) == before and not (root / "build/candidate").exists()


def test_unused_global_filter_configuration_does_not_block_raw_preparation(inputs):
    root, _ = inputs
    git(root, "config", "filter.unused.process", "false")
    git(root, "config", "filter.unused.required", "true")
    assert prepare(inputs)["candidate_created"]


def test_external_merge_drivers_are_not_invoked(inputs):
    root, _ = inputs
    git(root, "config", "merge.custom.driver", "false")
    before = working_state(root)
    with pytest.raises(ValueError, match="external merge drivers"):
        prepare(inputs)
    assert working_state(root) == before and not (root / "build/candidate").exists()


@pytest.mark.parametrize("target", ["owner.cc", "../../outside", "loop-link"])
def test_only_internal_file_symlinks_are_materialized(inputs, target):
    root, context = inputs
    (root / "code/loop-link").symlink_to(target)
    git(root, "add", "code/loop-link")
    git(root, "commit", "-m", "Test-owned symlink input")
    context = {**context, "target_revision": git(root, "rev-parse", "HEAD")}
    if target == "owner.cc":
        result = prepare((root, context))
        assert (Path(result["worktree"]) / "code/loop-link").is_symlink()
        assert (Path(result["worktree"]) / "code/loop-link").read_bytes() == (root / "code/owner.cc").read_bytes().replace(b"return 1", b"return 2")
    else:
        with pytest.raises(ValueError):
            prepare((root, context))
        assert not (root / "build/candidate").exists()
