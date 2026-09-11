"""Large observations reach read-only coordinators without dropping authority."""

from copy import deepcopy
import json
from pathlib import Path
import sys

import pytest

from match_recovery.agent_command import run_agent
from match_recovery.coordination import coordinate
from match_recovery.evidence_delivery import compact_coordinator_request, REFERENCE_KEY
from match_recovery.model import canonical
from test_coordination import answer, discover, setup
from test_coordination_diagnostics import configuration
from test_evidence_delivery import _resolve
from test_source_configuration import configured
from test_task_source import project


def test_compaction_retains_all_authority_and_diagnostic_envelopes():
    large = [{"detail": "observation" * 1000}] * 160
    authority = {name: deepcopy(large) for name in (
        "task", "delegation", "pending_discoveries", "subjects", "editable_files",
        "instructions", "mapped_records", "decision_facts", "previous_rejection",
        "routing", "route_choices", "allowed_actions", "investigating")}
    context = {name: deepcopy(large) for name in (
        "impact", "restoration", "decision_state", "incoming_routes", "source_hints",
        "protected_obligations", "instructions", "subjects")}
    full = {**authority, "diagnostics": {
        "epoch": "a" * 64, "verification": "diagnostic_only", "ready_for_collection": False,
        "context": {"exit_code": 0, "report": context},
        "check": {"exit_code": 3, "report": {"result": "unknown", "detail/~": large}}}}
    before, manifest = canonical(full), []
    brief = compact_coordinator_request(full, "/tmp/request.json", "b" * 64, manifest)
    assert canonical(full) == before
    assert {key: brief[key] for key in authority} == authority
    observation = brief["diagnostics"]
    assert observation["epoch"] == "a" * 64
    assert observation["verification"] == "diagnostic_only" and not observation["ready_for_collection"]
    assert observation["context"]["exit_code"] == 0 and observation["check"]["exit_code"] == 3
    assert observation["check"]["report"]["result"] == "unknown"
    for name in context.keys() - {"impact", "restoration"}:
        assert observation["context"]["report"][name] == context[name]
    assert {row["pointer"] for row in manifest} == {
        "/diagnostics/context/report/impact", "/diagnostics/context/report/restoration",
        "/diagnostics/check/report/detail~1~0"}
    for row in manifest:
        assert _resolve(full, row["pointer"]) == large
        assert _resolve(brief, row["pointer"])[REFERENCE_KEY]["request_sha256"] == "b" * 64


@pytest.mark.parametrize("observation", [None, {"context": {"exit_code": 3, "report": {"error": "unavailable"}}, "check": None}])
def test_absent_or_failed_context_never_gains_a_check(observation):
    full = {} if observation is None else {"diagnostics": observation}
    manifest = []
    assert compact_coordinator_request(full, "/tmp/request.json", "a" * 64, manifest) == full
    assert manifest == []


@pytest.mark.parametrize("damage", [None, "request", "schema", "source", "response"])
def test_large_native_coordinator_request_reads_failing_tail_without_publication(setup, damage):
    root, store, task, _, delegation = setup
    discover(setup, task.subjects[0].entity_id)
    script = '''import hashlib, json, sys
from pathlib import Path
raw = Path(sys.argv[1]).read_bytes()
request = json.loads(raw)
prompt = sys.stdin.read()
assert len(raw) > 5_700_000 and len(prompt) < 128 * 1024
assert hashlib.sha256(raw).hexdigest() in prompt
assert request['diagnostics']['check']['report']['findings'][-1]['result'] == 'fail'
assert 'before editing or deciding' in prompt
assert 'Remain read-only' in prompt
assert request['pending_discoveries'][0]['discovery_id'] == 'layout'
assert '"ready_for_collection":false' in prompt
response = {'snapshot': request['snapshot'], 'action': 'defer', 'discovery_id': 'layout',
            'subjects': [], 'editable_files': [], 'evidence': ['Inspected the failing tail'],
            'binding': None, 'route': None}
'''
    if damage in ("request", "schema"):
        script += f"Path(sys.argv[{1 if damage == 'request' else 2}]).write_text('{{}}')\n"
    elif damage == "source":
        script += "Path('code/owner.cc').write_text('preserved unauthorized change')\n"
    elif damage == "response":
        script += "response['snapshot'] = '0' * 64\n"
    script += "Path(sys.argv[3]).write_text(json.dumps(response))\n"
    role = {"command": {"argv": [sys.executable, "-c", script, "{request}", "{schema}", "{response}"], "timeout": 10},
            "instructions": [], "response": "file", "evidence_delivery": "on_demand"}
    config = Path("recovery/agents.json")
    (root / config).write_text(json.dumps({"schema_version": 1, "artifacts": "build/recovery/agents",
                                         "worker": role, "reviewer": role, "coordinator": role}))
    original = store.decisions(task.task_id), store.scope(task.task_id)
    captured = []

    def execute(command, current, name, payload):
        selected = command.argv[0]
        if selected == "context":
            return 0, {"impact": {"entities": ["source hint" * 1000] * 300}}
        if selected == "check":
            return 1, {"result": "fail", "findings": [{"detail": "observation" * 1000}] * 300 + [{"result": "fail"}]}
        assert selected == "coordinate"
        # Exercise the actual request validator and subprocess bridge, not a
        # compactor-only reconstruction or a saved provider receipt.
        code, response = run_agent(root, "coordinator", config, json.loads(canonical(payload)))
        captured.append((code, response))
        return code, response

    if damage in ("request", "schema", "source"):
        with pytest.raises(ValueError, match="modified its request|editing scope|read-only coordinator edited"):
            coordinate(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
    else:
        report = coordinate(root, task.task_id, delegation, diagnostic_configuration=configuration(setup), execute=execute)
        assert report["action"] == ("unavailable" if damage else "defer")
        assert not report["published"] and not report["ready_for_collection"]
        assert captured[0][0] == (3 if damage else 0)
    assert (store.decisions(task.task_id), store.scope(task.task_id)) == original
    if damage == "source":
        assert (root / "code/owner.cc").read_text() == "preserved unauthorized change"
