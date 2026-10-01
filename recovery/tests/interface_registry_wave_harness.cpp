#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct RECT {};
struct SubInterface {
    explicit SubInterface(int mode = 0) : field_4_(mode) {}
    virtual void on_status(int state);
    void delete_iface_mode();
    void set_iface_mode();
    void release_iface_mode();
    uint32_t field_4_;
};

int IfaceIdToSlotStorage[30] = {};
int ActiveIfaceIdStorage = 0;
SubInterface *InterfaceRegistry[12] = {};
std::vector<std::string> events;
std::vector<SubInterface *> status_receivers;
std::vector<int> status_states;
SubInterface *callback_target = 0;
int callback_mode = -1;
bool mutate_on_clip = false;
int clip_registry_index = -1;
SubInterface *clip_registry_value = 0;
SubInterface *mutate_on_status_receiver = 0;
int mutate_on_status_state = -1;
int mutate_stack_index = -1;
int mutate_stack_value = -1;
int mutate_registry_index = -1;
SubInterface *mutate_registry_value = 0;
SubInterface *mutate_field_target = 0;
int mutate_field_state = -1;
int mutate_field_value = -1;

struct MainInterfaceDouble {
    RECT rect;
    RECT *buffer_working_rect() { return &rect; }
    void set_clip(RECT *) {
        events.push_back("clip");
        if (mutate_on_clip && callback_target != 0) {
            callback_target->field_4_ = static_cast<uint32_t>(callback_mode);
        }
        if (clip_registry_index >= 0) {
            InterfaceRegistry[clip_registry_index] = clip_registry_value;
        }
    }
} MainInterfaceGlobal;

void SubInterface::on_status(int state) {
    status_receivers.push_back(this);
    status_states.push_back(state);
    events.push_back("status:" + std::to_string(field_4_) + ":" +
                     std::to_string(state));
    if (this == mutate_on_status_receiver && state == mutate_on_status_state) {
        if (mutate_stack_index >= 0) {
            IfaceIdToSlotStorage[mutate_stack_index] = mutate_stack_value;
        }
        if (mutate_registry_index >= 0) {
            InterfaceRegistry[mutate_registry_index] = mutate_registry_value;
        }
    }
    if (this == mutate_on_status_receiver && state == mutate_field_state &&
        mutate_field_target != 0) {
        mutate_field_target->field_4_ = static_cast<uint32_t>(mutate_field_value);
    }
}

struct Sprite {
    Sprite(int id = 0, int value = 0) : id(id), value(value) {}
    int id;
    int value;
    int height() const;
};
std::vector<int> height_reads;
int Sprite::height() const {
    height_reads.push_back(id);
    return value;
}

struct MultiWin {
    Sprite sprite3_, sprite4_, sprite5_, sprite7_;
    unsigned int field_A38_, field_A3C_, field_A40_, field_A44_;
    void UNK2();
};
int IsMultiplayerNet = 0;
uint32_t StatusFlags = 0;

// ACTUAL_PRODUCT_BODIES

static void reset_interface_state() {
    for (int i = 0; i < 30; ++i) IfaceIdToSlotStorage[i] = 0;
    for (int i = 0; i < 12; ++i) InterfaceRegistry[i] = 0;
    ActiveIfaceIdStorage = 0;
    events.clear();
    status_receivers.clear();
    status_states.clear();
    callback_target = 0;
    callback_mode = -1;
    mutate_on_clip = false;
    clip_registry_index = -1;
    clip_registry_value = 0;
    mutate_on_status_receiver = 0;
    mutate_on_status_state = -1;
    mutate_stack_index = -1;
    mutate_stack_value = -1;
    mutate_registry_index = -1;
    mutate_registry_value = 0;
    mutate_field_target = 0;
    mutate_field_state = -1;
    mutate_field_value = -1;
}

static void require(bool condition, const char *message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

static void test_registry_delete() {
    reset_interface_state();
    SubInterface first(4), absent(7);
    InterfaceRegistry[2] = &first;
    InterfaceRegistry[7] = &first;
    first.delete_iface_mode();
    require(InterfaceRegistry[2] == 0, "delete clears first matching slot");
    require(InterfaceRegistry[7] == &first, "delete leaves later duplicate pointer");
    absent.delete_iface_mode();
    require(InterfaceRegistry[7] == &first, "missing delete is a no-op");
}

static void test_set_gate_and_live_reads() {
    reset_interface_state();
    SubInterface caller(10), previous(1), next_before(2), next_after(3);
    InterfaceRegistry[1] = &previous;
    InterfaceRegistry[2] = &next_before;
    InterfaceRegistry[3] = &next_after;
    IfaceIdToSlotStorage[0] = 1;
    caller.set_iface_mode();
    require(events.empty() && ActiveIfaceIdStorage == 0,
            "mode 10 refuses entry over a nonzero current mode");

    IfaceIdToSlotStorage[0] = 0;
    InterfaceRegistry[10] = &next_before;
    caller.set_iface_mode();
    require(ActiveIfaceIdStorage == 1 && IfaceIdToSlotStorage[1] == 10,
            "mode 10 enters when current mode is zero");
    require(events.size() == 2 && events[0] == "clip" &&
                events[1] == "status:2:1",
            "mode 10 calls clip then its registered interface");

    reset_interface_state();
    SubInterface same(2);
    same.field_4_ = 2;
    IfaceIdToSlotStorage[0] = 2;
    same.set_iface_mode();
    require(events.empty() && ActiveIfaceIdStorage == 0,
            "request for current mode is a no-op");

    reset_interface_state();
    SubInterface mode_caller(2), mode_previous(1), mode_next_before(2), mode_next_after(3);
    InterfaceRegistry[1] = &mode_previous;
    InterfaceRegistry[2] = &mode_next_before;
    InterfaceRegistry[3] = &mode_next_after;
    IfaceIdToSlotStorage[0] = 1;
    // Previous-off changes the receiver's live mode after the stack snapshot;
    // set_clip then rebinds that mode's registry target before it is loaded.
    mutate_on_status_receiver = &mode_previous;
    mutate_field_state = 0;
    mutate_field_target = &mode_caller;
    mutate_field_value = 3;
    clip_registry_index = 3;
    clip_registry_value = &mode_next_after;
    mode_caller.set_iface_mode();
    require(ActiveIfaceIdStorage == 1, "set pushes one active mode");
    require(IfaceIdToSlotStorage[1] == 2,
            "set stores mode read before callback/clip");
    require(events.size() == 3 && events[0] == "status:1:0" &&
                events[1] == "clip" && events[2] == "status:3:1",
            "set uses previous-off, clip, then live next-on order");
    require(status_receivers.size() == 2 && status_receivers[0] == &mode_previous &&
                status_receivers[1] == &mode_next_after && status_states[0] == 0 &&
                status_states[1] == 1,
            "set observes callback mode mutation and post-clip registry rebinding");
}

static void test_release_top_and_repeated_downward() {
    reset_interface_state();
    SubInterface top(2), previous(1), rebound_previous(1), changed_top(3), non_top(1);
    InterfaceRegistry[2] = &top;
    InterfaceRegistry[1] = &previous;
    ActiveIfaceIdStorage = 2;
    IfaceIdToSlotStorage[0] = 0;
    IfaceIdToSlotStorage[1] = 1;
    IfaceIdToSlotStorage[2] = 2;
    mutate_on_status_receiver = &top;
    mutate_on_status_state = 0;
    mutate_stack_index = 1;
    mutate_stack_value = 3;
    mutate_registry_index = 1;
    mutate_registry_value = &rebound_previous;
    clip_registry_index = 3;
    clip_registry_value = &changed_top;
    top.release_iface_mode();
    require(ActiveIfaceIdStorage == 1, "top release pops exactly one");
    require(IfaceIdToSlotStorage[1] == 3,
            "off callback may mutate the backing next-mode slot");
    require(events.size() == 3 && events[0] == "status:2:0" &&
                events[1] == "clip" && events[2] == "status:1:1",
            "release uses snapshotted next-mode despite stack mutation");
    require(status_receivers.size() == 2 && status_receivers[0] == &top &&
                status_receivers[1] == &rebound_previous && status_states[0] == 0 &&
                status_states[1] == 1,
            "release snapshots next-mode but loads that registry entry after callback");
    require(InterfaceRegistry[3] == &changed_top,
            "mutated stack's alternate entry remains distinct");
    require(InterfaceRegistry[1] == &rebound_previous,
            "off callback registry rebinding is preserved");
    require(events.size() == 3,
            "release callback/clip/rebound-target ordering is exact");

    reset_interface_state();
    ActiveIfaceIdStorage = 5;
    IfaceIdToSlotStorage[0] = 0;
    IfaceIdToSlotStorage[1] = 1;
    IfaceIdToSlotStorage[2] = 2;
    IfaceIdToSlotStorage[3] = 1;
    IfaceIdToSlotStorage[4] = 3;
    IfaceIdToSlotStorage[5] = 9;
    non_top.release_iface_mode();
    require(ActiveIfaceIdStorage == 3, "downward release removes both repeats");
    require(IfaceIdToSlotStorage[0] == 0 && IfaceIdToSlotStorage[1] == 2 &&
                IfaceIdToSlotStorage[2] == 3 && IfaceIdToSlotStorage[3] == 9,
            "downward release compacts stack after each matching entry");
    require(events.empty(), "non-top removal does not notify or reset clip");
}

static void test_multiwin_dimensions_and_flag() {
    MultiWin window;
    window.sprite3_ = Sprite(3, 13);
    window.sprite4_ = Sprite(4, 17);
    window.sprite5_ = Sprite(5, 11);
    window.sprite7_ = Sprite(7, 19);
    window.field_A38_ = 1;
    window.field_A3C_ = 2;
    window.field_A40_ = 3;
    window.field_A44_ = 4;
    StatusFlags = 0x205;
    IsMultiplayerNet = 0;
    height_reads.clear();
    window.UNK2();
    require(height_reads == std::vector<int>({5, 7, 3}),
            "single-player reads sprite5, sprite7, sprite3 in native order");
    require(window.field_A38_ == 0 && window.field_A3C_ == 0 &&
                window.field_A40_ == 0x8a && window.field_A44_ == 109,
            "single-player dimensions populate native fields");
    require(StatusFlags == 0x245, "multiwin ORs flag 0x40 without dropping bits");

    IsMultiplayerNet = 1;
    height_reads.clear();
    window.UNK2();
    require(height_reads == std::vector<int>({5, 7, 3, 4}),
            "multiplayer reads sprite4 only after the common heights");
    require(window.field_A44_ == 159,
            "multiplayer adds sprite4, sprite7, and 14");
}

int main() {
    test_registry_delete();
    test_set_gate_and_live_reads();
    test_release_top_and_repeated_downward();
    test_multiwin_dimensions_and_flag();
    return 0;
}
