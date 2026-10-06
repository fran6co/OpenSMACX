// Synthetic host services for the complete source-extracted serializer.
// These bounded controls are not a product build or full runtime proof.
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#ifndef __cdecl
#define __cdecl
#endif

typedef char *LPSTR;
struct RulesTimeControl { LPSTR name; int words[7]; };
RulesTimeControl TimeControl[6];
char StringTemp[1032];
int GameDataWriteMode;
int GameDataEncryptedMode;
int GameDataLoadStatusWord;
int GameDataPlayerCountSnapshot;
struct ConsoleGlobalMock {
    int faction_id_;
    unsigned int cursor_active_;
    int selected_vehicle_;
    int cursor_history_index_;
    int cursor_history_x_[32];
    int cursor_history_y_[32];
    unsigned char field_23D20_[4];
    int game_data_private_word(FILE *file, int write_mode);
} ConsoleGlobal;
int GameDataLoadRequest;
struct PlayerData {
    unsigned char prefix_[0x458];
    unsigned char facility_announced[9];
    unsigned char announcement_reserved[3];
    int planet_ecology;
    int base_id_atk_target;
    int unk_37;
    char saved_queue_name[8][24];
    int saved_queue_size[8];
    int saved_queue_items[8][10];
    int unk_38[8];
    int unk_39[8][9];
    int unk_46[9];
    int unk_47;
    int unk_48;
    int unk_49;
    unsigned int nutrient_surplus_total;
    int labs_total;
    unsigned int satellites_[4];
    int best_weapon_value;
    int best_psi_offense;
    int best_psi_defense;
    int best_armor_value;
    int best_land_speed;
    int enemy_best_weapon_value;
    int enemy_best_armor_value;
    int enemy_best_land_speed;
    int enemy_best_psi_offense;
    int enemy_best_psi_defense;
    int unk_64;
    int unk_65;
    int unk_66;
    int unk_67;
    int unk_68;
    char unk_69[4];
    unsigned char proto_id_active[512];
    unsigned char proto_id_queue[512];
    short proto_id_lost[512];
    int total_mil_veh;
    unsigned int current_num_bases;
    int mil_strength_1;
    int mil_strength_2;
    int pop_total;
    int unk_70;
    int planet_busters;
    int unk_71;
    int unk_72;
    unsigned short region_total_combat_vehs[128];
    unsigned char region_total_bases[128];
    unsigned char region_total_offensive_vehs[128];
    unsigned short region_force_rating[128];
    unsigned short unk_77[128];
    unsigned short unk_78[128];
    unsigned short unk_79[128];
    unsigned short unk_80[128];
    unsigned short unk_81[128];
    unsigned char unk_82[128];
    unsigned char unk_83[128];
    unsigned char region_base_plan[128];
    struct Goal { unsigned char bytes_[0x10]; } goals[75];
    Goal sites[25];
    int unk_93;
    int unk_94;
    int unk_95;
    int unk_96;
    int unk_97;
    unsigned int tech_achieved;
    int time_bonus_count;
    int unk_99;
    unsigned int secret_project_intel[8];
    int corner_market_turn;
    int corner_market_active;
    int unk_101;
    int unk_102;
    int prototype_history_cursor;
    unsigned int flags_ext;
    int unk_105;
    int unk_106;
    int unk_107;
    int unk_108;
    int unk_109;
    int score_items_[9];
};
typedef char PlayerDataFixtureHasCurrentStride[
    sizeof(PlayerData) == 0x20CC ? 1 : -1];
PlayerData PlayersData[8];
struct Player { unsigned char bytes_[0x59C]; } Players[8];
uint32_t FactionRankingsUnk[8];
uint16_t FactionRankingHistory[8000];
int ReplayEventBytes;
struct ReplayEventRecord { unsigned char bytes_[8]; } ReplayEvents[1024];
struct Continent { unsigned char bytes_[0x70]; } Continents[32];
struct VehPrototype {
    char veh_name[32];
    unsigned int ability_flags;
    unsigned char chassis_id;
    unsigned char weapon_id;
    unsigned char armor_id;
    unsigned char reactor_id;
    unsigned char carry_capacity;
    unsigned char cost;
    unsigned char plan;
    signed char unk_1;
    unsigned char obsolete_factions;
    signed char combat_factions;
    signed char icon_offset;
    signed char padding;
    unsigned short flags;
    short preq_tech;
} VehPrototypes[512];
struct Veh {
    int16_t x;
    int16_t y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t unk_1;
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
} Vehs[2049];
struct Base {
    int16_t x;
    int16_t y;
    uint8_t faction_id_current;
    uint8_t faction_id_former;
    int8_t population_size;
    uint8_t assimilation_turns_left;
    uint8_t nerve_staple_turns_left;
    uint8_t unk_1;
    uint8_t visibility;
    uint8_t faction_pop_size_intel[8];
    char name_string[25];
    int16_t unk_x;
    int16_t unk_y;
    uint32_t state;
    uint32_t event;
    uint32_t governor;
    int nutrients_accumulated;
    int minerals_accumulated;
    int production_id_last;
    int eco_damage;
    uint32_t queue_size;
    int queue_production_id[10];
    int resource_sqr_radius;
    int specialist_total;
    int specialist_unk_1;
    uint32_t spec[2];
    uint8_t facilities_built[12];
    int mineral_surplus_final;
    int minerals_accumulated_2;
    int unk_6;
    int unk_7;
    int unk_8;
    int unk_9;
    int nutrient_intake_1;
    int mineral_intake_1;
    int energy_intake_1;
    int unused_intake_1;
    int nutrient_intake_2;
    int mineral_intake_2;
    int energy_intake_2;
    int unused_intake_2;
    int nutrient_surplus;
    int mineral_surplus;
    int energy_surplus;
    int unused_surplus;
    int nutrient_inefficiency;
    int mineral_inefficiency;
    int energy_inefficiency;
    int unused_inefficiency;
    int nutrient_consumption;
    int mineral_consumption;
    int energy_consumption;
    int unused_consumption;
    int economy_total;
    int psych_total;
    int labs_total;
    int unk_10;
    int16_t auto_forward_land_base_id;
    int16_t auto_forward_sea_base_id;
    int16_t auto_forward_air_base_id;
    int16_t padding_0;
    int talent_total;
    int drone_total;
    int super_drone_total;
    int random_event_turns;
    int nerve_staple_count;
    int unk_11;
    int location;
} Bases[512];
typedef char BaseFixtureHasNativeExtent[sizeof(Base) == 0x134 ? 1 : -1];
typedef char BaseFixtureHasNativeFacilityOffset[
    offsetof(Base, facilities_built) == 0x8C ? 1 : -1];
struct MapWin { int field_1DD74_; };
MapWin *MapWinTable[8];
struct WorldWinMock { MapWin mapWin_; } WorldClimateWorldWin;
char ScenarioSaveNameBuffer[256];
char SaveNameBuffer[256];
struct ScenarioSaveState {
    int objective_req_victory;
    uint32_t objectives_sudden_death_victory;
    uint32_t objective_achieve_points;
    uint32_t victory_achieve_bonus_points;
    uint32_t starting_mission_year;
    uint32_t ending_mission_year;
    uint8_t wire_opaque_94b4d8[0x80];
};
ScenarioSaveState ScenarioSave;
typedef char ScenarioSaveHasNativeExtent[
    sizeof(ScenarioSaveState) == 0x98 ? 1 : -1];
typedef char ScenarioSaveReqAtZero[
    offsetof(ScenarioSaveState, objective_req_victory) == 0 ? 1 : -1];
typedef char ScenarioSaveSuddenAtFour[
    offsetof(ScenarioSaveState, objectives_sudden_death_victory) == 4 ? 1 : -1];
typedef char ScenarioSavePointsAtEight[
    offsetof(ScenarioSaveState, objective_achieve_points) == 8 ? 1 : -1];
typedef char ScenarioSaveBonusAtTwelve[
    offsetof(ScenarioSaveState, victory_achieve_bonus_points) == 12 ? 1 : -1];
typedef char ScenarioSaveStartAtSixteen[
    offsetof(ScenarioSaveState, starting_mission_year) == 16 ? 1 : -1];
typedef char ScenarioSaveEndAtTwenty[
    offsetof(ScenarioSaveState, ending_mission_year) == 20 ? 1 : -1];
typedef char ScenarioSaveTailHasNativeOffset[
    offsetof(ScenarioSaveState, wire_opaque_94b4d8) == 0x18 ? 1 : -1];
struct SaveNameDefaultStorageMock {
    char game_data_migration_default_[1];
    char padding_[3];
    char scenario_equal_default_[1];
    char padding2_[3];
    char save_equal_default_[1];
} SaveNameDefaultStorageGlobal;
struct MonumentFaction { unsigned char bytes_[0x4F4]; } FactionMonuments[8];
uint32_t TectonicDetonationCount[8];
struct OfferResetCell { unsigned char bytes_[0x90]; } OfferResetCells[8][8];
uint32_t PendingDiplomacyFlags[8][8];
int FactionTurnCombatWins[8];
int FactionTurnCombatLosses[8];
static int scenario_clear_calls;
static int monuments_clear_calls;
static void clear_scenario() { ++scenario_clear_calls; }
static void clear_monuments() { ++monuments_clear_calls; }
static int map_data_block(FILE *, void *, unsigned int);
int ConsoleGlobalMock::game_data_private_word(FILE *file, int write_mode) {
    if (write_mode != 0) {
        memset(ConsoleGlobal.field_23D20_, 0,
               sizeof(ConsoleGlobal.field_23D20_));
    }
    return map_data_block(file, ConsoleGlobal.field_23D20_, 4);
}
static int map_read_calls;
static int map_write_calls;
static int map_read(FILE *) { ++map_read_calls; return 0; }
static int map_write(FILE *) { ++map_write_calls; return 0; }
static int game_io(MapWin *, FILE *) { return 0; }
static void *mem_get(size_t size) { return malloc(size); }
struct BaseSecretProjectMock { int words[64]; };
struct BaseSecretProject { unsigned char bytes_[0x100]; };
/* ACTUAL_GAME_DATA_STATE_HEADER */
GameDataStateWindow GameDataStateStorage;
struct NetSaveStorageMock { unsigned char process_slots_[0xCE0]; };
struct NetDaemonNetMock { NetSaveStorageMock save_storage_; };
NetDaemonNetMock NetDaemonNet;

struct StringStructPayload { char *text; unsigned int field_8_; unsigned int field_C_; };
struct StringEntry { int id; StringStructPayload payload; };
struct StringList {
    int entry_count_;
    int current_position_;
    void *allocator_;
    char *pending_text_;
    unsigned int field_20_;
    unsigned int field_24_;
    int cursor_;
    std::vector<StringEntry> entries_;
    std::vector<std::string> texts_;
    void remove_all() { entries_.clear(); texts_.clear(); entry_count_ = 0; }
    int seek_pos(int index) {
        if (index > entry_count_ - 1) return 0;
        cursor_ = index;
        current_position_ = index;
        return 1;
    }
    StringStructPayload *current_entry() {
        return entry_count_ ? &entries_[cursor_].payload : 0;
    }
    int current_id() { return entry_count_ ? entries_[cursor_].id : 0; }
    int add(int id) {
        StringEntry entry;
        entry.id = id;
        texts_.push_back(pending_text_ ? pending_text_ : const_cast<char *>(""));
        entry.payload.text = &texts_.back()[0];
        entries_.push_back(entry);
        entry_count_ = static_cast<int>(entries_.size());
        cursor_ = entry_count_ - 1;
        current_position_ = cursor_;
        return 0;
    }
};
StringList MessageWinDeferredBaseMessages[8];
StringList DiplomacyTranscriptLists[8][8];
struct CouncWin {
    uint32_t field_A1C_;
    uint32_t field_A20_;
    uint32_t field_A24_;
    uint32_t field_A28_;
    uint32_t leader_label_kind_[8];
    int leader_tech_[8];
} CouncilWindow;
int CouncilPbemGate;
int CouncilNotifyFormSlots[8];
int CouncilNotifyPluralSlots[8];
int TamperState93A9AC;
int TamperState93A9B0;
int TamperState93A9B4;
int TamperDialogFaction[8];
int TamperDialogValue[8];
int IfaceScreenWidth;
int IfaceScreenHeight;

struct TransferStep { void *buffer; unsigned int size; };
static std::vector<TransferStep> transfer_steps;
static int fail_at_step = -1;
static int transfer_header_version = 12;
static int transfer_header_vehicle_count;
static int transfer_header_base_count;
static int plain_transfer_count;
static int read_scenario_from_stream;
static int read_header_from_stream;
static int read_state_from_stream;
static int plain_read_from_stream;
static int map_data_block(FILE *file, void *buffer, unsigned int size) {
    const int index = static_cast<int>(transfer_steps.size());
    TransferStep step = {buffer, size};
    transfer_steps.push_back(step);
    if (index == fail_at_step) return 0;
    if (GameDataWriteMode != 0) {
        // fwrite observes, but never initializes or mutates, its input. In
        // particular, do not inspect the intentionally indeterminate native
        // 0x31C header buffer on this path.
        return 1;
    }
    if (read_header_from_stream && size == 0x31C)
        return static_cast<int>(fread(buffer, size, 1, file));
    if (read_state_from_stream && buffer == &GameDataStateStorage
        && size == 0x398)
        return static_cast<int>(fread(buffer, size, 1, file));
    if (read_scenario_from_stream && buffer == &ScenarioSave && size == 0x98)
        return static_cast<int>(fread(buffer, size, 1, file));
    if (size == 0x31C) {
        memset(buffer, 0, size);
        unsigned char *const header = static_cast<unsigned char *>(buffer);
        *reinterpret_cast<int *>(header + 0x304) = transfer_header_version;
        if (transfer_header_version < 0xB) {
            *reinterpret_cast<int *>(header + 0x38) = transfer_header_vehicle_count;
            *reinterpret_cast<int *>(header + 0x3C) = transfer_header_base_count;
        }
    }
    if (GameDataWriteMode == 0 && size == 4) memset(buffer, 0, 4);
    if (size == 0xC0) memset(buffer, 0xA5, size);
    if (size == 0x105E0) {
        memset(buffer, 0xCD, size);
        unsigned char *const bytes = static_cast<unsigned char *>(buffer);
        bytes[0x458] = 0x31;
        bytes[0x459] = 0x32;
        bytes[0x45A] = 0x33;
        bytes[0x45B] = 0x34;
        bytes[0x45C] = 0x35;
        *reinterpret_cast<int *>(bytes + 0x460) = 0x12345678;
        *reinterpret_cast<int *>(bytes + 0x464) = 0x23456789;
        *reinterpret_cast<int *>(bytes + 0x468) = 0x3456789A;
        bytes[0x854] = 0xA1;
        bytes[0xA54] = 0xA2;
        *reinterpret_cast<short *>(bytes + 0xC54) = static_cast<short>(0xBEEF);
        *reinterpret_cast<int *>(bytes + 0x7F0) = 0x456789AB;
        *reinterpret_cast<unsigned int *>(bytes + 0x85C) = 0x56789ABC;
        bytes[0x19F8] = 0x71;
        bytes[0x1EA8] = 0x72;
        *reinterpret_cast<unsigned int *>(bytes + 0x2058) = 0x6789ABCD;
    }
    if (size == 0xE5E0) {
        memset(buffer, 0x61, size);
        unsigned char *const bytes = static_cast<unsigned char *>(buffer);
        *reinterpret_cast<int *>(bytes + 0x460) = 0x456789AB;
        bytes[0x854] = 0x7A;
        bytes[0x954] = 0x7B;
        bytes[0xA54] = 0x7C;
    }
    if (size == 0x30) {
        unsigned char *const bytes = static_cast<unsigned char *>(buffer);
        memset(bytes, 0, size);
        *reinterpret_cast<int16_t *>(bytes + 0x00) = 0x1111;
        *reinterpret_cast<int16_t *>(bytes + 0x02) = 0x2222;
        *reinterpret_cast<uint32_t *>(bytes + 0x04) = 0x33334444;
        *reinterpret_cast<uint16_t *>(bytes + 0x08) = 0x5555;
        bytes[0x0A] = 7;
        bytes[0x0C] = 8;
        bytes[0x0D] = 9;
        bytes[0x0E] = 10;
        bytes[0x0F] = 11;
        *reinterpret_cast<int16_t *>(bytes + 0x10) = 0x1212;
        *reinterpret_cast<int16_t *>(bytes + 0x18) = 0x3434;
        bytes[0x20] = 12;
        bytes[0x21] = 13;
        bytes[0x22] = 14;
        bytes[0x23] = 15;
        bytes[0x24] = 16;
        bytes[0x25] = 0xF1;
        bytes[0x26] = 17;
        bytes[0x27] = 18;
        bytes[0x28] = 19;
        bytes[0x29] = 20;
        *reinterpret_cast<int16_t *>(bytes + 0x2A) = 0x2121;
        *reinterpret_cast<int16_t *>(bytes + 0x2C) = 0x2223;
        *reinterpret_cast<int16_t *>(bytes + 0x2E) = 0x2425;
    }
    if (size == 0x130) {
        unsigned char *const bytes = static_cast<unsigned char *>(buffer);
        memset(bytes, 0, size);
        *reinterpret_cast<int *>(bytes + 0x4C) = 1;
        *reinterpret_cast<int *>(bytes + 0x50) = -34;
        *reinterpret_cast<int *>(bytes + 0x54) = 5;
        *reinterpret_cast<unsigned short *>(bytes + 0x10C) = 0x1122;
        *reinterpret_cast<unsigned short *>(bytes + 0x10E) = 0x3344;
        *reinterpret_cast<unsigned short *>(bytes + 0x110) = 0x5566;
        *reinterpret_cast<unsigned int *>(bytes + 0x114) = 0x12345678;
        *reinterpret_cast<unsigned int *>(bytes + 0x118) = 0x23456789;
        *reinterpret_cast<unsigned int *>(bytes + 0x11C) = 0x3456789A;
        *reinterpret_cast<unsigned int *>(bytes + 0x120) = 0x456789AB;
        *reinterpret_cast<unsigned int *>(bytes + 0x124) = 0x56789ABC;
        *reinterpret_cast<unsigned int *>(bytes + 0x128) = 0x6789ABCD;
        *reinterpret_cast<unsigned int *>(bytes + 0x12C) = 0x789ABCDE;
    }
    return 1;
}
int __cdecl game_data_plain_transfer(FILE *file, void *buffer,
                                    unsigned int size, int write_mode) {
    ++plain_transfer_count;
    assert((write_mode != 0) == (GameDataWriteMode != 0));
    if (plain_read_from_stream && write_mode == 0)
        return static_cast<int>(fread(buffer, size, 1, file));
    return map_data_block(file, buffer, size);
}
static int veh_game_data_bunglist(FILE *file, int version) {
    return version >= 6 ? map_data_block(file, 0, 0x1000) : 1;
}
/* ACTUAL_GAME_DATA_IMPLEMENTATION */

static void reset_trace() {
    transfer_steps.clear();
    plain_transfer_count = 0;
    fail_at_step = -1;
    read_scenario_from_stream = 0;
    read_header_from_stream = 0;
    read_state_from_stream = 0;
    plain_read_from_stream = 0;
    GameDataWriteMode = 0;
    GameDataEncryptedMode = 0;
    GameDataLoadStatusWord = 0;
    GameDataPlayerCountSnapshot = 0;
    GameDataLoadRequest = 0;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
    GameDataStateStorage.fields_.game_more_preferences_ = 0;
    GameDataStateStorage.fields_.net_turn_flags_ = 0;
    ReplayEventBytes = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    map_read_calls = 0;
    map_write_calls = 0;
    transfer_header_version = 12;
    transfer_header_vehicle_count = 0;
    transfer_header_base_count = 0;
}

static int has_transfer_size(unsigned int size) {
    for (size_t index = 0; index < transfer_steps.size(); ++index)
        if (transfer_steps[index].size == size) return 1;
    return 0;
}

int main() {
    FILE *file = tmpfile();
    assert(file != 0);

    reset_trace();
    GameDataStateStorage.fields_.game_state_ = 0;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps.size() == 74);
    assert(transfer_steps[0].size == 0x31C);
    assert(transfer_steps[1].size == 0x398);  // initial v11+ state record
    assert(transfer_steps[2].size == 4);      // plain current faction
    assert(transfer_steps[3].size == 0x10660); // current PlayerData array
    assert(transfer_steps[4].size == 0x2CE0);  // Player records
    assert(transfer_steps[5].size == 0x20);    // ranking words
    assert(map_read_calls == 1);
    assert(map_write_calls == 0);
    assert(transfer_steps[6].size == 16000);
    assert(transfer_steps[7].size == 4);
    assert(transfer_steps[8].size == 0xE00);
    assert(transfer_steps[9].size == 0x6800);
    assert(transfer_steps[10].size == 0x1000); // owner-local BungList block
    assert(transfer_steps[11].size == 0x100);  // save name
    assert(transfer_steps[12].size == 0x98);   // scenario/objective record
    assert(transfer_steps[13].size == 0x27A0); // faction monuments
    assert(transfer_steps[14].size == 0x20);   // tectonic counters
    assert(transfer_steps[15].size == 4);      // cursor active
    assert(transfer_steps[16].size == 4);      // selected vehicle
    assert(transfer_steps[17].size == 4);      // one history X cell
    assert(transfer_steps[18].size == 4);      // one history Y cell
    assert(transfer_steps[19].size == 4);      // Console private word API
    assert(transfer_steps[20].size == 0x100);  // PendingDiplomacyFlags
    assert(transfer_steps[21].size == 0x20);   // faction combat wins
    assert(transfer_steps[22].size == 0x20);   // faction combat losses
    for (int list_step = 23; list_step < 58; ++list_step)
        assert(transfer_steps[list_step].size == 4);
    assert(transfer_steps[58].buffer == &CouncilWindow.field_A1C_);
    assert(transfer_steps[62].buffer == CouncilWindow.leader_label_kind_);
    assert(transfer_steps[63].buffer == CouncilWindow.leader_tech_);
    assert(transfer_steps[64].buffer == &CouncilPbemGate);
    assert(transfer_steps[72].size == 4);  // saved screen width local
    assert(transfer_steps[73].size == 4);  // saved screen height local
    assert(GameDataEncryptedMode == 0);
    assert(plain_transfer_count == 35);

    reset_trace();
    PlayersData[1].unk_102 = 0x11223344;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ =
        (1u << 9) | (1u << 10) | (1u << 1) | (1u << 2);
    assert(game_data(file, 0) == 1);
    assert(GameDataLoadStatusWord == 1);
    assert(map_read_calls == 0);
    assert(transfer_steps.size() == 6);
    PlayersData[1].unk_102 = 0;

    reset_trace();
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.game_more_preferences_ = 0x20;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps.size() == 74);
    for (size_t step = 0; step < transfer_steps.size(); ++step)
        assert(transfer_steps[step].size != 0x2400);

    reset_trace();
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.game_more_preferences_ = 0;
    GameDataStateStorage.fields_.net_turn_flags_ = 0x20;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps.size() == 75);
    assert(transfer_steps[20].size == 0x2400);
    assert(transfer_steps[21].size == 0x100);
    assert(transfer_steps[22].size == 0x20);
    assert(transfer_steps[23].size == 0x20);

    reset_trace();
    GameDataStateStorage.fields_.game_state_ = 0x10000000u;
    for (int index = 0; index < 6; ++index)
        TimeControl[index].name = reinterpret_cast<char *>(0x1000 + index * 4);
    assert(game_data(file, 0) == 0);
    assert(transfer_steps.size() == 78);
    assert(plain_transfer_count == 35);
    assert(transfer_steps[0].size == 0x31C);
    assert(transfer_steps[1].size == 0x398);
    assert(transfer_steps[2].size == 0x398);  // encrypted duplicate record
    assert(transfer_steps[3].size == 0xCE0);
    assert(transfer_steps[4].size == 4);
    assert(transfer_steps[5].size == 0xC0);
    assert(transfer_steps[6].size == 4);
    assert(transfer_steps[7].size == 0x10660);
    assert(transfer_steps[8].size == 0x2CE0);
    assert(transfer_steps[9].size == 0x20);
    assert(transfer_steps[10].size == 16000);
    assert(transfer_steps[11].size == 4);
    assert(transfer_steps[12].size == 0xE00);
    assert(transfer_steps[13].size == 0x6800);
    assert(transfer_steps[14].size == 0x1000);
    assert(transfer_steps[15].size == 0x100);
    assert(transfer_steps[16].size == 0x98);
    assert(transfer_steps[17].size == 0x27A0);
    assert(transfer_steps[18].size == 0x20);
    assert(transfer_steps[19].size == 4);
    assert(transfer_steps[20].size == 4);
    assert(transfer_steps[21].size == 4);
    assert(transfer_steps[22].size == 4);
    assert(transfer_steps[23].size == 4);
    assert(transfer_steps[24].size == 0x100);
    assert(transfer_steps[25].size == 0x20);
    assert(transfer_steps[26].size == 0x20);
    for (int list_step = 27; list_step < 62; ++list_step)
        assert(transfer_steps[list_step].size == 4);
    for (int index = 0; index < 6; ++index)
        assert(TimeControl[index].name ==
               reinterpret_cast<char *>(0x1000 + index * 4));

    reset_trace();
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 2;
    GameDataStateStorage.fields_.base_current_count_ = 3;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps.size() == 76);
    assert(transfer_steps[10].size == 0x1000);
    assert(transfer_steps[11].buffer == Vehs);
    assert(transfer_steps[11].size == 2 * 0x34);
    assert(transfer_steps[12].buffer == Bases);
    assert(transfer_steps[12].size == 3 * 0x134);

    reset_trace();
    GameDataStateStorage.fields_.game_state_ = 0;
    char pending_message[] = "hello";
    MessageWinDeferredBaseMessages[2].pending_text_ = pending_message;
    MessageWinDeferredBaseMessages[2].add(42);
    reset_trace();
    GameDataWriteMode = 1;
    assert(game_data_candidate_string_list(
               file, MessageWinDeferredBaseMessages[2], 1) == 0);
    assert(transfer_steps.size() == 4);
    assert(transfer_steps[0].size == 4);  // count
    assert(transfer_steps[1].size == 4);  // entry id
    assert(transfer_steps[2].size == 4);  // text length
    assert(transfer_steps[3].size == 5);  // text bytes

    // The IO mock must only observe write buffers. This concrete initialized
    // control is safe to inspect after transfer; it is deliberately separate
    // from game_data's indeterminate native header on the write path.
    unsigned char write_sentinel[8];
    unsigned char write_snapshot[8];
    memset(write_sentinel, 0xB7, sizeof(write_sentinel));
    memcpy(write_snapshot, write_sentinel, sizeof(write_snapshot));
    reset_trace();
    GameDataWriteMode = 1;
    assert(map_data_block(file, write_sentinel, sizeof(write_sentinel)) == 1);
    assert(transfer_steps.size() == 1);
    assert(memcmp(write_sentinel, write_snapshot, sizeof(write_snapshot)) == 0);

    reset_trace();
    MessageWinDeferredBaseMessages[2].remove_all();
    MessageWinDeferredBaseMessages[2].pending_text_ = 0;
    char diplomacy_message[] = "tri";
    DiplomacyTranscriptLists[1][4].pending_text_ = diplomacy_message;
    DiplomacyTranscriptLists[1][4].add(19);
    reset_trace();
    GameDataWriteMode = 1;
    assert(game_data_candidate_string_list(
               file, DiplomacyTranscriptLists[1][4], 1) == 0);
    assert(transfer_steps.size() == 4);
    assert(transfer_steps[0].size == 4); // count
    assert(transfer_steps[1].size == 4); // entry id
    assert(transfer_steps[2].size == 4); // text length
    assert(transfer_steps[3].size == 3); // text bytes

    reset_trace();
    MapWin primary_map = {0x1234567};
    MapWin loaded_map = {0x12345678};
    MapWinTable[0] = &primary_map;
    MapWinTable[1] = &loaded_map;
    IfaceScreenWidth = 800;
    IfaceScreenHeight = 600;
    assert(game_data(file, 0) == 0);
    assert(primary_map.field_1DD74_ == 0x1234567);
    assert(loaded_map.field_1DD74_ == 0);
    MapWinTable[0] = 0;
    MapWinTable[1] = 0;
    IfaceScreenWidth = 0;
    IfaceScreenHeight = 0;

    reset_trace();
    transfer_header_version = 3;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps.size() == 70);
    assert(transfer_steps[0].size == 0x31C);
    assert(transfer_steps[1].size == 4);       // old-version faction word
    assert(transfer_steps[2].size == 0x105E0); // v3..10 wire array
    assert(transfer_steps[3].size == 0x2CE0);
    assert(transfer_steps[4].size == 0x20);
    assert(transfer_steps[5].size == 16000);
    assert(transfer_steps[6].size == 4);
    assert(transfer_steps[7].size == 0xE00);
    assert(transfer_steps[8].size == 0x6800);
    assert(transfer_steps[9].size == 0x100);
    assert(transfer_steps[10].size == 0x98);
    assert(transfer_steps[11].size == 4);
    assert(transfer_steps[12].size == 4);
    assert(transfer_steps[13].size == 4);
    assert(transfer_steps[14].size == 4);
    assert(transfer_steps[15].size == 4);
    assert(map_read_calls == 1);
    assert(PlayersData[0].facility_announced[0] == 0x31);
    assert(PlayersData[0].facility_announced[4] == 0x35);
    assert(PlayersData[0].facility_announced[5] == 0);
    assert(PlayersData[0].facility_announced[8] == 0);
    assert(PlayersData[0].planet_ecology == 0x12345678);
    assert(PlayersData[0].base_id_atk_target == 0x23456789);
    assert(PlayersData[0].unk_37 == 0x3456789A);
    assert(PlayersData[0].proto_id_active[0] == 0xA1);
    assert(PlayersData[0].proto_id_queue[0] == 0xA2);
    assert(static_cast<unsigned short>(PlayersData[0].proto_id_lost[0]) ==
           0xBEEF);
    assert(PlayersData[0].unk_47 == 0x456789AB);
    assert(PlayersData[0].nutrient_surplus_total == 0x56789ABC);
    assert(PlayersData[0].goals[0].bytes_[0] == 0x71);
    assert(PlayersData[0].sites[0].bytes_[0] == 0x72);
    assert(PlayersData[0].secret_project_intel[0] == 0x6789ABCD);
    assert(PlayersData[0].secret_project_intel[5] == 0);
    assert(VehPrototypes[8].icon_offset == 3);
    assert(VehPrototypes[7].icon_offset == 2);

    reset_trace();
    transfer_header_version = 2;
    transfer_header_vehicle_count = 1;
    transfer_header_base_count = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    VehPrototypes[32].veh_name[0] = 'x';
    VehPrototypes[32].flags = 0x1234;
    VehPrototypes[32].unk_1 = 7;
    VehPrototypes[32].preq_tech = 9;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps.size() == 79);
    assert(transfer_steps[0].size == 0x31C);
    assert(transfer_steps[1].size == 4);
    assert(transfer_steps[2].size == 0xE5E0);  // old rows expand in memory
    assert(transfer_steps[3].size == 0x2CE0);
    assert(transfer_steps[4].size == 0x20);
    assert(transfer_steps[5].size == 16000);
    assert(transfer_steps[6].size == 4);
    assert(transfer_steps[7].size == 0xE00);
    for (int prototype_step = 8; prototype_step < 16; ++prototype_step)
        assert(transfer_steps[prototype_step].size == 0x680);
    assert(VehPrototypes[32].veh_name[0] == '\0');
    assert(VehPrototypes[32].flags == 0);
    assert(VehPrototypes[32].unk_1 == 0);
    assert(VehPrototypes[32].preq_tech == 9);
    assert(transfer_steps[16].size == 0x30);
    assert(transfer_steps[17].size == 0x130);
    assert(Vehs[0].x == 0x1111);
    assert(Vehs[0].y == 0x2222);
    assert(Vehs[0].state == 0x33334444);
    assert(Vehs[0].flags == 0x5555);
    assert(Vehs[0].proto_id == 7);
    assert(Vehs[0].unk_1 == 0);
    assert(Vehs[0].faction_id == 7);
    assert(Vehs[0].year_end_lurking == 0);
    assert(Vehs[0].dmg_incurred == 8);
    assert(Vehs[0].order == 9);
    assert(Vehs[0].waypoint_count == 10);
    assert(Vehs[0].waypoint_x[0] == 0x1212);
    assert(Vehs[0].waypoint_y[0] == 0x3434);
    assert(Vehs[0].morale == 12);
    assert(Vehs[0].terraforming_turns == 13);
    assert(Vehs[0].order_auto_type == 14);
    assert(Vehs[0].facing == static_cast<int8_t>(0xF1));
    assert(map_read_calls == 1);
    assert(PlayersData[0].planet_ecology == 0x456789AB);
    assert(PlayersData[0].proto_id_active[0] == 0x7A);
    assert(PlayersData[0].proto_id_queue[0] == 0x7B);
    assert(PlayersData[0].proto_id_lost[0] == 0x617C);
    assert(Bases[0].queue_production_id[0] == -34 - 31);
    assert(Bases[0].queue_production_id[1] == 5);
    assert(Bases[0].spec[0] == 0x1122u);
    assert(Bases[0].spec[1] == 0x55663344u);
    assert(Bases[0].facilities_built[0] == 0x78);
    assert(Bases[0].facilities_built[4] == 0x89);
    assert(Bases[0].facilities_built[8] == 0x9A);

    reset_trace();
    transfer_header_version = 2;
    transfer_header_vehicle_count = 1;
    transfer_header_base_count = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    fail_at_step = 17;  // legacy Base read after the Veh migration has run
    const int retained_base_x = Bases[0].x;
    assert(game_data(file, 0) == 1);
    assert(transfer_steps.size() == 18);
    assert(Vehs[0].x == 0x1111);  // no rollback of earlier conversion
    assert(Bases[0].x == retained_base_x);  // failed row is not converted

    reset_trace();
    fail_at_step = 1;
    GameDataStateStorage.fields_.game_state_ = 0;
    assert(game_data(file, 0) == 1);
    assert(transfer_steps.size() == 2);  // immediate error return

    // The real single-record fread may mutate a prefix and still report
    // failure. Confirm that the serializer leaves that prefix in place and
    // does not roll back or split the ScenarioSave transfer.
    FILE *partial_file = tmpfile();
    assert(partial_file != 0);
    unsigned char scenario_prefix[37];
    for (unsigned int index = 0; index < sizeof(scenario_prefix); ++index)
        scenario_prefix[index] = static_cast<unsigned char>(0x40 + index);
    assert(fwrite(scenario_prefix, sizeof(scenario_prefix), 1, partial_file) == 1);
    rewind(partial_file);
    memset(&ScenarioSave, 0xCC, sizeof(ScenarioSave));
    reset_trace();
    read_scenario_from_stream = 1;
    GameDataStateStorage.fields_.game_state_ = 0;
    assert(game_data(partial_file, 0) == 1);
    assert(transfer_steps.size() == 13);
    assert(transfer_steps[12].buffer == &ScenarioSave);
    assert(transfer_steps[12].size == 0x98);
    const unsigned char *const scenario_bytes =
        reinterpret_cast<const unsigned char *>(&ScenarioSave);
    assert(memcmp(scenario_bytes, scenario_prefix,
                  sizeof(scenario_prefix)) == 0);
    for (unsigned int index = sizeof(scenario_prefix); index < 0x98; ++index)
        assert(scenario_bytes[index] == 0xCC);
    fclose(partial_file);

    // A short live v11+ state read uses real fread semantics: fread mutates
    // the prefix before returning zero items, and game_data returns without
    // rolling that prefix back.
    FILE *partial_state_file = tmpfile();
    assert(partial_state_file != 0);
    unsigned char state_prefix[53];
    for (unsigned int index = 0; index < sizeof(state_prefix); ++index)
        state_prefix[index] = static_cast<unsigned char>(0x80 + index);
    assert(fwrite(state_prefix, sizeof(state_prefix), 1, partial_state_file) == 1);
    rewind(partial_state_file);
    memset(&GameDataStateStorage, 0xCC, sizeof(GameDataStateStorage));
    reset_trace();
    read_state_from_stream = 1;
    GameDataStateStorage.fields_.game_state_ = 0;
    // The state sentinel is intentionally restored after setting the branch
    // selector because the mock header is supplied separately from this file.
    memset(&GameDataStateStorage, 0xCC, sizeof(GameDataStateStorage));
    assert(game_data(partial_state_file, 0) == 1);
    assert(transfer_steps.size() == 2);
    assert(transfer_steps[1].buffer == &GameDataStateStorage);
    assert(transfer_steps[1].size == 0x398);
    assert(memcmp(&GameDataStateStorage, state_prefix,
                  sizeof(state_prefix)) == 0);
    const unsigned char *const state_bytes =
        reinterpret_cast<const unsigned char *>(&GameDataStateStorage);
    for (unsigned int index = sizeof(state_prefix); index < 0x398; ++index)
        assert(state_bytes[index] == 0xCC);
    fclose(partial_state_file);

    // A short first legacy-header read fails before the native copy from the
    // local header into GameDataStateStorage. Its live state remains intact.
    FILE *partial_header_file = tmpfile();
    assert(partial_header_file != 0);
    unsigned char header_prefix[41];
    for (unsigned int index = 0; index < sizeof(header_prefix); ++index)
        header_prefix[index] = static_cast<unsigned char>(0x20 + index);
    assert(fwrite(header_prefix, sizeof(header_prefix), 1, partial_header_file) == 1);
    rewind(partial_header_file);
    reset_trace();
    memset(&GameDataStateStorage, 0x6D, sizeof(GameDataStateStorage));
    unsigned char retained_state[0x398];
    memcpy(retained_state, &GameDataStateStorage, sizeof(retained_state));
    read_header_from_stream = 1;
    assert(game_data(partial_header_file, 0) == 1);
    assert(transfer_steps.size() == 1);
    assert(transfer_steps[0].size == 0x31C);
    assert(memcmp(&GameDataStateStorage, retained_state,
                  sizeof(retained_state)) == 0);
    fclose(partial_header_file);

    // Native string-list reading calls fread even for a zero text length.
    // Standard fread(buffer, 0, 1, file) returns zero, so the candidate exits
    // before adding that entry. This directly invokes the extracted candidate
    // helper, not a reimplementation of its parser.
    FILE *empty_string_file = tmpfile();
    assert(empty_string_file != 0);
    const int empty_string_record[3] = {1, 73, 0};
    assert(fwrite(empty_string_record, sizeof(empty_string_record), 1,
                  empty_string_file) == 1);
    rewind(empty_string_file);
    StringList empty_string_list = {};
    reset_trace();
    plain_read_from_stream = 1;
    assert(game_data_candidate_string_list(empty_string_file,
                                           empty_string_list, 0) == 1);
    assert(plain_transfer_count == 4);
    assert(empty_string_list.entry_count_ == 0);
    fclose(empty_string_file);

    // Explicit threshold controls around the distinct legacy/current and
    // owner/monument gates. These use synthetic read headers only; the write
    // path above is intentionally never given synthetic header bytes.
    reset_trace();
    transfer_header_version = 5;
    assert(game_data(file, 0) == 0);
    assert(!has_transfer_size(0x1000)); // BungList owner is version >= 6

    reset_trace();
    transfer_header_version = 6;
    monuments_clear_calls = 0;
    assert(game_data(file, 0) == 0);
    assert(has_transfer_size(0x1000));
    assert(!has_transfer_size(0x27A0));
    assert(monuments_clear_calls == 1);

    reset_trace();
    transfer_header_version = 7;
    monuments_clear_calls = 0;
    assert(game_data(file, 0) == 0);
    assert(has_transfer_size(0x1000));
    assert(has_transfer_size(0x27A0));
    assert(monuments_clear_calls == 0);

    reset_trace();
    transfer_header_version = 10;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps[1].size == 4);
    assert(transfer_steps[2].size == 0x105E0);
    assert(!has_transfer_size(0x10660));

    reset_trace();
    transfer_header_version = 11;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps[1].size == 0x398);
    assert(transfer_steps[3].buffer == PlayersData);
    assert(transfer_steps[3].size == 0x10660);

    fclose(file);
    return 0;
}
