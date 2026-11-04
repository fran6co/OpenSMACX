from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]


def test_dialog_origin_members_are_signed_and_propagated_to_every_owner():
    header = (ROOT / "src/dialog.h").read_text(encoding="utf-8")
    declaration = header[header.index("class Dialog {"):header.index("static_assert(sizeof(Dialog)")]
    assert "int window_x_;" in declaration
    assert "int window_y_;" in declaration
    assert "uint32_t field_24_;" not in declaration
    assert "uint32_t field_28_;" not in declaration

    sources = (
        "dialog.cpp", "listbox.cpp", "spritebox.cpp", "editgroup.cpp",
        "pickwin.cpp", "net_session_picker.cpp", "reportif.cpp",
        "newgame.cpp", "jackal/checkbox.cpp",
        "jackal/radiobutton.cpp", "jackal/filewin.cpp",
    )
    for relative in sources:
        source = (ROOT / "src" / relative).read_text(encoding="utf-8")
        assert "Dialog::field_24_" not in source, relative
        assert "Dialog::field_28_" not in source, relative
        assert "window_x_" in source, relative
        assert "window_y_" in source, relative


def test_three_draw_offsets_preserve_original_dword_wrap_before_signed_use():
    expectations = {
        "src/jackal/checkbox.cpp": ("Dialog::window_x_", "original_x", "Dialog::window_y_", "original_y"),
        "src/jackal/radiobutton.cpp": ("Dialog::window_x_", "original_x", "Dialog::window_y_", "original_y"),
        "src/spritebox.cpp": ("Dialog::window_x_", "x", "Dialog::window_y_", "y"),
    }
    for relative, (x_origin, x_local, y_origin, y_local) in expectations.items():
        source = (ROOT / relative).read_text(encoding="utf-8")
        for origin, local in ((x_origin, x_local), (y_origin, y_local)):
            pattern = (
                r"static_cast<int32_t>\(\s*static_cast<uint32_t>\("
                + re.escape(origin)
                + r"\)\s*\+\s*static_cast<uint32_t>\("
                + re.escape(local)
                + r"\)\s*\)"
            )
            assert len(re.findall(pattern, source)) == 1, relative
