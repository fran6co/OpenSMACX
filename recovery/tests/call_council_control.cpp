// C++98 host doubles for the actual call_council source control. The test
// driver extracts src/council.cpp into CALL_COUNCIL_EXTRACTED; no product
// body or ignored recovery draft is copied into this fixture.
#include <cassert>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl
typedef const char *LPSTR;

static std::vector<std::string> events;
static int popup_selection;
static int popup_items;
static int call_can_council;
static int calls_to_council;
static int calls_to_netmsg_pop;
static int calls_to_netmsg_close;
static int calls_to_message_data;
static int calls_to_diplo_lock;
static int calls_to_diplo_unlock;
static int calls_to_wait_loop;
static int calls_to_has_tech;
static int tech_result;
static int council_lock_result;
static int council_exit_turn;
static int redraw_pending_after_wait;
static int popup_exec_mask_after;
static int vote_values[8];
static int wants_values[8][11];
static std::vector<std::string> displayed_items;
static std::vector<std::string> popped_messages;

static const int MaxPlayerNum = 8;
static const int MaxProposalNum = 11;
static const int PROP_ELECT_PLANETARY_GOVERNOR = 0;
static const int PROP_UNITE_SUPREME_LEADER = 1;
static const int PROP_SALVAGE_UNITY_CORE = 2;
static const int PROP_GLOBAL_TRADE_PACT = 3;
static const int PROP_REPEAL_GLOBAL_TRADE_PACT = 4;
static const int PROP_LAUNCH_SOLAR_SHADE = 5;
static const int PROP_INCREASE_SOLAR_SHADE = 6;
static const int PROP_MELT_POLAR_CAPS = 7;
static const int PROP_REPEAL_UN_CHARTER = 8;
static const int PROP_REINSTATE_UN_CHARTER = 9;
static const unsigned int RFLAG_ALIEN = 0x80;
static const unsigned int DTREATY_PACT = 1;

struct BaseSecretProject {
    unsigned char bytes_[0x100];
    int empath_guild() const { return -1; }
};
#include "../../src/game_data_state_window.h"
GameDataStateWindow GameDataStateStorage;
struct Player { unsigned int rule_flags; };
struct PlayerData {
    int ranking;
    unsigned int diplo_treaties[8];
    int council_call_turn;
    int unk_29[11];
    int unk_30[11];
};
struct RulesProposal { const char *name; const char *description; int preq_tech; };
struct RulesType { unsigned int min_turns_councils; };
struct ConsoleType { int faction_id_; };
struct Dialogs {
    int item(char *text, int index) {
        assert(index == popup_items); ++popup_items;
        displayed_items.push_back(text);
        return 0;
    }
};
struct Scroll {
    void close() { events.push_back("scroll.close"); }
    ~Scroll() { events.push_back("scroll.dtor"); close(); }
};
static unsigned int &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
struct Popup {
    Scroll scroll_;
    Dialogs dialogs_;
    Popup() { events.push_back("popup.ctor"); }
    ~Popup() { events.push_back("popup.dtor.begin"); close(); events.push_back("popup.dtor.end"); }
    void close() { scroll_.close(); events.push_back("popup.baseclose"); }
    void start(char *, const char *title, int, char *, int, void *) {
        assert(std::strcmp(title, "COUNCILISSUES") == 0);
        events.push_back("popup.start");
    }
    Dialogs &dialogs() { return dialogs_; }
    int exec(int, int) {
        events.push_back("popup.exec");
        if (popup_exec_mask_after >= 0)
            LockEnableMask = static_cast<unsigned int>(popup_exec_mask_after);
        return popup_selection;
    }
};
struct Sprite {
    int init(char *, int, int) { events.push_back("sprite.init"); return 0; }
    ~Sprite() { events.push_back("sprite.dtor"); }
};
struct Buffer { static int get_pcx_dimensions(const char *, int *w, int *h) { *w=32; *h=32; events.push_back("dimensions"); return 0; } };
struct Strings {
    char *get(const char *text) {
        const int id = static_cast<int>(reinterpret_cast<intptr_t>(text));
        static char names[MaxProposalNum][24];
        static char descriptions[MaxProposalNum][24];
        if (id >= 100 && id < 100 + MaxProposalNum) {
            std::sprintf(names[id - 100], "proposal%d", id - 100);
            return names[id - 100];
        }
        if (id >= 200 && id < 200 + MaxProposalNum) {
            std::sprintf(descriptions[id - 200], "description%d", id - 200);
            return descriptions[id - 200];
        }
        assert(false && "unexpected proposal string-table id");
        return const_cast<char *>("");
    }
};
struct NetMsg {
    void pop(const char *message, int, int, const char *) {
        ++calls_to_netmsg_pop; popped_messages.push_back(message);
        events.push_back("netmsg.pop");
    }
    void close() { ++calls_to_netmsg_close; events.push_back("netmsg.close"); }
};

static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
static RulesProposal Proposal[MaxProposalNum];
static RulesType Rules;
static ConsoleType ConsoleGlobal;
static Strings StringTable;
static NetMsg NetMsgPrimary;
static int (&CouncilResolutionCount)[MaxProposalNum] =
    GameDataStateStorage.fields_.council_resolution_count_;
static int (&CouncilLastResolutionTurn)[MaxProposalNum] =
    GameDataStateStorage.fields_.council_last_resolution_turn_;
static int CouncilPendingMessageArgs[3];
static int &GovernorFaction = GameDataStateStorage.fields_.governor_faction_;
static int ExpansionEnabled;
static int ConsoleControlTurnActive;
static int IsMultiplayerNet;
static int &TurnCurrentNum = GameDataStateStorage.fields_.turn_current_num_;
static unsigned int &GameState = GameDataStateStorage.fields_.game_state_;
static unsigned int &GameRules = GameDataStateStorage.fields_.game_rules_;
static unsigned int &NetTurnFlags = GameDataStateStorage.fields_.net_turn_flags_;
static int &NetTurnFaction = GameDataStateStorage.fields_.net_turn_faction_;
static int TurnRedrawPending;
static int ExitTurnLoop;
static char PopupStartCaption[256];
static char StringTemp[1032];

static int calls_to_can_call_council;
static int can_call_council(int, int) {
    ++calls_to_can_call_council;
    return call_can_council;
}
static int council_votes(int faction) { return vote_values[faction]; }
static int council_get_vote(int, int, int) { return -1; }
static int wants_prop(int, int proposal, int target) { return wants_values[target][proposal]; }
static int has_tech(int, int) { ++calls_to_has_tech; return tech_result; }
static int game_year(int turn) { return 2300 + turn; }
static int parse_num(int, int) { return 0; }
static int diplo_lock(int) { ++calls_to_diplo_lock; return council_lock_result; }
static void diplo_unlock() { ++calls_to_diplo_unlock; }
static void wait_loop() { ++calls_to_wait_loop; ExitTurnLoop = council_exit_turn; TurnRedrawPending = redraw_pending_after_wait; }
static int message_data(int, int, int, int, int, int) { ++calls_to_message_data; return 0; }
static void council(int, int, int) { ++calls_to_council; events.push_back("council"); }

#ifndef CALL_COUNCIL_EXTRACTED
#error Actual source extraction is required
#endif
#include CALL_COUNCIL_EXTRACTED

static void reset_state() {
    events.clear();
    displayed_items.clear();
    popped_messages.clear();
    popup_items = calls_to_council = calls_to_netmsg_pop = calls_to_netmsg_close = 0;
    calls_to_message_data = calls_to_diplo_lock = calls_to_diplo_unlock = 0;
    calls_to_wait_loop = calls_to_has_tech = 0; tech_result = 0;
    call_can_council = 1; popup_selection = 0; council_lock_result = 0;
    council_exit_turn = 1; redraw_pending_after_wait = 0;
    popup_exec_mask_after = -1;
    calls_to_can_call_council = 0;
    std::memset(&GameDataStateStorage, 0, sizeof GameDataStateStorage);
    std::memset(Players, 0, sizeof Players);
    std::memset(PlayersData, 0, sizeof PlayersData);
    std::memset(CouncilResolutionCount, 0, sizeof CouncilResolutionCount);
    std::memset(CouncilLastResolutionTurn, 0, sizeof CouncilLastResolutionTurn);
    std::memset(CouncilPendingMessageArgs, 0xff, sizeof CouncilPendingMessageArgs);
    std::memset(vote_values, 0, sizeof vote_values);
    std::memset(wants_values, 0, sizeof wants_values);
    for (int i = 0; i < MaxProposalNum; ++i) {
        Proposal[i].name = reinterpret_cast<const char *>(static_cast<intptr_t>(100 + i));
        Proposal[i].description = reinterpret_cast<const char *>(static_cast<intptr_t>(200 + i));
        Proposal[i].preq_tech = -1;
    }
    Rules.min_turns_councils = 20;
    ConsoleGlobal.faction_id_ = 1;
    GovernorFaction = 0; LockEnableMask = 0; ExpansionEnabled = 0;
    ConsoleControlTurnActive = 0; IsMultiplayerNet = 0;
    TurnCurrentNum = 100; GameState = 0x1080; GameRules = 8;
    NetTurnFlags = 0; NetTurnFaction = 0; TurnRedrawPending = 0; ExitTurnLoop = 0;
    PlayersData[1].council_call_turn = 0;
}

static void assert_normal_popup_teardown() {
    assert(events.size() >= 7);
    assert(events[events.size()-7] == "sprite.dtor");
    assert(events[events.size()-6] == "popup.dtor.begin");
    assert(events[events.size()-5] == "scroll.close");
    assert(events[events.size()-4] == "popup.baseclose");
    assert(events[events.size()-3] == "popup.dtor.end");
    assert(events[events.size()-2] == "scroll.dtor");
    assert(events[events.size()-1] == "scroll.close");
    int scroll_closes = 0;
    for (size_t i = 0; i < events.size(); ++i)
        if (events[i] == "scroll.close") ++scroll_closes;
    assert(scroll_closes == 2);  // Popup close + Scroll member dtor.
}

static void test_early_guard_constructs_then_destroys() {
    reset_state(); ExpansionEnabled=1; Players[1].rule_flags=RFLAG_ALIEN;
    call_council(1);
    assert(events[0] == "popup.ctor");
    assert_normal_popup_teardown();
    assert(popup_items == 0 && calls_to_council == 0);
}

static void test_can_call_announcement_and_refusal() {
    reset_state(); call_can_council=0;
    LockEnableMask = 1u << 1; call_council(1);
    assert_normal_popup_teardown();
    assert(popup_items == 0 && calls_to_has_tech == 0);
}

static void test_full_proposal_loop_and_normal_dispatch() {
    reset_state();
    GovernorFaction=7;
    LockEnableMask = (1u << 1) | (0xffu << 8);
    vote_values[1]=1; vote_values[2]=3; vote_values[3]=4;
    call_council(1);
    assert(popup_items > 0);
    assert(displayed_items[0] == "proposal0|description0");
    assert(calls_to_has_tech == MaxProposalNum);
    assert(calls_to_council == 1);
    assert_normal_popup_teardown();
}

static void test_multiplayer_deferred_arguments_are_ordered() {
    reset_state(); IsMultiplayerNet=1; GovernorFaction=7;
    LockEnableMask=(0xffu << 8);
    TurnCurrentNum=110;
    vote_values[1]=1; vote_values[2]=3; vote_values[3]=4;
    wants_values[2][0]=1;
    call_council(1);
    assert(CouncilPendingMessageArgs[0] == 1);
    assert(CouncilPendingMessageArgs[1] == 0);
    assert(CouncilPendingMessageArgs[2] == 2);
    assert(calls_to_message_data == 0 && calls_to_diplo_lock == 0);
    assert_normal_popup_teardown();
}

static void test_network_lock_success_and_failure() {
    reset_state(); IsMultiplayerNet=1;
    LockEnableMask=(1u << 1) | (1u << (8 + 2)) | (1u << (8 + 3));
    vote_values[1]=1; vote_values[2]=3; vote_values[3]=4;
    council_lock_result=0; call_council(1);
    assert(calls_to_diplo_lock == 1 && calls_to_message_data == 1);
    assert(calls_to_wait_loop == 1 && calls_to_diplo_unlock == 1);
    assert(calls_to_netmsg_close == 0);

    reset_state(); IsMultiplayerNet=1;
    LockEnableMask=(1u << 1) | (1u << (8 + 2)) | (1u << (8 + 3));
    vote_values[1]=1; vote_values[2]=3; vote_values[3]=4;
    council_lock_result=1; call_council(1);
    assert(calls_to_diplo_lock == 1 && calls_to_message_data == 0);
    assert(calls_to_diplo_unlock == 0 && calls_to_netmsg_close == 1);
}

static void test_live_mask_reload_after_modal_events() {
    reset_state();
    LockEnableMask = 1u << 1;
    LockEnableMask |= (1u << (8 + 2)) | (1u << (8 + 3));
    popup_selection = 1;
    popup_exec_mask_after = 0;
    PlayersData[1].council_call_turn = 95;
    TurnCurrentNum = 100;
    vote_values[1] = 1; vote_values[2] = 3; vote_values[3] = 4;
    call_council(1);
    assert(calls_to_council == 0);
    assert(popped_messages.empty());
    assert_normal_popup_teardown();
}

static void test_proposal_one_uses_overwritten_game_state_value() {
    reset_state();
    LockEnableMask = (1u << 1) | (1u << (8 + 2)) | (1u << (8 + 3));
    GameState = 0x1081;
    tech_result = 0;
    vote_values[1] = 1; vote_values[2] = 3; vote_values[3] = 4;
    for (int i = 0; i < MaxProposalNum; ++i)
        Proposal[i].preq_tech = -2;
    Proposal[1].preq_tech = -1;
    call_council(1);
    assert(popup_items == 0);
    assert(calls_to_council == 0);
}

static void test_council_call_half_delay_and_resolution_cooldown() {
    reset_state();
    LockEnableMask = 1u << 1;
    LockEnableMask |= (1u << (8 + 2)) | (1u << (8 + 3));
    GovernorFaction = 1;
    GameState = 0x80;
    popup_selection = 1;
    tech_result = 1;
    PlayersData[1].council_call_turn = 88;
    vote_values[1] = 1; vote_values[2] = 3; vote_values[3] = 4;
    call_council(1);
    assert(calls_to_council == 1); // 12 turns reaches governor's 10-turn delay.
    assert(popped_messages.empty());

    reset_state();
    LockEnableMask = 1u << 1;
    LockEnableMask |= (1u << (8 + 2)) | (1u << (8 + 3));
    GovernorFaction = 7;
    GameState = 0x80;
    popup_selection = 1;
    tech_result = 1;
    PlayersData[1].council_call_turn = 88;
    vote_values[1] = 1; vote_values[2] = 3; vote_values[3] = 4;
    call_council(1);
    assert(calls_to_council == 0); // Normal 20-turn delay has not elapsed.
    assert(popped_messages.size() == 1);
    assert(popped_messages[0] == "COUNCILRECENTCALL");

    reset_state();
    LockEnableMask = 1u << 1;
    LockEnableMask |= (1u << (8 + 2)) | (1u << (8 + 3));
    GameState = 0x80;
    popup_selection = 0;
    tech_result = 1;
    CouncilLastResolutionTurn[0] = 95;
    vote_values[1] = 1; vote_values[2] = 3; vote_values[3] = 4;
    call_council(1);
    assert(calls_to_council == 0);
    assert(popped_messages.size() == 1);
    assert(popped_messages[0] == "COUNCILRECENTPROP");
}

static void test_human_byte_does_not_include_adjacent_alive_byte() {
    reset_state();
    ConsoleControlTurnActive = 1;
    ConsoleGlobal.faction_id_ = 8;
    LockEnableMask = 0x00000100u;
    call_can_council = 0;
    call_council(8);
    // Native reads byte +0 for this predicate. The adjacent alive bit at +1
    // must not short-circuit the later admission helper.
    assert(calls_to_can_call_council == 1);
}

int main() {
    test_early_guard_constructs_then_destroys();
    test_can_call_announcement_and_refusal();
    test_full_proposal_loop_and_normal_dispatch();
    test_multiplayer_deferred_arguments_are_ordered();
    test_network_lock_success_and_failure();
    test_live_mask_reload_after_modal_events();
    test_proposal_one_uses_overwritten_game_state_value();
    test_council_call_half_delay_and_resolution_cooldown();
    test_human_byte_does_not_include_adjacent_alive_byte();
    return 0;
}
