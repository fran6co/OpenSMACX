// Actual-body host syntax/control probe. This includes the composed candidate
// verbatim and supplies only typed owner/callee doubles; it is not a source TU.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#define __cdecl
#define MaxPlayerNum 8
#define MaxTechnologyNum 89
#define BaseCurrentCount 64
#define RFLAG_ALIEN 0x80u

// Minimal typed view of the real state-window members read by communicate.
// This fixture proves host behavior only; it does not claim ABI layout.
struct GameDataLockMaskBytes {
    uint8_t human_mask_, alive_mask_, unassigned_[2];
};
union GameDataLockMaskView {
    uint32_t enable_mask_;
    GameDataLockMaskBytes bytes_;
};
struct GameDataStateWindowFields {
    uint32_t game_rules_;
    uint32_t game_state_;
    int base_current_count_;
    int turn_current_num_;
    GameDataLockMaskView lock_masks_;
    uint8_t game_tech_achieved_[MaxTechnologyNum];
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
GameDataStateWindow GameDataStateStorage;

/*ACTUAL_SOCIAL_EFFECT_OWNER*/
struct SocialCategory { int models_[4]; };
struct Player {
    char *filename, *name_leader, *title_leader, *noun_faction;
    char *adj_name_faction, *adj_insult_faction, *adj_insult_leader;
    char *insult_leader;
    int is_leader_female, is_noun_plural, noun_gender, rule_flags;
    int soc_ideology_category, soc_ideology_model, soc_ideology_effect;
    int soc_anti_ideology_category, soc_anti_ideology_model;
};
struct PlayerData {
    int diplo_treaties[8], diplo_agenda[8], diplo_friction[8];
    char diplo_patience[8];
    int diplo_spoke[8], diplo_unk1[8], diplo_unk3[8];
    int diplo_unk4[8], diplo_wrongs[8], loan_balance[8], loan_payment[8];
    uint16_t region_force_rating[128], region_total_combat_vehs[128];
    uint8_t region_total_bases[128], region_total_offensive_vehs[128];
    uint16_t unk_77[128];
    int current_num_bases, energy_reserves, current_num_vehs, total_mil_veh;
    int mil_strength_1, mil_strength_2, ranking, diff_level, ai_fight;
    int atrocities, traded_maps, tech_accumulated, tech_id_researching;
    uint32_t tech_pact_shared_goals[12];
    int tech_commerce_bonus, tech_ranking;
    int integrity_blemishes, global_reputation, maint_cost_total, unk_13;
    SocialEffect soc_effect_pending, soc_effect_base;
    SocialCategory soc_category_pending;
};
struct Base {
    int faction_id_current, faction_id_former, visibility, population_size;
    int x, y;
    char *name_string;
};
struct RulesTechnology {
    uint32_t flags;
    char *name;
    char id[8];
    int padding, growth_value, tech_value, wealth_value, power_value;
    int preq_tech_1, preq_tech_2;
};
struct RulesBasic {
    int tgl_human_always_contact_net;
    int tgl_humans_always_contact_pbem;
    int subspace_generators_needed, size_base_subspace_gen;
};
struct Sprite {};
struct FactionArt { Sprite diplomacy; };

struct Console {
    int faction_id_;
    void focus(int, int, int) {}
    void update_data(int) {}
};
struct ConsoleInfo {
    int redraw_phase_;
    void set_redraw_phase(int value) { redraw_phase_ = value; }
};
struct NetDaemon {
    void await_diplo(int = 0) {}
    int add_global(int = 0) { return 0; }
    void unlock(int = 0) {}
};
extern int g_popup_constructions;
extern int g_popup_destructions;
extern int g_popup;
struct Popup {
    Popup() { ++g_popup_constructions; }
    ~Popup() { ++g_popup_destructions; }
    int set_picture(Sprite *, int, char *) { return 0; }
    int start(char *, const char *, int, char *, int, void *) { return 0; }
    int start(const char *, int) { return 0; }
    int exec(int (*)()) { return g_popup; }
    int item(char *, int) { return 0; }
};
struct MapWin { void focus(int, int) {} };
MapWin *console_map_win() { static MapWin window; return &window; }

Player Players[MaxPlayerNum];
PlayerData PlayersData[MaxPlayerNum];
Base Bases[BaseCurrentCount];
RulesTechnology Technology[MaxTechnologyNum];
RulesBasic Rules;
Console ConsoleGlobal;
ConsoleInfo ConsoleInfoWin;
NetDaemon NetDaemonNet;
char StringTemp[4096];
char PopupStartCaption[256];
char ScriptPopupFile[256];
int FactionDiplomacyFlags[MaxPlayerNum];
Sprite *FactionDiplomacyPortraits[MaxPlayerNum];
Sprite g_portraits[MaxPlayerNum];
char g_player_filenames[MaxPlayerNum][16];
int GamePreferences, GameMorePreferences, GameWarnings;
int DiplomacyActive, DiplomacyContactResult, DiplomacyEndState;
int DiplomacyMapTransferChanged, DiplomacyProposalMode, EndgameExitReason;
int DiplomacyProposalGateCount;
int DiploFriction;
uint32_t DiploFrictionFactionID, DiploFrictionFactionIDWith;
int IsMultiplayerNet, IsMultiplayerPBEM;
int BaseIDCurrentSelected, WinScreenWidth;
int PopupDialogFactionID, ExitTurnLoop, GenderDefault, PluralityDefault;
int FALSE = 0, TRUE = 1;
FactionArt FactionsArt[MaxPlayerNum];
int MaxPlayerNumValue = MaxPlayerNum;
uint32_t DiplomacyRegionMask[128];

int DiploScratchF660, DiploScratchF7B4, DiploScratchF7B8, DiploScratchF7C0;
int DiploScratchF7C4, DiploScratchF7C8, DiploScratchF7D0, DiploScratchF7D4;
int DiploScratchF7D8, DiploScratchF7DC, DiploScratchF7E0, DiploScratchF7E4;
int DiploScratchF7E8, DiploScratchF7F0, DiploScratchF7FC, DiploScratchF800;
int DiploScratchF808, DiploScratchFA18, DiploScratchFA1C, DiploScratchFA24;
int DiploScratchFA28, DiploScratchFA2C, DiploScratchFA34, DiploScratchFA38;
int DiploScratchFA3C, DiploScratchFA48, DiploScratchFA4C, DiploScratchFA50;
int DiploScratchFA54, DiploScratchFA60, DiploScratchFA64, DiploScratchFA68;
int DiploScratchFA70, DiploScratchFA74, DiploScratchFA78, DiploScratchFA80;
int DiploScratchFA88, DiploScratchFA8C, DiploScratchFA90, DiploScratchFA94;
int DiploScratchFA98, DiploScratchFAA0, DiploScratchFAA8, DiploScratchFABC;
int DiploScratchFAC0, DiploScratchFAC8, DiploScratchFAD0, DiploScratchFAD4;

int g_host_entry = 0;
int g_pact_end_calls, g_pact_end_actor, g_pact_end_peer, g_pact_end_wait;
int g_pact_calls, g_pact_actor, g_pact_peer;
int g_loan_calls, g_loan_actor, g_loan_peer, g_loan_balance, g_loan_payment;
int g_energy_calls, g_energy_actor, g_energy_amount, g_energy_peer, g_energy_peer_amount, g_energy_wait;
int g_clear_calls, g_patience;
const char *g_short_label;
int g_speak = 0;
int g_popup = 0;
int g_popup_constructions = 0;
int g_popup_destructions = 0;
int g_wants_calls = 0;
int g_short_popup_calls = 0;
int g_caption_popup_calls = 0;
int g_parse_say_calls = 0;
int g_parse_says_calls = 0;
char *g_caption_popup_caption = 0;
const char *g_caption_popup_label = 0;
int g_caption_popup_value = 0;
Sprite *g_caption_popup_sprite = 0;
int (*g_caption_popup_callback)() = 0;
int pop_wait() { return 0; }
int random(int, int) { return 0; }
int range(int value, int low, int high) {
    return value < low ? low : (value > high ? high : value);
}
/*ACTUAL_COMMUNICATE_SUPPORT*/
void bitmask(int input, int *offset, int *mask) {
    *offset = input / 32;
    *mask = 1 << (input & 31);
}
int bit_count(int value) {
    unsigned int bits = (unsigned int)value;
    int count = 0;
    while (bits) { count += bits & 1u; bits >>= 1; }
    return count;
}
int territory(int, int, int, int *base_id, int) { *base_id = 0; return 0; }
int x_pops_short(const char *label, int, Sprite *, int (*)()) {
    g_short_label = label;
    ++g_short_popup_calls;
    return g_popup;
}
int x_pops_caption_value(char *caption, const char *label, int value,
                         Sprite *sprite, int (*callback)()) {
    ++g_caption_popup_calls;
    g_caption_popup_caption = caption;
    g_caption_popup_label = label;
    g_caption_popup_value = value;
    g_caption_popup_sprite = sprite;
    g_caption_popup_callback = callback;
    return g_popup;
}
int parse_say(int, int, int, int) { ++g_parse_say_calls; return 0; }
int parse_says(int, const char *, int, int) {
    ++g_parse_says_calls;
    return 0;
}
int diplomacy_check(int, int, int) { return 0; }
int wants_to_speak(int, int) { ++g_wants_calls; return g_speak; }
int has_treaty(int actor, int peer, int treaty) {
    return PlayersData[actor].diplo_treaties[peer] & treaty;
}
int has_agenda(int actor, int peer, int agenda) {
    return PlayersData[actor].diplo_agenda[peer] & agenda;
}
int has_tech(int, int) { return 0; }
int has_project(int, int) { return 0; }
int wants_to_attack(...) { return 0; }
int contiguous(...) { return 0; }
int factions_status_byte(int index) {
    return reinterpret_cast<const uint8_t *>(
        &GameDataStateStorage.fields_.lock_masks_.enable_mask_)[index];
}
int is_human(int faction) {
    return factions_status_byte(0) & (1u << faction);
}
int is_alive(int faction) {
    return factions_status_byte(1) & (1u << faction);
}
char *get_title(...) { return (char *)"title"; }
char *get_name(...) { return (char *)"name"; }
char *get_noun(...) { return (char *)"noun"; }
char *get_noun_phrase(...) { return (char *)"noun"; }
char *get_adjective(...) { return (char *)"adj"; }
char *get_adjective_faction(...) { return (char *)"adj"; }
char *get_adjective_insult_faction(...) { return (char *)"adj"; }
char *get_adjective_leader(...) { return (char *)"adj"; }
char *get_adjective_insult_leader(...) { return (char *)"adj"; }
char *get_him_her(...) { return (char *)"them"; }
char *get_his_her(...) { return (char *)"their"; }
char *get_insult_leader(...) { return (char *)"insult"; }
char *get_pact(...) { return (char *)"pact"; }
char *get_pact_hood(...) { return (char *)"pact"; }
char *get_pacts(...) { return (char *)"pacts"; }
char *get_phrase(...) { return (char *)"phrase"; }
int get_patience(int, int) { return g_patience; }

char *const DiploLabelSendingDOUBLECROSS = (char *)"Sending DOUBLE CROSS";
char *const DiploLabelSendingTECH = (char *)"Sending TECH";
char *const DiploLabelSendingENERGY = (char *)"Sending ENERGY";
char *const DiploLabelReleasingDIPLOMATICLOCK = (char *)"Releasing DIPLOMATIC LOCK";

#define INT_STUB(name) inline int name(...) { return 0; }
#define VOID_STUB(name) inline void name(...) {}
INT_STUB(agenda_off) INT_STUB(auto_contact) INT_STUB(bad_reg)
INT_STUB(battle_plans) INT_STUB(cause_friction) INT_STUB(check_spock)
INT_STUB(demand_withdrawal) INT_STUB(call_off_vendetta)
INT_STUB(diplomacy_caption) INT_STUB(diplomacy_ends) INT_STUB(diplomacy_menu)
INT_STUB(do_withdrawal) INT_STUB(double_cross) INT_STUB(draw_map)
INT_STUB(draw_tile) INT_STUB(energy_value) INT_STUB(great_beelzebub)
INT_STUB(great_satan) INT_STUB(help_topic) INT_STUB(log_say)
INT_STUB(make_a_proposal) INT_STUB(make_treaty)
INT_STUB(message_data) INT_STUB(net_agenda_off) INT_STUB(net_cede_base)
INT_STUB(net_double_cross)
INT_STUB(net_maps)  INT_STUB(net_set_agenda)
INT_STUB(net_set_treaty) INT_STUB(net_tech) INT_STUB(net_treaty_off)
INT_STUB(net_treaty_on) INT_STUB(pact_attacks)
INT_STUB(parse_num)
INT_STUB(parse_string) INT_STUB(say_num) INT_STUB(say_tech) INT_STUB(says)
INT_STUB(set_treaty) INT_STUB(spot_base) INT_STUB(tech_achieved)
INT_STUB(tech_analysis) INT_STUB(tech_avail) INT_STUB(tech_rate)
INT_STUB(tech_trade) INT_STUB(tech_val) INT_STUB(treaty_off)
INT_STUB(treaty_on) INT_STUB(pact_ends) INT_STUB(swap)
INT_STUB(mention_prototypes) INT_STUB(pledge_truce)
INT_STUB(text_get) INT_STUB(X_text_open) INT_STUB(exec)
INT_STUB(wait_task) INT_STUB(timeGetTime) INT_STUB(has_tech_call)
char *TextBufferGetPtr; INT_STUB(leaf_00559210_redirect)
char *_itoa(int value, char *buffer, int radix) {
    assert(radix == 10); sprintf(buffer, "%d", value); return buffer;
}
 VOID_STUB(diplo_lock) VOID_STUB(diplo_unlock)

#undef INT_STUB
#undef VOID_STUB

void clear() { ++g_clear_calls; StringTemp[0] = 0; }
void net_pact_ends(int actor, int peer, int wait) {
    ++g_pact_end_calls; g_pact_end_actor = actor; g_pact_end_peer = peer; g_pact_end_wait = wait;
}
void pact_of_brotherhood(int actor, int peer) {
    ++g_pact_calls; g_pact_actor = actor; g_pact_peer = peer; g_speak = 0;
}
void net_loan(int actor, int peer, int balance, int payment) {
    ++g_loan_calls; g_loan_actor = actor; g_loan_peer = peer;
    g_loan_balance = balance; g_loan_payment = payment; g_speak = 0;
}
void net_energy(int actor, int amount, int peer, int peer_amount, int wait) {
    ++g_energy_calls; g_energy_actor = actor; g_energy_amount = amount;
    g_energy_peer = peer; g_energy_peer_amount = peer_amount; g_energy_wait = wait;
}
/*ACTUAL_COMMUNICATE_FUNCTION*/

void reset_extra_fixture(int entry) {
    memset(PlayersData, 0, sizeof PlayersData);
    memset(FactionDiplomacyFlags, 0, sizeof FactionDiplomacyFlags);
    for (int i = 0; i < 8; ++i) {
        Players[i].soc_ideology_category = -1;
        Players[i].soc_ideology_effect = -1;
        Players[i].soc_anti_ideology_category = -1;
    }
    g_host_entry = entry;
    g_short_popup_calls = g_pact_end_calls = g_pact_calls = g_loan_calls = 0;
    g_energy_calls = g_clear_calls = 0;
    g_short_label = 0;
    g_popup = 0;
    g_speak = 1;
    g_patience = 3;
    DiplomacyEndState = 0;
    DiploScratchFA60 = DiploScratchFA70 = DiploScratchFA98 = 0;
    DiploScratchFA78 = DiploScratchFA74 = DiploScratchFA3C = 0;
    DiploScratchFAA0 = DiploScratchFA50 = DiploScratchFA54 = 0;
    DiploScratchF7FC = DiploScratchF7E8 = DiploScratchFA8C = 0;
    DiploScratchFA4C = 0;
    IsMultiplayerNet = IsMultiplayerPBEM = 0;
    GameDataStateStorage.fields_.game_rules_ = 0;
    GameDataStateStorage.fields_.game_state_ = 0;
}
void run_extra_fixtures() {
    assert(sizeof(SocialEffect) == 0x2C);
    assert(communicate_wrapped_add(0x7FFFFFFF, 1) == (-0x7FFFFFFF - 1));
    assert(communicate_wrapped_sub((-0x7FFFFFFF - 1), 1) == 0x7FFFFFFF);
    assert(communicate_wrapped_mul(0x40000000, 4) == 0);
    assert(communicate_sar_i32(-1, 1) == -1);
    assert(communicate_sar_i32((-0x7FFFFFFF - 1), 31) == -1);
    assert(communicate_imul_high_sar(-1, 0x66666667u, 1, 0) == 0);
    assert(communicate_imul_high_sar(10, 0x66666667u, 1, 0) == 2);
    assert(communicate_imul_high_sar(-10, 0x66666667u, 1, 0) == -2);
    const int initial_constructions = g_popup_constructions;
    const int initial_destructions = g_popup_destructions;
    reset_extra_fixture(1);
    GameDataStateStorage.fields_.turn_current_num_ = 73;
    communicate(0, 1, 0);
    assert(g_short_popup_calls == 1 && strcmp(g_short_label, "BREAKPACT") == 0);
    assert(g_pact_end_calls == 1);
    assert(g_pact_end_actor == 1 && g_pact_end_peer == 0 && g_pact_end_wait == 1);
    assert(PlayersData[0].diplo_spoke[1] == 73);

    reset_extra_fixture(2);
    Players[1].soc_ideology_category = 2;
    Players[1].soc_ideology_model = 3;
    PlayersData[0].soc_category_pending.models_[2] = 1;
    PlayersData[1].diplo_agenda[0] = 1;
    PlayersData[0].diplo_treaties[1] = 1;
    communicate(0, 1, 0);
    assert(g_short_popup_calls == 1 && g_pact_end_calls == 1);
    assert(g_clear_calls == 1);
    assert(g_short_label == StringTemp);

    reset_extra_fixture(3);
    g_popup = 1; // Every nonzero response accepts without halving.
    PlayersData[0].energy_reserves = 400;
    PlayersData[1].unk_13 = 4;
    communicate(0, 1, 0);
    assert(g_loan_calls == 1 && g_energy_calls == 1);
    assert(g_loan_actor == 1 && g_loan_peer == 0);
    assert(g_loan_balance == 300 && g_loan_payment == 3);
    assert(g_energy_actor == 0 && g_energy_amount == -200);
    assert(g_energy_peer == 1 && g_energy_peer_amount == 200 && g_energy_wait == 1);

    reset_extra_fixture(3);
    g_popup = 2; // Only response two halves the accepted amount.
    PlayersData[0].energy_reserves = 400;
    PlayersData[1].unk_13 = 4;
    communicate(0, 1, 0);
    assert(g_loan_calls == 1 && g_energy_calls == 1);
    assert(g_loan_balance == 200 && g_loan_payment == 2);
    assert(g_energy_amount == -100 && g_energy_peer_amount == 100);

    reset_extra_fixture(4);
    communicate(0, 1, 0);
    assert(g_pact_calls == 1);
    assert(g_pact_actor == 0 && g_pact_peer == 1);
    assert(g_short_popup_calls == 1 && strcmp(g_short_label, "SWEARAPACT") == 0);

    reset_extra_fixture(5);
    PlayersData[1].diplo_patience[0] = static_cast<char>(-2);
    communicate(0, 1, 0);
    assert(static_cast<int8_t>(PlayersData[1].diplo_patience[0]) == 3);
    assert(g_short_popup_calls == 0);
    for (int effect = 0; effect < 11; ++effect) {
        reset_extra_fixture(0);
        g_speak = 0;
        Players[1].soc_ideology_effect = effect;
        PlayersData[0].soc_effect_pending.values_[effect] = -1;
        PlayersData[1].diplo_friction[0] = 19;
        DiploFrictionFactionID = 1;
        DiploFrictionFactionIDWith = 0;
        DiploFriction = 18;
        FactionDiplomacyFlags[1] = 4;
        communicate(0, 1, 0);
        assert(PlayersData[1].diplo_friction[0] == 20);
        assert(DiploFriction == 21); // Native global update is not clamped.
    }
    assert(g_popup_constructions == initial_constructions + 17);
    assert(g_popup_destructions == initial_destructions + 17);
    puts("communicate actual-body host fixtures: 20 passed (3 entry/lifetime, 6 controlled branches, 11 proven social-effect members)");
}


int main() {
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x303;
    for (int player = 0; player < MaxPlayerNum; ++player) {
        sprintf(g_player_filenames[player], "PLAYER%d", player);
        Players[player].filename = g_player_filenames[player];
        FactionDiplomacyPortraits[player] = &g_portraits[player];
    }
    PlayersData[1].diplo_treaties[0] = 0x08000000;
    FactionDiplomacyFlags[1] = 4;
    DiploScratchFA48 = -1;
    DiploScratchFA8C = -1;
    g_speak = 0;
    communicate(0, 1, 0);
    assert(g_popup_constructions == 1);
    assert(g_popup_destructions == 1);
    assert(g_wants_calls == 1);
    assert(DiplomacyContactResult == 8);
    assert(DiploScratchFA48 == 0);
    assert(DiploScratchFA8C == 0);
    FactionDiplomacyFlags[1] = 4;
    communicate(0, 1, 1);
    assert(g_popup_constructions == 2);
    assert(g_popup_destructions == 2);
    assert(g_wants_calls == 2);
    assert(DiplomacyContactResult == 8);
    PlayersData[1].diplo_treaties[0] = 0;
    FactionDiplomacyFlags[1] = 0;
    g_speak = 1;
    communicate(0, 1, 1);
    assert(g_popup_constructions == 3);
    assert(g_popup_destructions == 3);
    assert(DiplomacyContactResult == 3);
    assert(g_caption_popup_calls > 0);
    assert(g_caption_popup_caption == Players[1].filename);
    assert(g_caption_popup_label == StringTemp);
    assert(g_caption_popup_value == 0x00100000);
    assert(g_caption_popup_sprite == FactionDiplomacyPortraits[1]);
    assert(g_caption_popup_callback == pop_wait);
    assert(g_parse_says_calls > 0);
    run_extra_fixtures();
    return 0;
}

// CONTROLLED_ENTRY_BEGIN
// Test-only controlled entry. Removing this include from body-host-v1.inc
// yields body-v1.inc byte for byte; no translated operation is replaced.
if (g_host_entry != 0) {
    communicate_stack_14 = 0;
    communicate_stack_1c = 0;
    communicate_stack_20 = 0;
    communicate_stack_34 = 0;
    communicate_stack_38 = 0;
    communicate_stack_3c = 0;
    communicate_stack_44 = 1;
    communicate_stack_48 = 0;
    communicate_stack_50 = 0;
    communicate_stack_54 = g_host_entry == 5 ? 1 : 0;
    if (g_host_entry == 1) {
        communicate_stack_c = 0;
        goto communicate_late_contact_priority;
    }
    if (g_host_entry == 2) goto communicate_after_large_offer_result;
    if (g_host_entry == 3) goto communicate_large_offer_payment_calculation;
    if (g_host_entry == 4) goto communicate_pact_offer_fallthrough;
    if (g_host_entry == 5) goto communicate_final_cleanup;
    assert(false);
}

// CONTROLLED_ENTRY_END
