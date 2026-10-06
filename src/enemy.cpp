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
#include "temp.h"
#include "terraforming.h"
#include "veh.h"
#include "xpops.h"
#include <stddef.h>
#include <string.h>
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
    union { int local_44; const short *base_cursor_44; };
    int local_40, local_3c, local_38, local_34;
    union { int local_30; const short *base_cursor_30; };
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
        has_tech(Terraforming[TERRA_MAGTUBE].preq_tech, faction_id)) {
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
        (LockEnableMask & (1u << faction_id)) != 0;
    bool jumped_to_common_order_dispatch = false;
    if (!faction_locked && (vehicle.flags & 0x0800u) != 0) {
        if (has_tech(Terraforming[TERRA_MAGTUBE].preq_tech, faction_id))
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
            (GameMorePreferences & 8u) != 0) {
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
            const int tech = low ? remove_fungus.preq_tech_sea
                                 : remove_fungus.preq_tech;
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
                if (has_tech(Terraforming[TERRA_FARM].preq_tech,
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
            const int solar_tech = low ? solar.preq_tech_sea : solar.preq_tech;
            if (has_tech(solar_tech, faction_id)) {
                vehicle.order = 0x07;
                return ENEMY_MOVE_TERRAIN_JOIN_578F46;
            }
        }

        tile = map_loc(current_x,current_y);
        if (vehicle.order_auto_type == 5 && (tile->bit & 0x10u) == 0) {
            const RulesTerraforming &mine = Terraforming[TERRA_MINE];
            const int mine_tech = low ? mine.preq_tech_sea : mine.preq_tech;
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
        has_tech(Terraforming[TERRA_ROAD].preq_tech, faction_id)) {
        vehicle.order = 0x09;
        return ENEMY_MOVE_TERRAIN_JOIN_578F46;
    }

    tile = map_loc(current_x,current_y);
    if ((tile->bit & 0x8000u) != 0 ||
        (tile->val3 & 0xc0u) >= 0x80u ||
        !has_tech(Terraforming[TERRA_FARM].preq_tech, faction_id)) {
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
struct SeaScoringState {
    int &vehicle_id;
    int &faction;
    int &x;
    int &y;
    int &plan;
    int &region;
    int &faction_base_distance;
    int &faction_base_id;
    int &unfiltered_distance;
    int &unfiltered_base;
    int &low_altitude;
    int &region_plan;
    int &upstream_route;
    int &scan_priority;
    int &selection_request;
    int &direction;
    int &chosen_base;
    int &available;
    int &target_region;
    int &target_strategy;
    int &scratch58;
    int &unavailable;
    int &scratch4c;
    int &capacity_limit;
    int &scratch44;
    int &score;
    int &mode;
    unsigned int &mask;
    int &best;
    int &index;
    int &scratch20;
    int &stack_vehicle;
    int &scan_x;
    int &scan_y;
    int (&coast_y)[64], (&coast_x)[64];
    explicit SeaScoringState(EnemyMoveFullFrame &f)
        : vehicle_id(f.vehicle_id),
          faction(f.local_c),
          x(f.local_18),
          y(f.local_14),
          plan(f.local_50),
          region(f.local_34),
          faction_base_distance(f.local_64),
          faction_base_id(f.local_5c),
          unfiltered_distance(f.local_7c),
          unfiltered_base(f.local_9c),
          low_altitude(f.local_74),
          region_plan(f.local_88),
          upstream_route(f.local_8c),
          scan_priority(f.local_90),
          selection_request(f.local_b8),
          direction(f.local_80),
          chosen_base(f.local_78),
          available(f.local_6c),
          target_region(f.local_68),
          target_strategy(f.local_60),
          scratch58(f.local_58),
          unavailable(f.local_54),
          scratch4c(f.local_4c),
          capacity_limit(f.local_48),
          scratch44(f.local_44),
          score(f.local_38),
          mode(f.local_30),
          mask(f.local_2c),
          best(f.local_28),
          index(f.local_24),
          scratch20(f.local_20),
          stack_vehicle(f.local_1c),
          scan_x(f.local_4),
          scan_y(f.local_8),
          coast_y(f.coast_y),
          coast_x(f.coast_x) {}
};

static unsigned int sea_category_bit(int weapon) {
    return weapon == 1 ? 7u : 1u << (weapon & 31);
}
static int sea_base_region(int base) {
    return map_loc(Bases[base].x, Bases[base].y)->region;
}
static int sea_base_distance(const SeaScoringState &s, const Base &b) {
    return (abs(s.y - b.y) + x_dist(s.x, b.x)) >> 1;
}
static int sea_base_connected(const SeaScoringState &s, int base) {
    return s.faction_base_distance
        ? base_on_sea(base, s.region)
        : port_to_port(s.faction_base_id, base);
}

static SeaScoringExit enemy_move_57666a_tail(SeaScoringState &s);

static SeaScoringExit enemy_move_sea_scoring(SeaScoringState &s) {
    Veh &acting = Vehs[s.vehicle_id];
    PlayerData &player = PlayersData[s.faction];
    // Entry574AC4 belongs to plan7. Entry574A88 initialized the five values
    // below before testing plan7; do not initialize capacity_limit/chosen_base.
    s.available = 0;
    s.scratch44 = 0;
    s.unavailable = 0;
    s.mode = 0;
    s.mask = 0;
    if (!s.faction_base_distance) stack_sort_2(s.vehicle_id);
    s.scratch58 = 0;
    int count = 0;
    for (s.stack_vehicle = veh_top(s.vehicle_id); s.stack_vehicle >= 0;
         s.stack_vehicle = Vehs[s.stack_vehicle].next_veh_id_stack) {
        const int id = s.stack_vehicle;
        if (id == s.vehicle_id) continue;
        Veh &v = Vehs[id];
        VehPrototype &p = VehPrototypes[v.proto_id];
        if (Chassis[p.chassis_id].triad) continue;
        if (veh_cargo(id)) continue;
        if (v.faction_id != s.faction) continue;
        if ((v.state & 0x40000u) && !s.unfiltered_distance &&
            (!(v.state & 0x20000u) || s.region_plan != 7)) continue;
        // proto_power accepts VEHICLE ID, not prototype ID.
        const bool capacity = v.dmg_incurred <= proto_power(id) / 3;
        const int candidate_plan = p.plan;
        if (v.order != 2) {
            if (!s.low_altitude && candidate_plan == 12) {
                v.order = 2;
                continue;
            }
            if (s.low_altitude &&
                ((v.order == 1 &&
                  (v.waypoint_x[0] < 0 || v.waypoint_x[0] == s.vehicle_id)) ||
                 (v.order == 0 && s.unfiltered_distance))) {
                ++count;
                if (capacity) s.mask |= sea_category_bit(candidate_plan);
                else ++s.unavailable;
                continue;
            }
            if (candidate_plan == 9) continue;
            if (candidate_plan == 2 && s.scratch44 < s.capacity_limit) {
                if (!s.faction_base_distance) {
                    ++s.scratch44;
                    v.order = 2;
                    continue;
                }
            } else if (!s.faction_base_distance && !capacity) {
                v.order = 2;
                continue;
            }
            ++count;
            v.order = 0;
            if (capacity) s.mask |= sea_category_bit(candidate_plan);
        } else {
            if (s.faction_base_distance ||
                (candidate_plan == 12 && !s.low_altitude) || !capacity) continue;
            bool clear_order = candidate_plan > 8;
            if (!clear_order) {
                if (candidate_plan != 2 && !(v.state & 0x10000000u)) {
                    const int stack = stack_check(id, 2, 2, -1, -1);
                    if (stack || (p.plan != 1 && p.plan != 3)) {
                        clear_order = stack_check(id, 0x10, -1, -1, -1) != 0;
                    }
                }
                if (!clear_order) {
                    ++s.scratch44;
                    if (s.scratch44 <= s.capacity_limit ||
                        (s.region_plan <= 2 && s.scratch58)) continue;
                    ++s.scratch58;
                }
            }
            ++count;
            v.order = 0;
            s.mask |= sea_category_bit(VehPrototypes[v.proto_id].plan);
        }
    }
    s.available = count;
    if (!count && acting.order == 0x18 &&
        (acting.move_to_ai_type == 'J' || acting.move_to_ai_type == 'r' ||
         acting.move_to_ai_type == 'I' || acting.move_to_ai_type == 'E'))
        acting.order = 0;
    if (s.mask & 0x80fu) {
        s.mode = 2;
        if ((s.mask & 0x100u) && !(s.mask & 3u)) s.mode = 1;
    }
    if ((acting.move_to_ai_type == 'E' || acting.move_to_ai_type == 'r' ||
         acting.move_to_ai_type == 'I') && (!s.mode || acting.order != 0x18))
        acting.move_to_ai_type = '-';

    if (!s.upstream_route &&
        !(acting.order == 0x18 && acting.move_to_ai_type == 'd')) {
        if (((s.mask & 0x1000u) && s.unfiltered_distance) ||
            (s.unavailable && count <= s.unavailable &&
             (s.unfiltered_distance <= 8 || TurnCurrentNum < 50))) {
            s.best = 9999;
            for (s.index = 0; s.index < BaseCurrentCount; ++s.index) {
                Base &b = Bases[s.index];
                if (b.faction_id_current != s.faction ||
                    !base_on_sea(s.index, s.region)) continue;
                s.score = vector_dist(x_dist(s.x, b.x), abs(s.y - b.y));
                if (!s.score) return SEA_JOIN_578B34;
                // Native Base+2e dword spans unk_y and low state word.
                const unsigned int word = (unsigned short)b.unk_y |
                    ((b.state & 0xffffu) << 16);
                if (word & 0x400u) s.score += 1000;
                if (s.score < s.best) {
                    s.best = s.score;
                    go_to(s.vehicle_id, 0x61, b.x, b.y);
                }
            }
        }
        int free_cargo = veh_cargo(s.vehicle_id) - s.available;
        if ((acting.state & 0x20000000u) && !s.available) free_cargo = 0;
        bool scan_ports = free_cargo == veh_cargo(s.vehicle_id);
        if (!scan_ports && !s.faction_base_distance)
            scan_ports = veh_cargo(s.vehicle_id) / 2 <= free_cargo &&
                         !(s.mask & 0x100u);
        if (scan_ports) {
            s.best = 9999;
            s.target_strategy = 0;
            for (s.index = 0; s.index < BaseCurrentCount; ++s.index) {
                Base &b = Bases[s.index];
                if (b.faction_id_current != s.faction || !is_port(s.index, 0))
                    continue;
                s.target_region = sea_base_region(s.index);
                if (player.region_base_plan[s.target_region] == 2) continue;
                s.score = sea_base_distance(s, b);
                s.scratch44 = s.score;
                const bool same_waypoint = acting.waypoint_x[0] == b.x &&
                                           acting.waypoint_y[0] == b.y;
                const int threshold = s.scratch44 && !same_waypoint ? 1 : 2;
                bool try_base = false;
                if (player.x_target >= 0 && s.target_region ==
                    map_loc(player.x_target, player.y_target)->region) {
                    s.scratch4c = 1;
                    if ((player.flags & 0x800u) && b.x == player.x_target &&
                        b.y == player.y_target) {
                        try_base = stack_check(veh_at(b.x, b.y), 2, 7, -1, -1)
                                   < threshold;
                    }
                } else if (!b.unk_1 || (acting.order == 0x18 && same_waypoint) ||
                    (!s.faction_base_distance && s.faction_base_id == s.index &&
                     stack_check(s.vehicle_id, 2, 7, -1, -1) <= 1)) {
                    if (stack_check(veh_at(b.x, b.y), 2, 7, -1, -1) < threshold) {
                        s.scratch4c = 0;
                        try_base = true;
                    }
                }
                if (!try_base || !sea_base_connected(s, s.index)) continue;
                const int r = s.target_region;
                const unsigned short combat = player.region_total_combat_vehs[r];
                const unsigned short flags = player.unk_77[r];
                if ((unsigned short)(player.region_total_bases[r] * 2 + 1) >=
                    combat && !(flags & 3u) && !(flags & 0x1800u)) continue;
                s.score = s.scratch44 + 4;
                if (player.region_base_plan[r] != 7 && !(flags & 0x800u) &&
                    !same_waypoint) {
                    if ((s.index + TurnCurrentNum + s.vehicle_id) & 7) continue;
                    s.score *= 2;
                }
                if ((s.scratch44 == 0 ? 1 : 0) <
                    stack_check(veh_at(b.x, b.y), 2, 7, s.faction, -1))
                    s.score *= 2;
                s.score += player.region_total_bases[r] * 2 - combat;
                if (s.score < s.best) {
                    s.chosen_base = s.index;
                    s.scratch58 = player.region_base_plan[r];
                    s.target_strategy = s.scratch4c;
                    s.best = s.score;
                    go_to(s.vehicle_id, 0x65, b.x, b.y);
                }
            }
            const int limit = s.target_strategy || s.scratch58 == 7 ? 6 : 2;
            const int passengers = range(limit, 0, veh_cargo(s.vehicle_id));
            if (s.target_strategy || s.best <=
                veh_moves(s.vehicle_id) * passengers / 2)
                assemble_passengers(s.chosen_base, s.scratch58, passengers);
        }
    }

    if (acting.move_to_ai_type == 'e' && acting.waypoint_x[0] == s.x &&
        acting.waypoint_y[0] == s.y)
        Bases[s.unfiltered_base].state |= 1u;
    if (s.unavailable < s.available && acting.move_to_ai_type != 'E') {
        for (s.scratch58 = 0; s.scratch58 < 32; s.scratch58 += 4) {
            const int n = s.scratch58 / 4;
            s.scan_x = xrange(s.x + RadiusBaseX[n]);
            s.scan_y = s.y + RadiusBaseY[n];
            if (!on_map(s.scan_x, s.scan_y)) continue;
            Map *tile = map_loc(s.scan_x, s.scan_y);
            if ((tile->climate & 0xe0) < 0x60) continue;
            s.target_region = tile->region;
            const int r = s.target_region;
            if ((r & 0x3f) == 0x3f || !(r & 0x3f)) continue;
            s.unavailable = s.scan_priority > 2;
            int foreign = whose_territory(s.faction, s.scan_x, s.scan_y, 0, 0);
            if (foreign >= 0 && foreign != s.faction) {
                unsigned int treaty = player.diplo_treaties[foreign];
                if (treaty & 1u) continue;
                if (treaty & 0xc10u) {
                    if (s.mode) s.unavailable = 1;
                } else {
                    if (treaty & 2u) {
                        if (!(LockEnableMask & (unsigned char)(1u << (foreign & 31))) ||
                            !s.mode || TurnCurrentNum -
                            PlayersData[foreign].diplo_spoke[s.faction] < 16)
                            continue;
                        diplomacy_check(foreign, s.faction, 0);
                        if (!DiploScratchFA60 && !DiploScratchFA3C) continue;
                    } else if ((treaty & 4u) &&
                        (!(LockEnableMask & (unsigned char)(1u << (foreign & 31))) ||
                         TurnCurrentNum - PlayersData[foreign].diplo_spoke[s.faction] < 16))
                        continue;
                    // Original rereads treaty byte after diplomacy_check.
                    treaty = player.diplo_treaties[foreign];
                    if (!(treaty & 8u) && s.mode && !(s.mask & 0x100u))
                        s.unavailable = 1;
                }
            }
            foreign = veh_who(s.scan_x, s.scan_y);
            if (foreign >= 0 && foreign != s.faction) continue;
            bool enter_mode = true;
            if ((s.mask & 0x100u) && (int)Continents[r].open_terrain >= 4 &&
                (!player.region_total_bases[r] || player.region_base_plan[r] == 8)) {
                if (s.mode || player.region_base_plan[s.region] >= 3)
                    s.unavailable = 1;
                else enter_mode = false;
            }
            if (enter_mode && s.mode) {
                if (!s.unavailable && !player.region_total_bases[r] &&
                    player.region_base_plan[r] == 8) {
                    for (foreign = 1; foreign < 8; ++foreign) {
                        const unsigned int treaty = player.diplo_treaties[foreign];
                        if (PlayersData[foreign].region_total_bases[r] &&
                            ((treaty & 0x10u) || !(treaty & 8u) ||
                             TurnCurrentNum - PlayersData[foreign].diplo_spoke[s.faction] > 16))
                            s.unavailable = 1;
                    }
                }
                if (player.region_base_plan[r] < 3) s.unavailable = 1;
            }
            if (!s.unavailable) continue;
            int movement_gate = s.unavailable;
            if (!(s.mask & (1u << (player.region_base_plan[r] & 31))) &&
                s.scan_priority <= 2 && !(GameState & 0x100u) &&
                (acting.order != 0x18 || r != map_loc(
                    acting.waypoint_x[0], acting.waypoint_y[0])->region ||
                 acting.move_to_ai_type == 'e')) {
                s.stack_vehicle = veh_top(s.vehicle_id);
                bool reject_tile = false;
                for (; s.stack_vehicle >= 0;
                     s.stack_vehicle = Vehs[s.stack_vehicle].next_veh_id_stack) {
                    const Veh &v = Vehs[s.stack_vehicle];
                    if (Chassis[VehPrototypes[v.proto_id].chassis_id].triad ||
                        v.order != 1 || (v.waypoint_x[0] >= 0 &&
                        v.waypoint_x[0] != s.vehicle_id) || v.home_base_id < 0 ||
                        sea_base_region(v.home_base_id) != r) continue;
                    foreign = zoc_veh(s.scan_x, s.scan_y, s.faction);
                    if (foreign <= 1) { reject_tile = true; break; }
                    const unsigned int treaty = player.diplo_treaties[foreign - 1];
                    if (((!(treaty & 0x10u) || !s.mode || !(s.mask & 3u)) &&
                         (treaty & 8u)) || !s.unavailable) {
                        reject_tile = true;
                        break;
                    }
                    movement_gate = -1;
                    break;
                }
                if (reject_tile) continue;
            }
            s.direction = -1;
            if (veh_moves(s.vehicle_id) >= (int)Rules.move_rate_roads &&
                s.unfiltered_distance && movement_gate >= 1) {
                s.best = -1;
                s.mode = 0;
                for (s.mode = 0; s.mode < 9; ++s.mode) {
                    const int pos = s.mode;
                    s.scan_x = xrange(s.x + RadiusOffsetX[pos]);
                    s.scan_y = s.y + RadiusOffsetY[pos];
                    if (!on_map(s.scan_x, s.scan_y) ||
                        map_loc(s.scan_x, s.scan_y)->region != s.region) continue;
                    const int owner = veh_who(s.scan_x, s.scan_y);
                    if (owner >= 0 && veh_who(s.scan_x, s.scan_y) != s.faction)
                        continue;
                    s.scratch20 = 0;
                    for (int d = 1; d <= 8; ++d) {
                        const int nx = xrange(s.scan_x + RadiusOffsetX[d]);
                        const int ny = s.scan_y + RadiusOffsetY[d];
                        if (!on_map(nx, ny) || map_loc(nx, ny)->region != r) continue;
                        if (base_who(nx, ny) >= 1) s.scratch20 += 4;
                        ++s.scratch20;
                    }
                    if (s.best < s.scratch20) {
                        s.best = s.scratch20;
                        s.direction = pos - 1;
                    }
                }
            }
            for (s.stack_vehicle = veh_top(s.vehicle_id); s.stack_vehicle >= 0;
                 s.stack_vehicle = Vehs[s.stack_vehicle].next_veh_id_stack) {
                Veh &v = Vehs[s.stack_vehicle];
                if (!Chassis[VehPrototypes[v.proto_id].chassis_id].triad)
                    v.order = 0;
            }
            acting.move_to_ai_type = 'u';
            return s.selection_request ? SEA_RETURN_ZERO : SEA_JOIN_578B26;
        }
    }
    if (s.selection_request) return SEA_RETURN_ZERO;
    if (s.upstream_route) return SEA_JOIN_57717F;
    if (acting.order == 0x18 && acting.move_to_ai_type == 'd')
        return SEA_JOIN_57717F;
    if (acting.order != 0x18 && (s.mask & 0x100u) && s.mode <= 0) {
        for (int coast_index = 0; coast_index < 64; ++coast_index)
            s.coast_x[coast_index] = s.coast_y[coast_index] = -1;
        for (s.scan_y = 1; s.scan_y < MapLatitudeBounds - 1; ++s.scan_y) {
            for (s.scan_x = s.scan_y & 1; s.scan_x < MapLongitudeBounds;
                 s.scan_x += 2) {
                const int r = map_loc(s.scan_x, s.scan_y)->region;
                if (r <= 0x3f && (s.coast_y[r] < 10 ||
                    MapLatitudeBounds - 10 < s.coast_y[r]) &&
                    is_coast(s.scan_x, s.scan_y, 0)) {
                    s.coast_x[r] = s.scan_x;
                    s.coast_y[r] = s.scan_y;
                }
            }
        }
        s.chosen_base = -1;
        s.best = 0;
        s.target_region = 1;
        s.scratch58 = 0;
        for (int r = 1; r < 64; ++r, s.scratch58 += 4, ++s.target_region) {
            const int tiles = (int)Continents[r].tile_count;
            if (tiles < 10 || player.region_total_bases[r] ||
                !on_map(s.coast_x[r], s.coast_y[r])) continue;
            int enemies = 0;
            for (int f = 1; f < 8; ++f) {
                const int bases = PlayersData[f].region_total_bases[r];
                if (!bases) continue;
                enemies += tiles / 4 < bases * 20 ? 3 : 1;
                if (player.diplo_treaties[f] & 7u) enemies += 2;
            }
            if (enemies > 1) continue;
            const int distance = vector_dist(x_dist(s.x, s.coast_x[r]),
                                            abs(s.y - s.coast_y[r]));
            const int rating = tiles * 100 / (distance + 1);
            if (s.best <= rating) { s.chosen_base = r; s.best = rating; }
        }
        if (s.chosen_base >= 0) set_course(s.vehicle_id, 99,
            s.coast_x[s.chosen_base], s.coast_y[s.chosen_base]);
    }
    if (acting.order != 0x18 && s.mode) {
        s.best = 9999;
        for (s.index = 0; s.index < BaseCurrentCount; ++s.index) {
            Base &b = Bases[s.index];
            if (b.faction_id_current != s.faction) continue;
            s.target_region = sea_base_region(s.index);
            if (player.region_base_plan[s.target_region] != 2 ||
                !sea_base_connected(s, s.index)) continue;
            s.score = sea_base_distance(s, b);
            if (veh_who(b.x, b.y) < 0) s.score /= 2;
            if (at_goal(s.faction, 2, b.x, b.y)) s.score /= 2;
            if (player.base_id_atk_target >= 0) {
                const int target = player.base_id_atk_target;
                if ((player.diplo_treaties[Bases[target].faction_id_current] &
                     0x10u) && sea_base_region(target) == s.target_region)
                    s.score = 0;
            }
            if (s.score < s.best) {
                s.best = s.score;
                go_to(s.vehicle_id, 'E', b.x, b.y);
            }
        }
        if (acting.order != 0x18 && s.mode >= 2) {
            s.best = 9999;
            for (s.index = 0; s.index < BaseCurrentCount; ++s.index) {
                Base &b = Bases[s.index];
                const int f = b.faction_id_current;
                if (f == s.faction || !(is_objective(s.index) ||
                    (player.flags & 0x200000u) ||
                    player.base_id_atk_target == s.index ||
                    !((s.index + TurnCurrentNum) & 3))) continue;
                // Native57602D repurposes -30 from mode into the faction's
                // dword-table stride. Retain the observable reaching integer.
                s.mode = s.faction * 0x833;
                const unsigned int treaty = player.diplo_treaties[f];
                const bool human = (LockEnableMask &
                    (unsigned char)(1u << (f & 31))) != 0;
                if ((treaty & 1u) || ((treaty & 2u) &&
                    (!human || !(treaty & 0xc00u))) ||
                    ((treaty & 4u) && (!human || (!(treaty & 0xc00u) &&
                    TurnCurrentNum - PlayersData[f].diplo_spoke[s.faction] < 16))))
                    continue;
                const int r = sea_base_region(s.index);
                if (r == s.region && !s.low_altitude) continue;
                if (!(s.faction_base_distance ? sea_coast(r, s.region) :
                    port_to_coast(s.faction_base_id, r))) continue;
                s.unavailable = 1;
                for (s.stack_vehicle = veh_top(s.vehicle_id); s.stack_vehicle >= 0;
                     s.stack_vehicle = Vehs[s.stack_vehicle].next_veh_id_stack) {
                    const int home = Vehs[s.stack_vehicle].home_base_id;
                    if (home >= 0 && sea_base_region(home) == r) s.unavailable = 0;
                }
                if (!s.unavailable && !(treaty & 0x10u) && (treaty & 8u) &&
                    !(GameState & 0x100u) &&
                    !(s.mask & (1u << (player.region_base_plan[r] & 31)))) continue;
                const int distance = sea_base_distance(s, b);
                s.score = distance * 32;
                if (f != s.faction && !(b.visibility &
                    (unsigned char)(1u << (s.faction & 31)))) s.score = distance * 64;
                if (b.queue_production_id[0] == -102) s.score /= 4;
                if (has_fac(1, s.index, 0)) {
                    s.score /= 2;
                    if (player.flags & 0x200000u) s.score /= 2;
                    if (TurnCurrentNum < PlayersData[f].corner_market_turn)
                        s.score /= 8;
                }
                if (is_objective(s.index)) {
                    s.score /= 2;
                    if (player.flags & 0x80000000u) s.score /= 2;
                }
                if (treaty & 0x40u) s.score /= 2;
                if (treaty & 0x10u) {
                    s.score /= 2;
                    if (veh_who(b.x, b.y) < 0) s.score /= 2;
                    if (player.base_id_atk_target == s.index) s.score = 0;
                }
                if (has_fac(1, s.index, 0)) s.score /= 2;
                s.stack_vehicle = veh_at(b.x, b.y);
                bool low_garrison = s.stack_vehicle < 0;
                if (!low_garrison) {
                    s.scratch4c = stack_check(s.stack_vehicle, 0xe, -1, -1, -1);
                    low_garrison = s.scratch4c < garrison_check(s.index);
                }
                if (low_garrison && human && (PlayersData[f].diff_level >= 4 ||
                    (player.flags & 0x200u) || (treaty & 0x1000u))) s.score /= 2;
                if (s.score < s.best) {
                    s.best = s.score;
                    set_course(s.vehicle_id, 'I', b.x, b.y);
                }
            }
        }
        if (acting.order != 0x18) {
            s.best = 9999;
            for (s.index = 0; s.index < BaseCurrentCount; ++s.index) {
                Base &b = Bases[s.index];
                if (b.faction_id_current != s.faction) continue;
                const int r = sea_base_region(s.index);
                const int p = player.region_base_plan[r];
                if (r == s.region || p == 7 || (s.mask & 0x100u) ||
                    !(s.mask & (1u << (p & 31))) ||
                    !sea_base_connected(s, s.index)) continue;
                s.score = sea_base_distance(s, b);
                if (s.score < s.best) {
                    s.best = s.score;
                    go_to(s.vehicle_id, 'r', b.x, b.y);
                }
            }
        }
        if (acting.order != 0x18) {
            for (int pos = 1; pos <= 288; ++pos) {
                s.scan_x = xrange(s.x + RadiusOffsetX[pos] * 2);
                s.scan_y = s.y + RadiusOffsetY[pos] * 2;
                if (!on_map(s.scan_x, s.scan_y)) continue;
                Map *tile = map_loc(s.scan_x, s.scan_y);
                const int r = tile->region;
                if ((tile->climate & 0xe0) < 0x60 || (r & 0x3f) == 0x3f ||
                    !(r & 0x3f) || player.region_total_bases[r] ||
                    !(s.mask & (1u << (player.region_base_plan[s.region] & 31))))
                    continue;
                if (!(s.faction_base_distance ? sea_coast(r, s.region) :
                    port_to_coast(s.faction_base_id, r))) continue;
                set_course(s.vehicle_id, 'J', s.scan_x, s.scan_y);
                break;
            }
        }
    }
    return enemy_move_57666a_tail(s);
}

// Other plans reaching57666A must call this tail directly, retaining their
// live score and without running plan7 initialization or stack mutation.
static SeaScoringExit enemy_move_57666a_tail(SeaScoringState &s) {
    Veh &acting = Vehs[s.vehicle_id];
    if (acting.order == 0x18 && acting.move_to_ai_type == 'd')
        return SEA_JOIN_577071;
    if (s.plan != 6) return SEA_JOIN_576DAF;
    if (VehPrototypes[acting.proto_id].unk_1 != 21) return SEA_JOIN_576835;
    s.best = 9999;
    for (s.stack_vehicle = 0; s.stack_vehicle < VehCurrentCount;
         ++s.stack_vehicle) {
        const Veh &v = Vehs[s.stack_vehicle];
        if (v.faction_id != s.faction ||
            VehPrototypes[v.proto_id].plan != 7) continue;
        s.scan_x = v.x;
        s.scan_y = v.y;
        if ((map_loc(s.scan_x, s.scan_y)->climate & 0xe0) >= 0x60 ||
            base_who(s.scan_x, s.scan_y) >= 0) continue;
        s.score = (abs(s.y - s.scan_y) + x_dist(s.x, s.scan_x)) >> 1;
        if (s.score) {
            if (stack_check(s.stack_vehicle, 2, 6, -1, -1)) continue;
            // The original divisor can be zero. Do not insert a guard.
            s.score /= stack_check(s.stack_vehicle, 1, -1, -1, -1);
        }
        if (s.score < s.best) {
            s.best = s.score;
            go_to(s.vehicle_id, 'C', s.scan_x, s.scan_y);
        }
    }
    return SEA_JOIN_57681D;
}

// Complete57681D/576835/576DAF continuation. Include aftersea_scoring.cpp.
// Result joins are the original shared577071 split or578B34 action owner.
static SeaScoringExit enemy_move_sea_after_scoring(
    SeaScoringState &s,SeaScoringExit entry) {
    Veh &acting=Vehs[s.vehicle_id];
    PlayerData &player=PlayersData[s.faction];
    if(entry==SEA_JOIN_576DAF)goto sea576daf;
    if(entry==SEA_JOIN_57681D){
        if(acting.order==0x18)return s.score?SEA_JOIN_577071:SEA_JOIN_578B34;
    } else if(entry==SEA_JOIN_576835){
        if(acting.order==0x18&&acting.move_to_ai_type!='6')goto sea576a95;
    }
    //576847: faction rule100 gates a complete hostile-base target scan.
    if(Players[s.faction].rule_flags&0x100u){
        s.best=0;s.chosen_base=-1;
        for(s.index=0;s.index<BaseCurrentCount;++s.index){
            const Base &b=Bases[s.index];
            if(b.faction_id_current==s.faction||
                !(player.diplo_treaties[b.faction_id_current]&0x10u))continue;
            // Original nonzero-distance arm checks the ORIGINAL friendly
            //base, not the current foreign candidate. Preserve that quirk.
            if(!(s.faction_base_distance?
                base_on_sea(s.faction_base_id,s.region):
                port_to_port(s.faction_base_id,s.index)))continue;
            const int distance=(abs(s.y-b.y)+x_dist(s.x,b.x))>>1;
            s.scratch20=(b.population_size*b.population_size*32)/(distance+1);
            for(s.stack_vehicle=0;s.stack_vehicle<VehCurrentCount;++s.stack_vehicle){
                if(s.stack_vehicle==s.vehicle_id)continue;
                const Veh &v=Vehs[s.stack_vehicle];
                if(v.faction_id!=s.faction||VehPrototypes[v.proto_id].plan!=6)continue;
                const int delta=abs(v.y-b.y)+x_dist(v.x,b.x);
                if((delta&~1)<=4 || (v.move_to_ai_type=='6'&&
                    v.waypoint_x[0]==b.x&&v.waypoint_y[0]==b.y))s.scratch20/=2;
            }
            if(s.best<s.scratch20){s.chosen_base=s.index;s.best=s.scratch20;
                go_to(s.vehicle_id,'6',b.x,b.y);}
        }
        if(s.chosen_base>=0){const Base &b=Bases[s.chosen_base];
            if(((abs(s.y-b.y)+x_dist(s.x,b.x))&~1)<=2){
                acting.order=0;return SEA_JOIN_577071;}}
    }
sea576a95:
    if(acting.order!=0x18&&!((s.vehicle_id+TurnCurrentNum)&3)){
        s.best=-9999;
        for(s.scan_y=0;s.scan_y<MapLatitudeBounds;++s.scan_y){
            for(s.scan_x=s.scan_y&1;s.scan_x<MapLongitudeBounds;s.scan_x+=2){
                const Map &tile=*map_loc(s.scan_x,s.scan_y);
                if((tile.climate&0xe0)>=0x60)continue;
                if(s.faction_base_distance){if(tile.region!=s.region)continue;}
                else if(!base_on_sea(s.faction_base_id,tile.region))continue;
                if(((abs(s.y-s.scan_y)+x_dist(s.x,s.scan_x))&~1)<16)continue;
                bool assigned=false;
                for(s.stack_vehicle=0;s.stack_vehicle<VehCurrentCount;++s.stack_vehicle){
                    const Veh &v=Vehs[s.stack_vehicle];
                    if(s.stack_vehicle!=s.vehicle_id&&v.faction_id==s.faction&&
                        v.waypoint_x[0]==s.scan_x&&v.waypoint_y[0]==s.scan_y){assigned=true;break;}}
                if(assigned)continue;
                s.scratch20=0;
                for(s.scratch44=0;s.scratch44<=0x11f;s.scratch44+=4){
                    const int pos=9+s.scratch44/4;
                    const int x=xrange(s.scan_x+RadiusOffsetX[pos]);
                    const int y=s.scan_y+RadiusOffsetY[pos];
                    if(!on_map(x,y)||(map_loc(x,y)->climate&0xe0)<0x60)continue;
                    const int ox=xrange(s.scan_x-RadiusOffsetX[pos]);
                    const int oy=s.scan_y-RadiusOffsetY[pos];
                    if(!on_map(ox,oy)||(map_loc(ox,oy)->climate&0xe0)<0x60)continue;
                    const int r=map_loc(x,y)->region;
                    const int opposite=map_loc(ox,oy)->region;
                    if(player.region_total_bases[r]<2||player.region_total_bases[opposite]>1)continue;
                    for(int f=1;f<8;++f){
                        if(f!=s.faction&&(player.diplo_treaties[f]&0xc10u)&&
                            PlayersData[f].region_total_bases[r]<=1&&
                            PlayersData[f].region_total_bases[opposite]>=2)++s.scratch20;
                    }
                }
                if(s.scratch20){
                    const int distance=vector_dist(x_dist(s.x,s.scan_x),abs(s.y-s.scan_y));
                    // Original signed divide remains unguarded.
                    s.score=(s.scratch20<<8)/distance;
                    if(s.best<s.score){s.best=s.score;go_to(s.vehicle_id,'D',s.scan_x,s.scan_y);}
                }
            }
        }
    }
sea576daf:
    if(acting.order==0x18&&(!(player.flags&0x20000000u)||
        stack_check(s.vehicle_id,1,-1,-1,-1)!=1))return SEA_JOIN_577071;
    if((s.vehicle_id+TurnCurrentNum)&3)return SEA_JOIN_577071;
    s.best=-9999;
    for(s.scan_y=0;s.scan_y<MapLatitudeBounds;++s.scan_y){
        for(s.scan_x=s.scan_y&1;s.scan_x<MapLongitudeBounds;s.scan_x+=2){
            if(!is_known(s.scan_x,s.scan_y,s.faction))continue;
            const Map &tile=*map_loc(s.scan_x,s.scan_y);
            if((tile.climate&0xe0)>=0x60)continue;
            if(s.faction_base_distance){if(tile.region!=s.region)continue;}
            else if(!base_on_sea(s.faction_base_id,tile.region))continue;
            s.scratch20=0;
            if((player.flags&0x20000000u)&&!(tile.bit&0x100000u)){
                s.scratch20=(tile.bit&0x20u)&&(tile.climate&0xe0)>=0x40?2:1;
                if(tile.bit&0x2000u)++s.scratch20;
            }
            for(int n=0;n<8;++n){
                const int x=xrange(s.scan_x+RadiusBaseX[n]);
                const int y=s.scan_y+RadiusBaseY[n];
                if(!on_map(x,y))continue;
                if(!(player.flags&0x20000000u)||(map_loc(x,y)->bit&0x100000u)){
                    if(is_known(x,y,s.faction))continue;
                }
                s.scratch20+=y>=2&&y<MapLatitudeBounds-3?6:2;
            }
            if(!s.scratch20)continue;
            int reference_x=s.x,reference_y=s.y;
            if(s.faction_base_id>=0){reference_x=Bases[s.faction_base_id].x;
                reference_y=Bases[s.faction_base_id].y;}
            const int distance=vector_dist(x_dist(reference_x,s.scan_x),abs(reference_y-s.scan_y));
            if(!distance)continue; // Native576Fxx explicitly tests zero.
            s.score=(s.scratch20<<8)/distance;
            if(s.best<s.score){s.best=s.score;go_to(s.vehicle_id,'X',s.scan_x,s.scan_y);}
        }
    }
    return SEA_JOIN_577071;
}

// Pinned enemy_move577443..579208. Complete tactical scan and action tail.
// No ORIGINAL marker: integration fragment, not a separately mapped function.
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

struct TacticalFrame {
    int &vehicle;
    int &faction;
    int &x;
    int &y;
    int &triad;
    int &plan;
    int &proto;
    int &region;
    int &faction_base;
    int &faction_distance;
    int &unfiltered_base;
    int &unfiltered_distance;
    int &low_altitude;
    int &existing_route;
    int &center_bias;
    int &vehicle_zoc;
    int &priority;
    int &explore_bonus;
    int &strategic60000;
    int &network;
    int &aggressive;
    int &sea_zoc;
    int &route_flag;
    int &region_plan;
    int &local84;
    int &direction;
    int &local70;
    explicit TacticalFrame(EnemyMoveFullFrame &f)
        : vehicle(f.vehicle_id),
          faction(f.local_c),
          x(f.local_18),
          y(f.local_14),
          triad(f.local_40),
          plan(f.local_50),
          proto(f.local_ac),
          region(f.local_34),
          faction_base(f.local_5c),
          faction_distance(f.local_64),
          unfiltered_base(f.local_9c),
          unfiltered_distance(f.local_7c),
          low_altitude(f.local_74),
          existing_route(f.local_c4),
          center_bias(f.local_c0),
          vehicle_zoc(f.local_bc),
          priority(f.local_b4),
          explore_bonus(f.local_b0),
          strategic60000(f.local_a8),
          network(f.local_a0),
          aggressive(f.local_98),
          sea_zoc(f.local_94),
          route_flag(f.local_8c),
          region_plan(f.local_88),
          local84(f.local_84),
          direction(f.local_80),
          local70(f.local_70) {}
};
static bool tactical_low(int x, int y) {
    return (map_loc(x,y)->climate & 0xe0u) < 0x60u;
}
static int tactical_offense(int unit) {
    return Weapon[VehPrototypes[Vehs[unit].proto_id].weapon_id].offense_rating;
}
static int enemy_move_tactical_postscan(TacticalFrame &s, int combat_choice);

// Shared final action/path owner. Keeps saved coordinates, rather than
// assuming that prior movement/probe calls left them equal to Veh.x/y.
static int enemy_move_tactical_action_tail(TacticalFrame &s) {
    Veh &v=Vehs[s.vehicle];
    if (v.order < 0x18) v.state &= ~0x10000u;
    if (v.order == 0 || v.order == 2) {
        if (s.triad != 2 || !s.faction_distance ||
            (map_loc(s.x,s.y)->bit & 0x40000u)) {
            v.order = (LockEnableMask & (unsigned char)(1u << (s.faction & 31))) &&
                      !(v.state & 0x200u) ? 0 : 2;
            veh_skip(s.vehicle);
        } else action_home(s.vehicle,0);
    }
    if (v.order == 0x18 && v.waypoint_x[0] == s.x && v.waypoint_y[0] == s.y) {
        v.order=0;
        veh_skip(s.vehicle);
    }
    if ((v.order != 0x18 && v.order != 0x58) ||
        (s.plan != 8 && s.plan != 9)) return 0;
    const int direction=Paths.move(s.vehicle,(v.order & 0x40)?-1:v.faction_id);
    if (direction<0) { v.order=0; return 0; }
    const int x=xrange(s.x+RadiusBaseX[direction]);
    const int y=s.y+RadiusBaseY[direction];
    if (base_who(x,y)>=0) return 0;
    for (int n=0;n<8;++n) {
        const int nx=xrange(x+RadiusBaseX[n]);
        const int ny=y+RadiusBaseY[n];
        if (!on_map(nx,ny) || tactical_low(nx,ny)!=tactical_low(x,y)) continue;
        if (((abs(ny-s.y)+x_dist(nx,s.x)) & ~1)<3) continue;
        const int enemy=stack_fix(veh_at(nx,ny));
        if (enemy<0 || Vehs[enemy].faction_id==s.faction ||
            !(PlayersData[s.faction].diplo_treaties[Vehs[enemy].faction_id] & 0x10u))
            continue;
        if (tactical_offense(enemy) || stack_check(enemy,0xe,-1,-1,-1)) {
            v.order=0;
            return 0;
        }
    }
    return 0;
}

static int enemy_move_tactical_tail(TacticalFrame &s) {
    Veh &acting=Vehs[s.vehicle];
    PlayerData &player=PlayersData[s.faction];
    int best=-9999;
    int combat_choice=0;
    const int initial_zone=zoc_sea(s.x,s.y,s.faction);
    if (initial_zone || s.existing_route || s.aggressive || s.strategic60000)
        s.aggressive=0;
    else {
        s.aggressive=1;
        if (s.plan==12) s.aggressive=0;
    }
    char bias_text[]="ENEMYBIAS";
    const int rotation=rnd(9,bias_text);
    for (int iteration=0;iteration<9;++iteration) {
        const int pos=(iteration+rotation)%9;
        const int x=xrange(s.x+RadiusOffsetX[pos]);
        const int y=s.y+RadiusOffsetY[pos];
        if (!on_map(x,y) || (!pos && !s.sea_zoc && s.plan!=2)) continue;
        const int low=tactical_low(x,y);
        s.local84=low;
        int unit=stack_fix(veh_at(x,y));
        const int base=base_at(x,y);
        const int region=map_loc(x,y)->region;
        int owner=s.local70;
        if (s.triad==1 && !low) {
            if (unit<0 || owner==s.faction || !s.low_altitude) continue;
        } else if (!s.low_altitude && !low && s.vehicle_zoc && !s.triad &&
            unit<0 && !(VehPrototypes[acting.proto_id].ability_flags & 4u) &&
            VehPrototypes[acting.proto_id].plan!=11 &&
            zoc_move(x,y,s.faction)) continue;
        if (!tactical_offense(s.vehicle) && !(unit>=0 && pos) && base<0 &&
            !(acting.state & 8u)) {
            owner=zoc_sea(x,y,s.faction);
            s.local70=owner;
            if (owner==1) continue;
            if (owner>0) {
                const unsigned treaty=player.diplo_treaties[owner-1];
                if ((treaty & 1u) && (treaty & 0x2000000u)) continue;
                if (s.plan!=11) { if(treaty & 0x10u)continue; }
                else if (acting.moves_expended) {
                    const int cost=hex_cost(s.proto,s.faction,s.x,s.y,x,y,0);
                    if (veh_moves(s.vehicle)-cost<=0 && (treaty & 0x10u)) continue;
                }
            }
        }
        char score_text[]="ENEMY3";
        int score=rnd(6,score_text);
        if (low && !s.triad) {
            bool eligible=base>=0 && has_abil(acting.proto_id,8);
            if (!eligible && unit>=0 && Vehs[unit].faction_id==s.faction &&
                stack_check(unit,8,-1,-1,-1)>=1 &&
                Vehs[unit].dmg_incurred<=VehPrototypes[Vehs[unit].proto_id].reactor_id*2u) {
                if (s.plan==12) { score+=99;eligible=true; }
                else if (s.plan!=8) {
                    if (s.plan!=9 && ((player.region_base_plan[s.region]==7 &&
                        s.plan<2) || (!player.region_total_bases[s.region] &&
                        !(player.unk_77[s.region]&6u) &&
                        player.region_base_plan[s.region]>=3))) {score+=16;eligible=true;}
                } else if (player.region_base_plan[s.region]==7) {score+=16;eligible=true;}
            }
            if (!eligible)continue;
        }
        if (s.low_altitude && !low && !s.triad && !s.faction_distance &&
            !has_abil(acting.proto_id,8) && !stack_check(s.vehicle,2,7,-1,-1)) continue;
        if (s.low_altitude==low && (acting.state&0x10000u) &&
            (!s.sea_zoc || (s.sea_zoc>=2 &&
             (player.diplo_treaties[s.sea_zoc-1]&7u)))) {
            if (pos) {
                int angular=acting.facing-pos+1;
                if (angular<0)angular=pos-acting.facing-1;
                if (angular>=5)angular=8-angular;
                score-=angular*4;
                if (angular==4)score-=256;
            } else score-=12;
        }
        if(s.plan!=12){if(s.center_bias&&!pos)score+=8;}
        else {
            int coast_penalty=0;
            if(zoc_veh(x,y,s.faction))coast_penalty=-32;
            if(!is_coast(x,y,0))coast_penalty-=32;
            if(!pos&&!coast_penalty){score+=(s.center_bias?24:0)+8;
                base_find(x,y,s.faction);if(BaseFindDist<=s.faction_distance)score+=8;}
            score+=coast_penalty;
        }
        if(player.flags&0x20000000u){const Map &t=*map_loc(x,y);
            if(!(t.bit&0x100000u)){score+=(t.bit&0x20u)&&(t.climate&0xe0)>=0x40?16:8;
                if(t.bit&0x2000u)score+=8;}}
        owner=whose_territory(s.faction,x,y,0,0);
        s.local70=owner;
        if(owner>=0&&owner!=s.faction&&VehPrototypes[acting.proto_id].plan!=11){
            const unsigned treaty=player.diplo_treaties[owner];
            if(!(treaty&0xc10u)){
                if((player.diplo_agenda[owner]&2u)||(treaty&0x100000u))continue;
                if(treaty&4u)score-=6;
                if(treaty&2u){score-=12;
                    if(!(treaty&1u)&&(s.low_altitude||!(treaty&0x2020u)||SunspotDuration>0))continue;}
            }
        }
        if(s.strategic60000&&s.plan==2&&unit>=0&&owner==s.faction&&zoc_veh(x,y,s.faction))
            score-=stack_check(s.vehicle,1,-1,-1,-1)*2;
        if(s.low_altitude&&!s.triad&&s.faction_distance){
            if(player.region_base_plan[region]==s.plan)score+=32;
            if(player.region_base_plan[region]!=7)score+=32;
            base_find(x,y);
            score+=32-BaseFindDist-stack_check(unit,1,-1,-1,-1);
        }
        if(goody_at(x,y)){
            if(s.plan==7&&stack_check(s.vehicle,1,-1,-1,-1)>1)continue;
            score+=99;
        }
        if(!s.low_altitude&&!low&&s.region!=region&&player.region_total_bases[s.region]){
            bool permit=false;
            if(tactical_offense(s.vehicle)){
                if(s.plan==3)permit=true;
                else if(s.region_plan>=3)permit=!(player.unk_77[s.region]&0x20u);
            } else if(s.plan==8)permit=!player.unk_82[s.region];
            if(!permit)continue;
        }
        if(!(s.aggressive&&s.explore_bonus)){
            const int policy=s.strategic60000?2:s.plan;
            bool friendly=unit>=0&&(Vehs[unit].faction_id==s.faction||
                (player.diplo_treaties[Vehs[unit].faction_id]&1u));
            if(!pos&&stack_check(unit,1,unit,-1,-1)==1)friendly=false;
            // Native retains an unguarded Vehs[unit] read on this arm.
            if(s.low_altitude&&!s.triad&&policy!=8&&Vehs[unit].faction_id==s.faction){
                if(stack_check(unit,2,8,-1,-1))score+=8;
                if(policy<=1||policy==11){if(player.unk_77[region]&0x20u)score+=16;
                    if(player.unk_77[region]&0x40u)score+=16;}}
            switch(policy){
            case 0:
                if(friendly&&!s.aggressive)score+=stack_check(unit,2,2,-1,-1)*4;
                score-=stack_check(unit,2,0,-1,-1)*2;
                score-=hex_cost(s.proto,s.faction,s.x,s.y,x,y,0)/(int)Rules.move_rate_roads;
                break;
            case 1:if(zoc_veh(x,y,s.faction))score+=defense_value(s.faction,x,y,s.vehicle,-1)*2;break;
            case 2:
                if(zoc_veh(x,y,s.faction))score+=defense_value(s.faction,x,y,s.vehicle,-1)*2;
                if(friendly&&!s.aggressive)score+=stack_check(unit,2,0,-1,-1)*4-stack_check(unit,2,2,-1,-1)*2;
                break;
            case 3:score-=hex_cost(s.proto,s.faction,s.x,s.y,x,y,0)/(int)Rules.move_rate_roads;break;
            case 6:score+=stack_check(unit,2,7,-1,-1)*4-stack_check(unit,2,6,-1,-1)*2;break;
            case 7:if(!s.aggressive)score+=stack_check(unit,2,6,-1,-1)*4;
                score-=stack_check(unit,2,7,-1,-1)*8;break;
            case 8:
                if(s.low_altitude&&zoc_sea(x,y,s.faction))continue;
                if(owner>=0&&owner!=s.faction&&(player.diplo_treaties[owner]&3u)&&acting.order!=0x18)score-=16;
                if(s.low_altitude&&player.region_base_plan[region]==8){score+=Continents[region].open_terrain+16;
                    if(!(player.unk_77[region]&0x20u))score+=8;
                    if(!(player.unk_77[region]&0x10u))score+=16;}
                break;
            case 11:if(friendly)score+=stack_check(unit,0xe,-1,-1,-1)*2;break;
            }
            if((map_loc(s.x,s.y)->climate&0xe0)<(map_loc(x,y)->climate&0xe0)&&
                (Rules.combat_pct_atk_higher_elev||Rules.combat_pen_pct_atk_lwr_elev))score+=2;
        }
        int combat=0;
        if(unit<0){
            if(base>=0&&Bases[base].faction_id_current!=s.faction&&
                !(player.diplo_treaties[owner]&1u)){
                const int enemy= Bases[base].faction_id_current;
                if(s.plan==11&&!(VehPrototypes[acting.proto_id].ability_flags&0x10000000u)&&has_project(15,enemy))continue;
                const unsigned treaty=player.diplo_treaties[owner];
                if((treaty&7u)&&!(treaty&0x800u)){
                    if(!(treaty&0x20u))continue;
                    if(treaty&2u){char text[]="ENEMY4";
                        if(reputation(owner,s.faction)<rnd(6,text)-player.ai_fight)continue;}}
                if(!tactical_offense(s.vehicle))continue;
                combat=1;score+=9999;
            }
            if((map_loc(x,y)->bit&0x2000u)&&want_monolith(s.vehicle))score+=99;
        } else {
            const int enemy=Vehs[unit].faction_id;
            s.local70=enemy;
            const unsigned treaty=player.diplo_treaties[enemy];
            const bool direct= s.triad!=2 || !Chassis[VehPrototypes[acting.proto_id].chassis_id].range ||
                !acting.terraforming_turns || acting.order!=0x18 ||
                (acting.waypoint_x[0]==x&&acting.waypoint_y[0]==y);
            // Raw57813E/57814E skips this candidate immediately when the
            // ranged-air waypoint differs; Kuna's outer else is misleading.
            if(!direct)continue;
            const bool single_colony=stack_check(unit,1,-1,-1,-1)==1&&
                !(treaty&0x11u)&&VehPrototypes[Vehs[unit].proto_id].plan==12&&
                tactical_offense(s.vehicle)&&base<0;
            if(!single_colony){
                if(s.plan==11&&enemy==s.faction)goto tactical_finish_score;
                bool handled=false;
                if(s.plan==11&&stack_check(unit,2,11,-1,-1)&&
                    (!(treaty&1u)||!(treaty&0x2000000u))){
                    combat=1;
                    int odds=battle_fight(s.vehicle,pos-1,0,0,&unit);
                    if(whose_territory(s.faction,x,y,0,0)!=s.faction){
                        if(base<0&&(treaty&7u))odds/=2;
                    } else odds*=2;
                    if(odds>=8){score+=odds;handled=true;}
                    else if(base<0)continue;
                    else handled=true;
                }
                if(!handled&&enemy!=s.faction&&!(treaty&1u)){
                    if(s.low_altitude&&!low&&s.triad==1&&!can_arty(s.proto,1))continue;
                    const bool protected_probe=stack_check(unit,1,-1,-1,-1)==1&&
                        VehPrototypes[Vehs[unit].proto_id].plan==11&&
                        (treaty&6u)&&whose_territory(s.faction,Vehs[unit].x,Vehs[unit].y,0,0)==s.faction;
                    if(protected_probe){combat=1;score+=99;}
                    else {
                        if(s.plan==7&&stack_check(s.vehicle,1,-1,-1,-1)>1)continue;
                        if(!(s.plan<8||s.plan==11))continue;
                        if(s.low_altitude&&!s.triad&&!has_abil(acting.proto_id,8))continue;
                        const int artillery=stack_check(unit,3,2,-1,-1);
                        if(artillery&&!has_abil(acting.proto_id,0x20)&&base<0&&
                            !(map_loc(x,y)->bit&0x40000u)&&!stack_check(unit,6,0x80,-1,-1))continue;
                        if(s.plan!=11){
                            if(tactical_offense(s.vehicle)&&(treaty&0x810u)){
                                combat=1;
                                int odds=battle_fight(s.vehicle,pos-1,0,0,&unit);
                                if(s.priority|| (veh_moves(s.vehicle)<(int)Rules.move_rate_roads&&s.proto!=8)||
                                    (!s.faction_distance&&s.plan>=1)){
                                    const int reverse=battle_fight(unit,(pos+3)&7,0,0,0);
                                    if(reverse){const int damage=s.priority?s.priority+1:0;
                                        if((unsigned)(4-damage)*reverse<(unsigned)odds*4u||reverse<=7||odds<s.priority+6)continue;}
                                    else if(stack_check(s.vehicle,1,-1,-1,-1)==1&&
                                        VehPrototypes[Vehs[unit].proto_id].plan!=11)continue;
                                }
                                odds=compute_odds(odds,s.faction,s.vehicle,unit,base);
                                if(!s.local70&&(s.faction_base<0||!Bases[s.faction_base].eco_damage)&&
                                    Vehs[unit].order_auto_type!=s.faction){
                                    odds*=player.soc_effect_active.planet>=3?8:2;}
                                if(odds>=8){score+=odds;handled=true;}
                            }
                        } else if(stack_check(unit,1,-1,-1,-1)<=1&&
                            base_who(Vehs[unit].x,Vehs[unit].y)<0){
                            const int before=VehCurrentCount;
                            if(probe(s.vehicle,-1,unit,1)){
                                if(before!=VehCurrentCount)return 1;
                                best=999;s.direction=pos-1;combat_choice=1;
                            }
                        }
                        if(!handled)continue;
                    }
                }
            } else {
                char intercept_text[]="ENEMYMISC";
                if(!(treaty&0x20u)&&whose_territory(s.faction,x,y,0,0)!=s.faction&&
                    !rnd(2,intercept_text))continue;
                combat=1;score+=8;
            }
        }
tactical_finish_score:
        if((s.aggressive||s.explore_bonus)&&pos&&
            (s.triad==2||low==(s.triad==1))){
            if(s.explore_bonus){base_find(x,y,s.faction);score+=BaseFindDist*2;}
            for(int step=0;step<8;++step){
                const int rx=xrange(x+RadiusOffsetX[pos]*step);
                const int ry=y+RadiusOffsetY[pos]*step;
                if(!on_map(rx,ry)||(s.triad!=2&&tactical_low(rx,ry)!=(s.triad==1)))continue;
                for(int n=0;n<8;++n){
                    const int nx=xrange(rx+RadiusBaseX[n]);const int ny=ry+RadiusBaseY[n];
                    if(!on_map(nx,ny))continue;
                    const Map &tile=*map_loc(nx,ny);
                    if((player.flags&0x20000000u)&&!(tile.bit&0x100000u)){
                        score+=(tile.bit&0x20u)&&(tile.climate&0xe0)>=0x40?16:8;
                        if(tile.bit&0x2000u)score+=8;}
                    if(!is_known(nx,ny,s.faction)){score+=s.explore_bonus*2+2;
                        if(s.triad==1&&(tile.climate&0xe0)>=0x60&&!player.region_total_bases[tile.region])score+=8;}
                    if(goody_at(nx,ny))score+=s.explore_bonus*2+2;
                    if(s.plan==8){const int territory=whose_territory(s.faction,nx,ny,0,0);
                        s.local70=territory;
                        if(territory>=0&&territory!=s.faction&&(player.diplo_treaties[territory]&3u))score-=6;}
                }
            }
        }
        if(best<score){s.direction=pos-1;best=score;combat_choice=combat;}
    }
    return enemy_move_tactical_postscan(s,combat_choice);
}

//578AE1 is a loop increment, NOT a whole-function dispatch fallback. Every
//surviving scan ends here with the original winner and combat flag live.
static int enemy_move_tactical_postscan(TacticalFrame &s,int combat_choice) {
    Veh &v=Vehs[s.vehicle];
    PlayerData &player=PlayersData[s.faction];
    if(combat_choice){
        if(planet_buster(s.vehicle)){s.direction=-1;v.order=0;}
    } else {
        if(s.route_flag){s.direction=-1;v.order=0;}
        if(s.existing_route&&v.order==0x18)return enemy_move_tactical_action_tail(s);
        if(can_arty(s.proto,1)&&!v.order&&
            (s.triad==1 || !tactical_low(s.x,s.y) || !s.unfiltered_distance)){
            for(int pos=0;pos<25;++pos){
                const int x=xrange(s.x+RadiusOffsetX[pos]);
                const int y=s.y+RadiusOffsetY[pos];
                if(!on_map(x,y)||tactical_low(x,y)||anything_at(x,y)>=0)continue;
                const int owner=whose_territory(s.faction,x,y,0,0);
                if(owner<0||owner==s.faction||!(player.diplo_treaties[owner]&0x10u))continue;
                const int region=map_loc(x,y)->region;
                if(player.region_total_bases[region]&&player.region_base_plan[region]!=2&&!s.triad)continue;
                const unsigned bits=map_loc(s.x,s.y)->bit;
                if(!(bits&0x1000u)||!(bits&0x81208854u))continue;
                if(!s.network){
                    char text[]="Enemy doing ARTY DESTROY 2";
                    log_say(text,s.vehicle,x,y);
                    action_destroy(s.vehicle,0,x,y);
                    return 1;
                }
                char text[]="Auto sending DESTROY2 message";
                log_say(text,s.vehicle,x,y);
                message_data(0x2413,s.vehicle,x,y,0,0);
                NetDaemonNet.await_exec(1);
                return 1;
            }
        }
    }
    v.move_to_ai_type='x';
    if(s.direction>=0){
        const int x=xrange(s.x+RadiusBaseX[s.direction]);
        const int y=s.y+RadiusBaseY[s.direction];
        v.order=0x58;v.waypoint_x[0]=(short)x;v.waypoint_y[0]=(short)y;
    } else {
        //578B34 falls through here after the absent direction.
        v.state&=~0x10000u;
        v.order=0;
    }
    return enemy_move_tactical_action_tail(s);
}

// Complete577361..577443 bridge. Include aftertactical_tail.cpp.
// Returns1 for direct578F46,0 for tactical577443. Does not execute either.
static void enemy_move_prepare_empty_region_goal(TacticalFrame &s,bool skip_triad_gate=false){
    const PlayerData &player=PlayersData[s.faction];
    if((!skip_triad_gate&&s.triad)||s.plan==8||player.region_total_bases[s.region]||
        (player.unk_77[s.region]&6u)||player.region_base_plan[s.region]<3||
        ((s.y<1||s.y>=MapLatitudeBounds-1)&&s.plan!=12))return;
    for(int n=0;n<8;++n){
        const int x=xrange(s.x+RadiusBaseX[n]);const int y=s.y+RadiusBaseY[n];
        if(on_map(x,y)&&(map_loc(x,y)->climate&0xe0)<0x60){
            add_goal(s.faction,7,1,x,y,-1);s.center_bias=1;return;
        }
    }
}
static int enemy_move_pre_tactical(TacticalFrame &s){
    Veh &v=Vehs[s.vehicle];
    if(v.order==0x18&&!s.local84&&!s.route_flag){
        if(!zoc_veh(s.x,s.y,s.faction))return 1;
        const int distance=abs(s.y-v.waypoint_y[0])+x_dist(s.x,v.waypoint_x[0]);
        if((distance&~1)==2){
            s.local70=anything_at(v.waypoint_x[0],v.waypoint_y[0]);
            if(s.local70>=0&&s.local70!=s.faction&&
                !(PlayersData[s.faction].diplo_treaties[s.local70]&1u)&&
                v.move_to_ai_type!='p')v.order=0;
        }
    }
    if(v.order==0x18)s.existing_route=1;
    return 0;
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
struct StrategicActionState {
    int &vehicle;
    int &faction;
    int &x;
    int &y;
    int &triad;
    int &region;
    int &faction_base;
    int &faction_distance;
    int &unfiltered_distance;
    int &low_altitude;
    int &local84;
    int &special_mode;
    int &altitude_or_scratch;
    unsigned int &faction_mask;
    int &movement_zone;
    int &sensor_routed;
    int &recommendation;
    int &chosen_base;
    int &scratch58;
    int &radius_direction;
    int &score;
    int &best;
    unsigned int &mask;
    int &stack_vehicle;
    int &scan_x;
    int &scan_y;
    explicit StrategicActionState(EnemyMoveFullFrame &f)
        : vehicle(f.vehicle_id),
          faction(f.local_c),
          x(f.local_18),
          y(f.local_14),
          triad(f.local_40),
          region(f.local_34),
          faction_base(f.local_5c),
          faction_distance(f.local_64),
          unfiltered_distance(f.local_7c),
          low_altitude(f.local_74),
          local84(f.local_84),
          special_mode(f.local_30),
          altitude_or_scratch(f.local_a4),
          faction_mask(reinterpret_cast<unsigned int&>(f.local_68)),
          movement_zone(f.local_bc),
          sensor_routed(f.local_4c),
          recommendation(f.local_44),
          chosen_base(f.local_24),
          scratch58(f.local_58),
          radius_direction(f.local_54),
          score(f.local_38),
          best(f.local_28),
          mask(f.local_2c),
          stack_vehicle(f.local_1c),
          scan_x(f.local_4),
          scan_y(f.local_8) {}
};
static bool strategic_former_assigned(StrategicActionState &s,int x,int y){
    for(s.stack_vehicle=0;s.stack_vehicle<VehCurrentCount;++s.stack_vehicle){
        const Veh &v=Vehs[s.stack_vehicle];
        if(v.faction_id!=s.faction||VehPrototypes[v.proto_id].plan!=9||
           s.stack_vehicle==s.vehicle)continue;
        if((v.x==x&&v.y==y)||(v.order==0x18&&
            v.waypoint_x[0]==x&&v.waypoint_y[0]==y))return true;
    }
    return false;
}
static StrategicActionExit enemy_move_strategic_actions(StrategicActionState &s){
    Veh &v=Vehs[s.vehicle];PlayerData &player=PlayersData[s.faction];
    s.sensor_routed=0; //573032, not an inferred default.
    if(!player.current_num_bases){
        for(s.stack_vehicle=0;s.stack_vehicle<VehCurrentCount;++s.stack_vehicle){
            const Veh &colony=Vehs[s.stack_vehicle];
            if(colony.faction_id==s.faction&&VehPrototypes[colony.proto_id].plan==8){
                go_to(s.vehicle,'#',colony.x,colony.y);
                return STRATEGIC_JOIN_57707F;
            }
        }
    }
    const int at_former_goal=at_goal(s.faction,9,s.x,s.y);
    s.recommendation=can_terraform(s.faction,s.x,s.y,
        at_former_goal||s.special_mode,s.faction_base);
    if(!s.local84&&(s.triad==2||s.low_altitude==(s.triad==1))&&s.faction_distance){
        del_site(s.faction,s.low_altitude?25:9,s.x,s.y,0);
        if(at_goal(s.faction,0x79,s.x,s.y)&&!is_sensor(s.x,s.y)){
            v.order=13;return STRATEGIC_JOIN_578F46;
        }
        if(at_goal(s.faction,0x29,s.x,s.y)&&!s.special_mode&&s.altitude_or_scratch<=5){
            // Native keeps the range upper bound on the stack across the
            // THREE-argument terraform_cost call; Kuna infers five args.
            const int cost=range(terraform_cost(s.x,s.y,s.faction),0,player.energy_reserves/4);
            player.energy_reserves=range(player.energy_reserves-cost,0,99999);
            v.order=20;del_site(s.faction,8,s.x,s.y,1);
            return STRATEGIC_JOIN_578F46;
        }
        if(at_goal(s.faction,0x49,s.x,s.y)&&terrain_avail(12,s.low_altitude,s.faction)){
            v.order=16;return STRATEGIC_JOIN_578F46;}
        if(at_goal(s.faction,13,s.x,s.y)&&terrain_avail(13,s.low_altitude,s.faction)){
            v.order=17;return STRATEGIC_JOIN_578F46;}
        if(at_goal(s.faction,0x69,s.x,s.y)&&terrain_avail(14,s.low_altitude,s.faction)){
            v.order=18;return STRATEGIC_JOIN_578F46;}
        if(!s.low_altitude&&!s.special_mode&&
            !((unsigned char)s.faction_mask&LockEnableMask)&&terrain_avail(16,0,s.faction)&&
            ((int)Continents[s.region].tile_count<=49||
             ((int)Continents[s.region].tile_count<=149&&BaseFindDist>=4))&&
            s.unfiltered_distance>=2&&(map_loc(s.x,s.y)->climate&0xe0)<=0xbf){
            bool high=false;
            for(int pos=0;pos<49;++pos){
                s.scan_x=xrange(s.x+RadiusOffsetX[pos]);s.scan_y=s.y+RadiusOffsetY[pos];
                if(on_map(s.scan_x,s.scan_y)&&(map_loc(s.scan_x,s.scan_y)->climate&0xe0)>=0xc0){high=true;break;}}
            if(!high){v.order=20;return STRATEGIC_JOIN_578F46;}
        }
        if(s.faction_distance>=3)s.radius_direction=0;
        else {
            const Map &tile=*map_loc(s.x,s.y);const unsigned bits=tile.bit;
            if((bits&0x10u||bits&0x200000u)&&!(bits&4u)&&
                (!(bits&0x20u)||(tile.climate&0xe0)<=0x3f)&&!s.low_altitude&&
                (!((unsigned char)s.faction_mask&LockEnableMask)||!(GameMorePreferences&0x400000u))){
                v.order=9;return STRATEGIC_JOIN_578F46;}
            s.radius_direction=radius_move(Bases[s.faction_base].x,Bases[s.faction_base].y,s.x,s.y,21);
        }
        const int former_goal=at_goal(s.faction,s.low_altitude?25:9,s.x,s.y);
        s.mask=former_goal||(v.waypoint_x[0]==s.x&&v.waypoint_y[0]==s.y)||
            stack_check(s.vehicle,9,3,-1,-1)||
            (s.radius_direction&&(Bases[s.faction_base].resource_sqr_radius&
                (1u<<(s.radius_direction&31))));
        const Map &tile=*map_loc(s.x,s.y);
        if((tile.bit&0x20u)&&(tile.climate&0xe0)>=0x40&&s.recommendation&&
            (has_abil(v.proto_id,0x2000)||has_tech((map_loc(s.x,s.y)->climate&0xe0)<0x60?
                Terraforming[10].preq_tech_sea:Terraforming[10].preq_tech,s.faction))&&
            (s.mask||(s.faction_distance<=2&&(Bases[s.faction_base].eco_damage||
                Bases[s.faction_base].population_size>=6)))){
            v.order=14;return STRATEGIC_JOIN_578F46;
        }
        // Native573620 tests resource word NONZERO and then tests the
        // shifted mask itself, not resource_word & shifted_mask.
        if(s.recommendation==4&&(s.mask||(s.radius_direction&&
            Bases[s.faction_base].resource_sqr_radius&&
            (1u<<(s.radius_direction&31))))){
            v.order=4;v.move_to_ai_type='+';return STRATEGIC_JOIN_578F46;
        }
        if(!(s.low_altitude||(((unsigned char)s.faction_mask&LockEnableMask)&&
                            (GameMorePreferences&0x400000u)))){
            bool roads=s.mask!=0;
            if(!roads&&s.faction_distance<=2){
                const unsigned bits=map_loc(s.x,s.y)->bit;
                if(bits&0x10u||bits&0x8000u)roads=true;
                else if(bits&0x200000u||s.recommendation){
                    roads=!(bits&0x20u)||(map_loc(s.x,s.y)->climate&0xe0)<=0x3f;
                }
            }
            if(roads){
                const unsigned bits=map_loc(s.x,s.y)->bit;
                if(bits&4u){
                    if(has_tech(Terraforming[6].preq_tech,s.faction)&&s.mask&&!(bits&8u)){
                        v.order=9;v.move_to_ai_type='R';return STRATEGIC_JOIN_578F46;}
                } else if(!(bits&0x20u)||(map_loc(s.x,s.y)->climate&0xe0)<=0x3f||
                    has_tech(Rules.tech_improve_fungus_sqr,s.faction)){
                    v.order=9;v.move_to_ai_type='r';return STRATEGIC_JOIN_578F46;
                }
            }
        }
        if(s.recommendation&&(s.faction_distance<=2||stack_check(s.vehicle,9,3,-1,-1))&&
            (s.mask||!v.order)){
            v.order=(signed char)s.recommendation;v.move_to_ai_type='+';
            return STRATEGIC_JOIN_578F46;
        }
        if(!((unsigned char)s.faction_mask&LockEnableMask)||(GameMorePreferences&0x20u)){
            if(s.mask||!v.order){
                if(is_sensor(s.x,s.y)&&whose_territory(s.faction,s.x,s.y,0,0)==s.faction&&
                    s.faction_distance>=2&&!(map_loc(s.x,s.y)->bit&0x800u)&&
                    has_tech(Terraforming[7].preq_tech,s.faction)){
                    v.order=11;return STRATEGIC_JOIN_578F46;}
                if(good_sensor(s.faction,s.x,s.y)){v.order=13;return STRATEGIC_JOIN_578F46;}
                if(s.faction_distance>=2&&s.faction_distance<=3&&
                    (map_loc(s.x,s.y)->bit&4u)&&!(map_loc(s.x,s.y)->bit&0x800u)&&
                    has_tech(Terraforming[7].preq_tech,s.faction)){
                    bool bunker=false;
                    for(int pos=0;pos<9;++pos){
                        s.scan_x=xrange(s.x+RadiusOffsetX[pos]);s.scan_y=s.y+RadiusOffsetY[pos];
                        if(on_map(s.scan_x,s.scan_y)&&(map_loc(s.scan_x,s.scan_y)->bit&0x800u)){bunker=true;break;}}
                    if(!bunker){v.order=11;return STRATEGIC_JOIN_578F46;}
                }
                if(s.mask&&has_tech(Terraforming[9].preq_tech,s.faction)){
                    s.scan_x=s.x;s.scan_y=s.y;s.sensor_routed=1;
                    if(!Paths.sensors(s.faction,&s.scan_x,&s.scan_y)&&s.scan_x==s.x&&s.scan_y==s.y){
                        v.order=13;return STRATEGIC_JOIN_578F46;}
                }
            }
        }
    }
    //5739AA: unguarded home/base accesses are original, including base-1.
    if(v.order!=0x18&&!s.local84){
        if(s.special_mode){s.chosen_base=v.home_base_id;
            s.altitude_or_scratch=Bases[s.chosen_base].x;s.scratch58=Bases[s.chosen_base].y;
        } else {
            s.chosen_base=s.faction_base;
            if(v.state&0x200u){s.altitude_or_scratch=Bases[s.faction_base].x;
                s.scratch58=Bases[s.faction_base].y;}
            else {s.altitude_or_scratch=s.x;s.scratch58=s.y;}
            int current_formers=0;
            for(int pos=1;pos<=20;++pos){
                s.scan_x=xrange(Bases[s.faction_base].x+RadiusOffsetX[pos]);
                s.scan_y=Bases[s.faction_base].y+RadiusOffsetY[pos];
                if(!on_map(s.scan_x,s.scan_y)||map_loc(s.scan_x,s.scan_y)->region!=s.region)continue;
                s.stack_vehicle=veh_at(s.scan_x,s.scan_y);
                if(s.stack_vehicle>=0&&Vehs[s.stack_vehicle].faction_id==s.faction){
                    current_formers+=stack_check(s.stack_vehicle,2,9,-1,-1);
                    if(s.scan_x==s.x&&s.scan_y==s.y)--current_formers;}
            }
            if(current_formers){
                s.best=9999;s.radius_direction=0;
                for(s.radius_direction=0;s.radius_direction<BaseCurrentCount;++s.radius_direction){
                    const Base &b=Bases[s.radius_direction];
                    if(b.faction_id_current!=s.faction||map_loc(b.x,b.y)->region!=s.region)continue;
                    int formers=0;
                    for(int pos=0;pos<25;++pos){
                        const int x=xrange(b.x+RadiusOffsetX[pos]);const int y=b.y+RadiusOffsetY[pos];
                        if(!on_map(x,y)||map_loc(x,y)->region!=s.region)continue;
                        s.stack_vehicle=veh_at(x,y);
                        if(s.stack_vehicle>=0)formers+=stack_check(s.stack_vehicle,2,9,-1,-1);
                    }
                    if(formers<current_formers){
                        s.score=vector_dist(x_dist(s.x,b.x),abs(s.y-b.y))*(formers+1)*(formers+1);
                        if(s.score<s.best){s.chosen_base=s.radius_direction;s.best=s.score;}
                    }
                }
            }
        }
        if(s.chosen_base>=0){
            s.best=9999;
            for(s.mask=1;s.mask<21;++s.mask){
                const int pos=(TurnCurrentNum+(int)s.mask)%21;
                s.scan_x=xrange(s.altitude_or_scratch+RadiusOffsetX[pos]);
                s.scan_y=s.scratch58+RadiusOffsetY[pos];
                if(!on_map(s.scan_x,s.scan_y)||base_who(s.scan_x,s.scan_y)>=0)continue;
                const Map &tile=*map_loc(s.scan_x,s.scan_y);
                const int low=(tile.climate&0xe0)<0x60;
                if(low!=(s.triad==1)||(tile.region!=s.region&&low==s.low_altitude)||
                    zoc_move(s.scan_x,s.scan_y,s.faction))continue;
                s.stack_vehicle=veh_at(s.scan_x,s.scan_y);
                if(s.stack_vehicle>=0&&(Vehs[s.stack_vehicle].faction_id!=s.faction||
                    stack_check(s.stack_vehicle,2,9,-1,-1)))continue;
                const int flag=((unsigned char)s.faction_mask&LockEnableMask)&&s.special_mode;
                if(!can_terraform(s.faction,s.scan_x,s.scan_y,flag,s.chosen_base))continue;
                s.score=vector_dist(x_dist(Bases[s.chosen_base].x,s.scan_x),abs(Bases[s.chosen_base].y-s.scan_y));
                if(s.score>2)continue;
                if(!(Bases[s.chosen_base].resource_sqr_radius&(1u<<(s.mask&31)))&&
                    (!(tile.bit&0x20u)||(tile.climate&0xe0)<=0x3f))s.score*=3;
                if(s.score>=s.best||strategic_former_assigned(s,s.scan_x,s.scan_y))continue;
                s.best=s.score;go_to(s.vehicle,'w',s.scan_x,s.scan_y);
            }
            if(v.order==0x18)return STRATEGIC_JOIN_57707F;
        }
        if(!s.sensor_routed&&(!((unsigned char)s.faction_mask&LockEnableMask)||
                             (GameMorePreferences&0x20u))){
            s.scan_x=s.x;s.scan_y=s.y;
            if(!Paths.sensors(s.faction,&s.scan_x,&s.scan_y)){
                if(s.scan_x==s.x&&s.scan_y==s.y){v.order=13;return STRATEGIC_JOIN_578F46;}
                const int distance=vector_dist(x_dist(s.x,s.scan_x),abs(s.y-s.scan_y));
                if(!(s.special_mode&&distance>2)&&!strategic_former_assigned(s,s.scan_x,s.scan_y)){
                    go_to(s.vehicle,'s',s.scan_x,s.scan_y);return STRATEGIC_JOIN_57707F;}
            }
        }
        if(s.faction_distance&&!s.low_altitude&&player.region_total_bases[s.region]){
            s.stack_vehicle=v.home_base_id>=0?v.home_base_id:s.faction_base;
            if(s.stack_vehicle>=0){
                const Base &home=Bases[s.stack_vehicle];
                if(map_loc(home.x,home.y)->region!=s.region)s.stack_vehicle=s.faction_base;
                if(s.stack_vehicle>=0){const Base &b=Bases[s.stack_vehicle];
                    const int distance=vector_dist(x_dist(s.x,b.x),abs(s.y-b.y));
                    if(distance>(TurnCurrentNum<=50)+3||s.movement_zone){
                        go_to(s.vehicle,'b',b.x,b.y);return STRATEGIC_JOIN_57707F;}
                }
            }
        }
        if(s.special_mode&&!v.order){v.state&=~0x200u;v.order=0;return STRATEGIC_RETURN_ZERO;}
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
    offsetof(PlayerData,soc_effect_pending)+offsetof(SocialEffect,planet)==0x268 ? 1 : -1];
#endif
enum NonseaExit {
    NONSEA_JOIN_578B26, NONSEA_JOIN_57707F,
    NONSEA_JOIN_577071, NONSEA_JOIN_574A88
};
enum NonseaEntry { NONSEA_ENTRY_5741CF, NONSEA_ENTRY_57460A, NONSEA_ENTRY_574A7E };
// Inputs and shared-frame outputs. No constructor/defaults: untouched slots
// retain their predecessor values, including unreached/uninitialized scratch.
struct NonseaState {
    int &vehicle;
    int &faction;
    int &x;
    int &y;
    int &plan;
    int &triad;
    int &region;
    int &region_plan;
    int &low_altitude;
    int &explore_bonus;
    int &faction_base;
    int &direction;
    int &available;
    int &altitude_or_scratch;
    int &owner;
    int &previous_ring_best;
    int &scratch58;
    int &ring;
    int &scratch4c;
    int &ring_best;
    int &score;
    int &mode;
    unsigned int &radius_index;
    int &best;
    int &scratch20;
    int &stack_vehicle;
    int &scan_x;
    int &scan_y;
    explicit NonseaState(EnemyMoveFullFrame &f)
        : vehicle(f.vehicle_id),
          faction(f.local_c),
          x(f.local_18),
          y(f.local_14),
          plan(f.local_50),
          triad(f.local_40),
          region(f.local_34),
          region_plan(f.local_88),
          low_altitude(f.local_74),
          explore_bonus(f.local_b0),
          faction_base(f.local_5c),
          direction(f.local_80),
          available(f.local_6c),
          altitude_or_scratch(f.local_a4),
          owner(f.local_70),
          previous_ring_best(f.local_60),
          scratch58(f.local_58),
          ring(f.local_54),
          scratch4c(f.local_4c),
          ring_best(f.local_44),
          score(f.local_38),
          mode(f.local_30),
          radius_index(f.local_2c),
          best(f.local_28),
          scratch20(f.local_20),
          stack_vehicle(f.local_1c),
          scan_x(f.local_4),
          scan_y(f.local_8) {}
};

static NonseaExit enemy_move_nonsea(NonseaState &s,
    NonseaEntry entry = NONSEA_ENTRY_5741CF) {
    if (entry == NONSEA_ENTRY_5741CF && s.region_plan >= 3 &&
        (int)Continents[s.region].tile_count <= (int)PlayersData[s.faction].unk_79[s.region] &&
        s.plan <= 3 && !s.triad && Vehs[s.vehicle].order != 0x18 &&
        !s.low_altitude) {
        if (coast_or_border(s.x,s.y,s.x,s.y,s.faction)) {
            int direction = (Vehs[s.vehicle].facing + 4) % 8;
            s.scan_x = xrange(s.x + RadiusBaseX[direction]);
            s.scan_y = s.y + RadiusBaseY[direction];
            if (!coast_or_border(s.scan_x,s.scan_y,s.scan_x,s.scan_y,s.faction)) {
                if (s.vehicle & 1) Vehs[s.vehicle].state |= 0x80000000u;
                else Vehs[s.vehicle].state &= 0x7fffffffu;
            }
            s.mode = 0;
            s.available = 0;
            s.scratch4c = 3;
            do {
                int rotation = s.scratch4c;
                if (Vehs[s.vehicle].state & 0x80000000u)
                    rotation = s.available - 3;
                direction = (rotation + Vehs[s.vehicle].facing + 8) % 8;
                s.scan_x = xrange(s.x + RadiusBaseX[direction]);
                s.scan_y = s.y + RadiusBaseY[direction];
                if (s.mode) {
                    if (on_map(s.scan_x,s.scan_y) &&
                        map_loc(s.scan_x,s.scan_y)->region == s.region &&
                        whose_territory(s.faction,s.scan_x,s.scan_y,NULL,0) == s.faction) {
                        s.direction = direction;
                        Vehs[s.vehicle].move_to_ai_type = 'p';
                        return NONSEA_JOIN_578B26;
                    }
                } else if (on_map(s.scan_x,s.scan_y)) {
                    if (map_loc(s.scan_x,s.scan_y)->region != s.region ||
                        whose_territory(s.faction,s.scan_x,s.scan_y,NULL,0) != s.faction)
                        s.mode = 1;
                }
                ++s.available;
                --s.scratch4c;
            } while (s.scratch4c > -5);
        }
        s.best = 0;
        s.scan_y = 0;
        while (s.scan_y < (int)MapLatitudeBounds) {
            s.scan_x = s.scan_y & 1;
            while (s.scan_x < (int)MapLongitudeBounds) {
                if (map_loc(s.scan_x,s.scan_y)->region == s.region &&
                    whose_territory(s.faction,s.scan_x,s.scan_y,NULL,0) == s.faction) {
                    s.scratch20 = 1;
                    s.scratch58 = 0;
                    do {
                        const int offset = s.scratch58 / 4;
                        const int xx = xrange(s.scan_x + RadiusOffsetX[offset]);
                        const int yy = s.scan_y + RadiusOffsetY[offset];
                        if (on_map(xx,yy)) {
                            if (map_loc(xx,yy)->region != s.region) ++s.scratch20;
                            else {
                                if (whose_territory(s.faction,xx,yy,NULL,0) != s.faction)
                                    s.scratch20 += 3;
                                if (anything_at(xx,yy) == s.faction) s.scratch20 -= 6;
                            }
                        }
                        s.scratch58 += 4;
                    } while (s.scratch58 < 0x24);
                    int candidate_score = (int)((unsigned int)((abs(s.y-s.scan_y) +
                        x_dist(s.x,s.scan_x)) >> 1) * (unsigned int)s.scratch20);
                    s.stack_vehicle = 0;
                    if (VehCurrentCount > 0) {
                        s.scratch4c = VehCurrentCount;
                        s.stack_vehicle = VehCurrentCount;
                        int vehicle = 0;
                        do {
                            if (Vehs[vehicle].faction_id == s.faction &&
                                Vehs[vehicle].waypoint_x[0] == s.scan_x &&
                                Vehs[vehicle].waypoint_y[0] == s.scan_y)
                                candidate_score /= 2;
                            ++vehicle;
                            --s.scratch4c;
                        } while (s.scratch4c);
                    }
                    if (candidate_score > s.best) {
                        s.best = candidate_score;
                        go_to(s.vehicle,'P',s.scan_x,s.scan_y);
                    }
                }
                s.scan_x += 2;
            }
            ++s.scan_y;
        }
    }
    if (entry == NONSEA_ENTRY_57460A || (entry == NONSEA_ENTRY_5741CF &&
        (s.explore_bonus || (s.plan == 3 && TurnCurrentNum < 100)))) {
        if (s.triad != 2 &&
            (Vehs[s.vehicle].order != 0x18 || Vehs[s.vehicle].move_to_ai_type != '/')) {
            s.scratch4c = 0;
            s.best = 0;
            s.previous_ring_best = 0;
            s.ring = 0;
            do {
                s.ring_best = 0;
                s.radius_index = s.ring ? RadiusRange[s.ring-1] : 0;
                while ((int)s.radius_index < RadiusRange[s.ring]) {
                    s.scan_x = xrange(s.x + RadiusOffsetX[s.radius_index]);
                    s.scan_y = s.y + RadiusOffsetY[s.radius_index];
                    if (on_map(s.scan_x,s.scan_y) &&
                        map_loc(s.scan_x,s.scan_y)->region == s.region) {
                        s.owner = whose_territory(s.faction,s.scan_x,s.scan_y,NULL,0);
                        if (s.owner < 0 || s.owner == s.faction ||
                            !(PlayersData[s.faction].diplo_treaties[s.owner] & 7)) {
                            int candidate_score = is_known(s.scan_x,s.scan_y,s.faction)
                                ? (goody_at(s.scan_x,s.scan_y) ? 512 : 0) : 0;
                            s.mode = 0;
                            do {
                                // Original defect: indexes radius_index, NOT mode.
                                // Thus the same offset tile is examined nine times.
                                const int xx = xrange(s.scan_x + RadiusOffsetX[s.radius_index]);
                                const int yy = s.scan_y + RadiusOffsetY[s.radius_index];
                                if (on_map(xx,yy)) {
                                    if (is_known(xx,yy,s.faction)) {
                                        s.owner = veh_who(xx,yy);
                                        if (s.owner >= 0 && s.owner != s.faction) {
                                            candidate_score = 0;
                                            break;
                                        }
                                    } else candidate_score +=
                                        map_loc(xx,yy)->region == s.region ? 16 : 1;
                                }
                                ++s.mode;
                            } while (s.mode < 9);
                            if ((map_loc(s.scan_x,s.scan_y)->bit & 0x20) &&
                                (map_loc(s.scan_x,s.scan_y)->climate & 0xe0) >= 0x40 &&
                                PlayersData[s.faction].soc_effect_pending.planet <= 0)
                                candidate_score /= 2;
                            s.altitude_or_scratch = candidate_score;
                            if (s.faction_base >= 0) {
                                const Base &base = Bases[s.faction_base];
                                s.scratch58 = abs(s.scan_y-base.y);
                                int base_bonus = 16 - ((s.scratch58 +
                                    x_dist(s.scan_x,base.x)) >> 1);
                                if (base_bonus >= 0) {
                                    // Native repeats both distance calls on this arm.
                                    s.scratch58 = abs(s.scan_y-base.y);
                                    base_bonus = 16 - ((s.scratch58 +
                                        x_dist(s.scan_x,base.x)) >> 1);
                                } else base_bonus = 0;
                                candidate_score += base_bonus;
                            }
                            s.scratch58 = abs(s.y-s.scan_y);
                            s.scratch20 = (int)((unsigned int)candidate_score << 8) /
                                (((s.scratch58 + x_dist(s.x,s.scan_x)) >> 1) + 2);
                            if (s.ring_best < s.scratch20) s.ring_best = s.scratch20;
                            if (s.best < s.scratch20) {
                                s.stack_vehicle = 0;
                                bool claimed = false;
                                while (s.stack_vehicle < VehCurrentCount) {
                                    const Veh &vehicle = Vehs[s.stack_vehicle];
                                    if (vehicle.faction_id == s.faction &&
                                        s.stack_vehicle != s.vehicle && vehicle.order == 0x18) {
                                        s.score = (abs(s.scan_y-vehicle.waypoint_y[0]) +
                                            x_dist(s.scan_x,vehicle.waypoint_x[0])) >> 1;
                                        if (s.score <= 1) {
                                            const int acting_distance = (abs(s.y-s.scan_y) +
                                                x_dist(s.x,s.scan_x)) >> 1;
                                            s.scratch58 = abs(vehicle.y-s.scan_y);
                                            if (((s.scratch58 + x_dist(vehicle.x,s.scan_x)) >> 1) - 1
                                                <= acting_distance) {
                                                claimed = true;
                                                break;
                                            }
                                        }
                                    }
                                    ++s.stack_vehicle;
                                }
                                if (!claimed && s.scratch20) {
                                    s.scratch4c = s.altitude_or_scratch;
                                    s.best = s.scratch20;
                                    go_to(s.vehicle,'/',s.scan_x,s.scan_y);
                                }
                            }
                        }
                    }
                    ++s.radius_index;
                }
                if (Vehs[s.vehicle].order == 0x18 &&
                    s.ring_best < s.previous_ring_best && s.scratch4c >= 16) break;
                s.previous_ring_best = s.ring_best;
                ++s.ring;
            } while (s.ring < 9);
            if (Vehs[s.vehicle].order == 0x18) return NONSEA_JOIN_57707F;
        }
    }
    return s.triad == 1 ? NONSEA_JOIN_574A88 : NONSEA_JOIN_577071;
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
            f.local_4 = xrange(f.local_18 + RadiusOffsetX[offset]);
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
            while (f.local_24 < BaseCurrentCount) {
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
        while (f.local_24 < BaseCurrentCount) {
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
            f.local_4 = xrange(target.x + RadiusOffsetX[offset]);
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
            f.local_4 = xrange(f.local_18 + RadiusBaseX[f.local_2c]);
            f.local_8 = f.local_14 + RadiusBaseY[f.local_2c];
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
        f.local_4 = xrange(f.local_18 + RadiusBaseX[f.local_2c]);
        f.local_8 = f.local_14 + RadiusBaseY[f.local_2c];
        if (on_map(f.local_4,f.local_8) &&
            (map_loc(f.local_4,f.local_8)->climate & 0xe0) >= 0x60) {
            const int dy = abs(f.local_8-target.y);
            if (((dy + x_dist(f.local_4,target.x)) & ~1) == 2) {
                f.local_20 = (int)((unsigned int)defense_value(f.local_c,f.local_4,f.local_8,actor,-1) << 3);
                for (int direction = 0; direction < 8; ++direction) {
                    const int xx = xrange(f.local_4 + RadiusBaseX[direction]);
                    const int yy = f.local_8 + RadiusBaseY[direction];
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
        const int x = xrange(s.local_18 + RadiusOffsetX[offset]);
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
        const int x = xrange(f.local_18 + RadiusBaseX[direction]);
        const int y = f.local_14 + RadiusBaseY[direction];
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
// ORIGINAL: 0x0056B5B0 ?enemy_move@@YAHH@Z 0x0056B5B0-0x00579208 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:9d6ac074ba331b079a584bc73bfd0ce20699a9b4:src/unrecovered/0056b5b0.cpp is deleted seed history; its original identity, full span, claims and useful lessons are retained here through the source writer.", "Pinned instructions establish distinct raw route/damage flags, integer stack priority, saved versus live callback reads, and original unguarded/uninitialized error paths.", "Complete selected, strategic, nonsea, sea and tactical continuations execute once; helpers do not introduce new original identities.", "Vehicle, prototype, PlayerData and Base fields use their real source owners; original matching and full runtime restoration obligations remain."], "reason": "Complete typed per-vehicle enemy movement owner with one path-dependent raw frame and genuine product owners; superseded executable forwarding and scaffold are removed together."}
// kind game
int __cdecl enemy_move(int incoming_vehicle_id) {
    EnemyMoveFullFrame f;
    TacticalFrame tactical(f);
    SeaScoringState sea(f);
    StrategicActionState strategic(f);
    NonseaState nonsea(f);
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
        !(static_cast<unsigned char>(LockEnableMask) & (1u << (f.local_c & 31)))) {
        veh_skip(f.vehicle_id);
        return 0;
    }
    if (!veh_moves(f.vehicle_id)) {
        f.local_b8 = 1;
        goto label_574a7e;
    }
    if (Vehs[f.vehicle_id].order > 3 && Vehs[f.vehicle_id].order < 24) {
        if (!f.local_94 || (f.local_94 >= 2 &&
            (!(static_cast<unsigned char>(LockEnableMask) & (1u << (f.local_c & 31))) ||
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
        !f.local_94 && !(static_cast<unsigned char>(LockEnableMask) &
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
        const int tech = f.local_40 == 1 ? remove.preq_tech_sea : remove.preq_tech;
        if (!has_tech_call(tech,f.local_c) &&
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
        if (BaseCurrentCount > 0) {
            int distance_scratch = f.local_38; // original uninitialized reaching read
            f.base_cursor_30 = &Bases[0].x;
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
                f.base_cursor_30 += sizeof(Base)/sizeof(short);
            } while (f.local_24 < BaseCurrentCount);
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
    if (BaseCurrentCount > 0) {
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
                        has_tech(Terraforming[TERRA_REMOVE_FUNGUS].preq_tech,f.local_c))
                        road_mode = 0;
                    f.local_80 = Paths.find(path_x,path_y,base_x,f.local_58,0,-1,road_mode,-1);
                    if (f.local_80 < 0) break;
                    path_x = xrange(path_x+RadiusBaseX[f.local_80]);
                    path_y += RadiusBaseY[f.local_80];
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
        } while (f.local_24 < BaseCurrentCount);
        if (f.local_78 >= 0) {
            Vehs[f.vehicle_id].move_to_ai_type = 0x52;
            Vehs[f.vehicle_id].order = static_cast<unsigned char>(f.local_30);
            Vehs[f.vehicle_id].waypoint_x[0] = Bases[f.local_78].x;
            Vehs[f.vehicle_id].waypoint_y[0] = Bases[f.local_78].y;
            goto label_578f46;
        }
    }
    if (!(static_cast<unsigned char>(LockEnableMask) & static_cast<unsigned char>(f.local_68)) &&
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
        const int x = xrange(f.local_18 + RadiusBaseX[direction]);
        const int y = f.local_14 + RadiusBaseY[direction];
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
        const int project_base = SecretProject.universal_translator;
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
        for (; f.local_24 < BaseCurrentCount; ++f.local_24) {
            const int base_id = f.local_24;
            Base &base = Bases[base_id];
            if (base.faction_id_current != f.local_c ||
                map_loc(base.x,base.y)->region != f.local_34) continue;
            int score = vector_dist(x_dist(f.local_18, base.x),
                                    abs(f.local_14 - base.y));
            if (base_id != project_base) {
                const bool skip_penalties = base.queue_production_id[0] < -0x46 &&
                    base.minerals_accumulated >= 0x64 &&
                    (lock_mask & static_cast<uint8_t>(LockEnableMask)) == 0;
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
        (static_cast<unsigned char>(LockEnableMask) &
         static_cast<unsigned char>(f.local_68)) == 0) {
        f.local_28 = 0x270f;
        f.local_78 = -1;
        f.local_24 = 0;
        int best_base = -1;
        int best_distance = 0x270f;
        for (; f.local_24 < BaseCurrentCount; ++f.local_24) {
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
            for (; f.local_1c < VehCurrentCount; ++f.local_1c) {
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
    if ((static_cast<unsigned char>(LockEnableMask) &
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
            for (int base_id = 0; base_id < BaseCurrentCount;
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
     static_cast<unsigned char>(LockEnableMask)) != 0)
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
if (BaseCurrentCount > 0) {
for (; enemy_base_index < BaseCurrentCount; ++enemy_base_index) {
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
    for (f.local_54 = 0; f.local_54 < BaseCurrentCount; ++f.local_54) {
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
if (BaseCurrentCount > 0) {
    f.base_cursor_48 = reinterpret_cast<const short *>(&Bases[0].faction_id_current);
    for (; f.local_24 < BaseCurrentCount;
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
            const int neighbor_x = xrange(f.local_4 + RadiusBaseX[direction]);
            const int neighbor_y = f.local_8 + RadiusBaseY[direction];
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
        while (f.local_1c < VehCurrentCount) {
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
            static_cast<int>(PlayersData[f.local_c].current_num_bases) >= 2) {
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
                for (int vehicle = 0; vehicle < VehCurrentCount; ++vehicle)
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
        if (!f.local_30 || !((TurnCurrentNum+f.local_c) & (f.local_48 ? 15 : 3)) || !f.local_7c) {
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
                    if ((!f.local_30 || !((TurnCurrentNum+f.local_c) & 15)) &&
                        (is_known(f.local_4,f.local_8,f.local_c) ||
                         (DiffLevelCurrent >= 4 &&
                          !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(LockEnableMask)))) &&
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
                                    if ((DiffLevelCurrent >= 4 &&
                                        !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(LockEnableMask))) ||
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
                        (int)PlayersData[f.local_c].current_num_bases >= 2)
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
                            (DiffLevelCurrent >= 4 &&
                             !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(LockEnableMask))))) {
                            const int territory = whose_territory(f.local_c,f.local_4,f.local_8,NULL,0);
                            if (Paths.find(f.local_4,f.local_8,f.local_18,f.local_14,
                                territory,f.local_c,0x201,-1) >= 0) {
                                if (is_known(f.local_4,f.local_8,f.local_c) ||
                                    (DiffLevelCurrent >= 4 &&
                                     !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(LockEnableMask)))) {
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
                        (DiffLevelCurrent >= 4 &&
                         !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(LockEnableMask)))) {
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
        if ((static_cast<unsigned char>(LockEnableMask) & (1u << (owner & 31))) &&
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
    if (TurnCurrentNum >= 100 && !f.local_4c && f.local_64 > 6 &&
        Vehs[f.vehicle_id].order == 24 && Vehs[f.vehicle_id].move_to_ai_type != 'X' &&
        !(Vehs[f.vehicle_id].state & 0x40000000u) && f.local_50 != 3)
        goto label_57137d;
    f.local_54 = 25;
    if (f.local_50 == 3 || f.local_40 == 1) f.local_54 = 49;
    if (f.local_50 == 7 &&
        (stack_check(f.vehicle_id,2,8,-1,-1) || f.local_90 >= 2)) f.local_54 = 9;
    if (TurnCurrentNum >= (f.local_50 == 6 ? 400 : 100) &&
        Vehs[f.vehicle_id].order == 24 && Vehs[f.vehicle_id].move_to_ai_type != 'X' &&
        f.local_50 != 3) f.local_54 = 9;
    f.local_2c = 1;
    if (f.local_54 <= 1) goto label_57137d;
    do {
        f.local_4 = xrange(f.local_18+RadiusOffsetX[f.local_2c]);
        f.local_8 = f.local_14+RadiusOffsetY[f.local_2c];
        if (on_map(f.local_4,f.local_8) && goody_at(f.local_4,f.local_8) &&
            (is_known(f.local_4,f.local_8,f.local_c) ||
             (DiffLevelCurrent >= 4 &&
              !(static_cast<unsigned char>(f.local_68) & static_cast<unsigned char>(LockEnableMask)))) &&
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
    if (!(static_cast<unsigned char>(LockEnableMask) & f.local_68) &&
        !f.local_64 && !f.local_6c && f.local_94 && !is_objective(f.local_5c) &&
        (f.local_88 == 2 || PlayersData[f.local_c].region_total_bases[f.local_34] == 1 ||
         (PlayersData[f.local_c].flags & 0x2000000u))) {
        f.local_70 = Bases[f.local_5c].faction_id_former;
        if (stack_check(f.vehicle_id,1,-1,-1,-1) <= (Bases[f.local_5c].population_size+1)/2 &&
            stack_check(f.vehicle_id,16,-1,-1,-1) < (Vehs[f.vehicle_id].dmg_incurred != 0)+2 &&
            f.local_70 != f.local_c && (static_cast<unsigned char>(LockEnableMask) &
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
        !(static_cast<unsigned char>(LockEnableMask) & static_cast<unsigned char>(f.local_68)))
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
        (static_cast<unsigned char>(LockEnableMask) & static_cast<unsigned char>(f.local_68)) &&
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
                                while (f.local_1c < VehCurrentCount) {
                                    const Veh &other = Vehs[f.local_1c];
                                    if (other.faction_id == f.local_c &&
                                        VehPrototypes[other.proto_id].plan == 9 &&
                                        f.local_1c != f.vehicle_id &&
                                        ((other.x == goal.x && other.y == goal.y) ||
                                         (other.order == 0x18 && other.waypoint_x[0] == goal.x &&
                                          other.waypoint_y[0] == goal.y))) break;
                                    ++f.local_1c;
                                }
                                if (f.local_1c >= VehCurrentCount) {
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
        if (BaseCurrentCount > 0) {
            f.base_cursor_44 = &Bases[0].y;
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
                                    while (f.local_1c < VehCurrentCount) {
                                        const Veh &other = Vehs[f.local_1c];
                                        if (other.faction_id == f.local_c &&
                                            VehPrototypes[other.proto_id].plan == 9 &&
                                            f.local_1c != f.vehicle_id &&
                                            ((other.x == f.local_4 && other.y == f.local_8) ||
                                             (other.order == 0x18 && other.waypoint_x[0] == f.local_4 &&
                                              other.waypoint_y[0] == f.local_8))) break;
                                        ++f.local_1c;
                                    }
                                    if (f.local_1c >= VehCurrentCount && f.local_38 < f.local_28) {
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
                f.base_cursor_44 += sizeof(Base)/sizeof(short);
            } while (f.local_24 < BaseCurrentCount);
        }
    }

    {
        const StrategicActionExit result = enemy_move_strategic_actions(strategic);
        if (result == STRATEGIC_RETURN_ZERO) return 0;
        if (result == STRATEGIC_JOIN_57707F) goto label_57707f;
        if (result == STRATEGIC_JOIN_578F46) goto label_578f46;
    }
label_5741cf:
    {
        const NonseaExit exit = enemy_move_nonsea(nonsea);
        if (exit == NONSEA_JOIN_578B26) goto label_578b26;
        if (exit == NONSEA_JOIN_57707F) goto label_57707f;
        if (exit == NONSEA_JOIN_577071) goto label_577071;
        goto label_574a88;
    }
label_57460a:
    {
        const NonseaExit exit = enemy_move_nonsea(nonsea,NONSEA_ENTRY_57460A);
        if (exit == NONSEA_JOIN_578B26) goto label_578b26;
        if (exit == NONSEA_JOIN_57707F) goto label_57707f;
        if (exit == NONSEA_JOIN_577071) goto label_577071;
        goto label_574a88;
    }
label_574a7e:
    if (f.local_40 != 1) goto label_577071;
label_574a88:
    if (f.local_50 == 6) del_site(f.local_c,6,f.local_18,f.local_14,0);
    if (f.local_50 == 7) sea_exit = enemy_move_sea_scoring(sea);
    else {
        f.local_6c = 0; f.local_44 = 0; f.local_54 = 0;
        f.local_30 = 0; f.local_2c = 0;
        if (f.local_8c) goto label_577071;
        sea_exit = enemy_move_57666a_tail(sea);
    }
    if (sea_exit == SEA_JOIN_576DAF || sea_exit == SEA_JOIN_576835 || sea_exit == SEA_JOIN_57681D)
        sea_exit = enemy_move_sea_after_scoring(sea,sea_exit);
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
    enemy_move_prepare_empty_region_goal(tactical,true);
label_57717f:
    if (Vehs[f.vehicle_id].order == 24 &&
        (!Weapon[VehPrototypes[Vehs[f.vehicle_id].proto_id].weapon_id].offense_rating ||
         (Vehs[f.vehicle_id].moves_expended != 0 && speed(f.vehicle_id,0) > Rules.move_rate_roads &&
          !f.local_50 && !f.local_40)) && !(Vehs[f.vehicle_id].state & 8u)) {
        if (((abs(f.local_14-Vehs[f.vehicle_id].waypoint_y[0])+
              x_dist(f.local_18,Vehs[f.vehicle_id].waypoint_x[0])) & ~1) > 2) {
            f.local_80 = Paths.move(f.vehicle_id,f.local_c);
            if (f.local_80 >= 0) {
                f.local_4 = xrange(f.local_18+RadiusBaseX[f.local_80]);
                f.local_8 = f.local_14+RadiusBaseY[f.local_80];
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
    if (enemy_move_pre_tactical(tactical)) goto label_578f46;
    return enemy_move_tactical_tail(tactical);
label_578b26:
label_578b29:
    if (f.local_80 < 0) goto label_578b34;
    f.local_4 = xrange(f.local_18+RadiusBaseX[f.local_80]);
    f.local_8 = f.local_14+RadiusBaseY[f.local_80];
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
    return enemy_move_tactical_action_tail(tactical);
}
