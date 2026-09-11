"""Routing bridge context is opt-in evidence, never provider-owned permission."""

import json
from pathlib import Path
import sys

import pytest

from match_recovery.agent_command import _request, run_agent
from match_recovery.agent_protocol import response_schema
from match_recovery.coordination import decision_facts, validate_response
from match_recovery.source_inventory import source_snapshot
from match_recovery.tasks import Task
from test_coordination import discover, setup
from test_source_configuration import configured
from test_task_source import project


@pytest.fixture
def routing_request(setup):
    from match_recovery.coordination import routing_facts
    from match_recovery.routing_owners import RoutingOwners
    root, store, task, caller, delegation = setup
    discover(setup)
    owner = Task("caller-owner", task.target, "caller-coordinator", "Existing caller owner",
                 task.base_revision, (caller,), ("code/caller.cc",), ("Synthetic ownership",))
    store.create(owner)
    delegation = {**delegation, "route_discoveries": True}
    selected, editable = store.scope(task.task_id)
    pending = store.status(task.task_id, selected)["pending_discoveries"]
    routing = RoutingOwners(root, []).snapshot(root, task.task_id)
    # Availability is controller-supplied evidence at the bridge boundary, not
    # authority to publish or activate an owner. Native publication checks its
    # separately retained live RoutingOwners instance.
    for row in routing["owners"]:
        if row["task"]["task_id"] == owner.task_id:
            row["availability"] = "managed"
    value = {"task": task.to_dict(), "decision_revision": len(store.decisions(task.task_id)),
             "pending_discoveries": pending, "subjects": sorted(selected), "editable_files": sorted(editable),
             "delegation": delegation, "mapped_records": [], "instructions": [], "snapshot": "a" * 64,
             "decision_facts": decision_facts(source_snapshot(root), selected, editable, delegation, pending),
             "routing": routing, "route_choices": routing_facts(root, task.task_id, delegation, routing)}
    return root, store, task, owner, caller, json.loads(json.dumps(value))


def response(caller):
    return {"snapshot": "a" * 64, "action": "route", "discovery_id": "layout",
            "subjects": [], "editable_files": [], "evidence": ["Existing outside owner needs this request"],
            "binding": None, "route": {"owner": "caller-owner", "subjects": [caller.entity_id]}}


def configure_bridge(root, script):
    role = {"command": {"argv": [sys.executable, "-c", script], "timeout": 5},
            "instructions": [], "response": "stdout"}
    path = Path("recovery/agents.json")
    (root / path).write_text(json.dumps({"schema_version": 1, "artifacts": "build/recovery/agents",
                                       "worker": role, "reviewer": role, "coordinator": role}))
    return path


def test_provider_schema_has_only_nullable_owner_subject_route_not_authority_fields():
    schema = response_schema("coordinator")
    assert "route" in schema["properties"]["action"]["enum"]
    assert "route" in schema["required"]
    null, route = schema["properties"]["route"]["anyOf"]
    assert null == {"type": "null"}
    assert set(route["properties"]) == set(route["required"]) == {"owner", "subjects"}
    assert route["additionalProperties"] is False
    assert route["properties"]["subjects"]["minItems"] == 1
    assert route["properties"]["subjects"]["uniqueItems"] is True


def test_native_routing_response_keeps_scope_and_binding_fields_empty(routing_request):
    _, _, _, _, caller, _ = routing_request
    assert validate_response(response(caller), "a" * 64) is None


@pytest.mark.parametrize("damage", ["missing_route", "null_route", "owner_path", "empty_subjects", "duplicate_subjects",
                                    "invalid_identity", "unsorted", "extra_author", "extra_root", "extra_reference",
                                    "scope_subject", "scope_file", "binding", "wrong_action", "stale"])
def test_malformed_routing_response_cannot_smuggle_scope_or_target_authority(routing_request, damage):
    _, _, task, _, caller, _ = routing_request
    value = response(caller)
    if damage == "missing_route": value.pop("route")
    elif damage == "null_route": value["route"] = None
    elif damage == "owner_path": value["route"]["owner"] = "../another-checkout"
    elif damage == "empty_subjects": value["route"]["subjects"] = []
    elif damage == "duplicate_subjects": value["route"]["subjects"] *= 2
    elif damage == "invalid_identity": value["route"]["subjects"] = ["unknown"]
    elif damage == "unsorted": value["route"]["subjects"] = sorted([caller.entity_id, task.subjects[0].entity_id], reverse=True)
    elif damage == "extra_author": value["route"]["author"] = "provider"
    elif damage == "extra_root": value["route"]["root"] = "/other/checkout"
    elif damage == "extra_reference": value["route"]["reference"] = {"task_sha256": "b" * 64}
    elif damage == "scope_subject": value["subjects"] = [caller.entity_id]
    elif damage == "scope_file": value["editable_files"] = ["code/caller.cc"]
    elif damage == "binding": value["binding"] = {"entity_id": caller.entity_id}
    elif damage == "wrong_action": value["action"] = "defer"
    else: value["snapshot"] = "f" * 64
    with pytest.raises(ValueError):
        validate_response(value, "a" * 64)


def test_bridge_revalidates_live_routing_and_separate_choices(routing_request):
    root, _, task, owner, caller, value = routing_request
    assert value["route_choices"] == [{"discovery_id": "layout", "owners": [
        {"owner": owner.task_id, "subjects": [caller.entity_id]}]}]
    observed, _, editable = _request(root, "coordinator", value)
    assert observed == task and not editable
    assert "route" not in value["decision_facts"][0]["actions"]


@pytest.mark.parametrize("field", ["routing", "route_choices"])
def test_bridge_requires_routing_snapshot_and_choices_as_a_pair(routing_request, field):
    root, _, _, _, _, value = routing_request
    value.pop(field)
    with pytest.raises(ValueError, match="together"):
        _request(root, "coordinator", value)


@pytest.mark.parametrize("optin", [None, False, "true", 1])
def test_bridge_refuses_routing_context_without_explicit_boolean_permission(routing_request, optin):
    root, _, _, _, _, value = routing_request
    if optin is None: value["delegation"].pop("route_discoveries")
    else: value["delegation"]["route_discoveries"] = optin
    with pytest.raises(ValueError):
        _request(root, "coordinator", value)


@pytest.mark.parametrize("damage", ["scope", "owner", "unknown", "extra", "shape"])
def test_bridge_recomputes_routing_choices_instead_of_trusting_provider_request_data(routing_request, damage):
    root, _, task, _, _, value = routing_request
    choices = value["route_choices"]
    if damage == "scope": choices[0]["owners"][0]["subjects"] = [task.subjects[0].entity_id]
    elif damage == "owner": choices[0]["owners"][0]["owner"] = task.task_id
    elif damage == "unknown": choices[0]["owners"][0]["subjects"] = ["entity-" + "f" * 64]
    elif damage == "extra": choices[0]["owners"][0]["editable_files"] = ["code/caller.cc"]
    else: value["route_choices"] = {}
    with pytest.raises(ValueError, match="routing choices"):
        _request(root, "coordinator", value)


@pytest.mark.parametrize("damage", ["catalog_root", "owner_root", "branch", "scope", "history", "unavailable"])
def test_invalid_or_stale_routing_snapshot_refuses_before_provider(routing_request, damage):
    root, store, _, owner, caller, value = routing_request
    routing = value["routing"]
    if damage == "catalog_root": routing["catalogs"][0]["root"] = str(root.parent)
    elif damage == "owner_root": routing["owners"][0]["root"] = str(root.parent)
    elif damage == "branch": routing["catalogs"][0]["branch"] = "refs/heads/invented"
    elif damage == "scope": routing["owners"][0]["subjects"].append("entity-" + "b" * 64)
    elif damage == "history":
        store.discover(owner.task_id, request_id="new-owner-evidence", author="owner-worker",
                       hypothesis="Fresh owner history", affects=[caller.entity_id],
                       evidence=["New observation after request snapshot"], expected_revision=0)
    else:
        (root / "recovery/tasks" / (owner.task_id + ".json")).unlink()
    launched = root / "provider-launched"
    config = configure_bridge(root, "from pathlib import Path; Path('provider-launched').write_text('unsafe')")
    with pytest.raises((ValueError, OSError)):
        run_agent(root, "coordinator", config, value)
    assert not launched.exists()


def test_legacy_coordinator_request_remains_valid_without_routing_optin_or_fields(routing_request):
    root, _, task, _, _, value = routing_request
    for field in ("routing", "route_choices"):
        value.pop(field)
    value["delegation"].pop("route_discoveries")
    assert _request(root, "coordinator", value)[0] == task
    legacy = {"snapshot": "a" * 64, "action": "defer", "discovery_id": "layout",
              "subjects": [], "editable_files": [], "evidence": ["Legacy read-only decision"]}
    assert validate_response(legacy, "a" * 64) is None


def test_real_readonly_bridge_routing_response_publishes_nothing_and_explains_boundary(routing_request):
    root, store, task, owner, caller, value = routing_request
    original = store.decisions(task.task_id), store.decisions(owner.task_id), store.scope(task.task_id)
    script = ("import sys; prompt=sys.stdin.read(); "
              "assert 'full discovery stays pending' in prompt; "
              "assert 'serial controller publishes' in prompt; "
              "assert 'paths, author and target reference pins' in prompt; "
              "print(" + repr(json.dumps(response(caller))) + ")")
    config = configure_bridge(root, script)
    assert run_agent(root, "coordinator", config, value) == (0, response(caller))
    assert (store.decisions(task.task_id), store.decisions(owner.task_id), store.scope(task.task_id)) == original
