"""Host controls compile actual src/game.cpp end_of_game body and mutants.

The harness owns only dependency boundaries and fixture state. It extracts the
product source's actual namespace helpers and function body verbatim;
the test does not contain a parallel endgame implementation.
"""
from pathlib import Path
import os
import subprocess
import tempfile
from current_game_storage_fixture import current_game_storage_fixture


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/game.cpp"


FIXTURE = r'''
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#define __cdecl
typedef int BOOL;
enum { STATE_UNK_1=1, STATE_UNK_8=8, STATE_IS_SCENARIO=0x10,
       STATE_SCN_VICT_HIGHEST_AC_SCORE_WINS=0x80000,
       STATE_VICTORY_ECONOMIC=0x400000 };
enum { RULES_VICTORY_COOPERATIVE=0x1000,
       RULES_SCN_VICT_SOLO_MISSION=0x80000,
       PREF_AV_SECRET_PROJECT_MOVIES=0x10000000,
       PREF_AV_INTERLUDES_DISABLED=0x40000, RFLAG_ALIEN=0x80 };
struct Player {
    int is_leader_female;
    char filename[32];
    char title_leader[32];
    char name_leader[32];
    int noun_gender;
    int is_noun_plural;
    char noun_faction[32];
    int rule_flags;
};
struct PlayerData {
    uint32_t flags;
    int corner_market_turn;
    int corner_market_active;
    uint32_t diplo_treaties[8];
};
struct Base { int faction_id_current; int8_t population_size; };
struct RulesFixture { uint32_t size_base_subspace_gen; uint32_t subspace_generators_needed; };
struct LockStateEntry {
    uint32_t leading_words_[4];
    union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_;
    uint32_t trailing_words_[10];
};
struct LockCollection { int clears; void clear() { ++clears; } };
struct AlphaNet { int closes; AlphaNet() : closes(0) {} void close() { ++closes; } };
struct NetDaemon : AlphaNet {
    struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; uint32_t trailing_words_[10]; } primary_state_;
    LockStateEntry channels_[2];
    LockCollection locks_;
    int hangs;
    NetDaemon() : hangs(0) { memset(&primary_state_, 0, sizeof(primary_state_)); memset(channels_, 0, sizeof(channels_)); }
    uint32_t &network_message_count() { return channels_[1].trailing_words_[7]; }
    void hang_up() { ++hangs; }
};

Player Players[8];
PlayerData PlayersData[8];
Base Bases[512];
RulesFixture Rules = {10, 6};
NetDaemon NetDaemonNet;
/* ACTUAL_GAME_STORAGE */
int GameRestartQueued;
int EndgameExitReason;
int ExitTurnLoop;
int IsMultiplayerNet;
int ConsoleFaction = 1;
struct ConsoleFixture { int faction_id_; } ConsoleGlobal = {1};

int Language;
int GenderDefault;
int PluralityDefault;
char StringTemp[1032];
char *ScriptPopupFile = const_cast<char *>("script");
int popup_calls, xpopup_calls, parse_calls, credits_calls, score_calls;
int hall_calls, replay_calls, quayle_calls, interlude_calls, movie_calls;
int mon_calls, last_reason, popup_result;
int objective_result;
char last_xpopup[64];
char last_popp_label[64];
char first_popp_label[64];

int __cdecl parse_says(int, const char *, int, int) { ++parse_calls; return 0; }
int __cdecl parse_num(int, int) { return 0; }
void __cdecl say_year(char *out) { strcpy(out, "year"); }
int __cdecl popp(const char *, const char *label, int, const char *, int) {
    ++popup_calls; strncpy(last_popp_label, label, sizeof(last_popp_label) - 1);
    if (popup_calls == 1)
        strncpy(first_popp_label, label, sizeof(first_popp_label) - 1);
    return popup_result;
}
int __cdecl X_pop(const char *label, int) {
    ++xpopup_calls; strncpy(last_xpopup, label, sizeof(last_xpopup) - 1);
    return popup_result;
}
int __cdecl X_pop(char *caption, const char *, int (*)()) {
    ++xpopup_calls; strncpy(last_xpopup, caption, sizeof(last_xpopup) - 1);
    return popup_result;
}
char *_itoa(int value, char *buffer, int radix) {
    if (radix != 10) return buffer;
    sprintf(buffer, "%d", value); return buffer;
}
BOOL __cdecl has_fac(int, int, int) { return 0; }
int __cdecl num_objectives(int, int) { return objective_result; }
void __cdecl compute_score(int, int *score, int *items, int) {
    *score = objective_result; memset(items, 0, 9 * sizeof(int));
}
void __cdecl interlude(int, char *, int, int) { ++interlude_calls; }
void __cdecl amovie_project(const char *) { ++movie_calls; }
void __cdecl mon_winning_unify(int, int) { ++mon_calls; }
void __cdecl show_credits() { ++credits_calls; }
void __cdecl report_score(int) { ++score_calls; }
void __cdecl quayle(int) { ++quayle_calls; }
void __cdecl hall_of_fame(int) { ++hall_calls; }
void __cdecl show_replay() { ++replay_calls; }
char *CharUpperA(char *s) {
    for (char *p = s; *p; ++p) *p = static_cast<char>(toupper(*p));
    return s;
}

static void reset_fixture() {
    memset(Players, 0, sizeof(Players));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Bases, 0, sizeof(Bases));

    NetDaemonNet = NetDaemon();
    GameDataStateStorage = GameDataStateWindow();
    ScenarioSave = ScenarioSaveState();
    ScenarioSave.ending_mission_year = 2500;
    GameRestartQueued = EndgameExitReason = 0;
    ExitTurnLoop = IsMultiplayerNet = 0;
    ConsoleGlobal.faction_id_ = 1;
    GameDataStateStorage.fields_.turn_current_num_ = 1; Language = 0;
    NetDaemonNet.network_message_count() = 0;
    GenderDefault = PluralityDefault = 0;
    memset(StringTemp, 0, sizeof(StringTemp));
    popup_calls = xpopup_calls = parse_calls = credits_calls = score_calls = 0;
    hall_calls = replay_calls = quayle_calls = interlude_calls = movie_calls = 0;
    mon_calls = last_reason = popup_result = objective_result = 0;
    memset(last_xpopup, 0, sizeof(last_xpopup));
    memset(last_popp_label, 0, sizeof(last_popp_label));
    memset(first_popp_label, 0, sizeof(first_popp_label));
}

/* ACTUAL_CANDIDATE_HELPERS */
/* ACTUAL_END_OF_GAME_BODY */

int main() {
    int failures = 0;
    reset_fixture();
    if (end_of_game(0) != 0 || popup_calls || xpopup_calls || score_calls)
        ++failures;

    reset_fixture();
    GameDataStateStorage.fields_.game_state_ = STATE_UNK_1;
    EndgameExitReason = 1;
    if (end_of_game(0) != 1 || credits_calls != 1 || score_calls != 1 ||
        hall_calls != 1 || quayle_calls != 1 || replay_calls != 1 ||
        ExitTurnLoop != 1) ++failures;
    const int prior_scores = score_calls;
    if (end_of_game(0) != 0 || score_calls != prior_scores)
        ++failures;

    reset_fixture();
    GameDataStateStorage.fields_.mission_year_current_ = ScenarioSave.ending_mission_year - 20;
    if (end_of_game(1) != 0 || strcmp(last_xpopup, "RETIREWARNING") != 0 ||
        GameDataStateStorage.fields_.game_state_ != 0) ++failures;

    reset_fixture();
    GameDataStateStorage.fields_.game_state_ = STATE_IS_SCENARIO;
    GameDataStateStorage.fields_.mission_year_current_ = ScenarioSave.ending_mission_year - 10;
    if (end_of_game(1) != 0 || strcmp(last_xpopup, "SCENARIOWARNING") != 0 ||
        GameDataStateStorage.fields_.game_state_ != STATE_IS_SCENARIO) ++failures;

    reset_fixture();
    GameDataStateStorage.fields_.game_state_ = STATE_UNK_1;
    EndgameExitReason = 11;
    PlayersData[2].flags = 0x00020000u;
    PlayersData[2].diplo_treaties[ConsoleGlobal.faction_id_] = 0x00040020u;
    if (end_of_game(0) != 1 ||
        strcmp(first_popp_label, "DIPLOMATICLOSE2") != 0) ++failures;

    reset_fixture();
    GameDataStateStorage.fields_.game_state_ = STATE_UNK_1;
    EndgameExitReason = 1;
    IsMultiplayerNet = 1;
    if (end_of_game(0) != 1 || NetDaemonNet.hangs != 1 ||
        NetDaemonNet.closes != 1 || NetDaemonNet.locks_.clears != 1 ||
        IsMultiplayerNet != 0 || GameRestartQueued != 0 ||
        NetDaemonNet.channels_[1].trailing_words_[6] != 0xFF)
        ++failures;

    if (failures) fprintf(stderr, "endgame source control failures: %d\n", failures);
    return failures != 0;
}
'''


def extract_braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function: {signature}")


def compile_and_run(body: str, helpers: str) -> subprocess.CompletedProcess:
    compiler = os.environ.get("CXX", "g++")
    source = FIXTURE.replace(
        "/* ACTUAL_GAME_STORAGE */", current_game_storage_fixture(scenario=True))
    source = source.replace("/* ACTUAL_CANDIDATE_HELPERS */", helpers)
    source = source.replace("/* ACTUAL_END_OF_GAME_BODY */", body)
    with tempfile.TemporaryDirectory(prefix="osmx-endgame-host-") as temp_dir:
        source_path = Path(temp_dir) / "control.cpp"
        executable = Path(temp_dir) / "control"
        source_path.write_text(source)
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", str(source_path), "-o", str(executable)],
            text=True, capture_output=True, check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(executable)], text=True, capture_output=True,
                              check=False)


def test_actual_endgame_source_and_presented_state_mutant():
    source = SOURCE.read_text()
    reason_enum = source.index("enum EndgameReasonValue")
    namespace_start = source.rfind("namespace {", 0, reason_enum)
    definition_start = source.index("int __cdecl end_of_game(int check_victory) {")
    namespace_end = source.rfind("\n}\n", reason_enum, definition_start) + 2
    assert namespace_end > namespace_start
    helpers = source[namespace_start:namespace_end]
    signature = "int __cdecl end_of_game(int check_victory)"
    body = extract_braced(source, signature)

    result = compile_and_run(body, helpers)
    assert result.returncode == 0, result.stdout + result.stderr

    mutant = body.replace(
        "if ((GameDataStateStorage.fields_.game_state_ & EndgameRecorded) != 0 &&\n        (GameDataStateStorage.fields_.game_state_ & EndgamePresented) != 0)",
        "if ((GameDataStateStorage.fields_.game_state_ & EndgameRecorded) != 0 && false)",
        1,
    ).replace(
        "if ((GameDataStateStorage.fields_.game_state_ & EndgamePresented) != 0) return 0;",
        "if (false) return 0;",
        1,
    )
    assert mutant != body
    result = compile_and_run(mutant, helpers)
    assert result.returncode != 0, "presented-state mutant unexpectedly passed"

    treaty_mutant = body.replace(
        "PlayersData[winner]",
        "PlayersData[ConsoleGlobal.faction_id_]",
    )
    assert treaty_mutant != body
    result = compile_and_run(treaty_mutant, helpers)
    assert result.returncode != 0, "reversed treaty-index mutant unexpectedly passed"
