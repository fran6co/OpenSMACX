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
#include "stdafx.h"
#include "popup.h"
#include "general.h"
#include "win.h"
#include "map.h"
#include "map_coordinates.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "interface_palette.h"
#include "path.h"
#include "random.h"
#include "strings.h"
#include "technology.h"
#include "terraforming.h"
#include "veh.h"
#include "log.h"
#include "lock.h"     // map_tile_bytes() - site_at/set_dirty walk its records
#include "netdaemon.h"
#include "net_checksum_report.h"
#include "console.h"
#include "netmsg.h"   // NetMsgPrimary - synch_display posts through it
#include "mapwin.h"
#include "worldwin.h"
#include "xpops.h"
#include "palette.h"
#include "crt_random_inline.h"

// Existing shared punctuation: declared in string_formatting.cpp and defined in socialwin.cpp.
extern char SocialWinCommaSpaceText[];

// Inferred map-grid/storage contribution core. The address-bounded group
// includes map allocation and serialization, tile state, altitude/landmark
// operations, and their shared source-owned data. Its historical TU boundary
// remains unproved.

// Shared landmark lookups belong to these source arrays, not the old process.
// Direction data has 49 independently identified rows of five bytes; keep
// the following three-byte image gap out of the object. Initializers remain
// pinned ignored configure-time derivatives, not tracked binary dumps.
// RECOVERY_OBJECT: v2 0x00685684 0x00685779 ?AltDirectionTable@@3QBEB 0x005927D0
extern const unsigned char AltDirectionTable[49 * 5] = {
#include "terrain_alt_direction.inc"
};
static_assert(sizeof(AltDirectionTable) == 245, "landmark direction span");

// Both the writable rows and immutable templates have 113-byte accessed
// source windows. Original upper allocation bounds are unproved; their v1
// bindings claim starts only. Writable initial windows are loader-zero.
// RECOVERY_OBJECT: v1 0x007D3B74 ?AltBonusArid@@3PAEA
unsigned char AltBonusArid[113];
// RECOVERY_OBJECT: v1 0x0068577C ?AltTemplateArid@@3QBEB
extern const unsigned char AltTemplateArid[113] = {
#include "terrain_landmark_arid.inc"
};
// RECOVERY_OBJECT: v1 0x007F65D0 ?AltBonusMoist@@3PAEA
unsigned char AltBonusMoist[113];
// RECOVERY_OBJECT: v1 0x006857F0 ?AltTemplateMoist@@3QBEB
extern const unsigned char AltTemplateMoist[113] = {
#include "terrain_landmark_moist.inc"
};
// RECOVERY_OBJECT: v1 0x007F655C ?AltBonusRainy@@3PAEA
unsigned char AltBonusRainy[113];
// RECOVERY_OBJECT: v1 0x0068593C ?AltTemplateRainy@@3QBEB
extern const unsigned char AltTemplateRainy[113] = {
#include "terrain_landmark_rainy.inc"
};
// RECOVERY_OBJECT: v1 0x007D3A78 ?AltBonusFlat@@3PAEA
unsigned char AltBonusFlat[113];
// RECOVERY_OBJECT: v1 0x00685864 ?AltTemplateFlat@@3QBEB
extern const unsigned char AltTemplateFlat[113] = {
#include "terrain_landmark_flat.inc"
};
// RECOVERY_OBJECT: v1 0x007F64E8 ?AltBonusOcean@@3PAEA
unsigned char AltBonusOcean[113];
// RECOVERY_OBJECT: v1 0x006859B0 ?AltTemplateOcean@@3QBEB
extern const unsigned char AltTemplateOcean[113] = {
#include "terrain_landmark_ocean.inc"
};

// world_build copies eleven words from AltNaturalDefault to AltNatural.
// world_rivers selects AltContour by the native altitude band. The three
// adjacent original tables are separate real arrays, not pointers into an
// old executable. Preserve the initial values in pinned ignored includes;
// recovered callers read and mutate these source-owned objects only.
// RECOVERY_OBJECT: v2 0x0068FAF4 0x0068FB20 ?AltNaturalDefault@@3PAIA 0x005C86E0
uint32_t AltNaturalDefault[11] = {
#include "terrain_altitude_default.inc"
};
// RECOVERY_OBJECT: v2 0x0068FB20 0x0068FB4C ?AltContour@@3PAIA 0x005C38B0
uint32_t AltContour[11] = {
#include "terrain_altitude_contour.inc"
};
// RECOVERY_OBJECT: v2 0x0068FB4C 0x0068FB78 ?AltNatural@@3PAIA 0x005C86E0
uint32_t AltNatural[11] = {
#include "terrain_altitude_natural.inc"
};
static_assert(sizeof(AltNaturalDefault) == 44, "default altitude table extent");
static_assert(sizeof(AltContour) == 44, "river contour table extent");
static_assert(sizeof(AltNatural) == 44, "mutable altitude table extent");

// goody_at reads these signed words when distributing supply pods. The names
// describe their observed roles, not recovered original identifiers. The pin
// supplies their initial values and direct loads; no direct writer is known.
// RECOVERY_OBJECT: v2 0x0068FB78 0x0068FB7C ?MapPodDifferenceMultiplier@@3HA 0x00592140
int MapPodDifferenceMultiplier = 61;
// RECOVERY_OBJECT: v2 0x0068FB7C 0x0068FB80 ?MapPodAverageMultiplier@@3HA 0x00592140
int MapPodAverageMultiplier = 11;
// Loader-zero pointer cells at 94A30C/94A310. map_init allocates both
// arrays; map_shutdown releases them with the matching CRT free.
// Resampling retains its previous allocation and dimensions in these
// original scratch cells (945AD0/945AD4/945AD8).
// RECOVERY_OBJECT: v1 0x00945AD0 ?MapResamplePreviousTiles@@3PAUMap@@A
Map *MapResamplePreviousTiles;
// RECOVERY_OBJECT: v1 0x00945AD4 ?MapResamplePreviousWidth@@3HA
int MapResamplePreviousWidth;
// RECOVERY_OBJECT: v1 0x00945AD8 ?MapResamplePreviousHeight@@3HA
int MapResamplePreviousHeight;

// Metadata scalars formerly lived here; landmarks_ and file_path_ formerly
// lived in alpha.cpp. All had static zero initialization. Their exact native
// address/offset relationship is now declared in map.h in one real allocation.
// MapLongitude (68FAF0) remains an independently initialized row-stride object.
// map_read/map_write transfer this writable zero-fill window as 0xAA4 bytes.
// This records the observed serialized region, not a claim about original type,
// complete allocation, or lifetime.
// RECOVERY_OBJECT: v2 0x00949870 0x0094A314 ?MapStorage@@3UMapStorageWindow@@A 0x005910B0
// RECOVERY_OBJECT: v3 0x0094A30C ?MapTiles@@3PAUMap@@A member ?MapStorage@@3UMapStorageWindow@@A tiles_
// RECOVERY_OBJECT: v3 0x0094A310 ?MapAbstractTiles@@3PAEA member ?MapStorage@@3UMapStorageWindow@@A abstract_tiles_
// RECOVERY_OBJECT: v3 0x00949870 ?MapLongitudeBounds@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.longitude_bounds_
// RECOVERY_OBJECT: v3 0x00949874 ?MapLatitudeBounds@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.latitude_bounds_
// RECOVERY_OBJECT: v3 0x00949884 ?MapArea@@3IA member ?MapStorage@@3UMapStorageWindow@@A metadata_.area_
// RECOVERY_OBJECT: v3 0x0094988C ?MapIsFlat@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.is_flat_
MapStorageWindow MapStorage;
// RECOVERY_OBJECT: v4 0x0068FAF0 ?MapLongitude@@3IA target ?MapLongitude@@3HA
int MapLongitude = 1;  // half-width tile-row stride; original default is 1
LPCSTR MapExtension = "MP";

/*
Purpose: Shutdown allocated map variables.
// ORIGINAL: 0x00590E90 ?map_shutdown@@YAXXZ 0x00590E90-0x00590EC5 BYTE_EXACT
// body      src/map_grid.cpp
// size      53 bytes
// prototype
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// LEVER: map_read (0x00591130), one of its three callers, call_diff'd FEWER by
//        exactly one call - the image inlines this whole body there. Moved to
//        map.h as `MEASURED inline`; map_init's call site (already call_diff-clean)
//        was unaffected.
// PINNED CALL-SITE CORRECTION: map_init (0x00590ED0) also expands the frees
// directly at 0x0059100B and 0x0059101D; neither inline consumer calls 0x590E90.
Return Value: n/a
Status: Complete
*/
void __cdecl map_shutdown() {
    if (map_tiles()) {
        free(map_tiles());
    }
    if (MapAbstract()) {
        free(MapAbstract());
    }
    map_tiles() = 0;
    MapAbstract() = 0;
}
/*
Purpose: Initialize map variables.
// ORIGINAL: 0x00590ED0 ?map_init@@YAXXZ 0x00590ED0-0x00591036
// LEVER: `sprintf_s(MapFilePath, 80, "maps\\%s.%s", label_get(676), MapExtension)`
//        was ONE call the image never makes - it builds the path by hand into
//        StringTemp ("no C++ library" idiom): StringTemp[0]=0; strcat "maps\\";
//        strcat label_get(676); strcat "."; then strcpy StringTemp into
//        MapFilePath and strcat MapExtension onto that. Took this from 5/105
//        (0.745 similar) to 25/105 (0.919 similar). TRIED beyond this
//        point: the image pins ebx=0/edi=1 in registers across the whole
//        function (push edi; mov edi,1 at entry; cmp eax,ebx instead of
//        test eax,eax at every null-check; mov eax,edi at the return-true
//        tail) - a VC6 register-allocation heuristic, not a source shape;
//        no rewrite of the two null-checks (`!= 0`, ternary, guard clause)
//        changed which register VC6 picked.
// symbol    ?map_init@@YAHXZ
// size      358 bytes
// prototype
// callers   6   call targets   7
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00471240 0x005D4510 0x006169A0 0x006290E0 0x00644EF2 0x00645460 0x00645470
Return Value: n/a
Status: Complete
*/
BOOL __cdecl map_init() {
    StringTemp[0] = 0;
    strcat(StringTemp, "maps\\");
    strcat(StringTemp, label_get(676));
    strcat(StringTemp, ".");
    strcpy((LPSTR)MapFilePath, StringTemp);
    strcat((LPSTR)MapFilePath, MapExtension);
    MapLongitude = MapLongitudeBounds / 2;
    MapArea = MapLongitude * MapLatitudeBounds;
    MapAreaSqRoot = quick_root(MapArea);
    map_tiles() = 0;
    map_tiles() = (Map *)mem_get(MapArea * sizeof(Map));
    if (map_tiles()) {
        MapAbstractLongBounds = (MapLongitudeBounds + 4) / 5;
        MapAbstractLatBounds = (MapLatitudeBounds + 4) / 5;
        MapAbstractArea = MapAbstractLatBounds * ((MapAbstractLongBounds + 1) / 2);
        MapAbstract() = (uint8_t *)mem_get(MapAbstractArea);
        if (MapAbstract()) {
            mapwin_terrain_fixup();
            return false;
        }
    }
    release_map_storage();
    return true;
}
/*
Purpose: Reset the map to a blank state. Doesn't wipe unk_1 and territory fields.
// ORIGINAL: 0x00591040 ?map_wipe@@YAXXZ 0x00591040-0x005910B0 BYTE_EXACT
// LEVER: `Map *tile = map_tiles();` walked with `tile++` each iteration,
//        instead of indexing `map_tiles()[i]` (which re-reads the global
//        every field access AND fails to fold the `ZeroMemory` call to a
//        `rep stosd`, forcing a real call) - matches the image's running
//        pointer (`edx`, `add edx, 0x2c`). Field order also fixed to the
//        image's `bit` BEFORE `val3`, and `bit2` folded into one
//        `ZeroMemory` spanning `bit2`+`bit_visible` (the image zeroes both
//        in the same `rep stosd`, `bit2` is never stored separately). `int i`
//        instead of `uint32_t i`: the image's loop-bound compare is `jl`
//        signed. Together: 9/38 (0.682) -> 30/38 (0.921), 38/38 total
//        instructions, same count as the image.
// TRIED: declaring `tile` before `MapRandSeed = random(...)+1` (to chase
//            the image scheduling the `map_tiles()` load between the
//            `random()` call and consuming its result) - worse, 20/38 (0.895).
// size      112 bytes
// prototype
// callers   3   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00625810
Return Value: n/a
Status: Complete
*/
void __cdecl map_wipe() {
    MapSeaLevelCouncil = 0;
    MapLandmarkCount = 0;
    MapSeaLevel = 0;
    MapRandSeed = random(0, 0x7FFF) + 1;
    Map *tile = map_tiles();
    for (int i = 0; i < MapArea; ++i, ++tile) {
        tile->climate = ALT_BIT_OCEAN;
        tile->contour = 20;
        tile->val2 = 0xF;
        tile->region = 0;
        tile->visibility = 0;
        tile->bit = 0;
        tile->val3 = 0;
        ZeroMemory(&tile->bit2, sizeof(tile->bit2) + sizeof(tile->bit_visible));
    }
}
/*
Purpose: Write map data to a file.
// ORIGINAL: 0x005910B0 ?map_write@@YAHPAUFILE@@@Z 0x005910B0-0x00591125 BYTE_EXACT
// symbol    ?map_write@@YAHPAU_iobuf@@@Z
// size      117 bytes
// prototype int (__cdecl ?map_write@@YAHPAUFILE@@@Z)(FILE* file)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064603F
Return Value: Did an error occur? true/false
Status: Complete
*/
// TRIED: call_diff names the image's target as `__fwrite`
// (src/recovered/0064603f.cpp - deleted 2026-09-03 with the CRT-range
// demotions - at 0x0064603F, a CRT-internal
// `__lock_file`/`_fwrite`/`__unlock_file` wrapper, which was BYTE_EXACT
// there but NOT a build input) rather than the public `fwrite()`. Neither
// `__fwrite` nor its own callees (`__lock_file`, `__unlock_file`, the raw
// `_fwrite`) are exported by the linked CRT import libraries - `extern "C"`
// forward declarations of any of them fail LNK2001, and defining local
// bodies for all three (to promote the artifact properly) is a bigger
// change than this pass's scope. Left calling the public `fwrite()`.
// LEVER: the image does NOT `&&`-chain the three calls into one condition -
//        the first two are separate `if (!fwrite(...)) return true;` early
//        returns, and the LAST is a direct `return !fwrite(...);` (branchless
//        `neg;sbb;inc` boolean, no third branch at all). Matching that
//        shape is BYTE_EXACT.
BOOL __cdecl map_write(FILE *map_file) {
    if (!fwrite(&MapStorage, 0xAA4, 1, map_file)) {
        return true;
    }
    if (!fwrite(map_tiles(), MapArea * sizeof(Map), 1, map_file)) {
        return true;
    }
    return !fwrite(MapAbstract(), MapAbstractArea, 1, map_file);
}
/*
Purpose: Read the map data from a file and write it into memory.
// ORIGINAL: 0x00591130 ?map_read@@YAHPAUFILE@@@Z 0x00591130-0x00591208 BYTE_EXACT
// LEVER: same shape as map_write (0x005910B0) - the image does NOT `||`-chain the last two `fread`s into one condition; each is its own `if (!fread(...)) return true;`. BYTE_EXACT on that alone.
// symbol    ?map_read@@YAHPAU_iobuf@@@Z
// size      216 bytes
// prototype int (__cdecl ?map_read@@YAHPAUFILE@@@Z)(FILE* file)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00590ED0 0x00592940 0x00644EF2 0x00646178
Return Value: Did an error occur? true/false
Status: Complete
*/
BOOL __cdecl map_read(FILE *map_file) {
    release_map_storage();
    if (!fread(&MapStorage, 0xAA4, 1, map_file)) {
        return true;
    }
    map_tiles() = 0;
    MapAbstract() = 0;
    if (map_init()) {
        return true;
    }
    if (!fread(map_tiles(), MapArea * sizeof(Map), 1, map_file)) {
        return true;
    }
    if (!fread(MapAbstract(), MapAbstractArea, 1, map_file)) {
        return true;
    }
    fixup_landmarks();
    return false;
}
/*
Purpose: Get the region value for the specified tile.
// ORIGINAL: 0x00591210 ?abstract_at@@YAHHH@Z 0x00591210-0x00591230 BYTE_EXACT
// body      src/map.h
// symbol    ?abstract_at@@YAEHH@Z
// size      32 bytes
// prototype int (__cdecl ?abstract_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: MNEMONIC_ONLY 10 of 12, and it is the same one-instruction
//            plateau abstract_set (0x00591230) records. The image sums the
//            ROW into the base register and leaves the column in its own
//            (`add edx, eax; mov al, byte ptr [ecx + edx]`); VC6 -O2 always
//            pre-sums the two index terms instead (`add eax, ecx; mov al,
//            byte ptr [eax + edx]`), whatever the source says.
// TRIED: six spellings, every one MNEMONIC_ONLY with the divergence at
//            the same instruction 8 - the committed `(x >> 1) + y * mult`,
//            the reversed `y * mult + (x >> 1)`, an explicit `uint8_t *row`
//            local, `row` plus a separate `col` local, a `col` local alone,
//            `row += ...` as a statement, and the pointer-arithmetic
//            `*(MapAbstract() + ... + ...)` form.
Return Value: Abstract value (region)
Status: Complete
*/
// ABI evidence: the getter defines AL only. Both mapped enemy_strategy
// calls (0x56313B and 0x5631C3) mask EAX to one byte before using it.
// This supports the current byte-valued interface; the catalogue int
// spelling is historical and does not prove the original source prototype.


/*
Purpose: Set the region value for the specified tile.
// ORIGINAL: 0x00591230 ?abstract_set@@YAXHHE@Z 0x00591230-0x00591253 BYTE_EXACT
// body      src/map_grid.cpp
// TRIED: same plateau as abstract_at/alt_at. Image computes a row
//            pointer (base + y*(bounds>>1)) explicitly, then indexes it by
//            x>>1 in the final SIB store; VC6 -O2 always pre-sums the two
//            index terms into one register first regardless of source shape.
//            Tried: (x>>1)+y*mult, y*mult+(x>>1), an explicit `row` pointer
//            local, and a separate `col=x>>1` local before the row - all
//            identical 10/13 MNEMONIC_ONLY, same first divergence.
// size      35 bytes
// prototype void (__cdecl ?abstract_set@@YAXHHE@Z)(int xCoord, int yCoord, unsigned int8 val)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl abstract_set(int x, int y, uint8_t region) {
    (MapAbstract())[(x >> 1) + y * (MapAbstractLongBounds >> 1)] = region;
}
/*
Purpose: Set the altitude details for the specified tile.
// ORIGINAL: 0x00591260 ?alt_put_detail@@YAXHHH@Z 0x00591260-0x00591288 BYTE_EXACT
// body      src/map_grid.cpp
// TRIED: shared plateau with region_set (identical shape). Image loads
//            the byte parameter into cl BEFORE loading map_tiles(); the
//            compiler always loads map_tiles() first regardless of source
//            order. Tried: a cast-only local, a `Map *tile = map_loc(x,y);`
//            temp before the store, both no-op - same 11/14 MNEMONIC_ONLY,
//            same swapped pair of loads.
// size      40 bytes
// prototype void (__cdecl ?alt_put_detail@@YAXHHH@Z)(int xCoord, int yCoord, int detail)
// callers   5   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl alt_put_detail(int x, int y, int detail) {
    map_loc(x, y)->contour = detail;
}
// ORIGINAL: 0x00591290 ?alt_set@@YAXHHH@Z 0x00591290-0x00591894
// RECOVERY_BINDING: v1 {"evidence": ["git:1fe327399e8313c7d34d3b190f391eeeafdb0c28:src/unrecovered/00591290.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "Recover signed altitude changes through real Map/Base/Veh/faction owners, preserving native submersion, facility, compaction and callback paths. Explicit unsigned shift retains x86 low-byte altitude encoding without C++ signed-shift undefined behavior. Historical mappings and seed lessons are retained; no new byte/layout/runtime/linked-preservation claim."}
// TRIED: no Ghidra hint for this one - straight register-variable/goto transcription from the raw disassembly (44-byte tile struct, 0x833-stride owner tables, round-to-zero /2 idiom, extern lever on the lea-taken 0x00946d54 table). Compiles; diverges at instruction #0 (push vs mov), so the prologue/frame shape itself differs, not yet investigated further.
void __cdecl alt_set(int x, int y, int altitude) {
    static char submerged_title[] = "BASESUBMERGED";
    static char submerged_lost_title[] = "BASESUBMERGED2";

    Map *const tile = map_loc(x, y);

    // The original turn-zero branch skips gameplay consequences only.
    // Its common altitude write and dirty flag remain unconditional.
    if (GameDataStateStorage.fields_.turn_current_num_ != 0) {
        const int old_altitude = (tile->climate & 0xE0) >> 5;
        const int was_water = old_altitude < ALT_SHORE_LINE;
        const int becomes_water = altitude < ALT_SHORE_LINE;

        if (altitude != old_altitude) {
            delete_landmark(x, y);

            if (was_water != becomes_water) {
                tile->bit2 &= 0x7E1377A3u;
                for (int faction = 0; faction < 7; ++faction) {
                    tile->bit_visible[faction] = tile->bit;
                }

                if (GameDataStateStorage.fields_.turn_current_num_ != 0 &&
                    WorldClimateSkipTerritoryReset == 0 && becomes_water) {
                    const int base_id = base_at(x, y);
                    if (base_id >= 0 &&
                        !has_fac_built(FAC_PRESSURE_DOME, base_id)) {
                        Base &base = Bases[base_id];
                        const int faction_id = base.faction_id_current;

                        const int pressure_cost = (rand() % 3) + 2;
                        const int half_population =
                            static_cast<int>(base.population_size) / 2;
                        int population_loss;
                        if (half_population < pressure_cost ||
                            pressure_cost > 10) {
                            population_loss = pressure_cost;
                        } else if (half_population <= 10) {
                            population_loss = half_population;
                        } else {
                            population_loss = 10;
                        }
                        base.population_size = static_cast<int8_t>(
                            base.population_size - population_loss);

                        set_fac(FAC_PRESSURE_DOME, base_id, 1);
                        ++MapBaseSubmergedCount[faction_id];
                        // The native writer sets this neutral PlayerData flag;
                        // no semantic flag name is established in the owner.
                        PlayersData[faction_id].flags |= 0x00080000u;

                        if (is_objective(base_id) && base.population_size <= 0) {
                            base.population_size = 1;
                        }

                        if (base.population_size <= 0) {
                            if (PlayersData[faction_id].current_num_bases <= 1) {
                                base.population_size = 1;
                                if (VehBattleUnkTgl == 0) {
                                    const int local_faction =
                                        ConsoleGlobal.faction_id_;
                                    if (faction_id == local_faction) {
                                        ConsoleGlobal.focus(base.x, base.y,
                                                            local_faction);
                                        parse_says(0, base.name_string, -1, -1);
                                        X_pop(submerged_title,
                                                            "subbase_sm.pcx", 0);
                                    } else if (base.visibility &
                                               (1u << (local_faction & 0x1F))) {
                                        ConsoleGlobal.focus(base.x, base.y,
                                                            local_faction);
                                        parse_says(0, base.name_string, -1, -1);
                                        parse_says(1,
                                            Players[faction_id].adj_name_faction,
                                            -1, -1);
                                        X_pop(submerged_lost_title,
                                                            "subbase_sm.pcx", 0);
                                    }
                                }
                            } else {
                                if ((!IsMultiplayerNet ||
                                     ConsoleControlTurnActive != 0) &&
                                    VehBattleUnkTgl == 0) {
                                    const int local_faction =
                                        ConsoleGlobal.faction_id_;
                                    if (faction_id == local_faction) {
                                        ConsoleGlobal.focus(base.x, base.y,
                                                            local_faction);
                                        parse_says(0, base.name_string, -1, -1);
                                        popp(ScriptPopupFile, "BASESUBMERGED1", 0,
                                             "subbase_sm.pcx", 0);
                                    } else if (base.visibility &
                                               (1u << (local_faction & 0x1F))) {
                                        ConsoleGlobal.focus(base.x, base.y,
                                                            local_faction);
                                        parse_says(0, base.name_string, -1, -1);
                                        parse_says(1,
                                            Players[faction_id].adj_name_faction,
                                            -1, -1);
                                        popp(ScriptPopupFile, "BASESUBMERGED3", 0,
                                             "subbase_sm.pcx", 0);
                                    }
                                }

                                base_kill(base_id);

                                int vehicle_id = stack_fix(veh_at(x, y));
                                while (vehicle_id >= 0) {
                                    Veh &vehicle = Vehs[vehicle_id];
                                    int next_vehicle = vehicle.next_veh_id_stack;
                                    const int triad = Chassis[
                                        VehPrototypes[vehicle.proto_id].chassis_id
                                    ].triad;
                                    if (triad != TRIAD_AIR &&
                                        (vehicle.flags & 0x20u) == 0 &&
                                        ((triad == TRIAD_SEA) != becomes_water)) {
                                        const int vehicle_faction =
                                            vehicle.faction_id;
                                        MapBaseIdClosestSubmergedVeh[vehicle_faction] =
                                            base_find(x, y, vehicle_faction);
                                        veh_kill(vehicle_id);
                                        if (next_vehicle > vehicle_id) {
                                            --next_vehicle;
                                        }
                                    }
                                    vehicle_id = next_vehicle;
                                }
                            }
                        }
                    }
                }
            }

            // Land emerging from the sea can gain one rocky level; a land tile
            // that is lowered can lose one. The probability divisors and the
            // asymmetric altitude bounds follow the two native branches.
            if (!becomes_water) {
                const int rockiness = tile->val3 >> 6;
                if (altitude > old_altitude) {
                    if (rockiness < ROCKINESS_ROCKY &&
                        rockiness + 1 <= altitude - ALT_SHORE_LINE) {
                        const int divisor = rockiness + 4;
                        if (divisor <= 1 || rand() % divisor == 0) {
                            tile->val3 = static_cast<uint8_t>(
                                ((rockiness + 1) << 6) | (tile->val3 & 0x3F));
                            UnkBitfield1 |= 1;
                        }
                    }
                } else if (rockiness > ROCKINESS_FLAT &&
                           rockiness > altitude - ALT_SHORE_LINE) {
                    const int divisor = rockiness + 1;
                    if (divisor > 1 && rand() % divisor != 0) {
                        tile->val3 = static_cast<uint8_t>(
                            ((rockiness + 3) << 6) | (tile->val3 & 0x3F));
                        UnkBitfield1 |= 1;
                    }
                }
            }
        }
    }
    tile->climate = static_cast<uint8_t>((tile->climate & 0x1F) |
                                         (static_cast<uint32_t>(altitude) << 5));
    UnkBitfield1 |= 4;
}
// Semantic owner for shared altitude operations: current world-generation
// callers are in world.cpp, while these canonical bodies share this TU.

/*
Purpose: Calculate the natural altitude of the specified tile.
// ORIGINAL: 0x005918A0 ?alt_natural@@YAHHH@Z 0x005918A0-0x005918EF BYTE_EXACT
// body      src/map_grid.cpp
// TRIED: same index-fold plateau as alt_at/alt_detail_at - the inlined
//            map_loc(x,y)->contour read (via alt_detail_at) schedules the
//            width-load/imul/shift with ecx/eax swapped from the image's
//            allocation; best 4/28 (0.618) across all flag sets, default
//            flags give 3/28 (0.793 similar, MISMATCH). Not chased further.
// size      79 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Natural altitude on a scale from 0 (ocean trench) to 6 (mountain tops)
Status: Complete
*/
int __cdecl alt_natural(int x, int y) {
    int natural = 0;
    // Native SUB wraps the metadata dword, then JGE orders the result and
    // threshold as signed32. These bit conversions use the VC6/x86 contract.
    const int32_t contour = static_cast<int32_t>(
        static_cast<uint32_t>(alt_detail_at(x, y)) - static_cast<uint32_t>(MapSeaLevel));
    for (int threshold = ALT_3_LEVELS_ABOVE_SEA; threshold >= 0; threshold--) {
        if (contour >= static_cast<int32_t>(AltNatural[threshold])) {
            natural = threshold;
            break;
        }
    }
    return natural;
}
/*
Purpose: Set both the altitude and natural altitude for the specified tile. The altitude_natural
         parameter can be between 0 to 9.
// ORIGINAL: 0x005918F0 ?alt_set_both@@YAXHHH@Z 0x005918F0-0x005919C0
// LEVER: historical call_diff work found the image calls `alt_natural`
//   from world_linearize_contours but expands its scan here. This is a
//   historical compiler observation; the current caller is in world.cpp,
//   while this setter remains with the canonical helpers in this TU. That
//   source boundary allows the local helper
//   expansion. The `rnd()` call was replaced with its own body, since a
//   cross-TU call can never inline. 0.513 -> 0.843
//   similar. TRIED beyond that: `Map*`/pointer hoists for
//   AltNatural[altitude_natural] (no change); rewriting alt_natural's
//   `while (cond && natural) natural--;` as a decrement-then-clamp loop to
//   chase the image's pointer-walk shape (made both worse, 0.843->0.817).
// size      208 bytes
// prototype void (__cdecl ?alt_set_both@@YAXHHH@Z)(int xCoord, int yCoord, int altitude)
// callers   2   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591290 0x0064601D
Return Value: n/a
Status: Complete
*/
void __cdecl alt_set_both(int x, int y, int altitude_natural) {
    alt_set(x, y, altitude_natural);
    if (alt_natural(x, y) != altitude_natural) {
        int roll = bounded_crt_random(AltNatural[altitude_natural + 1] - AltNatural[altitude_natural]);
        alt_put_detail(x, y, AltNatural[altitude_natural] + MapSeaLevel + roll);
    }
}
/*
Purpose: Calculate the elevation of the specified tile.
// ORIGINAL: 0x005919C0 ?elev_at@@YAHHH@Z 0x005919C0-0x00591A77
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5919D6 loads68FB2C, AltContour[3];591A13 compares against68FB20+4*(climate>>5) with signed591A1A JLE.", "Current src/map_grid.cpp owns AltContour at68FB20..68FB4C; the former ElevDetail header array was duplicate storage, not an independent original object."], "reason": "Read the genuine shared contour target table rather than a header-local copy of its initial values."}
// LEVER: two fixes got this from 37/67 to 57/67 (0.985 similar). (1) A
//        shared `Map *tile` read once for both `contour` and `climate`,
//        instead of calling `alt_detail_at`/`alt_at` as separate accessors
//        that each recompute map_loc(x,y). (2) BRANCH POLARITY: the ternary
//        `contour <= threshold ? 10 : modulo` put the short `+10` arm in
//        the fall-through and jumped to the long modulo block, backwards
//        from the image (which falls through to the modulo work and jumps
//        to +10). Rewriting as `if (contour > threshold) { modulo } else
//        { +10 }` matches. `contour` must be `int`: as `uint32_t` the
//        comparison compiled `ja`/unsigned, the image's `jle` is signed.
// TRIED: the remaining divergence is a single MapSeaLevel global load
//            (`mov edi, [0x94987c]`) that the image schedules AFTER the
//            contour byte-read and this tree schedules one slot earlier,
//            regardless of source statement order - tried an intermediate
//            `diff` local isolating the two subtractions, no effect. Same
//            instruction count (67/67) either way, just one load's position.
// TRIED (2026-09-03): `contour - MapSeaLevel - ElevDetail[3]` operand order,
//            a `int sea = MapSeaLevel` local used for both subterms, and
//            computing `elev` inside each branch - all exactly 57/67, the
//            load's slot does not follow from any of them.
// size      183 bytes
// prototype int (__cdecl ?elev_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Elevation (bounded to: -3000 to 3500)
Status: Complete
*/
int __cdecl elev_at(int x, int y) {
    Map *const tile = map_loc(x, y);
    int contour = tile->contour;
    int elev = 50 * (contour - MapSeaLevel - AltContour[3]);
    if (contour > (int)AltContour[tile->climate >> 5]) {
        elev += (x * 113 + y * 217 + MapSeaLevel * 301) % 50;
    } else {
        elev += 10;
    }
    elev = range(elev, -3000, 3500);
    return elev;
}
/*
Purpose: Set the rainfall (climate) for the specified tile.
// ORIGINAL: 0x00591A80 ?climate_set@@YAXHHH@Z 0x00591A80-0x00591ACE BYTE_EXACT
// size      78 bytes
// prototype void (__cdecl ?climate_set@@YAXHHH@Z)(int xCoord, int yCoord, int climate)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl climate_set(int x, int y, int rainfall) {
    Map *tile = map_loc(x, y);
    tile->climate &= 0xE7;
    tile->climate |= (rainfall & 3) << 3;
    tile->bit2 |= 0x400000; // TODO: identify value
    UnkBitfield1 |= 1; // TODO: identify global + value
}
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the temperature for the specified tile.
// ORIGINAL: 0x00591AD0 ?temp_set@@YAXHHH@Z 0x00591AD0-0x00591B05 BYTE_EXACT
// size      53 bytes
// prototype
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl temp_set(int x, int y, int temperature) {
    Map *tile = map_loc(x, y);
    tile->climate &= 0xF8;
    tile->climate |= temperature & 7;
}
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the faction owner for the specified tile.
// ORIGINAL: 0x00591B10 ?owner_set@@YAXHHH@Z 0x00591B10-0x00591B48
// TRIED: direct `tile->val2` read/write (no pointer hoist) - worse (0.857 vs 0.895).
//            Rewriting the RHS as the explicit XOR-merge identity
//            (`*field ^ ((*field ^ faction_id) & 0xF)`, which is what the image's
//            `xor/and/xor` sequence computes for `(*field & 0xF0) | (faction_id & 0xF)`)
//            scored identically to the plain OR-of-ANDs (0.895/8 of 20) - the compiler
//            already finds that transform on its own. The image factors the record-index
//            multiply as `lea x11` + implicit SIB scale-4 (`[ecx+eax*4+2]`); this tree's
//            imul-by-0x2c/lea-plus-ecx form is the same "multiply factoring" plateau as
//            whose_territory, not something the RHS shape changes.
// size      56 bytes
// prototype void (__cdecl ?owner_set@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   13   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl owner_set(int x, int y, int faction_id) {
    // Through a pointer to the field; see `code_set` for why.
    uint8_t *const field = &map_loc(x, y)->val2;
    *field = static_cast<uint8_t>((*field & 0xF0) | (faction_id & 0xF));
}
/*
Purpose: Set the site for the specified tile.
// ORIGINAL: 0x00591B50 ?site_set@@YAXHHH@Z 0x00591B50-0x00591B86 BYTE_EXACT
// TRIED: SHAPE_EXACT 17/19 plateau. Image folds the +2 field offset
//            into the READ's addressing mode but recomputes a bare tile
//            pointer (no offset) for the WRITE, applying +2 as the store's
//            own displacement instead - the field-pointer form folds +2
//            into a single lea reused for both, one byte longer at that
//            lea and everything after shifts. Tried: a `Map *tile` local
//            with two independent `->val2` accesses (worse, 15/19 and a
//            full reorder of the shl/and), an intermediate `uint8_t v`
//            local, re-calling `map_loc(x,y)` a second time for the write,
//            and swapping the OR operand order - all equal or worse.
// size      54 bytes
// prototype void (__cdecl ?site_set@@YAXHHH@Z)(int xCoord, int yCoord, int site)
// callers   9   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl site_set(int x, int y, int site) {
    Map *const tile = map_loc(x, y);
    const uint8_t old_value = tile->val2;
    const uint8_t site_byte = static_cast<uint8_t>(site);
    tile->val2 = static_cast<uint8_t>((old_value & 0x0F) | (site_byte << 4));
}
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the region for the specified tile.
// ORIGINAL: 0x00591B90 ?region_set@@YAXHHH@Z 0x00591B90-0x00591BB8 BYTE_EXACT
// body      src/map_grid.cpp
// TRIED: same plateau as alt_put_detail - byte-parameter load and
//            map_tiles() pointer load are swapped by the compiler
//            regardless of source order. Tried a `Map *tile` temp before
//            the store; no-op, same 11/14 MNEMONIC_ONLY.
// size      40 bytes
// prototype void (__cdecl ?region_set@@YAXHHH@Z)(int xCoord, int yCoord, int region)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl region_set(int x, int y, int region) {
    map_loc(x, y)->region = region;
}
/*
Purpose: Set the rockiness for the specified tile.
// ORIGINAL: 0x00591BC0 ?rocky_set@@YAXHHH@Z 0x00591BC0-0x00591C0E
// LEVER: `uint8_t *const field = &tile->val3;` plus a local for the masked read, same idiom
//            as owner_set/site_set - the image reads `val3` and masks it BEFORE it evaluates
//            `rocky << 6`, where `tile->val3 = (tile->val3 & 0x3F) | (rocky << 6)` in one
//            statement (either operand order) evaluated the shift first regardless. 0.920 ->
//            0.960 best-across-flags similarity.
// TRIED: delaying the `val3` store until after computing `tile->bit2 | 0x400000` into a
//            local (matching the image's read-both-then-store-both order for the two fields)
//            made it WORSE (0.960 -> 0.885) - the local for `bit2` changes the image's
//            `mov ecx,[eax+0xc]/or ecx,.../mov [eax+0xc],ecx` register-folded shape into
//            something that costs more elsewhere. Left storing `val3` immediately. Also tried a
//            `uint32_t bit2 = tile->bit2;` local read BEFORE the `val3` store, OR'd and stored
//            AFTER (matching the image's load-then-store-val3-then-or-store-bit2 order exactly) -
//            forced a fourth callee-saved register (`push esi`), which shifts every subsequent
//            offset and drops it to 8/25. Confirms the plateau is a spill-slot ceiling, not order.
// TRIED (2026-09-03): `tile->bit2 |= 0x400000` written BEFORE the `val3`
//            store (16/25); `uint32_t bit2 = tile->bit2 | 0x400000` local,
//            stored after (15/25); `tile->bit2 = tile->bit2 | 0x400000` then
//            the `val3` store (15/25). The image's 2-instruction interleave
//            (bit2 load+or, val3 store, bit2 store) is VC6 scheduling a load
//            above an independent store, not a source order this tree can
//            name. Left at 22/25.
// size      78 bytes
// prototype void (__cdecl ?rocky_set@@YAXHHH@Z)(int xCoord, int yCoord, int rocky)
// callers   11   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl rocky_set(int x, int y, int rocky) {
    Map *tile = map_loc(x, y);
    tile->val3 = static_cast<uint8_t>((tile->val3 & 0x3F) | (rocky << 6));
    tile->bit2 |= 0x400000; // TODO: identify value
    UnkBitfield1 |= 1; // TODO: identify variable + value
}
/*
Purpose: Set the using faction id for the specified tile.
// ORIGINAL: 0x00591C10 ?using_set@@YAXHHH@Z 0x00591C10-0x00591C48 BYTE_EXACT
// TRIED: 12/20 plateau. Image reads tile->val3 TWICE (once via the
//   pre-offset [ecx+eax*4+5] load, once again via [eax+5] after computing
//   the plain tile pointer); this tree's `&=0xF8; |=faction&7;` compiles the
//   same XOR-merge trick but CSEs the two reads into one, one instruction
//   short. Tried: single `(v&0xF8)|(f&7)` assignment, explicit
//   `v ^= (v^f)&7`, splitting map_loc() per statement, a `uint8_t v = ...`
//   temp, casts to uint8_t/uint32_t on either side - all equal or worse.
// size      56 bytes
// prototype void (__cdecl ?using_set@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl using_set(int x, int y, int faction_id) {
    Map *tile = map_loc(x, y);
    tile->val3 &= 0xF8;
    tile->val3 |= faction_id & 7;
}
/*
Purpose: Set the lock faction id for the specified tile.
// ORIGINAL: 0x00591C50 ?lock_set@@YAHHHH@Z 0x00591C50-0x00591C88 BYTE_EXACT
// body      src/map.h
// symbol    ?lock_set@@YAXHHH@Z
// size      56 bytes
// prototype int (__cdecl ?lock_set@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: `callers 0` - lock_map (0x00591C90, its only source-level caller) call_diff'd
//        MORE by exactly one call on this. Moved to map.h as `MEASURED inline`.
Return Value: n/a
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Lock the specified tile for the faction id.
// ORIGINAL: 0x00591C90 ?lock_map@@YAHHHH@Z 0x00591C90-0x00591CE2 BYTE_EXACT
// size      82 bytes
// prototype int (__cdecl ?lock_map@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: True if already locked by another faction, otherwise false
Status: Complete
*/
BOOL __cdecl lock_map(int x, int y, int faction_id) {
    uint32_t lock_id = lock_at(x, y);
    if (lock_id != faction_id) {
        if (lock_id) {
            return true;
        }
        lock_set(x, y, faction_id);
    }
    return false;
}
/*
Purpose: Unlock the specified tile for faction id.
// ORIGINAL: 0x00591CF0 ?unlock_map@@YAXHHH@Z 0x00591CF0-0x00591D29 BYTE_EXACT
// LEVER: the `Map *tile = map_loc(x, y);` local must be declared INSIDE the
//        if-block, after the lock_at() check, not hoisted above it - the
//        image computes the tile pointer only on the taken branch.
// size      57 bytes
// prototype void (__cdecl ?unlock_map@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl unlock_map(int x, int y, int faction_id) {
    if (lock_at(x, y) == faction_id) {
        Map *tile = map_loc(x, y);
        tile->val3 &= 0xC7;
    }
}

/*
Purpose: Set the bit for the specified tile.
// ORIGINAL: 0x00591D30 ?bit_put@@YAXHHH@Z 0x00591D30-0x00591D58 BYTE_EXACT
// size      40 bytes
// prototype void (__cdecl ?bit_put@@YAXHHH@Z)(int xCoord, int yCoord, int bit)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// TRIED: best MNEMONIC_ONLY (needs /Oy- to even get the ebp-frame; with
//            it, 12/14 instructions, all mnemonics agree). The image loads
//            the `bit` parameter into ecx BEFORE the map_tiles() pointer into
//            edx, this tree's `map_loc(x, y)->bit = bit;` always loads the
//            pointer first and `bit` second. Tried: a local `int new_bit =
//            bit;` before the store, and a local `Map *tile = map_loc(x, y);`
//            before it - both compiled byte-identical to the plain form, so
//            the scheduling is fixed by the compiler regardless of source
//            shape. Not chased further.
void __cdecl bit_put(int x, int y, int bit) {
    map_loc(x, y)->bit = bit;
}
/*
Purpose: Set or unset bit for the specified tile.
// ORIGINAL: 0x00591D60 ?bit_set@@YAXHHHH@Z 0x00591D60-0x00591DA2 BYTE_EXACT
// LEVER: the image computes the tile's `&bit` address ONCE, unconditionally, before testing `set` - this tree's `if (set) map_loc(x,y)->bit |= bit; else map_loc(x,y)->bit &= ~bit;` let the compiler test `set` FIRST and compute the address separately in each arm instead. Hoisting `uint32_t *const field = &map_loc(x, y)->bit;` above the `if` (see map.h) is BYTE_EXACT.
// size      66 bytes
// prototype void (__cdecl ?bit_set@@YAXHHHH@Z)(int xCoord, int yCoord, int bit, int)
// callers   32   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl bit_set(int x, int y, int bit, BOOL set) {
    uint32_t *const field = &map_loc(x, y)->bit;
    if (set) {
        *field |= bit;
    } else {
        *field &= ~bit;
    }
}
/*
Purpose: Set or unset bit2 for the specified tile.
// ORIGINAL: 0x00591DB0 ?bit2_set@@YAXHHHH@Z 0x00591DB0-0x00591DF2 BYTE_EXACT
// Historical LEVER/measurement note: this was `MEASURED inline` in map.h; world_borehole (0x005C7020) and siblings call_diff'd FEWER because the image keeps a real call. It was un-inlined beside bit_set/code_set (now in map_grid.cpp); `osmx check` stayed at 0 REGRESSED and world_borehole's call count matched.
// size      66 bytes
// prototype void (__cdecl ?bit2_set@@YAXHHHH@Z)(int xCoord, int yCoord, int bit2, int)
// callers   19   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl bit2_set(int x, int y, int bit2, BOOL set) {
    uint32_t *const field = &map_loc(x, y)->bit2; // same LEVER as bit_set: fold the offset once.
    if (set) {
        *field |= bit2;
    } else {
        *field &= ~bit2;
    }
}
/*
Purpose: Set the code for the specified tile which keeps track of tile sequence order for landmarks.
// ORIGINAL: 0x00591E00 ?code_set@@YAXHHH@Z 0x00591E00-0x00591E45 BYTE_EXACT
// size      69 bytes
// prototype void (__cdecl ?code_set@@YAXHHH@Z)(int xCoord, int yCoord, int)
// callers   16   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl code_set(int x, int y, int code) {
    // THROUGH A POINTER TO THE FIELD. The image folds the member offset into
    // the address computation - `lea eax, [ecx + eax*4 + 0xc]` at 0x00591E29 -
    // and reading and writing `tile->bit2` twice puts the 0xc on the `mov`
    // instead. Taking `&tile->bit2` once is what produces the image's lea.
    uint32_t *const field = &map_loc(x, y)->bit2;
    *field = (*field & 0xFFFFFF) | (static_cast<uint32_t>(code) << 24);
    UnkBitfield1 |= 4; // TODO: identify variable + value
}
/*
Purpose: Synchronize the actual tile bit with the faction visible bit.
// ORIGINAL: 0x00591E50 ?synch_bit@@YAXHHH@Z 0x00591E50-0x00591E82 BYTE_EXACT
// LEVER: `Map *tile = map_loc(x, y);` shared between the read of `tile->bit`
//        and the write to `tile->bit_visible[...]`, instead of two separate
//        `map_loc()`/`bit_at()` calls, so the address is computed once.
// size      50 bytes
// prototype void (__cdecl ?synch_bit@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   24   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Defined after this unit's callers; their public declaration is in map.h.
void __cdecl synch_bit(int x, int y, int faction_id) {
    if (faction_id) {
        Map *tile = map_loc(x, y);
        tile->bit_visible[faction_id - 1] = tile->bit;
    }
}
/*
Purpose: Name the faction working a tile that this faction may enter by treaty.
// ORIGINAL: 0x00591E90 sub_591e90 0x00591E90-0x00591EF9 FILE BYTE_EXACT
// symbol    ?treaty_using_faction@@YAHHHH@Z
// NO IMAGE CALLER AND NO IMAGE REFERENCE AT ALL (no E8 to it, no 4-byte
//        image of its address anywhere in the file), so the name is derived
//        from the body, not from a catalogue entry: it returns the tile's
//        `using` faction when the two are not the same faction, the tile is
//        land, and the treaty word says TREATY without PACT - the "right of
//        passage through an ally's territory" test. Everything it reads was
//        already named: `val3 & 7` is using_at(), the altitude nibble of
//        `climate` is the ALT_BIT_* ladder, and the per-faction word is
//        PlayersData[].diplo_treaties[] - 0x96C9E0 + faction * 0x20CC + the
//        subject * 4, which is the same arithmetic spying_recovery.h spells
//        `SpyingStatusTable[faction * 2099 + subject]`.
// LEVER: (1) ONE exit. The image's five failures all jump to a shared
//        `or eax,-1`, so the body is a nest whose arms fall through to a
//        single `return -1` - a `||` chain of early returns gives five
//        separate epilogues. (2) `& ~0x1F`, not `& 0xE0` - the complement
//        stops VC6 narrowing the whole test to `and dl,0xe0 / cmp dl,0x60 /
//        jb`, which is where `xor edx,edx / mov dl,[eax] / and edx,
//        0xffffffE0 / cmp edx,0x60 / jl` comes from (game.cpp lever 4,
//        measured again here). 3/39 -> BYTE_EXACT 39/39.
// size      105 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: The using faction id, or -1
Status: Complete
*/
int __cdecl treaty_using_faction(int x, int y, int faction_id) {
    if (faction_id > 0) {
        Map *tile = map_loc(x, y);
        if ((tile->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
            int using_id = tile->val3 & 7;
            if (using_id > 0 && using_id != faction_id) {
                uint32_t treaty = PlayersData[faction_id].diplo_treaties[using_id];
                if (!(treaty & DTREATY_PACT) && (treaty & DTREATY_TREATY)) {
                    return using_id;
                }
            }
        }
    }
    return -1;
}
/*
Purpose: Determine the tile's mineral count that translates to rockiness.
// ORIGINAL: 0x00591F00 ?minerals_at@@YAHHH@Z 0x00591F00-0x00592007
// LEVER: three fixes, 2/113 -> 13/113 (0.529 similar). (1) SIGNED / 2 IS NOT
//        >> 1: the image's `avg` computation carries the `cdq;sub;sar`
//        round-toward-zero fixup, so `avg` (and the `x/2`, `avg/2` subterms
//        of val1) must be written `/2`, not `>>1`. (2) `alt_at(x, y)` is
//        read by the image LAST, right before the switch, off a register
//        that survives the whole body untouched - moved the call to just
//        before `type` and read through a saved `origX` so the mutated `x`
//        (`x -= avg`) does not corrupt it. (3) The jump-table dump at
//        0x592008/0x592018 shows cases 1 and 2 of the outer `switch(type)`
//        are themselves compare-decrement chains over `val2`, i.e. nested
//        `switch(val2)` in the source, not the ternary chains that were
//        here - only `case 3`'s ternary reproduces its target's
//        test/cmp shape untouched.
// TRIED: not chased past this MISMATCH plateau - 263-byte body, three
//            jump tables, register-allocation-sensitive scheduling of the
//            early `push esi`/`push edi` pair. --all-flags agrees (best is
//            the same /O2 /Ob0 /Gy /GR- /Oy- /GX set already in use).
// size      263 bytes
// prototype int (__cdecl ?minerals_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: 0 (Flat), 1 (Rolling), 2 (Rocky)
Status: Complete
*/
// The guarded dispatches at591FA3 and591FE9 each select four absolute
// code targets. Both companion tables are generated from the nested switches;
// their data is outside the preserved original code extent.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00591F00 0x00592008 0x00592018 0x00591FA3
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00591F00 0x00592018 0x00592028 0x00591FE9
int __cdecl minerals_at(int x, int y) {
    if (!y || y == (MapLatitudeBounds - 1)) {
        return 2; // poles
    }
    int origX = x;
    int avg = (x + y) / 2;
    x -= avg;
    int val1 = (x / 2) + MapRandSeed + 2 * (x / 2) + 2 * (avg / 2);
    int val2 = (val1 - 2 * (x & 1) - (avg & 1)) & 3;
    int alt = alt_at(origX, y);
    int type = alt - ALT_SHORE_LINE;
    if (type < 0) {
        type = ALT_SHORE_LINE - alt;
    }
    if (alt < ALT_SHORE_LINE) {
        type--;
    }
    switch (type) {
      case 0: {
        switch (val2) {
          case 0:
            return 1;
          case 1:
          case 3:
            return 0;
          case 2:
            return ((val1 & 4) != 0) + 1; // 1 or 2
          default:
            return ~val2 & 1;
        }
      }
      case 1:
        switch (val2) {
          case 0: return 0;
          case 1: return 1;
          case 2: return 1;
          default: return 2;
        }
      case 2:
        switch (val2) {
          case 0: return 0;
          case 1: return 1;
          default: return 2;
        }
      case 3:
        return (val2 < 0 || val2 > 1) ? 2 : 1;
      default:
        return ~val2 & 1;
    }
}
/*
Purpose: Determine if the tile has a resource bonus. While the last parameter is unused, it's set to
         1 by two calls inside world_site(). Otherwise, all other calls have it set to 0.
// ORIGINAL: 0x00592030 ?bonus_at@@YAHHHH@Z 0x00592030-0x00592135
// TRIED (body is in map.h): the image reads the climate byte (alt_at) before the bit
//            dword (bit_at) at the shared `site_tile`-style address, while map.h's body
//            declares `bit` before `alt`. Swapping the declaration order to match the
//            image's read order made it WORSE (25/117 -> 17/117), so the declaration
//            order already in the header is the closer one; the remaining gap is the
//            image folding the bit-field load into the SIB-addressed `mov` where this
//            tree materialises the tile address into a register first - the same
//            register-allocation plateau as the rest of this family.
// LEVER: `alt` and `chk` typed `int` instead of `uint32_t` - the image compares
//            them against ALT_OCEAN_SHELF/ALT_SHORE_LINE with `jl`/`jge` and divides
//            `chk` with `idiv`, both signed forms; the tree's `uint32_t` locals were
//            producing `jb`/`jae`/`div`. 25/117 (0.800 similar) -> 20/117 (0.856
//            similar) across --all-flags.
// TRIED: `avg` typed `int` to match - the image's one remaining `avg >> 2` uses
//            `sar`, which `int` does reproduce, but it costs agreement elsewhere and
//            the best-flags similarity drops to 0.825. Left `uint32_t`.
// size      261 bytes
// prototype int (__cdecl ?bonus_at@@YAHHHH@Z)(int xCoord, int yCoord, int unkVal)
// callers   17   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: 0 (no bonus), 1 (nutrient), 2 (mineral), 3 (energy)
Status: Complete
*/
// This source owner provides the resource-bonus query. Its callers retain
// the original external-call boundary rather than header-exported copies.
// TRIED: a non-inline `bonus_at_call` forwarder, routed through from
//            crop_yield/mine_yield/energy_yield (base.cpp) so bonus_at(x,y,0)
//            would emit `call 0x592030` there - measured NO improvement to
//            the best-across-flags similarity for any of the three (each
//            tied exactly with not using it: crop_yield 0.292, mine_yield
//            0.388, energy_yield 0.137), because their winning flag set is
//            /Ob0-based and bonus_at's body already does not get folded in
//            there regardless of the forwarder. Not added.

int __cdecl bonus_at(int x, int y, int UNUSED(unk_val)) {
    int alt = alt_at(x, y);
    uint32_t bit = bit_at(x, y);
    BOOL has_rsc_bonus = bit & BIT_RSC_BONUS;
    if (!has_rsc_bonus) {
        if (!MapRandSeed) {
            return 0;
        }
        if (alt >= ALT_SHORE_LINE && !(GameDataStateStorage.fields_.game_rules_ & RULES_NO_UNITY_SCATTERING)) {
            return 0;
        }
    }
    int avg = (x + y) >> 1;
    x -= avg;
    int chk = (avg & 3) + 4 * (x & 3);
    if (!has_rsc_bonus && chk != ((-5 * (avg >> 2) - 3 * (x >> 2) + MapRandSeed) & 0xF)) {
        return 0;
    }
    if (alt < ALT_OCEAN_SHELF) {
        return 0;
    }
    int ret = (alt < ALT_SHORE_LINE) ? chk % 3 + 1 : (chk % 5) & 3;
    if (!ret || bit & BIT_NUTRIENT_RSC) {
        if (bit & BIT_ENERGY_RSC) {
            ret = 3; // energy
        } else {
            ret = (bit & BIT_MINERAL_RSC) ? 2 : 1; // nutrient or mineral
        }
    }
    return ret;
}
/*
Purpose: Determine if the tile has a supply pod and if so what type.
// ORIGINAL: 0x00592140 ?goody_at@@YAHHH@Z 0x00592140-0x00592248
// TRIED: --all-flags's winner (39/114, the default set) already beats every other set
//            by a wide margin; the divergence is in the prologue - the image delays
//            loading x (`mov esi,[ebp+8]`) until after the `y * MapLongitude` multiply and
//            the callee-saved pushes, this tree loads x into edx immediately before the
//            multiply. Same register/instruction-scheduling plateau as bonus_at and the
//            rest of this family.
// size      264 bytes
// prototype int (__cdecl ?goody_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   14   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: 0 (no supply pod), 1 (standard supply pod), 2 (unity pod?)
Status: Complete
*/
int __cdecl goody_at(int x, int y) {
    uint32_t bit = bit_at(x, y);
    if (bit & (BIT_SUPPLY_REMOVE | BIT_MONOLITH)) {
        return 0; // nothing, supply pod already opened or monolith
    }
    if (GameDataStateStorage.fields_.game_rules_ & RULES_NO_UNITY_SCATTERING) {
        return (bit & (BIT_UNK_4000000 | BIT_UNK_8000000)) ? 2 : 0; // ?
    }
    if (bit & BIT_SUPPLY_POD) {
        return 1; // supply pod
    }
    if (!MapRandSeed) {
        return 0; // nothing
    }
    int avg = (x + y) >> 1;
    int x_diff = x - avg;
    if (!is_ocean(x, y)
        && (avg & 3) + 4 * (x_diff & 3)
            == ((-5 * (avg >> 2) - 3 * (x_diff >> 2) + MapRandSeed) & 0xF)) {
        return 2;
    }
    return (avg & 3) + 4 * (x_diff & 3) == ((MapPodDifferenceMultiplier * (x_diff / 4)
        + MapPodAverageMultiplier * (avg / 4) + MapRandSeed + 8) & 0x1F); // 0 or 1
}
// Pinned say_loc also references shared punctuation at0x00682E9C/0x00682E98.
// Their canonical original bindings are beside say_vehicles in datalink_entries.cpp;
// qualifying this TU's pooled copies remains a reference-restoration obligation.
// ORIGINAL: 0x00592250 ?say_loc@@YAXPADHHHHH@Z 0x00592250-0x005923FD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/veh.h:say_loc;src/map.h:map_loc", "Historical initial canonical-anchor owner: src/string_formatting.cpp:independently corroborated parenthesis RECOVERY_LITERAL bindings", "src/buffer.cpp:BufferJoinSpace;src/socialwin.cpp:SocialWinCommaSpaceText", "image:0x00592250-0x005923FD uses 0x00682820,0x00682E90,0x00682E9C,0x00682E98", "source:src/datalink_entries.cpp retains the unique canonical parentheses bindings; say_loc original operands at0x00682E9C/0x00682E98 still require qualification for this TUs pooled copies."], "reason": "Keep say_loc with the adjacent map-grid operations: its mapped body at 0x592250 lies between goody_at and site_radius and directly reads map location state. Its punctuation and shared literal bindings remain separately evidenced; this is current semantic source ownership, not proof of the original TU."}
// size      429 bytes
// prototype void (__cdecl ?say_loc@@YAXPADHHHHH@Z)(int8*, int xCoord, int yCoord, int, int, int)
// callers   4   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3D50 0x006169A0 0x00645470 0x0064FC88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592250
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592250/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_loc@@YAXPADHHHHH@Z  at 0x00592250  (429 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void __cdecl say_loc(char *text, int x, int y, int want_name, int trailer_mode,
                     int want_coords) {
    char number[80];
    int base_id = base_at(x, y);
    if (want_name != 0) {
        if (base_id < 0) {
            want_coords = 0;
            base_id = base_find(x, y, -1, -1, -1, ConsoleGlobal.faction_id_);
            if (base_id < 0) {
                goto trailer;
            }
            strcat(text, label_get(0xf8 / 4));
            strcat(text, BufferJoinSpace);
        } else {
            goto skip_first_check;
        }
    }
    if (base_id < 0) {
        goto trailer;
    }
skip_first_check:
    if (want_coords != 0) {
        strcat(text, label_get(0x20 / 4));
        strcat(text, BufferJoinSpace);
    }
    strcat(text, Bases[base_id].name_string);
    if (trailer_mode != 0) {
        strcat(text, BufferJoinSpace);
    }

trailer:
    if (trailer_mode == 1 || (trailer_mode == 2 && base_id < 0)) {
        strcat(text, "(");
        _itoa(x, number, 10);
        strcat(text, number);
        strcat(text, SocialWinCommaSpaceText);
        _itoa(y, number, 10);
        strcat(text, number);
        strcat(text, ")");

        int flags = GameDataStateStorage.fields_.game_state_;
        if ((flags & 0x40) != 0 && (flags & 0x1000) != 0) {
            strcat(StringTemp, BufferJoinSpace);
            Map *tile = map_loc(x, y);
            _itoa(tile->region, number, 10);
            strcat(StringTemp, number);
        }
    }
}
/*
Purpose: Clear the map's site values in a radius from the tile.
// ORIGINAL: 0x00592400 ?site_radius@@YAXHHH@Z 0x00592400-0x00592480 BYTE_EXACT
// LEVER: fixed by fixing `xrange()` itself (see 0x0048BEE0, src/planwin.cpp) - `MapIsFlat & 1` and the swapped if/else polarity. No change needed here.

// size      128 bytes
// prototype void (__cdecl ?site_radius@@YAXHHH@Z)(int xCoord, int yCoord, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl site_radius(int x, int y, int UNUSED(unk_val)) {
    for (int i = 0; i < 21; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            Map *tile = map_loc(x_radius, y_radius);
            tile->val2 &= 0x0F;
        }
    }
}
/*
Purpose: Plant fungus on a tile and take its radius away from every owner.
// ORIGINAL: 0x00592480 sub_592480 0x00592480-0x00592550 FILE BYTE_EXACT
// symbol    ?fungus_radius@@YAXHH@Z
// NO IMAGE CALLER AND NO IMAGE REFERENCE AT ALL, so the name is derived from
//        the body, not from a catalogue entry: the centre tile gains
//        BIT_FUNGUS and loses MINE | SOLAR_TIDAL | FOREST | CONDENSER |
//        ECH_MIRROR | THERMAL_BORE (0x01E00050 - the terraforms fungus
//        cannot coexist with), and then the same
//        21-tile walk site_radius() does clears each tile's owner nibble.
// size      208 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl fungus_radius(int x, int y) {
    map_loc(x, y)->bit |= BIT_FUNGUS;
    map_loc(x, y)->bit &= ~(BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST
                            | BIT_CONDENSER | BIT_ECH_MIRROR
                            | BIT_THERMAL_BORE);
    for (int i = 0; i < 21; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            Map *tile = map_loc(x_radius, y_radius);
            tile->val2 &= 0x0F;
        }
    }
}
/*
Purpose: Search for the first landmark found within the radius range of the specified tile.
// ORIGINAL: 0x00592550 ?find_landmark@@YAHHHH@Z 0x00592550-0x005925F8 BYTE_EXACT
// size      168 bytes
// prototype
// callers   5   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: kill_landmark (0x005926F0), the only source-level caller, call_diff'd MORE
//        by exactly one call on this. Moved to map.h as `MEASURED inline`.
Return Value: Landmark offset or -1 if none found
Status: Complete
*/
// The landmark query is owned here. Its definition remains visible to the
// same-unit removal operation; external clients use the header declaration.

int __cdecl find_landmark(int x, int y, int radius_range_offset) {
    int radius = RadiusRange[radius_range_offset];
    for (int i = 0; i < radius; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            for (int lm = 0; lm < MapLandmarkCount; lm++) {
                if (x_radius == MapLandmark[lm].x && y_radius == MapLandmark[lm].y) {
                    return lm;
                }
            }
        }
    }
    return -1;
}
/*
Purpose: Set up a new landmark with the provided name at the specified tile.
// ORIGINAL: 0x00592600 ?new_landmark@@YAHHHPAD@Z 0x00592600-0x0059264B BYTE_EXACT
// LEVER (2026-09-03): two fixes, 3/28 -> BYTE_EXACT 28/28. (1) The guard
//            tests the GLOBAL, and the local is only bound afterwards -
//            `int landmark_offset = MapLandmarkCount++`, not a local read
//            hoisted above the guard. Reading the count into a local first
//            made VC6 push esi ahead of the early return and run the guard
//            in it. (2) The three writes index `MapLandmark[landmark_offset]`
//            directly instead of going through a `Landmark *lm`: the image
//            folds the array base into each access's own displacement
//            (`mov [eax + 0x949894], ecx` / `lea edx, [eax + 0x94989c]`),
//            where a pointer materialises base + offset once and pays
//            `add eax, 8` for the name. Mirror of lever 7 - here the
//            array form is what the image has, not the pointer form.
// symbol    ?new_landmark@@YAHHHPBD@Z
// size      75 bytes
// prototype int (__cdecl ?new_landmark@@YAHHHPAD@Z)(int xCoord, int yCoord, int8* name)
// callers   18   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00645460
Return Value: Landmark offset or -1 if max landmark count is reached
Status: Complete
*/
int __cdecl new_landmark(int x, int y, LPCSTR name) {
    if (MapLandmarkCount >= MaxLandmarkNum) {
        return -1;
    }
    int landmark_offset = MapLandmarkCount++;
    MapLandmark[landmark_offset].x = x;
    MapLandmark[landmark_offset].y = y;
    strcpy(MapLandmark[landmark_offset].name, name);
    return landmark_offset;
}
/*
Purpose: Check whether the specified faction has permission to name a landmark on the provided tile.
// ORIGINAL: 0x00592650 ?valid_landmark@@YAHHHH@Z 0x00592650-0x005926EF BYTE_EXACT
// size      159 bytes
// prototype int (__cdecl ?valid_landmark@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3B80 0x004E3EF0
Return Value: Does the faction have control of the tile to set a landmark? true/false
Status: Complete
*/
BOOL __cdecl valid_landmark(int x, int y, int faction_id) {
    int terr_faction_id = IsMultiplayerNet ? map_loc(x, y)->territory
        : whose_territory(faction_id, x, y, NULL, false);
    if (terr_faction_id == faction_id) {
        return true;
    }
    if (terr_faction_id > 0) {
        return false;
    }
    int base_id = base_find(x, y);
    return base_id < 0 ? true : Bases[base_id].faction_id_current == faction_id;
}
/*
Purpose: Remove the landmark at the specified tile.
// ORIGINAL: 0x005926F0 ?kill_landmark@@YAXHH@Z 0x005926F0-0x005927D0 BYTE_EXACT
// LEVER: the image's shift-down is a per-element `MapLandmark[i] =
//        MapLandmark[i+1]` loop (struct assignment of `Landmark`, 40 bytes,
//        compiles to `mov ecx,0xa; rep movsd` per iteration) - NOT the
//        single bulk `memcpy_s` a prior pass had substituted for "the
//        original loop". Writing the loop out took this from MISMATCH
//        5/86 (0.851) to MNEMONIC_ONLY 60/86 (1.000 similar).
// size      224 bytes
// prototype void (__cdecl ?kill_landmark@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl kill_landmark(int x, int y) {
    int landmark_to_kill = -1;
    int radius = RadiusRange[1];
    for (int i = 0; i < radius; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            for (int lm = 0; lm < MapLandmarkCount; lm++) {
                if (x_radius == MapLandmark[lm].x && y_radius == MapLandmark[lm].y) {
                    landmark_to_kill = lm;
                    goto remove_landmark;
                }
            }
        }
    }
    return;

remove_landmark:
    if (landmark_to_kill >= 0) {
        for (int i = landmark_to_kill; i < MapLandmarkCount - 1; i++) {
            MapLandmark[i] = MapLandmark[i + 1];
        }
        MapLandmarkCount -= 1;
    }
}
// ORIGINAL: 0x005927D0 ?delete_landmark@@YAXHH@Z 0x005927D0-0x00592938
// RECOVERY_BINDING: v1 {"evidence": ["git:1fe327399e8313c7d34d3b190f391eeeafdb0c28:src/recovered/units/005927d0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "Recover landmark clearing through real Map members, shared source-owned lookup arrays and MapWin callbacks. Preserve the duplicate Flat-table clear, numeric9,999,999 cache sentinel, store-before-clear order and native absence of extra window/bounds guards. Historical mappings and seed lessons are retained; no new byte/layout/runtime/linked-preservation claim."}
void __cdecl delete_landmark(int x, int y) {
    Map *tile = map_tiles() + MapLongitude * y + (x >> 1);
    const uint32_t old_flags = tile->bit2;
    if (old_flags == 0) {
        return;
    }

    tile->bit2 = old_flags & ~0x80u;
    const int type = static_cast<int32_t>(tile->bit2) >> 24;
    if (type < 0) {
        return;
    }

    const unsigned char *digits = AltDirectionTable + type * 5;
    if ((old_flags & 0x1u) != 0) {
        for (int i = 0; i < 5; ++i) AltBonusMoist[digits[i]] = 0;
    }
    if ((old_flags & 0x2u) != 0) {
        for (int i = 0; i < 5; ++i) AltBonusArid[digits[i]] = 0;
    }
    if ((old_flags & 0x200u) != 0) {
        for (int i = 0; i < 5; ++i) AltBonusRainy[digits[i]] = 0;
    }
    if ((old_flags & 0x100u) != 0) {
        for (int i = 0; i < 5; ++i) AltBonusFlat[digits[i]] = 0;
    }
    if ((old_flags & 0x800u) != 0) {
        // The native 0x59288C arm clears the flat table again. Keep this
        // duplicate rather than assigning a guessed table to the ridge flag.
        for (int i = 0; i < 5; ++i) AltBonusFlat[digits[i]] = 0;
    }

    const uint32_t encoded_type =
        (static_cast<uint32_t>(-1 - type) & 0xffu) << 24;
    tile->bit2 = (tile->bit2 & 0x00ffffffu) | encoded_type;
    UnkBitfield1 |= 4u;

    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *window = MapWinTable[slot];
        if (window->view_active_ != 0) {
            // Native 0x592915 stores immediate 0x0098967F at +8; this is a
            // numeric cache sentinel (9,999,999), not a pointer.
            window->field_8_ = 0x0098967Fu;
            window->clear_alt(x, y, 1);
        }
    }
}
/*
Purpose: Rebuild the altitude-bonus tables from the landmarks on the map.
// ORIGINAL: 0x00592940 ?fixup_landmarks@@YAXXZ 0x00592940-0x00592A74 FILE
// size      308 bytes
// prototype void (__cdecl ?fixup_landmarks@@YAXXZ)(void)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: the five 0x31-byte template runs the memcpy seeds the bonus tables
//        from are the ones the 0x0066EF28 note in map.h lists - 0x0068577C
//        (Arid), 0x006857F0 (Moist), 0x00685864 (Flat), 0x0068593C (Rainy),
//        0x006859B0 (Ocean) - and the tables they seed are the same
//        AltBonus* bindings mapwin.cpp's get_alt reads, so they moved to
//        map.h for this body to reach.
// TRIED: 10/100, every divergence register allocation. The copy loop and the
//        two bit arms agree; what is left is that the image spends `push ebx`
//        on a third callee-saved register and keeps `xor ebx,ebx` in it for
//        the `= 0` stores and the `bit2 == 0` test, where this tree materialises
//        each zero at its use - the extra-callee-saved-register case the brief
//        calls a hard ceiling. Its y loop also lives in a second stack slot
//        (`[ebp-8]`) that this tree does not need. Not chased further.
// TRIED: as 00592940.cpp the whole body was spelled with a raw
//        `int32_t *records` on the tile array; `map_loc()` names it and lets
//        the same `bit2` read through a member.
// BUG IN THE ORIGINAL: the BIT2_RIDGE arm (flags & 0x800) clears
//        AltBonusFlat - the SAME table and the SAME five entries the
//        BIT2_MESA arm (flags & 0x100) just cleared, unconditionally and
//        with nothing in between. Whatever the second arm was meant to
//        touch, it is a duplicate today; it is reproduced as written,
//        because removing it would change the program the image ships.
Return Value: n/a
Status: Complete
*/

void __cdecl fixup_landmarks() {
    // 113 bytes each: the image inlines every one as `mov ecx, 0x1c / rep
    // movsd / movsb`, which is 28 dwords plus the last byte.
    memcpy(AltBonusOcean, AltTemplateOcean, 113);
    memcpy(AltBonusFlat, AltTemplateFlat, 113);
    memcpy(AltBonusRainy, AltTemplateRainy, 113);
    memcpy(AltBonusArid, AltTemplateArid, 113);
    memcpy(AltBonusMoist, AltTemplateMoist, 113);

    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            Map *tile = map_loc(x, y);
            if (tile->bit2 == 0) {
                continue;
            }
            // Native5929F3 uses SAR, not an unsigned top-byte extraction.
            int type = static_cast<int32_t>(tile->bit2) >> 24;
            if (type >= 0) {
                continue;
            }
            type = -1 - type;
            const unsigned char *digits = AltDirectionTable + type * 5;
            if (tile->bit2 & BIT2_VOLCANO) {
                for (int i = 0; i < 5; i++) {
                    AltBonusArid[digits[i]] = 0;
                }
            }
            if (tile->bit2 & BIT2_CRATER) {
                for (int i = 0; i < 5; i++) {
                    // Native592A1D resets EBX before the crater byte stores.
                    AltBonusMoist[digits[i]] = 0;
                }
            }
        }
    }
}
// ORIGINAL: 0x00592A80 ?set_dirty@@YAXXZ 0x00592A80-0x00592AE6 FILE BYTE_EXACT
// size      102 bytes
// prototype
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592A80
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592a80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_dirty@@YAXXZ  at 0x00592A80  (102 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
void __cdecl set_dirty() {
    int y = 0;
    Map *tile = map_tiles();
    if (MapLatitudeBounds > 0) {
        int limit = MapLongitudeBounds;
        do {
            int x = y & 1;
            if (x < limit) {
                do {
                    tile->bit2 |= BIT_CONDENSER;
                    x += 2;
                    tile++;
                    limit = MapLongitudeBounds;
                } while (x < limit);
            }
            y++;
        } while (y < MapLatitudeBounds);
        UnkBitfield1 |= 4;
        return;
    }
    UnkBitfield1 |= 4;
}
// ORIGINAL: 0x00592AF0 ?clear_dirty@@YAXXZ 0x00592AF0-0x00592B3C FILE BYTE_EXACT
// size      76 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

void __cdecl clear_dirty() {
    int i = 0;
    Map *record = map_tiles();
    if (MapLatitudeBounds > 0) {
        int threshold = MapLongitudeBounds;
        do {
            int j = i & 1;
            if (j < threshold) {
                do {
                    record->bit2 |= 0x400000;
                    j += 2;
                    ++record;
                    threshold = MapLongitudeBounds;
                } while (j < threshold);
            }
            i++;
        } while (i < MapLatitudeBounds);
    }
}
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: single cached srcIndex/width/height locals across the record copy loop cause /O2 to spill to a `sub esp` stack frame where the original keeps everything in registers and the [ebp+8]/ [ebp+0xc] parameter slots as scratch; first divergence is the prologue itself (mov ebp,esp vs sub esp,N). Semantics verified against the Ghidra decompile and disasm field-by-field.
// ORIGINAL: 0x00592B40 sub_592b40 0x00592B40-0x00592D6F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00592b40.cpp: canonical full559byte original span and retained TRIED evidence", "historical pre-move source:src/map.cpp contained map_init allocation/failure and map read/write lifetime", "source:recovery/tasks/map-allocation-owner-closure.json: coordinator-reviewed producer/reader closure"], "reason": "Original592B40 is the map allocation owners shrink/resample producer: it saves prior allocation/dimensions, obtains new map storage through map_init, copies terrain fields and releases old tiles. Source-owned pointer cells preserve actual allocation and consumer relationships."}
// symbol    ?map_resample@@YAXHH@Z
void __cdecl map_resample(int width, int height) {
    int old_width = MapLongitudeBounds;
    int old_height = MapLatitudeBounds;
    MapResamplePreviousWidth = old_width;
    MapResamplePreviousHeight = old_height;
    if (width >= old_width || height >= old_height) {
        return;
    }
    Map *old_tiles = map_tiles();
    MapResamplePreviousTiles = old_tiles;
    map_tiles() = 0;
    MapLongitudeBounds = width;
    MapLatitudeBounds = height;
    // The original does not release the previous abstract allocation here,
    // nor test map_init's result. Preserve that lifecycle and failure path.
    map_init();
    MapSeaLevelCouncil = 0;
    MapLandmarkCount = 0;
    MapSeaLevel = 0;
    MapRandSeed = random(0, 0x7fff) + 1;
    Map *new_tiles = map_tiles();
    for (int i = 0; i < MapArea; ++i) {
        Map *tile = new_tiles + i;
        tile->climate = ALT_BIT_OCEAN;
        tile->contour = 20;
        tile->val2 = 15;
        tile->region = 0;
        tile->visibility = 0;
        tile->bit = 0;
        tile->val3 = 0;
        ZeroMemory(&tile->bit2, sizeof(tile->bit2) + sizeof(tile->bit_visible));
    }
    for (int y = 0; y < MapLatitudeBounds; ++y) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            int source_y = MapResamplePreviousHeight * y / MapLatitudeBounds;
            int source_x = (MapResamplePreviousWidth * x / MapLongitudeBounds) >> 1;
            Map *destination = new_tiles + MapLongitude * y + (x >> 1);
            Map *source = old_tiles + (old_width >> 1) * source_y + source_x;
            destination->contour = source->contour;
            destination->bit = source->bit;
            destination->bit2 = source->bit2;
            destination->val3 = source->val3 & 0xc0;
        }
    }
    if (old_tiles) {
        free(old_tiles);
    }
    // The scratch pointer deliberately remains stale after free, as original.
    world_climate();
}
