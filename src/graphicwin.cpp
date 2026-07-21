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

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall buffer_subobject_destructor(void *self) {
    // Source-owned: dispatches to the recovered Buffer destructor rather than
    // the original body at 0x005D7410.
    buffer_destructor_redirect(reinterpret_cast<Buffer *>(self), nullptr);
}

void __thiscall buffer_subobject_close(void *self) {
    reinterpret_cast<Buffer *>(self)->close();
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

func_subobject_destructor *BufferSubobjectDestructor = &buffer_subobject_destructor;
func_subobject_destructor *WinOriginalDestructor =
    (func_subobject_destructor *)0x005EBC90;
func_subobject_close *BufferSubobjectClose = &buffer_subobject_close;
func_subobject_close *WinOriginalClose = (func_subobject_close *)0x005EB640;
uint32_t *GraphicWinFieldA0CDefault = (uint32_t *)0x009B33C0;

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

namespace {

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef uint32_t(__thiscall *func_scalar_deleting_destructor)(
    void *, uint32_t);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

}  // namespace

/*
Purpose: Close a GraphicWin by closing its Win base and Buffer subobject,
         resetting its window-specific state, and deleting the trailing
         owned interface when present.
Original Offset: 005D4E40
Return Value: field_A0C_ default when no interface is present; otherwise the
              scalar-deleting virtual call's return residue
Status: Complete with temporary Win close dependency
*/
uint32_t GraphicWin::close() {
    if (WinOriginalClose) {
        WinOriginalClose(this);
    }
    if (BufferSubobjectClose) {
        BufferSubobjectClose(reinterpret_cast<uint8_t *>(this) + 0x444);
    }

    volatile uint32_t *const ordered =
        reinterpret_cast<volatile uint32_t *>(this);
    void *const release_target = reinterpret_cast<void *>(
        static_cast<uintptr_t>(ordered[0xA08 / 4]));
    ordered[0xA10 / 4] = 0;
    ordered[0x134 / 4] = 0;
    ordered[0x138 / 4] = 0;
    ordered[0x9CC / 4] = 0;
    ordered[0x9D0 / 4] = 0;
    ordered[0x9D4 / 4] = 0;
    ordered[0x9D8 / 4] = 0;
    ordered[0x9DC / 4] = 0;
    ordered[0x9E0 / 4] = 0;
    ordered[0x9E4 / 4] = 0;
    ordered[0x9E8 / 4] = 0;
    ordered[0x9EC / 4] = 0;
    ordered[0x9F0 / 4] = 0;
    ordered[0x9F4 / 4] = 0;
    ordered[0x9F8 / 4] = 0;
    ordered[0x9FC / 4] = 0;
    ordered[0xA00 / 4] = 0;
    ordered[0xA04 / 4] = 0;
    const uint32_t default_value = *GraphicWinFieldA0CDefault;
    ordered[0xA0C / 4] = default_value;
    if (!release_target) {
        return default_value;
    }

    void **const vtable = *reinterpret_cast<void ***>(release_target);
    const uint32_t result = reinterpret_cast<func_scalar_deleting_destructor>(
        vtable[0])(release_target, 1);
    ordered[0xA08 / 4] = 0;
    return result;
}

uint32_t __fastcall graphic_win_close_redirect(GraphicWin *self, void *) {
    return self->close();
}

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
