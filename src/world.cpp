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
// World-generation algorithms; shared Map storage remains in map.cpp.
#include "stdafx.h"
#include "popup.h"
#include "general.h"
#include "win.h"
#include "map.h"
#include "map_coordinates.h"
#include "fractal.h"  // Fractal - world_erosion / world_fungus build one on the stack
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "interface_palette.h"
#include "path.h"
#include "random.h"
#include "strings.h"
#include "technology.h"
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

/*
Purpose: Handle setting the world altitude.
// ORIGINAL: 0x005C2020 ?world_alt_set@@YAXHHHH@Z 0x005C2020-0x005C2374
// TRIED: MISMATCH plateau, not chased to BYTE_EXACT (852-byte function,
//   282 image instructions, best 48/282 similar 0.298 at /c /O2 /Gy /GR-
//   /Oy- /GX). call_diff agrees on call count. The compiled body runs 396
//   instructions against the image's 282 from early on (RadiusRange/
//   RadiusOffsetX array reads land in a different register, and the loop
//   guard around `alt_at` pulls in two extra real calls the image's copy
//   doesn't show at that point) - a deep restructuring, not a 2-3 candidate
//   fix, so left at the pre-existing body.
// size      852 bytes
// prototype void (__cdecl ?world_alt_set@@YAXHHHH@Z)(int xCoord, int yCoord, int altitude, int isSetBoth)
// callers   23   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591290 0x005918F0 0x00591B10 0x00592030
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_alt_set(int x, int y, int altitude, BOOL is_set_both) {
    ZeroMemory(MapBaseSubmergedCount, sizeof(int) * MaxPlayerNum);
    memset(MapBaseIdClosestSubmergedVeh, -1, sizeof(int) * MaxPlayerNum);
    if (on_map(x, y)) {
        is_set_both ? alt_set_both(x, y, altitude) : alt_set(x, y, altitude);
    }
    BOOL has_set_alt;
    int i, alt;
    // RadiusRange[0..1] is [1,9): the eight nearest neighbors, not
    // [9,25). Original 5C20A9/5C2221 reads one entry before its bound.
    for (i = 1, alt = altitude - 1; i < altitude; i++, alt--) {
        has_set_alt = false;
        for (int j = RadiusRange[i - 1]; j < RadiusRange[i]; j++) {
            int x_radius = xrange(x + RadiusOffsetX[j]);
            int y_radius = y + RadiusOffsetY[j];
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) < alt) {
                if (is_set_both) {
                    alt_set_both(x_radius, y_radius, alt);
                    if (anything_at(x_radius, y_radius) < 0 && !bonus_at(x_radius, y_radius, 0)) {
                        owner_set(x_radius, y_radius, -1);
                    }
                } else {
                    alt_set(x_radius, y_radius, alt);
                }
                has_set_alt = true;
            }
        }
        if (!has_set_alt) {
            break;
        }
    }
    int max_alt = 8 - altitude;
    for (i = 1, alt = i + altitude; i < max_alt; i++, alt++) {
        has_set_alt = false;
        for (int j = RadiusRange[i - 1]; j < RadiusRange[i]; j++) {
            int x_radius = xrange(x + RadiusOffsetX[j]);
            int y_radius = y + RadiusOffsetY[j];
            // Original 5C22AE/5C22B0 skips existing heights <= this cap.
            // This ring lowers high neighbors; it must not raise low ones.
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) > alt) {
                if (is_set_both) {
                    alt_set_both(x_radius, y_radius, alt);
                    if (anything_at(x_radius, y_radius) < 0 && !bonus_at(x_radius, y_radius, 0)) {
                        owner_set(x_radius, y_radius, -1);
                    }
                } else {
                    alt_set(x_radius, y_radius, alt);
                }
                has_set_alt = true;
            }
        }
        if (!has_set_alt) {
            break;
        }
    }
}

/*
Purpose: Handle raising the altitude of the specified tile.
// ORIGINAL: 0x005C2380 ?world_raise_alt@@YAXHH@Z 0x005C2380-0x005C23DE BYTE_EXACT
// LEVER: the image `inc eax`s BEFORE the push, so the argument is `altitude + 1`, not the
//            original value - `altitude++` (post-increment) pushed the pre-increment value;
//            `++altitude` fixed it. Also `int altitude`, not `uint32_t`: the image's `< 6`
//            compare is `jge`/signed, `uint32_t` gave `jae`. Both together: 0.961 -> BYTE_EXACT.
// size      94 bytes
// prototype void (__cdecl ?world_raise_alt@@YAXHH@Z)(int xCoord, int yCoord)
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C2020
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_raise_alt(int x, int y) {
    if (on_map(x, y)) {
        int altitude = alt_at(x, y);
        if (altitude < 6) {
            world_alt_set(x, y, ++altitude, true);
        }
    }
}

/*
Purpose: Handle lowering the altitude of the specified tile.
// ORIGINAL: 0x005C23E0 ?world_lower_alt@@YAXHH@Z 0x005C23E0-0x005C243B BYTE_EXACT
// LEVER: same as world_raise_alt (0x005C2380) - the image `dec eax`s BEFORE the push, so
//            the pushed argument is `altitude - 1`; `altitude--` (post-decrement) pushed the
//            pre-decrement value, `--altitude` fixed it. `int altitude`, not `uint32_t`, to
//            match.
// size      91 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C2020
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_lower_alt(int x, int y) {
    if (on_map(x, y)) {
        int altitude = alt_at(x, y);
        if (altitude) {
            world_alt_set(x, y, --altitude, true);
        }
    }
}

/*
Purpose: Set up the brush for creating world terrain.
// ORIGINAL: 0x005C2440 ?brush@@YAXHHH@Z 0x005C2440-0x005C27E1
// TRIED: MISMATCH plateau, not chased to BYTE_EXACT (929-byte function,
//   304 image instructions, best 8/304 similar 0.370 at /c /O1 /Ob0 /Gy
//   /GR- /Oy- /GX). call_diff agrees on call count. Diverges from the first
//   loop iteration on: which register RadiusOffsetX[i]/y_radius land in
//   (matches the file's known ebx/edi plateau), and the `on_map()` +
//   `alt_at()` guard is ordered/interleaved differently than this tree's
//   compile produces - a deep restructuring, not a 2-3 candidate fix, so
//   left at the pre-existing body.
// size      929 bytes
// prototype void (__cdecl ?brush@@YAXHHH@Z)(int xCoord, int yCoord, int altitude)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AEF0 0x00591AD0 0x005C2020
Return Value: n/a
Status: Complete - testing
*/
void __cdecl brush(int x, int y, int altitude) {
    BOOL use_draw_radius = false;
    for (int i = 0; i < 4; i++) {
        if (i != 2) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) != altitude) {
                BOOL skip = false;
                for (uint32_t j = 0; j < 8; j++) {
                    int x_rad_base = xrange(x_radius + VehMoveDeltaX[j]);
                    int y_rad_base = y_radius + VehMoveDeltaY[j];
                    if (on_map(x_rad_base, y_rad_base) && !is_ocean(x_rad_base, y_rad_base)
                        && !region_at(x_rad_base, y_rad_base)) {
                        skip = true;
                    }
                }
                if (!skip) {
                    if (altitude != 3) {
                        for (uint32_t j = 0, k = 0; j < 7; j += 3, k += 2) {
                            int x_rad_base = xrange(x_radius + VehMoveDeltaX[k + 1]);
                            int y_rad_base = y_radius + VehMoveDeltaY[k + 1];
                            if (on_map(x_rad_base, y_rad_base)
                                && altitude_at(x_rad_base, y_rad_base) == ALT_BIT_SHORE_LINE) {
                                int x_rad_base2 = xrange(x_radius + VehMoveDeltaX[(k + 2) % 8]);
                                int y_rad_base2 = y_radius + VehMoveDeltaY[(k + 2) % 8];
                                int x_rad_base3 = xrange(x_radius + VehMoveDeltaX[k % 8]);
                                int y_rad_base3 = y_radius + VehMoveDeltaY[k % 8];
                                if (on_map(x_rad_base2, y_rad_base2) 
                                    && on_map(x_rad_base3, y_rad_base3)
                                    && altitude_at(x_rad_base2, y_rad_base2) < ALT_BIT_SHORE_LINE
                                    && altitude_at(x_rad_base3, y_rad_base3) < ALT_BIT_SHORE_LINE) {
                                    skip = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!skip) {
                        region_set(x_radius, y_radius, 1);
                        use_draw_radius |= alt_at(x_radius, y_radius) != altitude;
                        world_alt_set(x_radius, y_radius, altitude, true);
                        if (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW && altitude > 3) {
                            temp_set(x_radius, y_radius, 1);
                        }
                        BrushVal1 += 1;
                        BrushVal2 += 1;
                    }
                }
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW && use_draw_radius) {
        draw_radius(x, y, 2, 2);
    }
}

/*
Purpose: Paint land to assist in the creation of the world terrain.
// ORIGINAL: 0x005C27F0 ?paint_land@@YAXHHHH@Z 0x005C27F0-0x005C28E6
// LEVER: two fixes, 0.849 -> 0.904 similar (9/89). (1) `x_rad_base`/
//        `y_rad_base` are a CUMULATIVE random walk - the image's inlined
//        `xrange()` adds `VehMoveDeltaX[offset]` to the PREVIOUS `esi`
//        (`x_rad_base`), and `edi` (`y_rad_base`) is a plain running
//        `add edi, VehMoveDeltaY[offset]` with no reset - not an offset from
//        the fixed `x`/`y` parameters every iteration, which is a BUG this
//        recovery had (paints a single-tile-radius dot around the start
//        instead of wandering). (2) `offset` as `int`: the image's
//        `rand() % 8` carries the full signed-modulo fixup
//        (`and 0x80000007; jns; dec; or 0xfffffff8; inc`), which `uint32_t`
//        drops to a plain `and eax, 7`.
// TRIED: not chased past this MISMATCH plateau - remaining gap starts in
//            the prologue frame-size (image reserves 3 stack dwords, this
//            tree keeps `unk_val` in a register alone).
// size      246 bytes
// prototype void (__cdecl ?paint_land@@YAXHHHH@Z)(int xCoord, int yCoord, int altitude, int radius)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C2440 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl paint_land(int x, int y, int altitude, int radius) {
    BrushVal2 = 0;
    for (int i = 0; i < 2000 && BrushVal2 < radius; i++) {
        int search = -1;
        uint32_t unk_val = 0;
        int x_rad_base = x;
        int y_rad_base = y;
        do {
            brush(x_rad_base, y_rad_base, altitude);
            int offset = rand() % 8 | 1;
            if (offset == search) {
                // Pinned5C2841 keeps the low dword of IMUL, then5C284C
                // uses signed JLE on that wrapped product plus two. Large
                // positive file-loaded settings survive world_build's test.
                uint32_t coverage = static_cast<uint32_t>(MapLandCoverage);
                uint32_t square = coverage * coverage;
                uint32_t threshold_bits = square + 2u;
                int threshold;
                memcpy(&threshold, &threshold_bits, sizeof(threshold));
                ++unk_val;
                int count;
                memcpy(&count, &unk_val, sizeof(count));
                if (count > threshold) {
                    // Original5C284E..2854 is low-dword SUB/LEA and AND7.
                    offset = (static_cast<uint32_t>(offset) - square
                        + unk_val - 2u) & 7u;
                }
            } else {
                search = offset;
                unk_val = 0;
            }
            x_rad_base = xrange(x_rad_base + VehMoveDeltaX[offset]);
            y_rad_base += VehMoveDeltaY[offset];
            i++;
        } while (i < 2000 && on_map(x_rad_base, y_rad_base) && BrushVal2 < radius);
    }
}

/*
Purpose: Build out the map continents.
// ORIGINAL: 0x005C28F0 ?build_continent@@YAXH@Z 0x005C28F0-0x005C2B3C
// LEVER: `osmx calls` shows exactly 2 game calls (paint_land,
//        do_all_non_input) plus rand() itself - `rnd(bounds, NULL)` is
//        cross-TU and can never inline, so all three call sites had to be
//        rewritten as rnd's own body (`(bounds - 1 > 0) ? rand() % bounds :
//        0`), the same idiom as alt_set_both. The region-zero loop also
//        moved to the `Map *tile` pointer-walk idiom (map_wipe's lever).
//        0.658 -> 0.698 similar (13/201).
// TRIED: not chased past this MISMATCH plateau - 588-byte body, deeply
//            nested ratio/radius conditionals; the remaining gap starts in
//            the very first loop's own scheduling (image loads MapArea into
//            ecx before zeroing eax, this tree the other order).
// size      588 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C27F0 0x005FCB20 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl build_continent(int size) {
    Map *tile = map_tiles();
    for (int i = 0; i < MapArea; i++) {
        tile->region = 0;
        tile++;
    }
    int coverage = MapLandCoverage;
    if (coverage && BrushVal1 >= WorldBuildVal1) {
        coverage--;
    }
    int radius = WorldBuilder.continent_mod * coverage * coverage + WorldBuilder.continent_base;
    int x;
    int y;
    int count = 0;
    do {
        int x_bound = MapLongitudeBounds - (MapIsFlat * 8);
        x = ((x_bound - 1 > 0) ? rand() % x_bound : 0) + MapIsFlat * 4;
        int y_bound = MapLatitudeBounds - 8;
        y = ((y_bound - 1 > 0) ? rand() % y_bound : 0) + 4;
        if (x & 1) {
            x--;
        }
        if (y & 1) {
            x++; // validate whether this should be y and/or reasoning for x
        }
    } while (count++ < 100 && alt_at(x, y) >= ALT_BIT_SHORE_LINE);
    if (alt_at(x, y) < ALT_BIT_SHORE_LINE) {
        int ratio = (size * 3200) / MapArea;
        if (ratio > WorldBuilder.cont_size_ratio5 || BrushVal1 >= WorldBuildVal1) {
            radius /= 4;
            if (BrushVal1 >= WorldBuildVal1 && ratio > WorldBuilder.cont_size_ratio5) {
                radius = rand() % 3 + 1;
            }
        } else if (ratio <= WorldBuilder.cont_size_ratio4) {
            if (ratio <= WorldBuilder.cont_size_ratio3) {
                if (ratio <= WorldBuilder.cont_size_ratio2) {
                    if (ratio < WorldBuilder.cont_size_ratio1 && MapLandCoverage > 1) {
                        radius += radius / 2;
                    }
                } else {
                    uint32_t r_bound_bits = 4u - static_cast<uint32_t>(MapLandCoverage);
                    int r_bound;
                    memcpy(&r_bound, &r_bound_bits, sizeof(r_bound));
                    // Original5C2A9D LEA wraps before TEST/JLE5C2AA0/2.
                    uint32_t bound_test_bits = r_bound_bits - 1u;
                    int bound_test;
                    memcpy(&bound_test, &bound_test_bits, sizeof(bound_test));
                    if ((bound_test > 0) ? rand() % r_bound : 0) {
                        radius /= 2;
                    }
                }
            } else {
                radius /= 2;
            }
        } else {
            radius /= 3;
        }
        if (BrushVal1 < WorldBuildVal1 && radius < 4) {
            radius = 4;
        }
        paint_land(x, y, ALT_SHORE_LINE, radius);
        do_all_non_input();
    }
}

/*
Purpose: Build out the map hills.
// ORIGINAL: 0x005C2B40 ?build_hills@@YAXH@Z 0x005C2B40-0x005C2CA8
// LEVER: same as build_continent (0x005C28F0) - `osmx calls` shows only
//        paint_land/do_all_non_input plus rand() itself, so the two
//        `rnd(bounds, NULL)` cross-TU calls were rewritten as rnd's own
//        body. 0.677 -> 0.743 similar.
// TRIED: not chased past this MISMATCH plateau - 360-byte body, same
//            prologue frame-size gap as build_continent.
// size      360 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C27F0 0x005FCB20 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl build_hills(int altitude) {
    int x;
    int y;
    int i = 0;
    for (;;) {
        int x_bound = MapLongitudeBounds - ((MapIsFlat & 1) * 8);
        x = ((x_bound - 1 > 0) ? rand() % x_bound : 0) + ((MapIsFlat & 1) * 4);
        int y_bound = MapLatitudeBounds - 8;
        y = ((y_bound - 1 > 0) ? rand() % y_bound : 0) + 4;
        if (x & 1) {
            x--;
        }
        if (y & 1) {
            x++; // should this have been y?
        }
        i += 2;
        if (i > 100) {
            return;
        }
        BOOL rejected_seed = false;
        for (int j = 0; j < 9; j++) {
            int x_radius = xrange(x + RadiusOffsetX[j]);
            int y_radius = y + RadiusOffsetY[j];
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) < (altitude - 1)) {
                rejected_seed = true;
                break;
            }
        }
        if (rejected_seed) {
            continue;
        }
        const uint32_t plat_mod_bits = WorldBuilder.plateau_mod
            * (2u - static_cast<uint32_t>(MapOceanCoverage));
        int plat_mod;
        memcpy(&plat_mod, &plat_mod_bits, sizeof(plat_mod));
        if (altitude != ALT_1_LEVEL_ABOVE_SEA) {
            plat_mod /= 4;
        }
        int plat_base;
        memcpy(&plat_base, &WorldBuilder.plateau_base, sizeof(plat_base));
        if (MapOceanCoverage >= 2) {
            plat_base /= 2;
        }
        const uint32_t radius_bits = static_cast<uint32_t>(plat_mod)
            + static_cast<uint32_t>(plat_base);
        int radius;
        memcpy(&radius, &radius_bits, sizeof(radius));
        paint_land(x, y, altitude, radius);
        do_all_non_input();
        return;
    }
}

/*
Purpose: Build out the world river beds.
// ORIGINAL: 0x005C3680 ?world_riverbeds@@YAXXZ 0x005C3680-0x005C38AF
// LEVER: `osmx calls` shows only ONE game call (bit_set) plus rand() itself
//        - both `rnd(bounds, NULL)` sites are cross-TU and were rewritten
//        as rnd's own body (same idiom as build_continent/build_hills), and
//        the riverbed-clearing loop moved to the `Map *tile` pointer-walk
//        idiom (map_wipe's lever). 0.711 -> 0.784 similar.
// TRIED: not chased past this MISMATCH plateau - same `is_ocean`
//            map_tiles-load-timing family ceiling as the standalone
//            `is_ocean`/`port_to_port` plateau this batch's sibling pass
//            already documented, on top of a 559-byte body.
// size      559 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_riverbeds() {
    Map *tile = map_tiles();
    // Native5C3697..36B6 retains this loaded area for the signed clearing
    // loop and the low-dword target product; a high-bit area skips clearing.
    int area = MapArea;
    int i;
    for (i = 0; i < area; i++) {
        tile->bit &= ~(BIT_RIVERBED);
        tile++;
    }
    int riverbed_count = 0;
    // Pinned5C36C4..36FF: low-dword multiply/add/subtract, signed /3,
    // another low-dword multiply by the retained area, then signed /3200.
    // Unsigned builder storage must not turn these two quotients unsigned.
    uint32_t rain_bits = WorldBuilder.rivers_base
        + static_cast<uint32_t>(MapCloudCover) * WorldBuilder.rivers_rain_mod;
    uint32_t target_bits = (4u - static_cast<uint32_t>(MapOceanCoverage)) * rain_bits;
    int target;
    memcpy(&target, &target_bits, sizeof(target));
    uint32_t scaled_bits = static_cast<uint32_t>(area) * static_cast<uint32_t>(target / 3);
    int scaled;
    memcpy(&scaled, &scaled_bits, sizeof(scaled));
    int max_riverbeds = scaled / 3200;
    for (i = 0; i < 4000 && riverbed_count < max_riverbeds; i++) {
        int x_bound = MapLongitudeBounds;
        int x = (x_bound - 1 > 0) ? rand() % x_bound : 0;
        int y_bound = MapLatitudeBounds;
        int y = (y_bound - 1 > 0) ? rand() % y_bound : 0;
        if (x & 1) {
            x--;
        }
        if (y & 1) {
            x++;
        }
        if (!is_ocean(x, y) && !(bit_at(x, y) & BIT_RIVERBED)) {
            uint32_t search_alt = 0;
            int x_search = -1;
            int y_search = -1;
            for (int j = 0; j < RadiusRange[4]; j++) {
                int x_radius = xrange(x + RadiusOffsetX[j]);
                int y_radius = y + RadiusOffsetY[j];
                if (on_map(x_radius, y_radius)) {
                    if (bit_at(x_radius, y_radius) & BIT_RIVERBED) {
                        break; // exit if riverbed already in search area
                    }
                    if (j < RadiusRange[3]) { // limit scope of where riverbeds can be in search
                        uint32_t alt = alt_at(x_radius, y_radius);
                        if (alt > search_alt) {
                            search_alt = alt;
                            x_search = x_radius; // replaced storing j offset with radius values
                            y_search = y_radius;
                        }
                    }
                }
            }
            if (x_search >= 0) {
                bit_set(x_search, y_search, BIT_RIVERBED, true);
                riverbed_count++;
            }
        }
    }
}

/*
Purpose: Determine if there are any issues with how the world continents are set up.
// ORIGINAL: 0x005C40F0 ?world_validate@@YAHXZ 0x005C40F0-0x005C416B
// LEVER: three fixes, 0.609 -> 0.914 similar (20/52). (1) `val1`/`val2`/
//        `tile_count` as `int`, not `uint32_t` - the image's final
//        threshold uses a SIGNED div-by-3 magic-constant `imul` and a
//        SIGNED `sar` div-by-2, both lost under unsigned. (2) the
//        accumulate step rewritten `if (tile_count >= val2) {val1=val2;
//        val2=tile_count;} else if (tile_count >= val1) {val1=tile_count;}`
//        - logically the same as the nested `if (tile_count < val2)` this
//        had, but matches the image's actual `jl`/fall-through order.
//        (3) BRANCH POLARITY on the ternary: the image's fall-through
//        (no jump) computes `val2/2` and jumps away for
//        `MapLandCoverage==1`'s `(val2*2)/3` - `if (MapLandCoverage != 1)
//        {val2/2} else {(val2*2)/3}` matches; the textual `==1` order did
//        not.
// TRIED: remaining gap is an esi/edi register swap between `val1` and
//            the `region` loop counter, present at every flag set tried;
//            swapping `val1`/`val2` declaration order made it one
//            instruction worse (19/52), not better.
// size      123 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0059C790
Return Value: Are there any issues with the world continents? true/false
Status: Complete - testing
*/
BOOL __cdecl world_validate() {
    Paths.continents();
    int val1 = 0; // rename to better var descriptions
    int val2 = 0; // rename to better var descriptions
    for (uint32_t region = 1; region < MaxRegionLandNum; region++) {
        if (!bad_reg(region)) {
            int tile_count = Continents[region].tile_count;
            if (tile_count >= val2) {
                val1 = val2;
                val2 = tile_count;
            } else if (tile_count >= val1) {
                val1 = tile_count;
            }
        }
    }
    int threshold;
    if (MapLandCoverage != 1) {
        threshold = val2 / 2;
    } else {
        threshold = (val2 * 2) / 3;
    }
    return val1 < threshold;
}

/*
Purpose: Set up the world temperature.
// ORIGINAL: 0x005C4170 ?world_temperature@@YAXXZ 0x005C4170-0x005C4401
// TRIED: MISMATCH plateau, not chased to BYTE_EXACT (657-byte function,
//   226 image instructions). LEVER that DID help: `MapLatitudeBounds /
//   WorldBuilder.solar_energy` (and the three siblings) is `int / uint32_t`,
//   which promotes to an UNSIGNED `div`; the image uses signed `idiv` at all
//   four, so each RHS is cast `(int)`. Raised best similarity 0.611 -> 0.654
//   (best flags /c /O2 /Ob0 /Gy /GR- /GX). Remaining gap starts at the
//   `bit2_at`/`code_at` pair in the loop guard: the image inlines both, this
//   tree's compile keeps them as real calls at this nesting depth - an
//   inlining-heuristic difference, not chased further.
// size      657 bytes
// prototype 
// callers   2   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591AD0 0x005FCB20 0x006257E0 0x00625810
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_temperature() {
    random_reseed(MapRandSeed + 17);
    // SIGNED DIVISION: `WorldBuilder`'s fields are `uint32_t`, and dividing
    // `int / uint32_t` promotes to an UNSIGNED `div`; the image uses `idiv`
    // at all four, so the RHS is cast back to `int` to match.
    int temp_heat = MapLatitudeBounds / (int)WorldBuilder.solar_energy;
    int thermal_banding = MapLatitudeBounds / (int)WorldBuilder.thermal_band;
    int thermal_deviance = MapLatitudeBounds / (int)WorldBuilder.thermal_deviance;
    int global_warming = MapLatitudeBounds / (int)WorldBuilder.global_warming;
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            if ((bit2_at(x, y) & (BIT2_UNK_80000000 | BIT2_CRATER)) != BIT2_CRATER
                || code_at(x, y) >= 21) {
                int rand_seed = random(0, thermal_deviance * 2);
                // The native load follows the first random call; INC423C
                // wraps even when a file-loaded orbit is INT_MAX.
                uint32_t orbit_bound_bits = static_cast<uint32_t>(MapPlanetaryOrbit) + 1u;
                int orbit_bound;
                memcpy(&orbit_bound, &orbit_bound_bits, sizeof(orbit_bound));
                int rand_orbit = random(0, orbit_bound);
                int val1 = (MapLatitudeBounds / 2) - rand_seed - y + thermal_deviance;
                if (val1 < 0) { // abs?
                    val1 = rand_seed - (MapLatitudeBounds / 2) - thermal_deviance + y;
                }
                // Reload the orbit after random as at426E. The products and
                // SUB/LEA4279..4290 keep low dwords; IDIV4294 is signed.
                uint32_t orbit_heat_bits = static_cast<uint32_t>(temp_heat)
                    * (static_cast<uint32_t>(MapPlanetaryOrbit) - 1u);
                uint32_t council_heat_bits = static_cast<uint32_t>(MapSeaLevelCouncil)
                    * static_cast<uint32_t>(global_warming);
                uint32_t band_bits = static_cast<uint32_t>(thermal_banding / 2)
                    + (static_cast<uint32_t>(val1) - orbit_heat_bits - council_heat_bits) * 2u;
                int band;
                memcpy(&band, &band_bits, sizeof(band));
                int val2 = band / thermal_banding;
                int temperature = (val2 > 2) ? ((val2 <= 9) + 1) : 3;
                int alt = alt_at(x, y);
                for (int i = 0; i < 8; i++) {
                    int x_radius = xrange(x + RadiusOffsetX[i]);
                    int y_radius = y + RadiusOffsetY[i];
                    if (on_map(x_radius, y_radius) && bit_at(x_radius, y_radius) & BIT_THERMAL_BORE
                        && (alt <= ALT_1_LEVEL_ABOVE_SEA || alt_at(x_radius, y_radius) >= alt)) {
                        temperature++;
                        break;
                    }
                }
                if (temperature > 1) {
                    if (alt <= ALT_2_LEVELS_ABOVE_SEA) {
                        // == ALT_2_LEVELS_ABOVE_SEA, simplify?
                        if (alt > ALT_1_LEVEL_ABOVE_SEA && !rand_orbit) {
                            temperature--;
                        }
                    } else if (MapPlanetaryOrbit < 2 || !random(0, 2)) {
                        temperature--;
                    }
                }
                temp_set(x, y, (uint8_t)range(temperature - 1, 0, 2));
            }
        }
        do_all_non_input();
    }
}

/*
Purpose: Score the specified tile as a site for a new base.
// ORIGINAL: 0x005C4FD0 ?world_site@@YAHHHH@Z 0x005C4FD0-0x005C55B5
// TRIED: site_tile/site_xrange were being emitted as real calls; __forceinline on both
//            (site_tile at 2423, site_xrange at 1730) brought the call count from 9 down to
//            the image's 3 (2x bonus_at, 1x goody_at) but did not reach BYTE_EXACT.
// TRIED: remaining mismatch is register allocation across a 1509-byte body, the prior
//            pass's noted plateau for this family - not chased further.
// size      1509 bytes
// prototype 
// callers   8   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00592030 0x00592140
Return Value: Site value 0 (unusable) or 1-15
Status: Complete

Two rings. The outer one, radius offsets 21 to 48, only counts how crowded the
neighbourhood already is; the inner one, the twenty-one tiles of a base's own
production radius, is what is actually scored. `is_ocean_site` says which kind
of site is wanted, and it is compared against the centre tile's own altitude
rather than being normalised - a caller passing anything but 0 or 1 never
matches.

The original accumulates a rainfall total across the inner ring into a stack
local at ebp-0x30 - zeroed at 0x005C50C5, read at 0x005C5327, stored back at
0x005C5339 - and never reads it again. It is a dead local, not observable from
outside the frame, and it is not transcribed.
*/
int __cdecl world_site(int x, int y, BOOL is_ocean_site) {
    int crowding = 0;
    for (int i = 21; i < 49; i++) {
        int x_radius = site_xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (y_radius < 0 || y_radius >= MapLatitudeBounds || x_radius < 0
            || x_radius >= MapLongitudeBounds) {
            continue;
        }
        Map *tile = site_tile(x_radius, y_radius);
        if (!(tile->bit & BIT_BASE_IN_TILE)) {
            continue;
        }
        crowding++;
        if (i >= 25) {
            continue; // the first four count double, and can count quadruple
        }
        crowding++;
        if ((site_tile(x, y)->climate & 0xE0) < ALT_BIT_SHORE_LINE
            && (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
            crowding += 2;
        }
    }
    int special = 0;       // bonus resources, landmarks and pods
    int rich_terrain = 0;  // monolith, forest, or rainy rolling land above sea
    int fair_terrain = 0;  // rainy rolling land that is none of those
    int ocean_count = 0;
    int deep_count = 0;    // ocean below the shelf
    int river_tiles = 0;
    int fungus_tiles = 0;
    int rocky_tiles = 0;
    BOOL big_continent = false;
    /*
    * The original's frame aliases this pointer onto the slot that held the
    * MapIsFlat copy, and the epilogue reads it whether or not the loop ever
    * assigned it. With every tile of the radius out of bounds that is a
    * dereference of 0 or 1. Reproduced rather than guarded: the two are the
    * same program, and the caller that could reach it is off the map already.
    */
    Map *radius_tile = reinterpret_cast<Map *>(
        static_cast<uintptr_t>(MapIsFlat & 1));
    for (i = 0; i < 21; i++) {
        int x_radius = site_xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (y_radius < 0 || y_radius >= MapLatitudeBounds || x_radius < 0
            || x_radius >= MapLongitudeBounds) {
            continue;
        }
        radius_tile = site_tile(x_radius, y_radius);
        int altitude = radius_tile->climate & 0xE0;
        if (altitude < ALT_BIT_SHORE_LINE) {
            ocean_count++;
            if (altitude < ALT_BIT_OCEAN_SHELF) {
                deep_count++;
            }
            // Only the nine innermost tiles decide whether this is a big land.
            if (i < 9 && Continents[radius_tile->region].tile_count >= 50) {
                big_continent = true;
            }
            if (!i || !bonus_at(x_radius, y_radius, 1)) {
                continue;
            }
            Map *bonus_tile = site_tile(x_radius, y_radius);
            if ((bonus_tile->bit & BIT_FUNGUS)
                && (int)(bonus_tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
                continue;
            }
            special++;
            continue;
        }
        int rockiness = radius_tile->val3 >> 6;
        if (rockiness && i && rockiness > 1) {
            rocky_tiles++;
        }
        if (!(radius_tile->bit & BIT_FUNGUS)) {
            if (bonus_at(x_radius, y_radius, 1)) {
                special += i ? 2 : -1;
            }
            uint32_t bit2 = site_tile(x_radius, y_radius)->bit2;
            // The top byte is the landmark's tile sequence code; past nine the
            // tile is too far along the landmark to be worth anything.
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO
                && (bit2 & 0xFF000000) < 0x09000000) {
                special += i ? 1 : -1;
            }
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_CRATER)) == BIT2_CRATER
                && (bit2 & 0xFF000000) < 0x09000000) {
                special += i ? 2 : -1;
            }
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_URANIUM)) == BIT2_URANIUM) {
                special += i ? 1 : -3;
            }
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_JUNGLE)) == BIT2_JUNGLE
                && (int)(radius_tile->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
                special += 2;
            }
            if (i && goody_at(x_radius, y_radius)) {
                special++;
            }
        }
        int elevation = (radius_tile->climate >> 5) - 2;
        if (elevation < 0) {
            elevation = 0;
        }
        int rainfall = (radius_tile->climate >> 3) & 3;
        if (rockiness == ROCKINESS_ROCKY
            || (radius_tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO))
                == BIT2_VOLCANO) {
            rainfall = 0;
        }
        uint32_t bit = radius_tile->bit;
        if (bit & BIT_FUNGUS) {
            fungus_tiles++;
        }
        if (bit & BIT_BASE_RADIUS) {
            continue; // already worked by somebody
        }
        BOOL is_rich = (bit & (BIT_MONOLITH | BIT_FOREST)) != 0;
        if (!is_rich && rainfall && rockiness == ROCKINESS_ROLLING) {
            if (elevation && !(bit & BIT_FUNGUS)) {
                is_rich = true;
            } else {
                fair_terrain += i ? 1 : -1;
                if (rainfall > rockiness && !(bit & BIT_FUNGUS)) {
                    fair_terrain += i ? 1 : -1;
                }
            }
        }
        if (is_rich) {
            rich_terrain += i ? 1 : -1;
            if ((bit & BIT_MONOLITH) && !(bit & BIT_FOREST)
                && (rainfall > 1 || elevation > 1)) {
                rich_terrain += i ? 1 : -2;
            }
        }
        if (bit & BIT_RIVER) {
            river_tiles++;
        }
    }
    int score = river_tiles / 4 + special;
    if (big_continent) {
        score += is_ocean_site ? -2 : 4;
    }
    if (radius_tile->bit & (BIT_UNK_4000 | BIT_UNK_40000000)) {
        score += 4;
    }
    int water = (ocean_count - big_continent) / 2 + deep_count / 2;
    score += is_ocean_site ? water : -water;
    score += (rich_terrain * 3 + 6) / 4;
    score += (fair_terrain + 1) / 3;
    if (GameDataStateStorage.fields_.turn_current_num_ > 150) {
        score += fungus_tiles / -3;
    }
    Map *tile = site_tile(x, y);
    int altitude = tile->climate & 0xE0;
    int is_ocean = (altitude < ALT_BIT_SHORE_LINE) ? 1 : 0;
    score -= rocky_tiles / 4;
    score -= crowding * ((is_ocean != is_ocean_site) ? 2 : 1);
    if (((tile->bit & BIT_FUNGUS) && altitude >= ALT_BIT_OCEAN_SHELF)
        || (tile->bit & BIT_MONOLITH)) {
        return 0;
    }
    if ((tile->val3 & 0xC0) > 0x40 && altitude >= ALT_BIT_SHORE_LINE) {
        return 0; // rocky land cannot hold a base
    }
    if (is_ocean != is_ocean_site && crowding >= 4) {
        return 0; // the wrong element, and no room for it either
    }
    if (score < 1) {
        return 1;
    }
    return (score > 15) ? 15 : score;
}

/*
Purpose: Analysis of the world map.
// ORIGINAL: 0x005C55C0 ?world_analysis@@YAXXZ 0x005C55C0-0x005C58B8
// LEVER: call_diff had this MORE by 3 calls: the inner loop's three `bit_set(x, y, ...)`
//            calls were real calls where the image writes bit_set's body out inline and
//            reuses ONE tile pointer for all three (the image keeps a running `Map*` in
//            ebx, bumped by sizeof(Map) once per x-loop iteration, instead of recomputing
//            map_loc(x, y) at each site) - replaced with `uint32_t *const tile_bit =
//            &map_loc(x, y)->bit;` hoisted once, then `*tile_bit`. Also fixed `i >= 32` /
//            `i == 32` to `i >= 8` / `i == 8`: the image's `cmp esi, 0x20` is the SAME
//            loop counter strength-reduced to a byte offset for RadiusOffsetX/Y addressing
//            (esi = 4*i), so the comparison the source made was against `i`, not against
//            that scaled induction variable - at `i < 20` the old `>= 32` could never be
//            true, so the region_radius/BIT_UNK_4000 branch was dead code. 0.454 -> 0.484
//            best-across-flags similarity; call_diff now agrees (0 vs 1 became 1 vs 1).
// TRIED (not attempted further): the remaining gap is the `imul 0x2c` vs
//            `lea`+SIB-scale-4 Map-stride plateau this file's other bodies already hit,
//            compounded by a prologue register-scheduling difference (ebx/edi assignment
//            order) at the very first divergence - both are the documented
//            register-allocation plateau for this family, not a source-shape defect.
// size      760 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FCB20
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_analysis() {
    for (uint32_t i = 0; i < MaxContinentNum; i++) {
        Continents[i].open_terrain = 0;
    }
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            uint32_t region = region_at(x, y);
            BOOL is_ocean_tile = is_ocean(x, y);
            if (!is_ocean_tile && climate_at(x, y) != RAINFALL_ARID 
                && rocky_at(x, y) < TERRAIN_BIT_ROCKY
                && (!(bit_at(x, y) & BIT_FUNGUS || altitude_at(x, y) < ALT_BIT_OCEAN_SHELF))) {
                Continents[region].open_terrain++;
            }
            uint32_t *const tile_bit = &map_loc(x, y)->bit; // bit_set, inlined: the image writes
            *tile_bit &= ~(BIT_UNK_40000000 | BIT_UNK_4000); // it out here, not a call
            if (!is_ocean_tile) {                                  // correctly for all bit_*
                int search_val = -1;
                uint32_t count_val1 = 0;
                uint32_t count_val2 = 0;
                for (uint32_t i = 0; i < 20; i++) {
                    int x_radius = xrange(x + RadiusOffsetX[i + 1]);
                    int y_radius = y + RadiusOffsetY[i + 1];
                    if (on_map(x_radius, y_radius)) {
                        BOOL is_ocean_radius = is_ocean(x_radius, y_radius);
                        if (i >= 8) {
                            if (is_ocean_radius != search_val || i == 8) {
                                search_val = is_ocean_radius;
                                count_val1++;
                            }
                            uint32_t region_radius;
                            if (y_radius > 4 && y_radius < ((int)MapLatitudeBounds - 4)
                                && (region_radius = region_at(x_radius, y_radius),
                                    region_radius < MaxRegionLandNum && region != region_radius
                                    && Continents[region].tile_count > 40
                                    && Continents[region_radius].tile_count > 40)) {
                                *tile_bit |= BIT_UNK_4000; // bit_set, inlined
                            }
                        } else {
                            if (is_ocean_radius != search_val) {
                                search_val = is_ocean_radius;
                                count_val2++;
                                if (count_val2 >= 4 && Continents[region].tile_count >= 80) {
                                    *tile_bit |= BIT_UNK_40000000; // bit_set, inlined
                                }
                            }
                        }
                    }
                }
                if (count_val1 < 4) {
                    *tile_bit &= ~BIT_UNK_40000000; // bit_set, inlined
                }
            }
        }
        do_all_non_input();
    }
    int area_index;
    for (area_index = 0; area_index < MapArea; area_index++) {
        map_tiles()[area_index].val2 &= 0xF; // clear map sites
    }
}

// veh_init (0x005C03D0) is still an archived body with no product definition,
// and world_fungus is its only caller in this file - the same handoff shape
// console.cpp carries at its head. Its forwarder is in pending_bodies.cpp.
int __cdecl veh_init(int unit_type, int faction_id, int x_coord, int y_coord);

/*
Purpose: Fill the fractal's raw 19x19 field with random values, copy it into
         the smooth field the sampler reads, and wrap both fields' edges.
// ORIGINAL: 0x005C1E20 ?init@Fractal@@QAEXXZ 0x005C1E20-0x005C1F12 FILE BYTE_EXACT
// symbol    ?init@Fractal@@QAEXXZ
// size      242 bytes
// prototype void (__thiscall ?init@Fractal@@QAEXXZ)(Fractal* this)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0064601D  (the C library rand)
// BUG IN THE ORIGINAL: the weighted pass in the middle sums each grid_[1]
// byte times 4 against eight RadiusOffsetX neighbours of grid_[1] and then
// throws the sum away - `mov esi, [ebp-4]` right after the inner loop
// discards it and reloads the row pointer before the store, which stores the
// byte it just read. Kept rather than eliminated: VC6 did not eliminate it
// either, so the dead arithmetic is what the shipped bytes do. Ghidra's own
// decompilation of this block drops it entirely (leaving a no-op pointer
// walk), which is the trap; the loop below is written from the raw bytes.
// LEVER: the same one that took ?sample to BYTE_EXACT - subscript the two
// fields as `grid_[1][row][col]` rather than walking a flat pointer. 37 of
// 86, against 7 of 86 for the walked-pointer spelling.
// TRIED: a walked row pointer for the first loop with a do-while row counter
// (the shape the image's own registers suggest) - 19 of 86, worse, because it
// reintroduces the named pointer locals the image does not spill. The image
// keeps `this` in EBX unspilled, recomputes each loop's pointer from it and
// reuses ONE stack slot (first the row counter, then the grid_[1] row
// pointer); at that earlier checkpoint this tree reserved three slots
// (`sub esp, 0xc` against `push ecx`) and swapped ESI/EDI. That stack-slot
// diagnosis is historical, not a requirement on the current implementation.
*/
void Fractal::init() {
    // grid_[1]: 18 rows of 18 columns of rand() % 16, then the two wraps.
    for (int row = 0; row < 18; ++row) {
        for (int col = 0; col < 18; ++col) {
            grid_[1][row][col] = static_cast<char>(rand() % 16);
        }
        grid_[1][row][18] = grid_[1][row][0];
    }
    for (int wrap_col = 0; wrap_col < 18; ++wrap_col) {
        grid_[1][18][wrap_col] = grid_[1][0][wrap_col];
    }

    // grid_[0][row][col] = grid_[1][row][col] for rows 0..16, cols 0..16.
    // The dead weighted sum is the image's; see the BUG note above. Its
    // neighbor load at 0x005C1EA7 includes the 19-byte row offset advanced
    // at 0x005C1ECA. The original unchecked reads can extend beyond grid_;
    // do not remove them, clamp them or infer extra object storage from them.
    for (int band_row = 0; band_row <= 16; ++band_row) {
        for (int band_col = 0; band_col <= 16; ++band_col) {
            int total = grid_[1][band_row][band_col] * 4;
            for (int neighbor = 1; neighbor <= 8; ++neighbor) {
                int step = RadiusOffsetX[neighbor];
                total += (&grid_[1][0][0])[20 + band_row * 19 + step * 20
                                          + band_col];
            }
            grid_[0][band_row][band_col] = grid_[1][band_row][band_col];
        }
    }

    // grid_[0] col 16 = col 0, then row 16 = row 0.
    for (int dup_row = 0; dup_row <= 16; ++dup_row) {
        grid_[0][dup_row][16] = grid_[0][dup_row][0];
    }
    for (int dup_col = 0; dup_col <= 16; ++dup_col) {
        grid_[0][16][dup_col] = grid_[0][0][dup_col];
    }
}

/*
Purpose: Blend the four grid_[0] taps around a 0..255 coordinate pair into one
         0..15 sample. Both world passes call it at two spacings to get two
         octaves of noise.
// ORIGINAL: 0x005C1F20 ?UNK1@Fractal@@QAEHHH@Z 0x005C1F20-0x005C1FB9 FILE BYTE_EXACT
// symbol    ?sample@Fractal@@QAEHHH@Z
// size      153 bytes
// prototype int (__thiscall ?UNK1@Fractal@@QAEHHH@Z)(Fractal* this, int, int)
// callers   3   call targets   0
// kind      game
// flags     (none)
// calls     (none)
*/
int Fractal::sample(int x_coord, int y_coord) {
    // Subtracted IN PLACE - the image loads each argument and `sub`s 0x80,
    // and reusing the parameter is what puts col_fraction in its stack slot.
    x_coord -= 0x80;
    y_coord -= 0x80;
    int row = (x_coord >> 8) & 0xf;
    int col = (y_coord >> 8) & 0xf;
    int col_fraction = (y_coord >> 3) & 0x1f;
    int row_fraction = (x_coord >> 3) & 0x1f;

    int top = grid_[0][row + 1][col] * row_fraction
              + grid_[0][row][col] * (32 - row_fraction);
    int bottom = grid_[0][row][col + 1] * (32 - row_fraction)
                 + grid_[0][row + 1][col + 1] * row_fraction;

    return (top * (32 - col_fraction) + bottom * col_fraction) >> 5;
}

/*
Purpose: Two octaves of sample() reduced to one 0..100 intensity - the value
         the fungus and erosion passes threshold against. Both callers spell
         the same arithmetic inline, which is how the name was read off them.
// ORIGINAL: 0x005C1FC0 ?UNK2@Fractal@@QAEHHHH@Z 0x005C1FC0-0x005C2020 FILE BYTE_EXACT
// symbol    ?level@Fractal@@QAEHHHH@Z
// size      96 bytes
// prototype int (__thiscall ?UNK2@Fractal@@QAEHHHH@Z)(Fractal* this, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1F20
// LEVER: both scaled coordinates live in registers ACROSS the first call -
// the image shifts the same two registers again for the second octave - so
// the octaves are spelled as `coarse_x` / `coarse_y` locals, and the *5 lands
// before the second call. 33 of 40.
// TRIED: `sample(x << 4, y << 4)` then `(x << 4) << shift` inline at the call
// sites, which re-derives the shift from the argument each time (10 of 40);
// the clamp as a nested ternary `value < 0 ? 0 : value > 100 ? 100 : value`
// against two early returns - identical 33 of 40, the zero arm is the only
// disagreement either way (the image jumps forward to it, this tree inlines
// it behind a `jns`), so the shape is block placement, not source form.
*/
int Fractal::level(int x_coord, int y_coord, int shift) {
    // Preserve the original scaled-coordinate reuse and arithmetic order.
    int coarse_x = x_coord << 4;
    int coarse_y = y_coord << 4;
    int coarse = sample(coarse_x, coarse_y) * 5;
    int fine = sample(coarse_x << shift, coarse_y << shift);
    int value = (coarse + fine * 2) * 7 >> 8;
    if (value >= 0) {
        if (value > 100) {
            return 100;
        }
        return value;
    }
    return 0;
}

/*
Purpose: Paint xenofungus over the map. A Fractal field is sampled at two
         spacings per tile to get a 0..100 intensity, the intensity bands and
         the native-life-forms setting decide whether the tile takes fungus,
         and tiles that pass get bit 0x20 set in their `bit` dword and every
         other flag in MapTileFlagClearMask stripped. On a 1-in-40 roll a
         fungus vehicle is seeded on the tile, which SMACX only (see
         ExpansionEnabled).
// ORIGINAL: 0x005C3440 ?world_fungus@@YAXXZ 0x005C3440-0x005C3678 FILE
// size      568 bytes
// prototype void (__cdecl ?world_fungus@@YAXXZ)()
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C03D0 0x005C1E20 0x005C1F20 0x005FCB20 0x0064601D
// LEVER: homed 2026-09-05 over src/fractal.h. The image's `sub esp, 0x2e4`
// is the Fractal object (722 bytes) plus the four int locals - no padding -
// which is what pinned sizeof(Fractal) to 0x2D2 and settled the class layout.
// Every fixed address in it is a tree name except MapTileFlagClearMask
// (0x006919EC, named on this pass - see map.cpp's definition).
// call_diff MORE is the known `MEASURED inline` do_all_non_input ceiling
// (AGENT_BRIEF: the image CALLS 0x005FCB20 once per row, this tree expands
// each site into check_net/do_net/do_non_input).
// TRIED: the y loop counter wants to live in EDI (the image's first
// callee-saved push) and lands in EBX here, which shifts every later
// register. Declaring `int y_coord = 0` BEFORE fungus_level with the rows as
// a do/while - the image's own statement order, and worth 7 of 183 -> 29 of
// 183 - is the best of four measured shapes: a for-loop inside the guard
// (7 of 183), an early `return` instead of the guard (7 of 183), and
// computing coarse_y/fine_y before the x-loop guard all scored worse. The
// remaining 154 disagreeing instructions are that one register assignment
// cascading; the call sequence, both sample spacings and every tile-field
// offset are the image's.
*/
void __cdecl world_fungus() {
    Fractal fractal;
    fractal.init();

    int y_coord = 0;
    int fungus_level = (static_cast<uint32_t>(MapNativeLifeForms) - 1u) * WorldBuilder.fungus;

    if (MapLatitudeBounds > 0) {
        do {
            // Both octaves' scaled coordinates are loop-invariant per row.
            int coarse_y = y_coord << 4;
            int fine_y = coarse_y << 4;

            for (int x_coord = y_coord & 1; x_coord < MapLongitudeBounds;
                 x_coord += 2) {
                int coarse_x = x_coord << 4;
                int level = fractal.sample(coarse_x, coarse_y) * 5
                            + fractal.sample(coarse_x << 4, fine_y) * 2;
                level = level * 7 >> 8;
                if (level < 0) {
                    level = 0;
                } else if (level > 100) {
                    level = 100;
                }

                Map *tile = map_tiles() + MapLongitude * y_coord
                            + (x_coord >> 1);
                if ((tile->bit2 & 0x80000002) == 2
                    && (tile->bit2 & 0xff000000) < 0x9000000) {
                    continue;
                }

                if (level < 20 - fungus_level
                    || level > fungus_level * 2 + 25) {
                    if (level < 40 - fungus_level
                        || level > fungus_level * 2 + 45) {
                        if (level < 60 - fungus_level
                            || level > fungus_level * 2 + 65) {
                            // Only the first and last rows fall through here.
                            if (y_coord != 0
                                && y_coord != MapLatitudeBounds - 1) {
                                continue;
                            }
                            if (rand() % 2 == 0) {
                                continue;
                            }
                        }
                    }
                }

                if ((tile->val3 & ~0x1F) >= 0x80
                    && (tile->climate & 0xE0) >= 0x60) {
                    continue;
                }

                tile->bit |= 0x20;
                tile->bit &= ~MapTileFlagClearMask;

                if (rand() % 40 == 0 && (tile->climate & 0xE0) >= 0x60
                    && ExpansionEnabled != 0) {
                    veh_init(0x13, 0, x_coord, y_coord);
                }
            }

            do_all_non_input();
            y_coord++;
        } while (y_coord < MapLatitudeBounds);
    }
}

/*
Purpose: Re-cut every land tile's altitude bands from the shore-to-peak
         threshold table, then walk the map six more times spreading each
         natural altitude level one step outwards; refresh the visible map
         windows as it goes when the omniscient view is on and the
         regeneration is allowed to redraw.
// ORIGINAL: 0x005C3F70 ?world_shorelines@@YAXXZ 0x005C3F70-0x005C40E3 FILE
// size      371 bytes
// prototype void (__cdecl ?world_shorelines@@YAXXZ)()
// callers   3   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046AF40 0x0046B190 0x0046FD90 0x00591290 0x00591AD0 0x005C2020 0x005FCB20
// LEVER: homed from src/recovered/units/005c3f70.cpp on this pass. Its five
//   fixed-address bindings are the tree's names - MapLatitudeBounds,
//   MapLongitudeBounds, MapLongitude (the tile row stride), MapSeaLevel,
//   AltNatural (the threshold table the closing walk steps down), the
//   LockMapTable slot for the tile records themselves - so nothing is bound
//   twice. The byte read of GameState's omniscient bit is spelled as the
//   dword test the rest of this TU uses.
// TRIED: byte-exactness is capped by the same ceiling AGENT_BRIEF records for
//   Path::continent - the image CALLS 0x005FCB20 (do_all_non_input) twice and
//   this tree's `MEASURED inline` in temp.h expands each site into
//   check_net/do_net/do_non_input, 14 calls against the image's 8 and 167
//   instructions against 131 (call_diff MORE). The call sequence itself is
//   the image's: alt_set, temp_set, draw_tile, do_all_non_input,
//   world_alt_set, do_all_non_input, MapWin::clear_terrain, draw_map.
Status: homed, not yet byte-exact.
*/
void __cdecl world_shorelines() {
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            int tile_index = MapLongitude * y + (x >> 1);
            int altitude = 0;
            Map *tile = &map_tiles()[tile_index];
            int depth = tile->contour - MapSeaLevel;

            int natural_index = 6;
            do {
                if (depth >= static_cast<int32_t>(AltNatural[natural_index])) {
                    altitude = natural_index;
                    break;
                }
                natural_index--;
            } while (natural_index >= 0);

            alt_set(x, y, altitude);

            if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0
                && WorldClimateRedrawRequest != 0) {
                if (altitude >= 3) {
                    int temperature = altitude - 3;
                    if (temperature < 0) {
                        temperature = 0;
                    } else if (temperature > 2) {
                        temperature = 2;
                    }
                    temp_set(x, y, temperature);
                }
                draw_tile(x, y, 2);
            }
        }
        do_all_non_input();
    }

    for (int natural = 6; natural >= 2; natural--) {
        for (int y = 0; y < MapLatitudeBounds; y++) {
            for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
                int tile_index = MapLongitude * y + (x >> 1);
                Map *tile = &map_tiles()[tile_index];
                int rocky = tile->climate >> 5;
                if (rocky == natural) {
                    world_alt_set(x, y, rocky, 1);
                }
            }
            do_all_non_input();
        }
    }

    if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0
        && WorldClimateRedrawRequest != 0) {
        MapWinTable[0]->clear_terrain();
        draw_map(1);
    }
}

// Pinned NUL-terminated log text at 0x00691E2C and 0x00691E4C. The
// following alignment bytes are not part of either string's source extent.
// RECOVERY_OBJECT: v2 0x00691E2C 0x00691E4A _WorldClimateStartLog 0x005C5A30
static char WorldClimateStartLog[] = "-- ADJUSTING WORLD CLIMATE --";
// RECOVERY_OBJECT: v2 0x00691E4C 0x00691E69 _WorldClimateCompleteLog 0x005C5A30
static char WorldClimateCompleteLog[] = "-- WORLD CLIMATE COMPLETE --";

/*
Purpose: Regenerate the world's climate: shorelines, temperature, rivers,
         rainfall, continents, then analysis; reset territory unless told
         not to; reseed the random generator from the clock; and clear
         terrain on every live map window (plus the world overview window)
         unless told not to.
// ORIGINAL: 0x005C5A30 ?world_climate@@YAXXZ 0x005C5A30-0x005C5ADB BYTE_EXACT
// size      171 bytes
// prototype void (__cdecl ?world_climate@@YAXXZ)()
// callers   18   call targets   12
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046FD90 0x004C45E0 0x0051E760 0x00523DD0 0x0059C790 0x005C38B0
//           0x005C3F70 0x005C4170 0x005C4470 0x005C55C0 0x006257E0 0x006262F0
// indirect  0x005C5A7B
Return Value: n/a
Status: Complete
*/
void __cdecl world_climate() {
    do_checksums(2);
    log_say(WorldClimateStartLog, 0, 0, 0);
    world_shorelines();
    world_temperature();
    world_rivers();
    world_rainfall();
    Paths.continents();
    world_analysis();
    if (WorldClimateSkipTerritoryReset == 0) {
        reset_territory();
    }

    random_reseed(timeGetTime());

    if (WorldClimateSkipTerrainClear == 0) {
        for (int slot = 0; slot < 8; ++slot) {
            MapWin *const window = MapWinTable[slot];
            if (window->view_active_ != 0) {
                window->clear_terrain();
            }
        }
        WorldClimateWorldWin.clear_terrain();
    }

    log_say(WorldClimateCompleteLog, 0, 0, 0);
    do_checksums(3);
}

/*
Purpose: Set the default altitude details for the specified tile.
// ORIGINAL: 0x005C58C0 ?world_alt_put_detail@@YAXHH@Z 0x005C58C0-0x005C58DB BYTE_EXACT
// body      src/world.cpp
// size      27 bytes
// prototype void (__cdecl ?world_alt_put_detail@@YAXHH@Z)(int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591260
Return Value: n/a
Status: Complete
*/
void __cdecl world_alt_put_detail(int x, int y) {
    alt_put_detail(x, y, AltNatural[3]);
}

/*
Purpose: Set up the world polar caps.
// ORIGINAL: 0x005C58E0 ?world_polar_caps@@YAXXZ 0x005C58E0-0x005C5A2F BYTE_EXACT
// BUG IN THE RECOVERY: the second pole write was `x - 1` where the image
//            computes `x + 1` (`lea eax, [esi + 1]` at 0x5C590A) - the image
//            pairs (x, 0) with (x+1, MapLatitudeBounds-1), so x=0 wrote
//            (-1, MapLatitudeBounds-1), wrapping to the far column, and every
//            pair was one tile out. `x + 1` is also what makes this
//            BYTE_EXACT.
// LEVER: three call-shape fixes, 1/128 -> BYTE_EXACT 128/128. (1) A prior
//            workaround sent the six pole writes through a non-inline
//            `alt_put_detail_call` forwarder to force the original direct
//            calls to alt_put_detail (0x00591260); call_diff had named the
//            inlined `world_alt_put_detail` a WRONG CALLEE. The canonical
//            altitude owner now lives in another TU, so these real calls
//            retain the direct boundary. (2) The four
//            random columns are `rnd(MapLongitudeBounds / 2, NULL)` written
//            out as its own body (guard clause + `rand() % seed`) - the
//            image calls rand() directly, never rnd(), and it computes the
//            halving from MapLongitudeBounds itself rather than reading the
//            pre-halved MapLongitude. (3) The second loop's bound is the
//            expression, not a local: `uint32_t bounds =
//            MapLongitudeBounds / 16` lets VC6 hoist the bound and count
//            down (`dec edi / jne`), where the image re-evaluates it every
//            iteration and counts up (`inc edi / cmp / jl`). Loop variable
//            is `int`, not `uint8_t` (the image's `x` lives in esi as a
//            dword), and the AltNatural[3] argument is not cast to
//            uint8_t - the image pushes the full dword.
// size      335 bytes
// prototype 
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00591260 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_polar_caps() {
    // world_alt_put_detail(x, y) written out at every site: the image calls
    // alt_put_detail (0x00591260) directly from all six - historical
    // `call_diff` called the inlined `world_alt_put_detail` a WRONG CALLEE -
    // and calls rand() rather than rnd(). The helper's canonical body is in
    // map_altitude.cpp, outside this caller TU.
    for (int x = 0; x < MapLongitudeBounds; x += 2) {
        alt_put_detail(x, 0, AltNatural[3]);
        alt_put_detail(x + 1, MapLatitudeBounds - 1, AltNatural[3]);
    }
    for (int i = 0; i < MapLongitudeBounds / 16; i++) {
        // rnd(MapLongitude, NULL)'s own body (0x00579770): the image has no
        // call to rnd here, only to rand().
        int seed = MapLongitudeBounds / 2;
        int x_rand;
        if (seed - 1 <= 0) {
            x_rand = 0;
        } else {
            x_rand = rand() % seed;
        }
        alt_put_detail(x_rand * 2, 0, AltNatural[3]);
        seed = MapLongitudeBounds / 2;
        if (seed - 1 <= 0) {
            x_rand = 0;
        } else {
            x_rand = rand() % seed;
        }
        alt_put_detail(x_rand * 2 + 1, 1, AltNatural[3]);
        seed = MapLongitudeBounds / 2;
        if (seed - 1 <= 0) {
            x_rand = 0;
        } else {
            x_rand = rand() % seed;
        }
        alt_put_detail(x_rand * 2 + 1, MapLatitudeBounds - 1, AltNatural[3]);
        seed = MapLongitudeBounds / 2;
        if (seed - 1 <= 0) {
            x_rand = 0;
        } else {
            x_rand = rand() % seed;
        }
        alt_put_detail(x_rand * 2, MapLatitudeBounds - 2, AltNatural[3]);
    }
}

/*
Purpose: Set up the world contours.
// ORIGINAL: 0x005C5AE0 ?world_linearize_contours@@YAXXZ 0x005C5AE0-0x005C5BC3
// RECOVERY_BINDING: v1 {"reason":"Linearize tile contours from the default input intervals into the real contour target table, preserving the original signed division and final table publication.","evidence":["Pinned5C5B35/4F/58 loadAltContour at68FB20/24/30;5C5B3E/62 loadAltNaturalDefault at68FAF4/68FAF8;CMP6/JGE selects the +4 target.","Pinned5C5B73 IMUL retains low32bits, followed by CDQ/IDIV at5C5B78/79 and wrappedADD at5C5B7B;5C5B80 passes the full result to the byte-writing alt_put_detail owner.","Pinned5C5B98 calls do_all_non_input after each row;5C5BB1..5C5BBB copies11dwords fromAltContour68FB20 toAltNatural68FB4C only after all rows."]}
// LEVER: BUG IN THIS RECOVERY (not the original): `alt_nat +
//        (alt_nat >= ALT_3_LEVELS_ABOVE_SEA) ? 4 : 1` - `+` binds tighter
//        than `?:`, so this parsed as `(alt_nat + (alt_nat >= ...)) ? 4 :
//        1`, collapsing the array index to the plain constant 4 or 1. The
//        image indexes `ElevDetail` at `[eax*4 + 0x68fb24]` /
//        `[eax*4 + 0x68fb30]` - i.e. `alt_nat + 1` or `alt_nat + 4`,
//        register-relative on `alt_nat`. Parenthesizing the ternary
//        (`alt_nat + ((alt_nat >= ALT_3_LEVELS_ABOVE_SEA) ? 4 : 1)`) fixes
//        the semantics; 0.690 -> 0.705 similar (best flags /O2 /Ob0).
// TRIED: not chased past this MISMATCH plateau - remaining gap starts
//            in the prologue frame-size (image reserves 2 stack dwords for
//            the divide's intermediates, this tree's compile picks a
//            different spill count).
// size      227 bytes
// prototype
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591260 0x005918A0 0x005FCB20
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_linearize_contours() {
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            const int band = alt_natural(x, y);
            const uint32_t detail = static_cast<uint32_t>(alt_detail_at(x, y));
            const uint32_t contour = AltContour[band];
            const uint32_t input_floor = AltNaturalDefault[band];
            const uint32_t target = AltContour[band
                + ((band >= ALT_3_LEVELS_ABOVE_SEA) ? 4 : 1)];
            const int32_t denominator = static_cast<int32_t>(
                AltNaturalDefault[band + 1] - input_floor);
            // IMUL wraps before CDQ/IDIV interprets the numerator as signed.
            // Preserve the original divide faults; there is no zero guard.
            const int32_t numerator = static_cast<int32_t>(
                (target - contour) * (detail - input_floor));
            const uint32_t result = contour
                + static_cast<uint32_t>(numerator / denominator);
            alt_put_detail(x, y, static_cast<int32_t>(result));
        }
        do_all_non_input();
    }
    // `memcpy`, and the ARRAYS, not their addresses. The image ends with
    // `mov ecx, 0xb; mov esi, 0x68fb20; mov edi, 0x68fb4c; rep movsd` - the
    // intrinsic, copying AltContour over AltNatural. `memcpy_s` emits a call
    // with four arguments, and `&AltNatural` wrote over the POINTER rather
    // than the array it names.
    memcpy(AltNatural, AltContour, sizeof(AltContour));
}

/*
Purpose: Determine if the specified tile is near a landmark.
// ORIGINAL: 0x005C5BD0 ?near_landmark@@YAHHH@Z 0x005C5BD0-0x005C5C61
// size      145 bytes
// prototype BOOL (__cdecl ?near_landmark@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: call shape already matches - `osmx calls` shows 0 calls, same as the image,
//            so xrange/on_map/code_at are already inlined here. Best flag set still only
//            reaches 17/56 agreeing instructions; the divergence starts at the prologue's
//            register assignment (ebx loaded from MapIsFlat before the frame push order this
//            tree emits), the stack-frame/register-allocation plateau noted for this family.
// LEVER (2026-08-21): moved to map.h as `MEASURED inline`. world_unity (0x005C7750) and
//            world_geothermal (0x005C83B0) call_diff'd off by exactly one call each, and
//            in both cases it was this: a real call to near_landmark where the image has
//            it inlined. world_borehole (0x005C7020) already inlined it either way, so
//            this is additive there, and `osmx check` stayed at 0 REGRESSED.
Return Value: Is the tile near a landmark? true/false
Status: Complete
*/
// Defined here so this owner can inline it; other TUs use the public declaration.

BOOL __cdecl near_landmark(int x, int y) {
    for (int i = 0; i < RadiusRange[8]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && code_at(x_radius, y_radius)) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Setup the 'Garland Crater' landmark.
// ORIGINAL: 0x005C5C70 ?world_crater@@YAXHH@Z 0x005C5C70-0x005C5EEF
// size      639 bytes
// prototype void (__cdecl ?world_crater@@YAXHH@Z)(int xCoord, int yCoord)
// callers   4   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591AD0 0x00591BC0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: two fixes, 0.703 -> 0.779 similar (28/221 raw). (1) `rnd(bounds,
//        NULL)` WRITTEN OUT as rnd's own body (0x00579770's ternary) at both
//        call sites - the image never calls rnd(), only rand() - matching
//        the image's call count. (2) rnd's ternary compiles with INVERTED
//        branch polarity from the image's; an `if (bounds - 1 <= 0) { x = 0;
//        } else { x = rand() % bounds; }` guard clause (zero branch inline,
//        compute branch out-of-line via `jg`) reproduces the image's
//        fall-through instead. Also: `int lon_bounds = MapLongitudeBounds;`
//        declared once, used in both the second guard's condition and its
//        `rand() % lon_bounds` - matches the image reusing one register for
//        both, where the bare global re-read three times.
// TRIED: same xrange/on_map two-induction-variable loop family already
//            documented across this file (bonus_at, goody_at, base_on_sea,
//            world_borehole) - the prologue's on_map inline and the loop's
//            register scheduling (which global gets reloaded early during a
//            call's latency slot) do not follow from source shape. Not
//            chased past this MISMATCH plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_crater(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    world_alt_set(x, y, ALT_OCEAN_TRENCH, true);
    world_alt_set(x, y, ALT_3_LEVELS_ABOVE_SEA, true);
    world_alt_set(x, y, ALT_SHORE_LINE, true);
    world_alt_set(x, y, ALT_OCEAN_SHELF, true);
    world_alt_set(x, y, ALT_SHORE_LINE, true);
    uint32_t temperature = temp_at(x, y);
    for (int i = 0; i < RadiusRange[3]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            bit_set(x_radius, y_radius, BIT_RIVERBED, false);
            bit2_set(x_radius, y_radius, BIT2_CRATER, true);
            code_set(x_radius, y_radius, i);
            if (i < 21) {
                rocky_set(x_radius, y_radius, ROCKINESS_ROLLING);
                temp_set(x_radius, y_radius, (uint8_t)temperature);
            }
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_CRATER].name));
}

/*
Purpose: Setup the 'Monsoon Jungle' landmark.
// ORIGINAL: 0x005C5EF0 ?world_monsoon@@YAXHH@Z 0x005C5EF0-0x005C6200
// size      784 bytes
// prototype void (__cdecl ?world_monsoon@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E49D0 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x005C4470 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - `rnd(MapLongitudeBounds,
//        NULL)` written out as an `int lon_bounds = MapLongitudeBounds; if
//        (lon_bounds - 1 <= 0) { x_seed = 0; } else { x_seed = rand() %
//        lon_bounds; }` guard clause, matching the image's call count (no
//        real call to rnd()) and fall-through polarity. Best flag set (/O2
//        /Gy /GR- /Oy- /GX) moved 0.398 -> 0.455 similar.
// TRIED: same xrange/on_map two-induction-variable loop family plateau as
//            world_crater - the on_map prologue and the loop's register
//            scheduling do not follow from source shape. Not chased past
//            this MISMATCH plateau (784-byte body, 260 image instructions).
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_monsoon(int x, int y) {
    world_rainfall();
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            uint32_t land_count;
            do {
                y = MapLatitudeBounds / 2 + rand() % 4 - 2;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                land_count = 0;
                for (int i = 0; i < RadiusRange[5]; i++) {
                    int x_radius = xrange(x + RadiusOffsetX[i]);
                    int y_radius = y + RadiusOffsetY[i];
                    if (on_map(x_radius, y_radius) && !is_ocean(x_radius, y_radius)) {
                        land_count++;
                    }
                }
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y) || !is_coast(x, y, true) || land_count < 40
                || climate_at(x, y) <= RAINFALL_MOIST);
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[5]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (abs(RadiusOffsetY[i]) <= 8 &&  on_map(x_radius, y_radius)) {
            if (i < 21 && is_ocean(x_radius, y_radius)) {
                world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            }
            bit2_set(x_radius, y_radius, BIT2_JUNGLE, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_JUNGLE].name));
}

/*
Purpose: Setup the 'New Sargasso' landmark.
// ORIGINAL: 0x005C6200 ?world_sargasso@@YAXHH@Z 0x005C6200-0x005C6498
// size      664 bytes
// prototype void (__cdecl ?world_sargasso@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 1/220 -> 26/220, 0.591 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_sargasso(int x, int y) {
    uint32_t loc_attempts = 0;
    BOOL has_skipped = false;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (!is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[4]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (is_ocean(x_radius, y_radius) || i < 21)) {
            world_alt_set(x_radius, y_radius, ALT_OCEAN_SHELF, true);
            bit2_set(x_radius, y_radius, BIT2_SARGASSO, true);
            code_set(x_radius, y_radius, i);
            if (i <= 4) {
                bit_set(x_radius, y_radius, BIT_SUPPLY_POD, true);
            }
            if ((has_skipped || i < RadiusRange[2] || rand() % 24) 
                && (i < RadiusRange[3] || rand() % 3)) {
                bit_set(x_radius, y_radius, BIT_FUNGUS, true);
            } else {
                has_skipped = true;
            }
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_SARGASSO].name));
}

/*
Purpose: Setup 'The Ruins' landmark.
// ORIGINAL: 0x005C64A0 ?world_ruin@@YAXHH@Z 0x005C64A0-0x005C6734
// size      660 bytes
// prototype void (__cdecl ?world_ruin@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 1/221 -> 8/221, 0.422 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_ruin(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[2]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            if (i < RadiusRange[2] || !is_ocean(x_radius, y_radius)) { // is_ocean check pointless?
                world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            }
            if (is_ocean(x_radius, y_radius)) {
                if (i && i < 9) {
                    bit_set(x_radius, y_radius, BIT_MONOLITH, true); // in ocean tiles?
                }
            } else {
                bit_set(x_radius, y_radius, BIT_FUNGUS | BIT_SUPPLY_REMOVE, true);
                if (!i) {
                    bit_set(x_radius, y_radius, BIT_FUNGUS, false);
                } else if (i < 9) {
                    bit_set(x_radius, y_radius, BIT_MONOLITH, true);
                }
            }
            bit2_set(x_radius, y_radius, BIT2_RUINS, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_RUINS].name));
}

/*
Purpose: Setup the 'Great Dunes' landmark.
// ORIGINAL: 0x005C6740 ?world_dune@@YAXHH@Z 0x005C6740-0x005C69DB
// size      667 bytes
// prototype void (__cdecl ?world_dune@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x005C4470 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70), plus its own
//        `half_vert_bounds` local already matched rnd's `bounds`
//        parameter - written out as an `int dune_bounds = half_vert_bounds;`
//        guard clause (Y) and the usual `lon_bounds` guard clause (X). Best
//        flag set (/O2 /Gy /GR- /Oy- /GX) moved 5/228 -> 9/228, 0.559
//        similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater, plus world_rainfall()'s own call at
//            the top. Not chased past this MISMATCH plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_dune(int x, int y) {
    world_rainfall();
    uint32_t loc_attempts = 0;
    uint32_t half_vert_bounds = MapLatitudeBounds / 2;
    if (!on_map(x, y)) {
        do {
            do {
                int dune_bounds = half_vert_bounds;
                int rnd_y;
                if (dune_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % dune_bounds;
                }
                y = rnd_y + half_vert_bounds - MapLatitudeBounds / 4;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y) || climate_at(x, y) != RAINFALL_ARID);
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[2]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (!is_ocean(x_radius, y_radius) || i < 9 || rand() % 3)) {
            if (i == 2 || i == 7) {
                world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            } else {
                world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            }
            bit_set(x_radius, y_radius, BIT_FUNGUS, false);
            bit2_set(x_radius, y_radius, BIT2_DUNES, true);
            rocky_set(x_radius, y_radius, ROCKINESS_FLAT);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_DUNES].name));
}

/*
Purpose: Setup the 'Uranium Flats' landmark.
// ORIGINAL: 0x005C69E0 ?world_diamond@@YAXHH@Z 0x005C69E0-0x005C6C31
// size      593 bytes
// prototype void (__cdecl ?world_diamond@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 7/200 -> 14/200, 0.887 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_diamond(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (uint32_t i = 0; i < 21; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            if (i < 9 || !is_ocean(x_radius, y_radius) || rand() % 3) {
                world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
                bit2_set(x_radius, y_radius, BIT2_URANIUM, true);
                rocky_set(x_radius, y_radius, ROCKINESS_FLAT);
                bit_set(x_radius, y_radius, BIT_FUNGUS, false);
                code_set(x_radius, y_radius, i);
            }
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_URANIUM].name));
}

/*
Purpose: Setup the 'Freshwater Sea' landmark.
// ORIGINAL: 0x005C6C40 ?world_fresh@@YAXHH@Z 0x005C6C40-0x005C6DA4
// size      356 bytes
// prototype void (__cdecl ?world_fresh@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591DB0 0x00592600 0x006169A0
// LEVER: `bit2_set(x_it, y_it, BIT2_FRESH, true)`, not `LM_FRESH` - the image
//        pushes 0x80 (BIT2_FRESH), not 7 (LM_FRESH's enum index), same
//        LM_ vs BIT2_ mixup already fixed at world_geothermal (0x005C83B0).
//        Confirmed the constant now matches byte-for-byte (`push 0x80` on
//        both sides at the bit2_set call site) but the overall score does
//        not move (still 4/125, 0.667 similar, best flag set /O2 /Gy /GR-
//        /Oy- /GX) - the earlier structural mismatch (see TRIED) already
//        dominates the comparison.
// TRIED: the image places the `bit2_set`/`x_search` update block AFTER a
//            second nested branch this tree's straight if/else-if puts it
//            before - looks like the compiler restructured the two mutually
//            exclusive branches into a different instruction order than a
//            literal transcription of the nested if/else-if produces. Not
//            chased further; same xrange/on_map-loop family register-
//            scheduling plateau as this file's other worldbuilder bodies
//            besides.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_fresh(int x, int y) {
    uint32_t region;
    if (on_map(x, y)) {
        region = region_at(x, y);
        if (!is_ocean(x, y)) {
            return;
        }
    } else {
        int tile_search = 0;
        int region_search = -1;
        for (uint32_t i = MaxLandmarkNum; i < MaxContinentNum; i++) {
            int tile_count = Continents[i].tile_count;
            if (tile_count >= 3 && tile_count <= 32 && tile_count >= tile_search) {
                tile_search = tile_count;
                region_search = i;
            }
        }
        if (region_search < 0) {
            return;
        }
        region = region_search;
    }
    int x_search = -1;
    BOOL has_set_landmark = false;
    for (int y_it = MapLatitudeBounds - 1; y_it >= 0 ; y_it--) {
        for (int x_it = y_it & 1; x_it < MapLongitudeBounds; x_it += 2) {
            if (region_at(x_it, y_it) == region) {
                bit2_set(x_it, y_it, BIT2_FRESH, true);
                if (x_search < 0) {
                    x_search = x_it;
                }
            } else if(!has_set_landmark && x_search >= 0) {
                int x_fresh = (x_search + x_it - 2) / 2;
                new_landmark(((x_fresh ^ y_it) & 1) ^ x_fresh, y_it,
                    StringTable.get(Natural[LM_FRESH].name));
                has_set_landmark = true;
            }
        }
    }
}

/*
Purpose: Setup the 'Mount Planet' landmark.
// ORIGINAL: 0x005C6DB0 ?world_volcano@@YAXHHH@Z 0x005C6DB0-0x005C7016
// size      614 bytes
// prototype void (__cdecl ?world_volcano@@YAXHHH@Z)(int xCoord, int yCoord, int)
// callers   4   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591BC0 0x00591D30 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds, NULL)`
//        calls written out as guard clauses (`if (bounds - 1 <= 0) { x = 0;
//        } else { x = rand() % bounds; }`), matching the image's call count
//        and fall-through polarity. Needs /Oy- (the marker's own `frame`
//        flag) for the comparison to even mean anything - best flag set
//        (/O2 /Gy /GR- /Oy- /GX) is 20/206, 0.633 similar; without /Oy- the
//        fuzzy similarity score is misleadingly higher (0.868) because the
//        frame-omitted form shares more MNEMONICS despite matching 0 bytes
//        from instruction 0.
// TRIED: same xrange/on_map two-induction-variable loop family plateau as
//            world_crater/world_monsoon. Not chased past this MISMATCH
//            plateau (614-byte body, 206 image instructions).
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_volcano(int x, int y, BOOL is_not_landmark) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    world_alt_set(x, y, ALT_BIT_OCEAN_TRENCH, true); // purpose??
    world_alt_set(x, y, ALT_3_LEVELS_ABOVE_SEA, true);
    for (int i = 0; i < RadiusRange[1]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            bit2_set(x_radius, y_radius, BIT2_VOLCANO, true);
            code_set(x_radius, y_radius, i);
            uint32_t bit = bit_at(x_radius, y_radius) & ~(BIT_SUPPLY_POD | BIT_THERMAL_BORE
                | BIT_ECH_MIRROR | BIT_CONDENSER | BIT_SOIL_ENRICHER | BIT_FARM | BIT_RIVERBED
                | BIT_SOLAR_TIDAL | BIT_FUNGUS | BIT_MINE | BIT_MAGTUBE | BIT_ROAD);
            if (!i) {
                bit &= ~(BIT_UNK_4000000 | BIT_UNK_8000000);
            }
            bit |= BIT_SUPPLY_REMOVE;
            bit_put(x_radius, y_radius, bit);
            rocky_set(x_radius, y_radius, ROCKINESS_ROCKY);
        }
    }
    GameDataStateStorage.fields_.mount_planet_x_ = x;
    GameDataStateStorage.fields_.mount_planet_y_ = y;
    if (!is_not_landmark) {
        new_landmark(x, y, StringTable.get(Natural[LM_VOLCANO].name));
    }
}

/*
Purpose: Setup the 'Borehole Cluster' landmark. Added to SMAC in 3.0 patch.
// ORIGINAL: 0x005C7020 ?world_borehole@@YAXHH@Z 0x005C7020-0x005C753B
// size      1307 bytes
// prototype void (__cdecl ?world_borehole@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004712A0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// indirect  0x005C7145
// TRIED (2026-08-21): call_diff says 12 calls here vs the image's 14 (missing
//            on_map/bit_set/bit2_set/code_set instances). on_map, bit_set, bit2_set and
//            code_set are all `MEASURED inline` in map.h already - each is called from
//            dozens of sites across map.cpp, and the image itself keeps SOME of those
//            calls real while inlining others per call site (on_map has `callers 1` on
//            its OWN listing even though this function alone names it 5 times in source).
//            Un-inlining any of them here would touch every OTHER caller map.h shares
//            them with; `osmx check` has no way to hold those fixed while this one
//            function moves. Tried un-inlining near_landmark (its ORIGINAL marker, right
//            above world_crater below) on the same theory - zero effect, reverted there.
//            Left as MISMATCH; the register-allocation plateau this file already notes
//            for xrange/on_map loop bodies applies here too once the call shape is fixed.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_borehole(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                y = rnd(MapLatitudeBounds - 16, NULL) + 8;
                int x_seed = rnd(MapLongitudeBounds, NULL);
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    uint32_t seed = timeGetTime();
    int val1 = 8;
    int val2 = ((seed % 8) / 3) + 5;
    int val3 = 3 - ((seed % 8) % 3);
    uint32_t val4 = (seed / 8) % 4;
    if (val4 & 2) {
        val2--;
        val3--;
        if (val4 & 1) {
            val2++;
            val1 = 8;
            val3++;
        } else {
            val2--;
            val1 = 6;
            val3--;
        }
        val1 = (val1 + 8) % 8 + 1;
        val2 = (val2 + 8) % 8 + 1;
        val3 = (val3 + 8) % 8 + 1;
    }
    for (int i = 0; i < RadiusRange[1]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            bit_set(x_radius, y_radius, BIT_SUPPLY_REMOVE, true);
            if (i == val1 || i == val2 || i == val3) {
                bit_set(x_radius, y_radius, BIT_THERMAL_BORE, true);
                bit2_set(x_radius, y_radius, BIT2_BOREHOLE, true);
                code_set(x_radius, y_radius, i);
            }
        }
    }
    int x_offset_val1 = xrange(x + RadiusOffsetX[val1]); // removed from loop
    int y_offset_val1 = y + RadiusOffsetY[val1]; // removed from loop
    for (i = 0; i < 8; i++) {
        int x_radius = xrange(x_offset_val1 + RadiusOffsetX[i + 1]);
        int y_radius = y_offset_val1 + RadiusOffsetY[i + 1];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
        }
    }
    int x_offset_val2 = xrange(x + RadiusOffsetX[val2]); // removed from loop
    int y_offset_val2 = y + RadiusOffsetY[val2]; // removed from loop
    for (i = 0; i < 8; i++) {
        int x_radius = xrange(x_offset_val2 + RadiusOffsetX[i + 1]);
        int y_radius = y_offset_val2 + RadiusOffsetY[i + 1];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
        }
    }
    int x_offset_val3 = xrange(x + RadiusOffsetX[val3]); // removed from loop
    int y_offset_val3 = y + RadiusOffsetY[val3]; // removed from loop
    for (i = 0; i < 8; i++) {
        int x_radius = xrange(x_offset_val3 + RadiusOffsetX[i + 1]);
        int y_radius = y_offset_val3 + RadiusOffsetY[i + 1];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
        }
    }
    bit2_set(x, y, BIT2_BOREHOLE, true);
    new_landmark(x, y, StringTable.get(Natural[LM_BOREHOLE].name));
}

/*
Purpose: Setup 'The Manifold Nexus' landmark. Added to SMAC in 4.0 patch.
// ORIGINAL: 0x005C7540 ?world_temple@@YAXHH@Z 0x005C7540-0x005C7749
// size      521 bytes
// prototype void (__cdecl ?world_temple@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 11/175 -> 12/175, 0.864 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_temple(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[1]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            bit_set(x_radius, y_radius, BIT_SUPPLY_REMOVE, true);
            bit2_set(x_radius, y_radius, BIT2_NEXUS, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_NEXUS].name));
}

/*
Purpose: Setup the 'Unity Wreckage' landmark (SMACX only).
// ORIGINAL: 0x005C7750 ?world_unity@@YAXHH@Z 0x005C7750-0x005C7A75
// size      805 bytes
// prototype void (__cdecl ?world_unity@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls (inside the `if (ExpansionEnabled)` guard) written
//        out as guard clauses matching the image's call count and
//        fall-through polarity. Best flag set (/O2 /Gy /GR- /Oy- /GX)
//        moved 3/279 -> 41/279 raw agreeing (0.216 similar - this
//        function's `x--/y--` then `x+=2/y+=2` then `x--/y--` re-walk of
//        the same radius loop three times gives the register allocator
//        much more to disagree about than its siblings).
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater, repeated three times over. Not
//            chased past this MISMATCH plateau (805-byte body, 279 image
//            instructions).
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_unity(int x, int y) {
    if (ExpansionEnabled) {
        uint32_t loc_attempts = 0;
        if (!on_map(x, y)) {
            do {
                do {
                    int lat_bounds = MapLatitudeBounds - 16;
                    int rnd_y;
                    if (lat_bounds - 1 <= 0) {
                        rnd_y = 0;
                    } else {
                        rnd_y = rand() % lat_bounds;
                    }
                    y = rnd_y + 8;
                    int lon_bounds = MapLongitudeBounds;
                    int x_seed;
                    if (lon_bounds - 1 <= 0) {
                        x_seed = 0;
                    } else {
                        x_seed = rand() % lon_bounds;
                    }
                    x = ((x_seed ^ y) & 1) ^ x_seed;
                    if (++loc_attempts >= 1000) {
                        return;
                    }
                } while (is_ocean(x, y));
            } while (near_landmark(x, y));
            if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
                return;
            }
        }
        x--;
        y--;
        for (int i = 0; i < RadiusRange[2]; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius)) {
                world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            }
        }
        x += 2;
        y += 2;
        for (i = 0; i < RadiusRange[2]; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius)) {
                world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            }
        }
        x--;
        y--;
        for (i = 0; i < RadiusRange[2]; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius)) {
                rocky_set(x_radius, y_radius, ROCKINESS_FLAT);
                bit_set(x_radius, y_radius, BIT_RIVERBED, false);
                bit_set(x_radius, y_radius, BIT_FUNGUS, false);
                bit2_set(x_radius, y_radius, BIT2_UNITY, true);
                code_set(x_radius, y_radius, i);
                if (!i || i == 8 || i == 10 || i == 19) {
                    bit_set(x_radius, y_radius, BIT_SUPPLY_POD, true);
                } else {
                    bit_set(x_radius, y_radius, BIT_SUPPLY_REMOVE, true);
                }
            }
        }
        new_landmark(x, y, StringTable.get(Natural[LM_UNITY].name));
    }
}

/*
Purpose: Setup the 'Fossil Ridge' landmark (SMACX only).
// ORIGINAL: 0x005C7A80 ?world_fossil@@YAXHH@Z 0x005C7A80-0x005C7CA1
// HISTORICAL KNOWN DIVERGENCE (now restored): the `if (ExpansionEnabled)`
//            guard at the top of this body has no counterpart in the image -
//            0x005C7A80 goes straight into the site search, and the only
//            guard is in world_landmarks (0x005C8629). It is this tree's own
//            "added check similar to Unity Wreckage", so calling world_fossil
//            from anywhere but world_landmarks (Console::editor_natural does)
//            behaves differently from the shipped image with the expansion
//            disabled. Removing it is a behaviour change, not a spelling
//            fix, and needs a decision before the body can be chased.
// size      545 bytes
// prototype void (__cdecl ?world_fossil@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls (inside the `if (ExpansionEnabled)` guard) written
//        out as guard clauses matching the image's call count and
//        fall-through polarity. Best flag set (/O2 /Gy /GR- /Oy- /GX)
//        moved 1/183 -> 31/183, 0.850 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_fossil(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (!is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (uint32_t i = 0; i < 6; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_OCEAN, true);
            bit2_set(x_radius, y_radius, BIT2_FOSSIL, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_FOSSIL].name));
}

/*
Purpose: Setup the 'Nessus Canyon' landmark.
// ORIGINAL: 0x005C7CB0 ?world_canyon@@YAXHH@Z 0x005C7CB0-0x005C7F38
// size      648 bytes
// prototype void (__cdecl ?world_canyon@@YAXHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
// Historical implementation note: ALT4/ALT3 and BIT2/code were rolled into one loop; the
// pinned function performs the altitude pass first and a separate mask/code pass afterward.
void __cdecl world_canyon(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                y = rnd(MapLatitudeBounds - 16, NULL) + 8;
                int x_seed = rnd(MapLongitudeBounds, NULL);
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    } 
    for (uint32_t i = 0; i < 12; i++) {
        int x_radius = xrange(x + RadiusOffsetX[WorldCanyonOffsetValues[i]]);
        int y_radius = y + RadiusOffsetY[WorldCanyonOffsetValues[i]];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
        }
    }
    for (uint32_t j = 0; j < 12; j++) {
        int x_radius = xrange(x + RadiusOffsetX[WorldCanyonOffsetValues[j]]);
        int y_radius = y + RadiusOffsetY[WorldCanyonOffsetValues[j]];
        if (on_map(x_radius, y_radius)) {
            bit2_set(x_radius, y_radius, BIT2_CANYON, true);
            code_set(x_radius, y_radius, WorldCanyonOffsetValues[j]);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_CANYON].name));
}

/*
Purpose: Setup the 'Sunny Mesa' landmark.
// ORIGINAL: 0x005C7F40 ?world_mesa@@YAXHH@Z 0x005C7F40-0x005C8148
// size      520 bytes
// prototype void (__cdecl ?world_mesa@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 1/175 -> 12/175, 0.851 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_mesa(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[3]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, (i < RadiusRange[2]) + ALT_SHORE_LINE, true);
            // 5C80F5 supplies the terrain bit, not Natural[]'s landmark id.
            bit2_set(x_radius, y_radius, BIT2_MESA, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_MESA].name));
}

/*
Purpose: Setup the 'Pholus Ridge' landmark.
// ORIGINAL: 0x005C8150 ?world_ridge@@YAXHH@Z 0x005C8150-0x005C83AB
// size      603 bytes
// prototype void (__cdecl ?world_ridge@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 4/199 -> 5/199, 0.699 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
// Historical implementation note: BIT2/code was rolled into the ALT4 loop; the pinned function
// performs separate 13-entry altitude and mask/code passes using the original offset table.
void __cdecl world_ridge(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (uint32_t i = 0; i < 13; i++) {
        int x_radius = xrange(x + RadiusOffsetX[world_ridge_offset_val[i]]);
        int y_radius = y + RadiusOffsetY[world_ridge_offset_val[i]];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
        }
    }
    for (uint32_t j = 0; j < 13; j++) {
        int x_radius = xrange(x + RadiusOffsetX[world_ridge_offset_val[j]]);
        int y_radius = y + RadiusOffsetY[world_ridge_offset_val[j]];
        if (on_map(x_radius, y_radius)) {
            bit2_set(x_radius, y_radius, BIT2_RIDGE, true);
            code_set(x_radius, y_radius, world_ridge_offset_val[j]);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_RIDGE].name));
}

/*
Purpose: Setup the 'Geothermal Shallows' landmark.
// ORIGINAL: 0x005C83B0 ?world_geothermal@@YAXHH@Z 0x005C83B0-0x005C8627
// size      631 bytes
// prototype void (__cdecl ?world_geothermal@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 3/207 -> 12/207, 0.492 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_geothermal(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (!is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    // LEVER: this is the ONLY calls block - the image has exactly one
    // world_alt_set/bit2_set/code_set per iteration, gated by BOTH conditions below,
    // with a plain `has_skipped = true;` (no calls at all) on the else side. The prior
    // recovery invented a second, unconditional set of the same three calls after the
    // if/else, which the disassembly does not have. The earlier call comparison
    // caught it (this tree made 13 calls against the image's 9),
    // and walking 0x005C83B0's raw bytes end to end confirmed the single-block shape.
    // The bit2_set argument is BIT2_GEOTHERMAL (0x400), not LM_GEOTHERMAL (10) - the
    // image pushes 0x400.
    BOOL has_skipped = false;
    for (int i = 0; i < RadiusRange[2]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (is_ocean(x_radius, y_radius) || i < 9)) {
            if ((has_skipped || !i || rand() % 25) && (i < 9 || rand() % 3)) {
                world_alt_set(x_radius, y_radius, ALT_OCEAN_SHELF, true);
                bit2_set(x_radius, y_radius, BIT2_GEOTHERMAL, true);
                code_set(x_radius, y_radius, i);
            } else {
                has_skipped = true;
            }
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_GEOTHERMAL].name));
}

/*
Purpose: Set up all the landmarks.
// ORIGINAL: 0x005C8630 ?world_landmarks@@YAXXZ 0x005C8630-0x005C86DC BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Preserve the original landmark generation sequence without the later Nessus Canyon addition.","evidence":["Pinned5C8630..5C86DC calls fifteen landmark generators and tail-jumps592940 fixup_landmarks; no world_canyon call follows world_geothermal.","Pinned5C8671..5C8690 tests ExpansionEnabled before unity and reloads it before fossil only within the enabled path."]}
// BUG IN THE RECOVERY: the two ExpansionEnabled guards were dropped from here
//            ("removed redundant check") but the image performs BOTH -
//            `mov eax,[0x9a6488]; test eax,eax; je` before world_unity and
//            again before world_fossil (0x5C8622 and 0x5C8629). world_unity
//            re-tests it internally (0x5C7756); world_fossil does NOT, so the
//            caller's guard is the only thing that keeps the Fossil Field off
//            a non-expansion map. Restoring them took 30/57 -> 55/57.
// TRIED: the last 5 instructions are the deliberate `world_canyon(-1, -1)`
//            this tree adds after world_geothermal ("Change: add Nessus
//            Canyon"): it is one more `push -1 / push -1 / call` block plus
//            8 bytes of stack the image's `add esp, 0x30` does not account
//            for, so no byte-exact match is reachable while that landmark is
//            generated. Historical deviation, not the current faithful sequence.
// size      172 bytes
// prototype 
// callers   0   call targets   15
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C5C70 0x005C5EF0 0x005C6200 0x005C64A0 0x005C6740 0x005C69E0 0x005C6C40 0x005C6DB0 0x005C7020 0x005C7540 0x005C7750 0x005C7A80 0x005C7F40 0x005C8150 0x005C83B0
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_landmarks() {
    world_monsoon(-1, -1);
    world_crater(-1, -1);
    world_volcano(-1, -1, false);
    world_mesa(-1, -1);
    world_ridge(-1, -1);
    world_diamond(-1, -1);
    world_ruin(-1, -1);
    if (ExpansionEnabled) {
        world_unity(-1, -1);
        if (ExpansionEnabled) {
            world_fossil(-1, -1);
        }
    }
    world_temple(-1, -1);
    world_borehole(-1, -1);
    world_sargasso(-1, -1);
    world_dune(-1, -1);
    world_fresh(-1, -1);
    world_geothermal(-1, -1);
    fixup_landmarks();
}

// ORIGINAL: 0x005C32A0 ?world_rocky@@YAXXZ 0x005C32A0-0x005C343F FILE
// size      415 bytes
// prototype 
// callers   2   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591F00 0x005FCB20 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C32A0
// measured tier  MISMATCH
// divergence     66
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?world_rocky@@YAXXZ  at 0x005C32A0  (415 bytes)
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
void __cdecl world_rocky() {
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            int half_x = x >> 1;
            Map *rec = &map_tiles()[MapLongitude * y + half_x];
            // offset 0xc is the tile's bit2: a landmark code already on the
            // tile disqualifies it from being re-rolled.
            uint32_t val3 = rec->bit2;

            bool skip = false;
            if ((val3 & 0x80000001) == 1 && static_cast<int>(val3 & 0xff000000) < 0x15000000) {
                skip = true;
            } else if ((val3 & 0x80004000) == 0x4000) {
                unsigned int hi = val3 & 0xff000000;
                if (static_cast<int>(hi) < 0x9000000 || hi == 0xc000000 || hi == 0xa000000 ||
                    hi == 0x13000000 || hi == 0xf000000 || hi == 0x14000000 || hi == 0x10000000) {
                    skip = true;
                }
            }

            if (skip) {
                continue;
            }

            int minerals = minerals_at(x, y);
            rocky_set(x, y, minerals);

            Map *tile = &map_tiles()[MapLongitude * y + half_x];
            uint32_t flags = tile->bit2;

            if ((flags & 0x80) != 0 && (tile->climate & 0xe0) <= 0x5f) {
                continue;
            }

            unsigned int bitval;
            if ((tile->climate & 0xe0) < 0x60) {
                bitval = 0x10000;
            } else {
                bool useRandomFlat = false;
                if ((flags & 0x80000004) == 4) {
                    unsigned int r = rand();
                    r &= 0x80000001;
                    bool zero;
                    if (static_cast<int>(r) < 0) {
                        zero = ((r - 1) | 0xfffffffe) == 0xffffffff;
                    } else {
                        zero = (r == 0);
                    }
                    useRandomFlat = zero;
                }
                if (useRandomFlat) {
                    bitval = 0x10000;
                } else {
                    unsigned char b = tile->climate;
                    if ((b & 0xe0) < 0x81 && (b & 0x18) < 9) {
                        continue;
                    }
                    bitval = 0x20000;
                }
            }
            bit_set(x, y, bitval, 1);
        }
        do_all_non_input();
    }
}

// ORIGINAL: 0x005C4410 ?world_UNK1@@YAXHH@Z 0x005C4410-0x005C4467 FILE
// size      87 bytes
// prototype void (__cdecl ?world_UNK1@@YAXHH@Z)(int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591A80
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C4410
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c4410/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?world_UNK1@@YAXHH@Z  at 0x005C4410  (87 bytes)
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
void __cdecl world_UNK1(int x, int y) {
    int rainfall = ((map_loc(x, y)->climate >> 3) & 3) + 1;
    if (rainfall < 0) {
        rainfall = 0;
    } else if (rainfall > 2) {
        rainfall = 2;
    }
    climate_set(x, y, rainfall);
}

/*
Purpose: Carve rivers. Every land tile is tried as a source in turn: the
         walker picks a weighted random direction each step, prefers downhill
         and existing channels, stops at the sea or after 999 steps, and marks
         the tiles it ran through. Tiles that stayed dry keep bit 0x200 out of
         their `bit` dword; bit 0x80 is then set on every tile a river crossed.
// ORIGINAL: 0x005C38B0 ?world_rivers@@YAXXZ 0x005C38B0-0x005C3F66 FILE
// size      1718 bytes
// prototype void (__cdecl ?world_rivers@@YAXXZ)()
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591260 0x00591290 0x005FCB20 0x006257E0 0x00625810
// LEVER: homed 2026-09-05 out of src/unrecovered/005c38b0.cpp (deleted).
//   Every fixed address was already a tree name once AltContour (0x0068FB20)
//   landed: RiverStepX / RiverStepY are the paired diagonal tables the
//   RadiusOffset note in map.h documents, and the rest are MapLongitude /
//   MapLongitudeBounds / MapLatitudeBounds / MapArea / MapIsFlat /
//   map_tiles(). Its six calls are random_reseed, random (three sites),
//   alt_put_detail and do_all_non_input.
// TRIED: byte-exactness is out of reach on this pass. The image tests
//   `tile->bit` with byte-width forms (`test ch, 1` for the 0x100 bit) and
//   interleaves its fourteen stack locals differently from any spelling
//   measured here; the archived transcription stopped at instruction #15 of
//   the framed build for the same reason. The control flow, the field offsets
//   and the call sequence below are the image's, so call_diff and
//   dead_branches can both check it. The five ways a step can end early are
//   NOT interchangeable, and this is the part a rewrite gets wrong: hitting
//   water (bit 0x80) or a three-river junction ABANDONS the whole walk with
//   to_sea cleared and the head not advanced, while a candidate whose own
//   eight neighbours disqualify it only drops out of the four-direction loop.
*/
void __cdecl world_rivers() {
    // Every tile: forget that a river ran through it.
    for (int i = 0; i < MapArea; ++i) {
        map_tiles()[i].bit &= 0xfffffd7f;
    }

    if (MapLatitudeBounds > 0) {
        int y_coord = 0;
        do {
            for (int x_coord = y_coord & 1; x_coord < MapLongitudeBounds;
                 x_coord += 2) {
                Map *source = map_tiles() + MapLongitude * y_coord
                              + (x_coord >> 1);
                if (((source->bit & 0x100) == 0)
                    || ((source->climate & 0xE0) <= 0x5F)) {
                    continue;
                }

                // A fresh source: forget every earlier river's path first.
                for (int i = 0; i < MapArea; ++i) {
                    map_tiles()[i].bit2 &= 0xff7fffff;
                }

                random_reseed(y_coord * 0x2f + x_coord * 0x1f);
                int river_dir = random(0, 4);

                int band = source->climate >> 5;
                int contour = source->contour;
                source->bit2 |= 0x800000;

                int cur_x = x_coord;
                int cur_y = y_coord;
                int joined = 0;      // stepped into an existing river
                int to_sea = 1;      // the walk has not hit water yet
                int hill_run = 1;    // tiles climbed at this altitude
                int step_parity = 1;
                int steps = 0;
                uint32_t bits = source->bit;

                while ((bits & 0x1000000) == 0) {
                    int last_dir = random(0, 2);
                    int turn = last_dir + (river_dir - (step_parity & 1)) & 3;
                    int straight = river_dir ^ 2;
                    int nb_contour = 0;
                    int nb_climate = 0;
                    int cand_split[3];
                    int cand_dir[3];
                    int cand_weight[3];
                    int cand_count = 0;
                    int total_weight = 0;
                    int weight_base = 8;
                    int best_contour = 9999;
                    bool abandon = false;

                    for (int dir = 0; dir < 4; dir++) {
                        if (dir == straight) {
                            continue;
                        }
                        int next_x = RiverStepX[dir] + cur_x;
                        int flat = MapIsFlat & 1;
                        if (flat == 0) {
                            if (next_x < 0) {
                                next_x += MapLongitudeBounds;
                            } else if (MapLongitudeBounds <= next_x) {
                                next_x -= MapLongitudeBounds;
                            }
                        }
                        int next_y = RiverStepY[dir] + cur_y;
                        if (next_y < 0 || next_y >= MapLatitudeBounds
                            || next_x < 0 || next_x >= MapLongitudeBounds) {
                            continue;
                        }

                        Map *neighbour = map_tiles()
                                         + MapLongitude * next_y
                                         + (next_x >> 1);
                        if ((neighbour->bit & 0x80) != 0) {
                            abandon = true;
                            break;
                        }
                        if ((neighbour->bit & 0x100) != 0) {
                            // A junction, never a source: count this
                            // neighbour's own river neighbours.
                            int rivers = 0;
                            for (int k = 0; k < 4; k++) {
                                int nx = RiverStepX[k] + next_x;
                                if (flat == 0) {
                                    if (nx < 0) {
                                        nx += MapLongitudeBounds;
                                    } else if (MapLongitudeBounds <= nx) {
                                        nx -= MapLongitudeBounds;
                                    }
                                }
                                int ny = RiverStepY[k] + next_y;
                                if (ny < 0 || ny >= MapLatitudeBounds
                                    || nx < 0 || nx >= MapLongitudeBounds) {
                                    continue;
                                }
                                if ((map_tiles()[MapLongitude * ny + (nx >> 1)]
                                         .bit
                                     & 0x100)
                                    != 0) {
                                    rivers++;
                                }
                            }
                            if (rivers > 1) {
                                abandon = true;
                                break;
                            }
                        }

                        nb_climate = neighbour->climate;
                        nb_contour = neighbour->contour;
                        if ((nb_climate >> 5) > band) {
                            // Downhill or level only.
                            continue;
                        }

                        int drop = (contour - nb_contour) + 2;
                        if (drop < 1) {
                            drop = 1;
                        } else if (drop > 9999) {
                            drop = 9999;
                        }
                        int weight = drop * 0x20;
                        if (dir == turn) {
                            weight += 0x1f;
                        }
                        if (contour < nb_contour) {
                            if (best_contour < nb_contour) {
                                continue;
                            }
                            weight = weight_base;
                            weight_base++;
                            best_contour = nb_contour;
                        }

                        int next_split = 0;
                        int climb = 0;
                        for (int k = 0; k < 4; k++) {
                            if (k == (dir ^ 2)) {
                                continue;
                            }
                            int kx = RiverStepX[k] + next_x;
                            if (flat == 0) {
                                if (kx < 0) {
                                    kx += MapLongitudeBounds;
                                } else if (MapLongitudeBounds <= kx) {
                                    kx -= MapLongitudeBounds;
                                }
                            }
                            int ky = RiverStepY[k] + next_y;
                            if (ky < 0 || ky >= MapLatitudeBounds || kx < 0
                                || kx >= MapLongitudeBounds) {
                                continue;
                            }
                            Map *around = map_tiles() + MapLongitude * ky
                                          + (kx >> 1);
                            if ((around->climate & 0xE0) < 0x60
                                && (nb_climate & 0xE0) > 0x5F && hill_run < 5) {
                                weight /= 2;
                            }
                            if (nb_contour < around->contour) {
                                climb++;
                                if ((around->bit & 0x80) != 0) {
                                    break;
                                }
                            }
                            if ((around->bit2 & 0x800000) != 0) {
                                if (joined != 0
                                    || (around->climate & 0xE0) < 0x60) {
                                    break;
                                }
                                next_split = 1;
                                weight = weight / 8;
                            }
                        }
                        if (weight == 0) {
                            continue;
                        }
                        if (climb == 3) {
                            weight = 1;
                        }
                        total_weight += weight;
                        cand_weight[cand_count] = weight;
                        cand_split[cand_count] = next_split;
                        cand_dir[cand_count] = dir;
                        cand_count++;
                    }

                    if (abandon) {
                        to_sea = 0;
                        break;
                    }
                    if (cand_count == 0) {
                        break;
                    }

                    int pick = random(0, total_weight);
                    river_dir = 0;
                    for (int i = 0; i < cand_count; i++) {
                        if (pick < cand_weight[i]) {
                            joined = cand_split[i];
                            river_dir = cand_dir[i];
                            break;
                        }
                        pick -= cand_weight[i];
                    }

                    cur_x += RiverStepX[river_dir];
                    if ((MapIsFlat & 1) == 0) {
                        if (cur_x < 0) {
                            cur_x += MapLongitudeBounds;
                        } else if (MapLongitudeBounds <= cur_x) {
                            cur_x -= MapLongitudeBounds;
                        }
                    }
                    cur_y += RiverStepY[river_dir];

                    Map *head = map_tiles() + MapLongitude * cur_y
                                + (cur_x >> 1);
                    band = head->climate >> 5;
                    int next_contour = head->contour;

                    if (contour < next_contour) {
                        // Drop the tile to its band's nominal contour, but
                        // never below one step under the tile we came from.
                        int target = AltContour[band];
                        if (AltContour[band] < contour - 1) {
                            target = contour - 1;
                        }
                        alt_put_detail(cur_x, cur_y, target);
                        next_contour = target;
                    }

                    steps++;
                    contour = next_contour;
                    if (steps > 999) {
                        break;
                    }
                    head->bit2 |= 0x800000;
                    step_parity++;
                    hill_run++;
                    bits = head->bit;
                }

                if (band > 2) {
                    Map *mouth = map_tiles() + MapLongitude * cur_y
                                 + (cur_x >> 1);
                    if (((mouth->bit & 0x80) == 0) && (to_sea != 0)
                        && (((mouth->bit & 3) == 0)
                            || (7 < (mouth->val2 & 0xf)))) {
                        mouth->bit |= 0x200;
                    }
                }

                // Every tile this river crossed is now a river tile.
                for (int crossed = 0; crossed < MapArea;
                     ++crossed) {
                    if ((map_tiles()[crossed].bit2 & 0x800000) != 0) {
                        map_tiles()[crossed].bit |= 0x80;
                    }
                }
            }
            do_all_non_input();
            y_coord++;
        } while (y_coord < MapLatitudeBounds);
    }

    for (int swept = 0; swept < MapArea; ++swept) {
        map_tiles()[swept].bit2 &= 0xff7fffff;
    }
}

// Historical source-seed record retained during owner migration.
// The dated BLOCKED note below is historical evidence, not current status.
// BLOCKED (map pass, 2026-09-05): NOT homed - world_build (map.cpp) reaches it
//   through a pending_bodies.cpp forwarder. Everything this body needs is now
//   NAMED, which was the only thing missing: Fractal is src/fractal.h
//   (char grid_[2][19][19], 722 bytes - this body's `sub esp, 0x314` is that
//   plus its fourteen stack locals), VehMoveDeltaX/VehMoveDeltaY are map.h's
//   tables at 0x0066EF50/0x0066EF74, 0x0066EFE0/0x0066F464 are
//   RadiusOffsetX[9]/RadiusOffsetY[9], 0x0068FB50 is &AltNatural[1],
//   0x00945B00 is path.cpp's Paths (Paths.continents() at 0x005C2D0F), and the
//   WorldBuilder fields at 0x009502F0..0x00950304 are deep_water / shelf /
//   plains / beach / hills. The rewrite is tree-style only: keep this
//   artifact's five goto targets distinct - 0x005C2DF6 is "do the level
//   computation", 0x005C3058 is its band switch, 0x005C30DE is the post-walk
//   coast clamp, and the two that SETTLE for the tile are 0x005C321F and
//   0x005C3058's tail - and note that `local_1c` / `local_8` are read before
//   they are written on the band==2 path (BUG IN THE ORIGINAL, carry-over from
//   the previous tile; reproduce it, do not fix it).

// ORIGINAL: 0x005C2CB0 ?world_erosion@@YAXXZ 0x005C2CB0-0x005C3299
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: complete original entry 0x005C2CB0 with span 0x005C2CB0-0x005C3299; native disassembly traced across both erosion passes.", "src/fractal.h owns the 722-byte Fractal sampled by the two native calls; src/map.h owns Map, map_tiles, neighbor tables and AltNatural; src/path.h owns Paths.continents; map.cpp includes the genuine WorldBuilder and terrain interfaces.", "Pinned native flow initializes the per-tile intensity at 0x005C2DC1, 0x005C2DCA or 0x005C2DCF before altitude dispatch at 0x005C2DF6, and initializes neighbor counters before their walks at 0x005C2E18, 0x005C2EB1 and 0x005C2F43. The seed's uninitialized/carry warning is historical, not a proved current defect; the owner body follows the pinned initialized paths.", "Pinned branch targets 0x005C30B6/0x005C3221 settle altitude-2 low threshold directly; 0x005C30DE applies the independent altitude-3 neighbor clamp before coast blending."], "reason": "Home the pinned 0x005C2CB0 terrain-erosion body in map.cpp's real Fractal, Map, WorldBuilder, Paths and altitude-table owners while preserving the original image identity and extent."}
// TRIED: Ghidra-shaped goto/label transcription (verified instruction-by- instruction against the raw disassembly for control flow, push order and branch polarity) compiles and matches through instruction #41; diverges afterward on register-allocation shape (shl vs mov) inside the inner tile loop. Not chased further given the function's size (~500 instructions).
void __cdecl world_erosion() {
    Fractal fractal;
    fractal.init();

    int map_scale = 1;
    int world_scale = 1;
    if (MapLandCoverage == 1) {
        map_scale = quick_root(0xC80);
        world_scale = quick_root(MapArea);
    }

    int pass;
    for (pass = 0; pass < 2; ++pass) {
        if (pass != 0)
            world_shorelines();
        Paths.continents();

        int y;
        for (y = 0; y < MapLatitudeBounds; ++y) {
            int x = y & 1;
            for (; x < MapLongitudeBounds; x += 2) {
                Map *const tile = &map_tiles()[
                    MapLongitude * y + (x >> 1)];
                const unsigned contour = tile->contour;
                const unsigned altitude = tile->climate >> 5;
                int intensity = fractal.sample(x << 4, y << 4) * 5;
                const int fine = fractal.sample(x << 7, y << 7);
                intensity = (intensity + fine * 2) * 7 >> 8;
                if (intensity < 0)
                    intensity = 0;
                else if (intensity > 100)
                    intensity = 100;

                const bool process = (pass == 0) ? altitude <= 2 : altitude > 2;
                if (!process)
                    continue;

                int result = 0;
                int local_1c;
                int local_8;
                int region = -1;
                int neighbor_index;

                if (altitude == 2 || altitude == 3) {
                    // Count low-altitude tiles in the four-neighbor ring.
                    local_8 = 0;
                    for (neighbor_index = 0; neighbor_index < 8;
                         ++neighbor_index) {
                        int nx = x + VehMoveDeltaX[neighbor_index];
                        if (!(MapIsFlat & 1)) {
                            if (nx < 0)
                                nx += MapLongitudeBounds;
                            else if (nx >= MapLongitudeBounds)
                                nx -= MapLongitudeBounds;
                        }
                        const int ny = y + VehMoveDeltaY[neighbor_index];
                        if (ny >= 0 && ny < MapLatitudeBounds
                            && nx >= 0 && nx < MapLongitudeBounds) {
                            const Map *const neighbor = &map_tiles()[
                                MapLongitude * ny + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) < 0x41)
                                ++local_8;
                        }
                    }

                    if (altitude != 3) {
                        local_1c = 0;
                        region = -1;
                        for (neighbor_index = 0; neighbor_index < 8;
                             ++neighbor_index) {
                            int nx = x + VehMoveDeltaX[neighbor_index];
                            if (!(MapIsFlat & 1)) {
                                if (nx < 0)
                                    nx += MapLongitudeBounds;
                                else if (nx >= MapLongitudeBounds)
                                    nx -= MapLongitudeBounds;
                            }
                            const int ny = y + VehMoveDeltaY[neighbor_index];
                            if (ny < 0 || ny >= MapLatitudeBounds
                                || nx < 0 || nx >= MapLongitudeBounds)
                                continue;
                            const Map *const neighbor = &map_tiles()[
                                MapLongitude * ny + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) <= 0x40)
                                continue;
                            const int next_region = neighbor->region;
                            if (region < 0) {
                                region = next_region;
                            } else if (region != next_region) {
                                uint32_t scaled_area_bits =
                                    static_cast<uint32_t>(MapArea) * 0x32u;
                                int scaled_area;
                                memcpy(&scaled_area, &scaled_area_bits, sizeof(scaled_area));
                                uint32_t neighboring_tiles_bits =
                                    static_cast<uint32_t>(Continents[region].tile_count)
                                  + static_cast<uint32_t>(Continents[next_region].tile_count);
                                int neighboring_tiles;
                                memcpy(&neighboring_tiles, &neighboring_tiles_bits,
                                       sizeof(neighboring_tiles));
                                if (scaled_area / 0xC80 <= neighboring_tiles)
                                    local_1c = 1;
                                if (Continents[region].tile_count
                                    < Continents[next_region].tile_count)
                                    region = next_region;
                            }
                        }
                    } else {
                        local_1c = 0;
                        for (neighbor_index = 0; neighbor_index < 16;
                             ++neighbor_index) {
                            int nx = x + RadiusOffsetX[9 + neighbor_index];
                            if (!(MapIsFlat & 1)) {
                                if (nx < 0)
                                    nx += MapLongitudeBounds;
                                else if (nx >= MapLongitudeBounds)
                                    nx -= MapLongitudeBounds;
                            }
                            const int ny = y + RadiusOffsetY[9 + neighbor_index];
                            if (ny < 0 || ny >= MapLatitudeBounds
                                || nx < 0 || nx >= MapLongitudeBounds)
                                continue;
                            const Map *const neighbor = &map_tiles()[
                                MapLongitude * ny + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) <= 0x40)
                                continue;
                            const unsigned next_region = neighbor->region;
                            if (next_region < 0x40
                                && next_region != tile->region)
                                ++local_1c;
                        }
                    }

                    if (altitude == 2) {
                        if (local_1c != 0) {
                            local_8 += 4;
                            if (local_8 < 0)
                                local_8 = 0;
                            else if (local_8 > 8)
                                local_8 = 8;
                        }
                        int adjusted = intensity;
                        if (intensity < static_cast<int>(contour))
                            adjusted = static_cast<int>(WorldBuilder.shelf)
                                     + intensity;
                        if (adjusted < static_cast<int>(AltNatural[altitude + 1])) {
                            result = adjusted;
                            goto settle_tile;
                        }

                        const int value = (local_8 - 3)
                            * static_cast<int>(WorldBuilder.shelf);
                        intensity = -((value + ((value >> 31) & 3)) >> 2);
                        result = adjusted + intensity;
                        goto settle_tile;
                    } else if (local_1c != 0) {
                        // Native 0x5C30DE applies this carry to altitude 3
                        // independently of altitude 2's earlier clamp.
                        local_8 += 4;
                        if (local_8 < 0)
                            local_8 = 0;
                        else if (local_8 > 8)
                            local_8 = 8;
                    }

                    result = ((8 - local_8)
                              * WorldBuilder.plains
                              * world_scale / (map_scale * 8)) + intensity;
                    if (result < static_cast<int>(contour)) {
                        int ocean_coverage = MapOceanCoverage < 1 ? 1
                            : (MapOceanCoverage > 2 ? 2
                                : static_cast<int>(MapOceanCoverage));
                        intensity = (((((static_cast<int>(WorldBuilder.hills)
                            * static_cast<int>(MapErosiveForces)) / 2)
                            * (8 - local_8) / (ocean_coverage * 8))
                            + WorldBuilder.beach) * world_scale) / map_scale;
                        result += intensity;
                    } else {
                        int roughness = local_8 < 0 ? 0
                            : (local_8 > 8 ? 8 : local_8);
                        int ocean_coverage = MapOceanCoverage < 1 ? 1
                            : (MapOceanCoverage > 2 ? 2
                                : static_cast<int>(MapOceanCoverage));
                        result = ((result - static_cast<int>(contour))
                            * (8 - roughness) * static_cast<int>(MapErosiveForces))
                            / (ocean_coverage * 8) + static_cast<int>(contour);
                    }
                } else if (altitude < 2) {
                    result = intensity - WorldBuilder.deep_water;
                } else if (altitude == 4) {
                    if (static_cast<int>(contour) < intensity)
                        intensity = intensity * 2 - static_cast<int>(contour);
                    result = intensity
                        + static_cast<int>(WorldBuilder.hills)
                          * static_cast<int>(MapErosiveForces)
                        + WorldBuilder.peaks;
                } else if (altitude > 4) {
                    result = static_cast<int>(WorldBuilder.hills)
                        * static_cast<int>(MapErosiveForces) + intensity;
                }

settle_tile:
                result = (result + 1 + static_cast<int>(contour)) / 2;
                if (MapErosiveForces == 0 && result > 0x4A)
                    result = 0x4A;
                alt_put_detail(x, y, result);
            }
            do_all_non_input();
        }
    }
}

/*
Purpose: Build the whole planet: reset the altitude table and the Map* rules
         run, wipe the tiles, raise continents and hills until the land targets
         are met, then hand the result to erosion, the polar caps, the
         shorelines and every landmark/terrain pass, and refresh the map
         windows. Repeats up to four times until world_validate accepts the
         planet or the omniscient-view bit aborts the retry.
// ORIGINAL: 0x005C86E0 ?world_build@@YAXXZ 0x005C86E0-0x005C899D FILE
// size      701 bytes
// prototype void (__cdecl ?world_build@@YAXXZ)()
// callers   2   call targets   30
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046B190 0x0046FD90 0x004C45E0 0x00591040 0x00592940 0x005C28F0
//           0x005C2B40 0x005C2CB0 0x005C32A0 0x005C3440 0x005C3680 0x005C3F70
//           0x005C40F0 0x005C4170 0x005C4470 0x005C5A30 0x005C5AE0 0x005C5EF0
//           ... and the seventeen world_* landmark passes.
// LEVER: homed 2026-09-05. Every callee was already homed in this file or
//   declared in map.h, and every fixed address already had a tree name but
//   three: 0x0068FAF4 and 0x0068FB20 are now AltNaturalDefault and AltContour
//   (map.h, the two 11-int tables that bracket AltNatural), and 0x009B22EC /
//   0x009B22E8 are map.cpp's own WorldClimateSkipTerritoryReset /
//   WorldClimateRedrawRequest, now extern in map.h.
//   The archived transcription this replaces MISSED the `mov [0x94988c], esi`
//   at 0x005C8769 - world_build clears MapIsFlat before raising anything,
//   which is why the flat-map guard at 0x005C8839 tests the same dword.
// TRIED, all measured: the eleven-int AltNatural reset as a pointer-bump
//   do/while chasing the image's `rep movsd` (VC6 kept a one-pointer loop
//   with a +0x58 displacement and the body went frameless, 15 of 215 against
//   22 of 215 for the plain for-loop); hoisting MapWinTable into a local
//   reference for the tail walk. call_diff FEWER by one is the map_win_table
//   accessor: the image folds 0x007D3C3C into `mov esi, imm32` inside the
//   loop, this tree's `MapWinSlots &map_win_table()` in mapwin.h is a real
//   CALL at all three sites here, and forcing it inline is a mapwin.h edit
//   that would touch game.cpp's and time.cpp's walks as well as this body.
//   The image's register plan (EBX = continent count live across the whole
//   body, ESI/EDI parked on the copy) is otherwise the wall.
*/
void __cdecl world_build() {
    int continent_count = 0;

    int attempt = 0;
    do {
        // TRIED: the pointer-bump do/while spelling of this copy, hoping for
        // the image's single `rep movsd`; VC6 kept a one-pointer loop with a
        // +0x58 displacement and the whole body came out frameless (/GX
        // selected) at 15 of 215 against 22 of 215 for the plain for-loop
        // below. The `rep movsd` is a counted-copy idiom this tree has not
        // reached from source yet.
        for (int i = 0; i < 11; ++i) {
            AltNatural[i] = AltNaturalDefault[i];
        }

        WorldClimateRedrawRequest = 1;
        WorldClimateSkipTerritoryReset = 1;

        int *const rule_end = MapStorage.metadata_.planet_rules_ + 7;
        for (int *rules = MapStorage.metadata_.planet_rules_; rules < rule_end; ++rules) {
            if (*rules >= 0) {
                continue;
            }
            *rules = rand() % 3;
        }

        map_wipe();

        MapIsFlat = 0;
        uint32_t land_base_bits = (2u - static_cast<uint32_t>(MapOceanCoverage))
                                  * WorldBuilder.land_mod + WorldBuilder.land_base;
        int land_target;
        memcpy(&land_target, &land_base_bits, sizeof(land_target));
        uint32_t land_target_bits = static_cast<uint32_t>(land_target)
                                    * static_cast<uint32_t>(MapArea);
        memcpy(&land_target, &land_target_bits, sizeof(land_target));
        land_target /= 3200;
        uint32_t island_target_bits = static_cast<uint32_t>(WorldBuilder.islands)
                                      * static_cast<uint32_t>(MapArea);
        int island_target;
        memcpy(&island_target, &island_target_bits, sizeof(island_target));
        island_target /= 3200;
        BrushVal1 = 0;
        WorldBuildVal1 = land_target;

        if ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
            MapWinTable[0]->clear_terrain();
            draw_map(1);
        }

        int builds = 0;
        while (BrushVal1 < WorldBuildVal1 || continent_count < island_target) {
            // The original increments and checks this before calling build_continent;
            // attempt 100 exits, so at most 99 builds are issued.
            if (++builds >= 100) {
                break;
            }
            continent_count++;
            build_continent(continent_count);
        }

        if (MapErosiveForces >= 1) {
            uint32_t hills_base_bits = (static_cast<uint32_t>(MapErosiveForces) - 1u)
                                       * WorldBuilder.hills_mod + WorldBuilder.hills_base;
            int hills;
            memcpy(&hills, &hills_base_bits, sizeof(hills));
            uint32_t hills_bits = static_cast<uint32_t>(hills)
                                  * static_cast<uint32_t>(MapArea);
            memcpy(&hills, &hills_bits, sizeof(hills));
            hills /= 3200;
            if (MapOceanCoverage == 2) {
                hills = hills / 2;
            }
            if (hills > 0) {
                do {
                    build_hills(4);
                    hills--;
                } while (hills != 0);
            }
        }

        world_erosion();

        if ((MapIsFlat & 1) == 0 && MapOceanCoverage < 2
            && (MapOceanCoverage == 0 || rand() % 3 == 0)) {
            world_polar_caps();
        }

        world_shorelines();
    } while ((GameDataStateStorage.fields_.game_state_ & 0x80) == 0 && world_validate() != 0
             && (++attempt, attempt < 4));

    world_temperature();
    world_riverbeds();
    world_fungus();
    world_monsoon(-1, -1);
    world_crater(-1, -1);
    world_volcano(-1, -1, 0);
    world_mesa(-1, -1);
    world_ridge(-1, -1);
    world_diamond(-1, -1);
    world_ruin(-1, -1);
    if (ExpansionEnabled != 0) {
        world_unity(-1, -1);
        if (ExpansionEnabled != 0) {
            world_fossil(-1, -1);
        }
    }
    world_temple(-1, -1);
    world_borehole(-1, -1);
    world_sargasso(-1, -1);
    world_dune(-1, -1);
    world_fresh(-1, -1);
    world_geothermal(-1, -1);

    fixup_landmarks();
    world_linearize_contours();
    WorldClimateRedrawRequest = 0;
    world_climate();
    world_rocky();
    WorldClimateSkipTerritoryReset = 0;

    if (WorldClimateSkipTerrainClear == 0) {
        MapWinSlots &table = MapWinTable;
        for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); slot++) {
            if (table[slot]->view_active_ != 0) {
                table[slot]->clear_terrain();
            }
        }
        WorldClimateWorldWin.clear_terrain();
    }
}

extern int TerraformTileFaction;
extern int TerraformDrownedBaseId;

static void world_rainfall_pass2(int saved_faction) {
    int pass2_count = 0;
    int rain_carry = 0;
    int cloud_count = 0;
    int y = 0;
    Map *tile_cursor = MapTiles;

    for (y = 0; y < MapLatitudeBounds; ++y) {
        int x = y & 1;
        for (; x < MapLongitudeBounds; x += 2) {
            int rainfall_class = 0;
            int adjustment = 0;
            int tile_baseline = 0;
            if (y > 0 && y < MapLatitudeBounds - 1) {
                Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];
                const uint8_t climate = tile->climate;
                const uint32_t flags = tile->bit2;
                const int rainfall_level = climate & 7;
                const int altitude = climate >> 5;
                const uint8_t scratch = tile_cursor->unk_1;
                const uint32_t coastal = flags & 0x80000040u;
                if (coastal == 0x40u || (flags & 0x80000100u) == 0x100u)
                    rain_carry = 1;

                if (scratch & 0x20) {
                    rainfall_class = tile_baseline = rain_carry = 1;
                    cloud_count = 0;
                }

                const int cloud_cover = static_cast<int>(MapCloudCover);
                if (scratch & 0x80) {
                    rain_carry = rainfall_level > 1 ? 1 : 0;
                    pass2_count = 0;
                    cloud_count = 0;
                    if (cloud_cover >= 1)
                        adjustment = 1;
                }

                if (scratch & 0x10) {
                    if (rain_carry != 0)
                        --rain_carry;
                    --adjustment;
                }

                if ((scratch & 0x40) && altitude <= 4) {
                    pass2_count = 0;
                    cloud_count = 0;
                    if (adjustment < 0)
                        adjustment = 0;
                    rainfall_class = tile_baseline;
                }

                if (altitude > 3) {
                    ++pass2_count;
                    const int pass_limit =
                        static_cast<int>(WorldBuilder.rainfall_coeff)
                        * cloud_cover * 4 + 1;
                    if (pass2_count > pass_limit && rain_carry != 0)
                        --rain_carry;
                } else if (altitude < 3) {
                    cloud_count = 0;
                    rain_carry = 1;
                    if (pass2_count != 0) {
                        pass2_count += -1 - cloud_cover;
                        if (pass2_count < 1) {
                            pass2_count = 0;
                            cloud_count = 0;
                        }
                    }
                } else if (rain_carry == 0) {
                    if (pass2_count != 0) {
                        pass2_count += -1 - cloud_cover;
                        if (pass2_count < 1) {
                            pass2_count = 0;
                            cloud_count = 0;
                            if (!(cloud_cover == 0 && altitude >= 3))
                                rain_carry = 1;
                        }
                    }
                } else if (pass2_count != 0) {
                    pass2_count += -1 - cloud_cover;
                    if (pass2_count < 1) {
                        pass2_count = 0;
                        cloud_count = 0;
                        if (!(cloud_cover == 0 && altitude >= 3))
                            rain_carry = 1;
                    }
                } else {
                    ++cloud_count;
                    const int cloud_limit = cloud_cover * 4 + 2;
                    if (cloud_count > cloud_limit) {
                        rain_carry = 0;
                        cloud_count = 0;
                    }
                }

                if (tile->bit & BIT_RIVER) {
                    if (((x >> 1) % 3) != 0 && adjustment < 0)
                        adjustment = 0;
                    rain_carry = 1;
                    cloud_count = 0;
                }

                if (scratch & 0x02) {
                    --rainfall_class;
                    rain_carry = 0;
                } else if (rain_carry != 0) {
                    adjustment += rain_carry;
                }

                if (altitude <= 4 + (rainfall_level != 0)) {
                    rainfall_class += adjustment;
                } else if (cloud_cover - altitude + rainfall_level + 5 >= 2
                           && adjustment > 0) {
                    ++rainfall_class;
                }

                if ((flags & 0x80000004u) == 0x4u
                    && (climate & 0xE0) >= 0x60 && altitude < 6) {
                    int marker = static_cast<int8_t>(flags >> 24);
                    if (marker < 0) marker = -marker;
                    if (marker < 0x31) {
                        rainfall_class += 2;
                    } else if (marker < 0x51 && altitude < 5) {
                        rainfall_class += 2;
                    } else {
                        ++rainfall_class;
                    }
                }

                if (coastal == 0x40u)
                    rainfall_class = 0;

                if ((flags & 0x80004000u) == 0x4000u || rainfall_class < 0) {
                    rainfall_class = 0;
                } else if (rainfall_class > 2) {
                    rainfall_class = 2;
                }

                if (scratch & 0x01) {
                    rain_carry = 1;
                    if (rainfall_class < 2)
                        ++rainfall_class;
                }
                if (scratch & 0x04) {
                    rain_carry = 1;
                    if (rainfall_class < 2)
                        ++rainfall_class;
                }

                if (GameDataStateStorage.fields_.turn_current_num_ != 0) {
                    const int old_class = (tile_cursor->climate >> 3) & 3;
                    if (old_class != rainfall_class && TerraformDrownedBaseId < 0) {
                        if (old_class > rainfall_class) {
                            bool search = true;
                            if (TerraformTileFaction >= 0
                                && (static_cast<uint8_t>(
                                        GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                                    & (1u << (TerraformTileFaction & 0x1F))))
                                search = false;
                            if (search) {
                                const int found = base_find(x, y);
                                if (found >= 0)
                                    TerraformTileFaction =
                                        Bases[found].faction_id_current;
                            }
                        }
                        const int found = base_find(x, y, saved_faction);
                        if (found >= 0 && BaseFindDist <= 2)
                            TerraformDrownedBaseId = found;
                    }
                }

                tile_cursor->climate = static_cast<uint8_t>(
                    (tile_cursor->climate & 0xE7) | (rainfall_class << 3));
            } else {
                tile_cursor->climate = static_cast<uint8_t>(tile_cursor->climate & 0xE7);
            }
            ++tile_cursor;
        }
        do_all_non_input();
    }
}

static int world_rainfall_xrange(int x, int width, int flat) {
    if ((flat & 1) == 0) {
        if (x < 0)
            x += width;
        else if (x >= width)
            x -= width;
    }
    return x;
}

// Historical seed deferral, superseded by the reconstruction below:
// NOTE (map pass, 2026-09-05): NOT homed - it stays behind its
//   pending_bodies.cpp forwarder, and this artifact is a CONDENSATION, not a
//   register-level transcription, so homing it means re-deriving pass 2's
//   middle ~80 instructions from the raw bytes first. It does not touch
//   Fractal at all (its only calls are do_all_non_input x3, base_find x2 and
//   climate_set) and it names no new globals, so the Fractal class pass that
//   unblocked world_erosion and world_fungus leaves this one where it was.
//   Its frame is `sub esp, 0x38` with no stack object.
// ORIGINAL: 0x005C4470 ?world_rainfall@@YAXXZ 0x005C4470-0x005C4FC1
// RECOVERY_BINDING: v1 {"reason":"Recover the complete three-pass rainfall algorithm through existing Map storage, WorldBuilder terrain parameters and real base/climate APIs. The common land-terrain join and west-to-east scan continuation come from the original control flow, not the old condensed seed. Restore these original state, callback-read and column-arithmetic effects without normalizing the zero-carry, lock-byte or persistent-cursor quirks. The retained TRIED condensation describes seed history; these branch corrections do not establish full-game or linked-preservation acceptance.","evidence":["original:0x005C4470-0x005C4FC1; image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","git:8ebd9101:src/unrecovered/005c4470.cpp","Original0x5C45AD joins at0x5C47DB before all-tile altitude processing; original0x5C499D/0x5C49A7 join at0x5C49E5 before the east scan.","src/map.h Map and radius tables; src/alpha.h RulesWorldbuilder; src/base.h base_find; src/console.h ConsoleGlobal; existing src/veh.cpp terraform results.","Pinned5C4665/46F3/4774 and4919/49BB/49EB/4A86 reload cloud cover at the consumer and cloudmass scan tails; row callbacks service networking. Entry5C4487/448C resets drowned-base then tile-faction.","Pinned5C4C2A/57/5D/6B separates altitude0..2 carry/reset/counter decay from exactly3;5C4CE2 reaches4C6B without forcing carry.5C4B33 preserves x>>1 through river IDIV3 at4CA3.","Pinned5C4ACF/4B84/4DD7/4E61/4E89 retains a separate Map cursor for scratch, live old/final climate and each processed record, while4B27..49 indexes live terrain storage. Base callbacks may alter climate;5C4DFE tests only the lock-mask low byte."]}
// TRIED: no Ghidra hypothesis was available for this address, so this is a from-scratch disassembly transcription (three map passes: windward-hint marking, 2-bit rainfall-class finalization with base_find caching, moderate-rain propagation). Pass 2's middle heuristic (unk_1 bit cascade feeding the 0-3 class value) is a best-effort condensation of ~80 instructions, not a literal register replay - the surrounding loop bounds, byte-merge write and base_find call sites are transcribed directly.
void __cdecl world_rainfall() {
    const int saved_faction = ConsoleGlobal.faction_id_;
    TerraformDrownedBaseId = -1;
    TerraformTileFaction = -1;
    int i;
    for (i = 0; i < MapArea; ++i)
        MapTiles[i].unk_1 = 0;

    int y;
    for (y = 0; y < MapLatitudeBounds; ++y) {
        int x = y & 1;
        for (; x < MapLongitudeBounds; x += 2) {
            Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];
            if (tile->bit & BIT_CONDENSER) {
                for (int dir_cond = 0; dir_cond < 9; ++dir_cond) {
                    int nx = world_rainfall_xrange(
                        x + RadiusOffsetX[dir_cond], MapLongitudeBounds, MapIsFlat);
                    int ny = y + RadiusOffsetY[dir_cond];
                    if (ny >= 0 && ny < MapLatitudeBounds
                        && nx >= 0 && nx < MapLongitudeBounds) {
                        Map *neighbor = &MapTiles[
                            MapLongitude * ny + (nx >> 1)];
                        if (neighbor->unk_1 & 1)
                            neighbor->unk_1 |= 4;
                        else
                            neighbor->unk_1 |= 1;
                    }
                }
            }

            if ((tile->climate & 0xE0) < 0x60) {
                int west = x;
                for (int step_west = 0; step_west < 3; ++step_west) {
                    const int left_x = world_rainfall_xrange(
                        west - 2, MapLongitudeBounds, MapIsFlat);
                    const int center_x = world_rainfall_xrange(
                        west, MapLongitudeBounds, MapIsFlat);
                    if (left_x >= 0 && left_x < MapLongitudeBounds
                        && center_x >= 0 && center_x < MapLongitudeBounds) {
                        Map *left = &MapTiles[
                            MapLongitude * y + (left_x >> 1)];
                        Map *center = &MapTiles[
                            MapLongitude * y + (center_x >> 1)];
                        if ((left->climate >> 5) >= (center->climate >> 5)
                            && step_west <= static_cast<int>(MapCloudCover))
                            left->unk_1 &= 0xEF;
                    }
                    west -= 2;
                }

                const int east_x = world_rainfall_xrange(
                    x + 2, MapLongitudeBounds, MapIsFlat);
                if (east_x >= 0 && east_x < MapLongitudeBounds)
                    MapTiles[MapLongitude * y + (east_x >> 1)].unk_1 |= 0x80;

                const int x4_cloud_cover = static_cast<int>(MapCloudCover);
                if (x4_cloud_cover > 0) {
                    const int x4 = world_rainfall_xrange(
                        x + 4, MapLongitudeBounds, MapIsFlat);
                    if (x4 >= 0 && x4 < MapLongitudeBounds) {
                        Map *tile4 = &MapTiles[
                            MapLongitude * y + (x4 >> 1)];
                        if ((tile4->climate & 7) > 1 || x4_cloud_cover > 1) {
                            tile4->unk_1 |= 0x80;
                            if (static_cast<int>(MapCloudCover) > 1) {
                                const int x6 = world_rainfall_xrange(
                                    x + 6, MapLongitudeBounds, MapIsFlat);
                                if (x6 >= 0 && x6 < MapLongitudeBounds
                                    && (MapTiles[MapLongitude * y + (x6 >> 1)]
                                        .climate & 7) != 0)
                                    MapTiles[MapLongitude * y + (x6 >> 1)]
                                        .unk_1 |= 0x80;
                            }
                        }
                    }
                }
            }

            // Native 0x5C45AD's jge reaches this shared terrain-class block,
            // not the end of the low-altitude ocean/cloud-hint block above.
            const int altitude = tile->climate >> 5;
            if (altitude <= 4) {
                if (altitude >= 3 && (tile->bit & BIT_FOREST)) {
                    for (int row_forest = y - 1; row_forest <= y + 1;
                         ++row_forest) {
                        const int nx = world_rainfall_xrange(
                            x + 1, MapLongitudeBounds, MapIsFlat);
                        if (row_forest >= 0 && row_forest < MapLatitudeBounds
                            && nx >= 0 && nx < MapLongitudeBounds)
                            MapTiles[MapLongitude * row_forest + (nx >> 1)]
                                .unk_1 |= 0x40;
                    }
                    const int nx2 = world_rainfall_xrange(
                        x + 2, MapLongitudeBounds, MapIsFlat);
                    if (nx2 >= 0 && nx2 < MapLongitudeBounds)
                        MapTiles[MapLongitude * y + (nx2 >> 1)]
                            .unk_1 |= 0x40;
                }
            } else {
                const int cloudmass = static_cast<int>(
                    altitude <= 5 ? WorldBuilder.cloudmass_hills
                                  : WorldBuilder.cloudmass_peaks);
                int west_limit = (static_cast<int>(MapCloudCover) - 1)
                    * static_cast<int>(WorldBuilder.rainfall_coeff)
                    + cloudmass;
                if (west_limit >= 1) {
                    int col = x - 2;
                    for (int distance_west = -1;
                         distance_west >= -west_limit; --distance_west) {
                        const int nx = world_rainfall_xrange(
                            col, MapLongitudeBounds, MapIsFlat);
                        if (nx >= 0 && nx < MapLongitudeBounds) {
                            Map *neighbor = &MapTiles[
                                MapLongitude * y + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) < 0x60) {
                                break;
                            }
                            if ((neighbor->climate >> 5) >= altitude) {
                                break;
                            }
                            neighbor->unk_1 |= 0x20;
                        }
                        col -= 2;
                        west_limit = (static_cast<int>(MapCloudCover) - 1)
                            * static_cast<int>(WorldBuilder.rainfall_coeff)
                            + cloudmass;
                    }
                }

                int east_limit = cloudmass
                    - (static_cast<int>(MapCloudCover) - 1)
                      * static_cast<int>(WorldBuilder.rainfall_coeff);
                if (east_limit >= 1) {
                    int col = x + 2;
                    for (int distance_east = 1;
                         distance_east <= east_limit; ++distance_east) {
                        const int nx = world_rainfall_xrange(
                            col, MapLongitudeBounds, MapIsFlat);
                        if (nx >= 0 && nx < MapLongitudeBounds) {
                            Map *neighbor = &MapTiles[
                                MapLongitude * y + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) < 0x60)
                                break;
                            if ((neighbor->climate >> 5) >= altitude)
                                break;
                            neighbor->unk_1 |= 0x10;
                        }
                        col += 2;
                        east_limit = cloudmass
                            - (static_cast<int>(MapCloudCover) - 1)
                              * static_cast<int>(WorldBuilder.rainfall_coeff);
                    }
                }
            }
        }
        do_all_non_input();
    }

    world_rainfall_pass2(saved_faction);

    for (y = 0; y < MapLatitudeBounds; ++y) {
        int x = y & 1;
        if (x < MapLongitudeBounds) {
            for (; x < MapLongitudeBounds; x += 2) {
                Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];
                if ((tile->climate & 0x18) == 0) {
                    for (int dir_neighbor = 0; dir_neighbor < 8;
                         ++dir_neighbor) {
                        const int nx = world_rainfall_xrange(
                            x + VehMoveDeltaX[dir_neighbor], MapLongitudeBounds,
                            MapIsFlat);
                        const int ny = y + VehMoveDeltaY[dir_neighbor];
                        if (ny < 0 || ny >= MapLatitudeBounds
                            || nx < 0 || nx >= MapLongitudeBounds)
                            continue;
                        const Map *neighbor = &MapTiles[
                            MapLongitude * ny + (nx >> 1)];
                        if ((neighbor->climate & 0xE0) < 0x60)
                            continue;
                        if ((neighbor->climate & 0x18) == 0x10) {
                            climate_set(x, y, 1);
                            break;
                        }
                    }
                }
            }
        }
        do_all_non_input();
    }
}

