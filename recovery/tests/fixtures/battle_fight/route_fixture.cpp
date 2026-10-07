#include <assert.h>
#include <stdint.h>
#define __cdecl
struct Veh { int faction_id; uint16_t flags; uint8_t visibility; };
struct PlayerData { uint32_t diplo_treaties[8]; };
static Veh Vehs[4]; static PlayerData PlayersData[8];
struct TestGameDataStateFields {
    uint32_t game_state_, game_more_preferences_, game_preferences_;
    struct { uint32_t enable_mask_; } lock_masks_;
};
struct TestGameDataState { TestGameDataStateFields fields_; };
static TestGameDataState GameDataStateStorage;
struct ConsoleMock { int faction_id_; };
static ConsoleMock ConsoleGlobal;
static int post_setup_calls;
static void battle_fight_post_prompt_setup(int,int,int,int,int,char*,char*) {
    ++post_setup_calls;
}
