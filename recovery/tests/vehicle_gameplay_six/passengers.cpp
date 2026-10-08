#include <stdint.h>
#include <assert.h>
#include <string.h>
#define __cdecl
#include "game_data_state_host.h"
GameDataStateWindow GameDataStateStorage;
enum { MaxPlayerNum = 8, DTREATY_VENDETTA = 0x10,
       AI_GOAL_ATTACK = 0, AI_GOAL_DEFEND = 2, AI_GOAL_SCOUT = 3,
       AI_GOAL_COLONIZE = 8, AI_GOAL_UNK_3 = 11, BSTATE_UNK_1 = 1 };
struct Base { short x, y; unsigned char faction_id_current; unsigned char pad[0x2b];
              unsigned state; unsigned char nerve_staple_turns_left; };
struct Map { unsigned char climate, contour, val2, region, rest[40]; };
struct PlayerData {
    unsigned diplo_treaties[8];
    unsigned char before_ai[0x3dc - 0x20];
    int ai_fight; int ai_growth;
    unsigned char before_regions[0x117c - 0x3e4];
    unsigned char region_total_bases[128];
    unsigned char region_total_offensive_vehs[128];
    unsigned short region_force_rating[128];
    unsigned short unk_77[128];
    unsigned short unk_78[128];
    unsigned short unk_79[128];
};
Base Bases[8]; PlayerData PlayersData[8]; Map Tiles[16]; Map *MapTiles = Tiles;
int MapLongitude = 8;
Map *map_loc(int x, int y) { return &MapTiles[(x >> 1) + y * MapLongitude]; }
int goals[32][6], goal_count, stack_result, vehicle_result;
int contiguous_calls, stack_calls, veh_calls;
int __cdecl contiguous(int, int, int, int) { ++contiguous_calls; return 1; }
int __cdecl veh_at(int, int) { ++veh_calls; return vehicle_result; }
int __cdecl stack_check(int, int, int, int, int) { ++stack_calls; return stack_result; }
void __cdecl add_goal(int f, int t, int p, int x, int y, int b) {
    int *g = goals[goal_count++]; g[0]=f; g[1]=t; g[2]=p; g[3]=x; g[4]=y; g[5]=b;
}
void reset() {
    memset(Bases, 0, sizeof(Bases)); memset(PlayersData, 0, sizeof(PlayersData));
    memset(Tiles, 0, sizeof(Tiles)); memset(goals, 0, sizeof(goals));
    goal_count = stack_result = vehicle_result = contiguous_calls = stack_calls = veh_calls = 0;
    MapLongitude = 8; GameDataStateStorage.fields_.turn_current_num_ = 0;
}

#include "actual.inc"

int main() {
    // Mode 8 and the native-tested unk_77 bit set take the ordinary count
    // branch; with count below the target it adds colonize and common goals.
    reset(); Bases[0].x=2; Bases[0].y=1; Bases[0].faction_id_current=1;
    PlayersData[1].ai_growth=2; PlayersData[1].ai_fight=0;
    PlayersData[1].unk_77[0] = 0x0800;
    assemble_passengers(0, 8, 7);
    assert(goal_count == 4 && goals[0][1] == AI_GOAL_COLONIZE);
    assert(goals[0][2] == 7 && goals[1][1] == AI_GOAL_DEFEND);

    // Clearing the bit skips the mode-8 colonize branch, distinguishing 0x800
    // from a neighboring flag.
    reset(); Bases[0].x=2; Bases[0].y=1; Bases[0].faction_id_current=1;
    PlayersData[1].ai_growth=99; PlayersData[1].unk_77[0] = 0x0400;
    assemble_passengers(0, 8, 4);
    assert(goal_count == 3 && goals[0][1] == AI_GOAL_DEFEND);
    assert(veh_calls == 0 && stack_calls == 0);

    // Low mode queues colonize then skips common goals.
    reset(); Bases[0].x=2; Bases[0].y=1; Bases[0].faction_id_current=1;
    PlayersData[1].ai_growth=1; vehicle_result=12; stack_result=0;
    assemble_passengers(0, 2, 6);
    assert(goal_count == 1 && goals[0][1] == AI_GOAL_COLONIZE);
    assert(goals[0][2] == 6 && goals[0][5] == -1);
    assert(veh_calls == 1 && stack_calls == 1);

    // Mode 3 is the strict boundary; mode 7 additionally adds the scout tail.
    reset(); Bases[0].x=2; Bases[0].y=1; Bases[0].faction_id_current=1;
    PlayersData[1].ai_growth=0;
    assemble_passengers(0, 3, 5);
    assert(goal_count == 4 && goals[1][1] == AI_GOAL_DEFEND);
    reset(); Bases[0].x=2; Bases[0].y=1; Bases[0].faction_id_current=1;
    PlayersData[1].ai_growth=0;
    assemble_passengers(0, 7, 5);
    assert(goal_count == 5 && goals[4][1] == AI_GOAL_SCOUT);

    // Vendetta count reads the declared byte region_total_bases member.
    reset(); Bases[0].x=2; Bases[0].y=1; Bases[0].faction_id_current=1;
    Tiles[9].region = 1;
    PlayersData[1].diplo_treaties[2] = DTREATY_VENDETTA;
    PlayersData[2].region_total_bases[1] = 1;
    PlayersData[1].ai_growth=0;
    assemble_passengers(0, 2, 5);
    assert(contiguous_calls == 1);
    PlayersData[2].region_total_bases[1] = 0;
    contiguous_calls = 0;
    assemble_passengers(0, 2, 5);
    assert(contiguous_calls == 0);
    return 0;
}
