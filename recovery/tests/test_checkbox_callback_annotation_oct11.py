"""A callback typedef is not a class-pointer layout interpretation."""
from dataclasses import replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.layout_source import parse_members, read_members
from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "acb1bc751"
PATH = "src/jackal/checkbox.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
INVALID = "// RECOVERY_MEMBER: v3 CheckBox key_notification_callback_ WinNotificationCallback 0x8 0x0060F8B0"
VALID = "// RECOVERY_MEMBER: v1 CheckBox key_notification_callback_ 0x8 0x0060F8B0"


def test_exact_annotation_only_correction_and_source_obligation_continuity():
    before = subprocess.check_output(["git", "show", BASE + ":" + PATH], cwd=ROOT, text=True)
    after = (ROOT / PATH).read_text()
    assert before.count(INVALID) == 1
    assert after == before.replace(INVALID, VALID, 1)
    with pytest.raises(ValueError, match="version/type"):
        parse_members(before, PATH, PIN, address_space="pe:va")
    records = lambda text: [replace(r, line=1, region="") for r in read_text(text, ROOT / PATH)]
    assert records(before) == records(after)
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
    assert [replace(r, line=1) for r in current if r is not new] == [replace(r, line=1) for r in prior]
    assert new in read_members(ROOT, PIN)
