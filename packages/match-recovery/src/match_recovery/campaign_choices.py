"""Durable campaign scheduling choices, never a source or acceptance ledger.

Write intent before creating worktrees or starting integration. Resumption checks
actual Git/source state; recorded disposition is routing, not a passing verdict.
Coordinator authorization transitions apply to the remainder of the unchanged
bounded campaign, including later batches. Their saved batch is the grant's
origin, not a replacement assignment or a reusable acceptance report.
"""

from pathlib import Path
import re

from .dispatch import _configuration
from .model import digest, require_digest, require_text
from .records import publish_json
from .source_history import git
from .tasks import task_name


def revision(value):
    if not isinstance(value, str) or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", value):
        raise ValueError("campaign choice requires an immutable Git revision")
    return value


def ancestor(root, before, after="HEAD"):
    revision(before)
    return git(root, "merge-base", before, after).decode().strip() == before


def manifest(root, configuration, profiles, initial):
    return {"schema_version": 1, "root": str(root), "initial_revision": revision(initial),
            "configuration": configuration, "profiles_sha256": digest(profiles)}


def load_run(root, parent, path, configuration, profiles, coordinator_authorization=None):
    path = Path(path)
    path = path if path.is_absolute() else root / path
    if path.resolve() != path or path.parent != parent or not path.is_dir():
        raise ValueError("resume requires a canonical run directory in this coordinator's campaign storage")
    if coordinator_authority(root, path, configuration, profiles, coordinator_authorization) != digest(profiles):
        raise ValueError("resume configuration, profiles, destination or initial ancestry changed")
    return path


def coordinator_authority(root, directory, configuration, profiles, authorization=None, *, publish=False):
    """Validate an append-only delegation chain; publication requires the caller's lease.

    Preview is read-only. The returned digest is effective authority, not an
    assertion that the current profiles match it (the resume caller checks that).
    An exact retry of the last grant is harmless even after destination progress.
    """
    from .coordination import policy
    from .tasks import TaskStore

    value = _configuration(directory, "run.json")
    initial = revision(value.get("initial_revision"))
    effective = value.get("profiles_sha256")
    require_digest(effective)
    expected = manifest(root, configuration, profiles, initial)
    expected["profiles_sha256"] = effective
    if type(value.get("schema_version")) is not int or value != expected or not ancestor(root, initial):
        raise ValueError("resume configuration, destination or initial ancestry changed")
    entries = []
    for path in directory.glob("coordinator-authorization-*.json"):
        match = re.fullmatch(r"coordinator-authorization-([1-9][0-9]*)\.json", path.name)
        if not match:
            raise ValueError("invalid coordinator authorization filename")
        entries.append((int(match[1]), path.name))
    entries.sort()
    if [number for number, _ in entries] != list(range(1, len(entries) + 1)):
        raise ValueError("coordinator authorization chain must be contiguous")
    if not entries and authorization is None:
        return effective
    if "coordinator" not in configuration or not isinstance(profiles.get("coordinator"), dict):
        raise ValueError("coordinator authorization requires an existing coordinator profile")
    saved = batches(root, directory, configuration)
    if not saved:
        raise ValueError("coordinator authorization requires an existing assigned batch")
    by_digest = {digest(batch): batch for batch in saved}
    tasks = [TaskStore(root).read(name) for name in sorted({row["task"] for batch in saved for row in batch["plan"]["jobs"]})]
    fields = {"schema_version", "author", "run_sha256", "batch_sha256", "previous_profiles_sha256",
              "previous_coordinator", "coordinator", "destination_revision", "evidence"}
    mutable = {"subjects", "editable_files", "max_decisions", "evidence", "route_discoveries"}

    def validate(record, previous, *, fresh):
        if (not isinstance(record, dict) or set(record) != fields
                or type(record["schema_version"]) is not int or record["schema_version"] != 1):
            raise ValueError("invalid coordinator authorization record")
        for key in ("run_sha256", "batch_sha256", "previous_profiles_sha256"):
            require_digest(record[key])
        require_text(record["author"], "coordinator authorization author")
        evidence = record["evidence"]
        if (not isinstance(evidence, list) or not evidence
                or any(not isinstance(item, str) for item in evidence) or len(set(evidence)) != len(evidence)):
            raise ValueError("coordinator authorization requires unique evidence")
        for item in evidence:
            require_text(item, "coordinator authorization evidence")
        if (record["author"] != configuration["author"] or record["run_sha256"] != digest(value)
                or record["previous_profiles_sha256"] != previous or record["batch_sha256"] not in by_digest):
            raise ValueError("coordinator authorization does not bind this run, assignment or prior authority")
        before, after = record["previous_coordinator"], record["coordinator"]
        for task in tasks:
            policy(before, task)
            policy(after, task)
        if (before == after or {key: item for key, item in before.items() if key not in mutable}
                != {key: item for key, item in after.items() if key not in mutable}):
            raise ValueError("coordinator authorization may change only scope, budget, evidence and discovery-routing opt-in")
        if digest({**profiles, "coordinator": before}) != previous:
            raise ValueError("coordinator authorization cannot change other profiles or prior delegation")
        destination = revision(record["destination_revision"])
        bound = by_digest[record["batch_sha256"]]
        if not ancestor(root, bound["revision"], destination) or not ancestor(root, destination):
            raise ValueError("coordinator authorization destination is outside retained assignment ancestry")
        if fresh and (record["batch_sha256"] != digest(saved[-1])
                      or destination != git(root, "rev-parse", "HEAD").decode().strip()):
            raise ValueError("new coordinator authorization requires the latest batch and current destination")
        return digest({**profiles, "coordinator": after})

    last = None
    for _, name in entries:
        last = _configuration(directory, name)
        effective = validate(last, effective, fresh=False)
    if authorization is not None and authorization != last:
        effective = validate(authorization, effective, fresh=True)
        if effective != digest(profiles):
            raise ValueError("new coordinator authorization must name the configured coordinator")
        if publish:
            publish_json(directory / f"coordinator-authorization-{len(entries) + 1}.json", authorization)
    return effective


def worker_choice(directory, number, base, names, configuration):
    if type(number) is not int or number < 0 or not names or len(names) != len(set(names)):
        raise ValueError("invalid campaign batch selection")
    for name in names:
        task_name(name)
    jobs = [{"task": name, "root": str(directory / f"batch-{number}-worker-{index}"),
             "config": configuration["driver"], "worker": configuration["worker"], "reviewer": configuration["reviewer"],
             "checkpoint": configuration["checkpoint"], "iterations": configuration["iterations"],
             **({"migration": configuration["migration"]} if "migration" in configuration else {})}
            for index, name in enumerate(names)]
    plan = {"schema_version": 1, "author": configuration["author"], "evidence": configuration["evidence"],
            "max_parallel": configuration["max_parallel"], "resources": configuration["resources"],
            "timeout": configuration["timeout"], "jobs": jobs}
    return {"schema_version": 1, "index": number, "revision": revision(base), "plan": plan,
            "branches": [f"recovery/campaign-{directory.name}-{number}-{index}" for index in range(len(names))]}


def batches(root, directory, configuration):
    entries = []
    for path in directory.glob("batch-*-workers.json"):
        match = re.fullmatch(r"batch-(0|[1-9][0-9]*)-workers.json", path.name)
        if not match:
            raise ValueError("invalid campaign choice filename")
        entries.append((int(match[1]), path.name))
    result = []
    for number, name in sorted(entries):
        value = _configuration(directory, name)
        if (not isinstance(value.get("plan"), dict) or not isinstance(value["plan"].get("jobs"), list)
                or any(not isinstance(row, dict) or not isinstance(row.get("task"), str) for row in value["plan"]["jobs"])):
            raise ValueError("invalid saved campaign selection")
        expected = worker_choice(directory, number, value.get("revision"), [row["task"] for row in value["plan"]["jobs"]], configuration)
        if value != expected or len(value["plan"]["jobs"]) > configuration["max_parallel"] or not ancestor(root, value["revision"]):
            raise ValueError("saved campaign selection differs from current authorization or destination ancestry")
        result.append(value)
    return result


def latest_batch(root, directory, configuration):
    values = batches(root, directory, configuration)
    return values[-1] if values else None


def integration_choice(directory, number, name, context):
    task_name(name)
    return {"schema_version": 1, "task": name, "context": context,
            "worktree": str(directory / f"batch-{number}-candidate-{name}"),
            "branch": f"recovery/campaign-{directory.name}-{number}-candidate-{name}"}


def retire(directory, number, name, worker_revision, destination_revision, *, reason):
    if reason not in ("integrated", "no_checkpoint"):
        raise ValueError("invalid campaign retirement choice")
    value = {"schema_version": 1, "task": task_name(name), "worker_revision": revision(worker_revision),
             "destination_revision": revision(destination_revision), "reason": reason}
    publish_json(directory / f"batch-{number}-retired-{name}.json", value, allow_identical=True)


def retirement(root, directory, number, name):
    path = directory / f"batch-{number}-retired-{name}.json"
    if not path.exists() and not path.is_symlink():
        return None
    value = _configuration(directory, path.name)
    if (set(value) != {"schema_version", "task", "worker_revision", "destination_revision", "reason"}
            or type(value["schema_version"]) is not int or value["schema_version"] != 1 or value["task"] != name
            or value["reason"] not in ("integrated", "no_checkpoint")
            or not ancestor(root, revision(value["worker_revision"]), revision(value["destination_revision"]))
            or not ancestor(root, value["destination_revision"])):
        raise ValueError("retired attempt is not retained in current destination history; preserve work")
    return value
