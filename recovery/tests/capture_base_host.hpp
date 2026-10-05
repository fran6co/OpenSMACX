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
    int human_genome_project, command_nexus, weather_paradigm, merchant_exchange;
    int empath_guild, citizens_defense_force, virtual_world, planetary_transit_system;
    int xenoempathy_dome, neural_amplifier, maritime_control_center, planetary_datalinks;
    int supercollider, ascetic_virtues, longevity_vaccine, hunter_seeker_algorithm;
    int pholus_mutagen, cyborg_factory, theory_of_everything, dream_twister;
    int universal_translator, network_backbone;
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
