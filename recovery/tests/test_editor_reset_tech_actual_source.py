"""Actual-source sanitizer controls for Console::editor_reset_tech."""
from __future__ import annotations

import os
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def _block(text: str, start: int) -> str:
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{": depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0: return text[opening : pos + 1]
    raise AssertionError("unterminated C++ block")


def _struct(path: str, name: str) -> str:
    text = (ROOT / path).read_text()
    marker = f"struct {name} {{"
    start = text.index(marker)
    return f"struct {name} " + _block(text, start) + ";"


def _owner_declarations() -> tuple[str, str, str]:
    assert "uint8_t game_tech_achieved_[89];" in (ROOT / "src/game_data_state_window.h").read_text()
    return (_struct("src/veh.h", "Veh"), _struct("src/veh.h", "VehPrototype"),
            _struct("src/base.h", "Base"))


def _installed() -> tuple[str, str, str, str]:
    source = (ROOT / "src/console.cpp").read_text()
    marker = "void Console::editor_reset_tech()"
    body = _block(source, source.index(marker))
    for forbidden in ("reinterpret_cast", "0x96ee08", "0x9a6670", "0x9ab898",
                      "0x9b2098", "0x952832", "0x97d08c", "0x9a64c8", "0x9a64cc"):
        assert forbidden.lower() not in body.lower(), f"still has raw binding: {forbidden}"
    for required in ("PlayersData", "tech_ranking", "game_tech_achieved_", "VehPrototypes",
                     "obsolete_factions", "selected_vehicle_", "Vehs", "proto_id",
                     "base_current_count_", "queue_size", "queue_production_id", "draw_map(1)"):
        assert required in body, f"typed owner method missing expected use {required}"
    assert "veh_kill(selected_vehicle_)" in body, "preserve live selected id through kill callback"
    veh, proto, base = _owner_declarations()
    return veh, proto, base, f"void Console::editor_reset_tech() {body}"


HARNESS = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
using std::int8_t; using std::int16_t; using std::int32_t;
using std::uint8_t; using std::uint16_t; using std::uint32_t;

static const int MaxPlayerNum = 8;
static const int MaxTechnologyNum = 89;
static const int MaxVehProtoNum = 512;
static const int MaxVehProtoFactionNum = 64;
static const int MaxBaseNum = 512;
static int dialog_result = 0;
static int popup_calls = 0;
static int draw_calls = 0;
static int draw_argument = -1;
static bool mutate_base_count_in_kill = false;
static int base_count_after_kill = -1;
static std::vector<int> events;
static std::vector<int> killed;
static const char StrResetAllTech[] = "RESETALLTECH";

struct PlayerData {
    uint8_t prefix[0x35c];
    int32_t tech_ranking;
    uint8_t rest[0x20cc - 0x360];
};
static_assert(offsetof(PlayerData, tech_ranking) == 0x35c, "PlayerData rank offset");
static_assert(sizeof(PlayerData) == 0x20cc, "PlayerData stride");
PlayerData PlayersData[MaxPlayerNum]{};

struct GameDataStateWindowFields {
    uint8_t prefix[0x38];
    int32_t vehicle_current_count_;
    int32_t base_current_count_;
    uint8_t game_tech_achieved_[MaxTechnologyNum + 1]; // extra byte is a harness guard
};
struct GameStorage { GameDataStateWindowFields fields_; };
GameStorage GameDataStateStorage{};

VEH_DECL
PROTO_DECL
BASE_DECL
static_assert(sizeof(Veh) == 0x34, "Veh stride");
static_assert(offsetof(Veh, proto_id) == 0x0a, "Veh proto_id offset");
static_assert(sizeof(VehPrototype) == 0x34, "VehPrototype stride");
static_assert(offsetof(VehPrototype, obsolete_factions) == 0x2c, "prototype obsolete field");
static_assert(offsetof(VehPrototype, flags) == 0x30, "prototype flags field");
static_assert(sizeof(Base) == 0x134, "Base stride");
static_assert(offsetof(Base, queue_size) == 0x4c, "Base queue_size offset");
static_assert(offsetof(Base, queue_production_id) == 0x50, "Base queue row offset");

VehPrototype VehPrototypes[MaxVehProtoNum]{};
Veh Vehs[2049]{};
Base Bases[MaxBaseNum]{};

struct Console {
    uint8_t prefix[0x23bdc];
    int32_t selected_vehicle_;
    void editor_reset_tech();
};
static_assert(offsetof(Console, selected_vehicle_) == 0x23bdc, "Console selection offset");
Console ConsoleGlobal{};

int X_pop(const char *label, int (*)()) {
    assert(label == StrResetAllTech);
    ++popup_calls;
    events.push_back(1);
    return dialog_result;
}
void draw_map(int arg) {
    ++draw_calls; draw_argument = arg; events.push_back(3);
}
void veh_kill(int veh_id) {
    assert(veh_id >= 0 && veh_id < GameDataStateStorage.fields_.vehicle_current_count_);
    killed.push_back(veh_id); events.push_back(2);
    const int old_count = GameDataStateStorage.fields_.vehicle_current_count_;
    for (int i = veh_id; i + 1 < old_count; ++i) Vehs[i] = Vehs[i + 1];
    GameDataStateStorage.fields_.vehicle_current_count_ = old_count - 1;
    if (ConsoleGlobal.selected_vehicle_ == veh_id) ConsoleGlobal.selected_vehicle_ = -1;
    else if (veh_id < ConsoleGlobal.selected_vehicle_) --ConsoleGlobal.selected_vehicle_;
    if (mutate_base_count_in_kill)
        GameDataStateStorage.fields_.base_current_count_ = base_count_after_kill;
}

void Console::editor_reset_tech() BODY

static void reset_fixture() {
    events.clear(); killed.clear(); popup_calls = draw_calls = 0; draw_argument = -1;
    dialog_result = 0; mutate_base_count_in_kill = false; base_count_after_kill = -1;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    ConsoleGlobal.selected_vehicle_ = 123;
    for (int f = 0; f < MaxPlayerNum; ++f) PlayersData[f].tech_ranking = 100 + f;
    for (int i = 0; i <= MaxTechnologyNum; ++i) GameDataStateStorage.fields_.game_tech_achieved_[i] = 0xA5;
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        VehPrototypes[i].obsolete_factions = 0xFF;
        VehPrototypes[i].flags = static_cast<uint16_t>(0xA500u | ((i & 1) ? 1u : 0u));
        VehPrototypes[i].veh_name[0] = static_cast<char>(i);
    }
    for (int i = 0; i < MaxBaseNum; ++i) {
        Bases[i].queue_size = 0x100u + static_cast<uint32_t>(i);
        Bases[i].queue_production_id[0] = -100 - i;
    }
}

static void assert_reset_globals() {
    for (int f = 1; f < MaxPlayerNum; ++f) assert(PlayersData[f].tech_ranking == 0);
    assert(PlayersData[0].tech_ranking == 100);
    for (int i = 0; i < MaxTechnologyNum; ++i) assert(GameDataStateStorage.fields_.game_tech_achieved_[i] == 0);
    assert(GameDataStateStorage.fields_.game_tech_achieved_[MaxTechnologyNum] == 0xA5);
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        assert(VehPrototypes[i].obsolete_factions == 0);
        const uint16_t original = static_cast<uint16_t>(0xA500u | ((i & 1) ? 1u : 0u));
        const uint16_t expected = i < MaxVehProtoFactionNum
            ? original : static_cast<uint16_t>(original & 0xFFFEu);
        assert(VehPrototypes[i].flags == expected);
        assert(VehPrototypes[i].veh_name[0] == static_cast<char>(i));
    }
}

static void test_cancel_is_noop() {
    reset_fixture();
    dialog_result = 0;
    ConsoleGlobal.editor_reset_tech();
    assert((events == std::vector<int>{1}));
    assert(popup_calls == 1 && draw_calls == 0 && killed.empty());
    assert(ConsoleGlobal.selected_vehicle_ == 123);
    assert(PlayersData[1].tech_ranking == 101 && GameDataStateStorage.fields_.game_tech_achieved_[0] == 0xA5);
    assert(VehPrototypes[64].obsolete_factions == 0xFF);
    assert(Bases[0].queue_size == 0x100u);
}

static void test_accepted_reset_and_signed_vehicle_quirk() {
    reset_fixture();
    dialog_result = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = 5;
    GameDataStateStorage.fields_.base_current_count_ = 2;
    const int16_t ids[5] = {63, -1, 64, 80, 63};
    for (int i = 0; i < 5; ++i) {
        Vehs[i].proto_id = ids[i];
        Vehs[i].x = static_cast<int16_t>(10 + i);
    }
    for (int i = 0; i < 4; ++i) Bases[i].queue_production_id[0] = (i == 0 || i == 2) ? 64 : 63;
    mutate_base_count_in_kill = true;
    base_count_after_kill = 3; // proves the base count is sampled after the vehicle callback

    ConsoleGlobal.editor_reset_tech();
    assert_reset_globals();
    assert((events == std::vector<int>{1, 2, 3}));
    assert((killed == std::vector<int>{3}));
    assert(GameDataStateStorage.fields_.vehicle_current_count_ == 4);
    assert(ConsoleGlobal.selected_vehicle_ == -2); // veh_kill clears it; caller then decrements live member
    assert(Vehs[0].proto_id == 63 && Vehs[1].proto_id == -1 && Vehs[2].proto_id == 64 && Vehs[3].proto_id == 63);
    assert(Bases[0].queue_size == 0xFFFFFFBBu && Bases[1].queue_size == 0u);
    assert(Bases[2].queue_size == 0xFFFFFFBBu && Bases[3].queue_size == 0x103u);
    assert(Bases[0].queue_production_id[0] == 64 && Bases[1].queue_production_id[0] == 63);
    assert(draw_calls == 1 && draw_argument == 1);
}

static void test_no_user_vehicle_walks_down_and_negative_proto_is_signed() {
    reset_fixture();
    dialog_result = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = 4;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    Vehs[0].proto_id = 63; Vehs[1].proto_id = -1;
    Vehs[2].proto_id = 0;  Vehs[3].proto_id = 63;
    ConsoleGlobal.editor_reset_tech();
    assert(killed.empty());
    assert(ConsoleGlobal.selected_vehicle_ == -1);
    assert((events == std::vector<int>{1, 3}));
}

int main() {
    test_cancel_is_noop();
    test_accepted_reset_and_signed_vehicle_quirk();
    test_no_user_vehicle_walks_down_and_negative_proto_is_signed();
}
'''


def _compile_run(compiler: str, source: str, output: Path, tmp_path: Path) -> subprocess.CompletedProcess[str]:
    path = tmp_path / (output.name + ".cpp")
    path.write_text(source)
    subprocess.run(
        [compiler, "-std=c++11", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(path), "-o", str(output)],
        check=True,
    )
    return subprocess.run([str(output)], text=True, capture_output=True,
                          env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})


def _exercise(method: str, veh: str, proto: str, base: str, tmp_path: Path) -> None:
    harness = (HARNESS.replace("VEH_DECL", veh).replace("PROTO_DECL", proto)
               .replace("BASE_DECL", base)
               .replace("BODY", method[len("void Console::editor_reset_tech() "):]))
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), f"required host compilers unavailable: {compilers}"
    for compiler in compilers:
        good = _compile_run(compiler, harness, tmp_path / (Path(compiler).name + "-good"), tmp_path)
        assert good.returncode == 0, good.stderr
        # Negative Veh::proto_id is signed in the original CMP/JL. This deliberate
        # unsigned mutant must be killed by the same extracted-body fixture.
        needle = "Vehs[selected_vehicle_].proto_id >= 0x40"
        assert needle in method, "could not apply scoped signedness mutation"
        mutant_method = method.replace(needle,
            "static_cast<uint16_t>(Vehs[selected_vehicle_].proto_id) >= 0x40", 1)
        mutant = harness.replace(method[len("void Console::editor_reset_tech() "):],
                                 mutant_method[len("void Console::editor_reset_tech() "):])
        bad = _compile_run(compiler, mutant, tmp_path / (Path(compiler).name + "-signedness-mutant"), tmp_path)
        assert bad.returncode != 0, f"{Path(compiler).name} failed to detect unsigned proto-id mutant"


def test_installed_actual_source_and_signedness_mutant(tmp_path: Path) -> None:
    veh, proto, base, method = _installed()
    _exercise(method, veh, proto, base, tmp_path)
