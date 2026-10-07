#include <cassert>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
static const int MaxPlayerNum = 8;
static const int MaxRankingHistoryTurns = 1000;
static const int MaxTechnologyNum = 89;
static int *active_pane_cell;
static int switch_active_pane_once;
static int *console_faction_cell;
static int console_faction_new_value;
static int change_console_faction_once;
static int *pane_state_cell;
static int pane_state_new_value;
static int change_pane_state_once;

struct Font {
    int unk_1_;
    int height;
    int line_height_value;
    int line_height() const { return line_height_value; }
    int pixel_height() const { return height; }
};

struct Buffer {
    RECT clip;
    RECT work;
    std::vector<RECT> boxes;
    std::vector<RECT> writes;
    std::vector<int> box_first_colors, box_second_colors, fill_colors;
    std::vector<int> line_colors;
    std::vector<int> line_from_x, line_to_x;
    std::vector<int> line_from_y;
    std::vector<int> line_to_y;
    int frame_calls;
    Buffer() {
        clip.left = work.left = clip.top = work.top = 0;
        clip.right = work.right = 180;
        clip.bottom = work.bottom = 130;
        frame_calls = 0;
    }
    int set_clip(RECT *r) { clip = *r; return 1; }
    const RECT &clip_rect() const { return work; }
    int box_sprite(RECT *, int *) {
        ++frame_calls;
        if (switch_active_pane_once) {
            switch_active_pane_once = 0;
            *active_pane_cell = 1;
        }
        if (change_console_faction_once) {
            change_console_faction_once = 0;
            *console_faction_cell = console_faction_new_value;
        }
        if (change_pane_state_once) {
            change_pane_state_once = 0;
            *pane_state_cell = pane_state_new_value;
        }
        return 1;
    }
    int box(RECT *r, int first, int second) {
        boxes.push_back(*r); box_first_colors.push_back(first);
        box_second_colors.push_back(second); return 1;
    }
    int fill(RECT *, int color) { fill_colors.push_back(color); return 1; }
    int line(int x0, int y0, int x1, int y1, int color) {
        line_from_x.push_back(x0); line_to_x.push_back(x1);
        line_colors.push_back(color); line_from_y.push_back(y0);
        line_to_y.push_back(y1); return 1;
    }
    int text_width(char *s) { return static_cast<int>(strlen(s)) * 5; }
    int set_font(Font *, int, int, int) { return 1; }
    int set_text_color(int, int, int, int) { return 1; }
    int write_cent_l(char *, RECT *r, int) { writes.push_back(*r); return 1; }
};
struct MainInterface : public Buffer {};
static MainInterface MainInterfaceGlobal;
static int InterfaceBoxStyles[32];

struct Player {
    int noun_gender, is_noun_plural, is_leader_female;
    char formal_name_faction[40], title_leader[24], name_leader[24];
    char noun_faction[24], search_key[24];
};
struct PlayerData {
    int tech_id_researching, se_alloc_labs, se_alloc_psych;
    int tech_commerce_bonus, turn_commerce_income, integrity_blemishes;
    int ranking, tech_ranking, mil_strength_1, pop_total, energy_reserves;
    uint32_t diplo_treaties[8];
    uint16_t unk_78[128];
};
struct RuleItem { int name; };
struct EnergyItem { int abbrev; };
struct MightItem { int adj; };
struct NaturalItem { int name_short; };
struct GameDataLockMaskView { uint32_t enable_mask_; };
struct GameDataStateWindowFields {
    uint32_t game_state_, game_rules_;
    int turn_current_num_;
    GameDataLockMaskView lock_masks_;
    uint32_t faction_rankings_[8];
    uint32_t ranking_faction_id_unk1_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
struct ScenarioSaveState {
    int objective_req_victory;
    uint32_t objectives_sudden_death_victory;
    uint32_t objective_achieve_points;
};

static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
static GameDataStateWindow GameDataStateStorage;
static ScenarioSaveState ScenarioSave;
static RuleItem Technology[MaxTechnologyNum];
static EnergyItem Energy[3];
static int Repute[8];
static MightItem Might[8];
static int Mood[16];
static NaturalItem Natural[16];
static RuleItem Terraforming[64];
static uint16_t FactionRankingHistory[MaxPlayerNum * MaxRankingHistoryTurns];
static uint32_t FactionRankingsUnk[8];
static int InfoFactionTextColors[8], InfoFactionShadowColors[8];
static int ChatItemIdTable[8], ChatShadowColors[8];
static int MapWinCaptionScale, MapWinCaptionDivisor, Language;
static int EnergyEconomyTotal, NetIncomeGlobal, BreakthroughTurnsGlobal;
static int MapEditorPaintId, MapEditorBrushId, MapLongitudeBounds, MapLatitudeBounds;
static int MapIsFlat, MapCloudCover;
static char MapFilePath[80];
static uint32_t DiploFrictionFactionID;
static int DiploFriction;
static int GenderDefault, PluralityDefault;

struct Console { int faction_id_; };
static Console ConsoleGlobal;
struct Strings {
    char storage[32];
    char *get(int id) {
        sprintf(storage, "S%d", id);
        return storage;
    }
};
static Strings StringTable;

static char labels[2048][24];
static char *label_get(int index) {
    assert(index >= 0 && index < 2048);
    sprintf(labels[index], "L%d", index);
    return labels[index];
}
static char *_itoa(int value, char *buffer, int) {
    sprintf(buffer, "%d", value);
    return buffer;
}
static char *CharUpperA(char *s) {
    for (char *p = s; *p; ++p) *p = static_cast<char>(toupper(*p));
    return s;
}

static int last_energy_faction = -1;
static int rankings_calls;
static int last_rankings_arg = -1;
static int last_special_faction = -1;
static int last_special_gender = -1;
static int last_special_plurality = -1;
static int last_council_faction = -1;
static int mood_value;
static void energy_compute(int faction, int) { last_energy_faction = faction; }
static void say_year(char *s) { strcat(s, "Y"); }
static void say_special(char *s, char *mood, int faction) {
    (void)mood;
    last_special_faction = faction;
    last_special_gender = GenderDefault;
    last_special_plurality = PluralityDefault;
    strcat(s, "SPECIAL");
}
static int get_mood(int) { return mood_value; }
static int council_votes(int faction) {
    last_council_faction = faction;
    return faction * 3;
}
static void rankings(int arg) { ++rankings_calls; last_rankings_arg = arg; }

struct InfoWin {
    Font font_;
    int field_B0_, active_pane_;
    int pane_states_[2], row_count_[2], text_lengths_[2];
    RECT pane_rects_[2];
    char row_text_[2][12][80];
    int row_x_[2][12], row_y_[2][12], row_color_[2][12], row_reveal_color_[2][12];
    void setup_text();
};

/*ACTUAL_SOURCE*/

static void reset_fixture(InfoWin &win, int kind) {
    memset(&win, 0, sizeof(win));
    memset(Players, 0, sizeof(Players));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    memset(&ScenarioSave, 0, sizeof(ScenarioSave));
    memset(GameDataStateStorage.fields_.faction_rankings_, 0, sizeof(GameDataStateStorage.fields_.faction_rankings_));
    memset(FactionRankingHistory, 0, sizeof(FactionRankingHistory));
    memset(FactionRankingsUnk, 0, sizeof(FactionRankingsUnk));
    MainInterfaceGlobal.boxes.clear();
    MainInterfaceGlobal.writes.clear();
    MainInterfaceGlobal.line_colors.clear();
    MainInterfaceGlobal.line_from_x.clear();
    MainInterfaceGlobal.line_to_x.clear();
    MainInterfaceGlobal.box_first_colors.clear();
    MainInterfaceGlobal.box_second_colors.clear();
    MainInterfaceGlobal.fill_colors.clear();
    MainInterfaceGlobal.line_from_y.clear();
    MainInterfaceGlobal.line_to_y.clear();
    MainInterfaceGlobal.frame_calls = 0;
    MainInterfaceGlobal.clip.left = MainInterfaceGlobal.clip.top = 0;
    MainInterfaceGlobal.clip.right = 180;
    MainInterfaceGlobal.clip.bottom = 130;
    MainInterfaceGlobal.work.left = MainInterfaceGlobal.work.top = 0;
    MainInterfaceGlobal.work.right = 180;
    MainInterfaceGlobal.work.bottom = 130;
    win.active_pane_ = 1;
    active_pane_cell = 0;
    switch_active_pane_once = 0;
    console_faction_cell = 0;
    console_faction_new_value = -1;
    change_console_faction_once = 0;
    pane_state_cell = 0;
    pane_state_new_value = -1;
    change_pane_state_once = 0;
    win.pane_states_[1] = kind;
    win.pane_rects_[0].left = win.pane_rects_[0].top = 0;
    win.pane_rects_[0].right = 180; win.pane_rects_[0].bottom = 130;
    win.pane_rects_[1] = win.pane_rects_[0];
    win.font_.unk_1_ = 4; win.font_.height = 8;
    win.font_.line_height_value = 8;
    win.field_B0_ = 4;
    ConsoleGlobal.faction_id_ = 2;
    DiploFrictionFactionID = 3;
    last_energy_faction = -1; rankings_calls = 0; last_rankings_arg = -1;
    last_special_faction = -1;
    last_special_gender = -1;
    last_special_plurality = -1;
    last_council_faction = -1;
    MapWinCaptionScale = MapWinCaptionDivisor = 0;
    ScenarioSave.objective_req_victory = 9000;
    ScenarioSave.objectives_sudden_death_victory = 9000;
    ScenarioSave.objective_achieve_points = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 0;
    GameDataStateStorage.fields_.ranking_faction_id_unk1_ = 0;
    EnergyEconomyTotal = 40; NetIncomeGlobal = 70; BreakthroughTurnsGlobal = 2;
    Language = 1;
    GameDataStateStorage.fields_.game_state_ = 0x80; GameDataStateStorage.fields_.game_rules_ = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xFE00u;
    MapEditorPaintId = 0; MapEditorBrushId = 0;
    MapLongitudeBounds = 128; MapLatitudeBounds = 64; MapIsFlat = 0; MapCloudCover = 2;
    strcpy(MapFilePath, "fixture.sav");
    for (int i = 0; i < MaxPlayerNum; ++i) {
        sprintf(Players[i].formal_name_faction, "Faction%d", i);
        sprintf(Players[i].title_leader, "Title%d", i);
        sprintf(Players[i].name_leader, "Leader%d", i);
        sprintf(Players[i].noun_faction, "Noun%d", i);
        sprintf(Players[i].search_key, "FACTION%d", i);
        Players[i].noun_gender = i;
        Players[i].is_noun_plural = i & 1;
        Players[i].is_leader_female = i & 1;
        GameDataStateStorage.fields_.faction_rankings_[i] = static_cast<uint32_t>(i);
        InfoFactionTextColors[i] = 20 + i;
        InfoFactionShadowColors[i] = 40 + i;
        ChatItemIdTable[i] = 60 + i;
        ChatShadowColors[i] = 80 + i;
        FactionRankingsUnk[i] = static_cast<uint32_t>(i * 10);
    }
    for (int i = 0; i < 16; ++i) Natural[i].name_short = 100 + i;
    for (int i = 0; i < 64; ++i) Terraforming[i].name = 200 + i;
    FactionRankingsUnk[0] = 9000; // Must not participate in the native 1..7 scan.
}

static void assert_row_aggregation(const InfoWin &win, int pane) {
    int total = 0;
    for (int row = 0; row < win.row_count_[pane]; ++row)
        total += static_cast<int>(strlen(win.row_text_[pane][row]));
    assert(total == win.text_lengths_[pane]);
}

static int native_unsigned_kind_reference(int value) {
    const uint32_t bits = static_cast<uint32_t>(value);
    const uint32_t masked = bits & 0x80000007u;
    if ((masked & 0x80000000u) == 0) return static_cast<int>(masked);
    const uint32_t transformed = ((masked - 1u) | 0xfffffff8u) + 1u;
    if (transformed == 0) return 0;
    return -static_cast<int>(0u - transformed);
}

static void test_signed_kind_normalization_edges() {
    const int values[] = {INT_MIN, -8, -7, -6, -5, -4, -3, -2, -1,
                          0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16};
    InfoWin win;
    for (unsigned i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        const int value = values[i];
        const int expected_kind = native_unsigned_kind_reference(value);
        assert(value % 8 == expected_kind);
        reset_fixture(win, value);
        win.setup_text();
        if (expected_kind == 0) {
            assert(win.row_count_[1] == 5);
            assert(last_energy_faction == 2);
        } else if (expected_kind < 0) {
            assert(win.row_count_[1] == 0);
        } else {
            assert(win.row_count_[1] > 0);
        }
        assert_row_aggregation(win, 1);
    }
}

static void test_unknown_default_tail_two_pane_metrics() {
    const int font_metrics[] = {3, -1};
    InfoWin win;
    for (unsigned branch = 0; branch < 2; ++branch) {
        reset_fixture(win, -1);
        win.active_pane_ = 0;
        win.pane_states_[0] = -1;
        win.pane_states_[1] = -7;
        win.pane_rects_[0].left = 11;
        win.pane_rects_[0].top = 17;
        win.pane_rects_[1].left = 103;
        win.pane_rects_[1].top = 41;
        win.font_.unk_1_ = font_metrics[branch];
        win.font_.height = 9;
        win.font_.line_height_value = 6;
        MapWinCaptionScale = 100;
        MapWinCaptionDivisor = 100;
        win.setup_text();

        const int expected_pitch = branch == 0 ? 12 : 6;
        for (int row = 0; row < 12; ++row) {
#if defined(INFOWIN_TAIL_ALIGNMENT_CONTROL)
            assert(win.row_x_[0][row] == 210);
#else
            assert(win.row_x_[0][row] == 16);
#endif
            assert(win.row_x_[1][row] == 108);
            assert(win.row_y_[0][row] == 22 + row * expected_pitch);
            assert(win.row_y_[1][row] == 46 + row * expected_pitch);
        }
#if defined(INFOWIN_TAIL_ALIGNMENT_CONTROL)
        assert(win.row_count_[0] == 1 && win.row_count_[1] == 0);
#else
        assert(win.row_count_[0] == 0 && win.row_count_[1] == 0);
#endif
        assert(win.text_lengths_[0] == 0 && win.text_lengths_[1] == 0);
        assert(MainInterfaceGlobal.frame_calls == 1);
    }
}

#if defined(INFOWIN_TAIL_ALIGNMENT_CONTROL)
static void test_tail_alignment_forced_default_row() {
    const int font_metrics[] = {3, -1};
    InfoWin win;
    for (unsigned branch = 0; branch < 2; ++branch) {
        reset_fixture(win, -1);
        win.active_pane_ = 0;
        win.pane_states_[0] = -1;
        win.pane_states_[1] = -7;
        win.pane_rects_[0].left = 11;
        win.pane_rects_[0].top = 100;
        win.pane_rects_[1].left = 103;
        win.pane_rects_[1].top = 10;
        win.font_.unk_1_ = font_metrics[branch];
        win.font_.height = 9;
        win.font_.line_height_value = 6;
        MapWinCaptionScale = 100;
        MapWinCaptionDivisor = 100;
        win.setup_text();

        const int expected_pitch = branch == 0 ? 12 : 6;
        assert(win.row_y_[0][0] == 105);
        assert(win.row_y_[0][1] == 105 + expected_pitch);
        assert(win.row_y_[1][0] == 15);
        assert(win.row_y_[1][1] == 15 + expected_pitch);
        assert(win.row_x_[1][0] == 108);
        assert(win.row_x_[0][0] == 210);
        assert(win.row_x_[0][1] == 16);
        assert(win.row_count_[0] == 1 && win.text_lengths_[0] == 0);
    }
}
#endif

#if defined(INFOWIN_ACTIVE_PANE_CONTROL)
static void test_active_pane_reload_native_control() {
    InfoWin win;
    reset_fixture(win, -1);
    win.active_pane_ = 0;
    win.pane_states_[0] = -1;
    win.pane_states_[1] = -7;
    active_pane_cell = &win.active_pane_;
    switch_active_pane_once = 1;
    win.setup_text();
    // The native tail reloads active_pane_ at 0x45836E and re-enters the pane
    // body for pane 1 after this callback mutates the field during pane 0.
    assert(MainInterfaceGlobal.frame_calls == 2);
}
#endif

static void test_current_faction_and_treaty_direction() {
    InfoWin win; reset_fixture(win, 1);
    PlayersData[2].tech_commerce_bonus = 7;
    PlayersData[2].turn_commerce_income = 222;
    PlayersData[3].tech_commerce_bonus = 88;
    PlayersData[3].turn_commerce_income = 999;
    win.setup_text();
    assert(last_energy_faction == 2 && win.row_count_[1] == 4);
    assert(strstr(win.row_text_[1][0], "{8}") != 0);
    assert(strstr(win.row_text_[1][1], "222") != 0);
    assert(strstr(win.row_text_[1][0], "89") == 0);
    assert_row_aggregation(win, 1);

    reset_fixture(win, 1);
    Language = 0;
    PlayersData[2].tech_commerce_bonus = 7;
    PlayersData[2].turn_commerce_income = 222;
    PlayersData[3].tech_commerce_bonus = 88;
    PlayersData[3].turn_commerce_income = 999;
    win.setup_text();
    assert(win.row_count_[1] == 5);
    assert(strstr(win.row_text_[1][3], "Total Costs:") != 0);
    assert(strstr(win.row_text_[1][4], "222") == 0);
    assert(strstr(win.row_text_[1][4], "70") != 0);

    reset_fixture(win, 2);
    PlayersData[2].diplo_treaties[3] = 1;
    PlayersData[3].diplo_treaties[2] = 2;
    win.setup_text();
    assert(strstr(win.row_text_[1][3], "L211") != 0);
    assert(strstr(win.row_text_[1][3], "L212") == 0);
    assert_row_aggregation(win, 1);

    reset_fixture(win, 3);
    PlayersData[2].integrity_blemishes = 3;
    PlayersData[2].ranking = 2;
    win.setup_text();
    assert(win.row_count_[1] == 5);
    assert(strstr(win.row_text_[1][4], "6") != 0);
    assert(last_special_faction == 2);
}

static void test_case_three_current_actor_and_gender() {
    InfoWin win;
    reset_fixture(win, 3);
    strcpy(Players[2].formal_name_faction, "CurrentFaction");
    strcpy(Players[2].title_leader, "CurrentTitle");
    strcpy(Players[2].name_leader, "CurrentLeader");
    Players[2].noun_gender = 37;
    Players[2].is_leader_female = 1;
    strcpy(Players[3].formal_name_faction, "FrictionFaction");
    strcpy(Players[3].title_leader, "FrictionTitle");
    strcpy(Players[3].name_leader, "FrictionLeader");
    Players[3].noun_gender = 0;
    Players[3].is_leader_female = 0;
    PlayersData[2].integrity_blemishes = 3;
    PlayersData[2].ranking = 2;
    win.setup_text();

    assert(strcmp(win.row_text_[1][0], "CurrentFaction") == 0);
    assert(strcmp(win.row_text_[1][1], "CurrentTitle CurrentLeader") == 0);
    assert(strstr(win.row_text_[1][1], "Friction") == 0);
    assert(last_special_gender == Players[2].is_leader_female);
    assert(last_special_plurality == 0);
    assert(last_special_faction == 2);
    assert(last_council_faction == 2);
    assert(win.row_count_[1] == 5);
}

static void test_kind_is_read_after_box_callback() {
    InfoWin win;
    reset_fixture(win, -1);
    pane_state_cell = &win.pane_states_[1];
    pane_state_new_value = 0;
    change_pane_state_once = 1;
    win.setup_text();
    assert(win.pane_states_[1] == 0);
    assert(win.row_count_[1] == 5);
    assert(last_energy_faction == 2);
}

static void test_case_six_uses_fresh_live_faction_fallback() {
    InfoWin win;
    reset_fixture(win, 6);
    for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        PlayersData[candidate].tech_ranking = -100 - candidate;
        PlayersData[candidate].mil_strength_1 = -100 - candidate;
        PlayersData[candidate].pop_total = -100 - candidate;
        PlayersData[candidate].energy_reserves = -100 - candidate;
    }
    console_faction_cell = &ConsoleGlobal.faction_id_;
    console_faction_new_value = 4;
    change_console_faction_once = 1;
    win.setup_text();

    assert(ConsoleGlobal.faction_id_ == 4);
    assert(strstr(win.row_text_[1][0], "Leader4") != 0);
    assert(strstr(win.row_text_[1][1], "Leader4") != 0);
    assert(strstr(win.row_text_[1][2], "Leader4") != 0);
    assert(strstr(win.row_text_[1][3], "Leader4") != 0);
    assert(strstr(win.row_text_[1][4], "Leader7") != 0);
    assert(win.row_count_[1] == 5);
}

static void test_chart_counts_and_index_seven() {
    InfoWin win; reset_fixture(win, 4);
    GameDataStateStorage.fields_.turn_current_num_ = 300;
    FactionRankingHistory[3 * MaxPlayerNum + 1] = 0xFF9C;
    win.setup_text();
    assert(win.row_count_[1] == 8);
    assert(rankings_calls == 1);
    assert(last_rankings_arg == 0);
    assert(win.row_text_[1][0][0] != 0);
    assert(!MainInterfaceGlobal.line_from_y.empty());
    assert(MainInterfaceGlobal.line_colors[0] == InfoFactionShadowColors[1]);
    assert(MainInterfaceGlobal.line_from_x.size() == 1043);
    assert(MainInterfaceGlobal.line_from_x[0] == 3);
    assert(MainInterfaceGlobal.line_to_x[0] == 4);
    assert(MainInterfaceGlobal.line_from_y[0] == 126);
    assert(MainInterfaceGlobal.line_to_y[0] == 226);
    assert(win.row_text_[1][1][0] != 0 && win.row_text_[1][7][0] != 0);
    assert(strcmp(win.row_text_[1][1], "Noun7") == 0);
    assert(strcmp(win.row_text_[1][7], "Noun1") == 0);
    assert(MainInterfaceGlobal.line_to_y[0] > MainInterfaceGlobal.clip.bottom);
    assert_row_aggregation(win, 1);

    reset_fixture(win, 5);
    win.setup_text();
    assert(win.row_count_[1] == 1);
    assert(rankings_calls == 1 && last_rankings_arg == 0);
    assert(MainInterfaceGlobal.boxes.size() == 7);
    assert(MainInterfaceGlobal.boxes[0].left == 9);
    assert(MainInterfaceGlobal.boxes[0].right == 32);
    assert(MainInterfaceGlobal.boxes[0].bottom == 125);
    assert(MainInterfaceGlobal.box_first_colors[0] == InfoFactionShadowColors[1]);
    assert(MainInterfaceGlobal.box_second_colors[0] == InfoFactionShadowColors[1]);
    assert(MainInterfaceGlobal.fill_colors.size() == 7);
    assert(MainInterfaceGlobal.fill_colors[0] == InfoFactionTextColors[1]);
    assert(MainInterfaceGlobal.boxes[6].left == 147);
    assert(MainInterfaceGlobal.boxes[6].right == 170);
    assert(MainInterfaceGlobal.boxes[6].top < MainInterfaceGlobal.boxes[0].top);
    assert(MainInterfaceGlobal.writes.size() == 7);
    for (int i = 0; i < 7; ++i) {
        int expected_height = (i + 1) * 16;
        assert(MainInterfaceGlobal.boxes[i].bottom - MainInterfaceGlobal.boxes[i].top ==
               expected_height);
    }
    assert_row_aggregation(win, 1);

    reset_fixture(win, 5);
    win.font_.height = 11;
    win.font_.line_height_value = 6;
    win.setup_text();
    const int distinct_metric_heights[] = {16, 32, 48, 65, 81, 97, 114};
    for (int i = 0; i < 7; ++i)
        assert(MainInterfaceGlobal.boxes[i].bottom - MainInterfaceGlobal.boxes[i].top ==
               distinct_metric_heights[i]);
    assert(win.row_y_[1][0] == 5);
    assert(win.row_y_[1][1] == 20); // height 11 + font metric 4, not line height 6
}

static void test_kind_six_typed_winners_and_region_range() {
    InfoWin win; reset_fixture(win, 6);
    PlayersData[1].tech_ranking = 40000;
    PlayersData[2].tech_ranking = 35000;
    PlayersData[3].tech_ranking = -4;
    PlayersData[4].tech_ranking = 100000;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(1u << (8 + 4)); // Inactive leader must not win.
    PlayersData[2].mil_strength_1 = 77;
    PlayersData[3].mil_strength_1 = 77; // Later equal candidate wins.
    PlayersData[2].pop_total = 100;
    PlayersData[1].energy_reserves = 900;
    PlayersData[1].unk_78[5] = 12;
    PlayersData[1].unk_78[66] = 13;
    PlayersData[2].unk_78[5] = 50;
    for (int sample = 6; sample < 66; ++sample)
        PlayersData[1].unk_78[sample] = 2;
    PlayersData[2].unk_78[1] = 1000; // Outside the native sample interval.
    win.setup_text();
    assert(win.row_count_[1] == 5);
    assert(strstr(win.row_text_[1][0], "L263") != 0);
    assert(strstr(win.row_text_[1][0], "Leader1") != 0);
    assert(strstr(win.row_text_[1][1], "Leader3") != 0);
    assert(strstr(win.row_text_[1][3], "Leader1") != 0);
    assert(strstr(win.row_text_[1][4], "Leader1") != 0);
    assert(strstr(win.row_text_[1][0], "L263") != 0);
    assert(strstr(win.row_text_[1][0], "L1052") == 0);
    assert(strstr(win.row_text_[1][1], "L264") != 0);
    assert(strstr(win.row_text_[1][2], "L265") != 0);
    assert(strstr(win.row_text_[1][3], "L316") != 0);
    assert(strstr(win.row_text_[1][4], "L607") != 0);
    assert_row_aggregation(win, 1);

    GameDataStateStorage.fields_.ranking_faction_id_unk1_ = 2;
    reset_fixture(win, 6);
    GameDataStateStorage.fields_.ranking_faction_id_unk1_ = 2;
    GameDataStateStorage.fields_.faction_rankings_[6] = 3;
    GameDataStateStorage.fields_.faction_rankings_[7] = 6;
    win.setup_text();
    assert(win.row_count_[1] == 6);
    assert(strstr(win.row_text_[1][5], "Leader6") != 0);
}

static void test_map_pane_and_common_text_tail() {
    InfoWin win; reset_fixture(win, 7);
    MapEditorBrushId = 1;
    win.setup_text();
    assert(win.row_count_[1] == 6);
    assert(strstr(win.row_text_[1][0], "L427") != 0);
    assert(strstr(win.row_text_[1][1], "fixture.sav") != 0);
    assert(strstr(win.row_text_[1][2], "L943") != 0);
    assert(win.text_lengths_[1] > 0);
    assert(strstr(win.row_text_[1][2], "L943") != 0);
    assert(strstr(win.row_text_[1][3], "L418") != 0);
    assert(strstr(win.row_text_[1][4], "64x64") != 0);
    assert(strstr(win.row_text_[1][4], "L865") != 0);
    assert(strstr(win.row_text_[1][5], "L22") != 0);
    assert_row_aggregation(win, 1);

    const int paint_ids[] = {0, 1, 2, 3, 4, 5, 26, 27, 28, 29, 6, 17};
    const char *paint_names[] = {"L943", "L944", "L945", "L946", "L947", "L948",
                                 "S102", "S106", "S103", "S110", "S200", "S211"};
    for (unsigned i = 0; i < sizeof(paint_ids) / sizeof(paint_ids[0]); ++i) {
        reset_fixture(win, 7);
        MapEditorPaintId = paint_ids[i];
        win.setup_text();
        assert(win.row_count_[1] == 6);
        assert(win.row_text_[1][2][0] != 0);
        assert(strstr(win.row_text_[1][2], paint_names[i]) != 0);
    }

    reset_fixture(win, 7);
    MapEditorPaintId = 17;
    MapEditorBrushId = 1;
    MapIsFlat = 1;
    MapCloudCover = 5;
    win.setup_text();
    assert(strstr(win.row_text_[1][2], "S") != 0);
    assert(strstr(win.row_text_[1][3], "L418") != 0);
    assert(strstr(win.row_text_[1][4], "L864") != 0);
    assert(strstr(win.row_text_[1][5], "L25") != 0);

    reset_fixture(win, 0);
    PlayersData[2].tech_id_researching = 7;
    win.setup_text();
    assert(last_energy_faction == 2 && win.row_count_[1] == 5);
    assert(win.text_lengths_[1] > 0);
    assert(strstr(win.row_text_[1][1], "L106") != 0);
    PlayersData[2].tech_id_researching = -1;
    win.setup_text();
    assert(win.row_count_[1] == 5);
    assert(win.row_text_[1][1][0] == 0);
}

static void run_standard_suite() {
    test_unknown_default_tail_two_pane_metrics();
    test_signed_kind_normalization_edges();
    test_current_faction_and_treaty_direction();
    test_case_three_current_actor_and_gender();
    test_kind_is_read_after_box_callback();
    test_case_six_uses_fresh_live_faction_fallback();
    test_chart_counts_and_index_seven();
    test_kind_six_typed_winners_and_region_range();
    test_map_pane_and_common_text_tail();
}

int main() {
#if defined(INFOWIN_TAIL_ALIGNMENT_CONTROL)
    if (false) run_standard_suite();
    test_tail_alignment_forced_default_row();
#elif defined(INFOWIN_ACTIVE_PANE_CONTROL)
    if (false) run_standard_suite();
    test_active_pane_reload_native_control();
#else
    run_standard_suite();
#endif
    return 0;
}
