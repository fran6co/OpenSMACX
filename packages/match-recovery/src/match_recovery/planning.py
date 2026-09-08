"""Select source-owned work, checkpoint its authorization, then use the driver.

The read-only planner proposes, never creates tasks or accepts code. Explicit
local policy authorizes the package to publish one task decision commit. Existing
tasks retain their baselines and reservations; completion is not guessed from
claims, previous reviews, or cached acceptance. No executable bounds are inferred.
"""

from pathlib import Path, PurePosixPath
import subprocess

from .components import relative_path
from .driver import Command, _resumable_history, changes, drive, invoke, working_state
from .locking import file_lock
from .model import digest, require_digest, require_text
from .ownership_source import component_manifests, ownership_files, project_view
from .reviews import ReviewStore
from .source_config import SourceConfiguration
from .source_inventory import assert_snapshot_current
from .task_source import create_task
from .tasks import TaskStore, task_name


def policy(value):
    if (not isinstance(value, dict) or set(value) != {
            "schema_version", "author", "command", "max_entities", "max_editable_files", "commit_decision", "evidence"}
            or type(value["schema_version"]) is not int or value["schema_version"] != 1):
        raise ValueError("invalid planner delegation")
    require_text(value["author"], "planner author")
    Command.read(value["command"])
    for name in ("max_entities", "max_editable_files"):
        if type(value[name]) is not int or not 1 <= value[name] <= 10000:
            raise ValueError("planner scope limits must be integers in 1..10000")
    if value["commit_decision"] is not True:
        raise ValueError("planner needs explicit permission to commit new task decisions")
    if not isinstance(value["evidence"], list) or not value["evidence"]:
        raise ValueError("planner delegation needs authorization evidence")
    for item in value["evidence"]:
        require_text(item, "planner delegation evidence")
    return value


def validate_response(value, snapshot):
    if not isinstance(value, dict) or set(value) != {
            "snapshot", "action", "task_id", "group", "intent", "editable_files", "evidence"}:
        raise ValueError("invalid planner response")
    if value["snapshot"] != snapshot:
        raise ValueError("planner returned a stale source/decision snapshot")
    if value["action"] not in ("create", "resume", "defer"):
        raise ValueError("unsupported planner action")
    if (not isinstance(value["editable_files"], list)
            or any(not isinstance(item, str) for item in value["editable_files"])
            or len(set(value["editable_files"])) != len(value["editable_files"])):
        raise ValueError("planner editing files must be a unique array")
    for item in value["editable_files"]:
        relative_path(item)
    if not isinstance(value["evidence"], list) or not value["evidence"]:
        raise ValueError("planner choice needs evidence")
    for item in value["evidence"]:
        require_text(item, "planner choice evidence")
    if value["action"] in ("create", "resume"):
        task_name(value["task_id"])
    elif value["task_id"] != "":
        raise ValueError("deferral cannot select a task")
    if value["action"] == "create":
        require_text(value["group"], "ownership group")
        require_text(value["intent"], "task intent")
    elif value["group"] != "" or value["intent"] != "" or value["editable_files"]:
        raise ValueError("resume/defer cannot change scope or task intent")


def validate_request(value):
    if not isinstance(value, dict) or set(value) != {
            "snapshot", "target", "delegation", "groups", "tasks", "create_available", "instructions"}:
        raise ValueError("invalid planner request")
    require_digest(value["snapshot"])
    require_text(value["target"], "planner target")
    policy(value["delegation"])
    if (type(value["create_available"]) is not bool
            or any(not isinstance(value[key], list) for key in ("groups", "tasks", "instructions"))):
        raise ValueError("invalid planner context")
    for item in value["instructions"]:
        require_text(item, "planner instruction")


def _git(root, *args):
    return subprocess.run(["git", "--literal-pathspecs", *args], cwd=root, capture_output=True, check=True).stdout


def _clean(root):
    return not _git(root, "status", "--porcelain", "--untracked-files=all").strip()


def _context(root, target, delegation):
    view, source = project_view(root, allow_empty=True)
    config = SourceConfiguration.from_dict(source["source_configuration"])
    vendor_roots = [item.source_root for item in component_manifests(source)]
    dependency_files = {name for name in source["files"] if name.endswith(".recovery.h") or
                        any(PurePosixPath(name).is_relative_to(directory) for directory in vendor_roots)}
    store = TaskStore(root)
    tasks, occupied_entities, occupied_files = [], set(), set()
    directory = root / "recovery/tasks"
    if directory.resolve() != directory:
        raise ValueError("planner cannot use a symlinked task directory")
    for path in sorted(directory.glob("*.json")):
        if path.is_symlink() or not path.is_file():
            raise ValueError("planner needs regular task records")
        task = store.read(path.stem)
        _resumable_history(root, store, ReviewStore(root, task.task_id), task.task_id)
        subjects, files = store.scope(task.task_id)
        occupied_entities.update(subjects)
        occupied_files.update(files)
        eligible = (task.target == target and task.author == delegation["author"]
                    and all((item.image_sha256, item.address_space) == (source["image_sha256"], config.address_space)
                            for item in task.subjects))
        tasks.append({"task": {key: value for key, value in task.to_dict().items()
                               if key not in ("schema_version", "subjects", "editable_files")},
                      "subject_count": len(subjects), "editable_files": sorted(files), "eligible": eligible,
                      "pending_discoveries": store.status(task.task_id, subjects)["pending_discoveries"]})
    groups = []
    for group in view["groups"]:
        files = ownership_files(source, group)
        subjects = {item["entity_id"] for item in group["entities"]}
        reasons = []
        if group["next_action"] != "review_ownership_and_structure":
            reasons.append(group["next_action"])
        if not files:
            reasons.append("establish_source_home")
        if subjects & occupied_entities or set(files) & occupied_files:
            reasons.append("reserved_by_existing_task")
        if len(subjects) > delegation["max_entities"] or len(files) > delegation["max_editable_files"]:
            reasons.append("exceeds_delegated_scope_limit")
        if any(name not in source["files"] or config.is_seed(name) or name == config.image_pin for name in files):
            reasons.append("requires_source_ownership_decision")
        if set(files) & dependency_files and "verify_dependency" not in reasons:
            reasons.append("verify_dependency")
        groups.append({**{key: value for key, value in group.items() if key != "entities"},
                       "editable_files": files, "eligible": not reasons, "reasons": reasons})
    return {"target": target, "delegation": delegation, "groups": groups, "tasks": tasks,
            "create_available": _clean(root),
            "instructions": ["Remain read-only. Choose one evidence-backed ownership task to create or resume, or defer.",
                             "Byte claims and stored reviews do not prove structure or completion; inspect source evidence.",
                             "Existing task reservations stay in force. Resume them instead of duplicating or resetting their baselines.",
                             "Do not invent original extents, edit source/policy, publish decisions, commit or accept code.",
                             "Inspect detailed members/seed alternatives with recover partition --group SELECTOR --details --impact, and tasks context TASK for existing work.",
                             "Dependency, conflicting and unassigned groups need their indicated analysis, not automatic application recovery."]}, source, occupied_files | dependency_files


def _checkpoint(root, task, before):
    name = "recovery/tasks/" + task["task_id"] + ".json"
    expected = working_state(root)
    if changes(before, expected) != {name}:
        raise ValueError("task publication raced other edits; preserve work and inspect before checkpointing")
    # This is a coordinator checkpoint before any worker/measurement epoch.
    # On failure preserve the decision and current index for inspection; never reset.
    _git(root, "add", "--", name)
    _git(root, "commit", "--only", "-m", "Authorize recovery task " + task["task_id"], "--", name)
    after = working_state(root)
    if (_git(root, "rev-parse", "HEAD^") != before["head"]
            or _git(root, "diff-tree", "--no-commit-id", "--name-only", "-r", "HEAD").decode().splitlines() != [name]
            or after["files"] != expected["files"] or not _clean(root)):
        raise ValueError("task checkpoint changed unexpected state; preserve work and inspect before dispatch")
    return after["head"].decode().strip()


def select_task(root: Path, target: str, configuration: dict, *, execute=invoke):
    """One read-only selection plus optional task-only commit; never acceptance."""
    root = root.resolve()
    require_text(target, "planner target")
    delegation = policy(configuration)
    with file_lock(root / "build/recovery/driver.lock", timeout=0):
        before = working_state(root)
        request, source, occupied_files = _context(root, target, delegation)
        request["snapshot"] = digest({"request": request, "source": digest(source),
                                       "head": before["head"].hex(), "index": before["index"].hex(), "files": before["files"]})
        if changes(before, working_state(root)):
            raise ValueError("planner inputs changed during context generation")
        try:
            code, response = execute(Command.read(delegation["command"]), root, "", request)
        finally:
            if changes(before, working_state(root)):
                raise ValueError("read-only planner edited the worktree; preserve work and inspect manually")
        assert_snapshot_current(root, source)
        if code != 0:
            return {"action": "unavailable", "agent_error": response, "ready_for_collection": False}
        validate_response(response, request["snapshot"])
        result = {"action": response["action"], "evidence": response["evidence"], "ready_for_collection": False}
        if response["action"] == "defer":
            return result
        if response["action"] == "resume":
            task = next((item for item in request["tasks"] if item["task"]["task_id"] == response["task_id"] and item["eligible"]), None)
            if task is None:
                raise ValueError("planner may resume only an existing task within delegated authority")
            return {**result, "task_id": response["task_id"], "checkpoint": None}
        group = next((item for item in request["groups"] if item["selector"] == response["group"] and item["eligible"]), None)
        if group is None or not request["create_available"] or not _clean(root):
            raise ValueError("planner creation needs a clean worktree and an eligible unreserved ownership group")
        files = set(group["editable_files"]) | set(response["editable_files"])
        config = SourceConfiguration.from_dict(source["source_configuration"])
        if len(files) > delegation["max_editable_files"] or files & occupied_files:
            raise ValueError("planner exceeded delegated file scope or an existing task reservation")
        for name in files:
            path = config.path(root, name)
            if (name not in source["files"] or config.is_seed(name) or name == config.image_pin
                    or path.is_symlink() or path.resolve() != path):
                raise ValueError("planner editing authority covers regular existing product source files only")
        created = create_task(root, name=response["task_id"], group=response["group"], author=delegation["author"],
                              intent=response["intent"], target=target, editable_files=files,
                              evidence=[*response["evidence"], "Planner delegation: " + digest(delegation)])
        if not created["created"]:
            raise ValueError("planner must resume an existing task instead of recreating it")
        checkpoint = _checkpoint(root, created["task"], before)
        return {**result, "task_id": response["task_id"], "checkpoint": checkpoint}


def start(root, configuration, worker_configuration, planner_configuration, *, iterations,
          reviewer_configuration=None, coordinator_configuration=None, migration_configuration=None,
          checkpoint_configuration=None, execute=invoke):
    """Select/checkpoint one task, then run the existing bounded recovery loop."""
    delegation = policy(planner_configuration)
    if not isinstance(configuration, dict) or not isinstance(configuration.get("target"), str):
        raise ValueError("driver configuration needs a target before task selection")
    for role in (worker_configuration,) + ((reviewer_configuration,) if reviewer_configuration is not None else ()):
        if not isinstance(role, dict) or set(role) != {"author", "command"}:
            raise ValueError("recovery role needs author and command before task selection")
        require_text(role["author"], "recovery role author")
        Command.read(role["command"])
    if delegation["author"] in {worker_configuration["author"],
                                reviewer_configuration["author"] if reviewer_configuration else None}:
        raise ValueError("planner must be independent of worker and reviewer")
    if type(iterations) is not int or not 1 <= iterations <= 100:
        raise ValueError("iterations must be 1..100")
    if checkpoint_configuration is not None:
        from .checkpoints import authorization
        if configuration.get("acceptance_scope", "full_project") != "change":
            raise ValueError("checkpoint publication currently requires change acceptance scope")
        authorization(checkpoint_configuration, delegation["author"], worker_configuration["author"],
                      reviewer_configuration["author"] if reviewer_configuration else None)
    selection = select_task(root, configuration["target"], delegation, execute=execute)
    if selection["action"] not in ("create", "resume"):
        return {"stop_reason": "planning_" + selection["action"], "selection": selection, "ready_for_collection": False}
    report = drive(root, selection["task_id"], configuration, worker_configuration, iterations=iterations,
                   reviewer_configuration=reviewer_configuration, coordinator_configuration=coordinator_configuration,
                   migration_configuration=migration_configuration,
                   checkpoint_configuration=checkpoint_configuration, execute=execute)
    return {**report, "selection": selection}
