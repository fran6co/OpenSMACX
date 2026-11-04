#include "stdafx.h"
#include "faction.h"
#include "alpha.h"
#include "base.h"
#include "console.h"
#include "council.h"
#include "path.h"
#include "popup.h"
#include "technology.h"

// Original: 0x00527290..0x005275AC, ?faction_upkeep@@YAXH@Z.

static int upkeep_wrap_add(int left, int right) {
    return (int32_t)((uint32_t)left + (uint32_t)right);
}

static int upkeep_wrap_subtract(int left, int right) {
    return (int32_t)((uint32_t)left - (uint32_t)right);
}

// ORIGINAL: 0x00527290 ?faction_upkeep@@YAXH@Z 0x00527290-0x005275AC
// RECOVERY_BINDING: v1 {"evidence":["original:0x00527290-0x005275AC:the full pinned function span is faction upkeep","original:0x0052756D-0x00527581:the final call_council gate tests the HUMAN mask and calls for nonhuman factions","original:0x00527506-0x0052750B:the two Path destination words are reset independently at function exit","source:src/faction.h:Players and PlayersData own faction state and turn bookkeeping","source:src/path.h:Path owns x_dst_ and y_dst_ and the candidate uses its reset operation","source:src/council.h:can_call_council and call_council are the canonical council interfaces"],"reason":"Recover faction upkeep in its faction owner with the native nonhuman council gate, fresh callback state, and Path-owned destination reset."}
// TRIED: literal asm-mirrored translation compiles and matches through instr #41 (of ~230); diverges at #42 (shl vs and, likely the corner_market/has_tech gated block's field-offset codegen) - not chased further, landed at MISMATCH.
void __cdecl faction_upkeep(int faction_id) {
    // Native literal identities remain ordinary C++ literals: "SANCTIONSENDALIEN" 0x68B9F4..0x68BA06, "council_sm.pcx" 0x68BA08..0x68BA17, "SANCTIONSEND" 0x68BA18..0x68BA25, "econwin_sm.pcx" 0x68BA28..0x68BA37, "CORNERWARNING" 0x68BA38..0x68BA46, and "COUNCILOPEN" 0x68BA58..0x68BA64.

    PlayerData &faction_data = PlayersData[faction_id];
    Player &faction = Players[faction_id];

    UnkGlobal0093A934 = 1;
    social_upkeep(faction_id);
    do_all_non_input();
    repair_phase(faction_id);
    do_all_non_input();
    production_phase(faction_id);
    do_all_non_input();

    if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) == 0 || (GameDataStateStorage.fields_.game_state_ & STATE_UNK_8) != 0) {
        allocate_energy(faction_id);
        do_all_non_input();
        enemy_diplomacy(faction_id);
        do_all_non_input();
        enemy_strategy(faction_id);
        do_all_non_input();
        social_ai(faction_id, -1, -1, -1, -1, 0);
        do_all_non_input();

        if (!is_human((uint32_t)faction_id)) {
            const int amount = corner_market(faction_id);
            if (amount < faction_data.energy_reserves &&
                has_tech(Rules.tech_economic_victory, faction_id) &&
                (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_ECONOMIC) != 0) {
                faction_data.corner_market_turn = upkeep_wrap_add(
                    GameDataStateStorage.fields_.turn_current_num_, (int)Rules.turns_corner_gbl_energy_mrkt);
                faction_data.corner_market_active = corner_market(faction_id);
                faction_data.energy_reserves = upkeep_wrap_subtract(
                    faction_data.energy_reserves, corner_market(faction_id));

                GenderDefault = faction.is_leader_female;
                PluralityDefault = 0;
                parse_says(0, faction.title_leader, -1, -1);

                GenderDefault = faction.is_leader_female;
                PluralityDefault = 0;
                parse_says(1, faction.name_leader, -1, -1);

                GenderDefault = faction.noun_gender;
                PluralityDefault = faction.is_noun_plural;
                parse_says(2, faction.noun_faction, -1, -1);
                parse_says(3, faction.adj_name_faction, -1, -1);

                parse_num(0, (int)game_year(faction_data.corner_market_turn));
                popp("SCRIPT", "CORNERWARNING", 0, "econwin_sm.pcx", 0);
            }
        }
    }

    int base_id = 0;
    if (GameDataStateStorage.fields_.base_current_count_ > 0) {
        int remaining_bases = GameDataStateStorage.fields_.base_current_count_;
        do {
            if (Bases[base_id].faction_id_current == faction_id) {
                // Native 0x52748E clears bits 0 and 30; bit 0's role is unknown.
                Bases[base_id].state &= ~(BSTATE_PRODUCTION_HURRIED |
                                          BSTATE_UNK_1);
            }
            ++base_id;
            --remaining_bases;
        } while (remaining_bases != 0);
    }

    const int hurry_cost = faction_data.hurry_cost_total;
    faction_data.energy_reserves = upkeep_wrap_subtract(
        faction_data.energy_reserves, hurry_cost);
    faction_data.hurry_cost_total = 0;
    if (faction_data.energy_reserves < 0) {
        faction_data.energy_reserves = 0;
    }

    if (faction_data.current_num_bases == 0 &&
        faction_data.proto_id_active[0] == 0 &&
        faction_data.proto_id_active[21] == 0) {
        eliminate_player(faction_id, 0);
    }

    UnkGlobal0093A934 = 0;
    Paths.clear_destination();

    if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) == 0 || (GameDataStateStorage.fields_.game_state_ & STATE_UNK_8) != 0) {
        if (faction_id == ConsoleGlobal.faction_id_ &&
            (GameDataStateStorage.fields_.game_state_ & (STATE_DISPLAYED_COUNCIL_AVAIL_MSG |
                          STATE_COUNCIL_HAS_CONVENED)) == 0) {
            const int council_available = can_call_council(faction_id, 0);
            if (council_available != 0 && (GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) == 0) {
                GameDataStateStorage.fields_.game_state_ |= STATE_DISPLAYED_COUNCIL_AVAIL_MSG;
                popp("SCRIPT", "COUNCILOPEN", 0, "council_sm.pcx", 0);
            }
        }
        // The native test is the HUMAN bit, not is_alive(). Re-read after the
        // council-availability callback rather than caching prior faction state.
        if (!is_human((uint32_t)faction_id)) {
            call_council(faction_id);
        }
    }

    if (IsMultiplayerNet == 0 &&
        (GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_AUTOSAVE_EACH_TURN) != 0 &&
        faction_id == ConsoleGlobal.faction_id_) {
        auto_save();
    }
}
