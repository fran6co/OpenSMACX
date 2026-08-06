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
#include "original_seam.h"
#include "planwin.h"
#include "mapwin.h"
#include <cstring>

/*
Purpose: Clear the plan window's line count.
Original Offset: 0048AF20
Return Value: n/a
Status: Complete
*/
void PlanWin::clear_lines() {
    field_21FF8_ = 0;
}

void __fastcall plan_win_clear_lines_redirect(PlanWin *self, void *) {
    self->clear_lines();
}

/*
Purpose: Close the plan window by dropping its blink state and running the
         close it inherits from MapWin.
Original Offset: 0048BC50
Return Value: n/a
Status: Complete

The original clears the field and tail-jumps into MapWin::close with `this`
untouched, which is a plain base-class call: the MapWin subobject opens a
PlanWin at offset 0. It cannot be spelled as inheritance here because MapWin
holds its virtual base as a member at MapWin's own 0x21A6C, while a PlanWin
puts that base at 0x22050 - but MapWin::close never uses the member, reaching
the base through the vbtable instead, so it reads the right one either way.
*/
void PlanWin::close() {
    field_21A68_ = 0;
    reinterpret_cast<MapWin *>(this)->close();
}

void __fastcall plan_win_close_redirect(PlanWin *self, void *) {
    self->close();
}

namespace {

// blink dispatches through the virtual base's own vtable rather than a C++
// virtual call, so no vtable this toolchain lays out can disagree with the
// original's. Slot 0x30 is read from whatever object is actually there.
typedef void (OriginalObject::*func_base_vtable_slot)();

constexpr size_t PlanWinBlinkSlot = 0x30;

// Both methods reach the virtual base the way the original does - through the
// vbtable rather than through the member - so they stay correct for any class
// that inherits this one and moves the base.
uint8_t *virtual_base_of(void *self) {
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    return reinterpret_cast<uint8_t *>(self) + vbtable[1];
}

}  // namespace

/*
Purpose: Toggle the plan window's blink phase and redraw, when blinking is on.
Original Offset: 0048BC20
Return Value: n/a
Status: Complete
*/
void PlanWin::blink() {
    if (!field_21A68_) {
        return;
    }
    field_21A6C_ = (field_21A6C_ == 0) ? 1 : 0;
    uint8_t *const base = virtual_base_of(this);
    void **const vtable = *reinterpret_cast<void ***>(base);
    reinterpret_cast<func_base_vtable_slot *>(
        vtable[PlanWinBlinkSlot / sizeof(void *)])(base);
}

void __fastcall plan_win_blink_redirect(PlanWin *self, void *) {
    self->blink();
}

/*
Purpose: Restore the window's backing image from the plan window's own buffer.
Original Offset: 0048B3C0
Return Value: n/a
Status: Complete

The height is passed negated. That is the original's, not a transcription
slip: it loads the buffer's field at 0x84 and runs `neg edx` before pushing.
*/
void PlanWin::UNK1() {
    uint8_t *const base = virtual_base_of(this);
    auto *const window_buffer = reinterpret_cast<Buffer *>(base + 0x444);
    int32_t width;
    int32_t height;
    std::memcpy(&width, base + 0x4C4, sizeof(width));
    std::memcpy(&height, base + 0x4C8, sizeof(height));
    window_buffer->copy(reinterpret_cast<Buffer *>(buffer_), 0, 0, width,
                        -height);
}

void __fastcall plan_win_unk1_redirect(PlanWin *self, void *) {
    self->UNK1();
}
