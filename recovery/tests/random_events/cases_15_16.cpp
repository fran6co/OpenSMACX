#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <string>

enum {
    BIT_VEH_IN_TILE = 0x2,
    BIT_FOREST = 0x200000,
    BIT_FARM = 0x8000,
    FAC_BIOLOGY_LAB = 9,
    VK_SHIFT = 0x10
};

struct PlayerData {
    uint32_t flags;
    int ranking;
    uint32_t current_num_bases;
};
struct Base {
    int16_t x;
    int16_t y;
    unsigned char faction_id_current;
    unsigned char visibility;
    int8_t population_size;
    uint32_t event;
    int random_event_turns;
    uint8_t facilities_built[12];
    char name_string[25];
};
struct Map {
    uint8_t climate;
    uint8_t visibility;
    uint32_t bit;
    uint32_t bit2;
    uint32_t bit_visible[7];
};
struct FacilityData {
    int preq_tech;
};
struct Veh {
    int16_t x;
    int16_t y;
    uint8_t faction_id;
};
struct Console {
    int faction_id_;
    std::string *log;
    int focus(int x, int y, int faction) {
        assert(x == 40 && y == 40 && faction == faction_id_);
        log->push_back('F');
        return 1;
    }
};

static PlayerData PlayersData[8];
static Base Bases[8];
static Map MapStore[80 * 40];
static Map *MapTiles = MapStore;
static Veh Vehs[16];
static FacilityData Facility[16];
static int DustCloudDuration;
static char script_popup_title[] = "SCRIPT";
static char *script_popup_title_pointer = script_popup_title;
static char **ClimatePopupTitle = &script_popup_title_pointer;
static int VehCurrentCount;
static const int RadiusOffsetX[49] = {
    0, 1, 2, 1, 0, -1, -2, -1, 0, 2, 2, -2, -2, 1, 3, 3, 1, -1, -3, -3,
    -1, 4, -4, 0, 0, 1, 2, 3, 4, 5, 5, 4, 3, 2, 1, -1, -2, -3, -4, -5,
    -5, -4, -3, -2, -1, 0, 6, 0, -6
};
static const int RadiusOffsetY[49] = {
    0, -1, 0, 1, 2, 1, 0, -1, -2, -2, 2, 2, -2, -3, -1, 1, 3, 3, 1, -1,
    -3, 0, 0, 4, -4, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
    -1, -2, -3, -4, -5, 6, 0, -6, 0
};
static unsigned int LockEnableMask;
static int TurnCurrentNum;
static int MapIsFlat;
static int MapLongitudeBounds;
static int MapLatitudeBounds;
static unsigned int MapLongitude;
static int IsMultiplayerNet;
static int IsMultiplayerPBEM;
static uint32_t GameState;
static int MountPlanetX;
static int MountPlanetY;
static int ascent_tech_available;
static int landmark_nearby;
static int focus_result;
static int parse_calls;
static int popup_calls;
static int popp_calls;
static int bit_set_calls;
static int synch_calls;
static int crater_calls;
static int climate_calls;
static int wait_calls;
static int wait_200_calls;
static int wait_50_calls;
static int async_key_state;
static int draw_calls;
static int territory_result;
static int territory_calls;
static std::string call_log;
static Console ConsoleGlobal;

static Map *map_tiles() { return MapTiles; }
static int near_landmark(int, int) {
    call_log.push_back('N');
    return landmark_nearby;
}
static int has_tech(int tech_id, int faction) {
    assert(tech_id == 77 && faction >= 0 && faction < 8);
    call_log.push_back('H');
    return ascent_tech_available;
}
static int has_fac_built(int facility_id, int base_id) {
    assert(facility_id == FAC_BIOLOGY_LAB);
    const int offset = facility_id / 8;
    const int mask = 1 << (facility_id & 7);
    return (Bases[base_id].facilities_built[offset] & mask) != 0;
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
    ++bit_set_calls;
    call_log.push_back('B');
}
static void synch_bit(int, int, int) {
    ++synch_calls;
    call_log.push_back('S');
}
static void veh_kill(int vehicle_id) {
    assert(vehicle_id >= 0 && vehicle_id < VehCurrentCount);
    --VehCurrentCount;
    if (vehicle_id != VehCurrentCount) Vehs[vehicle_id] = Vehs[VehCurrentCount];
    call_log.push_back('V');
}
static int base_at(int x, int y) {
    call_log.push_back('A');
    for (int i = 0; i < 8; ++i)
        if (Bases[i].population_size > 0 && Bases[i].x == x && Bases[i].y == y)
            return i;
    return -1;
}
static void base_kill(int base_id) {
    Bases[base_id].population_size = 0;
    call_log.push_back('K');
}
static void parse_says(int channel, char *, int a, int b) {
    assert(channel == 0 && a == -1 && b == -1);
    ++parse_calls;
    call_log.push_back('P');
}
static void world_crater(int x, int y) {
    assert(x == 40 && y == 40);
    ++crater_calls;
    call_log.push_back('C');
}
static void world_climate() {
    ++climate_calls;
    call_log.push_back('W');
}
static void draw_map(int draw_type) {
    assert(draw_type == 1);
    ++draw_calls;
    call_log.push_back('M');
}
static void draw_tile(int, int, int type) {
    assert(type == 2);
    call_log.push_back('Y');
}
static void draw_radius(int x, int y, int radius, int draw_type) {
    assert(x == 40 && y == 40 && radius == 2 && draw_type == 2);
    call_log.push_back('Q');
}
static int GetAsyncKeyState(int key) {
    assert(key == VK_SHIFT);
    call_log.push_back('X');
    return async_key_state;
}
static void clock_wait(int milliseconds) {
    ++wait_calls;
    if (milliseconds == 200) ++wait_200_calls;
    if (milliseconds == 50) ++wait_50_calls;
    call_log.push_back('D');
}
static int POP2(const char *title, const char *image, int value) {
    assert(value == 41);
    assert(strcmp(image, "markbm_sm.pcx") == 0);
    assert(strcmp(title, "BLIGHT1") == 0 || strcmp(title, "BLIGHT") == 0);
    ++popup_calls;
    call_log.push_back('P');
    return 0;
}
static int popp(const char *caption, const char *label, int flags,
                const char *picture, int (*callback)()) {
    assert(strcmp(caption, "SCRIPT") == 0);
    assert(strcmp(label, "EATTHIS") == 0);
    assert(flags == 0 && strcmp(picture, "astp_sm.pcx") == 0 && callback == 0);
    ++popp_calls;
    call_log.push_back('O');
    return 0;
}

#include "actual_blight_cache.inc"
#include "actual_cases_15_16.inc"

static Map &tile_at(int x, int y) {
    return MapStore[y * static_cast<int>(MapLongitude) + (x >> 1)];
}
static void reset_fixture() {
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Bases, 0, sizeof(Bases));
    memset(MapStore, 0, sizeof(MapStore));
    memset(Vehs, 0, sizeof(Vehs));
    memset(Facility, 0, sizeof(Facility));
    Facility[FAC_BIOLOGY_LAB].preq_tech = 77;
    DustCloudDuration = 0;
    VehCurrentCount = 0;
    LockEnableMask = 0;
    TurnCurrentNum = 100;
    MapIsFlat = 1;
    MapLongitudeBounds = 80;
    MapLatitudeBounds = 80;
    MapLongitude = 40;
    IsMultiplayerNet = IsMultiplayerPBEM = 0;
    GameState = 0;
    MountPlanetX = MountPlanetY = 40;
    ascent_tech_available = 1;
    landmark_nearby = 0;
    focus_result = 1;
    parse_calls = popup_calls = popp_calls = 0;
    bit_set_calls = synch_calls = 0;
    crater_calls = climate_calls = wait_calls = wait_200_calls = wait_50_calls = 0;
    draw_calls = 0;
    async_key_state = 0x8000;
    territory_result = -1;
    territory_calls = 0;
    call_log.clear();
    ConsoleGlobal.faction_id_ = 1;
    ConsoleGlobal.log = &call_log;
    Bases[0].x = Bases[0].y = 40;
    Bases[0].faction_id_current = 2;
    Bases[0].population_size = 5;
    strcpy(Bases[0].name_string, "Target");
    focus_result = 1;
}

static void test_volcano_eligible_site_and_entity_effects() {
    reset_fixture();
    PlayersData[2].current_num_bases = 8;
    for (int i = 0; i < 49; ++i) {
        const int x = 40 + RadiusOffsetX[i];
        const int y = 40 + RadiusOffsetY[i];
        tile_at(x, y).climate = 0x60;
    }
    Bases[1].x = 42;
    Bases[1].y = 40;
    Bases[1].faction_id_current = 1;
    Bases[1].population_size = 5;
    strcpy(Bases[1].name_string, "Local");
    tile_at(40, 40).bit = BIT_VEH_IN_TILE;
    tile_at(41, 39).bit = 0;
    VehCurrentCount = 2;
    Vehs[0].x = 40;
    Vehs[0].y = 40;
    Vehs[1].x = 41;
    Vehs[1].y = 39;
    Vehs[1].faction_id = 1;
    random_events_cases_15_16(15, 2, 0, 41, 40, 40);
    assert(VehCurrentCount == 0);
    assert(Bases[0].population_size == 0 && Bases[1].population_size == 0);
    assert(parse_calls == 1 && crater_calls == 1 && climate_calls == 1);
    assert(bit_set_calls == 49 && synch_calls == 49);
    assert(popp_calls == 1 && DustCloudDuration == 10);
}

static void test_volcano_rejection_boundaries() {
    reset_fixture();
    PlayersData[2].current_num_bases = 7;
    random_events_cases_15_16(15, 2, 0, 41, 40, 40);
    assert(crater_calls == 0);

    reset_fixture();
    PlayersData[2].current_num_bases = 8;
    TurnCurrentNum = 74;
    random_events_cases_15_16(15, 2, 0, 41, 40, 40);
    assert(crater_calls == 0);

    reset_fixture();
    PlayersData[2].current_num_bases = 8;
    landmark_nearby = 1;
    random_events_cases_15_16(15, 2, 0, 41, 40, 40);
    assert(crater_calls == 0);

    reset_fixture();
    PlayersData[2].current_num_bases = 8;
    MapStore[40 * 40].climate = 0x20; // inner crater area has ocean terrain
    random_events_cases_15_16(15, 2, 0, 41, 40, 40);
    assert(crater_calls == 0);
}

static void test_blight_technology_facility_rank_and_callbacks() {
    reset_fixture();
    IsMultiplayerPBEM = 1;
    TurnCurrentNum = 149;
    PlayersData[2].ranking = 3;
    PlayersData[1].flags = 0x200;
    Bases[0].facilities_built[FAC_BIOLOGY_LAB / 8] =
        static_cast<uint8_t>(1u << (FAC_BIOLOGY_LAB & 7));
    tile_at(40, 40).climate = 0x60;
    tile_at(40, 40).visibility = static_cast<uint8_t>(1u << 2);
    tile_at(40, 40).bit = BIT_FOREST | BIT_FARM;
    tile_at(40, 40).bit2 = 0x288000;
    tile_at(40, 40).bit_visible[0] = 0x288000;
    for (int i = 1; i < 7; ++i) tile_at(40, 40).bit_visible[i] = 0x288000;
    random_events_cases_15_16(16, 2, 0, 41, 40, 40);
    assert((tile_at(40, 40).bit & 0x288000) == 0);
    assert((tile_at(40, 40).bit2 & 0x288000) != 0);
    assert((tile_at(40, 40).bit_visible[0] & 0x288000) == 0);
    for (int i = 1; i < 7; ++i)
        assert((tile_at(40, 40).bit_visible[i] & 0x288000) != 0);
    assert(Bases[0].event == 0x200 && Bases[0].random_event_turns == 10);
    assert(popup_calls == 1 && call_log.find("H") == 0);
}

static void test_blight_without_lab_requires_ranking_and_territory() {
    reset_fixture();
    TurnCurrentNum = 149;
    PlayersData[2].ranking = 3;
    tile_at(40, 40).climate = 0x60;
    tile_at(40, 40).visibility = static_cast<uint8_t>(1u << 2);
    tile_at(40, 40).bit = BIT_FOREST | BIT_FARM;
    random_events_cases_15_16(16, 2, 0, 41, 40, 40);
    assert(territory_calls == 0 && bit_set_calls == 0 && popup_calls == 0);

    reset_fixture();
    IsMultiplayerPBEM = 1;
    TurnCurrentNum = 150;
    PlayersData[2].ranking = 5;
    PlayersData[1].flags = 0x200;
    tile_at(40, 40).climate = 0x60;
    tile_at(40, 40).visibility = static_cast<uint8_t>(1u << 2);
    tile_at(40, 40).bit = BIT_FOREST | BIT_FARM;
    territory_result = 3;
    random_events_cases_15_16(16, 2, 0, 41, 40, 40);
    assert(territory_calls == 1 && bit_set_calls == 0 && popup_calls == 0);
}

static void test_blight_without_lab_focuses_once_before_first_tile() {
    reset_fixture();
    TurnCurrentNum = 150;
    PlayersData[1].ranking = 5;
    PlayersData[1].flags = 0x200; // Does not suppress the native first-tile wait.
    async_key_state = 0;
    tile_at(40, 40).climate = 0x60;
    tile_at(40, 40).visibility = static_cast<uint8_t>(1u << 1);
    tile_at(40, 40).bit = BIT_FOREST | BIT_FARM;
    random_events_cases_15_16(16, 1, 0, 41, 40, 40);
    assert(bit_set_calls == 1 && draw_calls == 0);
    assert(wait_calls == 2 && wait_200_calls == 1 && wait_50_calls == 1);
    assert(popup_calls == 1 && Bases[0].event == 0x200);
}

int main() {
    test_volcano_eligible_site_and_entity_effects();
    test_volcano_rejection_boundaries();
    test_blight_technology_facility_rank_and_callbacks();
    test_blight_without_lab_requires_ranking_and_territory();
    test_blight_without_lab_focuses_once_before_first_tile();
    return 0;
}

