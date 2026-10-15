"""Compile the fixture and assert its five bodies are extracted from src owners."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.layout_source import parse_members
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired


# When installed at recovery/tests/, parents[2] is the project root. The
# candidate packet mirrors that exact layout under candidate-src/.
ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / "recovery/tests/fixtures/remaining_game_leaf_owner_host.cpp"

CASES = (
    (0x0046AF90, "src/mapwin.cpp", "void __cdecl draw_tile_fixup(int x_coord, int y_coord, int wider_radius,"),
    (0x0046B040, "src/mapwin.cpp", "void __cdecl draw_tile_fixup(int x_coord, int y_coord, int second_x_coord,"),
    (0x004C96E0, "src/map.cpp", "int __cdecl action_build(int veh_id, char *base_name)"),
    (0x00405B20, "src/popup.cpp", "int Popup::on_dialog_item_back_draw(GraphicWin *a1, int, int a3, RECT *a4)"),
    (0x005F2830, "src/win.cpp", "int __cdecl Win::display_action(int action)"),
)
EXPECTED_ROLES = {
}


def function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated function {signature}")


def test_all_five_owner_bodies_are_extracted_and_compilable(tmp_path):
    fixture = FIXTURE.read_text()
    assert_leaf_collection_retired(ROOT)
    for address, relative, signature in CASES:
        assert_current_owner(ROOT, address, relative)
        owner = (ROOT / relative).read_text()
        entries = [line for line in owner.splitlines()
                   if line.startswith(f"// ORIGINAL: 0x{address:08X}")]
        assert len(entries) == 1
        annotation_window = "\n".join(
            owner.splitlines()[max(0, owner.splitlines().index(entries[0]) - 1):
                                owner.splitlines().index(entries[0]) + 1])
        if address in EXPECTED_ROLES:
            assert f"RECOVERY_ROLE: {EXPECTED_ROLES[address]}" in annotation_window
        else:
            assert "RECOVERY_ROLE: SCAFFOLD" not in annotation_window
        assert function(owner, signature) == function(fixture, signature)

    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    exe = tmp_path / "remaining_game_leaf_owner_host"
    result = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-pedantic",
         str(FIXTURE), "-o", str(exe)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True)
    assert run.returncode == 0, run.stderr


def test_popup_and_action_data_sources_are_real_owners():
    assert_leaf_collection_retired(ROOT)
    popup = (ROOT / "src/popup.cpp").read_text()
    palette = (ROOT / "src/interface_palette.cpp").read_text()
    map_cpp = (ROOT / "src/map.cpp").read_text()
    for address, relative in ((0x00405B20, "src/popup.cpp"),
                             (0x004BA220, "src/interface_palette.cpp"),
                             (0x004C96E0, "src/map.cpp"),
                             (0x005F2830, "src/win.cpp")):
        assert_current_owner(ROOT, address, relative)
    assert "TextColourTable[192 - a3 * 2]" in popup
    assert "TextColourTable[80 - a3 * 2]" in popup
    assert "void __cdecl textcolor_init" in palette
    assert 'char ActionBuildLogCaption[12] = "Action Base";' in map_cpp
    assert 'char ActionBuildTurnLogCaption[17] = "Action Base Turn";' in map_cpp
    assert "initial bytes at 0x687B60 and 0x687B6C" in map_cpp
    assert "original symbols and wider allocation extents are unproved" in map_cpp
    assert "original .data section is writable" in map_cpp
    assert "RECOVERY_OBJECT: v2 0x00687B60 0x00687B6C" in map_cpp
    assert "RECOVERY_OBJECT: v2 0x00687B6C 0x00687B7D" in map_cpp

    autosound = (ROOT / "src/autosound.cpp").read_text()
    autosound_header = (ROOT / "src/autosound.h").read_text()
    assert autosound.count("val_35_ = AutoSoundDefaults.val_35_;") == 3
    assert "val_35_ = 0;" in autosound
    assert "RECOVERY_MEMBER: v3 AutoSound val_35_ Sound* 0x8C 0x005F2830" in autosound_header


def test_mapwin_callers_use_the_public_owner_declarations():
    for relative in ("src/console.cpp", "src/faction.cpp", "src/veh.cpp"):
        source = (ROOT / relative).read_text()
        declarations = [line for line in source.splitlines()
                        if "draw_tile_fixup(" in line
                        and (line.strip().startswith("void __cdecl")
                             or line.strip().startswith("extern void __cdecl"))]
        assert declarations == []
    header = (ROOT / "src/mapwin.h").read_text()
    assert header.count("void __cdecl draw_tile_fixup(") == 2


def test_callback_offset_binding_uses_supported_source_grammar():
    pin = next(line.split()[1] for line in (ROOT / "src/IMAGE_PIN").read_text().splitlines()
               if line.startswith("sha256 "))
    rows = parse_members((ROOT / "src/win.h").read_text(), "src/win.h", pin,
                         address_space="pe:va")
    row = next(row for row in rows if row.class_name == "Win" and row.member == "field_438_")
    assert row.offset == 0x438 and row.evidence.address == 0x5F2830
    # A function-pointer typedef is not an embedded class or a class pointer.
    # Its declaration owns the signature; v1 records the independently proven offset.
    assert row.class_type is None and row.class_pointer_type is None


def test_win_return_paths_are_explicit_in_the_owner():
    header = (ROOT / "src/win.h").read_text()
    source = (ROOT / "src/win.cpp").read_text()
    assert "static int __cdecl display_action(int action);" in header
    assert "typedef int (__cdecl *WinDisplayActionHookFn)();" in header
    body = function(source, "int __cdecl Win::display_action(int action)")
    assert "return ShowWindow(HandleMain, SW_MAXIMIZE);" in body
    assert "return ShowWindow(HandleMain, SW_MINIMIZE);" in body
    assert "result = hook();" in body
    assert "return child->play();" in body
    assert "static_cast<uint32_t>(action) - 2u" in body
    assert "RECOVERY_ROLE: SCAFFOLD" not in source[
        source.index("// ORIGINAL: 0x005F2830"):source.index("// size      267 bytes")]
