"""Fresh native prerequisites for a reviewed structural checkpoint.

This measures admission inputs; it never approves a review, waives obligations,
or establishes final runtime/structural acceptance. Outside-scope verification
requires qualified linked preservation in addition to unchanged source bindings,
compiled residuals and a fresh current build. Unchanged build policy alone cannot
prove archive/COMDAT/common selection or equivalence of the running program.
"""

from pathlib import Path
from contextlib import ExitStack
import hashlib
import json

from .build_check import Build, check_build
from .build_policy import build_policy_changes, unchanged_build_policy
from .build_graph import application_graph
from .claim_integrity import check_claims
from .driver import working_state
from .mapping_check import check_mappings
from .measurement import Measurement
from .migration_protocol import extend_inventory, permissions
from .model import Result, digest, require_digest
from .preservation import check_preservation
from .records import read_json
from .restoration import measure_restoration
from .reviews import ReviewStore, merge_interpretations, pending_interpretations, validate_review
from .source_audits import _validate_policy, audit_source
from .source_integrity import check_source
from .source_obligations import task_obligations
from .structural_requirements import declared_outcome
from .tasks import TaskStore


def _all(values):
    values = tuple(values)
    return (Result.UNKNOWN if not values or Result.UNKNOWN in values else
            Result.FAIL if Result.FAIL in values else Result.PASS)


def measure_migration(root: Path, task_name: str, image: Path, request: dict, *, progress=None) -> dict:
    root = root.resolve()
    image = image if image.is_absolute() else root / image
    fields = {"task", "snapshot", "protected", "permission", "review", "review_policy", "interpretations"}
    if not isinstance(request, dict) or set(request) != fields:
        raise ValueError("invalid native migration request")
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    scope, _ = store._scope(task, decisions)
    if digest(request["task"]) != digest(task.to_dict()):
        raise ValueError("migration request does not name the current task")
    require_digest(request["snapshot"])
    if not isinstance(request["review_policy"], dict):
        raise ValueError("migration requires the reviewed policy")
    integration = request["review_policy"].get("integration")
    if integration is not None:
        from .integration import validate
        validate(root, task, integration)
    permissions(request["permission"], task, scope)
    if digest(request["review_policy"].get("migration")) != digest(request["permission"]):
        raise ValueError("migration permission must exactly match the independently reviewed policy")
    validate_review(request["review"])
    if integration is not None and task.author in {request["review"]["author"], request["review"]["reviewer"]}:
        raise ValueError("integration coordinator must be independent of worker and reviewer")
    if request["permission"].get("generated_changes") and task.author in {
            request["review"]["author"], request["review"]["reviewer"]}:
        raise ValueError("generated-change coordinator must be independent of worker and reviewer")
    if request["review"]["decision"] != "approve" or request["review"]["snapshot"] != request["snapshot"]:
        raise ValueError("migration requires the current independent approval")
    # The driver normally stops for discoveries before requesting admission,
    # but this verifier is also a public standalone boundary. Generic approval
    # (including permission for a matching regression) cannot resolve a finding.
    # Only inspect pending decisions here; source mapping completeness is checked
    # separately by the native mapping/restoration prerequisites below.
    pending = store.status(task_name, scope)["pending_discoveries"]
    if pending:
        raise ValueError("migration requires resolution of pending discoveries: "
                         + ", ".join(item["discovery_id"] for item in pending))
    inventory = task_obligations(root, task_name)
    protected = {}
    extend_inventory(protected, {"obligations": request["protected"]}, scope)
    extend_inventory(protected, inventory, scope)
    interpretations = {}
    merge_interpretations(interpretations, request["interpretations"], scope)
    merge_interpretations(interpretations, inventory["interpretations"], scope)
    if pending_interpretations(request["review"], interpretations):
        raise ValueError("migration has unresolved historical interpretations")
    settings, build = Measurement.read(root), Build.read(root)
    project = read_json(root / "recovery/project.json")
    policy_path = settings.source.path(root, project.get("audits"))
    policy = read_json(policy_path)
    _validate_policy(policy)
    def snapshot():
        state = working_state(root)
        journal = store.journal.stream_directory("task:" + task_name)
        paths = [root / "recovery/tasks" / (task_name + ".json"), *journal.glob("*.json")]
        if (journal.resolve() != journal or journal.exists() and not journal.is_dir()
                or any(path.resolve() != path or not path.is_file() for path in paths)):
            raise ValueError("migration requires canonical regular task decision files")
        decision_inputs = {path.relative_to(root).as_posix() for path in paths}
        if any(state["files"].get(name) is None for name in decision_inputs):
            raise ValueError("migration task decisions must participate in the review snapshot; unignore them and obtain fresh review")
        if ReviewStore(root, task_name).snapshot(state, request["review_policy"]) != request["snapshot"]:
            raise ValueError("migration inputs changed from the reviewed source snapshot")
        with image.open("rb") as stream:
            image_sha = hashlib.file_digest(stream, "sha256").hexdigest()
        if image_sha != inventory["image_sha256"]:
            raise ValueError("migration requires the pinned original image")
        return digest({"state": {"files": state["files"], "head": state["head"].hex(), "index": state["index"].hex()},
                       "image_sha256": image_sha,
                       "measurement": settings.snapshot(root, request),
                       "build": build.snapshot(root), "inventory": task_obligations(root, task_name),
                       "audit_policy": read_json(policy_path)})

    before = snapshot()
    diagnostics = []

    def diagnose(name, report):
        if report["result"] != Result.PASS:
            text = json.dumps(report, ensure_ascii=False, sort_keys=True)
            if len(text) > 12000:
                text = text[:6000] + "\n[diagnostic truncated]\n" + text[-6000:]
            diagnostics.append({"check": name, "result": report["result"], "evidence": [text]})

    def fresh():
        if snapshot() != before or store.read(task_name) != task or store.decisions(task_name) != decisions:
            raise ValueError("migration inputs changed; preserve source and recheck")

    def run(name, operation):
        fresh()
        if progress:
            progress({"check": name, "event": "start"})
        try:
            report = operation()
            if not isinstance(report, dict) or report.get("result") not in tuple(Result):
                raise ValueError("native migration prerequisite returned no result")
        except (OSError, ValueError) as error:
            report = {"result": Result.UNKNOWN, "reason": str(error)}
        fresh()
        diagnose(name, report)
        if progress:
            progress({"check": name, "event": "finish", "result": report["result"]})
        return report

    # New commands/link options cannot inherit an outside-preservation claim
    # merely because both source versions compile with today's changed policy.
    # Unknown/untracked historical policy inputs refuse this prerequisite.
    # The measurement compiler may use different helpers from the application
    # build. Those helpers must also retain their baseline contents; otherwise
    # both replicas could agree only because today's compiler adapter changed.
    policy_change = build_policy_changes(root, task, settings, build, integration=integration)
    unchanged_policy = policy_change["unchanged"]
    generated_permission = ({"permission": request["permission"]}
                            if request["permission"].get("generated_changes") else {})
    if not unchanged_policy and not policy_change["graph_only"]:
        diagnose("baseline-build-policy", {"result": Result.UNKNOWN,
                                           "reason": "compiler/build inputs differ from or lack the authorized preservation baseline; preserve work and request coordinator evidence"})
    checks = [run("source", lambda: check_source(root)), run("mappings", lambda: check_mappings(root)),
              run("claim-identities", lambda: check_claims(root))]
    checks += [run("audit:" + name, lambda name=name: audit_source(root, name)) for name in sorted(policy["audits"])]
    try:
        with ExitStack() as contexts:
            application = None

            def graph_evidence():
                nonlocal application
                application = contexts.enter_context(application_graph(root, task_name, integration=integration,
                                                                        **generated_permission))
                return application.report

            graph = (run("build-graph", graph_evidence)
                     if policy_change["graph_only"] or generated_permission or
                        unchanged_policy and build.compile_link is not None else None)
            built = graph.get("current_build") if graph is not None else None
            if not isinstance(built, dict) or built.get("result") != Result.PASS:
                built = run("build", lambda: check_build(root))
            if built["result"] != Result.PASS:
                return {"snapshot": request["snapshot"], "changed": sorted(scope), "build": built["result"],
                        "structure": Result.UNKNOWN, "references": Result.UNKNOWN, "outside_scope": Result.UNKNOWN,
                        "measurements": [], "current": {}, "diagnostics": diagnostics}
            restored = run("restoration", lambda: measure_restoration(
                root, task_name, image, review=request["review"], review_snapshot=request["snapshot"]))
            preservation = run("preservation", lambda: check_preservation(
                root, task_name, **({"integration": integration} if integration is not None else {}),
                **({"application": application} if application is not None else {}), **generated_permission))
            linked_preservation = (graph["result"] if graph is not None else
                                   preservation.get("outside_scope_verification", Result.UNKNOWN))
            if linked_preservation not in tuple(Result):
                linked_preservation = Result.UNKNOWN
            if graph is None:
                diagnose("linked-preservation", {
                    "result": linked_preservation, "compiled_residual": preservation["result"],
                    "reason": "outside-scope linked preservation requires fresh baseline/current selection and contribution evidence; "
                              "compiled residual agreement, unchanged build policy and a successful current link do not establish it",
                    "next_action": "preserve the candidate and obtain native linked-selection/residual verification against the original "
                                   "task or integration baseline; reviews and matching-regression permissions cannot waive this prerequisite"})
            structural = (declared_outcome(restored.get("structural_requirements", []), interpretations,
                                           review=request["review"])[0] if interpretations else Result.PASS)
            measurements = {row["obligation_id"]: row for row in restored.get("measurements", [])}
            current = {key: value for key, value in restored.get("current", {}).items() if key in protected}
            for key in sorted(protected.keys() - measurements.keys()):
                current[key] = digest({"inputs": before, "obligation": key})
                measurements[key] = {"obligation_id": key, "fingerprint": current[key], "result": Result.UNKNOWN,
                                     "evidence": "protected requirement has no current native measurement"}
            fresh()
            return {"snapshot": request["snapshot"], "changed": sorted(scope), "build": built["result"],
                    "structure": structural, "references": restored.get("references", Result.UNKNOWN),
                    "outside_scope": (Result.FAIL if graph is not None and graph["result"] == Result.FAIL else
                                      _all([*(row["result"] for row in checks), preservation["result"], linked_preservation,
                                            Result.PASS if graph is not None or unchanged_policy else Result.UNKNOWN])),
                    "measurements": [measurements[key] for key in sorted(protected)], "current": current,
                    "diagnostics": diagnostics}
    finally:
        # Context cleanup is part of the proof epoch, not post-verification
        # housekeeping. A refused close cannot publish an earlier report.
        fresh()
