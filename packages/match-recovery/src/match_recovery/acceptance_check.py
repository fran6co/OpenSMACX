"""Native acceptance orchestration; configured proof commands remain trusted.

One fresh whole-project build is retained for structural/runtime checks. Native
matches, declared structure and outside preservation cannot be waived by those
commands. Missing capabilities yield unknown; no source claims are written.
"""

import hashlib
import os
from pathlib import Path
import re
import subprocess
from contextlib import ExitStack

from .build_check import Build
from .build_graph import application_graph, require_bound_graph
from .components import relative_path
from .driver import Command, invoke, working_state
from .fingerprint import input_snapshot
from .gate import check_project, project_inputs
from .measurement import Measurement
from .migration_check import unchanged_build_policy
from .migration_protocol import extend_inventory
from .model import Dimension, digest, require_text
from .preservation import check_preservation
from .records import read_json
from .restoration import measure_restoration
from .source_obligations import task_obligations
from .source_form import check_source_form
from .reviews import ReviewStore, acceptance_result, merge_interpretations, pending_interpretations
from .structural_requirements import declared_outcome
from .tasks import TaskStore
from .native_diagnostics import diagnostic_evidence


def _all(values):
    values = tuple(values)
    return "fail" if "fail" in values else "unknown" if not values or "unknown" in values else "pass"


def _diagnostic(name, report):
    # Shared bounded display; never replace the report consumed by admission.
    return name + ": " + "\n".join(diagnostic_evidence(report))


def _policy(root):
    config = read_json(root / "recovery/project.json").get("acceptance", {})
    if not isinstance(config, dict) or set(config) - {"checks", "inputs", "environment"}:
        raise ValueError("invalid acceptance policy")
    checks = config.get("checks", {})
    if not isinstance(checks, dict) or set(checks) - {"structure", "runtime"}:
        raise ValueError("acceptance proof commands may only verify structure or runtime")
    commands = {name: Command.read(value) for name, value in checks.items()}
    inputs, environment = config.get("inputs", []), config.get("environment", [])
    for values in (inputs, environment):
        if not isinstance(values, list) or any(not isinstance(value, str) for value in values) or len(set(values)) != len(values):
            raise ValueError("acceptance inputs/environment must be unique string arrays")
    for name in inputs:
        relative_path(name)
    if any(not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name) for name in environment):
        raise ValueError("invalid acceptance environment name")
    environment = {"PATH", *environment}
    for command in commands.values():
        for word in command.argv:
            environment.update(re.findall(r"\{env:([A-Za-z_][A-Za-z_0-9]*)\}", word))
    return config, commands, inputs, environment


def _task_decision_state(root, reviewed_files):
    """Validate all task decisions and require their bytes in the review snapshot."""
    store = TaskStore(root)
    directory = root / "recovery/tasks"
    if directory.resolve() != directory or not directory.is_dir():
        raise ValueError("full acceptance requires a canonical task directory")
    state = {}
    for path in sorted(directory.glob("*.json")):
        if path.resolve() != path or not path.is_file():
            raise ValueError("full acceptance requires regular task records")
        task = store.read(path.stem)
        journal = store.journal.stream_directory("task:" + task.task_id)
        if journal.resolve() != journal or journal.exists() and not journal.is_dir():
            raise ValueError("full acceptance requires canonical task journals")
        records = list(journal.glob("*.json"))
        if any(item.resolve() != item or not item.is_file() for item in records):
            raise ValueError("full acceptance requires regular task journal records")
        if any(reviewed_files.get(item.relative_to(root).as_posix()) is None for item in (path, *records)):
            raise ValueError("acceptance task decisions must participate in review snapshot")
        decisions = store.decisions(task.task_id)
        subjects, _ = store._scope(task, decisions)
        status = store.status(task.task_id, subjects)
        if store.read(task.task_id) != task or store.decisions(task.task_id) != decisions:
            raise ValueError("task decisions changed during full acceptance inventory")
        state[task.task_id] = {"task": task.to_dict(), "decisions": decisions,
                              "pending": status["pending_discoveries"]}
    return state


def measure_acceptance(root: Path, task_name: str, image: Path, request: dict, *, progress=None) -> dict:
    # Native capabilities expire on every exit, including failed proof commands.
    # The exact current build files survive for the driver's artifact recheck.
    with ExitStack() as contexts:
        return _measure_acceptance(root, task_name, image, request, contexts=contexts, progress=progress)


def _measure_acceptance(root, task_name, image, request, *, contexts, progress=None):
    root = root.resolve()
    image = (image if image.is_absolute() else root / image).resolve()
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    scope, _ = store._scope(task, decisions)
    report = {"scope": "full_project", "result": "unknown", "build": "unknown", "structure": "unknown",
              "references": "unknown", "obligations": "unknown", "runtime": "unknown",
              "request_fingerprint": digest(request), "measurements": [], "current": {}, "findings": []}
    # Validates the request/review and receipt schema before running commands.
    acceptance_result(3, report, request=request)
    if digest(request["task"]) != digest(task.to_dict()):
        raise ValueError("acceptance request does not name the current task")
    state = working_state(root)
    task_state = _task_decision_state(root, state["files"])
    pending = [name + ":" + finding["discovery_id"] for name, row in task_state.items()
               for finding in row["pending"]]
    if pending:
        raise ValueError("full acceptance has unresolved discoveries: " + ", ".join(pending))
    inventory = task_obligations(root, task_name)
    protected, live = {}, {}
    extend_inventory(protected, {"obligations": request["protected"]}, scope)
    extend_inventory(live, inventory, scope)
    if not live.keys() <= protected.keys():
        raise ValueError("acceptance request omits current historical obligations")
    interpretations = {}
    merge_interpretations(interpretations, request["interpretations"], scope)
    if digest(sorted(interpretations.values(), key=lambda row: row["interpretation_id"])) != digest(
            sorted(inventory["interpretations"], key=lambda row: row["interpretation_id"])):
        raise ValueError("acceptance interpretation inventory is stale")
    if pending_interpretations(request["review"], interpretations):
        raise ValueError("acceptance requires disposition of historical interpretations")
    config, commands, inputs, environment = _policy(root)
    # A matching request/review token alone cannot establish that the source
    # still has the contents independently reviewed. Check at this boundary,
    # including standalone CLI entry, before compiling or invoking proof code.
    if ReviewStore(root, task_name).snapshot(state, request["review_policy"]) != request["snapshot"]:
        raise ValueError("acceptance requires the current reviewed source snapshot")

    def proof_inputs():
        value = input_snapshot(root, inputs, {"policy": config, "environment": {
            name: hashlib.sha256(os.environ[name].encode()).hexdigest() if name in os.environ else None
            for name in environment}})
        if any(value is None for value in value["inputs"].values()):
            raise ValueError("declared acceptance input is missing")
        return value

    proof_before = proof_inputs()

    def source_fresh():
        current_state = working_state(root)
        if (current_state != state or proof_inputs() != proof_before
                or store.read(task_name) != task or store.decisions(task_name) != decisions
                or _task_decision_state(root, current_state["files"]) != task_state):
            raise ValueError("acceptance source, policy or task changed; preserve work and recheck")

    # The gate owns preparation BEFORE its measurement epoch. Do not capture
    # generated compiler recipes until that preparation has completed.
    application = None

    def acquire_application():
        nonlocal application
        if application is not None:
            raise ValueError("acceptance application build cannot be replaced")
        application = contexts.enter_context(application_graph(root, task_name, retain_current=True))
        require_bound_graph(application)
        if application.root != root or application.task_name != task_name:
            raise ValueError("acceptance application graph names a different project or task")
        return application

    source_fresh()
    gate = check_project(root, image, progress=progress, retain_build=True, **(
        {"_application_build": acquire_application} if Build.read(root).compile_link is not None else {}))
    source_fresh()
    built = next((row["report"] for row in gate["checks"] if row["name"] == "build"), {})
    report["build"] = built.get("result", "unknown")
    if gate["result"] != "pass":
        report["obligations"] = gate["result"]
        report["findings"].append({"dimension": "obligations", "message": "Native whole-project gate did not pass.",
                                   "evidence": [_diagnostic(row["name"], row["report"]) for row in gate["checks"]
                                                if row["result"] != "pass"]})
        return _finish(report, request)

    epoch = project_inputs(root, image)
    if digest(epoch) != gate["input_fingerprint"]:
        raise ValueError("acceptance inputs changed after the project gate")
    directory = relative_path(built.get("directory"))
    if application is not None:
        require_bound_graph(application)
        if (root / directory != application.current.directory
                or built.get("outputs") != application.current.report["outputs"]):
            raise ValueError("acceptance gate and linked preservation must use the same current build")
    artifacts = {directory + "/" + relative_path(row["path"]): row["sha256"] for row in built["outputs"]}
    if not artifacts:
        raise ValueError("acceptance requires fresh retained build artifacts")
    report["artifacts"] = dict(sorted(artifacts.items()))

    def fresh():
        source_fresh()
        if application is not None:
            require_bound_graph(application)
        if project_inputs(root, image) != epoch:
            raise ValueError("acceptance compiler/image inputs changed")
        for name, fingerprint in artifacts.items():
            path = root / name
            if any(part.is_symlink() for part in (path, *path.parents) if part.is_relative_to(root)):
                raise ValueError("acceptance artifact became a symlink")
            with path.open("rb") as stream:
                if hashlib.file_digest(stream, "sha256").hexdigest() != fingerprint:
                    raise ValueError("acceptance build artifact changed during verification")

    def run(name, action):
        fresh()
        if progress:
            progress({"check": name, "event": "start"})
        try:
            value = action()
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            value = {"result": "unknown", "reason": str(error)}
        fresh()
        if progress:
            progress({"check": name, "event": "finish", "result": value.get("result", "unknown")})
        return value

    restored = run("restoration", lambda: measure_restoration(
        root, task_name, image, review=request["review"], review_snapshot=request["snapshot"]))
    preservation = run("preservation", lambda: check_preservation(root, task_name, **(
        {"application": application} if application is not None else {})))
    linked = application.report if application is not None else {}
    linked_dimensions = [linked.get(name) for name in ("result", "linked_effects")]
    linked_preservation = ("fail" if "fail" in linked_dimensions else
                           "pass" if all(value == "pass" for value in linked_dimensions) else "unknown")
    # Negative native evidence remains a failure. A compiled-only positive
    # report, however, cannot supply the missing linked-build capability.
    if preservation.get("outside_scope_verification") == "fail":
        linked_preservation = "fail"
    if linked_preservation not in ("pass", "fail", "unknown"):
        linked_preservation = "unknown"
    if linked_preservation != "pass":
        report["findings"].append({
            "dimension": "obligations",
            "message": "Native linked outside-scope preservation is incomplete; compiled residual agreement and unchanged build policy "
                       "do not prove baseline/current linked selection and contributions. Structure/runtime proof commands cannot waive this prerequisite.",
            "evidence": [_diagnostic("linked-preservation", {
                "result": linked_preservation, "compiled_residual": preservation["result"],
                "application": linked,
                "next_action": "obtain fresh native linked-selection/residual evidence against the original preservation baseline; "
                               "keep the source and historical obligations until that verification is complete"})]})
    source_form = run("cpp-source-form", lambda: check_source_form(root))
    policy = run("baseline-build-policy", lambda: {"result": "pass" if unchanged_build_policy(
        root, task, Measurement.read(root), Build.read(root)) else "unknown"})
    native_structure = (declared_outcome(restored.get("structural_requirements", []), interpretations,
                                         review=request["review"])[0] if interpretations else "pass")
    native_structure = _all((native_structure, source_form["result"]))
    if source_form["result"] != "pass":
        report["findings"].append({"dimension": "structure", "message": "Native C++ source-form check did not pass.",
                                   "evidence": [_diagnostic("cpp-source-form", source_form)]})
    binary = restored.get("candidate", {}).get("checks", {}).get("binary", {}).get("result", "unknown")
    report["references"] = restored.get("references", "unknown")
    measurements = {row["obligation_id"]: row for row in restored.get("measurements", [])}
    current = dict(restored.get("current", {}))
    for key in protected.keys() - measurements.keys():
        current[key] = digest({"inputs": epoch, "obligation": key})
        measurements[key] = {"obligation_id": key, "fingerprint": current[key], "result": "unknown",
                             "evidence": "native measurement unavailable"}

    for dimension in ("structure", "runtime"):
        selected = {key for key, item in protected.items() if item.dimension == Dimension(dimension)}
        payload = {**request, "dimension": dimension,
                   "protected": [row for row in request["protected"] if row["obligation_id"] in selected],
                   "build": {"directory": directory, "outputs": built["outputs"]},
                   "image": {"path": str(image), "sha256": inventory["image_sha256"]}}

        def prove():
            if dimension not in commands:
                return {"result": "unknown", "reason": "no configured " + dimension + " proof command"}
            code, value = invoke(commands[dimension], root, task_name, payload)
            fields = {"request_fingerprint", "result", "evidence", "measurements", "current"}
            if not isinstance(value, dict) or set(value) != fields or value["request_fingerprint"] != digest(payload):
                raise ValueError("invalid or stale " + dimension + " proof receipt")
            if not isinstance(value["evidence"], list) or not value["evidence"]:
                raise ValueError("proof needs evidence")
            for item in value["evidence"]:
                require_text(item, "proof evidence")
            # Apply the same protected identity/freshness rules to this dimension
            # only. No command receives authority over native byte/layout checks.
            check_request = {key: payload[key] for key in request}
            proof = {**report, **{key: "pass" for key in ("build", "structure", "references", "runtime")},
                     "result": value["result"], "obligations": value["result"], "findings": [],
                     "request_fingerprint": digest(check_request),
                     "measurements": value["measurements"], "current": value["current"]}
            acceptance_result(code, proof, request=check_request)
            return value

        value = run("proof:" + dimension, prove)
        report[dimension] = _all((native_structure, value["result"])) if dimension == "structure" else value["result"]
        for row in value.get("measurements", []):
            key = row["obligation_id"]
            if measurements[key]["result"] == "unknown":
                measurements[key], current[key] = row, value["current"].get(key)
        if report[dimension] != "pass":
            report["findings"].append({"dimension": dimension, "message": value.get("reason", "Proof or native declaration check did not pass."),
                                       "evidence": value.get("evidence", ["native acceptance:" + dimension])})
    protected_results = [row["result"] if row["fingerprint"] == current.get(key) else "unknown"
                         for key, row in measurements.items() if key in protected]
    report["obligations"] = _all([binary, preservation["result"], linked_preservation, policy["result"], *protected_results])
    report["measurements"] = [measurements[key] for key in sorted(protected)]
    report["current"] = {key: current[key] for key in protected if current.get(key) is not None}
    if report["obligations"] != "pass":
        report["findings"].append({"dimension": "obligations", "message": "Strict matching, preservation or protected restoration is incomplete.",
                                   "evidence": [_diagnostic("strict task binary", restored.get("candidate", {}).get("checks", {}).get("binary", {})),
                                                _diagnostic("outside preservation", preservation),
                                                _diagnostic("baseline build policy", policy)]})
    fresh()
    return _finish(report, request)


def _finish(report, request):
    report["result"] = _all(report[key] for key in ("build", "structure", "references", "obligations", "runtime"))
    acceptance_result({"pass": 0, "fail": 1, "unknown": 3}[report["result"]], report, request=request)
    return report
