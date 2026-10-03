"""Actual owner bodies: host behavioral witnesses, not native/game acceptance."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]
BODIES = {
    "slide": (0x45D440, "void __cdecl slide_show(", "slide_show_owner_control.cpp"),
    "scroll": (0x45DEA0, "void __cdecl scroll_show(", "scroll_show_owner_control.cpp"),
}


def projected_control(kind, damage=None):
    address, signature, filename = BODIES[kind]
    record = next(row for row in read_file(ROOT / "src/popup.cpp") if row.address == address)
    body = _definition(record.region, signature)
    if damage is not None:
        old, new = damage
        assert old in body
        body = body.replace(old, new, 1)
    fixture = (Path(__file__).parent / filename).read_text()
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


@pytest.mark.parametrize("kind", BODIES)
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
])
def test_animation_damage_detected(tmp_path, kind, old, new):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = run_control(tmp_path, compiler, projected_control(kind, (old, new)))
    assert result.returncode < 0, "damaged body must fail a behavioral assertion"
