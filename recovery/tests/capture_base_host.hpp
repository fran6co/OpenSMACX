#ifndef CAPTURE_BASE_HOST_PROJECTION_HPP
#define CAPTURE_BASE_HOST_PROJECTION_HPP
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
    int human_genome_project_, command_nexus_, weather_paradigm_, merchant_exchange_;
    int empath_guild_, citizens_defense_force_, virtual_world_, planetary_transit_system_;
    int xenoempathy_dome_, neural_amplifier_, maritime_control_center_, planetary_datalinks_;
    int supercollider_, ascetic_virtues_, longevity_vaccine_, hunter_seeker_algorithm_;
    int pholus_mutagen_, cyborg_factory_, theory_of_everything_, dream_twister_;
    int universal_translator_, network_backbone_;

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
};

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
extern int BaseCurrentCount, VehCurrentCount, TurnCurrentNum;
extern unsigned int MapLongitude, LockEnableMask, GamePreferences, GameRules,
    GameState, UnkBitfield1;
extern int MapLongitudeBounds, MapLatitudeBounds, MapIsFlat, IsMultiplayerNet,
    BaseIDCurrentSelected;
extern Console ConsoleGlobal;
extern NetMessage NetMsgPrimary;
extern Window WorldClimateWorldWin, TutWinBaseWindow;
extern BaseSecretProject SecretProject;
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
void draw_tile(int, int, int);
char *label_get(int);

extern "C" void capture_base(int, int, int);

#endif
