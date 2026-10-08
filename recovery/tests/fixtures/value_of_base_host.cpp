#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define __cdecl
typedef int BOOL;

struct HostBase {
    int x;
    int y;
    uint32_t faction_id_current;
    int8_t population_size;
};
struct HostPlayerData {
    int energy_reserves;
    uint8_t region_total_bases[128];
};
struct HostPlayer { char filename[32]; };
struct HostFacility { int cost; };
struct HostVehicle { int next_veh_id_stack; uint32_t faction_id; int proto_id; };
struct HostPrototype { int cost; };

static HostBase Bases[8];
static HostPlayerData PlayersData[8];
static HostPlayer Players[8];
static HostFacility Facility[128];
static HostVehicle Vehs[32];
static HostPrototype VehPrototypes[32];
static int RadiusRange[8];
static int RadiusOffsetX[256];
static int RadiusOffsetY[256];
static int FacilitySPStart;
static int FacilityRepStart;
static int MaxSecretProjectNum;
static int HostDistFactor;
static int HostVehWho;

static int vulnerable(int, int, int) { return HostDistFactor; }
static uint32_t region_at(int, int) { return 0; }
static int xrange(int x) { return x; }
static int on_map(int, int) { return 0; }
static int base_at(int, int) { return -1; }
static int stack_fix(int value) { return value; }
static int veh_at(int, int) { return 0; }
static int veh_who(int, int) { return HostVehWho; }
static int has_fac_built(uint32_t, int) { return 0; }
static int base_project(uint32_t) { return -1; }
static int veh_top(int) { return -1; }
static int is_objective(int) { return 0; }
static int _stricmp(const char *left, const char *right) {
    return strcmp(left, right);
}

/* EXTRACTED_VALUE_OF_BASE_HELPERS */
/* EXTRACTED_VALUE_OF_BASE_BODY */

static void reset_host(int population, int reserves_req, int reserves_res,
                       int request_region_count, int response_region_count) {
    memset(Bases, 0, sizeof(Bases));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Players, 0, sizeof(Players));
    memset(Facility, 0, sizeof(Facility));
    memset(Vehs, 0, sizeof(Vehs));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(RadiusRange, 0, sizeof(RadiusRange));
    memset(RadiusOffsetX, 0, sizeof(RadiusOffsetX));
    memset(RadiusOffsetY, 0, sizeof(RadiusOffsetY));
    RadiusRange[6] = 1;
    FacilitySPStart = 1;
    FacilityRepStart = 1;
    MaxSecretProjectNum = 0;
    HostDistFactor = 1;
    HostVehWho = 0;
    strcpy(Players[0].filename, "DEFAULT");
    strcpy(Players[1].filename, "DEFAULT");
    Bases[0].x = 10;
    Bases[0].y = 20;
    Bases[0].population_size = static_cast<int8_t>(population);
    PlayersData[0].energy_reserves = reserves_req;
    PlayersData[1].energy_reserves = reserves_res;
    PlayersData[0].region_total_bases[0] =
        static_cast<uint8_t>(request_region_count);
    PlayersData[1].region_total_bases[0] =
        static_cast<uint8_t>(response_region_count);
}

static int evaluate(int overmatch, BOOL tgl) {
    return value_of_base(0, 0, 1, overmatch, tgl);
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    if (strcmp(argv[1], "ordinary") == 0) {
        reset_host(5, 1000, 0, 2, 1);
        if (evaluate(0, 0) != 2000) return 10;
    } else if (strcmp(argv[1], "negative_population") == 0) {
        reset_host(-1, 0, 0, 2, 1);
        if (evaluate(0, 0) != 600) return 11;
    } else if (strcmp(argv[1], "signed_clamp") == 0) {
        reset_host(3, -2000, -3000, 2, 1);
        if (evaluate(0, 0) != 100) return 12;
    } else if (strcmp(argv[1], "vehicle_half") == 0) {
        reset_host(3, 1499999000, 0, 2, 1);
        HostVehWho = -1;
        if (evaluate(0, 0) != 450000000) return 13;
    } else if (strcmp(argv[1], "wrapped_halves") == 0) {
        reset_host(3, 1499999000, 0, 2, 10);
        if (evaluate(1, 1) != -173741824) return 14;
    } else {
        return 3;
    }
    return 0;
}
