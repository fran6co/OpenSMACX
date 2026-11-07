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
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
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
#pragma function(memcpy)
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
    unsigned char *body_table;

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
    // The native retry edge from the table-create loop below returns here
    // with StringTemp still naming the output .tmp file.
    view = static_cast<unsigned char *>(cache_file.create(StringTemp, 0x80, 1));
    if (view == nullptr) {
        if (timeGetTime() - start_time < 0xBB8) {
            goto check_values;
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
    body_table = &BodyTintColorTables[faction_id][0][0];
    {
        for (int row = 0; row < 0x1800; row += 0x100) {
            const unsigned char *source_palette =
                g_VBP_CAVIARDATA.record()->palette_ + row + 0x302;
            for (int color = 0; color < 256; ++color) {
                PALETTEENTRY shade = g_PALETTE1.entries()[source_palette[color]];
                shade.peRed = static_cast<unsigned char>(static_cast<int>(
                    shade.peRed * static_cast<float>(tint.peRed) * (1.0f / 255.0f)));
                shade.peGreen = static_cast<unsigned char>(static_cast<int>(
                    shade.peGreen * static_cast<float>(tint.peGreen) * (1.0f / 255.0f)));
                shade.peBlue = static_cast<unsigned char>(static_cast<int>(
                    shade.peBlue * static_cast<float>(tint.peBlue) * (1.0f / 255.0f)));
                body_table[row + color] = static_cast<unsigned char>(
                    g_PALETTE1.get_nearest_palette_index(
                        shade.peRed, shade.peGreen, shade.peBlue, 0));
            }
        }
    }
    memcpy(view, body_table, 0xC000);
done:
    cache_file.close();
}
#pragma intrinsic(memcpy)


// ORIGINAL: 0x00423570 ?colortables_init@@YAXXZ 0x00423570-0x0042439F;0x00652C0A-0x00652C32
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original primary and EH spans establish operations, local destruction order, standing receiver and mapped cache branches.", "Original bounded row loops and cache sizes establish array extents; model record palette pointer is independently owned storage, not a record-relative address.", "Original paths and dependency-doubled behavioral tests cover cache reuse and invalidation, tint results, parser sequencing and exceptional cleanup."], "reason": "Recover the palette cache and table producer with real Filemap, Buffer and Palette lifetimes, standing palette and model owners, and typed independently owned remap arrays. Historical seeds remain evidence and restoration obligations, not borrowed product claims."}
// TRIED: full instruction-level transcription of the palette-cache load/rebuild state machine (6 Filemap opens, text_open/ text_get resource loads, memcmp cache validation, the create_table_from_color regenerate loops, two RGB_to_HSV tint passes) using goto-labelled control flow matching the original's jump targets and a raw ebp-relative scratch buffer (FRM macro) for the untyped stack locals; no SEH frame reconstruction attempted (MISMATCH #0, prologue only).
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
    void *basic_tables[6] = {ShadowColorTable, SpotlightColorTable,
        ReddishColorTable, RedTintColorTable, BlueTintColorTable,
        FadeColorTables};
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

