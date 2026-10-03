"""Actual owner bodies: host behavioral witnesses, not native/game acceptance."""
from pathlib import Path
import shutil
import subprocess
import re

import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]
BODIES = {
    "slide": (0x45D440, "void __cdecl slide_show(", "slide_show_owner_control.cpp"),
    "scroll": (0x45DEA0, "void __cdecl scroll_show(", "scroll_show_owner_control.cpp"),
    "popup": (0x404920, "void Popup::show(", "popup_animation_owner_control.cpp"),
}
HIDE_BODIES = {
    "hide_slide": (0x45D970, "void __cdecl slide_hide("),
    "hide_scroll": (0x45E540, "void __cdecl scroll_hide("),
}


def projected_control(kind, damage=None):
    if kind in HIDE_BODIES:
        bodies = []
        rows = read_file(ROOT / "src/popup.cpp")
        for name, (address, signature) in HIDE_BODIES.items():
            row = next(row for row in rows if row.address == address)
            body = _definition(row.region, signature)
            if damage is not None and kind == name:
                old, new = damage
                assert old in body
                body = body.replace(old, new, 1)
            bodies.append(body)
        fixture = (Path(__file__).parent / "popup_hide_owner_control.cpp").read_text()
        assert fixture.count("// ACTUAL_SOURCE_BODIES") == 1
        return fixture.replace("// ACTUAL_SOURCE_BODIES", "\n\n".join(bodies))
    address, signature, filename = BODIES[kind]
    record = next(row for row in read_file(ROOT / "src/popup.cpp") if row.address == address)
    body = _definition(record.region, signature)
    if damage is not None:
        old, new = damage
        assert old in body
        body = body.replace(old, new, 1)
    fixture = (Path(__file__).parent / filename).read_text()
    if kind == "popup":
        header = (ROOT / "src/buffer.h").read_text()
        getters = []
        for name in ("pixel_width", "pixel_height"):
            matches = re.findall(r"int " + name + r"\(\) const \{[^\n]*\}", header)
            assert len(matches) == 1
            getters.append(matches[0])
        assert fixture.count("// ACTUAL_BUFFER_GETTERS") == 1
        fixture = fixture.replace("// ACTUAL_BUFFER_GETTERS", "\n".join(getters))
    assert fixture.count("// ACTUAL_SOURCE_BODY") == 1
    return fixture.replace("// ACTUAL_SOURCE_BODY", body)


def run_control(tmp_path, compiler, source):
    path = tmp_path / "control.cpp"
    path.write_text(source)
    binary = tmp_path / "control"
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(path), "-o", str(binary)],
        check=True, capture_output=True,
    )
    return subprocess.run([str(binary)], capture_output=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [False, True])
def test_actual_interface_up_sprite_accessor(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header = (ROOT / "src/maininterface.h").read_text()
    up = _definition(header, "Sprite &interface_up_sprite(")
    down = _definition(header, "Sprite &interface_down_sprite(")
    if damage:
        assert "sprites13_[0]" in up
        up = up.replace("sprites13_[0]", "sprites13_[1]")
    fixture = """
#include <assert.h>
struct Sprite { int identity; };
class MainInterface {
    Sprite sprites13_[2];
public:
    MainInterface() { sprites13_[0].identity=17; sprites13_[1].identity=29; }
    // UP
    // DOWN
};
int main() {
    MainInterface owner;
    assert(owner.interface_up_sprite().identity==17);
    assert(owner.interface_down_sprite().identity==29);
    assert(&owner.interface_up_sprite()!=&owner.interface_down_sprite());
}
""".replace("// UP", up).replace("// DOWN", down)
    result = run_control(tmp_path, compiler, fixture)
    if damage:
        assert result.returncode < 0
    else:
        assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("kind", [*BODIES, *HIDE_BODIES])
@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_animation_body(tmp_path, kind, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = run_control(tmp_path, compiler, projected_control(kind))
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("kind,old,new", [
    ("slide", "window->Win::is_visible()", "!window->Win::is_visible()"),
    ("slide", "bottom_bounds().left", "bottom_bounds().right"),
    ("slide", "IsMultiplayerNet == 0", "IsMultiplayerNet != 0"),
    ("scroll", "scroll->Win::is_visible()", "!scroll->Win::is_visible()"),
    ("scroll", "-frame_height + lower_left.height()", "frame_height + lower_left.height()"),
    ("scroll", "frame_height = area.bottom - area.top", "frame_height = area_height"),
    ("scroll", "0, panel_bottom + 3, 0, panel_bottom", "0, 0, 0, panel_bottom + 3"),
    ("scroll", "area.top - original_area.top", "original_area.top - area.top"),
    ("scroll", "panel_bottom - 0xA", "panel_bottom - 0xA - PopupScrollVerticalFrameCount"),
    ("popup", "external_window_ != 0 && dialogs_.kind_ == 2", "dialogs_.kind_ == 2"),
    ("popup", "Win::show(3)", "Win::show(requested_mode)"),
    ("popup", "auto_hide_mode == 1000", "auto_hide_mode == 1001"),
    ("popup", "WinScreenHeight - this->Buffer::pixel_height()", "WinScreenHeight + this->Buffer::pixel_height()"),
    ("popup", "> dialogs_.ListBox::visible_item_capacity()", ">= dialogs_.ListBox::visible_item_capacity()"),
    ("hide_slide", "frame < frame_divisor", "frame < mode"),
    ("hide_slide", "window_frame.fill(WinFillColour);", "/* missing frame fill */"),
    ("hide_slide", "area.left + window_frame.pixel_width()", "area.right"),
    ("hide_slide", "MapMouseFeedbackBusy != 0 || ConsoleControlTurnActive != 0", "false"),
    ("hide_slide", "delay = delay / 2", "delay = static_cast<DWORD>(delay) / 2"),
    ("hide_scroll", "area.left = right_edge_x;", "/* missing narrow clip */"),
    ("hide_scroll", "area.left - screen_left, area.top - screen_top", "0, 0"),
    ("hide_scroll", "0, 0, area.left, screen_top", "0, 0, screen_left, screen_top"),
    ("hide_scroll", "frame_x -= PopupScrollHorizontalStep", "frame_x -= horizontal_step"),
    ("hide_scroll", "if (enter_vertical_close)\n        window->Buffer::set_clip", "if (PopupScrollVerticalFrameCount > 1)\n        window->Buffer::set_clip"),
])
def test_animation_damage_detected(tmp_path, kind, old, new):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = run_control(tmp_path, compiler, projected_control(kind, (old, new)))
    assert result.returncode < 0, "damaged body must fail a behavioral assertion"
