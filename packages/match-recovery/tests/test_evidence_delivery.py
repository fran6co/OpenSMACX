"""Portable controls for worker on-demand evidence transport."""

import json
import hashlib
from pathlib import Path
import sys

import pytest

from match_recovery.agent_command import run_agent
from match_recovery.evidence_delivery import (
    EVIDENCE_THRESHOLD,
    REFERENCE_KEY,
    REQUEST_THRESHOLD,
    compact_worker_request,
)
from match_recovery.model import canonical
from test_agent_command import request
from test_driver import project


def _resolve(document, pointer):
    if pointer == "":
        return document
    assert pointer.startswith("/")
    value = document
    for raw in pointer[1:].split("/"):
        for index, char in enumerate(raw):
            if char == "~":
                assert index + 1 < len(raw) and raw[index + 1] in "01"
        token = raw.replace("~1", "/").replace("~0", "~")
        if isinstance(value, list):
            assert token.isdigit() and (token == "0" or not token.startswith("0"))
            value = value[int(token)]
        else:
            value = value[token]
    return value


def _configuration(root, script, *, evidence_delivery=None, include_evidence_delivery=False):
    command = {"argv": [sys.executable, "-c", script, "{request}", "{schema}", "{response}", "{role}"],
               "timeout": 5}
    role = {"command": command, "instructions": [], "response": "file"}
    if evidence_delivery is not None or include_evidence_delivery:
        role["evidence_delivery"] = evidence_delivery
    (root / "recovery/agents.json").write_text(json.dumps({
        "schema_version": 1, "artifacts": "build/recovery/agents",
        "worker": role, "reviewer": role.copy(),
    }))
    return Path("recovery/agents.json")


def _large_request(task):
    value = request(task)
    value["context"] = {
        "impact": {"status": "fail", "path/with~escape": "i" * (EVIDENCE_THRESHOLD + 20)},
        "restoration": [{"result": "unknown", "detail": "r" * 5000} for _ in range(3)],
    }
    value["check"] = {
        "result": "fail",
        "limitations": "visible limitation",
        "candidate": {
            "result": "fail",
            "checks": [{"result": "pass", "detail": "c" * 5000} for _ in range(30)]
                      + [{"result": "fail", "detail": "failing tail"}],
        },
    }
    assert len(canonical(value)) > REQUEST_THRESHOLD
    return value


def test_pointer_roundtrip_threshold_and_escaped_collision_are_explicit():
    request_value = {"context": {"impact": {
        "path/with~escape": "x" * (EVIDENCE_THRESHOLD + 1),
        "existing": {REFERENCE_KEY: {"schema": "user-data", "pointer": "not-routing"}},
        "small": "x",
    }}}
    manifest = []
    compact = compact_worker_request(request_value, "/tmp/request.json", "a" * 64, manifest)
    reference = compact["context"]["impact"]["path/with~escape"]
    assert reference[REFERENCE_KEY]["json_pointer"] == "/context/impact/path~1with~0escape"
    assert _resolve(request_value, reference[REFERENCE_KEY]["json_pointer"]) == "x" * (EVIDENCE_THRESHOLD + 1)
    assert manifest == [{"pointer": "/context/impact/path~1with~0escape",
                         "request_path": "/tmp/request.json", "request_sha256": "a" * 64,
                         "value_type": "string", "item_count": 1,
                         "byte_count": len(canonical("x" * (EVIDENCE_THRESHOLD + 1)))}]
    assert compact["context"]["impact"]["existing"][REFERENCE_KEY]["schema"] == "user-data"
    assert compact["context"]["impact"]["small"] == "x"


def test_large_request_routes_only_diagnostics_and_retains_critical_fields(project):
    root, task = project
    full = _large_request(task)
    compact = compact_worker_request(full, "/abs/request.json", "b" * 64)
    for key in ("task", "editable_files", "round", "instructions", "review_feedback"):
        assert compact[key] == full[key]
    assert compact["check"]["result"] == "fail"
    assert compact["check"]["candidate"]["result"] == "fail"
    assert compact["check"]["limitations"] == "visible limitation"
    checks_ref = compact["check"]["candidate"]["checks"][REFERENCE_KEY]
    assert checks_ref["item_count"] == len(full["check"]["candidate"]["checks"])
    assert _resolve(full, checks_ref["json_pointer"])[-1]["result"] == "fail"
    assert "passing or empty" in checks_ref["omitted_evidence_is_not"]


def test_default_inline_worker_prompt_is_unchanged_and_reviewer_on_demand_refuses(project):
    root, task = project
    full = _large_request(task)
    script = ("from pathlib import Path; import json,sys; "
              "Path('src/f.cpp').write_text(sys.stdin.read()); "
              "Path(sys.argv[3]).write_text(json.dumps({'discoveries':[]}))")
    config = _configuration(root, script)
    assert run_agent(root, "worker", config, full)[0] == 0
    prompt = (root / "src/f.cpp").read_text()
    assert canonical(full).decode() in prompt
    assert REFERENCE_KEY not in prompt

    config = _configuration(root, script, evidence_delivery="on_demand")
    reviewer_request = request(task, "reviewer")
    reviewer_request["context"] = full["context"]
    reviewer_request["check"] = full["check"]
    reviewer_request["snapshot"] = "a" * 64
    with pytest.raises(ValueError, match="only for workers"):
        run_agent(root, "reviewer", config, reviewer_request)


@pytest.mark.parametrize("mode", [True, [], {}, None, "unsupported"])
def test_invalid_evidence_delivery_modes_refuse_before_provider(project, mode):
    root, task = project
    config = _configuration(root, "raise AssertionError('must not launch')", evidence_delivery=mode,
                            include_evidence_delivery=True)
    with pytest.raises(ValueError, match="evidence_delivery"):
        run_agent(root, "worker", config, request(task))


def test_on_demand_subprocess_reads_full_immutable_request_and_references_failing_tail(project):
    root, task = project
    full = _large_request(task)
    script = r'''import json, sys
from pathlib import Path
request_path, schema_path, response_path, role = sys.argv[1:]
raw = Path(request_path).read_bytes()
request = json.loads(raw)
assert len(raw) > 128 * 1024
assert request["check"]["candidate"]["checks"][-1]["result"] == "fail"
assert request["context"]["impact"]["path/with~escape"].startswith("i")
Path("src/f.cpp").write_text(sys.stdin.read())
Path(response_path).write_text(json.dumps({"discoveries": []}))
'''
    config = _configuration(root, script, evidence_delivery="on_demand")
    code, result = run_agent(root, "worker", config, full)
    assert code == 0 and result == {"discoveries": []}
    prompt = (root / "src/f.cpp").read_text()
    request_path = next(line for line in prompt.splitlines() if "authoritative full request is at" in line)
    request_path = request_path.split(" at ", 1)[1].split(" and has SHA-256", 1)[0]
    assert request_path.startswith("/") and request_path.endswith("request.json")
    assert "SHA-256 " + hashlib.sha256(canonical(full)).hexdigest() in prompt
    assert "~1" in prompt and "~0" in prompt and '"detail":"failing tail"' not in prompt
    assert REFERENCE_KEY in prompt


def test_provider_request_mutation_is_refused_even_when_prompt_is_routed(project):
    root, task = project
    full = _large_request(task)
    script = ("from pathlib import Path; import json,sys; "
              "Path(sys.argv[1]).write_text('{}'); "
              "Path(sys.argv[3]).write_text(json.dumps({'discoveries':[]}))")
    config = _configuration(root, script, evidence_delivery="on_demand")
    with pytest.raises(ValueError, match="modified its request"):
        run_agent(root, "worker", config, full)


def test_large_authorizations_discoveries_and_feedback_never_route(project):
    _, task = project
    full = _large_request(task)
    large = [{"evidence": "e" * EVIDENCE_THRESHOLD}] * 20
    for key in ("decision_state", "pending_discoveries", "discovery_investigations",
                "protected_obligations", "instructions", "source_hints"):
        full["context"][key] = large
    for key in ("review_feedback", "acceptance_feedback", "checkpoint_feedback"):
        full[key] = {"evidence": large}
    before = canonical(full)
    manifest = []
    brief = compact_worker_request(full, "/tmp/request.json", hashlib.sha256(before).hexdigest(), manifest)
    assert canonical(full) == before
    assert all(brief[key] == value for key, value in full.items() if key not in ("context", "check"))
    assert all(brief["context"][key] == value for key, value in full["context"].items()
               if key not in ("impact", "restoration"))
    assert manifest
    assert len(canonical(brief)) > REQUEST_THRESHOLD  # Never a cap on authorization.


def test_reference_shaped_data_cannot_impersonate_a_generated_location():
    spoof = {REFERENCE_KEY: {"schema": "match-recovery/evidence-reference/v1",
                            "json_pointer": "/check/data", "request_path": "/tmp/request.json"}}
    full = {"check": {"data": "x" * EVIDENCE_THRESHOLD, "spoof": spoof}}
    manifest = []
    brief = compact_worker_request(full, "/tmp/request.json", "a" * 64, manifest)
    assert brief["check"]["spoof"] == spoof
    assert [entry["pointer"] for entry in manifest] == ["/check/data"]
    assert _resolve(brief, manifest[0]["pointer"]) != spoof


@pytest.mark.parametrize("delta", [0, 1])
def test_diagnostic_threshold_uses_canonical_utf8_bytes(delta):
    # JSON quotes and final newline count, not just string character count.
    text = "\u00e9" * 4094 + "x" * (1 + delta)
    assert len(canonical(text)) == EVIDENCE_THRESHOLD + delta
    full = {"check": {"message": text}}
    manifest = []
    brief = compact_worker_request(full, "/tmp/request.json", "b" * 64, manifest)
    if delta:
        assert manifest[0]["byte_count"] == EVIDENCE_THRESHOLD + 1
        assert _resolve(full, manifest[0]["pointer"]) == text
    else:
        assert brief == full and manifest == []


@pytest.mark.parametrize("delta", [0, 1])
def test_request_threshold_leaves_small_on_demand_requests_inline(project, delta):
    root, task = project
    full = request(task)
    full["context"] = {"impact": {"large": ""}}
    full["context"]["impact"]["large"] = "x" * (REQUEST_THRESHOLD - len(canonical(full)) + delta)
    assert len(canonical(full)) == REQUEST_THRESHOLD + delta
    script = ("from pathlib import Path; import json,sys; "
              "Path('src/f.cpp').write_text(sys.stdin.read()); "
              "Path(sys.argv[3]).write_text(json.dumps({'discoveries':[]}))")
    config = _configuration(root, script, evidence_delivery="on_demand")
    assert run_agent(root, "worker", config, full) == (0, {"discoveries": []})
    prompt = (root / "src/f.cpp").read_text()
    if delta:
        assert canonical(full).decode() not in prompt
        assert "Only nodes AT the JSON Pointer locations" in prompt
    else:
        assert canonical(full).decode() in prompt
        assert REFERENCE_KEY not in prompt


def test_large_reviewer_request_remains_inline(project):
    root, task = project
    full = request(task, "reviewer")
    full["check"] = _large_request(task)["check"]
    script = r'''import json, sys
from pathlib import Path
request = json.loads(Path(sys.argv[1]).read_text())
Path('build/reviewer-prompt.txt').write_text(sys.stdin.read())
Path(sys.argv[3]).write_text(json.dumps({
    'snapshot': request['snapshot'], 'decision': 'approve',
    'evidence': ['test-owned independent review'], 'feedback': 'Synthetic review',
    'supersessions': [], 'retractions': []}))
'''
    assert len(canonical(full)) > REQUEST_THRESHOLD
    config = _configuration(root, script, evidence_delivery="inline")
    assert run_agent(root, "reviewer", config, full)[0] == 0
    prompt = (root / "build/reviewer-prompt.txt").read_text()
    assert canonical(full).decode() in prompt and REFERENCE_KEY not in prompt
