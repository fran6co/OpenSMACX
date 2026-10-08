// Proposed bounded harness for actual Net group-send source bodies.
#include <cassert>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define __stdcall
#define FALSE 0
#define WAIT_OBJECT_0 0
struct StopAtFirstWait {};
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
struct NetThreadInfo { HANDLE events_[5]; };
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
static int g_send_calls, g_enum_calls, g_wait_calls;
static DWORD g_wait_timeout;
static int g_enum_saw_published_table, g_enum_saw_null_guid;
static uint16_t g_last_packet_type;
static int g_last_reliable;
static int (__stdcall *g_enum_callback)(unsigned long, unsigned long,
                                        const DPNAME *, unsigned long, void *);
static NetPlayer *g_callback_table;
static unsigned int g_callback_index;
static unsigned long g_flags_written;
static unsigned long g_capacity_seen, g_getplayerdata_flags, g_getplayerdata_id;
static int g_mutate_selection_in_getplayerdata;

void *mem_get(int n) { return std::malloc(static_cast<size_t>(n)); }
void parse_says(int, char *, int, int) {}
int pop_caption(char *, char *, int (*)()) { return 0; }
void MessageBoxA(void *, const char *, const char *, unsigned int) {}
void ResetEvent(HANDLE) {}
DWORD WaitForMultipleObjects(DWORD, const HANDLE *, int, DWORD timeout) {
    ++g_wait_calls;
    g_wait_timeout = timeout;
    throw StopAtFirstWait();
}
DWORD timeGetTime() { return 100; }

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
        for (int i = 0; i < 16; ++i) {
            if (NetPlayerTable[i].field_0_ != 0)
                g_enum_callback(NetPlayerTable[i].field_0_, 0, 0, 0, 0);
        }
    }
    return 0;
}
int FakeDirectPlay::CreateGroup(unsigned long *out, DPNAME *, void *, DWORD,
                                DWORD) {
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

@@PRODUCT_CALLBACK_CREATE_SEND@@
@@PRODUCT_ENUM_PLAYERS@@

int main() {
    Net net;
    std::memset(&net, 0, sizeof(net));
    net.field_6E4_ = 0x1000;
    net.field_760_ = 0x77;
    net.thread_info_.events_[0] = reinterpret_cast<HANDLE>(1);
    net.thread_info_.events_[1] = reinterpret_cast<HANDLE>(2);
    char payload[12] = "hello";

    assert(net.create_group(const_cast<char *>("test")) == 0x1234);
    DirectPlayObject.create_group_result = 0;
    assert(net.create_group(const_cast<char *>("test")) == 0);
    DirectPlayObject.create_group_result = 0x1234;

    // No provider exits before polling, allocation, or retry locals.
    const int polls_before_reject = net.poll_count;
    NetDirectPlay = 0;
    assert(net.create_group(const_cast<char *>("test")) == 0);
    assert(net.send_group(payload, 5, 0x99, 1) == 0);
    assert(net.poll_count == polls_before_reject);
    NetDirectPlay = &DirectPlayObject;

    // Ordinary unreliable send finishes before any acknowledgement state.
    assert(net.send_group(payload, 5, 0x99, 0) == 0);
    assert(g_send_calls == 1 && g_last_packet_type == 1 && g_last_reliable == 0);

    // First DPID has no ACK flag; second DPID is ACK-selected. Original
    // first-DPID selection must pass delay 7 to the first wait.
    net.players_[0].field_0_ = 0x44;
    net.players_[0].flags_14_ = 0;
    net.players_[0].field_C_ = 7;
    net.players_[1].field_0_ = 0x45;
    net.players_[1].flags_14_ = 2;
    net.players_[1].field_C_ = 91;
    bool stopped = false;
    try {
        net.send_group(payload, 5, 0x99, 1);
    } catch (const StopAtFirstWait &) {
        stopped = true;
    }
    assert(stopped);
    assert(g_enum_calls == 1 && g_enum_saw_published_table && g_enum_saw_null_guid);
    assert(net.players_[0].field_8_ == 0 && net.players_[1].field_8_ == 1);
    assert(g_send_calls == 2 && g_last_packet_type == 4 && g_last_reliable == 0);
    assert(g_wait_calls == 1 && g_wait_timeout == 7);

    // Callback contract controls execute after the bounded stop.
    NetPlayerTable = net.players_;
    assert(g_enum_callback(0x44, 0, 0, 0, 0) == 1);
    assert(g_enum_callback(0x45, 0, 0, 0, 0) == 1);
    assert(g_enum_callback(0x46, 0, 0, 0, 0) == 1);
    NetPlayerTable = 0;
    assert(g_enum_callback(0x44, 0, 0, 0, 0) == 0);

    // Enum-player callback mutation keeps the actual shared table/index reads.
    NetPlayer first_table[4], callback_table[4];
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
    assert(g_getplayerdata_id == 0x12345678 && g_capacity_seen == 1);
    assert(g_getplayerdata_flags == 0);
    assert(first_table[0].field_0_ == 0x12345678);
    assert(callback_table[1].flags_14_ == 3);
    assert(std::strcmp(callback_table[1].name_and_tail_, "Remote Player") == 0);
    assert(net.field_764_ == 0x12345678 && NetPlayerWriteIndex == 2);
    std::puts("bounded actual-source Net group controls passed");
}
