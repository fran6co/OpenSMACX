/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

 /*
  * Map related objects, variables and functions.
  */
enum TerrainAltitudeBit {
    ALT_BIT_OCEAN_TRENCH = 0x0,
    ALT_BIT_OCEAN = 0x20,
    ALT_BIT_OCEAN_SHELF = 0x40,
    ALT_BIT_SHORE_LINE = 0x60,
    ALT_BIT_1_LEVEL_ABOVE_SEA = 0x80,
    ALT_BIT_2_LEVELS_ABOVE_SEA = 0xA0,
    ALT_BIT_3_LEVELS_ABOVE_SEA = 0xC0,
};

enum TerrainAltitude {
    ALT_OCEAN_TRENCH = 0,
    ALT_OCEAN = 1,
    ALT_OCEAN_SHELF = 2,
    ALT_SHORE_LINE = 3,
    ALT_1_LEVEL_ABOVE_SEA = 4,
    ALT_2_LEVELS_ABOVE_SEA = 5,
    ALT_3_LEVELS_ABOVE_SEA = 6,
};

enum TerrainRainfallBit { // land modifiers only
    RAINFALL_ARID = 0x0,
    RAINFALL_MOIST = 0x8, // can only have one value set, otherwise tile glitches
    RAINFALL_RAINY = 0x10, // " "
};

enum TerrainRockBit { // land modifiers only
    TERRAIN_BIT_ROLLING = 0x40, // can only have one value set, otherwise game will crash
    TERRAIN_BIT_ROCKY = 0x80, // " "
};

enum TerrainRockiness { // land modifiers only
    ROCKINESS_FLAT = 0,
    ROCKINESS_ROLLING = 1, // can only have one value set, otherwise game will crash
    ROCKINESS_ROCKY = 2, // " "
};

enum TerrainBit {  // was `: uint32_t`; C++11, and the compiler picks unsigned anyway for 0x80000000
    BIT_BASE_IN_TILE = 0x1,
    BIT_VEH_IN_TILE = 0x2,
    BIT_ROAD = 0x4,
    BIT_MAGTUBE = 0x8,
    BIT_MINE = 0x10,
    BIT_FUNGUS = 0x20,
    BIT_SOLAR_TIDAL = 0x40,
    BIT_RIVER = 0x80,
    BIT_RIVERBED = 0x100, // River source for world river creation and visual effect
    BIT_RIVER_LAKE = 0x200, // Shows more water for visual effect
    BIT_RSC_BONUS = 0x400, // Mineral, Nutrient, Energy
    BIT_BUNKER = 0x800,
    BIT_BASE_RADIUS = 0x1000, // production radius; 21 tiles per base (20 surrounding + base tile)
    BIT_MONOLITH = 0x2000,
    BIT_UNK_4000 = 0x4000, // continent + single tile island off coast, used by AI; boundary?
    BIT_FARM = 0x8000,
    BIT_ENERGY_RSC = 0x10000,
    BIT_MINERAL_RSC = 0x20000,
    BIT_AIRBASE = 0x40000,
    BIT_SOIL_ENRICHER = 0x80000,
    BIT_SUPPLY_REMOVE = 0x100000, // Prevent randomly generated pods from appearing here
    BIT_FOREST = 0x200000,
    BIT_CONDENSER = 0x400000,
    BIT_ECH_MIRROR = 0x800000,
    BIT_THERMAL_BORE = 0x1000000,
    BIT_UNK_2000000 = 0x2000000, // related to monoliths
    BIT_UNK_4000000 = 0x4000000, // related to pods
    BIT_UNK_8000000 = 0x8000000, // related to pods
    BIT_SUPPLY_POD = 0x10000000,
    BIT_NUTRIENT_RSC = 0x20000000,
    BIT_UNK_40000000 = 0x40000000, // ?
    BIT_SENSOR_ARRAY = 0x80000000,
};

enum TerrainLandmarkBit2 {
    BIT2_CRATER = 0x1,
    BIT2_VOLCANO = 0x2,
    BIT2_JUNGLE = 0x4,
    BIT2_URANIUM = 0x8,
    BIT2_SARGASSO = 0x10,
    BIT2_RUINS = 0x20,
    BIT2_DUNES = 0x40,
    BIT2_FRESH = 0x80,
    BIT2_MESA = 0x100,
    BIT2_CANYON = 0x200,
    BIT2_GEOTHERMAL = 0x400,
    BIT2_RIDGE = 0x800,
    BIT2_BOREHOLE = 0x1000,
    BIT2_NEXUS = 0x2000,
    BIT2_UNITY = 0x4000,
    BIT2_FOSSIL = 0x8000,
    //
    BIT2_UNK_80000000 = 0x80000000,
};

enum TerrainLandmarkId {
    LM_CRATER = 0,
    LM_VOLCANO = 1,
    LM_JUNGLE = 2,
    LM_URANIUM = 3,
    LM_SARGASSO = 4,
    LM_RUINS = 5,
    LM_DUNES = 6,
    LM_FRESH = 7,
    LM_MESA = 8,
    LM_CANYON = 9,
    LM_GEOTHERMAL = 10,
    LM_RIDGE = 11,
    LM_BOREHOLE = 12,
    LM_NEXUS = 13,
    LM_UNITY = 14,
    LM_FOSSIL = 15,
};

enum ResourceType {
    RSC_NUTRIENTS = 0,
    RSC_MINERALS = 1,
    RSC_ENERGY = 2,
    RSC_PSI = 3, // dropped mechanic
};

/*
* Region notes:
* A tile's region is visible with debug mode in the bottom left of the main interface tile area
* Region id is used to index the [128] planning variable arrays in player_data struct
*
* 0: bad region, n/a
* 1...62: any offshoots from poles starting at 1,1 up to continents and islands
* 63: bad region - both poles land + single tile islands (this seems specific to huge Planet map)
* 64: bad region, n/a
* 65...126: start of water bodies from 1,1 onward (usually primary ocean) to smaller enclosed
* 127: bad region - both poles water
* 128: bad region, n/a
*/
struct Map {
    uint8_t climate; // 000 00 000 | altitude (3 bit) ; rainfall (2 bit) ; temperature (3 bit)
    uint8_t contour; // altitude details
    uint8_t val2; // 0000 0000 | site (0xF0) ; owner (0x0F) - last immediate control occupying tile
                                                            // or 0x0F for unoccupied
    uint8_t region; // grouping of disjoint water/land areas; see above notes for more details
    uint8_t visibility; // faction bitfield of those who can see tile (mapped: dim/bright)
    uint8_t val3; // 00 000 000 | rocky (2 bit); lock faction_id (3 bit); using faction_id (3 bit)
    uint8_t unk_1; // flags? bitfield
    int8_t territory; // physical owner factionID of geographic area or -1 for unclaimed
    uint32_t bit; // see terrain_bit
    uint32_t bit2; // bitwho[0]? FF FF FFFF | code (landmark tile sequence); unk flags; landmark id
    uint32_t bit_visible[7]; // how each faction sees tile (ex. pods another faction already got)
};

// THE STRIDE THE IMAGE INDEXES `MapTiles` BY, at 0x0040CEBC and six other
// sites, all of the same shape:
//
//     lea edx, [eax + eax*4]        ; i*5
//     lea eax, [eax + edx*2]        ; i + i*10          = i*11
//     mov edx, dword ptr [0x94a30c] ; *MapTiles
//     lea eax, [edx + eax*4]        ; base + i*44       = 0x2C
//     test byte ptr [eax + 8], 1    ; ->bit & 1
//
// Three member offsets fall out of the same sites and agree with the fields
// above, which is what makes this a reading of the layout rather than only of
// its size: `[+0]` read as a byte and masked `0xE0` is `climate`'s altitude
// bits (0x0041ACD2), `[+0]` read whole is `climate` (0x0040B2FB), and `[+8]`
// tested against 1 is the low byte of `bit` (0x0040CECB).
static_assert(sizeof(Map) == 0x2C,
              "Map layout must match the original executable");

struct Landmark {
    int x;
    int y;
    char name[32];
};

struct Continent {
    uint32_t tile_count; // count of tiles in region
    uint32_t open_terrain; // count of non-rocky, non-fungus tiles (only 1 movement point to travel)
    uint32_t unk_3; // highest world_site value (0-15)
    uint32_t pods; // current count of supply and unity pods in region
    uint32_t unk_5; // padding?
    uint8_t sea_coasts[8]; // sea specific regions, connections to land regions? bitmask
};

struct RulesNatural {
    LPSTR name;
    LPSTR name_short;
};

// THE ARRAY EXACTLY FILLS THE SPACE BETWEEN ITS BASE AND THE NEXT NAMED
// GLOBAL, which is what makes this a measurement and not a restatement.
// `Natural` sits at 0x0094ADE0 and `Weapon` at 0x0094AE60, a gap of
// 0x80. 16 elements of 0x8 is 0x80 - exactly that,
// with nothing left over. A wrong element size would either overflow the
// neighbour or leave a hole, and the element count is the tree's own
// (MaxNaturalNum).
//
// This is the SINGLETON-AND-ARRAY case that tools/derive_array_strides.py
// cannot reach: it reads a stride off an index computation, and code that
// walks these tables with a pointer never computes one.
static_assert(sizeof(RulesNatural) == 0x8,
              "RulesNatural layout must match the original executable");


static const int MaxNaturalNum = 16;
static const int MaxLandmarkNum = 64;
static const int RegionBounds = 63;
static const int MaxContinentNum = 128;
static const int MaxRegionLandNum = 64;

const int RadiusRange[] = { 1, 9, 25, 49, 81, 121, 169, 225, 289 };
const int RadiusBaseX[] = {  1, 2, 1, 0, -1, -2, -1,  0, 0 };
const int RadiusBaseY[] = { -1, 0, 1, 2,  1,  0, -1, -2, 0 };
const int RadiusOffsetX[] = {
     0,  1,  2,  1,  0, -1, -2,  -1,   0,   2,   2,  -2,  -2,   1,   3,   3,   1,  -1,  -3,  -3,
    -1,  4, -4,  0,  0,  1,  2,   3,   4,   5,   5,   4,   3,   2,   1,  -1,  -2,  -3,  -4,  -5,
    -5, -4, -3, -2, -1,  0,  6,   0,  -6,   0,   1,   2,   3,   4,   5,   6,   7,   8,   7,   6,
     5,  4,  3,  2,  1,  0, -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -7,  -6,  -5,  -4,  -3,  -2,
    -1,  0,  1,  2,  3,  4,  5,   6,   7,   8,   9,  10,   9,   8,   7,   6,   5,   4,   3,   2,
     1,  0, -1, -2, -3, -4, -5,  -6,  -7,  -8,  -9, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,
    -1,  0,  1,  2,  3,  4,  5,   6,   7,   8,   9,  10,  11,  12,  11,  10,   9,   8,   7,   6,
     5,  4,  3,  2,  1,  0, -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11, -12, -11, -10,
    -9, -8, -7, -6, -5, -4, -3,  -2,  -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
    11, 12, 13, 14, 13, 12, 11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  -1,  -2,
    -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -13, -12, -11, -10,  -9,  -8,  -7,  -6,
    -5, -4, -3, -2, -1,  0,  1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
    15, 16, 15, 14, 13, 12, 11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  -1,  -2,
    -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -15, -14, -13, -12, -11, -10,
    -9, -8, -7, -6, -5, -4, -3,  -2,  -1, };
const int RadiusOffsetY[] = {
     0,  -1,   0,   1,   2,   1,   0,  -1,  -2,  -2,   2,   2,  -2,  -3, -1,  1,  3,  3,  1, -1,
    -3,   0,   0,   4,  -4,  -5,  -4,  -3,  -2,  -1,   1,   2,   3,   4,  5,  5,  4,  3,  2,  1,
    -1,  -2,  -3,  -4,  -5,   6,   0,  -6,   0,  -8,  -7,  -6,  -5,  -4, -3, -2, -1,  0,  1,  2,
     3,   4,   5,   6,   7,   8,   7,   6,   5,   4,   3,   2,   1,   0, -1, -2, -3, -4, -5, -6,
    -7, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,   0,   1,   2,  3,  4,  5,  6,  7,  8,
     9,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  -1,  -2, -3, -4, -5, -6, -7, -8,
    -9, -12, -11, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,   0,  1,  2,  3,  4,  5,  6,
     7,   8,   9,  10,  11,  12,  11,  10,   9,   8,   7,   6,   5,   4,  3,  2,  1,  0, -1, -2,
    -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4,
    -3,  -2,  -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 11, 12, 13, 14, 13, 12,
    11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  -1,  -2, -3, -4, -5, -6, -7, -8,
    -9, -10, -11, -12, -13, -16, -15, -14, -13, -12, -11, -10,  -9,  -8, -7, -6, -5, -4, -3, -2,
    -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12, 13, 14, 15, 16, 15, 14,
    13,  12,  11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0, -1, -2, -3, -4, -5, -6,
    -7,  -8,  -9, -10, -11, -12, -13, -14, -15, };
const uint32_t ElevDetail[] = { 0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200 };

/*
 * The four map tiles that meet at each corner of a tile's rendered diamond,
 * as offsets from that tile. The first index is the corner: 0 is west, 1
 * north, 2 east, 3 south. Within a row the order is constant relative to the
 * corner point, so element [c][1] is always the tile above it and [c][3]
 * always the tile below - which is what alt_get_ocean_detail() reads when it
 * shades a corner.
 *
 * Read from 00685EA0 and 00685EE0. Each address is referenced seven times in
 * .text: three times by alt_get_ocean_detail() at 00462190, twice by its
 * caller MapWin::gen_terrain_poly (00463FBB and 00464584) and twice more
 * between 004700DA and 004708E2, so the name has to serve the renderer rather
 * than just the one function.
 */
const int CornerOffsetX[4][4] = {
    { -2, -1,  0, -1 },
    { -1,  0,  1,  0 },
    {  0,  1,  2,  1 },
    { -1,  0,  1,  0 },
};
const int CornerOffsetY[4][4] = {
    {  0, -1,  0,  1 },
    { -1, -2, -1,  0 },
    {  0, -1,  0,  1 },
    {  1,  0,  1,  2 },
};

extern int MapLongitudeBounds; // x
extern int MapLatitudeBounds; // y
extern uint32_t MapRandSeed;
extern int MapSeaLevel;
extern int MapSeaLevelCouncil; // Solar Shade (-3); Melt Polar Caps (+3)
extern uint32_t MapArea;
extern uint32_t MapAreaSqRoot;
extern BOOL MapIsFlat;
extern int MapLandmarkCount;
extern Landmark *MapLandmark;
extern uint32_t MapAbstractLongBounds; // abstract x
extern uint32_t MapAbstractLatBounds; // abstract y
extern uint32_t MapAbstractArea;
extern uint32_t MapSizePlanet;
extern uint32_t MapOceanCoverage;
extern uint32_t MapLandCoverage; // 2 - MapOceanCoverage
extern uint32_t MapErosiveForces;
extern uint32_t MapPlanetaryOrbit; // affects temp
extern uint32_t MapCloudCover; // affects rainfall, rivers
extern uint32_t MapNativeLifeForms;
LPSTR *const MapFilePath = (LPSTR *)0x0094A2BC;
// AN LVALUE AT A FIXED ADDRESS, not a pointer to one. The tile array's base
// lives AT 0x0094A30C and the image reads it with a single absolute
// `mov ecx, [0x94a30c]`. Spelled `Map **const MapTiles = (Map **)0x0094A30C`
// it took TWO loads - VC6 keeps storage for the constant, reads that, then
// dereferences it - and every tile access in the tree paid for the extra one.
inline Map *&map_tiles() { return *reinterpret_cast<Map **>(0x0094A30C); }
uint8_t **const MapAbstract = (uint8_t **)0x0094A310;
extern int MapBaseSubmergedCount[8];
extern int MapBaseIdClosestSubmergedVeh[8];
extern uint32_t BrushVal1; // TODO: more descriptive variable name
extern uint32_t BrushVal2; // TODO: more descriptive variable name
extern uint32_t WorldBuildVal1; // TODO: more descriptive variable name

extern Continent *Continents;
RulesNatural *const Natural = (RulesNatural *)0x0094ADE0;
extern uint32_t MapLongitude; // halve of MapLongitudeBounds
extern uint32_t *AltNatural; // Default: { 0, 15, 32, 45, 60,  75,  80, 100, 100, 100, 100 };

BOOL __cdecl on_map(int x, int y);
int __cdecl xrange(int x);
// `int` throughout, matching both the definition in src/map.cpp and the
// catalogue's ?whose_territory@@YAHHHHPAHH@Z. The header said `uint32_t`,
// so callers emitted ...@@YAHIIIPAHH@Z and found no definition.
int __cdecl whose_territory(int faction_id, int x, int y, int *base_id,
                                      BOOL ignore_comm);
int __cdecl base_territory(int faction_id, int x, int y);
int __cdecl crappy(int x, int y);
int __cdecl vector_dist(int x_distance, int y_distance);
int __cdecl vector_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b);
BOOL __cdecl sea_coast(int region_dst, int region_src);
int __cdecl sea_coasts(int region_src);
BOOL __cdecl base_on_sea(int base_id, int region_sea);
int __cdecl base_coast(int base_id);
BOOL __cdecl port_to_coast(int base_id, int region);
BOOL __cdecl port_to_port(int base_id_src, int base_id_dst);
BOOL __cdecl transport_base(int base_id);
BOOL __cdecl naval_base(int base_id);
BOOL __cdecl convoy(int veh_id, int base_id);
BOOL __cdecl bad_reg(int region);
BOOL __cdecl get_there(int veh_id, int x_dst, int y_dst);
BOOL __cdecl coast_or_border(uint32_t x_point_a, uint32_t y_point_a, uint32_t x_point_b, 
                                       uint32_t y_point_b, uint32_t faction_id);
// INLINE, AND IN THE HEADER. The image has no `map_loc` - every caller
// computes the tile address itself, because this is a helper the tree
// invented to name what the original open-coded. Defined in a .cpp, VC6
// cannot inline it and every one of its callers pays a `call` the image
// does not make; there are more than a hundred of them under map.cpp alone.
// `int`, not `uint32_t`: the image shifts x with `sar ecx, 1` at 0x005001BF,
// which is the arithmetic shift a SIGNED right-shift compiles to. Unsigned
// gives `shr` and is one byte different in every caller.
inline Map *__cdecl map_loc(int x, int y) {
  return &map_tiles()[(x >> 1) + y * MapLongitude];
}
uint32_t __cdecl temp_at(uint32_t x, uint32_t y);
void __cdecl temp_set(int x, int y, int temperature);
uint32_t __cdecl climate_at(uint32_t x, uint32_t y);
void __cdecl climate_set(int x, int y, int climate);
int __cdecl elev_at(int x, int y);
int __cdecl alt_natural(int x, int y);
void __cdecl alt_set_both(int x, int y, int altitude_natural);
int __cdecl alt_at(int x, int y);
int __cdecl altitude_at(uint32_t x, uint32_t y);
int __cdecl alt_detail_at(int x, int y);
int __cdecl alt_get_ocean_detail(int x, int y, int corner, int point);
void __cdecl alt_put_detail(int x, int y, int detail);
uint32_t __cdecl owner_at(uint32_t x, uint32_t y);
void __cdecl owner_set(int x, int y, int faction_id);
void __cdecl site_set(int x, int y, int site);
int __cdecl region_at(int x, int y);
void __cdecl region_set(int x, int y, int region);
uint32_t __cdecl using_at(uint32_t x, uint32_t y);
void __cdecl using_set(int x, int y, int faction_id);
uint32_t __cdecl lock_at(uint32_t x, uint32_t y);
void __cdecl lock_set(int x, int y, int faction_id);
BOOL __cdecl lock_map(int x, int y, int faction_id);
void __cdecl unlock_map(int x, int y, int faction_id);
uint32_t __cdecl rocky_at(uint32_t x, uint32_t y);
void __cdecl rocky_set(int x, int y, int rocky);
int __cdecl bit_at(int x, int y);
void __cdecl bit_put(int x, int y, int bit);
void __cdecl bit_set(int x, int y, int bit, BOOL set);
uint32_t __cdecl bit2_at(uint32_t x, uint32_t y);
void __cdecl bit2_set(int x, int y, int bit2, BOOL set);
uint32_t __cdecl code_at(uint32_t x, uint32_t y);
void __cdecl code_set(int x, int y, int code);
void __cdecl synch_bit(int x, int y, int faction_id);
int __cdecl minerals_at(int x, int y);
int __cdecl bonus_at(int x, int y, int UNUSED(unk_val) unk_val);
int __cdecl goody_at(int x, int y);
void __cdecl site_radius(int x, int y, int UNUSED(unk_val) unk_val);
int __cdecl find_landmark(int x, int y, int radius_range_offset);
int __cdecl new_landmark(int x, int y, LPCSTR name);
BOOL __cdecl valid_landmark(int x, int y, int faction_id);
void __cdecl kill_landmark(int x, int y);
BOOL __cdecl is_coast(int x, int y, BOOL is_base_radius);
BOOL __cdecl is_ocean(int x, int y);
int __cdecl veh_who(int x, int y);
void __cdecl rebuild_vehicle_bits();
void __cdecl rebuild_base_bits();
int __cdecl x_dist(int x_point_a, int x_point_b);
BOOL __cdecl is_known(int x, int y, int faction_id);
int __cdecl base_who(int x, int y);
int __cdecl anything_at(int x, int y);
void __cdecl map_shutdown();
BOOL __cdecl map_init();
void __cdecl map_wipe();
BOOL __cdecl map_write(FILE *map_file);
BOOL __cdecl map_read(FILE *map_file);
uint8_t __cdecl abstract_at(int x, int y);
void __cdecl abstract_set(int x, int y, uint8_t region);
void __cdecl quick_zoc(uint32_t x_src, uint32_t y_src, uint32_t faction_id, int x_dst,
                                 int y_dst, int *x_zoc, int *y_zoc);
int __cdecl radius_move(int x_radius_off, int y_radius_off, int range);
 int __cdecl radius_move(int x_src, int y_src, int x_dst, int y_dst, int range);
int __cdecl compass_move(int x_src, int y_src, int x_dst, int y_dst);
int __cdecl is_sensor(int x, int y);
int __cdecl good_sensor(int faction_id, int x, int y);
BOOL __cdecl has_temple(int faction_id);
void __cdecl world_alt_set(int x, int y, int altitude, BOOL is_set_both);
void __cdecl world_raise_alt(int x, int y);
void __cdecl world_lower_alt(int x, int y);
void __cdecl brush(int x, int y, int altitude);
void __cdecl paint_land(int x, int y, int altitude, int radius);
void __cdecl build_continent(int size);
void __cdecl build_hills(int altitude);
void __cdecl world_riverbeds();
BOOL __cdecl world_validate();
void __cdecl world_temperature();
int __cdecl world_site(int x, int y, BOOL is_ocean_site);
void __cdecl world_analysis();
void __cdecl world_alt_put_detail(int x, int y);
void __cdecl world_polar_caps();
// ?world_climate@@YAXXZ at 0x005C5A30 - still an original body, behind a
// forwarder in src/pending_bodies.cpp. See draw_map in mapwin.h.
void __cdecl world_climate();
// ?custom_planet@@YAHHH@Z at 0x0058C2A0 - still an original body, behind a
// forwarder in src/pending_bodies.cpp. It has a second span at
// 0x006605A5-0x006607D7, which is why it is a long way from being promoted.
int __cdecl custom_planet(int a, int b);
void __cdecl world_linearize_contours();
BOOL __cdecl near_landmark(int x, int y);
void __cdecl world_crater(int x, int y);
void __cdecl world_monsoon(int x, int y);
void __cdecl world_sargasso(int x, int y);
void __cdecl world_ruin(int x, int y);
void __cdecl world_dune(int x, int y);
void __cdecl world_diamond(int x, int y);
void __cdecl world_fresh(int x, int y);
void __cdecl world_volcano(int x, int y, BOOL is_not_landmark);
void __cdecl world_borehole(int x, int y);
void __cdecl world_temple(int x, int y);
void __cdecl world_unity(int x, int y);
void __cdecl world_fossil(int x, int y);
void __cdecl world_canyon(int x, int y);
void __cdecl world_mesa(int x, int y);
void __cdecl world_ridge(int x, int y);
void __cdecl world_geothermal(int x, int y);
void __cdecl world_landmarks();
int __cdecl zoc_any(int x, int y, int faction_id);
int __cdecl zoc_veh(int x, int y, int faction_id);
int __cdecl zoc_sea(int x, int y, int faction_id);
int __cdecl zoc_move(int x, int y, int faction_id);
int __cdecl cursor_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b);
int __cdecl mandate_color(int mandate);
int __cdecl mandate_color_redirect(int mandate);
uint32_t *const MandateColors = (uint32_t *)0x008C6DE4;
