"""Check local provider argument parsing without a network call or model run."""

import json
import os
from pathlib import Path
import shutil
import subprocess

import pytest


@pytest.mark.parametrize("role", ["planner", "worker", "coordinator", "reviewer"])
def test_configured_provider_accepts_arguments_before_loading_schema(tmp_path, role):
    settings = json.loads((Path(__file__).resolve().parents[1] / "agents.json").read_text())
    command = settings[role]["command"]["argv"]
    if shutil.which(command[0]) is None:
        pytest.skip("configured provider CLI is not installed")
    schema = tmp_path / "intentionally-absent-schema.json"
    values = {"{schema}": str(schema), "{response}": str(tmp_path / "response.json"),
              "{env:VC6_PREFIX}": str(tmp_path)}
    argv = [values.get(word, word) for word in command]
    assert not schema.exists()
    # Schema loading fails before any request: catches mutually exclusive CLI
    # options that --help accepts, without provider credentials or network use.
    result = subprocess.run(argv, input="Configuration validation only; do not run tools.",
                            capture_output=True, text=True, timeout=15)
    assert result.returncode != 0
    assert "Failed to read output schema file " + str(schema) in result.stderr, result.stderr
    assert not (tmp_path / "response.json").exists()


@pytest.mark.skipif(os.environ.get("OPENSMACX_LIVE_PROVIDER_SCHEMA") != "1",
                    reason="explicit live-provider schema preflight opt-in required")
@pytest.mark.parametrize("role", ["planner", "worker", "coordinator", "reviewer"])
def test_live_provider_accepts_native_response_schema_in_isolated_empty_checkout(tmp_path, role):
    """Provider transport only: no game source, compiler, decisions or acceptance."""
    from match_recovery.agent_protocol import response_schema

    settings = json.loads((Path(__file__).resolve().parents[1] / "agents.json").read_text())
    command = settings[role]["command"]["argv"]
    if shutil.which(command[0]) is None:
        pytest.fail("configured provider CLI is not installed")
    subprocess.run(["git", "init", "-q", str(tmp_path)], check=True)
    resource = tmp_path / "unused-compiler-resource"
    resource.mkdir()
    schema, response = tmp_path / "schema.json", tmp_path / "response.json"
    schema.write_text(json.dumps(response_schema(role)))
    evidence = ["Schema transport preflight only; no recovery decision."]
    common = {"snapshot": "a" * 64, "evidence": evidence}
    expected = {
        "planner": {**common, "action": "defer", "task_id": "", "group": "", "intent": "", "editable_files": []},
        "worker": {"discoveries": []},
        "coordinator": {**common, "action": "defer", "discovery_id": "transport-preflight",
                        "subjects": [], "editable_files": [], "binding": None, "route": None},
        "reviewer": {**common, "decision": "request_changes", "feedback": evidence[0],
                     "supersessions": [], "retractions": []},
    }[role]
    values = {"{schema}": str(schema), "{response}": str(response), "{env:VC6_PREFIX}": str(resource)}
    argv = [values.get(word, word) for word in command]
    prompt = ("Transport-only preflight in an empty temporary checkout. Do not use tools, inspect files, "
              "edit source or make recovery decisions. Return exactly this JSON object: " + json.dumps(expected))
    result = subprocess.run(argv, cwd=tmp_path, input=prompt, text=True, capture_output=True, timeout=180)
    assert result.returncode == 0, result.stderr[-12000:]
    assert json.loads(response.read_text()) == expected
