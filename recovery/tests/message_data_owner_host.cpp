#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef uint32_t DWORD;
#ifndef __cdecl
#define __cdecl
#endif

struct FakeLockState {
    uint32_t leading_words_[4];
    union Payload {
        uint32_t flags;
        // Only the null store is projected; this is not a host pointer ABI.
        uint32_t vehicle_selection_follow;
    } payload_;
    uint32_t trailing_words_[10];
};
static_assert(offsetof(FakeLockState, payload_) == 0x10, "native payload word");
static_assert(offsetof(FakeLockState, trailing_words_) == 0x14, "native counter word");
static_assert(sizeof(FakeLockState) == 0x3C, "native lock-state stride");

struct FakeConsole {
    int faction_id_;
} ConsoleGlobal;

int IsMultiplayerNet;
int NetBusyCount;
int NetTurnFlags;
int ConsoleControlTurnActive;
int NetTurnFaction;
uint32_t fake_time;
int fake_time_calls;
int fake_send_result;
int fake_send_calls;
int fake_busy_at_send;
uint32_t fake_target;
unsigned long fake_length;
int fake_send_flags;
uint32_t fake_payload[8];
int fake_who_calls;
int fake_who_argument;
int fake_log_calls;

void log_say(const char *, int faction, int first, int second) {
    assert(faction == ConsoleGlobal.faction_id_ && first == 0 && second == 0);
    ++fake_log_calls;
}

uint32_t timeGetTime() {
    ++fake_time_calls;
    return ++fake_time;
}

class Net {
 public:
    int send(void *data, int length, unsigned long target, int flags) {
        ++fake_send_calls;
        fake_busy_at_send = NetBusyCount;
        fake_target = static_cast<uint32_t>(target);
        fake_length = static_cast<unsigned long>(length);
        fake_send_flags = flags;
        unsigned char *bytes = static_cast<unsigned char *>(data);
        for (int i = 0; i < 8; ++i) {
            fake_payload[i] = 0;
            const int offset = 4 + i * 4;
            if (offset + 4 <= length)
                memcpy(&fake_payload[i], bytes + offset, 4);
        }
        return fake_send_result;
    }
};

class AlphaNet {
 public:
    int who_2_pid(int faction) {
        ++fake_who_calls;
        fake_who_argument = faction;
        return faction + 100;
    }
};

class NetDaemon : public Net, public AlphaNet {
 public:
    int field_764_;
    FakeLockState lock_state_[3];
    int send_message(char *message, unsigned long length, int target);
    uint32_t unlock_veh();
};

NetDaemon NetDaemonNet;

// ACTUAL_MESSAGE_DATA_BODIES

static void reset_observations() {
    fake_send_calls = 0;
    fake_busy_at_send = -1;
    fake_target = 0;
    fake_length = 0;
    fake_send_flags = 0;
    fake_who_calls = 0;
    fake_who_argument = 0;
}

int main() {
    unsigned char packet[0x20];
    memset(packet, 0, sizeof packet);
    fake_send_result = 0x13572468;
    IsMultiplayerNet = 1;
    NetDaemonNet.field_764_ = 88;
    NetDaemonNet.lock_state_[0].leading_words_[0] = 0x11223344;
    NetDaemonNet.lock_state_[0].trailing_words_[0] = 1;
    ConsoleGlobal.faction_id_ = 7;
    NetBusyCount = 4;
    fake_time = 900;
    fake_time_calls = 0;
    reset_observations();

    const int result = message_data(0x4000, 27, 0x101, 0x202, 0x303, 0x404);
    assert(result == fake_send_result);
    assert(fake_send_calls == 1 && fake_target == 0 && fake_length == 0x20);
    assert(fake_send_flags == 1 && fake_busy_at_send == 5 && NetBusyCount == 4);
    assert(fake_time_calls == 2);
    assert(fake_payload[0] == 0x00000007); // record +4: faction
    assert(fake_payload[1] == 902);        // record +8: 0x4000 refreshes time
    assert(fake_payload[2] == 0x11223344); // record +0xC: send sequence
    assert(fake_payload[3] == 0x101);      // record +0x10
    assert(fake_payload[4] == 0x202);      // record +0x14
    assert(fake_payload[5] == 0x303);      // record +0x18
    assert(fake_payload[6] == 0x404);      // record +0x1C

    // The transport-disabled native path leaves EAX as the initial zero flag.
    IsMultiplayerNet = 0;
    reset_observations();
    assert(NetDaemonNet.send_message(reinterpret_cast<char *>(packet),
                                     sizeof packet, 13) == 0);
    assert(fake_send_calls == 0);
    fake_time_calls = 0;
    reset_observations();
    assert(message_data(0x1234, 13, 1, 2, 3, 4) == 0);
    assert(fake_send_calls == 0 && fake_busy_at_send == -1);
    assert(NetBusyCount == 4 && fake_time_calls == 1);

    IsMultiplayerNet = 1;
    fake_send_result = -37;
    uint16_t flags = 0x8000;
    memcpy(packet, &flags, sizeof flags);
    NetDaemonNet.lock_state_[0].trailing_words_[0] = 1;
    reset_observations();
    assert(NetDaemonNet.send_message(reinterpret_cast<char *>(packet),
                                     sizeof packet, 13) == -37);
    assert(fake_target == 13 && fake_who_calls == 0);

    NetDaemonNet.lock_state_[0].trailing_words_[0] = 2;
    reset_observations();
    assert(NetDaemonNet.send_message(reinterpret_cast<char *>(packet),
                                     sizeof packet, 13) == -37);
    assert(fake_target == 88 && fake_who_calls == 0);

    // The earlier 0x8000 test takes precedence when both bits are set.
    flags = 0xA000;
    memcpy(packet, &flags, sizeof flags);
    NetTurnFlags = 0x10;
    ConsoleControlTurnActive = 0;
    NetTurnFaction = 3;
    reset_observations();
    assert(NetDaemonNet.send_message(reinterpret_cast<char *>(packet),
                                     sizeof packet, 13) == -37);
    assert(fake_target == 88 && fake_who_calls == 0);

    // The native mask at the third branch is 0xA0 after the earlier 0x80
    // branch; reaching it requires 0x2000 without 0x8000.
    flags = 0x2000;
    memcpy(packet, &flags, sizeof flags);
    NetDaemonNet.lock_state_[0].trailing_words_[0] = 2;
    NetTurnFlags = 0x10;
    ConsoleControlTurnActive = 0;
    NetTurnFaction = 3;
    reset_observations();
    assert(NetDaemonNet.send_message(reinterpret_cast<char *>(packet),
                                     sizeof packet, 13) == -37);
    assert(fake_target == 103 && fake_who_calls == 1 &&
           fake_who_argument == 3);

    flags = 0;
    memcpy(packet, &flags, sizeof flags);
    NetDaemonNet.lock_state_[0].trailing_words_[0] = 2;
    reset_observations();
    assert(NetDaemonNet.send_message(reinterpret_cast<char *>(packet),
                                     sizeof packet, 13) == -37);
    assert(fake_target == 113 && fake_who_calls == 1 &&
           fake_who_argument == 13);

    // Caller closure: all returned bits survive the original cleanup stores.
    memset(NetDaemonNet.lock_state_, 0x55, sizeof NetDaemonNet.lock_state_);
    NetDaemonNet.lock_state_[0].trailing_words_[0] = 1;
    fake_log_calls = 0;
    reset_observations();
    assert(NetDaemonNet.unlock_veh() == static_cast<uint32_t>(-37));
    assert(fake_log_calls == 1 && fake_send_calls == 1 && fake_target == 0);
    assert(NetDaemonNet.lock_state_[2].leading_words_[3] == 0);
    assert(NetDaemonNet.lock_state_[2].leading_words_[2] == 0);
    assert(NetDaemonNet.lock_state_[2].trailing_words_[5] == 0);
    assert(NetDaemonNet.lock_state_[2].trailing_words_[4] == 0);
    assert(NetDaemonNet.lock_state_[2].trailing_words_[3] == 0);
    assert(NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow == 0);
    assert(NetBusyCount == 4);

    IsMultiplayerNet = 0;
    NetDaemonNet.lock_state_[2].leading_words_[3] = 77;
    NetDaemonNet.lock_state_[2].trailing_words_[5] = 88;
    NetDaemonNet.lock_state_[2].trailing_words_[3] = 99;
    NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow = 123;
    fake_log_calls = 0;
    reset_observations();
    assert(NetDaemonNet.unlock_veh() == 0);
    assert(fake_log_calls == 0 && fake_send_calls == 0);
    assert(NetDaemonNet.lock_state_[2].leading_words_[3] == 77);
    assert(NetDaemonNet.lock_state_[2].trailing_words_[5] == 88);
    assert(NetDaemonNet.lock_state_[2].trailing_words_[3] == 0);
    assert(NetDaemonNet.lock_state_[1].payload_.vehicle_selection_follow == 0);
    return 0;
}
