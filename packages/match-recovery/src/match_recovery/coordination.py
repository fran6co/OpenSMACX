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
from .tasks import entity_id, TaskStore


def policy(value, task):
    from .driver import Command
    fields = {"schema_version", "author", "command", "subjects", "editable_files", "evidence", "max_decisions"}
    if (not isinstance(value, dict) or not fields <= value.keys() or value.keys() - fields - {"seed_bindings"}
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
    if not isinstance(response, dict) or not fields <= response.keys() or response.keys() - fields - {"binding"}:
        raise ValueError("invalid coordinator response")
    if response["snapshot"] != snapshot:
        raise ValueError("coordinator response refers to stale source or decisions")
    if response["action"] not in ("expand", "incorporate", "reject", "defer", "investigate", "bind_seed"):
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
        if not isinstance(binding, dict) or set(binding) != {"entity_id", "source", "line", "source_sha256", "name", "reason"}:
            raise ValueError("seed binding needs an identity, fresh source location, symbol and rationale")
        entity_id(binding["entity_id"])
        relative_path(binding["source"])
        require_digest(binding["source_sha256"])
        require_text(binding["name"], "binding symbol")
        require_text(binding["reason"], "binding rationale")
        if type(binding["line"]) is not int or binding["line"] < 1:
            raise ValueError("seed binding requires a positive source line")
    elif binding is not None:
        raise ValueError("only bind_seed may propose a source binding")


def coordinate(root, task_name, configuration, *, execute=None, investigating=()):
    """Execute at most one read-only decision; an external runner owns the budget."""
    from .driver import Command, changes, invoke, working_state
    execute = invoke if execute is None else execute
    root = root.resolve()
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    delegation = policy(configuration, task)
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
    request = {"task": task.to_dict(), "decision_revision": len(decisions),
               "pending_discoveries": state["pending_discoveries"],
               **({"investigating": sorted(skipped)} if skipped else {}),
               "subjects": sorted(selected), "editable_files": sorted(editable),
               "delegation": delegation,
               "mapped_records": [row for row in source["records"] if row["entity_id"] in affected],
               "instructions": ["Remain read-only; propose one evidence-backed discovery decision.",
                                "Delegation is an upper bound, not a reason to expand everything.",
                                "Expand scope before incorporating a cross-owner discovery.",
                                "Incorporation means the insight is expressed in source, not that it is correct or accepted.",
                                "Use investigate to request one bounded worker round for a discovery already inside current scope; it remains unresolved and cannot pass acceptance.",
                                "With explicit seed_bindings delegation, bind_seed may map an existing scoped seed identity to a worker-written body in an editable product TU. Supply its fresh source digest/location and evidence-backed symbol; native publication retains the existing original spans and pending finding. Otherwise return binding:null.",
                                "Choose a pending discovery not listed in investigating; those IDs already have transient authorization for this unchanged round.",
                                "Defer unknown originals or insufficient evidence; do not hide structural work or reset baselines."]}
    request["snapshot"] = digest({"request": request, "source": digest(source),
                                   "head": before["head"].hex(), "index": before["index"].hex(),
                                   "files": before["files"]})
    try:
        code, response = execute(Command.read(delegation["command"]), root, task_name, request)
    finally:
        if changes(before, working_state(root)):
            raise ValueError("read-only coordinator edited the worktree; preserve work and inspect manually")
    assert_snapshot_current(root, source)
    if task != store.read(task_name) or decisions != store.decisions(task_name):
        raise ValueError("task decisions changed during coordination")
    if code != 0:
        return {"action": "unavailable", "published": False, "agent_error": response,
                "ready_for_collection": False}
    validate_response(response, request["snapshot"])
    discovery = next((item for item in pending
                      if item["discovery_id"] == response["discovery_id"]), None)
    if discovery is None:
        raise ValueError("coordinator named a discovery that is not pending")
    action = response["action"]
    if action != "defer" and not set(discovery["affects"]) <= origins.keys():
        raise ValueError("unknown originals require a separate binding decision; defer this discovery")
    if action != "defer" and not set(discovery["affects"]) <= selected | set(delegation["subjects"]):
        raise ValueError("coordinator cannot resolve discoveries outside delegated scope")
    if action == "investigate" and not set(discovery["affects"]) <= selected:
        raise ValueError("expand current task scope before authorizing investigation")
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
        # The trusted native writer changes annotations only, between worker
        # epochs. No discovery is resolved and no task baseline is changed.
        binding_report = bind(root, image=image, address=origins[key].address,
                              spans=[Span(*span) for span in spans],
                              **{name: value for name, value in proposal.items() if name != "entity_id"},
                              evidence=response["evidence"])
    subjects, files = set(response["subjects"]), set(response["editable_files"])
    if action == "expand":
        if (not subjects <= set(delegation["subjects"]) or not files <= set(delegation["editable_files"])
                or not subjects <= set(discovery["affects"])):
            raise ValueError("coordinator exceeded delegated discovery scope")
        if not (subjects - selected or files - editable):
            raise ValueError("coordinator expansion makes no scope progress")
        for name in files:
            # Delegation cannot smuggle policy/identity files into worker scope.
            path = config.path(root, name)
            if (name not in source["files"] or name == config.image_pin
                    or path.is_symlink() or path.resolve() != path):
                raise ValueError("coordinator may expand editing only to existing regular source files")
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
    return {"action": action, "published": action in ("expand", "incorporate", "reject", "bind_seed"),
            **({"binding": binding_report} if binding_report is not None else {}),
            **({"discovery_id": discovery["discovery_id"], "snapshot": request["snapshot"]} if action == "investigate" else {}),
            "evidence": response["evidence"],
            "ready_for_collection": False, "verification": "not_measured"}
