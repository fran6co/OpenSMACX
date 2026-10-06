/* Host-only typed-owner projection. These simplified structs are not native ABI evidence. */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define __cdecl

/*ACTUAL_PROJECT_OWNER*/
struct Base { unsigned char faction_id_current; int labs_total, economy_total; };
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    SocialEffect soc_effect_pending;
    uint32_t diplo_treaties[8];
    int loan_balance[8], loan_payment[8], sanction_turns;
};
static BaseSecretProject SecretProject;
static Base Bases[512];
static PlayerData PlayersData[8];
static int BaseCurrentCount, BreakthroughTurnsGlobal, EnergyLabsTotal;
static int NetIncomeGlobal, EnergyMaintenanceTotal, EnergyEconomyTotal, EnergyCommerceDelta;
static int EnergyFacilityCounts[70], EnergyBaseMaintenance[512];
static uint32_t LockEnableMask;
static int SocialEffectTable[8 * 53 + 8 * 11 + 64];
static int SocialEffectRow[64];
static int test_maintenance[512], test_facility_count[512];
static int test_tech_rate;
static int test_set_base_calls, test_base_compute_calls;
static int active_base, mutate_selector_on_base, selector_after_callback;
static void set_base(int base_id) {
    ++test_set_base_calls; active_base = base_id;
    if (base_id == mutate_selector_on_base)
        SecretProject.self_aware_colony() = selector_after_callback;
}
static void base_compute(int) { ++test_base_compute_calls; }
static int has_fac_built(int facility, int base_id) {
    return facility < test_facility_count[base_id];
}
static int fac_maint(int facility, int) { return test_maintenance[facility]; }
static int tech_rate(int) { return test_tech_rate; }
struct SocialWin { int factionID_; int immune(int, int, int); };
/*ACTUAL_METHODS*/

static void reset_fixture() {
    /*ACTUAL_RESET*/
    memset(Bases, 0, sizeof(Bases)); memset(PlayersData, 0, sizeof(PlayersData));
    memset(EnergyBaseMaintenance, 0, sizeof(EnergyBaseMaintenance));
    memset(test_maintenance, 0, sizeof(test_maintenance));
    memset(test_facility_count, 0, sizeof(test_facility_count));
    memset(EnergyFacilityCounts, 0, sizeof(EnergyFacilityCounts));
    memset(SocialEffectTable, 0, sizeof(SocialEffectTable));
    memset(SocialEffectRow, 0, sizeof(SocialEffectRow));
    BaseCurrentCount = 0; LockEnableMask = 0; test_tech_rate = 0;
    test_set_base_calls = test_base_compute_calls = 0;
    active_base = mutate_selector_on_base = -1; selector_after_callback = -1;
    EnergyLabsTotal = NetIncomeGlobal = EnergyMaintenanceTotal = 0;
    EnergyEconomyTotal = EnergyCommerceDelta = BreakthroughTurnsGlobal = 0;
}
static void test_immune_owners() {
    reset_fixture();
    SocialWin win; win.factionID_ = 3;
    SecretProject.cloning_vats() = 5;
    SecretProject.network_backbone() = 2;
    Bases[5].faction_id_current = 3;
    Bases[2].faction_id_current = 4;
    SocialEffectTable[1 * 11 + 7 + 2 * 53] = -1;
    SocialEffectRow[9] = -1;
    assert(win.immune(2, 1, 7) == 1);       // cloning vats owns faction 3
    assert(win.immune(3, 1, 9) == 0);       // network backbone belongs to 4
    Bases[2].faction_id_current = 3;
    assert(win.immune(3, 1, 9) == 1);
    Bases[5].faction_id_current = 4;
    assert(win.immune(2, 1, 7) == 0);
    SecretProject.cloning_vats() = -1;
    SecretProject.network_backbone() = -1;
    assert(win.immune(2, 1, 7) == 0 && win.immune(3, 1, 9) == 0);
}
static void test_energy_owner_and_carry() {
    reset_fixture();
    BaseCurrentCount = 3;
    Bases[0].faction_id_current = 2; Bases[0].economy_total = 20; Bases[0].labs_total = 1;
    Bases[1].faction_id_current = 2; Bases[1].economy_total = 20; Bases[1].labs_total = 1;
    Bases[2].faction_id_current = 4; Bases[2].economy_total = 20; Bases[2].labs_total = 1;
    test_facility_count[0] = test_facility_count[1] = test_facility_count[2] = 1;
    test_maintenance[0] = 5;
    SecretProject.self_aware_colony() = 1;
    energy_compute(2, 0);
    assert(EnergyBaseMaintenance[0] == 2); // first odd base truncates after halving
    assert(EnergyBaseMaintenance[1] == 3); // carried odd unit rounds the next base up
    assert(EnergyBaseMaintenance[2] == 0);
    assert(EnergyMaintenanceTotal == 5 && NetIncomeGlobal == 35);

    Bases[1].faction_id_current = 4;
    energy_compute(2, 0);
    assert(EnergyBaseMaintenance[0] == 5 && EnergyBaseMaintenance[1] == 0);
    assert(EnergyMaintenanceTotal == 5 && NetIncomeGlobal == 15);
    SecretProject.self_aware_colony() = 99;
    energy_compute(2, 0);
    assert(EnergyBaseMaintenance[0] == 5 && EnergyMaintenanceTotal == 5);
    SecretProject.self_aware_colony() = -1;
    energy_compute(2, 0);
    assert(EnergyBaseMaintenance[0] == 5 && EnergyMaintenanceTotal == 5);

    // set_base is an external callback. The native loop reloads the project
    // owner at each iteration, so a callback update before base 1 is observed.
    reset_fixture();
    BaseCurrentCount = 2;
    Bases[0].faction_id_current = Bases[1].faction_id_current = 2;
    test_facility_count[0] = test_facility_count[1] = 1;
    test_maintenance[0] = 5;
    SecretProject.self_aware_colony() = -1;
    mutate_selector_on_base = 1; selector_after_callback = 1;
    energy_compute(2, 1);
    assert(test_set_base_calls == 2 && test_base_compute_calls == 2);
    assert(EnergyBaseMaintenance[0] == 5 && EnergyBaseMaintenance[1] == 2);
}
static void test_reset_and_remap() {
    reset_fixture();
    assert(sizeof(SecretProject) == 64 * sizeof(int));
    assert((reinterpret_cast<const char *>(&SecretProject.empath_guild()) - reinterpret_cast<const char *>(&SecretProject)) == 0x10);
    assert((reinterpret_cast<const char *>(&SecretProject.network_backbone()) - reinterpret_cast<const char *>(&SecretProject)) == 0x54);
    assert((reinterpret_cast<const char *>(&SecretProject.cloning_vats()) - reinterpret_cast<const char *>(&SecretProject)) == 0x60);
    assert((reinterpret_cast<const char *>(&SecretProject.self_aware_colony()) - reinterpret_cast<const char *>(&SecretProject)) == 0x64);
    assert(SecretProject.empath_guild() == -1 && SecretProject.network_backbone() == -1);
    assert(SecretProject.cloning_vats() == -1 && SecretProject.self_aware_colony() == -1);
    SecretProject.human_genome_project() = 2;
    SecretProject.empath_guild() = 7; SecretProject.network_backbone() = 4;
    SecretProject.cloning_vats() = 5; SecretProject.self_aware_colony() = 6;
    base_remap_projects(4);
    assert(SecretProject.human_genome_project() == 2);
    assert(SecretProject.empath_guild() == 6);
    assert(SecretProject.network_backbone() == -2);
    assert(SecretProject.cloning_vats() == 4);
    assert(SecretProject.self_aware_colony() == 5);
}
int main() {
    test_immune_owners();
    test_energy_owner_and_carry();
    test_reset_and_remap();
    return 0;
}
