"""Permanent controls that compile the body extracted from product source."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, State, read_file


ROOT = Path(__file__).resolve().parents[2]
OWNER = ROOT / "src/alphamenu.cpp"
ADDRESS = 0x00460DD0
END = 0x0046207E
MARKER = "/* ACTUAL ALPHAMENU CHECK BODY */"


def actual_body():
    rows = [row for row in read_file(OWNER) if row.address == ADDRESS]
    assert len(rows) == 1, "native writer must retain exactly one AlphaMenu check identity"
    row, = rows
    assert row.name == "?check@MainMenu@@QAEXH@Z"
    assert row.symbol == "?check@AlphaMenu@@QAEXH@Z"
    assert row.image_spans == ((ADDRESS, END),)
    assert row.mode == Mode.BODY and row.state == State.IMPLEMENTED

    text = OWNER.read_text(encoding="utf-8")
    signature = "void AlphaMenu::check(int veh_id) {"
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start:offset + 1]
    raise AssertionError("unterminated AlphaMenu::check definition")


def compile_run(tmp_path, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    fixture_path = Path(__file__).with_name(
        "alphamenu_check_actual_source_harness.cpp")
    fixture = fixture_path.read_text(encoding="utf-8")
    assert fixture.count(MARKER) == 1
    source = tmp_path / "alphamenu-check.cpp"
    binary = tmp_path / "alphamenu-check"
    source.write_text(fixture.replace(MARKER, body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(source), "-o", str(binary)], capture_output=True, text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run(
        [str(binary)], capture_output=True, text=True, timeout=10,
        env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"),
    )


def test_product_source_body_matches_owner_and_ordered_controls(tmp_path):
    body = actual_body()
    result = compile_run(tmp_path, body)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("name,before,after", [
    ("fallback enables wrong item",
     "if (fallback_item_id == 0x313) enable_menu_item(3, fallback_item_id);",
     "if (fallback_item_id == 0x314) enable_menu_item(3, fallback_item_id);"),
    ("plan-nine inverts row-specific tile flags",
     "const bool can_build_here = row_action_flag && terrain_avail(",
     "const bool can_build_here = !row_action_flag && terrain_avail("),
    ("turn label singular/plural reversed",
     "label_get(turns == 1 ? 0x30 : 0x31)",
     "label_get(turns == 1 ? 0x31 : 0x30)"),
    ("multiplayer any-flag gate becomes all-flags",
     "if (IsMultiplayerNet || IsMultiplayerPBEM)",
     "if (IsMultiplayerNet && IsMultiplayerPBEM)"),
    ("air climate boundary becomes inclusive",
     "(tile->climate & 0xE0u) < 0x60u",
     "(tile->climate & 0xE0u) <= 0x60u"),
    ("plan-eight branch becomes its complement",
     "if (selected_proto.plan == 0x08)",
     "if (selected_proto.plan != 0x08)"),
    ("map owner nibble 8 becomes accepted",
     "tile_owner >= 0 && tile_owner < 8",
     "tile_owner >= 0 && tile_owner <= 8"),
])
def test_product_source_mutants_are_detected(tmp_path, name, before, after):
    body = actual_body()
    assert before in body, name
    mutant = body.replace(before, after, 1)
    assert mutant != body
    result = compile_run(tmp_path, mutant)
    assert result.returncode != 0, f"actual-source fixture missed {name}"


def test_real_console_owner_and_callers_are_not_repointed():
    header = (ROOT / "src/console.h").read_text(encoding="utf-8")
    assert "AlphaMenu alpha_menu_;" in header
    console = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    assert "alpha_menu_.check(selected_vehicle_);" in console
    assert "alpha_menu_.check(veh_id);" in console
    pending = (ROOT / "src/pending_bodies.cpp").read_text(encoding="utf-8")
    assert "PENDING_BODY(0x00460DD0" not in pending
