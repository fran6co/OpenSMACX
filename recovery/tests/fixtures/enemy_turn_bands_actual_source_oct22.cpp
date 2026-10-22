#include <cassert>
#include <vector>

#define __cdecl

enum { ABL_DROP_POD = 0x10, PFLAG_UNK_10000 = 0x10000,
       ORDER_SENTRY_BOARD = 1, ORDER_MOVE_TO = 24 };

struct GameDataFields { int vehicle_current_count_; };
struct GameDataStorage { GameDataFields fields_; };
static GameDataStorage GameDataStateStorage;
struct Vehicle {
    int faction_id;
    int proto_id;
    int dmg_incurred;
    int order;
    int moves_expended;
};
typedef Vehicle Veh;
static Vehicle Vehs[8];
struct Prototype { int plan; int unk_1; int chassis_id; };
static Prototype VehPrototypes[8];
struct ChassisRecord { int triad; };
static ChassisRecord Chassis[8];
struct PlayerRecord { unsigned int flags; };
static PlayerRecord PlayersData[8];

static std::vector<int> ability_calls;
static std::vector<int> stack_calls;
static std::vector<int> arty_calls;
static std::vector<int> speed_calls;
static std::vector<int> enemy_calls;
static std::vector<int> skip_calls;
static bool drop_pod_ability;
static bool arty_result;
static bool stack_result;
static int speed_result;
static int enemy_stop_after;
static int mutate_vehicle_count_on_enemy;

static int has_abil(int proto_id, int ability) {
    assert(ability == ABL_DROP_POD);
    ability_calls.push_back(proto_id);
    return drop_pod_ability;
}
static int can_arty(int proto_id, int mode) {
    assert(mode == 1); arty_calls.push_back(proto_id); return arty_result;
}
static int stack_check(int veh_id, int a, int b, int c, int d) {
    assert(a == 3 && b == 0 && c == -1 && d == -1);
    stack_calls.push_back(veh_id); return stack_result;
}
static int speed(int veh_id, int mode) {
    assert(mode == 0); speed_calls.push_back(veh_id); return speed_result;
}
static int enemy_veh(int veh_id) {
    enemy_calls.push_back(veh_id);
    if (mutate_vehicle_count_on_enemy)
        --GameDataStateStorage.fields_.vehicle_current_count_;
    return enemy_stop_after > 0 && static_cast<int>(enemy_calls.size()) >= enemy_stop_after;
}
static void veh_skip(int veh_id) {
    skip_calls.push_back(veh_id);
    Vehs[veh_id].order = ORDER_SENTRY_BOARD;
}

/* ACTUAL_ENEMY_TURN_BODY */

static void reset_fixture(int vehicle_count) {
    GameDataStateStorage.fields_.vehicle_current_count_ = vehicle_count;
    for (int i = 0; i < 8; ++i) {
        Vehs[i] = Vehicle{1, i, 1, 0, 0};
        VehPrototypes[i] = Prototype{6, 1, i};
        Chassis[i] = ChassisRecord{0};
        PlayersData[i].flags = 0xFFFFFFFFu;
    }
    ability_calls.clear(); stack_calls.clear(); arty_calls.clear();
    speed_calls.clear(); enemy_calls.clear(); skip_calls.clear();
    drop_pod_ability = false; arty_result = false; stack_result = true;
    speed_result = 1; enemy_stop_after = 1;
    mutate_vehicle_count_on_enemy = false;
}

static void ready_late_band_fixture(int faction_id, int count) {
    reset_fixture(count);
    drop_pod_ability = true; // lower bands reach but fail the common filter
    for (int i = 0; i < count; ++i) {
        Vehs[i].faction_id = faction_id;
        Vehs[i].proto_id = i;
        Vehs[i].order = 0;
        VehPrototypes[i] = Prototype{7, 1, i};
        Chassis[i].triad = 0;
    }
}

static void check_late_band_dispatch() {
    ready_late_band_fixture(2, 1);
    enemy_turn(2);
    assert((enemy_calls == std::vector<int>{0, 0, 0})); // priorities 7, 8, 9
    assert((speed_calls == std::vector<int>{0, 0})); // priority 8 first turn skips speed
    assert(!ability_calls.empty()); // lower bands still evaluate shared filters
    assert(PlayersData[2].flags == (0xFFFFFFFFu & ~PFLAG_UNK_10000));
}

static void check_early_band_predicates() {
    // Plan6, ground triad, damaged, no artillery: only bands5/6/7/9.
    reset_fixture(1);
    enemy_turn(1);
    assert(enemy_calls.size() == 4);

    reset_fixture(1);
    arty_result = true; // band0 adds one action
    enemy_turn(1);
    assert(enemy_calls.size() == 5 && arty_calls.size() == 1);

    reset_fixture(1);
    VehPrototypes[0].plan = 2; // band1's unconditional plan branch
    enemy_turn(1);
    assert(enemy_calls.size() == 5 && stack_calls.empty());

    reset_fixture(1);
    VehPrototypes[0].plan = 7; // band1 via stack and band8 both qualify
    enemy_turn(1);
    assert(enemy_calls.size() == 6 && stack_calls.size() == 1);

    reset_fixture(1);
    VehPrototypes[0].plan = 7;
    stack_result = false; // band8 remains live while band1 fails
    enemy_turn(1);
    assert(enemy_calls.size() == 5 && stack_calls.size() == 1);

    reset_fixture(1);
    Chassis[0].triad = 2; // band2 accepts a non-plan4 aircraft
    enemy_turn(1);
    assert(enemy_calls.size() == 5);

    reset_fixture(1);
    Chassis[0].triad = 2;
    VehPrototypes[0].plan = 4; // band2 and other early bands reject plan4
    enemy_turn(1);
    assert(enemy_calls.size() == 2);

    reset_fixture(1);
    VehPrototypes[0].plan = 1; // band3
    enemy_turn(1);
    assert(enemy_calls.size() == 5);

    reset_fixture(1);
    Vehs[0].dmg_incurred = 0; // band4
    enemy_turn(1);
    assert(enemy_calls.size() == 5);
}

static void check_late_bands_bypass_filters() {
    ready_late_band_fixture(2, 1);
    drop_pod_ability = false; // isolate the plan4 filter from the ability filter
    VehPrototypes[0].plan = 4;
    enemy_turn(2);
    assert((enemy_calls == std::vector<int>{0, 0})); // priorities 7 and 9; 8 guard rejects

    ready_late_band_fixture(2, 1);
    drop_pod_ability = false; // isolate the unk_1 filter from the ability filter
    VehPrototypes[0].unk_1 = 0x15;
    enemy_turn(2);
    assert((enemy_calls == std::vector<int>{0, 0, 0}));
}

static void check_priority_eight_guard_and_decrement() {
    ready_late_band_fixture(0, 1);
    enemy_turn(0);
    assert((enemy_calls == std::vector<int>{0, 0})); // priorities 7 and 9 only

    ready_late_band_fixture(2, 2);
    VehPrototypes[1].plan = 6; // priority 8 rejects the higher-id vehicle
    enemy_turn(2);
    assert((enemy_calls == std::vector<int>{1, 0, 0, 1, 0}));
}

static void check_priority_eight_speed_exemption() {
    ready_late_band_fixture(2, 1);
    speed_result = 0;
    enemy_turn(2);
    assert((enemy_calls == std::vector<int>{0})); // p8 skips its initial speed test
    assert(!speed_calls.empty()); // p7/p9 were stopped by zero moves

    ready_late_band_fixture(2, 1);
    speed_result = -1;
    enemy_turn(2);
    assert((enemy_calls == std::vector<int>{0}));
}

static void check_sentry_wrong_faction_and_empty_population() {
    ready_late_band_fixture(2, 1);
    Vehs[0].order = ORDER_SENTRY_BOARD;
    enemy_turn(2);
    assert(enemy_calls.empty() && speed_calls.empty());

    ready_late_band_fixture(2, 1);
    Vehs[0].faction_id = 3;
    enemy_turn(2);
    assert(enemy_calls.empty());

    reset_fixture(0);
    enemy_turn(2);
    assert(enemy_calls.empty() && speed_calls.empty());
    assert(PlayersData[2].flags == (0xFFFFFFFFu & ~PFLAG_UNK_10000));
}

static void check_live_count_mutation_stops_sweep() {
    ready_late_band_fixture(2, 1);
    mutate_vehicle_count_on_enemy = true;
    enemy_stop_after = 100;
    enemy_turn(2);
    assert((enemy_calls == std::vector<int>{0}));
    assert(GameDataStateStorage.fields_.vehicle_current_count_ == 0);
}

static void check_skip_threshold_and_order_effects() {
    ready_late_band_fixture(2, 1);
    enemy_stop_after = 18;
    Vehs[0].order = 0; // non-move order and non-air triad add two per turn
    enemy_turn(2);
    assert(skip_calls.size() == 1 && skip_calls[0] == 0);
    assert(enemy_calls.size() == 17); // 34 turns_spent, then strictly > 0x20
    assert(Vehs[0].order == ORDER_SENTRY_BOARD);

    ready_late_band_fixture(2, 1);
    enemy_stop_after = 100;
    Vehs[0].order = ORDER_MOVE_TO;
    Chassis[0].triad = 2; // one turn_spent increment per action
    enemy_turn(2);
    assert(skip_calls.size() == 1 && enemy_calls.size() == 33);
}

int main() {
    check_early_band_predicates();
    check_late_band_dispatch();
    check_late_bands_bypass_filters();
    check_priority_eight_guard_and_decrement();
    check_priority_eight_speed_exemption();
    check_sentry_wrong_faction_and_empty_population();
    check_live_count_mutation_stops_sweep();
    check_skip_threshold_and_order_effects();
}

