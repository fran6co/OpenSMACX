"""Fresh native prerequisites for a reviewed structural checkpoint.

This measures admission inputs; it never approves a review, waives obligations,
or establishes final runtime/structural acceptance. Outside-scope verification
means unchanged source bindings and compiled residuals under an unchanged build
policy, plus a fresh current build, not equivalence of the whole running program.
"""

from pathlib import Path
import hashlib
import json
import os

from .build_check import Build, check_build
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
from .source_history import _tree, git
from .source_integrity import check_source
from .source_obligations import task_obligations
from .structural_requirements import declared_outcome
from .tasks import TaskStore


def _all(values):
    values = tuple(values)
    return (Result.UNKNOWN if not values or Result.UNKNOWN in values else
            Result.FAIL if Result.FAIL in values else Result.PASS)


def unchanged_build_policy(root, task, settings, build, *, integration=None):
    """Use unchanged task policy, or the explicitly authorized destination policy."""
    from .integration import preservation_base
    baseline_revision = preservation_base(root, task, integration)
    project = read_json(root / "recovery/project.json")
    def outside_source(name):
        return not any(Path(name).is_relative_to(directory) for directory in settings.source.roots)
    generators = [build]
    if "preparation" in project:
        generators.append(Build.read(root, key="preparation"))
    policy_inputs = set(project.get("compiler", {}).get("inputs", []))
    policy_roots = set()
    for generator in generators:
        policy_inputs.update(generator.snapshot(root)["inputs"])
        policy_roots.update(generator.input_roots)
    policy_files = {name for name in policy_inputs if outside_source(name)} | {"recovery/project.json"}
    baseline = {name: entry for name, entry in _tree(
        root, baseline_revision, tuple(sorted(policy_files | policy_roots))).items() if outside_source(name)}
    unchanged = set(baseline) == policy_files
    for name, (mode, kind, blob) in baseline.items():
        path = root / name
        expected = git(root, "cat-file", "blob", blob) if kind == "blob" else None
        actual = (os.fsencode(os.readlink(path)) if mode == "120000" and path.is_symlink() else
                  path.read_bytes() if mode in ("100644", "100755") and path.is_file() and not path.is_symlink() else None)
        unchanged = unchanged and expected is not None and expected == actual
    return unchanged


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
    validate_review(request["review"])
    if integration is not None and task.author in {request["review"]["author"], request["review"]["reviewer"]}:
        raise ValueError("integration coordinator must be independent of worker and reviewer")
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
    unchanged_policy = unchanged_build_policy(root, task, settings, build, integration=integration)
    if not unchanged_policy:
        diagnose("baseline-build-policy", {"result": Result.UNKNOWN,
                                           "reason": "compiler/build inputs differ from or lack the authorized preservation baseline; preserve work and request coordinator evidence"})
    checks = [run("source", lambda: check_source(root)), run("mappings", lambda: check_mappings(root)),
              run("claim-identities", lambda: check_claims(root))]
    checks += [run("audit:" + name, lambda name=name: audit_source(root, name)) for name in sorted(policy["audits"])]
    built = run("build", lambda: check_build(root))
    if built["result"] != Result.PASS:
        return {"snapshot": request["snapshot"], "changed": sorted(scope), "build": built["result"],
                "structure": Result.UNKNOWN, "references": Result.UNKNOWN, "outside_scope": Result.UNKNOWN,
                "measurements": [], "current": {}, "diagnostics": diagnostics}
    restored = run("restoration", lambda: measure_restoration(
        root, task_name, image, review=request["review"], review_snapshot=request["snapshot"]))
    preservation = run("preservation", lambda: check_preservation(
        root, task_name, **({"integration": integration} if integration is not None else {})))
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
            "outside_scope": _all([*(row["result"] for row in checks), preservation["result"],
                                   Result.PASS if unchanged_policy else Result.UNKNOWN]),
            "measurements": [measurements[key] for key in sorted(protected)], "current": current,
            "diagnostics": diagnostics}
