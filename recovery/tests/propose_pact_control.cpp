// The actual candidate is included verbatim. Only its external services and
// product data types are minimal fixture doubles; no second pact algorithm is
// used as an oracle.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define __cdecl
#define WINAPI __cdecl
typedef uint32_t DWORD;
struct Sprite { int id; };
struct Player {
    int is_leader_female;
    char name_leader[24];
    char title_leader[24];
    char noun_faction[24];
    int noun_gender;
    int is_noun_plural;
};
struct PlayerData {
    uint32_t flags;
    uint32_t ranking;
    int diff_level;
    uint32_t diplo_treaties[8];
    int diplo_agenda[8];
    int global_reputation;
    int energy_reserves;
    int tech_commerce_bonus;
    int mil_strength_1;
    uint32_t diplo_friction[8];
    int diplo_spoke[8];
    char diplo_patience[8];
    uint32_t current_num_bases;
};
// Minimal fixture view of the real nested GameDataStateStorage API used by
// the extracted function; this supplies fields, not compatibility aliases.
struct GameDataStateWindowFields {
    uint32_t game_rules_;
    int turn_current_num_;
    uint32_t faction_rankings_[8];
    uint32_t ranking_faction_id_unk1_;
    uint32_t ranking_faction_id_unk2_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
GameDataStateWindow GameDataStateStorage;
struct Console {
    int updates[8]; int update_count;
    void update_data(int value) { updates[update_count++] = value; }
};
struct InfoWin {
    int redraw_phase;
    void set_redraw_phase(int value) { redraw_phase = value; }
};
struct NetDaemon {
    int messages[8]; int message_count;
    int unlock_count;
    int awaits[8]; int await_count;
    int add_global_result;
    int add_global_count;
    void await_diplo(int opcode) { awaits[await_count++] = opcode; }
    int add_global() { ++add_global_count; return add_global_result > 0 ? add_global_result-- : 0; }
    void unlock() { ++unlock_count; }
};

Player Players[8];
PlayerData PlayersData[8];
Sprite diplomacy_sprites[8];
Sprite *FactionDiplomacyPortraits[8];
uint32_t FactionRankingsUnk[8];
Console ConsoleGlobal;
InfoWin ConsoleInfoWin;
NetDaemon NetDaemonNet;
char StringTemp[1024];
int IsMultiplayerNet;
int PopupDialogFactionID;
int GenderDefault;
int PluralityDefault;
int ExitTurnLoop;
int DiploFriction;
uint32_t DiploFrictionFactionID;
uint32_t DiploFrictionFactionIDWith;
int DiploScratchF7E0, DiploScratchF7E8, DiploScratchF7F0, DiploScratchF7FC;
int DiploScratchFA18, DiploScratchFA24, DiploScratchFA2C, DiploScratchFA3C, DiploScratchFA4C;
int DiploScratchFA54, DiploScratchFA68, DiploScratchFA78, DiploScratchFA8C;
int DiploScratchFA98, DiploScratchFAA8;
int DiploScratchFA9C, DiploScratchFA70, DiplomacyProposalGateCount;
int DiplomacyProposalMode;

char *DiploLabelSendingENERGY = (char *)"Sending ENERGY";
char *DiploLabelSendingSETTREATY = (char *)"Sending SET TREATY";
char *DiploLabelSendingTECH = (char *)"Sending TECH";
char *DiploLabelGettingDIPLOMATICLOCK = (char *)"Getting DIPLOMATIC LOCK";
char *DiploLabelReleasingDIPLOMATICLOCK = (char *)"Releasing DIPLOMATIC LOCK";
char *DiploLabelSendingDOUBLECROSS = (char *)"Sending DOUBLE CROSS";
char *DiploLabelSendingTREATYOFF = (char *)"Sending TREATY OFF";

static int signed_bits(uint32_t value) {
    int result;
    memcpy(&result, &value, sizeof result);
    return result;
}
int diplo_add(int left, int right) {
    return signed_bits((uint32_t)left + (uint32_t)right);
}
int diplo_sub(int left, int right) {
    return signed_bits((uint32_t)left - (uint32_t)right);
}
int diplo_mul(int left, int right) {
    return signed_bits((uint32_t)left * (uint32_t)right);
}

enum { TRACE_POPUP = 1, TRACE_LOG = 2, TRACE_MESSAGE = 3, TRACE_AWAIT = 4,
       TRACE_TECH = 5, TRACE_TREATY = 6, TRACE_PACT = 7, TRACE_UNLOCK = 8 };
int trace_events[64], trace_count;
char popup_labels[8][64];
int popup_count, popup_result, last_popup_value, parsed_energy;
int popup_results[8], popup_response_count, popup_response_index;
int log_count, message_count, message_opcodes[8], message_tech_ids[8];
int tech_count, treaty_count, pact_count, great_satan_result;
int tech_value, random_count, help_count, parse_count, caption_count;
int time_calls, spock_calls;
int mutate_offer_on_callback, callback_offer, callback_second_offer;
int mutate_offer_on_tech_log, tech_log_offer;
int mutate_fa8c_on_callback;
int mutate_second_offer_on_tech_message, second_offer_on_message;
int mutate_energy_on_callback, energy_my_after_callback, energy_other_after_callback;
int achieved_techs[8], achieved_tech_count, tech_val_ids[8], tech_val_count;
int message_args[8][6], last_pact_args[2], last_treaty_args[4];
int last_double_cross_args[3], last_treaty_off_args[3], last_noun_faction;
char *parse_values[8];
int switch_network_on_spock;

int __cdecl x_pops_short(const char *label, int value, Sprite *, int (__cdecl *callback)()) {
    (void)callback;
    strncpy(popup_labels[popup_count], label, sizeof popup_labels[0] - 1);
    last_popup_value = value;
    ++popup_count;
    trace_events[trace_count++] = TRACE_POPUP;
    int result = popup_result;
    if (popup_response_index < popup_response_count) {
        result = popup_results[popup_response_index++];
    }
    if (mutate_offer_on_callback && strncmp(label, "GIMMETECH", 9) == 0) {
        DiploScratchFAA8 = callback_offer;
        DiploScratchFA18 = callback_second_offer;
    }
    if (mutate_energy_on_callback && strncmp(label, "GIMMEENERGY", 11) == 0) {
        PlayersData[1].energy_reserves = energy_my_after_callback;
        PlayersData[2].energy_reserves = energy_other_after_callback;
    }
    if (mutate_fa8c_on_callback) DiploScratchFA8C = 0;
    return result;
}
int __cdecl pop_wait() { return 0; }
void __cdecl log_say(char *label, int, int, int) {
    ++log_count; trace_events[trace_count++] = TRACE_LOG;
    if (mutate_offer_on_tech_log && strcmp(label, "Sending TECH") == 0) {
        DiploScratchFAA8 = tech_log_offer;
        mutate_offer_on_tech_log = 0;
    }
}
int __cdecl message_data(int opcode, int flags, int source, int tech_id, int target, int data) {
    int *args = message_args[message_count];
    args[0] = opcode; args[1] = flags; args[2] = source;
    args[3] = tech_id; args[4] = target; args[5] = data;
    message_opcodes[message_count++] = opcode;
    message_tech_ids[message_count - 1] = tech_id;
    trace_events[trace_count++] = TRACE_MESSAGE;
    if (mutate_second_offer_on_tech_message && opcode == 0x244b) {
        DiploScratchFA18 = second_offer_on_message;
        mutate_second_offer_on_tech_message = 0;
    }
    return 0;
}
int __cdecl great_satan(int, int) { return great_satan_result; }
void __cdecl diplomacy_caption(int, int) { ++caption_count; }
int __cdecl parse_says(int slot, char *value, int, int) { ++parse_count; parse_values[slot] = value; return 0; }
int __cdecl parse_num(int value, int) { last_popup_value = value; parsed_energy = value; return 0; }
char *__cdecl get_pact(int) { return (char *)"PACT"; }
char *__cdecl get_pacts(int, int) { return (char *)"PACTS"; }
char *__cdecl get_phrase(int, int) { return (char *)"PHRASE"; }
char *__cdecl get_noun_phrase(int faction, int) { last_noun_faction = faction; return (char *)"NOUN"; }
int __cdecl random(int low, int high) { ++random_count; return low + high; }
char *_itoa(int value, char *out, int) { sprintf(out, "%d", value); return out; }
void __cdecl say_tech(char *out, int tech, int) { sprintf(out, "TECH%d", tech); }
int __cdecl tech_val(int tech, int, int) {
    tech_val_ids[tech_val_count++] = tech;
    return tech_value;
}
void __cdecl tech_achieved(int, int tech, int, int) {
    achieved_techs[achieved_tech_count++] = tech;
    ++tech_count; trace_events[trace_count++] = TRACE_TECH;
}
void __cdecl set_treaty(int a, int b, int c, int d) { last_treaty_args[0]=a; last_treaty_args[1]=b; last_treaty_args[2]=c; last_treaty_args[3]=d; ++treaty_count; trace_events[trace_count++] = TRACE_TREATY; }
void __cdecl treaty_off(int a, int b, int c) { last_treaty_off_args[0]=a; last_treaty_off_args[1]=b; last_treaty_off_args[2]=c; ++treaty_count; trace_events[trace_count++] = TRACE_TREATY; }
void __cdecl double_cross(int a, int b, int c) { last_double_cross_args[0]=a; last_double_cross_args[1]=b; last_double_cross_args[2]=c; ++treaty_count; trace_events[trace_count++] = TRACE_TREATY; }
void __cdecl pact_of_brotherhood(int a, int b) { last_pact_args[0]=a; last_pact_args[1]=b; ++pact_count; trace_events[trace_count++] = TRACE_PACT; }
void __cdecl help_topic(int, int) { ++help_count; }
void __cdecl check_spock() { ++spock_calls; if (switch_network_on_spock) IsMultiplayerNet = 1; }
uint32_t WINAPI timeGetTime() { ++time_calls; return (uint32_t)time_calls; }

// @@PROPOSE_PACT_ACTUAL_SOURCE@@

static void reset_fixture() {
    memset(Players, 0, sizeof Players);
    memset(PlayersData, 0, sizeof PlayersData);
    memset(&GameDataStateStorage, 0, sizeof GameDataStateStorage);
    memset(diplomacy_sprites, 0, sizeof diplomacy_sprites);
    memset(FactionRankingsUnk, 0, sizeof FactionRankingsUnk);
    memset(&ConsoleGlobal, 0, sizeof ConsoleGlobal);
    memset(&ConsoleInfoWin, 0, sizeof ConsoleInfoWin);
    memset(&NetDaemonNet, 0, sizeof NetDaemonNet);
    memset(StringTemp, 0, sizeof StringTemp);
    memset(popup_labels, 0, sizeof popup_labels);
    memset(trace_events, 0, sizeof trace_events);
    memset(message_opcodes, 0, sizeof message_opcodes);
    memset(message_tech_ids, 0, sizeof message_tech_ids);
    memset(&PlayersData[0].diplo_treaties, 0, sizeof PlayersData[0].diplo_treaties);
    for (int faction = 0; faction < 8; ++faction) {
        FactionDiplomacyPortraits[faction] = &diplomacy_sprites[faction];
        PlayersData[faction].ranking = 1;
        strcpy(Players[faction].name_leader, "LEADER");
    }
    IsMultiplayerNet = 0; PopupDialogFactionID = -1;
    GenderDefault = PluralityDefault = 0;
    ExitTurnLoop = 0; DiploFriction = 0;
    DiploFrictionFactionID = DiploFrictionFactionIDWith = 99;
    DiploScratchF7E0 = DiploScratchF7E8 = DiploScratchF7F0 = DiploScratchF7FC = 0;
    DiploScratchFA18 = DiploScratchFA24 = DiploScratchFA2C = DiploScratchFA3C = 0;
    DiploScratchFA54 = DiploScratchFA68 = DiploScratchFA78 = DiploScratchFA8C = 0;
    DiploScratchFA98 = 0; DiploScratchFAA8 = -1;
    DiploScratchFA9C = DiploScratchFA70 = DiplomacyProposalGateCount = 0;
    DiplomacyProposalMode = 1;
    popup_count = popup_result = last_popup_value = parsed_energy = 0;
    memset(popup_results, 0, sizeof popup_results);
    popup_response_count = popup_response_index = 0;
    log_count = message_count = tech_count = treaty_count = pact_count = 0;
    great_satan_result = tech_value = 0;
    random_count = help_count = parse_count = caption_count = 0;
    time_calls = spock_calls = 0; trace_count = 0;
    mutate_offer_on_callback = callback_offer = callback_second_offer = 0;
    mutate_offer_on_tech_log = tech_log_offer = 0;
    mutate_fa8c_on_callback = 0;
    mutate_second_offer_on_tech_message = second_offer_on_message = 0;
    mutate_energy_on_callback = energy_my_after_callback = energy_other_after_callback = 0;
    memset(achieved_techs, 0, sizeof achieved_techs);
    memset(tech_val_ids, 0, sizeof tech_val_ids);
    achieved_tech_count = tech_val_count = 0;
    memset(message_args, 0, sizeof message_args);
    memset(parse_values, 0, sizeof parse_values);
    memset(last_pact_args, 0, sizeof last_pact_args);
    memset(last_treaty_args, 0, sizeof last_treaty_args);
    memset(last_double_cross_args, 0, sizeof last_double_cross_args);
    memset(last_treaty_off_args, 0, sizeof last_treaty_off_args);
    last_noun_faction = -1; switch_network_on_spock = 0;
}


int main() {
    // Native 543F74..544015 converts the heuristic into an eligibility bit.
    reset_fixture();
    assert(propose_pact(1, 2) == 1 && popup_count == 0 && pact_count == 1);
    assert(last_pact_args[0] == 1 && last_pact_args[1] == 2);
    reset_fixture(); DiploScratchFA70 = 1;
    assert(propose_pact(1, 2) == 1 && popup_count == 1 && pact_count == 0);
    assert(strcmp(popup_labels[0], "REJPACT") == 0);
    reset_fixture(); PlayersData[1].diplo_agenda[2] = 0x2000;
    assert(propose_pact(1, 2) == 1 && popup_count == 1 && pact_count == 0);
    reset_fixture(); PlayersData[1].diff_level = 1; DiploScratchFA9C = 1;
    assert(propose_pact(1, 2) == 1 && popup_count == 1 && pact_count == 0);
    reset_fixture(); PlayersData[1].diff_level = 1; DiploScratchFA9C = 2;
    assert(propose_pact(1, 2) == 1 && popup_count == 0 && pact_count == 1);

    // Reject routes: native uses current FA78/FA2C but the ordinary MY column.
    for (int fa78 = 2; fa78 <= 3; ++fa78)
    for (int fa2c = 0; fa2c <= 1; ++fa2c)
    for (int fa98 = 0; fa98 <= 1; ++fa98)
    for (int friction = 2; friction <= 3; ++friction)
    for (int treaty = 0; treaty <= 1; ++treaty) {
        reset_fixture(); DiploScratchFA70 = 1;
        DiploScratchFA78 = fa78; DiploScratchFA2C = fa2c;
        DiploScratchFA98 = fa98; DiploFriction = friction;
        PlayersData[2].diplo_treaties[1] = treaty ? 0x20 : 0;
        const bool meek = (fa78 > 2 || fa2c != 0) &&
                          (fa98 != 0 || friction <= 2) && !treaty;
        const bool hate = !meek && (treaty || fa78 > 2 || fa2c != 0);
        assert(propose_pact(1, 2) == 1 && popup_count == 1);
        assert(strcmp(popup_labels[0], meek ? "REJPACTMEEK" : hate ? "REJPACTHATE" : "REJPACT") == 0);
        assert(PlayersData[2].diplo_patience[1] == (hate ? 1 : 0));
        assert(PlayersData[1].diplo_patience[1] == 0);
        assert(PlayersData[2].diplo_patience[3] == 0);
    }
    for (int my_rank = 1; my_rank <= 2; ++my_rank)
    for (int other_rank = 1; other_rank <= 2; ++other_rank) {
        reset_fixture(); DiploScratchFA70 = 1;
        PlayersData[1].ranking = my_rank; PlayersData[2].ranking = other_rank;
        assert(propose_pact(1, 2) == 1 && popup_count == 1);
        assert(strcmp(popup_labels[0], other_rank <= my_rank ? "REJPACT" : "REJPACTPOWER") == 0);
    }
    reset_fixture(); DiplomacyProposalMode = 3;
    assert(propose_pact(1, 2) == 0 && popup_count == 0 && pact_count == 0);
    reset_fixture(); DiplomacyProposalMode = 3; DiploScratchFA24 = 1;
    assert(propose_pact(1, 2) == 1 && treaty_count == 1 && pact_count == 1);

    // Independently evaluated staged IDIV and repeated gift reduction.
    const int cap_cases[][7] = {
        // my bases, other bases, my energy, my military, other military, gift, friction delta
        {0, 0, 1000, 100, 100, 500, -5},
        {4, 0, 1000, 100, 100, 600, -6},
        {0, 4, 1000, 100, 100, 400, -5},
        {0, 0, 1200, 100, 100, 600, -6},
        {0, 0, 1000, 200, 100, 300, -4},
        {0, 0, 1000, 100, 200, 650, -6},
        {10, 0, 200, 1, 100, 200, -3}
    };
    for (unsigned int c = 0; c < sizeof cap_cases / sizeof cap_cases[0]; ++c) {
        reset_fixture(); DiplomacyProposalMode = 2;
        DiploFriction = 11; DiploScratchFA78 = 1;
        PlayersData[1].current_num_bases = cap_cases[c][0];
        PlayersData[2].current_num_bases = cap_cases[c][1];
        PlayersData[1].energy_reserves = cap_cases[c][2];
        PlayersData[2].energy_reserves = 9000;
        PlayersData[1].mil_strength_1 = cap_cases[c][3];
        PlayersData[2].mil_strength_1 = cap_cases[c][4];
        PlayersData[1].tech_commerce_bonus = 8; PlayersData[2].tech_commerce_bonus = 30;
        PlayersData[2].diplo_friction[1] = 12;
        PlayersData[2].diplo_friction[3] = 19; // distinguish obsolete sum-column route
        DiploFrictionFactionID = 2; DiploFrictionFactionIDWith = 1;
        popup_result = 1;
        assert(propose_pact(1, 2) == 1 && pact_count == 1);
        assert(parsed_energy == cap_cases[c][5]);
        assert(PlayersData[1].energy_reserves == cap_cases[c][2] - cap_cases[c][5]);
        assert(PlayersData[2].energy_reserves == 9000 + cap_cases[c][5]);
        assert(propose_pact_signed_bits(PlayersData[2].diplo_friction[1]) == 12 + cap_cases[c][6]);
        assert(PlayersData[2].diplo_friction[3] == 19);
        assert(DiploFriction == 11 + cap_cases[c][6]);
    }

    // Live callback balances, saved gift; faction zero is not a bank account.
    reset_fixture(); DiplomacyProposalMode = 2; DiploFriction = 11; DiploScratchFA78 = 1;
    PlayersData[1].energy_reserves = 1000; PlayersData[2].energy_reserves = 9000;
    PlayersData[1].mil_strength_1 = PlayersData[2].mil_strength_1 = 100;
    popup_result = 1; mutate_energy_on_callback = 1;
    energy_my_after_callback = 800; energy_other_after_callback = 9200;
    assert(propose_pact(1, 2) == 1 && parsed_energy == 500);
    assert(PlayersData[1].energy_reserves == 300 && PlayersData[2].energy_reserves == 9700);
    reset_fixture(); DiplomacyProposalMode = 2; DiploFriction = 11; DiploScratchFA78 = 1;
    PlayersData[0].energy_reserves = 1000; PlayersData[2].energy_reserves = 9000;
    PlayersData[0].mil_strength_1 = PlayersData[2].mil_strength_1 = 100; popup_result = 1;
    assert(propose_pact(0, 2) == 1 && parsed_energy == 500);
    assert(PlayersData[0].energy_reserves == 1000 && PlayersData[2].energy_reserves == 9500);

    // Accepted tech is snapshotted for transfer; valuation rereads its live ID.
    reset_fixture(); DiplomacyProposalMode = 4; DiploScratchFAA8 = 3;
    PlayersData[1].energy_reserves = 123; // not part of the tech-friction formula
    popup_result = 1; mutate_offer_on_callback = 1;
    callback_offer = 5; callback_second_offer = 6; tech_value = 20;
    PlayersData[2].diplo_friction[1] = 12;
    assert(propose_pact(1, 2) == 1 && achieved_tech_count == 2);
    assert(achieved_techs[0] == 5 && achieved_techs[1] == 6);
    assert(tech_val_count == 1 && tech_val_ids[0] == 5);
    assert(last_noun_faction == 2);
    assert(PlayersData[2].diplo_friction[1] == 2);
    reset_fixture(); DiplomacyProposalMode = 4; DiploScratchFAA8 = 3; IsMultiplayerNet = 1;
    popup_result = 1; mutate_offer_on_callback = 1; callback_offer = 5; callback_second_offer = 6;
    mutate_offer_on_tech_log = 1; tech_log_offer = 7;
    assert(propose_pact(1, 2) == 1 && message_count == 2);
    assert(message_tech_ids[0] == 5 && message_tech_ids[1] == 6);
    assert(tech_val_count == 1 && tech_val_ids[0] == 7);

    // Declined help may continue directly; the forced no-help path increments
    // OTHER patience at MY index and returns its distinct original label.
    reset_fixture(); DiplomacyProposalMode = 2; DiploScratchF7E8 = 3;
    assert(propose_pact(1, 2) == 1 && pact_count == 1 && popup_count == 1);
    assert(strcmp(popup_labels[0], "GIMMEHELP") == 0 && PlayersData[2].diplo_patience[1] == 0);
    reset_fixture(); DiplomacyProposalMode = 2; DiploScratchF7E8 = 3; DiplomacyProposalGateCount = 1;
    assert(propose_pact(1, 2) == 1 && pact_count == 0 && popup_count == 2);
    assert(strcmp(popup_labels[1], "REJPACTNOHELP") == 0 && PlayersData[2].diplo_patience[1] == 1);
    reset_fixture(); DiplomacyProposalMode = 2; DiploScratchF7E8 = 3; popup_result = 1;
    assert(propose_pact(1, 2) == 1 && pact_count == 1 && treaty_count == 3);
    assert(time_calls == 1 && NetDaemonNet.unlock_count == 1);
    assert(parse_values[5] == Players[1].title_leader);
    assert(last_double_cross_args[0] == 1 && last_double_cross_args[1] == 3 && last_double_cross_args[2] == 2);
    assert(last_treaty_off_args[0] == 1 && last_treaty_off_args[1] == 2 && last_treaty_off_args[2] == 0x800000);
    assert(last_treaty_args[0] == 2 && last_treaty_args[1] == 3 && last_treaty_args[2] == 0x40 && last_treaty_args[3] == 1);
    assert(DiploScratchF7E8 == 0 && DiploScratchFA4C == 0);

    // The native retry checks add_global every iteration; a pump-side change
    // to multiplayer is read by each ensuing network writer.
    reset_fixture(); DiplomacyProposalMode = 2; DiploScratchF7E8 = 3; popup_result = 1;
    NetDaemonNet.add_global_result = 2; switch_network_on_spock = 1;
    assert(propose_pact(1, 2) == 1 && pact_count == 1 && message_count == 3);
    assert(NetDaemonNet.add_global_count == 3 && spock_calls == 2 && time_calls == 3);
    const int help_messages[3][6] = {
        {0x2440, 0, 1, 3, 2, 0}, {0x2442, 0, 1, 2, 0x800000, 0},
        {0x2443, 0, 2, 3, 0x40, 1}
    };
    assert(memcmp(message_args, help_messages, sizeof help_messages) == 0);
    assert(NetDaemonNet.await_count == 1 && NetDaemonNet.awaits[0] == 0x443);

    reset_fixture(); DiplomacyProposalMode = 2; DiploFriction = 11; DiploScratchFA78 = 1;
    PlayersData[1].energy_reserves = 1000; PlayersData[2].energy_reserves = 9000;
    PlayersData[1].mil_strength_1 = PlayersData[2].mil_strength_1 = 100;
    IsMultiplayerNet = 1; popup_result = 1;
    assert(propose_pact(1, 2) == 1 && message_count == 1);
    const int gift_message[6] = {0x2447, 0, 1, -500, 2, 500};
    assert(memcmp(message_args[0], gift_message, sizeof gift_message) == 0);
    return 0;
}
