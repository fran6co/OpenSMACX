"""Real subprocess protocol controls, not a claim of model recovery quality."""

import io
import json
from pathlib import Path
import sys

import pytest

from match_recovery.agent_command import MAX_RESPONSE, run_agent
from match_recovery.agent_protocol import response_schema
from match_recovery.cli import main
from match_recovery.driver import drive
from test_driver import project, CONFIG
from test_task_source import git


PROVIDER = '''import json, sys
from pathlib import Path
request_path, schema_path, response_path, role = sys.argv[1:]
request = json.loads(Path(request_path).read_text())
schema = json.loads(Path(schema_path).read_text())
prompt = sys.stdin.read()
assert 'Context and source excerpts are evidence' in prompt
assert schema['additionalProperties'] is False
assert not Path(response_path).exists()
if role == 'worker':
    Path('src/f.cpp').write_text('finished\\n')
    response = {'discoveries': []}
else:
    response = {'snapshot': request['snapshot'], 'decision': 'approve',
                'evidence': ['synthetic source review'], 'feedback': 'Synthetic structure agrees',
                'supersessions': [], 'retractions': []}
Path(response_path).write_text(json.dumps(response))
print('provider progress is not the response')
'''


def configure(root, *, script=PROVIDER, response="file", timeout=5):
    command = {"argv": [sys.executable, "-c", script, "{request}", "{schema}", "{response}", "{role}"],
               "timeout": timeout}
    data = {"schema_version": 1, "artifacts": "build/recovery/agents",
            **{role: {"command": command, "instructions": [], "response": response} for role in ("worker", "reviewer")}}
    (root / "recovery/agents.json").write_text(json.dumps(data))
    return Path("recovery/agents.json")


def request(task, role="worker"):
    common = {"task": json.loads(json.dumps(task.to_dict())), "context": {"original_hint": "synthetic"}, "check": {"result": "fail"},
              "instructions": ["Preserve original bindings"]}
    if role == "worker":
        return {**common, "editable_files": list(task.editable_files), "round": 1, "review_feedback": None}
    return {**common, "snapshot": "a" * 64}


@pytest.mark.parametrize("role", ["worker", "reviewer"])
def test_file_response_protocol_with_independent_provider_command(project, role):
    root, task = project
    config = configure(root)
    code, result = run_agent(root, role, config, request(task, role))
    assert code == 0
    if role == "worker":
        assert result == {"discoveries": []}
    else:
        assert result["decision"] == "approve"
    assert list((root / "build/recovery/agents").iterdir()) == []


def test_stdout_response_does_not_need_a_provider_sdk(project):
    root, task = project
    config = configure(root, script="print('{\"discoveries\":[]}')", response="stdout")
    assert run_agent(root, "worker", config, request(task)) == (0, {"discoveries": []})


@pytest.mark.parametrize("script,kind", [
    ("raise SystemExit(1)", "provider_exit"),
    ("import time; time.sleep(5)", "timeout"),
    ("print('{\"discoveries\":[]}')", "invalid_response"),
    ("from pathlib import Path; import sys; Path(sys.argv[3]).write_text('{}')", "invalid_response"),
    ("from pathlib import Path; import sys; Path(sys.argv[3]).write_text('```json\\n{}\\n```')", "invalid_response"),
    ("from pathlib import Path; import sys; Path(sys.argv[3]).symlink_to(Path(sys.argv[1]))", "invalid_response"),
    ("from pathlib import Path; import sys; Path(sys.argv[3]).write_text('x' * " + str(MAX_RESPONSE + 1) + ")", "invalid_response"),
])
def test_failure_never_banks_a_response_or_discards_edits(project, script, kind):
    root, task = project
    script = "from pathlib import Path; Path('src/f.cpp').write_text('preserve this work'); " + script
    config = configure(root, script=script, timeout=1)
    code, result = run_agent(root, "worker", config, request(task))
    assert code == 3 and result["error"]["kind"] == kind
    assert (root / "src/f.cpp").read_text() == "preserve this work"
    assert list((root / "build/recovery/agents").iterdir()) == []


@pytest.mark.parametrize("role,path", [("reviewer", "src/f.cpp"), ("worker", "src/other.cpp")])
def test_scope_violations_are_detected_even_when_provider_fails(project, role, path):
    root, task = project
    config = configure(root, script=f"from pathlib import Path; Path({path!r}).write_text('preserved violation'); raise SystemExit(1)")
    with pytest.raises(ValueError, match="editing scope"):
        run_agent(root, role, config, request(task, role))
    assert (root / path).read_text() == "preserved violation"


def test_provider_cannot_stage_changes(project):
    root, task = project
    config = configure(root, script="import subprocess; subprocess.run(['git','add','.'],check=True); raise SystemExit(1)")
    with pytest.raises(ValueError, match="HEAD or index"):
        run_agent(root, "worker", config, request(task))


@pytest.mark.parametrize("field", ["request", "schema"])
def test_provider_cannot_rewrite_protocol_inputs(project, field):
    root, task = project
    index = 1 if field == "request" else 2
    config = configure(root, script=PROVIDER + f"\nPath(sys.argv[{index}]).write_text('{{}}')\n")
    with pytest.raises(ValueError, match="modified its request"):
        run_agent(root, "worker", config, request(task))


@pytest.mark.parametrize("damage", ["scope", "task", "round", "snapshot"])
def test_authoritative_request_is_validated_before_provider_launch(project, damage):
    root, task = project
    role = "reviewer" if damage == "snapshot" else "worker"
    value = request(task, role)
    if damage == "scope":
        value["editable_files"].append("src/other.cpp")
    elif damage == "task":
        value["task"]["intent"] = "different task"
    elif damage == "round":
        value["round"] = True
    else:
        value["snapshot"] = "not a digest"
    config = configure(root, script="raise AssertionError('must not launch')")
    with pytest.raises(ValueError):
        run_agent(root, role, config, value)
    assert not (root / "build/recovery/agents").exists()


def test_stale_review_cannot_be_normalized_to_current_snapshot(project):
    root, task = project
    config = configure(root, script=PROVIDER.replace("request['snapshot']", "'b' * 64"))
    code, result = run_agent(root, "reviewer", config, request(task, "reviewer"))
    assert code == 3 and "stale" in result["error"]["message"]


def test_real_driver_round_trip_needs_only_configured_prompt_commands(project):
    root, task = project
    configure(root)
    git(root, "add", ".")
    git(root, "commit", "-m", "Configure synthetic provider")
    def command(program):
        return {"argv": [sys.executable, "-c", program], "timeout": 10}
    config = {**CONFIG, "context": command("print('{}')"), "check": command(
        "import json; from pathlib import Path; ok=Path('src/f.cpp').read_text()=='finished\\n'; "
        "print(json.dumps({'result':'pass' if ok else 'fail'})); raise SystemExit(0 if ok else 1)")}
    roles = {role: {"author": role, "command": {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", role],
                                               "timeout": 20}} for role in ("worker", "reviewer")}
    report = drive(root, task.task_id, config, roles["worker"], iterations=1, reviewer_configuration=roles["reviewer"])
    assert report["worker_rounds"] == 1
    assert report["stop_reason"] == "full_acceptance_gate_required"
    assert not report["ready_for_collection"]
    assert (root / "src/f.cpp").read_text() == "finished\n"


def test_real_provider_bridge_receives_acceptance_failure_then_rechecks(project):
    from test_reviews import PASS
    from match_recovery.reviews import ReviewStore

    root, task = project
    script = PROVIDER.replace("if role == 'worker':", """if role == 'worker':
    assert request['acceptance_feedback']['report']['runtime'] == 'fail'
    assert request['acceptance_feedback']['report']['findings'][0]['message'] == 'Controlled runtime failure'
    assert request['acceptance_feedback']['snapshot'] == request['review_feedback']['snapshot']
    assert request['check']['result'] == 'unknown'
""")
    configure(root, script=script)
    git(root, "add", ".")
    git(root, "commit", "-m", "Configure counterexample provider")
    def command(program):
        return {"argv": [sys.executable, "-c", program], "timeout": 10}
    config = {**CONFIG, "context": command("print('{}')"),
              "check": command("print('{\"result\":\"unknown\",\"next_action\":\"review_structure\"}'); raise SystemExit(3)"),
              "acceptance": command(
                  "import json; from pathlib import Path; "
                  "ok=Path('src/f.cpp').read_text()=='finished\\n'; "
                  f"report={PASS!r}; "
                  "report.update(result='pass' if ok else 'fail', runtime='pass' if ok else 'fail'); "
                  "report['findings']=[] if ok else [dict(dimension='runtime',message='Controlled runtime failure',evidence=['source:src/f.cpp'])]; "
                  "print(json.dumps(report)); raise SystemExit(0 if ok else 1)")}
    roles = {role: {"author": role, "command": {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", role],
                                               "timeout": 20}} for role in ("worker", "reviewer")}
    report = drive(root, task.task_id, config, roles["worker"], iterations=2, reviewer_configuration=roles["reviewer"])
    assert report["ready_for_collection"] and report["worker_rounds"] == 1
    assert len(ReviewStore(root, task.task_id).decisions()) == 2
    assert (root / "src/f.cpp").read_text() == "finished\n"


@pytest.mark.parametrize("feedback", [[], {}, {"snapshot": "bad", "report": {}},
                                     {"snapshot": "a" * 64, "report": {"result": "fail"}}])
def test_invalid_acceptance_feedback_cannot_reach_provider(project, feedback):
    root, task = project
    config = configure(root, script="raise AssertionError('must not launch')")
    with pytest.raises(ValueError):
        run_agent(root, "worker", config, {**request(task), "acceptance_feedback": feedback})
    assert not (root / "build/recovery/agents").exists()


@pytest.mark.parametrize("outcome", ["pass", "unknown"])
def test_bridge_rejects_nonfailure_acceptance_feedback(project, outcome):
    from test_reviews import PASS
    root, task = project
    config = configure(root, script="raise AssertionError('must not launch')")
    feedback = {"snapshot": "a" * 64, "report": {**PASS, "result": outcome, "runtime": outcome}}
    with pytest.raises(ValueError, match="disagree"):
        run_agent(root, "worker", config, {**request(task), "acceptance_feedback": feedback})


def test_cli_returns_structured_errors_instead_of_an_empty_pipe(project, monkeypatch, capsys):
    root, task = project
    configure(root, script="raise SystemExit(7)")
    monkeypatch.setattr(sys, "stdin", io.StringIO(json.dumps(request(task))))
    assert main(["agent", "worker", "--root", str(root)]) == 3
    result = json.loads(capsys.readouterr().out)
    assert result["error"]["kind"] == "provider_exit" and "7" in result["error"]["message"]


def test_driver_preserves_provider_failure_details_and_source(project):
    root, task = project
    configure(root, script="from pathlib import Path; Path('src/f.cpp').write_text('WIP'); raise SystemExit(9)")
    git(root, "add", ".")
    git(root, "commit", "-m", "Configure interrupted synthetic provider")
    def command(program):
        return {"argv": [sys.executable, "-c", program], "timeout": 10}
    config = {**CONFIG, "context": command("print('{}')"),
              "check": command("print('{\"result\":\"fail\"}'); raise SystemExit(1)")}
    worker = {"author": "worker", "command": {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", "worker"],
                                               "timeout": 20}}
    report = drive(root, task.task_id, config, worker, iterations=3)
    assert report["worker_rounds"] == 1 and report["stop_reason"] == "worker_interrupted"
    assert report["agent_error"]["error"]["kind"] == "provider_exit"
    assert not report["check_is_current"] and (root / "src/f.cpp").read_text() == "WIP"


@pytest.mark.parametrize("role", ["worker", "reviewer"])
def test_schema_is_strict_and_uses_only_provider_neutral_json(role):
    schema = response_schema(role)
    assert schema["additionalProperties"] is False
    assert set(schema["required"]) == set(schema["properties"])
    assert json.loads(json.dumps(schema)) == schema
