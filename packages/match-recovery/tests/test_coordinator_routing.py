"""Native serial proposal -> source journal -> peer evidence, without acceptance."""

from contextlib import ExitStack

import pytest

from match_recovery.coordination import DecisionBudget, SerialCoordinationRequired, coordinate
from match_recovery.driver import drive, working_state
from match_recovery.locking import file_lock
from match_recovery.routing_owners import RoutingOwners
from match_recovery.model import Origin
from match_recovery.tasks import TaskStore
from test_routing_owners import ownership, children
from test_dispatch import batch
from test_source_configuration import configured
from test_task_source import project


@pytest.fixture
def proposed(ownership):
    root, assignments, inventory = ownership
    source, target = children(ownership)
    store, owner = TaskStore(source), TaskStore(target)
    task, peer = store.read("owner"), owner.read("other")
    affected = sorted([task.subjects[0].entity_id, peer.subjects[0].entity_id])
    store.discover("owner", request_id="cross-owner", author="worker", hypothesis="Owned caller needs peer interpretation",
                   affects=affected, evidence=["Synthetic complete finding"], expected_revision=0)
    delegation = {"schema_version": 1, "author": task.author, "command": {"argv": ["coordinator"], "timeout": 10},
                  "subjects": [], "editable_files": [], "evidence": ["Managed request permission, not editing scope"],
                  "max_decisions": 3, "route_discoveries": True}
    return root, assignments, inventory, source, target, task, peer, delegation


def propose(request, peer):
    return {"snapshot": request["snapshot"], "action": "route", "discovery_id": "cross-owner",
            "subjects": [], "editable_files": [], "binding": None,
            "route": {"owner": peer.task_id, "subjects": [peer.subjects[0].entity_id]},
            "evidence": ["Ask the existing peer, retaining the full source finding"]}


def test_serial_coordinator_borrows_real_leases_and_delivers_request_without_acceptance(proposed):
    root, assignments, _, source, target, task, peer, delegation = proposed
    original = TaskStore(source).decisions(task.task_id)[0]
    target_before = working_state(target)
    source_scope = TaskStore(source).scope(task.task_id)
    with ExitStack() as stack:
        leases = {path: stack.enter_context(file_lock(path / "build/recovery/driver.lock", timeout=0))
                  for path in (root, source, target)}
        inventory = RoutingOwners(root, assignments, leases=leases)
        seen = []
        def execute(command, child, name, request):
            seen.append(request)
            assert request["route_choices"] == [{"discovery_id": "cross-owner", "owners": [
                {"owner": peer.task_id, "subjects": [peer.subjects[0].entity_id]}]}]
            return 0, propose(request, peer)
        output = coordinate(source, task.task_id, delegation, execute=execute, _routing=inventory, _held_lease=leases[source])
        assert output["action"] == "route" and output["published"] and not output["ready_for_collection"]
        incoming, = inventory.snapshot(target, peer.task_id)["incoming"]
        assert incoming["discovery"] == original and incoming["route"]["subjects"] == [peer.subjects[0].entity_id]
        assert len(seen) == 1 and all(lease.active for lease in leases.values())
    assert TaskStore(source).scope(task.task_id) == source_scope
    assert TaskStore(source).status(task.task_id, source_scope[0])["pending_discoveries"] == [original]
    assert working_state(target) == target_before
    assert TaskStore(root).decisions(task.task_id) == ()


def test_restricted_proposal_and_serial_publication_share_one_budget(proposed):
    _, _, inventory, source, _, task, peer, delegation = proposed
    budget = DecisionBudget(3)
    seen = []
    def execute(command, child, name, request):
        seen.append(request["snapshot"])
        return 0, propose(request, peer)
    before = working_state(source)
    with pytest.raises(SerialCoordinationRequired):
        coordinate(source, task.task_id, delegation, execute=execute, _routing=inventory, _budget=budget,
                   allowed_actions=("investigate", "investigate_local", "defer"))
    assert budget.used == 1 and working_state(source) == before
    assert coordinate(source, task.task_id, delegation, execute=execute, _routing=inventory, _budget=budget)["published"]
    assert budget.used == 2 and len(seen) == 2 and seen[0] != seen[1]
    with pytest.raises(ValueError, match="available managed owner"):
        coordinate(source, task.task_id, delegation, execute=execute, _routing=inventory, _budget=budget)
    assert budget.used == 3 and len(TaskStore(source).decisions(task.task_id)) == 2
    assert coordinate(source, task.task_id, delegation, execute=lambda *args: pytest.fail("budget exhausted"),
                      _routing=inventory, _budget=budget)["action"] == "budget_exhausted"


def test_known_owner_subset_can_be_routed_without_hiding_unknown_remainder(proposed):
    _, _, inventory, source, _, task, peer, delegation = proposed
    store = TaskStore(source)
    unknown = Origin(task.subjects[0].image_sha256, task.subjects[0].address_space, 0x999).entity_id
    store.discover(task.task_id, request_id="partly-known", author="worker", hypothesis="Known peer plus unbound dependency",
                   affects=[unknown, peer.subjects[0].entity_id], evidence=["Unknown remainder must survive"], expected_revision=1)
    def execute(command, child, name, request):
        answer = propose(request, peer)
        answer["discovery_id"] = "partly-known"
        return 0, answer
    result = coordinate(source, task.task_id, delegation, execute=execute, _routing=inventory)
    assert result["published"] and not result["ready_for_collection"]
    pending = store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
    assert len(pending) == 2 and unknown in pending[1]["affects"]
    assert store.decisions(task.task_id)[-1]["subjects"] == [peer.subjects[0].entity_id]


@pytest.mark.parametrize("damage", ["missing_permission", "missing_context", "reserved", "retired", "own_subject", "unknown_owner", "stale_peer"])
def test_unavailable_or_stale_owners_never_publish(proposed, damage):
    root, assignments, inventory, source, target, task, peer, delegation = proposed
    if damage == "missing_permission": delegation = {key: value for key, value in delegation.items() if key != "route_discoveries"}
    if damage == "missing_context": inventory = None
    if damage == "reserved":
        inventory = RoutingOwners(root, assignments[:1], [{"root": str(target), "tasks": [peer.task_id]}])
    if damage == "retired":
        inventory = RoutingOwners(root, assignments, retired={peer.task_id})
    def execute(command, child, name, request):
        output = propose(request, peer)
        if damage == "own_subject": output["route"]["subjects"] = [task.subjects[0].entity_id]
        if damage == "unknown_owner": output["route"]["owner"] = "unknown-owner"
        if damage == "stale_peer":
            TaskStore(target).discover(peer.task_id, request_id="changed", author="other-worker", hypothesis="New peer evidence",
                                       affects=[peer.subjects[0].entity_id], evidence=["Concurrent evidence"], expected_revision=0)
        return 0, output
    with pytest.raises(ValueError):
        coordinate(source, task.task_id, delegation, execute=execute, _routing=inventory)
    assert len(TaskStore(source).decisions(task.task_id)) == 1


def test_serial_driver_preserves_routed_source_and_delivers_complete_request_to_owner_worker(proposed):
    _, _, inventory, source, target, task, peer, delegation = proposed
    source_seen = []
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10}, "check": {"argv": ["check"], "timeout": 10}}
    worker = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}
    def source_execute(command, child, name, request):
        if command.argv == ("context",): return 0, {}
        if command.argv == ("check",): return 1, {"result": "fail"}
        assert command.argv == ("coordinator",)
        source_seen.append(request)
        answer = propose(request, peer)
        if len(source_seen) > 1: answer.update(action="defer", route=None)
        return 0, answer
    outcome = drive(source, task.task_id, config, worker, iterations=1,
                    coordinator_configuration=delegation, execute=source_execute, _routing=inventory)
    assert outcome["stop_reason"] == "coordination_defer" and outcome["worker_rounds"] == 0
    original = TaskStore(source).decisions(task.task_id)[0]
    source_before = working_state(source)
    workers = []
    def owner_execute(command, child, name, request):
        if command.argv == ("context",): return 0, {}
        if command.argv == ("check",): return 1, {"result": "fail"}
        assert command.argv == ("worker",)  # No local finding needs fake fabrication.
        workers.append(request)
        incoming, = request["context"]["owner_requests"]
        assert incoming["discovery"] == original
        assert set(request["editable_files"]) == set(TaskStore(target).scope(peer.task_id)[1])
        return 0, {"discoveries": []}
    owner_delegation = {**delegation, "author": peer.author}
    result = drive(target, peer.task_id, config, worker, iterations=1,
                   coordinator_configuration=owner_delegation, execute=owner_execute, _routing=inventory)
    assert len(workers) == 1 and not result["ready_for_collection"]
    assert working_state(source) == source_before
    assert TaskStore(target).decisions(peer.task_id) == ()
    assert TaskStore(source).status(task.task_id, TaskStore(source).scope(task.task_id)[0])["pending_discoveries"]
