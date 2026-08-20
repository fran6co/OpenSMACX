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
#include "graphicwin.h"
#include "buffer.h"
#include "scroll.h"
#include "vector_teardown.h"
#include "temp.h"  // HandleMain, the window the invalidate targets

#include <cstring>

const uint32_t GraphicWinPrimaryVtable = 0x0066FC50;
const uint32_t GraphicWinBufferVtable = 0x0066FC48;

void __fastcall buffer_subobject_destructor(void *self) {
    // Source-owned: dispatches to the recovered Buffer destructor rather than
    // the original body at 0x005D7410.
    buffer_destructor_redirect(reinterpret_cast<Buffer *>(self), nullptr);
}

void __fastcall buffer_subobject_close(void *self) {
    reinterpret_cast<Buffer *>(self)->close();
}

func_subobject_destructor BufferSubobjectDestructor = original_method<func_subobject_destructor>(
    reinterpret_cast<unsigned long>(&buffer_subobject_destructor));
func_subobject_destructor WinOriginalDestructor =
    original_method<func_subobject_destructor>(0x005EBC90);
func_subobject_close BufferSubobjectClose = original_method<func_subobject_close>(
    reinterpret_cast<unsigned long>(&buffer_subobject_close));
func_subobject_close WinOriginalClose = original_method<func_subobject_close>(0x005EB640);
uint32_t *GraphicWinFieldA0CDefault = (uint32_t *)0x009B33C0;

/*
Purpose: Construct the Win base and Buffer subobject, then install GraphicWin
         tables and initialize its trailing window state.
// ORIGINAL: 0x005D4CF0 ??0GraphicWin@@QAE@XZ 0x005D4CF0-0x005D4DC4;0x00662B10-0x00662B22
// size      230 bytes
// prototype void (__thiscall ??0GraphicWin@@QAE@XZ)(GraphicWin* this)
// callers   52   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x005EB3D0
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
Verification note: the 0x134 and 0x138 stores are redundant - Win::construct
runs first and already zeroes both - so dropping either is unobservable. They
mirror the original's instruction sequence rather than deduplicating it. The
Win/Buffer construction order is likewise unobservable because the two write
disjoint regions.
*/
void GraphicWin::construct() {
    static_cast<Win *>(this)->construct();
    new (&buffer_) Buffer();
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = GraphicWinPrimaryVtable;
    object[0x444 / 4] = GraphicWinBufferVtable;
    object[0xA10 / 4] = 0;
    object[0x134 / 4] = 0;
    object[0x138 / 4] = 0;
    for (size_t offset = 0x9CC; offset <= 0xA08; offset += 4) {
        object[offset / 4] = 0;
    }
    object[0xA0C / 4] = *GraphicWinFieldA0CDefault;
}

GraphicWin *__fastcall graphic_win_construct_redirect(
        GraphicWin *self, void *) {
    self->construct();
    return self;
}

namespace {

struct DestructorProbe {
    int buffer_calls;
    int win_calls;
    void *buffer_target;
    void *win_target;
    int order;
};

DestructorProbe Probe = {0};

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

typedef uint32_t (OriginalObject::*func_scalar_deleting_destructor)(uint32_t);

}  // namespace

/*
Purpose: Close a GraphicWin by closing its Win base and Buffer subobject,
         resetting its window-specific state, and deleting the trailing
         owned interface when present.
// ORIGINAL: 0x005D4E40 ?close@GraphicWin@@QAEXXZ 0x005D4E40-0x005D4EE6
// symbol    ?close@GraphicWin@@QAEIXZ
// size      166 bytes
// prototype void (__thiscall ?close@GraphicWin@@QAEXXZ)(GraphicWin* this)
// callers   66   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005EB640
// indirect  0x005D4EDB
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original Win close dependency
Return Value: field_A0C_ default when no interface is present; otherwise the
              scalar-deleting virtual call's return residue
Status: Complete with temporary Win close dependency
*/
uint32_t GraphicWin::close() {
    if (WinOriginalClose) {
        (ORIGINAL(this)->*WinOriginalClose)();
    }
    if (BufferSubobjectClose) {
        (ORIGINAL(reinterpret_cast<uint8_t *>(this) + 0x444)->*BufferSubobjectClose)();
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
    const uint32_t result = (ORIGINAL(release_target)->*original_method<func_scalar_deleting_destructor>(reinterpret_cast<unsigned long>(vtable[0])))(1);
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
// ORIGINAL: 0x005D4DD0 ??1GraphicWin@@QAE@XZ 0x005D4DD0-0x005D4E37;0x00662B22-0x00662B34
// size      121 bytes
// prototype void (__thiscall ??1GraphicWin@@QAE@XZ)(GraphicWin* this)
// callers   185   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7410 0x005EBC90
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
        (ORIGINAL(buffer_subobject)->*BufferSubobjectDestructor)();
    }

    void *const win_subobject = reinterpret_cast<void *>(base);
    Probe.win_target = win_subobject;
    Probe.win_calls++;
    Probe.order = (Probe.order << 4) | 1;
    if (WinOriginalDestructor) {
        (ORIGINAL(win_subobject)->*WinOriginalDestructor)();
    }
    return self;
}

/*
Purpose: Fill a rectangle in the window's own buffer.
// ORIGINAL: 0x005D5440 ?fill@GraphicWin@@QAEHHHHHH@Z 0x005D5440-0x005D5467
// size      39 bytes
// prototype int (__thiscall ?fill@GraphicWin@@QAEHHHHHH@Z)(GraphicWin* this, int xLeft, int yTop, int length, int width, int)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8240
Return Value: whatever Buffer::fill returns
Status: Complete
*/
func_buffer_fill BufferOriginalFill = original_method<func_buffer_fill>(0x005D8240);

int GraphicWin::fill(int x1, int y1, int x2, int y2, int color) {
    return (ORIGINAL(reinterpret_cast<uint8_t *>(this) + 0x444)->*BufferOriginalFill)(x1, y1, x2, y2, color);
}

int __fastcall graphic_win_fill_redirect(GraphicWin *self, void *,
                                         int x1, int y1, int x2, int y2,
                                         int color) {
    return self->fill(x1, y1, x2, y2, color);
}

// Slot 0xF4 on the parent's vtable. The stock body at 0x004042B0 is
// `mov eax, ecx; ret`, so a non-null parent answers with itself.
typedef void * (OriginalObject::*func_graphic_win_parent_query)();
// Virtual slot 0x30, the window's own paint.
typedef void (OriginalObject::*func_graphic_win_paint)();

func_graphic_win_buffer_fill_color BufferOriginalFillColor =
    original_method<func_graphic_win_buffer_fill_color>(0x005DFB50);
func_graphic_win_map_colors BufferOriginalMapColors =
    original_method<func_graphic_win_map_colors>(0x005DA330);
func_graphic_win_overlay_nonclient GraphicWinOverlayNonclient =
    original_method<func_graphic_win_overlay_nonclient>(0x005D6AC0);
void **GraphicWinColorMapTable = reinterpret_cast<void **>(0x009B3390);
// USER32!InvalidateRect, read out of the executable's import table. This is
// resolved on FIRST USE rather than by a dynamic initializer, because the
// initializer runs in every binary that links this translation unit and only
// one of them has anything mapped at 0x00669304. In the host test executables
// the address lands past the end of the image, and whether the load faults
// depends on what the loader happened to place after it - measured on
// recovery-gameplay-tests, the same read survived at one binary size and took
// the process down with an unhandled page fault before main at another. The
// variable stays writable so tests can still rebind it; a test that installs
// its own hook never reaches the resolve.
func_graphic_win_invalidate_rect *GraphicWinInvalidateRect = nullptr;

const uintptr_t GraphicWinInvalidateRectImport = 0x00669304;

/*
Purpose: Paint the window's surface in one colour. A window that is marked
         transparent and whose parent agrees copies the parent's pixels in
         instead, then remaps them through the process colour table.
// ORIGINAL: 0x005D5250 ?fill@GraphicWin@@QAEXH@Z 0x005D5250-0x005D5346
// size      246 bytes
// prototype void (__thiscall ?fill@GraphicWin@@QAEXH@Z)(GraphicWin* this, int)
// callers   58   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DA330 0x005DFB50 0x005DFF00
// indirect  0x005D5277
Return Value: n/a
Status: Complete
Verification note: three loads the original performs are deliberately absent.
         It copies a RECT at 0x14C into stack scratch and then overwrites that
         scratch with the one at 0x13C before either is read, so the first
         pair (0x154, 0x158) is dead; only the two sums that reach the blit
         survive here. It also emits `test esi, esi` at 0x005D52BC and passes
         a null source buffer on the false arm - unreachable, because `this`
         cannot be null on a thiscall, so that arm is not transcribed. A
         reader diffing against the disassembly will see both differences.
*/
void GraphicWin::fill(int color) {
    uint8_t *const object = reinterpret_cast<uint8_t *>(this);
    uint8_t *const surface = object + 0x444;
    uint32_t flags;
    std::memcpy(&flags, object + 0x98, sizeof(flags));
    uint8_t *parent;
    std::memcpy(&parent, object + 0xC4, sizeof(parent));
    // Bit 19 alone does not make the window transparent: the parent has to
    // exist and has to answer its slot 0xF4 with a nonzero value. The stock
    // implementation at 0x004042B0 is `mov eax, ecx; ret`, so a non-null
    // parent answers with itself and the copy path is the default.
    bool transparent = false;
    if ((flags & 0x80000) != 0 && parent != nullptr) {
        uintptr_t *const parent_vtable =
            *reinterpret_cast<uintptr_t **>(parent);
        func_graphic_win_parent_query const query =
            original_method<func_graphic_win_parent_query>(
                parent_vtable[0xF4 / 4]);
        transparent = (ORIGINAL(parent)->*query)() != nullptr;
    }
    if (!transparent) {
        (ORIGINAL(surface)->*BufferOriginalFillColor)(color);
        return;
    }
    int32_t outer_x, outer_y, inner_x, inner_y;
    std::memcpy(&outer_x, object + 0x14C, sizeof(outer_x));
    std::memcpy(&outer_y, object + 0x150, sizeof(outer_y));
    std::memcpy(&inner_x, object + 0x13C, sizeof(inner_x));
    std::memcpy(&inner_y, object + 0x140, sizeof(inner_y));
    int32_t width, height;
    std::memcpy(&width, object + 0x4C4, sizeof(width));
    std::memcpy(&height, object + 0x4C8, sizeof(height));
    reinterpret_cast<Buffer *>(parent + 0x444)->copy(
        reinterpret_cast<Buffer *>(surface), outer_x + inner_x,
        outer_y + inner_y, 0, 0, width, -height);
    void *const table = *GraphicWinColorMapTable;
    if (table == nullptr) {
        return;
    }
    // Re-read, do NOT hoist onto the pair above: the original reloads both at
    // 0x005D5302 and 0x005D5308, so a blit that resized the surface is seen
    // by the remap. The bounds are inclusive, hence width-1 and -1-height.
    std::memcpy(&width, object + 0x4C4, sizeof(width));
    std::memcpy(&height, object + 0x4C8, sizeof(height));
    (ORIGINAL(surface)->*BufferOriginalMapColors)(0, 0, width - 1, -1 - height, table);
}

void __fastcall graphic_win_fill_color_redirect(GraphicWin *self, void *,
                                                int color) {
    self->fill(color);
}

/*
Purpose: Repaint the window and invalidate the screen area it occupies. A
         window already inside a redraw is skipped, so a paint hook that
         redraws again cannot recurse.
// ORIGINAL: 0x005D5A70 ?redraw@GraphicWin@@QAEXXZ 0x005D5A70-0x005D5B64
// size      244 bytes
// prototype void (__thiscall ?redraw@GraphicWin@@QAEXXZ)(GraphicWin* this)
// callers   42   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D6AC0 0x005ED240 0x005F7E90
// indirect  0x005D5AA9 0x005D5AAF 0x005D5B59
Return Value: n/a
Status: Complete
Verification note: the calling convention of the 0xA10 paint hook is inferred,
         not decoded. It is reached by a bare `call eax` at 0x005D5AA9 with
         nothing pushed and ECX not set, which rules out __thiscall but leaves
         zero-argument __cdecl and __stdcall indistinguishable - they are
         ABI-identical here, so either spelling works and neither is a
         callee-pop fact.
*/
void GraphicWin::redraw() {
    uint8_t *const object = reinterpret_cast<uint8_t *>(this);
    if (HandleMain == nullptr) {
        return;
    }
    uint32_t state;
    std::memcpy(&state, object + 0x1A0, sizeof(state));
    if ((state & 1) != 0) {
        return;
    }
    state |= 1;
    std::memcpy(object + 0x1A0, &state, sizeof(state));
    *ScrollCurrentWin = static_cast<Win *>(this);

    func_graphic_win_paint_hook *hook;
    std::memcpy(&hook, object + 0xA10, sizeof(hook));
    if (hook != nullptr) {
        hook();
    }
    uintptr_t *const vtable = *reinterpret_cast<uintptr_t **>(object);
    func_graphic_win_paint const paint =
        original_method<func_graphic_win_paint>(vtable[0x30 / 4]);
    (ORIGINAL(this)->*paint)();
    (ORIGINAL(this)->*GraphicWinOverlayNonclient)(nullptr);

    // Re-read, do NOT reuse the latched value: the original reloads at
    // 0x005D5ABB before clearing bit 0, so any bit the paint hook or the
    // virtual paint set on 0x1A0 survives this clear.
    std::memcpy(&state, object + 0x1A0, sizeof(state));
    state &= 0xFFFFFFFEU;
    std::memcpy(object + 0x1A0, &state, sizeof(state));

    if (!static_cast<Win *>(this)->is_visible()) {
        return;
    }
    RECT area;
    std::memcpy(&area, object + 0x474, sizeof(area));
    int x_offset = 0;
    int y_offset = 0;
    static_cast<Win *>(this)->client_to_screen(&x_offset, &y_offset);
    area.left += x_offset;
    area.right += x_offset;
    area.top += y_offset;
    area.bottom += y_offset;
    // The window handle is read again here rather than reused from the guard
    // above, matching the original's second load at 0x005D5B43.
    if (!GraphicWinInvalidateRect) {
        GraphicWinInvalidateRect = *reinterpret_cast<func_graphic_win_invalidate_rect **>(
            GraphicWinInvalidateRectImport);
    }
    GraphicWinInvalidateRect(HandleMain, &area, FALSE);
}

void __fastcall graphic_win_redraw_redirect(GraphicWin *self, void *) {
    self->redraw();
}

// GraphicWin::init's four remaining original dependencies. Win::init carries
// the whole window-creation closure, compute_min_size is frame arithmetic,
// nonclient_to_client converts an outer size to a client size in place, and
// Buffer::init is DirectDraw/GDI surface creation; all four stay at their
// original addresses until that closure is source-owned. Tests rebind them.
func_win_init WinOriginalInit = original_method<func_win_init>(0x005EBD80);
func_graphic_win_compute_min_size GraphicWinOriginalComputeMinSize =
    original_method<func_graphic_win_compute_min_size>(0x005D7030);
func_win_nonclient_to_client WinOriginalNonclientToClient =
    original_method<func_win_nonclient_to_client>(0x005EEF60);
func_buffer_init BufferOriginalInit = original_method<func_buffer_init>(0x005D7670);

uint32_t *GraphicWinInitDefaults = reinterpret_cast<uint32_t *>(0x009B3394);

/*
Purpose: Initialise a GraphicWin. Reset the window, republish the eleven
         process window defaults when the style asks for them, optionally
         allocate the owned Buffer the window parks at 0xA08, initialise the
         Win base, then size and initialise the window's own drawing surface
         and sync it to the active palette.
// ORIGINAL: 0x005D4EF0 ?init@GraphicWin@@QAEHHHHHPADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z 0x005D4EF0-0x005D5096;0x00662B34-0x00662B49
// symbol    ?init@GraphicWin@@QAEHHHHHPADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z
// CORRECTED from ?init@GraphicWin@@QAEXHHHHPADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z
//   BaseButton::init calls it at 0x006072A2 and immediately tests the
//   result (`test eax, eax` / `jne` at 0x006072A7); all three exits set
//   EAX deliberately - the Win::init passthrough at 0x005D500E, the
//   Buffer::init passthrough at 0x005D5071, and `xor eax, eax` at
//   0x005D5081. The independent IDA prototype already reads int, so only
//   the symbol string is stale
// size      443 bytes
// prototype int (__thiscall ?init@GraphicWin@@QAEHHHHHPADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z)(GraphicWin* this, int, int, int, int, int8*, int, Win*, Menu*, BorderSizing*)
// callers   47   call targets   9
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x005D7030 0x005D7210 0x005D7670 0x005DE8F0 0x005EBD80 0x005EEF60 0x0064557F 0x0064558A
Return Value: 0 on success; otherwise the nonzero failure code passed straight
              through from Win::init or from Buffer::init
Status: Complete with temporary Win init/nonclient_to_client, GraphicWin
        compute_min_size and Buffer init dependencies
Verification note: this function returns int, and the catalogue used to spell
        the name with an X - void. BaseButton::init calls it at 0x006072A2 and
        immediately does `test eax, eax` / `jne 0x0060734C` at 0x006072A7, and
        all three exits set EAX deliberately: the Win::init passthrough (`jne
        0x005D5083` at 0x005D500E), the Buffer::init passthrough (`jne
        0x005D5083` at 0x005D5071), and `xor eax, eax` at 0x005D5081. The
        independent IDA prototype already read `int (__thiscall ...)`, so only
        the symbol string was stale.
        CORRECTED 2026-08-14 in tools/catalogue_corrections.py, and the
        disagreement it came from is now a gate check, `return-agreement`.
        This note asked its readers not to "fix" the return type back to void
        on the strength of the name, and for as long as that was all it was,
        the emitter went on declaring void to 47 callers - none of which could
        then capture the failure code. A fact stated in a comment is one only a
        human re-reader can act on.
Verification note: three stores in the original have no counterpart here
        because they belong to the omitted C++ EH frame - the operator-new
        pointer spilled into the caller's sixth argument slot at 0x005D4FB3,
        and the two EH state writes at 0x005D4FB9 and 0x005D4FCE. The only
        reader of that spill is the unwind funclet at 0x00662B34, which runs
        operator delete on it; the handler pushed at 0x005D4EF8 is a
        `mov eax, <FuncInfo>` / `jmp __CxxFrameHandler` thunk, so the frame is
        omittable. The style argument itself survives the clobber because the
        original latched it into EDI at 0x005D4F0E, before the spill.
Verification note: the emission order of the eleven default stores is
        transcribed but is not observable - eleven distinct fields from eleven
        distinct table slots, so any permutation of the statements leaves the
        same final state. Do not read the sweep's clean result as pinning that
        order: it generates no swap mutants inside this block at all (only two
        for the whole function, neither of them here), so the order is
        untested rather than tested-and-equivalent. What IS observable, and
        what the fixture pins with eleven distinct sentinels, is the
        slot-to-field mapping, which is not the identity.
Verification note: the original has no null check on title, parent, menu,
        border or on `this`, and none on the operator-new result beyond
        skipping the constructor. Their absence is deliberate.
*/
int GraphicWin::init(int x, int y, int width, int height, LPSTR title,
                     int flags, Win *parent, Menu *menu,
                     BorderSizing *border) {
    // Direct call at 0x005D4F09, not a virtual dispatch, and its return is
    // discarded: init on a live window tears the old one down first.
    close();

    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);

    // One `test edi, 0x30000000` at 0x005D4F12 covers both style bits: either
    // one republishes the whole block.
    if ((flags & 0x30000000) != 0) {
        volatile uint32_t *const defaults =
            reinterpret_cast<volatile uint32_t *>(GraphicWinInitDefaults);
        // The most error-prone part of the function, because the table slot
        // and the destination field are two different permutations. Read
        // straight off the stores at 0x005D4F1E through 0x005D4F98, the
        // slot-to-field map is
        //   [0]->0x9CC  [1]->0x9D0  [2]->0x9D4  [3]->0x9D8
        //   [4]->0x9E8  [5]->0x9E4  [6]->0x9DC  [7]->0x9E0
        //   [8]->0x9F0  [9]->0x9EC  [10]->0x9F4
        // so slots 4..7 land on 0x9E8/0x9E4/0x9DC/0x9E0 rather than in
        // ascending order, and 8/9 are crossed. The statement order below is
        // additionally the original's own, which visits the slots
        // 0, 2, 1, 3, 4, 5, 6, 7, 9, 8, 10 - note that the 1/2 inversion is
        // scheduling only, since both of those slots map to their own field.
        object[0x9CC / 4] = defaults[0];
        object[0x9D4 / 4] = defaults[2];
        object[0x9D0 / 4] = defaults[1];
        object[0x9D8 / 4] = defaults[3];
        object[0x9E8 / 4] = defaults[4];
        object[0x9E4 / 4] = defaults[5];
        object[0x9DC / 4] = defaults[6];
        object[0x9E0 / 4] = defaults[7];
        object[0x9EC / 4] = defaults[9];
        object[0x9F0 / 4] = defaults[8];
        object[0x9F4 / 4] = defaults[10];
    }

    // 0x588 is sizeof(Buffer). The allocation goes through the executable's
    // own operator new at 0x0064558A, already bound for the Wave work - the
    // address is what matters, not the name - and the legacy body stores
    // whatever it got, null included, at 0xA08. It does not check that 0xA08
    // was empty first; close() has just cleared it if it held anything.
    if ((flags & 0x40000000) != 0) {
        void *const block = WaveOperatorNew(0x588);
        Buffer *owned = nullptr;
        if (block != nullptr) {
            owned = new (block) Buffer();
        }
        object[0xA08 / 4] =
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(owned));
    }

    // All nine arguments go straight through in order.
    const int base_result = (ORIGINAL(this)->*WinOriginalInit)(x, y, width, height, title, flags, parent, menu, border);
    if (base_result != 0) {
        return base_result;
    }

    // The window's own Buffer keeps a back pointer to the window in its field
    // at 0x4, written as `mov [esi+0x448], esi` at 0x005D5012 - before the
    // minimum-size computation, in the original's order.
    object[0x448 / 4] =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this));
    (ORIGINAL(this)->*GraphicWinOriginalComputeMinSize)();

    if ((flags & 0x800) == 0) {
        // Converts the outer size to a client size in place. What the callee
        // subtracts is its own business and is not asserted here.
        (ORIGINAL(this)->*WinOriginalNonclientToClient)(&width, &height);
    } else {
        // The 0x800 arm goes the other way and ADDS the process scrollbar
        // thickness, pairing bit 3 of the 0x98 flag dword with the width and
        // bit 2 with the height. Both the flag dword and the thickness are
        // loaded once, at 0x005D5038 and 0x005D503E, ahead of either test;
        // the original tests only AL, which is equivalent for bits 2 and 3.
        const uint32_t nonclient_flags = object[0x98 / 4];
        const int thickness = ScrollDefaultThickness;
        if ((nonclient_flags & 8) != 0) {
            width += thickness;
        }
        if ((nonclient_flags & 4) != 0) {
            height += thickness;
        }
    }

    const int surface_result =
        (ORIGINAL(&buffer_)->*BufferOriginalInit)(width, height, 0, nullptr);
    if (surface_result != 0) {
        return surface_result;
    }
    // Return discarded: the original zeroes EAX at 0x005D5081 straight after.
    buffer_.sync_to_palette(*WinActivePalette);
    return 0;
}

int __fastcall graphic_win_init_redirect(GraphicWin *self, void *,
                                         int x, int y, int width, int height,
                                         LPSTR title, int flags, Win *parent,
                                         Menu *menu, BorderSizing *border) {
    return self->init(x, y, width, height, title, flags, parent, menu, border);
}

/*
// ORIGINAL: 0x005D6310 ?on_mouse_move@GraphicWin@@QAEXHHIH@Z 0x005D6310-0x005D632C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?on_mouse_move@GraphicWin@@QAEXHHIH@Z)(GraphicWin* this, int, int, unsigned int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F6320
Status: Complete
*/
void GraphicWin::on_mouse_move(int a1, int a2, unsigned int a3, int a4) {
    reinterpret_cast<Win *>(this)->on_mouse_move(a1, a2, a3, a4);
}

func_graphic_win_soft_update GraphicWinOriginalSoftUpdate =
    original_method<func_graphic_win_soft_update>(0x005D5890);

/*
Purpose: Blit the window's own buffer to the screen without running the full
         repaint path.
Forwards To: 005D5890
Return Value: n/a
Status: Original dependency - forwards to the original image.
*/
void GraphicWin::soft_update() {
    (ORIGINAL(this)->*GraphicWinOriginalSoftUpdate)();
}

func_graphic_win_update GraphicWinOriginalUpdate =
    original_method<func_graphic_win_update>(0x005D56B0);

/*
 * A forwarder, not a recovery: ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z at
 * 0x005D56B0, the one-argument overload of three. See soft_update above for
 * the auto_inline reasoning.
 */
#pragma auto_inline(off)
void GraphicWin::update(GraphicWin *target) {
    (ORIGINAL(this)->*GraphicWinOriginalUpdate)(target);
}
#pragma auto_inline(on)
