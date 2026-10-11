// Actual source declarations and function bodies are extracted from src/.
// LPSTR is a 32-bit host word, not native pointer ABI proof. Controls execute
// only within declared fixture allocations; unknown neighbor effects are not tested.
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef uint32_t LPSTR;
typedef int BOOL;
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

/* RULE DECLARATIONS */

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
RulesStorageWindow RulesStorage;
typedef char check_rules_window[(offsetof(RulesStorageWindow, tech_valid_count_) == 0 &&
    offsetof(RulesStorageWindow, tech_commerce_count_) == 4 &&
    offsetof(RulesStorageWindow, rules_) == 8 && sizeof(RulesStorageWindow) == 0x13C) ? 1 : -1];
RulesWorldbuilder WorldBuilder;
RulesResourceinfo ResourceInfo[MaxResourceInfoNum];

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
    unsigned char staged[0x1920], original_stage[sizeof staged];
    fill_bytes(staged, sizeof staged, 0x55);
    memcpy(original_stage, staged, sizeof staged);
    const int opcodes[] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
                           0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D};
    const size_t extents[] = {0xF4C, 0x13C, 0x7C, 0x280, 0x90, 0xC0,
                              0x510, 0x30, 0x1A0, 0xE0, 0x32C,
                              0x1920, 0x118, 0x350};
    void *owners[] = {Technology, &RulesStorage, &WorldBuilder, Terraforming,
                     ResourceInfo, TimeControl, Chassis, Reactor, Weapon,
                     Armor, Ability, Facility, Citizen, SocialCategories};
    for (size_t i = 0; i < sizeof opcodes / sizeof opcodes[0]; ++i) {
        const bool direct = opcodes[i] == 0x81 || opcodes[i] == 0x82 || opcodes[i] == 0x84;
        unsigned char wire[0x1920], before[sizeof wire], received[sizeof wire];
        fill_bytes(static_cast<unsigned char *>(owners[i]), extents[i], opcodes[i]);
        memcpy(before, owners[i], extents[i]);
        const size_t first = extents[i] > 0x800 ? 0x800 : extents[i];
        // All nominal outgoing chunks, including final partial chunks.
        for (size_t offset = 0; offset < extents[i]; offset += 0x800) {
            const size_t n = extents[i] - offset < 0x800 ? extents[i] - offset : 0x800;
            assert(NetDaemonSyncRules::copy_rule_wire_chunk(
                opcodes[i], offset, wire + offset, n, false, 0));
        }
        assert(memcmp(before, wire, extents[i]) == 0);
        fill_bytes(received, extents[i], 0xA0 + opcodes[i]);
        memcpy(staged, original_stage, sizeof staged);
        assert(NetDaemonSyncRules::copy_rule_wire_chunk(
            opcodes[i], 0, received, first, true, staged));
        if (direct) {
            assert(memcmp(owners[i], received, first) == 0);
            assert(memcmp(staged, original_stage, sizeof staged) == 0);
            assert(!NetDaemonSyncRules::commit_staged_rules(opcodes[i], staged));
            // Even a negative incoming index cannot displace a MOV owner.
            assert(NetDaemonSyncRules::copy_rule_wire_chunk(
                opcodes[i], static_cast<size_t>(0xFFFFF800u), wire, extents[i], false, 0));
            assert(memcmp(wire, owners[i], extents[i]) == 0);
            assert(NetDaemonSyncRules::copy_rule_wire_chunk(
                opcodes[i], static_cast<size_t>(0xFFFFF800u), received, extents[i], true, staged));
            assert(memcmp(owners[i], received, extents[i]) == 0);
            assert(memcmp(staged, original_stage, sizeof staged) == 0);
        } else {
            assert(memcmp(owners[i], before, extents[i]) == 0);
            assert(memcmp(staged, received, first) == 0);
            assert(memcmp(staged + first, original_stage + first, sizeof staged - first) == 0);
            for (size_t offset = first; offset < extents[i]; offset += 0x800) {
                const size_t n = extents[i] - offset < 0x800 ? extents[i] - offset : 0x800;
                assert(NetDaemonSyncRules::copy_rule_wire_chunk(
                    opcodes[i], offset, received + offset, n, true, staged));
            }
            assert(memcmp(staged, received, extents[i]) == 0);
            assert(memcmp(owners[i], before, extents[i]) == 0);
        }
    }
    assert(!NetDaemonSyncRules::copy_rule_wire_chunk(0x2A, 0, staged, 1, false, 0));

    // Chassis has 19 actual source rows but a nine-row wire payload. Crossing
    // that nominal edge within the actual owner is not clipped.
    unsigned char cross[32];
    fill_bytes(reinterpret_cast<unsigned char *>(Chassis), sizeof Chassis, 0x44);
    assert(NetDaemonSyncRules::copy_rule_wire_chunk(0x86, 0x508, cross, sizeof cross, false, 0));
    assert(memcmp(cross, reinterpret_cast<unsigned char *>(Chassis) + 0x508, sizeof cross) == 0);
    // The shared stage likewise is not clipped to a small opcode's payload.
    unsigned char receive[32]; fill_bytes(receive, sizeof receive, 0xAB);
    assert(NetDaemonSyncRules::copy_rule_wire_chunk(0x87, 0x28, receive, sizeof receive, true, staged));
    assert(memcmp(staged + 0x28, receive, sizeof receive) == 0);
    if (sizeof(size_t) > 4) {
        const size_t wrapped = (static_cast<size_t>(0x10000u) << 16) + 8;
        assert(NetDaemonSyncRules::copy_rule_wire_chunk(0x86, wrapped, cross, sizeof cross, false, 0));
        assert(memcmp(cross, reinterpret_cast<unsigned char *>(Chassis) + 8, sizeof cross) == 0);
        assert(NetDaemonSyncRules::copy_rule_wire_chunk(0x87, wrapped, receive, sizeof receive, true, staged));
        assert(memcmp(staged + 8, receive, sizeof receive) == 0);
    }
    // Direct traffic leaves a partly received larger staged table untouched.
    fill_bytes(staged, sizeof staged, 0x23); memcpy(original_stage, staged, sizeof staged);
    unsigned char direct_wire[0x13C]; fill_bytes(direct_wire, sizeof direct_wire, 0x71);
    assert(NetDaemonSyncRules::copy_rule_wire_chunk(0x81, 0x800, direct_wire, sizeof direct_wire, true, staged));
    assert(memcmp(&TechValidCount, direct_wire, 4) == 0);
    assert(memcmp(&TechCommerceCount, direct_wire + 4, 4) == 0);
    assert(memcmp(&Rules, direct_wire + 8, sizeof Rules) == 0);
    assert(memcmp(staged, original_stage, sizeof staged) == 0);

    // Both overlap directions across all 81 members use the whole parent.
    for (int direction = 0; direction < 2; ++direction) {
        unsigned char expected[sizeof RulesStorage];
        fill_bytes(reinterpret_cast<unsigned char *>(&RulesStorage), sizeof RulesStorage, 0x80 + direction);
        memcpy(expected, &RulesStorage, sizeof expected);
        unsigned char *owner = reinterpret_cast<unsigned char *>(&RulesStorage);
        if (direction == 0) {
            memmove(expected + 3, expected, sizeof expected - 3);
            assert(NetDaemonSyncRules::copy_rule_wire_chunk(0x81, 0x800, owner + 3, sizeof expected - 3, false, 0));
        } else {
            memmove(expected, expected + 3, sizeof expected - 3);
            assert(NetDaemonSyncRules::copy_rule_wire_chunk(0x81, 0x800, owner + 3, sizeof expected - 3, true, staged));
        }
        assert(memcmp(owner, expected, sizeof expected) == 0);
    }
    for (int direction = 0; direction < 2; ++direction) {
        unsigned char expected[sizeof staged];
        fill_bytes(staged, sizeof staged, 0x90 + direction);
        memcpy(expected, staged, sizeof expected);
        const size_t dst = direction ? 0 : 4, src = direction ? 4 : 0;
        memmove(expected + dst, expected + src, 64);
        assert(NetDaemonSyncRules::copy_rule_wire_chunk(0x80, dst, staged + src, 64, true, staged));
        assert(memcmp(staged, expected, sizeof staged) == 0);
    }
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
