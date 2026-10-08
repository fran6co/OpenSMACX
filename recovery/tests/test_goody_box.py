"""Permanent actual-source controls for the goody_box v24 recovery.

Fixture support is embedded here; no ignored witness/candidate is read at test
runtime. Tests extract the helper namespace and real goody_box owner from
src/veh.cpp and fail if either is absent or still a placeholder.
"""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest
from current_game_storage_fixture import current_game_storage_fixture

ROOT = Path(__file__).resolve().parents[2]
OWNER = ROOT / "src" / "veh.cpp"

FIXTURE_CPP = r'''
// Runnable host witnesses for the actual private body include. All game
// services below are explicitly external doubles; this is not native proof.
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#define __cdecl
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef short int16_t;
#define TRUE 1
#define FALSE 0

struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
};
static_assert(offsetof(Map, val2) == 2, "product Map::val2 offset");
static_assert(offsetof(Map, val3) == 5, "product Map::val3 offset");
static_assert(offsetof(Map, bit) == 8, "product Map::bit offset");
static_assert(sizeof(Map) == 0x2C, "product Map size");
struct Veh {
    int x, y, faction_id, proto_id; uint8_t visibility; uint16_t flags;
    uint32_t state;
    int moves_expended, order, probe_sabotage_id, home_base_id;
    int16_t waypoint_y[4];
    uint8_t order_auto_type;
    uint8_t dmg_incurred, morale;
};
struct VehiclePrototype { int chassis_id, weapon_id; char veh_name[32]; int plan; };
struct RulesChassis { uint8_t speed, padding[0x48], triad; };
struct RulesWeapon { char *name, *name_short; int8_t offense_rating; };
struct RulesBasic { uint32_t move_rate_roads; };
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    int unk_94, unk_95, unk_96, unk_97, energy_reserves;
    int tech_accumulated, flags, current_num_bases, unk_69, tech_achieved;
    int total_mil_veh, unk_70, ranking, tech_ranking;
    SocialEffect soc_effect_pending;
    uint32_t diplo_treaties[8]; uint8_t proto_id_active[512];
    uint8_t region_total_bases[128];
};
struct Base {
    int faction_id_current, minerals_accumulated, queue_production_id[2];
    int queue_size; uint32_t state; char name_string[25];
};
struct RulesFacility { const char *name; int effect, pad, cost; };
struct RulesTechnology { uint32_t flags; const char *name; char id[8]; int pad;
    int growth, tech, wealth, power, preq_tech_1, preq_tech_2; };
struct Player {
    int is_leader_female;
    char filename[24], search_key[24], name_leader[24], title_leader[24];
    char padding[640];
    char noun_faction[24]; int noun_gender, is_noun_plural;
    int rule_flags;
};
struct MapWin { void redraw(); };
struct Console { int faction_id_; int focus(int, int, int); };
struct NetMsg { void pop(const char *, int, int, const char *); };
struct FX { void play(int); void fade(int); };
struct InfoWin { void on_redraw(); };
struct TutWin { int tut_map(const char *, int, int, int, int, int, int, int); };
char StringTemp[1032];

Map MapTiles[5000];
Veh Vehs[8];
VehiclePrototype VehPrototypes[32];
PlayerData PlayersData[8];
Base Bases[32];
Player Players[8];
RulesChassis Chassis[20];
RulesWeapon Weapon[26];
RulesBasic Rules;
RulesFacility Facility[128];
RulesTechnology Technology[89];
int MapLongitude = 50, MapLongitudeBounds = 100, MapLatitudeBounds = 100, MapIsFlat = 1;
// Controlled direction-table doubles; product storage lives in veh.cpp.
int VehMoveDeltaX[9] = {0}, VehMoveDeltaY[9] = {0};
int RadiusOffsetX[289] = {0}, RadiusOffsetY[289] = {0};
int BaseFindDist;
int IsMultiplayerNet, ExpansionEnabled, ConsoleIdUnused;
/* ACTUAL_GAME_STORAGE */
uint32_t MapNativeLifeForms;
int GenderDefault, PluralityDefault;
Console ConsoleGlobal;
MapWin MainMapWin;
MapWin *MapWinTable[8] = { &MainMapWin };

NetMsg NetMsgPrimary;
FX g_FX;
InfoWin ConsoleInfoWin;
TutWin TutWinObject;
TutWin *TutWinGlobal = &TutWinObject;
enum { TECH_FOSSIL = 38, TECH_BRAIN = 48, TECH_ORBITAL = 77 };


static int random_values[128], random_count, random_cursor;
static int spot_calls, map_draws, world_raises, rocky_calls, play_calls;
static int bit_set_calls, last_bit_set_mask, last_bit_set_value;
static int bit_set_x[128], bit_set_y[128];
static const char *last_popup_title, *last_popup_icon;
static int last_popup_duration;
static int base_result, cost_factor_value, veh_cost_value, veh_init_value;
static int base_find_source_distance, base_find_candidate_distance;
static int base_find_owned_distance, base_find_calls;
static int move_source_on_base_find_call, move_source_x, move_source_y;
static int veh_init_calls, veh_skip_calls;
static int veh_init_proto[64], veh_init_x[64], veh_init_y[64];
static int mutate_tile_after_init_call, mutate_tile_index;
static int mutate_source_val3_on_rand_call, mutate_source_val3_value;
static int treaty_calls;
static int kill_calls;
static int tech_available, tech_earned, last_tech_earned;
static int has_tech_52[8], has_tech_53[8];
static int track_spawn_callbacks, spawn_callback_length;
static char spawn_callback_order[16];
static int mutate_turn_after_init_call, mutated_turn_value;
static int mutate_initialized_proto;
static int sleep_calls, sleep_seen_home_base, sleep_seen_morale;
static void mark_spawn_callback(char event) {
    if (!track_spawn_callbacks) return;
    spawn_callback_order[spawn_callback_length++] = event;
    spawn_callback_order[spawn_callback_length] = 0;
}
static int parse_says_calls, parse_says_selectors[8];
static int monolith_calls;
static int bonus_at_calls;
static int alt_set_both_calls, alt_set_both_x, alt_set_both_y;
static int site_radius_calls, site_radius_x, site_radius_y, site_radius_faction;
static int draw_tile_calls, draw_tile_x, draw_tile_y, draw_tile_mode;
static int mutate_basefind_on_draw_call, mutate_basefind_value;
static int veh_cargo_limit = 1;
static int cargo_values[8], cargo_value_count, cargo_value_cursor;
static int cargo_calls;
static int mutate_console_on_spot;
static int last_stack_x, last_stack_y;
static char trace[128];
static int trace_length;
static void mark(char c) { trace[trace_length++] = c; trace[trace_length] = 0; }
static void reset(int climate, int event_roll, int tectonic_roll) {
    for (int i = 0; i < 5000; ++i) MapTiles[i] = Map();
    for (int i = 0; i < 8; ++i) { Vehs[i] = Veh(); PlayersData[i] = PlayerData(); }
    for (int i = 0; i < 32; ++i) Bases[i] = Base();
    for (int i = 0; i < 32; ++i) VehPrototypes[i] = VehiclePrototype();
    for (int i = 0; i < 20; ++i) Chassis[i] = RulesChassis();
    for (int i = 0; i < 9; ++i) VehMoveDeltaX[i] = VehMoveDeltaY[i] = 0;
    for (int i = 0; i < 289; ++i) RadiusOffsetX[i] = RadiusOffsetY[i] = 0;
    Vehs[0].x = 40; Vehs[0].y = 50; Vehs[0].faction_id = 0; Vehs[0].proto_id = 0;
    MapTiles[50 * MapLongitude + 20].climate = (uint8_t)climate;
    MapTiles[50 * MapLongitude + 20].region = 2;
    VehPrototypes[0].chassis_id = 0;
    PlayersData[0].current_num_bases = 2;
    ConsoleGlobal.faction_id_ = 0;
    IsMultiplayerNet = 0; ExpansionEnabled = 0;
    GameDataStateStorage = GameDataStateWindow();
    ScenarioSave = ScenarioSaveState();
    Rules.move_rate_roads = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 0;
    GameDataStateStorage.fields_.diff_level_current_ = 3; BaseFindDist = 13;
    random_values[0] = event_roll; random_values[1] = tectonic_roll;
    random_count = 2; random_cursor = 0;
    spot_calls = map_draws = world_raises = rocky_calls = play_calls = 0;
    bit_set_calls = last_bit_set_mask = last_bit_set_value = 0;
    base_result = -1; cost_factor_value = 1; veh_cost_value = 10;
    base_find_source_distance = base_find_candidate_distance = 13;
    base_find_owned_distance = 13; base_find_calls = 0;
    move_source_on_base_find_call = move_source_x = move_source_y = -1;
    veh_init_value = -1; veh_init_calls = veh_skip_calls = 0;
    for (int i = 0; i < 64; ++i)
        veh_init_proto[i] = veh_init_x[i] = veh_init_y[i] = -1;
    mutate_tile_after_init_call = mutate_tile_index = -1;
    mutate_source_val3_on_rand_call = mutate_source_val3_value = -1;
    treaty_calls = 0;
    kill_calls = 0;
    tech_available = tech_earned = last_tech_earned = 0;
    for (int i = 0; i < 8; ++i) has_tech_52[i] = has_tech_53[i] = 0;
    track_spawn_callbacks = spawn_callback_length = 0;
    spawn_callback_order[0] = 0;
    mutate_turn_after_init_call = mutated_turn_value = -1;
    mutate_initialized_proto = -1;
    sleep_calls = 0; sleep_seen_home_base = sleep_seen_morale = -1;
    parse_says_calls = 0;
    for (int i = 0; i < 8; ++i) parse_says_selectors[i] = -1;
    GenderDefault = PluralityDefault = 0;
    monolith_calls = 0;
    bonus_at_calls = 0;
    alt_set_both_calls = alt_set_both_x = alt_set_both_y = 0;
    site_radius_calls = site_radius_x = site_radius_y = site_radius_faction = 0;
    draw_tile_calls = draw_tile_x = draw_tile_y = draw_tile_mode = 0;
    mutate_basefind_on_draw_call = mutate_basefind_value = -1;
    veh_cargo_limit = 1;
    cargo_value_count = cargo_value_cursor = cargo_calls = 0;
    for (int i = 0; i < 8; ++i) cargo_values[i] = veh_cargo_limit;
    mutate_console_on_spot = 0;
    last_stack_x = last_stack_y = -1;

    trace_length = 0; trace[0] = 0;
    last_popup_title = last_popup_icon = 0;
    last_popup_duration = 0;
    StringTemp[0] = 0;
}

void NetMsg::pop(const char *title, int duration, int, const char *icon) {
    last_popup_title = title; last_popup_duration = duration;
    last_popup_icon = icon; mark('N'); mark_spawn_callback('N');
}
void FX::play(int) { ++play_calls; mark('P'); }
void FX::fade(int) { mark('F'); }
void InfoWin::on_redraw() {}
void MapWin::redraw() { mark('W'); }
int Console::focus(int, int, int) { mark('X'); return 0; }
int TutWin::tut_map(const char *, int, int, int, int, int, int, int) { return 0; }
int random(int, int) { return 1; }
int rand() {
    if (random_cursor >= random_count) {
        fprintf(stderr, "exhausted CRT rand script at %d/%d\n",
                random_cursor, random_count);
        assert(random_cursor < random_count);
    }
    const int value = random_values[random_cursor++];
    if (random_cursor == mutate_source_val3_on_rand_call)
        MapTiles[MapLongitude * Vehs[0].y + (Vehs[0].x >> 1)].val3 =
            (uint8_t)mutate_source_val3_value;
    return value;
}
int do_unity_crash(int, int, int) { return 0; }
int my_srand(int) { return 0; }
int base_find(int x, int y) {
    ++base_find_calls;
    BaseFindDist = (x == Vehs[0].x && y == Vehs[0].y)
        ? base_find_source_distance : base_find_candidate_distance;
    if (base_find_calls == move_source_on_base_find_call) {
        Vehs[0].x = move_source_x;
        Vehs[0].y = move_source_y;
    }
    return base_result;
}
int base_find(int, int, int) {
    BaseFindDist = base_find_owned_distance;
    return base_result;
}
void bit_set(int x, int y, int mask, int value) {
    if (bit_set_calls < 128) {
        bit_set_x[bit_set_calls] = x;
        bit_set_y[bit_set_calls] = y;
    }
    ++bit_set_calls; last_bit_set_mask = mask; last_bit_set_value = value;
    Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];
    if (value) tile->bit |= (uint32_t)mask;
    else tile->bit &= ~(uint32_t)mask;
}
int bonus_at(int, int, int) { ++bonus_at_calls; return 0; }
void site_radius(int x, int y, int faction) {
    ++site_radius_calls; site_radius_x = x; site_radius_y = y;
    site_radius_faction = faction;
    for (int i = 0; i < 21; ++i) {
        int tx = x + RadiusOffsetX[i];
        if (!(MapIsFlat & 1)) {
            if (tx < 0) tx += MapLongitudeBounds;
            else if (tx >= MapLongitudeBounds) tx -= MapLongitudeBounds;
        }
        const int ty = y + RadiusOffsetY[i];
        if (tx >= 0 && tx < MapLongitudeBounds
            && ty >= 0 && ty < MapLatitudeBounds)
            MapTiles[MapLongitude * ty + (tx >> 1)].val2 &= 0x0F;
    }
}
void alt_set_both(int x, int y, int) {
    ++alt_set_both_calls; alt_set_both_x = x; alt_set_both_y = y;
}
int wave_it(int) { return 0; }
int ambience(int) { return 0; }
int parse_say(int, int, int, int) { return 0; }
void clear() { StringTemp[0] = 0; }
void says(const char *text) { strcat(StringTemp, text); }
int parse_says(int selector, char *, int, int) {
    mark_spawn_callback('P');
    if (parse_says_calls < 8) parse_says_selectors[parse_says_calls] = selector;
    ++parse_says_calls;
    return 0;
}
int parse_num(int, int) { return 0; }
int veh_init(int proto, int, int x, int y) {
    mark_spawn_callback('I');
    if (veh_init_calls < 64) {
        veh_init_proto[veh_init_calls] = proto;
        veh_init_x[veh_init_calls] = x;
        veh_init_y[veh_init_calls] = y;
    }
    ++veh_init_calls;
    if (track_spawn_callbacks && veh_init_value >= 0)
        Vehs[veh_init_value].home_base_id = 77;
    if (track_spawn_callbacks && veh_init_value >= 0
        && mutate_initialized_proto >= 0)
        Vehs[veh_init_value].proto_id = mutate_initialized_proto;
    if (veh_init_calls == mutate_turn_after_init_call)
        GameDataStateStorage.fields_.turn_current_num_ = mutated_turn_value;
    if (veh_init_calls == mutate_tile_after_init_call)
        MapTiles[mutate_tile_index].bit &= ~3u;
    return veh_init_value;
}
int veh_cargo(int) {
    ++cargo_calls;
    if (cargo_value_cursor < cargo_value_count)
        return cargo_values[cargo_value_cursor++];
    return veh_cargo_limit;
}
int speed_proto(int) { return 4; }
void draw_tile(int x, int y, int mode) {
    ++draw_tile_calls; draw_tile_x = x; draw_tile_y = y;
    draw_tile_mode = mode; mark('D');
    if (draw_tile_calls == mutate_basefind_on_draw_call)
        BaseFindDist = mutate_basefind_value;
}
void draw_tiles(int, int, int) {}
void draw_map(int) { ++map_draws; mark('D'); }
void world_climate() { mark('C'); }
void world_raise_alt(int, int) { ++world_raises; mark('R'); }
void rocky_set(int, int, int) { ++rocky_calls; mark('K'); }
int minerals_at(int, int) { return 0; }
void synch_bit(int, int, int) {}
int cost_factor(int, int, int) { return cost_factor_value; }
int veh_cost(int, int, int *) { return veh_cost_value; }
int stack_check(int, int, int, int, int) { return 1; }
int stack_veh(int veh, int) { mark('V'); return veh; }
void stack_put(int, int x, int y) { last_stack_x = x; last_stack_y = y; mark('T'); }
void draw_tile_fixup(int, int, int, int) { mark('F'); }
int has_abil(int, int) { return 0; }
int zoc_any(int, int, int) { return 0; }
int abs(int value) { return value < 0 ? -value : value; }
void spot_all(int, int) { mark('S'); }
int study_artifact(int) { return 0; }
int spot_loc(int, int, int) {
    ++spot_calls;
    if (mutate_console_on_spot) ConsoleGlobal.faction_id_ = 1;
    return 0;
}
int tut_check(int) { return 0; }
void monolith(int) { ++monolith_calls; }
void treaty_on(int, int, int) { ++treaty_calls; }
void veh_skip(int) { ++veh_skip_calls; }
void sleep(int vehicle_id) {
    ++sleep_calls;
    sleep_seen_home_base = Vehs[vehicle_id].home_base_id;
    sleep_seen_morale = Vehs[vehicle_id].morale;
    mark_spawn_callback('L');
}
int has_tech(int tech, int faction) {
    return tech == 0x52 ? has_tech_52[faction]
         : tech == 0x53 ? has_tech_53[faction] : 0;
}
int tech_avail(int, int) { return tech_available; }
int tech_is_preq(int, int, int) { return 0; }
void tech_achieved(int faction, int tech, int, int) {
    ++tech_earned; last_tech_earned = tech;
    ++PlayersData[faction].tech_achieved;
}
void kill(int) { ++kill_calls; }

'''


def _block(source, brace, start, label):
    depth = 0
    state = "code"
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/": state = "line"; index += 2; continue
            if char == "/" and nxt == "*": state = "block"; index += 2; continue
            if char == chr(34): state = "string"
            elif char == chr(39): state = "character"
            elif char == "{": depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0: return source[start:index + 1]
        elif state == "line":
            if char == chr(10): state = "code"
        elif state == "block":
            if char == "*" and nxt == "/": state = "code"; index += 2; continue
        elif state == "string":
            if char == chr(92): index += 2; continue
            if char == chr(34): state = "code"
        elif state == "character":
            if char == chr(92): index += 2; continue
            if char == chr(39): state = "code"
        index += 1
    raise AssertionError("unterminated C++ block: " + label)


def _extract_namespace(source):
    match = re.search(r"\bnamespace\s+goody_box_(?:detail|candidate)\s*\{", source)
    assert match, "goody_box helper namespace missing from src/veh.cpp"
    brace = source.find("{", match.start(), match.end())
    return _block(source, brace, match.start(), "goody_box namespace")


def _extract_owner(source):
    match = re.search(r"\bint\s+__cdecl\s+goody_box\s*\([^)]*\)\s*\{",
                      source, re.S)
    assert match, "real goody_box definition missing from src/veh.cpp"
    brace = source.find("{", match.start(), match.end())
    body = _block(source, brace, match.start(), "goody_box")
    assert "BODY GOES HERE." not in body and "PLACEHOLDER" not in body, (
        "goody_box is still a placeholder"
    )
    return body


def _compilers():
    found = [shutil.which(name) for name in ("g++", "clang++")]
    found = [compiler for compiler in found if compiler]
    if not found: pytest.skip("neither GCC nor Clang is installed")
    return found


def _compile_run(compiler, namespace, owner, main, expected_run=0):
    with tempfile.TemporaryDirectory(prefix="osmx-goody-actual-source-") as temp:
        cpp = Path(temp) / "goody_actual_source.cpp"
        exe = Path(temp) / "goody_actual_source"
        fixture = FIXTURE_CPP.replace(
            "/* ACTUAL_GAME_STORAGE */", current_game_storage_fixture(scenario=True))
        # Keep the product's signed faction / unsigned ranking equality intact.
        # The former signed fixture global hid these two host diagnostics.
        # Observe and whitelist only these warnings below, not future warnings.
        cpp.write_text(fixture + namespace + chr(10) + owner + chr(10) + main)
        built = subprocess.run(
            [compiler, "-std=c++14", "-Wall", "-Wextra", "-Werror",
             "-Wno-parentheses", "-Wno-error=sign-compare", "-O0", str(cpp), "-o", str(exe)],
            text=True, capture_output=True, check=False,
        )
        assert built.returncode == 0, (compiler + " compile rc="
            + str(built.returncode) + chr(10) + built.stdout + built.stderr)
        signedness = re.findall(r"warning:[^\n]*\[-Wsign-compare\]", built.stderr)
        assert len(signedness) == 2, built.stderr
        assert owner.count(
            "faction_id == GameDataStateStorage.fields_.ranking_faction_id_unk2_") == 2
        diagnostics = built.stderr.split("warning:")[1:]
        assert len(diagnostics) == 2 and all(
            "ranking_faction_id_unk2_" in diagnostic for diagnostic in diagnostics), built.stderr
        ran = subprocess.run([str(exe)], check=False)
        if expected_run is None:
            assert ran.returncode != 0, compiler + " mutant unexpectedly passed"
        else:
            assert ran.returncode == expected_run, (compiler + " run rc="
                + str(ran.returncode) + ", expected=" + str(expected_run))


RETRY_MAIN = r'''
int main() {
    reset(0x40, 12, 0);
    base_find_source_distance = 1;
    move_source_on_base_find_call = 2;
    move_source_x = 44; move_source_y = 0;
    random_values[0] = 12; random_values[1] = 1;
    random_values[2] = 0; random_values[3] = 1;
    random_values[4] = 1; random_values[5] = 0;
    for (int draw = 6; draw < 16; ++draw) random_values[draw] = 1;
    random_count = 16;
    assert(goody_box(0, 0) == 0);
    assert(Vehs[0].x == 44 && Vehs[0].y == 0);
    assert(random_cursor == 6 && PlayersData[0].energy_reserves == 25);
    return 0;
}
'''

SPAWN_MAIN = r'''
int main() {
    reset(0x20, 9, 0);
    random_values[0] = 9; random_values[1] = 1; random_values[2] = 0;
    random_count = 3; veh_init_value = 3;
    GameDataStateStorage.fields_.turn_current_num_ = 50;
    track_spawn_callbacks = 1; mutate_initialized_proto = 14;
    VehPrototypes[13].chassis_id = 0;
    VehPrototypes[14].chassis_id = 1;
    Chassis[0].triad = 1;
    Chassis[1].triad = 0;
    assert(goody_box(0, 0) == 0);
    assert(veh_init_calls == 1 && veh_init_proto[0] == 13);
    assert(Vehs[3].proto_id == 14 && sleep_calls == 1);
    assert(strcmp(spawn_callback_order, "ILPN") == 0);
    return 0;
}
'''

RELOCATION_MAIN = r'''
int main() {
    reset(0x60, 4, 0);
    GameDataStateStorage.fields_.turn_current_num_ = 0x32;
    random_values[0] = 4; random_values[1] = 46; random_values[2] = 54;
    random_count = 3;
    MapTiles[54 * MapLongitude + 23].climate = 0x60;
    MapTiles[54 * MapLongitude + 23].region = 2;
    assert(goody_box(0, 0) == 0);
    assert(random_cursor == 3 && last_stack_x == 46 && last_stack_y == 54);
    assert(strcmp(trace, "NVTFXWS") == 0);
    return 0;
}
'''

OVERFLOW_MAIN = r'''
int main() {
    reset(0x60, 25, 0);
    random_values[0] = 25; random_values[1] = 0; random_count = 2;
    assert(goody_box(0, 0) == 0);
    assert(bit_set_calls == 1 && last_bit_set_mask == 0x400
           && last_bit_set_value == TRUE);
    assert(site_radius_calls == 1 && random_cursor == 2);
    return 0;
}
'''

EVENT18_MAIN = r'''
int main() {
    reset(0x60, 18, 0);
    VehPrototypes[0].weapon_id = 0; Weapon[0].offense_rating = 0;
    random_values[0] = 18; random_values[1] = 0;
    random_values[2] = 1; random_values[3] = 0; random_count = 4;
    assert(goody_box(0, 0) == 0);
    assert(bit_set_calls == 0 && PlayersData[0].energy_reserves == 25);
    assert(random_cursor == 4);
    return 0;
}
'''


def _actual_parts():
    source = OWNER.read_text()
    return _extract_namespace(source), _extract_owner(source)


def test_actual_retry_reloads_live_vehicle_coordinates_and_edge_roll():
    namespace, owner = _actual_parts()
    retry_block = "x = Vehs[veh_id].x;\n        y = Vehs[veh_id].y;"
    assert owner.count(retry_block) >= 2, "source/retry coordinate loads missing"
    for compiler in _compilers():
        _compile_run(compiler, namespace, owner, RETRY_MAIN)
        before, separator, after = owner.rpartition(retry_block)
        assert separator, "shared retry coordinate reload missing"
        mutant = before + "/* stale snapshot mutant */" + after
        _compile_run(compiler, namespace, mutant, RETRY_MAIN, expected_run=None)


def test_actual_spawn_uses_initialized_prototype_for_live_triad_and_order():
    namespace, owner = _actual_parts()
    live_proto_read = "Vehs[new_vehicle].proto_id"
    assert live_proto_read in owner, "spawn gate no longer reads initialized prototype"
    for compiler in _compilers():
        _compile_run(compiler, namespace, owner, SPAWN_MAIN)
        mutant = owner.replace(live_proto_read, "proto_id", 1)
        _compile_run(compiler, namespace, mutant, SPAWN_MAIN, expected_run=None)


def test_actual_relocation_callback_order_and_coordinates():
    namespace, owner = _actual_parts()
    for compiler in _compilers():
        _compile_run(compiler, namespace, owner, RELOCATION_MAIN)


def test_actual_overflow_claim_and_event18_nonoverflow_gate():
    namespace, owner = _actual_parts()
    overflow_gate = "if (event_id > 18) {"
    assert overflow_gate in owner, "overflow/default event gate changed"
    for compiler in _compilers():
        _compile_run(compiler, namespace, owner, OVERFLOW_MAIN)
        _compile_run(compiler, namespace, owner, EVENT18_MAIN)
        mutant = owner.replace(overflow_gate, "if (event_id >= 18) {")
        _compile_run(compiler, namespace, mutant, EVENT18_MAIN, expected_run=None)
