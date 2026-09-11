"""Partial repair expansion retains outside impact context and all findings."""

from copy import deepcopy

import pytest

from match_recovery.campaign import _scoped_coordinator
from match_recovery.coordination import SerialCoordinationRequired, coordinate
from match_recovery.driver import working_state
from match_recovery.model import Origin, require_digest
from match_recovery.tasks import Task, TaskStore
from test_coordination import answer, setup
from test_source_configuration import configured
from test_task_source import git, project


@pytest.fixture
def partial(setup):
    root, store, task, repair, delegation = setup
    outside = Origin("a" * 64, "custom:offset", 48)
    (root / "code/external.cc").write_text(
        "// ORIGINAL: 0x00000030 _external 0x00000030-0x00000038\nvoid external() {}\n")
    other = Task("external-owner", task.target, "other-coordinator", "Preserve external ownership",
                 task.base_revision, (outside,), ("code/external.cc",), ("Known caller ownership",))
    store.create(other)
    git(root, "add", "code/external.cc", "recovery/tasks/external-owner.json")
    git(root, "commit", "-m", "Independent known affected owner")
    store.discover(task.task_id, request_id="layout", author="worker",
                   hypothesis="Repair the delegated callee while retaining its outside caller as impact context",
                   affects=[repair.entity_id, outside.entity_id], evidence=["Original caller and callee inspected"],
                   expected_revision=0)
    return root, store, task, repair, outside, delegation


def expansion(partial, *, delegation=None, subjects=None, files=None):
    root, _, task, repair, _, original = partial
    return coordinate(root, task.task_id, original if delegation is None else delegation,
                      execute=lambda *args: (0, answer(args[-1], "expand",
                          subjects=[repair.entity_id] if subjects is None else subjects,
                          editable_files=["code/caller.cc"] if files is None else files)))


def preserved_state(root, store, name):
    """Include exact source/history bytes as well as effective ownership."""
    contents = {path.relative_to(root).as_posix(): path.read_bytes()
                for directory in ("code", "hints", "recovery/tasks", "recovery/history")
                for path in (root / directory).rglob("*") if path.is_file()}
    return working_state(root), store.read(name), store.scope(name), store.decisions(name), contents


def assert_refusal(report, reason, *, action="expand"):
    assert set(report) == {"action", "published", "discovery_id", "snapshot", "proposed_action",
                           "reason_code", "feedback", "ready_for_collection", "verification"}
    assert report["action"] == "proposal_rejected"
    assert report["proposed_action"] == action
    assert report["reason_code"] == reason
    assert report["discovery_id"] == "layout"
    assert report["published"] is False and report["ready_for_collection"] is False
    assert report["verification"] == "not_measured"
    assert isinstance(report["feedback"], str) and report["feedback"].strip()
    require_digest(report["snapshot"])


def test_partial_expansion_repairs_only_delegated_subset_and_retains_other_owner(partial):
    root, store, task, repair, outside, delegation = partial
    original = store.decisions(task.task_id)[0]
    other = store.read("external-owner"), store.scope("external-owner")
    source = {path: path.read_bytes() for path in (root / "code").iterdir()}
    head = git(root, "rev-parse", "HEAD")
    report = expansion(partial)
    assert report["action"] == "expand" and report["published"]
    assert not report["ready_for_collection"] and report["verification"] == "not_measured"
    subjects, files = store.scope(task.task_id)
    assert subjects == {task.subjects[0].entity_id, repair.entity_id}
    assert files == {"code/owner.cc", "code/caller.cc"}
    assert outside.entity_id not in subjects and "code/external.cc" not in files
    status = store.status(task.task_id, subjects | {outside.entity_id})
    assert status["pending_discoveries"] == [original]
    assert status["scope_expansion_required"] == [outside.entity_id]
    assert store.read(task.task_id) == task
    assert (store.read("external-owner"), store.scope("external-owner")) == other
    assert git(root, "rev-parse", "HEAD") == head
    assert all(path.read_bytes() == contents for path, contents in source.items())

    def investigate(command, current, name, request):
        assert set(request["subjects"]) == subjects
        assert request["pending_discoveries"] == [original]
        assert outside.entity_id in {row["entity_id"] for row in request["mapped_records"]}
        return 0, answer(request, "investigate_local")

    before = store.decisions(task.task_id)
    local = coordinate(root, task.task_id, delegation, execute=investigate)
    assert local["action"] == "investigate_local" and not local["published"]
    assert store.decisions(task.task_id) == before


@pytest.mark.parametrize("damage", ["no_delegation", "outside_subject", "outside_file", "unknown_impact",
                                    "unknown_repair", "unaffected_repair", "policy_file", "no_progress"])
def test_partial_expansion_does_not_relax_original_binding_or_editing_bounds(partial, damage):
    root, store, task, repair, outside, original = partial
    delegation = deepcopy(original)
    subjects, files = [repair.entity_id], ["code/caller.cc"]
    if damage == "no_delegation":
        delegation.update(subjects=[], editable_files=[])
    elif damage == "outside_subject":
        subjects.append(outside.entity_id)
    elif damage == "outside_file":
        files.append("code/external.cc")
    elif damage == "unknown_impact":
        # A new unknown original remains visible and cannot authorize even a
        # known repair subset of that particular finding.
        store.discover(task.task_id, request_id="unknown", author="worker", hypothesis="Unknown caller impact",
                       affects=[repair.entity_id, "entity-" + "b" * 64], evidence=["Unresolved address"], expected_revision=1)
    elif damage == "unknown_repair":
        subjects = ["entity-" + "b" * 64]
        delegation["subjects"] = subjects
    elif damage == "unaffected_repair":
        subjects = [task.subjects[0].entity_id]
        delegation["subjects"] = subjects
    elif damage == "policy_file":
        files = ["recovery/project.json"]
        delegation["editable_files"] = files
    else:
        subjects, files = [], []
    before = preserved_state(root, store, task.task_id)
    if damage == "unknown_impact":
        with pytest.raises(ValueError, match="unknown originals"):
            coordinate(root, task.task_id, delegation, execute=lambda *args: (0, answer(args[-1], "expand",
                       discovery_id="unknown", subjects=subjects, editable_files=files)))
    elif damage == "policy_file":
        with pytest.raises(ValueError, match="existing regular source files"):
            expansion(partial, delegation=delegation, subjects=subjects, files=files)
    else:
        report = expansion(partial, delegation=delegation, subjects=subjects, files=files)
        assert_refusal(report, "expansion_no_progress" if damage == "no_progress" else "expansion_exceeds_delegation")
    assert preserved_state(root, store, task.task_id) == before
    assert store.read(task.task_id).base_revision == task.base_revision


def test_existing_owner_is_still_removed_from_shared_campaign_expansion_grants(partial):
    root, store, task, repair, outside, original = partial
    offered = {**original, "subjects": [repair.entity_id, outside.entity_id],
               "editable_files": ["code/caller.cc", "code/external.cc"]}
    delegated = _scoped_coordinator(root, offered, task.task_id, [])
    assert delegated["subjects"] == [repair.entity_id]
    assert delegated["editable_files"] == ["code/caller.cc"]
    before = preserved_state(root, store, task.task_id)
    report = expansion(partial, delegation=delegated, subjects=[repair.entity_id, outside.entity_id])
    assert_refusal(report, "expansion_exceeds_delegation")
    assert preserved_state(root, store, task.task_id) == before
    assert len(store.decisions(task.task_id)) == 1
    assert expansion(partial, delegation=delegated)["published"]


def test_native_preplanning_instructions_route_partial_expansion_without_publishing(partial):
    root, store, task, repair, _, delegation = partial
    before = store.decisions(task.task_id), store.scope(task.task_id)

    def propose(command, current, name, request):
        assert set(request["allowed_actions"]) == {"investigate", "investigate_local", "defer"}
        instructions = "\n".join(request["instructions"])
        assert "read-only impact context" in instructions and "fresh serial coordination" in instructions
        return 0, answer(request, "expand", subjects=[repair.entity_id], editable_files=["code/caller.cc"])

    with pytest.raises(SerialCoordinationRequired):
        coordinate(root, task.task_id, delegation, execute=propose,
                   allowed_actions=("investigate", "investigate_local", "defer"))
    assert (store.decisions(task.task_id), store.scope(task.task_id)) == before


@pytest.mark.parametrize("extra_delegation", [False, True])
def test_partial_repair_cannot_resolve_whole_cross_owner_finding(partial, extra_delegation):
    root, store, task, repair, outside, original = partial
    expansion(partial)
    delegation = deepcopy(original)
    if extra_delegation:
        # Even allowing central read/decision context is not ownership: the
        # TaskStore independently refuses incorporation beyond actual scope.
        delegation["subjects"].append(outside.entity_id)
    before = preserved_state(root, store, task.task_id)
    report = coordinate(root, task.task_id, delegation,
                        execute=lambda *args: (0, answer(args[-1], "incorporate")))
    assert_refusal(report, "resolution_requires_expansion" if extra_delegation else "outside_delegated_scope",
                   action="incorporate")
    assert preserved_state(root, store, task.task_id) == before
    with pytest.raises(ValueError):
        store.resolve(task.task_id, discovery_id="layout", request_id="premature-incorporation", author=task.author,
                      disposition="incorporated", source_evidence=["Local repair is not the entire affected scope"],
                      expected_revision=len(store.decisions(task.task_id)))
    assert preserved_state(root, store, task.task_id) == before
    assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
