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
// ORIGINAL: 0x00559230 _$E21 0x00559230-0x00559246 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x0068263C selects initializer0x00559230-0x00559246 and its registration names fini0x00559250-0x0055925A; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FB88; FontQueue constructor/destructor559290/55B740. Source object binding names ?g_FONTQUEUE_VAL2@@3VFontQueue@@A.", "Fresh owner COFF selects _$E21 via .CRT$XCU and registers _$E19, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original startup function to the genuine compiler-generated lifetime of source-owned g_FONTQUEUE_VAL2; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E8
// ORIGINAL: 0x00559250 _$E19 0x00559250-0x0055925A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x0068263C selects initializer0x00559230-0x00559246 and its registration names fini0x00559250-0x0055925A; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FB88; FontQueue constructor/destructor559290/55B740. Source object binding names ?g_FONTQUEUE_VAL2@@3VFontQueue@@A.", "Fresh owner COFF selects _$E21 via .CRT$XCU and registers _$E19, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original exit function to the genuine compiler-generated lifetime of source-owned g_FONTQUEUE_VAL2; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E6
// ORIGINAL: 0x00559260 _$E26 0x00559260-0x00559276 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682640 selects initializer0x00559260-0x00559276 and its registration names fini0x00559280-0x0055928A; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FAE8; FontQueue constructor/destructor559290/55B740. Source object binding names ?g_FONTQUEUE_VAL1@@3VFontQueue@@A.", "Fresh owner COFF selects _$E26 via .CRT$XCU and registers _$E24, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original startup function to the genuine compiler-generated lifetime of source-owned g_FONTQUEUE_VAL1; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E13
// ORIGINAL: 0x00559280 _$E24 0x00559280-0x0055928A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x00682640 selects initializer0x00559260-0x00559276 and its registration names fini0x00559280-0x0055928A; complete instruction decode fixes both ends.", "Original operations identify storage0x0093FAE8; FontQueue constructor/destructor559290/55B740. Source object binding names ?g_FONTQUEUE_VAL1@@3VFontQueue@@A.", "Fresh owner COFF selects _$E26 via .CRT$XCU and registers _$E24, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original exit function to the genuine compiler-generated lifetime of source-owned g_FONTQUEUE_VAL1; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E11
#include "stdafx.h"
#include "interface_fonts.h"
#include "font_queue.h"
#include "mapwin.h"

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

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: `int foundPos=-1` sentinel search vs a `while` loop with an early `goto`-style break matching the disassembly's `jl` dead branch (SF!=OF on an always-nonnegative counter) - landed with the sentinel form, MISMATCH at #2 already (loop-entry `cmp` vs the original's cached `mov`).
// ORIGINAL: 0x005592D0 ?get_zoom@FontQueue@@QAEHHHHH@Z 0x005592D0-0x005593D9
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5592D0..5593D9 computes signed scale and height in two division stages, substitutes11 for10, and reads the shared691E6C/70 override only when its numerator is positive. The constructor559290 establishes three Font objects of stride28 followed by height/style/order arrays at78/84/90.", "Original559334..559374 rotates the oldest slot, publishes keys before calling actual Font::init618F40 with the source-owned DefaultFontFace, ignores the status result and returns the same Font pointer. Original559382..5593D9 promotes a cache hit without initialization; missing order entries leave the order unchanged. Retain the unclaimed seed and catalogue signature as historical evidence."], "reason": "Recover the three-slot font-height/style LRU as the existing FontQueue owner, returning its actual Font member rather than a synthetic integer address."}
// symbol    ?get_zoom@FontQueue@@QAEPAVFont@@HHHH@Z
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
            for (int position = 0; position < 3; ++position) {
                if (lru_slots_[position] == slot) {
                    for (int next = position; next < 2; ++next) {
                        lru_slots_[next] = lru_slots_[next + 1];
                    }
                    lru_slots_[2] = slot;
                    break;
                }
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
