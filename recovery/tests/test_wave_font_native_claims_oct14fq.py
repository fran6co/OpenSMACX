"""Only the three authorized native claim promotions may alter these files."""

from dataclasses import fields, replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes
from match_recovery.tasks import TaskStore


ROOT = Path(__file__).resolve().parents[2]
BASE = "cfe9967126d59604c39bb76475662989e77f4d1b"
TASK = "wave-font-native-claims-owner-oct14fq"
TARGETS = {
    ("src/wave.cpp", 0x004C70A0),
    ("src/wave.cpp", 0x004C6C20),
    ("src/main.cpp", 0x0045F940),
}


def original_bytes(relative):
    return subprocess.check_output(["git", "show", f"{BASE}:{relative}"], cwd=ROOT)


def check_claim_only(current_files):
    for relative in ("src/wave.cpp", "src/main.cpp"):
        path = ROOT / relative
        original = original_bytes(relative)
        before = read_text(original.decode("utf-8"), path)
        current = current_files.get(relative, path.read_bytes())
        after = read_text(current.decode("utf-8"), path)
        selected = [replace(row, byte_exact=True, semantic=False) for row in before
                    if (relative, row.address) in TARGETS]
        assert len(selected) == sum(item[0] == relative for item in TARGETS)
        assert all(not row.byte_exact and not row.semantic for row in before
                   if (relative, row.address) in TARGETS)
        assert all(row.image_spans and row.image_spans[0][0] == row.address for row in selected)
        assert write_bytes(original, selected) == current
        assert [row.address for row in before] == [row.address for row in after]
        for previous, recorded in zip(before, after):
            expected = (replace(previous, byte_exact=True, semantic=False)
                        if (relative, previous.address) in TARGETS else previous)
            for field in fields(previous):
                if field.name == "region":
                    continue  # Whole-file writer projection above covers FILE regions.
                assert getattr(recorded, field.name) == getattr(expected, field.name)


def test_native_publication_changes_only_three_claim_flags():
    check_claim_only({})
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    assert {subject.address for subject in task.subjects} == {address for _, address in TARGETS}
    assert all(subject.address_space == "pe:va"
               and subject.image_sha256 == "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
               for subject in task.subjects)


def test_withdrawing_a_claim_is_rejected():
    withdrawn = {relative: original_bytes(relative)
                 for relative, _address in TARGETS}
    with pytest.raises(AssertionError):
        check_claim_only(withdrawn)


def test_unrelated_source_edit_is_rejected():
    relative = "src/wave.cpp"
    current = (ROOT / relative).read_bytes()
    damaged = current.replace(b"Wave::set_reverb_mix", b"Wave::set_reverb_miX", 1)
    assert damaged != current
    with pytest.raises(AssertionError):
        check_claim_only({relative: damaged})
