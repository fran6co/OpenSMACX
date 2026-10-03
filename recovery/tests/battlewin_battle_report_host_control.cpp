#include "battlewin_battle_report_host_types.h"

static int count_prefix(const char *prefix) {
    int count = 0;
    const size_t length = strlen(prefix);
    for (size_t i = 0; i < test_events.values.size(); ++i) {
        if (test_events.values[i].compare(0, length, prefix) == 0) ++count;
    }
    return count;
}

static bool has_exact(const char *value) {
    for (size_t i = 0; i < test_events.values.size(); ++i) {
        if (test_events.values[i] == value) return true;
    }
    return false;
}

static bool has_prefix(const char *value) {
    return count_prefix(value) != 0;
}

static bool contains_text(const char *value) {
    for (size_t i = 0; i < test_events.values.size(); ++i) {
        if (test_events.values[i].find(value) != std::string::npos) return true;
    }
    return false;
}

static size_t event_index(const char *value) {
    for (size_t i = 0; i < test_events.values.size(); ++i) {
        if (test_events.values[i] == value) return i;
    }
    return test_events.values.size();
}

static bool has_exact(const char *value);

static void reset_test_state() {
    test_events.values.clear();
    font_ctor_count = 0;
    font_dtor_count = 0;
    stats_call_count = 0;
    morale_call_count = 0;
    memset(morale_faction_args, 0, sizeof(morale_faction_args));
    union_rect_calls = 0;
    memset(IfaceIdToSlotStorage, 0, sizeof(IfaceIdToSlotStorage));
    memset(Vehs, 0, sizeof(Vehs));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Weapon, 0, sizeof(Weapon));
    memset(Armor, 0, sizeof(Armor));
    memset(Players, 0, sizeof(Players));
    memset(VehBattleModCount, 0, sizeof(VehBattleModCount));
    memset(VehBasicBattleMorale, 0, sizeof(VehBasicBattleMorale));
    memset(StringTemp, 0, sizeof(StringTemp));
    ActiveIfaceIdStorage = 0;
    WinScreenWidth = 800;
    GameState = 0;
    Language = 0;
    ConsoleGlobal.faction_id_ = 0;
    MessageName = "table";
    MapWinCaptionScale = 3;
    MapWinCaptionDivisor = 4;
    for (int i = 0; i != 8; ++i) {
        ChatItemIdTable[i] = 20 + i;
        ChatShadowColors[i] = 40 + i;
        strcpy(Players[i].formal_name_faction, "Faction");
        strcpy(Players[i].adj_name_faction, "FactionAdj");
        strcpy(Players[i].noun_faction, "FactionNoun");
        Players[i].noun_gender = i % 3;
        Players[i].is_noun_plural = i & 1;
    }
    for (int i = 0; i != 4; ++i) {
        Weapon[i].name = reinterpret_cast<char *>(static_cast<size_t>(101));
        Armor[i].name = reinterpret_cast<char *>(static_cast<size_t>(202));
        VehPrototypes[i].veh_name[0] = 'U';
        strcpy(VehPrototypes[i].veh_name, "Unit");
        VehPrototypes[i].reactor_id = 1;
    }
    g_BATTLE_ISLE_DEEP_SPRITE.tag = "isle";
    g_BATTLE_LOCUSTS_CHIRON_SPRITE.tag = "locusts";
    g_BATTLE_SEALURK_SPRITE.tag = "sealurk";
    g_BATTLE_SPORE_LAUNCHER_SPRITE.tag = "spore";
    g_BATTLE_FUNGAL_TOWER_SPRITE.tag = "fungal";
    FactionsArt[1].council_small.tag = "faction1";
}

static void init_window(BattleWin &win) {
    win.battle_rects_[0].left = 10; win.battle_rects_[0].top = 20;
    win.battle_rects_[0].right = 210; win.battle_rects_[0].bottom = 220;
    win.battle_rects_[1].left = 220; win.battle_rects_[1].top = 20;
    win.battle_rects_[1].right = 420; win.battle_rects_[1].bottom = 220;
    win.battle_rects_[2].left = 10; win.battle_rects_[2].top = 230;
    win.battle_rects_[2].right = 210; win.battle_rects_[2].bottom = 430;
    win.battle_rects_[3].left = 220; win.battle_rects_[3].top = 230;
    win.battle_rects_[3].right = 420; win.battle_rects_[3].bottom = 430;
}

int main() {
    // Mode mismatch is the original early return after all three Font
    // constructors; it must not paint, update the timer, or skip destructors.
    reset_test_state();
    BattleWin inactive;
    init_window(inactive);
    IfaceIdToSlotStorage[0] = 5;
    inactive.battle_report(1, 2, -1, -1, 0);
    if (font_ctor_count != 3 || font_dtor_count != 3 || count_prefix("box") ||
        has_exact("time_pulse")) return 11;

    // Equal IDs use one report pass, keep the 800-pixel sizing and still draw
    // both portrait panels, then restore clip/scale and schedule the callback.
    reset_test_state();
    BattleWin equal;
    init_window(equal);
    IfaceIdToSlotStorage[0] = 10;
    Vehs[1].proto_id = 0;
    Vehs[1].faction_id = 0;
    strcpy(VehPrototypes[0].veh_name, "Mind Worm");
    equal.battle_report(1, 1, -1, -1, 0);
    if (count_prefix("box") != 4 || count_prefix("sprite_draw") != 2 ||
        count_prefix("veh_draw:") != 0 || stats_call_count != 1 ||
        morale_call_count != 1 ||
        morale_faction_args[0] != 0 ||
        !has_exact("sprite_draw:mindworm:0:104:324") ||
        !has_exact("sprite_draw:mindworm:0:314:324") ||
        !has_exact("wraprect:13:337:207:427:Faction") ||
        count_prefix("time_pulse") != 1 || !has_exact("restore_clip") ||
        !has_exact("update") || !has_exact("do_non_input") ||
        MapWinCaptionScale != -1 || MapWinCaptionDivisor != -1) return 12;

    // Distinct vehicles on a non-800 layout take both loop iterations and
    // both expanded vehicle draws. Negative offense with nonnegative sentinels
    // selects the alternate combat-label path through the real candidate body.
    reset_test_state();
    BattleWin distinct;
    init_window(distinct);
    IfaceIdToSlotStorage[0] = 10;
    WinScreenWidth = 1024;
    Vehs[1].proto_id = 0;
    Vehs[1].faction_id = 1;
    Vehs[2].proto_id = 1;
    Vehs[2].faction_id = 2;
    Weapon[0].offense_rating = -1;
    Weapon[1].offense_rating = 1;
    Armor[0].defense_rating = 1;
    Armor[1].defense_rating = 1;
    VehPrototypes[0].chassis_id = 0;
    VehPrototypes[1].chassis_id = 1;
    Chassis[0].triad = 0;
    Chassis[1].triad = 2;
    Rules.psi_combat_ratio_atk[2] = 77;
    Rules.psi_combat_ratio_def[2] = 88;
    strcpy(VehPrototypes[0].veh_name, "First");
    strcpy(VehPrototypes[1].veh_name, "Second");
    distinct.battle_report(1, 2, 0x00008000, 0x00004000, 0);
    if (count_prefix("box") != 4 || count_prefix("sprite_draw") != 2 ||
        count_prefix("veh_draw:") != 2 || stats_call_count != 2 ||
        morale_call_count != 2 ||
        morale_faction_args[0] != 2 || morale_faction_args[1] != 1 ||
        !has_exact("veh_draw:1:24:23:0") ||
        !has_exact("veh_draw:2:234:23:0") ||
        !contains_text("wrap:110:28:97:stats") ||
        !contains_text("wrap:320:28:97:stats") ||
        count_prefix("time_pulse") != 1 || !has_prefix("wrap:")) return 13;
    bool saw_alternate_label = false;
    for (size_t i = 0; i < test_events.values.size(); ++i) {
        if (test_events.values[i].find("L784") != std::string::npos)
            saw_alternate_label = true;
    }
    if (!saw_alternate_label || !contains_text("L784: 77") ||
        !contains_text("L784: 88")) return 14;

    // A regular ratings branch with both ratings nonnegative and a nonzero
    // first faction must use the typed weapon/armor name records, not label 784.
    reset_test_state();
    BattleWin regular;
    init_window(regular);
    IfaceIdToSlotStorage[0] = 10;
    Vehs[1].proto_id = 0;
    Vehs[1].faction_id = 1;
    Vehs[2].proto_id = 1;
    Vehs[2].faction_id = 2;
    Weapon[0].offense_rating = 1;
    Armor[1].defense_rating = 1;
    regular.battle_report(1, 2, 0x100, 0x200, 0);
    if (contains_text("L784") || !contains_text("WeaponLabel") ||
        !contains_text("ArmorLabel") ||
        !contains_text("wrap:13:0:97:stats")) return 15;
    if (count_prefix("time_pulse") != 1) return 16;

    // With identical IDs the native live-argument gate reaches the armor
    // label when flag bit 1 is clear and the prototype is not non-land.
    reset_test_state();
    BattleWin identical_standard;
    init_window(identical_standard);
    IfaceIdToSlotStorage[0] = 10;
    Vehs[1].proto_id = 0;
    Vehs[1].faction_id = 1;
    VehPrototypes[0].chassis_id = 1;
    Weapon[0].offense_rating = 1;
    Armor[0].defense_rating = 1;
    identical_standard.battle_report(1, 1, 0x100, 0x200, 0);
    if (!contains_text("ArmorLabel") || contains_text("WeaponLabel")) return 17;

    // The display strings are a packed byte region: side stride 0x140 and
    // per-modifier stride 0x50, not an array of LPSTR values.
    reset_test_state();
    BattleWin modifiers;
    init_window(modifiers);
    IfaceIdToSlotStorage[0] = 10;
    Vehs[1].proto_id = 0;
    Vehs[1].faction_id = 1;
    Vehs[2].proto_id = 1;
    Vehs[2].faction_id = 2;
    VehBattleModCount[0] = 1;
    VehBattleModCount[1] = 2;
    VehBattleModifier[0] = 3;
    VehBattleModifier[4] = -4;
    VehBattleModifier[5] = 5;
    strcpy(VehBattleDisplay[0], "FIRSTMOD");
    strcpy(VehBattleDisplay[4], "SECONDMOD");
    strcpy(VehBattleDisplay[5], "THIRDMOD");
    modifiers.battle_report(1, 2, -1, -1, 0);
    if (!contains_text("FIRSTMOD +3%") ||
        !contains_text("SECONDMOD -4%") ||
        !contains_text("THIRDMOD +5%")) return 18;

    // Wide expired-unit labeling uses local style-2 font and sits above the
    // report text. This path is sensitive to plan and lurking-age inputs.
    reset_test_state();
    BattleWin expired;
    init_window(expired);
    IfaceIdToSlotStorage[0] = 10;
    WinScreenWidth = 1024;
    Vehs[1].proto_id = 0;
    Vehs[1].faction_id = 1;
    Vehs[1].year_end_lurking = 1;
    VehPrototypes[0].plan = 0x0C;
    expired.battle_report(1, 1, -1, -1, 0);
    if (!contains_text("write:L1184") || !has_exact("font:12:2") ||
        !has_exact("set_font:2")) return 19;

    // Portrait case dispatch keys on prototype id, and a nonzero faction
    // replaces the chassis portrait with that faction's council image.
    reset_test_state();
    BattleWin portraits;
    init_window(portraits);
    IfaceIdToSlotStorage[0] = 10;
    Vehs[1].proto_id = 9;
    Vehs[1].faction_id = 0;
    Vehs[2].proto_id = 1;
    Vehs[2].faction_id = 1;
    VehPrototypes[9].chassis_id = 0;
    portraits.battle_report(1, 2, -1, -1, 0);
    if (!has_exact("sprite_draw:isle:0:104:324") ||
        !has_exact("sprite_draw:faction1:0:314:324")) return 20;
    const size_t font_before_portrait = event_index("set_font:-1");
    const size_t portrait_draw = event_index("sprite_draw:isle:0:104:324");
    if (font_before_portrait >= portrait_draw) return 22;

    reset_test_state();
    BattleWin special_proto_portraits;
    init_window(special_proto_portraits);
    IfaceIdToSlotStorage[0] = 10;
    Vehs[1].proto_id = 15;
    Vehs[1].faction_id = 0;
    Vehs[2].proto_id = 19;
    Vehs[2].faction_id = 0;
    VehPrototypes[15].chassis_id = 0;
    VehPrototypes[19].chassis_id = 0;
    special_proto_portraits.battle_report(1, 2, -1, -1, 0);
    if (!has_exact("sprite_draw:fungal:0:104:324") ||
        !has_exact("sprite_draw:spore:0:314:324")) return 23;

    reset_test_state();
    BattleWin localized_debug;
    init_window(localized_debug);
    IfaceIdToSlotStorage[0] = 10;
    Vehs[1].proto_id = 0;
    Vehs[1].faction_id = 1;
    Vehs[2].proto_id = 1;
    Vehs[2].faction_id = 2;
    Language = 1;
    GameState = 0x80;
    localized_debug.battle_report(1, 2, -1, -1, 0);
    if (!contains_text("Unit (FactionNoun)(1)")) return 21;

    return 0;
}
