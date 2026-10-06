#ifndef OPENSMACX_GAME_DATA_STATE_WINDOW_H
#define OPENSMACX_GAME_DATA_STATE_WINDOW_H

// Included from base.h after BaseSecretProject is complete. This records the
// exact typed source owner for the 0x009A6490..0x009A6828 game_data window.
// This records the accessed span, not the original enclosing allocation.
#include <stddef.h>
#include <stdint.h>

struct GameDataLockMaskBytes {
    uint8_t human_mask_;
    uint8_t alive_mask_;
    uint8_t unassigned_[2];
};

union GameDataLockMaskView {
    uint32_t enable_mask_;
    GameDataLockMaskBytes bytes_;
};

struct GameDataStateWindowFields {
    uint32_t game_preferences_;                  // +0x000
    uint32_t game_more_preferences_;             // +0x004
    uint32_t game_warnings_;                     // +0x008
    uint32_t game_rules_;                        // +0x00C
    uint8_t game_wire_opaque_9a64a0_[4];          // +0x010
    uint32_t game_interlude_flags_;              // +0x014
    uint8_t game_wire_opaque_9a64a8_[4];          // +0x018
    int tutorial_flags_;                         // +0x01C
    int tutorial_flags_2_;                       // +0x020
    uint32_t terraform_announced_mask_[2];       // +0x024
    int tutwin_flags_9a64bc_;                    // +0x02C
    uint32_t game_state_;                        // +0x030
    int diff_level_current_;                     // +0x034
    int vehicle_current_count_;                  // +0x038
    int base_current_count_;                    // +0x03C
    int game_setup_reset_word_;                 // +0x040
    int turn_current_num_;                      // +0x044
    uint32_t mission_year_current_;             // +0x048
    int vehicle_selection_follow_limit_;        // +0x04C
    int time_controls_selection_;               // +0x050
    int turn_control_duration_milliseconds_;    // +0x054
    GameDataLockMaskView lock_masks_;            // +0x058
    uint32_t faction_rankings_[8];              // +0x05C
    uint32_t ranking_faction_id_unk1_;          // +0x07C
    uint32_t ranking_faction_id_unk2_;          // +0x080
    BaseSecretProject secret_project_;          // +0x084, genuine nested owner
    int governor_faction_;                      // +0x184
    int council_resolution_count_[11];             // +0x188
    int council_last_resolution_turn_[11];         // +0x1B4
    uint8_t game_tech_achieved_[89];               // +0x1E0
    char legacy_game_save_name_[256];            // +0x239
    uint8_t game_wire_opaque_9a67c9_[3];         // +0x339
    int climate_rise_step_;                     // +0x33C
    int climate_rise_limit_;                    // +0x340
    int climate_rise_accum_;                    // +0x344
    int climate_sea_level_delta_;               // +0x348
    int transcendence_score_bonus_;             // +0x34C
    int transcendence_score_weights_[8];        // +0x350
    int sunspot_duration_;                      // +0x370
    uint32_t mount_planet_x_;                   // +0x374
    uint32_t mount_planet_y_;                   // +0x378
    int dust_cloud_duration_;                   // +0x37C
    int save_daemon_state_word_;                // +0x380
    uint32_t scenario_victory_facility_;        // +0x384
    int global_energy_event_state_;             // +0x388
    uint32_t net_turn_flags_;                   // +0x38C
    int net_turn_faction_;                      // +0x390
    int ecology_event_accumulator_;             // +0x394
};

struct GameDataStateWindow {
    GameDataStateWindowFields fields_;
};

extern GameDataStateWindow GameDataStateStorage;

// Former global storage provenance. These comments retain source identity
// and initializer history while the source owner moves into this parent.
// They do not claim the original allocation ended at this accessed extent.
// GamePreferences at 0x009A6490 -> game_preferences_; former source src/game.cpp:670; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// GameMorePreferences at 0x009A6494 -> game_more_preferences_; former source src/game.cpp:1324; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// GameWarnings at 0x009A6498 -> game_warnings_; former source src/game.cpp:1325; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// GameRules at 0x009A649C -> game_rules_; former source src/game.cpp:1326; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// GameWireOpaque_9A64A0 at 0x009A64A0 -> game_wire_opaque_9a64a0_; former source src/game.cpp:2330; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A64A0 0x009A64A4 ?GameWireOpaque_9A64A0@@3PAEA 0x00532E00
// GameInterludeFlags at 0x009A64A4 -> game_interlude_flags_; former source src/game.cpp:2324; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v1 0x009A64A4 ?GameInterludeFlags@@3IA
// GameWireOpaque_9A64A8 at 0x009A64A8 -> game_wire_opaque_9a64a8_; former source src/game.cpp:2332; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A64A8 0x009A64AC ?GameWireOpaque_9A64A8@@3PAEA 0x00532E00
// TutorialFlags at 0x009A64AC -> tutorial_flags_; former source src/tutwin.cpp:88; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// TutorialFlags2 at 0x009A64B0 -> tutorial_flags_2_; former source src/tutwin.cpp:89; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// TerraformAnnouncedMask at 0x009A64B4 -> terraform_announced_mask_; former source src/veh.cpp:10714; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// TutWinFlags9A64BC at 0x009A64BC -> tutwin_flags_9a64bc_; former source src/tutwin.cpp:76; no explicit initializer; static zero initialization.
// Original identity (src/tutwin.cpp): // RECOVERY_OBJECT: v1 0x009A64BC ?TutWinFlags9A64BC@@3HA
// GameState at 0x009A64C0 -> game_state_; former source src/game.cpp:1327; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// DiffLevelCurrent at 0x009A64C4 -> diff_level_current_; former source src/game.cpp:1328; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// VehCurrentCount at 0x009A64C8 -> vehicle_current_count_; former source src/veh.cpp:505; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// BaseCurrentCount at 0x009A64CC -> base_current_count_; former source src/base.cpp:316; no explicit initializer; static zero initialization.
// Original identity (src/base.cpp): // RECOVERY_OBJECT: v2 0x009A64CC 0x009A64D0 ?BaseCurrentCount@@3HA 0x004E68B0
// GameSetupResetWord at 0x009A64D0 -> game_setup_reset_word_; former source src/game.cpp:1333; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A64D0 0x009A64D4 ?GameSetupResetWord@@3HA 0x005B3920
// TurnCurrentNum at 0x009A64D4 -> turn_current_num_; former source src/faction.cpp:465; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// MissionYearCurrent at 0x009A64D8 -> mission_year_current_; former source src/game.cpp:1342; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// VehSelectionFollowLimit at 0x009A64DC -> vehicle_selection_follow_limit_; former source src/veh.cpp:9258; no explicit initializer; static zero initialization.
// Original identity (src/veh.cpp): // RECOVERY_OBJECT: v2 0x009A64DC 0x009A64E0 ?VehSelectionFollowLimit@@3HA 0x005C08C0
// TimeControlsSelection at 0x009A64E0 -> time_controls_selection_; former source src/console.cpp:2641; explicit zero initializer.
// Original identity (src/console.cpp): // RECOVERY_OBJECT: v1 0x009A64E0 ?TimeControlsSelection@@3HA
// TurnControlDurationMilliseconds at 0x009A64E4 -> turn_control_duration_milliseconds_; former source src/game.cpp:1337; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A64E4 0x009A64E8 ?TurnControlDurationMilliseconds@@3HA 0x00523C60
// LockEnableMask at 0x009A64E8 -> lock_masks_.enable_mask_; former source src/lock.cpp:22; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// FactionRankings at 0x009A64EC -> faction_rankings_; former source src/alpha.cpp:126; no explicit initializer; static zero initialization.
// Original identity (src/alpha.cpp): // RECOVERY_OBJECT: v2 0x009A64EC 0x009A650C ?FactionRankings@@3PAIA 0x005AC690
// RankingFactionIDUnk1 at 0x009A650C -> ranking_faction_id_unk1_; former source src/faction.cpp:567; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// RankingFactionIDUnk2 at 0x009A6510 -> ranking_faction_id_unk2_; former source src/faction.cpp:568; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// SecretProject at 0x009A6514 -> secret_project_; former source src/alpha.cpp:147; no explicit initializer; static zero initialization.
// Original identity (src/alpha.cpp): // RECOVERY_OBJECT: v2 0x009A6514 0x009A6614 ?SecretProject@@3UBaseSecretProject@@A 0x005002E0
// GovernorFaction at 0x009A6614 -> governor_faction_; former source src/base.cpp:5156; no explicit initializer; static zero initialization.
// Original identity (src/base.cpp): // RECOVERY_OBJECT: v2 0x009A6614 0x009A6618 ?GovernorFaction@@3HA 0x00424410
// CouncilResolutionCount at 0x009A6618 -> council_resolution_count_; former source src/council.cpp:46; no explicit initializer; static zero initialization.
// Original identity (src/council.cpp): // RECOVERY_OBJECT: v2 0x009A6618 0x009A6644 ?CouncilResolutionCount@@3PAHA 0x005B3920
// CouncilLastResolutionTurn at 0x009A6644 -> council_last_resolution_turn_; former source src/council.cpp:48; no explicit initializer; static zero initialization.
// Original identity (src/council.cpp): // RECOVERY_OBJECT: v2 0x009A6644 0x009A6670 ?CouncilLastResolutionTurn@@3PAHA 0x005B3920
// GameTechAchieved at 0x009A6670 -> game_tech_achieved_; former source src/technology.cpp:85; no explicit initializer; static zero initialization.
// Original identity (src/technology.cpp): // RECOVERY_OBJECT: v2 0x009A6670 0x009A66C9 ?GameTechAchieved@@3PAEA 0x004DFAD0
// LegacyGameSaveName at 0x009A66C9 -> legacy_game_save_name_; former source src/game.cpp:2336; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A66C9 0x009A67C9 ?LegacyGameSaveName@@3PADA 0x005A6AC0
// GameWireOpaque_9A67C9 at 0x009A67C9 -> game_wire_opaque_9a67c9_; former source src/game.cpp:2338; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A67C9 0x009A67CC ?GameWireOpaque_9A67C9@@3PAEA 0x00532E00
// ClimateRiseStep at 0x009A67CC -> climate_rise_step_; former source src/game.cpp:3340; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A67CC 0x009A67D0 ?ClimateRiseStep@@3HA 0x00524210
// ClimateRiseLimit at 0x009A67D0 -> climate_rise_limit_; former source src/game.cpp:3342; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A67D0 0x009A67D4 ?ClimateRiseLimit@@3HA 0x00524210
// ClimateRiseAccum at 0x009A67D4 -> climate_rise_accum_; former source src/game.cpp:3344; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A67D4 0x009A67D8 ?ClimateRiseAccum@@3HA 0x00524210
// ClimateSeaLevelDelta at 0x009A67D8 -> climate_sea_level_delta_; former source src/game.cpp:3346; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A67D8 0x009A67DC ?ClimateSeaLevelDelta@@3HA 0x00524210
// TranscendenceScoreBonus at 0x009A67DC -> transcendence_score_bonus_; former source src/base.cpp:10367; no explicit initializer; static zero initialization.
// Original identity (src/base.cpp): // RECOVERY_OBJECT: v2 0x009A67DC 0x009A67E0 ?TranscendenceScoreBonus@@3HA 0x005ACBE0
// TranscendenceScoreWeights at 0x009A67E0 -> transcendence_score_weights_; former source src/base.cpp:10369; no explicit initializer; static zero initialization.
// Original identity (src/base.cpp): // RECOVERY_OBJECT: v2 0x009A67E0 0x009A6800 ?TranscendenceScoreWeights@@3PAHA 0x005ACBE0
// SunspotDuration at 0x009A6800 -> sunspot_duration_; former source src/game.cpp:1345; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// MountPlanetX at 0x009A6804 -> mount_planet_x_; former source src/game.cpp:1496; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// MountPlanetY at 0x009A6808 -> mount_planet_y_; former source src/game.cpp:1497; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// DustCloudDuration at 0x009A680C -> dust_cloud_duration_; former source src/game.cpp:1498; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// SaveDaemonStateWord at 0x009A6810 -> save_daemon_state_word_; former source src/game.cpp:1502; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A6810 0x009A6814 ?SaveDaemonStateWord@@3HA 0x005A94F0
// ScnVictFacilityObj at 0x009A6814 -> scenario_victory_facility_; former source src/base.cpp:318; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// GlobalEnergyEventState at 0x009A6818 -> global_energy_event_state_; former source src/base.cpp:5168; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// NetTurnFlags at 0x009A681C -> net_turn_flags_; former source src/game.cpp:1507; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// NetTurnFaction at 0x009A6820 -> net_turn_faction_; former source src/game.cpp:1508; no explicit initializer; static zero initialization.
// No separate RECOVERY_OBJECT annotation was present for this former global.
// EcologyEventAccumulator at 0x009A6824 -> ecology_event_accumulator_; former source src/game.cpp:1522; no explicit initializer; static zero initialization.
// Original identity (src/game.cpp): // RECOVERY_OBJECT: v2 0x009A6824 0x009A6828 ?EcologyEventAccumulator@@3HA 0x004F67F0

typedef char GameDataStateLockMaskHasNativeExtent[
    sizeof(GameDataLockMaskView) == 4 ? 1 : -1];
typedef char GameDataStateLockMaskBytesHasNativeExtent[
    sizeof(GameDataLockMaskBytes) == 4 ? 1 : -1];
typedef char GameDataStateLockHumanByteAtZero[
    offsetof(GameDataLockMaskBytes, human_mask_) == 0 ? 1 : -1];
typedef char GameDataStateLockAliveByteAtOne[
    offsetof(GameDataLockMaskBytes, alive_mask_) == 1 ? 1 : -1];
typedef char GameDataStateSecretProjectHasNativeExtent[
    sizeof(BaseSecretProject) == 0x100 ? 1 : -1];
typedef char GameDataStateWindowHasNativeExtent[
    sizeof(GameDataStateWindowFields) == 0x398 ? 1 : -1];
typedef char GameDataStateWindowWrapperHasNativeExtent[
    sizeof(GameDataStateWindow) == 0x398 ? 1 : -1];
typedef char GameDataStateTerraformMaskHasNativeExtent[
    sizeof(((GameDataStateWindowFields *)0)->terraform_announced_mask_) == 8 ? 1 : -1];
typedef char GameDataStateTechnologyBytesHasNativeExtent[
    sizeof(((GameDataStateWindowFields *)0)->game_tech_achieved_) == 89 ? 1 : -1];
typedef char GameDataStateOffset_game_preferences_[
    offsetof(GameDataStateWindowFields, game_preferences_) == 0x000 ? 1 : -1];
typedef char GameDataStateOffset_game_more_preferences_[
    offsetof(GameDataStateWindowFields, game_more_preferences_) == 0x004 ? 1 : -1];
typedef char GameDataStateOffset_game_warnings_[
    offsetof(GameDataStateWindowFields, game_warnings_) == 0x008 ? 1 : -1];
typedef char GameDataStateOffset_game_rules_[
    offsetof(GameDataStateWindowFields, game_rules_) == 0x00C ? 1 : -1];
typedef char GameDataStateOffset_game_wire_opaque_9a64a0_[
    offsetof(GameDataStateWindowFields, game_wire_opaque_9a64a0_) == 0x010 ? 1 : -1];
typedef char GameDataStateOffset_game_interlude_flags_[
    offsetof(GameDataStateWindowFields, game_interlude_flags_) == 0x014 ? 1 : -1];
typedef char GameDataStateOffset_game_wire_opaque_9a64a8_[
    offsetof(GameDataStateWindowFields, game_wire_opaque_9a64a8_) == 0x018 ? 1 : -1];
typedef char GameDataStateOffset_tutorial_flags_[
    offsetof(GameDataStateWindowFields, tutorial_flags_) == 0x01C ? 1 : -1];
typedef char GameDataStateOffset_tutorial_flags_2_[
    offsetof(GameDataStateWindowFields, tutorial_flags_2_) == 0x020 ? 1 : -1];
typedef char GameDataStateOffset_terraform_announced_mask_[
    offsetof(GameDataStateWindowFields, terraform_announced_mask_) == 0x024 ? 1 : -1];
typedef char GameDataStateOffset_tutwin_flags_9a64bc_[
    offsetof(GameDataStateWindowFields, tutwin_flags_9a64bc_) == 0x02C ? 1 : -1];
typedef char GameDataStateOffset_game_state_[
    offsetof(GameDataStateWindowFields, game_state_) == 0x030 ? 1 : -1];
typedef char GameDataStateOffset_diff_level_current_[
    offsetof(GameDataStateWindowFields, diff_level_current_) == 0x034 ? 1 : -1];
typedef char GameDataStateOffset_vehicle_current_count_[
    offsetof(GameDataStateWindowFields, vehicle_current_count_) == 0x038 ? 1 : -1];
typedef char GameDataStateOffset_base_current_count_[
    offsetof(GameDataStateWindowFields, base_current_count_) == 0x03C ? 1 : -1];
typedef char GameDataStateOffset_game_setup_reset_word_[
    offsetof(GameDataStateWindowFields, game_setup_reset_word_) == 0x040 ? 1 : -1];
typedef char GameDataStateOffset_turn_current_num_[
    offsetof(GameDataStateWindowFields, turn_current_num_) == 0x044 ? 1 : -1];
typedef char GameDataStateOffset_mission_year_current_[
    offsetof(GameDataStateWindowFields, mission_year_current_) == 0x048 ? 1 : -1];
typedef char GameDataStateOffset_vehicle_selection_follow_limit_[
    offsetof(GameDataStateWindowFields, vehicle_selection_follow_limit_) == 0x04C ? 1 : -1];
typedef char GameDataStateOffset_time_controls_selection_[
    offsetof(GameDataStateWindowFields, time_controls_selection_) == 0x050 ? 1 : -1];
typedef char GameDataStateOffset_turn_control_duration_milliseconds_[
    offsetof(GameDataStateWindowFields, turn_control_duration_milliseconds_) == 0x054 ? 1 : -1];
typedef char GameDataStateOffset_lock_masks_[
    offsetof(GameDataStateWindowFields, lock_masks_) == 0x058 ? 1 : -1];
typedef char GameDataStateOffset_faction_rankings_[
    offsetof(GameDataStateWindowFields, faction_rankings_) == 0x05C ? 1 : -1];
typedef char GameDataStateOffset_ranking_faction_id_unk1_[
    offsetof(GameDataStateWindowFields, ranking_faction_id_unk1_) == 0x07C ? 1 : -1];
typedef char GameDataStateOffset_ranking_faction_id_unk2_[
    offsetof(GameDataStateWindowFields, ranking_faction_id_unk2_) == 0x080 ? 1 : -1];
typedef char GameDataStateOffset_secret_project_[
    offsetof(GameDataStateWindowFields, secret_project_) == 0x084 ? 1 : -1];
typedef char GameDataStateOffset_governor_faction_[
    offsetof(GameDataStateWindowFields, governor_faction_) == 0x184 ? 1 : -1];
typedef char GameDataStateOffset_council_resolution_count_[
    offsetof(GameDataStateWindowFields, council_resolution_count_) == 0x188 ? 1 : -1];
typedef char GameDataStateOffset_council_last_resolution_turn_[
    offsetof(GameDataStateWindowFields, council_last_resolution_turn_) == 0x1B4 ? 1 : -1];
typedef char GameDataStateOffset_game_tech_achieved_[
    offsetof(GameDataStateWindowFields, game_tech_achieved_) == 0x1E0 ? 1 : -1];
typedef char GameDataStateOffset_legacy_game_save_name_[
    offsetof(GameDataStateWindowFields, legacy_game_save_name_) == 0x239 ? 1 : -1];
typedef char GameDataStateOffset_game_wire_opaque_9a67c9_[
    offsetof(GameDataStateWindowFields, game_wire_opaque_9a67c9_) == 0x339 ? 1 : -1];
typedef char GameDataStateOffset_climate_rise_step_[
    offsetof(GameDataStateWindowFields, climate_rise_step_) == 0x33C ? 1 : -1];
typedef char GameDataStateOffset_climate_rise_limit_[
    offsetof(GameDataStateWindowFields, climate_rise_limit_) == 0x340 ? 1 : -1];
typedef char GameDataStateOffset_climate_rise_accum_[
    offsetof(GameDataStateWindowFields, climate_rise_accum_) == 0x344 ? 1 : -1];
typedef char GameDataStateOffset_climate_sea_level_delta_[
    offsetof(GameDataStateWindowFields, climate_sea_level_delta_) == 0x348 ? 1 : -1];
typedef char GameDataStateOffset_transcendence_score_bonus_[
    offsetof(GameDataStateWindowFields, transcendence_score_bonus_) == 0x34C ? 1 : -1];
typedef char GameDataStateOffset_transcendence_score_weights_[
    offsetof(GameDataStateWindowFields, transcendence_score_weights_) == 0x350 ? 1 : -1];
typedef char GameDataStateOffset_sunspot_duration_[
    offsetof(GameDataStateWindowFields, sunspot_duration_) == 0x370 ? 1 : -1];
typedef char GameDataStateOffset_mount_planet_x_[
    offsetof(GameDataStateWindowFields, mount_planet_x_) == 0x374 ? 1 : -1];
typedef char GameDataStateOffset_mount_planet_y_[
    offsetof(GameDataStateWindowFields, mount_planet_y_) == 0x378 ? 1 : -1];
typedef char GameDataStateOffset_dust_cloud_duration_[
    offsetof(GameDataStateWindowFields, dust_cloud_duration_) == 0x37C ? 1 : -1];
typedef char GameDataStateOffset_save_daemon_state_word_[
    offsetof(GameDataStateWindowFields, save_daemon_state_word_) == 0x380 ? 1 : -1];
typedef char GameDataStateOffset_scenario_victory_facility_[
    offsetof(GameDataStateWindowFields, scenario_victory_facility_) == 0x384 ? 1 : -1];
typedef char GameDataStateOffset_global_energy_event_state_[
    offsetof(GameDataStateWindowFields, global_energy_event_state_) == 0x388 ? 1 : -1];
typedef char GameDataStateOffset_net_turn_flags_[
    offsetof(GameDataStateWindowFields, net_turn_flags_) == 0x38C ? 1 : -1];
typedef char GameDataStateOffset_net_turn_faction_[
    offsetof(GameDataStateWindowFields, net_turn_faction_) == 0x390 ? 1 : -1];
typedef char GameDataStateOffset_ecology_event_accumulator_[
    offsetof(GameDataStateWindowFields, ecology_event_accumulator_) == 0x394 ? 1 : -1];

#endif
