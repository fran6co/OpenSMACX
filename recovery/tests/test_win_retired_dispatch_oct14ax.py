"""Retire unused dispatch scaffolding without changing mapped Win identities.

These checks establish source/history continuity and named-call use only; they
do not claim added function coverage, compiled equality, or runtime acceptance.
"""

from dataclasses import fields
from pathlib import Path
import re
import subprocess

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
BASE = "f481d97cf2924ff0c4db617b6cad02fe3aa3c7d9"
WIN_CPP = ROOT / "src/win.cpp"
ON_LBUTTON_DOWN = "void Win::OnLButtonDown(HWND window, LONG dbl, int x, int y, WPARAM keys)"
KEY_CLICK = "int Win::key_click_event(int key, int flags)"
HISTORY_FIELDS = {
    "address", "state", "name", "image_spans", "symbol", "kind", "exclusion",
    "byte_exact", "semantic", "recipe", "levers", "ruled_out", "unrecoverable",
    "deferred", "source_role", "source_role_reason",
}


def _baseline() -> str:
    result = subprocess.run(
        ["git", "show", f"{BASE}:src/win.cpp"], cwd=ROOT,
        check=True, capture_output=True, text=True, timeout=30,
    )
    return result.stdout


def _function(text: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        text,
    )
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unterminated function {signature}")


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin or ())


def _metadata(row):
    return tuple((item.name, getattr(row, item.name)) for item in fields(row)
                 if item.name in HISTORY_FIELDS)


def test_original_mapping_identities_and_history_are_unchanged_from_task_base():
    baseline = _baseline()
    current = WIN_CPP.read_text(encoding="utf-8")
    before = {row.address: row for row in read_text(baseline, f"{BASE}:src/win.cpp")}
    after = {row.address: row for row in read_text(current, WIN_CPP)}
    assert set(after) == set(before)
    for address, old in before.items():
        assert _metadata(after[address]) == _metadata(old), f"mapping metadata {address:#x}"
        assert _history(after[address]) == _history(old), f"mapping history {address:#x}"


def test_only_dead_dispatch_scaffolding_is_absent_and_named_calls_remain():
    source = WIN_CPP.read_text(encoding="utf-8")
    code = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    for identifier in (
        "WinSlotHitTest", "WinSlotLeftDown", "func_win_hit_test", "func_win_left_down",
        "VCall2Fn", "VCall0", "VCall1",
    ):
        assert re.search(rf"\b{re.escape(identifier)}\b", code) is None, identifier

    button = _function(source, ON_LBUTTON_DOWN)
    assert "target->on_nc_hittest(x, y)" in button
    assert "target->on_l_button_down(dbl, x, y, keys, WinMouseDirect)" in button
    assert "reinterpret_cast<uint8_t **>(target)" not in button
    assert not re.search(r"\b(?:vtable|vtbl|tvtbl|vtbl2|cvtbl|cvtbl2)\b", button)

    key_click = _function(source, KEY_CLICK)
    for call in ("this->on_button_clicked(-2)", "this->on_button_clicked(-1)",
                 "target->pass_dialog_focus()", "target->key_click_event(key, flags)",
                 "this->on_key_click(key, flags)"):
        assert call in key_click, call
    assert "reinterpret_cast<" not in key_click
    assert not re.search(r"\b(?:vtable|vtbl|tvtbl|vtbl2|cvtbl|cvtbl2)\b", key_click)
