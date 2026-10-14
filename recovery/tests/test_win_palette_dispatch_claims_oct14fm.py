"""Native claim-only publication controls, not linked/game acceptance."""
from dataclasses import fields, replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes
from match_recovery.tasks import TaskStore

ROOT = Path(__file__).resolve().parents[2]
BASE = "203891c982dbba68a1f98f73afec58237581cf21"
TASK = "win-palette-dispatch-claims-owner-oct14fm"
RECORDED = {0x5F1070, 0x5EE9F0}


def original_bytes():
    return subprocess.check_output(["git", "show", BASE + ":src/win.cpp"], cwd=ROOT)


def check_claim_only(current):
    path = ROOT / "src/win.cpp"
    old = original_bytes()
    before = read_text(old.decode(), path)
    after = read_text(current.decode(), path)
    selected = [replace(row, byte_exact=True) for row in before
                if row.address in RECORDED]
    assert len(selected) == 2
    assert all(not row.byte_exact and not row.semantic for row in before
               if row.address in RECORDED)
    assert write_bytes(old, selected) == current
    assert [row.address for row in before] == [row.address for row in after]
    for previous, recorded in zip(before, after):
        expected = replace(previous, byte_exact=True) if previous.address in RECORDED else previous
        for field in fields(previous):
            if field.name == "region":
                # Exact whole-file writer projection above covers FILE regions.
                continue
            assert getattr(expected, field.name) == getattr(recorded, field.name)


def test_native_publication_changes_only_two_flags():
    check_claim_only((ROOT / "src/win.cpp").read_bytes())
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    assert {subject.address for subject in task.subjects} == RECORDED


def test_source_behavior_damage_is_not_hidden_by_claim_projection():
    current = (ROOT / "src/win.cpp").read_bytes()
    assert b"if (BufferDirectDraw != 0)" in current
    damaged = current.replace(b"if (BufferDirectDraw != 0)", b"if (BufferDirectDraw == 0)", 1)
    with pytest.raises(AssertionError):
        check_claim_only(damaged)


def test_withdrawing_a_claim_is_rejected():
    with pytest.raises(AssertionError):
        check_claim_only(original_bytes())
