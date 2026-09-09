"""Durable campaign scheduling choices, never a source or acceptance ledger.

Write intent before creating worktrees or starting integration. Resumption checks
actual Git/source state; recorded disposition is routing, not a passing verdict.
"""

from pathlib import Path
import re

from .dispatch import _configuration
from .model import digest
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


def load_run(root, parent, path, configuration, profiles):
    path = Path(path)
    path = path if path.is_absolute() else root / path
    if path.resolve() != path or path.parent != parent or not path.is_dir():
        raise ValueError("resume requires a canonical run directory in this coordinator's campaign storage")
    value = _configuration(path, "run.json")
    initial = revision(value.get("initial_revision"))
    if value != manifest(root, configuration, profiles, initial) or not ancestor(root, initial):
        raise ValueError("resume configuration, profiles, destination or initial ancestry changed")
    return path


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
