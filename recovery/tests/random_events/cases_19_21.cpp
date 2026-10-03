#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    FAC_CHILDREN_CRECHE = 2,
    BIT_MINE = 0x10,
    BIT_FARM = 0x8000,
    RFLAG_ALIEN = 0x80,
    VK_SHIFT = 0x10
};

struct PlayerData { uint32_t flags; int ranking; };
struct Player { uint32_t rule_flags; };
struct Base {
    int16_t x, y;
    uint8_t faction_id_current, visibility;
    int8_t population_size;
    uint8_t assimilation_turns_left;
    uint8_t facilities_built[12];
    int nutrient_surplus;
    char *name_string;
};
struct Map {
    uint8_t climate, region, val2, territory, visibility, pad[3];
    uint32_t bit, bit2, bit_visible[7];
};
struct RulesFacility { int preq_tech; };

static PlayerData PlayersData[8];
static Player Players[8];
static Base Bases[8];
static Map Tiles[8 * 4];
static Map *MapTiles = Tiles;
static RulesFacility Facility[16];
static const int RadiusOffsetX[21] = {
    0, 1, 2, 1, 0, -1, -2, -1, 0, 2, 2, -2, -2, 1, 3, 3, 1, -1, -3, -3, -1
};
static const int RadiusOffsetY[21] = {
    0, -1, 0, 1, 2, 1, 0, -1, -2, -2, 2, 2, -2, -3, -1, 1, 3, 3, 1, -1, -3
};
static int TurnCurrentNum, MapLongitudeBounds, MapLatitudeBounds, MapIsFlat;
static uint32_t MapLongitude;
static int IsMultiplayerPBEM, IsMultiplayerNet;
static int territory_result, mutate_climate_in_territory;
static int focus_calls, draw_calls, wait_200_calls, wait_50_calls;
static int bit_set_calls, synch_calls, parse_calls, popup_calls, set_base_calls;
static int base_compute_calls, last_popup_id, last_popup_picture, last_popup_caption;
static int focus_result;
static int tile_at_calls;
static int mutate_faction_during_focus;

struct Console {
    int faction_id_;
    int focus(int, int, int) {
        ++focus_calls;
        if (mutate_faction_during_focus) faction_id_ = 1;
        return focus_result;
    }
};
static Console ConsoleGlobal;

static Map *map_tiles() { return MapTiles; }
static int has_tech(int tech, int faction) {
    assert(tech == Facility[FAC_CHILDREN_CRECHE].preq_tech);
    assert(faction >= 0 && faction < 8);
    return 1;
}
static int has_fac_built_call(int facility, int base_id) {
    assert(facility == FAC_CHILDREN_CRECHE);
    return (Bases[base_id].facilities_built[0] & (1u << facility)) != 0;
}
static void set_base(int base_id) { assert(base_id == 3); ++set_base_calls; }
static int base_compute(int force) {
    assert(force == 1);
    ++base_compute_calls;
    --Bases[3].nutrient_surplus;
    return 0;
}
static int whose_territory(int, int x, int y, int *base, int ignore_comm) {
    assert(base == 0 && ignore_comm == 0);
    ++tile_at_calls;
    if (mutate_climate_in_territory && x == 4 && y == 4)
        map_tiles()[y * MapLongitude + (x >> 1)].climate = 0x60;
    return territory_result;
}
static void bit_set_call(int x, int y, int bit, int set) {
    assert(x == 4 && y == 4 && set == 0);
    Map &tile = map_tiles()[y * MapLongitude + (x >> 1)];
    tile.bit &= ~static_cast<uint32_t>(bit);
    ++bit_set_calls;
}
static void synch_bit(int x, int y, int faction) {
    assert(x == 4 && y == 4);
    assert(faction >= 0 && faction < 8);
    ++synch_calls;
}
static void draw_tile(int x, int y, int flags) {
    assert(x == 4 && y == 4 && flags == 2);
    ++draw_calls;
}
static int GetAsyncKeyState(int key) { assert(key == VK_SHIFT); return 0; }
static void clock_wait(int ms) {
    if (ms == 200) ++wait_200_calls;
    else { assert(ms == 50); ++wait_50_calls; }
}
static int parse_says(int slot, char *text, int x, int y) {
    assert(slot == 0 && text == Bases[3].name_string && x == -1 && y == -1);
    ++parse_calls;
    return 0;
}
static int POP2(const char *caption, const char *picture, int base_id) {
    assert(caption != 0);
    ++popup_calls;
    last_popup_id = base_id;
    last_popup_caption = strcmp(caption, "KELPWIPE") == 0 ? 20 :
                         strcmp(caption, "PLATFORMWIPE") == 0 ? 21 : 19;
    if (last_popup_caption == 19) {
        assert(strcmp(caption, "CRECHE1") == 0 || strcmp(caption, "CRECHE") == 0);
        assert(picture != 0);
        assert(strcmp(picture, "al_pop_sm.pcx") == 0 ||
               strcmp(picture, "pop_sm.pcx") == 0);
    } else if (last_popup_caption == 20) {
        assert(picture == 0 && strcmp(caption, "KELPWIPE") == 0);
    } else {
        assert(picture == 0 && strcmp(caption, "PLATFORMWIPE") == 0);
    }
    last_popup_picture = picture ? 1 : 0;
    return 0;
}

#include "actual_cases_19_21.inc"

static Map &tile_at(int x, int y) {
    return Tiles[y * static_cast<int>(MapLongitude) + (x >> 1)];
}
static void reset_fixture() {
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Players, 0, sizeof(Players));
    memset(Bases, 0, sizeof(Bases));
    memset(Tiles, 0, sizeof(Tiles));
    memset(Facility, 0, sizeof(Facility));
    TurnCurrentNum = 100;
    MapLongitudeBounds = 8;
    MapLatitudeBounds = 8;
    MapLongitude = 4;
    MapIsFlat = 1;
    IsMultiplayerPBEM = IsMultiplayerNet = 0;
    territory_result = -1;
    mutate_climate_in_territory = 0;
    focus_calls = draw_calls = wait_200_calls = wait_50_calls = 0;
    bit_set_calls = synch_calls = parse_calls = popup_calls = 0;
    set_base_calls = base_compute_calls = tile_at_calls = 0;
    last_popup_id = last_popup_caption = last_popup_picture = -99;
    focus_result = 1;
    mutate_faction_during_focus = 0;
    ConsoleGlobal.faction_id_ = 0;
    Bases[3].x = 4;
    Bases[3].y = 4;
    Bases[3].faction_id_current = 0;
    Bases[3].name_string = const_cast<char *>("Delta");
    Facility[FAC_CHILDREN_CRECHE].preq_tech = 77;
    PlayersData[0].ranking = 6;
}

int main() {
    // Creche population uses the source-owned facility/tech records and may
    // execute base_compute twice at most. This callback consumes one surplus.
    reset_fixture();
    Bases[3].facilities_built[0] = 1u << FAC_CHILDREN_CRECHE;
    Bases[3].nutrient_surplus = 1;
    random_events_cases_19_21(19, 0, 3, 123, 4, 4);
    assert(Bases[3].population_size == 1);
    assert(set_base_calls == 1 && base_compute_calls == 1);
    assert(popup_calls == 1 && last_popup_id == 3 && last_popup_caption == 19);

    // Without the creche, the native rank/assimilation path stores 5 before
    // any UI callback; native rereads the viewer after focus before deciding
    // whether PBEM permits the popup.
    reset_fixture();
    PlayersData[0].ranking = 5;
    TurnCurrentNum = 150;
    mutate_faction_during_focus = 1;
    random_events_cases_19_21(19, 0, 3, 0, 4, 4);
    assert(Bases[3].assimilation_turns_left == 5);
    assert(focus_calls == 1 && popup_calls == 0);

    // The first climate gate rejects ocean, and an ordinary territory query
    // cannot make the second, opposing climate gate true. This is the pinned
    // native ordering, not a repaired terrain test.
    reset_fixture();
    PlayersData[0].flags = 0x200;
    tile_at(4, 4).climate = 0x20;
    tile_at(4, 4).bit = BIT_FARM;
    random_events_cases_19_21(20, 0, 3, 0, 4, 4);
    assert(tile_at_calls > 0);
    assert(bit_set_calls == 0 && synch_calls == 0 && popup_calls == 0);

    // The effect path is still native when a territory test double opens the
    // second terrain predicate, but an unseen foreign event changes the map
    // without local focus/draw/parse/popup work.
    reset_fixture();
    ConsoleGlobal.faction_id_ = 1;
    PlayersData[0].flags = 0x200;
    tile_at(4, 4).climate = 0x20;
    tile_at(4, 4).bit = BIT_FARM;
    mutate_climate_in_territory = 1;
    random_events_cases_19_21(20, 0, 3, 0, 4, 4);
    assert(bit_set_calls == 1 && synch_calls == 1);
    assert(focus_calls == 0 && draw_calls == 0 && parse_calls == 0 && popup_calls == 0);

    reset_fixture();
    IsMultiplayerPBEM = 1;
    tile_at(4, 4).climate = 0x20;
    tile_at(4, 4).bit = BIT_FARM;
    random_events_cases_19_21(20, 0, 3, 0, 4, 4);
    assert(tile_at_calls == 0 && bit_set_calls == 0);

    reset_fixture();
    TurnCurrentNum = 74;
    tile_at(4, 4).climate = 0x20;
    tile_at(4, 4).bit = BIT_MINE;
    random_events_cases_19_21(21, 0, 3, 0, 4, 4);
    assert(tile_at_calls == 0 && bit_set_calls == 0);

    reset_fixture();
    PlayersData[0].flags = 0x200;
    tile_at(4, 4).climate = 0x20;
    tile_at(4, 4).bit = BIT_MINE;
    random_events_cases_19_21(21, 0, 3, 0, 4, 4);
    assert(tile_at_calls > 0);
    assert(bit_set_calls == 0 && synch_calls == 0 && popup_calls == 0);

    // A mutating territory double is only a branch-control aid: it demonstrates
    // that the second native gate and its downstream typed calls remain in the
    // C++ body. The real whose_territory implementation does not do this.
    reset_fixture();
    PlayersData[0].flags = 0x200;
    tile_at(4, 4).climate = 0x20;
    tile_at(4, 4).bit = BIT_FARM;
    mutate_climate_in_territory = 1;
    random_events_cases_19_21(20, 0, 3, 0, 4, 4);
    assert(bit_set_calls == 1 && synch_calls == 1 && draw_calls == 1);
    assert(wait_200_calls == 1 && wait_50_calls == 1);
    assert(parse_calls == 1 && popup_calls == 1 && last_popup_id == 3);
    assert(last_popup_picture == 0 && last_popup_caption == 20);

    reset_fixture();
    PlayersData[0].flags = 0x200;
    tile_at(4, 4).climate = 0x20;
    tile_at(4, 4).bit = BIT_MINE;
    mutate_climate_in_territory = 1;
    random_events_cases_19_21(21, 0, 3, 0, 4, 4);
    assert(bit_set_calls == 1 && synch_calls == 1 && draw_calls == 1);
    assert(parse_calls == 1 && popup_calls == 1 && last_popup_id == 3);
    assert(last_popup_picture == 0 && last_popup_caption == 21);
}

