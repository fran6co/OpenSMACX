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

// ORIGINAL: 0x005594F0 ?veh_draw@@YAHPAUBuffer@@HHHHHH@Z 0x005594F0-0x0055A14E
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5594F0..55A14E retains raw screen coordinates for vehicle bodies, applies metric offsets only to overlays, and re-reads shared caption scale after callbacks. Health foreground reuses the rectangle passed to black fill. Independent original review corrects seed receiver offsets and preserves cached faction versus fresh Console reads.", "Original559F40 forms the four-byte label scratch at[ebp-4] and calls strcpy645460 with the complete StringTable label. Original559FEC/559FF0 subsequently clear bytes2/1;55A075 reads byte0 and55A082 may duplicate a bracket into byte1. Preserve the unchecked full copy, including malformed-label overflow; the native effects on adjacent stack storage remain unverified. Prior deliberate first-character-only safety interpretation is retained in git:ab21c58fd7992141e7aa620e429574292d9b955f, not the faithful milestone implementation.", "Preserve existing seed identity and complete original spans. Source binding is not a measured matching or runtime claim."], "reason": "Recover vehicle bodies and overlays using actual sprites, chassis metrics, faction palettes and font storage."}
// symbol    ?veh_draw@@YAHPAVBuffer@@HHHHHH@Z
int __cdecl veh_draw(Buffer * buffer, int veh_id, int screen_x, int screen_y, int zoom, int flags, int frame) {
    int x = screen_x, y = screen_y;

    if ((flags & 1) != 0) {
        veh_id = pick_top_veh(veh_id);
        if (VehDrawSuppressedVehicle != -1 && VehDrawSuppressedVehicle == veh_id) {
            return 0;
        }
    }

    unsigned int faction = Vehs[veh_id].faction_id;
    unsigned int factionSaved = faction;

    if ((zoom < -8 || (flags & 0x100) != 0) && (flags & 8) == 0 && (flags & 0x800) == 0) {
        int prototype = Vehs[veh_id].proto_id;
        if (faction != (unsigned int)ConsoleGlobal.faction_id_ &&
            VehPrototypes[prototype].plan == '\v' && (GameDataStateStorage.fields_.game_state_ & 0x80) == 0) {
            return 0;
        }
        if ((flags & 0x400) != 0) {
            return 0;
        }
        int rw, rh;
        if (MapWinCaptionScale < 1) {
            rh = (zoom * 5 + 0x50) * 10 / 16;
            rw = (zoom * 5 + 0x50) * 0x14 / 16;
        } else {
            rh = (MapWinCaptionScale * 0x32) / MapWinCaptionDivisor;
            rw = (MapWinCaptionScale * 100) / MapWinCaptionDivisor;
        }
        rw += x;
        rh += y;
        if ((flags & 0x200) == 0) {
            int dxy;
            if (MapWinCaptionScale < 1) {
                dxy = (zoom * 8 + 0x80) / 16;
            } else {
                dxy = (MapWinCaptionScale * 8) / MapWinCaptionDivisor;
            }
            x += dxy;
            rw -= dxy;
            y += dxy;
            rh -= dxy;
        } else {
            x -= 2;
            rw += 2;
            y -= 2;
            rh += 2;
        }
        int qy = (rh - y) / 4;
        int botY = y + qy;
        int botYr = rh + qy;
        int mid = ((rw - x) - botYr + botY) / 2;
        int leftX = x + mid;
        int rightX = rw - mid;
        RECT r1;
        r1.left = leftX; r1.top = botY; r1.right = rightX; r1.bottom = botYr;
        int col = (strcmp(Players[faction].search_key, "SPARTANS") == 0) ? 44 : 0;
        buffer->box(&r1, col, col);
        r1.left++; r1.bottom--; r1.top++; r1.right--;
        buffer->fill(&r1, InfoFactionTextColors[faction]);
        return 0;
    }

    if (MapWinCaptionScale < 1) {
        SpriteScaleNumerator = zoom + 0x10;
        SpriteScaleDenominator = 0x10;
    } else {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    }

    int prototype = Vehs[veh_id].proto_id;
    unsigned int category = VehPrototypes[prototype].chassis_id;
    if ((char)Weapon[VehPrototypes[prototype].weapon_id].icon > -1 &&
        category == 0) {
        category = (char)Weapon[VehPrototypes[prototype].weapon_id].icon + 9;
    }
    if ((char)VehPrototypes[prototype].icon_offset > -1) {
        category = (char)VehPrototypes[prototype].icon_offset + 9;
    }
    unsigned int sprite_category = category;

    unsigned int morale;
    if (faction == 0) {
        morale = morale_alien(veh_id, ConsoleGlobal.faction_id_);
    } else {
        morale = morale_veh(veh_id, 0, 0);
    }

    unsigned int variation = 0;
    if (sprite_category == 12 || sprite_category == 10 || sprite_category == 13 || sprite_category == 14)
        variation = morale;
    else if (sprite_category == 15) {
        const int prototype = Vehs[veh_id].proto_id;
        if (prototype == 17) variation = 1;
        if (prototype == 18) variation = 2;
    }

    if ((flags & 0x1000) == 0) {
        int dx, dy;
        if (MapWinCaptionScale < 1) {
            dx = (zoom + 0x10) * Chassis[sprite_category].sprite_flag_x_off[variation] / 16;
        } else {
            dx = (int)(Chassis[sprite_category].sprite_flag_x_off[variation] * MapWinCaptionScale) / MapWinCaptionDivisor;
        }
        x += dx;
        if (MapWinCaptionScale < 1) {
            dy = (zoom + 0x10) * Chassis[sprite_category].sprite_flag_y_off[variation] / 16;
        } else {
            dy = (int)(Chassis[sprite_category].sprite_flag_y_off[variation] * MapWinCaptionScale) / MapWinCaptionDivisor;
        }
        y += dy;
    }

    unsigned int flagBit800 = flags & 0x800;
    if (flagBit800 == 0) {
        if ((int)sprite_category > 8) {
            Sprite *sp = &g_VEH_SPRITES[variation + sprite_category * 8];
            // EBX/ESI retain the unshifted origin; the adjusted stack
            // coordinates are only for the flag/health/order overlays.
            sp->draw_dest(buffer, sp->cTransparentIndex_, screen_x, screen_y, ShadowColorTable);
        }
        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
        if ((flags & 0x80) == 0 && (int)sprite_category < 9) {
            if ((flags & 0x10) == 0) {
                if ((flags & 0x20) == 0) {
                    vehdraw_draw(buffer, veh_id, screen_x, screen_y, flags, zoom, frame);
                } else {
                    vehdraw_scoot(buffer, screen_x, screen_y, frame, zoom);
                }
            } else {
                vehdraw_turn(buffer, screen_x, screen_y, frame, zoom);
            }
        }
    }

    if (MapWinCaptionScale < 1) {
        SpriteScaleDenominator = 0x10;
        SpriteScaleNumerator = zoom + 0x10;
    } else {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    }

    if ((((flags & 0x40) != 0 || (flags & 0x400) != 0) && flagBit800 == 0) ||
        (factionSaved != (unsigned int)ConsoleGlobal.faction_id_ &&
         VehPrototypes[Vehs[veh_id].proto_id].plan == '\v' &&
         (GameDataStateStorage.fields_.game_state_ & 0x80) == 0)) {
        goto tail;
    }

    {
        int exhausted = 0;
        if (IsMultiplayerNet == 0) {
            if (faction == (unsigned int)ConsoleGlobal.faction_id_) {
                unsigned int spd = speed(veh_id, 0);
                int diff = (int)spd - Vehs[veh_id].moves_expended;
                if (diff < 0 || (diff < 1000 && spd == Vehs[veh_id].moves_expended)) {
                    exhausted = 1;
                }
            }
        } else {
            unsigned int spd = speed(veh_id, 0);
            int diff = (int)spd - Vehs[veh_id].moves_expended;
            if (diff < 0 || (diff < 1000 && spd == Vehs[veh_id].moves_expended)) {
                exhausted = 1;
            }
        }

        unsigned int stackFaction = faction;
        if ((flags & 2) != 0) {
            int stackN = stack_check(veh_id, 1, -1, -1, -1);
            stackFaction = faction;
            if (stackN > 1) {
                if (Chassis[sprite_category].sprite_flag_x_off[variation] > 0x32) {
                    int dxs;
                    if (MapWinCaptionScale < 1) {
                        dxs = (zoom * 3 + 0x30) * 2 / 16;
                    } else {
                        dxs = (MapWinCaptionScale * 6) / MapWinCaptionDivisor;
                    }
                    x -= dxs;
                }
                Sprite *sp = &g_FLAGS_VEH_SPRITES[exhausted + (faction * 7 + morale) * 2];
                sp->draw_mono(buffer, sp->cTransparentIndex_, x, y, InfoFactionShadowColors[faction]);
                if (MapWinCaptionScale < 1) {
                    x += (zoom * 3 + 0x30) * 2 / 16;
                } else {
                    x += (MapWinCaptionScale * 6) / MapWinCaptionDivisor;
                }
            }
        }

        Sprite *sp2 = &g_FLAGS_VEH_SPRITES[exhausted + (stackFaction * 7 + morale) * 2];
        sp2->draw(buffer, sp2->cTransparentIndex_, x, y);

        RECT r2;
        int rr24, rr28, rr2c, rr30;
        if (MapWinCaptionScale < 1) {
            rr24 = (zoom + 0x10) * 0x15 / 16;
            rr28 = (zoom * 2 + 0x20) / 16;
            rr2c = (zoom * 3 + 0x30) * 2 / 16;
            rr30 = (zoom * 3 + 0x30) * 2 / 16;
        } else {
            rr24 = (MapWinCaptionScale * 0x15) / MapWinCaptionDivisor;
            rr28 = (MapWinCaptionScale * 2) / MapWinCaptionDivisor;
            rr2c = (MapWinCaptionScale * 6) / MapWinCaptionDivisor;
            rr30 = (MapWinCaptionScale * 6) / MapWinCaptionDivisor;
        }
        rr2c += y;
        rr30 += x;
        rr28 = rr30 + rr28;
        rr24 = rr2c + rr24;
        r2.left = rr30; r2.top = rr2c; r2.right = rr28; r2.bottom = rr24;
        buffer->fill(&r2, 0);

        // No callback intervenes between these state reads and the foreground
        // fill. Reactor and damage are bytes; health is bounded to 1..1000.
        const int health_prototype = Vehs[veh_id].proto_id;
        int reactor = VehPrototypes[health_prototype].reactor_id;
        if (reactor < 1) reactor = 1;
        else if (reactor > 100) reactor = 100;
        const int maximum_health = VehPrototypes[health_prototype].plan == 12
            ? 1 : reactor * 10;
        const int damage = Vehs[veh_id].dmg_incurred;
        const int remaining_health = maximum_health - damage;
        if (remaining_health > 0) {
            // Reuse the rectangle passed to black fill, including any changes
            // made by that call, rather than rebuilding it from stale locals.
            r2.bottom = (r2.bottom - r2.top) * remaining_health / maximum_health + r2.top;
            int health_color = 249;
            if (damage < maximum_health * 2 / 3) health_color = 251;
            if (damage < maximum_health / 3 || damage == 0) health_color = 250;
            buffer->fill(&r2, health_color);
        }

        VehDrawFont = g_FONTQUEUE_VAL2.get_zoom(zoom, 1, 4, 1);
        buffer->set_font(VehDrawFont, 0, 0, 0);
        int col2 = (strcmp(Players[faction].search_key, "SPARTANS") == 0) ? 44 : 0;
        buffer->set_text_color(col2, -1, 1, 1);

        int triadIdx = (signed char)Vehs[veh_id].order;
        if (triadIdx < 0) triadIdx = 0;
        else if (triadIdx > 0x1d) triadIdx = 0x1d;

        if (MapWinCaptionScale < 1) {
            rr24 = (zoom + 0x10) * 0xe / 16;
            rr28 = (zoom + 0x10) * 0xb / 16;
            rr2c = (zoom * 5 + 0x50) / 16;
            rr30 = (zoom * 5 + 0x50) * 2 / 16;
        } else {
            rr24 = (MapWinCaptionScale * 0xe) / MapWinCaptionDivisor;
            rr28 = (MapWinCaptionScale * 0xb) / MapWinCaptionDivisor;
            rr2c = (MapWinCaptionScale * 5) / MapWinCaptionDivisor;
            rr30 = (MapWinCaptionScale * 10) / MapWinCaptionDivisor;
        }
        rr2c += y;
        rr30 += x;
        rr28 = rr30 + rr28;
        rr24 = rr2c + rr24;
        r2.left = rr30; r2.top = rr2c; r2.right = rr28; r2.bottom = rr24;

        char *word = StringTable.get(Order[triadIdx].letter);
        // Original 559F40 copies the complete label before the later byte
        // clears. Preserve its unchecked four-byte scratch, not a safe prefix
        // copy; malformed-label effects on adjacent native storage are unproved.
        char lineBuf[4];
        strcpy(lineBuf, word);

        if ((Vehs[veh_id].state & 0x4000) != 0) {
            strcpy(lineBuf, "/");
        }
        if ((Vehs[veh_id].state & 0x1000200) == 0x1000200) {
            strcpy(lineBuf, "P");
        }
        if ((Vehs[veh_id].state & 0x200) != 0) {
            if (Vehs[veh_id].order_auto_type == '\v') {
                strcpy(lineBuf, "*");
            }
            if ((Vehs[veh_id].state & 0x200) != 0 &&
                Vehs[veh_id].order_auto_type == '\n') {
                strcpy(lineBuf, "B");
            }
        }

        // 559FEC/559FF0 terminate the one-character label. 55A082
        // duplicates brackets into byte1 ("[[" / "]]"), not a dead local.
        lineBuf[2] = 0;
        lineBuf[1] = 0;
        if (faction == (unsigned int)ConsoleGlobal.faction_id_ &&
            (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (unsigned char)(1 << (faction & 0x1f))) != 0) {
            if (lineBuf[0] == '[' || lineBuf[0] == ']') {
                lineBuf[1] = lineBuf[0];
            }
        } else {
            if ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0 ||
                (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (unsigned char)(1 << (ConsoleGlobal.faction_id_ & 0x1f))) == 0) {
                if ((GameDataStateStorage.fields_.game_state_ & 0x40) != 0 && (GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                    lineBuf[0] = Vehs[veh_id].move_to_ai_type;
                }
                if (lineBuf[0] != '-' && (Vehs[veh_id].state & 0x40000) != 0) {
                    int col3 = ((Vehs[veh_id].state & 0x20000) == 0) ? 0xfb : 0xf8;
                    buffer->set_text_color(col3, -1, 1, 1);
                }
                if (lineBuf[0] == '[' || lineBuf[0] == ']') {
                    lineBuf[1] = lineBuf[0];
                }
            } else {
                lineBuf[0] = '-';
            }
        }

        if (faction == (unsigned int)ConsoleGlobal.faction_id_) {
            if ((Vehs[veh_id].state & 0x200) != 0) {
                buffer->set_text_color(InfoFactionShadowColors[faction], -1, 1, 1);
                if (lineBuf[0] == '-') lineBuf[0] = 'a';
                if ((GameDataStateStorage.fields_.game_state_ & 0x40) != 0 && (GameDataStateStorage.fields_.game_state_ & 0x1000) != 0) {
                    lineBuf[0] = Vehs[veh_id].move_to_ai_type;
                }
            }
            if (faction == (unsigned int)ConsoleGlobal.faction_id_ &&
                (Vehs[veh_id].state & 0x8000000) != 0) {
                int col4 = ConsoleGlobal.faction_id_ == 3 ? 250 : 255;
                buffer->set_text_color(col4, -1, 1, 1);
            }
        }
        buffer->write_cent_l(lineBuf, &r2, (int)strlen(lineBuf));
    }

tail:
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
    if (flagBit800 != 0) {
        SpriteScaleDenominator = 1;
        SpriteScaleNumerator = 1;
        return 0;
    }
    return (int)sprite_category < 9;
}

// ORIGINAL: 0x0055A150 ?veh_scoot@@YAXHHHHH@Z 0x0055A150-0x0055AF11;0x0065FF70-0x0065FFC7
// RECOVERY_BINDING: v1 {"evidence": ["Original55A175..55A1D9 constructs Buffer[8],Vector[8],singleton Vector,Vector[8], with reverse normal teardown55AEA7..55AEFB and EH65FF70..65FFC7. Use genuine value subtraction, mutating scale and compound operations. Preserve flags1 turn-only cleanup, flags4000 immediate redraw, fresh MapWinTable receivers and preference reads, source/destination copy direction, wrap and timing. Original55A29D/55A65C skips active-slot zeroing for null MapWinTable entries; later reads55AA60/55AC10/55AE26 retain the indeterminate-stack quirk. Buffer array [EBP-2EF8,EBP-2B8) is disjoint from active flags [EBP-B0,EBP-90). Null-slot runtime effects remain unverified, not normalized to inactive.", "Preserve existing seed identity and complete original spans. Source binding is not a measured matching or runtime claim."], "reason": "Recover real movement animation with automatic Buffer and Vector lifetime rather than raw stack/return-slot scaffolding."}
void __cdecl veh_scoot(int veh_id, int tile_x, int tile_y, int direction, int flags) {
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

    VehDrawPositionY = tile_y;
    VehDrawPositionX = tile_x;
    VehDrawPositionVehicle = veh_id;
    bool retain_origin = false;
    if (direction >= 8) {
        direction %= 8;
        retain_origin = true;
    }
    g_SCOOT_WAVE.play();
    int next_x = tile_x + VehMoveDeltaX[direction];
    if (!(MapIsFlat & 1)) {
        if (next_x < 0) next_x += MapLongitudeBounds;
        else if (next_x >= MapLongitudeBounds) next_x -= MapLongitudeBounds;
    }
    const int next_y = tile_y + VehMoveDeltaY[direction];
    const bool immediate = (flags & 0x4000) != 0;
    int delay = 0, move_frames = 0;
    if (!immediate && IsMultiplayerNet &&
        Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_) {
        WorldClimateWorldWin.set_detail_map();
        WorldClimateWorldWin.viewport_outline_enabled_ = 1;
    }
    draw_tile(tile_x, tile_y, 0);
    draw_tile(next_x, next_y, 0);
    if (!immediate) {
        delay = IsMultiplayerNet ? 10 : 16;
        const int triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
        if (triad == 1) delay = delay * 2 / 3;
        if (triad == 2) delay /= 3;
        move_frames = FastUnitAnim < 0 ? 9 : (FastUnitAnim == 0 ? 6 : 3);
        if (IsMultiplayerNet) {
            if (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_) delay = 0;
            move_frames = 4;
            Vehs[veh_id].facing = static_cast<int8_t>(direction);
        }
    }
    for (int i = 0; i < 8; ++i) {
        MapWin *window = MapWinTable[i];
        if (!window) continue;
        active[i] = 0;
        if (i != 0 && !window->view_active_) continue;
        bool visible = false;
        for (int endpoint = 0; endpoint < 2; ++endpoint) {
            int x = endpoint ? next_x : tile_x;
            const int y = endpoint ? next_y : tile_y;
            if (y < static_cast<int>(window->field_1DDA8_) ||
                y >= static_cast<int>(window->field_1DDD8_ + window->field_1DDD0_ + window->field_1DDA8_))
                continue;
            if (!(MapIsFlat & 1)) {
                if (x < static_cast<int>(window->field_1DDA4_)) x += MapLongitudeBounds;
                if (x >= static_cast<int>(window->field_1DDD4_ + window->field_1DDCC_ + window->field_1DDA4_))
                    x -= MapLongitudeBounds;
            }
            if (x >= static_cast<int>(window->field_1DDA4_) &&
                x < static_cast<int>(window->field_1DDD4_ + window->field_1DDCC_ + window->field_1DDA4_)) {
                visible = true;
                break;
            }
        }
        if (!visible) continue;
        active[i] = 1;
        window->tile_to_pixel(tile_x, tile_y, &start_x[i], &start_y[i]);
        MapWinTable[i]->tile_to_pixel(next_x, next_y, &end_x[i], &end_y[i]);
        window = MapWinTable[i];
        start_y[i] -= window->field_1DDC0_;
        end_y[i] -= window->field_1DDC0_;
        if (!immediate) {
            position[i].x_ = static_cast<float>(start_x[i]);
            position[i].y_ = static_cast<float>(start_y[i]);
            destination.x_ = static_cast<float>(end_x[i]);
            destination.y_ = static_cast<float>(end_y[i]);
            increment[i] = destination - position[i];
            increment[i].scale(1.0f / static_cast<float>(move_frames));
            position[i] += increment[i];
        }
        window = MapWinTable[i];
        area[i].left = start_x[i] < end_x[i] ? start_x[i] : end_x[i];
        area[i].top = start_y[i] < end_y[i] ? start_y[i] : end_y[i];
        area[i].right = (start_x[i] > end_x[i] ? start_x[i] : end_x[i]) + window->tile_pixel_width_;
        area[i].bottom = (start_y[i] > end_y[i] ? start_y[i] : end_y[i]) +
                         window->tile_pixel_height_ + window->field_1DDC0_;
        if (immediate) {
            RECT origin;
            origin.left = start_x[i]; origin.top = start_y[i];
            origin.right = start_x[i] + window->tile_pixel_width_;
            origin.bottom = start_y[i] + window->tile_pixel_height_ + window->field_1DDC0_;
            UnionRect(&area[i], &origin, &area[i]);
        }
        RECT end_clip, start_clip;
        MapWinTable[i]->compute_clip(next_x, next_y, 0, &end_clip);
        MapWinTable[i]->compute_clip(tile_x, tile_y, 0, &start_clip);
        UnionRect(&area[i], &end_clip, &start_clip);
        if (!immediate) {
            margin_x[i] = (area[i].right - area[i].left) / 4;
            margin_y[i] = (area[i].bottom - area[i].top) / 4;
            area[i].left -= margin_x[i]; area[i].right += margin_x[i];
            area[i].top -= margin_y[i]; area[i].bottom += margin_y[i];
            saved[i].init(area[i].right - area[i].left, area[i].bottom - area[i].top, 0, 0);
            static_cast<Buffer *>(MapWinTable[i])->copy(&saved[i], area[i].left, area[i].top,
                0, 0, area[i].right - area[i].left, area[i].bottom - area[i].top);
        }
    }
    if (immediate) {
        Vehs[veh_id].facing = static_cast<int8_t>(direction);
    } else {
        // Preference is re-read after all preparation callbacks.
        const int turn_speed = FastUnitAnim < 0 ? 25 : (FastUnitAnim == 0 ? 16 : 8);
        const int turn_frames = vehdraw_turn_start(veh_id, direction, turn_speed, 2);
        DWORD previous_time = timeGetTime();
        for (int turn_frame = 0; turn_frame < turn_frames; ++turn_frame) {
            for (int i = 0; i < 8; ++i) if (active[i]) {
                const int width = area[i].right - area[i].left;
                const int height = area[i].bottom - area[i].top;
                RECT moving = {start_x[i], start_y[i], start_x[i] + width, start_y[i] + height};
                RECT intersection;
                if (IntersectRect(&intersection, &moving, &area[i])) {
                    veh_draw(static_cast<Buffer *>(MapWinTable[i]), veh_id, start_x[i], start_y[i],
                        MapWinTable[i]->zoom_offset_, 0x12, turn_frame);
                    MapWinTable[i]->update(&area[i], 0);
                    saved[i].copy(static_cast<Buffer *>(MapWinTable[i]),
                        start_x[i] - margin_x[i] - area[i].left,
                        start_y[i] - margin_y[i] - area[i].top,
                        start_x[i] - margin_x[i], start_y[i] - margin_y[i], width, height);
                }
            }
            DWORD now;
            do { do_non_input(); now = timeGetTime(); }
            while (static_cast<int>(now - previous_time) < delay / 2);
            previous_time = now;
        }
        // Original jumps straight to global-coordinate cleanup for this flag.
        if (flags & 1) {
            VehDrawPositionX = -1;
            VehDrawPositionY = -1;
            flush_input();
            return;
        }
        vehdraw_scoot_start(veh_id, direction, move_frames, 2);
        for (int move_frame = 0; move_frame < move_frames; ++move_frame) {
            for (int i = 0; i < 8; ++i) if (active[i]) {
                const int width = area[i].right - area[i].left;
                const int height = area[i].bottom - area[i].top;
                RECT moving;
                moving.top = static_cast<int>(position[i].y_);
                moving.left = static_cast<int>(position[i].x_);
                moving.right = moving.left + width; moving.bottom = moving.top + height;
                RECT intersection;
                if (IntersectRect(&intersection, &moving, &area[i])) {
                    // Convert again after IntersectRect, as the original does.
                    const int y = static_cast<int>(position[i].y_);
                    const int x = static_cast<int>(position[i].x_);
                    veh_draw(static_cast<Buffer *>(MapWinTable[i]), veh_id, x, y,
                        MapWinTable[i]->zoom_offset_, 0x22, move_frame);
                    MapWinTable[i]->update(&area[i], 0);
                    const int restore_y = static_cast<int>(position[i].y_);
                    const int restore_x = static_cast<int>(position[i].x_);
                    saved[i].copy(static_cast<Buffer *>(MapWinTable[i]),
                        restore_x - margin_x[i] - area[i].left,
                        restore_y - margin_y[i] - area[i].top,
                        restore_x - margin_x[i], restore_y - margin_y[i], width, height);
                }
                position[i] += increment[i];
            }
            DWORD now;
            do { do_non_input(); now = timeGetTime(); }
            while (static_cast<int>(now - previous_time) < delay);
            previous_time = now;
        }
        for (int i = 0; i < 8; ++i) position[i] -= increment[i];
    }
    if (!retain_origin) {
        VehDrawPositionX = next_x;
        VehDrawPositionY = next_y;
    }
    for (int final_slot = 0; final_slot < 8; ++final_slot) if (active[final_slot]) {
        veh_draw(static_cast<Buffer *>(MapWinTable[final_slot]), veh_id,
            end_x[final_slot], end_y[final_slot],
            MapWinTable[final_slot]->zoom_offset_, 2, 1);
        MapWinTable[final_slot]->soft_update(&area[final_slot]);
        do_all_draws();
    }
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
// ORIGINAL: 0x0055AF20 ?base_draw@@YAXPAUBuffer@@HHHHH@Z 0x0055AF20-0x0055B739
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0055af20.cpp retains the 2073-byte transcription and original span; the seed's Base, Map, Sprite, FontQueue and Buffer dependencies have typed product owners."], "reason": "Recover the base renderer's distant zoom badge, faction sprite, population caption, visibility check and riot marker through typed objects; compiler and runtime verification remain separate."}
// symbol    ?base_draw@@YAXPAVBuffer@@HHHHH@Z
void __cdecl base_draw(Buffer *target, int base_id, int pixel_x, int pixel_y,
                       int style, int flags) {
    Base &base = Bases[base_id];
    const int faction = base.faction_id_current;
    RECT rect;

    // At distant zoom, show the base as a colored lozenge rather than a
    // building sprite and population caption.
    if ((style < -8 || (flags & 0x100)) && !(flags & (8 | 0x800))) {
        int height;
        int width;
        int inset;
        if (MapWinCaptionScale < 1) {
            height = ((style * 5 + 80) * 10) / 16;
            width = ((style * 5 + 80) * 20) / 16;
            inset = (style * 4 + 64) / 16;
        } else {
            height = MapWinCaptionScale * 50 / MapWinCaptionDivisor;
            width = MapWinCaptionScale * 100 / MapWinCaptionDivisor;
            inset = MapWinCaptionScale * 4 / MapWinCaptionDivisor;
        }
        if (flags & 0x200) {
            rect.left = pixel_x - 2;
            rect.right = pixel_x + width + 2;
            rect.top = pixel_y - 2;
            rect.bottom = pixel_y + height + 2;
            inset = 2;
        } else {
            rect.left = pixel_x + inset;
            rect.right = pixel_x + width - inset;
            rect.top = pixel_y + inset;
            rect.bottom = pixel_y + height - inset;
        }
        const int horizontal_trim = (rect.right - rect.bottom - rect.left + rect.top) / 2;
        rect.left += horizontal_trim;
        rect.right -= horizontal_trim;
        const int vertical_shift = MapWinCaptionScale < 1
            ? (style * 3 + 48) * 2 / 16
            : MapWinCaptionScale * 6 / MapWinCaptionDivisor;
        rect.top += vertical_shift - inset;
        rect.bottom += vertical_shift - inset;
        const int border = strcmp(Players[faction].search_key, "SPARTANS") == 0 ? 44 : 0;
        target->box(&rect, border, border);
        ++rect.left;
        --rect.right;
        ++rect.top;
        --rect.bottom;
        target->fill(&rect, InfoFactionTextColors[faction]);
        return;
    }

    if (MapWinCaptionScale < 1) {
        SpriteScaleNumerator = style + 16;
        SpriteScaleDenominator = 16;
    } else {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    }

    const int population = base.population_size;
    int mode = 3;
    if (population < 4) {
        mode = 0;
    } else if (population < static_cast<int>(Rules.pop_limit_sans_hab_complex + 1 < 8
                                      ? 7 : Rules.pop_limit_sans_hab_complex + 1)) {
        mode = 1;
    } else if (population < static_cast<int>(Rules.pop_limit_sans_hab_dome + 1 < 11
                                      ? 10 : Rules.pop_limit_sans_hab_dome + 1)) {
        mode = 2;
    }
    int byte_index, bit;
    bitmask(1, &byte_index, &bit);
    if (base.facilities_built[byte_index] & bit && mode < 3) ++mode;
    int danger = 0;
    bitmask(4, &byte_index, &bit);
    if ((base.facilities_built[byte_index] & bit)
        || (GameDataStateStorage.fields_.secret_project_.citizens_defense_force() >= 0
            && Bases[GameDataStateStorage.fields_.secret_project_.citizens_defense_force()].faction_id_current == faction)) {
        danger = 1;
    }
    bitmask(5, &byte_index, &bit);
    if (base.facilities_built[byte_index] & bit) danger = 2;

    if (!(flags & 0x800)) {
        Map *tile = map_loc(base.x, base.y);
        const int terrain = (static_cast<unsigned char>(tile->bit) & 0xE0) < 0x60 ? 1 : 0;
        Sprite *sprite = FactionBaseSprites[base.faction_id_former][terrain][mode][danger];
        sprite->draw_dest(target, sprite->transparent_index(), pixel_x, pixel_y,
                          ShadowColorTable);
    }

    BaseDrawFont = g_FONTQUEUE_VAL1.get_zoom(style, 1, 3, 1);
    if (flags & 0x40) {
        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
        return;
    }

    char number[80];
    char measure_text[80] = "";
    char caption[80] = "";
    _itoa(population, number, 10);
    strcat(caption, number);
    strcat(measure_text, "8");
    if (population >= 10) strcat(measure_text, "8");
    rect.left = pixel_x;
    rect.right = pixel_x + BaseDrawFont->width(measure_text) + 5;
    rect.top = pixel_y;
    rect.bottom = pixel_y + BaseDrawFont->pixel_height() + 4;

    Map *tile = map_loc(base.x, base.y);
    int visible_occupier = -1;
    if (tile->bit & 2) {
        visible_occupier = tile->val2 & 15;
        if (visible_occupier > 7) visible_occupier = -1;
    }
    bool reveal = false;
    const int viewer = ConsoleGlobal.faction_id_;
    if (visible_occupier < 0) {
        reveal = (PlayersData[faction].diplo_treaties[viewer] & 1)
            || spying(faction)
            || (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
            || (PlayersData[viewer].flags & PFLAG_MAP_REVEALED)
            || faction == viewer;
        if (!reveal) {
            for (int direction = 0; direction < 8; ++direction) {
                int nx = base.x + VehMoveDeltaX[direction];
                if (!(MapIsFlat & 1)) {
                    if (nx < 0) nx += MapLongitudeBounds;
                    else if (nx >= MapLongitudeBounds) nx -= MapLongitudeBounds;
                }
                const int ny = base.y + VehMoveDeltaY[direction];
                if (ny >= 0 && ny < MapLatitudeBounds
                    && nx >= 0 && nx < MapLongitudeBounds) {
                    Map *neighbor = map_loc(nx, ny);
                    int occupant = -1;
                    if (neighbor->bit & 2) {
                        occupant = neighbor->val2 & 15;
                        if (occupant > 7) occupant = -1;
                    }
                    if (occupant == viewer) { reveal = true; break; }
                }
            }
        }
    }

    if (reveal) {
        rect.left = 0;
        rect.top = 0;
        target->box(&rect, 0, 0);
        ++rect.left;
        --rect.right;
        ++rect.top;
        --rect.bottom;
        target->set_font(BaseDrawFont, 0, 0, 0);
        target->set_text_color(ChatItemIdTable[faction], -1, 1, 1);
    } else {
        const int border = strcmp(Players[faction].search_key, "SPARTANS") == 0
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
    }
    target->write_l(caption, rect.bottom + 2, rect.top + 1, strlen(caption));
    --rect.left;
    ++rect.right;
    --rect.top;
    ++rect.bottom;
    if (base.state & BSTATE_DRONE_RIOTS_ACTIVE) {
        int icon_x = pixel_x + 2 + rect.right - rect.left;
        int icon_y = pixel_y;
        Sprite *icon = &g_RED_MALE_HEAD_ICON_SPRITE;
        if (Players[faction].rule_flags & RFLAG_ALIEN) {
            icon = &g_RED_ALIEN_HEAD_ICON_SPRITE;
            icon_x += 2;
            icon_y -= 6;
        }
        icon->draw(target, icon->transparent_index(), icon_x, icon_y);
    }
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
}
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

// These header-defined lifetime callbacks are emitted for the real
// Vector arrays in veh_scoot below, not forced by a measurement anchor.
// veh_scoot pairs this destructor with Vector construction for arrays of
// eight 12-byte elements, in both normal and exceptional cleanup paths.
// ORIGINAL: 0x0055B750 ??1Vector@@QAE@XZ 0x0055B750-0x0055B755 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x0055B750 tail-jumps to Vector::close at 0x006343D0; span agrees with git:99832567:src/unrecovered/0055b750.cpp historical seed.", "veh_scoot constructs two eight-element arrays of stride 12 using constructor 0x006343C0 and destructor 0x0055B750 at 0x0055A197-0x0055A1D9; normal and exceptional array cleanup reuse that destructor."], "reason": "Real Vector destructor calling close, replacing the seed-only synthetic receiver shim without removing its historical obligation."}
// LEVER: thiscall shim - a synthetic class exposes close() as a member so "call close(); ret" folds into the original's bare 5-byte jmp
// body      src/vector.h

/*
Purpose: Initialize a zero vector.
// ORIGINAL: 0x006343C0 ??0Vector@@QAE@XZ 0x006343C0-0x006343CD BYTE_EXACT
// body      src/vector.h
// size      13 bytes
// prototype void (__thiscall ??0Vector@@QAE@XZ)(Vector* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/

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
