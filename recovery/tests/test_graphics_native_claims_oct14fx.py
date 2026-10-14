"""Only the five authorized Buffer/Caviar native claim flags may change."""

from dataclasses import fields, replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes
from match_recovery.tasks import TaskStore


ROOT = Path(__file__).resolve().parents[2]
BASE = "67421740d9a95637145b928ed432709fb7448432"
TASK = "graphics-native-claims-owner-oct14fx"
IMAGE = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
TARGETS = {
    ("src/buffer.cpp", 0x005DAB90),
    ("src/caviar.cpp", 0x00618E10),
    ("src/caviar.cpp", 0x00618E90),
    ("src/caviar.cpp", 0x00618560),
    ("src/caviar.cpp", 0x00618DA0),
}
BASELINE_SEMANTIC = {0x00618DA0}


def original_bytes(relative):
    return subprocess.check_output(["git", "show", f"{BASE}:{relative}"], cwd=ROOT)


def check_claim_only(current_files):
    for relative in ("src/buffer.cpp", "src/caviar.cpp"):
        path = ROOT / relative
        original = original_bytes(relative)
        before = read_text(original.decode("utf-8"), path)
        current = current_files.get(relative, path.read_bytes())
        after = read_text(current.decode("utf-8"), path)

        selected = [replace(row, byte_exact=True, semantic=False)
                    for row in before if (relative, row.address) in TARGETS]
        assert len(selected) == sum(item[0] == relative for item in TARGETS)
        selected_before = [row for row in before
                           if (relative, row.address) in TARGETS]
        assert {row.address for row in selected_before if row.semantic} == (
            BASELINE_SEMANTIC & {address for file, address in TARGETS
                                 if file == relative})
        assert all(not row.byte_exact for row in selected_before)
        assert all(row.image_spans and row.image_spans[0][0] == row.address
                   for row in selected)

        # BYTE_EXACT subsumes the previous instruction-shape obligation; it
        # does not withdraw that ratchet when the SEMANTIC marker is replaced.
        assert {row.address for row in selected_before
                if row.byte_exact or row.semantic} == (
            BASELINE_SEMANTIC & {address for file, address in TARGETS
                                 if file == relative})
        assert all(row.byte_exact or row.semantic
                   for row in selected)
        assert write_bytes(original, selected) == current
        assert [row.address for row in before] == [row.address for row in after]
        for previous, recorded in zip(before, after):
            expected = (replace(previous, byte_exact=True, semantic=False)
                        if (relative, previous.address) in TARGETS else previous)
            for field in fields(previous):
                if field.name == "region":
                    continue  # Whole-file writer projection covers FILE regions.
                assert getattr(recorded, field.name) == getattr(expected, field.name)


def test_native_publication_changes_only_five_claim_flags():
    check_claim_only({})
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    assert task.target == "opensmacx"
    assert set(task.editable_files) == {
        "src/buffer.cpp", "src/caviar.cpp",
        "recovery/tests/test_graphics_native_claims_oct14fx.py",
    }
    assert {(subject.address_space, subject.address, subject.image_sha256)
            for subject in task.subjects} == {
                ("pe:va", address, IMAGE) for _relative, address in TARGETS
            }


def test_withdrawing_a_claim_is_rejected():
    withdrawn = {relative: original_bytes(relative)
                 for relative, _address in TARGETS}
    with pytest.raises(AssertionError):
        check_claim_only(withdrawn)


def test_unrelated_source_edit_is_rejected():
    relative = "src/caviar.cpp"
    current = (ROOT / relative).read_bytes()
    damaged = current.replace(b"return mem_get(size);",
                              b"return mem_get(size + 1);", 1)
    assert damaged != current
    with pytest.raises(AssertionError):
        check_claim_only({relative: damaged})
