// VC6-era language-subset control for the private candidate body. This is a
// mock harness, not evidence of native ABI or game acceptance.
#include <cassert>
#include <cstring>
#include <string>

#define __cdecl
#define _stricmp strcasecmp

enum { MaxPlayerNum = 8, RULES_IRONMAN = 0x400 };
struct Player {
    int is_leader_female;
    char name_leader[24];
    char noun_faction[24];
    int noun_gender;
    int is_noun_plural;
};
struct PlayerData { int unk_102; };
struct SaveNameDefaultStorage {
    char game_data_migration_default_[1];
    char unassigned_1_[3];
    char scenario_equal_default_[1];
    char unassigned_2_[3];
    char save_equal_default_[1];
};
struct LockStateEntry {
    unsigned leading_words_[4];
    union Payload { unsigned flags; int *vehicle_selection_follow; } payload_;
    unsigned trailing_words_[10];
};
struct ConsoleMock {
    int visible;
    int faction_id_;
    int is_visible() const { return visible; }
};
struct LockMock { int clear_count; void clear() { ++clear_count; } };
struct AlphaNet { int close_count; void close() { ++close_count; } };
struct NetMock : AlphaNet {
    LockStateEntry lock_state_[24];
    LockMock locks_;
    int hang_up_count;
    void hang_up() { ++hang_up_count; }
};
struct FileBox {
    int set_def_name(const char *) { return 0; }
    int set_def_ext(const char *) { return 0; }
    int add_filter(char *, char *) { return 0; }
    char *save() { return box_selection; }
    static char *box_selection;
};
char *FileBox::box_selection = 0;
struct AlphaSave {
    void init() {}
    int set_default_filename(char *) { return 0; }
    int add_default_extension(char *) { return 0; }
    char *save() { return alpha_selection; }
    static char *alpha_selection;
};
char *AlphaSave::alpha_selection = 0;

static ConsoleMock ConsoleGlobal = {0, 0};
static int WorldClimateSkipTerrainClear;
static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
static NetMock NetDaemonNet;
static char StringTemp[1032];
static char LegacyGameSaveName[256];
static char ScenarioSaveNameBuffer[256];
static SaveNameDefaultStorage SaveNameDefaultStorageGlobal;
static const char *SaveGameFileExtension = "SAV";
static const char *ScenarioFileExtension = "SC";
static int GenderDefault, PluralityDefault;
static unsigned GameState, GameRules;
static int IsMultiplayerPBEM, IsMultiplayerNet, ExitTurnLoop;
static int pref_value, save_status, error_count, ironman_count, draw_count;
static int autosave_count;
static std::string saved_path;
static std::string latest_save;

char *label_get(int index) {
    if (index == 0x2D0 / 4) return (char *)"Title";
    if (index == 0x9C8 / 4) return (char *)"Scenario";
    if (index == 0x674 / 4) return (char *)"Saved";
    assert(0);
    return 0;
}
void say_year(char *value) { std::strcat(value, "2150"); }
void yearmotize(char *) {}
int prefs_get(const char *, int fallback, int) { return pref_value == -999 ? fallback : pref_value; }
void prefs_put(const char *, int value, int) { assert(value == pref_value); }
void prefs_put(const char *key, const char *value) {
    if (std::strcmp(key, "Latest Save") == 0) latest_save = value;
}
int save_daemon(char *path) { saved_path = path; return save_status; }
int parse_says(int, const char *, int, int) { return 0; }
int X_pop(const char *name, int (*)()) {
    if (std::strcmp(name, "SAVEFAILED") == 0) ++error_count;
    if (std::strcmp(name, "IRONMAN") == 0) ++ironman_count;
    return 0;
}
void do_all_draws() { ++draw_count; }
void auto_save() { ++autosave_count; }

#include "save_game_candidate.inc"

static void initialize() {
    std::memset(Players, 0, sizeof Players);
    std::memset(PlayersData, 0, sizeof PlayersData);
    std::memset(StringTemp, 0, sizeof StringTemp);
    std::memset(LegacyGameSaveName, 0, sizeof LegacyGameSaveName);
    std::memset(ScenarioSaveNameBuffer, 0, sizeof ScenarioSaveNameBuffer);
    std::memset(&SaveNameDefaultStorageGlobal, 0,
                sizeof SaveNameDefaultStorageGlobal);
    std::memset(&NetDaemonNet, 0, sizeof NetDaemonNet);
    Players[0].name_leader[0] = 0;
    std::strcpy(Players[0].name_leader, "Leader");
    std::strcpy(Players[0].noun_faction, "Faction");
    ConsoleGlobal.visible = ConsoleGlobal.faction_id_ = 0;
    WorldClimateSkipTerrainClear = 0;
    GameState = GameRules = 0;
    IsMultiplayerPBEM = IsMultiplayerNet = ExitTurnLoop = 0;
    pref_value = save_status = error_count = ironman_count = draw_count = 0;
    autosave_count = 0;
    saved_path.clear();
    latest_save.clear();
}

int main() {
    char selection[] = "C:\\saves\\chosen.sav";
    initialize();
    FileBox::box_selection = selection;
    AlphaSave::alpha_selection = selection;
    save_status = 1;
    assert(save_game(0) == 1);
    assert(saved_path == selection && error_count == 1 && draw_count == 2);
    assert(std::strcmp(LegacyGameSaveName, "chosen") == 0);

    initialize();
    FileBox::box_selection = selection;
    save_status = 0;
    std::strcpy(ScenarioSaveNameBuffer, "old scenario");
    PlayersData[0].unk_102 = 44;
    assert(save_game(1) == 0);
    assert(std::strcmp(ScenarioSaveNameBuffer, "chosen") == 0);
    assert((GameState & 0x10) != 0 && PlayersData[0].unk_102 == 0);

    initialize();
    char matching_selection[] = "C:\\saves\\Leader Title Faction,2150.SAV";
    FileBox::box_selection = matching_selection;
    save_status = 0;
    std::strcpy(ScenarioSaveNameBuffer, "old scenario");
    PlayersData[0].unk_102 = 44;
    assert(save_game(1) == 0);
    assert(ScenarioSaveNameBuffer[0] == 0);
    assert((GameState & 0x10) == 0 && PlayersData[0].unk_102 == 44);

    initialize();
    FileBox::box_selection = matching_selection;
    save_status = 0;
    assert(save_game(0) == 0);
    assert(LegacyGameSaveName[0] == 0);
    assert(latest_save == "Leader Title Faction,2150");

    initialize();
    char auto_path[] = "auto\\";
    FileBox::box_selection = auto_path;
    save_status = 0;
    PlayersData[0].unk_102 = 44;
    assert(save_game(1) == 0);
    assert(saved_path == "auto\\");
    assert((GameState & 0x10) == 0 && PlayersData[0].unk_102 == 44);

    initialize();
    FileBox::box_selection = selection;
    save_status = 0;
    IsMultiplayerPBEM = 1;
    GameRules = RULES_IRONMAN;
    assert(save_game(0) == 0);
    assert(ExitTurnLoop == 1 && error_count == 0);

    initialize();
    FileBox::box_selection = selection;
    save_status = 0;
    GameRules = RULES_IRONMAN;
    IsMultiplayerNet = 1;
    std::memset(NetDaemonNet.lock_state_, 0x78,
                sizeof NetDaemonNet.lock_state_);
    assert(save_game(0) == 0);
    assert(autosave_count == 1 && ironman_count == 1);
    assert(NetDaemonNet.hang_up_count == 1);
    assert(NetDaemonNet.locks_.clear_count == 1);
    assert(NetDaemonNet.close_count == 1 && IsMultiplayerNet == 0);
    assert(NetDaemonNet.lock_state_[2].trailing_words_[6] == 0xFF);
    assert(NetDaemonNet.lock_state_[2].trailing_words_[9] == 0x78787878);

    initialize();
    FileBox::box_selection = 0;
    assert(save_game(0) == 1);
    assert(saved_path.empty() && error_count == 0);
    return 0;
}
