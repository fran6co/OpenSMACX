"""Native worker setup through VC6-backed recovery; reasoning is deterministic.

This test-owned application is not an Alpha Centauri or live-provider pilot.
The original executable, compiler, linking, restoration and acceptance are real.
"""

import json
import os
from pathlib import Path
import subprocess
import sys

from match_recovery.driver import working_state
from match_recovery.tasks import TaskStore
from match_recovery.worker_worktree import prepare_worker
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_native_acceptance_compiler import CLASSES, prepared_project


def test_prepared_worker_recovers_structure_and_reaches_fresh_change_acceptance(tmp_path, monkeypatch):
    provider = {role: {"command": {"argv": [sys.executable, "recovery/local_agents.py", role, "{request}"], "timeout": 30},
                       "instructions": [], "response": "stdout"} for role in ("worker", "reviewer", "coordinator")}
    provider.update(schema_version=1, artifacts="build/agents")
    root, image, task, git, evidence = prepared_project(tmp_path, monkeypatch, provider=provider, preparation=True)
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    prefix = Path(os.environ.get("VC6_PREFIX", VC6_ROOT / ".wineprefix")).resolve()
    resources = {"schema_version": 1, "slots": [
        {"name": "compiler", "directory": str(prefix), "environment": {"VC6_PREFIX": "{resource}"}}
    ]}
    resource_path = root / "build/resources.json"
    resource_path.write_text(json.dumps(resources))
    before = working_state(root)
    permission = {"author": task.author, "revision": git("rev-parse", "HEAD"), "prepare": True,
                  "evidence": ["Authorize test-owned native worker setup and fresh compiler recipes"]}
    setup = prepare_worker(root, task.task_id, permission, worktree="build/worker", branch="worker", resources=resources)
    child = Path(setup["worktree"])
    assert setup["compiler_prepared"] and not setup["ready_for_collection"]
    rows = json.loads((child / "build/compile_commands.json").read_text())
    assert all(Path(row["file"]).is_relative_to(child) and Path(row["directory"]).is_relative_to(child) for row in rows)
    assert TaskStore(child).read(task.task_id).base_revision == task.base_revision
    result = subprocess.run([sys.executable, "-I", "-m", "match_recovery", "run", task.task_id,
                             "--root", str(child), "--acceptance-scope", "change", "--iterations", "2",
                             "--reviewer", "recovery/reviewer.json", "--coordinator", "recovery/coordinator.json",
                             "--resources", str(resource_path)], cwd=tmp_path, capture_output=True, timeout=180)
    assert result.returncode == 0, result.stdout.decode() + result.stderr.decode()
    report = json.loads(result.stdout)
    assert report["stop_reason"] == "change_accepted" and report["ready_for_checkpoint"]
    assert not report["ready_for_collection"] and report["worker_rounds"] == 2
    assert (child / "src/owner.h").read_text() == evidence + CLASSES
    assert TaskStore(child).read(task.task_id).base_revision == task.base_revision
    assert working_state(root) == before
