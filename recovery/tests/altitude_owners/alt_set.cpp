
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define __cdecl
enum { MaxPlayerNum = 8 };
enum { ALT_SHORE_LINE = 3, ALT_2_LEVELS_ABOVE_SEA = 5,
       ROCKINESS_FLAT = 0, ROCKINESS_ROCKY = 2,
       FAC_PRESSURE_DOME = 26, TRIAD_LAND = 0, TRIAD_SEA = 1, TRIAD_AIR = 2 };

struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1, territory;
    uint32_t bit, bit2, bit_visible[7];
};
struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former;
    int8_t population_size;
    uint8_t assimilation_turns_left, nerve_staple_turns_left, unk_1, visibility;
    uint8_t faction_pop_size_intel[8];
    char name_string[25];
    int16_t unk_x, unk_y;
    uint32_t state;
    uint8_t tail[0x100];
};
struct PlayerData {
    uint32_t flags;
    uint8_t before_base_count[0x105C - 4];
    int current_num_bases;
    uint8_t tail[0x20CC - 0x1060];
};
struct Player { char adj_name_faction[128]; };
struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags, proto_id, unk_1;
    uint8_t faction_id, year_end_lurking, dmg_incurred;
    int8_t order;
    uint8_t waypoint_count, patrol_current_point;
    int16_t waypoint_x[4], waypoint_y[4];
    uint8_t morale, terraforming_turns, order_auto_type, visibility,
            moves_expended;
    int8_t facing;
    uint8_t unk_6, move_to_ai_type, probe_action, probe_sabotage_id;
    int16_t home_base_id, next_veh_id_stack, prev_veh_id_stack;
};
struct VehPrototype { uint8_t prefix[0x24]; uint8_t chassis_id; uint8_t tail[0x0F]; };
struct Chassis { uint8_t prefix[0x49]; uint8_t triad; uint8_t tail[0x46]; };
struct Console { int faction_id_; int focus_x, focus_y, focus_faction; void focus(int x,int y,int f) { focus_x=x; focus_y=y; focus_faction=f; } };

#define STATIC_CHECK(name, expr) typedef char static_check_##name[(expr) ? 1 : -1]
STATIC_CHECK(map_size, sizeof(Map) == 0x2C);
STATIC_CHECK(map_bits, offsetof(Map, bit_visible) == 0x10);
STATIC_CHECK(base_size, sizeof(Base) == 0x134);
STATIC_CHECK(base_population, offsetof(Base, population_size) == 6);
STATIC_CHECK(base_visibility, offsetof(Base, visibility) == 0xA);
STATIC_CHECK(base_name, offsetof(Base, name_string) == 0x13);
STATIC_CHECK(playerdata_size, sizeof(PlayerData) == 0x20CC);
STATIC_CHECK(playerdata_base_count, offsetof(PlayerData, current_num_bases) == 0x105C);
STATIC_CHECK(veh_size, sizeof(Veh) == 0x34);
STATIC_CHECK(veh_stack_link, offsetof(Veh, next_veh_id_stack) == 0x30);
STATIC_CHECK(prototype_size, sizeof(VehPrototype) == 0x34);
STATIC_CHECK(prototype_chassis, offsetof(VehPrototype, chassis_id) == 0x24);
STATIC_CHECK(chassis_size, sizeof(Chassis) == 0x90);
STATIC_CHECK(chassis_triad, offsetof(Chassis, triad) == 0x49);

struct GameDataFields { int turn_current_num_; };
struct GameDataState { GameDataFields fields_; } GameDataStateStorage;
uint32_t WorldClimateSkipTerritoryReset;
uint32_t UnkBitfield1;
struct TileStorage { Map before, tile, after; } TileStorageWithGuards;
#define Tile (TileStorageWithGuards.tile)
Base Bases[8];
PlayerData PlayersData[8];
Player Players[8];
Veh Vehs[16];
VehPrototype VehPrototypes[256];
Chassis Chassis[16];
int MapBaseSubmergedCount[8];
int MapBaseIdClosestSubmergedVeh[8];
int VehCurrentCount;
int VehBattleUnkTgl;
int ConsoleControlTurnActive;
int IsMultiplayerNet;
char ScriptPopupFileText[] = "SCRIPT";
char *ScriptPopupFile = ScriptPopupFileText;
Console ConsoleGlobal;
int BaseAtResult;
int ObjectiveResult;
int FacilityBuiltResult;
int StackHead;
int RandomValue;
int EventCount;
int Events[64];

void event(int n) { Events[EventCount++] = n; }
Map *map_loc(int, int) { return &Tile; }
void delete_landmark(int, int) { event(1); }
int base_at(int, int) { event(2); return BaseAtResult; }
int has_fac_built(int facility, int base) {
    if (facility != FAC_PRESSURE_DOME || base != BaseAtResult) return -9;
    event(3); return FacilityBuiltResult;
}
void set_fac(int facility, int base, int value) {
    if (facility == FAC_PRESSURE_DOME && base == BaseAtResult && value == 1) event(4);
}
int rand(void) { event(5); return RandomValue; }
int is_objective(int base) { return base == BaseAtResult ? ObjectiveResult : 0; }
int parse_says(int, const char *, int, int) { event(6); return 0; }
int popp(const char *, const char *, int, const char *, int) { event(7); return 0; }
int X_pop(char *, const char *, int) { event(8); return 0; }
void base_kill(int base) { if (base == BaseAtResult) event(9); }
int veh_at(int, int) { event(10); return StackHead; }
int stack_fix(int value) { return value; }
int base_find(int, int, int) { event(11); return 77; }
void veh_kill(int id) {
    event(12 + id);
    for (int i = id; i + 1 < VehCurrentCount; ++i) Vehs[i] = Vehs[i + 1];
    --VehCurrentCount;
}


/* ACTUAL_BODIES */

void reset_fixture() {
    memset(&TileStorageWithGuards, 0x5A, sizeof(TileStorageWithGuards));
    memset(&Tile, 0, sizeof(Tile));
    memset(Bases, 0, sizeof(Bases));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Players, 0, sizeof(Players));
    memset(Vehs, 0, sizeof(Vehs));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Chassis, 0, sizeof(Chassis));
    memset(MapBaseSubmergedCount, 0, sizeof(MapBaseSubmergedCount));
    memset(MapBaseIdClosestSubmergedVeh, 0xFF, sizeof(MapBaseIdClosestSubmergedVeh));
    GameDataStateStorage.fields_.turn_current_num_ = 1;
    WorldClimateSkipTerritoryReset = 0; UnkBitfield1 = 0;
    VehCurrentCount = 0; VehBattleUnkTgl = 0; ConsoleControlTurnActive = 0;
    IsMultiplayerNet = 0;
    ConsoleGlobal.faction_id_ = 2; BaseAtResult = 0; ObjectiveResult = 0;
    FacilityBuiltResult = 0; StackHead = -1; RandomValue = 0;
    EventCount = 0;
}
void check(bool ok, int line) { if (!ok) { fprintf(stderr, "check failed at %d\n", line); exit(1); } }
#define CHECK(x) check((x), __LINE__)
int main() {
    reset_fixture();
    GameDataStateStorage.fields_.turn_current_num_ = 0;
    Tile.climate = 0x60; Tile.bit = 0x1234;
    alt_set(4, 6, ALT_2_LEVELS_ABOVE_SEA);
    CHECK(Tile.climate == 0xA0 && UnkBitfield1 == 4 && EventCount == 0);
    CHECK(Tile.bit == 0x1234);

    // Original turn-zero branch joins the unconditional climate/dirty tail.
    // Preserve all other tile bytes and flags, including for wraparound input.
    const int altitudes[] = {INT_MIN, -257, -1, 0, 1, 2, 3, 5, 7, 8, 255, 256, INT_MAX};
    const uint8_t climates[] = {0x00, 0x1F, 0x6B, 0xE3, 0xFF};
    for (size_t a = 0; a < sizeof(altitudes) / sizeof(altitudes[0]); ++a) {
        for (size_t c = 0; c < sizeof(climates) / sizeof(climates[0]); ++c) {
            reset_fixture();
            GameDataStateStorage.fields_.turn_current_num_ = 0;
            memset(&Tile, 0xA7, sizeof(Tile));
            Tile.climate = climates[c];
            Map expected = Tile;
            const Map before_guard = TileStorageWithGuards.before;
            const Map after_guard = TileStorageWithGuards.after;
            expected.climate = static_cast<uint8_t>((climates[c] & 0x1Fu)
                | ((static_cast<uint32_t>(altitudes[a]) & 0xFFu) << 5));
            UnkBitfield1 = 0xA1;
            alt_set(4, 6, altitudes[a]);
            CHECK(memcmp(&Tile, &expected, sizeof(Tile)) == 0);
            CHECK(memcmp(&TileStorageWithGuards.before, &before_guard, sizeof(Map)) == 0);
            CHECK(memcmp(&TileStorageWithGuards.after, &after_guard, sizeof(Map)) == 0);
            CHECK(UnkBitfield1 == 0xA5 && EventCount == 0);
        }
    }

    reset_fixture();
    Tile.climate = static_cast<uint8_t>(3 << 5); Tile.val3 = 0x03;
    alt_set(4, 6, 4);
    CHECK(Tile.climate == (4 << 5));
    CHECK((Tile.val3 >> 6) == 1);
    CHECK((UnkBitfield1 & 5) == 5);
    CHECK(EventCount == 2 && Events[0] == 1 && Events[1] == 5);

    reset_fixture();
    Tile.climate = static_cast<uint8_t>(2 << 5); Tile.bit = 0xA5A5;
    Tile.bit2 = 0xFFFFFFFFu;
    alt_set(5, 7, 3);
    CHECK(Tile.climate == (3 << 5));
    CHECK(Tile.bit2 == (0xFFFFFFFFu & 0x7E1377A3u));
    for (int i = 0; i < 7; ++i) CHECK(Tile.bit_visible[i] == Tile.bit);
    CHECK((UnkBitfield1 & 4) != 0);
    CHECK(EventCount >= 1 && Events[0] == 1);

    reset_fixture();
    Tile.climate = static_cast<uint8_t>(3 << 5); Tile.bit = 0x55AA;
    Bases[0].x = 5; Bases[0].y = 7; Bases[0].faction_id_current = 2;
    Bases[0].population_size = 6; BaseAtResult = 0;
    PlayersData[2].current_num_bases = 2; RandomValue = 0;
    alt_set(5, 7, 2);
    CHECK(Bases[0].population_size == 3);
    CHECK(MapBaseSubmergedCount[2] == 1);
    CHECK((PlayersData[2].flags & 0x80000u) != 0);
    CHECK(Events[0] == 1 && Events[1] == 2 && Events[2] == 3 &&
          Events[3] == 5 && Events[4] == 4);

    reset_fixture();
    Tile.climate = static_cast<uint8_t>(3 << 5);
    Bases[0].x = 1; Bases[0].y = 2; Bases[0].faction_id_current = 2;
    Bases[0].population_size = 2; strcpy(Bases[0].name_string, "Delta");
    PlayersData[2].current_num_bases = 2; BaseAtResult = 0;
    VehCurrentCount = 2; StackHead = 0;
    Vehs[0].proto_id = 0; Vehs[0].faction_id = 2; Vehs[0].next_veh_id_stack = 1;
    Vehs[1].proto_id = 1; Vehs[1].faction_id = 2; Vehs[1].next_veh_id_stack = -1;
    VehPrototypes[0].chassis_id = 0; VehPrototypes[1].chassis_id = 1;
    Chassis[0].triad = TRIAD_LAND; Chassis[1].triad = TRIAD_SEA;
    RandomValue = 0;
    alt_set(1, 2, 2);
    CHECK(Bases[0].population_size <= 0);
    CHECK(Events[EventCount - 3] == 10); // veh_at follows base_kill
    CHECK(Events[EventCount - 2] == 11); // base_find before veh_kill
    CHECK(Events[EventCount - 1] == 12);
    CHECK(VehCurrentCount == 1 && Vehs[0].proto_id == 1);

    reset_fixture();
    Tile.climate = static_cast<uint8_t>(3 << 5);
    Bases[0].faction_id_current = 2; Bases[0].population_size = 2;
    PlayersData[2].current_num_bases = 1; BaseAtResult = 0;
    RandomValue = 0;
    alt_set(0, 0, 2);
    CHECK(Bases[0].population_size == 1);
    CHECK(EventCount > 0 && Events[EventCount - 1] == 8);

    reset_fixture();
    Tile.climate = static_cast<uint8_t>(3 << 5);
    Bases[0].faction_id_current = 2; Bases[0].population_size = 2;
    PlayersData[2].current_num_bases = 2; BaseAtResult = 0;
    ObjectiveResult = 1; RandomValue = 0;
    alt_set(0, 0, 2);
    CHECK(Bases[0].population_size == 1);
    CHECK(EventCount == 5 && Events[0] == 1 && Events[1] == 2 &&
          Events[2] == 3 && Events[3] == 5 && Events[4] == 4);

    reset_fixture();
    Tile.climate = static_cast<uint8_t>(3 << 5);
    Bases[0].faction_id_current = 2; Bases[0].population_size = 2;
    PlayersData[2].current_num_bases = 2; BaseAtResult = 0;
    IsMultiplayerNet = 1; ConsoleControlTurnActive = 0; RandomValue = 0;
    alt_set(0, 0, 2);
    CHECK(Bases[0].population_size <= 0);
    for (int i = 0; i < EventCount; ++i) CHECK(Events[i] != 6 && Events[i] != 7);
    CHECK(Events[EventCount - 2] == 9 && Events[EventCount - 1] == 10);

    reset_fixture();
    Tile.climate = static_cast<uint8_t>(3 << 5);
    Bases[0].faction_id_current = 2; Bases[0].population_size = 6;
    PlayersData[2].current_num_bases = 2; BaseAtResult = 0;
    WorldClimateSkipTerritoryReset = 1;
    alt_set(0, 0, 2);
    CHECK(Bases[0].population_size == 6 && MapBaseSubmergedCount[2] == 0);
    CHECK(EventCount == 1 && Events[0] == 1);
    reset_fixture(); BaseAtResult = -1; Tile.climate = 0x60;
    alt_set(0, 0, -1);
    CHECK((Tile.climate & 0xE0) == 0xE0);
    return 0;
}
