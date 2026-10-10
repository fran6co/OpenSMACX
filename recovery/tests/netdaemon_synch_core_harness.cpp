#include <assert.h>
#include <stdint.h>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct BaseSecretProject { int words_[64]; };
#include "src/game_data_state_window.h"
#define GameRules GameDataStateStorage.fields_.game_rules_
#define VehCurrentCount GameDataStateStorage.fields_.vehicle_current_count_
#define BaseCurrentCount GameDataStateStorage.fields_.base_current_count_
#define TurnCurrentNum GameDataStateStorage.fields_.turn_current_num_

#define NETDAEMON_SYNCH_HOST_STUBS
#define VK_CONTROL 0x11

typedef int BOOL;
#define __cdecl
struct Map { unsigned char bytes[44]; };
struct Veh { int16_t x, y; uint32_t state; uint16_t flags; int16_t proto_id;
    uint16_t unk_1; uint8_t faction_id, year_end_lurking, dmg_incurred;
    int8_t order; uint8_t rest[0x34 - 0x12]; };
struct Base { int16_t x, y; uint8_t prefix[0x13 - 4]; char name_string[25]; };
struct VehPrototype { char veh_name[32]; };
struct Console { int faction_id_; };
struct char256 { char str[256]; };
static const int MaxVehProtoNum = 512;
extern Veh Vehs[2049];
extern Base Bases[512];
extern VehPrototype VehPrototypes[MaxVehProtoNum];
extern Console ConsoleGlobal;
extern char256 ParseStrBuffer[10];
class Path { public: void init(); };
class BaseWin { public: void check_base(int); void check_veh(int); };
class NetMsg { public: void pop(const char *, int, int, const char *); void close(); };
extern NetMsg &NetMsgPrimary;
class NetDaemon {
public:
    struct LockWords { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
    struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; uint32_t trailing_words_[10]; } primary_state_;
    LockWords channels_[2];
    struct DeletionRecord { uint8_t acknowledgements_; uint8_t reserved_[3]; int owner_; int16_t count_; int16_t vehicle_ids_[25]; };
    struct DeletionList { DeletionRecord records_[24]; int current_owner_; uint32_t sequence_bits() const { uint32_t x; memcpy(&x, &current_owner_, 4); return x; } void set_sequence_bits(uint32_t x) { memcpy(&current_owner_, &x, 4); } uint32_t advance_sequence() { uint32_t x=sequence_bits()+1; set_sequence_bits(x); return x; } } deletion_list_;
    void *file_index_records_; uint32_t field_1BE4_;
    void send_message(char *message, unsigned long length, int target);
    void synch(int16_t opcode, int a2, int a3, int a4, char *packet,
               int outgoing, int16_t flags);
};
extern int BaseAtDebugPopupFlag;
extern int ConsoleGroupAction;
int __cdecl X_pop(const char *, int (__cdecl *)());
short __cdecl GetAsyncKeyState(int);

/* SOURCE INTERFACE */

BOOL IsMultiplayerNet = 1;
Veh Vehs[2049];
Base Bases[512];
VehPrototype VehPrototypes[MaxVehProtoNum];
Console ConsoleGlobal = { 1 };
char256 ParseStrBuffer[10];
GameDataStateWindow GameDataStateStorage = {};
int ReplayEventBytes = 0x98;
int MapLatitudeBounds = 64;
int MapLongitudeBounds = 64;
uint32_t MapArea = 64;
uint32_t MapLongitude = 64;
static Map MapFixture[64];
Map *MapTiles = MapFixture;
static Map MapFixtureMutated[256];
Path Paths;
BaseWin TutWinBaseWindow;
static NetMsg NetMsgFixture;
NetMsg &NetMsgPrimary = NetMsgFixture;
static uint32_t faction_id = 5;
static bool fail_copy = false;
static int display_count = 0;
static int display_opcode = -1, display_chunk = -1, display_total = -1;
static int commit_count = 0, rebuild_v_count = 0;
static int rebuild_b_count = 0, prefs_count = 0, map_finish_count = 0;
static int base_finish_count = 0, veh_finish_count = 0, diplo_finish_count = 0;
static int social_finish_count = 0;
static int last_social_faction = -1;
static int last_base_check = -1, last_veh_check = -1;
static int map_shutdown_count = 0, map_init_count = 0, path_init_count = 0;
static int callback_payload_fill = -1;
static int popup_count = 0, netmsg_close_count = 0, draws_count = 0;
static int log_count = 0;
int ConsoleControlTurnActive = 0;
int SpockNotifyPending = 0;
static int message_data_count = 0;
static int message_data_opcode = -1;
static int message_data_args[6];
static int debug_popup_count = 0, async_key_count = 0;
static int debug_event_order = 0;
static bool debug_popup_mutates_action = false;
int BaseAtDebugPopupFlag = 0;
int ConsoleGroupAction = 0;
static int parse_num_count = 0, parse_says_count = 0;
static unsigned char *active_packet_bytes = 0;
static int callback_index_override = -1;
static int callback_a2_override = -1, callback_a3_override = -1;
static int last_saved_diplo_a = -1, last_saved_diplo_b = -1;
static uint32_t last_diplomacy_wire[18];
static int copy_in_count = 0, copy_out_count = 0;
static size_t in_offsets[16], in_lengths[16], out_offsets[16], out_lengths[16];
static std::vector< std::vector<unsigned char> > sent;
static Map *out_map_tiles[16], *in_map_tiles[16];
static size_t out_map_capacity[16], in_map_capacity[16];
static size_t out_map_first[16], in_map_first[16];
static Map *mutate_map_on_send = 0;
static Map *mutate_map_on_display = 0;
static int mutate_longitude_on_send = 0;
static int mutate_latitude_on_send = 0;
static int mutate_longitude_on_display = 0;

namespace NetDaemonSyncWire {
uint32_t current_faction() { return faction_id; }
bool copy_out(int16_t opcode, int, int, int, size_t, size_t offset,
              void *bytes, size_t n,
              const MapRouteCapture &map_capture, uint32_t diplomacy[18]) {
    if (fail_copy) return false;
    assert(copy_out_count < 16);
    out_offsets[copy_out_count] = offset; out_lengths[copy_out_count] = n;
    out_map_tiles[copy_out_count] = map_capture.tiles;
    out_map_capacity[copy_out_count] = map_capture.capacity;
    out_map_first[copy_out_count] = map_capture.first;
    ++copy_out_count;
    if (opcode == 0x16 && n == sizeof last_diplomacy_wire)
        memcpy(last_diplomacy_wire, diplomacy, n);
    for (size_t i = 0; i < n; ++i)
        static_cast<unsigned char *>(bytes)[i] =
            static_cast<unsigned char>((offset + i) & 0xFF);
    return true;
}
bool copy_in(int16_t opcode, int, int, int, size_t, size_t offset,
             const void *bytes, size_t n,
             const MapRouteCapture &map_capture, uint32_t diplomacy[18]) {
    if (fail_copy) return false;
    assert(copy_in_count < 16);
    in_offsets[copy_in_count] = offset; in_lengths[copy_in_count] = n;
    in_map_tiles[copy_in_count] = map_capture.tiles;
    in_map_capacity[copy_in_count] = map_capture.capacity;
    in_map_first[copy_in_count] = map_capture.first;
    ++copy_in_count;
    if (n && callback_payload_fill >= 0)
        assert(static_cast<const unsigned char *>(bytes)[0] ==
               static_cast<unsigned char>(callback_payload_fill));
    if (opcode == 0x16 && n == sizeof last_diplomacy_wire)
        memcpy(diplomacy, bytes, n);
    return true;
}
void commit_rules(int16_t) { ++commit_count; }
void gather_saved_diplomacy(int a, int b, uint32_t wire[18]) {
    last_saved_diplo_a = a; last_saved_diplo_b = b;
    for (int i = 0; i < 18; ++i) wire[i] = 0xB0000000u + i;
}

void scatter_saved_diplomacy(int a, int b, const uint32_t wire[18]) {
    last_saved_diplo_a = a; last_saved_diplo_b = b;
    memcpy(last_diplomacy_wire, wire, sizeof last_diplomacy_wire);
    ++diplo_finish_count;
}
}

NetDaemon NetDaemonNet;
int __cdecl parse_num(int, int) { ++parse_num_count; return 0; }
int __cdecl parse_says(int, const char *, int, int) {
    ++parse_says_count; return 0;
}
uint32_t __cdecl message_data(int a, int b, int c, int d, int e, int f) {
    ++message_data_count; message_data_opcode = a;
    message_data_args[0] = a; message_data_args[1] = b;
    message_data_args[2] = c; message_data_args[3] = d;
    message_data_args[4] = e; message_data_args[5] = f;
    return 0;
}
int __cdecl X_pop(const char *, int (__cdecl *)()) {
    assert(debug_event_order == 0);
    debug_event_order = 1;
    ++debug_popup_count;
    if (debug_popup_mutates_action) ConsoleGroupAction = 1;
    return 0;
}
short __cdecl GetAsyncKeyState(int key) {
    assert(key == VK_CONTROL && debug_event_order == 1);
    debug_event_order = 2;
    ++async_key_count;
    return 0;
}
BOOL map_init() { ++map_init_count; return 1; }
void map_shutdown() { ++map_shutdown_count; }
void prefs_use() { ++prefs_count; }
void social_set(uint32_t faction) {
    ++social_finish_count; last_social_faction = static_cast<int>(faction);
}
void rebuild_vehicle_bits() { ++rebuild_v_count; }
void rebuild_base_bits() { ++rebuild_b_count; }
void Path::init() { ++path_init_count; }
void BaseWin::check_base(int id) { ++base_finish_count; last_base_check = id; }
void BaseWin::check_veh(int id) { ++veh_finish_count; last_veh_check = id; }
void NetMsg::pop(const char *, int, int, const char *) { ++popup_count; }
void NetMsg::close() { ++netmsg_close_count; }
void __cdecl do_all_draws() { ++draws_count; }
void __cdecl log_say(const char *, int, int, int) { ++log_count; }
void __cdecl log_say(const char *, const char *, int, int, int) { ++log_count; }

void *mem_get(size_t length) {
    void *p = malloc(length);
    assert(p != 0);
    memset(p, 0xA5, length);
    return p;
}
void NetDaemon::send_message(char *message, unsigned long length, int target) {
    assert(target == 0);
    sent.push_back(std::vector<unsigned char>(message, message + length));
    if (mutate_map_on_send && sent.size() == 1) {
        MapTiles = mutate_map_on_send;
        MapLongitude = mutate_longitude_on_send;
        MapLatitudeBounds = mutate_latitude_on_send;
    }
}
void synch_display(int op, int chunk, int total, int) {
    ++display_count; display_opcode = op; display_chunk = chunk;
    display_total = total;
    if (callback_payload_fill >= 0 && active_packet_bytes)
        memset(active_packet_bytes + 0x24, callback_payload_fill, 0x800);
    if (callback_index_override >= 0 && active_packet_bytes) {
        const int16_t index = static_cast<int16_t>(callback_index_override);
        memcpy(active_packet_bytes + 0x12, &index, sizeof index);
    }
    if (callback_a2_override >= 0 && active_packet_bytes) {
        const int32_t field = callback_a2_override;
        memcpy(active_packet_bytes + 0x14, &field, sizeof field);
    }
    if (callback_a3_override >= 0 && active_packet_bytes) {
        const int32_t field = callback_a3_override;
        memcpy(active_packet_bytes + 0x18, &field, sizeof field);
    }
    if (mutate_map_on_display) {
        MapTiles = mutate_map_on_display;
        MapLongitude = mutate_longitude_on_display;
    }
}

static void reset() {
    sent.clear(); display_count = 0;
    display_opcode = display_chunk = display_total = -1;
    commit_count = rebuild_v_count = rebuild_b_count = 0;
    prefs_count = map_finish_count = base_finish_count = veh_finish_count = 0;
    diplo_finish_count = social_finish_count = 0;
    last_social_faction = last_base_check = last_veh_check = -1;
    map_shutdown_count = map_init_count = path_init_count = 0;
    copy_in_count = copy_out_count = 0; callback_payload_fill = -1;
    mutate_map_on_send = mutate_map_on_display = 0;
    mutate_longitude_on_send = mutate_latitude_on_send = 0;
    mutate_longitude_on_display = 0;
    fail_copy = false; active_packet_bytes = 0;
    GameRules = 0; ConsoleControlTurnActive = 0;
    GameDataStateStorage.fields_.net_turn_faction_ = 0;
    VehCurrentCount = 8; BaseCurrentCount = 8; TurnCurrentNum = 4;
    SpockNotifyPending = 0; message_data_count = 0;
    message_data_opcode = -1; memset(message_data_args, 0, sizeof message_data_args);
    BaseAtDebugPopupFlag = ConsoleGroupAction = 0;
    debug_popup_count = async_key_count = debug_event_order = 0;
    debug_popup_mutates_action = false;
    parse_num_count = parse_says_count = 0;
    memset(&NetDaemonNet, 0, sizeof NetDaemonNet);
    callback_index_override = -1;
    callback_a2_override = callback_a3_override = -1;
    last_saved_diplo_a = last_saved_diplo_b = -1;
    memset(last_diplomacy_wire, 0, sizeof last_diplomacy_wire);
    popup_count = netmsg_close_count = draws_count = log_count = 0;
}
static uint16_t u16(const std::vector<unsigned char> &b, size_t at) {
    uint16_t x; memcpy(&x, &b[at], 2); return x;
}
static uint32_t u32(const std::vector<unsigned char> &b, size_t at) {
    uint32_t x; memcpy(&x, &b[at], 4); return x;
}

/* SOURCE EXTRACTS */
int main() {
    NetDaemon d;
    d.deletion_list_.set_sequence_bits(0x12345678);
    d.primary_state_.leading_words_[0] = 1;

    // Independent expected-size table from the pinned jump table at
    // 0x53412C. First field is hex opcode (decimal shown by its value), then
    // a2, a3, and the independently recorded native payload size.
    struct SizeCase { int opcode; int a2; int a3; int expected; };
    const SizeCase sizes[] = {
        {0x00, 4, 1, 0x398}, {0x01, 4, 1, 1}, {0x02, 4, 1, 0xA9C},
        {0x03, 4, 1, 4}, {0x04, 4, 1, 64 * 44}, {0x05, 4, 1, 0xE00},
        {0x06, 4, 1, 0x20CC}, {0x07, 4, 1, 0x59C}, {0x08, 4, 1, 0x14},
        {0x09, 4, 1, 8}, {0x0A, 4, 1, 4}, {0x0B, 4, 1, 4}, {0x0C, 4, 1, 8},
        {0x0D, 4, 1, 0x14}, {0x0E, 4, 1, 0x34}, {0x0F, 4, 1, 52},
        {0x10, 4, 1, 1}, {0x11, 4, 1, 0x30}, {0x12, 4, 1, 8 * 52},
        {0x13, 4, 1, 0x134}, {0x14, 4, 1, 8 * 308}, {0x15, 4, 1, 4},
        {0x16, 4, 1, 0x48}, {0x17, 4, 1, 4}, {0x18, 4, 1, 0x224},
        {0x19, 4, 1, 0x20}, {0x1A, 4, 1, 4}, {0x1B, 4, 1, 4},
        {0x1C, 4, 1, 4}, {0x1D, 4, 1, 4}, {0x1E, 4, 1, 4}, {0x1F, 4, 1, 4},
        {0x20, 4, 1, 4 * 16}, {0x21, 4, 1, 4}, {0x22, 4, 1, 0x98},
        {0x23, 4, 1, 0x14}, {0x24, 4, 1, 0x98}, {0x25, 4, 1, 4},
        {0x26, 4, 1, 4}, {0x27, 4, 1, 0x104}, {0x28, 4, 1, 0},
        {0x29, 4, 1, 0x1000},
        {0x30, 15, 1, 32 * (64 - 4 * 15) * 44},
        {0x1002, 4, 1, 0xA9C},
        {0x80, 4, 1, 0xF4C}, {0x81, 4, 1, 0x13C},
        {0x82, 4, 1, 0x7C}, {0x83, 4, 1, 0x280},
        {0x84, 4, 1, 0x90}, {0x85, 4, 1, 0xC0},
        {0x86, 4, 1, 0x510}, {0x87, 4, 1, 0x30},
        {0x88, 4, 1, 0x1A0}, {0x89, 4, 1, 0xE0},
        {0x8A, 4, 1, 0x32C}, {0x8B, 4, 1, 0x1920},
        {0x8C, 4, 1, 0x118}, {0x8D, 4, 1, 0x350}
    };
    for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; ++i) {
        reset();
        d.synch(static_cast<int16_t>(sizes[i].opcode), sizes[i].a2,
                sizes[i].a3, 0, 0, 1, 0x21);
        const int n = sizes[i].expected;
        const int chunks = (n + 0x7FF) / 0x800;
        assert(copy_out_count == chunks);
        int total = 0;
        for (int j = 0; j < chunks; ++j) total += static_cast<int>(out_lengths[j]);
        assert(total == n);
    }

    // Single chunk with a remainder: source offset is zero and header fields
    // are exact; untouched native header gap +8..+B remains allocator data.
    reset(); d.synch(0x81, 7, 8, 9, 0, 1, 0x2101);
    assert(sent.size() == 1 && sent[0].size() == 0x13C + 0x24);
    assert(u16(sent[0], 0) == 0x2101 && u32(sent[0], 4) == 5);
    assert(u32(sent[0], 0x0C) == 0x12345678);
    assert(u16(sent[0], 0x10) == 0x81 && u16(sent[0], 0x12) == 0);
    assert(u32(sent[0], 0x14) == 7 && u32(sent[0], 0x18) == 8 &&
           u32(sent[0], 0x1C) == 9);
    assert(sent[0][8] == 0xA5 && sent[0][0xB] == 0xA5);
    assert(sent[0][0x24] == 0 && sent[0][0x25] == 1);
    assert(out_offsets[0] == 0 && out_lengths[0] == 0x13C);

    // Multi-chunk partial final packet: native offset=(chunks-1)*0x800.
    reset(); d.synch(0x8B, 0, 0, 0, 0, 1, 0x2101);
    assert(sent.size() == 4);
    assert(out_offsets[0] == 0 && out_offsets[1] == 0x800 &&
           out_offsets[2] == 0x1000 && out_offsets[3] == 0x1800);
    assert(out_lengths[0] == 0x800 && out_lengths[1] == 0x800 &&
           out_lengths[2] == 0x800 && out_lengths[3] == 0x120);
    assert(sent[3].size() == 0x120 + 0x24 && u16(sent[3], 0x12) == 3);

    // Exact one full chunk (default-size path) is one 0x800 payload.
    reset(); d.synch(0x7F, 0x800, 0, 0, 0, 1, 0x55);
    assert(sent.size() == 1 && sent[0].size() == 0x824);
    assert(out_offsets[0] == 0 && out_lengths[0] == 0x800);

    // Receive displays the packet's initial signed index, then reads the
    // callback-mutated payload at its original pointer before scatter.
    reset();
    std::vector<unsigned char> incoming(0x24 + 0x800, 0x22);
    int16_t op = 0x7F, ix = 0; uint32_t sender = 3;
    memcpy(&incoming[0] + 0x10, &op, 2); memcpy(&incoming[0] + 0x12, &ix, 2);
    memcpy(&incoming[0] + 4, &sender, 4);
    callback_payload_fill = 0x6D; active_packet_bytes = &incoming[0];
    d.channels_[1].trailing_words_[1] = 0xFF;
    d.synch(0x7F, 0x1800, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(display_count == 1 && display_opcode == 0x7F && display_chunk == 1 &&
           display_total == 3);
    assert(copy_in_count == 1 && in_offsets[0] == 0 && in_lengths[0] == 0x800);
    assert(d.channels_[1].trailing_words_[1] == 0xFF);

    // Incoming final partial chunk commits rule staging exactly once and
    // clears only sender's bit at the final packet boundary.
    reset(); d.channels_[1].trailing_words_[1] = 0xFF;
    incoming.assign(0x24 + 0x120, 0x33); op = 0x8B; ix = 3; sender = 2;
    memcpy(&incoming[0] + 0x10, &op, 2); memcpy(&incoming[0] + 0x12, &ix, 2);
    memcpy(&incoming[0] + 4, &sender, 4);
    d.synch(0x8B, 0, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(copy_in_count == 1 && in_offsets[0] == 0x1800 &&
           in_lengths[0] == 0x120);
    assert(commit_count == 1 && d.channels_[1].trailing_words_[1] == 0xFB);

    // Destination and transfer length are resolved from the initial signed
    // packet index, while postprocessing re-reads +0x12 after the callback.
    reset(); d.channels_[1].trailing_words_[1] = 0xFF;
    incoming.assign(0x24 + 0x800, 0x44); op = 0x8B; ix = 2; sender = 1;
    memcpy(&incoming[0] + 0x10, &op, 2); memcpy(&incoming[0] + 0x12, &ix, 2);
    memcpy(&incoming[0] + 4, &sender, 4);
    active_packet_bytes = &incoming[0]; callback_index_override = 3;
    d.synch(0x8B, 0, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(display_chunk == 3 && display_total == 4);
    assert(copy_in_count == 1 && in_offsets[0] == 0x1000 &&
           in_lengths[0] == 0x800);
    assert(commit_count == 1 && d.channels_[1].trailing_words_[1] == 0xFF);

    // One-chunk opcode 0x11 comparison may suppress copy, but epilogue is kept.
    reset(); Vehs[0].x = 9; Vehs[0].y = 10;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    incoming.assign(0x24 + 0x30, 0);
    op = 0x11; ix = 0; memcpy(&incoming[0] + 0x10, &op, 2);
    memcpy(&incoming[0] + 0x12, &ix, 2);
    d.synch(0x11, 0, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(copy_in_count == 0 && veh_finish_count == 1);
    assert(last_veh_check == 0);
    assert(SpockNotifyPending == 1 && message_data_count == 0);
    assert(parse_says_count == 1);

    // The native notification gate queues the same message when the receiver
    // is not held by the singleton daemon; it does not set Spock pending.
    reset(); Vehs[0].x = 9; Vehs[0].y = 10;
    incoming.assign(0x24 + 0x30, 0);
    op = 0x11; ix = 0; memcpy(&incoming[0] + 0x10, &op, 2);
    memcpy(&incoming[0] + 0x12, &ix, 2);
    d.synch(0x11, 0, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(message_data_count == 1 && SpockNotifyPending == 0);

    // Base notifications parse both retained/current names, suppress the
    // scatter on coordinate mismatch, and preserve the incoming final check.
    reset(); Bases[0].x = 17; Bases[0].y = 18;
    incoming.assign(0x24 + 0x134, 0);
    op = 0x13; ix = 0; memcpy(&incoming[0] + 0x10, &op, 2);
    memcpy(&incoming[0] + 0x12, &ix, 2);
    memcpy(&incoming[0] + 0x24 + 0x13, "old base", 9);
    d.synch(0x13, 0, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(copy_in_count == 0 && base_finish_count == 1);
    assert(parse_says_count == 2 && message_data_count == 1);

    // Typed epilogues use the retained caller IDs and preserve the map-lock gate.
    reset();
    d.primary_state_.leading_words_[0] = 1;
    d.synch(0x02, 0, 0, 0, 0, 1, 0x2101);
    assert(map_shutdown_count == 0 && map_init_count == 0 && path_init_count == 0);
    reset();
    d.primary_state_.leading_words_[0] = 0;
    d.synch(0x02, 0, 0, 0, 0, 1, 0x2101);
    assert(map_shutdown_count == 1 && map_init_count == 1 && path_init_count == 1);
    reset();
    d.synch(0x0D, 6, 0, 0, 0, 1, 0x2101);
    assert(social_finish_count == 1 && last_social_faction == 6);
    reset();
    d.synch(0x13, 9, 0, 0, 0, 1, 0x2101);
    assert(base_finish_count == 1 && last_base_check == 9);

    // Empty live record table returns before packet work; network guard is
    // first; size-zero opcode does not invent a packet or callback.
    reset(); VehCurrentCount = 0;
    d.synch(18, 0, 0, 0, 0, 1, 0);
    assert(sent.empty() && copy_out_count == 0);
    VehCurrentCount = 8;
    reset(); IsMultiplayerNet = 0;
    d.synch(0x81, 0, 0, 0, 0, 1, 0);
    assert(sent.empty() && copy_out_count == 0);
    IsMultiplayerNet = 1;
    reset(); d.synch(0x28, 0, 0, 0, 0, 1, 0);
    assert(sent.empty() && display_count == 0);

    // Pair IDs and the native local tuple survive a callback that mutates the
    // incoming packet's a2/a3 words; the epilogue scatters only afterward.
    reset();
    incoming.assign(0x24 + 0x48, 0x5A);
    op = 0x16; ix = 0; sender = 4;
    memcpy(&incoming[0] + 0x10, &op, 2);
    memcpy(&incoming[0] + 0x12, &ix, 2);
    memcpy(&incoming[0] + 0x04, &sender, 4);
    for (int i = 0; i < 18; ++i) {
        const uint32_t word = 0xC0000000u + static_cast<uint32_t>(i);
        memcpy(&incoming[0] + 0x24 + i * 4, &word, 4);
    }
    callback_a2_override = 6; callback_a3_override = 7;
    active_packet_bytes = &incoming[0];
    d.synch(op, 1, 2, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(diplo_finish_count == 1 && last_saved_diplo_a == 1 &&
           last_saved_diplo_b == 2);
    for (int i = 0; i < 18; ++i)
        assert(last_diplomacy_wire[i] == 0xC0000000u + static_cast<uint32_t>(i));

    reset();
    d.synch(0x16, 3, 4, 0, 0, 1, 0x2101);
    assert(last_saved_diplo_a == 3 && last_saved_diplo_b == 4);
    for (int i = 0; i < 18; ++i)
        assert(last_diplomacy_wire[i] == 0xB0000000u + static_cast<uint32_t>(i));

    // The native map-row count is captured once before send callbacks, while
    // the tile pointer/longitude are resolved again for each outgoing chunk.
    reset();
    MapArea = 256; MapLongitudeBounds = 64; MapLatitudeBounds = 64;
    MapLongitude = 64; MapTiles = MapFixture;
    mutate_map_on_send = MapFixtureMutated;
    mutate_longitude_on_send = 32; mutate_latitude_on_send = 16;
    d.synch(0x30, 15, 0, 0, 0, 1, 0x2101);
    assert(copy_out_count == 3 && sent.size() == 3);
    assert(out_map_tiles[0] == MapFixture && out_map_first[0] == 3840);
    assert(out_map_tiles[1] == MapFixtureMutated && out_map_first[1] == 1920);
    assert(out_map_tiles[2] == MapFixtureMutated && out_map_first[2] == 1920);
    assert(out_map_capacity[0] == 256 && out_map_capacity[1] == 256);

    // Receive selects the map destination before synch_display. A display
    // callback changing globals must not redirect this chunk's scatter.
    reset();
    MapArea = 256; MapLongitudeBounds = 64; MapLatitudeBounds = 64;
    MapLongitude = 64; MapTiles = MapFixture;
    incoming.assign(0x24 + 0x800, 0x5C); op = 0x30; ix = 0;
    memcpy(&incoming[0] + 0x10, &op, 2);
    memcpy(&incoming[0] + 0x12, &ix, 2);
    active_packet_bytes = &incoming[0];
    mutate_map_on_display = MapFixtureMutated;
    mutate_longitude_on_display = 32;
    d.synch(0x30, 15, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(copy_in_count == 1 && in_map_tiles[0] == MapFixture);
    assert(in_map_first[0] == 3840 && in_map_capacity[0] == 256);

    // Exhaust all eight combinations of the rule bit, scripted-turn bit, and
    // faction-equality predicate. The singleton daemon lock is held for this
    // table, so the only notification is rules-enabled, non-scripted, and
    // addressed to a different faction. Native message opcode is argument 0.
    for (int rule = 0; rule != 2; ++rule) {
        for (int scripted = 0; scripted != 2; ++scripted) {
            for (int same_faction = 0; same_faction != 2; ++same_faction) {
                reset();
                Vehs[0].x = 9; Vehs[0].y = 10;
                GameRules = rule ? 0x10u : 0;
                ConsoleControlTurnActive = scripted;
                ConsoleGlobal.faction_id_ = 3;
                GameDataStateStorage.fields_.net_turn_faction_ = same_faction ? 3 : 4;
                NetDaemonNet.primary_state_.leading_words_[0] = 1;
                incoming.assign(0x24 + 0x30, 0);
                op = 0x11; ix = 0;
                memcpy(&incoming[0] + 0x10, &op, 2);
                memcpy(&incoming[0] + 0x12, &ix, 2);
                d.synch(0x11, 0, 0, 0,
                    reinterpret_cast<char *>(&incoming[0]), 0, 0);
                const bool should_message = rule && !scripted && !same_faction;
                assert(message_data_count == (should_message ? 1 : 0));
                assert(SpockNotifyPending == (should_message ? 0 : 1));
                if (should_message) {
                    assert(message_data_opcode == 0x2308);
                    for (int i = 1; i < 6; ++i) assert(message_data_args[i] == 0);
                }
            }
        }
    }

    // With the rules branch disabled and the singleton lock open, native
    // message_data is still called with 0x2308 in its first argument.
    reset(); Vehs[0].x = 9; Vehs[0].y = 10;
    incoming.assign(0x24 + 0x30, 0); op = 0x11; ix = 0;
    memcpy(&incoming[0] + 0x10, &op, 2); memcpy(&incoming[0] + 0x12, &ix, 2);
    d.synch(0x11, 0, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(message_data_count == 1 && message_data_opcode == 0x2308);
    assert(SpockNotifyPending == 0);

    // The VEHSYNCH popup precedes the ConsoleGroupAction reload. A popup-side
    // mutation suppresses the subsequent control-key poll; without that
    // mutation GetAsyncKeyState(VK_CONTROL) follows the popup.
    reset(); Vehs[0].x = 9; Vehs[0].y = 10;
    BaseAtDebugPopupFlag = 1; debug_popup_mutates_action = true;
    incoming.assign(0x24 + 0x30, 0); op = 0x11; ix = 0;
    memcpy(&incoming[0] + 0x10, &op, 2); memcpy(&incoming[0] + 0x12, &ix, 2);
    d.synch(0x11, 0, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(debug_popup_count == 1 && async_key_count == 0 && debug_event_order == 1);
    reset(); Vehs[0].x = 9; Vehs[0].y = 10;
    BaseAtDebugPopupFlag = 1;
    incoming.assign(0x24 + 0x30, 0); op = 0x11; ix = 0;
    memcpy(&incoming[0] + 0x10, &op, 2); memcpy(&incoming[0] + 0x12, &ix, 2);
    d.synch(0x11, 0, 0, 0, reinterpret_cast<char *>(&incoming[0]), 0, 0);
    assert(debug_popup_count == 1 && async_key_count == 1 && debug_event_order == 2);

    puts("synch chunk-core controls passed");
    return 0;
}
