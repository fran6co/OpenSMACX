"""Explicit owner inventories are fresh evidence, not execution permissions."""

from copy import deepcopy
from dataclasses import replace
import json
from pathlib import Path

import pytest

from match_recovery.driver import working_state
from match_recovery.locking import FileLease, file_lock
from match_recovery.model import Origin
from match_recovery.routing_owners import RoutingOwners, validate_snapshot
from match_recovery.task_routes import route_task, target_reference
from match_recovery.tasks import Task, TaskStore
from test_dispatch import batch
from test_source_configuration import configured
from test_task_source import git, project


@pytest.fixture
def ownership(batch):
    root, plan = batch
    assignments = [{"root": row["root"], "task": row["task"], "branch": "refs/heads/" + row["task"]}
                   for row in plan["jobs"]]
    return root, assignments, RoutingOwners(root, assignments)


def children(ownership):
    return [Path(row["root"]) for row in ownership[1]]


def discover(root, name, request="finding"):
    store = TaskStore(root)
    store.discover(name, request_id=request, author="worker", hypothesis="Synthetic ownership question",
                   affects=[store.read(name).subjects[0].entity_id], evidence=["Synthetic, no recovery proof"],
                   expected_revision=len(store.decisions(name)))


def test_all_managed_owners_and_every_replica_are_visible_without_writes(ownership):
    root, _, inventory = ownership
    source, owner = children(ownership)
    before = {path: working_state(path) for path in (root, source, owner)}
    snapshot = inventory.snapshot(source, "owner")
    assert [(row["task"]["task_id"], row["root"], row["availability"])
            for row in snapshot["owners"]] == [("other", str(owner), "managed"), ("owner", str(source), "managed")]
    assert snapshot["incoming"] == []
    assert snapshot["conflicts"] == []
    assert snapshot["catalogs"][0]["root"] == str(root)
    assert len(snapshot["catalogs"]) == 3
    assert all(len(row["tasks"]) == 2 for row in snapshot["catalogs"])
    validate_snapshot(source, "owner", json.loads(json.dumps(snapshot)))
    assert {path: working_state(path) for path in before} == before


def test_managed_current_history_wins_over_root_prefix_and_snapshot_refreshes(ownership):
    _, _, inventory = ownership
    source, owner = children(ownership)
    before = inventory.snapshot(source, "owner")
    discover(owner, "other")
    after = inventory.snapshot(source, "owner")
    selected = next(row for row in after["owners"] if row["task"]["task_id"] == "other")
    assert selected["task"] == target_reference(TaskStore(owner), "other")
    assert selected["task"]["history_head"] is not None
    with pytest.raises(ValueError, match="snapshot differs"):
        validate_snapshot(source, "owner", before)
    validate_snapshot(source, "owner", after)


def test_native_managed_scope_expansion_is_visible_without_rewriting_root_task(ownership):
    root, _, inventory = ownership
    source, owner = children(ownership)
    store = TaskStore(owner)
    original = TaskStore(root).scope("other")
    extra = Origin("a" * 64, "custom:offset", 48)
    store.expand("other", request_id="new-member", author="coordinator", subjects=(extra,),
                 editable_files=["code/other.hpp"], evidence=["Synthetic native class insight"], expected_revision=0)
    snapshot = inventory.snapshot(source, "owner")
    row = next(item for item in snapshot["owners"] if item["task"]["task_id"] == "other")
    assert extra.entity_id in row["subjects"] and "code/other.hpp" in row["editable_files"]
    assert TaskStore(root).scope("other") == original
    validate_snapshot(source, "owner", snapshot)


def test_native_cross_checkout_route_reaches_owner_with_complete_original_finding(ownership):
    root, _, inventory = ownership
    source, owner = children(ownership)
    store, target = TaskStore(source), TaskStore(owner)
    source_task, owner_task = store.read("owner"), target.read("other")
    affects = [source_task.subjects[0].entity_id, owner_task.subjects[0].entity_id]
    store.discover("owner", request_id="cross-owner", author="worker", hypothesis="Synthetic cross-owner question",
                   affects=affects, evidence=["Retain full finding"], expected_revision=0)
    finding, = store.journal.read("task:owner")
    owner_before = target.decisions("other"), target.scope("other"), working_state(owner)
    route_task(source, "owner", {"request_id": "request-other", "expected_revision": 1,
               "route": {"kind": "discovery_routed", "author": source_task.author,
                         "discovery_id": "cross-owner", "discovery_event_id": finding.event_id,
                         "target": target_reference(target, "other"), "subjects": [owner_task.subjects[0].entity_id],
                         "evidence": ["Ask existing owner; do not activate or resolve"]}}, target_root=owner)
    snapshot = inventory.snapshot(owner, "other")
    incoming, = snapshot["incoming"]
    assert incoming["discovery"] == finding.payload and set(incoming["discovery"]["affects"]) == set(affects)
    assert incoming["source_task"] == target_reference(store, "owner")
    assert store.status("owner", store.scope("owner")[0])["pending_discoveries"] == [finding.payload]
    assert (target.decisions("other"), target.scope("other"), working_state(owner)) == owner_before
    assert TaskStore(root).decisions("owner") == ()
    validate_snapshot(owner, "other", snapshot)


def test_reserved_and_unassigned_owners_remain_unavailable(ownership):
    root, assignments, _ = ownership
    source, owner = children(ownership)
    inventory = RoutingOwners(root, assignments[:1], [{"root": str(owner), "tasks": ["other"]}])
    snapshot = inventory.snapshot(source, "owner")
    other = next(row for row in snapshot["owners"] if row["task"]["task_id"] == "other")
    assert other["availability"] == "reserved" and other["root"] == str(owner)
    validate_snapshot(source, "owner", snapshot)
    unassigned = RoutingOwners(root, assignments[:1]).snapshot(source, "owner")
    other = next(row for row in unassigned["owners"] if row["task"]["task_id"] == "other")
    assert other["availability"] == "unassigned" and other["root"] == str(root)
    assert str(owner) not in {row["root"] for row in unassigned["catalogs"]}


def test_absent_assignment_keeps_root_owner_unassigned(ownership):
    root, assignments, _ = ownership
    source, _ = children(ownership)
    rows = deepcopy(assignments)
    rows[1]["root"] = str(root / "build/absent-retired")
    snapshot = RoutingOwners(root, rows).snapshot(source, "owner")
    other = next(row for row in snapshot["owners"] if row["task"]["task_id"] == "other")
    assert other["availability"] == "unassigned" and other["root"] == str(root)
    assert len(snapshot["catalogs"]) == 2


@pytest.mark.parametrize("damage", ["identity", "fork", "stale_managed"])
def test_conflicting_or_newer_replica_cannot_silently_replace_managed_owner(ownership, damage):
    root, _, inventory = ownership
    source, owner = children(ownership)
    if damage == "identity":
        task = TaskStore(root).read("other")
        (root / "recovery/tasks/other.json").write_text(json.dumps(replace(task, intent="Replaced owner").to_dict()))
    elif damage == "fork":
        discover(root, "other", "root-fork")
        discover(owner, "other", "worker-fork")
    else:
        discover(root, "other", "newer-than-managed")
    with pytest.raises(ValueError, match="replicas conflict|stale task history"):
        inventory.snapshot(source, "owner")


@pytest.mark.parametrize("overlap", ["subject", "file"])
def test_ownership_conflicts_include_unassigned_tasks_in_all_explicit_stores(ownership, overlap):
    _, _, inventory = ownership
    source, owner = children(ownership)
    store = TaskStore(owner)
    task = store.read("other")
    intruder = Task("intruder", task.target, task.author, "Conflicting synthetic owner", task.base_revision,
                    task.subjects if overlap == "subject" else (Origin("a" * 64, "custom:offset", 48),),
                    task.editable_files if overlap == "file" else ("code/new.cc",), ("Synthetic conflict",))
    store.create(intruder)
    before = {path: working_state(path) for path in (source, owner)}
    snapshot = inventory.snapshot(source, "owner")
    assert snapshot["conflicts"] == [{"tasks": ["intruder", "other"],
                                     "subjects": [task.subjects[0].entity_id] if overlap == "subject" else [],
                                     "editable_files": list(task.editable_files) if overlap == "file" else []}]
    rows = {row["task"]["task_id"]: row for row in snapshot["owners"]}
    assert rows["other"]["subjects"] == sorted(store.scope("other")[0])
    assert rows["intruder"]["subjects"] == sorted(store.scope("intruder")[0])
    assert rows["intruder"]["availability"] == "unassigned"
    # The independent third owner remains visible and disjoint. Actual route
    # choices must exclude conflicting owners; this inventory grants none.
    assert rows["owner"]["availability"] == "managed"
    assert "owner" not in {name for row in snapshot["conflicts"] for name in row["tasks"]}
    validate_snapshot(source, "owner", snapshot)
    assert {path: working_state(path) for path in before} == before


def test_conflicts_group_by_exact_owner_set_and_cannot_be_hidden(ownership):
    _, _, inventory = ownership
    source, owner = children(ownership)
    store = TaskStore(owner)
    task = store.read("other")
    for name, subjects in (("first", task.subjects),
                           ("second", (Origin("a" * 64, "custom:offset", 48),))):
        store.create(Task(name, task.target, task.author, "Synthetic historical ownership overlap",
                          task.base_revision, subjects, task.editable_files, ("Keep this ambiguity visible",)))
    snapshot = inventory.snapshot(source, "owner")
    assert snapshot["conflicts"] == [
        {"tasks": ["first", "other"], "subjects": [task.subjects[0].entity_id], "editable_files": []},
        {"tasks": ["first", "other", "second"], "subjects": [], "editable_files": list(task.editable_files)},
    ]
    validate_snapshot(source, "owner", snapshot)
    for damage in ([], snapshot["conflicts"][:1], [{**snapshot["conflicts"][0], "tasks": ["first"]}]):
        with pytest.raises(ValueError, match="snapshot differs"):
            validate_snapshot(source, "owner", {**snapshot, "conflicts": damage})


def test_bridge_detects_new_disjoint_tasks_in_otherwise_unchanged_explicit_catalog(ownership):
    _, _, inventory = ownership
    source, owner = children(ownership)
    before = inventory.snapshot(source, "owner")
    store = TaskStore(owner)
    task = store.read("other")
    store.create(Task("new-owner", task.target, task.author, "New synthetic owner", task.base_revision,
                      (Origin("a" * 64, "custom:offset", 48),), ("code/new.cc",), ("New task",)))
    with pytest.raises(ValueError, match="snapshot differs"):
        validate_snapshot(source, "owner", before)
    assert len(inventory.snapshot(source, "owner")["owners"]) == 3


def test_catalog_mutation_during_incoming_derivation_refuses(ownership, monkeypatch):
    from match_recovery import routing_owners
    _, _, inventory = ownership
    source, owner = children(ownership)
    original = routing_owners.incoming_routes

    def changed(*args, **kwargs):
        result = original(*args, **kwargs)
        discover(owner, "other")
        return result

    monkeypatch.setattr(routing_owners, "incoming_routes", changed)
    with pytest.raises(ValueError, match="catalogs changed"):
        inventory.snapshot(source, "owner")


@pytest.mark.parametrize("damage", ["missing_owner", "missing_catalog", "subjects", "incoming", "reference"])
def test_bridge_recomputes_owner_rows_instead_of_trusting_payload(ownership, damage):
    _, _, inventory = ownership
    source, _ = children(ownership)
    snapshot = inventory.snapshot(source, "owner")
    if damage == "missing_owner":
        snapshot["owners"].pop()
    elif damage == "missing_catalog":
        snapshot["catalogs"][0]["tasks"].pop()
    elif damage == "subjects":
        snapshot["owners"][0]["subjects"] = []
    elif damage == "incoming":
        snapshot["incoming"] = [{"invented": "request"}]
    else:
        snapshot["owners"][0]["task"]["base_revision"] = "0" * 40
    with pytest.raises(ValueError):
        validate_snapshot(source, "owner", snapshot)


def test_availability_remains_non_authoritative_controller_input(ownership):
    _, _, inventory = ownership
    source, _ = children(ownership)
    snapshot = inventory.snapshot(source, "owner")
    snapshot["owners"][0]["availability"] = "reserved"
    validate_snapshot(source, "owner", snapshot)
    # Bridge freshness validation is deliberately not a route publication grant.
    assert inventory.snapshot(source, "owner")["owners"][0]["availability"] == "managed"


def test_legitimate_source_edits_do_not_masquerade_as_changed_ownership(ownership):
    _, _, inventory = ownership
    source, owner = children(ownership)
    snapshot = inventory.snapshot(source, "owner")
    for path, name in ((source, "owner"), (owner, "other")):
        file = path / f"code/{name}.cc"
        file.write_text(file.read_text() + "// Synthetic local progress\n")
    assert inventory.snapshot(source, "owner") == snapshot
    validate_snapshot(source, "owner", snapshot)


@pytest.mark.parametrize("mutation", ["source", "header", "newfile", "ignored"])
def test_external_epoch_detects_peer_source_changes_that_owner_snapshot_cannot(ownership, mutation):
    root, _, inventory = ownership
    source, owner = children(ownership)
    if mutation == "ignored":
        (owner / ".gitignore").write_text("build/\ncode/hidden.cc\n")
        (owner / "code/hidden.cc").write_text("// Ignored source still requires preservation\n")
    snapshot = inventory.snapshot(source, "owner")
    before = inventory.external_epoch(source)
    assert set(before) == {root, owner} and source not in before
    if mutation == "source":
        path = owner / "code/other.cc"
    elif mutation == "header":
        path = owner / "code/owner.hpp"
    elif mutation == "newfile":
        path = owner / "code/new.hpp"
    else:
        path = owner / "code/hidden.cc"
    path.write_text((path.read_text() if path.exists() else "") + "// Unauthorized external mutation\n")
    assert inventory.snapshot(source, "owner") == snapshot
    after = inventory.external_epoch(source)
    assert after[root] == before[root] and after[owner] != before[owner]
    assert path.relative_to(owner).as_posix() in after[owner]["state"]["files"]
    assert path.read_text().endswith("// Unauthorized external mutation\n")


def test_external_epoch_includes_reserved_and_absent_assignments_without_implicit_checkout_scan(ownership):
    root, assignments, _ = ownership
    source, owner = children(ownership)
    missing = root / "build/absent-managed"
    inventory = RoutingOwners(root, [assignments[0], {"root": str(missing), "task": "retired", "branch": "refs/heads/retired"}],
                              [{"root": str(owner), "tasks": ["other"]}])
    before = inventory.external_epoch(source)
    assert set(before) == {root, owner, missing} and before[missing] is None
    (owner / "code/other.cc").write_text("// Preserve unauthorized reserved source change\n")
    assert inventory.external_epoch(source)[owner] != before[owner]
    missing.mkdir()
    # A returned empty/non-worktree path cannot masquerade as the prior absence.
    with pytest.raises((ValueError, RuntimeError)):
        inventory.external_epoch(source)


def test_external_epoch_is_read_only_and_ignores_current_worker_source_progress(ownership):
    root, _, inventory = ownership
    source, owner = children(ownership)
    before_files = {path: working_state(path) for path in (root, source, owner)}
    before = inventory.external_epoch(source)
    assert {path: working_state(path) for path in before_files} == before_files
    (source / "code/owner.cc").write_text("// Authorized local worker edit\n")
    assert inventory.external_epoch(source) == before
    unlisted = root / "build/unlisted-checkout"
    unlisted.mkdir()
    (unlisted / "unrelated").write_text("Not an explicit ownership store")
    assert inventory.external_epoch(source) == before


def test_external_epoch_refuses_alias_of_absent_assignment(ownership):
    root, assignments, _ = ownership
    source, owner = children(ownership)
    missing = root / "build/absent-managed"
    rows = [assignments[0], {"root": str(missing), "task": "retired", "branch": "refs/heads/retired"}]
    inventory = RoutingOwners(root, rows)
    assert inventory.external_epoch(source)[missing] is None
    missing.symlink_to(owner, target_is_directory=True)
    with pytest.raises(ValueError, match="canonical explicit checkout"):
        inventory.external_epoch(source)


def test_actual_held_lease_can_be_exposed_and_is_rechecked(ownership):
    root, assignments, _ = ownership
    source, _ = children(ownership)
    with file_lock(source / "build/recovery/driver.lock", timeout=0) as lease:
        inventory = RoutingOwners(root, assignments, leases={source: lease})
        assert inventory.leases[source] is lease
        inventory.snapshot(source, "owner")
        with pytest.raises(TimeoutError):
            with file_lock(source / "build/recovery/driver.lock", timeout=0):
                pass
    with pytest.raises(ValueError, match="active local file lease"):
        inventory.snapshot(source, "owner")


def test_fabricated_lease_refuses(ownership):
    root, assignments, _ = ownership
    source, _ = children(ownership)
    inventory = RoutingOwners(root, assignments, leases={source: FileLease(source / "build/recovery/driver.lock")})
    with pytest.raises(ValueError, match="active local file lease"):
        inventory.snapshot(source, "owner")


@pytest.mark.parametrize("damage", ["alias", "wrong_branch", "foreign_repo", "duplicate", "reserved_duplicate"])
def test_assignment_boundaries_refuse(ownership, tmp_path, damage):
    root, assignments, _ = ownership
    source, owner = children(ownership)
    rows, reservations = deepcopy(assignments), []
    if damage == "alias":
        alias = root / "build/owner-alias"
        alias.symlink_to(owner, target_is_directory=True)
        rows[1]["root"] = str(alias)
    elif damage == "wrong_branch":
        rows[1]["branch"] = "refs/heads/not-the-assigned-branch"
    elif damage == "foreign_repo":
        foreign = tmp_path / "foreign"
        foreign.mkdir()
        git(foreign, "init", "--initial-branch=other")
        git(foreign, "commit", "--allow-empty", "-m", "Unrelated repository")
        rows[1]["root"] = str(foreign)
    elif damage == "duplicate":
        rows.append(deepcopy(rows[0]))
    else:
        reservations = [{"root": str(owner), "tasks": ["other"]}]
    with pytest.raises(ValueError):
        RoutingOwners(root, rows, reservations).snapshot(source, "owner")
