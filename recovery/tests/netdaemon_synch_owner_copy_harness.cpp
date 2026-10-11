#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct BaseSecretProject { int words_[64]; };
#include "src/game_data_state_window.h"

typedef int BOOL;
static const int MaxPlayerNum = 8;
static const int MaxVehProtoNum = 512;
static const int MaxBaseNum = 512;
static const int MaxContinentNum = 128;
static const int MaxRankingHistoryTurns = 1000;
/* MAP STORAGE DECLARATIONS */
struct Continent { unsigned char bytes[28]; };
struct SocialCategory { uint32_t models_[4]; };
struct ScenarioSaveState {
    int objective_req_victory;
    uint32_t objectives_sudden_death_victory, objective_achieve_points;
    uint32_t victory_achieve_bonus_points, starting_mission_year, ending_mission_year;
    uint8_t wire_opaque_94b4d8[0x80];
};
ScenarioSaveState ScenarioSave;
#define GameWireOpaque_94B4D8 ScenarioSave.wire_opaque_94b4d8
struct Player { unsigned char bytes[0x59C]; };
struct PlayerData {
    uint32_t flags;
    uint32_t diplo_treaties[8];
    int diplo_agenda[8], diplo_spoke[8], diplo_merc[8];
    uint32_t diplo_friction[8];
    char diplo_patience[8];
    int global_reputation, diplo_unk1[8], diplo_wrongs[8];
    unsigned char pad_004[0x130];
    int energy_reserves;
    unsigned char pad_224[4];
    SocialCategory soc_category_pending;
    SocialCategory soc_category_active;
    unsigned char pad_248[0xDC];
    int se_alloc_psych, se_alloc_labs;
    unsigned char pad_32C[0x9C];
    int tech_accumulated, tech_id_researching;
    unsigned char pad_3D0[12];
    int ai_fight;
    BOOL ai_growth, ai_tech, ai_wealth, ai_power;
    unsigned char pad_3E8[0x7C];
    int unk_37;
    char saved_queue_name[8][24];
    int saved_queue_size[8];
    int saved_queue_items[8][10];
    int unk_38[8];
    unsigned char tail[0x20CC - 0x6B0];
};
struct VehPrototype {
    char veh_name[32]; uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id, carry_capacity,
        cost, plan; int8_t unk_1; uint8_t obsolete_factions, combat_factions,
        icon_offset, padding; uint16_t flags; int16_t preq_tech;
};
typedef char PlayerSyncPointerStartsAtNativeOffset[
    offsetof(PlayerData, unk_37) == 0x46C ? 1 : -1];
typedef char PlayerQueueNamesFollowNativeStart[
    offsetof(PlayerData, saved_queue_name) == 0x470 ? 1 : -1];
struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t unk_1;
    uint8_t faction_id, year_end_lurking, dmg_incurred;
    int8_t order;
    uint8_t rest[0x34 - 0x12];
};
struct Base {
    unsigned char prefix[0x78];
    int resource_sqr_radius, specialist_total, specialist_unk_1;
    uint32_t spec[2];
    unsigned char tail[0x134 - 0x8C];
};
struct ReplayEventRecord { unsigned char bytes[8]; };
struct ProtoBung {
    uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
};
static ProtoBung BungList[8][64];

uint32_t LockEnableMask;
char NetDaemonFileTransferPath[0x104];
uint32_t MapLongitude;
Continent Continents[MaxContinentNum];
Player Players[MaxPlayerNum];
PlayerData PlayersData[MaxPlayerNum];
VehPrototype VehPrototypes[MaxVehProtoNum];
Veh Vehs[2049];
Base Bases[MaxBaseNum];
int ControlTurnPhase;
uint32_t FactionRankingsUnk[MaxPlayerNum];
uint16_t FactionRankingHistory[MaxPlayerNum * MaxRankingHistoryTurns];
int ReplayEventBytes;
ReplayEventRecord ReplayEvents[1024];
#define ObjectiveReqVictory ScenarioSave.objective_req_victory
#define ObjectivesSuddenDeathVictory ScenarioSave.objectives_sudden_death_victory
#define ObjectiveAchievePts ScenarioSave.objective_achieve_points
#define VictoryAchieveBonusPts ScenarioSave.victory_achieve_bonus_points
#define StartingMissionYear ScenarioSave.starting_mission_year
#define EndingMissionYear ScenarioSave.ending_mission_year
GameDataStateWindow GameDataStateStorage = {};
#define GamePreferences GameDataStateStorage.fields_.game_preferences_
#define GameMorePreferences GameDataStateStorage.fields_.game_more_preferences_
#define GameWarnings GameDataStateStorage.fields_.game_warnings_
#define GameRules GameDataStateStorage.fields_.game_rules_
#define GameWireOpaque_9A64A0 GameDataStateStorage.fields_.game_wire_opaque_9a64a0_
#define GameWireOpaque_9A64A8 GameDataStateStorage.fields_.game_wire_opaque_9a64a8_
#define GameWireOpaque_9A67C9 GameDataStateStorage.fields_.game_wire_opaque_9a67c9_
#define LegacyGameSaveName GameDataStateStorage.fields_.legacy_game_save_name_
#define EcologyEventAccumulator GameDataStateStorage.fields_.ecology_event_accumulator_
#define GameInterludeFlags GameDataStateStorage.fields_.game_interlude_flags_
#define TutorialFlags GameDataStateStorage.fields_.tutorial_flags_
#define TutorialFlags2 GameDataStateStorage.fields_.tutorial_flags_2_
#define TerraformAnnouncedMask GameDataStateStorage.fields_.terraform_announced_mask_
#define TutWinFlags9A64BC GameDataStateStorage.fields_.tutwin_flags_9a64bc_
#define GameState GameDataStateStorage.fields_.game_state_
#define VehCurrentCount GameDataStateStorage.fields_.vehicle_current_count_
#define BaseCurrentCount GameDataStateStorage.fields_.base_current_count_
#define DiffLevelCurrent GameDataStateStorage.fields_.diff_level_current_
#define GameSetupResetWord GameDataStateStorage.fields_.game_setup_reset_word_
#define TurnCurrentNum GameDataStateStorage.fields_.turn_current_num_
#define MissionYearCurrent GameDataStateStorage.fields_.mission_year_current_
#define VehSelectionFollowLimit GameDataStateStorage.fields_.vehicle_selection_follow_limit_
#define TimeControlsSelection GameDataStateStorage.fields_.time_controls_selection_
#define TurnControlDurationMilliseconds GameDataStateStorage.fields_.turn_control_duration_milliseconds_
#define FactionRankings GameDataStateStorage.fields_.faction_rankings_
#define RankingFactionIDUnk1 GameDataStateStorage.fields_.ranking_faction_id_unk1_
#define RankingFactionIDUnk2 GameDataStateStorage.fields_.ranking_faction_id_unk2_
#define SecretProject GameDataStateStorage.fields_.secret_project_
#define GovernorFaction GameDataStateStorage.fields_.governor_faction_
#define CouncilResolutionCount GameDataStateStorage.fields_.council_resolution_count_
#define CouncilLastResolutionTurn GameDataStateStorage.fields_.council_last_resolution_turn_
#define GameTechAchieved GameDataStateStorage.fields_.game_tech_achieved_
int ClimateRiseStep, ClimateRiseLimit, ClimateRiseAccum, ClimateSeaLevelDelta;
int TranscendenceScoreBonus;
uint32_t TranscendenceScoreWeights[8];
int SunspotDuration;
uint32_t MountPlanetX, MountPlanetY;
int DustCloudDuration, SaveDaemonStateWord;
uint32_t ScnVictFacilityObj, GlobalEnergyEventState, NetTurnFlags;

/* SOURCE EXTRACTS */

static void fill(unsigned char *p, size_t n, unsigned char v) { memset(p, v, n); }

static void route_roundtrip(int16_t opcode, int a2, int a3,
                            size_t payload_size, size_t offset,
                            unsigned char *selected_owner,
                            size_t count, bool mov_overwrites_offset = false) {
    unsigned char packet[64];
    assert(count <= sizeof packet);
    for (size_t i = 0; i < count; ++i)
        selected_owner[mov_overwrites_offset ? 0 : offset + i] =
            static_cast<unsigned char>((i * 17u + opcode + 0x31u) & 0xFFu);
    unsigned char *const expected = selected_owner +
        (mov_overwrites_offset ? 0 : offset);
    const size_t effective_offset = mov_overwrites_offset ? 0 : offset;
    fill(packet, count, 0xA5);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(opcode, a2, a3, 0,
        payload_size, offset, packet, count, false, 0, 0, 0));
    assert(memcmp(packet, expected, count) == 0);
    fill(packet, count, 0xB6);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(opcode, a2, a3, 0,
        payload_size, offset, packet, count, true, 0, 0, 0));
    for (size_t i = 0; i < count; ++i) assert(expected[i] == 0xB6);
    (void)effective_offset;
}

static void metadata_routes() {
    assert(sizeof(MapMetadataFields) == 0xA9C);
    assert(offsetof(MapStorageWindow, metadata_) == 0);
    // Host pointer alignment may differ. These controls establish host
    // behavior within the actual owner, not the original i386 pointer ABI.
    assert(sizeof(void *) != 4 || offsetof(MapStorageWindow, tiles_) == 0xA9C);
    assert(sizeof(void *) != 4 || sizeof(MapStorageWindow) == 0xAA4);
    unsigned char *const owner = reinterpret_cast<unsigned char *>(&MapStorage);
    unsigned char expected[sizeof MapStorage];
    unsigned char packet[0xA9C];
    const int16_t opcodes[] = {0x02, 0x1002};
    for (size_t op = 0; op < 2; ++op) {
        for (size_t i = 0; i < sizeof MapStorage; ++i)
            owner[i] = static_cast<unsigned char>(i * 17u + 9u);
        memcpy(expected, owner, sizeof expected);
        assert(NetDaemonSyncWire::copy_nonrule_chunk(opcodes[op], 0, 0, 0,
            0xA9C, 0, packet, sizeof packet, false, 0, 0, 0));
        assert(memcmp(packet, expected, sizeof packet) == 0);
        assert(memcmp(owner, expected, sizeof expected) == 0);

        // Nominal network chunks cover the entire metadata prefix and leave
        // both pointer cells (and any host padding) unchanged.
        for (size_t i = 0; i < sizeof packet; ++i)
            packet[i] = static_cast<unsigned char>(i * 23u + 0x41u);
        memcpy(expected, packet, sizeof packet);
        assert(NetDaemonSyncWire::copy_nonrule_chunk(opcodes[op], 0, 0, 0,
            0xA9C, 0, packet, sizeof packet, true, 0, 0, 0));
        assert(memcmp(owner, expected, sizeof expected) == 0);
        for (size_t chunk = 0; chunk < 2; ++chunk) {
            const size_t offset = chunk * 0x800;
            const size_t count = chunk == 0 ? 0x800 : 0x29C;
            assert(NetDaemonSyncWire::copy_nonrule_chunk(opcodes[op], 0, 0, 0,
                0xA9C, offset, packet + offset, count, true, 0, 0, 0));
            memset(packet + offset, 0, count);
            assert(NetDaemonSyncWire::copy_nonrule_chunk(opcodes[op], 0, 0, 0,
                0xA9C, offset, packet + offset, count, false, 0, 0, 0));
            assert(memcmp(packet + offset, expected + offset, count) == 0);
        }
        assert(memcmp(owner, expected, sizeof expected) == 0);
        // Cross landmarks->abstract dimensions, rules->path, and final edge.
        for (size_t i = 0; i < 3; ++i) {
            const size_t offsets[] = {0xA20, 0xA48, 0xA98};
            route_roundtrip(opcodes[op], 0, 0, 0xA9C, offsets[i], owner,
                            i == 2 ? 4 : 16);
        }
        // Native address selection ignores the nominal payload boundary.
        // Stay inside the whole owner; no neighboring allocation/fault effects
        // are represented by this host-only malformed-transfer control.
        route_roundtrip(opcodes[op], 0, 0, 0xA9C, 0xA98, owner, 8);
        route_roundtrip(opcodes[op], 0, 0, 0xA9C,
            offsetof(MapStorageWindow, tiles_), owner, sizeof(MapStorage.tiles_));
        for (int receive = 0; receive < 2; ++receive) {
            for (int reverse = 0; reverse < 2; ++reverse) {
                const size_t source = reverse ? 0xA00 : 0x9C0;
                const size_t destination = reverse ? 0x9C0 : 0xA00;
                memcpy(expected, owner, sizeof expected);
                memmove(expected + destination, expected + source, 0x80);
                assert(NetDaemonSyncWire::copy_nonrule_chunk(opcodes[op], 0, 0, 0,
                    0xA9C, receive ? destination : source,
                    owner + (receive ? source : destination), 0x80,
                    receive != 0, 0, 0, 0));
                assert(memcmp(owner, expected, sizeof expected) == 0);
            }
        }
    }
}

static void map_allocation_routes() {
    Map allocation[256];
    Map redirected[256];
    MapTiles = redirected;
    MapArea = 1;
    unsigned char *const owner = reinterpret_cast<unsigned char *>(&allocation);
    unsigned char expected[sizeof allocation];
    unsigned char packet[0x800];
    memset(redirected, 0xD4, sizeof redirected);
    const int16_t opcodes[] = {0x04, 0x30};
    for (size_t op = 0; op < 2; ++op) {
        const uint32_t base = op == 0 ? 0 : 8 * 44u;
        for (size_t i = 0; i < sizeof allocation; ++i)
            owner[i] = static_cast<unsigned char>(i * 11u + 0x19u);
        memcpy(expected, owner, sizeof expected);
        // Native selects the captured allocation, not the newly rebound
        // global. A chunk crossing records and the advertised payload stays
        // within this real fixture's lifetime/extent; MapArea is not capacity.
        assert(NetDaemonSyncWire::copy_nonrule_chunk(opcodes[op], 0, 0, 0,
            44, 0x800, packet, sizeof packet, false, 0, allocation, base));
        assert(memcmp(packet, owner + base + 0x800, sizeof packet) == 0);
        memset(packet, 0x83, sizeof packet);
        memset(expected + base + 0x800, 0x83, sizeof packet);
        assert(NetDaemonSyncWire::copy_nonrule_chunk(opcodes[op], 0, 0, 0,
            44, 0x800, packet, sizeof packet, true, 0, allocation, base));
        assert(memcmp(owner, expected, sizeof expected) == 0);
        for (int receive = 0; receive < 2; ++receive) {
            for (int reverse = 0; reverse < 2; ++reverse) {
                const uint32_t source = base + (reverse ? 0x480 : 0x440);
                const uint32_t destination = base + (reverse ? 0x440 : 0x480);
                memcpy(expected, owner, sizeof expected);
                memmove(expected + destination, expected + source, 0x100);
                assert(NetDaemonSyncWire::copy_nonrule_chunk(opcodes[op], 0, 0, 0,
                    44, (receive ? destination : source) - base,
                    owner + (receive ? source : destination), 0x100,
                    receive != 0, 0, allocation, base));
                assert(memcmp(owner, expected, sizeof expected) == 0);
            }
        }
        for (size_t i = 0; i < sizeof redirected; ++i)
            assert(reinterpret_cast<unsigned char *>(&redirected)[i] == 0xD4);
    }
    // Native displacement addition wraps before forming the allocation view.
    // The wrapped result is zero; no out-of-allocation address is executed.
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x30, 0, 0, 0,
        44, 44, packet, 44, false, 0, allocation, 0xFFFFFFD4u));
    assert(memcmp(packet, owner, 44) == 0);
    MapTiles = 0;
}

int main() {
    metadata_routes();
    map_allocation_routes();
    Map tiles[32];
    Map *captured = tiles;
    MapTiles = tiles;
    MapArea = 32;
    MapLongitudeBounds = 8;
    MapLatitudeBounds = 16;
    MapLongitude = 4;

    // Native-offset transfers intentionally cross selected fields and records
    // while staying inside their complete declared source owner. Each check
    // exercises actual extracted product code in both transfer directions.
    route_roundtrip(0x00, 0, 0, 0x398, 0x50,
        reinterpret_cast<unsigned char *>(&GameDataStateStorage), 32);
    route_roundtrip(0x01, 0, 0, 1, 1,
        reinterpret_cast<unsigned char *>(&GameDataStateStorage) +
            offsetof(GameDataStateWindowFields, lock_masks_), 8);
    route_roundtrip(0x05, 0, 0, 0xE00, 0x7F0,
        reinterpret_cast<unsigned char *>(&Continents), 32);
    route_roundtrip(0x07, 0, 0, 0x59C, 0x800,
        reinterpret_cast<unsigned char *>(&Players), 32);
    route_roundtrip(0x08, 2, 0, 0x14, 0x800,
        reinterpret_cast<unsigned char *>(&PlayersData[2].ai_fight), 32);
    route_roundtrip(0x09, 2, 0, 8, 0x800,
        reinterpret_cast<unsigned char *>(&PlayersData[2].tech_accumulated), 32);
    route_roundtrip(0x0A, 2, 0, 4, 0x800,
        reinterpret_cast<unsigned char *>(&PlayersData[2].tech_id_researching), 32);
    route_roundtrip(0x0B, 2, 0, 4, 0x800,
        reinterpret_cast<unsigned char *>(&PlayersData[2].energy_reserves), 32);
    route_roundtrip(0x0C, 2, 0, 8, 0x800,
        reinterpret_cast<unsigned char *>(&PlayersData[2].se_alloc_psych), 32);
    route_roundtrip(0x0D, 2, 0, 0x14, 0x800,
        reinterpret_cast<unsigned char *>(&PlayersData[2].soc_category_pending), 32);
    route_roundtrip(0x0E, 3, 0, 0x34, 0x34,
        reinterpret_cast<unsigned char *>(&VehPrototypes[3]), 32);
    route_roundtrip(0x0F, 3, 2, 0x68, 0x34,
        reinterpret_cast<unsigned char *>(&VehPrototypes[3]), 32);
    route_roundtrip(0x10, 3, 0, 1, 1,
        reinterpret_cast<unsigned char *>(&VehPrototypes[3].obsolete_factions), 8);
    route_roundtrip(0x11, 4, 0, 0x30, 0x34,
        reinterpret_cast<unsigned char *>(&Vehs[4]), 32);
    route_roundtrip(0x12, 0, 0, 3 * 0x34, 0x800,
        reinterpret_cast<unsigned char *>(&Vehs), 32);
    route_roundtrip(0x13, 3, 0, 0x134, 0x800,
        reinterpret_cast<unsigned char *>(&Bases[3]), 32);
    route_roundtrip(0x14, 0, 0, 3 * 0x134, 0x800,
        reinterpret_cast<unsigned char *>(&Bases), 32);
    route_roundtrip(0x17, 4, 0, 4, 0x800,
        reinterpret_cast<unsigned char *>(&PlayersData[4]), 32);
    route_roundtrip(0x18, 4, 0, 0x224, 0x800,
        reinterpret_cast<unsigned char *>(&PlayersData[4].unk_37), 32);
    route_roundtrip(0x19, 0, 0, 0x20, 0x10,
        reinterpret_cast<unsigned char *>(&FactionRankingsUnk), 16);
    route_roundtrip(0x20, 0, 0, 16, 8,
        reinterpret_cast<unsigned char *>(&FactionRankingHistory), 16);
    route_roundtrip(0x21, 0, 0, 4, 0,
        reinterpret_cast<unsigned char *>(&ReplayEventBytes), 4, true);
    route_roundtrip(0x22, 0, 0, 16, 0x800,
        reinterpret_cast<unsigned char *>(&ReplayEvents), 32);
    route_roundtrip(0x23, 3, 0, 0x14, 0x800,
        reinterpret_cast<unsigned char *>(&Bases[3].resource_sqr_radius), 32);
    route_roundtrip(0x24, 0, 0, 0x98, 0x10,
        reinterpret_cast<unsigned char *>(&ScenarioSave), 32);
    route_roundtrip(0x25, 0, 0, 4, 4,
        reinterpret_cast<unsigned char *>(
            &GameDataStateStorage.fields_.vehicle_current_count_), 4, true);
    route_roundtrip(0x26, 0, 0, 4, 4,
        reinterpret_cast<unsigned char *>(
            &GameDataStateStorage.fields_.base_current_count_), 4, true);
    route_roundtrip(0x27, 0, 0, 0x104, 0x800,
        reinterpret_cast<unsigned char *>(&NetDaemonFileTransferPath), 32, true);
    route_roundtrip(0x29, 0, 0, 0x1000, 0x800,
        reinterpret_cast<unsigned char *>(&BungList), 32);

    uint32_t diplomacy[18];
    PlayersData[1].diplo_patience[2] = static_cast<char>(0xFE);
    PlayersData[2].diplo_patience[1] = 0x31;
    PlayersData[1].diplo_friction[2] = 0x10203040;
    PlayersData[2].diplo_friction[1] = 0x50607080;
    NetDaemonSyncWire::gather_saved_diplomacy(1, 2, diplomacy);
    assert(diplomacy[0] == 0xFFFFFFFEu && diplomacy[1] == 0x31u);
    assert(diplomacy[2] == 0x10203040u && diplomacy[3] == 0x50607080u);
    for (int i = 0; i < 18; ++i) diplomacy[i] = 0xA0000000u + i;
    NetDaemonSyncWire::scatter_saved_diplomacy(1, 2, diplomacy);
    assert(static_cast<unsigned char>(PlayersData[1].diplo_patience[2]) == 0);
    assert(static_cast<unsigned char>(PlayersData[2].diplo_patience[1]) == 1);
    assert(static_cast<uint32_t>(PlayersData[1].diplo_friction[2]) == 0xA0000002u);
    assert(static_cast<uint32_t>(PlayersData[2].diplo_friction[1]) == 0xA0000003u);

    // Actual saved tuple width is 18 DWORDs. The specialized native route
    // ignores offset and forces 0x48, even if common-loop count was 0x800.
    // Only complete tuples/packets are supplied; short-packet effects are not
    // claimed. No larger native stack owner or neighboring fields are modeled.
    unsigned char tuple_packet[0x48];
    const size_t tuple_offsets[] = {0, 0x800, 0xFFFFF800u, 0x03FFF800u, 0xFC000000u};
    for (size_t at = 0; at < sizeof tuple_offsets / sizeof tuple_offsets[0]; ++at) {
        fill(tuple_packet, sizeof tuple_packet, static_cast<unsigned char>(0x11 + at));
        assert(NetDaemonSyncWire::copy_nonrule_chunk(0x16, 1, 2, 0,
            0x48, tuple_offsets[at], tuple_packet, 0x800, true, diplomacy, captured, 0));
        assert(memcmp(diplomacy, tuple_packet, sizeof diplomacy) == 0);
        fill(tuple_packet, sizeof tuple_packet, 0);
        assert(NetDaemonSyncWire::copy_nonrule_chunk(0x16, 1, 2, 0,
            0x48, tuple_offsets[at], tuple_packet, 0x800, false, diplomacy, captured, 0));
        assert(memcmp(diplomacy, tuple_packet, sizeof diplomacy) == 0);
    }
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x16, 1, 2, 0,
        0x48, 0, reinterpret_cast<unsigned char *>(diplomacy), 0, true, diplomacy, captured, 0));

    // Native opcode 0x06 copies a complete contiguous PlayerData record in
    // five chunks (four full chunks and the 0xCC-byte tail). Exercise the
    // actual extracted product router in both directions across every byte.
    assert(sizeof(PlayerData) == 0x20CC);
    unsigned char player_record_expected[0x20CC];
    unsigned char player_record_chunk[0x800];
    unsigned char *player_record_bytes =
        reinterpret_cast<unsigned char *>(&PlayersData[4]);
    for (size_t i = 0; i < sizeof player_record_expected; ++i) {
        player_record_expected[i] = static_cast<unsigned char>((i * 37u + 11u) & 0xFFu);
        player_record_bytes[i] = player_record_expected[i];
    }
    for (size_t chunk = 0; chunk < 5; ++chunk) {
        const size_t offset = chunk * 0x800;
        const size_t count = chunk == 4 ? 0xCC : 0x800;
        fill(player_record_chunk, count, 0xA5);
        assert(NetDaemonSyncWire::copy_nonrule_chunk(0x06, 4, 0, 0,
            0x20CC, offset, player_record_chunk, count, false,
            0, captured, 0));
        assert(memcmp(player_record_chunk, player_record_expected + offset,
                      count) == 0);
    }
    for (size_t chunk = 0; chunk < 5; ++chunk) {
        const size_t offset = chunk * 0x800;
        const size_t count = chunk == 4 ? 0xCC : 0x800;
        for (size_t i = 0; i < count; ++i)
            player_record_chunk[i] =
                static_cast<unsigned char>(((offset + i) * 19u + 7u) & 0xFFu);
        memcpy(player_record_expected + offset, player_record_chunk, count);
        assert(NetDaemonSyncWire::copy_nonrule_chunk(0x06, 4, 0, 0,
            0x20CC, offset, player_record_chunk, count, true,
            0, captured, 0));
    }
    assert(memcmp(player_record_bytes, player_record_expected,
                  sizeof player_record_expected) == 0);

    // A nonfinal native chunk with signed index 5 begins beyond the selected
    // a2=0 record but remains entirely within the declared PlayersData array.
    // This safely distinguishes a raw contiguous copy from the old 0x20CC
    // payload guard/member intersection without claiming external neighbors.
    unsigned char *all_player_data =
        reinterpret_cast<unsigned char *>(&PlayersData);
    const size_t crossing_offset = 5 * 0x800;
    const size_t crossing_count = 0x800;
    assert(crossing_offset + crossing_count < sizeof PlayersData);
    for (size_t i = 0; i < crossing_count; ++i)
        all_player_data[crossing_offset + i] =
            static_cast<unsigned char>((i * 13u + 0x39u) & 0xFFu);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x06, 0, 0, 0,
        0x20CC, crossing_offset, player_record_chunk, crossing_count,
        false, 0, captured, 0));
    for (size_t i = 0; i < crossing_count; ++i)
        assert(player_record_chunk[i] ==
            static_cast<unsigned char>((i * 13u + 0x39u) & 0xFFu));
    fill(player_record_chunk, crossing_count, 0xB6);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x06, 0, 0, 0,
        0x20CC, crossing_offset, player_record_chunk, crossing_count,
        true, 0, captured, 0));
    for (size_t i = 0; i < crossing_count; ++i)
        assert(all_player_data[crossing_offset + i] == 0xB6);

    // The native common copy routine is overlap-aware. Exercise its direction
    // for packet-to-owner and owner-to-packet aliases, all within the declared
    // PlayersData array; memcpy-forward substitutes fail under ASan.
    unsigned char alias_expected[0x300];
    memcpy(alias_expected, all_player_data, sizeof alias_expected);
    memmove(alias_expected + 0x180, alias_expected + 0x100, 0x100);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x06, 0, 0, 0,
        0x20CC, 0x180, all_player_data + 0x100, 0x100,
        true, 0, captured, 0));
    assert(memcmp(all_player_data, alias_expected, sizeof alias_expected) == 0);

    memcpy(alias_expected, all_player_data, sizeof alias_expected);
    memmove(alias_expected + 0x180, alias_expected + 0x100, 0x100);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x06, 0, 0, 0,
        0x20CC, 0x100, all_player_data + 0x180, 0x100,
        false, 0, captured, 0));
    assert(memcmp(all_player_data, alias_expected, sizeof alias_expected) == 0);

    // Native opcode 0x17/23 shares PlayerData's base with 0x06/6 but transfers
    // only its leading dword; it must not copy an entire 0x20CC record.
    PlayersData[3].flags = 0x12345678u;
    unsigned char player_flags[4];
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x17, 3, 0, 0, 4,
        0, player_flags, sizeof player_flags, false, 0, captured, 0));
    assert(memcmp(player_flags, &PlayersData[3].flags, 4) == 0);
    memset(player_flags, 0xA4, sizeof player_flags);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x17, 3, 0, 0, 4,
        0, player_flags, sizeof player_flags, true, 0, captured, 0));
    assert(PlayersData[3].flags == 0xA4A4A4A4u);

    // 0x0E/14 is one 0x34-byte VehPrototype view within the declared array.
    fill(reinterpret_cast<unsigned char *>(&VehPrototypes[7]), 0x34, 0x6D);
    unsigned char proto[0x34];
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x0E, 7, 0, 0, 0x34,
        0, proto, sizeof proto, false, 0, captured, 0));
    assert(proto[0] == 0x6D && proto[0x33] == 0x6D);

    // Opcode 0x24/36 round-trips its six typed words and opaque observed tail.
    const int objective = 77;
    ObjectiveReqVictory = objective;
    for (int i = 0; i < 0x80; ++i)
        GameWireOpaque_94B4D8[i] = static_cast<uint8_t>(i ^ 0xA7);
    unsigned char tail[0x98];
    fill(tail, sizeof tail, 0xCC);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x24, 0, 0, 0, 0x98,
        0, tail, sizeof tail, false, 0, captured, 0));
    assert(tail[0] == static_cast<unsigned char>(objective));
    assert(tail[0x18] == GameWireOpaque_94B4D8[0]);
    assert(tail[0x97] == GameWireOpaque_94B4D8[0x7F]);
    fill(GameWireOpaque_94B4D8, sizeof GameWireOpaque_94B4D8, 0xCC);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x24, 0, 0, 0, 0x98,
        0, tail, sizeof tail, true, 0, captured, 0));
    assert(ObjectiveReqVictory == objective);
    assert(GameWireOpaque_94B4D8[0] == 0xA7 &&
           GameWireOpaque_94B4D8[0x7F] ==
               static_cast<uint8_t>(0x7F ^ 0xA7));

    // Opcode 0x27/39 is the persistent 260-byte transfer path, not a chunk
    // local buffer. Both transfer directions clip to its observed owner span.
    for (int i = 0; i < 0x104; ++i)
        NetDaemonFileTransferPath[i] = static_cast<char>(i ^ 0x5C);
    unsigned char path_wire[0x104];
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x27, 0, 0, 0, 0x104,
        0, path_wire, sizeof path_wire, false, 0, captured, 0));
    assert(path_wire[0] == 0x5C &&
           path_wire[0x103] == static_cast<uint8_t>(0x103 ^ 0x5C));
    fill(reinterpret_cast<unsigned char *>(NetDaemonFileTransferPath),
         sizeof NetDaemonFileTransferPath, 0);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x27, 0, 0, 0, 0x104,
        0, path_wire, sizeof path_wire, true, 0, captured, 0));
    assert(static_cast<unsigned char>(NetDaemonFileTransferPath[0]) == 0x5C &&
           static_cast<unsigned char>(NetDaemonFileTransferPath[0x103]) ==
               static_cast<unsigned char>(0x103 ^ 0x5C));

    // Rectangle pointer start is supplied as the pre-callback capture.
    fill(reinterpret_cast<unsigned char *>(&tiles[8]), 8 * sizeof(Map), 0x82);
    unsigned char tile_packet[8 * sizeof(Map)];
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x30, 1, 0, 0,
        sizeof tile_packet, 0, tile_packet, sizeof tile_packet, false, 0, captured, 8 * sizeof(Map)));
    assert(tile_packet[0] == 0x82 && tile_packet[sizeof tile_packet - 1] == 0x82);
    return 0;
}
