#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

struct NetWinSessionOptionsState {
    uint8_t status_, player_count_, difficulty_, time_control_;
    uint8_t option_4_, option_5_, planet_size_, ocean_coverage_;
    uint8_t erosion_, native_life_, cloud_cover_, reserved_B_;
    uint32_t rule_bits_a_, rule_bits_b_, rule_bits_c_;
};
struct NetWinSessionBlock { uint32_t mode_; uint8_t state_[0x164]; };
struct NetWinLobbyPacketHeader {
    uint8_t opcode_, flags_;
    uint16_t field_2_;
    int32_t slot_, field_8_;
    uint32_t field_C_;
};
struct NetWinLobbyRow {
    uint8_t prefix_, ready_or_occupied_, field_2_;
    int8_t faction_or_color_;
    uint8_t field_4_;
    char leader_name_[0x18], formal_faction_name_[0x28], noun_faction_[0x18];
    char adjective_faction_[0x80], leader_title_[0x18], adjective_leader_[0x80];
    uint8_t tail_[3];
    uint32_t owner_;
};
/* NETWIN_FACTION_RECORD_INSERTION */
struct NetWinLobbyFullPacket {
    NetWinLobbyPacketHeader header_;
    NetWinLobbyRow rows_[7];
};
struct NetWinLobbyLeavePacket {
    NetWinLobbyPacketHeader header_;
    uint8_t trailing_[0x10];
};
struct NetWinSessionOptionsPacket {
    uint16_t opcode_;
    uint8_t reserved_[0x0E];
    NetWinSessionOptionsState options_;
};
struct NetWinSessionPacket {
    NetWinLobbyPacketHeader header_;
    NetWinSessionBlock session_;
};

struct PopMenu {
    struct Dialogs { void item(char *, int) {} } dialogs_;
    void init() {}
    Dialogs &dialogs() { return dialogs_; }
};
struct Buffer {
    int load_pcx(const char *, int, int, int) { return 0; }
    int get_pixel(int, int) const { return 0x32; }
};
struct Net {
    int sends = 0;
    unsigned sizes[8] = {};
    uint8_t payloads[8][0xA74] = {};
    void send(void *data, unsigned size, int, int) {
        assert(sends < 8 && size <= sizeof(payloads[0]));
        sizes[sends] = size;
        std::memcpy(payloads[sends], data, size);
        ++sends;
    }
};
struct NetDaemon : Net {
    int pid_calls = 0;
    int player_count_ = 3;
    int pid_2_idx(uint32_t) { ++pid_calls; return 1; }
};
struct Player {
    char filename[0x18] = "faction";
    char name_leader[0x18] = "leader";
    char formal_name_faction[0x28] = "formal";
    char noun_faction[0x18] = "noun";
    char adj_name_faction[0x80] = "adjective";
    char title_leader[0x18] = "title";
    char adj_leader[0x80] = "adjleader";
    int is_leader_female = 0;
};
struct PlayerData { uint32_t unk_102 = 0; };
struct DefaultPreferences { uint32_t difficulty = 5; };
struct AlphaIniPreference { uint32_t preferences, more_preferences, announce, rules, semaphore; };

struct NetWin {
    void *faction_records_ = nullptr;
    void *faction_records_copy_ = nullptr;
    int field_7728_ = 0;
    char loaded_file_path_[0x104] = "C:\\scenario.scn";
    void pick_map();
};

static int scripted_choice = -1;
static int map_result = 0, game_result = 0;
static int map_loads = 0, game_loads = 0, last_game_host = -1;
static int pops = 0, parses = 0;
static int parse_value = -1, faction_sends = 0;
static char *label_get(int) { return const_cast<char *>("menu item"); }
static int pick_answer(NetWin &, PopMenu &) { return scripted_choice; }
static int load_map() { ++map_loads; return map_result; }
static int load_game(int host, int) {
    ++game_loads;
    last_game_host = host;
    return game_result;
}
static int bit_count(int value) {
    unsigned bits = static_cast<unsigned>(value);
    int count = 0;
    while (bits) { count += static_cast<int>(bits & 1u); bits >>= 1; }
    return count;
}
static void parse_num(int value, int) { ++parses; parse_value = value; }
static void X_pop(const char *, int) { ++pops; }
static int MapLongitudeBounds = 48, MapLatitudeBounds = 32;
struct GameDataFields {
    struct { uint32_t enable_mask_; } lock_masks_;
    uint32_t net_turn_flags_, diff_level_current_, game_rules_, game_state_;
};
struct GameDataStorage { GameDataFields fields_; };
static GameDataStorage GameDataStateStorage =
    {{{0x700u}, 0x10u, 4u, 0x13572468u, 0x01800040u}};
static int GameDataLoadRequest = 0;
static DefaultPreferences DefaultPrefs;
static AlphaIniPreference AlphaIniPrefs = {0, 0, 0, 0xA5A55A5Au, 0};
static NetWinSessionOptionsState NetWinSessionOptions;
static NetWinSessionBlock NetWinSessionData;
static NetWinLobbyRow NetWinLobbyRows[8];
static NetDaemon NetDaemonNet;
static uint32_t NetWinPlayerId = 1;
static int NetWinOtherPlayerId = 2;
static int netwin_rules_gate = 1;
static int *const NetWinRulesGate = &netwin_rules_gate;
static Player Players[8];
static PlayerData PlayersData[8];
static char string_temp[256];
static char *StringTemp = string_temp;
static char NetWinScenarioStorage[0x164];
static char *NetWinScenarioText = NetWinScenarioStorage;
static const char *NetWinPcxSuffix = ".pcx";
static const char *NetWinDefaultPcx = "faction.pcx";

static void send_faction_data(int) { ++faction_sends; }

#define CONTROL_REQUIRE(condition, message) do { \
    if (!(condition)) { \
        std::fprintf(stderr, "CONTROL FAILURE: %s\n", message); \
        return 98; \
    } \
} while (0)

/* NETWIN_PICK_MAP_BODY_INSERTION */

static void reset_case() {
    scripted_choice = -1;
    map_result = game_result = 0;
    map_loads = game_loads = pops = parses = faction_sends = 0;
    last_game_host = -1;
    parse_value = -1;
    NetDaemonNet.sends = NetDaemonNet.pid_calls = 0;
    NetWinSessionOptions = {};
    NetWinSessionOptions.rule_bits_b_ = 0x76543210u;
    NetWinSessionData = {};
    NetWinSessionData.state_[0x20] = 0xA5;
    NetWinScenarioStorage[0] = 0;
    GameDataLoadRequest = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x700u;
    netwin_rules_gate = 1;
}

int main() {
    {
        reset_case();
        NetWin win;
        scripted_choice = -1;
        win.pick_map();
        assert(NetDaemonNet.sends == 0 && NetDaemonNet.pid_calls == 0);
        assert(NetWinSessionData.state_[0x20] == 0xA5);
    }
    {
        reset_case();
        uint8_t source[0x194 + 8 * 399] = {};
        uint8_t destination[sizeof(source)] = {};
        source[1] = 8;
        NetWin win;
        win.faction_records_copy_ = source;
        win.faction_records_ = destination;
        scripted_choice = 0;
        win.pick_map();
        assert(std::memcmp(source, destination, sizeof(source)) == 0);
        assert(win.field_7728_ == 0);
        assert(NetWinSessionData.state_[0x20] == 0);
        assert(NetDaemonNet.sends == 3 && NetDaemonNet.pid_calls == 2);
        assert(faction_sends == 0);
        assert(NetDaemonNet.sizes[0] == 0x28 && NetDaemonNet.sizes[1] == 0xA74);
        assert(NetDaemonNet.sizes[2] == 0x178);
        const NetWinSessionOptionsState &early =
            reinterpret_cast<const NetWinSessionOptionsPacket *>(
                NetDaemonNet.payloads[0])->options_;
        assert(early.status_ == 0xFF && early.difficulty_ == 5);
        assert(early.rule_bits_a_ == 0xA5A55A5Au);
        assert(reinterpret_cast<NetWinSessionOptionsPacket *>(NetDaemonNet.payloads[0])->opcode_ == 0x2F02);
        assert(reinterpret_cast<NetWinLobbyFullPacket *>(NetDaemonNet.payloads[1])->header_.flags_ == 0x4F);
        assert(reinterpret_cast<NetWinSessionPacket *>(NetDaemonNet.payloads[2])->header_.opcode_ == 3);
        assert(reinterpret_cast<NetWinSessionPacket *>(NetDaemonNet.payloads[2])->header_.flags_ == 0x2F);
    }
    {
        reset_case();
        NetWin win;
        scripted_choice = 1;
        win.pick_map();
        assert(map_loads == 1 && game_loads == 0);
        assert(GameDataLoadRequest == 0);
        assert(NetDaemonNet.sends == 3);
        assert(NetWinSessionData.state_[0x20] == 0xA5);
    }
    {
        reset_case();
        NetWin win;
        scripted_choice = 1;
        map_result = -1;
        win.pick_map();
        assert(map_loads == 1);
        assert(GameDataLoadRequest == 1);
        assert(NetDaemonNet.sends == 0);
    }
    {
        reset_case();
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x300u;
        NetDaemonNet.player_count_ = 3;
        NetWin win;
        scripted_choice = 2;
        win.pick_map();
        assert(game_loads == 1 && parses == 1 && parse_value == 1 && pops == 1);
        assert(GameDataLoadRequest == 1); // native early-return leaves request active
        assert(NetDaemonNet.sends == 0);
    }
    {
        reset_case();
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xF00u;
        NetWin win;
        NetWinFactionRecord scenario_records[10] = {};
        win.faction_records_ = scenario_records;
        win.field_7728_ = 1;
        scripted_choice = 2;
        win.pick_map();
        assert(game_loads == 1 && last_game_host == 1);
        assert(GameDataLoadRequest == 0);
        assert(NetWinSessionData.state_[0x20] == 0);
        assert(std::strcmp(NetWinScenarioStorage, "scenario.scn") == 0);
        assert(NetDaemonNet.sends == 4);
        const NetWinSessionOptionsState &published =
            reinterpret_cast<const NetWinSessionOptionsPacket *>(
                NetDaemonNet.payloads[0])->options_;
        assert(published.status_ == 0x0F);
        assert(published.difficulty_ == 4);
        assert(published.rule_bits_a_ == 0x13572468u);
        assert(published.rule_bits_c_ == 0x10u);
        assert(published.rule_bits_b_ == 0x76543210u);
    }
    {
        reset_case();
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x7F00u;
        NetWin win;
        NetWinFactionRecord scenario_records[10] = {};
        win.faction_records_ = scenario_records;
        win.field_7728_ = 2;
        scripted_choice = 2;
        std::strcpy(Players[1].name_leader, "RosterLeader");
        std::strcpy(Players[1].formal_name_faction, "RosterFormal");
        std::strcpy(Players[1].noun_faction, "RosterNoun");
        std::strcpy(Players[1].adj_name_faction, "RosterAdjective");
        std::strcpy(Players[1].title_leader, "RosterTitle");
        std::memset(Players[1].adj_leader, 'q', sizeof(Players[1].adj_leader));
        Players[1].adj_leader[0x7F] = 0;
        Players[1].is_leader_female = 0x6E;
        win.pick_map();
        CONTROL_REQUIRE(std::strcmp(scenario_records[1].leader_name_,
                                    "RosterLeader") == 0,
                        "server roster preserves leader name");
        CONTROL_REQUIRE(std::strcmp(scenario_records[1].formal_faction_name_,
                                    "RosterFormal") == 0,
                        "server roster preserves formal name");
        CONTROL_REQUIRE(std::strcmp(scenario_records[1].noun_faction_,
                                    "RosterNoun") == 0,
                        "server roster preserves noun");
        CONTROL_REQUIRE(std::strcmp(scenario_records[1].adjective_faction_,
                                    "RosterAdjective") == 0,
                        "server roster stores adjective in named +0x7A slot");
        CONTROL_REQUIRE(std::strcmp(scenario_records[1].leader_title_,
                                    "RosterTitle") == 0,
                        "server roster stores title in named +0xFA slot");
        CONTROL_REQUIRE(scenario_records[1].unresolved_112_[0] == 'q' &&
                        scenario_records[1].unresolved_112_[0x7D] == 'q',
                        "server roster copies leader adjective through unresolved tail");
        CONTROL_REQUIRE(scenario_records[2].record_type_ == 'q' &&
                        scenario_records[2].record_count_ == 0,
                        "server leader adjective still spills two bytes across stride");
        CONTROL_REQUIRE(scenario_records[2].field_2_[0] == 0x6E,
                        "server gender byte still aliases next row at +0x192");
        CONTROL_REQUIRE(faction_sends == 1,
                        "completed server roster is sent once");
    }
    {
        reset_case();
        uint8_t source[0x194 + 8 * 399] = {};
        uint8_t destination[sizeof(source)] = {};
        source[1] = 8;
        NetWin win;
        win.faction_records_copy_ = source;
        win.faction_records_ = destination;
        win.field_7728_ = 1;
        scripted_choice = 0;
        win.pick_map();
        assert(win.field_7728_ == 0);
        assert(faction_sends == 0);
    }
    {
        reset_case();
        uint8_t source[0x194 + 8 * 399] = {};
        uint8_t destination[sizeof(source)] = {};
        source[1] = 8;
        NetWin win;
        win.faction_records_copy_ = source;
        win.faction_records_ = destination;
        win.field_7728_ = 2;
        scripted_choice = 0;
        win.pick_map();
        assert(win.field_7728_ == 0);
        assert(faction_sends == 1);  // tail tests entry mode, not mutated mode
    }
    {
        reset_case();
        NetWin win;
        scripted_choice = 4;  // original tests only choices 0/1/2 specially
        win.pick_map();
        assert(game_loads == 1 && last_game_host == 0);
        assert(GameDataLoadRequest == 0);
        assert(parses == 0 && pops == 0);
        assert(NetDaemonNet.sends == 3);
    }
    {
        reset_case();
        NetWin win;
        scripted_choice = 3;
        game_result = -1;
        win.pick_map();
        assert(game_loads == 1 && last_game_host == 0);
        assert(GameDataLoadRequest == 1);
        assert(NetDaemonNet.sends == 0);
    }
    return 0;
}
