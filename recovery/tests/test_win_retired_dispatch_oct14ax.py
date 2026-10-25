"""Actual-source checks for typed Win dispatch after scaffolding retirement.

Whole-project native checks protect original identities and historical claims;
these controls keep event calls direct and typed across legitimate source moves.
"""

from pathlib import Path
import re

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
WIN_CPP = ROOT / "src/win.cpp"
ON_LBUTTON_DOWN = "void Win::OnLButtonDown(HWND window, LONG dbl, int x, int y, WPARAM keys)"
KEY_CLICK = "int Win::key_click_event(int key, int flags)"
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
