"""Fresh scoped observations for discovery decisions, never acceptance inputs.

The caller already owns the worker/compiler leases. Run configured context and
checking only; do not prepare recipes, build, allocate a second prefix, or alter
the historical finding. Every decision obtains a fresh observation of its scope.
"""

from pathlib import Path

from .model import digest, require_digest
from .tasks import TaskStore


def _output(value, *, check=False):
    if (not isinstance(value, dict) or set(value) != {"exit_code", "report"}
            or type(value["exit_code"]) is not int or not isinstance(value["report"], dict)):
        raise ValueError("invalid coordination diagnostic output")
    if check:
        result = value["report"].get("result")
        if result not in ("pass", "fail", "unknown") or value["exit_code"] != {"pass": 0, "fail": 1, "unknown": 3}[result]:
            raise ValueError("coordination diagnostic checker result and exit code disagree")


def validate(root, task_name, value):
    from .investigation_round import epoch
    fields = {"schema_version", "task_id", "epoch", "configuration_sha256", "context", "check",
              "verification", "ready_for_collection"}
    if (not isinstance(value, dict) or set(value) != fields
            or type(value["schema_version"]) is not int or value["schema_version"] != 1
            or value["task_id"] != task_name or value["verification"] != "diagnostic_only"
            or value["ready_for_collection"] is not False):
        raise ValueError("invalid coordinator diagnostic observation")
    require_digest(value["configuration_sha256"])
    require_digest(value["epoch"])
    _output(value["context"])
    if value["context"]["exit_code"] == 0:
        _output(value["check"], check=True)
    elif value["check"] is not None:
        raise ValueError("coordination cannot check with unavailable context")
    if epoch(Path(root).resolve(), task_name) != value["epoch"]:
        raise ValueError("stale coordinator diagnostics; source, runtime or decisions changed")


def collect(root, task_name, configuration, execute):
    from .driver import Command
    from .investigation_round import epoch
    root = Path(root).resolve()
    task = TaskStore(root).read(task_name)
    if (not isinstance(configuration, dict) or type(configuration.get("schema_version")) is not int
            or configuration["schema_version"] != 1 or configuration.get("target") != task.target):
        raise ValueError("coordination diagnostics require the task's configured driver")
    context_command = Command.read(configuration.get("context"))
    check_command = Command.read(configuration.get("check"))
    before, configuration_key = epoch(root, task_name), digest(configuration)

    def fresh():
        if epoch(root, task_name) != before or digest(configuration) != configuration_key:
            raise ValueError("coordination diagnostic inputs changed during execution")

    def run(command, payload):
        fresh()
        try:
            code, report = execute(command, root, task_name, payload)
        finally:
            fresh()
        output = {"exit_code": code, "report": report}
        _output(output)
        return output

    context = run(context_command, {"task": task.to_dict()})
    check = (run(check_command, {"task": task.to_dict(), "context": context["report"]})
             if context["exit_code"] == 0 else None)
    observation = {"schema_version": 1, "task_id": task_name, "epoch": before,
                   "configuration_sha256": configuration_key, "context": context, "check": check,
                   "verification": "diagnostic_only", "ready_for_collection": False}
    validate(root, task_name, observation)
    return observation
