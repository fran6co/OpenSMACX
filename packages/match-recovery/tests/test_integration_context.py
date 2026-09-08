"""Target-context preservation does not replace original task obligations."""

import json

import pytest

from match_recovery.driver import working_state
from match_recovery.integration import validate
from match_recovery.reviews import ReviewStore
from match_recovery.source_obligations import task_obligations
from match_recovery.tasks import TaskStore
from test_preservation import project, check
from test_task_source import git


@pytest.fixture
def integrated(project):
    root = project
    base = git(root, "rev-parse", "HEAD")
    git(root, "switch", "-c", "incoming")
    owner = root / "code/owner.cc"
    owner.write_text(owner.read_text().replace("return 1", "return 2"))
    git(root, "add", "code/owner.cc")
    git(root, "commit", "-m", "Incoming ownership change")
    source = git(root, "rev-parse", "HEAD")
    git(root, "switch", "-c", "destination", base)
    caller = root / "code/caller.cc"
    caller.write_text(caller.read_text().replace("return 1", "return 2"))
    git(root, "add", "code/caller.cc")
    git(root, "commit", "-m", "Independent destination change (test-owned input)")
    target = git(root, "rev-parse", "HEAD")
    git(root, "merge", "--no-ff", "incoming", "-m", "Candidate integration; not accepted")
    permission = {"author": "coordinator", "source_revision": source, "target_revision": target,
                  "evidence": ["Coordinator selected incoming task and independently advanced destination"]}
    return root, permission


def test_fresh_target_comparison_retains_original_history_and_preserves_other_work(integrated):
    root, permission = integrated
    task = TaskStore(root).read("owner")
    before = working_state(root)
    obligations = task_obligations(root, "owner")
    assert check(root)["result"] == "fail"  # The destination legitimately differs from the original task base.
    report = check(root, integration=permission)
    assert report["result"] == "pass"
    assert report["base_revision"] == permission["target_revision"]
    assert report["task_base_revision"] == task.base_revision
    assert report["changed_files"] == ["code/owner.cc"]
    assert report["integration"] == permission
    assert task_obligations(root, "owner") == obligations and TaskStore(root).read("owner") == task
    assert working_state(root) == before and not report["ready_for_collection"]


def test_an_integration_context_cannot_waive_collateral_header_damage(integrated):
    root, permission = integrated
    (root / "code/model.h").write_text("struct Model { int padding; int field; };\n")
    report = check(root, integration=permission)
    assert report["result"] == "fail"
    assert next(row for row in report["units"] if row["unit"] == "code/caller.cc")["result"] == "fail"


@pytest.mark.parametrize("change", ["source", "policy", "notes"])
def test_context_refuses_edits_outside_the_incoming_task(integrated, change):
    root, permission = integrated
    path = root / {"source": "code/caller.cc", "policy": "recovery/project.json", "notes": "unrelated.txt"}[change]
    path.write_text((path.read_text() if path.exists() else "") + "\n ")
    before = working_state(root)
    with pytest.raises(ValueError, match="exceed authorized"):
        validate(root, TaskStore(root).read("owner"), permission)
    assert working_state(root) == before


@pytest.mark.parametrize("change", ["author", "alias", "extra", "evidence", "target_after_source"])
def test_invalid_permissions_cannot_move_the_preservation_anchor(integrated, change):
    root, permission = integrated
    damaged = dict(permission)
    if change == "author": damaged["author"] = "worker"
    elif change == "alias": damaged["target_revision"] = "destination"
    elif change == "extra": damaged["allow_regressions"] = True
    elif change == "evidence": damaged["evidence"] = []
    else: damaged["target_revision"] = git(root, "rev-parse", "HEAD")
    with pytest.raises(ValueError):
        check(root, integration=damaged)


def test_unincorporated_source_refuses_before_comparing(integrated):
    root, permission = integrated
    git(root, "switch", "incoming")
    with pytest.raises(ValueError, match="both be incorporated"):
        check(root, integration=permission)


def test_permission_cannot_replace_the_task_baseline(integrated):
    root, permission = integrated
    path = root / "recovery/tasks/owner.json"
    value = json.loads(path.read_text())
    # Even an ancestor revision cannot replace this immutable source decision.
    value["base_revision"] = git(root, "rev-parse", permission["source_revision"] + "^")
    path.write_text(json.dumps(value))
    with pytest.raises(ValueError, match="task/review history"):
        check(root, integration=permission)


def test_source_claim_introduced_on_incoming_branch_survives_integration_history(project):
    root = project
    base = git(root, "rev-parse", "HEAD")
    owner = root / "code/owner.cc"
    owner.write_text(owner.read_text().replace("0x00401006\n", "0x00401006 BYTE_EXACT\n"))
    git(root, "add", "code/owner.cc")
    git(root, "commit", "-m", "Synthetic incoming historical byte obligation")
    source = git(root, "rev-parse", "HEAD")
    permission = {"author": "coordinator", "source_revision": source, "target_revision": base,
                  "evidence": ["Fast-forward integration control"]}
    assert check(root, integration=permission)["result"] == "pass"
    before = task_obligations(root, "owner")
    assert len(before["obligations"]) == 1
    owner.write_text(owner.read_text().replace(" BYTE_EXACT", ""))
    after = task_obligations(root, "owner")
    assert [{k: v for k, v in row.items() if k != "source_evidence"} for row in after["obligations"]] == [
        {k: v for k, v in row.items() if k != "source_evidence"} for row in before["obligations"]]
    assert after["obligations"][0]["source_evidence"][0]["revision"] == source
    validate(root, TaskStore(root).read("owner"), permission)
    # Inventory, not a passing acceptance: actual restoration remains mandatory.


def test_old_review_does_not_cover_a_new_integration_policy(integrated):
    root, permission = integrated
    reviews = ReviewStore(root, "owner")
    state = working_state(root)
    assert reviews.snapshot(state, {}) != reviews.snapshot(state, {"integration": permission})


@pytest.mark.parametrize("side", ["source", "target"])
def test_committed_merge_resolution_cannot_discard_either_sides_task_decisions(integrated, side):
    root, permission = integrated
    original_candidate = git(root, "rev-parse", "HEAD")
    git(root, "switch", "-c", "decision-side", permission[side + "_revision"])
    store = TaskStore(root)
    task = store.read("owner")
    store.discover("owner", request_id="side-discovery", author="worker", expected_revision=0,
                   hypothesis="A source-supported relationship must survive integration",
                   affects=(task.subjects[0].entity_id,), evidence=("source:code/owner.cc",))
    git(root, "add", "recovery/history")
    git(root, "commit", "-m", "Committed side-branch discovery")
    permission = {**permission, side + "_revision": git(root, "rev-parse", "HEAD")}
    git(root, "switch", "-c", "resolved-candidate", original_candidate)
    git(root, "merge", "--no-ff", "decision-side", "-m", "Merge side decision before testing loss")
    path = next((root / "recovery/history").rglob("*.json"))
    path.unlink()
    git(root, "add", "-u")
    git(root, "commit", "-m", "Deliberately bad committed merge resolution loses discovery")
    before = working_state(root)
    with pytest.raises(ValueError, match="task/review history"):
        check(root, integration=permission)
    assert working_state(root) == before
