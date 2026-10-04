// Actual-source sender host controls; not native ABI or game acceptance.
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl
typedef uint32_t DWORD;
typedef uint32_t DPID;
typedef void *HANDLE;
typedef int BOOL;
#define DP_OK 0
#define FALSE 0
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258U

static int g_queue_adds;
static int g_allocations;
static int g_frees;

struct NetPlayer {
    uint32_t field_0_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t flags_14_;
    char name_and_tail_[64];
};

struct NetThreadInfo {
    HANDLE events_[2];
};

struct QueueStub {
    void add(void *, unsigned long, uint32_t, int, unsigned long) {
        ++g_queue_adds;
    }
};

static uint32_t g_clock;
static int g_wait_mode;
static int g_wait_calls;
static int g_caption_choice;
static int g_caption_calls;
static int g_message_boxes;
static int g_send_failure;
static int g_directplay_send_count;
static DPID g_send_targets[64];
static uint16_t g_wire_types[64];
static DWORD g_wait_timeouts[64];
static HANDLE g_wait_handles[64][2];
static HANDLE g_reset_handles[128];
static int g_reset_count;
static int g_last_parse_count;
static char g_last_parse[2048];
static int g_closed;
static int ExitTurnLoop;
static char StringTemp[2048];

struct DirectPlayStub {
    int Send(DPID, DPID target, DWORD, void *packet, DWORD) {
        if (g_directplay_send_count < 64) {
            g_send_targets[g_directplay_send_count] = target;
            memcpy(&g_wire_types[g_directplay_send_count], packet,
                   sizeof(g_wire_types[0]));
        }
        ++g_directplay_send_count;
        return g_send_failure ? 1 : DP_OK;
    }
};

static DirectPlayStub g_directplay;
static DirectPlayStub *NetDirectPlay = &g_directplay;

class Net {
public:
    NetThreadInfo thread_info_;
    NetPlayer players_[16];
    QueueStub queue_10c_;
    uint32_t field_6E4_;
    uint32_t field_6DC_;
    uint32_t field_E4_;
    uint32_t field_760_;
    uint32_t field_764_;
    uint32_t voice_flags_D8_;

    Net() : field_6E4_(4096), field_6DC_(2), field_E4_(0),
            field_760_(99), field_764_(99), voice_flags_D8_(0) {
        int i;
        thread_info_.events_[0] = reinterpret_cast<HANDLE>(1);
        thread_info_.events_[1] = reinterpret_cast<HANDLE>(2);
        for (i = 0; i < 16; ++i) {
            memset(&players_[i], 0, sizeof(players_[i]));
            strcpy(players_[i].name_and_tail_, "slot-zero");
        }
    }
    void check_polling() {}
    void handle_sys_msg();
    void close() { ++g_closed; }
    int send_packet_type(void *, int, unsigned long, int, int);
    int send(void *, int, unsigned long, int);
};

static Net *g_active_net;

static void *mem_get(int size) {
    ++g_allocations;
    return malloc(static_cast<size_t>(size));
}
static void tracked_free(void *memory) {
    if (memory != 0) ++g_frees;
    free(memory);
}
static DWORD timeGetTime() { return g_clock; }
static BOOL ResetEvent(HANDLE event_handle) {
    if (g_reset_count < 128) g_reset_handles[g_reset_count] = event_handle;
    ++g_reset_count;
    return 1;
}
static DWORD WaitForMultipleObjects(DWORD, const HANDLE *handles, BOOL,
                                    DWORD timeout) {
    ++g_wait_calls;
    if (g_wait_calls <= 64) {
        g_wait_timeouts[g_wait_calls - 1] = timeout;
        g_wait_handles[g_wait_calls - 1][0] = handles[0];
        g_wait_handles[g_wait_calls - 1][1] = handles[1];
    }
    if (g_wait_mode == 3) {
        if (g_wait_calls == 1) {
            g_clock = 20100U;
            return WAIT_TIMEOUT;
        }
        g_clock = 20200U;
        return WAIT_OBJECT_0 + 1U;
    }
    if (g_wait_mode == 1) {
        if (g_wait_calls == 1) {
            g_clock = 1200U;
            return WAIT_TIMEOUT;
        }
        g_clock = 1300U;
        return WAIT_OBJECT_0 + 1U;
    }
    if (g_wait_mode == 2) {
        g_clock = 21000U;
        return WAIT_TIMEOUT;
    }
    return WAIT_TIMEOUT;
}
static int MessageBoxA(void *, const char *, const char *, unsigned int) {
    ++g_message_boxes;
    return 0;
}
static void parse_says(int, char *text, int, int) {
    ++g_last_parse_count;
    strncpy(g_last_parse, text, sizeof(g_last_parse) - 1);
    g_last_parse[sizeof(g_last_parse) - 1] = '\0';
}
static int pop_caption(char *, char *, int (__cdecl *)()) {
    ++g_caption_calls;
    if (g_wait_mode == 3 && g_caption_calls == 1) {
        g_active_net->players_[0].field_C_ = 2000U;
        g_active_net->thread_info_.events_[0] = reinterpret_cast<HANDLE>(3);
        g_active_net->thread_info_.events_[1] = reinterpret_cast<HANDLE>(4);
        return 0;
    }
    return g_caption_choice;
}

void Net::handle_sys_msg() {
    if (g_wait_mode == 1 && g_wait_calls == 2) {
        players_[0].field_8_ = 0;
        players_[1].field_8_ = 0;
        players_[0].field_C_ = 1600U;
        players_[1].field_C_ = 1600U;
    }
    if (g_wait_mode == 3 && g_wait_calls == 2) {
        players_[0].field_8_ = 0;
        players_[0].field_C_ = 1700U;
    }
}

#define free tracked_free
// @@NET_PACKET_TYPE_BODY@@
// @@NET_SEND_BODY@@
#undef free

static void reset_observers() {
    g_clock = 100U;
    g_wait_mode = 0;
    g_wait_calls = 0;
    g_caption_choice = 1;
    g_caption_calls = 0;
    g_message_boxes = 0;
    g_send_failure = 0;
    g_directplay_send_count = 0;
    g_last_parse_count = 0;
    g_last_parse[0] = '\0';
    g_closed = 0;
    ExitTurnLoop = 0;
    memset(g_send_targets, 0, sizeof(g_send_targets));
    memset(g_wire_types, 0, sizeof(g_wire_types));
    memset(g_wait_timeouts, 0, sizeof(g_wait_timeouts));
    memset(g_wait_handles, 0, sizeof(g_wait_handles));
    memset(g_reset_handles, 0, sizeof(g_reset_handles));
    g_reset_count = 0;
    g_queue_adds = 0;
}

static void test_send_broadcast_retry_and_ack_rtt() {
    Net net;
    unsigned char payload = 0x5A;
    int result;
    const unsigned int native_epoch = 100U;
    const unsigned int old_wrong_epoch = native_epoch + 1000U;
    const unsigned int first_wait_now = 1200U;
    const unsigned int native_ack =
        (1300U + (1600U - 1200U) + 0x14U) >> 1;
    const unsigned int old_wrong_ack =
        1300U + 0x14U + ((1600U - 1200U) >> 1);
    // Negative witnesses for the two original v1 formulas: the old shifted
    // epoch suppresses both peer retries; the old ACK grouping yields 1520.
    assert(1000U < first_wait_now - native_epoch);
    assert(1000U >= first_wait_now - old_wrong_epoch);
    assert(native_ack == 860U);
    assert(old_wrong_ack == 1520U);
    assert(old_wrong_ack != native_ack);
    reset_observers();
    g_wait_mode = 1;
    g_active_net = &net;
    net.players_[0].field_0_ = 10;
    net.players_[0].field_8_ = 0;
    net.players_[0].field_C_ = 1000U;
    net.players_[0].flags_14_ = 2U;
    strcpy(net.players_[0].name_and_tail_, "Alpha");
    net.players_[1].field_0_ = 11;
    net.players_[1].field_8_ = 0;
    net.players_[1].field_C_ = 1000U;
    net.players_[1].flags_14_ = 2U;
    strcpy(net.players_[1].name_and_tail_, "Beta");

    result = net.send(&payload, 1, 0, 1);
    assert(result == 1);
    assert(g_directplay_send_count == 3);
    assert(g_send_targets[0] == 0);
    assert(g_send_targets[1] == 10);
    assert(g_send_targets[2] == 11);
    assert(g_wire_types[0] == 4U);
    // Retries sample 1200; callback mutates each live +0xC stamp to 1600,
    // and native grouping is (1300 + (1600 - 1200) + 20) >> 1 = 860.
    assert(net.players_[0].field_C_ == 860U);
    assert(net.players_[1].field_C_ == 860U);
}

static void test_packet_type_broadcast_lists_all_pending_peers() {
    Net net;
    unsigned char payload = 0x3C;
    int result;
    reset_observers();
    g_wait_mode = 2;
    g_caption_choice = 2;
    g_active_net = &net;
    net.players_[0].field_0_ = 10;
    net.players_[0].field_C_ = 50000U;
    net.players_[0].flags_14_ = 2U;
    strcpy(net.players_[0].name_and_tail_, "Alpha");
    net.players_[1].field_0_ = 11;
    net.players_[1].field_C_ = 50000U;
    net.players_[1].flags_14_ = 2U;
    strcpy(net.players_[1].name_and_tail_, "Beta");
    net.players_[2].field_0_ = 12;
    net.players_[2].field_C_ = 50000U;
    net.players_[2].flags_14_ = 2U;
    strcpy(net.players_[2].name_and_tail_, "Gamma");

    result = net.send_packet_type(&payload, 1, 0, 1, 1);
    assert(result == 0);
    assert(g_last_parse_count == 1);
    assert(strcmp(g_last_parse, "AlphaBeta,  Gamma,  .") == 0);
    assert(g_directplay_send_count == 1);
    assert(ExitTurnLoop == 1);
}

static void test_packet_type_target_timeout_names_selected_peer() {
    Net net;
    unsigned char payload = 0x3C;
    int result;
    reset_observers();
    g_wait_mode = 2;
    g_caption_choice = 2;
    g_active_net = &net;
    net.players_[0].field_0_ = 10;
    net.players_[0].field_C_ = 50000U;
    net.players_[0].flags_14_ = 2U;
    strcpy(net.players_[0].name_and_tail_, "Alpha");
    net.players_[1].field_0_ = 11;
    net.players_[1].field_C_ = 50000U;
    net.players_[1].flags_14_ = 2U;
    strcpy(net.players_[1].name_and_tail_, "Beta");
    net.players_[2].field_0_ = 12;
    net.players_[2].field_C_ = 50000U;
    net.players_[2].flags_14_ = 2U;
    strcpy(net.players_[2].name_and_tail_, "Gamma");

    result = net.send_packet_type(&payload, 1, 11, 1, 1);
    assert(result == 0);
    assert(g_last_parse_count == 1);
    assert(strcmp(g_last_parse, "Beta.") == 0);
    assert(g_directplay_send_count == 1);
    assert(ExitTurnLoop == 1);
}

static void test_send_targeted_wait_interval_reloads_live_peer_rtt() {
    Net net;
    unsigned char payload = 0x69;
    reset_observers();
    g_wait_mode = 3;
    g_active_net = &net;
    net.players_[0].field_0_ = 10;
    net.players_[0].field_C_ = 1000U;
    net.players_[0].flags_14_ = 2U;
    strcpy(net.players_[0].name_and_tail_, "Alpha");

    assert(net.send(&payload, 1, 10, 1) == 1);
    assert(g_wait_calls == 2);
    assert(g_wait_timeouts[0] == 1000U);
    assert(g_wait_timeouts[1] == 2000U);
    assert(g_wait_handles[0][0] == reinterpret_cast<HANDLE>(1));
    assert(g_wait_handles[0][1] == reinterpret_cast<HANDLE>(2));
    assert(g_wait_handles[1][0] == reinterpret_cast<HANDLE>(1));
    assert(g_wait_handles[1][1] == reinterpret_cast<HANDLE>(2));
    assert(g_reset_count == 4);
    assert(g_reset_handles[0] == reinterpret_cast<HANDLE>(2));
    assert(g_reset_handles[1] == reinterpret_cast<HANDLE>(1));
    assert(g_reset_handles[2] == reinterpret_cast<HANDLE>(4));
    assert(g_reset_handles[3] == reinterpret_cast<HANDLE>(3));
}

static void test_packet_type_targeted_wait_interval_reloads_live_peer_rtt() {
    Net net;
    unsigned char payload = 0x6A;
    reset_observers();
    g_wait_mode = 3;
    g_active_net = &net;
    net.players_[0].field_0_ = 10;
    net.players_[0].field_C_ = 1000U;
    net.players_[0].flags_14_ = 2U;
    strcpy(net.players_[0].name_and_tail_, "Alpha");

    assert(net.send_packet_type(&payload, 1, 10, 1, 1) == 1);
    assert(g_wait_calls == 2);
    assert(g_wait_timeouts[0] == 1000U);
    assert(g_wait_timeouts[1] == 2000U);
    assert(g_wait_handles[0][0] == reinterpret_cast<HANDLE>(1));
    assert(g_wait_handles[0][1] == reinterpret_cast<HANDLE>(2));
    assert(g_wait_handles[1][0] == reinterpret_cast<HANDLE>(1));
    assert(g_wait_handles[1][1] == reinterpret_cast<HANDLE>(2));
    assert(g_reset_count == 4);
    assert(g_reset_handles[0] == reinterpret_cast<HANDLE>(2));
    assert(g_reset_handles[1] == reinterpret_cast<HANDLE>(1));
    assert(g_reset_handles[2] == reinterpret_cast<HANDLE>(4));
    assert(g_reset_handles[3] == reinterpret_cast<HANDLE>(3));
}

static void test_direct_send_failure_dialogs_are_distinct() {
    Net net;
    unsigned char payload = 0xA5;
    reset_observers();
    g_send_failure = 1;
    assert(net.send(&payload, 1, 10, 0) == 0);
    assert(g_message_boxes == 1);
    assert(g_wire_types[0] == 1U);

    reset_observers();
    g_send_failure = 1;
    assert(net.send_packet_type(&payload, 1, 10, 1, 0) == 0);
    assert(g_message_boxes == 0);
    assert(g_wire_types[0] == 1U);
}

static void test_signed_size_and_player_count_comparisons() {
    Net net;
    unsigned char payload = 0x33;
    reset_observers();
    net.field_6E4_ = 0x80000000U;
    assert(net.send(&payload, 1, 10, 0) == 0);
    assert(g_message_boxes == 1);
    assert(g_directplay_send_count == 0);

    reset_observers();
    net.field_6E4_ = 0x80000000U;
    assert(net.send_packet_type(&payload, 1, 10, 1, 0) == 0);
    assert(g_directplay_send_count == 0);

    reset_observers();
    net.field_6E4_ = 4096U;
    net.field_6DC_ = 0x80000000U;
    net.voice_flags_D8_ = 1U;
    assert(net.send(&payload, 1, 0, 1) == 0);
    assert(g_queue_adds == 1);
    assert(g_directplay_send_count == 0);

    reset_observers();
    net.field_6E4_ = 4096U;
    net.field_6DC_ = 0x80000000U;
    assert(net.send_packet_type(&payload, 1, 0, 1, 4) == 0);
    assert(g_queue_adds == 1);
    assert(g_directplay_send_count == 0);
}

static void test_send_packet_word_for_voice_and_protocol_paths() {
    Net net;
    unsigned char payload = 0x44;
    reset_observers();
    assert(net.send(&payload, 1, 10, 0) == 0);
    assert(g_send_failure == 0);
    assert(g_wire_types[0] == 1U);

    reset_observers();
    net.voice_flags_D8_ = 0x10000000U;
    assert(net.send(&payload, 1, 10, 1) == 1);
    assert(g_wire_types[0] == 1U);

    reset_observers();
    assert(net.send_packet_type(&payload, 1, 10, 0x1234, 0) == 1);
    assert(g_wire_types[0] == 0x1234U);
}

int main() {
    test_send_broadcast_retry_and_ack_rtt();
    test_packet_type_broadcast_lists_all_pending_peers();
    test_packet_type_target_timeout_names_selected_peer();
    test_send_targeted_wait_interval_reloads_live_peer_rtt();
    test_packet_type_targeted_wait_interval_reloads_live_peer_rtt();
    test_direct_send_failure_dialogs_are_distinct();
    test_signed_size_and_player_count_comparisons();
    test_send_packet_word_for_voice_and_protocol_paths();
    assert(g_allocations == g_frees);
    return 0;
}
