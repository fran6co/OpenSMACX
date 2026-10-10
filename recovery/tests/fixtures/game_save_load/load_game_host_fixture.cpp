#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <stdint.h>

#define __cdecl

static int dialog_cancel = 0;
static int daemon_results[8] = {0};
static int daemon_calls = 0;
static int prompt_results[8] = {0};
static int prompt_calls = 0;
static int open_dialog_calls = 0;
static int file_box_calls = 0;
static int retry_closes = 0;
static int draw_calls = 0;
static int show_calls = 0;
static int hide_calls = 0;
static int password_pop_calls = 0;
static char parsed_filename[256] = "";
static char observed_default_name[256] = "";
static char observed_filter[256] = "";
static int selected_mode = 0;
static char selected_file[256] = "saves\\test.sav";
static char pref_value[256] = "";

enum { MaxPlayerNum = 8, MaxDiffNum = 6 };
struct FileBox {
    int set_def_name(char *name) {
        std::strcpy(observed_default_name, name);
        return 0;
    }
    int set_def_ext(char *) { return 0; }
    int add_filter(char *filter, char *) {
        std::strcpy(observed_filter, filter);
        return 0;
    }
    char *load() { ++file_box_calls; return dialog_cancel ? 0 : selected_file; }
};
struct AlphaSave {
    void init() {}
    int set_default_filename(char *) { return 0; }
    int add_default_extension(char *) { return 0; }
    char *load() { ++open_dialog_calls; return dialog_cancel ? 0 : selected_file; }
    void close() {}
    void close_for_load_retry() { ++retry_closes; }
};
struct PlayerData { int unk_102; };
struct Player {
    int is_leader_female;
    char adj_name_faction[128];
    char name_leader[24];
    int noun_gender;
    int is_noun_plural;
    char noun_faction[24];
};
struct Console { int faction_id_; };
struct Button {
    void show(int) { ++show_calls; }
    void hide() { ++hide_calls; }
};
struct Interface {
    Button &flat_button(int) { static Button button; return button; }
};
struct Label { int *strings_ptr; };
struct FakeStrings { char *get(int) { return const_cast<char *>("YEAR"); } };
struct Sprite {};
struct NetWin {
    char loaded_file_path_[0x104];
    int field_7728_;
};
static int label_ids[0x300] = {0};
PlayerData PlayersData[MaxPlayerNum] = {};
Player Players[MaxPlayerNum] = {};
Console ConsoleGlobal = {0};
Interface MainInterfaceGlobal;
Label Labels = {label_ids};
FakeStrings StringTable;
char *Difficulty[MaxDiffNum] = {const_cast<char *>("DIFF")};
char StringTemp[1032] = {};
char SaveNameBuffer[256] = "old";
char ScenarioSaveNameBuffer[256] = {};
char *SaveGameFileExtension = const_cast<char *>("SAV");
char *ScenarioFileExtension = const_cast<char *>("SC");
char *ScriptPopupFile = const_cast<char *>("SCRIPT");
NetWin NetWinDialog = {};
int ExitTurnLoop = 0;
int GameSessionActive = 0;
int DiffLevelCurrent = 0;
struct GameDataFields {
    unsigned int game_state_, game_preferences_, diff_level_current_;
};
struct GameDataStorage { GameDataFields fields_; };
GameDataStorage GameDataStateStorage = {};
int IsMultiplayerNet = 0;
int IsMultiplayerPBEM = 0;
unsigned int WorldClimateSkipTerrainClear = 1;
unsigned int DialogDefaults[37] = {};
int GenderDefault = 0;
int PluralityDefault = 0;
Sprite *FactionDiplomacyPortraits[MaxPlayerNum] = {};

char *label_get(int) { return const_cast<char *>("FILTER"); }
char *prefs_get(const char *, const char *, int) { return pref_value; }
int prefs_get(const char *, int, int) { return selected_mode; }
void prefs_put(const char *, int, int) {}
void prefs_put(const char *, const char *) {}
void prefs_use() {}
void do_all_draws() { ++draw_calls; }
int load_daemon(const char *, int) {
    const int value = daemon_results[daemon_calls % 8];
    ++daemon_calls;
    return value;
}
void load_faction_art(int) {}
void load_flags() {}
void filefind_set_alternative(const char *) {}
int X_pop(const char *, int (*)()) { return 0; }
int X_pop(char *, const char *, int (*)()) { ++password_pop_calls; return 0; }
int X_pops(const char *, int, Sprite *, int (*)()) {
    const int value = prompt_results[prompt_calls % 8];
    ++prompt_calls;
    return value;
}
int parse_says(int id, char *text, int, int) {
    if (id == -1) std::strcpy(parsed_filename, text);
    return 0;
}
int parse_say(int, char *, int, int) { return 0; }
int parse_num(int, int) { return 0; }
void say_year(char *text) { std::strcat(text, " 2100"); }
char *_itoa(int value, char *text, int) { std::sprintf(text, "%d", value); return text; }

// The test replaces this marker with the actual balanced product body.
#include "load_game_candidate.inc"

int main(int argc, char **argv) {
    if (argc > 1 && std::strcmp(argv[1], "cancel") == 0) dialog_cancel = 1;
    if (argc > 1 && std::strcmp(argv[1], "scenario") == 0) {
        selected_mode = 1;
        selected_file[0] = '\0';
        std::strcpy(selected_file, "scenarios\\sample.sc");
        IsMultiplayerNet = 1;
        GameDataStateStorage.fields_.game_state_ = 0xC0;
    }
    if (argc > 1 && std::strcmp(argv[1], "dot") == 0) {
        std::strcpy(selected_file, "saves\\test.part.sav");
        IsMultiplayerNet = 1;
    }
    if (argc > 1 && std::strcmp(argv[1], "retry") == 0) {
        prompt_results[0] = -1;
        prompt_results[1] = 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "daemon-error") == 0)
        daemon_results[0] = 2;
    if (argc > 1 && std::strcmp(argv[1], "password") == 0) {
        PlayersData[1].unk_102 = 1;
        WorldClimateSkipTerrainClear = 0;
    }
    if (argc > 1 && std::strcmp(argv[1], "password-skip") == 0)
        PlayersData[1].unk_102 = 1;
    if (argc > 1 && std::strcmp(argv[1], "prompt-positive") == 0)
        prompt_results[0] = 7;
    const int mode = (argc > 1 && std::strcmp(argv[1], "scenario") == 0) ? 1 : 0;
    const int result = load_game(mode, 7);
    const char *expected_default = mode != 0 ? "scenarios\\*.SC"
                                              : "saves\\*.SAV";
    const char *expected_filter = mode != 0 ? "*.scenarios\\*.SC"
                                             : "*.saves\\*.SAV";
    if (std::strcmp(observed_default_name, expected_default) != 0 ||
        std::strcmp(observed_filter, expected_filter) != 0)
        return 10;
    if (argc > 1 && std::strcmp(argv[1], "cancel") == 0)
        return result == 1 && daemon_calls == 0 ? 0 : 1;
    if (argc > 1 && std::strcmp(argv[1], "scenario") == 0)
        return result == 0 && NetWinDialog.field_7728_ == 3 &&
                       std::strcmp(NetWinDialog.loaded_file_path_, "scenarios\\sample") == 0 &&
                       hide_calls == 1 && show_calls == 0 && prompt_calls == 0
                   ? 0 : 2;
    if (argc > 1 && std::strcmp(argv[1], "dot") == 0)
        return std::strcmp(NetWinDialog.loaded_file_path_, "saves\\test") == 0
                   ? 0 : 5;
    if (argc > 1 && std::strcmp(argv[1], "retry") == 0)
        return result == 0 && daemon_calls == 2 && prompt_calls == 2 &&
                       retry_closes == 1
                   ? 0 : 3;
    if (argc > 1 && std::strcmp(argv[1], "daemon-error") == 0)
        return result == 2 && daemon_calls == 1 && prompt_calls == 0 &&
                       std::strcmp(parsed_filename, selected_file) == 0
                   ? 0 : 4;
    if (argc > 1 && std::strcmp(argv[1], "password") == 0)
        return result == 5 && password_pop_calls == 1 && ExitTurnLoop == 1
                   ? 0 : 6;
    if (argc > 1 && std::strcmp(argv[1], "password-skip") == 0)
        return result == 5 && password_pop_calls == 1 && ExitTurnLoop == 0
                   ? 0 : 7;
    if (argc > 1 && std::strcmp(argv[1], "prompt-positive") == 0)
        return result == 0 && prompt_calls == 1 ? 0 : 8;
    return result == 0 && prompt_calls == 1 ? 0 : 4;
}
