// Host control for the source-owner adapter's shape and its 9..11 dispatch.
#include <assert.h>
#include <stdint.h>
#include <string.h>

struct PlayerData { int satellites_[4]; int ranking; };
struct RulesResource { const char *name_singular; const char *name_plural; };
struct RulesFacility { const char *name; };
struct Base {
    int faction_id_current, population_size, visibility, x, y;
    unsigned char facilities_built[12];
};
struct Map { unsigned int visibility; unsigned char climate; };
struct FixtureMapWin {
    int centers, last_x, last_y, last_animate;
    FixtureMapWin() : centers(0), last_x(0), last_y(0), last_animate(0) {}
    void set_center(int x, int y, int animate) {
        ++centers;
        last_x = x;
        last_y = y;
        last_animate = animate;
    }
};
struct FixtureConsole {
    int faction_id_;
    int focus(int, int, int) { return 0; }
};
static RulesResource Resource[4];
static RulesFacility Facility[134];
static Map FixtureTile;
static FixtureMapWin PrimaryMapWindow;
static FixtureMapWin FarmMapWindow;
static FixtureMapWin *MapWinTable[8];
static FixtureMapWin *&console_map_win() { return MapWinTable[0]; }
static FixtureConsole ConsoleGlobal;
static PlayerData PlayersData[8];
static Base Bases[8];
static int GlobalEnergyEventState;
static int SunspotDuration;
static int TurnCurrentNum;
static int IsMultiplayerPBEM;
static int MapLongitudeBounds, MapLongitude, MapIsFlat, BaseCurrentCount;
static int ProjectOwners[32];
static const int MaxPlayerNum = 8;
static int random_values[8], random_count, random_index;
static int parse_calls, popup_calls, popup_arg;
static char popup_caption[32], popup_image[32];

static int random(int low, int high) {
    assert(low == 0 && (high == 3 || high == 5 || high == 11));
    assert(random_index < random_count);
    return random_values[random_index++];
}
static int parse_num(int low, int high) {
    ++parse_calls;
    return high - low;
}
static int parse_say(int, int, int, int) { return 0; }
static bool has_fac_built(int facility_id, int base_id) {
    const int offset = facility_id / 8;
    const int mask = 1 << (facility_id & 7);
    return (Bases[base_id].facilities_built[offset] & mask) != 0;
}
static int base_project(int slot) { return ProjectOwners[slot]; }
static int bonus_at(int, int, int) { return 0; }
static int whose_territory(int, int, int, int *, int) { return -1; }
static Map *map_loc(int, int) { return &FixtureTile; }
static void bit_set(int, int, int, int) {}
static void synch_bit(int, int, int) {}
static void draw_tile(int, int, int) {}
static void clock_wait(int) {}
static short GetAsyncKeyState(int key) { return (short)key; }
static int IsMultiplayerNet, NetBusyCount;
static int POP2(const char *caption, const char *image, int argument) {
    strcpy(popup_caption, caption);
    strcpy(popup_image, image);
    popup_arg = argument;
    ++popup_calls;
    return 0;
}

#include "actual_cases_9_11.hpp"
#include "actual_case6.hpp"

#include "actual_native_services.inc"

static void reset() {
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Bases, 0, sizeof(Bases));
    memset(ProjectOwners, 0xFF, sizeof(ProjectOwners));
    GlobalEnergyEventState = 0;
    SunspotDuration = -40;
    TurnCurrentNum = 50;
    IsMultiplayerPBEM = 0;
    MapLongitudeBounds = 128; MapLongitude = 64; MapIsFlat = 1;
    BaseCurrentCount = 0; ConsoleGlobal.faction_id_ = 1;
    PrimaryMapWindow = FixtureMapWin();
    FarmMapWindow = FixtureMapWin();
    MapWinTable[0] = &PrimaryMapWindow;
    random_count = random_index = parse_calls = popup_calls = 0;
    popup_caption[0] = popup_image[0] = '\0';
    popup_arg = 0;
}

static void test_position_adapter_centers_primary_not_farm_window() {
    reset();
    RandomEventsNativePositionServices services;
    services.center_map(17, 23, 1);
    assert(PrimaryMapWindow.centers == 1);
    assert(PrimaryMapWindow.last_x == 17 && PrimaryMapWindow.last_y == 23);
    assert(PrimaryMapWindow.last_animate == 1);
    assert(FarmMapWindow.centers == 0);
}

static void test_position_wait_gate_binds_multiplayer_net_not_busy_count() {
    reset();
    RandomEventsNativePositionServices services;
    NetBusyCount = 7;
    IsMultiplayerNet = 0;
    assert(services.is_multiplayer_net() == 0);
    NetBusyCount = 0;
    IsMultiplayerNet = 1;
    assert(services.is_multiplayer_net() == 1);
}

static void test_case9_actual_global_and_players() {
    reset();
    TurnCurrentNum = 0;
    random_values[random_count++] = 0;
    PlayersData[1].satellites_[2] = 1;
    random_events_native_cases_9_11(9);
    assert(GlobalEnergyEventState == 1);
    assert(PlayersData[1].satellites_[2] == 0);
    assert(popup_calls == 1);
    assert(strcmp(popup_caption, "SOLARSTORM") == 0);
    assert(strcmp(popup_image, "sun_sm.pcx") == 0);
}

static void test_case10_only_notifies_on_change() {
    reset();
    TurnCurrentNum = 0;
    random_values[random_count++] = 0;
    PlayersData[1].satellites_[1] = 1;
    random_events_native_cases_9_11(10);
    assert(GlobalEnergyEventState == 1);
    assert(PlayersData[1].satellites_[1] == 0);
    assert(popup_calls == 1);
    assert(strcmp(popup_caption, "ASTEROID") == 0);
    assert(strcmp(popup_image, "astm_sm.pcx") == 0);
}

static void test_case9_unchanged_message() {
    reset();
    random_values[random_count++] = 0;
    random_events_native_cases_9_11(9);
    assert(GlobalEnergyEventState == 1);
    assert(popup_calls == 1);
    assert(strcmp(popup_caption, "SOLARFLARE") == 0);
    assert(strcmp(popup_image, "heat_sm.pcx") == 0);
}

static void test_case11_store_parse_popup_order_result() {
    reset();
    SunspotDuration = -40;
    random_values[random_count++] = 3;
    random_events_native_cases_9_11(11);
    assert(SunspotDuration == 13);
    assert(parse_calls == 1 && popup_calls == 1);
    assert(strcmp(popup_caption, "SUNSPOTS") == 0);
    assert(strcmp(popup_image, "solar_sm.pcx") == 0);
    assert(popup_arg == -1);
}

static void test_case6_native_owner_bindings() {
    reset();
    TurnCurrentNum = 100;
    MapLongitudeBounds = 100; MapLongitude = 50; MapIsFlat = 1;
    BaseCurrentCount = 2;
    ConsoleGlobal.faction_id_ = 1;
    PlayersData[2].ranking = 4;
    Bases[0].x = 10; Bases[0].y = 10;
    Bases[0].faction_id_current = 2; Bases[0].population_size = 6;
    Bases[0].visibility = 1u << 1;
    Bases[1].x = 12; Bases[1].y = 10;
    Bases[1].faction_id_current = 1; Bases[1].population_size = 7;
    RandomEventsNativeCase6State state;
    RandomEventsNativeCase6Services services;
    random_events_cases::case6(state, services, 2, 0, 9);
    assert(Bases[0].population_size == 3);
    assert(Bases[1].population_size == 4);
    assert(popup_calls == 1);
    assert(strcmp(popup_caption, "PROMETHEUS0") == 0);
    assert(strcmp(popup_image, "biohazd_sm.pcx") == 0);
}

int main() {
    test_position_adapter_centers_primary_not_farm_window();
    test_position_wait_gate_binds_multiplayer_net_not_busy_count();
    test_case9_actual_global_and_players();
    test_case10_only_notifies_on_change();
    test_case9_unchanged_message();
    test_case11_store_parse_popup_order_result();
    test_case6_native_owner_bindings();
}

