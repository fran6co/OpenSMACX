#ifndef ENEMY_PB_HOST_FIXTURE_H
#define ENEMY_PB_HOST_FIXTURE_H
#include <stdint.h>
#include <stdlib.h>
#ifndef __cdecl
#define __cdecl
#endif

typedef int BOOL;
struct Veh {
    int faction_id, proto_id;
    short x, y;
    unsigned char visibility;
};
struct Base {
    int faction_id_current, faction_id_former;
    short x, y;
    signed char population_size;
    int queue_production_id[10];
    int minerals_accumulated;
};
struct PlayerData {
    uint32_t flags, ranking;
    int diff_level, ai_fight, mil_strength_1, major_atrocities;
    uint32_t diplo_treaties[8], satellites_[4], sat_odp_deployed;
    int corner_market_turn;
    uint8_t region_base_plan[128], region_total_bases[128];
};
struct VehPrototype { unsigned short cost; };
struct RulesBasic { uint32_t move_rate_roads; };
struct Map { unsigned char bit, val2, region; };

extern Veh Vehs[32];
extern Base Bases[32];
extern PlayerData PlayersData[8];
extern VehPrototype VehPrototypes[32];
extern RulesBasic Rules;
struct GameDataLockMaskView { uint32_t enable_mask_; };
struct GameDataStateWindowFields {
    int base_current_count_, vehicle_current_count_, turn_current_num_;
    uint32_t game_rules_; GameDataLockMaskView lock_masks_;
    int council_resolution_count_[11];
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
extern GameDataStateWindow GameDataStateStorage;
extern uint32_t &LockEnableMask, &GameRules;
extern int &BaseCurrentCount, &VehCurrentCount, &TurnCurrentNum;
extern int (&CouncilResolutionCount)[11];
extern int MapIsFlat, MapLongitude, MapLongitudeBounds, MapLatitudeBounds;
extern int RadiusOffsetX[16], RadiusOffsetY[16], RadiusBaseX[8], RadiusBaseY[8];
extern int RadiusRange[4];
extern int mock_at_climax_calls, mock_lift_calls, mock_drop_calls;
extern int mock_drop_x, mock_drop_y, mock_stack_value;
extern int mock_veh_at[16][16];
extern int mock_base_at[16][16];
extern Map MockMap[16][16];

enum { MaxSecretProjectNum = 64 };
enum { PROP_REPEAL_UN_CHARTER = 8, PROP_REINSTATE_UN_CHARTER = 9 };
enum { FAC_HEADQUARTERS = 1, FAC_ASCENT_TO_TRANSCENDENCE = 99 };
enum { SP_HUNTER_SEEKER_ALGO = 2, SP_CLONING_VATS = 3 };
enum {
    DTREATY_PACT = 1, DTREATY_TREATY = 2, DTREATY_TRUCE = 4,
    DTREATY_WANT_REVENGE = 0x20, DTREATY_UNK_80000 = 0x80000
};
enum { PFLAG_COMMIT_ATROCIT_WANTONLY = 0x1000000 };
enum { RULES_INTENSE_RIVALRY = 0x40 };


extern int SecretProject[MaxSecretProjectNum];

inline int get_proto_offense_rating(uint32_t) { return 'a'; }
inline int get_offense_rating(uint32_t) { return 'a'; }
inline uint32_t planet_buster(int) { return 0; }
inline int speed(int, int) { return 10000; }
inline int at_climax(int) { ++mock_at_climax_calls; return 0; }
inline int reputation(int, int) { return 0; }
inline int base_at(int x, int y) { return mock_base_at[y][x]; }
inline int veh_at(int x, int y) { return mock_veh_at[y][x]; }
inline int stack_fix(int vehicle) { return vehicle; }
inline int stack_check(int, int, int, int, int) { return mock_stack_value; }
inline Map *map_loc(int x, int y) { return &MockMap[y][x]; }
inline int has_fac_built(int, int) { return 0; }
inline int base_project(int project) { return SecretProject[project]; }
inline int veh_lift(int) { ++mock_lift_calls; return 0; }
inline int veh_drop(int vehicle_id, int x, int y) {
    ++mock_drop_calls; mock_drop_x = x; mock_drop_y = y;
    /* Model a callback that mutates the live vehicle after the lift. */
    Vehs[vehicle_id].x = static_cast<short>(x);
    Vehs[vehicle_id].y = static_cast<short>(y);
    Vehs[vehicle_id].visibility = 0x7f;
    return 0;
}

#endif
