"""Bounded recovery execution through configured, trusted command adapters.

Commands exchange JSON over stdin/stdout, never shell fragments. Workers must
run in an externally sandboxed worktree: scope checks detect violations, they
are not a security sandbox. This driver never commits, collects or discards work.
"""

from dataclasses import asdict, dataclass
import hashlib
import os
from pathlib import Path
import re
import subprocess
import uuid

from .locking import file_lock
from .agent_protocol import validate_reviewer_response, validate_worker_response
from .migration import GateExit
from .migration_protocol import admit, extend_inventory, permissions
from .model import canonical, require_text
from .records import parse_json
from .processes import run_bounded
from .reviews import ReviewStore, acceptance_result, merge_interpretations, pending_interpretations, validate_review, validate_supersessions
from .tasks import TaskStore


@dataclass(frozen=True)
class Command:
    argv: tuple[str, ...]
    timeout: int

    @classmethod
    def read(cls, value):
        if not isinstance(value, dict) or set(value) != {"argv", "timeout"}:
            raise ValueError("command needs argv and timeout")
        if not isinstance(value["argv"], list) or not value["argv"]:
            raise ValueError("command argv must be a nonempty array, not a shell string")
        for argument in value["argv"]:
            require_text(argument, "command argument")
        if type(value["timeout"]) is not int or not 1 <= value["timeout"] <= 3600:
            raise ValueError("command timeout must be 1..3600 seconds")
        return cls(tuple(value["argv"]), value["timeout"])


def invoke(command: Command, root: Path, task: str, payload: dict) -> tuple[int, dict]:
    def expand(match):
        name = match[1]
        if name == "task":
            return task
        if name == "root":
            return str(root)
        if name.startswith("env:") and name[4:] in os.environ:
            return os.environ[name[4:]]
        raise ValueError(f"missing command input: {name}")
    argv = [re.sub(r"\{(task|root|env:[A-Za-z_][A-Za-z_0-9]*)\}", expand, argument)
            for argument in command.argv]
    result = run_bounded(argv, cwd=root, input=canonical(payload), timeout=command.timeout)
    if result.returncode not in (0, 1, 3):
        raise ValueError(f"adapter exited {result.returncode}: {result.stderr.decode(errors='replace')[-2000:]}")
    return result.returncode, parse_json(result.stdout)


def working_state(root: Path) -> dict:
    def git(*args):
        return subprocess.run(["git", *args], cwd=root, capture_output=True, check=True).stdout
    paths = git("ls-files", "-z", "--cached", "--others", "--exclude-standard").split(b"\0")
    files = {}
    for raw in sorted(set(paths) - {b""}):
        name = os.fsdecode(raw)
        path = root / name
        if path.is_symlink():
            files[name] = ("symlink", os.readlink(path))
        elif path.is_file():
            files[name] = (path.stat().st_mode, hashlib.sha256(path.read_bytes()).hexdigest())
        elif path.exists():
            raise ValueError(f"unsupported worktree input: {name}")
        else:
            files[name] = None
    return {"head": git("rev-parse", "HEAD"), "index": git("ls-files", "--stage", "-z"), "files": files}


def changes(before: dict, after: dict) -> set[str]:
    if before["head"] != after["head"] or before["index"] != after["index"]:
        raise ValueError("adapter changed Git HEAD or index; preserve work and inspect manually")
    return {name for name in before["files"].keys() | after["files"].keys()
            if before["files"].get(name) != after["files"].get(name)}


def drive(root: Path, task_name: str, configuration: dict, worker_configuration: dict, *,
          iterations: int, reviewer_configuration: dict | None = None,
          migration_configuration: dict | None = None, execute=invoke) -> dict:
    """Recompute context/checks, edit, recheck, repeat; discoveries stop for review."""
    if type(iterations) is not int or not 1 <= iterations <= 100:
        raise ValueError("iterations must be 1..100")
    required = {"schema_version", "target", "context", "check"}
    if (not required <= set(configuration) or set(configuration) - required - {"acceptance", "obligations", "migration_check", "build"}
            or type(configuration["schema_version"]) is not int or configuration["schema_version"] != 1):
        raise ValueError("invalid driver configuration")
    if set(worker_configuration) != {"author", "command"}:
        raise ValueError("worker configuration needs author and command")
    require_text(worker_configuration["author"], "worker author")
    context_command = Command.read(configuration["context"])
    check_command = Command.read(configuration["check"])
    build_command = Command.read(configuration["build"]) if "build" in configuration else None
    worker_command = Command.read(worker_configuration["command"])
    reviewer_command = None
    if reviewer_configuration is not None:
        if set(reviewer_configuration) != {"author", "command"}:
            raise ValueError("reviewer configuration needs author and command")
        require_text(reviewer_configuration["author"], "reviewer author")
        if reviewer_configuration["author"] == worker_configuration["author"]:
            raise ValueError("reviewer must be independent of the worker")
        reviewer_command = Command.read(reviewer_configuration["command"])
    acceptance = Command.read(configuration["acceptance"]) if "acceptance" in configuration else None
    inventory_command = Command.read(configuration["obligations"]) if "obligations" in configuration else None
    migration_command = Command.read(configuration["migration_check"]) if "migration_check" in configuration else None
    if migration_configuration is not None and (inventory_command is None or migration_command is None):
        raise ValueError("migration requires source obligations and migration-check adapters")
    root = root.resolve()
    store = TaskStore(root)
    with file_lock(root / "build/recovery/driver.lock", timeout=0):
        task = store.read(task_name)
        if task.target != configuration["target"]:
            raise ValueError("task target disagrees with configured target adapter")
        selected, editable = store.scope(task_name)
        permission = permissions(migration_configuration, task, selected) if migration_configuration is not None else None
        protected = {}
        initial_decisions = store.decisions(task_name)
        reviews = ReviewStore(root, task_name)
        policy = {"project": configuration, "worker": worker_configuration, "reviewer": reviewer_configuration}
        if permission is not None:
            policy["migration"] = permission
        review_feedback = None
        acceptance_feedback = None
        interpretations = {}
        # Existing scoped edits are resumable. Unrelated dirty work is not the
        # runner's property and must be checkpointed before dispatch.
        dirty = subprocess.run(["git", "diff", "--name-only", "HEAD", "-z"], cwd=root,
                               capture_output=True, check=True).stdout.split(b"\0")
        untracked = subprocess.run(["git", "ls-files", "--others", "--exclude-standard", "-z"],
                                   cwd=root, capture_output=True, check=True).stdout.split(b"\0")
        if {os.fsdecode(name) for name in dirty + untracked if name} - editable:
            raise ValueError("checkpoint unrelated changes before running this task")
        def finish(reason, rounds, check=None, *, current=True):
            return {"task_id": task_name, "stop_reason": reason, "worker_rounds": rounds,
                    "check": check, "check_is_current": check is not None and current,
                    **({"acceptance_feedback": acceptance_feedback} if acceptance_feedback else {}),
                    "ready_for_collection": False, "persisted_results": False}
        def readonly(command, payload):
            before = working_state(root)
            try:
                code, output = execute(command, root, task_name, payload)
            finally:
                if changes(before, working_state(root)):
                    raise ValueError("read-only adapter edited the worktree; preserve work and inspect manually")
            return code, output
        def refresh_obligations():
            if inventory_command is not None:
                code, inventory = readonly(inventory_command, {"task": task.to_dict()})
                if code != 0:
                    raise ValueError("source restoration inventory unavailable")
                extend_inventory(protected, inventory, selected)
                merge_interpretations(interpretations, inventory.get("interpretations", []), selected)
                policy["interpretations"] = [interpretations[key] for key in sorted(interpretations)]
            return [{**asdict(item), "obligation_id": key} for key, item in sorted(protected.items())]
        def review_candidate(context, check, round_index, *, structural_handoff=False):
            nonlocal review_feedback, acceptance_feedback
            if reviewer_command is None:
                return finish("independent_review_required", round_index, check)
            snapshot = reviews.snapshot(working_state(root), policy)
            decisions = reviews.decisions()
            current = [value for value in decisions if value["snapshot"] == snapshot
                       and value["author"] == worker_configuration["author"]
                       and value["reviewer"] == reviewer_configuration["author"]
                       and (value["decision"] != "approve" or not pending_interpretations(value, interpretations))]
            if current:
                decision = current[-1]
            else:
                code, response = readonly(reviewer_command, {"task": task.to_dict(), "context": context,
                                                            "check": check, "snapshot": snapshot,
                                                            "instructions": ["Review structure and source evidence independently.",
                                                                             "Do not edit files, stamp claims or infer correctness from a byte match.",
                                                                             "Explicitly address every historical review_interpretation using supersessions or evidence-backed retractions; neither waives proof obligations.",
                                                                             "Return reasoning and evidence references, never copied image bytes or disassembly."]})
                if code != 0:
                    output = finish("review_unavailable", round_index, check)
                    output["agent_error"] = response
                    return output
                validate_reviewer_response(response, snapshot)
                decision = {"kind": "review", "author": worker_configuration["author"],
                            "reviewer": reviewer_configuration["author"], **response}
                validate_review(decision)
                validate_supersessions(decision, interpretations)
                if decision["snapshot"] != snapshot or reviews.snapshot(working_state(root), policy) != snapshot:
                    raise ValueError("review refers to stale source")
                reviews.append(decision, expected_revision=len(decisions))
            validate_supersessions(decision, interpretations)
            review_feedback = decision
            if decision["decision"] == "request_changes":
                return None  # Feed the independent findings to the next worker round.
            pending = pending_interpretations(decision, interpretations)
            if pending:
                output = finish("interpretation_review_incomplete", round_index, check)
                output.update(review=decision, pending_interpretations=pending)
                return output  # A generic approval cannot silently discard structural history.
            migration_report = None
            if build_command is not None and check["build_check"]["result"] != "pass":
                output = finish("build_verification_required", round_index, check)
                output["review"] = decision
                return output  # Review cannot waive an independently failed or unknown build.
            if permission is not None:
                obligations = refresh_obligations()
                if reviews.snapshot(working_state(root), policy) != snapshot:
                    raise ValueError("source or interpretation inventory changed after review")
                code, measured = readonly(migration_command, {"task": task.to_dict(), "snapshot": snapshot,
                                                              "protected": obligations, "permission": permission,
                                                              "review": decision, "interpretations": policy.get("interpretations", [])})
                if reviews.snapshot(working_state(root), policy) != snapshot:
                    raise ValueError("source changed after migration review")
                migration_report = admit(permission, tuple(protected.values()), selected, decision, snapshot, code, measured)
                if migration_report.exit_code == GateExit.TRACKED_REGRESSIONS:
                    output = finish("migration_admitted", round_index, check)
                    output.update(review=decision, migration=asdict(migration_report))
                    return output  # Checkpoint only, never full acceptance or collection.
            if check["result"] != "pass" and not structural_handoff:
                output = finish("structural_checkpoint_reviewed", round_index, check)
                output["review"] = decision
                return output  # Approval does not admit a matching regression.
            if acceptance is None:
                output = finish("full_acceptance_gate_required", round_index, check)
            else:
                obligations = refresh_obligations()
                if reviews.snapshot(working_state(root), policy) != snapshot:
                    raise ValueError("source or interpretation inventory changed after review")
                code, report = readonly(acceptance, {"task": task.to_dict(), "snapshot": snapshot,
                                                     "protected": obligations, "review": decision,
                                                     "interpretations": policy.get("interpretations", [])})
                if reviews.snapshot(working_state(root), policy) != snapshot:
                    raise ValueError("source changed after review")
                result = acceptance_result(code, report)
                acceptance_feedback = ({"snapshot": snapshot, "report": report}
                                       if result == "fail" else None)
                if result == "fail" and round_index < iterations:
                    # A current full-gate counterexample needs another scoped
                    # worker round, not manual restart after a passing review.
                    # Unknown proof capabilities still stop; no blind edits.
                    return None
                output = finish("ready_for_collection" if result == "pass" else "acceptance_" + result,
                                round_index, check)
                output["acceptance"] = report
                output["ready_for_collection"] = result == "pass"
            output["review"] = decision
            if migration_report is not None:
                output["migration"] = asdict(migration_report)
            return output
        for round_index in range(iterations + 1):
            state = store.status(task_name, selected)
            if state["pending_discoveries"]:
                return finish(state["next_action"], round_index)
            round_state = working_state(root)
            code, context = readonly(context_command, {"task": task.to_dict()})
            if code != 0:
                return finish("context_unavailable", round_index)
            obligations = refresh_obligations()
            if inventory_command is not None:
                context = {**context, "protected_obligations": obligations,
                           "review_interpretations": policy["interpretations"]}
            if permission is not None:
                context = {**context, "migration_permission": permission}
            code, check = readonly(check_command, {"task": task.to_dict(), "context": context})
            if changes(round_state, working_state(root)):
                raise ValueError("source changed between context generation and checking")
            result = check.get("result")
            if result not in ("pass", "fail", "unknown") or code != {"pass": 0, "fail": 1, "unknown": 3}[result]:
                raise ValueError("checker result and exit code disagree")
            if build_command is not None:
                build_code, build = readonly(build_command, {"task": task.to_dict(), "context": context})
                build_result = build.get("result")
                if build_result not in ("pass", "fail", "unknown") or build_code != {"pass": 0, "fail": 1, "unknown": 3}[build_result]:
                    raise ValueError("build checker result and exit code disagree")
                combined = ("unknown" if "unknown" in (result, build_result) else
                            "fail" if "fail" in (result, build_result) else "pass")
                check = {**check, "recovery_result": result, "build_check": build, "result": combined}
                result = combined
                if changes(round_state, working_state(root)):
                    raise ValueError("source changed between recovery and build checking")
            review_requested = (result == "unknown" and check.get("next_action") == "review_structure"
                                and (build_command is None or check["build_check"]["result"] == "pass"))
            if result == "pass" or review_requested:
                reviewed = review_candidate(context, check, round_index, structural_handoff=review_requested)
                if reviewed is not None:
                    return reviewed
            if round_index == iterations:
                if result != "pass" and not review_requested and reviewer_command is not None:
                    reviewed = review_candidate(context, check, round_index)
                    if reviewed is not None:
                        return reviewed
                return finish("iteration_budget_exhausted", round_index, check)
            before = working_state(root)
            try:
                code, response = execute(worker_command, root, task_name,
                                         {"task": task.to_dict(), "editable_files": sorted(editable),
                                          "context": context, "check": check, "round": round_index + 1,
                                          "review_feedback": review_feedback,
                                          "acceptance_feedback": acceptance_feedback,
                                          "instructions": ["Edit only authorized files; never run Git mutations.",
                                                           "Report structural discoveries even outside task scope.",
                                                           "Acceptance feedback belongs to its recorded snapshot; use it to investigate, not as current proof after edits.",
                                                           "Do not erase source mappings or restoration obligations."]})
            finally:
                changed = changes(before, working_state(root))
                if changed - editable:
                    raise ValueError("worker exceeded editing scope; preserve work and inspect manually")
            if store.read(task_name) != task or store.decisions(task_name) != initial_decisions:
                raise ValueError("task decisions changed during dispatch; inspect before resuming")
            if code != 0:
                output = finish("worker_interrupted", round_index + 1, check, current=False)
                output["agent_error"] = response
                return output
            validate_worker_response(response)
            for finding in response["discoveries"]:
                store.discover(task_name, request_id="worker-" + uuid.uuid4().hex,
                               author=worker_configuration["author"], expected_revision=len(store.decisions(task_name)),
                               **finding)
            if response["discoveries"]:
                return finish(store.status(task_name, selected)["next_action"], round_index + 1, check, current=False)
            if not changed:
                if permission is not None and reviewer_command is not None:
                    reviewed = review_candidate(context, check, round_index + 1)
                    if reviewed is not None:
                        return reviewed
                return finish("no_source_progress", round_index + 1, check)
        raise AssertionError("bounded loop did not terminate")
