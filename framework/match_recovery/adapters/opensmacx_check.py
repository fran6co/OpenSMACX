"""Joint scoped evidence for structure-led recovery, never full acceptance."""

import hashlib
from pathlib import Path

from ..fingerprint import input_snapshot
from ..model import Result, digest
from ..tasks import TaskStore
from .opensmacx_binary import _inputs, verify_task
from .opensmacx_layout import verify_layout
from .vc6 import FLAG_SETS


def check_task(root: Path, task_name: str, image: Path, *,
               binary_checker=verify_task, layout_checker=verify_layout) -> dict:
    root = root.resolve()
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    if task.target != "opensmacx":
        raise ValueError("combined checker requires an OpenSMACX task")

    def snapshot():
        with image.open("rb") as stream:
            image_sha = hashlib.file_digest(stream, "sha256").hexdigest()
        return input_snapshot(root, _inputs(root),
                              {"task": store.read(task_name), "decisions": store.decisions(task_name),
                               "image_sha256": image_sha, "flags": FLAG_SETS, "layout_debug": "/Z7"})

    before = snapshot()
    checks = {}
    # A failed byte match must not prevent discovery of a structural regression,
    # and a passing body must not hide unknown or regressed layout evidence.
    for name, checker, args in (("binary", binary_checker, (root, task_name, image)),
                                ("layout", layout_checker, (root, task_name))):
        try:
            report = checker(*args)
            if not isinstance(report, dict) or report.get("task_id") != task_name:
                raise ValueError("checker returned a report for another task")
            Result(report.get("result"))
            checks[name] = report
        except (OSError, ValueError) as error:
            checks[name] = {"task_id": task_name, "result": Result.UNKNOWN, "error": str(error)}
    if before != snapshot() or task != store.read(task_name) or decisions != store.decisions(task_name):
        raise ValueError("source, image, build inputs or task changed between scoped checks; retry at a checkpoint")
    results = [report["result"] for report in checks.values()]
    result = (Result.UNKNOWN if Result.UNKNOWN in results else
              Result.FAIL if Result.FAIL in results else Result.PASS)
    return {"task_id": task_name, "check": "scoped_binary_and_layout", "result": result,
            "checks": checks, "input_fingerprint": digest(before),
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "build_verification": Result.UNKNOWN, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN,
            "limitations": ["joint scoped diagnostics only; neither byte matching nor declared offsets prove full structure",
                            "no full-project build, outside-scope obligations, runtime or independent review acceptance",
                            "fresh sequential measurements; diagnostic fingerprint is not a reusable cache key"]}
