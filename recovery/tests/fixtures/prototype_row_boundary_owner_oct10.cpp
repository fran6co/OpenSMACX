#include <assert.h>
#include <stdint.h>
#include <string.h>

#define __cdecl
typedef int BOOL;

static const int MaxVehProtoFactionNum = 64;
static const int MaxVehProtoNum = 512;
static const int PROTO_ACTIVE = 0x001;
static const uint32_t PROTO_TYPED_COMPLETE = 0x004;
static const int WPN_MODE_TRANSPORT = 3;
static const int WPN_TECTONIC_PAYLOAD = 25;
static const int WPN_FUNGAL_PAYLOAD = 26;
static const int PLAN_TECTONIC_MISSILE = 1;
static const int PLAN_FUNGAL_MISSILE = 2;
static const int PLAN_OFFENSIVE = 3;
static const int PLAN_PLANET_BUSTER = 4;
static const int PLAN_NAVAL_SUPERIORITY = 5;
static const int PLAN_AIR_SUPERIORITY = 6;
static const int PLAN_DEFENSIVE = 7;
static const int PLAN_COMBAT = 8;
static const int PLAN_RECONNAISANCE = 9;
static const int TRIAD_SEA = 1;
static const int TRIAD_AIR = 2;
static const int ABL_AIR_SUPERIORITY = 0x20;
static const int ABL_POLICE_2X = 0x400000;
static const int ABL_ARTILLERY = 0x8000;
static const int ABL_DROP_POD = 0x10000;
static const int ABL_AMPHIBIOUS = 0x8;

struct VehPrototype {
    uint32_t flags;
    int preq_tech;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
    int ability_flags;
    uint8_t cost, carry_capacity;
    int plan, unk_1;
    uint32_t obsolete_factions;
    int combat_factions;
    int icon_offset;
};
struct WeaponRecord { int offense_rating, mode; };
struct ArmorRecord { int defense_rating; };
struct ChassisRecord { int missile, triad, speed; };

static VehPrototype VehPrototypes[MaxVehProtoNum];
static WeaponRecord Weapon[256];
static ArmorRecord Armor[256];
static ChassisRecord Chassis[256];
static int tech_available, tech_calls;

static int has_tech(int technology, int faction) {
    assert(technology == 77 && faction == 1);
    ++tech_calls;
    return tech_available;
}
static int proto_cost(int, int, int, int, int) { return 11; }
static int transport_val(int, int, int) { return 7; }
static int has_abil(int, int) { return 0; }

@@MAKE_PROTO@@

static void reset_case(int candidate_id, int typed_complete, int available) {
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Weapon, 0, sizeof(Weapon));
    memset(Armor, 0, sizeof(Armor));
    memset(Chassis, 0, sizeof(Chassis));
    tech_available = available;
    tech_calls = 0;

    Weapon[1].offense_rating = 1;
    Armor[1].defense_rating = 1;
    Chassis[1].triad = 0;
    Chassis[1].speed = 2;

    VehPrototypes[candidate_id].flags = PROTO_ACTIVE |
        (typed_complete ? PROTO_TYPED_COMPLETE : 0);
    VehPrototypes[candidate_id].preq_tech = 77;
    VehPrototypes[candidate_id].weapon_id = 1;
    VehPrototypes[candidate_id].armor_id = 1;
    VehPrototypes[candidate_id].chassis_id = 1;

    // This final active faction-specific row makes the result flags reveal
    // whether the matching candidate contributed all three component gates,
    // without itself matching any requested component. The production
    // typed-complete bit is 0x4; make_proto's separate unchanged tail test is
    // flags & 0x10, so this sentinel intentionally carries both bits.
    VehPrototypes[127].flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE | 0x010;
    VehPrototypes[127].preq_tech = 88;
}

static void run_case(int candidate_id, int typed_complete, int available,
                     int expected_flags, int expected_tech_calls) {
    reset_case(candidate_id, typed_complete, available);
    make_proto(64, 1, 1, 1, 0, 0);
    assert((int)VehPrototypes[64].flags == expected_flags);
    assert(tech_calls == expected_tech_calls);
}

int main() {
    // Basic row 63 requires its technology in both directions.
    run_case(63, 0, 0, PROTO_ACTIVE, 1);
    run_case(63, 0, 1, 0x115, 1);

    // Row 64 is faction-specific: technology cannot admit an untyped row,
    // while typed-complete state admits it without a technology query.
    run_case(64, 0, 1, PROTO_ACTIVE, 0);
    run_case(64, 1, 0, 0x115, 0);
}
