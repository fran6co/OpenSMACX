// Per-vehicle enemy movement, using genuine source-owned game APIs.
#include "stdafx.h"
#include "alpha.h"
#include "base.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "general.h"
#include "map.h"
#include "path.h"
#include "probe.h"
#include "technology.h"
#include "win.h"
#include "terraforming.h"
#include "veh.h"
#include "xpops.h"
#include "popup.h"
#include "council.h"
#include "enemy_pb_policy.h"
#include "log.h"
#include "netmsg.h"
#include "crt_random.h"
#include "crt_random_inline.h"
#include <stddef.h>
#include <string.h>

/*
Purpose: Direct the unit to start moving automatically towards the specified tile.
// ORIGINAL: 0x00560AD0 ?go_to@@YAXHDHH@Z 0x00560AD0-0x00560B25 BYTE_EXACT
// size      85 bytes
// prototype void (__cdecl ?go_to@@YAXHDHH@Z)(int vehID, int8 type, int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: set_course's two calls to go_to are
// both inlined in the image (its own `calls` list has neither), while
// scenario.cpp's single call stays real - `osmx calls` on set_course names
// no go_to among its targets. LEVER: matches bitmask/battle_init above.

/*
Purpose: Determine ideal non-offense (defense, combat, recon) unit count for the specified base.
// ORIGINAL: 0x00560B30 ?garrison_check@@YAHH@Z 0x00560B30-0x00560D30 BYTE_EXACT
// size      512 bytes
// prototype int (__cdecl ?garrison_check@@YAHH@Z)(int baseID)
// callers   4   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x005AC060 0x005C8BA0
// LEVER: deleted the `BOOL is_obj = is_objective(base_id);` local and wrote
//        is_objective(base_id) at both use sites - the image calls
//        0x5AC060 (is_objective) twice, not once with a cached result. Best
//        similarity 0.763 -> 0.809; agreeing count unchanged at 4/170. The
//        bitmask() half of the evidence (has_fac_built folding to a byte
//        test) is the documented systemic gap - TRIED on has_fac
//        base.cpp:104 - and not fixable here.
// LEVER: that gap IS fixable here, for this one site - the FAC_HEADQUARTERS
//        test has a LITERAL facility id, so routing it through
//        has_fac_built_call forces the real `call 0x50ba00` the image makes
//        (base_making/pop_goal/pop_goal_fac's lever). Compiled body
//        159 -> 167 instructions against the image's 170, differing runs
//        26 -> 19. NOTE the same lever is the WRONG way round in
//        value_of_base (0x0054CB50): its `has_fac_built(fac, base_id)` has
//        a RUNTIME fac, and forcing the call there costs 3 instructions and
//        5 more differing runs - reverted.
Return Value: Amount of non-offensive units needed (1-10)
Status: Complete
*/
int __cdecl garrison_check(int base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    int region = region_at(Bases[base_id].x, Bases[base_id].y);
    int sp_count = 0;
    for (int project = 0; project < MaxSecretProjectNum; project++) {
        if (GameDataStateStorage.fields_.secret_project_.project_base[project] == base_id) {
            sp_count++;
        }
    }
    int garrison = (sp_count + 2) / 3 + (Bases[base_id].population_size + 1) / 4 + 1;
    if (has_fac(FAC_HEADQUARTERS, base_id, 0)
        || bit_at(Bases[base_id].x, Bases[base_id].y) & BIT_UNK_40000000
        || is_objective(base_id)) {
        garrison++;
    }
    if (is_objective(base_id) && PlayersData[faction_id].flags & PFLAG_STRAT_DEF_OBJECTIVES) {
        garrison++;
    }
    if (PlayersData[faction_id].region_base_plan[region] == PLAN_COLONIZATION) {
        garrison--;
    }
    int faction_id_sea = zoc_sea(Bases[base_id].x, Bases[base_id].y, faction_id) - 1;
    if (faction_id_sea > 0
        && ((PlayersData[faction_id].diplo_treaties[faction_id_sea]
             & (DTREATY_VENDETTA | DTREATY_WANT_REVENGE))
        || (PlayersData[faction_id_sea].diplo_treaties[faction_id] & DTREATY_WANT_REVENGE)
        || PlayersData[faction_id_sea].integrity_blemishes >= 4)) {
        garrison++;
    }
    int support = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::SUPPORT];
    if (support <= -4) {
        garrison--;
    }
    if (support <= -3 && garrison > 2) {
        garrison--;
    }
    return range(garrison, 1, 10);
}

/*
Purpose: Determine the number of defensive units needed to protect the specified base.
// ORIGINAL: 0x00560D30 ?defensive_check@@YAHH@Z 0x00560D30-0x00560D4D BYTE_EXACT
// size      29 bytes
// prototype int (__cdecl ?defensive_check@@YAHH@Z)(int baseID)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00560B30
Return Value: Amount of defensive units needed (1-8)
Status: Complete
*/
int __cdecl defensive_check(int base_id) {
    int defenses = garrison_check(base_id);
    if (defenses > 5) {
        defenses--;
    }
    if (defenses > 2) {
        defenses--;
    }
    return defenses;
}

/*
Purpose: Determine the ideal unit count to protect a faction's bases in the specified land region.
// ORIGINAL: 0x00560D50 ?guard_check@@YAHHH@Z 0x00560D50-0x00560DC3 BYTE_EXACT
// LEVER: `plan_region` as `uint32_t` forced the image's byte load into a
//        zero-extend (`xor edx,edx; mov dl,...`) followed by 32-bit `cmp edx,N`;
//        the field is `uint8_t region_base_plan[128]`, and typing the local
//        `uint8_t` drops the `xor` and compares `dl` directly, matching the
//        image byte-for-byte. Also needed `--flags` with `/Oy-` to see the kept
//        frame pointer - the tool's own default flag set omits it.
// size      115 bytes
// prototype int (__cdecl ?guard_check@@YAHHH@Z)(int factionID, int region)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Amount of non-offensive units needed to guard region
Status: Complete
*/
int __cdecl guard_check(int faction_id, int region) {
    if (region >= MaxRegionLandNum) {
        return 0;
    }
    int factor = 2 - PlayersData[faction_id].ai_fight;
    uint8_t plan_region = PlayersData[faction_id].region_base_plan[region];
    if (plan_region == PLAN_COLONIZATION) {
        factor += 2;
    } else if (plan_region == PLAN_DEFENSIVE) {
        factor = 1; // 1-1 unit per base ratio
    }
    if (PlayersData[faction_id].flags & PFLAG_STRAT_DEF_OBJECTIVES) {
        factor = 1; // 1-1 unit per base ratio
    }
    return (PlayersData[faction_id].region_total_bases[region] + factor - 1) / factor;
}

/*
Purpose: Calculate specified faction's best available weapon and armor ratings as well as the
         fastest moving ground Veh chassis. Compare these capabilities to faction's best opponent
         capabilities based on current diplomacy.
// ORIGINAL: 0x00560DD0 ?enemy_capabilities@@YAXH@Z 0x00560DD0-0x00561076 BYTE_EXACT
// LEVER: call_diff showed this tree FEWER (8 vs 9) - the image calls
//        veh_avail(BSC_MIND_WORMS,...) TWICE, once for the best_psi_offense
//        ternary and again for best_psi_defense, rather than caching the
//        result in one `has_worms` local and reusing it. Un-cached both call
//        sites to match. Also every "keep the running max" comparison in the
//        function compiles non-strict in the image (`jl`/`jg` skip-the-store,
//        i.e. store on >=/<=) where this tree had strict `>`/`<`: fixed
//        weap_val/best_psi_offense, weap_val/best_weapon_value, arm_val/
//        best_psi_defense, arm_val/best_armor_value, Chassis[i].speed/
//        best_land_speed, and all five enemy_best_X vs PlayersData[j].best_X
//        comparisons in the closing loop (storing the same value back on a
//        tie is behaviourally inert, so this only affects codegen). Moved
//        0.430 -> 0.606 similar at the winning flag set. Remaining
//        divergence is the closing loop's has_treaty(i,j,*) reuse: the image
//        loads `PlayersData[i].diplo_treaties[j]` ONCE and reuses the byte
//        for all four DTREATY_* bit tests plus a `+=4`-per-j pointer walk;
//        this tree's O2 recomputes the address per has_treaty() call site -
//        register/CSE allocation, not chased per the no-register-only rule.
// LEVER (2026-09-03): both of those "register/CSE" residuals were the two
//        inlined helpers, and both closed. 18/210 (0.711 at the recorded
//        /O1 set) -> 133/210 at the DEFAULT /O2 /Gy /GR- /Oy- /GX:
//        (1) has_tech_call in all three loops - the image CALLS 0x005B9F20
//        three times, and technology.h's MEASURED inline was open-coded at
//        every site (measured 133/210 under /Ob0 /O2 BEFORE the switch,
//        which is what named the inline as the defect); (2) the closing
//        loop's four has_treaty() calls are now ONE read of
//        `PlayersData[i].diplo_treaties[j]` into a local with the DTREATY_*
//        tests applied to it, which is exactly what the image's single load
//        and byte reuse is. First divergence moved from instruction 4 to
//        126. Remaining: the closing loop's register roles and the
//        `+=4`-per-j pointer walk.
// size      678 bytes
// prototype void (__cdecl ?enemy_capabilities@@YAXH@Z)(int factionID)
// callers   3   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D270 0x0057D2E0 0x005B9F20 0x005BA910
Return Value: n/a
Status: Complete - testing / WIP
*/
void __cdecl enemy_capabilities(int faction_id) {
    PlayersData[faction_id].best_psi_offense = veh_avail(BSC_MIND_WORMS, faction_id, -1)
        ? weap_strat(WPN_PSI_ATTACK, faction_id) : 0;
    PlayersData[faction_id].best_weapon_value = 1;
    // has_tech_call, not has_tech, in all three loops: the image CALLS
    // 0x005B9F20 at 0x00560E2F / 0x00560EC1 / 0x00560F20 rather than
    // open-coding the preq walk, so /Ob0 (which stops technology.h's
    // MEASURED inline) agrees with the image on 133 of 210 instructions
    // where plain /O2 manages 33.
    for (int i = 0; i < MaxWeaponNum; i++) {
        if (has_tech(Weapon[i].preq_tech, faction_id) && Weapon[i].offense_rating < 99) {
            int weap_val = weap_strat(i, faction_id);
            if (Weapon[i].offense_rating < 0 && weap_val
                >= PlayersData[faction_id].best_psi_offense) {
                PlayersData[faction_id].best_psi_offense = weap_val;
            }
            if (weap_val >= PlayersData[faction_id].best_weapon_value) {
                PlayersData[faction_id].best_weapon_value = weap_val;
            }
        }
    }
    PlayersData[faction_id].best_psi_defense = veh_avail(BSC_MIND_WORMS, faction_id, -1)
        ? arm_strat(ARM_PSI_DEFENSE, faction_id) : 0;
    PlayersData[faction_id].best_armor_value = 1;
    for (i = 0; i < MaxArmorNum; i++) {
        if (has_tech(Armor[i].preq_tech, faction_id)) {
            int arm_val = arm_strat(i, faction_id);
            if (Armor[i].defense_rating < 0 && arm_val >= PlayersData[faction_id].best_psi_defense) {
                PlayersData[faction_id].best_psi_defense = arm_val;
            }
            if (arm_val >= PlayersData[faction_id].best_armor_value) {
                PlayersData[faction_id].best_armor_value = arm_val;
            }
        }
    }
    PlayersData[faction_id].best_land_speed = 1;
    for (i = 0; i < MaxChassisNum; i++) {
        if (has_tech(Chassis[i].preq_tech, faction_id) && Chassis[i].triad == TRIAD_LAND) {
            if (Chassis[i].speed >= PlayersData[faction_id].best_land_speed) {
                PlayersData[faction_id].best_land_speed = Chassis[i].speed;
            }
        }
    }
    PlayersData[faction_id].enemy_best_land_speed = 0;
    PlayersData[faction_id].enemy_best_armor_value = 0;
    PlayersData[faction_id].enemy_best_weapon_value = 0;
    PlayersData[faction_id].enemy_best_psi_defense = 0;
    PlayersData[faction_id].enemy_best_psi_offense = 0;
    for (int pass = 0; pass < 4 && !PlayersData[faction_id].enemy_best_weapon_value; pass++) {
        // 1st pass: vendetta, no treaty, has commlink
        // 2nd pass: no treaty, has commlink
        // 3rd pass: has commlink
        // 4th pass: any non-pact faction
        for (int j = 1; j < MaxPlayerNum; j++) {
            if (j == faction_id) continue;
            // Read the faction_id's treaty word once, then apply each pass filter.
            uint32_t treaties = PlayersData[faction_id].diplo_treaties[j];
            if (treaties & DTREATY_PACT) continue;
            if (pass == 0 && (treaties & DTREATY_VENDETTA) == 0) continue;
            if (pass <= 1 && (treaties & DTREATY_TREATY) != 0) continue;
            if (pass <= 2 && (treaties & DTREATY_COMMLINK) == 0) continue;
            {
                int best = PlayersData[faction_id].enemy_best_weapon_value;
                const int other = PlayersData[j].best_weapon_value;
                if (best <= other) best = other;
                PlayersData[faction_id].enemy_best_weapon_value = best;
            }
            {
                int best = PlayersData[faction_id].enemy_best_armor_value;
                const int other = PlayersData[j].best_armor_value;
                if (best <= other) best = other;
                PlayersData[faction_id].enemy_best_armor_value = best;
            }
            {
                int best = PlayersData[faction_id].enemy_best_land_speed;
                const int other = PlayersData[j].best_land_speed;
                if (best <= other) best = other;
                PlayersData[faction_id].enemy_best_land_speed = best;
            }
            {
                int best = PlayersData[faction_id].enemy_best_psi_offense;
                const int other = PlayersData[j].best_psi_offense;
                if (best <= other) best = other;
                PlayersData[faction_id].enemy_best_psi_offense = best;
            }
            {
                int best = PlayersData[faction_id].enemy_best_psi_defense;
                const int other = PlayersData[j].best_psi_defense;
                if (best <= other) best = other;
                PlayersData[faction_id].enemy_best_psi_defense = best;
            }
        }
    }
}

// ORIGINAL: 0x00561080 ?enemy_strategy@@YAXH@Z 0x00561080-0x00564888
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x00561080..0x00564888 instructions drive genuine PlayersData, Base, Veh, Map, Goal and Continent accesses; preserve original arithmetic, branch ordering and state mutations.", "The entry reset does not clear region_total_offensive_vehs; the later region 1..127 clear leaves index zero intact. Preserve the original prior-value behavior.", "Historical seed caution about scaled pointer indices remains above; former UNBLOCKER: cross-check indexed/scaled fields against pinned address and relocation evidence, not decompiler index arithmetic. No exactness or full-game acceptance claim."], "reason": "Complete typed faction strategy owner; source separation does not assert an original translation-unit boundary."}
// DEFERRED: this is the AI military-planning engine (single `int factionID` param), not a UI or I/O function - 4239 instructions, 1875 Ghidra lines, frame 0x288 bytes (moderate: real locals, no giant embedded C++ object graph the way the Popup/Dialogs-heavy addresses in this batch have). Its call list is the opposite shape from goody_box/game_data: LOW repetition, many DISTINCT one- or two-off calls (?stack_check@@ 17x is the only high-frequency one; ?add_goal@@ 7x; then base_at/veh_at/weap_val/veh_cargo/is_coast/ has_abil/garrison_check/zoc_sea/weap_strat/wants_to_attack/ veh_kill/terraform_cost/stack_fix/planet_buster/kill/contiguous/ bitmask each 1-5x, plus 20x _abs). That is the same risk profile as tech_achieved (0x005BB000, deferred earlier in this batch for a confirmed hand-arithmetic error on scaled-pointer-index field offsets) rather than game_data's or goody_box's - bespoke per-site reasoning, not a mechanically repeatable pattern, so the same caution applies: a first hand pass is exactly where a wrong PlayersData/base/veh field offset would compile clean and be wrong.
void __cdecl enemy_strategy(int faction_id) {
    PlayerData &faction_data = PlayersData[faction_id];

    // 0x00561080..0x00561152: reset the faction's turn-local totals and
    // indexed planning state before scanning the live bases and vehicles.
    faction_data.total_mil_veh = 0;
    faction_data.current_num_bases = 0;
    faction_data.mil_strength_1 = 0;
    faction_data.mil_strength_2 = 0;
    faction_data.pop_total = 0;
    faction_data.unk_70 = 0;
    faction_data.planet_busters = 0;
    faction_data.unk_71 = 0;
    faction_data.unk_72 = 0;

    const int turn_faction = GameDataStateStorage.fields_.turn_current_num_ + faction_id;
    const uint16_t retained_region_planning_bits =
        ((turn_faction & 0x0F) != 0) ? 0x0008 : 0;
    for (int reset_region = 0; reset_region < 128; ++reset_region) {
        faction_data.region_total_combat_vehs[reset_region] = 0;
        faction_data.region_total_bases[reset_region] = 0;
        faction_data.region_force_rating[reset_region] = 0;
        faction_data.unk_77[reset_region] &= retained_region_planning_bits;
    }
    for (int proto_id = 0; proto_id < 512; ++proto_id) {
        faction_data.proto_id_active[proto_id] = 0;
        faction_data.proto_id_queue[proto_id] = 0;
    }

    const uint32_t faction_bit = 1u << (static_cast<uint32_t>(faction_id) & 31u);

    // The 0x561168 walk treats foreign bases first, adding colonize/scout
    // opportunities for unclaimed or fungus locations and an attack goal for
    // visible bases selected by the current diplomacy predicates. The
    // 0x561459 edge is the distinct own-base update path.
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        Base &base = Bases[base_id];
        const int x = base.x;
        const int y = base.y;
        const int owner = base.faction_id_current;

        if (owner != faction_id) {
            const uint8_t visibility_bit = static_cast<uint8_t>(faction_bit);
            if ((base.visibility & visibility_bit) == 0) continue;

            const uint32_t relationship = faction_data.diplo_treaties[owner];
            if ((relationship & 0x0C10u) == 0
                || (relationship & 0x0001u) != 0) continue;

            int attack_priority = 1;
            const Map *tile = map_loc(x, y);
            if ((tile->bit & 0x00000002u) == 0
                || (tile->val2 & 0x0Fu) >= 8) {
                add_goal(faction_id, AI_GOAL_DEFEND, 2, x, y, -1);
                add_goal(faction_id, AI_GOAL_SCOUT, 2, x, y, -1);
            }

            const int region = tile->region;
            if (base.faction_id_former == faction_id) {
                ++attack_priority;
            }
            if (faction_data.region_total_bases[region] != 0) {
                ++attack_priority;
            }

            int facility_offset = 0;
            int facility_mask = 0;
            bitmask(4, &facility_offset, &facility_mask);
            if ((base.facilities_built[facility_offset] & facility_mask) == 0) {
                ++attack_priority;
            }
            if (base.queue_production_id[0] == -102) {
                attack_priority *= 4;
            }

            bitmask(1, &facility_offset, &facility_mask);
            if ((base.facilities_built[facility_offset] & facility_mask) != 0) {
                ++attack_priority;
                if ((faction_data.flags & 0x00200000u) != 0) {
                    attack_priority *= 2;
                }
                if (GameDataStateStorage.fields_.turn_current_num_ < PlayersData[owner].corner_market_turn) {
                    attack_priority <<= 3;
                }
            }

            if (faction_data.base_id_atk_target != base_id
                || (faction_data.diplo_treaties[owner] & 0x10u) == 0) {
                if (is_objective(base_id)) {
                    attack_priority = 4;
                }
            } else {
                attack_priority <<= 2;
            }
            add_goal(faction_id, AI_GOAL_ATTACK, attack_priority, x, y, -1);

            const int target_x = base.unk_x;
            const int target_y = base.unk_y;
            if (target_y >= 0 && target_y < MapLatitudeBounds
                && target_x >= 0 && target_x < MapLongitudeBounds) {
                const int target_base = base_find(target_x, target_y);
                if (target_base >= 0
                    && Bases[target_base].faction_id_current == owner
                    && terraform_cost(target_x, target_y, faction_id)
                        <= ((faction_data.energy_reserves
                            + (faction_data.energy_reserves >> 31 & 7)) >> 3)) {
                    add_goal(faction_id, 41, 2, target_x, target_y, -1);
                }
            }
            continue;
        }

        base.state &= 0xF3FFFFFFu;
        if (base.nerve_staple_turns_left != 0) {
            base.nerve_staple_turns_left = static_cast<uint8_t>(
                base.nerve_staple_turns_left - 1u);
        }
        ++faction_data.current_num_bases;
        faction_data.pop_total += static_cast<int>(base.population_size);

        const int stack = stack_fix(veh_at(x, y));
        if (!stack_check(stack, 0x0E, faction_id, -1, -1)
            && !stack_check(stack, 2, 3, faction_id, -1)) {
            add_goal(faction_id, AI_GOAL_DEFEND, 4, x, y, -1);
        }

        for (int queue_index = 0; queue_index <= base.queue_size; ++queue_index) {
            const int queued_id = base.queue_production_id[queue_index];
            if (queued_id >= 0) {
                ++faction_data.proto_id_queue[queued_id];
            }
        }

        const int tile_region = map_loc(x, y)->region;
        ++faction_data.region_total_bases[tile_region];
        if (is_coast(x, y, 0)) {
            faction_data.unk_77[tile_region] |= 0x0080;
        }
        if (((GameDataStateStorage.fields_.turn_current_num_ + base_id) & 3) == 0) {
            base.event &= 0xFFFFFF00u;
        }
    }

    // 0x005615D9..0x00561622: reset flagged bases for factions whose turn is
    // not locked to a human, preserving the native sentinel test in queue[0].
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0) {
        for (int reset_base_id = 0; reset_base_id < GameDataStateStorage.fields_.base_current_count_;
             ++reset_base_id) {
            Base &reset_base = Bases[reset_base_id];
            if (reset_base.faction_id_current == faction_id
                && reset_base.queue_production_id[0] == -69) {
                base_reset(reset_base_id, 0);
            }
        }
    }

    int region_plan_count[128];
    for (int region_index = 0; region_index < 128; ++region_index) {
        region_plan_count[region_index] = 0;
    }

    enemy_capabilities(faction_id);
    faction_data.unk_64 = 0;
    faction_data.unk_65 = 0;
    faction_data.unk_66 = 0;
    faction_data.unk_68 = 0;
    faction_data.unk_105 = 0;
    faction_data.unk_106 = 0;
    int carrier_count = 0;
    int maximum_carrier_cargo = 2;
    int invalid_target_vehicle_count = 0;

    for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle_id >= 0; --vehicle_id) {
        Veh &vehicle = Vehs[vehicle_id];
        const int prototype_id = vehicle.proto_id;
        const VehPrototype &prototype = VehPrototypes[prototype_id];
        const int unit_owner = vehicle.faction_id;

        if (unit_owner != faction_id) {
            const uint32_t relationship = faction_data.diplo_treaties[unit_owner];
            if ((relationship & 0x8u) != 0
                && ((relationship & 0x3u) == 0 || (relationship & 0x0C20u) != 0)) {
                if (prototype.plan == 7) {
                    ++faction_data.unk_64;
                }
                if (prototype.plan == 6) {
                    faction_data.unk_65 += prototype.carry_capacity;
                }

                const int chassis_id = prototype.chassis_id;
                const RulesChassis &chassis = Chassis[chassis_id];
                if (chassis.triad == TRIAD_SEA) {
                    if (prototype.plan == 0 && chassis.missile == 0
                        && weap_val(prototype_id, faction_id)
                            >= faction_data.best_weapon_value) {
                        ++faction_data.unk_66;
                    }
                    if (chassis.missile != 0 && planet_buster(vehicle_id) == 0) {
                        ++faction_data.unk_68;
                    }
                }

                // Native 0x56178D..0x561863 adds the special goal only for
                // plan-9 sea vehicles. It operates on ocean tiles and rejects
                // a base-in-tile whose owner nibble is below eight. The
                // visibility byte is tested against this actor's bit only on
                // the two native move-marker/state paths that reach it.
                if (prototype.plan == 9 && chassis.triad == TRIAD_SEA) {
                    const Map *tile = map_loc(vehicle.x, vehicle.y);
                    const bool ocean_tile = (tile->climate & 0xE0u) < 0x60u;
                    const bool owned_base_tile_rejected =
                        (tile->bit & BIT_BASE_IN_TILE) != 0
                        && (tile->val2 & 0x0Fu) < 8;
                    const bool bypass_visibility =
                        vehicle.move_to_ai_type == faction_id
                        && (vehicle.state & 0x00000440u) != 0x00000440u;
                    const bool visible_to_actor =
                        (vehicle.visibility
                         & static_cast<uint8_t>(faction_bit)) != 0;
                    if (ocean_tile && !owned_base_tile_rejected
                        && (bypass_visibility || visible_to_actor)) {
                        const int priority =
                            (faction_data.flags & 0x00080000u) != 0 ? 4 : 2;
                        add_goal(faction_id, 6, priority,
                                 vehicle.x, vehicle.y, -1);
                    }
                }
            }
            continue;
        }

        ++faction_data.proto_id_active[prototype_id];
        if (vehicle.year_end_lurking != 0) {
            --vehicle.year_end_lurking;
            if (vehicle.year_end_lurking == 0) {
                vehicle.flags &= 0xFBBFu;
            }
        }

        int upgrade_result = 1;
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0
            && (prototype.obsolete_factions & static_cast<int8_t>(faction_bit)) != 0) {
            // A successful upgrade skips only the obsolete-design subpath;
            // the native body still processes this vehicle's current state.
            upgrade_result = ConsoleGlobal.upgrade(vehicle_id);
        }

        // 0x0056193F..0x00561C41: stale-design and sea-zone checks. The
        // native code rereads the vehicle after upgrade(), which may replace
        // its design or otherwise alter the record.
        Veh &current_vehicle = Vehs[vehicle_id];
        const int current_prototype_id = current_vehicle.proto_id;
        const VehPrototype &current_prototype = VehPrototypes[current_prototype_id];
        const int current_chassis_id = current_prototype.chassis_id;
        const RulesChassis &current_chassis = Chassis[current_chassis_id];
        const int current_x = current_vehicle.x;
        const int current_y = current_vehicle.y;
        const Map *current_tile = map_loc(current_x, current_y);
            int current_region = current_tile->region;
        const int current_triad = current_chassis.triad;
        const int current_plan = current_prototype.plan;
        // Native 0x561D01 masks the altitude class and compares it with the
        // shore-line value. In the source map encoding this predicate means
        // ocean (altitude class below shore), not land.
        const bool is_ocean_tile = (current_tile->climate & 0xE0u) < 0x60u;

        if (upgrade_result == 0
            && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0
            && ((GameDataStateStorage.fields_.turn_current_num_ + vehicle_id) & 3) != 0
            && GameDataStateStorage.fields_.turn_current_num_ >= 0x1A
            && current_triad == TRIAD_LAND
            && (current_vehicle.move_to_ai_type != 'F'
                && current_vehicle.move_to_ai_type != '1'
                && current_vehicle.move_to_ai_type != '2')
            && current_plan > 0 && current_plan <= 7
            && (current_plan != 1
                || faction_data.region_base_plan[current_region] > 3)
            && (current_plan == faction_data.region_base_plan[current_region]
                || (current_plan != 3
                    || faction_data.region_base_plan[current_region] != 8
                    || current_vehicle.order == 2))) {
            const bool shallow_fungus = (current_tile->bit & 1u) != 0
                && (current_tile->climate & 0x0Fu) < 8;
            if ((!shallow_fungus
                 || stack_check(vehicle_id, 0x0E, faction_id, -1, -1) > 1)
                && (current_tile->climate & 0xE0u) >= 0x60u
                && faction_data.region_base_plan[current_region] != 2
                && zoc_sea(current_x, current_y, faction_id) == 0
                && current_vehicle.move_to_ai_type != '1') {
                bool outclassed_by_available_design = false;
                for (int candidate_proto_id = faction_id * 64;
                     candidate_proto_id < faction_id * 64 + 64;
                     ++candidate_proto_id) {
                    const VehPrototype &candidate = VehPrototypes[candidate_proto_id];
                    if ((candidate.flags & 1u) == 0
                        || (candidate.obsolete_factions & faction_bit) != 0
                        || candidate.plan != current_plan
                        || Chassis[candidate.chassis_id].triad != current_triad) {
                        continue;
                    }
                    if (current_triad <= TRIAD_SEA) {
                        if (arm_val(candidate_proto_id, faction_id)
                            > arm_val(current_prototype_id, faction_id)) {
                            outclassed_by_available_design = true;
                            break;
                        }
                    } else if (weap_val(candidate_proto_id, faction_id)
                               > weap_val(current_prototype_id, faction_id)) {
                        outclassed_by_available_design = true;
                        break;
                    }
                }
                if (!outclassed_by_available_design
                    && (current_vehicle.state & 0x20u) == 0) {
                    kill(vehicle_id);
                    continue;
                }
            }
        }

        current_vehicle.state &= 0xCFFFFFDBu;

        // 0x561D5E..0x561DF4: order 1 is cleared unless this is a land-triad
        // unit and the current tile's native shore-line predicate is ocean.
        // The later ocean/land branches also run when the lock mask bypasses
        // this order-byte update.
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0 && current_vehicle.order == 1) {
            if (current_triad != TRIAD_LAND || !is_ocean_tile) {
                current_vehicle.order = 0;
            }
        }

        if (is_ocean_tile && current_triad == TRIAD_LAND) {
            const bool shallow_fungus = (current_tile->bit & 1u) != 0
                && (current_tile->climate & 0x0Fu) < 8;
            if (!shallow_fungus) {
                const int stack_result =
                    stack_check(vehicle_id, 2, 7, -1, -1);
                if (stack_result == 0) {
                    if ((current_vehicle.flags & 0x20u) == 0) {
                        veh_kill(vehicle_id);
                        continue;
                    }
                } else if (current_vehicle.order != 1) {
                    sleep(vehicle_id);
                }
            }
        } else if (!is_ocean_tile && current_triad == TRIAD_SEA) {
            const bool shallow_fungus = (current_tile->bit & 1u) != 0
                && (current_tile->climate & 0x0Fu) < 8;
            if (!shallow_fungus && (current_vehicle.flags & 0x20u) == 0) {
                veh_kill(vehicle_id);
                continue;
            }
        }

        // Native 0x561E65..0x561F2E applies the garrison check only to
        // prototype plans 0/1 and when state bit 0x40000 is clear. The home
        // base fallback is used only for the native 'F' move marker.
        if (current_plan <= 1 && (current_vehicle.state & 0x00040000u) == 0) {
            int selected_base = base_at(current_x, current_y);
            if (selected_base < 0 && current_vehicle.move_to_ai_type == 'F') {
                selected_base = current_vehicle.home_base_id;
            }
            if (selected_base >= 0) {
                const int original_garrison = garrison_check(selected_base);
                int capped_garrison = garrison_check(selected_base);
                if (capped_garrison >= 6) --capped_garrison;
                if (capped_garrison >= 3) --capped_garrison;
                const Base &garrison_base = Bases[selected_base];
                const int base_stack = veh_at(garrison_base.x, garrison_base.y);
                const int protected_stack = stack_check(
                    base_stack, 0x0B, 0x10000000, 0x10000000, faction_id);
                if (protected_stack + capped_garrison < original_garrison) {
                    current_vehicle.state |= 0x10000000u;
                }
            }
        }

        // Plan-specific per-vehicle summaries and owner-side cleanup.
        if (current_vehicle.move_to_ai_type != 'F') {
            if (current_plan == 2 && current_vehicle.home_base_id >= 0) {
                const Base &home = Bases[current_vehicle.home_base_id];
                if (home.x != current_x || home.y != current_y) {
                    Bases[current_vehicle.home_base_id].state |= 0x04000000u;
                }
            } else if (current_plan == 3) {
                const bool is_shallow_ocean = (current_tile->bit & 1u) != 0
                    && (current_tile->climate & 0x0Fu) < 8;
                if (!is_shallow_ocean) {
                    faction_data.unk_77[current_region] |= 0x20u;
                }
            } else if (current_plan == 8) {
                uint16_t &region_flags = faction_data.unk_77[current_region];
                region_flags |= (region_flags & 2u) != 0 ? 0x0800u : 2u;
            } else if (current_plan == 9) {
                current_vehicle.probe_action &= 0xF7u;
                ++region_plan_count[current_region];
                const int home_base_id = current_vehicle.home_base_id;
                // Native uses one of two discrete state masks here. The
                // original selection is from the current chassis triad:
                // sea uses 0x80000; land/air use 0x20.
                const uint32_t relocation_base_mask =
                    current_triad == TRIAD_SEA ? 0x00080000u : 0x00000020u;
                if (home_base_id >= 0
                    && (Bases[home_base_id].state & relocation_base_mask) != 0
                    && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0) {
                    int nearest_base_id = -1;
                    int nearest_distance = 9999;
                    for (int candidate_id = 0;
                         candidate_id < GameDataStateStorage.fields_.base_current_count_; ++candidate_id) {
                        if (candidate_id == home_base_id) continue;
                        const Base &candidate = Bases[candidate_id];
                        if (map_loc(candidate.x, candidate.y)->region != current_region
                            || (candidate.state & relocation_base_mask) != 0) continue;
                        // Native reads the unaligned dword at Base+0xD2.
                        // Express the same little-endian word from the two
                        // typed adjacent source members rather than an offset
                        // alias: high 16 bits of nutrient_surplus, then low
                        // 16 bits of mineral_surplus.
                        const uint32_t relocation_value_bits =
                            (static_cast<uint32_t>(candidate.nutrient_surplus) >> 16)
                            | (static_cast<uint32_t>(candidate.mineral_surplus) << 16);
                        int32_t relocation_value;
                        memcpy(&relocation_value, &relocation_value_bits,
                               sizeof(relocation_value));
                        if (relocation_value >= 2) continue;
                        const int distance = vector_dist(
                            x_dist(current_x, candidate.x),
                            current_y - candidate.y);
                        if (distance < nearest_distance) {
                            nearest_distance = distance;
                            nearest_base_id = candidate_id;
                        }
                    }
                    if (nearest_base_id >= 0) {
                        Bases[nearest_base_id].state |= relocation_base_mask;
                        current_vehicle.home_base_id =
                            static_cast<int16_t>(nearest_base_id);
                    }
                }
            } else if (current_plan == 0x0B) {
                faction_data.unk_77[current_region] |= 0x10u;
            }
        }

        // The native common-count block at 0x56226B..0x562284 is reached only
        // for plans 0..7; plan 8/9/11 routes leave via 0x56216D first.
        if (current_plan <= 7) {
            ++faction_data.total_mil_veh;
            // The local native altitude predicate is an ocean test. Native
            // counts this land-triad vehicle when that predicate is false.
            if (current_region >= 0 && current_triad == TRIAD_LAND
                && !is_ocean_tile) {
                ++faction_data.region_total_combat_vehs[current_region];
            }
        }
        if (current_triad == TRIAD_SEA) {
            ++faction_data.unk_70;
        } else if (current_triad == TRIAD_AIR) {
            if (current_plan == 0) ++faction_data.unk_105;
            else if (current_plan == 4) ++faction_data.unk_106;
        }

        if (current_plan == 7) {
            ++carrier_count;
            const int cargo_count = veh_cargo(vehicle_id);
            if (cargo_count > maximum_carrier_cargo) {
                maximum_carrier_cargo = cargo_count;
            }
        }

        // Native 0x562326 tests planet_buster first. A buster increments its
        // counter and jumps straight to 0x5624BC, skipping strategy scoring
        // AND the subsequent 0x562405 target-state accounting. Keep those
        // statements on the non-buster edge: zero-filling the native skipped
        // stack locals would invent target-state behavior for busters.
        if (planet_buster(vehicle_id) != 0) {
            ++faction_data.planet_busters;
        } else {
            const int weapon_strategy = weap_strat(
                current_prototype.weapon_id, faction_id);
            const int armor_strategy = arm_strat(
                current_prototype.armor_id, faction_id);
            const int morale = morale_veh(vehicle_id, 0, 0) + 6;
            const int force = (weapon_strategy + armor_strategy) * morale;
            faction_data.mil_strength_1 +=
                (force + (force >> 31 & 7)) >> 3;
            if (armor_strategy < weapon_strategy || current_vehicle.order != 2) {
                const int offense = morale * weapon_strategy;
                const int rounded_offense =
                    (offense + (offense >> 31 & 7)) >> 3;
                faction_data.mil_strength_2 += rounded_offense;
                faction_data.proto_id_lost[current_prototype_id] =
                    static_cast<int16_t>(faction_data.proto_id_lost[current_prototype_id]
                                         + rounded_offense);
                if (current_region >= 0 && current_region < 128) {
                    // Native 0x5623FA performs a word add at
                    // PlayerData+0x127C + region*2: region_force_rating.
                    faction_data.region_force_rating[current_region] =
                        static_cast<uint16_t>(faction_data.region_force_rating[current_region]
                                              + rounded_offense);
                }
            }
            // Native 0x562405..0x5624BC accounts for vehicles carrying one
            // of the target-assignment state combinations. These are
            // PlayerData.x_target/y_target, distinct from base_id_atk_target.
            const uint32_t target_state = current_vehicle.state & 0x00060000u;
            if (target_state == 0x00040000u) {
                const int target_x = faction_data.x_target;
                const int target_y = faction_data.y_target;
                bool target_is_in_vehicle_region = false;
                if (target_x >= 0 && target_x < MapLongitudeBounds
                    && target_y >= 0 && target_y < MapLatitudeBounds) {
                    target_is_in_vehicle_region =
                        map_loc(target_x, target_y)->region == current_region;
                }
                if (!target_is_in_vehicle_region) {
                    current_vehicle.state |= 0x00020000u;
                } else {
                    ++faction_data.unk_71;
                    if (current_plan == 0x0B) {
                        faction_data.unk_72 |= 2;
                    } else if (armor_strategy > weapon_strategy) {
                        faction_data.unk_72 |= 1;
                    }
                }
            } else if (target_state == 0x00060000u) {
                ++invalid_target_vehicle_count;
            }
            // The private actual-body harness snapshots this point to
            // distinguish native buster skips from ordinary vehicles.
            // ENEMY_STRATEGY_TARGET_STATE_TEST_TAP
        }
    }

    // Native consumes this local count in the region planner at 0x563557.
    (void)invalid_target_vehicle_count;

    faction_data.unk_70 += ((maximum_carrier_cargo * 0x800 + carrier_count) << 11);

    int strongest_unpacted_enemy_navy = 0;
    for (int other = 1; other < MaxPlayerNum; ++other) {
        if (other == faction_id) continue;
        const uint32_t relationship = faction_data.diplo_treaties[other];
        if ((relationship & 0x0008u) == 0 || (relationship & 0x0001u) != 0) {
            continue;
        }
        const int naval_total = PlayersData[other].unk_70;
        const int scaled_total = (naval_total
            + (naval_total >> 31 & 0x003FFFFF)) >> 22;
        if (scaled_total > strongest_unpacted_enemy_navy) {
            strongest_unpacted_enemy_navy = scaled_total;
        }
    }
    faction_data.unk_64 += strongest_unpacted_enemy_navy * 0x800;

    if (carrier_count >= 2) {
        for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
            Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id != faction_id) continue;
            const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
            if (Chassis[prototype.chassis_id].triad == TRIAD_SEA
                && prototype.plan == 7
                && (vehicle.proto_id == 13
                    || faction_data.proto_id_active[vehicle.proto_id] >= 2)) {
                vehicle.state |= 0x20000000u;
                break;
            }
        }
    }

    for (int coastal_vehicle_id = 0; coastal_vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++coastal_vehicle_id) {
        Veh &vehicle = Vehs[coastal_vehicle_id];
        if (vehicle.faction_id != faction_id
            || VehPrototypes[vehicle.proto_id].plan != 9) continue;
        const int region = map_loc(vehicle.x, vehicle.y)->region;
        if (region <= 0x3F && region_plan_count[region] >= 3
            && faction_data.region_total_bases[region] >= 3) {
            // Native 0x562663 ORs 0x0800 into the word immediately before
            // Veh.proto_id (Veh.flags at +0x30); this is not probe_action.
            vehicle.flags |= 0x0800u;
            if (vehicle.home_base_id >= 0) {
                Bases[vehicle.home_base_id].state &= ~0x20u;
            }
            region_plan_count[region] = 0;
        }
    }

    // Native 0x56269A..0x56294A retains or invalidates the prior movement
    // target before region planning. The decision uses the saved x/y target,
    // the stack actually at that tile, and this faction's target-region
    // counters; it is separate from base_id_atk_target.
    const uint32_t human_turn_bit =
        static_cast<uint32_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & 0xFFu & faction_bit;
    bool reset_target = false;
    int old_attack_target_region = 0;
    if (faction_data.y_target >= 0) {
        const int target_x = faction_data.x_target;
        const int target_y = faction_data.y_target;
        const Map *target_tile = map_loc(target_x, target_y);
        const int target_region = target_tile->region;
        old_attack_target_region = target_region;
        const int target_base_id = base_at(target_x, target_y);
        if (target_base_id >= 0
            && Bases[target_base_id].faction_id_current == faction_id) {
            const Base &target_base = Bases[target_base_id];
            const int target_stack = stack_fix(veh_at(target_base.x, target_base.y));
            if (target_stack >= 0) {
                const int protected_target_units = stack_check(
                    target_stack, 0x0B, 0x00060000, 0x00040000, -1);
                int required_target_units =
                    static_cast<int>(faction_data.region_total_bases[target_region]) * 2 / 3;
                if (required_target_units < 3) required_target_units = 3;
                else if (required_target_units > 8) required_target_units = 8;
                if (required_target_units
                    <= protected_target_units - bit_count(faction_data.unk_72)) {
                    reset_target = true;
                }

                const uint8_t target_region_plan =
                    faction_data.region_base_plan[target_region];
                if (target_region_plan == 2
                    || (faction_data.unk_77[target_region] & 0x4200u) != 0) {
                    reset_target = true;
                }
                if (target_region_plan > 2
                    && stack_check(target_stack, 2, 7, -1, -1) != 0) {
                    const int naval_requirement =
                        ((faction_data.unk_70
                          + (faction_data.unk_70 >> 31 & 0x003FFFFF)) >> 22) * 2;
                    if (protected_target_units >= naval_requirement) {
                        reset_target = true;
                    }
                    const int target_sea_units =
                        stack_check(target_stack, 2, 7, -1, -1);
                    if (target_sea_units > 1) {
                        const int air_units = stack_check(target_stack, 8, -1, -1, -1);
                        const int non_air_units = stack_check(target_stack, 3, 0, -1, -1);
                        if (air_units + non_air_units <= protected_target_units) {
                            reset_target = true;
                        }
                    }
                }
            }
        }

        // The two target-region branches at 0x5628D6 and 0x5628E5 jump
        // directly to region planning and deliberately bypass cleanup.
        if (human_turn_bit != 0 || reset_target) {
            reset_target = true;
        } else {
            const uint8_t target_region_plan =
                faction_data.region_base_plan[target_region];
            if (target_region_plan <= 2) {
                GameDataStateStorage.fields_.game_state_ &= ~0x00000800u;
            } else if (target_region_plan != 8
                       && is_coast(target_x, target_y, 0)) {
                GameDataStateStorage.fields_.game_state_ |= 0x00000800u;
            } else {
                reset_target = true;
            }
        }
    } else if (human_turn_bit != 0) {
        reset_target = true;
    }

    if (reset_target) {
        for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
            Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id == faction_id
                && (vehicle.state & 0x00040000u) != 0) {
                vehicle.state |= 0x00020000u;
            }
        }
        faction_data.x_target = -1;
        faction_data.y_target = -1;
        faction_data.unk_71 = 0;
        faction_data.unk_72 = 0;
    }

    // Native 0x56294B..0x563A63 plans regions. At 0x562963 the byte at
    // PlayerData+0x11FC (the per-region byte cleared here) is zeroed. The
    // short-circuit at 0x562973..0x562992 assigns plan 8 and jumps to 0x5634C5
    // (the loop increment) for regions whose low six bits are 0 or 0x3F and
    // whose region_total_bases entry is zero. This is a continue, not a jump
    // into a shared bases_reset path.
    //
    // The per-region state machine below follows 0x56294B..0x5634D5. In
    // particular, the native aggregates include the current faction where
    // noted, and the two contiguous() modes are ordered/short-circuited.
    int best_region_score = 0;
    int selected_attack_region = -1;
    for (int region = 1; region < 128; ++region) {
        // The original store at 0x562963 is PlayerData+0x11FC, the byte
        // region_total_offensive_vehs[region], not the later unk_83 array.
        faction_data.region_total_offensive_vehs[region] = 0;
        const uint8_t previous_region_plan =
            faction_data.region_base_plan[region];
        const int region_class = region & 0x3F;
        if (((region_class == 0 || region_class == 0x3F)
             && faction_data.region_total_bases[region] == 0)) {
            faction_data.region_base_plan[region] = 8;
            continue;
        }

        uint16_t &planning_flags = faction_data.unk_77[region];
        const int own_bases = faction_data.region_total_bases[region];
        const int own_combat = faction_data.region_total_combat_vehs[region];
        const int own_force = faction_data.region_force_rating[region];
        int factions_with_bases = 0;
        int no_base_or_mode0_count = 0;
        int no_base_or_either_mode_count = 0;
        int passing_hostile_count = 0;
        int failing_hostile_count = 0;
        int attack_flags = 0;

        // The native iterates faction IDs 1..7, including self for the broad
        // aggregates and base-presence flags. It skips self only after the
        // early combat/force gate.
        for (int other = 1; other < MaxPlayerNum; ++other) {
            const PlayerData &other_data = PlayersData[other];
            const uint8_t other_bases = other_data.region_total_bases[region];
            const uint16_t other_combat =
                other_data.region_total_combat_vehs[region];
            const uint16_t other_force = other_data.region_force_rating[region];
            const uint32_t treaty = faction_data.diplo_treaties[other];

            if (other_bases != 0) {
                ++factions_with_bases;
                planning_flags |= other == faction_id ? 0x0010u : 0x0020u;
                if ((treaty & 0x10u) != 0) planning_flags |= 0x0040u;
            }

            if (other_combat == 0 && own_force > 5 * other_force) continue;
            if (other == faction_id) continue;
            if ((treaty & 1u) != 0 || (treaty & 8u) == 0) continue;

            bool mode0_or_no_bases = false;
            bool either_mode_or_no_bases = false;
            if (GameDataStateStorage.fields_.turn_current_num_ > 1) {
                const int continent_size = Continents[region].tile_count;
                const bool large_continent = continent_size >= 500;
                const bool large_combined_base_count =
                    static_cast<int>(other_bases) * own_bases >= 50;
                if (large_continent || large_combined_base_count) {
                    const int cycle = continent_size >= 1000
                        ? (GameDataStateStorage.fields_.turn_current_num_ + other * 2) & 0x0F
                        : (GameDataStateStorage.fields_.turn_current_num_ + other) & 7;
                    if (cycle != 0 && other_bases != 0
                        && other_data.region_base_plan[region] > 2) {
                        mode0_or_no_bases = false;
                        either_mode_or_no_bases = false;
                    } else {
                        mode0_or_no_bases = true;
                        either_mode_or_no_bases = true;
                    }
                } else {
                    mode0_or_no_bases = other_bases == 0
                        || contiguous(faction_id, other, region, 0) != 0;
                    either_mode_or_no_bases = mode0_or_no_bases;
                    if (!either_mode_or_no_bases) {
                        either_mode_or_no_bases =
                            contiguous(faction_id, other, region, 1) != 0;
                    }
                    if (other_bases == 0) {
                        mode0_or_no_bases = true;
                        either_mode_or_no_bases = true;
                    }
                }
            } else {
                mode0_or_no_bases = other_bases == 0
                    || contiguous(faction_id, other, region, 0) != 0;
                either_mode_or_no_bases = mode0_or_no_bases;
                if (!either_mode_or_no_bases) {
                    either_mode_or_no_bases =
                        contiguous(faction_id, other, region, 1) != 0;
                }
                if (other_bases == 0) {
                    mode0_or_no_bases = true;
                    either_mode_or_no_bases = true;
                }
            }

            if ((treaty & 0x0C10u) != 0) {
                if (either_mode_or_no_bases) ++no_base_or_either_mode_count;
                if (mode0_or_no_bases) {
                    ++no_base_or_mode0_count;
                    if (no_base_or_mode0_count > 1) planning_flags |= 0x0200u;
                }

                const bool passes_force_test =
                    own_force <= static_cast<int>(other_combat)
                    || own_force <= static_cast<int>(other_force);
                const bool passes_pressure_test =
                    own_combat <= static_cast<int>(other_force)
                    && passes_force_test && own_bases != 0
                    && either_mode_or_no_bases;
                if (passes_pressure_test) ++passing_hostile_count;
                else ++failing_hostile_count;

                // Native 0x562C05..0x562C26 sets this additional movement
                // flag for sufficiently strong opponents with a larger
                // regional base count and an available contiguous mode.
                const int pressure_threshold = (3 * own_force) / 2;
                if (static_cast<int>(other_force) >= pressure_threshold
                    && own_bases > other_bases
                    && either_mode_or_no_bases) {
                    planning_flags |= 0x4000u;
                }

            } else if (own_bases != 0 && (treaty & 0x20u) != 0
                       && other_force > own_combat) {
                const bool weaker_than_double_own_force =
                    static_cast<int>(other_force) < 2 * own_force;
                const bool own_force_exceeds_other_combat =
                    own_force > static_cast<int>(other_combat);
                const bool other_bases_are_present = other_bases != 0;
                if (own_force_exceeds_other_combat && other_bases_are_present
                    && weaker_than_double_own_force) {
                    attack_flags |= 2;
                } else {
                    attack_flags |= either_mode_or_no_bases ? 1 : 2;
                }
            }

            // The native's human-faction branch checks actual unit presence
            // only for factions with a lock bit and at least one successful
            // contiguity/no-base mode. This is distinct from the later
            // wants_to_attack() flag below.
            const uint8_t other_lock_bit = static_cast<uint8_t>(
                1u << (static_cast<uint32_t>(other) & 7u));
            if ((treaty & 0x0C10u) != 0
                && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & other_lock_bit) != 0
                && (mode0_or_no_bases || either_mode_or_no_bases)) {
                for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_;
                     ++vehicle_id) {
                    const Veh &vehicle = Vehs[vehicle_id];
                    if (vehicle.faction_id == other
                        && map_loc(vehicle.x, vehicle.y)->region == region) {
                        planning_flags |= 0x0004u;
                        break;
                    }
                }
            }

            if ((treaty & 3u) != 0
                && (wants_to_attack(faction_id, other, 0)
                    || wants_to_attack(other, faction_id, 0))) {
                planning_flags |= 0x0004u;
            }
        }

        // 0x562D61 computes the region threshold from the planet's highest
        // faction ranking, this region's unk_82 byte, current globals, and
        // the self-inclusive faction count. These are source-owned fields;
        // arithmetic order mirrors the native integer instructions.
        const int regional_threshold = great_satan(GameDataStateStorage.fields_.faction_rankings_[7], 0)
            + (faction_data.unk_82[region] < 4 ? 1 : 0)
            - static_cast<int>((GameDataStateStorage.fields_.game_state_ >> 7) & 2u)
            + (GameDataStateStorage.fields_.diff_level_current_ > 4 ? 1 : 0)
            + (MapArea > 3200 ? 1 : 0)
            + (GameDataStateStorage.fields_.turn_current_num_ > 75 ? 1 : 0)
            + 3 + factions_with_bases;

        bool priority_region = false;
        if (own_bases != 0) {
            priority_region = faction_data.unk_82[region] < 2;
            const int controlled_area = faction_data.unk_80[region];
            const int area_delta = controlled_area - faction_data.unk_81[region];
            priority_region = priority_region
                || (planning_flags & 8u) != 0
                || own_bases >= 20
                || area_delta < 4;
            if (2 * own_bases
                    * (faction_data.ai_growth - faction_data.ai_wealth
                       + regional_threshold)
                > 2 * controlled_area) {
                const int half_bases = (own_bases + 1) / 2;
                if (area_delta / half_bases + faction_data.unk_82[region]
                    < 2 * regional_threshold) {
                    priority_region = true;
                }
            }
        }
        if (priority_region) planning_flags |= 1u;

        int region_plan = no_base_or_either_mode_count == 0 ? 8 : 7;
        if (no_base_or_either_mode_count == 0 && own_bases != 0) {
            planning_flags |= 0x0100u;
        } else {
            int local_plan_flags = attack_flags;
            if (failing_hostile_count != 0) local_plan_flags = 0;
            if ((local_plan_flags & 2) != 0) region_plan = 0;
            if (passing_hostile_count == 0 && (local_plan_flags & 1) != 0) {
                region_plan = 2;
            }
        }
        if (region >= 0x40) region_plan = 7;
        faction_data.region_base_plan[region] =
            static_cast<uint8_t>(region_plan);

        if (region < 0x40 && human_turn_bit == 0 &&
            region == old_attack_target_region &&
            no_base_or_mode0_count >= 2) {
            for (int group_id = 0; group_id < GameDataStateStorage.fields_.vehicle_current_count_; ++group_id) {
                Veh &group_vehicle = Vehs[group_id];
                if (group_vehicle.faction_id == faction_id
                    && (group_vehicle.state & 0x00040000u) != 0) {
                    group_vehicle.state |= 0x00020000u;
                }
            }
            faction_data.x_target = -1;
            faction_data.y_target = -1;
            faction_data.unk_71 = 0;
            faction_data.unk_72 = 0;
        }

        // Native 0x562F93..0x563092 picks a region only when the prior x/y
        // target is absent and this faction is not locked. Low plans prefer
        // regions with a +1000 bias, subject to the native counters/flag;
        // plan 7 uses the raw regional base count. Strict comparisons retain
        // the first region on ties.
        if (region < 0x40 && !reset_target && faction_data.x_target < 0
            && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0) {
            if (region_plan < 2 && no_base_or_mode0_count < 2
                && (planning_flags & 0x4000u) == 0) {
                const int candidate_score = own_bases + 1000;
                if (candidate_score > best_region_score) {
                    best_region_score = candidate_score;
                    selected_attack_region = region;
                }
            } else if (region_plan == 7 && own_bases > best_region_score) {
                best_region_score = own_bases;
                selected_attack_region = region;
            }
        }

        if (region < 0x40 && ((((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0)
             || (faction_id == ConsoleGlobal.faction_id_
                 && (GameDataStateStorage.fields_.game_preferences_ & 0x00020000u) != 0))
            && own_bases != 0
            && (region_plan == 7
                || (region_plan == 8 && Continents[region].tile_count < 0x96)))) {
            // Native 0x56310D..0x563433 scans every matching abstract
            // cell. For each outer neighbor direction, a same-region
            // neighbor subtracts one; otherwise it adds two and independently
            // probes all eight full-resolution radius starts, walking each
            // candidate ray in the outer direction.
            int best_site_score = 0;
            int best_site_x = -1;
            int best_site_y = -1;
            for (int abstract_y = 0;
                 abstract_y < static_cast<int>(MapAbstractLatBounds);
                 ++abstract_y) {
                for (int abstract_x = abstract_y & 1;
                     abstract_x < MapAbstractLongBounds;
                     abstract_x += 2) {
                    if (abstract_at(abstract_x, abstract_y) != region) continue;

                    int cell_score = 0;
                    int cell_site_x = -1;
                    int cell_site_y = -1;
                    for (int outer_direction = 0; outer_direction < 8;
                         ++outer_direction) {
                        int neighbor_x =
                            abstract_x + VehMoveDeltaX[outer_direction];
                        const int neighbor_y =
                            abstract_y + VehMoveDeltaY[outer_direction];
                        if ((MapIsFlat & 1) == 0) {
                            if (neighbor_x < 0) {
                                neighbor_x += MapAbstractLongBounds;
                            } else if (neighbor_x >= MapAbstractLongBounds) {
                                neighbor_x -= MapAbstractLongBounds;
                            }
                        }
                        if (neighbor_x >= 0
                            && neighbor_x < MapAbstractLongBounds
                            && neighbor_y >= 0
                            && neighbor_y < static_cast<int>(MapAbstractLatBounds)
                            && abstract_at(neighbor_x, neighbor_y) == region) {
                            --cell_score;
                            continue;
                        }

                        cell_score += 2;
                        for (int inner_direction = 0; inner_direction < 8;
                             ++inner_direction) {
                            int site_x = abstract_x * 5
                                + VehMoveDeltaX[inner_direction];
                            const int site_y = abstract_y * 5
                                + VehMoveDeltaY[inner_direction];
                            if ((MapIsFlat & 1) == 0) {
                                if (site_x < 0) site_x += MapLongitudeBounds;
                                else if (site_x >= MapLongitudeBounds) {
                                    site_x -= MapLongitudeBounds;
                                }
                            }
                            if (site_x < 0 || site_x >= MapLongitudeBounds
                                || site_y < 0 || site_y >= MapLatitudeBounds) {
                                continue;
                            }
                            if ((map_loc(site_x, site_y)->climate & 0xE0u) < 0x60u) {
                                continue;
                            }

                            int ray_x = site_x;
                            int ray_y = site_y;
                            int land_steps = 1;
                            int edge_x = -1;
                            int edge_y = -1;
                            for (; land_steps < 100; ++land_steps) {
                                int next_x =
                                    ray_x + VehMoveDeltaX[outer_direction];
                                const int next_y =
                                    ray_y + VehMoveDeltaY[outer_direction];
                                if ((MapIsFlat & 1) == 0) {
                                    if (next_x < 0) next_x += MapLongitudeBounds;
                                    else if (next_x >= MapLongitudeBounds) {
                                        next_x -= MapLongitudeBounds;
                                    }
                                }
                                if (next_y < 0 || next_y >= MapLatitudeBounds
                                    || next_x < 0 || next_x >= MapLongitudeBounds
                                    || (map_loc(next_x, next_y)->climate & 0xE0u) < 0x60u) {
                                    edge_x = ray_x;
                                    edge_y = ray_y;
                                    break;
                                }
                                ray_x = next_x;
                                ray_y = next_y;
                            }
                            if (land_steps >= 100 || edge_x < 0) continue;

                            const Map *edge_tile = map_loc(edge_x, edge_y);
                            if ((edge_tile->bit & 1u) != 0
                                && (edge_tile->val2 & 0x0Fu) < 8u) {
                                continue;
                            }
                            cell_score += 6;
                            if (Continents[region].tile_count >= 0x28) {
                                cell_score += 2;
                            }
                            cell_site_x = edge_x;
                            cell_site_y = edge_y;
                        }
                    }
                    if (cell_site_x >= 0 && cell_score > best_site_score) {
                        best_site_score = cell_score;
                        best_site_x = cell_site_x;
                        best_site_y = cell_site_y;
                    }
                }
            }
            if (best_site_score >= 1 && best_site_x >= 0
                && terraform_cost(best_site_x, best_site_y, faction_id)
                    <= ((faction_data.energy_reserves
                        + (faction_data.energy_reserves >> 31 & 7)) >> 3)) {
                add_goal(faction_id, AI_GOAL_LANDING_SITE, 2,
                         best_site_x, best_site_y, -1);
            }
        }
        // Native 0x5634A5 calls bases_reset only after the optional landing
        // site search/goal insertion, and only on the unlocked-faction edge.
        if (region < 0x40 && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0) {
            bases_reset(region, faction_id,
                        region_plan == previous_region_plan);
        }
    }

    // Native 0x5634D5..0x563A05 chooses a regional opponent, a movement
    // target, and up to eight nearby attacking vehicles. This is separate
    // from base_id_atk_target: it writes the x/y movement target and the
    // 0x800 strategy flag.
    int regional_enemy = -1;
    int regional_enemy_score = -1;
    const int selected_region = selected_attack_region;
    if (selected_region > 0 && selected_region < 0x3F
        && reset_target == false
        && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0
        && (faction_data.flags & 0x20000000u) == 0) {
        int current_rank = -1;
        for (int other = 1; other < MaxPlayerNum; ++other) {
                if (other == faction_id) continue;
                const uint32_t treaty = faction_data.diplo_treaties[other];
                if ((treaty & 0x0C14u) == 0) continue;
                if ((treaty & 0x0810u) == 0
                    && (invalid_target_vehicle_count > 3
                        || PlayersData[other].region_total_bases[selected_region] == 0)) {
                    continue;
                }
                const int score = ((treaty & 0x10u) << 7)
                    + PlayersData[other].region_total_bases[selected_region];
                if (score > current_rank) {
                    current_rank = score;
                    regional_enemy = other;
                    regional_enemy_score = score;
                }
        }

        if (regional_enemy >= 0) {
            const bool regional_special_case = regional_enemy_score == 0x800;
            int nearest_target_vehicle = -1;
            int nearest_target_distance = 9999;
            for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
                const Veh &vehicle = Vehs[vehicle_id];
                if (vehicle.faction_id != faction_id) continue;
                const Map *tile = map_loc(vehicle.x, vehicle.y);
                if ((tile->climate & 0xE0u) < 0x60u
                    || tile->region != selected_region) {
                    continue;
                }
                if (regional_special_case
                    && !is_coast(vehicle.x, vehicle.y, 0)) {
                    continue;
                }

                int distance = 9999;
                const int designated_base = faction_data.base_id_atk_target;
                if (designated_base >= 0) {
                    const Base &target_base = Bases[designated_base];
                    const int target_owner = target_base.faction_id_current;
                    if ((faction_data.diplo_treaties[target_owner] & 0x0810u) != 0
                        && map_loc(target_base.x, target_base.y)->region
                            == selected_region) {
                        distance = vector_dist(
                            x_dist(vehicle.x, target_base.x),
                            vehicle.y - target_base.y);
                        BaseFindDist = distance;
                    }
                }
                if (distance == 9999) {
                    const int found_base = base_find(
                        vehicle.x, vehicle.y, regional_enemy, selected_region,
                        -1, -1);
                    if (found_base >= 0) {
                        distance = BaseFindDist;
                    } else {
                        base_find(vehicle.x, vehicle.y, regional_enemy);
                        distance = BaseFindDist * 2;
                    }
                }
                if (distance < nearest_target_distance) {
                    nearest_target_distance = distance;
                    nearest_target_vehicle = vehicle_id;
                }
            }

            if (nearest_target_vehicle >= 0) {
                const Veh &target_vehicle = Vehs[nearest_target_vehicle];
                faction_data.x_target = target_vehicle.x;
                faction_data.y_target = target_vehicle.y;
                if (regional_special_case) faction_data.flags |= 0x00000800u;
                else faction_data.flags &= ~0x00000800u;

                int assigned_attackers = 0;
                const int32_t twice_naval_total = static_cast<int32_t>(
                    static_cast<uint32_t>(faction_data.unk_70) << 1);
                const int special_case_cap =
                    (twice_naval_total + (twice_naval_total >> 31 & 0x003FFFFF)) >> 22;
                for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_;
                     ++vehicle_id) {
                    Veh &vehicle = Vehs[vehicle_id];
                    if (vehicle.faction_id != faction_id
                        || (vehicle.state & 0x00060000u) != 0) {
                        continue;
                    }
                    const VehPrototype &prototype =
                        VehPrototypes[vehicle.proto_id];
                    if (prototype.plan > 2
                        || Chassis[prototype.chassis_id].triad != TRIAD_LAND) {
                        continue;
                    }
                    const Map *tile = map_loc(vehicle.x, vehicle.y);
                    if ((tile->climate & 0xE0u) < 0x60u
                        || tile->region != selected_region) {
                        continue;
                    }
                    if (vehicle.order != 0 && vehicle.order != 0x18) continue;

                    const int nearby_base = base_find(
                        vehicle.x, vehicle.y, regional_enemy);
                    if (nearby_base < 0) continue;
                    const int target_distance = vector_dist(
                        x_dist(vehicle.x, faction_data.x_target),
                        vehicle.y - faction_data.y_target);
                    if (BaseFindDist <= target_distance) continue;

                    const int weapon_value =
                        weap_strat(prototype.weapon_id, faction_id);
                    const int armor_value =
                        arm_strat(prototype.armor_id, faction_id);
                    if (weapon_value < armor_value
                        && ((faction_data.unk_72 & 1) != 0
                            || regional_special_case)) {
                        continue;
                    }
                    vehicle.state &= ~0x00020000u;
                    vehicle.state |= 0x00040000u;
                    ++faction_data.unk_71;
                    ++assigned_attackers;
                    if (weapon_value < armor_value) faction_data.unk_72 |= 1;
                    if (assigned_attackers >= 8
                        || (regional_special_case
                            && assigned_attackers >= special_case_cap)) {
                        break;
                    }
                }
            }
        }
    }

    // Native 0x563A64..0x564858 runs only for a nonzero turn and nonzero
    // faction. It clears the 75 goal-use counters, converts this faction's
    // prior move marker '1' to the retry marker '2', then performs two
    // descending passes (the first only processes marker '2').
    if (GameDataStateStorage.fields_.turn_current_num_ != 0 && faction_id != 0) {
      int goal_assignments[75];
      for (int goal_id = 0; goal_id < 75; ++goal_id) {
          goal_assignments[goal_id] = 0;
      }
      for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle_id >= 0; --vehicle_id) {
          Veh &vehicle = Vehs[vehicle_id];
          if (vehicle.faction_id == faction_id && vehicle.move_to_ai_type == '1') {
              vehicle.move_to_ai_type = '2';
          }
      }
      for (int planning_pass = 0; planning_pass < 2; ++planning_pass) {
        for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle_id >= 0; --vehicle_id) {
            Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id != faction_id) continue;
            if (planning_pass == 0 && vehicle.move_to_ai_type != '2') continue;

            const int prototype_id = vehicle.proto_id;
            const VehPrototype &prototype = VehPrototypes[prototype_id];
            const int plan = prototype.plan;
            const int x = vehicle.x;
            const int y = vehicle.y;
            const Map *current_tile = map_loc(x, y);
            int region = current_tile->region;
            int region_plan = faction_data.region_base_plan[region];
            const int current_triad = Chassis[prototype.chassis_id].triad;

            if (planning_pass == 0) {
                // Native 0x563B14..0x563B47 restricts the retry pass to
                // marker '2', excludes plan 9 unconditionally, and also
                // excludes plans 0..2 when the region plan is at most 2.
                if (vehicle.move_to_ai_type != '2' || plan == 9
                    || (region_plan <= 2 && plan <= 2)) {
                    continue;
                }
            }

            // Native 0x563B4A clears this flag after the first-pass filters,
            // but before the order and locked-human filters below.
            vehicle.state &= ~0x40000000u;

            if (vehicle.order > 3 && vehicle.order < 0x18) continue;
            const bool human_controlled = (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) != 0;
            if (human_controlled) {
                if ((vehicle.state & 0x200u) == 0) continue;
                if (plan == 9 && vehicle.probe_action != 0) continue;
                // Native 0x563BB2 rejects the exact pair of state bits
                // 0x1000200.  The following 0x563BC6 test is a different
                // two-bit case and compares the native adjacent vehicle
                // byte pair; these are not one generic "human unit" gate.
                if ((vehicle.state & 0x1000200u) == 0x1000200u) continue;
                if ((vehicle.state & 0x2000200u) == 0x2000200u
                    && vehicle.patrol_current_point < vehicle.waypoint_count) {
                    continue;
                }
            }

            int movement_class = plan;

            const uint32_t movement_target_state = vehicle.state & 0x00060000u;
            const bool has_both_target_state_bits =
                movement_target_state == 0x00060000u;
            if ((vehicle.state & 0x10000000u) != 0) {
                // 0x563C20 increments the BYTE region_total_offensive_vehs
                // counter and routes this unit as movement class 2.
                ++faction_data.region_total_offensive_vehs[region];
                movement_class = 2;
            } else if (region < 0x40 && movement_class <= 1
                       && region_plan <= 2) {
                // Native 0x563C29..0x563D42 balances offensive units against
                // a per-region capacity. The divisor begins at 2-ai_fight,
                // then plan 8 adds two, plan 2 selects one, and the late-game
                // GameState bit selects one. The original signed divide is
                // `(bases + divisor - 1) / divisor`.
                int divisor = 2 - faction_data.ai_fight;
                if (region_plan == 8) divisor += 2;
                else if (region_plan == 2) divisor = 1;
                if ((GameDataStateStorage.fields_.game_state_ & 0x40000000u) != 0) divisor = 1;
                const int regional_capacity =
                    (static_cast<int>(faction_data.region_total_bases[region])
                     + divisor - 1) / divisor;
                if (regional_capacity > 0) {
                    const Map *tile = map_loc(x, y);
                    int tile_climate = -1;
                    if ((tile->bit & 1u) != 0) {
                        const int climate = tile->climate & 0x0F;
                        if (climate < 8) tile_climate = climate;
                    }
                    if (tile_climate == faction_id
                        && (region_plan == 2
                            || (faction_data.unk_77[region] & 0x40u) != 0)
                        && stack_check(vehicle_id, 0x0B, 0x10000000,
                                       0x10000000, -1) < 1) {
                        if (movement_target_state == 0
                            && faction_data.region_total_offensive_vehs[region]
                                   < regional_capacity) {
                            ++faction_data.region_total_offensive_vehs[region];
                            vehicle.state |= 0x10000000u;
                            movement_class = 2;
                        }
                    }
                }
            }
            // 0x563D45..0x563E69 compares the current tile's water predicate
            // with whether the region record is class 1. On a mismatch it
            // gates on the fungus bit and owner nibble, then runs an
            // eight-offset local search. The search's success comparison
            // repeats the same saved water/class pair used by the mismatch
            // gate, without changing either value inside the loop; its success
            // edge is therefore unreachable. The native block has no global
            // side effects, so keep the observable edge (reject non-fungus or
            // owner IDs outside 0..7) and continue planning in the original
            // region.
            if (current_triad != TRIAD_AIR) {
                const bool current_is_ocean =
                    (current_tile->climate & 0xE0u) < 0x60u;
                const bool chassis_is_sea = current_triad == TRIAD_SEA;
                if (current_is_ocean != chassis_is_sea) {
                    const int tile_owner = current_tile->val2 & 0x0F;
                    if ((current_tile->bit & 1u) == 0 || tile_owner >= 8) {
                        continue;
                    }
                }
            }
            // Native's class-1 remap uses the original tile region: the
            // apparent neighbor-region assignment above cannot execute.
            if (movement_class == 1) {
                if (region_plan == 2 || region_plan == 8) movement_class = 2;
                else movement_class = 0;
            }

            // The target-state shortcut follows terrain transition and its
            // region-plan remap, before class-7 cargo and goal processing.
            if (movement_target_state == 0x00040000u
                && faction_data.x_target >= 0) {
                vehicle.order = 0x18;
                vehicle.move_to_ai_type = '0';
                vehicle.waypoint_x[0] = static_cast<int16_t>(faction_data.x_target);
                vehicle.waypoint_y[0] = static_cast<int16_t>(faction_data.y_target);
                if (plan == 8) vehicle.probe_sabotage_id = 0;
                continue;
            }
            if (has_both_target_state_bits) {
                movement_class = 0;
            }
            // At 0x563EFB..0x563F19 the native rejects an under-filled
            // carrier before visiting any goal. Keep this ahead of the goal
            // loop: the loop can delete invalid sites, so a post-selection
            // cargo test would have observably different side effects.
            if (movement_class == 7
                && stack_check(vehicle_id, 8, -1, -1, -1)
                    < veh_cargo(vehicle_id)) {
                continue;
            }
            // Native checks the 'F' move marker only after its class-7 cargo
            // validation, and suppresses this unit's goal search when its
            // resulting class is 2.
            if (vehicle.move_to_ai_type == 'F' && movement_class == 2) {
                continue;
            }
            // Native 0x563F61..0x564017 tests a marked advance through
            // fungus only after the class-7 cargo and 'F'/class-2 gates.
            // Keep its two independent stack queries and base garrison
            // fallback in native order.
            if (vehicle.move_to_ai_type == '2' && !has_both_target_state_bits
                && (current_tile->bit & 1u) != 0
                && (current_tile->climate & 0x0Fu) <= 7) {
                const int class_c_stack = stack_check(
                    vehicle_id, 0x0C, -1, -1, -1);
                const int all_stack = stack_check(
                    vehicle_id, 1, -1, -1, -1);
                if (all_stack - class_c_stack <= 1) continue;
                if (movement_class == 1 || movement_class == 2
                    || movement_class == 3) {
                    const int current_base = base_at(x, y);
                    const int fortification_stack = stack_check(
                        vehicle_id, 0x10, -1, -1, -1);
                    if (fortification_stack < garrison_check(current_base)) {
                        continue;
                    }
                }
            }
            // The native skips the complete goal walk for class-9 vehicles
            // carrying the 0x0800 flag in the word at Veh+0x30.
            if (movement_class == 9 && (vehicle.flags & 0x0800u) != 0) {
                continue;
            }

            int best_goal = -1;
            int best_cost = 99999;
            for (int goal_id = 0; goal_id < 75; ++goal_id) {
                Goal &goal = faction_data.goals[goal_id];
                // Native 0x564057 filters on the positive priority word,
                // then separately rejects negative goal types at 0x56406C.
                // Type zero (attack) is a valid goal and must remain eligible.
                if (goal.priority <= 0 || goal.type < 0) continue;
                const int goal_kind = goal.type & 0x0F;
                if (goal_kind != movement_class
                    && !(goal.type == AI_GOAL_SCOUT && !has_both_target_state_bits
                         && movement_class <= 3
                         && vehicle.move_to_ai_type != '2')
                    && !(goal_kind == AI_GOAL_LANDING_SITE && plan == 9)) {
                    continue;
                }
                // Native validates coordinates only after the goal-type /
                // movement-class compatibility gates (0x564057..0x5640D8).
                // Invalid coordinates are reset as a side effect at that
                // point; a type-incompatible goal is left untouched.
                if (goal.x < 0 || goal.x >= MapLongitudeBounds
                    || goal.y < 0 || goal.y >= MapLatitudeBounds) {
                    goal.x = -1;
                    goal.y = -1;
                    continue;
                }
                const int goal_region = map_loc(goal.x, goal.y)->region;
                const RulesChassis &movement_chassis = Chassis[prototype.chassis_id];
                if (movement_chassis.triad == TRIAD_AIR) continue;
                const bool goal_is_ocean =
                    (map_loc(goal.x, goal.y)->climate & 0xE0u) < 0x60u;
                if (goal_is_ocean != (movement_chassis.triad == TRIAD_SEA)) {
                    continue;
                }
                if (prototype.unk_1 == 0x15) continue;
                if (goal_region != region) continue;

                if (movement_class == 0 && goal.type == AI_GOAL_ATTACK
                    && (map_loc(goal.x, goal.y)->bit & 0x00002000u) != 0) {
                    if (!want_monolith(vehicle_id) || has_both_target_state_bits
                        || goal_assignments[goal_id] != 0) {
                        continue;
                    }
                }
                if (goal.type == AI_GOAL_SCOUT
                    && has_abil(prototype_id, 0x0800)) {
                    continue;
                }
                const int dx = x_dist(x, goal.x);
                const int dy = y - goal.y;
                int scaled_distance = vector_dist(dx, dy) * 0x100;
                if (goal.type == AI_GOAL_COLONIZE
                    && base_territory(faction_id, goal.x, goal.y) >= 0) {
                    del_site(faction_id, AI_GOAL_COLONIZE, goal.x, goal.y, 0);
                    continue;
                }

                // A landing-site goal's native priority is multiplied by
                // its cached site rank. If the cache is zero, the original
                // computes and stores world_site on this goal tile; the
                // 0x1000 bit in Map::bit2 is the native forced-minimum case.
                int landing_site_rank = 1;
                if (goal.type == AI_GOAL_COLONIZE) {
                    Map *landing_tile = map_loc(goal.x, goal.y);
                    if ((landing_tile->bit & 0x00001000u) == 0) {
                        landing_site_rank = (landing_tile->val2 >> 4) & 0x0F;
                        if (landing_site_rank == 0) {
                            landing_site_rank = world_site(goal.x, goal.y, 0);
                            site_set(goal.x, goal.y, landing_site_rank);
                        }
                        if (landing_site_rank < 1) landing_site_rank = 1;
                        else if (landing_site_rank >= 16) landing_site_rank = 15;
                    }
                }

                // Native 0x5644BD..0x56450B applies this only to units whose
                // state already has 0x10000000, and queries the stack at the
                // goal coordinate. The one-unit allowance comes from the
                // scaled distance being zero (the vehicle is already there),
                // not from its movement class.
                if ((vehicle.state & 0x10000000u) != 0) {
                    const int goal_stack = veh_at(goal.x, goal.y);
                    if (goal_stack >= 0) {
                        const int protected_goal_units = stack_check(
                            goal_stack, 0x0B, 0x10000000, 0x10000000,
                            faction_id);
                        const int stack_allowance = scaled_distance == 0 ? 1 : 0;
                        if (protected_goal_units > stack_allowance) continue;
                    }
                }

                const int goal_base_id = base_at(goal.x, goal.y);
                const int goal_record_base_id = goal.base_id;
                int prior_assignments = goal_assignments[goal_id];
                if (has_both_target_state_bits
                    || (goal_base_id >= 0
                        && Bases[goal_base_id].faction_id_current == faction_id
                        && Bases[goal_base_id].nerve_staple_turns_left != 0)) {
                    prior_assignments = 0;
                }
                const int remaining_priority =
                    (static_cast<int>(goal.priority) - prior_assignments)
                    * landing_site_rank;
                if (remaining_priority <= 0) continue;

                // Native 0x56451F..0x564553 tests the candidate goal record's
                // base state, then applies the 3x cost only when this unit's
                // home base is a different ID. It does not read the current
                // unit's home-base state for this first surcharge.
                if (goal.type == AI_GOAL_TERRAFORM_LAND
                    && goal_record_base_id >= 0
                    && (Bases[goal_record_base_id].state & 0x20u) != 0
                    && vehicle.home_base_id != goal_record_base_id) {
                    scaled_distance *= 3;
                }
                const int territory_owner =
                    whose_territory(faction_id, goal.x, goal.y, 0, 0);
                if (territory_owner >= 0 && territory_owner != faction_id) {
                    const uint32_t territory_relation =
                        faction_data.diplo_treaties[territory_owner];
                    // 0x5646E1..0x5646FD leaves 0xC10 and treaty bit 0 at
                    // the unmodified-distance edge, rejects 0x00100000 or
                    // treaty bit 1, and doubles only when none apply.
                    if ((territory_relation & 0x00100000u) != 0) {
                        continue;
                    }
                    if ((territory_relation & 0x00000002u) != 0) {
                        continue;
                    }
                    if ((territory_relation & (0x00000C10u | 0x00000001u)) == 0) {
                        scaled_distance *= 2;
                    }
                }
                if (goal.type == AI_GOAL_PRIORITY_COMBAT) {
                    if (!has_abil(prototype_id, 0x8000)
                        || movement_chassis.triad != TRIAD_LAND) {
                        continue;
                    }
                    // Native uses CDQ / SUB sign / SAR at 0x56474A..0x56474F;
                    // this is signed division by two rounded toward zero.
                    scaled_distance /= 2;
                }
                // Native 0x564511 gates this scan on the current goal record's
                // type 9 (terraform-land), not on this vehicle's plan. It
                // reads this goal's base_id and compares the current vehicle
                // against other friendly plan-9 vehicles using that base's
                // relocation state.
                if (goal.type == AI_GOAL_TERRAFORM_LAND) {
                    bool closer_plan9_unit = false;
                    for (int other_vehicle_id = 0;
                         other_vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_;
                         ++other_vehicle_id) {
                        if (other_vehicle_id == vehicle_id) continue;
                        const Veh &other_vehicle = Vehs[other_vehicle_id];
                        if (other_vehicle.faction_id != faction_id
                            || VehPrototypes[other_vehicle.proto_id].plan != 9) {
                            continue;
                        }
                        int other_distance = vector_dist(
                            x_dist(other_vehicle.x, goal.x),
                            other_vehicle.y - goal.y) * 0x100;
                        if (goal_record_base_id >= 0
                            && (Bases[goal_record_base_id].state & 0x20u) != 0
                            && other_vehicle.home_base_id != goal_record_base_id) {
                            other_distance *= 3;
                        }
                        if (other_distance < scaled_distance) {
                            closer_plan9_unit = true;
                            break;
                        }
                    }
                    if (closer_plan9_unit) continue;
                }
                const int cost = scaled_distance / remaining_priority;
                if (cost < best_cost) {
                    best_cost = cost;
                    best_goal = goal_id;
                }
            }

            if (best_goal < 0) continue;
            Goal &goal = faction_data.goals[best_goal];
            const int target_base = base_at(goal.x, goal.y);
            const uint8_t previous_move_marker = vehicle.move_to_ai_type;
            vehicle.order = 0x18;
            vehicle.move_to_ai_type = '1';
            vehicle.waypoint_x[0] = static_cast<int16_t>(goal.x);
            vehicle.waypoint_y[0] = static_cast<int16_t>(goal.y);
            if (previous_move_marker < 0x33 || previous_move_marker >= 0x36) {
                ++goal_assignments[best_goal];
            }
            if (plan == 8) vehicle.probe_sabotage_id = 0;
            if (goal.type == 3) vehicle.state |= 0x40000000u;
            if (goal.type == 2 && target_base >= 0
                && Bases[target_base].faction_id_current == faction_id) {
                Bases[target_base].state |= 0x08000000u;
                if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) == 0) {
                    base_reset(target_base, 0);
                }
            }
        }
      }
        wipe_goals(faction_id);
    }
}


/*
Purpose: Wrap an x coordinate on a round map, as set_course does it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

xrange() at 0048BEE0 cannot stand in, for one bit: it tests the whole of MapIsFlat while
set_course reads the low BYTE and tests bit zero (`mov cl, byte ptr [94988Ch] / test cl, 1`),
the same reading valid_patrol above and reset_territory use. The two agree on the 0 and 1 the
game stores there and disagree on everything else.
*/
static inline int course_xrange(int x) {
    if (!(MapIsFlat & 1)) {
        if (x >= 0) {
            if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        } else {
            x += MapLongitudeBounds;
        }
    }
    return x;
}

/*
Purpose: Send the unit toward the given tile the long way round - by water - choosing the anchorage
         next to it that fronts the most of the destination's own landmass.
// ORIGINAL: 0x00564890 ?set_course@@YAXHDHH@Z 0x00564890-0x00564B82
// TRIED: call structure already matches (`osmx calls --all` lists only
//            base_at and base_on_sea, same as the image's own `calls` list),
//            so the gap is register/control-flow noise across a 754-byte,
//            21-candidate scoring loop, not a missing inline or a wrong
//            callee. Best flag set /O2 /Ob0 /Gy /GR- /Oy- /GX, 0.558
//            similar. Not chased further in this pass - the semantic content
//            is already verified (mutant sweep kills 28/30, two equivalent).
// size      754 bytes
// prototype void (__cdecl ?set_course@@YAXHDHH@Z)(int vehID, int8 type, int xCoord, int yCoord)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x0050DE50
Return Value: n/a
Status: Complete

The direct answer comes first and is narrow: only when the destination is in the SAME region as
the unit AND the unit is itself afloat - its own tile below ALT_BIT_SHORE_LINE - does this hand
the coordinates straight to go_to. Anything else is a crossing, and the rest of the body picks
where to cross to.

The candidates are the 21 tiles of the destination's base radius, and each must be water. Which
water is decided by what the unit is sitting in: a unit inside a base asks base_on_sea whether
that base touches the candidate's sea region, and a unit at sea requires the candidate to be in
its own region. That is the reachability test, and it is the only one - nothing here checks
distance or fuel.

Each surviving candidate is then scored by FRONTAGE: how many of its eight neighbours are land,
belong to the DESTINATION's region, and hold no base. Land is the same ALT_BIT_SHORE_LINE line
read the other way round, the region is the destination's rather than the candidate's, and the
base test is base_who() - which answers -1 both for an empty tile and for a base tile whose owner
nibble is 0xF, so only a real faction-owned base takes a neighbour out of the count.

The best frontage wins, and the starting bar depends on the destination itself: zero when the
destination is land, minus one when it is water. A water destination therefore accepts an
anchorage that fronts NOTHING, which is what lets a unit be sent to open sea at all; a land
destination will not move the unit unless some anchorage fronts at least one tile of it.

Ties keep the earlier candidate, and every improvement re-issues the order, so the unit ends up
aimed at the last candidate that strictly beat everything before it.

The x wrap is the shared wrap_map_x inline (x < 0 tested first, as the image does).

Verification note: the sweep against recovery-gameplay-tests kills 28 of 30 valid mutants twice
over, and both survivors are equivalences rather than untested behaviour.

  - Widening the neighbour loop to nine reaches RadiusBase entry 8, which is (0, 0) - the
    anchorage itself. Three lines earlier the anchorage was required to be below
    ALT_BIT_SHORE_LINE, and a neighbour is required to be at or above it, so that iteration can
    never reach the count no matter what the map holds.
  - Swapping `best_frontage = frontage;` with the go_to call behind it. go_to writes Veh fields
    and reads Vehs and VehPrototypes; the assignment reads a local the callee cannot name and
    writes a local the callee cannot see. Neither statement observes the other's effect, in
    either order.
*/
void __cdecl set_course(int veh_id, char type, int x, int y) {
    Veh &veh = Vehs[veh_id];
    int veh_region = map_loc(veh.x, veh.y)->region;
    int base_id = base_at(veh.x, veh.y);
    Map *destination = map_loc(x, y);
    int dst_region = destination->region;
    if (veh_region == dst_region
        && (map_loc(veh.x, veh.y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
        go_to(veh_id, type, x, y);
        return;
    }
    int best_frontage = ((destination->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) ? 0 : -1;
    for (int i = 0; i < 21; i++) {   // the destination's own base radius
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius)) {
            continue;
        }
        Map *anchorage = map_loc(x_radius, y_radius);
        if ((anchorage->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
            continue;
        }
        if (base_id >= 0) {
            if (!base_on_sea(base_id, anchorage->region)) {
                continue;
            }
        } else if (anchorage->region != veh_region) {
            continue;
        }
        int frontage = 0;
        for (int j = 0; j < 8; j++) {
            int x_adj = wrap_map_x(x_radius + VehMoveDeltaX[j]);
            int y_adj = y_radius + VehMoveDeltaY[j];
            if (!on_map(x_adj, y_adj)) {
                continue;
            }
            Map *shore = map_loc(x_adj, y_adj);
            if ((shore->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
                continue;
            }
            if (shore->region != dst_region) {
                continue;
            }
            if (base_who(x_adj, y_adj) >= 0) {
                continue;
            }
            frontage++;
        }
        if (frontage > best_frontage) {
            best_frontage = frontage;
            go_to(veh_id, type, x_radius, y_radius);
        }
    }
}

// ORIGINAL: 0x00564B90 ?assemble_passengers@@YAXHHH@Z 0x00564B90-0x00564D90 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:99d6e72bcd5ae3be3566222736528cb0dc57935c:src/recovered/units/00564b90.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover full passenger-goal orchestration with real region_total_bases and unk_77 arrays, vendetta/contiguity tests and colony/defend/attack/scout goal order."}
void __cdecl assemble_passengers(int base_id, int mode, int priority) {
    const int x = Bases[base_id].x;
    const int y = Bases[base_id].y;
    const int faction_id = Bases[base_id].faction_id_current;
    const int region = map_loc(x, y)->region;
    int disconnected_factions = 0;

    for (int other_faction = 1; other_faction < MaxPlayerNum; ++other_faction) {
        if (other_faction == faction_id) {
            continue;
        }

        if ((PlayersData[faction_id].diplo_treaties[other_faction] &
             DTREATY_VENDETTA) == 0) {
            continue;
        }

        if (PlayersData[other_faction].region_total_bases[region] == 0 ||
            contiguous(other_faction, faction_id, region, 1) == 0) {
            ++disconnected_factions;
        }
    }

    Bases[base_id].state |= BSTATE_UNK_1;
    Bases[base_id].unk_1 = 2;

    if (mode != 8 || (PlayersData[faction_id].unk_77[region] & 0x0800) != 0) {
        if (disconnected_factions <
            range(PlayersData[faction_id].ai_growth -
                      PlayersData[faction_id].ai_fight -
                      GameDataStateStorage.fields_.turn_current_num_ / 100 + 2,
                  0, 99)) {
            add_goal(faction_id, AI_GOAL_COLONIZE,
                     stack_check(veh_at(x, y), 2, AI_GOAL_COLONIZE,
                                 faction_id, -1) != 0 ? 1 : priority,
                     x, y, -1);
        }

        if (mode <= 2) {
            goto scout_goal;
        }
    }

    add_goal(faction_id, AI_GOAL_DEFEND, priority, x, y, -1);
    add_goal(faction_id, AI_GOAL_ATTACK, priority, x, y, -1);
    add_goal(faction_id, AI_GOAL_UNK_3, priority, x, y, -1);
scout_goal:
    if (mode == 7) {
        add_goal(faction_id, AI_GOAL_SCOUT, 1, x, y, -1);
    }
}

/*
Purpose: Say whether a convoy-to-nearest-base order is possible from a tile:
         reject occupied-enemy stacks, then walk the base array - wrapping
         the x distance on a wrapping map - and reject when any base within
         convoy range (the hex distance minus the diamond correction below
         three) has the approach direction bit set in its worked-tile word.

// ORIGINAL: 0x00564D90 ?can_convoy@@YAHHH@Z 0x00564D90-0x00564EAA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["veh.h owns the actual Veh and Base arrays and the vehicle APIs veh_at/stack_check; base.h/map.h own the traversed Base records, map globals, and radius_move API.","console.cpp and enemy.cpp are current source callers and both include veh.h; the duplicate console.h declaration is removed in favor of the actual vehicle API header.","The redundant four abs calls, x-wrap guard, pointer walk, worked-radius test, and early rejection order remain unchanged."],"reason":"Move the complete convoy eligibility helper beside the vehicle owner and expose its existing cdecl API through veh.h without simplifying its distance or call sequence."}
// TRIED: transcribing Ghidra's redundant double-abs() calls (abs(dx) and abs(abs(dx))) literally rather than simplifying them away, since the original genuinely emits four `_abs` calls per loop iteration, not two. Landed as MISMATCH at 0.78 similarity: the remaining gap starts with the prologue's `push ecx` filler (a one-slot local reserved with a throwaway push instead of `sub esp,4`) that this body's local declaration order does not reproduce.
// size      282 bytes
// prototype int (__cdecl ?can_convoy@@YAHHH@Z)(int32 xCoord, int32 yCoord)
// callers   2   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005A65D0 0x005B9580 0x005BFE90 0x00644F3A
// notes     Seed provenance (TRIED): the image's four _abs calls per
//           iteration are kept (including the redundant re-abs of the
//           already-wrapped dx) and the walk stays a pointer walk over the
//           0x134-byte base records. The surviving divergence is the
//           prologue's one-slot local reserved by a throwaway `push ecx`
//           (the mon_* family wall), so the body is not measured exact.
Return Value: 0 impossible, 1 possible
Status: Coverage transcription - unclaimed body, seed
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00564d90.cpp retained.
LEVER: veh_at nested in the stack_check call, an indexed Bases[i] loop, and
        x_dist plus vector_dist(int, int) as inline helpers.
*/
int __cdecl can_convoy(int x, int y) {
    if (stack_check(veh_at(x, y), 9, 3, -1, -1) != 0) {
        return 0;
    }
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        const int base_y = Bases[i].y;
        if (vector_dist(x_dist(x, Bases[i].x), abs(y - base_y)) <= 2) {
            const int dir = radius_move(Bases[i].x, Bases[i].y, x, y, 0x15);
            if (dir >= 0 && (Bases[i].resource_sqr_radius & (1 << dir)) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

/*
Purpose: Check whether a sensor array is worth building on the specified tile.
// ORIGINAL: 0x00564EB0 ?good_sensor@@YAHHHH@Z 0x00564EB0-0x00565157
// size      679 bytes
// prototype int (__cdecl ?good_sensor@@YAHHHH@Z)(int factionID, int xCoord, int yCoord)
// callers   2   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3EF0 0x00592030 0x005B9F20 0x005BF010
Return Value: Is the tile a good sensor site? true/false
Status: Complete

TRIED: wrap_map_x in both loops, base_who, and the shift forms of the rocky and
altitude tests make the shape exact apart from the first loop's x_radius register
(edi in the image) and its flat-map block order. has_tech stays a real call.
LEVER: the first loop's wrap as a flat-first ternary gives the image's block order
(MNEMONIC_ONLY 240/242). Left: the reload order of faction/y/x before the last
whose_territory call (image ebx, esi, edi).

Three questions in order, and every one of them can answer no on its own.

WHO OWNS IT. The tile must already be the faction's own territory, and if it is fungus at
ALT_BIT_OCEAN_SHELF or above the faction must hold Rules.tech_improve_fungus_sqr - the same
technology that lets it work a fungus square at all, at offset 0x3C of RulesBasic.

IS IT WANTED. The 25 tiles of the surrounding radius are scanned. Any one of them that already
carries a sensor standing on the faction's own territory answers no outright, which is what stops
sensors being stacked on top of each other. Then, for the NINE innermost tiles only, three
independent reasons to want one are collected as bits of a mask:

  1 - the tile is not the faction's own territory. A sensor on the border is worth having.
  2 - one of the nine holds a base of this faction.
  4 - that base has no sensor anywhere in its own 25-tile radius.

A mask of zero answers no. Nothing here weighs the three against each other; any one will do, and
2 without 4 is reachable because the base's radius is larger than the nine tiles just searched.

Note that bit 2 is set once per qualifying base and bit 4 is decided per base, so a second base
without cover sets 4 even when the first one had it. The original tests the flags only for being
nonzero, so this is a distinction without a difference, and it is reproduced as written.

IS THE TILE FREE. A resource bonus, a base, a monolith, a condenser, a thermal borehole, a mine or
a solar collector each answer no, because the sensor would displace something already there. What
is left answers yes when it is ROCKY, or when it is arid - neither RAINFALL_RAINY nor
RAINFALL_MOIST - which are the tiles worth least as terrain. Anything else answers yes only if it
is fungus at ALT_BIT_OCEAN_SHELF or above, and no otherwise.

Verification note: the sweep against recovery-gameplay-tests kills 23 of 31 valid mutants twice
over, and the eight survivors are two proven equivalences rather than untested behaviour.

BIT 4 CANNOT CHANGE THE ANSWER, and neither can the 25-tile search that decides it. Seven
survivors are that one fact: the `25 -> 0`, `0 -> 1` and `< -> <=` mutants of the inner loop, the
inverted territory comparison inside it, the dropped `base_covered = true`, the dropped
`reasons |= 4` and its `4 -> 0`. The proof is short. `reasons` is written in exactly three places
and read in exactly one, `if (!reasons)`. Bit 4 is set only inside the block that has already
executed `reasons |= 2` two statements earlier, so every state in which bit 4 could be set is
already nonzero, and nothing distinguishes 6 from 2. The whole cover search is therefore dead as
far as the return value goes - in the original as much as here, since this is a transcription of
what 0x00564FEE through 0x00565076 does.

It is kept because it is what the original executes, and because it is not quite side-effect
free: is_sensor falls through to base_find, which writes BaseFindDist. That write is overwritten
again by every later on-map iteration of the OUTER loop, so it survives the call only when the
base sits in the inner nine and every remaining radius tile is off the map - which no fixture
here builds and which changes nothing this function returns either way.

The eighth survivor is bonus_at's third argument. The parameter is declared UNUSED in
map_grid.cpp:00592030's recovery and read nowhere in its body, so 0 and 1 are the same call. The
original passes 0 and so does this.
*/
int __cdecl good_sensor(int faction_id, int x, int y) {
    if (whose_territory(faction_id, x, y, NULL, false) != faction_id) {
        return false;
    }
    Map *tile = map_loc(x, y);
    if (tile->bit & BIT_FUNGUS && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF
        && !has_tech(Rules.tech_improve_fungus_sqr, faction_id)) {
        return false;
    }
    int reasons = 0;
    for (int i = 0; i < 25; i++) {
        int x_raw = x + RadiusOffsetX[i];
        int x_radius = (MapIsFlat & 1) ? x_raw
            : (x_raw < 0 ? x_raw + MapLongitudeBounds
               : (x_raw >= (int)MapLongitudeBounds ? x_raw - MapLongitudeBounds : x_raw));
        int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius)) {
            continue;
        }
        if (is_sensor(x_radius, y_radius)
            && whose_territory(faction_id, x_radius, y_radius, NULL, false) == faction_id) {
            return false;
        }
        if (i >= 9) {   // the inner ring only, from here down
            continue;
        }
        int base_id = base_at(x_radius, y_radius);
        if (base_id >= 0 && Bases[base_id].faction_id_current == faction_id) {
            reasons |= 2;
            BOOL base_covered = false;
            for (int j = 0; j < 25; j++) {
                int x_base = wrap_map_x(x_radius + RadiusOffsetX[j]);
                int y_base = y_radius + RadiusOffsetY[j];
                if (!on_map(x_base, y_base)) {
                    continue;
                }
                if (is_sensor(x_base, y_base)
                    && whose_territory(faction_id, x_base, y_base, NULL, false) == faction_id) {
                    base_covered = true;
                    break;
                }
            }
            if (!base_covered) {
                reasons |= 4;
            }
        }
        if (whose_territory(faction_id, x_radius, y_radius, NULL, false) != faction_id) {
            reasons |= 1;
        }
    }
    if (!reasons || bonus_at(x, y, 0) || base_who(x, y) >= 0) {
        return false;
    }
    Map *site = map_loc(x, y);
    uint32_t bit = site->bit;
    if (bit & (BIT_MONOLITH | BIT_CONDENSER | BIT_THERMAL_BORE)
        || bit & (BIT_MINE | BIT_SOLAR_TIDAL)) {
        return false;
    }
    if ((site->val3 >> 6) > 1 || !(site->climate & (RAINFALL_RAINY | RAINFALL_MOIST))) {
        return true;   // rocky or arid
    }
    return (bit & BIT_FUNGUS) && (site->climate >> 5) >= ALT_OCEAN_SHELF;
}


/*
Purpose: Return 1 immediately for a unity-bonus tile. Otherwise select the
         last pact or own base in the eight-neighbor movement table, stopping
         early on an own base. Return 1 if that base has an adjacent tile
         other than the input whose altitude is below shore and which either
         lacks fungus or lies below the ocean-shelf altitude; else return 0.

// ORIGINAL: 0x00565160 sub_565160 0x00565160-0x0056531A FILE BYTE_EXACT
// symbol    _sub_565160
// TRIED (seed): two-phase radius-table scan (find owning base, then scan
//            around it) with direct treaty-flags array indexing; reaches #11
//            before the second bonus_at-style helper call ordering
// size      442 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00592030
Return Value: 1 for the bonus or a qualifying neighbor, 0 otherwise.
Status: Coverage transcription - unclaimed body. The original mapping and
        seed evidence remain in git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00565160.cpp without a
        BYTE_EXACT claim. VehMoveDeltaX/Y provide the scan order, Bases
        provide the x/y/owner triple, PlayersData owns the pact flags, and
        Map owns the climate and fungus bits. The image calls bonus_at out
        of line; source inlining remains a separate measurement question.
*/
extern "C" int __cdecl sub_565160(int faction_id, int tile_x, int tile_y) {
    int found = -1;
    if (bonus_at(tile_x, tile_y, 0) == 3) {
        return 1;
    }
    for (int i = 0; i < 8; i++) {
        int x = wrap_map_x(tile_x + VehMoveDeltaX[i]);
        int y = tile_y + VehMoveDeltaY[i];
        if (on_map(x, y)) {
            int base_id = base_at(x, y);
            if (base_id >= 0) {
                int owner = Bases[base_id].faction_id_current;
                if (owner == faction_id
                    || PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT) {
                    found = base_id;
                    if (owner == faction_id) {
                        break;
                    }
                }
            }
        }
    }
    if (found >= 0) {
        int base_x = Bases[found].x;
        int base_y = Bases[found].y;
        for (int i = 0; i < 8; i++) {
            int x = wrap_map_x(base_x + VehMoveDeltaX[i]);
            int y = base_y + VehMoveDeltaY[i];
            if ((x != tile_x || y != tile_y) && on_map(x, y)) {
                Map *tile = map_loc(x, y);
                int altitude = tile->climate & ~0x1F;
                if (altitude < ALT_BIT_SHORE_LINE
                    && (!(tile->bit & BIT_FUNGUS) || altitude < ALT_BIT_OCEAN_SHELF)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
Purpose: Wrap an x coordinate on a round map, as can_terraform() wraps it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

The third private copy of this, after map.cpp's site_xrange() and game.cpp's
territory_xrange(), and for the same reason: the exported xrange() at 0048BEE0
tests the whole of MapIsFlat while can_terraform's base-radius census reads the
low BYTE and tests bit zero (`mov ebx, [94988Ch] / and ebx, 1`). The two agree
on the 0 and 1 the game stores there and disagree on everything else, and the
difference is in the original rather than in the transcription.
*/
static int terraform_xrange(int x) {
    if (!(MapIsFlat & 1)) {
        if (x >= 0) {
            if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        } else {
            x += MapLongitudeBounds;
        }
    }
    return x;
}

/*
Purpose: Decide which Former order, if any, should be issued on the specified tile.
// ORIGINAL: 0x00565320 ?can_terraform@@YAHHHHHH@Z 0x00565320-0x00565F1E
// LEVER: real-bitmask-calls the two facility checks in the forest-value block
//   are `has_fac_built_call` (base.h), not `has_fac_built`. The image CALLS
//   `bitmask` at 0x0050BA00 twice, from 0x00565449 and 0x00565487; the plain
//   spelling folds `MEASURED inline bitmask` into a shift/and and this body
//   then made 26 calls against the image's 28 - `call_diff` FEWER. With the
//   forwarder the counts agree, 28 against 28, and best similarity moved
//   0.168 -> 0.207 at `/c /O2 /Gy /GR- /Oy- /GX`. That is a call the image
//   really makes, so the fix stands on its own regardless of the tier.
// TRIED: not attempted to byte-exactness this pass - out of budget, and
//   said so rather than half-ground. Measured 2026-08-22 over all ten flag
//   sets: best 0.214 at `/c /O2 /Ob0 /Gy /GR- /Oy- /GX`, 15 of 1113
//   instructions. The gap is SIZE, not one lever: this tree compiles 1,566
//   instructions against the image's 1,113, a 40% excess, across 51 differing
//   runs, and that shape says the image leaves out-of-line what this tree
//   inlines somewhere in the accessor chain rather than that any one
//   expression is spelled wrong. The divergence opens at instruction 3, on
//   the `map_loc` index: the image reads MapLongitude at the fixed 0x0068FAF0
//   while this tree reads it as a linker symbol, and the base-in-tile test
//   arrives as `mov ecx, 1; test cl, bl` instead of a folded memory-operand
//   test. Anyone picking this up should start from `osmx calls` plus
//   `inline_candidates.py` on the accessors this body reaches, not from the
//   listing: with 453 instructions of excess, the first divergence is not
//   where the cause is.
// size      3070 bytes
// prototype int (__cdecl ?can_terraform@@YAHHHHHH@Z)(int factionID, int xCoord, int yCoord, int, int baseID)
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0 0x004E6E50 0x0050BA00 0x00592030 0x005B9F20 0x005BAB40
Return Value: The ORDER_* to issue, or ORDER_NONE for none
Status: Complete

THE NAME IS A PREDICATE AND THE FUNCTION IS NOT ONE. It returns an OrderType id from veh.h -
ORDER_FARM, ORDER_MINE, ORDER_SOLAR_COLLECTOR, ORDER_PLANT_FOREST, ORDER_REMOVE_FUNGUS,
ORDER_PLANT_FUNGUS, ORDER_TERRAFORM_UP - or ORDER_NONE. ORDER_x is TERRA_x + 4, the same offset
contribution() above already uses, and every returned constant is guarded by the matching
Terraforming[] prerequisite: 4 by TERRA_FARM's, 6 by TERRA_MINE's, 7 by TERRA_SOLAR_TIDAL's,
8 by TERRA_FOREST's, 14 by TERRA_REMOVE_FUNGUS's, 15 by TERRA_PLANT_FUNGUS's, and 20 by
terrain_avail(TERRA_RAISE_LAND). Console::give_hints reads the answer as a value rather than a
truth, comparing it against 4 at 00513311.

ORDER_FARM covers the soil enricher as well as the farm: in this game the enricher is built by
issuing "build farm" on a tile that already has one, so the two share order 4.

WHERE IT SPLITS. A tile that already holds somebody's base is refused outright. Everything after
that divides on the tile's own altitude at ALT_BIT_SHORE_LINE.

BELOW THE SHORE LINE. Deep water - anything under ALT_BIT_OCEAN_SHELF - is only ever raised.
On the shelf, fungus is removed if the faction's fungus yield is worth less than 4 and the human
player has asked for automatic fungus removal; bare shelf gets a kelp farm, and failing that a
mining platform or a tidal harness chosen by censusing the owning base's own 21-tile radius for
mines and harnesses already standing on water. A shelf tile that already has a farm and not yet
an enricher gets order 4 again.

ABOVE IT. Foreign territory without a pact refuses. Fungus is removed unless the base's
eco-damage-adjusted terrain value says the fungus is worth more. A monolith refuses; so does an
existing forest that is worth more than the terrain. What is left runs a cascade of
solar / mine / farm / enricher rules over crop yield, rockiness, rainfall, elevation, resource
bonus and the owning base's three planning bits, and then asks two closing questions: is a
forest worth more than the terrain, and after that is fungus worth more than the forest.

force_improve is the fourth parameter and it is the AI's "I already want this tile improved"
flag - base_terraform passes 1, Console::give_hints passes 0, and enemy_move passes
at_goal(faction_id, AI_GOAL_TERRAFORM_LAND, x, y). Each of its four reads turns a marginal tile
into a definite order.

Verification note: five kinds of unreachable code are transcribed as written rather than
deleted, and a mutation sweep will report them as survivors.

  1. elev CANNOT BE ZERO here. The land tail is entered only when (climate & 0xE0) >=
     ALT_BIT_SHORE_LINE, which is alt_at(x, y) >= 3, and elev is alt_at(x, y) - 2 floored at
     zero, so elev >= 1 always. That makes `!elev` at the solar rule and at the rolling-mine
     rule, `elev == 0` in the plant_value adjustment, and the `elev != 0` forest rejection all
     constant. The last one is the one that matters: it means a tile that already has a farm can
     never be forested, and the rainfall and is_human tests behind it are unreachable. True of
     the original at 00565A23, 00565ADC, 00565B64, 00565DC5 and 00565E4C, not just of this
     transcription.

  2. `rockiness < ROCKINESS_ROCKY` inside the BSTATE_UNK_2000 mine rule is reached only after
     `rockiness > ROCKINESS_ROLLING` has already been excluded (00565BB2), so it is constant.

  3. The owner nibble is masked to 0xF before `owner >= 0` is asked (0056535A), so that half of
     the base guard is constant.

  4. A DEAD COMPARISON, collapsed rather than transcribed. At 00565A7B the original computes
     `(crop >= has_farm) + 2` and branches on the result being zero; the value is 2 or 3, so the
     branch is never taken and both arms of the enclosing `if (crop)` reach order = ORDER_MINE.
     The block therefore is `if (bonus == 2 || rockiness == ROCKINESS_ROCKY) order = ORDER_MINE`,
     which is what is written below. The comparison has no side effect - only flags - so unlike
     a dead call there is nothing to preserve. Its shape shows the intent: has_farm is the RAW
     `bit & BIT_FARM`, 0 or 0x8000, so `crop >= has_farm` was meant to be the
     `crop >= (has_farm ? 2 : 1)` used three times elsewhere in this function.

  5. Two calls whose results the original throws away, kept because they are what it executes
     and because deleting one is a purity claim about another translation unit:
     bonus_at(x, y, 0) on the sea path (005655FD, EAX overwritten at 00565609) and
     has_tech(Terraforming[TERRA_FARM].preq_tech, faction_id) on the land path (005659F3, EAX
     overwritten at 005659F8 and the same question asked again for real at 00565BF1).

Bug note: both fungus-removal paths read Terraforming[TERRA_REMOVE_FUNGUS].preq_tech_SEA
(006919C4). On the sea path that is right. On the land path (0056596A) it should be preq_tech at
006919C0, and every other land/sea pair in the function is correctly discriminated. Stock
terraform.txt gives Remove Fungus the same technology for both, so it is invisible in a default
game and visible in a mod that splits them. Transcribed as written.
*/
#pragma optimize("y", off)
int __cdecl can_terraform(int faction_id, int x, int y, int force_improve, int base_id) {
    Map *tile = map_loc(x, y);
    if (map_loc(x, y)->bit & BIT_BASE_IN_TILE) {
        int owner = (int)owner_at(x, y);
        if (owner < 8 && owner >= 0) {
            return ORDER_NONE;
        }
    }
    uint32_t bit = tile->bit;
    // Fungus in deep water does not count: the altitude is the tile's own.
    BOOL has_fungus = (bit & BIT_FUNGUS)
        && static_cast<int>(static_cast<uint32_t>(tile->climate) & ~0x1Fu)
            >= ALT_BIT_OCEAN_SHELF;
    uint32_t has_mine = bit & BIT_MINE;
    uint32_t has_solar = bit & BIT_SOLAR_TIDAL;
    uint32_t has_farm = bit & BIT_FARM;
    uint32_t has_soil_enricher = bit & BIT_SOIL_ENRICHER;
    uint32_t has_forest = bit & BIT_FOREST;
    BOOL is_special = (bit & (BIT_CONDENSER | BIT_ECH_MIRROR | BIT_THERMAL_BORE))
        || (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO)
            && !(tile->bit2 & 0xFF000000));  // landmark code zero, ie. the volcano's own tile
    int fungus_value = PlayersData[faction_id].tech_fungus_[RSC_NUTRIENTS]
        + PlayersData[faction_id].tech_fungus_[RSC_MINERALS]
        + PlayersData[faction_id].tech_fungus_[RSC_ENERGY];
    int forest_value = ResourceInfo[RSCINFO_FOREST_SQ].nutrients
        + ResourceInfo[RSCINFO_FOREST_SQ].minerals
        + ResourceInfo[RSCINFO_FOREST_SQ].energy;
    if (base_id >= 0) {
        // REAL bitmask() CALLS, not the folded shift/and: the image calls
        // 0x0050BA00 twice, at 0x00565449 and 0x00565487, which is what
        // `has_fac_built_call` (base.h) is for. Plain `has_fac_built` inlines
        // `MEASURED inline bitmask` and this body then makes 26 calls against
        // the image's 28.
        if (has_fac(FAC_TREE_FARM, base_id, 0)) {
            forest_value++;
        }
        if (has_fac(FAC_HYBRID_FOREST, base_id, 0)) {
            forest_value += 2;
        }
    }
    if (PlayersData[faction_id].ai_growth && !PlayersData[faction_id].ai_power
        && !PlayersData[faction_id].ai_wealth && !PlayersData[faction_id].ai_tech
        && PlayersData[faction_id].ai_fight < 0 && !is_human(faction_id)) {
        forest_value++;   // a growth-only AI plan likes forest one point more
    }
    // 56550A..56551E: resolve this phase after the facility queries; the
    // land arm retains this same climate byte and tile for rainfall/rockiness.
    Map *const terrain_tile = map_loc(x, y);
    const uint32_t terrain_climate = terrain_tile->climate;
    int altitude = static_cast<int>(terrain_climate & ~0x1F);
    if (altitude < ALT_BIT_SHORE_LINE) {
        // ---- water ---------------------------------------------------------
        if (has_fungus) {
            if (fungus_value >= 4) {
                return ORDER_NONE;
            }
            if (is_human(faction_id)
                && !(GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AUTO_FORMER_REMOVE_FUNGUS)) {
                return ORDER_NONE;
            }
            return has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[1], faction_id)
                ? ORDER_REMOVE_FUNGUS : ORDER_NONE;
        }
        if (altitude < ALT_BIT_OCEAN_SHELF) {
            return terrain_avail(TERRA_RAISE_LAND, true, faction_id)
                ? ORDER_TERRAFORM_UP : ORDER_NONE;
        }
        if (terrain_tile->bit & BIT_THERMAL_BORE) {
            return ORDER_NONE;
        }
        if (!has_farm
            && has_tech(Terraforming[TERRA_FARM].prerequisite_tech[1], faction_id)) {
            return ORDER_FARM;   // kelp farm
        }
        if (!has_solar && !has_mine && !is_special) {
            bonus_at(x, y, 0);   // result discarded by the original, see the note above
            if (base_id >= 0) {
                if ((Bases[base_id].state & BSTATE_UNK_2000)
                    && has_tech(Terraforming[TERRA_MINE].prerequisite_tech[1], faction_id)) {
                    return ORDER_MINE;
                }
                if ((Bases[base_id].state & BSTATE_UNK_1000)
                    && has_tech(Terraforming[TERRA_SOLAR_TIDAL].prerequisite_tech[1], faction_id)) {
                    return ORDER_SOLAR_COLLECTOR;
                }
                if (((Bases[base_id].state & BSTATE_UNK_4000)
                        || Bases[base_id].nutrient_surplus < 2)
                    && has_tech(Terraforming[TERRA_SOLAR_TIDAL].prerequisite_tech[1], faction_id)
                    && Rules.tgl_nutrient_effect_with_mine < 0) {
                    return ORDER_SOLAR_COLLECTOR;
                }
                // Census the base's own radius for water improvements already standing.
                int mine_count = (Rules.tgl_nutrient_effect_with_mine < 0) ? 1 : 0;
                int solar_count = 0;
                int base_x = Bases[base_id].x;
                int base_y = Bases[base_id].y;
                for (int i = 0; i < 21; i++) {
                    int x_radius = terraform_xrange(base_x + RadiusOffsetX[i]);
                    int y_radius = base_y + RadiusOffsetY[i];
                    if (!on_map(x_radius, y_radius)) {
                        continue;
                    }
                    Map *radius_tile = map_loc(x_radius, y_radius);
                    if ((radius_tile->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
                        continue;   // land
                    }
                    if (radius_tile->bit & BIT_MINE) {
                        mine_count++;
                    }
                    if (radius_tile->bit & BIT_SOLAR_TIDAL) {
                        solar_count++;
                    }
                }
                if (mine_count
                        + (has_tech(Terraforming[TERRA_MINE].prerequisite_tech[1], faction_id) ? 0 : 1)
                        > solar_count
                    && has_tech(Terraforming[TERRA_SOLAR_TIDAL].prerequisite_tech[1], faction_id)) {
                    return ORDER_SOLAR_COLLECTOR;
                }
            }
            if (has_tech(Terraforming[TERRA_MINE].prerequisite_tech[1], faction_id)
                && (Rules.tgl_nutrient_effect_with_mine >= 0
                    || (base_id >= 0 && Bases[base_id].nutrient_surplus > 1
                        && !(Bases[base_id].state & BSTATE_UNK_4000)))) {
                return ORDER_MINE;
            }
        }
        if (has_farm && !has_soil_enricher
            && has_tech(Terraforming[TERRA_SOIL_ENR].prerequisite_tech[1], faction_id)) {
            return ORDER_FARM;   // upgrade the kelp farm to an enricher
        }
        return ORDER_NONE;
    }
    // ---- land --------------------------------------------------------------
    // Rainfall runs arid, moist, rainy.
    int rainfall = (terrain_climate >> 3) & 3;
    int rockiness = terrain_tile->val3 >> 6;           // ROCKINESS_FLAT / ROLLING / ROCKY
    int order = ORDER_NONE;
    int crop = crop_yield(faction_id, -1, x, y, 0);
    // 565872/565880: elevation retains the same pre-crop climate snapshot.
    int elev = static_cast<int>(terrain_climate >> 5) - 2;
    if (elev < 0) {
        elev = 0;
    }
    // The bonus runs none, nutrient, mineral, energy.
    int bonus = bonus_at(x, y, 0);
    int value = elev + rainfall + rockiness + 1;
    if (has_tech(Terraforming[TERRA_SOIL_ENR].prerequisite_tech[0], faction_id) && has_farm) {
        value++;
    }
    int territory = whose_territory(faction_id, x, y, NULL, false);
    if (territory >= 0 && territory != faction_id
        && !(PlayersData[faction_id].diplo_treaties[territory] & DTREATY_PACT)) {
        return ORDER_NONE;
    }
    if (has_fungus) {
        if (base_id >= 0 && Bases[base_id].eco_damage / 2 + value <= fungus_value) {
            return ORDER_NONE;   // the fungus is worth more than the terrain under it
        }
        if (is_human(faction_id)
            && !(GameDataStateStorage.fields_.game_more_preferences_ & MPREF_AUTO_FORMER_REMOVE_FUNGUS)) {
            return ORDER_NONE;
        }
        return has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[1], faction_id)
            ? ORDER_REMOVE_FUNGUS : ORDER_NONE;
    }
    // 565987..5659A4: read the tile again after the intervening queries.
    if (bit_at(x, y) & BIT_MONOLITH) {
        return ORDER_NONE;
    }
    if (has_forest && value <= forest_value) {
        return ORDER_NONE;
    }
    BOOL mine_tech = has_tech(Terraforming[TERRA_MINE].prerequisite_tech[0], faction_id);
    BOOL solar_tech = has_tech(Terraforming[TERRA_SOLAR_TIDAL].prerequisite_tech[0], faction_id);
    has_tech(Terraforming[TERRA_FARM].prerequisite_tech[0], faction_id);   // result discarded, see above
    if (!has_mine && !has_solar && !is_special) {
        if (solar_tech && elev
            && (((rockiness == ROCKINESS_ROLLING || elev >= 2)
                    && crop >= (has_farm ? 2 : 1))
                || force_improve)) {
            order = ORDER_SOLAR_COLLECTOR;
        }
        if (mine_tech) {
            if (bonus == 2 /* mineral */ || rockiness == ROCKINESS_ROCKY) {
                order = ORDER_MINE;
            }
            BOOL rolling_rule = false;
            BOOL farm_rule = false;
            if (bonus == 2 && rockiness == ROCKINESS_ROCKY) {
                farm_rule = true;
            } else if (rockiness == ROCKINESS_ROCKY
                       || Rules.tgl_nutrient_effect_with_mine >= 0) {
                rolling_rule = true;
            } else {
                int worked = crop + (has_farm ? 0 : 1);
                if (worked <= 1) {
                    rolling_rule = true;
                } else if (rockiness == ROCKINESS_FLAT && worked > 2) {
                    farm_rule = true;
                }
            }
            if (rolling_rule) {
                if (rockiness == ROCKINESS_ROLLING && !elev) {
                    order = ORDER_MINE;
                }
                if (bonus == 3 /* energy */) {
                    order = ORDER_MINE;
                }
                farm_rule = true;
            }
            if (farm_rule && force_improve && has_farm && !order) {
                order = ORDER_MINE;
            }
        }
        if (force_improve && has_farm && !order && elev && solar_tech) {
            order = ORDER_SOLAR_COLLECTOR;
        }
        if (base_id >= 0) {
            uint32_t state = Bases[base_id].state;
            if ((state & BSTATE_UNK_1000) && solar_tech && elev
                && (crop >= (has_farm ? 2 : 1) || elev > 1)) {
                order = ORDER_SOLAR_COLLECTOR;
            }
            if ((state & BSTATE_UNK_2000) && mine_tech) {
                if (crop >= (has_farm ? 3 : 2)) {
                    order = ORDER_MINE;
                }
                BOOL wants_mine = rockiness > ROCKINESS_ROLLING || bonus == 2
                    || (rainfall != 0 && rockiness < ROCKINESS_ROCKY && elev < 2);
                if (wants_mine && (has_farm || crop >= 2)) {
                    order = ORDER_MINE;
                }
            }
        }
    }
    if (has_tech(Terraforming[TERRA_FARM].prerequisite_tech[0], faction_id) && !has_farm
        && rockiness < ROCKINESS_ROCKY) {
        // 565C15..565C59: one fresh tile and bit snapshot for these guards.
        Map *const farm_tile = map_loc(x, y);
        const uint32_t farm_bits = farm_tile->bit;
        if (!(farm_bits & BIT_THERMAL_BORE)
            && (farm_tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) != BIT2_VOLCANO
            && !(farm_bits & BIT_ECH_MIRROR)
            && (crop < 2 || has_tech(Rules.tech_three_nutrients_sqr, faction_id)
                || bonus == 1 /* nutrient */)) {
            if (has_solar || has_mine) {
                order = ORDER_FARM;
            }
            BOOL forced_farm = false;
            if (base_id >= 0 && (Bases[base_id].state & BSTATE_UNK_4000)) {
                order = ORDER_FARM;
            } else if (order == ORDER_SOLAR_COLLECTOR) {
                if (crop < 2) {
                    order = ORDER_FARM;
                }
            } else if (order == ORDER_NONE) {
                forced_farm = true;
            }
            if (forced_farm || rainfall < 2) {
                order = ORDER_FARM;
            }
        }
    }
    if (has_tech(Terraforming[TERRA_SOIL_ENR].prerequisite_tech[0], faction_id) && has_farm
        && !has_soil_enricher) {
        BOOL enrich = false;
        if (crop > 1
            && (crop != 2 || has_tech(Rules.tech_three_nutrients_sqr, faction_id))) {
            if (has_solar || has_mine) {
                order = ORDER_FARM;
            }
            if (base_id >= 0 && (Bases[base_id].state & BSTATE_UNK_4000)) {
                enrich = true;
            }
        }
        if (!enrich && order == ORDER_NONE && (force_improve || crop > 1)) {
            enrich = true;
        }
        if (enrich) {
            order = ORDER_FARM;
        }
    }
    // What the terrain would be worth if it were planted, scored separately from `value`.
    int plant_value = elev + rainfall + rockiness + 1;
    if (has_tech(Terraforming[TERRA_SOIL_ENR].prerequisite_tech[0], faction_id) && has_farm) {
        plant_value++;
    }
    if (base_id >= 0) {
        uint32_t state = Bases[base_id].state;
        if (state & BSTATE_UNK_4000) {
            if (rainfall > 1) {
                plant_value++;
            }
            if (rainfall == 0) {
                plant_value--;
            }
        }
        if (state & BSTATE_UNK_1000) {
            if (elev > 1) {
                plant_value++;
            }
            if (elev == 0) {
                plant_value--;
            }
        }
        if (state & BSTATE_UNK_2000) {
            if (rockiness == ROCKINESS_FLAT) {
                plant_value--;
            }
        }
        if (!(state & (BSTATE_UNK_2000 | BSTATE_UNK_4000))) {
            if (has_solar) {
                plant_value++;
            }
        }
    }
    if (rainfall && elev && rockiness == ROCKINESS_ROLLING && GameDataStateStorage.fields_.turn_current_num_ < 50) {
        plant_value++;
    }
    int result = order;
    if (has_tech(Terraforming[TERRA_FOREST].prerequisite_tech[0], faction_id)
        && rockiness < ROCKINESS_ROCKY && !has_mine && !has_forest
        && (!has_farm || (!elev && rainfall < 2 && !is_human(faction_id)))
        && !is_special
        // 565E6A..565E90: a fresh lookup at the final forest predicate.
        && (bit2_at(x, y) & (BIT2_UNK_80000000 | BIT2_VOLCANO)) != BIT2_VOLCANO
        && plant_value < forest_value
        && (!is_human(faction_id)
            || (faction_id == ConsoleGlobal.faction_id_
                && (GameDataStateStorage.fields_.game_preferences_ & PREF_AUTO_FORMER_PLANT_FORESTS)))) {
        result = ORDER_PLANT_FOREST;
    }
    if (has_tech(Terraforming[TERRA_PLANT_FUNGUS].prerequisite_tech[0], faction_id)
        && !is_human(faction_id)) {
        if (rockiness == ROCKINESS_ROCKY) {
            plant_value += has_mine ? 2 : 1;
        }
        if (!is_special) {
            plant_value++;
            if (plant_value < fungus_value) {
                result = ORDER_PLANT_FUNGUS;
            }
        }
    }
    return result;
}
#pragma optimize("", on)

/*
Purpose: Weight the raw odds of an attack by how badly the faction's AI wants that particular
         stack dead. The odds arrive already computed by battle_fight; everything here is a
         strategic multiplier on top of them.
// ORIGINAL: 0x00565F20 ?compute_odds@@YAHHHHHH@Z 0x00565F20-0x005665CC
// TRIED: call structure already matches exactly (`osmx calls --all`'s 15
//            call sites resolve to the same 5 targets as the image's own
//            `calls` list: base_at, at_goal, stack_check x9, has_abil,
//            veh_at x2), so the 123/572 (0.540 best, /O2 /Gy /GR- /Oy- /GX)
//            gap is register/control-flow noise across a 1708-byte function,
//            not a missing inline or wrong callee. Not chased further in
//            this pass - too large for the budget available here.
// size      1708 bytes
// prototype int (__cdecl ?compute_odds@@YAHHHHHH@Z)(int, int factionID, int vehID1, int vehID2, int)
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00579CC0 0x005B9580 0x005BF1F0 0x005BFE90
Return Value: Weighted attack odds
Status: Complete

THE BASIS is what the target is worth divided by what the attacker costs. Out in the field the
defender's own prototype cost counts in full and the rest of its stack counts at ONE FIFTH, so a
stack of ten is worth roughly three of them; an Alien Artifact anywhere in that stack adds half
again. On a base tile the stack is ignored and only the defender's own cost is used, because the
garrison gets its own rule below. Either way the result is divided by the attacker's prototype
cost, which is what makes this a cost-effectiveness number rather than a value.

EVERYTHING AFTER THAT IS A DOUBLING, and they compose:

  x2   the defender's plan is PLAN_NAVAL_TRANSPORT or later, i.e. it cannot fight back
  x3/2 an offensive or combat LAND attacker, on a region this faction has no base plan for
  x2   the attacker's domain matches the tile - a sea unit over water or a land unit over land,
       never an air unit - but only for a base tile or a declared AI_GOAL_ATTACK goal
  x2   a base tile whose garrison is one real combat unit or fewer (total minus the ones with no
       offense rating); mutually exclusive with the domain bonus, which returns first
  x4   the unit carries VFLAG_IS_OBJECTIVE

then one of two branches on the attacker's triad.

AN AIR ATTACKER additionally gets x2 for a missile aimed at open water away from a base; and, if
the stack is TERRAFORMING, x2 for not being a missile, x2 over water when the faction carries
player flag 0x80000, and x2 again for each of the two terraform orders. On a base tile it looks
at the eight neighbours for a friendly stack containing LAND units - something already in place
to take the base - and doubles once for the first one it finds. Then, if the garrison is no
larger than a tenth of the faction's active count of this attacker's prototype plus one, it
scans this faction's sixty-four prototype slots for an active, typed, in-service design with
ABL_DROP_POD, and doubles for it - twice if the attacker is a missile.

ANY OTHER ATTACKER on a base tile counts LAND units across all eight neighbouring friendly
stacks and doubles once per whole garrison's worth of them, stopping at a million.

THE TAIL undoes some of it. An air-superiority attacker aimed at something that is neither
airborne nor armed is worth HALF; a plain PLAN_OFFENSIVE air unit that is not a missile is worth
a THIRD, or two thirds where the region's base plan is 2, because it is expected to be doing
something else.

Bug notes, reproduced rather than corrected:

  - veh_at() answers -1 for an empty tile. Both original neighbour loops read
    byte[52*id + 0x952836] before comparing the faction (0x566308/0x566498).
    For -1 that is 0x952802, the low byte of Reactor[0].power at offset 0xA.
    The source arrays have independent storage, so indexing before Vehs does
    not preserve this alias. Read the named reactor field for exactly -1;
    retain other indices, the comparison, and stack_check(-1) when it agrees.
    This preserves the original defective lookup, not a safe fallback.
  - The division by the attacker's prototype cost is a bare idiv with no zero guard.
  - The two terraform-order tests are two independent ifs in the original, not an else-if. They
    are mutually exclusive in practice because `order` holds one value, so this is a shape
    difference only, and it is transcribed as written.
  - The `while` below terminates only because `defenders` is known nonzero; the guard is the
    original's, not an addition.

The x wrap is course_xrange() rather than xrange(), for the one bit that separates them: the
original reads the low BYTE of MapIsFlat and tests bit zero (`mov cl, byte ptr [94988Ch] /
test cl, 1`), exactly as set_course and reset_territory do.

Verification note: eighteen mutants survive the sweep and every one is an equivalence that can be
read off another recovered body.

SIXTEEN ARE ARGUMENTS stack_check DOES NOT LOOK AT. Its recovered switch takes cond1, cond2 and
cond3 but each arm reads only some of them, and the -1 in an unread slot is therefore free:
type 1, type 16 and type 19 read cond1 alone, so their cond2 and cond3 are dead - two survivors
each at the stack cost, the two thin-garrison censuses, the drop-pod censuses and the defender
count; type 2 and type 3 read cond1 and cond2, so their cond3 is dead - one survivor each at the
artifact test, the terraforming test and the two neighbour counts. The original passes -1 in all
of them and so does this, because the call is the same call.

THE OTHER TWO ARE THE NINTH RADIUSBASE ENTRY. Both neighbour loops stop at eight, and mutating
`i < 8` to `i <= 8` adds entry eight, which is (0,0) - the tile under attack. The stack there is
the defender's, and a tile in this game holds one faction's units, so
Vehs[veh_at(x_def, y_def)].faction_id is the defender's faction and the loop's
`!= faction_id` skips it for any caller that is attacking rather than reinforcing. Every other
tile of the ring is separately covered, including entry 0, which the first sweep found untested.

A NOTE ON THE SWEEP ITSELF, because it cost a pass to work out: the mutation harness perturbs
digits inside trailing comments as well as in code. A `// RadiusBase entry 8 is (0,0) ...` note
on the `for` line produced four unkillable survivors that read exactly like real coverage holes.
The comment is now on its own line and carries no digits.
*/
int __cdecl compute_odds(int odds, int faction_id, int veh_id_atk, int veh_id_def, int base_id) {
    const int x_def = Vehs[veh_id_def].x;
    const int y_def = Vehs[veh_id_def].y;
    const int triad_atk = Chassis[VehPrototypes[Vehs[veh_id_atk].proto_id].chassis_id].triad;
    const int plan_atk = VehPrototypes[Vehs[veh_id_atk].proto_id].plan;
    if (base_id < -1) {
        // Strictly below -1 asks for the lookup; -1 asserts there is no base here.
        base_id = base_at(x_def, y_def);
    }
    const int region_plan =
        PlayersData[faction_id].region_base_plan[map_loc(x_def, y_def)->region];

    int value;
    if (base_id < 0) {
        const int cost_def = VehPrototypes[Vehs[veh_id_def].proto_id].cost;
        // The rest of the stack counts at one fifth; the defender itself counts in full.
        value = ((stack_check(veh_id_def, 7, -1, -1, -1) - cost_def) / 5 + cost_def) * odds;
        if (stack_check(veh_id_def, 2, PLAN_ALIEN_ARTIFACT, -1, -1)) {
            value = value * 3 / 2;
        }
    } else {
        value = VehPrototypes[Vehs[veh_id_def].proto_id].cost * odds;
    }
    value /= VehPrototypes[Vehs[veh_id_atk].proto_id].cost;

    if (VehPrototypes[Vehs[veh_id_def].proto_id].plan >= PLAN_NAVAL_TRANSPORT) {
        value *= 2;   // it cannot fight back
    }
    if (plan_atk <= PLAN_COMBAT && !region_plan && triad_atk == TRIAD_LAND) {
        value = value * 3 / 2;
    }
    if (base_id >= 0 || at_goal(faction_id, AI_GOAL_ATTACK, x_def, y_def)) {
        if ((triad_atk == TRIAD_SEA) == (is_ocean(x_def, y_def) != 0) && triad_atk != TRIAD_AIR) {
            value *= 2;   // the attacker's own domain, and this returns before the next rule
        } else if (base_id >= 0) {
            const int noncombat = stack_check(veh_id_def, 19, -1, -1, -1);
            const int total = stack_check(veh_id_def, 1, -1, -1, -1);
            if (total - noncombat <= 1) {
                value *= 2;   // one real defender or none
            }
        }
    }
    if (Vehs[veh_id_def].flags & VFLAG_IS_OBJECTIVE) {
        value *= 4;
    }

    if (triad_atk == TRIAD_AIR) {
        if (is_proto_missile(Vehs[veh_id_atk].proto_id)
            && is_ocean(x_def, y_def) && base_id < 0) {
            value *= 2;
        }
        if (stack_check(veh_id_def, 2, PLAN_TERRAFORMING, -1, -1)) {
            if (!is_proto_missile(Vehs[veh_id_atk].proto_id)) {
                value *= 2;
            }
            if (is_ocean(x_def, y_def) && (PlayersData[faction_id].flags & 0x80000)) {
                value *= 2;   // unnamed player flag; PlayerFlagsBitfield skips this bit
            }
            // Two separate tests in the original, not an else-if; `order` cannot be both.
            if (Vehs[veh_id_def].order == ORDER_TERRAFORM_UP) {
                value *= 2;
            }
            if (Vehs[veh_id_def].order == ORDER_TERRAFORM_DOWN) {
                value *= 2;
            }
        }
        if (base_id >= 0) {
            for (int i = 0; i < 8; i++) {
                // The ninth RadiusBase entry is the tile itself and is never reached.
                const int x_radius = course_xrange(x_def + VehMoveDeltaX[i]);
                const int y_radius = y_def + VehMoveDeltaY[i];
                if (!on_map(x_radius, y_radius)) {
                    continue;
                }
                const int veh_id = veh_at(x_radius, y_radius);
                // An empty tile (-1) reads before Vehs[], as the original does.
                if (Vehs[veh_id].faction_id != faction_id) {
                    continue;
                }
                if (stack_check(veh_id, 3, TRIAD_LAND, -1, -1)) {
                    value *= 2;   // somebody is already on the ground next to it
                    break;
                }
            }
            const int noncombat = stack_check(veh_id_def, 19, -1, -1, -1);
            const int total = stack_check(veh_id_def, 1, -1, -1, -1);
            const int proto_atk = Vehs[veh_id_atk].proto_id;
            if (total - noncombat
                <= PlayersData[faction_id].proto_id_active[proto_atk] / 10 + 1) {
                for (int i = 0; i < MaxVehProtoFactionNum; i++) {
                    const int proto_id = faction_id * MaxVehProtoFactionNum + i;
                    if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)) {
                        continue;
                    }
                    if (!(VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE)) {
                        continue;
                    }
                    if (!PlayersData[faction_id].proto_id_active[proto_id]) {
                        continue;
                    }
                    if (!has_abil(proto_id, ABL_DROP_POD)) {
                        continue;
                    }
                    value *= 2;
                    if (is_proto_missile(Vehs[veh_id_atk].proto_id)) {
                        value *= 2;
                    }
                    break;
                }
            }
        }
    } else if (base_id >= 0) {
        int attackers = 0;
        for (int i = 0; i < 8; i++) {
            const int x_radius = course_xrange(x_def + VehMoveDeltaX[i]);
            const int y_radius = y_def + VehMoveDeltaY[i];
            if (!on_map(x_radius, y_radius)) {
                continue;
            }
            const int veh_id = veh_at(x_radius, y_radius);
            // The same unchecked -1 read as the air loop.
            if (Vehs[veh_id].faction_id != faction_id) {
                continue;
            }
            attackers += stack_check(veh_id, 3, TRIAD_LAND, -1, -1);
        }
        const int defenders = stack_check(veh_id_def, 16, -1, -1, -1);
        if (defenders) {   // without this the subtraction below never terminates
            while (attackers > defenders && value < 1000000) {
                value *= 2;
                attackers -= defenders;
            }
        }
    }

    if (plan_atk == PLAN_AIR_SUPERIORITY) {
        const int proto_def = Vehs[veh_id_def].proto_id;
        if (Chassis[VehPrototypes[proto_def].chassis_id].triad == TRIAD_AIR) {
            return value;   // an interceptor doing its job
        }
        if (!Weapon[VehPrototypes[proto_def].weapon_id].offense_rating) {
            return value;   // nothing there to intercept anything
        }
        return value / 2;
    }
    if (triad_atk == TRIAD_AIR && plan_atk == PLAN_OFFENSIVE
        && !is_proto_missile(Vehs[veh_id_atk].proto_id)) {
        if (region_plan != 2) {
            return value / 3;
        }
        return value * 2 / 3;
    }
    return value;
}

/*
Purpose: Choose which base a native life form standing on the specified tile should head for.
// ORIGINAL: 0x005665D0 ?alien_base@@YAHHHH@Z 0x005665D0-0x00566802
// size      562 bytes
// prototype int (__cdecl ?alien_base@@YAHHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050DE50 0x005B9580 0x005BFE90 0x00644F3A
Return Value: Base id of the most attractive target, or -1 when there is none
Status: Complete

WHAT THE SCORE MEANS. The comparison variable is a cost, not a merit: the loop keeps the LOWEST
value, seeded at 9999, and the winner is published in BaseFindDist for the caller. The cost is the
radiating distance from the unit to the base, scaled by 32 and then divided by that base's own
mineral plus energy intake plus 32, so a productive base pulls a worm from further away. Three
adjustments follow, each of which the original applies as a whole doubling or halving:

  - an Alien Artifact standing in the base halves the cost. stack_check type 2 counts units in the
    tile's stack whose prototype PLAN is PLAN_ALIEN_ARTIFACT, so this is literally "something is
    sitting there that Planet wants back".
  - the unit's own home base halves it again. Native life gets a home base when a faction breeds
    it, and this is what makes a bred worm drift back toward the base that made it.
  - a base whose owner runs a positive PLANET rating and has no eco damage DOUBLES it. That is the
    in-game promise that a green faction is left alone, expressed as a cost the worm has to pay.

Verification note: the third test compares Veh+0x26 against the base's owner. veh.h names that
byte order_auto_type, and on this path the original is reading it as a faction id rather than as
an order type - base.cpp reads the same byte as a resource id for convoys, so the field is already
known to be multi-purpose. Transcribed as the original reads it rather than corrected.

REACHABILITY. The region test only applies to a native standing on LAND. Water bodies are numbered
from MaxRegionLandNum upward (see the region notes in map.h), so a unit already at sea reaches
every base and the test is skipped - which is the whole reason it is written as a bound and not as
an equality. On land, the base has to share the unit's region, except for a Sealurk - prototype
14, the only native the loop names - which may also take a base that merely touches the region it
is standing in, and base_on_sea answers that.

Verification note: stack_check's fifth argument survives a mutation sweep and always will. Type 2
reads cond1 and cond2 and never looks at cond3, so the -1 the original pushes for it cannot be
observed by any fixture. The fourth argument is observable and is covered - the artifact unit in
the fixture carries a non-zero faction precisely so that turning that -1 into 0 changes the count.

The original inlines region_at (0x00500220), x_dist (0x00579790) and the two-argument vector_dist
(0x004F8090); each inlined copy is instruction-for-instruction the standalone body, so they are
called here. One consequence is visible in the disassembly and worth recording: the inlined
map_loc halves x with SAR, while the exported map_loc that region_at reaches takes uint32_t and
would halve it with SHR. The two agree on every non-negative x, and alien_move at 0x005668A0 -
the only caller - passes a unit's own sign-extended coordinates.

// LEVER: FEWER (6 calls vs image's 7) - the image calls `abs()` FOUR times
//        here and never calls vector_dist(int,int) itself (0x4F8090 is not
//        in its `calls` list, despite being a real, `callers 5`-elsewhere
//        function). Open-coded the `vector_dist(x_dist(...), abs(...))`
//        call as the abs()/largest/smallest expansion vector_dist's own
//        body performs - the same pattern base.cpp's black_market and
//        base_find already use for the 4-arg form. Call count now agrees
//        (0 disagree); best flag set's similarity 0.782 -> 0.883
//        (/O2 /Oi- /GR- /Oy- /GX).
*/
int __cdecl alien_base(int veh_id, int x, int y) {
    int best_value = 9999;
    BaseFindDist = 9999;
    int base_id_best = -1;
    const int region = region_at(x, y);
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Vehs[veh_id].proto_id == BSC_SEALURK) {
            if (region < MaxRegionLandNum && region != (int)region_at(Bases[base_id].x, Bases[base_id].y)
                && !base_on_sea(base_id, region)) {
                continue;
            }
        } else if (region < MaxRegionLandNum && region != (int)region_at(Bases[base_id].x, Bases[base_id].y)) {
            continue;
        }
        const int base_y = Bases[base_id].y;
        int value = vector_dist(x_dist(x, Bases[base_id].x), abs(y - base_y)) * 32
            / (Bases[base_id].intake_2[RSC_MINERALS] + Bases[base_id].intake_2[RSC_ENERGY] + 32);
        const int veh_id_at = veh_at(Bases[base_id].x, Bases[base_id].y);
        if (stack_check(veh_id_at, 2, PLAN_ALIEN_ARTIFACT, -1, -1)) {
            value /= 2;
        }
        if (base_id == Vehs[veh_id].home_base_id) {
            value /= 2;
        }
        int owner = Bases[base_id].faction_id_current;
        if (Vehs[veh_id].order_auto_type != owner
            && PlayersData[owner].soc_effect_active.values_[SocialEffect::PLANET] > 0
            && !Bases[base_id].eco_damage) {
            value *= 2;
        }
        if (value <= best_value) {
            best_value = value;
            base_id_best = base_id;
        }
    }
    if (base_id_best >= 0) {
        BaseFindDist = best_value;
    }
    return base_id_best;
}

// Inlined: alien_move's halved x_dist + |dy| reach.
static inline int alien_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b) {
    int x_distance = x_dist(x_point_a, x_point_b);
    int y_distance = abs(y_point_a - y_point_b);
    return (x_distance + y_distance) >> 1;
}

// ORIGINAL: 0x00566810 ?alien_move@@YAHH@Z 0x00566810-0x005678B9
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/unrecovered/00566810.cpp", "recovery/tasks/alien-buster-owner-oct5.json", "pinned full native instruction/argument and directed-field review; compiler-observed current function symbol", "PlayerData sites[14].base_id alias: native addresses and compiler offsetof witness; original policy order checked by actual-product controls"], "reason": "Full vehicle-owner semantic recovery with actual Base, PlayerData, vehicle, map and callback relationships. Preserve original no-base adjacent-storage reads, native radius exits and uninitialized launch scratch; byte restoration and runtime fidelity remain unfinished."}
// symbol    ?alien_move@@YAHH@Z
// TRIED: near-direct Ghidra transcription (clean decompile, no extraout/jumptable warnings). Per-vehicle/per-target table fields were unified onto two byte offsets (vehOff = a1*0x34, iVar18v = target*0x134) since every DAT_xxx family the disassembly touches shares that stride; a few element widths (e.g. DAT_0096da3c as byte vs int) were guessed rather than confirmed against the raw bytes.
int __cdecl alien_move(int veh_id) {
    int dir_selected = -1;
    if (!veh_moves(veh_id)) {
        return 0;
    }
    if (Vehs[veh_id].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) {
        veh_skip(veh_id);
        return 0;
    }
    if (Vehs[veh_id].proto_id == BSC_UNITY_SCOUT_CHOPPER) {
        veh_skip(veh_id);
        return 0;
    }
    const int x = Vehs[veh_id].x;
    const int y = Vehs[veh_id].y;
    int base_id = alien_base(veh_id, x, y);
    int faction_id;
    if (base_id >= 0) {
        faction_id = Bases[base_id].faction_id_current;
    } else {
        if (!zoc_any(x, y, 0)) {
            kill(veh_id);
            return 1;
        }
        faction_id = 0;
        BaseFindDist = 99;
    }
    // With no base this reads Bases[-1], as the original does.
    int region = region_at(Bases[base_id].x, Bases[base_id].y);
    Vehs[veh_id].terraforming_turns++;
    if (veh_cargo(veh_id) && stack_check(veh_id, 3, TRIAD_LAND, -1, -1)) {
        if (Vehs[veh_id].terraforming_turns > 24) {
            kill(veh_id);
            return 1;
        }
        for (int i = 0; i < 8; i++) {
            int x2 = wrap_map_x(x + VehMoveDeltaX[i]);
            int y2 = y + VehMoveDeltaY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            if (veh_who(x2, y2) > 0) {
                if (is_ocean(x2, y2)) {
                    dir_selected = i;
                    goto move_direction;
                }
                continue;
            }
            if (is_ocean(x2, y2) || region_at(x2, y2) != region) {
                continue;
            }
            if (base_id >= 0 && alien_dist(x2, y2, Bases[base_id].x, Bases[base_id].y) > 6) {
                continue;
            }
            {
                uint8_t visibility = map_loc(x2, y2)->visibility;
                for (int veh_id_stack = veh_top(veh_id); veh_id_stack >= 0;
                     veh_id_stack = Vehs[veh_id_stack].next_veh_id_stack) {
                    Vehs[veh_id_stack].order = ORDER_NONE;
                    Vehs[veh_id_stack].visibility |= visibility;
                }
            }
            veh_skip(veh_id);
            if (faction_id != ConsoleGlobal.faction_id_) {
                return 0;
            }
            if ((Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_
                    || (Vehs[veh_id].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) == (VFLAG_LURKER | VFLAG_INVISIBLE))
                && !(Vehs[veh_id].visibility & (1 << ConsoleGlobal.faction_id_))) {
                return 0;
            }
            if (!ConsoleGlobal.focus(x2, y2, ConsoleGlobal.faction_id_)) {
                draw_tile(x2, y2, 2);
            }
            parse_says(0, Bases[base_id].name_string, -1, -1);
            if (IsMultiplayerNet) {
                if (Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER) {
                    NetMsgPrimary.pop("SPORELAUNCHSWARM", 3000, 0, 0);
                    return 0;
                }
                NetMsgPrimary.pop("MINDWORMSWARM", 3000, 0, 0);
                return 0;
            }
            if (Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER) {
                popp(ScriptPopupFile, "SPORELAUNCHSWARM", 0, "sporlnch_sm.pcx", 0);
                return 0;
            }
            popp(ScriptPopupFile, "MINDWORMSWARM", 0, "mindworm_sm.pcx", 0);
            return 0;
        }
        set_course(veh_id, 99, Bases[base_id].x, Bases[base_id].y);
        if (Vehs[veh_id].order == ORDER_MOVE_TO
            && !is_ocean(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0])
            && alien_dist(x, y, Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]) <= 1) {
            veh_skip(veh_id);
        }
        return 0;
    }
    {
    if ((BaseFindDist > 7 || region_at(x, y) != region) && !zoc_any(x, y, 0)
        && Vehs[veh_id].terraforming_turns > (get_triad(veh_id) ? 16 : 6)
        && !((GameDataStateStorage.fields_.turn_current_num_ + veh_id) & 7)) {
        kill(veh_id);
        return 1;
    }
    int move_dir;
    if (base_id >= 0 && PlayersData[faction_id].current_num_bases > 2
        && GameDataStateStorage.fields_.turn_current_num_ > 50
        && (Vehs[veh_id].order_auto_type == faction_id
            || PlayersData[faction_id].soc_effect_active.values_[SocialEffect::PLANET] <= 0
            || Vehs[veh_id].proto_id == BSC_SEALURK)) {
        Vehs[veh_id].waypoint_x[0] = Bases[base_id].x;
        Vehs[veh_id].waypoint_y[0] = Bases[base_id].y;
        move_dir = Paths.move(veh_id, 0);
    } else {
        move_dir = Vehs[veh_id].facing;
    }
    if (can_arty(Vehs[veh_id].proto_id, true)) {
        int best_score = 0;
        int best_offset = -1;
        for (int i = 0; i < 25; i++) {
            int x2 = wrap_map_x(x + RadiusOffsetX[i]);
            int y2 = y + RadiusOffsetY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            int score;
            if (anything_at(x2, y2) < 0) {
                if (is_ocean(x2, y2) || !(map_loc(x2, y2)->bit & 0x81208854)) {
                    continue;
                }
                score = (map_loc(x2, y2)->bit & BIT_SENSOR_ARRAY) ? 32 : 2;
                if (!i) {
                    score = 0;
                }
                if (score > best_score) {
                    best_score = score;
                    best_offset = i;
                }
            } else {
                int veh_id_stack = stack_fix(veh_at(x2, y2));
                if (veh_id_stack < 0) {
                    continue;
                }
                score = (stack_check(veh_id_stack, 2, 6, -1, -1)
                    + stack_check(veh_id_stack, 6, ABL_ARTILLERY, -1, -1)) * 8;
                if (!zoc_veh(x, y, 0) || i < 9) {
                    score += stack_check(veh_id_stack, 4, -1, -1, -1);
                }
                if (alien_dist(x, y, x2, y2) > Rules.artillery_max_rng && score > 1) {
                    score = 1;
                }
                if (!i) {
                    score = 0;
                }
                if (score > best_score) {
                    best_score = score;
                    best_offset = i;
                }
            }
        }
        if (best_offset >= 0) {
            int x2 = wrap_map_x(x + RadiusOffsetX[best_offset]);
            int y2 = y + RadiusOffsetY[best_offset];
            if (alien_dist(x, y, x2, y2) <= Rules.artillery_max_rng) {
                if (anything_at(x2, y2) < 0) {
                    log_say("Enemy doing ARTY destroy 1...", veh_id, x2, y2);
                    action_destroy(veh_id, 0, x2, y2);
                    synch_bit(x2, y2, 1);
                    synch_bit(x2, y2, 2);
                    synch_bit(x2, y2, 3);
                    synch_bit(x2, y2, 4);
                    synch_bit(x2, y2, 5);
                    synch_bit(x2, y2, 6);
                    synch_bit(x2, y2, 7);
                    draw_tile(x2, y2, 2);
                    return 1;
                }
                if (best_offset >= 9) {
                    log_say("Enemy doing ARTY...", veh_id, best_offset, 0);
                    battle_fight(veh_id, best_offset, 1, 1, 0);
                    return 1;
                }
            }
            // Unreachable in practice: best_offset never exceeds 24.
            if (best_offset >= 25 && anything_at(x2, y2) >= 0) {
                dir_selected = -1;
            }
        } else if (stack_check(veh_id, 15, -1, -1, -1) <= 1 && map_loc(x, y)->bit & BIT_UNK_4000) {
            dir_selected = -1;
        }
    }
    int best_score = 0;
    const int morale = morale_alien(-1, 0);
    // Not reset per tile: only the 12345 sea-base sentinel survives the test below.
    int score;
    for (int i = 0; i < 8; i++) {
        int x2 = wrap_map_x(x + VehMoveDeltaX[i]);
        int y2 = y + VehMoveDeltaY[i];
        if (!on_map(x2, y2)) {
            continue;
        }
        if (is_ocean(x2, y2)) {
            if (!get_triad(veh_id)) {
                continue;
            }
        } else {
            if (get_triad(veh_id) == TRIAD_SEA) {
                if (Vehs[veh_id].proto_id != BSC_SEALURK || base_at(x2, y2) == -1) {
                    continue;
                }
                score = 12345;
            }
            if (!is_ocean(x, y) && !get_triad(veh_id) && veh_who(x2, y2) > 0) {
                continue;
            }
        }
        if (score != 12345) {
            score = 0;
        }
        if (base_who(x2, y2) > 0) {
            score += 9999;
        } else {
            int veh_id_tile = veh_at(x2, y2);
            if (veh_id_tile >= 0 && Vehs[veh_id_tile].faction_id) {
                if (Vehs[veh_id_tile].proto_id < MaxVehProtoFactionNum
                    && get_proto_offense_rating(Vehs[veh_id_tile].proto_id) < 0) {
                    continue;
                }
                if (get_triad(veh_id_tile) == TRIAD_AIR && get_triad(veh_id) != TRIAD_AIR) {
                    continue;
                }
                score += 999;
            }
        }
        uint32_t bits = map_loc(x2, y2)->bit;
        score += bit_count(bits & 0x80208054) * 4;
        if (GameDataStateStorage.fields_.diff_level_current_ > 2) {
            score += (bits & BIT_MONOLITH && !(Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED)) ? 8 : 0;
        }
        score += rand() % 8;
        int turn = ((move_dir ^ -4) - i) & 7;
        if (turn > 4) {
            turn = 8 - turn;
        }
        score += (morale + 1) * turn;
        if (i == move_dir) {
            score += morale * morale;
        }
        if (score > best_score) {
            best_score = score;
            dir_selected = i;
        }
    }
    int retreat;
    switch (morale) {
    case 0:
        retreat = 0;
        break;
    case 1:
        retreat = 1;
        break;
    case 2:
        retreat = 2;
        break;
    default:
        retreat = 4;
        break;
    }
    if (Vehs[veh_id].dmg_incurred) {
        retreat *= 2;
    }
    if ((1 << Bases[base_id].faction_id_current) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_
        && retreat && rand() % 4 < ((best_score < 999) * retreat ? 2 : 1)
        && veh_moves(veh_id) <= Rules.move_rate_roads && get_triad(veh_id) != TRIAD_AIR) {
        int level = (GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)
            ? 5 : PlayersData[Bases[base_id].faction_id_current].diff_level;
        if (bounded_crt_random(level + 1) && map_loc(x, y)->bit & 0x80E88058) {
            action_destroy(veh_id, 0, -1, -1);
            return 0;
        }
    }
    if (best_score < 999 && !Vehs[veh_id].moves_expended && Vehs[veh_id].dmg_incurred) {
        if (!(map_loc(x, y)->bit & BIT_FUNGUS) || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF) {
            retreat = (retreat + 1) / 2;
        }
        if (bounded_crt_random(retreat + 1)) {
            dir_selected = -1;
        }
    }
    if (best_score < 99 && veh_cargo(veh_id) && !stack_check(veh_id, 3, TRIAD_LAND, -1, -1)) {
        kill(veh_id);
        return 1;
    }
    }
move_direction:
    if (dir_selected < 0) {
        Vehs[veh_id].order = ORDER_NONE;
        Vehs[veh_id].state &= ~VSTATE_UNK_10000;
        veh_skip(veh_id);
        return 0;
    }
    Vehs[veh_id].order = ORDER_AI_MOVE_TO;
    Vehs[veh_id].waypoint_x[0] = wrap_map_x(x + VehMoveDeltaX[dir_selected]);
    Vehs[veh_id].waypoint_y[0] = y + VehMoveDeltaY[dir_selected];
    return 0;
}

// Aircraft/missile movement and target selection, preserving the five-argument ABI.

// ORIGINAL: 0x005678C0 ?air_power@@YAHHHHPAH0@Z 0x005678C0-0x0056A89E
// RECOVERY_BINDING: v1 {"evidence": ["git:846a0ba2a949fc3692ce8bc7ccb7f5020f9e0d9f:src/unrecovered/005678c0.cpp complete original identity/span and historical lessons", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x005678C0-0x0056A89E full native body and independently reviewed field/callback/branch evidence.", "source:src/veh.cpp and existing veh/map/base/game/netdaemon declarations; existing TU-local seven-argument battle_fight overload, no fake wrapper or executable-address dependency."], "reason": "Complete aircraft and missile movement/target selection through actual vehicle, map, player and base owners; original nullness, callbacks and range/ordering behavior retained."}
// TRIED: MISMATCH #4 mov/push at prologue; full-body transcription from Ghidra hypothesis, corrected against raw disasm for: veh_at (declared 2-arg, several call sites showed 6 - a dangling outer push for the immediately-following stack_check(int,int,int,int,int) 5-arg call), veh_lift/veh_drop split (PROPOSAL: veh_lift should be `int veh_lift(int)`, its mangled YAX return hides a real value the caller forwards into veh_drop's first arg), and the VEH struct field widths (short at 0x952828/2a/3c/3e/44/46, byte at 0x952836/38/39/4d/4e/4f/50/51/53) recovered from `movsx word/byte ptr [reg+ADDR]` on every g_009528xx access, since the scaffold's g_ globals are declared int* uniformly.
int __cdecl air_power(int a1, int a2, int a3, int *a4, int *a5) {
    int live_iVar10 = 0;
    int live_iVar11 = 0;
    int requested_region = -1;
    int result = -3;
    uint32_t bombing_auto_state = 0;

    Veh &attacker = Vehs[a1];
    if (attacker.order == ORDER_MOVE_TO &&
        (attacker.waypoint_x[0] != *a4 ||
         attacker.waypoint_y[0] != *a5)) {
        if (a3 == 0 ||
            VehPrototypes[attacker.proto_id].plan == PLAN_AIR_SUPERIORITY) {
            return -2;
        }
        return -1;
    }

    requested_region = map_loc(*a4, *a5)->region;
    {
    const int chassis_id = VehPrototypes[attacker.proto_id].chassis_id;
    const int range = Chassis[chassis_id].range;
    const int road_rate = Rules.move_rate_roads;
    if (range == 0) {
        const int unit_speed = speed(a1, 0);
        live_iVar11 = unit_speed * 6;
        live_iVar10 = unit_speed * 6;
    } else if (range == 1) {
        live_iVar10 = speed(a1, 0) - attacker.moves_expended;
        if (live_iVar10 < 0) live_iVar10 = 0;
        else if (live_iVar10 > 999) live_iVar10 = 999;
        live_iVar11 = live_iVar10;
        if (attacker.dmg_incurred == 0)
            live_iVar11 = speed(a1, 0) + live_iVar10;
    } else {
        live_iVar11 = speed(a1, 0) - attacker.moves_expended;
        if (live_iVar11 < 0) live_iVar11 = 0;
        else if (live_iVar11 > 999) live_iVar11 = 999;
        live_iVar10 = speed(a1, 0);
        live_iVar11 = live_iVar10 * (range - attacker.dmg_incurred - 1)
            + live_iVar11;
        live_iVar10 = live_iVar11;
    }
    live_iVar11 /= road_rate;
    live_iVar10 /= road_rate;

    if (range != 0 && attacker.terraforming_turns != 0) {
        action_home(a1, 4);
        if (attacker.order == ORDER_MOVE_TO) return -2;
    }

    if ((attacker.state & VSTATE_UNK_200) != 0 &&
        attacker.order_auto_type == ORDERA_ON_ALERT) {
        if (attacker.dmg_incurred != 0) {
            action_home(a1, 0);
            return -2;
        }
    } else if ((attacker.state & VSTATE_UNK_200) != 0) {
        if (attacker.order_auto_type == ORDERA_AUTOMATE_AIR_DEFENSE) {
            if (attacker.dmg_incurred != 0) {
                action_home(a1, 0);
                return -2;
            }
        } else if (attacker.order_auto_type == ORDERA_BOMBING_RUN) {
            bombing_auto_state = attacker.state;
            goto air_power_bombing;
        }
    }

    {
        const int faction_id = attacker.faction_id;
        const int relocation_region = map_loc(*a4, *a5)->region;
        const bool forced_relocation = (attacker.state & 0x4000u) != 0;
        const bool terraform_region_relocation =
            (map_loc(*a4, *a5)->climate & 0xE0u) > 0x5Fu
            && PlayersData[faction_id]
                   .region_base_plan[relocation_region + 32] == 8;
        if (forced_relocation || terraform_region_relocation) {
            int best_x = -1;
            int best_y = -1;
            int best_visibility_count = 0;
            for (int direction = 0; direction < 8; ++direction) {
                int candidate_x = VehMoveDeltaX[direction] * (live_iVar10 / 2)
                    + *a4;
                if ((MapIsFlat & 1) == 0) {
                    if (candidate_x < 0) candidate_x += MapLongitudeBounds;
                    else if (candidate_x >= MapLongitudeBounds)
                        candidate_x -= MapLongitudeBounds;
                }
                const int candidate_y = VehMoveDeltaY[direction]
                    * (live_iVar10 / 2) + *a5;
                if (candidate_x < 0 || candidate_x >= MapLongitudeBounds
                    || candidate_y < 0 || candidate_y >= MapLatitudeBounds)
                    continue;

                int visibility_count = 0;
                for (int neighbor = 0; neighbor < 9; ++neighbor) {
                    int neighbor_x = VehMoveDeltaX[neighbor] + candidate_x;
                    if ((MapIsFlat & 1) == 0) {
                        if (neighbor_x < 0) neighbor_x += MapLongitudeBounds;
                        else if (neighbor_x >= MapLongitudeBounds)
                            neighbor_x -= MapLongitudeBounds;
                    }
                    const int neighbor_y = VehMoveDeltaY[neighbor] + candidate_y;
                    if (neighbor_x < 0 || neighbor_x >= MapLongitudeBounds
                        || neighbor_y < 0 || neighbor_y >= MapLatitudeBounds
                        || (PlayersData[faction_id].flags & 0x200u) != 0)
                        continue;
                    const uint8_t faction_bit = static_cast<uint8_t>(
                        1U << (faction_id & 0x1F));
                    if ((map_loc(neighbor_x, neighbor_y)->visibility
                         & faction_bit) == 0)
                        ++visibility_count;
                }
                if (best_visibility_count < visibility_count) {
                    best_visibility_count = visibility_count;
                    best_x = candidate_x;
                    best_y = candidate_y;
                }
            }

            if (best_x >= 0) {
                attacker.order = ORDER_MOVE_TO;
                attacker.move_to_ai_type = 0x61;
                attacker.waypoint_x[0] = static_cast<int16_t>(best_x);
                attacker.waypoint_y[0] = static_cast<int16_t>(best_y);
                if (VehPrototypes[attacker.proto_id].plan == PLAN_COLONIZATION)
                    attacker.terraforming_turns = 0;
                return -2;
            }

            int best_base_id = -1;
            int best_random_distance = -1;
            for (int base_id = 0;
                 base_id < GameDataStateStorage.fields_.base_current_count_;
                 ++base_id) {
                const Base &base = Bases[base_id];
                if (base.faction_id_current != faction_id) continue;
                int dx = abs(*a4 - base.x);
                if ((MapIsFlat & 1) == 0 && MapLongitude < dx)
                    dx = static_cast<int>(MapLongitudeBounds) - dx;
                const int dy = abs(*a5 - base.y);
                const int distance = (dx + dy) >> 1;
                if (distance <= live_iVar10) {
                    const int random_distance = distance < 1
                        ? 0 : rand() % (distance + 1);
                    if (best_random_distance < random_distance) {
                        best_base_id = base_id;
                        best_random_distance = random_distance;
                    }
                }
            }
            if (best_base_id >= 0) {
                const Base &base = Bases[best_base_id];
                attacker.order = ORDER_MOVE_TO;
                attacker.move_to_ai_type = 0x63;
                attacker.waypoint_x[0] = static_cast<int16_t>(base.x);
                attacker.waypoint_y[0] = static_cast<int16_t>(base.y);
                if (VehPrototypes[attacker.proto_id].plan == PLAN_COLONIZATION)
                    attacker.terraforming_turns = 0;
                return -2;
            }
            if (forced_relocation) return -3;
        }
    }

    const int post_relocation_chassis =
        VehPrototypes[attacker.proto_id].chassis_id;
    if (Chassis[post_relocation_chassis].missile != 0) {
        const uint32_t faction_bit =
            1U << (attacker.faction_id & 0x1F);
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0 && IsMultiplayerNet)
            return -3;
// Original missile-goal, selector, route-commit and fallback effects stay
// in this mapped owner; branch and callback ordering are unchanged.
{
    const int missile_plan = VehPrototypes[attacker.proto_id].plan;
    int missile_selected_x = -1;
    int missile_selected_y = -1;

    if (missile_plan == 13) {
        const int faction_id = attacker.faction_id;
        const int actor_plan = missile_plan;
        int best_x = -1;
        int best_y = -1;
        int best_score = 0;
        int goal_index;

        for (goal_index = 0; goal_index < 75; ++goal_index) {
            const Goal &goal = PlayersData[faction_id].goals[goal_index];
            if (goal.type == 0x29) {
                const int distance = cursor_dist(*a4, *a5, goal.x, goal.y);
                const int move_limit = speed(a1, 0) / Rules.move_rate_roads
                                     - Rules.move_rate_roads;
                int score = (int)goal.priority * 8;
                if (move_limit < distance) score /= 2;
                const Map *tile = map_loc(goal.x, goal.y);
                if (((tile->bit & 3) == 0 || (tile->val2 & 0x0F) > 7)
                    && (tile->climate & 0xE0) > 0x5F
                    && best_score < score) {
                    best_x = goal.x;
                    best_y = goal.y;
                    best_score = score;
                }
            }
        }

        int base_id;
        for (base_id = 0;
             base_id < GameDataStateStorage.fields_.base_current_count_;
             ++base_id) {
            const Base &base = Bases[base_id];
            const Map *base_tile;
            int score;
            int approach_x;
            if (base.faction_id_current != faction_id) continue;
            base_tile = map_loc(base.x, base.y);
            if ((base_tile->climate & 0xE0) <= 0x5F) continue;
            if (PlayersData[faction_id].region_total_bases[base_tile->region] != 0) {
                base_find(base.x, base.y, faction_id);
                if (BaseFindDist <= 9) continue;
            }
            // Base::population_size is the original signed low-byte value;
            // retain its signed division/promotion behavior here.
            score = ((int)base.population_size + 4) / 5 * 8;
            approach_x = wrap_map_x((int)base.x + 4);
            if (base.y < 0 || base.y >= (int)MapLatitudeBounds
                || approach_x < 0 || approach_x >= (int)MapLongitudeBounds)
                continue;
            base_tile = map_loc(approach_x, base.y);
            if ((base_tile->bit & 3) != 0 && (base_tile->val2 & 0x0F) <= 7)
                continue;
            {
                const int distance = cursor_dist(*a4, *a5, approach_x, base.y);
                if (speed(a1, 0) / Rules.move_rate_roads
                        - Rules.move_rate_roads < distance)
                    score /= 2;
            }
            if (best_score < score) {
                best_x = approach_x;
                best_y = base.y;
                best_score = score;
            }
        }

        if (best_x >= 0) {
            missile_selected_x = best_x;
            missile_selected_y = best_y;
            const int distance = cursor_dist(*a4, *a5, best_x, best_y);
            if (distance <= speed(a1, 0) / Rules.move_rate_roads
                               - Rules.move_rate_roads) {
                attacker.order = ORDER_MOVE_TO;
                attacker.move_to_ai_type = 0x6D;
                attacker.waypoint_x[0] = (int16_t)best_x;
                attacker.waypoint_y[0] = (int16_t)best_y;
                if (actor_plan == 8) attacker.terraforming_turns = 0;
            } else {
                int closest_base_distance = 9999;
                for (base_id = 0;
                     base_id < GameDataStateStorage.fields_.base_current_count_;
                     ++base_id) {
                    const Base &base = Bases[base_id];
                    const int base_faction = base.faction_id_current;
                    if (base_faction == faction_id
                        || (PlayersData[faction_id].diplo_treaties[base_faction] & 1) != 0) {
                        const int distance_from_target = cursor_dist(
                            *a4, *a5, base.x, base.y);
                        if (distance_from_target
                            <= speed(a1, 0) / Rules.move_rate_roads
                                 - Rules.move_rate_roads) {
                            const int distance = cursor_dist(best_x, best_y,
                                                             base.x, base.y);
                            if (distance < closest_base_distance) {
                                attacker.order = ORDER_MOVE_TO;
                                attacker.move_to_ai_type = 0x4D;
                                attacker.waypoint_x[0] = base.x;
                                attacker.waypoint_y[0] = base.y;
                                closest_base_distance = distance;
                                if (actor_plan == 8)
                                    attacker.terraforming_turns = 0;
                            }
                        }
                    }
                }
            }
            return -3;
        }
    } else if (missile_plan == 14) {
        const int faction_id = attacker.faction_id;
        int best_score = 0;
        int base_id;
        missile_selected_x = -1;
        missile_selected_y = -1;
        for (base_id = 0;
             base_id < GameDataStateStorage.fields_.base_current_count_;
             ++base_id) {
            const Base &base = Bases[base_id];
            const Map *base_tile;
            int base_score;
            int offset_index;
            if (base.faction_id_current == faction_id) continue;
            if ((PlayersData[faction_id].diplo_treaties[base.faction_id_current]
                 & 0x10) == 0)
                continue;
            if (PlayersData[base.faction_id_current]
                    .soc_effect_pending.values_[SocialEffect::PLANET] < 1)
                continue;
            base_tile = map_loc(base.x, base.y);
            if (PlayersData[faction_id].region_total_bases[base_tile->region] != 0) {
                base_find(base.x, base.y, faction_id);
                if (BaseFindDist <= 4) continue;
            }
            base_score = (int)base.population_size * 8;
            for (offset_index = 0; offset_index < 21; ++offset_index) {
                const int x = wrap_map_x((int)base.x + RadiusOffsetX[offset_index]);
                const int y = (int)base.y + RadiusOffsetY[offset_index];
                const Map *tile;
                int score = base_score;
                if (y < 0 || y >= (int)MapLatitudeBounds
                    || x < 0 || x >= (int)MapLongitudeBounds)
                    continue;
                tile = map_loc(x, y);
                if ((tile->bit & 0x20) != 0 && (tile->climate & 0xE0) >= 0x40)
                    continue;
                if ((tile->bit & 3) != 0 && (tile->val2 & 0x0F) <= 7)
                    continue;
                {
                    const int distance = cursor_dist(*a4, *a5, x, y);
                    if (speed(a1, 0) / Rules.move_rate_roads
                            - Rules.move_rate_roads < distance)
                        score /= 2;
                }
                score += bit_count((int)(tile->bit & 0x01E8881C));
                if (best_score < score) {
                    missile_selected_x = x;
                    missile_selected_y = y;
                    best_score = score;
                }
            }
        }

        if (missile_selected_x >= 0) {
            const int distance = cursor_dist(*a4, *a5,
                                             missile_selected_x,
                                             missile_selected_y);
            if (distance <= speed(a1, 0) / Rules.move_rate_roads
                               - Rules.move_rate_roads) {
                attacker.order = ORDER_MOVE_TO;
                attacker.move_to_ai_type = 0x6D;
                attacker.waypoint_x[0] = (int16_t)missile_selected_x;
                attacker.waypoint_y[0] = (int16_t)missile_selected_y;
                if (VehPrototypes[attacker.proto_id].plan == 8)
                    attacker.terraforming_turns = 0;
            } else {
                int closest_base_distance = 9999;
                for (base_id = 0;
                     base_id < GameDataStateStorage.fields_.base_current_count_;
                     ++base_id) {
                    const Base &base = Bases[base_id];
                    if (base.faction_id_current == faction_id) {
                        const int distance_from_target = cursor_dist(
                            *a4, *a5, base.x, base.y);
                        if (distance_from_target
                            <= speed(a1, 0) / Rules.move_rate_roads
                                 - Rules.move_rate_roads) {
                            const int distance = cursor_dist(
                                missile_selected_x, missile_selected_y,
                                base.x, base.y);
                            if (distance < closest_base_distance) {
                                attacker.order = ORDER_MOVE_TO;
                                attacker.move_to_ai_type = 0x4D;
                                attacker.waypoint_x[0] = base.x;
                                attacker.waypoint_y[0] = base.y;
                                closest_base_distance = distance;
                                if (VehPrototypes[attacker.proto_id].plan == 8)
                                    attacker.terraforming_turns = 0;
                            }
                        }
                    }
                }
            }
        } else {
            missile_selected_x = -1;
        }
    } else {
        // The original 64-iteration tally advances the prototype id by the
        // actor faction id, not by one; preserve this observed odd stride.
        const int tally_faction_id = attacker.faction_id;
        int proto_active_count = 0;
        int proto_id = 64;
        int remaining = 64;
        while (remaining != 0) {
            if ((VehPrototypes[proto_id].flags & 1) != 0
                && has_abil(proto_id, 0x10))
                proto_active_count += PlayersData[tally_faction_id]
                    .proto_id_active[proto_id];
            proto_id += tally_faction_id;
            --remaining;
        }
        int best_score = (int)VehPrototypes[attacker.proto_id].cost << 4;
        int best_vehicle = -1;
        int best_distance = 9999;
        int candidate_id;
        live_iVar10 = 0;

        for (candidate_id = 0;
             candidate_id < GameDataStateStorage.fields_.vehicle_current_count_;
             ++candidate_id) {
            Veh &candidate = Vehs[candidate_id];
            const int faction_id = attacker.faction_id;
            const int candidate_faction = candidate.faction_id;
            const int candidate_triad =
                Chassis[VehPrototypes[candidate.proto_id].chassis_id].triad;
            const uint8_t faction_bit = (uint8_t)(1U << (faction_id & 0x1F));
            const int base_id = base_at(candidate.x, candidate.y);
            bool colony = false;
            if (candidate_faction == faction_id
                || (PlayersData[faction_id]
                        .diplo_treaties[candidate_faction] & 0x10) == 0)
                continue;
            if (candidate.order == 0x15) {
                const Map *candidate_tile = map_loc(candidate.x, candidate.y);
                if ((candidate_tile->climate & 0xE0) < 0x60) {
                    const int nearby_base = base_find(candidate.x, candidate.y,
                                                      faction_id);
                    if (nearby_base >= 0) {
                        const Base &base = Bases[nearby_base];
                        const int dx = x_dist(candidate.x, base.x);
                        const int dy = abs((int)candidate.y - (int)base.y);
                        if (4 >= ((dx + dy) & 0xFFFFFFFE)) colony = true;
                    }
                }
            }
            if (!colony
                && ((candidate_faction != faction_id
                     || (candidate.flags & 0x440) == 0x440)
                    && (candidate.visibility & faction_bit) == 0)
                && (base_id < 0
                    || (Bases[base_id].faction_id_current != faction_id
                        && (Bases[base_id].visibility & faction_bit) == 0)))
                continue;
            if (candidate_triad == 2 && base_id < 0) continue;

            const int battle = battle_fight(a1, 0, candidate.x, candidate.y,
                                            0, 0, 0);
            int odds = compute_odds(battle, faction_id, a1,
                                    candidate_id, -2);
            int strategic_weight;
            if (base_id < 0) {
                odds /= 2;
                strategic_weight = 4;
                if (stack_check(candidate_id, 2, 7, -1, -1) == 0
                    && (stack_check(candidate_id, 2, 9, -1, -1) == 0
                        || (map_loc(candidate.x, candidate.y)->climate & 0xE0) > 0x5F)
                    && whose_territory(faction_id, candidate.x,
                                       candidate.y, 0, 0) != faction_id) {
                    strategic_weight = 2;
                    odds /= 2;
                } else {
                    ++proto_active_count;
                }
            } else if (proto_active_count == 0) {
                bool friendly_vehicle_in_radius = false;
                int radius_index;
                for (radius_index = 0; radius_index < 25; ++radius_index) {
                    const int x = wrap_map_x((int)Bases[base_id].x
                                         + RadiusOffsetX[radius_index]);
                    const int y = (int)Bases[base_id].y
                                + RadiusOffsetY[radius_index];
                    if (y < 0 || y >= (int)MapLatitudeBounds
                        || x < 0 || x >= (int)MapLongitudeBounds)
                        continue;
                    const Map *tile = map_loc(x, y);
                    if ((tile->bit & 2) != 0 && (tile->val2 & 0x0F) <= 7
                        && (tile->val2 & 0x0F) == faction_id) {
                        proto_active_count = 1;
                        friendly_vehicle_in_radius = true;
                        break;
                    }
                }
                if (friendly_vehicle_in_radius) {
                    strategic_weight = 16;
                } else {
                    strategic_weight = 4;
                    odds /= 2;
                }
            } else {
                strategic_weight = 16;
            }

            const int noncombat_count = stack_check(candidate_id, 0x13,
                                                     -1, -1, -1);
            const int total_count = stack_check(candidate_id, 1, -1, -1, -1);
            const int sea_combat_count = stack_check(candidate_id, 3, 2, -1, -1);
            if (total_count - noncombat_count <= sea_combat_count) {
                odds *= 2;
                strategic_weight *= 2;
                ++proto_active_count;
            }
            if (proto_active_count != 0) {
                if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_
                     & (1U << (candidate_faction & 0x1F))) != 0)
                    strategic_weight *= 2;
                if (PlayersData[candidate_faction].ranking == 7)
                    strategic_weight *= 2;
            }
            if (colony) odds <<= 3;

            int distance = cursor_dist(*a4, *a5, candidate.x, candidate.y);
            if (speed(a1, 0) / Rules.move_rate_roads
                    - Rules.move_rate_roads < distance)
                strategic_weight /= 2;
            if (7 >= odds / 2) continue;

            const int score = (odds / 2) * strategic_weight;
            if (best_score <= score) {
                best_vehicle = candidate_id;
                best_score = score;
                best_distance = distance;
                const int route_distance = cursor_dist(
                    *a4, *a5, candidate.x, candidate.y);
                if (route_distance > speed(a1, 0) / Rules.move_rate_roads
                                         - Rules.move_rate_roads) {
                    int closest_base_distance = 9999;
                    int base_index;
                    for (base_index = 0;
                         base_index < GameDataStateStorage.fields_.base_current_count_;
                         ++base_index) {
                        const Base &base = Bases[base_index];
                        const int base_faction = base.faction_id_current;
                        if (base_faction != faction_id
                            && (PlayersData[base_faction]
                                    .diplo_treaties[faction_id] & 1) == 0)
                            continue;
                        const int target_to_base = cursor_dist(
                            *a4, *a5, base.x, base.y);
                        if (target_to_base > speed(a1, 0) / Rules.move_rate_roads
                                                 - Rules.move_rate_roads)
                            continue;
                        const int candidate_to_base = cursor_dist(
                            candidate.x, candidate.y, base.x, base.y);
                        if (candidate_to_base < closest_base_distance) {
                            attacker.order = ORDER_MOVE_TO;
                            attacker.move_to_ai_type = 0x4D;
                            attacker.waypoint_x[0] = base.x;
                            attacker.waypoint_y[0] = base.y;
                            closest_base_distance = candidate_to_base;
                            if (VehPrototypes[attacker.proto_id].plan == 8)
                                attacker.terraforming_turns = 0;
                        }
                    }
                } else {
                    attacker.order = ORDER_MOVE_TO;
                    attacker.move_to_ai_type = 0x6D;
                    attacker.waypoint_x[0] = candidate.x;
                    attacker.waypoint_y[0] = candidate.y;
                    if (VehPrototypes[attacker.proto_id].plan == 8)
                        attacker.terraforming_turns = 0;
                }
            }
        }
        (void)best_vehicle;
        (void)best_distance;
    }

    if (missile_plan != 13
        && (missile_plan != 14 || missile_selected_x >= 0)
        && attacker.order == ORDER_MOVE_TO) {
        const int route_plan = VehPrototypes[attacker.proto_id].plan;
        const int route_x = attacker.waypoint_x[0];
        const int route_y = attacker.waypoint_y[0];
        const Map *route_tile = map_loc(route_x, route_y);
        if ((route_tile->bit & BIT_BASE_IN_TILE) != 0
            && (route_tile->val2 & 0x0F) == attacker.faction_id) {
            *a4 = route_x;
            *a5 = route_y;
            attacker.order = 0;
            attacker.visibility = 0;
            const int lifted = veh_lift(a1);
            veh_drop(lifted, *a4, *a5);
            return -3;
        }

        attacker.order = 0;
        if (route_plan == 13) {
            const int lifted = veh_lift(a1);
            veh_drop(lifted, missile_selected_x, missile_selected_y);
            action_tectonic(a1, missile_selected_x, missile_selected_y);
            return -5;
        }
        if (route_plan == 14) {
            const int lifted = veh_lift(a1);
            veh_drop(lifted, missile_selected_x, missile_selected_y);
            action_fungal(a1, missile_selected_x, missile_selected_y);
            return -5;
        }

        int direction;
        for (direction = 0; direction < 8; ++direction) {
            const int x = wrap_map_x(route_x + VehMoveDeltaX[direction]);
            const int y = route_y + VehMoveDeltaY[direction];
            if (y < 0 || y >= (int)MapLatitudeBounds
                || x < 0 || x >= (int)MapLongitudeBounds)
                continue;
            const Map *tile = map_loc(x, y);
            if ((tile->bit & 3) != 0 && (tile->val2 & 0x0F) <= 7)
                continue;
            *a4 = x;
            *a5 = y;
            attacker.visibility = 0;
            const int lifted = veh_lift(a1);
            veh_drop(lifted, x, y);
            attacker.facing = (int8_t)direction;
            return direction ^ 4;
        }
        attacker.order = 0;
    }

    {
        const int actor_stack = stack_check(a1, 0x0D, -1, -1, -1);
        if (actor_stack >= 2) {
            int base_id;
            for (base_id = 0;
                 base_id < GameDataStateStorage.fields_.base_current_count_;
                 ++base_id) {
                const Base &base = Bases[base_id];
                if (base.faction_id_current == attacker.faction_id) {
                    const int occupying_vehicle = veh_at(base.x, base.y);
                    const int base_stack = stack_check(occupying_vehicle, 0x0D,
                                                       -1, -1, -1);
                    if (base_stack < actor_stack) {
                        const int distance = cursor_dist(*a4, *a5,
                                                         base.x, base.y);
                        if (distance <= speed(a1, 0) / Rules.move_rate_roads
                                            - Rules.move_rate_roads) {
                            const int lifted = veh_lift(a1);
                            veh_drop(lifted, base.x, base.y);
                            attacker.visibility = 0;
                            *a4 = base.x;
                            *a5 = base.y;
                            return -3;
                        }
                    }
                }
            }
        }
    }
    return -3;
}
    }

    int live_cVar21 =
        VehPrototypes[attacker.proto_id].plan;
    result = -3;
    {
        const int faction_id = attacker.faction_id;
        const int attacker_plan =
            VehPrototypes[attacker.proto_id].plan;
        live_cVar21 = attacker_plan;
        const bool scan_enabled = attacker_plan == PLAN_AIR_SUPERIORITY
            || ((attacker.state & VSTATE_UNK_200) != 0
                && attacker.order_auto_type == ORDERA_AUTOMATE_AIR_DEFENSE)
            || attacker_plan == PLAN_OFFENSIVE;

        if (scan_enabled) {
            int best_vehicle_id = -1;
            int best_cost = 9999;
            int best_distance = 9999;
            best_vehicle_id = -1;
            best_distance = 9999;
            best_cost = 9999;
            const int attacker_triad =
                Chassis[VehPrototypes[attacker.proto_id].chassis_id].triad;
            const uint8_t faction_bit = static_cast<uint8_t>(
                1U << (faction_id & 0x1F));

            for (int candidate_id = 0;
                 candidate_id < GameDataStateStorage.fields_.vehicle_current_count_;
                 ++candidate_id) {
                const Veh &candidate = Vehs[candidate_id];
                const int candidate_faction = candidate.faction_id;
                const int candidate_plan =
                    VehPrototypes[candidate.proto_id].plan;
                const int candidate_chassis =
                    VehPrototypes[candidate.proto_id].chassis_id;
                const int candidate_triad = Chassis[candidate_chassis].triad;
                const int x = candidate.x;
                const int y = candidate.y;
                const Map *tile = map_loc(x, y);
                bool near_base_exception = false;

                if (candidate_faction == faction_id) {
                    if (attacker_plan == PLAN_AIR_SUPERIORITY
                        && attacker_triad != TRIAD_SEA
                        && (((tile->bit & 1u) == 0)
                            || ((tile->val2 & 0x0Fu) >= 8))
                        && (tile->bit & 0x40000u) == 0
                        && stack_check(candidate_id, 6, 0x80, -1, -1) == 0) {
                        const int candidate_speed = speed(candidate_id, 0);
                        const int remaining_moves = candidate_speed
                            - candidate.moves_expended;
                        if ((remaining_moves < 0
                             || (remaining_moves < 1000
                                 && candidate_speed ==
                                        candidate.moves_expended))
                            && stack_check(candidate_id, 2, 4, -1, -1) == 0
                            && ((candidate_triad == TRIAD_AIR
                                 && candidate_plan != PLAN_AIR_SUPERIORITY)
                                || candidate_plan == PLAN_NAVAL_TRANSPORT)) {
                            int dx = abs(*a4 - x);
                            if ((MapIsFlat & 1) == 0 && MapLongitude < dx)
                                dx = static_cast<int>(MapLongitudeBounds) - dx;
                            const int dy = abs(*a5 - y);
                            const int distance = (dx + dy) >> 1;
                            int cost = distance * 2 + 1;
                            if (speed(a1, 0) / Rules.move_rate_roads < distance)
                                cost *= 4;
                            cost /= stack_check(candidate_id, 1, -1, -1, -1) + 1;
                            if (cost <= best_cost) {
                                best_vehicle_id = candidate_id;
                                best_cost = cost;
                                best_distance = distance;
                            }
                        }
                    }
                    continue;
                }

                if (candidate.order == ORDER_TERRAFORM_DOWN) {
                    if ((tile->climate & 0xE0u) > 0x5Fu)
                        goto ordinary_target_filter;
                    const int base_id = base_find(x, y, faction_id);
                    if (base_id < 0) goto ordinary_target_filter;
                    int dx = abs(x - Bases[base_id].x);
                    if ((MapIsFlat & 1) == 0 && MapLongitude < dx)
                        dx = static_cast<int>(MapLongitudeBounds) - dx;
                    const int dy = abs(y - Bases[base_id].y);
                    if (((dx + dy) & ~1) > 4)
                        goto ordinary_target_filter;
                    near_base_exception = true;
                } else {
ordinary_target_filter:
                    if (((candidate_faction != faction_id
                          || (candidate.flags & 0x440u) == 0x440u)
                         && (candidate.visibility & faction_bit) == 0))
                        continue;
                }

                if ((PlayersData[faction_id]
                         .diplo_treaties[candidate_faction] & 7u) != 0)
                    continue;

                if (candidate_triad == TRIAD_AIR) {
                    if (attacker_plan == PLAN_AIR_SUPERIORITY
                        && (((tile->bit & 1u) == 0)
                            || ((tile->val2 & 0x0Fu) >= 8))
                        && (tile->bit & 0x40000u) == 0)
                        goto ordinary_defensive_candidate;
                } else if (attacker_plan != PLAN_AIR_SUPERIORITY
                           || (attacker.state & VSTATE_UNK_200) == 0
                           || attacker.order_auto_type
                                  != ORDERA_AUTOMATE_AIR_DEFENSE) {
                    const bool base_at_target =
                        (tile->bit & 1u) != 0 && (tile->val2 & 0x0Fu) < 8;
                    const int defender_chassis =
                        VehPrototypes[candidate.proto_id].chassis_id;
                    const bool run_combat = attacker_plan != 4
                        || (attacker.state & VSTATE_UNK_200) == 0
                        || attacker.order_auto_type
                               != ORDERA_AUTOMATE_AIR_DEFENSE;
                    if (run_combat &&
                        !(Chassis[defender_chassis].triad == TRIAD_AIR
                          && !base_at_target)) {
                        const int battle = battle_fight(
                            a1, 0, candidate.x, candidate.y, 0, 0, 0);
                        int odds = compute_odds(battle, attacker.faction_id,
                                                a1, candidate_id, -2);
                        const Map *defender_tile = map_loc(candidate.x,
                                                           candidate.y);
                        if ((defender_tile->bit & 1u) != 0 &&
                            (defender_tile->val2 & 0x0Fu) < 8) {
                            if (attacker_plan != 0) {
                                const int defenders = stack_check(
                                    candidate_id, 0x13, -1, -1, -1);
                                const int units = stack_check(
                                    candidate_id, 1, -1, -1, -1);
                                if (units - defenders != 1) odds /= 2;
                            } else {
                                continue;
                            }
                        }
                        int combat_score = (odds * 2) / 3;
                        if (near_base_exception) combat_score *= 8;
                        if (attacker_plan != 4
                            || (attacker.state & VSTATE_UNK_200) == 0
                            || attacker.order_auto_type
                                   != ORDERA_AUTOMATE_AIR_DEFENSE)
                            combat_score /= 2;
                        if (combat_score > 7)
                            goto ordinary_defensive_candidate;
                    }
                }
                continue;

ordinary_defensive_candidate:
                if (!(PlayersData[faction_id].unk_67 == 0
                      || PlayersData[faction_id].unk_67 * 2
                             <= PlayersData[faction_id].unk_105
                      || whose_territory(faction_id, x, y, 0, 0)
                             == faction_id)) {
                    const int base_id = base_find(x, y);
                    if (base_id >= 0 &&
                        Bases[base_id].faction_id_current != faction_id)
                        continue;
                }

                if ((attacker.state & VSTATE_UNK_200) != 0 &&
                    attacker.order_auto_type == ORDERA_AUTOMATE_AIR_DEFENSE) {
                    if ((tile->climate & 0xE0u) < 0x60u) {
                        base_find(x, y, faction_id);
                        if (BaseFindDist >= 3) continue;
                    } else if (whose_territory(faction_id, x, y, 0, 0)
                               != faction_id) {
                        continue;
                    }
                }

                int dx = abs(*a4 - x);
                if ((MapIsFlat & 1) == 0 && MapLongitude < dx)
                    dx = static_cast<int>(MapLongitudeBounds) - dx;
                const int dy = abs(*a5 - y);
                const int distance = (dx + dy) >> 1;
                int cost;
                if (live_iVar11 / 2 < distance) {
                    cost = distance * 2;
                } else {
                    const int region = map_loc(x, y)->region;
                    if (PlayersData[faction_id].region_base_plan[
                            region + 32] == 2) {
                        cost = (distance - ((dx + dy) >> 31)) >> 1;
                    } else {
                        cost = distance;
                        if (PlayersData[faction_id]
                                .region_total_bases[region] == 0)
                            cost = distance * 2;
                    }
                    if (near_base_exception) cost = 0;
                }

                if (cost <= best_cost) {
                    best_cost = cost;
                    best_distance = near_base_exception ? 0 : distance;
                    best_vehicle_id = candidate_id;
                }
            }

            if (best_vehicle_id >= 0) {
                if (Vehs[best_vehicle_id].faction_id == faction_id)
                    best_distance = (best_distance - 8) / 2;

                if (best_distance <= live_iVar11 / 2) {
                    attacker.order = ORDER_MOVE_TO;
                    attacker.move_to_ai_type = 0x61;
                    attacker.waypoint_x[0] = Vehs[best_vehicle_id].x;
                    attacker.waypoint_y[0] = Vehs[best_vehicle_id].y;
                    if (VehPrototypes[attacker.proto_id].plan
                        == PLAN_COLONIZATION)
                        attacker.terraforming_turns = 0;
                    if (*a4 != Vehs[best_vehicle_id].x ||
                        *a5 != Vehs[best_vehicle_id].y)
                        return -2;
                    return -1;
                }

                const bool try_allied_base =
                    (((attacker.state & VSTATE_UNK_200) == 0)
                     || attacker.order_auto_type != ORDERA_ON_ALERT)
                    && (VehPrototypes[attacker.proto_id].plan
                            != PLAN_OFFENSIVE
                        || ((attacker.state & VSTATE_UNK_200) != 0
                            && attacker.order_auto_type
                                   == ORDERA_AUTOMATE_AIR_DEFENSE));
                if (try_allied_base) {
                    const int selected_x = Vehs[best_vehicle_id].x;
                    const int selected_y = Vehs[best_vehicle_id].y;
                    for (int base_id = 0;
                         base_id < GameDataStateStorage.fields_.base_current_count_;
                         ++base_id) {
                        const Base &base = Bases[base_id];
                        const int base_faction = base.faction_id_current;
                        if (base_faction != faction_id
                            && (PlayersData[base_faction]
                                    .diplo_treaties[faction_id] & 1u) == 0)
                            continue;

                        int dx = abs(selected_x - base.x);
                        if ((MapIsFlat & 1) == 0 && MapLongitude < dx)
                            dx = static_cast<int>(MapLongitudeBounds) - dx;
                        const int dy = abs(selected_y - base.y);
                        const int distance = (dx + dy) >> 1;
                        if (distance <= live_iVar10 &&
                            distance < best_distance) {
                            attacker.order = ORDER_MOVE_TO;
                            attacker.move_to_ai_type = 0x61;
                            attacker.waypoint_x[0] = base.x;
                            attacker.waypoint_y[0] = base.y;
                            best_distance = distance;
                            if (VehPrototypes[attacker.proto_id].plan
                                == PLAN_COLONIZATION)
                                attacker.terraforming_turns = 0;
                        }
                    }
                    if (attacker.order == ORDER_MOVE_TO) return -2;
                }
            }

            live_cVar21 = 0;
            if (VehPrototypes[attacker.proto_id].plan != PLAN_OFFENSIVE) {
                if (a2 == 0) return -3;
                action_home(a1, 0);
                return -2;
            }
        }
    }

    if ((attacker.state & VSTATE_UNK_200) != 0
        && attacker.order_auto_type == ORDERA_ON_ALERT) {
        if ((live_cVar21 == 0 || live_cVar21 == 6)
            && attacker.order != ORDER_MOVE_TO) {
            const int scan_count = live_iVar11 < 2 ? 8 : 24;
            int best_offense = 0;
            const int faction_id = attacker.faction_id;
            for (int offset = 1; offset <= scan_count; ++offset) {
                const int attacker_triad = Chassis[
                    VehPrototypes[attacker.proto_id].chassis_id].triad;
                int x = *a4 + RadiusOffsetX[offset];
                if ((MapIsFlat & 1) == 0) {
                    if (x < 0) x += MapLongitudeBounds;
                    else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
                }
                const int y = *a5 + RadiusOffsetY[offset];
                if (x < 0 || x >= MapLongitudeBounds
                    || y < 0 || y >= MapLatitudeBounds)
                    continue;

                const int stack = veh_at(x, y);
                const int defender_id = stack_fix(stack);
                if (defender_id < 0) continue;
                const Veh &defender = Vehs[defender_id];
                const int defender_faction = defender.faction_id;
                if (defender_faction == faction_id
                    || (PlayersData[defender_faction]
                            .diplo_treaties[faction_id] & 0x10) == 0)
                    continue;
                const int defender_chassis =
                    VehPrototypes[defender.proto_id].chassis_id;
                const int defender_triad = Chassis[defender_chassis].triad;
                if (defender_triad == TRIAD_AIR) continue;

                const Map *tile = map_loc(x, y);
                const uint8_t faction_bit = static_cast<uint8_t>(
                    1U << (faction_id & 0x1F));
                const bool attacker_has_full_map =
                    (PlayersData[faction_id].flags & 0x200u) != 0;
                const bool tile_known = (tile->visibility & faction_bit) != 0;
                const bool visible_target =
                    (((tile->bit & 1u) != 0 && (tile->val2 & 0x0Fu) < 8)
                     || (defender.visibility & faction_bit) != 0);
                const bool triad_terrain_match =
                    (((tile->climate & 0xE0u) < 0x60u)
                     == (attacker_triad == TRIAD_SEA));
                if (attacker_triad != TRIAD_AIR
                    && !((attacker_has_full_map || tile_known)
                         && visible_target && triad_terrain_match))
                    continue;

                int offense = offense_proto(defender_chassis, -1, 0);
                if (defender_triad == TRIAD_LAND) offense *= 3;
                int score = offense;
                if ((tile->climate & 0xE0u) < 0x60u
                    && stack_check(defender_id, 2, 9, -1, -1) != 0) {
                    score = offense * 2;
                    if ((PlayersData[faction_id].flags & 0x80000u) != 0)
                        score = offense * 4;
                }

                if (best_offense < score) {
                    attacker.order = ORDER_MOVE_TO;
                    attacker.move_to_ai_type = 0x4C;
                    attacker.waypoint_x[0] = static_cast<int16_t>(x);
                    attacker.waypoint_y[0] = static_cast<int16_t>(y);
                    best_offense = score;
                    if (VehPrototypes[attacker.proto_id].plan == 8)
                        attacker.terraforming_turns = 0;
                }
            }

            if (attacker.order == ORDER_MOVE_TO)
                return (a3 != 0) - 2;
        }

        if (attacker.order != ORDER_MOVE_TO
            && (attacker.x != attacker.waypoint_x[1]
                || attacker.y != attacker.waypoint_y[1])) {
            attacker.order = ORDER_MOVE_TO;
            attacker.move_to_ai_type = 0x6C;
            attacker.waypoint_x[0] = attacker.waypoint_x[1];
            attacker.waypoint_y[0] = attacker.waypoint_y[1];
            if (VehPrototypes[attacker.proto_id].plan == 8)
                attacker.terraforming_turns = 0;
            return -2;
        }
        return -4;
    }

    bombing_auto_state = attacker.state;
    }

air_power_bombing: ;
    const int faction_id = attacker.faction_id;
    const uint8_t faction_bit = static_cast<uint8_t>(
        1U << (attacker.faction_id & 0x1F));
    const uint8_t auto_bombing = ORDERA_BOMBING_RUN;
    const int attack_range = live_iVar10;
    const int search_range = live_iVar11;
    const int param_3_value = a3 != 0 ? 1 : 0;
    int best_base_x = -1;
    int best_base_y = -1;
    int best_score = 0;
    int best_base_route_state = 0;

    if (VehPrototypes[attacker.proto_id].plan == 0
        || ((bombing_auto_state & 0x200u) != 0
            && attacker.order_auto_type == auto_bombing)) {
        for (int enemy_base_id = 0;
             enemy_base_id < GameDataStateStorage.fields_.base_current_count_;
             ++enemy_base_id) {
            const Base &base = Bases[enemy_base_id];
            const int base_faction = base.faction_id_current;
            if (base_faction == faction_id
                || (PlayersData[faction_id].diplo_treaties[base_faction] & 7)
                       != 0)
                continue;
            if (((bombing_auto_state & 0x200u) == 0
                 || attacker.order_auto_type != auto_bombing)
                && (base.x != attacker.waypoint_x[1]
                    || base.y != attacker.waypoint_y[1]))
                continue;

            const int defender_stack = veh_at(base.x, base.y);
            const int defender_id = stack_fix(defender_stack);
            if (defender_id < 0) continue;
            if ((bombing_auto_state & 0x200u) == 0
                || attacker.order_auto_type != auto_bombing) {
                const int battle = battle_fight(a1, 0,
                    Vehs[defender_id].x, Vehs[defender_id].y, 0, 0, 0);
                int odds = compute_odds(battle, faction_id, a1,
                                        defender_id, -2);
                const Map *defender_tile = map_loc(Vehs[defender_id].x,
                                                   Vehs[defender_id].y);
                if ((defender_tile->bit & 1U) == 0
                    || (defender_tile->val2 & 0x0F) >= 8) {
                    odds /= 2;
                } else {
                    const int defender_count = stack_check(
                        defender_id, 0x13, -1, -1, -1);
                    const int stack_count = stack_check(
                        defender_id, 1, -1, -1, -1);
                    if (stack_count - defender_count != 1) odds /= 2;
                }
                if (odds < 8) continue;
            }

            const Map *base_tile = map_loc(base.x, base.y);
            const int region = base_tile->region;
            const int population_and_region_bases =
                static_cast<int>(base.population_size)
                + PlayersData[faction_id].region_total_bases[region] * 2;
            int score = population_and_region_bases * 0x20;
            bool has_nearby_friendly_base = false;
            for (int neighbor = 1; neighbor < 9; ++neighbor) {
                int neighbor_x = base.x + RadiusOffsetX[neighbor];
                if ((MapIsFlat & 1) == 0) {
                    if (neighbor_x < 0) neighbor_x += MapLongitudeBounds;
                    else if (neighbor_x >= MapLongitudeBounds)
                        neighbor_x -= MapLongitudeBounds;
                }
                const int neighbor_y = base.y + RadiusOffsetY[neighbor];
                if (neighbor_x < 0 || neighbor_x >= MapLongitudeBounds
                    || neighbor_y < 0 || neighbor_y >= MapLatitudeBounds)
                    continue;
                const Map *neighbor_tile = map_loc(neighbor_x, neighbor_y);
                if (neighbor_tile->region == region
                    && (neighbor_tile->bit & 2U) != 0
                    && (neighbor_tile->val2 & 0x0F) <= 7
                    && (neighbor_tile->val2 & 0x0F) == faction_id) {
                    has_nearby_friendly_base = true;
                    score = population_and_region_bases * 0x40 + 1;
                    break;
                }
            }

            int support_needs_halving = 0;
            if (!has_nearby_friendly_base
                && ((bombing_auto_state & 0x200u) == 0
                    || attacker.order_auto_type != auto_bombing)) {
                const int base_stack = veh_at(base.x, base.y);
                const int stack_count = stack_check(base_stack, 1,
                                                    -1, -1, -1);
                support_needs_halving =
                    PlayersData[faction_id].unk_105 < stack_count * 2;
                const int preq_tech = Ability[abil_index(0x10)].preq_tech;
                if (!has_tech(preq_tech, faction_id)) {
                    support_needs_halving = 1;
                } else if (support_needs_halving == 0) {
                    goto air_power_bombing_v2_scored_base;
                }
                score = (score + ((score >> 31) & 7)) >> 3;
            }
air_power_bombing_v2_scored_base:
            if ((base.event & faction_bit) != 0) score *= 4;
            if ((base.state & 8U) != 0) score *= 3;
            if ((base.state & 0x100000U) != 0) score *= 2;
            if (enemy_base_id == PlayersData[faction_id].base_id_atk_target)
                score *= 2;
            if (region == requested_region) score *= 2;
            if (PlayersData[faction_id].region_base_plan[region + 32] < 3)
                score <<= 2;

            int x_distance = abs(*a4 - base.x);
            if ((MapIsFlat & 1) == 0 && MapLongitude < x_distance)
                x_distance = MapLongitudeBounds - x_distance;
            const int y_distance = abs(*a5 - base.y);
            int travel_limit = attack_range / 2;
            if (travel_limit < 1) travel_limit = 1;
            else if (travel_limit > 99) travel_limit = 99;
            const int denominator =
                (((x_distance + y_distance) >> 1) - 1) / travel_limit + 1;
            score /= denominator;
            if (best_score <= score) {
                best_base_route_state = support_needs_halving;
                best_base_x = base.x;
                best_base_y = base.y;
                best_score = score;
            }
        }

        if (best_base_y >= 0 && best_base_y < MapLatitudeBounds
            && best_base_x >= 0 && best_base_x < MapLongitudeBounds) {
            int x_distance = abs(*a4 - best_base_x);
            if ((MapIsFlat & 1) == 0 && MapLongitude < x_distance)
                x_distance = MapLongitudeBounds - x_distance;
            const int y_distance = abs(*a5 - best_base_y);
            if (attack_range / 2 < ((x_distance + y_distance) >> 1)) {
                int nearest_friendly_base_distance = 9999;
                for (int friendly_base_id = 0;
                     friendly_base_id < GameDataStateStorage.fields_.base_current_count_;
                     ++friendly_base_id) {
                    const Base &base = Bases[friendly_base_id];
                    const int base_faction = base.faction_id_current;
                    if (base_faction != faction_id
                        && (PlayersData[faction_id]
                                .diplo_treaties[base_faction] & 1) == 0)
                        continue;
                    int distance_x = abs(best_base_x - base.x);
                    if ((MapIsFlat & 1) == 0 && MapLongitude < distance_x)
                        distance_x = MapLongitudeBounds - distance_x;
                    const int distance_y = abs(best_base_y - base.y);
                    const int distance = (distance_x + distance_y) >> 1;
                    if (distance <= search_range
                        && distance < nearest_friendly_base_distance) {
                        attacker.order = ORDER_MOVE_TO;
                        attacker.move_to_ai_type = 0x61;
                        attacker.waypoint_x[0] = base.x;
                        attacker.waypoint_y[0] = base.y;
                        nearest_friendly_base_distance = distance;
                        if (VehPrototypes[attacker.proto_id].plan == 8)
                            attacker.terraforming_turns = 0;
                    }
                }
            } else if (best_base_route_state == 0) {
                attacker.order = ORDER_MOVE_TO;
                attacker.move_to_ai_type = 0x61;
                attacker.waypoint_x[0] = static_cast<int16_t>(best_base_x);
                attacker.waypoint_y[0] = static_cast<int16_t>(best_base_y);
                if (VehPrototypes[attacker.proto_id].plan == 8)
                    attacker.terraforming_turns = 0;
            }
        }
    }

    if (attacker.order == ORDER_MOVE_TO) {
        if (param_3_value != 0
            && ((bombing_auto_state & 0x200u) == 0
                || attacker.order_auto_type != auto_bombing))
            return -1;
        return -2;
    }
    return -3;
}

// Inlined: the halved x_dist + |dy| reach test.
static inline int buster_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b) {
    int x_distance = x_dist(x_point_a, x_point_b);
    int y_distance = abs(y_point_a - y_point_b);
    int distance = (x_distance + y_distance) >> 1;
    return distance;
}

// ORIGINAL: 0x0056A8A0 ?enemy_planet_buster@@YAHH@Z 0x0056A8A0-0x0056B31A
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/unrecovered/0056a8a0.cpp", "recovery/tasks/alien-buster-owner-oct5.json", "pinned full native instruction/argument and directed-field review; compiler-observed current function symbol", "PlayerData sites[14].base_id alias: native addresses and compiler offsetof witness; original policy order checked by actual-product controls"], "reason": "Full vehicle-owner semantic recovery with actual Base, PlayerData, vehicle, map and callback relationships. Preserve original no-base adjacent-storage reads, native radius exits and uninitialized launch scratch; byte restoration and runtime fidelity remain unfinished."}
// symbol    ?enemy_planet_buster@@YAHH@Z
// TRIED: full-fidelity register-level transcription; direct struct fields (Vehs/Bases/VehPrototypes/Weapon layouts confirmed via src/veh.h, src/base.h) instead of raw address casts didn't close the divergence either; prologue shape differs (#0 push vs sub) from local/frame layout, not from any single wrong op.
int __cdecl enemy_planet_buster(int vehicle_id) {
    const int faction_id = Vehs[vehicle_id].faction_id;
    // The image tests only the mask's low byte; faction bits all live there.
    if ((1 << faction_id) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) {
        return -1;
    }
    int heavy_production_bases = 0;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current == faction_id
            && Bases[base_id].queue_production_id[0] >= 0
            && get_proto_offense_rating(Bases[base_id].queue_production_id[0]) >= 99) {
            heavy_production_bases++;
        }
    }
    int carried_heavy_vehicles = 0;
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        if (Vehs[veh_id].faction_id == faction_id && get_offense_rating(veh_id) >= 99) {
            carried_heavy_vehicles++;
        }
    }
    const int blast_radius = RadiusRange[planet_buster(vehicle_id)];
    // Assigned only by the reachable-own-base scan; pass 0 reads it uninitialized.
    int launch_base_id;
    for (int pass = 0; pass < 2; pass++) {
        int best_score = 0;
        int best_target_base = -1;
        int best_launch_base = 0;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            const int target_faction = Bases[base_id].faction_id_current;
            if (faction_id == target_faction) {
                continue;
            }
            const uint32_t treaty = PlayersData[faction_id].diplo_treaties[target_faction];
            if (treaty & DTREATY_PACT) {
                continue;
            }
            if (treaty & (DTREATY_PACT | DTREATY_TREATY | DTREATY_TRUCE)
                && (!(treaty & (DTREATY_WANT_REVENGE | DTREATY_SHALL_BETRAY | DTREATY_UNK_800))
                    || !at_climax(faction_id))) {
                continue;
            }
            int score = 0;
            int reachable = 0;
            if (!pass) {
                if (buster_dist(Vehs[vehicle_id].x, Vehs[vehicle_id].y, Bases[base_id].x, Bases[base_id].y)
                    > speed(vehicle_id, 0) / Rules.move_rate_roads) {
                    continue;
                }
            } else {
                for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                    if (Bases[i].faction_id_current == faction_id
                        && buster_dist(Bases[i].x, Bases[i].y, Bases[base_id].x, Bases[base_id].y)
                            <= speed(vehicle_id, 0) / Rules.move_rate_roads) {
                        reachable = 1;
                        launch_base_id = i;
                    }
                    if (Bases[i].faction_id_current == target_faction
                        && Bases[i].faction_id_former == faction_id) {
                        score++;
                    }
                }
                if (!reachable) {
                    continue;
                }
            }
            if (at_climax(faction_id)) {
                score += 2;
            }
            if (!(PlayersData[faction_id].diplo_treaties[target_faction] & DTREATY_WANT_REVENGE)
                && !(PlayersData[faction_id].flags & PFLAG_COMMIT_ATROCIT_WANTONLY)
                && score < 9 - ((GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)
                    ? 6 : PlayersData[target_faction].diff_level)
                && (!((1 << target_faction) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                    || (!(GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)
                        && (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER]
                                <= GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]
                            || PlayersData[faction_id].ai_fight < 1))
                    || (int)PlayersData[target_faction].ranking < (int)PlayersData[faction_id].ranking
                    || !reputation(target_faction, faction_id))) {
                continue;
            }
            if (!at_climax(faction_id) || PlayersData[faction_id].ai_fight < 0
                || !(GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)) {
                if (PlayersData[faction_id].diplo_treaties[target_faction] & DTREATY_UNK_80000) {
                    if (PlayersData[faction_id].major_atrocities >= PlayersData[target_faction].major_atrocities
                        && PlayersData[target_faction].mil_strength_1 * 2 >= PlayersData[faction_id].mil_strength_1
                        && !at_climax(faction_id)) {
                        if (score < 4
                            && GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER]
                                <= GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]) {
                            continue;
                        }
                        if ((int)PlayersData[target_faction].ranking > (int)PlayersData[faction_id].ranking) {
                            continue;
                        }
                    }
                } else if (!(PlayersData[faction_id].flags & PFLAG_COMMIT_ATROCIT_WANTONLY)) {
                    const int difficulty = (GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)
                        ? 6 : PlayersData[target_faction].diff_level;
                    if (PlayersData[target_faction].ai_fight
                            + (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER]
                                > GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]) * 4
                            + difficulty + score < 11) {
                        continue;
                    }
                    if (PlayersData[faction_id].mil_strength_1 * 2 >= PlayersData[target_faction].mil_strength_1) {
                        if (score < 4) {
                            continue;
                        }
                        if ((int)PlayersData[faction_id].ranking > (int)PlayersData[target_faction].ranking) {
                            continue;
                        }
                    }
                }
            }
            score = 0;
            for (int i = 0; i < blast_radius; i++) {
                int x = Bases[base_id].x + RadiusOffsetX[i];
                if (!(MapIsFlat & 1)) {
                    if (x < 0) {
                        x += MapLongitudeBounds;
                    } else if (x >= MapLongitudeBounds) {
                        x -= MapLongitudeBounds;
                    }
                }
                int y = Bases[base_id].y + RadiusOffsetY[i];
                if (!on_map(x, y)) {
                    continue;
                }
                int base_id_hit = base_at(x, y);
                if (base_id_hit >= 0) {
                    if (Bases[base_id_hit].faction_id_current == faction_id
                        || PlayersData[faction_id].diplo_treaties[Bases[base_id_hit].faction_id_current]
                            & (DTREATY_PACT | DTREATY_TREATY | DTREATY_TRUCE)) {
                        score = -9999;
                        break;
                    }
                    score += PlayersData[Bases[base_id_hit].faction_id_current].ranking
                        * Bases[base_id_hit].population_size;
                    if (has_fac(FAC_HEADQUARTERS, base_id_hit, 0)
                        && PlayersData[Bases[base_id_hit].faction_id_current].corner_market_turn
                            > GameDataStateStorage.fields_.turn_current_num_) {
                        score += 2000;
                    }
                    if (Bases[base_id_hit].queue_production_id[0] == -FAC_ASCENT_TO_TRANSCENDENCE) {
                        score += Bases[base_id_hit].minerals_accumulated * 2;
                    }
                    if (base_project(SP_HUNTER_SEEKER_ALGO) == base_id_hit) {
                        score += 1000;
                    }
                    if (base_project(SP_CLONING_VATS) == base_id_hit) {
                        score += 500;
                    }
                    for (int project_id = 0; project_id < MaxSecretProjectNum; project_id++) {
                        if (base_project(project_id) == base_id_hit) {
                            score += PlayersData[Bases[base_id_hit].faction_id_current].ranking * 60;
                        }
                    }
                }
                int veh_id_hit = stack_fix(veh_at(x, y));
                if (veh_id_hit < 0) {
                    if (!i) {
                        score = -9999;
                        break;
                    }
                } else if (Vehs[veh_id_hit].faction_id == faction_id) {
                    score -= stack_check(veh_id_hit, 7, -1, -1, -1) * PlayersData[faction_id].ranking;
                } else {
                    int value = stack_check(veh_id_hit, 7, -1, -1, -1)
                        * PlayersData[Vehs[veh_id_hit].faction_id].ranking;
                    int region = region_at(Vehs[veh_id_hit].x, Vehs[veh_id_hit].y);
                    if (region < 64) {
                        if (PlayersData[faction_id].region_base_plan[region] == 2) {
                            value *= 2;
                        } else if (PlayersData[faction_id].region_base_plan[region]) {
                            value /= 2;
                            if (!PlayersData[faction_id].region_total_bases[region]) {
                                value /= 2;
                            }
                        }
                    }
                    score += value;
                }
            }
            const int defense = PlayersData[target_faction].satellites_[3]
                - PlayersData[target_faction].sat_odp_deployed;
            if (carried_heavy_vehicles < defense + 1) {
                if (heavy_production_bases) {
                    continue;
                }
                score /= 2;
                if (carried_heavy_vehicles < defense / 2) {
                    score /= 2;
                }
            }
            if (score > best_score) {
                best_score = score;
                best_target_base = base_id;
                best_launch_base = launch_base_id;
            }
        }
        if (!pass) {
            // No best_target_base >= 0 guard, as in the original.
            if (best_score >= VehPrototypes[Vehs[vehicle_id].proto_id].cost
                    * PlayersData[faction_id].ranking) {
                int x_target = Bases[best_target_base].x;
                int y_target = Bases[best_target_base].y;
                for (int i = 0; i < 8; i++) {
                    int x = x_target + VehMoveDeltaX[i];
                    if (!(MapIsFlat & 1)) {
                        if (x < 0) {
                            x += MapLongitudeBounds;
                        } else if (x >= MapLongitudeBounds) {
                            x -= MapLongitudeBounds;
                        }
                    }
                    int y = y_target + VehMoveDeltaY[i];
                    if (on_map(x, y) && anything_at(x, y) < 0) {
                        veh_drop(veh_lift(vehicle_id), x, y);
                        Vehs[vehicle_id].visibility = 0;
                        return i ^ 4;
                    }
                }
            }
        } else if (best_launch_base >= 0 && Bases[best_launch_base].faction_id_current == faction_id) {
            veh_drop(veh_lift(vehicle_id), Bases[best_launch_base].x, Bases[best_launch_base].y);
            Vehs[vehicle_id].visibility = 0;
        }
    }
    return -1;
}

/*
Purpose: Determine whether specified unit can physically reach the destination coordinates.
// ORIGINAL: 0x0056B320 ?get_there@@YAHHHH@Z 0x0056B320-0x0056B47D BYTE_EXACT
// size      349 bytes
// prototype int (__cdecl ?get_there@@YAHHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x0050DDC0 0x0050DE50 0x0050E030 0x0050E160
Return Value: Can unit reach tile? true/false
Status: Complete
*/
BOOL __cdecl get_there(int veh_id, int x_dst, int y_dst) {
    uint8_t triad = get_triad(veh_id);
    if (triad == TRIAD_AIR) {
        return true;
    }
    uint32_t region_src = region_at(Vehs[veh_id].x, Vehs[veh_id].y);
    uint32_t region_dst = region_at(x_dst, y_dst);
    if (triad) {
        int base_id_src = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        int base_id_dst = base_at(x_dst, y_dst);
        if (base_id_src >= 0) {
            if (base_id_dst >= 0) {
                return port_to_port(base_id_src, base_id_dst);
            }
        } else if (base_id_dst >= 0) {
            return base_on_sea(base_id_dst, region_src);
        }
        if (!is_ocean(x_dst, y_dst)) {
            if (base_id_src >= 0) {
                return port_to_coast(base_id_src, region_dst);
            }
            return sea_coast(region_dst, region_src);
        }
        if (base_id_src >= 0) {
            return base_on_sea(base_id_src, region_dst);
        }
    }
    return (region_src == region_dst);
}

/*
Purpose: Return the first neighboring tile index that satisfies the coast/border checks, or 0.
         Earlier recovery prose treated the point-B comparison and integer result as unnecessary;
         the pinned function reads both regions and returns the successful neighbor index.
// ORIGINAL: 0x0056B480 ?coast_or_border@@YAHHHHHH@Z 0x0056B480-0x0056B5A9 BYTE_EXACT
// size      297 bytes
// prototype int (__cdecl ?coast_or_border@@YAHHHHHH@Z)(int xCoordPtA, int yCoordPtA, int xCoordPtB, int yCoordPtB, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0
Return Value: First qualifying neighbor index (1..8), or zero if none qualifies.
Status: Complete
*/
// Historical TRIED hypothesis below is contradicted by the pinned assembly: it checks ownership
// before rereading point A's region.
// TRIED: best 7/110, 0.826 similar (best flag set /O2 /Gy /GR- /GX; call
//            count already matches the image at 2, both to whose_territory).
//            The remaining gap looks like the `region_a != region_b` OR-term
//            being evaluated ahead of the `whose_territory(...)` call in the
//            image (a cheap loop-invariant compare scheduled before an
//            expensive call) - swapping the source's OR order to match
//            scored WORSE (0.780), so that is not the lever. Same
//            xrange/on_map/is_ocean-loop register-scheduling plateau as the
//            rest of this file otherwise. Not chased further.
int __cdecl coast_or_border(int x_point_a, int y_point_a, int x_point_b,
                            int y_point_b, int faction_id) {
    if (whose_territory(faction_id, x_point_a, y_point_a, NULL, false) != (int)faction_id) {
        return 0; // faction doesn't control point A
    }
    uint32_t region_b = region_at(x_point_b, y_point_b);
    for (int i = 1; i < 9; i++) {
        int x_radius = wrap_map_x(x_point_a + RadiusOffsetX[i]);
        int y_radius = y_point_a + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (is_ocean(x_radius, y_radius)
                || whose_territory(faction_id, x_radius, y_radius, NULL, false) != (int)faction_id
                || region_at(x_point_a, y_point_a) != region_b)) {
            return i;
        }
    }
    return 0;
}

// One source-owned automatic frame; only native proven stores initialize it.
// Semantic helper views bind references to these words without reading them.
struct EnemyMoveFullFrame {
    int vehicle_id;
    int local_c4, local_c0, local_bc, local_b8, local_b4, local_b0;
    int local_ac, local_a8, local_a4, local_a0, local_9c, local_98;
    int local_94, local_90, local_8c, local_88, local_84, local_80;
    int local_7c, local_78, local_74, local_70, local_6c;
    int local_68;
    int local_64, local_60, local_5c, local_58, local_54, local_50;
    int local_4c;
    union { int local_48; const short *base_cursor_48; };
    int local_44;
    int local_40, local_3c, local_38, local_34;
    int local_30;
    unsigned int local_2c;
    int local_28, local_24, local_20, local_1c, local_18, local_14;
    int local_10, local_c, local_8, local_4;
    int coast_y[64], coast_x[64];
};
typedef EnemyMoveFullFrame EnemyMoveNativeFrame;

struct EnemyMoveTerraformDirectionFacts {
    int direction;
    int x;
    int y;
    int terrain_owner;
    bool native_local_70_written;
    int native_local_70_value;
    int vehicle_zone_value;
    int movement_zone_value;
};

enum EnemyMoveNoWaypointJoin {
    ENEMY_MOVE_JOIN_578DE9,
    ENEMY_MOVE_JOIN_56C9E1,
    ENEMY_MOVE_JOIN_56C35D,
    ENEMY_MOVE_JOIN_572922,
    ENEMY_MOVE_JOIN_56C6D3,
    ENEMY_MOVE_JOIN_56BF5F,
    ENEMY_MOVE_RETURN_ZERO,
    ENEMY_MOVE_JOIN_578F46
};

enum EnemyMoveOrdinaryTerrainJoin {
    ENEMY_MOVE_TERRAIN_JOIN_578F46,
    ENEMY_MOVE_TERRAIN_RETURN_ZERO
};

static int enemy_move_damage_priority(int vehicle_id,
                                      EnemyMoveNativeFrame &native_frame) {
    const Veh &vehicle = Vehs[vehicle_id];
    const VehPrototype &design = VehPrototypes[vehicle.proto_id];
    const int damage = vehicle.dmg_incurred;
    const int total_power = design.plan == 12
        ? 1 : range(design.reactor_id, 1, 100) * 10;

    int priority;
    if (damage >= total_power / 2) priority = 4;
    else if (damage > total_power / 3) priority = 3;
    else if (damage > design.reactor_id * 2) priority = 2;
    else priority = damage != 0 ? 1 : 0;

    if (planet_buster(vehicle_id) != 0 ||
        vehicle.proto_id == 0x10 || vehicle.proto_id == 0x11 ||
        vehicle.proto_id == 0x12) {
        priority = 0;
    }
    native_frame.local_90 = priority;
    return priority;
}

static EnemyMoveTerraformDirectionFacts enemy_move_terraform_direction(
    int x, int y, int faction_id, int triad, int current_movement_zone,
    EnemyMoveFullFrame &frame) {
    EnemyMoveTerraformDirectionFacts facts;
    facts.direction = -1;
    facts.x = x;
    facts.y = y;
    facts.terrain_owner = -1;
    facts.native_local_70_written = false;
    facts.native_local_70_value = 0;
    facts.vehicle_zone_value = 0;
    facts.movement_zone_value = 0;

    for (int direction = 0; direction < 8; ++direction) {
        int target_x = x + VehMoveDeltaX[direction];
        if (!(MapIsFlat & 1)) {
            if (target_x < 0) target_x += MapLongitudeBounds;
            else if (target_x >= MapLongitudeBounds) target_x -= MapLongitudeBounds;
        }
        const int target_y = y + VehMoveDeltaY[direction];
        frame.local_4 = target_x;
        frame.local_8 = target_y;
        if (target_y < 0 || target_y >= MapLatitudeBounds || target_x < 0 ||
            target_x >= MapLongitudeBounds) {
            continue;
        }

        const Map *tile = map_loc(target_x, target_y);
        const bool low_altitude = (tile->climate & 0xe0u) < 0x60u;
        if (triad != 2 && low_altitude != (triad == 1)) continue;

        int terrain_owner = -1;
        if ((tile->bit & 3u) != 0) {
            terrain_owner = tile->val2 & 0x0f;
            if (terrain_owner >= 8) terrain_owner = -1;
        }
        // Native writes local -0x70 at 0x56BB3B/0x56BB4F/0x56BB5A
        // after the map-owner flags/low nibble are read, even if the later
        // pact/ZOC gates reject this neighbor. Preserve the last such write
        // across the full eight-direction scan; skipped earlier gates leave
        // the prior native local untouched.
        facts.native_local_70_written = true;
        facts.native_local_70_value = terrain_owner;
        frame.local_70 = terrain_owner;
        if (terrain_owner >= 0 && terrain_owner != faction_id &&
            (PlayersData[faction_id].diplo_treaties[terrain_owner] & 1u) == 0) {
            continue;
        }

        const int vehicle_zone = zoc_veh(target_x, target_y, faction_id);
        if (vehicle_zone != 0) continue;
        int movement_zone = 0;
        if (current_movement_zone != 0) {
            movement_zone = zoc_move(target_x, target_y, faction_id);
            if (movement_zone != 0) continue;
        }

        facts.direction = direction;
        facts.x = target_x;
        facts.y = target_y;
        facts.terrain_owner = terrain_owner;
        facts.vehicle_zone_value = vehicle_zone;
        facts.movement_zone_value = movement_zone;
        return facts;
    }
    return facts;
}

static bool enemy_move_downgrade_unresearched_magtube_order(
    int vehicle_id, int faction_id) {
    Veh &vehicle = Vehs[vehicle_id];
    if (vehicle.order_auto_type != 2 ||
        has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0], faction_id)) {
        return false;
    }
    vehicle.order_auto_type = 1;
    return true;
}

static bool enemy_move_revalidate_order_base_target(int vehicle_id) {
    Veh &vehicle = Vehs[vehicle_id];
    if (vehicle.order != 0x1b && vehicle.order != 0x1c) {
        return false;
    }
    const int base_id = base_at(vehicle.waypoint_x[0], vehicle.waypoint_y[0]);
    if (base_id >= 0 &&
        Bases[base_id].faction_id_current == vehicle.faction_id) {
        return true;
    }
    vehicle.order = 0;
    return false;
}

static EnemyMoveNoWaypointJoin enemy_move_no_waypoint_dispatch(
    int vehicle_id, int faction_id, int plan, int x = 0, int y = 0) {
    Veh &vehicle = Vehs[vehicle_id];
    const uint32_t state = vehicle.state;
    if ((state & 0x01000200u) == 0x01000200u ||
        ((state & 0x02000200u) == 0x02000200u &&
         vehicle.patrol_current_point < vehicle.waypoint_count)) {
        return ENEMY_MOVE_JOIN_578DE9;
    }

    const bool faction_locked =
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << faction_id)) != 0;
    bool jumped_to_common_order_dispatch = false;
    if (!faction_locked && (vehicle.flags & 0x0800u) != 0) {
        if (has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0], faction_id))
            vehicle.order_auto_type = 2;
        jumped_to_common_order_dispatch = true;
    }

    if (jumped_to_common_order_dispatch) {
        return enemy_move_revalidate_order_base_target(vehicle_id)
            ? ENEMY_MOVE_JOIN_578F46 : ENEMY_MOVE_JOIN_56C35D;
    }

    if ((state & 0x00000200u) == 0 || plan != 9) {
        return ENEMY_MOVE_JOIN_56C9E1;
    }

    switch (vehicle.order_auto_type) {
    case 3:
        return ENEMY_MOVE_JOIN_572922;
    case 4:
    case 5:
        return ENEMY_MOVE_JOIN_56C6D3;
    case 1:
        return enemy_move_revalidate_order_base_target(vehicle_id)
            ? ENEMY_MOVE_JOIN_578F46 : ENEMY_MOVE_JOIN_56C35D;
    case 2:
        enemy_move_downgrade_unresearched_magtube_order(
            vehicle_id, faction_id);
        return enemy_move_revalidate_order_base_target(vehicle_id)
            ? ENEMY_MOVE_JOIN_578F46 : ENEMY_MOVE_JOIN_56C35D;
    case 6:
        return ENEMY_MOVE_JOIN_56BF5F;
    default:
        {
        const bool uses_long_range_sensor =
            has_abil(vehicle.proto_id, 0x2000) != 0;
        if (uses_long_range_sensor && vehicle.order_auto_type == 0 &&
            (GameDataStateStorage.fields_.game_more_preferences_ & 8u) != 0) {
            return ENEMY_MOVE_JOIN_56BF5F;
        }
        if (vehicle.order_auto_type != 7) return ENEMY_MOVE_JOIN_56C9E1;

        if (good_sensor(faction_id, x, y)) {
            vehicle.order = 0x0d;
            return ENEMY_MOVE_JOIN_578F46;
        }
        int sensor_x = x;
        int sensor_y = y;
        if (Paths.sensors(faction_id, &sensor_x, &sensor_y) != 0) {
            vehicle.order = 0;
            vehicle.state &= ~0x00000200u;
            return ENEMY_MOVE_RETURN_ZERO;
        }
        if (sensor_x == x && sensor_y == y) {
            vehicle.order = 0x0d;
            return ENEMY_MOVE_JOIN_578F46;
        }
        go_to(vehicle_id, 0x73, sensor_x, sensor_y);
        return ENEMY_MOVE_RETURN_ZERO;
        }
    }
}

static EnemyMoveOrdinaryTerrainJoin enemy_move_ordinary_clear_order(
    Veh &vehicle) {
    vehicle.order = 0;
    vehicle.state &= ~0x00000200u;
    return ENEMY_MOVE_TERRAIN_RETURN_ZERO;
}

static EnemyMoveOrdinaryTerrainJoin enemy_move_ordinary_auto_terrain_policy(
    int vehicle_id, int faction_id, int current_x, int current_y,
    bool tile_below_altitude_3, int terrain_altitude) {
    Veh &vehicle = Vehs[vehicle_id];
    const Map *tile = map_loc(current_x, current_y);
    const int auto_order = vehicle.order_auto_type;
    const bool low = tile_below_altitude_3;

    if (auto_order == 4 || auto_order == 5) {
        const int climate_band = tile->climate & 0xe0;
        if (climate_band < 0x40 || (tile->bit & 0x2000u) != 0) {
            return enemy_move_ordinary_clear_order(vehicle);
        }
        if ((tile->bit & 1u) != 0 && (tile->val2 & 0x0f) < 8) {
            return enemy_move_ordinary_clear_order(vehicle);
        }

        if ((tile->bit & 0x20u) != 0) {
            const RulesTerraforming &remove_fungus =
                Terraforming[TERRA_REMOVE_FUNGUS];
            const int tech = low ? remove_fungus.prerequisite_tech[1]
                                 : remove_fungus.prerequisite_tech[0];
            if (has_tech(tech, faction_id)) {
                vehicle.order = 0x0e;
                return ENEMY_MOVE_TERRAIN_JOIN_578F46;
            }
        }

        if ((tile->bit & 0x20u) == 0 && !low &&
            (tile->bit & 0x8000u) == 0 &&
            (tile->val3 & 0xc0u) < 0x80u) {
            const int climate_low_bits = tile->climate & 0x18;
            if (climate_low_bits < 0x10 ||
                (terrain_altitude - 2 >= 0 && terrain_altitude - 2 > 1)) {
                if (has_tech(Terraforming[TERRA_FARM].prerequisite_tech[0],
                             faction_id)) {
                    vehicle.order = 0x04;
                    return ENEMY_MOVE_TERRAIN_JOIN_578F46;
                }
            }
        }
    }

    tile = map_loc(current_x,current_y);
    const bool improvements_allowed =
        (tile->bit & 0x20u) == 0 || (tile->climate & 0xe0u) < 0x40u ||
        has_tech(Rules.tech_improve_fungus_sqr, faction_id);
    if (improvements_allowed) {
        tile = map_loc(current_x,current_y);
        if (vehicle.order_auto_type == 4 && (tile->bit & 0x40u) == 0 &&
            energy_yield(faction_id, -1, current_x, current_y, 1) != 0) {
            const RulesTerraforming &solar = Terraforming[TERRA_SOLAR_TIDAL];
            const int solar_tech = low ? solar.prerequisite_tech[1] : solar.prerequisite_tech[0];
            if (has_tech(solar_tech, faction_id)) {
                vehicle.order = 0x07;
                return ENEMY_MOVE_TERRAIN_JOIN_578F46;
            }
        }

        tile = map_loc(current_x,current_y);
        if (vehicle.order_auto_type == 5 && (tile->bit & 0x10u) == 0) {
            const RulesTerraforming &mine = Terraforming[TERRA_MINE];
            const int mine_tech = low ? mine.prerequisite_tech[1] : mine.prerequisite_tech[0];
            if (has_tech(mine_tech, faction_id)) {
                vehicle.order = 0x06;
                return ENEMY_MOVE_TERRAIN_JOIN_578F46;
            }
        }
    }

    tile = map_loc(current_x,current_y);
    if (low) return enemy_move_ordinary_clear_order(vehicle);
    if ((tile->bit & 4u) == 0 &&
        ((tile->bit & 0x20u) == 0 || (tile->climate & 0xe0u) < 0x40u ||
         has_tech(Rules.tech_build_roads_fungus, faction_id)) &&
        has_tech(Terraforming[TERRA_ROAD].prerequisite_tech[0], faction_id)) {
        vehicle.order = 0x09;
        return ENEMY_MOVE_TERRAIN_JOIN_578F46;
    }

    tile = map_loc(current_x,current_y);
    if ((tile->bit & 0x8000u) != 0 ||
        (tile->val3 & 0xc0u) >= 0x80u ||
        !has_tech(Terraforming[TERRA_FARM].prerequisite_tech[0], faction_id)) {
        return enemy_move_ordinary_clear_order(vehicle);
    }
    vehicle.order = 0x04;
    return ENEMY_MOVE_TERRAIN_JOIN_578F46;
}

static int16_t enemy_move_route_coordinate(const Veh &vehicle, bool y_axis,
                                           unsigned int index) {
    if (index < 4) {
        return y_axis ? vehicle.waypoint_y[index] : vehicle.waypoint_x[index];
    }
    if (index == 4) {
        if (!y_axis) return vehicle.waypoint_y[0];
        int16_t value;
        memcpy(&value, &vehicle.morale, sizeof(value));
        return value;
    }
    // Preserve the original unchecked route-count access, including malformed
    // counts extending beyond the declared arrays. There is no native zero arm.
    return y_axis ? vehicle.waypoint_y[index] : vehicle.waypoint_x[index];
}

static void enemy_move_store_route_coordinate(Veh &vehicle, bool y_axis,
                                               unsigned int index,
                                               int16_t value) {
    if (index < 4) {
        if (y_axis) vehicle.waypoint_y[index] = value;
        else vehicle.waypoint_x[index] = value;
    } else if (index == 4) {
        if (!y_axis) vehicle.waypoint_y[0] = value;
        else memcpy(&vehicle.morale, &value, sizeof(value));
    } else {
        if (y_axis) vehicle.waypoint_y[index] = value;
        else vehicle.waypoint_x[index] = value;
    }
}

static void enemy_move_consume_route_waypoint(int vehicle_id) {
    Veh &vehicle = Vehs[vehicle_id];
    const uint32_t state = vehicle.state;
    const bool advance_route =
        (state & 0x02000200u) == 0x02000200u &&
        vehicle.patrol_current_point < vehicle.waypoint_count &&
        (state & 0x01000200u) != 0x01000200u;

    if (vehicle.order == 0x18) return;

    if (advance_route) {
        vehicle.order = 0x18;
        for (int index = 0; index < vehicle.waypoint_count; ++index) {
            enemy_move_store_route_coordinate(vehicle, false,
                static_cast<unsigned int>(index),
                enemy_move_route_coordinate(vehicle, false,
                    static_cast<unsigned int>(index + 1)));
            enemy_move_store_route_coordinate(vehicle, true,
                static_cast<unsigned int>(index),
                enemy_move_route_coordinate(vehicle, true,
                    static_cast<unsigned int>(index + 1)));
        }
        --vehicle.waypoint_count;
        if (vehicle.waypoint_count == 0) {
            vehicle.state &= ~0x02010200u;
        }
        return;
    }

    const int16_t first_x = enemy_move_route_coordinate(vehicle, false, 0);
    const int16_t first_y = enemy_move_route_coordinate(vehicle, true, 0);
    for (int index = 0; index < vehicle.waypoint_count; ++index) {
        enemy_move_store_route_coordinate(vehicle, false,
            static_cast<unsigned int>(index),
            enemy_move_route_coordinate(vehicle, false,
                static_cast<unsigned int>(index + 1)));
        enemy_move_store_route_coordinate(vehicle, true,
            static_cast<unsigned int>(index),
            enemy_move_route_coordinate(vehicle, true,
                static_cast<unsigned int>(index + 1)));
    }
    // Native stores at slot waypoint_count (not count - 1), including the
    // count-four aliases resolved by the member helpers above.
    enemy_move_store_route_coordinate(vehicle, false, vehicle.waypoint_count,
                                      first_x);
    enemy_move_store_route_coordinate(vehicle, true, vehicle.waypoint_count,
                                      first_y);
    vehicle.state &= ~0x00010000u;
    if (enemy_move_route_coordinate(vehicle, false, 0) == first_x &&
        enemy_move_route_coordinate(vehicle, true, 0) == first_y &&
        vehicle.waypoint_count == 1) {
        vehicle.order = 0;
        vehicle.state &= ~0x01000200u;
    } else {
        vehicle.order = 0x18;
    }
}

// Integration proposal: pinned enemy_move 574AC4..5767ED, with the
// logically complete colony-21 scan extending to57681D. No source claims.
#ifndef SEA_SCORING_HOST
#include "stdafx.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "general.h"
#include "map.h"
#include "veh.h"
#endif
#include <stddef.h>
#ifndef SEA_SCORING_HOST
typedef char SeaScoringPrototypeCategoryOffsetsMustMatchNative[
    (offsetof(VehPrototype,weapon_id)==0x25 &&
     offsetof(VehPrototype,reactor_id)==0x27 &&
     offsetof(VehPrototype,plan)==0x2a &&
     offsetof(VehPrototype,unk_1)==0x2b) ? 1 : -1];
typedef char SeaScoringVehicleDamageOffsetMustMatchNative[
    offsetof(Veh,dmg_incurred)==0x10 ? 1 : -1];
typedef char SeaScoringFactionRegionPlanOffsetMustMatchNative[
    offsetof(PlayerData,region_base_plan)==0x197c ? 1 : -1];
#endif
#include <stdlib.h>

enum SeaScoringExit {
    SEA_RETURN_ZERO, SEA_JOIN_578B34, SEA_JOIN_578B26,
    SEA_JOIN_57717F, SEA_JOIN_577071, SEA_JOIN_576DAF,
    SEA_JOIN_576835, SEA_JOIN_57681D
};

// All values are existing enemy_move locals, NOT independently initialized
// helper defaults. Native stack offsets below supersede Kuna's +4 comments.


static unsigned int sea_category_bit(int weapon) {
    return weapon == 1 ? 7u : 1u << (weapon & 31);
}
static int sea_base_region(int base) {
    return map_loc(Bases[base].x, Bases[base].y)->region;
}
static int sea_base_distance(const EnemyMoveFullFrame &enemy_frame, const Base &b) {
    return (abs(enemy_frame.local_14 - b.y) + x_dist(enemy_frame.local_18, b.x)) >> 1;
}
static int sea_base_connected(const EnemyMoveFullFrame &enemy_frame, int base) {
    return enemy_frame.local_64
        ? base_on_sea(base, enemy_frame.local_34)
        : port_to_port(enemy_frame.local_5c, base);
}

static SeaScoringExit enemy_move_57666a_tail(EnemyMoveFullFrame &enemy_frame);

static SeaScoringExit enemy_move_sea_scoring(EnemyMoveFullFrame &enemy_frame) {
    Veh &acting = Vehs[enemy_frame.vehicle_id];
    PlayerData &player = PlayersData[enemy_frame.local_c];
    // Entry574AC4 belongs to plan7. Entry574A88 initialized the five values
    // below before testing plan7; do not initialize capacity_limit/chosen_base.
    enemy_frame.local_6c = 0;
    enemy_frame.local_44 = 0;
    enemy_frame.local_54 = 0;
    enemy_frame.local_30 = 0;
    enemy_frame.local_2c = 0;
    if (!enemy_frame.local_64) stack_sort_2(enemy_frame.vehicle_id);
    enemy_frame.local_58 = 0;
    int count = 0;
    for (enemy_frame.local_1c = veh_top(enemy_frame.vehicle_id); enemy_frame.local_1c >= 0;
         enemy_frame.local_1c = Vehs[enemy_frame.local_1c].next_veh_id_stack) {
        const int id = enemy_frame.local_1c;
        if (id == enemy_frame.vehicle_id) continue;
        Veh &v = Vehs[id];
        VehPrototype &p = VehPrototypes[v.proto_id];
        if (Chassis[p.chassis_id].triad) continue;
        if (veh_cargo(id)) continue;
        if (v.faction_id != enemy_frame.local_c) continue;
        if ((v.state & 0x40000u) && !enemy_frame.local_7c &&
            (!(v.state & 0x20000u) || enemy_frame.local_88 != 7)) continue;
        // proto_power accepts VEHICLE ID, not prototype ID.
        const bool capacity = v.dmg_incurred <= proto_power(id) / 3;
        const int candidate_plan = p.plan;
        if (v.order != 2) {
            if (!enemy_frame.local_74 && candidate_plan == 12) {
                v.order = 2;
                continue;
            }
            if (enemy_frame.local_74 &&
                ((v.order == 1 &&
                  (v.waypoint_x[0] < 0 || v.waypoint_x[0] == enemy_frame.vehicle_id)) ||
                 (v.order == 0 && enemy_frame.local_7c))) {
                ++count;
                if (capacity) enemy_frame.local_2c |= sea_category_bit(candidate_plan);
                else ++enemy_frame.local_54;
                continue;
            }
            if (candidate_plan == 9) continue;
            if (candidate_plan == 2 && enemy_frame.local_44 < enemy_frame.local_48) {
                if (!enemy_frame.local_64) {
                    ++enemy_frame.local_44;
                    v.order = 2;
                    continue;
                }
            } else if (!enemy_frame.local_64 && !capacity) {
                v.order = 2;
                continue;
            }
            ++count;
            v.order = 0;
            if (capacity) enemy_frame.local_2c |= sea_category_bit(candidate_plan);
        } else {
            if (enemy_frame.local_64 ||
                (candidate_plan == 12 && !enemy_frame.local_74) || !capacity) continue;
            bool clear_order = candidate_plan > 8;
            if (!clear_order) {
                if (candidate_plan != 2 && !(v.state & 0x10000000u)) {
                    const int stack = stack_check(id, 2, 2, -1, -1);
                    if (stack || (p.plan != 1 && p.plan != 3)) {
                        clear_order = stack_check(id, 0x10, -1, -1, -1) != 0;
                    }
                }
                if (!clear_order) {
                    ++enemy_frame.local_44;
                    if (enemy_frame.local_44 <= enemy_frame.local_48 ||
                        (enemy_frame.local_88 <= 2 && enemy_frame.local_58)) continue;
                    ++enemy_frame.local_58;
                }
            }
            ++count;
            v.order = 0;
            enemy_frame.local_2c |= sea_category_bit(VehPrototypes[v.proto_id].plan);
        }
    }
    enemy_frame.local_6c = count;
    if (!count && acting.order == 0x18 &&
        (acting.move_to_ai_type == 'J' || acting.move_to_ai_type == 'r' ||
         acting.move_to_ai_type == 'I' || acting.move_to_ai_type == 'E'))
        acting.order = 0;
    if (enemy_frame.local_2c & 0x80fu) {
        enemy_frame.local_30 = 2;
        if ((enemy_frame.local_2c & 0x100u) && !(enemy_frame.local_2c & 3u)) enemy_frame.local_30 = 1;
    }
    if ((acting.move_to_ai_type == 'E' || acting.move_to_ai_type == 'r' ||
         acting.move_to_ai_type == 'I') && (!enemy_frame.local_30 || acting.order != 0x18))
        acting.move_to_ai_type = '-';

    if (!enemy_frame.local_8c &&
        !(acting.order == 0x18 && acting.move_to_ai_type == 'd')) {
        if (((enemy_frame.local_2c & 0x1000u) && enemy_frame.local_7c) ||
            (enemy_frame.local_54 && count <= enemy_frame.local_54 &&
             (enemy_frame.local_7c <= 8 || GameDataStateStorage.fields_.turn_current_num_ < 50))) {
            enemy_frame.local_28 = 9999;
            for (enemy_frame.local_24 = 0; enemy_frame.local_24 < GameDataStateStorage.fields_.base_current_count_; ++enemy_frame.local_24) {
                Base &b = Bases[enemy_frame.local_24];
                if (b.faction_id_current != enemy_frame.local_c ||
                    !base_on_sea(enemy_frame.local_24, enemy_frame.local_34)) continue;
                enemy_frame.local_38 = vector_dist(x_dist(enemy_frame.local_18, b.x), abs(enemy_frame.local_14 - b.y));
                if (!enemy_frame.local_38) return SEA_JOIN_578B34;
                // Native Base+2e dword spans unk_y and low state word.
                const unsigned int word = (unsigned short)b.unk_y |
                    ((b.state & 0xffffu) << 16);
                if (word & 0x400u) enemy_frame.local_38 += 1000;
                if (enemy_frame.local_38 < enemy_frame.local_28) {
                    enemy_frame.local_28 = enemy_frame.local_38;
                    go_to(enemy_frame.vehicle_id, 0x61, b.x, b.y);
                }
            }
        }
        int free_cargo = veh_cargo(enemy_frame.vehicle_id) - enemy_frame.local_6c;
        if ((acting.state & 0x20000000u) && !enemy_frame.local_6c) free_cargo = 0;
        bool scan_ports = free_cargo == veh_cargo(enemy_frame.vehicle_id);
        if (!scan_ports && !enemy_frame.local_64)
            scan_ports = veh_cargo(enemy_frame.vehicle_id) / 2 <= free_cargo &&
                         !(enemy_frame.local_2c & 0x100u);
        if (scan_ports) {
            enemy_frame.local_28 = 9999;
            enemy_frame.local_60 = 0;
            for (enemy_frame.local_24 = 0; enemy_frame.local_24 < GameDataStateStorage.fields_.base_current_count_; ++enemy_frame.local_24) {
                Base &b = Bases[enemy_frame.local_24];
                if (b.faction_id_current != enemy_frame.local_c || !is_port(enemy_frame.local_24, 0))
                    continue;
                enemy_frame.local_68 = sea_base_region(enemy_frame.local_24);
                if (player.region_base_plan[enemy_frame.local_68] == 2) continue;
                enemy_frame.local_38 = sea_base_distance(enemy_frame, b);
                enemy_frame.local_44 = enemy_frame.local_38;
                const bool same_waypoint = acting.waypoint_x[0] == b.x &&
                                           acting.waypoint_y[0] == b.y;
                const int threshold = enemy_frame.local_44 && !same_waypoint ? 1 : 2;
                bool try_base = false;
                if (player.x_target >= 0 && enemy_frame.local_68 ==
                    map_loc(player.x_target, player.y_target)->region) {
                    enemy_frame.local_4c = 1;
                    if ((player.flags & 0x800u) && b.x == player.x_target &&
                        b.y == player.y_target) {
                        try_base = stack_check(veh_at(b.x, b.y), 2, 7, -1, -1)
                                   < threshold;
                    }
                } else if (!b.unk_1 || (acting.order == 0x18 && same_waypoint) ||
                    (!enemy_frame.local_64 && enemy_frame.local_5c == enemy_frame.local_24 &&
                     stack_check(enemy_frame.vehicle_id, 2, 7, -1, -1) <= 1)) {
                    if (stack_check(veh_at(b.x, b.y), 2, 7, -1, -1) < threshold) {
                        enemy_frame.local_4c = 0;
                        try_base = true;
                    }
                }
                if (!try_base || !sea_base_connected(enemy_frame, enemy_frame.local_24)) continue;
                const int r = enemy_frame.local_68;
                const unsigned short combat = player.region_total_combat_vehs[r];
                const unsigned short flags = player.unk_77[r];
                if ((unsigned short)(player.region_total_bases[r] * 2 + 1) >=
                    combat && !(flags & 3u) && !(flags & 0x1800u)) continue;
                enemy_frame.local_38 = enemy_frame.local_44 + 4;
                if (player.region_base_plan[r] != 7 && !(flags & 0x800u) &&
                    !same_waypoint) {
                    if ((enemy_frame.local_24 + GameDataStateStorage.fields_.turn_current_num_ + enemy_frame.vehicle_id) & 7) continue;
                    enemy_frame.local_38 *= 2;
                }
                if ((enemy_frame.local_44 == 0 ? 1 : 0) <
                    stack_check(veh_at(b.x, b.y), 2, 7, enemy_frame.local_c, -1))
                    enemy_frame.local_38 *= 2;
                enemy_frame.local_38 += player.region_total_bases[r] * 2 - combat;
                if (enemy_frame.local_38 < enemy_frame.local_28) {
                    enemy_frame.local_78 = enemy_frame.local_24;
                    enemy_frame.local_58 = player.region_base_plan[r];
                    enemy_frame.local_60 = enemy_frame.local_4c;
                    enemy_frame.local_28 = enemy_frame.local_38;
                    go_to(enemy_frame.vehicle_id, 0x65, b.x, b.y);
                }
            }
            const int limit = enemy_frame.local_60 || enemy_frame.local_58 == 7 ? 6 : 2;
            const int passengers = range(limit, 0, veh_cargo(enemy_frame.vehicle_id));
            if (enemy_frame.local_60 || enemy_frame.local_28 <=
                veh_moves(enemy_frame.vehicle_id) * passengers / 2)
                assemble_passengers(enemy_frame.local_78, enemy_frame.local_58, passengers);
        }
    }

    if (acting.move_to_ai_type == 'e' && acting.waypoint_x[0] == enemy_frame.local_18 &&
        acting.waypoint_y[0] == enemy_frame.local_14)
        Bases[enemy_frame.local_9c].state |= 1u;
    if (enemy_frame.local_54 < enemy_frame.local_6c && acting.move_to_ai_type != 'E') {
        for (enemy_frame.local_58 = 0; enemy_frame.local_58 < 32; enemy_frame.local_58 += 4) {
            const int n = enemy_frame.local_58 / 4;
            enemy_frame.local_4 = wrap_map_x(enemy_frame.local_18 + VehMoveDeltaX[n]);
            enemy_frame.local_8 = enemy_frame.local_14 + VehMoveDeltaY[n];
            if (!on_map(enemy_frame.local_4, enemy_frame.local_8)) continue;
            Map *tile = map_loc(enemy_frame.local_4, enemy_frame.local_8);
            if ((tile->climate & 0xe0) < 0x60) continue;
            enemy_frame.local_68 = tile->region;
            const int r = enemy_frame.local_68;
            if ((r & 0x3f) == 0x3f || !(r & 0x3f)) continue;
            enemy_frame.local_54 = enemy_frame.local_90 > 2;
            int foreign = whose_territory(enemy_frame.local_c, enemy_frame.local_4, enemy_frame.local_8, 0, 0);
            if (foreign >= 0 && foreign != enemy_frame.local_c) {
                unsigned int treaty = player.diplo_treaties[foreign];
                if (treaty & 1u) continue;
                if (treaty & 0xc10u) {
                    if (enemy_frame.local_30) enemy_frame.local_54 = 1;
                } else {
                    if (treaty & 2u) {
                        if (!(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (unsigned char)(1u << (foreign & 31))) ||
                            !enemy_frame.local_30 || GameDataStateStorage.fields_.turn_current_num_ -
                            PlayersData[foreign].diplo_spoke[enemy_frame.local_c] < 16)
                            continue;
                        diplomacy_check(foreign, enemy_frame.local_c, 0);
                        if (!DiploScratchFA60 && !DiploScratchFA3C) continue;
                    } else if ((treaty & 4u) &&
                        (!(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (unsigned char)(1u << (foreign & 31))) ||
                         GameDataStateStorage.fields_.turn_current_num_ - PlayersData[foreign].diplo_spoke[enemy_frame.local_c] < 16))
                        continue;
                    // Original rereads treaty byte after diplomacy_check.
                    treaty = player.diplo_treaties[foreign];
                    if (!(treaty & 8u) && enemy_frame.local_30 && !(enemy_frame.local_2c & 0x100u))
                        enemy_frame.local_54 = 1;
                }
            }
            foreign = veh_who(enemy_frame.local_4, enemy_frame.local_8);
            if (foreign >= 0 && foreign != enemy_frame.local_c) continue;
            bool enter_mode = true;
            if ((enemy_frame.local_2c & 0x100u) && (int)Continents[r].open_terrain >= 4 &&
                (!player.region_total_bases[r] || player.region_base_plan[r] == 8)) {
                if (enemy_frame.local_30 || player.region_base_plan[enemy_frame.local_34] >= 3)
                    enemy_frame.local_54 = 1;
                else enter_mode = false;
            }
            if (enter_mode && enemy_frame.local_30) {
                if (!enemy_frame.local_54 && !player.region_total_bases[r] &&
                    player.region_base_plan[r] == 8) {
                    for (foreign = 1; foreign < 8; ++foreign) {
                        const unsigned int treaty = player.diplo_treaties[foreign];
                        if (PlayersData[foreign].region_total_bases[r] &&
                            ((treaty & 0x10u) || !(treaty & 8u) ||
                             GameDataStateStorage.fields_.turn_current_num_ - PlayersData[foreign].diplo_spoke[enemy_frame.local_c] > 16))
                            enemy_frame.local_54 = 1;
                    }
                }
                if (player.region_base_plan[r] < 3) enemy_frame.local_54 = 1;
            }
            if (!enemy_frame.local_54) continue;
            int movement_gate = enemy_frame.local_54;
            if (!(enemy_frame.local_2c & (1u << (player.region_base_plan[r] & 31))) &&
                enemy_frame.local_90 <= 2 && !(GameDataStateStorage.fields_.game_state_ & 0x100u) &&
                (acting.order != 0x18 || r != map_loc(
                    acting.waypoint_x[0], acting.waypoint_y[0])->region ||
                 acting.move_to_ai_type == 'e')) {
                enemy_frame.local_1c = veh_top(enemy_frame.vehicle_id);
                bool reject_tile = false;
                for (; enemy_frame.local_1c >= 0;
                     enemy_frame.local_1c = Vehs[enemy_frame.local_1c].next_veh_id_stack) {
                    const Veh &v = Vehs[enemy_frame.local_1c];
                    if (Chassis[VehPrototypes[v.proto_id].chassis_id].triad ||
                        v.order != 1 || (v.waypoint_x[0] >= 0 &&
                        v.waypoint_x[0] != enemy_frame.vehicle_id) || v.home_base_id < 0 ||
                        sea_base_region(v.home_base_id) != r) continue;
                    foreign = zoc_veh(enemy_frame.local_4, enemy_frame.local_8, enemy_frame.local_c);
                    if (foreign <= 1) { reject_tile = true; break; }
                    const unsigned int treaty = player.diplo_treaties[foreign - 1];
                    if (((!(treaty & 0x10u) || !enemy_frame.local_30 || !(enemy_frame.local_2c & 3u)) &&
                         (treaty & 8u)) || !enemy_frame.local_54) {
                        reject_tile = true;
                        break;
                    }
                    movement_gate = -1;
                    break;
                }
                if (reject_tile) continue;
            }
            enemy_frame.local_80 = -1;
            if (veh_moves(enemy_frame.vehicle_id) >= (int)Rules.move_rate_roads &&
                enemy_frame.local_7c && movement_gate >= 1) {
                enemy_frame.local_28 = -1;
                enemy_frame.local_30 = 0;
                for (enemy_frame.local_30 = 0; enemy_frame.local_30 < 9; ++enemy_frame.local_30) {
                    const int pos = enemy_frame.local_30;
                    enemy_frame.local_4 = wrap_map_x(enemy_frame.local_18 + RadiusOffsetX[pos]);
                    enemy_frame.local_8 = enemy_frame.local_14 + RadiusOffsetY[pos];
                    if (!on_map(enemy_frame.local_4, enemy_frame.local_8) ||
                        map_loc(enemy_frame.local_4, enemy_frame.local_8)->region != enemy_frame.local_34) continue;
                    const int owner = veh_who(enemy_frame.local_4, enemy_frame.local_8);
                    if (owner >= 0 && veh_who(enemy_frame.local_4, enemy_frame.local_8) != enemy_frame.local_c)
                        continue;
                    enemy_frame.local_20 = 0;
                    for (int d = 1; d <= 8; ++d) {
                        const int nx = wrap_map_x(enemy_frame.local_4 + RadiusOffsetX[d]);
                        const int ny = enemy_frame.local_8 + RadiusOffsetY[d];
                        if (!on_map(nx, ny) || map_loc(nx, ny)->region != r) continue;
                        if (base_who(nx, ny) >= 1) enemy_frame.local_20 += 4;
                        ++enemy_frame.local_20;
                    }
                    if (enemy_frame.local_28 < enemy_frame.local_20) {
                        enemy_frame.local_28 = enemy_frame.local_20;
                        enemy_frame.local_80 = pos - 1;
                    }
                }
            }
            for (enemy_frame.local_1c = veh_top(enemy_frame.vehicle_id); enemy_frame.local_1c >= 0;
                 enemy_frame.local_1c = Vehs[enemy_frame.local_1c].next_veh_id_stack) {
                Veh &v = Vehs[enemy_frame.local_1c];
                if (!Chassis[VehPrototypes[v.proto_id].chassis_id].triad)
                    v.order = 0;
            }
            acting.move_to_ai_type = 'u';
            return enemy_frame.local_b8 ? SEA_RETURN_ZERO : SEA_JOIN_578B26;
        }
    }
    if (enemy_frame.local_b8) return SEA_RETURN_ZERO;
    if (enemy_frame.local_8c) return SEA_JOIN_57717F;
    if (acting.order == 0x18 && acting.move_to_ai_type == 'd')
        return SEA_JOIN_57717F;
    if (acting.order != 0x18 && (enemy_frame.local_2c & 0x100u) && enemy_frame.local_30 <= 0) {
        for (int coast_index = 0; coast_index < 64; ++coast_index)
            enemy_frame.coast_x[coast_index] = enemy_frame.coast_y[coast_index] = -1;
        for (enemy_frame.local_8 = 1; enemy_frame.local_8 < MapLatitudeBounds - 1; ++enemy_frame.local_8) {
            for (enemy_frame.local_4 = enemy_frame.local_8 & 1; enemy_frame.local_4 < MapLongitudeBounds;
                 enemy_frame.local_4 += 2) {
                const int r = map_loc(enemy_frame.local_4, enemy_frame.local_8)->region;
                if (r <= 0x3f && (enemy_frame.coast_y[r] < 10 ||
                    MapLatitudeBounds - 10 < enemy_frame.coast_y[r]) &&
                    is_coast(enemy_frame.local_4, enemy_frame.local_8, 0)) {
                    enemy_frame.coast_x[r] = enemy_frame.local_4;
                    enemy_frame.coast_y[r] = enemy_frame.local_8;
                }
            }
        }
        enemy_frame.local_78 = -1;
        enemy_frame.local_28 = 0;
        enemy_frame.local_68 = 1;
        enemy_frame.local_58 = 0;
        for (int r = 1; r < 64; ++r, enemy_frame.local_58 += 4, ++enemy_frame.local_68) {
            const int tiles = Continents[r].tile_count;
            if (tiles < 10 || player.region_total_bases[r] ||
                !on_map(enemy_frame.coast_x[r], enemy_frame.coast_y[r])) continue;
            int enemies = 0;
            for (int f = 1; f < 8; ++f) {
                const int bases = PlayersData[f].region_total_bases[r];
                if (!bases) continue;
                enemies += tiles / 4 < bases * 20 ? 3 : 1;
                if (player.diplo_treaties[f] & 7u) enemies += 2;
            }
            if (enemies > 1) continue;
            const int distance = vector_dist(x_dist(enemy_frame.local_18, enemy_frame.coast_x[r]),
                                            abs(enemy_frame.local_14 - enemy_frame.coast_y[r]));
            const int rating = tiles * 100 / (distance + 1);
            if (enemy_frame.local_28 <= rating) { enemy_frame.local_78 = r; enemy_frame.local_28 = rating; }
        }
        if (enemy_frame.local_78 >= 0) set_course(enemy_frame.vehicle_id, 99,
            enemy_frame.coast_x[enemy_frame.local_78], enemy_frame.coast_y[enemy_frame.local_78]);
    }
    if (acting.order != 0x18 && enemy_frame.local_30) {
        enemy_frame.local_28 = 9999;
        for (enemy_frame.local_24 = 0; enemy_frame.local_24 < GameDataStateStorage.fields_.base_current_count_; ++enemy_frame.local_24) {
            Base &b = Bases[enemy_frame.local_24];
            if (b.faction_id_current != enemy_frame.local_c) continue;
            enemy_frame.local_68 = sea_base_region(enemy_frame.local_24);
            if (player.region_base_plan[enemy_frame.local_68] != 2 ||
                !sea_base_connected(enemy_frame, enemy_frame.local_24)) continue;
            enemy_frame.local_38 = sea_base_distance(enemy_frame, b);
            if (veh_who(b.x, b.y) < 0) enemy_frame.local_38 /= 2;
            if (at_goal(enemy_frame.local_c, 2, b.x, b.y)) enemy_frame.local_38 /= 2;
            if (player.base_id_atk_target >= 0) {
                const int target = player.base_id_atk_target;
                if ((player.diplo_treaties[Bases[target].faction_id_current] &
                     0x10u) && sea_base_region(target) == enemy_frame.local_68)
                    enemy_frame.local_38 = 0;
            }
            if (enemy_frame.local_38 < enemy_frame.local_28) {
                enemy_frame.local_28 = enemy_frame.local_38;
                go_to(enemy_frame.vehicle_id, 'E', b.x, b.y);
            }
        }
        if (acting.order != 0x18 && enemy_frame.local_30 >= 2) {
            enemy_frame.local_28 = 9999;
            for (enemy_frame.local_24 = 0; enemy_frame.local_24 < GameDataStateStorage.fields_.base_current_count_; ++enemy_frame.local_24) {
                Base &b = Bases[enemy_frame.local_24];
                const int f = b.faction_id_current;
                if (f == enemy_frame.local_c || !(is_objective(enemy_frame.local_24) ||
                    (player.flags & 0x200000u) ||
                    player.base_id_atk_target == enemy_frame.local_24 ||
                    !((enemy_frame.local_24 + GameDataStateStorage.fields_.turn_current_num_) & 3))) continue;
                // Native57602D repurposes -30 from mode into the faction's
                // dword-table stride. Retain the observable reaching integer.
                enemy_frame.local_30 = enemy_frame.local_c * 0x833;
                const unsigned int treaty = player.diplo_treaties[f];
                const bool human = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ &
                    (unsigned char)(1u << (f & 31))) != 0;
                if ((treaty & 1u) || ((treaty & 2u) &&
                    (!human || !(treaty & 0xc00u))) ||
                    ((treaty & 4u) && (!human || (!(treaty & 0xc00u) &&
                    GameDataStateStorage.fields_.turn_current_num_ - PlayersData[f].diplo_spoke[enemy_frame.local_c] < 16))))
                    continue;
                const int r = sea_base_region(enemy_frame.local_24);
                if (r == enemy_frame.local_34 && !enemy_frame.local_74) continue;
                if (!(enemy_frame.local_64 ? sea_coast(r, enemy_frame.local_34) :
                    port_to_coast(enemy_frame.local_5c, r))) continue;
                enemy_frame.local_54 = 1;
                for (enemy_frame.local_1c = veh_top(enemy_frame.vehicle_id); enemy_frame.local_1c >= 0;
                     enemy_frame.local_1c = Vehs[enemy_frame.local_1c].next_veh_id_stack) {
                    const int home = Vehs[enemy_frame.local_1c].home_base_id;
                    if (home >= 0 && sea_base_region(home) == r) enemy_frame.local_54 = 0;
                }
                if (!enemy_frame.local_54 && !(treaty & 0x10u) && (treaty & 8u) &&
                    !(GameDataStateStorage.fields_.game_state_ & 0x100u) &&
                    !(enemy_frame.local_2c & (1u << (player.region_base_plan[r] & 31)))) continue;
                const int distance = sea_base_distance(enemy_frame, b);
                enemy_frame.local_38 = distance * 32;
                if (f != enemy_frame.local_c && !(b.visibility &
                    (unsigned char)(1u << (enemy_frame.local_c & 31)))) enemy_frame.local_38 = distance * 64;
                if (b.queue_production_id[0] == -102) enemy_frame.local_38 /= 4;
                if (has_fac(1, enemy_frame.local_24, 0)) {
                    enemy_frame.local_38 /= 2;
                    if (player.flags & 0x200000u) enemy_frame.local_38 /= 2;
                    if (GameDataStateStorage.fields_.turn_current_num_ < PlayersData[f].corner_market_turn)
                        enemy_frame.local_38 /= 8;
                }
                if (is_objective(enemy_frame.local_24)) {
                    enemy_frame.local_38 /= 2;
                    if (player.flags & 0x80000000u) enemy_frame.local_38 /= 2;
                }
                if (treaty & 0x40u) enemy_frame.local_38 /= 2;
                if (treaty & 0x10u) {
                    enemy_frame.local_38 /= 2;
                    if (veh_who(b.x, b.y) < 0) enemy_frame.local_38 /= 2;
                    if (player.base_id_atk_target == enemy_frame.local_24) enemy_frame.local_38 = 0;
                }
                if (has_fac(1, enemy_frame.local_24, 0)) enemy_frame.local_38 /= 2;
                enemy_frame.local_1c = veh_at(b.x, b.y);
                bool low_garrison = enemy_frame.local_1c < 0;
                if (!low_garrison) {
                    enemy_frame.local_4c = stack_check(enemy_frame.local_1c, 0xe, -1, -1, -1);
                    low_garrison = enemy_frame.local_4c < garrison_check(enemy_frame.local_24);
                }
                if (low_garrison && human && (PlayersData[f].diff_level >= 4 ||
                    (player.flags & 0x200u) || (treaty & 0x1000u))) enemy_frame.local_38 /= 2;
                if (enemy_frame.local_38 < enemy_frame.local_28) {
                    enemy_frame.local_28 = enemy_frame.local_38;
                    set_course(enemy_frame.vehicle_id, 'I', b.x, b.y);
                }
            }
        }
        if (acting.order != 0x18) {
            enemy_frame.local_28 = 9999;
            for (enemy_frame.local_24 = 0; enemy_frame.local_24 < GameDataStateStorage.fields_.base_current_count_; ++enemy_frame.local_24) {
                Base &b = Bases[enemy_frame.local_24];
                if (b.faction_id_current != enemy_frame.local_c) continue;
                const int r = sea_base_region(enemy_frame.local_24);
                const int p = player.region_base_plan[r];
                if (r == enemy_frame.local_34 || p == 7 || (enemy_frame.local_2c & 0x100u) ||
                    !(enemy_frame.local_2c & (1u << (p & 31))) ||
                    !sea_base_connected(enemy_frame, enemy_frame.local_24)) continue;
                enemy_frame.local_38 = sea_base_distance(enemy_frame, b);
                if (enemy_frame.local_38 < enemy_frame.local_28) {
                    enemy_frame.local_28 = enemy_frame.local_38;
                    go_to(enemy_frame.vehicle_id, 'r', b.x, b.y);
                }
            }
        }
        if (acting.order != 0x18) {
            for (int pos = 1; pos <= 288; ++pos) {
                enemy_frame.local_4 = wrap_map_x(enemy_frame.local_18 + RadiusOffsetX[pos] * 2);
                enemy_frame.local_8 = enemy_frame.local_14 + RadiusOffsetY[pos] * 2;
                if (!on_map(enemy_frame.local_4, enemy_frame.local_8)) continue;
                Map *tile = map_loc(enemy_frame.local_4, enemy_frame.local_8);
                const int r = tile->region;
                if ((tile->climate & 0xe0) < 0x60 || (r & 0x3f) == 0x3f ||
                    !(r & 0x3f) || player.region_total_bases[r] ||
                    !(enemy_frame.local_2c & (1u << (player.region_base_plan[enemy_frame.local_34] & 31))))
                    continue;
                if (!(enemy_frame.local_64 ? sea_coast(r, enemy_frame.local_34) :
                    port_to_coast(enemy_frame.local_5c, r))) continue;
                set_course(enemy_frame.vehicle_id, 'J', enemy_frame.local_4, enemy_frame.local_8);
                break;
            }
        }
    }
    return enemy_move_57666a_tail(enemy_frame);
}

// Other plans reaching57666A must call this tail directly, retaining their
// live score and without running plan7 initialization or stack mutation.
static SeaScoringExit enemy_move_57666a_tail(EnemyMoveFullFrame &enemy_frame) {
    Veh &acting = Vehs[enemy_frame.vehicle_id];
    if (acting.order == 0x18 && acting.move_to_ai_type == 'd')
        return SEA_JOIN_577071;
    if (enemy_frame.local_50 != 6) return SEA_JOIN_576DAF;
    if (VehPrototypes[acting.proto_id].unk_1 != 21) return SEA_JOIN_576835;
    enemy_frame.local_28 = 9999;
    for (enemy_frame.local_1c = 0; enemy_frame.local_1c < GameDataStateStorage.fields_.vehicle_current_count_;
         ++enemy_frame.local_1c) {
        const Veh &v = Vehs[enemy_frame.local_1c];
        if (v.faction_id != enemy_frame.local_c ||
            VehPrototypes[v.proto_id].plan != 7) continue;
        enemy_frame.local_4 = v.x;
        enemy_frame.local_8 = v.y;
        if ((map_loc(enemy_frame.local_4, enemy_frame.local_8)->climate & 0xe0) >= 0x60 ||
            base_who(enemy_frame.local_4, enemy_frame.local_8) >= 0) continue;
        enemy_frame.local_38 = (abs(enemy_frame.local_14 - enemy_frame.local_8) + x_dist(enemy_frame.local_18, enemy_frame.local_4)) >> 1;
        if (enemy_frame.local_38) {
            if (stack_check(enemy_frame.local_1c, 2, 6, -1, -1)) continue;
            // The original divisor can be zero. Do not insert a guard.
            enemy_frame.local_38 /= stack_check(enemy_frame.local_1c, 1, -1, -1, -1);
        }
        if (enemy_frame.local_38 < enemy_frame.local_28) {
            enemy_frame.local_28 = enemy_frame.local_38;
            go_to(enemy_frame.vehicle_id, 'C', enemy_frame.local_4, enemy_frame.local_8);
        }
    }
    return SEA_JOIN_57681D;
}

// Complete57681D/576835/576DAF continuation. Include aftersea_scoring.cpp.
// Result joins are the original shared577071 split or578B34 action owner.
static SeaScoringExit enemy_move_sea_after_scoring(
    EnemyMoveFullFrame &enemy_frame,SeaScoringExit entry) {
    Veh &acting=Vehs[enemy_frame.vehicle_id];
    PlayerData &player=PlayersData[enemy_frame.local_c];
    if(entry==SEA_JOIN_576DAF)goto sea576daf;
    if(entry==SEA_JOIN_57681D){
        if(acting.order==0x18)return enemy_frame.local_38?SEA_JOIN_577071:SEA_JOIN_578B34;
    } else if(entry==SEA_JOIN_576835){
        if(acting.order==0x18&&acting.move_to_ai_type!='6')goto sea576a95;
    }
    //576847: faction rule100 gates a complete hostile-base target scan.
    if(Players[enemy_frame.local_c].rule_flags&0x100u){
        enemy_frame.local_28=0;enemy_frame.local_78=-1;
        for(enemy_frame.local_24=0;enemy_frame.local_24<GameDataStateStorage.fields_.base_current_count_;++enemy_frame.local_24){
            const Base &b=Bases[enemy_frame.local_24];
            if(b.faction_id_current==enemy_frame.local_c||
                !(player.diplo_treaties[b.faction_id_current]&0x10u))continue;
            // Original nonzero-distance arm checks the ORIGINAL friendly
            //base, not the current foreign candidate. Preserve that quirk.
            if(!(enemy_frame.local_64?
                base_on_sea(enemy_frame.local_5c,enemy_frame.local_34):
                port_to_port(enemy_frame.local_5c,enemy_frame.local_24)))continue;
            const int distance=(abs(enemy_frame.local_14-b.y)+x_dist(enemy_frame.local_18,b.x))>>1;
            enemy_frame.local_20=(b.population_size*b.population_size*32)/(distance+1);
            for(enemy_frame.local_1c=0;enemy_frame.local_1c<GameDataStateStorage.fields_.vehicle_current_count_;++enemy_frame.local_1c){
                if(enemy_frame.local_1c==enemy_frame.vehicle_id)continue;
                const Veh &v=Vehs[enemy_frame.local_1c];
                if(v.faction_id!=enemy_frame.local_c||VehPrototypes[v.proto_id].plan!=6)continue;
                const int delta=abs(v.y-b.y)+x_dist(v.x,b.x);
                if((delta&~1)<=4 || (v.move_to_ai_type=='6'&&
                    v.waypoint_x[0]==b.x&&v.waypoint_y[0]==b.y))enemy_frame.local_20/=2;
            }
            if(enemy_frame.local_28<enemy_frame.local_20){enemy_frame.local_78=enemy_frame.local_24;enemy_frame.local_28=enemy_frame.local_20;
                go_to(enemy_frame.vehicle_id,'6',b.x,b.y);}
        }
        if(enemy_frame.local_78>=0){const Base &b=Bases[enemy_frame.local_78];
            if(((abs(enemy_frame.local_14-b.y)+x_dist(enemy_frame.local_18,b.x))&~1)<=2){
                acting.order=0;return SEA_JOIN_577071;}}
    }
sea576a95:
    if(acting.order!=0x18&&!((enemy_frame.vehicle_id+GameDataStateStorage.fields_.turn_current_num_)&3)){
        enemy_frame.local_28=-9999;
        for(enemy_frame.local_8=0;enemy_frame.local_8<MapLatitudeBounds;++enemy_frame.local_8){
            for(enemy_frame.local_4=enemy_frame.local_8&1;enemy_frame.local_4<MapLongitudeBounds;enemy_frame.local_4+=2){
                const Map &tile=*map_loc(enemy_frame.local_4,enemy_frame.local_8);
                if((tile.climate&0xe0)>=0x60)continue;
                if(enemy_frame.local_64){if(tile.region!=enemy_frame.local_34)continue;}
                else if(!base_on_sea(enemy_frame.local_5c,tile.region))continue;
                if(((abs(enemy_frame.local_14-enemy_frame.local_8)+x_dist(enemy_frame.local_18,enemy_frame.local_4))&~1)<16)continue;
                bool assigned=false;
                for(enemy_frame.local_1c=0;enemy_frame.local_1c<GameDataStateStorage.fields_.vehicle_current_count_;++enemy_frame.local_1c){
                    const Veh &v=Vehs[enemy_frame.local_1c];
                    if(enemy_frame.local_1c!=enemy_frame.vehicle_id&&v.faction_id==enemy_frame.local_c&&
                        v.waypoint_x[0]==enemy_frame.local_4&&v.waypoint_y[0]==enemy_frame.local_8){assigned=true;break;}}
                if(assigned)continue;
                enemy_frame.local_20=0;
                for(enemy_frame.local_44=0;enemy_frame.local_44<=0x11f;enemy_frame.local_44+=4){
                    const int pos=9+enemy_frame.local_44/4;
                    const int x=wrap_map_x(enemy_frame.local_4+RadiusOffsetX[pos]);
                    const int y=enemy_frame.local_8+RadiusOffsetY[pos];
                    if(!on_map(x,y)||(map_loc(x,y)->climate&0xe0)<0x60)continue;
                    const int ox=wrap_map_x(enemy_frame.local_4-RadiusOffsetX[pos]);
                    const int oy=enemy_frame.local_8-RadiusOffsetY[pos];
                    if(!on_map(ox,oy)||(map_loc(ox,oy)->climate&0xe0)<0x60)continue;
                    const int r=map_loc(x,y)->region;
                    const int opposite=map_loc(ox,oy)->region;
                    if(player.region_total_bases[r]<2||player.region_total_bases[opposite]>1)continue;
                    for(int f=1;f<8;++f){
                        if(f!=enemy_frame.local_c&&(player.diplo_treaties[f]&0xc10u)&&
                            PlayersData[f].region_total_bases[r]<=1&&
                            PlayersData[f].region_total_bases[opposite]>=2)++enemy_frame.local_20;
                    }
                }
                if(enemy_frame.local_20){
                    const int distance=vector_dist(x_dist(enemy_frame.local_18,enemy_frame.local_4),abs(enemy_frame.local_14-enemy_frame.local_8));
                    // Original signed divide remains unguarded.
                    enemy_frame.local_38=(enemy_frame.local_20<<8)/distance;
                    if(enemy_frame.local_28<enemy_frame.local_38){enemy_frame.local_28=enemy_frame.local_38;go_to(enemy_frame.vehicle_id,'D',enemy_frame.local_4,enemy_frame.local_8);}
                }
            }
        }
    }
sea576daf:
    if(acting.order==0x18&&(!(player.flags&0x20000000u)||
        stack_check(enemy_frame.vehicle_id,1,-1,-1,-1)!=1))return SEA_JOIN_577071;
    if((enemy_frame.vehicle_id+GameDataStateStorage.fields_.turn_current_num_)&3)return SEA_JOIN_577071;
    enemy_frame.local_28=-9999;
    for(enemy_frame.local_8=0;enemy_frame.local_8<MapLatitudeBounds;++enemy_frame.local_8){
        for(enemy_frame.local_4=enemy_frame.local_8&1;enemy_frame.local_4<MapLongitudeBounds;enemy_frame.local_4+=2){
            if(!is_known(enemy_frame.local_4,enemy_frame.local_8,enemy_frame.local_c))continue;
            const Map &tile=*map_loc(enemy_frame.local_4,enemy_frame.local_8);
            if((tile.climate&0xe0)>=0x60)continue;
            if(enemy_frame.local_64){if(tile.region!=enemy_frame.local_34)continue;}
            else if(!base_on_sea(enemy_frame.local_5c,tile.region))continue;
            enemy_frame.local_20=0;
            if((player.flags&0x20000000u)&&!(tile.bit&0x100000u)){
                enemy_frame.local_20=(tile.bit&0x20u)&&(tile.climate&0xe0)>=0x40?2:1;
                if(tile.bit&0x2000u)++enemy_frame.local_20;
            }
            for(int n=0;n<8;++n){
                const int x=wrap_map_x(enemy_frame.local_4+VehMoveDeltaX[n]);
                const int y=enemy_frame.local_8+VehMoveDeltaY[n];
                if(!on_map(x,y))continue;
                if(!(player.flags&0x20000000u)||(map_loc(x,y)->bit&0x100000u)){
                    if(is_known(x,y,enemy_frame.local_c))continue;
                }
                enemy_frame.local_20+=y>=2&&y<MapLatitudeBounds-3?6:2;
            }
            if(!enemy_frame.local_20)continue;
            int reference_x=enemy_frame.local_18,reference_y=enemy_frame.local_14;
            if(enemy_frame.local_5c>=0){reference_x=Bases[enemy_frame.local_5c].x;
                reference_y=Bases[enemy_frame.local_5c].y;}
            const int distance=vector_dist(x_dist(reference_x,enemy_frame.local_4),abs(reference_y-enemy_frame.local_8));
            if(!distance)continue; // Native576Fxx explicitly tests zero.
            enemy_frame.local_38=(enemy_frame.local_20<<8)/distance;
            if(enemy_frame.local_28<enemy_frame.local_38){enemy_frame.local_28=enemy_frame.local_38;go_to(enemy_frame.vehicle_id,'X',enemy_frame.local_4,enemy_frame.local_8);}
        }
    }
    return SEA_JOIN_577071;
}

#ifndef TACTICAL_TAIL_HOST
#include "stdafx.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "general.h"
#include "log.h"
#include "map.h"
#include "netdaemon.h"
#include "path.h"
#include "veh.h"
#endif
#include <stdlib.h>


static bool tactical_low(int x, int y) {
    return (map_loc(x,y)->climate & 0xe0u) < 0x60u;
}
static int tactical_offense(int unit) {
    return Weapon[VehPrototypes[Vehs[unit].proto_id].weapon_id].offense_rating;
}




// Complete577361..577443 bridge. Include aftertactical_tail.cpp.
// Returns1 for direct578F46,0 for tactical577443. Does not execute either.
static void enemy_move_prepare_empty_region_goal(EnemyMoveFullFrame &enemy_frame,bool skip_triad_gate=false){
    const PlayerData &player=PlayersData[enemy_frame.local_c];
    if((!skip_triad_gate&&enemy_frame.local_40)||enemy_frame.local_50==8||player.region_total_bases[enemy_frame.local_34]||
        (player.unk_77[enemy_frame.local_34]&6u)||player.region_base_plan[enemy_frame.local_34]<3||
        ((enemy_frame.local_14<1||enemy_frame.local_14>=MapLatitudeBounds-1)&&enemy_frame.local_50!=12))return;
    for(int n=0;n<8;++n){
        const int x=wrap_map_x(enemy_frame.local_18+VehMoveDeltaX[n]);const int y=enemy_frame.local_14+VehMoveDeltaY[n];
        if(on_map(x,y)&&(map_loc(x,y)->climate&0xe0)<0x60){
            add_goal(enemy_frame.local_c,7,1,x,y,-1);enemy_frame.local_c0=1;return;
        }
    }
}

// Pinned57302D..5741CF continuation after57300C's base-loop backedge finishes.
// Genuine source fragment: no separate function mapping or acceptance claim.
#ifndef STRATEGIC_ACTIONS_HOST
#include "stdafx.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "map.h"
#include "path.h"
#include "technology.h"
#include "terraforming.h"
#include "veh.h"
#endif
#include <stddef.h>
#ifndef STRATEGIC_ACTIONS_HOST
typedef char StrategicActionBaseAndFactionFieldsMustMatchNative[
    (offsetof(Base,resource_sqr_radius)==0x78 &&
     offsetof(Base,eco_damage)==0x48 &&
     offsetof(PlayerData,current_num_bases)==0x105c &&
     offsetof(PlayerData,energy_reserves)==0x220) ? 1 : -1];
#endif
#include <stdlib.h>
enum StrategicActionExit {
    STRATEGIC_RETURN_ZERO,STRATEGIC_JOIN_57707F,
    STRATEGIC_JOIN_578F46,STRATEGIC_JOIN_5741CF
};

static bool strategic_former_assigned(EnemyMoveFullFrame &enemy_frame,int x,int y){
    for(enemy_frame.local_1c=0;enemy_frame.local_1c<GameDataStateStorage.fields_.vehicle_current_count_;++enemy_frame.local_1c){
        const Veh &v=Vehs[enemy_frame.local_1c];
        if(v.faction_id!=enemy_frame.local_c||VehPrototypes[v.proto_id].plan!=9||
           enemy_frame.local_1c==enemy_frame.vehicle_id)continue;
        if((v.x==x&&v.y==y)||(v.order==0x18&&
            v.waypoint_x[0]==x&&v.waypoint_y[0]==y))return true;
    }
    return false;
}
static StrategicActionExit enemy_move_strategic_actions(EnemyMoveFullFrame &enemy_frame){
    Veh &v=Vehs[enemy_frame.vehicle_id];PlayerData &player=PlayersData[enemy_frame.local_c];
    enemy_frame.local_4c=0; //573032, not an inferred default.
    if(!player.current_num_bases){
        for(enemy_frame.local_1c=0;enemy_frame.local_1c<GameDataStateStorage.fields_.vehicle_current_count_;++enemy_frame.local_1c){
            const Veh &colony=Vehs[enemy_frame.local_1c];
            if(colony.faction_id==enemy_frame.local_c&&VehPrototypes[colony.proto_id].plan==8){
                go_to(enemy_frame.vehicle_id,'#',colony.x,colony.y);
                return STRATEGIC_JOIN_57707F;
            }
        }
    }
    const int at_former_goal=at_goal(enemy_frame.local_c,9,enemy_frame.local_18,enemy_frame.local_14);
    enemy_frame.local_44=can_terraform(enemy_frame.local_c,enemy_frame.local_18,enemy_frame.local_14,
        at_former_goal||enemy_frame.local_30,enemy_frame.local_5c);
    if(!enemy_frame.local_84&&(enemy_frame.local_40==2||enemy_frame.local_74==(enemy_frame.local_40==1))&&enemy_frame.local_64){
        del_site(enemy_frame.local_c,enemy_frame.local_74?25:9,enemy_frame.local_18,enemy_frame.local_14,0);
        if(at_goal(enemy_frame.local_c,0x79,enemy_frame.local_18,enemy_frame.local_14)&&!is_sensor(enemy_frame.local_18,enemy_frame.local_14)){
            v.order=13;return STRATEGIC_JOIN_578F46;
        }
        if(at_goal(enemy_frame.local_c,0x29,enemy_frame.local_18,enemy_frame.local_14)&&!enemy_frame.local_30&&enemy_frame.local_a4<=5){
            // Native keeps the range upper bound on the stack across the
            // THREE-argument terraform_cost call; Kuna infers five args.
            const int cost=range(terraform_cost(enemy_frame.local_18,enemy_frame.local_14,enemy_frame.local_c),0,player.energy_reserves/4);
            player.energy_reserves=range(player.energy_reserves-cost,0,99999);
            v.order=20;del_site(enemy_frame.local_c,8,enemy_frame.local_18,enemy_frame.local_14,1);
            return STRATEGIC_JOIN_578F46;
        }
        if(at_goal(enemy_frame.local_c,0x49,enemy_frame.local_18,enemy_frame.local_14)&&terrain_avail(12,enemy_frame.local_74,enemy_frame.local_c)){
            v.order=16;return STRATEGIC_JOIN_578F46;}
        if(at_goal(enemy_frame.local_c,13,enemy_frame.local_18,enemy_frame.local_14)&&terrain_avail(13,enemy_frame.local_74,enemy_frame.local_c)){
            v.order=17;return STRATEGIC_JOIN_578F46;}
        if(at_goal(enemy_frame.local_c,0x69,enemy_frame.local_18,enemy_frame.local_14)&&terrain_avail(14,enemy_frame.local_74,enemy_frame.local_c)){
            v.order=18;return STRATEGIC_JOIN_578F46;}
        if(!enemy_frame.local_74&&!enemy_frame.local_30&&
            !(static_cast<unsigned char>(enemy_frame.local_68)&GameDataStateStorage.fields_.lock_masks_.enable_mask_)&&terrain_avail(16,0,enemy_frame.local_c)&&
            (Continents[enemy_frame.local_34].tile_count<=49||
             (Continents[enemy_frame.local_34].tile_count<=149&&BaseFindDist>=4))&&
            enemy_frame.local_7c>=2&&(map_loc(enemy_frame.local_18,enemy_frame.local_14)->climate&0xe0)<=0xbf){
            bool high=false;
            for(int pos=0;pos<49;++pos){
                enemy_frame.local_4=wrap_map_x(enemy_frame.local_18+RadiusOffsetX[pos]);enemy_frame.local_8=enemy_frame.local_14+RadiusOffsetY[pos];
                if(on_map(enemy_frame.local_4,enemy_frame.local_8)&&(map_loc(enemy_frame.local_4,enemy_frame.local_8)->climate&0xe0)>=0xc0){high=true;break;}}
            if(!high){v.order=20;return STRATEGIC_JOIN_578F46;}
        }
        if(enemy_frame.local_64>=3)enemy_frame.local_54=0;
        else {
            const Map &tile=*map_loc(enemy_frame.local_18,enemy_frame.local_14);const unsigned bits=tile.bit;
            if((bits&0x10u||bits&0x200000u)&&!(bits&4u)&&
                (!(bits&0x20u)||(tile.climate&0xe0)<=0x3f)&&!enemy_frame.local_74&&
                (!(static_cast<unsigned char>(enemy_frame.local_68)&GameDataStateStorage.fields_.lock_masks_.enable_mask_)||!(GameDataStateStorage.fields_.game_more_preferences_&0x400000u))){
                v.order=9;return STRATEGIC_JOIN_578F46;}
            enemy_frame.local_54=radius_move(Bases[enemy_frame.local_5c].x,Bases[enemy_frame.local_5c].y,enemy_frame.local_18,enemy_frame.local_14,21);
        }
        const int former_goal=at_goal(enemy_frame.local_c,enemy_frame.local_74?25:9,enemy_frame.local_18,enemy_frame.local_14);
        enemy_frame.local_2c=former_goal||(v.waypoint_x[0]==enemy_frame.local_18&&v.waypoint_y[0]==enemy_frame.local_14)||
            stack_check(enemy_frame.vehicle_id,9,3,-1,-1)||
            (enemy_frame.local_54&&(Bases[enemy_frame.local_5c].resource_sqr_radius&
                (1u<<(enemy_frame.local_54&31))));
        const Map &tile=*map_loc(enemy_frame.local_18,enemy_frame.local_14);
        if((tile.bit&0x20u)&&(tile.climate&0xe0)>=0x40&&enemy_frame.local_44&&
            (has_abil(v.proto_id,0x2000)||has_tech((map_loc(enemy_frame.local_18,enemy_frame.local_14)->climate&0xe0)<0x60?
                Terraforming[10].prerequisite_tech[1]:Terraforming[10].prerequisite_tech[0],enemy_frame.local_c))&&
            (enemy_frame.local_2c||(enemy_frame.local_64<=2&&(Bases[enemy_frame.local_5c].eco_damage||
                Bases[enemy_frame.local_5c].population_size>=6)))){
            v.order=14;return STRATEGIC_JOIN_578F46;
        }
        // Native573620 tests resource word NONZERO and then tests the
        // shifted mask itself, not resource_word & shifted_mask.
        if(enemy_frame.local_44==4&&(enemy_frame.local_2c||(enemy_frame.local_54&&
            Bases[enemy_frame.local_5c].resource_sqr_radius&&
            (1u<<(enemy_frame.local_54&31))))){
            v.order=4;v.move_to_ai_type='+';return STRATEGIC_JOIN_578F46;
        }
        if(!(enemy_frame.local_74||((static_cast<unsigned char>(enemy_frame.local_68)&GameDataStateStorage.fields_.lock_masks_.enable_mask_)&&
                            (GameDataStateStorage.fields_.game_more_preferences_&0x400000u)))){
            bool roads=enemy_frame.local_2c!=0;
            if(!roads&&enemy_frame.local_64<=2){
                const unsigned bits=map_loc(enemy_frame.local_18,enemy_frame.local_14)->bit;
                if(bits&0x10u||bits&0x8000u)roads=true;
                else if(bits&0x200000u||enemy_frame.local_44){
                    roads=!(bits&0x20u)||(map_loc(enemy_frame.local_18,enemy_frame.local_14)->climate&0xe0)<=0x3f;
                }
            }
            if(roads){
                const unsigned bits=map_loc(enemy_frame.local_18,enemy_frame.local_14)->bit;
                if(bits&4u){
                    if(has_tech(Terraforming[6].prerequisite_tech[0],enemy_frame.local_c)&&enemy_frame.local_2c&&!(bits&8u)){
                        v.order=9;v.move_to_ai_type='R';return STRATEGIC_JOIN_578F46;}
                } else if(!(bits&0x20u)||(map_loc(enemy_frame.local_18,enemy_frame.local_14)->climate&0xe0)<=0x3f||
                    has_tech(Rules.tech_improve_fungus_sqr,enemy_frame.local_c)){
                    v.order=9;v.move_to_ai_type='r';return STRATEGIC_JOIN_578F46;
                }
            }
        }
        if(enemy_frame.local_44&&(enemy_frame.local_64<=2||stack_check(enemy_frame.vehicle_id,9,3,-1,-1))&&
            (enemy_frame.local_2c||!v.order)){
            v.order=(signed char)enemy_frame.local_44;v.move_to_ai_type='+';
            return STRATEGIC_JOIN_578F46;
        }
        if(!(static_cast<unsigned char>(enemy_frame.local_68)&GameDataStateStorage.fields_.lock_masks_.enable_mask_)||(GameDataStateStorage.fields_.game_more_preferences_&0x20u)){
            if(enemy_frame.local_2c||!v.order){
                if(is_sensor(enemy_frame.local_18,enemy_frame.local_14)&&whose_territory(enemy_frame.local_c,enemy_frame.local_18,enemy_frame.local_14,0,0)==enemy_frame.local_c&&
                    enemy_frame.local_64>=2&&!(map_loc(enemy_frame.local_18,enemy_frame.local_14)->bit&0x800u)&&
                    has_tech(Terraforming[7].prerequisite_tech[0],enemy_frame.local_c)){
                    v.order=11;return STRATEGIC_JOIN_578F46;}
                if(good_sensor(enemy_frame.local_c,enemy_frame.local_18,enemy_frame.local_14)){v.order=13;return STRATEGIC_JOIN_578F46;}
                if(enemy_frame.local_64>=2&&enemy_frame.local_64<=3&&
                    (map_loc(enemy_frame.local_18,enemy_frame.local_14)->bit&4u)&&!(map_loc(enemy_frame.local_18,enemy_frame.local_14)->bit&0x800u)&&
                    has_tech(Terraforming[7].prerequisite_tech[0],enemy_frame.local_c)){
                    bool bunker=false;
                    for(int pos=0;pos<9;++pos){
                        enemy_frame.local_4=wrap_map_x(enemy_frame.local_18+RadiusOffsetX[pos]);enemy_frame.local_8=enemy_frame.local_14+RadiusOffsetY[pos];
                        if(on_map(enemy_frame.local_4,enemy_frame.local_8)&&(map_loc(enemy_frame.local_4,enemy_frame.local_8)->bit&0x800u)){bunker=true;break;}}
                    if(!bunker){v.order=11;return STRATEGIC_JOIN_578F46;}
                }
                if(enemy_frame.local_2c&&has_tech(Terraforming[9].prerequisite_tech[0],enemy_frame.local_c)){
                    enemy_frame.local_4=enemy_frame.local_18;enemy_frame.local_8=enemy_frame.local_14;enemy_frame.local_4c=1;
                    if(!Paths.sensors(enemy_frame.local_c,&enemy_frame.local_4,&enemy_frame.local_8)&&enemy_frame.local_4==enemy_frame.local_18&&enemy_frame.local_8==enemy_frame.local_14){
                        v.order=13;return STRATEGIC_JOIN_578F46;}
                }
            }
        }
    }
    //5739AA: unguarded home/base accesses are original, including base-1.
    if(v.order!=0x18&&!enemy_frame.local_84){
        if(enemy_frame.local_30){enemy_frame.local_24=v.home_base_id;
            enemy_frame.local_a4=Bases[enemy_frame.local_24].x;enemy_frame.local_58=Bases[enemy_frame.local_24].y;
        } else {
            enemy_frame.local_24=enemy_frame.local_5c;
            if(v.state&0x200u){enemy_frame.local_a4=Bases[enemy_frame.local_5c].x;
                enemy_frame.local_58=Bases[enemy_frame.local_5c].y;}
            else {enemy_frame.local_a4=enemy_frame.local_18;enemy_frame.local_58=enemy_frame.local_14;}
            int current_formers=0;
            for(int pos=1;pos<=20;++pos){
                enemy_frame.local_4=wrap_map_x(Bases[enemy_frame.local_5c].x+RadiusOffsetX[pos]);
                enemy_frame.local_8=Bases[enemy_frame.local_5c].y+RadiusOffsetY[pos];
                if(!on_map(enemy_frame.local_4,enemy_frame.local_8)||map_loc(enemy_frame.local_4,enemy_frame.local_8)->region!=enemy_frame.local_34)continue;
                enemy_frame.local_1c=veh_at(enemy_frame.local_4,enemy_frame.local_8);
                if(enemy_frame.local_1c>=0&&Vehs[enemy_frame.local_1c].faction_id==enemy_frame.local_c){
                    current_formers+=stack_check(enemy_frame.local_1c,2,9,-1,-1);
                    if(enemy_frame.local_4==enemy_frame.local_18&&enemy_frame.local_8==enemy_frame.local_14)--current_formers;}
            }
            if(current_formers){
                enemy_frame.local_28=9999;enemy_frame.local_54=0;
                for(enemy_frame.local_54=0;enemy_frame.local_54<GameDataStateStorage.fields_.base_current_count_;++enemy_frame.local_54){
                    const Base &b=Bases[enemy_frame.local_54];
                    if(b.faction_id_current!=enemy_frame.local_c||map_loc(b.x,b.y)->region!=enemy_frame.local_34)continue;
                    int formers=0;
                    for(int pos=0;pos<25;++pos){
                        const int x=wrap_map_x(b.x+RadiusOffsetX[pos]);const int y=b.y+RadiusOffsetY[pos];
                        if(!on_map(x,y)||map_loc(x,y)->region!=enemy_frame.local_34)continue;
                        enemy_frame.local_1c=veh_at(x,y);
                        if(enemy_frame.local_1c>=0)formers+=stack_check(enemy_frame.local_1c,2,9,-1,-1);
                    }
                    if(formers<current_formers){
                        enemy_frame.local_38=vector_dist(x_dist(enemy_frame.local_18,b.x),abs(enemy_frame.local_14-b.y))*(formers+1)*(formers+1);
                        if(enemy_frame.local_38<enemy_frame.local_28){enemy_frame.local_24=enemy_frame.local_54;enemy_frame.local_28=enemy_frame.local_38;}
                    }
                }
            }
        }
        if(enemy_frame.local_24>=0){
            enemy_frame.local_28=9999;
            for(enemy_frame.local_2c=1;enemy_frame.local_2c<21;++enemy_frame.local_2c){
                const int pos=(GameDataStateStorage.fields_.turn_current_num_+(int)enemy_frame.local_2c)%21;
                enemy_frame.local_4=wrap_map_x(enemy_frame.local_a4+RadiusOffsetX[pos]);
                enemy_frame.local_8=enemy_frame.local_58+RadiusOffsetY[pos];
                if(!on_map(enemy_frame.local_4,enemy_frame.local_8)||base_who(enemy_frame.local_4,enemy_frame.local_8)>=0)continue;
                const Map &tile=*map_loc(enemy_frame.local_4,enemy_frame.local_8);
                const int low=(tile.climate&0xe0)<0x60;
                if(low!=(enemy_frame.local_40==1)||(tile.region!=enemy_frame.local_34&&low==enemy_frame.local_74)||
                    zoc_move(enemy_frame.local_4,enemy_frame.local_8,enemy_frame.local_c))continue;
                enemy_frame.local_1c=veh_at(enemy_frame.local_4,enemy_frame.local_8);
                if(enemy_frame.local_1c>=0&&(Vehs[enemy_frame.local_1c].faction_id!=enemy_frame.local_c||
                    stack_check(enemy_frame.local_1c,2,9,-1,-1)))continue;
                const int flag=(static_cast<unsigned char>(enemy_frame.local_68)&GameDataStateStorage.fields_.lock_masks_.enable_mask_)&&enemy_frame.local_30;
                if(!can_terraform(enemy_frame.local_c,enemy_frame.local_4,enemy_frame.local_8,flag,enemy_frame.local_24))continue;
                enemy_frame.local_38=vector_dist(x_dist(Bases[enemy_frame.local_24].x,enemy_frame.local_4),abs(Bases[enemy_frame.local_24].y-enemy_frame.local_8));
                if(enemy_frame.local_38>2)continue;
                if(!(Bases[enemy_frame.local_24].resource_sqr_radius&(1u<<(enemy_frame.local_2c&31)))&&
                    (!(tile.bit&0x20u)||(tile.climate&0xe0)<=0x3f))enemy_frame.local_38*=3;
                if(enemy_frame.local_38>=enemy_frame.local_28||strategic_former_assigned(enemy_frame,enemy_frame.local_4,enemy_frame.local_8))continue;
                enemy_frame.local_28=enemy_frame.local_38;go_to(enemy_frame.vehicle_id,'w',enemy_frame.local_4,enemy_frame.local_8);
            }
            if(v.order==0x18)return STRATEGIC_JOIN_57707F;
        }
        if(!enemy_frame.local_4c&&(!(static_cast<unsigned char>(enemy_frame.local_68)&GameDataStateStorage.fields_.lock_masks_.enable_mask_)||
                             (GameDataStateStorage.fields_.game_more_preferences_&0x20u))){
            enemy_frame.local_4=enemy_frame.local_18;enemy_frame.local_8=enemy_frame.local_14;
            if(!Paths.sensors(enemy_frame.local_c,&enemy_frame.local_4,&enemy_frame.local_8)){
                if(enemy_frame.local_4==enemy_frame.local_18&&enemy_frame.local_8==enemy_frame.local_14){v.order=13;return STRATEGIC_JOIN_578F46;}
                const int distance=vector_dist(x_dist(enemy_frame.local_18,enemy_frame.local_4),abs(enemy_frame.local_14-enemy_frame.local_8));
                if(!(enemy_frame.local_30&&distance>2)&&!strategic_former_assigned(enemy_frame,enemy_frame.local_4,enemy_frame.local_8)){
                    go_to(enemy_frame.vehicle_id,'s',enemy_frame.local_4,enemy_frame.local_8);return STRATEGIC_JOIN_57707F;}
            }
        }
        if(enemy_frame.local_64&&!enemy_frame.local_74&&player.region_total_bases[enemy_frame.local_34]){
            enemy_frame.local_1c=v.home_base_id>=0?v.home_base_id:enemy_frame.local_5c;
            if(enemy_frame.local_1c>=0){
                const Base &home=Bases[enemy_frame.local_1c];
                if(map_loc(home.x,home.y)->region!=enemy_frame.local_34)enemy_frame.local_1c=enemy_frame.local_5c;
                if(enemy_frame.local_1c>=0){const Base &b=Bases[enemy_frame.local_1c];
                    const int distance=vector_dist(x_dist(enemy_frame.local_18,b.x),abs(enemy_frame.local_14-b.y));
                    if(distance>(GameDataStateStorage.fields_.turn_current_num_<=50)+3||enemy_frame.local_bc){
                        go_to(enemy_frame.vehicle_id,'b',b.x,b.y);return STRATEGIC_JOIN_57707F;}
                }
            }
        }
        if(enemy_frame.local_30&&!v.order){v.state&=~0x200u;v.order=0;return STRATEGIC_RETURN_ZERO;}
    }
    return STRATEGIC_JOIN_5741CF;
}

// Complete pinned enemy_move 5741CF..574A88 proposal. No byte claims.
#ifndef ENEMY_NONSEA_HOST
#include "stdafx.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "general.h"
#include "map.h"
#include "veh.h"
#endif
#include <stddef.h>
#include <stdlib.h>
#ifndef ENEMY_NONSEA_HOST
typedef char NonseaRegionVisibilityOffset[
    offsetof(PlayerData,unk_79)==0x157c ? 1 : -1];
typedef char NonseaPendingPlanetOffset[
    offsetof(PlayerData,soc_effect_pending)+(offsetof(SocialEffect,values_)+sizeof(int)*SocialEffect::PLANET)==0x268 ? 1 : -1];
#endif
enum NonseaExit {
    NONSEA_JOIN_578B26, NONSEA_JOIN_57707F,
    NONSEA_JOIN_577071, NONSEA_JOIN_574A88
};
enum NonseaEntry { NONSEA_ENTRY_5741CF, NONSEA_ENTRY_57460A, NONSEA_ENTRY_574A7E };
// Inputs and shared-frame outputs. No constructor/defaults: untouched slots
// retain their predecessor values, including unreached/uninitialized scratch.


static NonseaExit enemy_move_nonsea(EnemyMoveFullFrame &enemy_frame,
    NonseaEntry entry = NONSEA_ENTRY_5741CF) {
    if (entry == NONSEA_ENTRY_5741CF && enemy_frame.local_88 >= 3 &&
        Continents[enemy_frame.local_34].tile_count <= (int)PlayersData[enemy_frame.local_c].unk_79[enemy_frame.local_34] &&
        enemy_frame.local_50 <= 3 && !enemy_frame.local_40 && Vehs[enemy_frame.vehicle_id].order != 0x18 &&
        !enemy_frame.local_74) {
        if (coast_or_border(enemy_frame.local_18,enemy_frame.local_14,enemy_frame.local_18,enemy_frame.local_14,enemy_frame.local_c)) {
            int direction = (Vehs[enemy_frame.vehicle_id].facing + 4) % 8;
            enemy_frame.local_4 = wrap_map_x(enemy_frame.local_18 + VehMoveDeltaX[direction]);
            enemy_frame.local_8 = enemy_frame.local_14 + VehMoveDeltaY[direction];
            if (!coast_or_border(enemy_frame.local_4,enemy_frame.local_8,enemy_frame.local_4,enemy_frame.local_8,enemy_frame.local_c)) {
                if (enemy_frame.vehicle_id & 1) Vehs[enemy_frame.vehicle_id].state |= 0x80000000u;
                else Vehs[enemy_frame.vehicle_id].state &= 0x7fffffffu;
            }
            enemy_frame.local_30 = 0;
            enemy_frame.local_6c = 0;
            enemy_frame.local_4c = 3;
            do {
                int rotation = enemy_frame.local_4c;
                if (Vehs[enemy_frame.vehicle_id].state & 0x80000000u)
                    rotation = enemy_frame.local_6c - 3;
                direction = (rotation + Vehs[enemy_frame.vehicle_id].facing + 8) % 8;
                enemy_frame.local_4 = wrap_map_x(enemy_frame.local_18 + VehMoveDeltaX[direction]);
                enemy_frame.local_8 = enemy_frame.local_14 + VehMoveDeltaY[direction];
                if (enemy_frame.local_30) {
                    if (on_map(enemy_frame.local_4,enemy_frame.local_8) &&
                        map_loc(enemy_frame.local_4,enemy_frame.local_8)->region == enemy_frame.local_34 &&
                        whose_territory(enemy_frame.local_c,enemy_frame.local_4,enemy_frame.local_8,NULL,0) == enemy_frame.local_c) {
                        enemy_frame.local_80 = direction;
                        Vehs[enemy_frame.vehicle_id].move_to_ai_type = 'p';
                        return NONSEA_JOIN_578B26;
                    }
                } else if (on_map(enemy_frame.local_4,enemy_frame.local_8)) {
                    if (map_loc(enemy_frame.local_4,enemy_frame.local_8)->region != enemy_frame.local_34 ||
                        whose_territory(enemy_frame.local_c,enemy_frame.local_4,enemy_frame.local_8,NULL,0) != enemy_frame.local_c)
                        enemy_frame.local_30 = 1;
                }
                ++enemy_frame.local_6c;
                --enemy_frame.local_4c;
            } while (enemy_frame.local_4c > -5);
        }
        enemy_frame.local_28 = 0;
        enemy_frame.local_8 = 0;
        while (enemy_frame.local_8 < (int)MapLatitudeBounds) {
            enemy_frame.local_4 = enemy_frame.local_8 & 1;
            while (enemy_frame.local_4 < (int)MapLongitudeBounds) {
                if (map_loc(enemy_frame.local_4,enemy_frame.local_8)->region == enemy_frame.local_34 &&
                    whose_territory(enemy_frame.local_c,enemy_frame.local_4,enemy_frame.local_8,NULL,0) == enemy_frame.local_c) {
                    enemy_frame.local_20 = 1;
                    enemy_frame.local_58 = 0;
                    do {
                        const int offset = enemy_frame.local_58 / 4;
                        const int xx = wrap_map_x(enemy_frame.local_4 + RadiusOffsetX[offset]);
                        const int yy = enemy_frame.local_8 + RadiusOffsetY[offset];
                        if (on_map(xx,yy)) {
                            if (map_loc(xx,yy)->region != enemy_frame.local_34) ++enemy_frame.local_20;
                            else {
                                if (whose_territory(enemy_frame.local_c,xx,yy,NULL,0) != enemy_frame.local_c)
                                    enemy_frame.local_20 += 3;
                                if (anything_at(xx,yy) == enemy_frame.local_c) enemy_frame.local_20 -= 6;
                            }
                        }
                        enemy_frame.local_58 += 4;
                    } while (enemy_frame.local_58 < 0x24);
                    int candidate_score = (int)((unsigned int)((abs(enemy_frame.local_14-enemy_frame.local_8) +
                        x_dist(enemy_frame.local_18,enemy_frame.local_4)) >> 1) * (unsigned int)enemy_frame.local_20);
                    enemy_frame.local_1c = 0;
                    if (GameDataStateStorage.fields_.vehicle_current_count_ > 0) {
                        enemy_frame.local_4c = GameDataStateStorage.fields_.vehicle_current_count_;
                        enemy_frame.local_1c = GameDataStateStorage.fields_.vehicle_current_count_;
                        int vehicle = 0;
                        do {
                            if (Vehs[vehicle].faction_id == enemy_frame.local_c &&
                                Vehs[vehicle].waypoint_x[0] == enemy_frame.local_4 &&
                                Vehs[vehicle].waypoint_y[0] == enemy_frame.local_8)
                                candidate_score /= 2;
                            ++vehicle;
                            --enemy_frame.local_4c;
                        } while (enemy_frame.local_4c);
                    }
                    if (candidate_score > enemy_frame.local_28) {
                        enemy_frame.local_28 = candidate_score;
                        go_to(enemy_frame.vehicle_id,'P',enemy_frame.local_4,enemy_frame.local_8);
                    }
                }
                enemy_frame.local_4 += 2;
            }
            ++enemy_frame.local_8;
        }
    }
    if (entry == NONSEA_ENTRY_57460A || (entry == NONSEA_ENTRY_5741CF &&
        (enemy_frame.local_b0 || (enemy_frame.local_50 == 3 && GameDataStateStorage.fields_.turn_current_num_ < 100)))) {
        if (enemy_frame.local_40 != 2 &&
            (Vehs[enemy_frame.vehicle_id].order != 0x18 || Vehs[enemy_frame.vehicle_id].move_to_ai_type != '/')) {
            enemy_frame.local_4c = 0;
            enemy_frame.local_28 = 0;
            enemy_frame.local_60 = 0;
            enemy_frame.local_54 = 0;
            do {
                enemy_frame.local_44 = 0;
                enemy_frame.local_2c = enemy_frame.local_54 ? RadiusRange[enemy_frame.local_54-1] : 0;
                while ((int)enemy_frame.local_2c < RadiusRange[enemy_frame.local_54]) {
                    enemy_frame.local_4 = wrap_map_x(enemy_frame.local_18 + RadiusOffsetX[enemy_frame.local_2c]);
                    enemy_frame.local_8 = enemy_frame.local_14 + RadiusOffsetY[enemy_frame.local_2c];
                    if (on_map(enemy_frame.local_4,enemy_frame.local_8) &&
                        map_loc(enemy_frame.local_4,enemy_frame.local_8)->region == enemy_frame.local_34) {
                        enemy_frame.local_70 = whose_territory(enemy_frame.local_c,enemy_frame.local_4,enemy_frame.local_8,NULL,0);
                        if (enemy_frame.local_70 < 0 || enemy_frame.local_70 == enemy_frame.local_c ||
                            !(PlayersData[enemy_frame.local_c].diplo_treaties[enemy_frame.local_70] & 7)) {
                            int candidate_score = is_known(enemy_frame.local_4,enemy_frame.local_8,enemy_frame.local_c)
                                ? (goody_at(enemy_frame.local_4,enemy_frame.local_8) ? 512 : 0) : 0;
                            enemy_frame.local_30 = 0;
                            do {
                                // Original defect: indexes radius_index, NOT mode.
                                // Thus the same offset tile is examined nine times.
                                const int xx = wrap_map_x(enemy_frame.local_4 + RadiusOffsetX[enemy_frame.local_2c]);
                                const int yy = enemy_frame.local_8 + RadiusOffsetY[enemy_frame.local_2c];
                                if (on_map(xx,yy)) {
                                    if (is_known(xx,yy,enemy_frame.local_c)) {
                                        enemy_frame.local_70 = veh_who(xx,yy);
                                        if (enemy_frame.local_70 >= 0 && enemy_frame.local_70 != enemy_frame.local_c) {
                                            candidate_score = 0;
                                            break;
                                        }
                                    } else candidate_score +=
                                        map_loc(xx,yy)->region == enemy_frame.local_34 ? 16 : 1;
                                }
                                ++enemy_frame.local_30;
                            } while (enemy_frame.local_30 < 9);
                            if ((map_loc(enemy_frame.local_4,enemy_frame.local_8)->bit & 0x20) &&
                                (map_loc(enemy_frame.local_4,enemy_frame.local_8)->climate & 0xe0) >= 0x40 &&
                                PlayersData[enemy_frame.local_c].soc_effect_pending.values_[SocialEffect::PLANET] <= 0)
                                candidate_score /= 2;
                            enemy_frame.local_a4 = candidate_score;
                            if (enemy_frame.local_5c >= 0) {
                                const Base &base = Bases[enemy_frame.local_5c];
                                enemy_frame.local_58 = abs(enemy_frame.local_8-base.y);
                                int base_bonus = 16 - ((enemy_frame.local_58 +
                                    x_dist(enemy_frame.local_4,base.x)) >> 1);
                                if (base_bonus >= 0) {
                                    // Native repeats both distance calls on this arm.
                                    enemy_frame.local_58 = abs(enemy_frame.local_8-base.y);
                                    base_bonus = 16 - ((enemy_frame.local_58 +
                                        x_dist(enemy_frame.local_4,base.x)) >> 1);
                                } else base_bonus = 0;
                                candidate_score += base_bonus;
                            }
                            enemy_frame.local_58 = abs(enemy_frame.local_14-enemy_frame.local_8);
                            enemy_frame.local_20 = (int)((unsigned int)candidate_score << 8) /
                                (((enemy_frame.local_58 + x_dist(enemy_frame.local_18,enemy_frame.local_4)) >> 1) + 2);
                            if (enemy_frame.local_44 < enemy_frame.local_20) enemy_frame.local_44 = enemy_frame.local_20;
                            if (enemy_frame.local_28 < enemy_frame.local_20) {
                                enemy_frame.local_1c = 0;
                                bool claimed = false;
                                while (enemy_frame.local_1c < GameDataStateStorage.fields_.vehicle_current_count_) {
                                    const Veh &vehicle = Vehs[enemy_frame.local_1c];
                                    if (vehicle.faction_id == enemy_frame.local_c &&
                                        enemy_frame.local_1c != enemy_frame.vehicle_id && vehicle.order == 0x18) {
                                        enemy_frame.local_38 = (abs(enemy_frame.local_8-vehicle.waypoint_y[0]) +
                                            x_dist(enemy_frame.local_4,vehicle.waypoint_x[0])) >> 1;
                                        if (enemy_frame.local_38 <= 1) {
                                            const int acting_distance = (abs(enemy_frame.local_14-enemy_frame.local_8) +
                                                x_dist(enemy_frame.local_18,enemy_frame.local_4)) >> 1;
                                            enemy_frame.local_58 = abs(vehicle.y-enemy_frame.local_8);
                                            if (((enemy_frame.local_58 + x_dist(vehicle.x,enemy_frame.local_4)) >> 1) - 1
                                                <= acting_distance) {
                                                claimed = true;
                                                break;
                                            }
                                        }
                                    }
                                    ++enemy_frame.local_1c;
                                }
                                if (!claimed && enemy_frame.local_20) {
                                    enemy_frame.local_4c = enemy_frame.local_a4;
                                    enemy_frame.local_28 = enemy_frame.local_20;
                                    go_to(enemy_frame.vehicle_id,'/',enemy_frame.local_4,enemy_frame.local_8);
                                }
                            }
                        }
                    }
                    ++enemy_frame.local_2c;
                }
                if (Vehs[enemy_frame.vehicle_id].order == 0x18 &&
                    enemy_frame.local_44 < enemy_frame.local_60 && enemy_frame.local_4c >= 16) break;
                enemy_frame.local_60 = enemy_frame.local_44;
                ++enemy_frame.local_54;
            } while (enemy_frame.local_54 < 9);
            if (Vehs[enemy_frame.vehicle_id].order == 0x18) return NONSEA_JOIN_57707F;
        }
    }
    return enemy_frame.local_40 == 1 ? NONSEA_JOIN_574A88 : NONSEA_JOIN_577071;
}


// Complete pinned enemy_move571982..572922. EnemyMoveFullFrame must already
// be declared by the including complete owner; no secondary automatic frame.
#ifndef ENEMY_SELECTED_HOST
#include "stdafx.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "general.h"
#include "log.h"
#include "map.h"
#include "netdaemon.h"
#include "technology.h"
#include "veh.h"
#endif
#include <stddef.h>
#include <stdlib.h>
#ifndef ENEMY_SELECTED_HOST
typedef char SelectedBaseEventOffset[offsetof(Base,event)==0x34 ? 1 : -1];
typedef char SelectedPrototypePlanOffset[offsetof(VehPrototype,plan)==0x2a ? 1 : -1];
typedef char SelectedChassisOffsets[(offsetof(RulesChassis,speed)==0x48 &&
    offsetof(RulesChassis,triad)==0x49) ? 1 : -1];
#endif
enum SelectedExit {
    SELECTED_RETURN_ZERO, SELECTED_RETURN_ONE,
    SELECTED_JOIN_57707F, SELECTED_JOIN_572922, SELECTED_JOIN_578B26
};

static SelectedExit enemy_move_selected(EnemyMoveFullFrame &f) {
    const int actor = f.vehicle_id;
    if (Vehs[actor].order != 0x18 && (f.local_44 || f.local_50 == 2)) {
        f.local_6c = 0;
        Vehs[actor].order = 0;
        do {
            const int offset = f.local_6c / 4;
            f.local_4 = wrap_map_x(f.local_18 + RadiusOffsetX[offset]);
            f.local_8 = f.local_14 + RadiusOffsetY[offset];
            if (on_map(f.local_4,f.local_8) &&
                map_loc(f.local_4,f.local_8)->region == f.local_34 &&
                whose_territory(f.local_c,f.local_4,f.local_8,NULL,0) == f.local_c) {
                const int dy = abs(f.local_14-f.local_8);
                f.local_38 = vector_dist(x_dist(f.local_18,f.local_4),dy);
                f.local_1c = stack_fix(veh_at(f.local_4,f.local_8));
                bool eligible = false;
                if (f.local_1c < 0 || Vehs[f.local_1c].faction_id == f.local_c) {
                    if ((map_loc(f.local_4,f.local_8)->bit & 0x2000) &&
                        !(Vehs[actor].state & 0x100) && want_monolith(actor)) {
                        f.local_38 += 1000;
                        eligible = true;
                    }
                } else if (f.local_44) {
                    const int other = f.local_1c;
                    bool foreign_allowed = true;
                    if (Vehs[other].faction_id) {
                        if (stack_check(other,1,-1,-1,-1) >= 2 ||
                            !(PlayersData[f.local_c].diplo_treaties[Vehs[other].faction_id] & 0x10) ||
                            Weapon[VehPrototypes[Vehs[other].proto_id].weapon_id].offense_rating >= 0)
                            foreign_allowed = false;
                    }
                    if (foreign_allowed &&
                        !Chassis[VehPrototypes[Vehs[other].proto_id].chassis_id].triad)
                        eligible = true;
                }
                if (eligible && f.local_38 < 9999) {
                    f.local_38 = 9999;
                    int marker = 'F';
                    if (Vehs[actor].home_base_id >= 0) {
                        f.local_24 = Vehs[actor].home_base_id;
                        if ((Bases[f.local_24].state & 0x4000000) &&
                            Vehs[actor].move_to_ai_type != 'F') marker = 'y';
                    }
                    go_to(actor,(char)marker,f.local_4,f.local_8);
                }
            }
            f.local_6c += 4;
        } while (f.local_6c < 0x484);
        if (Vehs[actor].order == 0x18) {
            f.local_8c = 0;
            f.local_b4 = 0;
            return SELECTED_JOIN_57707F;
        }
        if (f.local_b4) return SELECTED_JOIN_57707F;
        if (f.local_44) {
            f.local_24 = 0;
            while (f.local_24 < GameDataStateStorage.fields_.base_current_count_) {
                const Base &base = Bases[f.local_24];
                if (base.faction_id_current == f.local_c &&
                    map_loc(base.x,base.y)->region == f.local_34) {
                    const int dy = abs(f.local_14-base.y);
                    int score = vector_dist(x_dist(f.local_18,base.x),dy);
                    f.local_38 = score;
                    if (f.local_24 != Vehs[actor].home_base_id) {
                        score += 1000;
                        f.local_38 = score;
                    }
                    // Native571CFC calls veh_at directly, carrying the four
                    // following stack_check arguments; no stack_fix/veh_top.
                    if (stack_check(veh_at(base.x,base.y),0x11,0x13,f.local_c,-1)) {
                        score = (int)((unsigned int)score * 2u);
                        f.local_38 = score;
                    }
                    if (score < 9999) {
                        f.local_38 = 9999;
                        go_to(actor,'f',base.x,base.y);
                    }
                }
                ++f.local_24;
            }
        }
    }
    //571D57. The second has_fac deliberately uses the reaching faction base,
    // not the scanned candidate; do not normalize this original defect.
    if (f.local_50 != f.local_88 && f.local_88 >= 3 && !f.local_64 &&
        has_fac(0x21,f.local_5c,0) && !(Bases[f.local_5c].state & 0x20000000)) {
        f.local_24 = 0;
        while (f.local_24 < GameDataStateStorage.fields_.base_current_count_) {
            const Base &base = Bases[f.local_24];
            if (base.faction_id_current == f.local_c &&
                has_fac(0x21,f.local_5c,0) && !(base.state & 0x20000000)) {
                const int region = map_loc(base.x,base.y)->region;
                if (region != f.local_34 &&
                    (PlayersData[f.local_c].region_base_plan[region] == f.local_50 ||
                     (f.local_50 <= 2 && PlayersData[f.local_c].region_base_plan[region] <= 2))) {
                    if (!f.local_a0) {
                        log_say("Enemy doing GATE",actor,f.local_24,0);
                        action_gate(actor,f.local_24);
                        return SELECTED_RETURN_ONE;
                    }
                    if (!NetDaemonNet.add_lock(0,base.x,base.y)) {
                        message_veh(0x2410,actor,f.local_24,0);
                        NetDaemonNet.await_exec(1);
                        return SELECTED_RETURN_ONE;
                    }
                }
            }
            ++f.local_24;
        }
    }
    if (!f.local_98 && !f.local_a8 &&
        f.local_18 == PlayersData[f.local_c].x_target &&
        f.local_14 == PlayersData[f.local_c].y_target) {
        const int proto = Vehs[actor].proto_id;
        if ((VehPrototypes[proto].plan == 0 || VehPrototypes[proto].plan == 1) &&
            !Chassis[VehPrototypes[proto].chassis_id].triad && !has_abil(proto,0x10))
            Vehs[actor].state |= 0x40000;
    }
    if ((f.local_50 > 1 && !f.local_a8) || Vehs[actor].order != 0x18)
        return SELECTED_JOIN_572922;
    f.local_4c = abs(f.local_14-Vehs[actor].waypoint_y[0]);
    if (((f.local_4c + x_dist(f.local_18,Vehs[actor].waypoint_x[0])) & ~1) != 4)
        return SELECTED_JOIN_572922;
    f.local_24 = base_at(Vehs[actor].waypoint_x[0],Vehs[actor].waypoint_y[0]);
    if (f.local_24 < 0) return SELECTED_JOIN_572922;
    f.local_6c = f.local_24 * 0x134;
    f.local_30 = f.local_c * 0x833;
    Base &target = Bases[f.local_24];
    if (!(PlayersData[f.local_c].diplo_treaties[target.faction_id_current] & 0x10))
        return SELECTED_JOIN_572922;
    target.event |= (unsigned int)f.local_68;
    f.local_2c = 0;
    f.local_54 = 0;
    f.local_44 = 0;
    // Three distinct census rings: offsets1..8,9..24,25..168.
    for (int pass = 0; pass < 3; ++pass) {
        const int start = pass == 0 ? 1 : pass == 1 ? 9 : 25;
        const int end = pass == 0 ? 9 : pass == 1 ? 25 : 169;
        for (int offset = start; offset < end; ++offset) {
            f.local_4 = wrap_map_x(target.x + RadiusOffsetX[offset]);
            f.local_8 = target.y + RadiusOffsetY[offset];
            if (!on_map(f.local_4,f.local_8)) continue;
            f.local_1c = stack_fix(veh_at(f.local_4,f.local_8));
            if (f.local_1c < 0) continue;
            f.local_1c = veh_top(f.local_1c);
            while (f.local_1c >= 0) {
                const Veh &member = Vehs[f.local_1c];
                if (member.faction_id == f.local_c &&
                    VehPrototypes[member.proto_id].plan <= 1 &&
                    !has_abil(member.proto_id,0x8000)) {
                    if (pass == 0) ++f.local_2c;
                    else if (pass == 1) {
                        if ((member.move_to_ai_type >= 0x33 && member.move_to_ai_type <= 0x35) ||
                            (member.waypoint_x[0] == Vehs[actor].waypoint_x[0] &&
                             member.waypoint_y[0] == Vehs[actor].waypoint_y[0])) {
                            if (member.moves_expended) ++f.local_44;
                            else ++f.local_54;
                        }
                    } else {
                        bool count = true;
                        // Original predicate reads the ACTOR's upper marker.
                        if (member.move_to_ai_type < 0x33 || Vehs[actor].move_to_ai_type > 0x35) {
                            if (base_who(f.local_4,f.local_8) >= 0 && !(member.state & 0x10000))
                                count = false;
                            else if (member.waypoint_x[0] != Vehs[actor].waypoint_x[0] ||
                                     member.waypoint_y[0] != Vehs[actor].waypoint_y[0]) count = false;
                        }
                        if (count) ++f.local_44;
                    }
                }
                f.local_1c = member.next_veh_id_stack;
            }
        }
    }
    if ((int)f.local_2c + f.local_54 < 8 && f.local_44 > (int)f.local_2c) {
        const int first_count = stack_check(actor,2,1,-1,-1);
        f.local_30 = first_count + stack_check(actor,2,0,-1,-1);
        if (zoc_move(f.local_18,f.local_14,f.local_c) || f.local_30 <= 1) {
            f.local_8c = 1;
            Vehs[actor].move_to_ai_type = 0x34;
            return SELECTED_JOIN_57707F;
        }
        f.local_78 = -1;
        f.local_28 = 0;
        f.local_2c = 0;
        do {
            f.local_4 = wrap_map_x(f.local_18 + VehMoveDeltaX[f.local_2c]);
            f.local_8 = f.local_14 + VehMoveDeltaY[f.local_2c];
            if (on_map(f.local_4,f.local_8) &&
                (map_loc(f.local_4,f.local_8)->climate & 0xe0) >= 0x60) {
                const int dy = abs(f.local_8-target.y);
                if (((dy + x_dist(f.local_4,target.x)) & ~1) == 4) {
                    f.local_70 = anything_at(f.local_4,f.local_8);
                    if (f.local_70 < 0 || f.local_70 == f.local_c) {
                        int candidate_score = 10;
                        if (zoc_sea(f.local_4,f.local_8,f.local_c)) {
                            const int cost = hex_cost(f.local_ac,f.local_c,f.local_18,f.local_14,
                                f.local_4,f.local_8,0);
                            if (cost >= veh_moves(actor)-1) goto selected_second_next;
                            candidate_score = 20;
                        }
                        const int vehicle = veh_at(f.local_4,f.local_8);
                        if (vehicle >= 0) {
                            const int first = stack_check(vehicle,2,1,-1,-1);
                            const int count = first + stack_check(vehicle,2,0,-1,-1);
                            if (count >= f.local_30) goto selected_second_next;
                            candidate_score -= count;
                        }
                        if (f.local_28 < candidate_score) {
                            f.local_28 = candidate_score;
                            f.local_78 = (int)f.local_2c;
                        }
                    }
                }
            }
selected_second_next:
            ++f.local_2c;
        } while (f.local_2c < 8);
        if (f.local_78 < 0) {
            f.local_8c = 1;
            Vehs[actor].move_to_ai_type = 0x34;
            return SELECTED_JOIN_57707F;
        }
        f.local_80 = f.local_78;
        Vehs[actor].move_to_ai_type = 0x35;
        return SELECTED_JOIN_578B26;
    }
    //5725E2 first count gate: full defense/range scoring, with last-on-tie.
    if (zoc_move(f.local_18,f.local_14,f.local_c)) return SELECTED_JOIN_572922;
    f.local_28 = -9999;
    f.local_2c = 0;
    do {
        f.local_4 = wrap_map_x(f.local_18 + VehMoveDeltaX[f.local_2c]);
        f.local_8 = f.local_14 + VehMoveDeltaY[f.local_2c];
        if (on_map(f.local_4,f.local_8) &&
            (map_loc(f.local_4,f.local_8)->climate & 0xe0) >= 0x60) {
            const int dy = abs(f.local_8-target.y);
            if (((dy + x_dist(f.local_4,target.x)) & ~1) == 2) {
                f.local_20 = (int)((unsigned int)defense_value(f.local_c,f.local_4,f.local_8,actor,-1) << 3);
                for (int direction = 0; direction < 8; ++direction) {
                    const int xx = wrap_map_x(f.local_4 + VehMoveDeltaX[direction]);
                    const int yy = f.local_8 + VehMoveDeltaY[direction];
                    if (on_map(xx,yy) && (xx != target.x || yy != target.y)) {
                        f.local_1c = stack_fix(veh_at(xx,yy));
                        if (f.local_1c >= 0 && Vehs[f.local_1c].faction_id != f.local_c &&
                            !(PlayersData[f.local_c].diplo_treaties[Vehs[f.local_1c].faction_id] & 7) &&
                            stack_check(f.local_1c,4,-1,-1,-1)) f.local_20 /= 2;
                    }
                }
                int candidate_score = f.local_20;
                const int proto = Vehs[actor].proto_id;
                if (!Vehs[actor].moves_expended &&
                    (map_loc(f.local_18,f.local_14)->bit & 4) &&
                    (map_loc(f.local_4,f.local_8)->bit & 4))
                    candidate_score += Chassis[VehPrototypes[proto].chassis_id].speed >= 2 ? 256 : 8;
                if ((map_loc(f.local_4,f.local_8)->bit & 0x20) &&
                    (map_loc(f.local_4,f.local_8)->climate & 0xe0) >= 0x40) {
                    if (has_project(8,f.local_c) ||
                        (Weapon[VehPrototypes[proto].weapon_id].offense_rating < 0 && f.local_ac < 0x40)) {
                        if (proto < 0x40 &&
                            (Weapon[VehPrototypes[proto].weapon_id].offense_rating < 0 || proto == 0xf))
                            candidate_score += 256;
                        else candidate_score += 8;
                    } else if (!has_tech(Rules.tech_ease_fungus_movement,f.local_c))
                        candidate_score -= 256;
                }
                f.local_1c = veh_at(f.local_4,f.local_8);
                // Pinned5728DF has ONE call; Kuna incorrectly duplicates it.
                candidate_score -= stack_check(f.local_1c,1,-1,-1,-1) * 8;
                if (candidate_score >= f.local_28) {
                    f.local_28 = candidate_score;
                    f.local_80 = (int)f.local_2c;
                }
            }
        }
        ++f.local_2c;
    } while (f.local_2c < 8);
    if (f.local_80 >= 0) {
        Vehs[actor].move_to_ai_type = 0x33;
        return SELECTED_JOIN_578B26;
    }
    return SELECTED_JOIN_572922;
}


// Source-shaped subgraph for enemy_move 0x56DA90..0x56DC96.
//
// This file is an ignored integration fragment. The caller must arrive only
// after the native 0x56DA35 gates that precede the radius probe. Its inputs
// below are the exact values consumed by that block; it leaves every other
// raw stack slot untouched. The two outputs are direct CFG labels, not a
// request to rerun a later action/sea helper.

enum EnemyMoveUpstreamProbeJoin {
    ENEMY_MOVE_UPSTREAM_PROBE_CONTINUE_56DC96,
    ENEMY_MOVE_UPSTREAM_PROBE_JOIN_57708A
};

// At 0x56DA90 the original checks the three upstream eligibility locals,
// then scans exactly the first nine RadiusOffset entries. A winner at index
// zero takes the marker/damage store path; a later winner calls go_to.
// Both arrive at 0x57708A. No winner leaves the caller at 0x56DC96.
static EnemyMoveUpstreamProbeJoin enemy_move_upstream_56da90_probe(
    EnemyMoveFullFrame &s) {
    Veh &acting = Vehs[s.vehicle_id];
    const int proto_id = acting.proto_id;
    const int faction_id = s.local_c;
    const int weapon_id = VehPrototypes[proto_id].weapon_id;

    if (s.local_90 < 3 || s.local_84 != 0 ||
        s.local_7c == 0 || s.local_40 != 0 ||
        Weapon[weapon_id].offense_rating == 0) {
        return ENEMY_MOVE_UPSTREAM_PROBE_CONTINUE_56DC96;
    }

    for (int offset = 0; offset < 9; ++offset) {
        const int x = wrap_map_x(s.local_18 + RadiusOffsetX[offset]);
        const int y = s.local_14 + RadiusOffsetY[offset];
        // Native 0x56DAB5/0x56DABA publishes each candidate into raw -4/-8
        // before reading its map data. These are live frame stores, not a
        // disposable pair of C++ temporaries.
        s.local_4 = x;
        s.local_8 = y;
        if (!on_map(x, y)) continue;

        const Map *tile = map_loc(x, y);
        if ((tile->climate & 0xe0u) < 0x60u ||
            (tile->bit & 0x00002000u) == 0) {
            continue;
        }
        const int current_proto_id = acting.proto_id;
        if (ExpansionEnabled != 0 &&
            (current_proto_id == 0x10 || current_proto_id == 0x11 ||
             current_proto_id == 0x12)) {
            continue;
        }

        const int owner = anything_at(x, y);
        // Native 0x56DB30 stores the query result in raw -0x70. Its JL edge
        // at 0x56DB35 reaches the success route; it does not advance to another
        // offset. The same route is taken for an own-faction occupant and pact.
        s.local_70 = owner;
        if (owner >= 0 && owner != faction_id &&
            (PlayersData[faction_id].diplo_treaties[owner] & 1u) == 0)
            continue;

        if (offset == 0) {
            acting.move_to_ai_type = 0x64;
            s.local_8c = 1;
            acting.dmg_incurred = 0;
        } else {
            go_to(s.vehicle_id, 0x64, x, y);
        }
        return ENEMY_MOVE_UPSTREAM_PROBE_JOIN_57708A;
    }
    return ENEMY_MOVE_UPSTREAM_PROBE_CONTINUE_56DC96;
}

enum EnemyMoveUpstreamLowStackJoin {
    ENEMY_MOVE_UPSTREAM_LOW_STACK_CONTINUE_56DDD7,
    ENEMY_MOVE_UPSTREAM_LOW_STACK_JOIN_578B26
};

// Native 0x56DC96..0x56DDD7 low-altitude stack search. It is reached only
// through the exact raw-local gates below. A matching plan-7 stack sets the
// native move marker and direction (-0x80), then joins the existing 0x578B26
// waypoint writer. No direction/local is initialized on the decline edge.
static EnemyMoveUpstreamLowStackJoin
enemy_move_upstream_56dc96_low_stack(EnemyMoveFullFrame &f) {
    Veh &acting = Vehs[f.vehicle_id];
    if (f.local_90 < 3 || f.local_84 != 0 || f.local_40 != 0 ||
        f.local_7c == 0)
        return ENEMY_MOVE_UPSTREAM_LOW_STACK_CONTINUE_56DDD7;
    if (veh_cargo(f.vehicle_id) != 0 ||
        (acting.state & 0x8u) != 0)
        return ENEMY_MOVE_UPSTREAM_LOW_STACK_CONTINUE_56DDD7;

    for (int direction = 0; direction < 8; ++direction) {
        const int x = wrap_map_x(f.local_18 + VehMoveDeltaX[direction]);
        const int y = f.local_14 + VehMoveDeltaY[direction];
        f.local_4 = x;
        f.local_8 = y;
        if (!on_map(x, y) || (map_loc(x, y)->climate & 0xe0u) > 0x5fu)
            continue;

        f.local_1c = stack_fix(veh_at(x, y));
        for (; f.local_1c >= 0;
             f.local_1c = Vehs[f.local_1c].next_veh_id_stack) {
            const int candidate_id = f.local_1c;
            if (Vehs[candidate_id].faction_id != f.local_c ||
                VehPrototypes[Vehs[candidate_id].proto_id].plan != 7 ||
                stack_check(candidate_id, 8, -1, -1, -1) <= 0) {
                continue;
            }
            acting.move_to_ai_type = 0x64;
            f.local_80 = direction;
            return ENEMY_MOVE_UPSTREAM_LOW_STACK_JOIN_578B26;
        }
    }
    return ENEMY_MOVE_UPSTREAM_LOW_STACK_CONTINUE_56DDD7;
}

enum EnemyMoveUpstreamDDD7Join {
    ENEMY_MOVE_UPSTREAM_DDD7_CONTINUE_56DEB8,
    ENEMY_MOVE_UPSTREAM_DDD7_JOIN_578B34
};

// Native 0x56DDD7..0x56DEB8 tests the separate low-altitude/triad/distance
// locals. On the nonzero-distance arm it walks the acting vehicle's own stack
// (veh_top, then next_veh_id_stack) and suppresses the 0x578B34 route if a
// plan-7 member has damage exceeding proto_power/3. This is
// not the neighboring-tile stack scan above.
static EnemyMoveUpstreamDDD7Join enemy_move_upstream_56ddd7_stack_budget(
    EnemyMoveFullFrame &f) {
    Veh &acting = Vehs[f.vehicle_id];
    if (f.local_90 <= 2 || f.local_74 == 0 || f.local_40 != 0)
        return ENEMY_MOVE_UPSTREAM_DDD7_CONTINUE_56DEB8;

    if (f.local_7c == 0) {
        acting.move_to_ai_type = 0x64;
        return ENEMY_MOVE_UPSTREAM_DDD7_JOIN_578B34;
    }

    int should_join_578b34 = 1;
    f.local_1c = veh_top(f.vehicle_id);
    for (; f.local_1c >= 0;
         f.local_1c = Vehs[f.local_1c].next_veh_id_stack) {
        const int member_id = f.local_1c;
        if (VehPrototypes[Vehs[member_id].proto_id].plan == 7) {
            const int power = proto_power(member_id);
            if (Vehs[member_id].dmg_incurred > power / 3)
                should_join_578b34 = 0;
        }
    }
    if (!should_join_578b34)
        return ENEMY_MOVE_UPSTREAM_DDD7_CONTINUE_56DEB8;

    acting.move_to_ai_type = 0x64;
    return ENEMY_MOVE_UPSTREAM_DDD7_JOIN_578B34;
}

// Native 0x571537..0x571577 is entered only when the map/region and high
// current-tile gates at 0x571362 succeed. It conditionally sets the current
// faction's raw PlayerData::unk_77[region] bit, always requests order 'G' at
// the already-selected candidate coordinates, then joins 0x57707F.
static void enemy_move_upstream_571537_tile_route(EnemyMoveFullFrame &f) {
    if (Vehs[f.vehicle_id].state & 0x40000000u)
        PlayersData[f.local_c].unk_77[f.local_34] |= 0x2000u;
    go_to(f.vehicle_id, 0x47, f.local_4, f.local_8);
}

// Native 56FE51 passes this storage address to message_base. Only its
// initial NUL byte is established; the historical complete capacity,
// TU/name and indirect writes remain unknown. This is a start-only view.
// RECOVERY_OBJECT: v1 0x0093FC30 ?EnemyBuildBaseMessageText@@3PADA
char EnemyBuildBaseMessageText[1] = {0};

// Complete owner assembly. Native stack words are stored only on proven edges.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0056B5B0 0x00579208 0x00579238 0x00577DD3
// ORIGINAL: 0x0056B5B0 ?enemy_move@@YAHH@Z 0x0056B5B0-0x00579208 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:9d6ac074ba331b079a584bc73bfd0ce20699a9b4:src/unrecovered/0056b5b0.cpp is deleted seed history; its original identity, full span, claims and useful lessons are retained here through the source writer.", "Pinned instructions establish distinct raw route/damage flags, integer stack priority, saved versus live callback reads, and original unguarded/uninitialized error paths.", "Complete selected, strategic, nonsea, sea and tactical continuations execute once; helpers do not introduce new original identities.", "Vehicle, prototype, PlayerData and Base fields use their real source owners; original matching and full runtime restoration obligations remain."], "reason": "Complete typed per-vehicle enemy movement owner with one path-dependent raw frame and genuine product owners; superseded executable forwarding and scaffold are removed together."}
// kind game
int __cdecl enemy_move(int incoming_vehicle_id) {
    EnemyMoveFullFrame f;
    int available_units;
    int defensive_units;
    int capacity_limit;
    SeaScoringExit sea_exit;
    f.vehicle_id = incoming_vehicle_id;
    f.local_80 = -1;
    f.local_84 = 0;
    f.local_c4 = 0;
    f.local_b4 = 0;
    f.local_8c = 0;
    f.local_c0 = 0;
    f.local_b8 = 0;
    if (f.vehicle_id < 0) {
        f.vehicle_id = ~f.vehicle_id;
        f.local_6c = 1;
    } else f.local_6c = 0;
    f.local_a0 = IsMultiplayerNet != 0 &&
        Vehs[f.vehicle_id].faction_id == ConsoleGlobal.faction_id_;
    do_all_non_input();
    f.local_10 = f.vehicle_id * static_cast<int>(sizeof(Veh));
    if (Vehs[f.vehicle_id].faction_id == 0 && !f.local_a0)
        return alien_move(f.vehicle_id);
    f.local_ac = Vehs[f.vehicle_id].proto_id;
    f.local_c = Vehs[f.vehicle_id].faction_id;
    f.local_14 = Vehs[f.vehicle_id].y;
    f.local_18 = Vehs[f.vehicle_id].x;
    f.local_50 = VehPrototypes[f.local_ac].plan;
    f.local_40 = Chassis[VehPrototypes[f.local_ac].chassis_id].triad;
    f.local_a4 = map_loc(f.local_18,f.local_14)->climate >> 5;
    f.local_74 = (map_loc(f.local_18,f.local_14)->climate & 0xe0) < 0x60;
    f.local_34 = map_loc(f.local_18,f.local_14)->region;
    f.local_98 = (Vehs[f.vehicle_id].state & 0x60000u) == 0x40000u;
    f.local_a8 = (Vehs[f.vehicle_id].state & 0x60000u) == 0x60000u;
    f.local_44 = f.local_50 == 3 && has_abil(f.local_ac,0x800) != 0;
    f.local_3c = f.local_c * static_cast<int>(sizeof(PlayerData));
    f.local_88 = PlayersData[f.local_c].region_base_plan[f.local_34];
    enemy_move_damage_priority(f.vehicle_id,f);
    f.local_bc = zoc_move(f.local_18,f.local_14,f.local_c);
    f.local_94 = zoc_veh(f.local_18,f.local_14,f.local_c);
    f.local_9c = base_find(f.local_18,f.local_14,-1,
        f.local_40 ? -1 : f.local_34,-1,-1);
    f.local_7c = BaseFindDist;
    f.local_5c = base_find(f.local_18,f.local_14,f.local_c,
        f.local_40 ? -1 : f.local_34,-1,-1);
    f.local_64 = BaseFindDist;
    f.local_60 = whose_territory(f.local_c,f.local_18,f.local_14,NULL,0);
    f.local_4c = f.local_60 == f.local_c;
    f.local_b0 = Vehs[f.vehicle_id].state & 0x4000u;
    if (f.local_6c) goto label_57138d;
    if ((Vehs[f.vehicle_id].flags & 0x440) != 0 &&
        !(static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << (f.local_c & 31)))) {
        veh_skip(f.vehicle_id);
        return 0;
    }
    if (!veh_moves(f.vehicle_id)) {
        f.local_b8 = 1;
        goto label_574a7e;
    }
    if (Vehs[f.vehicle_id].order > 3 && Vehs[f.vehicle_id].order < 24) {
        if (!f.local_94 || (f.local_94 >= 2 &&
            (!(static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << (f.local_c & 31))) ||
             !(PlayersData[f.local_c].diplo_treaties[f.local_94-1] & 0x10u))))
            goto label_578f46;
        if ((Vehs[f.vehicle_id].state & 0x200u) &&
            (Vehs[f.vehicle_id].order_auto_type == 4 ||
             Vehs[f.vehicle_id].order_auto_type == 5)) {
            // Native56BA5A/62 joins56C2D1, not the terrain-policy entry.
            Vehs[f.vehicle_id].order = 0;
            Vehs[f.vehicle_id].state &= ~0x200u;
            return 0;
        }
        {
            const EnemyMoveTerraformDirectionFacts direction =
                enemy_move_terraform_direction(f.local_18,f.local_14,f.local_c,
                    f.local_40,f.local_bc,f);
            if (direction.native_local_70_written)
                f.local_70 = direction.native_local_70_value;
            if (direction.direction >= 0) {
                f.local_80 = direction.direction;
                goto label_578b26;
            }
        }
        if (Vehs[f.vehicle_id].state & 0x200u) {
            Vehs[f.vehicle_id].order = 0;
            Vehs[f.vehicle_id].state &= ~0x20000u;
            return 0;
        }
    }
    if (f.local_94 >= 2 && f.local_98 &&
        (PlayersData[f.local_c].diplo_treaties[f.local_94-1] & 0x10u)) {
        f.local_98 = 0;
        Vehs[f.vehicle_id].state &= ~0x40000u;
        if (f.local_18 == PlayersData[f.local_c].x_target &&
            f.local_14 == PlayersData[f.local_c].y_target) {
            PlayersData[f.local_c].x_target = -1;
            f.local_a8 = 1;
            f.local_1c = f.vehicle_id;
            while (f.local_1c >= 0 && Vehs[f.local_1c].prev_veh_id_stack >= 0)
                f.local_1c = Vehs[f.local_1c].prev_veh_id_stack;
            while (f.local_1c >= 0) {
                if (f.local_1c == f.vehicle_id || (Vehs[f.local_1c].state & 0x40000u))
                    Vehs[f.local_1c].state |= 0x60000u;
                f.local_1c = Vehs[f.local_1c].next_veh_id_stack;
            }
        }
    }
    if (f.local_a8 && Vehs[f.vehicle_id].moves_expended >= Rules.move_rate_roads &&
        !f.local_94 && !(static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                       (1u << (f.local_c & 31))) &&
        stack_check(f.vehicle_id,14,-1,-1,-1)) {
        veh_skip(f.vehicle_id);
        goto label_578f46;
    }
    if (f.local_98 && Vehs[f.vehicle_id].x == PlayersData[f.local_c].x_target &&
        Vehs[f.vehicle_id].y == PlayersData[f.local_c].y_target) {
        f.local_b4 = 1;
        f.local_8c = 1;
        goto label_57707f;
    }
    if ((Vehs[f.vehicle_id].state & 0x1000200u) == 0x1000200u ||
        ((Vehs[f.vehicle_id].state & 0x2000200u) == 0x2000200u &&
         Vehs[f.vehicle_id].patrol_current_point < Vehs[f.vehicle_id].waypoint_count))
        goto label_578de9;
    f.local_68 = 1u << (f.local_c & 31);
    {
        const EnemyMoveNoWaypointJoin join = enemy_move_no_waypoint_dispatch(
            f.vehicle_id,f.local_c,f.local_50,f.local_18,f.local_14);
        switch (join) {
        case ENEMY_MOVE_JOIN_578DE9: goto label_578de9;
        case ENEMY_MOVE_JOIN_56C9E1: goto label_56c9e1;
        case ENEMY_MOVE_JOIN_56C35D: goto label_56c35d;
        case ENEMY_MOVE_JOIN_572922:
            f.local_24 = Vehs[f.vehicle_id].home_base_id;
            if (f.local_24 < 0) f.local_24 = f.local_5c;
            if (f.local_24 < 0) {
                Vehs[f.vehicle_id].state &= ~0x200u;
                veh_skip(f.vehicle_id);
                return 0;
            }
            goto label_572922;
        case ENEMY_MOVE_JOIN_56C6D3: goto label_56c6d3;
        case ENEMY_MOVE_JOIN_56BF5F: goto label_56bf5f;
        case ENEMY_MOVE_RETURN_ZERO: return 0;
        case ENEMY_MOVE_JOIN_578F46: goto label_578f46;
        }
    }
label_56bf5f:
    {
        const RulesTerraforming &remove = Terraforming[TERRA_REMOVE_FUNGUS];
        const int tech = f.local_40 == 1 ? remove.prerequisite_tech[1] : remove.prerequisite_tech[0];
        if (!has_tech(tech,f.local_c) &&
            !has_abil(Vehs[f.vehicle_id].proto_id,0x2000) &&
            Vehs[f.vehicle_id].order_auto_type == 6) {
            Vehs[f.vehicle_id].order = 0;
            Vehs[f.vehicle_id].state &= ~0x200u;
            return 0;
        }
        if (Vehs[f.vehicle_id].order == 0x18) goto label_578f46;
        if ((map_loc(f.local_18,f.local_14)->bit & 0x20u) &&
            (map_loc(f.local_18,f.local_14)->climate & 0xe0) >= 0x40) {
            Vehs[f.vehicle_id].order = 0x0e;
            goto label_578f46;
        }
        if (f.local_40 == 1 &&
            (map_loc(f.local_18,f.local_14)->climate & 0xe0) >= 0x60)
            goto label_57717f;
    }
    f.local_28 = 0x1869f;
    f.local_48 = 2;
    do {
        if (Vehs[f.vehicle_id].order == 0x18) goto label_578f46;
        f.local_24 = 0;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            int distance_scratch = f.local_38; // original uninitialized reaching read
            do {
                if (Bases[f.local_24].faction_id_current == f.local_c &&
                    (f.local_40 == 1 ? base_on_sea(f.local_24,f.local_34) != 0 :
                     map_loc(Bases[f.local_24].x,Bases[f.local_24].y)->region == f.local_34)) {
                    int offset = f.local_48 == 2 ? 1 : RadiusRange[f.local_48-1];
                    while (offset < RadiusRange[f.local_48]) {
                        f.local_4 = xrange(Bases[f.local_24].x+RadiusOffsetX[offset]);
                        f.local_8 = Bases[f.local_24].y+RadiusOffsetY[offset];
                        if (on_map(f.local_4,f.local_8)) {
                            const Map &tile = *map_loc(f.local_4,f.local_8);
                            if (tile.region == f.local_34 && (tile.bit & 0x20u) &&
                                (tile.climate & 0xe0) >= 0x40) {
                                f.local_70 = -1;
                                if (tile.bit & 3u) {
                                    f.local_70 = tile.val2 & 15;
                                    if (f.local_70 >= 8) f.local_70 = -1;
                                }
                                if (f.local_70 < 0 ||
                                    (f.local_70 == f.local_c && !distance_scratch)) {
                                    const int dy = abs(f.local_14-f.local_8);
                                    const int dx = x_dist(f.local_18,f.local_4);
                                    distance_scratch = vector_dist(dx,dy);
                                    if (!zoc_move(f.local_4,f.local_8,f.local_c) &&
                                        distance_scratch < f.local_28) {
                                        f.local_28 = distance_scratch;
                                        Vehs[f.vehicle_id].order = 0x18;
                                        Vehs[f.vehicle_id].move_to_ai_type = 0x66;
                                        Vehs[f.vehicle_id].waypoint_x[0] = static_cast<short>(f.local_4);
                                        Vehs[f.vehicle_id].waypoint_y[0] = static_cast<short>(f.local_8);
                                        if (VehPrototypes[Vehs[f.vehicle_id].proto_id].plan == 8)
                                            Vehs[f.vehicle_id].terraforming_turns = 0;
                                    }
                                }
                            }
                        }
                        ++offset;
                    }
                }
                ++f.local_24;
            } while (f.local_24 < GameDataStateStorage.fields_.base_current_count_);
            f.local_38 = distance_scratch;
        }
        ++f.local_48;
    } while (f.local_48 <= (Vehs[f.vehicle_id].order_auto_type == 6 ? 6 : 3));
    if (Vehs[f.vehicle_id].order == 0x18) goto label_578f46;
    if (Vehs[f.vehicle_id].order_auto_type != 6) goto label_56c9e1;
    Vehs[f.vehicle_id].order = 0;
    Vehs[f.vehicle_id].state &= ~0x200u;
    return 0;

label_56c35d:
    f.local_28 = 0x270f;
    f.local_30 = (Vehs[f.vehicle_id].order_auto_type == 2)+0x1b;
    f.local_78 = -1;
    f.local_24 = 0;
    if (GameDataStateStorage.fields_.base_current_count_ > 0) {
        f.base_cursor_48 = &Bases[0].y;
        do {
            const int owner = Bases[f.local_24].faction_id_current;
            if ((owner == f.local_c ||
                 ((PlayersData[f.local_c].diplo_treaties[owner] & 1u) &&
                  (f.local_7c || Bases[f.local_9c].faction_id_current == f.local_c))) &&
                map_loc(Bases[f.local_24].x,Bases[f.local_24].y)->region == f.local_34) {
                const int base_x = Bases[f.local_24].x;
                f.local_58 = Bases[f.local_24].y;
                int path_x = f.local_18;
                int path_y = f.local_14;
                f.local_54 = 1;
                f.local_2c = 0;
                f.local_38 = 0;
                do {
                    int road_mode = 4;
                    if (has_tech(Rules.tech_build_roads_fungus,f.local_c) ||
                        has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[0],f.local_c))
                        road_mode = 0;
                    f.local_80 = Paths.find(path_x,path_y,base_x,f.local_58,0,-1,road_mode,-1);
                    if (f.local_80 < 0) break;
                    path_x = xrange(path_x+VehMoveDeltaX[f.local_80]);
                    path_y += VehMoveDeltaY[f.local_80];
                    ++f.local_38;
                    if (path_x == base_x && path_y == f.local_58) break;
                    const Map &tile = *map_loc(path_x,path_y);
                    if (!(tile.bit & 1u) || (tile.val2 & 15) >= 8) {
                        if (!(tile.bit & 4u) ||
                            (!(tile.bit & 8u) && Vehs[f.vehicle_id].order_auto_type == 2))
                            f.local_54 = 0;
                    }
                    ++f.local_2c;
                    if (f.local_2c > 100) break;
                } while (f.local_54);
                if (!f.local_54) {
                    if (Bases[f.local_24].faction_id_current != f.local_c) f.local_38 *= 2;
                    const int path_owner = whose_territory(f.local_c,path_x,path_y,NULL,0);
                    if (path_owner >= 0 && path_owner != f.local_c &&
                        !(PlayersData[f.local_c].diplo_treaties[path_owner] & 1u))
                        f.local_38 += 100;
                    if (zoc_sea(path_x,path_y,f.local_c)) f.local_38 += 10;
                    if (f.local_38 < f.local_28) {
                        f.local_28 = f.local_38;
                        f.local_78 = f.local_24;
                    }
                }
            }
            ++f.local_24;
            f.base_cursor_48 += sizeof(Base)/sizeof(short);
        } while (f.local_24 < GameDataStateStorage.fields_.base_current_count_);
        if (f.local_78 >= 0) {
            Vehs[f.vehicle_id].move_to_ai_type = 0x52;
            Vehs[f.vehicle_id].order = static_cast<unsigned char>(f.local_30);
            Vehs[f.vehicle_id].waypoint_x[0] = Bases[f.local_78].x;
            Vehs[f.vehicle_id].waypoint_y[0] = Bases[f.local_78].y;
            goto label_578f46;
        }
    }
    if (!(static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & static_cast<unsigned char>(f.local_68)) &&
        (Vehs[f.vehicle_id].flags & 0x800)) goto label_56c9e1;
    Vehs[f.vehicle_id].order = 0;
    Vehs[f.vehicle_id].state &= ~0x200u;
    veh_skip(f.vehicle_id);
    if (f.local_c == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.focus(f.local_18,f.local_14,f.local_c);
        X_pop(Vehs[f.vehicle_id].order_auto_type == 1 ? "AUTOROADSDONE" : "AUTOTUBESDONE",NULL);
    }
    return 0;

label_56c6d3:
    if (enemy_move_ordinary_auto_terrain_policy(f.vehicle_id,f.local_c,f.local_18,
        f.local_14,f.local_74 != 0,f.local_a4) == ENEMY_MOVE_TERRAIN_RETURN_ZERO) return 0;
    goto label_578f46;

label_56c9e1:
    // The upstream closure dispatches the original C9E1..571384 labels.
// Native 0x56C9E1 entry and its adjacent/rival-base prefix. This is included
// inside enemy_move; all stack locals remain the shared raw frame `f`.
{
    if (f.local_7c == 1) {
        const Base &adjacent = Bases[f.local_9c];
        const Veh &acting = Vehs[f.vehicle_id];
        const VehPrototype &design = VehPrototypes[acting.proto_id];
        if (adjacent.faction_id_current == f.local_c && f.local_40 != 2 &&
            Weapon[design.weapon_id].offense_rating == 0) {
            const Map *base_tile = map_loc(adjacent.x, adjacent.y);
            if (base_tile->region == f.local_34 &&
                (base_tile->bit & 7u) == 0 &&
                (((base_tile->climate & 0xe0u) < 0x60u) ==
                    (f.local_40 == 1)) && f.local_50 <= 9) {
                f.local_84 = 1;
                f.local_90 = 0;
                Vehs[f.vehicle_id].order = 0;
            }
        }
    }

    if (f.local_9c != f.local_5c) {
        const int rival_owner = Bases[f.local_9c].faction_id_current;
        const uint32_t relation =
            PlayersData[f.local_c].diplo_treaties[rival_owner];
        const Veh &acting = Vehs[f.vehicle_id];
        if ((relation & 2u) != 0 && f.local_74 == 0 &&
            (relation & 1u) == 0 && acting.order != 0x18 &&
            whose_territory(f.local_c,f.local_18,f.local_14,NULL,0) == rival_owner &&
            static_cast<int>(PlayersData[f.local_c]
                .diplo_friction[rival_owner]) < 10 &&
            (PlayersData[f.local_c].diplo_treaties[rival_owner] & 0x0c00u) == 0 &&
            ((acting.faction_id == rival_owner && (acting.flags & 0x440u) != 0x440u) ||
             (acting.visibility & static_cast<unsigned char>(1u << rival_owner)) != 0)) {
            goto label_56cbd2;
        }
    }
    goto label_56cd0c;
}

label_56cbd2: {
    if (f.local_5c < 0) goto label_56cc2f;
    const Base &selected = Bases[f.local_5c];
    if (map_loc(selected.x, selected.y)->region == f.local_34) {
        go_to(f.vehicle_id, 0x77, selected.x, selected.y);
        goto label_57707f;
    }
    goto label_56cc2f;
}

label_56cc2f: {
    const Veh &acting = Vehs[f.vehicle_id];
    const int chassis = VehPrototypes[acting.proto_id].chassis_id;
    if (Chassis[chassis].triad != 0) goto label_56cd0c;
    for (int direction = 0; direction < 8; ++direction) {
        const int x = xrange(f.local_18 + VehMoveDeltaX[direction]);
        const int y = f.local_14 + VehMoveDeltaY[direction];
        f.local_4 = x;
        f.local_8 = y;
        if (!on_map(x, y) || (map_loc(x, y)->climate & 0xe0u) >= 0x60u)
            continue;
        const int candidate = stack_fix(veh_at(x, y));
        f.local_1c = candidate;
        if (candidate < 0 || Vehs[candidate].faction_id != f.local_c ||
            stack_check(candidate, 8, -1, -1, -1) <= 0) continue;
        // Native CCFC joins FEEE: only the direction store precedes the
        // waypoint writer. It does not install an independent order/marker.
        f.local_80 = direction;
        goto label_578b26;
    }
    goto label_56cd0c;
}

label_56cd0c: {
    const Veh &acting = Vehs[f.vehicle_id];
    if ((acting.state & 0x00000200u) != 0 &&
        acting.order_auto_type == 0x0b && f.local_40 != 2) {
        f.local_8c = 1;
        goto label_57707f;
    }
    goto label_56cfc1;
}

label_56cfc1: {
    Veh &acting = Vehs[f.vehicle_id];
    if (acting.move_to_ai_type == 0x46) {
        if (acting.order != 0x18) acting.move_to_ai_type = 0x2d;
        if ((acting.state & 0x00000100u) != 0 && f.local_50 == 2) {
            acting.move_to_ai_type = 0x2d;
            f.local_24 = acting.home_base_id;
            if (acting.home_base_id < 0) goto label_56cdc1;
            const Base &home = Bases[acting.home_base_id];
            if ((home.x != f.local_18 || home.y != f.local_14) &&
                map_loc(home.x, home.y)->region == f.local_34) {
                go_to(f.vehicle_id, 0x46, home.x, home.y);
                goto label_57707f;
            }
            if (home.x == f.local_18 && home.y == f.local_14)
                Bases[acting.home_base_id].state &= ~0x04000000u;
            goto label_56cdc1;
        }
    }

    if (f.local_50 == 12 && (acting.flags & 0x20u) == 0 &&
        f.local_64 == 0) {
        if (f.local_a0 != 0) {
            veh_skip(f.vehicle_id);
            synch_veh(f.vehicle_id);
            return 1;
        }
    }

    if (f.local_50 == 12) {
        const int project_base = GameDataStateStorage.fields_.secret_project_.universal_translator();
        if ((acting.flags & 0x20u) == 0 && f.local_64 == 0) {
            const bool network_node =
                has_fac(FAC_NETWORK_NODE, f.local_5c, 0) != 0;
            const bool artifact_base_valid =
                (network_node &&
                 ((Bases[f.local_5c].state & 0x04000000u) == 0 ||
                  project_base == f.local_5c)) ||
                (!network_node && project_base == f.local_5c);
            if (artifact_base_valid && study_artifact(f.vehicle_id) != 0)
                return 1;
        }
        int best_base = -1;
        int best_score = 0x270f;
        f.local_28 = best_score;
        f.local_78 = best_base;
        f.local_24 = 0;
        const uint8_t lock_mask =
            static_cast<uint8_t>(1u << (f.local_c & 0x1f));
        for (; f.local_24 < GameDataStateStorage.fields_.base_current_count_; ++f.local_24) {
            const int base_id = f.local_24;
            Base &base = Bases[base_id];
            if (base.faction_id_current != f.local_c ||
                map_loc(base.x,base.y)->region != f.local_34) continue;
            int score = vector_dist(x_dist(f.local_18, base.x),
                                    abs(f.local_14 - base.y));
            if (base_id != project_base) {
                const bool skip_penalties = base.queue_production_id[0] < -0x46 &&
                    base.minerals_accumulated >= 0x64 &&
                    (lock_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0;
                if (!skip_penalties) {
                    if (!has_fac(FAC_NETWORK_NODE, base_id, 0)) score += 0x3e8;
                    if (base.state & 0x04000000u) {
                        score += 0x7d0;
                    } else {
                        const int stack = stack_check(
                            veh_at(base.x, base.y), 2, 0x0c, -1, -1);
                        if (stack > static_cast<int>(base.x == f.local_18 && base.y == f.local_14))
                            score += 0x7d0;
                    }
                }
            }
            if (score >= best_score) continue;
            best_score = score;
            best_base = base_id;
            f.local_28 = score;
            f.local_78 = base_id;
        }
        if (best_base >= 0) {
            go_to(f.vehicle_id, 0x74, Bases[best_base].x, Bases[best_base].y);
            goto label_578f46;
        }
        goto label_57707f;
    }

    // Native D265 dispatches every non-plan-10 unit directly to CDC1.
    if (f.local_50 != 10) goto label_56cdc1;

    if (f.local_50 == 10 &&
        (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<unsigned char>(f.local_68)) == 0) {
        f.local_28 = 0x270f;
        f.local_78 = -1;
        f.local_24 = 0;
        int best_base = -1;
        int best_distance = 0x270f;
        for (; f.local_24 < GameDataStateStorage.fields_.base_current_count_; ++f.local_24) {
            const int base_id = f.local_24;
            const Base &base = Bases[base_id];
            if (base.faction_id_current != f.local_c ||
                map_loc(base.x, base.y)->region != f.local_34) continue;
            f.local_4c = f.local_18;
            const int distance =
                (x_dist(f.local_4c,base.x)+abs(f.local_14-base.y)) >> 1;
            if (base.queue_production_id[0] > -0x46 ||
                distance >= best_distance) continue;
            best_distance = distance;
            best_base = base_id;
            f.local_28 = distance;
            f.local_78 = base_id;
            if (distance == 0) {
                Base &selected = Bases[base_id];
                const int cost = veh_cost(acting.proto_id, -1, NULL);
                selected.minerals_accumulated += cost * 10;
                selected.unused_surplus = selected.minerals_accumulated;
                selected.production_id_last = selected.queue_production_id[0];
                kill(f.vehicle_id);
                return 1;
            }
        }
        if (best_base >= 0) {
            go_to(f.vehicle_id, 0x56, Bases[best_base].x, Bases[best_base].y);
            goto label_57707f;
        }
    }

    if (acting.order == 3) {
        const Map *tile = map_loc(f.local_18, f.local_14);
        bool make_goal = false;
        switch (acting.order_auto_type) {
        case 0: make_goal = (tile->bit & 0x8000u) == 0 &&
                            (tile->val3 & 0xc0u) < 0x80u; break;
        case 1: make_goal = ((~tile->bit >> 4) & 1u) != 0; break;
        case 2: make_goal = (tile->bit & 0x40u) == 0 && f.local_a4 > 2; break;
        default: break;
        }
        if (make_goal) add_goal(f.local_c, 9, 1, f.local_18, f.local_14, -1);
        goto label_578f46;
    }

    if (acting.order != 0x18 && f.local_50 != 12) {
        int best_score = 0;
        int best_resource = 0;
        f.local_28 = 0;
        f.local_44 = 0;
        f.local_78 = 0;
        for (int offset = 0; offset < 289; ++offset) {
            const int x = xrange(f.local_18 + RadiusOffsetX[offset]);
            const int y = f.local_14 + RadiusOffsetY[offset];
            f.local_4 = x;
            f.local_8 = y;
            f.local_44 = offset * 4;
            if (!on_map(x, y)) continue;
            const Map *tile = map_loc(x, y);
            if (tile->region != f.local_34) continue;
        if (f.local_40 != 2 &&
                (((tile->climate & 0xe0u) < 0x60u) != (f.local_40 == 1)))
                continue;
            if (!can_convoy(x, y)) continue;
            if (!bonus_at(x, y, 0)) {
                tile = map_loc(x,y);
                if ((tile->val3 & 0xc0u) < 0x80u &&
                    (tile->climate & 0xe0u) < 0xc0u) continue;
            }
            if (whose_territory(f.local_c, x, y, NULL, 0) != f.local_c)
                continue;
            (void)base_find(x, y, f.local_c);
            if (BaseFindDist <= 2) continue;
            bool occupied_goal = false;
            f.local_1c = 0;
            for (; f.local_1c < GameDataStateStorage.fields_.vehicle_current_count_; ++f.local_1c) {
                const int other_id = f.local_1c;
                const Veh &other = Vehs[other_id];
                if (other_id == f.vehicle_id || other.faction_id != f.local_c ||
                    VehPrototypes[other.proto_id].plan != 10) continue;
                if ((other.x == x && other.y == y && other.order == 3) ||
                    (other.order == 0x18 && other.waypoint_x[0] == x &&
                     other.waypoint_y[0] == y)) {
                    occupied_goal = true;
                    break;
                }
            }
            if (occupied_goal) continue;
            const int distance =
                (x_dist(f.local_18, x) + abs(f.local_14 - y)) >> 1;
            int yield_score = 1;
            int resource = 0;
            f.local_78 = 0;
            for (int type = 0; type < 3; ++type) {
                int value = resource_yield(type, f.local_c,
                    acting.home_base_id, x, y) * 2;
                if (type == 1) ++value;
                if (value >= yield_score) {
                    yield_score = value;
                    resource = type;
                    f.local_78 = type;
                }
            }
            const int score = (yield_score << 9) - distance;
            if (score <= best_score) continue;
            go_to(f.vehicle_id, 0x76, x, y);
            best_score = score;
            f.local_28 = score;
            best_resource = resource;
            if (offset == 0) {
                Vehs[f.vehicle_id].order = 3;
                Vehs[f.vehicle_id].order_auto_type =
                    static_cast<uint8_t>(best_resource);
                goto label_578f46;
            }
        }
        f.local_44 = 0x484;
        goto label_57707f;
    }
    goto label_56cdc1;
}

label_56cdc1:
    f.local_70 = f.local_60;
    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<unsigned char>(f.local_68)) != 0 ||
        f.local_60 == f.local_c || f.local_60 <= 0)
        goto label_56d7fc;
    f.local_48 = static_cast<int>(sizeof(PlayerData) / sizeof(uint32_t)) *
        f.local_c + f.local_60;
    if ((PlayersData[f.local_c].diplo_treaties[f.local_60] & 8u) == 0)
        goto label_56d7fc;
    goto label_56ce13;

label_56ce13: {
    const Veh &acting = Vehs[f.vehicle_id];
    const VehPrototype &design = VehPrototypes[acting.proto_id];
    // Pinned 56CE00 stores the flattened faction/territory treaty index
    // before its first mask test, even on all later decline edges.
    f.local_48 = static_cast<int>(sizeof(PlayerData) / sizeof(uint32_t)) *
        f.local_c + f.local_60;
    const uint32_t relation =
        PlayersData[f.local_c].diplo_treaties[f.local_60];
    if ((Weapon[design.weapon_id].offense_rating <= 2 && f.local_50 != 2) ||
        f.local_50 == 3 || (relation & 0x00040c30u) != 0 ||
        (relation & 3u) == 0 ||
        PlayersData[f.local_60].integrity_blemishes > 2) {
        goto label_56d7fc;
    }
    if (acting.order != 0x18) goto label_56d762;
    const int waypoint_owner = whose_territory(
        f.local_c, acting.waypoint_x[0], acting.waypoint_y[0], NULL, 0);
    if (waypoint_owner == f.local_c || waypoint_owner == f.local_60 ||
        waypoint_owner <= 0 || f.local_50 > 1 || f.local_40 != 0)
        goto label_56d762;
    const PlayerData &current = PlayersData[f.local_c];
    const PlayerData &territory = PlayersData[f.local_60];
    if (static_cast<int>(territory.ranking) >= static_cast<int>(current.ranking)) {
        const int multiplier = static_cast<int>((~relation & 1u) | 2u);
        if (PlayersData[waypoint_owner].mil_strength_1 <=
            (multiplier * territory.mil_strength_1) / 2) goto label_56d762;
    }
    if ((current.diplo_treaties[waypoint_owner] & 0x10u) != 0 &&
        (territory.diplo_treaties[waypoint_owner] & 0x10u) != 0) {
        action_give(f.vehicle_id, f.local_60);
        return 1;
    }
}

label_56d762: {
    const uint8_t state = PlayersData[f.local_60].region_base_plan[f.local_34];
    const uint32_t relation =
        PlayersData[f.local_c].diplo_treaties[f.local_60];
    const PlayerData &current = PlayersData[f.local_c];
    const PlayerData &territory = PlayersData[f.local_60];
    if (state == 2 && f.local_50 <= 2 && (relation & 1u) != 0) {
        action_give(f.vehicle_id, f.local_60);
        return 1;
    }
    if (current.region_total_bases[f.local_34] != 0 ||
        territory.region_total_bases[f.local_34] == 0 || f.local_40 != 0)
        goto label_56d7fc;
    if (state == 2) {
        if ((relation & 1u) != 0 || static_cast<int>(territory.ranking) < 7) {
            action_give(f.vehicle_id, f.local_60);
            return 1;
        }
    }
    if (state != 8 && (relation & 1u) != 0) {
        action_give(f.vehicle_id, f.local_60);
        return 1;
    }
}

label_56d7fc: {
    const Veh &acting = Vehs[f.vehicle_id];
    const int priority = f.local_90;
    if (priority == 0 || (f.local_a8 != 0 && priority <= 3))
        goto label_56e011;
    if (f.local_7c != 0 &&
        (f.local_40 != 2 ||
         (map_loc(f.local_18,f.local_14)->bit & 0x00040000u) == 0))
        goto label_56d8b6;
    if ((acting.state & 8u) != 0 ||
        (Bases[f.local_9c].state & 0x00200000u) != 0) {
        if (priority < 4 || veh_moves(f.vehicle_id) > Rules.move_rate_roads)
            goto label_56d8b6;
    }
    Vehs[f.vehicle_id].move_to_ai_type = 0x64;
    f.local_8c = 1;
    f.local_b4 = 1;
    goto label_571384;
}

label_56d8b6: {
    const Veh &acting = Vehs[f.vehicle_id];
    if (has_tech(0x16, f.local_c) == 0 || f.local_84 != 0 ||
        f.local_40 == 2 || acting.moves_expended != 0 ||
        (f.local_bc != 0 && f.local_90 <= 3) ||
        (acting.state & 8u) != 0) goto label_56d924;
    Vehs[f.vehicle_id].move_to_ai_type = 0x64;
    f.local_8c = 1;
    if (f.local_40 == 1) goto label_574a88;
    goto label_57707f;
}

label_56e011: {
    if (f.local_50 == 11) {
        if (f.local_a0 != 0) {
            Vehs[f.vehicle_id].order = 0;
            Vehs[f.vehicle_id].state &= ~0x00000200u;
            synch_veh(f.vehicle_id);
            NetDaemonNet.await_synch();
            NetDaemonNet.unlock_veh();
            return 1;
        }

        // Native 0x56E06A gates the offline pass on raw -0xb0, then
        // compares the reached low-altitude fact with (raw -0x40 == 1).
        // The comparison's result is stored in raw -0x48 before any scan.
        if (f.local_b0 == 0) {
            f.local_48 = (f.local_40 == 1);
            if (f.local_74 != f.local_48) goto label_56e507;
            for (f.local_58 = 0; f.local_58 < 0x54; f.local_58 += 4) {
                const int offset = f.local_58 / 4;
                const int x = xrange(f.local_18 + RadiusOffsetX[offset]);
                const int y = f.local_14 + RadiusOffsetY[offset];
                f.local_4 = x;
                f.local_8 = y;
                if (!on_map(x, y)) continue;
                const Map *tile = map_loc(x, y);
                if (f.local_40 != 2 &&
                    (((tile->climate & 0xe0u) < 0x60u) !=
                     (f.local_48 != 0))) continue;
                if (tile->region != f.local_34) continue;

                const int target = stack_fix(veh_at(x, y));
                f.local_1c = target;
                if (target < 0 || Vehs[target].faction_id == f.local_c)
                    continue;
                if (stack_check(target, 2, 0x0b, -1, -1) == 0) {
                    if (veh_who(x, y) >= 0 ||
                        probe(f.vehicle_id, -1, target, 0) == 0) continue;
                }

                const int target_faction = Vehs[f.local_1c].faction_id;
                const uint32_t relation =
                    PlayersData[f.local_c].diplo_treaties[target_faction];
                if ((relation & 1u) != 0 &&
                    (relation & 0x02000000u) != 0) continue;
                if ((relation & 0x30u) != 0 ||
                    whose_territory(f.local_c,x,y,NULL,0) == f.local_c) {
                    go_to(f.vehicle_id, 0x50, x, y);
                    goto label_57707f;
                }
            }

            f.local_28 = 0x1869f;
            f.local_24 = 0;
            for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_;
                 f.local_24 = ++base_id) {
                f.local_24 = base_id;
                const Base &base = Bases[base_id];
                f.local_4c = base.x;
                const Map *tile = map_loc(base.x, base.y);
                if (tile->region != f.local_34) continue;
                f.local_60 = f.local_18;
                const int distance =
                    (x_dist(f.local_60,base.x)+abs(f.local_14-base.y)) >> 1;
                int score = distance * 16;
                if (base.faction_id_current != f.local_c) {
                    const int has_base_tech =
                        has_tech(0x0f, base.faction_id_current);
                    if (has_base_tech) continue;
                    const uint32_t relation = PlayersData[f.local_c]
                        .diplo_treaties[base.faction_id_current];
                    if ((base.state & 0x40u) != 0) {
                        f.local_4c = base.faction_id_current;
                        if ((relation & 7u) != 0) {
                            if ((relation & 0x20u) == 0) continue;
                            if (PlayersData[f.local_c].region_total_bases[f.local_34] != 0 &&
                                PlayersData[f.local_4c].region_total_combat_vehs[f.local_34] >
                                PlayersData[f.local_c].region_total_combat_vehs[f.local_34] / 2 +
                                PlayersData[f.local_c].region_total_bases[f.local_34])
                                continue;
                        }
                        score *= 2;
                    }
                    f.local_30 = f.local_c *
                        static_cast<int>(sizeof(PlayerData) / sizeof(uint32_t));
                    f.local_4c = static_cast<int>(relation);
                    if ((relation & 0x10u) != 0) score /= 2;
                    const bool has_pact = (relation & 1u) != 0;
                    f.local_4c = has_pact;
                    if (has_pact) {
                        score *= 2;
                        if (static_cast<int>(PlayersData[base.faction_id_current].ranking) < 6 ||
                            (relation & 0x01000000u) != 0) continue;
                    }
                    if ((relation & 2u) != 0) score *= 2;
                    if (has_pact && (relation & 0x02000000u) != 0)
                        continue;
                    if ((relation & 7u) != 0 &&
                        (relation & 0x20u) == 0) {
                        if (PlayersData[f.local_c].tech_ranking >=
                            PlayersData[base.faction_id_current].tech_ranking)
                            continue;
                        if (!is_human(static_cast<uint32_t>(f.local_c)) &&
                            (PlayersData[f.local_c]
                                 .diplo_treaties[base.faction_id_current] &
                             0x01000000u) != 0) continue;
                    }
                } else {
                    base_find(base.x,base.y,-1,-1,f.local_c,-1);
                    const int stack = veh_at(base.x, base.y);
                    const int occupied = stack_check(stack, 2, 0x0b, -1, -1);
                    score += ((occupied + (score != 0)) * 64 +
                              BaseFindDist) * 64;
                }
                if (score < f.local_28) {
                    f.local_28 = score;
                    go_to(f.vehicle_id, 0x70, base.x, base.y);
                }
            }
            if (Vehs[f.vehicle_id].order == 0x18) {
                goto label_57707f;
            } else {
                kill(f.vehicle_id);
                return 1;
            }
        }
    }
    goto label_56e507;
}

label_56d924: {
    if (f.local_64 == 1) {
        const Base &base = Bases[f.local_5c];
        if (f.local_90 >= 4 ||
            (Bases[f.local_9c].state & 0x00200000u) == 0) {
            go_to(f.vehicle_id, 0x64, base.x, base.y);
            goto label_578f46;
        }
    }
    goto label_56d977;
}

label_56d977: {
    const int zone_step = f.local_40 + 1;
    const int limit = 2 * zone_step * zone_step * f.local_90;
    if (f.local_7c <= limit &&
        (Bases[f.local_9c].state & 0x00200000u) == 0) {
        const Base &base = Bases[f.local_9c];
        if (get_there(f.vehicle_id, base.x, base.y) != 0 &&
            f.local_9c == f.local_5c) {
            go_to(f.vehicle_id, 0x64, base.x, base.y);
            if (f.local_40 == 1) goto label_574a88;
            goto label_57707f;
        }
    }
    f.local_58 = f.local_9c * static_cast<int>(sizeof(Base));
    f.local_30 = f.local_c *
        static_cast<int>(sizeof(PlayerData) / sizeof(uint32_t));
    if (PlayersData[f.local_c]
        .diplo_treaties[Bases[f.local_9c].faction_id_current] & 1u) {
        const Base &base = Bases[f.local_9c];
        go_to(f.vehicle_id,0x64,base.x,base.y);
        if (f.local_40 == 1) goto label_574a88;
        goto label_57707f;
    }
    goto label_56da90;
}

label_56da90: {
    if (enemy_move_upstream_56da90_probe(f) ==
        ENEMY_MOVE_UPSTREAM_PROBE_JOIN_57708A) goto label_57708a;
    goto label_56db67;
}

label_56db67: {
    const Veh &acting = Vehs[f.vehicle_id];
    const VehPrototype &prototype = VehPrototypes[acting.proto_id];
    if (f.local_90 >= 4 && f.local_84 == 0 && f.local_7c != 0 &&
        acting.moves_expended == 0 &&
        Chassis[prototype.chassis_id].range == 0 &&
        veh_moves(f.vehicle_id) == Rules.move_rate_roads && f.local_bc == 0 &&
        (acting.state & 8u) == 0 &&
        speed_proto(acting.proto_id) > Rules.move_rate_roads &&
        acting.proto_id != 0x10 && acting.proto_id != 0x11 &&
        acting.proto_id != 0x12) {
        Vehs[f.vehicle_id].move_to_ai_type = 0x64;
        if (f.local_40 == 1) {
            f.local_8c = 1;
            goto label_574a88;
        }
        goto label_578b34;
    }
    goto label_56dc96;
}

label_56dc96: {
    const EnemyMoveUpstreamLowStackJoin low_stack =
        enemy_move_upstream_56dc96_low_stack(f);
    if (low_stack == ENEMY_MOVE_UPSTREAM_LOW_STACK_JOIN_578B26)
        goto label_578b26;
    if (enemy_move_upstream_56ddd7_stack_budget(f) ==
        ENEMY_MOVE_UPSTREAM_DDD7_JOIN_578B34) goto label_578b34;
    goto label_56deb8;
}

label_56dc52: {
    if (f.local_40 == 1) {
        f.local_8c = 1;
        goto label_574a88;
    }
    goto label_578b34;
}

label_56deb8:
    if (f.local_4c == 0) {
        const int owner = Bases[f.local_58 / static_cast<int>(sizeof(Base))]
            .faction_id_current;
        const int faction = f.local_30 /
            static_cast<int>(sizeof(PlayerData) / sizeof(uint32_t));
        const uint32_t relation =
            PlayersData[faction].diplo_treaties[owner];
        if ((relation & 2u) != 0 && (relation & 1u) == 0 &&
            reputation(owner, f.local_c) < 6) goto label_56df59;
    }
    goto label_56deef;

label_56deef: {
    const Veh &acting = Vehs[f.vehicle_id];
    const VehPrototype &design = VehPrototypes[acting.proto_id];
    if (acting.moves_expended != 0 || f.local_94 != 0 ||
        Chassis[design.chassis_id].triad != 0 ||
        (acting.state & 8u) != 0 ||
        (f.local_90 <= 1 && has_tech(0x16, f.local_c) == 0)) {
        goto label_56df59;
    }
    Vehs[f.vehicle_id].move_to_ai_type = 0x64;
    goto label_578b34;
}

label_56df59: {
    const Veh &acting = Vehs[f.vehicle_id];
    const int triad = f.local_40;
    if (f.local_90 >= 3 && triad != 0) {
        action_home(f.vehicle_id, 2);
        if (Vehs[f.vehicle_id].order == 0x18) {
            Vehs[f.vehicle_id].move_to_ai_type = 0x64;
            if (triad == 1) goto label_574a88;
            goto label_57707f;
        }
        goto label_56df99;
    }
label_56df99:
    if (f.local_5c >= 0 &&
        (((acting.state & 8u) != 0) || f.local_94 != 0) &&
        f.local_90 >= 4) {
        const Base &base = Bases[f.local_5c];
        if (get_there(f.vehicle_id, base.x, base.y) != 0) {
            go_to(f.vehicle_id, 0x64, base.x, base.y);
            if (triad == 1) goto label_574a88;
            goto label_57707f;
        }
    }
    goto label_56e011;
}

// Pinned enemy_move continuation 0x0056E507..0x0056ED86.
// Raw frame words are written only where the original stores them; several
// scratch words deliberately retain path-dependent values.
label_56e507:
if (f.local_40) goto label_56ed87;
if (!has_abil(Vehs[f.vehicle_id].proto_id, 0x10)) goto label_56e62d;
if (Vehs[f.vehicle_id].moves_expended != 0) goto label_56e62d;
if (!Weapon[VehPrototypes[Vehs[f.vehicle_id].proto_id].weapon_id].offense_rating)
    goto label_56e62d;
if (!stack_check(f.vehicle_id, 2, 2, -1, -1)) goto label_56e62d;
if (Vehs[f.vehicle_id].state & 0x20u) goto label_56e62d;
if ((static_cast<unsigned char>(f.local_68) &
     static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0)
    goto label_56e62d;
if (f.local_90 >= 4) goto label_56e62d;

if (f.local_7c) {
    if (map_loc(f.local_18, f.local_14)->bit & 0x40000u)
        goto label_56e6a9;
    if (whose_territory(f.local_c, f.local_18, f.local_14, NULL, 0) != f.local_c)
        goto label_56e62d;
    if (Vehs[f.vehicle_id].order == 0x18 || f.local_5c < 0)
        goto label_56e62d;
    go_to(f.vehicle_id, 0x49, Bases[f.local_5c].x, Bases[f.local_5c].y);
} else goto label_56e6a9;

label_56e62d:
if (f.local_64) goto label_56ed87;
if (f.local_88 == f.local_50) goto label_56ed87;
if (Bases[f.local_5c].unk_1) {
    f.local_8c = 1;
    f.local_b4 = 1;
    goto label_57708a;
}
if (!stack_check(f.vehicle_id, 2, 7, -1, -1)) goto label_56ed87;
if (f.local_88 == 9) goto label_56ed87;
if (f.local_50 != 1 || f.local_88 > 2) {
    f.local_8c = 1;
    f.local_b4 = 1;
    goto label_57708a;
}
goto label_56ef2f;

label_56e6a9:
f.local_38 = drop_range(f.local_c);
f.local_28 = 99999;
f.local_78 = -1;
f.local_24 = 0;
{
int enemy_base_index = 0;
if (GameDataStateStorage.fields_.base_current_count_ > 0) {
for (; enemy_base_index < GameDataStateStorage.fields_.base_current_count_; ++enemy_base_index) {
    const Base &candidate = Bases[enemy_base_index];
    const int owner = candidate.faction_id_current;
    if (owner == f.local_c) continue;
    const unsigned int treaty = PlayersData[f.local_c].diplo_treaties[owner];
    if (treaty & 1u) continue;
    if (!(treaty & 0xc10u)) continue;
    if (has_fac(0x1d, enemy_base_index, 0)) continue;
    if (has_tech(0x23, candidate.faction_id_current)) continue;
    if (veh_who(candidate.x, candidate.y) >= 0) continue;
    f.local_60 = f.local_18;
    f.local_58 = candidate.x;
    const int distance = (x_dist(candidate.x, f.local_18) +
                          abs(candidate.y - f.local_14)) >> 1;
    if (distance <= f.local_38) {
        f.local_24 = enemy_base_index;
        action_airdrop(f.vehicle_id, candidate.x, candidate.y, 3);
        return 1;
    }
    if (distance < f.local_28) {
        f.local_28 = distance;
        f.local_78 = enemy_base_index;
    }
}
f.local_24 = enemy_base_index;
}
}

if (f.local_78 >= 0) {
    f.local_24 = f.local_78;
    f.local_28 = 99999;
    for (f.local_54 = 0; f.local_54 < GameDataStateStorage.fields_.base_current_count_; ++f.local_54) {
        const Base &candidate = Bases[f.local_54];
        if (candidate.faction_id_current != f.local_c) continue;
        const Base &selected = Bases[f.local_78];
        f.local_60 = abs(candidate.y - selected.y);
        if (((x_dist(candidate.x, selected.x) + f.local_60) >> 1) > f.local_38)
            continue;
        if (stack_check(veh_at(candidate.x, candidate.y), 6, 0x10, -1, -1))
            continue;
        f.local_60 = candidate.x;
        const int current_distance_y = abs(candidate.y - f.local_14);
        f.local_60 = current_distance_y;
        if (((x_dist(candidate.x, f.local_18) + f.local_60) >> 1) <= f.local_38) {
            veh_skip(f.vehicle_id);
            action_airdrop(f.vehicle_id, candidate.x, candidate.y, 3);
            return 1;
        }
        if (map_loc(candidate.x, candidate.y)->region == f.local_34) {
            f.local_60 = f.local_18;
            const int distance = (x_dist(candidate.x, f.local_60) +
                                  abs(candidate.y - f.local_14)) >> 1;
            if (distance < f.local_28) {
                f.local_28 = distance;
                go_to(f.vehicle_id, 0x69, candidate.x, candidate.y);
            }
        }
    }
}
else goto label_56e9aa;

goto label_56e62d;

label_56e9aa:
Vehs[f.vehicle_id].order = 0;
if (PlayersData[f.local_c].proto_id_active[f.local_ac] <= 1)
    goto label_56e62d;
f.local_58 = -1;
f.local_60 = -1;
f.local_24 = 0;
if (GameDataStateStorage.fields_.base_current_count_ > 0) {
    f.base_cursor_48 = reinterpret_cast<const short *>(&Bases[0].faction_id_current);
    for (; f.local_24 < GameDataStateStorage.fields_.base_current_count_;
         ++f.local_24,
         f.base_cursor_48 = reinterpret_cast<const short *>(
             &Bases[f.local_24].faction_id_current)) {
    const Base &candidate = Bases[f.local_24];
    f.local_70 = candidate.faction_id_current;
    if (f.local_70 == f.local_c) continue;
    const unsigned int treaty = PlayersData[f.local_c].diplo_treaties[f.local_70];
    if (treaty & 1u) continue;
    if (!(treaty & 0xc10u)) continue;
    if (has_fac(0x1d, f.local_24, 0)) continue;
    if (has_tech(0x23, candidate.faction_id_current)) continue;

    f.local_20 = candidate.population_size;
    const unsigned char attitude =
        PlayersData[f.local_70].region_base_plan[f.local_34];
    if (attitude == 7) f.local_20 *= 3;
    if (attitude == 8) f.local_20 *= 2;
    if (attitude == 0) {
        const int sign = f.local_20 >> 31;
        f.local_20 = (f.local_20 - sign) >> 1;
    }

    f.local_1c = stack_fix(veh_at(candidate.x, candidate.y));
    if (stack_check(f.local_1c, 2, 4, -1, -1)) continue;
    f.local_30 = stack_check(f.local_1c, 0x10, -1, -1, -1);
    for (f.local_54 = 0; f.local_54 < 0x60; f.local_54 += 4) {
        const int radius_index = 1 + f.local_54 / 4;
        f.local_4 = xrange(candidate.x + RadiusOffsetX[radius_index]);
        f.local_8 = candidate.y + RadiusOffsetY[radius_index];
        if (!on_map(f.local_4, f.local_8)) continue;
        if ((map_loc(f.local_4, f.local_8)->climate & 0xe0) < 0x60) continue;
        if (((x_dist(f.local_4, f.local_18) + abs(f.local_8 - f.local_14)) >> 1) >
            f.local_38) continue;
        const int territory = anything_at(f.local_4, f.local_8);
        if (territory >= 0 && territory != f.local_c) continue;

        int score = f.local_20 +
                    static_cast<int>(rnd(8, const_cast<char *>("ENEMYSTUFF")));
        if (territory == f.local_c) {
            const int scaled = score * 3;
            score = (scaled + ((scaled >> 31) & 3)) >> 2;
        }
        if (f.local_54 <= 0x1f && f.local_30 >= 2) {
            const int sign = score >> 31;
            score = (score - sign) >> 1;
        }
        const unsigned int tile_bits = map_loc(f.local_4, f.local_8)->bit;
        if (tile_bits & 0x01608050u) score *= 2;
        if (tile_bits & 4u) {
            const int scaled = score * 4;
            score = scaled / 3;
        }

        f.local_2c = 0;
        for (; f.local_2c < 0x20; f.local_2c += 4) {
            const int direction = static_cast<int>(f.local_2c / 4);
            const int neighbor_x = xrange(f.local_4 + VehMoveDeltaX[direction]);
            const int neighbor_y = f.local_8 + VehMoveDeltaY[direction];
            if (!on_map(neighbor_x, neighbor_y)) continue;
            const int neighbor_owner = whose_territory(f.local_c, neighbor_x,
                                                       neighbor_y, NULL, 0);
            if (neighbor_owner == f.local_c) score = 0;
            f.local_1c = stack_fix(veh_at(neighbor_x, neighbor_y));
            if (f.local_1c < 0) continue;
            const int neighbor_vehicle = f.local_1c;
            if (Vehs[neighbor_vehicle].faction_id != f.local_c) {
                if (Vehs[neighbor_vehicle].faction_id == f.local_70 &&
                    VehPrototypes[Vehs[neighbor_vehicle].proto_id].plan <= 1) {
                    const int sign = score >> 31;
                    score = (score - sign) >> 1;
                }
            } else if (has_abil(Vehs[neighbor_vehicle].proto_id, 0x10)) {
                score += 0x20;
            }
        }
        if (score > f.local_28) {
            f.local_28 = score;
            f.local_60 = f.local_4;
            f.local_58 = f.local_8;
        }
    }
    }
}
if (on_map(f.local_60, f.local_58)) {
    action_airdrop(f.vehicle_id, f.local_60, f.local_58, 3);
    return 1;
}
goto label_56e62d;

// Native56ED87..570001. Direct body joins; no synthetic exit statuses.
label_56ed87:
    if (!((f.local_50 == 6 && (Vehs[f.vehicle_id].state & 0x200u) &&
            Vehs[f.vehicle_id].order_auto_type == 11) || f.local_40 == 2))
        goto label_56ef2f;
    if (planet_buster(f.vehicle_id) && f.local_40 == 2) {
        f.local_80 = enemy_planet_buster(f.vehicle_id);
        f.local_18 = Vehs[f.vehicle_id].x;
        f.local_14 = Vehs[f.vehicle_id].y;
        goto label_578b29;
    }
    {
        const int result = air_power(f.vehicle_id,f.local_64,f.local_bc,
                                    &f.local_18,&f.local_14);
        if (result == -5) return 1;
        if (result == -1) {
            if (Vehs[f.vehicle_id].x == Vehs[f.vehicle_id].waypoint_x[0] &&
                Vehs[f.vehicle_id].y == Vehs[f.vehicle_id].waypoint_y[0])
                f.local_8c = 1;
            else {
                f.local_70 = anything_at(Vehs[f.vehicle_id].waypoint_x[0],
                                         Vehs[f.vehicle_id].waypoint_y[0]);
                if (f.local_70 >= 0 && f.local_70 == f.local_c) goto label_578f46;
                // Native indexes this row even when anything_at returns -1.
                if (PlayersData[f.local_c].diplo_treaties[f.local_70] & 1u)
                    goto label_578f46;
            }
            goto label_57707f;
        }
        if (result == -2) {
            if (Vehs[f.vehicle_id].x != Vehs[f.vehicle_id].waypoint_x[0] ||
                Vehs[f.vehicle_id].y != Vehs[f.vehicle_id].waypoint_y[0])
                goto label_578f46;
            f.local_8c = 1;
            goto label_57707f;
        }
        if (result == -4) { f.local_8c = 1; goto label_57707f; }
        if (result == -3 && (Vehs[f.vehicle_id].state & 0x200u) &&
            Vehs[f.vehicle_id].order_auto_type == 10) {
            Vehs[f.vehicle_id].order = 0;
            Vehs[f.vehicle_id].state &= ~0x200u;
            return 0;
        }
        if (result >= 0) f.local_80 = result;
        goto label_578b29;
    }
label_56ef2f:
    if (f.local_b0) goto label_57460a;
    if (f.local_50 == 6) {
        if (Vehs[f.vehicle_id].order == 24)
            f.local_28 = (abs(f.local_14-Vehs[f.vehicle_id].waypoint_y[0])+
                         x_dist(f.local_18,Vehs[f.vehicle_id].waypoint_x[0])) >> 1;
        else f.local_28 = 9999;
        f.local_2c = 0;
        // The speed/range bound is evaluated again after every candidate.
        while (static_cast<int>(f.local_2c) <
               RadiusRange[range(veh_moves(f.vehicle_id)/Rules.move_rate_roads,1,8)]) {
            f.local_4 = xrange(f.local_18+RadiusOffsetX[f.local_2c]);
            f.local_8 = f.local_14+RadiusOffsetY[f.local_2c];
            if (on_map(f.local_4,f.local_8) &&
                (map_loc(f.local_4,f.local_8)->climate & 0xe0) < 0x60) {
                bool candidate_region;
                if (f.local_64)
                    candidate_region = map_loc(f.local_4,f.local_8)->region == f.local_34;
                else candidate_region = base_on_sea(f.local_5c,
                    map_loc(f.local_4,f.local_8)->region) != 0;
                if (candidate_region && base_who(f.local_4,f.local_8) < 0) {
                    f.local_1c = stack_fix(veh_at(f.local_4,f.local_8));
                    while (f.local_1c >= 0 &&
                           Chassis[VehPrototypes[Vehs[f.local_1c].proto_id].chassis_id].triad != 1)
                        f.local_1c = Vehs[f.local_1c].next_veh_id_stack;
                    if (f.local_1c >= 0) {
                        const int candidate_distance =
                            (abs(f.local_14-f.local_8)+x_dist(f.local_18,f.local_4)) >> 1;
                        f.local_38 = candidate_distance;
                        const int owner = Vehs[f.local_1c].faction_id;
                        bool eligible;
                        if (owner != f.local_c)
                            eligible = (PlayersData[f.local_c].diplo_treaties[owner] & 0x10u) != 0;
                        else {
                            eligible = false;
                            if (VehPrototypes[Vehs[f.local_1c].proto_id].plan == 7 &&
                                !veh_moves(f.local_1c)) {
                                const int same_tile = Vehs[f.local_1c].x == f.local_18 &&
                                                      Vehs[f.local_1c].y == f.local_14;
                                eligible = stack_check(f.local_1c,2,6,-1,-1) <= same_tile;
                            }
                        }
                        if (eligible && candidate_distance < f.local_28) {
                            f.local_28 = candidate_distance;
                            go_to(f.vehicle_id,'n',f.local_4,f.local_8);
                        }
                    }
                }
            }
            ++f.local_2c;
        }
        f.local_1c = 0;
        while (f.local_1c < GameDataStateStorage.fields_.vehicle_current_count_) {
            const int owner = Vehs[f.local_1c].faction_id;
            if (owner != f.local_c &&
                (PlayersData[f.local_c].diplo_treaties[owner] & 0xc30u) &&
                (static_cast<unsigned char>(f.local_68) & Vehs[f.local_1c].visibility) &&
                (map_loc(Vehs[f.local_1c].x,Vehs[f.local_1c].y)->climate & 0xe0) < 0x60 &&
                base_who(Vehs[f.local_1c].x,Vehs[f.local_1c].y) < 0) {
                int candidate_distance = (abs(f.local_14-Vehs[f.local_1c].y)+
                    x_dist(f.local_18,Vehs[f.local_1c].x)) >> 1;
                f.local_38 = candidate_distance;
                if (VehPrototypes[Vehs[f.local_1c].proto_id].plan != 7) {
                    candidate_distance *= 2;
                    f.local_38 = candidate_distance;
                }
                if (stack_check(f.local_1c,1,-1,-1,-1) >= 2) {
                    candidate_distance /= 2;
                    f.local_38 = candidate_distance;
                }
                if (candidate_distance < f.local_28) {
                    add_goal(f.local_c,6,2,Vehs[f.local_1c].x,Vehs[f.local_1c].y,-1);
                    go_to(f.vehicle_id,'N',Vehs[f.local_1c].x,Vehs[f.local_1c].y);
                    // Native does not update local_28 in this second pass.
                }
            }
            ++f.local_1c;
        }
    }
    if (!can_arty(Vehs[f.vehicle_id].proto_id,1) ||
        (f.local_40 != 1 && (map_loc(f.local_18,f.local_14)->climate & 0xe0) < 0x60 &&
         f.local_7c) || f.local_84) goto label_56f8ad;
    f.local_28 = 0;
    f.local_78 = -1;
    f.local_58 = f.local_64 ? 25 : 81;
    for (f.local_2c = 0; static_cast<int>(f.local_2c) < f.local_58; ++f.local_2c) {
        f.local_4 = xrange(f.local_18+RadiusOffsetX[f.local_2c]);
        f.local_8 = f.local_14+RadiusOffsetY[f.local_2c];
        if (!on_map(f.local_4,f.local_8)) continue;
        if (anything_at(f.local_4,f.local_8) >= 0) {
            f.local_1c = stack_fix(veh_at(f.local_4,f.local_8));
            if (f.local_1c < 0) continue;
            const int owner = Vehs[f.local_1c].faction_id;
            if (owner == f.local_c || !(PlayersData[f.local_c].diplo_treaties[owner] & 0x10u))
                continue;
            const int combat_count = stack_check(f.local_1c,2,6,-1,-1);
            int score = (combat_count+stack_check(f.local_1c,6,0x8000,-1,-1))*8;
            if (f.local_40 == 1) {
                if ((map_loc(f.local_4,f.local_8)->climate & 0xe0) < 0x60 &&
                    base_who(f.local_4,f.local_8) >= 0) goto label_56f83a;
                if ((map_loc(f.local_4,f.local_8)->climate & 0xe0) < 0x60) {
                    f.local_60 = f.local_4;
                    const int candidate_distance =
                        (abs(f.local_14-f.local_8)+x_dist(f.local_18,f.local_60)) >> 1;
                    if (candidate_distance <= veh_moves(f.vehicle_id)/Rules.move_rate_roads)
                        goto label_56f83a;
                }
                if (Vehs[f.vehicle_id].order == 24) continue;
            }
            if (!f.local_94 || f.local_2c < 9)
                score += stack_check(f.local_1c,4,-1,-1,-1);
            f.local_60 = f.local_18;
            const int distance = (abs(f.local_14-f.local_8)+x_dist(f.local_60,f.local_4)) >> 1;
            if (distance > Rules.artillery_max_rng && score > 1) score = 1;
            if (score > f.local_28) { f.local_28 = score; f.local_78 = f.local_2c; }
        } else if ((map_loc(f.local_4,f.local_8)->climate & 0xe0) >= 0x60) {
            const int owner = whose_territory(f.local_c,f.local_4,f.local_8,NULL,0);
            if (owner < 0 || owner == f.local_c ||
                !(PlayersData[f.local_c].diplo_treaties[owner] & 0x10u)) continue;
            const Map *tile = map_loc(f.local_4,f.local_8);
            const unsigned int bits = tile->bit;
            const int region = tile->region;
            if (PlayersData[f.local_c].region_total_bases[region] &&
                PlayersData[f.local_c].region_base_plan[region] != 2 && !f.local_40 &&
                !(bits & 0x80000000u)) continue;
            if (!(bits & 0x80001000u) ||
                (!(bits & 0x80000000u) && f.local_40 == 1 && Vehs[f.vehicle_id].order == 24) ||
                !(bits & 0x81208854u)) continue;
            const int score = (bits & 0x80000000u) ? 32 : 2;
            if (score > f.local_28) { f.local_28 = score; f.local_78 = f.local_2c; }
        }
    }
    if (f.local_78 >= 0) {
        f.local_4 = xrange(f.local_18+RadiusOffsetX[f.local_78]);
        f.local_8 = f.local_14+RadiusOffsetY[f.local_78];
        f.local_60 = f.local_18;
        const int target_distance = (abs(f.local_14-f.local_8)+x_dist(f.local_60,f.local_4)) >> 1;
        if (target_distance <= Rules.artillery_max_rng) {
            if (anything_at(f.local_4,f.local_8) < 0) {
                if (f.local_a0) {
                    log_say("Auto sending DESTROY2 message",f.vehicle_id,f.local_4,f.local_8);
                    message_data(0x2413,f.vehicle_id,f.local_4,f.local_8,0,0);
                    NetDaemonNet.await_exec(1);
                } else {
                    log_say("Enemy doing ARTY destroy 1...",f.vehicle_id,f.local_4,f.local_8);
                    action_destroy(f.vehicle_id,0,f.local_4,f.local_8);
                }
                return 1;
            }
            if (f.local_78 >= 9 || (f.local_40 == 1 &&
                (map_loc(f.local_4,f.local_8)->climate & 0xe0) >= 0x60)) {
                if (f.local_a0) {
                    log_say("Auto sending ARTY message",f.vehicle_id,f.local_c,0);
                    message_veh(0x2402,f.vehicle_id,f.local_78,0);
                    NetDaemonNet.await_exec(1);
                } else {
                    // PUSH0 at56FB28 remains pending across the offline branch.
                    log_say("Enemy doing ARTY...",f.vehicle_id,f.local_78,0);
                    battle_fight(f.vehicle_id,f.local_78,1,1,0);
                }
                return 1;
            }
        }
        if (!f.local_64 && f.local_78 >= 25 && (!f.local_a8 || f.local_88) &&
            anything_at(f.local_4,f.local_8) >= 0) goto label_578b34;
        goto label_56f8ad;
    }
    goto label_56f841;
label_56f83a:
    f.local_78 = -1;
label_56f841:
    if (!f.local_64 && !f.local_a8 && f.local_88 &&
        stack_check(f.vehicle_id,15,-1,-1,-1) <= 1 && f.local_40 != 1 &&
        ((map_loc(f.local_18,f.local_14)->bit & 0x4000u) || f.local_88 != 2))
        goto label_578b34;
label_56f8ad:
    del_site(f.local_c,3,f.local_18,f.local_14,0);
    if ((f.local_50 != 8 && f.local_40) || f.local_84) goto label_570002;
    f.local_48 = f.local_40 == 1;
    if (f.local_74 != f.local_48) goto label_570002;
    if (f.local_50 == 8) del_site(f.local_c,8,f.local_18,f.local_14,1);
    f.local_24 = 0;
    if (f.local_50 == 8 && Vehs[f.vehicle_id].order == 24 &&
        vector_dist(x_dist(f.local_18,Vehs[f.vehicle_id].waypoint_x[0]),
                    abs(f.local_14-Vehs[f.vehicle_id].waypoint_y[0])) > 2)
        f.local_24 = site_at(Vehs[f.vehicle_id].waypoint_x[0],Vehs[f.vehicle_id].waypoint_y[0],0)-1;
    for (f.local_2c = 0; f.local_2c < 25; ++f.local_2c) {
        f.local_4 = xrange(f.local_18+RadiusOffsetX[f.local_2c]);
        f.local_8 = f.local_14+RadiusOffsetY[f.local_2c];
        if (!on_map(f.local_4,f.local_8)) continue;
        const Map *tile = map_loc(f.local_4,f.local_8);
        const int climate = tile->climate & 0xe0;
        if ((climate < 0x60) != f.local_48 ||
            ((tile->bit & 0x20u) && climate >= 0x40) ||
            ((tile->val3 & 0xc0) > 0x40 && climate >= 0x60) ||
            tile->region != f.local_34 || (tile->bit & 0x2000u) ||
            base_territory(f.local_c,f.local_4,f.local_8) >= 0) continue;
        if (!f.local_40) {
            if (!(PlayersData[f.local_c].unk_77[f.local_34] & 0x80u) &&
                !is_coast(f.local_4,f.local_8,0) &&
                static_cast<int>(Continents[f.local_34].open_terrain-
                                 PlayersData[f.local_c].unk_81[f.local_34]) < 20) continue;
        } else if (f.local_40 == 1) {
            bool hostile_neighborhood = false;
            f.local_30 = 0;
            while (f.local_30 < 0x144) {
                const int neighbor_x = xrange(f.local_4+RadiusOffsetX[f.local_30/4]);
                const int neighbor_y = f.local_8+RadiusOffsetY[f.local_30/4];
                if (on_map(neighbor_x,neighbor_y)) {
                    const int base = base_at(neighbor_x,neighbor_y);
                    if (base >= 0 && Bases[base].faction_id_current != f.local_c &&
                        (PlayersData[f.local_c].diplo_treaties[Bases[base].faction_id_current] & 0x10u) &&
                        (is_port(base,0) || f.local_2c < 21)) {
                        hostile_neighborhood = true; break;
                    }
                    f.local_1c = stack_fix(veh_at(neighbor_x,neighbor_y));
                    if (f.local_1c >= 0 && Vehs[f.local_1c].faction_id != f.local_c &&
                        (PlayersData[f.local_c].diplo_treaties[Vehs[f.local_1c].faction_id] & 0x10u) &&
                        stack_check(f.local_1c,2,6,-1,-1)) {
                        hostile_neighborhood = true; break;
                    }
                }
                f.local_30 += 4;
            }
            if (hostile_neighborhood) continue;
        }
        const int score = site_at(f.local_4,f.local_8,0);
        if (score <= f.local_24) continue;
        base_find(f.local_4,f.local_8);
        if (score < (8-BaseFindDist)*2 &&
            PlayersData[f.local_c].region_total_bases[f.local_34] &&
            PlayersData[f.local_c].current_num_bases >= 2) {
            int threshold = f.local_88 != 8 ? 4 : 1;
            if (threshold < PlayersData[f.local_c].unk_82[f.local_34])
                threshold = PlayersData[f.local_c].unk_82[f.local_34];
            if (score < threshold && (f.local_50 != 8 || Vehs[f.vehicle_id].order == 24 ||
                static_cast<int>(Continents[f.local_34].open_terrain) >=
                (PlayersData[f.local_c].region_total_bases[f.local_34]+1)*6))
                continue;
        }
        f.local_24 = score;
        f.local_80 = f.local_2c;
    }
    if (!f.local_24) goto label_570002;
    if (!f.local_80 && f.local_50 == 8) {
        ++Vehs[f.vehicle_id].terraforming_turns;
        f.local_24 += Vehs[f.vehicle_id].terraforming_turns;
    }
    if (!f.local_24 || f.local_80 < 0 ||
        (f.local_bc && !(PlayersData[f.local_c].diplo_treaties[f.local_bc-1] & 7u)))
        goto label_570002;
    if (f.local_50 == 8) {
        if (!f.local_80) {
            if (f.local_a0) {
                log_say("Auto sending BUILD BASE message",f.vehicle_id,f.local_c,0);
                message_base(0x2408,f.vehicle_id,EnemyBuildBaseMessageText,0);
                NetDaemonNet.await_exec(1);
                return 1;
            }
            action_build(f.vehicle_id,NULL);
            if (PlayersData[f.local_c].current_num_bases == 1) {
                for (int vehicle = 0; vehicle < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle)
                    if (Vehs[vehicle].faction_id == f.local_c &&
                        VehPrototypes[Vehs[vehicle].proto_id].plan == 9)
                        go_to(vehicle,'C',f.local_18,f.local_14);
            }
            return 1;
        }
        Vehs[f.vehicle_id].move_to_ai_type = 'b';
        if (f.local_80 >= 9) {
            const int target_y = f.local_14+RadiusOffsetY[f.local_80];
            const int target_x = xrange(f.local_18+RadiusOffsetX[f.local_80]);
            go_to(f.vehicle_id,'b',target_x,target_y);
            goto label_57000c;
        }
        --f.local_80;
        goto label_578b26;
    }
    if (f.local_80 < 9) {
        f.local_4 = xrange(f.local_18+RadiusOffsetX[f.local_80]);
        f.local_8 = f.local_14+RadiusOffsetY[f.local_80];
        f.local_20 = f.local_24 >= 6 ? 2 : 1;
        if (f.local_7c >= 0) {
            f.local_38 = (abs(f.local_8-Bases[f.local_9c].y)+
                          x_dist(f.local_4,Bases[f.local_9c].x)) >> 1;
            if (f.local_38 <= 4) f.local_20 = 1;
            if (f.local_38 >= 6 && f.local_38 < 12 && f.local_24 >= 10) f.local_20 = 3;
        }
        del_site(f.local_c,8,f.local_18,f.local_14,2);
        add_site(f.local_c,8,f.local_20,f.local_4,f.local_8);
    }
    goto label_570002;

// Complete native570002..571384, directly included inside enemy_move.
// Frame f is the one actual EnemyMoveFullFrame. No entry scratch defaults.
label_570002:
    if (f.local_50 != 8) goto label_5702ed;
label_57000c:
    if (f.local_40 == 1 && Vehs[f.vehicle_id].order != 24 && f.local_74) {
        f.local_28 = 0;
        f.local_8 = 0;
        while (f.local_8 < MapLatitudeBounds) {
            f.local_4 = f.local_8 & 1;
            while (f.local_4 < MapLongitudeBounds) {
                if (on_map(f.local_4,f.local_8) &&
                    map_loc(f.local_4,f.local_8)->region == f.local_34 &&
                    anything_at(f.local_4,f.local_8) < 0 &&
                    !((map_loc(f.local_4,f.local_8)->bit & 0x20u) &&
                      (map_loc(f.local_4,f.local_8)->climate & 0xe0) >= 0x40)) {
                    const int rating = site_at(f.local_4,f.local_8,0);
                    const int dy = abs(f.local_14-f.local_8);
                    f.local_20 = (int)((unsigned int)rating * 10000u) /
                        (vector_dist(x_dist(f.local_18,f.local_4),dy)+1);
                    f.local_24 = base_find(f.local_4,f.local_8);
                    if (f.local_24 >= 0 && Bases[f.local_24].faction_id_current != f.local_c &&
                        (PlayersData[f.local_c].diplo_treaties[Bases[f.local_24].faction_id_current] & 0x10u))
                        f.local_20 /= 2;
                    if (f.local_20 >= f.local_28) {
                        f.local_48 = 0;
                        bool rejected = false;
                        do {
                            const int offset = f.local_48/4;
                            const int xx = xrange(f.local_4+RadiusOffsetX[offset]);
                            const int yy = f.local_8+RadiusOffsetY[offset];
                            if (on_map(xx,yy) && (map_loc(xx,yy)->climate & 0xe0) < 0x60) {
                                f.local_24 = base_at(xx,yy);
                                if (f.local_24 >= 0) {
                                    const int owner = Bases[f.local_24].faction_id_current;
                                    // Native reverse-row query is intentional.
                                    if (owner != f.local_c &&
                                        (PlayersData[owner].diplo_treaties[f.local_c] & 0x10u)) {
                                        rejected = true; break;
                                    }
                                }
                                f.local_1c = stack_fix(veh_at(xx,yy));
                                if (f.local_1c >= 0) {
                                    const int owner = Vehs[f.local_1c].faction_id;
                                    if (owner != f.local_c &&
                                        (PlayersData[owner].diplo_treaties[f.local_c] & 0x10u)) {
                                        rejected = true; break;
                                    }
                                }
                            }
                            f.local_48 += 4;
                        } while (f.local_48 < 0x144);
                        if (!rejected) {
                            f.local_28 = f.local_20;
                            go_to(f.vehicle_id,99,f.local_4,f.local_8);
                        }
                    }
                }
                f.local_4 += 2;
            }
            ++f.local_8;
        }
    }
    goto label_5702f7;
label_5702ed:
    if (f.local_50 != 3) goto label_570c89;
label_5702f7:
    if ((Vehs[f.vehicle_id].order != 24 || !f.local_7c) && !f.local_40 && !f.local_74) {
        f.local_30 = 0;
        f.local_48 = 0;
        f.local_60 = 25;
        int site_index = 0;
        do {
            const Goal &site = PlayersData[f.local_c].sites[site_index];
            if (site.type == f.local_50 && site.priority >= 1 &&
                map_loc(site.x,site.y)->region == f.local_34) {
                ++f.local_30;
                if (f.local_50 == 3) {
                    if (goody_at(site.x,site.y)) ++f.local_48;
                } else {
                    int threshold = PlayersData[f.local_c].unk_82[f.local_34];
                    if (threshold > 8) threshold = 8;
                    if (f.local_88 != 8 && threshold < 4) threshold = 4;
                    if (site_at(site.x,site.y,0) >= threshold) ++f.local_48;
                }
            }
            ++site_index;
            --f.local_60;
        } while (f.local_60);
        f.local_2c = Vehs[f.vehicle_id].order == 24;
        if (!f.local_30 || !((GameDataStateStorage.fields_.turn_current_num_+f.local_c) & (f.local_48 ? 15 : 3)) || !f.local_7c) {
            f.local_28 = 0;
            f.local_8 = 0;
            while (f.local_8 < MapLatitudeBounds) {
                f.local_4 = f.local_8 & 1;
                while (f.local_4 < MapLongitudeBounds) {
                    if (!on_map(f.local_4,f.local_8) ||
                        map_loc(f.local_4,f.local_8)->region != f.local_34)
                        goto late_policy_next_tile;
                    f.local_54 = whose_territory(f.local_c,f.local_4,f.local_8,NULL,0);
                    if (f.local_88 != 8 && f.local_54 != f.local_c && f.local_54 >= 0)
                        goto late_policy_next_tile;
                    if (base_territory(f.local_c,f.local_4,f.local_8) >= 0)
                        goto late_policy_next_tile;
                    if ((!f.local_30 || !((GameDataStateStorage.fields_.turn_current_num_+f.local_c) & 15)) &&
                        (is_known(f.local_4,f.local_8,f.local_c) ||
                         (GameDataStateStorage.fields_.diff_level_current_ >= 4 &&
                          !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)))) &&
                        goody_at(f.local_4,f.local_8)) {
                        const int near_base = base_find(f.local_4,f.local_8);
                        if ((near_base >= 0 && Bases[near_base].faction_id_current == f.local_c) ||
                            f.local_54 == f.local_c)
                            add_site(f.local_c,3,(BaseFindDist < 6)+1,f.local_4,f.local_8);
                    }
                    {
                        const Map &tile = *map_loc(f.local_4,f.local_8);
                        if ((tile.climate & 0xe0) < 0x60 ||
                            ((tile.bit & 0x20u) && (tile.climate & 0xe0) >= 0x40) ||
                            (tile.bit & 0x2000u) || (tile.val3 & 0xc0) > 0x40)
                            goto late_policy_next_tile;
                    }
                    if ((f.local_54 == f.local_c || f.local_54 < 0) &&
                        !is_known(f.local_4,f.local_8,f.local_c)) {
                        f.local_20 = 0;
                        f.local_48 = 0;
                        bool existing_site = false;
                        do {
                            const int offset = f.local_48/4;
                            const int xx = xrange(f.local_4+RadiusOffsetX[offset]);
                            const int yy = f.local_8+RadiusOffsetY[offset];
                            if (on_map(xx,yy) && map_loc(xx,yy)->region == f.local_34) {
                                if (at_site(f.local_c,3,xx,yy)) { existing_site = true; break; }
                                if (is_known(xx,yy,f.local_c)) {
                                    if ((GameDataStateStorage.fields_.diff_level_current_ >= 4 &&
                                        !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_))) ||
                                        f.local_48 < 0x24) f.local_20 |= 0x20;
                                } else {
                                    ++f.local_20;
                                    if (goody_at(xx,yy)) ++f.local_20;
                                }
                            }
                            f.local_48 += 4;
                        } while (f.local_48 < 0x54);
                        if (!existing_site && f.local_20 >= 0x21) {
                            const int priority = f.local_20 >= 0x28 ? 3 : (f.local_20 >= 0x23)+1;
                            add_site(f.local_c,3,priority,f.local_4,f.local_8);
                        }
                    }
                    if (f.local_88 != 8 && f.local_54 != f.local_c)
                        goto late_policy_next_tile;
                    f.local_48 = f.local_c*0x20cc+f.local_34*2;
                    if (!(PlayersData[f.local_c].unk_77[f.local_34] & 0x80u) &&
                        !is_coast(f.local_4,f.local_8,0) &&
                        (int)(Continents[f.local_34].open_terrain-
                            PlayersData[f.local_c].unk_81[f.local_34]) < 20)
                        goto late_policy_next_tile;
                    f.local_20 = site_at(f.local_4,f.local_8,0);
                    if (!f.local_20) goto late_policy_next_tile;
                    if (f.local_20 < (8-BaseFindDist)*2-
                        (PlayersData[f.local_c].region_total_bases[f.local_34] < 2 ? 2 : 0) &&
                        f.local_20 < PlayersData[f.local_c].unk_82[f.local_34] &&
                        PlayersData[f.local_c].region_total_bases[f.local_34] &&
                        PlayersData[f.local_c].current_num_bases >= 2)
                        goto late_policy_next_tile;
                    f.local_58 = 1;
                    f.local_60 = 0;
                    do {
                        const int offset = f.local_60/4+1;
                        const int xx = xrange(f.local_4+RadiusOffsetX[offset]);
                        const int yy = f.local_8+RadiusOffsetY[offset];
                        if (on_map(xx,yy)) {
                            const Map &tile = *map_loc(xx,yy);
                            if (tile.region == f.local_34 &&
                                (!(tile.bit & 0x20u) || (tile.climate & 0xe0) < 0x40) &&
                                !(tile.bit & 0x2000u) && (tile.val3 & 0xc0) <= 0x40 &&
                                base_territory(f.local_c,xx,yy) < 0 &&
                                !(map_loc(xx,yy)->bit & 0x1000u)) {
                                const int rating = site_at(xx,yy,0);
                                if (rating >= f.local_20 &&
                                    (rating != f.local_20 || at_site(f.local_c,8,xx,yy)) &&
                                    ((PlayersData[f.local_c].unk_77[f.local_34] & 0x80u) ||
                                     is_coast(xx,yy,0) ||
                                     (int)(Continents[f.local_34].open_terrain-
                                         PlayersData[f.local_c].unk_81[f.local_34]) >= 20) &&
                                    (f.local_88 == 8 ||
                                     whose_territory(f.local_c,xx,yy,NULL,0) == f.local_c)) {
                                    f.local_58 = 0;
                                    break;
                                }
                            }
                        }
                        f.local_60 += 4;
                    } while (f.local_60 < 0x60);
                    {
                        const int dy = abs(f.local_14-f.local_8);
                        const int distance = (dy+x_dist(f.local_18,f.local_4)) >> 1;
                        const int score = (int)((unsigned int)f.local_20 << 16)/(distance+1);
                        if (f.local_58 && (score > f.local_28 ||
                            is_known(f.local_4,f.local_8,f.local_c) ||
                            (GameDataStateStorage.fields_.diff_level_current_ >= 4 &&
                             !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_))))) {
                            const int territory = whose_territory(f.local_c,f.local_4,f.local_8,NULL,0);
                            if (Paths.find(f.local_4,f.local_8,f.local_18,f.local_14,
                                territory,f.local_c,0x201,-1) >= 0) {
                                if (is_known(f.local_4,f.local_8,f.local_c) ||
                                    (GameDataStateStorage.fields_.diff_level_current_ >= 4 &&
                                     !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)))) {
                                    del_site(f.local_c,8,f.local_4,f.local_8,2);
                                    add_site(f.local_c,8,2,f.local_4,f.local_8);
                                    ++f.local_30;
                                }
                                if (score > f.local_28) {
                                    f.local_28 = score;
                                    if (!f.local_2c && f.local_50 == 8)
                                        go_to(f.vehicle_id,99,f.local_4,f.local_8);
                                }
                            }
                        }
                    }
late_policy_next_tile:
                    f.local_4 += 2;
                }
                ++f.local_8;
            }
            if (f.local_50 != 3 && !f.local_2c) {
                int site_count = f.local_30;
                if (Vehs[f.vehicle_id].order == 24) {
                    if (!site_count) site_count = 1;
                    add_site(f.local_c,3,2,Vehs[f.vehicle_id].waypoint_x[0],Vehs[f.vehicle_id].waypoint_y[0]);
                    if (is_known(Vehs[f.vehicle_id].waypoint_x[0],Vehs[f.vehicle_id].waypoint_y[0],f.local_c) ||
                        (GameDataStateStorage.fields_.diff_level_current_ >= 4 &&
                         !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)))) {
                        // Native bug: del_site uses terminal map-scan coordinates.
                        del_site(f.local_c,8,f.local_4,f.local_8,2);
                        add_site(f.local_c,8,3,Vehs[f.vehicle_id].waypoint_x[0],Vehs[f.vehicle_id].waypoint_y[0]);
                    } else Vehs[f.vehicle_id].order = 0;
                }
                if (!site_count) {
                    PlayersData[f.local_c].unk_80[f.local_34] =
                        (unsigned short)(PlayersData[f.local_c].unk_80[f.local_34]-10);
                    PlayersData[f.local_c].unk_77[f.local_34] |= 9u;
                    if (f.local_88 == 8) {
                        f.local_88 = 7;
                        PlayersData[f.local_c].region_base_plan[f.local_34] = 7;
                    }
                }
            }
        }
        if (f.local_50 == 8 && Vehs[f.vehicle_id].order != 24) {
            f.local_70 = whose_territory(f.local_c,f.local_18,f.local_14,NULL,0);
            if (f.local_70 >= 0 && f.local_70 != f.local_c &&
                (PlayersData[f.local_c].diplo_treaties[f.local_70] & 3u)) {
                f.local_24 = base_find(f.local_18,f.local_14,f.local_c,f.local_34,-1,-1);
                if (f.local_24 >= 0) {
                    go_to(f.vehicle_id,0x6b,Bases[f.local_24].x,Bases[f.local_24].y);
                    goto label_57717f;
                }
            }
        }
    }
label_570c89:
    if (speed(f.vehicle_id,0) <= Rules.move_rate_roads &&
        (map_loc(f.local_18,f.local_14)->bit & 0x800000u) &&
        Chassis[VehPrototypes[Vehs[f.vehicle_id].proto_id].chassis_id].triad != 2 &&
        f.local_9c >= 0) {
        f.local_4 = xrange(f.local_18+6);
        f.local_24 = base_find(f.local_4,f.local_14);
        bool destroy;
        if (f.local_24 >= 0 && BaseFindDist <= 2)
            destroy = (PlayersData[f.local_c].diplo_treaties[Bases[f.local_24].faction_id_current] & 0x10u) != 0;
        else destroy = Bases[f.local_9c].faction_id_current == f.local_c && f.local_7c <= 2;
        if (destroy) {
            if (!f.local_a0) { action_destroy(f.vehicle_id,0x800000,-1,-1); return 0; }
            log_say("Auto sending DESTROY message",f.vehicle_id,0x800000,0);
            message_data(0x2406,f.vehicle_id,0x800000,0,0,0);
            NetDaemonNet.await_exec(1);
            return 1;
        }
    }
    goto label_570df9;
label_570df9:
    if (speed(f.vehicle_id,0) <= Rules.move_rate_roads &&
        (map_loc(f.local_18,f.local_14)->bit & 0x400000u) &&
        Chassis[VehPrototypes[Vehs[f.vehicle_id].proto_id].chassis_id].triad != 2 &&
        f.local_9c >= 0 && f.local_7c <= 2 &&
        Bases[f.local_9c].faction_id_current != f.local_c && f.local_64 > 3 &&
        (PlayersData[f.local_c].diplo_treaties[Bases[f.local_9c].faction_id_current] & 0x10u)) {
        if (!f.local_a0) { action_destroy(f.vehicle_id,0x400000,-1,-1); return 0; }
        log_say("Auto sending DESTROY message",f.vehicle_id,0x400000,0);
        message_data(0x2406,f.vehicle_id,0x400000,0,0,0);
        NetDaemonNet.await_exec(1);
        return 1;
    }
    if (speed(f.vehicle_id,0) <= Rules.move_rate_roads && f.local_9c >= 0) {
        const int owner = Bases[f.local_9c].faction_id_current;
        if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1u << (owner & 31))) &&
            (PlayersData[f.local_c].diplo_treaties[owner] & 0x10u) && f.local_40 != 2 &&
            (((map_loc(f.local_18,f.local_14)->climate & 0xe0) < 0x60) == (f.local_40 == 1)) &&
            Weapon[VehPrototypes[Vehs[f.vehicle_id].proto_id].weapon_id].offense_rating && !f.local_84 &&
            whose_territory(f.local_c,f.local_18,f.local_14,NULL,0) == owner &&
            ((map_loc(f.local_18,f.local_14)->bit & 0x81208850u) ||
             ((map_loc(f.local_18,f.local_14)->bit & 4u) &&
              !PlayersData[f.local_c].region_total_bases[f.local_34]))) {
            if (!f.local_a0) {
                log_say("Enemy doing DESTROY",f.vehicle_id,f.local_18,f.local_14);
                action_destroy(f.vehicle_id,0,-1,-1);
                goto label_578f46;
            }
            log_say("Auto sending DESTROY message",f.vehicle_id,0,0);
            message_data(0x2406,f.vehicle_id,0,0,0,0);
            NetDaemonNet.await_exec(1);
            return 1;
        }
    }
    if (f.local_88 >= 3 && !(PlayersData[f.local_c].unk_77[f.local_34] & 0x2000u) &&
        !f.local_94 && f.local_50 <= 3 && !f.local_40)
        Vehs[f.vehicle_id].state |= 0x40000000u;
    else if (f.local_40 == 2) goto label_57137d;
    goto label_571101;
label_571101:
    if (f.local_50 == 8 || f.local_50 == 9) goto label_57137d;
    if (!f.local_64 && !(Vehs[f.vehicle_id].state & 0x40000000u)) {
        int limit = 2;
        if (f.local_50 == 3) {
            if (Vehs[f.vehicle_id].home_base_id != f.local_5c && !f.local_94)
                goto label_57117a;
        } else if (f.local_50 != 2 && f.local_50 != 1) limit = f.local_40 == 0;
        if (stack_check(f.vehicle_id,16,-1,-1,-1) <= limit || f.local_94)
            goto label_57137d;
    }
label_57117a:
    if (f.local_84) goto label_57137d;
    if ((Vehs[f.vehicle_id].order || f.local_50 == 7) && f.local_50 != 3 &&
        !(Vehs[f.vehicle_id].state & 0x40000000u)) {
        if (!f.local_40) goto label_57137d;
        if (f.local_50 == 7) {
            if (!(Vehs[f.vehicle_id].state & 0x20000000u)) goto label_57137d;
            goto label_5711d4;
        }
    }
    goto label_5711c6;
label_5711c6:
    if (f.local_50 == 6 && Vehs[f.vehicle_id].order == 24) goto label_57137d;
label_5711d4:
    if (GameDataStateStorage.fields_.turn_current_num_ >= 100 && !f.local_4c && f.local_64 > 6 &&
        Vehs[f.vehicle_id].order == 24 && Vehs[f.vehicle_id].move_to_ai_type != 'X' &&
        !(Vehs[f.vehicle_id].state & 0x40000000u) && f.local_50 != 3)
        goto label_57137d;
    f.local_54 = 25;
    if (f.local_50 == 3 || f.local_40 == 1) f.local_54 = 49;
    if (f.local_50 == 7 &&
        (stack_check(f.vehicle_id,2,8,-1,-1) || f.local_90 >= 2)) f.local_54 = 9;
    if (GameDataStateStorage.fields_.turn_current_num_ >= (f.local_50 == 6 ? 400 : 100) &&
        Vehs[f.vehicle_id].order == 24 && Vehs[f.vehicle_id].move_to_ai_type != 'X' &&
        f.local_50 != 3) f.local_54 = 9;
    f.local_2c = 1;
    if (f.local_54 <= 1) goto label_57137d;
    do {
        f.local_4 = xrange(f.local_18+RadiusOffsetX[f.local_2c]);
        f.local_8 = f.local_14+RadiusOffsetY[f.local_2c];
        if (on_map(f.local_4,f.local_8) && goody_at(f.local_4,f.local_8) &&
            (is_known(f.local_4,f.local_8,f.local_c) ||
             (GameDataStateStorage.fields_.diff_level_current_ >= 4 &&
              !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)))) &&
            (((map_loc(f.local_4,f.local_8)->climate & 0xe0) < 0x60) == (f.local_40 == 1)) &&
            map_loc(f.local_4,f.local_8)->region == f.local_34)
            goto label_571537;
        ++f.local_2c;
    } while ((int)f.local_2c < f.local_54);
label_57137d:
    f.local_80 = -1;
    goto label_571384;
label_571384:
    goto label_57138d;

label_571537:
    if (Vehs[f.vehicle_id].state & 0x40000000u)
        PlayersData[f.local_c].unk_77[f.local_34] |= 0x2000u;
    go_to(f.vehicle_id,0x47,f.local_4,f.local_8);
    goto label_57707f;

label_57138d:
    f.local_68 = 1u << (f.local_c & 31);
    if (!(static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & f.local_68) &&
        !f.local_64 && !f.local_6c && f.local_94 && !is_objective(f.local_5c) &&
        (f.local_88 == 2 || PlayersData[f.local_c].region_total_bases[f.local_34] == 1 ||
         (PlayersData[f.local_c].flags & 0x2000000u))) {
        f.local_70 = Bases[f.local_5c].faction_id_former;
        if (stack_check(f.vehicle_id,1,-1,-1,-1) <= (Bases[f.local_5c].population_size+1)/2 &&
            stack_check(f.vehicle_id,16,-1,-1,-1) < (Vehs[f.vehicle_id].dmg_incurred != 0)+2 &&
            f.local_70 != f.local_c && (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                                    (1u << (f.local_70 & 31)))) {
            const unsigned treaty = PlayersData[f.local_c].diplo_treaties[f.local_70];
            if ((treaty & 0x20u) && !(treaty & 3u) &&
                ((treaty & 0x40000u) || PlayersData[f.local_70].integrity_blemishes > 5 ||
                 static_cast<int>(PlayersData[f.local_c].diplo_friction[f.local_70]) >
                     3-PlayersData[f.local_c].ai_fight*2)) {
                log_say("Enemy doing GIVE",f.vehicle_id,f.local_18,f.local_14);
                action_give(f.vehicle_id,f.local_5c);
                return 1;
            }
        }
    }
label_571579:
    if (f.local_7c || (f.local_64 && !f.local_90) || f.local_b0)
        goto label_571977;
    f.local_48 = garrison_check(f.local_9c);
    defensive_units = defensive_check(f.local_9c);
    f.local_30 = stack_check(f.vehicle_id,11,0x10000000,0x10000000,-1);
    available_units = range(stack_check(f.vehicle_id,2,2,f.local_c,-1),0,defensive_units)+f.local_30;
    if ((Vehs[f.vehicle_id].state & 0x10000000u) &&
        f.local_30 <= (f.local_48-defensive_units < 1 ? 1 : f.local_48-defensive_units))
        f.local_50 = 2;
    if (f.local_88 == 2 || f.local_88 == 8) {
        available_units += stack_check(f.vehicle_id,2,1,f.local_c,-1);
        if (f.local_50 == 1) f.local_50 = 2;
    }
    if (!available_units) available_units = stack_check(f.vehicle_id,2,3,f.local_c,-1);
    if (!f.local_98 && !f.local_a8)
        f.local_48 += stack_check(f.vehicle_id,11,0x40000,0x40000,-1);
    if (!zoc_veh(f.local_18,f.local_14,f.local_c) && f.local_88 > 2 && !f.local_6c) {
        if (f.local_50 == 3 || f.local_50 <= 2) {
            if (!(f.local_50 != 3 && PlayersData[f.local_c].region_total_combat_vehs[f.local_34] >= 2) &&
                !(PlayersData[f.local_c].unk_77[f.local_34] & 0x2000u)) goto label_571982;
        }
    }
    if (f.local_50 == 2 || f.local_50 == 3 ||
        (f.local_50 == 1 && (f.local_88 == 2 || f.local_88 == 8))) goto label_571853;
    if (stack_check(f.vehicle_id,2,2,f.local_c,-1) ||
        stack_check(f.vehicle_id,2,3,f.local_c,-1)) goto label_571977;
    available_units = 1;
    if (stack_check(f.vehicle_id,2,1,f.local_c,-1) > static_cast<int>(f.local_50 == 1)) goto label_571977;
    add_goal(f.local_c,2,stack_check(f.vehicle_id,range(Bases[f.local_9c].population_size,1,3),
             -1,-1,-1),f.local_18,f.local_14,-1);
    if (!Weapon[VehPrototypes[Vehs[f.vehicle_id].proto_id].weapon_id].offense_rating &&
        !zoc_sea(f.local_18,f.local_14,f.local_c)) goto label_571977;
    f.local_b4 = 1;
    f.local_8c = 1;
    if (f.local_6c) return 1;
    goto label_57707f;
label_571853:
    capacity_limit = f.local_48;
    if (available_units < capacity_limit) add_goal(f.local_c,2,2,f.local_18,f.local_14,-1);
    if ((available_units > capacity_limit &&
         (f.local_50 != 2 || !f.local_30 || available_units >= 3)) || f.local_98 || f.local_a8) {
        if (available_units >= 2 && (!f.local_44 || !f.local_94)) goto label_571977;
    }
    if (!f.local_64 && Vehs[f.vehicle_id].home_base_id >= 0 &&
        !(static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & static_cast<unsigned char>(f.local_68)))
        Vehs[f.vehicle_id].home_base_id = static_cast<short>(f.local_5c);
    Vehs[f.vehicle_id].order = 0;
    f.local_8c = 1;
    f.local_b4 = (available_units <= 1)+1;
    if (f.local_6c) return 0;
    if (f.local_50 != 2 || f.local_88 <= 2 || available_units < capacity_limit ||
        (Bases[f.local_5c].state & 0x4000000u) ||
        (available_units <= 1 && (PlayersData[0].region_total_combat_vehs[f.local_34] ||
                                (PlayersData[f.local_c].flags & 0x10000u)))) goto label_57707f;
    goto label_571982;
label_571977:
    if (f.local_6c) return 0;
label_571982:
    {
        const SelectedExit exit = enemy_move_selected(f);
        if (exit == SELECTED_RETURN_ONE) return 1;
        if (exit == SELECTED_JOIN_57707F) goto label_57707f;
        if (exit == SELECTED_JOIN_578B26) goto label_578b26;
        goto label_572922;
    }
label_572922:
    if (f.local_b0 || (f.local_50 & 15) != 9) goto label_5741cf;
    if (f.local_94 == 1 || (f.local_94 &&
        (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & static_cast<unsigned char>(f.local_68)) &&
        (PlayersData[f.local_c].diplo_treaties[f.local_94-1] & 0x10u))) goto label_57707f;
    //572984..57302D. Goal records and base coordinates retain their owners.
    f.local_30 = (Vehs[f.vehicle_id].state & 0x200u) != 0 &&
        VehPrototypes[Vehs[f.vehicle_id].proto_id].plan == 9 &&
        Vehs[f.vehicle_id].order_auto_type == 3;
    f.local_4c = 0;
    if (Vehs[f.vehicle_id].order != 0x18) {
        if (f.local_30 || f.local_5c < 0) {
            f.local_24 = Vehs[f.vehicle_id].home_base_id;
            if (f.local_30) {
                if (f.local_24 < 0) f.local_24 = f.local_5c;
            }
        } else {
            f.local_24 = f.local_5c;
            const int home = Vehs[f.vehicle_id].home_base_id;
            if (home >= 0 && home != f.local_24) {
                const int dy = abs(f.local_14-Bases[home].y);
                const int dx = x_dist(f.local_18,Bases[home].x);
                if (vector_dist(dx,dy) < f.local_64*2)
                    f.local_24 = Vehs[f.vehicle_id].home_base_id;
            }
        }
        if (f.local_24 >= 0) {
            f.local_44 = 0;
            do {
                Goal &goal = PlayersData[f.local_c].goals[f.local_44];
                if (goal.type >= 0 && (goal.type & 15) == 9 && goal.type != 0x29 &&
                    on_map(goal.x,goal.y) && goal.priority != 0) {
                    f.local_60 = goal.x;
                    const int goal_dy = abs(goal.y-Bases[f.local_24].y);
                    const int goal_dx = x_dist(f.local_60,Bases[f.local_24].x);
                    if (vector_dist(goal_dx,goal_dy) <= 2 ||
                        (!f.local_30 && goal.base_id == f.local_24)) {
                        f.local_60 = f.local_18;
                        f.local_60 = abs(f.local_14-goal.y);
                        const int distance = x_dist(f.local_18,goal.x)+f.local_60;
                        if (!(distance & ~1)) {
                            Vehs[f.vehicle_id].order = 0;
                            f.local_4c = 1;
                            if (!f.local_64) goto label_578b34;
                            break;
                        }
                        const int low = (map_loc(goal.x,goal.y)->climate & 0xe0) < 0x60;
                        if (low == (f.local_40 == 1) &&
                            (map_loc(goal.x,goal.y)->region == f.local_34 ||
                             f.local_74 != low)) {
                            f.local_70 = anything_at(goal.x,goal.y);
                            if (f.local_70 < 0 || f.local_70 == f.local_c ||
                                (PlayersData[f.local_c].diplo_treaties[f.local_70] & 1u)) {
                                f.local_1c = 0;
                                while (f.local_1c < GameDataStateStorage.fields_.vehicle_current_count_) {
                                    const Veh &other = Vehs[f.local_1c];
                                    if (other.faction_id == f.local_c &&
                                        VehPrototypes[other.proto_id].plan == 9 &&
                                        f.local_1c != f.vehicle_id &&
                                        ((other.x == goal.x && other.y == goal.y) ||
                                         (other.order == 0x18 && other.waypoint_x[0] == goal.x &&
                                          other.waypoint_y[0] == goal.y))) break;
                                    ++f.local_1c;
                                }
                                if (f.local_1c >= GameDataStateStorage.fields_.vehicle_current_count_) {
                                    go_to(f.vehicle_id,0x67,goal.x,goal.y);
                                    break;
                                }
                            }
                        }
                    }
                }
                ++f.local_44;
            } while (f.local_44 < 75);
        }
    }
    // An existing order18 reaches here without initializing raw -24.
    if (f.local_30 && !f.local_4c) {
        const int dy = abs(f.local_14-Bases[f.local_24].y);
        f.local_4c = dy;
        const int distance = x_dist(f.local_18,Bases[f.local_24].x)+f.local_4c;
        if ((distance & ~1) > 4 && Vehs[f.vehicle_id].order != 0x18) {
            go_to(f.vehicle_id,0x68,Bases[f.local_24].x,Bases[f.local_24].y);
            if (Vehs[f.vehicle_id].order == 0x18) goto label_57707f;
        }
    }
label_572d66:
    if (f.local_40 == 1 && Vehs[f.vehicle_id].order != 0x18) {
        f.local_28 = 0x98967f;
        f.local_24 = 0;
        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
            do {
                if (Bases[f.local_24].faction_id_current == f.local_c &&
                    (!f.local_30 || f.local_24 == (Vehs[f.vehicle_id].home_base_id >= 0 ?
                        Vehs[f.vehicle_id].home_base_id : f.local_5c))) {
                    f.local_6c = 0;
                    do {
                        const int offset = f.local_6c/4;
                        f.local_4 = xrange(Bases[f.local_24].x+RadiusOffsetX[offset]);
                        f.local_8 = Bases[f.local_24].y+RadiusOffsetY[offset];
                        if (on_map(f.local_4,f.local_8)) {
                            const int altitude = map_loc(f.local_4,f.local_8)->climate & 0xe0;
                            if (altitude < 0x60 && altitude >= 0x40) {
                                f.local_70 = anything_at(f.local_4,f.local_8);
                                if ((f.local_70 < 0 || f.local_70 == f.local_c ||
                                     (PlayersData[f.local_c].diplo_treaties[f.local_70] & 1u)) &&
                                    can_terraform(f.local_c,f.local_4,f.local_8,0,f.local_24)) {
                                    const int dy = abs(f.local_14-f.local_8);
                                    const int dx = x_dist(f.local_18,f.local_4);
                                    f.local_38 = vector_dist(dx,dy)*8;
                                    if ((map_loc(Bases[f.local_24].x,Bases[f.local_24].y)->climate & 0xe0) < 0x60)
                                        f.local_38 /= 2;
                                    if (f.local_24 == Vehs[f.vehicle_id].home_base_id) f.local_38 /= 2;
                                    if (bonus_at(f.local_4,f.local_8,0)) f.local_38 /= 2;
                                    f.local_1c = 0;
                                    while (f.local_1c < GameDataStateStorage.fields_.vehicle_current_count_) {
                                        const Veh &other = Vehs[f.local_1c];
                                        if (other.faction_id == f.local_c &&
                                            VehPrototypes[other.proto_id].plan == 9 &&
                                            f.local_1c != f.vehicle_id &&
                                            ((other.x == f.local_4 && other.y == f.local_8) ||
                                             (other.order == 0x18 && other.waypoint_x[0] == f.local_4 &&
                                              other.waypoint_y[0] == f.local_8))) break;
                                        ++f.local_1c;
                                    }
                                    if (f.local_1c >= GameDataStateStorage.fields_.vehicle_current_count_ && f.local_38 < f.local_28) {
                                        f.local_28 = f.local_38;
                                        go_to(f.vehicle_id,0x6f,f.local_4,f.local_8);
                                    }
                                }
                            }
                        }
                        f.local_6c += 4;
                    } while (f.local_6c < 0x54);
                }
                ++f.local_24;
            } while (f.local_24 < GameDataStateStorage.fields_.base_current_count_);
        }
    }

    {
        const StrategicActionExit result = enemy_move_strategic_actions(f);
        if (result == STRATEGIC_RETURN_ZERO) return 0;
        if (result == STRATEGIC_JOIN_57707F) goto label_57707f;
        if (result == STRATEGIC_JOIN_578F46) goto label_578f46;
    }
label_5741cf:
    {
        const NonseaExit exit = enemy_move_nonsea(f);
        if (exit == NONSEA_JOIN_578B26) goto label_578b26;
        if (exit == NONSEA_JOIN_57707F) goto label_57707f;
        if (exit == NONSEA_JOIN_577071) goto label_577071;
        goto label_574a88;
    }
label_57460a:
    {
        const NonseaExit exit = enemy_move_nonsea(f,NONSEA_ENTRY_57460A);
        if (exit == NONSEA_JOIN_578B26) goto label_578b26;
        if (exit == NONSEA_JOIN_57707F) goto label_57707f;
        if (exit == NONSEA_JOIN_577071) goto label_577071;
        goto label_574a88;
    }
label_574a7e:
    if (f.local_40 != 1) goto label_577071;
label_574a88:
    if (f.local_50 == 6) del_site(f.local_c,6,f.local_18,f.local_14,0);
    if (f.local_50 == 7) sea_exit = enemy_move_sea_scoring(f);
    else {
        f.local_6c = 0; f.local_44 = 0; f.local_54 = 0;
        f.local_30 = 0; f.local_2c = 0;
        if (f.local_8c) goto label_577071;
        sea_exit = enemy_move_57666a_tail(f);
    }
    if (sea_exit == SEA_JOIN_576DAF || sea_exit == SEA_JOIN_576835 || sea_exit == SEA_JOIN_57681D)
        sea_exit = enemy_move_sea_after_scoring(f,sea_exit);
    switch (sea_exit) {
    case SEA_RETURN_ZERO: return 0;
    case SEA_JOIN_578B34: goto label_578b34;
    case SEA_JOIN_578B26: goto label_578b26;
    case SEA_JOIN_57717F: goto label_57717f;
    case SEA_JOIN_577071: goto label_577071;
    case SEA_JOIN_576DAF: case SEA_JOIN_576835: case SEA_JOIN_57681D: break;
    }
label_577071:
    if (f.local_b8) return 0;
label_57707f:
    if (f.local_40) goto label_57717f;
label_57708a:
    enemy_move_prepare_empty_region_goal(f,true);
label_57717f:
    if (Vehs[f.vehicle_id].order == 24 &&
        (!Weapon[VehPrototypes[Vehs[f.vehicle_id].proto_id].weapon_id].offense_rating ||
         (Vehs[f.vehicle_id].moves_expended != 0 && speed(f.vehicle_id,0) > Rules.move_rate_roads &&
          !f.local_50 && !f.local_40)) && !(Vehs[f.vehicle_id].state & 8u)) {
        if (((abs(f.local_14-Vehs[f.vehicle_id].waypoint_y[0])+
              x_dist(f.local_18,Vehs[f.vehicle_id].waypoint_x[0])) & ~1) > 2) {
            f.local_80 = Paths.move(f.vehicle_id,f.local_c);
            if (f.local_80 >= 0) {
                f.local_4 = xrange(f.local_18+VehMoveDeltaX[f.local_80]);
                f.local_8 = f.local_14+VehMoveDeltaY[f.local_80];
                f.local_70 = zoc_sea(f.local_4,f.local_8,f.local_c);
                bool cancel_path = f.local_70 == 1;
                if (f.local_70 > 1 &&
                    (PlayersData[f.local_c].diplo_treaties[f.local_70] & 0x10u)) {
                    if (f.local_50 != 11) cancel_path = true;
                    else {
                        const int cost = hex_cost(f.local_ac,f.local_c,f.local_18,f.local_14,
                                                  f.local_4,f.local_8,0);
                        cancel_path = veh_moves(f.vehicle_id)-cost < 1;
                    }
                }
                if (cancel_path) {
                    if (!f.local_94 || (f.local_50 != 11 &&
                        !Weapon[VehPrototypes[Vehs[f.vehicle_id].proto_id].weapon_id].offense_rating))
                        Vehs[f.vehicle_id].order = 0;
                    if (!f.local_94) goto label_578b34;
                }
            } else if (f.local_94 == 1) {
                Vehs[f.vehicle_id].order = 0;
                Vehs[f.vehicle_id].state &= 0xfcffbdffu;
            }
        }
    }
label_577361:
    // Native577361..577443 continues into the tactical scan in this owner.
    {

    Veh &v=Vehs[f.vehicle_id];
    if(v.order==0x18&&!f.local_84&&!f.local_8c){
        if(!zoc_veh(f.local_18,f.local_14,f.local_c))goto label_578f46;
        const int distance=abs(f.local_14-v.waypoint_y[0])+x_dist(f.local_18,v.waypoint_x[0]);
        if((distance&~1)==2){
            f.local_70=anything_at(v.waypoint_x[0],v.waypoint_y[0]);
            if(f.local_70>=0&&f.local_70!=f.local_c&&
                !(PlayersData[f.local_c].diplo_treaties[f.local_70]&1u)&&
                v.move_to_ai_type!='p')v.order=0;
        }
    }
    if(v.order==0x18)f.local_c4=1;
    }
// Pinned enemy_move577443..579208. Complete tactical scan and action tail.
// No ORIGINAL marker: integration fragment, not a separately mapped function.
    // Native577443..578AE1 scan and578AF1postscan share this frame.
    {

    Veh &acting=Vehs[f.vehicle_id];
    PlayerData &player=PlayersData[f.local_c];
    int best=-9999;
    int combat_choice=0;
    const int initial_zone=zoc_sea(f.local_18,f.local_14,f.local_c);
    if (initial_zone || f.local_c4 || f.local_98 || f.local_a8)
        f.local_98=0;
    else {
        f.local_98=1;
        if (f.local_50==12) f.local_98=0;
    }
    char bias_text[]="ENEMYBIAS";
    const int rotation=rnd(9,bias_text);
    for (int iteration=0;iteration<9;++iteration) {
        const int pos=(iteration+rotation)%9;
        const int x=xrange(f.local_18+RadiusOffsetX[pos]);
        const int y=f.local_14+RadiusOffsetY[pos];
        if (!on_map(x,y) || (!pos && !f.local_94 && f.local_50!=2)) continue;
        const int low=tactical_low(x,y);
        f.local_84=low;
        int unit=stack_fix(veh_at(x,y));
        const int base=base_at(x,y);
        const int region=map_loc(x,y)->region;
        int owner=f.local_70;
        if (f.local_40==1 && !low) {
            if (unit<0 || owner==f.local_c || !f.local_74) continue;
        } else if (!f.local_74 && !low && f.local_bc && !f.local_40 &&
            unit<0 && !(VehPrototypes[acting.proto_id].ability_flags & 4u) &&
            VehPrototypes[acting.proto_id].plan!=11 &&
            zoc_move(x,y,f.local_c)) continue;
        if (!tactical_offense(f.vehicle_id) && !(unit>=0 && pos) && base<0 &&
            !(acting.state & 8u)) {
            owner=zoc_sea(x,y,f.local_c);
            f.local_70=owner;
            if (owner==1) continue;
            if (owner>0) {
                const unsigned treaty=player.diplo_treaties[owner-1];
                if ((treaty & 1u) && (treaty & 0x2000000u)) continue;
                if (f.local_50!=11) { if(treaty & 0x10u)continue; }
                else if (acting.moves_expended) {
                    const int cost=hex_cost(f.local_ac,f.local_c,f.local_18,f.local_14,x,y,0);
                    if (veh_moves(f.vehicle_id)-cost<=0 && (treaty & 0x10u)) continue;
                }
            }
        }
        char score_text[]="ENEMY3";
        int score=rnd(6,score_text);
        if (low && !f.local_40) {
            bool eligible=base>=0 && has_abil(acting.proto_id,8);
            if (!eligible && unit>=0 && Vehs[unit].faction_id==f.local_c &&
                stack_check(unit,8,-1,-1,-1)>=1 &&
                Vehs[unit].dmg_incurred<=VehPrototypes[Vehs[unit].proto_id].reactor_id*2u) {
                if (f.local_50==12) { score+=99;eligible=true; }
                else if (f.local_50!=8) {
                    if (f.local_50!=9 && ((player.region_base_plan[f.local_34]==7 &&
                        f.local_50<2) || (!player.region_total_bases[f.local_34] &&
                        !(player.unk_77[f.local_34]&6u) &&
                        player.region_base_plan[f.local_34]>=3))) {score+=16;eligible=true;}
                } else if (player.region_base_plan[f.local_34]==7) {score+=16;eligible=true;}
            }
            if (!eligible)continue;
        }
        if (f.local_74 && !low && !f.local_40 && !f.local_64 &&
            !has_abil(acting.proto_id,8) && !stack_check(f.vehicle_id,2,7,-1,-1)) continue;
        if (f.local_74==low && (acting.state&0x10000u) &&
            (!f.local_94 || (f.local_94>=2 &&
             (player.diplo_treaties[f.local_94-1]&7u)))) {
            if (pos) {
                int angular=acting.facing-pos+1;
                if (angular<0)angular=pos-acting.facing-1;
                if (angular>=5)angular=8-angular;
                score-=angular*4;
                if (angular==4)score-=256;
            } else score-=12;
        }
        if(f.local_50!=12){if(f.local_c0&&!pos)score+=8;}
        else {
            int coast_penalty=0;
            if(zoc_veh(x,y,f.local_c))coast_penalty=-32;
            if(!is_coast(x,y,0))coast_penalty-=32;
            if(!pos&&!coast_penalty){score+=(f.local_c0?24:0)+8;
                base_find(x,y,f.local_c);if(BaseFindDist<=f.local_64)score+=8;}
            score+=coast_penalty;
        }
        if(player.flags&0x20000000u){const Map &t=*map_loc(x,y);
            if(!(t.bit&0x100000u)){score+=(t.bit&0x20u)&&(t.climate&0xe0)>=0x40?16:8;
                if(t.bit&0x2000u)score+=8;}}
        owner=whose_territory(f.local_c,x,y,0,0);
        f.local_70=owner;
        if(owner>=0&&owner!=f.local_c&&VehPrototypes[acting.proto_id].plan!=11){
            const unsigned treaty=player.diplo_treaties[owner];
            if(!(treaty&0xc10u)){
                if((player.diplo_agenda[owner]&2u)||(treaty&0x100000u))continue;
                if(treaty&4u)score-=6;
                if(treaty&2u){score-=12;
                    if(!(treaty&1u)&&(f.local_74||!(treaty&0x2020u)||GameDataStateStorage.fields_.sunspot_duration_>0))continue;}
            }
        }
        if(f.local_a8&&f.local_50==2&&unit>=0&&owner==f.local_c&&zoc_veh(x,y,f.local_c))
            score-=stack_check(f.vehicle_id,1,-1,-1,-1)*2;
        if(f.local_74&&!f.local_40&&f.local_64){
            if(player.region_base_plan[region]==f.local_50)score+=32;
            if(player.region_base_plan[region]!=7)score+=32;
            base_find(x,y);
            score+=32-BaseFindDist-stack_check(unit,1,-1,-1,-1);
        }
        if(goody_at(x,y)){
            if(f.local_50==7&&stack_check(f.vehicle_id,1,-1,-1,-1)>1)continue;
            score+=99;
        }
        if(!f.local_74&&!low&&f.local_34!=region&&player.region_total_bases[f.local_34]){
            bool permit=false;
            if(tactical_offense(f.vehicle_id)){
                if(f.local_50==3)permit=true;
                else if(f.local_88>=3)permit=!(player.unk_77[f.local_34]&0x20u);
            } else if(f.local_50==8)permit=!player.unk_82[f.local_34];
            if(!permit)continue;
        }
        if(!(f.local_98&&f.local_b0)){
            const int policy=f.local_a8?2:f.local_50;
            bool friendly=unit>=0&&(Vehs[unit].faction_id==f.local_c||
                (player.diplo_treaties[Vehs[unit].faction_id]&1u));
            if(!pos&&stack_check(unit,1,unit,-1,-1)==1)friendly=false;
            // Native retains an unguarded Vehs[unit] read on this arm.
            if(f.local_74&&!f.local_40&&policy!=8&&Vehs[unit].faction_id==f.local_c){
                if(stack_check(unit,2,8,-1,-1))score+=8;
                if(policy<=1||policy==11){if(player.unk_77[region]&0x20u)score+=16;
                    if(player.unk_77[region]&0x40u)score+=16;}}
            switch(policy){
            case 0:
                if(friendly&&!f.local_98)score+=stack_check(unit,2,2,-1,-1)*4;
                score-=stack_check(unit,2,0,-1,-1)*2;
                score-=hex_cost(f.local_ac,f.local_c,f.local_18,f.local_14,x,y,0)/(int)Rules.move_rate_roads;
                break;
            case 1:if(zoc_veh(x,y,f.local_c))score+=defense_value(f.local_c,x,y,f.vehicle_id,-1)*2;break;
            case 2:
                if(zoc_veh(x,y,f.local_c))score+=defense_value(f.local_c,x,y,f.vehicle_id,-1)*2;
                if(friendly&&!f.local_98)score+=stack_check(unit,2,0,-1,-1)*4-stack_check(unit,2,2,-1,-1)*2;
                break;
            case 3:score-=hex_cost(f.local_ac,f.local_c,f.local_18,f.local_14,x,y,0)/(int)Rules.move_rate_roads;break;
            case 6:score+=stack_check(unit,2,7,-1,-1)*4-stack_check(unit,2,6,-1,-1)*2;break;
            case 7:if(!f.local_98)score+=stack_check(unit,2,6,-1,-1)*4;
                score-=stack_check(unit,2,7,-1,-1)*8;break;
            case 8:
                if(f.local_74&&zoc_sea(x,y,f.local_c))continue;
                if(owner>=0&&owner!=f.local_c&&(player.diplo_treaties[owner]&3u)&&acting.order!=0x18)score-=16;
                if(f.local_74&&player.region_base_plan[region]==8){score+=Continents[region].open_terrain+16;
                    if(!(player.unk_77[region]&0x20u))score+=8;
                    if(!(player.unk_77[region]&0x10u))score+=16;}
                break;
            case 11:if(friendly)score+=stack_check(unit,0xe,-1,-1,-1)*2;break;
            }
            if((map_loc(f.local_18,f.local_14)->climate&0xe0)<(map_loc(x,y)->climate&0xe0)&&
                (Rules.combat_pct_atk_higher_elev||Rules.combat_pen_pct_atk_lwr_elev))score+=2;
        }
        int combat=0;
        if(unit<0){
            if(base>=0&&Bases[base].faction_id_current!=f.local_c&&
                !(player.diplo_treaties[owner]&1u)){
                const int enemy= Bases[base].faction_id_current;
                if(f.local_50==11&&!(VehPrototypes[acting.proto_id].ability_flags&0x10000000u)&&has_project(15,enemy))continue;
                const unsigned treaty=player.diplo_treaties[owner];
                if((treaty&7u)&&!(treaty&0x800u)){
                    if(!(treaty&0x20u))continue;
                    if(treaty&2u){char text[]="ENEMY4";
                        if(reputation(owner,f.local_c)<rnd(6,text)-player.ai_fight)continue;}}
                if(!tactical_offense(f.vehicle_id))continue;
                combat=1;score+=9999;
            }
            if((map_loc(x,y)->bit&0x2000u)&&want_monolith(f.vehicle_id))score+=99;
        } else {
            const int enemy=Vehs[unit].faction_id;
            f.local_70=enemy;
            const unsigned treaty=player.diplo_treaties[enemy];
            const bool direct= f.local_40!=2 || !Chassis[VehPrototypes[acting.proto_id].chassis_id].range ||
                !acting.terraforming_turns || acting.order!=0x18 ||
                (acting.waypoint_x[0]==x&&acting.waypoint_y[0]==y);
            // Raw57813E/57814E skips this candidate immediately when the
            // ranged-air waypoint differs; Kuna's outer else is misleading.
            if(!direct)continue;
            const bool single_colony=stack_check(unit,1,-1,-1,-1)==1&&
                !(treaty&0x11u)&&VehPrototypes[Vehs[unit].proto_id].plan==12&&
                tactical_offense(f.vehicle_id)&&base<0;
            if(!single_colony){
                if(f.local_50==11&&enemy==f.local_c)goto tactical_finish_score;
                bool handled=false;
                if(f.local_50==11&&stack_check(unit,2,11,-1,-1)&&
                    (!(treaty&1u)||!(treaty&0x2000000u))){
                    combat=1;
                    int odds=battle_fight(f.vehicle_id,pos-1,0,0,&unit);
                    if(whose_territory(f.local_c,x,y,0,0)!=f.local_c){
                        if(base<0&&(treaty&7u))odds/=2;
                    } else odds*=2;
                    if(odds>=8){score+=odds;handled=true;}
                    else if(base<0)continue;
                    else handled=true;
                }
                if(!handled&&enemy!=f.local_c&&!(treaty&1u)){
                    if(f.local_74&&!low&&f.local_40==1&&!can_arty(f.local_ac,1))continue;
                    const bool protected_probe=stack_check(unit,1,-1,-1,-1)==1&&
                        VehPrototypes[Vehs[unit].proto_id].plan==11&&
                        (treaty&6u)&&whose_territory(f.local_c,Vehs[unit].x,Vehs[unit].y,0,0)==f.local_c;
                    if(protected_probe){combat=1;score+=99;}
                    else {
                        if(f.local_50==7&&stack_check(f.vehicle_id,1,-1,-1,-1)>1)continue;
                        if(!(f.local_50<8||f.local_50==11))continue;
                        if(f.local_74&&!f.local_40&&!has_abil(acting.proto_id,8))continue;
                        const int artillery=stack_check(unit,3,2,-1,-1);
                        if(artillery&&!has_abil(acting.proto_id,0x20)&&base<0&&
                            !(map_loc(x,y)->bit&0x40000u)&&!stack_check(unit,6,0x80,-1,-1))continue;
                        if(f.local_50!=11){
                            if(tactical_offense(f.vehicle_id)&&(treaty&0x810u)){
                                combat=1;
                                int odds=battle_fight(f.vehicle_id,pos-1,0,0,&unit);
                                if(f.local_b4|| (veh_moves(f.vehicle_id)<(int)Rules.move_rate_roads&&f.local_ac!=8)||
                                    (!f.local_64&&f.local_50>=1)){
                                    const int reverse=battle_fight(unit,(pos+3)&7,0,0,0);
                                    if(reverse){const int damage=f.local_b4?f.local_b4+1:0;
                                        if((unsigned)(4-damage)*reverse<(unsigned)odds*4u||reverse<=7||odds<f.local_b4+6)continue;}
                                    else if(stack_check(f.vehicle_id,1,-1,-1,-1)==1&&
                                        VehPrototypes[Vehs[unit].proto_id].plan!=11)continue;
                                }
                                odds=compute_odds(odds,f.local_c,f.vehicle_id,unit,base);
                                if(!f.local_70&&(f.local_5c<0||!Bases[f.local_5c].eco_damage)&&
                                    Vehs[unit].order_auto_type!=f.local_c){
                                    odds*=player.soc_effect_active.values_[SocialEffect::PLANET]>=3?8:2;}
                                if(odds>=8){score+=odds;handled=true;}
                            }
                        } else if(stack_check(unit,1,-1,-1,-1)<=1&&
                            base_who(Vehs[unit].x,Vehs[unit].y)<0){
                            const int before=GameDataStateStorage.fields_.vehicle_current_count_;
                            if(probe(f.vehicle_id,-1,unit,1)){
                                if(before!=GameDataStateStorage.fields_.vehicle_current_count_)return 1;
                                best=999;f.local_80=pos-1;combat_choice=1;
                            }
                        }
                        if(!handled)continue;
                    }
                }
            } else {
                char intercept_text[]="ENEMYMISC";
                if(!(treaty&0x20u)&&whose_territory(f.local_c,x,y,0,0)!=f.local_c&&
                    !rnd(2,intercept_text))continue;
                combat=1;score+=8;
            }
        }
tactical_finish_score:
        if((f.local_98||f.local_b0)&&pos&&
            (f.local_40==2||low==(f.local_40==1))){
            if(f.local_b0){base_find(x,y,f.local_c);score+=BaseFindDist*2;}
            for(int step=0;step<8;++step){
                const int rx=xrange(x+RadiusOffsetX[pos]*step);
                const int ry=y+RadiusOffsetY[pos]*step;
                if(!on_map(rx,ry)||(f.local_40!=2&&tactical_low(rx,ry)!=(f.local_40==1)))continue;
                for(int n=0;n<8;++n){
                    const int nx=xrange(rx+VehMoveDeltaX[n]);const int ny=ry+VehMoveDeltaY[n];
                    if(!on_map(nx,ny))continue;
                    const Map &tile=*map_loc(nx,ny);
                    if((player.flags&0x20000000u)&&!(tile.bit&0x100000u)){
                        score+=(tile.bit&0x20u)&&(tile.climate&0xe0)>=0x40?16:8;
                        if(tile.bit&0x2000u)score+=8;}
                    if(!is_known(nx,ny,f.local_c)){score+=f.local_b0*2+2;
                        if(f.local_40==1&&(tile.climate&0xe0)>=0x60&&!player.region_total_bases[tile.region])score+=8;}
                    if(goody_at(nx,ny))score+=f.local_b0*2+2;
                    if(f.local_50==8){const int territory=whose_territory(f.local_c,nx,ny,0,0);
                        f.local_70=territory;
                        if(territory>=0&&territory!=f.local_c&&(player.diplo_treaties[territory]&3u))score-=6;}
                }
            }
        }
        if(best<score){f.local_80=pos-1;best=score;combat_choice=combat;}
    }
//578AE1 is a loop increment, NOT a whole-function dispatch fallback. Every
//surviving scan ends here with the original winner and combat flag live.
    // The native scan falls into its postscan with combat_choice live.
    {

    Veh &v=Vehs[f.vehicle_id];
    PlayerData &player=PlayersData[f.local_c];
    if(combat_choice){
        if(planet_buster(f.vehicle_id)){f.local_80=-1;v.order=0;}
    } else {
        if(f.local_8c){f.local_80=-1;v.order=0;}
        if(f.local_c4&&v.order==0x18)goto label_578f46;
        if(can_arty(f.local_ac,1)&&!v.order&&
            (f.local_40==1 || !tactical_low(f.local_18,f.local_14) || !f.local_7c)){
            for(int pos=0;pos<25;++pos){
                const int x=xrange(f.local_18+RadiusOffsetX[pos]);
                const int y=f.local_14+RadiusOffsetY[pos];
                if(!on_map(x,y)||tactical_low(x,y)||anything_at(x,y)>=0)continue;
                const int owner=whose_territory(f.local_c,x,y,0,0);
                if(owner<0||owner==f.local_c||!(player.diplo_treaties[owner]&0x10u))continue;
                const int region=map_loc(x,y)->region;
                if(player.region_total_bases[region]&&player.region_base_plan[region]!=2&&!f.local_40)continue;
                const unsigned bits=map_loc(f.local_18,f.local_14)->bit;
                if(!(bits&0x1000u)||!(bits&0x81208854u))continue;
                if(!f.local_a0){
                    char text[]="Enemy doing ARTY DESTROY 2";
                    log_say(text,f.vehicle_id,x,y);
                    action_destroy(f.vehicle_id,0,x,y);
                    return 1;
                }
                char text[]="Auto sending DESTROY2 message";
                log_say(text,f.vehicle_id,x,y);
                message_data(0x2413,f.vehicle_id,x,y,0,0);
                NetDaemonNet.await_exec(1);
                return 1;
            }
        }
    }
    v.move_to_ai_type='x';
    if(f.local_80>=0){
        const int x=xrange(f.local_18+VehMoveDeltaX[f.local_80]);
        const int y=f.local_14+VehMoveDeltaY[f.local_80];
        v.order=0x58;v.waypoint_x[0]=(short)x;v.waypoint_y[0]=(short)y;
    } else {
        //578B34 falls through here after the absent direction.
        v.state&=~0x10000u;
        v.order=0;
    }
    goto label_578f46;
    }
    }
label_578b26:
label_578b29:
    if (f.local_80 < 0) goto label_578b34;
    f.local_4 = xrange(f.local_18+VehMoveDeltaX[f.local_80]);
    f.local_8 = f.local_14+VehMoveDeltaY[f.local_80];
    Vehs[f.vehicle_id].order = 0x58;
    Vehs[f.vehicle_id].waypoint_x[0] = static_cast<short>(f.local_4);
    Vehs[f.vehicle_id].waypoint_y[0] = static_cast<short>(f.local_8);
    goto label_578f46;
label_578b34:
    Vehs[f.vehicle_id].state &= ~0x10000u;
    Vehs[f.vehicle_id].order = 0;
    goto label_578f46;
label_578de9:
    enemy_move_consume_route_waypoint(f.vehicle_id);
label_578f46:
// Shared final action/path owner. Keeps saved coordinates, rather than
// assuming that prior movement/probe calls left them equal to Veh.x/y.
    // One shared native578F46..579208 action/path continuation.
    {

    Veh &v=Vehs[f.vehicle_id];
    if (v.order < 0x18) v.state &= ~0x10000u;
    if (v.order == 0 || v.order == 2) {
        if (f.local_40 != 2 || !f.local_64 ||
            (map_loc(f.local_18,f.local_14)->bit & 0x40000u)) {
            v.order = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (unsigned char)(1u << (f.local_c & 31))) &&
                      !(v.state & 0x200u) ? 0 : 2;
            veh_skip(f.vehicle_id);
        } else action_home(f.vehicle_id,0);
    }
    if (v.order == 0x18 && v.waypoint_x[0] == f.local_18 && v.waypoint_y[0] == f.local_14) {
        v.order=0;
        veh_skip(f.vehicle_id);
    }
    if ((v.order != 0x18 && v.order != 0x58) ||
        (f.local_50 != 8 && f.local_50 != 9)) return 0;
    const int direction=Paths.move(f.vehicle_id,(v.order & 0x40)?-1:v.faction_id);
    if (direction<0) { v.order=0; return 0; }
    const int x=xrange(f.local_18+VehMoveDeltaX[direction]);
    const int y=f.local_14+VehMoveDeltaY[direction];
    if (base_who(x,y)>=0) return 0;
    for (int n=0;n<8;++n) {
        const int nx=xrange(x+VehMoveDeltaX[n]);
        const int ny=y+VehMoveDeltaY[n];
        if (!on_map(nx,ny) || tactical_low(nx,ny)!=tactical_low(x,y)) continue;
        if (((abs(ny-f.local_14)+x_dist(nx,f.local_18)) & ~1)<3) continue;
        const int enemy=stack_fix(veh_at(nx,ny));
        if (enemy<0 || Vehs[enemy].faction_id==f.local_c ||
            !(PlayersData[f.local_c].diplo_treaties[Vehs[enemy].faction_id] & 0x10u))
            continue;
        if (tactical_offense(enemy) || stack_check(enemy,0xe,-1,-1,-1)) {
            v.order=0;
            return 0;
        }
    }
    return 0;
    }
}

// ---------------------------------------------------------------------------
// THE ENEMY AI MOVE DRIVER (0x00579240 / 0x00579510), homed 2026-09-06 in
// general.cpp and moved beside enemy_move, its translation unit, 2026-11-07.
//
// enemy_turn sweeps the vehicle table ten times, once per priority band, and
// hands each of the faction's vehicles to enemy_veh; enemy_veh drives one
// vehicle's move and reports whether the turn moved on.
// ---------------------------------------------------------------------------

// Original direct-address writable .data strings passed to the logging calls.
// RECOVERY_OBJECT: v2 0x0068EAFC 0x0068EB0F ?LogEnemyVehicleDone@@3PADA 0x00579240
char LogEnemyVehicleDone[] = "Enemy Vehicle DONE";
// RECOVERY_OBJECT: v2 0x0068EB10 0x0068EB25 ?LogEnemyVehicleAction@@3PADA 0x00579240
char LogEnemyVehicleAction[] = "Enemy Vehicle ACTION";
// RECOVERY_OBJECT: v2 0x0068EB28 0x0068EB36 ?LogEnemyVehicle@@3PADA 0x00579240
char LogEnemyVehicle[] = "Enemy Vehicle";

// ORIGINAL: 0x00579240 ?enemy_veh@@YAHH@Z 0x00579240-0x005794C8 BYTE_EXACT
// LEVER (2026-11-07, enemy TU): byte-exact once (1) the faction is an int
//        local passed through two identity switches before zoc_any - the
//        image's two jump tables at 0x5794C8/0x5794E8 are exactly what VC6
//        emits for `switch (f) { case 0: f = 0; break; ... case 7: f = 7; }`
//        written twice; (2) the moves test is `!veh_moves(veh_id)`, the
//        clamped 0..999 inline; (3) the local-turn branch and the AI branch
//        are an if/else falling into one `return 0`; (4) the action and state
//        logs share one 8-byte buffer.
// TRIED: `Veh *veh = &Vehs[veh_id]` - the image never materialises the array
//        base, it folds 0x9528xx into every access displacement and keeps only
//        `unit_id * sizeof(Veh)` live in edi; a pointer local puts the base in
//        the register and the field offset in the displacement instead, so
//        every member access disagrees. The array spelling `Vehs[unit_id].f`
//        is what is here.
// TRIED: reassigning the parameter (the image reuses [ebp+8] for the
//        local-faction flag, so it must have done) - no change, still 4/208
//        with the same push/mov order, so the readable named local stays.
// LEVER: not reproduced - the image's two jump tables on the faction id are
//        32 instructions that both map 0..7 onto themselves (identity, so no
//        source computes anything) and this body has nothing where they are.
//        Everything else is register placement: image `mov ebx,[ebp+8]` /
//        ours `push esi; push edi; mov edi,[ebp+8]`, and the base-fold above.
int __cdecl enemy_veh(int veh_id) {
    int faction_id = Vehs[veh_id].faction_id;
    BOOL is_local_turn;
    if (faction_id == ConsoleGlobal.faction_id_ && IsMultiplayerNet != 0) {
        is_local_turn = true;
    } else {
        is_local_turn = false;
    }

    if (IsMultiplayerNet != 0) {
        NetDaemonNet.channels_[0].leading_words_[3] = 3;
    }

    do_all_non_input();

    // Two identity switches on the faction, as the image's two jump tables
    // at 0x005794C8 and 0x005794E8 have them. Nothing is computed.
    switch (faction_id) {
    case 0: faction_id = 0; break;
    case 1: faction_id = 1; break;
    case 2: faction_id = 2; break;
    case 3: faction_id = 3; break;
    case 4: faction_id = 4; break;
    case 5: faction_id = 5; break;
    case 6: faction_id = 6; break;
    case 7: faction_id = 7; break;
    }
    switch (faction_id) {
    case 0: faction_id = 0; break;
    case 1: faction_id = 1; break;
    case 2: faction_id = 2; break;
    case 3: faction_id = 3; break;
    case 4: faction_id = 4; break;
    case 5: faction_id = 5; break;
    case 6: faction_id = 6; break;
    case 7: faction_id = 7; break;
    }

    if (Vehs[veh_id].moves_expended != 0
            && Vehs[veh_id].order == ORDER_MOVE_TO
            && VehPrototypes[Vehs[veh_id].proto_id].plan != 7
            && !(Vehs[veh_id].state & VSTATE_EXPLORE)) {
        if (zoc_any(Vehs[veh_id].x, Vehs[veh_id].y, faction_id) == 0) {
            goto after_move;  // blocked by a zone of control
        }
    }

    if (enemy_move(veh_id) != 0) {
        return 1;
    }

    if (!veh_moves(veh_id)) {
        if (IsMultiplayerNet != 0) {
            log_say(LogEnemyVehicleDone, veh_id, 0, 0);
        }
        if (is_local_turn) {
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
            NetDaemonNet.unlock_veh();
        }
        return 1;
    }

after_move:
    if (is_local_turn) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
        if (NetDaemonNet.action(veh_id, 1) != 0) {
            return 1;
        }
    } else {
        char message[8];
        if (IsMultiplayerNet != 0) {
            char number_text[0x50];
            message[0] = 0;
            _itoa(veh_id, number_text, 10);
            strcat(message, number_text);
            log_say(LogEnemyVehicleAction, message, Vehs[veh_id].order,
                    Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
        }

        if (action(veh_id) == 0) {
            veh_skip(veh_id);
        }

        if (IsMultiplayerNet != 0) {
            message[0] = Vehs[veh_id].move_to_ai_type;
            message[1] = 0;
            log_say(LogEnemyVehicle, message, veh_id, Vehs[veh_id].x,
                    Vehs[veh_id].y);
        }
    }

    return 0;
}

// ORIGINAL: 0x00579510 ?enemy_turn@@YAHH@Z 0x00579510-0x00579769
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x00579643-0x00579687 routes priorities7..9 around the DROP_POD, plan4 and unk_1==0x15 filters used by priorities0..6; priority8 alone then requires plan7 and nonzero faction at0x0057968A-0x005796A5.", "Original rejected priority8 paths jump0x005795A7 to decrement the vehicle index and pointer; the for-loop increment clause preserves this tail for continue as well as ordinary completion."], "reason": "Preserve original enemy-turn early-band filters, late-band bypass and common descending-vehicle tail."}
// symbol    ?enemy_turn@@YAXH@Z
// LEVER: void, not the catalogue's int - the body's only `ret` leaves eax
//        holding whatever the last address calculation left there
//        (`faction_id * 0x20CC`, from the PlayersData store), not a return
//        value. See the same recast on 0x00579510's archived transcription.
// TRIED: `bool matched` locals - the image has no such variable. Each band
//        ends in two bare jumps (one to the matched point 0x579648, one to the
//        next-vehicle tail), with no setcc and no store, so a `matched`
//        local costs a `setne al; test al,al` pair the image does not have.
//        Faithful source here would be goto-per-band, which this pass did not
//        spend the budget on: the body is 12/184 on register placement alone.
// TRIED: `Veh *veh` across the sweep - the image keeps exactly one such
//        pointer (esi, `sub esi,0x34`), this tree spills a second to
//        [ebp-0xc], so the frame is 0xc against the image's 8.
// LEVER (2026-11-07, enemy TU): 0.77 -> 0.854 with `continue` per band (no
//        `matched`), Vehs[veh_id] instead of a pointer, `!veh_moves(veh_id)`
//        for the moves test, and turns_spent declared before the
//        `veh_id >= 0` test, which gives the image's registers (veh_id ebx,
//        priority and turns_spent in the frame). Left: the image places the
//        next-vehicle tail and the epilogue right after band 1 and the rest of
//        the body after them; a switch on priority and goto-per-band do not
//        give that layout.
void __cdecl enemy_turn(int faction_id) {
    for (int priority = 0; priority < 10; priority++) {
        for (int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
                veh_id >= 0; veh_id--) {
            if (veh_id >= GameDataStateStorage.fields_.vehicle_current_count_
                    || Vehs[veh_id].faction_id != faction_id) {
                continue;
            }
            if (priority == 0) {
                if (!can_arty(Vehs[veh_id].proto_id, 1)) {
                    continue;
                }
            } else if (priority == 1) {
                if (VehPrototypes[Vehs[veh_id].proto_id].plan != 2) {
                    if (VehPrototypes[Vehs[veh_id].proto_id].plan != 7
                            || !stack_check(veh_id, 3, 0, -1, -1)) {
                        continue;
                    }
                }
            } else if (priority == 2) {
                if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad
                        != priority
                        || VehPrototypes[Vehs[veh_id].proto_id].plan == 4) {
                    continue;
                }
            } else if (priority == 3) {
                if (VehPrototypes[Vehs[veh_id].proto_id].plan != 1) {
                    continue;
                }
            } else if (priority == 4) {
                if (Vehs[veh_id].dmg_incurred) {
                    continue;
                }
            }
            if (priority < 7) {
                if (has_abil(Vehs[veh_id].proto_id, ABL_DROP_POD)
                        || VehPrototypes[Vehs[veh_id].proto_id].plan == 4
                        || VehPrototypes[Vehs[veh_id].proto_id].unk_1 == 0x15) {
                    continue;
                }
            }
            if (priority == 8
                    && (VehPrototypes[Vehs[veh_id].proto_id].plan != 7 || !faction_id)) {
                continue;
            }
            int turns_spent = 0;
            if (veh_id >= 0) {
                while (Vehs[veh_id].order != ORDER_SENTRY_BOARD) {
                    if ((priority != 8 || turns_spent != 0) && !veh_moves(veh_id)) {
                        break;
                    }
                    const int veh_count_before = GameDataStateStorage.fields_.vehicle_current_count_;
                    if (enemy_veh(veh_id) != 0
                            || veh_count_before != GameDataStateStorage.fields_.vehicle_current_count_) {
                        break;
                    }
                    if (Vehs[veh_id].order != ORDER_MOVE_TO
                            && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != 2) {
                        turns_spent++;
                    }
                    turns_spent++;
                    if (turns_spent > 0x20) {
                        veh_skip(veh_id);
                    }
                }
            }
        }
    }

    PlayersData[faction_id].flags &= ~PFLAG_UNK_10000;
}

/*
Purpose: Calculate a random value within the provided bounds. The unused 2nd parameter was possibly 
         meant to have the random value append to it.
// ORIGINAL: 0x00579770 ?rnd@@YAHHPAD@Z 0x00579770-0x00579790 BYTE_EXACT
// LEVER: ternary flipped to a guard-clause `if (bounds - 1 <= 0) return 0;`
//        then fall through to `return rand() % bounds;` - the image jumps
//        TO the rand() work and falls through to the early return.
// symbol    ?rnd@@YAIHPAD@Z
// size      32 bytes
// prototype int (__cdecl ?rnd@@YAHHPAD@Z)(int seed, int8*)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0064601D
Return Value: Bounded random value
Status: Complete
*/
uint32_t __cdecl rnd(int bounds, LPSTR UNUSED(input)) {
    return bounded_crt_random(bounds);
}

/*
Purpose: Calculate the distance between two x coordinates with handling for round maps.
// ORIGINAL: 0x00579790 ?x_dist@@YAHHH@Z 0x00579790-0x005797C7 BYTE_EXACT
// body      src/map.h
// size      55 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
Return Value: Distance
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
// LEVER: `!(MapIsFlat & 1)`, not `!MapIsFlat` - same idiom as xrange
//        (0x0048BEE0) - took the best flag set (/O2 /Oi- /Gy /GR- /Oy- /GX)
//        from 14/21 to 16/21 (still 0.923 similar - the raw count moved, the
//        rounded ratio didn't).
// TRIED: remaining gap (best flag set, 18 instructions here vs the
//            image's 21) is a trailing `mov ecx, eax` the image emits right
//            before `pop ebp; ret` that this tree's `return dist;` never
//            produces - same register-caching family as vector_dist
//            (0x005A5910)'s note on this same function. Not chased further.
// LEVER (2026-11-07): the round-map arm as a ternary,
//        `return dist > (int)MapLongitude ? MapLongitudeBounds - dist : dist;`
//        inside `if (!(MapIsFlat & 1))`, gives the trailing `mov ecx, eax`
//        and is byte-exact; it also made sub_500380 exact and no x_dist
//        caller in the 17 files that use it lost a match.

// ORIGINAL: 0x005797D0 ?site_at@@YAHHHH@Z 0x005797D0-0x00579832 FILE BYTE_EXACT
// TRIED: single combined expression and explicit temp reads of the first two parameters both match 44 of 46 mnemonics at 98 of 98 bytes; VC6 interleaves the push/mov prologue differently (push, push, mov, mov instead) - register scheduling, not logic
// working copy - scaffold materialised by --work
// size      98 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591B50 0x005C4FD0
int __cdecl site_at(int x, int y, int is_ocean_site) {
    Map *tile = map_loc(x, y);
    if (tile->bit & BIT_BASE_RADIUS) {
        return 0;
    }
    int site = tile->val2 >> 4;
    if (site != 0) {
        return site;
    }
    const int generated = world_site(x, y, is_ocean_site);
    site_set(x, y, generated);
    return generated;
}

/*
Purpose: Check whether a faction can see the specified tile.
// ORIGINAL: 0x00579840 ?is_known@@YAHHHH@Z 0x00579840-0x00579896 BYTE_EXACT
// size      86 bytes
// prototype int (__cdecl ?is_known@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: 16 of 30 at the best flag set (/O2 /Gy /GR- /Oy- /GX), 32
//            compiled instructions against the image's 30. The two extra are
//            `push ebx`/`pop ebx`: the image computes `1 << faction_id` AFTER
//            the tile-index `lea` chain, when edx is dead, and reuses ecx for
//            map_tiles() (`mov edx, 1; shl edx, cl; mov ecx, [0x94a30c];
//            test byte ptr [ecx + eax*4 + 4], dl`); VC6 here hoists the shift
//            ahead of that chain, where eax/ecx/edx are all live, so the mask
//            lands in a callee-saved register and costs the save/restore.
// TRIED: nothing moves that scheduling decision. Measured, all 16 of 30
//            and all 32 instructions: a `mask`/`vis`/`tile`/`flags` local, a
//            `(uint8_t)` cast on either operand, `!!(...)`, an explicit
//            `!= 0`, extra parentheses, and swapping the `&` operands
//            (`(1 << faction_id) & ...->visibility`).
// TRIED: splitting the `||` is WORSE, not better - the mirror of
//            map_write/map_read. A guard clause (`if (A) return true; return
//            B;`) scores 0 of 30, and so does the ternary form; the image
//            shares one `mov eax, 1; pop ebp; ret` tail between both tests
//            and falls through to `xor eax, eax` only once.
// TRIED: the equivalent bit test `(map_loc(x, y)->visibility >>
//            faction_id) & 1` scores HIGHER, 25 of 30 with the instruction
//            count matching at 30, because `shr al, cl; test al, 1` needs no
//            second register - but it emits a shift of the VALUE where the
//            image shifts the MASK, so it trades one divergence for another
//            and still reaches no tier. Left as the image's own mask test.
Return Value: Is tile visible/known to faction? true/false
Status: Complete
*/
BOOL __cdecl is_known(int x, int y, int faction_id) {
    return (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED
        || map_loc(x, y)->visibility & (1 << faction_id));
}

/*
Purpose: If a base exists, get the owner of the specified tile.
// ORIGINAL: 0x005798A0 ?base_who@@YAHHH@Z 0x005798A0-0x005798D8 BYTE_EXACT
// body      src/map.h
// size      56 bytes
// prototype 
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Owner (faction id) or -1
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: six bodies call it where the image
// calls nothing - `call_diff` names it - and 0x005798A0 is a real body.

/*
Purpose: If a base or unit exists, get the owner of the specified tile.
// ORIGINAL: 0x005798E0 ?anything_at@@YAHHH@Z 0x005798E0-0x00579918 BYTE_EXACT
// body      src/map.h
// size      56 bytes
// prototype int (__cdecl ?anything_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Owner (faction id) or -1
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: world_alt_set and action_fungal inline
// it, and 0x005798E0 is a real body.

/*
Purpose: Get the unit on the top of the stack.
// ORIGINAL: 0x00579920 ?veh_top@@YAHH@Z 0x00579920-0x0057995A BYTE_EXACT
// body src/veh.h
// LEVER: the climb rereads Vehs[top].prev_veh_id_stack in a while loop (no
//   int16_t induction local); that spelling also matches the inlined copies in
//   stack_fix and spot_loc.
// size      58 bytes
// prototype int (__cdecl ?veh_top@@YAHH@Z)(int vehID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Unit id if found, otherwise -1
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image expands it in spot_stack,
// spot_loc and stack_fix (veh_stack.cpp), veh_at and veh_promote
// (veh_queries.cpp) and other TUs, and calls this copy only from enemy_move
// and spot_all. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Get the current moves left for the specified unit.
// ORIGINAL: 0x00579960 ?veh_moves@@YAHH@Z 0x00579960-0x00579998 BYTE_EXACT
// body      src/veh.h
// size      56 bytes
// prototype int (__cdecl ?veh_moves@@YAHH@Z)(int vehID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540
Return Value: Remaining moves
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Get the specified unit's reactor power value from its prototype.
// ORIGINAL: 0x005799A0 ?proto_power@@YAHH@Z 0x005799A0-0x005799F9 BYTE_EXACT
// body      src/veh.h
// LEVER: neither of its two callers (best_defender 0x5044D0, action_home
//        0x4CBAA0) has it in their `calls` list - both inline it - but this
//        tree defined it as a plain (non-`inline`) function, so under /Ob1
//        (the default for every flag set but /Ob0) it was never a candidate
//        and both callers paid a real call the image does not make. `MEASURED
//        inline` keeps this address's own BYTE_EXACT claim measurable
//        (the /Ob0 flag sets still force it real for that purpose).
// size      89 bytes
// prototype int (__cdecl ?proto_power@@YAHH@Z)(int vehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Power
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Determine if base is a port.
// ORIGINAL: 0x00579A00 ?is_port@@YAHHH@Z 0x00579A00-0x00579A30 BYTE_EXACT
// size      48 bytes
// prototype int (__cdecl ?is_port@@YAHHH@Z)(int baseID, int isBaseRadius)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E49D0
Return Value: Is base a port? true/false
Status: Complete
*/
BOOL __cdecl is_port(int base_id, BOOL is_base_radius) {
    return is_coast(Bases[base_id].x, Bases[base_id].y, is_base_radius);
}
