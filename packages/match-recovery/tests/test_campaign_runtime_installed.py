"""A real installed CLI resumes preserved campaign work through native dispatch."""

import json
from pathlib import Path
import subprocess

from match_recovery.campaign import _policy
from match_recovery.driver import working_state
from match_recovery.model import digest
from match_recovery.records import read_json
from match_recovery.resources import ACTIVE
from match_recovery.tasks import TaskStore
from test_campaign import configured_campaign, compiler_project, project
from test_campaign_resume import stopped
from test_framework_runtime import install_copy, write_binding
from test_task_source import git


def test_installed_runtime_resumes_real_native_dispatch_without_accepting_unknown_work(configured_campaign):
    root, configuration = configured_campaign
    configuration["max_batches"] = 1
    root, configuration, directory, child = stopped((root, configuration))
    source = child / "code/model.h"
    source.write_text(source.read_text() + "// Retained unfinished structural candidate\n")
    # Only the fixture's standard-library recipe generator runs; it neither
    # invokes a compiler nor uses a provider. Unknown checker output is deliberate.
    installed = install_copy(root / "build/installed-framework")
    binding = write_binding(root / "build", installed)
    token = {"schema_version": 1, "author": configuration["author"],
             "run_sha256": digest(read_json(directory / "run.json")),
             "batch_sha256": digest(read_json(directory / "batch-0-workers.json")),
             "profiles_sha256": digest(_policy(root, configuration)),
             "destination_revision": git(root, "rev-parse", "HEAD"),
             "runtime_binding": str(binding), "runtime_sha256": digest(read_json(binding)),
             "evidence": ["Select the real installed framework for this test-owned retained campaign"]}
    permission = root / "build/runtime-permission.json"
    permission.write_text(json.dumps(token))
    settings = root / "build/campaign-settings.json"
    settings.write_text(json.dumps(configuration))
    manifests = {name: (directory / name).read_bytes() for name in ("run.json", "batch-0-workers.json")}
    before = working_state(root), working_state(child)
    original_task = TaskStore(child).read("owner")
    argv = [str(installed[0]), "-I", "-B", "-m", "match_recovery", "--runtime", str(binding),
            "campaign", "--root", str(root), "--config", str(settings), "--resume", str(directory),
            "--runtime-authorization", str(permission)]
    result = subprocess.run(argv, cwd=root, capture_output=True, text=True, timeout=60)
    assert result.returncode == 3, result.stderr
    report = json.loads(result.stdout)
    assert not report["full_project_accepted"] and not report["ready_for_collection"]
    assert report["stop_reason"] == "replanning_required", report
    batch, = report["batches"]
    assert batch["workers"][0]["compiler_prepared"]
    assert batch["workers"][0]["resumed"] and not batch["workers"][0]["worker_created"]
    dispatched = batch["dispatch"]
    assert dispatched["verification"] == "not_measured_by_dispatch"
    assert not dispatched["batch_completed"] and not dispatched["ready_for_collection"]
    job, = dispatched["jobs"]
    assert job["task"] == "owner" and job["root"] == str(child)
    assert job["returncode"] == 3 and job["scope_intact"] and not job["timed_out"]
    # This message is emitted only after the installed native driver invokes
    # the configured obligation hook, whose deliberately empty object is not
    # evidence. Runtime activation or import refusal cannot pass this control.
    diagnostic = Path(job["stderr"]).read_text()
    assert "invalid restoration inventory" in diagnostic, diagnostic
    events = [json.loads(line) for line in result.stderr.splitlines()]
    assert any(event["stage"] == "worker" and event.get("event") == "started" for event in events)
    assert read_json(directory / "runtime-authorization.json") == token
    assert all((directory / name).read_bytes() == contents for name, contents in manifests.items())
    assert (working_state(root), working_state(child)) == before
    assert TaskStore(child).read("owner") == original_task
    assert "Retained unfinished structural candidate" in source.read_text()
    resources = read_json(root / configuration["resources"])
    assert all(not (Path(slot["directory"]) / ACTIVE).exists() for slot in resources["slots"])
