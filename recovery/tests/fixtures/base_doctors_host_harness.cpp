#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#ifndef __cdecl
#define __cdecl
#endif

enum {
    BSTATE_DRONE_RIOTS_ACTIVE = 2u,
    GOV_MANAGES_CITIZENS_SPECS = 0x40u,
    GOV_ACTIVE = 0x80000000u,
    RFLAG_ALIEN = 0x80u,
    FAC_BROOD_PIT = 35,
    FAC_HAB_COMPLEX = 24,
    FAC_HABITATION_DOME = 25,
    SP_ASCETIC_VIRTUES = 13,
    SP_VIRTUAL_WORLD = 6,
    SP_CLONING_VATS = 24,
    SP_TELEPATHIC_MATRIX = 30,
    CITIZEN_TECHNICIAN = 0,
};
static const int MaxSpecialistNum = 7;

struct Base {
    uint8_t faction_id_current;
    int8_t population_size;
    uint32_t state;
    uint32_t governor;
    int nutrients_accumulated;
    int nutrient_surplus;
    int queue_production_id[10];
    int specialist_total;
    int specialist_unk_1;
    uint32_t spec[2];
    int drone_total;
    int talent_total;
};
static Base Bases[8];
static Base *BaseCurrent;
static int BaseIDCurrentSelected;
static int BaseComputedID;

struct SocialEffect { enum { POLICE = 5 }; int values_[11]; };
struct PlayerData {
    SocialEffect soc_effect_pending;
    int se_alloc_psych;
};
static PlayerData PlayersData[8];
struct Player { int rule_population; uint32_t rule_flags; };
static Player Players[8];
struct RulesBasic {
    uint32_t nutrient_req_citizen;
    uint32_t drones_genejack_factory;
    uint32_t pop_limit_sans_hab_complex;
    uint32_t pop_limit_sans_hab_dome;
    uint32_t min_base_size_specialists;
};
static RulesBasic Rules;
struct RulesCitizen {
    int preq_tech, obsol_tech, ops_bonus, psych_bonus, research_bonus;
};
static RulesCitizen Citizen[MaxSpecialistNum];
struct BaseSecretProject {
    int project_base[64];
    int &operator[](int project) { return project_base[project]; }
};
struct GameDataFields {
    struct { uint32_t enable_mask_; } lock_masks_;
    BaseSecretProject secret_project_;
};
struct GameDataWindow { GameDataFields fields_; };
static GameDataWindow GameDataStateStorage;
static int BaseNutrientAdequate;
static int BaseCurrentGrowthRate;
static int UnkGlobal0093A934;
struct Console { int faction_id_; };
static Console ConsoleGlobal;

static int recompute_calls;
static int recompute_mode;
static int facility_calls;
static int facility_present;
static int cost_factor_calls;
static int popup_calls;
static int has_tech_calls;
static bool switch_base_on_cost_factor;
static bool switch_base_on_has_tech;
static bool switch_base_on_yield;
static std::string popup_label, popup_picture;
static std::vector<std::string> events;

static int base_project(int id) { return GameDataStateStorage.fields_.secret_project_[id]; }
static int has_fac(int id, int, int) {
    ++facility_calls;
    return id == facility_present;
}
static int has_tech(int, int) {
    ++has_tech_calls;
    if (switch_base_on_has_tech) BaseCurrent = &Bases[1];
    return 1;
}
static int cost_factor(int, int, int) {
    ++cost_factor_calls;
    if (switch_base_on_cost_factor) BaseCurrent = &Bases[1];
    return 1;
}
static int popb(const char *label, int, int, const char *picture, void *) {
    ++popup_calls; popup_label = label; popup_picture = picture; return 0;
}
static void base_support() { ++recompute_calls; events.push_back("support"); }
static void base_yield() {
    events.push_back("yield");
    if (switch_base_on_yield) {
        BaseCurrent = &Bases[1];
        switch_base_on_yield = false;
    }
    if (recompute_mode == 1) BaseCurrent->talent_total = 2;
}
static void base_nutrient() { events.push_back("nutrient"); }
static void base_minerals() { events.push_back("minerals"); }
static void base_energy() { events.push_back("energy"); }

/* EXTRACTED_BASE_DOCTORS_HELPERS */
/* EXTRACTED_BASE_DOCTORS_BODY */

static void reset_state() {
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(Players, 0, sizeof(Players));
    std::memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    std::memset(Citizen, 0, sizeof(Citizen));
    BaseIDCurrentSelected = BaseComputedID = 0;
    BaseCurrent = &Bases[0];
    BaseCurrent->faction_id_current = 2;
    BaseCurrent->population_size = 3;
    BaseCurrent->specialist_total = 1;
    Rules.nutrient_req_citizen = 1;
    Rules.drones_genejack_factory = 99;
    Rules.pop_limit_sans_hab_complex = 7;
    Rules.pop_limit_sans_hab_dome = 14;
    Rules.min_base_size_specialists = 5;
    facility_present = 0;
    GameDataStateStorage.fields_.secret_project_[SP_ASCETIC_VIRTUES] = -1;
    GameDataStateStorage.fields_.secret_project_[SP_VIRTUAL_WORLD] = -1;
    GameDataStateStorage.fields_.secret_project_[SP_CLONING_VATS] = -1;
    GameDataStateStorage.fields_.secret_project_[SP_TELEPATHIC_MATRIX] = -1;
    BaseNutrientAdequate = 0;
    BaseCurrentGrowthRate = 0;
    UnkGlobal0093A934 = 0;
    ConsoleGlobal.faction_id_ = 2;
    recompute_calls = recompute_mode = facility_calls = cost_factor_calls = popup_calls = 0;
    has_tech_calls = 0;
    switch_base_on_cost_factor = switch_base_on_has_tech = false;
    switch_base_on_yield = false;
    popup_label.clear(); popup_picture.clear(); events.clear();
}

static void test_human_governor_gate() {
    reset_state();
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1u << 2;
    base_doctors();
    assert(recompute_calls == 0 && BaseCurrent->specialist_total == 1);
}

static void test_riot_doctor_adjustment() {
    reset_state();
    BaseCurrent->state = BSTATE_DRONE_RIOTS_ACTIVE;
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 1;
    BaseCurrent->nutrients_accumulated = 1;
    BaseCurrent->nutrient_surplus = 1;
    PlayersData[2].soc_effect_pending.values_[5] = -2;
    BaseNutrientAdequate = 1;
    BaseCurrentGrowthRate = 6;
    UnkGlobal0093A934 = 1;
    recompute_mode = 1;
    base_doctors();
    assert(cost_factor_calls == 0);
    assert(recompute_calls == 1);
    assert(BaseCurrent->specialist_total == 2);

    reset_state();
    BaseCurrent->state = BSTATE_DRONE_RIOTS_ACTIVE;
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 1;
    BaseCurrent->nutrients_accumulated = 1;
    PlayersData[2].soc_effect_pending.values_[5] = -2;
    BaseNutrientAdequate = 1;
    BaseCurrentGrowthRate = 6;
    Rules.nutrient_req_citizen = 0;
    base_doctors();
    assert(cost_factor_calls == 1);
    assert(recompute_calls == 0);

}

static void test_riot_population_uses_selected_base() {
    reset_state();
    BaseCurrent->state = BSTATE_DRONE_RIOTS_ACTIVE;
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 1;
    BaseCurrent->nutrient_surplus = 1;
    BaseNutrientAdequate = 1;
    BaseCurrentGrowthRate = 6;
    PlayersData[2].soc_effect_pending.values_[5] = -2;
    BaseIDCurrentSelected = 1;
    Bases[1].faction_id_current = 3;
    Bases[1].population_size = 20;
    facility_present = FAC_HAB_COMPLEX;
    base_doctors();
    assert(recompute_calls == 0);
    assert(BaseCurrent->specialist_total == 1);
}

static void test_cost_factor_reloads_current_base() {
    reset_state();
    BaseCurrent->state = BSTATE_DRONE_RIOTS_ACTIVE;
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 1;
    BaseCurrent->population_size = 3;
    BaseCurrent->nutrients_accumulated = 0;
    PlayersData[2].soc_effect_pending.values_[5] = -2;
    BaseNutrientAdequate = 1;
    switch_base_on_cost_factor = true;
    Base *entry = BaseCurrent;
    Bases[1].faction_id_current = 2;
    Bases[1].drone_total = 1;
    Bases[1].talent_total = 1;
    Bases[1].specialist_total = 1;
    Bases[1].nutrients_accumulated = 2;
    Bases[1].nutrient_surplus = 2;
    recompute_mode = 1;
    base_doctors();
    assert(cost_factor_calls == 1);
    assert(BaseCurrent == &Bases[1]);
    assert(entry->specialist_total == 1);
    assert(BaseCurrent->specialist_total == 2);
    assert(recompute_calls == 1);
}

static void test_specialist_callback_reload_and_score_order() {
    reset_state();
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 0;
    BaseNutrientAdequate = 1;
    BaseCurrent->specialist_total = 1;
    Bases[1].faction_id_current = 2;
    Bases[1].population_size = 8;
    Bases[1].specialist_total = 3;
    Citizen[0].psych_bonus = 0;
    Citizen[1].psych_bonus = 1;
    Citizen[1].ops_bonus = 2;
    Citizen[1].research_bonus = 3;
    Rules.min_base_size_specialists = 5;
    switch_base_on_has_tech = true;
    base_doctors();
    assert(has_tech_calls >= 1);
    assert(Bases[0].specialist_total == 1);
    assert(Bases[1].specialist_total == 4);
    assert(((Bases[1].spec[0] >> 12) & 0xFu) == 1u);
}

static void test_recompute_callback_reload() {
    reset_state();
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 0;
    BaseNutrientAdequate = 1;
    Bases[1].faction_id_current = 2;
    switch_base_on_yield = true;
    base_doctors();
    assert(recompute_calls == 1);
    assert(BaseCurrent == &Bases[1]);
    assert(Bases[0].specialist_total == 2);
    assert(Bases[1].specialist_total == 0);
}

static void test_matrix_authorizes_removal_gate() {
    reset_state();
    BaseCurrent->specialist_total = 2;
    BaseCurrent->specialist_unk_1 = 0;
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 1;
    BaseNutrientAdequate = 0;
    base_doctors();
    assert(recompute_calls == 0 && BaseCurrent->specialist_total == 2);

    reset_state();
    BaseCurrent->specialist_total = 2;
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 1;
    BaseNutrientAdequate = 0;
    GameDataStateStorage.fields_.secret_project_[SP_TELEPATHIC_MATRIX] = 1;
    Bases[1].faction_id_current = 2;
    base_doctors();
    assert(recompute_calls == 2 && BaseCurrent->specialist_total == 0);
}

static void test_warning_popup_and_exemptions() {
    reset_state();
    BaseCurrent->specialist_total = 2;
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 0;
    BaseNutrientAdequate = 1;
    UnkGlobal0093A934 = 1;
    recompute_mode = 1;
    base_doctors();
    assert(popup_calls == 1 && popup_label == "PSYCHREQUEST");
    assert(popup_picture == "talent_sm.pcx");

    reset_state();
    BaseCurrent->specialist_total = 2;
    BaseCurrent->drone_total = 1;
    BaseCurrent->talent_total = 0;
    BaseNutrientAdequate = 1;
    UnkGlobal0093A934 = 1;
    PlayersData[2].se_alloc_psych = 2;
    recompute_mode = 1;
    base_doctors();
    assert(popup_calls == 0);
}

int main() {
    (void)&has_fac;
    (void)&cost_factor;
    (void)&popb;
    test_human_governor_gate();
    test_riot_doctor_adjustment();
    test_riot_population_uses_selected_base();
    test_cost_factor_reloads_current_base();
    test_specialist_callback_reload_and_score_order();
    test_recompute_callback_reload();
    test_matrix_authorizes_removal_gate();
    test_warning_popup_and_exemptions();
}
