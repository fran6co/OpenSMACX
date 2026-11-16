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
// ORIGINAL: 0x00559230 _$E21 0x00559230-0x00559246 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x0068263C selects initializer0x00559230-0x00559246 and its registration names fini0x00559250-0x0055925A; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FB88; FontQueue constructor/destructor559290/55B740. Source object binding names ?g_FONTQUEUE_VAL2@@3VFontQueue@@A.", "Fresh owner COFF selects _$E21 via .CRT$XCU and registers _$E19, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original startup function to the genuine compiler-generated lifetime of source-owned g_FONTQUEUE_VAL2; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E12
// ORIGINAL: 0x00559250 _$E19 0x00559250-0x0055925A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x0068263C selects initializer0x00559230-0x00559246 and its registration names fini0x00559250-0x0055925A; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FB88; FontQueue constructor/destructor559290/55B740. Source object binding names ?g_FONTQUEUE_VAL2@@3VFontQueue@@A.", "Fresh owner COFF selects _$E21 via .CRT$XCU and registers _$E19, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original exit function to the genuine compiler-generated lifetime of source-owned g_FONTQUEUE_VAL2; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E10
// ORIGINAL: 0x00559260 _$E26 0x00559260-0x00559276 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682640 selects initializer0x00559260-0x00559276 and its registration names fini0x00559280-0x0055928A; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FAE8; FontQueue constructor/destructor559290/55B740. Source object binding names ?g_FONTQUEUE_VAL1@@3VFontQueue@@A.", "Fresh owner COFF selects _$E26 via .CRT$XCU and registers _$E24, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original startup function to the genuine compiler-generated lifetime of source-owned g_FONTQUEUE_VAL1; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E17
// ORIGINAL: 0x00559280 _$E24 0x00559280-0x0055928A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682640 selects initializer0x00559260-0x00559276 and its registration names fini0x00559280-0x0055928A; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FAE8; FontQueue constructor/destructor559290/55B740. Source object binding names ?g_FONTQUEUE_VAL1@@3VFontQueue@@A.", "Fresh owner COFF selects _$E26 via .CRT$XCU and registers _$E24, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original exit function to the genuine compiler-generated lifetime of source-owned g_FONTQUEUE_VAL1; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E15
// WORKAROUND(decl): see vector.h; veh_scoot calls 0x006343C0 out of line at 0x0055A1BB.
#define VECTOR_CTOR_OUT_OF_LINE
#include "stdafx.h"
#include "interface_fonts.h"
#include "font_queue.h"
#include "mapwin.h"
#include "general.h"
#include "veh.h"
#include "base.h"
#include "map.h"
#include "faction.h"
#include "game.h"
#include "infowin.h"
#include "strings.h"
#include "vector.h"
#include "wave.h"
#include "worldwin.h"
#include "alpha.h"
#include "rectangle_operations.h"

// The per-element teardown the queue passes: the Font destructor, bound here
// under its own name so this file needs nothing from the retired thunk
// family; the old atexit_thunks.cpp binding covered the same address.
// Its construction-side companion, likewise bound locally rather than
// pulling in a shared header; init_thunks.cpp used to bind the same
// address as FontElementCtor (both retired with the lifecycle batch).

/*
Purpose: Construct the queue: hand the three-slot walk to the CRT vector
         iterator with the Font constructor/destructor pair, then seed the
         three parallel per-slot arrays that follow the Font storage.
// ORIGINAL: 0x00559290 ??0FontQueue@@QAE@XZ 0x00559290-0x005592CD BYTE_EXACT
// size      61 bytes
// prototype void (__thiscall ??0FontQueue@@QAE@XZ)(FontQueue* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006457C2
Return Value: n/a
Status: Complete
*/
FontQueue::FontQueue() {
    for (int i = 0; i < 3; i++) {
        heights_[i] = -999;
        styles_[i] = 0;
        lru_slots_[i] = i;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: `int foundPos=-1` sentinel search vs a `while` loop with an early `goto`-style break matching the disassembly's `jl` dead branch (SF!=OF on an always-nonnegative counter) - landed with the sentinel form, MISMATCH at #2 already (loop-entry `cmp` vs the original's cached `mov`).
// ORIGINAL: 0x005592D0 ?get_zoom@FontQueue@@QAEHHHHH@Z 0x005592D0-0x005593D9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5592D0..5593D9 computes signed scale and height in two division stages, substitutes11 for10, and reads the shared691E6C/70 override only when its numerator is positive. The constructor559290 establishes three Font objects of stride28 followed by height/style/order arrays at78/84/90.", "Original559334..559374 rotates the oldest slot, publishes keys before calling actual Font::init618F40 with the source-owned DefaultFontFace, ignores the status result and returns the same Font pointer. Original559382..5593D9 promotes a cache hit without initialization; missing order entries leave the order unchanged. Retain the unclaimed seed and catalogue signature as historical evidence."], "reason": "Recover the three-slot font-height/style LRU as the existing FontQueue owner, returning its actual Font member rather than a synthetic integer address."}
// symbol    ?get_zoom@FontQueue@@QAEPAVFont@@HHHH@Z
#pragma optimize("y", off)
Font *FontQueue::get_zoom(int zoom, int numerator, int denominator, int style) {
    int scale;
    if (MapWinCaptionScale > 0) {
        scale = (MapWinCaptionScale * 50) / MapWinCaptionDivisor;
    } else {
        scale = ((zoom * 5 + 80) * 10) / 16;
    }
    int height = (scale * numerator) / denominator;
    if (height == 10) {
        height = 11;
    }

    for (int slot = 0; slot < 3; ++slot) {
        if (heights_[slot] == height && styles_[slot] == style) {
            int found = -1;
            for (int position = 0; position < 3; ++position) {
                if (lru_slots_[position] == slot) {
                    found = position;
                    break;
                }
            }
            if (found >= 0) {
                for (int next = found; next < 2; ++next) {
                    lru_slots_[next] = lru_slots_[next + 1];
                }
                lru_slots_[2] = slot;
            }
            return &fonts_[slot];
        }
    }

    int oldest = lru_slots_[0];
    for (int position = 0; position < 2; ++position) {
        lru_slots_[position] = lru_slots_[position + 1];
    }
    lru_slots_[2] = oldest;
    heights_[oldest] = height;
    styles_[oldest] = style;
    Font *font = &fonts_[oldest];
    // The original publishes the cache keys before init and returns the slot
    // even if font creation fails. Do not invent a retry or null result here.
    font->init(DefaultFontFace, height, style);
    return font;
}
#pragma optimize("", on)

// Movement rendering publishes this font for the current order label.
// Original 0x00559DF6 stores the FontQueue result, also passed to set_font.
// RECOVERY_OBJECT: v2 0x0093FAE0 0x0093FAE4 ?VehDrawFont@@3PAVFont@@A 0x005594F0
Font *VehDrawFont;

#pragma optimize("y", off)
// ORIGINAL: 0x005593E0 ?pick_top_veh@@YAHH@Z 0x005593E0-0x005594EB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original5593E0..5594EB climbs signed previous links then visits next links, snapshots the Console faction, excludes temporary suppression, prefers eligible selected vehicle and retains the first highest-score entry.", "Original559466 test dh,42 establishes state mask4200, correcting the seed420000. Original ctor/reset member accesses and ConsoleGlobal9156B0 establish faction/cursor/selected/enable ownership, not independent globals. Observed VC6 symbol and original complete seed extent."], "reason": "Recover stack display selection against real vehicle/prototype/chassis records and Console state."}
int __cdecl pick_top_veh(int veh_id) {
    int best_score = -99;
    int best = veh_id;
    if (veh_id >= 0) {
        int index = veh_top(veh_id);
        if (index >= 0) {
            const int faction = ConsoleGlobal.faction_id_;
            do {
                if (VehDrawSuppressedVehicle == -1 || index != VehDrawSuppressedVehicle) {
                    if (index == ConsoleGlobal.selected_vehicle_
                        && Vehs[index].faction_id == faction
                        && (Vehs[index].state & 0x4200) == 0
                        && ConsoleGlobal.cursor_active_ != 0
                        && ConsoleGlobal.field_23BE4_ != 0) {
                        return index;
                    }
                    const int prototype = Vehs[index].proto_id;
                    const int chassis = VehPrototypes[prototype].chassis_id;
                    const int score = Chassis[chassis].triad
                        + (Vehs[index].faction_id == faction ? 4 : 0);
                    if (score > best_score) {
                        best_score = score;
                        best = index;
                    }
                }
                index = Vehs[index].next_veh_id_stack;
            } while (index >= 0);
        }
    }
    return best;
}

// WORKAROUND(decl): 0x0055AFEF-0x0055B05B computes the y inset before the x
// inset, i.e. the right-to-left arguments of an inlined rect shrink; the
// original helper's name/spelling is unknown.
static inline void base_draw_shrink_rect(RECT *rect, int dx, int dy) {
    rect->left += dx;
    rect->right -= dx;
    rect->top += dy;
    rect->bottom -= dy;
}

// Every size/offset on the map overlay scales the same way: by the caption
// override when one is active, otherwise by the zoom step out of 16. The image
// evaluates the value once before the scale test (0x55983A, 0x559874), which a
// macro repeating it in both arms would not do.
// WORKAROUND(decl): file-local stand-in for the original's zoom-scale helper/macro (name unknown).
static inline int veh_draw_scale(int value, int zoom) {
    return MapWinCaptionScale > 0
        ? value * MapWinCaptionScale / MapWinCaptionDivisor
        : (zoom + 16) * value / 16;
}

// ORIGINAL: 0x005594F0 ?veh_draw@@YAHPAUBuffer@@HHHHHH@Z 0x005594F0-0x0055A14E
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5594F0..55A14E retains raw screen coordinates for vehicle bodies, applies metric offsets only to overlays, and re-reads shared caption scale after callbacks. Health foreground reuses the rectangle passed to black fill. Independent original review corrects seed receiver offsets and preserves cached faction versus fresh Console reads.", "Original559F40 forms the four-byte label scratch at[ebp-4] and calls strcpy645460 with the complete StringTable label. Original559FEC/559FF0 subsequently clear bytes2/1;55A075 reads byte0 and55A082 may duplicate a bracket into byte1. Preserve the unchecked full copy, including malformed-label overflow; the native effects on adjacent stack storage remain unverified. Prior deliberate first-character-only safety interpretation is retained in git:e8d406295528953809aa8bf5d2cb929e7fb6659f, not the faithful milestone implementation.", "Preserve existing seed identity and complete original spans. Source binding is not a measured matching or runtime claim."], "reason": "Recover vehicle bodies and overlays using actual sprites, chassis metrics, faction palettes and font storage."}
// symbol    ?veh_draw@@YAHPAVBuffer@@HHHHHH@Z
int __cdecl veh_draw(Buffer * buffer, int veh_id, int screen_x, int screen_y, int zoom, int flags, int frame) {
    RECT rect;
    char label[4];

    if (flags & 1) {
        veh_id = pick_top_veh(veh_id);
        if (VehDrawSuppressedVehicle != -1 && VehDrawSuppressedVehicle == veh_id) {
            return 0;
        }
    }

    const int faction = Vehs[veh_id].faction_id;

    if ((zoom < -8 || (flags & 0x100)) && !(flags & 8) && !(flags & 0x800)) {
        if (faction != ConsoleGlobal.faction_id_
            && VehPrototypes[Vehs[veh_id].proto_id].plan == 11
            && !(GameDataStateStorage.fields_.game_state_ & 0x80)) {
            return 0;
        }
        if (flags & 0x400) {
            return 0;
        }
        set_rectangle_dimensions(&rect, screen_x, screen_y,
                                 veh_draw_scale(100, zoom), veh_draw_scale(50, zoom));
        if (!(flags & 0x200)) {
            base_draw_shrink_rect(&rect, veh_draw_scale(8, zoom), veh_draw_scale(8, zoom));
        } else {
            base_draw_shrink_rect(&rect, -2, -2);
        }
        const int quarter = (rect.bottom - rect.top) / 4;
        rect.top += quarter;
        rect.bottom += quarter;
        const int half = (rect.right - rect.left - rect.bottom + rect.top) / 2;
        rect.left += half;
        rect.right -= half;
        const int frame_color = strcmp(Players[faction].search_key, "SPARTANS") == 0 ? 44 : 0;
        buffer->box(&rect, frame_color, frame_color);
        rect.left++;
        rect.right--;
        rect.top++;
        rect.bottom--;
        buffer->fill(&rect, InfoFactionTextColors[faction]);
        return 0;
    }

    if (MapWinCaptionScale > 0) {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    } else {
        SpriteScaleNumerator = zoom + 16;
        SpriteScaleDenominator = 16;
    }

    const int prototype = Vehs[veh_id].proto_id;
    int category = VehPrototypes[prototype].chassis_id;
    if (Weapon[VehPrototypes[prototype].weapon_id].icon >= 0 && category == 0) {
        category = Weapon[VehPrototypes[prototype].weapon_id].icon + 9;
    }
    if (VehPrototypes[prototype].icon_offset >= 0) {
        category = VehPrototypes[prototype].icon_offset + 9;
    }

    int morale;
    if (faction) {
        morale = morale_veh(veh_id, 0, 0);
    } else {
        morale = morale_alien(veh_id, ConsoleGlobal.faction_id_);
    }

    // Separate stores per arm (0x5597D4..0x5597F2): not one `||` test.
    int variation = 0;
    if (category == 12) {
        variation = morale;
    } else if (category == 10) {
        variation = morale;
    } else if (category == 13) {
        variation = morale;
    } else if (category == 14) {
        variation = morale;
    } else if (category == 15) {
        if (Vehs[veh_id].proto_id == 17) variation = 1;
        if (Vehs[veh_id].proto_id == 18) variation = 2;
    }

    // The body sprites keep the raw screen origin; only the flag, health
    // bar and order label use the chassis-adjusted position.
    int x;
    int y;
    if (!(flags & 0x1000)) {
        x = screen_x + veh_draw_scale(Chassis[category].sprite_flag_x_off[variation], zoom);
        y = screen_y + veh_draw_scale(Chassis[category].sprite_flag_y_off[variation], zoom);
    } else {
        x = screen_x;
        y = screen_y;
    }

    const int no_sprite = flags & 0x800;
    if (!no_sprite) {
        if (category >= 9) {
            Sprite *sprite = &g_VEH_SPRITES[category * 8 + variation];
            sprite->draw_dest(buffer, sprite->cTransparentIndex_, screen_x, screen_y, ShadowColorTable);
        }
        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
        if (!(flags & 0x80) && category < 9) {
            if (flags & 0x10) {
                vehdraw_turn(buffer, screen_x, screen_y, frame, zoom);
            } else if (flags & 0x20) {
                vehdraw_scoot(buffer, screen_x, screen_y, frame, zoom);
            } else {
                vehdraw_draw(buffer, veh_id, screen_x, screen_y, flags, zoom, frame);
            }
        }
    }

    if (MapWinCaptionScale > 0) {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    } else {
        SpriteScaleNumerator = zoom + 16;
        SpriteScaleDenominator = 16;
    }

    if (((flags & 0x40) || (flags & 0x400)) && !no_sprite) {
        goto tail;
    }
    if (faction != ConsoleGlobal.faction_id_
        && VehPrototypes[Vehs[veh_id].proto_id].plan == 11
        && !(GameDataStateStorage.fields_.game_state_ & 0x80)) {
        goto tail;
    }

    {
        // One shared `exhausted = 1` (0x559A44) reached from both arms.
        int exhausted = 0;
        if (IsMultiplayerNet) {
            if (!veh_moves(veh_id)) {
                exhausted = 1;
            }
        } else if (faction == ConsoleGlobal.faction_id_ && !veh_moves(veh_id)) {
            exhausted = 1;
        }

        if ((flags & 2) && stack_check(veh_id, 1, -1, -1, -1) > 1) {
            if (Chassis[category].sprite_flag_x_off[variation] > 50) {
                x -= veh_draw_scale(6, zoom);
            }
            Sprite *stack_flag = &g_FLAGS_VEH_SPRITES[(faction * 7 + morale) * 2 + exhausted];
            stack_flag->draw_mono(buffer, stack_flag->cTransparentIndex_, x, y,
                                  InfoFactionShadowColors[faction]);
            x += veh_draw_scale(6, zoom);
        }
        Sprite *flag = &g_FLAGS_VEH_SPRITES[(faction * 7 + morale) * 2 + exhausted];
        flag->draw(buffer, flag->cTransparentIndex_, x, y);

        set_rectangle_dimensions(&rect, x + veh_draw_scale(6, zoom), y + veh_draw_scale(6, zoom),
                                 veh_draw_scale(2, zoom), veh_draw_scale(21, zoom));
        buffer->fill(&rect, 0);

        // Health is re-expanded at every use (five proto_power copies,
        // 0x559C40..0x559DA9); the foreground reuses the rectangle the black
        // fill received.
        if (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999)) {
            rect.bottom = (rect.bottom - rect.top)
                * range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999)
                / proto_power(veh_id) + rect.top;
            int health_color = 249;
            if (Vehs[veh_id].dmg_incurred < proto_power(veh_id) * 2 / 3) {
                health_color = 251;
            }
            if (Vehs[veh_id].dmg_incurred < proto_power(veh_id) / 3 || !Vehs[veh_id].dmg_incurred) {
                health_color = 250;
            }
            buffer->fill(&rect, health_color);
        }

        VehDrawFont = g_FONTQUEUE_VAL2.get_zoom(zoom, 1, 4, 1);
        buffer->set_font(VehDrawFont, 0, 0, 0);
        buffer->set_text_color(strcmp(Players[faction].search_key, "SPARTANS") == 0 ? 44 : 0, -1, 1, 1);

        const int order = range(Vehs[veh_id].order, 0, 29);
        set_rectangle_dimensions(&rect, x + veh_draw_scale(10, zoom), y + veh_draw_scale(5, zoom),
                                 veh_draw_scale(11, zoom), veh_draw_scale(14, zoom));

        // Original 559F40 copies the complete label before the later byte
        // clears. Preserve its unchecked four-byte scratch, not a safe prefix
        // copy; malformed-label effects on adjacent native storage are unproved.
        strcpy(label, StringTable.get(Order[order].letter));
        if (Vehs[veh_id].state & 0x4000) {
            strcpy(label, "/");
        }
        if ((Vehs[veh_id].state & 0x1000200) == 0x1000200) {
            strcpy(label, "P");
        }
        if (Vehs[veh_id].state & 0x200) {
            if (Vehs[veh_id].order_auto_type == 11) {
                strcpy(label, "*");
            }
            if ((Vehs[veh_id].state & 0x200) && Vehs[veh_id].order_auto_type == 10) {
                strcpy(label, "B");
            }
        }

        // 559FEC/559FF0 terminate the one-character label. 55A082
        // duplicates brackets into byte1 ("[[" / "]]"), not a dead local.
        label[2] = 0;
        label[1] = 0;
        if (faction != ConsoleGlobal.faction_id_
            || !((unsigned char)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << faction))) {
            if (!(GameDataStateStorage.fields_.game_state_ & 0x80)
                && ((unsigned char)GameDataStateStorage.fields_.lock_masks_.enable_mask_
                    & (1 << ConsoleGlobal.faction_id_))) {
                label[0] = '-';
            } else {
                if ((GameDataStateStorage.fields_.game_state_ & 0x40)
                    && (GameDataStateStorage.fields_.game_state_ & 0x1000)) {
                    label[0] = Vehs[veh_id].move_to_ai_type;
                }
                if (label[0] != '-' && (Vehs[veh_id].state & 0x40000)) {
                    if (Vehs[veh_id].state & 0x20000) {
                        buffer->set_text_color(0xF8, -1, 1, 1);
                    } else {
                        buffer->set_text_color(0xFB, -1, 1, 1);
                    }
                }
            }
        }
        if (label[0] == '[' || label[0] == ']') {
            label[1] = label[0];
        }

        if (faction == ConsoleGlobal.faction_id_) {
            if (Vehs[veh_id].state & 0x200) {
                buffer->set_text_color(InfoFactionShadowColors[faction], -1, 1, 1);
                if (label[0] == '-') {
                    label[0] = 'a';
                }
                if ((GameDataStateStorage.fields_.game_state_ & 0x40)
                    && (GameDataStateStorage.fields_.game_state_ & 0x1000)) {
                    label[0] = Vehs[veh_id].move_to_ai_type;
                }
            }
            if (faction == ConsoleGlobal.faction_id_ && (Vehs[veh_id].state & 0x8000000)) {
                buffer->set_text_color(ConsoleGlobal.faction_id_ == 3 ? 250 : 255, -1, 1, 1);
            }
        }
        // Header inline Buffer::write_cent_l(LPSTR, RECT *): its null test
        // survives at 0x55A0FE.
        buffer->write_cent_l(label, &rect);
    }

tail:
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
    if (!no_sprite) {
        return category < 9;
    }
    return 0;
}

// Visible-interval test for one map tile in one map window, across the seam.
// Original 55A2C0..55A333 and 55A339..55A3B5 (and 55A67F../55A6F9.. in the
// animated scan) repeat this whole sequence once per endpoint.
// WORKAROUND(decl): file-local stand-in for the original's tile-visibility helper (name unknown).
static inline int scoot_tile_visible(MapWin *window, int x, int y) {
    if (y >= window->field_1DDA8_ &&
        y < window->field_1DDD8_ + window->field_1DDD0_ + window->field_1DDA8_) {
        if (!(MapIsFlat & 1)) {
            if (x < window->field_1DDA4_) x += MapLongitudeBounds;
            if (x >= window->field_1DDD4_ + window->field_1DDCC_ + window->field_1DDA4_)
                x -= MapLongitudeBounds;
        }
        if (x >= window->field_1DDA4_ &&
            x < window->field_1DDD4_ + window->field_1DDCC_ + window->field_1DDA4_)
            return 1;
    }
    return 0;
}

// WORKAROUND(decl): file-local stand-in for the original's four-edge rect
// setter (name unknown): the image evaluates the four edges right to left
// (bottom, right, top, left) before storing them, as call arguments do
// (quayle_set_rect in quaylewin.cpp is the same shape).
static inline void scoot_set_rect(RECT *rect, int left, int top, int right, int bottom) {
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

// WORKAROUND(inline): expand_rect (scroll.cpp) is a plain out-of-line
// function in this tree; veh_scoot expands it in place (0x0055A976..: left and
// top lowered, right and bottom raised by the quarter margins).
static inline RECT *scoot_expand_rect(RECT *rect, int horizontal, int vertical) {
    rect->left -= horizontal;
    rect->right += horizontal;
    rect->top -= vertical;
    rect->bottom += vertical;
    return rect;
}

// ORIGINAL: 0x0055A150 ?veh_scoot@@YAXHHHHH@Z 0x0055A150-0x0055AF11;0x0065FF70-0x0065FFC7
// RECOVERY_BINDING: v1 {"evidence": ["Original55A175..55A1D9 constructs Buffer[8],Vector[8],singleton Vector,Vector[8], with reverse normal teardown55AEA7..55AEFB and EH65FF70..65FFC7. Use genuine value subtraction, mutating scale and compound operations. Preserve flags1 turn-only cleanup, flags4000 immediate redraw, fresh MapWinTable receivers and preference reads, source/destination copy direction, wrap and timing. Original55A29D/55A65C skips active-slot zeroing for null MapWinTable entries; later reads55AA60/55AC10/55AE26 retain the indeterminate-stack quirk. Buffer array [EBP-2EF8,EBP-2B8) is disjoint from active flags [EBP-B0,EBP-90). Null-slot runtime effects remain unverified, not normalized to inactive.", "Preserve existing seed identity and complete original spans. Source binding is not a measured matching or runtime claim."], "reason": "Recover real movement animation with automatic Buffer and Vector lifetime rather than raw stack/return-slot scaffolding."}
void __cdecl veh_scoot(int veh_id, int tile_x, int tile_y, int direction, int flags) {
    int retain_origin = 0;
    Buffer saved[8];
    Vector position[8];
    Vector destination;
    Vector increment[8];
    // Pinned scans initialize active[i] only for non-null MapWinTable[i].
    // Retain the original indeterminate-read quirk for null slots; do not
    // substitute a deterministic inactive value for that unverified path.
    int active[8];
    int start_x[8], start_y[8], end_x[8], end_y[8];
    int margin_x[8], margin_y[8];
    RECT area[8];
    RECT intersection;
    RECT end_clip;
    RECT start_clip;
    // One function-scope RECT serves the origin tile and both per-frame
    // moving rectangles: the image frame (Buffer array at EBP-2EF8, EH
    // cleanup 0x0065FF70) has no separate block-scope slots for them.
    RECT tile_rect;
    int i;

    VehDrawPositionY = tile_y;
    VehDrawPositionX = tile_x;
    VehDrawPositionVehicle = veh_id;
    if (direction >= 8) {
        direction %= 8;
        retain_origin = 1;
    }
    g_SCOOT_WAVE.play();
    const int next_x = wrap_map_x(VehMoveDeltaX[direction] + tile_x);
    const int next_y = VehMoveDeltaY[direction] + tile_y;
    if (flags & 0x4000) {
        draw_tile(tile_x, tile_y, 0);
        draw_tile(next_x, next_y, 0);
        for (i = 0; i < 8; ++i) {
            if (!MapWinTable[i]) continue;
            active[i] = 0;
            if (i && !MapWinTable[i]->view_active_) continue;
            if (scoot_tile_visible(MapWinTable[i], tile_x, tile_y) ||
                scoot_tile_visible(MapWinTable[i], next_x, next_y)) {
                active[i] = 1;
                MapWinTable[i]->tile_to_pixel(tile_x, tile_y, &start_x[i], &start_y[i]);
                MapWinTable[i]->tile_to_pixel(next_x, next_y, &end_x[i], &end_y[i]);
                start_y[i] -= MapWinTable[i]->field_1DDC0_;
                end_y[i] -= MapWinTable[i]->field_1DDC0_;
                set_rectangle_dimensions(&tile_rect, start_x[i], start_y[i],
                    MapWinTable[i]->tile_pixel_width_,
                    MapWinTable[i]->tile_pixel_height_ + MapWinTable[i]->field_1DDC0_);
                scoot_set_rect(&area[i],
                    start_x[i] < end_x[i] ? start_x[i] : end_x[i],
                    start_y[i] < end_y[i] ? start_y[i] : end_y[i],
                    (start_x[i] > end_x[i] ? start_x[i] : end_x[i]) +
                        MapWinTable[i]->tile_pixel_width_,
                    (start_y[i] > end_y[i] ? start_y[i] : end_y[i]) +
                        MapWinTable[i]->tile_pixel_height_ +
                        MapWinTable[i]->field_1DDC0_);
                UnionRect(&area[i], &tile_rect, &area[i]);
                MapWinTable[i]->compute_clip(next_x, next_y, 0, &end_clip);
                MapWinTable[i]->compute_clip(tile_x, tile_y, 0, &start_clip);
                UnionRect(&area[i], &end_clip, &start_clip);
            }
        }
        Vehs[veh_id].facing = static_cast<int8_t>(direction);
    } else {
        if (IsMultiplayerNet && Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_) {
            WorldClimateWorldWin.set_detail_map();
            WorldClimateWorldWin.viewport_outline_enabled_ = 1;
        }
        draw_tile(tile_x, tile_y, 0);
        draw_tile(next_x, next_y, 0);
        int delay = IsMultiplayerNet ? 10 : 16;
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == 1)
            delay = delay * 2 / 3;
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == 2)
            delay /= 3;
        int move_frames = FastUnitAnim < 0 ? 9 : (FastUnitAnim == 0 ? 6 : 3);
        if (IsMultiplayerNet) {
            if (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_) delay = 0;
            move_frames = 4;
            Vehs[veh_id].facing = static_cast<int8_t>(direction);
        }
        for (i = 0; i < 8; ++i) {
            if (!MapWinTable[i]) continue;
            active[i] = 0;
            if (i && !MapWinTable[i]->view_active_) continue;
            if (scoot_tile_visible(MapWinTable[i], tile_x, tile_y) ||
                scoot_tile_visible(MapWinTable[i], next_x, next_y)) {
                active[i] = 1;
                MapWinTable[i]->tile_to_pixel(tile_x, tile_y, &start_x[i], &start_y[i]);
                MapWinTable[i]->tile_to_pixel(next_x, next_y, &end_x[i], &end_y[i]);
                start_y[i] -= MapWinTable[i]->field_1DDC0_;
                end_y[i] -= MapWinTable[i]->field_1DDC0_;
                position[i].x_ = static_cast<float>(start_x[i]);
                position[i].y_ = static_cast<float>(start_y[i]);
                destination.x_ = static_cast<float>(end_x[i]);
                destination.y_ = static_cast<float>(end_y[i]);
                increment[i] = destination - position[i];
                increment[i].scale(1.0f / static_cast<float>(move_frames));
                position[i] += increment[i];
                scoot_set_rect(&area[i],
                    start_x[i] < end_x[i] ? start_x[i] : end_x[i],
                    start_y[i] < end_y[i] ? start_y[i] : end_y[i],
                    (start_x[i] > end_x[i] ? start_x[i] : end_x[i]) +
                        MapWinTable[i]->tile_pixel_width_,
                    (start_y[i] > end_y[i] ? start_y[i] : end_y[i]) +
                        MapWinTable[i]->tile_pixel_height_ +
                        MapWinTable[i]->field_1DDC0_);
                MapWinTable[i]->compute_clip(next_x, next_y, 0, &end_clip);
                MapWinTable[i]->compute_clip(tile_x, tile_y, 0, &start_clip);
                UnionRect(&area[i], &end_clip, &start_clip);
                margin_x[i] = (area[i].right - area[i].left) / 4;
                margin_y[i] = (area[i].bottom - area[i].top) / 4;
                scoot_expand_rect(&area[i], margin_x[i], margin_y[i]);
                saved[i].init(area[i].right - area[i].left, area[i].bottom - area[i].top, 0, 0);
                // Implicit receiver conversion: the image applies the virtual
                // base offset here without the null test an argument gets.
                MapWinTable[i]->copy(&saved[i], area[i].left, area[i].top, 0, 0,
                    area[i].right - area[i].left, area[i].bottom - area[i].top);
            }
        }
        // Preference is re-read after all preparation callbacks.
        const int turn_frames = vehdraw_turn_start(veh_id, direction,
            FastUnitAnim < 0 ? 25 : (FastUnitAnim == 0 ? 16 : 8), 2);
        int previous_time = timeGetTime();
        int now;
        int frame;
        for (frame = 0; frame < turn_frames; ++frame) {
            for (i = 0; i < 8; ++i) {
                if (active[i]) {
                    set_rectangle_dimensions(&tile_rect, start_x[i], start_y[i],
                        area[i].right - area[i].left, area[i].bottom - area[i].top);
                    if (IntersectRect(&intersection, &tile_rect, &area[i])) {
                        veh_draw(MapWinTable[i], veh_id, start_x[i], start_y[i],
                            MapWinTable[i]->zoom_offset_, 0x12, frame);
                        MapWinTable[i]->update(&area[i], 0);
                        saved[i].copy(MapWinTable[i],
                            start_x[i] - margin_x[i] - area[i].left,
                            start_y[i] - margin_y[i] - area[i].top,
                            start_x[i] - margin_x[i], start_y[i] - margin_y[i],
                            area[i].right - area[i].left, area[i].bottom - area[i].top);
                    }
                }
            }
            do {
                do_non_input();
                now = timeGetTime();
            } while (now - previous_time < delay / 2);
            previous_time = now;
        }
        // Original55ABCD jumps straight to the global-coordinate cleanup.
        if (flags & 1) goto finish;
        vehdraw_scoot_start(veh_id, direction, move_frames, 2);
        for (frame = 0; frame < move_frames; ++frame) {
            for (i = 0; i < 8; ++i) {
                if (active[i]) {
                    set_rectangle_dimensions(&tile_rect,
                        static_cast<int>(position[i].x_), static_cast<int>(position[i].y_),
                        area[i].right - area[i].left, area[i].bottom - area[i].top);
                    if (IntersectRect(&intersection, &tile_rect, &area[i])) {
                        veh_draw(MapWinTable[i], veh_id,
                            static_cast<int>(position[i].x_), static_cast<int>(position[i].y_),
                            MapWinTable[i]->zoom_offset_, 0x22, frame);
                        MapWinTable[i]->update(&area[i], 0);
                        saved[i].copy(MapWinTable[i],
                            static_cast<int>(position[i].x_) - margin_x[i] - area[i].left,
                            static_cast<int>(position[i].y_) - margin_y[i] - area[i].top,
                            static_cast<int>(position[i].x_) - margin_x[i],
                            static_cast<int>(position[i].y_) - margin_y[i],
                            area[i].right - area[i].left, area[i].bottom - area[i].top);
                    }
                    position[i] += increment[i];
                }
            }
            do {
                do_non_input();
                now = timeGetTime();
            } while (now - previous_time < delay);
            previous_time = now;
        }
        for (i = 0; i < 8; ++i) position[i] -= increment[i];
    }
    if (!retain_origin) {
        VehDrawPositionX = next_x;
        VehDrawPositionY = next_y;
    }
    for (i = 0; i < 8; ++i) {
        if (active[i]) {
            veh_draw(MapWinTable[i], veh_id, end_x[i], end_y[i],
                MapWinTable[i]->zoom_offset_, 2, 1);
            MapWinTable[i]->soft_update(&area[i]);
            do_all_draws();
        }
    }
finish:
    VehDrawPositionX = -1;
    VehDrawPositionY = -1;
    flush_input();
}
#pragma optimize("", on)

// The base-caption renderer retains the font selected from its shared queue.
// Original storage: 0x0093FC24. No other product owner currently publishes it.
// RECOVERY_OBJECT: v2 0x0093FC24 0x0093FC28 ?BaseDrawFont@@3PAVFont@@A 0x0055AF20
Font *BaseDrawFont;

#pragma optimize("y", off)
// base_draw is the 2073-byte base renderer reached by MapWin::draw_bases.
// The seed's coordinate, sprite, caption, visibility and status branches are
// expressed through their owning objects below. The original span and seed
// remain available for compiler and reference comparison.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: full line-by-line translation compiles and matches control flow, call args verified against raw disasm (bitmask/box/fill/draw_dest/ get_zoom/set_font/set_text_color/write_l/width/itoa/strcat all cross-checked); prologue register allocation differs (#0 push/sub).
// Tile-zoom scaling repeated at every use in the image: the scale branch
// first (`test esi,esi; jle`), else (style + 16) * n / 16.
#define BASE_DRAW_SCALE(n) (MapWinCaptionScale > 0 ? MapWinCaptionScale * (n) / MapWinCaptionDivisor : (style + 16) * (n) / 16)

// ORIGINAL: 0x0055AF20 ?base_draw@@YAXPAUBuffer@@HHHHH@Z 0x0055AF20-0x0055B739
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0055af20.cpp retains the 2073-byte transcription and original span; the seed's Base, Map, Sprite, FontQueue and Buffer dependencies have typed product owners."], "reason": "Recover the base renderer's distant zoom badge, faction sprite, population caption, visibility check and riot marker through typed objects; compiler and runtime verification remain separate."}
// symbol    ?base_draw@@YAXPAVBuffer@@HHHHH@Z
void __cdecl base_draw(Buffer *target, int base_id, int pixel_x, int pixel_y,
                       int style, int flags) {
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    int faction = Bases[base_id].faction_id_current;
    RECT rect;

    // At distant zoom, show the base as a colored lozenge.
    if ((style < -8 || (flags & 0x100)) && !(flags & 8) && !(flags & 0x800)) {
        set_rectangle_dimensions(&rect, pixel_x, pixel_y,
                                 BASE_DRAW_SCALE(100), BASE_DRAW_SCALE(50));
        const int outline = flags & 0x200;
        if (!outline) {
            base_draw_shrink_rect(&rect, BASE_DRAW_SCALE(4), BASE_DRAW_SCALE(4));
        } else {
            base_draw_shrink_rect(&rect, -2, -2);
        }
        int trim = (rect.right - rect.bottom - rect.left + rect.top) / 2;
        rect.left += trim;
        rect.right -= trim;
        const int inset = outline ? 2 : BASE_DRAW_SCALE(4);
        int shift = BASE_DRAW_SCALE(6) - inset;
        rect.top += shift;
        rect.bottom += shift;
        int border = strcmp(Players[faction].search_key, "SPARTANS") == 0 ? 44 : 0;
        target->box(&rect, border, border);
        ++rect.left;
        --rect.right;
        ++rect.top;
        --rect.bottom;
        target->fill(&rect, InfoFactionTextColors[faction]);
        return;
    }

    if (MapWinCaptionScale > 0) {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    } else {
        SpriteScaleNumerator = style + 16;
        SpriteScaleDenominator = 16;
    }

    int mode = 3;
    if (Bases[base_id].population_size < 4) {
        mode = 0;
    } else if (Bases[base_id].population_size
               < max(Rules.pop_limit_sans_hab_complex + 1, 7)) {
        mode = 1;
    } else if (Bases[base_id].population_size
               < max(Rules.pop_limit_sans_hab_dome + 1, 10)) {
        mode = 2;
    }
    if (has_fac(FAC_HEADQUARTERS, base_id, 0)) {
        mode = range(mode + 1, 0, 3);
    }
    int danger = 0;
    if (has_fac(FAC_PERIMETER_DEFENSE, base_id, 0)
        || has_project(SP_CITIZENS_DEFENSE_FORCE, faction)) {
        danger = 1;
    }
    if (has_fac(FAC_TACHYON_FIELD, base_id, 0)) {
        danger = 2;
    }

    if (!(flags & 0x800)) {
        Sprite *sprite = FactionBaseSprites[Bases[base_id].faction_id_former]
                                           [is_ocean(x, y)][mode][danger];
        sprite->draw_dest(target, sprite->transparent_index(), pixel_x, pixel_y,
                          ShadowColorTable);
    }

    BaseDrawFont = g_FONTQUEUE_VAL1.get_zoom(style, 1, 3, 1);
    if (!(flags & 0x40)) {

        char number[80];
        char measure[80];
        char caption[80];
        caption[0] = '\0';
        measure[0] = '\0';
        _itoa(Bases[base_id].population_size, number, 10);
        strcat(caption, number);
        strcat(measure, "8");
        if (Bases[base_id].population_size >= 10) {
            strcat(measure, "8");
        }
        set_rectangle_dimensions(&rect, pixel_x, pixel_y,
                                 BaseDrawFont->width(measure) + 5,
                                 BaseDrawFont->pixel_height() + 4);

        int revealed = veh_who(x, y) >= 0;
        if (!revealed) {
            if ((PlayersData[Bases[base_id].faction_id_current]
                     .diplo_treaties[ConsoleGlobal.faction_id_] & 1)
                || spying(Bases[base_id].faction_id_current)
                || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
                || (PlayersData[ConsoleGlobal.faction_id_].flags & PFLAG_MAP_REVEALED)
                || faction == ConsoleGlobal.faction_id_) {
                revealed = 1;
            } else {
                for (int i = 0; i < 8; i++) {
                    int nx = wrap_map_x(Bases[base_id].x + VehMoveDeltaX[i]);
                    int ny = Bases[base_id].y + VehMoveDeltaY[i];
                    if (on_map(nx, ny) && veh_who(nx, ny) == ConsoleGlobal.faction_id_) {
                        revealed = 1;
                        break;
                    }
                }
            }
        }
        if (!revealed) {
            int border = strcmp(Players[faction].search_key, "SPARTANS") == 0
                ? InfoFactionShadowColors[faction] : 0;
            target->box(&rect, border, border);
            ++rect.left;
            --rect.right;
            ++rect.top;
            --rect.bottom;
            target->fill(&rect, InfoFactionTextColors[faction]);
            target->set_font(BaseDrawFont, 0, 0, 0);
            target->set_text_color(strcmp(Players[faction].search_key, "SPARTANS") == 0
                                       ? 44 : 0, -1, 1, 1);
            target->write_l(caption, rect.left + 2, rect.top + 1);
        } else {
            target->box(&rect, 0, 0);
            ++rect.left;
            --rect.right;
            ++rect.top;
            --rect.bottom;
            target->set_font(BaseDrawFont, 0, 0, 0);
            target->set_text_color(ChatItemIdTable[faction], -1, 1, 1);
            target->write_l(caption, rect.left + 2, rect.top + 1);
        }
        --rect.left;
        ++rect.right;
        --rect.top;
        ++rect.bottom;
        if (Bases[base_id].state & BSTATE_DRONE_RIOTS_ACTIVE) {
            int icon_x = pixel_x + (rect.right - rect.left) + 2;
            if (Players[Bases[base_id].faction_id_current].rule_flags & RFLAG_ALIEN) {
                g_RED_ALIEN_HEAD_ICON_SPRITE.draw(
                    target, g_RED_ALIEN_HEAD_ICON_SPRITE.transparent_index(),
                    icon_x + 2, pixel_y - 6);
            } else {
                g_RED_MALE_HEAD_ICON_SPRITE.draw(
                    target, g_RED_MALE_HEAD_ICON_SPRITE.transparent_index(),
                    icon_x, pixel_y);
            }
        }
    }
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
}
#undef BASE_DRAW_SCALE
#pragma optimize("", on)

/*
Purpose: Destroy the queue: hand the three-slot walk to the CRT vector
         iterator with the queue itself as the array base and the Font
         destructor as the per-element teardown.
// ORIGINAL: 0x0055B740 ??1FontQueue@@QAE@XZ 0x0055B740-0x0055B750 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ??1FontQueue@@QAE@XZ)(FontQueue* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
FontQueue::~FontQueue() {
}

// This header-defined destructor is emitted for the real Vector arrays in
// veh_scoot above, not forced by a measurement anchor; the constructor
// 0x006343C0 is out of line in vector.cpp.
// veh_scoot pairs this destructor with Vector construction for arrays of
// eight 12-byte elements, in both normal and exceptional cleanup paths.
// ORIGINAL: 0x0055B750 ??1Vector@@QAE@XZ 0x0055B750-0x0055B755 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x0055B750 tail-jumps to Vector::close at 0x006343D0; span agrees with git:2d73ab9a:src/unrecovered/0055b750.cpp historical seed.", "veh_scoot constructs two eight-element arrays of stride 12 using constructor 0x006343C0 and destructor 0x0055B750 at 0x0055A197-0x0055A1D9; normal and exceptional array cleanup reuse that destructor."], "reason": "Real Vector destructor calling close, replacing the seed-only synthetic receiver shim without removing its historical obligation."}
// LEVER: thiscall shim - a synthetic class exposes close() as a member so "call close(); ret" folds into the original's bare 5-byte jmp
// body      src/vector.h


// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// RECOVERY_OBJECT: v1 0x0093FB88 ?g_FONTQUEUE_VAL2@@3VFontQueue@@A
FontQueue g_FONTQUEUE_VAL2;
// RECOVERY_OBJECT: v1 0x0093FAE8 ?g_FONTQUEUE_VAL1@@3VFontQueue@@A
FontQueue g_FONTQUEUE_VAL1;
