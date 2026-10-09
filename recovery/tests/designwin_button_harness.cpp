// Bounded host control: pytest extracts the actual mapped product body.
// The narrow owner projections below record calls, not game runtime behavior.
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifndef __cdecl
#define __cdecl
#endif

struct VehPrototype {
    char veh_name[32];
    uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
    uint8_t carry_capacity, cost, plan;
    int8_t unk_1;
    uint8_t obsolete_factions, combat_factions;
    int8_t icon_offset, padding;
    uint16_t flags;
    int16_t preq_tech;
};
struct RulesWeapon { int8_t offense_rating; };
struct RulesArmor { int8_t defense_rating; };
struct Player { uint32_t rule_flags; };
struct PlayerData { uint8_t proto_id_active[512]; };

static const int MaxPlayerNum = 8;
static const int MaxVehProtoFactionNum = 64;
static const int MaxAbilityNum = 29;
static VehPrototype VehPrototypes[512];
static RulesWeapon Weapon[26];
static RulesArmor Armor[14];
static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
static int ProtoSortCount;
static int ProtoSortIds[128];
struct GameDataStateFields { uint32_t game_state_; };
struct GameDataStateWindow { GameDataStateFields fields_; };
static GameDataStateWindow GameDataStateStorage;
static int IsMultiplayerNet;
static uint32_t UpgradePromptAnswered;
static char StringTemp[1032];
static char PopupStartCaption[256];
static char *ScriptPopupFile = const_cast<char *>("SCRIPT");
static uint32_t DesignConsiderAbilities;
static int DesignConsiderPrototype;
static int DesignConsiderWeapon;
static int DesignConsiderChassis;
static int DesignConsiderReactor;
static int DesignConsiderArmor;
static int DesignConsiderOverride;
static int popup_answer;
static int popup_selection;
static std::vector<std::string> events;
static std::vector<int> item_ids;
static std::string last_popup_label;
static std::string last_popup_picture;

static void event(const char *name) { events.push_back(name); }

struct TutWin { void reset() { event("reset"); } };
static TutWin tut_win;
static TutWin *TutWinGlobal = &tut_win;

struct FX {
    void fade(int value) { assert(value == 0x18); event("fade"); }
    void play(int) { event("play"); }
};
static FX g_FX;

struct NetDaemon {
    int lock(int a, int b, int c, int d, int e, int f) {
        assert(a == 2 && b == -1 && c == -1 && d == 0 && e == -1 && f == -1);
        event("net-lock");
        return 0;
    }
    void unlock() { event("net-unlock"); }
    void await_exec(int value) { assert(value == 0); event("await"); }
};
static NetDaemon NetDaemonNet;

struct Popup {
    int start(char *, const char *label, int, char *, int, void *) {
        assert(std::strcmp(label, "WHICHUPGRADE2") == 0);
        event("popup-start");
        return 1;
    }
    int item(char *, int id) { item_ids.push_back(id); return 1; }
    int exec(int, int (__cdecl *callback)()) {
        (void)callback;
        event("popup-exec");
        return popup_selection;
    }
};

class DesignWin {
public:
    uint32_t field_CDC_;
    uint32_t field_CE4_;
    uint32_t factionID_;
    uint32_t protoID_;
    uint32_t protoChassisType_;
    uint32_t protoWeaponType_;
    uint32_t protoArmorType_;
    uint32_t protoAbilityFlags_;
    uint32_t protoReactorType_;
    uint32_t protoAbilityID1_;
    uint32_t protoAbilityID2_;
    DesignWin() : field_CDC_(0), field_CE4_(0), factionID_(0), protoID_(0),
        protoChassisType_(0), protoWeaponType_(0), protoArmorType_(0),
        protoAbilityFlags_(0), protoReactorType_(0), protoAbilityID1_(0),
        protoAbilityID2_(0) {}
    int design_it() { event("design-it"); return 1; }
    void setup_veh(int, int) { event("setup-veh"); }
    void release_modal() { event("release"); }
    void redraw() { event("redraw"); }
    void draw_stack() { event("draw-stack"); }
    void select_name() { event("select-name"); }
    void on_redraw() { event("on-redraw"); }
    void on_update() { event("on-update"); }
    void on_button_clicked(int button_id);
};

static int __cdecl pop_wait() { event("pop-wait"); return 0; }
static int __cdecl popp(const char *, const char *label, int, const char *picture,
                        int (__cdecl *callback)()) {
    (void)callback;
    last_popup_label = label;
    last_popup_picture = picture;
    event("popp");
    return popup_answer;
}
static int parse_says(int, const char *, int, int) { event("parse-says"); return 0; }
static int parse_num(int, int) { event("parse-num"); return 0; }
static void proto_sort_2(int) { event("proto-sort"); }
static void retire_proto(int, int) { event("retire"); }
void __cdecl clear_bunglist(int) { event("clear-bunglist"); }
static uint32_t message_data(int, int, int, int, int, int) {
    event("message-data"); return 0;
}
static void synch_obs(int) { event("synch-obs"); }
static void say_stats_3(char *, int) { event("say-stats"); }
static int upgrade_prototype(int, int, int, int) { event("upgrade"); return 1; }
static void consider_designs(int) { event("consider"); }

int __cdecl diplo_lock(int patience);
void __cdecl diplo_unlock();
/* EXTRACTED_DESIGNWIN_BUTTON */

int __cdecl diplo_lock(int) { event("diplo-lock"); return 0; }
void __cdecl diplo_unlock() { event("diplo-unlock"); }

static void reset_state() {
    std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
    std::memset(Weapon, 0, sizeof(Weapon));
    std::memset(Armor, 0, sizeof(Armor));
    std::memset(Players, 0, sizeof(Players));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(ProtoSortIds, 0, sizeof(ProtoSortIds));
    std::memset(StringTemp, 0, sizeof(StringTemp));
    events.clear(); item_ids.clear(); last_popup_label.clear();
    last_popup_picture.clear();
    ProtoSortCount = 1; ProtoSortIds[0] = 0;
    GameDataStateStorage.fields_.game_state_ = 0x40; IsMultiplayerNet = 0;
    popup_answer = 0; popup_selection = 1;
    DesignConsiderOverride = 0;
}

static int event_index(const char *name) {
    for (size_t i = 0; i < events.size(); ++i)
        if (events[i] == name) return static_cast<int>(i);
    return -1;
}

static int test_ordering() {
    reset_state();
    DesignWin window;
    window.on_button_clicked(-2);
    assert(events.size() == 3);
    assert(events[0] == "reset" && events[1] == "release" && events[2] == "fade");
    reset_state();
    window.on_button_clicked(8);
    assert(events.size() == 3);
    assert(events[0] == "reset" && events[1] == "release" && events[2] == "fade");
    return 0;
}

static int test_retire_prompt() {
    reset_state();
    DesignWin window;
    window.factionID_ = 2; window.protoID_ = 3;
    Players[2].rule_flags = 0x80;
    PlayersData[2].proto_id_active[3] = 5;
    popup_answer = 0;
    window.on_button_clicked(0);
    assert(last_popup_label == "RETIRETYPE");
    assert(last_popup_picture == "alunit_sm.pcx");
    assert(event_index("parse-num") >= 0 && event_index("retire") < 0);

    reset_state();
    window.factionID_ = 2; window.protoID_ = 3;
    Players[2].rule_flags = 0;
    PlayersData[2].proto_id_active[3] = 0;
    popup_answer = 1;
    ProtoSortCount = 8; ProtoSortIds[0] = 8; ProtoSortIds[6] = 3;
    window.field_CE4_ = 0;
    window.on_button_clicked(0);
    assert(last_popup_label == "RETIRETYPE2");
    assert(last_popup_picture == "unit_sm.pcx");
    assert(event_index("retire") >= 0 && event_index("setup-veh") >= 0);
    assert(window.field_CE4_ == 1);
    return 0;
}

static int test_retire_all_variant() {
    reset_state();
    DesignWin window;
    window.factionID_ = 3;
    Players[3].rule_flags = 0x80;
    popup_answer = 0;
    window.on_button_clicked(3);
    assert(last_popup_label == "RETIREALL");
    assert(last_popup_picture == "alunit_sm.pcx");
    assert(event_index("clear-bunglist") < 0);

    reset_state();
    window.factionID_ = 3;
    Players[3].rule_flags = 0;
    popup_answer = 0;
    window.on_button_clicked(3);
    assert(last_popup_label == "RETIREALL");
    assert(last_popup_picture == "unit_sm.pcx");
    reset_state();
    window.factionID_ = 3;
    GameDataStateStorage.fields_.game_state_ = 0x40;
    popup_answer = 2;
    for (int i = 0; i < 512; ++i) {
        VehPrototypes[i].obsolete_factions = 0xFF;
        VehPrototypes[i].flags = 1;
    }
    window.on_button_clicked(3);
    for (int slot = 0; slot < 64; ++slot) {
        assert(VehPrototypes[slot].obsolete_factions == 1);
        assert(VehPrototypes[slot].flags == 1);
    }
    for (int i = 64; i < 512; ++i) {
        assert(VehPrototypes[i].flags == 0);
        assert(VehPrototypes[i].obsolete_factions == 0xFF);
    }
    return 0;
}

static int test_upgrade_filter_and_variant() {
    reset_state();
    DesignWin window;
    window.factionID_ = 0; window.protoID_ = 0;
    VehPrototype &old = VehPrototypes[0];
    old.chassis_id = 2; old.weapon_id = 2; old.armor_id = 1;
    old.plan = 5; old.flags = 1;
    Weapon[2].offense_rating = 4;
    Armor[1].defense_rating = 3;
    Armor[0].defense_rating = 1;

    VehPrototype &valid = VehPrototypes[1];
    valid.chassis_id = 2; valid.weapon_id = 3; valid.armor_id = 2;
    valid.plan = 5; valid.flags = 1;
    Weapon[3].offense_rating = 5;
    Armor[2].defense_rating = 2;

    VehPrototype &bad_plan = VehPrototypes[2];
    bad_plan.chassis_id = 2; bad_plan.weapon_id = 3; bad_plan.armor_id = 2;
    bad_plan.plan = 3; bad_plan.flags = 1;

    VehPrototype &bad_weapon = VehPrototypes[3];
    bad_weapon.chassis_id = 2; bad_weapon.weapon_id = 4; bad_weapon.armor_id = 2;
    bad_weapon.plan = 5; bad_weapon.flags = 1;
    Weapon[4].offense_rating = 3;

    VehPrototype &bad_armor = VehPrototypes[4];
    bad_armor.chassis_id = 2; bad_armor.weapon_id = 3; bad_armor.armor_id = 0;
    bad_armor.plan = 5; bad_armor.flags = 1;

    VehPrototype &strong_armor = VehPrototypes[5];
    strong_armor.chassis_id = 2; strong_armor.weapon_id = 2;
    strong_armor.armor_id = 3; strong_armor.plan = 5; strong_armor.flags = 1;
    Armor[3].defense_rating = 4;

    PlayersData[0].proto_id_active[1] = 1;
    popup_selection = 1;
    window.on_button_clicked(7);
    assert(item_ids.size() == 2 && item_ids[0] == 1 && item_ids[1] == 5);
    assert(event_index("net-lock") >= 0 && event_index("net-unlock") >= 0);
    assert(event_index("upgrade") >= 0);
    assert(window.protoID_ == 1);
    assert(last_popup_label == "YESUPGRADE");
    assert(last_popup_picture == "unit_sm.pcx");
    return 0;
}

static int test_upgrade_alien_variant() {
    reset_state();
    DesignWin window;
    window.factionID_ = 1; window.protoID_ = 64;
    Players[1].rule_flags = 0x80;
    VehPrototypes[64].veh_name[0] = 'O';
    VehPrototypes[64].veh_name[1] = '\0';
    VehPrototypes[65].chassis_id = VehPrototypes[64].chassis_id;
    VehPrototypes[65].weapon_id = VehPrototypes[64].weapon_id;
    VehPrototypes[65].armor_id = VehPrototypes[64].armor_id;
    VehPrototypes[65].plan = VehPrototypes[64].plan;
    VehPrototypes[65].flags = 1;
    PlayersData[1].proto_id_active[65] = 1;
    popup_selection = 65;
    window.on_button_clicked(7);
    assert(last_popup_label == "YESUPGRADE");
    assert(last_popup_picture == "alunit_sm.pcx");
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    if (std::strcmp(argv[1], "ordering") == 0) return test_ordering();
    if (std::strcmp(argv[1], "retire") == 0) return test_retire_prompt();
    if (std::strcmp(argv[1], "retire-all") == 0) return test_retire_all_variant();
    if (std::strcmp(argv[1], "upgrade-filter") == 0) return test_upgrade_filter_and_variant();
    if (std::strcmp(argv[1], "upgrade-alien") == 0) return test_upgrade_alien_variant();
    return 2;
}
