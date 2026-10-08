#include <cassert>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define __stdcall
#define FALSE 0
#define WAIT_OBJECT_0 0
typedef void *HANDLE;
typedef unsigned long DWORD;
typedef unsigned long HRESULT;
typedef unsigned long DPID;
struct DPNAME {
    unsigned long dwSize;
    char *lpszShortNameA;
    char *lpszLongNameA;
};
struct GUID { unsigned long data1, data2, data3, data4; };
typedef int (__stdcall *DPCallback)(unsigned long, unsigned long,
                                    const DPNAME *, unsigned long, void *);
struct FakeDirectPlay {
    unsigned long create_group_result;
    int CreateGroup(unsigned long *, DPNAME *, void *, DWORD, DWORD);
    int GetPlayerData(unsigned long, void *, unsigned long *, unsigned long);
    int Send(unsigned long, unsigned long, unsigned long, void *, unsigned long);
    int EnumGroupPlayers(unsigned long, const GUID *, DPCallback, void *,
                         unsigned long);
};
struct NetPlayer {
    unsigned long field_0_;
    unsigned long field_4_;
    unsigned long field_8_;
    unsigned long field_C_;
    unsigned char flags_14_;
    char pad[3];
    char name_and_tail_[64];
};
struct FakeQueue {
    int adds;
    void *data;
    unsigned long size, group, flags, sequence;
    void add(void *d, unsigned long s, unsigned long g, unsigned long f,
             unsigned long q) {
        ++adds; data=d; size=s; group=g; flags=f; sequence=q;
    }
};
// Accessed members only: this host double does not assert native layout.
struct NetThreadInfo {
    HANDLE events_[5];
};
struct Net {
    uint32_t voice_flags_D8_, field_E4_, field_6DC_, field_6E4_, field_760_;
    unsigned long field_764_;
    FakeQueue queue_10c_;
    NetPlayer players_[16];
    NetThreadInfo thread_info_;
    int poll_count, sys_count, close_count;
    int create_group(char *);
    int send_group(void *, int, long, int);
    int enum_players(unsigned long, unsigned long, const DPNAME *,
                     unsigned long, void *);
    void check_polling() { ++poll_count; }
    void handle_sys_msg() { ++sys_count; }
    void close() { ++close_count; }
};

NetPlayer *NetPlayerTable = 0;
unsigned int NetPlayerWriteIndex = 0;
Net *ActiveNet = 0;
FakeDirectPlay DirectPlayObject = { 0x1234 };
FakeDirectPlay *NetDirectPlay = &DirectPlayObject;
char StringTemp[1032];
int ExitTurnLoop;
static int g_pop_answer;
static int g_send_calls, g_enum_calls, g_message_boxes, g_wait_calls;
static int g_delayed_ack_waits;
static unsigned long g_tick = 100;
static unsigned long g_wait_tick_advance;
static DWORD g_wait_timeout;
static int g_peer_44_retries, g_peer_45_retries;
static int g_enum_saw_published_table;
static int g_enum_saw_null_guid;
static uint16_t g_last_packet_type;
static int g_last_reliable;
static int (__stdcall *g_enum_callback)(unsigned long, unsigned long,
                                       const DPNAME *, unsigned long, void *);
static Net *g_wait_net;
static int g_wait_ack_slot;
static NetPlayer *g_callback_table;
static unsigned int g_callback_index;
static unsigned long g_flags_written;
static unsigned long g_capacity_seen;
static unsigned long g_getplayerdata_flags;
static unsigned long g_getplayerdata_id;
static int g_mutate_selection_in_getplayerdata;

void *mem_get(int n) { return std::malloc(static_cast<size_t>(n)); }
void parse_says(int, char *, int, int) {}
int pop_caption(char *, char *, int (*)()) { return g_pop_answer; }
void MessageBoxA(void *, const char *, const char *, unsigned int) {
    ++g_message_boxes;
}
void ResetEvent(HANDLE) {}
DWORD WaitForMultipleObjects(DWORD, const HANDLE *, int, DWORD timeout) {
    ++g_wait_calls;
    g_wait_timeout = timeout;
    g_tick += g_wait_tick_advance;
    if (g_wait_calls > g_delayed_ack_waits &&
        g_wait_net != 0 && g_wait_ack_slot < 16)
        g_wait_net->players_[g_wait_ack_slot++].field_8_ = 0;
    return 0;
}
DWORD timeGetTime() { return g_tick++; }

int __stdcall group_send_stub(void *, unsigned long, long, int reliable,
                              void *packet, int) {
    ++g_send_calls;
    std::memcpy(&g_last_packet_type, packet, sizeof(g_last_packet_type));
    g_last_reliable = reliable;
    return 0;
}
int __stdcall enum_group_stub(void *, unsigned long, const GUID *guid,
                              void *callback, void *, unsigned long) {
    ++g_enum_calls;
    g_enum_saw_published_table = NetPlayerTable != 0;
    g_enum_saw_null_guid = guid == 0;
    g_enum_callback = reinterpret_cast<int (__stdcall *)(unsigned long,
        unsigned long, const DPNAME *, unsigned long, void *)>(callback);
    if (NetPlayerTable != 0) {
        for (int i=0;i<16;++i) {
            if (NetPlayerTable[i].field_0_ != 0) {
                g_enum_callback(NetPlayerTable[i].field_0_, 0, 0, 0, 0);
            }
        }
    }
    return 0;
}
int FakeDirectPlay::CreateGroup(unsigned long *out, DPNAME *, void *, DWORD, DWORD) {
    *out = create_group_result;
    return 0;
}
int FakeDirectPlay::GetPlayerData(unsigned long id, void *data,
                                  unsigned long *capacity, unsigned long flags) {
    g_getplayerdata_id = id;
    g_capacity_seen = *capacity;
    g_getplayerdata_flags = flags;
    *static_cast<unsigned char *>(data) = static_cast<unsigned char>(g_flags_written);
    if (g_mutate_selection_in_getplayerdata) {
        NetPlayerTable = g_callback_table;
        NetPlayerWriteIndex = g_callback_index;
    }
    return 0;
}
int FakeDirectPlay::Send(unsigned long from, unsigned long to,
                         unsigned long reliable, void *packet,
                         unsigned long size) {
    if (to == 0x44) ++g_peer_44_retries;
    if (to == 0x45) ++g_peer_45_retries;
    return group_send_stub(this, from, static_cast<long>(to),
                           static_cast<int>(reliable), packet,
                           static_cast<int>(size));
}
int FakeDirectPlay::EnumGroupPlayers(unsigned long group, const GUID *guid,
                                     DPCallback callback, void *context,
                                     unsigned long flags) {
    return enum_group_stub(this, group, guid,
                           reinterpret_cast<void *>(callback), context, flags);
}

// Product bodies are extracted from the actual source by the test.
@@SOURCE_BODIES@@
@@ENUM_PLAYERS_BODY@@

int main() {
    Net net;
    std::memset(&net, 0, sizeof(net));
    net.field_6E4_ = 0x1000;
    net.field_760_ = 0x77;
    net.thread_info_.events_[0] = reinterpret_cast<HANDLE>(1);
    net.thread_info_.events_[1] = reinterpret_cast<HANDLE>(2);
    for (int i=0;i<16;++i) net.players_[i].field_0_ = 0;
    net.players_[0].field_0_ = 0x44;
    net.players_[0].flags_14_ = 2;
    net.players_[0].field_C_ = 5;
    std::strcpy(net.players_[0].name_and_tail_, "peer");

    assert(net.create_group(const_cast<char *>("test")) == 0x1234);
    DirectPlayObject.create_group_result = 0;
    assert(net.create_group(const_cast<char *>("test")) == 0);

    char payload[12] = "hello";
    assert(net.send_group(payload, 5, 0x77, 0) == 0);
    assert(g_send_calls == 1 && g_last_packet_type == 1);
    assert(g_last_reliable == 0);

    // Reliable confirmed path must publish this exact Net instance's player
    // storage before enumeration; the callback resolves the member by DPID.
    g_send_calls = g_enum_calls = g_wait_calls = g_enum_saw_published_table = 0;
    net.voice_flags_D8_ = 0;
    g_wait_net = &net;
    assert(net.send_group(payload, 5, 0x99, 1) == 1);
    assert(g_enum_calls == 1 && g_enum_saw_published_table && g_enum_saw_null_guid);
    assert(NetPlayerTable == net.players_);
    assert(net.players_[0].field_8_ == 0);
    assert(g_send_calls == 1 && g_wait_calls == 1);

    // If EnumGroupPlayers finds no flagged member, the confirmed path returns
    // the new sequence without sending or waiting. The shared pointer is still
    // the actual receiver's member array for this enumeration lifetime.
    net.players_[0].flags_14_ = 0;
    g_send_calls = g_enum_calls = g_wait_calls = 0;
    assert(net.send_group(payload, 5, 0x99, 1) == 2);
    assert(g_enum_calls == 1 && g_enum_saw_published_table && g_enum_saw_null_guid);
    assert(g_send_calls == 0 && g_wait_calls == 0);

    // The voice/local queue route is a separate pre-allocation branch.
    net.voice_flags_D8_ = 1;
    const int prior_sends = g_send_calls;
    const int prior_adds = net.queue_10c_.adds;
    assert(net.send_group(payload, 5, 0x77, 0) == 2);
    assert(net.queue_10c_.adds == prior_adds + 1);
    assert(net.queue_10c_.data == payload && net.queue_10c_.size == 5);
    assert(g_send_calls == prior_sends);

    net.voice_flags_D8_ = 0;
    net.field_6E4_ = 4;
    const int polls_before_reject = net.poll_count;
    assert(net.send_group(payload, 5, 0x77, 0) == 0);
    assert(net.poll_count == polls_before_reject);

    // Direct callback controls: match+flag marks, nonmatch is a successful
    // enumeration continuation but cannot modify a row.
    net.players_[0].flags_14_ = 2;
    net.players_[0].field_8_ = 0;
    assert(g_enum_callback(0x44, 0, 0, 0, 0) == 1);
    assert(net.players_[0].field_8_ == 1);
    assert(g_enum_callback(0x45, 0, 0, 0, 0) == 1);
    assert(net.players_[0].field_8_ == 1);
    NetPlayerTable = 0;
    assert(g_enum_callback(0x44, 0, 0, 0, 0) == 0);

    // enum_players passes the native zero flags argument and one-byte output
    // location, while preserving the original DPID across callback mutation
    // of both shared selector objects.
    NetPlayer first_table[4];
    NetPlayer callback_table[4];
    std::memset(first_table, 0, sizeof(first_table));
    std::memset(callback_table, 0, sizeof(callback_table));
    NetPlayerTable = first_table;
    NetPlayerWriteIndex = 0;
    net.voice_flags_D8_ = 2;
    net.field_764_ = 0;
    ActiveNet = &net;
    g_callback_table = callback_table;
    g_callback_index = 1;
    g_flags_written = 1;
    g_mutate_selection_in_getplayerdata = 1;
    DPNAME player_name;
    player_name.dwSize = sizeof(player_name);
    player_name.lpszShortNameA = 0;
    player_name.lpszLongNameA = const_cast<char *>("Remote Player");
    assert(net.enum_players(0x12345678, 0, &player_name, 0, 0) == 1);
    assert(g_getplayerdata_id == 0x12345678);
    assert(g_capacity_seen == 1 && g_getplayerdata_flags == 0);
    assert(first_table[0].field_0_ == 0x12345678);
    assert(first_table[0].flags_14_ == 0 && first_table[0].name_and_tail_[0] == 0);
    assert(callback_table[1].flags_14_ == 3); // returned bit + voice bit
    assert(std::strcmp(callback_table[1].name_and_tail_, "Remote Player") == 0);
    assert(net.field_764_ == 0x12345678); // test returned low bit set below
    assert(NetPlayerWriteIndex == 2);

#ifdef APPROVED_ALL_ACTIVE_PEERS
    // Approved repair: service every callback-selected peer, not the native
    // indeterminate stack entries after the first, and wait for both ACKs.
    for (int i = 0; i < 16; ++i) {
        net.players_[i].field_0_ = 0;
        net.players_[i].field_8_ = 0;
        net.players_[i].flags_14_ = 0;
    }
    net.players_[0].field_0_ = 0x44;
    net.players_[0].flags_14_ = 2;
    net.players_[0].field_C_ = 1000;
    net.players_[1].field_0_ = 0x45;
    net.players_[1].flags_14_ = 2;
    net.players_[1].field_C_ = 1000;
    net.field_6E4_ = 0x1000;
    net.voice_flags_D8_ = 0;
    g_wait_ack_slot = 0;
    g_wait_net = &net;
    g_enum_calls = g_send_calls = g_wait_calls = 0;
    assert(net.send_group(payload, 5, 0x99, 1) == 3);
    assert(g_enum_calls == 1 && g_send_calls == 1 && g_enum_saw_null_guid);
    assert(g_wait_calls == 2);
    assert(net.players_[0].field_8_ == 0 && net.players_[1].field_8_ == 0);

    // Hold back both ACKs beyond their deadlines: each actual peer must receive
    // an individual retry before the next waits acknowledge them separately.
    net.players_[0].field_C_ = 1;
    net.players_[1].field_C_ = 1;
    g_wait_ack_slot = 0;
    g_wait_calls = g_send_calls = 0;
    g_peer_44_retries = g_peer_45_retries = 0;
    g_delayed_ack_waits = 1;
    g_wait_tick_advance = 10;
    assert(net.send_group(payload, 5, 0x99, 1) == 4);
    assert(g_wait_calls == 3 && g_wait_timeout == 1);
    assert(g_peer_44_retries >= 1 && g_peer_45_retries >= 1);
    assert(net.players_[0].field_8_ == 0 && net.players_[1].field_8_ == 0);
#endif

    std::puts("net group host controls passed");
}
