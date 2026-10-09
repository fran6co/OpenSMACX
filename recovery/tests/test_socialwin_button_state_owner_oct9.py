"""Source controls for SocialWin's recovered typed button/effect access."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated function: {signature}")


def test_button_text_uses_declared_controls_and_preserves_branch_order():
    source = (ROOT / "src/socialwin.cpp").read_text()
    function = body(source, "int SocialWin::on_button_text_draw(")
    flat = (ROOT / "src/flatbutton.h").read_text()
    base = (ROOT / "src/basebutton.h").read_text()

    assert "char *button = reinterpret_cast" not in function
    assert "const FlatButton &button = flatButtons_[button_id];" in function
    assert "int status = button.field_AB8();" in function
    assert "status == 1 || status == 2" in function
    assert "button.selected_state() != 0" in function
    assert "button.field_A74() == 0" in function
    assert "socEffect_.values_[effect]" in function
    assert "reinterpret_cast<const int *>(&socEffect_)" not in function

    assert "int field_AB8() const { return static_cast<int>(field_AB8_); }" in flat
    assert "int selected_state() const { return static_cast<int>(field_A18_); }" in base
    assert "int field_A74() const { return static_cast<int>(field_A74_); }" in base

    guard = function.index("if (button_id < 0x11 || button_id >= 0x1b)")
    button = function.index("const FlatButton &button")
    assert guard < button
    assert function.index("int status = button.field_AB8();") < function.index(
        "button.selected_state() != 0"
    ) < function.index("button.field_A74() == 0")
