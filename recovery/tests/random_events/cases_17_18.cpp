#include <cassert>
#include <cstring>

enum { FAC_ENERGY_BANK = 7, FAC_NETWORK_NODE = 8, VK_SHIFT = 0x10 };

struct PlayerData {
    unsigned int flags;
    unsigned int ranking;
    int energy_reserves;
    int tech_accumulated;
    int net_random_event;
};
struct Base {
    short x, y;
    unsigned int facility_mask;
};
struct FacilityDef { int preq_tech; };
struct Map {
    unsigned char climate, contour, val2, region, visibility, val3, unk_1;
    signed char territory;
    unsigned int bit, bit2, bit_visible[7];
};

static PlayerData PlayersData[8];
static Base Bases[16];
static FacilityDef Facility[128];
static Map MapTiles[72];
static int RadiusOffsetX[289];
static int RadiusOffsetY[289];
static int TurnCurrentNum, MapLongitudeBounds, MapLatitudeBounds, MapLongitude;
static int MapIsFlat, IsMultiplayerNet, IsMultiplayerPBEM;
static int has_tech_result, territory_result, focus_result, shift_state;
static int tech_rate_result, energy_bank_checks, network_node_checks;
static int tech_checks, last_tech_preq, territory_checks, bit_set_calls, sync_calls, draw_calls;
static int radius_calls, wait_calls, wait_values[8], popup_calls;
static int popup_focus_x[8], popup_focus_y[8], popup_focus_faction[8];
static int popup_mutate_base;
static int popup_mutate_faction;
static char popup_key[32], popup_art[32];
static int popup_argument;

struct Console {
    int faction_id_;
    int focus(int x, int y, int faction) {
        const int index = focus_calls++;
        assert(index < 8);
        popup_focus_x[index] = x;
        popup_focus_y[index] = y;
        popup_focus_faction[index] = faction;
        if (popup_mutate_base && index == 0) {
            Bases[0].x = 11;
            Bases[0].y = 10;
        }
        if (popup_mutate_faction && index == 0) faction_id_ = 3;
        return focus_result;
    }
    int focus_calls;
};
static Console ConsoleGlobal;

static bool has_tech(int preq, int faction) {
    (void)faction; ++tech_checks; last_tech_preq = preq;
    return has_tech_result != 0;
}
static bool has_fac_built(int facility, int base_id) {
    if (facility == FAC_ENERGY_BANK) ++energy_bank_checks;
    if (facility == FAC_NETWORK_NODE) ++network_node_checks;
    return (Bases[base_id].facility_mask & (1u << facility)) != 0;
}
static int whose_territory(int faction, int x, int y, int *base, int flags) {
    (void)faction; (void)x; (void)y; (void)base; (void)flags;
    ++territory_checks; return territory_result;
}
static Map *map_tiles() { return MapTiles; }
static void bit_set(int x, int y, int mask, int value) {
    ++bit_set_calls;
    Map &tile = MapTiles[MapLongitude * y + (x >> 1)];
    if (value) tile.bit |= static_cast<unsigned int>(mask);
    else tile.bit &= ~static_cast<unsigned int>(mask);
}
static void synch_bit(int x, int y, int faction) {
    (void)x; (void)y; (void)faction; ++sync_calls;
}
static void draw_tile(int x, int y, int mode) {
    (void)x; (void)y; (void)mode; ++draw_calls;
}
static void draw_radius(int x, int y, int radius, int mode) {
    (void)x; (void)y; (void)radius; (void)mode; ++radius_calls;
}
static int GetAsyncKeyState(int key) { (void)key; return shift_state; }
static void clock_wait(int ms) {
    assert(wait_calls < 8); wait_values[wait_calls++] = ms;
}
static int tech_rate(int faction) { (void)faction; return tech_rate_result; }
static void popup(const char *key, const char *art, int argument) {
    ++popup_calls;
    std::strcpy(popup_key, key);
    std::strcpy(popup_art, art);
    popup_argument = argument;
}
#define POP2(key, art, argument) popup((key), (art), (argument))

#include "actual_cases_17_18.inc"

static void reset_fixture() {
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(Facility, 0, sizeof(Facility));
    std::memset(MapTiles, 0, sizeof(MapTiles));
    std::memset(RadiusOffsetX, 0, sizeof(RadiusOffsetX));
    std::memset(RadiusOffsetY, 0, sizeof(RadiusOffsetY));
    TurnCurrentNum = 100;
    MapLongitudeBounds = 12;
    MapLatitudeBounds = 12;
    MapLongitude = 6;
    MapIsFlat = 1;
    IsMultiplayerNet = 0;
    IsMultiplayerPBEM = 0;
    Facility[FAC_ENERGY_BANK].preq_tech = 107;
    Facility[FAC_NETWORK_NODE].preq_tech = 108;
    has_tech_result = 1;
    territory_result = -1;
    focus_result = 0;
    shift_state = 0;
    tech_rate_result = 8;
    energy_bank_checks = network_node_checks = tech_checks = 0;
    territory_checks = bit_set_calls = sync_calls = draw_calls = 0;
    radius_calls = wait_calls = popup_calls = 0;
    ConsoleGlobal.focus_calls = 0;
    popup_mutate_base = 0;
    popup_mutate_faction = 0;
    popup_key[0] = popup_art[0] = 0;
    popup_argument = -1;
    ConsoleGlobal.faction_id_ = 2;
    Bases[0].x = 9;
    Bases[0].y = 10;
}

static void prepare_surge_tile(int x, int y, int faction) {
    for (int i = 1; i < 21; ++i) RadiusOffsetX[i] = 100;
    Map &tile = MapTiles[MapLongitude * y + (x >> 1)];
    tile.climate = 0x60;
    tile.visibility = static_cast<unsigned char>(1u << faction);
    tile.bit = 0x10;
    for (int i = 0; i < 7; ++i) tile.bit_visible[i] = 0x10;
}

int main() {
    // Case 17: missing prerequisite must be inert.
    reset_fixture();
    has_tech_result = 0;
    random_events_cases_17_18(17, 2, 0, 37, 4, 4);
    assert(tech_checks == 1 && bit_set_calls == 0 && popup_calls == 0);
    assert(last_tech_preq == 107);

    // Rank gate is 4 before turn 150; no site mutation when below it.
    reset_fixture();
    PlayersData[2].ranking = 3;
    random_events_cases_17_18(17, 2, 0, 37, 4, 4);
    assert(energy_bank_checks == 1 && territory_checks == 0 && popup_calls == 0);

    // Energy Surge uses the pre-parser coordinate snapshot, one-wrap/radius
    // cell access, visibility clear, side-effect ordering, and fallback draw.
    reset_fixture();
    PlayersData[2].ranking = 4;
    prepare_surge_tile(4, 4, 2);
    popup_mutate_base = 1;
    random_events_cases_17_18(17, 2, 0, 37, 4, 4);
    assert(territory_checks == 1 && bit_set_calls == 1 && sync_calls == 1);
    assert(draw_calls == 1 && radius_calls == 1);
    assert(MapTiles[MapLongitude * 4 + 2].bit == 0);
    for (int i = 0; i < 7; ++i)
        assert(MapTiles[MapLongitude * 4 + 2].bit_visible[i] == 0);
    assert(ConsoleGlobal.focus_calls == 2 && popup_focus_x[0] == 4 && popup_focus_y[0] == 4);
    assert(popup_focus_x[1] == 11 && popup_focus_y[1] == 10);
    assert(popup_focus_faction[0] == 2 && popup_focus_faction[1] == 2);
    assert(wait_calls == 2 && wait_values[0] == 200 && wait_values[1] == 50);
    assert(std::strcmp(popup_key, "SURGE") == 0);
    assert(std::strcmp(popup_art, "markbm_sm.pcx") == 0 && popup_argument == 37);

    // Unsuitable terrain, missing faction visibility, and enemy territory
    // each reject a tile before the mutation path.
    reset_fixture();
    PlayersData[2].ranking = 4;
    prepare_surge_tile(4, 4, 2);
    MapTiles[MapLongitude * 4 + 2].climate = 0x40;
    random_events_cases_17_18(17, 2, 0, 37, 4, 4);
    assert(bit_set_calls == 0 && popup_calls == 0);
    reset_fixture();
    PlayersData[2].ranking = 4;
    prepare_surge_tile(4, 4, 2);
    MapTiles[MapLongitude * 4 + 2].visibility = 0;
    random_events_cases_17_18(17, 2, 0, 37, 4, 4);
    assert(territory_checks == 0 && bit_set_calls == 0 && popup_calls == 0);
    reset_fixture();
    PlayersData[2].ranking = 4;
    prepare_surge_tile(4, 4, 2);
    territory_result = 3;
    random_events_cases_17_18(17, 2, 0, 37, 4, 4);
    assert(territory_checks == 1 && bit_set_calls == 0 && popup_calls == 0);

    // Energy Bank changes both the +50 reserve mutation and popup variant;
    // foreign factions mutate the event before the PBEM display gate.
    reset_fixture();
    Bases[0].facility_mask = 1u << FAC_ENERGY_BANK;
    PlayersData[3].energy_reserves = 20;
    IsMultiplayerPBEM = 1;
    ConsoleGlobal.faction_id_ = 2;
    prepare_surge_tile(4, 4, 3);
    random_events_cases_17_18(17, 3, 0, 41, 4, 4);
    assert(PlayersData[3].energy_reserves == 70);
    assert(std::strcmp(popup_key, "SURGE1") == 0 && popup_calls == 1);
    assert(bit_set_calls == 0 && sync_calls == 0 && draw_calls == 0 && wait_calls == 0);

    // Foreign Energy Surge outside PBEM mutates terrain/energy but suppresses
    // the popup, matching the late native multiplayer gate.
    reset_fixture();
    Bases[0].facility_mask = 1u << FAC_ENERGY_BANK;
    PlayersData[3].energy_reserves = 20;
    prepare_surge_tile(4, 4, 3);
    random_events_cases_17_18(17, 3, 0, 41, 4, 4);
    assert(PlayersData[3].energy_reserves == 70 && popup_calls == 0);

    // Focus can reenter and change the active local faction; the native
    // routine reloads it after focus and applies the foreign-PBEM gate.
    reset_fixture();
    PlayersData[2].ranking = 4;
    prepare_surge_tile(4, 4, 2);
    popup_mutate_faction = 1;
    random_events_cases_17_18(17, 2, 0, 41, 4, 4);
    assert(bit_set_calls == 1 && popup_calls == 0);

    // Case 18: Network Node supplies a quarter-progress threshold and then
    // restores research to the complete native tech_rate value.
    reset_fixture();
    PlayersData[2].tech_accumulated = 2;
    tech_rate_result = 8;
    Bases[0].facility_mask = 1u << FAC_NETWORK_NODE;
    random_events_cases_17_18(18, 2, 0, 19, 4, 4);
    assert(last_tech_preq == 108);
    assert(PlayersData[2].tech_accumulated == 8);
    assert(popup_calls == 1 && std::strcmp(popup_key, "NETBONUS") == 0);
    assert(std::strcmp(popup_art, "markbm_sm.pcx") == 0 && popup_argument == 19);
    assert(ConsoleGlobal.focus_calls == 1 && popup_focus_x[0] == 9 && popup_focus_y[0] == 10);
    assert(popup_focus_faction[0] == 2);

    // A focus callback can change the active faction before the native
    // post-focus multiplayer test; research is updated, popup is suppressed.
    reset_fixture();
    PlayersData[2].tech_accumulated = 2;
    Bases[0].facility_mask = 1u << FAC_NETWORK_NODE;
    popup_mutate_faction = 1;
    random_events_cases_17_18(18, 2, 0, 19, 4, 4);
    assert(PlayersData[2].tech_accumulated == 8 && popup_calls == 0);

    // A failed progress threshold is inert even with a Network Node.
    reset_fixture();
    PlayersData[2].tech_accumulated = 1;
    Bases[0].facility_mask = 1u << FAC_NETWORK_NODE;
    random_events_cases_17_18(18, 2, 0, 19, 4, 4);
    assert(PlayersData[2].tech_accumulated == 1 && popup_calls == 0);

    // The native cdq/and/add/sar threshold rounds negative rates upward;
    // keep that signed edge distinct from `(rate + 3) / 4` in C++.
    reset_fixture();
    tech_rate_result = -5;
    PlayersData[2].tech_accumulated = -1;
    Bases[0].facility_mask = 1u << FAC_NETWORK_NODE;
    random_events_cases_17_18(18, 2, 0, 19, 4, 4);
    assert(PlayersData[2].tech_accumulated == -5 && popup_calls == 1);

    // Without a Network Node, the ranking gate controls the crash branch;
    // accepted crash clears both fields before notification.
    reset_fixture();
    PlayersData[2].ranking = 4;
    PlayersData[2].tech_accumulated = 2;
    PlayersData[2].net_random_event = 1;
    random_events_cases_17_18(18, 2, 0, 23, 4, 4);
    assert(PlayersData[2].tech_accumulated == 0 &&
           PlayersData[2].net_random_event == 0);
    assert(popup_calls == 1 && std::strcmp(popup_key, "NETCRASH") == 0);
    assert(std::strcmp(popup_art, "netcr_sm.pcx") == 0 && popup_argument == 23);

    // Turn-150 threshold rises to rank 5. This reject must leave state intact.
    reset_fixture();
    TurnCurrentNum = 150;
    PlayersData[2].ranking = 4;
    PlayersData[2].tech_accumulated = 2;
    PlayersData[2].net_random_event = 1;
    random_events_cases_17_18(18, 2, 0, 23, 4, 4);
    assert(PlayersData[2].tech_accumulated == 2 &&
           PlayersData[2].net_random_event == 1 && popup_calls == 0);

    // The original performs state changes before the foreign non-PBEM
    // notification gate for both Net Node outcomes.
    reset_fixture();
    PlayersData[3].tech_accumulated = 2;
    PlayersData[3].net_random_event = 1;
    PlayersData[3].ranking = 4;
    random_events_cases_17_18(18, 3, 0, 23, 4, 4);
    assert(PlayersData[3].tech_accumulated == 0 &&
           PlayersData[3].net_random_event == 0 && popup_calls == 0);

    return 0;
}

