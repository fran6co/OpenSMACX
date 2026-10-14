// Actual-source queue controls; host behavior evidence, not native/game proof.
#include <cassert>
#include <cstddef>
#include <cstring>
#define __cdecl
#define private public
// @@DECLARATIONS@@
#undef private

char StringTemp[1032];
int sequence[20], sequence_count;
int values[10], details[10], callback_count;
DatalinkSelectionQueue *active_queue;
int callback_mode, init_mode;
int selected_index = -1;

void record_text(char *output, int value, int detail) {
    assert(output == StringTemp);
    assert(StringTemp[0] == 0);
    values[callback_count] = value;
    details[callback_count++] = detail;
    StringTemp[0] = 'x';
    if (callback_mode == 1 && callback_count == 1) {
        active_queue->append(-9, 2, 42);
    }
    if (callback_mode == 2) {
        active_queue->clear();
    }
}
#define FORMATTER(name) void name(char *output, int value, int detail) { record_text(output, value, detail); }
FORMATTER(say_reactor)
FORMATTER(say_weapon)
FORMATTER(say_armor)
FORMATTER(say_chassis)
FORMATTER(say_facility)
FORMATTER(say_society)
FORMATTER(say_ability)
FORMATTER(say_techs)
FORMATTER(say_vehicles)
FORMATTER(say_project)
#undef FORMATTER
int help_value = -1;
#define HELP(name) void name(int value) { help_value = value; }
HELP(help_weapon)
HELP(help_armor)
HELP(help_chassis)
HELP(help_facility)
HELP(help_social)
HELP(help_abil)
HELP(help_tech)
HELP(help_veh)
HELP(help_project)
#undef HELP
int pop_wait() { return 9; }
struct Win {
    virtual int set_modal(int flag, int (*poll)(), Win *owner) {
        assert(flag == 0 && poll == pop_wait && owner == 0);
        sequence[sequence_count++] = 4;
        return 0;
    }
};
struct FakeDatalink : Win {
    void init() {
        sequence[sequence_count++] = 1;
        if (init_mode == 1) { active_queue->append(-9, 2, 11); }
        if (init_mode == 2) { active_queue->clear(); }
    }
    void show(int flag) { assert(flag == 0); sequence[sequence_count++] = 3; }
    virtual int set_modal(int flag, int (*poll)(), Win *owner) {
        assert(flag == 0 && poll == pop_wait && owner == 0);
        sequence[sequence_count++] = 4;
        return 0;
    }
} DatalinkMain;
int datalink_entry_selection(int index) {
    selected_index = index;
    sequence[sequence_count++] = 2;
    return index;
}

// @@BODIES@@
DatalinkEntryCallbackTable DatalinkEntryCallbackTableGlobal;

void reset(DatalinkSelectionQueue &queue) {
    queue.clear(); active_queue = &queue;
    callback_count = 0; sequence_count = 0; selected_index = -1;
    callback_mode = 0; init_mode = 0; StringTemp[0] = 'z';
}
void modal_sequence() {
    assert(sequence_count == 4);
    for (int i = 0; i < 4; ++i) { assert(sequence[i] == i + 1); }
}
int main() {
    assert(sizeof(DatalinkSelectionEntry) == 4);
    assert(offsetof(DatalinkSelectionEntry, category) == 2);
    assert(offsetof(DatalinkSelectionEntry, value) == 3);
    assert(sizeof(DatalinkSelectionQueue) == 0x204);
    assert(offsetof(DatalinkSelectionQueue, count_) == 0x200);
    assert(DatalinkEntryCallbackTableGlobal.entries[0].help == 0);
    assert(DatalinkEntryCallbackTableGlobal.entries[0].append_text == 0);
    DatalinkHelpCallback helps[] = {0, 0, help_weapon, help_armor, help_chassis,
        help_facility, help_social, help_abil, help_tech, help_veh, help_project};
    DatalinkTextCallback texts[] = {0, say_reactor, say_weapon, say_armor,
        say_chassis, say_facility, say_society, say_ability, say_techs,
        say_vehicles, say_project};
    for (int i = 0; i < 11; ++i) {
        assert(DatalinkEntryCallbackTableGlobal.entries[i].help == helps[i]);
        assert(DatalinkEntryCallbackTableGlobal.entries[i].append_text == texts[i]);
    }
    DatalinkSelectionQueue queue;
    reset(queue);
    for (int i = 0; i < 129; ++i) { queue.append(static_cast<short>(i), 2, 255); }
    assert(queue.count_ == 128);
    assert(queue.entries_[127].entry_id == 127 && queue.entries_[127].value == 255);
    reset(queue);
    queue.append(-9, 2, 255);
    queue.entries_[0].show_help(); assert(help_value == 255);
    StringTemp[0] = 0;
    queue.entries_[0].append_text(StringTemp, 17);
    assert(callback_count == 1 && values[0] == 255 && details[0] == 17);
    queue.entries_[0].category = 0;
    queue.entries_[0].show_help(); queue.entries_[0].append_text(StringTemp, 1);
    assert(callback_count == 1 && help_value == 255);

    reset(queue);
    queue.append(8, 2, 10); queue.append(-9, 5, 70);
    queue.append(8, 2, 11); queue.append(-9, 5, 255);
    queue.dispatch_matching(-9);
    assert(selected_index == 1 && callback_count == 2);
    assert(values[0] == 0 && values[1] == 185 && details[0] == 1 && details[1] == 1);
    assert(queue.entries_[1].category == 10 && queue.entries_[1].value == 0);
    assert(queue.entries_[3].category == 10 && queue.entries_[3].value == 185);
    modal_sequence();
    reset(queue);
    queue.append(-9, 5, 69); queue.dispatch_matching(-9);
    assert(values[0] == 69 && queue.entries_[0].category == 5);
    modal_sequence();
    reset(queue);
    queue.append(-9, 2, 10); callback_mode = 1; queue.dispatch_matching(-9);
    assert(callback_count == 2 && values[1] == 42); modal_sequence();
    reset(queue);
    queue.append(-9, 2, 10); queue.append(-9, 2, 11);
    callback_mode = 2; queue.dispatch_matching(-9);
    assert(callback_count == 1); modal_sequence();
    reset(queue);
    init_mode = 1; queue.dispatch_matching(-9);
    assert(callback_count == 1 && values[0] == 11); modal_sequence();
    reset(queue);
    queue.append(-9, 2, 10); init_mode = 2; queue.dispatch_matching(-9);
    assert(callback_count == 0 && sequence_count == 1);
    reset(queue);
    queue.append(7, 2, 10); queue.dispatch_matching(-9);
    assert(callback_count == 0 && sequence_count == 1 && selected_index == -1);
    reset(queue); queue.dispatch_matching(-9);
    assert(callback_count == 0 && sequence_count == 1);
}
