// The entry probe includes the exact mapped-function prefix before the first
// event branch. It exercises annual controls and selection/callback ordering,
// not game ABI.
#include <assert.h>
#include <stdint.h>
#include <string.h>

struct Base {
    short x, y;
    unsigned char faction_id_current;
    signed char population_size;
    unsigned int event;
    int random_event_turns;
    char name_string[25];
};
struct PlayerData {
    unsigned int current_num_bases;
    unsigned int diplo_treaties[8];
    unsigned int diff_level, ranking, energy_reserves, flags, satellites_[4];
};
static Base Bases[8];
static PlayerData PlayersData[8];
static char StringTemp[1032];
static unsigned int faction_rankings[8];
static unsigned int lock_enable_mask;
static int global_energy_event_state, sunspot_duration, dust_cloud_duration;
static int base_current_count, turn_current_num, diff_level_current;
static int game_state, game_rules;
static unsigned int MapRandSeed;
static int StartingMissionYear;
static int IsMultiplayerPBEM;
static int random_values[8], random_count, random_index;
static int random_calls, reseed_calls, reseed_value;
static int parse_calls, parse_order[4], mutate_base_on_parse;
static int set_treaty_calls, treaty_row, treaty_column, popup_calls;
static char popup_captions[8][32], popup_images[8][32];
static int popup_args[8];
static int entry_probe_reached, entry_probe_event, entry_probe_faction;
static int entry_probe_base, entry_probe_roll, entry_probe_x, entry_probe_y;

struct LockMasksRef { unsigned int &enable_mask_; explicit LockMasksRef(unsigned int &v) : enable_mask_(v) {} };
struct GameFieldsRef {
    int &global_energy_event_state_, &sunspot_duration_, &dust_cloud_duration_;
    LockMasksRef lock_masks_;
    int &base_current_count_, &turn_current_num_, &diff_level_current_;
    int &game_state_, &game_rules_;
    unsigned int (&faction_rankings_)[8];
    GameFieldsRef(int &energy, int &sunspot, int &dust, unsigned int &lock,
                  int &base_count, int &turn, int &diff, int &state, int &rules,
                  unsigned int (&rankings)[8])
        : global_energy_event_state_(energy), sunspot_duration_(sunspot),
          dust_cloud_duration_(dust), lock_masks_(lock),
          base_current_count_(base_count), turn_current_num_(turn),
          diff_level_current_(diff), game_state_(state), game_rules_(rules),
          faction_rankings_(rankings) {}
};
struct GameStateRef {
    GameFieldsRef fields_;
    GameStateRef(int &energy, int &sunspot, int &dust, unsigned int &lock,
                 int &base_count, int &turn, int &diff, int &state, int &rules,
                 unsigned int (&rankings)[8])
        : fields_(energy, sunspot, dust, lock, base_count, turn, diff, state,
                  rules, rankings) {}
};
static GameStateRef GameDataStateStorage(global_energy_event_state, sunspot_duration,
    dust_cloud_duration, lock_enable_mask, base_current_count, turn_current_num,
    diff_level_current, game_state, game_rules, faction_rankings);

static int game_year(int turn) { return StartingMissionYear + turn; }
static int random(int low, int high) {
    assert(random_index < random_count);
    assert(low == 0 && (high == 100 || high == 0x16));
    ++random_calls;
    return random_values[random_index++];
}
static void random_reseed(int seed) { ++reseed_calls; reseed_value = seed; }
static int parse_num(int low, int high) {
    assert(low == 0 && high == 10);
    parse_order[parse_calls++] = 1;
    if (mutate_base_on_parse) { Bases[0].x = 19; Bases[0].y = 20; }
    return 0;
}
static int parse_says(int, const char *name, int x, int y) {
    assert(strcmp(name, "NATIVE BASE") == 0 && x == -1 && y == -1);
    parse_order[parse_calls++] = 2;
    return 0;
}
static void set_treaty(int row, int column, int treaty, int enabled) {
    assert(row >= 0 && row < 8 && column >= 0 && column < 8);
    assert(treaty == 0xC00 && enabled == 0);
    ++set_treaty_calls; treaty_row = row; treaty_column = column;
}
static int POP2(const char *caption, const char *image, int argument) {
    assert(popup_calls < 8);
    strcpy(popup_captions[popup_calls], caption);
    strcpy(popup_images[popup_calls], image);
    popup_args[popup_calls] = argument;
    ++popup_calls;
    return 0;
}

#include "actual_entry_probe.inc"

static void reset_fixture() {
    memset(Bases, 0, sizeof(Bases)); memset(PlayersData, 0, sizeof(PlayersData));
    memset(faction_rankings, 0, sizeof(faction_rankings)); memset(StringTemp, 0, sizeof(StringTemp));
    global_energy_event_state = sunspot_duration = dust_cloud_duration = 0;
    lock_enable_mask = 0; base_current_count = 2; turn_current_num = 100;
    diff_level_current = game_state = game_rules = 0; MapRandSeed = 0;
    StartingMissionYear = 0;
    IsMultiplayerPBEM = 0; random_count = random_index = random_calls = 0;
    reseed_calls = reseed_value = parse_calls = set_treaty_calls = popup_calls = 0;
    treaty_row = treaty_column = -1;
    memset(popup_captions, 0, sizeof(popup_captions));
    memset(popup_images, 0, sizeof(popup_images));
    memset(popup_args, 0, sizeof(popup_args));
    mutate_base_on_parse = entry_probe_reached = 0;
    entry_probe_event = entry_probe_faction = entry_probe_base = entry_probe_roll = -1;
    entry_probe_x = entry_probe_y = -1;
    Bases[0].x = 4; Bases[0].y = 5; Bases[0].population_size = 4;
    Bases[0].faction_id_current = 2; strcpy(Bases[0].name_string, "NATIVE BASE");
    PlayersData[2].current_num_bases = 2;
}

int main() {
    // Every event kind traverses the actual common selection prefix; parser
    // callbacks mutate live coordinates only after the captured pair exists.
    for (int event = 0; event < 22; ++event) {
        reset_fixture(); random_values[0] = 0; random_values[1] = event; random_count = 2;
        mutate_base_on_parse = 1; random_events_entry_probe(1);
        assert(entry_probe_reached && random_calls == 2 && parse_calls == 2);
        assert(parse_order[0] == 1 && parse_order[1] == 2);
        assert(entry_probe_event == event && entry_probe_faction == 2 && entry_probe_base == 0);
        assert(entry_probe_roll == 0 && entry_probe_x == 4 && entry_probe_y == 5);
        assert(Bases[0].x == 19 && Bases[0].y == 20 && StringTemp[0] == '\0');
    }

    reset_fixture(); random_values[0] = 2; random_count = 1;
    random_events_entry_probe(1);
    assert(!entry_probe_reached && random_calls == 1 && parse_calls == 0);

    reset_fixture(); Bases[0].population_size = 3;
    random_values[0] = 0; random_count = 1;
    random_events_entry_probe(1);
    assert(!entry_probe_reached && random_calls == 1 && parse_calls == 0);

    reset_fixture(); PlayersData[2].current_num_bases = 1;
    random_values[0] = 0; random_count = 1;
    random_events_entry_probe(1);
    assert(!entry_probe_reached && random_calls == 1 && parse_calls == 0);

    reset_fixture(); turn_current_num = 40;
    random_events_entry_probe(1);
    assert(!entry_probe_reached && random_calls == 0 && reseed_calls == 0);

    reset_fixture(); MapRandSeed = 1000; random_values[0] = 0;
    random_values[1] = 4; random_count = 2;
    random_events_entry_probe(0);
    assert(reseed_calls == 1 && reseed_value == 2300);
    assert(entry_probe_reached && entry_probe_event == 4);

    // Annual timer ordering and its intentionally distinct negative paths.
    reset_fixture(); global_energy_event_state = 3; sunspot_duration = -1500;
    dust_cloud_duration = -1; random_values[0] = 0; random_values[1] = 4; random_count = 2;
    random_events_entry_probe(0);
    assert(global_energy_event_state == 0); // bit 2 clears before bit 1 is tested
    assert(sunspot_duration == -1001 && dust_cloud_duration == 0);
    assert(popup_calls == 0);

    reset_fixture(); global_energy_event_state = 1; sunspot_duration = 1;
    dust_cloud_duration = 1; random_values[0] = 0; random_values[1] = 4; random_count = 2;
    random_events_entry_probe(0);
    assert(global_energy_event_state == 2 && sunspot_duration == 0 && dust_cloud_duration == 0);
    assert(popup_calls == 2);
    assert(strcmp(popup_captions[0], "NOMORESPOTS") == 0 &&
           strcmp(popup_images[0], "space_sm.pcx") == 0 && popup_args[0] == -1);
    assert(strcmp(popup_captions[1], "NOMOREDUST") == 0 &&
           strcmp(popup_images[1], "stars_sm.pcx") == 0 && popup_args[1] == -1);

    reset_fixture(); lock_enable_mask = 1u << 2;
    for (int row = 0; row < 8; ++row) PlayersData[row].diplo_treaties[2] = 0x10;
    PlayersData[4].diplo_treaties[2] = 0;
    random_values[0] = 0; random_values[1] = 4; random_count = 2;
    random_events_entry_probe(0);
    assert(set_treaty_calls == 1 && treaty_row == 4 && treaty_column == 2);

    reset_fixture(); base_current_count = 3;
    Bases[0].event = 0x200; Bases[0].random_event_turns = 1;
    Bases[1].event = 0x400; Bases[1].random_event_turns = -1;
    Bases[2].event = 0x100; Bases[2].random_event_turns = 0;
    random_values[0] = 0; random_values[1] = 4; random_count = 2;
    random_events_entry_probe(0);
    assert(Bases[0].random_event_turns == 0 && Bases[0].event == 0);
    assert(Bases[1].random_event_turns == -1 && Bases[1].event == 0x400);
    assert(Bases[2].random_event_turns == 0 && Bases[2].event == 0x100);

    reset_fixture(); game_state = 0x100000;
    StartingMissionYear = 0x8A2 - turn_current_num;
    random_events_entry_probe(0);
    assert((game_state & 0x100000) == 0 && random_calls == 0);
    assert(popup_calls == 1 && strcmp(popup_captions[0], "PERIHELIONENDS") == 0);
    assert(strcmp(popup_images[0], "heat_sm.pcx") == 0 && popup_args[0] == -1);

    reset_fixture(); game_state = 0;
    StartingMissionYear = 0x88E - turn_current_num;
    random_events_entry_probe(0);
    assert((game_state & 0x100000) != 0 && random_calls == 0);
    assert(popup_calls == 1 && strcmp(popup_captions[0], "PERIHELION") == 0);

    reset_fixture(); game_rules = 0x8000;
    random_events_entry_probe(1);
    assert(!entry_probe_reached && random_calls == 0 && reseed_calls == 0);
    reset_fixture(); diff_level_current = 1; turn_current_num = 64;
    random_events_entry_probe(1);
    assert(!entry_probe_reached && random_calls == 0);
    reset_fixture(); diff_level_current = 1; turn_current_num = 65;
    random_values[0] = 0; random_values[1] = 4; random_count = 2;
    random_events_entry_probe(1);
    assert(entry_probe_reached && random_calls == 2);
    return 0;
}
