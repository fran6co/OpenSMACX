"""Native routing guards external source, not only task/history references."""

import subprocess

import pytest

from match_recovery.coordination import coordinate
from match_recovery.driver import drive, working_state
from match_recovery.tasks import TaskStore
from test_coordinator_routing import proposed, propose
from test_routing_owners import ownership
from test_dispatch import batch
from test_source_configuration import configured
from test_task_source import project, git


@pytest.mark.parametrize("location", ["peer", "coordinator_root", "ignored_peer"])
@pytest.mark.parametrize("outcome", ["success", "nonzero", "exception"])
def test_route_provider_cannot_edit_external_source_even_with_valid_unchanged_owner_metadata(proposed, location, outcome):
    root, _, inventory, source, target, task, peer, delegation = proposed
    if location == "coordinator_root":
        damaged = root / "code/other.cc"
    elif location == "ignored_peer":
        damaged = target / "code/ignored.hpp"
        damaged.write_text("struct External {};\n")
        # A per-checkout .gitignore is fixture setup, not the tested mutation.
        (target / ".gitignore").write_text("build/\ncode/ignored.hpp\n")
        assert git(target, "check-ignore", "code/ignored.hpp") == "code/ignored.hpp"
    else:
        damaged = target / "code/other.cc"
    old = damaged.read_bytes()
    source_before = working_state(source)
    source_events = TaskStore(source).decisions(task.task_id)
    peer_events = TaskStore(target).decisions(peer.task_id)
    metadata = inventory.snapshot(source, task.task_id)
    calls = []
    def execute(command, child, name, request):
        calls.append(command.argv)
        damaged.write_bytes(old + b"\n// preserved unauthorized external edit\n")
        assert inventory.snapshot(source, task.task_id) == metadata
        if outcome == "exception":
            raise subprocess.TimeoutExpired("synthetic coordinator", 1)
        return (0 if outcome == "success" else 3), propose(request, peer)
    with pytest.raises(ValueError):
        coordinate(source, task.task_id, delegation, execute=execute, _routing=inventory)
    assert len(calls) == 1
    assert damaged.read_bytes() == old + b"\n// preserved unauthorized external edit\n"
    assert working_state(source) == source_before
    assert TaskStore(source).decisions(task.task_id) == source_events
    assert TaskStore(target).decisions(peer.task_id) == peer_events


@pytest.mark.parametrize("hook", ["context", "check", "worker"])
@pytest.mark.parametrize("outcome", ["success", "exception"])
def test_owner_driver_refuses_peer_cpp_changes_during_readonly_or_worker_hooks(proposed, hook, outcome):
    _, _, inventory, source, target, task, peer, delegation = proposed
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10}, "check": {"argv": ["check"], "timeout": 10}}
    worker = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}
    damaged = source / "code/owner.cc"
    original = damaged.read_bytes()
    before = TaskStore(source).decisions(task.task_id), TaskStore(target).decisions(peer.task_id)
    calls = []
    def execute(command, child, name, request):
        role = command.argv[0]
        calls.append(role)
        if role == hook:
            damaged.write_bytes(original + b"\n// keep rejected external WIP\n")
            if outcome == "exception":
                raise subprocess.TimeoutExpired("synthetic " + hook, 1)
        if role == "context": return 0, {}
        if role == "check": return 1, {"result": "fail"}
        assert role == "worker"
        return 0, {"discoveries": []}
    with pytest.raises(ValueError):
        drive(target, peer.task_id, config, worker, iterations=1,
              coordinator_configuration={**delegation, "author": peer.author}, execute=execute, _routing=inventory)
    assert calls[-1] == hook
    assert damaged.read_bytes() == original + b"\n// keep rejected external WIP\n"
    assert (TaskStore(source).decisions(task.task_id), TaskStore(target).decisions(peer.task_id)) == before


def test_external_epoch_does_not_freeze_authorized_current_worker_cpp(proposed):
    root, _, inventory, source, target, task, peer, delegation = proposed
    config = {"schema_version": 1, "target": task.target,
              "context": {"argv": ["context"], "timeout": 10}, "check": {"argv": ["check"], "timeout": 10}}
    worker = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}
    before = {path: working_state(path) for path in (root, source)}
    changed = target / "code/other.cc"
    original = changed.read_bytes()
    calls = []
    def execute(command, child, name, request):
        role = command.argv[0]
        if role == "context": return 0, {}
        if role == "check": return 1, {"result": "fail"}
        assert role == "worker" and child == target
        calls.append(role)
        changed.write_bytes(original + b"\n// Authorized current ownership work\n")
        return 0, {"discoveries": []}
    result = drive(target, peer.task_id, config, worker, iterations=1,
                   coordinator_configuration={**delegation, "author": peer.author}, execute=execute, _routing=inventory)
    assert calls == ["worker"] and result["worker_rounds"] == 1
    assert result["stop_reason"] == "iteration_budget_exhausted" and not result["ready_for_collection"]
    assert changed.read_bytes() == original + b"\n// Authorized current ownership work\n"
    assert {path: working_state(path) for path in before} == before
