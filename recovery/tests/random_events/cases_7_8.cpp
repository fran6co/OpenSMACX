#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdint.h>
#include <vector>

struct Base {
    int x;
    int y;
    int faction_id_current;
    unsigned int visibility;
};

struct PlayerData {
    unsigned int flags;
};

struct Map {
    unsigned int visibility;
    unsigned char climate;
    unsigned int bit;
};

struct ResourceEntry {
    const char *name_plural;
};

struct RandomEventsFields {
    int turn_current_num_;
};

struct RandomEventsGameState {
    RandomEventsFields fields_;
};

struct FixtureConsole {
    int faction_id_;
    int focus_count;
    int focus_x, focus_y, focus_faction;

    FixtureConsole() : faction_id_(2), focus_count(0), focus_x(0), focus_y(0),
                       focus_faction(0) {}
    int focus(int x, int y, int faction) {
        ++focus_count;
        focus_x = x;
        focus_y = y;
        focus_faction = faction;
        return 0;
    }
};

struct FixtureMapWin {
    int centers;
    int last_x, last_y, last_animate;

    FixtureMapWin() : centers(0), last_x(0), last_y(0), last_animate(0) {}
    void set_center(int x, int y, int animate) {
        ++centers;
        last_x = x;
        last_y = y;
        last_animate = animate;
    }
};

static const int VK_SHIFT = 0x10;
static const unsigned int BIT_RESOURCE = 0x400u;
static int MapLongitudeBounds = 64;
static int MapLatitudeBounds = 32;
static int MapLongitude = 32;
static int MapIsFlat = 1;
static int IsMultiplayerPBEM;
static int IsMultiplayerNet;
static int bonus_values[32][64];
static Map MapTiles[32 * 32];
static Base Bases[8];
static PlayerData PlayersData[8];
static ResourceEntry Resource[4];
static RandomEventsGameState GameDataStateStorage;
static FixtureConsole ConsoleGlobal;
static FixtureMapWin PrimaryMapWindow;
static FixtureMapWin *MapWinTable[1];
static int random_value;
static int random_calls;
static std::vector<int> bonus_script;
static std::vector<int> key_script;
static std::vector<int> bit_values;
static std::vector<int> bit_x;
static std::vector<int> bit_y;
static std::vector<int> call_order;
static std::vector<int> waits;
static std::vector<int> parse_resources;
static std::vector<int> map_queries_x;
static std::vector<int> map_queries_y;
static int bonus_call_count;
static int owner_call_count;
static int territory_owner;
static int popup_count;
static int popup_argument;
static char popup_caption[32];
static char popup_image[32];
static bool mutate_base_x_on_first_bonus;
static bool mutate_local_faction_on_draw;

static const int RadiusOffsetX[22] = {
    0, 1, 2, 1, 0, -1, -2, -1, 0,
    2, 2, -2, -2, 1, 3, 3, 1, -1, -3, -3, -1, 4
};
static const int RadiusOffsetY[22] = {
    0, -1, 0, 1, 2, 1, 0, -1, -2,
    -2, 2, 2, -2, -3, -1, 1, 3, 3, 1, -1, -3, 0
};

static Map *map_loc(int x, int y) {
    assert(x >= 0 && x < MapLongitudeBounds && y >= 0 && y < MapLatitudeBounds);
    map_queries_x.push_back(x);
    map_queries_y.push_back(y);
    return &MapTiles[y * MapLongitude + (x >> 1)];
}

static int random(int low, int high) {
    assert(low == 0 && high == 20);
    assert(random_calls == 0);
    ++random_calls;
    call_order.push_back(1);
    return random_value;
}

static int bonus_at(int x, int y, int unused) {
    assert(unused == 0);
    ++bonus_call_count;
    call_order.push_back(2);
    if (mutate_base_x_on_first_bonus && bonus_call_count == 1)
        Bases[0].x = 12;
    if (!bonus_script.empty()) {
        const int result = bonus_script.front();
        bonus_script.erase(bonus_script.begin());
        return result;
    }
    return bonus_values[y][x];
}

static int whose_territory(int faction_id, int x, int y, int *base_id,
                           int ignore_comm) {
    (void)faction_id;
    (void)x;
    (void)y;
    assert(base_id == 0 && ignore_comm == 0);
    ++owner_call_count;
    call_order.push_back(3);
    return territory_owner;
}

static void bit_set(int x, int y, int mask, int enabled) {
    assert(static_cast<unsigned int>(mask) == BIT_RESOURCE);
    Map *tile = map_loc(x, y);
    if (enabled) tile->bit |= BIT_RESOURCE;
    else tile->bit &= ~BIT_RESOURCE;
    bit_values.push_back(enabled);
    bit_x.push_back(x);
    bit_y.push_back(y);
    call_order.push_back(4);
}

static void synch_bit(int x, int y, int faction_id) {
    (void)faction_id;
    (void)map_loc(x, y);
    call_order.push_back(5);
}

static void draw_tile(int x, int y, int mode) {
    assert(mode == 2);
    (void)map_loc(x, y);
    call_order.push_back(6);
    if (mutate_local_faction_on_draw) ConsoleGlobal.faction_id_ = 3;
}

static FixtureMapWin *console_map_win() { return MapWinTable[0]; }

static short GetAsyncKeyState(int key) {
    assert(key == VK_SHIFT);
    call_order.push_back(7);
    if (key_script.empty()) return 0;
    const int result = key_script.front();
    key_script.erase(key_script.begin());
    return static_cast<short>(result);
}

static void clock_wait(int milliseconds) {
    waits.push_back(milliseconds);
    call_order.push_back(8);
}

static int parse_say(int, const char *resource, int, int) {
    int resource_id = -1;
    for (int i = 0; i < 4; ++i)
        if (resource == Resource[i].name_plural) resource_id = i;
    assert(resource_id >= 0);
    parse_resources.push_back(resource_id);
    call_order.push_back(9);
    return 0;
}

static int POP2(const char *caption, const char *image, int argument) {
    assert(popup_count == 0);
    std::strcpy(popup_caption, caption);
    std::strcpy(popup_image, image);
    popup_argument = argument;
    ++popup_count;
    call_order.push_back(10);
    return 0;
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#include "actual_cases_7_8.hpp"
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

static void reset() {
    std::memset(bonus_values, 0, sizeof(bonus_values));
    std::memset(MapTiles, 0, sizeof(MapTiles));
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    Resource[0].name_plural = "nutrient";
    Resource[1].name_plural = "mineral";
    Resource[2].name_plural = "energy";
    Resource[3].name_plural = "rare";
    MapLongitudeBounds = 64;
    MapLatitudeBounds = 32;
    MapLongitude = 32;
    MapIsFlat = 1;
    IsMultiplayerPBEM = 0;
    IsMultiplayerNet = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 100;
    ConsoleGlobal = FixtureConsole();
    PrimaryMapWindow = FixtureMapWin();
    MapWinTable[0] = &PrimaryMapWindow;
    random_value = 0;
    random_calls = bonus_call_count = owner_call_count = 0;
    territory_owner = -1;
    bonus_script.clear();
    key_script.clear();
    bit_values.clear();
    bit_x.clear();
    bit_y.clear();
    call_order.clear();
    waits.clear();
    parse_resources.clear();
    map_queries_x.clear();
    map_queries_y.clear();
    popup_count = popup_argument = 0;
    popup_caption[0] = popup_image[0] = '\0';
    mutate_base_x_on_first_bonus = false;
    mutate_local_faction_on_draw = false;
    Bases[0].x = 10;
    Bases[0].y = 10;
    Bases[0].faction_id_current = 2;
}

static void run_case(int event_kind) {
    random_events_cases_7_8_from_mapped_body(event_kind, 2, 0, 17, 4, 5);
}

static void test_pbem_guard_precedes_random_and_map_queries() {
    reset();
    IsMultiplayerPBEM = 1;
    run_case(7);
    assert(random_calls == 0 && bonus_call_count == 0);
    assert(map_queries_x.empty() && bit_values.empty() && popup_count == 0);
}

static void test_nearby_bonus_scan_and_removal_preserve_effect_order() {
    reset();
    random_value = 19;
    PlayersData[2].flags = 0;
    Map *first = map_loc(10, 10);
    first->visibility = 1u << 1; // Present, but not visible to faction 2.
    bonus_values[10][10] = 1;
    Map *chosen = map_loc(11, 9);
    chosen->visibility = 1u << 2;
    chosen->climate = 0x60; // Land; unowned territory is accepted.
    bonus_values[9][11] = 3;
    bonus_script.push_back(1);
    bonus_script.push_back(3);
    bonus_script.push_back(3); // Final presence and resource-table reads.
    bonus_script.push_back(3);
    bonus_script.push_back(0); // Post-clear callback read.

    run_case(7);
    assert(random_calls == 1 && random_value == 19);
    assert(bonus_call_count == 5 && owner_call_count == 2);
    assert(parse_resources.size() == 1 && parse_resources[0] == 2);
    assert(bit_values.size() == 21 && bit_values[0] == 0);
    assert(bit_values[1] == 1 && bit_values[2] == 0);
    assert(waits.size() == 21 && waits.front() == 0x14 && waits.back() == 0x64);
    assert(PrimaryMapWindow.centers == 1);
    assert(PrimaryMapWindow.last_x == 11 && PrimaryMapWindow.last_y == 9 &&
           PrimaryMapWindow.last_animate == 1);
    assert(popup_count == 1 && std::strcmp(popup_caption, "NEWRESOURCE") == 0);
    assert(std::strcmp(popup_image, "rdminl_sm.pcx") == 0 && popup_argument == -1);
    assert(call_order.front() == 1);
    assert(call_order[call_order.size() - 1] == 10);
    const int parse_at = static_cast<int>(std::find(call_order.begin(),
        call_order.end(), 9) - call_order.begin());
    const int first_bit_at = static_cast<int>(std::find(call_order.begin(),
        call_order.end(), 4) - call_order.begin());
    assert(parse_at < first_bit_at);
}

static void test_live_base_coordinate_reread_after_bonus_callback() {
    reset();
    mutate_base_x_on_first_bonus = true;
    bonus_values[9][13] = 2;
    Map *selected = map_loc(13, 9);
    selected->visibility = 1u << 2;
    selected->climate = 0x40;
    bonus_script.push_back(0); // First scan callback moves base x from 10 to 12.
    bonus_script.push_back(2); // Offset 1 is then read from x=13.
    bonus_script.push_back(0); // Recomputed result-position checks.
    run_case(8);
    assert(Bases[0].x == 12);
    assert(bonus_call_count >= 2);
    bool saw_live_coordinate = false;
    for (std::size_t i = 0; i < map_queries_x.size(); ++i)
        if (map_queries_x[i] == 13 && map_queries_y[i] == 9) saw_live_coordinate = true;
    assert(saw_live_coordinate);
}

static void test_foreign_territory_rejects_final_fallback() {
    reset();
    PlayersData[2].flags = 0x200u; // Bypass the visibility mask.
    Map *nearby = map_loc(10, 10);
    nearby->climate = 0x60;
    bonus_values[10][10] = 1;
    territory_owner = 4;
    random_value = 0; // Fallback offset 1 is independently rejected.
    Map *fallback = map_loc(11, 9);
    fallback->climate = 0x60;
    run_case(7);
    assert(owner_call_count >= 2);
    assert(bit_values.empty() && popup_count == 0);
}

static void test_nearby_removal_turn_gate_precedes_mutation() {
    reset();
    GameDataStateStorage.fields_.turn_current_num_ = 74;
    bonus_values[10][10] = 1;
    Map *target = map_loc(10, 10);
    target->visibility = 1u << 2;
    target->climate = 0x60;
    bonus_script.push_back(1); // Nearby scan candidate.
    bonus_script.push_back(1); // Final presence check before the turn gate.
    run_case(7);
    assert(bonus_call_count == 2);
    assert(parse_resources.empty());
    assert(bit_values.empty() && popup_count == 0);
}

static void test_single_wrap_and_fallback_offset_twenty() {
    reset();
    Bases[0].x = 0;
    MapIsFlat = 0;
    bonus_values[11][63] = 2;
    Map *wrapped = map_loc(63, 11);
    wrapped->visibility = 1u << 2;
    wrapped->climate = 0x40;
    bonus_script.push_back(2); // Accept the wrapped radius slot 5.
    bonus_script.push_back(0); // Fallback/resource reads terminate safely.
    random_value = 8;
    run_case(7);
    assert(std::find(map_queries_x.begin(), map_queries_x.end(), 63) !=
           map_queries_x.end());

    reset();
    random_value = 19; // Random + 1 selects radius slot 20.
    Map *fallback = map_loc(9, 7);
    fallback->visibility = 1u << 2;
    fallback->climate = 0x40;
    run_case(8);
    assert(bit_values.size() == 1 && bit_values[0] == 1);
    assert(bit_x[0] == 9 && bit_y[0] == 7);
    assert(bonus_call_count == 10); // Nine scan checks, then selected cell.
}

static void test_key_skip_and_mid_animation_poll_keep_native_toggles() {
    reset();
    random_value = 8; // Fallback slot 9 -> (12,8).
    Map *target = map_loc(12, 8);
    target->visibility = 1u << 2;
    target->climate = 0x60;
    for (int i = 0; i < 9; ++i) bonus_script.push_back(0);
    bonus_script.push_back(3); // Fallback resource lookup.
    key_script.push_back(0x0100); // Native high-byte test skips animation.
    run_case(7);
    assert(bit_values.size() == 1 && bit_values[0] == 1);
    assert(PrimaryMapWindow.centers == 1 && waits.empty());
    assert(popup_count == 1 && std::strcmp(popup_image, "rdminl_sm.pcx") == 0);

    reset();
    IsMultiplayerNet = 1;
    random_value = 8;
    target = map_loc(12, 8);
    target->visibility = 1u << 2;
    target->climate = 0x40;
    for (int i = 0; i < 9; ++i) bonus_script.push_back(0);
    bonus_script.push_back(3);
    key_script.push_back(0);
    key_script.push_back(0x0100); // Mid-animation press does not end the loop.
    run_case(7);
    assert(bit_values.size() == 21);
    assert(key_script.empty());
    assert(waits.empty()); // Multiplayer network suppresses every wait.
}

static void test_local_faction_is_reloaded_after_draw_callback() {
    reset();
    mutate_local_faction_on_draw = true;
    random_value = 0;
    bonus_values[10][10] = 1;
    Map *target = map_loc(10, 10);
    target->visibility = 1u << 2;
    target->climate = 0x40;
    bonus_script.push_back(1); // Scan and selected-position reads.
    bonus_script.push_back(1);
    bonus_script.push_back(1);
    bonus_script.push_back(0); // Post-draw recheck.
    run_case(8);
    assert(ConsoleGlobal.faction_id_ == 3);
    assert(popup_count == 0 && PrimaryMapWindow.centers == 0);
}

int main() {
    test_pbem_guard_precedes_random_and_map_queries();
    test_nearby_bonus_scan_and_removal_preserve_effect_order();
    test_live_base_coordinate_reread_after_bonus_callback();
    test_foreign_territory_rejects_final_fallback();
    test_nearby_removal_turn_gate_precedes_mutation();
    test_single_wrap_and_fallback_offset_twenty();
    test_key_skip_and_mid_animation_poll_keep_native_toggles();
    test_local_faction_is_reloaded_after_draw_callback();
    return 0;
}
