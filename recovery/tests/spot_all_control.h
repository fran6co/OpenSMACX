#ifndef SPOT_ALL_HOST_SUPPORT_H
#define SPOT_ALL_HOST_SUPPORT_H

#ifndef __cdecl
#define __cdecl
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef short int16_t;
typedef int BOOL;

enum {
    AI_GOAL_ATTACK = 0,
    AI_GOAL_DEFEND = 2,
    AI_GOAL_SCOUT = 3,
    AI_GOAL_UNK_1 = 6,
    VFLAG_LURKER = 0x0040,
    VFLAG_INVISIBLE = 0x0400,
    VSTATE_UNK_200 = 0x00000200u,
    VSTATE_UNK_1000000 = 0x01000000u,
    VSTATE_UNK_2000000 = 0x02000000u,
    STATE_OMNISCIENT_VIEW = 0x00000080u,
    PFLAG_MAP_REVEALED = 0x00000200u,
    PFLAG_UNK_80 = 0x00000080u,
    DTREATY_UNK_200 = 0x00000200u
};

struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t reserved;
    uint8_t faction_id;
    uint8_t year_end_lurking;
    uint8_t dmg_incurred;
    int8_t order;
    uint8_t waypoint_count;
    uint8_t patrol_current_point;
    int16_t waypoint_x[4];
    int16_t waypoint_y[4];
    uint8_t morale;
    uint8_t terraforming_turns;
    uint8_t order_auto_type;
    uint8_t visibility;
    uint8_t moves_expended;
    int8_t facing;
    uint8_t unk_6;
    uint8_t move_to_ai_type;
    uint8_t probe_action;
    uint8_t probe_sabotage_id;
    int16_t home_base_id;
    int16_t next_veh_id_stack;
    int16_t prev_veh_id_stack;
};

struct Map {
    uint8_t climate;
    uint8_t contour;
    uint8_t val2;
    uint8_t region;
    uint8_t visibility;
    uint8_t val3;
    uint8_t unk_1;
    int8_t territory;
    uint32_t bit;
    uint32_t bit2;
    uint32_t bit_visible[7];
};

struct Base {
    int16_t x, y;
    uint8_t faction_id_current, faction_id_former;
    int8_t population_size;
    uint8_t assimilation_turns_left, nerve_staple_turns_left, unk_1;
    uint8_t visibility;
    uint8_t faction_pop_size_intel[8];
    uint8_t reserved_to_facilities[0x79];
    uint8_t facilities_built[12];
};

struct PlayerData {
    uint32_t flags;
    uint32_t diplo_treaties[8];
    int x_target;
    int y_target;
};

struct Player {
    uint32_t rule_flags;
};

// The sensing owner reads a prototype's chassis id and then the chassis triad.
// Keep the host fixture typed like those existing game owners.
struct TestVehPrototype {
    uint8_t chassis_id;
    uint8_t weapon_id;
    uint8_t plan;
};

struct TestRulesChassis {
    uint8_t triad;
    uint8_t range;
};

struct TestRulesWeapon {
    int8_t offense_rating;
};

struct Console {
    int faction_id_;
};

struct MapWin {
    void on_redraw();
};

struct GraphicWin {
    void redraw();
};

struct WorldWin : public GraphicWin {
    void on_redraw();
};

extern Veh Vehs[32];
extern Map TestMap[32];
extern Base TestBases[8];
extern PlayerData PlayersData[8];
extern Player Players[8];
extern Console ConsoleGlobal;
extern uint32_t GameState;
extern int TurnCurrentNum;
extern uint32_t LockEnableMask;
extern uint32_t UnkBitfield1;
extern int TestMultiplayer;
extern int MapLongitudeBounds;
extern int MapLatitudeBounds;
extern int MapLongitude;
extern int MapIsFlat;
extern int TestVisibilityScanAllowed;
extern int TestSpotLocationCount;
extern int TestSpotLocationX[64];
extern int TestSpotLocationY[64];
extern int TestSpotLocationFaction[64];
extern int TestBaseAt[32];
extern int TestSensorAt[32];
extern int TestUseSensorGrid;
extern TestVehPrototype TestVehPrototypes[512];
extern TestRulesChassis TestRulesChassisTable[16];
extern TestRulesWeapon TestRulesWeaponTable[26];
extern int TestVehicleAt[32];
extern int TestStackCheckResult;
extern int TestStackCheckCount;
extern int TestSpotBaseCount;
extern int TestSpotBaseArgs[16][2];
extern int TestObservedVehicleId;
extern uint32_t TestObservedVehicleStateAtBaseReveal;
extern int TestRadiusDrawCount;
extern int TestRadiusDrawArgs[8][4];
extern int TestMainMapRedrawCount;
extern int TestInterludeCount;
extern int TestInterludeArgs[4][4];
extern MapWin *MapWinTable[1];
extern WorldWin WorldClimateWorldWin;
extern int TestWorldGraphicRedrawCount;
extern int TestWorldOnRedrawCount;
extern const int TestRadiusOffsetX[289];
extern const int TestRadiusOffsetY[289];
extern const int TestVehMoveDeltaX[9];
extern const int TestVehMoveDeltaY[9];
extern int TestMapWidth;
extern int TestMapHeight;
extern int TestMapSyncCount;
extern int TestWakeCount;
extern int TestWakeFaction;
extern int TestWakeSource;
extern int TestEventOrder;
extern int TestEncounterCount;
extern int TestEncounterFaction[16];
extern int TestEncounterArgs[16][6];
extern int TestGoalCount;
extern int TestGoalArgs[64][6];

int veh_top(int vehicle_id);
int base_at(int x, int y);
int base_find(int x, int y);
int veh_at(int x, int y);
int stack_fix(int vehicle_id);
int stack_check(int vehicle_id, int type, int condition_one,
                int condition_two, int condition_three);
BOOL on_map(int x, int y);
Map *map_loc(int x, int y);
void synch_bit(int x, int y, int faction_id);
void spot_stack(int vehicle_id, int faction_id);
void spot_base(int base_id, int faction_id);
void wake_stack(int vehicle_id);
BOOL want_to_wake(int faction_id, int vehicle_id, int spotted_vehicle_id);
void encounter(int faction_id, int other_faction_id, int x, int y,
               int event_flag_a, int event_flag_b);
BOOL has_abil(int prototype_id, int ability_id);
BOOL is_sensor(int x, int y);
int whose_territory(int faction_id, int x, int y, int *owner_out, int mode);
void bitmask(int input, int *offset, int *mask);
int get_proto_triad(int prototype_id);
void spot_loc(int x, int y, int faction_id);
void spot_tile(int x, int y, int faction_id);
void add_goal(int faction_id, int subgoal, int goal_type,
              int x, int y, int expiry);
void draw_radius(int x, int y, int radius, int draw_type);
void interlude(int id, char *text, int mode, int ending);

// Bind source-owned identifiers to host fixtures only for the private
// control projection. The source-shaped candidate names the production owners.
#define Bases TestBases
#define VehPrototypes TestVehPrototypes
#define Chassis TestRulesChassisTable
#define Weapon TestRulesWeaponTable
#define RadiusOffsetX TestRadiusOffsetX
#define RadiusOffsetY TestRadiusOffsetY
#define VehMoveDeltaX TestVehMoveDeltaX
#define VehMoveDeltaY TestVehMoveDeltaY
#define IsMultiplayerNet TestMultiplayer
#define GameMorePreferences TestVisibilityScanAllowed

#endif
