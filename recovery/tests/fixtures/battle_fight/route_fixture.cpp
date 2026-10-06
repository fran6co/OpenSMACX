#include <assert.h>
#include <stdint.h>
#define __cdecl
struct Veh { int faction_id; uint16_t flags; uint8_t visibility; };
struct PlayerData { uint32_t diplo_treaties[8]; };
static Veh Vehs[4]; static PlayerData PlayersData[8];
static uint32_t GameState, GamePreferences;
struct ConsoleMock { int faction_id_; };
static ConsoleMock ConsoleGlobal;
static int post_setup_calls;
static void battle_fight_post_prompt_setup(int,int,int,int,int,char*,char*) {
    ++post_setup_calls;
}
