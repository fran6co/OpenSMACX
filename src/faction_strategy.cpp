#include "stdafx.h"
#include "base.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "general.h"
#include "lock.h"
#include "map.h"
#include "terraforming.h"
#include "veh.h"

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
                || (region_plan == 8 && Continents[region].tile_count < 0x96u)))) {
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
                            if (Continents[region].tile_count >= 0x28u) {
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
