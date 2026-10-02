#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>

typedef char *LPSTR;
static std::vector<std::string> events;
static void note(const std::string &value) { events.push_back(value); }
static int find_event(const char *value) {
    for (size_t i = 0; i < events.size(); ++i)
        if (events[i] == value) return static_cast<int>(i);
    return -1;
}
static void assert_before(const char *a, const char *b) {
    assert(find_event(a) >= 0 && find_event(b) >= 0);
    assert(find_event(a) < find_event(b));
}

struct NetWinLobbyRow {
    uint8_t prefix_, ready_or_occupied_, field_2_;
    int8_t faction_or_color_;
    uint8_t field_4_;
    char leader_name_[0x18];
    char formal_faction_name_[0x28];
    char noun_faction_[0x18];
    char adjective_faction_[0x80];
    char leader_title_[0x18];
    char adjective_leader_[0x80];
    uint8_t tail_[3];
    uint32_t owner_;
};
struct NetWinSessionBlock {
    uint32_t mode_;
    uint8_t state_[0x164];
    void write_row8_prefix(uint8_t value);
    char *row8_leader_name();
    char *row8_noun_faction();
};
struct NetWinSessionOptionsState {
    uint8_t status_, player_count_, difficulty_, time_control_;
    uint8_t option_4_, option_5_, planet_size_, ocean_coverage_;
    uint8_t erosion_, native_life_, cloud_cover_, reserved_B_;
    uint32_t rule_bits_a_, rule_bits_b_, rule_bits_c_;
};
static_assert(sizeof(NetWinLobbyRow) == 0x17C, "row stride");
static_assert(sizeof(NetWinSessionBlock) == 0x168, "session extent");
static_assert(sizeof(NetWinSessionOptionsState) == 0x18, "options extent");
static_assert(offsetof(NetWinLobbyRow, leader_name_) +
              sizeof(((NetWinLobbyRow *)0)->leader_name_) <= sizeof(NetWinSessionBlock),
              "leader alias stays in session object");
static_assert(offsetof(NetWinLobbyRow, noun_faction_) +
              sizeof(((NetWinLobbyRow *)0)->noun_faction_) <= sizeof(NetWinSessionBlock),
              "noun alias stays in session object");

extern NetWinSessionBlock NetWinSessionData;
extern NetWinSessionOptionsState NetWinSessionOptions;
extern NetWinLobbyRow NetWinLobbyRows[8];
extern int NextTurnReady;
extern int NetUpkeepPhase;
extern uint32_t NetHostChangeSlot;

struct AlphaNetProcessSlot {
    char name_[0x20];
    uint32_t pid_;
    int8_t identity_;
    uint8_t field_25_, field_26_;
    int8_t field_27_;
    uint8_t status_bytes_[0x174];
};
struct AlphaNetLedger {
    uint8_t leading_[0x24];
    uint32_t session_flags_;
    uint8_t tail_[0x20];
    uint32_t max_players_;
};
struct Net {
    uint32_t field_760_, field_764_;
    int send(void *, int, unsigned long, int);
};
struct AlphaNet : public Net {
    int player_count_;
    AlphaNetProcessSlot process_slots_[8];
    AlphaNetLedger ledger_;
};
struct LockStateEntry { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };

struct PlayerLock {
    struct Entry { int flag, second, first; };
    uint8_t active_;
    uint8_t padding_[3];
    Entry entries_[2];
    void clear();
    int active();
};
struct Locks {
    PlayerLock records_[8];
    uint32_t field_E0_, field_E4_, field_E8_;
};
struct NetDaemon : public AlphaNet {
    Locks locks_;
    LockStateEntry lock_state_[24];
    void on_player_added(char *name, unsigned long id);
    void on_player_deleted(char *name, unsigned long id);
};

struct Popup {
    void pop(const char *text, int duration, int mode, const char *) {
        note(std::string("popup:") + text);
        assert(duration == 3000);
        assert(mode == 0 || mode == 2);
    }
};
struct NetWinDialogType {
    int visible, redraws, button_calls;
    int is_visible() { note("visible"); return visible; }
    void on_redraw() { ++redraws; note("redraw"); }
    void on_button_clicked(int id) { assert(id == -2); ++button_calls; note("button:-2"); }
};
struct FX { void play(int id) { assert(id == 0x15); note("fx:21"); } };
struct ConsoleType { int faction_id_; };
struct ParseStrType { char str[16]; };

NetWinLobbyRow NetWinLobbyRows[8];
NetWinDialogType NetWinDialog;
Popup popup_storage;
Popup &NetMsgPrimary = popup_storage;
FX g_FX;
ConsoleType ConsoleGlobal;
ParseStrType ParseStrBuffer[1];
uint32_t LockEnableMask;
uint32_t NetTurnFlags;
int NetTurnFaction;
int TurnCurrentNum;
int ConsoleControlTurnActive;
int NetPlayerDropped;
char labels_for_call[2][16];
int label_calls;
int message_calls;
int send_order[8];
int send_count;
int last_message_id;
int last_message_arg;
int logs;
char incoming_name[32];

static void clear_trace() {
    events.clear(); label_calls = 0; message_calls = 0;
    send_count = 0; logs = 0; last_message_id = 0; last_message_arg = 0;
}
static char *get_player_name(unsigned long id) {
    note(std::string("get-name:") + (id == 20 ? "20" : "other"));
    return incoming_name;
}
static void parse_says(int, char *text, int, int) {
    note(std::string("parse:") + (text ? text : "null"));
}
static void message_data(int id, int, int a2, int, int, int) {
    ++message_calls; last_message_id = id; last_message_arg = a2;
    char buffer[32]; sprintf(buffer, "message:%x", id); note(buffer);
}
static void log_say(const char *, const char *text, int, int, int) {
    ++logs; note(std::string("log:") + (text ? text : "null"));
}
static void log_say(const char *, int, int, int) {
    ++logs; note("log:formatted");
}
static char *label_get(int) {
    assert(label_calls < 2);
    sprintf(labels_for_call[label_calls], "label%d", label_calls + 1);
    ++label_calls;
    note(label_calls == 1 ? "label-get:1" : "label-get:2");
    return labels_for_call[label_calls - 1];
}
static void send_all_player_packets(int slot) {
    assert(slot == -1); send_order[send_count++] = 1; note("send-all");
}
static void send_session_packet(int slot, int target) {
    assert(slot == 2 && target == -1); send_order[send_count++] = 2; note("send-session");
}
static void send_session2_packet(int slot, int target) {
    assert(slot == 2 && target == -1); send_order[send_count++] = 3; note("send-session2");
}
static void send_faction_data(int slot) {
    assert(slot == 2); send_order[send_count++] = 4; note("send-faction");
}

/* PRODUCT */

int Net::send(void *, int, unsigned long, int) { note("net-send"); return 0; }

static void reset_daemon(NetDaemon &daemon) {
    memset(&daemon, 0, sizeof(daemon));
    memset(NetWinLobbyRows, 0, sizeof(NetWinLobbyRows));
    memset(&NetWinSessionData, 0, sizeof(NetWinSessionData));
    memset(&NetWinSessionOptions, 0, sizeof(NetWinSessionOptions));
    NetWinDialog.visible = 0; NetWinDialog.redraws = 0; NetWinDialog.button_calls = 0;
    NetHostChangeSlot = 998; NextTurnReady = 0; NetUpkeepPhase = 7;
    NetTurnFlags = 0; NetTurnFaction = 0; TurnCurrentNum = 14;
    LockEnableMask = 0; ConsoleControlTurnActive = 0; NetPlayerDropped = 0;
    ConsoleGlobal.faction_id_ = 1;
    strcpy(ParseStrBuffer[0].str, "LOG");
    strcpy(incoming_name, "new-player");
    clear_trace();
}
static void setup_two_player_delete(NetDaemon &daemon, int lock_phase) {
    reset_daemon(daemon);
    daemon.player_count_ = 2;
    daemon.process_slots_[1].pid_ = 10;
    strcpy(daemon.process_slots_[1].name_, "host");
    daemon.process_slots_[1].identity_ = 1;
    daemon.process_slots_[2].pid_ = 20;
    strcpy(daemon.process_slots_[2].name_, "departed");
    daemon.process_slots_[2].identity_ = 2;
    daemon.field_760_ = 10;
    daemon.field_764_ = 10;
    daemon.lock_state_[0].trailing_words_[0] = lock_phase;
    daemon.lock_state_[0].trailing_words_[4] = 1u << 2;
    daemon.lock_state_[1].trailing_words_[0] = 0xFF;
    daemon.lock_state_[1].trailing_words_[3] = 0x8;
    NetWinLobbyRows[2].prefix_ = 0x55;
    strcpy(NetWinLobbyRows[2].leader_name_, "old-leader");
    strcpy(NetWinLobbyRows[2].noun_faction_, "old-noun");
}

int main(int argc, char **argv) {
    assert(argc == 2);
    NetDaemon daemon;
    const char *scenario = argv[1];
    if (strcmp(scenario, "added") == 0) {
        reset_daemon(daemon);
        daemon.player_count_ = 1;
        daemon.field_760_ = daemon.field_764_ = 44;
        daemon.process_slots_[1].pid_ = 44;
        strcpy(daemon.process_slots_[1].name_, "host");
        daemon.process_slots_[1].identity_ = 1;
        NetWinSessionOptions.difficulty_ = 4;
        NetWinDialog.visible = 1;
        clear_trace();
        daemon.on_player_added(incoming_name, 20);
        assert(daemon.player_count_ == 2);
        assert(daemon.process_slots_[2].pid_ == 20);
        assert(daemon.process_slots_[2].identity_ == 2);
        assert(strcmp(daemon.process_slots_[2].name_, "new-player") == 0);
        assert(NetWinLobbyRows[2].prefix_ == 2);
        assert(NetWinLobbyRows[2].field_2_ == 4);
        assert(NetWinLobbyRows[2].owner_ == 0);
        assert(NetWinLobbyRows[2].faction_or_color_ == -1);
        assert(find_event("popup:ADDED") >= 0);
        assert_before("get-name:20", "parse:new-player");
        assert_before("parse:new-player", "popup:ADDED");
        assert_before("popup:ADDED", "send-all");
        assert_before("send-all", "send-session");
        assert_before("send-session", "send-session2");
        assert_before("send-session2", "send-faction");
        assert_before("send-faction", "redraw");
        assert(send_count == 4 && send_order[0] == 1 && send_order[3] == 4);
        assert(NetWinDialog.redraws == 1);
    } else if (strcmp(scenario, "delete_found") == 0) {
        setup_two_player_delete(daemon, 1);
        clear_trace();
        daemon.on_player_deleted(0, 20);
        assert(daemon.player_count_ == 1);
        assert(NetWinLobbyRows[2].prefix_ == 0xFF);
        assert(strcmp(NetWinLobbyRows[2].leader_name_, "label1") == 0);
        assert(strcmp(NetWinLobbyRows[2].noun_faction_, "label2") == 0);
        assert(label_calls == 2);
        assert(daemon.locks_.records_[2].entries_[0].first == -1);
        assert(daemon.lock_state_[1].trailing_words_[0] == (0xFF & ~0x8u));
        assert(NetWinDialog.redraws == 1);
        assert_before("parse:ED", "fx:21");
        assert_before("fx:21", "parse:departed");
        assert_before("parse:departed", "label-get:1");
        assert_before("label-get:1", "label-get:2");
        assert_before("label-get:2", "popup:DROPPED");
        assert_before("popup:DROPPED", "redraw");
    } else if (strcmp(scenario, "delete_inactive_lock") == 0) {
        setup_two_player_delete(daemon, 2);
        daemon.lock_state_[0].leading_words_[1] = 0;
        LockEnableMask = 0x0808; // alive and human faction 3
        daemon.locks_.records_[3].entries_[0].flag = 0;
        clear_trace();
        daemon.on_player_deleted(0, 20);
        assert(daemon.locks_.records_[3].active_ == 0);
        assert(daemon.locks_.records_[3].entries_[0].flag == 0);
        assert(daemon.lock_state_[0].trailing_words_[0] == 2);
        assert(NetWinLobbyRows[2].prefix_ == 0x55); // phase 2: no prefix write
        assert(message_calls == 0);
        assert(label_calls == 2);
    } else if (strcmp(scenario, "delete_active_lock") == 0) {
        setup_two_player_delete(daemon, 2);
        daemon.lock_state_[0].leading_words_[1] = 0;
        LockEnableMask = 0x0808;
        daemon.locks_.records_[3].entries_[0].flag = 1;
        clear_trace();
        daemon.on_player_deleted(0, 20);
        assert(daemon.locks_.records_[3].active_ == 4);
        assert(daemon.lock_state_[0].trailing_words_[0] == 2);
        assert(NetWinLobbyRows[2].prefix_ == 0x55);
        assert(message_calls == 0);
    } else if (strcmp(scenario, "delete_slot8_miss") == 0) {
        reset_daemon(daemon);
        daemon.player_count_ = 7;
        daemon.field_760_ = 107; daemon.field_764_ = 107;
        for (int slot = 1; slot <= 7; ++slot) {
            daemon.process_slots_[slot].pid_ = 100 + slot;
            daemon.process_slots_[slot].identity_ = static_cast<int8_t>(slot);
            sprintf(daemon.process_slots_[slot].name_, "player%d", slot);
        }
        daemon.ledger_.session_flags_ = 0xAB000003;
        daemon.ledger_.max_players_ = 0x1200;
        for (int row = 1; row <= 4; ++row) {
            NetWinLobbyRows[row].prefix_ = static_cast<uint8_t>(0x30 + row);
            NetWinLobbyRows[row].ready_or_occupied_ = static_cast<uint8_t>(0x40 + row);
            NetWinLobbyRows[row].field_2_ = static_cast<uint8_t>(0x50 + row);
            NetWinLobbyRows[row].owner_ = static_cast<uint32_t>(0x600 + row);
        }
        NetWinSessionData.mode_ = 0x11223344;
        LockEnableMask = 0;
        memset(&NetWinSessionOptions, 0xA5, sizeof(NetWinSessionOptions));
        NetWinSessionOptions.difficulty_ = 5;
        NetWinSessionOptionsState options_before = NetWinSessionOptions;
        NetWinDialog.visible = 0;
        clear_trace();
        daemon.on_player_deleted(0, 999);
        assert(daemon.player_count_ == 6); // the native not-found tail decrements
        assert(daemon.ledger_.session_flags_ == 0xFF0000FF);
        assert((daemon.ledger_.max_players_ & 0xFF) == 2);
        assert(*reinterpret_cast<unsigned char *>(&NetWinSessionData) == 0xFF);
        assert(strcmp(NetWinSessionData.row8_leader_name(), "label1") == 0);
        assert(strcmp(NetWinSessionData.row8_noun_faction(), "label2") == 0);
        assert(memcmp(&NetWinSessionOptions, &options_before,
                      sizeof(NetWinSessionOptions)) == 0);
        assert(label_calls == 2);
        for (int row = 1; row <= 4; ++row) {
            assert(NetWinLobbyRows[row].prefix_ == 0x30 + row);
            assert(NetWinLobbyRows[row].ready_or_occupied_ == 0x40 + row);
            assert(NetWinLobbyRows[row].field_2_ == 0x50 + row);
            assert(NetWinLobbyRows[row].owner_ == static_cast<uint32_t>(0x600 + row));
        }
        assert_before("parse:ED", "label-get:1");
        assert_before("label-get:1", "label-get:2");
        assert_before("label-get:2", "popup:DROPPED");
        assert(NetWinDialog.redraws == 1);
    } else {
        assert(false);
    }
    return 0;
}
