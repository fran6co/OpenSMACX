"""A callback typedef is not a class-pointer layout interpretation."""
from dataclasses import replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.layout_source import parse_members, read_members

ROOT = Path(__file__).resolve().parents[2]
BASE = "acb1bc751"
PATH = "src/jackal/checkbox.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
INVALID = "// RECOVERY_MEMBER: v3 CheckBox key_notification_callback_ WinNotificationCallback 0x8 0x0060F8B0"
VALID = "// RECOVERY_MEMBER: v1 CheckBox key_notification_callback_ 0x8 0x0060F8B0"


def _assert_key_member_contract(text):
    records = parse_members(text, PATH, PIN, address_space="pe:va")
    matches = [r for r in records
               if r.class_name == "CheckBox" and r.member == "key_notification_callback_"]
    assert len(matches) == 1
    member = matches[0]
    assert (member.offset, member.evidence.address) == (0x8, 0x0060F8B0), (
        "callback member must remain at offset 0x8 with pinned 0x0060F8B0 evidence"
    )
    assert member.class_type is member.class_pointer_type is member.class_array_type is None
    assert VALID in text
    return member


def test_exact_annotation_only_correction_and_source_obligation_continuity():
    before = subprocess.check_output(["git", "show", BASE + ":" + PATH], cwd=ROOT, text=True)
    after = (ROOT / PATH).read_text()
    assert before.count(INVALID) == 1
    with pytest.raises(ValueError, match="version/type"):
        parse_members(before, PATH, PIN, address_space="pe:va")

    # The correction remains the exact supported v1 member annotation, while
    # unrelated accessors/members may be added elsewhere in this live header.
    assert after.count(VALID) == 1
    _assert_key_member_contract(after)
    assert "WinNotificationCallback key_notification_callback_;" in after
    assert "typedef void (__cdecl *WinNotificationCallback)(int selected_id);" in (ROOT / "src/win.h").read_text()


def test_native_layout_reader_preserves_all_other_members():
    before = subprocess.check_output(["git", "show", BASE + ":" + PATH], cwd=ROOT, text=True)
    old_valid = before.replace("  " + INVALID + "\n", "", 1)
    prior = parse_members(old_valid, PATH, PIN, address_space="pe:va")
    current = parse_members((ROOT / PATH).read_text(), PATH, PIN, address_space="pe:va")
    new = next(r for r in current if r.member == "key_notification_callback_")
    assert (new.class_name, new.offset, new.evidence.address) == ("CheckBox", 8, 0x60F8B0)
    assert new.class_type is new.class_pointer_type is new.class_array_type is None
    current_by_key = {(r.class_name, r.member): r for r in current}
    assert len(current_by_key) == len(current)
    for old in prior:
        retained = current_by_key[(old.class_name, old.member)]
        assert replace(retained, line=1) == replace(old, line=1), (
            f"existing member annotation changed: {old.class_name}::{old.member}"
        )
    assert new in read_members(ROOT, PIN)


def test_member_annotation_damage_controls_reject_wrong_version_or_offset():
    current = (ROOT / PATH).read_text()
    assert current.count(VALID) == 1
    wrong_version = current.replace(VALID, INVALID, 1)
    with pytest.raises(ValueError, match="version/type"):
        parse_members(wrong_version, PATH, PIN, address_space="pe:va")

    wrong_offset_line = VALID.replace("0x8", "0xC")
    wrong_offset = current.replace(VALID, wrong_offset_line, 1)
    with pytest.raises(AssertionError, match="0x8"):
        _assert_key_member_contract(wrong_offset)
