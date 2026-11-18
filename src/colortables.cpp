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
// The colortables translation unit, 0x00422F20-0x0042439F (image-inferred,
// see the 2026-11-07 TU map): palette comparison and the remap/tint table
// caches. It sits between the BattleWin tail and the CouncWin init thunks.
#include "stdafx.h"
#include <shellapi.h>
#include <ctype.h> // yearmotize preserves the original signed-byte CRT arguments.
#include <stddef.h>
#include <mmsystem.h>
#include "win.h"
#include "xpops.h"  // X_pop
#include "popup.h"  // Existing POP2 owner interface.
#include "ambience.h"  // load_music, the startup faction ambience owner.
#include "game.h"
#include "interface_palette.h"
#include "sound.h"
#include "netmsg.h"
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
#include "prefwin.h"  // ConsolePrefWin.init needs the complete owner declaration.
#include "lock.h"
#include "map.h"
#include "main.h"  // CommandLineText
#include "mapwin.h" // draw_tile, mapwin_system_shutdown
#include "multiwin.h"
#include "log.h"  // log_set_state
#include "random.h"  // random_reseed
#include "battlewin.h"  // the standing battle window
#include "datalink.h"  // DatalinkMain
#include "messagewin.h"  // the standing MessageWin
#include "multidebug.h"  // MultiDebug, the standing debug window
#include "netdaemon.h"  // NetDaemonNet
#include "halloffame.h"
#include "interlude.h"
#include "quaylewin.h"
#include "reportif.h"
#include "statuswin.h"  // StatusWin::close
#include "strings.h"  // StringTable
#include "text.h"  // text_open/text_get/text_item, TextBufferGetPtr
#include "time.h"  // stop_timers
#include "tutwin.h"  // TutWinDesWindow, TutWinBaseWindow, TutWinIfaceWindow
#include "veh.h"
#include "crt_random_inline.h"  // bounded_crt_random
#include "wave.h"  // g_WAVE_GENERAL
#include "worldwin.h"  // WorldClimateWorldWin
#include "diplowin.h"  // DiploWin::exec, homed here out of the archive
#include "diplopop.h"  // DiplomacyPopup
#include "filemap.h"
#include "palette.h"
#include "caviar.h"
#include "vector.h"  // free three-float operations
#include "newtechwin.h"  // Existing technology-popup sprite assets.
#include "infowin.h"  // Shared faction text/shadow colors.
#include "maininterface.h"  // Shared chat foreground/shadow colors.
#include "pushbutton.h"
#include "flatbutton.h"
#include "council.h"
#include "councwin.h"  // game_data transfers the live council window fields
#include "replaywin.h"
#include "technology.h"
#include "path.h"
#include "alphasave.h"
#include "netwin.h"
#include "deletionlist.h"

// Distinct writable string cells observed at the original call sites.
// These role names do not claim original source names or allocation bounds.
// RECOVERY_OBJECT: v1 0x00682ECC _PaletteCompareOpenPath
static char PaletteCompareOpenPath[] = "palcheck.tmp";
// RECOVERY_OBJECT: v1 0x00682EDC _PaletteCompareCreatePath
static char PaletteCompareCreatePath[] = "palcheck.tmp";

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
// LEVER: memcmp/memcpy called through a cast function pointer instead of directly - the scaffold's own memcmp() declaration takes no params (it only needs to be an address, not a callable prototype), so redeclaring it is C2011; casting sidesteps that AND stops VC6 inlining the constant-size memcpy as rep movsd, which the original does not do.
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and measured there - MISMATCH 10/85 against an image of 64, so the homing was reverted and the claim left here. Two independent walls, both from the include context this TU gives the body: (1) `Filemap::close` is the MEASURED in-class definition in filemap.h, so VC6 INLINES it at this site and the body grows 21 instructions of UnmapViewOfFile/CloseHandle chain where the image CALLS 0x006287C0 - the brief's MEASURED-inline ceiling, and the out-of-line forwarder idiom does not fix that one. (2) the four null tests in the body make VC6 keep a zero register: `xor edi,edi` then `cmp esi,edi` against the image's `test esi,esi`, and `mov [ebp-4],edi` against the image's immediate 0. Home it only together with a way to make `close` out-of-line at this site alone.
// This routine calls CRT memcpy in the original. Keep that call boundary
// without cast-call scaffolding; restore the TU's intrinsic policy afterward.
int __cdecl compare_palettes() {
    Filemap file;
    void *cache = file.open(PaletteCompareOpenPath, 1);
    if (cache) {
        if (memcmp(cache, g_PALETTE1.entries(), 256 * sizeof(PALETTEENTRY)) == 0) {
            return 0;
        }
        memcpy(cache, g_PALETTE1.entries(), 256 * sizeof(PALETTEENTRY));
    } else {
        cache = file.create(PaletteCompareCreatePath, 256 * sizeof(PALETTEENTRY), 1);
        if (cache) {
            memcpy(cache, g_PALETTE1.entries(), 256 * sizeof(PALETTEENTRY));
        }
    }
    file.close();
    return 1;
}


// ORIGINAL: 0x00423000 ?colortables_init_faction@@YAXH@Z 0x00423000-0x00423566;0x00652BE2-0x00652C0A
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/recovered/units/00423000.cpp", "recovery/tasks/ambience-faction-art-owner-oct5.json", "pinned complete original spans; fresh native owner compilation and actual-source behavior controls"], "reason": "Complete semantic recovery through genuine source-owned members, globals and lifetime operations; retain original defects and historical obligations. Current byte restoration remains unfinished."}
// symbol    ?colortables_init_faction@@YAXH@Z
// The original calls the CRT memcpy here.
void __cdecl colortables_init_faction(int faction_id) {
    Palette local_palette;
    Filemap cache_file;

    int body_tint_index_by_faction[8] = {96, 105, 156, 0, 85, 47, 89, 227};
    int territory_index_by_faction[8] = {96, 105, 156, 0, 85, 47, 89, 227};
    int weight_by_faction[8] = {80, 80, 80, 80, 80, 80, 80, 80};

    Buffer source_buffer;
    const int palettes_differ = compare_palettes();
    char *filename = Players[faction_id].filename;
    unsigned char *view;
    int refresh_tables;
    DWORD start_time;
    PALETTEENTRY tint;
    HSV hsv;
    int row;
    int color;
    const unsigned char *source_palette;
    PALETTEENTRY shade;

    StringTemp[0] = '\0';
    strcat(StringTemp, filename);
    strcat(StringTemp, ".pcx");
    if (source_buffer.load_pcx(StringTemp, 0, 10, 0xEC) != 0) {
        return;
    }
    territory_index_by_faction[faction_id] = source_buffer.get_pixel(0xA1, 0x2EB);
    {
        const unsigned char weight_sample =
            static_cast<unsigned char>(source_buffer.get_pixel(0xA1, 0x2F5));
        if (weight_sample == WinFillColour) {
            return;
        }
        weight_by_faction[faction_id] =
            static_cast<unsigned char>(weight_sample - 10) * 100 / 255;
    }
    {
        const unsigned char body_sample =
            static_cast<unsigned char>(source_buffer.get_pixel(0x1B3, 0x2E9));
        if (body_sample == WinFillColour) {
            return;
        }
        body_tint_index_by_faction[faction_id] = body_sample;
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, filename);
    strcat(StringTemp, "vals.tmp");
    refresh_tables = 0;
    view = static_cast<unsigned char *>(cache_file.open(StringTemp, 1));
    start_time = timeGetTime();
    // The values-file create retry is a loop back edge; mismatches and the
    // table-create retry below jump to check_values. The table-create retry
    // arrives with StringTemp still naming the output .tmp file.
    for (;;) {
check_values:
        if (view != nullptr && !palettes_differ) {
            if (memcmp(&weight_by_faction[faction_id], view, 4) != 0) {
                view = nullptr;
                goto check_values;
            }
            view += 4;
            if (memcmp(&territory_index_by_faction[faction_id], view, 4) != 0) {
                view = nullptr;
                goto check_values;
            }
            view += 4;
            if (memcmp(&body_tint_index_by_faction[faction_id], view, 4) == 0) {
                goto open_tables;
            }
            view = nullptr;
            goto check_values;
        }
        view = static_cast<unsigned char *>(cache_file.create(StringTemp, 0x80, 1));
        if (view == nullptr) {
            if (timeGetTime() - start_time < 0xBB8) {
                continue;
            }
        } else {
            memcpy(view, &weight_by_faction[faction_id], 4);
            view += 4;
            memcpy(view, &territory_index_by_faction[faction_id], 4);
            view += 4;
            memcpy(view, &body_tint_index_by_faction[faction_id], 4);
            view += 4;
            cache_file.close(view);
        }
        break;
    }
    refresh_tables = 1;
open_tables:
    StringTemp[0] = '\0';
    strcat(StringTemp, filename);
    strcat(StringTemp, ".tmp");
    if (refresh_tables) {
        start_time = timeGetTime();
    } else {
        view = static_cast<unsigned char *>(cache_file.open(StringTemp, 1));
        if (view != nullptr && GetFileSize(cache_file.get_handle(), 0) >= 0xC100) {
            memcpy(TerritoryColorTables[faction_id], view, 0x100);
            view += 0x100;
            memcpy(BodyTintColorTables[faction_id], view, 0x100);
            goto done;
        }
    }
    view = static_cast<unsigned char *>(cache_file.create(StringTemp, 0xC100, 1));
    if (view == nullptr) {
        if (timeGetTime() - start_time < 0xBB8) {
            goto check_values;
        }
        if (view == nullptr) {
            goto done;
        }
    }
    g_PALETTE1.create_table_from_color(
        territory_index_by_faction[faction_id], TerritoryColorTables[faction_id],
        10, 0xEC, 100 - weight_by_faction[faction_id], weight_by_faction[faction_id]);
    memcpy(view, TerritoryColorTables[faction_id], 0x100);
    view += 0x100;
    tint = g_PALETTE1.entries()[body_tint_index_by_faction[faction_id]];
    RGB_to_HSV(&tint, &hsv);
    for (row = 0; row < 24; ++row) {
        source_palette = g_VBP_CAVIARDATA.record()->palette_ + row * 256 + 0x302;
        for (color = 0; color < 256; ++color) {
            shade = g_PALETTE1.entries()[source_palette[color]];
            shade.peRed = static_cast<unsigned char>(static_cast<int>(
                shade.peRed * static_cast<float>(tint.peRed) * (1.0f / 255.0f)));
            shade.peGreen = static_cast<unsigned char>(static_cast<int>(
                shade.peGreen * static_cast<float>(tint.peGreen) * (1.0f / 255.0f)));
            shade.peBlue = static_cast<unsigned char>(static_cast<int>(
                shade.peBlue * static_cast<float>(tint.peBlue) * (1.0f / 255.0f)));
            BodyTintColorTables[faction_id][row][color] = static_cast<unsigned char>(
                g_PALETTE1.get_nearest_palette_index(
                    shade.peRed, shade.peGreen, shade.peBlue, 0));
        }
    }
    memcpy(view, BodyTintColorTables[faction_id], 0xC000);
done:
    cache_file.close();
}


// ORIGINAL: 0x00423570 ?colortables_init@@YAXXZ 0x00423570-0x0042439F;0x00652C0A-0x00652C32
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original primary and EH spans establish operations, local destruction order, standing receiver and mapped cache branches.", "Original bounded row loops and cache sizes establish array extents; model record palette pointer is independently owned storage, not a record-relative address.", "Original paths and dependency-doubled behavioral tests cover cache reuse and invalidation, tint results, parser sequencing and exceptional cleanup."], "reason": "Recover the palette cache and table producer with real Filemap, Buffer and Palette lifetimes, standing palette and model owners, and typed independently owned remap arrays. Historical seeds remain evidence and restoration obligations, not borrowed product claims."}
// TRIED: full instruction-level transcription of the palette-cache load/rebuild state machine (6 Filemap opens, text_open/ text_get resource loads, memcmp cache validation, the create_table_from_color regenerate loops, two RGB_to_HSV tint passes) using goto-labelled control flow matching the original's jump targets and a raw ebp-relative scratch buffer (FRM macro) for the untyped stack locals; no SEH frame reconstruction attempted (MISMATCH #0, prologue only).
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
    char number[80];
    Buffer buffer;
    void *cache;
    unsigned char *out;
    int i;
    int j;
    int regenerate;
    int matched;

    int palette_changed = compare_palettes();
    if (palette_changed) {
make_basic:
        g_PALETTE1.create_table(ShadowColorTable, 0, 256, -43);
        g_PALETTE1.create_table(SpotlightColorTable, 0, 256, 50);
        g_PALETTE1.create_table_from_color(131, ReddishColorTable, 0, 256, 75, 50);
        g_PALETTE1.create_table_from_color(34, RedTintColorTable, 0, 256, 20, 80);
        g_PALETTE1.create_table_from_color(179, BlueTintColorTable, 0, 256, 20, 80);
        for (i = 0; i < 8; ++i) {
            g_PALETTE1.create_table_from_color(255, FadeColorTables[i], 0, 256, 100, 100 - 12 * i);
        }
        if ((cache = file.create("shadow.tmp", 256, 1)) != 0) memcpy(cache, ShadowColorTable, 256);
        if ((cache = file.create("spotlight.tmp", 256, 1)) != 0) memcpy(cache, SpotlightColorTable, 256);
        if ((cache = file.create("reddish.tmp", 256, 1)) != 0) memcpy(cache, ReddishColorTable, 256);
        if ((cache = file.create("redtint.tmp", 256, 1)) != 0) memcpy(cache, RedTintColorTable, 256);
        if ((cache = file.create("bluetint.tmp", 256, 1)) != 0) memcpy(cache, BlueTintColorTable, 256);
        if ((cache = file.create("fade.tmp", 2048, 1)) != 0) memcpy(cache, FadeColorTables, 2048);
    } else {
        if ((cache = file.open("shadow.tmp", 1)) == 0) { palette_changed = 1; goto make_basic; }
        memcpy(ShadowColorTable, cache, 256);
        if ((cache = file.open("spotlight.tmp", 1)) == 0) { palette_changed = 1; goto make_basic; }
        memcpy(SpotlightColorTable, cache, 256);
        if ((cache = file.open("reddish.tmp", 1)) == 0) { palette_changed = 1; goto make_basic; }
        memcpy(ReddishColorTable, cache, 256);
        if ((cache = file.open("redtint.tmp", 1)) == 0) { palette_changed = 1; goto make_basic; }
        memcpy(RedTintColorTable, cache, 256);
        if ((cache = file.open("bluetint.tmp", 1)) == 0) { palette_changed = 1; goto make_basic; }
        memcpy(BlueTintColorTable, cache, 256);
        if ((cache = file.open("fade.tmp", 1)) == 0) { palette_changed = 1; goto make_basic; }
        memcpy(FadeColorTables, cache, 2048);
    }

    if (text_open("system", "energyglow") == 0) {
        for (i = 0; i < 3; ++i) {
            text_get();
            energy_colors[i] = text_item_number();
            energy_weights[i] = text_item_number();
        }
    }
    for (i = 0; i < 13; ++i) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "explosions");
        _itoa(i, number, 10);
        strcat(StringTemp, number);
        if (text_open("system", StringTemp) == 0) {
            for (j = 0; j < 16; ++j) {
                text_get();
                explosion_colors[i][j] = text_item_number();
                explosion_weights[i][j] = text_item_number();
            }
        }
    }
    if (text_open("system", "VEHICLECOLORING") == 0) {
        for (i = 0; i < 8; ++i) {
            text_get();
            // Original adds ten to AL, not a potentially overflowing int.
            body_colors[i] = static_cast<unsigned char>(text_item_number() + 10);
        }
    }
    if (text_open("system", "ARMORCOLORING") == 0) {
        for (i = 0; i < 4; ++i) {
            text_get();
            armor_colors[i] = static_cast<unsigned char>(text_item_number() + 10);
        }
    }
    if (text_open("system", "ARMORCOLORING_TRANS") == 0) {
        text_get();
        transparent_color = static_cast<unsigned char>(text_item_number() + 10);
        transparent_first = text_item_number();
        transparent_second = text_item_number();
    }
    if (text_open("system", "boundarypercents") == 0) {
        for (i = 0; i < 8; ++i) {
            text_get();
            territory_weights[i] = text_item_number();
        }
    }
    buffer.load_pcx("palette.PCX", 0, 10, 236);
    for (i = 0, j = 580; i < 8; ++i) {
        territory_colors[i] = buffer.get_pixel(j, 56);
        j += 8;
    }

    regenerate = 0;
    out = static_cast<unsigned char *>(file.open("colorvals.tmp", 1));
    matched = 0;
    if (out != 0 && !palette_changed &&
        !memcmp(energy_colors, out, sizeof(energy_colors))) {
        out += sizeof(energy_colors);
        if (!memcmp(energy_weights, out, sizeof(energy_weights))) {
            out += sizeof(energy_weights);
            for (i = 0; i < 13; ++i) {
                if (memcmp(explosion_colors[i], out, sizeof(explosion_colors[i]))) goto compared;
                out += sizeof(explosion_colors[i]);
                if (memcmp(explosion_weights[i], out, sizeof(explosion_weights[i]))) goto compared;
                out += sizeof(explosion_weights[i]);
            }
            if (memcmp(body_colors, out, sizeof(body_colors))) goto compared;
            out += sizeof(body_colors);
            if (memcmp(territory_weights, out, sizeof(territory_weights))) goto compared;
            out += sizeof(territory_weights);
            if (memcmp(territory_colors, out, sizeof(territory_colors))) goto compared;
            out += sizeof(territory_colors);
            if (memcmp(armor_colors, out, sizeof(armor_colors))) goto compared;
            out += sizeof(armor_colors);
            if (memcmp(&transparent_color, out, sizeof(transparent_color))) goto compared;
            out += sizeof(transparent_color);
            if (!memcmp(&transparent_first, out, sizeof(transparent_first))) matched = 1;
        }
    }
compared:
    if (!matched) {
    out = static_cast<unsigned char *>(file.create("colorvals.tmp", 2048, 1));
    if (out) {
        memcpy(out, energy_colors, sizeof(energy_colors)); out += sizeof(energy_colors);
        memcpy(out, energy_weights, sizeof(energy_weights)); out += sizeof(energy_weights);
        for (i = 0; i < 13; ++i) {
            memcpy(out, explosion_colors[i], sizeof(explosion_colors[i])); out += sizeof(explosion_colors[i]);
            memcpy(out, explosion_weights[i], sizeof(explosion_weights[i])); out += sizeof(explosion_weights[i]);
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
make_explosions:
    regenerate = 1;
    for (i = 0; i < 13; ++i) {
        for (j = 0; j < 16; ++j) {
            g_PALETTE1.create_table_from_color(explosion_colors[i][j] + 10,
                ExplosionColorTables[i][j], 10, 236, 100 - explosion_weights[i][j],
                explosion_weights[i][j]);
        }
        char name[80];
        StringTemp[0] = '\0';
        strcat(StringTemp, "explosions");
        _itoa(i, name, 10);
        strcat(StringTemp, name);
        strcat(StringTemp, ".tmp");
        cache = file.create(StringTemp, sizeof(ExplosionColorTables[i]), 1);
        memcpy(cache, ExplosionColorTables[i], sizeof(ExplosionColorTables[i]));
        file.close();
    }
    } else {
    i = 0;
    while (1) {
        if (i >= 13) break;
        StringTemp[0] = '\0';
        strcat(StringTemp, "explosions");
        _itoa(i, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ".tmp");
        if ((cache = file.open(StringTemp, 1)) == 0) goto make_explosions;
        memcpy(ExplosionColorTables[i], cache, sizeof(ExplosionColorTables[i]));
        file.close();
        ++i;
    }
    }

    // Rebuilt explosions force a territory rebuild even if that file exists.
    // A missing territory file alone does not invalidate body/armor tints.
    if (regenerate) {
make_territory:
        out = static_cast<unsigned char *>(file.create("territory.tmp", sizeof(TerritoryColorTables), 1));
        for (i = 0; i < 8; ++i) {
            g_PALETTE1.create_table_from_color(territory_colors[i], TerritoryColorTables[i],
                10, 236, 100 - territory_weights[i], territory_weights[i]);
            memcpy(out, TerritoryColorTables[i], sizeof(TerritoryColorTables[i]));
            out += sizeof(TerritoryColorTables[i]);
        }
        file.close();
    } else {
        if ((cache = file.open("territory.tmp", 1)) == 0) goto make_territory;
        memcpy(TerritoryColorTables, cache, sizeof(TerritoryColorTables));
        file.close();
    }

    if (regenerate) {
        for (i = 0; i < 8; ++i) {
            PALETTEENTRY tint = g_PALETTE1.entries()[body_colors[i]];
            HSV hsv;
            RGB_to_HSV(&tint, &hsv);
            for (int row = 0; row < 24; ++row) {
                const unsigned char *source = g_VBP_CAVIARDATA.record()->palette_ + 0x302 + row * 256;
                for (int color = 0; color < 256; ++color) {
                    PALETTEENTRY entry = g_PALETTE1.entries()[source[color]];
                    entry.peRed = (unsigned char)(int)((float)entry.peRed * tint.peRed * (1.0f / 255.0f));
                    entry.peGreen = (unsigned char)(int)((float)entry.peGreen * tint.peGreen * (1.0f / 255.0f));
                    entry.peBlue = (unsigned char)(int)((float)entry.peBlue * tint.peBlue * (1.0f / 255.0f));
                    BodyTintColorTables[i][row][color] = (unsigned char)g_PALETTE1.get_nearest_palette_index(entry.peRed, entry.peGreen, entry.peBlue, 0);
                }
            }
        }
        cache = file.create("bodygrads.tmp", sizeof(BodyTintColorTables), 1);
        memcpy(cache, BodyTintColorTables, sizeof(BodyTintColorTables));
        file.close();
    } else {
        cache = file.open("bodygrads.tmp", 1);
        memcpy(BodyTintColorTables, cache, sizeof(BodyTintColorTables));
        file.close();
    }

    if (regenerate) {
make_armor:
        for (i = 0; i < 4; ++i) {
            PALETTEENTRY tint = g_PALETTE1.entries()[armor_colors[i]];
            HSV hsv;
            RGB_to_HSV(&tint, &hsv);
            for (int row = 0; row < 24; ++row) {
                const unsigned char *source = g_VBP_CAVIARDATA.record()->palette_ + 0x302 + row * 256;
                for (int color = 0; color < 256; ++color) {
                    PALETTEENTRY entry = g_PALETTE1.entries()[source[color]];
                    entry.peRed = (unsigned char)(int)((float)entry.peRed * tint.peRed * (1.0f / 255.0f));
                    entry.peGreen = (unsigned char)(int)((float)entry.peGreen * tint.peGreen * (1.0f / 255.0f));
                    entry.peBlue = (unsigned char)(int)((float)entry.peBlue * tint.peBlue * (1.0f / 255.0f));
                    ArmorTintColorTables[i][row][color] = (unsigned char)g_PALETTE1.get_nearest_palette_index(entry.peRed, entry.peGreen, entry.peBlue, 0);
                }
            }
        }
        cache = file.create("armorgrads.tmp", sizeof(ArmorTintColorTables), 1);
        memcpy(cache, ArmorTintColorTables, sizeof(ArmorTintColorTables));
    } else {
        if ((cache = file.open("armorgrads.tmp", 1)) == 0) { regenerate = 1; goto make_armor; }
        memcpy(ArmorTintColorTables, cache, sizeof(ArmorTintColorTables));
    }
    file.close();

    for (i = 0; i < 24; ++i) {
        for (int entry = 0; entry < 256; ++entry) {
            ConstantColorTables[i][entry] = static_cast<unsigned char>(i);
        }
    }

    if (regenerate) {
make_transparent:
        for (i = 0; i < 24; ++i) {
            const int weight = ((24 - i) * transparent_first + i * transparent_second) / 24;
            g_PALETTE1.create_table_from_color(transparent_color, ArmorTransparentColorTables[i], 10, 236, 100 - weight, weight);
        }
        cache = file.create("armorgrads_trans.tmp", sizeof(ArmorTransparentColorTables), 1);
        memcpy(cache, ArmorTransparentColorTables, sizeof(ArmorTransparentColorTables));
    } else {
        if ((cache = file.open("armorgrads_trans.tmp", 1)) == 0) goto make_transparent;
        memcpy(ArmorTransparentColorTables, cache, sizeof(ArmorTransparentColorTables));
    }
    file.close();
}

