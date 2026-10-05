// Host dependency boundaries and controls. The pytest harness inserts
// the actual production bodies; no parallel recovery implementation lives here.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __cdecl
#define __cdecl
#endif

typedef char *LPSTR;
typedef const char *LPCSTR;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t unk_1;
    uint8_t faction_id, year_end_lurking, dmg_incurred;
    int8_t order;
    uint8_t waypoint_count, patrol_current_point;
    int16_t waypoint_x[4];
    int16_t waypoint_y[4];
    uint8_t morale, terraforming_turns, order_auto_type, visibility;
    uint8_t moves_expended;
    int8_t facing;
    uint8_t unk_6, move_to_ai_type, probe_action, probe_sabotage_id;
    int16_t home_base_id, next_veh_id_stack, prev_veh_id_stack;
};
struct VehPrototype { char name[32]; uint32_t abilities; uint8_t chassis, weapon,
    armor, reactor, carry, cost, plan; int8_t unk; uint8_t obsolete, combat;
    int8_t icon, padding; uint16_t flags; int16_t preq; };
struct RulesOrder { LPSTR order, order_sea, letter; };
struct RulesResource { LPSTR name_singular, name_plural; };
struct Base { char name_string[25]; };
struct Map { uint8_t climate; };
struct TestStrings {
    LPSTR get(int id) { return const_cast<LPSTR>(texts[id]); }
    const char *texts[2048];
} StringTable;

static Veh Vehs[2049];
static VehPrototype VehPrototypes[512];
static RulesOrder Order[30];
static int LabelIds[2048];
static LPSTR Compass[8];
static RulesResource Resource[4];
static Base Bases[512];
static Map TileStorage[4096];
static Map *MapTiles = TileStorage;
static unsigned int MapLongitude = 8;
static int TestBaseId = -1;

enum VehOrders { ORDER_NONE=0, ORDER_CONVOY=3, ORDER_MOVE=25,
    ORDER_MOVE_TO=24, ORDER_ROAD_TO=27, ORDER_MAGTUBE_TO=28 };

static void set_text(int id, const char *text) { StringTable.texts[id] = text; }
static void set_label(int byte_offset, int string_id) {
    LabelIds[byte_offset / 4] = string_id;
}
static const char *label_get(int index) { return StringTable.get(LabelIds[index]); }
static LPSTR encoded_id(int id) { return reinterpret_cast<LPSTR>(static_cast<intptr_t>(id)); }
// ACTUAL_COMPASS_BODY
static const char *parsed_source;
static void parse_says(int, LPCSTR input, int, int) { parsed_source = input; }
static int parse_string(LPSTR input, LPSTR output) {
    strcpy(output, input);
    strcat(output, parsed_source);
    return 0;
}
static int base_at(int, int) { return TestBaseId; }
static void say_base(char *output, int) { strcat(output, "HOME"); }
static int resource_yield(int, int, int, int, int) { return 17; }
static void say_loc(char *output, int x, int y, int, int, int) {
    char point[40];
    sprintf(point, "[%d,%d]", x, y);
    strcat(output, point);
}
static char *_itoa(int value, char *output, int radix) {
    if (radix != 10) abort();
    sprintf(output, "%d", value);
    return output;
}

// ACTUAL_SAY_ORDERS_BODY

static void reset_case() {
    memset(Vehs, 0, sizeof(Vehs));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Order, 0, sizeof(Order));
    memset(Bases, 0, sizeof(Bases));
    memset(TileStorage, 0, sizeof(TileStorage));
    memset(LabelIds, 0, sizeof(LabelIds));
    memset(Compass, 0, sizeof(Compass));
    memset(Resource, 0, sizeof(Resource));
    memset(StringTable.texts, 0, sizeof(StringTable.texts));
    TestBaseId = -1;
}

int main() {
    reset_case();
    Vehs[3].state = 0x02000200;
    Vehs[3].order_auto_type = 11;
    Vehs[3].waypoint_count = 2;
    Vehs[3].patrol_current_point = 1;
    set_label(0x6F4, 21);
    set_text(21, "must not be prefixed");
    Order[ORDER_NONE].order = encoded_id(22);
    Order[ORDER_NONE].order_sea = encoded_id(22);
    set_text(22, "idle");
    char text[512] = "";
    say_orders(text, 3);
    assert(strcmp(text, "idle") == 0);

    reset_case();
    set_text(11, "Route ");
    Vehs[4].state = 0x200;
    Vehs[4].order_auto_type = 10;
    Vehs[4].waypoint_x[1] = 3;
    Vehs[4].waypoint_y[1] = 5;
    TestBaseId = 2;
    strcpy(Bases[2].name_string, "1234567Gaia");
    set_label(0x6F0, 11);
    Order[ORDER_NONE].order = encoded_id(23);
    Order[ORDER_NONE].order_sea = encoded_id(23);
    set_text(23, "idle");
    text[0] = '\0';
    say_orders(text, 4);
    assert(strcmp(text, "(Route 1234567Gaia) idle") == 0);
    assert(parsed_source == Bases[2].name_string);

    reset_case();
    Vehs[5].state = 0x01000200;
    Vehs[5].order = ORDER_MOVE_TO;
    Vehs[5].waypoint_count = 2;
    Vehs[5].waypoint_x[0] = 2; Vehs[5].waypoint_y[0] = 3;
    Vehs[5].waypoint_x[1] = 4; Vehs[5].waypoint_y[1] = 5;
    Vehs[5].waypoint_x[2] = 6; Vehs[5].waypoint_y[2] = 7;
    set_label(0x65C, 12); set_label(0x660, 13);
    set_text(12, "Waypoint"); set_text(13, "then");
    text[0] = '\0';
    say_orders(text, 5);
    assert(strcmp(text, "Waypoint: [2,3] then [4,5] then [6,7]") == 0);

    reset_case();
    Vehs[6].state = 0x4000;
    Vehs[6].order = ORDER_MOVE;
    Vehs[6].waypoint_x[0] = 7;
    Order[26].order = encoded_id(14);
    Compass[7] = encoded_id(15);
    set_text(14, "Unavailable"); set_text(15, "wrongly continued");
    Order[ORDER_MOVE].order = encoded_id(19);
    Order[ORDER_MOVE].order_sea = encoded_id(20);
    set_text(19, "move land"); set_text(20, "move sea");
    text[0] = '\0';
    say_orders(text, 6);
    assert(strcmp(text, "Unavailable") == 0);

    reset_case();
    Vehs[7].order = ORDER_MOVE;
    Vehs[7].waypoint_x[0] = 2;
    Compass[2] = encoded_id(16); set_text(16, "destination detail");
    Order[ORDER_MOVE].order = encoded_id(17);
    Order[ORDER_MOVE].order_sea = encoded_id(18);
    set_text(17, "move land"); set_text(18, "move sea");
    text[0] = '\0';
    say_orders(text, 7);
    assert(strcmp(text, "move sea destination detail") == 0);

    reset_case();
    Vehs[8].order = ORDER_CONVOY;
    Vehs[8].order_auto_type = 1;
    Vehs[8].home_base_id = 5;
    Order[ORDER_CONVOY].order = encoded_id(40);
    Order[ORDER_CONVOY].order_sea = encoded_id(40);
    Resource[1].name_plural = encoded_id(41);
    set_label(0x6C8, 42);
    set_text(40, "Convoy"); set_text(41, "minerals");
    set_text(42, "at");
    TestBaseId = 2;
    text[0] = '\0';
    say_orders(text, 8);
    assert(strcmp(text, "Convoy minerals at HOME") == 0);
    return 0;
}
