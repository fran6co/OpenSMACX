// Host projection for test_time_controls.py. The two function bodies are
// injected from actual annotated game.cpp/popup.cpp records. This is not the
// application ABI, native matching, linked preservation, or game runtime.

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>

#ifndef __cdecl
#define __cdecl
#endif

static const int MaxPlayerNum = 8;
static const int MaxTimeControlNum = 6;

struct RulesTimeControl {
    char *name;
    int turn;
    int base;
    int unit;
    int event;
    int extra;
    int refresh;
    int accumulated;
};

struct PlayerData {
    uint32_t current_num_bases;
    int time_bonus_count;
};

struct Veh {
    uint32_t state;
    uint8_t faction_id;
    int8_t order;
};

struct LockStateEntry {
    uint32_t trailing_words_[10];
};

struct NetDaemonProjection {
    LockStateEntry lock_state_[3];
};

RulesTimeControl TimeControl[MaxTimeControlNum];
PlayerData PlayersData[MaxPlayerNum + 1]; // [8] sentinel detects upper-bound damage
Veh Vehs[2049];
int VehCurrentCount;
int TimeControlsSelection;
uint32_t LockEnableMask;
int ConsoleControlTurnActive;
int ControlTurnPhase;
int TurnCurrentNum;
int TurnControlDurationMilliseconds;
NetDaemonProjection NetDaemonNet;

// Source-like 32-bit key conversion for this 64-bit host only. Product source
// keeps its native reinterpret_cast<int>(LPSTR); only the extracted function
// body is rewritten by the Python driver to call this helper. StringTable
// stores and resolves the same low-32-bit pointer identity, not name contents.
static int host_string_pointer_key(const char *pointer) {
    const uint32_t low_address = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(pointer));
    return static_cast<int>(low_address);
}

struct StringsProjection {
    std::map<uint32_t, char *> entries;

    void add(char *text) {
        const uint32_t key = static_cast<uint32_t>(
            reinterpret_cast<uintptr_t>(text));
        assert(entries.find(key) == entries.end());
        entries[key] = text;
    }

    char *get(int address) {
        const uint32_t key = static_cast<uint32_t>(address);
        std::map<uint32_t, char *>::iterator found = entries.find(key);
        assert(found != entries.end());
        return found->second;
    }
} StringTable;

struct LabelProjection {
    char *strings_ptr[0x1094 / 4 + 1];
} Labels;

static char *label_get(int slot) {
    return StringTable.get(host_string_pointer_key(Labels.strings_ptr[slot]));
}

char StringTemp[1032];
char PopupStartCaption[256] = "host-caption";

static char *_itoa(int value, char *buffer, int radix) {
    assert(radix == 10);
    ::sprintf(buffer, "%d", value);
    return buffer;
}

struct Sprite {};

struct DialogsProjection {
    int item_count;
    int slots[MaxTimeControlNum];
    char rows[MaxTimeControlNum][256];

    DialogsProjection() : item_count(0) {
        memset(slots, 0, sizeof(slots));
        memset(rows, 0, sizeof(rows));
    }

    void clear() {
        item_count = 0;
        memset(slots, 0, sizeof(slots));
        memset(rows, 0, sizeof(rows));
    }

    void item(char *text, int slot) {
        assert(slot >= 0 && slot < MaxTimeControlNum);
        assert(item_count < MaxTimeControlNum);
        slots[item_count] = slot;
        ::strncpy(rows[item_count], text, sizeof(rows[item_count]) - 1);
        rows[item_count][sizeof(rows[item_count]) - 1] = '\0';
        ++item_count;
    }
};

struct Popup {
    DialogsProjection dialogs_;
    int start_calls;
    char *started_caption;
    const char *started_label;
    int started_value;
    char *started_text;
    int started_flags;
    Sprite *started_sprite;

    Popup() : start_calls(0), started_caption(0), started_label(0),
              started_value(0), started_text(0), started_flags(0),
              started_sprite(0) {}

    int start(char *caption, const char *label, int value, char *text,
              int flags, Sprite *sprite) {
        ++start_calls;
        started_caption = caption;
        started_label = label;
        started_value = value;
        started_text = text;
        started_flags = flags;
        started_sprite = sprite;
        dialogs_.clear();
        return 0;
    }

    DialogsProjection &dialogs() { return dialogs_; }
};

// Independent native-behavior oracle. It reads a snapshot, applies the
// observed faction/vehicle rules in data-oriented passes, and returns the
// duration plus bonus updates. It does not call the extracted implementation.
struct OracleResult {
    int milliseconds;
    int bonus_counts[MaxPlayerNum + 1];
    int active_factions;
    int weighted_bases;
    int vehicle_support;
};

static bool oracle_vehicle_is_eligible(const Veh &vehicle, int faction_id) {
    if (vehicle.faction_id != faction_id) return false;
    if (vehicle.order == 1 || vehicle.order == 2) return false;
    if ((vehicle.state & 0x200u) != 0) return false;
    if ((vehicle.state & 0x01000200u) == 0x01000200u) return false;
    if ((vehicle.state & 0x4000u) != 0) return false;
    return true;
}

static OracleResult native_oracle(
    const RulesTimeControl &control,
    const PlayerData players[MaxPlayerNum + 1],
    uint32_t lock_enable_mask,
    int console_control_turn_active,
    int control_turn_phase,
    int turn_current_num,
    int network_multiplier,
    const Veh *vehicles,
    int vehicle_count) {
    OracleResult result;
    result.active_factions = 0;
    result.weighted_bases = 0;
    result.vehicle_support = 0;
    for (int id = 0; id <= MaxPlayerNum; ++id) {
        result.bonus_counts[id] = players[id].time_bonus_count;
    }

    const uint32_t active_low_byte = lock_enable_mask & 0xFFu;
    if (console_control_turn_active != 0 && control_turn_phase == 0 &&
        control.refresh != 0 && turn_current_num % control.refresh == 0) {
        for (int id = 1; id < MaxPlayerNum; ++id) {
            int next = result.bonus_counts[id] + 1;
            if (next >= control.accumulated) next = control.accumulated;
            result.bonus_counts[id] = next;
        }
    }

    for (int id = 1; id < MaxPlayerNum; ++id) {
        if ((active_low_byte & (1u << id)) == 0) continue;
        ++result.active_factions;
        result.weighted_bases += control.base *
            static_cast<int>(players[id].current_num_bases);
        for (int veh_id = 0; veh_id < vehicle_count; ++veh_id) {
            if (oracle_vehicle_is_eligible(vehicles[veh_id], id)) {
                result.vehicle_support += control.unit;
            }
        }
    }

    int seconds = control.turn;
    if (result.active_factions > 0) {
        const int score = (result.weighted_bases + result.vehicle_support) /
            result.active_factions;
        const int candidate = score + control.event * network_multiplier;
        if (seconds < candidate) seconds = candidate;
    }
    if (turn_current_num < 6) seconds *= 2;
    result.milliseconds = seconds * 1000;
    return result;
}

static void initialize_time_control(RulesTimeControl &control,
                                    char *name, int turn, int base,
                                    int unit, int event, int refresh,
                                    int accumulated) {
    control.name = name;
    control.turn = turn;
    control.base = base;
    control.unit = unit;
    control.event = event;
    control.extra = 0;
    control.refresh = refresh;
    control.accumulated = accumulated;
}

static void reset_globals() {
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Vehs, 0, sizeof(Vehs));
    memset(&NetDaemonNet, 0, sizeof(NetDaemonNet));
    VehCurrentCount = 0;
    TimeControlsSelection = 2;
    LockEnableMask = 0;
    ConsoleControlTurnActive = 0;
    ControlTurnPhase = 0;
    TurnCurrentNum = 0;
    TurnControlDurationMilliseconds = -1;
    memset(StringTemp, 0, sizeof(StringTemp));
}

/*EXTRACTED_SET_TIME_CONTROLS*/

static void prepare_dialog_fixture() {
    static char names[6][16] = {
        "Standard", "Fast", "Slow", "Custom", "Long", "Manual"
    };
    static char labels[5][16] = {
        "L468", "L1094", "L46C", "L470", "L474"
    };
    const int label_slots[5] = {
        0x468 / 4, 0x1094 / 4, 0x46C / 4, 0x470 / 4, 0x474 / 4
    };
    StringTable.entries.clear();
    memset(Labels.strings_ptr, 0, sizeof(Labels.strings_ptr));
    for (int i = 0; i < 6; ++i) {
        initialize_time_control(TimeControl[i], names[i], i * 11 + (i != 0),
                                i * 2 + 1, i * 3 + 2, i * 4 + 3, 0, 0);
        StringTable.add(names[i]);
    }
    for (int i = 0; i < 5; ++i) {
        Labels.strings_ptr[label_slots[i]] = labels[i];
        StringTable.add(labels[i]);
    }
}

/*EXTRACTED_TIME_CONTROLS_DIALOG*/

static void test_setter_against_native_oracle() {
    reset_globals();
    char *names[6] = {0, 0, 0, 0, 0, 0};
    initialize_time_control(TimeControl[0], names[0], 100, 99, 99, 99, 0, 0);
    initialize_time_control(TimeControl[2], names[2], 10, 3, 5, 7, 2, 4);
    TimeControlsSelection = 2;

    for (int id = 0; id <= MaxPlayerNum; ++id) {
        PlayersData[id].current_num_bases = id * 100;
        PlayersData[id].time_bonus_count = 0;
    }
    for (int id = 1; id < MaxPlayerNum; ++id) {
        PlayersData[id].current_num_bases = id;
    }

    VehCurrentCount = 8;
    Vehs[0].faction_id = 0; Vehs[0].order = 0; Vehs[0].state = 0;
    Vehs[1].faction_id = 1; Vehs[1].order = 0; Vehs[1].state = 0;
    Vehs[2].faction_id = 2; Vehs[2].order = 1; Vehs[2].state = 0;
    Vehs[3].faction_id = 3; Vehs[3].order = 2; Vehs[3].state = 0;
    Vehs[4].faction_id = 4; Vehs[4].order = 0; Vehs[4].state = 0x200u;
    Vehs[5].faction_id = 5; Vehs[5].order = 0; Vehs[5].state = 0x01000200u;
    Vehs[6].faction_id = 6; Vehs[6].order = 0; Vehs[6].state = 0x4000u;
    Vehs[7].faction_id = 7; Vehs[7].order = 0; Vehs[7].state = 0x01000000u;

    // Bit zero is intentionally set too: native iterates factions 1..7 only.
    LockEnableMask = 0xFFu;
    TurnCurrentNum = 5;
    ConsoleControlTurnActive = 0;
    NetDaemonNet.lock_state_[2].trailing_words_[7] = 999;
    NetDaemonNet.lock_state_[2].trailing_words_[8] = 2;

    PlayerData snapshot[MaxPlayerNum + 1];
    memcpy(snapshot, PlayersData, sizeof(snapshot));
    const OracleResult expected = native_oracle(
        TimeControl[TimeControlsSelection], snapshot, LockEnableMask,
        ConsoleControlTurnActive, ControlTurnPhase, TurnCurrentNum,
        static_cast<int>(NetDaemonNet.lock_state_[2].trailing_words_[8]),
        Vehs, VehCurrentCount);
    set_time_controls();

    assert(expected.active_factions == 7);
    assert(expected.weighted_bases == 84); // base 3 times factions 1..7 sum 28
    assert(expected.vehicle_support == 10); // only factions 1 and 7 qualify
    assert(expected.milliseconds == 54000); // ((84+10)/7 + 7*2) * 2 * 1000
    assert(TurnControlDurationMilliseconds == expected.milliseconds);
    assert(TurnControlDurationMilliseconds != 999 * 1000);
}

static void test_refresh_inactive_factions_and_cap() {
    reset_globals();
    initialize_time_control(TimeControl[2], const_cast<char *>("refresh"),
                            10, 0, 0, 0, 2, 4);
    TimeControlsSelection = 2;
    PlayersData[0].time_bonus_count = 8;
    PlayersData[1].time_bonus_count = 5; // over cap: next value must clamp down
    PlayersData[2].time_bonus_count = 3; // next value reaches cap
    PlayersData[7].time_bonus_count = 0; // inactive endpoint still updates
    PlayersData[8].time_bonus_count = 9; // out-of-domain sentinel stays untouched
    LockEnableMask = 0;
    ConsoleControlTurnActive = 1;
    ControlTurnPhase = 0;
    TurnCurrentNum = 6;
    set_time_controls();
    assert(PlayersData[0].time_bonus_count == 8);
    assert(PlayersData[1].time_bonus_count == 4);
    assert(PlayersData[2].time_bonus_count == 4);
    assert(PlayersData[7].time_bonus_count == 1);
    assert(PlayersData[8].time_bonus_count == 9);
    assert(TurnControlDurationMilliseconds == 10000);

    PlayersData[1].time_bonus_count = 1;
    ControlTurnPhase = 1;
    set_time_controls();
    assert(PlayersData[1].time_bonus_count == 1);
}

static void test_no_factions_baseline_maximum_and_early_turn_boundary() {
    reset_globals();
    initialize_time_control(TimeControl[2], const_cast<char *>("selected"),
                            10, 3, 5, 7, 0, 0);
    initialize_time_control(TimeControl[0], const_cast<char *>("wrong-row"),
                            100, 99, 99, 99, 0, 0);
    TimeControlsSelection = 2;
    LockEnableMask = 1u; // bit zero is not an active faction in this routine
    TurnCurrentNum = 5;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 20000);

    LockEnableMask = 0xFFu;
    for (int id = 1; id < MaxPlayerNum; ++id) {
        PlayersData[id].current_num_bases = 0;
    }
    TurnCurrentNum = 6;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 10000);

    PlayersData[1].current_num_bases = 1;
    TimeControl[2].turn = 100;
    NetDaemonNet.lock_state_[2].trailing_words_[8] = 0;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 100000); // max(configured, candidate)
}

static void test_vehicle_zero_and_each_individual_exclusion() {
    reset_globals();
    initialize_time_control(TimeControl[2], const_cast<char *>("vehicle"),
                            1, 0, 70, 0, 0, 0);
    TimeControlsSelection = 2;
    LockEnableMask = 1u << 1;
    TurnCurrentNum = 6;
    VehCurrentCount = 1;

    // The sole qualifying vehicle is at index zero. Its contribution is
    // intentionally large so integer division cannot hide an omitted scan.
    Vehs[0].faction_id = 1;
    Vehs[0].order = 0;
    Vehs[0].state = 0;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 70000);

    // Isolate each exclusion predicate, again with no other vehicle and one
    // active faction, so each independent mutation changes the result.
    Vehs[0].order = 1;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 1000);
    Vehs[0].order = 2;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 1000);
    Vehs[0].order = 0;
    Vehs[0].state = 0x200u;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 1000);
    Vehs[0].state = 0x01000200u;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 1000);
    Vehs[0].state = 0x4000u;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 1000);
}

static void test_refresh_guards_and_empty_vehicle_range() {
    reset_globals();
    initialize_time_control(TimeControl[2], const_cast<char *>("guards"),
                            3, 0, 0, 0, 2, 4);
    TimeControlsSelection = 2;
    ConsoleControlTurnActive = 1;
    PlayersData[1].time_bonus_count = 1;
    TurnCurrentNum = 3; // not divisible by refresh
    set_time_controls();
    assert(PlayersData[1].time_bonus_count == 1);
    TurnCurrentNum = 5;
    set_time_controls();
    assert(PlayersData[1].time_bonus_count == 1);
    TimeControl[2].refresh = 0;
    TurnCurrentNum = 6;
    set_time_controls();
    assert(PlayersData[1].time_bonus_count == 1);
    TimeControl[2].refresh = 2;
    ControlTurnPhase = 1;
    set_time_controls();
    assert(PlayersData[1].time_bonus_count == 1);
    ControlTurnPhase = 0;
    ConsoleControlTurnActive = 0;
    set_time_controls();
    assert(PlayersData[1].time_bonus_count == 1);

    // Negative native count means the source for-loop body is not entered.
    reset_globals();
    initialize_time_control(TimeControl[2], const_cast<char *>("empty"),
                            10, 0, 99, 0, 0, 0);
    TimeControlsSelection = 2;
    LockEnableMask = 1u << 1;
    TurnCurrentNum = 6;
    VehCurrentCount = -1;
    Vehs[0].faction_id = 1;
    Vehs[0].order = 0;
    Vehs[0].state = 0;
    set_time_controls();
    assert(TurnControlDurationMilliseconds == 10000);
}

static void test_dialog_exact_rows_and_slots() {
    reset_globals();
    prepare_dialog_fixture();
    TimeControl[0].turn = 0;
    TimeControl[1].turn = 12; TimeControl[1].base = 4; TimeControl[1].unit = 3;
    TimeControl[2].turn = 22; TimeControl[2].base = 5; TimeControl[2].unit = 6;
    TimeControl[3].turn = 33; TimeControl[3].base = 7; TimeControl[3].unit = 8;
    TimeControl[4].turn = 44; TimeControl[4].base = 9; TimeControl[4].unit = 10;
    TimeControl[5].turn = 55; TimeControl[5].base = 11; TimeControl[5].unit = 12;

    Popup popup;
    time_controls_dialog(&popup);
    assert(popup.start_calls == 1);
    assert(popup.started_caption == PopupStartCaption);
    assert(strcmp(popup.started_label, "TIMECONTROLS") == 0);
    assert(popup.started_value == -1);
    assert(popup.started_text == 0);
    assert(popup.started_flags == 0x40);
    assert(popup.started_sprite == 0);
    assert(popup.dialogs_.item_count == 6);

    const char *expected[6] = {
        "Standard|(L474)",
        "Fast|(12 L468 L1094 4 L46C + 3 L470)",
        "Slow|(22 L468 L1094 5 L46C + 6 L470)",
        "Custom|(33 L468 L1094 7 L46C + 8 L470)",
        "Long|(44 L468 L1094 9 L46C + 10 L470)",
        "Manual|(55 L468 L1094 11 L46C + 12 L470)"
    };
    for (int row = 0; row < 6; ++row) {
        assert(popup.dialogs_.slots[row] == row);
        assert(strcmp(popup.dialogs_.rows[row], expected[row]) == 0);
    }
}

int main() {
    test_setter_against_native_oracle();
    test_refresh_inactive_factions_and_cap();
    test_no_factions_baseline_maximum_and_early_turn_boundary();
    test_vehicle_zero_and_each_individual_exclusion();
    test_refresh_guards_and_empty_vehicle_range();
    test_dialog_exact_rows_and_slots();
    puts("PASS: source-extracted time-control host controls");
    return 0;
}
