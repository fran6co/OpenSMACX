"""Native subprocess dispatch of fresh, single-use investigation permissions."""

import json
import os
from pathlib import Path

import pytest

from match_recovery.dispatch import dispatch
from match_recovery.driver import working_state
from match_recovery.investigation_round import authorize
from match_recovery.tasks import TaskStore
from test_coordination import answer
from test_dispatch import _replace_worker, batch
from test_source_configuration import configured
from test_task_source import project


@pytest.fixture
def investigation_batch(batch):
    root, plan = batch
    _replace_worker(batch, "from pathlib import Path; "
                    "p=Path('build/worker-rounds'); "
                    "p.write_text((p.read_text() if p.exists() else '') + 'worker\\n'); "
                    "print('{\"discoveries\": []}')")
    for index, job in enumerate(plan["jobs"]):
        child, task_name = Path(job["root"]), job["task"]
        store = TaskStore(child)
        task = store.read(task_name)
        affected = task.subjects[0].entity_id if index == 0 else store.read("owner").subjects[0].entity_id
        store.discover(task_name, request_id="pending", author="worker", hypothesis="Investigate retained ownership evidence",
                       affects=[affected], evidence=["Current source model"], expected_revision=0)
        path = child / task.editable_files[0]
        path.write_text(path.read_text() + "// retained scoped worker candidate\n")
        policy = {"schema_version": 1, "author": task.author,
                  "command": {"argv": ["coordinate"], "timeout": 10},
                  "subjects": [], "editable_files": [], "evidence": ["One current-scope investigation round"],
                  "max_decisions": 1}
        action = "investigate" if index == 0 else "investigate_local"
        result = authorize(child, task_name, policy,
                           execute=lambda *args, action=action: (0, answer(args[-1], action, discovery_id="pending")))
        assert result["status"] == "authorized"
        job["investigation"] = result["authorization"]
    return root, plan


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_native_dispatch_runs_two_scoped_investigations_with_distinct_resources(investigation_batch):
    root, plan = investigation_batch
    before = working_state(root)
    decisions = {job["task"]: TaskStore(Path(job["root"])).decisions(job["task"]) for job in plan["jobs"]}
    events = []
    report = dispatch(root, plan, progress=events.append)
    assert [event["event"] for event in events[:2]] == ["started", "started"]
    assert working_state(root) == before
    assert not report["batch_completed"] and not report["ready_for_collection"]
    assert len({(root / "build/signals" / name).read_text() for name in ("owner", "other")}) == 2
    for row, job in zip(report["jobs"], plan["jobs"]):
        assert row["returncode"] == 3 and row["scope_intact"] and not row["timed_out"]
        receipt = json.loads(Path(row["stdout"]).read_text())
        assert receipt["worker_rounds"] == 1 and receipt["stop_reason"] == "no_source_progress"
        assert not receipt["ready_for_collection"]
        child, name = Path(job["root"]), job["task"]
        store = TaskStore(child)
        assert store.decisions(name) == decisions[name]
        assert store.status(name, store.scope(name)[0])["pending_discoveries"]
        assert not TaskStore(root).decisions(name), "Child findings must not be copied into the coordinator journal"
        assert (child / "build/worker-rounds").read_text() == "worker\n"
        token = job["investigation"]["request_id"]
        assert (child / "build/recovery/investigation-uses" / (token + ".json")).is_file()


@pytest.mark.parametrize("damage", ["source", "journal", "iterations", "coordinator"])
def test_dispatch_refuses_stale_or_unbounded_investigation_before_starting_any_job(investigation_batch, damage):
    root, plan = investigation_batch
    job = plan["jobs"][1]  # A bad later job must also prevent the first child starting.
    child, name = Path(job["root"]), job["task"]
    if damage == "source":
        path = child / "code/other.cc"
        path.write_text(path.read_text() + "// subsequent source work retained\n")
    elif damage == "journal":
        store = TaskStore(child)
        store.discover(name, request_id="new", author="worker", hypothesis="New structural finding",
                       affects=[store.read(name).subjects[0].entity_id], evidence=["Current source"], expected_revision=1)
    elif damage == "iterations": job["iterations"] = 2
    else: job["coordinator"] = "recovery/coordinator.json"
    events = []
    with pytest.raises(ValueError):
        dispatch(root, plan, progress=events.append)
    assert not events
    assert not list((root / "build/signals").iterdir())
    assert all(not (Path(row["root"]) / "build/worker-rounds").exists() for row in plan["jobs"])
    if damage == "source":
        assert "subsequent source work retained" in (child / "code/other.cc").read_text()


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_repeated_dispatch_never_executes_a_worker_again_with_consumed_permission(investigation_batch):
    root, plan = investigation_batch
    first = dispatch(root, plan)
    assert all(json.loads(Path(row["stdout"]).read_text())["worker_rounds"] == 1 for row in first["jobs"])
    repeated = dispatch(root, plan)
    assert not repeated["batch_completed"] and not repeated["ready_for_collection"]
    for row in repeated["jobs"]:
        assert row["returncode"] == 3 and not row["timed_out"]
        assert "already consumed" in Path(row["stderr"]).read_text()
        assert (Path(row["root"]) / "build/worker-rounds").read_text() == "worker\n"
        store = TaskStore(Path(row["root"]))
        assert len(store.decisions(row["task"])) == 1
