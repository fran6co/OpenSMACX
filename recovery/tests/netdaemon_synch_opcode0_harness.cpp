#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// These storage declarations mirror the current source-owner primitive types
// and bounds. BaseSecretProject's field sequence is copied from src/base.h.
struct BaseSecretProject {
    int human_genome_project, command_nexus, weather_paradigm, merchant_exchange;
    int empath_guild, citizens_defense_force, virtual_world, planetary_transit_system;
    int xenoempathy_dome, neural_amplifier, maritime_control_center, planetary_datalinks;
    int supercollider, ascetic_virtues, longevity_vaccine, hunter_seeker_algorithm;
    int pholus_mutagen, cyborg_factory, theory_of_everything, dream_twister;
    int universal_translator, network_backbone, nano_factory, living_refinery;
    int cloning_vats, self_aware_colony, clinical_immortality, space_elevator;
    int singularity_inductor, bulk_matter_transmitter, telepathic_matrix, voice_of_planet;
    int ascent_to_transcendence, manifold_harmonics, nethack_terminus, cloudbase_academy;
    int planetary_energy_grid, user_defined_38, user_defined_39, user_defined_40;
    int user_defined_41, user_defined_42, user_defined_43, user_defined_44;
    int user_defined_45, user_defined_46, user_defined_47, user_defined_48;
    int user_defined_49, user_defined_50, user_defined_51, user_defined_52;
    int user_defined_53, user_defined_54, user_defined_55, user_defined_56;
    int user_defined_57, user_defined_58, user_defined_59, user_defined_60;
    int user_defined_61, user_defined_62, user_defined_63, user_defined_64;
};

uint32_t GamePreferences, GameMorePreferences, GameWarnings, GameRules;
uint8_t GameWireOpaque_9A64A0[4], GameWireOpaque_9A64A8[4], GameWireOpaque_9A67C9[3];
uint32_t GameInterludeFlags, GameState;
int TutorialFlags, TutorialFlags2, TutWinFlags9A64BC, DiffLevelCurrent;
int VehCurrentCount, BaseCurrentCount, GameSetupResetWord, TurnCurrentNum;
uint32_t MissionYearCurrent;
int VehSelectionFollowLimit, TimeControlsSelection, TurnControlDurationMilliseconds;
uint32_t LockEnableMask, FactionRankings[8];
uint32_t RankingFactionIDUnk1, RankingFactionIDUnk2;
BaseSecretProject SecretProject;
int GovernorFaction, CouncilResolutionCount[11], CouncilLastResolutionTurn[11];
uint8_t GameTechAchieved[89];
char LegacyGameSaveName[256];
int ClimateRiseStep, ClimateRiseLimit, ClimateRiseAccum, ClimateSeaLevelDelta;
uint32_t TerraformAnnouncedMask[2];
int TranscendenceScoreBonus, TranscendenceScoreWeights[8];
int SunspotDuration;
uint32_t MountPlanetX, MountPlanetY;
int DustCloudDuration, SaveDaemonStateWord;
uint32_t ScnVictFacilityObj, GlobalEnergyEventState, NetTurnFlags;
int NetTurnFaction, EcologyEventAccumulator;

// Scratch overlay for root integration: include from the actual owner's TU
// after game.h, veh.h, console.h, base.h, alpha.h, technology.h,
// terraforming.h, and council.h. VC6-compatible negative-array assertions.
#define ND_WIRE0_ASSERT(name, expression) \
    typedef char nd_wire0_assert_##name[(expression) ? 1 : -1]

ND_WIRE0_ASSERT(GamePreferences, sizeof GamePreferences == 4);
ND_WIRE0_ASSERT(GameMorePreferences, sizeof GameMorePreferences == 4);
ND_WIRE0_ASSERT(GameWarnings, sizeof GameWarnings == 4);
ND_WIRE0_ASSERT(GameRules, sizeof GameRules == 4);
ND_WIRE0_ASSERT(GameWireOpaque_9A64A0, sizeof GameWireOpaque_9A64A0 == 4);
ND_WIRE0_ASSERT(GameInterludeFlags, sizeof GameInterludeFlags == 4);
ND_WIRE0_ASSERT(GameWireOpaque_9A64A8, sizeof GameWireOpaque_9A64A8 == 4);
ND_WIRE0_ASSERT(TutorialFlags, sizeof TutorialFlags == 4);
ND_WIRE0_ASSERT(TutorialFlags2, sizeof TutorialFlags2 == 4);
ND_WIRE0_ASSERT(TerraformAnnouncedMask, sizeof TerraformAnnouncedMask == 8);
ND_WIRE0_ASSERT(TutWinFlags9A64BC, sizeof TutWinFlags9A64BC == 4);
ND_WIRE0_ASSERT(GameState, sizeof GameState == 4);
ND_WIRE0_ASSERT(DiffLevelCurrent, sizeof DiffLevelCurrent == 4);
ND_WIRE0_ASSERT(VehCurrentCount, sizeof VehCurrentCount == 4);
ND_WIRE0_ASSERT(BaseCurrentCount, sizeof BaseCurrentCount == 4);
ND_WIRE0_ASSERT(GameSetupResetWord, sizeof GameSetupResetWord == 4);
ND_WIRE0_ASSERT(TurnCurrentNum, sizeof TurnCurrentNum == 4);
ND_WIRE0_ASSERT(MissionYearCurrent, sizeof MissionYearCurrent == 4);
ND_WIRE0_ASSERT(VehSelectionFollowLimit, sizeof VehSelectionFollowLimit == 4);
ND_WIRE0_ASSERT(TimeControlsSelection, sizeof TimeControlsSelection == 4);
ND_WIRE0_ASSERT(TurnControlDurationMilliseconds, sizeof TurnControlDurationMilliseconds == 4);
ND_WIRE0_ASSERT(LockEnableMask, sizeof LockEnableMask == 4);
ND_WIRE0_ASSERT(FactionRankings, sizeof FactionRankings == 32);
ND_WIRE0_ASSERT(RankingFactionIDUnk1, sizeof RankingFactionIDUnk1 == 4);
ND_WIRE0_ASSERT(RankingFactionIDUnk2, sizeof RankingFactionIDUnk2 == 4);
ND_WIRE0_ASSERT(SecretProject, sizeof SecretProject == 0x100);
ND_WIRE0_ASSERT(GovernorFaction, sizeof GovernorFaction == 4);
ND_WIRE0_ASSERT(CouncilResolutionCount, sizeof CouncilResolutionCount == 0x2C);
ND_WIRE0_ASSERT(CouncilLastResolutionTurn, sizeof CouncilLastResolutionTurn == 0x2C);
ND_WIRE0_ASSERT(GameTechAchieved, sizeof GameTechAchieved == 89);
ND_WIRE0_ASSERT(LegacyGameSaveName, sizeof LegacyGameSaveName == 256);
ND_WIRE0_ASSERT(GameWireOpaque_9A67C9, sizeof GameWireOpaque_9A67C9 == 3);
ND_WIRE0_ASSERT(ClimateRiseStep, sizeof ClimateRiseStep == 4);
ND_WIRE0_ASSERT(ClimateRiseLimit, sizeof ClimateRiseLimit == 4);
ND_WIRE0_ASSERT(ClimateRiseAccum, sizeof ClimateRiseAccum == 4);
ND_WIRE0_ASSERT(ClimateSeaLevelDelta, sizeof ClimateSeaLevelDelta == 4);
ND_WIRE0_ASSERT(TranscendenceScoreBonus, sizeof TranscendenceScoreBonus == 4);
ND_WIRE0_ASSERT(TranscendenceScoreWeights, sizeof TranscendenceScoreWeights == 32);
ND_WIRE0_ASSERT(SunspotDuration, sizeof SunspotDuration == 4);
ND_WIRE0_ASSERT(MountPlanetX, sizeof MountPlanetX == 4);
ND_WIRE0_ASSERT(MountPlanetY, sizeof MountPlanetY == 4);
ND_WIRE0_ASSERT(DustCloudDuration, sizeof DustCloudDuration == 4);
ND_WIRE0_ASSERT(SaveDaemonStateWord, sizeof SaveDaemonStateWord == 4);
ND_WIRE0_ASSERT(ScnVictFacilityObj, sizeof ScnVictFacilityObj == 4);
ND_WIRE0_ASSERT(GlobalEnergyEventState, sizeof GlobalEnergyEventState == 4);
ND_WIRE0_ASSERT(NetTurnFlags, sizeof NetTurnFlags == 4);
ND_WIRE0_ASSERT(NetTurnFaction, sizeof NetTurnFaction == 4);
ND_WIRE0_ASSERT(EcologyEventAccumulator, sizeof EcologyEventAccumulator == 4);

ND_WIRE0_ASSERT(BaseSecretProject, sizeof(BaseSecretProject) == 0x100);
ND_WIRE0_ASSERT(FactionRankingsCount, sizeof FactionRankings / sizeof FactionRankings[0] == 8);
ND_WIRE0_ASSERT(TerraformAnnouncedMaskCount, sizeof TerraformAnnouncedMask / sizeof TerraformAnnouncedMask[0] == 2);
ND_WIRE0_ASSERT(TechnologyCount, sizeof GameTechAchieved / sizeof GameTechAchieved[0] == 89);
ND_WIRE0_ASSERT(CouncilCount, sizeof CouncilResolutionCount / sizeof CouncilResolutionCount[0] == 11);

// Address-relative boundaries from the pinned opcode-0 transfer interval.
// These equations make a changed member size fail at its next boundary too.
ND_WIRE0_ASSERT(boundary_004, 0x000 + sizeof GamePreferences == 0x004);
ND_WIRE0_ASSERT(boundary_010, 0x00C + sizeof GameRules == 0x010);
ND_WIRE0_ASSERT(boundary_014, 0x010 + sizeof GameWireOpaque_9A64A0 == 0x014);
ND_WIRE0_ASSERT(boundary_01C, 0x018 + sizeof GameWireOpaque_9A64A8 == 0x01C);
ND_WIRE0_ASSERT(boundary_024, 0x020 + sizeof TutorialFlags2 == 0x024);
ND_WIRE0_ASSERT(boundary_02C, 0x024 + sizeof TerraformAnnouncedMask == 0x02C);
ND_WIRE0_ASSERT(boundary_05C, 0x058 + sizeof LockEnableMask == 0x05C);
ND_WIRE0_ASSERT(boundary_07C, 0x05C + sizeof FactionRankings == 0x07C);
ND_WIRE0_ASSERT(boundary_084, 0x07C + sizeof RankingFactionIDUnk1 + sizeof RankingFactionIDUnk2 == 0x084);
ND_WIRE0_ASSERT(boundary_184, 0x084 + sizeof SecretProject == 0x184);
ND_WIRE0_ASSERT(boundary_1E0, 0x1B4 + sizeof CouncilLastResolutionTurn == 0x1E0);
ND_WIRE0_ASSERT(boundary_239, 0x1E0 + sizeof GameTechAchieved == 0x239);
ND_WIRE0_ASSERT(boundary_339, 0x239 + sizeof LegacyGameSaveName == 0x339);
ND_WIRE0_ASSERT(boundary_33C, 0x339 + sizeof GameWireOpaque_9A67C9 == 0x33C);
ND_WIRE0_ASSERT(boundary_370, 0x350 + sizeof TranscendenceScoreWeights == 0x370);
ND_WIRE0_ASSERT(boundary_398, 0x370 + sizeof SunspotDuration + sizeof MountPlanetX + sizeof MountPlanetY + sizeof DustCloudDuration + sizeof SaveDaemonStateWord + sizeof ScnVictFacilityObj + sizeof GlobalEnergyEventState + sizeof NetTurnFlags + sizeof NetTurnFaction + sizeof EcologyEventAccumulator == 0x398);

#undef ND_WIRE0_ASSERT

/* SOURCE EXTRACTS */

static unsigned char owners[0x398];
static unsigned char second[0x398];
static unsigned char output[0x398];

static void pattern(unsigned char *p, size_t n, unsigned int seed) {
    size_t i;
    for (i = 0; i < n; ++i)
        p[i] = static_cast<unsigned char>((seed + i * 73u + (i >> 2)) & 0xFFu);
}

static void gather_all(unsigned char *out) {
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0, out, 0x398, false));
}

int main() {
    size_t i;
    pattern(owners, sizeof owners, 0x35);
    pattern(second, sizeof second, 0xC1);
    memset(output, 0, sizeof output);

    // Every byte belongs to exactly one declared owner; one-byte requests also
    // ensure there is no unnoticed hole between adjacent independent objects.
    for (i = 0; i < sizeof owners; ++i) {
        assert(NetDaemonSyncWire::wire0_range_is_owned(i, 1));
        assert(NetDaemonSyncWire::opcode0_copy_chunk(i, owners + i, 1, true));
    }
    gather_all(output);
    assert(memcmp(output, owners, sizeof owners) == 0);

    // Scatter/gather through the entire 0x398-byte opcode-0 payload with
    // distinct, nonzero values across every object boundary.
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0, second, sizeof second, true));
    gather_all(output);
    assert(memcmp(output, second, sizeof second) == 0);

    // Native sends reread live storage for each chunk, not a method-entry
    // snapshot. Change the next field between two successful gathers.
    unsigned char before[4];
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0x00C, before, 4, false));
    GameRules ^= 0xA55A3CC3u;
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0x00C, output + 0x00C, 4, false));
    assert(memcmp(output + 0x00C, before, 4) != 0);
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0, second, sizeof second, true));

    // Receive chunks commit only their covered live objects; a simulated
    // callback mutation between chunks remains visible until its later chunk.
    uint32_t callback_word = GameState;
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0x030, owners + 0x030, 4, true));
    GameState = callback_word ^ 0x5AA59669u;
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0x034, owners + 0x034, 4, true));
    assert(GameState == (callback_word ^ 0x5AA59669u));

    // Bounds/preflight controls: the exact interval ends at 0x398; overflow
    // and requests beyond it fail before mutating the target.
    memset(output, 0xD7, sizeof output);
    assert(!NetDaemonSyncWire::opcode0_copy_chunk(0x399, output, 0, false));
    assert(!NetDaemonSyncWire::opcode0_copy_chunk(0x397, output, 2, false));
    assert(!NetDaemonSyncWire::wire0_range_is_owned(0x398, 1));
    for (i = 0; i < sizeof output; ++i) assert(output[i] == 0xD7);
    assert(NetDaemonSyncWire::opcode0_copy_chunk(0x398, output, 0, false));
    return 0;
}

