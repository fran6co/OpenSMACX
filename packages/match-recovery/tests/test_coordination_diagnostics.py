"""Discovery decisions see fresh scoped diagnostics, never inherited verdicts."""

from copy import deepcopy
import json

import pytest

from match_recovery.coordination import coordinate
from match_recovery.driver import drive, working_state
from match_recovery.investigation_round import authorize
from test_coordination import answer, discover, setup
from test_investigation_round import driver_configuration, role
from test_source_configuration import configured
from test_task_source import git, project


def configuration(setup):
    return driver_configuration(setup[2])


@pytest.mark.parametrize("result", ["pass", "fail", "unknown"])
def test_fresh_scoped_report_reaches_coordinator_without_resolving_pending_finding(setup, result):
    root, store, task, _, delegation = setup
    store.discover(task.task_id, request_id="layout", author="worker",
                   hypothesis="Previous attempt could not invoke the configured compiler",
                   affects=[task.subjects[0].entity_id], evidence=["Old unavailable compiler run"], expected_revision=0)
    before = store.decisions(task.task_id), store.scope(task.task_id)
    context = {"current_scope": sorted(store.scope(task.task_id)[0]), "notes": ["Current source, not an old report"]}
    check = {"result": result, "restoration": "pass", "scope_complete": False,
             "findings": [{"entity_id": task.subjects[0].entity_id, "detail": "Candidate layout remains unknown"}]}
    calls = []

    def execute(command, current, name, payload):
        selected = command.argv[0]
        calls.append(selected)
        assert current == root and name == task.task_id
        if selected == "context":
            assert payload["task"] == task.to_dict()
            return 0, deepcopy(context)
        if selected == "check":
            assert payload["context"] == context
            return {"pass": 0, "fail": 1, "unknown": 3}[result], deepcopy(check)
        assert selected == "coordinate", "Diagnostics may not invoke build, prepare, review or acceptance"
        observation = payload["diagnostics"]
        assert observation["context"] == {"exit_code": 0, "report": context}
        assert observation["check"] == {"exit_code": {"pass": 0, "fail": 1, "unknown": 3}[result], "report": check}
        assert observation["verification"] == "diagnostic_only" and observation["ready_for_collection"] is False
        assert observation["task_id"] == task.task_id and len(observation["epoch"]) == 64
        assert payload["pending_discoveries"] == [before[0][0]]
        assert store.decisions(task.task_id) == before[0]
        return 0, answer(payload, "investigate")

    report = coordinate(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
    assert calls == ["context", "check", "coordinate"]
    assert report["action"] == "investigate" and not report["published"] and not report["ready_for_collection"]
    assert (store.decisions(task.task_id), store.scope(task.task_id)) == before


def test_unavailable_context_is_observed_without_running_check_or_manufacturing_a_pass(setup):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    calls = []

    def execute(command, current, name, payload):
        selected = command.argv[0]
        calls.append(selected)
        if selected == "context":
            return 3, {"error": "Current context unavailable"}
        assert selected == "coordinate", "Unavailable context cannot supply a fabricated check input"
        assert payload["diagnostics"]["context"] == {"exit_code": 3, "report": {"error": "Current context unavailable"}}
        assert payload["diagnostics"]["check"] is None
        return 0, answer(payload, "defer")

    result = coordinate(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
    assert calls == ["context", "coordinate"] and result["action"] == "defer"
    assert len(store.decisions(task.task_id)) == 1


@pytest.mark.parametrize("stage", ["context", "check"])
@pytest.mark.parametrize("damage", ["source", "ignored_source", "ignored_history", "config_file", "config_object"])
def test_changed_diagnostic_inputs_refuse_before_coordinator_and_preserve_changes(setup, stage, damage):
    root, store, task, _, delegation = setup
    selected = configuration(setup)
    ignore = root / ".gitignore"
    ignore.write_text(ignore.read_text() + "code/ignored.hpp\nrecovery/history/\n")
    config_file = root / "recovery/driver.json"
    config_file.write_text(json.dumps(selected))
    git(root, "add", ".gitignore", "recovery/driver.json")
    git(root, "commit", "-m", "Declare synthetic diagnostic inputs")
    ignored = root / "code/ignored.hpp"
    ignored.write_text("void diagnostic_input();\n")
    discover(setup, task.subjects[0].entity_id)
    before = working_state(root)
    calls = []

    def execute(command, current, name, payload):
        name = command.argv[0]
        calls.append(name)
        assert name in ("context", "check"), "Stale diagnostic reached coordinator or publication"
        if name == stage:
            if damage == "source":
                path = root / "code/owner.cc"
                path.write_text(path.read_text() + "// Concurrent source retained\n")
            elif damage == "ignored_source":
                ignored.write_text("void concurrent_ignored_input();\n")
                assert working_state(root) == before
            elif damage == "ignored_history":
                store.discover(task.task_id, request_id="late", author="worker", hypothesis="New evidence invalidates diagnostics",
                               affects=[task.subjects[0].entity_id], evidence=["Concurrent decision"], expected_revision=1)
                assert working_state(root) == before
            elif damage == "config_file":
                config_file.write_text(config_file.read_text() + "\n")
            else:
                selected["check"]["timeout"] += 1
        return (0, {}) if name == "context" else (0, {"result": "pass"})

    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, diagnostic_configuration=selected, execute=execute)
    assert stage in calls and "coordinate" not in calls
    assert store.read(task.task_id).base_revision == task.base_revision
    assert len(store.decisions(task.task_id)) == (2 if damage == "ignored_history" else 1)
    if damage == "source":
        assert "Concurrent source retained" in (root / "code/owner.cc").read_text()
    elif damage == "ignored_source":
        assert ignored.read_text() == "void concurrent_ignored_input();\n"


@pytest.mark.parametrize("code,report", [(0, {"result": "unknown"}), (3, {"result": "pass"}),
                                        (1, {"result": "pass"}), (0, {}), (0, []),
                                        (False, {"result": "pass"}), (2, {"result": "fail"})])
def test_inconsistent_check_protocol_refuses_before_provider(setup, code, report):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)

    def execute(command, current, name, payload):
        name = command.argv[0]
        if name == "context":
            return 0, {}
        assert name == "check", "Malformed observation reached coordinator"
        return code, report

    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
    assert len(store.decisions(task.task_id)) == 1


def test_diagnostic_observations_participate_in_fresh_coordinator_snapshot(setup):
    root, _, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    observed, current_diagnostic = [], ["first live observation"]

    def execute(command, current, name, payload):
        name = command.argv[0]
        if name == "context": return 0, {}
        if name == "check": return 3, {"result": "unknown", "diagnostic": current_diagnostic[0]}
        assert name == "coordinate"
        observed.append(payload)
        return 0, answer(payload, "investigate")

    for diagnostic in ("first live observation", "second live observation"):
        current_diagnostic[0] = diagnostic
        coordinate(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
    assert observed[0]["diagnostics"]["epoch"] == observed[1]["diagnostics"]["epoch"]
    assert observed[0]["snapshot"] != observed[1]["snapshot"]


def test_serial_scope_expansion_collects_new_diagnostics_before_next_decision(setup):
    root, store, task, caller, delegation = setup
    discover(setup)
    observations, calls = [], []

    def execute(command, current, name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "context":
            return 0, {"subjects": sorted(store.scope(task.task_id)[0])}
        if name == "check":
            return 3, {"result": "unknown", "scoped_subjects": payload["context"]["subjects"]}
        if name == "build":
            assert len(observations) == 2, "Precoordination diagnostics must not build"
            return 0, {"result": "pass"}
        if name == "coordinate":
            observations.append(payload["diagnostics"])
            return 0, answer(payload, "expand" if len(observations) == 1 else "investigate")
        assert name == "worker" and len(observations) == 2
        return 0, {"discoveries": []}

    result = drive(root, task.task_id, configuration(setup), role("worker"), iterations=1,
                   reviewer_configuration=role("reviewer"), coordinator_configuration=delegation, execute=execute)
    assert len(observations) == 2 and observations[0]["epoch"] != observations[1]["epoch"]
    assert observations[0]["check"]["report"]["scoped_subjects"] == [task.subjects[0].entity_id]
    assert set(observations[1]["check"]["report"]["scoped_subjects"]) == {task.subjects[0].entity_id, caller.entity_id}
    assert calls[:6] == ["context", "check", "coordinate"] * 2
    assert result["worker_rounds"] == 1 and not result["ready_for_collection"]
    assert store.read(task.task_id).base_revision == task.base_revision
    assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]


def test_parallel_preplanning_observes_fresh_reports_without_resolving_findings(setup):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    store.discover(task.task_id, request_id="second", author="worker", hypothesis="Second issue needs current evidence",
                   affects=[task.subjects[0].entity_id], evidence=["Earlier observation"], expected_revision=1)
    before = store.decisions(task.task_id)
    calls, coordinated = [], []

    def execute(command, current, name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "context": return 0, {"scope": sorted(store.scope(task.task_id)[0])}
        if name == "check": return 0, {"result": "pass"}
        assert name == "coordinate", "Planning cannot run build, preparation or workers"
        assert payload["diagnostics"]["check"]["report"]["result"] == "pass"
        finding = "layout" if not coordinated else "second"
        coordinated.append(finding)
        return 0, answer(payload, "investigate", discovery_id=finding)

    result = authorize(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
    assert result["status"] == "authorized" and coordinated == ["layout", "second"]
    assert calls == ["context", "check", "coordinate"] * 2
    assert store.decisions(task.task_id) == before


@pytest.mark.parametrize("damage", [None, "stale", "acceptance", "verification", "wrong_task", "check_exit"])
def test_agent_bridge_independently_validates_diagnostic_request(setup, damage):
    from match_recovery.agent_command import _request

    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    requests = []

    def execute(command, current, name, payload):
        name = command.argv[0]
        if name == "context": return 0, {"source": "current"}
        if name == "check": return 3, {"result": "unknown", "diagnostic": "Unmeasured candidate"}
        assert name == "coordinate"
        requests.append(json.loads(json.dumps(payload)))  # The bridge consumes the JSON wire representation.
        return 0, answer(payload, "defer")

    coordinate(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
    request, = requests
    assert _request(root, "coordinator", request)[0] == task
    if damage == "stale":
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// Source changed after diagnostics\n")
    elif damage == "acceptance":
        request["diagnostics"]["ready_for_collection"] = True
    elif damage == "verification":
        request["diagnostics"]["verification"] = "accepted"
    elif damage == "wrong_task":
        request["diagnostics"]["task_id"] = "different-task"
    elif damage == "check_exit":
        request["diagnostics"]["check"]["exit_code"] = 0
    if damage is not None:
        with pytest.raises(ValueError):
            _request(root, "coordinator", request)
    else:
        actual_task, decisions, editable = _request(root, "coordinator", request)
        assert actual_task == task and decisions == store.decisions(task.task_id) and not editable
    assert len(store.decisions(task.task_id)) == 1


def test_new_journal_finding_before_collection_cannot_validate_an_older_coordinator_request(setup, monkeypatch):
    from match_recovery import coordination_diagnostics

    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    before_source = {path: path.read_bytes() for path in (root / "code").iterdir()}
    original = coordination_diagnostics.collect
    calls = []

    def collect(*args, **kwargs):
        # coordinate already captured the discovery list. The real collector
        # below sees a fresh, internally consistent epoch containing this new
        # event, which must not make the older request authoritative again.
        store.discover(task.task_id, request_id="between-snapshots", author="worker",
                       hypothesis="New evidence arrived before diagnostics started",
                       affects=[task.subjects[0].entity_id], evidence=["Concurrent journal append"],
                       expected_revision=1)
        return original(*args, **kwargs)

    def execute(command, current, name, payload):
        name = command.argv[0]
        calls.append(name)
        assert name in ("context", "check"), "Fresh diagnostics cannot legitimize a stale discovery request"
        return (0, {}) if name == "context" else (0, {"result": "pass"})

    monkeypatch.setattr(coordination_diagnostics, "collect", collect)
    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
    assert calls == ["context", "check"]
    assert len(store.decisions(task.task_id)) == 2
    assert [row["discovery_id"] for row in store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]] == [
        "layout", "between-snapshots"]
    assert all(path.read_bytes() == contents for path, contents in before_source.items())
