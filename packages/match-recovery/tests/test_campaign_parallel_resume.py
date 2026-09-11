"""Campaign resume plans centrally before native parallel investigation work."""

from copy import deepcopy
import json
import os
from pathlib import Path

import pytest

from match_recovery.campaign import _resume_round
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.tasks import TaskStore
from test_coordination import answer
from test_dispatch import batch
from test_source_configuration import configured
from test_task_source import git, project


@pytest.fixture
def retained_batch(batch):
    root, plan = batch
    directory = root / "build/campaign-resume"
    directory.mkdir()
    for job in plan["jobs"]:
        child, name = Path(job["root"]), job["task"]
        store = TaskStore(child)
        task = store.read(name)
        store.discover(name, request_id="pending", author="worker", hypothesis="Investigate retained ownership concern",
                       affects=[task.subjects[0].entity_id], evidence=["Current source"], expected_revision=0)
        path = child / task.editable_files[0]
        path.write_text(path.read_text() + "// preserved source investigation\n")
    profiles = {"resources": json.loads((root / "recovery/resources.json").read_text()),
                "driver": json.loads((root / "recovery/driver.json").read_text()),
                "coordinator": {"schema_version": 1, "author": "coordinator",
                                "command": {"argv": ["coordinate"], "timeout": 10},
                                "subjects": [], "editable_files": ["code/owner.hpp"],
                                "evidence": ["One bounded current-owner investigation"], "max_decisions": 1}}
    return root, plan, profiles, directory, git(root, "rev-parse", "HEAD")


def diagnostic_result(command, child, name, payload, profiles, calls):
    """Simulate configured observations; real dispatched children remain native."""
    assert Path(os.environ["TEST_COMPILER_SLOT"]).is_dir()
    if command.argv == tuple(profiles["driver"]["context"]["argv"]):
        assert payload["task"]["task_id"] == name
        calls.append((name, "context"))
        return 0, {"task_id": name, "source": f"code/{name}.cc"}
    if command.argv == tuple(profiles["driver"]["check"]["argv"]):
        assert calls[-1] == (name, "context")
        assert payload["task"]["task_id"] == name
        assert payload["context"] == {"task_id": name, "source": f"code/{name}.cc"}
        calls.append((name, "check"))
        return 1, {"result": "fail"}
    assert command.argv == ("coordinate",)
    assert calls[-2:] == [(name, "context"), (name, "check")]
    observation = payload["diagnostics"]
    assert observation["task_id"] == name and observation["verification"] == "diagnostic_only"
    assert observation["ready_for_collection"] is False
    assert observation["context"] == {"exit_code": 0, "report": {"task_id": name, "source": f"code/{name}.cc"}}
    assert observation["check"] == {"exit_code": 1, "report": {"result": "fail"}}
    calls.append((name, "coordinate"))
    return None


def run_round(retained_batch, execute, *, announce=lambda *args, **kwargs: None):
    root, plan, profiles, directory, revision = retained_batch
    before = working_state(root)

    def fresh():
        assert working_state(root) == before, "Central coordination must not change the root checkout"

    with file_lock(root / "build/recovery/driver.lock", timeout=0) as lease:
        return _resume_round(root, plan, profiles, directory, 0, revision,
                             execute=execute, fresh=fresh, announce=announce, lease=lease)


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_campaign_completes_central_planning_before_real_parallel_resume(retained_batch):
    root, plan, profiles, _, _ = retained_batch
    original_plan = deepcopy(plan)
    children = {job["task"]: Path(job["root"]) for job in plan["jobs"]}
    prior = {name: (TaskStore(child).read(name), TaskStore(child).decisions(name),
                    (child / f"code/{name}.cc").read_bytes()) for name, child in children.items()}
    coordinated, events, calls = [], [], []

    def execute(command, child, name, payload):
        assert not list((root / "build/signals").iterdir())
        result = diagnostic_result(command, child, name, payload, profiles, calls)
        if result is not None:
            return result
        coordinated.append(name)
        return 0, answer(payload, "investigate" if name == "owner" else "investigate_local", discovery_id="pending")

    def announce(stage, **fields):
        events.append((stage, fields))
        if stage == "worker" and fields["event"] == "started":
            assert coordinated == ["owner", "other"]

    report, decisions, deferred = run_round(retained_batch, execute, announce=announce)
    assert calls == [(name, stage) for name in ("owner", "other") for stage in ("context", "check", "coordinate")]
    assert not deferred and [row["status"] for row in decisions] == ["authorized", "authorized"]
    assert plan == original_plan and all("investigation" not in job for job in plan["jobs"])
    assert not report["ready_for_collection"] and not report["batch_completed"]
    assert len({(root / "build/signals" / name).read_text() for name in children}) == 2
    assert len([fields for event, fields in events if event == "worker" and fields["event"] == "started"]) == 2
    for row in report["jobs"]:
        name, child = row["task"], Path(row["root"])
        receipt = json.loads(Path(row["stdout"]).read_text())
        assert row["returncode"] == 3 and not row["timed_out"]
        assert receipt["worker_rounds"] == 1 and receipt["stop_reason"] == "no_source_progress"
        assert not receipt["ready_for_collection"]
        assert (TaskStore(child).read(name), TaskStore(child).decisions(name),
                (child / f"code/{name}.cc").read_bytes()) == prior[name]
        assert not TaskStore(root).decisions(name)


@pytest.mark.parametrize("when", ["between_authorizations", "inside_peer_coordination"])
def test_campaign_refuses_stale_grant_or_peer_mutation_before_dispatch(retained_batch, when):
    root, plan, profiles, _, _ = retained_batch
    owner = Path(plan["jobs"][0]["root"]) / "code/owner.cc"
    calls = []

    def mutate():
        owner.write_text(owner.read_text() + "// concurrent source work retained\n")

    def execute(command, child, name, payload):
        result = diagnostic_result(command, child, name, payload, profiles, calls)
        if result is not None:
            return result
        if when == "inside_peer_coordination" and name == "other": mutate()
        return 0, answer(payload, "investigate", discovery_id="pending")

    def announce(stage, **fields):
        assert stage != "worker", "Stale worker must be refused before process creation"
        if when == "between_authorizations" and stage == "authorize_investigation" and fields["task"] == "other":
            mutate()

    with pytest.raises(ValueError):
        run_round(retained_batch, execute, announce=announce)
    assert "concurrent source work retained" in owner.read_text()
    assert calls[:3] == [("owner", stage) for stage in ("context", "check", "coordinate")]
    assert not list((root / "build/signals").iterdir())


@pytest.mark.parametrize("action", ["defer", "expand"])
def test_campaign_routes_deferral_and_publishing_proposals_out_of_parallel_workers(retained_batch, monkeypatch, action):
    import match_recovery.campaign as implementation

    root, plan, profiles, _, _ = retained_batch
    before = {job["task"]: TaskStore(Path(job["root"])).decisions(job["task"]) for job in plan["jobs"]}
    routed, calls = [], []

    def execute(command, child, name, payload):
        result = diagnostic_result(command, child, name, payload, profiles, calls)
        if result is not None:
            return result
        selected = action if name == "owner" else "investigate"
        return 0, answer(payload, selected, discovery_id="pending", subjects=[],
                         editable_files=["code/owner.hpp"] if selected == "expand" else [])

    def dispatch(root, selected_plan, **kwargs):
        assert calls == [(name, stage) for name in ("owner", "other") for stage in ("context", "check", "coordinate")]
        routed.append(selected_plan)
        assert [job["task"] for job in selected_plan["jobs"]] == ["other"]
        assert "coordinator" not in selected_plan["jobs"][0]
        assert selected_plan["jobs"][0]["investigation"]["task"] == "other"
        return {"jobs": [], "batch_completed": False, "ready_for_collection": False}

    monkeypatch.setattr(implementation, "dispatch", dispatch)
    report, decisions, deferred = run_round(retained_batch, execute)
    assert len(routed) == 1 and not report["ready_for_collection"]
    assert decisions[0]["status"] == ("deferred" if action == "defer" else "serial_coordination_required")
    assert deferred == ({"owner"} if action == "defer" else set())
    for job in plan["jobs"]:
        store = TaskStore(Path(job["root"]))
        assert store.decisions(job["task"]) == before[job["task"]]
        assert store.scope(job["task"]) == TaskStore(root).scope(job["task"])
