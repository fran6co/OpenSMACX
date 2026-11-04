"""Report-owned helper continuity and caller contract; no whole-game proof."""

from dataclasses import fields
from pathlib import Path
import subprocess

from match_recovery.source.reader import read_file, read_text
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "bf1097c96656076db68ec8b7788f47c5ed250a56"
ADDRESS = 0x4A0810


def baseline(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT).decode()


def test_entire_identity_history_and_algorithm_move_to_report_owner():
    previous = baseline("src/stringstruct.cpp")
    current = (ROOT / "src/reportif.cpp").read_text()
    old = next(row for row in read_text(previous, ROOT / "src/stringstruct.cpp")
               if row.address == ADDRESS)
    new = next(row for row in read_file(ROOT / "src/reportif.cpp")
               if row.address == ADDRESS)
    for field in fields(old):
        if field.name not in {"path", "line", "region", "origin"}:
            assert getattr(old, field.name) == getattr(new, field.name)
    assert old.origin == new.origin
    assert not any(row.address == ADDRESS for row in read_file(ROOT / "src/stringstruct.cpp"))
    body = extract(previous, "void __cdecl sat_edit(")
    # No algorithm edit: only the evidence-backed argument identifiers change.
    import re
    names = {"body": "report_faction_id", "faction_id": "target_faction_id",
             "type": "satellite_type"}
    body = re.sub(r"\b(body|faction_id|type)\b", lambda m: names[m[0]], body)
    assert extract(current, "void __cdecl sat_edit(") == body
    for line in previous.splitlines():
        if line.startswith("// RECOVERY_BINDING:") and "Original4A0810" in line:
            assert line in current
        if line.startswith("// SUPERSEDED_BINDING_HISTORY:") and "Original4A0810" in line:
            assert line in current
