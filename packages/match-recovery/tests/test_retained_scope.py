"""Native retained ownership admission: permission is not recovery acceptance."""

from copy import deepcopy
from dataclasses import replace
import json
from pathlib import Path

import pytest

from match_recovery import retained_scope
from match_recovery.dispatch import dispatch
from match_recovery.driver import working_state
from match_recovery.model import Origin
from match_recovery.reviews import ReviewStore
from match_recovery.tasks import TaskStore
from test_dispatch import batch
from test_source_configuration import configured
from test_task_source import git, project


SPARE = Origin("a" * 64, "custom:offset", 48)


def finding(root, name, request="finding"):
    store = TaskStore(root)
    store.discover(name, request_id=request, author="worker", hypothesis="Retain ownership evidence",
                   affects=[store.read(name).subjects[0].entity_id], evidence=["synthetic source"],
                   expected_revision=len(store.decisions(name)))


def expand(root, name, *, subjects=(), files=("code/spare.cc", "code/owner.hpp"), request="expansion"):
    store = TaskStore(root)
    store.expand(name, request_id=request, author="coordinator", subjects=subjects,
                 editable_files=files, evidence=["explicit retained ownership decision"],
                 expected_revision=len(store.decisions(name)))


@pytest.fixture
def retained(batch):
    root, plan = batch
    (root / "code/spare.cc").write_text(
        "// ORIGINAL: 0x00000030 _spare 0x00000030-0x00000038\nvoid spare() {}\n")
    # Establish a real nonempty coordinator prefix before the assignment. The
    # workers may append to it, never rewrite or transplant a replacement chain.
    finding(root, "owner", "original-evidence")
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic retained assignment and source")
    revision = git(root, "rev-parse", "HEAD")
    for row in plan["jobs"]:
        git(Path(row["root"]), "merge", "--ff-only", revision)
    child, peer = (Path(row["root"]) for row in plan["jobs"])
    expand(child, "owner", subjects=(SPARE,))
    peers = [{"task": row["task"], "root": row["root"],
              "branch": "refs/heads/" + row["task"]} for row in plan["jobs"]]
    return root, child, peer, revision, peers, plan


def grant(retained, **overrides):
    root, child, _, revision, peers, _ = retained
    options = {"revision": revision, "branch": "refs/heads/owner", "peers": peers}
    options.update(overrides)
    return retained_scope.authorize(root, child, "owner", **options)


def test_admission_preserves_original_tasks_baselines_and_coordinator_history(retained):
    root, child, _, _, _, _ = retained
    before = working_state(root)
    task = TaskStore(root).read("owner")
    journal = TaskStore(root).decisions("owner")
    child_history = TaskStore(child).decisions("owner")
    permission = grant(retained)
    expected = TaskStore(child).scope("owner")
    assert expected != TaskStore(root).scope("owner")
    assert retained_scope.validate(child, "owner", json.loads(json.dumps(permission))) == expected
    assert retained_scope.consume(child, "owner", permission) == expected
    assert retained_scope.check_ownership(child, "owner", permission) == expected
    assert working_state(root) == before
    assert TaskStore(root).read("owner") == TaskStore(child).read("owner") == task
    assert TaskStore(root).decisions("owner") == journal
    assert TaskStore(child).decisions("owner") == child_history
    receipts = list((child / "build/recovery/retained-scope-uses").glob("*.json"))
    assert len(receipts) == 1
    assert git(child, "check-ignore", str(receipts[0]))
    assert "ready_for_collection" not in permission
    with pytest.raises(ValueError, match="consumed"):
        retained_scope.consume(child, "owner", permission)
    assert len(list(receipts[0].parent.glob("*.json"))) == 1


def test_real_committed_rehoming_and_uncommitted_member_edit_remain_admissible(retained):
    root, child, _, _, _, _ = retained
    before = working_state(root)
    owner, spare = child / "code/owner.cc", child / "code/spare.cc"
    owner.write_text(owner.read_text() + spare.read_text())
    spare.write_text("// Ownership moved to owner.cc by this synthetic control.\n")
    git(child, "add", "code", "recovery/history")
    git(child, "commit", "-m", "Retain authorized synthetic rehoming")
    (child / "code/owner.hpp").write_text("void owner();\nvoid spare();\n")
    finding(child, "owner", "post-rehoming-evidence")
    permission = grant(retained)
    assert retained_scope.validate(child, "owner", permission) == TaskStore(child).scope("owner")
    assert working_state(root) == before
    assert TaskStore(child).read("owner").base_revision == TaskStore(root).read("owner").base_revision


@pytest.mark.parametrize("target", ["own_source", "own_finding", "peer_source", "peer_finding", "peer_review"])
def test_ownership_guard_allows_parallel_source_and_same_scope_evidence(retained, target):
    _, child, peer, _, _, _ = retained
    permission = grant(retained)
    retained_scope.consume(child, "owner", permission)
    if target.endswith("source"):
        path = (child / "code/owner.hpp") if target.startswith("own") else peer / "code/other.cc"
        path.write_text(path.read_text() + "// concurrent in-scope work\n")
    elif target.endswith("finding"):
        finding(child if target.startswith("own") else peer,
                "owner" if target.startswith("own") else "other", "parallel-finding")
    else:
        ReviewStore(peer, "other").append(
            {"kind": "review", "author": "worker", "reviewer": "independent", "snapshot": "a" * 64,
             "decision": "request_changes", "evidence": ["synthetic source"],
             "feedback": "No recovery acceptance is implied"}, expected_revision=0)
    assert retained_scope.check_ownership(child, "owner", permission) == TaskStore(child).scope("owner")
    if target.startswith("own"):
        with pytest.raises(ValueError, match="stale"):
            retained_scope.validate(child, "owner", permission)
    else:
        # A peer legitimately progresses between planning and this worker's start.
        assert retained_scope.validate(child, "owner", permission) == TaskStore(child).scope("owner")


@pytest.mark.parametrize("damage", ["own_scope", "peer_scope", "own_task", "peer_task", "own_branch",
                                    "peer_branch", "root_source", "root_finding", "root_task", "root_head"])
def test_ownership_changes_refuse_without_discarding_pending_work(retained, damage):
    root, child, peer, _, _, _ = retained
    permission = grant(retained)
    if damage.endswith("scope"):
        expand(child if damage.startswith("own") else peer,
               "owner" if damage.startswith("own") else "other",
               files=("code/other.cc" if damage.startswith("own") else "code/owner.cc",), request="new-scope")
    elif damage.endswith("task"):
        directory, name = ((child, "owner") if damage.startswith("own") else
                           (peer, "other") if damage.startswith("peer") else (root, "owner"))
        task = TaskStore(directory).read(name)
        (directory / f"recovery/tasks/{name}.json").write_text(json.dumps(replace(task, intent="Changed ownership").to_dict()))
    elif damage.endswith("branch"):
        git(child if damage.startswith("own") else peer, "branch", "-m", "changed-assignment")
    elif damage == "root_source":
        path = root / "code/owner.hpp"
        path.write_text(path.read_text() + "// concurrent coordinator source\n")
    elif damage == "root_finding":
        finding(root, "owner", "coordinator-finding")
    else:
        git(root, "commit", "--allow-empty", "-m", "Moved coordinator checkpoint")
    before = [working_state(path) for path in (root, child, peer)]
    with pytest.raises(ValueError):
        retained_scope.check_ownership(child, "owner", permission)
    assert [working_state(path) for path in (root, child, peer)] == before


@pytest.mark.parametrize("damage", ["source", "finding", "head", "index", "permission"])
def test_prestart_epoch_changes_cannot_consume_a_receipt(retained, damage):
    _, child, _, _, _, _ = retained
    permission = grant(retained)
    if damage == "source":
        path = child / "code/owner.hpp"
        path.write_text(path.read_text() + "// retained new member evidence\n")
    elif damage == "finding":
        finding(child, "owner", "fresh")
    elif damage == "head":
        git(child, "commit", "--allow-empty", "-m", "Moved worker checkpoint")
    elif damage == "index":
        git(child, "add", "recovery/history")
    else:
        permission = {**permission, "accept": True}
    before = working_state(child)
    with pytest.raises(ValueError):
        retained_scope.consume(child, "owner", permission)
    assert working_state(child) == before
    assert not list((child / "build/recovery/retained-scope-uses").glob("*.json"))


@pytest.mark.parametrize("committed", [False, True])
def test_raw_outside_scope_work_refuses_even_if_committed(retained, committed):
    _, child, _, _, _, _ = retained
    path = child / "code/other.cc"
    path.write_text(path.read_text() + "// unauthorized but preserved edit\n")
    if committed:
        git(child, "add", "code/other.cc")
        git(child, "commit", "-m", "Synthetic forbidden collateral change")
    before = working_state(child)
    with pytest.raises(ValueError):
        grant(retained)
    assert working_state(child) == before


def test_ignored_configured_source_cannot_hide_collateral_work(retained):
    root, child, _, _, _, _ = retained
    (root / ".git/info/exclude").write_text("code/ignored.cc\n")
    path = child / "code/ignored.cc"
    path.write_text("// Unowned application source must remain visible to admission.\n")
    assert git(child, "check-ignore", "code/ignored.cc")
    before = working_state(child)
    with pytest.raises(ValueError):
        grant(retained)
    assert working_state(child) == before
    assert path.read_text().startswith("// Unowned")


@pytest.mark.parametrize("damage", ["assignment_ancestry", "assigned_branch", "baseline", "aliased_child", "aliased_peer"])
def test_assignment_and_canonical_worktree_identity_cannot_be_reinterpreted(retained, damage):
    root, child, peer, revision, peers, _ = retained
    options = {}
    if damage == "assignment_ancestry":
        git(peer, "commit", "--allow-empty", "-m", "Unrelated peer assignment")
        options["revision"] = git(peer, "rev-parse", "HEAD")
    elif damage == "assigned_branch":
        options["branch"] = "refs/heads/other"
    elif damage == "baseline":
        task = TaskStore(child).read("owner")
        (child / "recovery/tasks/owner.json").write_text(
            json.dumps(replace(task, base_revision=revision).to_dict()))
    else:
        alias = root / "build/checkout-alias"
        alias.symlink_to(child if damage == "aliased_child" else peer, target_is_directory=True)
        if damage == "aliased_child":
            with pytest.raises(ValueError, match="canonical"):
                retained_scope.authorize(root, alias, "owner", revision=revision,
                                         branch="refs/heads/owner", peers=peers)
            return
        options["peers"] = [{**row, "root": str(alias)} if row["task"] == "other" else row
                            for row in peers]
    before = [working_state(path) for path in (root, child, peer)]
    with pytest.raises(ValueError):
        grant(retained, **options)
    assert [working_state(path) for path in (root, child, peer)] == before


@pytest.mark.parametrize("damage", ["byte_rewrite", "divergent_chain", "missing_prefix"])
def test_coordinator_journal_must_be_an_exact_byte_prefix(retained, damage):
    _, child, _, _, _, _ = retained
    store = TaskStore(child)
    directory = store.journal.stream_directory("task:owner")
    if damage == "byte_rewrite":
        first = directory / "00000001.json"
        first.write_text(json.dumps(json.loads(first.read_text()), indent=4) + "\n")
    else:
        for path in directory.glob("*.json"):
            path.unlink()
        if damage == "divergent_chain":
            finding(child, "owner", "replacement-not-original")
        expand(child, "owner", subjects=(SPARE,))
    # Commit the replacement so the ordinary HEAD-prefix guard cannot substitute
    # for proving that the coordinator's original chain is still present.
    git(child, "add", "recovery/history")
    git(child, "commit", "-m", "Synthetic invalid rewritten history")
    before = working_state(child)
    with pytest.raises(ValueError, match="journal|history"):
        grant(retained)
    assert working_state(child) == before


@pytest.mark.parametrize("owner", ["root_only", "managed_peer", "external_reserved"])
@pytest.mark.parametrize("dimension", ["subject", "file"])
def test_conflicts_cover_all_root_tasks_inactive_peers_and_external_reservations(retained, owner, dimension):
    root, child, peer, _, peers, _ = retained
    subjects = (SPARE,) if dimension == "subject" else ()
    files = ("code/owner.hpp",) if dimension == "file" else ()
    expand(root if owner == "root_only" else peer, "other", subjects=subjects, files=files)
    options = {}
    if owner == "external_reserved":
        options = {"peers": [row for row in peers if row["task"] == "owner"],
                   "reservations": [{"root": str(peer), "tasks": ["other"]}]}
    before = [working_state(path) for path in (root, child, peer)]
    with pytest.raises(ValueError, match="overlap"):
        grant(retained, **options)
    assert [working_state(path) for path in (root, child, peer)] == before


def test_disjoint_external_reservation_revalidates_ownership_not_findings(retained):
    _, child, peer, _, peers, _ = retained
    permission = grant(retained, peers=[row for row in peers if row["task"] == "owner"],
                       reservations=[{"root": str(peer), "tasks": ["other"]}])
    finding(peer, "other", "reserved-finding")
    assert retained_scope.validate(child, "owner", permission) == TaskStore(child).scope("owner")
    expand(peer, "other", files=("code/owner.hpp",))
    with pytest.raises(ValueError):
        retained_scope.check_ownership(child, "owner", permission)


def test_peer_list_may_omit_self_but_cannot_repeat_a_checkout(retained):
    _, child, _, _, peers, _ = retained
    permission = grant(retained, peers=[row for row in peers if row["task"] != "owner"])
    assert retained_scope.validate(child, "owner", permission) == TaskStore(child).scope("owner")
    with pytest.raises(ValueError, match="duplicate"):
        grant(retained, peers=peers + [deepcopy(peers[-1])])


def test_ordinary_dispatch_cannot_infer_retained_scope_admission(retained):
    root, _, _, _, _, plan = retained
    with pytest.raises(ValueError, match="scope"):
        dispatch(root, plan)
    assert not list((root / "build/signals").iterdir())
