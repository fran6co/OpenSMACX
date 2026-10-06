#include <stdint.h>
#include <string.h>
#include <assert.h>
#define __cdecl
const uint32_t RFLAG_ALIEN = 0x80u;
// ACTUAL_SOURCE_TYPES
struct PlayerSeam { uint32_t rule_flags; };
struct ConsoleSeam { int faction_id_; };
PlayerSeam Players[8];
MonumentFaction FactionMonuments[8];
ConsoleSeam ConsoleGlobal;
int TurnCurrentNum;
// Test-only projection of the actual source-owned turn field.
struct GameDataStorageFixture { struct Fields { int turn_current_num_; } fields_; } GameDataStateStorage;
static int year_calls, notifications, year_effect;
static int expected_faction, expected_target, expected_first, expected_flag;
unsigned int game_year(int turn) {
    ++year_calls;
    if (year_effect == 1) Players[1].rule_flags = RFLAG_ALIEN;
    if (year_effect == 2) ConsoleGlobal.faction_id_ = 1;
    return 2100u + turn;
}
void monument(int from_load) {
    assert(from_load == 1);
    ++notifications;
    const MonumentFaction &f = FactionMonuments[expected_faction];
    const MonumentRecord &r = f.records_[6];
    assert(r.achieved_ == 1 && r.year_ == 2137);
    assert(r.word_8_ == expected_target && r.owner_faction_ == expected_first);
    assert(r.word_14_ == expected_flag && f.latest_type_ == 6);
}
// ACTUAL_SOURCE_BODY
static void reset(int faction, int target, int first, int flag) {
    memset(Players, 0, sizeof Players);
    memset(FactionMonuments, 0, sizeof FactionMonuments);
    ConsoleGlobal.faction_id_ = -1;
    TurnCurrentNum = 37;
    GameDataStateStorage.fields_.turn_current_num_ = TurnCurrentNum;
    year_calls = notifications = year_effect = 0;
    expected_faction = faction; expected_target = target;
    expected_first = first; expected_flag = flag;
}
int main() {
    reset(1, 2, 1, 1);
    Players[1].rule_flags = RFLAG_ALIEN;
    mon_enemy_destroyed(1, 2);
    assert(year_calls == 1 && notifications == 0);
    assert(FactionMonuments[1].records_[6].achieved_ == 0);
    reset(1, 0, 1, 1);
    mon_enemy_destroyed(1, 0);
    assert(year_calls == 1 && FactionMonuments[1].records_[6].achieved_ == 0);
    reset(1, 2, 1, 1);
    FactionMonuments[1].records_[6].achieved_ = 7;
    FactionMonuments[1].records_[6].word_8_ = 99;
    mon_enemy_destroyed(1, 2);
    assert(year_calls == 1 && FactionMonuments[1].records_[6].word_8_ == 99);

    // First achievement excludes the current faction, includes neutral peers,
    // writes only record6, and publishes every field before notification.
    reset(1, -7, 1, 1);
    ConsoleGlobal.faction_id_ = 1;
    FactionMonuments[1].records_[5].word_8_ = 99;
    mon_enemy_destroyed(1, -7);
    assert(notifications == 1);
    assert(FactionMonuments[1].records_[5].word_8_ == 99);
    mon_enemy_destroyed(1, 4);
    assert(year_calls == 2 && notifications == 1);
    assert(FactionMonuments[1].records_[6].word_8_ == -7);

    // No first flag among earlier achievers means target, not self, fallback.
    reset(1, 4, 4, 0);
    ConsoleGlobal.faction_id_ = 1;
    FactionMonuments[0].records_[6].achieved_ = 2;
    mon_enemy_destroyed(1, 4);
    assert(notifications == 1);

    // Last qualifying peer wins; peer7 must participate even if alien.
    reset(1, 4, 7, 0);
    ConsoleGlobal.faction_id_ = 1;
    FactionMonuments[0].records_[6].achieved_ = 1;
    FactionMonuments[0].records_[6].word_14_ = 1;
    FactionMonuments[2].records_[6].achieved_ = 1;
    FactionMonuments[2].records_[6].word_14_ = 9;
    FactionMonuments[7].records_[6].achieved_ = -1;
    FactionMonuments[7].records_[6].word_14_ = -1;
    Players[7].rule_flags = RFLAG_ALIEN;
    mon_enemy_destroyed(1, 4);
    assert(notifications == 1);

    // Peer7 as the sole prior achiever must suppress the self-first branch.
    reset(1, 4, 7, 0);
    ConsoleGlobal.faction_id_ = 1;
    FactionMonuments[7].records_[6].achieved_ = 1;
    FactionMonuments[7].records_[6].word_14_ = 1;
    mon_enemy_destroyed(1, 4);
    assert(notifications == 1);

    reset(1, 2, 1, 1);
    year_effect = 1;
    mon_enemy_destroyed(1, 2);
    assert(year_calls == 1 && FactionMonuments[1].records_[6].achieved_ == 0);
    reset(1, 2, 1, 1);
    year_effect = 2;
    mon_enemy_destroyed(1, 2);
    assert(notifications == 1);
    return 0;
}
