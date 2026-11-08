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
// Inlined: owner of a base on an already located tile, else -1.
static inline int base_owner_of(const Map *tile) {
    if (!(tile->bit & BIT_BASE_IN_TILE)) {
        return -1;
    }
    int owner = tile->val2 & 0xF;
    if (owner >= MaxPlayerNum) {
        return -1;
    }
    return owner;
}

void __cdecl enemy_strategy(int faction_id) {
    int base_id;
    int veh_id;
    int colony_count = 0;
    int transports = 0;
    int transport_cap = 2;
    int attackers = 0;

    PlayersData[faction_id].total_mil_veh = 0;
    PlayersData[faction_id].current_num_bases = 0;
    PlayersData[faction_id].mil_strength_1 = 0;
    PlayersData[faction_id].mil_strength_2 = 0;
    PlayersData[faction_id].pop_total = 0;
    PlayersData[faction_id].unk_70 = 0;
    PlayersData[faction_id].planet_busters = 0;
    PlayersData[faction_id].unk_71 = 0;
    PlayersData[faction_id].unk_72 = 0;
    int i;
    for (i = 0; i < 128; i++) {
        PlayersData[faction_id].region_total_combat_vehs[i] = 0;
        PlayersData[faction_id].region_total_bases[i] = 0;
        PlayersData[faction_id].region_force_rating[i] = 0;
        // Every 16th turn (staggered per region) the retained bit 8 clears too.
        PlayersData[faction_id].unk_77[i] &=
            ((GameDataStateStorage.fields_.turn_current_num_ + faction_id + i) & 0xF) ? 8 : 0;
    }
    for (i = 0; i < 512; i++) {
        PlayersData[faction_id].proto_id_active[i] = 0;
        PlayersData[faction_id].proto_id_queue[i] = 0;
    }

    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        int x = Bases[base_id].x;
        int y = Bases[base_id].y;
        uint32_t owner = Bases[base_id].faction_id_current;
        if (faction_id != owner) {
            if (!((1 << faction_id) & Bases[base_id].visibility)) {
                continue;
            }
            uint32_t treaty = PlayersData[faction_id].diplo_treaties[owner];
            if (!(treaty & 0xC10) || (treaty & DTREATY_PACT)) {
                continue;
            }
            int priority = 1;
            if (veh_who(x, y) < 0) {
                priority = 2;
                add_goal(faction_id, AI_GOAL_DEFEND, 2, x, y, -1);
                add_goal(faction_id, AI_GOAL_SCOUT, 2, x, y, -1);
            }
            if (Bases[base_id].faction_id_former == faction_id) {
                priority++;
            }
            if (PlayersData[faction_id].region_total_bases[region_at(x, y)]) {
                priority++;
            }
            if (!has_fac(FAC_PERIMETER_DEFENSE, base_id, 0)) {
                priority++;
            }
            if (Bases[base_id].queue_production_id[0] == -102) {
                priority *= 4;
            }
            if (has_fac(FAC_HEADQUARTERS, base_id, 0)) {
                priority++;
                if (PlayersData[faction_id].flags & 0x200000) {
                    priority *= 2;
                }
                if (PlayersData[Bases[base_id].faction_id_current].corner_market_turn
                    > GameDataStateStorage.fields_.turn_current_num_) {
                    priority <<= 3;
                }
            }
            if (PlayersData[faction_id].base_id_atk_target == base_id
                && (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                    & DTREATY_VENDETTA)) {
                priority <<= 2;
            } else if (is_objective(base_id)) {
                priority *= 4;
            }
            add_goal(faction_id, AI_GOAL_ATTACK, priority, x, y, -1);
            if (on_map(Bases[base_id].unk_x, Bases[base_id].unk_y)) {
                int near_base = base_find(Bases[base_id].unk_x, Bases[base_id].unk_y);
                if (near_base >= 0
                    && Bases[near_base].faction_id_current == Bases[base_id].faction_id_current
                    && terraform_cost(Bases[base_id].unk_x, Bases[base_id].unk_y, faction_id)
                        <= PlayersData[faction_id].energy_reserves / 8) {
                    add_goal(faction_id, AI_GOAL_LANDING_SITE, 2,
                             Bases[base_id].unk_x, Bases[base_id].unk_y, -1);
                }
            }
        } else {
            Bases[base_id].state &= 0xF3FFFFFF;
            if (Bases[base_id].unk_1) {
                Bases[base_id].unk_1--;
            }
            PlayersData[faction_id].current_num_bases++;
            PlayersData[faction_id].pop_total += Bases[base_id].population_size;
            int top = stack_fix(veh_at(x, y));
            if (!stack_check(top, 0xE, faction_id, -1, -1)
                && !stack_check(top, 2, 3, faction_id, -1)) {
                add_goal(faction_id, AI_GOAL_DEFEND, 4, x, y, -1);
            }
            for (int queue = 0; queue <= Bases[base_id].queue_size; queue++) {
                if (Bases[base_id].queue_production_id[queue] >= 0) {
                    PlayersData[faction_id].proto_id_queue[
                        Bases[base_id].queue_production_id[queue]]++;
                }
            }
            int region = region_at(x, y);
            PlayersData[faction_id].region_total_bases[region]++;
            if (is_coast(Bases[base_id].x, Bases[base_id].y, 0)) {
                PlayersData[faction_id].unk_77[region] |= 0x80;
            }
            if (!((GameDataStateStorage.fields_.turn_current_num_ + base_id) & 3)) {
                Bases[base_id].event &= 0xFFFFFF00;
            }
        }
    }

    int self_bit = 1 << faction_id;
    if (!(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            if (Bases[base_id].faction_id_current == faction_id
                && Bases[base_id].queue_production_id[0] == -69) {
                base_reset(base_id, 0);
            }
        }
    }

    int formers[128];
    for (i = 0; i < 128; i++) {
        formers[i] = 0;
    }

    enemy_capabilities(faction_id);
    PlayersData[faction_id].unk_68 = 0;
    PlayersData[faction_id].unk_66 = 0;
    PlayersData[faction_id].unk_65 = 0;
    PlayersData[faction_id].unk_64 = 0;
    PlayersData[faction_id].unk_106 = 0;
    PlayersData[faction_id].unk_105 = 0;

    for (veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; veh_id--) {
        if (veh_id >= GameDataStateStorage.fields_.vehicle_current_count_) {
            continue;
        }
        if (faction_id != Vehs[veh_id].faction_id) {
            uint32_t treaty = PlayersData[faction_id].diplo_treaties[Vehs[veh_id].faction_id];
            if (!(treaty & DTREATY_COMMLINK) || ((treaty & 3) && !(treaty & 0xC20))) {
                continue;
            }
            int proto_id = Vehs[veh_id].proto_id;
            uint8_t plan = VehPrototypes[proto_id].plan;
            if (plan == 7) {
                PlayersData[faction_id].unk_64++;
            }
            if (plan == 6) {
                PlayersData[faction_id].unk_65 += VehPrototypes[proto_id].reactor_id;
            }
            if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
                if (!plan && !Chassis[VehPrototypes[proto_id].chassis_id].missile
                    && weap_val(proto_id, faction_id) >= PlayersData[faction_id].enemy_best_armor_value) {
                    PlayersData[faction_id].unk_66++;
                }
                if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile
                    && !planet_buster(veh_id)) {
                    PlayersData[faction_id].unk_68++;
                }
            }
            if (VehPrototypes[Vehs[veh_id].proto_id].plan == 9
                && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA
                && alt_at(Vehs[veh_id].x, Vehs[veh_id].y) < ALT_SHORE_LINE
                && base_who(Vehs[veh_id].x, Vehs[veh_id].y) < 0
                && ((Vehs[veh_id].faction_id == faction_id
                     && (Vehs[veh_id].flags & 0x440) != 0x440)
                    || (Vehs[veh_id].visibility & (uint8_t)self_bit))) {
                add_goal(faction_id, AI_GOAL_UNK_1,
                         (PlayersData[faction_id].flags & 0x80000) ? 4 : 2,
                         Vehs[veh_id].x, Vehs[veh_id].y, -1);
            }
            continue;
        }

        int x = Vehs[veh_id].x;
        int y = Vehs[veh_id].y;
        int proto_id = Vehs[veh_id].proto_id;
        int region = region_at(x, y);
        BOOL ocean = is_ocean(x, y);
        int plan = VehPrototypes[proto_id].plan;
        int triad = Chassis[VehPrototypes[proto_id].chassis_id].triad;
        PlayersData[faction_id].proto_id_active[proto_id]++;
        if (Vehs[veh_id].year_end_lurking) {
            if (!--Vehs[veh_id].year_end_lurking) {
                Vehs[veh_id].flags &= 0xFBBF;
            }
        }
        // Obsolete designs far from danger disband when a better one exists.
        if (!(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
            && (VehPrototypes[Vehs[veh_id].proto_id].obsolete_factions & self_bit)
            && !ConsoleGlobal.upgrade(veh_id)
            && !((GameDataStateStorage.fields_.turn_current_num_ + veh_id) & 3)
            && GameDataStateStorage.fields_.turn_current_num_ > 50
            && Vehs[veh_id].move_to_ai_type != 'F'
            && Vehs[veh_id].move_to_ai_type != '1'
            && Vehs[veh_id].move_to_ai_type != '2') {
            uint8_t old_plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
            if ((old_plan != 1 || PlayersData[faction_id].region_base_plan[region] > 2)
                && old_plan < 8
                && old_plan != PlayersData[faction_id].region_base_plan[region]
                && (old_plan != 3 || PlayersData[faction_id].region_base_plan[region] != 8
                    || Vehs[veh_id].order == 2)
                && (base_who(x, y) < 0 || stack_check(veh_id, 1, faction_id, -1, -1) > 1)
                && alt_at(x, y) >= ALT_SHORE_LINE
                && PlayersData[faction_id].region_base_plan[region] != 2
                && !zoc_sea(x, y, faction_id)) {
                BOOL better = false;
                uint8_t veh_plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
                if (veh_plan <= 1) {
                    for (int k = 0; k < 64; k++) {
                        int other = faction_id * 64 + k;
                        if ((VehPrototypes[other].flags & 1)
                            && !(self_bit & VehPrototypes[other].obsolete_factions)
                            && VehPrototypes[other].plan == VehPrototypes[Vehs[veh_id].proto_id].plan
                            && Chassis[VehPrototypes[other].chassis_id].triad
                                == Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad
                            && weap_val(other, faction_id) > weap_val(proto_id, faction_id)) {
                            better = true;
                        }
                    }
                } else if (veh_plan == 2) {
                    for (int k = 0; k < 64; k++) {
                        int other = faction_id * 64 + k;
                        if ((VehPrototypes[other].flags & 1)
                            && !(self_bit & VehPrototypes[other].obsolete_factions)
                            && VehPrototypes[other].plan == VehPrototypes[Vehs[veh_id].proto_id].plan
                            && Chassis[VehPrototypes[other].chassis_id].triad
                                == Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad
                            // BUG(original): arm_val is given prototype ids.
                            && arm_val(other, faction_id) > arm_val(proto_id, faction_id)) {
                            better = true;
                        }
                    }
                } else {
                    better = true;
                }
                if (better) {
                    if (!(Vehs[veh_id].flags & 0x20)) {
                        kill(veh_id);
                    }
                    continue;
                }
            }
        }
        // Scouts in settled regions.
        if (!(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
            && ((GameDataStateStorage.fields_.turn_current_num_ + veh_id) & 3)
            && GameDataStateStorage.fields_.turn_current_num_ > 25
            && proto_id == BSC_SCOUT_PATROL
            && (PlayersData[faction_id].region_base_plan[region] == 7 || Vehs[veh_id].order == 2)
            && (base_who(x, y) < 0 || stack_check(veh_id, 0xE, faction_id, -1, -1) > 1)
            && alt_at(x, y) >= ALT_SHORE_LINE
            && PlayersData[faction_id].region_base_plan[region] != 2
            && !zoc_sea(x, y, faction_id)
            && Vehs[veh_id].move_to_ai_type != '1') {
            kill(veh_id);
            continue;
        }
        Vehs[veh_id].state &= 0xCFFFFFDB;
        if (!(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
            && Vehs[veh_id].order == 1 && (triad || !ocean)) {
            Vehs[veh_id].order = 0;
        }
        if (ocean && !triad && base_who(x, y) < 0) {
            if (stack_check(veh_id, 2, 7, -1, -1)) {
                if (Vehs[veh_id].order != 1) {
                    sleep(veh_id);
                }
            } else {
                if (!(Vehs[veh_id].flags & 0x20)) {
                    veh_kill(veh_id);
                }
                continue;
            }
        }
        if (!ocean && triad == TRIAD_SEA && base_who(x, y) < 0) {
            if (!(Vehs[veh_id].flags & 0x20)) {
                veh_kill(veh_id);
            }
            continue;
        }
        // Defensive units in a base short of defenders join its garrison.
        if (plan <= 1 && !(Vehs[veh_id].state & 0x40000)) {
            int base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
            if (base_id < 0 && Vehs[veh_id].move_to_ai_type == 'F') {
                base_id = Vehs[veh_id].home_base_id;
            }
            if (base_id >= 0) {
                int wanted = garrison_check(base_id);
                int margin = garrison_check(base_id);
                if (margin > 5) {
                    margin--;
                }
                if (margin > 2) {
                    margin--;
                }
                if (wanted > stack_check(veh_at(Bases[base_id].x, Bases[base_id].y), 0xB,
                                         0x10000000, 0x10000000, faction_id) + margin) {
                    Vehs[veh_id].state |= 0x10000000;
                }
            }
        }
        if (Vehs[veh_id].move_to_ai_type == 'F' && plan == 2) {
            if (Vehs[veh_id].home_base_id >= 0
                && (Bases[Vehs[veh_id].home_base_id].x != Vehs[veh_id].x
                    || Bases[Vehs[veh_id].home_base_id].y != Vehs[veh_id].y)) {
                Bases[Vehs[veh_id].home_base_id].state |= 0x4000000;
            }
        } else if (plan == 9) {
            Vehs[veh_id].flags &= ~0x800;
            formers[region]++;
            if (Vehs[veh_id].home_base_id >= 0) {
                int home_id = Vehs[veh_id].home_base_id;
                uint32_t mask = (triad != TRIAD_SEA) ? 0x20 : 0x80000;
                if ((mask & Bases[home_id].state)
                    && !(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
                    int best_dist = 9999;
                    int best_id = -1;
                    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_;
                         base_id++) {
                        if (base_id != Vehs[veh_id].home_base_id
                            && region_at(Bases[base_id].x, Bases[base_id].y) == region
                            && !(Bases[base_id].state & mask)
                            && Bases[base_id].mineral_surplus < 2) {
                            int dist = vector_dist(Vehs[veh_id].x, Vehs[veh_id].y,
                                                   Bases[base_id].x, Bases[base_id].y);
                            if (dist < best_dist) {
                                best_dist = dist;
                                best_id = base_id;
                            }
                        }
                    }
                    if (best_id >= 0) {
                        Bases[best_id].state |= mask;
                        Vehs[veh_id].home_base_id = (int16_t)best_id;
                    }
                } else {
                    Bases[home_id].state |= mask;
                }
            }
        } else if (plan == 8) {
            if (PlayersData[faction_id].unk_77[region] & 2) {
                PlayersData[faction_id].unk_77[region] |= 0x800;
            } else {
                PlayersData[faction_id].unk_77[region] |= 2;
            }
            colony_count++;
        } else if (plan == 11) {
            PlayersData[faction_id].unk_77[region] |= 0x1000;
        } else if (plan == 3) {
            if (base_who(x, y) < 0) {
                PlayersData[faction_id].unk_77[region] |= 0x2000;
            }
        }
        if (plan <= 7) {
            PlayersData[faction_id].total_mil_veh++;
            if (region >= 0 && !triad) {
                if (!ocean) {
                    PlayersData[faction_id].region_total_combat_vehs[region]++;
                }
                goto transport_check;
            }
        }
        if (triad == TRIAD_SEA) {
            PlayersData[faction_id].unk_70++;
        } else if (triad == TRIAD_AIR) {
            if (!plan) {
                PlayersData[faction_id].unk_105++;
            } else if (plan == 4) {
                PlayersData[faction_id].unk_106++;
            }
        }
    transport_check:
        if (plan == 7) {
            transports++;
            if (transport_cap <= veh_cargo(veh_id)) {
                transport_cap = veh_cargo(veh_id);
            }
        }
        if (planet_buster(veh_id)) {
            PlayersData[faction_id].planet_busters++;
            continue;
        }
        int weapon_strat = weap_strat(VehPrototypes[Vehs[veh_id].proto_id].weapon_id, faction_id);
        int armor_strat = arm_strat(VehPrototypes[Vehs[veh_id].proto_id].armor_id, faction_id);
        int morale = morale_veh(veh_id, 0, 0) + 6;
        PlayersData[faction_id].mil_strength_1 += (armor_strat + weapon_strat) * morale / 8;
        if (weapon_strat > armor_strat || Vehs[veh_id].order != 2) {
            int value = morale * weapon_strat / 8;
            PlayersData[faction_id].mil_strength_2 += value;
            if (region >= 0) {
                PlayersData[faction_id].region_force_rating[region] += (uint16_t)value;
            }
        }
        uint32_t target_mode = Vehs[veh_id].state & 0x60000;
        if (target_mode == 0x40000) {
            if (on_map(PlayersData[faction_id].x_target, PlayersData[faction_id].y_target)
                && region == region_at(PlayersData[faction_id].x_target,
                                       PlayersData[faction_id].y_target)) {
                PlayersData[faction_id].unk_71++;
                if (plan == 11) {
                    PlayersData[faction_id].unk_72 |= 2;
                } else if (armor_strat > weapon_strat) {
                    PlayersData[faction_id].unk_72 |= 1;
                }
            } else {
                Vehs[veh_id].state |= 0x20000;
                attackers++;
            }
        } else if (target_mode == 0x60000) {
            attackers++;
        }
    }

    PlayersData[faction_id].unk_70 += (transport_cap * 0x800 + transports) * 0x800;
    int rival_transports = 0;
    for (i = 1; i < MaxPlayerNum; i++) {
        if (i != faction_id) {
            uint32_t treaty = PlayersData[faction_id].diplo_treaties[i];
            if ((treaty & DTREATY_COMMLINK) && !(treaty & DTREATY_PACT)) {
                int value = PlayersData[i].unk_70 / 0x400000;
                if (rival_transports <= value) {
                    rival_transports = value;
                }
            }
        }
    }
    PlayersData[faction_id].unk_64 += rival_transports * 0x800;
    if (transports > 1) {
        for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
            if (Vehs[veh_id].faction_id == faction_id) {
                int proto_id = Vehs[veh_id].proto_id;
                if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA
                    && VehPrototypes[proto_id].plan == 7
                    && (proto_id == BSC_UNITY_FOIL
                        || PlayersData[faction_id].proto_id_active[proto_id] >= 2)) {
                    Vehs[veh_id].state |= 0x20000000;
                    break;
                }
            }
        }
    }
    // Regions with three or more formers and bases release them from base duty.
    for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        if (Vehs[veh_id].faction_id == faction_id && VehPrototypes[Vehs[veh_id].proto_id].plan == 9) {
            int region = region_at(Vehs[veh_id].x, Vehs[veh_id].y);
            if (region < 0x40 && formers[region] >= 3
                && PlayersData[faction_id].region_total_bases[region] >= 3) {
                Vehs[veh_id].flags |= 0x800;
                if (Vehs[veh_id].home_base_id >= 0) {
                    Bases[Vehs[veh_id].home_base_id].state &= ~0x20;
                }
                formers[region] = 0;
            }
        }
    }

    // Keep or give up the current attack target.
    int abandon = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF) & self_bit;
    int target_region = -1;
    if (PlayersData[faction_id].x_target >= 0) {
        int base_id = base_at(PlayersData[faction_id].x_target, PlayersData[faction_id].y_target);
        target_region = region_at(PlayersData[faction_id].x_target, PlayersData[faction_id].y_target);
        if (base_id >= 0 && Bases[base_id].faction_id_current == faction_id) {
            int top = stack_fix(veh_at(Bases[base_id].x, Bases[base_id].y));
            if (top >= 0) {
                int count = stack_check(top, 0xB, 0x60000, 0x40000, -1);
                int limit = range(PlayersData[faction_id].region_total_bases[target_region] * 2 / 3, 3, 8);
                if (count - bit_count(PlayersData[faction_id].unk_72) >= limit) {
                    abandon = 1;
                }
                if (PlayersData[faction_id].region_base_plan[target_region] == 2
                    || (PlayersData[faction_id].unk_77[target_region] & 0x4200)) {
                    abandon = 1;
                }
                if (PlayersData[faction_id].region_base_plan[target_region] > 2
                    && stack_check(top, 2, 7, -1, -1)) {
                    if (count >= PlayersData[faction_id].unk_70 / 0x400000 * 2) {
                        abandon = 1;
                    }
                    if (stack_check(top, 2, 7, -1, -1) > 1
                        && count >= stack_check(top, 8, -1, -1, -1) + stack_check(top, 3, 0, -1, -1)) {
                        abandon = 1;
                    }
                }
            }
            if (!abandon) {
                if (PlayersData[faction_id].region_base_plan[target_region] > 2) {
                    if (PlayersData[faction_id].region_base_plan[target_region] == 8
                        || !is_coast(Bases[base_id].x, Bases[base_id].y, 0)) {
                        abandon = 1;
                    } else {
                        PlayersData[faction_id].flags |= 0x800;
                    }
                } else {
                    PlayersData[faction_id].flags &= ~0x800;
                }
            }
        } else {
            abandon = 1;
        }
    }
    if (abandon) {
        for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
            if (Vehs[veh_id].faction_id == faction_id && (Vehs[veh_id].state & 0x40000)) {
                Vehs[veh_id].state |= 0x20000;
            }
        }
        PlayersData[faction_id].y_target = -1;
        PlayersData[faction_id].x_target = -1;
        PlayersData[faction_id].unk_72 = 0;
        PlayersData[faction_id].unk_71 = 0;
    }

    // Plan every region from the balance of forces there.
    int best_score = 0;
    int best_region = -1;
    int region;
    for (region = 1; region < 128; region++) {
        PlayersData[faction_id].region_total_offensive_vehs[region] = 0;
        if (bad_reg(region) && !PlayersData[faction_id].region_total_bases[region]) {
            PlayersData[faction_id].region_base_plan[region] = 8;
            continue;
        }
        int old_plan = PlayersData[faction_id].region_base_plan[region];
        int threat = 0;
        int strong = 0;
        int weak = 0;
        int total_combat = 0;
        int total_bases = 0;
        int occupied = 0;
        int contact = 0;
        int wars = 0;
        int hostile = 0;
        int my_combat = PlayersData[faction_id].region_total_combat_vehs[region];
        int my_force = PlayersData[faction_id].region_force_rating[region];
        int my_bases = PlayersData[faction_id].region_total_bases[region];
        for (int other = 1; other < MaxPlayerNum; other++) {
            int their_force = PlayersData[other].region_force_rating[region];
            int their_combat = PlayersData[other].region_total_combat_vehs[region];
            int their_bases = PlayersData[other].region_total_bases[region];
            if (PlayersData[faction_id].diplo_treaties[other] & DTREATY_PACT) {
                their_combat *= 2;
                their_bases *= 2;
            }
            total_combat += their_combat;
            total_bases += their_bases;
            if (their_bases) {
                occupied++;
                PlayersData[faction_id].unk_77[region] |= (faction_id != other) ? 0x20 : 0x10;
                if (PlayersData[faction_id].diplo_treaties[other] & DTREATY_VENDETTA) {
                    PlayersData[faction_id].unk_77[region] |= 0x40;
                }
            } else if (!their_combat || my_force > their_force * 5) {
                continue;
            }
            if (faction_id == other) {
                continue;
            }
            if (PlayersData[faction_id].diplo_treaties[other] & DTREATY_PACT) {
                continue;
            }
            if (!(PlayersData[faction_id].diplo_treaties[other] & DTREATY_COMMLINK)) {
                continue;
            }
            int reach;
            int path;
            if (GameDataStateStorage.fields_.turn_current_num_ > 1
                && (Continents[region].tile_count >= 500
                    || PlayersData[other].region_total_bases[region] * my_bases >= 50)
                && ((Continents[region].tile_count < 1000)
                    ? (GameDataStateStorage.fields_.turn_current_num_ + faction_id) & 7
                    : (GameDataStateStorage.fields_.turn_current_num_ + faction_id * 2) & 0xF)) {
                if (their_bases && PlayersData[faction_id].region_base_plan[region] > 2) {
                    path = 0;
                    reach = 0;
                } else {
                    path = 1;
                    reach = 1;
                }
            } else if (!their_bases || (reach = contiguous(faction_id, other, region, 0)) != 0) {
                reach = 1;
                path = 1;
            } else if (contiguous(faction_id, other, region, 1)) {
                path = 1;
            } else {
                path = 0;
            }
            uint32_t treaty = PlayersData[faction_id].diplo_treaties[other];
            if (treaty & 0xC10) {
                if (path) {
                    contact++;
                }
                if (reach && ++wars > 1) {
                    PlayersData[faction_id].unk_77[region] |= 0x200;
                }
                if ((my_force <= their_combat || my_force <= their_force)
                    && my_combat <= their_force && my_bases && path) {
                    strong++;
                } else {
                    weak++;
                }
                if (their_force >= my_force * 3 / 2 && my_bases > their_bases && path) {
                    PlayersData[faction_id].unk_77[region] |= 0x4000;
                }
                if (((1 << other) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
                    && (reach || path)) {
                    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_;
                         base_id++) {
                        if (Bases[base_id].faction_id_current == other
                            && Bases[base_id].faction_id_former == faction_id
                            && region_at(Bases[base_id].x, Bases[base_id].y) == region) {
                            PlayersData[faction_id].unk_77[region] |= 0x400;
                            break;
                        }
                    }
                }
            } else if (my_bases && (treaty & 0x20) && their_force > my_combat) {
                int level;
                if ((my_force > their_combat && their_bases && their_force < my_force * 2) || !path) {
                    level = 2;
                } else {
                    level = 1;
                }
                threat |= level;
            }
            if (!(PlayersData[faction_id].diplo_treaties[other] & 3)
                || ((wants_to_attack(faction_id, other, 0) || wants_to_attack(other, faction_id, 0))
                    && path)) {
                hostile = 1;
            }
        }
        uint8_t sites = PlayersData[faction_id].unk_82[region];
        int pressure = (GameDataStateStorage.fields_.turn_current_num_ > 75) + 3
            + (MapArea > 3200) + occupied
            + great_satan(GameDataStateStorage.fields_.faction_rankings_[7], 0)
            + ((sites < 4) - ((GameDataStateStorage.fields_.game_state_ >> 7) & 2))
            + (PlayersData[faction_id].current_num_bases > 4);
        int settle = 0;
        if (my_bases) {
            int full = (sites < 2 && PlayersData[faction_id].region_total_bases[region]) ? 1 : 0;
            int good_tiles = PlayersData[faction_id].unk_80[region];
            int free_tiles = good_tiles - PlayersData[faction_id].unk_81[region];
            full |= (PlayersData[faction_id].unk_77[region] & 8) | (my_bases >= 20) | (free_tiles < 4);
            // BUG(original): adds faction 0's count for the region.
            int grow = ((PlayersData[faction_id].ai_growth - PlayersData[faction_id].ai_wealth + pressure)
                            * my_bases * 2
                        > PlayersData[0].unk_80[region] + good_tiles
                        && free_tiles / ((my_bases + 1) / 2) + sites < pressure * 2) ? 1 : 0;
            settle = full | grow;
        }
        if (hostile) {
            PlayersData[faction_id].unk_77[region] |= 4;
        }
        if (settle) {
            PlayersData[faction_id].unk_77[region] |= 1;
        }
        PlayersData[faction_id].region_base_plan[region] = (uint8_t)((settle == 0) + 7);
        if (!contact && PlayersData[faction_id].region_total_bases[region]) {
            PlayersData[faction_id].unk_77[region] |= 0x100;
        } else {
            if (weak) {
                threat = 0;
                PlayersData[faction_id].region_base_plan[region] = 0;
            } else if (threat & 2) {
                PlayersData[faction_id].region_base_plan[region] = 0;
            }
            if (strong || (threat & 1)) {
                PlayersData[faction_id].region_base_plan[region] = 2;
            }
        }
        if (region >= 0x40) {
            PlayersData[faction_id].region_base_plan[region] = 7;
            continue;
        }
        if (!abandon && region == target_region && wars >= 2) {
            for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
                if (Vehs[veh_id].faction_id == faction_id && (Vehs[veh_id].state & 0x40000)) {
                    Vehs[veh_id].state |= 0x20000;
                }
            }
            PlayersData[faction_id].y_target = -1;
            PlayersData[faction_id].x_target = -1;
            PlayersData[faction_id].unk_72 = 0;
            PlayersData[faction_id].unk_71 = 0;
            target_region = -1;
        }
        if (PlayersData[faction_id].x_target < 0 && !abandon
            && !(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
            if (PlayersData[faction_id].region_base_plan[region] < 2 && wars < 2
                && !(PlayersData[faction_id].unk_77[region] & 0x4000)) {
                int score = PlayersData[faction_id].region_total_bases[region] + 1000;
                if (score > best_score) {
                    best_score = score;
                    best_region = region;
                    int target_id = PlayersData[faction_id].base_id_atk_target;
                    if (target_id >= 0
                        && (PlayersData[faction_id].diplo_treaties[Bases[target_id].faction_id_current]
                            & DTREATY_VENDETTA)
                        && region_at(Bases[target_id].x, Bases[target_id].y) == region) {
                        best_score = 0;
                    }
                }
            } else if (PlayersData[faction_id].region_base_plan[region] == 7
                       && PlayersData[faction_id].region_total_bases[region] > best_score) {
                best_score = PlayersData[faction_id].region_total_bases[region];
                best_region = region;
            }
        }
        // Raise a land bridge from the region's best shore towards another region.
        if ((!(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
             || (faction_id == ConsoleGlobal.faction_id_
                 && (GameDataStateStorage.fields_.game_preferences_ & PREF_AUTO_FORMER_RAISE_LWR_TERRAIN)))
            && PlayersData[faction_id].region_total_bases[region]
            && (PlayersData[faction_id].region_base_plan[region] == 7
                || (PlayersData[faction_id].region_base_plan[region] == 8
                    && Continents[region].tile_count < 150))) {
            // stand-in: forces the image's out-of-line abstract_at calls.
            uint8_t (__cdecl *const abstract_at_call)(int, int) = abstract_at;
            int bridge_score = 0;
            int bridge_x;
            int bridge_y;
            int site_x;
            int site_y;
            for (int ay = 0; ay < (int)MapAbstractLatBounds; ay++) {
                for (int ax = ay & 1; ax < MapAbstractLongBounds; ax += 2) {
                    if (abstract_at_call(ax, ay) != region) {
                        continue;
                    }
                    int score = 0;
                    int found = -1;
                    for (int dir = 0; dir < 8; dir++) {
                        int nx = ax + VehMoveDeltaX[dir];
                        if (!(MapIsFlat & 1)) {
                            if (nx < 0) {
                                nx += MapAbstractLongBounds;
                            } else if (nx >= MapAbstractLongBounds) {
                                nx -= MapAbstractLongBounds;
                            }
                        }
                        int ny = ay + VehMoveDeltaY[dir];
                        if (nx < 0 || nx >= MapAbstractLongBounds || ny < 0 || ny >= (int)MapAbstractLatBounds) {
                            continue;
                        }
                        int other_region = abstract_at_call(nx, ny);
                        if (other_region == region) {
                            score--;
                            continue;
                        }
                        score += 2;
                        for (int k = 0; k < 8; k++) {
                            int tx = wrap_map_x(ax * 5 + VehMoveDeltaX[k]);
                            int ty = ay * 5 + VehMoveDeltaY[k];
                            if (!on_map(tx, ty) || is_ocean(tx, ty)) {
                                continue;
                            }
                            int steps;
                            for (steps = 1; steps < 100; steps++) {
                                int sx = wrap_map_x(tx + VehMoveDeltaX[dir]);
                                int sy = ty + VehMoveDeltaY[dir];
                                if (!on_map(sx, sy) || is_ocean(sx, sy)) {
                                    break;
                                }
                                tx = sx;
                                ty = sy;
                            }
                            if (steps < 100) {
                                if (base_who(tx, ty) < 0) {
                                    found = dir;
                                    score += 4;
                                    site_x = tx;
                                    site_y = ty;
                                    if (other_region && Continents[other_region].tile_count >= 40) {
                                        score += 2;
                                    }
                                }
                                break;
                            }
                        }
                    }
                    if (score > bridge_score && found >= 0) {
                        bridge_score = score;
                        bridge_x = site_x;
                        bridge_y = site_y;
                    }
                }
            }
            if (bridge_score > 0
                && terraform_cost(bridge_x, bridge_y, faction_id)
                    <= PlayersData[faction_id].energy_reserves / 8) {
                add_goal(faction_id, AI_GOAL_LANDING_SITE, 2, bridge_x, bridge_y, -1);
            }
        }
        if (!(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
            bases_reset(region, faction_id, PlayersData[faction_id].region_base_plan[region] == old_plan);
        }
    }

    // New attack target: the own base nearest an enemy base in the best region.
    if (best_region > 0 && best_region < 0x3F && !abandon
        && !(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
        && !(PlayersData[faction_id].flags & 0x20000000)) {
        int enemy_score = -1;
        int enemy = -1;
        for (i = 1; i < MaxPlayerNum; i++) {
            if (i != faction_id) {
                uint32_t treaty = PlayersData[faction_id].diplo_treaties[i];
                if ((treaty & 0xC14)
                    && ((treaty & 0x810)
                        || (attackers <= 3 && PlayersData[i].region_total_bases[best_region]))) {
                    int score = ((treaty & DTREATY_VENDETTA) << 7)
                        + PlayersData[i].region_total_bases[best_region];
                    if (score > enemy_score) {
                        enemy_score = score;
                        enemy = i;
                    }
                }
            }
        }
        if (enemy > 0) {
            int coast_only = (enemy_score == 0x800);
            int best_dist = 9999;
            int best_id = -1;
            for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                if (Bases[base_id].faction_id_current != faction_id) {
                    continue;
                }
                int x = Bases[base_id].x;
                int y = Bases[base_id].y;
                if (is_ocean(x, y) || region_at(x, y) != best_region) {
                    continue;
                }
                if (coast_only && !is_coast(x, y, 0)) {
                    continue;
                }
                int target_id = PlayersData[faction_id].base_id_atk_target;
                if (target_id >= 0
                    && (PlayersData[faction_id].diplo_treaties[Bases[target_id].faction_id_current] & 0x810)
                    && region_at(Bases[target_id].x, Bases[target_id].y) == best_region) {
                    BaseFindDist = vector_dist(Bases[base_id].x, Bases[base_id].y,
                                               Bases[target_id].x, Bases[target_id].y);
                } else if (base_find(Bases[base_id].x, Bases[base_id].y, enemy, best_region, -1, -1) < 0) {
                    base_find(Bases[base_id].x, Bases[base_id].y, enemy);
                    BaseFindDist *= 2;
                }
                if (BaseFindDist < best_dist) {
                    best_dist = BaseFindDist;
                    best_id = base_id;
                }
            }
            if (best_id >= 0) {
                PlayersData[faction_id].x_target = Bases[best_id].x;
                PlayersData[faction_id].y_target = Bases[best_id].y;
                if (coast_only) {
                    PlayersData[faction_id].flags |= 0x800;
                } else {
                    PlayersData[faction_id].flags &= ~0x800;
                }
                for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
                    if (Vehs[veh_id].faction_id != faction_id || (Vehs[veh_id].state & 0x60000)
                        || VehPrototypes[Vehs[veh_id].proto_id].plan > 2
                        || Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad
                        || has_abil(Vehs[veh_id].proto_id, ABL_DROP_POD)
                        || region_at(Vehs[veh_id].x, Vehs[veh_id].y) != best_region
                        || (Vehs[veh_id].order && Vehs[veh_id].order != 0x18)) {
                        continue;
                    }
                    base_find(Vehs[veh_id].x, Vehs[veh_id].y, enemy);
                    if (BaseFindDist <= vector_dist(Vehs[veh_id].x, Vehs[veh_id].y,
                                                    PlayersData[faction_id].x_target,
                                                    PlayersData[faction_id].y_target)) {
                        continue;
                    }
                    int weapon_strat = weap_strat(VehPrototypes[Vehs[veh_id].proto_id].weapon_id, faction_id);
                    int armor_strat = arm_strat(VehPrototypes[Vehs[veh_id].proto_id].armor_id, faction_id);
                    if (weapon_strat < armor_strat
                        && ((PlayersData[faction_id].unk_72 & 1) || coast_only)) {
                        continue;
                    }
                    Vehs[veh_id].state = (Vehs[veh_id].state & ~0x20000) | 0x40000;
                    PlayersData[faction_id].unk_71++;
                    if (weapon_strat < armor_strat) {
                        PlayersData[faction_id].unk_72 |= 1;
                    }
                    if (PlayersData[faction_id].unk_71 >= 8
                        || (coast_only
                            && PlayersData[faction_id].unk_71
                                >= PlayersData[faction_id].unk_70 * 2 / 0x400000)) {
                        break;
                    }
                }
            }
        }
    }

    if (!GameDataStateStorage.fields_.turn_current_num_ || !faction_id) {
        return;
    }
    // Send units to the faction's goals: the retry pass, then everyone.
    int assigned[75];
    for (i = 0; i < 75; i++) {
        assigned[i] = 0;
    }
    for (veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; veh_id--) {
        if (Vehs[veh_id].faction_id == faction_id && Vehs[veh_id].move_to_ai_type == '1') {
            Vehs[veh_id].move_to_ai_type = '2';
        }
    }
    for (int pass = 0; pass < 2; pass++) {
        for (veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; veh_id--) {
            if (veh_id >= GameDataStateStorage.fields_.vehicle_current_count_) {
                continue;
            }
            if (Vehs[veh_id].faction_id != faction_id) {
                continue;
            }
            if (!pass && Vehs[veh_id].move_to_ai_type != '2') {
                continue;
            }
            int x = Vehs[veh_id].x;
            int y = Vehs[veh_id].y;
            Map *sq = map_loc(x, y);
            int region = sq->region;
            int plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
            if (pass && Vehs[veh_id].move_to_ai_type == '1') {
                if (plan == 9) {
                    continue;
                }
                if (PlayersData[faction_id].region_base_plan[region] <= 2 && plan <= 2) {
                    continue;
                }
            }
            Vehs[veh_id].state &= ~0x40000000;
            if (Vehs[veh_id].order >= 4 && Vehs[veh_id].order < 0x18) {
                continue;
            }
            if (self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) {
                if (!(Vehs[veh_id].state & 0x200)) {
                    continue;
                }
                if (VehPrototypes[Vehs[veh_id].proto_id].plan == 9 && Vehs[veh_id].order_auto_type) {
                    continue;
                }
                if ((Vehs[veh_id].state & 0x1000200) == 0x1000200) {
                    continue;
                }
                if ((Vehs[veh_id].state & 0x2000200) == 0x2000200
                    && Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count) {
                    continue;
                }
            }
            if (Vehs[veh_id].state & 0x10000000) {
                plan = 2;
                PlayersData[faction_id].region_total_offensive_vehs[region]++;
            } else if (region < 0x40) {
                int per_base = 2 - PlayersData[faction_id].ai_fight;
                if (PlayersData[faction_id].region_base_plan[region] == 8) {
                    per_base += 2;
                } else if (PlayersData[faction_id].region_base_plan[region] == 2) {
                    per_base = 1;
                }
                if (PlayersData[faction_id].flags & 0x40000000) {
                    per_base = 1;
                }
                int wanted = (PlayersData[faction_id].region_total_bases[region] + per_base - 1) / per_base;
                if (wanted && plan <= 1
                    && (PlayersData[faction_id].region_base_plan[region] > 2
                        || (base_owner_of(sq) == faction_id
                            && (PlayersData[faction_id].region_base_plan[region] == 2
                                || (PlayersData[faction_id].unk_77[region] & 0x4000))
                            && stack_check(veh_id, 0xB, 0x10000000, 0x10000000, -1) < 1))
                    && !(Vehs[veh_id].state & 0x60000)
                    && PlayersData[faction_id].region_total_offensive_vehs[region] < wanted) {
                    plan = 2;
                    PlayersData[faction_id].region_total_offensive_vehs[region]++;
                    Vehs[veh_id].state |= 0x10000000;
                }
            }
            uint8_t triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
            if (triad != TRIAD_AIR) {
                BOOL ocean = is_ocean(x, y);
                BOOL sea = (triad == TRIAD_SEA);
                if (ocean != sea) {
                    if (base_who(x, y) < 0) {
                        continue;
                    }
                    // BUG(original): compares this tile's own altitude, never a neighbour's.
                    for (int dir = 0; dir < 8; dir++) {
                        int nx = wrap_map_x(VehMoveDeltaX[dir] + x);
                        int ny = VehMoveDeltaY[dir] + y;
                        if (on_map(nx, ny) && is_ocean(x, y) == sea) {
                            x = nx;
                            y = ny;
                            region = region_at(x, y);
                            break;
                        }
                    }
                }
            }
            if (plan == 1) {
                if (PlayersData[faction_id].region_base_plan[region] == 2) {
                    plan = 2;
                } else {
                    plan = 0;
                    if (PlayersData[faction_id].region_base_plan[region] == 8) {
                        plan = 2;
                    }
                }
            }
            if ((Vehs[veh_id].state & 0x60000) == 0x40000 && PlayersData[faction_id].x_target >= 0) {
                go_to(veh_id, '0', PlayersData[faction_id].x_target, PlayersData[faction_id].y_target);
                continue;
            }
            if (plan == 7 && stack_check(veh_id, 8, -1, -1, -1) < veh_cargo(veh_id)) {
                continue;
            }
            if (Vehs[veh_id].move_to_ai_type == 'F' && plan == 2) {
                continue;
            }
            int retreat = 0;
            if ((Vehs[veh_id].state & 0x60000) == 0x60000) {
                plan = 0;
                retreat = 1;
            }
            if (Vehs[veh_id].order == 2 && !retreat && base_who(x, y) >= 0) {
                int artifacts = stack_check(veh_id, 2, 12, -1, -1);
                if (stack_check(veh_id, 1, -1, -1, -1) - artifacts <= 1) {
                    continue;
                }
                if (plan == 2 || plan == 3 || plan == 1) {
                    int base_id_here = base_at(x, y);
                    if (stack_check(veh_id, 0x10, -1, -1, -1) < garrison_check(base_id_here)) {
                        continue;
                    }
                }
            }
            if (plan == 9 && (Vehs[veh_id].flags & 0x800)) {
                continue;
            }
            int best_cost = 99999;
            int best_goal = -1;
            for (int goal_id = 0; goal_id < 75; goal_id++) {
                Goal &goal = PlayersData[faction_id].goals[goal_id];
                if (goal.priority <= 0 || goal.type < 0) {
                    continue;
                }
                if ((goal.type & 0xF) != plan
                    && (retreat || goal.type != AI_GOAL_SCOUT || plan > 3 || Vehs[veh_id].order == 2)) {
                    continue;
                }
                if (!on_map(goal.x, goal.y)) {
                    goal.x = -1;
                    goal.y = -1;
                    continue;
                }
                Map *goal_sq = map_loc(goal.x, goal.y);
                if (region != goal_sq->region) {
                    continue;
                }
                int proto_id = Vehs[veh_id].proto_id;
                int goal_triad = Chassis[VehPrototypes[proto_id].chassis_id].triad;
                if (goal_triad == TRIAD_AIR) {
                    continue;
                }
                if (((goal_sq->climate & ~0x1F) < ALT_BIT_SHORE_LINE) != (goal_triad == TRIAD_SEA)) {
                    continue;
                }
                if (VehPrototypes[proto_id].unk_1 == 0x15) {
                    continue;
                }
                if (!plan) {
                    if (goal_sq->bit & BIT_MONOLITH) {
                        if (!want_monolith(veh_id) || retreat || assigned[goal_id]) {
                            continue;
                        }
                    }
                } else if (plan == 8) {
                    if (base_territory(faction_id, goal.x, goal.y) >= 0) {
                        del_site(faction_id, AI_GOAL_COLONIZE, goal.x, goal.y, 0);
                        continue;
                    }
                } else if (plan == 3) {
                    if (has_abil(proto_id, ABL_EMPATHIC)) {
                        continue;
                    }
                }
                int goal_base = base_at(goal.x, goal.y);
                int cost = vector_dist(x, y, goal.x, goal.y) << 8;
                int taken = 0;
                if (!retreat
                    && (goal_base < 0 || Bases[goal_base].faction_id_current != faction_id
                        || !Bases[goal_base].unk_1)) {
                    taken = assigned[goal_id];
                }
                int left = goal.priority - taken;
                if (left <= 0) {
                    continue;
                }
                if (retreat && base_who(goal.x, goal.y) < 0) {
                    continue;
                }
                if (goal.type == AI_GOAL_SCOUT && plan != 3) {
                    cost *= 2;
                    if (Vehs[veh_id].state & 0x10000000) {
                        continue;
                    }
                    if (base_who(goal.x, goal.y) < 0) {
                        continue;
                    }
                }
                if (taken && (goal.type == AI_GOAL_SCOUT || goal.type == AI_GOAL_COLONIZE)) {
                    continue;
                }
                if (cost > 6 / (taken + 1)
                               * Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].speed * left
                               - taken + 4
                    && PlayersData[faction_id].region_base_plan[region] != plan
                    && goal.type != AI_GOAL_SCOUT
                    && (taken || goal.type != AI_GOAL_TERRAFORM_LAND)) {
                    continue;
                }
                if (goal.type == AI_GOAL_COLONIZE) {
                    left *= range(site_at(goal.x, goal.y, 0), 1, 15);
                }
                if (Vehs[veh_id].state & 0x10000000) {
                    int other = veh_at(goal.x, goal.y);
                    if (other >= 0
                        && stack_check(other, 0xB, 0x10000000, 0x10000000, faction_id) > (cost == 0)) {
                        continue;
                    }
                }
                if (goal.type == AI_GOAL_TERRAFORM_LAND) {
                    int goal_home = goal.base_id;
                    if (goal_home >= 0 && (Bases[goal_home].state & 0x20)
                        && Vehs[veh_id].home_base_id != goal_home) {
                        cost *= 3;
                    }
                    int closer = 0;
                    for (int other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_;
                         other_id++) {
                        if (Vehs[other_id].faction_id == faction_id
                            && VehPrototypes[Vehs[other_id].proto_id].plan == 9
                            && other_id != veh_id) {
                            int dist = vector_dist(Vehs[other_id].x, Vehs[other_id].y, goal.x, goal.y) << 8;
                            if (goal_home >= 0 && (Bases[goal_home].state & 0x20)
                                && Vehs[other_id].home_base_id != goal_home) {
                                dist *= 3;
                            }
                            if (dist < cost) {
                                closer = 1;
                                break;
                            }
                        }
                    }
                    if (closer) {
                        continue;
                    }
                    if (assigned[goal_id] && (goal_home < 0 || Vehs[veh_id].home_base_id != goal_home)) {
                        continue;
                    }
                }
                int weighted = cost;
                int owner = whose_territory(faction_id, goal.x, goal.y, 0, 0);
                if (owner >= 0 && owner != faction_id) {
                    uint32_t treaty = PlayersData[faction_id].diplo_treaties[owner];
                    if (!(treaty & 0xC10)) {
                        if (treaty & 0x100000) {
                            continue;
                        }
                        if (!(treaty & DTREATY_PACT)) {
                            if (treaty & 2) {
                                continue;
                            }
                            weighted = cost * 2;
                        }
                    }
                }
                if (goal.type == AI_GOAL_PRIORITY_COMBAT) {
                    if (!has_abil(Vehs[veh_id].proto_id, ABL_ARTILLERY)
                        || Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad) {
                        continue;
                    }
                    weighted /= 2;
                }
                if (weighted / left < best_cost) {
                    best_cost = weighted / left;
                    best_goal = goal_id;
                }
            }
            if (best_goal < 0) {
                continue;
            }
            if (Vehs[veh_id].move_to_ai_type < '3' || Vehs[veh_id].move_to_ai_type > '5') {
                assigned[best_goal]++;
            }
            int goal_x = PlayersData[faction_id].goals[best_goal].x;
            int goal_y = PlayersData[faction_id].goals[best_goal].y;
            go_to(veh_id, '1', goal_x, goal_y);
            if (PlayersData[faction_id].goals[best_goal].type == AI_GOAL_SCOUT) {
                Vehs[veh_id].state |= 0x40000000;
            }
            if (PlayersData[faction_id].goals[best_goal].type == AI_GOAL_DEFEND) {
                int goal_base = base_at(goal_x, goal_y);
                if (goal_base >= 0 && Bases[goal_base].faction_id_current == faction_id) {
                    Bases[goal_base].state |= 0x8000000;
                    if (!(self_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
                        base_reset(goal_base, 0);
                    }
                }
            }
        }
    }
    wipe_goals(faction_id);
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
// ORIGINAL: 0x00564890 ?set_course@@YAXHDHH@Z 0x00564890-0x00564B82 BYTE_EXACT
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
// map_loc spelled through MapTiles; map.h's map_tiles() form flips the
// MapLongitude/tile-base register tie here.
static inline Map *set_course_tile(int x, int y) {
    return &MapTiles[(x >> 1) + y * MapLongitude];
}

// wrap_map_x reading MapLongitudeBounds at each use instead of through
// map.h's local; that local flips the image's lea operand order.
static inline int set_course_wrap_x(int x) {
    if (!(MapIsFlat & 1)) {
        if (x < 0) return x + MapLongitudeBounds;
        if (x >= MapLongitudeBounds) return x - MapLongitudeBounds;
    }
    return x;
}

void __cdecl set_course(int veh_id, char type, int x, int y) {
    Veh &veh = Vehs[veh_id];
    int veh_region = set_course_tile(veh.x, veh.y)->region;
    int base_id = base_at(veh.x, veh.y);
    Map *destination = set_course_tile(x, y);
    int dst_region = destination->region;
    if (veh_region == dst_region
        && (set_course_tile(veh.x, veh.y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
        go_to(veh_id, type, x, y);
        return;
    }
    int best_frontage = ((destination->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) ? 0 : -1;
    for (int i = 0; i < 21; i++) {   // the destination's own base radius
        int x_radius = set_course_wrap_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius)) {
            continue;
        }
        Map *anchorage = set_course_tile(x_radius, y_radius);
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
            Map *shore = set_course_tile(x_adj, y_adj);
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
        if (x < 0) {
            x += MapLongitudeBounds;
        } else if (x >= MapLongitudeBounds) {
            x -= MapLongitudeBounds;
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
                int mine_count = 0;
                int solar_count = 0;
                if (Rules.tgl_nutrient_effect_with_mine < 0) {
                    mine_count = 1;
                }
                int base_x = Bases[base_id].x;
                int base_y = Bases[base_id].y;
                for (int i = 0; i < 21; i++) {
                    int x_radius = terraform_xrange(base_x + RadiusOffsetX[i]);
                    int y_radius = base_y + RadiusOffsetY[i];
                    if (!on_map(x_radius, y_radius)) {
                        continue;
                    }
                    Map *radius_tile = map_loc(x_radius, y_radius);
                    if ((radius_tile->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
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
                // BUG(original): a precedence slip, `?:` binds last, so this always holds.
                if ((!crop || crop >= has_farm) ? 3 : 2) {
                    order = ORDER_MINE;
                }
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

    if (base_id < 0) {
        const int cost_def = VehPrototypes[Vehs[veh_id_def].proto_id].cost;
        // The rest of the stack counts at one fifth; the defender itself counts in full.
        odds = ((stack_check(veh_id_def, 7, -1, -1, -1) - cost_def) / 5 + cost_def) * odds;
        if (stack_check(veh_id_def, 2, PLAN_ALIEN_ARTIFACT, -1, -1)) {
            odds = odds * 3 / 2;
        }
    } else {
        odds = VehPrototypes[Vehs[veh_id_def].proto_id].cost * odds;
    }
    odds /= VehPrototypes[Vehs[veh_id_atk].proto_id].cost;

    if (VehPrototypes[Vehs[veh_id_def].proto_id].plan >= PLAN_NAVAL_TRANSPORT) {
        odds *= 2;   // it cannot fight back
    }
    if (plan_atk <= PLAN_COMBAT && !region_plan && triad_atk == TRIAD_LAND) {
        odds = odds * 3 / 2;
    }
    if (base_id >= 0 || at_goal(faction_id, AI_GOAL_ATTACK, x_def, y_def)) {
        if ((triad_atk == TRIAD_SEA) == (is_ocean(x_def, y_def) != 0) && triad_atk != TRIAD_AIR) {
            odds *= 2;   // the attacker's own domain, and this returns before the next rule
        } else if (base_id >= 0) {
            if (stack_check(veh_id_def, 1, -1, -1, -1)
                    - stack_check(veh_id_def, 19, -1, -1, -1) <= 1) {
                odds *= 2;   // one real defender or none
            }
        }
    }
    if (Vehs[veh_id_def].flags & VFLAG_IS_OBJECTIVE) {
        odds *= 4;
    }

    if (triad_atk == TRIAD_AIR) {
        if (is_proto_missile(Vehs[veh_id_atk].proto_id)
            && is_ocean(x_def, y_def) && base_id < 0) {
            odds *= 2;
        }
        if (stack_check(veh_id_def, 2, PLAN_TERRAFORMING, -1, -1)) {
            if (!is_proto_missile(Vehs[veh_id_atk].proto_id)) {
                odds *= 2;
            }
            if (is_ocean(x_def, y_def) && (PlayersData[faction_id].flags & 0x80000)) {
                odds *= 2;   // unnamed player flag; PlayerFlagsBitfield skips this bit
            }
            // Two separate tests in the original, not an else-if; `order` cannot be both.
            if (Vehs[veh_id_def].order == ORDER_TERRAFORM_UP) {
                odds *= 2;
            }
            if (Vehs[veh_id_def].order == ORDER_TERRAFORM_DOWN) {
                odds *= 2;
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
                    odds *= 2;   // somebody is already on the ground next to it
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
                    odds *= 2;
                    if (is_proto_missile(Vehs[veh_id_atk].proto_id)) {
                        odds *= 2;
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
            while (attackers > defenders && odds < 1000000) {
                odds *= 2;
                attackers -= defenders;
            }
        }
    }

    if (plan_atk == PLAN_AIR_SUPERIORITY) {
        const int proto_def = Vehs[veh_id_def].proto_id;
        if (Chassis[VehPrototypes[proto_def].chassis_id].triad == TRIAD_AIR) {
            return odds;   // an interceptor doing its job
        }
        if (!Weapon[VehPrototypes[proto_def].weapon_id].offense_rating) {
            return odds;   // nothing there to intercept anything
        }
        return odds / 2;
    }
    if (triad_atk == TRIAD_AIR && plan_atk == PLAN_OFFENSIVE
        && !is_proto_missile(Vehs[veh_id_atk].proto_id)) {
        if (region_plan != 2) {
            return odds / 3;
        }
        return odds * 2 / 3;
    }
    return odds;
}

/*
Purpose: Choose which base a native life form standing on the specified tile should head for.
// ORIGINAL: 0x005665D0 ?alien_base@@YAHHHH@Z 0x005665D0-0x00566802 BYTE_EXACT
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
// region_at spelled through MapTiles; map.h's map_tiles() form flips the
// MapLongitude/tile-base register tie here.
static inline int alien_region_at(int x, int y) {
    return MapTiles[(x >> 1) + y * MapLongitude].region;
}

int __cdecl alien_base(int veh_id, int x, int y) {
    int best_value = 9999;
    BaseFindDist = 9999;
    int base_id_best = -1;
    const int region = alien_region_at(x, y);
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Vehs[veh_id].proto_id == BSC_SEALURK) {
            if (region < MaxRegionLandNum && region != (int)alien_region_at(Bases[base_id].x, Bases[base_id].y)
                && !base_on_sea(base_id, region)) {
                continue;
            }
        } else if (region < MaxRegionLandNum && region != (int)alien_region_at(Bases[base_id].x, Bases[base_id].y)) {
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
                int visibility = map_loc(x2, y2)->visibility;
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
    if ((BaseFindDist >= 8 || region_at(x, y) != region) && !zoc_any(x, y, 0)
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
        int x_raw = x + VehMoveDeltaX[i];
        int x2 = (MapIsFlat & 1) ? x_raw
            : (x_raw < 0 ? x_raw + MapLongitudeBounds
               : (x_raw >= (int)MapLongitudeBounds ? x_raw - MapLongitudeBounds : x_raw));
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
int __cdecl air_power(int veh_id, int a2, int a3, int *a4, int *a5) {
    if (Vehs[veh_id].order == ORDER_MOVE_TO
        && (Vehs[veh_id].waypoint_x[0] != *a4 || Vehs[veh_id].waypoint_y[0] != *a5)) {
        if (a3 && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_AIR_SUPERIORITY) {
            return -1;
        }
        return -2;
    }
    int faction_id = Vehs[veh_id].faction_id;
    int here_region = region_at(*a4, *a5);
    int strike_range;
    int return_range;
    int i;
    int odds;
    int best;
    int target_x;
    int target_y;
    if (!Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range) {
        return_range = speed(veh_id, 0) * 6;
        strike_range = return_range;
    } else if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range == 1) {
        return_range = veh_moves(veh_id);
        strike_range = return_range;
        if (!Vehs[veh_id].dmg_incurred) {
            strike_range = speed(veh_id, 0) + return_range;
        }
    } else {
        int moves = veh_moves(veh_id);
        return_range = speed(veh_id, 0)
            * (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range
               - Vehs[veh_id].terraforming_turns - 1)
            + moves;
        strike_range = return_range;
    }
    strike_range /= Rules.move_rate_roads;
    return_range /= Rules.move_rate_roads;
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range
        && Vehs[veh_id].terraforming_turns) {
        action_home(veh_id, 4);
        if (Vehs[veh_id].order == ORDER_MOVE_TO) {
            return -2;
        }
    }
    uint32_t state = Vehs[veh_id].state;
    uint32_t automated = state & 0x200;
    if ((automated && Vehs[veh_id].order_auto_type == ORDERA_ON_ALERT)
        || (automated && Vehs[veh_id].order_auto_type == ORDERA_AUTOMATE_AIR_DEFENSE)) {
        if (Vehs[veh_id].dmg_incurred) {
            action_home(veh_id, 0);
            return -2;
        }
    } else if (automated && Vehs[veh_id].order_auto_type == ORDERA_BOMBING_RUN) {
        goto base_attack;
    } else {
        // Explore: the reach point with the most unseen tiles, else a random own base.
        if ((state & 0x4000)
            || (!is_ocean(*a4, *a5)
                && PlayersData[faction_id].region_base_plan[here_region] == 8)) {
            int half_range = return_range / 2;
            int explore_y = -1;
            int explore_x = -1;
            best = 0;
            for (int dir = 0; dir < 8; dir++) {
                int x = VehMoveDeltaX[dir] * half_range + *a4;
                if (!(MapIsFlat & 1)) {
                    if (x < 0) {
                        x += MapLongitudeBounds;
                    } else if (x >= MapLongitudeBounds) {
                        x -= MapLongitudeBounds;
                    }
                }
                int y = VehMoveDeltaY[dir] * half_range + *a5;
                if (!on_map(x, y)) {
                    continue;
                }
                int unseen = 0;
                for (int k = 0; k < 9; k++) {
                    int x_k = wrap_map_x(VehMoveDeltaX[k] + x);
                    int y_k = VehMoveDeltaY[k] + y;
                    if (on_map(x_k, y_k) && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)
                        && !(map_loc(x_k, y_k)->visibility & (1 << faction_id))) {
                        unseen++;
                    }
                }
                if (unseen > best) {
                    best = unseen;
                    explore_x = x;
                    explore_y = y;
                }
            }
            if (explore_x >= 0) {
                go_to(veh_id, 'a', explore_x, explore_y);
                return -2;
            }
            int pick = -1;
            int pick_value = -1;
            for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                if (Bases[i].faction_id_current == faction_id) {
                    int dist = alien_dist(*a4, *a5, Bases[i].x, Bases[i].y);
                    if (dist <= return_range) {
                        int value = bounded_crt_random(dist + 1);
                        if (value > pick_value) {
                            pick_value = value;
                            pick = i;
                        }
                    }
                }
            }
            if (pick >= 0) {
                go_to(veh_id, 'c', Bases[pick].x, Bases[pick].y);
                return -2;
            }
            if (Vehs[veh_id].state & 0x4000) {
                return -3;
            }
        }
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile) {
            int self_bit = 1 << faction_id;
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & self_bit) && IsMultiplayerNet) {
                return -3;
            }
            if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_TECTONIC_MISSILE) {
                best = 0;
                target_x = -1;
                target_y = -1;
                for (int goal_id = 0; goal_id < 75; goal_id++) {
                    if (PlayersData[faction_id].goals[goal_id].type == AI_GOAL_LANDING_SITE) {
                        int score = PlayersData[faction_id].goals[goal_id].priority * 8;
                        if (alien_dist(*a4, *a5, PlayersData[faction_id].goals[goal_id].x,
                                       PlayersData[faction_id].goals[goal_id].y)
                            > speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                            score /= 2;
                        }
                        if (anything_at(PlayersData[faction_id].goals[goal_id].x,
                                        PlayersData[faction_id].goals[goal_id].y) < 0
                            && !is_ocean(PlayersData[faction_id].goals[goal_id].x,
                                         PlayersData[faction_id].goals[goal_id].y)
                            && score > best) {
                            best = score;
                            target_x = PlayersData[faction_id].goals[goal_id].x;
                            target_y = PlayersData[faction_id].goals[goal_id].y;
                        }
                    }
                }
                for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                    if (Bases[i].faction_id_current == faction_id
                        || !(PlayersData[faction_id].diplo_treaties[Bases[i].faction_id_current]
                             & DTREATY_VENDETTA)
                        || is_ocean(Bases[i].x, Bases[i].y)) {
                        continue;
                    }
                    if (PlayersData[faction_id].region_total_bases[region_at(Bases[i].x, Bases[i].y)]) {
                        base_find(Bases[i].x, Bases[i].y, faction_id);
                        if (BaseFindDist < 10) {
                            continue;
                        }
                    }
                    int score = (Bases[i].population_size + 4) / 5 * 8;
                    int x = wrap_map_x(Bases[i].x + 4);
                    int y = Bases[i].y;
                    if (!on_map(x, y)) {
                        goto stockpile;
                    }
                    if (anything_at(x, y) >= 0) {
                        continue;
                    }
                    if (alien_dist(*a4, *a5, x, y)
                        > speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                        score /= 2;
                    }
                    if (score > best) {
                        best = score;
                        target_x = x;
                        target_y = y;
                    }
                }
                if (target_x < 0) {
                    goto stockpile;
                }
                if (alien_dist(*a4, *a5, target_x, target_y)
                    > speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                    int nearest = 9999;
                    // BUG(original): walks every base slot, not just the live bases.
                    for (i = 0; i < MaxBaseNum; i++) {
                        if (Bases[i].faction_id_current == faction_id
                            && alien_dist(*a4, *a5, Bases[i].x, Bases[i].y)
                                <= speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                            int dist = alien_dist(target_x, target_y, Bases[i].x, Bases[i].y);
                            if (dist < nearest) {
                                nearest = dist;
                                go_to(veh_id, 'M', Bases[i].x, Bases[i].y);
                            }
                        }
                    }
                    return -3;
                }
                go_to(veh_id, 'm', target_x, target_y);
                return -3;
            } else if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_FUNGAL_MISSILE) {
                best = 0;
                target_x = -1;
                target_y = -1;
                for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                    int owner = Bases[i].faction_id_current;
                    if (owner == faction_id
                        || !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_VENDETTA)
                        || PlayersData[owner].soc_effect_pending.values_[SocialEffect::PLANET] > 0) {
                        continue;
                    }
                    if (PlayersData[faction_id].region_total_bases[region_at(Bases[i].x, Bases[i].y)]) {
                        base_find(Bases[i].x, Bases[i].y, faction_id);
                        if (BaseFindDist < 5) {
                            continue;
                        }
                    }
                    int base_score = Bases[i].population_size << 3;
                    for (int k = 0; k < 21; k++) {
                        int x = RadiusOffsetX[k] + Bases[i].x;
                        if (!(MapIsFlat & 1)) {
                            if (x < 0) {
                                x += MapLongitudeBounds;
                            } else if (x >= MapLongitudeBounds) {
                                x -= MapLongitudeBounds;
                            }
                        }
                        int y = RadiusOffsetY[k] + Bases[i].y;
                        if (!on_map(x, y)) {
                            continue;
                        }
                        if ((map_loc(x, y)->bit & BIT_FUNGUS)
                            && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                            continue;
                        }
                        if (anything_at(x, y) >= 0) {
                            continue;
                        }
                        int score = base_score;
                        if (alien_dist(*a4, *a5, x, y)
                            > speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                            score = base_score / 2;
                        }
                        score += bit_count(map_loc(x, y)->bit & 0x1E8881C);
                        if (score > best) {
                            best = score;
                            target_x = x;
                            target_y = y;
                        }
                    }
                }
                if (target_x < 0) {
                    goto stockpile;
                }
                if (alien_dist(*a4, *a5, target_x, target_y)
                    > speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                    int nearest = 9999;
                    // BUG(original): walks every base slot, not just the live bases.
                    for (i = 0; i < MaxBaseNum; i++) {
                        if (Bases[i].faction_id_current == faction_id
                            && alien_dist(*a4, *a5, Bases[i].x, Bases[i].y)
                                <= speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                            int dist = alien_dist(target_x, target_y, Bases[i].x, Bases[i].y);
                            if (dist < nearest) {
                                nearest = dist;
                                go_to(veh_id, 'M', Bases[i].x, Bases[i].y);
                            }
                        }
                    }
                } else {
                    go_to(veh_id, 'm', target_x, target_y);
                }
            } else {
                int pods = 0;
                // BUG(original): steps the prototype id by faction_id from slot 64.
                for (int k = 0; k < 64; k++) {
                    int proto_id = k * faction_id + 64;
                    if ((VehPrototypes[proto_id].flags & 1) && has_abil(proto_id, ABL_DROP_POD)) {
                        pods += PlayersData[faction_id].proto_id_active[proto_id];
                    }
                }
                int threshold = VehPrototypes[Vehs[veh_id].proto_id].cost << 4;
                for (i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
                    int owner = Vehs[i].faction_id;
                    if (owner == faction_id
                        || !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_VENDETTA)) {
                        continue;
                    }
                    int base_id = base_at(Vehs[i].x, Vehs[i].y);
                    int close = 0;
                    int near_base;
                    if (Vehs[i].order == 0x15 && is_ocean(Vehs[i].x, Vehs[i].y)
                        && (near_base = base_find(Vehs[i].x, Vehs[i].y, faction_id)) >= 0
                        && alien_dist(Vehs[i].x, Vehs[i].y, Bases[near_base].x, Bases[near_base].y) <= 2) {
                        close = 1;
                    } else if ((Vehs[i].faction_id != faction_id || (Vehs[i].flags & 0x440) == 0x440)
                               && !(self_bit & Vehs[i].visibility)
                               && (base_id < 0
                                   || (Bases[base_id].faction_id_current != faction_id
                                       && !(Bases[base_id].visibility & self_bit)))) {
                        continue;
                    }
                    if (Chassis[VehPrototypes[Vehs[i].proto_id].chassis_id].triad == TRIAD_AIR
                        && base_id < 0) {
                        continue;
                    }
                    odds = compute_odds(battle_fight(veh_id, 0, Vehs[i].x, Vehs[i].y, 0, 0, 0),
                                        faction_id, veh_id, i, -2);
                    int weight;
                    if (base_id >= 0) {
                        if (!pods) {
                            for (int k = 0; k < 25; k++) {
                                int x = wrap_map_x(RadiusOffsetX[k] + Bases[base_id].x);
                                int y = RadiusOffsetY[k] + Bases[base_id].y;
                                if (on_map(x, y) && veh_who(x, y) == faction_id) {
                                    pods = 1;
                                    break;
                                }
                            }
                        }
                        if (pods) {
                            weight = 16;
                        } else {
                            odds /= 2;
                            weight = 4;
                        }
                    } else {
                        odds /= 2;
                        weight = 4;
                        if (!stack_check(i, 2, 7, -1, -1)
                            && (!stack_check(i, 2, 9, -1, -1) || !is_ocean(Vehs[i].x, Vehs[i].y))
                            && whose_territory(faction_id, Vehs[i].x, Vehs[i].y, 0, 0) != faction_id) {
                            weight = 2;
                            odds /= 2;
                        } else {
                            pods++;
                        }
                    }
                    int artifacts = stack_check(i, 0x13, -1, -1, -1);
                    int units = stack_check(i, 1, -1, -1, -1) - artifacts;
                    if (stack_check(i, 3, 2, -1, -1) >= units) {
                        odds *= 2;
                        weight *= 2;
                        pods++;
                    }
                    if (pods) {
                        if ((1 << owner) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) {
                            weight *= 2;
                        }
                        if (PlayersData[owner].ranking == 7) {
                            weight *= 2;
                        }
                    }
                    if (close) {
                        odds <<= 3;
                    }
                    int value = odds / 2;
                    if (alien_dist(*a4, *a5, Vehs[i].x, Vehs[i].y)
                        > speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                        weight /= 2;
                    }
                    if (value >= 8) {
                        value *= weight;
                        if (value >= threshold) {
                            threshold = value;
                            if (alien_dist(*a4, *a5, Vehs[i].x, Vehs[i].y)
                                > speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                                int nearest = 9999;
                                // BUG(original): walks every base slot, not just the live bases.
                                for (int base_slot = 0; base_slot < MaxBaseNum; base_slot++) {
                                    if (Bases[base_slot].faction_id_current == faction_id
                                        && alien_dist(*a4, *a5, Bases[base_slot].x, Bases[base_slot].y)
                                            <= speed(veh_id, 0) / Rules.move_rate_roads
                                                - Rules.move_rate_roads) {
                                        int dist = alien_dist(Vehs[i].x, Vehs[i].y,
                                                              Bases[base_slot].x, Bases[base_slot].y);
                                        if (dist < nearest) {
                                            nearest = dist;
                                            go_to(veh_id, 'M', Bases[base_slot].x, Bases[base_slot].y);
                                        }
                                    }
                                }
                            } else {
                                go_to(veh_id, 'm', Vehs[i].x, Vehs[i].y);
                            }
                        }
                    }
                }
            }
            // Launch: land on an own base on the way, or strike the target.
            if (Vehs[veh_id].order == ORDER_MOVE_TO) {
                if (base_who(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]) == faction_id) {
                    *a4 = Vehs[veh_id].waypoint_x[0];
                    *a5 = Vehs[veh_id].waypoint_y[0];
                    Vehs[veh_id].order = ORDER_NONE;
                    Vehs[veh_id].visibility = 0;
                    veh_drop(veh_lift(veh_id), *a4, *a5);
                    return -3;
                }
                Vehs[veh_id].order = ORDER_NONE;
                if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_TECTONIC_MISSILE) {
                    veh_drop(veh_lift(veh_id), target_x, target_y);
                    action_tectonic(veh_id, target_x, target_y);
                    return -5;
                }
                if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_FUNGAL_MISSILE) {
                    veh_drop(veh_lift(veh_id), target_x, target_y);
                    action_fungal(veh_id, target_x, target_y);
                    return -5;
                }
                for (int dir = 0; dir < 8; dir++) {
                    int x = wrap_map_x(VehMoveDeltaX[dir] + Vehs[veh_id].waypoint_x[0]);
                    int y = VehMoveDeltaY[dir] + Vehs[veh_id].waypoint_y[0];
                    if (on_map(x, y) && anything_at(x, y) < 0) {
                        *a4 = x;
                        *a5 = y;
                        Vehs[veh_id].visibility = 0;
                        veh_drop(veh_lift(veh_id), x, y);
                        Vehs[veh_id].facing = (int8_t)dir;
                        return dir ^ 4;
                    }
                }
                Vehs[veh_id].order = ORDER_NONE;
            }
        stockpile:
            // A missile in a stack of several moves to an own base holding fewer.
            int stacked = stack_check(veh_id, 0xD, -1, -1, -1);
            if (stacked > 1) {
                for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                    if (Bases[i].faction_id_current == faction_id
                        && stack_check(veh_at(Bases[i].x, Bases[i].y), 0xD, -1, -1, -1) < stacked
                        && alien_dist(*a4, *a5, Bases[i].x, Bases[i].y)
                            <= speed(veh_id, 0) / Rules.move_rate_roads - Rules.move_rate_roads) {
                        veh_drop(veh_lift(veh_id), Bases[i].x, Bases[i].y);
                        Vehs[veh_id].visibility = 0;
                        *a4 = Bases[i].x;
                        *a5 = Bases[i].y;
                        return -3;
                    }
                }
            }
            return -3;
        }
    }
    // Interception and escort.
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_AIR_SUPERIORITY
        || ((state & 0x200) && Vehs[veh_id].order_auto_type == ORDERA_AUTOMATE_AIR_DEFENSE)
        || !VehPrototypes[Vehs[veh_id].proto_id].plan) {
        int best_dist = 9999;
        int best_cost = 9999;
        int pick = -1;
        for (i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
            if (faction_id == Vehs[i].faction_id) {
                if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_AIR_SUPERIORITY
                    && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range != 1
                    && base_who(Vehs[i].x, Vehs[i].y) < 0
                    && !(map_loc(Vehs[i].x, Vehs[i].y)->bit & 0x40000)
                    && !stack_check(i, 6, 0x80, -1, -1)
                    && !veh_moves(i)
                    && !stack_check(i, 2, PLAN_AIR_SUPERIORITY, -1, -1)
                    && ((Chassis[VehPrototypes[Vehs[i].proto_id].chassis_id].triad == TRIAD_AIR
                         && VehPrototypes[Vehs[i].proto_id].plan != PLAN_AIR_SUPERIORITY)
                        || VehPrototypes[Vehs[i].proto_id].plan == 7)) {
                    int dist = alien_dist(*a4, *a5, Vehs[i].x, Vehs[i].y);
                    int cost = dist * 2 + 1;
                    if (speed(veh_id, 0) / Rules.move_rate_roads < dist) {
                        cost <<= 2;
                    }
                    cost /= stack_check(i, 1, -1, -1, -1) + 1;
                    if (cost <= best_cost) {
                        best_cost = cost;
                        best_dist = dist;
                        pick = i;
                    }
                }
                continue;
            }
            int close = 0;
            int near_base;
            if (Vehs[i].order == 0x15 && is_ocean(Vehs[i].x, Vehs[i].y)
                && (near_base = base_find(Vehs[i].x, Vehs[i].y, faction_id)) >= 0
                && alien_dist(Vehs[i].x, Vehs[i].y, Bases[near_base].x, Bases[near_base].y) <= 2) {
                close = 1;
            } else if ((Vehs[i].faction_id != faction_id || (Vehs[i].flags & 0x440) == 0x440)
                       && !(Vehs[i].visibility & (1 << faction_id))) {
                continue;
            }
            if (PlayersData[faction_id].diplo_treaties[Vehs[i].faction_id] & 7) {
                continue;
            }
            if (Chassis[VehPrototypes[Vehs[i].proto_id].chassis_id].triad != TRIAD_AIR) {
                if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_AIR_SUPERIORITY
                    && (Vehs[veh_id].state & 0x200)
                    && Vehs[veh_id].order_auto_type == ORDERA_ON_ALERT) {
                    continue;
                }
                odds = compute_odds(battle_fight(veh_id, 0, Vehs[i].x, Vehs[i].y, 0, 0, 0),
                                    faction_id, veh_id, i, -2);
                if (base_who(Vehs[i].x, Vehs[i].y) >= 0) {
                    if (!VehPrototypes[Vehs[veh_id].proto_id].plan) {
                        continue;
                    }
                    int artifacts = stack_check(i, 0x13, -1, -1, -1);
                    if (stack_check(i, 1, -1, -1, -1) - artifacts != 1) {
                        odds /= 2;
                    }
                }
                odds = odds * 2 / 3;
                if (close) {
                    odds <<= 3;
                }
                if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_AIR_SUPERIORITY
                    || !((Vehs[veh_id].state & 0x200)
                         && Vehs[veh_id].order_auto_type == ORDERA_AUTOMATE_AIR_DEFENSE)) {
                    odds /= 2;
                }
                if (odds < 8) {
                    continue;
                }
            } else {
                if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_AIR_SUPERIORITY
                    || base_who(Vehs[i].x, Vehs[i].y) >= 0
                    || (map_loc(Vehs[i].x, Vehs[i].y)->bit & 0x40000)) {
                    continue;
                }
            }
            if (PlayersData[faction_id].unk_67
                && PlayersData[faction_id].unk_105 < PlayersData[faction_id].unk_67 * 2
                && whose_territory(faction_id, Vehs[i].x, Vehs[i].y, 0, 0) != faction_id) {
                int near_id = base_find(Vehs[i].x, Vehs[i].y);
                if (near_id >= 0 && Bases[near_id].faction_id_current != faction_id) {
                    continue;
                }
            }
            if ((Vehs[veh_id].state & 0x200)
                && Vehs[veh_id].order_auto_type == ORDERA_AUTOMATE_AIR_DEFENSE) {
                if (is_ocean(Vehs[i].x, Vehs[i].y)) {
                    base_find(Vehs[i].x, Vehs[i].y, faction_id);
                    if (BaseFindDist > 2) {
                        continue;
                    }
                } else if (whose_territory(faction_id, Vehs[i].x, Vehs[i].y, 0, 0) != faction_id) {
                    continue;
                }
            }
            int dist = alien_dist(*a4, *a5, Vehs[i].x, Vehs[i].y);
            int cost = dist;
            if (strike_range / 2 >= dist) {
                int region = region_at(Vehs[i].x, Vehs[i].y);
                if (PlayersData[faction_id].region_base_plan[region] == 2) {
                    cost /= 2;
                } else if (!PlayersData[faction_id].region_total_bases[region]) {
                    cost *= 2;
                }
                if (close) {
                    cost = 0;
                    dist = 0;
                }
            } else {
                cost *= 2;
            }
            if (cost <= best_cost) {
                best_cost = cost;
                best_dist = dist;
                pick = i;
            }
        }
        if (pick >= 0) {
            if (Vehs[pick].faction_id == faction_id) {
                best_dist = (best_dist - 8) / 2;
            }
            if (strike_range / 2 >= best_dist) {
                go_to(veh_id, 'a', Vehs[pick].x, Vehs[pick].y);
                if (*a4 == Vehs[pick].x && *a5 == Vehs[pick].y) {
                    return -1;
                }
                return -2;
            }
            uint32_t on_alert = Vehs[veh_id].state & 0x200;
            if (!(on_alert && Vehs[veh_id].order_auto_type == ORDERA_ON_ALERT)
                && (VehPrototypes[Vehs[veh_id].proto_id].plan
                    || (on_alert && Vehs[veh_id].order_auto_type == ORDERA_AUTOMATE_AIR_DEFENSE))) {
                int escort_x = Vehs[pick].x;
                int escort_y = Vehs[pick].y;
                for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                    if (Bases[i].faction_id_current == faction_id
                        || (PlayersData[faction_id].diplo_treaties[Bases[i].faction_id_current]
                            & DTREATY_PACT)) {
                        int dist = alien_dist(escort_x, escort_y, Bases[i].x, Bases[i].y);
                        if (return_range >= dist && dist < best_dist) {
                            best_dist = dist;
                            go_to(veh_id, 'a', Bases[i].x, Bases[i].y);
                        }
                    }
                }
                if (Vehs[veh_id].order == ORDER_MOVE_TO) {
                    return -2;
                }
            }
        }
        if (VehPrototypes[Vehs[veh_id].proto_id].plan) {
            if (a2) {
                action_home(veh_id, 0);
                return -2;
            }
            return -3;
        }
    }
    automated = Vehs[veh_id].state & 0x200;
    if (automated && Vehs[veh_id].order_auto_type == ORDERA_ON_ALERT) {
        // Patrol: strike the best enemy unit around the patrol point.
        if ((!VehPrototypes[Vehs[veh_id].proto_id].plan || VehPrototypes[Vehs[veh_id].proto_id].plan == 6)
            && Vehs[veh_id].order != ORDER_MOVE_TO) {
            best = 0;
            int count = (strike_range > 1) ? 25 : 9;
            for (int k = 1; k < count; k++) {
                int x = wrap_map_x(RadiusOffsetX[k] + *a4);
                int y = RadiusOffsetY[k] + *a5;
                if (!on_map(x, y)) {
                    continue;
                }
                int target = stack_fix(veh_at(x, y));
                if (target < 0 || Vehs[target].faction_id == faction_id
                    || !(PlayersData[faction_id].diplo_treaties[Vehs[target].faction_id] & DTREATY_VENDETTA)
                    || Chassis[VehPrototypes[Vehs[target].proto_id].chassis_id].triad == TRIAD_AIR) {
                    continue;
                }
                uint8_t triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
                if (triad != TRIAD_AIR) {
                    if (!(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)
                        && !(map_loc(x, y)->visibility & (1 << faction_id))) {
                        continue;
                    }
                    if (base_who(x, y) < 0 && !(Vehs[target].visibility & (1 << faction_id))) {
                        continue;
                    }
                    if (is_ocean(x, y) != (triad == TRIAD_SEA)) {
                        continue;
                    }
                }
                int score = offense_proto(Vehs[target].proto_id, -1, 0);
                if (Chassis[VehPrototypes[Vehs[target].proto_id].chassis_id].triad == TRIAD_LAND) {
                    score *= 3;
                }
                if (is_ocean(x, y) && stack_check(target, 2, 9, -1, -1)) {
                    score *= 2;
                    if (PlayersData[faction_id].flags & 0x80000) {
                        score *= 2;
                    }
                    // BUG(original): doubles the stale odds, not the score.
                    if (Vehs[target].order == 0x14) {
                        odds *= 2;
                    }
                    if (Vehs[target].order == 0x15) {
                        odds *= 2;
                    }
                }
                if (score > best) {
                    best = score;
                    go_to(veh_id, 'L', x, y);
                }
            }
            if (Vehs[veh_id].order == ORDER_MOVE_TO) {
                return (a3 != 0) - 2;
            }
        }
        if (Vehs[veh_id].order != ORDER_MOVE_TO
            && (Vehs[veh_id].x != Vehs[veh_id].waypoint_x[1]
                || Vehs[veh_id].y != Vehs[veh_id].waypoint_y[1])) {
            go_to(veh_id, 'l', Vehs[veh_id].waypoint_x[1], Vehs[veh_id].waypoint_y[1]);
            return -2;
        }
        return -4;
    }
base_attack:
    if (!VehPrototypes[Vehs[veh_id].proto_id].plan
        || (automated && Vehs[veh_id].order_auto_type == ORDERA_BOMBING_RUN)) {
        target_x = -1;
        target_y = -1;
        best = 0;
        int risky_best;
        for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
            if (Bases[i].faction_id_current == faction_id
                || (PlayersData[faction_id].diplo_treaties[Bases[i].faction_id_current] & 7)) {
                continue;
            }
            if ((Vehs[veh_id].state & 0x200) && Vehs[veh_id].order_auto_type == ORDERA_BOMBING_RUN
                && (Bases[i].x != Vehs[veh_id].waypoint_x[1] || Bases[i].y != Vehs[veh_id].waypoint_y[1])) {
                continue;
            }
            int target = stack_fix(veh_at(Bases[i].x, Bases[i].y));
            if (target < 0) {
                continue;
            }
            if (!((Vehs[veh_id].state & 0x200) && Vehs[veh_id].order_auto_type == ORDERA_BOMBING_RUN)) {
                odds = compute_odds(battle_fight(veh_id, 0, Vehs[target].x, Vehs[target].y, 0, 0, 0),
                                    faction_id, veh_id, target, -2);
                if (base_who(Vehs[target].x, Vehs[target].y) < 0) {
                    odds /= 2;
                } else {
                    int artifacts = stack_check(target, 0x13, -1, -1, -1);
                    if (stack_check(target, 1, -1, -1, -1) - artifacts != 1) {
                        odds /= 2;
                    }
                }
                if (odds < 8) {
                    continue;
                }
            }
            int x = Bases[i].x;
            int y = Bases[i].y;
            uint8_t region = map_loc(x, y)->region;
            int score = (Bases[i].population_size + PlayersData[faction_id].region_total_bases[region] * 2) << 5;
            int own_nearby = 0;
            for (int k = 1; k < 9; k++) {
                int x_k = wrap_map_x(RadiusOffsetX[k] + x);
                int y_k = RadiusOffsetY[k] + y;
                if (on_map(x_k, y_k) && map_loc(x_k, y_k)->region == region
                    && veh_who(x_k, y_k) == faction_id) {
                    score = score * 2 + 1;
                    own_nearby = 1;
                    break;
                }
            }
            int risky = 0;
            if (!own_nearby
                && !((Vehs[veh_id].state & 0x200) && Vehs[veh_id].order_auto_type == ORDERA_BOMBING_RUN)) {
                if (PlayersData[faction_id].unk_105 < stack_check(veh_at(x, y), 1, -1, -1, -1) * 2) {
                    risky = 1;
                }
                if (!has_tech(Ability[abil_index(ABL_DROP_POD)].preq_tech, faction_id)) {
                    risky = 1;
                }
                if (risky) {
                    score /= 8;
                }
            }
            if (Bases[i].event & (1 << faction_id)) {
                score *= 4;
            }
            if (Bases[i].state & 8) {
                score *= 3;
            }
            if (Bases[i].state & 0x100000) {
                score *= 2;
            }
            if (i == PlayersData[faction_id].base_id_atk_target) {
                score *= 2;
            }
            int base_region = region_at(Bases[i].x, Bases[i].y);
            if (base_region == here_region) {
                score *= 2;
            }
            if (PlayersData[faction_id].region_base_plan[base_region] <= 2) {
                score *= 4;
            }
            int dist = alien_dist(*a4, *a5, Bases[i].x, Bases[i].y);
            score /= (dist - 1) / range(strike_range / 2, 1, 99) + 1;
            if (score >= best) {
                best = score;
                risky_best = risky;
                target_x = Bases[i].x;
                target_y = Bases[i].y;
            }
        }
        if (on_map(target_x, target_y)) {
            if (alien_dist(*a4, *a5, target_x, target_y) <= strike_range / 2) {
                if (!risky_best) {
                    go_to(veh_id, 'a', target_x, target_y);
                }
            } else {
                int nearest = 9999;
                for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                    if (Bases[i].faction_id_current == faction_id
                        || (PlayersData[faction_id].diplo_treaties[Bases[i].faction_id_current]
                            & DTREATY_PACT)) {
                        int dist = alien_dist(target_x, target_y, Bases[i].x, Bases[i].y);
                        if (return_range >= dist && dist < nearest) {
                            nearest = dist;
                            go_to(veh_id, 'a', Bases[i].x, Bases[i].y);
                        }
                    }
                }
            }
        }
        if (Vehs[veh_id].order == ORDER_MOVE_TO) {
            if (a3 && !((Vehs[veh_id].state & 0x200)
                        && Vehs[veh_id].order_auto_type == ORDERA_BOMBING_RUN)) {
                return -1;
            }
            return -2;
        }
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
                    || PlayersData[target_faction].ranking < PlayersData[faction_id].ranking
                    || !reputation(target_faction, faction_id))) {
                continue;
            }
            if (!at_climax(faction_id) || PlayersData[faction_id].ai_fight < 0
                || !(GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)) {
                if (PlayersData[faction_id].diplo_treaties[target_faction] & DTREATY_UNK_80000) {
                    if (PlayersData[faction_id].major_atrocities >= PlayersData[target_faction].major_atrocities
                        && PlayersData[faction_id].mil_strength_1 * 2 >= PlayersData[target_faction].mil_strength_1
                        && !at_climax(faction_id)) {
                        if (score < 4
                            && GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER]
                                <= GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]) {
                            continue;
                        }
                        if (PlayersData[faction_id].ranking > PlayersData[target_faction].ranking) {
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
                        if (PlayersData[faction_id].ranking > PlayersData[target_faction].ranking) {
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

#include "netdaemon.h"
#include <stdlib.h>

// Native 56FE51 passes this storage address to message_base. Only its
// initial NUL byte is established; the historical complete capacity,
// TU/name and indirect writes remain unknown. This is a start-only view.
// Extent: compiled sizeof 0x1; next mapped object 0x0093FC34, gap unreferenced.
// RECOVERY_OBJECT: v2 0x0093FC30 0x0093FC31 ?EnemyBuildBaseMessageText@@3PADA 0x0056FE51
char EnemyBuildBaseMessageText[1] = {0};

// Complete owner assembly. Native stack words are stored only on proven edges.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0056B5B0 0x00579208 0x00579238 0x00577DD3
// ORIGINAL: 0x0056B5B0 ?enemy_move@@YAHH@Z 0x0056B5B0-0x00579208 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:9d6ac074ba331b079a584bc73bfd0ce20699a9b4:src/unrecovered/0056b5b0.cpp is deleted seed history; its original identity, full span, claims and useful lessons are retained here through the source writer.", "Pinned instructions establish distinct raw route/damage flags, integer stack priority, saved versus live callback reads, and original unguarded/uninitialized error paths.", "Complete selected, strategic, nonsea, sea and tactical continuations execute once; helpers do not introduce new original identities.", "Vehicle, prototype, PlayerData and Base fields use their real source owners; original matching and full runtime restoration obligations remain."], "reason": "Complete typed per-vehicle enemy movement owner with one path-dependent raw frame and genuine product owners; superseded executable forwarding and scaffold are removed together."}
// kind game
int __cdecl enemy_move(int veh_id) {
    // One variable per original frame slot; the original reuses several of
    // them for unrelated roles, and some paths read a slot this call has not
    // written (original uninitialised reads).
    int tx, ty, faction_id, y, x, other_id, score, base_id, best_score, i, j;
    int region, dist, triad, k, keep_count, own_ground, plan, m, n;
    int home_base, tile_owner, home_dist, faction_bit, hint, owner, at_sea;
    int best_id, near_dist, best_dir, near_hostile, region_plan, stay, damage;
    int zoc_owner, assault, near_base, remote_own, alt, retreat, proto_id;
    int state_4000, defend, no_moves, zoc_moved, coast_goal, keep_wp;
    int coast_y[64];
    int coast_x[64];
    int air_result;
    int rad;
    int cand;
    // ---- chunk 0: 0x56b5b0-0x56d265 ----
    best_dir = -1;
    near_hostile = 0;
    keep_wp = 0;
    defend = 0;
    stay = 0;
    coast_goal = 0;
    no_moves = 0;
    if (veh_id < 0) {
        // Hint mode: the caller passes -(veh_id + 1); the image open-codes |veh_id + 1|.
        veh_id = (veh_id + 1 < 0) ? -(veh_id + 1) : veh_id + 1;
        hint = 1;
    } else {
        hint = 0;
    }
    remote_own = IsMultiplayerNet
        && Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_;
    do_all_non_input();
    if (!Vehs[veh_id].faction_id && !remote_own) {
        return alien_move(veh_id);
    }
    proto_id = Vehs[veh_id].proto_id;
    faction_id = Vehs[veh_id].faction_id;
    y = Vehs[veh_id].y;
    x = Vehs[veh_id].x;
    plan = VehPrototypes[proto_id].plan;
    triad = Chassis[VehPrototypes[proto_id].chassis_id].triad;
    alt = alt_at(x, y);
    at_sea = is_ocean(x, y);
    region = region_at(x, y);
    assault = (Vehs[veh_id].state & 0x60000) == 0x40000;
    retreat = (Vehs[veh_id].state & 0x60000) == 0x60000;
    k = plan == PLAN_RECONNAISANCE && has_abil(proto_id, 0x800);
    region_plan = PlayersData[faction_id].region_base_plan[region];
    {
        int dmg = Vehs[veh_id].dmg_incurred;
        if (dmg >= proto_power(veh_id) / 2) {
            damage = 4;
        } else if (dmg > proto_power(veh_id) / 3) {
            damage = 3;
        } else if (dmg > VehPrototypes[Vehs[veh_id].proto_id].reactor_id * 2) {
            damage = 2;
        } else {
            damage = dmg != 0;
        }
    }
    if (planet_buster(veh_id)) {
        damage = 0;
    }
    if (proto_id == 0x10 || proto_id == 0x11 || proto_id == 0x12) {
        damage = 0;
    }
    zoc_moved = zoc_move(x, y, faction_id);
    zoc_owner = zoc_veh(x, y, faction_id);
    near_base = base_find(x, y, -1, triad == TRIAD_LAND ? region : -1, -1, -1);
    near_dist = BaseFindDist;
    home_base = base_find(x, y, faction_id, triad == TRIAD_LAND ? region : -1, -1, -1);
    home_dist = BaseFindDist;
    tile_owner = whose_territory(faction_id, x, y, NULL, 0);
    own_ground = tile_owner == faction_id;
    state_4000 = Vehs[veh_id].state & 0x4000;
    if (hint) {
        goto L_57138d;
    }
    if ((Vehs[veh_id].flags & 0x440)
        && !((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << faction_id))) {
        veh_skip(veh_id);
        return 0;
    }
    if (!veh_moves(veh_id)) {
        no_moves = 1;
        goto L_574a7e;
    }
    if (Vehs[veh_id].order >= 4 && Vehs[veh_id].order < 24) {
        if (!zoc_owner
            || (zoc_owner > 1
                && (!((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << faction_id))
                    || !(PlayersData[faction_id].diplo_treaties[zoc_owner - 1] & 0x10)))) {
            goto L_578f46;
        }
        if ((Vehs[veh_id].state & 0x200)
            && (Vehs[veh_id].order_auto_type == 4 || Vehs[veh_id].order_auto_type == 5)) {
            goto L_56c2d1;
        }
        for (int d = 0; d < 8; d++) {
            tx = wrap_map_x(x + VehMoveDeltaX[d]);
            ty = y + VehMoveDeltaY[d];
            if (on_map(tx, ty)
                && (triad == TRIAD_AIR || is_ocean(tx, ty) == (triad == TRIAD_SEA))) {
                owner = anything_at(tx, ty);
                if ((owner < 0 || owner == faction_id
                     || (PlayersData[faction_id].diplo_treaties[owner] & 1))
                    && !zoc_veh(tx, ty, faction_id)
                    && (!zoc_moved || !zoc_move(tx, ty, faction_id))) {
                    best_dir = d;
                    goto L_578b26;
                }
            }
        }
        if (Vehs[veh_id].state & 0x200) {
            goto L_56c9bf; // image jumps to 0x56c9c2, one register reload past it
        }
    }
    if (zoc_owner > 1 && assault
        && (PlayersData[faction_id].diplo_treaties[zoc_owner - 1] & 0x10)) {
        assault = 0;
        Vehs[veh_id].state &= ~0x40000;
        if (x == PlayersData[faction_id].x_target && y == PlayersData[faction_id].y_target) {
            PlayersData[faction_id].x_target = -1;
            retreat = 1;
            for (other_id = veh_top(veh_id); other_id >= 0;
                 other_id = Vehs[other_id].next_veh_id_stack) {
                if (other_id == veh_id || (Vehs[other_id].state & 0x40000)) {
                    Vehs[other_id].state |= 0x60000;
                }
            }
        }
    }
    if (retreat && Vehs[veh_id].moves_expended >= Rules.move_rate_roads && !zoc_owner
        && !((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << faction_id))
        && stack_check(veh_id, 14, -1, -1, -1)) {
        veh_skip(veh_id);
        goto L_578f46;
    }
    if (assault && Vehs[veh_id].x == PlayersData[faction_id].x_target
        && Vehs[veh_id].y == PlayersData[faction_id].y_target) {
        defend = 1;
        stay = 1;
        goto L_57707f;
    }
    if ((Vehs[veh_id].state & 0x1000200) == 0x1000200
        || ((Vehs[veh_id].state & 0x2000200) == 0x2000200
            && Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count)) {
        goto L_578de9;
    }
    faction_bit = 1 << faction_id;
    if (!((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit)
        && (Vehs[veh_id].flags & 0x800)) {
        if (has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0], faction_id)) {
            Vehs[veh_id].order_auto_type = 2;
        }
        goto L_56c30f;
    }
    if (!(Vehs[veh_id].state & 0x200) || plan != PLAN_TERRAFORMING) {
        goto L_56c9e1;
    }
    if (Vehs[veh_id].order_auto_type == 3) {
        base_id = Vehs[veh_id].home_base_id;
        if (base_id >= 0) {
            goto L_572922;
        }
        base_id = home_base;
        if (base_id >= 0) {
            goto L_572922;
        }
        Vehs[veh_id].state &= ~0x200;
        veh_skip(veh_id);
        return 0;
    }
    if (Vehs[veh_id].order_auto_type == 4 || Vehs[veh_id].order_auto_type == 5) {
        goto L_56c6d3;
    }
    if (Vehs[veh_id].order_auto_type == 1) {
        goto L_56c2f0;
    }
    if (Vehs[veh_id].order_auto_type == 2) {
        goto L_56c2f4;
    }
    if (Vehs[veh_id].order_auto_type == 6) {
        goto L_56bf5f;
    }
    if (has_abil(Vehs[veh_id].proto_id, 0x2000) && !Vehs[veh_id].order_auto_type
        && (GameDataStateStorage.fields_.game_more_preferences_ & 8)) {
        goto L_56bf5f;
    }
    if (Vehs[veh_id].order_auto_type != 7) {
        goto L_56c9e1;
    }
    if (good_sensor(faction_id, x, y)) {
        Vehs[veh_id].order = ORDER_SENSOR_ARRAY;
        goto L_578f46;
    }
    tx = x;
    ty = y;
    if (Paths.sensors(faction_id, &tx, &ty)) {
        goto L_56c2d1;
    }
    if (tx == x && ty == y) {
        Vehs[veh_id].order = ORDER_SENSOR_ARRAY;
        goto L_578f46;
    }
    go_to(veh_id, 0x73, tx, ty);
    return 0;

L_56bf5f:
    if (!has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[triad == TRIAD_SEA], faction_id)
        && !has_abil(Vehs[veh_id].proto_id, 0x2000)
        && Vehs[veh_id].order_auto_type == 6) {
        goto L_56c2d1;
    }
    if (Vehs[veh_id].order == ORDER_MOVE_TO) {
        goto L_578f46;
    }
    if ((map_loc(x, y)->bit & BIT_FUNGUS) && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
        Vehs[veh_id].order = ORDER_REMOVE_FUNGUS;
        goto L_578f46;
    }
    if (triad == TRIAD_SEA && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
        goto L_57717f;
    }
    best_score = 99999;
    for (keep_count = 2; keep_count <= (Vehs[veh_id].order_auto_type == 6 ? 6 : 3); keep_count++) {
        if (Vehs[veh_id].order == ORDER_MOVE_TO) {
            goto L_578f46;
        }
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            if (Bases[base_id].faction_id_current == faction_id
                && (triad == TRIAD_SEA
                    ? base_on_sea(base_id, region)
                    : region_at(Bases[base_id].x, Bases[base_id].y) == region)) {
                for (int r = (keep_count == 2 ? 1 : RadiusRange[keep_count - 1]);
                     r < RadiusRange[keep_count]; r++) {
                    tx = wrap_map_x(Bases[base_id].x + RadiusOffsetX[r]);
                    ty = Bases[base_id].y + RadiusOffsetY[r];
                    if (on_map(tx, ty) && region_at(tx, ty) == region
                        && (map_loc(tx, ty)->bit & BIT_FUNGUS)
                        && (map_loc(tx, ty)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                        owner = anything_at(tx, ty);
                        // BUG(original): `dist` is tested before this scan ever
                        // writes it (uninitialised on the first hit).
                        if (owner < 0 || (owner == faction_id && !dist)) {
                            // stand-in: forces the image's out-of-line calls at this site.
                            int (__cdecl *const x_dist_call)(int, int) = x_dist;
                            int (__cdecl *const vector_dist_call)(int, int) = vector_dist;
                            dist = vector_dist_call(x_dist_call(x, tx), abs(y - ty));
                            if (!zoc_move(tx, ty, faction_id) && dist < best_score) {
                                best_score = dist;
                                go_to(veh_id, 0x66, tx, ty);
                            }
                        }
                    }
                }
            }
        }
    }
    if (Vehs[veh_id].order == ORDER_MOVE_TO) {
        goto L_578f46;
    }
    if (Vehs[veh_id].order_auto_type != 6) {
        goto L_56c9e1;
    }
L_56c2d1:
    Vehs[veh_id].order = ORDER_NONE;
    Vehs[veh_id].state &= ~0x200;
    return 0;

L_56c2f0:
    if (Vehs[veh_id].order_auto_type == 2) {
L_56c2f4:
        if (!has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0], faction_id)) {
            Vehs[veh_id].order_auto_type = 1;
        }
    }
L_56c30f:
    if (Vehs[veh_id].order == ORDER_ROAD_TO || Vehs[veh_id].order == ORDER_MAGTUBE_TO) {
        int b = base_at(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
        if (b >= 0 && Bases[b].faction_id_current == faction_id) {
            goto L_578f46;
        }
        Vehs[veh_id].order = ORDER_NONE;
    }
    // 0x56c35d: nearest reachable base for an auto road/magtube builder.
    best_score = 9999;
    j = (Vehs[veh_id].order_auto_type == 2) + ORDER_ROAD_TO;
    best_id = -1;
    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current == faction_id
            || ((PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current] & 1)
                && (near_dist || Bases[near_base].faction_id_current == faction_id))) {
            n = Bases[base_id].y;
            int bx = Bases[base_id].x;
            if (region_at(bx, n) == region) {
                int px = x;
                int py = y;
                m = 1;
                i = 0;
                dist = 0;
                do {
                    best_dir = Paths.find(px, py, bx, n, 0, -1,
                        (has_tech(Rules.tech_build_roads_fungus, faction_id)
                         || has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[0], faction_id))
                        ? 0 : 4, -1);
                    if (best_dir < 0) {
                        break;
                    }
                    px = wrap_map_x(px + VehMoveDeltaX[best_dir]);
                    py += VehMoveDeltaY[best_dir];
                    dist++;
                    if (px == bx && py == n) {
                        break;
                    }
                    if (base_who(px, py) < 0
                        && (!(map_loc(px, py)->bit & BIT_ROAD)
                            || (!(map_loc(px, py)->bit & BIT_MAGTUBE)
                                && Vehs[veh_id].order_auto_type == 2))) {
                        m = 0;
                    }
                    if (++i > 100) {
                        break;
                    }
                } while (m);
                if (!m) {
                    if (Bases[base_id].faction_id_current != faction_id) {
                        dist *= 2;
                    }
                    int o = whose_territory(faction_id, px, py, NULL, 0);
                    if (o >= 0 && o != faction_id
                        && !(PlayersData[faction_id].diplo_treaties[o] & 1)) {
                        dist += 100;
                    }
                    if (zoc_sea(px, py, faction_id)) {
                        dist += 10;
                    }
                    if (dist < best_score) {
                        best_score = dist;
                        best_id = base_id;
                    }
                }
            }
        }
    }
    if (best_id >= 0) {
        Vehs[veh_id].move_to_ai_type = 0x52;
        Vehs[veh_id].order = (int8_t)j;
        Vehs[veh_id].waypoint_x[0] = Bases[best_id].x;
        Vehs[veh_id].waypoint_y[0] = Bases[best_id].y;
        goto L_578f46;
    }
    if (!((uint8_t)faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
        && (Vehs[veh_id].flags & 0x800)) {
        goto L_56c9e1;
    }
    Vehs[veh_id].order = ORDER_NONE;
    Vehs[veh_id].state &= ~0x200;
    veh_skip(veh_id);
    if (faction_id == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.focus(x, y, faction_id);
        if (Vehs[veh_id].order_auto_type == 1) {
            X_pop("AUTOROADSDONE", NULL);
            return 0;
        }
        X_pop("AUTOTUBESDONE", NULL);
        return 0;
    }
    return 0; // image: jne 0x5791ff, the function's shared `return 0` tail

L_56c6d3:
    // Automatic farm/mine terraformer (order_auto_type 4 or 5).
    if ((map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF) {
        goto L_56c9bf;
    }
    own_ground = map_loc(x, y)->bit; // slot reused for the tile bits
    if (own_ground & BIT_MONOLITH) {
        goto L_56c9bf;
    }
    if (base_who(x, y) >= 0) {
        goto L_56c9bf;
    }
    if (own_ground & BIT_FUNGUS) {
        if (has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[at_sea], faction_id)) {
            Vehs[veh_id].order = ORDER_REMOVE_FUNGUS;
            goto L_578f46;
        }
    } else if (!at_sea && !(own_ground & BIT_FARM)
               && (map_loc(x, y)->val3 & 0xC0) < 0x80
               && ((map_loc(x, y)->climate & 0x18) < 0x10
                   // literal: `alt - 2` is tested `< 0` then `<= 1`
                   || (alt - 2 >= 0 && alt - 2 > 1))
               && has_tech(Terraforming[TERRA_FARM].prerequisite_tech[0], faction_id)) {
        Vehs[veh_id].order = ORDER_FARM;
        goto L_578f46;
    }
    if (!(map_loc(x, y)->bit & BIT_FUNGUS)
        || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF
        || has_tech(Rules.tech_improve_fungus_sqr, faction_id)) {
        if (Vehs[veh_id].order_auto_type == 4 && !(map_loc(x, y)->bit & BIT_SOLAR_TIDAL)
            && energy_yield(faction_id, -1, x, y, 1)
            && has_tech(Terraforming[TERRA_SOLAR_TIDAL].prerequisite_tech[at_sea], faction_id)) {
            Vehs[veh_id].order = ORDER_SOLAR_COLLECTOR;
            goto L_578f46;
        }
        if (Vehs[veh_id].order_auto_type == 5 && !(map_loc(x, y)->bit & BIT_MINE)
            && has_tech(Terraforming[TERRA_MINE].prerequisite_tech[at_sea], faction_id)) {
            Vehs[veh_id].order = ORDER_MINE;
            goto L_578f46;
        }
    }
    if (!(map_loc(x, y)->bit & BIT_ROAD) && !at_sea
        && (!(map_loc(x, y)->bit & BIT_FUNGUS)
            || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF
            || has_tech(Rules.tech_build_roads_fungus, faction_id))
        && has_tech(Terraforming[TERRA_ROAD].prerequisite_tech[0], faction_id)) {
        Vehs[veh_id].order = ORDER_ROAD;
        goto L_578f46;
    }
    if (!at_sea && !(map_loc(x, y)->bit & BIT_FARM)
        && (map_loc(x, y)->val3 & 0xC0) < 0x80
        && has_tech(Terraforming[TERRA_FARM].prerequisite_tech[0], faction_id)) {
        Vehs[veh_id].order = ORDER_FARM;
        goto L_578f46;
    }
L_56c9bf:
    Vehs[veh_id].order = ORDER_NONE;
    Vehs[veh_id].state &= ~0x200;
    return 0;

L_56c9e1:
    if (near_dist == 1 && Bases[near_base].faction_id_current != faction_id
        && triad != TRIAD_AIR
        && Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating
        && veh_who(Bases[near_base].x, Bases[near_base].y) < 0
        && !(PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 7)
        && is_ocean(Bases[near_base].x, Bases[near_base].y) == (triad == TRIAD_SEA)
        && plan <= PLAN_TERRAFORMING) {
        near_hostile = 1;
        damage = 0;
        Vehs[veh_id].order = ORDER_NONE;
    }
    if (near_base != home_base) {
        if ((PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 2)
            && !at_sea
            && !(PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 1)
            && Vehs[veh_id].order != ORDER_MOVE_TO
            && whose_territory(faction_id, x, y, NULL, 0) == Bases[near_base].faction_id_current
            && (int)PlayersData[faction_id].diplo_friction[Bases[near_base].faction_id_current] < 10
            && !(PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 0xC00)
            && ((Vehs[veh_id].faction_id == Bases[near_base].faction_id_current
                 && (Vehs[veh_id].flags & 0x440) != 0x440)
                || (Vehs[veh_id].visibility & (1 << Bases[near_base].faction_id_current)))) {
            if (home_base >= 0
                && region_at(Bases[home_base].x, Bases[home_base].y) == region) {
                go_to(veh_id, 0x77, Bases[home_base].x, Bases[home_base].y);
                goto L_57707f;
            }
            if (!Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad) {
                for (int d = 0; d < 8; d++) {
                    tx = xrange(x + VehMoveDeltaX[d]);
                    ty = y + VehMoveDeltaY[d];
                    if (on_map(tx, ty) && !is_ocean(tx, ty)) {
                        other_id = stack_fix(veh_at(tx, ty));
                        if (other_id >= 0 && Vehs[other_id].faction_id == faction_id
                            && stack_check(other_id, 8, -1, -1, -1) > 0) {
                            // image: jg 0x56feee, a shared `best_dir = esi; goto 578b26` tail
                            best_dir = d;
                            goto L_578b26;
                        }
                    }
                }
            }
        }
    }
    if ((Vehs[veh_id].state & 0x200) && Vehs[veh_id].order_auto_type == 0xB
        && triad != TRIAD_AIR) {
        stay = 1;
        goto L_57707f;
    }
    if (Vehs[veh_id].move_to_ai_type == 0x46) {
        if (Vehs[veh_id].order != ORDER_MOVE_TO) {
            Vehs[veh_id].move_to_ai_type = 0x2D;
        }
        if ((Vehs[veh_id].state & 0x100) && plan == PLAN_DEFENSIVE) {
            Vehs[veh_id].move_to_ai_type = 0x2D;
            if (Vehs[veh_id].home_base_id >= 0) {
                base_id = Vehs[veh_id].home_base_id;
                if (x != Bases[base_id].x || y != Bases[base_id].y) {
                    goto L_56cf7a;
                }
                Bases[base_id].state &= ~0x4000000;
            }
L_56cdc1:
            owner = tile_owner;
            if (((uint8_t)faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                || owner == faction_id || owner <= 0
                || !(PlayersData[faction_id].diplo_treaties[owner] & 8)) {
                goto L_56d7fc;
            }
            if ((Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating <= 2
                 && plan != PLAN_DEFENSIVE)
                || plan == PLAN_RECONNAISANCE
                || (PlayersData[faction_id].diplo_treaties[owner] & 0x40C30)
                || !(PlayersData[faction_id].diplo_treaties[owner] & 3)
                || PlayersData[owner].integrity_blemishes > 2) {
                goto L_56d7fc;
            }
            owner = tile_owner; // stored a second time in the image (0x56ce84)
            if (Vehs[veh_id].order == ORDER_MOVE_TO) {
                int wt = whose_territory(faction_id, Vehs[veh_id].waypoint_x[0],
                                         Vehs[veh_id].waypoint_y[0], NULL, 0);
                if (wt != faction_id && wt != owner && wt > 0 && plan <= PLAN_COMBAT && !triad
                    && (PlayersData[owner].ranking < PlayersData[faction_id].ranking
                        || PlayersData[wt].mil_strength_1
                           > (int)((~PlayersData[faction_id].diplo_treaties[owner] & 1) | 2)
                             * PlayersData[owner].mil_strength_1 / 2)
                    && (PlayersData[faction_id].diplo_treaties[wt] & 0x10)
                    && (PlayersData[owner].diplo_treaties[wt] & 0x10)) {
                    action_give(veh_id, owner);
                    return 1;
                }
            }
            goto L_56d762;
L_56cf7a:
            if (region_at(Bases[base_id].x, Bases[base_id].y) != region) {
                goto L_56cdc1;
            }
            go_to(veh_id, 0x46, Bases[base_id].x, Bases[base_id].y);
            goto L_57707f;
        }
    }
    if (plan == PLAN_ALIEN_ARTIFACT) {
        if (!(Vehs[veh_id].flags & 0x20)) {
            if (!home_dist) {
                if (remote_own) {
                    veh_skip(veh_id);
                    synch_veh(veh_id);
                    return 1;
                }
                if (((has_fac(FAC_NETWORK_NODE, home_base, 0) && !(Bases[home_base].state & 0x400))
                     || GameDataStateStorage.fields_.secret_project_.universal_translator() == home_base)
                    && study_artifact(veh_id)) {
                    return 1;
                }
            }
            if (!((uint8_t)faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                && !home_dist
                && Bases[home_base].queue_production_id[0] <= -0x46
                && Bases[home_base].minerals_accumulated >= 100) {
                Bases[home_base].minerals_accumulated
                    += VehPrototypes[Vehs[veh_id].proto_id].cost * 10 / 2;
                Bases[home_base].minerals_accumulated_2 = Bases[home_base].minerals_accumulated;
                Bases[home_base].production_id_last = Bases[home_base].queue_production_id[0];
                kill(veh_id);
                return 1;
            }
        }
        best_score = 9999;
        best_id = -1;
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            if (Bases[base_id].faction_id_current == faction_id
                && region_at(Bases[base_id].x, Bases[base_id].y) == region) {
                int sc = vector_dist(x_dist(x, Bases[base_id].x), abs(y - Bases[base_id].y));
                if (GameDataStateStorage.fields_.secret_project_.universal_translator() != base_id
                    && (Bases[base_id].queue_production_id[0] >= -0x46
                        || Bases[base_id].minerals_accumulated < 100
                        || ((uint8_t)faction_bit
                            & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_))) {
                    if (!has_fac(FAC_NETWORK_NODE, base_id, 0)) {
                        sc += 1000;
                    }
                    if ((Bases[base_id].state & 0x400)
                        || stack_check(veh_at(Bases[base_id].x, Bases[base_id].y), 2, 0xC, -1, -1)
                           > (x == Bases[base_id].x && y == Bases[base_id].y)) {
                        sc += 2000;
                    }
                }
                if (sc < best_score) {
                    best_score = sc;
                    best_id = base_id;
                }
            }
        }
        if (best_id >= 0) {
            go_to(veh_id, 0x74, Bases[best_id].x, Bases[best_id].y);
            goto L_578f46;
        }
        goto L_57707f;
    }
    // ---- chunk 1: 0x56d265-0x56ee44 ----
L_56d265:
    if (plan == 10) {
    // Plan 10 (terraformer-style economic unit): feed it into a base that is
    // building a secret project, else look for a site to work.
    if (!((uint8_t)faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
        best_score = 9999;
        best_id = -1;
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            if (Bases[base_id].faction_id_current == faction_id
                && map_loc(Bases[base_id].x, Bases[base_id].y)->region == region) {
                int d = (abs(y - Bases[base_id].y) + x_dist(x, Bases[base_id].x)) >> 1;
                if (Bases[base_id].queue_production_id[0] <= -70 && d < best_score) {
                    best_score = d;
                    best_id = base_id;
                    if (!d)
                        goto L_56d372;
                }
            }
        }
        if (best_id >= 0) {
            go_to(veh_id, 0x56, Bases[best_id].x, Bases[best_id].y);
            goto L_57707f;
L_56d372:
            Bases[base_id].minerals_accumulated += veh_cost(Vehs[veh_id].proto_id, -1, NULL) * 10;
            Bases[base_id].unused_surplus = Bases[base_id].minerals_accumulated;
            Bases[base_id].production_id_last = Bases[base_id].queue_production_id[0];
            kill(veh_id);
            return 1;
        }
    }
    if (Vehs[veh_id].order == 3) {
        switch (Vehs[veh_id].order_auto_type) {
        case 2:
            if (!(map_loc(x, y)->bit & 0x40) && alt > 2)
                goto L_56d46c;
            break;
        case 1:
            if ((~map_loc(x, y)->bit >> 4) & 1) {
L_56d46c:
                add_goal(faction_id, 9, 1, x, y, -1);
            }
            break;
        case 0:
            if (!(map_loc(x, y)->bit & 0x8000) && (map_loc(x, y)->val3 & 0xC0) < 0x80)
                goto L_56d46c;
            break;
        }
        goto L_578f46;
    }
    if (Vehs[veh_id].order == 0x18)
        goto L_57707f;
    best_score = 0;
    for (k = 0; k < 289; k++) {
        tx = xrange(x + RadiusOffsetX[k]);
        ty = y + RadiusOffsetY[k];
        if (!on_map(tx, ty) || map_loc(tx, ty)->region != region)
            continue;
        if (triad != 2 && ((map_loc(tx, ty)->climate & 0xE0) < 0x60) != (triad == 1))
            continue;
        if (!can_convoy(tx, ty))
            continue;
        if (!bonus_at(tx, ty, 0) && (map_loc(tx, ty)->val3 & 0xC0) < 0x80
            && (map_loc(tx, ty)->climate & 0xE0) < 0xC0)
            continue;
        if (whose_territory(faction_id, tx, ty, NULL, 0) != faction_id)
            continue;
        base_find(tx, ty, faction_id);
        if (BaseFindDist <= 2)
            continue;
        for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
            if (Vehs[other_id].faction_id == faction_id
                && VehPrototypes[Vehs[other_id].proto_id].plan == 10
                && other_id != veh_id) {
                if (Vehs[other_id].x == tx && Vehs[other_id].y == ty && Vehs[other_id].order == 3)
                    goto L_56d72d;
                // BUG(original): compares this unit's own waypoint, not other_id's.
                if (Vehs[other_id].order == 0x18 && Vehs[veh_id].waypoint_x[0] == tx
                    && Vehs[veh_id].waypoint_y[0] == ty)
                    goto L_56d72d;
            }
        }
        {
            int d = (abs(y - ty) + x_dist(x, tx)) >> 1;
            int yield = 1;
            best_id = 0;
            for (int rt = 0; rt < 3; rt++) {
                int v = resource_yield(rt, faction_id, Vehs[veh_id].home_base_id, tx, ty) * 2;
                if (rt == 1)
                    v++;
                if (v >= yield) {
                    yield = v;
                    best_id = rt;
                }
            }
            int value = (yield << 9) - d;
            if (value > best_score) {
                best_score = value;
                go_to(veh_id, 0x76, tx, ty);
                if (!k)
                    goto L_56d74a;
            }
        }
L_56d72d:
        ;
    }
    goto L_57707f;
L_56d74a:
    Vehs[veh_id].order = 3;
    Vehs[veh_id].order_auto_type = (uint8_t)best_id;
    goto L_578f46;
    }
    goto L_56cdc1;

L_56d762:
    // Unit stands in owner's territory (continuation of the chunk 0 test).
    if (PlayersData[owner].region_base_plan[region] == 2 && plan <= 2
        && (PlayersData[faction_id].diplo_treaties[owner] & 1)) {
        action_give(veh_id, owner);
        return 1;
    }
    if (!PlayersData[faction_id].region_total_bases[region]
        && PlayersData[owner].region_total_bases[region]
        && !triad
        && ((PlayersData[owner].region_base_plan[region] == 2
             && ((PlayersData[faction_id].diplo_treaties[owner] & 1)
                 || PlayersData[owner].ranking < 7))
            || (PlayersData[owner].region_base_plan[region] != 8
                && (PlayersData[faction_id].diplo_treaties[owner] & 1)))) {
        action_give(veh_id, owner);
        return 1;
    }

L_56d7fc:
    // Damaged unit: try to heal / retreat.
    if (damage && (!retreat || damage > 3)) {
        if (!near_dist || (triad == 2 && (map_loc(x, y)->bit & 0x40000))) {
            if ((!(Vehs[veh_id].state & 8) && !(Bases[near_base].state & 0x200000))
                || (damage >= 4 && veh_moves(veh_id) <= Rules.move_rate_roads)) {
                Vehs[veh_id].move_to_ai_type = 0x64;
                stay = 1;
                defend = 1;
                goto L_571384;
            }
        }
        if (has_project(22, faction_id) && !near_hostile && triad != 2
            && !Vehs[veh_id].moves_expended && (!zoc_moved || damage > 3)
            && !(Vehs[veh_id].state & 8)) {
            Vehs[veh_id].move_to_ai_type = 0x64;
            stay = 1;
            if (triad == 1)
                goto L_574a88;
            goto L_57707f;
        }
        if (home_dist == 1 && (damage >= 4 || !(Bases[home_base].state & 0x200000))) {
            go_to(veh_id, 0x64, Bases[home_base].x, Bases[home_base].y);
            goto L_578f46;
        }
        if ((near_dist <= (triad + 1) * (triad + 1) * damage * 2
             && !(Bases[near_base].state & 0x200000)
             && get_there(veh_id, Bases[near_base].x, Bases[near_base].y)
             && near_base == home_base)
            || (PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 1)) {
            go_to(veh_id, 0x64, Bases[near_base].x, Bases[near_base].y);
            if (triad == 1)
                goto L_574a88;
            goto L_57707f;
        }
        if (damage >= 3
            && Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating
            && !near_hostile && near_dist && !triad) {
            for (rad = 0; rad < 9; rad++) {
                tx = xrange(x + RadiusOffsetX[rad]);
                ty = y + RadiusOffsetY[rad];
                if (on_map(tx, ty) && (map_loc(tx, ty)->climate & 0xE0) >= 0x60
                    && (map_loc(tx, ty)->bit & 0x2000)
                    && (!ExpansionEnabled
                        || (Vehs[veh_id].proto_id != 0x10 && Vehs[veh_id].proto_id != 0x11
                            && Vehs[veh_id].proto_id != 0x12))) {
                    owner = anything_at(tx, ty);
                    if (owner < 0 || owner == faction_id
                        || (PlayersData[faction_id].diplo_treaties[owner] & 1))
                        goto L_56dc57;
                }
            }
        }
        if (damage >= 4 && !near_hostile && near_dist && !Vehs[veh_id].moves_expended
            && !Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range
            && veh_moves(veh_id) == Rules.move_rate_roads && !zoc_moved
            && !(Vehs[veh_id].state & 8)
            && speed_proto(Vehs[veh_id].proto_id) > Rules.move_rate_roads
            && Vehs[veh_id].proto_id != 0x10 && Vehs[veh_id].proto_id != 0x11
            && Vehs[veh_id].proto_id != 0x12) {
            Vehs[veh_id].move_to_ai_type = 0x64;
            if (triad == 1) {
                stay = 1;
                goto L_574a88;
            }
            goto L_578b34;
L_56dc57:
            // Reached only from the radius scan above.
            if (!rad) {
                stay = 1;
                Vehs[veh_id].move_to_ai_type = 0x64;
                Vehs[veh_id].dmg_incurred = 0;
                goto L_57708a;
            }
            go_to(veh_id, 0x64, tx, ty);
            goto L_57708a;
        }
        if (damage >= 3 && !near_hostile && !triad && near_dist
            && !veh_cargo(veh_id) && !(Vehs[veh_id].state & 8)) {
            for (rad = 0; rad < 8; rad++) {
                tx = xrange(x + VehMoveDeltaX[rad]);
                ty = y + VehMoveDeltaY[rad];
                if (on_map(tx, ty) && (map_loc(tx, ty)->climate & 0xE0) < 0x60) {
                    for (other_id = stack_fix(veh_at(tx, ty)); other_id >= 0;
                         other_id = other_id < 0 ? other_id : Vehs[other_id].next_veh_id_stack) {
                        if (Vehs[other_id].faction_id == faction_id
                            && VehPrototypes[Vehs[other_id].proto_id].plan == 7
                            && stack_check(other_id, 8, -1, -1, -1) > 0)
                            goto L_56de10;
                    }
                }
            }
        }
        if (damage > 2 && at_sea && !triad) {
            if (!near_dist) {
                Vehs[veh_id].move_to_ai_type = 0x64;
                goto L_578b34;
L_56de10:
                // Reached only from the adjacent-transport scan above.
                best_dir = rad;
                Vehs[veh_id].move_to_ai_type = 0x64;
                goto L_578b26;
            }
            {
                int healthy = 1;
                for (other_id = veh_top(veh_id); other_id >= 0;
                     other_id = other_id < 0 ? other_id : Vehs[other_id].next_veh_id_stack) {
                    if (VehPrototypes[Vehs[other_id].proto_id].plan == 7
                        && Vehs[other_id].dmg_incurred > proto_power(other_id) / 3)
                        healthy = 0;
                }
                if (healthy) {
                    Vehs[veh_id].move_to_ai_type = 0x64;
                    goto L_578b34;
                }
            }
        }
        if (own_ground
            || !(PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 2)
            || (PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 1)
            || reputation(Bases[near_base].faction_id_current, faction_id) >= 6) {
            if (!Vehs[veh_id].moves_expended && !zoc_owner
                && !Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad
                && !(Vehs[veh_id].state & 8)
                && (damage > 1 || has_project(22, faction_id))) {
                Vehs[veh_id].move_to_ai_type = 0x64;
                goto L_578b34;
            }
        }
        if (damage >= 3 && triad) {
            action_home(veh_id, 2);
            if (Vehs[veh_id].order == 0x18) {
                Vehs[veh_id].move_to_ai_type = 0x64;
                if (triad == 1)
                    goto L_574a88;
                goto L_57707f;
            }
        }
        if (home_base >= 0 && ((Vehs[veh_id].state & 8) || zoc_owner) && damage >= 4
            && get_there(veh_id, Bases[home_base].x, Bases[home_base].y)) {
            go_to(veh_id, 0x64, Bases[home_base].x, Bases[home_base].y);
            if (triad == 1)
                goto L_574a88;
            goto L_57707f;
        }
    }

    // Plan 11 (probe team).
    if (plan == 11) {
        if (remote_own) {
            Vehs[veh_id].order = 0;
            Vehs[veh_id].state &= ~0x200;
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
            NetDaemonNet.unlock_veh();
            return 1;
        }
        if (!state_4000) {
            keep_count = triad == 1;
            if (at_sea == keep_count) {
                for (n = 0; n < 21; n++) {
                    tx = xrange(x + RadiusOffsetX[n]);
                    ty = y + RadiusOffsetY[n];
                    if (on_map(tx, ty)
                        && (triad == 2 || ((map_loc(tx, ty)->climate & 0xE0) < 0x60) == keep_count)
                        && map_loc(tx, ty)->region == region
                        && (other_id = stack_fix(veh_at(tx, ty))) >= 0
                        && Vehs[other_id].faction_id != faction_id
                        && (stack_check(other_id, 2, 11, -1, -1)
                            || (base_who(tx, ty) < 0 && probe(veh_id, -1, other_id, 0)))) {
                        uint32_t treaty =
                            PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id];
                        if ((!(treaty & 1) || !(treaty & 0x2000000))
                            && ((treaty & 0x30)
                                || whose_territory(faction_id, tx, ty, NULL, 0) == faction_id))
                            goto L_56e434;
                    }
                }
                best_score = 99999;
                for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                    if (map_loc(Bases[base_id].x, Bases[base_id].y)->region != region)
                        continue;
                    cand = ((abs(y - Bases[base_id].y) + x_dist(x, Bases[base_id].x)) >> 1) * 16;
                    if (Bases[base_id].faction_id_current != faction_id) {
                        if (has_project(15, Bases[base_id].faction_id_current))
                            continue;
                        if (Bases[base_id].state & 0x40) {
                            uint32_t treaty = PlayersData[faction_id]
                                .diplo_treaties[Bases[base_id].faction_id_current];
                            if (treaty & 7) {
                                if (!(treaty & 0x20))
                                    continue;
                                if (PlayersData[faction_id].region_total_bases[region]
                                    && PlayersData[Bases[base_id].faction_id_current].region_force_rating[region]
                                       > PlayersData[faction_id].region_force_rating[region] / 2
                                         + PlayersData[faction_id].region_total_combat_vehs[region])
                                    continue;
                            }
                            cand *= 2;
                        }
                        {
                            uint32_t treaty = PlayersData[faction_id]
                                .diplo_treaties[Bases[base_id].faction_id_current];
                            if (treaty & 0x10)
                                cand /= 2;
                            own_ground = treaty & 1;  // slot -0x4c reused as the pact flag
                            if (own_ground) {
                                cand *= 2;
                                if (PlayersData[Bases[base_id].faction_id_current].ranking < 6)
                                    continue;
                                if (treaty & 0x1000000)
                                    continue;
                            }
                            if (treaty & 2)
                                cand *= 2;
                            if (own_ground && (treaty & 0x2000000))
                                continue;
                            if ((treaty & 7) && !(treaty & 0x20)) {
                                if (PlayersData[faction_id].tech_ranking
                                    >= PlayersData[Bases[base_id].faction_id_current].tech_ranking)
                                    continue;
                                if (!at_climax(faction_id)
                                    && (PlayersData[faction_id]
                                            .diplo_treaties[Bases[base_id].faction_id_current]
                                        & 0x1000000))
                                    continue;
                            }
                        }
                        goto L_56e4a4;
L_56e434:
                        // Reached only from the adjacent-target scan above.
                        go_to(veh_id, 0x50, tx, ty);
                        goto L_57707f;
                    } else {
                        base_find(Bases[base_id].x, Bases[base_id].y, -1, -1, faction_id, -1);
                        cand += ((stack_check(veh_at(Bases[base_id].x, Bases[base_id].y), 2, 11, -1, -1)
                                  + (cand != 0)) * 64 + BaseFindDist) * 64;
                    }
L_56e4a4:
                    if (cand < best_score) {
                        best_score = cand;
                        go_to(veh_id, 0x70, Bases[base_id].x, Bases[base_id].y);
                    }
                }
                if (Vehs[veh_id].order == 0x18)
                    goto L_57707f;
                kill(veh_id);
                return 1;
            }
        }
    }

    if (!triad) {
        // Drop-capable land unit.
        if (has_abil(Vehs[veh_id].proto_id, 0x10) && !Vehs[veh_id].moves_expended
            && Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating
            && stack_check(veh_id, 2, 2, -1, -1)
            && !(Vehs[veh_id].state & 0x20)
            && !((uint8_t)faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
            && damage < 4) {
            if (!near_dist || (map_loc(x, y)->bit & 0x40000))
                goto L_56e6a9;
            if (whose_territory(faction_id, x, y, NULL, 0) == faction_id
                && Vehs[veh_id].order != 0x18 && home_base >= 0)
                go_to(veh_id, 0x49, Bases[home_base].x, Bases[home_base].y);
        }
L_56e62d:
        if (!home_dist && region_plan != plan) {
            if (!Bases[home_base].unk_1) {
                if (!stack_check(veh_id, 2, 7, -1, -1))
                    goto L_56ed87;
                if (plan == 9)
                    goto L_56ed87;
                if (plan == 1 && region_plan <= 2)
                    goto L_56ef2f;
            }
            stay = 1;
            defend = 1;
            goto L_57708a;
L_56e6a9:
            // Reached only from the drop-pod test above: pick an airdrop.
            dist = drop_range(faction_id);
            best_score = 99999;
            best_id = -1;
            for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                if (Bases[base_id].faction_id_current != faction_id) {
                    uint32_t treaty = PlayersData[faction_id]
                        .diplo_treaties[Bases[base_id].faction_id_current];
                    if (!(treaty & 1) && (treaty & 0xC10)
                        && !has_fac(29, base_id, 0)
                        && !has_project(35, Bases[base_id].faction_id_current)
                        && veh_who(Bases[base_id].x, Bases[base_id].y) < 0) {
                        int d = (abs(Bases[base_id].y - y) + x_dist(Bases[base_id].x, x)) >> 1;
                        if (d <= dist)
                            goto L_56e932;
                        if (d < best_score) {
                            best_score = d;
                            best_id = base_id;
                        }
                    }
                }
            }
            if (best_id >= 0) {
                base_id = best_id;
                best_score = 99999;
                for (m = 0; m < GameDataStateStorage.fields_.base_current_count_; m++) {
                    if (Bases[m].faction_id_current == faction_id
                        && ((abs(Bases[m].y - Bases[best_id].y)
                             + x_dist(Bases[m].x, Bases[best_id].x)) >> 1) <= dist
                        && !stack_check(veh_at(Bases[m].x, Bases[m].y), 6, 16, -1, -1)) {
                        if (((abs(Bases[m].y - y) + x_dist(Bases[m].x, x)) >> 1) <= dist)
                            goto L_56e96b;
                        if (map_loc(Bases[m].x, Bases[m].y)->region == region) {
                            int d = (abs(Bases[m].y - y) + x_dist(Bases[m].x, x)) >> 1;
                            if (d < best_score) {
                                best_score = d;
                                go_to(veh_id, 0x69, Bases[m].x, Bases[m].y);
                            }
                        }
                    }
                }
                goto L_56e62d;
L_56e932:
                action_airdrop(veh_id, Bases[base_id].x, Bases[base_id].y, 3);
                return 1;
L_56e96b:
                veh_skip(veh_id);
                action_airdrop(veh_id, Bases[m].x, Bases[m].y, 3);
                return 1;
            } else {
                Vehs[veh_id].order = 0;
                if (PlayersData[faction_id].proto_id_active[proto_id] > 1) {
                    n = -1;           // best drop y
                    tile_owner = -1;  // best drop x
                    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                        owner = Bases[base_id].faction_id_current;
                        if (owner != faction_id
                            && !(PlayersData[faction_id].diplo_treaties[owner] & 1)
                            && (PlayersData[faction_id].diplo_treaties[owner] & 0xC10)
                            && !has_fac(29, base_id, 0)
                            && !has_project(35, Bases[base_id].faction_id_current)) {
                            score = Bases[base_id].population_size;
                            if (PlayersData[owner].region_base_plan[region] == 7)
                                score *= 3;
                            if (PlayersData[owner].region_base_plan[region] == 8)
                                score *= 2;
                            if (!PlayersData[owner].region_base_plan[region])
                                score /= 2;
                            other_id = stack_fix(veh_at(Bases[base_id].x, Bases[base_id].y));
                            if (!stack_check(other_id, 2, 4, -1, -1)) {
                                j = stack_check(other_id, 16, -1, -1, -1);
                                for (m = 0; m < 24; m++) {
                                    tx = xrange(Bases[base_id].x + RadiusOffsetX[m + 1]);
                                    ty = Bases[base_id].y + RadiusOffsetY[m + 1];
                                    if (on_map(tx, ty) && (map_loc(tx, ty)->climate & 0xE0) >= 0x60) {
                                        int d = (abs(y - ty) + x_dist(x, tx)) >> 1;
                                        if (d <= dist) {
                                            int who = anything_at(tx, ty);
                                            if (who < 0 || who == faction_id) {
                                                cand = rnd(8, "ENEMYSTUFF") + score;
                                                if (who == faction_id)
                                                    cand = cand * 3 / 4;
                                                if (m < 8 && j > 1)
                                                    cand /= 2;
                                                if (map_loc(tx, ty)->bit & 0x1608050)
                                                    cand *= 2;
                                                if (map_loc(tx, ty)->bit & 4)
                                                    cand = cand * 4 / 3;
                                                for (i = 0; i < 8; i++) {
                                                    int x2 = xrange(tx + VehMoveDeltaX[i]);
                                                    int y2 = ty + VehMoveDeltaY[i];
                                                    if (on_map(x2, y2)) {
                                                        if (whose_territory(faction_id, x2, y2, NULL, 0) == faction_id)
                                                            cand = 0;
                                                        other_id = stack_fix(veh_at(x2, y2));
                                                        if (other_id >= 0) {
                                                            if (Vehs[other_id].faction_id == faction_id) {
                                                                if (has_abil(Vehs[other_id].proto_id, 0x10))
                                                                    cand += 32;
                                                            } else if (Vehs[other_id].faction_id == owner
                                                                       && VehPrototypes[Vehs[other_id].proto_id].plan <= 1) {
                                                                cand /= 2;
                                                            }
                                                        }
                                                    }
                                                }
                                                if (cand > best_score) {
                                                    best_score = cand;
                                                    tile_owner = tx;
                                                    n = ty;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (on_map(tile_owner, n)) {
                        action_airdrop(veh_id, tile_owner, n, 3);
                        return 1;
                    }
                }
                goto L_56e62d;
            }
        }
    }

L_56ed87:
    if ((plan != 6 || !(Vehs[veh_id].state & 0x200) || Vehs[veh_id].order_auto_type != 11)
        && triad != 2)
        goto L_56ef2f;
    if (planet_buster(veh_id) && triad == 2) {
        best_dir = enemy_planet_buster(veh_id);
        x = Vehs[veh_id].x;
        y = Vehs[veh_id].y;
        goto L_578b29;
    }
    air_result = air_power(veh_id, home_dist, zoc_moved, &x, &y);
    if (air_result == -5)
        goto L_578d9c;
    if (air_result != -1)
        goto L_56ee8f;
    if (Vehs[veh_id].x == Vehs[veh_id].waypoint_x[0] && Vehs[veh_id].y == Vehs[veh_id].waypoint_y[0]) {
        stay = 1;
        goto L_57707f;
    }
    // falls through to 0x56ee44 (chunk 2): air_result == -1, not at the waypoint.
    // ---- chunk 2: 0x56ee44-0x570969 ----
    // (continuation of `if (air_result == -1)`: unit is not on its waypoint)
L_56ee44:
    owner = anything_at(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
    // BUG(original): with nothing at the waypoint, owner == -1 still indexes the treaty row.
    if ((owner >= 0 && owner == faction_id)
        || (PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)) {
        goto L_578f46;
    }
    goto L_57707f;
L_56ee8f:
    if (air_result == -2) {
        if (Vehs[veh_id].x != Vehs[veh_id].waypoint_x[0]
            || Vehs[veh_id].y != Vehs[veh_id].waypoint_y[0]) {
            goto L_578f46;
        }
        stay = 1;
        goto L_57707f;
    } else if (air_result == -4) {
        stay = 1;
        goto L_57707f;
    } else if (air_result == -3) {
        if ((Vehs[veh_id].state & 0x200) && Vehs[veh_id].order_auto_type == 10) {
            Vehs[veh_id].order = ORDER_NONE;
            Vehs[veh_id].state &= ~0x200;
            return 0;
        }
    } else if (air_result >= 0) {
        best_dir = air_result;
    }
    goto L_578b29;

L_56ef2f:
    if (state_4000) {
        goto L_57460a;
    }
    if (plan == PLAN_NAVAL_SUPERIORITY) {
        if (Vehs[veh_id].order == ORDER_MOVE_TO) {
            best_score = (abs(y - Vehs[veh_id].waypoint_y[0])
                + x_dist(x, Vehs[veh_id].waypoint_x[0])) >> 1;
        } else {
            best_score = 9999;
        }
        // The radius bound is re-evaluated (veh_moves/range calls) every iteration.
        for (i = 0; i < RadiusRange[range(veh_moves(veh_id) / Rules.move_rate_roads, 1, 8)]; i++) {
            tx = xrange(x + RadiusOffsetX[i]);
            ty = y + RadiusOffsetY[i];
            if (on_map(tx, ty)
                && (map_loc(tx, ty)->climate & 0xE0) < 0x60
                && (!home_dist ? base_on_sea(home_base, map_loc(tx, ty)->region)
                               : map_loc(tx, ty)->region == region)
                && base_who(tx, ty) < 0) {
                for (other_id = stack_fix(veh_at(tx, ty)); other_id >= 0;
                     other_id = Vehs[other_id].next_veh_id_stack) {
                    if (Chassis[VehPrototypes[Vehs[other_id].proto_id].chassis_id].triad
                        == TRIAD_SEA) {
                        break;
                    }
                }
                if (other_id >= 0) {
                    dist = (abs(y - ty) + x_dist(x, tx)) >> 1;
                    if (Vehs[other_id].faction_id == faction_id) {
                        if (VehPrototypes[Vehs[other_id].proto_id].plan != PLAN_NAVAL_TRANSPORT
                            || veh_moves(other_id)
                            || stack_check(other_id, 2, 6, -1, -1)
                                > (Vehs[other_id].x == x && Vehs[other_id].y == y)) {
                            continue;
                        }
                    } else if (!(PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id]
                                 & DTREATY_VENDETTA)) {
                        continue;
                    }
                    if (dist < best_score) {
                        best_score = dist;
                        go_to(veh_id, 'n', tx, ty);
                    }
                }
            }
        }
        for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_;
             other_id++) {
            if (Vehs[other_id].faction_id != faction_id
                && (PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id] & 0xC30)
                && (faction_bit & Vehs[other_id].visibility)
                && (map_loc(Vehs[other_id].x, Vehs[other_id].y)->climate & 0xE0) < 0x60
                && base_who(Vehs[other_id].x, Vehs[other_id].y) < 0) {
                dist = (abs(y - Vehs[other_id].y) + x_dist(x, Vehs[other_id].x)) >> 1;
                if (VehPrototypes[Vehs[other_id].proto_id].plan != PLAN_NAVAL_TRANSPORT) {
                    dist += dist;
                }
                if (stack_check(other_id, 1, -1, -1, -1) > 1) {
                    dist /= 2;
                }
                // BUG(original): best_score is not lowered here, so every closer
                // target overrides the previous go_to.
                if (dist < best_score) {
                    add_goal(faction_id, 6, 2, Vehs[other_id].x, Vehs[other_id].y, -1);
                    go_to(veh_id, 'N', Vehs[other_id].x, Vehs[other_id].y);
                }
            }
        }
    }

    // Artillery.
    if (!can_arty(Vehs[veh_id].proto_id, 1)
        || (triad != TRIAD_SEA && (map_loc(x, y)->climate & 0xE0) < 0x60 && near_dist)
        || near_hostile) {
        goto L_56f8ad;
    }
    best_score = 0;
    best_id = -1;
    n = home_dist ? 25 : 81;
    for (i = 0; i < n; i++) {
        tx = xrange(x + RadiusOffsetX[i]);
        ty = y + RadiusOffsetY[i];
        if (!on_map(tx, ty)) {
            continue;
        }
        if (anything_at(tx, ty) < 0) {
            if ((map_loc(tx, ty)->climate & 0xE0) < 0x60) {
                continue;
            }
            {
                int terr = whose_territory(faction_id, tx, ty, NULL, 0);
                if (terr < 0 || terr == faction_id
                    || !(PlayersData[faction_id].diplo_treaties[terr] & DTREATY_VENDETTA)) {
                    continue;
                }
            }
            {
                uint32_t bits = map_loc(tx, ty)->bit;
                if (PlayersData[faction_id].region_total_bases[map_loc(tx, ty)->region]
                    && PlayersData[faction_id].region_base_plan[map_loc(tx, ty)->region] != 2
                    && !triad && !(bits & 0x80000000)) {
                    continue;
                }
                if (!(bits & 0x80001000)) {
                    continue;
                }
                if (!(bits & 0x80000000) && triad == TRIAD_SEA
                    && Vehs[veh_id].order == ORDER_MOVE_TO) {
                    continue;
                }
                if (!(bits & 0x81208854)) {
                    continue;
                }
                {
                    int val = (bits & 0x80000000) ? 32 : 2;
                    if (val > best_score) {
                        best_score = val;
                        best_id = i;  // tail shared with the vehicle branch (0x56f755)
                    }
                }
            }
        } else {
            other_id = stack_fix(veh_at(tx, ty));
            if (other_id < 0 || Vehs[other_id].faction_id == faction_id
                || !(PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id]
                     & DTREATY_VENDETTA)) {
                continue;
            }
            {
                int val = (stack_check(other_id, 2, 6, -1, -1)
                    + stack_check(other_id, 6, 0x8000, -1, -1)) * 8;
                if (triad == TRIAD_SEA) {
                    if ((map_loc(tx, ty)->climate & 0xE0) < 0x60 && base_who(tx, ty) >= 0) {
                        goto L_56f83a;
                    }
                    // slot -0x60 (tile_owner) is written with tx here: VC6 spill of the
                    // CSE'd tx across the abs call, not a source assignment.
                    if ((map_loc(tx, ty)->climate & 0xE0) < 0x60
                        && ((abs(y - ty) + x_dist(x, tx)) >> 1)
                            <= veh_moves(veh_id) / Rules.move_rate_roads) {
                        goto L_56f83a;
                    }
                    if (Vehs[veh_id].order == ORDER_MOVE_TO) {
                        continue;
                    }
                }
                if (!zoc_owner || i < 9) {
                    val += stack_check(other_id, 4, -1, -1, -1);
                }
                // slot -0x60 written with x here (spill, see above).
                if (((abs(y - ty) + x_dist(x, tx)) >> 1) > Rules.artillery_max_rng && val > 1) {
                    val = 1;
                }
                if (val > best_score) {
                    best_score = val;
                    best_id = i;
                }
            }
        }
    }
    if (best_id >= 0) {
        tx = xrange(x + RadiusOffsetX[best_id]);
        ty = y + RadiusOffsetY[best_id];
        // slot -0x60 written with x here (spill, see above).
        if (((abs(y - ty) + x_dist(x, tx)) >> 1) <= Rules.artillery_max_rng) {
            if (anything_at(tx, ty) < 0) {
                if (remote_own) {
                    log_say("Auto sending DESTROY2 message", veh_id, tx, ty);
                    message_data(0x2413, veh_id, tx, ty, 0, 0);
                    NetDaemonNet.await_exec(1);
                    return 1;
                }
                log_say("Enemy doing ARTY destroy 1...", veh_id, tx, ty);
                action_destroy(veh_id, 0, tx, ty);  // tail cross-jumped to 0x578d91
                return 1;
            }
            if (best_id >= 9
                || (triad == TRIAD_SEA && (map_loc(tx, ty)->climate & 0xE0) >= 0x60)) {
                if (remote_own) {
                    log_say("Auto sending ARTY message", veh_id, faction_id, 0);
                    message_veh(0x2402, veh_id, best_id, 0);
                    NetDaemonNet.await_exec(1);  // tail cross-jumped to 0x578d67
                    return 1;
                }
                log_say("Enemy doing ARTY...", veh_id, best_id, 0);
                battle_fight(veh_id, best_id, 1, 1, 0);
                return 1;
            }
        }
        if (!home_dist && best_id >= 25 && (!retreat || region_plan)
            && anything_at(tx, ty) >= 0) {
            goto L_578b34;
        }
        goto L_56f8ad;
    }
    goto L_56f841;
L_56f83a:
    best_id = -1;
L_56f841:
    if (!home_dist && !retreat && region_plan
        && stack_check(veh_id, 15, -1, -1, -1) <= 1
        && triad != TRIAD_SEA
        && ((map_loc(x, y)->bit & 0x4000) || region_plan != 2)) {
        goto L_578b34;
    }

    // Base site selection (colony pods) / site bookkeeping.
L_56f8ad:
    del_site(faction_id, 3, x, y, 0);
    if ((plan != PLAN_COLONIZATION && triad) || near_hostile) {
        goto L_570002;
    }
    keep_count = (triad == TRIAD_SEA);
    if (at_sea != keep_count) {
        goto L_570002;
    }
    if (plan == PLAN_COLONIZATION) {
        del_site(faction_id, plan, x, y, 1);
    }
    base_id = 0;  // best site value so far
    if (plan == PLAN_COLONIZATION && Vehs[veh_id].order == ORDER_MOVE_TO
        && vector_dist(x_dist(x, Vehs[veh_id].waypoint_x[0]),
                       abs(y - Vehs[veh_id].waypoint_y[0])) > 2) {
        base_id = site_at(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0], 0) - 1;
    }
    for (i = 0; i < 25; i++) {
        tx = xrange(x + RadiusOffsetX[i]);
        ty = y + RadiusOffsetY[i];
        if (!on_map(tx, ty)) {
            continue;
        }
        if (((map_loc(tx, ty)->climate & 0xE0) < 0x60) != keep_count) {
            continue;
        }
        if ((map_loc(tx, ty)->bit & 0x20) && (map_loc(tx, ty)->climate & 0xE0) >= 0x40) {
            continue;
        }
        if ((map_loc(tx, ty)->val3 & 0xC0) > 0x40 && (map_loc(tx, ty)->climate & 0xE0) >= 0x60) {
            continue;
        }
        if (map_loc(tx, ty)->region != region) {
            continue;
        }
        if (map_loc(tx, ty)->bit & 0x2000) {
            continue;
        }
        if (base_territory(faction_id, tx, ty) >= 0) {
            continue;
        }
        if (!triad) {
            if (!(PlayersData[faction_id].unk_77[region] & 0x80)
                && !is_coast(tx, ty, 0)
                && (int)(Continents[region].open_terrain
                         - PlayersData[faction_id].unk_81[region]) < 20) {
                continue;
            }
        } else if (triad == TRIAD_SEA) {
            // j is strength-reduced by VC6: its slot holds 4*j (0..0x144).
            for (j = 0; j < 81; j++) {
                int nx = xrange(tx + RadiusOffsetX[j]);
                int ny = ty + RadiusOffsetY[j];
                if (on_map(nx, ny)) {
                    int b = base_at(nx, ny);
                    if (b >= 0 && Bases[b].faction_id_current != faction_id
                        && (PlayersData[faction_id].diplo_treaties[Bases[b].faction_id_current]
                            & DTREATY_VENDETTA)
                        && (is_port(b, 0) || i < 21)) {
                        goto L_56fd9c;
                    }
                    other_id = stack_fix(veh_at(nx, ny));
                    if (other_id >= 0 && Vehs[other_id].faction_id != faction_id
                        && (PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id]
                            & DTREATY_VENDETTA)
                        && stack_check(other_id, 2, 6, -1, -1)) {
                        goto L_56fd9c;
                    }
                }
            }
        }
        {
            int site = site_at(tx, ty, 0);
            if (site <= base_id) {
                continue;
            }
            base_find(tx, ty);  // for BaseFindDist
            if (site < (8 - BaseFindDist) * 2
                && PlayersData[faction_id].region_total_bases[region]
                && PlayersData[faction_id].current_num_bases >= 2) {
                int threshold = region_plan != 8 ? 4 : 1;
                if (PlayersData[faction_id].unk_82[region] > threshold) {
                    threshold = PlayersData[faction_id].unk_82[region];
                }
                if (site < threshold
                    && (plan != PLAN_COLONIZATION || Vehs[veh_id].order == ORDER_MOVE_TO
                        || (int)Continents[region].open_terrain
                            >= (PlayersData[faction_id].region_total_bases[region] + 1) * 6)) {
                    continue;
                }
            }
            base_id = site;
            best_dir = i;
        }
L_56fd9c:
        ;
    }
    if (!base_id) {
        goto L_570002;
    }
    if (!best_dir && plan == PLAN_COLONIZATION) {
        base_id += ++Vehs[veh_id].terraforming_turns;
    }
    if (!base_id || best_dir < 0
        || (zoc_moved && !(PlayersData[faction_id].diplo_treaties[zoc_moved - 1] & 7))) {
        goto L_570002;
    }
    if (plan == PLAN_COLONIZATION) {
        if (!best_dir) {
            if (remote_own) {
                log_say("Auto sending BUILD BASE message", veh_id, faction_id, 0);
                message_base(0x2408, veh_id, EnemyBuildBaseMessageText, 0);
                NetDaemonNet.await_exec(1);  // tail cross-jumped to 0x578d67
                return 1;
            }
            action_build(veh_id, NULL);
            if (PlayersData[faction_id].current_num_bases == 1) {
                for (int v = 0; v < GameDataStateStorage.fields_.vehicle_current_count_; v++) {
                    if (Vehs[v].faction_id == faction_id
                        && VehPrototypes[Vehs[v].proto_id].plan == PLAN_TERRAFORMING) {
                        go_to(v, 'C', x, y);
                    }
                }
            }
            return 1;
        }
        Vehs[veh_id].move_to_ai_type = 'b';
        if (best_dir < 9) {
            best_dir--;  // radius index 1..8 -> direction 0..7
            // chunk 0 (0x56ccfc) jumps here with its direction in esi; the store
            // `mov [ebp-0x80], esi` at 0x56feee is the cross-jumped tail of its own
            // `best_dir = dir; goto L_578b26;` (see notes).
L_56feee:
            goto L_578b26;
        }
        go_to(veh_id, 'b', xrange(x + RadiusOffsetX[best_dir]), y + RadiusOffsetY[best_dir]);
        goto L_57000c;
    }
    if (best_dir < 9) {
        tx = xrange(x + RadiusOffsetX[best_dir]);
        ty = y + RadiusOffsetY[best_dir];
        score = 2;
        if (base_id < 6) {
            score = 1;
        }
        if (near_dist >= 0) {
            dist = (abs(ty - Bases[near_base].y) + x_dist(tx, Bases[near_base].x)) >> 1;
            if (dist <= 4) {
                score = 1;
            }
            if (dist >= 6 && dist < 12 && base_id >= 10) {
                score = 3;
            }
        }
        del_site(faction_id, 8, x, y, 2);
        add_site(faction_id, 8, score, tx, ty);
    }

L_570002:
    if (plan == PLAN_COLONIZATION) {
L_57000c:
        // Sea colony pod: scan the whole map for the best sea site in this region.
        if (triad == TRIAD_SEA && Vehs[veh_id].order != ORDER_MOVE_TO && at_sea) {
            best_score = 0;
            for (ty = 0; ty < MapLatitudeBounds; ty++) {
                for (tx = ty & 1; tx < MapLongitudeBounds; tx += 2) {
                    if (!on_map(tx, ty)) {
                        continue;
                    }
                    if (map_loc(tx, ty)->region != region) {
                        continue;
                    }
                    if (anything_at(tx, ty) >= 0) {
                        continue;
                    }
                    if ((map_loc(tx, ty)->bit & 0x20)
                        && (map_loc(tx, ty)->climate & 0xE0) >= 0x40) {
                        continue;
                    }
                    score = site_at(tx, ty, 0) * 10000
                        / (vector_dist(x_dist(x, tx), abs(y - ty)) + 1);
                    base_id = base_find(tx, ty);
                    if (base_id >= 0 && Bases[base_id].faction_id_current != faction_id
                        && (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                            & DTREATY_VENDETTA)) {
                        score /= 2;
                    }
                    if (score < best_score) {
                        continue;
                    }
                    // keep_count is strength-reduced by VC6: its slot holds 4*k (0..0x144).
                    for (keep_count = 0; keep_count < 81; keep_count++) {
                        int nx = xrange(tx + RadiusOffsetX[keep_count]);
                        int ny = ty + RadiusOffsetY[keep_count];
                        if (on_map(nx, ny) && (map_loc(nx, ny)->climate & 0xE0) < 0x60) {
                            base_id = base_at(nx, ny);
                            // BUG(original)? these two tests read the other faction's
                            // row (PlayersData[owner].diplo_treaties[faction_id]).
                            if (base_id >= 0 && Bases[base_id].faction_id_current != faction_id
                                && (PlayersData[Bases[base_id].faction_id_current]
                                        .diplo_treaties[faction_id] & DTREATY_VENDETTA)) {
                                goto L_5702c1;
                            }
                            other_id = stack_fix(veh_at(nx, ny));
                            if (other_id >= 0 && Vehs[other_id].faction_id != faction_id
                                && (PlayersData[Vehs[other_id].faction_id].diplo_treaties[faction_id]
                                    & DTREATY_VENDETTA)) {
                                goto L_5702c1;
                            }
                        }
                    }
                    best_score = score;
                    go_to(veh_id, 'c', tx, ty);
L_5702c1:
                    ;
                }
            }
        }
    } else if (plan != PLAN_RECONNAISANCE) {
        goto L_570c89;
    }

    // Land colony pods and recon units: site census, then a whole-map scan.
    // 0x5702f7
    if ((Vehs[veh_id].order == ORDER_MOVE_TO && near_dist) || triad || at_sea) {
        goto L_570c89;
    }
    j = 0;           // matching sites in this region
    keep_count = 0;  // of those, still worthwhile
    // VC6 turns this loop into a pointer walk with a 25..1 down-counter kept in
    // tile_owner's slot.
    for (tile_owner = 0; tile_owner < 25; tile_owner++) {
        if (PlayersData[faction_id].sites[tile_owner].type == plan
            && PlayersData[faction_id].sites[tile_owner].priority >= 1
            && map_loc(PlayersData[faction_id].sites[tile_owner].x,
                       PlayersData[faction_id].sites[tile_owner].y)->region == region) {
            j++;
            if (plan == PLAN_RECONNAISANCE) {
                if (goody_at(PlayersData[faction_id].sites[tile_owner].x,
                             PlayersData[faction_id].sites[tile_owner].y)) {
                    keep_count++;
                }
            } else {
                int threshold = PlayersData[faction_id].unk_82[region] > 8
                    ? 8 : PlayersData[faction_id].unk_82[region];
                if (region_plan != 8 && threshold < 4) {
                    threshold = 4;
                }
                if (site_at(PlayersData[faction_id].sites[tile_owner].x,
                            PlayersData[faction_id].sites[tile_owner].y, 0) >= threshold) {
                    keep_count++;
                }
            }
        }
    }
    i = (Vehs[veh_id].order == ORDER_MOVE_TO);
    if (j && ((GameDataStateStorage.fields_.turn_current_num_ + faction_id) & (keep_count ? 15 : 3))
        && near_dist) {
        goto L_570bdb;
    }
    best_score = 0;
    ty = 0;
    if (ty >= MapLatitudeBounds) {
        goto L_570ad4;
    }
    // for (ty...) { for (tx = ty & 1; ...; tx += 2) { ... } }: the increments and
    // loop tests live in chunk 3 (0x570aa9 / 0x570aaf / 0x570ac2).
L_570456:
    tx = ty & 1;
    if (tx >= MapLongitudeBounds) {
        goto L_570ac2;
    }
L_570466:
    if (!on_map(tx, ty)) {
        goto L_570aa9;
    }
    if (map_loc(tx, ty)->region != region) {
        goto L_570aaf;
    }
    m = whose_territory(faction_id, tx, ty, NULL, 0);
    if (region_plan != 8 && m != faction_id && m >= 0) {
        goto L_570aa9;
    }
    if (base_territory(faction_id, tx, ty) >= 0) {
        goto L_570aa9;
    }
    if ((!j || !((GameDataStateStorage.fields_.turn_current_num_ + faction_id) & 15))
        && (is_known(tx, ty, faction_id)
            || (GameDataStateStorage.fields_.diff_level_current_ > 3
                && !(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)))
        && goody_at(tx, ty)) {
        int b = base_find(tx, ty);
        if ((b >= 0 && Bases[b].faction_id_current == faction_id) || m == faction_id) {
            add_site(faction_id, 3, (BaseFindDist < 6) + 1, tx, ty);
        }
    }
    if ((map_loc(tx, ty)->climate & 0xE0) < 0x60) {
        goto L_570aaf;
    }
    if ((map_loc(tx, ty)->bit & 0x20) && (map_loc(tx, ty)->climate & 0xE0) >= 0x40) {
        goto L_570aaf;
    }
    if (map_loc(tx, ty)->bit & 0x2000) {
        goto L_570aaf;
    }
    if ((map_loc(tx, ty)->val3 & 0xC0) > 0x40) {
        goto L_570aaf;
    }
    if ((m == faction_id || m < 0) && !is_known(tx, ty, faction_id)) {
        // Unexplored tile: rate how much unknown land/pods lie around it.
        score = 0;
        // keep_count is strength-reduced by VC6: its slot holds 4*k (0..0x54).
        for (keep_count = 0; keep_count < 21; keep_count++) {
            int nx = xrange(tx + RadiusOffsetX[keep_count]);
            int ny = ty + RadiusOffsetY[keep_count];
            if (on_map(nx, ny) && map_loc(nx, ny)->region == region) {
                if (at_site(faction_id, 3, nx, ny)) {
                    goto L_570725;
                }
                if (is_known(nx, ny, faction_id)) {
                    if ((GameDataStateStorage.fields_.diff_level_current_ > 3
                         && !(faction_bit
                              & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))
                        || keep_count < 9) {
                        score |= 0x20;
                    }
                } else {
                    score++;
                    if (goody_at(nx, ny)) {
                        score++;
                    }
                }
            }
        }
        if (score >= 0x21) {
            add_site(faction_id, 3, score >= 0x28 ? 3 : (score >= 0x23) + 1, tx, ty);
        }
    }
L_570725:
    if (region_plan != 8 && m != faction_id) {
        goto L_570aaf;
    }
    // keep_count's slot is written at 0x570758 with the byte offset
    // faction_id*sizeof(PlayerData) + region*2 (VC6 CSE spill of the unk_77/unk_81
    // element address, reused at 0x5708f7), not a source assignment.
    if (!(PlayersData[faction_id].unk_77[region] & 0x80)
        && !is_coast(tx, ty, 0)
        && (int)(Continents[region].open_terrain - PlayersData[faction_id].unk_81[region]) < 20) {
        goto L_570aa9;
    }
    score = site_at(tx, ty, 0);
    if (!score) {
        goto L_570aa9;
    }
    if (score < (8 - BaseFindDist) * 2
                - (PlayersData[faction_id].region_total_bases[region] < 2 ? 2 : 0)
        && score < PlayersData[faction_id].unk_82[region]
        && PlayersData[faction_id].region_total_bases[region]
        && PlayersData[faction_id].current_num_bases >= 2) {
        goto L_570aa9;
    }
    // n: no better site in the surrounding ring.
    n = 1;
    // tile_owner is strength-reduced by VC6: its slot holds 4*k (0..0x60).
    for (tile_owner = 0; tile_owner < 24; tile_owner++) {
        int nx = xrange(tx + RadiusOffsetX[tile_owner + 1]);
        int ny = ty + RadiusOffsetY[tile_owner + 1];
        if (!on_map(nx, ny)) {
            continue;
        }
        if (map_loc(nx, ny)->region != region) {
            continue;
        }
        if ((map_loc(nx, ny)->bit & 0x20) && (map_loc(nx, ny)->climate & 0xE0) >= 0x40) {
            continue;
        }
        if (map_loc(nx, ny)->bit & 0x2000) {
            continue;
        }
        if ((map_loc(nx, ny)->val3 & 0xC0) > 0x40) {
            continue;
        }
        if (base_territory(faction_id, nx, ny) >= 0) {
            continue;
        }
        if (map_loc(nx, ny)->bit & 0x1000) {
            continue;
        }
        {
            int site = site_at(nx, ny, 0);
            if (site < score || (site == score && !at_site(faction_id, 8, nx, ny))) {
                continue;
            }
        }
        if (!(PlayersData[faction_id].unk_77[region] & 0x80)
            && !is_coast(nx, ny, 0)
            && (int)(Continents[region].open_terrain
                     - PlayersData[faction_id].unk_81[region]) < 20) {
            continue;
        }
        if (region_plan == 8 || whose_territory(faction_id, nx, ny, NULL, 0) == faction_id) {
            goto L_570969;  // chunk 3: n = 0 (the loop's break block)
        }
    }
    goto L_570970;
    // ---- chunk 3: 0x570969-0x5725b0 ----
    // Tail of the coastal-site tile scan (the inner ring loop above breaks
    // here with n = 0).
L_570969:
    n = 0;
L_570970:
    {
        int s; // edi
        s = (score << 16) / (((abs(y - ty) + x_dist(x, tx)) >> 1) + 1);
        if (n) {
            if (s > best_score || is_known(tx, ty, faction_id) ||
                (GameDataStateStorage.fields_.diff_level_current_ > 3 &&
                 !(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))) {
                if (Paths.find(tx, ty, x, y, whose_territory(faction_id, tx, ty, NULL, 0),
                               faction_id, 0x201, -1) >= 0) {
                    if (is_known(tx, ty, faction_id) ||
                        (GameDataStateStorage.fields_.diff_level_current_ > 3 &&
                         !(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))) {
                        del_site(faction_id, 8, tx, ty, 2);
                        add_site(faction_id, 8, 2, tx, ty);
                        j++;
                    }
                    if (s > best_score) {
                        best_score = s;
                        if (!i && plan == 8) go_to(veh_id, 'c', tx, ty);
                    }
                }
            }
        }
    }
    // Rotated map scan loops opened in chunk 2 (x step 2, y step 1).
L_570aa9:
L_570aaf:
    tx += 2;
    if (tx < MapLongitudeBounds) goto L_570466;
L_570ac2:
    ty++;
    if (ty < MapLatitudeBounds) goto L_570456;
L_570ad4:
    if (plan != 3 && !i) {
        int count; // ebx; j itself is not written back
        count = j;
        if (Vehs[veh_id].order == 24) {
            if (!count) count = 1;
            add_site(faction_id, 3, 2, Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
            if (is_known(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0], faction_id) ||
                (GameDataStateStorage.fields_.diff_level_current_ > 3 &&
                 !(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))) {
                // BUG(original): del_site uses the terminal scan coordinates tx/ty.
                del_site(faction_id, 8, tx, ty, 2);
                add_site(faction_id, 8, 3, Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
            } else {
                Vehs[veh_id].order = 0;
            }
        }
        if (!count) {
            PlayersData[faction_id].unk_80[region] += -10; // add word ptr ...,0xfff6
            PlayersData[faction_id].unk_77[region] |= 9;
            if (region_plan == 8) {
                region_plan = 7;
                PlayersData[faction_id].region_base_plan[region] = 7;
            }
        }
    }
L_570bdb:
    if (plan == 8 && Vehs[veh_id].order != 24) {
        owner = whose_territory(faction_id, x, y, NULL, 0);
        if (owner >= 0 && owner != faction_id &&
            (PlayersData[faction_id].diplo_treaties[owner] & 3)) {
            base_id = base_find(x, y, faction_id, region, -1, -1);
            if (base_id >= 0) {
                go_to(veh_id, 'k', Bases[base_id].x, Bases[base_id].y);
                goto L_57717f;
            }
        }
    }
L_570c89:
    // Destroy our own/pact roads next to bases (0x800000 = road bit).
    if (speed(veh_id, 0) <= Rules.move_rate_roads &&
        (map_loc(x, y)->bit & 0x800000) &&
        Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != 2 &&
        near_base >= 0) {
        tx = xrange(x + 6);
        base_id = base_find(tx, y);
        if (base_id >= 0 && BaseFindDist <= 2) {
            if (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current] & 0x10) {
                if (remote_own) {
                    log_say("Auto sending DESTROY message", veh_id, 0x800000, 0);
                    message_data(0x2406, veh_id, 0x800000, 0, 0, 0);
                    NetDaemonNet.await_exec(1);
                    return 1;
                }
                action_destroy(veh_id, 0x800000, -1, -1);
                return 0;
            }
        } else if (Bases[near_base].faction_id_current == faction_id && near_dist <= 2) {
            if (remote_own) {
                log_say("Auto sending DESTROY message", veh_id, 0x800000, 0);
                message_data(0x2406, veh_id, 0x800000, 0, 0, 0);
                NetDaemonNet.await_exec(1);
                return 1;
            }
            action_destroy(veh_id, 0x800000, -1, -1);
            return 0;
        }
    }
    if (speed(veh_id, 0) <= Rules.move_rate_roads &&
        (map_loc(x, y)->bit & 0x400000) &&
        Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != 2 &&
        near_base >= 0 && near_dist <= 2 &&
        Bases[near_base].faction_id_current != faction_id &&
        home_dist > 3 &&
        (PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 0x10)) {
        if (remote_own) {
            log_say("Auto sending DESTROY message", veh_id, 0x400000, 0);
            message_data(0x2406, veh_id, 0x400000, 0, 0, 0);
            NetDaemonNet.await_exec(1);
            return 1;
        }
        action_destroy(veh_id, 0x400000, -1, -1);
        return 0;
    }
    if (speed(veh_id, 0) <= Rules.move_rate_roads && near_base >= 0 &&
        (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &
         (1 << Bases[near_base].faction_id_current)) &&
        (PlayersData[faction_id].diplo_treaties[Bases[near_base].faction_id_current] & 0x10) &&
        triad != 2 &&
        (((map_loc(x, y)->climate & 0xE0) < 0x60) == (triad == 1)) &&
        Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating &&
        !near_hostile &&
        whose_territory(faction_id, x, y, NULL, 0) == Bases[near_base].faction_id_current &&
        ((map_loc(x, y)->bit & 0x81208850) ||
         ((map_loc(x, y)->bit & 4) && !PlayersData[faction_id].region_total_bases[region]))) {
        if (remote_own) {
            log_say("Auto sending DESTROY message", veh_id, 0, 0);
            message_data(0x2406, veh_id, 0, 0, 0, 0);
            NetDaemonNet.await_exec(1);
            return 1;
        }
        log_say("Enemy doing DESTROY", veh_id, x, y);
        action_destroy(veh_id, 0, -1, -1);
        goto L_578f46;
    }
    if (region_plan > 2 && !(PlayersData[faction_id].unk_77[region] & 0x2000) &&
        !zoc_owner && plan <= 3 && !triad) {
        Vehs[veh_id].state |= 0x40000000;
    } else if (triad == 2) {
        goto L_57137d;
    }
    // Look for a nearby supply pod.
    if (plan == 8 || plan == 9) goto L_57137d;
    if (!home_dist && !(Vehs[veh_id].state & 0x40000000)) {
        int limit; // esi
        limit = 2;
        if (plan == 3) {
            if (Vehs[veh_id].home_base_id != home_base && !zoc_owner) goto L_57117a;
        } else if (plan != 2 && plan != 1) {
            limit = (triad == 0);
        }
        if (stack_check(veh_id, 16, -1, -1, -1) <= limit || zoc_owner) goto L_57137d;
    }
L_57117a:
    if (near_hostile) goto L_57137d;
    if ((Vehs[veh_id].order || plan == 7) && plan != 3 &&
        !(Vehs[veh_id].state & 0x40000000)) {
        if (!triad) goto L_57137d;
        if (plan == 7) {
            if (!(Vehs[veh_id].state & 0x20000000)) goto L_57137d;
            goto L_5711d4;
        }
    }
    if (plan == 6 && Vehs[veh_id].order == 24) goto L_57137d;
L_5711d4:
    if (GameDataStateStorage.fields_.turn_current_num_ >= 100 && !own_ground &&
        home_dist > 6 && Vehs[veh_id].order == 24 &&
        Vehs[veh_id].move_to_ai_type != 'X' &&
        !(Vehs[veh_id].state & 0x40000000) && plan != 3) goto L_57137d;
    m = 25;
    if (plan == 3 || triad == 1) m = 49;
    if (plan == 7 && (stack_check(veh_id, 2, 8, -1, -1) || damage > 1)) m = 9;
    if (GameDataStateStorage.fields_.turn_current_num_ >= (plan == 6 ? 400 : 100) &&
        Vehs[veh_id].order == 24 && Vehs[veh_id].move_to_ai_type != 'X' && plan != 3) m = 9;
    for (i = 1; i < m; i++) {
        tx = xrange(x + RadiusOffsetX[i]);
        ty = y + RadiusOffsetY[i];
        if (on_map(tx, ty) && goody_at(tx, ty) &&
            (is_known(tx, ty, faction_id) ||
             (GameDataStateStorage.fields_.diff_level_current_ > 3 &&
              !(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_))) &&
            (((map_loc(tx, ty)->climate & 0xE0) < 0x60) == (triad == 1)) &&
            map_loc(tx, ty)->region == region) {
            // Image places this block at 0x571537 (after the OBLIT return).
            if (Vehs[veh_id].state & 0x40000000) {
                PlayersData[faction_id].unk_77[region] |= 0x2000;
            }
            go_to(veh_id, 'G', tx, ty);
            goto L_57707f;
        }
    }
L_57137d:
    best_dir = -1;
L_571384:
L_57138d:
    faction_bit = 1 << faction_id;
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) &&
        !home_dist && !hint && zoc_owner && !is_objective(home_base) &&
        (region_plan == 2 || PlayersData[faction_id].region_total_bases[region] == 1 ||
         (PlayersData[faction_id].flags & 0x2000000))) {
        owner = Bases[home_base].faction_id_former;
        if (stack_check(veh_id, 1, -1, -1, -1) <= (Bases[home_base].population_size + 1) / 2 &&
            stack_check(veh_id, 16, -1, -1, -1) < (Vehs[veh_id].dmg_incurred != 0) + 2 &&
            owner != faction_id &&
            (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << owner)) &&
            (PlayersData[faction_id].diplo_treaties[owner] & 0x20) &&
            !(PlayersData[faction_id].diplo_treaties[owner] & 3) &&
            ((PlayersData[faction_id].diplo_treaties[owner] & 0x40000) ||
             PlayersData[owner].integrity_blemishes > 5 ||
             PlayersData[faction_id].diplo_betrayed[owner] > 3 - PlayersData[faction_id].ai_fight * 2)) {
            log_say("Enemy doing OBLIT", veh_id, x, y);
            action_oblit(veh_id, home_base);
            return 1;
        }
    }
L_571579:
    // Garrison/defence of the nearest base.
    if (near_dist || (home_dist && !damage) || state_4000) goto L_571977;
    {
        int def;   // edi
        int avail; // esi
        keep_count = garrison_check(near_base);
        def = defensive_check(near_base);
        j = stack_check(veh_id, 11, 0x10000000, 0x10000000, -1);
        avail = range(stack_check(veh_id, 2, 2, faction_id, -1), 0, def) + j;
        if (Vehs[veh_id].state & 0x10000000) {
            int need; // eax
            need = keep_count - def;
            if (need < 1) need = 1;
            if (j <= need) plan = 2;
        }
        if (region_plan == 2 || region_plan == 8) {
            avail += stack_check(veh_id, 2, 1, faction_id, -1);
            if (plan == 1) plan = 2;
        }
        if (!avail) avail = stack_check(veh_id, 2, 3, faction_id, -1);
        if (!assault && !retreat) {
            keep_count += stack_check(veh_id, 11, 0x40000, 0x40000, -1);
        }
        if (!zoc_veh(x, y, faction_id) && region_plan > 2 && !hint &&
            (plan == 3 ||
             (plan <= 2 && PlayersData[faction_id].region_total_combat_vehs[region] < 2)) &&
            !(PlayersData[faction_id].unk_77[region] & 0x2000)) goto L_571982;
        if (plan == 2 || plan == 3 ||
            (plan == 1 && (region_plan == 2 || region_plan == 8))) goto L_571853;
        if (stack_check(veh_id, 2, 2, faction_id, -1)) goto L_571977;
        if (stack_check(veh_id, 2, 3, faction_id, -1)) goto L_571977;
        if (stack_check(veh_id, 2, 1, faction_id, -1) > (plan == 1)) goto L_571977;
        add_goal(faction_id, 2,
                 stack_check(veh_id, range(Bases[near_base].population_size, 1, 3), -1, -1, -1),
                 x, y, -1);
        if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating &&
            !zoc_sea(x, y, faction_id)) goto L_571977;
        defend = 1;
        stay = 1;
        if (hint) return 1;
        goto L_57707f;
L_571853:
        if (avail < keep_count) add_goal(faction_id, 2, 2, x, y, -1);
        if ((avail > keep_count && (plan != 2 || !j || avail > 2)) || assault || retreat) {
            if (avail > 1 && (!k || !zoc_owner)) goto L_571977;
        }
        if (!home_dist && Vehs[veh_id].home_base_id >= 0 &&
            !(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
            Vehs[veh_id].home_base_id = home_base;
        }
        Vehs[veh_id].order = 0;
        stay = 1;
        defend = (avail <= 1) + 1;
        if (hint) return 1;
        if (plan != 2 || region_plan <= 2 || avail < keep_count ||
            (Bases[home_base].state & 0x4000000)) goto L_57707f;
        // BUG(original)? region_total_combat_vehs is read from PlayersData[0].
        if (avail <= 1 && (PlayersData[0].region_total_combat_vehs[region] ||
                           (PlayersData[faction_id].flags & 0x10000))) goto L_57707f;
        goto L_571982;
    }
L_571977:
    if (hint) return 0;
L_571982:
    if (Vehs[veh_id].order != 24 && (k || plan == 2)) {
        Vehs[veh_id].order = 0;
        // hint is reused as the ring index (slot holds hint*4).
        for (hint = 0; hint < 289; hint++) {
            tx = xrange(x + RadiusOffsetX[hint]);
            ty = y + RadiusOffsetY[hint];
            if (on_map(tx, ty) && map_loc(tx, ty)->region == region &&
                whose_territory(faction_id, tx, ty, NULL, 0) == faction_id) {
                dist = vector_dist(x_dist(x, tx), abs(y - ty));
                other_id = stack_fix(veh_at(tx, ty));
                if (other_id < 0 || Vehs[other_id].faction_id == faction_id) {
                    if (!(map_loc(tx, ty)->bit & 0x2000) || (Vehs[veh_id].state & 0x100) ||
                        !want_monolith(veh_id)) continue;
                    dist += 1000;
                } else {
                    if (!k) continue;
                    if (Vehs[other_id].faction_id) {
                        if (stack_check(other_id, 1, -1, -1, -1) > 1) continue;
                        if (!(PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id] & 0x10)) continue;
                        if (Weapon[VehPrototypes[Vehs[other_id].proto_id].weapon_id].offense_rating >= 0) continue;
                    }
                    if (Chassis[VehPrototypes[Vehs[other_id].proto_id].chassis_id].triad) continue;
                }
                if (dist < 9999) {
                    int type; // ecx
                    dist = 9999;
                    type = 'F';
                    if (Vehs[veh_id].home_base_id >= 0) {
                        base_id = Vehs[veh_id].home_base_id;
                        if ((Bases[base_id].state & 0x4000000) &&
                            Vehs[veh_id].move_to_ai_type != 'F') type = 'y';
                    }
                    go_to(veh_id, type, tx, ty);
                }
            }
        }
        if (Vehs[veh_id].order == 24) {
            stay = 0;
            defend = 0;
            goto L_57707f;
        }
        if (defend) goto L_57707f;
        if (k) {
            for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                if (Bases[base_id].faction_id_current == faction_id &&
                    map_loc(Bases[base_id].x, Bases[base_id].y)->region == region) {
                    dist = vector_dist(x_dist(x, Bases[base_id].x), abs(y - Bases[base_id].y));
                    if (base_id != Vehs[veh_id].home_base_id) dist += 1000;
                    if (stack_check(veh_at(Bases[base_id].x, Bases[base_id].y), 0x11, 0x13, faction_id, -1)) {
                        dist *= 2;
                    }
                    if (dist < 9999) {
                        dist = 9999;
                        go_to(veh_id, 'f', Bases[base_id].x, Bases[base_id].y);
                    }
                }
            }
        }
    }
    // Psi gate to another region's base.
    if (plan != region_plan && region_plan > 2 && !home_dist &&
        has_fac(0x21, home_base, 0) && !(Bases[home_base].state & 0x20000000)) {
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            // BUG(original): the second has_fac tests home_base, not base_id.
            if (Bases[base_id].faction_id_current == faction_id &&
                has_fac(0x21, home_base, 0) &&
                !(Bases[base_id].state & 0x20000000)) {
                int r; // ebx
                r = map_loc(Bases[base_id].x, Bases[base_id].y)->region;
                if (r != region &&
                    (PlayersData[faction_id].region_base_plan[r] == plan ||
                     (plan <= 2 && PlayersData[faction_id].region_base_plan[r] <= 2))) {
                    // Image places both exits out of line at 0x5721de/0x5721fa.
                    if (!remote_own) {
                        log_say("Enemy doing GATE", veh_id, base_id, 0);
                        action_gate(veh_id, base_id);
                        return 1;
                    }
                    if (!NetDaemonNet.add_lock(0, Bases[base_id].x, Bases[base_id].y)) {
                        message_veh(0x2410, veh_id, base_id, 0);
                        NetDaemonNet.await_exec(1);
                        return 1;
                    }
                }
            }
        }
    }
    if (!assault && !retreat &&
        x == PlayersData[faction_id].x_target && y == PlayersData[faction_id].y_target &&
        (VehPrototypes[Vehs[veh_id].proto_id].plan == 0 ||
         VehPrototypes[Vehs[veh_id].proto_id].plan == 1) &&
        !Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad &&
        !has_abil(Vehs[veh_id].proto_id, 0x10)) {
        Vehs[veh_id].state |= 0x40000;
    }
    // Siege staging next to an enemy base we are moving onto.
    if (plan > 1 && !retreat) goto L_572922;
    if (Vehs[veh_id].order != 24) goto L_572922;
    // [ebp-0x4c] (own_ground) is only a spill slot for abs() across x_dist here.
    if (((abs(y - Vehs[veh_id].waypoint_y[0]) + x_dist(x, Vehs[veh_id].waypoint_x[0])) & ~1) != 4)
        goto L_572922;
    base_id = base_at(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
    if (base_id < 0) goto L_572922;
    // 0x571faf/0x571fc2 store base_id*0x134 into hint's slot and
    // faction_id*0x833 into j's slot: compiler CSE temps, not source.
    if (!(PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current] & 0x10))
        goto L_572922;
    Bases[base_id].event |= faction_bit;
    i = 0;
    m = 0;
    k = 0;
    {
        int r; // esi (scaled)
        for (r = 1; r < 9; r++) {
            tx = xrange(Bases[base_id].x + RadiusOffsetX[r]);
            ty = Bases[base_id].y + RadiusOffsetY[r];
            if (on_map(tx, ty)) {
                other_id = stack_fix(veh_at(tx, ty));
                if (other_id >= 0) {
                    for (other_id = veh_top(other_id); other_id >= 0;
                         other_id = other_id < 0 ? other_id : Vehs[other_id].next_veh_id_stack) {
                        if (Vehs[other_id].faction_id == faction_id &&
                            VehPrototypes[Vehs[other_id].proto_id].plan <= 1 &&
                            !has_abil(Vehs[other_id].proto_id, 0x8000)) {
                            i++;
                        }
                    }
                }
            }
        }
        for (r = 9; r < 25; r++) {
            tx = xrange(Bases[base_id].x + RadiusOffsetX[r]);
            ty = Bases[base_id].y + RadiusOffsetY[r];
            if (on_map(tx, ty)) {
                other_id = stack_fix(veh_at(tx, ty));
                if (other_id >= 0) {
                    for (other_id = veh_top(other_id); other_id >= 0;
                         other_id = other_id < 0 ? other_id : Vehs[other_id].next_veh_id_stack) {
                        if (Vehs[other_id].faction_id == faction_id &&
                            VehPrototypes[Vehs[other_id].proto_id].plan <= 1 &&
                            !has_abil(Vehs[other_id].proto_id, 0x8000) &&
                            ((Vehs[other_id].move_to_ai_type >= 0x33 &&
                              Vehs[other_id].move_to_ai_type <= 0x35) ||
                             (Vehs[other_id].waypoint_x[0] == Vehs[veh_id].waypoint_x[0] &&
                              Vehs[other_id].waypoint_y[0] == Vehs[veh_id].waypoint_y[0]))) {
                            if (Vehs[other_id].moves_expended) k++;
                            else m++;
                        }
                    }
                }
            }
        }
        for (r = 25; r < 169; r++) {
            tx = xrange(Bases[base_id].x + RadiusOffsetX[r]);
            ty = Bases[base_id].y + RadiusOffsetY[r];
            if (on_map(tx, ty)) {
                other_id = stack_fix(veh_at(tx, ty));
                if (other_id >= 0) {
                    for (other_id = veh_top(other_id); other_id >= 0;
                         other_id = other_id < 0 ? other_id : Vehs[other_id].next_veh_id_stack) {
                        // BUG(original): the upper bound tests the actor's move_to_ai_type.
                        if (Vehs[other_id].faction_id == faction_id &&
                            VehPrototypes[Vehs[other_id].proto_id].plan <= 1 &&
                            !has_abil(Vehs[other_id].proto_id, 0x8000) &&
                            ((Vehs[other_id].move_to_ai_type >= 0x33 &&
                              Vehs[veh_id].move_to_ai_type <= 0x35) ||
                             ((base_who(tx, ty) < 0 || (Vehs[other_id].state & 0x10000)) &&
                              Vehs[other_id].waypoint_x[0] == Vehs[veh_id].waypoint_x[0] &&
                              Vehs[other_id].waypoint_y[0] == Vehs[veh_id].waypoint_y[0]))) {
                            k++;
                        }
                    }
                }
            }
        }
    }
    if (m + i >= 8 || k <= i) goto L_5725e2;
    j = stack_check(veh_id, 2, 1, -1, -1) + stack_check(veh_id, 2, 0, -1, -1);
    if (zoc_move(x, y, faction_id) || j <= 1) goto L_5725c9;
    best_id = -1;
    best_score = 0;
    for (i = 0; i < 8; i++) {
        tx = xrange(x + VehMoveDeltaX[i]);
        ty = y + VehMoveDeltaY[i];
        if (on_map(tx, ty) && (map_loc(tx, ty)->climate & 0xE0) >= 0x60 &&
            ((abs(ty - Bases[base_id].y) + x_dist(tx, Bases[base_id].x)) & ~1) == 4) {
            owner = anything_at(tx, ty);
            if (owner < 0 || owner == faction_id) {
                int sc;    // ebx
                int stack; // edi
                sc = 10;
                if (zoc_sea(tx, ty, faction_id)) {
                    if (hex_cost(proto_id, faction_id, x, y, tx, ty, 0) >= veh_moves(veh_id) - 1) continue;
                    sc = 20;
                }
                stack = veh_at(tx, ty);
                if (stack >= 0) {
                    int c; // esi
                    c = stack_check(stack, 2, 1, -1, -1) + stack_check(stack, 2, 0, -1, -1);
                    if (c >= j) continue;
                    sc -= c;
                }
                if (sc > best_score) {
                    best_score = sc;
                    best_id = i;
                }
            }
        }
    }
    if (best_id >= 0) {
        best_dir = best_id;
        Vehs[veh_id].move_to_ai_type = 0x35;
        goto L_578b26;
    }
    // ---- chunk 4: 0x5725b0-0x573fd0 ----
L_5725b0:
    // best_id < 0 after the sea-escort scan
    stay = 1;
    Vehs[veh_id].move_to_ai_type = 0x34;
    goto L_57707f;
L_5725c9:
    // zoc_move(...) || j <= 1
    stay = 1;
    Vehs[veh_id].move_to_ai_type = 0x34;
    goto L_57707f;
L_5725e2:
    if (!zoc_move(x, y, faction_id)) {
        best_score = -9999;
        for (i = 0; i < 8; i++) {
            tx = xrange(x + VehMoveDeltaX[i]);
            ty = y + VehMoveDeltaY[i];
            if (on_map(tx, ty) && !is_ocean(tx, ty)
                && ((abs(ty - Bases[base_id].y) + x_dist(tx, Bases[base_id].x)) & ~1) == 2) {
                score = defense_value(faction_id, tx, ty, veh_id, -1) * 8;
                {
                    int d;
                    for (d = 0; d < 8; d++) {
                        int ax = xrange(tx + VehMoveDeltaX[d]);
                        int ay = ty + VehMoveDeltaY[d];
                        if (on_map(ax, ay) && (ax != Bases[base_id].x || ay != Bases[base_id].y)) {
                            other_id = stack_fix(veh_at(ax, ay));
                            if (other_id >= 0 && Vehs[other_id].faction_id != faction_id
                                && !(PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id] & 7)
                                && stack_check(other_id, 4, -1, -1, -1)) {
                                score /= 2;
                            }
                        }
                    }
                }
                {
                    int s = score;
                    if (!Vehs[veh_id].moves_expended && (map_loc(x, y)->bit & BIT_ROAD)
                        && (map_loc(tx, ty)->bit & BIT_ROAD)) {
                        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].speed > 1) {
                            s += 0x100;
                        } else {
                            s += 8;
                        }
                    }
                    if ((map_loc(tx, ty)->bit & BIT_FUNGUS) && (map_loc(tx, ty)->climate & ~0x1F) >= 0x40) {
                        if (has_project(8, faction_id)
                            || (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating < 0
                                && proto_id < 0x40)) {
                            if (Vehs[veh_id].proto_id < 0x40
                                && (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating < 0
                                    || Vehs[veh_id].proto_id == 0xF)) {
                                s += 0x100;
                            } else {
                                s += 8;
                            }
                        } else if (!has_tech(Rules.tech_ease_fungus_movement, faction_id)) {
                            s -= 0x100;
                        }
                    }
                    other_id = veh_at(tx, ty);
                    s -= stack_check(other_id, 1, -1, -1, -1) * 8;
                    if (s >= best_score) {
                        best_score = s;
                        best_dir = i;
                    }
                }
            }
        }
        // BUG(original): best_dir is not reset to -1 before this scan.
        if (best_dir >= 0) {
            Vehs[veh_id].move_to_ai_type = 0x33;
            goto L_578b26;
        }
    }
L_572922:
    if (state_4000 || (plan & 0xF) != 9) goto L_5741cf;
    if (zoc_owner == 1
        || (zoc_owner
            && (faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
            && (PlayersData[faction_id].diplo_treaties[zoc_owner - 1] & 0x10))) {
        goto L_57707f;
    }
    j = (Vehs[veh_id].state & 0x200) && VehPrototypes[Vehs[veh_id].proto_id].plan == 9
        && Vehs[veh_id].order_auto_type == 3;
    own_ground = 0;
    if (Vehs[veh_id].order != ORDER_MOVE_TO) {
        if (!j && home_base >= 0) {
            base_id = home_base;
        } else {
            base_id = Vehs[veh_id].home_base_id;
            if (j) {
                if (base_id < 0) base_id = home_base;
                goto L_572a5c;
            }
        }
        if (Vehs[veh_id].home_base_id >= 0 && base_id != Vehs[veh_id].home_base_id) {
            if (base_id < 0) goto L_572d66;
            if (vector_dist(x_dist(x, Bases[Vehs[veh_id].home_base_id].x),
                    abs(y - Bases[Vehs[veh_id].home_base_id].y)) < home_dist * 2) {
                base_id = Vehs[veh_id].home_base_id;
            }
        }
L_572a5c:
        if (base_id >= 0) {
            for (k = 0; k < 75; k++) {
                if (PlayersData[faction_id].goals[k].type >= 0
                    && (PlayersData[faction_id].goals[k].type & 0xF) == 9
                    && PlayersData[faction_id].goals[k].type != 0x29
                    && on_map(PlayersData[faction_id].goals[k].x, PlayersData[faction_id].goals[k].y)
                    && PlayersData[faction_id].goals[k].priority != 0
                    && (vector_dist(x_dist(PlayersData[faction_id].goals[k].x, Bases[base_id].x),
                            abs(PlayersData[faction_id].goals[k].y - Bases[base_id].y)) <= 2
                        || (!j && PlayersData[faction_id].goals[k].base_id == base_id))) {
                    if (!((abs(y - PlayersData[faction_id].goals[k].y)
                           + x_dist(x, PlayersData[faction_id].goals[k].x)) & ~1)) {
                        Vehs[veh_id].order = 0;
                        own_ground = 1;
                        if (!home_dist) goto L_578b34;
                        break;
                    }
                    if (is_ocean(PlayersData[faction_id].goals[k].x, PlayersData[faction_id].goals[k].y) == (triad == 1)
                        && (map_loc(PlayersData[faction_id].goals[k].x, PlayersData[faction_id].goals[k].y)->region == region
                            || at_sea != is_ocean(PlayersData[faction_id].goals[k].x, PlayersData[faction_id].goals[k].y))) {
                        owner = anything_at(PlayersData[faction_id].goals[k].x, PlayersData[faction_id].goals[k].y);
                        if (owner < 0 || owner == faction_id
                            || (PlayersData[faction_id].diplo_treaties[owner] & 1)) {
                            for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
                                if (Vehs[other_id].faction_id == faction_id
                                    && VehPrototypes[Vehs[other_id].proto_id].plan == 9
                                    && other_id != veh_id
                                    && ((Vehs[other_id].x == PlayersData[faction_id].goals[k].x
                                         && Vehs[other_id].y == PlayersData[faction_id].goals[k].y)
                                        || (Vehs[other_id].order == ORDER_MOVE_TO
                                            && Vehs[other_id].waypoint_x[0] == PlayersData[faction_id].goals[k].x
                                            && Vehs[other_id].waypoint_y[0] == PlayersData[faction_id].goals[k].y))) {
                                    goto L_572c77;
                                }
                            }
                            go_to(veh_id, 0x67, PlayersData[faction_id].goals[k].x,
                                PlayersData[faction_id].goals[k].y);
                            break;
                        }
                    }
                }
L_572c77:
                ;
            }
        }
    }
    // An order 0x18 unit arrives here with base_id from earlier code.
    if (j && !own_ground) {
        if (((abs(y - Bases[base_id].y) + x_dist(x, Bases[base_id].x)) & ~1) > 4
            && Vehs[veh_id].order != ORDER_MOVE_TO) {
            go_to(veh_id, 0x68, Bases[base_id].x, Bases[base_id].y);
            if (Vehs[veh_id].order == ORDER_MOVE_TO) goto L_57707f;
        }
    }
L_572d66:
    if (triad == 1 && Vehs[veh_id].order != ORDER_MOVE_TO) {
        best_score = 9999999;
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            if (Bases[base_id].faction_id_current != faction_id) continue;
            if (j) {
                if (Vehs[veh_id].home_base_id >= 0) {
                    if (base_id != Vehs[veh_id].home_base_id) continue;
                } else if (base_id != home_base) {
                    continue;
                }
            }
            for (hint = 0; hint < 21; hint++) {
                tx = xrange(Bases[base_id].x + RadiusOffsetX[hint]);
                ty = Bases[base_id].y + RadiusOffsetY[hint];
                if (on_map(tx, ty) && is_ocean(tx, ty) && (map_loc(tx, ty)->climate & ~0x1F) >= 0x40) {
                    owner = anything_at(tx, ty);
                    if ((owner < 0 || owner == faction_id
                         || (PlayersData[faction_id].diplo_treaties[owner] & 1))
                        && can_terraform(faction_id, tx, ty, 0, base_id)) {
                        dist = vector_dist(x_dist(x, tx), abs(y - ty)) * 8;
                        if (is_ocean(Bases[base_id].x, Bases[base_id].y)) dist /= 2;
                        if (base_id == Vehs[veh_id].home_base_id) dist /= 2;
                        if (bonus_at(tx, ty, 0)) dist /= 2;
                        for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
                            if (Vehs[other_id].faction_id == faction_id
                                && VehPrototypes[Vehs[other_id].proto_id].plan == 9
                                && other_id != veh_id
                                && ((Vehs[other_id].x == tx && Vehs[other_id].y == ty)
                                    || (Vehs[other_id].order == ORDER_MOVE_TO
                                        && Vehs[other_id].waypoint_x[0] == tx
                                        && Vehs[other_id].waypoint_y[0] == ty))) {
                                goto L_572ffa;
                            }
                        }
                        if (dist < best_score) {
                            best_score = dist;
                            go_to(veh_id, 0x6F, tx, ty);
                        }
                    }
                }
L_572ffa:
                ;
            }
        }
    }
    // 0x57302d
    own_ground = 0;
    if (!PlayersData[faction_id].current_num_bases) {
        for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
            if (Vehs[other_id].faction_id == faction_id
                && VehPrototypes[Vehs[other_id].proto_id].plan == 8) {
                go_to(veh_id, 0x23, Vehs[other_id].x, Vehs[other_id].y);
                goto L_57707f;
            }
        }
    }
    k = can_terraform(faction_id, x, y, at_goal(faction_id, 9, x, y) || j, home_base);
    if (!near_hostile
        && (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == 2
            || at_sea == (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == 1))
        && home_dist) {
        del_site(faction_id, at_sea ? 0x19 : 9, x, y, 0);
        if (at_goal(faction_id, 0x79, x, y) && !is_sensor(x, y)) {
            Vehs[veh_id].order = 0xD;
            goto L_578f46;
        }
        if (at_goal(faction_id, 0x29, x, y) && !j && alt <= 5) {
            PlayersData[faction_id].energy_reserves = range(PlayersData[faction_id].energy_reserves
                - range(terraform_cost(x, y, faction_id), 0, PlayersData[faction_id].energy_reserves / 4),
                0, 99999);
            Vehs[veh_id].order = 0x14;
            del_site(faction_id, 8, x, y, 1);
            goto L_578f46;
        }
        if (at_goal(faction_id, 0x49, x, y) && terrain_avail(TERRA_CONDENSER, at_sea, faction_id)) {
            Vehs[veh_id].order = 0x10;
            goto L_578f46;
        }
        if (at_goal(faction_id, 0xD, x, y) && terrain_avail(TERRA_ECH_MIRROR, at_sea, faction_id)) {
            Vehs[veh_id].order = 0x11;
            goto L_578f46;
        }
        if (at_goal(faction_id, 0x69, x, y) && terrain_avail(TERRA_THERMAL_BORE, at_sea, faction_id)) {
            Vehs[veh_id].order = 0x12;
            goto L_578f46;
        }
        if (!at_sea && !j
            && !(faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
            && terrain_avail(TERRA_RAISE_LAND, at_sea, faction_id)
            && (Continents[region].tile_count < 50
                || (Continents[region].tile_count < 150 && BaseFindDist >= 4))
            && near_dist >= 2
            && (map_loc(x, y)->climate & ~0x1F) < 0xC0) {
            {
                int r;
                for (r = 0; r < 49; r++) {
                    tx = xrange(x + RadiusOffsetX[r]);
                    ty = y + RadiusOffsetY[r];
                    if (on_map(tx, ty) && (map_loc(tx, ty)->climate & ~0x1F) >= 0xC0) goto L_5733d7;
                }
            }
            Vehs[veh_id].order = 0x14;
            goto L_578f46;
        }
L_5733d7:
        if (home_dist <= 2
            && ((map_loc(x, y)->bit & BIT_MINE) || (map_loc(x, y)->bit & BIT_FOREST))
            && !(map_loc(x, y)->bit & BIT_ROAD)
            && (!(map_loc(x, y)->bit & BIT_FUNGUS) || (map_loc(x, y)->climate & ~0x1F) < 0x40)
            && !at_sea
            && (!(faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                || !(GameDataStateStorage.fields_.game_more_preferences_ & 0x400000))) {
            Vehs[veh_id].order = 9;
            goto L_578f46;
        }
        if (home_dist <= 2) {
            m = radius_move(Bases[home_base].x, Bases[home_base].y, x, y, 21);
        } else {
            m = 0;
        }
        i = (at_sea ? at_goal(faction_id, 0x19, x, y) : at_goal(faction_id, 9, x, y))
            || (Vehs[veh_id].waypoint_x[0] == x && Vehs[veh_id].waypoint_y[0] == y)
            || stack_check(veh_id, 9, 3, -1, -1)
            || (m && (Bases[home_base].resource_sqr_radius & (1 << m)));
        if ((map_loc(x, y)->bit & BIT_FUNGUS) && (map_loc(x, y)->climate & ~0x1F) >= 0x40 && k
            && (has_abil(Vehs[veh_id].proto_id, 0x2000)
                || has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[is_ocean(x, y)], faction_id))
            && (i || (home_dist <= 2
                      && (Bases[home_base].eco_damage || Bases[home_base].population_size >= 6)))) {
            Vehs[veh_id].order = 0xE;
            goto L_578f46;
        }
        // BUG(original): `&&` where `&` was meant; any nonzero resource word
        // and any m pass (the shifted bit itself is tested).
        if (k == 4 && (i || (m && Bases[home_base].resource_sqr_radius && (1 << m)))) {
            Vehs[veh_id].order = 4;
            Vehs[veh_id].move_to_ai_type = 0x2B;
            goto L_578f46;
        }
        if (!at_sea
            && (!(faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                || !(GameDataStateStorage.fields_.game_more_preferences_ & 0x400000))
            && (i || (home_dist <= 2
                      && ((map_loc(x, y)->bit & BIT_MINE) || (map_loc(x, y)->bit & BIT_FARM)
                          || (((map_loc(x, y)->bit & BIT_FOREST) || k)
                              && (!(map_loc(x, y)->bit & BIT_FUNGUS)
                                  || (map_loc(x, y)->climate & ~0x1F) < 0x40)))))) {
            Map *tile = map_loc(x, y);
            uint32_t bits = tile->bit;
            if (bits & BIT_ROAD) {
                if (has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0], faction_id)
                    && i && !(bits & BIT_MAGTUBE)) {
                    Vehs[veh_id].order = 9;
                    Vehs[veh_id].move_to_ai_type = 0x52;
                    goto L_578f46;
                }
            } else if (!(bits & BIT_FUNGUS) || (tile->climate & ~0x1F) < 0x40
                       || has_tech(Rules.tech_improve_fungus_sqr, faction_id)) {
                Vehs[veh_id].order = 9;
                Vehs[veh_id].move_to_ai_type = 0x72;
                goto L_578f46;
            }
        }
        if (k && (home_dist <= 2 || stack_check(veh_id, 9, 3, -1, -1))
            && (i || !Vehs[veh_id].order)) {
            Vehs[veh_id].order = (int8_t)k;
            Vehs[veh_id].move_to_ai_type = 0x2B;
            goto L_578f46;
        }
        if ((!(faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
             || (GameDataStateStorage.fields_.game_more_preferences_ & 0x20))
            && (i || !Vehs[veh_id].order)) {
            if (is_sensor(x, y) && whose_territory(faction_id, x, y, 0, 0) == faction_id
                && home_dist > 1 && !(map_loc(x, y)->bit & BIT_BUNKER)
                && has_tech(Terraforming[TERRA_BUNKER].prerequisite_tech[0], faction_id)) {
                Vehs[veh_id].order = 0xB;
                goto L_578f46;
            }
            if (good_sensor(faction_id, x, y)) {
                Vehs[veh_id].order = 0xD;
                goto L_578f46;
            }
            if (home_dist > 1 && home_dist <= 3
                && (map_loc(x, y)->bit & BIT_ROAD) && !(map_loc(x, y)->bit & BIT_BUNKER)
                && has_tech(Terraforming[TERRA_BUNKER].prerequisite_tech[0], faction_id)) {
                {
                    int r;
                    for (r = 0; r < 9; r++) {
                        tx = xrange(x + RadiusOffsetX[r]);
                        ty = y + RadiusOffsetY[r];
                        if (on_map(tx, ty) && (map_loc(tx, ty)->bit & BIT_BUNKER)) goto L_573942;
                    }
                }
                Vehs[veh_id].order = 0xB;
                goto L_578f46;
            }
L_573942:
            if (i && has_tech(Terraforming[TERRA_SENSOR].prerequisite_tech[0], faction_id)) {
                tx = x;
                ty = y;
                own_ground = 1;
                if (!Paths.sensors(faction_id, &tx, &ty) && x == tx && y == ty) {
                    Vehs[veh_id].order = 0xD;
                    goto L_578f46;
                }
            }
        }
    }
    // 0x5739aa
    if (Vehs[veh_id].order == ORDER_MOVE_TO || near_hostile) goto L_5741cf;
    if (j) {
        base_id = Vehs[veh_id].home_base_id;
        alt = Bases[base_id].x;
        n = Bases[base_id].y;
    } else {
        base_id = home_base;
        if (Vehs[veh_id].state & 0x200) {
            alt = Bases[home_base].x;
            n = Bases[home_base].y;
        } else {
            alt = x;
            n = y;
        }
        score = 0;
        {
            int r;
            for (r = 1; r < 21; r++) {
                tx = xrange(Bases[home_base].x + RadiusOffsetX[r]);
                ty = Bases[home_base].y + RadiusOffsetY[r];
                if (on_map(tx, ty) && map_loc(tx, ty)->region == region) {
                    other_id = veh_at(tx, ty);
                    if (other_id >= 0 && Vehs[other_id].faction_id == faction_id) {
                        score += stack_check(other_id, 2, 9, -1, -1);
                        if (tx == x && ty == y) score--;
                    }
                }
            }
        }
        if (score) {
            best_score = 9999;
            for (m = 0; m < GameDataStateStorage.fields_.base_current_count_; m++) {
                if (Bases[m].faction_id_current == faction_id
                    && map_loc(Bases[m].x, Bases[m].y)->region == region) {
                    int cnt = 0;
                    for (k = 0; k < 25; k++) {
                        int bx = xrange(Bases[m].x + RadiusOffsetX[k]);
                        int by = Bases[m].y + RadiusOffsetY[k];
                        if (on_map(bx, by) && map_loc(bx, by)->region == region) {
                            other_id = veh_at(bx, by);
                            if (other_id >= 0) cnt += stack_check(other_id, 2, 9, -1, -1);
                        }
                    }
                    if (cnt < score) {
                        dist = vector_dist(x_dist(x, Bases[m].x), abs(y - Bases[m].y)) * (cnt + 1) * (cnt + 1);
                        if (dist < best_score) {
                            best_score = dist;
                            base_id = m;
                        }
                    }
                }
            }
        }
    }
    // 0x573ca0
    if (base_id >= 0) {
        best_score = 9999;
        for (i = 1; i < 21; i++) {
            {
                int r = (GameDataStateStorage.fields_.turn_current_num_ + i) % 21;
                tx = xrange(alt + RadiusOffsetX[r]);
                ty = n + RadiusOffsetY[r];
            }
            if (on_map(tx, ty) && base_who(tx, ty) < 0
                && is_ocean(tx, ty) == (triad == 1)
                && (map_loc(tx, ty)->region == region || at_sea != is_ocean(tx, ty))
                && !zoc_move(tx, ty, faction_id)) {
                other_id = veh_at(tx, ty);
                if ((other_id < 0
                     || (Vehs[other_id].faction_id == faction_id
                         && !stack_check(other_id, 2, 9, -1, -1)))
                    && can_terraform(faction_id, tx, ty,
                           (faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) && j,
                           base_id)) {
                    score = vector_dist(x_dist(Bases[base_id].x, tx), abs(Bases[base_id].y - ty));
                    if (score <= 2) {
                        // BUG(original): tests the worked-tile bit of the loop
                        // counter i, not of the rotated radius index.
                        if (!(Bases[base_id].resource_sqr_radius & (1 << i))
                            && (!(map_loc(tx, ty)->bit & BIT_FUNGUS)
                                || (map_loc(tx, ty)->climate & ~0x1F) < 0x40)) {
                            score *= 3;
                        }
                        if (score < best_score) {
                            for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
                                if (Vehs[other_id].faction_id == faction_id
                                    && VehPrototypes[Vehs[other_id].proto_id].plan == 9
                                    && other_id != veh_id
                                    && ((Vehs[other_id].x == tx && Vehs[other_id].y == ty)
                                        || (Vehs[other_id].order == ORDER_MOVE_TO
                                            && Vehs[other_id].waypoint_x[0] == tx
                                            && Vehs[other_id].waypoint_y[0] == ty))) {
                                    goto L_573f3c;
                                }
                            }
                            best_score = score;
                            go_to(veh_id, 0x77, tx, ty);
                        }
                    }
                }
            }
L_573f3c:
            ;
        }
        if (Vehs[veh_id].order == ORDER_MOVE_TO) goto L_57707f;
    }
    // 0x573f61
    if (own_ground) goto L_57408e;
    if ((faction_bit & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
        && !(GameDataStateStorage.fields_.game_more_preferences_ & 0x20)) {
        goto L_57408e;
    }
    tx = x;
    ty = y;
    if (Paths.sensors(faction_id, &tx, &ty)) goto L_57408e;
    if (tx == x && ty == y) {
        Vehs[veh_id].order = 0xD;
        goto L_578f46;
    }
    // falls through into chunk 5 at 0x573fd0
    // ---- chunk 5: 0x573fd0-0x575b27 ----
L_573fd0:
    if (!j || vector_dist(x_dist(x, tx), abs(y - ty)) <= 2) {
        for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
            if (Vehs[other_id].faction_id == faction_id
                && VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_TERRAFORMING
                && other_id != veh_id
                && ((Vehs[other_id].x == tx && Vehs[other_id].y == ty)
                    || (Vehs[other_id].order == ORDER_MOVE_TO
                        && Vehs[other_id].waypoint_x[0] == tx
                        && Vehs[other_id].waypoint_y[0] == ty))) {
                goto L_57408e;  // another former already has this tile
            }
        }
        go_to(veh_id, 's', tx, ty);
        goto L_57707f;
    }
L_57408e:
    if (home_dist && !at_sea && PlayersData[faction_id].region_total_bases[region]) {
        int b = (Vehs[veh_id].home_base_id >= 0) ? Vehs[veh_id].home_base_id : home_base;
        if (b >= 0) {
            if (map_loc(Bases[b].x, Bases[b].y)->region != region) {
                b = home_base;
            }
            if (b >= 0) {
                if (vector_dist(x_dist(x, Bases[b].x), abs(y - Bases[b].y))
                        > (GameDataStateStorage.fields_.turn_current_num_ <= 50) + 3
                    || zoc_moved) {
                    go_to(veh_id, 'b', Bases[b].x, Bases[b].y);
                    goto L_57707f;
                }
            }
        }
    }
    if (j && !Vehs[veh_id].order) {
        Vehs[veh_id].state &= ~0x200;
        Vehs[veh_id].order = ORDER_NONE;
        return 0;
    }
L_5741cf:
    if (region_plan > 2
        && PlayersData[faction_id].unk_79[region] >= Continents[region].tile_count
        && plan <= PLAN_RECONNAISANCE
        && !triad
        && Vehs[veh_id].order != ORDER_MOVE_TO
        && !at_sea) {
        if (coast_or_border(x, y, x, y, faction_id)) {
            int dir = (Vehs[veh_id].facing + 4) % 8;
            tx = xrange(x + VehMoveDeltaX[dir]);
            ty = y + VehMoveDeltaY[dir];
            if (!coast_or_border(tx, ty, tx, ty, faction_id)) {
                if (veh_id & 1) {
                    Vehs[veh_id].state |= 0x80000000;
                } else {
                    Vehs[veh_id].state &= 0x7FFFFFFF;
                }
            }
            j = 0;
            hint = 0;
            for (own_ground = 3; own_ground > -5; hint++, own_ground--) {
                dir = (Vehs[veh_id].facing
                       + ((Vehs[veh_id].state & 0x80000000) ? hint - 3 : own_ground) + 8) % 8;
                tx = xrange(x + VehMoveDeltaX[dir]);
                ty = y + VehMoveDeltaY[dir];
                if (!j) {
                    if (on_map(tx, ty)
                        && (map_loc(tx, ty)->region != region
                            || whose_territory(faction_id, tx, ty, NULL, 0) != faction_id)) {
                        j = 1;
                    }
                } else if (on_map(tx, ty)
                           && map_loc(tx, ty)->region == region
                           && whose_territory(faction_id, tx, ty, NULL, 0) == faction_id) {
                    best_dir = dir;
                    Vehs[veh_id].move_to_ai_type = 'p';
                    goto L_578b26;
                }
            }
        }
        best_score = 0;
        for (ty = 0; ty < MapLatitudeBounds; ty++) {
            for (tx = ty & 1; tx < MapLongitudeBounds; tx += 2) {
                if (map_loc(tx, ty)->region == region
                    && whose_territory(faction_id, tx, ty, NULL, 0) == faction_id) {
                    score = 1;
                    // n is strength-reduced to a byte offset (0..0x24) in the image.
                    for (n = 0; n < 9; n++) {
                        int xx = xrange(tx + RadiusOffsetX[n]);
                        int yy = ty + RadiusOffsetY[n];
                        if (on_map(xx, yy)) {
                            if (map_loc(xx, yy)->region != region) {
                                score++;
                            } else {
                                if (whose_territory(faction_id, xx, yy, NULL, 0) != faction_id) {
                                    score += 3;
                                }
                                if (anything_at(xx, yy) == faction_id) {
                                    score -= 6;
                                }
                            }
                        }
                    }
                    int value = ((abs(y - ty) + x_dist(x, tx)) >> 1) * score;
                    for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
                        if (Vehs[other_id].faction_id == faction_id
                            && Vehs[other_id].waypoint_x[0] == tx
                            && Vehs[other_id].waypoint_y[0] == ty) {
                            value /= 2;
                        }
                    }
                    if (value > best_score) {
                        best_score = value;
                        go_to(veh_id, 'P', tx, ty);
                    }
                }
            }
        }
    }
    if (state_4000 || (plan == PLAN_RECONNAISANCE && GameDataStateStorage.fields_.turn_current_num_ < 100)) {
L_57460a:
        if (triad != TRIAD_AIR
            && (Vehs[veh_id].order != ORDER_MOVE_TO || Vehs[veh_id].move_to_ai_type != '/')) {
            own_ground = 0;   // best tile's raw value
            best_score = 0;
            tile_owner = 0;   // previous ring's best score
            for (m = 0; m < 9; m++) {
                k = 0;        // this ring's best score
                for (i = m ? RadiusRange[m - 1] : 0; i < RadiusRange[m]; i++) {
                    tx = xrange(x + RadiusOffsetX[i]);
                    ty = y + RadiusOffsetY[i];
                    if (on_map(tx, ty) && map_loc(tx, ty)->region == region) {
                        owner = whose_territory(faction_id, tx, ty, NULL, 0);
                        if (owner < 0 || owner == faction_id
                            || !(PlayersData[faction_id].diplo_treaties[owner] & 7)) {
                            int value = is_known(tx, ty, faction_id) ? (goody_at(tx, ty) ? 512 : 0) : 0;
                            for (j = 0; j < 9; j++) {
                                // BUG(original): offsets by i, not j, so the same
                                // neighbour is examined nine times.
                                int xx = xrange(tx + RadiusOffsetX[i]);
                                int yy = ty + RadiusOffsetY[i];
                                if (on_map(xx, yy)) {
                                    if (is_known(xx, yy, faction_id)) {
                                        owner = veh_who(xx, yy);
                                        if (owner >= 0 && owner != faction_id) {
                                            value = 0;
                                            break;
                                        }
                                    } else {
                                        value++;
                                        if (map_loc(xx, yy)->region == region) {
                                            value += 15;
                                        }
                                    }
                                }
                            }
                            if ((map_loc(tx, ty)->bit & BIT_FUNGUS)
                                && (map_loc(tx, ty)->climate & 0xE0) >= 0x40
                                && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET] <= 0) {
                                value /= 2;
                            }
                            alt = value;
                            if (home_base >= 0) {
                                // max(0, 16 - dist): the macro evaluates the distance twice.
                                value += (0 > 16 - ((abs(ty - Bases[home_base].y) + x_dist(tx, Bases[home_base].x)) >> 1))
                                    ? 0
                                    : 16 - ((abs(ty - Bases[home_base].y) + x_dist(tx, Bases[home_base].x)) >> 1);
                            }
                            score = value * 256 / (((abs(y - ty) + x_dist(x, tx)) >> 1) + 2);
                            if (score > k) {
                                k = score;
                            }
                            if (score > best_score) {
                                for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
                                    if (Vehs[other_id].faction_id == faction_id
                                        && other_id != veh_id
                                        && Vehs[other_id].order == ORDER_MOVE_TO) {
                                        dist = (abs(ty - Vehs[other_id].waypoint_y[0])
                                                + x_dist(tx, Vehs[other_id].waypoint_x[0])) >> 1;
                                        if (dist <= 1
                                            && ((abs(y - ty) + x_dist(x, tx)) >> 1)
                                                >= ((abs(Vehs[other_id].y - ty) + x_dist(Vehs[other_id].x, tx)) >> 1) - 1) {
                                            goto L_574a23;  // a closer explorer already heads there
                                        }
                                    }
                                }
                                if (score) {
                                    best_score = score;
                                    own_ground = alt;
                                    go_to(veh_id, '/', tx, ty);
                                }
                            }
                        }
                    }
                L_574a23:;
                }
                if (Vehs[veh_id].order == ORDER_MOVE_TO && k < tile_owner && own_ground >= 16) {
                    break;
                }
                tile_owner = k;
            }
            if (Vehs[veh_id].order == ORDER_MOVE_TO) {
                goto L_57707f;
            }
        }
    }
L_574a7e:
    if (triad != TRIAD_SEA) {
        goto L_577071;
    }
L_574a88:
    if (plan == PLAN_NAVAL_SUPERIORITY) {
        del_site(faction_id, plan, x, y, 0);
    }
    hint = 0;
    k = 0;
    i = 0;
    m = 0;
    j = 0;
    if (plan != PLAN_NAVAL_TRANSPORT) {
        goto L_5767ed;
    }
    {
        int count = 0;  // passengers released to act on their own (ebx)
        if (!home_dist) {
            stack_sort_2(veh_id);
        }
        n = 0;
        // Step is veh_next_stack_id (0x0050B8F0) open-coded: the image re-tests the id.
        for (other_id = veh_top(veh_id); other_id >= 0;
             other_id = (other_id < 0) ? other_id : Vehs[other_id].next_veh_id_stack) {
            if (other_id == veh_id) {
                continue;
            }
            if (Chassis[VehPrototypes[Vehs[other_id].proto_id].chassis_id].triad != TRIAD_LAND) {
                continue;
            }
            if (veh_cargo(other_id)) {
                continue;
            }
            if (Vehs[other_id].faction_id != faction_id) {
                continue;
            }
            if ((Vehs[other_id].state & 0x40000) && !near_dist
                && (!(Vehs[other_id].state & 0x20000) || region_plan != 7)) {
                continue;
            }
            {
                int healthy = Vehs[other_id].dmg_incurred <= proto_power(other_id) / 3;
                if (Vehs[other_id].order != ORDER_HOLD) {
                    if (at_sea) {
                        if ((Vehs[other_id].order == ORDER_SENTRY_BOARD
                             && (Vehs[other_id].waypoint_x[0] < 0 || Vehs[other_id].waypoint_x[0] == veh_id))
                            || (Vehs[other_id].order == ORDER_NONE && near_dist)) {
                            count++;
                            if (healthy) {
                                i |= (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_COMBAT)
                                    ? 7 : (1 << VehPrototypes[Vehs[other_id].proto_id].plan);
                            } else {
                                m++;
                            }
                            continue;
                        }
                    } else if (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_ALIEN_ARTIFACT) {
                        Vehs[other_id].order = ORDER_HOLD;
                        continue;
                    }
                    if (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_TERRAFORMING) {
                        continue;
                    }
                    if (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_DEFENSIVE
                        && k < keep_count && !home_dist) {
                        k++;
                        Vehs[other_id].order = ORDER_HOLD;
                    } else if (!home_dist && !healthy) {
                        Vehs[other_id].order = ORDER_HOLD;
                    } else {
                        count++;
                        Vehs[other_id].order = ORDER_NONE;
                        if (healthy) {
                            i |= (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_COMBAT)
                                ? 7 : (1 << VehPrototypes[Vehs[other_id].proto_id].plan);
                        }
                    }
                } else {
                    if (home_dist) {
                        continue;
                    }
                    if (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_ALIEN_ARTIFACT && !at_sea) {
                        continue;
                    }
                    if (!healthy) {
                        continue;
                    }
                    if (VehPrototypes[Vehs[other_id].proto_id].plan <= PLAN_COLONIZATION
                        && (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_DEFENSIVE
                            || (Vehs[other_id].state & 0x10000000)
                            || (!stack_check(other_id, 2, 2, -1, -1)
                                && (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_COMBAT
                                    || VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_RECONNAISANCE))
                            || !stack_check(other_id, 16, -1, -1, -1))) {
                        if (++k <= keep_count) {
                            continue;
                        }
                        if (region_plan <= 2 && n) {
                            continue;
                        }
                        n++;
                    }
                    count++;
                    Vehs[other_id].order = ORDER_NONE;
                    i |= (VehPrototypes[Vehs[other_id].proto_id].plan == PLAN_COMBAT)
                        ? 7 : (1 << VehPrototypes[Vehs[other_id].proto_id].plan);
                }
            }
        }
        hint = count;
        if (!count) {
            if (Vehs[veh_id].order == ORDER_MOVE_TO
                && (Vehs[veh_id].move_to_ai_type == 'J' || Vehs[veh_id].move_to_ai_type == 'r'
                    || Vehs[veh_id].move_to_ai_type == 'I' || Vehs[veh_id].move_to_ai_type == 'E')) {
                Vehs[veh_id].order = ORDER_NONE;
            }
        }
        // i holds 1 << plan of the released passengers (combat sets 7).
        if ((i & 0xF) || (i & 0x800)) {
            j = 2;
            if ((i & 0x100) && !(i & 3)) {
                j = 1;
            }
        }
        if (Vehs[veh_id].move_to_ai_type == 'E' || Vehs[veh_id].move_to_ai_type == 'r'
            || Vehs[veh_id].move_to_ai_type == 'I') {
            if (!j || Vehs[veh_id].order != ORDER_MOVE_TO) {
                Vehs[veh_id].move_to_ai_type = '-';
            }
        }
        if (!stay && !(Vehs[veh_id].order == ORDER_MOVE_TO && Vehs[veh_id].move_to_ai_type == 'd')) {
            if (((i & 0x1000) && near_dist)
                || (m && m >= hint
                    && (near_dist <= 8 || GameDataStateStorage.fields_.turn_current_num_ < 50))) {
                best_score = 9999;
                for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                    if (Bases[base_id].faction_id_current == faction_id && base_on_sea(base_id, region)) {
                        dist = vector_dist(x_dist(x, Bases[base_id].x), abs(y - Bases[base_id].y));
                        if (!dist) {
                            goto L_578b34;
                        }
                        if (Bases[base_id].state & 0x400) {
                            dist += 1000;
                        }
                        if (dist < best_score) {
                            best_score = dist;
                            go_to(veh_id, 'a', Bases[base_id].x, Bases[base_id].y);
                        }
                    }
                }
            }
            {
                int space = veh_cargo(veh_id) - hint;
                if ((Vehs[veh_id].state & 0x20000000) && !hint) {
                    space = 0;
                }
                if (space == veh_cargo(veh_id)
                    || (!home_dist && space >= veh_cargo(veh_id) / 2 && !(i & 0x100))) {
                    best_score = 9999;
                    tile_owner = 0;
                    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                        if (Bases[base_id].faction_id_current != faction_id || !is_port(base_id, 0)) {
                            continue;
                        }
                        faction_bit = map_loc(Bases[base_id].x, Bases[base_id].y)->region;  // port's sea region
                        if (PlayersData[faction_id].region_base_plan[faction_bit] == 2) {
                            continue;
                        }
                        dist = k = (abs(y - Bases[base_id].y) + x_dist(x, Bases[base_id].x)) >> 1;
                        if (PlayersData[faction_id].x_target >= 0
                            && faction_bit == map_loc(PlayersData[faction_id].x_target,
                                                      PlayersData[faction_id].y_target)->region) {
                            own_ground = 1;
                            if (!(PlayersData[faction_id].flags & 0x800)
                                || Bases[base_id].x != PlayersData[faction_id].x_target
                                || Bases[base_id].y != PlayersData[faction_id].y_target
                                || stack_check(veh_at(Bases[base_id].x, Bases[base_id].y), 2, 7, -1, -1)
                                    >= ((k && (Vehs[veh_id].waypoint_x[0] != Bases[base_id].x
                                               || Vehs[veh_id].waypoint_y[0] != Bases[base_id].y)) ? 1 : 2)) {
                                continue;
                            }
                        } else if (!Bases[base_id].unk_1
                                   || (Vehs[veh_id].order == ORDER_MOVE_TO
                                       && Vehs[veh_id].waypoint_x[0] == Bases[base_id].x
                                       && Vehs[veh_id].waypoint_y[0] == Bases[base_id].y)
                                   || (!home_dist && home_base == base_id
                                       && stack_check(veh_id, 2, 7, -1, -1) <= 1)) {
                            if (stack_check(veh_at(Bases[base_id].x, Bases[base_id].y), 2, 7, -1, -1)
                                >= ((k && (Vehs[veh_id].waypoint_x[0] != Bases[base_id].x
                                           || Vehs[veh_id].waypoint_y[0] != Bases[base_id].y)) ? 1 : 2)) {
                                continue;
                            }
                            own_ground = 0;
                        } else {
                            continue;
                        }
                        if (!home_dist) {
                            if (!port_to_port(home_base, base_id)) {
                                continue;
                            }
                        } else if (!base_on_sea(base_id, region)) {
                            continue;
                        }
                        if (PlayersData[faction_id].region_total_combat_vehs[faction_bit]
                                <= PlayersData[faction_id].region_total_bases[faction_bit] * 2 + 1
                            && !(PlayersData[faction_id].unk_77[faction_bit] & 3)
                            && !(PlayersData[faction_id].unk_77[faction_bit] & 0x1800)) {
                            continue;
                        }
                        dist = k + 4;
                        if (PlayersData[faction_id].region_base_plan[faction_bit] != 7
                            && !(PlayersData[faction_id].unk_77[faction_bit] & 0x800)
                            && !(Vehs[veh_id].waypoint_x[0] == Bases[base_id].x
                                 && Vehs[veh_id].waypoint_y[0] == Bases[base_id].y)) {
                            if ((GameDataStateStorage.fields_.turn_current_num_ + base_id + veh_id) & 7) {
                                continue;
                            }
                            dist *= 2;
                        }
                        if (stack_check(veh_at(Bases[base_id].x, Bases[base_id].y), 2, 7, faction_id, -1) > !k) {
                            dist *= 2;
                        }
                        dist += PlayersData[faction_id].region_total_bases[faction_bit] * 2
                            - PlayersData[faction_id].region_total_combat_vehs[faction_bit];
                        if (dist < best_score) {
                            best_id = base_id;
                            best_score = dist;
                            n = PlayersData[faction_id].region_base_plan[faction_bit];
                            tile_owner = own_ground;
                            go_to(veh_id, 'e', Bases[base_id].x, Bases[base_id].y);
                        }
                    }
                    {
                        int limit = (tile_owner || n == 7) ? 6 : 2;
                        limit = range(limit, 0, veh_cargo(veh_id));
                        if (tile_owner || best_score <= veh_moves(veh_id) * limit / 2) {
                            assemble_passengers(best_id, n, limit);
                        }
                    }
                }
            }
        }
        if (Vehs[veh_id].move_to_ai_type == 'e'
            && Vehs[veh_id].waypoint_x[0] == x && Vehs[veh_id].waypoint_y[0] == y) {
            Bases[near_base].state |= 1;
        }
        if (hint > m && Vehs[veh_id].move_to_ai_type != 'E') {
            // n is strength-reduced to a byte offset (0..0x20) in the image.
            for (n = 0; n < 8; n++) {
                tx = xrange(x + VehMoveDeltaX[n]);
                ty = y + VehMoveDeltaY[n];
                if (!on_map(tx, ty)) {
                    continue;
                }
                if (is_ocean(tx, ty)) {
                    continue;
                }
                faction_bit = map_loc(tx, ty)->region;  // adjacent land region
                if (bad_reg(faction_bit)) {
                    continue;
                }
                m = damage > 2;   // unload flag from here on
                owner = whose_territory(faction_id, tx, ty, NULL, 0);
                if (owner >= 0 && owner != faction_id) {
                    if (PlayersData[faction_id].diplo_treaties[owner] & 1) {
                        continue;
                    }
                    if (PlayersData[faction_id].diplo_treaties[owner] & 0xC10) {
                        if (j) {
                            m = 1;
                        }
                    } else {
                        if (PlayersData[faction_id].diplo_treaties[owner] & 2) {
                            if (!((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << owner))
                                || !j
                                || GameDataStateStorage.fields_.turn_current_num_
                                    - PlayersData[owner].diplo_spoke[faction_id] < 16) {
                                continue;
                            }
                            diplomacy_check(owner, faction_id, 0);
                            if (!DiploScratchFA60 && !DiploScratchFA3C) {
                                continue;
                            }
                        } else if (PlayersData[faction_id].diplo_treaties[owner] & 4) {
                            if (!((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (uint8_t)(1 << owner))
                                || GameDataStateStorage.fields_.turn_current_num_
                                    - PlayersData[owner].diplo_spoke[faction_id] < 16) {
                                continue;
                            }
                        }
                        if (!(PlayersData[faction_id].diplo_treaties[owner] & 8) && j && !(i & 0x100)) {
                            m = 1;
                        }
                    }
                }
                owner = veh_who(tx, ty);
                if (owner >= 0 && owner != faction_id) {
                    continue;
                }
                if ((i & 0x100)
                    && (int)Continents[faction_bit].open_terrain >= 4
                    && (!PlayersData[faction_id].region_total_bases[faction_bit]
                        || PlayersData[faction_id].region_base_plan[faction_bit] == 8)) {
                    if (j || PlayersData[faction_id].region_base_plan[region] > 2) {
                        m = 1;
                    }
                }
                if (j) {
                    if (!m && !PlayersData[faction_id].region_total_bases[faction_bit]
                        && PlayersData[faction_id].region_base_plan[faction_bit] == 8) {
                        for (owner = 1; owner < 8; owner++) {
                            if (PlayersData[owner].region_total_bases[faction_bit]
                                && ((PlayersData[faction_id].diplo_treaties[owner] & 0x10)
                                    || !(PlayersData[faction_id].diplo_treaties[owner] & 8)
                                    || GameDataStateStorage.fields_.turn_current_num_
                                        - PlayersData[owner].diplo_spoke[faction_id] > 16)) {
                                m = 1;
                            }
                        }
                    }
                    if (PlayersData[faction_id].region_base_plan[faction_bit] <= 2) {
                        m = 1;
                    }
                }
                if (!m) {
                    continue;
                }
                if (!(i & (1 << PlayersData[faction_id].region_base_plan[faction_bit]))
                    && damage <= 2
                    && !(GameDataStateStorage.fields_.game_state_ & 0x100)
                    && (Vehs[veh_id].order != ORDER_MOVE_TO
                        || map_loc(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0])->region != faction_bit
                        || Vehs[veh_id].move_to_ai_type == 'e')) {
                    for (other_id = veh_top(veh_id); other_id >= 0;
                         other_id = (other_id < 0) ? other_id : Vehs[other_id].next_veh_id_stack) {
                        if (Chassis[VehPrototypes[Vehs[other_id].proto_id].chassis_id].triad == TRIAD_LAND
                            && Vehs[other_id].order == ORDER_SENTRY_BOARD
                            && (Vehs[other_id].waypoint_x[0] < 0 || Vehs[other_id].waypoint_x[0] == veh_id)
                            && Vehs[other_id].home_base_id >= 0
                            && map_loc(Bases[Vehs[other_id].home_base_id].x,
                                       Bases[Vehs[other_id].home_base_id].y)->region == faction_bit) {
                            goto L_5758ee;  // a passenger is going home to this region
                        }
                    }
                }
                if (m) {
                    goto L_575949;
                }
                continue;
            L_5758ee:
                owner = zoc_veh(tx, ty, faction_id);
                if (owner > 1
                    && (((PlayersData[faction_id].diplo_treaties[owner - 1] & 0x10) && j && (i & 3))
                        || !(PlayersData[faction_id].diplo_treaties[owner - 1] & 8))
                    && m) {
                    m = -1;   // unload without choosing a landing direction
                    goto L_575949;
                }
            }
            goto L_575b27;
        L_575949:
            best_dir = -1;
            if (veh_moves(veh_id) >= Rules.move_rate_roads && near_dist && m > 0) {
                best_score = -1;
                for (j = 0; j < 9; j++) {
                    tx = xrange(x + RadiusOffsetX[j]);
                    ty = y + RadiusOffsetY[j];
                    if (on_map(tx, ty) && map_loc(tx, ty)->region == region
                        && (veh_who(tx, ty) < 0 || veh_who(tx, ty) == faction_id)) {
                        score = 0;
                        {
                            int q;
                            for (q = 0; q < 8; q++) {
                                int xx = xrange(tx + RadiusOffsetX[q + 1]);
                                int yy = ty + RadiusOffsetY[q + 1];
                                if (on_map(xx, yy) && map_loc(xx, yy)->region == faction_bit) {
                                    if (base_who(xx, yy) > 0) {
                                        score += 4;
                                    }
                                    score++;
                                }
                            }
                        }
                        if (score > best_score) {
                            best_score = score;
                            best_dir = j - 1;
                        }
                    }
                }
            }
            for (other_id = veh_top(veh_id); other_id >= 0;
                 other_id = (other_id < 0) ? other_id : Vehs[other_id].next_veh_id_stack) {
                if (Chassis[VehPrototypes[Vehs[other_id].proto_id].chassis_id].triad == TRIAD_LAND) {
                    Vehs[other_id].order = ORDER_NONE;
                }
            }
            Vehs[veh_id].move_to_ai_type = 'u';
            if (no_moves) {
                return 0;
            }
            goto L_578b26;
        }
    }
    // ---- chunk 6: 0x575b27-0x577720 ----
L_575b27:
    if (no_moves) goto L_5791ff;
    if (stay) goto L_57717f;
    if (Vehs[veh_id].order == ORDER_MOVE_TO && Vehs[veh_id].move_to_ai_type == 'd') goto L_57717f;
    // 0x575b5d `cmp al, al; je`: the re-test of `order != ORDER_MOVE_TO` below,
    // already known false on that path.
    if (Vehs[veh_id].order != ORDER_MOVE_TO && (i & 0x100) && j < 1) {
        // Pick, per region, a coastal land tile away from the poles.
        memset(coast_y, -1, sizeof(coast_y));
        memset(coast_x, -1, sizeof(coast_x));
        for (ty = 1; ty < MapLatitudeBounds - 1; ty++) {
            for (tx = ty & 1; tx < MapLongitudeBounds; tx += 2) {
                int r = map_loc(tx, ty)->region;
                if (r < 64
                    && (coast_y[r] < 10 || coast_y[r] > MapLatitudeBounds - 10)
                    && is_coast(tx, ty, 0)) {
                    coast_x[r] = tx;
                    coast_y[r] = ty;
                }
            }
        }
        best_id = -1;
        best_score = 0;
        // [ebp-0x44] and [ebp-0x58] hold this loop's strength-reduced
        // induction pointers (&Continents[r], 4*(r-1)); not modelled.
        for (faction_bit = 1; faction_bit < 64; faction_bit++) {
            if (Continents[faction_bit].tile_count >= 10
                && !PlayersData[faction_id].region_total_bases[faction_bit]
                && on_map(coast_x[faction_bit], coast_y[faction_bit])) {
                int count = 0;
                for (owner = 1; owner < 8; owner++) {
                    if (PlayersData[owner].region_total_bases[faction_bit]) {
                        count++;
                        if (PlayersData[owner].region_total_bases[faction_bit] * 20
                            > Continents[faction_bit].tile_count / 4) {
                            count += 2;
                        }
                        if (PlayersData[faction_id].diplo_treaties[owner]
                            & (DTREATY_PACT | DTREATY_TREATY | DTREATY_TRUCE)) {
                            count += 2;
                        }
                    }
                }
                if (count <= 1) {
                    int rating = Continents[faction_bit].tile_count * 100
                        / (vector_dist(x_dist(x, coast_x[faction_bit]),
                                       abs(y - coast_y[faction_bit])) + 1);
                    if (rating >= best_score) {
                        best_score = rating;
                        best_id = faction_bit;
                    }
                }
            }
        }
        if (best_id >= 0) {
            set_course(veh_id, 'c', coast_x[best_id], coast_y[best_id]);
        }
    }
    if (Vehs[veh_id].order != ORDER_MOVE_TO && j) {
        // 0x575dd6: nearest own port on a region planned for defence ('E').
        best_score = 9999;
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            if (Bases[base_id].faction_id_current != faction_id) continue;
            faction_bit = map_loc(Bases[base_id].x, Bases[base_id].y)->region;
            if (PlayersData[faction_id].region_base_plan[faction_bit] != 2) continue;
            if (!home_dist ? !port_to_port(home_base, base_id)
                           : !base_on_sea(base_id, region)) continue;
            dist = (abs(y - Bases[base_id].y) + x_dist(x, Bases[base_id].x)) >> 1;
            if (veh_who(Bases[base_id].x, Bases[base_id].y) < 0) dist /= 2;
            if (at_goal(faction_id, 2, Bases[base_id].x, Bases[base_id].y)) dist /= 2;
            if (PlayersData[faction_id].base_id_atk_target >= 0
                && (PlayersData[faction_id].diplo_treaties[
                        Bases[PlayersData[faction_id].base_id_atk_target].faction_id_current]
                    & DTREATY_VENDETTA)
                && map_loc(Bases[PlayersData[faction_id].base_id_atk_target].x,
                           Bases[PlayersData[faction_id].base_id_atk_target].y)->region
                    == faction_bit) {
                dist = 0;
            }
            if (dist < best_score) {
                best_score = dist;
                go_to(veh_id, 'E', Bases[base_id].x, Bases[base_id].y);
            }
        }
        if (Vehs[veh_id].order != ORDER_MOVE_TO && j > 1) {
            // 0x575fb4: foreign base to raid ('I').
            // [ebp-0x48] holds the &Bases[base_id].faction_id_current induction
            // pointer and [ebp-0x30] the cached faction_id*0x833 treaty row
            // index (0x57602d); compiler temps, not modelled.
            best_score = 9999;
            for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                if (Bases[base_id].faction_id_current == faction_id) continue;
                if (!is_objective(base_id)
                    && !(PlayersData[faction_id].flags & PFLAG_STRAT_ATK_ENEMY_HQ)
                    && PlayersData[faction_id].base_id_atk_target != base_id
                    && ((GameDataStateStorage.fields_.turn_current_num_ + base_id) & 3)) continue;
                if (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                    & DTREATY_PACT) continue;
                if ((PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                     & DTREATY_TREATY)
                    && (!(static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                          & (1 << Bases[base_id].faction_id_current))
                        || !(PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                             & 0xC00))) continue;
                if (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                    & DTREATY_TRUCE) {
                    if (!(static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                          & (1 << Bases[base_id].faction_id_current))) continue;
                    if (!(PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                          & 0xC00)
                        && GameDataStateStorage.fields_.turn_current_num_
                               - PlayersData[Bases[base_id].faction_id_current].diplo_spoke[faction_id]
                           < 16) continue;
                }
                {
                    int r = map_loc(Bases[base_id].x, Bases[base_id].y)->region;
                    if (r == region && !at_sea) continue;
                    if (!home_dist ? !port_to_coast(home_base, r)
                                   : !sea_coast(r, region)) continue;
                    m = 1;
                    for (other_id = veh_top(veh_id); other_id >= 0;
                         other_id = other_id >= 0 ? Vehs[other_id].next_veh_id_stack : -1) {
                        if (Vehs[other_id].home_base_id >= 0
                            && map_loc(Bases[Vehs[other_id].home_base_id].x,
                                       Bases[Vehs[other_id].home_base_id].y)->region == r) {
                            m = 0;
                        }
                    }
                    if (!m
                        && !(PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                             & DTREATY_VENDETTA)
                        && (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                            & DTREATY_COMMLINK)
                        && !(GameDataStateStorage.fields_.game_state_ & 0x100)
                        && !(i & (1 << PlayersData[faction_id].region_base_plan[r]))) continue;
                }
                dist = ((abs(y - Bases[base_id].y) + x_dist(x, Bases[base_id].x)) >> 1) * 32;
                // BUG(original): redundant; foreign bases only reach here.
                if (Bases[base_id].faction_id_current != faction_id
                    && !(Bases[base_id].visibility & (1 << faction_id))) {
                    dist *= 2;
                }
                if (Bases[base_id].queue_production_id[0] == -102) dist /= 4;
                if (has_fac(FAC_HEADQUARTERS, base_id, 0)) {
                    dist /= 2;
                    if (PlayersData[faction_id].flags & PFLAG_STRAT_ATK_ENEMY_HQ) dist /= 2;
                    if (PlayersData[Bases[base_id].faction_id_current].corner_market_turn
                        > GameDataStateStorage.fields_.turn_current_num_) {
                        dist /= 8;
                    }
                }
                if (is_objective(base_id)) {
                    dist /= 2;
                    if (PlayersData[faction_id].flags & PFLAG_STRAT_ATK_OBJECTIVES) dist /= 2;
                }
                if (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                    & DTREATY_UNK_40) {
                    dist /= 2;
                }
                if (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                    & DTREATY_VENDETTA) {
                    dist /= 2;
                    if (veh_who(Bases[base_id].x, Bases[base_id].y) < 0) dist /= 2;
                    if (PlayersData[faction_id].base_id_atk_target == base_id) dist = 0;
                }
                // BUG(original): the headquarters halving is applied twice.
                if (has_fac(FAC_HEADQUARTERS, base_id, 0)) dist /= 2;
                other_id = veh_at(Bases[base_id].x, Bases[base_id].y);
                if ((other_id < 0
                     || stack_check(other_id, 14, -1, -1, -1) < garrison_check(base_id))
                    && (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                        & (1 << Bases[base_id].faction_id_current))
                    && (PlayersData[Bases[base_id].faction_id_current].diff_level >= 4
                        || (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)
                        || (PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                            & DTREATY_HAVE_INFILTRATOR))) {
                    dist /= 2;
                }
                if (dist < best_score) {
                    best_score = dist;
                    set_course(veh_id, 'I', Bases[base_id].x, Bases[base_id].y);
                }
            }
        }
        if (Vehs[veh_id].order != ORDER_MOVE_TO) {
            // 0x576458: own port on another region we want to reinforce ('r').
            best_score = 9999;
            for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                if (Bases[base_id].faction_id_current != faction_id) continue;
                int r = map_loc(Bases[base_id].x, Bases[base_id].y)->region;
                if (region == r) continue;
                int r_plan = PlayersData[faction_id].region_base_plan[r];
                if (r_plan == 7 || (i & 0x100) || !(i & (1 << r_plan))) continue;
                if (!home_dist ? !port_to_port(home_base, base_id)
                               : !base_on_sea(base_id, region)) continue;
                dist = (abs(y - Bases[base_id].y) + x_dist(x, Bases[base_id].x)) >> 1;
                if (dist < best_score) {
                    best_score = dist;
                    go_to(veh_id, 'r', Bases[base_id].x, Bases[base_id].y);
                }
            }
        }
        if (Vehs[veh_id].order != ORDER_MOVE_TO) {
            // 0x57658b: empty land region within radius 2*12 reachable by sea ('J').
            for (int p = 1; p < 289; p++) {
                tx = xrange(x + RadiusOffsetX[p] * 2);
                ty = y + RadiusOffsetY[p] * 2;
                if (!on_map(tx, ty)) continue;
                if ((map_loc(tx, ty)->climate & 0xE0) < 0x60) continue;
                int r = map_loc(tx, ty)->region;
                if (bad_reg(r)) continue;
                if (PlayersData[faction_id].region_total_bases[r]) continue;
                // BUG(original): tests the plan of the unit's own region.
                if (!(i & (1 << PlayersData[faction_id].region_base_plan[region]))) continue;
                if (!home_dist ? port_to_coast(home_base, r) : sea_coast(r, region)) {
                    // Laid out out of line at 0x5767d2.
                    set_course(veh_id, 'J', tx, ty);
                    break;
                }
            }
        }
    }
L_5767ed:
    // Only reachable with stay != 0 from 0x574abe; every path through
    // 0x575b35 already knows stay == 0 and threads past this test (the image
    // keeps it out of line at 0x5767ed).
    if (stay) goto L_577071;
    if (Vehs[veh_id].order == ORDER_MOVE_TO && Vehs[veh_id].move_to_ai_type == 'd') goto L_577071;
    if (plan == 6) {
        if (VehPrototypes[Vehs[veh_id].proto_id].unk_1 == 21) {
            // 0x5766ad: meet one of our colony pods out at sea ('C').
            best_score = 9999;
            for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
                if (Vehs[other_id].faction_id != faction_id) continue;
                if (VehPrototypes[Vehs[other_id].proto_id].plan != 7) continue;
                tx = Vehs[other_id].x;
                ty = Vehs[other_id].y;
                if ((map_loc(tx, ty)->climate & 0xE0) >= 0x60) continue;
                if (base_who(tx, ty) >= 0) continue;
                dist = (abs(y - ty) + x_dist(x, tx)) >> 1;
                if (dist) {
                    if (stack_check(other_id, 2, 6, -1, -1)) continue;
                    // BUG(original): the divisor is not guarded against zero.
                    dist /= stack_check(other_id, 1, -1, -1, -1);
                }
                if (dist < best_score) {
                    best_score = dist;
                    go_to(veh_id, 'C', tx, ty);
                }
            }
            if (Vehs[veh_id].order == ORDER_MOVE_TO) {
                if (dist == 0) goto L_578b34;
                goto L_577071;
            }
        } else if (Vehs[veh_id].order == ORDER_MOVE_TO && Vehs[veh_id].move_to_ai_type != '6') {
            goto L_576a95;
        }
        if (Players[faction_id].rule_flags & RFLAG_AQUATIC) {
            // 0x576865: hostile coastal base worth raiding ('6').
            best_score = 0;
            best_id = -1;
            for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                if (Bases[base_id].faction_id_current == faction_id) continue;
                if (!(PlayersData[faction_id].diplo_treaties[Bases[base_id].faction_id_current]
                      & DTREATY_VENDETTA)) continue;
                // BUG(original): the base_on_sea arm tests home_base, not base_id.
                if (!home_dist ? !port_to_port(home_base, base_id)
                               : !base_on_sea(home_base, region)) continue;
                score = Bases[base_id].population_size * Bases[base_id].population_size * 32
                    / (((abs(y - Bases[base_id].y) + x_dist(x, Bases[base_id].x)) >> 1) + 1);
                for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
                    if (other_id == veh_id) continue;
                    if (Vehs[other_id].faction_id != faction_id) continue;
                    if (VehPrototypes[Vehs[other_id].proto_id].plan != 6) continue;
                    if (((abs(Vehs[other_id].y - Bases[base_id].y)
                          + x_dist(Vehs[other_id].x, Bases[base_id].x)) & ~1) <= 4
                        || (Vehs[other_id].move_to_ai_type == '6'
                            && Vehs[other_id].waypoint_x[0] == Bases[base_id].x
                            && Vehs[other_id].waypoint_y[0] == Bases[base_id].y)) {
                        score /= 2;
                    }
                }
                if (score > best_score) {
                    best_score = score;
                    best_id = base_id;
                    go_to(veh_id, '6', Bases[base_id].x, Bases[base_id].y);
                }
            }
            if (best_id >= 0
                && ((abs(y - Bases[best_id].y) + x_dist(x, Bases[best_id].x)) & ~1) <= 2) {
                Vehs[veh_id].order = ORDER_NONE;
                goto L_577071;
            }
        }
L_576a95:
        if (Vehs[veh_id].order != ORDER_MOVE_TO
            && !((GameDataStateStorage.fields_.turn_current_num_ + veh_id) & 3)) {
            // 0x576ab9: open sea between a contested pair of land regions ('D').
            best_score = -9999;
            for (ty = 0; ty < MapLatitudeBounds; ty++) {
                for (tx = ty & 1; tx < MapLongitudeBounds; tx += 2) {
                    if ((map_loc(tx, ty)->climate & 0xE0) >= 0x60) continue;
                    if (!home_dist ? !base_on_sea(home_base, map_loc(tx, ty)->region)
                                   : map_loc(tx, ty)->region != region) continue;
                    if (((abs(y - ty) + x_dist(x, tx)) & ~1) < 16) continue;
                    for (other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; other_id++) {
                        if (Vehs[other_id].faction_id == faction_id && other_id != veh_id
                            && Vehs[other_id].waypoint_x[0] == tx
                            && Vehs[other_id].waypoint_y[0] == ty) {
                            goto L_576d87;
                        }
                    }
                    score = 0;
                    for (k = 9; k < 81; k++) {
                        int ax = xrange(tx + RadiusOffsetX[k]);
                        int ay = ty + RadiusOffsetY[k];
                        if (!on_map(ax, ay)) continue;
                        if ((map_loc(ax, ay)->climate & 0xE0) < 0x60) continue;
                        int bx = xrange(tx - RadiusOffsetX[k]);
                        int by = ty - RadiusOffsetY[k];
                        if (!on_map(bx, by)) continue;
                        if ((map_loc(bx, by)->climate & 0xE0) < 0x60) continue;
                        int ar = map_loc(ax, ay)->region;
                        int br = map_loc(bx, by)->region;
                        if (PlayersData[faction_id].region_total_bases[ar] <= 1) continue;
                        if (PlayersData[faction_id].region_total_bases[br] > 1) continue;
                        for (owner = 1; owner < 8; owner++) {
                            if (owner != faction_id
                                && (PlayersData[faction_id].diplo_treaties[owner] & 0xC10)
                                && PlayersData[owner].region_total_bases[ar] <= 1
                                && PlayersData[owner].region_total_bases[br] > 1) {
                                score++;
                            }
                        }
                    }
                    if (score) {
                        // BUG(original): the divisor is not guarded against zero.
                        int value = (score << 8) / vector_dist(x_dist(x, tx), abs(y - ty));
                        if (value > best_score) {
                            best_score = value;
                            go_to(veh_id, 'D', tx, ty);
                        }
                    }
L_576d87:;
                }
            }
        }
    }
    // 0x576daf: explore unknown sea ('X').
    if (Vehs[veh_id].order == ORDER_MOVE_TO
        && (!(PlayersData[faction_id].flags & PFLAG_STRAT_SEARCH_OBJECTIVES)
            || stack_check(veh_id, 1, -1, -1, -1) != 1)) goto L_577071;
    if ((GameDataStateStorage.fields_.turn_current_num_ + veh_id) & 3) goto L_577071;
    best_score = -9999;
    for (ty = 0; ty < MapLatitudeBounds; ty++) {
        for (tx = ty & 1; tx < MapLongitudeBounds; tx += 2) {
            if (!is_known(tx, ty, faction_id)) continue;
            if ((map_loc(tx, ty)->climate & 0xE0) >= 0x60) continue;
            if (!home_dist ? !base_on_sea(home_base, map_loc(tx, ty)->region)
                           : map_loc(tx, ty)->region != region) continue;
            score = 0;
            if (PlayersData[faction_id].flags & PFLAG_STRAT_SEARCH_OBJECTIVES) {
                if (!(map_loc(tx, ty)->bit & 0x100000)) {
                    score = 1;
                    if ((map_loc(tx, ty)->bit & 0x20)
                        && (map_loc(tx, ty)->climate & 0xE0) >= 0x40) {
                        score = 2;
                    }
                    if (map_loc(tx, ty)->bit & 0x2000) score++;
                }
            }
            for (int d = 0; d < 8; d++) {
                int ax = xrange(tx + VehMoveDeltaX[d]);
                int ay = ty + VehMoveDeltaY[d];
                if (!on_map(ax, ay)) continue;
                if (PlayersData[faction_id].flags & PFLAG_STRAT_SEARCH_OBJECTIVES) {
                    if ((map_loc(ax, ay)->bit & 0x100000) && is_known(ax, ay, faction_id)) continue;
                } else {
                    if (is_known(ax, ay, faction_id)) continue;
                }
                score += 2;
                if (ay >= 2 && ay < MapLatitudeBounds - 3) score += 4;
            }
            if (!score) continue;
            {
                int range_dist;
                if (home_base >= 0) {
                    range_dist = vector_dist(x_dist(Bases[home_base].x, tx),
                                             abs(Bases[home_base].y - ty));
                } else {
                    range_dist = vector_dist(x_dist(x, tx), abs(y - ty));
                }
                if (!range_dist) continue;
                int value = (score << 8) / range_dist;
                if (value > best_score) {
                    best_score = value;
                    go_to(veh_id, 'X', tx, ty);
                }
            }
        }
    }
L_577071:
    if (no_moves) goto L_5791ff;
L_57707f:
    if (triad) goto L_57717f;
L_57708a:
    // Unbased ocean region: flag an adjacent sea tile as a goal.
    if (plan != 8
        && !PlayersData[faction_id].region_total_bases[region]
        && !(PlayersData[faction_id].unk_77[region] & 6)
        && PlayersData[faction_id].region_base_plan[region] > 2
        && ((y > 0 && y < MapLatitudeBounds - 1) || plan == 12)) {
        for (int d = 0; d < 8; d++) {
            tx = xrange(x + VehMoveDeltaX[d]);
            ty = y + VehMoveDeltaY[d];
            if (on_map(tx, ty) && (map_loc(tx, ty)->climate & 0xE0) < 0x60) {
                // Laid out after the loop at 0x577161.
                add_goal(faction_id, 7, 1, tx, ty, -1);
                coast_goal = 1;
                break;
            }
        }
    }
L_57717f:
    if (Vehs[veh_id].order == ORDER_MOVE_TO
        && (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating
            || (Vehs[veh_id].moves_expended
                && speed(veh_id, false) > Rules.move_rate_roads
                && !plan && !triad))
        && !(Vehs[veh_id].state & VSTATE_UNK_8)
        && ((abs(y - Vehs[veh_id].waypoint_y[0]) + x_dist(x, Vehs[veh_id].waypoint_x[0])) & ~1) > 2) {
        best_dir = Paths.move(veh_id, faction_id);
        if (best_dir >= 0) {
            tx = xrange(x + VehMoveDeltaX[best_dir]);
            ty = y + VehMoveDeltaY[best_dir];
            owner = zoc_sea(tx, ty, faction_id);
            if (owner == 1
                || (owner > 1
                    && (PlayersData[faction_id].diplo_treaties[owner] & DTREATY_VENDETTA)
                    && (plan != 11
                        || veh_moves(veh_id) - hex_cost(proto_id, faction_id, x, y, tx, ty, 0) < 1))) {
                if (!zoc_owner
                    || (plan != 11
                        && !Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating)) {
                    Vehs[veh_id].order = ORDER_NONE;
                }
                if (!zoc_owner) goto L_578b34;
            }
        } else if (zoc_owner == 1) {
            Vehs[veh_id].order = ORDER_NONE;
            Vehs[veh_id].state &= 0xFCFFBDFF;
        }
    }
    if (Vehs[veh_id].order == ORDER_MOVE_TO && !near_hostile && !stay) {
        if (!zoc_veh(x, y, faction_id)) goto L_578f46;
        if (((abs(y - Vehs[veh_id].waypoint_y[0]) + x_dist(x, Vehs[veh_id].waypoint_x[0])) & ~1) == 2) {
            owner = anything_at(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
            if (owner >= 0 && owner != faction_id
                && !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)
                && Vehs[veh_id].move_to_ai_type != 'p') {
                Vehs[veh_id].order = ORDER_NONE;
            }
        }
        if (Vehs[veh_id].order == ORDER_MOVE_TO) keep_wp = 1;
    }
    // 0x577443: tactical scan of the nine tiles around the unit.
    best_score = -9999;
    alt = 0;
    if (zoc_sea(x, y, faction_id) || keep_wp || assault || retreat) {
        assault = 0;
    } else {
        assault = 1;
        if (plan == 12) assault = 0;
    }
    k = rnd(9, "ENEMYBIAS");
    hint = 0;
    do {   // 0x5774ac
    i = (hint + k) % 9;
    tx = xrange(x + RadiusOffsetX[i]);
    ty = y + RadiusOffsetY[i];
    if (!on_map(tx, ty)) goto L_578ae1;
    if (!i && !zoc_owner && plan != 2) goto L_578ae1;
    near_hostile = (map_loc(tx, ty)->climate & 0xE0) < 0x60;
    other_id = stack_fix(veh_at(tx, ty));
    base_id = base_at(tx, ty);
    faction_bit = map_loc(tx, ty)->region;
    if (triad == 1 && !near_hostile) {
        // `owner` here is whatever 0x57728c/0x5773f9 or the previous
        // iteration last stored.
        if (other_id < 0 || owner == faction_id || !at_sea) goto L_578ae1;
    } else if (!at_sea && !near_hostile && zoc_moved && !triad && other_id < 0
               && !(VehPrototypes[Vehs[veh_id].proto_id].ability_flags & ABL_CLOAKED)
               && VehPrototypes[Vehs[veh_id].proto_id].plan != 11
               && zoc_move(tx, ty, faction_id)) {
        goto L_578ae1;
    }
    if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating
        && (other_id < 0 || !i)
        && base_id < 0
        && !(Vehs[veh_id].state & VSTATE_UNK_8)) {
        owner = zoc_sea(tx, ty, faction_id);
        if (owner > 0) {
            if (owner == 1) goto L_578ae1;
            if ((PlayersData[faction_id].diplo_treaties[owner - 1] & DTREATY_PACT)
                && (PlayersData[faction_id].diplo_treaties[owner - 1] & 0x2000000)) goto L_578ae1;
            if (plan != 11
                || (Vehs[veh_id].moves_expended
                    && veh_moves(veh_id) - hex_cost(proto_id, faction_id, x, y, tx, ty, 0) <= 0)) {
                if (PlayersData[faction_id].diplo_treaties[owner - 1] & DTREATY_VENDETTA) goto L_578ae1;
            }
        }
    }
    // falls through to 0x577720 (chunk 7)
    // ---- chunk 7: 0x577720-0x579208 ----
    // Tail of the nine-tile tactical scan (loop counter hint, tile index i,
    // candidate tile tx/ty), its post-scan, the shared order/waypoint tails and
    // the final path-safety check.
L_577720:
L_577723:
    score = rnd(6, const_cast<char *>("ENEMY3"));
    if (near_hostile && !triad) {
        if (!(base_id >= 0 && has_abil(Vehs[veh_id].proto_id, 8))) {
            if (other_id < 0 || Vehs[other_id].faction_id != faction_id
                || stack_check(other_id, 8, -1, -1, -1) < 1
                || Vehs[other_id].dmg_incurred
                       > VehPrototypes[Vehs[other_id].proto_id].reactor_id * 2)
                goto L_578ae1;
            if (plan == 12) {
                score += 99;
            } else {
                if (plan == 8) {
                    if (PlayersData[faction_id].region_base_plan[region] != 7)
                        goto L_578ae1;
                } else {
                    if (plan == 9)
                        goto L_578ae1;
                    if (!(PlayersData[faction_id].region_base_plan[region] == 7 && plan < 2)) {
                        if (PlayersData[faction_id].region_total_bases[region]
                            || (PlayersData[faction_id].unk_77[region] & 6)
                            || PlayersData[faction_id].region_base_plan[region] <= 2)
                            goto L_578ae1;
                    }
                }
                score += 16;
            }
        }
    }
    if (at_sea && !near_hostile && !triad && !home_dist
        && !has_abil(Vehs[veh_id].proto_id, 8)
        && !stack_check(veh_id, 2, 7, -1, -1))
        goto L_578ae1;
    if (at_sea == near_hostile && (Vehs[veh_id].state & 0x10000)
        && (!zoc_owner
            || (zoc_owner > 1
                && (PlayersData[faction_id].diplo_treaties[zoc_owner - 1] & 7)))) {
        if (!i) {
            score -= 12;
        } else {
            int turn = Vehs[veh_id].facing - i + 1;
            if (turn < 0)
                turn = i - Vehs[veh_id].facing - 1;
            if (turn > 4)
                turn = 8 - turn;
            score -= turn * 4;
            if (turn == 4)
                score -= 256;
        }
    }
    if (plan == 12) {
        int coast = 0;
        if (zoc_veh(tx, ty, faction_id))
            coast = -32;
        if (!is_coast(tx, ty, 0))
            coast -= 32;
        if (!i && !coast) {
            score += (coast_goal ? 24 : 0) + 8;
            base_find(tx, ty, faction_id);
            if (BaseFindDist <= home_dist)
                score += 8;
        }
        score += coast;
    } else if (coast_goal && !i) {
        score += 8;
    }
    if (PlayersData[faction_id].flags & 0x20000000) {
        Map *sq = map_loc(tx, ty);
        if (!(sq->bit & 0x100000)) {
            score += 8;
            if ((sq->bit & 0x20) && (sq->climate & 0xE0) >= 0x40)
                score += 8;
            if (sq->bit & 0x2000)
                score += 8;
        }
    }
    owner = whose_territory(faction_id, tx, ty, 0, 0);
    if (owner >= 0 && owner != faction_id
        && VehPrototypes[Vehs[veh_id].proto_id].plan != 11
        && !(PlayersData[faction_id].diplo_treaties[owner] & 0xC10)) {
        if ((PlayersData[faction_id].diplo_agenda[owner] & 2)
            || (PlayersData[faction_id].diplo_treaties[owner] & 0x100000))
            goto L_578ae1;
        if (PlayersData[faction_id].diplo_treaties[owner] & 4)
            score -= 6;
        if (PlayersData[faction_id].diplo_treaties[owner] & 2) {
            score -= 12;
            if (!(PlayersData[faction_id].diplo_treaties[owner] & 1)
                && (at_sea || !(PlayersData[faction_id].diplo_treaties[owner] & 0x2020)
                    || GameDataStateStorage.fields_.sunspot_duration_ > 0))
                goto L_578ae1;
        }
    }
    if (retreat && plan == 2 && other_id >= 0 && owner == faction_id
        && zoc_veh(tx, ty, faction_id))
        score -= stack_check(veh_id, 1, -1, -1, -1) * 2;
    if (at_sea && !triad && home_dist) {
        // faction_bit holds the candidate tile's region here (set in chunk 6).
        if (PlayersData[faction_id].region_base_plan[faction_bit] == plan)
            score += 32;
        if (PlayersData[faction_id].region_base_plan[faction_bit] != 7)
            score += 32;
        base_find(tx, ty);
        score += 32 - BaseFindDist - stack_check(other_id, 1, -1, -1, -1);
    }
    if (goody_at(tx, ty)) {
        if (plan == 7 && stack_check(veh_id, 1, -1, -1, -1) > 1)
            goto L_578ae1;
        score += 99;
    }
    if (!at_sea && !near_hostile && region != faction_bit
        && PlayersData[faction_id].region_total_bases[region]) {
        if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating) {
            if (plan != 8 || PlayersData[faction_id].unk_82[region])
                goto L_578ae1;
        } else if (plan != 3) {
            if (region_plan <= 2 || (PlayersData[faction_id].unk_77[region] & 0x20))
                goto L_578ae1;
        }
    }
    if (!assault || !state_4000) {
        no_moves = 2;   // the scoring policy: 2 when retreating, else the plan
        if (!retreat)
            no_moves = plan;
        int friendly = other_id >= 0
            && (Vehs[other_id].faction_id == faction_id
                || (PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id] & 1));
        if (!i && stack_check(other_id, 1, other_id, -1, -1) == 1)
            friendly = 0;
        // BUG(original): Vehs[other_id] is read even when other_id < 0.
        if (at_sea && !triad && no_moves != 8
            && Vehs[other_id].faction_id == faction_id) {
            if (stack_check(other_id, 2, 8, -1, -1))
                score += 8;
            if (no_moves <= 1 || no_moves == 11) {
                if (PlayersData[faction_id].unk_77[faction_bit] & 0x20)
                    score += 16;
                if (PlayersData[faction_id].unk_77[faction_bit] & 0x40)
                    score += 16;
            }
        }
        switch (no_moves) {
        case 0:
            if (friendly && !assault)
                score += stack_check(other_id, 2, 2, -1, -1) * 4;
            score -= stack_check(other_id, 2, 0, -1, -1) * 2;
            score -= hex_cost(proto_id, faction_id, x, y, tx, ty, 0) / Rules.move_rate_roads;
            break;
        case 1:
            if (zoc_veh(tx, ty, faction_id))
                score += defense_value(faction_id, tx, ty, veh_id, -1) * 2;
            break;
        case 2:
            if (zoc_veh(tx, ty, faction_id))
                score += defense_value(faction_id, tx, ty, veh_id, -1) * 2;
            if (friendly && !assault) {
                score += stack_check(other_id, 2, 0, -1, -1) * 4;
                score -= stack_check(other_id, 2, 2, -1, -1) * 2;
            }
            break;
        case 3:
            score -= hex_cost(proto_id, faction_id, x, y, tx, ty, 0) / Rules.move_rate_roads;
            break;
        case 6:
            score += stack_check(other_id, 2, 7, -1, -1) * 4;
            score -= stack_check(other_id, 2, 6, -1, -1) * 2;
            break;
        case 7:
            if (!assault)
                score += stack_check(other_id, 2, 6, -1, -1) * 4;
            score -= stack_check(other_id, 2, 7, -1, -1) * 8;
            break;
        case 8:
            if (at_sea && zoc_sea(tx, ty, faction_id))
                goto L_578ae1;
            if (owner >= 0 && owner != faction_id
                && (PlayersData[faction_id].diplo_treaties[owner] & 3)
                && Vehs[veh_id].order != 0x18)
                score -= 16;
            if (at_sea && PlayersData[faction_id].region_base_plan[faction_bit] == 8) {
                score += Continents[faction_bit].open_terrain + 16;
                if (!(PlayersData[faction_id].unk_77[faction_bit] & 0x20))
                    score += 8;
                if (!(PlayersData[faction_id].unk_77[faction_bit] & 0x10))
                    score += 16;
            }
            break;
        case 11:
            if (friendly)
                score += stack_check(other_id, 14, -1, -1, -1) * 2;
            break;
        }
        if ((map_loc(tx, ty)->climate & 0xE0) > (map_loc(x, y)->climate & 0xE0)
            && (Rules.combat_pct_atk_higher_elev || Rules.combat_pen_pct_atk_lwr_elev))
            score += 2;
    }
    keep_count = 0;   // combat flag for this candidate
    if (other_id >= 0) {
        owner = Vehs[other_id].faction_id;
        if (triad == 2
            && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range
            && Vehs[veh_id].terraforming_turns && Vehs[veh_id].order == 0x18
            && (Vehs[veh_id].waypoint_x[0] != tx || Vehs[veh_id].waypoint_y[0] != ty))
            goto L_578ae1;
        if (stack_check(other_id, 1, -1, -1, -1) == 1
            && !(PlayersData[faction_id].diplo_treaties[owner] & 0x11)
            && VehPrototypes[Vehs[other_id].proto_id].plan == 12
            && Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating
            && base_id < 0) {
            if (!(PlayersData[faction_id].diplo_treaties[owner] & 0x20)
                && whose_territory(faction_id, tx, ty, 0, 0) != faction_id
                && !rnd(2, const_cast<char *>("ENEMYMISC")))
                goto L_578ae1;
            keep_count = 1;
            score += 8;
        } else if (plan == 11 && owner == faction_id) {
            // own probe team target: no combat score
        } else if (plan == 11 && stack_check(other_id, 2, 11, -1, -1)
                   && (!(PlayersData[faction_id].diplo_treaties[owner] & 1)
                       || !(PlayersData[faction_id].diplo_treaties[owner] & 0x2000000))) {
            keep_count = 1;
            int odds = battle_fight(veh_id, i - 1, 0, 0, &other_id);
            if (whose_territory(faction_id, tx, ty, 0, 0) == faction_id)
                odds *= 2;
            else if (base_id < 0 && (PlayersData[faction_id].diplo_treaties[owner] & 7))
                odds /= 2;
            if (odds >= 8)
                score += odds;
            else if (base_id < 0)
                goto L_578ae1;
        } else if (owner != faction_id
                   && !(PlayersData[faction_id].diplo_treaties[owner] & 1)) {
            if (at_sea && !near_hostile && triad == 1 && !can_arty(proto_id, 1))
                goto L_578ae1;
            if (stack_check(other_id, 1, -1, -1, -1) == 1
                && VehPrototypes[Vehs[other_id].proto_id].plan == 11
                && (PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id] & 6)
                && whose_territory(faction_id, Vehs[other_id].x, Vehs[other_id].y, 0, 0)
                       == faction_id) {
                keep_count = 1;
                score += 99;
            } else {
                if (plan == 7) {
                    if (stack_check(veh_id, 1, -1, -1, -1) > 1)
                        goto L_578ae1;
                } else if (plan >= 8 && plan != 11) {
                    goto L_578ae1;
                }
                if (at_sea && !triad && !has_abil(Vehs[veh_id].proto_id, 8))
                    goto L_578ae1;
                if (stack_check(other_id, 3, 2, -1, -1)
                    && !has_abil(Vehs[veh_id].proto_id, 0x20) && base_id < 0
                    && !(map_loc(tx, ty)->bit & 0x40000)
                    && !stack_check(other_id, 6, 0x80, -1, -1))
                    goto L_578ae1;
                if (plan == 11) {
                    if (stack_check(other_id, 1, -1, -1, -1) > 1)
                        goto L_578ae1;
                    if (base_who(Vehs[other_id].x, Vehs[other_id].y) >= 0)
                        goto L_578ae1;
                    int vehicles = GameDataStateStorage.fields_.vehicle_current_count_;
                    if (!probe(veh_id, -1, other_id, 1))
                        goto L_578ae1;
                    if (vehicles != GameDataStateStorage.fields_.vehicle_current_count_)
                        return 1;
                    best_score = 999;
                    alt = 1;
                    best_dir = i - 1;
                    goto L_578ae1;
                }
                if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating)
                    goto L_578ae1;
                if (!(PlayersData[faction_id].diplo_treaties[owner] & 0x810))
                    goto L_578ae1;
                keep_count = 1;
                int odds = battle_fight(veh_id, i - 1, 0, 0, &other_id);
                if (defend
                    || (veh_moves(veh_id) < Rules.move_rate_roads && proto_id != 8)
                    || (!home_dist && plan >= 1)) {
                    int reverse = battle_fight(other_id, (i + 3) & 7, 0, 0, 0);
                    if (reverse) {
                        if ((4 - (defend ? defend + 1 : 0)) * reverse < odds * 4
                            || reverse < 8 || odds < defend + 6)
                            goto L_578ae1;
                    } else if (stack_check(veh_id, 1, -1, -1, -1) == 1
                               && VehPrototypes[Vehs[other_id].proto_id].plan != 11) {
                        goto L_578ae1;
                    }
                }
                odds = compute_odds(odds, faction_id, veh_id, other_id, base_id);
                if (!owner && (home_base < 0 || !Bases[home_base].eco_damage)
                    && Vehs[other_id].order_auto_type != faction_id) {
                    if (PlayersData[faction_id].soc_effect_active.values_[SocialEffect::PLANET] > 2)
                        odds *= 8;
                    else
                        odds *= 2;
                }
                if (odds < 8)
                    goto L_578ae1;
                score += odds;
            }
        }
    } else {
        if (base_id >= 0 && Bases[base_id].faction_id_current != faction_id
            && !(PlayersData[faction_id].diplo_treaties[owner] & 1)) {
            if (plan == 11
                && !(VehPrototypes[Vehs[veh_id].proto_id].ability_flags & 0x10000000)
                && has_project(15, Bases[base_id].faction_id_current))
                goto L_578ae1;
            if ((PlayersData[faction_id].diplo_treaties[owner] & 7)
                && !(PlayersData[faction_id].diplo_treaties[owner] & 0x800)) {
                if (!(PlayersData[faction_id].diplo_treaties[owner] & 0x20))
                    goto L_578ae1;
                if ((PlayersData[faction_id].diplo_treaties[owner] & 2)
                    && static_cast<int>(rnd(6, const_cast<char *>("ENEMY4")))
                           - PlayersData[faction_id].ai_fight
                       > reputation(owner, faction_id))
                    goto L_578ae1;
            }
            if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating)
                goto L_578ae1;
            keep_count = 1;
            score += 9999;
        }
        if ((map_loc(tx, ty)->bit & 0x2000) && want_monolith(veh_id))
            score += 99;
    }
    // Exploration bonus along the ray through the candidate tile.
    if ((assault || state_4000) && i
        && (triad == 2 || near_hostile == (triad == 1))) {
        if (state_4000) {
            base_find(tx, ty, faction_id);
            score += BaseFindDist * 2;
        }
        for (j = 0; j < 8; j++) {
            // own_ground/tile_owner hold the ray tile x/y here.
            own_ground = xrange(tx + RadiusOffsetX[i] * j);
            tile_owner = ty + RadiusOffsetY[i] * j;
            if (!on_map(own_ground, tile_owner))
                continue;
            if (triad != 2
                && ((map_loc(own_ground, tile_owner)->climate & 0xE0) < 0x60) != (triad == 1))
                continue;
            for (n = 0; n < 8; n++) {
                int nx = xrange(own_ground + VehMoveDeltaX[n]);
                int ny = tile_owner + VehMoveDeltaY[n];
                if (!on_map(nx, ny))
                    continue;
                if (PlayersData[faction_id].flags & 0x20000000) {
                    Map *sq = map_loc(nx, ny);
                    if (!(sq->bit & 0x100000)) {
                        score += 8;
                        if ((sq->bit & 0x20) && (sq->climate & 0xE0) >= 0x40)
                            score += 8;
                        if (sq->bit & 0x2000)
                            score += 8;
                    }
                }
                if (!is_known(nx, ny, faction_id)) {
                    score += state_4000 * 2 + 2;
                    if (triad == 1 && (map_loc(nx, ny)->climate & 0xE0) >= 0x60
                        && !PlayersData[faction_id].region_total_bases[map_loc(nx, ny)->region])
                        score += 8;
                }
                if (goody_at(nx, ny))
                    score += state_4000 * 2 + 2;
                if (plan == 8) {
                    owner = whose_territory(faction_id, nx, ny, 0, 0);
                    if (owner >= 0 && owner != faction_id
                        && (PlayersData[faction_id].diplo_treaties[owner] & 3))
                        score -= 6;
                }
            }
        }
    }
    if (score > best_score) {
        best_score = score;
        best_dir = i - 1;
        alt = keep_count;   // combat flag of the best candidate
    }
L_578ae1:
    ;
    } while (++hint < 9);   // nine-tile scan, opened in chunk 6 at 0x5774ac
    // Post-scan.
    if (!alt)
        goto L_578b55;
    if (planet_buster(veh_id)) {
        best_dir = -1;
        Vehs[veh_id].order = 0;
    }
L_578b1c:
    Vehs[veh_id].move_to_ai_type = 'x';
L_578b26:
L_578b29:
    if (best_dir >= 0)
        goto L_578da8;
L_578b34:
    Vehs[veh_id].state &= ~0x10000;
    Vehs[veh_id].order = 0;
    goto L_578f46;
L_578b55:
    if (stay) {
        best_dir = -1;
        Vehs[veh_id].order = 0;
    }
    if (keep_wp && Vehs[veh_id].order == 0x18)
        goto L_578f46;
    if (can_arty(proto_id, 1) && !Vehs[veh_id].order
        && (triad == 1 || (map_loc(x, y)->climate & 0xE0) >= 0x60 || !near_dist)) {
        // own_ground is the radius index of this artillery search.
        for (own_ground = 0; own_ground < 25; own_ground++) {
            tx = xrange(x + RadiusOffsetX[own_ground]);
            ty = y + RadiusOffsetY[own_ground];
            if (!on_map(tx, ty))
                continue;
            if ((map_loc(tx, ty)->climate & 0xE0) < 0x60)
                continue;
            if (anything_at(tx, ty) >= 0)
                continue;
            {
                int terr = whose_territory(faction_id, tx, ty, 0, 0);
                if (terr < 0 || terr == faction_id
                    || !(PlayersData[faction_id].diplo_treaties[terr] & 0x10))
                    continue;
            }
            if (PlayersData[faction_id].region_total_bases[map_loc(tx, ty)->region]
                && PlayersData[faction_id].region_base_plan[map_loc(tx, ty)->region] != 2
                && !triad)
                continue;
            if ((map_loc(x, y)->bit & 0x1000) && (map_loc(x, y)->bit & 0x81208854))
                goto L_578d33;
        }
    }
    goto L_578b1c;
L_578d33:
    if (remote_own) {
        log_say("Auto sending DESTROY2 message", veh_id, tx, ty);
L_578d5f:   // cross-jump entry (mid-call in the image); see notes
        message_data(0x2413, veh_id, tx, ty, 0, 0);
L_578d67:
        NetDaemonNet.await_exec(1);
L_578d73:
        return 1;
    }
    log_say("Enemy doing ARTY DESTROY 2", veh_id, tx, ty);
L_578d91:   // cross-jump entry (mid-call in the image); see notes
    action_destroy(veh_id, 0, tx, ty);
L_578d9c:
    return 1;
L_578da8:
    tx = xrange(x + VehMoveDeltaX[best_dir]);
    ty = y + VehMoveDeltaY[best_dir];
    Vehs[veh_id].order = 0x58;
    Vehs[veh_id].waypoint_x[0] = static_cast<int16_t>(tx);
    Vehs[veh_id].waypoint_y[0] = static_cast<int16_t>(ty);
    goto L_578f46;
L_578de9:
    // Entered with (state & 0x1000200) == 0x1000200, or a patrol route
    // ((state & 0x2000200) == 0x2000200 and current point < count).
    // The image keeps the state word read at the jump site in a register.
    if (Vehs[veh_id].order != 0x18) {
        if ((Vehs[veh_id].state & 0x2000200) == 0x2000200
            && Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count
            && (Vehs[veh_id].state & 0x1000200) != 0x1000200) {
            Vehs[veh_id].order = 0x18;
            // BUG(original): index w + 1 reaches waypoint_x[4] (== waypoint_y[0])
            // and past waypoint_y[3].
            for (int w = 0; w < Vehs[veh_id].waypoint_count; w++) {
                Vehs[veh_id].waypoint_x[w] = Vehs[veh_id].waypoint_x[w + 1];
                Vehs[veh_id].waypoint_y[w] = Vehs[veh_id].waypoint_y[w + 1];
            }
            if (!--Vehs[veh_id].waypoint_count)
                Vehs[veh_id].state &= 0xFDFEFDFF;
        } else {
            tx = Vehs[veh_id].waypoint_x[0];
            ty = Vehs[veh_id].waypoint_y[0];
            for (int w = 0; w < Vehs[veh_id].waypoint_count; w++) {
                Vehs[veh_id].waypoint_x[w] = Vehs[veh_id].waypoint_x[w + 1];
                Vehs[veh_id].waypoint_y[w] = Vehs[veh_id].waypoint_y[w + 1];
            }
            // BUG(original): stores at slot waypoint_count, not count - 1.
            Vehs[veh_id].waypoint_x[Vehs[veh_id].waypoint_count] = static_cast<int16_t>(tx);
            Vehs[veh_id].waypoint_y[Vehs[veh_id].waypoint_count] = static_cast<int16_t>(ty);
            Vehs[veh_id].state &= ~0x10000;
            if (Vehs[veh_id].waypoint_x[0] == tx && Vehs[veh_id].waypoint_y[0] == ty
                && Vehs[veh_id].waypoint_count == 1) {
                Vehs[veh_id].order = 0;
                Vehs[veh_id].state &= 0xFEFFFDFF;
            } else {
                Vehs[veh_id].order = 0x18;
            }
        }
    }
L_578f46:
    if (Vehs[veh_id].order < 0x18)
        Vehs[veh_id].state &= ~0x10000;
    if (!Vehs[veh_id].order || Vehs[veh_id].order == 2) {
        if (triad == 2 && home_dist && !(map_loc(x, y)->bit & 0x40000)) {
            action_home(veh_id, 0);
        } else {
            Vehs[veh_id].order =
                (((1 << faction_id) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                 && !(Vehs[veh_id].state & 0x200)) ? 0 : 2;
            veh_skip(veh_id);
        }
    }
    if (Vehs[veh_id].order == 0x18 && Vehs[veh_id].waypoint_x[0] == x
        && Vehs[veh_id].waypoint_y[0] == y) {
        Vehs[veh_id].order = 0;
        veh_skip(veh_id);
    }
    if ((Vehs[veh_id].order == 0x18 || Vehs[veh_id].order == 0x58)
        && (plan == 9 || plan == 8)) {
        int dir = Paths.move(veh_id, (Vehs[veh_id].order & 0x40) ? -1 : Vehs[veh_id].faction_id);
        if (dir < 0)
            goto L_5791f5;
        tx = xrange(x + VehMoveDeltaX[dir]);
        ty = y + VehMoveDeltaY[dir];
        if (base_who(tx, ty) >= 0)
            return 0;
        // The image keeps this counter in the dead veh_id parameter slot.
        for (int d = 0; d < 8; d++) {
            int nx = xrange(tx + VehMoveDeltaX[d]);
            int ny = ty + VehMoveDeltaY[d];
            if (!on_map(nx, ny))
                continue;
            if (((map_loc(nx, ny)->climate & 0xE0) < 0x60)
                != ((map_loc(tx, ty)->climate & 0xE0) < 0x60))
                continue;
            if (((abs(ny - y) + x_dist(nx, x)) & ~1) <= 2)
                continue;
            other_id = stack_fix(veh_at(nx, ny));
            if (other_id < 0 || Vehs[other_id].faction_id == faction_id
                || !(PlayersData[faction_id].diplo_treaties[Vehs[other_id].faction_id] & 0x10))
                continue;
            if (Weapon[VehPrototypes[Vehs[other_id].proto_id].weapon_id].offense_rating
                || stack_check(other_id, 14, -1, -1, -1))
                goto L_5791f5;
        }
        return 0;
    }
    goto L_5791ff;
L_5791f5:
    Vehs[veh_id].order = 0;
L_5791ff:
L_579201:
    return 0;
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
// enemy_move, its only caller, calls it out of line in the image.
#pragma auto_inline(off)
uint32_t __cdecl rnd(int bounds, LPSTR UNUSED(input)) {
    return bounded_crt_random(bounds);
}
#pragma auto_inline(on)

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
