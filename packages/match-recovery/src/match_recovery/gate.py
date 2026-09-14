"""Package-owned project gate, independent of project CLI implementations.

Every native check participates; a project cannot turn a failing native check
off by omitting it from a command list. Additional project tests use explicit
bounded commands. Passing this gate is not full structural/runtime acceptance.
"""

import hashlib
import os
from pathlib import Path
import re
import subprocess
from time import monotonic

from .build_check import Build, check_build
from .build_graph import require_bound_graph
from .claim_integrity import check_claims
from .components import relative_path
from .data_check import check_data
from .driver import Command, working_state
from .fingerprint import input_snapshot
from .layout_check import check_layout
from .mapping_check import check_mappings
from .match_check import check_matches
from .measurement import Measurement
from .model import digest, require_text
from .processes import run_bounded
from .preparation import prepare_project
from .records import read_json
from .reference_check import check_references, reference_snapshot
from .source_audits import audit_source, _validate_policy
from .source_integrity import check_source
from .source_inventory import image_pin
from .symbol_check import check_symbols


def _configuration(root, source):
    project = read_json(root / "recovery/project.json")
    path = source.path(root, project.get("gate"))
    config = read_json(path)
    if (set(config) != {"schema_version", "checks", "inputs", "input_roots", "environment"}
            or type(config["schema_version"]) is not int or config["schema_version"] != 1
            or not isinstance(config["checks"], dict)):
        raise ValueError("invalid project gate configuration")
    for name in ("inputs", "input_roots", "environment"):
        if not isinstance(config[name], list) or any(not isinstance(item, str) for item in config[name]):
            raise ValueError(f"gate {name} must be a string array")
        if len(set(config[name])) != len(config[name]):
            raise ValueError(f"duplicate gate {name}")
    for name in (*config["inputs"], *config["input_roots"]):
        relative_path(name)
    if any(not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name) for name in config["environment"]):
        raise ValueError("invalid gate environment name")
    commands = {}
    environment = {"PATH", *config["environment"]}
    for name, value in config["checks"].items():
        require_text(name, "project check name")
        commands[name] = Command.read(value)
        for word in commands[name].argv:
            environment.update(re.findall(r"\{env:([A-Za-z_][A-Za-z_0-9]*)\}", word))
    # All configured source-policy ratchets run; no second list can drift or
    # leave a newly added audit outside the project gate.
    policy_path = source.path(root, project.get("audits"))
    policy = read_json(policy_path)
    _validate_policy(policy)
    return path, config, commands, environment, policy_path, sorted(policy["audits"])


def _command(root, image, command):
    def expand(match):
        name = match[1]
        if name == "root":
            return str(root)
        if name == "image":
            return str(image)
        if name.startswith("env:") and name[4:] in os.environ:
            return os.environ[name[4:]]
        raise ValueError(f"missing gate command input: {name}")
    argv = [re.sub(r"\{(root|image|env:[A-Za-z_][A-Za-z_0-9]*)\}", expand, word) for word in command.argv]
    try:
        outcome = run_bounded(argv, cwd=root, timeout=command.timeout)
    except (OSError, subprocess.TimeoutExpired) as error:
        return {"result": "unknown", "reason": str(error), "command": list(command.argv)}
    return {"result": {0: "pass", 1: "fail", 3: "unknown"}.get(outcome.returncode, "unknown"),
            "exit_code": outcome.returncode, "command": list(command.argv),
            "stdout": outcome.stdout.decode(errors="replace")[-8000:],
            "stderr": outcome.stderr.decode(errors="replace")[-8000:],
            "verification": "configured_command_exit_status"}


def project_inputs(root: Path, image: Path) -> dict:
    """Recompute the gate's compiler, image, policy and declared input identity."""
    measurement, build = Measurement.read(root), Build.read(root)
    path, config, _, environment, policy_path, _ = _configuration(root, measurement.source)
    preparation = (Build.read(root, key="preparation")
                   if "preparation" in read_json(root / "recovery/project.json") else None)
    with image.open("rb") as stream:
        actual = hashlib.file_digest(stream, "sha256").hexdigest()
    if actual != image_pin(root, measurement.source):
        raise ValueError("project gate requires the configured pinned original image")
    files = {path.relative_to(root).as_posix(), policy_path.relative_to(root).as_posix(), *config["inputs"]}
    for name in config["input_roots"]:
        directory = measurement.source.path(root, name)
        if not directory.is_dir():
            raise ValueError(f"missing gate input root: {name}")
        for item in (directory, *directory.rglob("*")):
            if item.is_symlink():
                raise ValueError("gate input roots cannot contain symlinks")
            if item.is_file():
                files.add(item.relative_to(root).as_posix())
    # Configured prose checks include target existence, not only document bytes.
    # A deleted referenced script must invalidate acceptance even if no doc changed.
    references = (digest(reference_snapshot(root))
                  if "references" in read_json(root / "recovery/project.json") else None)
    result = input_snapshot(root, files, {
        "references": references,
        "measurement": measurement.snapshot(root, {}), "build": build.snapshot(root),
        "preparation": preparation.snapshot(root) if preparation is not None else None,
        "image": {"path": str(image), "sha256": actual},
        "environment": {name: hashlib.sha256(os.environ[name].encode()).hexdigest() if name in os.environ else None
                        for name in environment}})
    if any(value is None for value in result["inputs"].values()):
        raise ValueError("declared project gate input is missing")
    return result


def _application_build_report(root, application):
    """Use an original live graph's current build, never a serialized receipt.

    Only the display directory is rebased to the checkout. Native command and
    compilation evidence retain their original private-root coordinate system.
    """
    require_bound_graph(application)
    if application.root != root:
        raise ValueError("gate application build belongs to a different project")
    report = application.current.report
    report["directory"] = application.current.directory.relative_to(root).as_posix()
    relative_path(report["directory"])
    require_bound_graph(application)
    return report


def check_project(root: Path, image: Path, *, jobs: int = 1, progress=None, retain_build=False,
                  _application_build=None) -> dict:
    root = root.resolve()
    if type(jobs) is not int or jobs < 1:
        raise ValueError("gate jobs must be a positive integer")
    if _application_build is not None and (not callable(_application_build) or retain_build is not True):
        raise ValueError("native application build hook requires retained build verification")
    image = (image if image.is_absolute() else root / image).resolve()
    measurement = Measurement.read(root)
    build = Build.read(root)
    path, config, commands, environment, policy_path, audits = _configuration(root, measurement.source)
    preparation = (Build.read(root, key="preparation")
                   if "preparation" in read_json(root / "recovery/project.json") else None)
    reports = []
    if preparation is not None:
        # Preparation is an explicit write phase BEFORE the measurement epoch;
        # no generator may change a database underneath already-started checks.
        with image.open("rb") as stream:
            actual = hashlib.file_digest(stream, "sha256").hexdigest()
        if actual != image_pin(root, measurement.source):
            raise ValueError("project gate requires the configured pinned original image")
        if progress:
            progress({"check": "preparation", "event": "start"})
        started = monotonic()
        try:
            report = prepare_project(root)
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            report = {"result": "unknown", "reason": str(error)}
        reports.append({"name": "preparation", "result": report["result"],
                        "elapsed_seconds": monotonic() - started, "report": report})
        if progress:
            progress({"check": "preparation", "event": "finish", "result": report["result"]})
        if report["result"] != "pass":
            return _result(reports, report.get("input_fingerprint"))
    state = working_state(root)

    def snapshot():
        return project_inputs(root, image)

    initial = snapshot()
    application = None

    def unchanged():
        if snapshot() != initial or working_state(root) != state:
            raise ValueError("project gate inputs or Git state changed; preserve edits and recheck")
        if application is not None:
            require_bound_graph(application)

    def build_current():
        nonlocal application
        if _application_build is None:
            return check_build(root, **({"retain": True} if retain_build else {}))
        # Acquisition is after gate-owned preparation and prior pure checks.
        # The caller holds its context open through every later acceptance check.
        candidate = _application_build()
        report = _application_build_report(root, candidate)
        application = candidate
        return report

    def run(name, operation):
        unchanged()
        if progress:
            progress({"check": name, "event": "start"})
        started = monotonic()
        try:
            report = operation()
            if not isinstance(report, dict) or report.get("result") not in ("pass", "fail", "unknown"):
                raise ValueError("checker returned no supported result")
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            report = {"result": "unknown", "reason": str(error)}
        reports.append({"name": name, "result": report["result"], "elapsed_seconds": monotonic() - started,
                        "report": report})
        unchanged()
        if progress:
            progress({"check": name, "event": "finish", "result": report["result"]})
        return report["result"]

    try:
        run("source", lambda: check_source(root, jobs=jobs))
        if "references" in read_json(root / "recovery/project.json"):
            run("live-references", lambda: check_references(root))
        run("mappings", lambda: check_mappings(root))
        run("claim-identities", lambda: check_claims(root))
        for name in audits:
            run("audit:" + name, lambda name=name: audit_source(root, name))
        for name, command in commands.items():
            run("project:" + name, lambda command=command: _command(root, image, command))
        built = run("build", build_current)
        # One compiler environment by default. Pure checks above finish before
        # the build, and compiled sweeps are serial; only the explicit match
        # sweep jobs parameter can use caller-provisioned compiler concurrency.
        for name, operation in (
            ("matches", lambda: check_matches(root, image, jobs=jobs)),
            ("symbols", lambda: check_symbols(root)),
            ("initial-data", lambda: check_data(root, image)),
            ("layout", lambda: check_layout(root, **({"progress": lambda event: progress({"check": "layout", **event})}
                                                    if progress else {}))),
        ):
            run(name, operation if built == "pass" else lambda: {
                "result": "unknown", "reason": "fresh project build did not pass; compiler sweep not run"})
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        reports.append({"name": "freshness", "result": "unknown", "report": {"reason": str(error)}})
    return _result(reports, digest(initial))


def _result(reports, fingerprint):
    results = {row["result"] for row in reports}
    result = "fail" if "fail" in results else "unknown" if "unknown" in results else "pass"
    return {"schema_version": 1, "check": "project_gate", "result": result,
            "exit_code": {"pass": 0, "fail": 1, "unknown": 3}[result], "checks": reports,
            "input_fingerprint": fingerprint, "persisted": False, "ready_for_collection": False,
            "limitations": ["all configured checks, not full structural, lifetime, dependency ABI or runtime acceptance",
                            "historical byte claims use relocation masking; strict reference checks remain separate",
                            "additional project commands are trusted exit-status checks, not inferred proof dimensions",
                            "unsupported or missing evidence remains unknown; source edits are never reverted"]}


def summary(report):
    """Keep every stage and failure identity, dropping only bulky observations."""
    import copy
    report = copy.deepcopy(report)
    for stage in report["checks"]:
        value = stage["report"]
        if stage["name"] == "matches" and "measurements" in value:
            value["measured"] = len(value.pop("measurements"))
        elif stage["name"] == "live-references" and "history" in value:
            value["historical_mentions"] = len(value.pop("history"))
        elif stage["name"] == "symbols":
            value["functions"] = [row for row in value.get("functions", []) if row["result"] != "pass"]
            value.pop("trials", None)
        elif stage["name"] == "layout":
            for key in ("expectations", "observations"):
                value[key] = [row for row in value.get(key, []) if row["result"] != "pass"]
            value.pop("coverage", None)
        elif stage["name"].startswith("audit:"):
            for key in ("per_file", "explanations"):
                value.pop(key, None)
    return report
