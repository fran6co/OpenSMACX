"""Share synthetic binary fixtures without shipping project tests in the library."""

from pathlib import Path
import sys
import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "packages/match-recovery/tests"))


@pytest.fixture(autouse=True)
def project_command_unit_boundary(monkeypatch):
    # Unit tests replace only transport so compiler/parser monkeypatches stay in
    # process. Installed-package tests exercise the real subprocess protocol.
    from recovery.opensmacx.operations import execute
    monkeypatch.setattr("match_recovery.project_commands.invoke_project", execute)
