"""Share synthetic binary fixtures without shipping project tests in the library."""

from pathlib import Path
import json
import sys
import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "packages/match-recovery/tests"))
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "packages/match-recovery/tests/source"))


@pytest.fixture
def source_configuration(tmp_path):
    """Explicit source-provider inputs for synthetic application fixtures."""
    configuration = tmp_path / "recovery/project.json"
    configuration.parent.mkdir(exist_ok=True)
    configuration.write_text(json.dumps({"schema_version": 1,
        "adapter": {"argv": ["synthetic-project-adapter"], "timeout": 10}, "source": {
        "image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
        "seed_roots": ["src/recovered", "src/unrecovered"], "symbol_format": "msvc"}}))


@pytest.fixture(autouse=True)
def project_command_unit_boundary(monkeypatch):
    # Unit tests replace only transport so compiler/parser monkeypatches stay in
    # process. Installed-package tests exercise the real subprocess protocol.
    from recovery.opensmacx.operations import execute
    monkeypatch.setattr("match_recovery.project_commands._invoke_adapter",
                        lambda root, configuration, arguments: execute(root, arguments))
