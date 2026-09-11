"""Managed routing sees inactive peers and borrows real controller leases."""

from copy import deepcopy
import json
from pathlib import Path

import pytest

from match_recovery.campaign import _resume_round, _routing_owners
from match_recovery.campaign_peers import peer_epoch
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.tasks import TaskStore
from test_dispatch import batch
from test_source_configuration import configured
from test_task_source import git, project


def test_peer_freshness_exports_only_actual_acquired_live_tokens(batch):
    root, plan = batch
    peer = Path(plan["jobs"][1]["root"])
    with peer_epoch(root, "owner", plan["jobs"]) as fresh:
        assert set(fresh.leases) == {peer}
        token = fresh.leases[peer]
        with file_lock(peer / "build/recovery/driver.lock", timeout=0, held=token) as borrowed:
            assert borrowed is token and token.active
        fresh()
    assert not token.active
    with pytest.raises(ValueError):
        with file_lock(peer / "build/recovery/driver.lock", held=token):
            pytest.fail("Released peer lease cannot be borrowed")


@pytest.mark.parametrize("mutate", [False, True])
def test_restricted_planning_supplies_all_assignments_and_active_source_peer_leases(batch, monkeypatch, mutate):
    import match_recovery.investigation_round as investigation

    root, original = batch
    owner, peer = (Path(job["root"]) for job in original["jobs"])
    store = TaskStore(owner)
    store.discover("owner", request_id="outside", author="worker", hypothesis="Existing other owner needs help",
                   affects=[store.read("other").subjects[0].entity_id], evidence=["Known other original"], expected_revision=0)
    assignments = [{"root": job["root"], "task": job["task"], "branch": "refs/heads/" + job["task"]}
                   for job in original["jobs"]]
    plan = {**original, "jobs": original["jobs"][:1]}  # Peer deliberately inactive, not absent.
    before_plan = deepcopy(plan)
    profiles = {"resources": json.loads((root / "recovery/resources.json").read_text()),
                "driver": json.loads((root / "recovery/driver.json").read_text()),
                "coordinator": {"schema_version": 1, "author": "coordinator", "route_discoveries": True,
                                "command": {"argv": ["coordinate"], "timeout": 10}, "max_decisions": 1,
                                "subjects": [], "editable_files": [], "evidence": ["Explicit route decisions only"]}}
    seen = []

    def coordinate(child, name, delegation, **kwargs):
        routing, source_lease = kwargs["_routing"], kwargs["_held_lease"]
        assert child == owner and name == "owner" and delegation["route_discoveries"] is True
        assert set(kwargs["allowed_actions"]) == {"investigate", "investigate_local", "defer"}
        assert set(routing.leases) == {root, peer}
        with file_lock(owner / "build/recovery/driver.lock", held=source_lease) as borrowed:
            assert borrowed is source_lease and source_lease.active
        context = routing.snapshot(owner, "owner")
        assert {row["root"] for row in context["catalogs"]} == {str(root), str(owner), str(peer)}
        other = next(row for row in context["owners"] if row["task"]["task_id"] == "other")
        assert other["root"] == str(peer) and other["availability"] == "managed"
        seen.append(context)
        if mutate:
            path = peer / "code/other.cc"
            path.write_text(path.read_text() + "// unauthorized inactive peer mutation retained\n")
        return {"action": "defer", "published": False, "ready_for_collection": False}

    monkeypatch.setattr(investigation, "coordinate", coordinate)
    directory = root / "build/routing-resume"
    directory.mkdir()
    before = working_state(root)

    def fresh(): assert working_state(root) == before

    with file_lock(root / "build/recovery/driver.lock", timeout=0) as lease:
        def invoke():
            return _resume_round(root, plan, profiles, directory, 0, git(root, "rev-parse", "HEAD"),
                                 execute=lambda *args: pytest.fail("No provider is necessary for lease plumbing control"),
                                 fresh=fresh, announce=lambda *args, **kwargs: None, lease=lease,
                                 assignments=assignments, scope_reservations=[])
        if mutate:
            with pytest.raises(ValueError, match="peer state"): invoke()
        else:
            report, decisions, deferred = invoke()
            assert report is None and deferred == {"owner"} and decisions[0]["status"] == "deferred"
    assert len(seen) == 1 and plan == before_plan
    assert len(store.decisions("owner")) == 1 and not list((root / "build/signals").iterdir())


def test_routing_opt_in_needs_complete_assignments_and_absence_changes_nothing(batch):
    root, _ = batch
    assert _routing_owners(root, {}, None, [], None, {}) is None
    with pytest.raises(ValueError, match="all original"):
        _routing_owners(root, {"coordinator": {"route_discoveries": True}}, None, [], None, {})


def test_reserved_owner_descriptors_are_forwarded_without_borrowing_reservation_leases(batch, monkeypatch):
    import match_recovery.routing_owners as owners

    root, plan = batch
    assignments = [{"root": row["root"], "task": row["task"], "branch": "refs/heads/" + row["task"]}
                   for row in plan["jobs"]]
    reservations = [{"root": str(root / "build/parked"), "tasks": ["parked-owner"]}]
    seen = []

    def construct(current, managed, **kwargs):
        assert current == root and managed == assignments and kwargs["reservations"] == reservations
        assert root / "build/parked" not in kwargs["leases"]
        assert all(token.active for token in kwargs["leases"].values())
        seen.append(kwargs)
        return "routing-construction-control"

    monkeypatch.setattr(owners, "RoutingOwners", construct)
    with file_lock(root / "build/recovery/driver.lock", timeout=0) as lease, peer_epoch(root, "owner", assignments) as fresh:
        assert _routing_owners(root, {"coordinator": {"route_discoveries": True}}, assignments,
                               reservations, lease, fresh.leases) == "routing-construction-control"
    assert len(seen) == 1
