"""Fresh one-attempt permissions for centrally planned parallel investigations.

These are coordinator decisions, not measurement receipts or discovery resolutions.
Task/source/Git inputs remain authoritative. A consumed permission is never retried;
obtain fresh coordination after interruption, even if source appears unchanged.
"""

from pathlib import Path
import re
import uuid

from .coordination import coordinate, policy, validate_response
from .locking import file_lock
from .model import digest, require_digest
from .records import publish_json
from .source_inventory import source_snapshot
from .source_config import SourceConfiguration
from .source_replica import artifact_parent
from .tasks import TaskStore


def epoch(root, name):
    from .driver import working_state
    store = TaskStore(root)
    state = working_state(root)
    return digest({"root": str(root.resolve()), "task": store.read(name).to_dict(),
                   "decisions": store.decisions(name), "source": source_snapshot(root),
                   "head": state["head"].hex(), "index": state["index"].hex(), "files": state["files"],
                   **({"framework_runtime": state["framework_runtime"]} if "framework_runtime" in state else {})})


def authorize(root, task_name, configuration, *, execute=None, _held_lease=None):
    """Gather read-only decisions for all findings while the worker is stopped."""
    root = Path(root).resolve()
    with file_lock(root / "build/recovery/driver.lock", timeout=0, held=_held_lease):
        store = TaskStore(root)
        task = store.read(task_name)
        policy(configuration, task)
        pending = store.status(task_name, store.scope(task_name)[0])["pending_discoveries"]
        if not pending:
            return {"status": "no_pending"}
        before = epoch(root, task_name)
        decisions = []
        for _ in pending:
            if len(decisions) >= configuration["max_decisions"]:
                return {"status": "deferred", "decision": {"action": "budget_exhausted"}}
            decision = coordinate(root, task_name, configuration, execute=execute,
                                  investigating=[item["discovery_id"] for item in decisions],
                                  allowed_actions=("investigate", "investigate_local", "defer"))
            if epoch(root, task_name) != before:
                raise ValueError("source or decisions changed during investigation planning")
            if decision["action"] not in ("investigate", "investigate_local"):
                return {"status": "deferred", "decision": decision}
            decisions.append(decision)
        return {"status": "authorized", "authorization": {
            "schema_version": 1, "kind": "investigation_round", "request_id": uuid.uuid4().hex,
            "root": str(root), "task": task_name, "author": task.author, "epoch": before,
            "delegation_sha256": digest(configuration), "investigations": decisions}}


def validate(root, task_name, permission):
    """Validate a trusted coordinator permission, without consuming it."""
    root = Path(root).resolve()
    fields = {"schema_version", "kind", "request_id", "root", "task", "author", "epoch",
              "delegation_sha256", "investigations"}
    if (not isinstance(permission, dict) or set(permission) != fields
            or type(permission["schema_version"]) is not int or permission["schema_version"] != 1
            or permission["kind"] != "investigation_round"
            or not isinstance(permission["request_id"], str)
            or not re.fullmatch(r"[0-9a-f]{32}", permission["request_id"])):
        raise ValueError("invalid investigation round permission")
    store = TaskStore(root)
    task = store.read(task_name)
    if permission["root"] != str(root) or permission["task"] != task_name or permission["author"] != task.author:
        raise ValueError("investigation permission names another worktree or task coordinator")
    require_digest(permission["epoch"])
    require_digest(permission["delegation_sha256"])
    if permission["epoch"] != epoch(root, task_name):
        raise ValueError("stale investigation permission; source or task decisions changed")
    pending = store.status(task_name, store.scope(task_name)[0])["pending_discoveries"]
    values = permission["investigations"]
    if not pending or not isinstance(values, list) or len(values) != len(pending):
        raise ValueError("investigation permission must cover every pending discovery")
    grants = {}
    for item in values:
        if (not isinstance(item, dict)
                or set(item) != {"action", "published", "discovery_id", "snapshot", "evidence",
                                 "ready_for_collection", "verification"}
                or item["action"] not in ("investigate", "investigate_local")
                or item["published"] is not False or item["ready_for_collection"] is not False
                or item["verification"] != "not_measured"):
            raise ValueError("investigation permission cannot contain resolution or acceptance")
        require_digest(item["snapshot"])
        validate_response({key: item[key] for key in ("snapshot", "action", "discovery_id", "evidence")}
                          | {"subjects": [], "editable_files": []}, item["snapshot"])
        if item["discovery_id"] in grants:
            raise ValueError("duplicate discovery investigation permission")
        grants[item["discovery_id"]] = item
    if set(grants) != {item["discovery_id"] for item in pending}:
        raise ValueError("investigation permission does not match pending discoveries")
    # Recheck origin/scope constraints independently of the supplied decisions.
    known = {row["entity_id"] for row in source_snapshot(root)["records"]}
    selected = store.scope(task_name)[0]
    for finding in pending:
        if not set(finding["affects"]) <= known:
            raise ValueError("unknown originals cannot receive an investigation permission")
        if grants[finding["discovery_id"]]["action"] == "investigate" and not set(finding["affects"]) <= selected:
            raise ValueError("ordinary investigation requires current task scope")
    if permission["epoch"] != epoch(root, task_name):
        raise ValueError("source or decisions changed while validating investigation permission")
    return grants


def consume(root, task_name, permission):
    """Called under the driver lease before any check/provider invocation."""
    grants = validate(root, task_name, permission)
    directory = artifact_parent(Path(root), "build/recovery/investigation-uses",
                                excluded_roots=SourceConfiguration.read(root).roots)
    try:
        publish_json(directory / (permission["request_id"] + ".json"),
                     {"permission_sha256": digest(permission), "task": task_name})
    except FileExistsError as error:
        raise ValueError("investigation permission already consumed; obtain fresh coordination") from error
    if permission["epoch"] != epoch(root, task_name):
        raise ValueError("source or decisions changed while consuming investigation permission")
    return grants
