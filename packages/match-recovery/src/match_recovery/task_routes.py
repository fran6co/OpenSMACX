"""Source-owned requests for help from an existing recovery owner.

Routes are durable decisions, not executable permissions or resolved findings.
Only the originating journal is written. Incoming context is derived from explicit
source stores, without copying history or recursively following dependency edges.
Publication is a trusted-coordinator operation; scheduling must independently
establish all campaign ownership, reservation and execution permissions.
"""

from contextlib import ExitStack
from pathlib import Path
import re

from .locking import file_lock
from .model import digest, require_digest, require_text
from .tasks import TaskStore, entity_id, task_name


def _reference_shape(value):
    if not isinstance(value, dict) or set(value) != {
            "task_id", "task_sha256", "base_revision", "history_head"}:
        raise ValueError("invalid routed target reference")
    task_name(value["task_id"])
    require_digest(value["task_sha256"])
    if (not isinstance(value["base_revision"], str)
            or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", value["base_revision"])):
        raise ValueError("routed target requires its original committed baseline")
    if value["history_head"] is not None:
        require_digest(value["history_head"])


def target_reference(store, name):
    """Pin task identity and scope lineage; no source/check result is persisted."""
    task = store.read(name)
    decisions = store.decisions(name)
    events = store.journal.read("task:" + name)
    if task != store.read(name) or tuple(event.payload for event in events) != decisions:
        raise ValueError("route target changed during observation")
    return {"task_id": name, "task_sha256": digest(task.to_dict()),
            "base_revision": task.base_revision,
            "history_head": events[-1].event_id if events else None}


def validate_route_payload(payload, task, discovery, event_id, allowed, routed):
    """Historical validation needs no target checkout, activation or measurements."""
    if not isinstance(payload, dict) or set(payload) != {
            "kind", "author", "discovery_id", "discovery_event_id", "target", "subjects", "evidence"}:
        raise ValueError("invalid discovery route fields")
    if payload["kind"] != "discovery_routed" or payload["author"] != task.author:
        raise ValueError("only the source task coordinator may route discoveries")
    if payload["discovery_id"] != discovery["discovery_id"] or payload["discovery_event_id"] != event_id:
        raise ValueError("route does not identify the exact discovery event")
    _reference_shape(payload["target"])
    if payload["target"]["task_id"] == task.task_id:
        raise ValueError("route requires a different target task")
    subjects = payload["subjects"]
    if not isinstance(subjects, list) or not subjects:
        raise ValueError("routed subjects must be a nonempty unique array")
    for subject in subjects:
        entity_id(subject)
    if subjects != sorted(set(subjects)):
        raise ValueError("routed subjects must be unique and sorted")
    if not set(subjects) <= set(discovery["affects"]):
        raise ValueError("route subjects are outside the discovery")
    if set(subjects) & set(allowed):
        raise ValueError("route subjects are already owned by the source task")
    if any((discovery["discovery_id"], subject) in routed for subject in subjects):
        raise ValueError("discovery subject is already routed; conflicting retargeting refuses")
    if not isinstance(payload["evidence"], list) or not payload["evidence"]:
        raise ValueError("route needs nonempty evidence")
    for item in payload["evidence"]:
        require_text(item, "route evidence")


def validate_route_target(store, name, reference, subjects):
    """A current owner may extend, but cannot replace, its pinned task history."""
    _reference_shape(reference)
    current = target_reference(store, name)
    if any(current[key] != reference[key] for key in ("task_id", "task_sha256", "base_revision")):
        raise ValueError("route target identity or original baseline changed")
    events = store.journal.read("task:" + name)
    head = reference["history_head"]
    if head is not None and head not in {event.event_id for event in events}:
        raise ValueError("route target history does not extend its pinned history head")
    # Ownership must already have existed at the referenced decision, not merely
    # appear in a later expansion after somebody guessed the intended owner.
    prefix = () if head is None else events[:next(i for i, event in enumerate(events) if event.event_id == head) + 1]
    task = store.read(name)
    original_scope, _ = store._scope(task, tuple(event.payload for event in prefix))
    current_scope, _ = store.scope(name)
    if not subjects or not set(subjects) <= original_scope & current_scope:
        raise ValueError("routed subjects were not owned by the pinned target scope")
    if current != target_reference(store, name):
        raise ValueError("route target changed during validation")


def _compatible_tasks(source, target):
    if (source.target != target.target or
            {(item.image_sha256, item.address_space) for item in source.subjects} !=
            {(item.image_sha256, item.address_space) for item in target.subjects}):
        raise ValueError("route target refers to an unrelated target image/address space")


def route_task(root, name, decision, *, target_root=None, _held_leases=None):
    """Publish one bound help request, never source changes or owner activation.

    Explicit target_root locates an existing owner for validation only; it is not
    serialized. Call from a serial coordinator checkpoint, not a running driver.
    Both driver leases and the target journal lease must be available. Existing
    route retries are idempotent but still require a live, matching target.
    """
    if not isinstance(decision, dict) or set(decision) != {"request_id", "expected_revision", "route"}:
        raise ValueError("invalid route decision")
    require_text(decision["request_id"], "route request id")
    if type(decision["expected_revision"]) is not int or decision["expected_revision"] < 0:
        raise ValueError("route revision must be a nonnegative integer")
    payload = decision["route"]
    if not isinstance(payload, dict) or "target" not in payload:
        raise ValueError("invalid discovery route")
    _reference_shape(payload["target"])
    name = task_name(name)
    root = Path(root).resolve(strict=True)
    target_root = root if target_root is None else Path(target_root).resolve(strict=True)
    source, owner = TaskStore(root), TaskStore(target_root)
    if _held_leases is not None and not isinstance(_held_leases, dict):
        raise ValueError("routing requires actual local lease tokens, not an already-locked flag")
    target_name = payload["target"]["task_id"]
    if name == target_name:
        raise ValueError("route requires a different target task")
    with ExitStack() as stack:
        for path in sorted({root, target_root}):
            stack.enter_context(file_lock(path / "build/recovery/driver.lock", timeout=0,
                                          held=(_held_leases or {}).get(path)))
        stack.enter_context(file_lock(owner.journal.lock_directory / (digest("task:" + target_name) + ".lock"), timeout=0))
        task, decisions = source.read(name), source.decisions(name)
        events = source.journal.read("task:" + name)
        if tuple(event.payload for event in events) != decisions:
            raise ValueError("route source history changed during observation")
        discovery_id = payload.get("discovery_id")
        found = next((event for event in events if event.payload["kind"] == "discovery"
                      and event.payload["discovery_id"] == discovery_id), None)
        if found is None or any(item["kind"] == "discovery_resolved" and item["discovery_id"] == discovery_id
                                for item in decisions):
            raise ValueError("routing requires a pending discovery")
        retry = next((event for event in events if event.request_id == decision["request_id"]), None)
        if retry is not None and retry.payload != payload:
            raise ValueError("request id already belongs to a different event")
        routed = {(item["discovery_id"], subject) for event in events if event is not retry
                  for item in (event.payload,) if item["kind"] == "discovery_routed" for subject in item["subjects"]}
        allowed, _ = source._scope(task, decisions)
        validate_route_payload(payload, task, found.payload, found.event_id, allowed, routed)
        owner_task = owner.read(target_name)
        _compatible_tasks(task, owner_task)
        validate_route_target(owner, target_name, payload["target"], payload["subjects"])
        owner_reference = target_reference(owner, target_name)
        if retry is None and owner_reference != payload["target"]:
            raise ValueError("route target history changed before publication; refresh the decision")
        # Check all task identities visible in these explicit stores. Unknown
        # campaign peers still need the scheduler's complete ownership checks.
        for store in (source, owner):
            for path in sorted((store.root / "recovery/tasks").glob("*.json")):
                candidate = store.read(path.stem)
                if candidate.task_id == target_name:
                    if digest(candidate.to_dict()) != payload["target"]["task_sha256"]:
                        raise ValueError("conflicting replicas of the routed owner task")
                elif set(payload["subjects"]) & store.scope(candidate.task_id)[0]:
                    raise ValueError("routed subjects have conflicting task ownership")
        if target_reference(owner, target_name) != owner_reference:
            raise ValueError("route target changed before publication")
        if source.read(name) != task or source.journal.read("task:" + name) != events:
            raise ValueError("route source task or history changed before publication")
        event = source.journal.append("task:" + name, request_id=decision["request_id"],
                                      payload=payload, expected_revision=decision["expected_revision"],
                                      expected_head=events[-1].event_id)
        return {"published": True, "route": event.payload,
                "verification": "not_measured", "ready_for_collection": False}


def incoming_routes(store, name, *, sources=None):
    """Recompute help requests from explicit stores, without activating any owner.

    Missing external stores are not searched for implicitly. History remains
    readable without them; a supplied stale/conflicting owner instead refuses.
    Cycles are ordinary unresolved requests, never recursively executed here.
    """
    owner_before = target_reference(store, name)
    owner_task = store.read(name)
    result, observed, catalogs, replicas = [], [], [], {}
    for source in (store,) if sources is None else sources:
        paths = tuple(sorted((source.root / "recovery/tasks").glob("*.json")))
        catalogs.append((source, paths))
        for path in paths:
            source_name = path.stem
            before = target_reference(source, source_name)
            events = source.journal.read("task:" + source_name)
            if (events[-1].event_id if events else None) != before["history_head"]:
                raise ValueError("incoming route history changed during observation")
            if before != target_reference(source, source_name):
                raise ValueError("incoming route history changed during observation")
            observed.append((source, source_name, before))
            previous = replicas.get(source_name)
            if previous is not None:
                _, previous_reference, previous_events = previous
                shared = min(len(events), len(previous_events))
                if (before["task_sha256"] != previous_reference["task_sha256"]
                        or events[:shared] != previous_events[:shared]):
                    raise ValueError("incoming routes have conflicting source task/history replicas")
                if len(events) <= len(previous_events):
                    continue
            replicas[source_name] = (source, before, events)
    # A stale replica must not resurrect a finding resolved in a supplied newer
    # prefix. Choose one compatible latest history per immutable task first.
    for source_name in sorted(replicas):
        source, before, events = replicas[source_name]
        resolved = {event.payload["discovery_id"] for event in events
                    if event.payload["kind"] == "discovery_resolved"}
        discoveries = {event.payload["discovery_id"]: event.payload for event in events
                       if event.payload["kind"] == "discovery"}
        for event in events:
            route = event.payload
            if (route["kind"] != "discovery_routed" or route["target"]["task_id"] != name
                    or route["discovery_id"] in resolved):
                continue
            validate_route_target(store, name, route["target"], route["subjects"])
            _compatible_tasks(source.read(source_name), owner_task)
            result.append({"source_task": before, "route_event_id": event.event_id,
                           "route": route, "discovery": discoveries[route["discovery_id"]]})
    for source, paths in catalogs:
        if tuple(sorted((source.root / "recovery/tasks").glob("*.json"))) != paths:
            raise ValueError("incoming route source catalog changed during observation")
    for source, source_name, before in observed:
        if before != target_reference(source, source_name):
            raise ValueError("incoming route history changed during observation")
    if owner_before != target_reference(store, name):
        raise ValueError("incoming route target changed during observation")
    return result
