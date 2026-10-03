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
#include <mmsystem.h>
#include "temp.h"
#include "xpops.h"  // X_pop
#include "game.h"
#include "alpha.h"
#include "base.h"
#include "basepop.h"  // BasePopDefaultField322C
#include "buffer.h"  // g_PCX_PARSE_TEMP_BUFFER1
#include "graphicwin.h"  // opening window's Buffer base and virtual update
#include "sprite.h"  // Sprite::extract
#include "texture.h" // Terrain-sheet texture assets.
#include "basewin.h"  // BaseWin::close
#include "console.h"  // ConsoleGlobal, ConsoleStatusWin
#include "designwin.h"  // DesignWin::close
#include "faction.h"
#include "fx.h"  // g_FX
#include "general.h"
#include "filebox.h"
#include "lock.h"
#include "map.h"
#include "main.h"  // CommandLineText
#include "mapwin.h" // draw_tile, mapwin_system_shutdown
#include "hypothesis_layouts.h"  // MultiWin, for MultiWindow
#include "log.h"  // log_set_state
#include "random.h"  // random_reseed
#include "battlewin.h"  // the standing battle window
#include "datalink.h"  // DatalinkMain
#include "messagewin.h"  // the standing MessageWin
#include "multidebug.h"  // MultiDebug, the standing debug window
#include "netdaemon.h"  // NetDaemonNet
#include "statuswin.h"  // StatusWin::close
#include "strings.h"  // StringTable
#include "text.h"  // text_open/text_get/text_item, TextBufferGetPtr
#include "time.h"  // stop_timers
#include "tutwin.h"  // TutWinDesWindow, TutWinBaseWindow, TutWinIfaceWindow
#include "veh.h"
#include "wave.h"  // g_WAVE_GENERAL
#include "worldwin.h"  // WorldClimateWorldWin
#include "diplowin.h"  // DiploWin::exec, homed here out of the archive
#include "filemap.h"
#include "palette.h"
#include "caviar.h"
#include "leaf_recoveries.h"
#include "newtechwin.h"  // Existing technology-popup sprite assets.
#include "infowin.h"  // Shared faction text/shadow colors.
#include "maininterface.h"  // Shared chat foreground/shadow colors.
#include "pushbutton.h"
#include "flatbutton.h"
#include "council.h"
#include "replaywin.h"
#include "technology.h"
#include "path.h"
#include "alphasave.h"
#include "netwin.h"

extern int __cdecl def_caption_draw(GraphicWin *, char *, Buffer *);

// RECOVERY_OBJECT: v1 0x006846B0 ?InterfaceStandardPopupBoxStyle@@3UBoxSpriteParams@@A
BoxSpriteParams InterfaceStandardPopupBoxStyle = {
    &g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE,
    &g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE,
    &g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE,
    &g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE,
    &g_IFACE_STD_POPUPS_TOP_MID_SPRITE,
    &g_IFACE_STD_POPUPS_BOT_MID_SPRITE,
    &g_IFACE_STD_POPUPS_MID_LEFT_SPRITE,
    &g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE,
    &g_IFACE_STD_POPUPS_MIDDLE_BUFFER,
    7
};

// The rainfall loader publishes three rows of four colors, one byte after
// each get_pixel. The coast renderer consumes row 1 forward or backward.
// RECOVERY_OBJECT: v2 0x00787E54 0x00787E60 ?TerrainCoastPalette@@3PAEA 0x00451C50
unsigned char TerrainCoastPalette[12];

// Original 452258 walks exactly 53 palette indices. No writer or pointer
// escape was observed; const is that bounded interpretation, not a claim
// about arbitrary computed addresses. Irregular image data stays in the
// pinned, ignored configure-time include, never a second source ledger.
// RECOVERY_OBJECT: v2 0x006846F8 0x0068472D _TerrainCoastRemapSource 0x00451C50
static const unsigned char TerrainCoastRemapSource[] = {
#include "terrain_coast_remap.inc"
};
static_assert(sizeof(TerrainCoastRemapSource) == 53, "terrain remap extent");

namespace {
void terrain_extract_texture_grid(Texture *textures, int count, int columns,
                                  int first_x, int first_y) {
    for (int index = 0; index < count; ++index) {
        textures[index].extract(&g_PCX_PARSE_TEMP_BUFFER1,
            first_x + (index % columns) * 57,
            first_y + (index / columns) * 57, 56, 56, 0);
    }
}

void terrain_extract_sprite(Sprite &sprite, int x, int y, int height) {
    // Both recolors occur before EVERY extraction, even if earlier calls
    // already replaced those colors. Preserve callback timing and failures.
    g_PCX_PARSE_TEMP_BUFFER1.change_color(263, 265);
    g_PCX_PARSE_TEMP_BUFFER1.change_color(262, 265);
    sprite.extract(&g_PCX_PARSE_TEMP_BUFFER1, 265, x, y, 100, height, 0);
}
}

// ORIGINAL: 0x00451C50 ?load_terrain@@YAXXZ 0x00451C50-0x0045370C;0x006550F9-0x0065513A
// RECOVERY_BINDING: v1 {"evidence": ["Body451C50..45370C and cleanup6550F9..65513A construct two four-Sprite arrays then a Buffer, with reverse destruction and partial-construction unwind. The unused first array remains real. CRT initializer/exit evidence beside asset definitions determines their counts and extents, not adjacency.", "Original451CD2 sets TextureRasterKey before the first load;452BB7 reinitializes scratch100x62 after TER1.PCX. Both correct retained-seed omissions/order. All service results are ignored; no rollback or early return is invented.", "Five sheets drive163 Texture extracts,101 Sprite extracts and198 repeated recolors. Twelve get_pixel results publish their low bytes immediately into TerrainCoastPalette. Two conversions call the genuine five-argument source-remap Sprite::draw5E9D44, reading its key after scratch init/fill.", "Original452258 reads the bounded53-byte remap input6846F8..68472D. No writer/escape was observed; const is a bounded interpretation. Source owns its mapping and type; CMake materializes only pinned data into ignored build context. No runtime access to the old executable is used.", "Rocky receiver order0,2,1,3; fungus banks15+15; wreck/fossil/nexus/resource/pod permutations follow original calls. Source grouping is not proof of the historical TU or cross-TU CRT order. Buffer::change_color and other existing callees retain separate structural debt. Preserve full seed src/unrecovered/00451c50.cpp; no byte or runtime acceptance is claimed."], "reason": "Recover the complete terrain-sheet loader through genuine Texture, Sprite and Buffer owners, preserving asset ordering, shared palette publication and C++ lifetime."}
void __cdecl load_terrain() {
    // Original constructs both four-Sprite arrays, even though the first
    // array is unused and only the first element of the second is extracted.
    // Native C++ owns successful construction, partial unwinds, and teardown
    // in reverse order: local buffer, remapped[], unused[].
    Sprite unused[4];
    Sprite remapped[4];
    Buffer scratch;

    // Native pooled literals are local to this TU; load_icons' identically
    // spelled wreck-sheet literal has a distinct original identity.
    // RECOVERY_LITERAL: v1 0x00684FBC 0x00684FC8 ??_C@_0M@DGKG@TEXTURE?4PCX?$AA@ 0x00451C50
    // RECOVERY_LITERAL: v1 0x00684FC8 0x00684FD5 ??_C@_0N@CDGJ@RAINFALL?4PCX?$AA@ 0x00451C50
    // RECOVERY_LITERAL: v1 0x00684FD8 0x00684FE1 ??_C@_08EOGP@glow?4pcx?$AA@ 0x00451C50
    // RECOVERY_LITERAL: v1 0x00684FE4 0x00684FF2 ??_C@_0O@CAND@TER1WRECK?4PCX?$AA@ 0x00451C50
    // RECOVERY_LITERAL: v1 0x00684FF4 0x00684FFD ??_C@_08BFGA@TER1?4PCX?$AA@ 0x00451C50
    TextureRasterKey = 9;  // Before the first load, not afterward as in seed.
    g_PCX_PARSE_TEMP_BUFFER1.load_pcx("TEXTURE.PCX", 0, 10, 236);
    scratch.init(28, 28, 0, 0);
    g_RADIUS1_TEXTURE.extract(&g_PCX_PARSE_TEMP_BUFFER1, 1, 571, 56, 56, 0);
    g_RADIUS2_TEXTURE.extract(&g_PCX_PARSE_TEMP_BUFFER1, 58, 571, 56, 56, 0);
    {
        const int order[4] = {0, 2, 1, 3};
        for (int n = 0; n < 4; ++n) {
            g_ROCKY_TEXTURES[order[n]].extract(
                &g_PCX_PARSE_TEMP_BUFFER1, 1 + n * 57, 1, 56, 56, 0);
        }
    }
    terrain_extract_texture_grid(g_OCEAN_TEXTURES, 2, 1, 280, 79);
    g_FLAT_ARID_LAND_TEXTURE.extract(&g_PCX_PARSE_TEMP_BUFFER1, 1, 58, 56, 56, 0);
    terrain_extract_texture_grid(g_MOIST_LAND_TEXTURES, 16, 4, 1, 115);
    terrain_extract_texture_grid(g_RAINY_LAND_TEXTURES, 16, 4, 1, 343);
    terrain_extract_texture_grid(g_JUNGLE_LAND_TEXTURES, 15, 4, 526, 259);
    g_DUNE_LAND_TEXTURE.extract(&g_PCX_PARSE_TEMP_BUFFER1, 229, 1, 56, 56, 0);
    terrain_extract_texture_grid(g_SUNNY_MESA_TEXTURES, 8, 4, 768, 15);
    terrain_extract_texture_grid(g_ROAD_TEXTURES, 9, 3, 775, 395);
    terrain_extract_texture_grid(g_MAGTUBE_TEXTURES, 9, 3, 775, 566);
    terrain_extract_texture_grid(g_RIVER_TEXTURES, 16, 4, 280, 259);
    terrain_extract_texture_grid(g_MOUNT_PLANET_TEXTURES, 3, 3, 1, 628);
    terrain_extract_texture_grid(g_GARLAND_CRATER_TEXTURES, 3, 3, 1, 685);
    // Each 4x4 grid skips its final cell; the second bank starts at15, not16.
    terrain_extract_texture_grid(g_FUNGUS_TEXTURES, 15, 4, 280, 516);
    terrain_extract_texture_grid(g_FUNGUS_TEXTURES + 15, 15, 4, 508, 516);
    terrain_extract_texture_grid(g_FARM_TEXTURES, 9, 3, 775, 219);
    terrain_extract_texture_grid(g_FOREST_TEXTURES, 16, 4, 526, 6);

    g_PCX_PARSE_TEMP_BUFFER1.load_pcx("RAINFALL.PCX", 0, 10, 236);
    terrain_extract_sprite(g_RAINFALL_DOUBLE_TILE_SPRITES[0], 1, 52, 100);
    remapped[0].extract(&g_PCX_PARSE_TEMP_BUFFER1, 265, 2, 333, 113, 56, 0);
    {
        for (int row = 0; row < 3; ++row) {
            for (int column = 0; column < 4; ++column) {
                // Each byte is published immediately after its get_pixel,
                // before the next callback. No delayed local palette copy.
                TerrainCoastPalette[row * 4 + column] =
                    static_cast<unsigned char>(g_PCX_PARSE_TEMP_BUFFER1.get_pixel(
                        130 + column * 14, 344 + row * 17));
            }
        }
    }
    unsigned char remap[256];
    memset(remap, 0, sizeof(remap));
    {
        for (int index = 0; index < 53; ++index) {
            const unsigned char color = static_cast<unsigned char>(
                TerrainCoastRemapSource[index] + 10);
            remap[color] = static_cast<unsigned char>(index);
        }
    }
    scratch.init(170, 60, 0, 0);
    remapped[0].draw(&scratch, remapped[0].transparent_index(), 0, 0, remap);
    g_RAINFALL_SINGLE_TILE_TEXTURES[0].extract(&scratch, 0, 0, 56, 56, 0);
    g_RAINFALL_SINGLE_TILE_TEXTURES[1].extract(&scratch, 57, 0, 56, 56, 0);
    const unsigned char nine_colors[9] = {211,208,41,143,116,151,206,95,192};
    remapped[0].extract(&g_PCX_PARSE_TEMP_BUFFER1, 265, 2, 254, 60, 60, 0);
    memset(remap, 0, sizeof(remap));
    {
        for (int index = 0; index < 9; ++index) {
            remap[static_cast<unsigned char>(nine_colors[index] + 10)] =
                static_cast<unsigned char>(index);
        }
    }
    scratch.fill(9);
    remapped[0].draw(&scratch, remapped[0].transparent_index(), 0, 0, remap);
    g_RAINFALL_SINGLE_TILE_TEXTURE.extract(&scratch, 0, 0, 60, 60, 0);

    g_PCX_PARSE_TEMP_BUFFER1.load_pcx("glow.pcx", 0, 0, 256);
    terrain_extract_sprite(g_GLOW_SPRITES[0], 1, 1, 62);
    terrain_extract_sprite(g_GLOW_SPRITES[1], 102, 1, 62);
    g_PCX_PARSE_TEMP_BUFFER1.load_pcx("TER1WRECK.PCX", 0, 10, 236);
    {
        const int order[15] = {13,10,14,6,7,8,5,0,1,4,3,2,12,9,11};
        for (int n = 0; n < 15; ++n) {
            terrain_extract_sprite(g_TER1WRECK_UNITY_WRECKAGE_SPRITES[order[n]],
                26 + (n % 3) * 101, 21 + (n / 3) * 63, 62);
        }
    }
    {
        for (int n = 0; n < 4; ++n) {
            terrain_extract_sprite(g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES[n],
                329 + (n % 2) * 101, 147 + (n / 2) * 63, 62);
        }
    }
    {
        const int order[6] = {5,0,1,4,3,2};
        for (int n = 0; n < 6; ++n) {
            terrain_extract_sprite(g_FOSSIL_FIELD_RIDGE_SPRITES[order[n]],
                329 + (n % 3) * 101, 21 + (n / 3) * 63, 62);
        }
    }
    g_PCX_PARSE_TEMP_BUFFER1.load_pcx("TER1.PCX", 0, 10, 236);
    scratch.init(100, 62, 0, 0); // 452BB7; omitted by the retained seed.
    {
        for (int n = 0; n < 3; ++n) {
            terrain_extract_sprite(g_TER1_WHITE_ORG_YEL_TILE_SPRITES[n * 2],
                1 + n * 101, 447, 62);
            terrain_extract_sprite(g_TER1_WHITE_ORG_YEL_TILE_SPRITES[n * 2 + 1],
                1 + n * 101, 579, 62);
        }
    }
    {
        for (int n = 0; n < 3; ++n) {
            terrain_extract_sprite(g_TER1_BOTTOM_LEFT_TILE_SPRITES[n * 3],
                1 + n * 101, 510, 62);
            terrain_extract_sprite(g_TER1_BOTTOM_LEFT_TILE_SPRITES[n * 3 + 1],
                1 + n * 101, 642, 62);
            terrain_extract_sprite(g_TER1_BOTTOM_LEFT_TILE_SPRITES[n * 3 + 2],
                1 + n * 101, 705, 62);
        }
    }
    terrain_extract_sprite(g_TER1_UNUSED_SPRITES2[1], 506, 1, 62);
    terrain_extract_sprite(g_TER1_MINE_SPRITE, 607, 64, 62);
    terrain_extract_sprite(g_TER1_MINING_PLATFORM_SPRITE, 506, 64, 62);
    terrain_extract_sprite(g_TER1_SOLAR_COLLECTOR_SPRITE, 607, 127, 62);
    terrain_extract_sprite(g_TER1_TIDAL_HARNESS_SPRITE, 506, 127, 62);
    terrain_extract_sprite(g_TER1_TUT_BLANK_SPRITE, 506, 190, 62);
    terrain_extract_sprite(g_TER1_KELP_FARM_SPRITE, 607, 190, 62);
    terrain_extract_sprite(g_TER1_BUNKER_SPRITE, 506, 316, 62);
    terrain_extract_sprite(g_TER1_AIRBASE_SPRITE, 607, 316, 62);
    terrain_extract_sprite(g_TER1_SENSOR_ARRAY_SPRITE, 708, 316, 62);
    {
        for (int n = 0; n < 5; ++n) {
            terrain_extract_sprite(g_TER1_UNUSED_SPRITES1[n], 620, 453 + n * 63, 62);
            terrain_extract_sprite(g_TER1_SOIL_ENRICHER_SPRITES[n], 822, 453 + n * 63, 62);
            terrain_extract_sprite(g_TER1_FARM_SPRITES[n], 923, 453 + n * 63, 62);
        }
    }
    terrain_extract_sprite(g_TER1_CONDENSER_SPRITE, 506, 253, 62);
    terrain_extract_sprite(g_TER1_ECHELON_MIRROR_SPRITE, 607, 253, 62);
    terrain_extract_sprite(g_TER1_BOREHOLE_SPRITE, 708, 253, 62);
    terrain_extract_sprite(g_TER1_BOREHOLE_CLUSTER_SPRITE, 708, 190, 62);
    terrain_extract_sprite(g_TER1_LANDMARK_RESOURCE_SPRITES[4], 822, 253, 62);
    terrain_extract_sprite(g_TER1_LANDMARK_RESOURCE_SPRITES[5], 822, 190, 62);
    {
        const int order[6] = {3,2,4,0,5,1};
        for (int n = 0; n < 6; ++n) {
            terrain_extract_sprite(g_TER1_MANIFOLD_NEXUS_SPRITES[order[n]],
                478 + (n / 2) * 101, 632 + (n % 2) * 63, 62);
        }
    }
    {
        for (int n = 0; n < 3; ++n) {
            terrain_extract_sprite(g_TER1_SEA_LAND_RESOURCE_SPRITES[n * 4 + 2], 1, 253 + n * 63, 62);
            terrain_extract_sprite(g_TER1_SEA_LAND_RESOURCE_SPRITES[n * 4 + 3], 102, 253 + n * 63, 62);
            terrain_extract_sprite(g_TER1_SEA_LAND_RESOURCE_SPRITES[n * 4], 203, 253 + n * 63, 62);
            terrain_extract_sprite(g_TER1_SEA_LAND_RESOURCE_SPRITES[n * 4 + 1], 304, 253 + n * 63, 62);
        }
    }
    {
        for (int n = 0; n < 6; ++n) {
            terrain_extract_sprite(g_TER1_UNITY_POD_SPRITES[n ^ 1], 418 + n * 101, 379, 62);
        }
    }
    terrain_extract_sprite(g_TER1_MONOLITH_SPRITE, 304, 1, 62);
    g_PCX_PARSE_TEMP_BUFFER1.close();
    // Return codes were ignored throughout. No early return/rollback is added.
}

// Mutable panel styles: load_box_sprites fills three columns by seventeen
// rows. The records borrow the actual Sprite and Buffer objects below.
// Original 0x78D528..0x78DD20 is initially zero; it is not a static address table.
// The original loader advances 0x28 per record and stops after 51 buffers.
// RECOVERY_OBJECT: v2 0x0078D528 0x0078DD20 ?InterfaceBoxStyles@@3PAUBoxSpriteParams@@A 0x00453A70
BoxSpriteParams InterfaceBoxStyles[51];

namespace {
void extract_interface_box_sprite(Sprite &sprite, int x, int y, int width, int height) {
    sprite.extract(&g_PCX_PARSE_TEMP_BUFFER1, WinFillColour,
                   x, y, width, height, nullptr);
}
}

// ORIGINAL: 0x00453A70 ?load_box_sprites@@YAXXZ 0x00453A70-0x00453F6F
// RECOVERY_BINDING: v1 {"evidence": ["Original453A70..453F6F and retained src/recovered/units/00453a70.cpp. Original455E35 calls this body from load_interface; that caller remains a separate unfinished startup dependency.", "Original453B43..453D4A advances three columns over seventeen rows, with 0x28-byte records and 0x588-byte mask buffers. Eight sprite pointers precede each current-mask get_pixel call; transparent masks publish null before close, and color7 follows either branch.", "Original453A82 loads interface.pcx. The complete body also extracts leading icons, popup edges/middle, tech arrows and six column-major new-tech icons, then closes the shared source canvas. Ignored return values remain ignored as in the original."], "reason": "The interface asset loader owns publication of 51 mutable shared panel-style records and populates existing Sprite/Buffer assets. Genuine member receivers replace the seed transcription's lost extraction and pixel-test receivers."}
void __cdecl load_box_sprites() {
    Buffer &canvas = g_PCX_PARSE_TEMP_BUFFER1;
    // Original ignores failure and continues the full publication sequence.
    // Original 0x453A82 names this 14-byte NUL-terminated literal.
    // RECOVERY_LITERAL: v1 0x00685014 0x00685022 ??_C@_0O@EGK@interface?4pcx?$AA@ 0x00453A70
    canvas.load_pcx("interface.pcx", 0, 10, 236);
    extract_interface_box_sprite(g_IFACE_LOCK_SPRITES[0], 247, 71, 15, 19);
    extract_interface_box_sprite(g_IFACE_LOCK_SPRITES[1], 263, 71, 15, 19);
    extract_interface_box_sprite(g_IFACE_GREEN_RIGHT_ARROW_SPRITE[0], 615, 530, 16, 15);
    for (int arrow = 0; arrow < 3; ++arrow) {
        extract_interface_box_sprite(g_IFACE_MP_COMBO_ARROW_SPRITES[arrow], 534 + arrow * 8, 537, 7, 13);
    }

    for (int style = 0; style < 51; ++style) {
        const int x = 33 + (style % 3) * 39;
        const int y = 229 + (style / 3) * 39;
        extract_interface_box_sprite(g_IFACE_BOX_SPRITES1[style], x - 32, y, 6, 6);
        extract_interface_box_sprite(g_IFACE_BOX_SPRITES2[style], x, y, 6, 6);
        extract_interface_box_sprite(g_IFACE_BOX_SPRITES3[style], x - 32, y + 32, 6, 6);
        extract_interface_box_sprite(g_IFACE_BOX_SPRITES4[style], x, y + 32, 6, 6);
        extract_interface_box_sprite(g_IFACE_BOX_SPRITES5[style], x - 25, y, 24, 6);
        extract_interface_box_sprite(g_IFACE_BOX_SPRITES6[style], x - 25, y + 32, 24, 6);
        extract_interface_box_sprite(g_IFACE_BOX_SPRITES7[style], x - 32, y + 7, 6, 24);
        extract_interface_box_sprite(g_IFACE_BOX_SPRITES8[style], x, y + 7, 6, 24);

        Buffer &mask = g_IFACE_BOX_SPRITE_BUFFERS[style];
        mask.init(24, 24, 0, nullptr);
        canvas.copy(&mask, x - 25, y + 7, 0, 0, 24, 24);
        InterfaceBoxStyles[style].corner0_ = &g_IFACE_BOX_SPRITES1[style];
        InterfaceBoxStyles[style].corner1_ = &g_IFACE_BOX_SPRITES2[style];
        InterfaceBoxStyles[style].corner2_ = &g_IFACE_BOX_SPRITES3[style];
        InterfaceBoxStyles[style].corner3_ = &g_IFACE_BOX_SPRITES4[style];
        InterfaceBoxStyles[style].edge0_ = &g_IFACE_BOX_SPRITES5[style];
        InterfaceBoxStyles[style].edge1_ = &g_IFACE_BOX_SPRITES6[style];
        InterfaceBoxStyles[style].edge2_ = &g_IFACE_BOX_SPRITES7[style];
        InterfaceBoxStyles[style].edge3_ = &g_IFACE_BOX_SPRITES8[style];
        // This style's Buffer, not canvas. Pointer publication precedes the
        // pixel test; a transparent mask is cleared before close can observe it.
        if (mask.get_pixel(0, 0) == 9) {
            InterfaceBoxStyles[style].mask_ = nullptr;
            mask.close();
        } else {
            InterfaceBoxStyles[style].mask_ = &mask;
        }
        InterfaceBoxStyles[style].mask_color_ = 7;
    }

    extract_interface_box_sprite(g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE, 296, 711, 61, 44);
    extract_interface_box_sprite(g_IFACE_STD_POPUPS_TOP_MID_SPRITE, 358, 711, 61, 44);
    extract_interface_box_sprite(g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE, 420, 711, 61, 44);
    extract_interface_box_sprite(g_IFACE_STD_POPUPS_MID_LEFT_SPRITE, 296, 756, 61, 44);
    extract_interface_box_sprite(g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE, 420, 756, 61, 44);
    extract_interface_box_sprite(g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE, 296, 801, 61, 44);
    extract_interface_box_sprite(g_IFACE_STD_POPUPS_BOT_MID_SPRITE, 358, 801, 61, 44);
    extract_interface_box_sprite(g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE, 420, 801, 61, 44);
    g_IFACE_STD_POPUPS_MIDDLE_BUFFER.init(61, 44, 0, nullptr);
    canvas.copy(&g_IFACE_STD_POPUPS_MIDDLE_BUFFER, 358, 756, 0, 0, 61, 44);

    for (int tech_arrow = 0; tech_arrow < 3; ++tech_arrow) {
        extract_interface_box_sprite(g_IFACE_TECH_TREE_ARROW_SPRITES[tech_arrow], 140 + tech_arrow * 8, 37, 7, 13);
    }
    for (int technology = 0; technology < 6; ++technology) {
        // Unlike the style pool, these six slots are column-major.
        extract_interface_box_sprite(g_NEWTECHWIN_SPRITES[technology],
                                     391 + (technology / 3) * 29,
                                     566 + (technology % 3) * 35, 28, 34);
    }
    canvas.close();
}

// Each row maps every palette index to its row number (original 0x007A81A0).
// RECOVERY_OBJECT: v2 0x007A81A0 0x007A99A0 ?VoxelRowColorTables@@3PAY0BAA@EA 0x00451030
unsigned char VoxelRowColorTables[24][256];

// Original 0x006861F0..0x00686330: mutable palette offsets selected by
// alt_get_ocean_detail's 0..79 result. Configuration replaces only the
// accepted prefix, so these startup values and later untouched entries
// persist across missing, partial and repeated load_cycling calls.
// RECOVERY_OBJECT: v2 0x006861F0 0x00686330 ?OceanDetailShade@@3PAHA 0x00455400
int OceanDetailShade[80] = {
    37, 37, 37, 37, 37, 37, 36, 36, 36, 36,
    35, 35, 35, 35, 35, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    16, 16, 14, 13, 13, 12, 12, 11, 11, 10,
    10, 9, 9, 8, 8, 8, 7, 7, 6, 6,
    5, 5, 4, 4, 3, 3, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// The temporary model retains the fourth record until scope exit, including
// unwinding. A valid record with a different step count leaves its offset alone.
// ORIGINAL: 0x00451030 ?load_voxels@@YAXXZ 0x00451030-0x00451A09;0x00655030-0x00655042
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original133ordered init calls establish model receivers, array indices and filename case. Existing caviar.cpp owners supply the actual objects; no original-address pointer stand-ins.", "Original four temporary records read first-part vectors only when step_count is2; null records zero the output and other counts retain it. Only the first two have explicit close calls; the third is replaced by init and the fourth is destroyed after configuration/table work. Original EH655030 reaches CaviarData destructor456100.", "Original4519BA high multiply0x2AAAAAAB and arithmetic shift2 implement signed divide24. Advancing the numerator24 per row yields24rows numbered0..23; the seed divide3 was incorrect. Light settings are individually converted/stored before in-place normalization."], "reason": "Recover voxel asset loading through genuine model objects, renderer setup, typed record parts and source-owned output tables; retain the temporary model through normal and exceptional scope exit."}
void __cdecl load_voxels() {
    CaviarData scratch;
    g_VEHDRAW_CAVIAR.init();
    vehdraw_setup();
    g_SSF_CAVIARDATA.init("ssf.cvr");
    g_SDP_CAVIARDATA.init("sdp.cvr");
    g_SAS_CAVIARDATA.init("sas.cvr");
    g_SCD_CAVIARDATA.init("scd.cvr");
    g_SCJ_CAVIARDATA.init("scj.cvr");
    g_SAGS_CAVIARDATA.init("sags.cvr");
    g_SFT_CAVIARDATA.init("sft.cvr");
    g_VHR_CAVIARDATA1.init("vhr.cvr");
    g_SHT_CAVIARDATA.init("sht.cvr");
    g_SRB_CAVIARDATA.init("srb.cvr");
    g_ASAS_CAVIARDATA.init("asas.cvr");
    g_RESLASER_CAVIARDATA.init("reslaser.cvr");
    g_RESBOLT_CAVIARDATA.init("resbolt.cvr");
    g_FUNLOAD_CAVIARDATA.init("funload.cvr");
    g_TECLOAD_CAVIARDATA.init("tecload.cvr");
    g_SP_SOPORIFIC_CAVIARDATA.init("SP-SOPORIFIC.cvr");
    g_SP_DISSWAVE_CAVIARDATA.init("SP-DISSWAVE.cvr");
    g_SP_MARINED_CAVIARDATA.init("SP-MARINED.cvr");
    g_SP_NANOO_CAVIARDATA.init("SP-NANOO.cvr");
    g_AA01_CAVIARDATA.init("AA01.cvr");
    g_AA_ROVER_CAVIARDATA.init("AA-Rover.cvr");
    g_AA_WING_CAVIARDATA[0].init("AA-WING0.cvr");
    g_AA_WING_CAVIARDATA[1].init("AA-WING.cvr");
    g_ACP_CAVIARDATA[0].init("ACP00.cvr");
    g_ACP_CAVIARDATA[1].init("ACP01.cvr");
    g_ACP_CAVIARDATA[2].init("ACP02.cvr");
    g_ACP_CAVIARDATA[3].init("ACP03.cvr");
    g_AA_CAVIARDATA.init("AA.cvr");
    g_SSFA_CAVIARDATA.init("SSFA.CVR");
    g_SFTA_CAVIARDATA.init("SFTA.CVR");
    g_VW00_CAVIARDATA.init("vw00.cvr");
    g_VIPTAWL_CAVIARDATA.init("viptawal.cvr");
    g_VIPTASGN_CAVIARDATA.init("viptasgn.cvr");
    g_VIPTAPSI_CAVIARDATA.init("viptapsi.cvr");
    g_VIPTR_CAVIARDATA[0].init("viptr00.cvr");
    g_VIPTR_CAVIARDATA[1].init("viptr01.cvr");
    g_VIPTR_CAVIARDATA[2].init("viptr02.cvr");
    g_VIPTR_CAVIARDATA[3].init("viptr03.cvr");
    g_VIPTA_CAVIARDATA[0].init("vipta00.cvr");
    g_VIPTA_CAVIARDATA[1].init("vipta01.cvr");
    g_PTMOD_CAVIARDATA.init("ptmod.cvr");
    g_VPT_CAVIARDATA.init("vpt.cvr");
    g_A_CAVIARDATA.init("a.cvr");
    g_APWALL_CAVIARDATA.init("apwall.cvr");
    g_APSID_CAVIARDATA.init("apsid.cvr");
    g_ASGEN_CAVIARDATA.init("asgen.cvr");
    g_VB_CAVIARDATA.init("VB.CVR");
    g_VBP_CAVIARDATA.init("VBP.CVR");
    g_VGMC_CAVIARDATA.init("VGMC.CVR");
    g_VGMCP_CAVIARDATA.init("VGMCP.CVR");
    g_VLIGHTS_CAVIARDATA.init("VLIGHTS.CVR");
    g_VA01_CAVIARDATA.init("VA01.cvr");
    g_VR_CAVIARDATA[0].init("VR00.cvr");
    g_VR_CAVIARDATA[1].init("VR01.cvr");
    g_VR_CAVIARDATA[2].init("VR02.cvr");
    g_VR_CAVIARDATA[3].init("VR03.cvr");
    g_VHR_CAVIARDATA2.init("VHR.CVR");
    g_VRC_CAVIARDATA[0].init("VRCP00.cvr");
    g_VRC_CAVIARDATA[1].init("VRCP01.cvr");
    g_VRC_CAVIARDATA[2].init("VRCP02.cvr");
    g_VRC_CAVIARDATA[3].init("VRCP03.cvr");
    g_VI_CAVIARDATA.init("VI.CVR");
    g_VGMT_CAVIARDATA.init("VGMT.CVR");
    g_VGMTP_CAVIARDATA.init("VGMTP.CVR");
    g_VW_CAVIARDATA[0].init("VW01.CVR");
    g_VW_CAVIARDATA[1].init("VW02.CVR");
    g_VW_CAVIARDATA[2].init("VW03.CVR");
    g_VW_CAVIARDATA[3].init("VW04.CVR");
    g_VW_CAVIARDATA[4].init("VW05.CVR");
    g_VW_CAVIARDATA[5].init("VW06.CVR");
    g_VW_CAVIARDATA[6].init("VW07.CVR");
    g_VW_CAVIARDATA[7].init("VW08.CVR");
    g_VW_CAVIARDATA[8].init("VW09.CVR");
    g_VW_CAVIARDATA[9].init("VW10.CVR");
    g_VW_CAVIARDATA[10].init("VW11.CVR");
    g_VW_CAVIARDATA[11].init("VW12.CVR");
    g_VW_CAVIARDATA[13].init("VW14.CVR");
    g_VW_CAVIARDATA[14].init("VW12.CVR");
    g_AX_CAVIARDATA.init("AX.CVR");
    g_ACOLPOD_CAVIARDATA.init("ACOLPOD.CVR");
    g_AT_CAVIARDATA.init("AT.CVR");
    g_VTA_CAVIARDATA.init("VTA.CVR");
    g_ATP_CAVIARDATA.init("ATp.CVR");
    scratch.init("AVGMTGUN.CVR");
    {
        const CvrRecord *record = scratch.record();
        if (!record) {
            VehGunMountOffset.values[0] = 0.0f;
            VehGunMountOffset.values[1] = 0.0f;
            VehGunMountOffset.values[2] = 0.0f;
        } else if (record->step_count_ == 2) {
            const VOX_Vect *vectors = record->parts_[0].vectors_;
            leaf_00628180_redirect(vectors[1].values, vectors[0].values, VehGunMountOffset.values);
        }
    }
    scratch.close();
    g_DROP_CAVIARDATA.init("drop.cvr");
    g_DROPLET_CAVIARDATA.init("droplet.cvr");
    scratch.init("adroper.cvr");
    {
        const CvrRecord *record = scratch.record();
        if (!record) {
            VehDropOffset.values[0] = 0.0f;
            VehDropOffset.values[1] = 0.0f;
            VehDropOffset.values[2] = 0.0f;
        } else if (record->step_count_ == 2) {
            const VOX_Vect *vectors = record->parts_[0].vectors_;
            leaf_00628180_redirect(vectors[1].values, vectors[0].values, VehDropOffset.values);
        }
    }
    scratch.close();
    g_VWNTU_CAVIARDATA.init("vwntu.cvr");
    g_VT_CAVIARDATA.init("vt.cvr");
    g_VCL_CAVIARDATA.init("VCL.cvr");
    g_VCLT00_CAVIARDATA.init("VCLT00.CVR");
    g_VSP_CAVIARDATA.init("VSP.cvr");
    g_VSPA0_CAVIARDATA[0].init("VSPA01.CVR");
    g_VSPA0_CAVIARDATA[1].init("VSPA02.CVR");
    g_VSPTF_CAVIARDATA.init("vsptf.cvr");
    g_VSPTB_CAVIARDATA.init("vsptb.cvr");
    g_VHT_VBP_CAVIARDATA.init("vht-vbp.cvr");
    g_VHTP_CAVIARDATA.init("vhtp.cvr");
    g_VHTA0_CAVIARDATA[0].init("vhta00.cvr");
    g_VHTA0_CAVIARDATA[1].init("vhta01.cvr");
    g_VHTTP_CAVIARDATA.init("vhttp.cvr");
    g_VHTTPA0_CAVIARDATA[0].init("vhtta00.cvr");
    g_VHTTPA0_CAVIARDATA[1].init("vhtta01.cvr");
    scratch.init("AVHT-GunReposition.cvr");
    {
        const CvrRecord *record = scratch.record();
        if (!record) {
            VehHoverGunOffset.values[0] = 0.0f;
            VehHoverGunOffset.values[1] = 0.0f;
            VehHoverGunOffset.values[2] = 0.0f;
        } else if (record->step_count_ == 2) {
            const VOX_Vect *vectors = record->parts_[0].vectors_;
            leaf_00628180_redirect(vectors[1].values, vectors[0].values, VehHoverGunOffset.values);
        }
    }
    scratch.init("AVHT-LightsReposition.cvr");
    {
        const CvrRecord *record = scratch.record();
        if (!record) {
            VehHoverLightsOffset.values[0] = 0.0f;
            VehHoverLightsOffset.values[1] = 0.0f;
            VehHoverLightsOffset.values[2] = 0.0f;
        } else if (record->step_count_ == 2) {
            const VOX_Vect *vectors = record->parts_[0].vectors_;
            leaf_00628180_redirect(vectors[1].values, vectors[0].values, VehHoverLightsOffset.values);
        }
    }
    g_VFL_CAVIARDATA.init("VFL.cvr");
    g_VGS_CAVIARDATA.init("VGS.cvr");
    g_VGSP_CAVIARDATA.init("VGSP.CVR");
    g_VJTP_CAVIARDATA.init("VJTP.CVR");
    g_VJT0_CAVIARDATA[0].init("VJT00.cvr");
    g_VJT0_CAVIARDATA[1].init("VJT01.cvr");
    g_VCU_CAVIARDATA.init("VCU.cvr");
    g_VCUP_CAVIARDATA.init("VCUP.CVR");
    g_VCUA0_CAVIARDATA[0].init("VCUA00.cvr");
    g_VCUA0_CAVIARDATA[1].init("VCUA01.cvr");
    g_VCUW_CAVIARDATA.init("VCUW.CVR");
    g_VCT_CAVIARDATA.init("vct.cvr");
    g_VCTP_CAVIARDATA.init("VCTP.CVR");
    g_VCT0_CAVIARDATA[0].init("vct00.cvr");
    g_VCT0_CAVIARDATA[1].init("vct01.cvr");
    g_VCTB_CAVIARDATA.init("VCTB.CVR");
    g_VWNTT_CAVIARDATA.init("VWNTT.CVR");
    g_VWNST_CAVIARDATA.init("VWNST.CVR");
    g_VWNAA_CAVIARDATA.init("VWNAA.CVR");
    g_VPBR0_CAVIARDATA[0].init("vpbr00.cvr");
    g_VPBR0_CAVIARDATA[1].init("vpbr01.cvr");
    g_VPBR0_CAVIARDATA[2].init("vpbr02.cvr");
    g_VPBR0_CAVIARDATA[3].init("vpbr03.cvr");
    g_VM_CAVIARDATA.init("VM.CVR");
    g_VM13_CAVIARDATA.init("vw13.cvr");
    g_NW_CAVIARDATA.init("nw.CVR");
    g_NI_CAVIARDATA.init("ni.CVR");
    g_NLC_CAVIARDATA.init("nlc.cvr");
    if (text_open("system", "VEH_LIGHT_DIR_MAP") == 0) {
        text_get();
        VehLightDirMap.values[0] = (float)text_item_number();
        VehLightDirMap.values[1] = (float)text_item_number();
        VehLightDirMap.values[2] = (float)text_item_number();
        vox_normalise_vector(VehLightDirMap.values, VehLightDirMap.values);
    }
    // The original signed divide-by-24 advances its numerator by 24 per row.
    for (int row = 0; row < 24; ++row) {
        for (int color = 0; color < 256; ++color) {
            VoxelRowColorTables[row][color] = (unsigned char)row;
        }
    }
}

char PronounBuffer[256];  // 0x00945838 - see game.h for the naming evidence


// RECOVERY_OBJECT: v1 0x009A6488 ?ExpansionEnabled@@3HA
BOOL ExpansionEnabled;  // 0x009A6488
uint32_t GamePreferences;  // 0x009A6490

uint32_t SystemInterfaceFlags;  // 0x00933420

// colortables_init loads 256 palette indices from shadow.tmp or builds the
// same remap with Palette::create_table(0, 256, -43), then caches all 256 bytes.
// RECOVERY_OBJECT: v2 0x006F107C 0x006F117C ?ShadowColorTable@@3PAEA 0x004BF8D0
unsigned char ShadowColorTable[256];
// Original 0x00423570 loads or writes these complete cache objects. Each
// table starts as zero-filled storage in the pinned image. The tint rows
// consume the model record's palette pointer, not memory after the record.
// RECOVERY_OBJECT: v2 0x006EEF7C 0x006F077C ?ArmorTransparentColorTables@@3PAY0BAA@EA 0x00423570
unsigned char ArmorTransparentColorTables[24][256];
// RECOVERY_OBJECT: v2 0x006F077C 0x006F0F7C ?TerritoryColorTables@@3PAY0BAA@EA 0x00423570
unsigned char TerritoryColorTables[8][256];
// RECOVERY_OBJECT: v2 0x006F0F7C 0x006F107C ?ReddishColorTable@@3PAEA 0x00423570
unsigned char ReddishColorTable[256];
// RECOVERY_OBJECT: v2 0x006F117C 0x006F127C ?RedTintColorTable@@3PAEA 0x00423570
unsigned char RedTintColorTable[256];
// RECOVERY_OBJECT: v2 0x006F127C 0x006F1A7C ?FadeColorTables@@3PAY0BAA@EA 0x00423570
unsigned char FadeColorTables[8][256];
// RECOVERY_OBJECT: v2 0x006F1A7C 0x006F1B7C ?BlueTintColorTable@@3PAEA 0x00423570
unsigned char BlueTintColorTable[256];
// RECOVERY_OBJECT: v2 0x006F1B7C 0x006FEB7C ?ExplosionColorTables@@3PAY1BA@BAA@EA 0x00423570
unsigned char ExplosionColorTables[13][16][256];
// RECOVERY_OBJECT: v2 0x006FEB7C 0x006FEC7C ?SpotlightColorTable@@3PAEA 0x00423570
unsigned char SpotlightColorTable[256];
// RECOVERY_OBJECT: v2 0x00788250 0x00789A50 ?ConstantColorTables@@3PAY0BAA@EA 0x00423570
unsigned char ConstantColorTables[24][256];
// RECOVERY_OBJECT: v2 0x00792664 0x00798664 ?ArmorTintColorTables@@3PAY1BI@BAA@EA 0x00423570
unsigned char ArmorTintColorTables[4][24][256];
// RECOVERY_OBJECT: v2 0x0079A7EC 0x007A67EC ?BodyTintColorTables@@3PAY1BI@BAA@EA 0x00423570
unsigned char BodyTintColorTables[8][24][256];

// The original compares exactly 256 PALETTEENTRYs. Existing mappings are
// writable: a changed palette overwrites the mapped cache before closing it.
// ORIGINAL: 0x00422F20 ?compare_palettes@@YAHXZ 0x00422F20-0x00422FFA;0x00652BD0-0x00652BE2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original primary and EH spans establish operations, local destruction order, standing receiver and mapped cache branches.", "Original bounded row loops and cache sizes establish array extents; model record palette pointer is independently owned storage, not a record-relative address.", "Original paths and dependency-doubled behavioral tests cover cache reuse and invalidation, tint results, parser sequencing and exceptional cleanup."], "reason": "Recover the palette cache and table producer with real Filemap, Buffer and Palette lifetimes, standing palette and model owners, and typed independently owned remap arrays. Historical seeds remain evidence and restoration obligations, not borrowed product claims."}
// This routine calls CRT memcpy in the original. Keep that call boundary
// without cast-call scaffolding; restore the TU's intrinsic policy afterward.
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int __cdecl compare_palettes() {
    Filemap file;
    void *cache = file.open("palcheck.tmp", 1);
    if (cache) {
        if (memcmp(cache, g_PALETTE1.entries(), 256 * sizeof(PALETTEENTRY)) == 0) {
            return 0;
        }
        memcpy(cache, g_PALETTE1.entries(), 256 * sizeof(PALETTEENTRY));
    } else {
        cache = file.create("palcheck.tmp", 256 * sizeof(PALETTEENTRY), 1);
        if (cache) {
            memcpy(cache, g_PALETTE1.entries(), 256 * sizeof(PALETTEENTRY));
        }
    }
    file.close();
    return 1;
}

// ORIGINAL: 0x00423570 ?colortables_init@@YAXXZ 0x00423570-0x0042439F;0x00652C0A-0x00652C32
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original primary and EH spans establish operations, local destruction order, standing receiver and mapped cache branches.", "Original bounded row loops and cache sizes establish array extents; model record palette pointer is independently owned storage, not a record-relative address.", "Original paths and dependency-doubled behavioral tests cover cache reuse and invalidation, tint results, parser sequencing and exceptional cleanup."], "reason": "Recover the palette cache and table producer with real Filemap, Buffer and Palette lifetimes, standing palette and model owners, and typed independently owned remap arrays. Historical seeds remain evidence and restoration obligations, not borrowed product claims."}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif
void __cdecl colortables_init() {
    // These are genuine local lifetimes even though all remap operations use
    // the standing palette. Normal and EH tails destroy Buffer, Filemap,
    // then Palette (original 0x00424361 and 0x00652C0A..0x00652C28).
    Palette local_palette;
    Filemap file;
    int body_colors[8] = {96, 105, 156, 0, 85, 47, 89, 227};
    int armor_colors[4] = {0, 0, 0, 0};
    int transparent_color = 0;
    int transparent_first = 50;
    int transparent_second = 100;
    int territory_colors[8] = {96, 105, 156, 0, 85, 47, 89, 227};
    int territory_weights[8] = {80, 80, 80, 80, 80, 80, 80, 80};
    // Required sections: the original does not initialize these arrays if
    // their text_open fails. Missing/malformed required configuration is not
    // a supported recovery path; do not invent a deterministic zero fallback.
    int energy_colors[3];
    int energy_weights[3];
    int explosion_colors[13][16];
    int explosion_weights[13][16];
    Buffer buffer;
    bool palette_changed = compare_palettes() != 0;
    unsigned char *basic_tables[6] = {ShadowColorTable, SpotlightColorTable,
        ReddishColorTable, RedTintColorTable, BlueTintColorTable,
        reinterpret_cast<unsigned char *>(FadeColorTables)};
    const char *basic_names[6] = {"shadow.tmp", "spotlight.tmp", "reddish.tmp",
        "redtint.tmp", "bluetint.tmp", "fade.tmp"};
    const int basic_sizes[6] = {256, 256, 256, 256, 256, 2048};
    if (!palette_changed) {
        for (int basic = 0; basic < 6; ++basic) {
            void *cache = file.open(basic_names[basic], 1);
            if (!cache) {
                palette_changed = true;
                break;
            }
            memcpy(basic_tables[basic], cache, basic_sizes[basic]);
        }
    }
    if (palette_changed) {
        g_PALETTE1.create_table(ShadowColorTable, 0, 256, -43);
        g_PALETTE1.create_table(SpotlightColorTable, 0, 256, 50);
        g_PALETTE1.create_table_from_color(131, ReddishColorTable, 0, 256, 75, 50);
        g_PALETTE1.create_table_from_color(34, RedTintColorTable, 0, 256, 20, 80);
        g_PALETTE1.create_table_from_color(179, BlueTintColorTable, 0, 256, 20, 80);
        for (int fade = 0; fade < 8; ++fade) {
            g_PALETTE1.create_table_from_color(255, FadeColorTables[fade], 0, 256, 100, 100 - 12 * fade);
        }
        for (int basic = 0; basic < 6; ++basic) {
            void *cache = file.create(basic_names[basic], basic_sizes[basic], 1);
            if (cache) memcpy(cache, basic_tables[basic], basic_sizes[basic]);
        }
    }

    if (text_open("system", "energyglow") == 0) {
        for (int energy = 0; energy < 3; ++energy) {
            text_get();
            energy_colors[energy] = text_item_number();
            energy_weights[energy] = text_item_number();
        }
    }
    for (int explosion = 0; explosion < 13; ++explosion) {
        char number[16];
        StringTemp[0] = '\0';
        strcat(StringTemp, "explosions");
        _itoa(explosion, number, 10);
        strcat(StringTemp, number);
        if (text_open("system", StringTemp) == 0) {
            for (int row = 0; row < 16; ++row) {
                text_get();
                explosion_colors[explosion][row] = text_item_number();
                explosion_weights[explosion][row] = text_item_number();
            }
        }
    }
    if (text_open("system", "VEHICLECOLORING") == 0) {
        for (int body = 0; body < 8; ++body) {
            text_get();
            // Original adds ten to AL, not a potentially overflowing int.
            body_colors[body] = (static_cast<unsigned int>(text_item_number()) + 10u) & 255u;
        }
    }
    if (text_open("system", "ARMORCOLORING") == 0) {
        for (int armor = 0; armor < 4; ++armor) {
            text_get();
            armor_colors[armor] = (static_cast<unsigned int>(text_item_number()) + 10u) & 255u;
        }
    }
    if (text_open("system", "ARMORCOLORING_TRANS") == 0) {
        text_get();
        transparent_color = (static_cast<unsigned int>(text_item_number()) + 10u) & 255u;
        transparent_first = text_item_number();
        transparent_second = text_item_number();
    }
    if (text_open("system", "boundarypercents") == 0) {
        for (int territory = 0; territory < 8; ++territory) {
            text_get();
            territory_weights[territory] = text_item_number();
        }
    }
    buffer.load_pcx("palette.PCX", 0, 10, 236);
    for (int territory = 0; territory < 8; ++territory) {
        territory_colors[territory] = buffer.get_pixel(580 + 8 * territory, 56);
    }

    unsigned char *cached = static_cast<unsigned char *>(file.open("colorvals.tmp", 1));
    bool header_matches = cached != 0 && !palette_changed;
    if (header_matches) {
        header_matches = memcmp(energy_colors, cached, sizeof(energy_colors)) == 0;
        cached += sizeof(energy_colors);
        if (header_matches) header_matches = memcmp(energy_weights, cached, sizeof(energy_weights)) == 0;
        cached += sizeof(energy_weights);
        for (int explosion = 0; explosion < 13 && header_matches; ++explosion) {
            header_matches = memcmp(explosion_colors[explosion], cached, sizeof(explosion_colors[explosion])) == 0;
            cached += sizeof(explosion_colors[explosion]);
            if (header_matches) header_matches = memcmp(explosion_weights[explosion], cached, sizeof(explosion_weights[explosion])) == 0;
            cached += sizeof(explosion_weights[explosion]);
        }
        if (header_matches) header_matches = memcmp(body_colors, cached, sizeof(body_colors)) == 0;
        cached += sizeof(body_colors);
        if (header_matches) header_matches = memcmp(territory_weights, cached, sizeof(territory_weights)) == 0;
        cached += sizeof(territory_weights);
        if (header_matches) header_matches = memcmp(territory_colors, cached, sizeof(territory_colors)) == 0;
        cached += sizeof(territory_colors);
        if (header_matches) header_matches = memcmp(armor_colors, cached, sizeof(armor_colors)) == 0;
        cached += sizeof(armor_colors);
        if (header_matches) header_matches = memcmp(&transparent_color, cached, sizeof(transparent_color)) == 0;
        cached += sizeof(transparent_color);
        if (header_matches) header_matches = memcmp(&transparent_first, cached, sizeof(transparent_first)) == 0;
    }
    if (!header_matches) {
        unsigned char *out = static_cast<unsigned char *>(file.create("colorvals.tmp", 2048, 1));
        if (out) {
            memcpy(out, energy_colors, sizeof(energy_colors)); out += sizeof(energy_colors);
            memcpy(out, energy_weights, sizeof(energy_weights)); out += sizeof(energy_weights);
            for (int explosion = 0; explosion < 13; ++explosion) {
                memcpy(out, explosion_colors[explosion], sizeof(explosion_colors[explosion])); out += sizeof(explosion_colors[explosion]);
                memcpy(out, explosion_weights[explosion], sizeof(explosion_weights[explosion])); out += sizeof(explosion_weights[explosion]);
            }
            memcpy(out, body_colors, sizeof(body_colors)); out += sizeof(body_colors);
            memcpy(out, territory_weights, sizeof(territory_weights)); out += sizeof(territory_weights);
            memcpy(out, territory_colors, sizeof(territory_colors)); out += sizeof(territory_colors);
            memcpy(out, armor_colors, sizeof(armor_colors)); out += sizeof(armor_colors);
            memcpy(out, &transparent_color, sizeof(transparent_color)); out += sizeof(transparent_color);
            memcpy(out, &transparent_first, sizeof(transparent_first)); out += sizeof(transparent_first);
            // Only 0x710 bytes are persisted. The second transparent weight
            // is deliberately absent from the original comparison and cache.
            file.close(out);
        }
    }

    bool regenerate = !header_matches;
    if (header_matches) {
        for (int explosion = 0; explosion < 13; ++explosion) {
            char number[16];
            StringTemp[0] = '\0'; strcat(StringTemp, "explosions");
            _itoa(explosion, number, 10); strcat(StringTemp, number); strcat(StringTemp, ".tmp");
            void *cache = file.open(StringTemp, 1);
            if (!cache) { regenerate = true; break; }
            memcpy(ExplosionColorTables[explosion], cache, sizeof(ExplosionColorTables[explosion]));
            file.close();
        }
    }
    if (regenerate) {
        for (int explosion = 0; explosion < 13; ++explosion) {
            for (int row = 0; row < 16; ++row) {
                const int weight = explosion_weights[explosion][row];
                g_PALETTE1.create_table_from_color(explosion_colors[explosion][row] + 10,
                    ExplosionColorTables[explosion][row], 10, 236, 100 - weight, weight);
            }
            char number[16];
            StringTemp[0] = '\0'; strcat(StringTemp, "explosions");
            _itoa(explosion, number, 10); strcat(StringTemp, number); strcat(StringTemp, ".tmp");
            void *cache = file.create(StringTemp, sizeof(ExplosionColorTables[explosion]), 1);
            memcpy(cache, ExplosionColorTables[explosion], sizeof(ExplosionColorTables[explosion]));
            file.close();
        }
    }

    // Rebuilt explosions force a territory rebuild even if that file exists.
    // A missing territory file alone does not invalidate body/armor tints.
    void *territory_cache = 0;
    if (!regenerate) territory_cache = file.open("territory.tmp", 1);
    if (territory_cache) {
        memcpy(TerritoryColorTables, territory_cache, sizeof(TerritoryColorTables));
        file.close();
    } else {
        unsigned char *out = static_cast<unsigned char *>(file.create("territory.tmp", sizeof(TerritoryColorTables), 1));
        for (int territory = 0; territory < 8; ++territory) {
            const int weight = territory_weights[territory];
            g_PALETTE1.create_table_from_color(territory_colors[territory], TerritoryColorTables[territory], 10, 236, 100 - weight, weight);
            memcpy(out, TerritoryColorTables[territory], sizeof(TerritoryColorTables[territory]));
            out += sizeof(TerritoryColorTables[territory]);
        }
        file.close();
    }

    if (regenerate) {
        for (int body = 0; body < 8; ++body) {
            PALETTEENTRY tint = g_PALETTE1.entries()[body_colors[body]];
            HSV hsv;
            RGB_to_HSV(&tint, &hsv);
            for (int row = 0; row < 24; ++row) {
                const unsigned char *source = g_VBP_CAVIARDATA.record()->palette_ + 0x302 + row * 256;
                for (int color = 0; color < 256; ++color) {
                    const PALETTEENTRY entry = g_PALETTE1.entries()[source[color]];
                    const unsigned char red = (unsigned char)(int)((double)tint.peRed * entry.peRed * (1.0f / 255.0f));
                    const unsigned char green = (unsigned char)(int)((double)tint.peGreen * entry.peGreen * (1.0f / 255.0f));
                    const unsigned char blue = (unsigned char)(int)((double)tint.peBlue * entry.peBlue * (1.0f / 255.0f));
                    BodyTintColorTables[body][row][color] = (unsigned char)g_PALETTE1.get_nearest_palette_index(red, green, blue, 0);
                }
            }
        }
        void *cache = file.create("bodygrads.tmp", sizeof(BodyTintColorTables), 1);
        memcpy(cache, BodyTintColorTables, sizeof(BodyTintColorTables));
        file.close();
    } else {
        void *cache = file.open("bodygrads.tmp", 1);
        memcpy(BodyTintColorTables, cache, sizeof(BodyTintColorTables));
        file.close();
    }
    void *armor_cache = 0;
    if (!regenerate) armor_cache = file.open("armorgrads.tmp", 1);
    if (armor_cache) {
        memcpy(ArmorTintColorTables, armor_cache, sizeof(ArmorTintColorTables));
        file.close();
    } else {
        regenerate = true;
        for (int armor = 0; armor < 4; ++armor) {
            PALETTEENTRY tint = g_PALETTE1.entries()[armor_colors[armor]];
            HSV hsv;
            RGB_to_HSV(&tint, &hsv);
            for (int row = 0; row < 24; ++row) {
                const unsigned char *source = g_VBP_CAVIARDATA.record()->palette_ + 0x302 + row * 256;
                for (int color = 0; color < 256; ++color) {
                    const PALETTEENTRY entry = g_PALETTE1.entries()[source[color]];
                    const unsigned char red = (unsigned char)(int)((double)tint.peRed * entry.peRed * (1.0f / 255.0f));
                    const unsigned char green = (unsigned char)(int)((double)tint.peGreen * entry.peGreen * (1.0f / 255.0f));
                    const unsigned char blue = (unsigned char)(int)((double)tint.peBlue * entry.peBlue * (1.0f / 255.0f));
                    ArmorTintColorTables[armor][row][color] = (unsigned char)g_PALETTE1.get_nearest_palette_index(red, green, blue, 0);
                }
            }
        }
        void *cache = file.create("armorgrads.tmp", sizeof(ArmorTintColorTables), 1);
        memcpy(cache, ArmorTintColorTables, sizeof(ArmorTintColorTables));
        file.close();
    }
    for (int row = 0; row < 24; ++row) memset(ConstantColorTables[row], row, sizeof(ConstantColorTables[row]));
    void *transparent_cache = 0;
    if (!regenerate) transparent_cache = file.open("armorgrads_trans.tmp", 1);
    if (transparent_cache) {
        memcpy(ArmorTransparentColorTables, transparent_cache, sizeof(ArmorTransparentColorTables));
    } else {
        for (int row = 0; row < 24; ++row) {
            const int weight = ((24 - row) * transparent_first + row * transparent_second) / 24;
            g_PALETTE1.create_table_from_color(transparent_color, ArmorTransparentColorTables[row], 10, 236, 100 - weight, weight);
        }
        void *cache = file.create("armorgrads_trans.tmp", sizeof(ArmorTransparentColorTables), 1);
        memcpy(cache, ArmorTransparentColorTables, sizeof(ArmorTransparentColorTables));
    }
    file.close();
}

// Eight faction blocks begin at94CA08, with21records at38 stride and
// latest-type at+498. Founding/secrets/victory consumers establish the
// natural container; reset's previous94CA18 view began inside record0.
// RECOVERY_OBJECT: v2 0x0094CA08 0x0094F1A8 ?FactionMonuments@@3PAUMonumentFaction@@A 0x00476DA0
MonumentFaction FactionMonuments[MaxPlayerNum];

// Original441250 clears four words in each of64 cells, preserving the
// payload and the flat/transposed write order. Cell stride is90 bytes.
// RECOVERY_OBJECT: v2 0x007388D8 0x0073ACD8 ?OfferResetCells@@3PAY07UOfferResetCell@@A 0x00441250
OfferResetCell OfferResetCells[MaxPlayerNum][MaxPlayerNum];

// Original51E390 clears these independent words and two faction arrays.
// Consumer5275B0 displays the faction and repeated-event value. The three
// scalar meanings remain unknown; they are not fabricated gameplay names.
// RECOVERY_OBJECT: v2 0x0093A9AC 0x0093A9B0 ?TamperState93A9AC@@3HA 0x0051E390
int TamperState93A9AC;
// RECOVERY_OBJECT: v2 0x0093A9B0 0x0093A9B4 ?TamperState93A9B0@@3HA 0x0051E390
int TamperState93A9B0;
// RECOVERY_OBJECT: v2 0x0093A9B4 0x0093A9B8 ?TamperState93A9B4@@3HA 0x0051E390
int TamperState93A9B4;
// RECOVERY_OBJECT: v2 0x0093A9B8 0x0093A9D8 ?TamperDialogFaction@@3PAHA 0x0051E390
int TamperDialogFaction[MaxPlayerNum];
// RECOVERY_OBJECT: v2 0x0093A9D8 0x0093A9F8 ?TamperDialogValue@@3PAHA 0x0051E390
int TamperDialogValue[MaxPlayerNum];
// The registry blob is exactly fifty six-DWORD entries; the adjacent cursor
// is consumed as a signed value by native IDIV before it is advanced.
// RECOVERY_OBJECT: v1 0x0093A060 ?TamperRegistryRecords@@3PAUTamperRecord@@A
TamperRecord TamperRegistryRecords[50];
// RECOVERY_OBJECT: v1 0x0093A9F8 ?TamperRegistryCursor@@3HA
int TamperRegistryCursor;

// ORIGINAL: 0x00476B30 ?clear_monuments@@YAXXZ 0x00476B30-0x00476B61
// RECOVERY_BINDING: v1 {"evidence": ["Pinned476B30 starts its writes at94CA18 with38stride in8faction rows4F4apart; founding476B70 and secrets476DA0 establish containing-record base94CA08", "recovery/tests/test_setup_dependencies_behavior.py checks everybyte against the exact original22word mutation footprint"], "reason": "Reset the shared natural monument container: clear21achieved words atrecord+10 andlatest_type atrow+498, preserving all other bytes. Historical reset-only shifted interpretation remains in Git/task history; original seed and restoration obligations are intact."}
void __cdecl clear_monuments() {
    for (int faction = 0; faction < MaxPlayerNum; ++faction) {
        MonumentFaction &row = FactionMonuments[faction];
        for (int slot = 0; slot < 21; ++slot)
            row.records_[slot].achieved_ = 0;
        row.latest_type_ = -1;
    }
}

// ORIGINAL: 0x00441250 ?clear_all_offers@@YAXXZ 0x00441250-0x00441296
// RECOVERY_BINDING: v1 {"evidence": ["Pinned441250 writes four words at offsets0/4/8/12 in90-byte cells over8x8 faction pairs. Source-extracted tracked assignments exercise exact interleaved flat/transposed order."], "reason": "Recover complete offer reset using genuine64-cell storage and typed fields while preserving flat/transposed write ordering and untouched payload. No wider offer semantics are invented."}
void __cdecl clear_all_offers() {
    for (int row = 0; row < MaxPlayerNum; ++row) {
        for (int column = 0; column < MaxPlayerNum; ++column) {
            OfferResetCell &flat = OfferResetCells[row][column];
            OfferResetCell &transposed = OfferResetCells[column][row];
            flat.count_[0] = 0;
            flat.count_[1] = 0;
            transposed.count_[0] = 0;
            transposed.count_[1] = 0;
            flat.state_[0] = 0;
            flat.state_[1] = 0;
            transposed.state_[0] = 0;
            transposed.state_[1] = 0;
        }
    }
}

// ORIGINAL: 0x0051E390 ?clear_tamper@@YAXXZ 0x0051E390-0x0051E3BC
// RECOVERY_BINDING: v1 {"evidence": ["Pinned51E390 resets value93A9D8 then faction93A9B8 arrays and scalar93A9AC/B0/B4. Consumer5275B0 uses typed faction/value pairs for tamper dialogs; source-extracted fixture checks full arrays and scalars."], "reason": "Recover complete tamper reset with two independent faction arrays and three independent state words. Keep scalar semantics neutral until their actual consumers are recovered."}
void __cdecl clear_tamper() {
    memset(TamperDialogValue, 0, sizeof TamperDialogValue);
    memset(TamperDialogFaction, 0, sizeof TamperDialogFaction);
    TamperState93A9AC = 0;
    TamperState93A9B0 = 0;
    TamperState93A9B4 = 0;
}

uint32_t GameMorePreferences;  // 0x009A6494
uint32_t GameWarnings;  // 0x009A6498
uint32_t GameRules;  // 0x009A649C
uint32_t GameState;  // 0x009A64C0
int DiffLevelCurrent;  // 0x009A64C4
int TurnCurrentNum;  // 0x009A64D4
// setup_game clears this independent DWORD after BaseCurrentCount. No
// additional semantic use is established; it is not part of the base count.
// RECOVERY_OBJECT: v2 0x009A64D0 0x009A64D4 ?GameSetupResetWord@@3HA 0x005B3920
int GameSetupResetWord;
// set_time_controls publishes its selected seconds multiplied by 1000.
// net_control_turn copies the duration to the distinct live wait accumulator.
// RECOVERY_OBJECT: v2 0x009A64E4 0x009A64E8 ?TurnControlDurationMilliseconds@@3HA 0x00523C60
int TurnControlDurationMilliseconds;
int ObjectiveReqVictory;  // 0x0094B4C0
uint32_t ObjectivesSuddenDeathVictory;  // 0x0094B4C4
uint32_t ObjectiveAchievePts;  // 0x0094B4C8
uint32_t VictoryAchieveBonusPts;  // 0x0094B4CC
uint32_t MissionYearCurrent;  // 0x009A64D8
uint32_t StartingMissionYear;  // 0x0094B4D0
uint32_t EndingMissionYear;  // 0x0094B4D4
int SunspotDuration;  // 0x009A6800
// config_popups copies the live screen dimensions to these independent
// dwords; both are beyond Console's proven end, not Console members.
// RECOVERY_OBJECT: v1 0x00939E58 ?IfaceScreenHeight@@3HA
int IfaceScreenHeight; // 0x00939E58
// RECOVERY_OBJECT: v1 0x00939E5C ?IfaceScreenWidth@@3HA
int IfaceScreenWidth;  // 0x00939E5C
// Original file-backed palette defaults; MessageWin owns the interleaved
// text/background cells at 0x68A5A4/B0 separately.
// RECOVERY_OBJECT: v1 0x0068A5A0 ?PopupThemeWindowColor@@3HA
int PopupThemeWindowColor = 0x62; // 0x0068A5A0
// RECOVERY_OBJECT: v1 0x0068A5A8 ?PopupThemeButtonColor@@3HA
int PopupThemeButtonColor = 0xE8; // 0x0068A5A8
// RECOVERY_OBJECT: v1 0x0068A5AC ?PopupThemeTextShadow@@3HA
int PopupThemeTextShadow = 0xE9; // 0x0068A5AC
// RECOVERY_OBJECT: v1 0x0068A5B8 ?PopupThemeSurfaceColor@@3HA
int PopupThemeSurfaceColor = 0x24; // 0x0068A5B8
// RECOVERY_OBJECT: v1 0x0068A5BC ?PopupThemeBevelUpper@@3HA
int PopupThemeBevelUpper = 0x1E; // 0x0068A5BC
// RECOVERY_OBJECT: v1 0x0068A5C0 ?PopupThemeBevelLower@@3HA
int PopupThemeBevelLower = 0x56; // 0x0068A5C0
// RECOVERY_OBJECT: v1 0x0068A5B4 ?PopupThemeColorB4@@3HA
int PopupThemeColorB4 = 0x94; // 0x0068A5B4, tenth independent palette cell

// Original455610..455E41. The explicit mode argument is unused. All service
// results are ignored in the original; failed loads still proceed through
// extraction/publication. The existing asset objects own their allocations.
// ORIGINAL: 0x00455610 ?load_interface@@YAXH@Z 0x00455610-0x00455E41
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00455610.cpp: retained original spans and seed history", "source:recovery/tasks/popup-font-configuration-owner-closure.json: coordinator-authorized ownership scope", "manual: independent frozen C++ review of original callback, font style order, palette, geometry and sprite assignments", "compiler:build/recovery/popup-font-physical-emission.json;build/recovery/interface-producer-physical.json: actual emitted symbols in both historical flag sets"], "reason": "Original-backed real font/default producer and consumer closure through source-owned storage and C++ classes; preserves seed extents and historical matching obligations without granting a new claim."}
// symbol    ?load_interface@@YAXH@Z
void __cdecl load_interface(int mode) {
    Buffer &canvas = g_PCX_PARSE_TEMP_BUFFER1;
    GraphicWinFieldA0CDefault = reinterpret_cast<uint32_t>(&def_caption_draw);
    canvas.load_pcx("iface.pcx", 0, 10, 0xEC);
    int *palette_cells[10] = {&PopupThemeWindowColor, &MessageWinTextColor,
        &PopupThemeButtonColor, &PopupThemeTextShadow, &MessageWinTextBackground,
        &PopupThemeColorB4, &PopupThemeSurfaceColor, &PopupThemeBevelUpper,
        &PopupThemeBevelLower, &DiploKnownTechColor};
    for (int color = 0; color < 10; ++color)
        *palette_cells[color] = canvas.get_pixel(0x3A3, 2 + color * 11);

    canvas.change_color(0x108, 0x109);
    Sprite *frames[11] = {&g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE,
        &g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE};
    const int rectangles[11][4] = {
        {0x223,0x26,0x4D,0x42},{0x296,0x26,0x3D,0x42},
        {0x223,0x90,0x4D,0x29},{0x296,0x90,0x3D,0x29},
        {0x271,0x26,0x24,0x42},{0x271,0x90,0x24,0x29},
        {0x223,0x69,0x4D,0x26},{0x296,0x69,0x3D,0x26},
        {0x223,0xC3,0x23,0x1F},{0x247,0xC3,0x30,0x1F},
        {0x278,0xC3,0x24,0x1F}};
    for (int frame = 0; frame < 11; ++frame) {
        canvas.change_color(0x107, 0x109);
        canvas.change_color(0x106, 0x109);
        frames[frame]->extract(&canvas, 0x109, rectangles[frame][0],
            rectangles[frame][1], rectangles[frame][2], rectangles[frame][3], 0);
    }
    NCSprites defaults = {frames[0],frames[1],frames[2],frames[3],frames[4],
        frames[6],frames[7],frames[5],frames[9],frames[8],frames[10]};
    GraphicWin::set_def_nc_sprites(&defaults);
    GraphicWin::load_all_nc_sprites_from_defs();

    // These are borrowed entries into the existing 27-Sprite button sheet.
    BasePopDefaultBlock31B8[12] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[18]);
    BasePopDefaultBlock31B8[13] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[21]);
    BasePopDefaultBlock31B8[14] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[24]);
    BasePopDefaultBlock31B8[15] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[20]);
    BasePopDefaultBlock31B8[16] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[23]);
    BasePopDefaultBlock31B8[17] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[26]);
    BasePopDefaultBlock31B8[6] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[19]);
    BasePopDefaultBlock31B8[7] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[22]);
    BasePopDefaultBlock31B8[8] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[25]);
    PushButtonDefaults[4] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[18]);
    PushButtonDefaults[13] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[24]);
    PushButtonDefaults[2] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[19]);
    PushButtonDefaults[11] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[25]);
    PushButtonDefaults[5] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[20]);
    PushButtonDefaults[14] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[26]);
    const int push_zero_slots[12] = {6,15,7,16,3,12,0,9,1,10,8,17};
    for (int slot = 0; slot < 12; ++slot) PushButtonDefaults[push_zero_slots[slot]] = 0;
    FlatButtonDefaultsTable[12] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[18]);
    FlatButtonDefaultsTable[13] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[21]);
    FlatButtonDefaultsTable[14] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[24]);
    FlatButtonDefaultsTable[6] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[19]);
    FlatButtonDefaultsTable[17] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[26]);
    FlatButtonDefaultsTable[8] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[25]);
    FlatButtonDefaultsTable[15] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[20]);

    InterfaceAltCaptionHeight = g_UNUSED_SPRITE_VAR29.source_height() + g_UNUSED_SPRITE_VAR29.top_offset();
    InterfaceAltBorderThickness = g_UNUSED_SPRITE_VAR27.source_width() + g_UNUSED_SPRITE_VAR27.left_offset();
    InterfaceAltBottomBorderThickness = g_UNUSED_SPRITE_VAR30.height() - g_UNUSED_SPRITE_VAR30.top_offset();
    InterfaceCaptionHeight = g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE.source_height()
        + g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE.top_offset();
    InterfaceBorderThickness = g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE.source_width()
        + g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE.left_offset();
    InterfaceBottomBorderThickness = g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE.height()
        - g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE.top_offset();
    InterfaceThirdCaptionHeight = g_UNUSED_SPRITE_VAR08.source_height() + g_UNUSED_SPRITE_VAR08.top_offset();
    InterfaceThirdBottomBorderThickness = g_UNUSED_SPRITE_VAR04.height() - g_UNUSED_SPRITE_VAR04.top_offset();
    InterfaceFourthBorderThickness = g_UNUSED_SPRITE_VAR16.source_width() + g_UNUSED_SPRITE_VAR16.left_offset();
    InterfaceFourthBottomBorderThickness = g_UNUSED_SPRITE_VAR13.height() - g_UNUSED_SPRITE_VAR13.top_offset();
    FlatButtonDefaultsTable[7] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[22]);
    FlatButtonDefaultsTable[16] = reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES[23]);
    const int flat_zero_slots[18] = {18,19,20,21,22,23,9,10,11,0,1,2,3,4,5,24,25,26};
    for (int zero = 0; zero < 18; ++zero) FlatButtonDefaultsTable[flat_zero_slots[zero]] = 0;
    InterfaceDefaultMetric = 45;
    InterfaceTopLeftMetric = 10;
    InterfaceMidRightMetric = 10;
    for (int window_id = 1; window_id < 8; ++window_id) {
        MapWin *window = MapWinTable[window_id];
        if (window != 0 && window->field_1DD74_ != 0) {
            GraphicWin *display = static_cast<GraphicWin *>(window);
            display->set_frame_top_right(&g_UNUSED_SPRITE_VAR31);
            display->compute_min_size();
            // Reload the published slot after the call, as the original does.
            static_cast<GraphicWin *>(MapWinTable[window_id])->on_redraw_nc(0, -1);
        }
    }
    WinStaticDefaults[5] = InterfaceBorderThickness;
    WinStaticDefaults[1] = InterfaceCaptionHeight;
    canvas.load_pcx("interface.pcx", 0, 10, 0xEC);
    for (int row = 0; row < 9; ++row) {
        for (int part = 0; part < 3; ++part) {
            canvas.change_color(0x107, 0x109);
            canvas.change_color(0x106, 0x109);
            const int x = part == 0 ? 0x40 : part == 1 ? 0x44 : 0x88;
            g_BASEWIN_SPRITES[row * 3 + part].extract(&canvas, 0x109, x,
                1 + row * 21, part == 1 ? 0x43 : 3, 20, 0);
        }
    }
    for (int icon = 0; icon < 3; ++icon) {
        canvas.change_color(0x107, 0x109);
        canvas.change_color(0x106, 0x109);
        g_IFACE_CLOSE_X_SPRITES[icon].extract(&canvas, 0x109, 0xE3,
            1 + icon * 19, 18, 18, 0);
    }
    canvas.close();
    load_box_sprites();
}
uint32_t MountPlanetX;  // 0x009A6804
uint32_t MountPlanetY;  // 0x009A6808
int DustCloudDuration;  // 0x009A680C
// save_daemon writes 12 before serialization; setup_game clears this word.
// Its precise save-format meaning is not established by that write alone.
// RECOVERY_OBJECT: v2 0x009A6810 0x009A6814 ?SaveDaemonStateWord@@3HA 0x005A94F0
int SaveDaemonStateWord;
// Original4838E3 reads this DWORD network-game flag; initial image storage is0.
// RECOVERY_OBJECT: v2 0x0093F660 0x0093F664 ?IsMultiplayerNet@@3HA 0x004838E0
BOOL IsMultiplayerNet;  // DirectPlay - Serial, Modem, Internet (TCP/IP)
BOOL IsMultiplayerPBEM;  // 0x0093A95C // HotSeat / PBEM
uint32_t NetTurnFlags;  // 0x009A681C
int NetTurnFaction;  // 0x009A6820

// Shared turn handoff flag: next_player_turn and player deletion write it;
// net_not_my_turn reads it. The observed four bytes start loader-zeroed.
// RECOVERY_OBJECT: v2 0x0093A950 0x0093A954 ?NextTurnReady@@3HA 0x0052DBF0
int NextTurnReady;
// net_upkeep resets/increments this phase and net_upkeep_phase consumes it.
// Its producer is still unfinished; this storage does not recover that body.
// RECOVERY_OBJECT: v2 0x0093A93C 0x0093A940 ?NetUpkeepPhase@@3HA 0x005290A0
int NetUpkeepPhase;
// base_ecology increments this signed accumulator, scales it in its event
// threshold, then clears it when the threshold is met. setup_game also clears
// it. The adjacent NetTurnFaction is a separate object, not an array element.
// RECOVERY_OBJECT: v2 0x009A6824 0x009A6828 ?EcologyEventAccumulator@@3HA 0x004F67F0
int EcologyEventAccumulator;

/*
Purpose: Wrap an x coordinate the way reset_territory() wraps it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

xrange() at 0048BEE0 cannot stand in, for one bit. It tests the whole of
MapIsFlat; reset_territory reads the low BYTE and tests bit zero
(`mov cl, byte ptr [94988Ch] / test cl, 1`). The two agree on the 0 and 1 the
game stores there and disagree on everything else, and the difference is in the
original rather than in the transcription. world_site() at 005C4FD0 carries the
same distinction as map.cpp's site_xrange().
*/
__forceinline static int territory_xrange(int x) {
    if (!(MapIsFlat & 1)) {
        if (x < 0) {
            x += MapLongitudeBounds;
        } else if (x >= MapLongitudeBounds) {
            x -= MapLongitudeBounds;
        }
    }
    return x;
}

/*
Purpose: Recompute which faction owns each tile of the map, and rebuild every
         per-faction and per-region tally that ownership feeds.
// ORIGINAL: 0x00523DD0 ?reset_territory@@YAXXZ 0x00523DD0-0x00524202
// LEVER: 9/349 and 0.403 similar -> 158/349 and 0.789 similar, on five measured
//   changes, in this order. (1) `territory_xrange` - a local static helper with no
//   marker of its own - was a real `call`; the image inlines it, and
//   `__forceinline` is what VC6 needs to be told. That alone made call_diff agree
//   (10 calls -> 9) and took 0.403 -> 0.434. (2) DROPPED the `PlayerData &player`
//   binding from the six-array zeroing loop and indexed `PlayersData[faction_id]`
//   at each store: 0.434 -> 0.482 and 21/349 -> 55/349. Binding the row gives VC6
//   a second pointer to strength-reduce, and it then carries the region index as
//   `lea esi, [edx + ecx]` off it, where the image keeps EDX on the two byte
//   arrays and ECX as a plain index (`cmp ecx, 0x80`). (3) The site block is an
//   `if (tile->bit & BIT_BASE_RADIUS) { site = 0; } else { ... }`, NOT
//   `site = 0;` followed by an inverted guard: the image falls THROUGH to the
//   zero and jumps to the val2 read (`test ah, 0x10 / je`). 0.482 -> 0.583.
//   (4) `& ~0x1F` and `& ~0x3F` rather than `& 0xE0` and `& 0xC0` - identical on
//   these `uint8_t` fields, but the complement stops VC6 narrowing the whole test
//   to a byte, which is what puts `xor ecx, ecx / mov cl, [edi] / and ecx,
//   0xFFFFFFE0 / cmp ecx, 0x60 / setl` in place of `and cl, 0xE0 / cmp cl, 0x60 /
//   sbb / neg`. 0.583 -> 0.613, and it brought the compiled instruction count to
//   the image's 349 exactly. (5) `territory_xrange` again, rewritten
//   `if (x < 0) ... else if (x >= bounds) ...`: the image's `jge` falls through to
//   the negative arm, the `if (x >= 0)` spelling inverts it. 0.613 -> 0.630.
//   (6) ONE `Continent *` for the region, reused by the unk_3 store and the pods
//   increment: the image computes `region * 28` once into EDI and addresses both
//   as `[edi + 0x9AA738]` / `[edi + 0x9AA73C]`, while `Continents[region].field`
//   at each site made VC6 recompute the whole lea/sub/shl chain across the
//   intervening `goody_at` call. 0.630 -> 0.789.
// TRIED: measured and rejected on the way. Splitting the rocky/fungus,
//   visibility/is_human and BaseFindDist chains into separate `if`s: all three
//   score exactly what the chained form does. Splitting the owner add_site
//   condition into two arms that each call add_site does raise positional
//   agreement (5 -> 13 at the time) but adds a third add_site call the image does
//   not make, which call_diff then reports as MORE - rejected on that. Hoisting
//   the masked climate/rockiness into `const int` locals: no change, VC6 folds
//   them. A `PlayerData *` walked with `player++` in the zeroing loop: no change.
//   Storing the byte arrays first in that loop: no change. `Continent &` instead
//   of `Continent *`, and a `uint8_t *` for the unk_82 read/modify/write: both
//   identical to what is committed, so they are free spellings, not levers.
// TRIED: what is left is register ASSIGNMENT, 20 diverging runs under
//   /c /O2 /Gy /GR- /Oy- /GX and one instruction short of the image's 349. The
//   faction loop is a straight ESI/EDI swap (image counts in ESI and walks the
//   unk_79 pointer in EDI, this tree the other way round); `region * 2` is CSE'd
//   into `shl eax, 1` by the image and re-encoded as `[eax + eax + disp]` here at
//   all three unk_78/80/81 increments; and `cmp eax, ecx` / `cmp ecx, eax` at the
//   territory-distance test is the same comparison with the operands in the other
//   registers. None of those is reachable from the statement level.
// size      1074 bytes
// prototype
// callers   11   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3D50 0x00579B70 0x00579D20 0x00591B50 0x00592140 0x005C4FD0
Return Value: n/a
Status: Complete

Ownership is decided by proximity: the nearest base within
Rules.territory_max_dist_base owns the tile, and an OCEAN tile halves that
distance and is then overruled outright by any base in its own eight-tile
ring - which is how coastal water ends up belonging to the base beside it
rather than to a larger one further off. A tile with no base in range is
unclaimed, which the map stores as -1.

Everything else here is tallies. Six per-region arrays are zeroed for all eight
factions before the walk, and the walk refills them:

  - unk_79[region] counts tiles the faction can see in that region - or, for a
    faction whose AI is running the objectives strategy, tiles carrying
    BIT_SUPPLY_REMOVE instead. That substitution is what PFLAG_STRAT_SEARCH_-
    OBJECTIVES selects, and it is a substitution rather than an addition.
  - unk_78[region] counts tiles the faction owns; unclaimed tiles count for
    faction zero.
  - unk_80[region] counts owned tiles that are wet, not rocky and not fungus,
    and unk_81[region] the subset of those already inside a base radius.
  - unk_82[region] is the best world_site score the faction owns there, and
    unk_83[region] the supply pods on its tiles.

Continents[region] gets the same last two facts for the region as a whole, in
unk_3 and pods, which is what those two field comments already say.

The walk is the same shape as num_objectives': one Map entry per tile visited,
never reset between rows, starting each row at `x = y & 1`. An empty row - one
where the start is already past MapLongitudeBounds - does not advance the
pointer at all.

A tile inside a base radius is not scored as a site and is not asked to be: its
score is taken as zero without reading val2. Elsewhere the stored score is used
if there is one and world_site() is called and the answer cached with
site_set() if there is not, so a full map walk costs at most one scoring pass
per tile ever.

The supply-pod arm ends in a site request, and its two branches are not
symmetric. An owned tile asks only its owner; an unclaimed one asks every
faction that has a base in the region, and each of those must additionally not
already hold a site here and must have a base at between one and two times the
territory distance - near enough to want it, far enough not to have it already.
Both branches require the faction either to see the tile or to be an AI above
difficulty 3, which is the standard "the AI is allowed to know" test.
*/
void __cdecl reset_territory() {
    // NO `PlayerData &player` HERE. Binding the faction row to a reference
    // gives VC6 a second live pointer to strength-reduce, and it then carries
    // the region INDEX as a subtraction off it; the image keeps `edx` on the
    // byte pair and `ecx` as the index. Indexing `PlayersData[faction_id]`
    // at each store instead is what reproduces that.
    for (int faction_id = 0; faction_id < MaxPlayerNum; faction_id++) {
        for (int region = 0; region < MaxContinentNum; region++) {
            PlayersData[faction_id].unk_80[region] = 0;
            PlayersData[faction_id].unk_78[region] = 0;
            PlayersData[faction_id].unk_79[region] = 0;
            PlayersData[faction_id].unk_81[region] = 0;
            PlayersData[faction_id].unk_83[region] = 0;
            PlayersData[faction_id].unk_82[region] = 0;
        }
    }
    for (int region = 0; region < MaxContinentNum; region++) {
        Continents[region].pods = 0;
        Continents[region].unk_3 = 0;
    }
    Map *tile = map_tiles();
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2, tile++) {
            int region = tile->region;
            for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
                uint32_t flags = PlayersData[faction_id].flags;
                if (flags & PFLAG_STRAT_SEARCH_OBJECTIVES) {
                    if (tile->bit & BIT_SUPPLY_REMOVE) {
                        PlayersData[faction_id].unk_79[region]++;
                    }
                } else if (flags & PFLAG_MAP_REVEALED
                    || tile->visibility & (1 << faction_id)) {
                    PlayersData[faction_id].unk_79[region]++;
                }
            }
            int owner = -1;
            int base_id = base_find(x, y, -1, region, -1, -1);
            // `& ~0x1F`, not `& 0xE0`. Identical on a `uint8_t` field - both
            // keep bits 5..7 of a value the promotion has already zero-extended -
            // but the complement is what stops VC6 narrowing the whole test to a
            // byte. The image loads `xor ecx, ecx / mov cl, [edi]`, masks
            // `and ecx, 0xFFFFFFE0` and compares the DWORD `cmp ecx, 0x60 / setl`;
            // `& 0xE0` gives `and cl, 0xE0 / cmp cl, 0x60 / sbb / neg` instead.
            BOOL is_ocean_tile = (tile->climate & ~0x1F) < (ALT_SHORE_LINE << 5);
            if (base_id >= 0) {
                int max_dist = (int)Rules.territory_max_dist_base;
                if (is_ocean_tile) {
                    max_dist >>= 1;
                }
                if (BaseFindDist < max_dist) {
                    owner = Bases[base_id].faction_id_current;
                }
            }
            if (is_ocean_tile) {
                for (int i = 0; i < 9; i++) {
                    int x_adj = territory_xrange(x + RadiusBaseX[i]);
                    int y_adj = y + RadiusBaseY[i];
                    if (!on_map(x_adj, y_adj)) {
                        continue;
                    }
                    int adjacent_id = base_at(x_adj, y_adj);
                    if (adjacent_id >= 0) {
                        owner = Bases[adjacent_id].faction_id_current;
                        break;
                    }
                }
            }
            if (owner != tile->territory) {
                tile->bit2 |= 0x400000; // TODO: identify value
                UnkBitfield1 |= 1; // TODO: identify global + value
            }
            tile->territory = (int8_t)owner;
            if (owner < 0) {
                owner = 0;
            }
            PlayersData[owner].unk_78[region]++;
            if (tile->climate & (RAINFALL_MOIST | RAINFALL_RAINY)) {
                // `& ~0x3F` for the same reason as the altitude mask above: the
                // image compares the widened dword (`and ecx, 0xFFFFFFC0 /
                // cmp ecx, 0x80 / jge`), `& 0xC0` compares the byte (`jae`).
                if ((tile->val3 & ~0x3F) < (ROCKINESS_ROCKY << 6)
                    && !(tile->bit & BIT_FUNGUS)) {
                    PlayersData[owner].unk_80[region]++;
                    if (tile->bit & BIT_BASE_RADIUS) {
                        PlayersData[owner].unk_81[region]++;
                    }
                }
            }
            // if/else, NOT `site = 0` then an inverted guard. The image
            // falls THROUGH to the zero and jumps to the val2 read
            // (`test ah, 0x10 / je 0x0052401F`), which is what the positive
            // test produces; the guard-clause form inverts every branch in
            // this block.
            int site;
            if (tile->bit & BIT_BASE_RADIUS) {
                site = 0;
            } else {
                site = tile->val2 >> 4;
                if (!site) {
                    site = world_site(x, y, false);
                    site_set(x, y, site);
                }
            }
            // ONE pointer to the region's Continent, reused by both the
            // unk_3 store here and the pods increment below. The image
            // computes `region * 28` once into EDI and addresses both fields
            // as `[edi + 0x9AA738]` / `[edi + 0x9AA73C]`; writing
            // `Continents[region].field` at each site made VC6 recompute the
            // whole `lea/sub/shl` chain for the second one.
            Continent *const continent = &Continents[region];
            int region_best = (int)continent->unk_3;
            if (site > region_best) {
                region_best = site;
            }
            continent->unk_3 = (uint32_t)region_best;
            if (owner) {
                int owner_best = PlayersData[owner].unk_82[region];
                if (site > owner_best) {
                    owner_best = site;
                }
                PlayersData[owner].unk_82[region] = (uint8_t)owner_best;
            }
            if (!goody_at(x, y)) {
                continue;
            }
            continent->pods++;
            PlayersData[owner].unk_83[region]++;
            if (owner) {
                if (tile->visibility & (1 << owner)
                    || (!is_human(owner) && DiffLevelCurrent > 3)) {
                    add_site(owner, 3, 1, x, y);
                }
                continue;
            }
            for (faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
                if (!PlayersData[faction_id].region_total_bases[region]) {
                    continue;
                }
                if (!(tile->visibility & (1 << faction_id))
                    && (is_human(faction_id) || DiffLevelCurrent <= 3)) {
                    continue;
                }
                if (at_site(faction_id, 3, x, y)) {
                    continue;
                }
                if (base_find(x, y, faction_id, region, -1, -1) < 0) {
                    continue;
                }
                int limit = (int)Rules.territory_max_dist_base;
                if (BaseFindDist < limit || BaseFindDist > limit + limit) {
                    continue;
                }
                add_site(faction_id, 3, 1, x, y);
            }
        }
    }
}

/*
Purpose: Determine whether the turn currently belongs to another faction in a
         networked game, which is what gates local input.
// ORIGINAL: 0x0052DC70 ?not_my_turn@@YAHXZ 0x0052DC70-0x0052DC9C BYTE_EXACT
// size      44 bytes
// prototype
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: TRUE only in a net game that is handing the turn around and whose
              active faction is not the local one
Status: Complete
*/
// BODY IN game.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Handle creation of pop-up message on Planetfall.
// ORIGINAL: 0x00589180 ?planetfall@@YAXH@Z 0x00589180-0x00589329
// size      425 bytes
// prototype void (__cdecl ?planetfall@@YAXH@Z)(int factionID)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF310 0x00625E30 0x00625E50 0x00625EC0 0x00645470 0x00645660
// indirect  0x0058923D
// TRIED: not resolved this pass. The image's FOUR `parse_says`
//        (0x00625EC0) call sites write the gender/plurality arguments to
//        FIXED GLOBALS (0x9bbfec/0x9bbff0) individually before each call
//        rather than through `parse_set`, in an ORDER that does not match
//        this body's own call order (id 2, then 3, then 0, then 1 - not
//        0, 2, 3, 1), and the calls list also names 0x00625E30 (parse_num)
//        AND 0x00625E50 (parse_say, the int-input overload) - this body
//        calls parse_num once and never parse_say, so one of its calls is
//        the wrong callee. Left at 11/125 for a pass that can trace which
//        source line the int-input `parse_say` belongs to.
Return Value: n/a
Status: Complete
*/
void __cdecl planetfall(int faction_id) {
    parse_set(Players[faction_id].is_leader_female, false);
    parse_says(0, Players[faction_id].adj_name_faction, -1, -1);
    parse_says(2, Players[faction_id].title_leader, -1, -1);
    parse_says(3, Players[faction_id].name_leader, -1, -1);
    parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
    parse_says(1, Players[faction_id].noun_faction, -1, -1); // unused in script, leaving in for now
    char script_id[13];
    if (TurnCurrentNum) { // shifted logic to top to fix nonexistent accelerated script ids
        parse_num(0, TurnCurrentNum);
        for (uint32_t i = 0; i < MaxSecretProjectNum; i++) {
            if (has_project(i, faction_id)) { // script assumes at least one SP is built per faction
                parse_says(4, Facility[FAC_HUMAN_GENOME_PROJ + i].name, -1, -1);
            }
        }
        strcpy_s(script_id, 13, "PLANETFALL2"); // bug fix: changed to fixed id rather than concat 2
    } else if (is_alien_faction(faction_id) && !_stricmp(Players[faction_id].filename, "USURPER")) {
        strcpy_s(script_id, 13, "PLANETFALLX");
    } else if (!_stricmp(Players[faction_id].filename, "FUNGBOY")) {
        strcpy_s(script_id, 13, "PLANETFALLF");
    } else {
        strcpy_s(script_id, 13, "PLANETFALL");
    }
    X_pop(script_id, NULL);
}

/*
Purpose: Clear and reset scenario game variables.
// ORIGINAL: 0x005B38D0 ?clear_scenario@@YAXXZ 0x005B38D0-0x005B3919 BYTE_EXACT
// size      73 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl clear_scenario() {
    ObjectiveReqVictory = 9999;
    ObjectivesSuddenDeathVictory = 9999;
    ObjectiveAchievePts = 0;
    VictoryAchieveBonusPts = 0;
    StartingMissionYear = Rules.normal_starting_year;
    EndingMissionYear = DiffLevelCurrent < DLVL_LIBRARIAN
        ? Rules.normal_end_year_low_three_diff : Rules.normal_end_year_high_three_diff;
}

/*
Purpose: Calculate game year from start date and turn number.
// ORIGINAL: 0x005C89A0 ?game_year@@YAHH@Z 0x005C89A0-0x005C89B0 BYTE_EXACT
// symbol    ?game_year@@YAIH@Z
// size      16 bytes
// prototype int (__cdecl ?game_year@@YAHH@Z)(int turn)
// callers   30   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Game year
Status: Complete
*/
// BODY IN game.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Craft game year string. Assume buffer of at least 80. Revisit and convert to std::string.
// ORIGINAL: 0x005C89B0 ?say_year@@YAXPAD@Z 0x005C89B0-0x005C89E4 BYTE_EXACT
// LEVER (2026-09-03, pair normalisation): plain `strcat`, not `strcat_s` - see
//   the note on the body. 5 of 20 -> 20 of 20 on that one word.
// size      52 bytes
// prototype
// callers   19   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_year(LPSTR output) {
    char year[80];
    _itoa_s(game_year(TurnCurrentNum), year, 80, 10);
    // Plain `strcat`, not `strcat_s`: the image calls 0x645470 here, and the
    // `_s` spelling reaches `strcat` through the inline in vc6_compat.h, which
    // the optimiser then folds in WITH strcat's own `repne scasb` +
    // `rep movsd` block expansion - 37 instructions against the image's 20.
    // Naming `strcat` directly is what keeps it a real call; see the same
    // finding on Font::init.
    strcat(output, year);
}

// Out-of-line forwarders for repair_phase's three helpers that the image
// CALLS and this tree would otherwise fold in. See the LEVER on the
// 0x00526030 marker below; `#pragma auto_inline(off)` is what VC6 6.0 has
// instead of `__declspec(noinline)`.
#pragma auto_inline(off)
static int __cdecl breed_mod_call(int base_id, int faction_id) {
    return breed_mod(base_id, faction_id);
}
static void __cdecl draw_tile_call(int x, int y, int flags) {
    draw_tile(x, y, flags);
}
#pragma auto_inline(on)

/*
Purpose: Run the repair phase for the specified faction: reset the per-turn unit state, heal every
         damaged unit by its reactor-weighted repair rate, and redraw the tiles the units stand on.
// ORIGINAL: 0x00526030 ?repair_phase@@YAXH@Z 0x00526030-0x005267A9
// LEVER: 2/572 and 0.125 similar -> 59/572 and 0.299 similar, and the direct-call
//   multiset went from 11 to the image's exact 8. Four measured changes.
//   (1) THREE helpers this tree folded in that the image CALLS: `breed_mod`
//   (0x004E65C0) and `draw_tile` (0x0046AF40) are both `MEASURED inline`, and
//   VC6 6.0 has no `__declspec(noinline)` - `#pragma auto_inline(off)` around a
//   plain forwarder in THIS file does the job, and the forwarders sit just above
//   this marker. breed_mod alone was worth three of the extra calls, because
//   inlining it brought its four `has_fac_built_call` bitmask calls with it where
//   the image makes exactly one. (2) That one belongs to THIS body: the
//   facility/project test uses `has_fac_built_call`, not `has_fac_built`, so the
//   bitmask is a real `E8` at 0x005263D1. (1)+(2) took 0.125 -> 0.241.
//   (3) `veh_top` HAND-INLINED. The note below already said the image writes the
//   climb out at 0x005264A8-0x005264C6 rather than calling 0x00579920, because
//   veh.cpp is a different translation unit - but this body still CALLED it, so
//   the note described the image and not the code beside it. Transcribing the
//   climb (starting after the dead `veh_id < 0` guard) brought the call count to
//   8 against the image's 8 and 0.241 -> 0.256; veh.cpp already does this at two
//   of its own sites. (4) `int damage_before`, not `uint8_t`: the image
//   zero-extends on the way in (`xor eax, eax / mov al, [esi + 0x952838]`) and
//   spills a DWORD, where the byte local spills a byte and drops the extension -
//   0.256 -> 0.297 on its own. (5) The VFLAG_UNK_2 test is written NEGATED with
//   the arms swapped, because the image's `jne` goes to the `&= ~VFLAG_UNK_2` arm
//   and falls through to `&= ~VFLAG_UNK_1`. 0.297 -> 0.299.
// TRIED: `triad` as `uint8_t` or `int` rather than `uint32_t`. The image does
//   keep the chassis triad in CL (`mov cl, [ecx + 0x94A379] / test cl, cl /
//   cmp cl, 2`) where this tree zero-extends it into a dword, so the width lever
//   looks right - but measured on all three declarations it is 0.300/0.299/0.299
//   against 0.299 committed, and both `uint8_t` variants RAISE the diverging-run
//   count (55 -> 61 and 62). It is not the lever it looks like.
// TRIED: the VFLAG arms are still 16-bit here (`and eax, 0xFFFE`) against the
//   image's 8-bit (`and al, 0xFE`), and writing the read-modify-write straight on
//   `Vehs[veh_id].flags` with no local does not change that - 0.256, identical to
//   the local. The other standing gap is `tile->bit`: the image re-reads it as a
//   memory operand at every test (`test byte ptr [eax + 8], 1`, then
//   `test dword ptr [eax + 8], 0x800`), while VC6 loads the dword once here and
//   tests CL and CH out of it. Both reads are already written out separately in
//   the source, so the CSE is not something the statement level can withdraw.
// size      1913 bytes
// prototype void (__cdecl ?repair_phase@@YAXH@Z)(int factionID)
// callers   1   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AF40 0x004E3A50 0x004E3EF0 0x004E65C0 0x0050BA00 0x005BF1F0 0x005C1760 0x005FCBB0
Return Value: n/a
Status: Complete

TWO PASSES OVER THE UNIT TABLE, NOT ONE. The first heals; the second redraws. They cannot be
folded together, because the first pass can retire a Fungal Tower's owner (below) and the second
pass filters on the faction byte the first one may have just rewritten.

WHAT "REPAIR RATE" MEANS. The rate is a count of repair points, and the damage actually removed is
`reactor_id * rate` - a Fission unit (reactor 1) removes one point per rate step, a Singularity
unit (reactor 4) removes four. The rate starts at 1 and is added to:

  +1  standing in your own territory
  +1  a land unit in a bunker, or an air unit on an airbase
  +1  standing in any base that is not rioting
  x2  a land transport carrying a unit with the Repair ability, at sea or under Sentry/Board

and it is replaced outright by the current damage - a full heal - when the base has the matching
repair facility for the unit's triad (Command Center / Naval Yard / Aerospace Complex) or the
faction holds the matching secret project (Command Nexus / Maritime Control Center / Cloudbase
Academy); when a native-life unit sits in a base whose faction has any lifecycle bonus at all
(`breed_mod`); or, anywhere on the map, when the faction holds the Nano Factory.

Native life in fungus is the one rate bonus that is not additive: it sets the rate to 2 rather
than incrementing it, and it is skipped entirely for a Fungal Tower.

MINIMUM DAMAGE. A unit in the field - non-native faction, not in a base, no repair unit in its
stack, no Nano Factory - cannot be healed below `reactor_id * 2`. Native life in fungus, and
anyone in fungus whose faction holds the Xenoempathy Dome, are exempt; the Dome additionally adds
one more reactor's worth of healing.

Verification note: three tests in this body can never take their false branch and are transcribed
rather than deleted. NONE of them is a mutation-sweep survivor, and the distinction is worth
keeping straight: a test that is constant-true still yields a mutant that is not, so "unreachable"
here is a statement about faithfulness to the original, not a coverage gap being excused.

  - `owner >= 0` after `owner < MaxPlayerNum`, in both passes. This is `base_who()` inlined
    (0x00526152 and 0x00526755); the nibble is masked to 0xF before it is asked, so the sign test
    is constant. can_terraform carries the identical dead half for the identical reason. Mutating
    it to `> 0` is NOT constant - a base owned by faction 0 separates them - and the fixtures
    cover that in both passes.
  - `if (min_damage)` at 0x005265AF. min_damage is `reactor_id * 2`, so the guard is false exactly
    when reactor_id is zero - and with reactor_id zero every statement it guards is a no-op
    (`min_damage` is already 0 and the Dome adds `reactor_id`). The branch is in the original;
    removing it changes nothing, and the harness derives no mutant from it because it carries
    neither a literal nor a comparison.
  - `veh_top()` returning negative. The original inlines the stack walk here rather than calling
    0x00579920, because veh.cpp is a different translation unit; its `veh_id < 0` guard
    (0x005264A2) and the `top < 0` test that follows (0x005264CC) are both dead, since the
    argument is this loop's own index. The walk at 0x005264A8-0x005264C6 is
    instruction-for-instruction that function, and it is now transcribed here rather than called -
    calling it was semantically exact and structurally wrong, one `E8` the image does not have.
    The `stack_id >= 0` this leaves behind is still killable, and killed: unit 0 is a real unit.

The two mutants that DO survive are equivalences rather than gaps. Swapping the `facility_id` and
`project_id` declarations reorders two uninitialised locals that are both assigned before either
is read. Turning `repaired >= damage` into `repaired > damage` changes only the case where they
are already equal, where the guarded statement assigns a variable to itself.

Bug note: the Fungal Tower's owner is reset to faction 0 (the native-life AI) on every repair
phase of whichever faction currently owns it, at 0x005261AF, BEFORE the second pass filters on
the faction byte. A captured Fungal Tower therefore reverts to Planet and is then skipped by its
captor's own redraw pass in the same call. Transcribed as written.
*/
void __cdecl repair_phase(int faction_id) {
    PlayersData[faction_id].sat_odp_deployed = 0;
    for (int veh_id = 0; veh_id < VehCurrentCount; veh_id++) {
        if (Vehs[veh_id].faction_id != faction_id) {
            continue;
        }
        int x = Vehs[veh_id].x;
        int y = Vehs[veh_id].y;
        // `int`, not `uint8_t`: the image zero-extends the byte on the way in
        // (`xor eax, eax / mov al, [esi + 0x952838]`) and spills a DWORD local,
        // where a `uint8_t` local spills a byte and drops the zero-extension.
        int damage_before = Vehs[veh_id].dmg_incurred;
        Vehs[veh_id].unk_6 = 0;
        Vehs[veh_id].moves_expended = 0;
        Vehs[veh_id].state &= ~(VSTATE_UNK_2 | VSTATE_UNK_2000 | VSTATE_CRAWLING);
        Vehs[veh_id].flags &= (uint16_t)~VFLAG_UNK_1000;
        // One unit in four each turn, cycling by unit id so the whole table is covered every
        // fourth turn rather than all at once.
        if (!((TurnCurrentNum + veh_id) & 3)) {
            Vehs[veh_id].state &= ~VSTATE_UNK_800;
            // Tested NEGATED, with the arms the other way round: the image's `jne`
            // goes to the `&= ~VFLAG_UNK_2` arm and FALLS THROUGH to
            // `&= ~VFLAG_UNK_1`, which is what the negated test produces.
            uint16_t flags = Vehs[veh_id].flags;
            if (!(flags & VFLAG_UNK_2)) {
                flags &= (uint16_t)~VFLAG_UNK_1;
            } else {
                flags &= (uint16_t)~VFLAG_UNK_2;
            }
            Vehs[veh_id].flags = flags;
        }
        // "Hold 10" stores its countdown in the first waypoint's y; plain Hold stores zero there
        // and so never expires.
        if (Vehs[veh_id].order == ORDER_SENTRY_BOARD || Vehs[veh_id].order == ORDER_HOLD) {
            int16_t countdown = Vehs[veh_id].waypoint_y[0];
            if (countdown) {
                countdown--;
                Vehs[veh_id].waypoint_y[0] = countdown;
                if (!countdown) {
                    Vehs[veh_id].order = ORDER_NONE;
                }
            }
        }
        if (Vehs[veh_id].state & VSTATE_UNK_8) {
            Map *tile = map_loc(x, y);
            BOOL is_sheltered = false;
            if (tile->bit & BIT_BASE_IN_TILE) {
                int owner = (int)owner_at(x, y);
                if (owner < MaxPlayerNum && owner >= 0) {
                    is_sheltered = true;
                }
            }
            if (!is_sheltered) {
                uint32_t triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
                if (triad == TRIAD_LAND && (tile->bit & BIT_BUNKER)) {
                    is_sheltered = true;
                } else if (triad == TRIAD_AIR && (tile->bit & BIT_AIRBASE)) {
                    is_sheltered = true;
                }
            }
            if (is_sheltered) {
                Vehs[veh_id].state &= ~VSTATE_UNK_8;
            }
        }
        if (Vehs[veh_id].proto_id == BSC_FUNGAL_TOWER) {
            Vehs[veh_id].faction_id = 0;  // see the bug note above
        }
        int proto_id = Vehs[veh_id].proto_id;
        if (proto_id == BSC_BATTLE_OGRE_MK1 || proto_id == BSC_BATTLE_OGRE_MK2
            || proto_id == BSC_BATTLE_OGRE_MK3) {
            continue;  // the Unity Ogres never repair
        }
        if (proto_id != BSC_FUNGAL_TOWER && (Vehs[veh_id].state & VSTATE_UNK_4)) {
            continue;
        }
        if (!Vehs[veh_id].dmg_incurred) {
            continue;
        }
        int repair_rate = 1;
        if (proto_id != BSC_FUNGAL_TOWER) {
            if (get_proto_offense_rating(proto_id) < 0 && proto_id < MaxVehProtoFactionNum) {
                Map *tile = map_loc(x, y);
                if ((tile->bit & BIT_FUNGUS) && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
                    repair_rate = 2;  // native life regenerates in fungus
                }
            }
            if (whose_territory(faction_id, x, y, NULL, false) == faction_id) {
                repair_rate++;
            }
            uint32_t triad = Chassis[VehPrototypes[proto_id].chassis_id].triad;
            if (triad == TRIAD_AIR && (map_loc(x, y)->bit & BIT_AIRBASE)) {
                repair_rate++;
            }
            if (triad == TRIAD_LAND && (map_loc(x, y)->bit & BIT_BUNKER)) {
                repair_rate++;
            }
        }
        int base_id = base_at(x, y);
        if (base_id >= 0 && !(Bases[base_id].state & BSTATE_DRONE_RIOTS_ACTIVE)) {
            repair_rate++;
            if (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0
                && Vehs[veh_id].proto_id < MaxVehProtoFactionNum) {
                // Native life is healed outright by any base whose owner has a lifecycle bonus,
                // and the bonus is the base owner's rather than the unit owner's.
                if (breed_mod_call(base_id, Bases[base_id].faction_id_current)) {
                    repair_rate = Vehs[veh_id].dmg_incurred;
                }
            } else {
                uint32_t triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
                int facility_id;
                int project_id;
                if (triad == TRIAD_SEA) {
                    facility_id = FAC_NAVAL_YARD;
                    project_id = SP_MARITIME_CONTROL_CENTER;
                } else if (triad == TRIAD_AIR) {
                    facility_id = FAC_AEROSPACE_COMPLEX;
                    project_id = SP_CLOUDBASE_ACADEMY;
                } else {
                    facility_id = FAC_COMMAND_CENTER;
                    project_id = SP_COMMAND_NEXUS;
                }
                if (has_fac_built_call(facility_id, base_id) || has_project(project_id, faction_id)) {
                    repair_rate = Vehs[veh_id].dmg_incurred;
                }
            }
        }
        BOOL has_repair_bay = false;
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_LAND
            && veh_cargo(veh_id)
            && (Vehs[veh_id].order == ORDER_SENTRY_BOARD
                || altitude_at(x, y) < ALT_BIT_SHORE_LINE)) {
            // veh_top (0x00579920) HAND-INLINED, matching the two sites in
            // veh.cpp that already do it: the image writes the climb out at
            // 0x005264A8-0x005264C6 rather than calling it, because veh.cpp is
            // a different translation unit and nothing can inline across one.
            // Its `veh_id < 0` guard is dead here - the argument is this loop's
            // own index - which is why the transcription starts at the climb.
            int top_veh_id = veh_id;
            for (int16_t prev = Vehs[top_veh_id].prev_veh_id_stack; prev >= 0;
                prev = Vehs[prev].prev_veh_id_stack) {
                top_veh_id = prev;
            }
            for (int stack_id = top_veh_id; stack_id >= 0;
                stack_id = Vehs[stack_id].next_veh_id_stack) {
                if (stack_id != veh_id && has_abil(Vehs[stack_id].proto_id, ABL_REPAIR)) {
                    has_repair_bay = true;
                }
            }
            if (has_repair_bay) {
                repair_rate *= 2;
            }
        }
        if (has_project(SP_NANO_FACTORY, faction_id)) {
            repair_rate = Vehs[veh_id].dmg_incurred;
        }
        int reactor_id = VehPrototypes[Vehs[veh_id].proto_id].reactor_id;
        int repair_total = reactor_id * repair_rate;
        int min_damage = 0;
        if (faction_id && !has_repair_bay && base_id < 0
            && !has_project(SP_NANO_FACTORY, faction_id)) {
            min_damage = reactor_id * 2;
            if (min_damage) {
                Map *tile = map_loc(x, y);
                if ((tile->bit & BIT_FUNGUS) && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
                    if (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0
                        && Vehs[veh_id].proto_id < MaxVehProtoFactionNum) {
                        min_damage = 0;
                    }
                    if (has_project(SP_XENOEMPATYH_DOME, faction_id)) {
                        min_damage = 0;
                        repair_total += reactor_id;
                    }
                }
            }
        }
        int damage = Vehs[veh_id].dmg_incurred;
        int repaired = range(damage - repair_total, min_damage, 999);
        if (repaired >= damage) {
            repaired = damage;  // repairing never adds damage
        }
        Vehs[veh_id].dmg_incurred = (uint8_t)repaired;
        // Wake a human player's sentry once it is as healed as it is going to get, but only in
        // the open: a land unit at sea is aboard a transport and stays boarded.
        if (Vehs[veh_id].dmg_incurred <= min_damage && is_human(faction_id)
            && Vehs[veh_id].order == ORDER_SENTRY_BOARD
            && Vehs[veh_id].dmg_incurred != damage_before) {
            if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != TRIAD_LAND
                || altitude_at(x, y) >= ALT_BIT_SHORE_LINE) {
                Vehs[veh_id].order = ORDER_NONE;
            }
        }
    }
    for (veh_id = 0; veh_id < VehCurrentCount; veh_id++) {
        int veh_faction_id = Vehs[veh_id].faction_id;
        if (veh_faction_id != faction_id) {
            continue;
        }
        int x = Vehs[veh_id].x;
        int y = Vehs[veh_id].y;
        Map *tile = map_loc(x, y);
        if (tile->bit & BIT_BASE_IN_TILE) {
            int owner = (int)owner_at(x, y);
            if (owner < MaxPlayerNum && owner >= 0) {
                continue;  // the base's own draw covers it
            }
        }
        if (veh_faction_id != ConsoleGlobal.faction_id_
            && !(Vehs[veh_id].visibility & (1 << ConsoleGlobal.faction_id_))) {
            continue;
        }
        draw_tile_call(x, y, -1);
    }
    do_all_draws();
}

// ---------------------------------------------------------------------------
// Merged from the retired src/game_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------

/*
Purpose: The ceiling on a single energy allocation slider. The legacy
         implementation is a constant return.
// ORIGINAL: 0x00445440 ?energy_limit@@YAHH@Z 0x00445440-0x00445446 BYTE_EXACT
// size      6 bytes
// prototype int (__cdecl ?energy_limit@@YAHH@Z)(int factionID)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 10
Status: Complete
*/
// BODY IN game.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


// ---------------------------------------------------------------------------
// The main loop.
// ---------------------------------------------------------------------------

// Loop state globals; addresses and roles documented in
// game.h next to each declaration.
int GameRestartQueued;  // 0x0093A948
int ExitTurnLoop;       // 0x009B2068
// Earlier recovery interpreted 0x009A6504/0x009A6508 as independent
// EndgameFactionSelectorA/B scalars. They are FactionRankings[6]/[7],
// owned in alpha.cpp: rankings writes all eight slots, and setup_player
// reads these slots at 0x005B231F/0x005B2300. Keep the historical
// interpretation here; separate globals would sever producer/consumer state.
// RECOVERY_OBJECT: v1 0x009B206C ?EndgameExitReason@@3HA
int EndgameExitReason;
// Original interlude reads/writes this complete story-mask word.
// RECOVERY_OBJECT: v1 0x009A64A4 ?GameInterludeFlags@@3IA
uint32_t GameInterludeFlags;
// Wire0 and game_data transfer these exact independent byte intervals.
// No direct semantic use or original allocation boundary is proved; they are
// retained state, never padding or zero-producing packet filler. Keep the
// filename tail separate from the exactly bounded legacy 256-byte copy.
// RECOVERY_OBJECT: v2 0x009A64A0 0x009A64A4 ?GameWireOpaque_9A64A0@@3PAEA 0x00532E00
uint8_t GameWireOpaque_9A64A0[4];
// RECOVERY_OBJECT: v2 0x009A64A8 0x009A64AC ?GameWireOpaque_9A64A8@@3PAEA 0x00532E00
uint8_t GameWireOpaque_9A64A8[4];
// Native game_data5A6BA3..5A6BB9 copies exactly256 bytes; setup_game
// clears only byte0. NUL-scanning consumers do not prove loaded termination.
// RECOVERY_OBJECT: v2 0x009A66C9 0x009A67C9 ?LegacyGameSaveName@@3PADA 0x005A6AC0
char LegacyGameSaveName[256];
// RECOVERY_OBJECT: v2 0x009A67C9 0x009A67CC ?GameWireOpaque_9A67C9@@3PAEA 0x00532E00
uint8_t GameWireOpaque_9A67C9[3];
// Logical faction0..7 indexing is independently observed; original
// containing allocation bounds are not asserted.
// RECOVERY_OBJECT: v1 0x00946138 ?TectonicDetonationCount@@3PAIA
uint32_t TectonicDetonationCount[8];
// Original691B0C contains pointer691B7C, whose file-backed text is SCRIPT.
// This owns the observed text and shared pointer cell; original allocation
// bounds remain unresolved rather than inferred from the current array.
// RECOVERY_OBJECT: v1 0x00691B7C ?ScriptPopupFileText@@3PADA
char ScriptPopupFileText[] = "SCRIPT";
// RECOVERY_OBJECT: v1 0x00691B0C ?ScriptPopupFile@@3PADA
char *ScriptPopupFile = ScriptPopupFileText;
int ControlTurnPhase;   // 0x009B2070
char SaveGameFileName[0x104];  // 0x0093AA0C
char SaveNameBuffer[0x104];    // 0x009B2078

// The window the loop paints colour 9 over and hides while the game boots,
// showing it again for a PBEM session - REAL STORAGE, not the
// `GraphicWin *const CoverWindow = (GraphicWin *)0x00937118` binding that
// named terranx.exe data, unmapped in a standalone build. Its class is not
// recovered - the artifacts reach it as a GraphicWin (fill) and a Win
// (show/hide), which is what this object's type says. The image names it
// with `mov ecx, 0x937118` immediates; the object keeps that fold, the
// displacement relocated. time.cpp's go_reset hides it by name too (its
// go_cover_window() helper is retired).
GraphicWin CoverWindow;  // 0x00937118

// The multiplayer window - REAL STORAGE at the image's 0x007FD648, not the
// `GraphicWin *const MultiWindow` binding (see game.h). Its class is
// MultiWin. The source now uses its real C++ constructor and destructor;
// their placement in the original initializer/finalizer sequence is still
// an independent restoration obligation.
// Its 0x28B8-byte owned layout ends at the independently catalogued
// g_MULTIWIN_WAVE pointer at 0x007FFF00. The game object file emits
// ?MultiWindow@@3VMultiWin@@A with real constructor/destructor references.
// RECOVERY_OBJECT: v2 0x007FD648 0x007FFF00 ?MultiWindow@@3VMultiWin@@A 0x0047A590
MultiWin MultiWindow;  // 0x007FD648

// 0x00689218 - the DirectPlay application GUID the lobby check registers the
// session under, bytes read off the image. The trailing words spell "DEST".
const GUID LobbyAppGuid = {0x02D1B480, 0xD680, 0x11D0,
                                  {0x82, 0x1E, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

/*
Purpose: Run the game. Brings the system and game up, decides the intro
         movie, then turns the wheel: menu or multiplayer handshake, desktop,
         one turn, teardown - until nothing queues another game.
// ORIGINAL: 0x0052AA30 ?control_game@@YAXXZ 0x0052AA30-0x0052AD2D FILE
// TRIED: MISMATCH #4 push/call - straight g_* global derefs and direct member calls gave the allocator no reason to hold ebx/esi/edi live across the whole body the way the original's cached fn-ptr (ebx) and zero/flag temps (esi/edi) do.
// LEVER: /Oy- flag set, not /O2 alone - the image keeps the ebp frame, and
// under /Oy- this body's prologue, spills ([ebp-8] for lobby_status) and
// tail all line up (2026-08-29: 19/215, best-similar set).
// LEVER: a `short (__stdcall *)(int)` local seeded once from GetAsyncKeyState
// is what holds the import across both Shift tests; a straight
// GetAsyncKeyState(x) at each site costs the load twice.
// LEVER: the loop clears lobby_status after the menu/multiplayer branch
// (`mov [ebp-8], esi` at 0x0052AC7D) - only the first pass runs the lobby
// handshake. The first transcription dropped that store; without it the
// image's re-read of [ebp-8] has no writer. NetTurnFlags went uint8_t ->
// uint32_t on the same pass: the image clears it with a whole dword.
// TRIED: `int window_flags = 0x8C0;` before the loop, volatile RMW, and the
// bare literal - all fold to `or reg, 0x8c0` where the image materializes
// 0x8C0 into a dead callee-saved register before the loop (`mov ebx, 0x8c0`
// at 0x0052AB8D) and ors through it at 0x0052ACDD. Not reached from source.
// TRIED: this tree's VC6 caches the literal 1 in a callee-saved register
// from the first `game_init(0, 1)` (`mov edi, 1` ahead of the prologue
// pushes) where the image pushes the immediate every time; spelling the call
// args, dropping the window_flags local, and both flag families leave it
// unchanged.
Status: homed from src/unrecovered/0052aa30.cpp on 2026-08-29. MISMATCH
        against the best-similar flag set, /O2 /Gy /GR- /Oy- /GX; the
        remaining runs are register-role permutation (image ebx/esi/edi =
        fn-ptr+0x8C0/zero/found-to-minus-1; this tree shifts the roles) and
        the two TRIED folds. Callees still original are forwarders in
        pending_bodies.cpp.
*/
void __cdecl control_game() {
    // The image loads the import once into a callee-saved register and calls
    // it twice (Shift before filefind, Shift before FX::init).
    typedef short(__stdcall *get_key_state_fn)(int);
    get_key_state_fn get_key_state;

    int lobby_status;
    HKEY hkey;
    unsigned long value_type;
    unsigned char value_data[12];
    int is_complete;
    int movie_disabled;
    unsigned long value_length;

    if (system_init() != 0) {
        return;
    }
    if (game_init(0, 1) != 0) {
        return;
    }

    GameRestartQueued = 0;
    WorldClimateSkipTerrainClear = 1;
    ExitTurnLoop = 0;
    IsMultiplayerNet = 0;
    lobby_status = NetDaemonNet.Net::check_for_lobby(CommandLineText,
                                                      const_cast<_GUID *>(&LobbyAppGuid), 3, 9);
    get_key_state = GetAsyncKeyState;

    if (lobby_status == 0) {
        // Holding Shift skips the lobby's "is the intro complete" registry
        // look-up and takes the movie path directly.
        if (HIBYTE(get_key_state(VK_SHIFT)) == 0) {
            is_complete = 0;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                              "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\"
                              "Sid Meier's Alpha Centauri",
                              0, 0, &hkey) == 0) {
                value_length = 10;
                if (RegQueryValueExA(hkey, "Complete", 0, &value_type, value_data,
                                     &value_length) == 0
                    && _strcmpi((const char *)value_data, "Yes") == 0) {
                    is_complete = 1;
                }
                RegCloseKey(hkey);
            }
        } else {
            is_complete = 1;
        }
        filefind_init("movies\\virtualWorld.wve", is_complete);

        prefs_load(0);
        GamePreferences = AlphaIniPrefs.preferences;
        movie_disabled = prefs_get("DisableOpeningMovie", 0, 0);
        prefs_put("DisableOpeningMovie", movie_disabled, 0);
        if (movie_disabled == 0 && (GamePreferences & 0x10000000) != 0) {
            amovie_project("openingx");
        }
    }

    if ((get_key_state(VK_SHIFT) & 0x8000) == 0) {
        g_FX.init();
    }
    for (;;) {
        if (GameRestartQueued != 0) {
            if (g_WAVE_GENERAL.Wave::is_playing() != 0) {
                g_WAVE_GENERAL.Sound::fade(2000);
            }
            ConsoleGlobal.Win::hide();
            do_all_draws();
            strcpy(SaveNameBuffer, SaveGameFileName);
            filefind_set_alternative(SaveNameBuffer);
            if (game_reload(0, 1) != 0) {
                goto close_game;
            }
            VehCurrentCount = 0;
            setup_game(1);
            GameRules = AlphaIniPrefs.rules;
            NetTurnFlags = 0;
        }

        GameRestartQueued = 0;
        WorldClimateSkipTerrainClear = 1;
        ExitTurnLoop = 0;
        IsMultiplayerNet = 0;
        flush_input();
        if (lobby_status == 0) {
            if (top_menu(0) != 0) {
                goto close_opening;
            }
        } else {
            prefs_load(0);
            prefs_use();
            if (multiplayer_init(1) != 0) {
                goto close_opening;
            }
        }

        // Only the first pass runs the lobby handshake; after the menu or the
        // multiplayer branch the loop clears it and every later pass comes
        // through top_menu.
        lobby_status = 0;

        if (IsMultiplayerPBEM != 0) {
            CoverWindow.fill(0);
            CoverWindow.Win::show(0);
        } else {
            CoverWindow.fill(9);
            CoverWindow.Win::hide();
        }
        if (desktop_init(ControlTurnPhase == 0) != 0) {
            goto close_opening;
        }
        {
            // The image loads the primary map-window slot before it stores
            // the skip flag, then ors the live-flag dword through a register.
            // The flag dword sits above the mapped fields; raw offset, the
            // way draw_tile reads its neighbour.
            MapWin *const primary_window = MapWinTable[0];
            WorldClimateSkipTerrainClear = 0;
            uint32_t flags = *reinterpret_cast<volatile uint32_t *>(
                reinterpret_cast<uint8_t *>(primary_window) + 0x1DD70);
            // The image materializes 0x8C0 into a dead callee-saved
            // register before the loop and ors through it; every spelling of
            // that constant this tree has tried folds to the immediate
            // instead (TRIED: note under the marker).
            flags |= 0x8C0;
            *reinterpret_cast<volatile uint32_t *>(
                reinterpret_cast<uint8_t *>(primary_window) + 0x1DD70) = flags;
        }
        ConsoleGlobal.focus_vehicle_ = -1;
        if (IsMultiplayerNet != 0) {
            net_control_turn();
        } else {
            control_turn();
        }
        desktop_close();
        if (GameRestartQueued == 0) {
            goto close_game;
        }
    }

close_opening:
    close_opening();
close_game:
    game_close(0);
    system_close();
}

// ---------------------------------------------------------------------------
// The teardown half of control_game's wheel, homed 2026-08-29 from
// src/recovered/units/ (0058eff0, 00589b20, 0058f430). Their forwarders in
// pending_bodies.cpp are gone; these definitions own the symbols now.
// ---------------------------------------------------------------------------

/*
Purpose: Tear the desktop down at the end of each main-loop pass.
// ORIGINAL: 0x0058EFF0 ?desktop_close@@YAXXZ 0x0058EFF0-0x0058F031 FILE BYTE_EXACT
// size      65 bytes
// prototype
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00408710 0x0043C1A0 0x004710E0 0x004B9F80 0x005D4E40
// LEVER: byte-exact on promotion. The fixed windows are REAL OBJECTS now
//   (TutWinDesWindow, TutWinBaseWindow in their domain .cpps; ConsoleStatusWin
//   in statuswin.cpp; MainInterfaceGlobal in mapwin.cpp for TutWinIfaceWindow)
//   and still fold each receiver to `mov ecx, imm32`, the displacement
//   relocated; `Win::hide` is qualified to keep the slot-2 virtual a direct
//   tail `jmp`, and `GraphicWin::close` on the WorldWin and on MultiWindow's
//   front is qualified so a later base-class declaration cannot steal the
//   call from the direct E8 the image has. WorldClimateWorldWin is declared
//   in worldwin.h and defined in map.cpp, so both bodies name the object -
//   same fold as the old by-value world_climate_window() helper, one binding
//   name instead of two.
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_close() {
    TutWinDesWindow.close();
    TutWinBaseWindow.close();
    mapwin_system_shutdown();
    ConsoleStatusWin.close();
    WorldClimateWorldWin.GraphicWin::close();
    // The MultiWin object's GraphicWin front - the image closes that base
    // directly (`call 0x5d4e40` on `mov ecx, 0x7fd648`).
    reinterpret_cast<GraphicWin *>(&MultiWindow)->close();
    MainInterfaceGlobal.Win::hide();
}

/*
Purpose: Swap the opening logo the desktop shows: load the PCX named a1 into
         a local Buffer, blit it into the opening GraphicWin's Buffer base
         from the decoded DIB, then call the window's virtual on_update
         (Win slot 63) so it repaints.
         Returns 7 when no opening window is up.

// ORIGINAL: 0x00589A30 ?change_opening@@YAHPAD@Z 0x00589A30-0x00589B1D;0x00660255-0x0066026A
// TRIED (seed): reproducing the SEH frame/unwind funclet for the local
//            `Buffer buf` explicitly - a real local of non-trivial type
//            (ctor/dtor declared, undefined) makes MSVC generate its own
//            construction/destruction and unwind automatically; landed
//            relying on that instead of hand-writing the funclet.
// size      258 bytes
// prototype int (__cdecl ?change_opening@@YAHPAD@Z)(int8* a1)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005D9610
Return Value: 0 when the opening swapped, 7 when there is no opening window
The second ORIGINAL span is the local Buffer's unwind funclet and scope
descriptor. opening_window() still reaches original storage; its producer and
lifetime consumers need a coordinated migration. The window is reloaded after
load_pcx and before on_update, as in the image. The retained seed's SEH attempt
relies on the compiler-generated unwind for the real local Buffer rather than
hand-writing a funclet.
*/
int __cdecl change_opening(char *filename) {
    Buffer buf;
    if (opening_window() == 0) {
        return 7;
    }
    buf.load_pcx(filename, 0, 10, 0xec);
    GraphicWin *window = opening_window();
    Buffer *surface = window != 0 ? static_cast<Buffer *>(window) : 0;
    int c1 = surface->dib_.bmiHeader.biWidth;
    int c2 = -surface->dib_.bmiHeader.biHeight;
    int p1 = buf.dib_.bmiHeader.biWidth;
    int p2 = -static_cast<int>(buf.dib_.bmiHeader.biHeight);
    buf.copy(surface, 0, 0, p1, p2, 0, 0, c1, c2);
    opening_window()->on_update();
    return 0;
}

/*
Purpose: Publish the Console's Win base for popup parenting, then destroy
         the owned opening window and clear its pointer.
// ORIGINAL: 0x00589B20 ?close_opening@@YAXXZ 0x00589B20-0x00589B57 FILE BYTE_EXACT
// size      55 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00589B4A
// The original converts Console through MapWin's virtual GraphicWin base.
// At 0x00589B4A it calls the opening object's scalar deleting destructor
// with flag one, not on_dialog_focus. GraphicWin's original primary table
// at 0x0066FC50 selects 0x005D7140, which destroys and frees the allocation.
Return Value: n/a
*/
void __cdecl close_opening() {
    Win *console_window = &ConsoleGlobal;
    GraphicWin *opening = opening_window();
    // Publish the ordinary C++ conversion to the fallback parent window.
    BasePopDefaultField322C = console_window;
    if (opening != 0) {
        delete opening;
        opening_window() = 0;
    }
}

// Out-of-line forwarders for game_close and game_reload: the image CALLS
// stop_timers (0x0050F440) and labels_shutdown (0x006169D0) at both sites, and
// both helpers are `MEASURED inline` in time.h and alpha.h, so at /O2 the
// optimiser folds them - and the Time::close runs and the Labels free that
// they carry - straight into the caller. `#pragma auto_inline(off)` is what
// VC6 6.0 has instead of `__declspec(noinline)`; see repair_phase's three
// forwarders below for the same device.
#pragma auto_inline(off)
static void __cdecl stop_timers_call() {
    stop_timers();
}
static void __cdecl labels_shutdown_call() {
    labels_shutdown();
}
#pragma auto_inline(on)

/*
Purpose: Shut the game state down: timers, label strings, then the string
         table. The mode parameter is read by none of it.
// ORIGINAL: 0x0058F430 ?game_close@@YAXH@Z 0x0058F430-0x0058F444 FILE BYTE_EXACT
// size      20 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0050F440 0x006169D0
// LEVER: byte-exact on promotion. StringTable is strings.h's header binding,
//   so the receiver folds to `mov ecx, 0x9b90d8`; the image's closing `jmp`
//   into Strings::shutdown comes free from the call being the last statement
//   of a void function, and the discarded int result costs nothing.
// LEVER (2026-09-03, pair normalisation): `stop_timers_call` and
//   `labels_shutdown_call`, not the helpers directly - the image CALLS
//   0x0050F440 and 0x006169D0 here and both are `MEASURED inline` in time.h
//   and alpha.h, so at /O2 the optimiser folded them (and the Time::close runs
//   and the Labels free they carry) straight into this 4-instruction body,
//   which became 67. Same-TU forwarders under `#pragma auto_inline(off)`, the
//   device repair_phase's three helpers already use; game_reload shares them.
//   0 of 4 -> 4 of 4.
Return Value: n/a
Status: Complete
*/
void __cdecl game_close(int mode) {
    stop_timers_call();
    labels_shutdown_call();
    StringTable.shutdown();
}

/*
Purpose: Bring the shared system up, once per process: create the four data
         directories, read the language preference and load the string and
         label tables, upper-case the two default button captions out of them,
         load the fixed art, then build the eight map-window slots and the
         desktop's standing windows.
// ORIGINAL: 0x0058F040 ?system_init@@YAHXZ 0x0058F040-0x0058F245;0x00660BD7-0x00660BEC
// size      538 bytes
// prototype int (__cdecl ?system_init@@YAHXZ)(void)
// callers   1   call targets   23
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004219E0 0x00429300 0x00455E80 0x004595C0 0x00459680 0x004626E0 0x00471D50 0x004BA220 0x0051D890 0x00588AA0 0x0059DB40 0x005C9860 0x006018A0 0x00601910 0x006168F0 0x006169A0 0x00616A00 0x00626450 0x00627100 0x00645470 0x0064557F 0x0064558A 0x00646829
// indirect  0x0058F11A 0x0058F159
// LEVER: the SEH frame the archive unit could not reproduce is the
//   `new MapWin` below it. MapWin has a real destructor (mapwin.h), so /GX
//   makes VC6 wrap the whole function - `push -1 / push handler / fs:[0]` at
//   instruction 0, the `[ebp-4]` state toggle around the constructor and the
//   `operator delete` funclet at 0x660BD7 - which is exactly the frame the
//   artifact's plain operator-new-plus-placement-new spelling was missing.
//   `new MapWin` with no argument is also what produces the image's
//   `push 1` before the ctor call: that 1 is the hidden most-derived flag,
//   not the catalogue's `H`.
// LEVER: the two captions are `label_get` (alpha.h's forceinline of
//   Strings::get over Labels.strings_ptr) at offsets 102 and 101, which is the
//   image's `[strings_ptr + 0x198]` and `[+ 0x194]`.
// LEVER: slot 0 of the map-window table is published last, as the Console -
//   the image's single `mov dword ptr [0x7d3c3c], 0x9156b0` - and the seven
//   slots above it are the ones the allocation loop walks (0x7D3C40 to
//   0x7D3C5C exclusive), so the loop is `&MapWinTable[1] .. &MapWinTable[8]`
//   and not the whole table.
// TRIED: `static_cast<MultiDebug *>(&WinBubbleWindow)` and a named extern for
//   the MessageWin storage - the image has no MessageWin object in this tree
//   (messagewin.cpp defines nothing at 0x7F67F8) and WinBubbleWindow is a
//   `Win`, so both stay address spellings, the way game_reload reaches
//   0x008E9F60.
// TRIED: set_language as a real `call` - alpha.h spells it `MEASURED inline`,
//   so VC6 expands it to `mov [Language], eax` where the image has
//   `push eax / call 0x627100`. `#pragma auto_inline(off)` is C2156 inside a
//   function, so this tree cannot scope it to the one site, and alpha.h is not
//   this batch's file. One instruction.
// TRIED: the two caption builds spelled `label_get(102)` / `label_get(101)`
//   instead of the local `LPSTR *labels` plus `StringTable.get(labels[n])` now
//   in the body - that spelling scores 32/135 against this one's 34, and
//   game_init measures the same either way, so the two bodies legitimately
//   spell it differently. What is left is the register the label value lands
//   in and the register holding `Labels.strings_ptr` across the StringTemp
//   store (image eax/eax, ours eax/ecx on the first caption), plus the
//   `SystemInterfaceFlags` read-modify-write, which the image interleaves with
//   maininterface_init's argument setup and runs in esi while ours runs it as
//   a block in edx and re-derives the zero twice. All register assignment;
//   /Ob0 scores worse (53/135) because map_win_table and friends turn into
//   real calls.
Return Value: 1 when the label table fails to load, else 0.
*/
int __cdecl system_init() {
    _mkdir("saves");
    _mkdir("saves\\auto");
    _mkdir("maps");
    _mkdir("scenarios");
    WorldClimateSkipTerrainClear = 1;
    log_set_state(0);
    set_language(prefs_get("Language", 0, 0));
    textcolor_init();
    StringTable.init(0xC320);
    if (labels_init() != 0) {
        return 1;
    }

    // The Cancel caption, label 102: upper-cased unless the language is 2.
    LPSTR *labels = (LPSTR *)Labels.strings_ptr;
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get((int)labels[102]));
    LPSTR (WINAPI *string_upper)(LPSTR) = CharUpperA;
    if (Language != 2) {
        string_upper(StringTemp);
    }
    BasePop::set_def_cancel_text(StringTemp);

    // The OK caption, label 101: upper-cased whatever the language.
    labels = (LPSTR *)Labels.strings_ptr;
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get((int)labels[101]));
    string_upper(StringTemp);
    BasePop::set_def_ok_text(StringTemp);

    load_fixed_sprites();
    config_popups();

    // Slots 1..7. The null check and the delete-on-throw are the compiler's,
    // from the EH frame the new-expression brings in. The bound is compared as
    // an INT - the image's `cmp esi, 0x7d3c5c / jl` - because a pointer `<`
    // renders unsigned and VC6 emits `jb` for it.
    for (MapWin **slot = &MapWinTable[1]; (int)slot < (int)&MapWinTable[8];
         ++slot) {
        *slot = new MapWin;
    }

    SystemInterfaceFlags &= ~0x8C0u;
    MapWinTable[0] = &ConsoleGlobal;

    MainInterfaceGlobal.init(0);
    battlewin_init(static_cast<BattleWin *>(sub_interface_global()));
    ConsoleGlobal.init(0);

    // The language performs the Console-to-Win virtual-base conversion.
    Win *console_window = &ConsoleGlobal;
    WinBubbleWindow.init(&NetDaemonNet, console_window, ShadowColorTable);

    messagewin_init(reinterpret_cast<MessageWin *>(0x007F67F8));
    datalink_init(&DatalinkMain);
    bung_momma();
    return 0;
}

/*
Purpose: Bring a game up: reseed both random chains off the clock, then either
         just turn the log off (mode 0) or reload the string table and
         re-read the two default button captions before the art and the rules
         load. mode 0 is the first boot, anything else a reload into a live
         system.
// ORIGINAL: 0x0058F2F0 ?game_init@@YAHHH@Z 0x0058F2F0-0x0058F421
// size      305 bytes
// prototype BOOL (__cdecl ?game_init@@YAHHH@Z)(BOOL tglParseStrings, BOOL tglAllRules)
// callers   4   call targets   12
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00455EB0 0x0050F3D0 0x005873C0 0x006018A0 0x00601910 0x006168F0 0x006169A0 0x00616A00 0x006257E0 0x00626450 0x00645470
// indirect  0x0058F304 0x0058F30F 0x0058F38D 0x0058F3CD
// LEVER: the clock slot is read ONCE into a register and called twice, the
//   same shape control_game's GetAsyncKeyState lever records - holding
//   WaveTimeGetTimeSlot in a local is what keeps the load from happening
//   twice.
// LEVER: the `add esp,4` at 0x58F3D9 is one instruction serving BOTH arms -
//   log_set_state's argument on the mode==0 arm, set_def_ok_text's on the
//   other - so the source converges the two arms into one `if/else` whose
//   join the compiler cleans once.
// TRIED: naming the three caption-icon stores through sprite.cpp's array -
//   sprite.h does not declare it, and game.h cannot add a member to a header
//   it does not own, so the three store their addresses (see WinCloseIcon in
//   game.h for the identification).
// TRIED: the two caption builds spelled `LPSTR *labels` and
//   `StringTable.get(labels[n])` instead of label_get - byte-identical output,
//   so it is a free spelling, not a lever. The six runs that remain are the
//   register the label value lands in (image eax, ours ecx on the first
//   caption), the register holding `Labels.strings_ptr` for the second caption
//   (image ecx, ours edx), and the register `reload` is read into for
//   read_rules (image edx, ours ecx). Register naming only; the instruction
//   count and the call set agree exactly.
Return Value: 1 when the label table or the rules fail to load, else 0.
Status: homed from src/unrecovered/0058f2f0.cpp on this pass; its pending
        forwarder is gone. 74/82 agreeing, best set /O2 /Gy /GR- /Oy- /GX,
        156 bytes discounted as relocations.
*/
int __cdecl game_init(int mode, int reload) {
    DWORD (WINAPI *const time_get_time)() = timeGetTime;
    WorldClimateSkipTerrainClear = 1;
    my_srand(time_get_time());
    random_reseed(time_get_time());

    if (mode == 0) {
        log_set_state(0);
    } else {
        StringTable.init(0xC320);
        if (labels_init() != 0) {
            return 1;
        }
        // The same two captions system_init builds, for the same two buttons.
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(102));
        LPSTR (WINAPI *string_upper)(LPSTR) = CharUpperA;
        if (Language != 2) {
            string_upper(StringTemp);
        }
        BasePop::set_def_cancel_text(StringTemp);

        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(101));
        string_upper(StringTemp);
        BasePop::set_def_ok_text(StringTemp);
    }

    load_sprites();
    if (read_rules(reload) != 0) {
        return 1;
    }
    start_timers();

    // The three caption-button icons: sprite.cpp's array at 0x0075AE20, a 0x2c
    // stride, so these are its [1], [2] and [0]. WinRestoreIcon, the slot
    // between the last two, is not set here.
    WinCloseIcon = &g_SILVER_MENU_ICON_SPRITES[1];
    WinMaximizeIcon = &g_SILVER_MENU_ICON_SPRITES[2];
    WinMinimizeIcon = &g_SILVER_MENU_ICON_SPRITES[0];
    return 0;
}

// Original 0x00455400: palette cycles and ocean offsets are one ordered
// configuration load. A missing cycling section also skips ocean offsets;
// initialization errors do not suppress the following start_cycle call.
// Original section-name strings include their terminating NULs. The two
// "system" arguments instead use distinct original copies at 685150 and
// 685168; a pooled compiler literal is not one shared original identity.
// RECOVERY_LITERAL: v1 0x00685148 0x00685150 ??_C@_07MAH@cycling?$AA@ 0x00455400
// RECOVERY_LITERAL: v1 0x00685158 0x00685165 ??_C@_0N@GLFC@oceanoffsets?$AA@ 0x00455400
// ORIGINAL: 0x00455400 ?load_cycling@@YAXXZ 0x00455400-0x004554C0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["45540F guards both sections; 455432..45546B read three cycle arguments and call Palette init/start in order; 45547C..4554B9 accept ASCII-digit-led ocean lines with an eighty-entry bound and one trailing read. The retained seed and exact startup table remain available."], "reason": "The original ordered configuration loader owns palette cycle setup and writes the shared eighty-entry ocean shade table consumed by terrain generation."}
void __cdecl load_cycling() {
    if (text_open("system", "cycling") != 0) {
        return;
    }
    text_get();
    const int count = text_item_number();
    for (int cycle = 0; cycle < count; ++cycle) {
        text_get();
        const int first = text_item_number();
        const int colors = text_item_number();
        const unsigned long interval =
            static_cast<unsigned int>(text_item_number());
        const int palette_first = static_cast<int>(static_cast<unsigned int>(first) + 10u);
        g_PALETTE1.init_cycle(cycle, palette_first, colors, interval);
        g_PALETTE1.start_cycle(cycle);
    }
    if (text_open("system", "oceanoffsets") != 0) {
        return;
    }
    int offset = 0;
    for (char *line = text_get(); line != 0; line = text_get()) {
        if (offset >= 80) {
            break;
        }
        if (*line >= '0' && *line <= '9') {
            OceanDetailShade[offset++] = text_item_number();
        }
        // The original reads another line after the eightieth accepted
        // value, before the next iteration checks the table boundary.
    }
}

/*
Purpose: Load the art that never changes between games: the palette, the
         miscellaneous bitmaps, the interface sprites (mode 0), the voxels,
         the Deswin sprites, the colour tables, and the cycling palette, in
         that order. system_init's only art step; the per-game art load is
         game_init's load_sprites.
// ORIGINAL: 0x00455E80 ?load_fixed_sprites@@YAXXZ 0x00455E80-0x00455EA8 BYTE_EXACT
// size      40 bytes
// prototype void (__cdecl ?load_fixed_sprites@@YAXXZ)(void)
// callers   1   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00423570 0x00451030 0x00451A10 0x00454D10 0x00455610 0x00455E50
// tail      0x00455400 - load_cycling, the image's closing `jmp`, which a
//           void function's last statement compiles the same way
// LEVER: byte-exact, promoted from the src/recovered/00455e80.cpp archive
//   unit. Nothing to it but the order: seven calls, six of them `call rel32`
//   and the last a tail `jmp`, which is what `load_cycling();` in tail
//   position compiles to for a void callee.
Return Value: n/a
Status: homed on this pass; its forwarder and its archive unit are gone. Six
        of its seven callees were declared nowhere in the tree, so the
        forwarders that carry them arrived with this body (game.h); the
        seventh was already compiled under a seam name.
*/
void __cdecl load_fixed_sprites() {
    load_palette();
    load_misc();
    load_interface(0);
    load_voxels();
    load_deswin_sprites();
    colortables_init();
    load_cycling();
}

/*
Purpose: Restart the game state: tear the desktop down (mode-gated, the same
         closer sequence desktop_close runs), stop the timers, drop the label
         and string tables, re-init the game, and re-init the desktop when the
         caller asked for it. Always clears the climate skip flag.
// ORIGINAL: 0x0058F450 ?game_reload@@YAHHH@Z 0x0058F450-0x0058F4E7 BYTE_EXACT
// LEVER (2026-09-03, pair normalisation): the same two forwarders game_close
//   uses - the image CALLS 0x0050F440 and 0x006169D0 in this body too, and
//   with the `MEASURED inline` helpers folded in it was 107 compiled
//   instructions against the image's 44, 12 of 44. 44 of 44 after the switch.
// LEVER: byte-exact on promotion from the 0058f450 archive unit. The
//   closer block is desktop_close's own sequence spelled the same way, so
//   every receiver folds to its `mov ecx, imm32` and every call lands on the
//   image's E8 target; `mode` is read into esi once at entry (the image's
//   `mov esi, [ebp + 8]`) because the source uses it twice - the opening
//   guard and the desktop_init guard - exactly as the image does.
// size      151 bytes
// prototype int (__cdecl ?game_reload@@YAHHH@Z)(int, BOOL tglAllRules)
// callers   2   call targets   11
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00408710 0x0043C1A0 0x004710E0 0x004B9F80 0x0050F440 0x0058EE60 0x0058F2F0 0x005D4E40 0x005EDCD0 0x00616950 0x006169D0
Return Value: 1 when game_init fails, else 0.
Status: Complete
*/
int __cdecl game_reload(int mode, int reload) {
    if (mode != 0) {
        TutWinDesWindow.close();
        TutWinBaseWindow.close();
        mapwin_system_shutdown();
        ConsoleStatusWin.close();
        // The world-climate window, named through worldwin.h's declaration.
        WorldClimateWorldWin.GraphicWin::close();
        reinterpret_cast<GraphicWin *>(&MultiWindow)->close();
        MainInterfaceGlobal.Win::hide();
    }
    stop_timers_call();
    labels_shutdown_call();
    StringTable.shutdown();
    if (game_init(1, reload) != 0) {
        return 1;
    }
    if (mode != 0) {
        desktop_init(1);
    }
    WorldClimateSkipTerrainClear = 0;
    return 0;
}

// ORIGINAL: 0x00453F70 ?load_flags@@YAXXZ 0x00453F70-0x00454261;0x0065513A-0x0065515A FILE
// RECOVERY_BINDING: v1 {"reason":"Recover the complete faction flag sprite loader with real local lifetimes and shared palette/asset owners.","evidence":["Original453F70..454261 and unwind65513A..65515A construct fourteen local Sprites and destroy successful constructions in reverse. UNITS extracts seven pairs at y700/731; FLAGS extracts the same locals at y1/32, with repeated recolors before each extraction.","Every faction receives fourteen copies from UNITS, each followed by foreground then shadow recoloring. FLAGS replaces only factions whose live Players.rule_flags contains RFLAG_ALIEN; no expansion-aware extra guard. Color entries are reloaded after the preceding service call.","The shared PCX buffer closes once on ordinary completion, not during local Sprite exception cleanup. All service return values are ignored. Existing empty placeholder and obsolete image-string aliases are replaced; separate original-only callers and asset startup remain unfinished."]}

// The original owns two distinct UNITS.PCX strings, passed by the flag
// and vehicle loaders respectively. Named constant arrays preserve those
// separate storage identities without old-image pointers or literal pooling.
// RECOVERY_OBJECT: v2 0x00685024 0x0068502E _FlagUnitSheet 0x00453F70
static const char FlagUnitSheet[] = "UNITS.PCX";
// RECOVERY_OBJECT: v2 0x00685058 0x00685062 _VehicleUnitSheet 0x00454270
static const char VehicleUnitSheet[] = "UNITS.PCX";

void __cdecl load_flags() {
    // RECOVERY_LITERAL: v1 0x00685030 0x0068503A ??_C@_09KHIG@FLAGS?4PCX?$AA@ 0x00453F70
    Sprite source[14];
    Buffer &sheet = g_PCX_PARSE_TEMP_BUFFER1;
    sheet.load_pcx(FlagUnitSheet, 0, 10, 236);
    int kind;
    int frame;
    int faction;
    for (kind = 0; kind < 7; ++kind) {
        for (frame = 0; frame < 2; ++frame) {
            sheet.change_color(0x107, 0x109);
            sheet.change_color(0x106, 0x109);
            source[kind * 2 + frame].extract(
                &sheet, 0x109, 1 + kind * 24, 700 + frame * 31, 23, 30, 0);
        }
    }
    for (faction = 0; faction < 8; ++faction) {
        for (kind = 0; kind < 14; ++kind) {
            Sprite &destination = g_FLAGS_VEH_SPRITES[faction * 14 + kind];
            source[kind].copy(&destination);
            destination.change_color(0x104, InfoFactionTextColors[faction]);
            destination.change_color(0x103, InfoFactionShadowColors[faction]);
        }
    }
    sheet.load_pcx("FLAGS.PCX", 0, 10, 236);
    for (kind = 0; kind < 7; ++kind) {
        for (frame = 0; frame < 2; ++frame) {
            sheet.change_color(0x107, 0x109);
            sheet.change_color(0x106, 0x109);
            source[kind * 2 + frame].extract(
                &sheet, 0x109, 1 + kind * 24, 1 + frame * 31, 23, 30, 0);
        }
    }
    for (faction = 0; faction < 8; ++faction) {
        // Original tests the rule bit directly, not the expansion-aware helper.
        if (Players[faction].rule_flags & RFLAG_ALIEN) {
            for (kind = 0; kind < 14; ++kind) {
                Sprite &destination = g_FLAGS_VEH_SPRITES[faction * 14 + kind];
                source[kind].copy(&destination);
                destination.change_color(0x104, InfoFactionTextColors[faction]);
                destination.change_color(0x103, InfoFactionShadowColors[faction]);
            }
        }
    }
    sheet.close();
}

// ORIGINAL: 0x00454270 ?load_vehicles@@YAXXZ 0x00454270-0x004547DB;0x0065515A-0x00655182
// RECOVERY_BINDING: v1 {"evidence": ["Original454270..4547DB plus unwind65515A..655182 constructs Sprite[2] then a singleton, and destroys them in reverse including partial unwind. All are real even though apparently unused; the shared PCX buffer is not a local lifetime.", "XI is9x16 sprites, XF is4 pairs of8 with receiver indices16*row+column and +8, not seed pointer+0x98. Original CRT construction/destruction independently bounds each Sprite bank. Four sheets perform244 extracts,74 Buffer recolors and37354 pixel queries before load_flags.", "Original palette loop only updates faction0, publishing each of four get_pixel results immediately into distinct eight-color owners. Two UNITS.PCX strings have distinct original storage685024 and685058; named constant arrays preserve the identities without old-image access or forced literal pooling.", "UNITS rows populate chassis9..18 with1,7 or3 variants; marker defaults and repeated get_pixel calls follow original order. The vertical marker scan uses sourceX-2/-1, not seed sourceY. Final chassis0..8 marker-Y stores target slot1 at4546CC while X uses slot0; no normalization or omitted ignored probes.", "load_flags is called after all marker scans; it closes the shared buffer once, followed by two further closes here. Service errors are ignored as in the original and exceptions only destroy locals. Existing unfinished loader/callee/runtime work remains separate; no byte claim or whole asset-startup acceptance."], "reason": "Recover the complete vehicle asset loader through genuine Sprite banks, chassis marker fields and shared palette owners."}
void __cdecl load_vehicles() {
    // RECOVERY_LITERAL: v1 0x0068503C 0x00685043 ??_C@_06FPN@XI?4PCX?$AA@ 0x00454270
    // RECOVERY_LITERAL: v1 0x00685044 0x0068504B ??_C@_06GECN@XF?4PCX?$AA@ 0x00454270
    // RECOVERY_LITERAL: v1 0x0068504C 0x00685058 ??_C@_0M@BHKA@PALETTE?4PCX?$AA@ 0x00454270
    // These apparently unused objects are real original lifetimes: two array
    // elements first, then a separate Sprite; exception cleanup reverses this.
    Sprite scratch[2];
    Sprite temporary;
    Buffer &sheet = g_PCX_PARSE_TEMP_BUFFER1;
    sheet.load_pcx("XI.PCX", 0, 0, 18);
    sheet.change_color(253, 255);
    int row;
    int column;
    int chassis;
    int variant;
    int x;
    int y;
    int marker_x;
    int marker_y;
    int y_offset;
    for (row = 0; row < 9; ++row) {
        for (column = 0; column < 16; ++column) {
            g_XI_BOOM_VEH_SPRITES[row * 16 + column].extract(
                &sheet, 255, 2 + column * 45, 2 + row * 49, 43, 47, 0);
        }
    }
    sheet.load_pcx("XF.PCX", 0, 0, 18);
    sheet.change_color(253, 255);
    for (row = 0; row < 4; ++row) {
        for (column = 0; column < 8; ++column) {
            g_XF_BOOM_VEH_SPRITES[row * 16 + column].extract(
                &sheet, 255, 2 + column * 102, 2 + row * 304, 100, 150, 0);
            g_XF_BOOM_VEH_SPRITES[row * 16 + column + 8].extract(
                &sheet, 255, 2 + column * 102, 154 + row * 304, 100, 150, 0);
        }
    }
    sheet.load_pcx("PALETTE.PCX", 0, 10, 236);
    // The original's eight-iteration loop performs work only for faction 0.
    InfoFactionTextColors[0] = sheet.get_pixel(580, 23);
    InfoFactionShadowColors[0] = sheet.get_pixel(580, 31);
    ChatItemIdTable[0] = sheet.get_pixel(580, 39);
    ChatShadowColors[0] = sheet.get_pixel(580, 47);
    sheet.load_pcx(VehicleUnitSheet, 0, 10, 236);

    for (chassis = 9; chassis < 19; ++chassis) {
        int source_y = 2 + (chassis - 9) * 77;
        int count = 1;
        if (source_y == 233 || source_y == 79 || source_y == 310 || source_y == 387)
            count = 7;
        else if (source_y == 464)
            count = 3;
        for (variant = 0; variant < count; ++variant) {
            int source_x = 2 + variant * 102;
            if (chassis == 18) {
                source_x = 716;
                source_y = 233;
            }
            sheet.change_color(0x107, 0x109);
            sheet.change_color(0x106, 0x109);
            g_VEH_SPRITES[chassis * 8 + variant].extract(
                &sheet, 0x109, source_x, source_y, 100, 75, 0);
            RulesChassis &record = Chassis[chassis];
            record.sprite_flag_y_off[variant] = 0;
            record.sprite_flag_x_off[variant] = 0;
            record.sprite_unk1_y_off[variant] = 0;
            record.sprite_unk1_x_off[variant] = 0;
            record.sprite_unk2_y_off[variant] = 157;
            record.sprite_unk2_x_off[variant] = 157;
            record.sprite_unk3_y_off[variant] = 157;
            record.sprite_unk3_x_off[variant] = 157;
            for (x = 0; x < 100; ++x) {
                for (marker_y = source_y - 2; marker_y <= source_y - 1; ++marker_y) {
                    // The first probe's result is ignored, but its call is real.
                    sheet.get_pixel(source_x + x, marker_y - 1);
                    if (sheet.get_pixel(source_x + x, marker_y) == 5)
                        record.sprite_flag_x_off[variant] = static_cast<unsigned char>(x);
                    if (sheet.get_pixel(source_x + x, marker_y) == 2)
                        record.sprite_unk1_x_off[variant] = static_cast<unsigned char>(x);
                }
            }
            for (y = 0; y < 75; ++y) {
                for (marker_x = source_x - 2; marker_x <= source_x - 1; ++marker_x) {
                    if (sheet.get_pixel(marker_x, source_y + y) == 5)
                        record.sprite_flag_y_off[variant] = static_cast<unsigned char>(y);
                    if (sheet.get_pixel(marker_x, source_y + y) == 2)
                        record.sprite_unk1_y_off[variant] = static_cast<unsigned char>(y);
                }
            }
        }
    }
    for (row = 0; row < 9; ++row) {
        int border_y = 2 + row * 77;
        for (x = 0; x < 100; ++x) {
            for (y = border_y - 1; y <= border_y; ++y) {
                sheet.get_pixel(918 + x, y - 1);
                if (sheet.get_pixel(918 + x, y) == 5)
                    Chassis[row].sprite_flag_x_off[0] = static_cast<unsigned char>(x);
            }
        }
        for (y_offset = 0; y_offset < 75; ++y_offset) {
            for (marker_x = 916; marker_x <= 917; ++marker_x) {
                if (sheet.get_pixel(marker_x, border_y + 1 + y_offset) == 5) {
                    // 4546CC adds the preceding final count (1) to 94A386.
                    // Preserve this original asymmetry; do not normalize to 0.
                    Chassis[row].sprite_flag_y_off[1] = static_cast<unsigned char>(y_offset);
                }
            }
        }
    }
    load_flags();
    sheet.close();
    sheet.close();
    // Automatic destructors: temporary first, then scratch[1], scratch[0].
    // On exceptions there is no compensating close of the shared Buffer.
}

// ORIGINAL: 0x00524210 ?climactic_changes@@YAXXZ 0x00524210-0x005242C0 FILE
// working copy - scaffold materialised by --work
// size      176 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0048C0A0 0x005C5A30

// Council shade/melt resolutions write these cells; the turn-phase consumer
// advances the accumulated sea-level change against the same signed counters.
// RECOVERY_OBJECT: v2 0x009A67CC 0x009A67D0 ?ClimateRiseStep@@3HA 0x00524210
int ClimateRiseStep;
// RECOVERY_OBJECT: v2 0x009A67D0 0x009A67D4 ?ClimateRiseLimit@@3HA 0x00524210
int ClimateRiseLimit;
// RECOVERY_OBJECT: v2 0x009A67D4 0x009A67D8 ?ClimateRiseAccum@@3HA 0x00524210
int ClimateRiseAccum;
// RECOVERY_OBJECT: v2 0x009A67D8 0x009A67DC ?ClimateSeaLevelDelta@@3HA 0x00524210
int ClimateSeaLevelDelta;

void __cdecl climactic_changes() {
    int sign;

    if (ClimateSeaLevelDelta != 0) {
        ClimateRiseAccum += ClimateRiseStep;
        if (ClimateRiseLimit <= ClimateRiseAccum) {
            ClimateRiseAccum -= ClimateRiseLimit;
            if (ClimateSeaLevelDelta > 0) {
                sign = 1;
            } else {
                sign = (ClimateSeaLevelDelta >= 0) - 1;
            }
            ClimateSeaLevelDelta -= sign;
            MapSeaLevel += sign;
            world_climate();
            draw_map(1);
            if (sign > 0) {
                popp(ScriptPopupFile, "SEARISING", 0, "searis_sm.pcx", 0);
            } else {
                popp(ScriptPopupFile, "SEAFALLING", 0, "seafal_sm.pcx", 0);
            }
        }
    }
}

// ORIGINAL: 0x0052A980 sub_52a980 0x0052A980-0x0052A9A5 FILE
// size      37 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052A980
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052a980/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_52a980  at 0x0052A980  (37 bytes)
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

// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const VehPrototypeFlagsBase = (int *)0x009AB898;
static int *const VehPrototypeFlagsEnd = (int *)0x009B2098;

int __cdecl sub_52a980() {
    int address = reinterpret_cast<int>(VehPrototypeFlagsBase);
    int8_t all = -1;
    do {
        *reinterpret_cast<uint32_t *>(address) &= 0xfffffffe;
        *reinterpret_cast<int8_t *>(address - 2) = all;
        address += 0x34;
    } while (address < reinterpret_cast<int>(VehPrototypeFlagsEnd));
    VehCurrentCount = 0;
    BaseCurrentCount = 0;
    return 0;
}

// ORIGINAL: 0x0052DB70 sub_52db70 0x0052DB70-0x0052DB9C FILE BYTE_EXACT
// symbol    ?sub_52db70@@YAHXZ
// size      44 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052DB70
// measured tier  MISMATCH
// divergence     14
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052db70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_52db70  at 0x0052DB70  (44 bytes)
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

// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.

int __cdecl sub_52db70() {
    // NEAR MISS, recorded so the next agent does not repeat the search.
    // MISMATCH: #14 original 'ret' vs rebuilt 'mov' (44 vs 45 bytes,
    // mnemonic similarity 0.9677, exactly one inserted instruction).
    // The first two guards match byte-for-byte. The final comparison
    // is the divergence: original is
    //     mov ecx, [0x9a6820]; mov edx, [0x939284]; xor eax, eax;
    //     cmp ecx, edx; sete al; ret
    // (operands land in ecx/edx, eax is reserved for the flag from the
    // start, no final move). Every spelling of `return a == b;` tried
    // here instead evaluates the first operand into EAX (regardless of
    // textual/declaration order), needs a second scratch register for
    // the flag, and ends with an extra `mov eax, <flag>` before `ret` -
    // proven in isolation with zero preceding code, so it is not caused
    // by the two guards above. RULED OUT: direct `return a==b`, ternary,
    // `!=` with swapped return literals, `!(a^b)`, unsigned casts, an
    // out-of-line `static` compare helper (does not inline here), bool
    // return type, a `this`-only `__thiscall` member (dead `this` in
    // ecx does not get reused for the load). A mutable `int result = 0;
    // if (a == b) result = 1; return result;` DOES reproduce the
    // ecx/edx/eax-preclear register roles exactly, but then compiles to
    // `cmp; jne; mov eax,1; ret` (branch, not `sete`) - 48 bytes. No
    // form found gets both the register roles and the `sete` tail at
    // once. Left as the closest candidate (edit_count 1) rather than
    // reverted, per the "do not throw away a measured near miss" rule.
    if ((NetTurnFlags & 0x10) == 0) {
        return 0;
    }
    if (ConsoleControlTurnActive != 0) {
        return 0;
    }
    return NetTurnFaction == ConsoleGlobal.faction_id_;
}

// ORIGINAL: 0x0052DBF0 ?next_player_turn@@YAHXZ 0x0052DBF0-0x0052DC68 FILE
// TRIED: do-while loop over i=[NetTurnFaction+1, 8) with hi/lo byte reads hoisted before the loop, matching the Ghidra hypothesis closely; diverges at instr #14 (original 'lea' vs rebuilt 'inc'), a register/addressing-mode choice around the `+1`.
// working copy - scaffold materialised by --work
// size      120 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0

// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const TurnsAssigned = (int *)0x0093E8C0;
static int *const LockAliveMask = (int *)0x009A64E9;
int __cdecl next_player_turn() {
    if ((NetTurnFlags & 0x10) == 0) {
        return 0;
    }
    if (*TurnsAssigned == 0) {
        return 0;
    }
    int i = NetTurnFaction + 1;
    if (i < 8) {
        unsigned int hi_byte = *reinterpret_cast<unsigned char *>(LockAliveMask);
        unsigned char lo_byte = *reinterpret_cast<unsigned char *>(&LockEnableMask);
        do {
            unsigned int bit = 1 << i;
            if ((bit & hi_byte) != 0 && (static_cast<unsigned char>(bit) & lo_byte) != 0) {
                message_data(0x4309, 0, i, 0, 0, 0);
                return 0;
            }
            i = i + 1;
        } while (i < 8);
    }
    NextTurnReady = 1;
    return 1;
}

// ORIGINAL: 0x005242C0 sub_5242c0 0x005242C0-0x00524317 BYTE_EXACT
// symbol    ?sub_5242c0@GraphicWinSub@@QAEXXZ
// size      87 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5930 0x005E4B4A
// 0x005242C0  sub_5242c0  ->  _sub_5242c0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


class GraphicWinSub : public GraphicWin { public:
    void sub_5242c0();
};

void GraphicWinSub::sub_5242c0() {
    // GraphicWin derives from Buffer, so the Buffer subobject at +0x444 IS the
    // base-class view of `this` - no pun, no offset.
    Buffer *buf = this;
    if (this) {
    } else {
        buf = 0;
    }
    char *self = reinterpret_cast<char *>(this);
    int flag = (*reinterpret_cast<int *>(self + 0xa18) == 0);
    char *rect = self + 0xa44;
    Sprite *spr = reinterpret_cast<Sprite *>(self + 0xa74 + flag * 0x2c);
    int p4 = *reinterpret_cast<int *>(self + 0xa48);
    int p3 = *reinterpret_cast<int *>(rect);
    int p2 = *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(spr) + 8);
    spr->draw(buf, p2, p3, p4, 1, 1);
    this->soft_update(reinterpret_cast<RECT *>(rect));
}

// ORIGINAL: 0x00524340 Demo::timer_callback 0x00524340-0x00524363 BYTE_EXACT
// symbol    _fn_00524340
// size      35 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0052435B
// 0x00524340  Demo::timer_callback  ->  _fn_00524340
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


/*
Purpose: Toggle the flag and repaint through virtual slot 62.
Original Offset: 00524340
Return Value: the slot's result
Status: Complete
*/
// The receiver arrives as an EXPLICIT stack argument here - `mov ecx,[ebp+8]`
// at entry - unlike the __thiscall sibling at 0x00524320, which never needs
// that load. Catalogued as taking none.
class Slot { public: int slot062(); };
typedef int (Slot::*Slot062Fn)();

extern "C" int __cdecl fn_00524340(void *self) {
    // ONE statement. Splitting the toggle into a temp and a separate store
    // hoists the vtable load ahead of the `sete`.
    *reinterpret_cast<int *>(reinterpret_cast<char *>(self) + 0xa18) =
        (*reinterpret_cast<int *>(reinterpret_cast<char *>(self) + 0xa18) == 0);
    return (reinterpret_cast<Slot *>(self)->*(*reinterpret_cast<Slot062Fn *>(
        reinterpret_cast<char *>(*reinterpret_cast<void **>(self)) + 0xf8)))();
}

// ORIGINAL: 0x0058F4F0 sub_58f4f0 0x0058F4F0-0x0058F609 FILE BYTE_EXACT
// LEVER: string-routines-are-calls the body was already right; the scaffold was compiling a string routine the image CALLS into an `/Oi` inline expansion. What was written down here as a tried source form - "VC6 lowers `local_104[0]=0;` as `and byte ptr [buf],0` rather than `mov`, and each branch shares one `lea` instead of recomputing it, both compiler instruction-selection choices at /O1 that plain restatement did not move" - was that expansion changing the register pressure around them. The if/else-if chain matching Ghidra's structure is the right shape and always was.
// working copy - scaffold materialised by --work
// size      281 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00625880 0x00645470

// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const SayTokOpen = (int *)0x0068FA70;
static int *const SayTokMale = (int *)0x0068FA74;
static int *const SayTokFemale = (int *)0x0068FA78;
static int *const SayTokNeutral = (int *)0x0068FA7C;
static int *const SayTokOne = (int *)0x0068FA80;
static int *const SayTokTwo = (int *)0x0068FA84;
static int *const SayTokSep = (int *)0x0068FA88;
static int *const SayTokClose = (int *)0x0068FA8C;

extern "C" char * __cdecl sub_58f4f0(char *a1, int a2, int a3, int a4) {
    char local_204[256];
    char local_104[256];

    if (a3 < 0) {
        a3 = PluralityDefault;
    }
    if (a4 < 0) {
        a4 = GenderDefault;
    }
    local_104[0] = 0;
    strcat(local_104, (char *)SayTokOpen);
    if (a4 == 0) {
        strcat(local_104, (char *)SayTokMale);
    } else if (a4 == 1) {
        strcat(local_104, (char *)SayTokFemale);
    } else if (a4 == 2) {
        strcat(local_104, (char *)SayTokNeutral);
    }
    if (a3 == 0) {
        strcat(local_104, (char *)SayTokOne);
    } else if (a3 == 1) {
        strcat(local_104, (char *)SayTokTwo);
    }
    strcat(local_104, (char *)SayTokSep);
    strcat(local_104, Players[a2].adj_name_faction);
    strcat(local_104, (char *)SayTokClose);
    parse_string(local_104, local_204);
    return strcat(a1, local_204);
}

// ORIGINAL: 0x0058F700 ?say_fac_special@@YAXPADPADH@Z 0x0058F700-0x0058F80A FILE BYTE_EXACT
// symbol    ?say_fac_special@@YAXPAD0H@Z
// size      266 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00625880 0x00645470
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0058F700
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0058f700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_fac_special@@YAXPADPADH@Z  at 0x0058F700  (266 bytes)
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

// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const SayFacTokOpen = (int *)0x0068FAB0;
static int *const SayFacTokMale = (int *)0x0068FAB4;
static int *const SayFacTokFemale = (int *)0x0068FAB8;
static int *const SayFacTokNeutral = (int *)0x0068FABC;
static int *const SayFacTokOne = (int *)0x0068FAC0;
static int *const SayFacTokTwo = (int *)0x0068FAC4;
static int *const SayFacTokSep = (int *)0x0068FAC8;
static int *const SayFacTokClose = (int *)0x0068FACC;

// Indexed table bases: the address itself is added to a computed index, so
// the const-pointer spelling (which /O2 folds into a literal and loses the
// add) is wrong here - see "the lever that keeps working" above.

void __cdecl say_fac_special(char *a1, char *a2, int a3) {
    char buf1[256];
    char buf2[256];

    int table_b = Players[a3].is_noun_plural;
    int table_a = Players[a3].noun_gender;

    buf1[0] = 0;
    strcat(buf1, reinterpret_cast<char *>(SayFacTokOpen));

    if (table_a == 0) {
        strcat(buf1, reinterpret_cast<char *>(SayFacTokMale));
    } else if (table_a == 1) {
        strcat(buf1, reinterpret_cast<char *>(SayFacTokFemale));
    } else if (table_a == 2) {
        strcat(buf1, reinterpret_cast<char *>(SayFacTokNeutral));
    }

    if (table_b == 0) {
        strcat(buf1, reinterpret_cast<char *>(SayFacTokOne));
    } else if (table_b == 1) {
        strcat(buf1, reinterpret_cast<char *>(SayFacTokTwo));
    }

    strcat(buf1, reinterpret_cast<char *>(SayFacTokSep));
    strcat(buf1, a2);
    strcat(buf1, reinterpret_cast<char *>(SayFacTokClose));

    parse_string(buf1, buf2);
    strcat(a1, buf2);
}

// ORIGINAL: 0x0052DB40 sub_52db40 0x0052DB40-0x0052DB67 BYTE_EXACT
// symbol    ?sub_52db40@@YAXXZ
// size      39 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0052DB40  sub_52db40  ->  _sub_52db40
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


static int *const AlphaNetProcessSlotPattern = (int *)0x0093E99E;

void sub_52db40() {
    char *record = reinterpret_cast<char *>(AlphaNetProcessSlotPattern) - 2;
    for (int i = 8; i != 0; --i) {
        record[0] = -1;
        record[1] = 0;
        record[2] = 0;
        record[3] = -1;
        record[4] = 2;
        record += 0x19c;
    }
}

// ORIGINAL: 0x00524370 sub_524370 0x00524370-0x00524396 BYTE_EXACT
// symbol    ?on_key@Widget@@QAEHHH@Z
// size      38 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00524387
// 0x00524370  sub_524370  ->  _sub_524370
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


/*
Purpose: Activate on Enter, Escape or Space, through virtual slot 58.
Original Offset: 00524370
Return Value: one
Status: Complete
*/
// A __thiscall member, not the catalogued `extern "C" int __stdcall`: ecx is
// used directly with no stack load, and `ret 8` pops the two explicit
// arguments a thiscall callee cleans up itself. The first is dead.
class Slot058 { public: int slot058(); };
typedef int (Slot058::*Slot058Fn)();

class Widget {
 public:
    int on_key(int a1, int a2);
};

int Widget::on_key(int, int a2) {
    // 0xd, 0x1b, 0x20 are VK_RETURN, VK_ESCAPE and VK_SPACE.
    if (a2 == 0xd || a2 == 0x1b || a2 == 0x20) {
        (reinterpret_cast<Slot058 *>(this)->*(*reinterpret_cast<Slot058Fn *>(
            reinterpret_cast<char *>(*reinterpret_cast<void **>(this))
            + 0xe8)))();
    }
    return 1;
}

// ORIGINAL: 0x00524320 sub_524320 0x00524320-0x0052433B BYTE_EXACT
// symbol    ?toggle_and_repaint@Base524320@@QAEHXZ
// size      27 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready
// calls     (none)
// 0x00524320  sub_524320  ->  _sub_524320
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


/*
Purpose: Toggle the flag and repaint through virtual slot 62.
Original Offset: 00524320
Return Value: the slot's result
Status: Complete
*/
// Catalogued `extern "C" int __cdecl sub_524320()`, and the disassembly
// disagrees: no stack frame at all, ecx read directly as `this`, and the body
// ends in `jmp [edx+0xf8]` rather than `ret`.
//
// The dispatch goes through a POINTER-TO-MEMBER-FUNCTION rather than a plain
// function pointer: VC6 rejects an explicit `__thiscall` function-pointer
// typedef with C4234, while a pointer-to-member is implicitly thiscall and
// reproduces the tail jump exactly.
class Slot524320 {
 public:
    int slot062();
};
typedef int (Slot524320::*Slot062Fn524320)();

// `Base524320` is already in hypothesis_layouts.h (0xA1C bytes, field_a18_ at
// 0xA18), which game.cpp includes - so the body reaches the flag through that
// layout instead of declaring a second one.
int Base524320::toggle_and_repaint() {
    field_a18_ = (field_a18_ == 0);
    return (reinterpret_cast<Slot524320 *>(this)
            ->*(*reinterpret_cast<Slot062Fn524320 *>(
                reinterpret_cast<char *>(*reinterpret_cast<void **>(this))
                + 0xf8)))();
}

// ===== homed out of the archive 2026-09-04 =====

// ORIGINAL: 0x0058FA20 ?get_pacts@@YAPADH@Z 0x0058FA20-0x0058FA69
// LEVER: the carried bindings renamed to the tree's own models - Players
//         (faction.h, 0x00946A50), Labels (alpha.h, 0x009B90F8), StringTable
//         (strings.h, 0x009B90D8) and general.h's noun defaults - so no
//         anonymous g_ binding arrived with the body.
// TRIED: as `Players[a1].is_leader_female` for the load; the image reads it
//         as `[players + a1*0x59c]`, so the member form is the same operand.
// MISMATCH #10 'mov' vs 'mov': the image keeps `add edx, 0xcb` as its own
//         instruction before `mov ecx, [eax+edx*4]`, where every spelling
//         tried folds the 0xcb into the addressing displacement
//         (`[ecx+edx*4+0x32c]`) and hoists the `Labels.strings_ptr` load
//         above the store. Tried (as 0x0058F9D0, four spellings): inline
//         expression, a named boolean, a named `off` combining the constant
//         with the flag, and a `char *` base with a manual `*4` multiply -
//         all four byte-identical at this divergence.
char *__cdecl get_pacts(int faction_id) {
    PluralityDefault = 1;
    int noun = Players[faction_id].is_leader_female;
    GenderDefault = noun;
    LPSTR *labels = (LPSTR *)Labels.strings_ptr;
    return StringTable.get((int)labels[0xcb + (noun != 0)]);
}

// ORIGINAL: 0x0058F9D0 ?get_pact@@YAPADH@Z 0x0058F9D0-0x0058FA19
// MISMATCH #3 'xor' vs 'mov': the original zeroes edx BEFORE storing
// PluralityDefault = 0, and every source order tried compiles that store
// first. A second divergence follows: the original keeps `add edx, 0xc9` as
// its own instruction before `mov ecx, [eax+edx*4]` where every spelling
// tried folds the 0xc9 into the addressing displacement. Tried: inline
// expression, a named boolean `flag`, a named `off` combining the constant
// with the flag, and a `char *` base with a manual `*4` multiply - all four
// byte-identical at the divergence, so it reads as an optimiser
// canonicalisation this call shape always hits.
// TRIED: homed out of the archive once its five fixed-address bindings were
//         all nameable from models the tree already holds (see get_pacts).
char *__cdecl get_pact(int faction_id) {
    PluralityDefault = 0;
    int noun = Players[faction_id].is_leader_female;
    GenderDefault = noun;
    LPSTR *labels = (LPSTR *)Labels.strings_ptr;
    return StringTable.get((int)labels[0xc9 + (noun != 0)]);
}

// ORIGINAL: 0x0052DBA0 ?current_server@@YAHXZ 0x0052DBA0-0x0052DBE6
// MISMATCH #6 'push' vs 'cmp': the original spends TWO push/pop pairs (esi,
// edi) and a xor/sete/test dance to compare NetTurnFaction against
// LocalFaction before restoring registers and testing the boolean; every
// source spelling tried here (direct `==`, named locals for one or both
// operands, top-level vs nested loads, goto-style vs nested-if for the
// shared "not active" tail) instead gets a single push/pop esi plus a direct
// cmp+jne under all four measured flag sets - a strictly shorter encoding
// the register allocator prefers once it is allowed to. Ruled out: not guard
// duplication or tail sharing (those match through the merged trailing
// return); it reads as five live values forcing two callee-saved spills.
int __cdecl current_server() {
    int in_transit = ConsoleControlTurnActive;
    if ((NetTurnFlags & 0x10) != 0 && in_transit == 0) {
        if (NetTurnFaction == ConsoleGlobal.faction_id_) {
            return 1;
        }
        return 0;
    }
    return *TurnsAssigned;
}

// ORIGINAL: 0x0058F610 ?say_special@@YAXPADPADH@Z 0x0058F610-0x0058F6F6 BYTE_EXACT
// LEVER: the eight fixed-address bindings are .rdata STRING LITERALS, read
//         out of the image with image_data.py and spelled as the text, the
//         same way netdaemon.cpp's captions went: 0x0068FA90 "$<",
//         0x0068FA94 "M", 0x0068FA98 "F", 0x0068FA9C "N", 0x0068FAA0 "1",
//         0x0068FAA8 ":" and 0x0068FAAC ">". 0x0068FAA4 "2" is not read by
//         this body and is left for whoever homes its caller. The eight
//         together are the markup the parser consumes: a faction noun of
//         M/F/N, a noun form digit, then the substituted text.
// LEVER: the ninth binding is `Players` (faction.h, 0x00946A50); the image
//         indexes it `a3 * 0x167` dwords, which is `Players[a3]` - the
//         stride IS sizeof(Player).
// symbol   ?say_special@@YAXPAD0H@Z
//         - this tree's name for the same (char *, char *, int) signature.
//         VC6 folds the two identical `char *` parameters into a
//         back-reference, where the catalogued name spells the second `PAD`
//         longhand; `measure` matches by address, and this fact is what lets
//         it resolve the symbol.
void __cdecl say_special(char *output, char *text, int faction_id) {
    char buffer[256];
    char parsed[256];

    buffer[0] = 0;
    int noun = Players[faction_id].is_leader_female;
    strcat(buffer, "$<");

    if (noun == 0) {
        strcat(buffer, "M");
    } else if (noun == 1) {
        strcat(buffer, "F");
    } else if (noun == 2) {
        strcat(buffer, "N");
    }

    strcat(buffer, "1");
    strcat(buffer, ":");
    strcat(buffer, text);
    strcat(buffer, ">");

    parse_string(buffer, parsed);
    strcat(output, parsed);
}

// ORIGINAL: 0x0058F810 ?get_phrase@@YAPADHH@Z 0x0058F810-0x0058F8A2
// LEVER: the faction-table bases at 0x00946A54 and 0x00946A6C are
//         `Players[a1].filename` and `Players[a1].search_key` - the image
//         computes `a1 * 0x59c` and adds those two constants, and 0x00946A50
//         (Players) plus 4 and plus 0x1c are exactly those two fields of
//         faction.h's Player.
// LEVER: 0x0068FAD0 is the .rdata literal "PLAYERFILE" (image_data.py), the
//         popup caption for the section that failed to open.
// LEVER: 0x009B7D00 is text.h's `TextBufferGetPtr`, already modelled; the
//         image returns the POINTER stored there (`mov eax, [0x9b7d00]`), so
//         the binding is a real global, not a buffer.
char *__cdecl get_phrase(int faction_id, int phrase_id) {
    char *search_key = Players[faction_id].search_key;
    char *filename = Players[faction_id].filename;
    int ok = text_open(filename, search_key);
    if (ok != 0) {
        ok = text_open(filename, filename);
        if (ok != 0) {
            parse_says(0, search_key, -1, -1);
            parse_says(1, filename, -1, -1);
            X_pop("PLAYERFILE", 0);
            return StringTable.get(0);
        }
    }
    int n = phrase_id + 9;
    if (n > 0) {
        do {
            text_get();
            n--;
        } while (n != 0);
    }
    // TRIED: the counter as one local driving both the guard and the loop
    //         (`int n = phrase_id + 9; if (n > 0) { do { text_get(); n--; }`)
    //         scores 46 of 58 - VC6 folds the add into `lea esi,[eax+9]` and
    //         tests esi, where the image tests eax and then copies it to esi
    //         for the loop. Modifying the parameter instead (`phrase_id =
    //         phrase_id + 9;` with a separate `int i`) moves the whole test
    //         to esi too: 45 of 58. The image's shape needs the guard value
    //         and the counter in DIFFERENT registers, which VC6 merges under
    //         both spellings.
    return TextBufferGetPtr;
}

// ORIGINAL: 0x0058F8B0 ?get_noun_phrase@@YAPADHH@Z 0x0058F8B0-0x0058F9C9
// LEVER: same three bindings as get_phrase - Players' filename/search_key at
//         0x00946A54/0x00946A6C, the "PLAYERFILE" caption at 0x0068FADC, and
//         text.h's TextBufferGetPtr at 0x009B7D00.
// BUG IN THE ARCHIVED TRANSCRIPTION, not in the shipped image: the artifact
//         returned the ADDRESS of the TextBufferGetPtr variable rather than
//         the pointer it holds. The image loads `[0x9b7d00]` into eax twice,
//         once as strcpy's destination and once as the return value, so it
//         returns the buffer. Fixed here; the two loads are kept separate on
//         purpose - caching one local would delete the second
//         `mov eax, [0x9b7d00]`.
char *__cdecl get_noun_phrase(int faction_id, int phrase_id) {
    char buffer[256];
    char *search_key = Players[faction_id].search_key;
    char *filename = Players[faction_id].filename;

    if ((text_open(filename, search_key) != 0)
        && (text_open(filename, filename) != 0)) {
        parse_says(0, search_key, -1, -1);
        parse_says(1, filename, -1, -1);
        X_pop("PLAYERFILE", 0);
        return StringTable.get(0);
    }

    phrase_id = phrase_id + 9;
    if (phrase_id > 0) {
        int i = phrase_id;
        do {
            text_get();
            --i;
        } while (i != 0);
    }
    strcpy(buffer, text_item());
    char *item = text_item();
    char c = item[0];
    int gender = 0;
    int number = 0;
    if (c == 'M' || c == 'm') {
        gender = 0;
    }
    if (c == 'F' || c == 'f') {
        gender = 1;
    }
    if (c == 'N' || c == 'n') {
        gender = 2;
    }
    if (item[1] == '1') {
        number = 0;
    }
    if (item[1] == '2') {
        number = 1;
    }
    GenderDefault = gender;
    PluralityDefault = number;
    strcpy(TextBufferGetPtr, buffer);
    return TextBufferGetPtr;
}

// ORIGINAL: 0x00450FB0 sub_450fb0 0x00450FB0-0x00451021 FILE
// symbol    _extract_sheet_icon
// LEVER: the fixed-address binding renamed to buffer.h's
//         g_PCX_PARSE_TEMP_BUFFER1 (0x00798668, defined in buffer.cpp) - the
//         tree already owns that name, so no anonymous g_ binding arrived.
// TRIED: signature had 0 params in the scaffold but the disassembly reads 5
//         stack args - changed to the five below. `-static_cast<int>((x&2)!=0)
//         &0x1d` for the neg/sbb/and offset mask compiles close but diverges at
//         instr #17 ('and' vs rebuilt 'shr').
// TRIED: the image gives this body no direct caller (0 in the image's call
//         graph), so the parameter names below are read off the body's own
//         uses - `sprite` is the extract receiver, `sheet_x`/`sheet_y` land in
//         extract's second and third slots, `quadrant` feeds only the two
//         &2 masks, `recolor` guards the two change_color calls. Nothing in
//         the image ties them to a caller-side meaning.

extern "C" int __cdecl extract_sheet_icon(int sprite, int sheet_x, int sheet_y,
                                         int quadrant, int recolor) {
    if (recolor != 0) {
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
    }
    int off_x = (-static_cast<int>(((quadrant + 1) & 2) != 0) & 0x1d) + sheet_x;
    int off_y = (-static_cast<int>((quadrant & 2) != 0) & 0x1d) + sheet_y;
    return reinterpret_cast<Sprite *>(sprite)->extract(
        &g_PCX_PARSE_TEMP_BUFFER1, 0x109, off_x, off_y, 0x1c, 0x1c, 0);
}

// RECOVERY_LITERAL: v1 0x0068511C 0x00685127 ??_C@_0L@GPCH@CURSOR?4PCX?$AA@ 0x00454FC0
// RECOVERY_LITERAL: v1 0x00685128 0x00685135 ??_C@_0N@NBMC@NEWICONS?4PCX?$AA@ 0x00454FC0
// RECOVERY_LITERAL: v1 0x00685138 0x00685145 ??_C@_0N@NHKJ@ALIENCIT?4PCX?$AA@ 0x00454FC0
// ORIGINAL: 0x00454FC0 ?load_cursors@@YAXXZ 0x00454FC0-0x004553F3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00454FC0-0x004553F3 complete body; 0x00454FC6 reads DirectDraw state and skips on nonnull.", "image:0x00455006-0x00455049 scans 32 samples in each axis, including vertical row32; 0x00455099-0x0045509F publishes the last marker only after extraction.", "image:0x0045512E and 0x004551FB advance human receiver by four Sprite objects for the second variant. Original initializer/destructor loops at 0x0044F420-0x0044F693 independently establish citizen array extents.", "image:0x0068511C,0x00685128,0x00685138 contain independently inspected bounded filename strings CURSOR.PCX, NEWICONS.PCX and ALIENCIT.PCX."], "reason": "Populate genuine cursor and citizen Sprite arrays from the three asset sheets, preserving hotspot scanning, extraction order and original mode/failure behavior."}
void __cdecl load_cursors() {
    if (BufferDirectDraw) {
        return;
    }

    g_PCX_PARSE_TEMP_BUFFER1.load_pcx("CURSOR.PCX", 0, 10, 236);
    int x = 1;
    for (int cursor = 0; cursor < 12; ++cursor) {
        int hot_y = 0;
        int hot_x = 0;
        for (int column = 0; column < 32; ++column) {
            if (g_PCX_PARSE_TEMP_BUFFER1.get_pixel(x + column, 0) == 5) {
                hot_x = column;
            }
        }
        for (int row = 0; row < 32; ++row) {
            if (g_PCX_PARSE_TEMP_BUFFER1.get_pixel(x - 1, row + 1) == 5) {
                hot_y = row;
            }
        }
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
        g_CURSOR_SPRITES[cursor].extract(
            &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 1, 32, 32, 0);
        CursorHotspotX[cursor] = hot_x;
        CursorHotspotY[cursor] = hot_y;
        x += 33;
    }

    g_PCX_PARSE_TEMP_BUFFER1.load_pcx("NEWICONS.PCX", 0, 10, 236);
    x = 1;
    int mood;
    int specialist;
    for (mood = 0; mood < 4; ++mood) {
        for (int variant = 0; variant < 2; ++variant) {
            g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
            g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
            g_CITIZEN_LG_CURSOR_SPRITES[mood + 4 * variant].extract(
                &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 501, 38, 48, 0);
            x += 39;
        }
    }
    for (specialist = 0; specialist < 7; ++specialist) {
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
        g_SPECIALIST_LG_CURSOR_SPRITES[specialist].extract(
            &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 501, 38, 48, 0);
        x += 39;
    }

    x = 44;
    for (mood = 0; mood < 4; ++mood) {
        for (int variant = 0; variant < 2; ++variant) {
            g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
            g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
            g_CITIZEN_SM_CURSOR_SPRITES[mood + 4 * variant].extract(
                &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 458, 30, 38, 0);
            x += 31;
        }
    }
    for (specialist = 0; specialist < 7; ++specialist) {
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
        g_SPECIALIST_SM_CURSOR_SPRITES[specialist].extract(
            &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 458, 30, 38, 0);
        x += 31;
    }

    g_PCX_PARSE_TEMP_BUFFER1.load_pcx("ALIENCIT.PCX", 0, 10, 236);
    x = 1;
    for (mood = 0; mood < 4; ++mood) {
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
        g_AL_CITIZEN_SM_CURSOR_SPRITES[mood].extract(
            &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 1, 30, 38, 0);
        x += 31;
    }
    for (specialist = 0; specialist < 7; ++specialist) {
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
        g_AL_SPECIALIST_SM_CURSOR_SPRITES[specialist].extract(
            &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 1, 30, 38, 0);
        x += 31;
    }

    x = 1;
    for (mood = 0; mood < 4; ++mood) {
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
        g_AL_CITIZEN_LG_CURSOR_SPRITES[mood].extract(
            &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 41, 38, 48, 0);
        x += 39;
    }
    for (specialist = 0; specialist < 7; ++specialist) {
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x107, 0x109);
        g_PCX_PARSE_TEMP_BUFFER1.change_color(0x106, 0x109);
        g_AL_SPECIALIST_LG_CURSOR_SPRITES[specialist].extract(
            &g_PCX_PARSE_TEMP_BUFFER1, 0x109, x, 41, 38, 48, 0);
        x += 39;
    }
    g_PCX_PARSE_TEMP_BUFFER1.close();
}

// The original walks eight factions, two terrain variants, four size tiers
// and three defense tiers, storing pointers to successive Sprite members.
// The real pointer table and its pointees share faction.cpp ownership.
// The preserved LEVER/TRIED notes below describe the superseded raw-storage
// spelling, not a requirement to keep an executable-address table.

// ORIGINAL: 0x00453A20 ?load_bases@@YAXXZ 0x00453A20-0x00453A6A FILE BYTE_EXACT
// LEVER: the two fixed-address bindings renamed off their addresses.
//         0x0076E890 is caviar.cpp's g_VCTB_CAVIARDATA (now declared in
//         caviar.h, so the same address does not carry two names), which makes
//         the table it terminates the 0x300 bytes immediately in front of it.
//         0x0078E978 is what the table points into - general.cpp's
//         FactionArtGlobal_storage, 8 FactionArt elements of 0x65c - and the
//         strides say so: 0x2c per Sprite (24 of them = FactionArt::base[6][4],
//         the 0x420 row bound), 0x84 per three sprites, 0x65c per faction.
//         src/recovered/units/00455eb0.cpp (load_sprites) walks the same table
//         with the same three strides, which corroborates both names
//         independently of this body.
// TRIED: the storage base spelled as a one-byte extern symbol, as a const
//         int, as a const pointer, and as the literal - all four measure the
//         same 13/26 here. The 25/26 the carried note records was scored
//         against 00453a20.cpp's OWN translation unit, whose scaffold
//         declarations differ; in this tree the wall is register ASSIGNMENT,
//         not the spelling of the binding: the image zeroes the outer
//         accumulator into EBX and keeps the table in EAX, this tree puts the
//         table in ECX and the inner 4-counter in EBX, so the accumulator's
//         `xor ebx, ebx` never appears at all.

void __cdecl load_bases() {
    for (int faction = 0; faction < MaxPlayerNum; ++faction) {
        for (int terrain = 0; terrain < 2; ++terrain) {
            for (int size = 0; size < 4; ++size) {
                for (int defense = 0; defense < 3; ++defense) {
                    FactionBaseSprites[faction][terrain][size][defense] =
                        &FactionsArt[faction].base[terrain][size][defense];
                }
            }
        }
    }
}

// ORIGINAL: 0x0052A9B0 sub_52a9b0 0x0052A9B0-0x0052AA2D FILE BYTE_EXACT
// symbol    _directplay_install_is_complete
// LEVER: the six fixed-address bindings renamed or deleted. Three of them -
//         0x00669000, 0x0066900C and 0x00669010 - are ADVAPI32 import slots
//         (tools/iat_names.py: RegQueryValueExA, RegOpenKeyExA, RegCloseKey),
//         so the dllimport declarations replace them outright. The other three
//         are .rdata literals, read with tools/image_data.py:
//         0x0068BE94 is the DirectPlay Applications key for this game,
//         0x0068BEDC is "Complete" and 0x0068BEE8 is "Yes". String literals
//         fold to their own addresses, so the pushes come out the same.
// LEVER: BYTE_EXACT on declaring `int result = 0` at the TOP of the body.
//         The image zeroes esi before RegOpenKeyExA and then pushes esi twice
//         as the call's two `0` arguments - which it can only do because the
//         result local is already live. Declaring `result` after the call
//         cost VC6 the shared zero, and every `0` argument then materialised
//         as its own `push 0` (24/49, prologue `lea` where the image has
//         `push esi`).

extern "C" int __cdecl directplay_install_is_complete() {
    typedef int (__cdecl *StrcmpiFn)(const char *, const char *);
    int result = 0;
    HKEY key;
    LONG status = RegOpenKeyExA(reinterpret_cast<HKEY>(0x80000002),
        "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\"
        "Sid Meier's Alpha Centauri", 0, 0, &key);
    if (status != 0) {
        return 0;
    }
    DWORD valueType;
    char valueBuf[12];
    DWORD valueSize = 10;
    status = RegQueryValueExA(key, "Complete", 0, &valueType,
        reinterpret_cast<BYTE *>(valueBuf), &valueSize);
    if (status == 0) {
        StrcmpiFn fn = reinterpret_cast<StrcmpiFn>(_strcmpi);
        if (fn(valueBuf, "Yes") == 0) {
            result = 1;
        }
    }
    RegCloseKey(key);
    return result;
}

// ORIGINAL: 0x0058FA70 ?get_pacts@@YAPADHH@Z 0x0058FA70-0x0058FAFA
// working copy - scaffold materialised by --work
// size      138 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0

// LEVER: the three fixed-address bindings renamed to the tree's own models -
//         Players (faction.h), Labels (alpha.h) and StringTable (strings.h), so
//         no anonymous g_ binding arrived with the body. 0x009BBFEC is
//         GenderDefault and 0x009BBFF0 is PluralityDefault (general.h).
// LEVER: `int flag; if (a && b) flag = 1; else flag = 0;` - the two else arms
//         SHARE one `xor edx,edx` and the true arm jumps over it
//         (`mov edx,1 / jmp / xor edx,edx`). The ternary and the bare `&&`
//         both compile the other way (`jne` straight to the store).
// TRIED: `code` as a ternary, as a nested ternary and with `labels` declared
//         ahead of it - all three move `mine` off ECX (22/43, 4/43, 12/43)
//         against 27/43 for the plain `int code = 0xcb; if (...) code = 0xcc;`.
//         What is left is register ASSIGNMENT: the image keeps `code` in EAX
//         (reusing the dead `Players[other_id]` load) and `labels` in EDX, and
//         hoists the third `test ecx,ecx` above the two global stores.

char *__cdecl get_pacts(int faction_id, int other_id) {
    int mine = Players[faction_id].is_leader_female;
    int theirs = 0;
    if (mine != 0) {
        theirs = Players[other_id].is_leader_female;
    }
    int flag;
    if (mine != 0 && theirs != 0) {
        flag = 1;
    } else {
        flag = 0;
    }
    GenderDefault = flag;
    PluralityDefault = 1;
    int code = 0xcb;
    if (mine != 0) {
        if (Players[other_id].is_leader_female != 0) {
            code = 0xcc;
        }
    }
    LPSTR *labels = (LPSTR *)Labels.strings_ptr;
    return StringTable.get((int)labels[code]);
}

// ORIGINAL: 0x005B0420 ?balance@@YAXXZ 0x005B0420-0x005B0A2C
// RECOVERY_BINDING: v1 {"evidence": ["Retained game-setup-owner-wave full body and seed005B0420; independent pinned-span review by report_draw_labs_review; actual-source test_setup_balance_behavior controls. Native faction rule bit0x100 is aquatic; no historical match claim copied."], "reason": "Recover the complete starting-position balance operation with genuine vehicle, faction, continent and map owners, retaining native neighbor scoring, defender gating and the mutable resource-placement bound."}
void __cdecl balance() {
    int vehicles_by_direction[7];
    for (int direction = 1; direction <= 7; ++direction) {
        const int index = direction - 1;
        int vehicle_id = veh_find(0, 0, direction, -1);
        vehicles_by_direction[index] = vehicle_id;
        if (vehicle_id >= 0) {
            const int x = Vehs[vehicle_id].x;
            const int y = Vehs[vehicle_id].y;
            const Map *tile = map_loc(x, y);
            if (tile->region >= 0x40) {
                vehicles_by_direction[index] = veh_find(x, y, direction, vehicle_id);
            }
        }
    }

    int direction_scores[7];
    int best_score = 0;
    for (int direction_index = 0; direction_index < 7; ++direction_index) {
        const int direction = direction_index + 1;
        const int vehicle_id = vehicles_by_direction[direction_index];
        if (vehicle_id < 0) continue;

        const int x = Vehs[vehicle_id].x;
        const int y = Vehs[vehicle_id].y;
        const int region = map_loc(x, y)->region;
        int nearest_score = 9999;
        for (int other_direction = 1; other_direction <= 7; ++other_direction) {
            if (other_direction == direction) continue;
            const int other_id = vehicles_by_direction[other_direction - 1];
            if (other_id < 0) continue;
            const int other_x = Vehs[other_id].x;
            const int other_y = Vehs[other_id].y;
            if (region != map_loc(other_x, other_y)->region) continue;

            int dx = abs(x - other_x);
            if (!(MapIsFlat & 1) && dx > static_cast<int>(MapLongitude)) {
                dx = MapLongitudeBounds - dx;
            }
            const int dy = abs(y - other_y);
            dx = abs(dx);
            const int high = dx > dy ? dx : dy;
            const int low = dx > dy ? dy : dx;
            const int diagonal = (((dx + dy) >> 1) - low + 1) >> 1;
            const int score = ((high - diagonal) * 25 * 128) /
                              static_cast<int>(MapArea);
            if (score < nearest_score) nearest_score = score;
        }

        if (nearest_score < 1000) {
            nearest_score = nearest_score / 5 - 3;
        } else {
            int terrain_score = (static_cast<int>(Continents[region].open_terrain) + 31) / 32;
            if (terrain_score < 1) terrain_score = 1;
            else if (terrain_score > 8) terrain_score = 8;
            nearest_score = terrain_score;
        }

        int adjacent_bonus = 0;
        for (int offset = 0; offset < 0x54; offset += 4) {
            int neighbor_x = x + RadiusOffsetX[offset / 4];
            const int width = MapLongitudeBounds;
            if (!(MapIsFlat & 1)) {
                if (neighbor_x < 0) neighbor_x += width;
                else if (neighbor_x >= width) neighbor_x -= width;
            }
            const int neighbor_y = y + RadiusOffsetY[offset / 4];
            if (neighbor_y < 0 || neighbor_y >= MapLatitudeBounds ||
                neighbor_x < 0 || neighbor_x >= width) continue;

            const Map *neighbor = map_loc(neighbor_x, neighbor_y);
            const int rainfall_code = (neighbor->climate >> 3) & 3;
            int altitude = neighbor->climate >> 5;
            const int owner = neighbor->val3 >> 6;
            if (rainfall_code < 1) continue;
            altitude -= 2;
            const bool elevated_or_river = altitude > 0 || (neighbor->bit & BIT_RIVER) != 0;
            if (elevated_or_river && owner < 2) {
                if (owner == 1) {
                    ++adjacent_bonus;
                } else if ((altitude >= 0 && altitude > 1) || rainfall_code > 1) {
                    ++adjacent_bonus;
                }
            }
            if (owner == 1 && altitude >= 0 && altitude > owner) {
                ++adjacent_bonus;
            }
        }

        int terrain_component = (static_cast<int>(Continents[region].open_terrain) + 63) / 64;
        if (terrain_component < 0) terrain_component = 0;
        else if (terrain_component > 3) terrain_component = 3;
        const int total = nearest_score + adjacent_bonus + terrain_component;
        direction_scores[direction_index] = total;
        if (total > best_score) best_score = total;
    }

    for (int faction_index = 0; faction_index < 7; ++faction_index) {
        const int faction_id = faction_index + 1;
        const int vehicle_id = vehicles_by_direction[faction_index];
        if (vehicle_id < 0) continue;

        const int x = Vehs[vehicle_id].x;
        const int y = Vehs[vehicle_id].y;
        const int seed = rand() % 16;
        int deficit = best_score - direction_scores[faction_index];
        int placed_count = 0;
        const Player &player = Players[faction_id];
        for (int bonus_index = 0; bonus_index < player.faction_bonus_count; ++bonus_index) {
            if (player.faction_bonus_id[bonus_index] == 1) {
                const int prototype_id = player.faction_bonus_val1[bonus_index];
                deficit -= VehPrototypes[prototype_id].plan == 8 ? 3 : 1;
            }
        }

        int difficulty = DiffLevelCurrent;
        if (difficulty > 1) {
            bool defend = deficit >= 3;
            if (!defend) {
                const int human_bit = (1 << faction_id) & (LockEnableMask & 0xFFu);
                defend = difficulty >= (human_bit ? 4 : 5);
            }
            if (defend) {
                deficit -= 3;
                if ((player.rule_flags & RFLAG_AQUATIC) == 0) {
                    const int defender_id = veh_init(0, faction_id, x, y);
                    difficulty = DiffLevelCurrent;
                    Vehs[defender_id].next_veh_id_stack = -1;
                }
            }
        }
        if (difficulty < 2) deficit = (deficit + 1) / 2;

        if (deficit > 0) {
            int angle = seed * 7;
            for (int place_index = 0; place_index < deficit; ++place_index) {
                angle += 7;
                const int offset_index = angle % 16;
                const int width = MapLongitudeBounds;
                int candidate_x = x + RadiusOffsetX[9 + offset_index];
                if (!(MapIsFlat & 1)) {
                    if (candidate_x < 0) candidate_x += width;
                    else if (candidate_x >= width) candidate_x -= width;
                }
                const int candidate_y = y + RadiusOffsetY[9 + offset_index];
                if (candidate_y < 0 || candidate_y >= MapLatitudeBounds ||
                    candidate_x < 0 || candidate_x >= width) continue;

                if (goody_at(candidate_x, candidate_y) != 0) {
                    const Map *candidate = map_loc(candidate_x, candidate_y);
                    if ((candidate->bit & 0x0C000000u) == 0) {
                        --direction_scores[faction_index];
                    }
                }

                if (placed_count < 2 && deficit - place_index > 2 &&
                    ((place_index ^ placed_count) & 1) == 0) {
                    bit_set(candidate_x, candidate_y, 0x04000000, 1);
                    --deficit;
                    ++placed_count;
                } else {
                    bit_set(candidate_x, candidate_y, 0x08000000, 1);
                }
            }
        }
    }
}

// ORIGINAL: 0x005247B0 ?generators@@YAHHPAH@Z 0x005247B0-0x00524862
// The generator threshold at 0x00949864 is Rules.size_base_subspace_gen,
// one of RulesBasic's last three fields, not a separate global in a gap.
// The original walk reads Players[].rule_flags and Bases[] ownership,
// population and facility bits. Compare population as signed, as in the image.
int __cdecl generators(int faction_id, int *count_out) {
    int count = 0;
    if (count_out != 0) {
        *count_out = 0;
    }
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
        return 0;
    }
    int total = BaseCurrentCount;
    if (total > 0) {
        int index = 0;
        do {
            if (Bases[index].faction_id_current == faction_id
                && has_fac_built_call(0x28, index)) {
                if (Bases[index].population_size
                    < static_cast<int>(Rules.size_base_subspace_gen)) {
                    if (count_out != 0) {
                        ++*count_out;
                    }
                } else {
                    ++count;
                }
            }
            ++index;
        } while (index < total);
    }
    return count;
}

// ORIGINAL: 0x0058FBC0 ?get_his_her@@YAPADHH@Z 0x0058FBC0-0x0058FC4B
// LEVER: the fixed-address bindings renamed to the tree's own models - Players
//         (faction.h), Labels (alpha.h), StringTable (strings.h), the CharUpperA
//         import slot (windows.h) and 0x00945838 as PronounBuffer below. The
//         use-site census names that buffer: get_his_her (this body),
//         get_him_her (0x0058FC50) and get_he_she (0x0058FCE0) are the only
//         three sites in the image that touch it, and each strcpy's its fetched
//         pronoun into it and hands back its address.
// TRIED: `LPSTR *labels` read inline in the call instead of as a named local -
//         20/42 against 21/42, so the local stays. Naming the flag `female`
//         first, swapping the `+` operands, and the `(x != 0) + 0xcf` form all
//         score the same 21/42 as what is committed. What is left is register
//         ASSIGNMENT: the image zeroes the `0xcf +` accumulator into EDX before
//         the index chain and keeps `Labels.strings_ptr` in EAX; this tree puts
//         the accumulator in EAX and the label table in ECX.

char *__cdecl get_his_her(int faction_id, int sentence_form) {
    int code = 0xcf + (Players[faction_id].is_leader_female != 0);
    LPSTR *labels = (LPSTR *)Labels.strings_ptr;
    strcpy(PronounBuffer, StringTable.get((int)labels[code]));
    if (sentence_form != 0) {
        if (sentence_form == 1) {
            char buf[2];
            buf[0] = PronounBuffer[0];
            buf[1] = 0;
            CharUpperA(buf);
            PronounBuffer[0] = buf[0];
            return PronounBuffer;
        }
        CharUpperA(PronounBuffer);
    }
    return PronounBuffer;
}

/*
Purpose: Fetch the "him/her" pronoun into the shared game pronoun buffer,
         optionally upper-casing it, beside the sibling get_his_her body.
// ORIGINAL: 0x0058FC50 ?get_him_her@@YAPADHH@Z 0x0058FC50-0x0058FCDB
// RECOVERY_BINDING: v1 {"evidence": ["Original58FC50..58FCDB; retired seed git:b18d3ac8:src/unrecovered/0058fc50.cpp records the same identity and spans.", "The fixed-address bindings rename to the tree's own models exactly as game.cpp's sibling get_his_her (0x0058FBC0) already does: Players (faction.h, the 0x167-stride first word is is_leader_female), Labels (alpha.h), StringTable (strings.h), PronounBuffer (game.h, 0x00945838) and the CharUpperA import the header supplies.", "Label code 0xC7/0xC8 and the sentence-form handling are the seed's measured behaviour."], "reason": "Home the sibling pronoun fetcher beside its shared buffer and get_his_her in the game source."}
Return Value: PronounBuffer
Status: Coverage transcription - unclaimed body
*/
char *__cdecl get_him_her(int faction_id, int sentence_form) {
    const int code = 0xC7 + (Players[faction_id].is_leader_female != 0);
    LPSTR *labels = (LPSTR *)Labels.strings_ptr;
    strcpy(PronounBuffer, StringTable.get((int)labels[code]));
    if (sentence_form != 0) {
        if (sentence_form == 1) {
            char buf[2];
            buf[0] = PronounBuffer[0];
            buf[1] = 0;
            CharUpperA(buf);
            PronounBuffer[0] = buf[0];
            return PronounBuffer;
        }
        CharUpperA(PronounBuffer);
    }
    return PronounBuffer;
}

// ORIGINAL: 0x0058FB00 ?get_pact_hood@@YAPADHH@Z 0x0058FB00-0x0058FB61
// LEVER: the five fixed-address bindings renamed to the tree's own models - see
//         get_pacts above for the address map. No anonymous g_ binding arrived.
// TRIED: `int code = 0xcd; if (...) code = 0xce;`, the ternary form, and a
//         named `other_female` local for the second read - all three score the
//         same 22/29 as the shared-else `if/else`. The image has no upfront
//         constant store (both arms materialise into EAX), which the
//         shared-else form is the only one to reproduce. What is left is
//         register ASSIGNMENT: the image reuses the dead `Players[other_id]`
//         register in EAX for `code`, this tree puts it in ECX.

char *__cdecl get_pact_hood(int faction_id, int other_id) {
    GenderDefault = 0;
    PluralityDefault = 0;
    int code;
    if (Players[faction_id].is_leader_female != 0
        && Players[other_id].is_leader_female != 0) {
        code = 0xce;
    } else {
        code = 0xcd;
    }
    LPSTR *labels = (LPSTR *)Labels.strings_ptr;
    return StringTable.get((int)labels[code]);
}

extern void __cdecl clear_hotseat_chat();
extern int __cdecl tech_advance(int faction_id);
extern uint32_t TerraformAnnouncedMask[2];
extern int TutorialFlags;
extern int TutorialFlags2;

// ORIGINAL: 0x005B3920 ?setup_game@@YAXH@Z 0x005B3920-0x005B4201
// RECOVERY_BINDING: v1 {"evidence": ["Retained setup owner full native seed and independent full-body review; corrected shared social-effect seen state and pre-callback player labels; actual-source setup controls; no historical byte claim copied."], "reason": "Recover complete game setup using actual source-owned reset storage, player construction and diplomacy initialization; preserve native callback order and final time-warp versus balance branch."}
void __cdecl setup_game(int reload) {
    // This initializer deliberately retains the native order: profile state,
    // transient game state, player construction, then optional diplomacy.
    GameTechAchieved[MaxTechnologyNum - 1] = 0;
    LegacyGameSaveName[0] = 0;

    // The image passes zero-filled 0x945E78/0x945E7C strings to these keys.
    // Pinned-image initial bytes are NUL, and no other absolute references to
    // either cell exist; the existing prefs_put overload consumes the values.
    prefs_put("Latest Save", "");
    prefs_put("Latest Scenario", "");
    if (DiffLevelCurrent == 0 && reload == 0 &&
        prefs_get("DontResetBeginnerPrefs", 0, 0) == 0) {
        prefs_load(1);
        GameRules |= 0x20;
        prefs_put("DontResetBeginnerPrefs", 0, 0);
    }
    if (IsMultiplayerNet) GameRules &= ~0x20u;
    prefs_use();

    ObjectiveAchievePts = 0;
    ObjectiveReqVictory = 9999;
    ObjectivesSuddenDeathVictory = 9999;
    VictoryAchieveBonusPts = 0;
    StartingMissionYear = Rules.normal_starting_year;
    EndingMissionYear = DiffLevelCurrent < 3
        ? Rules.normal_end_year_low_three_diff
        : Rules.normal_end_year_high_three_diff;

    clear_monuments();
    wipe_undo();
    ReplayEventBytes = 0;
    GameState &= 0x01800000u;
    GameInterludeFlags = 0;
    TutorialFlags = 0;
    TutorialFlags2 = 0;
    TerraformAnnouncedMask[0] = 0;
    TerraformAnnouncedMask[1] = 0;
    TutWinFlags9A64BC = 0;
    BaseCurrentCount = 0;
    GameSetupResetWord = 0;
    TurnCurrentNum = 0;
    LockEnableMask = (LockEnableMask & ~0x0000FF00u) | 0x0000FF00u;
    ClimateRiseStep = 0;
    ClimateRiseLimit = 0;
    ClimateRiseAccum = 0;
    ClimateSeaLevelDelta = 0;
    clear_all_offers();
    clear_hotseat_chat();
    clear_council_notify();
    clear_tamper();
    TurnControlDurationMilliseconds = 0;
    GovernorFaction = -1;
    for (int proposal = 0; proposal < MaxProposalNum; ++proposal) {
        CouncilResolutionCount[proposal] = 0;
        CouncilLastResolutionTurn[proposal] = -9999;
    }

    if (!IsMultiplayerNet && reload != 0 && GameRestartQueued == 0)
        GameState |= 0xE0;

    // The reset words and pending-diplomacy matrix have independent source owners.
    for (int technology = 0; technology < MaxTechnologyNum; ++technology)
        GameTechAchieved[technology] = 0;
    memset(&SecretProject, 0xFF, sizeof SecretProject);
    memset(ReplayEvents, 0, sizeof ReplayEvents);
    memset(FactionRankingHistory, 0, sizeof FactionRankingHistory);
    SunspotDuration = 0;
    MountPlanetX = 0;
    MountPlanetY = 0;
    DustCloudDuration = 0;
    SaveDaemonStateWord = 0;
    ScnVictFacilityObj = 0;
    GlobalEnergyEventState = 0;
    NetTurnFlags = 0;
    NetTurnFaction = 0;
    EcologyEventAccumulator = 0;

    if ((GameState & 0x01000000u) != 0) {
        int placed = 1;
        int seen_social_effects[12] = {0};
        for (int social_faction = 1; social_faction < MaxPlayerNum; ++social_faction) {
            int rejected = 0;
            int category;
            int model;
            do {
                category = rand() % 3;
                model = rand() % 3 + 1;
                const int effect_index = category * 3 + model;
                if (seen_social_effects[effect_index] == 0) break;
                ++rejected;
                if (rejected >= placed + 1) break;
            } while (true);
            ++placed;
            if (rejected <= placed) {
                Players[social_faction].soc_ideology_category = category;
                Players[social_faction].soc_ideology_model = model;
                seen_social_effects[category * 3 + model] = 1;
                const SocialEffect &effects =
                    SocialCategories[category].model_effect[model];
                for (int effect = 0; effect < 11; ++effect) {
                    int value = 0;
                    switch (effect) {
                    case 0: value = effects.economy; break;
                    case 1: value = effects.efficiency; break;
                    case 2: value = effects.support; break;
                    case 3: value = effects.talent; break;
                    case 4: value = effects.morale; break;
                    case 5: value = effects.police; break;
                    case 6: value = effects.growth; break;
                    case 7: value = effects.planet; break;
                    case 8: value = effects.probe; break;
                    case 9: value = effects.industry; break;
                    case 10: value = effects.research; break;
                    }
                    if (value > 0) {
                        Players[social_faction].soc_ideology_effect = effect;
                        break;
                    }
                }
            }
        }
    }

    int setup_results[MaxPlayerNum] = {0};
    for (int human_setup_faction = 0; human_setup_faction < MaxPlayerNum; ++human_setup_faction) {
        if ((LockEnableMask & (1u << human_setup_faction)) != 0) {
            setup_player(human_setup_faction, -1 - reload, 0);
            setup_results[human_setup_faction] = 0;
        }
    }
    memset(TectonicDetonationCount, 0, sizeof TectonicDetonationCount);
    for (int computer_setup_faction = 0; computer_setup_faction < MaxPlayerNum; ++computer_setup_faction) {
        if ((LockEnableMask & (1u << computer_setup_faction)) == 0) {
            setup_player(computer_setup_faction, -1 - reload, 0);
            setup_results[computer_setup_faction] = 0;
        }
    }

    // This is the row-major 8x8 pending-diplomacy flag matrix. Its native
    // reset follows both setup_player passes and precedes the label copies.
    memset(PendingDiplomacyFlags, 0, sizeof PendingDiplomacyFlags);

    // The original publishes the default human's labels before the optional
    // new-game diplomacy/time-warp branch, whose callbacks observe them.
    strcpy(Players[0].formal_name_faction, label_get(245));
    strcpy(Players[0].noun_faction, label_get(245));
    strcpy(Players[0].adj_name_faction, label_get(244));

    if (reload == 0) {
        // The native random-pair bookkeeping visits factions 1..7. A marked
        // faction first gets paired with a distinct random faction; an
        // optional Player::rule_flags bit then asks for a second pair.
        for (int pair_faction = 1; pair_faction < MaxPlayerNum; ++pair_faction) {
            if (setup_results[pair_faction] == 0) {
                int tries = 0;
                int other;
                do {
                    do {
                        other = rand() % 7 + 1;
                        ++tries;
                    } while (other == pair_faction);
                } while ((setup_results[other] != 0 &&
                          (tries < 200 ||
                           (tries < 400 &&
                            (Players[other].rule_flags & 0x1000) == 0))) ||
                         ((LockEnableMask & (1u << pair_faction)) != 0 &&
                          (LockEnableMask & (1u << other)) != 0 && tries < 100));
                ++setup_results[pair_faction];
                ++setup_results[other];
            }
        }

        for (int extra_pair_faction = 1; extra_pair_faction < MaxPlayerNum; ++extra_pair_faction) {
            if (setup_results[extra_pair_faction] < 2 &&
                (Players[extra_pair_faction].rule_flags & 0x1000) != 0) {
                int tries = 0;
                int other;
                do {
                    do {
                        other = rand() % 7 + 1;
                    } while (other == extra_pair_faction);
                } while ((LockEnableMask & (1u << extra_pair_faction)) != 0 &&
                         (LockEnableMask & (1u << other)) != 0 &&
                         (++tries, tries < 200));
                ++setup_results[extra_pair_faction];
                ++setup_results[other];
            }
        }

        if (!IsMultiplayerNet || (GameRules & 0x40u) != 0) {
            if ((GameState & 0x00800000u) == 0) {
                static const char *const pairs[][2] = {
                    {"PEACE", "SPARTANS"}, {"GAIANS", "MORGAN"},
                    {"HIVE", "BELIEVE"}, {"UNIV", "BELIEVE"},
                    {"FUNGBOY", "BELIEVE"}, {"PEACE", "PIRATES"},
                    {"DRONE", "PIRATES"}, {"DRONE", "MORGAN"},
                    {"DRONE", "HIVE"}, {"ANGELS", "HIVE"},
                    {"ANGELS", "CYBORG"}
                };
                for (int first = 1; first < MaxPlayerNum; ++first) {
                    for (int second = 1; second < MaxPlayerNum; ++second) {
                        if (first != second) {
                            char first_key[24] = {0};
                            char second_key[24] = {0};
                            strcat(first_key, Players[first].search_key);
                            strcat(second_key, Players[second].search_key);
                            CharUpperA(first_key);
                            CharUpperA(second_key);

                            bool matched = false;
                            for (int pair = 0; pair < 11; ++pair) {
                                if (!strcmp(first_key, pairs[pair][0]) &&
                                    !strcmp(second_key, pairs[pair][1])) {
                                    matched = true;
                                    break;
                                }
                            }
                            if (matched || (Players[first].rule_flags & RFLAG_ALIEN) != 0 ||
                                (Players[second].rule_flags & RFLAG_ALIEN) != 0)
                                agenda_on(second, first, 0x200);

                            if ((Players[first].rule_flags & RFLAG_ALIEN) != 0 &&
                                (Players[second].rule_flags & RFLAG_ALIEN) != 0) {
                                set_treaty(second, first, 0xC80C0C38, 1);
                                set_agenda(second, first, 0x2C2D, 1);
                            }
                        }
                    }
                }
            } else {
                unsigned int rules_snapshot = GameRules;
                for (int network_faction = 1; network_faction < MaxPlayerNum; ++network_faction) {
                    if ((LockEnableMask & (1u << network_faction)) != 0) {
                        int required = ((rules_snapshot >> 6) & 1u) +
                            (PlayersData[network_faction].diff_level > 2 ? 1 : 0);
                        int assigned = 0;
                        while (assigned < required) {
                            int tries = 0;
                            int other;
                            for (;;) {
                                other = rand() % 7 + 1;
                                ++tries;
                                if (tries > 99) break;
                                if ((PlayersData[other].diplo_agenda[network_faction] & 0x200) == 0) {
                                    const unsigned int other_bit = 1u << other;
                                    const bool alive = (LockEnableMask & (other_bit << 8)) != 0;
                                    const bool human = (LockEnableMask & other_bit) != 0;
                                    if (alive && !human) break;
                                }
                            }
                            if (tries < 100) agenda_on(other, network_faction, 0x200);
                            ++assigned;
                            rules_snapshot = GameRules;
                            required = ((rules_snapshot >> 6) & 1u) +
                                (PlayersData[network_faction].diff_level > 2 ? 1 : 0);
                        }
                    }
                }
            }
        }

        if ((GameRules & 0x80u) != 0) {
            time_warp();
            return;
        }
        balance();
    }

}

static int time_warp_find_prototype(int faction_id, int plan) {
    const uint8_t faction_bit = static_cast<uint8_t>(1u << faction_id);
    for (int scan_index = 0; scan_index < 128; ++scan_index) {
        const int prototype_id = scan_index < 64
            ? scan_index : faction_id * 64 + scan_index - 64;
        const VehPrototype &prototype = VehPrototypes[prototype_id];
        if ((prototype.flags & 1) == 0 ||
            (prototype.obsolete_factions & faction_bit) != 0) continue;
        if (prototype_id < 64 && !has_tech(prototype.preq_tech, faction_id)) continue;
        if (prototype.plan == plan) return prototype_id;
    }
    return -1;
}
// ORIGINAL: 0x005AEDE0 ?time_warp@@YAXXZ 0x005AEDE0-0x005B0412
// RECOVERY_BINDING: v1 {"evidence": ["Independent pinned full-span review by report_draw_labs_review; retained game-setup-owner-wave full body and actual-source controls for three profiles, land/aquatic sites, path modes and native relocation."], "reason": "Recover complete accelerated-start setup with typed faction, base, prototype, map and Path owners; preserve callback-sensitive region rereads and native facility/path order. External unfinished callees remain explicit."}
void __cdecl time_warp() {
    const int variation = rand() % 3;
    int scenario_turn;
    int initial_research;
    int research_adjustment;
    int expansion_count;
    int initial_population;
    if (variation == 0) {
        scenario_turn = 30;
        initial_research = 6;
        research_adjustment = 2;
        expansion_count = 3;
        initial_population = 5;
    } else if (variation == 1) {
        scenario_turn = 60;
        initial_research = 12;
        research_adjustment = 3;
        expansion_count = 4;
        initial_population = 6;
    } else {
        scenario_turn = 80;
        initial_research = 16;
        research_adjustment = 4;
        expansion_count = 5;
        initial_population = 6;
    }

    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        const uint32_t alive_faction = 1u << (faction_id + 8);
        if ((LockEnableMask & alive_faction) == 0) continue;
        const int starting_base_id = base_find(0, 0, faction_id);
        if (starting_base_id < 0) continue;

        Base &starting_base = Bases[starting_base_id];
        Player &player = Players[faction_id];
        PlayerData &player_data = PlayersData[faction_id];
        starting_base.population_size = static_cast<int8_t>(initial_population);
        if (player.rule_talent != 0) ++starting_base.population_size;
        if (player.rule_drone != 0 || player.rule_population != 0)
            --starting_base.population_size;
        starting_base.nutrients_accumulated =
            static_cast<int>(starting_base.population_size) * 10 / 2;

        const int starting_region = map_loc(starting_base.x, starting_base.y)->region;
        const int wealth_sign = player_data.ai_wealth < 1
            ? (player_data.ai_wealth >= 0 ? 0 : -1) : 1;
        const int power_sign = player_data.ai_power < 1
            ? (player_data.ai_power >= 0 ? 0 : -1) : 1;
        const int research_advances =
            (wealth_sign - power_sign) * research_adjustment + initial_research;

        AiResearchBatchActive = 1;
        for (int advance = 0; advance < research_advances; ++advance) {
            player_data.tech_id_researching = tech_ai(faction_id);
            tech_advance(faction_id);
        }
        player_data.tech_accumulated = static_cast<int>(player_data.tech_achieved << 1);
        AiResearchBatchActive = 0;
        player_data.tech_id_researching = -1;
        consider_designs(static_cast<uint32_t>(faction_id));

        set_fac(FAC_RECYCLING_TANKS, starting_base_id, 1);
        set_fac(FAC_RECREATION_COMMONS, starting_base_id, 1);
        if (player_data.ai_wealth != 0)
            set_fac(FAC_ENERGY_BANK, starting_base_id, 1);
        if (player_data.ai_power != 0)
            set_fac(FAC_COMMAND_CENTER, starting_base_id, 1);
        if (player_data.ai_growth != 0)
            set_fac(FAC_NETWORK_NODE, starting_base_id, 1);

        if (player_data.ai_fight != 0) {
            int initial_foil;
            if (ExpansionEnabled == 0 || (player.rule_flags & RFLAG_AQUATIC) == 0) {
                initial_foil = BSC_UNITY_ROVER;
            } else {
                initial_foil = has_tech(VehPrototypes[BSC_UNITY_FOIL].preq_tech,
                                        faction_id)
                    ? BSC_TRANSPORT_FOIL : BSC_UNITY_FOIL;
            }
            veh_init(initial_foil, faction_id, starting_base.x, starting_base.y);
            // Native pushes (facility, 1, starting-base-id) in this branch,
            // unlike the ordinary facility stores immediately above.
            set_fac(FAC_PERIMETER_DEFENSE, 1, starting_base_id);
        }

        // Starting unit selection first searches predefined designs, then
        // the faction's 64-design bank. The scan is ordered, and technology
        // is checked only for the predefined range.
        const int plan_six_prototype = time_warp_find_prototype(faction_id, 6);
        const BOOL has_foil_technology =
            has_tech(VehPrototypes[BSC_UNITY_FOIL].preq_tech, faction_id);
        int opening_prototype = time_warp_find_prototype(faction_id, 2);
        if (opening_prototype < 0)
            opening_prototype = time_warp_find_prototype(faction_id, 1);
        if (opening_prototype < 0) opening_prototype = 2;
        VehPrototypes[opening_prototype].flags |= 4;
        int opening_vehicle = veh_init(opening_prototype, faction_id,
                                       starting_base.x, starting_base.y);
        Vehs[opening_vehicle].waypoint_count = 2;
        ++Vehs[opening_vehicle].morale;
        opening_vehicle = veh_init(opening_prototype, faction_id,
                                   starting_base.x, starting_base.y);
        Vehs[opening_vehicle].waypoint_count = 2;
        const int third_prototype =
            ExpansionEnabled == 0 || (player.rule_flags & RFLAG_AQUATIC) == 0 ? 1 : 4;
        veh_init(third_prototype, faction_id, starting_base.x, starting_base.y);

        const int plan_one_prototype = time_warp_find_prototype(faction_id, 1);
        if (plan_one_prototype >= 0) {
            VehPrototypes[plan_one_prototype].flags |= 4;
            const int vehicle = veh_init(plan_one_prototype, faction_id,
                                         starting_base.x, starting_base.y);
            ++Vehs[vehicle].morale;
            if (plan_one_prototype == BSC_MIND_WORMS) Vehs[vehicle].morale = 2;
        }

        const int plan_zero_prototype = time_warp_find_prototype(faction_id, 0);
        if (plan_zero_prototype >= 0) {
            VehPrototypes[plan_zero_prototype].flags |= 4;
            if (player_data.net_random_event < 1 && player_data.ai_power == 0) {
                // The native branch deliberately skips this plan-0 vehicle,
                // then repeats its plan-1 scan and may add that design instead.
                const int fallback = time_warp_find_prototype(faction_id, 1);
                if (fallback >= 0) {
                    VehPrototypes[fallback].flags |= 4;
                    const int vehicle = veh_init(fallback, faction_id,
                                                 starting_base.x, starting_base.y);
                    if (fallback == BSC_MIND_WORMS) Vehs[vehicle].morale = 2;
                }
            } else {
                int vehicle = veh_init(plan_zero_prototype, faction_id,
                                       starting_base.x, starting_base.y);
                ++Vehs[vehicle].morale;
                if (player_data.net_random_event > 0)
                    veh_init(plan_zero_prototype, faction_id,
                             starting_base.x, starting_base.y);
            }
        }

        int starting_project;
        int current_project_base;
        do {
            starting_project = rand() % 8;
            switch (starting_project) {
            case 0: current_project_base = SecretProject.human_genome_project; break;
            case 1: current_project_base = SecretProject.command_nexus; break;
            case 2: current_project_base = SecretProject.weather_paradigm; break;
            case 3: current_project_base = SecretProject.merchant_exchange; break;
            case 5: current_project_base = SecretProject.citizens_defense_force; break;
            case 6: current_project_base = SecretProject.virtual_world; break;
            default: current_project_base = SecretProject.planetary_transit_system; break;
            }
        } while (current_project_base != SP_Unbuilt || starting_project == 4 ||
                 (starting_project == 5 && strcmp(player.filename, "HIVE") == 0));
        switch (starting_project) {
        case 0: SecretProject.human_genome_project = starting_base_id; break;
        case 1: SecretProject.command_nexus = starting_base_id; break;
        case 2: SecretProject.weather_paradigm = starting_base_id; break;
        case 3: SecretProject.merchant_exchange = starting_base_id; break;
        case 5: SecretProject.citizens_defense_force = starting_base_id; break;
        case 6: SecretProject.virtual_world = starting_base_id; break;
        default: SecretProject.planetary_transit_system = starting_base_id; break;
        }

        // The native expansion pass considers expansion_count - 1 rounds.
        // A round chooses the best free site around any faction base, then
        // establishes one new settlement and its opening garrison.
        bool placed_coastal_group = false;
        for (int expansion_round = 0; expansion_round < expansion_count - 1;
             ++expansion_round) {
            int best_score = 0;
            int best_x = 0;
            int best_y = 0;
            int best_base_id = -1;
            int scan_base_id = 0;
            int last_base_x = 0;
            int last_base_y = 0;
            if (BaseCurrentCount > 0) {
                for (; scan_base_id < BaseCurrentCount; ++scan_base_id) {
                    Base &search_base = Bases[scan_base_id];
                    if (search_base.faction_id_current != faction_id) continue;
                    const int center_x = search_base.x;
                    const int center_y = search_base.y;
                    last_base_x = center_x;
                    last_base_y = center_y;
                    const int center_region = map_loc(center_x, center_y)->region;
                    for (int offset_index = 25; offset_index < 289; ++offset_index) {
                        const int offset_distance = (offset_index - 25) * 4;
                        const int offset_x = RadiusOffsetX[offset_index];
                        const int offset_y = RadiusOffsetY[offset_index];
                        int candidate_x = center_x + offset_x;
                        if (!(MapIsFlat & 1)) {
                            if (candidate_x < 0) candidate_x += MapLongitudeBounds;
                            else if (candidate_x >= MapLongitudeBounds)
                                candidate_x -= MapLongitudeBounds;
                        }
                        const int candidate_y = center_y + offset_y;
                        if (candidate_x < 0 || candidate_x >= MapLongitudeBounds ||
                            candidate_y < 0 || candidate_y >= MapLatitudeBounds) continue;

                        Map *candidate = map_loc(candidate_x, candidate_y);
                        const int altitude = candidate->climate & 0xE0;
                        const uint8_t candidate_region = candidate->region;
                        const bool aquatic = (player.rule_flags & RFLAG_AQUATIC) != 0;
                        if (((candidate->bit & 0x20u) != 0 && altitude >= 0x40) ||
                            (aquatic ? altitude >= 0x60 : altitude < 0x60)) continue;

                        const uint8_t owner_state = candidate->val3;
                        if ((owner_state & 0xC0) >= 0x41 && !aquatic) continue;
                        int site_bonus = 0;
                        if ((owner_state & 7) == 0 && (candidate->bit & 0x1000u) == 0) {
                            site_bonus = candidate->climate >> 4;
                            if (site_bonus == 0) {
                                site_bonus = world_site(candidate_x, candidate_y,
                                                        aquatic ? TRUE : FALSE);
                                site_set(candidate_x, candidate_y, site_bonus);
                            }
                        }

                        int score = (static_cast<int>(search_base.population_size) + site_bonus) * 32;
                        if (center_region != candidate_region)
                            score /= 2;
                        if (offset_distance > 0xDF && offset_distance < 0x240)
                            score = (score * 3) / 2;
                        if (offset_distance > 799)
                            score -= (score + (score >> 31 & 3)) >> 2;
                        if (offset_distance < 0x60) score /= 2;

                        const int nearest_base_id = base_find(candidate_x, candidate_y);
                        if (nearest_base_id != scan_base_id) {
                            const Base &nearest_base = Bases[nearest_base_id];
                            int dx = abs(candidate_x - nearest_base.x);
                            if (!(MapIsFlat & 1) && dx > static_cast<int>(MapLongitude))
                                dx = MapLongitudeBounds - dx;
                            const int dy = abs(candidate_y - nearest_base.y);
                            if (((dx + dy) & ~1) < 8) continue;
                        }
                        const Base &nearest_base = Bases[nearest_base_id];
                        if (nearest_base.faction_id_current != faction_id) {
                            if (map_loc(candidate_x, candidate_y)->region == candidate_region) continue;
                            score /= 2;
                        }
                        if (score > best_score) {
                            best_score = score;
                            best_x = candidate_x;
                            best_y = candidate_y;
                            best_base_id = scan_base_id;
                        }
                    }
                }
            }

            if (best_score <= 0) {
                if (BaseCurrentCount > 0) {
                    int smallest_base_id = -1;
                    int smallest_population = 999;
                    for (int candidate_base_id = 0;
                         candidate_base_id < BaseCurrentCount; ++candidate_base_id) {
                        const Base &candidate_base = Bases[candidate_base_id];
                        if (candidate_base.faction_id_current == faction_id &&
                            candidate_base.population_size < smallest_population) {
                            smallest_population = candidate_base.population_size;
                            smallest_base_id = candidate_base_id;
                        }
                    }
                    if (smallest_base_id >= 0) {
                        Base &smallest_base = Bases[smallest_base_id];
                        if (smallest_base.population_size < 6)
                            smallest_base.population_size += 2;
                        set_fac(FAC_RECREATION_COMMONS, smallest_base_id, 1);
                        set_fac(FAC_RECYCLING_TANKS, smallest_base_id, 1);
                        set_fac(FAC_COMMAND_CENTER, smallest_base_id, 1);
                        int garrison = time_warp_find_prototype(faction_id, 1);
                        if (garrison >= 0) {
                            VehPrototypes[garrison].flags |= 4;
                            int new_vehicle = veh_init(garrison, faction_id,
                                                       smallest_base.x, smallest_base.y);
                            if (garrison == BSC_MIND_WORMS) Vehs[new_vehicle].morale = 2;
                        }
                        if (ExpansionEnabled == 0 ||
                            (player.rule_flags & RFLAG_AQUATIC) == 0)
                            veh_init(0, faction_id, smallest_base.x, smallest_base.y);
                        else
                            veh_init(21, faction_id, last_base_x, last_base_y);
                        veh_init(7, faction_id, smallest_base.x, smallest_base.y);
                    }
                }
                continue;
            }

            const int new_base_id = base_init(faction_id, best_x, best_y);
            Base &new_base = Bases[new_base_id];
            new_base.population_size = static_cast<int8_t>(expansion_count - expansion_round);
            if (new_base.population_size > 2) set_fac(FAC_RECREATION_COMMONS, new_base_id, 1);
            if (new_base.population_size > 3) set_fac(FAC_RECYCLING_TANKS, new_base_id, 1);
            new_base.nutrients_accumulated = static_cast<int>(new_base.population_size) * 10 / 2;

            int new_vehicle = veh_init(opening_prototype, faction_id, best_x, best_y);
            if (opening_prototype == BSC_MIND_WORMS) Vehs[new_vehicle].morale = 2;
            Vehs[new_vehicle].waypoint_count = 2;
            if (new_base.population_size > 2) {
                new_vehicle = veh_init(opening_prototype, faction_id, best_x, best_y);
                if (opening_prototype == BSC_MIND_WORMS) Vehs[new_vehicle].morale = 2;
                Vehs[new_vehicle].waypoint_count = 2;
            }
            if (player_data.ai_fight != 0 || new_base.population_size > 3) {
                const int defender_prototype =
                    (player.rule_flags & RFLAG_AQUATIC) != 0 ? 4 : 1;
                veh_init(defender_prototype, faction_id, best_x, best_y);
            }

            const Map *new_base_tile = map_loc(best_x, best_y);
            if (new_base_tile->region != starting_region && !placed_coastal_group &&
                is_coast(Bases[best_base_id].x, Bases[best_base_id].y, FALSE)) {
                for (int coast_index = 0; coast_index < 8; ++coast_index) {
                    int coast_x = best_x + VehMoveDeltaX[coast_index];
                    if (!(MapIsFlat & 1)) {
                        if (coast_x < 0) coast_x += MapLongitudeBounds;
                        else if (coast_x >= MapLongitudeBounds) coast_x -= MapLongitudeBounds;
                    }
                    const int coast_y = best_y + VehMoveDeltaY[coast_index];
                    if (coast_x < 0 || coast_x >= MapLongitudeBounds || coast_y < 0 ||
                        coast_y >= MapLatitudeBounds) continue;
                    if ((map_loc(coast_x, coast_y)->climate & 0xE0) >= 0x60) continue;
                    if (plan_six_prototype >= 0) {
                        VehPrototypes[plan_six_prototype].flags |= 4;
                        const int native_vehicle = veh_init(plan_six_prototype, faction_id,
                                                             coast_x, coast_y);
                        ++Vehs[native_vehicle].morale;
                    }
                    veh_init(has_foil_technology ? BSC_TRANSPORT_FOIL : BSC_UNITY_FOIL,
                             faction_id, coast_x, coast_y);
                    const int sleeping_vehicle = veh_init(BSC_UNITY_ROVER, faction_id,
                                                          coast_x, coast_y);
                    sleep(sleeping_vehicle);
                    placed_coastal_group = true;
                    break;
                }
            }
        }

        AiResearchBatchActive = 1;
        reset_territory();
        enemy_strategy(faction_id);
        for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
            Base &base = Bases[base_id];
            if (base.faction_id_current != faction_id) continue;
            for (int synch_radius_index = 0; synch_radius_index < 81; ++synch_radius_index) {
                int radius_x = base.x + RadiusOffsetX[synch_radius_index];
                if (!(MapIsFlat & 1)) {
                    if (radius_x < 0) radius_x += MapLongitudeBounds;
                    else if (radius_x >= MapLongitudeBounds)
                        radius_x -= MapLongitudeBounds;
                }
                const int radius_y = base.y + RadiusOffsetY[synch_radius_index];
                if (radius_x < 0 || radius_x >= MapLongitudeBounds || radius_y < 0 ||
                    radius_y >= MapLatitudeBounds) continue;
                Map *tile = map_loc(radius_x, radius_y);
                tile->visibility |= static_cast<uint8_t>(1u << faction_id);
                if (synch_radius_index < 21 && goody_at(radius_x, radius_y) != 0)
                    tile->bit |= 0x00100400u;
            }
            set_base(base_id);
            base_compute(1);
            const int terraform_passes =
                (static_cast<int>(base.population_size) * 2) / 3 + player_data.ai_fight;
            for (int pass = 0; pass < terraform_passes; ++pass) {
                base_terraform(1);
                base_compute(1);
            }
            base.resource_sqr_radius = 0;

            for (int radius_index = 0; radius_index < 81; ++radius_index) {
                int radius_x = base.x + RadiusOffsetX[radius_index];
                if (!(MapIsFlat & 1)) {
                    if (radius_x < 0) radius_x += MapLongitudeBounds;
                    else if (radius_x >= MapLongitudeBounds)
                        radius_x -= MapLongitudeBounds;
                }
                const int radius_y = base.y + RadiusOffsetY[radius_index];
                if (radius_x >= 0 && radius_x < MapLongitudeBounds && radius_y >= 0 &&
                    radius_y < MapLatitudeBounds)
                    synch_bit(radius_x, radius_y, faction_id);
            }
            const int base_region = map_loc(base.x, base.y)->region;
            for (int other_base_id = 0; other_base_id < BaseCurrentCount; ++other_base_id) {
                if (other_base_id == base_id ||
                    Bases[other_base_id].faction_id_current != faction_id) continue;
                Base &other_base = Bases[other_base_id];
                if (map_loc(other_base.x, other_base.y)->region != base_region) continue;
                const int has_tech_for_path =
                    has_tech(Rules.tech_build_roads_fungus, faction_id);
                const int path_mode = has_tech_for_path ? 0 : 4;
                int path_step = Paths.find(other_base.x, other_base.y, base.x, base.y,
                                           0, -1, path_mode, -1);
                int path_x = other_base.x;
                int path_y = other_base.y;
                int path_length = 0;
                while (path_step >= 0) {
                    path_x += VehMoveDeltaX[path_step];
                    if (!(MapIsFlat & 1)) {
                        if (path_x < 0) path_x += MapLongitudeBounds;
                        else if (path_x >= MapLongitudeBounds)
                            path_x -= MapLongitudeBounds;
                    }
                    path_y += VehMoveDeltaY[path_step];
                    if (path_x == base.x && path_y == base.y) break;
                    Map *path_tile = map_loc(path_x, path_y);
                    if ((path_tile->bit & 1u) == 0 || (path_tile->val2 & 0x0F) > 7) {
                        if ((path_tile->bit & 4u) == 0) {
                            path_tile->bit |= 4u;
                            synch_bit(path_x, path_y, faction_id);
                        }
                    }
                    ++path_length;
                    if (path_length > 50) break;
                    const int can_road_fungus =
                        has_tech(Rules.tech_build_roads_fungus, faction_id);
                    path_step = Paths.find(path_x, path_y, base.x, base.y,
                                           0, -1, can_road_fungus ? 0 : 4, -1);
                }
            }
            base_reset(base_id, 0);
        }
        AiResearchBatchActive = 0;

        for (int vehicle_id = 0; vehicle_id < VehCurrentCount; ++vehicle_id) {
            Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id != faction_id ||
                VehPrototypes[vehicle.proto_id].plan != 9) continue;
            const int native_triad = Chassis[VehPrototypes[vehicle.proto_id].chassis_id].triad;
            const bool aquatic = (player.rule_flags & RFLAG_AQUATIC) != 0;
            if ((!aquatic && native_triad != TRIAD_LAND) ||
                (aquatic && native_triad != TRIAD_SEA)) continue;
            int best_quality = 0;
            for (int move_index = 0; move_index < 8; ++move_index) {
                int destination_x = vehicle.x + VehMoveDeltaX[move_index];
                if (!(MapIsFlat & 1)) {
                    if (destination_x < 0) destination_x += MapLongitudeBounds;
                    else if (destination_x >= MapLongitudeBounds)
                        destination_x -= MapLongitudeBounds;
                }
                const int destination_y = vehicle.y + VehMoveDeltaY[move_index];
                if (destination_x < 0 || destination_x >= MapLongitudeBounds ||
                    destination_y < 0 || destination_y >= MapLatitudeBounds) continue;
                const Map *destination = map_loc(destination_x, destination_y);
                const int altitude = destination->climate & 0xE0;
                if ((!aquatic && altitude < 0x60) || (aquatic && altitude >= 0x60)) continue;
                int quality = 1;
                const uint8_t owner_state = destination->val3 & 0xC0;
                if (owner_state < 0x80) quality = 2;
                if ((destination->bit & 4u) != 0) quality += 2;
                if (owner_state == 0x40 && (destination->climate & 0x18) != 0 &&
                    (destination->climate >> 5) > 2)
                    quality += 2;
                if (quality > best_quality) {
                    best_quality = quality;
                    veh_drop(veh_lift(vehicle_id), destination_x, destination_y);
                }
            }
        }
    }
    TurnCurrentNum = scenario_turn;
}

void __cdecl monument(int from_load);

// ORIGINAL: 0x00476B70 ?mon_colony_founded@@YAXHPAD@Z 0x00476B70-0x00476C84 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x00476B70-0x00476C84", "source:src/recovered/units/00476b70.cpp", "Original faction stride4F4, record0 flag/first-owner words and name copy to4AC determine existing type ownership; preserve24byte strncpy behavior."], "reason": "Recover first colony founding in the shared genuine FactionMonuments owner, including first-achiever selection, the bounded faction-owned base-name field and live local notification."}
void __cdecl mon_colony_founded(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(TurnCurrentNum));
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;
    MonumentFaction &faction = FactionMonuments[faction_id];
    MonumentRecord &record = faction.records_[0];
    if (record.achieved_ != 0)
        return;

    record.word_14_ = 0;
    int peer;
    for (peer = 0; peer < MaxPlayerNum; ++peer) {
        if (peer != faction_id && FactionMonuments[peer].records_[0].achieved_ != 0)
            ++record.word_14_;
    }
    int first_faction = faction_id;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            const MonumentRecord &other = FactionMonuments[peer].records_[0];
            if (peer != faction_id && other.achieved_ != 0 && other.word_14_ != 0)
                first_faction = peer;
        }
    } else {
        record.word_14_ = 1;
    }
    record.owner_faction_ = first_faction;
    record.achieved_ = 1;
    record.year_ = year;
    // Native strncpy copies exactly 24 bytes; a full-length name is not
    // forcibly terminated here. The field remains owned by this faction.
    strncpy(faction.founding_base_name_, base_name, sizeof(faction.founding_base_name_));
    faction.latest_type_ = 0;
    if (faction_id == ConsoleGlobal.faction_id_)
        monument(1);
}
// ORIGINAL: 0x00476C90 ?mon_tech_discovered@@YAXHH@Z 0x00476C90-0x00476D93
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00476C90 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/recovered/units/00476c90.cpp; record1 at factionstride4F4, year0, payload8, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
void __cdecl mon_tech_discovered(int faction_id, int tech_id) {
    const int year = static_cast<int>(game_year(TurnCurrentNum));
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;
    MonumentFaction &faction = FactionMonuments[faction_id];
    MonumentRecord &record = faction.records_[1];
    if (record.achieved_ != 0)
        return;

    record.word_14_ = 0;
    int peer;
    for (peer = 0; peer < MaxPlayerNum; ++peer) {
        if (peer != faction_id && FactionMonuments[peer].records_[1].achieved_ != 0)
            ++record.word_14_;
    }
    int first_faction = faction_id;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            const MonumentRecord &other = FactionMonuments[peer].records_[1];
            if (peer != faction_id && other.achieved_ != 0 && other.word_14_ != 0)
                first_faction = peer;
        }
    } else {
        record.word_14_ = 1;
    }
    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = first_faction;
    record.word_8_ = tech_id;
    faction.latest_type_ = 1;
    if (faction_id == ConsoleGlobal.faction_id_)
        monument(1);
}

// ORIGINAL: 0x00477100 ?mon_secret_project@@YAXHH@Z 0x00477100-0x00477207
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00477100 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/recovered/units/00477100.cpp; record5 at factionstride4F4, year0, payload8, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
void __cdecl mon_secret_project(int faction_id, int project_id) {
    const int year = static_cast<int>(game_year(TurnCurrentNum));
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;
    MonumentFaction &faction = FactionMonuments[faction_id];
    MonumentRecord &record = faction.records_[5];
    if (record.achieved_ != 0)
        return;

    record.word_14_ = 0;
    int peer;
    for (peer = 0; peer < MaxPlayerNum; ++peer) {
        if (peer != faction_id && FactionMonuments[peer].records_[5].achieved_ != 0)
            ++record.word_14_;
    }
    int first_faction = faction_id;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            const MonumentRecord &other = FactionMonuments[peer].records_[5];
            if (peer != faction_id && other.achieved_ != 0 && other.word_14_ != 0)
                first_faction = peer;
        }
    } else {
        record.word_14_ = 1;
    }
    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = first_faction;
    record.word_8_ = project_id;
    faction.latest_type_ = 5;
    if (faction_id == ConsoleGlobal.faction_id_)
        monument(1);
}

// ORIGINAL: 0x00477840 ?mon_built_preserve@@YAXH@Z 0x00477840-0x0047793E
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00477840 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/unrecovered/00477840.cpp; record12 at factionstride4F4, year0, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
// TRIED: single `if(!(flag)&&active==0)` short-circuit form vs hoisting `winner`/`count=0` before the scan loop - both land the same MISMATCH #3 (original reserves the stack slot for `year` with `push ecx` ahead of the ebx/esi pushes; every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_built_preserve(int faction_id) {
    const int year = static_cast<int>(game_year(TurnCurrentNum));
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;
    MonumentFaction &faction = FactionMonuments[faction_id];
    MonumentRecord &record = faction.records_[12];
    if (record.achieved_ != 0)
        return;

    record.word_14_ = 0;
    int peer;
    for (peer = 0; peer < MaxPlayerNum; ++peer) {
        if (peer != faction_id && FactionMonuments[peer].records_[12].achieved_ != 0)
            ++record.word_14_;
    }
    int first_faction = faction_id;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            const MonumentRecord &other = FactionMonuments[peer].records_[12];
            if (peer != faction_id && other.achieved_ != 0 && other.word_14_ != 0)
                first_faction = peer;
        }
    } else {
        record.word_14_ = 1;
    }
    record.achieved_ = 1;
    record.owner_faction_ = first_faction;
    record.year_ = year;
    faction.latest_type_ = 12;
    if (faction_id == ConsoleGlobal.faction_id_)
        monument(1);
}

// ORIGINAL: 0x00476FE0 ?mon_facility_built@@YAXHPAD@Z 0x00476FE0-0x004770F2
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00476FE0 and seed git:fc8112d1ca80fbe743dd051511ad7e0af8454e94:src/recovered/units/00476fe0.cpp; faction stride4F4, record4, name field4C4 and24byte strncpy are established by the complete native body.", "Native first-achiever selection includes all eight peer factions; alien target and duplicate events return without updates. Local notification reads the live ConsoleGlobal owner after game_year and name copying."], "reason": "Recover the native milestone and its bounded base-name storage in the genuine shared FactionMonuments owner, not a duplicate raw table view."}
void __cdecl mon_facility_built(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(TurnCurrentNum));
    MonumentFaction &faction = FactionMonuments[faction_id];
    MonumentRecord &record = faction.records_[4];
    if (record.achieved_ != 0 || (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;

    record.word_14_ = 0;
    int peer;
    for (peer = 0; peer < MaxPlayerNum; ++peer) {
        if (peer != faction_id && FactionMonuments[peer].records_[4].achieved_ != 0)
            ++record.word_14_;
    }
    int first_faction = faction_id;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            const MonumentRecord &other = FactionMonuments[peer].records_[4];
            if (peer != faction_id && other.achieved_ != 0 && other.word_14_ != 0)
                first_faction = peer;
        }
    } else {
        record.word_14_ = 1;
    }
    record.owner_faction_ = first_faction;
    record.achieved_ = 1;
    record.year_ = year;
    // Native copies exactly 24 bytes, including strncpy's zero padding but
    // without forcing a terminator for a full-length name.
    strncpy(faction.facility_base_name_, base_name, sizeof(faction.facility_base_name_));
    const bool local = faction_id == ConsoleGlobal.faction_id_;
    faction.latest_type_ = 4;
    if (local)
        monument(1);
}

// ORIGINAL: 0x00477320 ?mon_conquer_base@@YAXHPAD@Z 0x00477320-0x00477434
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00477320 and seed git:fc8112d1ca80fbe743dd051511ad7e0af8454e94:src/recovered/units/00477320.cpp; faction stride4F4, record7, name field4DC and24byte strncpy are established by the complete native body.", "Native first-achiever selection includes all eight peer factions; alien target and duplicate events return without updates. Local notification reads the live ConsoleGlobal owner after game_year and name copying."], "reason": "Recover the native milestone and its bounded base-name storage in the genuine shared FactionMonuments owner, not a duplicate raw table view."}
void __cdecl mon_conquer_base(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(TurnCurrentNum));
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;
    MonumentFaction &faction = FactionMonuments[faction_id];
    MonumentRecord &record = faction.records_[7];
    if (record.achieved_ != 0)
        return;

    record.word_14_ = 0;
    int peer;
    for (peer = 0; peer < MaxPlayerNum; ++peer) {
        if (peer != faction_id && FactionMonuments[peer].records_[7].achieved_ != 0)
            ++record.word_14_;
    }
    int first_faction = faction_id;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            const MonumentRecord &other = FactionMonuments[peer].records_[7];
            if (peer != faction_id && other.achieved_ != 0 && other.word_14_ != 0)
                first_faction = peer;
        }
    } else {
        record.word_14_ = 1;
    }
    record.owner_faction_ = first_faction;
    record.achieved_ = 1;
    record.year_ = year;
    strncpy(faction.conquered_base_name_, base_name, sizeof(faction.conquered_base_name_));
    const bool local = faction_id == ConsoleGlobal.faction_id_;
    faction.latest_type_ = 7;
    if (local)
        monument(1);
}

// Opcode0x24 transfers this persistent window at 0x0094B4D8.
// No semantic fields or complete original allocation extent are asserted.
// RECOVERY_OBJECT: v1 0x0094B4D8 ?GameWireOpaque_94B4D8@@3PAEA
uint8_t GameWireOpaque_94B4D8[0x80];

// ORIGINAL: 0x00476DA0 ?mon_secrets_of_tech@@YAXH@Z 0x00476DA0-0x00476EDD
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00476da0.cpp preserved original identity/extent; pinned instructions and independent family review establish317byte body and fields; actual-source milestone/reset controls verify shared ownership and native branches"], "reason": "Record first/repeated technology secrets in genuine shared monument record2, retain first-achiever peer interpretation and notify the live local faction."}
void __cdecl mon_secrets_of_tech(int faction_id) {
    const int year = static_cast<int>(game_year(TurnCurrentNum));
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;

    MonumentRecord &record = FactionMonuments[faction_id].records_[2];
    if (record.achieved_ != 0) {
        if (record.word_8_ == -1)
            record.word_8_ = year;
        else if (record.word_4_ == -1)
            record.word_4_ = year;
    } else {
        record.word_14_ = 0;
        int peer;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            if (peer != faction_id &&
                FactionMonuments[peer].records_[2].achieved_ != 0)
                ++record.word_14_;
        }

        int first_faction = faction_id;
        if (record.word_14_ != 0) {
            record.word_14_ = 0;
            for (peer = 0; peer < MaxPlayerNum; ++peer) {
                const MonumentRecord &other = FactionMonuments[peer].records_[2];
                if (peer != faction_id && other.achieved_ != 0 &&
                    other.word_14_ != 0)
                    first_faction = peer;
            }
        } else {
            record.word_14_ = 1;
        }
        record.owner_faction_ = first_faction;
        record.year_ = year;
        record.achieved_ = 1;
        record.word_8_ = -1;
        record.word_4_ = -1;
    }
    FactionMonuments[faction_id].latest_type_ = 2;
    if (faction_id == ConsoleGlobal.faction_id_)
        monument(1);
}

// Shared load request: NetWin47D501/47D79D and58E79B/58E7AD write it;
// game_data5A7951 reads it under read mode. Broader storage remains unknown.
// RECOVERY_OBJECT: v1 0x00945D78 ?GameDataLoadRequest@@3HA
int GameDataLoadRequest;


// Save/load orchestration shares the transaction words below.

// Native transaction words: map_data writes the operation mode at 0x5A9295
// and toggles encrypted mode in game_data callers; consumers read these words
// at the same fixed image locations. 0x945D6C / 0x945D74 are distinct fields.
// RECOVERY_OBJECT: v1 0x00945D6C ?GameDataWriteMode@@3HA
int GameDataWriteMode;       // native storage start 0x00945D6C
// RECOVERY_OBJECT: v1 0x00945D74 ?GameDataEncryptedMode@@3HA
int GameDataEncryptedMode;   // native storage start 0x00945D74
// RECOVERY_OBJECT: v1 0x00945D7C ?GameDataLoadStatusWord@@3HA
int GameDataLoadStatusWord;  // native storage start 0x00945D7C

extern int __cdecl game_data(FILE *file, int load_request);

int __cdecl encrypt_read(void *buffer, unsigned int size, unsigned int count, FILE *file);

int __cdecl encrypt_write(void *buffer, unsigned int size, unsigned int count, FILE *file);

static const char MapHeaderText[] = "TERRANMAP";

static const char SaveHeaderText[] = "TERRAN";

static const char OpenFileMode[] = "rb";

static const char SaveFileMode[] = "wb";

static const char GameSaveReadMode[] = "rb";

static const char GameSaveHeaderText[] = "TERRAN";

static const char PathDot[] = ".";

static const char MapSaveHeaderText[] = "TERRANMAP";

static const char MapFileFilterPattern[] = "*.";

static const char EmptyMapName[] = "?";

static const char LatestMapPreference[] = "Latest Map";

// The original 0x691B34 pointer cell refers to the separate text at
// 0x691BEC; keep the pointer and its four-byte NUL-terminated pointee distinct.
// RECOVERY_OBJECT: v1 0x00691BEC ?SaveGameFileExtensionText@@3PADA
char SaveGameFileExtensionText[4] = "SAV";
// RECOVERY_OBJECT: v1 0x00691B34 ?SaveGameFileExtension@@3PBDB
LPCSTR SaveGameFileExtension = SaveGameFileExtensionText;
// Native5A9B6C/5ABBD1 load this distinct SC pointer, not the MP map
// extension. The three-byte view claims no neighboring allocation extent.
// RECOVERY_OBJECT: v1 0x00691BF0 ?ScenarioFileExtensionText@@3PADA
char ScenarioFileExtensionText[3] = "SC";
// RECOVERY_OBJECT: v1 0x00691B3C ?ScenarioFileExtension@@3PBDB
LPCSTR ScenarioFileExtension = ScenarioFileExtensionText;
// RECOVERY_OBJECT: v1 0x00690D9C ?GameSaveMultiplayerMarker@@3PADA
char GameSaveMultiplayerMarker[] = "multi";  // native string starts at 0x00690D9C

// These six literals are separate writable source objects because the image
// uses distinct storage starts for the read/write key, value names, and dialog
// label. The registry class strings are one-byte zero-filled views only; no
// larger capacity or independent writer is inferred here.
static char TamperReadApplicationKey[] =
    "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri";
static char TamperWriteApplicationKey[] =
    "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri";
static char TamperReadValueMVM[] = "MVM";
static char TamperReadValueMVB[] = "MVB";
static char TamperWriteValueMVM[] = "MVM";
static char TamperWriteValueMVB[] = "MVB";
static char TamperDialogLabel[] = "HOTNOTAMPER";
// RECOVERY_OBJECT: v1 0x0093A9FC _TamperReadClass
static char TamperReadClass[1];
// RECOVERY_OBJECT: v1 0x0093AA00 _TamperWriteClass
static char TamperWriteClass[1];

// ORIGINAL: 0x0051E110 ?check_tamper@@YAXXZ 0x0051E110-0x0051E38B
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0051e110.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover the full PBEM tamper check using shared typed registry records and cursor, faction state, direct registry APIs and the original unguarded corrupt-cursor/error behavior."}
// symbol    ?check_tamper@@YAXXZ
void __cdecl check_tamper() {
    if (IsMultiplayerPBEM == 0 || TamperState93A9AC == 0) {
        return;
    }

    HKEY key;
    DWORD value_type;
    DWORD byte_count;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, TamperReadApplicationKey, 0,
                        TamperReadClass, 0, 0xF003F, 0, &key, 0) == ERROR_SUCCESS) {
        byte_count = 4;
        RegQueryValueExA(key, TamperReadValueMVM, 0, &value_type,
                         reinterpret_cast<LPBYTE>(&TamperRegistryCursor),
                         &byte_count);
        byte_count = 0x4B0;
        RegQueryValueExA(key, TamperReadValueMVB, 0, &value_type,
                         reinterpret_cast<LPBYTE>(TamperRegistryRecords),
                         &byte_count);
        RegCloseKey(key);
    }

    int matching_index = -1;
    for (int index = 0; index < 50; ++index) {
        TamperRecord &record = TamperRegistryRecords[index];
        if (record.field_0_ == TamperState93A9AC &&
            record.field_4_ == TamperState93A9B0 &&
            record.field_8_ == TurnCurrentNum &&
            record.field_C_ == ConsoleGlobal.faction_id_ &&
            record.field_14_ == TamperState93A9B4) {
            matching_index = index;
            break;
        }
    }

    if (matching_index >= 0) {
        TamperRecord &record = TamperRegistryRecords[matching_index];
        ++record.count_;
        const int current_faction = ConsoleGlobal.faction_id_;
        const uint8_t human_mask = factions_status_byte(0);
        const uint8_t alive_mask = factions_status_byte(1);
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            if (faction != current_faction) {
                const uint8_t bit = static_cast<uint8_t>(1u << (faction & 31));
                if ((human_mask & bit) != 0 && (alive_mask & bit) != 0) {
                    TamperDialogFaction[faction] = current_faction;
                    TamperDialogValue[faction] = record.count_;
                }
            }
        }

        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, TamperWriteApplicationKey, 0,
                            TamperWriteClass, 0, 0xF003F, 0, &key, 0) == ERROR_SUCCESS) {
            RegSetValueExA(key, TamperWriteValueMVM, 0, REG_BINARY,
                           reinterpret_cast<const BYTE *>(&TamperRegistryCursor), 4);
            RegSetValueExA(key, TamperWriteValueMVB, 0, REG_BINARY,
                           reinterpret_cast<const BYTE *>(TamperRegistryRecords), 0x4B0);
            RegCloseKey(key);
        }
        X_pop(TamperDialogLabel, 0);
        return;
    }

    // Native code uses the registry cursor directly as an index before
    // reducing it modulo 50. Keep that corrupt/out-of-range behavior intact.
    const int cursor = TamperRegistryCursor;
    TamperRecord &record = TamperRegistryRecords[cursor];
    record.field_0_ = TamperState93A9AC;
    record.field_4_ = TamperState93A9B0;
    record.field_8_ = TurnCurrentNum;
    record.field_C_ = ConsoleGlobal.faction_id_;
    record.count_ = 0;
    record.field_14_ = TamperState93A9B4;
    const uint32_t advanced_bits = static_cast<uint32_t>(cursor) + 1u;
    const int advanced_cursor = advanced_bits <= 0x7FFFFFFFu
        ? static_cast<int>(advanced_bits)
        : -1 - static_cast<int>(0xFFFFFFFFu - advanced_bits);
    TamperRegistryCursor = advanced_cursor % 50;

    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, TamperWriteApplicationKey, 0,
                        TamperWriteClass, 0, 0xF003F, 0, &key, 0) == ERROR_SUCCESS) {
        RegSetValueExA(key, TamperWriteValueMVM, 0, REG_BINARY,
                       reinterpret_cast<const BYTE *>(&TamperRegistryCursor), 4);
        RegSetValueExA(key, TamperWriteValueMVB, 0, REG_BINARY,
                       reinterpret_cast<const BYTE *>(TamperRegistryRecords), 0x4B0);
        RegCloseKey(key);
    }
}


struct MapDataTransferHeader {
    char opaque_before_format_[0x380];
    int format_version_;
    char opaque_after_format_[0x14];
};

// ORIGINAL: 0x005A6510 sub_5a6510 0x005A6510-0x005A6594 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a6510.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover direction-sign lookup through the genuine nine-entry movement tables and historical C linkage. Preserve the exact seed obligation; new matching claims require native recording."}
// symbol    _sub_5a6510
extern "C" int __cdecl sub_5a6510(int x, int y) {
    int sign_x;
    if (x > 0) {
        sign_x = 1;
    } else {
        sign_x = (x >= 0) - 1;
    }

    int sign_y;
    if (y > 0) {
        sign_y = 1;
    } else {
        sign_y = (y >= 0) - 1;
    }

    for (int index = 0; index < 9; ++index) {
        const int delta_x = VehMoveDeltaX[index];
        int sign_delta_x;
        if (delta_x > 0) {
            sign_delta_x = 1;
        } else {
            sign_delta_x = (delta_x >= 0) - 1;
        }
        if (sign_x == sign_delta_x) {
            const int delta_y = VehMoveDeltaY[index];
            int sign_delta_y;
            if (delta_y > 0) {
                sign_delta_y = 1;
            } else {
                sign_delta_y = (delta_y >= 0) - 1;
            }
            if (sign_y == sign_delta_y) {
                return index;
            }
        }
    }
    return -1;
}

// ORIGINAL: 0x005A6890 ?game_io@@YAHHPAUFILE@@@Z 0x005A6890-0x005A6AC0
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a6890.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover the six ordered MapWin member transfers through standard buffered or encrypted I/O and shared transaction state, not raw self offsets."}
// symbol    ?game_io@@YAHPAVMapWin@@PAU_iobuf@@@Z
// TRIED: divergence at #4 (push edi/esi vs test) is register allocation for a1 not hoisting into a callee-saved reg; a hoisted `base` local and inline `(char*)a1+off` casts compile identically.
int __cdecl game_io(MapWin *record, FILE *file) {
    void *fields[6] = {
        &record->field_1DD78_, &record->field_1DD74_, &record->field_1DD70_,
        &record->field_1DD98_, &record->field_1DD9C_, &record->field_1DDA0_
    };
    for (int index = 0; index != 6; ++index) {
        int result;
        if (GameDataEncryptedMode != 0) {
            result = GameDataWriteMode != 0
                ? encrypt_write(fields[index], 4, 1, file)
                : encrypt_read(fields[index], 4, 1, file);
        } else {
            result = GameDataWriteMode != 0
                ? static_cast<int>(fwrite(fields[index], 4, 1, file))
                : static_cast<int>(fread(fields[index], 4, 1, file));
        }
        if (result == 0) return 1;
    }
    return 0;
}

static int map_data_plain_block(FILE *file, void *buffer, unsigned int size) {
    if (GameDataWriteMode == 0) return fread(buffer, size, 1, file);
    return fwrite(buffer, size, 1, file);
}

static int map_data_block(FILE *file, void *buffer, unsigned int size) {
    if (GameDataEncryptedMode == 0) {
        return map_data_plain_block(file, buffer, size);
    }
    if (GameDataWriteMode == 0) {
        return encrypt_read(buffer, size, 1, file);
    }
    return encrypt_write(buffer, size, 1, file);
}

// ORIGINAL: 0x005A9280 ?map_data@@YAHPAUFILE@@HH@Z 0x005A9280-0x005A94ED
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a9280.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover complete map-data transfer including archive-header blocks, legacy version-dependent block length, failure ordering and real map_read/map_write owners."}
// symbol    ?map_data@@YAHPAU_iobuf@@HH@Z
// TRIED: inline duplication of each read/write block - factored into rw_plain/rw_block helpers instead; DAT_00945d74 is written once (=0) and never again, so the encrypt branch is dead at runtime but still emitted (matches source, not a live check removed); diverges at instr #5
int __cdecl map_data(FILE *file, int write_mode, int transfer_map_header) {
    MapDataTransferHeader header;
    char large_block[0x10660];
    char medium_block[0x2CE0];
    char small_block[0x20];

    GameDataWriteMode = write_mode;
    GameDataEncryptedMode = 0;
    if (transfer_map_header != 0) {
        if (map_data_plain_block(file, &header, sizeof(header)) == 0) return 1;
        if (map_data_block(file, &write_mode, sizeof(write_mode)) == 0) return 1;

        const unsigned int large_size = header.format_version_ < 3
            ? 0xE5E0 : 0x10660;
        if (map_data_block(file, large_block, large_size) == 0) return 1;
        if (map_data_block(file, medium_block, sizeof(medium_block)) == 0) return 1;
        if (map_data_block(file, small_block, sizeof(small_block)) == 0) return 1;
    }

    const int result = GameDataWriteMode == 0 ? map_read(file) : map_write(file);
    return result != 0;
}

// ORIGINAL: 0x005A9C10 ?load_map_daemon@@YAHPAD@Z 0x005A9C10-0x005A9DAD
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a9c10.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover extension/header/version validation, map windows and map tiles, contour copy direction and Paths lifecycle through existing genuine owners."}
// symbol    ?load_map_daemon@@YAHPAD@Z
// TRIED: extension check, env_open, header_check against the TERRAN/ TERRANMAP string depending on the extension, then (fread reuses the `a1` parameter's own stack slot as scratch, matching the disassembly) a MapWin close sweep, map_data, and the counter table reset/copy. Landed at 0.80 similarity; diverges at #3 on register vs stack scheduling for the extension check.
int __cdecl load_map_daemon(char *path) {
    int is_saved_game = 0;
    char *dot = strchr(path, '.');
    // Native691B34 points to SAV. Saved games include the additional
    // extraction blocks; ordinary scenario archives use TERRANMAP.
    if (dot != 0 && _stricmp(dot + 1, SaveGameFileExtension) == 0) {
        is_saved_game = 1;
    }

    FILE *file = env_open(path, OpenFileMode);
    if (file == 0) return 1;

    int result = 2;
    char header[256];
    header_check(header, file);
    const char *expected_header = is_saved_game ? SaveHeaderText : MapHeaderText;
    if (strcmp(header, expected_header) == 0) {
        result = 3;
        int version = 0;
        if (fread(&version, sizeof(version), 1, file) != 0) {
            const int required_version = is_saved_game ? 0x56 : 5;
            if (version >= required_version) {
                for (size_t slot = 1; slot < MapWinTableSlots; ++slot) {
                    MapWin *const window = MapWinTable[slot];
                    if (window != nullptr && window->field_1DD74_ != 0) {
                        window->field_1DD74_ = 0;
                        window->close();
                    }
                }
                ConsoleGlobal.field_23BE4_ = 0;
                result = 4;
                if (map_data(file, 0, is_saved_game) == 0) {
                    ConsoleGlobal.cursor_active_ = 0;
                    ConsoleGlobal.selected_vehicle_ = -1;
                    BaseCurrentCount = 0;
                    VehCurrentCount = 0;
                    // Native5A9D58/5A9D73 use signed JLE/JL tests on the
                    // loaded area word, even though its shared owner is unsigned.
                    for (int map_tile_index = 0;
                         map_tile_index < static_cast<int>(MapArea);
                         ++map_tile_index) {
                        MapTiles[map_tile_index].visibility = 0;
                        MapTiles[map_tile_index].bit &= ~3U;
                    }
                    BaseComputedID = -1;
                    for (int contour_index = 0; contour_index < 11; ++contour_index) {
                        AltNatural[contour_index] = AltContour[contour_index];
                    }
                    result = 0;
                    Paths.init();
                }
            }
        }
    }

    fclose(file);
    return result;
}

// ORIGINAL: 0x005A9B20 ?save_map_daemon@@YAHPAD@Z 0x005A9B20-0x005A9C05
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a9b20.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover map-save header/version/data flow and preserve the original unconditional fclose and failed-file removal error behavior."}
// symbol    ?save_map_daemon@@YAHPAD@Z
int __cdecl save_map_daemon(char *path) {
    char filename[256];
    filename[0] = 0;
    strcat(filename, path);
    if (strchr(filename, '.') == 0) {
        strcat(filename, PathDot);
        strcat(filename, ScenarioFileExtension);
    }

    FILE *file = env_open(filename, SaveFileMode);
    int result = 1;
    if (file != 0) {
        result = 2;
        header_write(MapSaveHeaderText, file);
        int version = 5;
        if (fwrite(&version, sizeof(version), 1, file) != 0) {
            result = 3;
            if (map_data(file, 1, 0) == 0) result = 0;
        }
    }

    fclose(file);
    if (result != 0) remove(filename);
    return result;
}

// ORIGINAL: 0x005A9760 ?load_daemon@@YAHPADH@Z 0x005A9760-0x005A9B19
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a9760.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover the complete load orchestration through actual windows, paths, player/base/vehicle state and PBEM/tamper lifecycle. The called game_data serializer remains explicitly unfinished; this is not working-game acceptance."}
// symbol    ?load_daemon@@YAHPBDH@Z
// TRIED: no Ghidra decompilation was provided for this address; transcribed straight from the disassembly. The two `// indirect` sites are vtable slots 1 (takes an int) and 2 (nullary) on the static object at g_007b0cb8, dispatched through VCall as the brief's shim pattern.
// TRIED: nothing further tried; MISMATCH #0 (push vs sub - a prologue/ stack-layout difference from the local array sizing), similarity 0.73 over 953 bytes. The "copy loop" at 0x5A9A02 is a broadcast of the value at a fixed index into all 0x20 slots of two parallel int arrays (g_009392c0, g_00939340), not a running index as first guessed.
int __cdecl load_daemon(LPCSTR path, int linearize_contours) {
    FILE *file = env_open(path, GameSaveReadMode);
    if (file == 0) return 1;

    char header[0x110];
    header_check(header, file);
    if (strcmp(header, GameSaveHeaderText) != 0) {
        fclose(file);
        return 2;
    }

    int version = 0;
    if (fread(&version, sizeof(version), 1, file) == 0 || version < 0x56) {
        fclose(file);
        return 3;
    }

    for (size_t close_slot = 2; close_slot < MapWinTableSlots; ++close_slot) {
        MapWin *const window = MapWinTable[close_slot];
        if (window != 0 && window->field_1DD74_ != 0) {
            window->field_1DD74_ = 0;
            window->close();
        }
    }
    ConsoleGlobal.field_23BE4_ = 0;
    ConsoleGlobal.field_23BE8_ = 0;

    if (game_data(file, 0) != 0) {
        const int result = GameDataLoadStatusWord == 0 ? 4 : 5;
        fclose(file);
        return result;
    }

    if ((GameRules & RULES_IRONMAN) != 0) {
        int seed = 0;
        if (fread(&seed, sizeof(seed), 1, file) != 0) my_srand(seed);
    }

    if (IsMultiplayerNet == 0) {
        for (size_t offline_slot = 2; offline_slot < MapWinTableSlots; ++offline_slot) {
            MapWin *const window = MapWinTable[offline_slot];
            if (window != 0) window->field_1DD74_ = 0;
        }
    }

    Paths.init();
    if (linearize_contours != 0) world_linearize_contours();
    for (int contour_slot = 0; contour_slot < 11; ++contour_slot) AltNatural[contour_slot] = AltContour[contour_slot];

    for (int tech_faction = 1; tech_faction < MaxPlayerNum; ++tech_faction) {
        PlayerData &data = PlayersData[tech_faction];
        data.flags &= ~static_cast<uint32_t>(PFLAG_UNK_20000);
        for (int satellite_index = 0; satellite_index < 4; ++satellite_index) {
            if (data.satellites_[satellite_index] != 0) data.flags |= PFLAG_UNK_20000;
        }
        for (int technology_id = 0; technology_id < MaxTechnologyNum; ++technology_id) {
            if ((Technology[technology_id].flags & 8U) != 0 &&
                has_tech(technology_id, tech_faction) != 0) {
                data.flags |= PFLAG_UNK_20000;
            }
        }
    }

    for (int alive_faction = 1; alive_faction < MaxPlayerNum; ++alive_faction) {
        bool has_presence = false;
        for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
            if (Bases[base_id].faction_id_current == alive_faction) {
                has_presence = true;
                break;
            }
        }
        if (!has_presence) {
            for (int vehicle_id = 0; vehicle_id < VehCurrentCount; ++vehicle_id) {
                const Veh &vehicle = Vehs[vehicle_id];
                if (vehicle.faction_id == alive_faction &&
                    VehPrototypes[vehicle.proto_id].plan == PLAN_COLONIZATION) {
                    has_presence = true;
                    break;
                }
            }
        }
        const uint32_t faction_bit = 1U << (8 + alive_faction);
        if (has_presence) LockEnableMask |= faction_bit;
        else LockEnableMask &= ~faction_bit;
    }

    if ((GameRules & RULES_IRONMAN) != 0) remove(path);

    const int cursor_history = ConsoleGlobal.cursor_history_index_;
    const int cursor_x = ConsoleGlobal.cursor_history_x_[cursor_history];
    const int cursor_y = ConsoleGlobal.cursor_history_y_[cursor_history];
    for (int history_slot = 0; history_slot < 32; ++history_slot) {
        ConsoleGlobal.cursor_history_x_[history_slot] = cursor_x;
        ConsoleGlobal.cursor_history_y_[history_slot] = cursor_y;
    }

    rebuild_vehicle_bits();
    rebuild_base_bits();

    if (strstr(path, GameSaveMultiplayerMarker) != 0 && IsMultiplayerNet == 0) {
        ConsoleGlobal.cursor_active_ = 0;
        GameState |= 0x10C0;
        const unsigned char state_low_byte = static_cast<unsigned char>(GameState);
        if ((state_low_byte & 0x80) != 0 && (state_low_byte & 0x40) != 0) {
            MainInterfaceGlobal.flat_button_[2].show(0);
        } else {
            MainInterfaceGlobal.flat_button_[2].hide();
        }
    }

    ControlTurnPhase = ConsoleGlobal.faction_id_;
    if (IsMultiplayerNet == 0) {
        if ((NetTurnFlags & 0x20) == 0 && bit_count(LockEnableMask & 0xFFu) <= 1) {
            IsMultiplayerPBEM = 0;
        } else {
            IsMultiplayerPBEM = 1;
            check_tamper();
        }
    } else {
        IsMultiplayerPBEM = 0;
    }
    UnkBitfield1 |= 4;
    fclose(file);
    return 0;
}

// ORIGINAL: 0x005ABB90 ?save_map@@YAXXZ 0x005ABB90-0x005ABD0C;0x0066221F-0x00662234
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005abb90.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover map-save dialog, selection, preference and save orchestration through lexical FileBox lifetime. Preserve both original spans including compiler cleanup; retain the seed."}
// symbol    ?save_map@@YAXXZ
void __cdecl save_map() {
    FileBox box;
    box.set_def_name(MapFilePath);
    box.set_def_ext(ScenarioFileExtension);

    char filter[256];
    filter[0] = 0;
    strcat(filter, MapFileFilterPattern);
    strcat(filter, ScenarioFileExtension);
    box.add_filter(filter, label_get(0x678 / 4));

    char *const selection = box.save();
    do_all_draws();
    if (selection == 0) return;

    char name[256];
    strcpy(name, selection);
    if (strcmp(name, EmptyMapName) == 0) {
        strcpy(name, StringTemp);
    }
    char preference_value[256];
    strcpy(preference_value, name);
    char *dot = strchr(preference_value, '.');
    if (dot != 0) *dot = 0;

    strncpy(MapFilePath, preference_value, 0x4B);
    prefs_put(LatestMapPreference, preference_value);
    save_map_daemon(name);
}

// ORIGINAL: 0x005A6AC0 ?game_data@@YAHPAUFILE@@H@Z 0x005A6AC0-0x005A9279
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a6ac0.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Explicitly unfinished link-continuity body for the serializer called by recovered loaders. BODY GOES HERE remains; this body grants no recovery or runtime coverage and all original seed obligations remain."}
// symbol    ?game_data@@YAHPAU_iobuf@@H@Z
// DEFERRED: confirmed tractable shape, not yet landed - 1983 lines of Ghidra decompilation and 196 conditionals is more than this pass could hand-verify, but the structure is now characterised: Overwhelmingly mechanical: 220 of 256 call sites are __fwrite(60)/__fread(60)/?encrypt_write@@(50)/?encrypt_read@@(50); the other 36 are ?mem_get@@(4)/_memcpy(4)/_free(4)/_abs(4)/ _strcpy(3)/_strlen(2)/?game_io@@(2)/?add@StringStruct@@(2)/ ?map_write@@/?map_read@@/?clear_scenario@@/?clear_monuments@@/ ?clear_bunglist@@/?bit_count@@ (1 each). This is a save/load serializer, field after field. a2 selects the direction: the FIRST call is FUN_0064603f(local_378) when a2!=0 (write: pack current globals into a 0x31C-byte header buffer before writing it) vs FUN_00646178(local_378,0x31C,1,a1) when a2==0 (read: fread that many bytes off `a1` first). Confirmed against the raw disasm's branch on a2 at function entry. The named blockers are ONE gate, not many: `if (local_74 < 0xb)` (local_74 is the version field inside that same 0x31C legacy header) guards roughly 10 raw copy-loops migrating the OLD struct layout into the CURRENT globals (`?9a6490` region) - this is the whole "legacy-struct migration block" and it is READ-ONLY-PATH dead code on any save from version >= 0xb, i.e. every save this binary itself produces. It is real work to transcribe (found 0x2C/0x40/0x16-element copy loops in the first 120 Ghidra lines alone) but it is bounded and separable from the ~220 I/O calls. Frame is 0x374 bytes (from the brief) - count locals against it before landing; a batch of seven diverged only at the prologue over this exact mistake.
int __cdecl game_data(FILE *, int) {
    // BODY GOES HERE.
    return 0;
}

// ORIGINAL: 0x005AB600 ?load_map@@YAHXZ 0x005AB600-0x005ABB8D;0x006620A7-0x0066221F
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005ab600.cpp", "recovery/tasks/game-save-load-owner-coverage.json", "pinned image sha256:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_load_map_behavior.py"], "reason": "Recover map loading through AlphaSave's real selected-path return and existing extension, map-daemon, multiplayer, preference, and dialog owners."}
// TRIED: FileBox/AlphaSave are opaque (no fields) in this scaffold, so their real stack sizes are unknown; used one raw byte buffer + placement-new/reinterpret_cast at the exact ebp-relative offsets from the disasm (incl. the manual vtable pokes before AlphaSave/Scroll/FlatButton close() calls) instead of real SEH locals. Compiles, right call order/addresses/args, sim 0.34-0.50 - the missing SEH frame and giant flat buffer (vs per-object locals) rule out byte-exact; not retried given size (1797 bytes, ~30 embedded objects).
int __cdecl load_map() {
    FileBox file_box_lifetime;
    AlphaSave dialog;
    dialog.init();

    char scenario_filter[16] = ".";
    strcat(scenario_filter, ScenarioFileExtension);
    dialog.add_default_extension(scenario_filter);
    char save_filter[16] = ".";
    strcat(save_filter, SaveGameFileExtension);
    dialog.add_default_extension(save_filter);

    char initial_pattern[260] = "maps\\*.";
    strcat(initial_pattern, ScenarioFileExtension);
    LPSTR preference_name = prefs_get("Latest Map", initial_pattern, 0xFF);
    char default_name[264];
    strncpy(default_name, preference_name, 0xFF);
    if (strlen(default_name) == 0) strcpy(default_name, initial_pattern);
    dialog.set_default_filename(default_name);

    char *selected = dialog.load();
    do_all_draws();
    if (selected == 0) return 1;

    char selected_path[260];
    strcpy(selected_path, selected);
    char preference_value[260];
    strcpy(preference_value, selected);
    char *dot = strchr(preference_value, '.');
    if (dot != 0) *dot = '\0';
    prefs_put("Latest Map", preference_value);

    const int result = load_map_daemon(selected_path);
    if (IsMultiplayerNet != 0) {
        if (result == 0) {
            NetWinDialog.field_7624_[0] = '\0';
            strcat(reinterpret_cast<char *>(NetWinDialog.field_7624_),
                   preference_value);
            NetWinDialog.field_7728_ = 1;
        } else {
            parse_says(0, selected_path, -1, -1);
            parse_num(0, result);
            StringTemp[0] = '\0';
            strcat(StringTemp, "LOAD");
            switch (result) {
            case 1: strcat(StringTemp, "NONE"); break;
            case 2: strcat(StringTemp, "NOT"); break;
            case 3: strcat(StringTemp, "OLD"); break;
            default: strcat(StringTemp, "ERROR"); break;
            }
            X_pop(StringTemp, 0);
            do_all_draws();
        }
    } else if (result != 0) {
        parse_says(0, selected_path, -1, -1);
        parse_num(0, result);
        StringTemp[0] = '\0';
        strcat(StringTemp, "LOAD");
        switch (result) {
        case 1: strcat(StringTemp, "NONE"); break;
        case 2: strcat(StringTemp, "NOT"); break;
        case 3: strcat(StringTemp, "OLD"); break;
        default: strcat(StringTemp, "ERROR"); break;
        }
        X_pop(StringTemp, 0);
        do_all_draws();
    }
    return result;
}
