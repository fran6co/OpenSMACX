#if NETWIN_OWNER_CONTROL_KIND == 1
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

static std::vector<std::string> calls;
static void *watched_first;
static void *watched_second;
static unsigned freed_first;
static unsigned freed_second;
static void test_free(void *pointer);
struct GraphicWin {
    virtual ~GraphicWin() { calls.push_back("GraphicWin::~"); }
};
struct Spot {
    void shutdown();
    ~Spot() { calls.push_back("Spot::~"); }
};
struct StringBox {
    void close() { calls.push_back("StringBox.close"); }
    ~StringBox() {
        close();
        calls.push_back("StringBox.nested");
    }
};
struct EditBox {
    void close() { calls.push_back("EditBox.close"); }
    ~EditBox() {
        close();
        calls.push_back("EditBox.Time/GraphicWin");
    }
};
struct CheckBox {
    void close() { calls.push_back("CheckBox.close"); }
    ~CheckBox() { calls.push_back("CheckBox.virtual bases"); }
};
struct FlatButton {
    const char *name;
    void close() { calls.push_back(std::string(name) + ".close"); }
    ~FlatButton() {
        close();
        calls.push_back(std::string(name) + ".BaseButton dtor");
    }
};
class NetWin : public GraphicWin {
public:
    uint8_t field_6614_[0x1000];
    uint32_t field_7614_, field_7618_, field_761C_, field_7620_;
    uint8_t field_7624_[0x104];
    uint32_t field_7728_;
    void *faction_records_;
    void *faction_records_copy_;
    Spot spot_;
    StringBox stringBox_;
    EditBox editBox_;
    CheckBox checkBox_;
    FlatButton flatButton1_, flatButton2_;
    NetWin() {
        flatButton1_.name = "FlatButton1";
        flatButton2_.name = "FlatButton2";
        faction_records_ = nullptr;
        faction_records_copy_ = nullptr;
        field_761C_ = field_7620_ = 0;
    }
    ~NetWin();
};
#define free test_free
/* NETWIN_BODY_INSERTION */
#undef free

static NetWin *watch_window;
static void test_free(void *pointer) {
    if (pointer == watched_first) ++freed_first;
    if (pointer == watched_second) ++freed_second;
    std::free(pointer);
}

void Spot::shutdown() {
    assert(watch_window != nullptr);
    assert(watch_window->faction_records_ == nullptr);
    assert(watch_window->faction_records_copy_ == nullptr);
    assert(watch_window->field_761C_ == UINT32_MAX);
    assert(watch_window->field_7620_ == UINT32_MAX);
    calls.push_back("Spot.shutdown");
}

static void run_case(bool first, bool second) {
    calls.clear();
    freed_first = freed_second = 0;
    watched_first = first ? std::malloc(8) : nullptr;
    watched_second = second ? std::malloc(8) : nullptr;
    NetWin *window = new NetWin;
    window->faction_records_ = watched_first;
    window->faction_records_copy_ = watched_second;
    window->field_761C_ = 12;
    window->field_7620_ = 34;
    watch_window = window;
    delete window;
    assert(freed_first == (first ? 1u : 0u));
    assert(freed_second == (second ? 1u : 0u));
    watch_window = nullptr;
}

int main() {
    const std::vector<std::string> expected = {
        "Spot.shutdown",
        "StringBox.close",
        "EditBox.close",
        "CheckBox.close",
        "FlatButton1.close",
        "FlatButton2.close",
        "FlatButton2.BaseButton dtor",
        "FlatButton1.close",
        "FlatButton1.BaseButton dtor",
        "CheckBox.virtual bases",
        "EditBox.close",
        "EditBox.Time/GraphicWin",
        "StringBox.close",
        "StringBox.nested",
        "Spot::~",
        "GraphicWin::~",
    };
    for (int mask = 0; mask != 4; ++mask) {
        run_case((mask & 1) != 0, (mask & 2) != 0);
        assert(calls == expected);
    }
}

#elif NETWIN_OWNER_CONTROL_KIND == 2
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

class Random {
public:
    void reseed(unsigned) { seed = 0; }
    unsigned get(int, int max) {
        const unsigned result = values[index++ % values.size()];
        return max == 0 ? 0 : result % static_cast<unsigned>(max);
    }
    static std::vector<unsigned> values;
    static unsigned index;
    unsigned seed;
};
std::vector<unsigned> Random::values(1, 0);
unsigned Random::index = 0;

struct NetWinLobbyRow {
    uint8_t prefix_;
    uint8_t ready_or_occupied_;
    uint8_t field_2_;
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
struct NetWinFactionRecord {
    uint8_t record_type_;
    uint8_t record_count_;
    uint8_t field_2_[2];
    uint32_t owner_;
    uint8_t color_;
    uint8_t active_;
    char filename_[0x18];
    char leader_name_[0x18];
    char formal_faction_name_[0x28];
    char noun_faction_[0x18];
    uint8_t data_after_noun_[0x116];
};
struct AlphaNetProcessSlot {
    char name_[0x20];
    uint32_t pid_;
    int8_t identity_;
    uint8_t field_25_, field_26_;
    int8_t field_27_;
    uint8_t status_bytes_[0x174];
};
static_assert(sizeof(AlphaNetProcessSlot) == 0x19C, "process slot stride");
struct NetDaemon { AlphaNetProcessSlot process_slots_[8]; };
struct NetWinSessionBlock { int mode_; };
struct Player {
    int is_leader_female;
    char filename[24], search_key[24], name_leader[24], title_leader[24];
    char adj_leader[128], adj_insult_leader[128], adj_faction[128];
    char adj_insult_faction[128], padding1[128], noun_faction[24];
    int noun_gender, is_noun_plural;
    char adj_name_faction[128], formal_name_faction[40];
};
struct PlayerData { uint32_t prefix_[2]; int diff_level; };
class NetWin {
public:
    void *faction_records_;
    void prepare_game();
};

static NetWinLobbyRow NetWinLobbyRows[8];
static NetDaemon NetDaemonNet;
static NetWinSessionBlock NetWinSessionData;
static Player Players[8];
static PlayerData PlayersData[8];
static int NetWinPlayerId, NetWinOtherPlayerId;
static int faction_reads[8], modifier_calls[8], art_calls[8];
static int flag_loads;

static void read_faction(int faction) { ++faction_reads[faction]; }
static void compute_faction_modifiers(int faction) { ++modifier_calls[faction]; }
static void load_faction_art(int faction) { ++art_calls[faction]; }
static void load_flags() { ++flag_loads; }
static void _itoa(int value, char *text, int) {
    const std::string result = std::to_string(value);
    strcpy(text, result.c_str());
}
/* NETWIN_BODY_INSERTION */

static void reset() {
    memset(NetWinLobbyRows, 0, sizeof(NetWinLobbyRows));
    memset(&NetDaemonNet, 0xA5, sizeof(NetDaemonNet));
    memset(Players, 0, sizeof(Players));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(faction_reads, 0, sizeof(faction_reads));
    memset(modifier_calls, 0, sizeof(modifier_calls));
    memset(art_calls, 0, sizeof(art_calls));
    flag_loads = 0;
    Random::values.assign(1, 0);
    Random::index = 0;
}

int main() {
    NetWin win;
    NetWinFactionRecord records[40] = {};
    win.faction_records_ = records;
    records[0].record_count_ = 8;
    for (int faction = 0; faction < 8; ++faction) {
        records[faction].active_ = 1;
        strcpy(records[faction].filename_, "Faction");
    }

    reset();
    records[0].record_count_ = 8;
    for (int faction = 0; faction < 8; ++faction) {
        records[faction].active_ = 1;
        strcpy(records[faction].filename_, "Faction");
    }
    NetWinSessionData.mode_ = 0;
    for (int seat = 0; seat < 8; ++seat) {
        NetWinLobbyRows[seat].faction_or_color_ = static_cast<int8_t>(seat);
        strcpy(NetWinLobbyRows[seat].leader_name_, "Leader");
        strcpy(NetWinLobbyRows[seat].formal_faction_name_, "Formal");
        strcpy(NetWinLobbyRows[seat].noun_faction_, "Noun");
        strcpy(NetWinLobbyRows[seat].adjective_faction_, "AdjFaction");
        strcpy(NetWinLobbyRows[seat].leader_title_, "Title");
        strcpy(NetWinLobbyRows[seat].adjective_leader_, "AdjLeader");
    }
    NetWinLobbyRows[1].tail_[0] = 1;
    NetWinLobbyRows[4].field_2_ = 0xFE;
    uint8_t expected_slot_bytes[8][0x176];
    for (int seat = 0; seat < 8; ++seat) {
        memcpy(expected_slot_bytes[seat], &NetWinLobbyRows[seat], 0x176);
    }
    win.prepare_game();
    assert(strcmp(NetWinLobbyRows[2].leader_name_, "Leader1") == 0);
    assert(strcmp(NetWinLobbyRows[3].leader_name_, "Leader1") == 0);
    assert(strcmp(NetWinLobbyRows[4].leader_name_, "Leader1") == 0);
    assert(faction_reads[1] == 1 && modifier_calls[7] == 1);
    assert(strcmp(Players[4].formal_name_faction, "Formal") == 0);
    assert(Players[1].is_leader_female == 1);
    assert(NetDaemonNet.process_slots_[4].identity_ == 4);
    assert(NetDaemonNet.process_slots_[4].field_26_ ==
           NetWinLobbyRows[4].field_2_);
    for (int seat = 0; seat < 8; ++seat) {
        const AlphaNetProcessSlot &slot = NetDaemonNet.process_slots_[seat];
        const uint8_t *const bytes =
            reinterpret_cast<const uint8_t *>(&slot);
        expected_slot_bytes[seat][0] = static_cast<uint8_t>(seat);
        assert(memcmp(bytes + 0x24, expected_slot_bytes[seat], 0x176) == 0);
        for (int index = 0; index < 0x24; ++index) {
            assert(bytes[index] == 0xA5);
        }
        assert(bytes[0x19A] == 0xA5 && bytes[0x19B] == 0xA5);
    }
    assert(NetDaemonNet.process_slots_[4].field_26_ == 0xFE);
    assert(PlayersData[4].diff_level == -2);
    assert(flag_loads == 1);

    reset();
    records[0].record_count_ = 2;
    records[0].active_ = 0;
    records[1].active_ = 1;
    NetWinSessionData.mode_ = 2;
    NetWinPlayerId = 10;
    NetWinOtherPlayerId = 11;
    for (int seat = 0; seat < 8; ++seat) {
        NetWinLobbyRows[seat].faction_or_color_ = -1;
    }
    Random::values.assign(8, 1);
    win.prepare_game();
    assert(NetWinLobbyRows[1].faction_or_color_ == 1);
    assert(NetDaemonNet.process_slots_[1].identity_ == 1);
    for (int seat = 0; seat < 8; ++seat) {
        const AlphaNetProcessSlot &slot = NetDaemonNet.process_slots_[seat];
        const uint8_t *const bytes =
            reinterpret_cast<const uint8_t *>(&slot);
        assert(bytes[0x19A] == 0xA5 && bytes[0x19B] == 0xA5);
    }
    assert(art_calls[1] == 1 && art_calls[7] == 1);
    assert(flag_loads == 1);

    reset();
    memset(records, 0, sizeof(records));
    records[0].record_count_ = 34;
    for (int faction = 0; faction < 34; ++faction) {
        records[faction].active_ = 1;
        strcpy(records[faction].filename_, "Faction");
    }
    NetWinSessionData.mode_ = 2;
    NetWinPlayerId = 10;
    NetWinOtherPlayerId = 11;
    NetWinLobbyRows[0].faction_or_color_ = 0;
    NetWinLobbyRows[1].faction_or_color_ = -2;
    for (int seat = 2; seat < 8; ++seat) {
        NetWinLobbyRows[seat].faction_or_color_ = -1;
    }
    Random::values.assign(6, 33);
    Random::values[1] = 32;
    Random::values[2] = 31;
    Random::values[3] = 29;
    Random::values[4] = 28;
    Random::values[5] = 27;
    win.prepare_game();
    assert(NetWinLobbyRows[2].faction_or_color_ == 33);
    assert(NetWinLobbyRows[3].faction_or_color_ == 32);
    assert(NetWinLobbyRows[4].faction_or_color_ == 31);
    assert(NetWinLobbyRows[5].faction_or_color_ == 29);
    assert(NetWinLobbyRows[6].faction_or_color_ == 28);
    assert(NetWinLobbyRows[7].faction_or_color_ == 27);
    assert(flag_loads == 1);
}

#elif NETWIN_OWNER_CONTROL_KIND == 3
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#define __cdecl

struct NetWinLobbyRow {
    uint8_t prefix_[3];
    int8_t faction_or_color_;
    uint8_t reserved_[0x174];
    uint32_t owner_;
};
struct NetWinFactionRecord {
    uint32_t owner_;
    uint8_t padding_[0x5e];
    char noun_faction_[0x18];
};
struct NetWinSessionBlock { int mode_; };
struct char256 { char str[256]; };
class Scroll {
public:
    void close() { ++close_calls; events.push_back("scroll.close"); }
    ~Scroll() { close(); }
    static int close_calls;
    static std::vector<std::string> events;
};
int Scroll::close_calls = 0;
std::vector<std::string> Scroll::events;
class BasePop {
public:
    void close() { ++close_calls; Scroll::events.push_back("basepop.close"); }
    ~BasePop() { close(); }
    static int close_calls;
};
int BasePop::close_calls = 0;
class Popup : public BasePop {
public:
    void close() { scroll_.close(); BasePop::close(); }
    ~Popup() { close(); }
private:
    Scroll scroll_;
};
class Net {
public:
    int sends;
    int length;
    int target;
    int channel;
    uint8_t packet[0x18c];
    void send(const void *data, unsigned size, unsigned target_id, int c) {
        ++sends;
        length = size;
        target = target_id;
        channel = c;
        memcpy(packet, data, size);
    }
};
struct NetDaemon : Net {
    int slot;
    int pid_2_idx(int) { return slot; }
};
class NetWin {
public:
    void *faction_records_;
    int pick_pass(int payload);
};

static NetWinSessionBlock NetWinSessionData;
static NetWinLobbyRow NetWinLobbyRows[8];
static int NetWinPlayerId = 11;
static int NetWinOtherPlayerId = 22;
static NetDaemon NetDaemonNet;
static char ScriptPopupFile[] = "SCRIPT";
static char256 ParseStrBuffer[10];
char NetWinPasswordPromptInitialText;
char NetWinPasswordVerifyInitialText;
static int ask_results[8];
static const char *ask_texts[8];
static int ask_index;
static std::vector<std::string> events;

static void X_pop(const char *label, int (__cdecl *callback)()) {
    (void)callback;
    events.push_back(std::string("pop:") + label);
}
static int X_pop(char *, const char *label,
                              int (__cdecl *callback)()) {
    (void)callback;
    events.push_back(std::string("pop:") + label);
    return 0;
}
static int X_pop_ask(const char *label, int, char *initial_text,
                     int (__cdecl *callback)(), int) {
    (void)callback;
    events.push_back(std::string("ask:") + label);
    const int index = ask_index++;
    assert(initial_text != nullptr && initial_text[0] == 0);
    if (strcmp(label, "VERIFYPASSWORD") == 0) {
        assert(initial_text == &NetWinPasswordVerifyInitialText);
    } else {
        assert(initial_text == &NetWinPasswordPromptInitialText);
    }
    strcpy(ParseStrBuffer[0].str, ask_texts[index]);
    return ask_results[index];
}
static void parse_says(int, char *, int, int) {
    events.push_back("parse faction");
}
static uint32_t checksum_password(const char *value) {
    uint32_t result = 0;
    while (*value) result = result * 33 + static_cast<unsigned char>(*value++);
    return result;
}
/* NETWIN_BODY_INSERTION */

static void reset() {
    memset(NetWinLobbyRows, 0, sizeof(NetWinLobbyRows));
    memset(ParseStrBuffer, 0, sizeof(ParseStrBuffer));
    NetWinPasswordPromptInitialText = 0;
    NetWinPasswordVerifyInitialText = 0;
    memset(ask_results, 0, sizeof(ask_results));
    ask_index = 0;
    events.clear();
    Scroll::close_calls = 0;
    BasePop::close_calls = 0;
    Scroll::events.clear();
    NetDaemonNet.sends = 0;
    NetDaemonNet.slot = 2;
}

static void assert_popup_lifetime() {
    assert(Scroll::events.size() == 4);
    assert(Scroll::events[0] == "scroll.close");
    assert(Scroll::events[1] == "basepop.close");
    assert(Scroll::events[2] == "scroll.close");
    assert(Scroll::events[3] == "basepop.close");
}

int main() {
    NetWin win;
    NetWinFactionRecord records[8] = {};
    win.faction_records_ = records;

    reset();
    NetWinSessionData.mode_ = 2;
    NetWinLobbyRows[2].faction_or_color_ = -1;
    assert(win.pick_pass(0) == 1);
    assert(NetWinLobbyRows[2].owner_ == 0);
    assert(Scroll::close_calls == 2 && BasePop::close_calls == 2);
    assert_popup_lifetime();

    reset();
    NetWinSessionData.mode_ = 0;
    win.faction_records_ = nullptr;
    ask_results[0] = -1;
    ask_texts[0] = "cancel";
    assert(win.pick_pass(0) == 0);
    assert(ask_index == 1 && Scroll::close_calls == 2 &&
           BasePop::close_calls == 2);
    assert_popup_lifetime();

    reset();
    NetWinSessionData.mode_ = 2;
    win.faction_records_ = records;
    NetWinLobbyRows[2].faction_or_color_ = 3;
    strcpy(records[3].noun_faction_, "Spartan");
    records[3].owner_ = checksum_password("right");
    ask_results[0] = 0;
    ask_texts[0] = "right";
    assert(win.pick_pass(0) == 1);
    assert(NetWinLobbyRows[2].owner_ == records[3].owner_);
    assert(events.size() == 2 && events[0] == "parse faction" &&
           events[1] == "ask:ENTERPASSWORDFACTION");
    assert(NetDaemonNet.sends == 0);
    assert(Scroll::close_calls == 2 && BasePop::close_calls == 2);
    assert_popup_lifetime();

    reset();
    NetWinSessionData.mode_ = 3;
    win.faction_records_ = records;
    NetWinLobbyRows[2].faction_or_color_ = 3;
    records[3].owner_ = checksum_password("right");
    ask_results[0] = 0;
    ask_texts[0] = "wrong";
    assert(win.pick_pass(0) == 0);
    assert(events.size() == 3 && events[2] == "pop:BADPASSWORD");
    assert(NetDaemonNet.sends == 0 && Scroll::close_calls == 2 &&
           BasePop::close_calls == 2);
    assert_popup_lifetime();

    reset();
    NetWinSessionData.mode_ = 2;
    win.faction_records_ = records;
    NetWinLobbyRows[2].faction_or_color_ = 3;
    NetWinLobbyRows[2].owner_ = 0x55667788u;
    records[3].owner_ = checksum_password("protected");
    ask_results[0] = 0;
    ask_texts[0] = "";
    assert(win.pick_pass(0) == 0);
    assert(ask_index == 1 && NetWinLobbyRows[2].owner_ == 0x55667788u);
    assert(events.size() == 3 && events[2] == "pop:BADPASSWORD");
    assert(NetDaemonNet.sends == 0 && Scroll::close_calls == 2 &&
           BasePop::close_calls == 2);
    assert_popup_lifetime();

    reset();
    NetWinSessionData.mode_ = 0;
    win.faction_records_ = nullptr;
    NetWinLobbyRows[2].faction_or_color_ = 1;
    ask_results[0] = 0;
    ask_results[1] = 0;
    ask_texts[0] = "secret";
    ask_texts[1] = "secret";
    assert(win.pick_pass(0) == 1);
    assert(NetDaemonNet.sends == 1);
    assert(NetDaemonNet.length == 0x18c);
    assert(NetDaemonNet.target == 22 && NetDaemonNet.channel == 1);
    assert(NetWinLobbyRows[2].owner_ == checksum_password("secret"));
    assert(NetDaemonNet.packet[0] == 0x04 &&
           NetDaemonNet.packet[1] == 0x2f);
    uint32_t packet_slot, packet_zero;
    memcpy(&packet_slot, NetDaemonNet.packet + 4, sizeof(packet_slot));
    memcpy(&packet_zero, NetDaemonNet.packet + 8, sizeof(packet_zero));
    assert(packet_slot == 2 && packet_zero == 0);
    // Header +2..3 and +0xC..F are untouched by the original.
    assert(memcmp(NetDaemonNet.packet + 16, &NetWinLobbyRows[2], 0x17c) == 0);
    assert(Scroll::close_calls == 2 && BasePop::close_calls == 2);
    assert_popup_lifetime();

    reset();
    NetWinSessionData.mode_ = 0;
    win.faction_records_ = nullptr;
    NetWinLobbyRows[2].faction_or_color_ = 1;
    ask_results[0] = 0;
    ask_results[1] = 0;
    ask_results[2] = 0;
    ask_results[3] = 0;
    ask_texts[0] = "first";
    ask_texts[1] = "different";
    ask_texts[2] = "second";
    ask_texts[3] = "second";
    assert(win.pick_pass(0) == 1);
    assert(ask_index == 4 && NetDaemonNet.sends == 1);
    assert(events[2] == "pop:DIFFERENTPASSWORDS");
}

#else
#error unknown NetWin owner control
#endif
