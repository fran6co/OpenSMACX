"""Serial routing context is delivered independently of pending-source status."""

import json

import pytest

import match_recovery.campaign as campaign
from match_recovery.coordination import SerialCoordinationRequired
from match_recovery.model import Origin
from match_recovery.task_routes import route_task, target_reference
from match_recovery.tasks import Task, TaskStore
from test_campaign import choose, configured_campaign, compiler_project, project
from test_campaign_resume import stopped
from test_task_source import git


@pytest.mark.parametrize("routed", [False, True])
def test_serial_driver_receives_routing_without_pending_and_keeps_unresolved_route_sources(configured_campaign, monkeypatch, routed):
    import match_recovery.investigation_round as investigation

    root, config = configured_campaign
    config.update(max_batches=1, coordinator="recovery/coordinator.json")
    coordinator = {"schema_version": 1, "author": "coordinator", "subjects": [], "editable_files": [],
                   "max_decisions": 2, "route_discoveries": True, "evidence": ["Explicit owner-routing control"],
                   "command": json.loads((root / "recovery/planner.json").read_text())["command"]}
    (root / config["coordinator"]).write_text(json.dumps(coordinator))
    store = TaskStore(root)
    source = store.read("owner")
    if routed:
        original = source.subjects[0]
        caller = Origin(original.image_sha256, original.address_space, 0x401010)
        target = Task("zcaller", source.target, source.author, "Recover caller owner", source.base_revision,
                      (caller,), ("code/caller.cc",), ("Existing caller identity",))
        store.create(target)
        store.discover("owner", request_id="outside", author="worker", hypothesis="Original caller still requires repair",
                       affects=[caller.entity_id], evidence=["Known original caller"], expected_revision=0)
        event = store.journal.read("task:owner")[0]
        route_task(root, "owner", {"request_id": "route-outside", "expected_revision": 1,
                   "route": {"kind": "discovery_routed", "author": source.author, "discovery_id": "outside",
                             "discovery_event_id": event.event_id, "target": target_reference(store, "zcaller"),
                             "subjects": [caller.entity_id], "evidence": ["Typed help request remains unresolved"]}})
    git(root, "add", "recovery")
    git(root, "commit", "-m", "Authorize synthetic routing workflow")
    root, config, directory, child = stopped((root, config))
    before = TaskStore(child).decisions("owner")
    seen = []

    def dispatch(current, plan, **kwargs):
        return {"coordinator_changed": False, "jobs": [{"task": row["task"], "root": row["root"],
                "scope_intact": True, "timed_out": False, "returncode": 3} for row in plan["jobs"]]}

    def drive(current, name, *args, **kwargs):
        assert current == child and name == "owner"
        routing = kwargs["_routing"]
        context = routing.snapshot(current, name)
        assert {item["root"] for item in context["catalogs"]} == {str(root), str(child)}
        assert kwargs["coordinator_configuration"]["route_discoveries"] is True
        seen.append(context)
        return {"stop_reason": "no_source_progress", "ready_for_collection": False}

    monkeypatch.setattr(campaign, "dispatch", dispatch)
    monkeypatch.setattr(campaign, "drive", drive)
    if routed:
        def serial(*args, **kwargs):
            assert kwargs["_routing"] is not None
            raise SerialCoordinationRequired("Controlled valid publishing proposal requires serial coordination")
        monkeypatch.setattr(investigation, "authorize", serial)
        monkeypatch.setattr(campaign, "_remove_integrated", lambda *args: pytest.fail("Unresolved routed source cannot be cleaned"))
        monkeypatch.setattr(campaign, "prepare_integration", lambda *args, **kwargs: pytest.fail("Unresolved routed source cannot integrate"))
    result = campaign.campaign(root, config, resume=directory, execute=choose)
    assert len(seen) == 1 and not result["full_project_accepted"]
    if routed:
        assert child.exists() and TaskStore(child).decisions("owner") == before
        assert result["batches"][0]["pending"][0]["reason"] == "routed_discovery_pending"
        assert not list(directory.glob("*retired-owner.json"))
    else:
        assert not child.exists()  # No pending finding was invented by the opt-in.
