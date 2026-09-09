"""Real compilation/acceptance through the native multi-task coordinator.

Only agent reasoning is deterministic. This is not a live-agent/game pilot.
"""

import json
import os
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.tasks import TaskStore
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_native_acceptance_compiler import CLASSES, prepared_project


@pytest.mark.parametrize("authorize", [True, False])
def test_campaign_recovers_integrates_rechecks_and_selects_again(tmp_path, monkeypatch, authorize):
    provider = {role: {"command": {"argv": [sys.executable, "recovery/local_agents.py", role, "{request}"], "timeout": 30},
                       "instructions": [], "response": "stdout"} for role in ("planner", "worker", "reviewer", "coordinator")}
    provider.update(schema_version=1, artifacts="build/agents")
    root, image, task, git, evidence = prepared_project(tmp_path, monkeypatch, provider=provider, preparation=True,
                                                       proofs=False, authorize=authorize)
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    prefix = Path(os.environ.get("VC6_PREFIX", VC6_ROOT / ".wineprefix")).resolve()
    driver = root / "recovery/driver.json"
    driver.write_text(json.dumps({**json.loads(driver.read_text()), "acceptance_scope": "change"}))
    planner = {"schema_version": 1, "author": task.author,
               "command": {"argv": [sys.executable, "-m", "match_recovery", "agent", "planner"], "timeout": 30},
               "max_entities": 10, "max_editable_files": 4, "commit_decision": True,
               "evidence": ["Authorize planning of test-owned source groups"]}
    resources = {"schema_version": 1, "slots": [
        {"name": "compiler", "directory": str(prefix), "environment": {"VC6_PREFIX": "{resource}"}}]}
    checkpoint = {"author": task.author, "message": "Recover test-owned structure", "allow_migration": False}
    configuration = {"schema_version": 1, "author": task.author, "destination_branch": git("symbolic-ref", "HEAD"),
                     **{role: "recovery/" + role + ".json" for role in
                        ("planner", "driver", "worker", "reviewer", "coordinator", "checkpoint", "resources")},
                     "max_batches": 2, "max_parallel": 1, "iterations": 2, "timeout": 120,
                     "publication": {"enabled": True, "allow_migration": False, "cleanup": True,
                                     "message": "Integrate freshly verified test-owned recovery"},
                     "evidence": ["Authorize test-owned isolated recovery and fresh serial destination publication"]}
    for name, value in (("planner", planner), ("resources", resources), ("checkpoint", checkpoint), ("campaign", configuration)):
        (root / ("recovery/" + name + ".json")).write_text(json.dumps(value))
    git("add", "recovery")
    git("commit", "-m", "Authorize bounded native campaign")
    before = git("rev-parse", "HEAD")
    result = subprocess.run([sys.executable, "-I", "-m", "match_recovery", "campaign", "--root", str(root),
                             "--config", "recovery/campaign.json"], cwd=tmp_path, capture_output=True, timeout=180)
    assert result.returncode == 0, result.stdout.decode() + result.stderr.decode()
    report = json.loads(result.stdout)
    assert report["stop_reason"] == "planning_defer" and not report["full_project_accepted"]
    assert len(report["batches"]) == 2
    first, second = report["batches"]
    assert first["dispatch"]["jobs"][0]["scope_intact"]
    assert first["coordination"][0]["stop_reason"] == "change_accepted"
    assert first["integrations"][0]["destination_verified"]
    assert first["integrations"][0]["stop_reason"] == "destination_accepted"
    assert not first["pending"] and second["selections"][0]["action"] == "defer"
    assert git("rev-parse", "HEAD") == report["final_commit"] != before
    git("merge-base", "--is-ancestor", before, "HEAD")
    assert (root / "src/owner.h").read_text() == evidence + CLASSES
    assert "- input" not in (root / "src/owner.cpp").read_text()
    task_name = first["workers"][0]["task_id"]
    assert first["selections"][0]["action"] == ("resume" if authorize else "create")
    assert TaskStore(root).read(task_name).base_revision == (task.base_revision if authorize else before)
    assert not git("status", "--porcelain")
    assert git("worktree", "list", "--porcelain").count("worktree ") == 1
    assert not git("branch", "--list", "recovery/campaign-*")
