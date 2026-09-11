"""Native own-review appends must not invalidate retained worker admission."""

from copy import deepcopy
import json
from pathlib import Path

import pytest

from match_recovery.driver import drive
from match_recovery.model import digest
from match_recovery.reviews import ReviewStore
from match_recovery.tasks import TaskStore
from test_dispatch import batch
from test_retained_scope import grant, retained
from test_source_configuration import configured
from test_task_source import git, project
from test_reviews import PASS, receipt


@pytest.fixture
def reviewed_retained(retained):
    _, child, _, _, _, _ = retained
    store = TaskStore(child)
    store.resolve("owner", request_id="already-incorporated", author="coordinator",
                  discovery_id="original-evidence", disposition="incorporated",
                  source_evidence=["Synthetic original concern expressed in current source"], expected_revision=2)
    assert not store.status("owner", store.scope("owner")[0])["pending_discoveries"]
    return retained


def configuration():
    return {"schema_version": 1, "target": "custom",
            **{name: {"argv": [name], "timeout": 10} for name in ("context", "check")}}


def run(child, permission, execute, *, config=None):
    return drive(child, "owner", config or configuration(),
                 {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}},
                 iterations=1, reviewer_configuration={"author": "reviewer", "command": {"argv": ["review"], "timeout": 10}},
                 retained_scope_configuration=permission, execute=execute)


def review(payload, decision="request_changes"):
    return 0, {"snapshot": payload["snapshot"], "decision": decision,
               "evidence": ["Synthetic member declaration evidence"], "feedback": "Use the evidenced member declaration"}


def test_native_request_changes_review_allows_one_retained_worker_round(reviewed_retained):
    root, child, _, _, _, _ = reviewed_retained
    permission = grant(reviewed_retained)
    original = deepcopy(permission)
    task, history = TaskStore(child).read("owner"), TaskStore(child).decisions("owner")
    calls, receipts = [], []

    def execute(command, current, name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "context": return 0, {}
        if stage == "check": return 0, {"result": "pass"}
        if stage == "review": return review(payload, "approve" if "worker" in calls else "request_changes")
        assert stage == "worker"
        assert payload["review_feedback"]["decision"] == "request_changes"
        assert len(ReviewStore(child, "owner").decisions()) == 1
        receipt = child / "build/recovery/retained-scope-uses" / (permission["request_id"] + ".json")
        receipts.append(receipt.read_bytes())
        assert json.loads(receipts[0])["permission_sha256"] == digest(original)
        path = child / "code/owner.hpp"
        path.write_text(path.read_text() + "// synthetic evidenced member repair\n")
        return 0, {"discoveries": []}

    report = run(child, permission, execute)
    assert calls == ["context", "check", "review", "worker", "context", "check", "review"]
    assert report["worker_rounds"] == 1 and report["stop_reason"] == "full_acceptance_gate_required"
    assert not report["ready_for_collection"] and permission == original
    assert len(ReviewStore(child, "owner").decisions()) == 2
    assert TaskStore(child).read("owner") == task == TaskStore(root).read("owner")
    assert TaskStore(child).decisions("owner") == history
    receipt = child / "build/recovery/retained-scope-uses" / (permission["request_id"] + ".json")
    assert receipt.read_bytes() == receipts[0]


@pytest.mark.parametrize("stage", ["context", "check", "review", "native_append"])
@pytest.mark.parametrize("damage", ["ignored_source", "task_history", "old_review",
                                    "pending_git", "assume_unchanged", "skip_worktree"])
def test_only_exact_native_review_append_can_advance_preworker_state(reviewed_retained, monkeypatch, stage, damage):
    root, child, _, _, _, _ = reviewed_retained
    # Existing tracked source/history stay tracked; new ignored inputs must
    # nevertheless participate in the retained pre-worker scheduling snapshot.
    (root / ".git/info/exclude").write_text("code/ignored.cc\nrecovery/history/**\n")
    reviews = ReviewStore(child, "owner")
    reviews.append({"kind": "review", "author": "worker", "reviewer": "reviewer", "snapshot": "a" * 64,
                    "decision": "request_changes", "evidence": ["Earlier synthetic review"],
                    "feedback": "Preserve this original review record"}, expected_revision=0)
    old_review = reviews.journal.stream_directory(reviews.stream) / "00000001.json"
    previous = old_review.read_bytes()
    permission = grant(reviewed_retained)
    original, calls, damaged = deepcopy(permission), [], []

    def mutate():
        assert not damaged
        damaged.append(damage)
        if damage == "ignored_source":
            (child / "code/ignored.cc").write_text("// Unauthorized ignored source change retained\n")
        elif damage == "task_history":
            store = TaskStore(child)
            store.discover("owner", request_id="unauthorized-hook", author="worker",
                           hypothesis="Hook appended unexpected task history", affects=[store.read("owner").subjects[0].entity_id],
                           evidence=["Retain unauthorized history for inspection"], expected_revision=3)
        elif damage == "old_review":
            old_review.write_text(json.dumps(json.loads(previous), indent=4) + "\n")
        elif damage == "pending_git":
            marker = Path(git(child, "rev-parse", "--path-format=absolute", "--git-path", "MERGE_HEAD"))
            marker.write_text(git(child, "rev-parse", "HEAD") + "\n")
        else:
            git(child, "update-index", "--" + damage.replace("_", "-"), "code/owner.hpp")

    if stage == "native_append":
        original_append = ReviewStore.append

        def append(self, *args, **kwargs):
            value = original_append(self, *args, **kwargs)
            mutate()
            return value

        monkeypatch.setattr(ReviewStore, "append", append)

    def execute(command, current, name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == stage: mutate()
        if name == "context": return 0, {}
        if name == "check":
            # A failing check still proposes a worker; an untrusted ignored
            # history append cannot be hidden by the no-review branch either.
            if damage == "task_history" and stage in ("context", "check"):
                return 1, {"result": "fail"}
            return 0, {"result": "pass"}
        assert name == "review", "Changed pre-worker inputs must never reach the worker"
        return review(payload)

    with pytest.raises(ValueError):
        run(child, permission, execute)
    assert damaged == [damage] and "worker" not in calls and permission == original
    if damage == "ignored_source": assert (child / "code/ignored.cc").is_file()
    elif damage == "task_history": assert len(TaskStore(child).decisions("owner")) == 4
    elif damage == "old_review": assert old_review.read_bytes() != previous
    elif damage == "pending_git":
        assert Path(git(child, "rev-parse", "--path-format=absolute", "--git-path", "MERGE_HEAD")).is_file()
    else:
        assert git(child, "ls-files", "-v", "code/owner.hpp")[0] == ("h" if damage == "assume_unchanged" else "S")


@pytest.mark.parametrize("outcome", ["fail", "unknown"])
def test_native_approval_then_actionable_acceptance_feedback_can_start_retained_worker(reviewed_retained, outcome):
    root, child, _, _, _, _ = reviewed_retained
    permission = grant(reviewed_retained)
    original = deepcopy(permission)
    task, history = TaskStore(child).read("owner"), TaskStore(child).decisions("owner")
    calls = []
    failure = {**PASS, "runtime": outcome, "result": outcome, "findings": [
        {"dimension": "runtime", "message": "Synthetic lifecycle counterexample needs local investigation",
         "evidence": ["Synthetic protocol control, not game execution"]}]}

    def execute(command, current, name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "context": return 0, {}
        if stage == "check": return 0, {"result": "pass"}
        if stage == "review": return review(payload, "approve")
        if stage == "acceptance": return {"fail": 1, "unknown": 3}[outcome], receipt(payload, failure)
        assert stage == "worker"
        assert payload["review_feedback"]["decision"] == "approve"
        assert payload["acceptance_feedback"]["report"]["runtime"] == outcome
        path = child / "code/owner.hpp"
        path.write_text(path.read_text() + "// investigate synthetic lifecycle counterexample\n")
        return 0, {"discoveries": []}

    config = {**configuration(), "acceptance": {"argv": ["acceptance"], "timeout": 10}}
    report = run(child, permission, execute, config=config)
    assert calls == ["context", "check", "review", "acceptance", "worker", "context", "check", "review", "acceptance"]
    assert report["worker_rounds"] == 1 and report["stop_reason"] == "acceptance_" + outcome
    assert not report["ready_for_collection"] and permission == original
    assert TaskStore(child).read("owner") == task == TaskStore(root).read("owner")
    assert TaskStore(child).decisions("owner") == history
    assert [item["decision"] for item in ReviewStore(child, "owner").decisions()] == ["approve", "approve"]
