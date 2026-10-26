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
#include "sprite.h"
#include "buffer.h"
#include "texture.h"
#include "strings.h" // source-owned shared filename buffer
#include <stdlib.h>

// Application-owned assets loaded here; storage lives in sprite_objects.cpp.
extern Sprite g_TECH_ICON_SPRITES[89];
extern Sprite g_BATTLE_MIND_WORM_SPRITE;
extern Sprite g_BATTLE_ISLE_DEEP_SPRITE;
extern Sprite g_BATTLE_LOCUSTS_CHIRON_SPRITE;
extern Sprite g_BATTLE_FUNGAL_TOWER_SPRITE;
extern Sprite g_BATTLE_SPORE_LAUNCHER_SPRITE;
extern Sprite g_BATTLE_SEALURK_SPRITE;

// The icon and cursor loaders reference distinct original NEWICONS.PCX
// storage. Keep this loader beside its assets, separate from game.cpp
// literal pooling; this does not assert a historical TU boundary.
// Populate the existing resource/control icon objects from three shared
// sheets. Startup through load_sprites remains a separate recovery dependency.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004547E0 ?load_icons@@YAXXZ 0x004547E0-0x00454D0C
// RECOVERY_BINDING: v1 {"evidence": ["Full original4547E0..454D0C and retained seed src/recovered/units/004547e0.cpp establish the single no-argument loader, three sheet loads, recolor pairs before every extraction, all receivers/rectangles and unconditional terminal close.", "All service results are ignored. Original454B04 resets the arrow-column coordinate to104 before filler extraction. Menu and checkbox receivers belong to existing arrays, including checkbox element1 rather than separate75AF94 storage.", "Original per-asset CRT constructors/teardowns independently establish element counts and44-byte stride. Existing load_sprites startup and raw caption-icon publisher are separate unfinished ownership dependencies, not proof supplied by this binding."], "reason": "Complete original icon-sheet loader uses the existing shared PCX Buffer and thirteen existing Sprite groups. Recover ordinary C++ object/array accesses without original-address aliases or replacement assets."}
void __cdecl load_icons() {
    Buffer &buffer = g_PCX_PARSE_TEMP_BUFFER1;
    Sprite *icon;
    int source_x;
    int source_y;
    int columns_left;

    // Pinned literals: 685064 NEWICONS.PCX; 685074 TER1WRECK.PCX;
    // 685084 ICONS.PCX. All loads use palette 0, base 10, count 236.
    // There is no DirectDraw guard here, unlike load_cursors.
    // RECOVERY_LITERAL: v1 0x00685064 0x00685071 ??_C@_0N@NBMC@NEWICONS?4PCX?$AA@ 0x004547E0
    buffer.load_pcx("NEWICONS.PCX", 0, 10, 236);
    icon = g_ICONS_GENERAL_SPRITES;
    source_y = 13;
    do {
        source_x = 1;
        columns_left = 4;
        do {
            buffer.change_color(263, 265);
            buffer.change_color(262, 265);
            icon->extract(&buffer, 265, source_x, source_y, 22, 22, 0);
            source_x += 23;
            ++icon;
        } while (--columns_left);
        source_y += 23;
    } while (icon < g_ICONS_GENERAL_SPRITES + 16);

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_NULL_RESOURCE_ICON_SPRITE.extract(&buffer, 265, 1, 174, 22, 22, 0);

    icon = g_PEACE_SIGN_SPRITES;
    for (source_y = 220; icon < g_PEACE_SIGN_SPRITES + 2; source_y += 23) {
        buffer.change_color(263, 265);
        buffer.change_color(262, 265);
        icon->extract(&buffer, 265, 1, source_y, 22, 22, 0);
        ++icon;
    }

    icon = g_RESOURCE_ICON_SPRITES;
    source_y = 304;
    do {
        source_x = 174;
        columns_left = 8;
        do {
            buffer.change_color(263, 265);
            buffer.change_color(262, 265);
            icon->extract(&buffer, 265, source_x, source_y, 40, 40, 0);
            source_x += 41;
            ++icon;
        } while (--columns_left);
        source_y += 41;
    } while (icon < g_RESOURCE_ICON_SPRITES + 32);

    // RECOVERY_LITERAL: v1 0x00685074 0x00685082 ??_C@_0O@CAND@TER1WRECK?4PCX?$AA@ 0x004547E0
    buffer.load_pcx("TER1WRECK.PCX", 0, 10, 236);
    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_RED_ALIEN_HEAD_ICON_SPRITE.extract(&buffer, 265, 768, 148, 28, 42, 0);

    // RECOVERY_LITERAL: v1 0x00685084 0x0068508E ??_C@_09OIPD@ICONS?4PCX?$AA@ 0x004547E0
    buffer.load_pcx("ICONS.PCX", 0, 10, 236);
    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_ICON_TILE_SQUARE_SPRITE.extract(&buffer, 265, 1, 1, 100, 50, 0);

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_RED_MALE_HEAD_ICON_SPRITE.extract(&buffer, 265, 46, 303, 33, 30, 0);

    icon = g_SCROLL_BAR_ARROW_ICON_SPRITES;
    source_y = 1;
    do {
        source_x = 104;
        columns_left = 3;
        do {
            buffer.change_color(263, 265);
            buffer.change_color(262, 265);
            icon->extract(&buffer, 265, source_x, source_y, 24, 24, 0);
            source_x += 25;
            ++icon;
        } while (--columns_left);
        source_y += 25;
    } while (icon < g_SCROLL_BAR_ARROW_ICON_SPRITES + 12);

    // Original EDI is reset to 104 after the final arrow row; the filler
    // source X is not the previous row's final icon or its one-past position.
    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_SCROLL_BAR_FILLER_ICON_SPRITES.extract(&buffer, 265, 104, 101, 18, 18, 0);

    icon = g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES;
    source_y = 1;
    do {
        source_x = 192;
        columns_left = 3;
        do {
            buffer.change_color(263, 265);
            buffer.change_color(262, 265);
            icon->extract(&buffer, 265, source_x, source_y, 10, 10, 0);
            source_x += 11;
            ++icon;
        } while (--columns_left);
        source_y += 11;
    } while (icon < g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES + 12);

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE.extract(&buffer, 265, 225, 1, 10, 10, 0);

    icon = g_SILVER_MENU_ICON_SPRITES;
    for (source_x = 320; icon < g_SILVER_MENU_ICON_SPRITES + 4; source_x += 16) {
        buffer.change_color(263, 265);
        buffer.change_color(262, 265);
        icon->extract(&buffer, 265, source_x, 1, 15, 15, 0);
        ++icon;
    }

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_SILVER_CHECKBOX_ICON_SPRITES[0].extract(&buffer, 265, 320, 20, 16, 16, 0);

    buffer.change_color(263, 265);
    buffer.change_color(262, 265);
    g_SILVER_CHECKBOX_ICON_SPRITES[1].extract(&buffer, 265, 337, 20, 16, 16, 0);

    // Every service result above is deliberately ignored, including failed
    // loads and extractions. Close occurs only after all 86 extraction attempts.
    buffer.close();
}
// ORIGINAL: 0x00450DE0 sub_450de0 0x00450DE0-0x00450E2D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/00450de0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450de0 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
// symbol    _sub_450de0
extern "C" int __cdecl sub_450de0(Sprite *sprite, int x, int y) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    return sprite->extract(buffer, 0x109, x, y, 100, 50, 0);
}

// ORIGINAL: 0x00450E30 sub_450e30 0x00450E30-0x00450E7D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/00450e30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450e30 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
// symbol    _sub_450e30
extern "C" int __cdecl sub_450e30(Sprite *sprite, int x, int y) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    return sprite->extract(buffer, 0x109, x, y, 0x64, 0x3e, 0);
}

// ORIGINAL: 0x00450E80 sub_450e80 0x00450E80-0x00450ECD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/00450e80.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450e80 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
// symbol    _sub_450e80
extern "C" int __cdecl sub_450e80(Sprite *sprite, int x, int y) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    return sprite->extract(buffer, 0x109, x, y, 0x64, 0x4b, 0);
}

// ORIGINAL: 0x00450ED0 sub_450ed0 0x00450ED0-0x00450F21 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/units/00450ed0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450ed0 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
// symbol    _sub_450ed0
extern "C" int __cdecl sub_450ed0(Sprite *sprite, int x, int y,
                       int width, int height) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    return sprite->extract(buffer, 0x109, x, y, width, height, 0);
}

// ORIGINAL: 0x00450F30 sub_450f30 0x00450F30-0x00450F81 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/units/00450f30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450f30 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
// symbol    _sub_450f30
extern "C" void __cdecl sub_450f30(Sprite *sprite, int x, int y,
                        int width, int height) {
    Buffer *buffer = &g_PCX_PARSE_TEMP_BUFFER1;
    buffer->change_color(0x107, 0x109);
    buffer->change_color(0x106, 0x109);
    sprite->extract(buffer, 0x109, x, y, width, height, 0);
}

// ORIGINAL: 0x00450F90 sub_450f90 0x00450F90-0x00450FB0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f2df6224d8701da7e923701c56365402e6901cf:src/recovered/00450f90.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x450f90 complete native body and call ordering.", "source:src/buffer.cpp:g_PCX_PARSE_TEMP_BUFFER1 binds original storage 0x00798668; source:src/graphicwin.h:public Buffer secondary base supplies the original +0x444 draw receiver adjustment."], "reason": "Sprite/Texture extraction helper using the real shared PCX scratch Buffer; preserve palette remap order, native dimensions and result contract."}
// symbol    _sub_450f90
extern "C" int __cdecl sub_450f90(Texture *texture, int x, int y) {
    return texture->extract(&g_PCX_PARSE_TEMP_BUFFER1, x, y, 0x38, 0x38, 0);
}
// ORIGINAL: 0x00454D10 ?load_misc@@YAXXZ 0x00454D10-0x00454FB6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/00454d10.cpp", "task:asset-startup-loader-owner", "Native229 init receivers:89 tech,70 facility,64 project and6 battle Sprites;44-byte element stride; facility34..64 x prefix and >64 index-31; native files/dimensions/order. Existing Sprite storage is constructed in this owner, not original-address aliases."], "reason": "Recover the complete asset loader through source-owned C++ objects, arrays and normal lifecycle while retaining original mappings and historical findings; no asset/runtime acceptance claimed."}
void __cdecl load_misc() {
    int index;
    char number[80];
    char second_number[80];

    for (index = 0; index < 89; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "techs\\tech");
        if (index < 10) {
            strcat(StringTemp, "0");
        }
        _itoa(index, number, 10);
        strcat(StringTemp, number);
        g_TECH_ICON_SPRITES[index].init(StringTemp, 0, 0);
        g_TECH_ICON_SPRITES[index].iWidth_ = 0xD7;
        g_TECH_ICON_SPRITES[index].iHeight_ = 0xD7;
    }

    for (index = 0; index < 70; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "facs\\");
        if (index >= 34 && index <= 64) {
            strcat(StringTemp, "x");
        }
        strcat(StringTemp, "fac0");
        if (index < 10) {
            strcat(StringTemp, "0");
        }
        if (index > 64) {
            _itoa(index - 0x1F, number, 10);
            strcat(StringTemp, number);
        } else {
            _itoa(index, second_number, 10);
            strcat(StringTemp, second_number);
        }
        g_FACILITY_ICON_SPRITES[index].init(StringTemp, 0, 0);
        g_FACILITY_ICON_SPRITES[index].iWidth_ = 0xD7;
        g_FACILITY_ICON_SPRITES[index].iHeight_ = 0xD7;
    }

    for (index = 0; index < 64; ++index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "projs\\proj0");
        if (index < 10) {
            strcat(StringTemp, "0");
        }
        _itoa(index, second_number, 10);
        strcat(StringTemp, second_number);
        g_SECRET_PROJECT_ICON_SPRITES[index].init(StringTemp, 0, 0);
        g_SECRET_PROJECT_ICON_SPRITES[index].iWidth_ = 0xD7;
        g_SECRET_PROJECT_ICON_SPRITES[index].iHeight_ = 0xD7;
    }

    g_BATTLE_MIND_WORM_SPRITE.init("btwrm.pcx", 0, 0);
    g_BATTLE_MIND_WORM_SPRITE.iWidth_ = 0x60;
    g_BATTLE_MIND_WORM_SPRITE.iHeight_ = 0x73;
    g_BATTLE_ISLE_DEEP_SPRITE.init("btisl.pcx", 0, 0);
    g_BATTLE_ISLE_DEEP_SPRITE.iWidth_ = 0x60;
    g_BATTLE_ISLE_DEEP_SPRITE.iHeight_ = 0x73;
    g_BATTLE_LOCUSTS_CHIRON_SPRITE.init("btchi.pcx", 0, 0);
    g_BATTLE_LOCUSTS_CHIRON_SPRITE.iWidth_ = 0x60;
    g_BATTLE_LOCUSTS_CHIRON_SPRITE.iHeight_ = 0x73;
    g_BATTLE_FUNGAL_TOWER_SPRITE.init("BTfungtow.pcx", 0, 0);
    g_BATTLE_FUNGAL_TOWER_SPRITE.iWidth_ = 0x60;
    g_BATTLE_FUNGAL_TOWER_SPRITE.iHeight_ = 0x73;
    g_BATTLE_SPORE_LAUNCHER_SPRITE.init("BTsporlnch.pcx", 0, 0);
    g_BATTLE_SPORE_LAUNCHER_SPRITE.iWidth_ = 0x60;
    g_BATTLE_SPORE_LAUNCHER_SPRITE.iHeight_ = 0x73;
    g_BATTLE_SEALURK_SPRITE.init("BTsealrk.pcx", 0, 0);
    g_BATTLE_SEALURK_SPRITE.iWidth_ = 0x60;
    g_BATTLE_SEALURK_SPRITE.iHeight_ = 0x73;
}
