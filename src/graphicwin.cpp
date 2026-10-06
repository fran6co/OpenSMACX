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
// ORIGINAL: 0x005D7140 ??_GGraphicWin@@UAEPAXI@Z 0x005D7140-0x005D715E FILE
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x005D7140-0x005D715E", "Pinned class constructor ??0GraphicWin@@QAE@XZ at 0x005D4CF0 stores original class/view table VA 0x66fc50 at 0x005D4D2B (mov dword ptr [esi], 0x66fc50); the current genuine owner vftable ??_7GraphicWin@@6BWin@@@ selects generated ??_GGraphicWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EGraphicWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GGraphicWin@@UAEPAXI@Z at 0x005D7140 calls the source-mapped ??1GraphicWin@@QAE@XZ at 0x005D4DD0, then tests byte ptr [esp + 8], 1; je 0x5d7158 bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/graphicwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (no byte-exact claim existed); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original GraphicWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GGraphicWin@@UAEPAXI@Z
#include "stdafx.h"

// GraphicWin constructor-installed Win branch: capture slot4, update slot63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x0066FC50 ? ??_7GraphicWin@@6BWin@@@ 0x005D4CF0
#include "original_seam.h"
#include "graphicwin.h"
#include "buffer.h"
#include "scroll.h"
#include "temp.h"  // HandleMain, the window the invalidate targets
#include "statuswin.h"
#include "dialog.h"
#include "sounddevice.h"
#include "menu.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "basepop.h"
#include "sprite.h"

#include <cstring>

// SDK import identity; the original IAT slot is derived from the pinned image.
// RECOVERY_IMPORT: v1 __imp__InvalidateRect@12 USER32.dll name:InvalidateRect
// RECOVERY_IMPORT: v1 __imp__GetWindowRect@8 USER32.dll name:GetWindowRect
// RECOVERY_IMPORT: v1 __imp__IntersectRect@12 USER32.dll name:IntersectRect
// RECOVERY_IMPORT: v1 __imp__BitBlt@36 GDI32.dll name:BitBlt

void __fastcall buffer_subobject_destructor(void *self) {
    // Source-owned: dispatches to the recovered Buffer destructor rather than
    // the original body at 0x005D7410.
    buffer_destructor_redirect(reinterpret_cast<Buffer *>(self), nullptr);
}

void __fastcall buffer_subobject_close(void *self) {
    reinterpret_cast<Buffer *>(self)->close();
}

void __fastcall win_subobject_destructor(void *self) {
    // Source-owned since the body homed into win.cpp: dispatches to the
    // recovered Win destructor rather than the raw image address 0x005EBC90,
    // which is unmapped in a standalone build. Same move the Buffer
    // subobject destructor above made when its body landed.
    reinterpret_cast<Win *>(self)->Win::~Win();
}

func_subobject_destructor BufferSubobjectDestructor = original_method<func_subobject_destructor>(
    reinterpret_cast<unsigned long>(&buffer_subobject_destructor));
func_subobject_destructor WinOriginalDestructor =
    original_method<func_subobject_destructor>(
        reinterpret_cast<unsigned long>(&win_subobject_destructor));
func_subobject_close BufferSubobjectClose = original_method<func_subobject_close>(
    reinterpret_cast<unsigned long>(&buffer_subobject_close));
func_subobject_close WinOriginalClose = original_method<func_subobject_close>(0x005EB640);
// RECOVERY_OBJECT: v1 0x009B33C0 ?GraphicWinFieldA0CDefault@@3P6AHPAVGraphicWin@@PADPAVBuffer@@@ZA
GraphicWinCaptionDraw GraphicWinFieldA0CDefault;  // 0x009B33C0

// Eleven borrowed Sprite defaults, originally in PE zero-fill. The loader
// publishes them through set_def_nc_sprites after creating the frame assets.
// RECOVERY_OBJECT: v1 0x009B3394 ?GraphicWinInitDefaults@@3UGraphicWinFrameDefaults@@A
GraphicWinFrameDefaults GraphicWinInitDefaults;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005D5500 ?load_pcx@GraphicWin@@QAEHPADPAUPalette@@HH@Z 0x005D5500-0x005D5536 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005d5500.cpp", "image:terranx_original.exe:0x005D5500", "Original instructions establish owner, bounded body and callback/state order; SocialWin reloads faction after social_upheaval and every later stateful call. GraphicWin dispatches through the real Buffer base.", "recovery/tasks/interface-contract-report-closure.json; bounded UI-five scope preserves original baseline"], "reason": "Recover the original in its genuine UI owner using actual members, globals and base/virtual calls. Retain seed evidence and historical obligations; this binding does not grant a current matching or runtime claim."}
// symbol    ?load_pcx@GraphicWin@@QAEHPADPAVPalette@@HH@Z
int GraphicWin::load_pcx(char *path, Palette *palette, int flags, int transparency) {
    if ((iFlags_ & 0x800000) != 0) {
        return 1;
    }
    return Buffer::load_pcx(path, palette, flags, transparency);
}

// Process-lifetime borrowed colour-map pointer, originally a zero-filled DWORD
// at 0x009B3390. The pinned image has only the two fill loads: no direct writer
// or address escape was found. That is not proof against computed indirect
// writes; retain mutable storage and the optional remapping path. No allocation
// or destruction of the pointed-to table is evidenced here.
// RECOVERY_OBJECT: v2 0x009B3390 0x009B3394 ?GraphicWinColorMapTable@@3PAEA 0x005D5350
unsigned char *GraphicWinColorMapTable = nullptr;

/*
Purpose: Construct the Win base and Buffer subobject, then install GraphicWin
         tables and initialize its trailing window state.
// ORIGINAL: 0x005D4CF0 ??0GraphicWin@@QAE@XZ 0x005D4CF0-0x005D4DC4;0x00662B10-0x00662B22 BYTE_EXACT
// LEVER: `GraphicWin *construct()` returning `this` (`mov eax, esi` at the close) instead of `void construct()` - matches the image's closing instruction, which a void-returning method never emits. Reduced listing_diff from 5 differing runs to 4; did not change the `agreeing` count, so the tier stays MISMATCH.
// symbol    ??0GraphicWin@@QAE@XZ
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
// BOTH BASES, BOTH VFPTRS, ALL FROM THE COMPILER. The image runs
// Win::Win() then Buffer::Buffer() on the 0x444 subobject, then stores
// 0x66fc50 at offset 0 and 0x66fc48 at 0x444 - a plain
// base-ctors-then-vfptrs constructor, with no member-initialiser list
// to push the vfptr stores later. Buffer's constructor is reached
// UNCONDITIONALLY here, which is the null guard the old placement-new
// `new (&buffer_) Buffer()` could never shed.
GraphicWin::GraphicWin() {

    // Win's own vtable slot and the Buffer subobject's vtable slot are
    // compiler-managed, not ordinary members a derived class can name.
    // field_134_/field_138_ are Win's own, named now that the base's fields
    // are protected - one object, which is how the image reaches them.
    // Everything else below is GraphicWin's own field, written directly.
    field_A10_ = 0;
    field_134_ = 0;
    field_138_ = 0;
    frame_top_left_ = 0;
    frame_bottom_left_ = 0;
    frame_top_right_ = 0;
    frame_bottom_right_ = 0;
    frame_left_ = 0;
    frame_right_ = 0;
    frame_bottom_ = 0;
    frame_top_ = 0;
    frame_noncaption_left_ = 0;
    frame_noncaption_right_ = 0;
    frame_noncaption_middle_ = 0;
    overlay_top_left_ = 0;
    overlay_bottom_left_ = 0;
    overlay_top_right_ = 0;
    overlay_bottom_right_ = 0;
    owned_buffer_ = 0;
    field_A0C_ = GraphicWinFieldA0CDefault;
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

// Win::close is not source-owned yet (0x005EB640, still 1610 bytes of
// verbatim teardown). The image's own GraphicWin::close calls it with a
// plain `call rel32` and no null check, so the null-checked runtime seam has
// to move OFF the call site and into a one-argument `__fastcall` forwarder:
// a single-parameter `__fastcall` receives its argument in ECX only, which
// is the same encoding as a zero-argument `__thiscall`, so the call site
// itself compiles to the direct, unconditional call the image has. The
// pointer stays bindable inside the forwarder for hybrid-process tests.
void __fastcall win_close_original(Win *self) {
    if (WinOriginalClose) {
        (ORIGINAL(self)->*WinOriginalClose)();
    }
}

/*
Purpose: Close a GraphicWin by closing its Win base and Buffer subobject,
         resetting its window-specific state, and deleting the trailing
         owned interface when present.
// ORIGINAL: 0x005D4E40 ?close@GraphicWin@@QAEXXZ 0x005D4E40-0x005D4EE6 SEMANTIC
// TRIED: image calls both 0x5EB640 (Win::close) and 0x5D7470 (Buffer::close) directly with no null check - the null-checked `if (WinOriginalClose) (ORIGINAL(this)->*WinOriginalClose)();` form at the call site compiled an extra load/compare/branch/ indirect-call the image never has (0/37 agreeing, diverging at instruction 0). Moving the null check into `win_close_original` (a real function, so the call site is a direct E8) and calling the already-recovered `buffer_.close()` in place of the second seam fixed both calls at once.
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
    win_close_original(this);
    Buffer::close();

    // Win's own field_134_/field_138_ are named now that the base's fields
    // are protected; everything else here is GraphicWin's own field, written
    // directly.
    Buffer *const release_target = owned_buffer_;
    field_A10_ = 0;
    field_134_ = 0;
    field_138_ = 0;
    frame_top_left_ = 0;
    frame_bottom_left_ = 0;
    frame_top_right_ = 0;
    frame_bottom_right_ = 0;
    frame_left_ = 0;
    frame_right_ = 0;
    frame_bottom_ = 0;
    frame_top_ = 0;
    frame_noncaption_left_ = 0;
    frame_noncaption_right_ = 0;
    frame_noncaption_middle_ = 0;
    overlay_top_left_ = 0;
    overlay_bottom_left_ = 0;
    overlay_top_right_ = 0;
    overlay_bottom_right_ = 0;
    const GraphicWinCaptionDraw default_value = GraphicWinFieldA0CDefault;
    field_A0C_ = default_value;
    if (!release_target) {
        return field_A0C_value();
    }

    // Slot 0, called where it lives: `vtable_method` leaves the call operand
    // at `[vtable]` so VC6 emits one `call dword ptr [reg]` rather than
    // loading the slot into a register first.
    // TRIED: inlining the `vtable_method<...>(...)` call directly into
    // the `->*` expression instead of naming it `deleting_destructor` first -
    // no change, still `eax` where the image has `edx` for this one slot.
    func_scalar_deleting_destructor &deleting_destructor =
        vtable_method<func_scalar_deleting_destructor>(release_target, 0);
    const uint32_t result = (ORIGINAL(release_target)->*deleting_destructor)(1);
    owned_buffer_ = 0;
    return result;
}


/*
Purpose: Destroy a GraphicWin by installing the original virtual tables,
         clearing the trailing field, and destroying the Buffer subobject
         before the Win base.
// ORIGINAL: 0x005D4DD0 ??1GraphicWin@@QAE@XZ 0x005D4DD0-0x005D4E37;0x00662B22-0x00662B34 BYTE_EXACT
// symbol    ??1GraphicWin@@UAE@XZ
// size      121 bytes
// prototype void (__thiscall ??1GraphicWin@@QAE@XZ)(GraphicWin* this)
// callers   185   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7410 0x005EBC90
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete with temporary Buffer and Win subobject dependencies
*/
// A REAL DESTRUCTOR. Its own TRIED lesson named the precondition - Buffer
// and Win as a real base - and both are, so VC6 generates the SEH unwind
// chain and the base destructor calls itself.
GraphicWin::~GraphicWin() {
    field_A10_ = 0;
}

// ONLY BECAUSE C++ CANNOT TAKE A DESTRUCTOR'S ADDRESS. Every ordinary call
// site calls `->~GraphicWin()` directly now; what is left is
// src/leaf_recoveries.cpp, which stores this in a function-pointer slot and
// casts it to SingleArgDtor. That is the whole reason a free function still
// exists, and it forwards rather than modelling anything.
//
// THE NULL GUARD IS NOT THE IMAGE'S. ??1GraphicWin@@UAE@XZ is BYTE_EXACT
// with no guard at all; this one protects the pointer slot's callers, which
// the old free-function model used to protect from inside.
GraphicWin *__fastcall graphic_win_destructor_redirect(GraphicWin *self, void *) {
    if (self) {
        self->~GraphicWin();
    }
    return self;
}

/*
Purpose: Fill a rectangle in the window's own buffer.
// ORIGINAL: 0x005D5440 ?fill@GraphicWin@@QAEHHHHHH@Z 0x005D5440-0x005D5467 BYTE_EXACT
// size      39 bytes
// prototype int (__thiscall ?fill@GraphicWin@@QAEHHHHHH@Z)(GraphicWin* this, int xLeft, int yTop, int length, int width, int)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8240
Return Value: whatever Buffer::fill returns
Status: Complete
*/

int GraphicWin::fill(int x1, int y1, int x2, int y2, int color) {
    // The surface subobject: Buffer is a real base at 0x444, so the qualified
    // call names it directly instead of casting the offset by hand.
    return Buffer::fill(x1, y1, x2, y2, color);
}


// USER32!InvalidateRect - called directly (see GraphicWin::redraw below).
// image address 0x00669304 is that call's own IAT slot; it is not read as
// a raw address anywhere here, so there is nothing hardcoded to fault on
// in a host test executable with a different layout.

/*
Purpose: Paint the window's surface in one colour. A window that is marked
         transparent and whose parent agrees copies the parent's pixels in
         instead, then remaps them through the process colour table.
// ORIGINAL: 0x005D5250 ?fill@GraphicWin@@QAEXH@Z 0x005D5250-0x005D5346
// LEVER: `vtable_slot<parent_query_fn>(parent, 0xF4)(parent)` in place of
//        the pointer-to-member `original_method<...>(vtable[0xF4/4])`
//        spelling for the slot-0xF4 query - the image's own call is ONE
//        `call dword ptr [eax+0xf4]`, where the pointer-to-member form
//        loads the slot into a register first (two instructions).
// TRIED: not chased to completion - still a big structural gap beyond
//            that one call (80 image instructions, largely a different
//            register plan for `flags_`/`parent` starting at the very first
//            branch). This is a large function (246 bytes); the vtable-slot
//            fix is banked, the rest is not at this budget.
// LEVER: the member-form rewrite of 2026-08-27 (iFlags_/win_parent_/the two
//        RECTs/dib_ named instead of memcpy walks off raw offsets) scored
//        11/80 where the raw-offset form scored 1/80 - the honest spelling
//        is the close one, and the remaining gap stays a register-plan
//        divergence from the first branch on.
// READ OFF THE IMAGE (2026-08-27): the redundant test at 0x005D52BC is a
//        CONDITIONAL SELECT feeding copy's source argument -
//        `test esi,esi / je +8 / lea edi,[esi+0x444] / jmp / xor edi,edi` -
//        it computes the surface INSIDE the true arm from `this`, it does
//        not branch around the call, and it sits between the argument
//        spills. Three source shapes tried and all score BELOW the plain
//        body: ternary selecting the existing `surface` local (5/80), a
//        null test on a `Buffer *` copy with a two-arm call (1/80), the
//        select computed in the call argument from a `self` local (2/80).
//        The select is real but no spelling tried yet makes VC6 schedule
//        it where the image has it; the dominant gap is elsewhere.
// TRIED: 2026-08-28, `measure --dir`, seven more spellings of the two
//        remaining structural differences, against the tree's 11/80 (0.649).
//        (a) the query result held as a `void *` instead of `bool`, so the
//        guard is `test eax,eax / je` rather than `setne al / test al,al /
//        jne`: 1/80. (b) the whole transparent path nested INSIDE the two
//        guards (`if (flags && parent) { if (query) { ... } }`), which puts
//        the parent upcast where `win_parent_` is provably non-null and so
//        drops the parent select the tree emits: 3/80, similarity 0.728 -
//        the only spelling that scores ABOVE the tree on similarity while
//        agreeing with FEWER instructions, so the tier metric and the
//        agreement count disagree here. (c) (b) plus the parent's own
//        `Buffer *` upcast hoisted out of the inner test: 4/80. (d) (a) with
//        `surface` dropped and both Buffer calls qualified (`Buffer::fill` /
//        `Buffer::map_colors`): 11/80, no change. (e) the image's dead RECT
//        pair transcribed as four plain stores into two locals - the
//        compiler deletes them and the listing is byte-identical to the
//        tree's: 11/80. (f) the same pair `volatile`, which does keep the
//        stores: 7/80. (g) (e) with the first local's address taken: 11/80,
//        still deleted. The image's `sub esp,0x10` frame and its 10 dead
//        instructions are what no spelling reaches; the wall stands.
// TRIED: 2026-08-28 confirming sweep, one re-measure of the tree's own body
//        after the neighbourhood moved (basewin.cpp / basebutton.cpp edited
//        today): still 11/80 at /c /O2 /Gy /GR- /GX, 71 compiled. No new
//        lever attempted - nine refutations are on this marker already.
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
    // The window's Buffer subobject at +0x444, typed so its methods can be
    // called by NAME - through a pointer-to-member the call sites emitted
    // `call [ptr]` where the image has `call rel32`. Buffer is a real base,
    // so the upcast is the language's own +0x444.
    Buffer *const surface = this;
    // Bit 19 alone does not make the window transparent: the parent has to
    // exist and expose a GraphicWin through Win's slot 0xF4. A plain Win
    // returns null; GraphicWin returns itself.
    bool transparent = false;
    if ((iFlags_ & 0x80000) != 0 && win_parent_ != nullptr) {
        Win *const parent = win_parent_;
        transparent = parent->get_graphic_win() != nullptr;
    }
    if (!transparent) {
        surface->fill(color);
        return;
    }
    // The blit origin is where the window sits in its parent (outer_rect_)
    // plus where the draw area sits in the window (client_rect_).
    const int origin_x = client_rect_.left + outer_rect_.left;
    const int origin_y = client_rect_.top + outer_rect_.top;
    // The DIB's own dimensions: biHeight is stored negative (top-down), so
    // the vertical passes negate it back.
    int width = static_cast<int>(dib_.bmiHeader.biWidth);
    int height = static_cast<int>(dib_.bmiHeader.biHeight);
    static_cast<Buffer *>(static_cast<GraphicWin *>(win_parent_))->copy(
        surface, origin_x, origin_y, 0, 0, width, -height);
    unsigned char *const table = GraphicWinColorMapTable;
    if (table == nullptr) {
        return;
    }
    // Re-read, do NOT hoist onto the pair above: the original reloads both at
    // 0x005D5302 and 0x005D5308, so a blit that resized the surface is seen
    // by the remap. map_colors uses exclusive bounds, but the original caller
    // still subtracts one: its remap omits the final row and column.
    width = static_cast<int>(dib_.bmiHeader.biWidth);
    height = static_cast<int>(dib_.bmiHeader.biHeight);
    surface->map_colors(0, 0, width - 1, -1 - height, table);
}

// Rectangle counterpart of fill(int). The parent query is only an admission
// test: the original copies from the reloaded parent's Buffer base, not from
// the object returned by the query. As in the original, the transparent path
// requires a live rectangle and a parent that remains a GraphicWin.
// ORIGINAL: 0x005D5350 ?fill@GraphicWin@@QAEHPAURECT@@H@Z 0x005D5350-0x005D543E FILE
// RECOVERY_BINDING: v1 {"evidence": ["5D5373 queries Win slotF4;5D53D2 reloads parent before5D53E6 Buffer copy;5D53EB loads the shared table and5D53F5..5417 reread RECT and remap right/bottom-minus-one. Transparent exit returns0, opaque5D5435 returns Buffer fill result, both ret8."], "reason": "This rectangle fill uses the real Win hierarchy and Buffer base. Transparent admission queries the parent, but the reloaded actual parent supplies the copied pixels; opaque fill delegates and returns its result."}
// symbol    ?fill@GraphicWin@@QAEHPAUtagRECT@@H@Z
int GraphicWin::fill(RECT *area, int color) {
    bool transparent = false;
    if ((iFlags_ & 0x80000) != 0 && win_parent_ != nullptr) {
        Win *const parent = win_parent_;
        transparent = parent->get_graphic_win() != nullptr;
    }
    if (!transparent) {
        return Buffer::fill(area, color);
    }

    const int origin_x = client_rect_.left + outer_rect_.left;
    const int origin_y = client_rect_.top + outer_rect_.top;
    const int left = area->left;
    const int top = area->top;
    const int width = area->right - left;
    const int height = area->bottom - top;
    static_cast<Buffer *>(static_cast<GraphicWin *>(win_parent_))->copy(
        this, left + origin_x, top + origin_y, left, top, width, height);

    // Copy can invoke surface callbacks. Load the table and rectangle again
    // afterwards; neither the incoming bounds nor the table may be cached
    // across that call. The original ignores both drawing return codes.
    unsigned char *const table = GraphicWinColorMapTable;
    if (table != nullptr) {
        Buffer::map_colors(area->left, area->top,
                           area->right - 1, area->bottom - 1, table);
    }
    return 0;
}


/*
Purpose: Repaint the window and invalidate the screen area it occupies. A
         window already inside a redraw is skipped, so a paint hook that
         redraws again cannot recurse.
// ORIGINAL: 0x005D5A70 ?redraw@GraphicWin@@QAEXXZ 0x005D5A70-0x005D5B64 BYTE_EXACT
// symbol    ?redraw@GraphicWin@@UAEXXZ
// LEVER: `InvalidateRect(...)` called directly instead of through a
//        manually lazily-resolved function pointer read from a hardcoded
//        IAT address - the image just calls `dword ptr [0x669304]`
//        unconditionally, which is what an ordinary imported-function call
//        already compiles to. Dropped the now-dead
//        `GraphicWinInvalidateRect`/`GraphicWinInvalidateRectImport`
//        globals with it. 45/69 -> 69/69.
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
uint32_t GraphicWin::field_A0C_value() const {
    uint32_t value = 0;
    memcpy(&value, &field_A0C_, sizeof(value));
    return value;
}

// ORIGINAL: 0x005D63F0 ?on_redraw_nc@GraphicWin@@QAEXPAURECT@@H@Z 0x005D63F0-0x005D68B7
// RECOVERY_BINDING: v1 {"reason":"Recover the complete non-client frame painter in GraphicWin using its real Win/Buffer/member ownership.","evidence":["Pinned GraphicWin table 0x66FC50 slot 67 selects 0x5D63F0.","Win::on_redraw_nc calls slot 60 with each live border Buffer and quadrant; the owned caption and frame members are source-typed.","The old recovered-unit seed retains its complete ORIGINAL span and call-list evidence in Git history; no byte claim or old frame-layout typo is converted into an acceptance claim."]}
// kind      game
// symbol    ?on_redraw_nc@GraphicWin@@UAEXPAUtagRECT@@H@Z
void GraphicWin::on_redraw_nc(RECT *area, int side) {
    if ((iSomeFlag_ & 2) == 0) {
        return;
    }

    if (area != 0) {
        side = -1;
    }

    if ((iFlags_ & 0x10000000) == 0) {
        Win::on_redraw_nc(area, side);
        return;
    }

    int top_inset;
    Sprite *top_left;
    Sprite *top_edge;
    Sprite *top_right;
    if ((iFlags_ & 0x10) == 0 || (iFlags_ & 0x400000) != 0) {
        top_inset = border_thickness_;
        top_left = frame_noncaption_left_;
        top_edge = frame_noncaption_middle_;
        top_right = frame_noncaption_right_;
    } else {
        top_inset = caption_height_;
        top_left = frame_top_left_;
        top_edge = frame_top_;
        top_right = frame_top_right_;
    }

    int bottom_inset = bottom_border_thickness_;
    if (bottom_inset == -1) {
        bottom_inset = border_thickness_;
    }
    const int client_width = client_rect_.right - client_rect_.left;
    char *caption = field_E0_;

    // Quadrant 0: top caption/non-caption strip.
    if (buffer1_ != 0 && (side == -1 || side == 0)) {
        Buffer *const buffer = buffer1_;
        buffer->fill(WinFillColour);
        if (top_left != 0) {
            top_left->draw(buffer, top_left->transparent_index(), 0, 0, 1, 1);
        }
        if (top_right != 0) {
            top_right->draw(buffer, top_right->transparent_index(),
                            buffer->pixel_width() - top_right->width(), 0, 1, 1);
        }
        if (top_edge != 0 && top_left != 0 && top_right != 0) {
            buffer->tile(top_edge, 0, 0, top_left->width(), 0,
                         client_width - top_right->width() - top_left->width(),
                         buffer->pixel_height());
        }
        if (caption != 0 && (iFlags_ & 0x10) != 0) {
            if (field_A0C_ != 0) {
                field_A0C_(this, caption, buffer);
            }
            on_draw_caption(caption, buffer);
        }
        vslot_60(buffer, 0);
    }

    // Quadrant 2: bottom strip.
    if (buffer4_ != 0 && (side == -1 || side == 2)) {
        Buffer *const buffer = buffer4_;
        buffer->fill(WinFillColour);
        if (frame_bottom_left_ != 0) {
            frame_bottom_left_->draw(
                buffer, frame_bottom_left_->transparent_index(), 0,
                -(frame_bottom_left_->height() + buffer->pixel_height()), 1, 1);
        }
        if (frame_bottom_right_ != 0) {
            frame_bottom_right_->draw(
                buffer, frame_bottom_right_->transparent_index(),
                buffer->pixel_width() - frame_bottom_right_->width(),
                -(frame_bottom_right_->height() + buffer->pixel_height()), 1, 1);
        }
        if (frame_bottom_ != 0 && frame_bottom_left_ != 0 &&
            frame_bottom_right_ != 0) {
            buffer->tile(
                frame_bottom_, 0, 0, frame_bottom_left_->width(),
                -(buffer->pixel_height() + frame_bottom_->height()),
                (buffer->pixel_width() - frame_bottom_right_->width()) -
                    frame_bottom_left_->width(),
                frame_bottom_->height());
        }
        vslot_60(buffer, 2);
    }

    // Quadrant 3: right strip.
    if (buffer3_ != 0 && (side == -1 || side == 3)) {
        Buffer *const buffer = buffer3_;
        buffer->fill(WinFillColour);
        if (top_left != 0) {
            top_left->draw(buffer, top_left->transparent_index(),
                           0, -top_inset, 1, 1);
        }
        if (frame_bottom_left_ != 0) {
            frame_bottom_left_->draw(
                buffer, frame_bottom_left_->transparent_index(), 0,
                (bottom_inset - buffer->pixel_height()) -
                    frame_bottom_left_->height(), 1, 1);
        }
        if (frame_left_ != 0 && top_left != 0 &&
            frame_bottom_left_ != 0) {
            buffer->tile(
                frame_left_, 0, 0, 0,
                top_left->height() - top_inset,
                border_thickness_,
                (((bottom_inset - buffer->pixel_height()) -
                  frame_bottom_left_->height()) - top_left->height()) +
                    top_inset);
        }
        vslot_60(buffer, 3);
    }

    // Quadrant 1: left strip.
    if (buffer2_ != 0 && (side == -1 || side == 1)) {
        Buffer *const buffer = buffer2_;
        buffer->fill(WinFillColour);
        if (top_right != 0) {
            top_right->draw(buffer, top_right->transparent_index(),
                            buffer->pixel_width() - top_right->width(),
                            -top_inset, 1, 1);
        }
        if (frame_bottom_right_ != 0) {
            frame_bottom_right_->draw(
                buffer, frame_bottom_right_->transparent_index(),
                buffer->pixel_width() - frame_bottom_right_->width(),
                (bottom_inset - buffer->pixel_height()) -
                    frame_bottom_right_->height(), 1, 1);
        }
        if (frame_right_ != 0 && top_right != 0 &&
            frame_bottom_right_ != 0) {
            buffer->tile(
                frame_right_, 0, 0,
                buffer->pixel_width() - frame_right_->width(),
                top_right->height() - top_inset,
                frame_right_->width(),
                (((bottom_inset - buffer->pixel_height()) -
                  frame_bottom_right_->height()) - top_right->height()) +
                    top_inset);
        }
        vslot_60(buffer, 1);
    }
}

void GraphicWin::redraw() {
    if (HandleMain == nullptr) {
        return;
    }
    // Win's own re-entrancy latch, named now that the base's fields are
    // protected.
    uint32_t state = field_1A0_;
    if ((state & 1) != 0) {
        return;
    }
    state |= 1;
    field_1A0_ = state;
    WinCallbackWindow = this;

    // The paint hook, called before the paint itself.
    if (field_A10_ != nullptr) {
        field_A10_();
    }
    // The window's own paint, vtable slot 12: the image reaches it the way
    // any virtual call is reached, so the call is spelled as one.
    on_redraw();
    this->overlay_nonclient(nullptr);

    // Re-read, do NOT reuse the latched value: the original reloads at
    // 0x005D5ABB before clearing bit 0, so any bit the paint hook or the
    // virtual paint set on 0x1A0 survives this clear.
    state = field_1A0_;
    state &= 0xFFFFFFFEU;
    field_1A0_ = state;

    if (!static_cast<Win *>(this)->is_visible()) {
        return;
    }
    // The surface RECT inside the Buffer subobject - the same field
    // update() reads below.
    RECT area = rect2_;
    int x_offset = 0;
    int y_offset = 0;
    static_cast<Win *>(this)->client_to_screen(&x_offset, &y_offset);
    area.left += x_offset;
    area.right += x_offset;
    area.top += y_offset;
    area.bottom += y_offset;
    // The window handle is read again here rather than reused from the guard
    // above, matching the original's second load at 0x005D5B43. THE REAL
    // API, not a lazily-resolved pointer: the image just calls
    // `dword ptr [0x669304]` unconditionally - that address IS
    // `InvalidateRect`'s own IAT slot, which the compiler already produces
    // an indirect call through for an ordinary imported function.
    InvalidateRect(HandleMain, &area, FALSE);
}


/*
Purpose: When visible, tell the window to update and flip a rect the size of
         the RECT at this+0x474 (the same field redraw() reads), positioned
         at the origin.
// ORIGINAL: 0x005D56B0 ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z 0x005D56B0-0x005D571D BYTE_EXACT
// symbol    ?update@GraphicWin@@QAEXPAV1@@Z
// size      109 bytes
// prototype void (__thiscall ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z)(GraphicWin* this, GraphicWin*)
// callers   8   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECFE0 0x005EFD20 0x005F74A0 0x005F7E90
Return Value: n/a
Status: Complete
*/
void GraphicWin::update(GraphicWin *) {
    // Four separate field reads of the surface RECT, not a copied RECT: the
    // latter compiles a real `call memcpy` here (unlike redraw()'s own read
    // of the same field, which VC6 happens to inline), where the image loads
    // each of the four dwords directly.
    int height = rect2_.bottom - rect2_.top;
    int width = rect2_.right - rect2_.left;

    if (static_cast<Win *>(this)->is_visible()) {
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        static_cast<Win *>(this)->update_window(&rect);
        static_cast<Win *>(this)->client_to_screen(&rect);
        Win::flip(&rect);
    }
}

// ORIGINAL: 0x005D5630 ?update@GraphicWin@@QAEXPAURECT@@PAUGraphicWin@@@Z 0x005D5630-0x005D56AA
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005D5630-0x005D56AA"], "reason": "Rectangle update through real Buffer/Win bases; original returns zero and ignores its second explicit argument."}
// symbol    ?update@GraphicWin@@QAEHPAUtagRECT@@PAV1@@Z
int GraphicWin::update(RECT *area, GraphicWin *) {
    if (!area) area = &rect2_;
    int top = area->top;
    int left = area->left;
    int height = area->bottom - top;
    int width = area->right - left;
    if (Win::is_visible()) {
        RECT local;
        local.top = top;
        local.left = left;
        local.right = left + width;
        local.bottom = top + height;
        Win::update_window(&local);
        Win::client_to_screen(&local);
        Win::flip(&local);
    }
    return 0;
}

// Invalidate the Buffer bounds in screen coordinates without repainting now.
// ORIGINAL: 0x005D5890 ?soft_update@GraphicWin@@QAEXXZ 0x005D5890-0x005D592D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 0x005D58A3 reads the Buffer rectangle at GraphicWin+0x474; 0x005D58E0 calls Win::client_to_screen on a zero origin and 0x005D5922 calls USER32 InvalidateRect with HandleMain and erase=false. Complete span retained from src/unrecovered/005d5890.cpp."], "reason": "Invalidate the visible GraphicWin Buffer bounds through its real Win base and the SDK import, without copying pixels or calling the original executable."}
void GraphicWin::soft_update() {
    if (!Win::is_visible()) return;

    int x = 0, y = 0;
    RECT local = rect2_;
    Win::client_to_screen(&x, &y);
    local.left += x;
    local.right += x;
    local.top += y;
    local.bottom += y;
    InvalidateRect(HandleMain, &local, FALSE);
}

// Clip a borrowed client rectangle, then invalidate its screen projection.
// A null argument takes the full-buffer path, including its visibility check.
// ORIGINAL: 0x005D5930 ?soft_update@GraphicWin@@QAEXPAUtagRECT@@@Z 0x005D5930-0x005D5A6E
// RECOVERY_BINDING: v1 {"evidence": ["Original 0x005D5937 checks visibility and null path repeats it at 0x005D5954. 0x005D59D5 copies caller coordinates; 0x005D5A01 intersects the local copy with Buffer bounds, 0x005D5A1F converts the zero origin, and 0x005D5A60 invalidates HandleMain without erasing. Complete span retained from src/recovered/units/005d5930.cpp.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/005d5930.cpp", "Historical seed spelling ?soft_update@GraphicWin@@QAEXPAURECT@@@Z; current source mapping ?soft_update@GraphicWin@@QAEXPAUtagRECT@@@Z; emitted symbol ?soft_update@GraphicWin@@QAEXPAUtagRECT@@@Z."], "reason": "Invalidate a clipped, translated copy of a borrowed client rectangle; null takes the full Buffer path with the original repeated visibility check. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
void GraphicWin::soft_update(RECT *area) {
    if (!Win::is_visible()) return;
    if (!area) {
        soft_update();
        return;
    }

    RECT local = *area;
    if (!IntersectRect(&local, &local, &rect2_)) return;

    int x = 0, y = 0;
    Win::client_to_screen(&x, &y);
    local.left += x;
    local.right += x;
    local.top += y;
    local.bottom += y;
    InvalidateRect(HandleMain, &local, FALSE);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005D5720 ?soft_update@GraphicWin@@QAEXHHHH@Z 0x005D5720-0x005D588C
// RECOVERY_BINDING: v1 {"evidence":["source:src/recovered/units/005d5720.cpp","source:src/graphicwin.cpp:GraphicWin::soft_update"],"reason":"Own the four-coordinate invalidation path required by SetupWin and other window painters; clip against the client rectangle, translate to screen, and invalidate without erasing."}
void GraphicWin::soft_update(int x, int y, int width, int height) {
    if (!Win::is_visible()) return;
    RECT local = {x, y, x + width, y + height};
    if (!IntersectRect(&local, &local, &client_rect_)) return;

    int origin_x = 0;
    int origin_y = 0;
    Win::client_to_screen(&origin_x, &origin_y);
    OffsetRect(&local, origin_x, origin_y);
    InvalidateRect(HandleMain, &local, FALSE);
}

// Frame minima below use real Sprite dimensions. Window creation, client-size
// conversion and surface allocation remain separate recovery dependencies;
// tests of this orchestration do not establish their complete behavior.


/*
Purpose: Initialise a GraphicWin. Reset the window, republish the eleven
         process window defaults when the style asks for them, optionally
         allocate the owned Buffer the window parks at 0xA08, initialise the
         Win base, then size and initialise the window's own drawing surface
         and sync it to the active palette.
// ORIGINAL: 0x005D4EF0 ?init@GraphicWin@@QAEHHHHHPADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z 0x005D4EF0-0x005D5096;0x00662B34-0x00662B49
// LEVER: dropped the `uint32_t *const defaults = GraphicWinInitDefaults;`
//        local from the eleven-slot default table copy - `defaults` was
//        already a fixed-address constant, and caching it in a local
//        forced VC6 to hold it in a register across all eleven reads
//        instead of folding each one to its own absolute address the way
//        the image's eleven independent `mov reg, [0x9b33xx]` do. Indexing
//        `GraphicWinInitDefaults[N]` directly at each site instead reached
//        0.943 similar (up from a fresh mismatch at the very top of this
//        block).
// TRIED: remaining gap is a small, consistent eax/ecx swap around the
//            nonclient-flags/scrollbar-thickness pair near the end of the
//            function (`nonclient_flags`/`thickness` locals) - not chased
//            further; reordering their declarations was not tried because
//            this address's body is too large for a full-function
//            `try_spellings` candidate at this budget.
// TRIED: that reorder HAS now been tried, with try_spellings on the whole body, and it changes nothing: declaring `thickness` before `nonclient_flags` scores 2 of 112, identical to the committed order. So do the two shapes for the owned-Buffer allocation - collapsing `block`/`owned` into one `block != nullptr ? new (block) Buffer() : nullptr` ternary, and spilling the result as a `uint32_t` instead of a `Buffer *` - both 2 of 112. The eax/ecx swap is downstream, not the cause. What the cause is: under every one of the ten flag sets this tree scores, the compiled prologue opens `push ebp / mov ebp, esp` and an extra `push ecx` before its `mov eax, fs:[0]`, where the image reads fs:[0] FIRST and keeps no frame pointer - an FPO C++ EH frame whose handler is the `mov eax, <FuncInfo> / jmp __CxxFrameHandler` thunk at 0x00662B3F. That shifts every following instruction, which is why 112 instructions that listing_diff aligns into only 13 short diverging runs still score 2. `--all-flags` best is 0.943 similar at /c /O2 /Gy /GR- /Oy- /GX; /Oy alone does not drop the frame pointer here.
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
Status: Source-owned orchestration; dependency and runtime acceptance are separate.
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
// ORIGINAL: 0x005D50A0 ?init@GraphicWin@@QAEHPAURECT@@PADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z 0x005D50A0-0x005D50E8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:10163d406a831ec8afdb8b85d18d04d1fde4ec27:src/recovered/005d50a0.cpp retains the historical 72-byte seed; the pinned original 0x005D50A0-0x005D50E8 returns 0x10 for null bounds and forwards nonnull rectangle coordinates to 0x005D4EF0."], "reason": "Home the byte-exact rectangle initializer in GraphicWin so MapWin and other owners use its real null behavior and base lifetime."}
// symbol    ?init@GraphicWin@@QAEHPAUtagRECT@@PADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z
int GraphicWin::init(RECT *bounds, char *caption, int flags, Win *parent,
                     Menu *menu, BorderSizing *border) {
    if (bounds == 0) return 0x10;
    return init(bounds->left, bounds->top,
                bounds->right - bounds->left,
                bounds->bottom - bounds->top,
                caption, flags, parent, menu, border);
}

int GraphicWin::init(int x, int y, int width, int height, LPSTR title,
                     int flags, Win *parent, Menu *menu,
                     BorderSizing *border) {
    // Direct call at 0x005D4F09, not a virtual dispatch, and its return is
    // discarded: init on a live window tears the old one down first.
    close();

    // One `test edi, 0x30000000` at 0x005D4F12 covers both style bits: either
    // one republishes the whole block.
    if ((flags & 0x30000000) != 0) {
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
        // Every destination is GraphicWin's own field, so it is written
        // directly rather than through an aliased pointer into `this`.
        // The named defaults are real source-owned pointer storage.
        frame_top_left_ = GraphicWinInitDefaults.top_left;
        frame_top_right_ = GraphicWinInitDefaults.top_right;
        frame_bottom_left_ = GraphicWinInitDefaults.bottom_left;
        frame_bottom_right_ = GraphicWinInitDefaults.bottom_right;
        frame_top_ = GraphicWinInitDefaults.top;
        frame_bottom_ = GraphicWinInitDefaults.bottom;
        frame_left_ = GraphicWinInitDefaults.left;
        frame_right_ = GraphicWinInitDefaults.right;
        frame_noncaption_left_ = GraphicWinInitDefaults.noncaption_left;
        frame_noncaption_right_ = GraphicWinInitDefaults.noncaption_right;
        frame_noncaption_middle_ = GraphicWinInitDefaults.noncaption_middle;
    }

    // A separately owned Buffer, not another construction of our Buffer base.
    // close() has released any previous allocation before this point.
    if ((flags & 0x40000000) != 0) {
        owned_buffer_ = new Buffer;
    }

    // All nine arguments go straight through in order.
    const int base_result = Win::init(x, y, width, height, title, flags, parent, menu, border);
    if (base_result != 0) {
        return base_result;
    }

    // The window's own Buffer keeps a back pointer to the window, written as
    // `mov [esi+0x448], esi` at 0x005D5012 - before the minimum-size
    // computation, in the original's order. poOwner_ is Buffer's own member
    // now that the base's fields are protected, so the store needs no offset
    // and no cast.
    poOwner_ = this;
    compute_min_size();

    if ((flags & 0x800) == 0) {
        // Converts the outer size to a client size in place. What the callee
        // subtracts is its own business and is not asserted here.
        nonclient_to_client(&width, &height);
    } else {
        // The 0x800 arm goes the other way and ADDS the process scrollbar
        // thickness, pairing bit 3 of the 0x98 flag dword with the width and
        // bit 2 with the height. Both the flag dword and the thickness are
        // loaded once, at 0x005D5038 and 0x005D503E, ahead of either test;
        // the original tests only AL, which is equivalent for bits 2 and 3.
        const uint32_t nonclient_flags = iFlags_;
        const int thickness = ScrollDefaultThickness;
        if ((nonclient_flags & 8) != 0) {
            width += thickness;
        }
        if ((nonclient_flags & 4) != 0) {
            height += thickness;
        }
    }

    const int surface_result =
        Buffer::init(width, height, 0, nullptr);
    if (surface_result != 0) {
        return surface_result;
    }
    // Return discarded: the original zeroes EAX at 0x005D5081 straight after.
    Buffer::sync_to_palette(PaletteActive);
    return 0;
}


/*
// ORIGINAL: 0x005D6310 ?on_mouse_move@GraphicWin@@QAEXHHIH@Z 0x005D6310-0x005D632C BYTE_EXACT
// symbol    ?on_mouse_move@GraphicWin@@UAEXHHIH@Z
// size      28 bytes
// prototype void (__thiscall ?on_mouse_move@GraphicWin@@QAEXHHIH@Z)(GraphicWin* this, int, int, unsigned int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F6320
Status: Complete
*/
void GraphicWin::on_mouse_move(int x, int y, unsigned int keys, int from_parent) {
    // QUALIFIED, not a cast. Win is a non-virtual base at offset 0, so the
    // reinterpret_cast this body used to carry named no adjustment at all -
    // and once Win::on_mouse_move took vtable slot 16 the unqualified call
    // through it compiled to `mov eax,[ecx]` + an indirect dispatch. The
    // image calls 0x005F6320 directly (`calls` fact above): 10 of 11
    // instructions disagreed on that one load. Qualifying restores it.
    Win::on_mouse_move(x, y, keys, from_parent);
}




/*
Purpose: Step the receiver back to the subobject ??_GGraphicWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005D7160 ??_GGraphicWin@@WEEE@AEPAXI@Z 0x005D7160-0x005D716B BYTE_EXACT
// symbol    ??_EGraphicWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3GraphicWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005D7140` into
//   ??_GGraphicWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// ORIGINAL: 0x004042B0 sub_4042b0 0x004042B0-0x004042B3 FILE BYTE_EXACT
// symbol    ?get_graphic_win@GraphicWin@@UAEPAV1@XZ
// Win slot 61 and Buffer slot 1 identify the GraphicWin owning the surface.
// The original self-query is shared by derived windows.
GraphicWin *GraphicWin::get_graphic_win() {
    return this;
}

// ORIGINAL: 0x00404420 sub_404420 0x00404420-0x0040442B FILE BYTE_EXACT
// symbol    ?get_graphic_win@GraphicWin@@WEEE@AEPAV1@XZ
// Compiler-owned secondary-base entry: Buffer lies at GraphicWin +0x444.
// Original Buffer-side slot 1 at 0x00670C5C points here; this entry forwards
// to the self-query at 0x004042B0. There is no handwritten C++ thunk body.

// ORIGINAL: 0x005D5540 ?resize@GraphicWin@@QAEXHHH@Z 0x005D5540-0x005D5590 FILE BYTE_EXACT
// symbol    ?resize@GraphicWin@@UAEHHHH@Z
// The catalogued void/nonvirtual prototype is a seed interpretation.
// Win slot 3 and Buffer::load_pcx use the returned status.
// calls     0x005D7670 0x005ED880 0x005EEF60
int GraphicWin::resize(int width, int height, int client_size) {
    if (client_size == 0) {
        nonclient_to_client(&width, &height);
    }
    int result = Buffer::init(width, height, 0, nullptr);
    if (result == 0) {
        result = Win::resize(width, height, 1);
    }
    return result;
}

// ORIGINAL: 0x005D6330 ?on_set_cursor@GraphicWin@@QAEHPAXII@Z 0x005D6330-0x005D6350 FILE BYTE_EXACT
// symbol    ?on_set_cursor@GraphicWin@@UAEHPAXII@Z
// provenance (retired seed): git:57459ebf:src/recovered/units/005d6330.cpp
// Win-side virtual slot 85 forwards to the static cursor hook using the
// real main-window handle. The catalogued nonvirtual spelling is provisional.
// calls     0x005F2670
int GraphicWin::on_set_cursor(void *window, unsigned int hit, unsigned int message) {
    return Win::OnSetCursor(HandleMain, window, hit, message);
}

// ORIGINAL: 0x005D6210 ?on_size@GraphicWin@@QAEXIHH@Z 0x005D6210-0x005D6308 FILE
// symbol    ?on_size@GraphicWin@@UAEXIHH@Z
// provenance (retired seed): git:b5a9c5dd:src/unrecovered/005d6210.cpp
// The drawing surface is our Buffer base; the optional owned buffer is a
// separate allocation. PaletteActive is a pointer value, not its storage.
// calls     0x005D7670 0x005D79C0 0x005DE8F0 0x005EE330 0x005F7E90
void GraphicWin::on_size(unsigned int flags, int width, int height) {
    if (flags != 1) {
        int client_width = width;
        int client_height = height;
        if (client_width == 0 || client_height == 0) {
            client_width = dib_.bmiHeader.biWidth;
            client_height = -dib_.bmiHeader.biHeight;
        }
        const unsigned int window_flags = iFlags_;
        const int thickness = ScrollDefaultThickness;
        if ((window_flags & 8) != 0) {
            client_width -= thickness;
        }
        if ((window_flags & 4) != 0) {
            client_height -= thickness;
        }
        Buffer::init(client_width, client_height, init_flags_, nullptr);
        Buffer::sync_to_palette(PaletteActive);
        if ((iFlags_ & 0x40000000) != 0 && owned_buffer_ != nullptr) {
            owned_buffer_->resize(client_width, client_height);
            owned_buffer_->sync_to_palette(PaletteActive);
        }
        set_rects();
        on_nc_paint(nullptr, -1);
        resize_event(rect2_.right - rect2_.left, rect2_.bottom - rect2_.top);
        if (Win::is_visible()) {
            redraw();
        }
    }
}

// The original derived entry delegates non-client rectangle setup to Win.
// ORIGINAL: 0x005D63E0 ?set_rects@GraphicWin@@QAEXXZ 0x005D63E0-0x005D63E5 FILE BYTE_EXACT
// symbol    ?set_rects@GraphicWin@@UAEXXZ
// provenance (retired seed): git:078b2095:src/unrecovered/005d63e0.cpp
void GraphicWin::set_rects() {
    Win::set_rects();
}

// ORIGINAL: 0x005D6AC0 ?overlay_nonclient@GraphicWin@@QAEXPAURECT@@@Z 0x005D6AC0-0x005D6EB7 FILE
// symbol    ?overlay_nonclient@GraphicWin@@QAEXPAUtagRECT@@@Z
// Frame placement evidence: 0x005D6BB7-0x005D6D00 chooses the corner
// sprites and draws them; 0x005D6D15-0x005D6E65 tiles the edge sprites.
// The import at 0x006692A8 is GetWindowRect, not the seed's GetClientRect.
// The supplied area is in screen coordinates; the Buffer clip is local.
void GraphicWin::overlay_nonclient(RECT *area) {
    RECT saved = Buffer::rect1_;
    if (area != nullptr) {
        RECT window_rect;
        GetWindowRect(HandleMain, &window_rect);
        RECT clip;
        if (!IntersectRect(&clip, &window_rect, area)) {
            return;
        }
        clip.left -= window_rect.left;
        clip.right -= window_rect.left;
        clip.top -= window_rect.top;
        clip.bottom -= window_rect.top;
        Buffer::set_clip(&clip);
    } else {
        Buffer::set_clip(&rect2_);
    }

    int top_inset = ((iFlags_ & 0x10) && !(iFlags_ & 0x400000))
                        ? caption_height_ : border_thickness_;
    int bottom_inset = bottom_border_thickness_;
    if (bottom_inset == -1) {
        bottom_inset = border_thickness_;
    }

    if (iFlags_ & 0x60000000) {
        Sprite *top_left = overlay_top_left_ ? overlay_top_left_ : frame_top_left_;
        Sprite *top_right = overlay_top_right_ ? overlay_top_right_ : frame_top_right_;
        Sprite *bottom_left = overlay_bottom_left_ ? overlay_bottom_left_ : frame_bottom_left_;
        Sprite *bottom_right = overlay_bottom_right_ ? overlay_bottom_right_ : frame_bottom_right_;
        if (top_left) {
            top_left->draw(this, top_left->cTransparentIndex_,
                           -border_thickness_, -top_inset, 1, 1);
        }
        if (top_right) {
            top_right->draw(this, top_right->cTransparentIndex_,
                            dib_.bmiHeader.biWidth - top_right->iWidth_ + border_thickness_,
                            -top_inset, 1, 1);
        }
        if (bottom_left) {
            bottom_left->draw(this, bottom_left->cTransparentIndex_,
                              -border_thickness_,
                              bottom_inset - dib_.bmiHeader.biHeight - bottom_left->iHeight_, 1, 1);
        }
        if (bottom_right) {
            bottom_right->draw(this, bottom_right->cTransparentIndex_,
                               dib_.bmiHeader.biWidth - bottom_right->iWidth_ + border_thickness_,
                               bottom_inset - dib_.bmiHeader.biHeight - bottom_right->iHeight_, 1, 1);
        }

        if (!(iFlags_ & 0x40000000)) {
            // Reload frame geometry for each call, as the image does.
            // These callees can observe or change the window's drawing state.
            if (frame_top_) {
                Buffer::tile(frame_top_, 0, 0,
                    frame_top_left_->iWidth_ - border_thickness_, -top_inset,
                    client_rect_.right - client_rect_.left
                        - frame_top_right_->iWidth_ - frame_top_left_->iWidth_,
                    frame_top_->iHeight_);
            }
            if (frame_bottom_) {
                Buffer::tile(frame_bottom_, 0, 0,
                    frame_bottom_left_->iWidth_ - border_thickness_,
                    bottom_inset - dib_.bmiHeader.biHeight - frame_bottom_->iHeight_,
                    client_rect_.right - client_rect_.left
                        - frame_bottom_right_->iWidth_ - frame_bottom_left_->iWidth_,
                    frame_bottom_->iHeight_);
            }
            if (frame_left_) {
                Buffer::tile(frame_left_, 0, 0, -border_thickness_,
                    frame_top_left_->iHeight_ - top_inset, frame_left_->iWidth_,
                    client_rect_.bottom - client_rect_.top
                        - frame_bottom_left_->iHeight_ - frame_top_left_->iHeight_);
            }
            if (frame_right_) {
                // The original uses the top-left height for this origin too.
                Buffer::tile(frame_right_, 0, 0,
                    dib_.bmiHeader.biWidth + border_thickness_ - frame_right_->iWidth_,
                    frame_top_left_->iHeight_ - top_inset, frame_right_->iWidth_,
                    client_rect_.bottom - client_rect_.top
                        - frame_bottom_right_->iHeight_ - frame_top_right_->iHeight_);
            }
        } else if (owned_buffer_) {
            owned_buffer_->copy_mask(this, 0, 0, 0, 0,
                                    dib_.bmiHeader.biWidth, -dib_.bmiHeader.biHeight, 0x108);
        }
    }
    Buffer::set_clip(&saved);
}

// ORIGINAL: 0x005D6A50 ?set_def_nc_sprites@GraphicWin@@QAAXPAUNCSprites@@@Z 0x005D6A50-0x005D6ABA FILE BYTE_EXACT
// symbol    ?set_def_nc_sprites@GraphicWin@@SAXPAUNCSprites@@@Z
// The input's top-right/bottom-left and edge order differ from the shared
// defaults. Preserve each named field, not a byte copy of the input record.
// Original caller 0x004559F5 supplies one pointer and no implicit receiver;
// static class ownership follows the catalogue, not its QAA decoration.
void __cdecl GraphicWin::set_def_nc_sprites(NCSprites *sprites) {
    if (sprites != nullptr) {
        GraphicWinInitDefaults.top_left = sprites->top_left;
        GraphicWinInitDefaults.top_right = sprites->top_right;
        GraphicWinInitDefaults.bottom_left = sprites->bottom_left;
        GraphicWinInitDefaults.bottom_right = sprites->bottom_right;
        GraphicWinInitDefaults.top = sprites->top;
        GraphicWinInitDefaults.left = sprites->left;
        GraphicWinInitDefaults.right = sprites->right;
        GraphicWinInitDefaults.bottom = sprites->bottom;
        GraphicWinInitDefaults.noncaption_left = sprites->noncaption_left;
        GraphicWinInitDefaults.noncaption_right = sprites->noncaption_right;
        GraphicWinInitDefaults.noncaption_middle = sprites->noncaption_middle;
    }
}

// ORIGINAL: 0x005D6EC0 sub_5d6ec0 0x005D6EC0-0x005D6F4B FILE BYTE_EXACT
// symbol    ?load_nc_sprites_from_defs@GraphicWin@@QAEXXZ
// Refresh an existing styled window from the current process defaults.
// This receiver's fields and the init copy use the same data interpretation.
void GraphicWin::load_nc_sprites_from_defs() {
    if (iFlags_ & 0x30000000) {
        frame_top_left_ = GraphicWinInitDefaults.top_left;
        frame_top_right_ = GraphicWinInitDefaults.top_right;
        frame_bottom_left_ = GraphicWinInitDefaults.bottom_left;
        frame_bottom_right_ = GraphicWinInitDefaults.bottom_right;
        frame_top_ = GraphicWinInitDefaults.top;
        frame_bottom_ = GraphicWinInitDefaults.bottom;
        frame_left_ = GraphicWinInitDefaults.left;
        frame_right_ = GraphicWinInitDefaults.right;
        frame_noncaption_left_ = GraphicWinInitDefaults.noncaption_left;
        frame_noncaption_right_ = GraphicWinInitDefaults.noncaption_right;
        frame_noncaption_middle_ = GraphicWinInitDefaults.noncaption_middle;
    }
}

// The query is a predicate, not the receiver for the update. A callback may
// replace the current registry entry or change the live registry count.
// load_interface calls this class-wide operation without a receiver.
// ORIGINAL: 0x005D6F50 ?load_all_nc_sprites_from_defs@GraphicWin@@QAEXXZ 0x005D6F50-0x005D7022 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4559FD calls5D6F50 without setting a receiver. The complete body iterates existing WinZOrderArray9B6630 with signed live WinZOrderCount9B7B30; slot61 get_graphic_win is only a predicate and the array receiver is reloaded after its callback.", "The existing real eleven-Sprite-pointer GraphicWinInitDefaults9B3394 record and load_nc_sprites_from_defs5D6EC0 implement the identical flags30000000 guarded copy. Preserve seed history/exact obligation and observe the emitted static function symbol."], "reason": "Recover genuine GraphicWin frame behavior required by the interface asset loader, retaining its original seed identity."}
// symbol    ?load_all_nc_sprites_from_defs@GraphicWin@@SAXXZ
void GraphicWin::load_all_nc_sprites_from_defs() {
    int index = 0;
    if (0 < WinZOrderCount) {
        Win **cursor = WinZOrderArray;
        do {
            if ((*cursor)->get_graphic_win() != nullptr) {
                Win *window = *cursor;
                // A callback can replace the entry with a plain unstyled Win.
                // Test the base flags before assuming a GraphicWin receiver.
                if (window->iFlags_ & 0x30000000) {
                    GraphicWin *graphic = static_cast<GraphicWin *>(window);
                    graphic->frame_top_left_ = GraphicWinInitDefaults.top_left;
                    graphic->frame_top_right_ = GraphicWinInitDefaults.top_right;
                    graphic->frame_bottom_left_ = GraphicWinInitDefaults.bottom_left;
                    graphic->frame_bottom_right_ = GraphicWinInitDefaults.bottom_right;
                    graphic->frame_top_ = GraphicWinInitDefaults.top;
                    graphic->frame_bottom_ = GraphicWinInitDefaults.bottom;
                    graphic->frame_left_ = GraphicWinInitDefaults.left;
                    graphic->frame_right_ = GraphicWinInitDefaults.right;
                    graphic->frame_noncaption_left_ = GraphicWinInitDefaults.noncaption_left;
                    graphic->frame_noncaption_right_ = GraphicWinInitDefaults.noncaption_right;
                    graphic->frame_noncaption_middle_ = GraphicWinInitDefaults.noncaption_middle;
                }
            }
            ++index;
            ++cursor;
        } while (index < WinZOrderCount);
    }
}

// x86 geometry adds wrap at 32 bits before signed comparison. Unsigned
// addition avoids introducing signed-overflow UB for damaged sprite sizes.
static int frame_dimension_sum(int first, int second) {
    return static_cast<int>(static_cast<unsigned int>(first)
                          + static_cast<unsigned int>(second));
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005D7030 ?compute_min_size@GraphicWin@@QAEXXZ 0x005D7030-0x005D7140
// RECOVERY_BINDING: v1 {"evidence": ["Complete original5D7030..5D7140 has no callees. Flag0x10 selects caption or noncaption top corners; bottom corners are shared. Every selected pointer must exist before either previous minimum changes.", "OriginalSprite displayed width18/height1C accesses and signed greater-than branches establish independent horizontal and vertical corner-sum maxima. Preserve low32-bit addition before signed comparison, real Sprite accessors and existing Win minimum members. Seed Buffer receiver casts are incorrect; remove the product pending forwarder without changing the historical unclaimed identity."], "reason": "Recover genuine GraphicWin frame behavior required by the interface asset loader, retaining its original seed identity."}
// symbol    ?compute_min_size@GraphicWin@@QAEXXZ
void GraphicWin::compute_min_size() {
    Sprite *top_left = (iFlags_ & 0x10) ? frame_top_left_ : frame_noncaption_left_;
    Sprite *top_right = (iFlags_ & 0x10) ? frame_top_right_ : frame_noncaption_right_;
    Sprite *bottom_left = frame_bottom_left_;
    Sprite *bottom_right = frame_bottom_right_;
    if (!top_left || !top_right || !bottom_left || !bottom_right) {
        return;
    }

    int width = frame_dimension_sum(top_left->width(), top_right->width());
    field_134_ = width;
    int bottom_width = frame_dimension_sum(bottom_right->width(), bottom_left->width());
    if (bottom_width > width) {
        field_134_ = bottom_width;
    }
    int height = frame_dimension_sum(top_left->height(), bottom_left->height());
    field_138_ = height;
    int right_height = frame_dimension_sum(bottom_right->height(), top_right->height());
    if (right_height > height) {
        field_138_ = right_height;
    }
}

// ORIGINAL: 0x005D5CF0 ?on_paint@GraphicWin@@QAEHPAURECT@@@Z 0x005D5CF0-0x005D620A
// RECOVERY_BINDING: v1 {"evidence": ["GraphicWin table66FC50 slot73 and EditBox table66A038 slot73 both select5D5CF0, with an unadjusted Win receiver and Buffer base at444. The original caller ignores a return; use the existing Win void paint signature.", "Original5D5E04 and5D5E56 establish destination clipping;5D61D4 rereads WinBackBuffer before restoring its clip.5D61CF passes width then height to Buffer copy. Real surface Blt and software drawing own separate branches."], "reason": "Recover the inherited GraphicWin paint override selected by the original tables; keep the seed and its original span."}
// TRIED: transcribed from Ghidra's pseudocode (cross-checked several branches against the raw disassembly: win_parent_/iFlags_/ outer_rect_/client_rect_ offsets match the Win layout confirmed on 0x0050FB10/0x00510110; DAT_00696d14 in Ghidra's arg lists is a BYTE VALUE passed as int, not a pointer, confirmed by the draw()/draw_dest() signatures). Reusing one RECT local instead of two (matching the original's stack-slot reuse across the mutually-exclusive branches) made no measurable difference (sim stayed 0.49 at plain /O2). First divergence #22 is register scheduling in the client_to_screen/nonclient_to_screen setup, not chased further. Not byte-exact.
// working copy - scaffold materialised by --work
// symbol    ?on_paint@GraphicWin@@UAEXPAUtagRECT@@@Z
// size      1306 bytes
// prototype int (__thiscall ?on_paint@GraphicWin@@QAEHPAURECT@@@Z)(GraphicWin* this, RECT*)
// callers   0   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005D84C0 0x005D8B20 0x005D94B0 0x005DFF00 0x005ECFE0 0x005ED170 0x005ED240
// indirect  0x005D5DF0 0x005D5FD7

// Slot 73 in both the GraphicWin and EditBox original tables selects this
// handler. Clipping belongs to the destination, not this window's buffer.
// The no-parent branch intentionally clips ScreenBuffer even with a back buffer.
void GraphicWin::on_paint(RECT *area) {
    int origin_x = 0;
    int origin_y = 0;
    client_to_screen(&origin_x, &origin_y);
    RECT rectangle;
    Win *parent = win_parent_;
    if (parent != 0 && (iFlags_ & 0x20) != 0) {
        if (iFlags_ & 0x8000) {
            rectangle = parent->client_rect_;
            rectangle.right -= rectangle.left;
            rectangle.bottom -= rectangle.top;
            rectangle.left = rectangle.top = 0;
            parent->nonclient_to_screen(&rectangle);
        } else {
            rectangle = parent->outer_rect_;
            rectangle.right -= rectangle.left;
            rectangle.bottom -= rectangle.top;
            rectangle.left = rectangle.top = 0;
            parent->client_to_screen(&rectangle);
        }
        if (area != 0 && !IntersectRect(&rectangle, &rectangle, area)) {
            return;
        }
        if (WinBackBuffer != 0) {
            rectangle.left -= WinViewOriginX;
            rectangle.right -= WinViewOriginX;
            rectangle.top -= WinViewOriginY;
            rectangle.bottom -= WinViewOriginY;
            WinBackBuffer->set_clip(&rectangle);
        } else {
            ScreenBuffer.set_clip(&rectangle);
        }
    } else if (area != 0) {
        ScreenBuffer.set_clip(area);
    }

    if ((iFlags_ & 0x1000000) == 0 && surface_ != 0) {
        rectangle.left = origin_x;
        rectangle.top = origin_y;
        rectangle.right = origin_x + outer_rect_.right - outer_rect_.left;
        rectangle.bottom = origin_y + outer_rect_.bottom - outer_rect_.top;
        // Existing Buffer surfaces use the Surface1 prefix; the shared
        // device surface is Surface2. SDK slots through Blt are identical.
        DirectDrawBackBuffer->Blt(&rectangle,
            reinterpret_cast<IDirectDrawSurface2 *>(surface_), 0, DDBLT_WAIT, 0);
    } else {
        Buffer *destination = &ScreenBuffer;
        int x = origin_x;
        int y = origin_y;
        if (WinBackBuffer != 0) {
            destination = WinBackBuffer;
            x -= WinViewOriginX;
            y -= WinViewOriginY;
        }
        if ((iFlags_ & 0x1000000) == 0) {
            Buffer::copy(destination, 0, 0, x, y,
                         outer_rect_.right - outer_rect_.left,
                         outer_rect_.bottom - outer_rect_.top);
        } else if (field_A0_ != 0) {
            Buffer::draw_dest(destination, WinFillColour, x, y, 1, 1,
                              reinterpret_cast<unsigned char *>(field_A0_));
        } else if (field_A4_ != 0) {
            Buffer::draw_multi_table_dest(destination, WinFillColour, x, y, 1, 1,
                                          reinterpret_cast<unsigned char *>(field_A4_));
        } else {
            Buffer::draw(destination, WinFillColour, x, y, 1, 1);
        }
    }
    // Drawing can change the back buffer. The original rereads it here.
    if (WinBackBuffer != 0) {
        WinBackBuffer->restore_clip();
    } else {
        ScreenBuffer.restore_clip();
    }
}

// Shared surface callback: GraphicWin constructor5D4D2B installs
// table66FC50 slot63 ->404290; AlphaMovie/MapWin/PlanWin/Console inherit it.
// The catalogue named AlphaMovie, but this body uses the GraphicWin receiver.
/*
// ORIGINAL: 0x00404290 ?update@AlphaMovie@@QAEXXZ 0x00404290-0x00404298 BYTE_EXACT
// symbol    ?on_update@GraphicWin@@UAEXXZ
// size      8 bytes
// prototype void (__thiscall ?update@AlphaMovie@@QAEXXZ)(AlphaMovie* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D56B0
Status: Complete
*/
void GraphicWin::on_update() {
    update(0);
}

// Superseded seed deleted; binding references retain historical provenance.
// ORIGINAL: 0x005D68C0 sub_5d68c0 0x005D68C0-0x005D6A42
// RECOVERY_BINDING: v1 {"evidence": ["task:graphicwin-frame-sprite-owner original identity/spans and all seed lessons retained from src/unrecovered/005d68c0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:5D68C0..5D6A42 complete native stores, guards and signed jg/equality joins.", "source:src/graphicwin.h existing NCSprites and frame fields; Sprite width18/height1C and Win minima134/138; compute_min_size5D7030 corroborates selection and wrap semantics."], "reason": "Recover complete instance frame-sprite publication in genuine GraphicWin with existing NCSprites, Sprite accessors and Win minimum members. apply_frame_sprites is a provisional semantic name; preserve source input order, caption-bit selection, null guards and low32-bit sums before signed comparison."}
// symbol    ?apply_frame_sprites@GraphicWin@@QAEXPBUNCSprites@@@Z
// kind      game
// TRIED: none confirmed byte-exact; MISMATCH #44 'mov' vs 'jmp' - the IDA-guessed __stdcall(int) contradicts pervasive [ecx+N] reads with no matching stack slot, so this landed as a __thiscall member of a synthetic owner class instead (PROPOSAL below).
// TRIED: flipping the `(flags & 0x10) == 0` polarity to `!= 0` and swapping which field group goes in which arm moved the first divergence from #31 to #44 (similarity 0.70 -> 0.83).
void GraphicWin::apply_frame_sprites(const NCSprites *sprites) {
    if (sprites == 0) {
        return;
    }
    frame_top_left_ = sprites->top_left;
    frame_top_right_ = sprites->top_right;
    frame_bottom_left_ = sprites->bottom_left;
    frame_bottom_right_ = sprites->bottom_right;
    frame_top_ = sprites->top;
    frame_left_ = sprites->left;
    frame_right_ = sprites->right;
    frame_bottom_ = sprites->bottom;
    frame_noncaption_left_ = sprites->noncaption_left;
    frame_noncaption_right_ = sprites->noncaption_right;
    frame_noncaption_middle_ = sprites->noncaption_middle;

    Sprite *const top_left = (iFlags_ & 0x10) ? frame_top_left_ : frame_noncaption_left_;
    Sprite *const top_right = (iFlags_ & 0x10) ? frame_top_right_ : frame_noncaption_right_;
    Sprite *const bottom_left = frame_bottom_left_;
    Sprite *const bottom_right = frame_bottom_right_;
    if (!top_left || !top_right || !bottom_left || !bottom_right) {
        return;
    }

    int width = frame_dimension_sum(top_left->width(), top_right->width());
    field_134_ = width;
    const int bottom_width = frame_dimension_sum(bottom_right->width(), bottom_left->width());
    if (width <= bottom_width) {
        width = bottom_width;
    }
    field_134_ = width;

    int height = frame_dimension_sum(top_left->height(), bottom_left->height());
    field_138_ = height;
    const int right_height = frame_dimension_sum(bottom_right->height(), top_right->height());
    if (height <= right_height) {
        height = right_height;
    }
    field_138_ = height;
}

// ORIGINAL: 0x005D54A0 sub_5d54a0 0x005D54A0-0x005D54C2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:graphicwin-buffer-input-owner", "pinned:5D54A0/5D54D0 adjust GraphicWin receiver to Buffer at +444; 5D7AE0 forwards unchanged receiver to load_pcx.", "pinned:5D5590 virtual Win resize slot3, failure return, then move and Buffer palette synchronization in that order.", "pinned:5D7AE0 input fields +C/+10 only; PcxInputView is an accessed-prefix description, not a proven complete allocation or original name.", "historical seed:src/recovered/005d54a0.cpp; original mappings and parsed findings preserved."], "reason": "Rehome the original operation into genuine GraphicWin/Buffer inheritance and methods, preserving historical identity, spans, claims and findings."}
// symbol    ?probe_image_file@GraphicWin@@QAEHPBDPAVPalette@@HH@Z
int GraphicWin::probe_image_file(const char *filename, Palette *palette,
                                 int color_base, int color_count) {
  return Buffer::probe_image_file(filename, palette, color_base, color_count);
}

// ORIGINAL: 0x005D54D0 sub_5d54d0 0x005D54D0-0x005D54F2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:graphicwin-buffer-input-owner", "pinned:5D54A0/5D54D0 adjust GraphicWin receiver to Buffer at +444; 5D7AE0 forwards unchanged receiver to load_pcx.", "pinned:5D5590 virtual Win resize slot3, failure return, then move and Buffer palette synchronization in that order.", "pinned:5D7AE0 input fields +C/+10 only; PcxInputView is an accessed-prefix description, not a proven complete allocation or original name.", "historical seed:src/recovered/005d54d0.cpp; original mappings and parsed findings preserved."], "reason": "Rehome the original operation into genuine GraphicWin/Buffer inheritance and methods, preserving historical identity, spans, claims and findings."}
// symbol    ?load_pcx_from_source@GraphicWin@@QAEHPAUPcxInputView@@PAVPalette@@HH@Z
int GraphicWin::load_pcx_from_source(PcxInputView *source, Palette *palette,
                                     int color_base, int color_count) {
  return Buffer::load_pcx_from_source(source, palette, color_base,
                                      color_count);
}

// ORIGINAL: 0x005D5590 sub_5d5590 0x005D5590-0x005D55CC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:graphicwin-buffer-input-owner", "pinned:5D54A0/5D54D0 adjust GraphicWin receiver to Buffer at +444; 5D7AE0 forwards unchanged receiver to load_pcx.", "pinned:5D5590 virtual Win resize slot3, failure return, then move and Buffer palette synchronization in that order.", "pinned:5D7AE0 input fields +C/+10 only; PcxInputView is an accessed-prefix description, not a proven complete allocation or original name.", "historical seed:src/recovered/005d5590.cpp; original mappings and parsed findings preserved."], "reason": "Rehome the original operation into genuine GraphicWin/Buffer inheritance and methods, preserving historical identity, spans, claims and findings."}
// symbol    ?resize_screen_and_sync_palette@GraphicWin@@QAEHXZ
int GraphicWin::resize_screen_and_sync_palette() {
  const int result = resize(WinScreenWidth, WinScreenHeight, 0);
  if (result != 0) {
    return result;
  }
  Win::move(0, 0);
  Buffer::sync_to_palette(PaletteActive);
  return 0;
}

// The source declares the 33-word accessed window required by the pinned reads;
// neither the original allocation's upper bound nor its former owner is proven.
// RECOVERY_OBJECT: v1 0x00696B6C ?GraphicWinBlitPolynomialWords@@3PAIA
unsigned int GraphicWinBlitPolynomialWords[33] = {
#include "graphicwin_palette_polynomials.inc"
};

// ORIGINAL: 0x005D5B80 sub_5d5b80 0x005D5B80-0x005D5C4E
// RECOVERY_BINDING: v1 {"evidence":["Pinned5D5B80 uses ECX as the same receiver for Win calls and passes GraphicWin+444 as Buffer receiver.","GraphicWin's existing Win,Buffer bases and Buffer DIB width/height fields account for all observed object reads.","Source uses existing PaletteActive, Buffer methods, Win methods, bit_count_signed and BitBlt import; no code-address dependency.","This source-owned 33-word window is initialized from a pinned-data generated include; only the indexed window is asserted, not the original allocation upper bound or former owner."],"reason":"Provisional descriptive GraphicWin owner for the complete palette-synchronized LFSR row blit; preserve the native branches, lock cleanup and row recurrence."}
// symbol    ?blit_palette_pattern_rows@GraphicWin@@QAEXXZ
// kind game
// TRIED: Win::get_hdc/release_hdc called w/ explicit this (mismatch at implicit-ecx call sites); LFSR loop as do/while matching asm
// Original EAX is call residue on several return paths; no result consumer or
// original method spelling is proven. This provisional void API adds no result.
void GraphicWin::blit_palette_pattern_rows() {
    Buffer *const buffer = static_cast<Buffer *>(this);
    buffer->sync_to_palette(PaletteActive);
    sync_palette();
    PaletteActive->set();

    HDC const window_dc = Win::get_hdc();
    if (window_dc == 0) {
        return;
    }

    HDC const buffer_dc = buffer->get_hdc();
    if (buffer_dc == 0) {
        Win::release_hdc();
        return;
    }

    int const rows = -buffer->dib_.bmiHeader.biHeight;
    int const width = buffer->dib_.bmiHeader.biWidth;
    unsigned int const polynomial =
        GraphicWinBlitPolynomialWords[bit_count_signed(rows)];

    unsigned int y = 1;
    do {
        if (static_cast<int>(y) < rows) {
            BitBlt(window_dc, 0, static_cast<int>(y), width, 1,
                   buffer_dc, 0, static_cast<int>(y), SRCCOPY);
        }
        if ((y & 1U) == 0) {
            y = (y >> 1) ^ polynomial;
        } else {
            y >>= 1;
        }
    } while (y != 1);

    Win::release_hdc();
    buffer->release_hdc(static_cast<int>(y));
}

// ORIGINAL: 0x005D5B70 sub_5d5b70 0x005D5B70-0x005D5B75 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned5D5B70 is one E9 direct tail jump to5D5B80 and preserves ECX and stack.","No original C++ method name, direct caller or sampled virtual slot is proven; source forwarding member is provisional pending compiler output."],"reason":"Retain the full original forwarding identity in the genuine GraphicWin owner without an executable-address alias or hand-written jump."}
// symbol    ?blit_palette_pattern_rows_entry@GraphicWin@@QAEXXZ
// kind thunk
// LEVER: plain tailcall to a declared nullary callee; last-statement return
void GraphicWin::blit_palette_pattern_rows_entry() {
    blit_palette_pattern_rows();
}
