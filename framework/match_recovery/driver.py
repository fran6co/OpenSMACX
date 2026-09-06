"""Bounded recovery execution through configured, trusted command adapters.

Commands exchange JSON over stdin/stdout, never shell fragments. Workers must
run in an externally sandboxed worktree: scope checks detect violations, they
are not a security sandbox. This driver never commits, collects or discards work.
"""

from dataclasses import dataclass
import hashlib
import os
from pathlib import Path
import re
import signal
import subprocess
import uuid

from .locking import file_lock
from .model import canonical, require_text
from .records import parse_json
from .tasks import TaskStore, entity_id


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
    process = subprocess.Popen(argv, cwd=root, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, start_new_session=os.name != "nt")
    try:
        output, errors = process.communicate(canonical(payload), timeout=command.timeout)
    except BaseException:
        # Reap this command's process group, never a shared Wine prefix or an
        # unrelated agent. Windows process-tree isolation belongs to its adapter.
        if os.name != "nt":
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
        else:
            process.kill()
        try:
            process.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            # A child may have detached from the process group while retaining
            # a pipe. Do not let teardown defeat the command's time bound.
            for stream in (process.stdin, process.stdout, process.stderr):
                if stream is not None:
                    stream.close()
            process.wait(timeout=5)
        raise
    if process.returncode not in (0, 1, 3):
        raise ValueError(f"adapter exited {process.returncode}: {errors.decode(errors='replace')[-2000:]}")
    return process.returncode, parse_json(output)


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
          iterations: int, execute=invoke) -> dict:
    """Recompute context/checks, edit, recheck, repeat; discoveries stop for review."""
    if type(iterations) is not int or not 1 <= iterations <= 100:
        raise ValueError("iterations must be 1..100")
    if (set(configuration) != {"schema_version", "target", "context", "check"}
            or type(configuration["schema_version"]) is not int or configuration["schema_version"] != 1):
        raise ValueError("invalid driver configuration")
    if set(worker_configuration) != {"author", "command"}:
        raise ValueError("worker configuration needs author and command")
    require_text(worker_configuration["author"], "worker author")
    context_command = Command.read(configuration["context"])
    check_command = Command.read(configuration["check"])
    worker_command = Command.read(worker_configuration["command"])
    root = root.resolve()
    store = TaskStore(root)
    with file_lock(root / "build/recovery/driver.lock", timeout=0):
        task = store.read(task_name)
        if task.target != configuration["target"]:
            raise ValueError("task target disagrees with configured target adapter")
        selected, editable = store.scope(task_name)
        initial_decisions = store.decisions(task_name)
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
                    "ready_for_collection": False, "persisted_results": False}
        def readonly(command, payload):
            before = working_state(root)
            try:
                code, output = execute(command, root, task_name, payload)
            finally:
                if changes(before, working_state(root)):
                    raise ValueError("read-only adapter edited the worktree; preserve work and inspect manually")
            return code, output
        for round_index in range(iterations + 1):
            state = store.status(task_name, selected)
            if state["pending_discoveries"]:
                return finish(state["next_action"], round_index)
            code, context = readonly(context_command, {"task": task.to_dict()})
            if code != 0:
                return finish("context_unavailable", round_index)
            code, check = readonly(check_command, {"task": task.to_dict(), "context": context})
            result = check.get("result")
            if result not in ("pass", "fail", "unknown") or code != {"pass": 0, "fail": 1, "unknown": 3}[result]:
                raise ValueError("checker result and exit code disagree")
            if result == "pass":
                return finish("independent_review_required", round_index, check)
            if round_index == iterations:
                return finish("iteration_budget_exhausted", round_index, check)
            before = working_state(root)
            try:
                code, response = execute(worker_command, root, task_name,
                                         {"task": task.to_dict(), "editable_files": sorted(editable),
                                          "context": context, "check": check, "round": round_index + 1,
                                          "instructions": ["Edit only authorized files; never run Git mutations.",
                                                           "Report structural discoveries even outside task scope.",
                                                           "Do not erase source mappings or restoration obligations."]})
            finally:
                changed = changes(before, working_state(root))
                if changed - editable:
                    raise ValueError("worker exceeded editing scope; preserve work and inspect manually")
            if store.read(task_name) != task or store.decisions(task_name) != initial_decisions:
                raise ValueError("task decisions changed during dispatch; inspect before resuming")
            if code != 0:
                return finish("worker_interrupted", round_index + 1, check, current=False)
            if set(response) != {"discoveries"} or not isinstance(response["discoveries"], list):
                raise ValueError("worker must return a discoveries array, not a verdict")
            for finding in response["discoveries"]:
                if not isinstance(finding, dict) or set(finding) != {"hypothesis", "affects", "evidence"}:
                    raise ValueError("invalid worker discovery")
                require_text(finding["hypothesis"], "hypothesis")
                for key in ("affects", "evidence"):
                    if not isinstance(finding[key], list) or not finding[key]:
                        raise ValueError("discovery requires affected entities and evidence")
                for affected in finding["affects"]:
                    entity_id(affected)
                for evidence in finding["evidence"]:
                    require_text(evidence, "discovery evidence")
            for finding in response["discoveries"]:
                store.discover(task_name, request_id="worker-" + uuid.uuid4().hex,
                               author=worker_configuration["author"], expected_revision=len(store.decisions(task_name)),
                               **finding)
            if response["discoveries"]:
                return finish(store.status(task_name, selected)["next_action"], round_index + 1, check, current=False)
            if not changed:
                return finish("no_source_progress", round_index + 1, check)
        raise AssertionError("bounded loop did not terminate")
