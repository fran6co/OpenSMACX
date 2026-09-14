"""Delegated discovery decisions, never proof or unconstrained editing authority.

The configured coordinator command is read-only. Only this module publishes a
validated decision through TaskStore, within the caller's explicit delegation.
An explicitly delegated existing seed may receive an unclaimed product binding
through the native writer. No baseline, Git state or verifier policy is changed.
"""

import os
from pathlib import Path
import re
import uuid

from .components import relative_path
from .model import digest, Origin, Span, require_digest, require_text
from .source_config import SourceConfiguration
from .source_inventory import assert_snapshot_current, source_snapshot
from .tasks import entity_id, task_name as validate_task_name, TaskStore


class SerialCoordinationRequired(ValueError):
    """A read-only planning phase cannot publish this proposed decision."""


class DecisionBudget:
    """Trusted, process-local allowance shared by central planning phases."""

    def __init__(self, limit):
        if type(limit) is not int or not 1 <= limit <= 100:
            raise ValueError("coordinator decision budget must be 1..100")
        self.limit, self.used = limit, 0

    @property
    def remaining(self):
        return self.limit - self.used

    def charge(self):
        if self.remaining <= 0:
            raise ValueError("coordinator decision budget exhausted")
        self.used += 1


def decision_budget(configuration, value=None):
    result = DecisionBudget(configuration["max_decisions"]) if value is None else value
    if (not isinstance(result, DecisionBudget) or result.limit != configuration["max_decisions"]
            or type(result.used) is not int or not 0 <= result.used <= result.limit):
        raise ValueError("shared coordinator budget differs from delegated allowance")
    return result


def rejection_key(report):
    return tuple(report[key] for key in ("discovery_id", "proposed_action", "reason_code"))


def validate_rejection_feedback(root, task_name, configuration, value):
    from .investigation_round import epoch
    fields = {"action", "published", "discovery_id", "snapshot", "proposed_action",
              "reason_code", "feedback", "ready_for_collection", "verification"}
    reasons = {"outside_delegated_scope": {"investigate", "incorporate", "reject", "bind_seed"},
               "investigation_requires_expansion": {"investigate"},
               "resolution_requires_expansion": {"incorporate", "reject"},
               "expansion_exceeds_delegation": {"expand"}, "expansion_no_progress": {"expand"}}
    if not isinstance(value, dict) or set(value) != {"epoch", "delegation_sha256", "refusal"}:
        raise ValueError("invalid coordinator rejection feedback")
    report = value["refusal"]
    if (not isinstance(report, dict) or set(report) != fields or report["action"] != "proposal_rejected"
            or report["published"] is not False or report["ready_for_collection"] is not False
            or report["verification"] != "not_measured" or not isinstance(report["reason_code"], str)
            or report["reason_code"] not in reasons or not isinstance(report["proposed_action"], str)
            or report["proposed_action"] not in reasons[report["reason_code"]]):
        raise ValueError("feedback must be a pure unpublished coordinator proposal refusal")
    for key in ("discovery_id", "feedback"):
        require_text(report[key], "coordinator rejection " + key)
    for fingerprint in (value["epoch"], value["delegation_sha256"], report["snapshot"]):
        require_digest(fingerprint)
    if value["delegation_sha256"] != digest(configuration) or value["epoch"] != epoch(root, task_name):
        raise ValueError("stale coordinator rejection feedback; source, decisions, runtime or delegation changed")
    store = TaskStore(root)
    pending = store.status(task_name, store.scope(task_name)[0])["pending_discoveries"]
    if report["discovery_id"] not in {item["discovery_id"] for item in pending}:
        raise ValueError("rejection feedback names a discovery that is not pending")


def rejection_feedback(root, task_name, configuration, report, *, epoch_before):
    """Bind a native refusal to the state captured BEFORE its provider call."""
    result = {"epoch": epoch_before, "delegation_sha256": digest(configuration), "refusal": report}
    validate_rejection_feedback(root, task_name, configuration, result)
    return result


def decision_facts(source, selected, editable, delegation, pending, allowed_actions=None):
    """Derived proposal constraints, not repair evidence or editing authority."""
    config = SourceConfiguration.from_dict(source["source_configuration"])
    known = {row["entity_id"] for row in source["records"]}
    seeds = known - {row["entity_id"] for row in source["records"] if not config.is_seed(row["path"])}
    files = (set(delegation["editable_files"]) & set(source["files"])) - set(editable) - {config.image_pin}
    results = []
    for finding in pending:
        affected = set(finding["affects"])
        unknown = affected - known
        repairs = (affected & known & set(delegation["subjects"])) - set(selected)
        whole = not unknown and affected <= set(selected)
        central = "serial_required" if allowed_actions is not None else "central"
        results.append({"discovery_id": finding["discovery_id"],
                        "currently_owned": sorted(affected & set(selected)),
                        "delegated_repairs": sorted(repairs),
                        "outside_delegation": sorted((affected & known) - set(selected) - set(delegation["subjects"])),
                        "unknown_originals": sorted(unknown), "delegated_files": sorted(files),
                        "actions": {"defer": "read_only", "investigate": "read_only" if whole else "unavailable",
                                    "investigate_local": "conditional_local_work" if not unknown else "unavailable",
                                    "expand": central if not unknown and (repairs or files) else "unavailable",
                                    "incorporate": central if whole else "unavailable",
                                    "reject": central if whole else "unavailable",
                                    "bind_seed": central if not unknown and "seed_bindings" in delegation
                                    and affected <= set(selected) | set(delegation["subjects"])
                                    and affected & set(selected) & seeds else "unavailable"}})
        if allowed_actions is not None:
            results[-1]["actions"] = {action: "serial_required" if route != "unavailable"
                                      and action not in allowed_actions else route
                                      for action, route in results[-1]["actions"].items()}
    return results


def policy(value, task):
    from .driver import Command
    fields = {"schema_version", "author", "command", "subjects", "editable_files", "evidence", "max_decisions"}
    if (not isinstance(value, dict) or not fields <= value.keys() or value.keys() - fields - {"seed_bindings", "route_discoveries"}
            or type(value["schema_version"]) is not int or value["schema_version"] != 1):
        raise ValueError("invalid coordinator delegation")
    if value["author"] != task.author:
        raise ValueError("delegation must name the task coordinator")
    Command.read(value["command"])
    if type(value["max_decisions"]) is not int or not 1 <= value["max_decisions"] <= 100:
        raise ValueError("coordinator decision budget must be 1..100")
    for key in ("subjects", "editable_files", "evidence"):
        values = value[key]
        if not isinstance(values, list) or any(not isinstance(item, str) for item in values) or len(set(values)) != len(values):
            raise ValueError("delegation needs unique string arrays")
        for item in values:
            (entity_id if key == "subjects" else relative_path if key == "editable_files"
             else lambda item: require_text(item, "delegation evidence"))(item)
    if not value["evidence"]:
        raise ValueError("delegation needs evidence for its authorization boundary")
    if "route_discoveries" in value and value["route_discoveries"] is not True:
        raise ValueError("discovery routing requires explicit true permission")
    if "seed_bindings" in value:
        binding = value["seed_bindings"]
        if not isinstance(binding, dict) or set(binding) != {"image"}:
            raise ValueError("seed binding delegation requires only an original image path")
        require_text(binding["image"], "seed binding image")
        if ("{" in binding["image"] or "}" in binding["image"]) and not re.fullmatch(
                r"\{env:[A-Za-z_][A-Za-z_0-9]*\}", binding["image"]):
            raise ValueError("seed binding image supports a path or one environment placeholder")
    return value


def validate_response(response, snapshot):
    fields = {"snapshot", "action", "discovery_id", "subjects", "editable_files", "evidence"}
    if not isinstance(response, dict) or not fields <= response.keys() or response.keys() - fields - {"binding", "route"}:
        raise ValueError("invalid coordinator response")
    if response["snapshot"] != snapshot:
        raise ValueError("coordinator response refers to stale source or decisions")
    if response["action"] not in ("expand", "incorporate", "reject", "defer", "investigate", "investigate_local", "bind_seed", "route"):
        raise ValueError("unsupported coordinator action")
    require_text(response["discovery_id"], "coordinator discovery")
    for key in ("subjects", "editable_files", "evidence"):
        values = response[key]
        if not isinstance(values, list) or any(not isinstance(item, str) for item in values) or len(set(values)) != len(values):
            raise ValueError("invalid coordinator decision arrays")
        for item in values:
            (entity_id if key == "subjects" else relative_path if key == "editable_files"
             else lambda item: require_text(item, "coordinator evidence"))(item)
    if not response["evidence"]:
        raise ValueError("coordinator decision needs evidence")
    if response["action"] != "expand" and (response["subjects"] or response["editable_files"]):
        raise ValueError("only an expansion may request additional scope")
    binding = response.get("binding")
    if response["action"] == "bind_seed":
        if not isinstance(binding, dict) or set(binding) != {"entity_id", "source", "line", "source_sha256", "name", "reason", "evidence"}:
            raise ValueError("seed binding needs an identity, fresh source location, symbol, rationale and separate binding evidence")
        entity_id(binding["entity_id"])
        relative_path(binding["source"])
        require_digest(binding["source_sha256"])
        require_text(binding["name"], "binding symbol")
        require_text(binding["reason"], "binding rationale")
        from .binding_source import validate as validate_binding
        validate_binding({"reason": binding["reason"], "evidence": binding["evidence"]})
        if len(set(binding["evidence"])) != len(binding["evidence"]):
            raise ValueError("seed binding evidence references must be distinct")
        if type(binding["line"]) is not int or binding["line"] < 1:
            raise ValueError("seed binding requires a positive source line")
    elif binding is not None:
        raise ValueError("only bind_seed may propose a source binding")
    route = response.get("route")
    if response["action"] == "route":
        if not isinstance(route, dict) or set(route) != {"owner", "subjects"}:
            raise ValueError("routing selects only a supplied owner and affected subjects")
        validate_task_name(route["owner"])
        if not isinstance(route["subjects"], list) or not route["subjects"]:
            raise ValueError("routing requires nonempty sorted unique subjects")
        for subject in route["subjects"]:
            entity_id(subject)
        if route["subjects"] != sorted(set(route["subjects"])):
            raise ValueError("routing requires sorted unique subjects")
    elif route is not None:
        raise ValueError("only route may propose another owner")


def routing_facts(root, name, delegation, routing):
    """Known, unrouted subsets owned by supplied managed peers; never a grant."""
    from .task_routes import _compatible_tasks
    store = TaskStore(root)
    task, decisions = store.read(name), store.decisions(name)
    selected, _ = store._scope(task, decisions)
    pending = store.status(name, selected)["pending_discoveries"]
    known = {row["entity_id"] for row in source_snapshot(root)["records"]}
    conflicted = {name for row in routing.get("conflicts", []) for name in row["tasks"]}
    routed = {(item["discovery_id"], subject) for item in decisions if item["kind"] == "discovery_routed"
              for subject in item["subjects"]}
    results = []
    for finding in pending:
        owners = []
        for owner in routing["owners"]:
            if (delegation.get("route_discoveries") is not True or owner["availability"] != "managed"
                    or owner["task"]["task_id"] == name or name in conflicted
                    or owner["task"]["task_id"] in conflicted):
                continue
            _compatible_tasks(task, TaskStore(Path(owner["root"])).read(owner["task"]["task_id"]))
            subjects = sorted((set(finding["affects"]) & known & set(owner["subjects"])) - set(selected)
                              - {subject for key, subject in routed if key == finding["discovery_id"]})
            if subjects:
                owners.append({"owner": owner["task"]["task_id"], "subjects": subjects})
        results.append({"discovery_id": finding["discovery_id"], "owners": owners})
    return results


def coordinate(root, task_name, configuration, *, execute=None, investigating=(), allowed_actions=None,
               diagnostic_configuration=None, previous_rejection=None, _budget=None, _routing=None, _held_lease=None):
    """Execute at most one read-only decision; an external runner owns the budget."""
    from .driver import Command, changes, invoke, working_state
    execute = invoke if execute is None else execute
    root = root.resolve()
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    delegation = policy(configuration, task)
    routing = None
    routing_external = None
    if _routing is not None:
        from .routing_owners import RoutingOwners
        if not isinstance(_routing, RoutingOwners) or delegation.get("route_discoveries") is not True:
            raise ValueError("owner routing requires an explicit coordinator permission and native owner context")
        routing = _routing.snapshot(root, task_name)
        routing_external = _routing.external_epoch(root)
    budget = decision_budget(delegation, _budget)
    if previous_rejection is not None:
        validate_rejection_feedback(root, task_name, delegation, previous_rejection)
    if not budget.remaining:
        return {"action": "budget_exhausted", "published": False, "ready_for_collection": False}
    if allowed_actions is not None:
        if not allowed_actions or not set(allowed_actions) <= {"investigate", "investigate_local", "defer"}:
            raise ValueError("read-only coordination actions must be investigation or deferral")
    selected, editable = store._scope(task, decisions)
    before = working_state(root)
    image = None
    if "seed_bindings" in delegation:
        image_name = delegation["seed_bindings"]["image"]
        if image_name.startswith("{env:"):
            variable = image_name[5:-1]
            if not os.environ.get(variable):
                raise ValueError("missing seed binding image environment: " + variable)
            image_name = os.environ[variable]
        image = Path(image_name)
        image = image if image.is_absolute() else root / image
    source = source_snapshot(root, image=image)
    config = SourceConfiguration.from_dict(source["source_configuration"])
    origins = {row["entity_id"]: Origin(**row["origin"]) for row in source["records"]}
    if any((item.image_sha256, item.address_space) != (source["image_sha256"], config.address_space)
           for item in task.subjects):
        raise ValueError("coordinator task does not match the configured original")
    state = store.status(task_name, origins)
    if state["missing_subjects"]:
        raise ValueError("repair missing source mappings before coordinating discoveries")
    if not state["pending_discoveries"]:
        return {"action": "none", "published": False, "ready_for_collection": False}
    # Driver-owned, transient authorizations from this unchanged worker round.
    # They filter requests, never resolve or hide discoveries from task context.
    skipped = set(investigating)
    if not skipped <= {item["discovery_id"] for item in state["pending_discoveries"]}:
        raise ValueError("investigation filter names a discovery that is not pending")
    pending = [item for item in state["pending_discoveries"] if item["discovery_id"] not in skipped]
    if not pending:
        raise ValueError("no pending discovery remains for coordination")
    affected = {key for item in state["pending_discoveries"] for key in item["affects"]}
    diagnostics = None
    if diagnostic_configuration is not None:
        from .coordination_diagnostics import collect
        diagnostics = collect(root, task_name, diagnostic_configuration, execute)
    request = {"task": task.to_dict(), "decision_revision": len(decisions),
               "decision_facts": decision_facts(source, selected, editable, delegation, state["pending_discoveries"], allowed_actions),
               **({"previous_rejection": previous_rejection} if previous_rejection is not None else {}),
               **({"diagnostics": diagnostics} if diagnostics is not None else {}),
               **({"routing": routing, "route_choices": routing_facts(root, task_name, delegation, routing)}
                  if routing is not None else {}),
               **({"allowed_actions": sorted(set(allowed_actions))} if allowed_actions is not None else {}),
               "pending_discoveries": state["pending_discoveries"],
               **({"investigating": sorted(skipped)} if skipped else {}),
               "subjects": sorted(selected), "editable_files": sorted(editable),
               "delegation": delegation,
               "mapped_records": [row for row in source["records"] if row["entity_id"] in affected],
               "instructions": ["Remain read-only; propose one evidence-backed discovery decision.",
                                "decision_facts are derived constraints, not evidence that an action is warranted. A conditional local investigation requires useful work in current files; if only another owner's work can help, defer without inventing a local repair.",
                                "previous_rejection, when present, is native feedback for an unpublished proposal against unchanged source, decisions and delegation. Correct the proposal using fresh diagnostics; it grants no scope, resolution or acceptance. Repeated equivalent refusals stop this attempt.",
                                "Delegation is an upper bound, not a reason to expand everything.",
                                "When diagnostics are supplied, they are freshly run configured context/check observations for this unchanged scope. Historical failure reports in discoveries are not current measurements. Inspect actual coverage, exit status and remaining unknowns; a passing protected-body check does not prove an unmeasured candidate, build, layout or runtime behavior.",
                                "Diagnostics grant no resolution, scope or acceptance. Keep a genuine source discovery pending while investigating; do not reject it merely because a compiler now works. Controller diagnostics do not authorize preparation or standalone compiler execution on its leased prefix.",
                                "A discovery's affects list includes read-only impact context, not automatic editing permission. Expand only a known repair subset within delegation; other affected owners remain outside scope and the entire discovery remains pending.",
                                "During restricted preplanning, allowed_actions controls what this phase may grant or publish. An evidence-backed expansion, binding or resolution may still be proposed to request fresh serial coordination; this phase will not publish it. Do not relabel that required action as investigate or defer merely because it lies outside current task scope.",
                                "Expand scope before incorporating a cross-owner discovery.",
                                "Incorporation means the insight is expressed in source, not that it is correct or accepted.",
                                "Use investigate to request one bounded worker round for a discovery already inside current scope; it remains unresolved and cannot pass acceptance.",
                                "Use investigate_local to examine or repair only current task files when a discovery also involves known outside owners. This grants no outside editing, ownership transfer, resolution or acceptance; explain the useful local work in evidence. Defer if only outside work can help.",
                                "With explicit seed_bindings delegation, bind_seed may map an existing scoped seed identity to a worker-written body in an editable product TU. Supply its fresh source digest/location and evidence-backed symbol; native publication retains the existing original spans and pending finding. Otherwise return binding:null.",
                                "For bind_seed, binding.reason and binding.evidence are source-owned, load-bearing identity rationale and references: explain the original address/symbol, body and owner relationship. Top-level evidence explains this coordinator decision and may contain run diagnostics, request hashes, impact counts, compiler failures and permission checks. Never copy that run context into binding.reason or binding.evidence; both evidence arrays are independently required and neither substitutes for the other.",
                                "Choose a pending discovery not listed in investigating; those IDs already have transient authorization for this unchanged round.",
                                "When routing and route_choices are supplied, route can request help for a listed subset from an existing managed owner. Return only its supplied task id and subjects; the controller binds discovery, owner and history. It grants no editing scope, owner activation, investigation or resolution. Parked, retained historical, and unassigned owners are context, not selectable targets. Routes publish only in serial coordination; keep the complete finding pending.",
                                "Defer unknown originals or insufficient evidence; do not hide structural work or reset baselines."]}
    request["snapshot"] = digest({"request": request, "source": digest(source),
                                   "head": before["head"].hex(), "index": before["index"].hex(),
                                   "files": before["files"]})
    if diagnostics is not None:
        from .coordination_diagnostics import validate
        validate(root, task_name, diagnostics)
        assert_snapshot_current(root, source)
        if (changes(before, working_state(root)) or task != store.read(task_name)
                or decisions != store.decisions(task_name)):
            raise ValueError("coordinator inputs changed before diagnostic request dispatch")
    if previous_rejection is not None:
        validate_rejection_feedback(root, task_name, delegation, previous_rejection)
    if routing is not None and (_routing.snapshot(root, task_name) != routing
                                or _routing.external_epoch(root) != routing_external):
        raise ValueError("owner routing context changed before coordinator execution")
    budget.charge()  # Count actual coordinator calls, including serial handoffs and refusals.
    try:
        code, response = execute(Command.read(delegation["command"]), root, task_name, request)
    finally:
        if changes(before, working_state(root)):
            raise ValueError("read-only coordinator edited the worktree; preserve work and inspect manually")
        if routing is not None and _routing.external_epoch(root) != routing_external:
            raise ValueError("read-only coordinator changed an external owner; preserve work and replan")
    assert_snapshot_current(root, source)
    if diagnostics is not None:
        validate(root, task_name, diagnostics)
    if task != store.read(task_name) or decisions != store.decisions(task_name):
        raise ValueError("task decisions changed during coordination")
    if routing is not None and _routing.snapshot(root, task_name) != routing:
        raise ValueError("owner routing context changed during coordination")
    if code != 0:
        return {"action": "unavailable", "published": False, "agent_error": response,
                "ready_for_collection": False}
    validate_response(response, request["snapshot"])
    discovery = next((item for item in pending
                      if item["discovery_id"] == response["discovery_id"]), None)
    if discovery is None:
        raise ValueError("coordinator named a discovery that is not pending")
    action = response["action"]
    def refuse(reason_code, feedback):
        # Only pure proposal-policy failures reach this outcome. Freshness,
        # response syntax and native publication failures remain fatal above
        # or below; never catch them as a reason to continue a campaign.
        return {"action": "proposal_rejected", "published": False,
                "discovery_id": discovery["discovery_id"], "snapshot": request["snapshot"],
                "proposed_action": action, "reason_code": reason_code, "feedback": feedback,
                "ready_for_collection": False, "verification": "not_measured"}

    if action not in ("defer", "route") and not set(discovery["affects"]) <= origins.keys():
        raise ValueError("unknown originals require a separate binding decision; defer this discovery")
    # Expansion grants only its explicitly proposed subset, checked below. A
    # known outside caller in the impact footprint must not force ownership
    # takeover merely to authorize a repair elsewhere. Resolution retains the
    # stronger whole-finding boundary, including TaskStore's independent check.
    if action not in ("defer", "investigate_local", "expand", "route") and not set(discovery["affects"]) <= selected | set(delegation["subjects"]):
        return refuse("outside_delegated_scope", "The complete discovery includes owners outside delegated scope. "
                      "Keep it pending; propose investigate_local only for useful work in existing scope, or defer.")
    if action == "investigate" and not set(discovery["affects"]) <= selected:
        return refuse("investigation_requires_expansion", "Expand current task scope before authorizing whole-finding "
                      "investigation, or propose investigate_local for useful work within existing scope.")
    if action in ("incorporate", "reject") and not set(discovery["affects"]) <= selected:
        return refuse("resolution_requires_expansion", "Delegation is not current ownership. Expand current task scope "
                      "before resolving the complete discovery; do not dismiss its outside-owner effects.")
    binding_report = None
    if action == "bind_seed":
        from .binding import bind
        proposal = response["binding"]
        key = proposal["entity_id"]
        if image is None:
            raise ValueError("seed binding requires explicit coordinator delegation")
        if key not in selected or key not in discovery["affects"]:
            raise ValueError("seed binding must name an existing selected discovery identity")
        if proposal["source"] not in editable:
            raise ValueError("seed binding source is outside authorized editable files")
        records = [row for row in source["records"] if row["entity_id"] == key]
        if not records or any(not config.is_seed(row["path"]) for row in records):
            raise ValueError("seed binding requires an existing seed-only identity, not a product or new identity")
        spans = records[0]["spans"]
        if not spans or any(row["spans"] != spans for row in records):
            raise ValueError("seed binding requires agreeing existing original spans")
    subjects, files = set(response["subjects"]), set(response["editable_files"])
    if action == "route":
        if routing is None:
            raise ValueError("routing requires native managed owners and explicit delegation")
        proposal = response["route"]
        row = next(item for item in request["route_choices"] if item["discovery_id"] == discovery["discovery_id"])
        available = next((item for item in row["owners"] if item["owner"] == proposal["owner"]), None)
        if available is None or not set(proposal["subjects"]) <= set(available["subjects"]):
            raise ValueError("route must select an available managed owner and unrouted affected subjects")
    if action == "expand":
        if (not subjects <= set(delegation["subjects"]) or not files <= set(delegation["editable_files"])
                or not subjects <= set(discovery["affects"])):
            return refuse("expansion_exceeds_delegation", "Expand only explicitly delegated subjects/files and affected "
                          "identities. Keep other owners as pending read-only context.")
        if not (subjects - selected or files - editable):
            return refuse("expansion_no_progress", "The proposed expansion adds no scope. Choose a justified action "
                          "against current scope or defer; do not republish an existing expansion.")
        for name in files:
            # Delegation cannot smuggle policy/identity files into worker scope.
            path = config.path(root, name)
            if (name not in source["files"] or name == config.image_pin
                    or path.is_symlink() or path.resolve() != path):
                raise ValueError("coordinator may expand editing only to existing regular source files")
    if allowed_actions is not None and action not in allowed_actions:
        raise SerialCoordinationRequired("coordinator action requires fresh serial coordination: " + action)
    if action == "bind_seed":
        # All writers stay AFTER policy validation and serial routing.
        binding_report = bind(root, image=image, address=origins[key].address,
                              spans=[Span(*span) for span in spans],
                              **{name: value for name, value in proposal.items() if name != "entity_id"})
    evidence = [*response["evidence"], "Delegated policy: " + digest(delegation)]
    decision = dict(request_id="coordinator-" + uuid.uuid4().hex, author=task.author,
                    expected_revision=len(decisions))
    if action == "expand":
        store.expand(task_name, subjects=tuple(origins[key] for key in sorted(subjects - selected)),
                     editable_files=sorted(files - editable), evidence=evidence, **decision)
    elif action in ("incorporate", "reject"):
        store.resolve(task_name, discovery_id=discovery["discovery_id"],
                      disposition="incorporated" if action == "incorporate" else "rejected",
                      source_evidence=evidence, **decision)
    elif action == "route":
        from .task_routes import route_task
        owner = next(item for item in routing["owners"] if item["task"]["task_id"] == proposal["owner"])
        event = next(item for item in store.journal.read("task:" + task_name)
                     if item.payload["kind"] == "discovery" and item.payload["discovery_id"] == discovery["discovery_id"])
        if _routing.snapshot(root, task_name) != routing:
            raise ValueError("owner routing context changed before publication")
        if _routing.external_epoch(root) != routing_external:
            raise ValueError("external owner changed before route publication")
        route_task(root, task_name, {"request_id": decision["request_id"], "expected_revision": len(decisions),
                                    "route": {"kind": "discovery_routed", "author": task.author,
                                              "discovery_id": discovery["discovery_id"], "discovery_event_id": event.event_id,
                                              "target": owner["task"], "subjects": proposal["subjects"], "evidence": evidence}},
                   target_root=Path(owner["root"]), _held_leases={**_routing.leases, **({root: _held_lease} if _held_lease else {})})
    return {"action": action, "published": action in ("expand", "incorporate", "reject", "bind_seed", "route"),
            **({"binding": binding_report} if binding_report is not None else {}),
            **({"discovery_id": discovery["discovery_id"], "snapshot": request["snapshot"]}
               if action in ("investigate", "investigate_local") else {}),
            "evidence": response["evidence"],
            "ready_for_collection": False, "verification": "not_measured"}
