#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl

static const int ALT_BIT_SHORE_LINE = 0x60;
static const int TRIAD_LAND = 0;
static const int ORDER_NONE = 0;

struct PlayerData {
    int traded_maps;
    uint8_t proto_id_active[512];
    uint8_t proto_id_queue[512];
};
struct Map {
    uint8_t climate;
    uint8_t contour;
    uint8_t val2;
    uint8_t region;
    uint8_t visibility;
    uint8_t val3;
    uint8_t unk_1;
    int8_t territory;
    uint32_t bit;
    uint32_t bit2;
    uint32_t bit_visible[7];
};
struct Base {
    int16_t x;
    int16_t y;
    uint8_t faction_id_current;
    uint8_t faction_id_former;
    int8_t population_size;
    uint8_t assimilation_turns_left;
    uint8_t nerve_staple_turns_left;
    uint8_t unk_1;
    uint8_t visibility;
};
struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t unk_1;
    uint8_t faction_id;
    uint8_t year_end_lurking;
    uint8_t dmg_incurred;
    int8_t order;
    uint8_t waypoint_count;
    uint8_t patrol_current_point;
    int16_t waypoint_x[4];
    int16_t waypoint_y[4];
    uint8_t morale;
    uint8_t terraforming_turns;
    uint8_t order_auto_type;
    uint8_t visibility;
    uint8_t moves_expended;
    int8_t facing;
    uint8_t unk_6;
    uint8_t move_to_ai_type;
    uint8_t probe_action;
    uint8_t probe_sabotage_id;
    int16_t home_base_id;
};
struct VehPrototype { uint8_t armor_id, chassis_id; };
struct RulesChassis { uint8_t triad; };
struct Console { int faction_id_; };
struct GraphicWin { int redraw_count; void redraw() { ++redraw_count; } };
struct WorldWindow : GraphicWin {};

static PlayerData PlayersData[8];
static Map MapTiles[512];
static Base Bases[32];
static Veh Vehs[32];
static VehPrototype VehPrototypes[32];
static RulesChassis Chassis[32];
struct GameDataFields { int base_current_count_; int vehicle_current_count_; };
struct GameDataState { GameDataFields fields_; };
static GameDataState GameDataStateStorage;
static uint32_t MapArea;
static uint32_t MapLongitude;
static int MapLongitudeBounds;
static int MapLatitudeBounds;
static int MapIsFlat;
static uint32_t UnkBitfield1;
static int DiplomacyMapTransferChanged;
static Console ConsoleGlobal;
static WorldWindow WorldClimateWorldWin;
static int VehMoveDeltaX[9] = {1, 2, 1, 0, -1, -2, -1, 0, 0};
static int VehMoveDeltaY[9] = {-1, 0, 1, 2, 1, 0, -1, -2, 0};

static int spot_calls;
static int spot_base_ids[32];
static int spot_factions[32];
static int grow_base_count_on_spot;
static int expected_stack_faction;
static int last_veh_at_x, last_veh_at_y;
static int drop_calls;
static int change_vehicle_count_on_drop;
static int vehicle_count_after_drop;
static int mutate_proto_on_drop;
static int mutate_proto_slot;
static int proto_after_drop;
static int last_ability_proto;
static int base_find_result;
static int agenda_calls, treaty_calls, tile_fixup_calls, tile_draw_calls;
static int last_agenda[4], last_treaty[4];
static int last_tile_fixup[4];
static int destination_x, destination_y;
static int stack_check_calls;

void __cdecl spot_base(int base_id, int faction_id) {
    if (spot_calls < 32) {
        spot_base_ids[spot_calls] = base_id;
        spot_factions[spot_calls] = faction_id;
    }
    ++spot_calls;
    if (grow_base_count_on_spot && spot_calls == 1)
        GameDataStateStorage.fields_.base_current_count_ += grow_base_count_on_spot;
}
void __cdecl draw_map(int) {}
int __cdecl veh_cargo(int) { return 1; }
int __cdecl veh_at(int x, int y) {
    last_veh_at_x = x;
    last_veh_at_y = y;
    return 9;
}
int __cdecl stack_fix(int id) { return id; }
int __cdecl stack_check(int, int, int faction, int, int) {
    ++stack_check_calls;
    if (faction != expected_stack_faction) return 0;
    return last_veh_at_x == destination_x && last_veh_at_y == destination_y;
}
int __cdecl veh_lift(int id) { return id + 2; }
int __cdecl veh_drop(int id, int x, int y) {
    ++drop_calls;
    Vehs[id].x = static_cast<int16_t>(x);
    Vehs[id].y = static_cast<int16_t>(y);
    if (mutate_proto_on_drop) Vehs[mutate_proto_slot].proto_id = proto_after_drop;
    if (change_vehicle_count_on_drop) GameDataStateStorage.fields_.vehicle_current_count_ = vehicle_count_after_drop;
    return id;
}
void __cdecl draw_tiles(int, int, int) { ++tile_draw_calls; }
int __cdecl base_find(int, int, int) { return base_find_result; }
int __cdecl has_abil(int proto, int) { last_ability_proto = proto; return 3; }
void __cdecl draw_tile_fixup(int x, int y, int ability, int mode) {
    ++tile_fixup_calls;
    last_tile_fixup[0] = x;
    last_tile_fixup[1] = y;
    last_tile_fixup[2] = ability;
    last_tile_fixup[3] = mode;
}
void __cdecl set_agenda(int faction, int other, int flags, int set) {
    ++agenda_calls;
    last_agenda[0] = faction; last_agenda[1] = other;
    last_agenda[2] = flags; last_agenda[3] = set;
}
void __cdecl set_treaty(int faction, int other, int flags, int set) {
    ++treaty_calls;
    last_treaty[0] = faction; last_treaty[1] = other;
    last_treaty[2] = flags; last_treaty[3] = set;
}

// Extracted verbatim from the actual src/faction.cpp definitions.
@@TRADE_MAPS@@
@@DO_WITHDRAWAL@@

static void reset_all() {
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(MapTiles, 0, sizeof(MapTiles));
    memset(Bases, 0, sizeof(Bases));
    memset(Vehs, 0, sizeof(Vehs));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Chassis, 0, sizeof(Chassis));
    GameDataStateStorage.fields_.base_current_count_ = GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    MapArea = MapLongitude = 0;
    MapLongitudeBounds = MapLatitudeBounds = MapIsFlat = 0;
    UnkBitfield1 = 0;
    ConsoleGlobal.faction_id_ = -1;
    WorldClimateWorldWin.redraw_count = 0;
    spot_calls = 0;
    grow_base_count_on_spot = 0;
    drop_calls = tile_fixup_calls = tile_draw_calls = 0;
    agenda_calls = treaty_calls = stack_check_calls = 0;
    expected_stack_faction = -1;
    destination_x = destination_y = -100;
    change_vehicle_count_on_drop = 0;
    mutate_proto_on_drop = 0;
    mutate_proto_slot = 0;
    proto_after_drop = -1;
    last_ability_proto = -1;
    base_find_result = -1;
}

static void control_trade_direction_and_visibility() {
    int actor, partner;
    for (actor = 1; actor <= 7; ++actor) {
        for (partner = 1; partner <= 7; ++partner) {
            if (actor == partner) continue;
            reset_all();
            MapArea = 1;
            PlayersData[actor].traded_maps = 0x20;
            PlayersData[partner].traded_maps = 0x04;
            MapTiles[0].bit = 0x5A5A00FFu;
            MapTiles[0].visibility = static_cast<uint8_t>(1u << partner);
            MapTiles[0].bit_visible[actor - 1] = 0xA5A500FFu;
            MapTiles[0].bit_visible[partner - 1] = 0x333300FFu;
            trade_maps(actor, partner);
            const uint32_t combined = 0x20u | (1u << partner) | 0x04u;
            assert(static_cast<uint32_t>(PlayersData[actor].traded_maps) == combined);
            assert(static_cast<uint32_t>(PlayersData[partner].traded_maps) ==
                   (0x04u | (1u << actor) | combined));
            assert(MapTiles[0].visibility ==
                   static_cast<uint8_t>((1u << partner) | (1u << actor)));
            assert(MapTiles[0].bit_visible[actor - 1] ==
                   (0xA5A500FFu | (0x333300FFu & MapTiles[0].bit)));
            assert(MapTiles[0].bit_visible[partner - 1] == 0x333300FFu);
            assert(DiplomacyMapTransferChanged == 1);
        }
    }

    // The other native branch copies actor visibility into the partner slot.
    reset_all();
    MapArea = 1;
    MapTiles[0].bit = 0x0000FFFFu;
    MapTiles[0].visibility = 1u << 3;
    MapTiles[0].bit_visible[2] = 0x0010FFFFu;
    MapTiles[0].bit_visible[5] = 0x0020000Fu;
    trade_maps(3, 6);
    assert(MapTiles[0].bit_visible[5] == (0x0020000Fu | (0x0010FFFFu & 0x0000FFFFu)));
    assert(MapTiles[0].bit_visible[2] == 0x0010FFFFu);

    // Faction zero participates in the visibility mask, but native explicitly
    // excludes faction-zero word transfers because +0x0C overlays bit2.
    reset_all();
    MapArea = 1;
    MapTiles[0].bit = 0xFFFFFFFFu;
    MapTiles[0].visibility = 1u << 2;
    for (int i = 0; i < 7; ++i) MapTiles[0].bit_visible[i] = 0x12340000u + i;
    trade_maps(0, 2);
    for (int i = 0; i < 7; ++i) assert(MapTiles[0].bit_visible[i] == 0x12340000u + i);
    assert(MapTiles[0].visibility == 5);

    // The first base count is snapshotted; callback growth is not rescanned.
    reset_all();
    GameDataStateStorage.fields_.base_current_count_ = 3;
    Bases[0].visibility = 3; // actor-known wins and spots the partner
    Bases[1].visibility = 4; // partner-known spots the actor
    Bases[2].faction_id_current = 1; // actor-owned also spots the partner
    Bases[3].visibility = 2; // callback-added actor-known record is scanned live
    Bases[4].faction_id_current = 1;
    grow_base_count_on_spot = 2;
    MapArea = 0;
    trade_maps(1, 2);
    assert(spot_calls == 5);
    assert(spot_base_ids[0] == 0 && spot_factions[0] == 2);
    assert(spot_base_ids[1] == 1 && spot_factions[1] == 1);
    assert(spot_base_ids[2] == 2 && spot_factions[2] == 2);
    assert(spot_base_ids[3] == 3 && spot_factions[3] == 2);
    assert(spot_base_ids[4] == 4 && spot_factions[4] == 2);

    reset_all();
    MapArea = 1;
    ConsoleGlobal.faction_id_ = 2;
    trade_maps(1, 2);
    assert(UnkBitfield1 == 4);
    assert(WorldClimateWorldWin.redraw_count == 1);
}

static void control_withdrawal_moves_cleans_and_updates_treaty() {
    reset_all();
    MapLongitude = 8;
    MapLongitudeBounds = 16;
    MapLatitudeBounds = 6;
    GameDataStateStorage.fields_.vehicle_current_count_ = 2;
    Vehs[0].x = 4; Vehs[0].y = 2;
    Vehs[0].faction_id = 2; Vehs[0].proto_id = 0;
    Vehs[0].state = 0xFFFFFFFFu; Vehs[0].order = 7; Vehs[0].visibility = 0xFF;
    Vehs[1] = Vehs[0];
    Vehs[1].x = 4; Vehs[1].y = 2;
    VehPrototypes[0].armor_id = 0;
    VehPrototypes[0].chassis_id = 0;
    Chassis[0].triad = TRIAD_LAND;
    const int origin = static_cast<int>(MapLongitude * 2 + (4 >> 1));
    MapTiles[origin].climate = 0x80;
    MapTiles[origin].territory = 1;
    destination_x = 5; destination_y = 1; // RadiusBase index 0
    const int target = static_cast<int>(MapLongitude * destination_y + (destination_x >> 1));
    MapTiles[target].climate = 0x20; // sea destination admits stack_check
    expected_stack_faction = 2;
    change_vehicle_count_on_drop = 1;
    vehicle_count_after_drop = 1;

    const int moved = do_withdrawal(1, 2);
    assert(moved == 1);
    assert(drop_calls == 1); // live owner vehicle-count reload stops second record
    assert(GameDataStateStorage.fields_.vehicle_current_count_ == 1);
    assert(Vehs[0].x == 4 && Vehs[0].y == 2); // drop uses lift return, not source id
    assert(Vehs[1].x == 4 && Vehs[1].y == 2);
    assert(Vehs[2].x == destination_x && Vehs[2].y == destination_y);
    assert(Vehs[0].visibility == 0);
    assert(Vehs[0].order == ORDER_NONE);
    assert(Vehs[0].state == (0xFFFFFFFFu & 0xFCFFBDFFu));
    assert(tile_draw_calls == 1);
    assert(agenda_calls == 1 && last_agenda[0] == 2 && last_agenda[1] == 1 &&
           last_agenda[2] == 2 && last_agenda[3] == 1);
    assert(treaty_calls == 1 && last_treaty[0] == 2 && last_treaty[1] == 1 &&
           last_treaty[2] == 0x100000 && last_treaty[3] == 1);
}

static void control_withdrawal_base_callback_reads() {
    reset_all();
    MapLongitude = 8;
    MapLongitudeBounds = 16;
    MapLatitudeBounds = 6;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    Vehs[0].x = 4; Vehs[0].y = 2;
    Vehs[0].faction_id = 2; Vehs[0].proto_id = 0;
    Vehs[0].state = 0xFFFFFFFFu; Vehs[0].order = 7; Vehs[0].visibility = 0xFF;
    VehPrototypes[0].armor_id = 0;
    VehPrototypes[0].chassis_id = 1;
    Chassis[1].triad = 1; // skip the neighbor-move branch
    const int origin = static_cast<int>(MapLongitude * 2 + (4 >> 1));
    MapTiles[origin].climate = 0x80;
    MapTiles[origin].territory = 1;
    Bases[0].x = 10; Bases[0].y = 3;
    base_find_result = 0;
    mutate_proto_on_drop = 1;
    mutate_proto_slot = 0;
    proto_after_drop = 7;

    const int moved = do_withdrawal(1, 2);
    assert(moved == 1 && drop_calls == 1);
    assert(Vehs[0].x == 4 && Vehs[0].y == 2);
    assert(Vehs[2].x == 10 && Vehs[2].y == 3);
    assert(Vehs[0].visibility == 0 && Vehs[0].order == ORDER_NONE);
    assert(Vehs[0].state == (0xFFFFFFFFu & 0xFCFFBDFFu));
    assert(Vehs[0].proto_id == 7);
    assert(last_ability_proto == 7); // has_abil reads the post-drop vehicle field
    assert(tile_fixup_calls == 1 && last_tile_fixup[2] == 3);
    assert(agenda_calls == 1 && treaty_calls == 1);
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    if (strcmp(argv[1], "trade") == 0) {
        control_trade_direction_and_visibility();
        return 0;
    }
    if (strcmp(argv[1], "withdraw") == 0) {
        control_withdrawal_moves_cleans_and_updates_treaty();
        control_withdrawal_base_callback_reads();
        return 0;
    }
    return 2;
}
