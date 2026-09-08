"""Local publication controls; no model or application completeness claim."""

import os
import subprocess

import pytest

from match_recovery import checkpoints
from match_recovery.driver import drive, working_state
from match_recovery.reviews import ReviewStore
from match_recovery.tasks import TaskStore
from test_migration_driver import CONFIGURED, WORKER, REVIEWER, project, setup


PERMISSION = {"author": "coordinator", "message": "Recover fixture ownership", "allow_migration": False}


def git(root, *args):
    return subprocess.run(["git", *args], cwd=root, check=True, capture_output=True).stdout


def run(project, *, restored=True, permission=PERMISSION, damage=None):
    migration, execute, calls = setup(project, restored=restored, damage=damage)
    report = drive(project[0], project[1].task_id, {**CONFIGURED, "acceptance_scope": "change"},
                   WORKER, iterations=1, reviewer_configuration=REVIEWER,
                   migration_configuration=migration, checkpoint_configuration=permission, execute=execute)
    return report, calls


def test_driver_commits_only_freshly_reviewed_source_and_decisions(project):
    root, task = project
    parent = git(root, "rev-parse", "HEAD").decode().strip()
    original_task = (root / "recovery/tasks/fixture.json").read_bytes()
    (root / "src/f.cpp").write_text("genuine C++ fixture\n")
    report, calls = run(project)
    commit = report["checkpoint"]
    assert report["stop_reason"] == "change_accepted" and not report["ready_for_collection"]
    assert commit["published"] and commit["parent"] == parent
    assert git(root, "rev-parse", "HEAD").decode().strip() == commit["commit"]
    assert git(root, "show", "HEAD:src/f.cpp") == b"genuine C++ fixture\n"
    assert git(root, "log", "-1", "--format=%s").decode().strip() == PERMISSION["message"]
    assert not git(root, "status", "--porcelain")
    assert "src/f.cpp" in commit["files"] and len(commit["files"]) == 2
    assert (root / "recovery/tasks/fixture.json").read_bytes() == original_task
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
    assert calls.count("migration") == 1 and "acceptance" not in calls
    review = ReviewStore(root, task.task_id).decisions()[-1]
    assert review["snapshot"] == report["review"]["snapshot"]


def test_no_authorization_never_commits(project):
    before = git(project[0], "rev-parse", "HEAD")
    report, _ = run(project, permission=None)
    assert "checkpoint" not in report
    assert git(project[0], "rev-parse", "HEAD") == before


@pytest.mark.parametrize("allowed", [False, True])
def test_regression_publication_needs_separate_permission(project, allowed):
    before = git(project[0], "rev-parse", "HEAD")
    report, _ = run(project, restored=False, permission={**PERMISSION, "allow_migration": allowed})
    assert report["stop_reason"] == "migration_admitted" and report["migration"]["tracked"]
    assert report["checkpoint"]["published"] == allowed
    assert (git(project[0], "rev-parse", "HEAD") != before) == allowed
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("field,value", [("author", "worker"), ("author", "someone_else"),
                                         ("allow_migration", 1), ("message", "")])
def test_invalid_authorization_refuses_before_adapters(project, field, value):
    before = working_state(project[0])
    with pytest.raises(ValueError, match="checkpoint"):
        run(project, permission={**PERMISSION, field: value})
    assert working_state(project[0]) == before


@pytest.mark.parametrize("damage", ["staged", "intent_to_add", "unrelated", "merge", "cherry_pick", "rebase"])
def test_conflicts_and_unrelated_work_are_preserved(project, damage):
    root, _ = project
    if damage == "staged":
        (root / "src/f.cpp").write_text("user staged edit\n")
        git(root, "add", "src/f.cpp")
    elif damage == "intent_to_add":
        (root / "src/new.cpp").write_text("user staged intent\n")
        git(root, "add", "--intent-to-add", "src/new.cpp")
    elif damage == "unrelated":
        (root / "unrelated.txt").write_text("user notes\n")
    elif damage == "rebase":
        (root / ".git/rebase-merge").mkdir()
    else:
        (root / ".git" / ("MERGE_HEAD" if damage == "merge" else "CHERRY_PICK_HEAD")).write_bytes(git(root, "rev-parse", "HEAD"))
    before = working_state(root)
    with pytest.raises(ValueError):
        run(project)
    assert working_state(root) == before


@pytest.mark.parametrize("result", ["fail", "unknown"])
def test_nonpassing_verification_never_publishes(project, result):
    parent = git(project[0], "rev-parse", "HEAD")
    report, _ = run(project, damage=lambda report: report.update(outside_scope=result))
    assert "checkpoint" not in report and not report.get("ready_for_checkpoint")
    assert git(project[0], "rev-parse", "HEAD") == parent


@pytest.mark.parametrize("damage", ["source", "mode", "symlink", "index", "head", "branch", "untracked"])
def test_publication_refuses_changed_verified_state_without_discarding_it(project, damage):
    root, _ = project
    expected, branch = working_state(root), checkpoints.preflight(root)
    path = root / "src/f.cpp"
    if damage == "source":
        path.write_text("racing writer\n")
    elif damage == "mode":
        path.chmod(0o755)
    elif damage == "symlink":
        path.unlink()
        path.symlink_to("other.cpp")
    elif damage == "index":
        path.write_text("staged racing writer\n")
        git(root, "add", "src/f.cpp")
    elif damage == "head":
        git(root, "commit", "--allow-empty", "-m", "other coordinator")
    elif damage == "branch":
        git(root, "switch", "-c", "other")
    else:
        (root / "user-notes").write_text("preserve\n")
    changed = working_state(root)
    with pytest.raises(ValueError, match="state changed"):
        checkpoints.publish(root, expected, {"src/f.cpp"}, branch, "not published")
    assert working_state(root) == changed


@pytest.mark.parametrize("kind", ["new", "delete", "executable", "symlink", "crlf"])
def test_raw_verified_files_roundtrip_without_filters(project, kind):
    root, _ = project
    path = root / ("src/new file.cpp" if kind == "new" else "src/f.cpp")
    if kind == "delete":
        path.unlink()
    elif kind == "symlink":
        path.unlink()
        path.symlink_to("other.cpp")
    elif kind == "executable":
        git(root, "config", "core.fileMode", "false")
        path.chmod(0o755)
    else:
        path.write_bytes(b"changed\r\n" if kind == "crlf" else b"new\n")
    if kind == "crlf":
        git(root, "config", "core.autocrlf", "true")
    expected = working_state(root)
    report = checkpoints.publish(root, expected, {path.relative_to(root).as_posix()}, checkpoints.preflight(root), "raw source")
    assert report["published"]
    if kind == "delete":
        assert not git(root, "ls-tree", "HEAD", "src/f.cpp")
    else:
        assert git(root, "show", "HEAD:" + path.relative_to(root).as_posix()) == (
            os.fsencode(os.readlink(path)) if kind == "symlink" else path.read_bytes())
    if kind == "executable":
        assert git(root, "ls-tree", "HEAD", "src/f.cpp").startswith(b"100755")


def test_unchanged_tree_does_not_make_empty_commit(project):
    root, _ = project
    expected = working_state(root)
    assert checkpoints.publish(root, expected, set(), checkpoints.preflight(root), "empty") == {
        "published": False, "reason": "no_changes"}
    assert working_state(root) == expected


@pytest.mark.parametrize("policy", ["text", "text=auto", "eol=crlf", "autocrlf", "input"])
@pytest.mark.parametrize("publish_source", [False, True])
def test_normalized_outside_files_keep_both_committed_and_checkout_bytes(project, policy, publish_source):
    root, _ = project
    notes = root / "notes.csv"
    notes.write_bytes(b"name,value\noriginal,1\n")
    if policy in ("autocrlf", "input"):
        git(root, "config", "core.autocrlf", "true" if policy == "autocrlf" else "input")
    else:
        (root / ".gitattributes").write_text("*.csv " + policy + "\n")
    git(root, "add", ".")
    git(root, "commit", "-m", "Declare text checkout policy")
    original = git(root, "show", "HEAD:notes.csv")
    notes.write_bytes(original.replace(b"\n", b"\r\n"))
    assert not git(root, "diff", "--name-only", "HEAD")
    if publish_source:
        (root / "src/f.cpp").write_bytes(b"verified source\r\n")
    expected, branch = working_state(root), checkpoints.preflight(root)
    report = checkpoints.publish(root, expected, {"src/f.cpp"}, branch, "Preserve unrelated checkout")
    assert report["published"] == publish_source
    assert git(root, "show", "HEAD:notes.csv") == original
    assert notes.read_bytes() == original.replace(b"\n", b"\r\n")
    assert working_state(root)["files"] == expected["files"]
    if publish_source:
        assert report["files"] == ["src/f.cpp"]
        assert git(root, "show", "HEAD:src/f.cpp") == b"verified source\r\n"
    else:
        assert report == {"published": False, "reason": "no_changes"}
        assert working_state(root) == expected


@pytest.mark.parametrize("damage", ["content", "mode", "binary", "no_policy", "unset_text", "filter", "encoding", "ident", "new_policy"])
def test_normalization_cannot_hide_other_outside_changes_or_transforms(project, damage):
    root, _ = project
    notes = root / "notes.csv"
    attributes = root / ".gitattributes"
    attributes.write_text("*.csv text=auto\n")
    notes.write_bytes(b"original\n")
    if damage == "no_policy":
        attributes.write_text("")
    elif damage == "unset_text":
        attributes.write_text("*.csv -text\n")
    elif damage == "binary":
        notes.write_bytes(b"original\x00\n")
    elif damage in ("filter", "encoding", "ident"):
        attr = {"filter": "filter=unused", "encoding": "working-tree-encoding=UTF-8", "ident": "ident"}[damage]
        attributes.write_text("*.csv text=auto " + attr + "\n")
    elif damage == "new_policy":
        attributes.write_text("")
    git(root, "add", ".")
    git(root, "commit", "-m", "Outside file baseline")
    notes.write_bytes(notes.read_bytes().replace(b"\n", b"\r\n"))
    if damage == "content":
        notes.write_bytes(b"changed\r\n")
    elif damage == "mode":
        git(root, "config", "core.fileMode", "false")
        notes.chmod(0o755)
    elif damage == "new_policy":
        attributes.write_text("*.csv text=auto\n")
    if damage == "filter":
        # None of the publication/read-only normalization commands may run it.
        git(root, "config", "filter.unused.clean", "touch filter-ran; cat")
        git(root, "config", "filter.unused.required", "true")
    (root / "src/f.cpp").write_text("verified source\n")
    expected, branch = working_state(root), checkpoints.preflight(root)
    with pytest.raises(ValueError, match="outside authorized"):
        checkpoints.publish(root, expected, {"src/f.cpp", ".gitattributes"}, branch, "refused")
    assert working_state(root) == expected
    assert not (root / "filter-ran").exists()


def test_git_hooks_cannot_change_verified_source(project):
    root, _ = project
    hook = root / ".git/hooks/reference-transaction"
    hook.write_text("#!/bin/sh\nprintf 'hook damage' > src/f.cpp\n")
    hook.chmod(0o755)
    (root / "src/f.cpp").write_text("verified\n")
    report, _ = run(project)
    assert report["checkpoint"]["published"]
    assert (root / "src/f.cpp").read_text() == "verified\n"


def test_existing_index_lock_is_not_removed(project):
    root, _ = project
    (root / "src/f.cpp").write_text("verified\n")
    lock = root / ".git/index.lock"
    lock.write_text("other writer")
    with pytest.raises(FileExistsError):
        run(project)
    assert lock.read_text() == "other writer"


@pytest.mark.parametrize("flag", ["--assume-unchanged", "--skip-worktree"])
def test_special_index_flags_are_preserved_on_refusal(project, flag):
    root, _ = project
    git(root, "update-index", flag, "src/f.cpp")
    before = (root / ".git/index").read_bytes()
    with pytest.raises(ValueError, match="special index"):
        run(project)
    assert (root / ".git/index").read_bytes() == before


def test_failed_branch_compare_and_swap_preserves_source_and_index(project, monkeypatch):
    root, _ = project
    (root / "src/f.cpp").write_text("verified\n")
    expected, branch = working_state(root), checkpoints.preflight(root)
    original = checkpoints._git
    def git_failure(root, *args, **kwargs):
        if "update-ref" in args:
            raise subprocess.CalledProcessError(1, args)
        return original(root, *args, **kwargs)
    monkeypatch.setattr(checkpoints, "_git", git_failure)
    with pytest.raises(subprocess.CalledProcessError):
        checkpoints.publish(root, expected, {"src/f.cpp"}, branch, "failed publication")
    assert working_state(root) == expected
    assert not (root / ".git/index.lock").exists()


def test_raw_read_race_cannot_publish_bytes_other_than_the_verified_snapshot(project, monkeypatch):
    root, _ = project
    source = root / "src/f.cpp"
    source.write_text("verified\n")
    expected, branch = working_state(root), checkpoints.preflight(root)
    original = checkpoints._content
    def raced(root, name, state):
        if name != "src/f.cpp":
            return original(root, name, state)
        source.write_text("transient unverified content\n")
        try:
            return original(root, name, state)
        finally:
            source.write_text("verified\n")
    monkeypatch.setattr(checkpoints, "_content", raced)
    with pytest.raises(ValueError, match="file changed"):
        checkpoints.publish(root, expected, {"src/f.cpp"}, branch, "refused race")
    assert working_state(root) == expected


def test_interruption_after_ref_update_preserves_source_and_exposes_old_index(project, monkeypatch):
    root, _ = project
    (root / "src/f.cpp").write_text("verified\n")
    expected, branch = working_state(root), checkpoints.preflight(root)
    def interruption(*args):
        raise OSError("simulated index replacement failure")
    monkeypatch.setattr(checkpoints.os, "replace", interruption)
    with pytest.raises(OSError, match="simulated"):
        checkpoints.publish(root, expected, {"src/f.cpp"}, branch, "verified before interruption")
    assert working_state(root)["files"] == expected["files"]
    assert working_state(root)["index"] == expected["index"]
    assert git(root, "rev-parse", "HEAD^") == expected["head"]
    assert git(root, "show", "HEAD:src/f.cpp") == b"verified\n"
    assert not (root / ".git/index.lock").exists()


def test_repeated_run_reverifies_without_an_empty_commit(project):
    first, _ = run(project)
    second, calls = run(project)
    assert first["checkpoint"]["published"]
    assert second["stop_reason"] == "change_accepted"
    assert second["checkpoint"] == {"published": False, "reason": "no_changes"}
    assert calls.count("migration") == 1
    assert git(project[0], "rev-parse", "HEAD").decode().strip() == first["checkpoint"]["commit"]


def test_linked_worktree_uses_its_own_index(project, tmp_path_factory):
    root, _ = project
    linked = tmp_path_factory.mktemp("checkpoint-linked")
    before = working_state(root)
    git(root, "worktree", "add", "-b", "linked", str(linked))
    (linked / "src/f.cpp").write_text("linked verified\n")
    expected = working_state(linked)
    report = checkpoints.publish(linked, expected, {"src/f.cpp"}, checkpoints.preflight(linked), "linked checkpoint")
    assert report["published"] and not git(linked, "status", "--porcelain")
    assert working_state(root) == before
