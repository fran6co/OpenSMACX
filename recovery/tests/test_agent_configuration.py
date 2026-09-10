"""Check local provider argument parsing without a network call or model run."""

import json
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
