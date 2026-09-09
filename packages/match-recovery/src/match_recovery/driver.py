"""Bounded recovery execution through configured, trusted command adapters.

Commands exchange JSON over stdin/stdout, never shell fragments. Workers must
run in an externally sandboxed worktree: scope checks detect violations, they
are not a security sandbox. Only explicit coordinator authorization permits local
verified checkpoint commits. This driver never collects or discards work.
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
from .migration import AdmissionRefused, GateExit
from .migration_protocol import admit, extend_inventory, permissions, validate_checkpoint_feedback
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


def _resumable_history(root: Path, store: TaskStore, reviews: ReviewStore, task_name: str) -> set[str]:
    """Allow this task's valid append-only decisions, not arbitrary dirty history.

    Decisions retain the trusted-coordinator boundary of TaskStore/ReviewStore;
    content hashes are integrity checks, not authentication. Git protects the
    committed prefix. No checkpoint, acceptance result or new baseline is made.
    """
    store.decisions(task_name)
    reviews.decisions()
    allowed = set()
    for journal, stream in ((store.journal, "task:" + task_name), (reviews.journal, reviews.stream)):
        directory = journal.stream_directory(stream)
        events = journal.read(stream)
        paths = {directory / f"{event.sequence:08d}.json" for event in events}
        if directory.resolve() != directory:
            raise ValueError("resumable history cannot use symlinked directories")
        for path in paths:
            if not path.is_file() or path.is_symlink() or path.stat().st_mode & 0o111:
                raise ValueError("resumable history needs regular decision files")
        names = {path.relative_to(root).as_posix() for path in paths}
        committed = subprocess.run(
            ["git", "ls-tree", "-rz", "HEAD", "--", directory.relative_to(root).as_posix()],
            cwd=root, capture_output=True, check=True).stdout
        for entry in committed.split(b"\0"):
            if not entry:
                continue
            metadata, raw_name = entry.split(b"\t", 1)
            mode, kind, blob = metadata.split()
            name = os.fsdecode(raw_name)
            if name not in names or kind != b"blob" or mode != b"100644":
                raise ValueError("committed task history was removed or replaced; inspect before resuming")
            original = subprocess.run(["git", "cat-file", "blob", blob.decode("ascii")],
                                      cwd=root, capture_output=True, check=True).stdout
            path = root / name
            if path.read_bytes() != original:
                raise ValueError("committed task history changed; inspect before resuming")
        allowed.update(names)
    return allowed


def drive(root: Path, task_name: str, configuration: dict, worker_configuration: dict, *,
          iterations: int, reviewer_configuration: dict | None = None,
          migration_configuration: dict | None = None,
          coordinator_configuration: dict | None = None,
          checkpoint_configuration: dict | None = None,
          integration_configuration: dict | None = None, verify_only: bool = False, execute=invoke,
          _held_lease=None, _collection=None) -> dict:
    """Recompute context/checks, edit, recheck, repeat; discoveries stop for review."""
    if type(iterations) is not int or not 1 <= iterations <= 100:
        raise ValueError("iterations must be 1..100")
    if type(verify_only) is not bool:
        raise ValueError("verify_only must be boolean")
    if verify_only and coordinator_configuration is not None:
        raise ValueError("verification-only runs cannot delegate task or scope changes")
    if _collection is not None and not verify_only:
        raise ValueError("collection verification cannot dispatch repairs")
    round_limit = 0 if verify_only else iterations
    required = {"schema_version", "target", "context", "check"}
    if (not required <= set(configuration) or set(configuration) - required - {"acceptance", "acceptance_scope", "obligations", "migration_check", "build"}
            or type(configuration["schema_version"]) is not int or configuration["schema_version"] != 1):
        raise ValueError("invalid driver configuration")
    acceptance_scope = configuration.get("acceptance_scope", "full_project")
    if acceptance_scope not in ("change", "full_project"):
        raise ValueError("acceptance_scope must be change or full_project")
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
    if (migration_configuration is not None or acceptance_scope == "change") and (inventory_command is None or migration_command is None):
        raise ValueError("migration requires source obligations and migration-check adapters")
    root = root.resolve()
    store = TaskStore(root)
    with file_lock(root / "build/recovery/driver.lock", timeout=0, held=_held_lease):
        task = store.read(task_name)
        integration = None
        if integration_configuration is not None:
            from .integration import validate
            if acceptance_scope != "change":
                raise ValueError("integration context currently requires change acceptance scope")
            if task.author in {worker_configuration["author"], reviewer_configuration["author"] if reviewer_configuration else None}:
                raise ValueError("integration coordinator must be independent of worker and reviewer")
            integration = validate(root, task, integration_configuration)
        publication = publication_branch = None
        if checkpoint_configuration is not None:
            from .checkpoints import authorization, preflight
            if acceptance_scope != "change":
                raise ValueError("checkpoint publication currently requires change acceptance scope")
            publication = authorization(checkpoint_configuration, task.author, worker_configuration["author"],
                                        reviewer_configuration["author"] if reviewer_configuration else None)
            publication_branch = preflight(root)
        coordination_count, coordination_feedback = 0, None
        if coordinator_configuration is not None:
            from .coordination import coordinate, policy as coordination_policy
            coordination_policy(coordinator_configuration, task)
            if task.author in {worker_configuration["author"],
                              reviewer_configuration["author"] if reviewer_configuration else None}:
                raise ValueError("coordinator must be separate from worker and reviewer")
        if task.target != configuration["target"]:
            raise ValueError("task target disagrees with configured target adapter")
        selected, editable = store.scope(task_name)
        if acceptance_scope == "change" and migration_configuration is None:
            # Ordinary change acceptance grants NO regression permission. Only
            # an explicit coordinator decision can relax matching temporarily.
            migration_configuration = {"migration_id": "change:" + task_name, "author": task.author,
                                       "evidence": ["authorized task:" + task_name], "allowed": []}
        permission = permissions(migration_configuration, task, selected) if migration_configuration is not None else None
        protected = {}
        initial_decisions = store.decisions(task_name)
        reviews = ReviewStore(root, task_name)
        policy = {"project": configuration, "worker": worker_configuration, "reviewer": reviewer_configuration}
        if verify_only:
            policy["verification_only"] = True
        if _collection is not None:
            policy["collection"] = _collection
        if integration is not None:
            policy["integration"] = integration
        if publication is not None:
            policy["checkpoint"] = publication
        if coordinator_configuration is not None:
            policy["coordinator"] = coordinator_configuration
        if permission is not None:
            policy["migration"] = permission
        review_feedback = None
        acceptance_feedback = None
        checkpoint_feedback = None
        interpretations = {}
        # Scoped source and validated append-only decisions survive interruption.
        # They are not worker permission to edit history or unrelated dirty work.
        history = _resumable_history(root, store, reviews, task_name)
        dirty = subprocess.run(["git", "diff", "--name-only", "HEAD", "-z"], cwd=root,
                               capture_output=True, check=True).stdout.split(b"\0")
        untracked = subprocess.run(["git", "ls-files", "--others", "--exclude-standard", "-z"],
                                   cwd=root, capture_output=True, check=True).stdout.split(b"\0")
        if {os.fsdecode(name) for name in dirty + untracked if name} - editable - history:
            raise ValueError("checkpoint unrelated changes before running this task")
        def finish(reason, rounds, check=None, *, current=True):
            return {"task_id": task_name, "stop_reason": reason, "worker_rounds": rounds,
                    **({"coordinator_decisions": coordination_count, "coordination": coordination_feedback}
                       if coordinator_configuration is not None else {}),
                    "check": check, "check_is_current": check is not None and current,
                    **({"acceptance_feedback": acceptance_feedback} if acceptance_feedback else {}),
                    **({"checkpoint_feedback": checkpoint_feedback} if checkpoint_feedback else {}),
                    "acceptance_scope": acceptance_scope,
                    **({"verification_only": True} if verify_only else {}),
                    **({"integration": integration} if integration is not None else {}),
                    "ready_for_collection": False, "persisted_results": False}
        def readonly(command, payload):
            before = working_state(root)
            try:
                code, output = execute(command, root, task_name, payload)
            finally:
                if changes(before, working_state(root)):
                    raise ValueError("read-only adapter edited the worktree; preserve work and inspect manually")
            return code, output
        def checkpoint(output, expected):
            if publication is not None:
                if output["stop_reason"] == "migration_admitted" and not publication["allow_migration"]:
                    output["checkpoint"] = {"published": False, "reason": "migration_publication_not_authorized"}
                else:
                    from .checkpoints import publish
                    history = _resumable_history(root, store, reviews, task_name)
                    output["checkpoint"] = publish(root, expected, editable | history, publication_branch,
                                                    publication["message"])
            return output
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
            nonlocal review_feedback, acceptance_feedback, checkpoint_feedback
            if store.status(task_name, selected)["pending_discoveries"]:
                return finish("evaluate_structural_discoveries", round_index, check)
            if reviewer_command is None:
                return finish("independent_review_required", round_index, check)
            snapshot = reviews.snapshot(working_state(root), policy)
            decisions = reviews.decisions()
            current = [value for value in decisions if value["snapshot"] == snapshot
                       and value["author"] == worker_configuration["author"]
                       and value["reviewer"] == reviewer_configuration["author"]
                       and (value["decision"] != "approve" or not pending_interpretations(value, interpretations))]
            if current and not verify_only:
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
                if verify_only:
                    output = finish("review_changes_requested", round_index, check)
                    output["review"] = decision
                    return output
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
                verified_state = working_state(root)
                code, measured = readonly(migration_command, {"task": task.to_dict(), "snapshot": snapshot,
                                                              "protected": obligations, "permission": permission,
                                                              "review_policy": policy,
                                                              "review": decision, "interpretations": policy.get("interpretations", [])})
                if reviews.snapshot(working_state(root), policy) != snapshot:
                    raise ValueError("source changed after migration review")
                try:
                    migration_report = admit(permission, tuple(protected.values()), selected, decision, snapshot, code, measured)
                except AdmissionRefused as error:
                    if acceptance_scope != "change":
                        raise
                    checkpoint_feedback = {"snapshot": snapshot, "result": error.result,
                                           "reason": str(error), "report": measured}
                    validate_checkpoint_feedback(checkpoint_feedback)
                    actionable = (error.result == "fail" or measured.get("diagnostics") or
                                  any(row["result"] != "pass" for row in measured["measurements"]))
                    if actionable and round_index < round_limit:
                        return None  # Same scope/budget, then fresh checks and independent review.
                    output = finish("change_" + error.result, round_index, check)
                    output["review"] = decision
                    return output
                checkpoint_feedback = None
                if migration_report.exit_code == GateExit.TRACKED_REGRESSIONS:
                    output = finish("migration_admitted", round_index, check)
                    output.update(review=decision, migration=asdict(migration_report))
                    output["ready_for_checkpoint"] = True
                    return checkpoint(output, verified_state)  # Never full acceptance or collection.
                if acceptance_scope == "change":
                    # Protected history alone does not cover newly recovered
                    # bodies. Their current scoped binary check must pass too.
                    binary = check.get("candidate", {}).get("checks", {}).get("binary", {}).get("result")
                    if check["result"] != "pass" and not (structural_handoff and binary == "pass"):
                        output = finish("change_verification_required", round_index, check)
                        output.update(review=decision, migration=asdict(migration_report))
                        return output
                    output = finish("change_accepted", round_index, check)
                    output.update(review=decision, migration=asdict(migration_report), ready_for_checkpoint=True)
                    return checkpoint(output, verified_state)  # No claim about the whole application.
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
                request = {"task": task.to_dict(), "snapshot": snapshot,
                           "protected": obligations, "review": decision,
                           "review_policy": policy,
                           "interpretations": policy.get("interpretations", []),
                           "verification_id": uuid.uuid4().hex}
                code, report = readonly(acceptance, request)
                if reviews.snapshot(working_state(root), policy) != snapshot:
                    raise ValueError("source changed after review")
                result = acceptance_result(code, report, request=request)
                acceptance_feedback = ({"snapshot": snapshot, "report": report}
                                       if result != "pass" else None)
                if (result == "fail" or result == "unknown" and report.get("findings")) and round_index < round_limit:
                    # Missing evidence can have actionable diagnostics too:
                    # let the worker investigate or request scope, not waive it.
                    # No diagnostics, no source progress or an exhausted budget
                    # still stops; approval never turns an unknown into a pass.
                    return None
                output = finish("ready_for_collection" if result == "pass" else "acceptance_" + result,
                                round_index, check)
                output["acceptance"] = report
                output["ready_for_collection"] = result == "pass"
            output["review"] = decision
            if migration_report is not None:
                output["migration"] = asdict(migration_report)
            return output
        for round_index in range(round_limit + 1):
            state = store.status(task_name, selected)
            investigations = {}
            investigation_state = None
            while (any(item["discovery_id"] not in investigations for item in state["pending_discoveries"])
                   and coordinator_configuration is not None):
                if coordination_count == coordinator_configuration["max_decisions"]:
                    return finish("coordination_budget_exhausted", round_index)
                coordination_count += 1
                before_coordination = working_state(root)
                if investigations and changes(investigation_state, before_coordination):
                    raise ValueError("source changed between discovery investigation authorizations")
                coordination_feedback = coordinate(root, task_name, coordinator_configuration, execute=execute,
                                                   investigating=investigations)
                if coordination_feedback["action"] == "investigate":
                    if changes(before_coordination, working_state(root)):
                        raise ValueError("source changed after discovery investigation authorization")
                    investigation_state = before_coordination
                    investigations[coordination_feedback["discovery_id"]] = coordination_feedback
                    continue  # No resolution; every pending item needs its own scoped authorization.
                if not coordination_feedback["published"]:
                    return finish("coordination_" + coordination_feedback["action"], round_index)
                selected, editable = store.scope(task_name)
                initial_decisions = store.decisions(task_name)
                if migration_configuration is not None:
                    permission = permissions(migration_configuration, task, selected)
                    policy["migration"] = permission
                review_feedback = acceptance_feedback = checkpoint_feedback = None
                state = store.status(task_name, selected)
                investigations.clear()  # Decisions changed; obtain fresh authorization for remaining items.
            investigating = bool(state["pending_discoveries"])
            if investigating and not {item["discovery_id"] for item in state["pending_discoveries"]} <= investigations.keys():
                return finish(state["next_action"], round_index)
            round_state = working_state(root)
            if investigating and changes(investigation_state, round_state):
                raise ValueError("source changed before discovery investigation checks")
            code, context = readonly(context_command, {"task": task.to_dict()})
            if code != 0:
                return finish("context_unavailable", round_index)
            if investigating:
                context = {**context, "discovery_investigations": [investigations[key] for key in sorted(investigations)],
                           "pending_discoveries": state["pending_discoveries"]}
            obligations = refresh_obligations()
            if inventory_command is not None:
                context = {**context, "protected_obligations": obligations,
                           "review_interpretations": policy["interpretations"]}
            if permission is not None:
                context = {**context, "migration_permission": permission}
            if integration is not None:
                context = {**context, "integration_context": integration}
            if _collection is not None:
                context = {**context, "collection_context": _collection}
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
            if not investigating and (result == "pass" or review_requested):
                reviewed = review_candidate(context, check, round_index, structural_handoff=review_requested)
                if reviewed is not None:
                    return reviewed
            if round_index == round_limit:
                if investigating:
                    return finish("evaluate_structural_discoveries", round_index, check)
                if result != "pass" and not review_requested and reviewer_command is not None:
                    reviewed = review_candidate(context, check, round_index)
                    if reviewed is not None:
                        return reviewed
                return finish("verification_required" if verify_only else "iteration_budget_exhausted", round_index, check)
            before = working_state(root)
            if investigating and changes(investigation_state, before):
                raise ValueError("source changed before discovery investigation dispatch")
            try:
                code, response = execute(worker_command, root, task_name,
                                         {"task": task.to_dict(), "editable_files": sorted(editable),
                                          "context": context, "check": check, "round": round_index + 1,
                                          "review_feedback": review_feedback,
                                          "acceptance_feedback": acceptance_feedback,
                                          **({"checkpoint_feedback": checkpoint_feedback} if checkpoint_feedback else {}),
                                          "instructions": ["Edit only authorized files; never run Git mutations.",
                                                           "Discovery investigation authorizes only this bounded scoped attempt, not incorporation or acceptance. Keep pending findings and report evidence; the coordinator must resolve them against the resulting source.",
                                                           "Report structural discoveries even outside task scope.",
                                                           "Acceptance feedback belongs to its recorded snapshot; use it to investigate, not as current proof after edits.",
                                                           "Unknown acceptance findings require evidence, not speculative matching edits. Repair only authorized source; report missing capabilities or authority as discoveries without changing proof policy.",
                                                           "Checkpoint feedback is historical diagnostic evidence, not admission. Repair only scoped source; preserve justified structural work and report outside-scope effects or missing verification capabilities as discoveries.",
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
                if coordinator_configuration is not None:
                    initial_decisions = store.decisions(task_name)
                    continue  # Same worker budget; fresh scope, context and checks before dispatch.
                return finish(store.status(task_name, selected)["next_action"], round_index + 1, check, current=False)
            if not changed:
                if not investigating and permission is not None and reviewer_command is not None:
                    reviewed = review_candidate(context, check, round_index + 1)
                    if reviewed is not None:
                        return reviewed
                return finish("no_source_progress", round_index + 1, check)
        raise AssertionError("bounded loop did not terminate")
