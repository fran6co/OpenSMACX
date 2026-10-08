"""Strict actual-body controls for Console::editor_reset_faction.

Product statements are extracted verbatim; the fixture
selects actual member declaration spellings/types but intentionally does not
assert partial-record offsets or the full game layouts.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def struct_body(text: str, signature: str) -> str:
    return braced(text, signature) + ";"


def declaration(text: str, struct_name: str, member: str) -> str:
    body = struct_body(text, f"struct {struct_name} {{")
    for line in body.splitlines()[1:-1]:
        clean = line.split("//", 1)[0].strip()
        if clean.endswith(";") and re.search(r"\b" + re.escape(member) + r"\b", clean):
            return clean
    raise AssertionError(f"missing {struct_name}::{member} declaration")


def selected_struct(name: str, fields: tuple[str, ...], header: str) -> str:
    members = [declaration(header, name, field) for field in fields]
    return "struct " + name + " {\n" + "\n".join("    " + m for m in members) + "\n};"


def actual_parts():
    source = (ROOT / "src/console.cpp").read_text()
    faction_h = (ROOT / "src/faction.h").read_text()
    base_h = (ROOT / "src/base.h").read_text()
    veh_h = (ROOT / "src/veh.h").read_text()
    body = braced(source, "void Console::editor_reset_faction()")

    # Strict owner assertions: do not run an offset-based or hand-rewritten
    # fallback while the product migration is pending.
    for obsolete in ("faction_state", "base_governor", "veh_state",
                     "reinterpret_cast", "0x833", "0x4d", "0xd"):
        assert obsolete not in body, f"raw-layout path remains: {obsolete}"
    for required in ("faction->se_alloc_labs", "faction->se_alloc_psych",
                     "faction->soc_category_pending.models_",
                     "faction->soc_category_active.models_",
                     "faction->major_atrocities", "faction->atrocities",
                     "faction->integrity_blemishes", "base->faction_id_current",
                     "base->governor", "base->queue_size", "vehicle->faction_id",
                     "vehicle->order", "vehicle->state"):
        assert required in body, f"typed member access missing: {required}"

    assert body.index("faction->se_alloc_labs = 5") < body.index("compute_faction_modifiers")
    assert body.index("faction->se_alloc_psych = 0") < body.index("compute_faction_modifiers")
    assert body.index("compute_faction_modifiers") < body.index("*active_model = 0")
    assert body.index("*active_model = 0") < body.index("*pending_model = 0")
    assert body.index("social_upkeep") < body.index("int count = GameDataStateStorage.fields_.base_current_count_")
    assert body.index("faction->major_atrocities = 0") < body.index("faction->atrocities = 0")
    assert body.index("vehicle->order = 0") < body.index("vehicle->state &= 0xfcffbdff")
    assert body.count("GameDataStateStorage.fields_.vehicle_current_count_") == 1
    assert body.index("if (X_pop(") < body.index("PlayerData *faction = &PlayersData[1]")

    social_category = struct_body(faction_h, "struct SocialCategory {")
    player = selected_struct("PlayerData", (
        "se_alloc_psych", "se_alloc_labs", "soc_category_pending",
        "soc_category_active", "atrocities", "major_atrocities",
        "integrity_blemishes", "global_reputation"), faction_h)
    base = selected_struct("Base", (
        "faction_id_current", "governor", "queue_size",
        "queue_production_id"), base_h)
    veh = selected_struct("Veh", (
        "state", "flags", "proto_id", "faction_id", "order",
        "waypoint_count"), veh_h)
    return body, social_category, player, base, veh


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

SOCIAL_CATEGORY
PLAYER_DATA
BASE_RECORD
VEH_RECORD

struct Console { void editor_reset_faction(); };
typedef int (*NullCallback)();
static int popup_response;
static int popup_calls;
static std::vector<std::string> trace;
static const char StrResetAllFac[] = "RESET_ALL";
int X_pop(const char *label, NullCallback callback) {
    assert(std::string(label) == "RESET_ALL");
    assert(callback == 0);
    ++popup_calls;
    trace.push_back("popup");
    return popup_response;
}
static PlayerData PlayersData[8];
static Base Bases[4];
static Veh Vehs[4];
static uint32_t TectonicDetonationCount[8];
struct HostGameFields { int vehicle_current_count_, base_current_count_; };
struct HostGameStorage { HostGameFields fields_; } GameDataStateStorage;
static unsigned int compute_count, upkeep_count;

void compute_faction_modifiers(int faction_id) {
    assert(faction_id == static_cast<int>(compute_count + 1));
    PlayerData &p = PlayersData[faction_id];
    assert(p.se_alloc_psych == 0 && p.se_alloc_labs == 5);
    assert(p.soc_category_pending.models_[0] != 0);
    assert(p.soc_category_active.models_[0] != 0);
    trace.push_back("compute" + std::to_string(faction_id));
    // A callback-visible mutation must be cleared by the following loop.
    p.soc_category_pending.models_[3] = 0xD00D0000u + faction_id;
    ++compute_count;
}
void social_upkeep(int faction_id) {
    assert(faction_id == static_cast<int>(upkeep_count + 1));
    PlayerData &p = PlayersData[faction_id];
    for (int i = 0; i != 4; ++i) {
        assert(p.soc_category_active.models_[i] == 0);
        assert(p.soc_category_pending.models_[i] == 0);
    }
    trace.push_back("upkeep" + std::to_string(faction_id));
    // The product samples both record counts only after this callback.
    GameDataStateStorage.fields_.base_current_count_ = faction_id == 7 ? 3 : 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = faction_id == 7 ? 3 : 1;
    ++upkeep_count;
}

BODY

static void initialize() {
    std::memset(PlayersData, 0, sizeof PlayersData);
    std::memset(Bases, 0, sizeof Bases);
    std::memset(Vehs, 0, sizeof Vehs);
    for (int f = 0; f != 8; ++f) {
        PlayersData[f].se_alloc_psych = 70 + f;
        PlayersData[f].se_alloc_labs = 80 + f;
        PlayersData[f].atrocities = 90 + f;
        PlayersData[f].major_atrocities = 100 + f;
        PlayersData[f].integrity_blemishes = 110 + f;
        PlayersData[f].global_reputation = 120 + f;
        for (int i = 0; i != 4; ++i) {
            PlayersData[f].soc_category_pending.models_[i] = 0x1000u + f * 16 + i;
            PlayersData[f].soc_category_active.models_[i] = 0x2000u + f * 16 + i;
        }
        TectonicDetonationCount[f] = 200 + f;
    }
    Bases[0].faction_id_current = 7; Bases[0].governor = 0xFFFFFFFFu;
    Bases[0].queue_size = 7; Bases[0].queue_production_id[0] = 0x12345678;
    Bases[1].faction_id_current = 2; Bases[1].governor = 0x81234567u;
    Bases[1].queue_size = 8; Bases[1].queue_production_id[0] = 0x23456789;
    Bases[2].faction_id_current = 7; Bases[2].governor = 0xF2345678u;
    Bases[2].queue_size = 9; Bases[2].queue_production_id[0] = 0x3456789A;
    Vehs[0].faction_id = 7; Vehs[0].state = 0xFFFFFFFFu;
    Vehs[0].order = 5; Vehs[0].flags = 0x3456; Vehs[0].proto_id = 21;
    Vehs[1].faction_id = 2; Vehs[1].state = 0x12345678u;
    Vehs[1].order = 6; Vehs[1].flags = 0x4567; Vehs[1].proto_id = 22;
    Vehs[2].faction_id = 7; Vehs[2].state = 0x76543210u;
    Vehs[2].order = 7; Vehs[2].flags = 0x5678; Vehs[2].proto_id = 23;
    popup_calls = 0; compute_count = upkeep_count = 0;
    trace.clear();
    GameDataStateStorage.fields_.base_current_count_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
}

int main() {
    Console console;
    initialize();
    popup_response = 0;
    console.editor_reset_faction();
    assert(popup_calls == 1 && compute_count == 0 && upkeep_count == 0);
    for (int f = 0; f != 8; ++f) {
        assert(PlayersData[f].se_alloc_psych == 70 + f);
        assert(PlayersData[f].se_alloc_labs == 80 + f);
        assert(TectonicDetonationCount[f] == static_cast<uint32_t>(200 + f));
    }

    initialize();
    popup_response = 1;
    console.editor_reset_faction();
    assert(popup_calls == 1 && compute_count == 7 && upkeep_count == 7);
    assert(trace.size() == 15 && trace[0] == "popup");
    for (int f = 1; f <= 7; ++f) {
        assert(trace[2 * f - 1] == "compute" + std::to_string(f));
        assert(trace[2 * f] == "upkeep" + std::to_string(f));
        assert(PlayersData[f].se_alloc_psych == 0);
        assert(PlayersData[f].se_alloc_labs == 5);
        assert(PlayersData[f].atrocities == 0);
        assert(PlayersData[f].major_atrocities == 0);
        assert(PlayersData[f].integrity_blemishes == 0);
        assert(PlayersData[f].global_reputation == 120 + f);
        for (int i = 0; i != 4; ++i) {
            assert(PlayersData[f].soc_category_pending.models_[i] == 0);
            assert(PlayersData[f].soc_category_active.models_[i] == 0);
        }
        assert(TectonicDetonationCount[f] == 0);
    }
    assert(TectonicDetonationCount[0] == 200);

    // Only faction 7 sees the callback-published count 3. Its two matching
    // bases/vehicles are cleared; the nonmatching records and unrelated queue,
    // flag, and prototype fields remain untouched.
    assert(Bases[0].governor == 0x7FFFFFFFu && Bases[0].queue_size == 0);
    assert(Bases[0].queue_production_id[0] == 0x12345678);
    assert(Bases[1].governor == 0x81234567u && Bases[1].queue_size == 8);
    assert(Bases[1].queue_production_id[0] == 0x23456789);
    assert(Bases[2].governor == 0x72345678u && Bases[2].queue_size == 0);
    assert(Bases[2].queue_production_id[0] == 0x3456789A);
    assert(Vehs[0].state == (0xFFFFFFFFu & 0xFCFFBDFFu) && Vehs[0].order == 0);
    assert(Vehs[0].flags == 0x3456 && Vehs[0].proto_id == 21);
    assert(Vehs[1].state == 0x12345678u && Vehs[1].order == 6);
    assert(Vehs[1].flags == 0x4567 && Vehs[1].proto_id == 22);
    assert(Vehs[2].state == (0x76543210u & 0xFCFFBDFFu) && Vehs[2].order == 0);
    assert(Vehs[2].flags == 0x5678 && Vehs[2].proto_id == 23);
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_editor_reset_faction_actual_body(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is not installed")
    body, social, player, base, veh = actual_parts()
    program = (HARNESS.replace("SOCIAL_CATEGORY", social)
               .replace("PLAYER_DATA", player)
               .replace("BASE_RECORD", base)
               .replace("VEH_RECORD", veh)
               .replace("BODY", body))
    cpp = tmp_path / "editor-reset-faction.cpp"
    exe = tmp_path / "editor-reset-faction"
    cpp.write_text(program)
    build = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(cpp), "-o", str(exe)],
        text=True, capture_output=True, timeout=60,
    )
    assert build.returncode == 0, build.stderr
    run = subprocess.run(
        [str(exe)], text=True, capture_output=True, timeout=15,
        env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"),
    )
    assert run.returncode == 0, run.stdout + run.stderr
