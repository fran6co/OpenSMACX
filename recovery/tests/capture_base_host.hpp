#ifndef CAPTURE_BASE_HOST_PROJECTION_HPP
#define CAPTURE_BASE_HOST_PROJECTION_HPP
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl
typedef const char *LPCSTR;
typedef unsigned char BOOL;
static const int FAC_HUMAN_GENOME_PROJ = 70;

struct Map {
    unsigned char region, visibility;
    unsigned int bit;
    int val2;
    int climate;
};
struct Base {
    char name_string[32];
    unsigned char faction_id_current, faction_id_former, population_size;
    short x, y;
    int queue_production_id[16];
    unsigned int facilities_built[4], state;
    int nerve_staple_turns_left, assimilation_turns_left;
    int minerals_accumulated, unk_6;
    int queue_size, production_id_last, governor;
    unsigned int visibility;
};
struct PlayerData {
    int diplo_unk4[8], diplo_unk3[8], diplo_treaties[8];
    unsigned int secret_project_intel[8];
    int energy_reserves, pop_total, region_total_bases[128];
    int corner_market_turn, corner_market_active, last_turn_new_base;
    int current_num_bases, proto_id_queue[2048], base_id_atk_target;
    int tech_ranking, tech_accumulated;
};
struct Player {
    char *adj_name_faction, *title_leader, *name_leader, *noun_faction;
    char *assistant_city;
    int is_leader_female, noun_gender, is_noun_plural;
    int faction_bonus_count, faction_bonus_id[16], faction_bonus_val1[16];
    unsigned int rule_flags;
};
struct Veh {
    unsigned char faction_id;
    unsigned int state;
    int next_veh_id_stack;
    short home_base_id;
    short x, y;
};
struct RulesFacility { char *name; int preq_tech; };

struct BaseSecretProject {
    int project_base[64];
    int &operator[](int project) { return project_base[project]; }
    const int &operator[](int project) const { return project_base[project]; }
    int &universal_translator() { return project_base[20]; }
    const int &universal_translator() const { return project_base[20]; }
    int &network_backbone() { return project_base[21]; }
    const int &network_backbone() const { return project_base[21]; }
};

struct GameDataLockMaskBytes {
    uint8_t human_mask_, alive_mask_, unassigned_[2];
};
union GameDataLockMaskView {
    uint32_t enable_mask_;
    GameDataLockMaskBytes bytes_;
};
// Source-shaped GameDataStateWindowFields prefix through the project owner.
// Types/order are copied from src/game_data_state_window.h; this fixture does
// not claim to define or verify the full enclosing object.
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
    uint32_t faction_rankings_[8];
    uint32_t ranking_faction_id_unk1_, ranking_faction_id_unk2_;
    BaseSecretProject secret_project_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
extern GameDataStateWindow GameDataStateStorage;
typedef char game_state_field_offset[(offsetof(GameDataStateWindowFields, game_state_) == 0x30) ? 1 : -1];
typedef char vehicle_count_field_offset[(offsetof(GameDataStateWindowFields, vehicle_current_count_) == 0x38) ? 1 : -1];
typedef char base_count_field_offset[(offsetof(GameDataStateWindowFields, base_current_count_) == 0x3c) ? 1 : -1];
typedef char turn_field_offset[(offsetof(GameDataStateWindowFields, turn_current_num_) == 0x44) ? 1 : -1];
typedef char locks_field_offset[(offsetof(GameDataStateWindowFields, lock_masks_) == 0x58) ? 1 : -1];
typedef char project_field_offset[(offsetof(GameDataStateWindowFields, secret_project_) == 0x84) ? 1 : -1];
typedef char project_array_size[(sizeof(BaseSecretProject) == 0x100) ? 1 : -1];

struct Console {
    int faction_id_;
    int focus(int, int, int) { return 1; }
    void update_data(int) {}
    int ready_search(int) { return 0; }
};
struct NetMessage { void pop(const char *, int, int, int) {} };
struct Window { void redraw() {} void zoom(int, int) {} };

extern Base Bases[512];
extern PlayerData PlayersData[8];
extern Player Players[8];
extern Veh Vehs[8192];
extern RulesFacility Facility[134];
extern Map MapTiles[4096];
extern unsigned int MapLongitude, UnkBitfield1;
extern int MapLongitudeBounds, MapLatitudeBounds, MapIsFlat, IsMultiplayerNet,
    BaseIDCurrentSelected;
extern Console ConsoleGlobal;
extern NetMessage NetMsgPrimary;
extern Window WorldClimateWorldWin, TutWinBaseWindow;
extern char *ScriptPopupFile;
extern int GenderDefault, PluralityDefault;
extern int BaseKillCalls, BaseResetCalls, SetBaseCalls, ComputeCalls;
extern int OwnerSetCalls, DrawCalls, EliminateCalls, StealTechCalls;
extern const int RadiusOffsetX[289], RadiusOffsetY[289];

int base_project(int project_id);
void treaty_off(int, int, int); void treaty_on(int, int, int);
void log_say(const char *, const char *, int, int, int);
void parse_says(int, const char *, int, int); void parse_say(int, int, int, int);
void parse_num(int, int); int popp(const char *, const char *, int, const char *, int (*)());
int X_pop(const char *, int (*)()); void bitmask(int, int *, int *);
void set_fac(int, int, int); void base_reset(int, int); int is_objective(int);
void base_kill(int); void draw_map(int); void replay_base(int, int, int, int);
void using_set(int, int, int); void synch_bit(int, int, int); int veh_at(int, int);
void spot_base(int, int); void reset_territory(); int base_find(int, int, int);
void kill(int); int has_tech(int, int); void base_first(int); void owner_set(int, int, int);
void draw_tile_fixup(int, int, int, int); void ambience(int); int spying(int);
void draw_tile(int, int, int); void interlude(int, char *, int, int); void veh_init(int, int, int, int);
void steal_tech(int, int, int); void bases_reset(int, int, int);
void mon_conquer_base(int, char *); void make_base_unique(int); void text_close();
void eliminate_player(int, int); void set_base(int); void base_compute(int);
char *label_get(int);

extern "C" void capture_base(int, int, int);

#endif
