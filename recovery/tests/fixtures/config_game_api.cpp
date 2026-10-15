#ifndef CONFIG_GAME_HOST_FIXTURE_V1_H
#define CONFIG_GAME_HOST_FIXTURE_V1_H
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <stdexcept>
#define __cdecl
typedef unsigned int uint32_t;
typedef char *LPSTR;
enum { MaxPlayerNum = 8 };
struct Player {
    int is_leader_female;
    char filename[24], search_key[24], name_leader[24], title_leader[24];
    char adj_leader[128], noun_faction[24], adj_name_faction[128];
    char formal_name_faction[40], desc_name_faction[24];
    int noun_gender, is_noun_plural, rule_flags;
};
struct Sprite {};
struct FactionArt { Sprite diplomacy; };
struct Font {};
struct Landmark { char name[32]; };
struct ParseString { char str[256]; };
struct AlphaIniPref { uint32_t rules, semaphore, customize, time_controls; };
struct DefaultPref { uint32_t difficulty, faction_id; };
struct ConsoleFixture { int faction_id_; };
struct SYSTEMTIME { unsigned short wYear, wMonth, wDayOfWeek, wDay; };
extern std::vector<std::string> events;
void event(const std::string &name);
int answer(const std::string &name);
int action_value();
int config_rand();
#define rand config_rand
int _strnicmp(const char *, const char *, size_t);
class Win {
public:
    void hide() { event("setup.hide"); }
    void show(int value) { assert(value == 0); event("setup.show"); }
    void set_caption(char *) { event("preview.caption"); }
};
class GraphicWin : public Win {
public:
    void redraw() { event("picker.redraw"); }
    void close() { event("picker.close"); }
};
class Dialog {
public:
    int field_24_, field_28_;
    int selected;
    int get_selected_id() { return selected; }
};
class ListBox : public Dialog {
public:
    void (__cdecl *callback)(int);
    int item(char *text, int id) { event(std::string("picker.row:") + text); return id; }
    int attach(GraphicWin *, int, int, int flags) { assert(flags == 0x80020); return 0; }
    void set_selected_id(int id) { selected = id; }
    void set_selection_callback(void (__cdecl *cb)(int)) { callback = cb; }
};
class EditGroup {
public:
    int edit(char *label, char *text, int limit) {
        assert(limit == 0x17 || limit == 0x27);
        event(std::string("edit:") + label + "=" + text);
        return 0;
    }
};
class SpriteBox {
public:
    int sprite(Sprite *, char *, int) { event("preview.sprite"); return 0; }
};
class Dialogs : public ListBox, public EditGroup, public SpriteBox {
public:
    int item(char *text, int id) { event(std::string("popup.row:") + text); return id; }
};
class Popup;
extern Popup ConfigFactionPreview;
class BasePop : public GraphicWin {
public:
    std::string section;
    int action, selected_value;
    BasePop() : action(0), selected_value(0) {}
    int exec(int, int (*)());
    int string(char *text) { event(std::string("popup.string:") + text); return 0; }
    uint32_t input_poll_state() const { return action; }
    void set_value(int value) { selected_value = value; }
    void set_caller_picture(Sprite *) { event("preview.picture"); }
    bool picture_enabled() const { return false; }
    void set_string_font(Font *, Font *, Font *, Font *) { event("preview.fonts"); }
    void set_loc(int x, int y) { assert(x == 5 && y == 0x136); event("preview.loc"); }
};
class Popup : public BasePop {
public:
    Dialogs d;
    Popup() { event("popup.ctor"); }
    ~Popup() { event("popup.dtor"); }
    void close() { event(this == &ConfigFactionPreview ? "preview.close" : "popup.close"); }
    int start(char *, const char *label, int, char *, int, GraphicWin *) {
        section = label;
        event(std::string(this == &ConfigFactionPreview ? "preview.start:" : "popup.start:") + label);
        return 0;
    }
    Dialogs &dialogs() { return d; }
};
class SetupWin : public GraphicWin {
public:
    SetupWin() { event("setup.ctor"); }
    ~SetupWin() { event("setup.dtor"); }
    int do_menu(Popup *pop, int kind, int flags) {
        assert(kind == 1 && flags == 0); return answer(pop->section);
    }
};
class PickWin : public GraphicWin {
public:
    ListBox listBox_;
    uint32_t field_5DA4_, field_5DB4_;
    void init(void (*)(int), int, int) { event("picker.init"); }
    int go_girl() { int r = answer("PICKER"); field_5DA4_ = action_value(); return r; }
};
class Sound { public: void fade(unsigned long amount) { assert(amount == 0x7D0); event("wave.fade"); } };
class Wave : public Sound {
public:
    void unload() { event("wave.unload"); }
    int load(const char *name) { event(std::string("wave.load:") + name); return 0; }
    int load(char *name, unsigned long flags) { assert(flags == 0x100); event(std::string("wave.flagload:") + name); return 0; }
    int play() { event("wave.play"); return 0; }
};
class Strings {
public:
    char *get(int id);
    char *get(const char *key);
};
extern Player Players[8];
extern FactionArt FactionsArt[8];
extern Landmark MapLandmark[64];
extern ParseString ParseStrBuffer[10];
extern AlphaIniPref AlphaIniPrefs;
extern DefaultPref DefaultPrefs;
extern ConsoleFixture ConsoleGlobal;
extern PickWin PickWinGlobal;
extern Wave g_WAVE_GENERAL;
extern Strings StringTable;
extern LPSTR Difficulty[6];
extern char StringTemp[4096], PopupStartCaption[256];
extern char *TextBufferGetPtr;
extern const char *AlphaxFileID, *ScriptTxtID;
extern Font *PopupMenuFont, *PopupMenuBoldFont, *PopupMenuItalicFont;
// Bounded API fixture, not a witness of the application's complete save layout.
// Keep the field access surface used by the installed source; the legacy test
// names below are references to that same fixture storage, not separate globals.
struct ConfigGameHostStateFields {
    uint32_t game_rules_, game_state_, game_more_preferences_;
    int diff_level_current_, vehicle_current_count_, time_controls_selection_;
    struct LockMaskBytes { unsigned char human_mask_, alive_mask_, unassigned_[2]; };
    union LockMasks { uint32_t enable_mask_; LockMaskBytes bytes_; } lock_masks_;
};
struct ConfigGameHostStateWindow { ConfigGameHostStateFields fields_; };
extern ConfigGameHostStateWindow GameDataStateStorage;
extern uint32_t &GameRules, &GameState, &GameMorePreferences, &LockEnableMask;
extern int &VehCurrentCount;
extern uint32_t MapRandSeed;
extern int IsMultiplayerPBEM, IsMultiplayerNet, ExpansionEnabled;
extern int &DiffLevelCurrent, &TimeControlsSelection;
extern int PopupWriteCheckState, ConsoleGroupAction;
extern int PopupModalActive, MapSetupStatus;
extern int GenderDefault, PluralityDefault;
extern int TamperState93A9AC, TamperState93A9B0, TamperState93A9B4;
char *label_get(int);
unsigned char factions_status_byte(int);
int bit_count(int);
int default_rules();
int x_pop_short(const char *, int, int (*)());
int X_pop(const char *, int (*)());
int X_pop_ask(const char *, int, char *, int (*)(), int);
int text_open(const char *, const char *);
char *text_get();
char *text_item();
int X_text_open(char *, const char *);
int parse_says(int, char *, int, int);
int parse_string(char *, char *);
void read_faction(Player *, int);
void load_faction_art(int);
void compute_faction_modifiers(int);
void load_flags();
void do_all_draws();
void time_controls_dialog(Popup *);
void prefs_save(int);
void log_set_state(int);
void world_build();
void setup_game(int);
void scenario_setup();
void alien_start();
void planetfall(int);
void clear_tamper();
unsigned long GetTickCount();
void GetSystemTime(SYSTEMTIME *);
#endif

#include <sstream>
#include <cctype>
#include <iostream>

std::vector<std::string> events;
void event(const std::string &name) { events.push_back(name); }
struct Reply {
    std::string label;
    int result, action, secondary;
    Reply(const char *l, int r, int a = 0, int s = 0) : label(l), result(r), action(a), secondary(s) {}
};
std::vector<Reply> replies;
size_t reply_index;
int last_action;
int answer(const std::string &name) {
    event(std::string("answer:") + name);
    assert(reply_index < replies.size());
    Reply r = replies[reply_index++];
    if (r.label != name) { std::cerr << "expected " << r.label << ", actual " << name << '\n'; assert(r.label == name); }
    last_action = r.action;
    ConsoleGroupAction = r.secondary;
    if (r.result == -777) throw std::runtime_error("controlled menu failure");
    return r.result;
}
int action_value() { return last_action; }
std::vector<int> randoms;
size_t random_index;
int config_rand() { assert(random_index < randoms.size()); return randoms[random_index++]; }
int _strnicmp(const char *a, const char *b, size_t n) {
    for (size_t i = 0; i < n; ++i) { int v = std::tolower((unsigned char)a[i]) - std::tolower((unsigned char)b[i]); if (v || !a[i]) return v; }
    return 0;
}
Player Players[8]; FactionArt FactionsArt[8]; Landmark MapLandmark[64];
ParseString ParseStrBuffer[10]; AlphaIniPref AlphaIniPrefs; DefaultPref DefaultPrefs;
ConsoleFixture ConsoleGlobal; PickWin PickWinGlobal; Wave g_WAVE_GENERAL; Strings StringTable;
LPSTR Difficulty[6]; char StringTemp[4096], PopupStartCaption[256];
char text_line[256]; char *TextBufferGetPtr = text_line;
const char *AlphaxFileID = "ALPHAX", *ScriptTxtID = "SCRIPT";
Font *PopupMenuFont, *PopupMenuBoldFont, *PopupMenuItalicFont;
ConfigGameHostStateWindow GameDataStateStorage;
uint32_t &GameRules = GameDataStateStorage.fields_.game_rules_;
uint32_t &GameState = GameDataStateStorage.fields_.game_state_;
uint32_t &GameMorePreferences = GameDataStateStorage.fields_.game_more_preferences_;
uint32_t &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
int &VehCurrentCount = GameDataStateStorage.fields_.vehicle_current_count_;
int &DiffLevelCurrent = GameDataStateStorage.fields_.diff_level_current_;
int &TimeControlsSelection = GameDataStateStorage.fields_.time_controls_selection_;
uint32_t MapRandSeed;
int IsMultiplayerPBEM, IsMultiplayerNet, ExpansionEnabled;
int PopupWriteCheckState, ConsoleGroupAction, PopupModalActive, MapSetupStatus;
int GenderDefault, PluralityDefault, TamperState93A9AC, TamperState93A9B0, TamperState93A9B4;
int text_row;
std::string text_section;
bool fail_multi;
int BasePop::exec(int, int (*)()) {
    if (this == &ConfigFactionPreview) { event("preview.exec"); return 0; }
    int r = answer(section);
    action = action_value();
    return r;
}
char *Strings::get(int) { static char text[] = "difficulty"; return text; }
char *Strings::get(const char *) { static char text[] = "difficulty"; return text; }
char *label_get(int i) {
    static char labels[1024][32];
    if (i == 0xB88 / 4) { strcpy(labels[i], "Male"); return labels[i]; }
    if (i == 0xB8C / 4) { strcpy(labels[i], "Female"); return labels[i]; }
    std::ostringstream n; n << i;
    strcpy(labels[i], n.str().c_str());
    return labels[i];
}
unsigned char factions_status_byte(int n) { return (LockEnableMask >> (8 * n)) & 255; }
int bit_count(int n) { int count = 0; for (unsigned int v = n; v; v >>= 1) count += v & 1; return count; }
int default_rules() { event("default_rules"); return 0x42; }
int x_pop_short(const char *label, int, int (*)()) { int r = answer(label); if (r >= 0) PopupWriteCheckState = 0x12355; return r; }
int X_pop(const char *label, int (*)()) { return answer(label); }
int X_pop_ask(const char *label, int, char *initial, int (*)(), int) {
    event(std::string("ask.initial:") + initial);
    int r = answer(label);
    strcpy(ParseStrBuffer[0].str, "new leader");
    return r;
}
int text_open(const char *file, const char *section) {
    assert(std::string(file) == "ALPHAX");
    event(std::string("text.open:") + section);
    text_section = section; text_row = -1; return 0;
}
char *text_get() { ++text_row; return text_line; }
char *text_item() {
    if (text_section == "CUSTOMFACTIONS") { text_line[0] = 0; return text_line; }
    std::ostringstream s; s << "random_" << text_section << text_row;
    strcpy(text_line, s.str().c_str()); return text_line;
}
int X_text_open(char *file, const char *section) {
    assert(std::string(file) == "SCRIPT"); assert(std::string(section) == "MULTISTRING");
    event("multi.open"); return fail_multi;
}
int parse_says(int, char *, int, int) { event("parse.says"); return 0; }
int parse_string(char *, char *out) { strcpy(out, "multi row"); event("parse.string"); return 0; }
void read_faction(Player *, int) { event("faction.read"); }
void load_faction_art(int) { event("faction.art"); }
void compute_faction_modifiers(int) { event("faction.modifiers"); }
void load_flags() { event("load_flags"); }
void do_all_draws() { event("draws"); }
void time_controls_dialog(Popup *pop) { pop->section = "TIMECONTROLS"; event("time.dialog"); }
void prefs_save(int n) { event(n ? "prefs.1" : "prefs.0"); }
void log_set_state(int n) { event(n ? "log.1" : "log.0"); }
void world_build() { event("world_build"); }
void setup_game(int) { event("setup_game"); }
void scenario_setup() { event("scenario_setup"); }
void alien_start() { event("alien_start"); }
void planetfall(int) { event("planetfall"); }
void clear_tamper() { event("tamper.clear"); }
unsigned long GetTickCount() { return 123; }
void GetSystemTime(SYSTEMTIME *t) { t->wYear = 2026; t->wMonth = 10; t->wDay = 6; }

// @@ACTUAL_CONFIG_GAME@@

bool seen(const std::string &s) {
    for (size_t i = 0; i < events.size(); ++i) if (events[i] == s) return true;
    return false;
}
int occurrences(const std::string &s) {
    int count = 0; for (size_t i = 0; i < events.size(); ++i) if (events[i] == s) ++count; return count;
}
void reset() {
    events.clear(); replies.clear(); reply_index = 0; randoms.clear(); random_index = 0;
    memset(Players, 0, sizeof Players); memset(ParseStrBuffer, 0, sizeof ParseStrBuffer);
    for (int i = 0; i < 8; ++i) {
        std::ostringstream n; n << "faction" << i;
        strcpy(Players[i].filename, n.str().c_str());
        strcpy(Players[i].name_leader, "leader");
        strcpy(Players[i].formal_name_faction, "formal");
        strcpy(Players[i].desc_name_faction, "description");
        strcpy(Players[i].noun_faction, "noun");
        strcpy(Players[i].adj_name_faction, "adj faction");
        strcpy(Players[i].title_leader, "title");
        strcpy(Players[i].adj_leader, "adj leader");
    }
    Players[1].rule_flags = 0x80; Players[2].rule_flags = 0x80;
    GameRules = 0; GameState = 0; GameMorePreferences = 0;
    AlphaIniPrefs.rules = 0; AlphaIniPrefs.semaphore = 0; AlphaIniPrefs.customize = 0;
    AlphaIniPrefs.time_controls = 0; DefaultPrefs.difficulty = 3; DefaultPrefs.faction_id = 2;
    IsMultiplayerNet = 0; IsMultiplayerPBEM = 0; ExpansionEnabled = 1;
    ConsoleGlobal.faction_id_ = 2; MapSetupStatus = 1; LockEnableMask = 0xDEADFF00u;
    VehCurrentCount = 10; MapRandSeed = 10; ConfigFactionDetailMode = 0;
    ConsoleGroupAction = 0; PopupModalActive = 0; fail_multi = false;
}
void done() {
    assert(reply_index == replies.size());
    assert(events[events.size() - 2] == "setup.dtor");
    assert(events.back() == "popup.dtor");
}
void menu_start(int rules = 0) {
    replies.push_back(Reply("DIFFICULTY", 3)); replies.push_back(Reply("USERULES", rules));
}
int main() {
    reset(); replies.push_back(Reply("DIFFICULTY", -1));
    assert(config_game(0) == 1); done(); assert(!seen("prefs.0"));

    reset(); menu_start(); replies.push_back(Reply("PICKER", 2)); replies.push_back(Reply("NAME", 0));
    assert(config_game(0) == 0); done();
    assert(DefaultPrefs.faction_id == 2 && ConsoleGlobal.faction_id_ == 2);
    assert((LockEnableMask & 255) == 4 && (LockEnableMask & 0xFFFFFF00u) == 0xDEADFF00u);
    assert(seen("prefs.1") && seen("prefs.0") && seen("world_build") && seen("setup_game") && seen("planetfall"));
    assert(GameRules == 0x142); // selected alien rule byte clears200 and adds140

    reset(); IsMultiplayerNet = 1; GameMorePreferences = 0x4000;
    menu_start(1); replies.push_back(Reply("PICKER", 3)); replies.push_back(Reply("NAME", 0));
    replies.push_back(Reply("TIMECONTROLS", 5));
    assert(config_game(0) == 0); done();
    assert(seen("wave.load:voices\\faction3.mp3") && seen("wave.play") && seen("wave.fade"));
    assert(AlphaIniPrefs.time_controls == 5 && TimeControlsSelection == 5 && seen("log.1") && !seen("planetfall"));

    reset(); ExpansionEnabled = 0; GameMorePreferences = 0x4000;
    menu_start(); replies.push_back(Reply("FACTIONX", 3)); replies.push_back(Reply("NAME", 0));
    assert(config_game(0) == 0); done(); assert(seen("wave.flagload:voices\\faction3.mp3"));

    reset(); ExpansionEnabled = 0; LockEnableMask = 0xDEAD0200u;
    replies.push_back(Reply("DIFFICULTY", 3));
    replies.push_back(Reply("FACTIONX", 1)); replies.push_back(Reply("NAME", 0));
    assert(config_game(2) == 0); done(); assert(seen("scenario_setup") && !seen("setup_game"));
    assert(occurrences("popup.row:formal|(description)") == 1);

    reset(); IsMultiplayerPBEM = 1;
    menu_start(); replies.push_back(Reply("PICKER", 3)); replies.push_back(Reply("NAME", 0));
    replies.push_back(Reply("HOTSEATADD1", 0));
    assert(config_game(0) == 0); done(); assert((LockEnableMask & 255) == 8);
    assert((LockEnableMask & 0xFFFFFF00u) == 0xDEADFF00u);
    assert(seen("multi.open") && seen("parse.string") && seen("tamper.clear"));
    assert(TamperState93A9AC == 133 && TamperState93A9B4 == 0);

    reset(); menu_start(2); replies.push_back(Reply("RULES", 0));
    replies.push_back(Reply("PICKER", 4)); replies.push_back(Reply("NAME", 0));
    assert(config_game(0) == 0); done(); assert((GameRules & 0xFFFF) == 0xDB18);
    assert((GameState & 0x01800000u) == 0x00800000u);

    reset(); ExpansionEnabled = 0; menu_start();
    replies.push_back(Reply("FACTIONX", 2, 1)); replies.push_back(Reply("CUSTOMNAMES", 0));
    for (int i = 0; i < 6; ++i) { std::ostringstream t; t << "copy" << i; strcpy(ParseStrBuffer[i].str, t.str().c_str()); }
    assert(config_game(0) == 0); done();
    assert(std::string(Players[2].name_leader) == "copy0" && std::string(Players[2].noun_faction) == "copy2");
    assert(std::string(Players[2].title_leader) == "copy4" && std::string(Players[2].adj_leader) == "copy5");
    assert(seen("edit:176=noun") && seen("edit:483=title"));
    assert(Players[2].is_leader_female == 0); // validator rereads initial buffer, not parser row6

    reset(); ExpansionEnabled = 0; menu_start();
    replies.push_back(Reply("FACTIONX", 2, 3)); replies.push_back(Reply("RELOADFAC", 0));
    replies.push_back(Reply("FACTIONX", 2)); replies.push_back(Reply("NAME", 0));
    assert(config_game(0) == 0); done(); assert(seen("faction.modifiers"));
    assert(std::string(Players[2].filename) == "new leader" && std::string(Players[2].search_key) == "new leader");

    reset(); DefaultPrefs.faction_id = 4; assert(config_game(1) == 0); done();
    assert(ConsoleGlobal.faction_id_ == 4 && !seen("draws") && !seen("prefs.1"));

    reset(); GameRules = 0xFFFF1234u; GameState = 0x01800000u;
    config_game_seed_checks(); unsigned int saved = GameRules;
    GameRules &= 0xFFFF0000u; GameState &= 0xFE7FFFFFu; config_game_apply_checks();
    assert(GameRules == saved && (GameState & 0x01800000u) == 0x01800000u);

    reset(); replies.push_back(Reply("DIFFICULTY", -777));
    try { config_game(0); assert(false); } catch (const std::runtime_error &) {}
    done(); assert(!seen("prefs.0"));

    reset(); menu_start(); replies.push_back(Reply("PICKER", 3, 0x107));
    replies.push_back(Reply("PICKER", 4)); replies.push_back(Reply("NAME", 0));
    assert(config_game(0) == 0); done(); assert(ConfigFactionDetailMode == 1);
    assert(seen("preview.start:DATALINKS1") && seen("preview.start:DATALINKS2"));

    reset(); menu_start(); replies.push_back(Reply("PICKER", 2, 0x105));
    randoms.push_back(3); replies.push_back(Reply("NAME3", 0));
    assert(config_game(0) == 0); done(); assert(ConsoleGlobal.faction_id_ == 4);
    assert(seen("ask.initial:") && random_index == 1);

    reset(); IsMultiplayerPBEM = 1; fail_multi = true;
    menu_start(); replies.push_back(Reply("PICKER", 3)); replies.push_back(Reply("NAME", 0));
    replies.push_back(Reply("HOTSEATADD1", 1));
    replies.push_back(Reply("PICKER", 4)); replies.push_back(Reply("NAME", 0));
    replies.push_back(Reply("HOTSEATADD1", 0));
    assert(config_game(0) == 0); done(); assert((LockEnableMask & 255) == 24);
    assert(occurrences("multi.open") == 2 && !seen("parse.says")); // fail breaks row walk
    assert(ConsoleGlobal.faction_id_ == 3);

    reset(); IsMultiplayerPBEM = 1;
    menu_start(); replies.push_back(Reply("PICKER", 3)); replies.push_back(Reply("NAME", 0));
    replies.push_back(Reply("HOTSEATADD1", -1));
    replies.push_back(Reply("PICKER", 4)); replies.push_back(Reply("NAME", 0));
    replies.push_back(Reply("HOTSEATADD1", 0));
    assert(config_game(0) == 0); done(); assert((LockEnableMask & 255) == 16);

    reset(); DefaultPrefs.faction_id = 4; strcpy(Players[3].filename, "JENN282");
    randoms.push_back(13); randoms.push_back(7);
    assert(config_game(1) == 0); done(); assert(random_index == 2);
    assert(std::string(Players[3].filename) == "random_NEWFACTIONS0");
    assert(std::string(Players[3].search_key) == "random_NEWFACTIONS0");
    assert(occurrences("text.open:NEWFACTIONS") == 1 && !seen("text.open:FACTIONS"));

    reset(); DefaultPrefs.faction_id = 4; Players[1].rule_flags = 0; Players[2].rule_flags = 0;
    strcpy(Players[1].filename, "JENN282"); strcpy(Players[2].filename, "JENN282");
    randoms.push_back(1); randoms.push_back(6);
    assert(config_game(1) == 0); done(); assert(random_index == 2);
    assert(std::string(Players[1].filename) == "random_NEWFACTIONS5");
    assert(std::string(Players[2].filename) == "random_NEWFACTIONS6");

    reset(); DefaultPrefs.faction_id = 4; Players[2].rule_flags = 0;
    strcpy(Players[2].filename, "JENN282"); strcpy(Players[3].filename, "JENN282");
    randoms.push_back(1);
    assert(config_game(1) == 0); done(); assert(random_index == 1);
    assert(std::string(Players[3].filename) == "random_NEWFACTIONS5");
    assert(std::string(Players[2].filename) == "random_FACTIONS1");

    reset(); ExpansionEnabled = 0; menu_start(); replies.push_back(Reply("FACTIONX", 2, 1));
    replies.push_back(Reply("CUSTOMNAMES", 0, 1)); replies.push_back(Reply("CUSTOMNAMES", 0));
    assert(config_game(0) == 0); done(); assert(occurrences("faction.read") == 1);
    assert(occurrences("popup.start:CUSTOMNAMES") == 2);
    return 0;
}
