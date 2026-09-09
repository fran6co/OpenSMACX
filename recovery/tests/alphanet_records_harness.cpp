// Live record and AlphaNet declarations/bodies are inserted by pytest.
// Transport, ledger and Heap are instrumented collaborators, not game recovery.
#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
int events[16], event_count;
void event(int value) { assert(event_count < 16); events[event_count++] = value; }

class Net {
 public:
  Net() { event(1); }
  ~Net() { event(7); }
  virtual void on_net_event_0(int, int) {}
  virtual void on_player_added(char *, unsigned long) {}
  virtual void on_player_deleted(char *, unsigned long) {}
  virtual void on_player_name_changed(char *, unsigned long) {}
  virtual void on_session_lost() {}
  virtual void on_host_changed() {}
  char storage_[0x764];
  void close() { event(4); }
};
struct AlphaNetLedger {
  char storage_[0x40];
  AlphaNetLedger() { event(2); }
  ~AlphaNetLedger() { event(6); }
};
struct Heap {
  char storage_[0x14];
  Heap() { event(3); }
  ~Heap() { event(5); }
};

// LIVE_PLAYER_RECORD
// LIVE_ALPHANET_DECLARATION
// LIVE_ALPHANET_CTOR
// LIVE_ALPHANET_DTOR
// LIVE_ALPHANET_CLOSE
// LIVE_PID_INDEX
// LIVE_PID_IDENTITY
// LIVE_IDENTITY_PID
// LIVE_IDENTITY_INDEX
void AlphaNet::on_net_event_0(int, int) {}
void AlphaNet::on_player_name_changed(char *, unsigned long) {}
void AlphaNet::on_session_lost() {}
void AlphaNet::on_host_changed() {}

int main() {
    assert(sizeof(Net) == 0x768);
    assert(sizeof(AlphaNetProcessSlot) == 0x19C);
    assert(offsetof(AlphaNetProcessSlot, pid_) == 0x20);
    assert(offsetof(AlphaNetProcessSlot, identity_) == 0x24);
    assert(offsetof(AlphaNetProcessSlot, status_bytes_) == 0x28);
    assert(offsetof(AlphaNet, player_count_) == 0x768);
    assert(offsetof(AlphaNet, process_slots_) == 0x76C);
    assert(offsetof(AlphaNet, ledger_) == 0x144C);
    assert(offsetof(AlphaNet, heap_) == 0x148C);
    assert(sizeof(AlphaNet) == 0x14A0);
    {
        AlphaNet net;
        assert(event_count == 3 && events[0] == 1 && events[1] == 2 && events[2] == 3);
        assert(net.player_count_ == 0);
        for (int i = 0; i < 8; ++i) {
            AlphaNetProcessSlot &slot = net.process_slots_[i];
            assert(slot.pid_ == 0 && slot.identity_ == -1 && slot.field_25_ == 0);
            assert(slot.field_26_ == 0 && slot.field_27_ == -1 && slot.status_bytes_[0] == 2);
            memset(slot.name_, 'a' + i, sizeof(slot.name_));
            slot.pid_ = 100 + i;
            slot.identity_ = static_cast<int8_t>(i == 7 ? -7 : i);
            slot.status_bytes_[0] = static_cast<uint8_t>(i);
        }
        // Slot zero is reserved. All lookups inspect precisely slots 1..7.
        assert(net.pid_2_idx(100) == 0 && net.pid_2_who(100) == 0);
        assert(net.who_2_idx(0) == 0 && net.who_2_pid(0) == 0);
        for (int j = 1; j < 8; ++j) {
            int identity = j == 7 ? -7 : j;
            assert(net.pid_2_idx(100 + j) == j);
            assert(net.pid_2_who(100 + j) == identity);
            assert(net.who_2_idx(identity) == j);
            assert(net.who_2_pid(identity) == 100 + j);
        }
        assert(net.pid_2_idx(999) == 0 && net.pid_2_who(999) == 0);
        assert(net.who_2_idx(99) == 0 && net.who_2_pid(99) == 0);
        net.process_slots_[6].pid_ = net.process_slots_[1].pid_;
        net.process_slots_[6].identity_ = net.process_slots_[1].identity_;
        assert(net.pid_2_idx(101) == 1 && net.who_2_idx(1) == 1);
        net.player_count_ = 7;
        net.close();
        assert(event_count == 4 && events[3] == 4 && net.player_count_ == 0);
        for (int k = 0; k < 8; ++k) {
            assert(net.process_slots_[k].pid_ == 0);
            assert(net.process_slots_[k].name_[31] == 'a' + k);
            assert(net.process_slots_[k].status_bytes_[0] == k);
        }
    }
    // Destructor closes once, then normal members/base destroy in reverse order.
    assert(event_count == 8);
    assert(events[4] == 4 && events[5] == 5 && events[6] == 6 && events[7] == 7);
    return 0;
}
