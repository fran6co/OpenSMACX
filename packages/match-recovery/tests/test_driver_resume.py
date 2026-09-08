"""Restart from source and append-only decisions, never cached acceptance."""

from dataclasses import replace
import json

import pytest

from match_recovery.driver import drive, working_state
from match_recovery.model import digest
from match_recovery.reviews import ReviewStore
from match_recovery.tasks import TaskStore
from test_driver import WORKER, project, run
from test_reviews import GATED, REVIEWER, execution
from test_task_source import git


def discovery(root, task):
    store = TaskStore(root)
    store.discover(task.task_id, request_id="finding", author="worker", hypothesis="Member belongs to this owner",
                   affects=[task.subjects[0].entity_id], evidence=["source:src/f.cpp"], expected_revision=0)
    return store.journal.stream_directory("task:" + task.task_id) / "00000001.json"


def test_pending_discovery_resumes_without_a_git_checkpoint(project):
    root, task = project
    head = git(root, "rev-parse", "HEAD")
    path = discovery(root, task)
    state = working_state(root)
    report = run(project, lambda *_: pytest.fail("unresolved discovery must stop the worker"))
    assert report["stop_reason"] == "evaluate_structural_discoveries"
    assert report["worker_rounds"] == 0 and not report["ready_for_collection"]
    assert working_state(root) == state and path.is_file()
    assert git(root, "rev-parse", "HEAD") == head


@pytest.mark.parametrize("stage", [False, True])
def test_resolved_append_resumes_scoped_source_with_same_baseline(project, stage):
    root, task = project
    discovery(root, task)
    git(root, "add", "recovery/history")
    git(root, "commit", "-m", "Existing decision checkpoint")
    store = TaskStore(root)
    store.resolve(task.task_id, discovery_id="finding", request_id="resolved", author=task.author,
                  disposition="incorporated", source_evidence=["source:src/f.cpp"], expected_revision=1)
    (root / "src/f.cpp").write_text("partial recovery\n")
    if stage:
        git(root, "add", "recovery/history")
    before = working_state(root)
    def worker(root, payload):
        assert payload["task"]["base_revision"] == task.base_revision
        assert (root / "src/f.cpp").read_text() == "partial recovery\n"
        (root / "src/f.cpp").write_text("finished\n")
        return 0, {"discoveries": []}
    report = run(project, worker)
    after = working_state(root)
    assert after["head"] == before["head"] and after["index"] == before["index"]
    assert report["stop_reason"] == "independent_review_required"
    assert store.read(task.task_id) == task


@pytest.mark.parametrize("source_changed", [False, True])
def test_uncommitted_review_reuses_only_fresh_decision_and_always_rechecks_acceptance(project, source_changed):
    root, task = project
    execute, calls = execution(project)
    first = drive(root, task.task_id, GATED, WORKER, iterations=1,
                  reviewer_configuration=REVIEWER, execute=execute)
    assert first["ready_for_collection"]
    if source_changed:
        (root / "src/f.cpp").write_text("new source must be reviewed again\n")
    calls.clear()
    second = drive(root, task.task_id, GATED, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert second["ready_for_collection"]
    assert calls == ["context", "check"] + (["review"] if source_changed else []) + ["acceptance"]
    assert first["acceptance"]["request_fingerprint"] != second["acceptance"]["request_fingerprint"]
    assert len(ReviewStore(root, task.task_id).decisions()) == (2 if source_changed else 1)


@pytest.mark.parametrize("damage", ["deleted", "rehashed", "chmod", "symlink", "directory_symlink", "extra", "broken_hash", "other_task", "policy"])
def test_restart_refuses_damaged_history_and_unrelated_work(project, damage):
    root, task = project
    path = discovery(root, task)
    git(root, "add", "recovery/history")
    git(root, "commit", "-m", "Protect committed history prefix")
    if damage == "deleted":
        path.unlink()  # Even deleting the last event leaves a valid, but truncated, journal.
    elif damage in ("rehashed", "broken_hash"):
        value = json.loads(path.read_text())
        value["payload"]["hypothesis"] = "Rewritten coordinator history"
        if damage == "rehashed":
            value.pop("event_id")
            value["event_id"] = digest(value)
        path.write_text(json.dumps(value))
    elif damage == "chmod":
        path.chmod(0o755)
    elif damage == "symlink":
        saved = root / "build/saved.json"
        saved.parent.mkdir(exist_ok=True)
        path.rename(saved)
        path.symlink_to(saved)
    elif damage == "directory_symlink":
        saved = root / "build/saved-history"
        saved.parent.mkdir(exist_ok=True)
        path.parent.rename(saved)
        path.parent.symlink_to(saved, target_is_directory=True)
    elif damage == "extra":
        (path.parent / "notes.txt").write_text("not a native decision")
    elif damage == "other_task":
        other = replace(task, task_id="other")
        TaskStore(root).create(other)
        git(root, "add", "recovery/tasks")
        git(root, "commit", "-m", "Another task")
        discovery(root, other)
    else:
        (root / "recovery/policy.json").write_text("{}")
    state = working_state(root)
    with pytest.raises(ValueError):
        run(project, lambda *_: pytest.fail("damaged restart must not dispatch"))
    assert working_state(root) == state  # No cleanup, rollback or silent Git mutation.


def test_restart_rejects_invalid_uncommitted_decision_even_with_valid_hash_chain(project):
    root, task = project
    store = TaskStore(root)
    store.journal.append("task:" + task.task_id, request_id="invalid", expected_revision=0,
                         payload={"kind": "pretend_acceptance", "result": "pass"})
    with pytest.raises(ValueError, match="unsupported or invalid task decision"):
        run(project, lambda *_: pytest.fail("invalid history must not dispatch"))


def test_resumed_history_never_becomes_worker_editing_permission(project):
    root, task = project
    path = discovery(root, task)
    TaskStore(root).resolve(task.task_id, discovery_id="finding", request_id="resolved", author=task.author,
                           disposition="incorporated", source_evidence=["source:src/f.cpp"], expected_revision=1)
    def worker(*_):
        path.write_text("unauthorized worker edit, preserved for inspection")
        return 3, {"error": "interrupted"}
    with pytest.raises(ValueError, match="exceeded editing scope"):
        run(project, worker)
    assert path.read_text() == "unauthorized worker edit, preserved for inspection"
