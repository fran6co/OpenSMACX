"""Typed ownership routing preserves findings without expanding or accepting."""

from copy import deepcopy
from dataclasses import replace
import json
from pathlib import Path
import shutil

import pytest

from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.model import Origin, digest
from match_recovery.task_routes import route_task, target_reference, validate_route_target
from match_recovery.tasks import Task, TaskStore
from test_coordination import setup
from test_source_configuration import configured
from test_task_source import project


@pytest.fixture
def routed(setup):
    root, store, task, caller, _ = setup
    target = Task("caller-owner", task.target, "target-coordinator", "Recover caller ownership",
                  task.base_revision, (caller,), ("code/caller.cc",), ("Existing caller identity",))
    store.create(target)
    store.discover(target.task_id, request_id="target-evidence", author="target-worker",
                   hypothesis="Target has its own unfinished evidence", affects=[caller.entity_id],
                   evidence=["Caller original"], expected_revision=0)
    store.discover(task.task_id, request_id="relationship", author="worker",
                   hypothesis="Repair owned body while retaining the outside caller concern",
                   affects=[task.subjects[0].entity_id, caller.entity_id], evidence=["Original ownership relationship"],
                   expected_revision=0)
    event, = store.journal.read("task:" + task.task_id)
    route = {"kind": "discovery_routed", "author": task.author, "discovery_id": "relationship",
             "discovery_event_id": event.event_id, "target": target_reference(store, target.task_id),
             "subjects": [caller.entity_id], "evidence": ["Route outside repair to its existing owner; retain the whole finding"]}
    decision = {"request_id": "route-caller", "expected_revision": 1, "route": route}
    return root, store, task, target, caller, decision


def source_bytes(root):
    return {path.relative_to(root).as_posix(): path.read_bytes()
            for base in ("code", "hints", "recovery/tasks") for path in (root / base).rglob("*") if path.is_file()}


def test_route_is_a_pinned_decision_not_scope_resolution_or_acceptance(routed):
    root, store, source, target, caller, decision = routed
    original = store.decisions(source.task_id)[0]
    source_scope, target_scope = store.scope(source.task_id), store.scope(target.task_id)
    target_history = store.decisions(target.task_id)
    contents = source_bytes(root)
    reference = decision["route"]["target"]
    target_event, = store.journal.read("task:" + target.task_id)
    assert reference == {"task_id": target.task_id, "task_sha256": digest(target.to_dict()),
                         "base_revision": target.base_revision, "history_head": target_event.event_id}
    result = route_task(root, source.task_id, decision)
    assert result == {"published": True, "route": decision["route"],
                      "verification": "not_measured", "ready_for_collection": False}
    assert store.decisions(source.task_id) == (original, decision["route"])
    assert store.scope(source.task_id) == source_scope and store.scope(target.task_id) == target_scope
    assert store.decisions(target.task_id) == target_history and source_bytes(root) == contents
    status = store.status(source.task_id, source_scope[0] | {caller.entity_id})
    assert status["pending_discoveries"] == [original]
    assert caller.entity_id in status["scope_expansion_required"]
    assert store.read(source.task_id) == source and store.read(target.task_id) == target


def test_exact_request_retry_does_not_duplicate_routing_or_change_the_original_pin(routed):
    root, store, source, _, _, decision = routed
    first = route_task(root, source.task_id, decision)
    state = working_state(root)
    assert route_task(root, source.task_id, deepcopy(decision)) == first
    assert working_state(root) == state and len(store.decisions(source.task_id)) == 2
    changed = deepcopy(decision)
    changed["route"]["evidence"].append("Different decision cannot reuse the request identity")
    with pytest.raises(ValueError): route_task(root, source.task_id, changed)
    assert working_state(root) == state


@pytest.mark.parametrize("already_published", [False, True])
def test_new_routing_requires_current_target_head_but_exact_retry_accepts_target_progress(routed, already_published):
    root, store, source, target, caller, decision = routed
    if already_published: route_task(root, source.task_id, decision)
    store.discover(target.task_id, request_id="later-target-progress", author="target-worker",
                   hypothesis="Target has new source evidence", affects=[caller.entity_id],
                   evidence=["Current target evidence"], expected_revision=1)
    before = working_state(root)
    if already_published:
        assert route_task(root, source.task_id, decision)["route"] == decision["route"]
    else:
        with pytest.raises(ValueError): route_task(root, source.task_id, decision)
    assert working_state(root) == before
    assert len(store.decisions(source.task_id)) == (2 if already_published else 1)
    assert store.status(source.task_id, store.scope(source.task_id)[0] | {caller.entity_id})["pending_discoveries"]


def test_subject_order_is_canonical_even_when_every_subject_is_valid_and_target_owned(routed):
    root, store, source, target, caller, original = routed
    extra = Origin(caller.image_sha256, caller.address_space, 48)
    (root / "code/extra.cc").write_text("// ORIGINAL: 0x00000030 _extra 0x00000030-0x00000038\nvoid extra() {}\n")
    store.expand(target.task_id, request_id="extra-owned", author=target.author, subjects=(extra,),
                 editable_files=("code/extra.cc",), evidence=("Additional target-owned function",), expected_revision=1)
    subjects = sorted((caller.entity_id, extra.entity_id))
    store.discover(source.task_id, request_id="two-target-functions", author="worker", hypothesis="Both target functions participate",
                   affects=subjects, evidence=["Two original call relationships"], expected_revision=1)
    event = store.journal.read("task:" + source.task_id)[-1]
    decision = {"request_id": "route-two", "expected_revision": 2,
                "route": {**original["route"], "discovery_id": "two-target-functions", "discovery_event_id": event.event_id,
                          "target": target_reference(store, target.task_id), "subjects": list(reversed(subjects))}}
    before = working_state(root)
    with pytest.raises(ValueError): route_task(root, source.task_id, decision)
    assert working_state(root) == before
    decision["route"]["subjects"] = subjects
    assert route_task(root, source.task_id, decision)["published"]


@pytest.mark.parametrize("damage", ["revision", "event", "reference_task", "reference_hash", "reference_base",
                                    "reference_head", "author", "self_target", "empty", "duplicate", "owned",
                                    "unaffected", "extra_decision", "extra_route", "empty_evidence", "kind"])
def test_stale_or_malformed_route_refuses_without_mutating_either_owner(routed, damage):
    root, store, source, target, caller, original = routed
    decision = deepcopy(original)
    route = decision["route"]
    if damage == "revision": decision["expected_revision"] = 0
    elif damage == "event": route["discovery_event_id"] = "f" * 64
    elif damage == "reference_task": route["target"]["task_id"] = "missing-task"
    elif damage == "reference_hash": route["target"]["task_sha256"] = "f" * 64
    elif damage == "reference_base": route["target"]["base_revision"] = "f" * 40
    elif damage == "reference_head": route["target"]["history_head"] = "f" * 64
    elif damage == "author": route["author"] = target.author
    elif damage == "self_target": route["target"] = target_reference(store, source.task_id)
    elif damage == "empty": route["subjects"] = []
    elif damage == "duplicate": route["subjects"] = [caller.entity_id, caller.entity_id]
    elif damage == "owned": route["subjects"] = [source.subjects[0].entity_id]
    elif damage == "unaffected": route["subjects"] = [Origin("a" * 64, "custom:offset", 48).entity_id]
    elif damage == "extra_decision": decision["accept"] = True
    elif damage == "extra_route": route["accepted"] = True
    elif damage == "empty_evidence": route["evidence"] = []
    else: route["kind"] = "discovery_resolved"
    before = working_state(root)
    with pytest.raises((ValueError, FileNotFoundError)):
        route_task(root, source.task_id, decision)
    assert working_state(root) == before and len(store.decisions(source.task_id)) == 1


def test_already_routed_subject_cannot_be_reassigned_by_another_request(routed):
    root, store, source, _, _, decision = routed
    route_task(root, source.task_id, decision)
    before = working_state(root)
    repeated = {**deepcopy(decision), "request_id": "duplicate-routing", "expected_revision": 2}
    with pytest.raises(ValueError): route_task(root, source.task_id, repeated)
    assert working_state(root) == before and len(store.decisions(source.task_id)) == 2


def test_resolved_finding_cannot_be_routed_as_pending_work(routed):
    root, store, source, _, caller, decision = routed
    store.expand(source.task_id, request_id="scope-for-resolution", author=source.author, subjects=(caller,),
                 editable_files=(), evidence=("Synthetic whole-finding ownership",), expected_revision=1)
    store.resolve(source.task_id, request_id="closed", author=source.author, discovery_id="relationship",
                  disposition="incorporated", source_evidence=["Synthetic reviewed source expression"], expected_revision=2)
    decision = {**decision, "expected_revision": 3}
    before = working_state(root)
    with pytest.raises(ValueError): route_task(root, source.task_id, decision)
    assert working_state(root) == before and len(store.decisions(source.task_id)) == 3


@pytest.mark.parametrize("difference", ["target", "image", "address_space"])
def test_route_cannot_cross_project_or_original_mapping_domains(routed, difference):
    root, store, source, target, caller, decision = routed
    changed = replace(target, target="other-project") if difference == "target" else replace(
        target, subjects=(Origin("b" * 64 if difference == "image" else caller.image_sha256,
                                 "other:offset" if difference == "address_space" else caller.address_space, caller.address),))
    # Use a clean target history to avoid testing unrelated old-subject parsing.
    changed = replace(changed, task_id="foreign-owner")
    store.create(changed)
    decision = deepcopy(decision)
    decision["route"]["target"] = target_reference(store, changed.task_id)
    before = working_state(root)
    with pytest.raises(ValueError): route_task(root, source.task_id, decision)
    assert working_state(root) == before


def test_target_reference_permits_append_only_progress_but_not_a_fork(routed, tmp_path):
    _, store, _, target, caller, decision = routed
    reference = decision["route"]["target"]
    store.discover(target.task_id, request_id="target-progress", author="target-worker", hypothesis="Fresh target evidence",
                   affects=[caller.entity_id], evidence=["Current target source"], expected_revision=1)
    validate_route_target(store, target.task_id, reference, [caller.entity_id])
    alternate = TaskStore(tmp_path / "fork")
    alternate.create(target)
    alternate.discover(target.task_id, request_id="forked-evidence", author="target-worker", hypothesis="Unrelated replacement chain",
                       affects=[caller.entity_id], evidence=["Not the pinned original history"], expected_revision=0)
    with pytest.raises(ValueError): validate_route_target(alternate, target.task_id, reference, [caller.entity_id])


def test_empty_target_history_pin_and_later_target_progress_are_valid(routed):
    root, store, _, target, caller, _ = routed
    fresh = replace(target, task_id="fresh-target")
    store.create(fresh)
    reference = target_reference(store, fresh.task_id)
    assert reference["history_head"] is None
    store.discover(fresh.task_id, request_id="first", author="worker", hypothesis="First target progress",
                   affects=[caller.entity_id], evidence=["Source"], expected_revision=0)
    validate_route_target(store, fresh.task_id, reference, [caller.entity_id])


@pytest.mark.parametrize("change", ["identity", "base", "ownership_loss"])
def test_target_identity_baseline_and_ownership_are_rechecked(routed, change):
    root, store, _, target, caller, decision = routed
    altered = (replace(target, intent="Changed target decision") if change == "identity" else
               replace(target, base_revision="f" * 40) if change == "base" else
               replace(target, subjects=(Origin("a" * 64, caller.address_space, 48),)))
    (root / "recovery/tasks" / (target.task_id + ".json")).write_text(json.dumps(altered.to_dict()))
    with pytest.raises(ValueError):
        validate_route_target(store, target.task_id, decision["route"]["target"], [caller.entity_id])


@pytest.mark.parametrize("locked", ["source", "target", "target_journal"])
def test_route_requires_both_driver_leases_before_any_publication(routed, tmp_path_factory, locked):
    root, store, source, target, _, decision = routed
    other = tmp_path_factory.mktemp("route-target") / "checkout"
    shutil.copytree(root, other)
    before = working_state(root)
    selected = root if locked == "source" else other
    lock = (TaskStore(other).journal.lock_directory / (digest("task:" + target.task_id) + ".lock")
            if locked == "target_journal" else selected / "build/recovery/driver.lock")
    with file_lock(lock, timeout=0):
        with pytest.raises(TimeoutError): route_task(root, source.task_id, decision, target_root=other)
    assert working_state(root) == before and len(store.decisions(source.task_id)) == 1


def test_routed_history_is_readable_after_target_checkout_becomes_unavailable(routed, tmp_path_factory):
    root, store, source, target, caller, decision = routed
    other = tmp_path_factory.mktemp("route-target") / "checkout"
    shutil.copytree(root, other)
    # The route target exists only in the external checkout, not as a fallback
    # local task. Historical readers must validate shape without dereferencing it.
    (root / "recovery/tasks" / (target.task_id + ".json")).unlink()
    original = store.decisions(source.task_id)[0]
    route_task(root, source.task_id, decision, target_root=other)
    other.rename(other.with_name("preserved-offline-checkout"))
    assert store.decisions(source.task_id) == (original, decision["route"])
    assert store.status(source.task_id, {source.subjects[0].entity_id, caller.entity_id})["pending_discoveries"] == [original]
    assert store.scope(source.task_id)[0] == {source.subjects[0].entity_id}
