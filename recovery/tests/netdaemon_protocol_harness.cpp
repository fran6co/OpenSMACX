/* Strict host-only fixture for actual-owner NetDaemon action/order bodies. */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

typedef uint32_t DWORD;
#define WINAPI
static const int PLAN_INFO_WARFARE = 11;
static const int ORDER_MOVE_TO = 24;
static const int ORDER_AI_MOVE_TO = 88;
static const int ORDER_ROAD_TO = 27;
static const int ORDER_MAGTUBE_TO = 28;

struct LockStateEntry { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct Lock { int checks; Lock() : checks(0) {} void check_global() { ++checks; } };
static char test_message;
struct Net {
    int gets;
    int next_message;
    Net() : gets(0), next_message(0) {}
    void *get(unsigned long *first, unsigned long *second) {
        ++gets; *first = 17; *second = 23;
        return next_message ? &test_message : 0;
    }
};
struct NetDaemon : public Net {
    LockStateEntry lock_state_[24];
    Lock locks_;
    NetDaemon();
    int automated_lock(int *, int);
    int lock_veh(int *, int, int, int, int);
    int add_global();
    int wait(const char *, uint32_t *, int, int);
    void process_message(char *, unsigned long, int);
    int action(int, int);
    int order_veh(int, int, int);
};
NetDaemon::NetDaemon() { memset(lock_state_, 0, sizeof(lock_state_)); }

struct Veh {
    int16_t x, y;
    uint32_t state;
    int16_t proto_id;
    uint8_t faction_id;
    int8_t order;
    uint8_t moves_expended;
    uint8_t probe_action;
    int16_t prev_veh_id_stack;
};
struct Base { uint8_t faction_id_current; int8_t population_size; };
struct PlayerData { uint32_t diplo_treaties[8]; };
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk_1, territory; uint32_t bit, bit2, bit_visible[7]; };
struct VehPrototype { uint8_t plan; };
struct Console { int faction_id_; };
struct NetMsg {
    std::vector<std::string> popups;
    int closes;
    NetMsg() : closes(0) {}
    void pop(const char *s, int, int, const char *) { popups.push_back(s); }
    void close() { ++closes; }
};

static const int MaxVehicles = 2049;
static const int MaxBases = 512;
static const int MapCellCount = 64 * 64;
static Veh Vehs[MaxVehicles];
static Base Bases[MaxBases];
static PlayerData PlayersData[8];
static Map MapTiles[MapCellCount];
static VehPrototype VehPrototypes[64];
static int VehMoveDeltaX[9], VehMoveDeltaY[9];
static int RadiusOffsetX[289], RadiusOffsetY[289];
static int IsMultiplayerNet, NetBusyCount, NetDaemonWaitTicks, ExitTurnLoop;
// Host projection of the actual game-data owner, not a target ABI model.
struct GameDataLockMasksHost { uint32_t enable_mask_; };
struct GameDataFieldsHost { GameDataLockMasksHost lock_masks_; };
struct GameDataStorageHost { GameDataFieldsHost fields_; };
static GameDataStorageHost GameDataStateStorage;
static int MapLongitudeBounds, MapLatitudeBounds, MapLongitude, MapIsFlat;
static Console ConsoleGlobal;
static NetMsg NetMsgPrimary;
static NetDaemon NetDaemonNet;

static NetDaemon *active_receiver;
static int auto_lock_result, auto_lock_calls, auto_lock_category;
static int mutate_order_on_auto_lock, order_after_callback;
static int lock_vehicle_result, lock_vehicle_calls;
static int *add_global_results, add_global_result_count, add_global_calls;
static int free_action_result, free_action_calls, free_action_veh;
static int free_order_result, free_order_calls, free_order_veh, free_order_x, free_order_y;
static int go_to_calls, road_to_calls, go_to_veh, road_to_veh;
static int action_packets, move_packets, cancel_packets;
static int wait_calls, wait_value, process_calls, do_all_non_input_calls;
static int check_spock_calls, synch_veh_calls;
static int base_at_result, base_at_x, base_at_y;
static int planet_buster_result, stack_fix_result, stack_check_result;
static int vehicle_at_result, vehicle_at_calls, vehicle_at_x[32], vehicle_at_y[32];
static int speed_values[MaxVehicles], time_value, time_step;
static int callback_change_net_after_wait, callback_change_net_after_synch;
static int callback_change_exit_after_non_input;
static uint32_t message_data_last_opcode;
static std::vector<std::string> logs;
static std::vector<int> log_arg1, log_arg2;

DWORD timeGetTime() { time_value += time_step; return static_cast<DWORD>(time_value); }
int action(int veh) { ++free_action_calls; free_action_veh = veh; return free_action_result; }
void action_go_to(int veh) { ++go_to_calls; go_to_veh = veh; }
void action_road_to(int veh) { ++road_to_calls; road_to_veh = veh; }
void wait_task() {
    if (active_receiver) active_receiver->lock_state_[2].leading_words_[3] = 0;
    if (callback_change_net_after_wait) IsMultiplayerNet = 0;
}
void check_spock() { ++check_spock_calls; }
void do_all_non_input() {
    ++do_all_non_input_calls;
    if (callback_change_exit_after_non_input) ExitTurnLoop = 1;
}
void log_say(const char *s, int a, int b, int) {
    logs.push_back(s); log_arg1.push_back(a); log_arg2.push_back(b);
}
int message_data(int opcode, int, int, int, int, int) {
    message_data_last_opcode = static_cast<uint32_t>(opcode);
    if (opcode == 0x2212) ++cancel_packets;
    return 0;
}
void message_veh(int opcode, int, int, int) {
    if (opcode == 0x2404) ++action_packets;
    if (opcode == 0x2401) ++move_packets;
}
void synch_veh(int) {
    ++synch_veh_calls;
    if (callback_change_net_after_synch) IsMultiplayerNet = 0;
}
int base_at(int x, int y) { base_at_x = x; base_at_y = y; return base_at_result; }
int planet_buster(int) { return planet_buster_result; }
int stack_fix(int v) { return v < 0 ? v : stack_fix_result; }
int stack_check(int, int, int, int, int) { return stack_check_result; }
int veh_at(int x, int y) {
    if (vehicle_at_calls < 32) {
        vehicle_at_x[vehicle_at_calls] = x;
        vehicle_at_y[vehicle_at_calls] = y;
    }
    ++vehicle_at_calls;
    return vehicle_at_result;
}
int speed(int veh, int) { return speed_values[veh]; }
int order_veh(int veh, int x, int y) {
    ++free_order_calls; free_order_veh = veh; free_order_x = x; free_order_y = y;
    return free_order_result;
}

int NetDaemon::automated_lock(int *veh, int category) {
    ++auto_lock_calls; auto_lock_category = category;
    if (mutate_order_on_auto_lock) Vehs[*veh].order = static_cast<int8_t>(order_after_callback);
    return auto_lock_result;
}
int NetDaemon::lock_veh(int *veh, int, int, int, int) {
    ++lock_vehicle_calls;
    if (lock_vehicle_result == 0) {
        lock_state_[1].payload_.vehicle_selection_follow = veh;
        lock_state_[2].trailing_words_[3] = 1;
    }
    return lock_vehicle_result;
}
int NetDaemon::add_global() {
    const int i = add_global_calls++;
    return i < add_global_result_count ? add_global_results[i] : 0;
}
int NetDaemon::wait(const char *, uint32_t *flag, int, int) {
    ++wait_calls; *flag = 0;
    if (callback_change_net_after_wait) IsMultiplayerNet = 0;
    return wait_value;
}
void NetDaemon::process_message(char *, unsigned long, int) {
    ++process_calls;
    NetDaemonNet.lock_state_[1].leading_words_[3] = 0x77777777u;
}

/*ACTUAL_METHODS*/

static void reset_fixture(NetDaemon *receiver = 0) {
    memset(Vehs, 0, sizeof(Vehs)); memset(Bases, 0, sizeof(Bases));
    memset(PlayersData, 0, sizeof(PlayersData)); memset(MapTiles, 0, sizeof(MapTiles));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(VehMoveDeltaX, 0, sizeof(VehMoveDeltaX));
    memset(VehMoveDeltaY, 0, sizeof(VehMoveDeltaY));
    memset(RadiusOffsetX, 0, sizeof(RadiusOffsetX));
    memset(RadiusOffsetY, 0, sizeof(RadiusOffsetY));
    memset(NetDaemonNet.lock_state_, 0, sizeof(NetDaemonNet.lock_state_));
    if (receiver) memset(receiver->lock_state_, 0, sizeof(receiver->lock_state_));
    NetDaemonNet.gets = 0; NetDaemonNet.next_message = 0;
    IsMultiplayerNet = 1; NetBusyCount = 0; NetDaemonWaitTicks = 0; ExitTurnLoop = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xff00u;
    MapLongitudeBounds = 128; MapLatitudeBounds = 64;
    MapLongitude = 64; MapIsFlat = 0; ConsoleGlobal.faction_id_ = 2;
    active_receiver = 0; auto_lock_result = auto_lock_calls = auto_lock_category = 0;
    mutate_order_on_auto_lock = 0; order_after_callback = 0;
    lock_vehicle_result = lock_vehicle_calls = 0;
    add_global_results = 0; add_global_result_count = add_global_calls = 0;
    free_action_result = free_action_calls = free_action_veh = 0;
    free_order_result = free_order_calls = free_order_veh = free_order_x = free_order_y = 0;
    go_to_calls = road_to_calls = go_to_veh = road_to_veh = 0;
    action_packets = move_packets = cancel_packets = wait_calls = wait_value = 0;
    process_calls = do_all_non_input_calls = check_spock_calls = synch_veh_calls = 0;
    base_at_result = -1; base_at_x = base_at_y = 0;
    planet_buster_result = stack_fix_result = stack_check_result = 0;
    vehicle_at_result = -1; vehicle_at_calls = 0;
    memset(vehicle_at_x, 0, sizeof(vehicle_at_x)); memset(vehicle_at_y, 0, sizeof(vehicle_at_y));
    memset(speed_values, 0, sizeof(speed_values));
    time_value = 0; time_step = 1; callback_change_net_after_wait = 0;
    callback_change_net_after_synch = 0;
    callback_change_exit_after_non_input = 0; message_data_last_opcode = 0;
    logs.clear(); log_arg1.clear(); log_arg2.clear();
    NetMsg &popup = NetMsgPrimary;
    popup.popups.clear(); popup.closes = 0;
    Vehs[1].prev_veh_id_stack = -1;
    Vehs[2].prev_veh_id_stack = -1;
}

static void test_action_offline_and_classification() {
    int followed_vehicle = 99;
    NetDaemon subject; reset_fixture(&subject);
    IsMultiplayerNet = 0; free_action_result = 37;
    assert(subject.action(12, 0) == 37 && free_action_calls == 1 && free_action_veh == 12);

    reset_fixture(&subject); Vehs[12].order = 0x14;
    subject.lock_state_[2].leading_words_[3] = 0;
    assert(subject.action(12, 0) == 1);
    assert(auto_lock_calls == 1 && auto_lock_category == 0xC && action_packets == 1);

    reset_fixture(&subject); Vehs[12].order = ORDER_MOVE_TO;
    assert(subject.action(12, 1) == 1 && go_to_calls == 1 && go_to_veh == 12);
    assert(action_packets == 0);
    reset_fixture(&subject); Vehs[12].order = ORDER_ROAD_TO;
    assert(subject.action(12, 1) == 1 && road_to_calls == 1 && road_to_veh == 12);
    reset_fixture(&subject); Vehs[12].order = 3;
    subject.lock_state_[1].payload_.vehicle_selection_follow = &followed_vehicle; subject.lock_state_[2].trailing_words_[3] = 99;
    assert(subject.action(12, 0) == 0);
    assert(subject.lock_state_[1].payload_.vehicle_selection_follow == &followed_vehicle && subject.lock_state_[2].trailing_words_[3] == 99);
    assert(cancel_packets == 0);
    assert(subject.action(12, 1) == 0);
    assert(subject.lock_state_[1].payload_.vehicle_selection_follow == 0 && subject.lock_state_[2].trailing_words_[3] == 0);
    assert(cancel_packets == 1);
}

static void test_action_callback_reload_and_singleton_timing() {
    NetDaemon subject; reset_fixture(&subject);
    Vehs[12].order = 4;
    mutate_order_on_auto_lock = 1; order_after_callback = 0x15;
    subject.lock_state_[2].leading_words_[3] = 1;
    active_receiver = &subject;
    NetDaemonNet.lock_state_[1].leading_words_[3] = 0x12345678u;
    subject.lock_state_[1].leading_words_[3] = 0xabcdef01u;
    NetDaemonNet.next_message = 0x100;
    time_step = 2501;
    assert(subject.action(12, 0) == 1);
    assert(auto_lock_calls == 1 && auto_lock_category == 0);
    assert(action_packets == 1 && process_calls == 1 && check_spock_calls == 1);
    assert(NetBusyCount == 0 && NetDaemonNet.lock_state_[1].leading_words_[3] == 0x12345678u);
    assert(subject.lock_state_[1].leading_words_[3] == 0xabcdef01u);
    assert(NetMsgPrimary.closes == 1);
    bool logged_reloaded_order = false;
    for (size_t i = 0; i < logs.size(); ++i)
        if (logs[i] == "Net.Action Sending ACTION message to server" && log_arg2[i] == 0x15)
            logged_reloaded_order = true;
    assert(logged_reloaded_order);
}

static void test_order_offline_repeat_lock_and_speed_edges() {
    NetDaemon subject; reset_fixture(&subject);
    IsMultiplayerNet = 0; free_order_result = 8;
    assert(subject.order_veh(7, 4, 0) == 8);
    assert(free_order_calls == 1 && free_order_veh == 7 && free_order_x == 4 && free_order_y == 3);

    reset_fixture(&subject); subject.lock_state_[2].trailing_words_[5] = 0; subject.lock_state_[2].leading_words_[2] = 1;
    assert(subject.order_veh(7, 0, 0) == 0 && subject.lock_state_[2].trailing_words_[5] == 1);
    assert(NetMsgPrimary.popups.empty());
    assert(subject.order_veh(7, 0, 0) == 0 && subject.lock_state_[2].trailing_words_[5] == 2);
    assert(NetMsgPrimary.popups.size() == 1 && NetMsgPrimary.popups[0] == "MOVEPENDING");
    reset_fixture(&subject); subject.lock_state_[2].trailing_words_[5] = 0; subject.lock_state_[2].leading_words_[2] = 1;
    subject.lock_state_[2].trailing_words_[3] = 1;
    assert(subject.order_veh(7, 0, 0) == 0 && subject.lock_state_[2].trailing_words_[5] == 0);
    reset_fixture(&subject); lock_vehicle_result = 1;
    assert(subject.order_veh(7, 0, 0) == 0 && lock_vehicle_calls == 1);

    const int expended[] = {20, 20, 20, 20};
    const int speeds[] = {19, 20, 1019, 1020};
    for (int i = 0; i < 4; ++i) {
        reset_fixture(&subject); Vehs[7].x = 40; Vehs[7].y = 20;
        Vehs[7].moves_expended = static_cast<uint8_t>(expended[i]);
        speed_values[7] = speeds[i]; free_order_result = 1;
        const int result = subject.order_veh(7, 4, 1);
        if (i < 2) assert(result == 0 && move_packets == 0);
        else assert(result == 1 && move_packets == 1);
    }
}

static void test_order_asymmetric_treaty_and_radius_loop() {
    NetDaemon subject; reset_fixture(&subject);
    Vehs[7].x = 64; Vehs[7].y = 32; Vehs[7].faction_id = 2;
    Vehs[7].moves_expended = 1; speed_values[7] = 2;
    free_order_result = 1; planet_buster_result = 0;
    PlayersData[2].diplo_treaties[3] = 0;
    PlayersData[3].diplo_treaties[2] = 0x10;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ |= 1u << 3;
    for (int i = 0; i < MapCellCount; ++i) { MapTiles[i].bit = 3; MapTiles[i].val2 = 3; }
    vehicle_at_result = 1; Vehs[1].faction_id = 3; Vehs[1].moves_expended = 1;
    speed_values[1] = 2;
    callback_change_exit_after_non_input = 1;
    assert(subject.order_veh(7, 4, 1) == 1);
    assert(vehicle_at_calls == 21); // Native offsets 0..0x53 step by four: 0 through 0x50.
    assert(do_all_non_input_calls == 1);
}

static void test_order_planet_buster_uses_own_row() {
    NetDaemon subject; reset_fixture(&subject);
    Vehs[7].x = 40; Vehs[7].y = 20; Vehs[7].faction_id = 2;
    Vehs[1].faction_id = 3;
    Vehs[7].moves_expended = 1; speed_values[7] = 2;
    planet_buster_result = 1; vehicle_at_result = 1; stack_fix_result = 1;
    free_order_result = 1;
    PlayersData[2].diplo_treaties[3] = 0; // hostile: own faction -> other faction
    PlayersData[3].diplo_treaties[2] = 1; // asymmetric reverse must not affect decision
    int failures[] = {1};
    add_global_results = failures; add_global_result_count = 1;
    assert(subject.order_veh(7, 0, 1) == 0);
    assert(add_global_calls == 1 && move_packets == 0);
}

static void test_globalfailed_wait_and_popup() {
    NetDaemon subject; reset_fixture(&subject);
    Vehs[7].order = 6; Vehs[7].state = 0x80200U;
    Vehs[7].x = 40; Vehs[7].y = 20; Vehs[7].faction_id = 2;
    Vehs[7].proto_id = 4; VehPrototypes[4].plan = PLAN_INFO_WARFARE;
    Vehs[7].moves_expended = 1; speed_values[7] = 2;
    Bases[1].faction_id_current = 3; Bases[1].population_size = 3;
    base_at_result = 1; free_order_result = 1; stack_check_result = 0;
    PlayersData[2].diplo_treaties[3] = 1; // not a good-base hostile entry
    int failures[] = {1}; add_global_results = failures; add_global_result_count = 1;
    wait_value = 19;
    assert(subject.order_veh(7, 0, 0) == 0);
    assert(synch_veh_calls == 1 && wait_calls == 1 && NetDaemonWaitTicks == 19);
    assert(Vehs[7].order == 0 && Vehs[7].state == 0x80000U);
    assert(cancel_packets == 1 && subject.lock_state_[2].trailing_words_[3] == 0);
    assert(NetMsgPrimary.popups.size() == 1 && NetMsgPrimary.popups[0] == "GLOBALFAILED");
}

static void test_mode_zero_wait_mutates_network_gate() {
    NetDaemon subject; reset_fixture(&subject);
    Vehs[7].x = 40; Vehs[7].y = 20; Vehs[7].faction_id = 2;
    Vehs[7].moves_expended = 1; speed_values[7] = 2;
    free_order_result = 1; wait_value = 23;
    callback_change_net_after_wait = 1;
    assert(subject.order_veh(7, 0, 0) == 1);
    assert(move_packets == 1 && wait_calls == 1 && NetDaemonWaitTicks == 23);
    assert(cancel_packets == 0); // callback changed the live multiplayer gate before release
    assert(subject.lock_state_[1].payload_.vehicle_selection_follow == 0 && subject.lock_state_[2].trailing_words_[3] == 0);
}

static void test_sync_callback_mutates_network_gate() {
    NetDaemon subject; reset_fixture(&subject);
    Vehs[7].x = 40; Vehs[7].y = 20; Vehs[7].faction_id = 2;
    Vehs[7].proto_id = 4; VehPrototypes[4].plan = PLAN_INFO_WARFARE;
    Vehs[7].moves_expended = 1; speed_values[7] = 2;
    Bases[1].faction_id_current = 3; Bases[1].population_size = 3;
    base_at_result = 1; free_order_result = 1; stack_check_result = 0;
    PlayersData[2].diplo_treaties[3] = 1;
    int failures[] = {1}; add_global_results = failures; add_global_result_count = 1;
    callback_change_net_after_synch = 1;
    assert(subject.order_veh(7, 0, 0) == 0);
    assert(synch_veh_calls == 1 && wait_calls == 0 && NetDaemonWaitTicks == 0);
    assert(cancel_packets == 0 && subject.lock_state_[2].trailing_words_[3] == 0);
    assert(NetMsgPrimary.popups.size() == 1 && NetMsgPrimary.popups[0] == "GLOBALFAILED");
}

static void test_early_rejection_does_not_sync_vehicle() {
    NetDaemon subject; reset_fixture(&subject);
    Vehs[7].x = 40; Vehs[7].y = 20; Vehs[7].faction_id = 2;
    Vehs[7].order = 6; Vehs[7].state = 0x80000U;
    Vehs[7].moves_expended = 20; speed_values[7] = 20;
    free_order_result = 1;
    assert(subject.order_veh(7, 0, 0) == 0);
    assert(synch_veh_calls == 0 && Vehs[7].order == 6 && Vehs[7].state == 0x80000U);
    assert(wait_calls == 0 && move_packets == 0 && cancel_packets == 1);
    assert(subject.lock_state_[1].payload_.vehicle_selection_follow == 0 && subject.lock_state_[2].trailing_words_[3] == 0);
}

int main(int argc, char **argv) {
    if (argc == 1 || strcmp(argv[1], "action") == 0)
        test_action_offline_and_classification();
    if (argc == 1 || strcmp(argv[1], "timing") == 0)
        test_action_callback_reload_and_singleton_timing();
    if (argc == 1 || strcmp(argv[1], "order_speed") == 0)
        test_order_offline_repeat_lock_and_speed_edges();
    if (argc == 1 || strcmp(argv[1], "radius") == 0)
        test_order_asymmetric_treaty_and_radius_loop();
    if (argc == 1 || strcmp(argv[1], "direction") == 0)
        test_order_planet_buster_uses_own_row();
    if (argc == 1 || strcmp(argv[1], "globalfailed") == 0)
        test_globalfailed_wait_and_popup();
    if (argc == 1 || strcmp(argv[1], "mode0wait") == 0)
        test_mode_zero_wait_mutates_network_gate();
    if (argc == 1 || strcmp(argv[1], "syncgate") == 0)
        test_sync_callback_mutates_network_gate();
    if (argc == 1 || strcmp(argv[1], "earlyreject") == 0)
        test_early_rejection_does_not_sync_vehicle();
    return 0;
}
