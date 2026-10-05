// Host-only declarations and bounded behavior doubles for Probe source tests.
// These do not establish target ABI/layout or complete original runtime behavior.
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#define __cdecl
#define nullptr 0
typedef int BOOL;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned char uint1;
typedef unsigned short uint2;
typedef unsigned int uint4;

struct SocialEffect { int economy, efficiency, support, talent, morale, police,
    growth, planet, probe, industry, research; };
struct Base {
    int x, y, faction_id_current, population_size;
    char *name_string;
    int talent_total, drone_total, specialist_unk_1;
    int queue_production_id[8], minerals_accumulated_2, minerals_accumulated;
    int assimilation_turns_left, unk_x, nutrients_accumulated;
    uint32_t facilities_built[16], state, visibility;
};
struct Veh {
    int16_t proto_id, home_base_id, x, y;
    uint8_t faction_id, morale, probe_sabotage_id, terraforming_turns;
    uint8_t moves_expended, order, visibility, probe_action;
    int16_t prev_veh_id_stack;
    uint32_t state;
};
struct VehPrototype {
    int chassis_id, cost, weapon_id, armor_id, ability_flags, reactor_id;
    char *veh_name;
    int plan;
};
struct Chassis { int triad, range, speed; };
struct RulesWeapon { int8_t offense_rating; };
struct Player {
    int rule_flags, faction_bonus_count, faction_bonus_id[16],
        faction_bonus_val1[16];
    int is_leader_female, is_noun_plural, noun_gender;
    int noun_faction, adj_name_faction, title_leader, name_leader;
};
struct PlayerData {
    uint32_t flags, ranking;
    int diff_level, energy_reserves, mind_control_total, tech_ranking;
    int tech_accumulated, global_reputation, unk_108;
    char diplo_patience[8];
    int diplo_friction[8];
    uint32_t diplo_treaties[8], diplo_mind_control[8], stolen_data_count[8];
    SocialEffect soc_effect_active, soc_effect_temp, soc_effect_base;
    int unk_29[11], y_target, ai_growth, ai_wealth, unk_83[128];
    int best_weapon_value, best_armor_value, best_land_speed;
    uint16_t region_total_combat_vehs[128];
    uint8_t region_total_bases[128], region_total_offensive_vehs[128];
    uint16_t region_force_rating[128];
    uint8_t region_base_plan[128], tech_trade_source[92];
    uint8_t proto_id_active[512];
    int diplo_spoke[8];
};
struct BaseSecretProject {
    int hunter_seeker_algorithm, cloudbase_academy, nethack_terminus;
};
struct MonumentRecord { int achieved_, word_8_, year_; };
struct MonumentFaction {
    MonumentRecord records_[21];
    int latest_type_;
    uint8_t opaque_tail_prefix_[16];
};
struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
};
struct ChassisRules { };
struct AbilityRecord { char *name; };
struct MoraleRecord { char *name; };
struct TechnologyRecord { uint32_t flags; };
struct FacilityRecord { char *name; };
struct StringTableOwner { char *get(int) { static char s[256]; return s; } };
struct TutorialBaseWindow { void check_base(int) {} };
struct Rules { int tgl_probe_can_steal_tech; };
struct LockState { uint32_t leading_words_[4]; };
struct NetDaemon { LockState lock_state_[2]; };
struct Console {
    int faction_id_;
    void focus(int, int, int) {}
    void update_data(int) {}
};
struct DialogList { int item(const char *, int) { return 0; } };
struct Sprite { int init(const char *, int, int) { return 0; } };
static int ProbeHostPopupConstructs = 0;
static int ProbeHostPopupDestructs = 0;
static int ProbeHostPopupExecCalls = 0;
static int ProbeHostPopupExecResult = 0;
static int ProbeHostPopupExecSequence[4] = {0, 0, 0, 0};
static int ProbeHostPopupExecSequenceCount = 0;
static int ProbeHostRollThresholds[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static int ProbeHostRollThresholdCount = 0;
struct Popup {
    DialogList dialog_list_;
    Popup() { ++ProbeHostPopupConstructs; }
    ~Popup() { ++ProbeHostPopupDestructs; }
    DialogList &dialogs() { return dialog_list_; }
    void start(...) {}
    int exec(...) {
        const int call_index = ProbeHostPopupExecCalls++;
        return call_index < ProbeHostPopupExecSequenceCount
            ? ProbeHostPopupExecSequence[call_index]
            : ProbeHostPopupExecResult;
    }
    int item(char *, int) { return 0; }
    Sprite &owned_picture() { static Sprite s; return s; }
    void select_owned_picture() {}
};
struct Buffer { static int get_pcx_dimensions(const char *, int *, int *) { return 0; } };
struct NetMessage { void pop(...) {} };
struct FactionMonumentRecord { int achieved_; };
struct ProbeTechnologyScan {
    int known_flag_10;
    int flag_20_entries;
    int known_flag_20_entries;
};

static const int TRIAD_SEA = 1;
static const int WPN_PROBE_TEAM = 1;
static const int ORDER_HOLD = 0;
static const int ORDER_NONE = 1;
static const int RFLAG_ALIEN = 0x20000000;
static const int MaxTechnologyNum = 89;
static const int MaxPlayerNum = 8;
static char PopupStartCaption[] = "";
static char ScriptPopupFile[] = "";
static char *TextBufferGetPtr = nullptr;
static char *StringTemp = nullptr;
static int ProbeTargetFactionID = 0;
static int RankingFactionIDUnk1 = 0;
static int DiploScratchFA3C = 0, DiploScratchFA98 = 0, DiploScratchFA88 = 0;
static char *FactionDiplomacyPortraits[8] = {};
static int MapRandSeed = 0;
static int TurnCurrentNum = 0;
static int LockEnableMask = 0;
static int ExpansionEnabled = 0;
static int IsMultiplayerNet = 0;
static int GameState = 0;
static int VehCurrentCount = 0;
static int BaseCurrentCount = 0;
static Base *BaseCurrent = nullptr;
static int MapIsFlat = 0, MapLongitudeBounds = 64, MapLatitudeBounds = 64;
static const int RadiusOffsetX[21] = {
     0, 1, 2, 1, 0, -1, -2, -1, 0, 2, 2, -2, -2, 1, 3, 3, 1, -1, -3, -3, -1,
};
static const int RadiusOffsetY[21] = {
     0, 0, 1, 2, 1, 0, -1, -2, -2, 2, 2, -2, -1, -1, 3, 3, 1, 1, -1, -3, -3,
};
static int ProbeTargetHasHSA = 0, ProbeHasAlgoEnhancement = 0;
static int ProbeAliveBits = 0, ProbeHsaMarker = 0;
static int ProbeLocalState = 0;
static int ProbeActorFaction = 0, ProbeTargetFaction = 0;
static int ProbeEnergy = 0, ProbeValue = 0;
static int ProbeSelector = 0, ProbeResult = 0;
static int ProbeCached = 0;
static int ProbeChance = 0;
static int ProbeNative = 0;
static int ProbeState = 0;
static int ProbeNetwork = 0;
static int ProbeControl = 0;
static int ProbeIsAlien = 0;
static int ProbeTargetHasProject = 0;
static int ProbeCounter = 0;
static int ProbeSelection = 0;
static int ProbeCost = 0;
static int ProbeResearch = 0;
static int ProbeCurrent = 0;
static int ProbeTemporary = 0;
static int ProbeMarker = 0;
static int ProbeFlags = 0;
static int ProbeFaction = 0;
static int ProbeAmount = 0;
static int ProbeArray[16] = {};
static Base Bases[128] = {};
static Veh Vehs[4096] = {};
static VehPrototype VehPrototypes[128] = {};
static Chassis Chassis[32] = {};
static RulesWeapon Weapon[32] = {};
static Player Players[8] = {};
static PlayerData PlayersData[8] = {};
static BaseSecretProject SecretProject = {};
static MonumentFaction FactionMonuments[8] = {};
static Rules Rules = {};
static NetDaemon NetDaemonNet = {};
static Console ConsoleGlobal = {};
static NetMessage NetMsgPrimary = {};
static int CouncilResolutionCount[16] = {};
static int GenderDefault = 0, PluralityDefault = 0;
static int GameRules = 0, TurnCurrent = 0, Expansion = 0;
static MoraleRecord MoraleRecords[16] = {};
static AbilityRecord AbilityRecords[128] = {};
static TechnologyRecord Technology[128] = {};
static FacilityRecord Facility[128] = {};
static StringTableOwner StringTable = {};
static TutorialBaseWindow TutWinBaseWindow = {};
static MoraleRecord *Morale = MoraleRecords;
static AbilityRecord *Ability = AbilityRecords;
static int *MapTiles = nullptr;

static int abi_index(int x) { return x; }
static int abil_index(int x) { return x; }
static int has_abil(int, int) { return 0; }
static int has_fac(int, int, int) { return 0; }
static int has_project(int, int) { return 0; }
static int base_find(int, int, int) { return -1; }
static int base_on_sea(int, int) { return 0; }
static int is_port(int, int) { return 0; }
static int is_ocean(int, int) { return 0; }
static int region_at(int, int) { return 0; }
static int x_dist(int a, int b) { return std::abs(a-b); }
static int vector_dist(int a, int b) { return std::max(std::abs(a), std::abs(b)); }
static int vector_dist(int, int, int, int) { return 0; }
static int stack_check(int, int, int, int, int) { return 0; }
static int stack_fix(int x) { return x; }
static int veh_at(int, int) { return -1; }
static Map ProbeHostMaps[64 * 64] = {};
static int map_loc_x_dummy = 0;
static Map *map_loc(int x, int y) {
    return &ProbeHostMaps[(y & 63) * 64 + (x & 63)];
}
static int bitmask(int, int *, int *) { return 0; }
static int bitmask(int, uint32_t *, uint32_t *) { return 0; }
static int bitmask(int, uint8_t *, uint8_t *) { return 0; }
static int morale_veh(int, int, int) { return 0; }
static int success_rates(int, int, int, int) { return 0; }
static int mind_control(int, int, int) { return 0; }
static int range(int x, int, int) { return x; }
static int rnd(int, void *) { return 0; }
static int timeGetTime() { return 0; }
static int factions_status_byte(int) { return 0; }
static int aah_ooga(int, int) { return 0; }
static int climactic_battle() { return 0; }
static int is_alien_faction(unsigned int) { return 0; }
static int whose_territory(int, int, int, int *, int) { return -1; }
static int spying(int) { return 0; }
static int base_who(int, int) { return -1; }
static int propose_proto(int, int, int, int, int, int, int, const char *) {
    return -1;
}
static char *get_title(int) { return nullptr; }
static char *get_name(int) { return nullptr; }
static char *get_adjective(int) { return nullptr; }
static char *get_noun(int) { return nullptr; }
static int get_pact_hood(int, int) { return 0; }
static int get_his_her(int, int) { return 0; }
static char *get_him_her(int, int) { return (char *)"host pronoun"; }
static int get_fac_name(int) { return 0; }
static int set_treaty(int, int, int, int) { return 0; }
static int has_treaty(int, int, int) { return 0; }
static int treaty_on(int, int, int) { return 0; }
static int treaty_off(int, int, int) { return 0; }
static int pact_ends(int, int) { return 0; }
static int agenda_on(int, int, int) { return 0; }
static int set_agenda(int, int, int, int) { return 0; }
static int act_of_aggression(int, int) { return 0; }
static int capture_base(int, int, int) { return 0; }
static int owner_set(int, int, int) { return 0; }
static int veh_kill(int) { return 0; }
static int veh_put(int, int, int) { return 0; }
static int veh_skip(int) { return 0; }
static int draw_tiles(int, int, int) { return 0; }
static int draw_tile(int, int, int) { return 0; }
static int clear() { return 0; }
static int says(...) { return 0; }
static int say_num(int) { return 0; }
static int popp(...) { return 0; }
static int X_pop(char *, const char *, int (__cdecl *)()) { return 0; }
static int parse_says(int, ...) { return 0; }
static int parse_say(int, ...) { return 0; }
static int parse_num(int, ...) { return 0; }
static int parse_set(int, int) { return 0; }
static int parse_string(...) { return 0; }
static int parse_it(...) { return 0; }
static int X_text_open(const char *, const char *) { return 0; }
static int text_get() { return 0; }
static int report_intel(int) { return 0; }
static int has_tech(int, int) { return 0; }
static int has_tech_call(int, int) { return 0; }
static int steal_tech(int, int, int) { return 0; }
static int say_tech(int, int) { return 0; }
static int tech_achieved(...) { return 0; }
static int net_tech(...) { return 0; }
static int net_treaty_off(...) { return 0; }
static int net_treaty_on(...) { return 0; }
static int net_set_treaty(...) { return 0; }
static int diplomacy_check(int, int, int) { return 0; }
static int diplomacy_caption(int, int) { return 0; }
static int cause_friction(int, int, int) { return 0; }
static int x_pops_minimal(const char *, const char *, const char *) { return 0; }
static int double_cross(int, int, int) { return 0; }
static int setup_player(...) { return 0; }
static int veh_next_stack_id(int) { return -1; }
static int veh_health(int) { return 0; }
static int spot_stack(...) { return 0; }
static int spot_base(...) { return 0; }
static int set_base(...) { return 0; }
static int base_compute(int) { return 0; }
static int set_fac(...) { return 0; }
static int steal_energy(...) { return 0; }
static int atrocity(...) { return 0; }
static int cost_factor(...) { return 0; }
static int random(int, int) { return 0; }
static int random_reseed(int) { return 0; }
static int wave_it(int) { return 0; }
static int kill(int) { return 0; }
static int set_base(int, int, int) { return 0; }
static int set_fac(int, int) { return 0; }
static int steal_energy(int, int, int) { return 0; }
static int atrocity(int, int, int) { return 0; }
static int cost_factor(int) { return 0; }
static int diplomacy_check(...) { return 0; }
static int diplomacy_caption(...) { return 0; }
static int cause_friction(...) { return 0; }
static void space() {}
static void say_label(int) {}
static int has_project_marker(int) { return 0; }
static int veh_speed(int, int) { return 0; }
static int weap_strat(int, int) { return 0; }
static int arm_strat(int, int) { return 0; }
static int is_alien_faction(int) { return 0; }
