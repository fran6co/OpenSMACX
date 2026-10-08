// Private fixture-only host control for alien_fauna.v4.cpp.
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define __cdecl
#include "game_data_state_host.h"
GameDataStateWindow GameDataStateStorage;

const int MaxPlayerNum = 8;
const int TECH_CENTPSI = 63;
const int TECH_QUANTUM = 25;
const int ExpansionEnabled = 0;
int BaseFindDist = 0;
int MapLongitudeBounds = 8;
int MapLatitudeBounds = 8;
int MapLongitude = 4;
int MapArea = 0;
int MapIsFlat = 1;
uint32_t MapNativeLifeForms = 0;

struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
};
struct PlayerData { uint32_t flags; };
struct Base { uint8_t faction_id_current; int8_t population_size; char name_string[25]; };
struct Veh { int faction_id; uint32_t flags; uint8_t visibility; };
struct Continent { uint32_t open_terrain; };
struct Console { int faction_id_; };
struct MapWindow { int focus(int, int) { ++MapWindow::focus_calls; return focus_result; }
    static int focus_calls, focus_result; };
int MapWindow::focus_calls = 0;
int MapWindow::focus_result = 0;
struct NetMsg { void pop(const char *, int, int, const char *) { ++pop_calls; }
    static int pop_calls; };
int NetMsg::pop_calls = 0;

Map Tiles[64];
PlayerData PlayersData[8];
Base Bases[16];
Veh Vehs[32];
Continent Continents[64];
Console ConsoleGlobal = {0};
MapWindow PrimaryMapWindow;
MapWindow *MapWinTable[8] = {&PrimaryMapWindow};
NetMsg NetMsgPrimary;
int RadiusBaseX[9] = {0};
int RadiusBaseY[9] = {0};
int RadiusOffsetX[289] = {0};
int RadiusOffsetY[289] = {0};

int rand_calls = 0;
int rand_values[4096];
int rand_count = 0;
int veh_init_calls = 0;
int fail_prototype = -1;
int cargo_capacity = 0;
int sleep_calls = 0;
int veh_prototypes[64];
int draw_tile_calls = 0;
int draw_tiles_calls = 0;
int life_forms_on_first_draw = -1;
int focus_calls = 0;
int pop_calls = 0;
int parse_calls = 0;
int popup_calls = 0;

extern "C" int rand(void) {
    const int value = rand_calls < rand_count ? rand_values[rand_calls] : 0;
    ++rand_calls;
    return value;
}
int timeGetTime(void) { return 0; }
Map *map_loc(int x, int y) {
    return &Tiles[y * MapLongitude + (x >> 1)];
}
int base_find(int, int, int, int, int, int) { BaseFindDist = 0; return -1; }
int whose_territory(int, int, int, int, int) { return -1; }
void draw_tiles(int, int, int) { ++draw_tiles_calls; }
void draw_tile(int, int, int) {
    ++draw_tile_calls;
    if (draw_tile_calls == 1 && life_forms_on_first_draw >= 0) {
        MapNativeLifeForms = static_cast<uint32_t>(life_forms_on_first_draw);
    }
}
int parse_says(int, const char *, int, int) { ++parse_calls; return 0; }
int POP2(const char *, const char *, int) { ++popup_calls; return 0; }
int goody_at(int, int) { return 0; }
int zoc_any(int, int, int) { return 0; }
int veh_init(int prototype, int, int, int) {
    const int id = veh_init_calls++;
    veh_prototypes[id] = prototype;
    if (prototype == fail_prototype) return -1;
    Vehs[id].faction_id = ConsoleGlobal.faction_id_;
    Vehs[id].flags = 0;
    Vehs[id].visibility = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = veh_init_calls;
    return id;
}
int veh_cargo(int) { return cargo_capacity; }
void veh_promote(int) {}
void sleep(int) { ++sleep_calls; }

#include "actual.inc"

static void reset_state() {
    memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    memset(Tiles, 0, sizeof(Tiles));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Bases, 0, sizeof(Bases));
    memset(Vehs, 0, sizeof(Vehs));
    memset(Continents, 0, sizeof(Continents));
    memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    memset(GameDataStateStorage.fields_.game_tech_achieved_, 0, sizeof(GameDataStateStorage.fields_.game_tech_achieved_));
    for (int project = 0; project < 64; ++project)
        GameDataStateStorage.fields_.secret_project_.project_base[project] = -1;
    memset(rand_values, 0, sizeof(rand_values));
    memset(veh_prototypes, 0, sizeof(veh_prototypes));
    MapWindow::focus_calls = 0;
    NetMsg::pop_calls = 0;
    rand_calls = rand_count = veh_init_calls = draw_tile_calls = 0;
    fail_prototype = -1;
    cargo_capacity = sleep_calls = 0;
    draw_tiles_calls = parse_calls = popup_calls = 0;
    life_forms_on_first_draw = -1;
    GameDataStateStorage.fields_.game_rules_ = GameDataStateStorage.fields_.game_state_ = GameDataStateStorage.fields_.turn_current_num_ = GameDataStateStorage.fields_.diff_level_current_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = BaseFindDist = MapArea = 0;
    MapNativeLifeForms = 0;
    for (int project = 0; project < 64; ++project) {
        GameDataStateStorage.fields_.secret_project_[project] = -1;
    }
    ConsoleGlobal.faction_id_ = 0;
    MapLongitudeBounds = MapLatitudeBounds = 8;
    MapLongitude = 4;
    MapIsFlat = 1;
}

int main() {
    reset_state();
    // RULES bit 0x200000 bypasses all later turn-level fauna while still
    // allowing the one-area fungus-spread path to run.
    GameDataStateStorage.fields_.game_rules_ = 0x200000;
    ConsoleGlobal.faction_id_ = 1;
    MapArea = 33;
    RadiusBaseX[0] = 2;
    RadiusBaseY[0] = 0;
    rand_values[0] = 2;
    rand_values[1] = 2;
    rand_count = 2;
    map_loc(2, 2)->climate = 0;
    map_loc(2, 2)->bit = 0x8000;
    map_loc(2, 2)->visibility = 1 << 1;
    map_loc(2, 2)->bit_visible[0] = 0x8000;
    map_loc(4, 2)->climate = 0x40;
    alien_fauna();
    assert((map_loc(4, 2)->bit & 0x8000) != 0);
    assert((map_loc(4, 2)->bit & 0x20) == 0);
    assert(draw_tiles_calls == 1);
    assert(rand_calls == 2);

    reset_state();
    // A no-spread turn reaches the life-form limit check, then the native
    // early eligibility return must avoid vehicles, callbacks and popups.
    GameDataStateStorage.fields_.secret_project_.voice_of_planet() = 7;
    MapArea = 0;
    alien_fauna();
    assert(veh_init_calls == 0);
    assert(draw_tile_calls == 0 && draw_tiles_calls == 0);
    assert(popup_calls == 0);

    reset_state();
    // At turn 10 the alien transport site is selected once. A successful
    // sea transport is promoted and only then may the achieved technology
    // spawn the independent mindworm unit.
    GameDataStateStorage.fields_.turn_current_num_ = 10;
    MapArea = 0;
    GameDataStateStorage.fields_.game_tech_achieved_[TECH_CENTPSI] = 1;
    rand_values[0] = 2;
    rand_values[1] = 2;
    rand_values[2] = 0;
    rand_values[3] = 0;
    rand_values[4] = 0;
    rand_values[5] = 0;
    rand_count = 6;
    Map *transport_site = map_loc(2, 2);
    transport_site->climate = 0x40;
    transport_site->bit = 0x20;
    transport_site->val2 = 8;
    transport_site->region = 1;
    Continents[1].open_terrain = 0x32;
    alien_fauna();
    assert(veh_init_calls == 2);
    assert(veh_prototypes[0] == 9 && veh_prototypes[1] == 10);

    reset_state();
    GameDataStateStorage.fields_.turn_current_num_ = 10;
    GameDataStateStorage.fields_.game_tech_achieved_[TECH_QUANTUM] = 1;
    rand_values[0] = rand_values[1] = 2;
    rand_count = 2;
    transport_site = map_loc(2, 2);
    transport_site->climate = 0x40;
    transport_site->bit = 0x20;
    transport_site->val2 = 8;
    transport_site->region = 1;
    Continents[1].open_terrain = 0x32;
    fail_prototype = 9;
    alien_fauna();
    assert(veh_init_calls == 1 && veh_prototypes[0] == 9);

    reset_state();
    // Distinct MNL, difficulty and BaseFindDist values discriminate the
    // three native rule operands. At MNL=2, difficulty=0, turn=30:
    // modulo divisor=6, early threshold=20, and cargo count=3.
    GameDataStateStorage.fields_.turn_current_num_ = 30;
    GameDataStateStorage.fields_.diff_level_current_ = 0;
    BaseFindDist = 16;
    MapNativeLifeForms = 2;
    cargo_capacity = 10;
    rand_values[0] = 2;
    rand_values[1] = 2;
    rand_count = 2;
    transport_site = map_loc(2, 2);
    transport_site->climate = 0x40;
    transport_site->bit = 0x20;
    transport_site->val2 = 8;
    transport_site->region = 1;
    Continents[1].open_terrain = 0x32;
    alien_fauna();
    assert(veh_init_calls == 4);
    assert(veh_prototypes[0] == 9 && veh_prototypes[1] == 8 &&
           veh_prototypes[2] == 8 && veh_prototypes[3] == 8);
    assert(sleep_calls == 3);

    reset_state();
    // With MNL=0 and difficulty=4 the native signed early-turn threshold
    // classifies turn 10 as very early, reducing cargo to one.
    GameDataStateStorage.fields_.turn_current_num_ = 10;
    GameDataStateStorage.fields_.diff_level_current_ = 4;
    cargo_capacity = 10;
    rand_values[0] = 2;
    rand_values[1] = 2;
    rand_count = 2;
    transport_site = map_loc(2, 2);
    transport_site->climate = 0x40;
    transport_site->bit = 0x20;
    transport_site->val2 = 8;
    transport_site->region = 1;
    Continents[1].open_terrain = 0x32;
    alien_fauna();
    assert(veh_init_calls == 2);
    assert(veh_prototypes[0] == 9 && veh_prototypes[1] == 8);
    assert(sleep_calls == 1);

    reset_state();
    // A wholly invalid marine-transport search consumes 1000 x/y pairs, but
    // the counter jumps before evaluating tile 1000; then come the ordinary
    // two life-form samples.
    GameDataStateStorage.fields_.turn_current_num_ = 10;
    MapArea = 0;
    rand_count = 0;
    alien_fauna();
    assert(veh_init_calls == 0);
    assert(rand_calls == 2004);

    reset_state();
    // An AI-owned highland sample spawns one visible nearby mindworm. Its
    // draw callback changes the life-form count before the dynamic threshold
    // is checked; subsequent invalid samples finish at the enlarged limit.
    GameDataStateStorage.fields_.turn_current_num_ = 1;
    MapArea = 0;
    MapNativeLifeForms = 0;
    life_forms_on_first_draw = 2;
    RadiusOffsetX[9] = 2;
    RadiusOffsetY[9] = -2;
    rand_values[0] = 2;
    rand_values[1] = 2;
    rand_count = 2;
    Map *const source = map_loc(2, 2);
    source->climate = 0x60;
    source->bit = 0x22;
    source->region = 1;
    source->val2 = 2;
    Map *const neighbor = map_loc(4, 0);
    neighbor->climate = 0x60;
    neighbor->region = 1;
    alien_fauna();
    assert(veh_init_calls == 1 && veh_prototypes[0] == 8);
    assert(draw_tile_calls == 1 && MapNativeLifeForms == 2);

    reset_state();
    // Signed threshold/IDIV and signed JL/JGE all differ from unsigned
    // comparisons here. Turn zero is not very-early against the negative
    // wrapped early bound, so cargo capacity three is fully used; the wrapped
    // negative life limit then exits after the first sample.
    GameDataStateStorage.fields_.turn_current_num_ = 0;
    MapNativeLifeForms = 0x60000000u;
    cargo_capacity = 3;
    rand_values[0] = 2;
    rand_values[1] = 2;
    rand_count = 2;
    transport_site = map_loc(2, 2);
    transport_site->climate = 0x40;
    transport_site->bit = 0x20;
    transport_site->val2 = 8;
    transport_site->region = 1;
    Continents[1].open_terrain = 0x32;
    alien_fauna();
    // Two transport coordinates, three cargo prototype choices (rand is
    // evaluated before the expansion gate), then two sampler coordinates.
    assert(rand_calls == 7);
    assert(veh_init_calls == 4 && sleep_calls == 3);
    assert(veh_prototypes[0] == 9 && veh_prototypes[1] == 8 &&
           veh_prototypes[2] == 8 && veh_prototypes[3] == 8);
    return 0;
}
