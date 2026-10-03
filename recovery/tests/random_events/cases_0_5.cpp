// The included case bodies are extracted from src/game.cpp by pytest.
// This fixture supplies only controlled dependencies and assertions.
#include <assert.h>
#include <stdint.h>
#include <string.h>

enum {
    BEVENT_BUMPER = 0x0200,
    BEVENT_FAMINE = 0x0400,
    BEVENT_INDUSTRY = 0x0800,
    BEVENT_BUST = 0x1000,
    BEVENT_HEAT_WAVE = 0x2000,
    BEVENT_CLOUD_COVER = 0x4000
};

struct Base {
    unsigned int event;
    int random_event_turns;
};
struct PlayerData {
    unsigned int ranking;
    int diff_level;
};
struct Player {
    int rule_flags;
};
struct Console {
    int faction_id_;
};

static Base Bases[8];
static PlayerData PlayersData[8];
static Player Players[8];
static unsigned int FactionRankings[8];
static unsigned int LockEnableMask;
static int TurnCurrentNum;
static int IsMultiplayerPBEM;
static Console ConsoleGlobal;
static char StringTemp[1032];
static int random_results[8];
static int random_result_count;
static int random_calls;
static int popup_calls;
static char popup_caption[1032];
static char popup_picture[64];
static int popup_value;

static int random(int low, int high) {
    assert(low == 0);
    assert(high == 4);
    assert(random_calls < random_result_count);
    return random_results[random_calls++];
}

static int POP2(const char *caption, const char *picture, int value) {
    ++popup_calls;
    strcpy(popup_caption, caption);
    strcpy(popup_picture, picture);
    popup_value = value;
    return 0;
}

#include "actual_cases_0_5.inc"

static void reset_fixture() {
    memset(Bases, 0, sizeof(Bases));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Players, 0, sizeof(Players));
    memset(FactionRankings, 0, sizeof(FactionRankings));
    LockEnableMask = 0;
    TurnCurrentNum = 0;
    IsMultiplayerPBEM = 0;
    ConsoleGlobal.faction_id_ = 1;
    StringTemp[0] = '\0';
    random_result_count = random_calls = popup_calls = 0;
    popup_caption[0] = popup_picture[0] = '\0';
    popup_value = -99;
}

static void test_case0_rank_and_human_gates() {
    reset_fixture();
    FactionRankings[7] = 3;  // different top-ranked faction: no random draw
    random_events_cases_0_5(0, 2, 4, 17);
    assert(random_calls == 0);
    assert(Bases[4].event == BEVENT_BUMPER);
    assert(Bases[4].random_event_turns == 10);
    assert(popup_calls == 0);  // neither local nor PBEM

    reset_fixture();
    FactionRankings[7] = 1;
    LockEnableMask = 1u << 1;
    PlayersData[1].diff_level = 1;  // easy human bypasses RNG
    random_events_cases_0_5(0, 1, 2, 23);
    assert(random_calls == 0);
    assert(popup_calls == 1);
    assert(strcmp(popup_caption, "BUMPER") == 0);
    assert(strcmp(popup_picture, "bump_sm.pcx") == 0);
    assert(popup_value == 23);
}

static void test_case0_random_polarity_and_pbem() {
    reset_fixture();
    FactionRankings[7] = 2;
    LockEnableMask = 1u << 2;
    PlayersData[2].diff_level = 2;
    random_results[0] = 2;
    random_result_count = 1;
    random_events_cases_0_5(0, 2, 0, 8);
    assert(random_calls == 1);
    assert(Bases[0].event == 0);  // nonzero draw suppresses the event
    assert(popup_calls == 0);

    reset_fixture();
    FactionRankings[7] = 2;
    LockEnableMask = 1u << 2;
    PlayersData[2].diff_level = 2;
    IsMultiplayerPBEM = 1;
    random_results[0] = 0;
    random_result_count = 1;
    random_events_cases_0_5(0, 2, 0, 8);
    assert(random_calls == 1);
    assert(Bases[0].event == BEVENT_BUMPER);
    assert(popup_calls == 1);
}

static void test_cases1_3_5_signed_turn_thresholds() {
    reset_fixture();
    TurnCurrentNum = 149;
    PlayersData[1].ranking = 4;
    PlayersData[1].diff_level = 0;  // native gate is ranking, not difficulty
    random_events_cases_0_5(1, 1, 0, 31);
    assert(Bases[0].event == BEVENT_FAMINE);
    assert(popup_calls == 1);

    reset_fixture();
    TurnCurrentNum = 150;
    PlayersData[1].ranking = 4;
    random_events_cases_0_5(3, 1, 0, 32);
    assert(Bases[0].event == 0);  // threshold rises at 150

    reset_fixture();
    TurnCurrentNum = 150;
    PlayersData[1].ranking = 5;
    random_events_cases_0_5(3, 1, 0, 32);
    assert(Bases[0].event == BEVENT_BUST);
    assert(strcmp(popup_picture, "genwarning_sm.pcx") == 0);

    reset_fixture();
    TurnCurrentNum = 149;
    PlayersData[1].ranking = 3;
    random_events_cases_0_5(5, 1, 0, 33);
    assert(Bases[0].event == 0);
}

static void test_case1_alien_picture_and_case2_4_payloads() {
    reset_fixture();
    PlayersData[1].ranking = 4;
    PlayersData[1].diff_level = 1;
    Players[1].rule_flags = 0x80;
    random_events_cases_0_5(1, 1, 3, 44);
    assert(strcmp(popup_picture, "al_starv_sm.pcx") == 0);

    reset_fixture();
    PlayersData[1].ranking = 4;
    random_events_cases_0_5(2, 1, 3, 45);
    assert(Bases[3].event == BEVENT_INDUSTRY);
    assert(strcmp(popup_caption, "INDUSTRY") == 0);
    assert(strcmp(popup_picture, "indbm_sm.pcx") == 0);

    reset_fixture();
    PlayersData[1].ranking = 5;
    TurnCurrentNum = 150;
    random_events_cases_0_5(4, 1, 3, 46);
    assert(Bases[3].event == BEVENT_HEAT_WAVE);
    assert(strcmp(popup_picture, "heat_sm.pcx") == 0);

    reset_fixture();
    PlayersData[1].ranking = 4;
    random_events_cases_0_5(5, 1, 3, 47);
    assert(Bases[3].event == BEVENT_CLOUD_COVER);
    assert(strcmp(popup_picture, "cloud_sm.pcx") == 0);
}

int main() {
    test_case0_rank_and_human_gates();
    test_case0_random_polarity_and_pbem();
    test_cases1_3_5_signed_turn_thresholds();
    test_case1_alien_picture_and_case2_4_payloads();
    return 0;
}
