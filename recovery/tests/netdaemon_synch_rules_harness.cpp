// Host-only C++03 layout doubles; the test injects definitions extracted from
// src/alpha.cpp into the SOURCE EXTRACTS marker.
// These are not source declarations or VC6 ABI proof: pointers are represented
// as uint32_t to model the pinned 32-bit object fields on a 64-bit host.
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef uint32_t LPSTR;
static const int MaxTimeControlNum = 6;
static const int MaxFacilityNum = 134;
static const int MaxResourceInfoNum = 9;
static const int MaxSocialCatNum = 4;
static const int MaxTechnologyNum = 89;
static const int MaxTerrainNum = 20;
static const int MaxChassisNum = 9;
static const int ChassisStorageCount = 19;
static const int MaxReactorNum = 4;
static const int MaxWeaponNum = 26;
static const int MaxArmorNum = 14;
static const int MaxAbilityNum = 29;
static const int MaxCitizenNum = 10;

struct RulesTimeControl {
    LPSTR name;
    int turn, base, unit, event, extra, refresh, accumulated;
};
struct RulesFacility {
    LPSTR name;
    LPSTR effect;
    uint32_t pad;
    int cost;
    uint32_t maint;
    int preq_tech, free_tech, sp_ai_fight, sp_ai_growth, sp_ai_tech,
        sp_ai_wealth, sp_ai_power;
};
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct RulesSocialCategory {
    LPSTR type;
    int preq_tech[4];
    LPSTR name[4];
    SocialEffect model_effect[4];
};
struct RulesTechnology {
    uint32_t flags;
    LPSTR name;
    char id[8];
    int padding, growth_value, tech_value, wealth_value, power_value;
    int preq_tech_1, preq_tech_2;
};
struct RulesTerraforming {
    LPSTR name, name_sea;
    int preq_tech, preq_tech_sea;
    uint32_t bit, bit_incompatible;
    int rate;
    LPSTR shortcuts;
};
struct RulesChassis {
    unsigned char prefix[0x48];
    uint8_t speed, triad, range, cargo, cost, missile;
    unsigned char sprite_offsets[0x40];
    int16_t preq_tech;
};
struct RulesReactor { LPSTR name, name_short; int16_t preq_tech; uint16_t power; };
struct RulesWeapon {
    LPSTR name, name_short;
    int8_t offense_rating, icon;
    uint8_t mode, cost;
    int16_t preq_tech, padding;
};
struct RulesArmor {
    LPSTR name, name_short;
    int8_t defense_rating;
    uint8_t mode, cost, padding1;
    int16_t preq_tech, padding2;
};
struct RulesAbility {
    LPSTR name, description, abbreviation;
    int cost_factor, unk_1;
    uint32_t flags;
    int16_t preq_tech;
    uint16_t padding;
};
struct RulesCitizen {
    LPSTR singular_name, plural_name;
    int preq_tech, obsol_tech, ops_bonus, psych_bonus, research_bonus;
};

typedef char check_time_size[(sizeof(RulesTimeControl) == 0x20) ? 1 : -1];
typedef char check_facility_size[(sizeof(RulesFacility) == 0x30) ? 1 : -1];
typedef char check_social_size[(sizeof(RulesSocialCategory) == 0xD4) ? 1 : -1];
typedef char check_effect_size[(sizeof(SocialEffect) == 0x2C) ? 1 : -1];
typedef char check_tech_size[(sizeof(RulesTechnology) == 0x2C) ? 1 : -1];
typedef char check_terrain_size[(sizeof(RulesTerraforming) == 0x20) ? 1 : -1];
typedef char check_chassis_size[(sizeof(RulesChassis) == 0x90) ? 1 : -1];
typedef char check_reactor_size[(sizeof(RulesReactor) == 0x0C) ? 1 : -1];
typedef char check_weapon_size[(sizeof(RulesWeapon) == 0x10) ? 1 : -1];
typedef char check_armor_size[(sizeof(RulesArmor) == 0x10) ? 1 : -1];
typedef char check_ability_size[(sizeof(RulesAbility) == 0x1C) ? 1 : -1];
typedef char check_citizen_size[(sizeof(RulesCitizen) == 0x1C) ? 1 : -1];
typedef char check_tech_member_offsets[(offsetof(RulesTechnology, name) == 4 &&
    offsetof(RulesTechnology, id) == 8 && offsetof(RulesTechnology, padding) == 0x10 &&
    offsetof(RulesTechnology, growth_value) == 0x14 &&
    offsetof(RulesTechnology, wealth_value) == 0x1C) ? 1 : -1];
typedef char check_terraform_member_offsets[(offsetof(RulesTerraforming, preq_tech) == 8 &&
    offsetof(RulesTerraforming, bit) == 0x10 && offsetof(RulesTerraforming, rate) == 0x18 &&
    offsetof(RulesTerraforming, shortcuts) == 0x1C) ? 1 : -1];
typedef char check_chassis_member_offsets[(offsetof(RulesChassis, speed) == 0x48 &&
    offsetof(RulesChassis, preq_tech) == 0x8E) ? 1 : -1];
typedef char check_reactor_member_offsets[(offsetof(RulesReactor, preq_tech) == 8) ? 1 : -1];
typedef char check_weapon_member_offsets[(offsetof(RulesWeapon, offense_rating) == 8 &&
    offsetof(RulesWeapon, preq_tech) == 0xC) ? 1 : -1];
typedef char check_armor_member_offsets[(offsetof(RulesArmor, defense_rating) == 8 &&
    offsetof(RulesArmor, preq_tech) == 0xC) ? 1 : -1];
typedef char check_ability_member_offsets[(offsetof(RulesAbility, cost_factor) == 0xC) ? 1 : -1];
typedef char check_citizen_member_offsets[(offsetof(RulesCitizen, preq_tech) == 8 &&
    offsetof(RulesCitizen, psych_bonus) == 0x14) ? 1 : -1];

RulesTimeControl TimeControl[MaxTimeControlNum + 1];
// One sentinel row makes accidental loop extension observable without
// fabricating or permitting an out-of-bounds destination in the fixture.
RulesFacility Facility[MaxFacilityNum + 1];
RulesSocialCategory SocialCategories[MaxSocialCatNum + 1];
RulesTechnology Technology[MaxTechnologyNum + 1];
RulesTerraforming Terraforming[MaxTerrainNum + 1];
RulesChassis Chassis[ChassisStorageCount];
RulesReactor Reactor[MaxReactorNum + 1];
RulesWeapon Weapon[MaxWeaponNum + 1];
RulesArmor Armor[MaxArmorNum + 1];
RulesAbility Ability[MaxAbilityNum + 1];
RulesCitizen Citizen[MaxCitizenNum + 1];
struct RulesBasic { unsigned char bytes[0x134]; };
struct RulesWorldbuilder { unsigned char bytes[0x7C]; };
struct RulesResourceinfo { unsigned char bytes[0x10]; };
uint32_t TechValidCount;
uint32_t TechCommerceCount;
RulesBasic Rules;
RulesWorldbuilder WorldBuilder;
RulesResourceinfo ResourceInfo[9];

/* SOURCE EXTRACTS */

static void fill_bytes(unsigned char *out, size_t size, unsigned int seed) {
    for (size_t i = 0; i < size; ++i) {
        out[i] = static_cast<unsigned char>((seed + i * 37u + (i >> 2)) & 0xFFu);
    }
}

static void expect_range(unsigned char *expected, const unsigned char *wire,
                         int rows, size_t stride, size_t destination_offset,
                         size_t source_offset, size_t size) {
    for (int i = 0; i < rows; ++i) {
        memcpy(expected + i * stride + destination_offset,
               wire + i * stride + source_offset, size);
    }
}

static void compare_after(void *actual, const unsigned char *before,
                          const unsigned char *expected, size_t size) {
    (void)before;
    assert(memcmp(actual, expected, size) == 0);
}

static void time_control_roundtrip() {
    unsigned char wire[MaxTimeControlNum * 0x20];
    unsigned char sentinel_before[0x20];
    fill_bytes(reinterpret_cast<unsigned char *>(TimeControl), sizeof TimeControl, 0x11);
    memcpy(sentinel_before, &TimeControl[MaxTimeControlNum], sizeof sentinel_before);
    fill_bytes(wire, sizeof wire, 0x21);

    uint32_t names[MaxTimeControlNum];
    for (int i = 0; i < MaxTimeControlNum; ++i) {
        names[i] = TimeControl[i].name;
        wire[i * 0x20] ^= 0xA5; // Native compaction ignores staged name word.
        memset(&TimeControl[i].turn, 0xCC, 7 * sizeof(int));
    }
    assert(memcmp(sentinel_before, &TimeControl[MaxTimeControlNum], sizeof sentinel_before) == 0);
    NetDaemonSyncRules::compact_time_controls(wire);
    for (int i = 0; i < MaxTimeControlNum; ++i) {
        assert(TimeControl[i].name == names[i]);
        assert(memcmp(&TimeControl[i].turn, wire + i * 0x20 + 4,
                      7 * sizeof(int)) == 0);
    }
}

static void facility_table_roundtrip() {
    unsigned char wire[MaxFacilityNum * 0x30];
    unsigned char sentinel_before[0x30];
    fill_bytes(reinterpret_cast<unsigned char *>(Facility), sizeof Facility, 0x37);
    memcpy(sentinel_before, &Facility[MaxFacilityNum], sizeof sentinel_before);
    fill_bytes(wire, sizeof wire, 0x41);

    uint32_t names[MaxFacilityNum], effects[MaxFacilityNum];
    for (int i = 0; i < MaxFacilityNum; ++i) {
        names[i] = Facility[i].name;
        effects[i] = Facility[i].effect;
        memset(&Facility[i].pad, 0xD1, 10 * sizeof(uint32_t));
    }
    NetDaemonSyncRules::compact_facility_table(wire);
    for (int i = 0; i < MaxFacilityNum; ++i) {
        assert(Facility[i].name == names[i]);
        assert(Facility[i].effect == effects[i]);
        assert(memcmp(reinterpret_cast<unsigned char *>(&Facility[i]) + 8,
                      wire + i * 0x30 + 8, 0x28) == 0);
    }
    assert(memcmp(sentinel_before, &Facility[MaxFacilityNum], sizeof sentinel_before) == 0);
}

static void social_category_roundtrip() {
    unsigned char wire[MaxSocialCatNum * 0xD4];
    unsigned char sentinel_before[0xD4];
    fill_bytes(reinterpret_cast<unsigned char *>(SocialCategories),
               sizeof SocialCategories, 0x59);
    memcpy(sentinel_before, &SocialCategories[MaxSocialCatNum], sizeof sentinel_before);
    fill_bytes(wire, sizeof wire, 0x61);

    uint32_t types[MaxSocialCatNum];
    uint32_t preq[MaxSocialCatNum][4];
    uint32_t names[MaxSocialCatNum][4];
    for (int i = 0; i < MaxSocialCatNum; ++i) {
        types[i] = SocialCategories[i].type;
        memcpy(&preq[i][0], wire + i * 0xD4 + 0x04, sizeof preq[i][0]);
        for (int j = 0; j < 4; ++j) {
            if (j != 0) {
                preq[i][j] = static_cast<uint32_t>(SocialCategories[i].preq_tech[j]);
            }
            names[i][j] = SocialCategories[i].name[j];
        }
        SocialCategories[i].type ^= 0x77777777u;
        SocialCategories[i].preq_tech[0] ^= 0x01010101;
        for (int j = 1; j < 4; ++j) {
            SocialCategories[i].preq_tech[j] ^= 0x02020202;
            SocialCategories[i].name[j] ^= 0x03030303u;
        }
        SocialCategories[i].name[0] ^= 0x03030303u;
        memset(&SocialCategories[i].model_effect, 0xE2,
               sizeof SocialCategories[i].model_effect);
    }
    assert(memcmp(sentinel_before, &SocialCategories[MaxSocialCatNum], sizeof sentinel_before) == 0);
    NetDaemonSyncRules::compact_social_categories(wire);
    for (int i = 0; i < MaxSocialCatNum; ++i) {
        assert(SocialCategories[i].type == (types[i] ^ 0x77777777u));
        assert(static_cast<uint32_t>(SocialCategories[i].preq_tech[0]) == preq[i][0]);
        for (int j = 1; j < 4; ++j) {
            assert(static_cast<uint32_t>(SocialCategories[i].preq_tech[j]) ==
                   (preq[i][j] ^ 0x02020202u));
            assert(SocialCategories[i].name[j] == (names[i][j] ^ 0x03030303u));
        }
        assert(SocialCategories[i].name[0] == (names[i][0] ^ 0x03030303u));
        assert(memcmp(&SocialCategories[i].model_effect, wire + i * 0xD4 + 0x24,
                      sizeof SocialCategories[i].model_effect) == 0);
    }
}

static void technology_compaction() {
    unsigned char wire[MaxTechnologyNum * 0x2C];
    unsigned char before[sizeof Technology], expected[sizeof Technology];
    fill_bytes(reinterpret_cast<unsigned char *>(Technology), sizeof Technology, 0x13);
    fill_bytes(wire, sizeof wire, 0x23);
    memcpy(before, Technology, sizeof before);
    memcpy(expected, before, sizeof expected);
    for (int i = 0; i < MaxTechnologyNum; ++i) {
        unsigned char *dst = expected + i * 0x2C;
        const unsigned char *src = wire + i * 0x2C;
        memcpy(dst + 0x00, src + 0x00, 4);
        memcpy(dst + 0x14, src + 0x14, 0x18);
    }
    NetDaemonSyncRules::compact_technology_fields(wire);
    compare_after(Technology, before, expected, sizeof expected);
}

static void terraforming_compaction() {
    unsigned char wire[MaxTerrainNum * 0x20];
    unsigned char before[sizeof Terraforming], expected[sizeof Terraforming];
    fill_bytes(reinterpret_cast<unsigned char *>(Terraforming), sizeof Terraforming, 0x34);
    fill_bytes(wire, sizeof wire, 0x44);
    memcpy(before, Terraforming, sizeof before);
    memcpy(expected, before, sizeof expected);
    expect_range(expected, wire, MaxTerrainNum, 0x20, 0x08, 0x08, 0x14);
    NetDaemonSyncRules::compact_terraforming_fields(wire);
    compare_after(Terraforming, before, expected, sizeof expected);
}

static void chassis_compaction() {
    unsigned char wire[MaxChassisNum * 0x90];
    unsigned char before[sizeof Chassis], expected[sizeof Chassis];
    fill_bytes(reinterpret_cast<unsigned char *>(Chassis), sizeof Chassis, 0x55);
    fill_bytes(wire, sizeof wire, 0x65);
    memcpy(before, Chassis, sizeof before);
    memcpy(expected, before, sizeof expected);
    expect_range(expected, wire, MaxChassisNum, 0x90, 0x48, 0x48, 6);
    expect_range(expected, wire, MaxChassisNum, 0x90, 0x8E, 0x8E, 2);
    NetDaemonSyncRules::compact_chassis_fields(wire);
    compare_after(Chassis, before, expected, sizeof expected);
}

static void reactor_compaction() {
    unsigned char wire[MaxReactorNum * 0x0C];
    unsigned char before[sizeof Reactor], expected[sizeof Reactor];
    fill_bytes(reinterpret_cast<unsigned char *>(Reactor), sizeof Reactor, 0x76);
    fill_bytes(wire, sizeof wire, 0x86);
    memcpy(before, Reactor, sizeof before);
    memcpy(expected, before, sizeof expected);
    expect_range(expected, wire, MaxReactorNum, 0x0C, 0x08, 0x08, 2);
    NetDaemonSyncRules::compact_reactor_prerequisites(wire);
    compare_after(Reactor, before, expected, sizeof expected);
}

static void weapon_compaction() {
    unsigned char wire[MaxWeaponNum * 0x10];
    unsigned char before[sizeof Weapon], expected[sizeof Weapon];
    fill_bytes(reinterpret_cast<unsigned char *>(Weapon), sizeof Weapon, 0x97);
    fill_bytes(wire, sizeof wire, 0xA7);
    memcpy(before, Weapon, sizeof before);
    memcpy(expected, before, sizeof expected);
    expect_range(expected, wire, MaxWeaponNum, 0x10, 0x08, 0x08, 6);
    NetDaemonSyncRules::compact_weapon_fields(wire);
    compare_after(Weapon, before, expected, sizeof expected);
}

static void armor_compaction() {
    unsigned char wire[MaxArmorNum * 0x10];
    unsigned char before[sizeof Armor], expected[sizeof Armor];
    fill_bytes(reinterpret_cast<unsigned char *>(Armor), sizeof Armor, 0xB8);
    fill_bytes(wire, sizeof wire, 0xC8);
    memcpy(before, Armor, sizeof before);
    memcpy(expected, before, sizeof expected);
    expect_range(expected, wire, MaxArmorNum, 0x10, 0x08, 0x08, 3);
    expect_range(expected, wire, MaxArmorNum, 0x10, 0x0C, 0x0C, 2);
    NetDaemonSyncRules::compact_armor_fields(wire);
    compare_after(Armor, before, expected, sizeof expected);
}

static void ability_compaction() {
    unsigned char wire[MaxAbilityNum * 0x1C];
    unsigned char before[sizeof Ability], expected[sizeof Ability];
    fill_bytes(reinterpret_cast<unsigned char *>(Ability), sizeof Ability, 0xD9);
    fill_bytes(wire, sizeof wire, 0xE9);
    memcpy(before, Ability, sizeof before);
    memcpy(expected, before, sizeof expected);
    expect_range(expected, wire, MaxAbilityNum, 0x1C, 0x0C, 0x0C, 0x0C);
    expect_range(expected, wire, MaxAbilityNum, 0x1C, 0x18, 0x18, 2);
    NetDaemonSyncRules::compact_ability_fields(wire);
    compare_after(Ability, before, expected, sizeof expected);
}

static void citizen_compaction() {
    unsigned char wire[MaxCitizenNum * 0x1C];
    unsigned char before[sizeof Citizen], expected[sizeof Citizen];
    fill_bytes(reinterpret_cast<unsigned char *>(Citizen), sizeof Citizen, 0xFA);
    fill_bytes(wire, sizeof wire, 0x0A);
    memcpy(before, Citizen, sizeof before);
    memcpy(expected, before, sizeof expected);
    expect_range(expected, wire, MaxCitizenNum, 0x1C, 0x08, 0x08, 0x14);
    NetDaemonSyncRules::compact_citizen_fields(wire);
    compare_after(Citizen, before, expected, sizeof expected);
}

static void direct_rule_routes() {
    unsigned char wire81[0x13C];
    fill_bytes(wire81, sizeof wire81, 0x13);
    assert(NetDaemonSyncRules::commit_staged_rules(0x81, wire81));
    assert(memcmp(&TechValidCount, wire81, 4) == 0);
    assert(memcmp(&TechCommerceCount, wire81 + 4, 4) == 0);
    assert(memcmp(&Rules, wire81 + 8, sizeof Rules) == 0);

    unsigned char wire82[0x7C];
    fill_bytes(wire82, sizeof wire82, 0x82);
    assert(NetDaemonSyncRules::commit_staged_rules(0x82, wire82));
    assert(memcmp(&WorldBuilder, wire82, sizeof WorldBuilder) == 0);

    unsigned char wire84[9 * 0x10];
    fill_bytes(wire84, sizeof wire84, 0x84);
    assert(NetDaemonSyncRules::commit_staged_rules(0x84, wire84));
    assert(memcmp(ResourceInfo, wire84, sizeof ResourceInfo) == 0);
    assert(!NetDaemonSyncRules::commit_staged_rules(0x7F, wire84));
    assert(!NetDaemonSyncRules::commit_staged_rules(0x81, 0));

    // Outgoing gathers cover exactly each native rule extent. The fixture has
    // sentinel rows for fixed tables; those rows must not leak onto the wire.
    const int opcodes[] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
                           0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D};
    const size_t extents[] = {0xF4C, 0x13C, 0x7C, 0x280, 0x90, 0xC0,
                              0x510, 0x30, 0x1A0, 0xE0, 0x32C,
                              0x1920, 0x118, 0x350};
    for (size_t i = 0; i < sizeof opcodes / sizeof opcodes[0]; ++i) {
        unsigned char first = 0;
        assert(NetDaemonSyncRules::copy_rule_wire_chunk(
            opcodes[i], 0, &first, 1, false, 0));
        assert(NetDaemonSyncRules::copy_rule_wire_chunk(
            opcodes[i], extents[i] - 1, &first, 1, false, 0));
        assert(!NetDaemonSyncRules::copy_rule_wire_chunk(
            opcodes[i], extents[i], &first, 1, false, 0));
    }
    unsigned char boundary_wire[10];
    assert(NetDaemonSyncRules::copy_rule_wire_chunk(
        0x81, 1, boundary_wire, sizeof boundary_wire, false, 0));
    assert(memcmp(boundary_wire, reinterpret_cast<unsigned char *>(&TechValidCount) + 1, 3) == 0);
    assert(memcmp(boundary_wire + 3, &TechCommerceCount, 4) == 0);
    assert(memcmp(boundary_wire + 7, &Rules, 3) == 0);

    unsigned char staged[0x1921];
    memset(staged, 0xA5, sizeof staged);
    const unsigned char chunk[3] = {0x10, 0x20, 0x30};
    assert(NetDaemonSyncRules::copy_rule_wire_chunk(
        0x8B, 0x191D, const_cast<unsigned char *>(chunk), sizeof chunk,
        true, staged));
    assert(memcmp(staged + 0x191D, chunk, sizeof chunk) == 0);
    assert(staged[0x1920] == 0xA5);
    assert(!NetDaemonSyncRules::copy_rule_wire_chunk(
        0x8B, 0x191F, const_cast<unsigned char *>(chunk), sizeof chunk,
        true, staged));
}

int main() {
    time_control_roundtrip();
    facility_table_roundtrip();
    social_category_roundtrip();
    technology_compaction();
    terraforming_compaction();
    chassis_compaction();
    reactor_compaction();
    weapon_compaction();
    armor_compaction();
    ability_compaction();
    citizen_compaction();
    direct_rule_routes();
    return 0;
}
