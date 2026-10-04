// Typed service projection for actual-source C++98 callback/damage controls.
// No ignored candidate/include file is required to build this translation unit.
#include <cassert>
#include <cstdlib>
#include <cstring>

#define __cdecl
#define rand test_rand
enum { MaxPlayerNum = 8, PLAN_PLANET_BUSTER = 5,
       FAC_FLECHETTE_DEFENSE_SYS = 39 };

struct Veh { int proto_id; unsigned char faction_id; };
struct VehPrototype { int plan; int reactor_id; };
struct Player {
    int is_leader_female, noun_gender, is_noun_plural;
    char adj_name_faction[32], noun_faction[32], title_leader[32], name_leader[32];
};
struct PlayerData { unsigned int satellites_[4], sat_odp_deployed;
                    unsigned int diplo_treaties[MaxPlayerNum]; };
struct Base {
    short x, y;
    unsigned char faction_id_current, facilities_built[12];
    char name_string[32];
};
struct Console {
    int faction_id_;
    void focus(int, int, int) {}
};
struct Effect { int playing; int is_playing() { return playing; } };
struct FX {
    Effect effects[97];
    int plays[8], play_count;
    Effect &effect(int id) { return effects[id]; }
    void play(int id) { plays[play_count++] = id; }
};

Veh Vehs[2049] = {};
VehPrototype VehPrototypes[256] = {};
Player Players[MaxPlayerNum] = {};
PlayerData PlayersData[MaxPlayerNum] = {};
Base Bases[2048] = {};
Console ConsoleGlobal = {};
FX g_FX = {};
int VehBattleUnkTgl[2] = {}, BoomAnimationLevel, TerraformDrownedBaseId;
int MapLongitudeBounds = 32, MapLatitudeBounds = 24, MapIsFlat;
unsigned int MapLongitude = 16;
int GenderDefault, PluralityDefault, IsMultiplayerNet;
unsigned int LockEnableMask;
char ScriptPopupFileStorage[] = "script.txt";
char *ScriptPopupFile = ScriptPopupFileStorage;
const int RadiusRange[9] = {1,9,25,49,81,121,169,225,289};
const int RadiusOffsetX[289] = {
    0,1,2,1,0,-1,-2,-1,0,2,2,-2,-2,1,3,3,1,-1,-3,-3,-1,4,-4,0,0
};
const int RadiusOffsetY[289] = {
    0,-1,0,1,2,1,0,-1,-2,-2,2,2,-2,-3,-1,1,3,3,1,-1,-3,0,0,4,-4
};

enum Event { EVENT_PARSE, EVENT_POPUP, EVENT_SHOOT, EVENT_TERRITORY,
             EVENT_KILL, EVENT_DRAW_RADIUS, EVENT_CLIMATE, EVENT_FX };
int events[256], event_count;
int parse_calls, popup_calls, popup_mutates_base;
int parse_ids[16], base_lookup_id = -1, base_x, base_y;
int base_at_calls, mutate_base_at_call = -1;
int mutate_rand_call = -1, rand_calls, inventory_after_rand;
int random_values[16], random_count, random_index;
int shoot_returns[8], shoot_return_count, shoot_calls, shot_x[8], shot_y[8];
int base_at_id = -1, vehicle_at_id = -1, territory_id = -1;
int kill_calls, base_kill_calls, removed_base, stack_kill_calls;
int climate_calls, lower_calls, draw_calls, map_draw_calls, task_calls;
int atrocity_calls, base_find_calls;
int popup_sound = 0, popup_label_ok = 1;

static void event(Event e) { events[event_count++] = e; }
int test_rand() {
    ++rand_calls;
    if (rand_calls == mutate_rand_call)
        PlayersData[1].satellites_[3] = inventory_after_rand;
    return random_index < random_count ? random_values[random_index++] : 0;
}
int base_at(int x, int y) {
    ++base_at_calls;
    if (base_at_calls == mutate_base_at_call)
        PlayersData[1].satellites_[3] = 0;
    return x == base_x && y == base_y ? base_at_id : -1;
}
int base_find(int, int) { ++base_find_calls; return base_lookup_id; }
int base_find(int, int, int) { ++base_find_calls; return base_lookup_id; }
bool has_fac_built(int facility, int base) {
    assert(facility == FAC_FLECHETTE_DEFENSE_SYS);
    return (Bases[base].facilities_built[4] & 0x80) != 0;
}
int parse_says(int id, const char *, int, int) {
    event(EVENT_PARSE); parse_ids[parse_calls++] = id; return 0;
}
int parse_num(int id, int value) {
    assert(id >= 0 && id < 4);
    parse_ids[8 + id] = value;
    return 0;
}
int popp(char *, const char *label, int, const char *icon, int (*)()) {
    event(EVENT_POPUP); ++popup_calls;
    if (std::strcmp(label, "PLANETBUSTER") == 0) {
        popup_sound = std::strcmp(icon, "baseobl_sm.pcx") == 0 ? 1
                    : std::strcmp(icon, "astp_sm.pcx") == 0 ? 2 : -1;
        if (popup_mutates_base) { Bases[4].x = 14; Bases[4].y = 10; }
    } else if (std::strcmp(icon, "space_sm.pcx") != 0) popup_label_ok = 0;
    return 0;
}
int base_find_sound_stub(int, int) { return -1; }
int veh_at(int, int) { return vehicle_at_id; }
int stack_fix(int id) { return id; }
int shoot(int faction, int defender, int x, int y) {
    (void)faction; (void)defender;
    event(EVENT_SHOOT);
    shot_x[shoot_calls] = x; shot_y[shoot_calls] = y;
    const int call = shoot_calls++;
    return call < shoot_return_count ? shoot_returns[call] : 0;
}
int boom(int, int, int) { return 0; }
int whose_territory(int, int, int, int *, int) {
    event(EVENT_TERRITORY); return territory_id;
}
void kill(int) { event(EVENT_KILL); ++kill_calls; }
void draw_radius(int, int, int, int) { event(EVENT_DRAW_RADIUS); ++draw_calls; }
void world_lower_alt(int, int) { ++lower_calls; }
void world_climate() { event(EVENT_CLIMATE); ++climate_calls; }
void base_kill(int id) { removed_base = id; ++base_kill_calls; }
void stack_kill(int) { ++stack_kill_calls; }
void draw_map(int) { ++map_draw_calls; }
int do_task() { ++task_calls; g_FX.effect(59).playing = 0; return 0; }
void major_atrocity(int, int) { ++atrocity_calls; }
bool on_map(int x, int y) {
    return x >= 0 && x < MapLongitudeBounds && y >= 0 && y < MapLatitudeBounds;
}

// Actual shoot is tested separately from the planet shoot-service double.
#define shoot candidate_shoot
// @@SHOOT_HELPERS_AND_BODY@@
#undef shoot
// @@PLANET_BUSTING_BODY@@

static void reset() {
    std::memset(Vehs, 0, sizeof(Vehs)); std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
    std::memset(Players, 0, sizeof(Players)); std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(Bases, 0, sizeof(Bases)); std::memset(events, 0, sizeof(events));
    event_count = parse_calls = popup_calls = 0; popup_mutates_base = 0;
    base_lookup_id = base_at_id = vehicle_at_id = territory_id = -1;
    base_x = base_y = 0; base_at_calls = 0; mutate_base_at_call = -1;
    mutate_rand_call = -1; rand_calls = inventory_after_rand = 0;
    random_count = random_index = shoot_return_count = shoot_calls = 0;
    std::memset(shoot_returns, 0, sizeof(shoot_returns));
    kill_calls = base_kill_calls = stack_kill_calls = climate_calls = lower_calls = 0;
    draw_calls = map_draw_calls = task_calls = atrocity_calls = base_find_calls = 0;
    popup_sound = 0; popup_label_ok = 1; MapIsFlat = 1;
    IsMultiplayerNet = 0; LockEnableMask = 0;
    ConsoleGlobal.faction_id_ = 1; g_FX.play_count = 0;
    for (int i = 0; i < 97; ++i) g_FX.effects[i].playing = 0;
}

int main() {
    reset();
    assert(shoot_signed_bits(0x80000000u) == (-2147483647 - 1));
    assert(shoot_wrapped_subtract(2147483647, -1) == (-2147483647 - 1));
    assert(shoot_wrapped_subtract((-2147483647 - 1), 1) == 2147483647);

    // Hostile RAND callback changes the inventory: shotsLeft must reload it.
    PlayersData[1].satellites_[3] = 1; mutate_rand_call = 1; inventory_after_rand = 5;
    random_values[0] = 99; random_count = 1; base_lookup_id = 0;
    assert(candidate_shoot(2, 1, 10, 10) == 1);
    assert(parse_ids[9] == 4 && PlayersData[1].sat_odp_deployed == 1);
    assert(PlayersData[1].satellites_[3] == 5);

    // Inventory changes on the last base lookup of the second 25-cell scan.
    reset(); PlayersData[1].satellites_[3] = 1; PlayersData[1].sat_odp_deployed = 1;
    mutate_base_at_call = 50;
    assert(candidate_shoot(2, 1, 10, 10) == 0);
    assert(PlayersData[1].satellites_[3] == 0 && popup_calls == 0);

    // Bonus report bypasses prompt/decrement, and reports the pre-decrement count.
    reset(); PlayersData[1].satellites_[3] = 2; random_values[0] = 99; random_count = 1;
    base_lookup_id = 0;
    assert(candidate_shoot(2, 1, 10, 10) == 1);
    assert(PlayersData[1].satellites_[3] == 2 && PlayersData[1].sat_odp_deployed == 1);
    assert(popup_calls == 1 && parse_ids[8] == 2 && parse_ids[9] == 1);

    // A hostile popup changes base coordinates. Sound was already selected
    // from coordinates read after the four parses but before the popup.
    reset(); Vehs[7].proto_id = 4; Vehs[7].faction_id = 2;
    VehPrototypes[4].plan = PLAN_PLANET_BUSTER; VehPrototypes[4].reactor_id = 1;
    Players[2].is_leader_female = 1; Players[2].noun_gender = 3; Players[2].is_noun_plural = 1;
    std::strcpy(Players[2].title_leader, "title"); std::strcpy(Players[2].name_leader, "name");
    std::strcpy(Players[2].noun_faction, "noun"); base_lookup_id = 4;
    Bases[4].x = 12; Bases[4].y = 10; popup_mutates_base = 1;
    assert(planet_busting(7, 10, 10) == 0);
    assert(parse_calls == 4 && popup_calls == 1 && popup_sound == 1);
    assert(events[0] == EVENT_PARSE && events[3] == EVENT_PARSE && events[4] == EVENT_POPUP);
    assert(Bases[4].x == 14 && popup_label_ok);

    // Asymmetric treaty bits prove row=source faction, col=detected defender.
    reset(); Vehs[8].proto_id = 4; Vehs[8].faction_id = 2;
    VehPrototypes[4].plan = PLAN_PLANET_BUSTER; VehPrototypes[4].reactor_id = 1;
    vehicle_at_id = 9; Vehs[9].faction_id = 3;
    PlayersData[2].diplo_treaties[3] = 1; PlayersData[3].diplo_treaties[2] = 0;
    assert(planet_busting(8, 10, 10) == 0 && shoot_calls == 0);
    reset(); Vehs[8].proto_id = 4; Vehs[8].faction_id = 2;
    VehPrototypes[4].plan = PLAN_PLANET_BUSTER; VehPrototypes[4].reactor_id = 1;
    vehicle_at_id = 9; Vehs[9].faction_id = 3;
    PlayersData[2].diplo_treaties[3] = 0; PlayersData[3].diplo_treaties[2] = 1;
    assert(planet_busting(8, 10, 10) == 0 && shoot_calls == 1);

    // An intercepted radius shot does not skip the later territory callback.
    reset(); Vehs[7].proto_id = 4; Vehs[7].faction_id = 2;
    VehPrototypes[4].plan = PLAN_PLANET_BUSTER; VehPrototypes[4].reactor_id = 1;
    base_x = 11; base_y = 9; base_at_id = 5; Bases[5].faction_id_current = 3;
    territory_id = 4; shoot_returns[0] = 1; shoot_returns[1] = 0; shoot_return_count = 2;
    assert(planet_busting(7, 10, 10) == 0);
    assert(shoot_calls == 2);
    int territory_event = -1, second_shot_event = -1, kill_event = -1;
    int radius_event = -1, climate_event = -1, seen_shots = 0;
    for (int i = 0; i < event_count; ++i) {
        if (events[i] == EVENT_TERRITORY) territory_event = i;
        if (events[i] == EVENT_SHOOT && ++seen_shots == 2) second_shot_event = i;
        if (events[i] == EVENT_KILL) kill_event = i;
        if (events[i] == EVENT_DRAW_RADIUS) radius_event = i;
        if (events[i] == EVENT_CLIMATE) climate_event = i;
    }
    assert(territory_event >= 0 && second_shot_event > territory_event);
    assert(kill_event > second_shot_event && radius_event > kill_event);
    assert(climate_event > radius_event);
    assert(kill_calls == 1 && draw_calls == 1 && climate_calls == 1);
    return 0;
}
