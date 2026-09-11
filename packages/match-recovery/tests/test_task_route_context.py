"""Derived routing context never transfers task ownership or accepts source."""

from copy import deepcopy
from dataclasses import replace
import json
from types import SimpleNamespace

import pytest

from match_recovery.driver import working_state
from match_recovery.planning import _context
from match_recovery.task_routes import incoming_routes, route_task, target_reference
from match_recovery.task_source import task_context
from match_recovery.tasks import Task, TaskStore
from test_coordination import setup
from test_source_configuration import configured
from test_task_source import git, project


@pytest.fixture
def owners(setup):
    root, store, source, caller, _ = setup
    owner = Task("caller-owner", source.target, source.author, "Investigate the existing caller owner",
                 source.base_revision, (caller,), ("code/caller.cc",), ("Synthetic existing ownership",))
    store.create(owner)
    store.discover(source.task_id, request_id="cross-owner", author="worker",
                   hypothesis="One finding retains both local and caller impact",
                   affects=[source.subjects[0].entity_id, caller.entity_id],
                   evidence=["Synthetic caller relationship; not recovery acceptance"], expected_revision=0)
    return root, store, source, owner, caller


def decision_for(owners, *, source_store=None):
    _, store, source, owner, caller = owners
    origin = source_store or store
    event, = origin.journal.read("task:" + source.task_id)
    return {"request_id": "route-caller", "expected_revision": 1,
            "route": {"kind": "discovery_routed", "author": source.author, "discovery_id": "cross-owner",
                      "discovery_event_id": event.event_id, "target": target_reference(store, owner.task_id),
                      "subjects": [caller.entity_id], "evidence": ["Request investigation by the existing owner"]}}


def context(root, name, *, restoration_for=None):
    # This context test deliberately has no compiler or measured restoration.
    return task_context(root, name, restoration_for=restoration_for or
                        (lambda *args: {"result": "unknown", "reason": "No compiler in this synthetic control"}))


def planner(owners, **options):
    root, _, source, _, _ = owners
    delegation = {"schema_version": 1, "author": source.author,
                  "command": {"argv": ["unused-planner"], "timeout": 10},
                  "max_entities": 10, "max_editable_files": 5, "commit_decision": True,
                  "evidence": ["Synthetic task selection bounds"]}
    return _context(root, source.target, delegation, **options)[0]


def without_routes(request):
    result = deepcopy(request)
    for row in result["tasks"]:
        row.pop("incoming_routes")
        row.pop("outgoing_routes")
    return result


def test_context_exposes_exact_source_lineage_and_full_finding_without_target_mutation(owners):
    root, store, source, owner, caller = owners
    original = store.decisions(source.task_id)[0]
    decision = decision_for(owners)
    owner_before = store.read(owner.task_id), store.decisions(owner.task_id), store.scope(owner.task_id)
    route_task(root, source.task_id, decision)
    before = working_state(root)
    source_context, owner_context = context(root, source.task_id), context(root, owner.task_id)
    incoming, = owner_context["incoming_routes"]
    assert incoming["source_task"]["task_id"] == source.task_id
    assert incoming["source_task"]["base_revision"] == source.base_revision
    assert incoming["route_event_id"] == store.journal.read("task:" + source.task_id)[-1].event_id
    assert incoming["route"] == decision["route"]
    assert incoming["discovery"] == original
    assert set(incoming["discovery"]["affects"]) == {source.subjects[0].entity_id, caller.entity_id}
    assert source_context["incoming_routes"] == []
    assert source_context["decision_state"]["outgoing_routes"] == [decision["route"]]
    assert source_context["decision_state"]["pending_discoveries"] == [original]
    assert source_context["decision_state"]["scope_expansion_required"] == [caller.entity_id]
    assert owner_context["decision_state"]["pending_discoveries"] == []
    assert owner_context["decision_state"]["outgoing_routes"] == []
    assert owner_context["decision_state"]["ready_for_collection"] is False
    assert owner_context["persisted"] is False and owner_context["restoration"]["result"] == "unknown"
    instructions = " ".join(owner_context["instructions"])
    assert "owner authorization" in instructions and "original findings remain pending" in instructions
    assert (store.read(owner.task_id), store.decisions(owner.task_id), store.scope(owner.task_id)) == owner_before
    assert working_state(root) == before


def test_planner_visibility_leaves_every_existing_eligibility_and_scope_rule_unchanged(owners):
    root, store, source, owner, _ = owners
    before_request = planner(owners)
    scope = store.scope(source.task_id), store.scope(owner.task_id)
    decision = decision_for(owners)
    route_task(root, source.task_id, decision)
    state = working_state(root)
    request = planner(owners)
    rows = {row["task"]["task_id"]: row for row in request["tasks"]}
    assert rows[source.task_id]["outgoing_routes"] == [decision["route"]]
    assert rows[owner.task_id]["incoming_routes"] == incoming_routes(store, owner.task_id)
    assert rows[source.task_id]["pending_discoveries"] == [store.decisions(source.task_id)[0]]
    assert without_routes(request) == without_routes(before_request)
    assert (store.scope(source.task_id), store.scope(owner.task_id)) == scope
    assert working_state(root) == state
    instructions = " ".join(request["instructions"])
    assert "do not grant owner authorization" in instructions
    assert "activate reserved worktrees" in instructions


@pytest.mark.parametrize("blocked_by", ["reservation", "skip", "parallel"])
def test_incoming_request_never_overrides_planner_exclusions(owners, blocked_by):
    root, store, source, owner, caller = owners
    options = {}
    if blocked_by == "reservation":
        # _context receives an already derived reservation; no external checkout
        # is opened or activated by this visibility-only control.
        options["reserved"] = SimpleNamespace(tasks={owner.task_id}, subjects={caller.entity_id},
                                               files={"code/caller.cc"}, records=[{"tasks": [owner.task_id]}])
    elif blocked_by == "skip":
        options["skip_tasks"] = [owner.task_id]
    else:
        options["parallel_tasks"] = [owner.task_id]
    before = planner(owners, **options)
    route_task(root, source.task_id, decision_for(owners))
    request = planner(owners, **options)
    row = next(row for row in request["tasks"] if row["task"]["task_id"] == owner.task_id)
    assert row["incoming_routes"] and row["eligible"] is False
    assert without_routes(request) == without_routes(before)
    assert store.read(source.task_id) == source and store.read(owner.task_id) == owner


def test_explicit_resolution_removes_derived_active_route_but_retains_its_history(owners):
    root, store, source, owner, _ = owners
    route_task(root, source.task_id, decision_for(owners))
    prior = store.journal.read("task:" + source.task_id)
    assert context(root, owner.task_id)["incoming_routes"]
    store.resolve(source.task_id, discovery_id="cross-owner", request_id="reviewed-rejection", author=source.author,
                  disposition="rejected", source_evidence=["Synthetic explicit review disproves this finding"],
                  expected_revision=2)
    assert context(root, owner.task_id)["incoming_routes"] == []
    assert context(root, source.task_id)["decision_state"]["outgoing_routes"] == []
    assert store.journal.read("task:" + source.task_id)[:2] == prior
    assert store.decisions(owner.task_id) == ()


def test_owner_context_refuses_replaced_target_identity_without_reinterpreting_route(owners):
    root, store, source, owner, _ = owners
    route_task(root, source.task_id, decision_for(owners))
    (root / f"recovery/tasks/{owner.task_id}.json").write_text(
        json.dumps(replace(owner, intent="A different target authorization").to_dict()))
    before = working_state(root)
    for operation in (lambda: context(root, owner.task_id), lambda: planner(owners)):
        with pytest.raises(ValueError):
            operation()
    assert working_state(root) == before
    assert store.decisions(source.task_id)[-1]["kind"] == "discovery_routed"


def test_target_context_rechecks_incoming_route_freshness_after_other_context_work(owners):
    root, store, source, owner, _ = owners
    decision = decision_for(owners)

    def restoration(*args):
        route_task(root, source.task_id, decision)
        return {"result": "unknown", "reason": "Synthetic concurrent route publication"}

    with pytest.raises(ValueError, match="incoming.*routes changed"):
        context(root, owner.task_id, restoration_for=restoration)
    assert store.decisions(source.task_id)[-1] == decision["route"]
    assert store.decisions(owner.task_id) == ()


def test_planner_rechecks_route_context_after_ownership_group_work(owners, monkeypatch):
    import match_recovery.planning as planning

    root, store, source, owner, _ = owners
    decision = decision_for(owners)
    original = planning.ownership_files
    changed = []

    def ownership(*args, **kwargs):
        if not changed:
            route_task(root, source.task_id, decision)
            changed.append(True)
        return original(*args, **kwargs)

    monkeypatch.setattr(planning, "ownership_files", ownership)
    with pytest.raises(ValueError, match="routes changed.*planner context"):
        planner(owners)
    assert changed == [True] and store.decisions(source.task_id)[-1] == decision["route"]
    assert store.decisions(owner.task_id) == ()


def test_current_owner_may_extend_history_without_rewriting_route_lineage(owners):
    root, store, source, owner, caller = owners
    decision = decision_for(owners)
    route_task(root, source.task_id, decision)
    store.discover(owner.task_id, request_id="owner-observation", author="worker",
                   hypothesis="Independent current-owner finding", affects=[caller.entity_id],
                   evidence=["Synthetic evidence after routing"], expected_revision=0)
    store.expand(owner.task_id, request_id="owner-header", author=owner.author, subjects=(),
                 editable_files=("code/owner.hpp",), evidence=["Explicit owner header authorization"], expected_revision=1)
    routed, = context(root, owner.task_id)["incoming_routes"]
    assert routed["route"] == decision["route"] and routed["route"]["target"]["history_head"] is None
    assert store.status(source.task_id, store.scope(source.task_id)[0])["pending_discoveries"]
    assert len(context(root, owner.task_id)["decision_state"]["pending_discoveries"]) == 1


def test_external_source_routes_require_explicit_sources_and_are_not_implicitly_scanned(owners, tmp_path_factory):
    root, store, source, owner, _ = owners
    external = TaskStore(tmp_path_factory.mktemp("external-route-source"))
    external.create(source)
    discovery = store.decisions(source.task_id)[0]
    external.discover(source.task_id, request_id="cross-owner", expected_revision=0,
                      **{key: discovery[key] for key in ("author", "hypothesis", "affects", "evidence")})
    decision = decision_for(owners, source_store=external)
    external.journal.append("task:" + source.task_id, request_id=decision["request_id"],
                            expected_revision=decision["expected_revision"], payload=decision["route"])
    explicit, = incoming_routes(store, owner.task_id, sources=[external])
    assert explicit["route"] == decision["route"] and explicit["discovery"] == discovery
    assert context(root, owner.task_id)["incoming_routes"] == []
    assert next(row for row in planner(owners)["tasks"] if row["task"]["task_id"] == owner.task_id)["incoming_routes"] == []
    assert len(store.decisions(source.task_id)) == 1 and store.decisions(owner.task_id) == ()
