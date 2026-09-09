"""Real native child drivers in synthetic worktrees; not game acceptance."""

from copy import deepcopy
from dataclasses import replace
import json
import os
from pathlib import Path
import sys

import pytest

from match_recovery.cli import main
from match_recovery.dispatch import dispatch
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.model import Origin
from match_recovery.resources import ACTIVE, INHERITED, LOCK
from match_recovery.tasks import Task, TaskStore
from match_recovery.worker_worktree import prepare_worker
from test_source_configuration import configured
from test_task_source import git, project


def write(path, value):
    path.write_text(json.dumps(value))


def command(code):
    return {"argv": [sys.executable, "-c", code], "timeout": 10}


@pytest.fixture
def batch(project):
    root = project
    (root / ".gitignore").write_text("build/\n")
    (root / "code/other.cc").write_text(
        '// ORIGINAL: 0x00000020 _other 0x00000020-0x00000028\nvoid other() {}\n')
    config = json.loads((root / "recovery/project.json").read_text())
    config["compiler"] = {"environment": ["TEST_COMPILER_SLOT"]}
    write(root / "recovery/project.json", config)
    signals = root / "build/signals"
    signals.mkdir(parents=True)
    # Both drivers must be running with their own compiler leases before either
    # proceeds. These ignored signals are only process-test synchronization.
    context = ("import os,json,time; from pathlib import Path; "
               f"signals=Path({str(signals)!r}); "
               "(signals / '{task}').write_text(os.environ['TEST_COMPILER_SLOT']);\n"
               "while len(list(signals.iterdir())) < 2: time.sleep(.01)\n"
               "print('{}')")
    driver = {"schema_version": 1, "target": "custom", "context": command(context),
              "check": command("import json,sys; print(json.dumps({'result':'fail'})); sys.exit(1)")}
    write(root / "recovery/driver.json", driver)
    write(root / "recovery/worker.json", {"author": "worker", "command": command("print('{\"discoveries\": []}')")})
    write(root / "recovery/reviewer.json", {"author": "reviewer", "command": command("raise AssertionError('must not review failed checks')")})
    slots = []
    for index in range(2):
        directory = root / f"build/slot-{index}"
        directory.mkdir()
        slots.append({"name": f"slot-{index}", "directory": str(directory),
                      "environment": {"TEST_COMPILER_SLOT": "{resource}"}})
    write(root / "recovery/resources.json", {"schema_version": 1, "slots": slots})
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic driver environment")
    base = git(root, "rev-parse", "HEAD")
    for name, address in (("owner", 16), ("other", 32)):
        TaskStore(root).create(Task(name, "custom", "coordinator", "Recover source ownership", base,
                                   (Origin("a" * 64, "custom:offset", address),), (f"code/{name}.cc",),
                                   ("synthetic source evidence",)))
    git(root, "add", ".")
    git(root, "commit", "-m", "Authorize two disjoint tasks")
    jobs = []
    for name in ("owner", "other"):
        child = root / f"build/worktrees/{name}"
        prepared = prepare_worker(root, name, {"author": "coordinator", "revision": git(root, "rev-parse", "HEAD"),
                                               "evidence": ["Authorize native worker setup for the batch control"], "prepare": False},
                                  worktree=child, branch=name)
        assert prepared["worker_created"] and not prepared["ready_for_collection"]
        jobs.append({"task": name, "root": str(child), "config": "recovery/driver.json",
                     "worker": "recovery/worker.json", "reviewer": "recovery/reviewer.json", "iterations": 1})
    return root, {"schema_version": 1, "author": "coordinator", "evidence": ["Authorize bounded parallel source recovery"],
                  "max_parallel": 2, "resources": "recovery/resources.json", "timeout": 20, "jobs": jobs}


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_native_parallel_drivers_lease_distinct_slots_and_do_not_accept_failed_checks(batch):
    root, plan = batch
    before = working_state(root)
    events = []
    report = dispatch(root, plan, progress=events.append)
    assert [event["event"] for event in events[:2]] == ["started", "started"]
    assert working_state(root) == before
    assert not report["batch_completed"] and not report["ready_for_collection"]
    assert not report["coordinator_changed"]
    assert report["verification"] == "not_measured_by_dispatch"
    assert len({(root / f"build/signals/{name}").read_text() for name in ("owner", "other")}) == 2
    for row in report["jobs"]:
        assert row["scope_intact"] and not row["timed_out"] and row["returncode"] == 3
        receipt = json.loads(Path(row["stdout"]).read_text())
        assert receipt["stop_reason"] == "no_source_progress"
        assert not receipt["ready_for_collection"]
    for slot in json.loads((root / "recovery/resources.json").read_text())["slots"]:
        directory = Path(slot["directory"])
        assert not (directory / ACTIVE).exists()
        with file_lock(directory / LOCK, timeout=0):
            pass


@pytest.mark.parametrize("damage", ["root", "nested", "duplicate", "author", "scope", "policy", "seed",
                                    "delegation", "resource", "target", "reviewer", "task", "config_alias"])
def test_invalid_parallel_authorization_refuses_before_any_job_starts(batch, damage):
    root, plan = batch
    child = Path(plan["jobs"][0]["root"])
    if damage == "root":
        plan["jobs"][0]["root"] = str(root)
    elif damage == "nested":
        plan["jobs"][0]["root"] = str(child / "code")
    elif damage == "duplicate":
        plan["jobs"][1] = deepcopy(plan["jobs"][0])
    elif damage == "author":
        plan["author"] = "worker"
    elif damage in ("scope", "policy", "seed"):
        name = "other" if damage == "scope" else "owner"
        task = TaskStore(root).read(name)
        extra = {"scope": "code/owner.cc", "policy": "recovery/driver.json", "seed": "hints/attempt.cc"}[damage]
        task = replace(task, editable_files=(*task.editable_files, extra))
        target = Path(next(row["root"] for row in plan["jobs"] if row["task"] == name))
        for directory in (root, target):
            write(directory / f"recovery/tasks/{name}.json", task.to_dict())
    elif damage == "delegation":
        plan["jobs"][0]["coordinator"] = "recovery/coordinator.json"
    elif damage == "resource":
        plan["max_parallel"] = 3
    elif damage == "target":
        value = json.loads((child / "recovery/driver.json").read_text())
        write(child / "recovery/driver.json", {**value, "target": "unrelated"})
    elif damage == "reviewer":
        value = json.loads((child / "recovery/reviewer.json").read_text())
        write(child / "recovery/reviewer.json", {**value, "author": "worker"})
    elif damage == "task":
        task = TaskStore(child).read("owner")
        write(child / "recovery/tasks/owner.json", replace(task, intent="different permission").to_dict())
    elif damage == "config_alias":
        path = child / "recovery/worker.json"
        path.unlink()
        path.symlink_to(root / "recovery/worker.json")
    with pytest.raises(ValueError):
        dispatch(root, plan)
    assert not list((root / "build/signals").iterdir())


def test_dispatch_refuses_inherited_resources_and_busy_coordinator(batch, monkeypatch):
    root, plan = batch
    monkeypatch.setenv(INHERITED, "1")
    with pytest.raises(ValueError, match="inherited"):
        dispatch(root, plan)
    monkeypatch.delenv(INHERITED)
    with file_lock(root / "build/recovery/driver.lock"):
        with pytest.raises(TimeoutError):
            dispatch(root, plan)
    assert not list((root / "build/signals").iterdir())


def test_installed_dispatch_cli_refuses_invalid_plan(batch, capsys):
    root, plan = batch
    plan["jobs"][0]["coordinator"] = "not-allowed.json"
    write(root / "recovery/batch.json", plan)
    assert main(["dispatch", "--root", str(root), "--plan", "recovery/batch.json"]) == 3
    assert "scope delegation" in capsys.readouterr().err
    assert not list((root / "build/signals").iterdir())


def _replace_worker(batch, code):
    root, plan = batch
    write(root / "recovery/worker.json", {"author": "worker", "command": command(code)})
    git(root, "add", "recovery/worker.json")
    git(root, "commit", "-m", "Synthetic worker control")
    revision = git(root, "rev-parse", "HEAD")
    for row in plan["jobs"]:
        git(Path(row["root"]), "merge", "--ff-only", revision)


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_parallel_workers_preserve_candidates_and_discoveries_without_expanding_scope(batch):
    root, plan = batch
    other = Origin("a" * 64, "custom:offset", 48)
    response = {"discoveries": [{"hypothesis": "Another owner participates in this lifetime",
                                 "affects": [other.entity_id], "evidence": ["synthetic caller observation"]}]}
    code = ("import sys,json; from pathlib import Path; payload=json.load(sys.stdin); "
            "source=Path(payload['editable_files'][0]); "
            "source.write_text(source.read_text() + '\\n// synthetic candidate\\n'); "
            f"print({json.dumps(response)!r})")
    _replace_worker(batch, code)
    before = working_state(root)
    report = dispatch(root, plan)
    assert working_state(root) == before
    assert not report["batch_completed"] and not report["ready_for_collection"]
    for row in report["jobs"]:
        receipt = json.loads(Path(row["stdout"]).read_text())
        assert receipt["stop_reason"] == "review_scope_expansion"
        assert row["scope_intact"]
        child = Path(row["root"])
        assert "synthetic candidate" in (child / f"code/{row['task']}.cc").read_text()
        decisions = TaskStore(child).decisions(row["task"])
        assert [decision["kind"] for decision in decisions] == ["discovery"]
        assert decisions[0]["affects"] == [other.entity_id]
        assert not TaskStore(root).decisions(row["task"])


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_outer_timeout_quarantines_native_worker_resources(batch):
    root, plan = batch
    _replace_worker(batch, "import time; time.sleep(60)")
    plan["timeout"] = 1
    report = dispatch(root, plan)
    assert not report["batch_completed"] and not report["ready_for_collection"]
    assert all(row["timed_out"] for row in report["jobs"])
    for slot in json.loads((root / "recovery/resources.json").read_text())["slots"]:
        directory = Path(slot["directory"])
        assert (directory / ACTIVE).exists()
        with file_lock(directory / LOCK, timeout=0):
            pass  # Kernel lease released, marker still explicitly quarantined.


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_dispatch_detects_coordinator_edits_without_reverting_them(batch):
    root, plan = batch
    def progress(event):
        if event == {"job": 0, "event": "finished", "returncode": 3, "timed_out": False}:
            (root / "external-note.txt").write_text("preserve concurrent work")
    report = dispatch(root, plan, progress=progress)
    assert report["coordinator_changed"] and not report["batch_completed"]
    assert (root / "external-note.txt").read_text() == "preserve concurrent work"


@pytest.mark.parametrize("change", [{"schema_version": True}, {"evidence": []}, {"max_parallel": True},
                                    {"timeout": 0}, {"jobs": []}, {"unknown": "field"}])
def test_dispatch_plan_validation_is_fail_closed(batch, change):
    root, plan = batch
    with pytest.raises(ValueError):
        dispatch(root, {**plan, **change})
    assert not list((root / "build/signals").iterdir())


@pytest.mark.parametrize("variable", ["GIT_INDEX_FILE", "GIT_DIR", "GIT_WORK_TREE", "GIT_COMMON_DIR"])
def test_git_environment_cannot_alias_worker_state(batch, monkeypatch, variable):
    root, plan = batch
    monkeypatch.setenv(variable, str(root / ".git/index"))
    with pytest.raises(ValueError, match="Git path overrides"):
        dispatch(root, plan)


def test_in_progress_merge_is_preserved_and_refused(batch):
    root, plan = batch
    child = Path(plan["jobs"][0]["root"])
    marker = Path(git(child, "rev-parse", "--path-format=absolute", "--git-path", "MERGE_HEAD"))
    marker.write_text(git(root, "rev-parse", "HEAD") + "\n")
    with pytest.raises(ValueError, match="existing Git operation"):
        dispatch(root, plan)
    assert marker.exists()
    assert not list((root / "build/signals").iterdir())
