#ifndef VEHICLE_GAMEPLAY_SIX_GAME_DATA_STATE_HOST_H
#define VEHICLE_GAMEPLAY_SIX_GAME_DATA_STATE_HOST_H

// Host-only declaration adapter copied from the product source owner. The
// body controls use the same nested fields and one storage object; this is not
// an application header or native-layout proof.
#include <stdint.h>

struct GameDataLockMaskBytes { uint8_t human_mask_, alive_mask_, unassigned_[2]; };
union GameDataLockMaskView { uint32_t enable_mask_; GameDataLockMaskBytes bytes_; };
struct BaseSecretProject {
    int project_base[64];
    int &operator[](int i) { return project_base[i]; }
    const int &operator[](int i) const { return project_base[i]; }
    int &voice_of_planet() { return project_base[31]; }
    const int &voice_of_planet() const { return project_base[31]; }
    int &universal_translator() { return project_base[20]; }
    const int &universal_translator() const { return project_base[20]; }
    int &xenoempathy_dome() { return project_base[8]; }
    const int &xenoempathy_dome() const { return project_base[8]; }
};
struct GameDataStateWindowFields {
    uint32_t game_preferences_, game_more_preferences_, game_warnings_, game_rules_;
    uint8_t game_wire_opaque_9a64a0_[4];
    uint32_t game_interlude_flags_;
    uint8_t game_wire_opaque_9a64a8_[4];
    int tutorial_flags_, tutorial_flags_2_;
    uint32_t terraform_announced_mask_[2];
    int tutwin_flags_9a64bc_;
    uint32_t game_state_;
    int diff_level_current_, vehicle_current_count_, base_current_count_;
    int game_setup_reset_word_, turn_current_num_;
    uint32_t mission_year_current_;
    int vehicle_selection_follow_limit_, time_controls_selection_;
    int turn_control_duration_milliseconds_;
    GameDataLockMaskView lock_masks_;
    uint32_t faction_rankings_[8], ranking_faction_id_unk1_, ranking_faction_id_unk2_;
    BaseSecretProject secret_project_;
    int governor_faction_, council_resolution_count_[11], council_last_resolution_turn_[11];
    uint8_t game_tech_achieved_[89];
    char legacy_game_save_name_[256];
    uint8_t game_wire_opaque_9a67c9_[3];
    int climate_rise_step_, climate_rise_limit_, climate_rise_accum_, climate_sea_level_delta_;
    int transcendence_score_bonus_, transcendence_score_weights_[8], sunspot_duration_;
    uint32_t mount_planet_x_, mount_planet_y_;
    int dust_cloud_duration_, save_daemon_state_word_;
    uint32_t scenario_victory_facility_;
    int global_energy_event_state_;
    uint32_t net_turn_flags_;
    int net_turn_faction_, ecology_event_accumulator_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
extern GameDataStateWindow GameDataStateStorage;

#endif
