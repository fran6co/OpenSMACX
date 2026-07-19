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
#include "graphicwin.h"
#include "buffer.h"

const uint32_t GraphicWinPrimaryVtable = 0x0066FC50;
const uint32_t GraphicWinBufferVtable = 0x0066FC48;

void __thiscall buffer_subobject_destructor(void *self) {
    // Source-owned: dispatches to the recovered Buffer destructor rather than
    // the original body at 0x005D7410.
    buffer_destructor_redirect(reinterpret_cast<Buffer *>(self), nullptr);
}

func_subobject_destructor *BufferSubobjectDestructor = &buffer_subobject_destructor;
func_subobject_destructor *WinOriginalDestructor =
    (func_subobject_destructor *)0x005EBC90;

namespace {

struct DestructorProbe {
    int buffer_calls;
    int win_calls;
    void *buffer_target;
    void *win_target;
    int order;
};

DestructorProbe Probe = {};

}  // namespace

void graphic_win_destructor_probe_reset() {
    Probe = DestructorProbe();
}

int graphic_win_destructor_probe_buffer_calls() { return Probe.buffer_calls; }
int graphic_win_destructor_probe_win_calls() { return Probe.win_calls; }
void *graphic_win_destructor_probe_buffer_target() { return Probe.buffer_target; }
void *graphic_win_destructor_probe_win_target() { return Probe.win_target; }
int graphic_win_destructor_probe_order() { return Probe.order; }

/*
Purpose: Destroy a GraphicWin by installing the original virtual tables,
         clearing the trailing field, and destroying the Buffer subobject
         before the Win base.
Original Offset: 005D4DD0
Status: Complete with temporary Buffer and Win subobject dependencies
*/
GraphicWin *__fastcall graphic_win_destructor_redirect(GraphicWin *self, void *) {
    // The legacy body computes the Buffer subobject with a neg/sbb/and null
    // guard on the instance pointer, so a null instance stores nothing and
    // delegates nowhere.
    const uintptr_t base = reinterpret_cast<uintptr_t>(self);
    if (!base) {
        return self;
    }
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(base);
    ordered[0x000 / 4] = GraphicWinPrimaryVtable;
    ordered[0x444 / 4] = GraphicWinBufferVtable;
    ordered[0xA10 / 4] = 0;

    void *const buffer_subobject = reinterpret_cast<void *>(base + 0x444);
    Probe.buffer_target = buffer_subobject;
    Probe.buffer_calls++;
    Probe.order = (Probe.order << 4) | 2;
    if (BufferSubobjectDestructor) {
        BufferSubobjectDestructor(buffer_subobject);
    }

    void *const win_subobject = reinterpret_cast<void *>(base);
    Probe.win_target = win_subobject;
    Probe.win_calls++;
    Probe.order = (Probe.order << 4) | 1;
    if (WinOriginalDestructor) {
        WinOriginalDestructor(win_subobject);
    }
    return self;
}
