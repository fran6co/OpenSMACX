"""Actual-source new-game/menu host controls; not native/game proof."""
from pathlib import Path
import shutil
import subprocess
import re

import pytest
from match_recovery.source import read_file
from recovery.tests.test_pact_ascent import definition as extract_definition

ROOT = Path(__file__).resolve().parents[2]
CONTROL = Path(__file__).with_name("newgame_menu_control.cpp")
SOURCE = ROOT / "src/newgame.cpp"
DEFAULTS_HEADER = ROOT / "src/dialog_defaults.h"


def definition(text, signature):
    return extract_definition(text, re.escape(signature))


def product_region(address):
    rows = [row for row in read_file(SOURCE) if row.address == address]
    assert len(rows) == 1
    return rows[0].region


def product_function(address, signature):
    return definition(product_region(address), signature)


def product_reset_helper():
    return definition(SOURCE.read_text(), "static void multiplayer_reset_network_state()")


def custom_helpers():
    text = SOURCE.read_text()
    start = text.index("static const int CustomPlanetSlots[6]")
    end = text.index("// ORIGINAL: 0x0058C2A0", start)
    return text[start:end].strip()


def constant_block(start_marker, end_marker):
    text = SOURCE.read_text()
    start = text.index(start_marker)
    end = text.index(end_marker, start)
    return text[start:end].strip()


def top_menu_bundle():
    text = SOURCE.read_text()
    declaration = re.search(
        r"^static (?:inline )?void top_menu_present_opening\(", text, re.MULTILINE)
    assert declaration is not None, "missing actual opening helper"
    start = declaration.start()
    end = text.index("// ORIGINAL: 0x0058E360", start)
    helper = text[start:end]
    return helper + product_function(0x58E360, "int __cdecl top_menu(int mode)")


def case_template(name):
    text = CONTROL.read_text()
    start_marker = "// @@CASE_" + name + "@@\n"
    end_marker = "// @@ENDCASE_" + name + "@@"
    start = text.index(start_marker) + len(start_marker)
    end = text.index(end_marker, start)
    return text[start:end]


def extract_struct(text, name):
    signature = f"struct {name} {{"
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char in ('"', "'"):
                state = "string" if char == '"' else "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1] + ";"
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 1
        elif state in ("string", "character"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unterminated declaration: {name}")


def bind_actual_network_storage(fixture):
    header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
    player = fixture.index("struct MockPlayer {")
    old_state_start = fixture.index("struct MockOptions {")
    fixture = (fixture[:old_state_start] + "\n".join(
        extract_struct(header, name) for name in (
            "NetWinLobbyRow", "NetWinLobbyRowsStorage", "NetWinSessionBlock",
            "NetWinSessionDataStorage", "NetWinSessionOptionsState",
            "NetWinSessionOptionsStorage",
        )
    ) + "\n" + fixture[player:])
    fixture = fixture.replace("static MockOptions NetWinSessionOptions;",
                              "static NetWinSessionOptionsStorage NetWinSessionOptions;")
    fixture = fixture.replace("static MockSession NetWinSessionData;",
                              "static NetWinSessionDataStorage NetWinSessionData;")
    fixture = fixture.replace("static MockLobbyRow NetWinLobbyRows[8];",
                              "static NetWinLobbyRowsStorage NetWinLobbyRows;")
    fixture = fixture.replace("NetWinSessionData.", "NetWinSessionData.fields_.")
    fixture = fixture.replace("NetWinSessionOptions.", "NetWinSessionOptions.fields_.")
    fixture = fixture.replace("NetWinLobbyRows[", "NetWinLobbyRows.rows_[")
    fixture = fixture.replace("memset(&NetWinSessionData, 0, sizeof(NetWinSessionData));",
                              "memset(&NetWinSessionData.fields_, 0, sizeof(NetWinSessionData.fields_));")
    fixture = fixture.replace("memset(&NetWinSessionOptions, 0, sizeof(NetWinSessionOptions));",
                              "memset(&NetWinSessionOptions.fields_, 0, sizeof(NetWinSessionOptions.fields_));")
    fixture = fixture.replace("memset(NetWinLobbyRows, 0, sizeof(NetWinLobbyRows));",
                              "memset(NetWinLobbyRows.rows_, 0, sizeof(NetWinLobbyRows.rows_));")
    return fixture


def projected(name, damage=None):
    source_bodies = {
        "map_menu": product_function(0x58D6B0, "int __cdecl map_menu(int quick_start)"),
        "custom_planet": product_function(0x58C2A0, "int __cdecl custom_planet(int show_preview, int use_saved_choices)"),
        "size_of_planet": product_function(0x58CED0, "int __cdecl size_of_planet(int mode)"),
        "top_menu": top_menu_bundle(),
        "multiplayer_init": product_function(0x58DB30, "int __cdecl multiplayer_init(int startup_mode)"),
        "multiplayer_reset": product_reset_helper(),
    }
    body = source_bodies[name]
    if damage is not None:
        old, new = damage
        assert old in body, (name, old)
        body = body.replace(old, new, 1)
    fixture = case_template(name)
    if name == "multiplayer_init":
        fixture = bind_actual_network_storage(fixture)
    fixture = DEFAULTS_HEADER.read_text().replace("#pragma once\n", "", 1) + fixture
    fixture = fixture.replace("// @@BODY_" + name + "@@", body)
    if name == "map_menu":
        fixture = fixture.replace("// @@HELPERS_map_menu@@", constant_block(
            "static const char MapMenuTitle[]", "// ORIGINAL: 0x0058D6B0"))
    if name == "custom_planet":
        fixture = fixture.replace("// @@HELPERS_custom_planet@@", custom_helpers())
    if name == "size_of_planet":
        fixture = fixture.replace("// @@HELPERS_size_of_planet@@", constant_block(
            "static const char WorldSizePopupLabel[]", "// ORIGINAL: 0x0058CED0"))
    if name == "multiplayer_init":
        fixture = fixture.replace("// @@RESET_BODY@@", product_reset_helper())
    if name == "multiplayer_reset":
        fixture = fixture.replace("// @@RESET_BODY@@", body)
        fixture = fixture.replace("multiplayer_reset_network_state_owner_v1()",
                                  "multiplayer_reset_network_state()")
    assert "@@BODY_" not in fixture and "@@RESET_BODY@@" not in fixture
    return fixture


def run_control(tmp_path, compiler, source):
    path = tmp_path / "control.cpp"
    path.write_text(source)
    binary = tmp_path / "control"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-Wno-unused-function",
                    "-pedantic", str(path), "-o", str(binary)],
                   check=True, capture_output=True, timeout=45)
    return subprocess.run([str(binary)], capture_output=True, timeout=15)


@pytest.mark.parametrize("name", ["g++", "clang++"])
@pytest.mark.parametrize("case", ["map_menu", "custom_planet", "size_of_planet",
                                   "top_menu", "multiplayer_init", "multiplayer_reset"])
def test_actual_newgame_body(tmp_path, name, case):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + " unavailable")
    result = run_control(tmp_path, compiler, projected(case))
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("case,old,new", [
    ("map_menu", "MapXPlanetPath))", "MapXPlanetXPath))"),
    ("custom_planet", "if (show_preview != 0) {\n        for (int image = 0; image < 3; ++image)",
     "if (show_preview == 0) {\n        for (int image = 0; image < 3; ++image)"),
    ("size_of_planet", "prefs_put(CustomMapXKey, custom_width, 0);",
     "prefs_put(CustomMapYKey, custom_width, 0);"),
    ("top_menu", "esi = multiplayer_init(0);", "esi = map_menu(0);"),
    ("multiplayer_init", "static_cast<uint8_t>(\n            NetWinSessionOptions.fields_.time_control_)",
     "static_cast<int8_t>(\n            NetWinSessionOptions.fields_.time_control_)"),
    ("multiplayer_reset", "network.primary_state_.leading_words_[0] = 0;",
     "network.primary_state_.leading_words_[1] = 0;"),
    ("custom_planet", "if (slot == 3)", "if (saved_value == 3)"),
    ("custom_planet", "popup.set_value(static_cast<int>(AlphaIniPrefs.custom_world[slot]));",
     "popup.set_value(saved_value);"),
    ("top_menu", "faction = ConsoleGlobal.faction_id_;\n        GenderDefault",
     "/* omitted live faction reload */\n        GenderDefault"),
    ("top_menu", "num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000)",
     "num_objectives(ConsoleGlobal.faction_id_, (GameDataStateStorage.fields_.game_rules_ & 0x1000) != 0)"),
    ("multiplayer_init", "num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000u)",
     "num_objectives(faction, GameDataStateStorage.fields_.game_rules_ & 0x1000u)"),
    ("multiplayer_init", "num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000u)",
     "num_objectives(ConsoleGlobal.faction_id_, (GameDataStateStorage.fields_.game_rules_ & 0x1000u) != 0)"),
])
def test_actual_newgame_damage_fails(tmp_path, case, old, new):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = run_control(tmp_path, compiler, projected(case, (old, new)))
    assert result.returncode != 0, "damaged product body passed its runtime witness"
