#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// These storage declarations mirror the current source-owner primitive types
// and bounds. BaseSecretProject's field sequence is copied from src/base.h.
struct BaseSecretProject {
    int human_genome_project_, command_nexus_, weather_paradigm_, merchant_exchange_;
    int empath_guild_, citizens_defense_force_, virtual_world_, planetary_transit_system_;
    int xenoempathy_dome_, neural_amplifier_, maritime_control_center_, planetary_datalinks_;
    int supercollider_, ascetic_virtues_, longevity_vaccine_, hunter_seeker_algorithm_;
    int pholus_mutagen_, cyborg_factory_, theory_of_everything_, dream_twister_;
    int universal_translator_, network_backbone_, nano_factory_, living_refinery_;
    int cloning_vats_, self_aware_colony_, clinical_immortality_, space_elevator_;
    int singularity_inductor_, bulk_matter_transmitter_, telepathic_matrix_, voice_of_planet_;
    int ascent_to_transcendence_, manifold_harmonics_, nethack_terminus_, cloudbase_academy_;
    int planetary_energy_grid_, user_defined_38_, user_defined_39_, user_defined_40_;
    int user_defined_41_, user_defined_42_, user_defined_43_, user_defined_44_;
    int user_defined_45_, user_defined_46_, user_defined_47_, user_defined_48_;
    int user_defined_49_, user_defined_50_, user_defined_51_, user_defined_52_;
    int user_defined_53_, user_defined_54_, user_defined_55_, user_defined_56_;
    int user_defined_57_, user_defined_58_, user_defined_59_, user_defined_60_;
    int user_defined_61_, user_defined_62_, user_defined_63_, user_defined_64_;

 int &human_genome_project() { return human_genome_project_; }
 const int &human_genome_project() const { return human_genome_project_; }

 int &command_nexus() { return command_nexus_; }
 const int &command_nexus() const { return command_nexus_; }

 int &weather_paradigm() { return weather_paradigm_; }
 const int &weather_paradigm() const { return weather_paradigm_; }

 int &merchant_exchange() { return merchant_exchange_; }
 const int &merchant_exchange() const { return merchant_exchange_; }

 int &empath_guild() { return empath_guild_; }
 const int &empath_guild() const { return empath_guild_; }

 int &citizens_defense_force() { return citizens_defense_force_; }
 const int &citizens_defense_force() const { return citizens_defense_force_; }

 int &virtual_world() { return virtual_world_; }
 const int &virtual_world() const { return virtual_world_; }

 int &planetary_transit_system() { return planetary_transit_system_; }
 const int &planetary_transit_system() const { return planetary_transit_system_; }

 int &xenoempathy_dome() { return xenoempathy_dome_; }
 const int &xenoempathy_dome() const { return xenoempathy_dome_; }

 int &neural_amplifier() { return neural_amplifier_; }
 const int &neural_amplifier() const { return neural_amplifier_; }

 int &maritime_control_center() { return maritime_control_center_; }
 const int &maritime_control_center() const { return maritime_control_center_; }

 int &planetary_datalinks() { return planetary_datalinks_; }
 const int &planetary_datalinks() const { return planetary_datalinks_; }

 int &supercollider() { return supercollider_; }
 const int &supercollider() const { return supercollider_; }

 int &ascetic_virtues() { return ascetic_virtues_; }
 const int &ascetic_virtues() const { return ascetic_virtues_; }

 int &longevity_vaccine() { return longevity_vaccine_; }
 const int &longevity_vaccine() const { return longevity_vaccine_; }

 int &hunter_seeker_algorithm() { return hunter_seeker_algorithm_; }
 const int &hunter_seeker_algorithm() const { return hunter_seeker_algorithm_; }

 int &pholus_mutagen() { return pholus_mutagen_; }
 const int &pholus_mutagen() const { return pholus_mutagen_; }

 int &cyborg_factory() { return cyborg_factory_; }
 const int &cyborg_factory() const { return cyborg_factory_; }

 int &theory_of_everything() { return theory_of_everything_; }
 const int &theory_of_everything() const { return theory_of_everything_; }

 int &dream_twister() { return dream_twister_; }
 const int &dream_twister() const { return dream_twister_; }

 int &universal_translator() { return universal_translator_; }
 const int &universal_translator() const { return universal_translator_; }

 int &network_backbone() { return network_backbone_; }
 const int &network_backbone() const { return network_backbone_; }

 int &nano_factory() { return nano_factory_; }
 const int &nano_factory() const { return nano_factory_; }

 int &living_refinery() { return living_refinery_; }
 const int &living_refinery() const { return living_refinery_; }

 int &cloning_vats() { return cloning_vats_; }
 const int &cloning_vats() const { return cloning_vats_; }

 int &self_aware_colony() { return self_aware_colony_; }
 const int &self_aware_colony() const { return self_aware_colony_; }

 int &clinical_immortality() { return clinical_immortality_; }
 const int &clinical_immortality() const { return clinical_immortality_; }

 int &space_elevator() { return space_elevator_; }
 const int &space_elevator() const { return space_elevator_; }

 int &singularity_inductor() { return singularity_inductor_; }
 const int &singularity_inductor() const { return singularity_inductor_; }

 int &bulk_matter_transmitter() { return bulk_matter_transmitter_; }
 const int &bulk_matter_transmitter() const { return bulk_matter_transmitter_; }

 int &telepathic_matrix() { return telepathic_matrix_; }
 const int &telepathic_matrix() const { return telepathic_matrix_; }

 int &voice_of_planet() { return voice_of_planet_; }
 const int &voice_of_planet() const { return voice_of_planet_; }

 int &ascent_to_transcendence() { return ascent_to_transcendence_; }
 const int &ascent_to_transcendence() const { return ascent_to_transcendence_; }

 int &manifold_harmonics() { return manifold_harmonics_; }
 const int &manifold_harmonics() const { return manifold_harmonics_; }

 int &nethack_terminus() { return nethack_terminus_; }
 const int &nethack_terminus() const { return nethack_terminus_; }

 int &cloudbase_academy() { return cloudbase_academy_; }
 const int &cloudbase_academy() const { return cloudbase_academy_; }

 int &planetary_energy_grid() { return planetary_energy_grid_; }
 const int &planetary_energy_grid() const { return planetary_energy_grid_; }

 int &user_defined_38() { return user_defined_38_; }
 const int &user_defined_38() const { return user_defined_38_; }

 int &user_defined_39() { return user_defined_39_; }
 const int &user_defined_39() const { return user_defined_39_; }

 int &user_defined_40() { return user_defined_40_; }
 const int &user_defined_40() const { return user_defined_40_; }

 int &user_defined_41() { return user_defined_41_; }
 const int &user_defined_41() const { return user_defined_41_; }

 int &user_defined_42() { return user_defined_42_; }
 const int &user_defined_42() const { return user_defined_42_; }

 int &user_defined_43() { return user_defined_43_; }
 const int &user_defined_43() const { return user_defined_43_; }

 int &user_defined_44() { return user_defined_44_; }
 const int &user_defined_44() const { return user_defined_44_; }

 int &user_defined_45() { return user_defined_45_; }
 const int &user_defined_45() const { return user_defined_45_; }

 int &user_defined_46() { return user_defined_46_; }
 const int &user_defined_46() const { return user_defined_46_; }

 int &user_defined_47() { return user_defined_47_; }
 const int &user_defined_47() const { return user_defined_47_; }

 int &user_defined_48() { return user_defined_48_; }
 const int &user_defined_48() const { return user_defined_48_; }

 int &user_defined_49() { return user_defined_49_; }
 const int &user_defined_49() const { return user_defined_49_; }

 int &user_defined_50() { return user_defined_50_; }
 const int &user_defined_50() const { return user_defined_50_; }

 int &user_defined_51() { return user_defined_51_; }
 const int &user_defined_51() const { return user_defined_51_; }

 int &user_defined_52() { return user_defined_52_; }
 const int &user_defined_52() const { return user_defined_52_; }

 int &user_defined_53() { return user_defined_53_; }
 const int &user_defined_53() const { return user_defined_53_; }

 int &user_defined_54() { return user_defined_54_; }
 const int &user_defined_54() const { return user_defined_54_; }

 int &user_defined_55() { return user_defined_55_; }
 const int &user_defined_55() const { return user_defined_55_; }

 int &user_defined_56() { return user_defined_56_; }
 const int &user_defined_56() const { return user_defined_56_; }

 int &user_defined_57() { return user_defined_57_; }
 const int &user_defined_57() const { return user_defined_57_; }

 int &user_defined_58() { return user_defined_58_; }
 const int &user_defined_58() const { return user_defined_58_; }

 int &user_defined_59() { return user_defined_59_; }
 const int &user_defined_59() const { return user_defined_59_; }

 int &user_defined_60() { return user_defined_60_; }
 const int &user_defined_60() const { return user_defined_60_; }

 int &user_defined_61() { return user_defined_61_; }
 const int &user_defined_61() const { return user_defined_61_; }

 int &user_defined_62() { return user_defined_62_; }
 const int &user_defined_62() const { return user_defined_62_; }

 int &user_defined_63() { return user_defined_63_; }
 const int &user_defined_63() const { return user_defined_63_; }

 int &user_defined_64() { return user_defined_64_; }
 const int &user_defined_64() const { return user_defined_64_; }
};

#include "src/game_data_state_window.h"
GameDataStateWindow GameDataStateStorage = {};
#define GamePreferences GameDataStateStorage.fields_.game_preferences_
#define GameMorePreferences GameDataStateStorage.fields_.game_more_preferences_
#define GameWarnings GameDataStateStorage.fields_.game_warnings_
#define GameRules GameDataStateStorage.fields_.game_rules_
#define GameWireOpaque_9A64A0 GameDataStateStorage.fields_.game_wire_opaque_9a64a0_
#define GameWireOpaque_9A64A8 GameDataStateStorage.fields_.game_wire_opaque_9a64a8_
#define GameWireOpaque_9A67C9 GameDataStateStorage.fields_.game_wire_opaque_9a67c9_
#define GameInterludeFlags GameDataStateStorage.fields_.game_interlude_flags_
#define GameState GameDataStateStorage.fields_.game_state_
#define TutorialFlags GameDataStateStorage.fields_.tutorial_flags_
#define TutorialFlags2 GameDataStateStorage.fields_.tutorial_flags_2_
#define TutWinFlags9A64BC GameDataStateStorage.fields_.tutwin_flags_9a64bc_
#define DiffLevelCurrent GameDataStateStorage.fields_.diff_level_current_
#define VehCurrentCount GameDataStateStorage.fields_.vehicle_current_count_
#define BaseCurrentCount GameDataStateStorage.fields_.base_current_count_
#define GameSetupResetWord GameDataStateStorage.fields_.game_setup_reset_word_
#define TurnCurrentNum GameDataStateStorage.fields_.turn_current_num_
#define MissionYearCurrent GameDataStateStorage.fields_.mission_year_current_
#define VehSelectionFollowLimit GameDataStateStorage.fields_.vehicle_selection_follow_limit_
#define TimeControlsSelection GameDataStateStorage.fields_.time_controls_selection_
#define TurnControlDurationMilliseconds GameDataStateStorage.fields_.turn_control_duration_milliseconds_
#define LockEnableMask GameDataStateStorage.fields_.lock_masks_.enable_mask_
#define FactionRankings GameDataStateStorage.fields_.faction_rankings_
#define RankingFactionIDUnk1 GameDataStateStorage.fields_.ranking_faction_id_unk1_
#define RankingFactionIDUnk2 GameDataStateStorage.fields_.ranking_faction_id_unk2_
#define SecretProject GameDataStateStorage.fields_.secret_project_
#define GovernorFaction GameDataStateStorage.fields_.governor_faction_
#define CouncilResolutionCount GameDataStateStorage.fields_.council_resolution_count_
#define CouncilLastResolutionTurn GameDataStateStorage.fields_.council_last_resolution_turn_
#define GameTechAchieved GameDataStateStorage.fields_.game_tech_achieved_
#define LegacyGameSaveName GameDataStateStorage.fields_.legacy_game_save_name_
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
