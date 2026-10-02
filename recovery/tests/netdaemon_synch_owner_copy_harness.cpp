#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef int BOOL;
static const int MaxPlayerNum = 8;
static const int MaxVehProtoNum = 512;
static const int MaxBaseNum = 512;
static const int MaxContinentNum = 128;
static const int MaxLandmarkNum = 64;
static const int MaxRankingHistoryTurns = 1000;
struct Map { unsigned char bytes[44]; };
struct Landmark { unsigned char bytes[40]; };
struct Continent { unsigned char bytes[28]; };
struct SocialCategory { uint32_t models_[4]; };
struct Player { unsigned char bytes[0x59C]; };
struct PlayerData {
    uint32_t flags;
    uint32_t diplo_treaties[8];
    int diplo_agenda[8], diplo_spoke[8], diplo_merc[8];
    uint32_t diplo_friction[8];
    char diplo_patience[8];
    int global_reputation, diplo_unk1[8], diplo_wrongs[8];
    unsigned char pad_004[0x21C];
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
    unsigned char pad_3E8[0x70];
    char saved_queue_name[8][24];
    int saved_queue_size[8];
    int saved_queue_items[8][10];
    int unk_38[8];
    unsigned char tail[0x20CC - 0x5F8];
};
struct VehPrototype {
    char veh_name[32]; uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id, carry_capacity,
        cost, plan; int8_t unk_1; uint8_t obsolete_factions, combat_factions,
        icon_offset, padding; uint16_t flags; int16_t preq_tech;
};
struct Veh { unsigned char bytes[0x34]; };
struct Base {
    unsigned char prefix[0x78];
    int resource_sqr_radius, specialist_total, specialist_unk_1;
    uint32_t spec[2];
    unsigned char tail[0x134 - 0x8C];
};
struct ReplayEventRecord { unsigned char bytes[8]; };

uint32_t LockEnableMask;
uint8_t GameWireOpaque_94B4D8[0x80];
char NetDaemonFileTransferPath[0x104];
int MapLongitudeBounds, MapLatitudeBounds, MapSeaLevel, MapSeaLevelCouncil;
uint32_t MapRandSeed, MapArea, MapAreaSqRoot;
BOOL MapIsFlat;
int MapLandmarkCount, MapAbstractLongBounds;
uint32_t MapAbstractLatBounds, MapAbstractArea, MapSizePlanet,
    MapOceanCoverage, MapLandCoverage, MapErosiveForces, MapPlanetaryOrbit,
    MapCloudCover, MapNativeLifeForms, MapLongitude;
Landmark MapLandmark[MaxLandmarkNum];
char MapFilePath[80];
Map *MapTiles;
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
int ObjectiveReqVictory;
uint32_t ObjectivesSuddenDeathVictory, ObjectiveAchievePts,
    VictoryAchieveBonusPts, StartingMissionYear, EndingMissionYear;
int VehCurrentCount, BaseCurrentCount;
uint32_t GamePreferences, GameMorePreferences, GameWarnings, GameRules;
uint8_t GameWireOpaque_9A64A0[4], GameWireOpaque_9A64A8[4], GameWireOpaque_9A67C9[3];
uint32_t GameInterludeFlags;
int TutorialFlags, TutorialFlags2;
uint32_t TerraformAnnouncedMask[2];
int TutWinFlags9A64BC;
uint32_t GameState;
int DiffLevelCurrent, GameSetupResetWord, TurnCurrentNum, MissionYearCurrent;
int VehSelectionFollowLimit, TimeControlsSelection, TurnControlDurationMilliseconds;
uint32_t FactionRankings[8];
int RankingFactionIDUnk1, RankingFactionIDUnk2;
uint32_t SecretProject[64];
int GovernorFaction, CouncilResolutionCount[11], CouncilLastResolutionTurn[11];
uint8_t GameTechAchieved[89];
char LegacyGameSaveName[256];
int ClimateRiseStep, ClimateRiseLimit, ClimateRiseAccum, ClimateSeaLevelDelta;
int TranscendenceScoreBonus;
uint32_t TranscendenceScoreWeights[8];
int SunspotDuration;
uint32_t MountPlanetX, MountPlanetY;
int DustCloudDuration, SaveDaemonStateWord;
uint32_t ScnVictFacilityObj, GlobalEnergyEventState, NetTurnFlags;
int NetTurnFaction, EcologyEventAccumulator;

/* SOURCE EXTRACTS */

bool veh_bung_copy_chunk(size_t, unsigned char *, size_t, bool) { return false; }

static void fill(unsigned char *p, size_t n, unsigned char v) { memset(p, v, n); }

int main() {
    unsigned char bytes[0xA9C];
    fill(bytes, sizeof bytes, 0xA5);
    Map tiles[32];
    Map *captured = tiles;
    MapTiles = tiles;
    MapArea = 32;
    MapLongitudeBounds = 8;
    MapLatitudeBounds = 16;
    MapLongitude = 4;

    // Map scatter crosses independent globals/array boundaries but never a
    // synthetic containing object.
    uint32_t value = 0x44332211;
    memcpy(&MapRandSeed, &value, 4);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(2, 0, 0, 0, 0xA9C,
        8, bytes, 4, false, 0, captured, 32, 0));
    assert(memcmp(bytes, &value, 4) == 0);

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

    // Native opcode 0x17/23 shares PlayerData's base with 0x06/6 but transfers
    // only its leading dword; it must not copy an entire 0x20CC record.
    PlayersData[3].flags = 0x12345678u;
    unsigned char player_flags[4];
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x17, 3, 0, 0, 4,
        0, player_flags, sizeof player_flags, false, 0, captured, 32, 0));
    assert(memcmp(player_flags, &PlayersData[3].flags, 4) == 0);
    memset(player_flags, 0xA4, sizeof player_flags);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x17, 3, 0, 0, 4,
        0, player_flags, sizeof player_flags, true, 0, captured, 32, 0));
    assert(PlayersData[3].flags == 0xA4A4A4A4u);

    // 0x0E/14 is exactly one 0x34-byte VehPrototype; out-of-range index fails.
    fill(reinterpret_cast<unsigned char *>(&VehPrototypes[7]), 0x34, 0x6D);
    unsigned char proto[0x34];
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x0E, 7, 0, 0, 0x34,
        0, proto, sizeof proto, false, 0, captured, 32, 0));
    assert(proto[0] == 0x6D && proto[0x33] == 0x6D);
    assert(!NetDaemonSyncWire::copy_nonrule_chunk(0x0E, 512, 0, 0, 0x34,
        0, proto, sizeof proto, false, 0, captured, 32, 0));

    // Opcode 0x24/36 round-trips its six typed words and opaque observed tail.
    const int objective = 77;
    ObjectiveReqVictory = objective;
    for (int i = 0; i < 0x80; ++i)
        GameWireOpaque_94B4D8[i] = static_cast<uint8_t>(i ^ 0xA7);
    unsigned char tail[0x98];
    fill(tail, sizeof tail, 0xCC);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x24, 0, 0, 0, 0x98,
        0, tail, sizeof tail, false, 0, captured, 32, 0));
    assert(tail[0] == static_cast<unsigned char>(objective));
    assert(tail[0x18] == GameWireOpaque_94B4D8[0]);
    assert(tail[0x97] == GameWireOpaque_94B4D8[0x7F]);
    fill(GameWireOpaque_94B4D8, sizeof GameWireOpaque_94B4D8, 0xCC);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x24, 0, 0, 0, 0x98,
        0, tail, sizeof tail, true, 0, captured, 32, 0));
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
        0, path_wire, sizeof path_wire, false, 0, captured, 32, 0));
    assert(path_wire[0] == 0x5C &&
           path_wire[0x103] == static_cast<uint8_t>(0x103 ^ 0x5C));
    fill(reinterpret_cast<unsigned char *>(NetDaemonFileTransferPath),
         sizeof NetDaemonFileTransferPath, 0);
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x27, 0, 0, 0, 0x104,
        0, path_wire, sizeof path_wire, true, 0, captured, 32, 0));
    assert(static_cast<unsigned char>(NetDaemonFileTransferPath[0]) == 0x5C &&
           static_cast<unsigned char>(NetDaemonFileTransferPath[0x103]) ==
               static_cast<unsigned char>(0x103 ^ 0x5C));

    // Opcode 0x00/0 has a completely preflighted partition across independent
    // source owners; checks cover its leading fields, an interior boundary,
    // the save-name/opaque join, and the final byte.
    GamePreferences = 0x11223344u;
    TutorialFlags2 = 0x55667788;
    TerraformAnnouncedMask[0] = 0x99AABBCCu;
    GameWireOpaque_9A64A0[0] = 0xD1;
    GameWireOpaque_9A64A8[3] = 0xD8;
    LegacyGameSaveName[0] = 'N';
    GameWireOpaque_9A67C9[2] = 0xE3;
    EcologyEventAccumulator = 0xCAFEBABEu;
    unsigned char game_wire[0x398];
    fill(game_wire, sizeof game_wire, 0xA5);
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0, game_wire,
        sizeof game_wire, false));
    assert(game_wire[0] == 0x44 && game_wire[3] == 0x11);
    assert(game_wire[0x20] == 0x88 && game_wire[0x23] == 0x55);
    assert(game_wire[0x24] == 0xCC && game_wire[0x27] == 0x99);
    assert(game_wire[0x10] == 0xD1 && game_wire[0x1B] == 0xD8);
    assert(game_wire[0x239] == 'N' && game_wire[0x33B] == 0xE3);
    assert(game_wire[0x394] == 0xBE && game_wire[0x397] == 0xCA);
    memset(game_wire + 0x20, 0x3C, 8);
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0x20, game_wire + 0x20,
        8, true));
    assert(static_cast<unsigned char>(reinterpret_cast<unsigned char *>(
        &TutorialFlags2)[0]) == 0x3C);
    assert(static_cast<unsigned char>(reinterpret_cast<unsigned char *>(
        &TerraformAnnouncedMask[0])[0]) == 0x3C);
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0x398, game_wire, 1, false) == false);

    // Rectangle pointer start is supplied as the pre-callback capture.
    fill(reinterpret_cast<unsigned char *>(&tiles[8]), 8 * sizeof(Map), 0x82);
    unsigned char tile_packet[8 * sizeof(Map)];
    assert(NetDaemonSyncWire::copy_nonrule_chunk(0x30, 1, 0, 0,
        sizeof tile_packet, 0, tile_packet, sizeof tile_packet, false, 0,
        captured, 32, 8));
    assert(tile_packet[0] == 0x82 && tile_packet[sizeof tile_packet - 1] == 0x82);
    return 0;
}
