"""Limit the gameplay interface publication to its 18 native claim flags."""

from dataclasses import fields, replace
import hashlib
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes
from match_recovery.tasks import TaskStore


ROOT = Path(__file__).resolve().parents[2]
BASE = "190754049f2152430312e7b4fc60933e43261dfb"
TASK = "gameplay-interface-native-claims-oct14fy"
IMAGE = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
FILES = ("src/console.cpp", "src/designwin.cpp", "src/mapwin.cpp",
         "src/faction.cpp")
SNAPSHOT_FILES = (*FILES, "src/game.cpp")
TARGETS = {
    ("src/console.cpp", address) for address in
    (0x0051DED0, 0x0051DF40, 0x005107E0, 0x0051DF00,
     0x0051DEF0, 0x0051DF10, 0x0051DF30)
} | {
    ("src/designwin.cpp", 0x00439670),
} | {
    ("src/mapwin.cpp", address) for address in
    (0x004218C0, 0x004218D0, 0x004218E0, 0x004218F0, 0x00421900,
     0x00421910, 0x00421920, 0x00421930, 0x00421940)
} | {
    ("src/faction.cpp", 0x0055BB30),
}
TASK_ADDRESSES = {
    0x0055B870, 0x0055BB30, 0x0055D190, 0x0051E000,
    0x0051DED0, 0x0051DF40, 0x005107E0, 0x004E0120,
    0x004E0EA0, 0x004D8CD0, 0x0051DF00, 0x0051DDE0,
    0x0051DF50, 0x0051DEF0, 0x0051DF10, 0x0051DF30,
    0x00439670, 0x004218C0, 0x004218D0, 0x004218E0,
    0x004218F0, 0x00421900, 0x00421910, 0x00421930,
    0x00421940, 0x00421820, 0x00421950, 0x00421920,
}
PUBLICATION_SHA256 = {
    "src/console.cpp": "6ae1a40d80f7303f437bb38bec7ce39769ed8a889c009717d97ed634985c7904",
    "src/designwin.cpp": "af679414db2d774758f78b746193a410d5897b16db8fe59cc5810e6442ad7b91",
    "src/mapwin.cpp": "c04329a2f150536f8caa758dec2e4e7d781754aa83b456f322702b1257257463",
    "src/faction.cpp": "e133281f091eedcadcb43db766516b7f04abf90b124d31d064f9e298161b85bf",
    "src/game.cpp": "d5e1b73b303ab69d3dfdd7e4faa627db0fe139fc9456adb7fea92049be0cc12e",
}


def original_bytes(relative):
    return subprocess.check_output(["git", "show", f"{BASE}:{relative}"],
                                   cwd=ROOT)


def projected_publication():
    """Recreate the exact source snapshot measured at this claim checkpoint."""
    result = {}
    for relative in SNAPSHOT_FILES:
        path = ROOT / relative
        original = original_bytes(relative)
        before = read_text(original.decode("utf-8"), path)
        selected_before = [row for row in before
                           if (relative, row.address) in TARGETS]
        if relative == "src/game.cpp":
            assert not selected_before
            result[relative] = original
        else:
            selected = [replace(row, byte_exact=True, semantic=False)
                        for row in selected_before]
            assert len(selected) == sum(file == relative for file, _ in TARGETS)
            assert all(not row.byte_exact for row in selected_before)
            assert all(row.image_spans and row.image_spans[0][0] == row.address
                       for row in selected)
            result[relative] = write_bytes(original, selected)
        assert hashlib.sha256(result[relative]).hexdigest() == PUBLICATION_SHA256[relative]
    return result


def check_live_claim_continuity(current_files):
    """Check task identities and claim continuity, not line/order/body text."""
    task_rows = {}
    for relative in ("src/game.cpp", *FILES):
        original = original_bytes(relative)
        path = ROOT / relative
        before = read_text(original.decode("utf-8"), path)
        current = current_files.get(relative, path.read_bytes())
        after = read_text(current.decode("utf-8"), path)
        for previous in before:
            if previous.address not in TASK_ADDRESSES:
                continue
            identity = (previous.address, previous.name, previous.image_spans)
            task_rows[(relative, identity)] = previous
            matches = [row for row in after
                       if (row.address, row.name, row.image_spans) == identity]
            assert len(matches) == 1
            recorded = matches[0]
            if (relative, previous.address) in TARGETS:
                assert recorded.byte_exact
            elif previous.byte_exact or previous.semantic:
                # A byte claim may replace an earlier shape claim; neither
                # historical obligation may silently disappear.
                assert recorded.byte_exact or recorded.semantic
    assert {address for (_relative, (address, _name, _spans)) in task_rows} == TASK_ADDRESSES


def check_checkpoint_snapshot(current_files):
    """Historical full-file equality for the measured publication only."""
    expected = projected_publication()
    assert set(current_files) == set(SNAPSHOT_FILES)
    for relative in SNAPSHOT_FILES:
        assert current_files[relative] == expected[relative]


def test_native_publication_changes_only_18_claim_flags_and_preserves_task():
    snapshot = projected_publication()
    check_checkpoint_snapshot(snapshot)
    check_live_claim_continuity({})
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    assert task.target == "opensmacx"
    assert set(task.editable_files) == {
        "src/faction.cpp", "src/game.cpp", "src/console.cpp",
        "src/designwin.cpp", "src/mapwin.cpp",
        "recovery/tests/test_gameplay_interface_native_claims_oct14fy.py",
    }
    assert {(subject.address_space, subject.address, subject.image_sha256)
            for subject in task.subjects} == {
                ("pe:va", address, IMAGE) for address in TASK_ADDRESSES
            }
    assert len(task.subjects) == 28
    assert len(TARGETS) == 18


def test_withdrawing_any_claim_is_rejected_by_checkpoint_projection():
    withdrawn = projected_publication()
    withdrawn["src/faction.cpp"] = original_bytes("src/faction.cpp")
    with pytest.raises(AssertionError):
        check_live_claim_continuity(withdrawn)
    with pytest.raises(AssertionError):
        check_checkpoint_snapshot(withdrawn)


def test_unrelated_editor_energy_body_change_does_not_break_live_claims():
    check_live_claim_continuity({})
    relative = "src/console.cpp"
    snapshot = projected_publication()
    historical = snapshot[relative]
    restored = historical.replace(
        b"PlayersData[loop_faction].unk_48",
        b"PlayersData[loop_faction].energy_reserves", 1)
    restored = restored.replace(
        b"for (int loop_faction = 1; loop_faction < MaxPlayerNum; ++loop_faction) {",
        b"// Restored the leader-derived energy field.\n        for (int loop_faction = 1; loop_faction < MaxPlayerNum; ++loop_faction) {",
        1)
    assert restored != historical
    check_live_claim_continuity({relative: restored})
    with pytest.raises(AssertionError):
        check_checkpoint_snapshot({**snapshot, relative: restored})


def test_new_non_target_claim_is_allowed_by_live_continuity():
    relative = "src/game.cpp"
    current = (ROOT / relative).read_bytes()
    records = read_text(current.decode("utf-8"), ROOT / relative)
    candidate = next(row for row in records
                     if row.address not in TASK_ADDRESSES
                     and row.image_spans and not row.byte_exact and not row.semantic)
    strengthened = write_bytes(
        current, [replace(candidate, byte_exact=True, semantic=False)])
    check_live_claim_continuity({relative: strengthened})
