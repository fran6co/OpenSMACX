#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <string>

enum { BIT_SOLAR_TIDAL = 0x40, VK_SHIFT = 0x10 };

struct PlayerData {
    int flags;
    int ranking;
    int energy_reserves;
};
struct Base {
    int x;
    int y;
    unsigned char faction_id_current;
    unsigned char visibility;
};
struct Map {
    unsigned char climate;
    unsigned char visibility;
    uint32_t bit;
    uint32_t bit_visible[7];
};
static int focus_expected_x;
static int focus_expected_y;
struct Console {
    int faction_id_;
    std::string *log;
    void update_data(int value) {
        assert(value == 0);
        log->push_back('U');
    }
    int focus(int x, int y, int faction) {
        assert(x == focus_expected_x && y == focus_expected_y &&
               faction == faction_id_);
        log->push_back('F');
        return 0;
    }
};

static PlayerData PlayersData[8];
static Base Bases[8];
static Map MapStore[32 * 16];
static Map *MapTiles = MapStore;
static const int RadiusOffsetX[21] = {
    0, 1, 2, 1, 0, -1, -2, -1, 0, 2, 2, -2, -2, 1, 3, 3, 1, -1, -3, -3, -1
};
static const int RadiusOffsetY[21] = {
    0, -1, 0, 1, 2, 1, 0, -1, -2, -2, 2, 2, -2, -3, -1, 1, 3, 3, 1, -1, -3
};
static unsigned int LockEnableMask;
static int TurnCurrentNum;
static int MapIsFlat;
static int MapLongitudeBounds;
static int MapLatitudeBounds;
static unsigned int MapLongitude;
static int IsMultiplayerNet;
static int IsMultiplayerPBEM;
static int shift_results[8];
static int shift_result_count;
static int shift_calls;
static int territory_result;
static int territory_calls;
static int parse_calls;
static int parse_channel;
static int parse_value;
static int popup_calls;
static char popup_title[32];
static char popup_image[32];
static int popup_value;
static std::string call_log;
static Console ConsoleGlobal;
static Map *map_tiles() { return MapTiles; }

static int parse_num(int channel, int value) {
    ++parse_calls;
    parse_channel = channel;
    parse_value = value;
    call_log.push_back('P');
    return value;
}
static int POP2(const char *title, const char *image, int value) {
    ++popup_calls;
    strcpy(popup_title, title);
    strcpy(popup_image, image);
    popup_value = value;
    call_log.push_back('D');
    return 0;
}
static int whose_territory(int, int, int, int *base_id, int *other) {
    assert(base_id == 0 && other == 0);
    ++territory_calls;
    call_log.push_back('T');
    return territory_result;
}
static void bit_set(int x, int y, int mask, int enabled) {
    Map &tile = MapTiles[y * static_cast<int>(MapLongitude) + (x >> 1)];
    if (enabled) tile.bit |= static_cast<uint32_t>(mask);
    else tile.bit &= ~static_cast<uint32_t>(mask);
    call_log.push_back('B');
}
static void synch_bit(int x, int y, int faction) {
    Map &tile = MapTiles[y * static_cast<int>(MapLongitude) + (x >> 1)];
    if (faction) tile.bit_visible[faction - 1] = tile.bit;
    call_log.push_back('S');
}
static void draw_tile(int, int, int type) {
    assert(type == 2);
    call_log.push_back('R');
}
static void clock_wait(int millis) {
    assert(millis == 50 || millis == 200);
    call_log.push_back(millis == 200 ? 'W' : 'w');
}
static int GetAsyncKeyState(int key) {
    assert(key == VK_SHIFT);
    assert(shift_calls < shift_result_count);
    call_log.push_back('K');
    return shift_results[shift_calls++];
}

#include "actual_cases_12_13.inc"

static void reset_fixture() {
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Bases, 0, sizeof(Bases));
    memset(MapStore, 0, sizeof(MapStore));
    LockEnableMask = 0;
    TurnCurrentNum = 0;
    MapIsFlat = 1;
    MapLongitudeBounds = 32;
    MapLatitudeBounds = 32;
    MapLongitude = 16;
    IsMultiplayerNet = 0;
    IsMultiplayerPBEM = 0;
    shift_result_count = shift_calls = territory_calls = 0;
    parse_calls = popup_calls = 0;
    parse_channel = parse_value = popup_value = -99;
    popup_title[0] = popup_image[0] = '\0';
    territory_result = -1;
    focus_expected_x = focus_expected_y = 10;
    call_log.clear();
    ConsoleGlobal.faction_id_ = 1;
    ConsoleGlobal.log = &call_log;
    Bases[0].x = Bases[0].y = 10;
}

static void test_energy_boom_order_and_thresholds() {
    reset_fixture();
    PlayersData[2].ranking = 4;
    PlayersData[2].energy_reserves = 500;
    random_events_cases_12_13(12, 2, 0, 31, 10, 10);
    assert(PlayersData[2].energy_reserves == 1000);
    assert(parse_calls == 0 && popup_calls == 0);

    reset_fixture();
    PlayersData[1].ranking = 4;
    PlayersData[1].energy_reserves = 500;
    random_events_cases_12_13(12, 1, 0, 31, 10, 10);
    assert(PlayersData[1].energy_reserves == 1000);
    assert(parse_calls == 1 && parse_channel == 0 && parse_value == 1000);
    assert(popup_calls == 1 && strcmp(popup_title, "ENERGYBOOM") == 0);
    assert(strcmp(popup_image, "markbm_sm.pcx") == 0 && popup_value == -1);
    assert(call_log == "PUD");

    reset_fixture();
    PlayersData[1].ranking = 5;
    PlayersData[1].energy_reserves = 500;
    random_events_cases_12_13(12, 1, 0, 31, 10, 10);
    assert(PlayersData[1].energy_reserves == 500);
    assert(call_log.empty());
}

static void test_inflation_human_mask_and_pbem_order() {
    reset_fixture();
    PlayersData[2].energy_reserves = 1500;
    random_events_cases_12_13(12, 2, 0, 19, 10, 10);
    assert(PlayersData[2].energy_reserves == 1500);
    assert(call_log.empty());

    reset_fixture();
    PlayersData[2].energy_reserves = 1500;
    LockEnableMask = (1u << 2) | 0x100u;
    random_events_cases_12_13(12, 2, 0, 19, 10, 10);
    assert(PlayersData[2].energy_reserves == 375);
    assert(call_log.empty());

    reset_fixture();
    PlayersData[2].energy_reserves = 2001;
    IsMultiplayerPBEM = 1;
    random_events_cases_12_13(12, 2, 0, 19, 10, 10);
    assert(PlayersData[2].energy_reserves == 500);
    assert(parse_calls == 1 && parse_value == 500);
    assert(popup_calls == 1 && strcmp(popup_title, "INFLATION") == 0);
    assert(popup_value == -3);
    assert(call_log == "PD");  // PBEM does not refresh Console data.
}

static Map &tile_at(int x, int y) {
    return MapStore[y * static_cast<int>(MapLongitude) + (x >> 1)];
}

static void seed_hail_tile(int x, int y, int faction) {
    Map &tile = tile_at(x, y);
    tile.climate = 0x60;
    tile.visibility = static_cast<unsigned char>(1u << faction);
    tile.bit = BIT_SOLAR_TIDAL | 0x100;
    for (int i = 0; i < 7; ++i) tile.bit_visible[i] = BIT_SOLAR_TIDAL | 0x80;
}

static void test_hail_mutation_focus_wait_and_popup() {
    reset_fixture();
    TurnCurrentNum = 75;
    PlayersData[2].ranking = 4;
    Bases[0].visibility = static_cast<unsigned char>(1u << 1);
    seed_hail_tile(10, 10, 2);
    shift_result_count = 2;
    shift_results[0] = shift_results[1] = 0;
    random_events_cases_12_13(13, 2, 0, 44, 10, 10);
    assert(territory_calls == 1);
    assert((tile_at(10, 10).bit & BIT_SOLAR_TIDAL) == 0);
    assert((tile_at(10, 10).bit & 0x100) != 0);
    for (int i = 0; i < 7; ++i)
        assert((tile_at(10, 10).bit_visible[i] & BIT_SOLAR_TIDAL) == 0);
    assert(popup_calls == 1 && strcmp(popup_title, "HAIL") == 0);
    assert(strcmp(popup_image, "hail_sm.pcx") == 0 && popup_value == 44);
    assert(call_log == "TFKWBSRKwD");
}

static void test_hail_pbem_mutates_without_focus_or_delay() {
    reset_fixture();
    TurnCurrentNum = 150;
    PlayersData[2].ranking = 5;
    IsMultiplayerPBEM = 1;
    seed_hail_tile(10, 10, 2);
    shift_result_count = 1;
    shift_results[0] = 0;
    random_events_cases_12_13(13, 2, 0, 8, 10, 10);
    assert(call_log == "TBSRKD");
    assert(popup_calls == 1 && popup_value == 8);
}

static void test_hail_unseen_remote_event_mutates_without_announcement() {
    reset_fixture();
    TurnCurrentNum = 75;
    PlayersData[2].ranking = 4;
    Bases[0].visibility = 0;
    seed_hail_tile(10, 10, 2);
    random_events_cases_12_13(13, 2, 0, 8, 10, 10);
    assert(call_log == "TBS");
    assert((tile_at(10, 10).bit & BIT_SOLAR_TIDAL) == 0);
    assert(popup_calls == 0);
}

static void test_hail_flat_boundary_and_unowned_eligibility() {
    reset_fixture();
    TurnCurrentNum = 75;
    PlayersData[2].ranking = 4;
    MapIsFlat = 0;
    Bases[0].x = 0;
    Bases[0].y = 10;
    focus_expected_x = 0;
    focus_expected_y = 10;
    // Radius slot 5 has x=-1, which wraps once to the far map edge.
    seed_hail_tile(31, 11, 2);
    territory_result = -1;
    Bases[0].visibility = static_cast<unsigned char>(1u << 1);
    shift_result_count = 2;
    shift_results[0] = shift_results[1] = 0;
    random_events_cases_12_13(13, 2, 0, 8, 0, 10);
    assert((tile_at(31, 11).bit & BIT_SOLAR_TIDAL) == 0);
    assert(popup_calls == 1);
}

static void test_hail_year_ranking_territory_and_altitude_rejections() {
    reset_fixture();
    TurnCurrentNum = 74;
    PlayersData[2].ranking = 7;
    seed_hail_tile(10, 10, 2);
    random_events_cases_12_13(13, 2, 0, 8, 10, 10);
    assert(territory_calls == 0 && call_log.empty());

    reset_fixture();
    TurnCurrentNum = 75;
    PlayersData[2].ranking = 3;
    seed_hail_tile(10, 10, 2);
    random_events_cases_12_13(13, 2, 0, 8, 10, 10);
    assert(territory_calls == 0 && call_log.empty());

    reset_fixture();
    TurnCurrentNum = 75;
    PlayersData[2].ranking = 4;
    Bases[0].visibility = static_cast<unsigned char>(1u << 1);
    seed_hail_tile(10, 10, 2);
    territory_result = 3;  // another faction owns the tile
    random_events_cases_12_13(13, 2, 0, 8, 10, 10);
    assert(territory_calls == 1 && popup_calls == 0);
    assert((tile_at(10, 10).bit & BIT_SOLAR_TIDAL) != 0);

    reset_fixture();
    TurnCurrentNum = 75;
    PlayersData[2].ranking = 4;
    Bases[0].visibility = static_cast<unsigned char>(1u << 1);
    seed_hail_tile(10, 10, 2);
    tile_at(10, 10).climate = 0x40;  // shelf altitude is below land threshold
    random_events_cases_12_13(13, 2, 0, 8, 10, 10);
    assert(territory_calls == 1 && popup_calls == 0);
    assert((tile_at(10, 10).bit & BIT_SOLAR_TIDAL) != 0);
}

int main() {
    test_energy_boom_order_and_thresholds();
    test_inflation_human_mask_and_pbem_order();
    test_hail_mutation_focus_wait_and_popup();
    test_hail_pbem_mutates_without_focus_or_delay();
    test_hail_unseen_remote_event_mutates_without_announcement();
    test_hail_flat_boundary_and_unowned_eligibility();
    test_hail_year_ranking_territory_and_altitude_rejections();
    return 0;
}

