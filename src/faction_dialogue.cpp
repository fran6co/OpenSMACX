/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "worldwin.h"
#include "general.h"

#include "time.h"
#include "win.h"
#include "faction.h"
#include "tutwin.h"
#include "alpha.h"
#include "base.h"
#include "council.h"
#include "path.h"
#include "councwin.h"
#include "popup.h"
#include "dialogs.h"
#include "text.h"
#include "game.h"
#include "map.h"
#include "probe.h"
#include "technology.h"
#include "veh.h"
#include "netmsg.h"  // NetMsgPrimary, which the diplomacy bodies pop through
#include "netdaemon.h"  // NetDaemonNet, message_data - the net_* writers
#include "log.h"  // log_say, which the net_* writers announce each move with
#include "console.h"  // ConsoleGlobal, which net_energy refreshes
#include "tutorial_script.h"
#include "socialwin.h"
#include "mapwin.h"
#include "multiwin.h"
#include "planwin.h"
#include "alphamovie.h"
#include "strings.h"
#include "xpops.h"
#include "diplopop.h"
#include "diplowin.h"
#include "infowin.h"
#include "reportif.h"
#include "random.h"
#include "replaywin.h"
#include "datalink.h"

// Private support for the original signed IMUL/SAR quotient sequences used
// by the secondary technology offer path. This mirrors x86 high-word
// multiplication and arithmetic shifts without signed-overflow UB.
static int communicate_i32_from_bits(uint32_t bits) {
    if (bits <= 0x7FFFFFFFu) return static_cast<int>(bits);
    return -1 - static_cast<int>(~bits);
}


// These are genuine local arithmetic operations, not host-only doubles.
static int communicate_wrapped_add(int left, int right) {
    return communicate_i32_from_bits(static_cast<uint32_t>(left) +
                                     static_cast<uint32_t>(right));
}
static int communicate_wrapped_sub(int left, int right) {
    return communicate_i32_from_bits(static_cast<uint32_t>(left) -
                                     static_cast<uint32_t>(right));
}
static int communicate_wrapped_mul(int left, int right) {
    return communicate_i32_from_bits(static_cast<uint32_t>(left) *
                                     static_cast<uint32_t>(right));
}
static int communicate_pending_social_model(const SocialCategory &selection,
                                            int category) {
    // No new bound: the native pending-category read is unchecked.
    return static_cast<int>(selection.models_[category]);
}

static int communicate_sar_i32(int value, unsigned shift) {
    const uint32_t bits = static_cast<uint32_t>(value);
    if (shift == 0) return value;
    uint32_t shifted = bits >> shift;
    if ((bits & 0x80000000u) != 0) shifted |= ~(0xFFFFFFFFu >> shift);
    return communicate_i32_from_bits(shifted);
}

static int communicate_imul_high_sar(int value, uint32_t multiplier,
                                     unsigned shift, int adjustment) {
    const int64_t product = static_cast<int64_t>(value) * multiplier;
    const int64_t word_scale = static_cast<int64_t>(65536) * 65536;
    int64_t high = product / word_scale;
    if (product < 0 && product % word_scale != 0) --high;
    int quotient = communicate_sar_i32(
        communicate_i32_from_bits(static_cast<uint32_t>(high)), shift);
    quotient = communicate_wrapped_add(quotient,
        (static_cast<uint32_t>(quotient) >> 31));
    return communicate_wrapped_add(quotient, adjustment);
}

// Complete retained communicate continuation; the prior unfinished
// binding/history below remains a historical restoration obligation.
int communicate_round_down_multiple(int value, int step);

// ORIGINAL: 0x0054FFD0 ?communicate@@YAXHHH@Z 0x0054FFD0-0x00558980;0x0065FF4F-0x0065FF64
// RECOVERY_BINDING: v1 {"evidence": ["original:0x0054FFD0-0x00558980;0x0065FF4F-0x0065FF64", "source:src/unrecovered/0054ffd0.cpp:retained-seed", "task:base-colony-economy-coverage-wave:diplomacy-communicate-link-continuity-20261003", "Retired seed src/unrecovered/0054ffd0.cpp retains its complete original identity, hot/cold spans and historical findings in this owner", "SocialEffect.values_ is the genuine eleven-word indexed owner; original out-of-domain selector effects remain unresolved", "recovery/tests/test_communicate_owner.py: actual-source bounded behavior and mutation controls", "recovery/tasks/owner-coverage-wave-oct6.json"], "reason": "Recover the complete diplomacy conversation, topic and action paths through real faction, Base, social-effect array and UI owners, preserving original unchecked selectors, arithmetic, error and uninitialized-state behavior. Prior unfinished decision retained as history: Explicitly unfinished owner body for link continuity while complete communicate hot/cold behavior remains under recovery. The seed and original mapping are retained; this empty body does not implement diplomacy, reproduce its bytes or satisfy the pending dependency."}
// TRIED: the seven-way relationship-topic greeting text build (war/ vendetta/truce/treaty/pact, chosen via has_treaty bits and distance, each with its own get_adjective_leader/ get_noun_phrase string chain and jump-table case 0-6) was collapsed to one generic diplomacy_caption() + say_num(0) call. The diplomacy_menu() result loop and its 8-way action dispatch (cases 0/1-2/3/4/5/6/7) were read from the raw disassembly (no Ghidra hypothesis was reliable there either) and reproduced with real callee names/order; the patience/ loop-exit condition nesting is approximated, not exact.
void __cdecl communicate(int faction_id, int faction_id_with, int mode) {
// Single shared function-scope declarations for the private whole-body
// candidate and typed host harness. They are not an installed source edit.
const bool contact_flags = mode != 0;
int communicate_stack_c = faction_id_with; // original EBP+C argument slot
int communicate_stack_2c;
int communicate_stack_14, communicate_stack_1c;
int communicate_stack_20, communicate_stack_34;
int communicate_stack_38, communicate_stack_3c;
int communicate_stack_44, communicate_stack_48;
int communicate_stack_4c, communicate_stack_50;
int communicate_stack_54;
int communicate_territory_base_id;
int communicate_getout_result;
char communicate_getout_number[12];
int communicate_peer_to_actor_treaty;
int communicate_actor_commerce_threshold;
int communicate_ally_score;
int communicate_bribe_phrase_amount, communicate_captured_base_index;
int communicate_captured_base_offer_result;
int communicate_captured_base_population_limit;
int communicate_captured_base_scan;
int communicate_custom_amount, communicate_custom_tech_count;
int communicate_custom_tech_value, communicate_custom_text_open_result;
int communicate_demand_tech_popup_result, communicate_demand_topic_code;
bool communicate_holds_treaty_with_peer;
int communicate_intro_amount, communicate_intro_candidate;
int communicate_large_offer_capacity, communicate_large_offer_capacity_limit;
int communicate_large_offer_count, communicate_large_offer_limit;
int communicate_large_offer_popup_result, communicate_large_offer_scaled;
int communicate_large_offer_target, communicate_large_offer_target_plus_100;
int communicate_large_offer_text_variant, communicate_large_offer_total;
int communicate_late_contact_base_id, communicate_late_contact_best;
int communicate_late_contact_best_id, communicate_late_contact_best_tech;
int communicate_late_contact_best_tech_value;
int communicate_late_contact_bully_random, communicate_late_contact_choice;
int communicate_late_contact_general_region_hit;
int communicate_late_contact_general_rival;
int communicate_late_contact_general_strength;
const char *communicate_late_contact_hate_caption;
int communicate_late_contact_rank;
int communicate_late_contact_region, communicate_late_contact_region_reported;
int communicate_late_contact_region_rival;
int communicate_late_contact_region_strength;
int communicate_late_contact_research_tech, communicate_late_contact_response_mode;
int communicate_late_contact_tech, communicate_late_contact_tech_mask;
int communicate_late_contact_tech_mask_offset;
int communicate_late_contact_text_result;
int communicate_late_contact_vendetta_response;
int communicate_vendetta_text_variant;
int communicate_late_offer_amount, communicate_late_offer_eligible;
int communicate_late_offer_range;
int communicate_late_offer_special;
int communicate_late_score;
int communicate_late_score_other, communicate_late_score_peer_base_count;
int communicate_late_score_received_count, communicate_late_score_region;
int communicate_late_score_shared_count, communicate_late_social_slot;
int communicate_late_tech_difference, communicate_late_tech_id;
int communicate_late_truce_research_value;
int communicate_late_truce_tech_amount;
int communicate_late_truce_tech_popup_result;
int communicate_truce_random_limit;
int communicate_truce_response_result, communicate_truce_variant;
int communicate_truce_followup_limit, communicate_truce_followup_roll;
int communicate_local_menu_mode, communicate_map_mask;
int communicate_menu_choice, communicate_native_ebx_at_55304a;
int communicate_native_had_pact;
int communicate_peer_strength_third;
int communicate_plea_target;
int communicate_positive_reserves, communicate_relation_auxiliary_result;
int communicate_relation_choice, communicate_relation_offer_multiplier;
int communicate_relation_offer_tech_value, communicate_relation_offer_turn_scale;
int communicate_relation_offer_value, communicate_relation_popup_result;
int communicate_relation_selected_amount, communicate_rival_faction;
int communicate_secondary_action, communicate_secondary_best_tech;
int communicate_secondary_best_tech_value, communicate_secondary_offer_amount;
int communicate_secondary_target, communicate_secondary_tech_scan;
int communicate_social_category, communicate_social_model;
int communicate_special_war_region;
int communicate_special_war_selected_base_id;
int communicate_special_war_smallest_population;
int communicate_tertiary_mask, communicate_tertiary_reserve_limit;
int communicate_treaty_topic_result, communicate_zero_peer_energy;
int communicate_zero_tech_id;
int communicate_actor_reserves;
char communicate_demand_topic_suffix[2];
char communicate_demand_topic_number[10];
char *communicate_patience_ptr;

const int peer_id = faction_id_with;
// Mutable whole-function composition of the pinned communicate translation.
// This is private WIP, not accepted or byte-exact. Included inside a function
// scope after its native-entry locals have been declared.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00550000..0x005500C6 only.
// Insert at function scope in communicate(); the Popup's lifetime must
// continue through the normal 0x0055896A destructor and cold SEH funclet.
communicate_stack_20 = 0; // native 0x54FFFD
communicate_stack_38 = 0; // native [EBP-0x38]
communicate_stack_3c = 0;
communicate_stack_54 = 0;
communicate_stack_48 = 0;
communicate_stack_34 = 0; // native [EBP-0x34]
communicate_stack_1c = 0; // native [EBP-0x1C]
Popup popup;
PopupDialogFactionID = peer_id;
DiplomacyActive = 1;
// The native clears these two independently accessed cells at 0x0055003B
// and 0x00550080. Their neutral source bindings are start-only; do not infer
// a shared containing object, allocation extent or semantic relationship.
DiploScratchFA8C = 0;
const uint32_t peer_to_faction_treaty =
    PlayersData[peer_id].diplo_treaties[faction_id];
const bool early_contact_path =
    (peer_to_faction_treaty & 0x08000000u) == 0 ||
    (IsMultiplayerNet && (FactionDiplomacyFlags[peer_id] & 2u) != 0);
communicate_stack_44 = early_contact_path; // native 0x550078
DiploScratchFA48 = 0;
const uint32_t faction_to_peer_treaty =
    PlayersData[faction_id].diplo_treaties[peer_id];
communicate_stack_50 =
    (faction_to_peer_treaty & 4u) != 0 &&
    (faction_to_peer_treaty & 0x00010000u) == 0 ? 1 : 0;
// Native stores this predicate in [EBP-0x50]; later offer/relation branches
// consume the same live function-local flag.
communicate_stack_14 = 0; // native 0x5500B4
DiplomacyContactResult = contact_flags != 0 ? 3 : 0;
// INCOMPLETE BOUNDARY: native 0x005500C6 branches on early_contact_path.
// Its following ideological/contact-score block has not yet been translated.
// Do not treat this snippet as a whole body or a compilable completed unit.
// INCOMPLETE PRIVATE BODY SECTION: the native conditional at 0x005500C6.
// [EBP-0x44] compares against the prologue's zero register; early_contact_path
// is true exactly when native ECX is nonzero. The next instruction is the
// reciprocal category/model block at 0x005500CC.
if (!early_contact_path) {
    goto communicate_after_friction; // native JE 0x0055018D
}
// INCOMPLETE PRIVATE BODY SECTION: native 0x005500CC..0x0055013C.
// The entry-local early_contact_path is the branch predicate saved at
// native [EBP-0x44]. The category/model accesses below stay within the
// existing Player/PlayerData owners and preserve the original comparison
// order. The reciprocal social-effect check follows at 0x55013C.
communicate_friction_checks:
if (!early_contact_path) {
    goto communicate_after_friction; // native 0x005500C6 -> 0x0055018D
}
{
    const int category = Players[peer_id].soc_ideology_category;
    if (category >= 0) {
        const int model = Players[peer_id].soc_ideology_model;
        if (model != 0) {
            const int pending_model =
                static_cast<int>(PlayersData[faction_id]
                                     .soc_category_pending.models_[category]);
            if (pending_model != 0 && pending_model != model) {
                const uint32_t sum_bits =
                    PlayersData[peer_id].diplo_friction[faction_id] + 3u;
                const int sum = sum_bits <= 0x7FFFFFFFu
                    ? static_cast<int>(sum_bits)
                    : -1 - static_cast<int>(~sum_bits);
                PlayersData[peer_id].diplo_friction[faction_id] =
                    static_cast<uint32_t>(range(sum, 0, 20));
                if (DiploFrictionFactionID == static_cast<uint32_t>(peer_id) &&
                    DiploFrictionFactionIDWith ==
                        static_cast<uint32_t>(faction_id)) {
                    const uint32_t friction_bits =
                        static_cast<uint32_t>(DiploFriction) + 3u;
                    DiploFriction = friction_bits <= 0x7FFFFFFFu
                        ? static_cast<int>(friction_bits)
                        : -1 - static_cast<int>(~friction_bits);
                }
            }
        }
    }
}
// Fall through to the reciprocal social-effect check at native 0x0055013C.
// INCOMPLETE PRIVATE BODY SECTION: native 0x0055013C..0x0055018D.
// This is the reciprocal pending-effect gate following
// friction-category-check-500cc.inc. Native indexes SocialEffect by the
// Player's nonnegative effect selector without an added range check. The
// genuine array owner preserves the indexed word operation. The original
// has no upper bound; selectors outside 0..10 remain unsafe native-effect
// obligations. Do not substitute a pointer-member table or safety fallback.
communicate_secondary_friction_path: // native 0x0055013C
{
    const int effect_index = Players[peer_id].soc_ideology_effect;
    if (effect_index >= 0) {
        const int pending_effect =
            PlayersData[faction_id].soc_effect_pending.values_[effect_index];
        if (pending_effect < 0) {
            const uint32_t sum_bits =
                PlayersData[peer_id].diplo_friction[faction_id] + 3u;
            const int sum = sum_bits <= 0x7FFFFFFFu
                ? static_cast<int>(sum_bits)
                : -1 - static_cast<int>(~sum_bits);
            PlayersData[peer_id].diplo_friction[faction_id] =
                static_cast<uint32_t>(range(sum, 0, 20));
            if (DiploFrictionFactionID == static_cast<uint32_t>(peer_id) &&
                DiploFrictionFactionIDWith == static_cast<uint32_t>(faction_id)) {
                const uint32_t friction_bits =
                    static_cast<uint32_t>(DiploFriction) + 3u;
                DiploFriction = friction_bits <= 0x7FFFFFFFu
                    ? static_cast<int>(friction_bits)
                    : -1 - static_cast<int>(~friction_bits);
            }
        }
    }
}
// Fall through to diplomacy_check at native 0x0055018D.
// INCOMPLETE PRIVATE BODY SECTION: native 0x0055018D..0x005501DE.
// Live-ins: faction_id, peer_id, contact_flags, pending_pair_predicate,
// and the automatic Popup constructed in opening-50000-500c6.inc.
// The labels below are continuation points for later sections of the same
// communicate() body, not helper functions or independent exits.
communicate_after_friction: // native 0x0055018D
diplomacy_check(faction_id, peer_id, 1);
const uint32_t dialogue_flags_bits = FactionDiplomacyFlags[peer_id];
const int dialogue_flags = dialogue_flags_bits <= 0x7FFFFFFFu
    ? static_cast<int>(dialogue_flags_bits)
    : -1 - static_cast<int>(~dialogue_flags_bits);
FactionDiplomacyFlags[peer_id] = 0;
communicate_stack_4c = 0; // native 0x5501A3
if ((dialogue_flags & 0x2Cu) == 0) {
    goto communicate_main_dialogue_path; // native 0x00550A8B
}
if ((dialogue_flags & 0x24u) != 0) {
    DiplomacyContactResult = 8;
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_cleanup; // native 0x005587C8
}
if ((dialogue_flags & 4u) != 0) {
    goto communicate_territory_path; // native 0x005501DE
}
goto communicate_nonterritory_path; // native 0x00550848
// INCOMPLETE BOUNDARY: both continuations and the Popup cleanup path are
// supplied by later regions; this section alone is not a complete function.
communicate_territory_path: // native 0x005501DE
{
// Corrected private native translation for 0x005501DE..0x00550283.
// Function-scope automatic required by 0x00550283's next label:
//   int communicate_territory_base_id;
communicate_territory_base_id = -1;
territory(faction_id, peer_id, 0, &communicate_territory_base_id, 0);
diplomacy_caption(faction_id, peer_id);
Base &communicate_territory_base = Bases[communicate_territory_base_id];
if (communicate_territory_base.faction_id_current != peer_id &&
    (communicate_territory_base.visibility & (1u << peer_id)) == 0) {
    spot_base(communicate_territory_base_id, peer_id);
    draw_tile(communicate_territory_base.x, communicate_territory_base.y, 2);
    ConsoleGlobal.focus(communicate_territory_base.x,
                        communicate_territory_base.y, peer_id);
}
// Fall through to communicate_peer_dialogue_setup at 0x00550283.
// Composition-only lexical repair of the prior exact fragment: the popup
// result is a function-scope native stack local and is assigned here so later
// CFG labels can be entered without bypassing a C++ initializer.
parse_says(0, Bases[communicate_territory_base_id].name_string, -1, -1);
parse_says(1, Players[faction_id].title_leader, -1, -1);
parse_says(2, Players[faction_id].name_leader, -1, -1);
parse_says(3, Players[peer_id].title_leader, -1, -1);
parse_says(4, Players[peer_id].name_leader, -1, -1);
parse_says(5, Players[peer_id].adj_name_faction, -1, -1);
parse_says(6, Players[faction_id].adj_name_faction, -1, -1);
StringTemp[0] = 0;
strcat(StringTemp, "GETOUT");
_itoa(static_cast<int>(random(0, 5)) + 1,
      communicate_getout_number, 10);
strcat(StringTemp, communicate_getout_number);
communicate_getout_result = X_pops(
    StringTemp, 0x00100000, FactionDiplomacyPortraits[peer_id], pop_wait);
}
// Private native translation for the x_pops_short result==0 path at
// 0x005503E9..0x00550497. The result and dialogue_flags are function-scope
// locals set by the immediately preceding/earlier native sections.
if (communicate_getout_result == 0) {
    log_say("Getting DIPLOMATIC LOCK", 0, 0, 0);
    (void)timeGetTime();
    while (ExitTurnLoop == 0) {
        if (NetDaemonNet.add_global() == 0) {
            break;
        }
        check_spock();
        (void)timeGetTime();
    }
    if (IsMultiplayerNet != 0) {
        log_say("Sending WITHDRAWAL", peer_id, faction_id, 0);
        message_data(0x244A, 0, peer_id, faction_id, 0, 0);
        NetDaemonNet.await_diplo(0x44A);
    } else {
        do_withdrawal(peer_id, faction_id);
    }
    log_say("Releasing DIPLOMATIC LOCK", 0, 0, 0);
    NetDaemonNet.unlock();
    if ((dialogue_flags & 3) == 0) {
        goto communicate_cleanup; // native 0x005587C8
    }
    goto communicate_main_dialogue_path; // native 0x00550A8B
}
// Nonzero results continue into the independent result==1/other paths.
// Composition-only lexical repair of the result==1 gates: values which must
// remain live in this branch are function-scope locals assigned before use,
// not initializers which a native interior goto could jump past.
if (communicate_getout_result == 1) {
    if (DiploScratchFA60 == 0) {
        goto communicate_getout_result_one_lock_path;
    }
    communicate_peer_to_actor_treaty =
        PlayersData[peer_id].diplo_treaties[faction_id];
    if ((communicate_peer_to_actor_treaty & 0x00000800u) != 0 ||
        DiploScratchFA98 != 0) {
        goto communicate_getout_result_one_lock_path;
    }
    if ((communicate_peer_to_actor_treaty & 0x20u) != 0) {
        goto communicate_getout_result_one_lock_path;
    }
    communicate_actor_commerce_threshold = communicate_wrapped_mul(
        communicate_wrapped_sub(5, PlayersData[faction_id].ai_fight), 3);
    if (DiploFriction < communicate_actor_commerce_threshold) {
        goto communicate_getout_result_one_lock_path;
    }
    if (PlayersData[faction_id].mil_strength_1 <=
        PlayersData[peer_id].mil_strength_1) {
        goto communicate_getout_result_one_lock_path;
    }
    communicate_stack_38 = 1;
communicate_introduction_conversation: // native 0x00550522
communicate_contact_entry_exit:
    if (DiploScratchF7E4 < 0) {
        goto communicate_late_contact_vendetta; // native 0x0055559CB
    }
communicate_contact_entry_exit_alt: // native 0x00550529
    if (mode != 0 || has_treaty(peer_id, faction_id, 0x2000) != 0 ||
        has_agenda(peer_id, faction_id, 4) != 0) {
        goto communicate_late_contact_vendetta; // native 0x0055559CB
    }
    goto communicate_late_contact_vendetta_after_set_mode; // native 0x0055559D0
}
// Private native translation for the result==1 diplomatic-lock/withdrawal
// path at 0x0055056A..0x00550679. This is entered by explicit gates at
// 0x005504A5..0x00550519. The native intentionally performs two distinct
// withdrawal request/owner passes with opposite faction argument order.
communicate_getout_result_one_lock_path:
log_say("Getting DIPLOMATIC LOCK", 0, 0, 0);
(void)timeGetTime();
while (ExitTurnLoop == 0) {
    if (NetDaemonNet.add_global() == 0) {
        break;
    }
    check_spock();
    (void)timeGetTime();
}
if (IsMultiplayerNet != 0) {
    log_say("Sending WITHDRAWAL", peer_id, faction_id, 0);
    message_data(0x244A, 0, peer_id, faction_id, 0, 0);
    NetDaemonNet.await_diplo(0x44A);
} else {
    do_withdrawal(peer_id, faction_id);
}
if (IsMultiplayerNet != 0) {
    log_say("Sending WITHDRAWAL", faction_id, peer_id, 0);
    message_data(0x244A, 0, faction_id, peer_id, 0, 0);
    NetDaemonNet.await_diplo(0x44A);
} else {
    do_withdrawal(faction_id, peer_id);
}
log_say("Releasing DIPLOMATIC LOCK", 0, 0, 0);
NetDaemonNet.unlock();
X_pops("BOTHOUT", 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
if ((dialogue_flags & 3) == 0) {
    goto communicate_cleanup; // native 0x005587C8
}
goto communicate_main_dialogue_path; // native 0x00550A8B
// Private native translation for the independent nonzero/nonone popup-result
// friction update at 0x0055067E..0x005506B8.
{
    const int communicate_getout_friction = range(
        communicate_wrapped_add(PlayersData[peer_id].diplo_friction[faction_id], 10), 0, 20);
    PlayersData[peer_id].diplo_friction[faction_id] =
        static_cast<uint32_t>(communicate_getout_friction);
    if (DiploFrictionFactionID == static_cast<uint32_t>(peer_id) && DiploFrictionFactionIDWith == static_cast<uint32_t>(faction_id)) {
        DiploFriction = communicate_wrapped_add(DiploFriction, 10);
    }
}
// Continue to the separate diplomatic-lock/double-cross branch at 0x5506B8.
// Private native translation for the result-other diplomatic-lock and
// double-cross action at 0x005506B8..0x0055075E.
communicate_getout_result_other_lock_path: // native 0x005506B8
log_say("Getting DIPLOMATIC LOCK", 0, 0, 0);
(void)timeGetTime();
while (ExitTurnLoop == 0) {
    if (NetDaemonNet.add_global() == 0) {
        break;
    }
    check_spock();
    (void)timeGetTime();
}
if (IsMultiplayerNet != 0) {
    log_say("Sending DOUBLE CROSS", faction_id, peer_id, -1);
    message_data(0x2440, 0, faction_id, peer_id, -1, 0);
    NetDaemonNet.await_diplo(0x440);
} else {
    double_cross(faction_id, peer_id, -1);
}
log_say("Releasing DIPLOMATIC LOCK", 0, 0, 0);
NetDaemonNet.unlock();
// The subsequent parser/string path begins at native 0x0055075E; slot 3's
// native source type remains separately under review.
// Private typed translation of the result-other outrage response at
// 0x0055075E..0x00550848. This follows the lock/double-cross action and
// rejoins the common dialogue epilogue after the popup.
// Native repeatedly installs the peer's leader-female flag as the parser's
// default gender before each slot update. The fourth call consumes the
// function's reused second-argument slot, which the entry path overwrote with
// Players[peer_id].adj_insult_faction at 0x0055036A.
GenderDefault = Players[peer_id].is_leader_female;
parse_says(0, Players[peer_id].title_leader, -1, -1);
GenderDefault = Players[peer_id].is_leader_female;
parse_says(1, Players[peer_id].name_leader, -1, -1);
GenderDefault = Players[peer_id].is_leader_female;
parse_says(2, Players[peer_id].insult_leader, -1, -1);
GenderDefault = Players[peer_id].is_leader_female;
parse_says(3, Players[peer_id].adj_insult_faction, -1, -1);
StringTemp[0] = 0;
strcat(StringTemp, "OUTRAGE");
_itoa(static_cast<int>(random(0, 3)) + 1,
      communicate_getout_number, 10);
strcat(StringTemp, communicate_getout_number);
X_pops(StringTemp, 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
goto communicate_final_cleanup; // native 0x00550843 -> 0x005587C8
// Private translation of the independent nonterritory topic flags at
// 0x00550848..0x00550A82. The two bits have separate directionality and
// continuations: native bit 0x08 performs the GOAWAY topic and exits to the
// common epilogue; bit 0x20 performs the INOURTERRITORY2 topic and rejoins
// the main dialogue path. Do not combine their territory() calls.
communicate_nonterritory_path: // native 0x00550848
if ((dialogue_flags & 8) != 0) {
    communicate_territory_base_id = -1;
    territory(peer_id, faction_id, 0, &communicate_territory_base_id, 0);
    if ((dialogue_flags & 0x10) != 0) {
        if (IsMultiplayerNet != 0) {
            log_say("Sending SET TREATY", faction_id, peer_id, 0x400);
            message_data(0x2443, 0, faction_id, peer_id, 0x400, 1);
            NetDaemonNet.await_diplo(0x443);
        } else {
            set_treaty(faction_id, peer_id, 0x400, 1);
        }
    }
    Base &communicate_nonterritory_base =
        Bases[communicate_territory_base_id];
    if (communicate_nonterritory_base.faction_id_current != faction_id &&
        (communicate_nonterritory_base.visibility &
         (1u << (faction_id & 31))) == 0) {
        spot_base(communicate_territory_base_id, faction_id);
    }
    draw_tile(communicate_nonterritory_base.x,
              communicate_nonterritory_base.y, 2);
    ConsoleGlobal.focus(communicate_nonterritory_base.x,
                        communicate_nonterritory_base.y, faction_id);
    parse_says(0, communicate_nonterritory_base.name_string, -1, -1);
    StringTemp[0] = 0;
    strcat(StringTemp, "GOAWAY");
    _itoa(static_cast<int>(random(0, 2)) + 1,
          communicate_getout_number, 10);
    strcat(StringTemp, communicate_getout_number);
    X_pops(StringTemp, 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
    goto communicate_final_cleanup; // native 0x005509C2 -> 0x00550A82 -> 0x005587C8
}
if ((dialogue_flags & 0x20) != 0) {
    communicate_territory_base_id = -1;
    territory(faction_id, peer_id, 0, &communicate_territory_base_id, 0);
    if (communicate_territory_base_id >= 0) {
        Base &communicate_peer_territory_base =
            Bases[communicate_territory_base_id];
        if (communicate_peer_territory_base.faction_id_current != peer_id &&
            (communicate_peer_territory_base.visibility &
             (1u << (peer_id & 31))) == 0) {
            spot_base(communicate_territory_base_id, peer_id);
            draw_map(1);
        }
    }
    // The native negative-ID edge still reaches this source load; preserve
    // that control flow rather than adding a new invalid-ID guard.
    parse_says(0, Bases[communicate_territory_base_id].name_string, -1, -1);
    parse_says(1, Players[peer_id].adj_name_faction, -1, -1);
    X_pops("INOURTERRITORY2", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
}
// Native 0x00550A82 rejoins communicate_main_dialogue_path at 0x00550A8B.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00550A8B..0x00550BB7.
// Live-ins: faction_id, peer_id, contact_flags, dialogue_flags and the same
// automatic Popup. This is the zero-contact-flags branch; nonzero flags
// continue separately at native 0x00550BBC.
communicate_main_dialogue_path: // native 0x00550A8B
if (contact_flags != 0) {
    goto communicate_contact_flags_path; // native 0x00550BBC
}
PlayersData[faction_id].diplo_spoke[peer_id] = GameDataStateStorage.fields_.turn_current_num_;
PlayersData[peer_id].diplo_patience[faction_id] = 0;
if (IsMultiplayerNet) {
    log_say("Sending TREATY OFF", faction_id, peer_id, 0x04000000);
    message_data(0x2442, 0, faction_id, peer_id, 0x04000000, 0);
    NetDaemonNet.await_diplo(0x442);
} else {
    treaty_off(peer_id, faction_id, 0x04000000);
}
if (IsMultiplayerNet) {
    log_say("Sending AGENDA OFF", faction_id, peer_id, 0x100);
    message_data(0x2445, 0, faction_id, peer_id, 0x100, 0);
    NetDaemonNet.await_diplo(0x445);
} else {
    agenda_off(peer_id, faction_id, 0x100);
}
if ((PlayersData[peer_id].diplo_treaties[faction_id] & 0x00010000u) != 0 ||
    dialogue_flags >= 4) {
    goto communicate_after_intro; // native 0x00550E18
}
if (IsMultiplayerNet) {
    log_say("Sending TREATY OFF", faction_id, peer_id, 4);
    message_data(0x2442, 0, faction_id, peer_id, 4, 0);
    NetDaemonNet.await_diplo(0x442);
} else {
    treaty_off(peer_id, faction_id, 4);
}
goto communicate_after_intro; // native 0x00550E18
// INCOMPLETE BOUNDARY: the nonzero contact-flags branch at 0x550BBC and the
// continuation at 0x550E18 remain separate, ordered regions of this body.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00550BBC..0x00550C2D.
// This is the separate nonzero contact_flags path from 0x550A90. The treaty
// test is the faction->peer direction, and the typed pair fields are re-read
// before this path's later parser/callback sequence.
communicate_contact_flags_path: // native 0x00550BBC
if ((PlayersData[faction_id].diplo_treaties[peer_id] & 0x00002000u) != 0) {
    PlayersData[peer_id].diplo_patience[faction_id] = 0;
    if (IsMultiplayerNet) {
        log_say("Sending TREATY OFF", faction_id, peer_id, 0x04000000);
        message_data(0x2442, 0, faction_id, peer_id, 0x04000000, 0);
        NetDaemonNet.await_diplo(0x442);
    } else {
        treaty_off(peer_id, faction_id, 0x04000000);
    }
}
communicate_contact_flags_continuation: // native 0x00550C2D
// INCOMPLETE BOUNDARY: peer/faction parser setup, patience thresholds and the
// popup result routes continue at 0x550C2D.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00550C2D..0x00550C97.
// Continuation of contact-flags-treaty-path-50bbc.inc. Preserve the current
// faction's leader-gender default and its explicit plurality reset before
// each parse_says call, then branch on the entry-local 0x550044 route flag.
GenderDefault = Players[faction_id].is_leader_female;
PluralityDefault = FALSE;
parse_says(2, Players[faction_id].title_leader, -1, -1);
GenderDefault = Players[faction_id].is_leader_female;
PluralityDefault = FALSE;
parse_says(3, Players[faction_id].name_leader, -1, -1);
if (early_contact_path) {
    goto communicate_native_predicate_path; // native 0x00550E0B
}
goto communicate_patience_gate; // native 0x00550C97
// INCOMPLETE BOUNDARY: the patience-threshold calculations beginning at
// 0x550C97 and the 0x550E0B continuation remain untranslated.
{
// INCOMPLETE PRIVATE BODY SECTION: native 0x00550C97..0x00550D1F.
// The native route predicate early-outs before the treaty-specific patience
// calculation. Signed byte loads and the negative /8 truncation are explicit.
communicate_patience_gate: // native 0x00550C97
if (early_contact_path) {
    goto communicate_native_predicate_path; // native 0x00550C91 -> 0x00550E0B
}
const uint32_t peer_treaty = PlayersData[peer_id].diplo_treaties[faction_id];
int patience_threshold;
if ((peer_treaty & 0x10u) != 0) {
    patience_threshold = 1;
} else if ((peer_treaty & 1u) != 0) {
    patience_threshold = (peer_treaty & 0x02000000u) != 0 ? 500 : 6;
} else {
    const uint32_t friction_plus_three_bits =
        static_cast<uint32_t>(DiploFriction) + 3u;
    const int friction_plus_three = friction_plus_three_bits <= 0x7FFFFFFFu
        ? static_cast<int>(friction_plus_three_bits)
        : -1 - static_cast<int>(~friction_plus_three_bits);
    const uint32_t magnitude = friction_plus_three < 0
        ? 0u - static_cast<uint32_t>(friction_plus_three)
        : static_cast<uint32_t>(friction_plus_three);
    const int quotient = static_cast<int>(magnitude / 8u) *
        (friction_plus_three < 0 ? -1 : 1);
    patience_threshold = 3 + static_cast<int>((peer_treaty >> 1) & 1u) - quotient;
}
const int current_patience = static_cast<signed char>(
    PlayersData[peer_id].diplo_patience[faction_id]);
if (current_patience < patience_threshold && (peer_treaty & 0x140u) == 0) {
    goto communicate_native_predicate_path; // native 0x00550D00 -> 0x00550E0B
}
communicate_after_patience: // native 0x00550D0C
if (DiplomacyEndState == 0) {
    diplomacy_caption(faction_id, peer_id);
}
communicate_after_patience_caption: // native 0x00550D1F
// INCOMPLETE BOUNDARY: treaty response parsing/popup and the separate
// ending branch begin here.
;
}
// INCOMPLETE PRIVATE BODY SECTION: native 0x00550D1F..0x00550E0B.
// The treaty branch uses the peer's leader strings; the no-treaty branch uses
// the peer's faction noun plus its gender/plurality defaults. These are two
// distinct native popups and both route to the same outer Popup cleanup.
if ((PlayersData[faction_id].diplo_treaties[peer_id] & 3u) != 0) {
    GenderDefault = Players[peer_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(0, Players[peer_id].title_leader, -1, -1);
    GenderDefault = Players[peer_id].is_leader_female;
    PluralityDefault = FALSE;
    parse_says(1, Players[peer_id].name_leader, -1, -1);
    (void)X_pops("NOPATIENCEFRIEND", 0x100000,
                       &FactionsArt[peer_id].diplomacy, pop_wait);
    goto communicate_cleanup; // native 0x00550DAA -> 0x005587C8
}
GenderDefault = Players[peer_id].noun_gender;
PluralityDefault = Players[peer_id].is_noun_plural;
parse_says(0, Players[peer_id].noun_faction, -1, -1);
(void)X_pops("NOPATIENCE", 0x100000,
                   &FactionsArt[peer_id].diplomacy, pop_wait);
goto communicate_cleanup; // native 0x00550E06 -> 0x005587C8
// INCOMPLETE BOUNDARY: the separate wants_to_speak/treaty continuation begins
// at 0x00550E0B and is in contact-introduction-dispatch-50e0b.inc.
{
// INCOMPLETE PRIVATE BODY SECTION: native 0x00550E0B..0x00550EAD.
// This follows the typed contact-patience popup exits. Every branch keeps its
// native treaty direction and callback order; the long proposal/UI continuations
// are separate unfinished regions.
communicate_native_predicate_path: // native 0x00550E0B
(void)wants_to_speak(faction_id, peer_id);
communicate_after_intro: // native 0x00550E18
communicate_after_contact_introduction:
const uint32_t peer_treaty_after_callback =
    PlayersData[peer_id].diplo_treaties[faction_id];
if ((peer_treaty_after_callback & 0x00040000u) != 0) {
    DiploScratchFA60 = 1;
    DiploScratchFA88 = -1;
    goto communicate_introduction_conversation; // native 0x00550522
}
if ((peer_treaty_after_callback & 1u) == 0) {
    goto communicate_unpacted_conversation; // native 0x0055119E
}
if ((peer_treaty_after_callback & 0x02000000u) != 0) {
    tech_trade(faction_id, peer_id, 1, 4, 4);
    goto communicate_after_trade; // native 0x005571E7
}
if (DiploScratchFA64 != 0 && DiploScratchFA90 != 0) {
    DiploScratchF7E8 = 0;
}
tech_analysis(faction_id, peer_id);
if ((dialogue_flags & 0x40) == 0) {
    goto communicate_no_tech_greeting; // native 0x0055178F
}
if (DiploScratchF7E8 == 0 || wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_tech_greeting_done; // native 0x0055362D
}
// Native 0x00550EB3..0x00551168 continuation: strength/reserve gates,
// plea parser and popup, then the accepted-response lock wait. The callback
// result and later pair-state reads remain in their original order.
}
// Private typed source candidate for native 0x00550EB3..0x00551168.
// This joins the already recovered phrase block at 0x005513CA for a failed
// gate or a zero popup result, and enters no-pact/pact handling at 0x00551168
// after a nonzero result and the native lock wait.
// Keep these POD locals declared before the early native gotos, since their
// continuations are labels within the same whole-function body.
int communicate_peer_reserves;
int communicate_plea_amount;
int communicate_plea_result;
int communicate_plea_tech;
int communicate_plea_payment;
if (PlayersData[DiploScratchF7E8].mil_strength_1 <
    communicate_wrapped_mul(PlayersData[peer_id].mil_strength_1, 2)) {
    goto communicate_treaty_topic_phrases; // native 0x00550EEE -> 0x005513CA
}
communicate_peer_reserves = PlayersData[peer_id].energy_reserves;
if (communicate_peer_reserves < 10) {
    goto communicate_treaty_topic_phrases; // native 0x00550EFD -> 0x005513CA
}
communicate_plea_amount = communicate_peer_reserves / 50 * 25;
if (communicate_plea_amount < 25) {
    communicate_plea_amount = communicate_peer_reserves < 25
        ? communicate_peer_reserves : 25;
}
parse_num(0, communicate_plea_amount);
PluralityDefault = 0;
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
parse_says(0, Players[DiploScratchF7E8].title_leader, -1, -1);
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
parse_says(1, Players[DiploScratchF7E8].name_leader, -1, -1);
GenderDefault = Players[DiploScratchF7E8].noun_gender;
PluralityDefault = Players[DiploScratchF7E8].is_noun_plural;
parse_says(2, Players[DiploScratchF7E8].noun_faction, -1, -1);
parse_says(3, get_pact_hood(faction_id, peer_id), -1, -1);
parse_says(5, get_pact_hood(DiploScratchF7E8, faction_id), -1, -1);
parse_says(6, get_his_her(DiploScratchF7E8, 0), -1, -1);
parse_says(8, Players[DiploScratchF7E8].noun_faction, -1, -1);
parse_says(9, get_phrase(DiploScratchF7E8, 2), -1, -1);
StringTemp[0] = 0;
strcat(StringTemp, "PLEA");
if ((PlayersData[peer_id].diplo_treaties[DiploScratchF7E8] & 1u) != 0) {
    strcat(StringTemp, "PACT");
}
if (DiploScratchF800 >= 0) {
    strcat(StringTemp, "TECH");
    parse_say(4, Technology[DiploScratchF800].name, -1, -1);
}
communicate_plea_result = X_pops(
    StringTemp, 0x00100000, FactionDiplomacyPortraits[peer_id], pop_wait);
if (communicate_plea_result == 0) {
    goto communicate_protective_treaty_dispatch; // native 0x00551123 -> 0x00551336
}
log_say("Getting DIPLOMATIC LOCK", 0, 0, 0);
(void)timeGetTime();
while (ExitTurnLoop == 0) {
    if (NetDaemonNet.add_global() == 0) {
        break;
    }
    check_spock();
    (void)timeGetTime();
}
goto communicate_no_pact_pact_branch; // native 0x00551168
// PRIVATE, INCOMPLETE typed translation of the alternate unpacted-contact
// branch at native 0x0055119E..0x005511EB. This branch bypasses the plea path.
communicate_unpacted_conversation: // native 0x0055119E
if (DiploScratchFA98 == 0 || DiploScratchFA60 != 0 || mode != 0 ||
    (dialogue_flags & 0x40) != 0 ||
    (Players[peer_id].rule_flags & 7u) == 0) {
    goto communicate_introduction_conversation; // native 0x00550E81
}
tech_trade(faction_id, peer_id, 1, 4, 4);
goto communicate_after_trade; // native 0x005571E7
// PRIVATE v3 instruction correction for native 0x00551168..0x005511F3.
// Native network send at 0x551194 falls directly through to 0x5511F3;
// unlike adjacent transaction paths it does not call await_diplo here.
communicate_no_pact_pact_branch: // native 0x00551168
pact_of_brotherhood(faction_id, peer_id);
if (IsMultiplayerNet != 0) {
    log_say("Sending TREATY OFF", faction_id, peer_id, 0x60);
    message_data(0x2442, 0, faction_id, peer_id, 0x60, 0);
} else {
    treaty_off(faction_id, peer_id, 0x60);
}
communicate_after_no_pact_pact_branch: // native 0x005511F3
// PRIVATE v4 instruction correction for native 0x005511F3..0x00551336.
// Preserves v3 callback-visible reload order; removes the unsupported wait
// after TECH message_data. Native jumps from 0x551268 to 0x55127C directly.
communicate_plea_target = DiploScratchF7E8;
if (IsMultiplayerNet != 0) {
    log_say(DiploLabelSendingDOUBLECROSS, faction_id,
            communicate_plea_target, peer_id);
    message_data(0x2440, 0, faction_id,
                 communicate_plea_target, peer_id, 0);
} else {
    double_cross(faction_id, communicate_plea_target, peer_id);
}
communicate_plea_tech = DiploScratchF800;
if (communicate_plea_tech >= 0) {
    if (IsMultiplayerNet != 0) {
        log_say(DiploLabelSendingTECH, faction_id,
                communicate_plea_tech, peer_id);
        message_data(0x244B, 0, faction_id,
                     communicate_plea_tech, peer_id, 0);
    } else {
        tech_achieved(faction_id, communicate_plea_tech, peer_id, 0);
    }
    DiploScratchF800 = -1;
}
communicate_plea_payment = communicate_plea_amount;
if (IsMultiplayerNet != 0) {
    log_say(DiploLabelSendingENERGY, peer_id, faction_id,
            -communicate_plea_payment);
    message_data(0x2447, 0, peer_id, -communicate_plea_payment,
                 faction_id, communicate_plea_payment);
    NetDaemonNet.await_diplo(0x447);
} else {
    if (peer_id != 0) {
        PlayersData[peer_id].energy_reserves = communicate_wrapped_add(
            PlayersData[peer_id].energy_reserves,
            -communicate_plea_payment);
    }
    if (faction_id != 0) {
        PlayersData[faction_id].energy_reserves = communicate_wrapped_add(
            PlayersData[faction_id].energy_reserves,
            communicate_plea_payment);
    }
    ConsoleGlobal.update_data(1);
}
log_say(DiploLabelReleasingDIPLOMATICLOCK, 0, 0, 0);
NetDaemonNet.unlock();
DiploScratchFA8C = DiploScratchF7E8;
goto communicate_after_third_party_plea; // native 0x00551331 -> 0x005516C7
// PRIVATE typed translation of native 0x005516C7..0x005516FB.
// This is the actual target of the accepted-plea branch's 0x551331 jump.
communicate_after_third_party_plea: // native 0x005516C7
DiploScratchF7E8 = 0;
DiploScratchFA98 = 0;
DiploScratchFA88 = 0;
DiploScratchFA68 = 0;
DiploScratchFA60 = 0;
communicate_stack_20 = 1;
DiploScratchFA4C = 0;
communicate_stack_34 = 1;
goto communicate_after_proposal_menu; // native 0x005516FB -> 0x0055362D
// INCOMPLETE PRIVATE BODY SECTION: native 0x00551336..0x005513C5.
// Live-ins: faction_id, peer_id. The native multiplayer tests and their
// subsequent callbacks are retained as distinct paths.
communicate_protective_treaty_dispatch: // native 0x00551336
if (IsMultiplayerNet) {
    log_say("Sending TREATY ON", faction_id, peer_id, 0x00800000);
    message_data(0x2441, 0, faction_id, peer_id, 0x00800000, 0);
} else {
    treaty_on(faction_id, peer_id, 0x00800000);
}
if (IsMultiplayerNet) {
    log_say("Sending TREATY OFF", faction_id, peer_id, 0x01000000);
    message_data(0x2442, 0, faction_id, peer_id, 0x01000000, 0);
} else {
    treaty_off(faction_id, peer_id, 0x01000000);
}
DiploScratchF7B8 = DiploScratchF7E8;
goto communicate_protective_treaty_done; // native 0x0055362D
// INCOMPLETE BOUNDARY: 0x0055362D is the shared native continuation.
// INCOMPLETE PRIVATE BODY SECTION: native 0x005513CA..0x005514FA.
// Entered from the low-friction/low-income edges at 0x00550EEE and
// 0x00550EFD. Live-ins: faction_id, peer_id, and the current DiploScratchF7E8
// faction selector. These are the seven ordered parser slots of this topic.
communicate_treaty_topic_phrases: // native 0x005513CA
PluralityDefault = FALSE;
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
parse_says(0, Players[DiploScratchF7E8].title_leader, -1, -1);
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
PluralityDefault = FALSE;
parse_says(1, Players[DiploScratchF7E8].name_leader, -1, -1);
GenderDefault = Players[DiploScratchF7E8].noun_gender;
PluralityDefault = Players[DiploScratchF7E8].is_noun_plural;
parse_says(2, Players[DiploScratchF7E8].noun_faction, -1, -1);
parse_says(3, get_him_her(DiploScratchF7E8, 0), -1, -1);
parse_says(4, get_pact_hood(faction_id, peer_id), -1, -1);
PluralityDefault = FALSE;
GenderDefault = Players[faction_id].is_leader_female;
parse_says(5, Players[faction_id].title_leader, -1, -1);
parse_says(6, get_pact(DiploScratchF7E8), -1, -1);
// INCOMPLETE BOUNDARY: the next native operation at 0x005514FA begins the
// rendered topic/UI path; it is not part of the seven-slot parser sequence.
// INCOMPLETE PRIVATE BODY SECTION: native 0x005514FA..0x005515D9.
// Continues the seven parser slots. The native treaty test uses the
// DiploScratchF7E8 counterpart, not the initiating peer.
// Function-scope live-out local, declared before any native-entry labels:
//   int communicate_treaty_topic_result;
communicate_treaty_topic_popup: // native 0x005514FA
StringTemp[0] = '\0';
strcat(StringTemp, "PROPOSE");
if ((PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 1u) != 0) {
    strcat(StringTemp, "PACT");
    PluralityDefault = FALSE;
    GenderDefault = Players[peer_id].is_leader_female;
    parse_says(8, Players[peer_id].title_leader, -1, -1);
    parse_says(9, get_phrase(peer_id, 2), -1, -1);
} else {
    parse_says(8, Players[DiploScratchF7E8].adj_insult_faction, -1, -1);
    parse_says(9, get_phrase(peer_id, 5), -1, -1);
}
communicate_treaty_topic_result = X_pops(
    StringTemp, 0x00100000, FactionDiplomacyPortraits[peer_id], pop_wait);
if (communicate_treaty_topic_result == 0) {
    goto communicate_treaty_topic_zero_result; // native 0x005516FB
}
// INCOMPLETE BOUNDARY: nonzero popup result enters the native lock/wait loop
// at 0x005515D9; zero continues at 0x005516FB.
// INCOMPLETE PRIVATE BODY SECTION: native 0x005515D9..0x005516C2.
// This is the nonzero x_pops_short result path. The already-owned diplomacy
// lock/unlock helpers preserve the native add_global/check_spock/time polling
// loop and lock-release log; topic/result state stays live across callbacks.
communicate_treaty_topic_nonzero_result: // native 0x005515D9
if (communicate_treaty_topic_result != 0) {
    (void)diplo_lock(0);
    pact_of_brotherhood(faction_id, peer_id);
    if (IsMultiplayerNet) {
        log_say("Sending TREATY OFF", faction_id, peer_id, 0x60);
        message_data(0x2442, 0, faction_id, peer_id, 0x60, 0);
    } else {
        treaty_off(faction_id, peer_id, 0x60);
    }
    if (IsMultiplayerNet) {
        log_say("Sending DOUBLE CROSS", faction_id, DiploScratchF7E8, peer_id);
        message_data(0x2440, 0, faction_id, DiploScratchF7E8, peer_id, 0);
        NetDaemonNet.await_diplo(0x440);
    } else {
        double_cross(faction_id, DiploScratchF7E8, peer_id);
    }
    diplo_unlock();
}
// Native 0x005516C2 writes the selected counterpart to its separately bound
// neutral start-only scratch cell.
DiploScratchFA8C = DiploScratchF7E8;
// INCOMPLETE PRIVATE BODY SECTION: native 0x005516FB..0x0055178F.
// This is the zero-result edge from the PROPOSE popup; keep its action order
// and return-to-topic flow separate from the nonzero double-cross edge.
communicate_treaty_topic_zero_result: // native 0x005516FB
if (IsMultiplayerNet) {
    log_say("Sending TREATY ON", faction_id, peer_id, 0x00800000);
    message_data(0x2441, 0, faction_id, peer_id, 0x00800000, 0);
} else {
    treaty_on(faction_id, peer_id, 0x00800000);
}
if (IsMultiplayerNet) {
    log_say("Sending TREATY OFF", faction_id, peer_id, 0x01000000);
    message_data(0x2442, 0, faction_id, peer_id, 0x01000000, 0);
} else {
    treaty_off(faction_id, peer_id, 0x01000000);
}
DiploScratchF7B8 = DiploScratchF7E8;
goto communicate_protective_treaty_done; // native 0x0055362D
// INCOMPLETE BOUNDARY: nonzero-popup dialogue continuation starts at
// 0x0055178F.
// INCOMPLETE PRIVATE BODY SECTION: native 0x0055178F..0x00551896.
// This entry is the distinct dialogue-flags continuation. All pair fields are
// reread from their typed owners after the preceding popup/callback paths.
communicate_no_tech_greeting: // native 0x0055178F
communicate_third_party_topic_gates:
if ((PlayersData[faction_id].diplo_agenda[peer_id] & 0x00002408) != 0) {
    goto communicate_shared_dialogue_outro; // native 0x0055362D
}
if ((PlayersData[faction_id].diplo_treaties[peer_id] & 0x40u) != 0) {
    goto communicate_shared_dialogue_outro; // native 0x0055362D
}
if (DiploScratchFAA0 != 0 && early_contact_path == 0) {
    goto communicate_shared_dialogue_outro; // native 0x0055362D
}
if (DiploScratchF7E8 == 0) {
    goto communicate_third_party_war_exit; // native 0x00551EBE
}
if ((PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 0x80u) != 0) {
    goto communicate_third_party_war_exit; // native 0x00551EBE
}
if (DiploScratchFA60 == 0 && DiploScratchFA80 != 0) {
    goto communicate_third_party_pact_exit; // native 0x00551ECB
}
if (DiploScratchFA98 != 0 && DiplomacyProposalGateCount <= 0 &&
    DiploScratchFA70 != 0) {
    // This gate is intentionally written as the original short-circuit tests;
    // the native reaches the rating comparisons only after them.
    if (PlayersData[faction_id].diff_level >= 6 &&
        PlayersData[peer_id].diff_level <= 4 &&
        DiploScratchFA54 == 0 && DiploScratchF7FC == 0) {
        goto communicate_third_party_rating_checked; // native 0x00551884
    }
    if (great_satan(DiploScratchF7E8, 0) == 0 &&
        (PlayersData[faction_id].diplo_treaties[peer_id] & 1u) == 0) {
        goto communicate_third_party_war_exit; // native 0x00551EBE
    }
}
communicate_third_party_rating_checked: // native 0x00551884
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_third_party_war_exit; // native 0x00551EBE
}
// INCOMPLETE BOUNDARY: the slot-0/1 noun and UI continuation begins at
// native 0x00551896. Two exits above (0x551EBE/0x551ECB) remain unrendered.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00551896..0x005519D5.
// Reached after the typed third-party gates and wants_to_speak callback.
// Native selects the F7E8 counterpart for all seven ordered parser slots.
communicate_third_party_topic_phrases: // native 0x00551896
parse_says(0, get_pact(DiploScratchF7E8), -1, -1);
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
PluralityDefault = FALSE;
parse_says(1, Players[DiploScratchF7E8].title_leader, -1, -1);
GenderDefault = Players[DiploScratchF7E8].is_leader_female;
PluralityDefault = FALSE;
parse_says(2, Players[DiploScratchF7E8].name_leader, -1, -1);
GenderDefault = Players[DiploScratchF7E8].noun_gender;
PluralityDefault = Players[DiploScratchF7E8].is_noun_plural;
parse_says(3, Players[DiploScratchF7E8].noun_faction, -1, -1);
parse_says(4, get_his_her(DiploScratchF7E8, 0), -1, -1);
parse_says(5, get_him_her(DiploScratchF7E8, 0), -1, -1);
parse_says(6, Players[DiploScratchF7E8].adj_name_faction, -1, -1);
// INCOMPLETE BOUNDARY: the native counterpart-treaty/topic-choice tree begins
// at 0x005519D5 and constructs additional phrase slots before its menu.
// INCOMPLETE PRIVATE BODY SECTION: native 0x005519D5..0x00551C15.
// Requires function-scope automatic locals, declared before native-entry
// labels so earlier gotos cannot bypass initialization:
//   int communicate_demand_topic_code;
//   char communicate_demand_topic_suffix[2];
//   char communicate_demand_topic_number[10];
// The suffix contains only one of the pinned single-byte literals "a"/"b".
communicate_counterpart_demand_topic: // native 0x005519D5
communicate_demand_topic_suffix[0] = '\0';
if ((PlayersData[faction_id].diplo_treaties[peer_id] & 1u) != 0 &&
    (((PlayersData[faction_id].diplo_treaties[peer_id] & 0x2000u) != 0) ||
     ((PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 8u) == 0))) {
    communicate_demand_topic_code = 3;
    goto communicate_demand_topic_assemble; // native 0x00551BAA
}
if ((PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 1u) != 0) {
    communicate_demand_topic_code = 0;
    parse_says(9, get_phrase(DiploScratchF7E8, 5), -1, -1);
    goto communicate_demand_topic_assemble; // native 0x00551BAA
}
if ((PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 2u) != 0) {
    communicate_demand_topic_code = 1;
    parse_says(6, get_noun_phrase(DiploScratchF7E8, 8), -1, -1);
    parse_says(9, get_phrase(DiploScratchF7E8, 5), -1, -1);
    parse_says(8, get_phrase(DiploScratchF7E8, 2), -1, -1);
    if (random(0, 2) != 0 ||
        (PlayersData[faction_id].diplo_treaties[peer_id] & 0x20u) != 0) {
        strcat(communicate_demand_topic_suffix, "a");
    } else {
        strcat(communicate_demand_topic_suffix, "b");
    }
} else if ((PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 4u) != 0 &&
           (PlayersData[faction_id].diplo_treaties[DiploScratchF7E8] & 0x10000u) == 0) {
    communicate_demand_topic_code = 2;
    if (random(0, 2) != 0) {
        strcat(communicate_demand_topic_suffix, "a");
        parse_says(8, Players[DiploScratchF7E8].adj_insult_leader, -1, -1);
        parse_says(9, Players[DiploScratchF7E8].adj_insult_faction, -1, -1);
    } else {
        strcat(communicate_demand_topic_suffix, "b");
        parse_says(8, get_noun_phrase(peer_id, 9), -1, -1);
        parse_says(9, get_phrase(DiploScratchF7E8, 2), -1, -1);
    }
} else {
    communicate_demand_topic_code = 4;
}
communicate_demand_topic_assemble: // native 0x00551BAA
StringTemp[0] = '\0';
strcat(StringTemp, "DEMANDATTACK");
_itoa(communicate_demand_topic_code, communicate_demand_topic_number, 10);
strcat(StringTemp, communicate_demand_topic_number);
strcat(StringTemp, communicate_demand_topic_suffix);
if (X_pops(StringTemp, 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) != 0) {
    goto communicate_demand_topic_nonzero_result; // native 0x00551D71
}
// INCOMPLETE BOUNDARY: a zero popup result continues at native 0x00551C15;
// the next path updates patience/friction and enters further proposal UI.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00551C15..0x00551D71.
// Reached only when the preceding demand-topic popup returns zero. The native
// pair-direction fields are reloaded here; callback state is not cached.
// Function-scope local initialized at native entry and used again in later
// body regions: `int communicate_native_had_pact` (entry value 0).
communicate_demand_topic_acceptance: // native 0x00551C15
if ((PlayersData[peer_id].diplo_treaties[faction_id] & 1u) != 0) {
    communicate_stack_38 = 1; // native 551C2A
    if ((PlayersData[peer_id].diplo_treaties[faction_id] & 0x2000u) != 0) {
        DiploScratchFA60 = 1;
    }
    PlayersData[peer_id].diplo_friction[faction_id] = range(
        communicate_wrapped_add(PlayersData[peer_id].diplo_friction[faction_id], 4), 0, 20);
    if (DiploFrictionFactionID == static_cast<uint32_t>(peer_id) &&
        DiploFrictionFactionIDWith == static_cast<uint32_t>(faction_id)) {
        DiploFriction = communicate_wrapped_add(DiploFriction, 4);
    }
} else if (DiploScratchFA4C != 0 && DiploFriction >= 10 &&
           DiploScratchFA98 == 0 &&
           PlayersData[peer_id].mil_strength_1 >
               PlayersData[faction_id].mil_strength_1) {
    DiploScratchFA60 = 1;
}
if (IsMultiplayerNet) {
    log_say("Sending TREATY ON", faction_id, peer_id, 0x00800000);
    message_data(0x2441, 0, faction_id, peer_id, 0x00800000, 0);
} else {
    treaty_on(faction_id, peer_id, 0x00800000);
}
if (IsMultiplayerNet) {
    log_say("Sending TREATY OFF", faction_id, peer_id, 0x01000000);
    message_data(0x2442, 0, faction_id, peer_id, 0x01000000, 0);
} else {
    treaty_off(faction_id, peer_id, 0x01000000);
}
DiploScratchF7B8 = DiploScratchF7E8;
diplomacy_caption(faction_id, peer_id);
goto communicate_menu_continuation; // native 0x00553464
// INCOMPLETE BOUNDARY: nonzero demand-topic popup result enters the separate
// lock/pact path at 0x00551D71.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00551D71..0x00551E99.
// This is the nonzero demand-topic popup result path; preserve the native
// treaty-on/off order before the double-cross message/action.
communicate_demand_topic_nonzero_result: // native 0x00551D71
(void)diplo_lock(0);
if (IsMultiplayerNet) {
    log_say("Sending TREATY ON", faction_id, peer_id, 0x01000000);
    message_data(0x2441, 0, faction_id, peer_id, 0x01000000, 0);
} else {
    treaty_on(faction_id, peer_id, 0x01000000);
}
if (IsMultiplayerNet) {
    log_say("Sending TREATY OFF", faction_id, peer_id, 0x00800000);
    message_data(0x2442, 0, faction_id, peer_id, 0x00800000, 0);
} else {
    treaty_off(faction_id, peer_id, 0x00800000);
}
if (IsMultiplayerNet) {
    log_say("Sending DOUBLE CROSS", faction_id, DiploScratchF7E8, peer_id);
    message_data(0x2440, 0, faction_id, DiploScratchF7E8, peer_id, 0);
    NetDaemonNet.await_diplo(0x440);
} else {
    double_cross(faction_id, DiploScratchF7E8, peer_id);
}
diplo_unlock();
// Native 0x00551E99 stores the reloaded counterpart into its independent
// neutral start-only scratch cell.
DiploScratchFA8C = DiploScratchF7E8;
DiploScratchF7E8 = 0;
communicate_stack_20 = 1; // native [EBP-0x20]
DiploScratchFA4C = 0;
diplomacy_caption(faction_id, peer_id);
goto communicate_menu_continuation; // native 0x00553464
// PRIVATE typed continuation for native 0x00551E99..0x00551EBE.
// The original cdecl pushes ESI then EDI, so the call receives peer first,
// actor second (entry's EDI/ESI respectively), despite older WIP passing the
// opposite order.
DiploScratchFA8C = DiploScratchF7E8; // native load at 0x551E92, store 0x551E99
DiploScratchF7E8 = 0;
communicate_stack_20 = 1; // native [EBP-0x20]
DiploScratchFA4C = 0;
diplomacy_caption(peer_id, faction_id); // native pushes ESI, then EDI
goto communicate_menu_continuation; // native jump at 0x00551EB9 ends at 0x00551EBE
// INCOMPLETE PRIVATE BODY SECTION: native 0x00551EBE..0x00551F92.
// The two direct exits preserve their order and the native currently-selected
// F7D4 counterpart; no new faction validity guard is introduced.
communicate_third_party_war_exit: // native 0x00551EBE
if (DiploScratchFA60 != 0) {
    goto communicate_menu_continuation; // native 0x00553464
}
communicate_third_party_pact_exit: // native 0x00551ECB
if (DiploScratchFA70 != 0) {
    goto communicate_menu_continuation; // native 0x00553464
}
if (DiploScratchF7D4 == 0) {
    goto communicate_demand_topic_fallback; // native 0x005524EA
}
if ((PlayersData[faction_id].diplo_treaties[DiploScratchF7D4] & 0x08000008u) != 0) {
    goto communicate_demand_topic_fallback; // native 0x005524EA
}
if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
     static_cast<uint8_t>(1u << DiploScratchF7D4)) != 0) {
    if (IsMultiplayerNet != 0 && Rules.tgl_human_always_contact_net != 0) {
        goto communicate_demand_topic_fallback; // native 0x005524EA
    }
    if (IsMultiplayerPBEM != 0 && Rules.tgl_humans_always_contact_pbem != 0) {
        goto communicate_demand_topic_fallback; // native 0x005524EA
    }
}
if ((PlayersData[faction_id].diplo_treaties[peer_id] & 0x31u) != 0) {
    goto communicate_demand_topic_fallback; // native 0x005524EA
}
if (great_satan(faction_id, 0) != 0) {
    goto communicate_demand_topic_fallback; // native 0x005524EA
}
if (PlayersData[faction_id].diff_level > 5 &&
    PlayersData[peer_id].diff_level > 4) {
    goto communicate_demand_topic_fallback; // native 0x005524EA
}
// INCOMPLETE BOUNDARY: matching-faction follow-on comparisons begin at
// native 0x00551F92; fallback/menu continuations remain open.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00551F92..0x00552162.
// Requires function-scope `int communicate_ally_score`, assigned before use.
// This preserves the four sequential native score contributors rather than
// reordering them into an expression around great_satan.
communicate_third_party_introduction_topic: // native 0x00551F92
communicate_ally_score = static_cast<int>(great_satan(DiploScratchF7D4, 0));
communicate_ally_score +=
    (PlayersData[DiploScratchF7D4].diff_level >
     PlayersData[faction_id].diff_level);
communicate_ally_score += (DiploScratchFA54 == 0);
communicate_ally_score += (IsMultiplayerNet == 0);
if (communicate_ally_score < 2) {
    goto communicate_demand_topic_fallback; // native 0x005524EA
}
if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 ||
    (Players[peer_id].rule_flags & RFLAG_ALIEN) != 0) {
    goto communicate_demand_topic_fallback; // native 0x005524EA
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_demand_topic_fallback; // native 0x005524EA
}
GenderDefault = Players[DiploScratchF7D4].is_leader_female;
PluralityDefault = FALSE;
parse_says(0, Players[DiploScratchF7D4].title_leader, -1, -1);
GenderDefault = Players[DiploScratchF7D4].is_leader_female;
PluralityDefault = FALSE;
parse_says(1, Players[DiploScratchF7D4].name_leader, -1, -1);
GenderDefault = Players[DiploScratchF7D4].noun_gender;
PluralityDefault = Players[DiploScratchF7D4].is_noun_plural;
parse_says(2, Players[DiploScratchF7D4].noun_faction, -1, -1);
parse_says(3, get_phrase(DiploScratchF7D4, 5), -1, -1);
parse_says(4, get_pact_hood(faction_id, peer_id), -1, -1);
if ((Players[DiploScratchF7E8].rule_flags & RFLAG_ALIEN) != 0 &&
    X_pops("INTRODUCEENEMYALIEN", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
    goto communicate_menu_continuation; // native 0x00553464
}
// INCOMPLETE BOUNDARY: a successful/non-alien follow-up enters native wait
// and subsequent proposal text beginning at 0x00552168.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00552168..0x00552331.
// Continues only after the INTRODUCEENEMYALIEN popup returned nonzero.
communicate_introduction_after_popup: // native 0x00552168
log_say("Getting DIPLOMATIC LOCK", 0, 0, 0);
timeGetTime();
while (ExitTurnLoop == 0 && NetDaemonNet.add_global() != 0) {
    check_spock();
    timeGetTime();
}
net_set_treaty(faction_id, DiploScratchF7D4, 0x10000000, 1, 0);
pact_of_brotherhood(faction_id, peer_id);
net_treaty_on(faction_id, peer_id, 0x01000000, 0);
net_treaty_off(faction_id, peer_id, 0x00800000, 0);
net_double_cross(faction_id, DiploScratchF7D4, peer_id, 1);
net_treaty_on(faction_id, DiploScratchF7D4, 0x08000008, 0);
log_say("Releasing DIPLOMATIC LOCK", 0, 0, 0);
NetDaemonNet.unlock();
// Native reloads the current target only after callbacks and lock release.
DiploScratchFA8C = DiploScratchF7D4;
DiploScratchF7D4 = 0;
goto communicate_third_party_accepted_join_52331; // native 0x00552331
// INCOMPLETE PRIVATE BODY SECTION: native 0x00552331..0x0055234D.
// Shared join for the FA38/F7C8 acceptance paths and earlier introduction.
communicate_third_party_accepted_join_52331: // native 0x00552331
DiploScratchF7E8 = 0;
communicate_stack_20 = 1; // native [EBP-0x20]
DiploScratchFA4C = 0;
communicate_stack_34 = 1; // native [EBP-0x34]
goto communicate_menu_continuation; // native 0x00553464
// PRIVATE typed connected proposal for native 0x0055234D..0x005524E5,
// plus its in-body cleanup target 0x00552321..0x0055234D.
// This is the accepted alien-introduction contract route selected from the
// 0x552140 branch. Popup decline rejoins at 0x553464; acceptance performs the
// native wait, ordered treaty/pact/double-cross callbacks, unlock, then the
// target block's F7D4 capture/reset before the same menu continuation.
communicate_intro_contract_accepted:
if (X_pops("INTRODUCEENEMY", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
    goto communicate_menu_continuation; // native JE at 0x552369
}
log_say("Getting DIPLOMATIC LOCK", 0, 0, 0);
timeGetTime();
if (ExitTurnLoop == 0) {
    while (NetDaemonNet.add_global() != 0) {
        check_spock();
        timeGetTime();
        if (ExitTurnLoop != 0) {
            break;
        }
    }
}
if (IsMultiplayerNet != 0) {
    log_say("Sending SET TREATY", faction_id, DiploScratchF7D4,
            0x10000000);
    message_data(0x2443, 0, faction_id, DiploScratchF7D4,
                 0x10000000, 1);
} else {
    set_treaty(faction_id, DiploScratchF7D4, 0x10000000, 1);
}
pact_of_brotherhood(faction_id, peer_id);
if (IsMultiplayerNet != 0) {
    log_say("Sending TREATY ON", faction_id, peer_id, 0x01000000);
    message_data(0x2441, 0, faction_id, peer_id, 0x01000000, 0);
} else {
    treaty_on(faction_id, peer_id, 0x01000000);
}
if (IsMultiplayerNet != 0) {
    log_say("Sending TREATY OFF", faction_id, peer_id, 0x00800000);
    message_data(0x2442, 0, faction_id, peer_id, 0x00800000, 0);
} else {
    treaty_off(faction_id, peer_id, 0x00800000);
}
if (IsMultiplayerNet != 0) {
    log_say("Sending DOUBLE CROSS", faction_id, DiploScratchF7D4,
            faction_id);
    message_data(0x2440, 0, faction_id, DiploScratchF7D4, peer_id, 0);
    NetDaemonNet.await_diplo(0x440);
} else {
    double_cross(faction_id, DiploScratchF7D4, peer_id);
}
net_treaty_on(faction_id, DiploScratchF7D4, 0x08000008, 0);
diplo_unlock();
goto communicate_intro_contract_cleanup; // native JMP at 0x5524E5
communicate_intro_contract_cleanup: // native 0x00552321
{
    const int captured_faction = DiploScratchF7D4;
    DiploScratchF7D4 = 0;
    DiploScratchFA8C = captured_faction;
}
DiploScratchF7E8 = 0;
communicate_stack_20 = 1; // native [EBP-0x20]
DiploScratchFA4C = 0;
communicate_stack_34 = 1; // native [EBP-0x34]
goto communicate_menu_continuation; // native JMP at 0x552348
// INCOMPLETE PRIVATE BODY SECTION: native 0x005524EA..0x00552603.
// This is the FA38-selected third-party route. All failed gates rejoin the
// separate FA3C branch at native 0x00552770; no route-specific behavior is
// inferred from the scratch variable's name.
communicate_demand_topic_fallback: // native 0x005524EA
communicate_third_party_ally_eligibility:
if (DiploScratchFA38 == 0) {
    goto communicate_third_party_ally_fallback; // native 0x00552770
}
if (has_treaty(faction_id, peer_id, 0x31) != 0 ||
    has_treaty(faction_id, DiploScratchFA38, 0x10) != 0) {
    goto communicate_third_party_ally_fallback; // native 0x00552770
}
if (DiploScratchFA54 >= 2 &&
    great_satan(DiploScratchFA38, 0) == 0 &&
    (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
     static_cast<uint8_t>(static_cast<uint32_t>(1u) <<
                          (DiploScratchFA38 & 31))) != 0) {
    goto communicate_third_party_ally_fallback; // native 0x00552770
}
if (wants_to_attack(peer_id, DiploScratchFA38, faction_id) == 0) {
    goto communicate_third_party_ally_fallback; // native 0x00552770
}
if (PlayersData[faction_id].diff_level > 5 &&
    PlayersData[peer_id].diff_level > 4 &&
    great_satan(DiploScratchFA38, 0) == 0) {
    goto communicate_third_party_ally_fallback; // native 0x00552770
}
if (contiguous(faction_id, DiploScratchFA38, 0, 0) == 0 &&
    contiguous(peer_id, DiploScratchFA38, 0, 0) == 0) {
    goto communicate_third_party_ally_fallback; // native 0x00552770
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_third_party_ally_fallback; // native 0x00552770
}
// INCOMPLETE BOUNDARY: parser construction for this ally topic begins at
// native 0x00552603; the fallback topic begins at 0x00552770.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00552603..0x005526E8.
// This renders the FA38-selected BETRAYFRIEND topic and joins common exit on
// zero popup result. The following nonzero-result treaty transitions begin
// at 0x005526EE and end with another unresolved store to 0x0093FA8C.
communicate_third_party_betrayal_topic: // native 0x00552603
parse_says(0, get_name(faction_id), -1, -1);
parse_says(1, get_title(DiploScratchFA38), -1, -1);
parse_says(2, get_name(DiploScratchFA38), -1, -1);
parse_says(3, get_adjective_insult_faction(DiploScratchFA38), -1, -1);
parse_says(4, get_phrase(DiploScratchFA38, 2), -1, -1);
parse_says(5, get_pact_hood(faction_id, peer_id), -1, -1);
parse_says(6, get_noun(DiploScratchFA38), -1, -1);
if (X_pops("BETRAYFRIEND", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
    goto communicate_menu_continuation; // native 0x00553464
}
// INCOMPLETE BOUNDARY: accepted-popup treaty updates begin at native
// 0x005526EE and store to the admitted neutral start-only FA8C cell.
// INCOMPLETE PRIVATE BODY SECTION: native 0x005526EE..0x0055276B.
// Accepted FA38 topic result. Preserve each target reload across calls; the
// native does not keep one captured counterpart value for the entire chain.
communicate_third_party_betrayal_accepted: // native 0x005526EE
net_set_treaty(faction_id, DiploScratchFA38, 0x10000000, 1, 0);
diplo_lock(0);
pact_of_brotherhood(faction_id, peer_id);
net_treaty_on(faction_id, peer_id, 0x01000000, 0);
net_treaty_off(faction_id, peer_id, 0x00800000, 0);
net_double_cross(faction_id, DiploScratchFA38, peer_id, 1);
net_treaty_on(faction_id, DiploScratchFA38, 0x08000008, 0);
diplo_unlock();
DiploScratchFA8C = DiploScratchFA38;
DiploScratchFA38 = 0;
goto communicate_third_party_accepted_join_52331; // native 0x00552331
// INCOMPLETE PRIVATE BODY SECTION: native 0x00552770..0x00552885.
// This is the independent F7C8-selected fallback topic, not a retry through
// FA38. Native predicate order and the two-argument contiguous calls are kept.
communicate_third_party_ally_fallback: // native 0x00552770
communicate_third_party_secondary_eligibility:
if (DiploScratchF7C8 == 0) {
    goto communicate_final_secondary_fallback; // native 0x005529EF
}
if (has_treaty(faction_id, peer_id, 0x31) != 0 ||
    has_treaty(faction_id, DiploScratchF7C8, 0x10) != 0) {
    goto communicate_final_secondary_fallback; // native 0x005529EF
}
if (DiploScratchFA54 >= 2 &&
    great_satan(DiploScratchF7C8, 0) == 0 &&
    (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
     static_cast<uint8_t>(static_cast<uint32_t>(1u) <<
                          (DiploScratchF7C8 & 31))) != 0) {
    goto communicate_final_secondary_fallback; // native 0x005529EF
}
if (wants_to_attack(peer_id, DiploScratchF7C8, faction_id) == 0) {
    goto communicate_final_secondary_fallback; // native 0x005529EF
}
if (PlayersData[faction_id].diff_level > 5 &&
    PlayersData[peer_id].diff_level > 4 &&
    great_satan(DiploScratchF7C8, 0) == 0) {
    goto communicate_final_secondary_fallback; // native 0x005529EF
}
if (contiguous(faction_id, DiploScratchF7C8, 0, 0) == 0 &&
    contiguous(peer_id, DiploScratchF7C8, 0, 0) == 0) {
    goto communicate_final_secondary_fallback; // native 0x005529EF
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_final_secondary_fallback; // native 0x005529EF
}
// INCOMPLETE BOUNDARY: F7C8 phrase construction starts at native 0x00552885.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00552885..0x005528C8.
// The nonzero popup result enters a separate accepted-result continuation.
communicate_third_party_secondary_topic: // native 0x00552885
parse_says(0, get_name(faction_id), -1, -1);
parse_says(1, get_title(DiploScratchF7C8), -1, -1);
parse_says(2, get_name(DiploScratchF7C8), -1, -1);
parse_says(3, get_adjective_insult_faction(DiploScratchF7C8), -1, -1);
parse_says(4, get_phrase(DiploScratchF7C8, 2), -1, -1);
parse_says(5, get_pact_hood(faction_id, peer_id), -1, -1);
parse_says(6, get_noun(DiploScratchF7C8), -1, -1);
if (X_pops("BETRAYFRIEND", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
    goto communicate_menu_continuation; // native 0x00553464
}
goto communicate_third_party_secondary_accepted; // native 0x005528C8
// INCOMPLETE PRIVATE BODY SECTION: native 0x005528C8..0x005529EF.
// F7C8-selected BETRAYFRIEND popup acceptance and common 0x552331 join.
communicate_third_party_secondary_accepted: // native 0x005528C8
net_set_treaty(faction_id, DiploScratchF7C8, 0x10000000, 1, 0);
diplo_lock(0);
pact_of_brotherhood(faction_id, peer_id);
net_treaty_on(faction_id, peer_id, 0x01000000, 0);
net_treaty_off(faction_id, peer_id, 0x00800000, 0);
net_double_cross(faction_id, DiploScratchF7C8, peer_id, 1);
net_treaty_on(faction_id, DiploScratchF7C8, 0x08000008, 0);
diplo_unlock();
communicate_secondary_target = DiploScratchF7C8;
DiploScratchFA8C = communicate_secondary_target;
goto communicate_third_party_accepted_join_52331;
// INCOMPLETE PRIVATE BODY SECTION: native 0x005529EF..0x00552A41.
// Continuation uses the verified typed PlayerData::traded_maps member.
communicate_final_secondary_fallback: // native 0x005529EF
communicate_third_party_tertiary_gates:
if (DiploScratchF7D8 == 0 ||
    has_treaty(faction_id, peer_id, 0x31) != 0 ||
    has_treaty(faction_id, DiploScratchF7D8, 0x10000001) != 0) {
    goto communicate_secondary_third_party_gates; // native 0x00552C98
}
// INCOMPLETE BOUNDARY: native 0x00552A4C continues in the typed player-gates
// fragment; native +0x1CC is PlayerData::traded_maps.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00552A41..0x00552ACB.
// Original 0x0096CBAC is PlayerData::traded_maps at native member offset
// 0x1CC; both faction rows are addressed through the existing PlayersData
// owner and verified 0x20CC stride.
// Requires function-scope `uint32_t communicate_tertiary_mask` and
// `int communicate_tertiary_reserve_limit` because earlier labels can jump
// into this path.
communicate_third_party_tertiary_player_gates: // native 0x00552A41
communicate_tertiary_mask =
    static_cast<uint32_t>(1u) << (DiploScratchF7D8 & 31);
if ((PlayersData[faction_id].traded_maps & communicate_tertiary_mask) != 0) {
    goto communicate_secondary_third_party_gates; // native 0x00552C98
}
if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
     static_cast<uint8_t>(communicate_tertiary_mask)) != 0 &&
    (PlayersData[peer_id].traded_maps & communicate_tertiary_mask) == 0) {
        goto communicate_secondary_third_party_gates; // native 0x00552C98
}
if (DiploScratchFAA8 < 0 || (GameDataStateStorage.fields_.game_rules_ & 0x02000000u) != 0) {
    communicate_tertiary_reserve_limit = GameDataStateStorage.fields_.turn_current_num_ < 0x32 ? 25 : 50;
    if (PlayersData[faction_id].energy_reserves <
        communicate_tertiary_reserve_limit) {
        goto communicate_secondary_third_party_gates; // native 0x00552C98
    }
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_secondary_third_party_gates; // native 0x00552C98
}
// INCOMPLETE BOUNDARY: F7D8's parser/UI sequence starts at native 0x00552ACB.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00552ACB..0x00552BF7.
// Requires function-scope `int communicate_tertiary_reserve_limit`; the
// native parser uses the requested technology from DiploScratchFAA8.
communicate_third_party_tertiary_topic: // native 0x00552ACB
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_title(DiploScratchF7D8), -1, -1);
parse_says(3, get_name(DiploScratchF7D8), -1, -1);
parse_says(4, get_adjective(DiploScratchF7D8), -1, -1);
StringTemp[0] = '\0';
say_tech(DiploScratchFAA8, 1);
parse_says(5, StringTemp, -1, -1);
communicate_tertiary_reserve_limit = GameDataStateStorage.fields_.turn_current_num_ < 0x32 ? 25 : 50;
parse_num(0, communicate_tertiary_reserve_limit);
StringTemp[0] = '\0';
strcat(StringTemp, "ENEMYMAP");
strcat(StringTemp,
       PlayersData[faction_id].energy_reserves >=
               communicate_tertiary_reserve_limit
           ? "METALIEN"
           : "METFRIEND");
net_set_treaty(faction_id, DiploScratchF7D8, 0x10000000, 1, 0);
if (X_pops(StringTemp, 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
    goto communicate_menu_continuation; // native 0x00553464
}
// INCOMPLETE BOUNDARY: successful-popup effects continue in
// third-party-tertiary-accepted-52bfd.inc.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00552BFD..0x00552C98.
// Successful F7D8 popup result; the final FA48 assignment is the admitted
// neutral start-only scalar, not evidence of its wider native storage.
communicate_third_party_tertiary_accepted: // native 0x00552BFD
net_treaty_on(faction_id, peer_id, 0x01000000, 0);
net_tech(peer_id, DiploScratchF7D8, 9999, 1);
net_tech(faction_id, DiploScratchF7D8, 9999, 1);
if (PlayersData[faction_id].energy_reserves >=
    communicate_tertiary_reserve_limit) {
    net_energy(faction_id, -communicate_tertiary_reserve_limit,
               peer_id, communicate_tertiary_reserve_limit, 1);
} else {
    // Preserve the native argument order even though its third argument is
    // the actor id, not a normalized technology identifier.
    net_tech(peer_id, DiploScratchFAA8, faction_id, 1);
}
DiploScratchFA48 = 1;
DiploScratchFAA8 = DiploScratchFA18;
DiploScratchFA18 = -1;
communicate_stack_20 = 1; // native stack local [EBP-0x20]
DiploScratchF7D8 = 0;
goto communicate_menu_continuation; // native 0x00553464
// INCOMPLETE PRIVATE BODY SECTION: pinned native 0x00552C98..0x00552D19.
// This is the secondary third-party candidate's eligibility prefix. The
// continuation at 0x552D19 contains its ordered parser/topic setup.
communicate_secondary_third_party_gates: // native 0x00552C98
communicate_native_ebx_at_55304a = 0;
if (DiploScratchFA38 == 0 ||
    has_treaty(faction_id, peer_id, 0x30) != 0 ||
    has_project(4, faction_id) != 0 ||
    has_treaty(faction_id, DiploScratchFA38, 8) != 0) {
    goto communicate_secondary_third_party_exit; // native 0x0055304A
}
if (is_human(DiploScratchFA38) && auto_contact() != 0) {
    goto communicate_secondary_third_party_exit; // native 0x0055304A
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_secondary_third_party_exit; // native 0x0055304A
}
// Native 0x552D19 starts the parser setup for this selected faction.
// INCOMPLETE PRIVATE BODY SECTION: pinned native 0x00552D19..0x00552E0A.
// Ordered target-faction parser slots, then the native 0..88 unowned-tech
// valuation scan. Function-scope locals mirror reused native stack slots.
communicate_secondary_third_party_topic: // native 0x00552D19
communicate_secondary_action = 0; // native 552D23, live EBX zero
parse_says(2, get_title(DiploScratchFA38), -1, -1);
parse_says(3, get_name(DiploScratchFA38), -1, -1);
parse_says(4, get_noun(DiploScratchFA38), -1, -1);
parse_says(5, get_phrase(DiploScratchFA38, 9), -1, -1);
communicate_secondary_best_tech_value = 0; // native [EBP+8]
communicate_secondary_best_tech = -1; // native [EBP+0xC]
for (communicate_secondary_tech_scan = 0;
     communicate_secondary_tech_scan < 0x59;
     ++communicate_secondary_tech_scan) {
    const int candidate = communicate_secondary_tech_scan;
    if (has_tech(candidate, DiploScratchFA38) == 0 &&
        has_tech(candidate, faction_id) == 0 &&
        has_tech(candidate, peer_id) == 0) {
        const int value = tech_val(candidate, faction_id, TRUE);
        if (value > communicate_secondary_best_tech_value) {
            communicate_secondary_best_tech_value = value;
            communicate_secondary_best_tech = candidate;
        }
    }
}
// Native 0x552E0A converts the difficulty-scaled cost to the upcoming offer.
// INCOMPLETE PRIVATE BODY SECTION: pinned native 0x00552E0A..0x0055304A.
// Converts the selected-tech value into the secondary offer amount, keeps
// the native low-difficulty/lowest-difficulty order, builds the resulting
// popup text, then applies the original local/network treaty updates.
communicate_secondary_offer_price: // native 0x00552E0A
communicate_secondary_offer_amount = communicate_wrapped_mul(
    communicate_imul_high_sar(GameDataStateStorage.fields_.turn_current_num_, 0x51EB851Fu, 3, 1), 10);
if (static_cast<int>(PlayersData[faction_id].ranking) < 5) {
    communicate_secondary_offer_amount /= 2;
}
if (static_cast<int>(PlayersData[faction_id].ranking) == 7) {
    communicate_secondary_offer_amount = communicate_wrapped_add(
        communicate_secondary_offer_amount,
        communicate_secondary_offer_amount / 2);
}
if (PlayersData[faction_id].energy_reserves <
    communicate_secondary_offer_amount) {
    goto communicate_secondary_offer_debt; // native 0x00552EEB
}
communicate_secondary_action = 2; // native [EBP-0x18]
{
    const int random_ceiling = communicate_wrapped_add(
        communicate_imul_high_sar(GameDataStateStorage.fields_.turn_current_num_, 0x66666667u, 2, 2), 0);
    int amount = communicate_wrapped_add(communicate_secondary_offer_amount,
                           communicate_wrapped_mul(random(0, random_ceiling), 5));
    if (amount > PlayersData[faction_id].energy_reserves) {
        const int difference = communicate_wrapped_add(
            communicate_wrapped_sub(amount, PlayersData[faction_id].energy_reserves), 4);
        const uint32_t rounded_groups = static_cast<uint32_t>(
            (static_cast<uint64_t>(static_cast<uint32_t>(difference)) *
             0xCCCCCCCDu) >> 34);
        amount = communicate_wrapped_add(amount, communicate_wrapped_mul(
            -static_cast<int>(rounded_groups), 5));
    }
    parse_num(0, amount); // native ignores EAX
    communicate_secondary_offer_amount = amount;
    if (amount <= 0) {
        communicate_secondary_action = 0;
        goto communicate_secondary_offer_debt;
    }
}
goto communicate_secondary_offer_text; // native 0x00552F36
communicate_secondary_offer_debt: // native 0x00552EEB
if (DiploScratchFAA8 >= 0 && GameDataStateStorage.fields_.turn_current_num_ > 25 &&
    (GameDataStateStorage.fields_.game_rules_ & 0x02000000u) == 0) {
    communicate_secondary_action = 1;
    clear();
    say_tech(DiploScratchFAA8, 1);
    parse_says(6, StringTemp, -1, -1);
}
communicate_secondary_offer_text: // native 0x00552F36
if (communicate_secondary_best_tech >= 0) {
    parse_say(8, Technology[communicate_secondary_best_tech].name, -1, -1);
}
clear();
if ((Players[DiploScratchFA38].rule_flags & RFLAG_ALIEN) != 0) {
    says("METALIEN");
} else {
    says("METFRIEND");
}
say_num(communicate_secondary_action);
if (communicate_secondary_best_tech >= 0) says("a");
if (X_pops(StringTemp, 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
    goto communicate_menu_continuation; // native 0x00552FCD
}
net_treaty_on(faction_id, DiploScratchFA38, 8, 0);
net_treaty_on(faction_id, peer_id, 0x01000000, 0);
if (communicate_secondary_action == 2) {
    cause_friction(peer_id, faction_id, -1);
    net_energy(faction_id, -communicate_secondary_offer_amount,
               peer_id, communicate_secondary_offer_amount, 1);
    goto communicate_menu_continuation; // native 0x0055301B
}
if (communicate_secondary_action != 1) {
    goto communicate_menu_continuation; // native 0x00552FFC
}
cause_friction(peer_id, faction_id, -1);
net_tech(peer_id, DiploScratchFAA8, faction_id, 1);
DiploScratchFAA8 = -1;
goto communicate_menu_continuation; // native 0x00553045
// INCOMPLETE PRIVATE BODY SECTION: pinned native 0x0055304A..0x00553464.
// The initial comparison is against the EBX value live at 0x55304A; that
// value is deliberately a caller-supplied native live-in until its earlier
// reaching definition is composed.  The rest of this section translates the
// selected-map transfer and the separate introduction fallback.
communicate_map_bridge: // native 0x0055304A
communicate_secondary_third_party_exit: // same native entry
if (DiploScratchF7C8 == communicate_native_ebx_at_55304a) {
    goto communicate_introduction_fallback; // native 0x0055326C
}
if (has_treaty(faction_id, peer_id, 0x30) != 0 ||
    has_treaty(faction_id, DiploScratchF7C8, 0x10000001) != 0) {
    goto communicate_introduction_fallback; // native 0x0055326C
}
communicate_map_mask = static_cast<uint32_t>(1u) <<
    (static_cast<uint32_t>(DiploScratchF7C8) & 31u);
if ((PlayersData[faction_id].traded_maps & communicate_map_mask) != 0) {
    goto communicate_introduction_fallback; // native 0x0055326C
}
if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
     static_cast<uint8_t>(communicate_map_mask)) != 0 &&
    (PlayersData[peer_id].traded_maps & communicate_map_mask) == 0) {
    goto communicate_introduction_fallback; // native 0x0055326C
}
if (has_treaty(faction_id, DiploScratchF7C8, 0x08000000) == 0 ||
    DiploScratchFAA8 < 0 || wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_introduction_fallback; // native 0x0055326C
}
net_maps(peer_id, DiploScratchF7C8, 1); // native 0x00553114
parse_says(2, get_title(DiploScratchF7C8), -1, -1); // native 0x00553126
parse_says(3, get_name(DiploScratchF7C8), -1, -1); // native 0x00553145
parse_says(4, get_adjective_insult_faction(DiploScratchF7C8), -1, -1);
parse_says(5, get_adjective(DiploScratchF7C8), -1, -1);
clear();
say_tech(DiploScratchFAA8, 1);
parse_says(6, StringTemp, -1, -1);
net_set_treaty(faction_id, DiploScratchF7C8, 0x10000000, 1,
               communicate_native_ebx_at_55304a);
if (X_pops(
        (Players[DiploScratchF7C8].rule_flags & RFLAG_ALIEN) != 0
            ? "BOTHFRIENDALIEN" : "BOTHFRIEND",
        0x00100000, FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
    goto communicate_menu_continuation; // native 0x00553203
}
net_treaty_on(faction_id, peer_id, 0x01000000, 0);
net_maps(faction_id, peer_id, 1);
// Preserve the unusual native argument order at 0x55322F: the second
// argument is the tech ID and the third is the actor faction ID.
net_tech(peer_id, DiploScratchFAA8, faction_id, 1);
DiploScratchFA48 = 1;
DiploScratchFAA8 = DiploScratchFA18;
DiploScratchFA18 = -1;
communicate_stack_20 = 1; // native 0x553253
DiploScratchF7D8 = 0;
goto communicate_menu_continuation; // native 0x00553260
communicate_introduction_fallback: // native 0x0055326C
if (has_treaty(faction_id, peer_id, 0x30) != 0 ||
    has_project(4, peer_id) != 0) {
    goto communicate_menu_continuation; // native 0x00553464
}
communicate_intro_amount = communicate_wrapped_mul(
    communicate_wrapped_add(communicate_imul_high_sar(
        GameDataStateStorage.fields_.turn_current_num_, 0x51EB851Fu, 3, 1), 0), 10);
if (PlayersData[peer_id].energy_reserves < communicate_intro_amount) {
    goto communicate_menu_continuation; // native 0x005532CA
}
communicate_intro_candidate = 1; // native EBX at 0x005532D0
for (; communicate_intro_candidate < 8; ++communicate_intro_candidate) {
    const uint8_t human_bits = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
    const uint8_t candidate_bit = static_cast<uint8_t>(
        static_cast<uint32_t>(1u) <<
        (static_cast<uint32_t>(communicate_intro_candidate) & 31u));
    if ((human_bits & candidate_bit) == 0 ||
        communicate_intro_candidate == faction_id ||
        communicate_intro_candidate == peer_id) {
        continue;
    }
    if (has_treaty(faction_id, communicate_intro_candidate, 8) == 0 ||
        has_treaty(peer_id, communicate_intro_candidate, 8) != 0 ||
        has_treaty(peer_id, communicate_intro_candidate, 0x10000000) != 0 ||
        wants_to_attack(communicate_intro_candidate, peer_id, 0) != 0 ||
        wants_to_attack(peer_id, communicate_intro_candidate, 0) != 0) {
        continue;
    }
    break; // native 0x0055334B
}
if (communicate_intro_candidate >= 8 ||
    wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_menu_continuation; // native 0x00553357
}
parse_says(0, get_title(communicate_intro_candidate), -1, -1);
parse_says(1, get_name(communicate_intro_candidate), -1, -1);
parse_says(2, get_adjective(communicate_intro_candidate), -1, -1);
communicate_intro_amount = communicate_wrapped_mul(
    communicate_wrapped_add(communicate_imul_high_sar(
        GameDataStateStorage.fields_.turn_current_num_, 0x51EB851Fu, 3, 1), 0), 10);
communicate_stack_c = communicate_intro_amount; // native [EBP+0x0C] store at 0x5533C3
parse_num(0, communicate_intro_amount); // native result ignored
if ((Players[peer_id].rule_flags & RFLAG_ALIEN) != 0) {
    if (X_pops("ANYALIENNEWS", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
        goto communicate_menu_continuation; // native 0x00553404
    }
} else if (X_pops("ANYNEWS", 0x00100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait) == 0) {
    goto communicate_menu_continuation; // native 0x00553424
}
net_set_treaty(peer_id, communicate_intro_candidate, 0x10000000, 1, 0);
net_treaty_on(peer_id, communicate_intro_candidate, 8, 0);
net_energy(peer_id, -communicate_stack_c, faction_id,
           communicate_stack_c, 1);
cause_friction(peer_id, faction_id, -2);
communicate_stack_14 = 1; // native 0x553455
goto communicate_menu_continuation; // native 0x00553464
// INCOMPLETE PRIVATE BODY SECTION: native 0x00553464..0x00553580.
// Requires function-scope int locals for the selected-base scan. The Base
// identity/population fields are typed owners; no guessed array alias is used.
communicate_menu_continuation: // native 0x00553464
communicate_post_topic_menu_gates:
if (has_treaty(peer_id, faction_id, 0x00000003) != 0 ||
    DiploScratchFA98 != 0 ||
    has_treaty(faction_id, peer_id, 0x02000000) != 0 ||
    DiploScratchFA88 <= 0 || DiploScratchFA60 != 0 ||
    DiploScratchFA70 != 0 || IsMultiplayerNet != 0 ||
    communicate_stack_20 != 0 ||
    PlayersData[peer_id].mil_strength_1 <=
        PlayersData[faction_id].mil_strength_1) {
    goto communicate_after_proposal_menu; // native 0x0055362D
}
communicate_captured_base_index = -1;
communicate_captured_base_population_limit = 999;
for (communicate_captured_base_scan = 0;
     communicate_captured_base_scan < GameDataStateStorage.fields_.base_current_count_;
     ++communicate_captured_base_scan) {
    const Base &candidate = Bases[communicate_captured_base_scan];
    if (candidate.faction_id_current == faction_id &&
        candidate.faction_id_former == peer_id &&
        candidate.population_size <
            communicate_captured_base_population_limit) {
        communicate_captured_base_population_limit =
            candidate.population_size;
        communicate_captured_base_index = communicate_captured_base_scan;
    }
}
if (communicate_captured_base_index < 0 ||
    wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_after_proposal_menu; // native 0x0055362D
}
goto communicate_strong_base_topic; // native 0x00553580
// INCOMPLETE PRIVATE BODY SECTION: native 0x00553580..0x0055362D.
// Requires the selected-base automatic from captured-base-leverage-gates.
communicate_strong_base_topic: // native 0x00553580
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, Bases[communicate_captured_base_index].name_string, -1, -1);
communicate_captured_base_offer_result = X_pops(
    "RANSOMDEMAND", 0x00100000, FactionDiplomacyPortraits[peer_id], pop_wait);
if (communicate_captured_base_offer_result != 0) {
    communicate_stack_20 = 1; // native [EBP-0x20]
    net_cede_base(communicate_captured_base_index, peer_id, 1);
    net_set_treaty(faction_id, peer_id, 0x02000000, 0, 0);
    net_treaty_off(peer_id, faction_id, 0x00000020, 1);
    diplomacy_caption(faction_id, peer_id);
} else {
    communicate_stack_38 = 1; // native [EBP-0x38]
}
goto communicate_after_proposal_menu; // native 0x0055362D
// INCOMPLETE PRIVATE BODY SECTION: native 0x0055362D..0x0055371A.
// `tech_trade` remains a separately recovered source dependency; call order
// and five native arguments below come from the pinned direct call site.
communicate_post_ransom_call_gates: // native 0x0055362D
communicate_tech_greeting_done: // same native entry
communicate_shared_dialogue_outro: // same native entry
communicate_protective_treaty_done: // same native entry
communicate_after_proposal_menu:
communicate_relation_auxiliary_result = 0; // 553649
if ((GameDataStateStorage.fields_.game_rules_ & 0x02000000u) == 0) {
    tech_trade(faction_id, peer_id, 1, 4, 4);
}
if (great_satan(faction_id, 0) != 0 ||
    static_cast<int>(PlayersData[faction_id].ranking) < 6 ||
    static_cast<int>(PlayersData[peer_id].ranking) > 4 || DiploScratchF7F0 != 0 ||
    DiploScratchFA60 != 0 || DiploScratchFA70 != 0 ||
    has_agenda(faction_id, peer_id, 0x00002000) != 0) {
    goto communicate_after_contact_relations; // native 0x0055373A
}
if (has_treaty(faction_id, peer_id, 1) != 0 &&
    ((GameDataStateStorage.fields_.game_rules_ & 0x00001000u) != 0 || DiploScratchF808 != 0)) {
    goto communicate_after_contact_relations; // native 0x0055373A
}
if (DiploScratchFA88 != 0) {
    goto communicate_after_contact_relations; // native 0x0055373A
}
communicate_stack_1c = 1; // native [EBP-0x1C]
if (PlayersData[faction_id].energy_reserves < 125) {
    goto communicate_after_contact_relations; // native 0x0055373A
}
// Native 0x0055371A.. continues with a signed reserve-derived threshold and
// additional contact/topic dispatch; that arithmetic is not in this fragment.
// INCOMPLETE PRIVATE BODY SECTION: native 0x0055371A..0x005538D9.
// The entry gate guarantees positive reserves >=125, so the native signed
// divide sequence is represented by equivalent positive integer arithmetic.
communicate_positive_reserves = PlayersData[faction_id].energy_reserves;
DiploScratchFA88 = communicate_round_down_multiple(communicate_positive_reserves / 5, 25);
communicate_after_contact_relations: // native 0x0055373A continuation
if (mode != 0) {
    if (has_treaty(peer_id, faction_id, 0x00000010) == 0) {
        DiploScratchFA88 = 0;
        DiploScratchFA60 = 0;
        goto communicate_post_contact_preparation; // native 0x00553D24
    }
} else if (has_treaty(faction_id, peer_id, 1) != 0 &&
           communicate_stack_20 == 0 &&
           has_agenda(faction_id, peer_id, 0x00002000) == 0 &&
           (GameDataStateStorage.fields_.game_rules_ & 0x00001000u) == 0 && DiploScratchF808 == 0) {
    if ((DiploScratchFA64 != 0 && DiploScratchFA90 == 0) ||
        DiploScratchFA78 > 4 || DiploFriction > 8) {
        communicate_stack_1c = 1;
    }
}
if (DiploScratchFA88 <= 0 ||
    PlayersData[faction_id].energy_reserves < DiploScratchFA88 ||
    communicate_stack_20 != 0 || DiploScratchFA50 != 0 ||
    (DiploScratchFAA0 != 0 && DiploScratchFA18 >= 0) ||
    has_agenda(faction_id, peer_id, 0x00002008) != 0 ||
    DiploScratchFA60 != 0 || DiploScratchFA80 != 0) {
    goto communicate_post_contact_preparation; // native 0x00553D24
}
communicate_relation_auxiliary_result = faction_id; // 5537DD: row-offset zero test
communicate_holds_treaty_with_peer =
    has_treaty(faction_id, peer_id, 1) != 0;
if (communicate_holds_treaty_with_peer && DiploScratchFAA8 >= 0) {
    if (DiploScratchFA60 == 0) {
        goto communicate_post_contact_preparation; // native 0x00553D24
    }
    if (DiploScratchFA70 == 0) {
        goto communicate_contact_relation_topic; // native 0x005538AD
    }
} else if (DiploScratchFA60 != 0 && DiploScratchFA70 == 0) {
    goto communicate_contact_relation_topic; // native 0x005538AD
}
if (communicate_stack_38 == 0 &&
    (communicate_stack_1c == 0 ||
     PlayersData[faction_id].energy_reserves < 125 ||
     DiploScratchFAA8 >= 0)) {
    goto communicate_post_contact_preparation; // native 0x00553D24
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_post_contact_preparation; // native 0x00553D24
}
if (has_treaty(faction_id, peer_id, 0x00000011) == 0) {
    pact_attacks(faction_id, peer_id);
}
communicate_contact_relation_topic: // native 0x005538AD
// Native begins composing a relation-specific dialogue at 0x005538D9.
// INCOMPLETE PRIVATE BODY SECTION: native 0x005538D9..0x00553B0D.
// Builds the relation dialogue's seven ordered parser slots, numeric offer
// phrases and the native 0..6 random seed plus relationship overrides.
communicate_relation_topic_compose: // native 0x005538D9
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_title(peer_id), -1, -1);
parse_says(3, get_name(peer_id), -1, -1);
parse_says(4, get_adjective(peer_id), -1, -1);
parse_says(5, get_pact(faction_id), -1, -1);
parse_says(6, get_noun(peer_id), -1, -1);
parse_num(0, DiploScratchFA88);
communicate_bribe_phrase_amount = (DiploScratchFA88 / 20) * 10;
if (communicate_bribe_phrase_amount == 0) {
    communicate_bribe_phrase_amount = 1;
}
communicate_relation_auxiliary_result = communicate_bribe_phrase_amount;
parse_num(1, communicate_bribe_phrase_amount);
clear();
says("DEMANDBRIBE");
communicate_stack_c = random(0, 4);
communicate_stack_14 = 1; // native [EBP-0x14]
if (communicate_stack_1c != 0) {
    communicate_stack_c = 8;
}
if (has_treaty(peer_id, faction_id, 0x00000020) != 0 &&
    DiploScratchFA98 == 0 &&
    PlayersData[peer_id].mil_strength_1 <
        PlayersData[faction_id].mil_strength_1) {
    communicate_stack_c = 9;
}
if (great_satan(faction_id, 0) != 0) {
    communicate_peer_strength_third =
        communicate_wrapped_mul(PlayersData[peer_id].mil_strength_1, 4) / 3;
    if (PlayersData[faction_id].mil_strength_1 >
            communicate_peer_strength_third &&
        DiploScratchFA68 == 2) {
        communicate_stack_c = 7;
    }
}
if (has_treaty(faction_id, peer_id, 1) != 0) {
    communicate_stack_c = 6;
}
if (has_treaty(peer_id, faction_id, 0x00000010) != 0) {
    communicate_stack_c = mode != 0 ? 4 : 5;
}
if (has_treaty(peer_id, faction_id, 0x00000040) != 0) {
    communicate_stack_c = 10;
}
if (DiploScratchFA60 == 0 && DiploScratchFA68 == 0) {
    communicate_stack_48 = 1; // native [EBP-0x48]
}
communicate_stack_3c = 1; // native [EBP-0x3C]
say_num(communicate_stack_c);
// Native 0x00553B0D dispatches 0..6 through its internal jump table; cases
// 7..10 and their text/callback branches continue in a separate fragment.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00553B14..0x00553C9E.
// Native jump-table cases 0..6 override the seven-slot greeting with
// relationship-specific adjective/noun-phrase text before the shared popup.
switch (communicate_stack_c) {
case 0:
    parse_says(5, get_adjective_leader(faction_id), -1, -1);
    parse_says(6, get_insult_leader(peer_id), -1, -1);
    parse_says(8, get_noun_phrase(peer_id, 11), -1, -1);
    parse_says(9, get_phrase(peer_id, 12), -1, -1);
    break;
case 1:
    parse_says(8, get_noun_phrase(faction_id, 9), -1, -1);
    parse_says(9, get_phrase(peer_id, 13), -1, -1);
    break;
case 2:
    parse_says(6, get_noun_phrase(peer_id, 10), -1, -1);
    parse_says(8, get_noun_phrase(faction_id,
                             has_treaty(peer_id, faction_id, 0x20) != 0
                                 ? 8 : 9), -1, -1);
    parse_says(9, get_phrase(peer_id, 1), -1, -1);
    break;
case 3:
    parse_says(9, get_phrase(peer_id, 14), -1, -1);
    break;
case 5:
    if (DiploScratchFA98 == 0 &&
        PlayersData[peer_id].current_num_bases >
            PlayersData[faction_id].current_num_bases) {
        says("HALFBRIBE");
    } else if (has_treaty(peer_id, faction_id, 0x20) != 0) {
        clear();
        says("DEMANDBRIBE9");
    } else {
        says("DEMANDBRIBE");
    }
    break;
case 6:
    parse_says(9, get_noun_phrase(peer_id, 9), -1, -1);
    break;
default:
    // The native table's values 7..10 join directly at 0x00553C76.
    break;
}
communicate_relation_popup_result = X_pops(
    StringTemp, 0x00100000, FactionDiplomacyPortraits[peer_id], pop_wait);
communicate_relation_auxiliary_result = communicate_relation_popup_result;
if (communicate_relation_popup_result != 2) {
    goto communicate_relation_popup_nonselection; // native 0x00553EAB
}
goto communicate_relation_choice_popup_result_two;
// INCOMPLETE PRIVATE BODY SECTION: native 0x00553C9E..0x00553F11.
// Completes the result-2/case-6 relation popup detour, then rejoins the
// directed energy transfer. Requires function-scope `int
// communicate_relation_popup_result` and `communicate_relation_selected_amount`.
communicate_relation_choice_popup_result_two: // native 0x00553C9E
if (communicate_stack_c != 6) {
    goto communicate_relation_general_followup; // native 0x00553DDB
}
if (random(0, 2) == 0) {
    goto communicate_relation_store_input_amount; // native 0x00553EE7
}
if (DiploScratchFA60 != 0 ||
    (DiploScratchFA64 != 0 && DiploScratchFA90 != 0) ||
    DiploScratchFA50 != 0 || DiploScratchFAA0 != 0) {
    X_pops("WEASELOUT", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
    goto communicate_relation_choice_cancelled; // native 0x00553D0C
}
goto communicate_relation_store_input_amount; // native 0x00553EE7
communicate_relation_choice_cancelled: // native 0x00553D0C
net_treaty_off(peer_id, faction_id, 0x01000000, 0);
communicate_stack_38 = 1;
goto communicate_post_contact_preparation; // native 0x00553D24
communicate_relation_general_followup: // native 0x00553DDB
communicate_relation_auxiliary_result = 0;
if (great_satan(faction_id, 0) == 0 &&
    !(DiploScratchFA64 != 0 && DiploScratchFA90 != 0) &&
    DiploScratchFA50 == 0 && DiploScratchFAA0 == 0 &&
    has_treaty(faction_id, peer_id, 0x20) == 0) {
    goto communicate_relation_choose_energy_amount; // native 0x00553E5B
}
if (has_treaty(faction_id, peer_id, 0x400) != 0) {
    communicate_relation_auxiliary_result = 1;
} else {
    communicate_relation_auxiliary_result = random(0, 3);
}
if (random(0, 2) == 0) {
    goto communicate_relation_choice_cancelled; // native 0x00553EE7
}
communicate_relation_choose_energy_amount: // native 0x00553E5B
if (DiploScratchFA60 != 0) {
    if (communicate_relation_auxiliary_result == 0 && random(0, 2) != 0) {
        goto communicate_relation_prompt_input_amount; // native 0x00553EBA
    }
    parse_says(6, get_insult_leader(peer_id), -1, -1);
    communicate_relation_popup_result = X_pops(
        "WEASELOUT", 0x00100000,
        FactionDiplomacyPortraits[peer_id], pop_wait);
    if (communicate_relation_popup_result == 0) {
        goto communicate_relation_choice_cancelled; // native 0x00553D0C
    }
    // Native reloads the callback-visible amount; the popup result only gates it.
} else {
    goto communicate_relation_prompt_input_amount; // native 0x00553EBA
}
goto communicate_relation_amount_join; // native 0x00553EEE
communicate_relation_popup_nonselection: // native 0x00553EAB
if (communicate_relation_popup_result == 0) {
    goto communicate_relation_choice_cancelled; // native 0x00553D0C
}
// Native reloads the callback-visible amount; the popup result only gates it.
goto communicate_relation_amount_join; // native 0x00553EEE
communicate_relation_prompt_input_amount: // native 0x00553EBA
parse_num(2, communicate_bribe_phrase_amount);
(void)X_pops("WEASELEDOUT", 0x00100000,
                   FactionDiplomacyPortraits[peer_id], pop_wait);
communicate_relation_auxiliary_result = communicate_bribe_phrase_amount;
communicate_relation_store_input_amount: // native 0x00553EE7
DiploScratchFA88 = communicate_relation_auxiliary_result;
communicate_relation_amount_join: // native 0x00553EEE
communicate_stack_20 = 1;
communicate_relation_selected_amount = DiploScratchFA88;
goto communicate_relation_choice_transfer;
// Fall through to the native 0x00553EFD field adjustment and 0x00553F11
// net_energy call in relation-choice-transfer-553f11-553f68.inc.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00553D24..0x00553DDB.
// The first route depends on entry contact flags and the preceding popup's
// stack/global state; it rejoins later response preparation at 0x00553F6D.
communicate_post_contact_preparation: // native 0x00553D24
if (!early_contact_path || communicate_stack_20 != 0 ||
    (DiploScratchFAA8 < 0 && communicate_stack_38 == 0) ||
    DiploScratchFA70 != 0 || DiploScratchFA98 != 0) {
    goto communicate_relation_fa60_gate; // native 0x00553FC5
}
if (DiploScratchFA68 == 0) {
    if ((DiploScratchF7FC == 0 && static_cast<int>(PlayersData[peer_id].ranking) < 6) ||
        static_cast<int>(PlayersData[faction_id].ranking) < 6 || DiploScratchF7F0 != 0) {
        goto communicate_relation_fa60_gate; // native 0x00553FC5
    }
}
if ((GameDataStateStorage.fields_.game_rules_ & 0x00000040u) != 0) {
    goto communicate_relation_priority_553f84;
}
goto communicate_relation_priority_553f6d;
// INCOMPLETE PRIVATE BODY SECTION: native 0x00553F11..0x00553F68.
// This is the accepted/cancelled relationship-choice transfer tail.  The
// selected amount in EAX is paid by the actor and credited to the peer; the
// outer relation/menu continuation resumes at 0x00553D24.
communicate_relation_choice_transfer: // native 0x00553F11
if (communicate_stack_c == 10 &&
    PlayersData[faction_id].diplo_unk1[peer_id] != 0) {
    --PlayersData[faction_id].diplo_unk1[peer_id];
}
net_energy(faction_id, communicate_wrapped_sub(0, communicate_relation_selected_amount), peer_id,
           communicate_relation_selected_amount, 1);
ConsoleInfoWin.set_redraw_phase(0);
ConsoleGlobal.update_data(0);
diplomacy_caption(faction_id, peer_id);
net_treaty_on(peer_id, faction_id, 0x05000000, 0);
if (communicate_stack_50 != 0) {
    net_treaty_on(peer_id, faction_id, 0x00000004, 1);
}
goto communicate_post_contact_preparation; // native 0x00553D24
// INCOMPLETE PRIVATE BODY SECTION: native 0x00553F6D..0x00553FDD.
// When no prior relation result was selected, the image derives the
// relation-priority flag from the two faction rankings, peer reserves and
// the local reserve threshold. CDQ/SUB/SAR at 0x00553FAB..0x00553FB2 is
// equivalent to signed division truncated toward zero.
communicate_relation_priority_553f6d:
{
    const int priority_rank = PlayersData[faction_id].diff_level;
    const int rank_delta = communicate_wrapped_sub(5, priority_rank);
    const int rounded_rank_delta = rank_delta / 2;
    const int relation_reserve_score = communicate_wrapped_add(
        DiploFriction,
        communicate_wrapped_mul(PlayersData[peer_id].energy_reserves, 2));
    const int relation_reserve_limit = communicate_wrapped_add(10, rounded_rank_delta);
    if (relation_reserve_score > relation_reserve_limit) {
        DiploScratchFA60 = 1;
        goto communicate_post_priority_553fdd;
    }
}
goto communicate_relation_fa60_gate; // native 0x00553FC5
communicate_relation_priority_553f84:
// Native 0x00553DD1 injects ECX=5 and enters the shared computation at
// 0x00553F84. The regular entry at 0x00553F6D supplies actor ranking instead.
{
    const int relation_reserve_score = communicate_wrapped_add(
        DiploFriction,
        communicate_wrapped_mul(PlayersData[peer_id].energy_reserves, 2));
    if (relation_reserve_score > 10) {
        DiploScratchFA60 = 1;
        goto communicate_post_priority_553fdd;
    }
}
goto communicate_relation_fa60_gate; // native 0x00553FC5
// INCOMPLETE PRIVATE BODY SECTION: native 0x00553FDD..0x005540E7.
// `communicate_relation_auxiliary_result` is an opaque live-in for EBX; its
// production is in the still-untranslated popup/action dispatch preceding
// this section. It is deliberately not interpreted as a new flag here.
communicate_relation_fa60_gate: // original 553FC5
if (DiploScratchFA60 == 0 || communicate_stack_20 != 0)
    goto communicate_post_relation_menu;
communicate_post_priority_553fdd:
if (DiploScratchFA60 == 0 || communicate_stack_20 != 0 ||
    communicate_stack_38 != 0 || communicate_relation_auxiliary_result != 0 ||
    ((DiploScratchFA50 != 0 || DiploScratchFAA0 != 0) &&
     DiploScratchFA18 >= 0) ||
    DiploScratchFA88 <= PlayersData[faction_id].energy_reserves) {
    goto communicate_post_relation_menu; // native 0x005540E7
}
if (great_beelzebub(faction_id, 0) == 0) {
    if (static_cast<int>(PlayersData[faction_id].ranking) < 6 ||
        (DiploScratchF7FC == 0 && static_cast<int>(PlayersData[peer_id].ranking) < 6)) {
        goto communicate_post_relation_menu; // native 0x005540E7
    }
}
communicate_relation_offer_multiplier =
    (GameDataStateStorage.fields_.game_rules_ & 0x00000040u) != 0 ? 2 : 3;
if (great_satan(faction_id, 0) == 0) {
    ++communicate_relation_offer_multiplier;
}
communicate_relation_offer_tech_value =
    tech_val(DiploScratchFAA8, peer_id, 1);
communicate_relation_offer_turn_scale =
    communicate_wrapped_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8;
communicate_relation_offer_value = communicate_wrapped_mul(
    communicate_wrapped_mul(communicate_relation_offer_tech_value,
              communicate_relation_offer_turn_scale),
    communicate_relation_offer_multiplier) / 2;
if (DiploScratchFA88 > communicate_relation_offer_value) {
    DiploScratchFAA8 = -1;
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
goto communicate_post_relation_menu; // native 0x005540E7
// INCOMPLETE PRIVATE BODY SECTION: native 0x005540E7..0x005542CB.
// This is the relationship-offer/menu gate chain after the earlier tech-value
// threshold. `communicate_later_action_dispatch` is the native 0x554A4C join.
// The live selection locals and Popup remain owned by the enclosing body.
communicate_post_relation_menu: // native 0x005540E7
if (DiploScratchFAA8 < 0 || DiploScratchFAA8 == 9999 ||
    communicate_stack_20 != 0 || communicate_stack_38 != 0 ||
    (GameDataStateStorage.fields_.game_rules_ & 0x02000000u) != 0 ||
    (DiploScratchFAA0 != 0 && DiploScratchFA18 < 0)) {
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
if (DiploScratchFAA0 >= 2 &&
    PlayersData[peer_id].tech_ranking / 2 + DiploScratchFAA0 >=
        PlayersData[faction_id].tech_ranking / 2) {
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
if (has_agenda(peer_id, faction_id, 0x00002008) != 0) {
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
if (has_treaty(peer_id, faction_id, 0x00000040) != 0 &&
    DiploScratchFA18 < 0 &&
    static_cast<int>(PlayersData[peer_id].ranking) <= static_cast<int>(PlayersData[faction_id].ranking)) {
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
if (DiploScratchFA60 == 0 && DiploScratchFA80 != 0) {
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
if (DiploScratchFA98 == 0 &&
    has_treaty(faction_id, peer_id, 0x00000001) == 0 &&
    communicate_stack_1c == 0) {
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
// The native compares the actors' commerce bonuses. Only when the peer is
// strictly ahead does it apply the selected-faction, stack and treaty gates.
if (PlayersData[peer_id].tech_commerce_bonus >
        PlayersData[faction_id].tech_commerce_bonus &&
    BaseIDCurrentSelected != faction_id &&
    communicate_stack_1c == 0 &&
    has_treaty(peer_id, faction_id, 0x00000020) == 0 &&
    (has_treaty(peer_id, faction_id, 0x00000002) != 0 ||
     communicate_stack_44 != 0)) {
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_relation_later_action_dispatch; // native 0x00554A4C
}
if (has_treaty(peer_id, faction_id, 0x00000011) == 0) {
    mention_prototypes(faction_id, peer_id);
}
// Continue at native 0x5542CB with the gate sequence leading into the
// optional truce scoring and menu construction.
// INCOMPLETE PRIVATE BODY SECTION: native 0x005542CB..0x00554311.
// The native 0x628A50 target is the existing typed swap(int*, int*) owner.
communicate_relation_menu_optional_swap: // native 0x005542CB
if (DiploScratchFA18 >= 0 && DiploScratchFA50 == 0 &&
    DiploScratchFAA0 == 0 && DiploScratchF7C4 != 0 &&
    has_treaty(peer_id, faction_id, 0x00000040) == 0) {
    swap(&DiploScratchFAA8, &DiploScratchFA18);
}
goto communicate_relation_menu_popup_setup; // native 0x00554311
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554311..0x005543E7.
// This builds the ordered technology/title/name/pact parser slots for the
// selected relation response before the DEMANDTECH menu's random choices.
communicate_relation_menu_popup_setup: // native 0x00554311
clear();
say_tech(DiploScratchFAA8, 1);
parse_says(0, StringTemp, -1, -1);
parse_says(1, get_title(faction_id), -1, -1);
parse_says(2, get_name(faction_id), -1, -1);
parse_says(3, get_title(peer_id), -1, -1);
parse_says(4, get_name(peer_id), -1, -1);
parse_says(5, get_pact(peer_id), -1, -1);
if (DiploScratchFA18 >= 0) {
    clear();
    say_tech(DiploScratchFA18, 1);
    parse_says(6, StringTemp, -1, -1);
}
clear();
says("DEMANDTECH"); // pinned 0x0068E0F4
// The menu's random-choice/default selection begins at native 0x005543ED.
// INCOMPLETE PRIVATE BODY SECTION: native 0x005543ED..0x00554493.
// Required function-scope automatic: int communicate_stack_c. This is
// the initial randomized choice plus the native FA60/FA68/treaty overrides.
communicate_relation_menu_choice_setup: // native 0x005543ED
communicate_stack_c = random(0, 3);
if (DiploScratchFA60 == 0) {
    if (DiploScratchFA68 == 0) {
        communicate_stack_48 = 1;
    }
    communicate_stack_c = random(0, 3) + 9;
    if (has_treaty(peer_id, faction_id, 0x00000020) == 0 &&
        communicate_stack_1c == 0) {
        communicate_stack_c = 8;
    }
}
if (great_satan(faction_id, 0) != 0 || communicate_stack_1c != 0) {
    communicate_stack_c = 7;
}
if (has_treaty(peer_id, faction_id, 0x00000010) != 0) {
    communicate_stack_c =
        PlayersData[peer_id].diplo_unk4[faction_id] <=
                PlayersData[faction_id].diplo_unk4[peer_id]
            ? 3 : 4;
}
// Native treaty-bit 0x01 and the remaining higher-priority choices follow at
// 0x00554493; do not fold that later override into this section.
// Sequential continuation is the native post-social-treaty point.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554493..0x0055456C.
// Applies the directed treaty and relationship-count overrides before
// `say_num` serializes the selected native menu choice.
communicate_relation_menu_treaty_overrides: // native 0x00554493
if (has_treaty(peer_id, faction_id, 0x00000001) != 0) {
    communicate_stack_c = 5;
}
if (has_treaty(peer_id, faction_id, 0x00000040) != 0 &&
    has_treaty(peer_id, faction_id, 0x00000020) == 0) {
    const int received_actions =
        PlayersData[faction_id].diplo_unk1[peer_id];
    if (received_actions != 0 &&
        received_actions > PlayersData[faction_id].diplo_wrongs[peer_id]) {
        if (DiploScratchF7C0 >= 0) {
            parse_says(8, get_title(DiploScratchF7C0), -1, -1);
            communicate_stack_c = 14;
        }
        if (DiploScratchFA28 >= 0) {
            parse_says(9, get_title(DiploScratchFA28), -1, -1);
            communicate_stack_c = 15;
        }
    }
    if (communicate_stack_c < 14) {
        communicate_stack_c = DiploScratchFA18 < 0 ? 13 : 12;
    }
}
say_num(communicate_stack_c); // native 0x0055456C
// INCOMPLETE PRIVATE BODY SECTION: native 0x0055456C..0x005545D1.
// Required function-scope automatic: int communicate_relation_popup_result.
// The low relationship choices use their original phrase selector; choices
// 9..11 continue into the separately translated custom amount popup.
communicate_relation_menu_phrase_dispatch: // native 0x0055456C
if (communicate_stack_c == 0) {
    parse_says(9, get_noun_phrase(faction_id, 8), -1, -1);
} else if (communicate_stack_c == 1) {
    parse_says(9, get_phrase(faction_id, 7), -1, -1);
} else if (communicate_stack_c == 2) {
    parse_says(9, get_phrase(faction_id, 6), -1, -1);
} else if (communicate_stack_c == 9 || communicate_stack_c == 10) {
    parse_says(9, get_noun_phrase(peer_id, 9), -1, -1);
    goto communicate_relation_menu_custom_amount; // native 0x00554624
} else if (communicate_stack_c == 11) {
    goto communicate_relation_menu_custom_amount; // native 0x00554624
}
goto communicate_relation_menu_common_popup; // native 0x005545B5
communicate_relation_menu_common_popup: // native 0x005545B5
communicate_relation_popup_result = X_pops(
    StringTemp, 0x00100000, FactionDiplomacyPortraits[peer_id], pop_wait);
communicate_relation_auxiliary_result = communicate_relation_popup_result;
communicate_relation_menu_popup_result_dispatch: // native 0x005545D1
if (communicate_relation_popup_result == 2 ||
    communicate_relation_popup_result == 4) {
    help_topic(14, DiploScratchFAA8);
    goto communicate_relation_menu_optional_swap; // native 0x005542C8
}
goto communicate_relation_menu_non_help_result; // native 0x00554813
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554624..0x00554704.
// Required function-scope automatics: int communicate_custom_amount,
// int communicate_custom_tech_count, int communicate_custom_tech_value, and
// int communicate_custom_text_open_result. The byte-indexed game-tech
// achievement vector and existing tech valuation/rounding helpers are used
// directly; no new table or input validation is introduced.
communicate_relation_menu_custom_amount: // native 0x00554624
communicate_custom_tech_count =
    bit_count(static_cast<int>(GameDataStateStorage.fields_.game_tech_achieved_[DiploScratchFAA8]));
communicate_custom_tech_value = tech_val(DiploScratchFAA8, faction_id, 1);
communicate_custom_amount =
    communicate_wrapped_mul(communicate_custom_tech_value,
              communicate_wrapped_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8) /
        communicate_wrapped_add(communicate_custom_tech_count, 1);
communicate_custom_amount = communicate_wrapped_add(communicate_custom_amount, 24);
communicate_custom_amount =
    communicate_round_down_multiple(communicate_custom_amount, 25);
parse_num(0, communicate_custom_amount);
popup.start(PopupStartCaption, 0x00100000);
popup.set_picture(FactionDiplomacyPortraits[peer_id], 0, 0);
says("A"); // pinned original literal 0x0068E100
communicate_custom_text_open_result =
    X_text_open(ScriptPopupFile, StringTemp);
if (communicate_custom_text_open_result != 0) {
    goto communicate_relation_menu_text_open_failed; // native 0x00554811
}
if (DiploScratchF800 >= 0) {
clear();
say_tech(DiploScratchF800, 1);
parse_says(6, StringTemp, -1, -1);
}
// Original 5546D0..554704 is covered above.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554704..0x00554813.
// Builds each original custom-tech text item in order, conditionally adding
// the amount and secondary-tech entries, then executes the existing Popup.
communicate_relation_menu_custom_text_items: // native 0x00554704
text_get();
clear();
parse_string(TextBufferGetPtr, StringTemp);
popup.item(StringTemp, 0);
text_get();
clear();
parse_string(TextBufferGetPtr, StringTemp);
popup.item(StringTemp, 1);
text_get();
clear();
parse_string(TextBufferGetPtr, StringTemp);
if (communicate_custom_amount > 0) {
    popup.item(StringTemp, 2);
}
text_get();
clear();
parse_string(TextBufferGetPtr, StringTemp);
if (DiploScratchF800 >= 0) {
    popup.item(StringTemp, 3);
}
text_get();
clear();
parse_string(TextBufferGetPtr, StringTemp);
popup.item(StringTemp, 4);
communicate_relation_popup_result = popup.exec(pop_wait);
communicate_relation_auxiliary_result = communicate_relation_popup_result;
goto communicate_relation_menu_popup_result_dispatch; // native 0x005545D1
communicate_relation_menu_text_open_failed: // native 0x00554811
communicate_relation_popup_result = 0;
communicate_relation_auxiliary_result = communicate_relation_popup_result;
goto communicate_relation_menu_custom_response; // native 0x00554813
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554813..0x00554899.
// The popup result remains live in `communicate_relation_popup_result` while
// the fallback/response parser slots are rebuilt in native order.
communicate_relation_menu_non_help_result: // native 0x00554813
communicate_relation_menu_custom_response:
clear();
say_tech(DiploScratchFAA8, 1);
parse_says(0, StringTemp, -1, -1);
parse_says(1, get_title(faction_id), -1, -1);
parse_says(2, get_name(faction_id), -1, -1);
parse_says(3, get_title(peer_id), -1, -1);
parse_says(4, get_name(peer_id), -1, -1);
// Native branches on the retained Popup::exec result at 0x00554899.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554899..0x00554934.
// The callback result 2 is the energy-payment response; result 3 is a
// separate technology-transfer response. Function locals from the custom
// amount/text sections remain live here.
communicate_relation_menu_custom_response_dispatch: // native 0x00554899
if (communicate_relation_popup_result == 2) {
    const int actor_half_reserves = PlayersData[faction_id].energy_reserves / 2;
    const int peer_tech_value = tech_val(DiploScratchFAA8, peer_id, 1);
    const int peer_turn_value = communicate_wrapped_mul(
        peer_tech_value, communicate_wrapped_add(GameDataStateStorage.fields_.turn_current_num_, 7) / 8);
    if (actor_half_reserves >= communicate_custom_amount &&
        peer_turn_value > communicate_custom_amount) {
        X_pops("AGREEDENERGY", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
        net_energy(peer_id, communicate_wrapped_sub(0, communicate_custom_amount),
                   faction_id, communicate_custom_amount, 0);
        goto communicate_later_function_return; // native 0x00554E74
    }
    communicate_relation_popup_result = X_pops(
        "DEMANDTECHAGAIN1", 0x00100000,
        FactionDiplomacyPortraits[peer_id], pop_wait);
    if (communicate_relation_popup_result != 0) {
        goto communicate_later_function_return; // native 0x00554E74
    }
    goto communicate_custom_decline_effects; // native 0x005549E7
}
if (communicate_relation_popup_result != 3) {
    goto communicate_later_action_dispatch; // native 0x00554E6C
}
goto communicate_custom_tech_response_three; // native 0x00554934
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554934..0x005549E7.
// Result 3 offers the queued secondary technology. The native's wrapped
// two-thirds relative-value comparison and both ordered popup/net branches
// are kept intact.
communicate_custom_tech_response_three: // native 0x00554934
if (DiploScratchFA68 == 0) {
    const int offered_tech_value = tech_val(DiploScratchF800, peer_id, 1);
    const int scaled_offered_value = communicate_wrapped_mul(offered_tech_value, 2) / 3;
    const int requested_tech_value = tech_val(DiploScratchFAA8, peer_id, 1);
    if (scaled_offered_value <= requested_tech_value) {
        X_pops("AGREEDTECH", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
        net_tech(faction_id, DiploScratchF800, peer_id, 0);
        DiploScratchF800 = -1;
        goto communicate_later_function_return; // native 0x00554E74
    }
}
communicate_relation_popup_result = X_pops(
    "DEMANDTECHAGAIN2", 0x00100000,
    FactionDiplomacyPortraits[peer_id], pop_wait);
if (communicate_relation_popup_result != 0) {
    goto communicate_later_function_return; // native 0x00554E74
}
goto communicate_custom_decline_effects; // native 0x005549E7
// INCOMPLETE PRIVATE BODY SECTION: native 0x005549E7..0x00554A4C.
// The native sets EBX to -1 before the treaty test at 0x554A08, so the
// following EBX==0 friction/patience arm is unreachable from this popup
// continuation. Preserve the pure treaty predicate call, treaty-off order,
// stack flag, and diplomacy_check before the 0x554A4C shared join.
communicate_custom_decline_effects: // native 0x005549E7
communicate_large_offer_zero_limit: // native interior entry 0x005549EA
net_treaty_off(peer_id, faction_id, 0x01000000, 0);
communicate_stack_38 = 1;
(void)has_treaty(faction_id, peer_id, 0x00000001);
diplomacy_check(faction_id, peer_id, 1);
// Fall through to native 0x00554A4C relation/loan eligibility.
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554A4C..0x00554C2C.
// This relation-specific branch preserves the exact treaty argument order,
// the peer's faction-file check, both directed loan balances and the
// reserve-ratio gates. It rejoins the later payment calculation at 0x554C2C.
communicate_later_event_loop: // native 0x00554A42
diplomacy_caption(faction_id, peer_id);
communicate_relation_later_action_dispatch: // native 0x00554A4C
communicate_large_offer_eligibility:
if (has_treaty(faction_id, peer_id, 0x00000001) != 0) {
    ++DiploScratchF808;
}
if (strcmp(Players[peer_id].filename, "MORGAN") != 0) {
    ++DiploScratchF808;
}
communicate_actor_reserves = PlayersData[faction_id].energy_reserves;
communicate_peer_reserves = PlayersData[peer_id].energy_reserves;
if (communicate_actor_reserves <= communicate_peer_reserves ||
    communicate_actor_reserves < 200 || DiploScratchFA78 >= 4) {
    goto communicate_large_offer_rejected; // native 0x5551F7
}
if (communicate_actor_reserves < communicate_wrapped_mul(communicate_peer_reserves, 2) &&
    (has_treaty(faction_id, peer_id, 0x00000001) == 0 ||
     static_cast<int>(PlayersData[faction_id].ranking) <= static_cast<int>(PlayersData[peer_id].ranking))) {
    goto communicate_large_offer_rejected; // native 0x5551F7
}
if ((DiploScratchFA60 != 0 && communicate_stack_20 == 0) ||
    communicate_stack_14 != 0) {
    goto communicate_large_offer_rejected; // native 0x5551F7
}
if (communicate_stack_20 == 0 && communicate_stack_38 != 0 &&
    communicate_stack_48 == 0 &&
    has_treaty(faction_id, peer_id, 0x00000001) == 0) {
    goto communicate_large_offer_rejected; // native 0x5551F7
}
if (has_treaty(faction_id, peer_id, 0x00000010) != 0 ||
    has_treaty(faction_id, peer_id, 0x00010003) == 0 ||
    has_treaty(peer_id, faction_id, 0x00000020) != 0 ||
    PlayersData[faction_id].loan_balance[peer_id] != 0 ||
    PlayersData[peer_id].loan_balance[faction_id] != 0) {
    goto communicate_large_offer_rejected; // native 0x5551F7
}
if (communicate_peer_reserves <
        communicate_wrapped_mul(communicate_wrapped_add(DiploScratchF808, 1), 100)) {
    goto communicate_large_offer_rejected; // native 0x5551F7
}
if (DiploScratchF7E8 > 0 &&
    communicate_peer_reserves >=
        communicate_wrapped_mul(PlayersData[DiploScratchF7E8].energy_reserves, 3) / 4) {
    goto communicate_large_offer_rejected; // native 0x5551F7
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_large_offer_rejected; // native 0x5551F7
}
goto communicate_large_offer_payment_calculation; // native 0x554C2C
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554C2C..0x00554D26.
// Member names below are from the current native PlayerData COFF layout:
// +0x2FC unk_13, +0x300 maint_cost_total, +0x304 tech_commerce_bonus, and
// +0x124 integrity_blemishes. Their economic meaning is not inferred here.
// Original 554C2C..554E6C. Reused EBP-1C, EBP-14, EBP+C and EBP-2C
// remain the same C++ locals used by their later native consumers.
communicate_large_offer_payment_calculation:
communicate_stack_1c = PlayersData[faction_id].energy_reserves / 2;
communicate_large_offer_capacity_limit = communicate_wrapped_mul(
    communicate_wrapped_sub(PlayersData[peer_id].unk_13,
              static_cast<int>(PlayersData[peer_id].maint_cost_total)), 100);
if (communicate_stack_1c >= communicate_large_offer_capacity_limit)
    communicate_stack_1c = communicate_large_offer_capacity_limit;
if (communicate_stack_1c < 50) goto communicate_large_offer_rejected;
communicate_large_offer_target = communicate_wrapped_mul(communicate_wrapped_add(
    PlayersData[peer_id].integrity_blemishes,
    communicate_wrapped_add(communicate_wrapped_mul(communicate_wrapped_sub(PlayersData[faction_id].tech_commerce_bonus,
                               PlayersData[peer_id].tech_commerce_bonus), 2), 10)), 5);
if (has_treaty(faction_id, peer_id, 1) != 0)
    communicate_large_offer_target = communicate_wrapped_sub(communicate_large_offer_target, 10);
if (communicate_large_offer_target < 10) communicate_large_offer_target = 10;
communicate_large_offer_limit = communicate_wrapped_mul(
    communicate_wrapped_sub(10, PlayersData[peer_id].integrity_blemishes), 10);
if (communicate_large_offer_limit < 20) communicate_large_offer_limit = 20;
communicate_stack_14 = communicate_wrapped_add(communicate_large_offer_target, 100);
communicate_large_offer_scaled = communicate_wrapped_mul(communicate_stack_14,
                                           communicate_stack_1c) / 100;
if (communicate_large_offer_scaled < communicate_large_offer_limit)
    communicate_large_offer_limit = communicate_large_offer_scaled;
// The divisor, not the live EBX payment limit, is clamped. Preserve this
// distinction even on overflow/negative economic inputs.
communicate_stack_c = communicate_wrapped_add(communicate_large_offer_scaled,
    communicate_wrapped_sub(communicate_large_offer_limit, 1)) /
    (communicate_large_offer_limit < 1 ? 1 : communicate_large_offer_limit);
communicate_stack_2c = communicate_wrapped_mul(communicate_large_offer_limit, communicate_stack_c);
parse_says(0, get_title(faction_id), -1, -1);
communicate_large_offer_text:
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_title(peer_id), -1, -1);
parse_says(3, get_name(peer_id), -1, -1);
parse_num(0, communicate_stack_1c);
parse_num(1, communicate_stack_c);
parse_num(2, communicate_large_offer_limit);
clear();
says("ASKFORLOAN");
say_num(communicate_stack_c == 1 ? 1 : 2);
communicate_large_offer_popup_result = X_pops(
    StringTemp, 0x100000, FactionDiplomacyPortraits[peer_id], pop_wait);
if (communicate_large_offer_popup_result == 0)
    goto communicate_large_offer_zero_popup_return;
if (communicate_large_offer_popup_result == 2) {
    communicate_stack_1c /= 2;
    communicate_large_offer_scaled = communicate_wrapped_mul(communicate_stack_14,
                                               communicate_stack_1c) / 100;
    if (communicate_large_offer_scaled < communicate_large_offer_limit)
        communicate_large_offer_limit = communicate_large_offer_scaled;
    communicate_stack_c = communicate_wrapped_add(communicate_large_offer_scaled,
        communicate_wrapped_sub(communicate_large_offer_limit, 1)) /
        (communicate_large_offer_limit < 1 ? 1 : communicate_large_offer_limit);
    communicate_stack_2c = communicate_wrapped_mul(communicate_large_offer_limit, communicate_stack_c);
}
// Every nonzero popup result accepts: only result two halves the capacity.
net_treaty_on(peer_id, faction_id, 0x05000000, 0);
net_energy(faction_id, communicate_wrapped_sub(0, communicate_stack_1c),
           peer_id, communicate_stack_1c, 1);
net_loan(peer_id, faction_id, communicate_stack_2c, communicate_stack_c);
cause_friction(peer_id, faction_id,
               communicate_wrapped_sub(0, energy_value(communicate_stack_1c)));
communicate_stack_20 = 1;
goto communicate_large_offer_rejected;

// INCOMPLETE PRIVATE BODY SECTION: native 0x00554E6C..0x00554EDB.
// This preserves the unusual native argument positions at each net_tech
// call; no direction is normalized based on the resulting helper's name.
communicate_later_action_dispatch: // native 0x00554E6C
communicate_large_offer_declined:
net_treaty_on(faction_id, peer_id, 0x05000000, 0);
if (communicate_relation_auxiliary_result <= 0) {
    goto communicate_large_offer_zero_limit; // native 0x005549EA
}
communicate_later_function_return: // native 0x00554E74
net_tech(peer_id, DiploScratchFAA8, faction_id, 1);
if (communicate_stack_c >= 15) {
    net_tech(peer_id, DiploScratchFA28, faction_id, 1);
}
if (communicate_stack_c >= 14) {
    net_tech(peer_id, DiploScratchFA1C, faction_id, 1);
    communicate_stack_c = 6;
    goto communicate_relation_action_result_5550b0; // native 0x005550B0
}
if (communicate_stack_c == 12) {
    communicate_stack_c = 6;
    goto communicate_relation_action_result_5550b0; // native 0x005550B0
}
goto communicate_relation_later_response_gates; // native 0x00554EDB
// INCOMPLETE PRIVATE BODY SECTION: native 0x00554EDB..0x005550B0.
// Repeats the DEMANDTECH popup after the native help callback while its
// result is greater than one; result zero/one falls through to action choice.
communicate_relation_later_response_gates:
if ((DiploScratchFA50 == 0 && DiploScratchFAA0 == 0) ||
    DiploScratchFA60 == 0 || DiploScratchFA18 < 0) {
    goto communicate_relation_action_result_5550b0; // native 0x005550B0
}
communicate_demand_tech_popup:
clear();
say_tech(DiploScratchFAA8, 1);
parse_says(0, StringTemp, -1, -1);
parse_says(1, get_title(faction_id), -1, -1);
parse_says(2, get_name(faction_id), -1, -1);
parse_says(3, get_title(peer_id), -1, -1);
parse_says(4, get_name(peer_id), -1, -1);
communicate_stack_c = 6;
clear();
says("DEMANDTECH");
say_num(6);
communicate_demand_tech_popup_result = X_pops(
    StringTemp, 0x00100000, FactionDiplomacyPortraits[peer_id], pop_wait);
communicate_relation_auxiliary_result = communicate_demand_tech_popup_result;
if (communicate_demand_tech_popup_result > 1) {
    help_topic(14, DiploScratchFA18);
    goto communicate_demand_tech_popup;
}
goto communicate_relation_action_result_5550b0; // native 0x005550B0
// INCOMPLETE PRIVATE BODY SECTION: native 0x005550B0..0x005551F7.
// The native popup's EBX result is carried separately from the dialogue
// count. The two paths below preserve the exact treaty/friction ordering and
// the writes to the existing actor-row diplo_unk1[peer] member.
communicate_relation_action_result_5550b0:
if (communicate_relation_auxiliary_result == 0) {
    net_treaty_off(faction_id, peer_id, 0x01000000, 0);
    communicate_stack_38 = 1;
    goto communicate_later_event_loop; // native 0x00554A42
}
DiploScratchFAA8 = DiploScratchFA18;
DiploScratchFA18 = -1;
if (communicate_stack_c == 6) {
    net_tech(peer_id, DiploScratchFAA8, faction_id, 1);
    DiploScratchFAA8 = -1;
}
net_treaty_on(faction_id, peer_id, 0x05000000, 0);
if (has_treaty(faction_id, peer_id, 0x00000020) == 0) {
    DiploScratchFA70 = 0;
}
communicate_stack_20 = 1;
if (communicate_relation_auxiliary_result == 1) {
    if (communicate_stack_c >= 9 &&
        communicate_stack_c < 12) {
        const int diplomacy_tech_value =
            tech_val(DiploScratchFAA8, peer_id, 1);
        const int reduced_value = diplomacy_tech_value / 2;
        cause_friction(peer_id, faction_id,
                       communicate_wrapped_sub(0, reduced_value));
        if (PlayersData[faction_id].diplo_unk1[peer_id] == 0 ||
            PlayersData[faction_id].diplo_unk1[peer_id] <
                PlayersData[faction_id].integrity_blemishes) {
            ++PlayersData[faction_id].diplo_unk1[peer_id];
        }
    }
    if (communicate_stack_c >= 12) {
        if (PlayersData[faction_id].diplo_unk1[peer_id] != 0) {
            --PlayersData[faction_id].diplo_unk1[peer_id];
        }
        goto communicate_later_event_loop; // native 0x00554A42
    }
}
goto communicate_later_event_loop; // native 0x00554A42
communicate_large_offer_zero_popup_return:
if (has_treaty(faction_id, peer_id, 0x00000001) != 0) {
    cause_friction(peer_id, faction_id, 1);
}
goto communicate_after_large_offer_result; // native 0x005551F7
communicate_large_offer_zero_response:
net_treaty_off(faction_id, peer_id, 0x01000000, 0);
communicate_stack_38 = 1;
goto communicate_later_event_loop; // native 0x00554A42
// INCOMPLETE PRIVATE BODY SECTION: native 0x005551F7..0x005552FE.
// Completes the mode/stack/agreement gates, tests both configured social
// ideology slots against the real pending SocialCategory owner, and prepares
// the existing diplo_unk1/diplo_wrongs response mode.
communicate_large_offer_rejected: // native 0x005551F7
communicate_after_large_offer_result:
if (mode != 0 || communicate_stack_20 != 0) {
    goto communicate_late_contact_mode; // native 0x005552A5
}
if (DiploScratchF7FC == 0 && DiploScratchFA98 != 0 &&
    has_treaty(peer_id, faction_id, 0x00000001) == 0 &&
    has_agenda(peer_id, faction_id, 0x00000400) == 0) {
    goto communicate_late_contact_mode; // native 0x005552A5
}
if (has_agenda(peer_id, faction_id, 0x00000001) == 0 ||
    has_agenda(faction_id, peer_id, 0x00002000) != 0) {
    goto communicate_late_contact_mode; // native 0x005552A5
}
communicate_stack_c = 0; // original 55526B
if (Players[peer_id].soc_ideology_category >= 0 &&
    Players[peer_id].soc_ideology_model != 0 &&
    communicate_pending_social_model(PlayersData[faction_id].soc_category_pending,
                         Players[peer_id].soc_ideology_category) != 0 &&
    communicate_pending_social_model(PlayersData[faction_id].soc_category_pending,
                         Players[peer_id].soc_ideology_category) !=
        Players[peer_id].soc_ideology_model) {
    goto communicate_social_break_pact;
}
communicate_late_contact_mode:
communicate_stack_c = 0;
if (PlayersData[faction_id].diplo_unk1[peer_id] != 0 &&
    PlayersData[faction_id].diplo_unk1[peer_id] >
        PlayersData[faction_id].diplo_wrongs[peer_id] &&
    has_treaty(peer_id, faction_id, 0x00000050) == 0 &&
    has_agenda(peer_id, faction_id, 0x00002008) == 0 &&
    has_treaty(peer_id, faction_id, 0x00040020) == 0) {
    communicate_stack_c = 1;
}
goto communicate_late_contact_pact_gates; // native 0x005552FE
// INCOMPLETE PRIVATE BODY SECTION: native 0x005552FE..0x005553DE.
// Preserves the ordered directed pact/agenda tests and signed rank/commerce
// comparison. The destinations at 0x55555F/62/64A/64D/65D are translated in
// later action sections; they are kept distinct rather than collapsed.
communicate_late_contact_pact_gates:
if (has_treaty(peer_id, faction_id, 0x00000001) == 0) {
    goto communicate_late_contact_without_pact; // native 0x00555646
}
if (communicate_stack_20 != 0) {
    goto communicate_late_contact_stack_done; // native 0x0055565D
}
if (mode != 0 ||
    has_agenda(faction_id, peer_id, 0x00002000) != 0) {
    goto communicate_late_contact_mode_done; // native 0x0055564A
}
if (DiploScratchFA60 != 0) {
    goto communicate_late_contact_priority; // native 0x0055555F
}
if (!(static_cast<int>(PlayersData[faction_id].ranking) >= 6 &&
      static_cast<int>(PlayersData[peer_id].ranking) >= 6)) {
    const int commerce_difference = communicate_wrapped_sub(
        PlayersData[peer_id].tech_commerce_bonus,
        PlayersData[faction_id].tech_commerce_bonus);
    const int commerce_threshold =
        communicate_wrapped_add(communicate_wrapped_mul(commerce_difference, 2), 10);
    if (DiploFriction < commerce_threshold) {
        goto communicate_late_contact_rank_route; // native 0x005553B4
    }
    if (static_cast<int>(PlayersData[peer_id].ranking) >= 5 ||
        (GameDataStateStorage.fields_.game_rules_ & 0x00001000u) == 0 || GameDataStateStorage.fields_.turn_current_num_ < 200) {
        goto communicate_late_contact_rank_fallback; // native 0x00555562
    }
    goto communicate_late_contact_rank_route; // native 0x005553B4
}
goto communicate_late_contact_rank_fallback; // native 0x00555562
communicate_late_contact_rank_route: // native 0x005553B4
if (DiploScratchFA64 == 0 || DiploScratchFA90 == 0 ||
    (GameDataStateStorage.fields_.game_rules_ & 0x00001000u) != 0) {
    goto communicate_late_contact_rank_route_alt; // native 0x0055564D
}
goto communicate_late_contact_rank_fallback; // native 0x00555562
// INCOMPLETE PRIVATE BODY SECTION: native 0x005553DE..0x0055555F.
// The peer's matching social-model dialogue uses the selected primary or
// anti-ideology pair, then ends/establishes treaties in the original order.
communicate_social_break_pact:
(void)wants_to_speak(faction_id, peer_id);
if (DiplomacyEndState == 0) {
    diplomacy_caption(faction_id, peer_id);
}
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_pact_hood(faction_id, peer_id), -1, -1);
parse_says(3, get_noun(peer_id), -1, -1);
clear();
says("SOCIAL");
communicate_social_category = communicate_stack_c == 0
    ? Players[peer_id].soc_ideology_category
    : Players[peer_id].soc_anti_ideology_category;
communicate_social_model = communicate_stack_c == 0
    ? Players[peer_id].soc_ideology_model
    : Players[peer_id].soc_anti_ideology_model;
say_num(communicate_social_category);
says("CAT");
say_num(communicate_pending_social_model(
    PlayersData[faction_id].soc_category_pending,
    communicate_social_category));
says(has_treaty(faction_id, peer_id, 0x00000001) != 0 ? "PACT" : "WAR");
say_num(communicate_social_model);
X_pops(StringTemp, 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
if (has_treaty(faction_id, peer_id, 0x00000001) != 0) {
    (void)diplo_lock(0);
    net_pact_ends(peer_id, faction_id, 1);
    diplo_unlock();
    PlayersData[faction_id].diplo_spoke[peer_id] = GameDataStateStorage.fields_.turn_current_num_;
    goto communicate_epilogue; // native 0x005587C8
}
net_set_treaty(peer_id, faction_id, 0x00000040, 1, 1);
net_treaty_on(peer_id, faction_id, 0x00000010, 1);
PlayersData[faction_id].diplo_spoke[peer_id] = GameDataStateStorage.fields_.turn_current_num_;
goto communicate_epilogue; // native 0x005587C8
// INCOMPLETE PRIVATE BODY SECTION: native 0x0055555F..0x00555707.
// The dec/increment paths use the existing directed actor-row counters. The
// remaining gates preserve peer/actor treaty orientation and the two actual
// Player.rule_flags reads before transferring to the later popup block.
// Original 0x55555F..0x555707. EBP+0xC is the live zero/one response flag.
communicate_late_contact_priority:
communicate_late_contact_rank_fallback:
if (communicate_stack_c != 0) {
    DiploScratchFA60 = 0;
    PlayersData[faction_id].diplo_unk1[peer_id] = communicate_wrapped_sub(
        PlayersData[faction_id].diplo_unk1[peer_id], 1);
    goto communicate_late_contact_rank_route_alt;
}
wants_to_speak(faction_id, peer_id);
if (DiplomacyEndState == 0) diplomacy_caption(faction_id, peer_id);
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_pacts(faction_id, peer_id), -1, -1);
parse_says(3, get_noun_phrase(faction_id, 8), -1, -1);
X_pops("BREAKPACT", 0x100000, FactionDiplomacyPortraits[peer_id], pop_wait);
diplo_lock(0);
net_pact_ends(peer_id, faction_id, 1);
diplo_unlock();
PlayersData[faction_id].diplo_spoke[peer_id] = GameDataStateStorage.fields_.turn_current_num_;
goto communicate_final_cleanup;

communicate_late_contact_without_pact: // original 0x555646
if (communicate_stack_20 != 0) goto communicate_late_contact_stack_done;
communicate_late_contact_mode_done:
communicate_late_contact_rank_route_alt:
if (has_treaty(faction_id, peer_id, 1) != 0) {
    goto communicate_late_contact_stack_done;
}
if (DiploScratchFA60 == 0) goto communicate_late_contact_after_war_checks;
if (communicate_stack_c != 0) {
    DiploScratchFA60 = 0;
    PlayersData[faction_id].diplo_unk1[peer_id] = communicate_wrapped_sub(
        PlayersData[faction_id].diplo_unk1[peer_id], 1);
    goto communicate_late_contact_after_war_checks;
}
if (static_cast<int>(PlayersData[faction_id].ranking) <=
        static_cast<int>(PlayersData[peer_id].ranking) ||
    DiploScratchFA2C <= communicate_wrapped_add(PlayersData[faction_id].integrity_blemishes,
                                DiploScratchF7FC) ||
    DiploScratchFAA0 != 0 || DiploScratchFA50 != 0 ||
    has_treaty(peer_id, faction_id, 0x10) == 0 ||
    has_agenda(peer_id, faction_id, 8) != 0) {
    goto communicate_late_contact_after_war_checks;
}
if (has_treaty(peer_id, faction_id, 0x2000) != 0) {
    goto communicate_late_contact_special_war;
}
goto communicate_late_contact_after_war_checks;

communicate_late_contact_stack_done: // original 0x55565D
DiploScratchFA60 = 0;
communicate_late_contact_after_war_checks: // original 0x555707
if (DiploScratchFA4C == 0 || has_treaty(faction_id, peer_id, 0x10) != 0) {
    goto communicate_late_contact_hate_popup_exit;
}
if (DiploScratchFA60 == 0 || DiploScratchF7E8 == 0) {
    if ((Players[peer_id].rule_flags & 0x80) == 0 ||
        (Players[DiploScratchF7B8].rule_flags & 0x80) == 0) {
        goto communicate_late_contact_hate_popup_exit;
    }
}
goto communicate_late_contact_hate_popup;

// INCOMPLETE PRIVATE BODY SECTION: native 0x00555768..0x0055586D.
// Records the treaty/pact state and last-spoke turn before building the
// selected hatred popup. Its native shared X_pops return continuation remains
// in the later tail section.
communicate_late_contact_hate_popup:
clear();
(void)wants_to_speak(faction_id, peer_id);
net_set_treaty(peer_id, faction_id, 0x00000040, 1, 1);
net_treaty_on(peer_id, faction_id, 0x00000010, 1);
PlayersData[faction_id].diplo_spoke[peer_id] = GameDataStateStorage.fields_.turn_current_num_;
diplomacy_caption(faction_id, peer_id);
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_title(DiploScratchF7B8), -1, -1);
parse_says(3, get_name(DiploScratchF7B8), -1, -1);

communicate_late_contact_hate_caption =
    ((Players[peer_id].rule_flags & 0x80) != 0 &&
     (Players[DiploScratchF7B8].rule_flags & 0x80) != 0)
        ? "ALIENHATRED" : "COLLABORATOR";
X_pops(communicate_late_contact_hate_caption, 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
goto communicate_final_cleanup; // native 0x55584C/0x555868 -> 0x5587C0/0x5587C8
// INCOMPLETE PRIVATE BODY SECTION: starts at native 0x0055586D and currently
// continues through 0x00556723. The filename is historical; this draft owns
// substantial later labels and must be reconciled with the corrected
// score-report sections before whole-body composition.
// Native 0x55586D's failed predicates rejoin the entry cleanup at 0x550522
// or 0x550529; the accepted path records peer->actor vendetta state, renders
// BULLY, and jumps to the common epilogue. 0x5559CB begins a distinct action
// response mode and is retained as its own label below.
// Requires function-scope locals for native [EBP-0x38], the overwritten
// [EBP+0x0C] mode slot, BULLY's random value and VENDETTA's text selector.
communicate_late_contact_hate_popup_exit: // native 0x0055586D
if (DiploScratchFA60 == 0 || communicate_stack_38 == 0 ||
    has_treaty(faction_id, peer_id, 0x00000006) == 0) {
    goto communicate_contact_entry_exit; // native 0x00550522
}
if (has_treaty(peer_id, faction_id, 0x00000010) != 0) {
    goto communicate_contact_entry_exit; // native 0x00550522
}
if (DiploScratchF7E4 <= 0) {
    goto communicate_contact_entry_exit_alt; // native 0x00550529
}
if (has_treaty(peer_id, faction_id, 0x00002000) != 0) {
    goto communicate_contact_entry_exit; // native 0x00550522
}
communicate_late_contact_bully: // native 0x005558CD
(void)wants_to_speak(faction_id, peer_id);
net_treaty_on(peer_id, faction_id, 0x00000010, 1);
diplomacy_caption(faction_id, peer_id);
PlayersData[faction_id].diplo_spoke[peer_id] = GameDataStateStorage.fields_.turn_current_num_;
clear();
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_adjective_leader(faction_id), -1, -1);
parse_says(3, get_adjective_insult_faction(faction_id), -1, -1);
parse_says(4, get_phrase(peer_id, 13), -1, -1);
parse_says(5, get_noun_phrase(faction_id, 10), -1, -1);
says("BULLY");
communicate_late_contact_bully_random = random(0, 7);
say_num(communicate_late_contact_bully_random);
X_pops(StringTemp, 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
goto communicate_final_cleanup; // native 0x005587C8
communicate_late_contact_vendetta: // native 0x005559CB
communicate_local_menu_mode = 1;
communicate_late_contact_vendetta_after_set_mode: // native 0x005559D0
if (DiploScratchFA60 == 0) {
    goto communicate_special_war_response; // native 0x00555C42
}
if (communicate_stack_38 == 0 && communicate_local_menu_mode == 0) {
    goto communicate_final_cleanup; // native 0x005587C8
}
if (DiploScratchFA88 >= 0) {
    (void)wants_to_speak(faction_id, peer_id);
}
// The native query result is overwritten by the unconditional network update;
// preserve both calls and the latter's return value for the later selector.
communicate_late_contact_vendetta_response =
    has_treaty(peer_id, faction_id, 0x00000010);
net_treaty_on(peer_id, faction_id, 0x00000010, 1);
PlayersData[faction_id].diplo_spoke[peer_id] = GameDataStateStorage.fields_.turn_current_num_;
diplomacy_caption(faction_id, peer_id);
clear();
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_adjective_insult_leader(faction_id), -1, -1);
parse_says(3, get_adjective_leader(faction_id), -1, -1);
parse_says(4, get_noun_phrase(faction_id, 8), -1, -1);
parse_says(5, get_phrase(faction_id, 3), -1, -1);
parse_says(6, get_phrase(peer_id, 13), -1, -1);
parse_says(8, get_phrase(faction_id, 4), -1, -1);
parse_says(9, get_insult_leader(faction_id), -1, -1);
says("VENDETTA");
communicate_vendetta_text_variant =
    (communicate_stack_38 != 0 &&
     communicate_late_contact_vendetta_response == 0)
        ? random(0, 3) : random(0, 6) + 3;
if (great_satan(faction_id, 0) != 0 && random(0, 3) != 0) {
    communicate_vendetta_text_variant = random(0, 4) + 15;
}
if (communicate_local_menu_mode != 0 && DiploScratchFA88 >= 0) {
    communicate_vendetta_text_variant = 14;
}
if (has_treaty(peer_id, faction_id, 0x00040000) != 0 &&
    has_treaty(peer_id, faction_id, 0x00000020) != 0) {
    communicate_vendetta_text_variant = random(0, 5) + 9;
}
say_num(communicate_vendetta_text_variant);
if (communicate_vendetta_text_variant == 5 &&
    strcmp(Players[faction_id].filename, "CYBORG") == 0) {
    says("X");
}
if (has_agenda(peer_id, faction_id, 4) != 0) {
    clear();
    says("ATROCIOUS");
}
X_pops(StringTemp, 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
goto communicate_final_cleanup; // native 0x005587C8
communicate_late_contact_special_war: // native 0x00555C42
communicate_special_war_response:
if (communicate_stack_50 != 0 && communicate_stack_20 != 0) {
    (void)net_treaty_on(peer_id, faction_id, 0x00000004, 1);
}
if (has_treaty(peer_id, faction_id, 0x00000002) == 0) {
    if (DiploScratchFA8C == 0 ||
        has_treaty(peer_id, faction_id, 0x00000010) != 0) {
        goto communicate_special_war_rival_gates; // native 0x00556113
    }
}
if (has_treaty(peer_id, faction_id, 0x00000021) != 0 ||
    has_agenda(peer_id, faction_id, 0x00002000) != 0 ||
    DiploScratchFA3C != 0) {
    goto communicate_special_war_rival_gates; // native 0x00556113
}
communicate_special_war_rival_scan: // native 0x00555CC7
communicate_stack_1c = 0;
communicate_stack_14 = 0;
communicate_rival_faction = 1;
do {
    if (is_alive(communicate_rival_faction) &&
        has_treaty(peer_id, communicate_rival_faction,
                   0x00000010) != 0) {
        if (has_treaty(faction_id, communicate_rival_faction,
                       0x00000014) != 0) {
            communicate_stack_14 = 1;
        }
        if (has_treaty(faction_id, communicate_rival_faction,
                       0x00000001) != 0) {
            goto communicate_special_war_rival_gates; // native 0x00555D29
        }
        for (communicate_special_war_region = 1;
             communicate_special_war_region < 0x80;
             ++communicate_special_war_region) {
            if (bad_reg(communicate_special_war_region) == 0 &&
                PlayersData[peer_id].region_total_bases[
                    communicate_special_war_region] != 0 &&
                PlayersData[communicate_rival_faction].region_total_bases[
                    communicate_special_war_region] != 0) {
                ++communicate_stack_1c;
                break;
            }
        }
    }
    ++communicate_rival_faction;
} while (communicate_rival_faction < 8);
if (communicate_stack_14 < 0 ||
    great_satan(faction_id, 0) != 0 ||
    has_agenda(faction_id, peer_id, 0x00000400) != 0 ||
    communicate_stack_3c != 0) {
    goto communicate_special_war_rival_gates; // native 0x00555D95
}
// Native 0x555DD0..0x556113: the original pact offer fallthrough.
// The rival scan retains one hit per faction and the original zero/one flags.
communicate_pact_offer_fallthrough:
{
    int pact_threshold = range(communicate_wrapped_sub(
        PlayersData[peer_id].tech_commerce_bonus,
        PlayersData[faction_id].tech_commerce_bonus), 0, 99);
    pact_threshold = communicate_wrapped_add(pact_threshold,
        strcmp(Players[peer_id].filename, "HIVE") == 0);
    pact_threshold = communicate_wrapped_add(pact_threshold, (GameDataStateStorage.fields_.game_rules_ >> 12) & 1);
    pact_threshold = communicate_wrapped_add(pact_threshold, DiploScratchFA8C != 0);
    pact_threshold = communicate_wrapped_add(pact_threshold, 3);
    if (DiploScratchFA78 >= pact_threshold) {
        goto communicate_special_war_rival_gates;
    }

    communicate_stack_3c = 3;
    if (PlayersData[faction_id].global_reputation <= 3) {
        communicate_stack_3c = PlayersData[faction_id].global_reputation;
    }
    const int shared_victory =
        (GameDataStateStorage.fields_.game_rules_ & 0x1000) != 0 && (GameDataStateStorage.fields_.game_rules_ & 0x40) == 0;
    int commerce = range(communicate_wrapped_add(communicate_wrapped_sub(
        PlayersData[peer_id].tech_commerce_bonus,
        PlayersData[faction_id].tech_commerce_bonus), 1), 0, 99);
    commerce = communicate_wrapped_add(commerce,
        strcmp(Players[peer_id].filename, "HIVE") == 0);
    int threshold = communicate_wrapped_add(communicate_stack_14 != 0 ? 11 : 6,
                            shared_victory);
    threshold = communicate_wrapped_add(threshold, commerce);
    threshold = communicate_wrapped_add(threshold, DiploScratchFA8C != 0);
    if (communicate_wrapped_add(communicate_wrapped_sub(DiploScratchFA78, communicate_stack_3c),
                  DiploFriction) >= threshold) {
        goto communicate_special_war_rival_gates;
    }
    if (communicate_stack_1c != 0 && DiploScratchFA54 == 0 &&
        DiploScratchF7FC == 0) {
        if (DiploScratchFA98 == 0 && communicate_stack_14 == 0 &&
            static_cast<int>(PlayersData[faction_id].ranking) > 5 &&
            static_cast<int>(PlayersData[peer_id].ranking) > 5) {
            goto communicate_special_war_rival_gates;
        }
    } else if (static_cast<int>(PlayersData[faction_id].ranking) > 5 ||
               static_cast<int>(PlayersData[peer_id].ranking) > 5) {
        goto communicate_special_war_rival_gates;
    }
    if (wants_to_speak(faction_id, peer_id) == 0) {
        goto communicate_special_war_rival_gates;
    }
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_pact_hood(faction_id, peer_id), -1, -1);
    if (X_pops("SWEARAPACT", 0x100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait) != 0) {
        goto communicate_special_war_rival_gates;
    }
    if (DiploScratchF7E8 != 0) {
        parse_says(5, get_title(faction_id), -1, -1);
        parse_says(6, get_noun(DiploScratchF7E8), -1, -1);
        if (X_pops("GIMMEHELP", 0x100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait) != 0) {
            diplo_lock(0);
            net_double_cross(faction_id, DiploScratchF7E8, peer_id, 0);
            net_treaty_off(faction_id, peer_id, 0x800000, 0);
            net_set_treaty(faction_id, DiploScratchF7E8, 0x40, 1, 1);
            diplo_unlock();
            DiploScratchF7E8 = 0;
            DiploScratchFA4C = 0;
        } else {
            if (DiploScratchFA98 == 0) {
                parse_says(2, get_pacts(faction_id, peer_id), -1, -1);
                X_pops("REJPACTNOHELP", 0x100000,
                             FactionDiplomacyPortraits[peer_id], pop_wait);
                communicate_stack_34 = 1;
                goto communicate_special_war_rival_gates;
            }
            if (communicate_stack_34 == 0) {
                communicate_stack_34 = 1;
                goto communicate_special_war_rival_gates;
            }
        }
    }
    pact_of_brotherhood(faction_id, peer_id);
    diplomacy_caption(faction_id, peer_id);
    communicate_stack_34 = 1;
}

communicate_special_war_rival_gates: // native 0x00556113
if (has_treaty(peer_id, faction_id, 0x00000010) == 0 ||
    DiploScratchFA98 == 0 || DiploScratchFA78 > 2 ||
    has_treaty(peer_id, faction_id, 0x00040000) != 0 ||
    has_treaty(peer_id, faction_id, 0x02000000) != 0 ||
    has_agenda(peer_id, faction_id, 0x00002000) != 0 ||
    DiploScratchFA98 <= communicate_wrapped_add(DiplomacyProposalGateCount, 2) || IsMultiplayerNet != 0 ||
    wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_special_war_ransom_finish; // native 0x005562D9
}
communicate_special_war_selected_base_id = -1;
communicate_special_war_smallest_population = 999;
{
for (int communicate_base_scan_id = 0;
     communicate_base_scan_id < GameDataStateStorage.fields_.base_current_count_;
     ++communicate_base_scan_id) {
    const Base &communicate_candidate_base = Bases[communicate_base_scan_id];
    if (communicate_candidate_base.faction_id_current != peer_id ||
        communicate_candidate_base.faction_id_former != faction_id) {
        continue;
    }
    if (communicate_candidate_base.population_size <
        communicate_special_war_smallest_population) {
        communicate_special_war_smallest_population =
            communicate_candidate_base.population_size;
        communicate_special_war_selected_base_id = communicate_base_scan_id;
    }
}
}
if (communicate_special_war_selected_base_id < 0) {
    goto communicate_special_war_ransom_finish; // native 0x005562D9
}
if (wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_special_war_ransom_finish; // native 0x005562D9
}
clear();
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, Bases[communicate_special_war_selected_base_id]
                         .name_string, -1, -1);
if (X_pops("RANSOMTREATY", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) != 0) {
    goto communicate_special_war_ransom_finish; // native 0x005562D9
}
communicate_stack_34 = 1; // original 5562A3
net_cede_base(communicate_special_war_selected_base_id, faction_id, 1);
net_set_treaty(peer_id, faction_id, 0x02000000, 1, 0);
(void)pledge_truce(faction_id, peer_id, 0);
(void)make_treaty(faction_id, peer_id);
(void)diplomacy_caption(faction_id, peer_id);
communicate_special_war_ransom_finish: // native 0x005562D9
if (has_treaty(faction_id, peer_id, 0x00000010) == 0 ||
    wants_to_speak(faction_id, peer_id) == 0) {
    goto communicate_special_war_late_exit; // native 0x00556CDE
}
if (DiploScratchFA24 == 0) {
    goto communicate_offer_truce_prompt; // native 0x0055635D
}
if (has_agenda(faction_id, peer_id, 0x00000800) != 0 ||
    has_treaty(faction_id, peer_id, 0x00000020) == 0) {
    goto communicate_offer_truce_prompt; // native 0x0055635D
}
communicate_offer_truce_variant:
communicate_truce_random_limit =
    has_treaty(faction_id, peer_id, 0x00000400) != 0 ? 2 : 3;
if (random(0, communicate_truce_random_limit) == 0) {
    communicate_truce_response_result = 1;
    goto communicate_after_truce_prompt; // native 0x005564D7
}
communicate_offer_truce_prompt: // native 0x0055635D
if (communicate_stack_20 != 0) {
    communicate_truce_response_result =
        X_pops("OFFERTRUCE", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    goto communicate_after_truce_prompt; // native 0x005564D5
}
if (DiploScratchFA60 != 0) {
    communicate_truce_response_result =
        X_pops("MUSTTRUCE", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    goto communicate_after_truce_prompt; // native 0x005564D5
}
communicate_truce_prototype_prompt: // native 0x005563B5
mention_prototypes(faction_id, peer_id);
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_title(peer_id), -1, -1);
parse_says(3, get_name(peer_id), -1, -1);
clear();
communicate_truce_variant = random(0, 4);
if (communicate_truce_variant == 2 &&
    has_treaty(faction_id, peer_id, 0x00000020) != 0) {
    goto communicate_truce_alien_clause; // native 0x00556477
}
if (communicate_truce_variant >= 3) {
    goto communicate_truce_alien_clause; // native 0x00556477
}
says("WANTTOTRUCE");
say_num(communicate_truce_variant);
communicate_truce_response_result =
    X_pops(StringTemp, 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
goto communicate_after_truce_prompt; // native 0x005564D2
communicate_truce_alien_clause: // native 0x00556477
if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
    says("ALIEN");
}
says("FACTIONTRUCE");
communicate_truce_response_result =
    x_pops_caption_value(Players[peer_id].filename, StringTemp,
                         0x00100000, FactionDiplomacyPortraits[peer_id],
                         pop_wait);
communicate_after_truce_prompt: // native 0x005564D7
communicate_truce_followup_limit = communicate_wrapped_add(DiploScratchFA78, 2);
if (has_treaty(faction_id, peer_id, 0x00000020) != 0) {
    communicate_truce_followup_limit = communicate_wrapped_add(
        communicate_truce_followup_limit,
        communicate_truce_followup_limit);
}
communicate_truce_followup_limit = range(
    communicate_truce_followup_limit, 0,
    communicate_wrapped_add(PlayersData[peer_id].current_num_bases, 1));
if (communicate_truce_response_result == 0) {
    goto communicate_late_truce_result_zero; // native 0x00556B38
}
communicate_truce_followup_roll =
    random(0, communicate_truce_followup_limit);
if (communicate_truce_followup_roll < DiploScratchFA98) {
    goto communicate_late_truce_followup; // native 0x00556554
}
if (DiploScratchFA24 == 0) {
    goto communicate_late_truce_no_offer; // native 0x00556C2A
}
communicate_late_truce_followup: // native 0x00556554
if (DiploScratchF800 < 0) {
    goto communicate_late_truce_offer_declined; // native 0x0055666C
}
communicate_late_truce_research_value =
    tech_val(DiploScratchF800, peer_id, TRUE);
if (DiploScratchFA98 < communicate_late_truce_research_value / 2 ||
    DiploScratchFA24 != 0 || DiploScratchFA70 != 0 ||
    (GameDataStateStorage.fields_.game_rules_ & 0x02000000) != 0) {
    goto communicate_late_truce_offer_declined; // native 0x0055666C
}
clear();
say_tech(DiploScratchF800, 1);
parse_says(0, StringTemp, -1, -1);
communicate_late_truce_tech_popup_result =
    X_pops("TECHTRUCE", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
while (communicate_late_truce_tech_popup_result > 1) {
    help_topic(0x0E, DiploScratchF800);
    clear();
    say_tech(DiploScratchF800, 1);
    parse_says(6, StringTemp, -1, -1);
    communicate_late_truce_tech_popup_result =
        X_pops("TECHTRUCE", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
}
if (communicate_late_truce_tech_popup_result != 0) {
    goto communicate_late_truce_offer_declined; // native 0x0055666C
}
communicate_stack_1c = communicate_late_truce_tech_popup_result;
net_tech(faction_id, DiploScratchF800, peer_id, 1);
communicate_stack_54 = 1;
goto communicate_late_truce_tech_complete; // native 0x00556723
communicate_late_truce_offer_declined: // native 0x0055666C
if (DiploScratchF800 >= 0 && DiploScratchFA98 >= 4) {
    goto communicate_late_truce_tech_complete; // native 0x00556723
}
if (DiploScratchFA24 != 0 || DiploScratchFA70 != 0) {
    goto communicate_late_truce_tech_complete; // native 0x00556723
}
communicate_late_truce_tech_amount = communicate_wrapped_mul(DiploScratchFA98, 100);
if (communicate_late_truce_tech_amount >
    PlayersData[peer_id].energy_reserves) {
    communicate_late_truce_tech_amount =
        PlayersData[peer_id].energy_reserves;
}
communicate_late_truce_tech_amount =
    communicate_round_down_multiple(communicate_late_truce_tech_amount, 25);
if (communicate_late_truce_tech_amount == 0) {
    goto communicate_late_truce_tech_complete; // native 0x00556723
}
parse_num(0, communicate_late_truce_tech_amount);
if (X_pops("ENERGYTRUCE", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) != 0) {
    goto communicate_late_truce_tech_complete; // native 0x00556723
}
net_energy(peer_id, communicate_wrapped_sub(0, communicate_late_truce_tech_amount),
           faction_id, communicate_late_truce_tech_amount, 1);
communicate_stack_1c = 0;
communicate_stack_54 = 1;
// Native writes zero to ConsoleInfoWin.redraw_phase_ (+0x90, 0x007AD330)
// before updating the console data. The existing typed setter is a direct
// member assignment, matching the instruction order without a new alias.
ConsoleInfoWin.set_redraw_phase(0);
ConsoleGlobal.update_data(0);
// Continuations at 0x556723, 0x556B38, 0x556C2A and 0x556CDE are defined by
// the later score-report sections in the whole-body composition. In
// particular, the zero-result path must enter at B38 without replaying the
// B2C stores, while the no-offer path rejoins at C2A.
// Mutable composition slice: native 0x00556723..0x0055688D only.
// The overlapping old score fragment remains historical; this slice stops
// before its superseded nonzero gate/offer implementation.
communicate_late_truce_tech_complete: // native continuation 0x00556723
communicate_late_score_region = 0;
communicate_late_score = communicate_wrapped_add(
    communicate_wrapped_mul(communicate_wrapped_add(PlayersData[faction_id].mil_strength_2,
                        PlayersData[peer_id].total_mil_veh),
              communicate_wrapped_add(PlayersData[faction_id].diplo_unk3[peer_id],
                        PlayersData[faction_id].diplo_unk4[peer_id])) / 4,
    1);
communicate_stack_34 = communicate_wrapped_add(
    communicate_wrapped_add(PlayersData[peer_id].diplo_unk3[faction_id],
              PlayersData[peer_id].diplo_unk4[faction_id]),
    1);
for (communicate_late_score_region = 0;
     communicate_late_score_region < 128;
     ++communicate_late_score_region) {
    if (PlayersData[peer_id].region_total_bases[
            communicate_late_score_region] == 0) {
        communicate_late_score = communicate_wrapped_add(
            communicate_late_score,
            PlayersData[faction_id].region_force_rating[
                communicate_late_score_region] / 2);
    } else {
        communicate_stack_34 = communicate_wrapped_add(
            communicate_stack_34,
            PlayersData[peer_id].region_total_combat_vehs[
                communicate_late_score_region]);
        communicate_stack_34 = communicate_wrapped_add(
            communicate_stack_34,
            PlayersData[peer_id].region_force_rating[
                communicate_late_score_region]);
        communicate_late_score = communicate_wrapped_add(
            communicate_late_score,
            PlayersData[faction_id].region_force_rating[
                communicate_late_score_region]);
        for (communicate_late_score_other = 1;
             communicate_late_score_other < MaxPlayerNum;
             ++communicate_late_score_other) {
            if (has_treaty(communicate_late_score_other, peer_id, 1) != 0 ||
                has_treaty(communicate_late_score_other, faction_id,
                           0x10) == 0) {
                communicate_stack_34 = communicate_wrapped_add(
                    communicate_stack_34,
                    PlayersData[1].unk_77[communicate_late_score_region]);
            }
        }
    }
}
// Native's zero/nonzero branch starts in the corrected eligibility slice.
// Corrected private candidate for native 0x0055688D..0x00556907.
// Reaching native stack slot [EBP+8] is a PlayerData row-byte offset written
// at 0x556514; [EBP-0x10] is the actor row-byte offset. Thus the native
// +0x105C/+0x1060 reads are the real current_num_bases/mil_strength_1 members,
// not independent address-backed globals. Do not use the superseded alien
// faction check in late-contact-faction-score-556723-55688d.inc here.
communicate_late_score_nonzero_result: // native 0x0055688D
if (communicate_stack_1c == 0) {
    goto communicate_late_score_zero_result; // native 0x00556892
}
if ((Players[peer_id].rule_flags & RFLAG_ALIEN) != 0) {
    goto communicate_late_score_declined; // native 0x005568AB
}
if (DiploScratchFA24 != 0) {
    goto communicate_late_score_offer; // native 0x005568B8
}
communicate_late_score_peer_base_count = static_cast<int>(
    PlayersData[peer_id].current_num_bases);
if (DiploScratchFA98 < communicate_late_score_peer_base_count) {
    goto communicate_late_score_declined; // native 0x005568CA
}
if (communicate_late_score_peer_base_count <= 1) {
    goto communicate_late_score_offer; // native 0x005568D3
}
if (communicate_late_score_peer_base_count >
    communicate_late_score / communicate_stack_34) {
    goto communicate_late_score_declined; // native 0x005568DE
}
if (communicate_late_score_peer_base_count > 4 ||
    PlayersData[peer_id].mil_strength_1 >=
        PlayersData[faction_id].mil_strength_1) {
    goto communicate_late_score_declined; // native 0x005568E7 / 0x00556901
}
goto communicate_late_score_offer; // native 0x00556907
// INCOMPLETE PRIVATE BODY SECTION: pinned native 0x00556907..0x005569A2.
// Native parses the actor's title/name and current energy, counts technologies
// held by the peer but not the actor, then dispatches TRUCEPLEASE. Acceptance
// enters the existing 0x556C0F path; declining falls directly into the
// corrected report-zero path beginning at 0x5569A2.
communicate_late_score_offer: // native 0x00556907
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_num(0, PlayersData[faction_id].energy_reserves);
communicate_late_tech_difference = 0; // native [EBP+0x0C]
for (communicate_late_tech_id = 0;
     communicate_late_tech_id < 0x59;
     ++communicate_late_tech_id) {
    if (has_tech(communicate_late_tech_id, peer_id) != 0 &&
        has_tech(communicate_late_tech_id, faction_id) == 0) {
        ++communicate_late_tech_difference;
    }
}
parse_num(1, communicate_late_tech_difference);
if (X_pops("TRUCEPLEASE", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait) != 0) {
    goto communicate_late_score_accepted; // native 0x0055699C -> 0x00556C0F
}
goto communicate_late_score_report_zero; // native 0x005569A2
// Corrected private replacement for native 0x005569A2..0x00556CDE in
// late-contact-faction-score-556723-55688d.inc. Preserve that older file as
// history; this version follows the complete original path and call order.
// Function-scope locals required by the composed body:
//   int communicate_late_score_other, communicate_late_score_received_count;
//   int communicate_late_score_shared_count, communicate_zero_tech_id;
//   int communicate_zero_popup_result, communicate_zero_peer_energy.
communicate_late_score_report_zero: // native 0x005569A2
communicate_stack_54 = 1; // native [EBP-0x54] = 1
if ((GameDataStateStorage.fields_.game_rules_ & 0x02000000) == 0) {
    for (communicate_zero_tech_id = 0;
         communicate_zero_tech_id < 0x59;
         ++communicate_zero_tech_id) {
        if (has_tech(communicate_zero_tech_id, peer_id) != 0 &&
            has_tech(communicate_zero_tech_id, faction_id) == 0) {
            // Native argument order at 0x005569D7 is actor, tech, peer, 0.
            net_tech(faction_id, communicate_zero_tech_id, peer_id, 0);
        }
    }
}
communicate_zero_peer_energy = PlayersData[peer_id].energy_reserves;
net_energy(faction_id, communicate_zero_peer_energy, peer_id,
           communicate_wrapped_sub(0, communicate_zero_peer_energy), 1);
ConsoleInfoWin.set_redraw_phase(0);
ConsoleGlobal.update_data(0);
PlayersData[faction_id].diplo_friction[peer_id] = 0;
net_treaty_off(peer_id, faction_id, 0x20, 1);
net_treaty_on(faction_id, peer_id, 2, 0);
net_set_treaty(peer_id, faction_id, 0x02000000, 1, 0);
pact_of_brotherhood(faction_id, peer_id);
diplomacy_caption(faction_id, peer_id);
if (IsMultiplayerNet == 0 && (GameDataStateStorage.fields_.game_rules_ & 2) != 0) {
    communicate_late_score_received_count = 0;
    communicate_late_score_shared_count = 0;
    for (communicate_late_score_other = 1;
         communicate_late_score_other < 8;
         ++communicate_late_score_other) {
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &
             static_cast<uint8_t>(1u << communicate_late_score_other)) == 0) {
            continue;
        }
        ++communicate_late_score_received_count;
        if (has_treaty(communicate_late_score_other,
                       ConsoleGlobal.faction_id_, 1) != 0) {
            if (has_treaty(communicate_late_score_other,
                           ConsoleGlobal.faction_id_, 0x02000000) != 0) {
                --communicate_late_score_received_count;
                continue;
            }
        }
        if (has_treaty(communicate_late_score_other,
                       ConsoleGlobal.faction_id_, 1) != 0 &&
            (GameDataStateStorage.fields_.game_rules_ & 0x1000) != 0 &&
            communicate_late_score_shared_count < 2) {
            ++communicate_late_score_shared_count;
        }
    }
    if (communicate_late_score_received_count -
            communicate_late_score_shared_count < 2) {
        EndgameExitReason = 4;
        GameDataStateStorage.fields_.game_state_ |= 0x00002001;
        if (communicate_late_score_shared_count != 0) {
            EndgameExitReason = 5;
        }
    }
}
communicate_late_score_zero_result: // native 0x00556B2C
DiploScratchF7E4 = GameDataStateStorage.fields_.turn_current_num_;
communicate_stack_34 = 1; // native [EBP-0x34] = 1 before has_treaty
communicate_late_truce_result_zero: // interior native target 0x00556B38
if (has_treaty(faction_id, peer_id, 1) == 0) {
    pledge_truce(faction_id, peer_id, 1);
}
diplomacy_caption(faction_id, peer_id);
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
if (DiploScratchFA78 > 5) {
    X_pops("NOTORIETY", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
}
if (PlayersData[faction_id].loan_balance[peer_id] != 0) {
    parse_num(0, PlayersData[faction_id].loan_payment[peer_id]);
    parse_num(1, PlayersData[faction_id].loan_balance[peer_id]);
    X_pops("TRUCEMEPAY", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
} else if (PlayersData[peer_id].loan_balance[faction_id] != 0) {
    parse_num(0, PlayersData[peer_id].loan_payment[faction_id]);
    parse_num(1, PlayersData[peer_id].loan_balance[faction_id]);
    X_pops("TRUCEYOUPAY", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
}
goto communicate_special_war_late_exit; // native 0x00556CDE
communicate_late_score_accepted: // native 0x00556C0F
net_set_agenda(peer_id, faction_id, 8, 1, 0);
net_treaty_off(peer_id, faction_id, 0x20, 1);
communicate_late_truce_no_offer: // interior native target 0x00556C2A
communicate_late_score_declined: // native 0x00556C2A
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_adjective_insult_faction(faction_id), -1, -1);
X_pops("MOREWAR", 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
PlayersData[faction_id].diplo_spoke[peer_id] = GameDataStateStorage.fields_.turn_current_num_;
goto communicate_final_cleanup; // native 0x005587C8
communicate_special_war_late_exit: // native 0x00556CDE
// PRIVATE native-order source proposal for 0x00556CDE..0x00556D6D.
// This joins the planetary offer continuation. The entry-result live-in is
// the existing communicate_stack_44 value (the BOOL set at 0x550078).
// Scratch names below are existing source-owned cells; no semantic rename is
// asserted by this fragment.
if (communicate_stack_44 != 0 &&
    has_treaty(peer_id, faction_id, 0x10) == 0) {
    // Native passes peer, actor, the combined 0x10004 treaty value, wait=1.
    net_treaty_on(peer_id, faction_id, 0x10004, 1); // 0x00556CFE
}
if (DiploScratchFA78 > 5 &&
    has_treaty(peer_id, faction_id, 0x20) != 0) {
    if (DiploScratchF7E0 != 0 || DiploScratchFA3C != 0 ||
        PlayersData[peer_id].mil_strength_1 >
            PlayersData[faction_id].mil_strength_1) {
        DiploScratchFA70 = 1; // 0x00556D63
    }
}
// Continue with the independent bounded-action/rank gates at 0x00556D6D.
// PRIVATE corrected candidate for 0x00556D6D..0x00556EEE. The full-start
// native row algebra resolves both computed pointers to PlayersData[peer] and
// PlayersData[actor], stride 2099 dwords. This supersedes the withdrawn first
// draft; it remains an incomplete section of communicate.
// Required function-scope locals:
//   int communicate_late_offer_amount;
//   int communicate_late_offer_special;
//   int communicate_late_offer_eligible;
communicate_late_offer_eligible = 0;
if (has_treaty(faction_id, peer_id, 2) == 0 &&
    has_agenda(faction_id, peer_id, 0x2000) == 0) {
    communicate_late_offer_eligible = 1;
    communicate_late_offer_special = 0;
    if (communicate_stack_50 != 0) {
        communicate_late_offer_special =
            has_treaty(faction_id, peer_id, 4) == 0;
    }
    communicate_late_offer_amount = communicate_wrapped_mul(
        communicate_wrapped_mul(communicate_wrapped_add(PlayersData[faction_id].mil_strength_1, 1),
                  communicate_wrapped_sub(4, PlayersData[peer_id].ai_fight)),
        2) / communicate_wrapped_add(PlayersData[peer_id].mil_strength_1, 1);
    // The pinned literal at 0x0068E2B0 is "GAIANS".
    if (strcmp(Players[peer_id].filename, "GAIANS") != 0) {
        communicate_late_offer_amount = communicate_wrapped_add(
            communicate_late_offer_amount, 2);
    }
    if (communicate_late_offer_special != 0) {
        communicate_late_offer_amount = communicate_wrapped_add(
            communicate_late_offer_amount,
            communicate_wrapped_sub(3, PlayersData[peer_id].ai_fight));
    }
    const int actor_base_count =
        static_cast<int>(PlayersData[faction_id].current_num_bases);
    if (actor_base_count != 0 &&
        PlayersData[peer_id].tech_commerce_bonus >=
            PlayersData[faction_id].tech_commerce_bonus) {
        communicate_late_offer_amount = communicate_wrapped_add(
            communicate_late_offer_amount,
            PlayersData[faction_id].unk_13 / actor_base_count);
        if (PlayersData[peer_id].tech_commerce_bonus >
            PlayersData[faction_id].tech_commerce_bonus) {
            communicate_late_offer_amount = communicate_wrapped_add(
                communicate_late_offer_amount, 1);
        }
    }
    if (great_satan(faction_id, 0) == 0) {
        const int actor_rank = static_cast<int>(PlayersData[faction_id].ranking);
        const int peer_rank = static_cast<int>(PlayersData[peer_id].ranking);
        if (actor_rank <= 4 || peer_rank <= 4) {
            communicate_late_offer_amount = communicate_wrapped_add(
                communicate_late_offer_amount, 1);
            if (DiploScratchF7FC == 0) {
                communicate_late_offer_amount = communicate_wrapped_add(
                    communicate_late_offer_amount, 1);
            }
        }
    }
}
// PRIVATE provisional draft for 0x00556EEE..0x00556FA4. It depends on the
// unresolved amount calculation in `late-contact-offer-score-556d6d-556eee.inc`
// and must not be integrated until that native indexed source owner is resolved.
// Requires the prior amount
// fragment's function-scope offer locals plus
// `int communicate_late_offer_range`. `communicate_late_offer_eligible`
// is the continuation predicate consumed by the following UI fragment.
if (communicate_late_offer_eligible != 0) {
    if ((DiploScratchFA98 == 0 &&
         DiploFriction > communicate_late_offer_amount) ||
        DiploScratchFA3C != 0) {
        communicate_late_offer_eligible = 0;
    }
}
if (communicate_late_offer_eligible != 0) {
    communicate_late_offer_amount =
        communicate_late_offer_special != 0 ||
        strcmp(Players[peer_id].filename, "GAIANS") == 0 ? 1 : 0;
    const int commerce_difference = communicate_wrapped_sub(
        PlayersData[peer_id].tech_commerce_bonus,
        PlayersData[faction_id].tech_commerce_bonus);
    communicate_late_offer_range = range(commerce_difference, 0, 99);
    const int total_score = communicate_wrapped_add(
        communicate_wrapped_add(communicate_wrapped_add(communicate_late_offer_amount,
                            communicate_late_offer_range),
                  DiploScratchFA98 != 0 ? 1 : 0),
        5);
    if (communicate_wrapped_add(DiploScratchFA78, DiploScratchFA2C) >= total_score) {
        communicate_late_offer_eligible = 0;
    }
}
if (communicate_late_offer_eligible != 0 &&
    has_treaty(faction_id, peer_id, 0x20) != 0 &&
    (DiploScratchFA98 == 0 || communicate_late_offer_special == 0 ||
     DiploScratchFA78 > 4)) {
    communicate_late_offer_eligible = 0;
}
if (communicate_late_offer_eligible != 0 &&
    wants_to_speak(faction_id, peer_id) == 0) {
    communicate_late_offer_eligible = 0;
}
// A nonzero predicate now enters parser/popup construction at 0x00556FA4.
// PRIVATE provisional candidate for the 0x00556FA4..0x005571E7 offer/popup
// continuation. It depends on unresolved upstream offer amount/indexing, so do
// not integrate the connected offer path until that is resolved.
// It depends on the preceding offer-gate fragment's live locals and is not a
// full-function replacement. All post-popup PlayerData reads are deliberately
// made after the callback, from current typed owners.
if (communicate_late_offer_eligible != 0) {
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    parse_says(2, get_adjective_faction(faction_id), -1, -1);
    parse_says(3, get_name(peer_id), -1, -1);
    parse_says(4, get_title(peer_id), -1, -1);
    int communicate_late_offer_result = 0;
    if (communicate_stack_50 == 0) {
        clear();
        if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
            says("ALIEN");
        }
        says("FACTIONTREATY");
        communicate_late_offer_result = x_pops_caption_value(
            Players[peer_id].filename, StringTemp, 0x00100000,
            FactionDiplomacyPortraits[peer_id], pop_wait);
    } else {
        clear();
        says("OFFERTREATY");
        if (has_treaty(faction_id, peer_id, 4) == 0) says("2");
        communicate_late_offer_result = X_pops(
            StringTemp, 0x00100000,
            FactionDiplomacyPortraits[peer_id], pop_wait);
    }
    communicate_stack_1c = communicate_late_offer_result; // 0x5570D8
    if (communicate_late_offer_result != 0 &&
        has_treaty(peer_id, faction_id, 0x20) == 0) {
        const int energy_limit = communicate_wrapped_mul(DiploScratchFA98, 50);
        const int peer_energy = PlayersData[peer_id].energy_reserves;
        int communicate_late_energy_offer =
            peer_energy >= energy_limit ? energy_limit : peer_energy;
        communicate_late_energy_offer = communicate_round_down_multiple(
            communicate_late_energy_offer, 25);
        if (communicate_late_energy_offer != 0) {
            parse_says(0, get_title(peer_id), -1, -1);
            parse_says(1, get_name(peer_id), -1, -1);
            parse_says(2, get_noun_phrase(faction_id, 9), -1, -1);
            parse_num(0, communicate_late_energy_offer);
            communicate_late_offer_result = X_pops(
                "ENERGYTREATY", 0x00100000,
                FactionDiplomacyPortraits[peer_id], pop_wait);
            if (communicate_late_offer_result == 0) {
                communicate_stack_54 = 1;
                communicate_stack_1c = 0;
                net_energy(peer_id, communicate_wrapped_sub(0, communicate_late_energy_offer),
                           faction_id, communicate_late_energy_offer, 1);
                ConsoleInfoWin.set_redraw_phase(0);
                ConsoleGlobal.update_data(0);
            }
        }
    }
    communicate_stack_34 = 1; // native 0x5571CB
    if (communicate_stack_1c == 0) {
        make_treaty(faction_id, peer_id);
        diplomacy_caption(faction_id, peer_id);
    }
}
// Native rejoins the next contact-mode dispatch at 0x005571E7.
// PRIVATE v3 native-owner correction for 0x005571E7..0x0055792C.
// Continues from the late-contact offer branches. This is still a WIP: the
// subsequent alliance/treaty section and common dialog tail are not included.
// EDI/ESI remain the original actor/peer arguments. Do not merge as complete.
// Native 0x5571E7..0x5575C7, preceding the retained vendetta matrix.
communicate_after_trade:
if (communicate_stack_44 != 0) wants_to_speak(faction_id, peer_id);
if ((DiplomacyEndState == 0 || DiplomacyContactResult == 0) && mode == 0) {
    goto communicate_cleanup;
}
if (communicate_stack_34 != 0) {
    goto communicate_late_contact_vendetta_matrix;
}
if (communicate_stack_38 == 0 || communicate_stack_20 != 0) {
    goto communicate_late_contact_hate_matrix;
}
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_pacts(faction_id, peer_id), -1, -1);
parse_says(3, get_pact(faction_id), -1, -1);
if (communicate_stack_48 == 0) {
    if (has_treaty(peer_id, faction_id, 1) != 0) {
        X_pops("REBUFFEDPACT", 0x100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    } else if (has_treaty(faction_id, peer_id, 6) != 0) {
        X_pops("REBUFFEDTREATY", 0x100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    } else {
        X_pops("REBUFFED", 0x100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    }
} else if (has_treaty(peer_id, faction_id, 1) != 0) {
    X_pops("MILDPACT", 0x100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
} else if (has_treaty(faction_id, peer_id, 2) != 0) {
    X_pops("MILDTREATY", 0x100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
} else {
    X_pops("MILD", 0x100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
}
goto communicate_late_contact_matrix_done;

communicate_late_contact_hate_matrix: // native 0x557395
if (mode != 0) {
    if (PlayersData[peer_id].diplo_patience[faction_id] > 1) {
        if (has_treaty(peer_id, faction_id, 1) != 0) {
            parse_says(0, get_pact(faction_id), -1, -1);
            X_pops("LOWPATIENCEPACT", 0x100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait);
        } else {
            parse_says(0, get_title(faction_id), -1, -1);
            X_pops("LOWPATIENCE", 0x100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait);
        }
    } else {
        parse_says(0, get_title(faction_id), -1, -1);
        parse_says(1, get_name(faction_id), -1, -1);
        if (has_treaty(peer_id, faction_id, 1) != 0) {
            X_pops("WHATSUPPACT", 0x100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait);
        } else if (has_treaty(peer_id, faction_id, 2) != 0) {
            X_pops("WHATSUPTREATY", 0x100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait);
        } else {
            X_pops("WHATSUP", 0x100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait);
        }
    }
} else {
    parse_says(0, get_pact_hood(faction_id, peer_id), -1, -1);
    parse_says(1, get_title(faction_id), -1, -1);
    parse_says(2, get_name(faction_id), -1, -1);
    parse_says(3, get_noun_phrase(peer_id, 9), -1, -1);
    if (has_treaty(peer_id, faction_id, 1) != 0) {
        X_pops("HELLOPACT", 0x100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    } else if (has_treaty(peer_id, faction_id, 2) != 0) {
        X_pops("HELLOTREATY", 0x100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    } else {
        X_pops("HELLO", 0x100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    }
}
goto communicate_late_contact_matrix_done;

communicate_late_contact_vendetta_matrix: // native 0x005575C7
if (has_treaty(peer_id, faction_id, 0x60) != 0 &&
    (mode != 0 || DiploScratchFA78 >= 5)) {
    goto communicate_late_contact_matrix_done; // native 0x005575E6
}
communicate_late_contact_rank = 1;
communicate_late_contact_best = 0;
communicate_late_contact_best_id = -1;
for (communicate_late_contact_rank = 1;
     communicate_late_contact_rank < MaxPlayerNum;
     ++communicate_late_contact_rank) {
    if (communicate_late_contact_rank == faction_id ||
        communicate_late_contact_rank == peer_id) {
        continue;
    }
    if (has_treaty(communicate_late_contact_rank, faction_id, 0x10) != 0) {
        continue;
    }
    // Native reads the actor's diplo_wrongs row at 0x557638 (96CB30 base plus
    // actor row stride and candidate index), retaining the first strict max.
    if (PlayersData[faction_id].diplo_wrongs[communicate_late_contact_rank] >
        communicate_late_contact_best) {
        communicate_late_contact_best = PlayersData[faction_id].diplo_wrongs[
            communicate_late_contact_rank];
        communicate_late_contact_best_id = communicate_late_contact_rank;
    }
}
if (communicate_late_contact_best_id < 0) {
    goto communicate_late_contact_matrix_done; // native 0x0055765F
}
parse_says(0, get_title(communicate_late_contact_best_id), -1, -1);
parse_says(1, get_name(communicate_late_contact_best_id), -1, -1);
parse_says(2, get_adjective_insult_faction(communicate_late_contact_best_id), -1, -1);
parse_says(3, get_noun(communicate_late_contact_best_id), -1, -1);
parse_says(4, get_title(faction_id), -1, -1);
parse_says(5, get_name(faction_id), -1, -1);
PlayersData[faction_id].diplo_wrongs[communicate_late_contact_best_id] = 0;
clear();
says("WRONGED");
if (has_treaty(faction_id, communicate_late_contact_best_id, 0x10) != 0) {
    says("ENEMY");
} else if (PlayersData[peer_id].mil_strength_1 >
           PlayersData[faction_id].mil_strength_1) {
    says("MIGHTY");
} else {
    says("MEEK");
}
X_pops(StringTemp, 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
wants_to_speak(faction_id, peer_id);
communicate_late_contact_matrix_done: // native 0x0055778A
DiploScratchF7B4 = 0;
communicate_late_contact_rank = 0;
// Continue at the separate ordered player-rule response scan, native 0x5577A2.
// PRIVATE native-order translation proposal for 0x005577A2..0x0055792C.
// Requires function-scope int communicate_late_social_slot. The loop visits
// the peer's priority and opposition categories in order, using the matching
// ideology model field for each slot. Native does not bounds-check categories;
// do not add one here.
for (communicate_late_social_slot = 0;
     communicate_late_social_slot < 1;
     ++communicate_late_social_slot) {
    const int category = communicate_late_social_slot == 0
        ? Players[peer_id].soc_ideology_category
        : Players[peer_id].soc_anti_ideology_category;
    const int model = communicate_late_social_slot == 0
        ? Players[peer_id].soc_ideology_model
        : Players[peer_id].soc_anti_ideology_model;
    if (category < 0 || model == 0) {
        continue;
    }
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    clear();
    says("SOCIAL");
    say_num(category);
    says("CAT");
    const int pending_model = static_cast<int>(
        PlayersData[faction_id].soc_category_pending.models_[category]);
    if (pending_model == model) {
        say_num(model);
        says("GOOD");
        X_pops(StringTemp, 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
        goto communicate_late_social_terms_done;
    }
    if (pending_model == 0) {
        continue;
    }
    DiploScratchF7B4 = 1; // Native store at 0x0055782E precedes these gates.
    if (DiploScratchFA98 != 0 ||
        (has_treaty(peer_id, faction_id, 1) != 0 &&
         has_treaty(peer_id, faction_id, 0x02000000) != 0)) {
        continue;
    }
    say_num(pending_model);
    says("BAD");
    say_num(model);
    X_pops(StringTemp, 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
    net_set_agenda(peer_id, faction_id, 1, 1, 1);
    goto communicate_late_social_terms_done;
}
communicate_late_social_terms_done: // native 0x0055792C
// PRIVATE v3 native-order correction for 0x0055792C..0x00558152.
// Preserves v1/v2; applies native per-region/map-mask/base-count gates. Scratch F7B4 is centrally admitted
// neutral start-only owner; its extent/lifecycle/meaning remain unknown.
// Required locals: rival ID/counters, region ID, selected IDs/ratings and the
// temporary region-hit booleans. Verify this section in the composed host body.
if (DiploScratchF7B4 == 0) {
    net_set_agenda(faction_id, peer_id, 1, 0, 1); // 0x557935
}
if (PlayersData[faction_id].atrocities != 0) {
    if (has_treaty(peer_id, faction_id, 0x00200000) != 0) {
        if (has_treaty(peer_id, faction_id, 0x00400000) == 0) {
            goto communicate_late_contact_atrocity_done;
        }
        // 0x96CDBC is PlayerData.ai_fight, not ai_growth.
        if (PlayersData[faction_id].atrocities <=
            communicate_wrapped_add(PlayersData[peer_id].ai_fight, 1) ||
            (Players[faction_id].rule_flags & 0x80) != 0 ||
            (has_treaty(peer_id, faction_id, 1) != 0 &&
             has_treaty(peer_id, faction_id, 0x02000000) != 0)) {
            goto communicate_late_contact_atrocity_done;
        }
        for (communicate_late_contact_rank = 1;
             communicate_late_contact_rank < MaxPlayerNum;
             ++communicate_late_contact_rank) {
            if (communicate_late_contact_rank == faction_id ||
                communicate_late_contact_rank == peer_id) {
                continue;
            }
            if (has_treaty(communicate_late_contact_rank, faction_id,
                           0x00040000) != 0) {
                goto communicate_late_contact_third_party_atrocity;
            }
        }
    }
    parse_says(0, get_title(faction_id), -1, -1);
    parse_says(1, get_name(faction_id), -1, -1);
    X_pops("SELFATROCITY", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
    if (has_treaty(peer_id, faction_id, 0x00200000) == 0) {
        if (PlayersData[faction_id].atrocities > 1) {
            PlayersData[faction_id].atrocities = communicate_wrapped_sub(
                PlayersData[faction_id].atrocities, 1);
        }
        if (PlayersData[faction_id].integrity_blemishes > 1) {
            PlayersData[faction_id].integrity_blemishes = communicate_wrapped_sub(
                PlayersData[faction_id].integrity_blemishes, 1);
        }
    }
    net_set_treaty(peer_id, faction_id, 0x00200000, 1, 0);
    net_set_treaty(peer_id, faction_id, 0x00400000, 0, 0);
    goto communicate_late_contact_atrocity_done;
}
// The native outer-attempt guard also skips the committed-atrocity block
// when actor.atrocities is zero; the label is not a fallthrough continuation.
goto communicate_late_contact_atrocity_done;
communicate_late_contact_third_party_atrocity:
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_noun(communicate_late_contact_rank), -1, -1);
X_pops("COMMITTEDATROCITY", 0x00100000,
             FactionDiplomacyPortraits[peer_id], pop_wait);
if (has_treaty(peer_id, faction_id, 0x00200000) == 0) {
    if (PlayersData[faction_id].atrocities > 1) {
        PlayersData[faction_id].atrocities = communicate_wrapped_sub(
            PlayersData[faction_id].atrocities, 1);
    }
    if (PlayersData[faction_id].integrity_blemishes > 1) {
        PlayersData[faction_id].integrity_blemishes = communicate_wrapped_sub(
            PlayersData[faction_id].integrity_blemishes, 1);
    }
}
net_set_treaty(peer_id, faction_id, 0x00200000, 1, 0);
net_set_treaty(peer_id, faction_id, 0x00400000, 0, 0);
communicate_late_contact_atrocity_done: // native 0x557AC0
if (has_treaty(faction_id, peer_id, 1) != 0) {
    parse_says(0, get_pact(faction_id), -1, -1);
    net_maps(faction_id, peer_id, 1);
    if (DiplomacyMapTransferChanged != 0) {
        X_pops("HERESTHEMAP", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    }
}
if (has_treaty(faction_id, peer_id, 1) == 0 &&
    has_treaty(peer_id, faction_id, 0x80) == 0) {
    goto communicate_late_contact_map_done; // 0x557B3A
}
if ((communicate_stack_4c & 0x40) != 0) {
    goto communicate_late_contact_map_done; // 0x557B44
}
communicate_late_contact_general_rival = 0;
communicate_late_contact_general_strength = 0;
communicate_late_contact_region_rival = 0;
communicate_late_contact_region_strength = 0x270F;
communicate_late_contact_rank = 1;
for (; communicate_late_contact_rank < MaxPlayerNum;
     ++communicate_late_contact_rank) {
    const int rival = communicate_late_contact_rank;
    if (is_alive(rival) == 0 || rival == faction_id || rival == peer_id ||
        has_treaty(peer_id, rival, 0x10) == 0 ||
        has_treaty(peer_id, rival, 8) == 0 ||
        has_treaty(peer_id, rival, 0x8000) == 0 ||
        has_treaty(faction_id, rival, 8) == 0 ||
        (has_treaty(peer_id, rival, 0x40) == 0 &&
         has_treaty(peer_id, faction_id, 1) == 0)) {
        continue;
    }
    const int rival_strength = PlayersData[rival].mil_strength_1;
    if (rival_strength > communicate_late_contact_general_strength) {
        communicate_late_contact_general_strength = rival_strength;
        communicate_late_contact_general_rival = rival;
    }
    if (rival_strength >= communicate_late_contact_region_strength) {
        continue;
    }
    // The native pass tests each region in order; bad_reg receives the
    // current region (not a one-time constant 1) before its map/data checks.
    for (communicate_late_contact_region = 1;
         communicate_late_contact_region < 0x40;
         ++communicate_late_contact_region) {
        const int region = communicate_late_contact_region;
        if (bad_reg(region) != 0 ||
            PlayersData[peer_id].region_total_bases[region] == 0 ||
            PlayersData[rival].region_total_bases[region] == 0) {
            continue;
        }
        const int peer_region_total = communicate_wrapped_add(
            static_cast<int>(PlayersData[peer_id].region_total_combat_vehs[region]),
            static_cast<int>(PlayersData[peer_id].region_force_rating[region]));
        if (PlayersData[rival].region_force_rating[region] <= peer_region_total ||
            contiguous(peer_id, rival, region, 0) == 0) {
            continue;
        }
        communicate_late_contact_region_rival = rival;
        communicate_late_contact_region_strength = rival_strength;
        break;
    }
}
// Native independently examines the strongest rival's mapped regions. A
// REPORTLOSING dialog appears if actor/peer have the 0x2000 treaty bit or a
// mapped region has more rival combat units than the peer's region count.
communicate_late_contact_general_region_hit = 0;
if (communicate_late_contact_general_rival != 0) {
    for (communicate_late_contact_region = 1;
         communicate_late_contact_region < 128;
         ++communicate_late_contact_region) {
        const int region = communicate_late_contact_region;
        if (bad_reg(region) != 0 || DiplomacyRegionMask[region] == 0 ||
            PlayersData[communicate_late_contact_general_rival]
                .region_total_bases[region] == 0 ||
            PlayersData[peer_id].region_total_bases[region] == 0) {
            continue;
        }
        if (PlayersData[communicate_late_contact_general_rival]
                .region_total_combat_vehs[region] >
            PlayersData[peer_id].region_total_combat_vehs[region]) {
            communicate_late_contact_general_region_hit = 1;
            break;
        }
    }
    if (has_treaty(faction_id, peer_id, 0x2000) != 0 ||
        communicate_late_contact_general_region_hit != 0) {
        parse_says(1, get_title(communicate_late_contact_general_rival), -1, -1);
        parse_says(2, get_name(communicate_late_contact_general_rival), -1, -1);
        parse_says(3, get_noun(communicate_late_contact_general_rival), -1, -1);
        parse_says(4, get_his_her(communicate_late_contact_general_rival, 0), -1, -1);
        parse_says(5, get_adjective_insult_faction(
                          communicate_late_contact_general_rival), -1, -1);
        X_pops("REPORTLOSING", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    }
}
// Native separately tests the rival selected by the inner contiguous scan.
// This is intentionally not merged with the general strongest-rival report.
if (communicate_late_contact_region_rival != 0) {
    if (has_treaty(faction_id, peer_id, 0x2000) == 0) {
        communicate_late_contact_region_reported = 0;
        for (communicate_late_contact_region = 1;
             communicate_late_contact_region < 128;
             ++communicate_late_contact_region) {
            const int region = communicate_late_contact_region;
            if (bad_reg(region) != 0 ||
                DiplomacyRegionMask[region] == 0) {
                continue;
            }
            const uint32_t rival_bit = 1u <<
                communicate_late_contact_region_rival;
            if (rival_bit == 0) {
                continue;
            }
            if (PlayersData[peer_id].region_force_rating[region] >
                PlayersData[communicate_late_contact_region_rival]
                    .region_total_combat_vehs[region]) {
                parse_says(0, get_adjective(
                                  communicate_late_contact_region_rival), -1, -1);
                parse_says(1, get_title(
                                  communicate_late_contact_region_rival), -1, -1);
                parse_says(2, get_name(
                                  communicate_late_contact_region_rival), -1, -1);
                parse_says(3, get_phrase(
                                  communicate_late_contact_region_rival, 5), -1, -1);
                X_pops("REPORTWINNING", 0x00100000,
                             FactionDiplomacyPortraits[peer_id], pop_wait);
                communicate_late_contact_region_reported = 1;
                break;
            }
        }
        if (communicate_late_contact_region_reported == 0) {
            for (communicate_late_contact_region = 1;
                 communicate_late_contact_region < 128;
                 ++communicate_late_contact_region) {
                const int region = communicate_late_contact_region;
                if (bad_reg(region) != 0 ||
                    PlayersData[communicate_late_contact_region_rival]
                        .region_total_bases[region] == 0 ||
                    PlayersData[peer_id].region_total_bases[region] == 0) {
                    continue;
                }
                parse_says(1, get_title(
                                  communicate_late_contact_region_rival), -1, -1);
                parse_says(2, get_name(
                                  communicate_late_contact_region_rival), -1, -1);
                parse_says(3, get_noun(
                                  communicate_late_contact_region_rival), -1, -1);
                parse_says(5, get_noun_phrase(
                                  communicate_late_contact_region_rival, 8), -1, -1);
                X_pops("REPORTEVEN", 0x00100000,
                             FactionDiplomacyPortraits[peer_id], pop_wait);
                break;
            }
        }
    }
}
communicate_late_contact_map_done: // native 0x558152
// PRIVATE, STILL-INCOMPLETE function section: native 0x00558152..0x005584AC.
// This version replaces the rough tech/territory draft, preserving the
// original branch predicates, directionality, typed owners, and ordered
// bitmask/parser side effects. It is not a standalone function.
// Native asks whether the peer (ESI) has treaty bit 1 with this actor (EDI).
if (has_treaty(peer_id, faction_id, 1) != 0) {
    mention_prototypes(faction_id, peer_id); // 0x558162
}
if (has_treaty(peer_id, faction_id, 1) != 0 &&
    (GameDataStateStorage.fields_.game_rules_ & 0x02000000) == 0) {
    communicate_late_contact_research_tech =
        PlayersData[peer_id].tech_id_researching;
    if (communicate_late_contact_research_tech >= 0 &&
        has_tech(communicate_late_contact_research_tech, faction_id) == 0 &&
        communicate_late_contact_research_tech !=
            PlayersData[faction_id].tech_id_researching &&
        (GameDataStateStorage.fields_.game_rules_ & 0x06000000) == 0 &&
        PlayersData[faction_id].tech_accumulated >=
            tech_rate(faction_id) / 2) {
        communicate_late_contact_best_tech_value = 0;
        communicate_late_contact_best_tech = -1;
        for (communicate_late_contact_tech = 0;
             communicate_late_contact_tech < MaxTechnologyNum;
             ++communicate_late_contact_tech) {
            const int tech_id = communicate_late_contact_tech;
            const RulesTechnology &candidate = Technology[tech_id];
            if (has_tech(tech_id, faction_id) != 0 ||
                tech_id == PlayersData[faction_id].tech_id_researching ||
                tech_id == PlayersData[peer_id].tech_id_researching ||
                tech_avail(tech_id, faction_id) == 0) {
                continue;
            }
            // 0x55825D..0x55827F is signed IDIV by 3. For IDs below 87,
            // the image rejects remainder zero; for IDs 87 and 88 it rejects
            // nonzero remainder. Keep the native 32-bit wrapped numerator.
            const int alignment = communicate_wrapped_add(
                communicate_wrapped_add(communicate_wrapped_sub(tech_id,
                                    PlayersData[faction_id].tech_ranking),
                          faction_id), 2);
            const int alignment_remainder = alignment % 3;
            if ((tech_id < 87 && alignment_remainder == 0) ||
                (tech_id >= 87 && alignment_remainder != 0)) {
                continue;
            }
            if (has_tech(candidate.preq_tech_1, faction_id) == 0 ||
                has_tech(candidate.preq_tech_2, faction_id) == 0) {
                continue;
            }
            const int candidate_value = tech_val(tech_id, peer_id, 0);
            if (candidate_value >= 0x270F) {
                break;
            }
            if (candidate_value > communicate_late_contact_best_tech_value) {
                communicate_late_contact_best_tech_value = candidate_value;
                communicate_late_contact_best_tech = tech_id;
            }
        }
        if (communicate_late_contact_best_tech >= 0) {
            // Native order: mark the peer's current research in the peer row,
            // parse it into slot 0, then mark/parse the selected tech in the
            // actor row/slot 1. Do not collapse or reorder these effects.
            bitmask(communicate_late_contact_research_tech,
                    &communicate_late_contact_tech_mask_offset,
                    &communicate_late_contact_tech_mask);
            PlayersData[peer_id].tech_pact_shared_goals[
                communicate_late_contact_tech_mask_offset] |=
                    static_cast<uint32_t>(communicate_late_contact_tech_mask);
            parse_say(0, Technology[communicate_late_contact_research_tech].name, -1, -1);
            bitmask(communicate_late_contact_best_tech,
                    &communicate_late_contact_tech_mask_offset,
                    &communicate_late_contact_tech_mask);
            PlayersData[faction_id].tech_pact_shared_goals[
                communicate_late_contact_tech_mask_offset] |=
                    static_cast<uint32_t>(communicate_late_contact_tech_mask);
            parse_say(1, Technology[communicate_late_contact_best_tech].name, -1, -1);
            X_pops("TEAMTECH", 0x00100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait);
        }
    }
}
if (has_treaty(peer_id, faction_id, 3) == 0) { // 0x558392
    communicate_late_contact_base_id = -1;
    if (territory(peer_id, faction_id, 0,
                  &communicate_late_contact_base_id, 0) != 0) {
        spot_base(communicate_late_contact_base_id, faction_id);
        console_map_win()->focus(
            Bases[communicate_late_contact_base_id].x,
            Bases[communicate_late_contact_base_id].y);
        parse_says(1, get_pact(peer_id), -1, -1);
        X_pops("TERRITORY", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
        mention_prototypes(faction_id, peer_id); // 0x558425
    }
}
if (DiploScratchFA4C != 0) { // 0x55842D
    parse_says(0, get_title(DiploScratchF7B8), -1, -1);
    parse_says(1, get_name(DiploScratchF7B8), -1, -1);
    parse_says(2, get_adjective_insult_leader(DiploScratchF7B8), -1, -1);
    X_pops("QUISLING", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
}
// Native continuation point 0x5584AC.
// PRIVATE, INCOMPLETE correction of native 0x005584AC..0x00558980.
// Supersedes v2 only for the approved typed start-only neutral owners at
// 0x93FAD0 and 0x93FAD4. The owner proposal does not assert their allocation
// extents beyond the separately admitted DWORD cells or infer their roles.
DiploScratchFAD0 = 0; // native 0x5584AE
DiploScratchFAD4 = 0; // native 0x5584B9
communicate_patience_ptr =
    &PlayersData[peer_id].diplo_patience[faction_id];
communicate_contact_faction_scan: // native re-entry 0x005584D2
if (*communicate_patience_ptr < 0) {
    *communicate_patience_ptr = 0;
}
DiploScratchFA94 = -1;
DiploScratchF7DC = -1;
for (communicate_late_contact_rank = 1;
     communicate_late_contact_rank < MaxPlayerNum;
     ++communicate_late_contact_rank) {
    if (is_alive(communicate_late_contact_rank) == 0 ||
        communicate_late_contact_rank == faction_id ||
        communicate_late_contact_rank == peer_id) {
        continue;
    }
    if (has_treaty(peer_id, communicate_late_contact_rank, 0x10) == 0 ||
        has_treaty(faction_id, communicate_late_contact_rank, 8) == 0) {
        continue;
    }
    if (has_treaty(faction_id, communicate_late_contact_rank, 7) != 0) {
        DiploScratchF7DC = communicate_late_contact_rank;
    }
    if (has_treaty(faction_id, communicate_late_contact_rank, 0x10) != 0) {
        DiploScratchFA94 = communicate_late_contact_rank;
    }
}
if (IsMultiplayerNet != 0 && (GameDataStateStorage.fields_.game_state_ & 0x800) != 0) {
    goto communicate_late_contact_exit;
}
{
    communicate_late_contact_choice = diplomacy_menu(faction_id, peer_id);
    if (static_cast<uint32_t>(communicate_late_contact_choice) <= 7u) {
        switch (communicate_late_contact_choice) {
        case 0:
            goto communicate_final_cleanup; // native 0x5587C8
        case 1:
        case 2:
            make_a_proposal(faction_id, peer_id,
                            communicate_late_contact_choice == 1);
            if (DiplomacyProposalMode == 6) {
                goto communicate_late_contact_after_proposal; // 0x558701
            }
            goto communicate_late_contact_menu_loop; // 0x55870B
        case 3:
            communicate_late_contact_text_result = X_pops(
                "REALLYBROKE", 0x00100000,
                FactionDiplomacyPortraits[peer_id], pop_wait);
            if (communicate_late_contact_text_result == 0) {
                // Native re-enters after showing BROKEPACT and decrementing
                // the signed patience byte.
                parse_says(0, get_pact_hood(faction_id, peer_id), -1, -1);
                X_pops("DROIDS", 0x00100000,
                             FactionDiplomacyPortraits[peer_id], pop_wait);
                --*communicate_patience_ptr;
                goto communicate_late_contact_menu_loop; // native 0x55870B
            }
            diplo_lock(0);
            PlayersData[faction_id].integrity_blemishes = communicate_wrapped_add(
                PlayersData[faction_id].integrity_blemishes, 1);
            net_treaty_off(peer_id, faction_id, 1, 1);
            diplomacy_check(faction_id, peer_id, 0);
            diplomacy_caption(faction_id, peer_id);
            parse_says(0, get_pact(faction_id), -1, -1);
            parse_says(1, get_title(faction_id), -1, -1);
            parse_says(2, get_name(faction_id), -1, -1);
            X_pops("BROKEPACT", 0x00100000,
                         FactionDiplomacyPortraits[peer_id], pop_wait);
            net_pact_ends(faction_id, peer_id, 1);
            diplo_unlock();
            goto communicate_late_contact_menu_loop; // native 0x55870B
        case 4:
            demand_withdrawal(faction_id, peer_id);
            goto communicate_late_contact_menu_loop;
        case 5:
            battle_plans(faction_id, peer_id);
            goto communicate_late_contact_menu_loop;
        case 6:
            call_off_vendetta(faction_id, peer_id);
            goto communicate_late_contact_menu_loop;
        case 7:
            DiploScratchFA34 = 12; // admitted start-only cell at 0x93FA34
            DiplomacyProposalMode = 1;
            make_a_proposal(faction_id, peer_id, 1);
            goto communicate_late_contact_after_proposal;
        }
    }
}
communicate_late_contact_after_proposal: // native 0x558701
DiploScratchFAD0 = 0;
communicate_late_contact_menu_loop: // native 0x55870B
*communicate_patience_ptr = static_cast<char>(
    static_cast<uint8_t>(*communicate_patience_ptr) + 1u);
if (static_cast<int8_t>(*communicate_patience_ptr) <
    get_patience(faction_id, peer_id)) {
    goto communicate_contact_faction_scan; // native 0x5584D2
}
communicate_late_contact_exit: // native 0x558725
diplomacy_caption(faction_id, peer_id);
if (has_treaty(faction_id, peer_id, 0x10) != 0) {
    goto communicate_final_cleanup;
}
parse_says(0, get_title(faction_id), -1, -1);
parse_says(1, get_name(faction_id), -1, -1);
parse_says(2, get_name(peer_id), -1, -1);
if (has_treaty(peer_id, faction_id, 1) != 0) {
    X_pops("TIMESUPALLY", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
} else {
    X_pops("TIMESUP", 0x00100000,
                 FactionDiplomacyPortraits[peer_id], pop_wait);
}
communicate_cleanup: // native common cleanup entry
communicate_common_epilogue:
communicate_epilogue:
communicate_final_cleanup: // native 0x5587C8
net_treaty_off(peer_id, faction_id, 0x2000, 0);
net_agenda_off(peer_id, faction_id, 4, 0);
communicate_late_contact_text_result = 0;
if (DiploScratchFAA0 != 0 &&
    has_treaty(faction_id, peer_id, 0x11) == 0) {
    communicate_late_contact_text_result = 1;
}
net_set_agenda(peer_id, faction_id, 0x10,
               communicate_late_contact_text_result, 0);
communicate_late_contact_text_result = 0;
if (DiploScratchFA70 != 0 &&
    has_treaty(faction_id, peer_id, 0x11) == 0) {
    communicate_late_contact_text_result = 1;
}
net_set_treaty(peer_id, faction_id, 0x400,
               communicate_late_contact_text_result, 1);
if (communicate_stack_54 != 0 || (DiploScratchFA60 != 0 &&
    has_treaty(faction_id, peer_id, 0x10) == 0)) {
    char &patience = PlayersData[peer_id].diplo_patience[faction_id];
    patience = static_cast<char>(range(
        static_cast<int8_t>(patience),
        get_patience(faction_id, peer_id), 0x3E7));
}
diplomacy_ends(faction_id, peer_id);
if (communicate_stack_50 != 0 &&
    has_treaty(faction_id, peer_id, 0x10013) == 0 && mode == 0) {
    parse_says(0, get_title(peer_id), -1, -1);
    parse_says(1, get_name(peer_id), -1, -1);
    parse_says(2, get_noun(peer_id), -1, -1);
    if (has_treaty(faction_id, peer_id, 4) != 0) {
        X_pops("RENEWTRUCE", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    } else {
        X_pops("ENDTRUCE", 0x00100000,
                     FactionDiplomacyPortraits[peer_id], pop_wait);
    }
}
goto communicate_popup_destructor; // native 0x55895D
communicate_popup_destructor: // cold span 0x65FF4F..0x65FF64
// Let the single lexical Popup in the eventual whole body run its destructor.

; // The automatic Popup generates both normal and exceptional destruction.

}

