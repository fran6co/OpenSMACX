"""Only pure policy refusals permit fresh, bounded coordinator correction."""

from copy import deepcopy
import json

import pytest

from match_recovery.coordination import SerialCoordinationRequired, coordinate, rejection_feedback
from match_recovery.driver import drive, working_state
from match_recovery.investigation_round import authorize, epoch
from match_recovery.tasks import TaskStore
from test_coordination import answer, discover, setup
from test_source_configuration import configured
from test_task_source import project


def configuration(task):
    return {"schema_version": 1, "target": task.target,
            **{name: {"argv": [name], "timeout": 10} for name in ("context", "check", "acceptance")}}


def role(name):
    return {"author": name, "command": {"argv": [name], "timeout": 10}}


def test_action_facts_respect_a_narrower_read_only_phase(setup):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    before = working_state(root)

    def execute(command, current, name, payload):
        facts, = payload["decision_facts"]
        assert facts["actions"]["defer"] == "read_only"
        assert facts["actions"]["investigate"] == facts["actions"]["investigate_local"] == "serial_required"
        return 0, answer(payload, "investigate")

    with pytest.raises(SerialCoordinationRequired):
        coordinate(root, task.task_id, delegation, allowed_actions=("defer",), execute=execute)
    assert working_state(root) == before and len(store.decisions(task.task_id)) == 1


def test_rejected_resolution_requires_explicit_useful_local_proposal_before_worker(setup):
    root, store, task, caller, delegation = setup
    discover(setup)
    delegation.update(subjects=[], editable_files=[], max_decisions=2)
    history, scope = store.decisions(task.task_id), store.scope(task.task_id)
    calls, proposals, diagnostics = [], [], []

    def execute(command, current, name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "context": return 0, {}
        if stage == "check":
            return 0, {"result": "pass", "observation": calls.count("check")}
        if stage == "coordinate":
            proposals.append(payload)
            diagnostics.append(payload["diagnostics"])
            assert payload["decision_facts"]
            facts, = payload["decision_facts"]
            assert facts["outside_delegation"] == [caller.entity_id]
            assert facts["actions"]["reject"] == facts["actions"]["investigate"] == "unavailable"
            assert facts["actions"]["investigate_local"] == "conditional_local_work"
            assert payload["pending_discoveries"][0]["affects"] == [caller.entity_id]
            if len(proposals) == 1:
                assert "previous_rejection" not in payload
                return 0, answer(payload, "reject")
            assert payload["previous_rejection"]["refusal"]["reason_code"] == "outside_delegated_scope"
            assert payload["previous_rejection"]["refusal"]["snapshot"] == proposals[0]["snapshot"]
            return 0, answer(payload, "investigate_local", evidence=["Inspect the current owner's declared call relationship; leave the caller untouched"])
        assert stage == "worker", "Pending findings cannot reach reviewer or acceptance"
        assert len(proposals) == 2 and payload["editable_files"] == ["code/owner.cc"]
        assert payload["context"]["pending_discoveries"][0]["affects"] == [caller.entity_id]
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// inspect owned side of unresolved caller relationship\n")
        return 0, {"discoveries": []}

    result = drive(root, task.task_id, configuration(task), role("worker"), iterations=1,
                   reviewer_configuration=role("reviewer"), coordinator_configuration=delegation, execute=execute)
    assert calls[:6] == ["context", "check", "coordinate"] * 2
    assert diagnostics[0]["check"]["report"]["observation"] == 1
    assert diagnostics[1]["check"]["report"]["observation"] == 2
    assert proposals[0]["snapshot"] != proposals[1]["snapshot"]
    assert result["worker_rounds"] == 1 and not result["ready_for_collection"]
    assert store.decisions(task.task_id) == history and store.scope(task.task_id) == scope
    assert store.read(task.task_id).base_revision == task.base_revision


def test_successful_native_expansion_clears_feedback_and_rederives_next_decision_facts(setup):
    root, store, task, caller, delegation = setup
    discover(setup)
    delegation["max_decisions"] = 3
    requests = []

    def execute(command, current, name, payload):
        stage = command.argv[0]
        if stage == "context": return 0, {}
        if stage == "check": return 0, {"result": "pass"}
        if stage == "coordinate":
            requests.append(payload)
            if len(requests) == 1:
                assert "previous_rejection" not in payload
                return 0, answer(payload, "investigate")
            if len(requests) == 2:
                assert payload["previous_rejection"]["refusal"]["reason_code"] == "investigation_requires_expansion"
                return 0, answer(payload, "expand")
            assert len(requests) == 3 and "previous_rejection" not in payload
            assert payload["decision_revision"] == 2
            facts, = payload["decision_facts"]
            assert facts["currently_owned"] == [caller.entity_id] and not facts["delegated_repairs"]
            assert facts["actions"]["investigate"] == "read_only"
            return 0, answer(payload, "investigate")
        assert stage == "worker" and len(requests) == 3
        return 0, {"discoveries": []}

    result = drive(root, task.task_id, configuration(task), role("worker"), iterations=1,
                   coordinator_configuration=delegation, execute=execute)
    assert result["worker_rounds"] == 1 and not result["ready_for_collection"]
    assert requests[1]["diagnostics"]["epoch"] != requests[2]["diagnostics"]["epoch"]
    assert [row["kind"] for row in store.decisions(task.task_id)] == ["discovery", "scope_expanded"]
    assert store.read(task.task_id).base_revision == task.base_revision
    assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]


@pytest.mark.parametrize("budget", [2, 3])
def test_partial_investigation_and_rejected_retry_share_one_budget_without_resolving_findings(setup, budget):
    root, store, task, caller, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    store.discover(task.task_id, request_id="outside", author="worker", hypothesis="A different owner remains involved",
                   affects=[caller.entity_id], evidence=["caller source"], expected_revision=1)
    delegation.update(subjects=[], editable_files=[], max_decisions=budget)
    before = working_state(root), store.decisions(task.task_id), store.scope(task.task_id)
    proposals, hooks = [], []

    def execute(command, current, name, payload):
        stage = command.argv[0]
        hooks.append(stage)
        if stage == "context": return 0, {}
        if stage == "check": return 3, {"result": "unknown", "attempt": hooks.count("check")}
        assert stage == "coordinate"
        proposals.append(payload)
        if len(proposals) == 1:
            return 0, answer(payload, "investigate")
        assert payload["investigating"] == ["layout"] and len(payload["pending_discoveries"]) == 2
        if len(proposals) == 2:
            return 0, answer(payload, "investigate", discovery_id="outside")
        assert payload["previous_rejection"]["refusal"]["discovery_id"] == "outside"
        return 0, answer(payload, "investigate_local", discovery_id="outside",
                         evidence=["Inspect owned call-site evidence only; the other owner remains unresolved"])

    result = authorize(root, task.task_id, delegation, execute=execute, diagnostic_configuration=configuration(task))
    assert len(proposals) == budget and hooks == ["context", "check", "coordinate"] * budget
    assert (working_state(root), store.decisions(task.task_id), store.scope(task.task_id)) == before
    if budget == 2:
        assert result["status"] == "deferred" and "authorization" not in result
    else:
        assert result["status"] == "authorized"
        assert {row["discovery_id"] for row in result["authorization"]["investigations"]} == {"layout", "outside"}


@pytest.mark.parametrize("damage", [None, "facts", "feedback", "scope"])
def test_native_agent_bridge_rederives_decision_facts_and_validates_correction_feedback(setup, damage):
    from match_recovery.agent_command import _request

    root, store, task, caller, delegation = setup
    discover(setup)
    feedback = previous_refusal(setup)
    captured = []

    def execute(command, current, name, payload):
        captured.append(json.loads(json.dumps(payload)))
        return 0, answer(payload, "defer")

    coordinate(root, task.task_id, delegation, previous_rejection=feedback, execute=execute)
    request, = captured
    facts, = request["decision_facts"]
    assert facts["currently_owned"] == [] and facts["delegated_repairs"] == [caller.entity_id]
    assert facts["actions"]["investigate"] == "unavailable" and facts["actions"]["expand"] == "central"
    if damage == "facts": facts["actions"]["incorporate"] = "central"
    elif damage == "feedback": request["previous_rejection"]["refusal"]["ready_for_collection"] = True
    elif damage == "scope":
        store.expand(task.task_id, request_id="fresh-scope", author=task.author,
                     subjects=(caller,), editable_files=("code/caller.cc",), evidence=("New scope",), expected_revision=1)
    if damage is None:
        actual, history, _ = _request(root, "coordinator", request)
        assert actual == task and history == store.decisions(task.task_id)
    else:
        with pytest.raises(ValueError): _request(root, "coordinator", request)


def previous_refusal(setup):
    root, _, task, _, delegation = setup
    before = epoch(root, task.task_id)
    refusal = coordinate(root, task.task_id, delegation, execute=lambda *args: (0, answer(args[-1], "investigate")))
    assert refusal["action"] == "proposal_rejected"
    return rejection_feedback(root, task.task_id, delegation, refusal, epoch_before=before)


@pytest.mark.parametrize("damage", ["source", "history", "runtime", "policy"])
def test_changed_rejection_context_refuses_before_diagnostics_or_second_provider(setup, monkeypatch, damage):
    root, store, task, _, delegation = setup
    discover(setup)
    feedback = previous_refusal(setup)
    if damage == "source":
        path = root / "code/owner.cc"
        path.write_text(path.read_text() + "// concurrent source retained\n")
    elif damage == "history":
        store.discover(task.task_id, request_id="late", author="worker", hypothesis="New evidence",
                       affects=[task.subjects[0].entity_id], evidence=["source"], expected_revision=1)
    elif damage == "runtime":
        monkeypatch.setattr("match_recovery.framework_runtime.fingerprint", lambda: "f" * 64)
    else:
        delegation["evidence"].append("Changed authorization policy")
    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, previous_rejection=feedback,
                   diagnostic_configuration=configuration(task),
                   execute=lambda *args: pytest.fail("Stale correction reached a diagnostic hook or provider"))
    assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]


@pytest.mark.parametrize("damage", ["schema", "reason", "discovery", "published", "proposed_action"])
def test_malformed_rejection_feedback_is_not_permission_to_retry(setup, damage):
    root, store, task, _, delegation = setup
    discover(setup)
    feedback = deepcopy(previous_refusal(setup))
    if damage == "schema": feedback["accept"] = True
    elif damage == "reason": feedback["refusal"]["reason_code"] = "arbitrary_error"
    elif damage == "discovery": feedback["refusal"]["discovery_id"] = "not-pending"
    elif damage == "published": feedback["refusal"]["published"] = True
    else: feedback["refusal"]["proposed_action"] = "fabricated_action"
    with pytest.raises(ValueError):
        coordinate(root, task.task_id, delegation, previous_rejection=feedback,
                   execute=lambda *args: pytest.fail("Malformed feedback reached another provider"))
    assert len(store.decisions(task.task_id)) == 1


@pytest.mark.parametrize("damage", ["malformed", "stale", "writer"])
def test_fatal_second_proposal_is_not_retried_or_hidden_as_policy_feedback(setup, monkeypatch, damage):
    root, store, task, _, delegation = setup
    discover(setup)
    delegation["max_decisions"] = 4
    calls = []
    failure = ValueError("native publisher failed")
    if damage == "writer":
        def fail(*args, **kwargs): raise failure
        monkeypatch.setattr(TaskStore, "expand", fail)

    def execute(command, current, name, payload):
        stage = command.argv[0]
        if stage == "context": return 0, {}
        if stage == "check": return 3, {"result": "unknown"}
        assert stage == "coordinate", "Fatal proposal must never reach worker or review"
        calls.append(payload)
        if len(calls) == 1: return 0, answer(payload, "investigate")
        assert len(calls) == 2 and payload["previous_rejection"]
        response = answer(payload, "expand")
        if damage == "malformed": response["extra"] = True
        elif damage == "stale": response["snapshot"] = "f" * 64
        return 0, response

    with pytest.raises(ValueError) as raised:
        drive(root, task.task_id, configuration(task), role("worker"), iterations=1,
              coordinator_configuration=delegation, execute=execute)
    if damage == "writer": assert raised.value is failure
    assert len(calls) == 2 and len(store.decisions(task.task_id)) == 1


@pytest.mark.parametrize("proposal", ["invalid_expand", "unknown_expand", "legal_expand"])
def test_publishing_semantics_are_checked_before_restricted_phase_serial_routing(setup, proposal):
    root, store, task, _, delegation = setup
    discover(setup, "entity-" + "b" * 64 if proposal == "unknown_expand" else None)
    before = working_state(root), store.decisions(task.task_id), store.scope(task.task_id)

    def execute(*args):
        response = answer(args[-1], "expand")
        if proposal == "invalid_expand": response["editable_files"] = ["code/owner.cc"]
        return 0, response

    if proposal == "invalid_expand":
        result = coordinate(root, task.task_id, delegation, execute=execute,
                            allowed_actions=("investigate", "investigate_local", "defer"))
        assert result["action"] == "proposal_rejected" and result["reason_code"] == "expansion_exceeds_delegation"
    else:
        with pytest.raises(SerialCoordinationRequired if proposal == "legal_expand" else ValueError) as raised:
            coordinate(root, task.task_id, delegation, execute=execute,
                       allowed_actions=("investigate", "investigate_local", "defer"))
        if proposal == "unknown_expand": assert not isinstance(raised.value, SerialCoordinationRequired)
    assert (working_state(root), store.decisions(task.task_id), store.scope(task.task_id)) == before
