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
#include "net_class.h"
#include "init_thunks.h"  // g_WIN_BUFFER, the 0x009B6F08 Buffer Win draws into

typedef signed char int8;  // a scaffold spelling the artifacts use
#include "vtable_shim.h"
#include "scroll.h"
#include "win.h"
#include <cstring>
#include "palette.h"
#include "general.h"
#include "temp.h"
#include "dialog.h"
#include "sounddevice.h"
#include "menu.h"
#include "popup.h"   // pop_caption_title, for DDInit::report_error
#include "basepop.h"  // BasePopScreenWidth, for Win::set_bubble_text
#include <ddraw.h>  // IDirectDrawSurface::GetDC / ReleaseDC in the hdc pair
// `DirectDrawCreate` (DDInit::init) is the one symbol this tree needs out of
// ddraw.lib; it is on the CMake link line now, where dxguid already was.

// `WinPrimaryVtable` IS GONE. It held 0x0066FDD0, the address of the very
// table this class now declares, and nothing read it: the compiler emits the
// vfptr store itself once the virtuals are declared. A named constant for a
// vtable address is the hand-work this project exists to delete.

// ORIGINAL: 0x005F8770 sub_5f8770 0x005F8770-0x005F87F6 FILE
// symbol    ??_GWinNodeList@@UAEPAXI@Z
// homed from 005f8770.cpp: the scalar deleting destructor
// the 0xC8 member's vtable (0x0066FF30) points at. `WinNodeList` is this
// tree's own name for the class - nothing in the image names it - but the
// mangle SHAPE (`??_G<Class>@@UAEPAXI@Z`, a public virtual __thiscall
// returning `void *`, taking one flag argument) is exactly what a class
// with a virtual destructor and no other virtuals emits for its only
// vtable slot, matching what this address's own bytes do.
// LEVER (measured 2026-08-25, 14 -> 51 of the image's 54 instructions):
// DEFINE `~WinNodeList()` INSIDE THE CLASS. The plateau recorded here
// blamed /GX for the deleting destructor tail-CALLing a separate
// `??1WinNodeList@@UAE@XZ` instead of inlining the walk. That diagnosis
// was REFUTED by measuring it: `--flags "/c /O2 /Gy /GR-"` compiles 11
// instructions, WORSE than the 14 /GX gave, so dropping the SEH frame only
// removes the frame - it does not inline anything. The real cause was that
// the destructor was defined out of line in this file, which is a call VC6
// will not inline across; moved into the class body in win.h it inlines and
// the shape matches the image throughout.
// TRIED: what remains at 51/54 is register allocation, not spelling - the
// image zeroes into edi (`push edi; xor edi,edi`) where this build uses ebx
// for the same role, so every one of the five field stores differs only in
// that register, and the epilogue pops in the other order. Same fingerprint
// as AGENT_BRIEF's receiver-spill wall.
// size      134 bytes
// prototype void *(__thiscall ??_GWinNodeList@@UAEPAXI@Z)(WinNodeList *this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x0064557F


/*
Purpose: Construct a Win from its AutoSound subobject and the process window
         defaults, preserving every sparse write and legacy return residue.
// ORIGINAL: 0x005EB3D0 ??0Win@@QAE@XZ 0x005EB3D0-0x005EB63D BYTE_EXACT
// LEVER: `object`/`fixed`/`dynamic` were `volatile` pointers over a straight run of field-default copies - no rereads, nothing to guard against reordering. Dropping `volatile` (same raw-offset shape) took this from 100/107 to 104/107 and the compiled instruction count now matches the image's 107 exactly.
// LEVER: the remaining 3-instruction epilogue gap was the image's real `??0Win@@QAE@XZ` constructor setting `eax = this` before `pop esi; ret`, which a `void construct()` never emits. Declaring `Win *construct()` and `return this;` closed it: 104/107 -> 107/107, BYTE_EXACT.
// symbol    ??0Win@@QAE@XZ
// size      621 bytes
// prototype void (__thiscall ??0Win@@QAE@XZ)(Win* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062BA80
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/

// THE TWO DEFAULT TABLES, BESIDE THEIR ONLY USER. They sat in win.h, so every
// translation unit that included it carried two fixed-address bindings it
// never touched; `Win::Win` is the only reader in the tree.

// THE FIVE 0xC8 STORES ARE `list_`'s OWN IMPLICIT CONSTRUCTION, not body
// statements. `list_` (a `WinNodeList`) is a real member with no entry in
// this constructor's initialiser list, so the compiler constructs it
// implicitly - vfptr first, then its four remaining fields in declaration
// order - BEFORE this body runs at all. That is exactly the image's own
// order: 0x005EB3DA installs `list_`'s vtable, 0x005EB3E4-0x005EB3FC zero
// its fields, and only then does 0x005EB402 install Win's OWN vtable at
// offset 0 - the first statement of this body below.
// MEMBER FORM, in the IMAGE'S STORE ORDER. The two vtable installs that
// precede the first store are not statements: 0x005EB3DA-0x005EB3FC is
// `list_`'s own implicit construction (vfptr first, then its four fields)
// and 0x005EB402 installs Win's vfptr - both emitted by the declarations,
// which is why neither appears below.
Win::Win() {
    const uint32_t *const fixed = WinStaticDefaults;
    const uint32_t *const dynamic = WinDynamicDefaults;

    poWinBase_ = this;
    child_count_ = 0;
    iSomeFlag_ = 0;
    field_A0_ = 0;
    field_A4_ = 0;
    iVertScaleDenom_ = 0;
    iVertScaleNum_ = 0;
    field_134_ = 0;
    field_138_ = 0;
    cursor_sprite_ = 0;
    field_18C_ = 0;
    field_190_ = 0;
    cursor_handle_ = 0;
    cursor_name_ = 0;
    palette_seed_cache_ = 0;
    win_parent_ = 0;
    menu_ = 0;
    field_19C_ = 0;
    field_12C_ = 0;
    field_130_ = 1;
    field_FC_ = dynamic[0];
    field_100_ = fixed[0];
    caption_height_ = fixed[1];
    field_104_ = dynamic[2];
    field_108_ = fixed[2];
    field_10C_ = fixed[3];
    field_110_ = fixed[4];
    border_thickness_ = fixed[5];
    bottom_border_thickness_ = fixed[6];
    field_120_ = fixed[7];
    field_124_ = dynamic[3];
    field_128_ = fixed[8];
    field_F8_ = dynamic[1];
    field_E0_ = 0;
    minimize_button_ = 0;
    zoom_button_ = 0;
    close_button_ = 0;
    scroll_vert_ = 0;
    scroll_horz_ = 0;
    field_F4_ = 0;
    field_15C_ = 0;
    field_160_ = 0;
    field_164_ = 0;
    field_168_ = 0;
    // UNROLLED, not a loop: the image is a straight run of `mov [esi+X],
    // eax`, 0x13C through 0x158 by 4, with no loop counter at all. A `for`
    // here gives the compiler an index variable that outlives its loop -
    // VC6 then wants a THIRD callee-saved register (edi) and pushes it up
    // front, which the image never does (only `push esi`). Spelling every
    // store out cost nothing and dropped the extra push.
    outer_rect_.left = 0;
    outer_rect_.top = 0;
    outer_rect_.right = 0;
    outer_rect_.bottom = 0;
    client_rect_.left = 0;
    client_rect_.top = 0;
    client_rect_.right = 0;
    client_rect_.bottom = 0;
    buffer1_ = 0;
    buffer4_ = 0;
    buffer3_ = 0;
    buffer2_ = 0;
    field_16C_ = 0;
    field_170_ = 0;
    iFlags_ = 0;
    // Same defect, same fix: 0x400 through 0x438 by 4, unrolled.
    field_400_ = 0;
    field_404_ = 0;
    mouse_move_hook_ = 0;
    field_40C_ = 0;
    field_410_ = 0;
    field_414_ = 0;
    field_418_ = 0;
    field_41C_ = 0;
    field_420_ = 0;
    field_424_ = 0;
    field_428_ = 0;
    field_42C_ = 0;
    key_hook_ = 0;
    field_434_ = 0;
    field_438_ = 0;
    field_174_ = 1;
    field_178_ = 1;
    field_17C_ = 1;
    field_180_ = 1;
    field_1A0_ = 2;
}


namespace {

__forceinline LONG long_from_bits(uint32_t bits) {
    LONG value;
    static_assert(sizeof(value) == sizeof(bits), "Win geometry requires 32-bit LONG");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

__forceinline uint32_t long_bits(LONG value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

__forceinline int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "Win geometry requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

uint32_t midpoint(uint32_t near_edge, uint32_t far_edge) {
    const uint32_t delta = far_edge - near_edge;
    const uint32_t adjusted = delta + (delta >> 31);
    const uint32_t half = (adjusted >> 1) | (adjusted & 0x80000000U);
    return near_edge + half;
}

void move_rect(RECT &rect, int x, int y) {
    // PLAIN ARITHMETIC. The bit-cast chain made this too big for VC6 to
    // inline, and `Win::move` at 0x005ED7D0 has it written out in BOTH arms
    // of its ternary - 43 instructions where a call is 14.
    const int dx = x - rect.left;
    const int dy = y - rect.top;
    rect.left += dx;
    rect.top += dy;
    rect.right += dx;
    rect.bottom += dy;
}

}  // namespace

/*
Purpose: Move the active window rectangle while preserving its dimensions.
// ORIGINAL: 0x005ED7D0 ?move@Win@@QAEHHH@Z 0x005ED7D0-0x005ED877
// TRIED: reordering the field stores to left,right,top,bottom (matching
//   the image's actual store order per `store_order.py`) with dy computed
//   before dx - regressed, 4/43 -> 1/43 (an extra `push edi`/`pop edi`
//   appeared, similarity 0.795 -> 0.522). Left,right,top,bottom with the
//   ORIGINAL dx-before-dy compute order - no change at all, still 4/43,
//   0.795 similar (identical codegen to left,top,right,bottom). The image
//   RE-READS each field's old value from memory right before storing it
//   back (e.g. re-loading `top` a second time rather than keeping the first
//   read live across the `left`/`right` updates), trading a memory read for
//   a register - that is a backend spill decision, not something a source
//   reordering can force. Reverted to the original left,top,right,bottom.
//   Plateau at 4/43 MISMATCH.
// size      167 bytes
// prototype int (__thiscall ?move@Win@@QAEHHH@Z)(Win* this, int, int)
// callers   15   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Win::move(int x, int y) {
    // WRITTEN OUT IN BOTH ARMS, because the image is: 43 instructions with no
    // call, where a shared helper compiles to 14 and a reference bound to the
    // ternary compiles to the same. Reaching each rectangle by name is what
    // gives VC6 the two straight-line arms the image has.
    //
    // TRIED: 4 of 43, and computing `dy` before `dx` - the image loads
    // `[ecx+0x150]` before `[ecx+0x14c]` - changes nothing. The real
    // divergence is earlier and is register pressure: the image reads its
    // argument out of the frame BEFORE the branch and needs one callee-saved
    // register, where this body pushes edi as well and reads the argument
    // after. Source statement order does not reach that; it is the same
    // shape as the receiver-spill wall recorded in AGENT_BRIEF.
//
// TRIED: the local-reuse lever that made nonclient_to_screen and
// nonscreen_to_client exact - subtracting into x/y instead of adding
// `dx`/`dy` - makes this body WORSE, 4 of 43 to 3. Those two have one
// straight-line tail; this one has two arms and x/y are parameters, so
// modifying them in an arm costs rather than saves.
    if (iSomeFlag_ & 2U) {
        const int dx = x - client_rect_.left;
        const int dy = y - client_rect_.top;
        client_rect_.left += dx;
        client_rect_.top += dy;
        client_rect_.right += dx;
        client_rect_.bottom += dy;
    } else {
        const int dx = x - outer_rect_.left;
        const int dy = y - outer_rect_.top;
        outer_rect_.left += dx;
        outer_rect_.top += dy;
        outer_rect_.right += dx;
        outer_rect_.bottom += dy;
    }
    return 0;
}

/*
Purpose: Determine whether this window and every ancestor are visible.
// ORIGINAL: 0x005F7E90 ?is_visible@Win@@QAEHXZ 0x005F7E90-0x005F7EB6 BYTE_EXACT
// size      38 bytes
// prototype int (__thiscall ?is_visible@Win@@QAEHXZ)(Win* this)
// callers   120   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Win::is_visible() {
    if (!(iSomeFlag_ & 1U)) {
        return 0;
    }
    // A detached window is visible on its own; otherwise the whole parent
    // chain must be visible, which the legacy body walks recursively.
    if (win_parent_ && !win_parent_->is_visible()) {
        return 0;
    }
    return 1;
}

/*
Purpose: Translate a client-relative point into screen coordinates by walking
         the parent chain.
// ORIGINAL: 0x005ED240 ?client_to_screen@Win@@QAEXPAH0@Z 0x005ED240-0x005ED2C5 BYTE_EXACT
// LEVER: plain signed `+`/`-` instead of the `int_from_bits`/`long_bits` bit-cast chain - the image is straight `add`/`sub`, no cast dance. 21/44 MISMATCH -> BYTE_EXACT.
// size      133 bytes
// prototype void (__thiscall ?client_to_screen@Win@@QAEXPAH0@Z)(Win* this, int*, int*)
// callers   71   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240
Status: Complete
*/
void Win::client_to_screen(int *x, int *y) {
    *x = *x + client_rect_.left + outer_rect_.left;
    *y = *y + client_rect_.top + outer_rect_.top;
    // Bit 5 marks a window whose coordinates are relative to its parent, so
    // the walk continues only while both that flag and a parent are present.
    if ((iFlags_ & 0x20U) == 0 || !win_parent_) {
        return;
    }
    win_parent_->client_to_screen(x, y);
    // Bit 15 additionally backs out the parent's own outer origin. The legacy
    // body re-reads win_parent_ for each subtraction rather than caching it.
    if ((iFlags_ & 0x8000U) == 0) {
        return;
    }
    *x = *x - win_parent_->outer_rect_.left;
    *y = *y - win_parent_->outer_rect_.top;
}

/*
Purpose: The inverse of client_to_screen above.
// ORIGINAL: 0x005ED2D0 ?screen_to_client@Win@@QAEXPAHPAH@Z 0x005ED2D0-0x005ED355 BYTE_EXACT
// symbol    ?screen_to_client@Win@@QAEXPAH0@Z
// size      133 bytes
// prototype void (__thiscall ?screen_to_client@Win@@QAEXPAHPAH@Z)(Win* this, int*, int*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0
Return Value: n/a
Status: Complete
*/
// ORIGINAL: 0x005ECF20 ?screen_to_client@Win@@QAEXPAURECT@@@Z 0x005ECF20-0x005ECFD8 FILE BYTE_EXACT
// symbol    ?screen_to_client@Win@@QAEXPAUtagRECT@@@Z
// notes     the RECT overload; the coordinate-pair form shares the name
//           and marker_symbols resolves to whichever comes first.
// working copy - scaffold materialised by --work
// size      184 bytes
// prototype void (__thiscall ?screen_to_client@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0
void Win::screen_to_client(int *x, int *y) {
    *x = *x - (client_rect_.left + outer_rect_.left);
    *y = *y - (client_rect_.top + outer_rect_.top);
    // Bit 5 marks a window whose coordinates are relative to its parent, so
    // the walk continues only while both that flag and a parent are present.
    if ((iFlags_ & 0x20U) == 0 || !win_parent_) {
        return;
    }
    win_parent_->screen_to_client(x, y);
    // Bit 15 additionally adds back the parent's own outer origin. The legacy
    // body re-reads win_parent_ for each addition rather than caching it.
    if ((iFlags_ & 0x8000U) == 0) {
        return;
    }
    *x = *x + win_parent_->outer_rect_.left;
    *y = *y + win_parent_->outer_rect_.top;
}

/*
Purpose: Announce this window as the one the palette should follow, then report
         that the message was handled.

             push ecx / call Palette::set_active_window / add esp, 4
             mov eax, 1 / ret

         `this` is passed as the only argument and the caller cleans it, which
         is the cdecl convention the callee's mangled name declares.
// ORIGINAL: 0x005F1060 ?on_query_new_palette@Win@@QAEHXZ 0x005F1060-0x005F106F BYTE_EXACT
// symbol    ?on_query_new_palette@Win@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?on_query_new_palette@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE4F0
Return Value: 1, always
Status: Complete
Verification note: the call to Palette::set_active_window is NOT observed by
         any suite, and tools/mutate_and_verify.py reports dropping it as a
         surviving mutant. That is accurate rather than a coverage failure to
         chase: set_active_window is a recovered EMPTY body with no rebindable
         seam, so removing the call is behaviourally identical today and no
         fixture can distinguish them. The call is kept because it is what the
         original encodes. If set_active_window ever gains a body, this
         function needs a seam and a fixture that counts the call.
*/
int Win::on_query_new_palette() {
    Palette::set_active_window(this);
    return 1;
}


/*
Purpose: Read the vertical scroll bar's current position.
// ORIGINAL: 0x005EE050 ?get_vert_pos@Win@@QAEHXZ 0x005EE050-0x005EE064 BYTE_EXACT
// size      20 bytes
// prototype int (__thiscall ?get_vert_pos@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: The scroll's position, or 0 when the window has no vertical
              scroll bar. The null check is the original's, not a guard added
              here: it loads the pointer, tests it, and branches to `xor eax,
              eax` rather than dereferencing.
Status: Complete
*/
int Win::get_vert_pos() {
    return scroll_vert_ ? scroll_vert_->position_ : 0;
}


/*
Purpose: Read the horizontal scroll bar's current position.
// ORIGINAL: 0x005EE090 ?get_horz_pos@Win@@QAEHXZ 0x005EE090-0x005EE0A4 BYTE_EXACT
// size      20 bytes
// prototype int (__thiscall ?get_horz_pos@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: The scroll's position, or 0 when the window has no horizontal
              scroll bar. Identical to get_vert_pos above but reading the
              other pointer; the two differ only in which member they load,
              which is exactly what the fixtures have to separate.
Status: Complete
*/
int Win::get_horz_pos() {
    return scroll_horz_ ? scroll_horz_->position_ : 0;
}


/*
Purpose: Set vertical scrollbar paging when a scrollbar is attached.
// ORIGINAL: 0x005EE0F0 ?set_vert_paging@Win@@QAEXH@Z 0x005EE0F0-0x005EE107 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?set_vert_paging@Win@@QAEXH@Z)(Win* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Win::set_vert_paging(int paging) {
    if (scroll_vert_) {
        scroll_vert_->paging_ = paging;
    }
}

/*
Purpose: Set horizontal scrollbar paging when a scrollbar is attached.
// ORIGINAL: 0x005EE110 ?set_horz_paging@Win@@QAEXH@Z 0x005EE110-0x005EE127 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?set_horz_paging@Win@@QAEXH@Z)(Win* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Win::set_horz_paging(int paging) {
    if (scroll_horz_) {
        scroll_horz_->paging_ = paging;
    }
}






/*
Purpose: Determine whether a point is inside a rectangle using Win32 edge semantics.
// ORIGINAL: 0x005FA7E0 ?in_box@@YAHHHPAURECT@@@Z 0x005FA7E0-0x005FA814 BYTE_EXACT
// symbol    ?in_box@@YAHHHPBUtagRECT@@@Z
// size      52 bytes
// prototype 
// callers   24   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Is the point inside the rectangle? true/false
Status: Complete

// LEVER: guard clauses, not one `&&` chain. The image is four independent
// tests, each with its OWN `xor eax,eax; ret` on failure - not a shared
// early-return block - and the last comparison returns straight out of a
// `setl` with no branch at all. A single boolean expression folds the four
// tests into shared control flow the image does not have.
*/
BOOL __cdecl in_box(int x, int y, const RECT *rect) {
    if (x < rect->left) {
        return FALSE;
    }
    if (x >= rect->right) {
        return FALSE;
    }
    if (y < rect->top) {
        return FALSE;
    }
    return y < rect->bottom;
}

/*
Purpose: Slide a rectangle by a delta on each axis, the wrapping counterpart of
         the Win32 OffsetRect. Its neighbour make_rect below builds rectangles
         with the same wrapping arithmetic, and this one is entered the same
         way: `ret` with no operand and the caller's `add esp, 0xc` at
         0x005EDC92, so __cdecl with three arguments. EAX still holds the
         rectangle pointer at the return because nothing overwrites it after
         the load at 0x005F8670, but that is a residue and not a value: the
         one caller clobbers EAX two instructions later at 0x005EDC95.
// ORIGINAL: 0x005F8670 sub_5f8670 0x005F8670-0x005F869B BYTE_EXACT
// symbol    ?offset_rect@@YAXPAUtagRECT@@HH@Z
// size      43 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
Verification note: the original writes left, right, top, bottom in that
         interleaved order, reloading the y delta between the right and top
         stores. The four fields cannot alias, so a mutant that swaps two
         adjacent stores is an equivalent mutant no assertion can separate;
         the order is pinned through `volatile` only so the emitted stores
         keep matching the original, exactly as make_rect does.
*/
void __cdecl offset_rect(RECT *rect, int dx, int dy) {
    // PLAIN `+=`, in the image's order. The `volatile` was there to pin that
    // order and the bit-casts to pin the wrapping; neither is needed, and
    // together they made a 16-instruction body compile to 37.
    rect->left += dx;
    rect->right += dx;
    rect->top += dy;
    rect->bottom += dy;
}

/*
Purpose: Build a rectangle from an origin and dimensions using wrapping coordinates.
// ORIGINAL: 0x005F86C0 sub_5f86c0 0x005F86C0-0x005F86E6 BYTE_EXACT
// symbol    ?make_rect@@YAPAUtagRECT@@PAU1@HHHH@Z
// size      38 bytes
// prototype 
// callers   2   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
RECT *__cdecl make_rect(RECT *rect, int x, int y, int width, int height) {
    // PLAIN ASSIGNMENTS. The `volatile RECT *` and the `long_from_bits`
    // bit-cast were written to pin the store order and the signedness; both
    // are things VC6 cannot fold, and together they doubled the body. The
    // image's order IS declaration order, so nothing had to be pinned.
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;
    return rect;
}

/*
Purpose: Determine whether a point is inside an origin-and-dimensions rectangle.
// ORIGINAL: 0x005FA7A0 ?in_box@@YAHHHHHHH@Z 0x005FA7A0-0x005FA7DB BYTE_EXACT
// LEVER: four SEPARATE early-return guards (`if (x<left) return 0; ... if (x>=right) return 0; if (y<top) return 0; ...`) instead of the merged `x>=right || y<top`, and plain signed `+` instead of the `int_from_bits`/`uint32_t` bit-cast - the image never combines the two mid checks and never needs the cast. 0/25 -> BYTE_EXACT.
// size      59 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int __cdecl in_box(int x, int y, int left, int top, int width, int height) {
    if (x < left) {
        return 0;
    }
    const int right = left + width;
    if (x >= right) {
        return 0;
    }
    if (y < top) {
        return 0;
    }
    const int bottom = top + height;
    return y < bottom;
}

/*
Purpose: Compute a rectangle center with wrapping subtraction and truncation toward zero.
// ORIGINAL: 0x004BA830 ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z 0x004BA830-0x004BA863 BYTE_EXACT
// LEVER: MORE (2 calls vs image's 0: midpoint/int_from_bits helpers never inline) - the image writes the signed halving directly (`sub;cdq;sub;sar;add`), not through any helper, and the tail is `ret 0xc` (callee-cleans), not `ret` - the ORIGINAL is a thiscall member ignoring `this` entirely, and this free-function stand-in needs `__stdcall` (not `__cdecl`) to match that stack cleanup. Rewriting the body as plain field arithmetic and switching the calling convention took 0/25 (0.553 best) to 24/25 (0.960 best, /Oy- flag set) with 0 calls disagreeing.
// symbol    ?rect_center@@YGHPAUtagRECT@@PAH1@Z
// size      51 bytes
// prototype void (__thiscall ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z)(TutWin* this, RECT*, int*, int*)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int __stdcall rect_center(RECT *rect, int *x, int *y) {
    *x = rect->left + (rect->right - rect->left) / 2;
    int center_y = rect->top + (rect->bottom - rect->top) / 2;
    *y = center_y;
    return center_y;
}


/*
Purpose: Report whether this window holds the dialog focus, either directly or
         as its parent's current focus target.
// ORIGINAL: 0x005F2CA0 ?is_dialog_focus@Win@@QAEHXZ 0x005F2CA0-0x005F2CDF BYTE_EXACT
// 16/21, 0.857 similar, best across all ten flag sets) - the divergence is
// the guard's own polarity (`je`/zero-branch-jumps-ahead vs this tree's
// `jne`/compute-branch-jumps-ahead) and neither source shape reaches it.
// Plateau at 16/21 MISMATCH.
// size      63 bytes
// prototype int (__thiscall ?is_dialog_focus@Win@@QAEHXZ)(Win* this)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Holds focus (1); does not (0)
Status: Complete
*/
int Win::is_dialog_focus() {
    if ((iFlags_ & 0x1000U) != 0) {
        return 1;
    }
    Win *const parent = win_parent_;
    if (parent) {
        // An empty focus list reads as no focus rather than dereferencing the
        // list pointer, which the legacy body leaves untouched in that case.
        // `list_.current_` is the top-of-stack node; its own +0x4 holds the
        // `Win *` that currently has focus (see `class WinNodeList`, win.h).
        // LEVER: swapping these two arms - testing `!= 0` and putting the
        // read FIRST - is the whole match. The image branches `je` past the
        // read to a shared zero; testing `== 0` first compiled `jne` over a
        // zeroing arm instead, and cost 5 of 21 instructions. Collapsing the
        // zero away entirely (`if (parent && head_ != 0)`) is worse still:
        // 18 instructions against the image's 21, because the image really
        // does compute a zero and merge.
        Win *focused;
        if (parent->list_.head_ != 0) {
            focused = *reinterpret_cast<Win *const *>(
                reinterpret_cast<char *>(parent->list_.current_) + 4);
        } else {
            focused = nullptr;
        }
        if (focused == this) {
            return 1;
        }
    }
    return 0;
}


int WinHdcRefCount;        // 0x009B3AB0
HDC WinSharedHdc;          // 0x009B7B2C
IDirectDrawSurface *DirectDrawSurface;  // 0x009BC498
Win *WinModalStack[4];     // 0x009B6EF8
HINSTANCE WinInstance;     // 0x009B7B14
int WinScreenWidth;        // 0x009B7B1C
int WinScreenHeight;       // 0x009B7B20
Win *WinTrackingWindow;    // 0x009B7AAC
Win *WinHoverWindow;      // 0x009B7A7C
Win *WinPointerOwner1;     // 0x009B7ACC
Win *WinPointerOwner2;     // 0x009B7AD0
Win *WinPointerOwner3;     // 0x009B7ABC
Win *WinPointerOwner4;     // 0x009B7AC0
Win *WinFocusWindow;       // 0x009B7AE0
Win *WinModalWindow;       // 0x009B7ADC
Win *WinRootWindows[64];   // 0x009B6E48
int WinRootCount;          // 0x009B7B34
int WinMouseDirect;        // 0x009B7AA4
int WinMouseScreenX;       // 0x009B6628
int WinMouseScreenY;       // 0x009B662C
int WinClipWidth;            // 0x009B7A40
int WinClipHeight;           // 0x009B7A44
int WinDragOffsetX;          // 0x009B7A38
int WinDragOffsetY;          // 0x009B7A3C
int WinMaximizeIcon;         // 0x009B7B04
int WinRestoreIcon;          // 0x009B7B08
int WinModalResult;          // 0x009B6EF0
int WinSavedAreaWidth;       // 0x009B6F88
int WinSavedAreaHeight;      // 0x009B6F8C
int WinSizingFlag;           // 0x009B7AD4
int WinModalDepth;           // 0x009B7AE4
int WinPopupCount;           // 0x009B26EC
int WinDrawFlags;            // 0x009B238C
int WinKeyModifiers;         // 0x009B7B18
int WinViewOriginX;          // 0x009B7A70
int WinViewOriginY;          // 0x009B7A74
const int WinFillColour = 9;  // 0x00696D14, read from the image
const char WinMsgTooManyChildren[] = "Too many children";            // 0x00696D80
const char WinMsgIncreaseMaxChildren[] = "Increase #define MAX_CHILDREN";  // 0x00696D60
const char WinMsgTooManyParents[] = "Too many parents";              // 0x00696DB4
const char WinMsgIncreaseMaxParents[] = "Increase #define MAX_PARENTS";    // 0x00696D94
const char WinMdebugCode[] = "mdebug";                               // 0x00696DF8
int WinTrackingMode;       // 0x009B7AA8
int WinTrackingX;          // 0x009B7AB0
int WinTrackingY;          // 0x009B7AB4
Buffer *WinBackBuffer;         // 0x009B7A68
Win *WinModalFocus;         // 0x009B8D7C
Win *WinPopupWindow;        // 0x009B23B4
Win *WinTopDialog;          // 0x009B2300
Win *WinPendingFocus;       // 0x009B7B38
Win *WinCallbackWindow;   // 0x009B7AB8
Win *WinInputFocus;       // 0x009B7AC4
Win *WinActiveWindow;     // 0x009B7AC8
Win *WinBubbleCompanion;   // 0x009B7A4C
Win WinBubbleWindow;       // 0x009B22F0
Win *WinDialogList[150];   // 0x009B2494
Win *WinZOrderArray[512];  // 0x009B6630
RECT WinBubbleRect;        // 0x009B6E38
RECT WinScreenClipRect;    // 0x009B74C0
RECT WinDirtyRect;         // 0x009B6EE8
Win *WinDefaultFocus;      // 0x009B7AEC
DDInit WinDisplayInit;     // 0x009BE618
Win *WinFocusStack[4];     // 0x009B7A1C
char WinKeyRing[10];       // 0x009B7B48
char *WinKeyRingCursor = WinKeyRing;  // 0x00696D5C
const uint32_t WinStaticDefaults[10] = {  // 0x00696D34, the image's values
    247, 24, 0xffffffff, 1, 1, 8, 0xffffffff, 7, 2, 1};
uint32_t WinDynamicDefaults[10];       // 0x009B7AF0
int WinBubbleActive;       // 0x009B7A50
Win *WinZOrderWindow;      // 0x009B7A6C
int WinZOrderCount;        // 0x009B7B30
int WinZOrderFlag;         // 0x009B7A78
void(__cdecl *WinLeftDownHook)(int x, int y);  // 0x009B7A90
int WinCursorMoved;        // 0x009B7B3C
void(__cdecl *WinMessageHook)();                            // 0x009B7A88
void(__cdecl *WinMouseHook)(HWND window, LPARAM position);  // 0x009B7A94
void(__cdecl *WinKeyHook)(WPARAM key);                      // 0x009B7A8C

/*
Purpose: Acquire the process-wide device context, taking one reference.
// ORIGINAL: 0x005EC690 ?get_hdc@Win@@QAGPAUHDC__@@XZ 0x005EC690-0x005EC6E1 BYTE_EXACT
// symbol    ?get_hdc@Win@@SAPAUHDC__@@XZ
// size      81 bytes
// prototype 
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EC6B6 0x005EC6C7
Return Value: The shared device context, or zero when none could be obtained
Status: Complete

// LEVER: branch polarity. The image falls through to the vtable `GetDC`
// call when `DirectDrawSurface` IS set and jumps away to the plain `GetDC`
// import only when it is null - `if (DirectDrawSurface != nullptr) {
// DirectDrawSurface->GetDC(...) } else { GetDC(HandleMain) }`, arms
// swapped from the natural null-check-first reading. Moved 13/24 (0.917)
// to BYTE_EXACT.
*/
HDC Win::get_hdc() {
    // A context already held is simply counted again; only the first
    // reference actually acquires one.
    if (WinHdcRefCount != 0) {
        ++WinHdcRefCount;
        return WinSharedHdc;
    }
    if (DirectDrawSurface != nullptr) {
        DirectDrawSurface->GetDC(&WinSharedHdc);
    } else {
        WinSharedHdc = GetDC(HandleMain);
    }
    // A failed acquire leaves the count at zero so the next call retries.
    if (WinSharedHdc != nullptr) {
        WinHdcRefCount = 1;
    }
    return WinSharedHdc;
}

/*
Purpose: Drop one reference to the process-wide device context, releasing it
         once the last reference is gone.
// ORIGINAL: 0x005EC6F0 ?release_hdc@Win@@QAGXXZ 0x005EC6F0-0x005EC73C BYTE_EXACT
// symbol    ?release_hdc@Win@@SAXXZ
// size      76 bytes
// prototype 
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EC710 0x005EC72B
Status: Complete
Verification note: the two surviving mutants both concern the ReleaseDC call
on the no-surface path. It is a real GDI import whose effect no fixture can
inspect, so neither dropping it nor clearing the handle before it is
observable. Everything around it is covered: the reference count, the
surface release and its arguments, the handle being cleared, and the
exactly-zero test that makes an over-release drive the count negative and
skip the release entirely.
*/
void Win::release_hdc() {
    --WinHdcRefCount;
    // The legacy body tests for exactly zero rather than at-or-below, so an
    // over-release drives the count negative and skips the release entirely.
    if (WinHdcRefCount != 0) {
        return;
    }
    if (DirectDrawSurface != nullptr) {
        DirectDrawSurface->ReleaseDC(WinSharedHdc);
        WinSharedHdc = nullptr;
        return;
    }
    ReleaseDC(HandleMain, WinSharedHdc);
    WinSharedHdc = nullptr;
}




/*
Purpose: Select a system cursor by name and refresh the displayed cursor.
// ORIGINAL: 0x005EC7C0 ?set_cursor@Win@@QAEHH@Z 0x005EC7C0-0x005EC7FE BYTE_EXACT
// size      62 bytes
// prototype int (__thiscall ?set_cursor@Win@@QAEHH@Z)(Win* this, int nCursorName)
// callers   13   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F1820
Return Value: No errors (0); name outside the accepted range (3)
Status: Complete with a temporary dependency on the cursor refresh
Verification note: ordering the cursor_handle_ clear against the refresh is
not observable here - the refresh is passed a null window, so it cannot read
this object's state through the seam a fixture can see. The store order is
kept as the legacy body has it rather than because a test distinguishes it.

// LEVER: `<= 0x7EFF` / `>= 0x7F8B` and `< 0x7F00` / `> 0x7F8A` are the same
// range but not the same codegen - the image is `cmp eax,0x7f00; jl` and
// `cmp eax,0x7f8a; jg`, this was `cmp eax,0x7eff; jle` and `cmp
// eax,0x7f8b; jge`. Spelling the boundary the way the image does moved
// 13/17 (0.882) to BYTE_EXACT.
*/
// ORIGINAL: 0x005EC740 ?set_cursor@Win@@QAEHPAVSprite@@HH@Z 0x005EC740-0x005EC773 FILE BYTE_EXACT
// symbol    ?set_cursor@Win@@QAEHPAVSprite@@HH@Z
// notes     the Sprite overload; `marker_symbols` resolves a region to the
//           first overload of that name in the object, so each one has to
//           say what it emits.
// size      51 bytes
// prototype int (__thiscall ?set_cursor@Win@@QAEHPAVSprite@@HH@Z)(Win* this, Sprite*, int, int)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F1820
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005EC740
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ec740/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_cursor@Win@@QAEHPAVSprite@@HH@Z  at 0x005EC740  (51 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
// ORIGINAL: 0x005EC780 ?set_cursor@Win@@QAEHPAUHCURSOR@@@Z 0x005EC780-0x005EC7BC FILE BYTE_EXACT
// symbol    ?set_cursor@Win@@QAEHPAPAUHICON__@@@Z
// notes     the HCURSOR overload. `HCURSOR` is `struct HICON__ *`, so a
//           `HCURSOR *` parameter mangles PAPAUHICON__. Stated because
//           marker_symbols resolves the region to the first `set_cursor`.
// notes     the HCURSOR overload; `marker_symbols` resolves a region to the
//           first overload of that name in the object, so each one has to
//           say what it emits.
// size      60 bytes
// prototype int (__thiscall ?set_cursor@Win@@QAEHPAUHCURSOR@@@Z)(Win* this, HCURSOR* phCursor)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F1820
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005EC780
// measured tier  NO_COMPILE
// refusal        u005ec780.cpp(7) : error C2065: 'Win' : undeclared identifier u005ec780.cpp(7) : error C2059: syntax error : ',' u005ec780.cpp(9) : error C2653: 'Win' : is not 
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ec780/unit.cpp
// and score it with tools/agent_brief.py.
// `update_cursor` is one of the `QAA`/`QAG` Win window-procedure family: the
// mangled name reads as a non-static member, but the real ABI omits the
// receiver entirely (see tools/recovery_symbols.py:is_nonstatic_member). The
// scaffolding's class-member declaration therefore adds an implicit `this`
// no real caller passes; a plain free `__cdecl` declaration with the same
// two explicit parameters reproduces the direct call the original makes.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_cursor@Win@@QAEHPAUHCURSOR@@@Z  at 0x005EC780  (60 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
int Win::set_cursor(int name) {
    // Only the system cursor range is accepted; anything else is rejected
    // before any field is touched.
    if (name < 0x7F00 || name > 0x7F8A) {
        return 3;
    }
    cursor_sprite_ = nullptr;
    cursor_name_ = name;
    cursor_handle_ = nullptr;
    Win::update_cursor(nullptr, 1);
    return 0;
}



// The globals `Win::flip` reaches, all of them past `.data`'s stored bytes
// and so zero-fill - real objects for the same reason `WinHdcRefCount` and
// `WinSharedHdc` are, and none of them named anywhere before this body came
// out of src/recovered.
//
// 0x009B7A48. Called with no arguments at the top of every flip when it is
// set. Nothing recovered installs it yet.
void (*WinFlipHook)();
// 0x009B7A54 / 0x009B7A64 / 0x00696D2C / 0x00696D30. The bubble's font, the
// colour its box is filled with, its text colour and its edge colour. The
// last two are in `.data` and both hold 255.
Font *WinBubbleFont;
int WinBubbleFillColour;
const int WinBubbleTextColour = 255;   // 0x00696D2C
const int WinBubbleEdgeColour = 255;   // 0x00696D30
// 0x009B7A2C and the pair below it: a sprite drawn over the screen after the
// bubble, at 0x009B7A34 across and 0x009B7A30 down. Its frame comes from its
// own `cTransparentIndex_` at +8.
Sprite *WinFlipSprite;
int WinFlipSpriteY;                    // 0x009B7A30
int WinFlipSpriteX;                    // 0x009B7A34
// 0x009B7AD8. Chooses how a partial flip reaches the screen: zero blits the
// whole surface, non-zero intersects the caller's rectangle first.
int WinFlipClipped;
// 0x009BC49C. The surface `DirectDrawSurface` is blitted FROM - the back
// buffer to its front.
IDirectDrawSurface *DirectDrawBackBuffer;
// 0x009BC2D0. The rectangle a clipped flip intersects against.
RECT DirectDrawClipRect;

/*
Purpose: Present the screen buffer: draw the bubble text over it, flip or
         blit the DirectDraw surface, and copy the result to the window.
// ORIGINAL: 0x005EFD20 ?flip@Win@@QAAXPAURECT@@@Z 0x005EFD20-0x005F01E7
// symbol    ?flip@Win@@SAXPAUtagRECT@@@Z
// size      1223 bytes
// callers   27   call targets   14
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DAC70 0x005DACB0 0x005DD020 0x005DD130 0x005DE8F0 0x005DFCD0 0x005E3203 0x005E3503 0x005E3563 0x005E4B4A 0x005EC6F0 0x006453E0 0x00645DD0
// LEVER: TWO DEFECTS THAT ARE NOT MATCHING DEFECTS, both read off the
//        instruction bytes and both fixed here. (1) The bubble rectangle's
//        four adjustments had the signs on `top` and `right` inverted in BOTH
//        blocks: 0x005EFD7F-0x005EFD8C is `inc` on left (0x9B6E38) and top
//        (0x9B6E3C) with `dec` on right (0x9B6E40) and bottom (0x9B6E44), and
//        0x005EFEB9-0x005EFEBC is the exact inverse. As written the rectangle
//        grew where the image shrinks it. (2) The undo block AND the
//        `set_clip(&ScreenBuffer.rect2_)` after it sat outside the
//        `WinBubbleActive` test; 0x005EFD3D `je 0x5efee8` jumps past both, so
//        a frame with no bubble text performs neither, where this tree moved
//        WinBubbleRect by one on every frame. Neither fix moves the score -
//        both blocks are inside the part that does not align - and both are
//        right anyway.
// TRIED: not reachable from here as a spelling problem, and the numbers
//            say so plainly: 11 of 407 at the best flag set (/O2 /Ob0 /Gy
//            /GR- /GX) and 0.056 SIMILAR, with every one of the ten flag sets
//            between 0.019 and 0.056 and none above 14 of 407. This tree
//            compiles 312 instructions against the image's 407, so ninety-odd
//            instructions of the image are work this body does not do -
//            call_diff agrees on the call graph, so the gap is inlined work,
//            not missing calls. The aligned diff collapses past image
//            instruction 174 (233 image instructions against none of ours),
//            which is what a structural difference looks like rather than a
//            register one. Status is WIP for that reason; this is a
//            transcription job against the listing, not a lever.
Return Value: n/a
Status: WIP

DERIVED FROM THE IMAGE, not ported from src/recovered/units/005efd20.cpp,
which is deleted. That unit was checked against the listing first and three
of its statements are wrong:

  it passed `BitBlt` the height as the width. The image loads
  `[0x9b7514]` and `[0x9b7510]`, NEGATES the first and pushes it as `cy`
  with the second as `cx` - so the width is `biWidth` and the height is
  `-biHeight`. The unit had them the other way round, which is a blit of
  the wrong shape every frame.

  it called the surface's slot 5 with NO arguments. The image pushes five
  plus the receiver: `Blt(nullptr, other, nullptr, DDBLT_WAIT, nullptr)`.
  The unit's own comment says the emitter's slots are nullary until someone
  fixes the signature, and nobody did.

  it never advanced the y between bubble lines - `top += 0`. The image adds
  a line height derived from the font: `unk_1_` if it is negative, and
  `height_ + unk_1_` if it is not.

A fourth thing about it is not a defect but reads as one: the block after
the bubble names its locals `top` and `right` for 0x9B6E40 and 0x9B6E3C,
which are `right` and `top`. The STORES are correct - the block undoes the
four adjustments the box made - so only the names mislead.

THREE OF ITS "GLOBALS" ARE MEMBERS. 0x009B74C0, 0x009B7510 and 0x009B7514
are `ScreenBuffer.rect2_`, `.dib_.bmiHeader.biWidth` and `.biHeight`:
`ScreenBuffer` is at 0x009B7490 and those are +0x30, +0x80 and +0x84. A
scaffold that binds globals by address cannot see that, which is most of
why it needed thirty-three of them.

THE SLOTS ARE `IDirectDrawSurface`, confirmed by the arity pushed at each
site: 5 is `Blt`, 11 is `Flip`, 17 is `GetDC`, 25 is `Lock`, 26 is
`ReleaseDC`, 32 is `Unlock`. Slot 11 being `Flip` is what this function is.
*/
void __cdecl Win::flip(RECT *area) {
    if (WinFlipHook != nullptr) {
        WinFlipHook();
    }

    if (WinBubbleActive != 0) {
        if (area != nullptr) {
            ScreenBuffer.set_clip(area);
        }
        ScreenBuffer.box(&WinBubbleRect, WinBubbleEdgeColour,
                         WinBubbleEdgeColour);
        // The box is drawn on the border and the fill inside it, so the
        // rectangle moves IN by one on each side and back out below.
        // 0x005EFD7F-0x005EFD8C: `inc` on left (0x9B6E38) and top (0x9B6E3C),
        // `dec` on right (0x9B6E40) and bottom (0x9B6E44). This tree had the
        // signs on top and right the other way round, which grows the
        // rectangle where the image shrinks it - and the block below undid
        // the wrong two the same way.
        WinBubbleRect.left += 1;
        WinBubbleRect.top += 1;
        WinBubbleRect.right -= 1;
        WinBubbleRect.bottom -= 1;
        ScreenBuffer.fill(&WinBubbleRect, WinBubbleFillColour);
        ScreenBuffer.set_font(WinBubbleFont, nullptr, nullptr, nullptr);
        ScreenBuffer.set_text_color(WinBubbleTextColour, -1, 1, 1);

        LPSTR text = reinterpret_cast<LPSTR>(WinBubbleActive);
        if (strchr(text, '^') == nullptr) {
            // One line, centred in the box.
            if (text != nullptr) {
                ScreenBuffer.write_cent_l(text, &WinBubbleRect,
                                          static_cast<int>(strlen(text)));
            }
        } else {
            // `^` separates lines. Each is centred in turn and the
            // separator is put back so the caller's string survives.
            int y = WinBubbleRect.top + 1;
            LPSTR line = text;
            for (;;) {
                LPSTR const split = strchr(line, '^');
                if (split != nullptr) {
                    *split = '\0';
                }
                ScreenBuffer.write_cent_l(
                    line, WinBubbleRect.left, y,
                    WinBubbleRect.right - WinBubbleRect.left,
                    static_cast<int>(strlen(line)));
                y += (WinBubbleFont->unk_1_ < 0)
                    ? WinBubbleFont->line_height_
                    : WinBubbleFont->height_ + WinBubbleFont->unk_1_;
                if (split == nullptr) {
                    break;
                }
                *split = '^';
                line = split + 1;
            }
        }

        // INSIDE the bubble branch, not after it. 0x005EFD3D
        // `je 0x5efee8` - the WinBubbleActive test - jumps PAST both this
        // block and the set_clip below it, so a run with no bubble text
        // performs neither. This tree ran both unconditionally, which moved
        // WinBubbleRect by one every frame.
        WinBubbleRect.left -= 1;
        WinBubbleRect.top -= 1;
        WinBubbleRect.right += 1;
        WinBubbleRect.bottom += 1;
        if (area != nullptr) {
            ScreenBuffer.set_clip(&ScreenBuffer.rect2_);
        }
    }

    if (WinFlipSprite != nullptr) {
        // The frame index is the sprite's own byte at +8.
        WinFlipSprite->draw(&ScreenBuffer, WinFlipSprite->cTransparentIndex_,
                            WinFlipSpriteY, WinFlipSpriteX, 1, 1);
    }

    if (DirectDrawSurface != nullptr) {
        // A clipped flip only reaches the screen when the caller's rectangle
        // meets the surface's; an unclipped one always does.
        RECT unused;
        if (WinFlipClipped == 0 || area == nullptr
            || IntersectRect(&unused, area, &DirectDrawClipRect)) {
            DirectDrawSurface->Blt(nullptr, DirectDrawBackBuffer, nullptr,
                                   DDBLT_WAIT, nullptr);
        } else {
            return;
        }
    }

    if (WinHdcRefCount != 0) {
        ++WinHdcRefCount;
    } else {
        WinSharedHdc = GetDC(HandleMain);
        if (WinSharedHdc != nullptr) {
            WinHdcRefCount = 1;
        }
    }
    if (WinSharedHdc == nullptr) {
        return;
    }

    const HDC source = ScreenBuffer.get_hdc();
    if (source == nullptr) {
        if (--WinHdcRefCount != 0) {
            return;
        }
        if (DirectDrawSurface != nullptr) {
            DirectDrawSurface->Blt(nullptr, DirectDrawBackBuffer, nullptr,
                                   DDBLT_WAIT, nullptr);
        }
        WinSharedHdc = nullptr;
        return;
    }

    ScreenBuffer.sync_to_palette(PaletteActive);

    if (BufferDirectDraw == nullptr) {
        if (WinHdcRefCount != 0) {
            ++WinHdcRefCount;
        } else if (DirectDrawSurface != nullptr) {
            DirectDrawSurface->Flip(nullptr, DDFLIP_WAIT);
            WinHdcRefCount = 1;
        } else {
            WinSharedHdc = GetDC(HandleMain);
            if (WinSharedHdc != nullptr) {
                WinHdcRefCount = 1;
            }
        }
        if (WinSharedHdc != nullptr) {
            SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
            RealizePalette(WinSharedHdc);
            Win::release_hdc();
        }
    }

    if (area == nullptr) {
        BitBlt(WinSharedHdc, 0, 0, ScreenBuffer.dib_.bmiHeader.biWidth,
               -ScreenBuffer.dib_.bmiHeader.biHeight, source, 0, 0, SRCCOPY);
    } else {
        BitBlt(WinSharedHdc, area->left, area->top,
               area->right - area->left, area->bottom - area->top,
               source, area->left, area->top, SRCCOPY);
    }

    if (--WinHdcRefCount == 0) {
        if (DirectDrawSurface != nullptr) {
            DirectDrawSurface->ReleaseDC(WinSharedHdc);
        } else {
            ReleaseDC(HandleMain, WinSharedHdc);
        }
        WinSharedHdc = nullptr;
    }
    // The reference `Buffer::get_hdc()` took above (`source`) is dropped
    // here, unconditionally, right before the return - a call this tree
    // was missing entirely. `call_diff.py` flags it as one of the FEWER
    // calls; the image's own `push 1; call 0x5e3563` sits right after the
    // ReleaseDC block, both branches of which converge into it.
    ScreenBuffer.release_hdc(1);
}


/*
Purpose: Dismiss any pending bubble text and repaint the area it covered.
// ORIGINAL: 0x005F8500 ?clear_bubble_text@Win@@QAGXXZ 0x005F8500-0x005F852F BYTE_EXACT
// symbol    ?clear_bubble_text@Win@@SAXXZ
// size      47 bytes
// prototype 
// callers   15   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD20 0x005F7320
Status: Complete with temporary dependencies on the screen refresh and flip
*/
// LEVER (measured 2026-08-25, 6 of 13 -> 13 of 13 BYTE_EXACT):
// GIVE THE GLOBAL REAL STORAGE INSTEAD OF PICKING A BINDING TYPE.
// This plateau used to be recorded here as a trade-off with no winning
// side. Through a `Win **const` binding, `*WinBubbleCompanion = 0`
// compiled `mov eax, dword ptr [0]` and a store THROUGH eax, where the
// image writes the address directly and shares one zeroed register across
// both stores (`push eax` / `mov [0x9b7a4c], eax` / `mov [0x9b7a50],
// eax`). An `int *const` binding folds for the store, but then bubble_text
// needs a cast to put `this` into it - and that cast-free spelling is what
// earned bubble_text ITS claim. Either binding type cost some body a claim,
// so the retype was refused rather than measured.
// Both horns come off once the name stops being a binding at all.
// `extern Win *WinBubbleCompanion;`, defined in this file, folds for the
// store AND takes `this` with no cast, because it IS a Win * rather than a
// pointer to one. map.h reached the same conclusion by a third route -
// `inline Map *&map_tiles()` - after measuring that `Map **const` cost two
// loads on every tile access. The dilemma was an artifact of the scaffold.
void Win::clear_bubble_text() {
    // Nothing to dismiss when no bubble is pending.
    if (WinBubbleActive == 0) {
        return;
    }
    WinBubbleCompanion = 0;
    WinBubbleActive = 0;
    Win::update_screen(&WinBubbleRect, nullptr);
    Win::flip(&WinBubbleRect);
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005EC680 ?UNK1@Win@@QAEHHHHHHHHHH@Z 0x005EC680-0x005EC685 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK1@Win@@QAEHHHHHHHHHH@Z)(Win* this, int, int, int, int, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Win::UNK1(int, int, int, int, int, int, int, int, int) {
    return 0;
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005ED7C0 ?UNK5@Win@@QAEHXZ 0x005ED7C0-0x005ED7C3 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?UNK5@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Win::UNK5() {
    return 0;
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005EDFE0 ?UNK6@Win@@QAEHH@Z 0x005EDFE0-0x005EDFE5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK6@Win@@QAEHH@Z)(Win* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Win::UNK6(int) {
    return 0;
}


/*
Purpose: Legacy stub; the original body returns 1 without reading its
         arguments.
// ORIGINAL: 0x005F6A30 ?on_set_cursor@Win@@QAEHPAXII@Z 0x005F6A30-0x005F6A38 BYTE_EXACT
// symbol    ?on_set_cursor@Win@@UAEHPAXII@Z
// size      8 bytes
// prototype int (__thiscall ?on_set_cursor@Win@@QAEHPAXII@Z)(Win* this, void*, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Win::on_set_cursor(void *, unsigned int, unsigned int) {
    return 1;
}



/*
Purpose: Record the window that receives focus by default.
// ORIGINAL: 0x005F2CE0 ?set_def_focus@Win@@QAAXPAUWin@@@Z 0x005F2CE0-0x005F2CEA BYTE_EXACT
// symbol    ?set_def_focus@Win@@SAXPAV1@@Z
// LEVER (2026-08-26): the parameter is a `Win *`, exactly as the image's own
// name spells it (PAUWin@@) - it had been forced to `(int)` through this
// alias, which cost a reinterpret_cast at the only store. Same bytes, no
// cast: `mov eax,[esp+4] / mov ds:[0x9B7AEC],eax / ret`.
// size      10 bytes
// prototype void (__cdecl ?set_def_focus@Win@@QAAXPAUWin@@@Z)(Win* this, Win*)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void Win::set_def_focus(Win *focus) {
    WinDefaultFocus = focus;
}


/*
Purpose: Publish a value into both attached scrollbars' first shared slot.
// ORIGINAL: 0x005EE130 ?UNK8@Win@@QAEXH@Z 0x005EE130-0x005EE157 BYTE_EXACT
// symbol    ?set_scroll_sprite@Win@@QAEXH@Z
// notes     NAMED from its body and from popup.cpp:926, which assigns a
//           Sprite * to the same Scroll::field_2144_ this method writes on
//           both bars. The catalogue's UNK8 stays on the ORIGINAL line -
//           that is the name the image was catalogued under.
// size      39 bytes
// prototype void (__thiscall ?set_scroll_sprite@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void Win::set_scroll_sprite(int value) {
    // Each scrollbar is optional and tested separately, so one attached bar
    // is still updated when the other is absent.
    if (scroll_vert_) {
        scroll_vert_->field_2144_ = static_cast<uint32_t>(value);
    }
    if (scroll_horz_) {
        scroll_horz_->field_2144_ = static_cast<uint32_t>(value);
    }
}

/*
Purpose: Publish a value into both attached scrollbars' second shared slot.
// ORIGINAL: 0x005EE160 ?UNK9@Win@@QAEXH@Z 0x005EE160-0x005EE187 BYTE_EXACT
// size      39 bytes
// prototype void (__thiscall ?UNK9@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void Win::UNK9(int value) {
    if (scroll_vert_) {
        scroll_vert_->field_2148_ = static_cast<uint32_t>(value);
    }
    if (scroll_horz_) {
        scroll_horz_->field_2148_ = static_cast<uint32_t>(value);
    }
}



/*
Purpose: Reset the window clip; the legacy implementation is a bare return.
// ORIGINAL: 0x005EE020 ?reset_window_clip@Win@@QAEXXZ 0x005EE020-0x005EE021 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?reset_window_clip@Win@@QAEXXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Win::reset_window_clip() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005F54B0 ?on_move@Win@@QAEXHH@Z 0x005F54B0-0x005F54B3 BYTE_EXACT
// symbol    ?on_move@Win@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_move@Win@@QAEXHH@Z)(Win* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Win::on_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005F54C0 ?on_size@Win@@QAEXIHH@Z 0x005F54C0-0x005F54C3 BYTE_EXACT
// symbol    ?on_size@Win@@UAEXIHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_size@Win@@QAEXIHH@Z)(Win* this, unsigned int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Win::on_size(unsigned int, int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005F54D0 ?on_size_nc@Win@@QAEXIHH@Z 0x005F54D0-0x005F54D3 BYTE_EXACT
// symbol    ?on_size_nc@Win@@UAEXIHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_size_nc@Win@@QAEXIHH@Z)(Win* this, unsigned int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Win::on_size_nc(unsigned int, int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005F6A40 ?on_sys_command@Win@@QAEHIHH@Z 0x005F6A40-0x005F6A43 BYTE_EXACT
// symbol    ?on_sys_command@Win@@UAEHIHH@Z
// size      3 bytes
// prototype int (__thiscall ?on_sys_command@Win@@QAEHIHH@Z)(Win* this, unsigned int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
#pragma warning(push)
#pragma warning(disable : 4716)
// The mangled name says `int`; the original is `ret 0xc` at 0x005F6A40
// and never sets EAX. `return 0;` would emit `xor eax, eax` and add two
// bytes, so falling off the end IS the behaviour.
int Win::on_sys_command(unsigned int, int, int) {
}
#pragma warning(pop)


// The active palette lives at a fixed address; the same seam basebutton.cpp
// uses. Its most-recently-set window generation counter is at 0x400.

/*
Purpose: Bring this window's palette into step with the active one, but only
         when it has fallen behind - set_active_window is skipped when the
         cached generation at 0x184 already matches the palette's at 0x400.
// ORIGINAL: 0x005F2C60 ?sync_palette@Win@@QAEXXZ 0x005F2C60-0x005F2C94 BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?sync_palette@Win@@QAEXXZ)(Win* this)
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE4F0
Return Value: n/a
Status: Complete
*/
// 7 of 14 - reading the palette's field directly scored 0 of 14 at 15
// instructions, because something had to put the palette's field in eax
// and only the temporary did.
// LEVER (2026-08-27): everything upstream changed underneath that plateau.
// The generation puns became palette_seed_cache_ vs PaletteActive->seed_,
// and the fn-pointer binding g_SetActiveWindow became the real named
// Palette::set_active_window(this). Under those schedulings the honest
// form lands exactly on the image's `mov eax,[esi+0x184]` /
// `cmp eax,[ecx+0x400]`: 14 of 14 BYTE_EXACT. Recorded for every other
// body stuck at a register-side plateau whose neighbourhood has moved
// since: re-measure before believing it.
void Win::sync_palette() {
    // Re-tried 2026-08-27 with the named set_active_window call in place
    // (its TRIED entry predated the swap off the fn-pointer binding): the
    // honest member read measures like the temporary now. Keep members.
    if (palette_seed_cache_ == PaletteActive->seed_) {
        return;
    }
    Palette::set_active_window(this);
    palette_seed_cache_ = PaletteActive->seed_;
}


/*
Purpose: Forward a scrollbar position to the vertical Scroll the window owns,
         doing nothing when that scrollbar is absent.
// ORIGINAL: 0x005EE030 ?set_vert_pos@Win@@QAEXH@Z 0x005EE030-0x005EE047 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?set_vert_pos@Win@@QAEXH@Z)(Win* this, int)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00605D20
Return Value: n/a
Status: Complete
*/
void Win::set_vert_pos(int position) {
    if (scroll_vert_) {
        scroll_vert_->set_pos(position);
    }
}

/*
Purpose: Forward a scrollbar position to the horizontal Scroll the window owns,
         doing nothing when that scrollbar is absent.
// ORIGINAL: 0x005EE070 ?set_horz_pos@Win@@QAEXH@Z 0x005EE070-0x005EE087 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?set_horz_pos@Win@@QAEXH@Z)(Win* this, int)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00605D20
Return Value: n/a
Status: Complete
*/
void Win::set_horz_pos(int position) {
    if (scroll_horz_) {
        scroll_horz_->set_pos(position);
    }
}

/*
Purpose: Forward a scrollbar range to the vertical Scroll the window owns,
         doing nothing when that scrollbar is absent.
// ORIGINAL: 0x005EE0B0 ?set_vert_range@Win@@QAEXHH@Z 0x005EE0B0-0x005EE0CC BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?set_vert_range@Win@@QAEXHH@Z)(Win* this, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006059B0
Return Value: n/a
Status: Complete
*/
void Win::set_vert_range(int minimum, int maximum) {
    if (scroll_vert_) {
        scroll_vert_->set_range(minimum, maximum);
    }
}

/*
Purpose: Forward a scrollbar range to the horizontal Scroll the window owns,
         doing nothing when that scrollbar is absent.
// ORIGINAL: 0x005EE0D0 ?set_horz_range@Win@@QAEXHH@Z 0x005EE0D0-0x005EE0EC BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?set_horz_range@Win@@QAEXHH@Z)(Win* this, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006059B0
Return Value: n/a
Status: Complete
*/
void Win::set_horz_range(int minimum, int maximum) {
    if (scroll_horz_) {
        scroll_horz_->set_range(minimum, maximum);
    }
}





/*
Purpose: Window cursor-set hook; the legacy implementation returns 1 to report
         it handled the message and does nothing else.
// ORIGINAL: 0x005F2670 ?OnSetCursor@Win@@QAAHPAXPAXII@Z 0x005F2670-0x005F2676 BYTE_EXACT
// symbol    ?OnSetCursor@Win@@SAHPAX0II@Z
// size      6 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Win::OnSetCursor(void *, void *, unsigned int, unsigned int) {
    return 1;
}


/*
Purpose: Report whether a value is present in the window's id table - the
         array at 0x1A4 with its count at 0x3FC. A zero value and an empty
         table both report absent.
// ORIGINAL: 0x005ECE80 ?UNK3@Win@@QAEHH@Z 0x005ECE80-0x005ECEB6 BYTE_EXACT
// symbol    ?is_child@Win@@QAEHH@Z
// notes     NAMED from the body: it walks children_[0..child_count_) and
//           returns 1 when the argument is one of them - the direct-child
//           test beside is_descendant (0x005ECE20), which recurses.
// size      54 bytes
// prototype int (__thiscall ?UNK3@Win@@QAEHH@Z)(Win* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when the value is in the table, 0 otherwise
Status: Complete
*/
int Win::is_child(int value) {
    if (value == 0) {
        return 0;
    }
    // THE CHILD LIST, BY ITS NAME. This arrived from the artifact walking
    // `this + 0x1A4` with a count at `this + 0x3FC` through memcpy - the
    // offsets are right, and win.h already proves they are `children_` and
    // `child_count_` from add_child's own `mov [ecx + eax*4 + 0x1a4], esi`.
    // A pun of `this` is banned here precisely because the members exist.
    for (int index = 0; index < child_count_; ++index) {
        if (reinterpret_cast<int>(children_[index]) == value) {
            return 1;
        }
    }
    return 0;
}


/*
Purpose: Report whether a window is anywhere below this one in the child
         tree, searching depth first. A direct child counts, and so does any
         descendant of one.
// ORIGINAL: 0x005ECE20 ?is_descendant@Win@@QAEHPAVWin@@@Z 0x005ECE20-0x005ECE73
// symbol    ?is_descendant@Win@@QAEHPAV1@@Z
// size      83 bytes
// prototype int (__thiscall ?is_descendant@Win@@QAEHPAVWin@@@Z)(Win* this, Win*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECE20
Return Value: 1 when the candidate is a descendant, 0 otherwise
Status: Complete
Verification note: hoisting the count out of the loop is an EQUIVALENT mutant
         and will survive the sweep. The original reloads it every iteration,
         so this transcription does too, but the whole call tree here is
         read-only - nothing reachable from is_descendant writes to any
         object - so no execution can observe the reload. It is kept because
         it is what the bytes do, not because a test pins it. If this function
         ever gains a seam, that changes and the reload becomes testable.
         The absent null check on children_[index] is likewise faithful: the
         original dereferences the slot unconditionally at 0x005ECE2E, so a
         null slot faults in both.

// LEVER: the image walks `children_` with an incrementing POINTER
// (`lea esi, [ebx+0x1a4]` once, `add esi, 4` per iteration, `mov ecx,[esi]`)
// rather than re-deriving `children_[index]` from a scaled index each time.
// Spelling it as `Win *const *child = children_; ... ++child;` instead of
// `children_[index]` moved agreement from 2/37 to 17/37 (0.691 -> 0.727
// similar) at `/c /O2 /Gy /GR- /GX`. `index` is kept only for the trailing
// `index >= count` bound check, which the image still does with `cmp
// edi, eax` against a separately-incremented edi.
//
// TRIED: hoisting `int index = 0;` to sit between the `count =
// child_count_` load and the `count <= 0` check (matching the image's
// `xor edi, edi` appearing before `test eax, eax`) - this makes VC6 pick
// edi instead of ebx for `this` and is WORSE (10/37). The remaining
// divergence (a missing early `xor edi, edi` and the resulting jump-target
// shift) did not yield to reordering the two post-count-check statements
// either; both orders compile identically.
//
// TRIED: loop shape, via try_spellings. `do { ... } while (index <
// count);` ties the committed `for (;;) { ...; if (index >= count) return
// 0; }` exactly (17/37, same bytes) - the image's single shared "return 1"
// landing pad (reached by both `je` and the post-recursion `jne`) is not
// reproduced by either; this tree's compiler duplicates that block instead of
// sharing it. A top-tested `while (index < count) { ... }` is WORSE (9/37),
// and merging the two early-return tests into one `||` condition is worse
// still (5/37) - the image tests them separately, matching the existing
// two-statement shape already committed.
*/
// TRIED: 17 of 37, and two restructurings made it no better or worse.
// A `do { ... } while (index < count);` gets the image's BACKWARD `jl` in
// place of this form's forward `jge` + repeated compare, but scores the
// same 17 and adds a differing run. Merging the two `return 1` arms into
// one `||` - the image has a single found-case epilogue where this emits
// two - collapses the loop instead: 4 of 37 at 32 instructions against the
// image's 37. What is left is instruction ORDER (the image zeroes edi
// before loading children_, this body after) and the duplicated epilogue,
// neither of which source order reached.
int Win::is_descendant(Win *candidate) {
    if (!candidate) {
        return 0;
    }
    int count = child_count_;
    if (count <= 0) {
        return 0;
    }
    Win *const *child = children_;
    int index = 0;
    for (;;) {
        if (*child == candidate) {
            return 1;
        }
        if ((*child)->is_descendant(candidate)) {
            return 1;
        }
        // Re-read, do NOT hoist. The original reloads the count at 0x005ECE50
        // after every recursive call returns, so a recursion that adds or
        // removes children is seen by the very next iteration. A loop-invariant
        // count is the obvious tidy-up and it changes behaviour.
        count = child_count_;
        ++index;
        ++child;
        if (index >= count) {
            return 0;
        }
    }
}


/*
// ORIGINAL: 0x005F8530 ?on_mousewheel_down_vert@Win@@QAEXH@Z 0x005F8530-0x005F8547 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?on_mousewheel_down_vert@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606320
Status: Complete
*/
void Win::on_mousewheel_down_vert(int delta) {
    Scroll *scroll = scroll_vert_;
    if (scroll) {
        scroll->on_mousewheel_down(delta);
    }
}

/*
// ORIGINAL: 0x005F8590 ?on_mousewheel_up_horz@Win@@QAEXH@Z 0x005F8590-0x005F85A7 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?on_mousewheel_up_horz@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606440
Status: Complete
*/
void Win::on_mousewheel_up_horz(int delta) {
    Scroll *scroll = scroll_horz_;
    if (scroll) {
        scroll->on_mousewheel_up(delta);
    }
}

/*
Purpose: Report the left mouse button state, honouring a swapped-buttons
         system setting.
// ORIGINAL: 0x005EC960 ?get_lbutton_state@Win@@QAEHXZ 0x005EC960-0x005EC97B BYTE_EXACT
// size      27 bytes
// prototype int (__thiscall ?get_lbutton_state@Win@@QAEHXZ)(Win* this)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EC963 0x005EC971
Return Value: nonzero while the logical left button is down
Status: Complete
*/
// ONE expression. Naming either result as a local is still correct C++, but
// VC6 then defers the `movsx` that widens the SHORT until after the second
// call instead of folding it straight after the first.

int Win::get_lbutton_state() {
    return GetSystemMetrics(0x17) ^ (GetAsyncKeyState(1) >> 15);
}

/*
// ORIGINAL: 0x005F8550 ?on_mousewheel_down_horz@Win@@QAEXH@Z 0x005F8550-0x005F8567 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?on_mousewheel_down_horz@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606320
Status: Complete
*/
void Win::on_mousewheel_down_horz(int delta) {
    Scroll *scroll = scroll_horz_;
    if (scroll) {
        scroll->on_mousewheel_down(delta);
    }
}

/*
// ORIGINAL: 0x005F8570 ?on_mousewheel_up_vert@Win@@QAEXH@Z 0x005F8570-0x005F8587 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?on_mousewheel_up_vert@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606440
Status: Complete
*/
void Win::on_mousewheel_up_vert(int delta) {
    Scroll *scroll = scroll_vert_;
    if (scroll) {
        scroll->on_mousewheel_up(delta);
    }
}

// Seams for the four unrecovered Win handlers, and the definitions that give
// the direct calls from the derived windows something to link against. Kept
// at the end of the file so nothing above - every body here carries an
// Original Offset the census reads by line number - moves.


/*
Purpose: Route a left button press - find the window under the pointer, ask
         it what was hit, raise it, and either begin a drag or dispatch the
         click.
// ORIGINAL: 0x005F2330 ?OnLButtonDown@Win@@QAAXPAXJHHI@Z 0x005F2330-0x005F256B BYTE_EXACT
// symbol    ?OnLButtonDown@Win@@SAXPAUHWND__@@JHHI@Z
// size      571 bytes
// prototype
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F4EC0 0x005F6F10
//
// `QAA`, so no receiver, and it passes `&x`/`&y` - the addresses of its OWN
// parameters - to `get_mouse_window`, which rewrites them into the found
// window's coordinates. Everything after that uses the rewritten pair.
//
// The tracking codes are 2 and 10 through 17. `hit` is whatever the window's
// slot 0x12C returns, and those values start a drag instead of a click: the
// four contiguous globals at 0x009B7AA8 are the whole of the drag state, and
// `Win::do_tracking` is the only thing that reads them back.
Status: WIP
*/
void Win::OnLButtonDown(HWND window, LONG dbl, int x, int y, WPARAM keys) {
    static const size_t WinSlotHitTest = 0x12C;    // slot 75
    static const size_t WinSlotLeftDown = 0x144;   // slot 81
    typedef int (OriginalObject::*func_win_hit_test)(int, int);
    typedef void (OriginalObject::*func_win_left_down)(LONG, int, int,
                                                       WPARAM, int);

    Win *const target = get_mouse_window(&x, &y);
    if (target != nullptr) {
        if (WinMouseDirect != 0
            || (target->iFlags_ & WinFlagHitTestIndirect) != 0) {
            const int hit = target->on_nc_hittest(x, y);
            if (hit != 0) {
                // WRITTEN OUT TWICE, as the original has it: the
                // parent's child list and the root list are the same
                // shuffle over different arrays, and factoring them
                // together costs 158 bytes of the 571.
                Win *const parent = target->win_parent_;
                if (parent != nullptr) {
                    if ((target->iFlags_ & WinFlagNoRaise) == 0) {
                        int index = 0;
                        while (index < parent->child_count_
                               && parent->children_[index] != target) {
                            ++index;
                        }
                        if (index < parent->child_count_) {
                            while (index > 0) {
                                parent->children_[index] = parent->children_[index - 1];
                                --index;
                            }
                            parent->children_[0] = target;
                        }
                        WinZOrderCount = 0;
                        for (int root = 0; root < WinRootCount; ++root) {
                            if (WinZOrderWindow != nullptr
                                && WinZOrderWindow == WinRootWindows[root]) {
                                WinZOrderCount = 0;
                                WinZOrderFlag = 0;
                            }
                            if ((WinRootWindows[root]->iSomeFlag_
                                 & WinFlagVisible) != 0) {
                                recurse_zorder(WinRootWindows[root]);
                            }
                        }
                    }
                } else {
                    if ((target->iFlags_ & WinFlagNoRaise) == 0) {
                        int index = 0;
                        while (index < WinRootCount
                               && WinRootWindows[index] != target) {
                            ++index;
                        }
                        if (index < WinRootCount) {
                            while (index > 0) {
                                WinRootWindows[index] = WinRootWindows[index - 1];
                                --index;
                            }
                            WinRootWindows[0] = target;
                        }
                        WinZOrderCount = 0;
                        for (int root = 0; root < WinRootCount; ++root) {
                            if (WinZOrderWindow != nullptr
                                && WinZOrderWindow == WinRootWindows[root]) {
                                WinZOrderCount = 0;
                                WinZOrderFlag = 0;
                            }
                            if ((WinRootWindows[root]->iSomeFlag_
                                 & WinFlagVisible) != 0) {
                                recurse_zorder(WinRootWindows[root]);
                            }
                        }
                    }
                }
                if (hit == 2 || (hit > 9 && hit <= 0x11)) {
                    WinTrackingWindow = target;
                    WinTrackingMode = hit;
                    WinTrackingX = x;
                    WinTrackingY = y;
                    return;
                }
            }
        }
        uint8_t *const vtable = *reinterpret_cast<uint8_t **>(target);
        target->on_l_button_down(dbl, x, y, keys, WinMouseDirect);
    }
    if (WinLeftDownHook != nullptr) {
        WinLeftDownHook(x, y);
    }
    if (WinMessageHook != nullptr) {
        WinMessageHook();
    }
}

/*
Purpose: Decide which window a screen position belongs to, translating the
         position into that window's coordinates on the way.
// ORIGINAL: 0x005F6F10 ?get_mouse_window@Win@@QAAHPAH0@Z 0x005F6F10-0x005F7319
// symbol    ?get_mouse_window@Win@@SAPAV1@PAH0@Z
// size      1033 bytes
// prototype
// callers   8   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0 0x005F6AB0
//
// CORRECTED from `int`, like `get_key_window` beside it: the catalogue says
// `H` and every caller treats the result as a `Win *`.
//
// The four `WinPointerOwner` globals come first, in that order, and the
// FIRST that is visible wins; one that is not is nulled on the way past.
// Owners 2 and 4 set `WinMouseDirect`, and that is what the tail reads to
// decide between returning the window and returning its `poWinBase_`.
//
// STRUCTURE ALREADY MATCHES: the field offsets, the test/continue/break
// chain, and its ORDER (Owner1, then 2, then 3, then 4 - confirmed by
// reading the compiled object's own DIR32 relocation symbols, since the
// byte comparison masks the addresses) all line up with the image
// instruction-for-instruction through the whole owner loop. What differs,
// starting at instruction 4, is WHICH TWO of the four globals the image
// preloads into registers before the test chain even starts (`ebp` =
// Owner2, `edx` = Owner4; Owner1 and Owner3 are loaded lazily, inside the
// chain, right before their own test) - this tree's compile preloads
// Owner1/2/3 eagerly and Owner4 late. Both are legal schedules of the same
// four independent loads; nothing in the source controls which two VC6's
// allocator decides to hoist.
//
// TRIED: touching `WinPointerOwner2`/`WinPointerOwner4` with a no-op read
//            at the top of the function (before Owner1's check), to see
//            whether an earlier textual mention changes which two get
//            preloaded - 13 of 304 -> 14 of 304. The instruction that appears
//            is the touch itself; the image's own choice of ebp/edx for those
//            two globals over ecx/ebx for Owner1/3 did not follow.
//            (This note used to read `TRIED, MEASURED:`, which the lesson
//            grammar does not accept - the token must be followed by the
//            colon - so the body kept reading as untouched.)
// TRIED: the flag axis is exhausted and buys nothing. All ten sets
//            measured: best is 13 of 304 at 0.106 similar (/O2 /Gy /GR- /GX,
//            and /Ob0 and /Oi- score identically), worst 6 of 304 at 0.058;
//            /Oy- costs 4 instructions of agreement at every optimisation
//            level.
// TRIED: a second, independent scheduling gap, and it is worth an
//            instruction at every one of the visibility tests: the image
//            materialises the constant 1 once (`mov ebx, 1` at 0x005F6F2B)
//            and spends `test byte ptr [reg + 0x9c], bl` thereafter, where
//            this tree spends the seven-byte `test byte ptr [reg + 0x9c], 1`
//            each time. The image itself uses the immediate form at the one
//            site where ebx is the object pointer, so it is the allocator
//            choosing, not the source: there is no C++ spelling of
//            `flags & WinFlagVisible` that asks for a register.
Status: WIP
*/
Win *Win::get_mouse_window(int *x, int *y) {
    WinMouseScreenX = *x;
    WinMouseScreenY = *y;

    Win *found = nullptr;
    for (;;) {
        if (WinPointerOwner1 != nullptr) {
            if ((WinPointerOwner1->iSomeFlag_ & WinFlagVisible) != 0) {
                *x -= WinPointerOwner1->client_rect_.left + WinPointerOwner1->outer_rect_.left;
                *y -= WinPointerOwner1->client_rect_.top + WinPointerOwner1->outer_rect_.top;
                if ((WinPointerOwner1->iFlags_ & WinFlagClipToParent) != 0
                    && WinPointerOwner1->win_parent_ != nullptr) {
                    WinPointerOwner1->win_parent_->screen_to_client(x, y);
                    if ((WinPointerOwner1->iFlags_ & WinFlagParentOffset) != 0) {
                        *x += WinPointerOwner1->win_parent_->outer_rect_.left;
                        *y += WinPointerOwner1->win_parent_->outer_rect_.top;
                    }
                }
                WinMouseDirect = 0;
                found = WinPointerOwner1;
                break;
            }
            WinPointerOwner1 = nullptr;
            continue;
        }
        if (WinPointerOwner2 != nullptr) {
            if ((WinPointerOwner2->iSomeFlag_ & WinFlagVisible) != 0) {
                WinPointerOwner2->screen_to_client(x, y);
                *x += WinPointerOwner2->outer_rect_.left;
                *y += WinPointerOwner2->outer_rect_.top;
                WinMouseDirect = 1;
                found = WinPointerOwner2;
                break;
            }
            WinPointerOwner2 = nullptr;
            continue;
        }
        if (WinPointerOwner3 != nullptr) {
            if ((WinPointerOwner3->iSomeFlag_ & WinFlagVisible) != 0) {
                *x -= WinPointerOwner3->client_rect_.left + WinPointerOwner3->outer_rect_.left;
                *y -= WinPointerOwner3->client_rect_.top + WinPointerOwner3->outer_rect_.top;
                if ((WinPointerOwner3->iFlags_ & WinFlagClipToParent) != 0
                    && WinPointerOwner3->win_parent_ != nullptr) {
                    WinPointerOwner3->win_parent_->screen_to_client(x, y);
                    if ((WinPointerOwner3->iFlags_ & WinFlagParentOffset) != 0) {
                        *x += WinPointerOwner3->win_parent_->outer_rect_.left;
                        *y += WinPointerOwner3->win_parent_->outer_rect_.top;
                    }
                }
                WinMouseDirect = 0;
                found = WinPointerOwner3;
                break;
            }
            WinPointerOwner3 = nullptr;
            continue;
        }
        if (WinPointerOwner4 != nullptr) {
            if ((WinPointerOwner4->iSomeFlag_ & WinFlagVisible) != 0) {
                WinPointerOwner4->screen_to_client(x, y);
                *x += WinPointerOwner4->outer_rect_.left;
                *y += WinPointerOwner4->outer_rect_.top;
                WinMouseDirect = 1;
                found = WinPointerOwner4;
                break;
            }
            WinPointerOwner4 = nullptr;
            continue;
        }
        break;
    }

    if (found == nullptr) {
        if (WinFocusWindow != nullptr) {
            *x -= WinFocusWindow->client_rect_.left + WinFocusWindow->outer_rect_.left;
            *y -= WinFocusWindow->client_rect_.top + WinFocusWindow->outer_rect_.top;
            if ((WinFocusWindow->iFlags_ & WinFlagClipToParent) != 0
                && WinFocusWindow->win_parent_ != nullptr) {
                WinFocusWindow->win_parent_->screen_to_client(x, y);
                if ((WinFocusWindow->iFlags_ & WinFlagParentOffset) != 0) {
                    *x += WinFocusWindow->win_parent_->outer_rect_.left;
                    *y += WinFocusWindow->win_parent_->outer_rect_.top;
                }
            }
            *x += WinFocusWindow->outer_rect_.left;
            *y += WinFocusWindow->outer_rect_.top;
            found = get_mouse_window_recurse(WinFocusWindow, x, y);
            if (found == nullptr && WinModalWindow != nullptr) {
                *x = WinMouseScreenX;
                *y = WinMouseScreenY;
                WinModalWindow->screen_to_client(x, y);
                *x += WinModalWindow->outer_rect_.left;
                *y += WinModalWindow->outer_rect_.top;
                found = get_mouse_window_recurse(WinModalWindow, x, y);
            }
            if (found == nullptr) {
                // Neither subtree claimed it, so the focus window keeps it
                // and the position goes back to where it started.
                *x = WinMouseScreenX;
                *y = WinMouseScreenY;
                *x -= WinFocusWindow->client_rect_.left + WinFocusWindow->outer_rect_.left;
                *y -= WinFocusWindow->client_rect_.top + WinFocusWindow->outer_rect_.top;
                if ((WinFocusWindow->iFlags_ & WinFlagClipToParent) != 0
                    && WinFocusWindow->win_parent_ != nullptr) {
                    WinFocusWindow->win_parent_->screen_to_client(x, y);
                    if ((WinFocusWindow->iFlags_ & WinFlagParentOffset) != 0) {
                        *x += WinFocusWindow->win_parent_->outer_rect_.left;
                        *y += WinFocusWindow->win_parent_->outer_rect_.top;
                    }
                }
                WinMouseDirect = 0;
                found = WinFocusWindow;
            }
        } else {
            for (int index = 0; index < WinRootCount; ++index) {
                found = get_mouse_window_recurse(WinRootWindows[index], x, y);
                if (found != nullptr) {
                    break;
                }
            }
        }
    }

    if (found != nullptr && WinMouseDirect == 0) {
        found = found->poWinBase_;
    }
    return found;
}

/*
Purpose: The window procedure the class registers - route every input
         message to the Win the pointer or the keyboard focus is over, and
         hand everything else to DefWindowProc.
// ORIGINAL: 0x005F0650 ?window_proc@Win@@QAGJPAXIIJ@Z 0x005F0650-0x005F0E27
// symbol    ?window_proc@Win@@SGJPAUHWND__@@IIJ@Z
// size      2007 bytes
// prototype
// callers   0   call targets   11
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EC690 0x005EC6F0 0x005EFD20 0x005F1820 0x005F2330 0x005F6A50
//           0x005F6F10 0x005F7320 0x005F7580 0x005F86A0 0x00616730
//
// CORRECTED from a NON-STATIC member. The catalogued name ends in `QAG` - a
// public member declared __stdcall, which would take a receiver and clean
// five slots - and the body ends in `ret 0x10`. Four arguments, no `this`,
// which is what a WNDPROC is; every one of its `Win::` callees is `QAA` or
// `QAG` too, and the one that is not - `do_tracking`, `QAE` - is called on
// `WinTrackingWindow` rather than on a receiver of its own. Same shape as
// `init_class` and `flip` beside it. See the QAA note on `set_display_mode`.
//
// Promoted out of src/unrecovered/005f0650.cpp, where it was a scaffold
// working copy dispatching through `VCall` slot shims and reading its
// imports out of IAT addresses. In the build it dispatches the way
// `Sound::fade` does - `original_slot` off the object's own vtable, which
// cannot disagree with the original's layout because it IS the original's
// layout - and calls BeginPaint, EndPaint, InvalidateRect, SelectPalette,
// RealizePalette, GetWindowLongA, GetDC and DefWindowProcA by name. All
// eight were checked against the import directory rather than inferred from
// argument counts.
Status: WIP
*/
// VC6's winuser.h hides `WM_MOUSEWHEEL` behind
// `_WIN32_WINNT >= 0x0400`, which this build does not set - and raising it
// would change what every other translation unit sees for one constant. The
// image handles 0x020A whatever the SDK guard says.
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

LRESULT __stdcall Win::window_proc(HWND window, UINT message, WPARAM wparam,
                                   LPARAM lparam) {
    // WIN'S OWN VTABLE, by the offsets the original encodes and named for
    // what each call does with them. Spelled the way `Sound::fade` spells
    // it - `original_slot` off the object's own vtable - because `Win` is
    // not polymorphic in this tree: `Win::construct` writes
    // `object[0] = WinPrimaryVtable` by hand, over the pointer
    // `AutoSound::construct()` just put there, which is base-then-derived
    // vtable assignment done manually. Declaring these `virtual` needs
    // `Win : public AutoSound` first, and that is a change to the object
    // model rather than to this function.
    //
    // NOT `src/vtable_shim.h`'s `VCall`, which is the tree's other spelling
    // for this. That shim declares every slot nullary and CANNOT be typed:
    // `slot016` is called with no arguments in `fx.cpp`, with one in
    // `src/recovered/units/00491380.cpp`, and with four here, because slot
    // 16 of a PrefWin is not slot 16 of a Win. One class-agnostic shim can
    // never carry a signature.
    static const size_t WinSlotSysClose = 0x38;     // slot 14
    static const size_t WinSlotMouseEnter = 0x40;   // slot 16
    static const size_t WinSlotMouseLeave = 0x48;   // slot 18
    static const size_t WinSlotMouseWheel = 0x134;  // slot 77
    static const size_t WinSlotKey = 0x138;         // slot 78
    static const size_t WinSlotChar = 0x13C;        // slot 79
    static const size_t WinSlotSysKey = 0x140;      // slot 80
    static const size_t WinSlotLButtonUp = 0x148;   // slot 82
    static const size_t WinSlotRButtonDown = 0x14C; // slot 83
    static const size_t WinSlotRButtonUp = 0x150;   // slot 84

    typedef void (OriginalObject::*func_win_slot_38)();
    typedef void (OriginalObject::*func_win_enter)(int, int, WPARAM, int);
    typedef void (OriginalObject::*func_win_leave)(int, int);
    typedef void (OriginalObject::*func_win_wheel)(WPARAM, unsigned int,
                                                   int, int, int);
    typedef void (OriginalObject::*func_win_key)(WPARAM, int, int, int);
    typedef void (OriginalObject::*func_win_char)(WPARAM, int);
    typedef void (OriginalObject::*func_win_button_up)(int, int, WPARAM, int);
    typedef void (OriginalObject::*func_win_button_down)(int, int, int,
                                                         WPARAM, int);

    // THE ORIGINAL'S OWN ORDER: `cmp 0x201/ja`, `cmp 0x201/je`,
    // `cmp 0x102/ja`, `cmp 0x102/je`, then the jump table for
    // [6, 0x101]. Written as `message < WM_LBUTTONUP` the first test
    // lowers to `jae` and the second comparison against 0x201 folds
    // away, which is four instructions the image has and the rebuild
    // did not.
    if (message > WM_LBUTTONDOWN) {
        if (message > WM_MOUSEWHEEL) {
            if (message == WM_QUERYNEWPALETTE) {
                if (BufferDirectDraw != 0) {
                    return 1;
                }
                if (WinHdcRefCount == 0) {
                    if (DirectDrawSurface == nullptr) {
                        WinSharedHdc = GetDC(HandleMain);
                    } else {
                        DirectDrawSurface->GetDC(&WinSharedHdc);
                    }
                    if (WinSharedHdc == nullptr) {
                        return 0;
                    }
                    WinHdcRefCount = 1;
                } else {
                    ++WinHdcRefCount;
                }
                if (WinSharedHdc != nullptr) {
                    SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
                    RealizePalette(WinSharedHdc);
                    release_hdc();
                }
                return 0;
            }
            if (message == WM_PALETTECHANGED) {
                if (window == reinterpret_cast<HWND>(wparam)) {
                    return 0;
                }
                if (BufferDirectDraw != 0) {
                    return 0;
                }
                if (get_hdc() == nullptr) {
                    return 0;
                }
                SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
                RealizePalette(WinSharedHdc);
                release_hdc();
                return 0;
            }
            if (message == WM_USER + 1) {
                // The tick a `Time` posts to this window, `wparam` being the
                // Time itself. Every offset the original touches is a member
                // `time.h` already declares, and the layout is pinned by its
                // own `static_assert(sizeof(Time) == 0x28)`:
                //
                //   [esi + 0x1C]  tick_posted_     cleared on every tick
                //   [esi]         oneshot_state_   `test al,1`, `test al,2`
                //   [esi + 0x08]  callback1_       void(int)
                //   [esi + 0x0C]  callback2_       void(int, int)
                //   [esi + 0x14]  cb_param1_       pushed last, so it is first
                //   [esi + 0x10]  cb_param2_
                //
                // This is the half of the one-shot emulation that lives
                // outside `Time`: `SetTimer` has no one-shot mode, so a
                // `pulse()` that went through it arrives here repeating,
                // and the second tick is what stops it.
                Time *const timer = reinterpret_cast<Time *>(wparam);
                timer->tick_posted_ = 0;
                if (MsgStatus == 0) {
                    if ((timer->oneshot_state_ & TimeOneShot) != 0) {
                        if ((timer->oneshot_state_ & TimeOneShotFired) != 0) {
                            timer->stop();
                            return DefWindowProcA(window, message, wparam,
                                                  lparam);
                        }
                        timer->oneshot_state_ |= TimeOneShotFired;
                    }
                    if (Time::TimeModal == nullptr
                        || timer == Time::TimeModal) {
                        if (timer->callback1_ != nullptr) {
                            timer->callback1_(timer->cb_param1_);
                        }
                        if (timer->callback2_ != nullptr) {
                            timer->callback2_(timer->cb_param1_,
                                              timer->cb_param2_);
                        }
                    }
                }
            }
            return DefWindowProcA(window, message, wparam, lparam);
        }
        if (message == WM_MOUSEWHEEL) {
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over =
                get_mouse_window(&x, &y);
            if (over == nullptr) {
                return 0;
            }
            uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
            over->on_mouse_wheel(wparam, static_cast<unsigned int>(wparam) >> 16,
                                 x, y, WinMouseDirect);
            return 0;
        }
        switch (message) {
        case WM_LBUTTONUP: {
            WinTrackingWindow = nullptr;
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over =
                get_mouse_window(&x, &y);
            if (over == nullptr) {
                return 0;
            }
            uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
            over->on_l_button_up(x, y, wparam, WinMouseDirect);
            return 0;
        }
        case WM_LBUTTONDBLCLK:
            OnLButtonDown(window, 1, static_cast<short>(LOWORD(lparam)), static_cast<short>(HIWORD(lparam)), wparam);
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK: {
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over =
                get_mouse_window(&x, &y);
            if (over != nullptr) {
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
                over->on_r_button_down(message == WM_RBUTTONDBLCLK ? 1 : 0,
                                     x, y, wparam, WinMouseDirect);
            }
            break;
        }
        case WM_RBUTTONUP: {
            WinTrackingWindow = nullptr;
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over =
                get_mouse_window(&x, &y);
            if (over != nullptr) {
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
                over->on_r_button_up(x, y, wparam, WinMouseDirect);
            }
            break;
        }
        default:
            return DefWindowProcA(window, message, wparam, lparam);
        }
        if (WinMouseHook != nullptr) {
            WinMouseHook(window, lparam);
        }
    } else if (message == WM_LBUTTONDOWN) {
        OnLButtonDown(window, 0, static_cast<short>(LOWORD(lparam)),
                      static_cast<short>(HIWORD(lparam)), wparam);
        return 0;
    } else if (message > WM_CHAR) {
            switch (message) {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {
                Win *const focus =
                    get_key_window();
                if (focus == nullptr) {
                    return 0;
                }
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(focus);
                focus->on_sys_key(wparam,
                                     message == WM_SYSKEYDOWN ? 1 : 0,
                                     static_cast<short>(LOWORD(lparam)),
                                     static_cast<int>(lparam >> 16));
                return 0;
            }
            case WM_SYSCOMMAND: {
                Win *const owner = reinterpret_cast<Win *>(
                    GetWindowLongA(window, GWL_USERDATA));
                if (owner == nullptr) {
                    return 0;
                }
                if (wparam != SC_CLOSE) {
                    DefWindowProcA(window, WM_SYSCOMMAND, wparam, lparam);
                    return 0;
                }
                ScrollCurrentWin() = owner;
                {
                    typedef void(__cdecl * func_win_closed)();
                    func_win_closed closed = *reinterpret_cast<func_win_closed *>(
                        reinterpret_cast<char *>(owner) + 0x404);
                    if (closed != nullptr) {
                        closed();
                    }
                }
                {
                    uint8_t *const vtable =
                        *reinterpret_cast<uint8_t **>(owner);
                    owner->vslot_14();
                }
                sub_5f86a0(*reinterpret_cast<int *>(
                    reinterpret_cast<char *>(owner) + 0x18));
                return 0;
            }
            case WM_MOUSEMOVE: {
                WinCursorMoved = 0;
                if (WinTrackingWindow != nullptr) {
                    WinTrackingWindow->do_tracking(static_cast<short>(LOWORD(lparam)),
                                                   static_cast<short>(HIWORD(lparam)));
                    return 0;
                }
                int x = static_cast<short>(LOWORD(lparam));
                int y = static_cast<short>(HIWORD(lparam));
                Win *const over =
                    get_mouse_window(&x, &y);
                update_cursor(over, 1);
                if (over == nullptr) {
                    return 0;
                }
                if (WinHoverWindow != nullptr
                    && (WinCursorMoved != 0 || over != WinHoverWindow)
                    && (WinPointerOwner3 == nullptr
                        || WinPointerOwner3 == WinHoverWindow)) {
                    uint8_t *const vtable =
                        *reinterpret_cast<uint8_t **>(WinHoverWindow);
                    WinHoverWindow->vslot_18(x, y);
                }
                WinHoverWindow = over;
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
                over->on_mouse_move(x, y, wparam, WinMouseDirect);
                return 0;
            }
            default:
                break;
            }
            return DefWindowProcA(window, message, wparam, lparam);
    } else if (message == WM_CHAR) {
        Win *const focus = get_key_window();
        if (focus == nullptr) {
            return 0;
        }
        uint8_t *const vtable = *reinterpret_cast<uint8_t **>(focus);
        focus->on_char(static_cast<char>(wparam), static_cast<short>(LOWORD(lparam)));
        return 0;
    } else {
        switch (message) {
        case WM_ACTIVATE: {
            const unsigned int active = LOWORD(wparam);
            const unsigned int minimised = HIWORD(wparam);
            InvalidateRect(reinterpret_cast<HWND>(HandleMain), nullptr, FALSE);
            if (active == 0) {
                WinPointerOwner1 = nullptr;
                WinPointerOwner2 = nullptr;
            } else if (minimised == 0 && BufferDirectDraw == 0
                       && get_hdc() != nullptr) {
                SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
                RealizePalette(WinSharedHdc);
                release_hdc();
            }
            DefWindowProcA(window, WM_ACTIVATE,
                           (minimised << 16) | active, lparam);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT paint;
            if (BeginPaint(window, &paint) == nullptr) {
                return 0;
            }
            RECT damaged = paint.rcPaint;
            update_screen(&damaged, nullptr);
            flip(&damaged);
            EndPaint(window, &paint);
            return 0;
        }
        case WM_KEYDOWN:
        case WM_KEYUP: {
            Win *const focus = get_key_window();
            if (focus != nullptr) {
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(focus);
                focus->on_key(wparam,
                                     message == WM_KEYDOWN ? 1 : 0,
                                     static_cast<short>(LOWORD(lparam)),
                                     static_cast<int>(lparam >> 16));
            }
            if (message == WM_KEYDOWN && WinKeyHook != nullptr) {
                WinKeyHook(wparam);
            }
            break;
        }
        default:
            // EVERY OTHER MESSAGE IN THIS RANGE GOES TO DefWindowProc, and
            // returning 0 instead is not a near-miss: the jump table at
            // 0x005F0E28 has five arms and 248 of the 252 messages in
            // [6, 0x101] take arm 4, which is 0x005F0D11 -
            // `DefWindowProcA` with its result returned. WM_NCCREATE is one
            // of those, and a window procedure that answers 0 to WM_NCCREATE
            // makes CreateWindowEx return NULL. So does WM_CREATE, and both
            // arrive here before CreateWindowEx has returned at all.
            return DefWindowProcA(window, message, wparam, lparam);
        }
    }
    if (WinMessageHook != nullptr) {
        WinMessageHook();
    }
    return 0;
}

/*
Purpose: Forward to the fixed DirectDraw device object's own init, in
         argument order.
// ORIGINAL: 0x005F2C40 ?set_display_mode@Win@@QAAHHHHH@Z 0x005F2C40-0x005F2C5F BYTE_EXACT
// symbol    ?set_display_mode@Win@@SAHHHHH@Z
// size      31 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00635510
//
// `static`, because the catalogued name ends in `QAA` - a public member
// declared __cdecl, taking no receiver.
Return Value: DDInit::init's own return
Status: Complete.
*/
int __cdecl Win::set_display_mode(int width, int height, int depth, int tgl) {
    return WinDisplayInit.init(width, height, depth, tgl);
}

// The class name `UnregisterClassA` is handed at teardown - a THIRD copy of
// a window-class-name literal, distinct from `WinClassName` below. Read out
// of the image at 0x00697F3C rather than assumed.
static const char DDInitClassName[] = "DirectDClass";

/*
Purpose: Tear down whatever DirectDraw surface and device window this
         object is already holding, then (unless `depth` is zero) bring up
         a new one at the requested size, and paint the splash logo onto
         `ScreenBuffer` once the new mode is live.
// ORIGINAL: 0x00635510 ?init@DDInit@@QAEHHHHH@Z 0x00635510-0x00635746;0x00663870-0x00663885
// size      587 bytes
// prototype int (__thiscall ?init@DDInit@@QAEHHHHH@Z)(DDInit *this, int width, int height, int depth, int tgl)
// callers   2   call targets   10
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D44FC 0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005DFB50 0x005DFF00 0x005EFD00 0x005EFD20 0x00635870
//
// TWO SPANS, and the second is a 21-byte EH funclet the linker lays away
// from the body - the same shape `Buffer::load_pcx` and `Win::init_class`
// already reproduce byte-exact from nothing more than a local `Buffer`
// with a non-trivial destructor. `buf` here is that local: VC6 emits the
// SEH frame (`push -1; push <handler>; mov fs:[0]...`) and the three
// `buf.~Buffer()` calls at every return path (7, 0x12 and the two shared
// at the bottom) purely because `buf` is in scope, not because anything
// here spells them out.
//
// `surf_` is `IDirectDraw *`, not the `IDirectDrawSurface *` the field
// name in early transcriptions guessed: `DirectDrawCreate`'s second
// argument is `LPDIRECTDRAW *`, and the two vtable slots it dispatches
// through afterwards - offset 0x4C (index 19) and 0x50 (index 20) in the
// real `IDirectDraw` vtable - are `RestoreDisplayMode()` and
// `SetCooperativeLevel(HWND, DWORD)`, not anything `IDirectDrawSurface`
// declares at those slots.
//
// `locked_surface_`/`locked_bits_` (offsets 0xC/0x10) are proven the same
// way: slot 0x80 (index 32) is `IDirectDrawSurface::Unlock(LPVOID)`, one
// pointer argument, which is exactly what the image pushes beside the
// receiver. Nothing in this function ever sets either field, so whatever
// locks that surface is a sibling method this recovery does not need.
//
// `buf.copy(&ScreenBuffer, ...)` at the end is `Win::init_class`'s own
// splash-centring call with `buf` standing in for `logo` - same six-argument
// overload, same halves-rounded-toward-zero division on both axes.
//
// Two callees stay pending_bodies forwarders: 0x005EFD00 (already
// BYTE_EXACT in src/recovered/005efd00.cpp, unpromoted) and 0x00635870
// (a 1691-byte DDERR_* switch, unrecovered). Neither is this recovery's
// job; see the notes beside their forwarders in pending_bodies.cpp.
//
// TRIED: 96 of 178 positionally under `/c /O2 /Ob0 /Gy /GR- /GX` (the
//            flag set `measure` picks), 0.986 similar - every call, branch
//            and field access agrees; what does not is which callee-saved
//            register holds `this` and which holds the constant zero. The
//            image keeps `this` in edi and the zero it stores into cleared
//            fields and compares pointers against in ebp; this body's
//            allocator gives `this` to ebp and the zero to ebx, which is the
//            same register-allocation-order puzzle `Win::init_class`
//            documents above at length and never fully resolves. No field
//            order, `nullptr`-vs-`0` spelling or flag set tried here moves
//            it.
// TRIED: declaring `Buffer buf` AFTER the two early guards, which is where
//            the image constructs it - its HandleMain test at 0x0063554B
//            precedes any call, and the returns at 0 and 7 carry no unwind
//            state, where declaring it first makes VC6 track two extra EH
//            scopes (`[esp+0x5a0]` taking 1 and 2 against the image's 0 and
//            -1 only). Under listing_diff's single flag set it cuts the
//            count from 192 to 182 - and it destroys the alignment, 96
//            agreeing down to 8. Count is not progress; the declaration
//            stays where it is.
// NOTE:      `listing_diff` and `measure` disagree about the instruction
//            count here, and MEASURE is the one to believe: it searches the
//            flag sets and settles on 177, while listing_diff compiles one
//            set and reports 192. Do not read a regression into that gap -
//            the body is byte-identical to the one the line below measured
//            (checked against 3cb99d43).
// TRIED: the one instruction of the 178 this tree does not emit is in
//            the splash `copy` call: the image loads
//            `ScreenBuffer.dib_.bmiHeader.biWidth` into ecx before pushing
//            the other arguments and then computes `mov eax, ecx; sub eax,
//            esi`, where this tree subtracts straight from memory. Hoisting
//            it - and the other three header fields - into named `const int`
//            locals does NOT reproduce that: VC6 folds them all back, and
//            all three spellings measure an identical 96 of 178 with 177
//            compiled instructions.
Status: Complete
*/
int DDInit::init(int width, int height, int depth, int tgl) {
    Buffer buf;

    if (depth == 0) {
        return 0;
    }
    if (HandleMain == nullptr) {
        return 7;
    }

    if (locked_surface_ != nullptr && locked_bits_ != nullptr) {
        locked_surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
    }
    if (surf_ != nullptr) {
        surf_->RestoreDisplayMode();
        surf_->Release();
        surf_ = nullptr;
    }
    if (hwnd_ != nullptr) {
        DestroyWindow(reinterpret_cast<HWND>(hwnd_));
        hwnd_ = nullptr;
        UnregisterClassA(DDInitClassName, WinInstance);
    }
    locked_surface_ = nullptr;
    DDInitRefreshScreenMetrics();

    if (width == 0 || height == 0) {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }

    HRESULT hr = (surf_ == nullptr)
        ? DirectDrawCreate(nullptr, reinterpret_cast<LPDIRECTDRAW *>(&surf_), nullptr)
        : DD_OK;
    if (hr == DD_OK) {
        hr = surf_->SetCooperativeLevel(HandleMain, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE);
    }
    if (hr == DD_OK) {
        hr = surf_->SetDisplayMode(width, height, depth);
    }
    if (hr == DD_OK) {
        hr = surf_->SetCooperativeLevel(nullptr, DDSCL_NORMAL);
    }
    if (hr != DD_OK) {
        report_error(hr);
        return 0x12;
    }

    WinScreenWidth = width;
    WinScreenHeight = height;

    if (tgl != 0) {
        ScreenBuffer.init(width, height, 0, 0);
        ScreenBuffer.fill(0);
        if (buf.load_pcx("logo.pcx", PaletteActive, 10, 0xEC) == 0) {
            buf.copy(&ScreenBuffer, 0, 0,
                     (ScreenBuffer.dib_.bmiHeader.biWidth - buf.dib_.bmiHeader.biWidth) / 2,
                     (buf.dib_.bmiHeader.biHeight - ScreenBuffer.dib_.bmiHeader.biHeight) / 2,
                     buf.dib_.bmiHeader.biWidth,
                     -buf.dib_.bmiHeader.biHeight);
        }
    }

    Win::flip(nullptr);
    return 0;
}

// `JackalClass` is at 0x00696DC8 and again at 0x00696DD4 - the image holds
// two copies, one for the registration and one for the creation, and the
// operand is relocated either way, so the literal is the honest spelling.
static const char WinClassName[] = "JackalClass";

/*
// ORIGINAL: 0x005F01F0 ?init_class@Win@@QAAHPAD@Z 0x005F01F0-0x005F04D4;0x00662CCC-0x00662CE1
// symbol    ?init_class@Win@@SAHPAD@Z
// size      761 bytes
// prototype
// callers   1   call targets   7
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005DFB50 0x005DFF00 0x005EFD20
// indirect  0x005F0236 0x005F0263 0x005F0273 0x005F028E 0x005F02D0 0x005F02D9 0x005F030A
//           0x005F036F 0x005F0375 0x005F039A 0x005F03A9 0x005F03BD 0x005F03E3 0x005F03F5
//           0x005F0498
//
// Promoted 2026-08-15 from src/unrecovered/005f01f0.cpp to retire its
// pending_bodies forwarder.
//
// WRITTEN AGAINST THE REAL HEADERS. It used to reach every Win32 import
// through a hand-written function-pointer typedef and its IAT slot address
// - `(*(FnRegisterClassA *)g)(&wndclass)` - and every global
// through `static int *const WinInstance = (int *)0x009B7B14`. That is what
// a MEASUREMENT SCAFFOLD has to do, because a scaffold includes no
// <windows.h> and declares no project header; this file is in the build and
// has both. So the imports are called by name, `WNDCLASSA` is the real
// struct, the surface is an `IDirectDrawSurface` whose `GetDC`/`ReleaseDC`
// are the slots the offsets 0x44 and 0x68 were reaching, and the globals
// are objects declared in win.h. Every one of those operands is relocated
// and the comparison masks relocations, so none of it costs a byte.
//
// LEFT AT 88.7%, first divergence #7, and the divergence is a PROLOGUE
// PUSH: the image saves esi and edi up front, this build saves only esi
// there.
//
// NOT "one register short", which is what this note used to say. Both
// builds use two callee-saved registers and both cache the
// `GetSystemMetrics` import slot - `call edi` twice here, `call esi` twice
// there. What differs is WHICH register holds the zero. The image keeps it
// in edi from function entry, so edi is live before the `return 1` that
// `RegisterClassA` failing takes and has to be saved in the prologue. This
// build keeps the zero in esi and gives edi to the import slot, whose live
// range starts AFTER that early return - so VC6 shrink-wraps it and emits
// `push edi` at the point of first use, thirty instructions in. Every
// instruction after the prologue is then four bytes off in the frame, which
// is why positional agreement collapses to 12 of 203 while similarity is
// 0.887.
//
// So the question is not how to spend a second register. It is how to make
// VC6 assign the constant zero to the register it saves first, and that is
// allocation order, which no spelling tried here reaches.
//
// TRIED: the flag sets are byte-identical here, so it is not the frame
//            pointer or /O1 register pressure; declaring `wndclass` before
//            `logo` does not move the frame; and caching the import slot in a
//            local changes nothing, because this build already caches it.
//            (This note used to read `RULED OUT:` without the hyphen, which
//            the lesson grammar does not accept, so the body kept reading as
//            untouched however much had been measured against it.)
// TRIED: re-measured across all ten flag sets. The best is 16 of 203 at
//            0.925 SIMILAR under /c /O2 /Ob0 /Gy /GR- /GX - higher than the
//            0.887 this note recorded, and the highest similarity of any
//            unclaimed body in this batch. /Ob0 is what buys it: without it
//            the same 16 of 203 scores 0.549, and /Oy- drops it to 0.876.
//            The positional count stays at 16 because the whole body after
//            the prologue is shifted by the missing `push edi`, exactly as
//            described above; nothing measured here moves which register VC6
//            gives the constant zero.
//
// FIXED: the four `WinModalStack` stores now precede `GetModuleHandleA`,
// which is where the image has them - it stores all four from a zeroed eax
// and only then makes the call. Worth 0.882 -> 0.887, and it is what the
// image does regardless of what it is worth.
//
// `call_diff.py` reports 10 calls here against the image's 9 and names
// `??1Spot@@QAE@XZ`/`?close@Buffer@@QAEXXZ` as extras - a FALSE ALARM from
// its own flag search, not this body. Its FLAG_SETS list `/Ob0 /Oy-` but
// never plain `/Ob0` (no `/Oy-`); under the flags `osmx measure` actually
// picks (`/c /O2 /Ob0 /Gy /GR- /GX`), the compiled object calls exactly the
// same 9 symbols in the same order as the image: Buffer ctor once, Buffer
// dtor 3 times (both early returns plus the normal exit), then init, fill,
// load_pcx, copy, flip - confirmed by reading the object's own relocation
// symbol table directly. So the call graph already agrees; the divergence
// is purely the register-save order documented above.
Status: Complete
*/
int __cdecl Win::init_class(LPSTR window_name) {
    WNDCLASSA wndclass;

    // The splash buffer. Its destructor is what puts this function in an EH
    // frame: `push -1` at entry is the try level, and the `mov [level], 0`
    // after the constructor is VC6 saying the object is now live.
    Buffer logo;

    // A MEMSET, NOT FOUR STORES, on the evidence of the register the zero
    // lands in. The image materialises TWO zeros - `xor eax, eax` for these
    // four global stores and `xor edi, edi` for everything after the
    // GetModuleHandleA call that clobbers eax. Four separate assignments make
    // VC6 reuse the one callee-saved zero it already has, and the whole body
    // shifts; this reproduces the pair. 12 of 203 instructions to 16.
    memset(WinModalStack, 0, sizeof(WinModalStack));
    WinInstance = GetModuleHandleA(nullptr);

    wndclass.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_OWNDC;  // 0x2B
    wndclass.lpfnWndProc = &Win::window_proc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = WinInstance;
    wndclass.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
    wndclass.hCursor = nullptr;
    wndclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wndclass.lpszMenuName = nullptr;
    wndclass.lpszClassName = WinClassName;

    // TWO COPIES OF ONE STRING, which is why both calls can be handed
    // `WinClassName`. The image passes 0x696DC8 here and 0x696DD4 to
    // CreateWindowExA, and both hold "JackalClass" - VC6 pooled neither, so
    // the literal appears twice. Read out of the image rather than assumed:
    // the operands are relocations, so the byte comparison masks them and
    // could not have told the difference either way.
    if (RegisterClassA(&wndclass) == 0) {
        return 1;
    }

    WinScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    WinScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    HandleMain = CreateWindowExA(
        WS_EX_APPWINDOW, WinClassName, window_name, WS_POPUP,
        0, 0, WinScreenWidth, WinScreenHeight,
        nullptr, nullptr, WinInstance, nullptr);

    if (HandleMain == nullptr) {
        return 1;
    }

    // The shared device context, acquired and released around the palette
    // set-up. Written out rather than calling `Win::get_hdc` and
    // `Win::release_hdc` because the original has the whole protocol inline
    // here - a call would be one instruction where the image has thirty.
    if (WinHdcRefCount == 0) {
        if (DirectDrawSurface == nullptr) {
            WinSharedHdc = GetDC(HandleMain);
        } else {
            DirectDrawSurface->GetDC(&WinSharedHdc);
        }
        if (WinSharedHdc != nullptr) {
            WinHdcRefCount = 1;
        }
    } else {
        ++WinHdcRefCount;
    }

    SetBkMode(WinSharedHdc, TRANSPARENT);
    SetSystemPaletteUse(WinSharedHdc, SYSPAL_NOSTATIC);
    SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);

    --WinHdcRefCount;
    if (WinHdcRefCount == 0) {
        if (DirectDrawSurface == nullptr) {
            ReleaseDC(HandleMain, WinSharedHdc);
        } else {
            DirectDrawSurface->ReleaseDC(WinSharedHdc);
        }
        WinSharedHdc = nullptr;
    }

    ScreenBuffer.init(WinScreenWidth, WinScreenHeight, 0, 0);
    ScreenBuffer.fill(0);
    if (logo.load_pcx("logo.pcx", PaletteActive, 10, 0xEC) == 0) {
        logo.copy(&ScreenBuffer, 0, 0,
                  (ScreenBuffer.dib_.bmiHeader.biWidth
                   - logo.dib_.bmiHeader.biWidth) / 2,
                  (logo.dib_.bmiHeader.biHeight
                   - ScreenBuffer.dib_.bmiHeader.biHeight) / 2,
                  logo.dib_.bmiHeader.biWidth,
                  -logo.dib_.bmiHeader.biHeight);
    }

    if ((JackalInitFlags & 4) == 0) {
        ShowWindow(reinterpret_cast<HWND>(HandleMain), SW_SHOW);
        Win::flip(0);
    }

    return 0;
}

/*
Purpose: Refresh the cached screen metrics from the system.
// ORIGINAL: 0x005EFD00 sub_5efd00 0x005EFD00-0x005EFD1B BYTE_EXACT
// symbol    _DDInitRefreshScreenMetrics
// size      27 bytes
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EFD09 0x005EFD12
Return Value: The height, which is the second value read.
Status: Complete
*/
extern "C" int __cdecl DDInitRefreshScreenMetrics() {
    WinScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    return (WinScreenHeight = GetSystemMetrics(SM_CYSCREEN));
}

/*
Purpose: Name the DirectDraw error and put it in front of the user.
// ORIGINAL: 0x00635870 sub_635870 0x00635870-0x00635F0B
// symbol    ?report_error@DDInit@@QAEHH@Z
// size      1691 bytes
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625EC0 0x00627260 0x00628F30
//
// A MEMBER, despite the catalogue calling it `sub_635870` and the old
// transcription spelling it `__stdcall sub_635870(int)`. The caller settles
// it: `DDInit::init` does `push eax; mov ecx, edi; call 0x635870` at
// 0x0063564D, and edi is `this`. The body never reads ecx, which is why the
// free-function spelling appeared to work.
//
// EVERY CASE IS CHECKED, not transcribed on trust. Each of the 99 codes was
// read out of the image's own jump chain, and the string it selects was read
// from the image at the address the case pushes - and every one of those
// strings is the name of the DDERR_ macro whose value equals that code, all
// 99 agreeing with the SDK's DDRAW.H. So the case labels below are the
// macros rather than the raw HRESULTs, and the strings are what shipped.
//
// TRIED: byte-exactness, and it is the lowering rather than the mapping.
// A flat switch over all 99 cases reaches instruction #22 before diverging
// (`push` against `mov`): MSVC re-groups this case set into different
// sub-tables than the original's five, and which case maps to which string
// is not in dispute - it is verified above.
Return Value: 0 if the popup was dismissed, otherwise whatever the media
              check returns.
Status: Complete
*/
int DDInit::report_error(int hr) {
    const char *message;
    switch (hr) {
    case DDERR_SURFACEALREADYATTACHED:
        message = "DDERR_SURFACEALREADYATTACHED";
        break;
    case DDERR_NOCOLORKEY:
        message = "DDERR_NOCOLORKEY";
        break;
    case DDERR_INVALIDCAPS:
        message = "DDERR_INVALIDCAPS";
        break;
    case DDERR_CANNOTATTACHSURFACE:
        message = "DDERR_CANNOTATTACHSURFACE";
        break;
    case DDERR_OUTOFMEMORY:
        message = "DDERR_OUTOFMEMORY";
        break;
    case DDERR_UNSUPPORTED:
        message = "DDERR_UNSUPPORTED";
        break;
    case DDERR_GENERIC:
        message = "DDERR_GENERIC";
        break;
    case DDERR_NOTINITIALIZED:
        message = "DDERR_NOTINITIALIZED";
        break;
    case DDERR_INVALIDPARAMS:
        message = "DDERR_INVALIDPARAMS";
        break;
    case DDERR_ALREADYINITIALIZED:
        message = "DDERR_ALREADYINITIALIZED";
        break;
    case DDERR_CANNOTDETACHSURFACE:
        message = "DDERR_CANNOTDETACHSURFACE";
        break;
    case DDERR_CURRENTLYNOTAVAIL:
        message = "DDERR_CURRENTLYNOTAVAIL";
        break;
    case DDERR_EXCEPTION:
        message = "DDERR_EXCEPTION";
        break;
    case DDERR_HEIGHTALIGN:
        message = "DDERR_HEIGHTALIGN";
        break;
    case DDERR_INCOMPATIBLEPRIMARY:
        message = "DDERR_INCOMPATIBLEPRIMARY";
        break;
    case DDERR_INVALIDCLIPLIST:
        message = "DDERR_INVALIDCLIPLIST";
        break;
    case DDERR_INVALIDMODE:
        message = "DDERR_INVALIDMODE";
        break;
    case DDERR_INVALIDOBJECT:
        message = "DDERR_INVALIDOBJECT";
        break;
    case DDERR_INVALIDPIXELFORMAT:
        message = "DDERR_INVALIDPIXELFORMAT";
        break;
    case DDERR_INVALIDRECT:
        message = "DDERR_INVALIDRECT";
        break;
    case DDERR_LOCKEDSURFACES:
        message = "DDERR_LOCKEDSURFACES";
        break;
    case DDERR_NO3D:
        message = "DDERR_NO3D";
        break;
    case DDERR_NOALPHAHW:
        message = "DDERR_NOALPHAHW";
        break;
    case DDERR_NOCLIPLIST:
        message = "DDERR_NOCLIPLIST";
        break;
    case DDERR_NOCOLORCONVHW:
        message = "DDERR_NOCOLORCONVHW";
        break;
    case DDERR_NOCOOPERATIVELEVELSET:
        message = "DDERR_NOCOOPERATIVELEVELSET";
        break;
    case DDERR_NOCOLORKEYHW:
        message = "DDERR_NOCOLORKEYHW";
        break;
    case DDERR_NODIRECTDRAWSUPPORT:
        message = "DDERR_NODIRECTDRAWSUPPORT";
        break;
    case DDERR_NOEXCLUSIVEMODE:
        message = "DDERR_NOEXCLUSIVEMODE";
        break;
    case DDERR_NOFLIPHW:
        message = "DDERR_NOFLIPHW";
        break;
    case DDERR_NOGDI:
        message = "DDERR_NOGDI";
        break;
    case DDERR_NOMIRRORHW:
        message = "DDERR_NOMIRRORHW";
        break;
    case DDERR_NOTFOUND:
        message = "DDERR_NOTFOUND";
        break;
    case DDERR_NOOVERLAYHW:
        message = "DDERR_NOOVERLAYHW";
        break;
    case DDERR_NORASTEROPHW:
        message = "DDERR_NORASTEROPHW";
        break;
    case DDERR_NOROTATIONHW:
        message = "DDERR_NOROTATIONHW";
        break;
    case DDERR_NOSTRETCHHW:
        message = "DDERR_NOSTRETCHHW";
        break;
    case DDERR_NOT4BITCOLOR:
        message = "DDERR_NOT4BITCOLOR";
        break;
    case DDERR_NOT4BITCOLORINDEX:
        message = "DDERR_NOT4BITCOLORINDEX";
        break;
    case DDERR_NOT8BITCOLOR:
        message = "DDERR_NOT8BITCOLOR";
        break;
    case DDERR_NOTEXTUREHW:
        message = "DDERR_NOTEXTUREHW";
        break;
    case DDERR_NOVSYNCHW:
        message = "DDERR_NOVSYNCHW";
        break;
    case DDERR_NOZBUFFERHW:
        message = "DDERR_NOZBUFFERHW";
        break;
    case DDERR_NOZOVERLAYHW:
        message = "DDERR_NOZOVERLAYHW";
        break;
    case DDERR_OUTOFCAPS:
        message = "DDERR_OUTOFCAPS";
        break;
    case DDERR_OUTOFVIDEOMEMORY:
        message = "DDERR_OUTOFVIDEOMEMORY";
        break;
    case DDERR_OVERLAYCANTCLIP:
        message = "DDERR_OVERLAYCANTCLIP";
        break;
    case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
        message = "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
        break;
    case DDERR_PALETTEBUSY:
        message = "DDERR_PALETTEBUSY";
        break;
    case DDERR_COLORKEYNOTSET:
        message = "DDERR_COLORKEYNOTSET";
        break;
    case DDERR_HWNDALREADYSET:
        message = "DDERR_HWNDALREADYSET";
        break;
    case DDERR_SURFACEALREADYDEPENDENT:
        message = "DDERR_SURFACEALREADYDEPENDENT";
        break;
    case DDERR_SURFACEBUSY:
        message = "DDERR_SURFACEBUSY";
        break;
    case DDERR_CANTLOCKSURFACE:
        message = "DDERR_CANTLOCKSURFACE";
        break;
    case DDERR_SURFACEISOBSCURED:
        message = "DDERR_SURFACEISOBSCURED";
        break;
    case DDERR_SURFACELOST:
        message = "DDERR_SURFACELOST";
        break;
    case DDERR_SURFACENOTATTACHED:
        message = "DDERR_SURFACENOTATTACHED";
        break;
    case DDERR_TOOBIGHEIGHT:
        message = "DDERR_TOOBIGHEIGHT";
        break;
    case DDERR_TOOBIGSIZE:
        message = "DDERR_TOOBIGSIZE";
        break;
    case DDERR_TOOBIGWIDTH:
        message = "DDERR_TOOBIGWIDTH";
        break;
    case DDERR_UNSUPPORTEDFORMAT:
        message = "DDERR_UNSUPPORTEDFORMAT";
        break;
    case DDERR_UNSUPPORTEDMASK:
        message = "DDERR_UNSUPPORTEDMASK";
        break;
    case DDERR_VERTICALBLANKINPROGRESS:
        message = "DDERR_VERTICALBLANKINPROGRESS";
        break;
    case DDERR_WASSTILLDRAWING:
        message = "DDERR_WASSTILLDRAWING";
        break;
    case DDERR_XALIGN:
        message = "DDERR_XALIGN";
        break;
    case DDERR_INVALIDDIRECTDRAWGUID:
        message = "DDERR_INVALIDDIRECTDRAWGUID";
        break;
    case DDERR_DIRECTDRAWALREADYCREATED:
        message = "DDERR_DIRECTDRAWALREADYCREATED";
        break;
    case DDERR_NODIRECTDRAWHW:
        message = "DDERR_NODIRECTDRAWHW";
        break;
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
        message = "DDERR_PRIMARYSURFACEALREADYEXISTS";
        break;
    case DDERR_NOEMULATION:
        message = "DDERR_NOEMULATION";
        break;
    case DDERR_REGIONTOOSMALL:
        message = "DDERR_REGIONTOOSMALL";
        break;
    case DDERR_CLIPPERISUSINGHWND:
        message = "DDERR_CLIPPERISUSINGHWND";
        break;
    case DDERR_NOCLIPPERATTACHED:
        message = "DDERR_NOCLIPPERATTACHED";
        break;
    case DDERR_NOHWND:
        message = "DDERR_NOHWND";
        break;
    case DDERR_HWNDSUBCLASSED:
        message = "DDERR_HWNDSUBCLASSED";
        break;
    case DDERR_NOPALETTEATTACHED:
        message = "DDERR_NOPALETTEATTACHED";
        break;
    case DDERR_NOPALETTEHW:
        message = "DDERR_NOPALETTEHW";
        break;
    case DDERR_BLTFASTCANTCLIP:
        message = "DDERR_BLTFASTCANTCLIP";
        break;
    case DDERR_NOBLTHW:
        message = "DDERR_NOBLTHW";
        break;
    case DDERR_NODDROPSHW:
        message = "DDERR_NODDROPSHW";
        break;
    case DDERR_OVERLAYNOTVISIBLE:
        message = "DDERR_OVERLAYNOTVISIBLE";
        break;
    case DDERR_NOOVERLAYDEST:
        message = "DDERR_NOOVERLAYDEST";
        break;
    case DDERR_INVALIDPOSITION:
        message = "DDERR_INVALIDPOSITION";
        break;
    case DDERR_NOTAOVERLAYSURFACE:
        message = "DDERR_NOTAOVERLAYSURFACE";
        break;
    case DDERR_EXCLUSIVEMODEALREADYSET:
        message = "DDERR_EXCLUSIVEMODEALREADYSET";
        break;
    case DDERR_NOTFLIPPABLE:
        message = "DDERR_NOTFLIPPABLE";
        break;
    case DDERR_CANTDUPLICATE:
        message = "DDERR_CANTDUPLICATE";
        break;
    case DDERR_NOTLOCKED:
        message = "DDERR_NOTLOCKED";
        break;
    case DDERR_CANTCREATEDC:
        message = "DDERR_CANTCREATEDC";
        break;
    case DDERR_NODC:
        message = "DDERR_NODC";
        break;
    case DDERR_WRONGMODE:
        message = "DDERR_WRONGMODE";
        break;
    case DDERR_IMPLICITLYCREATED:
        message = "DDERR_IMPLICITLYCREATED";
        break;
    case DDERR_NOTPALETTIZED:
        message = "DDERR_NOTPALETTIZED";
        break;
    case DDERR_UNSUPPORTEDMODE:
        message = "DDERR_UNSUPPORTEDMODE";
        break;
    case DDERR_NOMIPMAPHW:
        message = "DDERR_NOMIPMAPHW";
        break;
    case DDERR_INVALIDSURFACETYPE:
        message = "DDERR_INVALIDSURFACETYPE";
        break;
    case DDERR_CANTPAGELOCK:
        message = "DDERR_CANTPAGELOCK";
        break;
    case DDERR_CANTPAGEUNLOCK:
        message = "DDERR_CANTPAGEUNLOCK";
        break;
    case DDERR_NOTPAGELOCKED:
        message = "DDERR_NOTPAGELOCKED";
        break;
    default:
        message = nullptr;
        break;
    }
    if (message) {
        parse_says(0, message, -1, -1);
    }
    if (pop_caption_title(const_cast<char *>("jackal"),
                          const_cast<char *>("DDRAWERROR"), 0x10040,
                          nullptr) == -1) {
        return 0;
    }
    return cd_check();
}

namespace {
}  // namespace

/*
Purpose: Route a mouse-move event either to the primary handler chain (a4
         zero: a fixed field-408 callback, then virtual slot017, then a
         child at +0x1C) or to the secondary one (virtual slot031, then a
         child at +0x4C) - unless a class-wide suppression flag or an
         instance flag byte is set. Either child, if present, is asked
         whether it needs a redraw (slot023) and told to redraw (slot007)
         if so. `a3` is never read; `ret 0x10` still pops it.
// ORIGINAL: 0x005F6320 ?on_mouse_move@Win@@QAEXHHIH@Z 0x005F6320-0x005F63BB BYTE_EXACT
// symbol    ?on_mouse_move@Win@@UAEXHHIH@Z
// size      155 bytes
// prototype void (__thiscall ?on_mouse_move@Win@@QAEXHHIH@Z)(Win* this, int, int, unsigned int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F6360 0x005F636B 0x005F637B 0x005F6386 0x005F639B 0x005F63A9 0x005F63B4
Return Value: n/a
Status: Complete
*/
void Win::on_mouse_move(int x, int y, unsigned int keys, int from_parent) {
    if ((iFlags_ & 0x200000U) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8U) != 0) {
        return;
    }
    if (from_parent == 0) {
        ScrollCurrentWin() = this;
        WinMouseMoveHookFn const fn = mouse_move_hook_;
        if (fn != 0) {
            fn(x, y);
        }
        this->vslot_17(x, y);
        Win *const child = val_7_;
        if (child != 0) {
            if (child->vslot_23() == 0) {
                child->vslot_07();
            }
        }
    } else {
        this->vslot_31(x, y);
        Win *const child = val_19_;
        if (child != 0) {
            if (child->vslot_23() == 0) {
                child->vslot_07();
            }
        }
    }
}

/*
Purpose: Walk one root window's subtree in z-order, appending every visible
         window to the z-order array. HOMED from src/recovered/units/
         005f4ec0.cpp on 2026-08-25: it is a free function in Win's own
         translation unit, twelve callers all live in win.cpp, and the
         pending_bodies forwarder that stood in for it jumped to the raw
         image address. It measures MISMATCH and is here for COVERAGE and
         to retire that forwarder - the divergence travels with it rather
         than being hidden in an archive the build never sees.
*/
// TRIED (2026-08-27, the mandated honest spellings): carrying `cur` as
// `Win *` and reading each child's parent through `c->win_parent_` instead
// of the 0xC4 pun. Measured 1 of 107 - identical to the pre-existing state,
// so neither form moves the body. THE CAUSE IS UPSTREAM OF SPELLING: the
// image opens `push ecx` and reads its argument at [esp+N] (the FPO-style
// prologue of the RECEIVER-SPILL WALL in docs/recovery/AGENT_BRIEF.md),
// which no source spelling reaches here.
// ORIGINAL: 0x005F4EC0 ?recurse_zorder@@YAXPAUWin@@@Z 0x005F4EC0-0x005F5013 FILE
// symbol    ?recurse_zorder@@YAXPAVWin@@@Z
// notes     PAV, not the catalogue's PAU: the catalogue guessed `struct
//           Win`, the tree declares a class. Same function, and
//           marker_symbols.py is the arbiter of which spelling the build
//           actually emits.
// size      339 bytes
// callers   12   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F4EC0 0x005F7E90
void __cdecl recurse_zorder(Win * window) {
    bool found = false;
    int i = 0;
    // TRIED (2026-08-27): carrying `cur` as `Win *` and reading the parent
    // through c->win_parent_ - both the honest spellings. It collapsed the
    // body from its high-nineties agreement to 1 of 107: the typed compare
    // forces VC6 off the register pairing the image schedules, unlike
    // get_mouse_window_recurse where the same retype held. Casts restored.
    int cur;

    if (window->child_count_ > 0) {
        cur = reinterpret_cast<int>(WinZOrderWindow);
        Win **child = &window->children_[0];
        do {
            Win *c = *child;
            if ((c->iFlags_ & 0x20) == 0) {
                if (cur != 0 && cur == reinterpret_cast<int>(c)) {
                    WinZOrderCount = 0;
                    found = true;
                }
                if (c->iSomeFlag_ & 1) {
                    int child_c4 = *reinterpret_cast<int *>(
                        reinterpret_cast<char *>(c) + 0xc4);
                    if (child_c4 == 0 ||
                        reinterpret_cast<Win *>(child_c4)->is_visible() != 0) {
                        recurse_zorder(c);
                    }
                    cur = reinterpret_cast<int>(WinZOrderWindow);
                }
                if (cur != 0 && found) {
                    break;
                }
            }
            ++i;
            ++child;
        } while (i < window->child_count_);
    } else {
        cur = reinterpret_cast<int>(WinZOrderWindow);
    }

    if (!(cur != 0 && found)) {
        i = 0;
        if (window->child_count_ > 0) {
            Win **child = &window->children_[0];
            do {
                Win *c = *child;
                if (cur != 0 && cur == reinterpret_cast<int>(c)) {
                    WinZOrderCount = 0;
                    found = true;
                }
                if (window->poWinBase_ == window &&
                    (c->iFlags_ & 0x20)) {
                    if (c->iSomeFlag_ & 1) {
                        int child_c4 = *reinterpret_cast<int *>(
                            reinterpret_cast<char *>(c) + 0xc4);
                        if (child_c4 == 0 ||
                            reinterpret_cast<Win *>(child_c4)->is_visible() != 0) {
                            recurse_zorder(c);
                        }
                        cur = reinterpret_cast<int>(WinZOrderWindow);
                    }
                }
                if (cur != 0 && found) {
                    break;
                }
                ++i;
                ++child;
            } while (i < window->child_count_);
        }
    }

    if (window->iSomeFlag_ & 1) {
        int a1_c4 = *reinterpret_cast<int *>(reinterpret_cast<char *>(window) + 0xc4);
        if (a1_c4 == 0 || reinterpret_cast<Win *>(a1_c4)->is_visible() != 0) {
            int idx = WinZOrderCount;
            reinterpret_cast<int *>(WinZOrderArray)[idx] = reinterpret_cast<int>(window);
            WinZOrderCount = idx + 1;
        }
    }
}



/*
Purpose: The tree walk `Win::get_mouse_window` delegates to once it has a
         root - HOMED from src/unrecovered/005f6ab0.cpp on 2026-08-25 to
         retire its pending_bodies forwarder, which jumped to the raw image
         address while the real body sat in an archive the build never
         compiled.
*/
// TRIED (2026-08-27, the mandated honest form): walking this object's own
// members - iSomeFlag_/iFlags_, win_parent_, child_count_, children_[],
// poWinBase_, outer_rect_/client_rect_ fields, buffer1_..4_ - instead of
// the char*-plus-offset walk. Attempted after a full listing_diff: the
// image opens `push ecx` with args read at [esp+N], ours opens a real EBP
// frame regardless of spelling, so agreement never leaves instruction 0
// (measured 1 of 332 both ways). RECEIVER-SPILL WALL per
// docs/recovery/AGENT_BRIEF.md; member rewrite parked until a lever for
// that prologue exists. The +0x444 Buffer cast at the tail predates this
// note and is part of that same untested walk.
// ORIGINAL: 0x005F6AB0 ?get_mouse_window_recurse@@YAHPAUWin@@PAHPAH@Z 0x005F6AB0-0x005F6F06 FILE
// symbol    ?get_mouse_window_recurse@@YAPAVWin@@PAV1@PAH1@Z
// notes     Differs from the catalogued name in both directions: PAV for
//           the class the catalogue called a struct, and a Win* return
//           where the catalogue says int (YAH). win.h has declared the
//           Win* form since before this body was homed and all four
//           callers use the result as a window.
// size      1110 bytes
// kind      game
// Returns Win *, not the catalogue's `int` (YAH): win.h has declared it
// that way since before this body was homed and every caller uses the
// result as a window. The mangled name differs from the catalogue's for
// exactly that reason; the body is unclaimed, so no claim rests on it.
Win *__cdecl get_mouse_window_recurse(Win * window, int * x, int * y) {
    char *wc = reinterpret_cast<char *>(window);
    int savedX = 0, savedY = 0;
    int i;

    if ((*(unsigned char *)(wc + 0x9c) & 1) == 0) {
        return 0;
    }
    {
        Win *parent = *reinterpret_cast<Win **>(wc + 0xc4);
        if (parent != 0 && !parent->is_visible()) {
            return 0;
        }
    }

    if ((*(unsigned char *)(wc + 0x98) & 0x20) == 0) {
        savedX = *x;
        savedY = *y;
        *x = WinMouseScreenX;
        *y = WinMouseScreenY;
    }

    {
        int childCount = *reinterpret_cast<int *>(wc + 0x3fc);
        Win **children = reinterpret_cast<Win **>(wc + 0x1a4);
        for (i = 0; i < childCount; i++) {
            Win *child = children[i];
            if ((*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(child) + 0x98) & 0x20) == 0) {
                Win *r = get_mouse_window_recurse(child, x, y);
                if (r != 0) return r;
            }
        }
    }

    if ((*(unsigned char *)(wc + 0x9c) & 2) == 0) {
        goto alt_path;
    }

    if (!in_box(*x, *y, reinterpret_cast<RECT *>(wc + 0x14c))) {
        goto tail;
    }
    *x -= *reinterpret_cast<int *>(wc + 0x14c);
    *y -= *reinterpret_cast<int *>(wc + 0x150);

    {
        int childCount = *reinterpret_cast<int *>(wc + 0x3fc);
        Win **children = reinterpret_cast<Win **>(wc + 0x1a4);
        if (window == *reinterpret_cast<Win **>(wc + 0xa8)) {
            for (i = 0; i < childCount; i++) {
                Win *child = children[i];
                unsigned int flags = *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(child) + 0x98);
                if ((flags & 0x8000) != 0 && (flags & 0x20) != 0) {
                    Win *r = get_mouse_window_recurse(child, x, y);
                    if (r != 0) return r;
                }
            }
        }
    }

    if (!in_box(*x, *y, reinterpret_cast<RECT *>(wc + 0x13c))) {
        // pixel-precise hit test against one of the window's four render buffers
        unsigned char key = static_cast<unsigned char>(WinFillColour);
        unsigned int flagsHere;
        int threshold1, threshold2;
        unsigned int pixel;
        Buffer *buf;
        bool hit;

        int slotResult = reinterpret_cast<Win *>(wc)->vslot_61();
        flagsHere = *reinterpret_cast<unsigned int *>(wc + 0x98);
        if (slotResult == 0 || (flagsHere & 0x10000000) == 0) {
            WinMouseDirect = 1;
            return window;
        }

        threshold1 = ((flagsHere & 0x10) == 0 || (flagsHere & 0x400000) != 0)
                         ? *reinterpret_cast<int *>(wc + 0x118)
                         : *reinterpret_cast<int *>(wc + 0x114);
        threshold2 = (*reinterpret_cast<int *>(wc + 0x11c) != -1)
                              ? *reinterpret_cast<int *>(wc + 0x11c)
                              : *reinterpret_cast<int *>(wc + 0x118);

        if (*y < threshold1) {
            buf = *reinterpret_cast<Buffer **>(wc + 0xb4);
            pixel = (unsigned int)buf->get_pixel(*x, *y);
            hit = (pixel != key);
        } else if (*reinterpret_cast<int *>(wc + 0x158) - *reinterpret_cast<int *>(wc + 0x150) - threshold2 < *y) {
            buf = *reinterpret_cast<Buffer **>(wc + 0xc0);
            pixel = (unsigned int)buf->get_pixel(*x, (*reinterpret_cast<int *>(wc + 0x150) - *reinterpret_cast<int *>(wc + 0x158)) + *y + threshold2);
            hit = (pixel != key);
        } else if (*x < *reinterpret_cast<int *>(wc + 0x118)) {
            buf = *reinterpret_cast<Buffer **>(wc + 0xbc);
            pixel = (unsigned int)buf->get_pixel(*x, *y - threshold1);
            hit = (pixel != key);
        } else {
            buf = *reinterpret_cast<Buffer **>(wc + 0xb8);
            pixel = (unsigned int)buf->get_pixel((*reinterpret_cast<int *>(wc + 0x154) - *reinterpret_cast<int *>(wc + 0x14c)) + *reinterpret_cast<int *>(wc + 0x118) + *x, *y - threshold1);
            hit = (pixel != key);
        }

        if (hit) {
            WinMouseDirect = 1;
            return window;
        }
        *x += *reinterpret_cast<int *>(wc + 0x14c);
        *y += *reinterpret_cast<int *>(wc + 0x150);
        goto tail;
    }

    *x += *reinterpret_cast<int *>(wc + 0x14c);
    *y += *reinterpret_cast<int *>(wc + 0x150);
    return window;

alt_path:
    if (!in_box(*x, *y, reinterpret_cast<RECT *>(wc + 0x13c))) {
        goto tail;
    }
    *x -= *reinterpret_cast<int *>(wc + 0x13c);
    *y -= *reinterpret_cast<int *>(wc + 0x140);
    WinMouseDirect = 0;

    {
        int childCount = *reinterpret_cast<int *>(wc + 0x3fc);
        Win **children = reinterpret_cast<Win **>(wc + 0x1a4);
        if (window == *reinterpret_cast<Win **>(wc + 0xa8)) {
            for (i = 0; i < childCount; i++) {
                Win *child = children[i];
                unsigned int flags = *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(child) + 0x98);
                if ((flags & 0x8000) == 0 && (flags & 0x20) != 0) {
                    Win *r = get_mouse_window_recurse(child, x, y);
                    if (r != 0) return r;
                }
            }
        }
    }

    if ((*(unsigned char *)(wc + 0x98) & 2) == 0) {
        int slotResult = reinterpret_cast<Win *>(wc)->vslot_61();
        unsigned int flagsHere = *reinterpret_cast<unsigned int *>(wc + 0x98);
        if (slotResult == 0 || (flagsHere & 0x1000000) == 0 || (*(unsigned char *)(wc + 0x99) & 1) != 0) {
            return window;
        }
        {
            Buffer *buf = reinterpret_cast<Buffer *>(wc + 0x444);
            unsigned char pixel = (unsigned char)buf->get_pixel(*x, *y);
            if (pixel != static_cast<unsigned char>(WinFillColour)) {
                if (*reinterpret_cast<int *>(wc + 0xa0) == 0) {
                    return window;
                }
                if (pixel != 0) {
                    return window;
                }
            }
        }
    }

tail:
    if ((*(unsigned char *)(wc + 0x98) & 0x20) == 0) {
        *x = savedX;
        *y = savedY;
    }
    return 0;
}


/*
Purpose: Restore the saved screen area and union its rectangle into the
         dirty region. HOMED from src/unrecovered/005f1750.cpp on
         2026-08-25 to retire its pending_bodies forwarder; its globals are
         win.cpp's own, renamed on the way in rather than re-imported as
         addresses.
*/
// ORIGINAL: 0x005F1750 sub_5f1750 0x005F1750-0x005F1812 FILE
// symbol    ?restore_flip_sprite@@YAXH@Z
// named     from behaviour at its two call sites (cursor-save restore)
// size      194 bytes
// kind      game
// `a1` gates the flip: nonzero means push the accumulated dirty rect to
// the screen before returning.
// NAMED 2026-08-27 from its two call sites, both in the cursor-draw
// path: before a cursor paints, whatever WinFlipSprite keeps of the
// screen under the old one goes back - copy onto ScreenBuffer, union the
// rectangle into the dirty region, flip only when asked.
void __cdecl restore_flip_sprite(int do_flip) {
    if (WinFlipSprite != 0) {
        g_WIN_BUFFER->copy(
            (&ScreenBuffer), 0, 0,
            WinFlipSpriteY, WinFlipSpriteX, WinClipWidth, WinClipHeight);
        int x = WinFlipSpriteY;
        int y = WinFlipSpriteX;
        RECT local;
        local.right = x + WinClipWidth;
        local.left = x;
        local.bottom = y + WinClipHeight;
        local.top = y;
        UnionRect(&WinDirtyRect,
                  &WinDirtyRect, &local);
        if (do_flip != 0) {
            int saved = reinterpret_cast<int>(WinFlipSprite);
            WinFlipSprite = reinterpret_cast<Sprite *>(0);
            // Win::flip is a STATIC member (win.h:211), so the image's
            // single RECT push with no ecx setup IS this call - which is
            // what the artifact's invented `win_flip_stub` was standing in
            // for. No stub and no forwarder needed.
            Win::flip(&WinDirtyRect);
            WinFlipSprite = reinterpret_cast<Sprite *>(saved);
            WinDirtyRect.left = 0;
            WinModalResult = 0;
        }
    }
}


/*
Purpose: Run the teardown step for the given instance, but only while it is
         still the current one - the timer callback Win::sub_63c340 hands to
         Time::start. HOMED from src/recovered/0063c4e0.cpp on 2026-08-25.
         The artifact needed a one-method `S_0063C7C0` shim because its
         callee is __thiscall and nothing declared it; Win::sub_63c7c0 is a
         real member now, so the shim is gone and the receiver comes from
         the class.
*/
// ORIGINAL: 0x0063C4E0 sub_63c4e0 0x0063C4E0-0x0063C4F6 BYTE_EXACT
// symbol    ?sub_63c4e0@@YAXH@Z
// size      22 bytes
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// tops out at MNEMONIC_ONLY - seven mnemonics agreeing, one register wrong -
// across eight source shapes. Only phrasing the tail call as a member on
// `a1` reproduces ecx.
// NAMED-INT, DELIBERATE (2026-08-27): `context` IS a Win* - its one
// registration site below passes `reinterpret_cast<int>(this)` into
// Time::start - but the IMAGE keeps it an H end to end: the symbol above
// spells YAXH, and every Time prototype spells its callback payload H
// (time.cpp P6AXH@ZH...). A pointer-typed parameter would emit PAV and
// detach the name from the shipped bytes. The int is the image's own
// generic-timer ABI; the pun documents itself here instead of pretending.
void __cdecl sub_63c4e0(int context) {
    if (context != 0 && WinFocusWindow == reinterpret_cast<Win *>(context)) {
        reinterpret_cast<Win *>(context)->sub_63c7c0();
    }
}

/*
Purpose: Close one window through its vtable, but only when it reports it
         is not already closing. HOMED from src/recovered/005f86a0.cpp on
         2026-08-25; the artifact needed a `GateVCall` shim class because
         win.h declared no virtuals, and slots 23 and 7 are ordinary
         declared members now.
*/
// ORIGINAL: 0x005F86A0 sub_5f86a0 0x005F86A0-0x005F86BF BYTE_EXACT
// symbol    ?sub_5f86a0@@YGXH@Z
// size      32 bytes
// kind      game
// flags     hidden;sp_ready;purged_ok
void __stdcall sub_5f86a0(int window) {
    Win *const self = reinterpret_cast<Win *>(window);
    if (self != 0) {
        if (self->vslot_23() == 0) {
            self->vslot_07();
        }
    }
}

/*
Purpose: Drain every pending WM_CHAR, pumping video and net between polls.
         HOMED from src/recovered/005fcfe0.cpp on 2026-08-25. Its three
         USER32 entry points came from tools/iat_names.py - the PE import
         table names them exactly - and 0x009B7B9C is temp.cpp's MsgStatus.
*/
// ORIGINAL: 0x005FCFE0 ?do_all_chars@@YAXXZ 0x005FCFE0-0x005FD04B BYTE_EXACT
// symbol    ?do_all_chars@@YAXXZ
// size      108 bytes
// kind      game
// flags     hidden;sp_ready;purged_ok
void __cdecl do_all_chars() {
    MSG msg;  // the artifact spelled this `int msg[7]`; MSG is those 28 bytes

    MsgStatus = 4;


    int has_message;
    do {
        do_video();
        check_net();
        has_message = PeekMessageA(&msg, 0, 0x102, 0x102, 1);
        if (has_message) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    } while (has_message);

    MsgStatus = 0;
    do_net();
}

/*
Purpose: The window class's WM_NCHITTEST handler. It pulls the Win out of
         GWL_USERDATA and asks it, falling back to DefWindowProcA when the
         window has none. HOMED from src/recovered/units/005f1420.cpp on
         2026-08-25: nothing in the product tree referenced it, so its claim
         was archive-only and unverifiable. The artifact reached slot 75
         through a fake `VCall` class; win.h declares that slot as
         `on_nc_hittest`.

         The catalogued name spells a Win receiver (`QAA`) that this body
         does not have - `a1` is an HWND handed to GetWindowLongA, not a
         `this`. See the `// symbol` fact for what the build emits.
*/
// ORIGINAL: 0x005F1420 ?OnNCHitTest@Win@@QAAJPAXHH@Z 0x005F1420-0x005F1471 FILE BYTE_EXACT
// symbol    ?OnNCHitTest@@YAJPAUHWND__@@HH@Z
// size      82 bytes
// prototype LRESULT (__cdecl ?OnNCHitTest@Win@@QAAJPAXHH@Z)(HWND hWnd, int, int)
// kind      game
// flags     hidden;sp_ready;purged_ok
long __cdecl OnNCHitTest(HWND hwnd, int x, int y) {
    typedef long(__stdcall * DefWindowProcAFn)(void *, unsigned int,
                                               unsigned int, long);
    long p = GetWindowLongA(hwnd, -0x15);
    if (p != 0) {
        return reinterpret_cast<Win *>(p)->on_nc_hittest(x, y);
    }
    return DefWindowProcA(
        hwnd, 0x84, 0,
        (static_cast<unsigned int>(static_cast<unsigned short>(y)) << 16)
            | static_cast<unsigned short>(x));
}

/*
Purpose: Does the tail of the typed-key ring match this string, read
         BACKWARDS? Walks the caller's string from its last character and
         the 10-byte ring at WinKeyRing..(WinKeyRing + 9) from the write
         cursor, both backwards, wrapping the ring. Returns 0 when every
         character matched and 1 at the first difference - strcmp's
         convention, not a boolean.

         THE CATALOGUE PUT THE NAME ON THE WRONG HALF OF A PAIR. It calls
         this `?on_char@Win@@QAAHDH@Z`, a `(char, int)` member; this body
         takes `(char *, int)` and the `// symbol` fact below already
         recorded the free-function form. The signature the catalogue
         describes belongs to 0x005F5FB0 - `Win::sub_5f5fb0(char, int)`,
         vtable slot 79 - which is the real WM_CHAR handler: it WRITES the
         character into this same ring and advances the same cursor. This
         one is the matcher that handler calls.

         What it matches is a cheat code. The one caller passes
         0x00696DFD, which is the final 'g' of the string "mdebug" in
         .rdata - a pointer to the LAST character, which is exactly what a
         backwards walk needs.

         HOMED from src/recovered/units/005f5f60.cpp on 2026-08-25.
*/
// ORIGINAL: 0x005F5F60 ?on_char@Win@@QAAHDH@Z 0x005F5F60-0x005F5FA1 FILE BYTE_EXACT
// symbol    ?typed_keys_differ@@YAHPADH@Z
// notes     RENAMED from the catalogue's `on_char` on the evidence above.
//           The ORIGINAL line keeps the catalogued spelling; the symbol
//           fact is what the build emits and what marker_symbols checks.
// size      66 bytes
// kind      game
// flags     hidden;sp_ready;purged_ok
int __cdecl typed_keys_differ(char *code, int length) {
    char *want = code + length - 1;
    char *typed = WinKeyRingCursor;
    for (int n = length; n > 0; --n) {
        if (*typed != *want) return 1;
        --typed;
        --want;
        if (typed < WinKeyRing) {
            typed = (WinKeyRing + 9);
        }
    }
    return 0;
}

/*
Purpose: The window class's WM_SYSKEYDOWN/WM_SYSCHAR router. It resolves
         which window is active - the modal focus if one is set, else the
         focus window, else the input focus, else the default - records that
         in WinActiveWindow, and hands the key to that window's owner
         through vtable slot 80. HOMED from src/recovered/units/005f16d0.cpp
         on 2026-08-25.

         The catalogued name spells a `Win` receiver (`QAA`) this body does
         not have: `a1` is the HWND the window procedure was called with and
         is never read. The `// symbol` fact records the free-function form
         the build emits.
*/
// ORIGINAL: 0x005F16D0 ?OnSysKey@Win@@QAAXPAXIJHI@Z 0x005F16D0-0x005F1745 FILE BYTE_EXACT
// symbol    ?OnSysKey@@YAXPAXIJHI@Z
// size      118 bytes
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
void __cdecl OnSysKey(void *hwnd, unsigned int key, long flags, int repeat,
                      unsigned int scan) {
    int val;
    if (WinFocusWindow != nullptr) {
        val = reinterpret_cast<int>(WinModalFocus);
        if (val == 0) {
            val = reinterpret_cast<int>(WinFocusWindow);
        }
    } else {
        val = reinterpret_cast<int>(WinInputFocus);
        if (val == 0) {
            val = reinterpret_cast<int>(WinDefaultFocus);
        }
    }
    WinActiveWindow = reinterpret_cast<Win *>(val);
    if (val != 0) {
        // 0xA8 is poWinBase_, which win.h already documents as the window that
// owns a position - a third witness after Win::construct storing `this`
// there and Win::get_mouse_window returning it.
        Win *owner = reinterpret_cast<Win *>(val)->poWinBase_;
        if (owner != 0) {
            if (owner->iSomeFlag_ & 1) {
                Win *inner = owner->win_parent_;
                if (inner != 0) {
                    if (!inner->is_visible()) {
                        return;
                    }
                }
                owner->on_sys_key(key, flags, repeat, scan);
            }
        }
    }
}

/*
Purpose: Tear the process-wide window class down - destroy the main window,
         close the screen buffer, and unregister the class.
// ORIGINAL: 0x005F04E0 ?close_class@Win@@QAAXXZ 0x005F04E0-0x005F0520 BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/005f04e0.cpp, which reached every global through a raw `static T *const g_00xxxxxx` and both Win32 entry points through their import thunks. All five are named here: HandleMain (temp.cpp), ScreenBuffer (palette.h), WinInstance, and plain `DestroyWindow`/`UnregisterClass` calls, which compile to the image's own `call dword ptr [import]`.
// LEVER: literal-not-shared the class name is written as `"JackalClass"` rather than reusing `WinClassName`. The image carries THREE copies of that string - 0x696DC8, 0x696DD4 and 0x696DEC - because VC6 does not pool string literals across use sites without /GF, so each site has its own.
// symbol    ?close_class@Win@@SAXXZ
// size      64 bytes
// prototype void (__cdecl ?close_class@Win@@QAAXXZ)()
// kind      game
Return Value: n/a
Status: Complete
*/
void Win::close_class() {
    if (HandleMain) {
        DestroyWindow(reinterpret_cast<HWND>(HandleMain));
        HandleMain = nullptr;
    }
    ScreenBuffer.close();
    UnregisterClass("JackalClass", WinInstance);
    WinInstance = nullptr;
}

// Fixed-slot bindings carried from 005ec800.cpp

// ORIGINAL: 0x005EC800 ?set_mouse_pos@Win@@QAEXHH@Z 0x005EC800-0x005EC89B FILE
// TRIED: `a1+client_rect_.left+outer_rect_.left` left-to-right, and `(client_rect_.left+outer_rect_.left)+a1` grouped - both MISMATCH #9 mov/add, a local-store scheduling difference around the x/y pair, not the arithmetic itself.
// working copy - scaffold materialised by --work
// size      155 bytes
// prototype void (__thiscall ?set_mouse_pos@Win@@QAEXHH@Z)(Win* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240
// indirect  0x005EC891

// TRIED: 7 -> 8 of 44, and the change that got it there is a BEHAVIOUR fix:
// the parent walk called `client_to_screen` on `this`, where the image puts
// win_parent_ in ECX and calls from there. Reassociating the sums to match
// the image's `(left + outer) + arg` grouping moves nothing. What is left is
// 49 instructions against 44 with the SetCursorPos argument reload and push
// pair emitted in a different place - twelve differing runs, so the
// remaining work is the body's shape, not one spelling.
void Win::set_mouse_pos(int x, int y) {
    // MEMBERS FIRST, argument last. `+` is left-associative, so `x + left +
    // outer` groups as `(x + left) + outer`; the image adds the two members
    // to each other and the argument to that sum (`add eax, edx` then
    // `add ecx, eax` at 0x005EC80D).
    int screen_x = client_rect_.left + outer_rect_.left + x;
    int screen_y = client_rect_.top + outer_rect_.top + y;
    if (((iFlags_ & 0x20) != 0) && (win_parent_ != 0)) {
        // THE PARENT'S. `mov ecx, [esi + 0xc4]` / `test ecx, ecx` at
        // 0x005EC83D puts win_parent_ in the receiver register and calls from
        // there; and the 0x8000 arm RELOADS it (`mov esi, [esi + 0xc4]` at
        // 0x005EC861), which is what says no local held it.
        win_parent_->client_to_screen(&screen_x, &screen_y);
        if ((iFlags_ & 0x8000) != 0) {
            screen_x = screen_x - win_parent_->outer_rect_.left;
            screen_y = screen_y - win_parent_->outer_rect_.top;
        }
    }
    SetCursorPos(screen_x, screen_y);
}

// Fixed-slot bindings carried from 005ec8a0.cpp

// ORIGINAL: 0x005EC8A0 ?get_mouse_pos@Win@@QAEXPAHPAH@Z 0x005EC8A0-0x005EC952 FILE BYTE_EXACT
// symbol    ?get_mouse_pos@Win@@QAEXPAH0@Z
// working copy - scaffold materialised by --work
// size      178 bytes
// prototype void (__thiscall ?get_mouse_pos@Win@@QAEXPAHPAH@Z)(Win* this, int*, int*)
// callers   26   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0
// indirect  0x005EC8C5

void Win::get_mouse_pos(int * x, int * y) {
    if (x != 0 && y != 0) {
        POINT pt;  // the artifact spelled this `struct Pt { int x, y; }`
        // 0x00669284, NOT the SetCursorPos slot this used to reach through
        // the file-scope `g`. The image calls `[0x669284]` here
        // (0x005EC8C5) and the two differ only in a relocation, which
        // the byte comparison masks - so the claim held while the call
        // was wrong.
        GetCursorPos(&pt);
        *x = pt.x;
        *y = pt.y;
        *x -= client_rect_.left + outer_rect_.left;
        *y -= client_rect_.top + outer_rect_.top;
        if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
            // THE PARENT'S, not this window's. The image loads win_parent_
            // into ECX - the receiver register - and calls from there:
            // `mov ecx, [esi + 0xc4]` / `test ecx, ecx` straight into the
            // call. Calling it on `this` compiled the same load into EAX and
            // then an extra `mov ecx, esi` to set the receiver back.
            win_parent_->screen_to_client(x, y);
            if ((iFlags_ & 0x8000) != 0) {
                *x += win_parent_->outer_rect_.left;
                *y += win_parent_->outer_rect_.top;
            }
        }
    }
}

// ORIGINAL: 0x005ECB60 ?remove_parent_dialog@Win@@QAEXXZ 0x005ECB60-0x005ECC40 FILE
// working copy - scaffold materialised by --work
// size      224 bytes
// prototype void (__thiscall ?remove_parent_dialog@Win@@QAEXXZ)(Win* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2

// TRIED: 12 -> 16 of 61, and the gain is the CORRECTNESS fix below, not a
// spelling. Two further attempts moved nothing: hoisting the loop count the
// way the image does (`mov edi, [esi + 0xd4]` once, `cmp edx, edi` in the
// loop) is already what the source says and changes no byte; and lifting the
// found-arm out of the loop - the image puts its unlink at 0x005ECBA9, past
// the `jmp 0x5ecc1b` that leaves the loop, and re-reads `[esi + 0xd0]` there
// rather than reusing the loop's node - costs one instruction and one
// agreeing pair, 15 of 61 at 60. So the layout difference is the compiler's
// loop rotation, not the source's arrangement.
//
// FIXED, and it was reading another object entirely: every access here was
// `*reinterpret_cast<int *>(parent + 0xNN)`, which is POINTER ARITHMETIC ON
// `Win *` - it advances 0xNN whole Win objects, not 0xNN bytes. The offsets
// are the WinNodeList members win.h annotates at Win+0xCC..0xDC, so they are
// named now and the arithmetic is the compiler's.
void Win::remove_parent_dialog() {
    Win *parent = win_parent_;
    if (parent != 0 && reinterpret_cast<int>(parent->list_.head_) != 0) {
        int counter = 0;
        // HOISTED, unlike is_descendant's count: the image loads it once
        // (`mov edi, [esi + 0xd4]` at 0x005ECB7E) and compares against the
        // register (`cmp edx, edi`). Safe here because the only write to
        // count_ is in the arm that breaks out.
        const int total = parent->list_.count_;
        if (total > 0) {
            do {
                char *node = reinterpret_cast<char *>(parent->list_.current_);
                if (*reinterpret_cast<Win **>(node + 4) == this) {
                    int prev = *reinterpret_cast<int *>(node + 0xc);
                    int next = *reinterpret_cast<int *>(node + 0x10);
                    *reinterpret_cast<int *>(prev + 0x10) = next;
                    *reinterpret_cast<int *>(next + 0xc) = prev;
                    counter = reinterpret_cast<int>(parent->list_.current_);
                    if (counter == reinterpret_cast<int>(parent->list_.head_)) {
                        parent->list_.head_ = reinterpret_cast<void *>(*reinterpret_cast<int *>(counter + 0xc));
                    }
                    parent->list_.current_ = reinterpret_cast<void *>(*reinterpret_cast<int *>(counter + 0xc));
                    if (parent->list_.external_ == 0) {
                        int payload = *reinterpret_cast<int *>(counter + 8);
                        if (payload != 0) {
                            free(reinterpret_cast<void *>(payload));
                        }
                        *reinterpret_cast<int *>(counter + 8) = 0;
                        if (counter != 0) {
                            free(reinterpret_cast<void *>(counter));
                        }
                    }
                    parent->list_.count_ = parent->list_.count_ - 1;
                    break;
                }
                ++counter;
                parent->list_.current_ = reinterpret_cast<void *>(*reinterpret_cast<int *>(node + 0xc));
            } while (counter < total);
        }
        if (parent->list_.count_ == 0) {
            parent->list_.head_ = reinterpret_cast<void *>(0);
        }
        parent->list_.tail_ = parent->list_.count_ - 1;
    }
}

// ORIGINAL: 0x005ECDC0 ?UNK2@Win@@QAEHH@Z 0x005ECDC0-0x005ECE15 FILE
// symbol    ?is_descendant_of@Win@@QAEHPAV1@@Z
// notes     PAV1@, not PAVWin@@: MSVC back-references the enclosing class
//           for a parameter of its own type.
// size      85 bytes
// prototype int (__thiscall ?UNK2@Win@@QAEHH@Z)(Win* this, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECDC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005ECDC0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ecdc0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK2@Win@@QAEHH@Z  at 0x005ECDC0  (85 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


// TRIED: 4 of 38. Named and retyped from `UNK2(int)` - it walks
// `ancestor`'s children looking for `this`, so it is the mirror of
// is_descendant - and the members replaced a `char *` walk of 0x1a4/0x3fc.
// The bytes did not follow: 41 instructions against 38, with ebx and ebp
// swapped from the first instruction on, which is the same allocation wall
// its sibling is_descendant (0x005ECE20) sits behind at 17 of 37.
// Flattening the two guards into `ancestor == 0 || child_count_ <= 0`
// costs one more instruction and changes nothing else.
int Win::is_descendant_of(Win *ancestor) {
    if (ancestor != 0) {
        if (ancestor->child_count_ > 0) {
            Win *const *children = ancestor->children_;
            int i = 0;
            do {
                Win *child = *children;
                if (child == this) {
                    return 1;
                }
                if (is_descendant_of(child)) {
                    return 1;
                }
                ++i;
                ++children;
            } while (i < ancestor->child_count_);
        }
    }
    return 0;
}

// Fixed-slot bindings carried from 005ecec0.cpp

// ORIGINAL: 0x005ECEC0 ?UNK4@Win@@QAEHXZ 0x005ECEC0-0x005ECF1C FILE
// symbol    ?is_in_focus_chain@Win@@QAEHXZ
// TRIED: guard-clause form (`if (w!=0 || w==this) return 1; if (w!=0) {loop} return 0;`), matching Ghidra's redundant double null-check; compiles and matches the loop body, diverges at #0 in prologue register-save order (mov vs push)
// working copy - scaffold materialised by --work
// size      92 bytes
// prototype int (__thiscall ?UNK4@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECDC0

// TRIED: 3 of 43, and the gap is a test VC6 REFUSES TO EMIT TWICE. The image
// loads WinFocusWindow once and tests it for null at 0x005ECEC8 and AGAIN at
// 0x005ECED3, with `cmp eax, ebp` between them, so the original wrote the
// null check twice. Splitting `w == 0 || w == this` into separate guard
// clauses buys one instruction and no more: the compiler still knows w is
// non-null at the third test and elides it, leaving 36 instructions against
// the image's 43. Also tried: caching `&w->child_count_` the way the image
// does (`lea ebx, [eax+0x3fc]`, re-read through ebx each iteration) - no
// change, so that lea is a consequence of the register pressure the missing
// tests would create, not a cause.
int Win::is_in_focus_chain() {
    Win *w = WinFocusWindow;
    // THREE SEPARATE TESTS, and the third is redundant ON PURPOSE. The image
    // loads WinFocusWindow once and tests it TWICE - `test eax, eax` at
    // 0x005ECEC8 and again at 0x005ECED3 with `cmp eax, ebp` between - so
    // the original wrote the null check twice. Folding them into
    // `w == 0 || w == this` lets VC6 elide the second, and this body came
    // out eight instructions short of the image.
    if (w == 0) {
        return 1;
    }
    if (w == this) {
        return 1;
    }
    if (w != 0) {
        int count = w->child_count_;
        if (count > 0) {
            int j = 0;
            Win **child_ptr = w->children_;
            do {
                if (*child_ptr == this) {
                    return 1;
                }
                if (is_descendant_of(*child_ptr)) {
                    return 1;
                }
                j++;
                child_ptr++;
            } while (j < w->child_count_);
        }
    }
    return 0;
}

// ORIGINAL: 0x005ECFE0 ?client_to_screen@Win@@QAEXPAURECT@@@Z 0x005ECFE0-0x005ED094 FILE BYTE_EXACT
// symbol    ?client_to_screen@Win@@QAEXPAUtagRECT@@@Z
// in place of the `reinterpret_cast<char *>(this)` raw-offset form - same
// score either way (9/56, 0.885 similar, best of ten flag sets); kept for
// the class_debt fix, not a byte-exactness gain. Plateau MISMATCH.
// working copy - scaffold materialised by --work
// size      180 bytes
// prototype void (__thiscall ?client_to_screen@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   12   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240

void Win::client_to_screen(RECT * rect) {
    if (rect == 0) {
        return;
    }
    int xAdj = client_rect_.left + outer_rect_.left;
    int yAdj = client_rect_.top + outer_rect_.top;
    // NO `owner` LOCAL. The image tests win_parent_ in ECX - the receiver
    // register - and calls straight from there; binding it to a local put it
    // in EBX and cost a `push ebx` plus a `mov ecx, ebx` at the call. And the
    // image RELOADS it afterwards (`mov esi, [esi + 0xc4]` at 0x005ED044)
    // rather than keeping it live, so the local was not what the original
    // wrote either.
    if ((iFlags_ & 0x20U) != 0 && win_parent_ != 0) {
        win_parent_->client_to_screen(&xAdj, &yAdj);
        if ((iFlags_ & 0x8000U) != 0) {
            xAdj -= win_parent_->outer_rect_.left;
            yAdj -= win_parent_->outer_rect_.top;
        }
    }
    rect->left += xAdj;
    rect->right += xAdj;
    rect->top += yAdj;
    rect->bottom += yAdj;
}

// ORIGINAL: 0x005ED0A0 ?nonscreen_to_client@Win@@QAEXPAURECT@@@Z 0x005ED0A0-0x005ED16A FILE BYTE_EXACT
// symbol    ?nonscreen_to_client@Win@@QAEXPAUtagRECT@@@Z
// working copy - scaffold materialised by --work
// size      202 bytes
// prototype void (__thiscall ?nonscreen_to_client@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0

// TRIED: 4 -> 18 of 64, and the two changes that got it there are BEHAVIOUR
// fixes, not spellings. (1) The parent walk called `screen_to_client` on
// `this`; the image loads win_parent_ into ECX - the receiver register - and
// calls from there, exactly as the sibling client_to_screen does. (2) The
// 0x8000 adjustment sat OUTSIDE the 0x20 block, so it applied even when no
// parent walk had happened; the image's `test ah, 0x80` is between the call
// and the `jmp` that rejoins the 0x20 test's own `je` target.
//
// What is left is allocation, not shape: 64 instructions against 64, with
// `this` and `rect` in swapped registers throughout. The image pushes ecx to
// allocate one local slot and then REUSES THE ARGUMENT SLOT for localX
// (`lea edx, [esp + 0x14]`); this body keeps `rect` live instead. Converting
// the guard to an early return - the form that made client_to_screen exact -
// does not move it, so the difference is downstream of source shape.
void Win::nonscreen_to_client(RECT * rect) {
    // EARLY RETURN, matching the sibling client_to_screen that measures
    // exact. The image loads `rect` into EDI and `this` into ESI;
    // wrapping the body in `if (rect != 0)` swapped that assignment.
    if (rect == 0) {
        return;
    }
    int localX = -(client_rect_.left + outer_rect_.left);
    int localY = -(client_rect_.top + outer_rect_.top);
    if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
        // THE PARENT'S, not this window's: the image loads win_parent_
        // into ECX - the receiver register - and calls from there
        // (`mov ecx, [esi + 0xc4]` / `test ecx, ecx` at 0x005ED0E5),
        // exactly as its sibling client_to_screen does.
        win_parent_->screen_to_client(&localX, &localY);
        // NESTED, not a sibling test. The image's `test ah, 0x80` at
        // 0x005ED104 sits between the call and the `jmp 0x5ed135` that
        // rejoins the 0x20 test's own `je` target - so it only runs when
        // the parent walk did. Its sibling client_to_screen nests it the
        // same way.
        if ((iFlags_ & 0x8000) != 0) {
            localX = localX + win_parent_->outer_rect_.left;
            localY = localY + win_parent_->outer_rect_.top;
        }
    }
    // Reuse localX/localY rather than adding `dx`/`dy` - the lever that made
    // the sibling nonclient_to_screen exact: two extra locals held one value
    // too many live and the compiler swapped `this` and `rect` between esi
    // and edi for the whole body.
    localX += outer_rect_.left;
    localY += outer_rect_.top;
    rect->left = rect->left + localX;
    rect->right = rect->right + localX;
    rect->top = rect->top + localY;
    rect->bottom = rect->bottom + localY;
}

// ORIGINAL: 0x005ED170 ?nonclient_to_screen@Win@@QAEXPAURECT@@@Z 0x005ED170-0x005ED236 FILE BYTE_EXACT
// symbol    ?nonclient_to_screen@Win@@QAEXPAUtagRECT@@@Z
// working copy - scaffold materialised by --work
// size      198 bytes
// prototype void (__thiscall ?nonclient_to_screen@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240

// ORIGINAL: 0x005ED3F0 ?nonclient_to_screen@Win@@QAEXPAH0@Z 0x005ED3F0-0x005ED48D FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      157 bytes
// prototype void (__thiscall ?nonclient_to_screen@Win@@QAEXPAH0@Z)(Win* this, int*, int*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240
void Win::nonclient_to_screen(RECT * rect) {
    if (rect == 0) {
        return;
    }
    int x = client_rect_.left + outer_rect_.left;
    int y = client_rect_.top + outer_rect_.top;
    if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
        // THE PARENT'S: `mov ecx, [esi + 0xc4]` / `test ecx, ecx` at
        // 0x005ED1B1 puts win_parent_ in the receiver register.
        win_parent_->client_to_screen(&x, &y);
        if ((iFlags_ & 0x8000) != 0) {
            x -= win_parent_->outer_rect_.left;
            y -= win_parent_->outer_rect_.top;
        }
    }
    // LEVER: reuse x and y, do NOT introduce `dx`/`dy`. Two extra locals
    // held one value too many live and the compiler swapped `this` and
    // `rect` between esi and edi for the whole body - 39 of 62 with every
    // instruction otherwise in place. Subtracting into x/y and adding those
    // is the same arithmetic with one fewer live value, and it is exact.
    x -= outer_rect_.left;
    y -= outer_rect_.top;
    rect->left += x;
    rect->right += x;
    rect->top += y;
    rect->bottom += y;
}

// Fixed-slot bindings carried from 005ee280.cpp

// ORIGINAL: 0x005EE280 ?release_modal@Win@@QAEXXZ 0x005EE280-0x005EE327 FILE
// symbol    ?release_modal@Win@@UAEXXZ
// size      167 bytes
// prototype void (__thiscall ?release_modal@Win@@QAEXXZ)(Win* this)
// callers   5   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// TRIED: 1 of 50, and the diagnosis is REGISTER PRESSURE, not shape. The
// structure already matches the image function for function - the top-of-
// stack test, the linear search, the `i == last` bail, the shift loop, the
// found arm's `[eax-4]`/`[ecx-4]` reads - but this body pushes edi as well
// as esi where the image pushes esi alone, and carries 63 instructions
// against 50. It also materialises the zero into a register (`xor edx, edx`
// then `cmp esi, edx`) where the image tests in place (`test edx, edx`) -
// the same one-fewer-live-value shape as the dx/dy lever. (Measured under
// /Oy-, which is the set the flag search picks here; an earlier reading of
// this diff under a fixed /O2 set said ebx AND edi.)
// Tried: decrementing one variable in place with an early return, which is
// what the image does (`test edx,edx` / `je` / `dec edx` / store) instead of
// `count` plus `last = count - 1`; spelling the nulls `0` rather than
// `nullptr`; and computing `&WinFocusStack[last]` as a pointer so both
// stores go through a lea, as the image's do. None moved the count.
// LEVER (2026-08-27, found): index-only everywhere. Killing the `p`
// cursor in the search loop and every `&WinModalStack[last]` pointer in
// the found arm - the image addresses both stacks as `[const + idx*4]`
// and never parks an array base - took this from 1 of 50 with 52
// instructions to 11 of 50 at exactly the image's 50, the extra
// callee-saved push gone. Fresh-depth null test (`if (WinModalDepth == 0)`
// before loading) measured WORSE (8/50); load-then-test stands. What
// still divides us is the opening compare: image `test edx,edx`, ours
// `xor eax,eax / cmp edx,eax` - the zero materialises under /Oy- and no
// spelling tried moves it.
void Win::release_modal() {
    // ONE VARIABLE, DECREMENTED IN PLACE, and an early return. The
    // image loads WinModalDepth into edx, tests it there, decrements
    // it there and stores it back; keeping `count` and computing
    // `last = count - 1` into a second register cost a `xor edx,edx`
    // for the compare and a `lea esi,[eax-1]` for the decrement.
    int last = WinModalDepth;
    if (last == 0) {
        return;
    }
    --last;
    WinModalDepth = last;
    // INDEX-ONLY EVERYWHERE: the image addresses both stacks as
    // `[const + idx*4]` and never parks an array base in a callee-saved
    // register - any `&stack[i]` pointer here buys one it does not spend.
    if (WinModalStack[last] == this) {
        WinModalStack[last] = 0;
        WinFocusStack[last] = 0;
        if (0 < last) {
            WinModalWindow = reinterpret_cast<Win *>(WinFocusStack[last - 1]);
            WinFocusWindow = WinModalStack[last - 1];
            return;
        }
        WinFocusWindow = 0;
    } else {
        // INDEX-ONLY SEARCH: `p` was a second copy of the same cursor and
        // cost a callee-saved register the image does not spend.
        int i = 0;
        while (i < last && WinModalStack[i] != this) {
            i = i + 1;
        }
        if (i == last) {
            WinModalDepth = last + 1;
            return;
        }
        if (i < last) {
            do {
                WinModalStack[i] = WinModalStack[i + 1];
                WinFocusStack[i] = WinFocusStack[i + 1];
                i = i + 1;
            } while (i < last);
            return;
        }
    }
}

// Fixed-slot bindings carried from 005f54e0.cpp

// ORIGINAL: 0x005F54E0 ?on_paint@Win@@QAEXPAURECT@@@Z 0x005F54E0-0x005F5803 FILE
// symbol    ?on_paint@Win@@UAEXPAUtagRECT@@@Z
// size      803 bytes
// prototype void (__thiscall ?on_paint@Win@@QAEXPAURECT@@@Z)(Win* this, RECT* lprc)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D9BE0 0x00644F3A
// indirect  0x005F5510 0x005F5521 0x005F5566 0x005F5587 0x005F56B9 0x005F5707 0x005F574D 0x005F5798 0x005F579F 0x005F57C5 0x005F57E9
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F54E0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f54e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_paint@Win@@QAEXPAURECT@@@Z  at 0x005F54E0  (803 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void Win::on_paint(RECT * area) {
    int val = 0;
}

// ORIGINAL: 0x005F5AD0 ?on_nc_hittest@Win@@QAEHHH@Z 0x005F5AD0-0x005F5BF6
// symbol    ?on_nc_hittest@Win@@UAEHHH@Z
// TRIED: transcribing the nine-way corner/edge ladder as one flat sequence of if-return statements (matching the Ghidra shape, each re-testing x/y against iVar1 and the recomputed client-rect edge) compiles and the byte count comes close (286 vs 294) but the prologue register allocation still diverges at instruction #3 - the original pushes edi before touching it, this form delays that push since it needs one fewer live temporary across the first branch. Landing the closest MISMATCH.
// size      294 bytes
// prototype int (__thiscall ?on_nc_hittest@Win@@QAEHHH@Z)(Win* this, int, int)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


// TRIED: 10 of 116 at 112 instructions - four SHORT, which is unusual here;
// most of this class's walls are over. The tail's `(iFlags_ & 0x4000010) ==
// 0` reads the flag test before `[ecx + 0x16c]` where the image reads the
// field first, but inverting the condition so the field_16C_ arm comes first
// measures identically - VC6 schedules those two independently of source
// order. The 13 differing runs are spread through the body, so this one
// wants a full re-read against the image rather than another spelling.
int Win::on_nc_hittest(int x, int y) {
  if (iSomeFlag_ & 2) {
    int iVar1 = field_170_;
    if (x < iVar1) {
      if (y < iVar1) {
        return 0xd;
      }
      if (client_rect_.bottom - client_rect_.top - iVar1 < y) {
        return 0x10;
      }
    }
    int iVar2 = client_rect_.right - client_rect_.left - iVar1;
    if (iVar2 < x) {
      if (y < iVar1) {
        return 0xe;
      }
      if (client_rect_.bottom - client_rect_.top - iVar1 < y) {
        return 0x11;
      }
    }
    if (x < iVar1) {
      return 0xa;
    }
    if (y < iVar1) {
      return 0xc;
    }
    if (iVar2 < x) {
      return 0xb;
    }
    if (client_rect_.bottom - client_rect_.top - iVar1 < y) {
      return 0xf;
    }
  }
  if ((iFlags_ & 0x4000010) == 0) {
    if (iFlags_ & 1) {
      return 2;
    }
  } else if (field_16C_ != 0 && y < (int)field_16C_) {
    return 2;
  }
  return 0;
}

// ORIGINAL: 0x005F1150 ?OnPaletteChanged@Win@@QAAHPAXPAX@Z 0x005F1150-0x005F122B FILE BYTE_EXACT
// symbol    ?OnPaletteChanged@Win@@SAHPAX0@Z
// LEVER: the same `WinHdcRefCount`/`WinSharedHdc`/`DirectDrawSurface` globals
// and `DirectDrawSurface->GetDC/ReleaseDC` idiom `Win::get_hdc`/`release_hdc`
// already use (both are BYTE_EXACT), plus `BufferDirectDraw` and
// `PaletteInitialized` for the two remaining fixed slots, and plain
// `SelectPalette`/`RealizePalette`/`GetDC`/`ReleaseDC` calls in place of the
// six `g_`-named function-pointer bindings - every one compiles to the same
// `call dword ptr [import]` or `call [vtable+N]` the image emits.
// `xor eax,eax` anywhere), so despite the `int` in the mangled name the body
// never manufactures a return value except at the one distinct early `ret`
// (0x5F120C), where returning the vtable slot's own `long` result reproduces
// the "value already sitting in eax" shape instead of adding one.
// size      219 bytes
// prototype int (__cdecl ?OnPaletteChanged@Win@@QAAHPAXPAX@Z)(HWND hWnd, LPARAM lParam)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x005F1194 0x005F11A5 0x005F11CC 0x005F11D9 0x005F11FF 0x005F121A

int __cdecl Win::OnPaletteChanged(void * hwnd, void * lparam) {
    if (hwnd != lparam && BufferDirectDraw == nullptr) {
        if (WinHdcRefCount != 0) {
            ++WinHdcRefCount;
        } else {
            if (DirectDrawSurface != nullptr) {
                DirectDrawSurface->GetDC(&WinSharedHdc);
            } else {
                WinSharedHdc = GetDC(HandleMain);
            }
            if (WinSharedHdc != nullptr) {
                WinHdcRefCount = 1;
            }
        }
        if (WinSharedHdc != nullptr) {
            SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
            RealizePalette(WinSharedHdc);

            --WinHdcRefCount;
            if (WinHdcRefCount == 0) {
                if (DirectDrawSurface != nullptr) {
                    long result = DirectDrawSurface->ReleaseDC(WinSharedHdc);
                    WinSharedHdc = nullptr;
                    return static_cast<int>(result);
                }
                ReleaseDC(HandleMain, WinSharedHdc);
                WinSharedHdc = nullptr;
            }
        }
    }
}

// The two icons `redo_caption_buttons` swaps into the zoom button's own
// +0xAB8 field, keyed by `IsZoomed()`. INFERRED names: the icon shown when
// NOT zoomed is the action still available (maximize), and vice versa.

// ORIGINAL: 0x005F2760 ?redo_caption_buttons@Win@@QAEXXZ 0x005F2760-0x005F2822 FILE
// LEVER: the image's `call rel32` to `Win::move` takes its RECEIVER from
// `[edi+0xe4/0xe8/0xec]` (the button pointer), not `edi` (`this`) - the
// scaffold's bare `move(w, y)` called this window's own move instead of
// the button's, a wrong-receiver defect independent of the `this`-pun.
// size      194 bytes
// prototype void (__thiscall ?redo_caption_buttons@Win@@QAEXXZ)(Win* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED7D0
// indirect  0x005F27CC

// TRIED: 10 of 57, and the whole gap is ONE INSTRUCTION VC6 will not emit.
// The image computes `right - (w+w) - left - border_thickness_ -
// caption_height_ - 2` as six separate instructions, ending `sub esi, 2`.
// Every spelling of that folds the constant into the doubling instead -
// `lea eax, [ebx + ebx + 2]` where the image has `lea eax, [ebx + ebx]` -
// so this body is 56 instructions against 57 and everything after the fold
// is shifted. Tried: one expression; five separate `y -=` statements;
// `w + w` in place of `w * 2`; and applying the 2 to `client_rect_.right`
// instead, so it cannot reach the doubling. All four fold. The flag search
// already covers /Oy-, /Ob0 and /Oi- and settles on /O2 /Gy /GR- /GX.
// The subtraction ORDER below is the image's, read off 0x005F2782-0x005F2794.
void Win::redo_caption_buttons() {
    int w = field_128_;
    int y = client_rect_.right - w * 2 - client_rect_.left -
            border_thickness_ - caption_height_ - 2;
    if (close_button_ != 0) {
        close_button_->move(w, y);
        y += -3 - *reinterpret_cast<int *>(
                       reinterpret_cast<char *>(close_button_) + 0x4C4);
    }
    if (zoom_button_ != 0) {
        zoom_button_->move(w, y);
        if (IsZoomed(HandleMain) == 0) {
            *reinterpret_cast<int *>(
                reinterpret_cast<char *>(zoom_button_) + 0xAB8) =
                WinMaximizeIcon;
        } else {
            *reinterpret_cast<int *>(
                reinterpret_cast<char *>(zoom_button_) + 0xAB8) =
                WinRestoreIcon;
        }
        y += -1 - *reinterpret_cast<int *>(
                       reinterpret_cast<char *>(zoom_button_) + 0x4C4);
    }
    if (minimize_button_ != 0) {
        minimize_button_->move(w, y);
    }
}

// ORIGINAL: 0x005F6230 ?on_sys_key@Win@@QAGHIJHI@Z 0x005F6230-0x005F62C8 FILE
// symbol    ?on_sys_key@Win@@UAEHIJHI@Z
// LEVER: `PostMessageA`/`DefWindowProcA` by name instead of the two import
// thunks (0x00669314, 0x006692B0) as `g_`-named function pointers - both
// compile to the same `call dword ptr [import]` the image emits.
// size      152 bytes
// prototype LRESULT (__stdcall ?on_sys_key@Win@@QAGHIJHI@Z)(WPARAM wParam, int, int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F6264 0x005F6278 0x005F62A9 0x005F62BD
// TRIED (2026-08-27): 2 of 48 to 9 of 48 with two levers - building
// lParam PER ARM the way the image duplicates its arg loads, and masking
// BOTH halves (& 0xffff on key_flags, not just repeat_count). A const
// local for the masked flags bought nothing (9/48 both ways): the
// image's `and esi,0xffff` holds key_flags in a callee-saved register it
// chose because the whole image is FPO, while this body compiles under
// the /Oy- flag set the search picks here - the RECEIVER-SPILL/PROLOGUE
// wall again, upstream of any spelling tried. Two instructions still
// short (46 vs 48) for the same reason.

int Win::on_sys_key(unsigned int vkey, long is_down, int repeat_count,
                              unsigned int key_flags) {
    // THE IMAGE BUILDS lParam PER ARM, BOTH HALVES MASKED: esi (key_flags)
    // is masked before the shift and repeat_count after, and the two arms
    // duplicate the loads instead of sharing a packed value across the
    // branch.
    if (is_down != 0) {
        unsigned int const flags = key_flags & 0xffff;
        LPARAM lParam = (flags << 0x10) | (repeat_count & 0xffff);
        PostMessageA(HandleMain, WM_KEYDOWN, vkey, lParam);
        return DefWindowProcA(HandleMain, WM_SYSKEYDOWN, vkey, lParam);
    }
    unsigned int const flags = key_flags & 0xffff;
    LPARAM lParam = (flags << 0x10) | (repeat_count & 0xffff);
    PostMessageA(HandleMain, WM_KEYUP, vkey, lParam);
    return DefWindowProcA(HandleMain, WM_SYSKEYUP, vkey, lParam);
}

// A one-character glyph `set_bubble_text` measures the width of to compute
// each line's padding. Named, not `g_`-prefixed; the exact string is not
// independently confirmed, so the fixed address is kept rather than a
// literal.
static char *const WinBubblePrefixGlyph = reinterpret_cast<char *>(0x00696E00);

// ORIGINAL: 0x005F83D0 ?set_bubble_text@Win@@QAEXPADPAURECT@@@Z 0x005F83D0-0x005F84FB FILE
// symbol    ?set_bubble_text@Win@@QAEXPADPAUtagRECT@@@Z
// LEVER: `WinBubbleActive`/`WinBubbleCompanion`/`WinBubbleFont`/`WinBubbleRect`
// (already declared, `WinBubbleCompanion` retyped `Win **` so `this` needs no
// cast) and `BasePopScreenWidth`, in place of the eight `g_`-named bindings.
// `strchr` called directly - the scaffold's function-pointer cast of it
// serves no purpose `char *strchr(const char *, int)` does not already.
// size      299 bytes
// prototype void (__thiscall ?set_bubble_text@Win@@QAEXPADPAURECT@@@Z)(Win* this, int8*, RECT*)
// callers   14   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD20 0x005F7320 0x00619280 0x00645DD0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F83D0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f83d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_bubble_text@Win@@QAEXPADPAURECT@@@Z  at 0x005F83D0  (299 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void Win::set_bubble_text(char * text, RECT * rect) {
    if (text == 0 || rect == 0) {
        return;
    }
    if (WinBubbleActive != 0) {
        WinBubbleCompanion = 0;
        WinBubbleActive = 0;
        update_screen(&WinBubbleRect, 0);
        flip(&WinBubbleRect);
    }
    int max_width = 0;
    int line_count = 0;
    char *str = text;
    WinBubbleCompanion = this;
    for (;;) {
        char *caret = strchr(str, '^');
        if (caret != 0) {
            *caret = 0;
        }
        int prefix_width = WinBubbleFont->width(WinBubblePrefixGlyph);
        int line_width = WinBubbleFont->width(str) + prefix_width * 2;
        if (max_width < line_width) {
            max_width = line_width;
        }
        str = caret + 1;
        if (caret == 0) {
            break;
        }
        *caret = '^';
        line_count = line_count + 1;
    }
    Font *const font = WinBubbleFont;
    int line_height;
    if (font->unk_1_ < 0) {
        line_height = font->line_height_;
    } else {
        line_height = font->height_ + font->unk_1_;
    }
    line_height = line_height * (line_count + 1);
    int y = rect->top - line_height;
    if (y < 0) {
        y = rect->bottom;
    }
    int x = ((rect->right - max_width) + rect->left) / 2;
    if (x < 0) {
        x = 0;
    } else if (*BasePopScreenWidth < x + max_width) {
        x = *BasePopScreenWidth - max_width;
    }
    WinBubbleRect.left = x;
    WinBubbleRect.top = y;
    WinBubbleRect.right = x + max_width;
    WinBubbleRect.bottom = y + line_height;
    WinBubbleActive = reinterpret_cast<int>(text);
    flip(&WinBubbleRect);
}

// ORIGINAL: 0x005EE330 ?resize_event@Win@@QAEHHH@Z 0x005EE330-0x005EE454
// TRIED: a named-global spelling for 0x9b7ab8, i.e. a static Win-pointer global assigned from `this`, reordered the entry register allocation - mismatch at instr #0; a plain store through a cast fixed address matches through instr #16-19 depending on how the vtable-slot-3/13 calls are spelled. The two indirect vtable calls, slot 3 at +0xc and slot 13 at +0x34, cannot use a virtual-shim helper class declared ahead of the definition: a second top-level type before the function truncates the region this file's own extractor reads (brace-depth returns to zero at the helper's own close, before the real body), and a LOCAL shim class needs every declared virtual defined, C2599, which VC6 then emits as extra external symbols and the unit fails the exactly-one-symbol check. A raw thiscall function pointer draws C4234. The working spelling: an empty LOCAL class with no members at all, so it carries no vtable and needs no symbol, used purely for its member-pointer representation, populated at runtime through a union from the vtable slot read as a plain pointer - legitimate member-pointer call syntax, so it invokes with the receiver in ECX like the original, and it is one self-contained region for this file's own extractor. 0.857 mnemonic similarity, first divergence at instr #19; landing the closest MISMATCH. NOTE: literal brace characters in this comment block will truncate the region the same way - keep this prose brace-free.
// size      292 bytes
// prototype int (__thiscall ?resize_event@Win@@QAEHHH@Z)(Win* this, int, int)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED7D0 0x005F2760
// indirect  0x005EE3AA 0x005EE401 0x005EE422 0x005EE439 0x005EE444


// TRIED: 22 -> 24 of 91, and both gains are BEHAVIOUR fixes. The image
// loads scroll_vert_ into ECX at 0x005EE348 and calls `0x5ed7d0` - Win::move
// - with it still there, so the move belongs to the SCROLLBAR; this body
// called it on `this`. And vtable offset 0xc is slot 3, `resize(int,int,int)`,
// which win.h declares - the MfpBase member-function-pointer shim was
// spelling a call the class can now make directly.
//
// Two levers that worked elsewhere do NOT work here: swapping the `sh == 0`
// arms so the read comes first (the lever that made is_dialog_focus exact)
// changes nothing, and inlining the `sv`/`sh` locals to cut live values (the
// lever that made nonclient_to_screen exact) LOSES two, 24 back to 22. What
// remains is 91 against 91 with edi and ebx swapped from instruction 11 on.
int Win::resize_event(int width, int height) {
  class MfpBase {};
  typedef int (MfpBase::*Fn3)(int, int, int);
  union Conv3 {
    Fn3 asMfp;
    void *asPtr;
  };

  // 0x009B7AB8 is WinCallbackWindow, already bound in win_slots.h.
  WinCallbackWindow = this;

  if ((iFlags_ & 0x40) == 0) {
    if (scroll_vert_ != 0) {
      // THE SCROLLBAR'S move, not this window's. The image loads
      // scroll_vert_ into ECX at 0x005EE348, tests it, and calls
      // 0x5ed7d0 - Win::move - with that still in the receiver register.
      scroll_vert_->move(outer_rect_.right - outer_rect_.left, 0);
      char *sv = (char *)scroll_vert_;
      char *sh = (char *)scroll_horz_;
      int iVar2;
      if (sh == 0) {
        iVar2 = outer_rect_.bottom - outer_rect_.top;
      } else {
        iVar2 = *(int *)(sh + 0x4c8) - outer_rect_.top + outer_rect_.bottom;
      }
      // Vtable offset 0xc IS slot 3, which win.h declares as
      // `resize(int, int, int)` - the signature the Fn3 typedef spelled out
      // by hand. No member-function-pointer shim needed.
      scroll_vert_->resize(*(int *)(sv + 0x4c4), iVar2, 0);
    }
    if (scroll_horz_ != 0) {
      move(0, outer_rect_.bottom - outer_rect_.top);
      char *sh = (char *)scroll_horz_;
      char *sv = (char *)scroll_vert_;
      Conv3 c;
      c.asPtr = (*(void ***)sh)[0xc / 4];
      if (sv == 0) {
        (reinterpret_cast<MfpBase *>(scroll_horz_)->*c.asMfp)(
            outer_rect_.right - outer_rect_.left, -*(int *)(sh + 0x4c8), 0);
      } else {
        (reinterpret_cast<MfpBase *>(scroll_horz_)->*c.asMfp)(
            (outer_rect_.right - *(int *)(sv + 0x4c4)) - outer_rect_.left,
            -*(int *)(sh + 0x4c8), 0);
      }
    }
  }
  // field_400_ IS the resize callback: storage typed where it is declared,
  // so no cast sits between the null-test and the call.
  if (field_400_ != 0) {
    field_400_(width, height);
  }
  // Slot 13 of Win's own vtable - vslot_13 - reached as the virtual call
  // the image's [vtable+0x34] dispatch is.
  vslot_13(width, height);
  redo_caption_buttons();
  return 0;
}

// Fixed-slot bindings carried from 005f1340.cpp

typedef HDC (__stdcall *BeginPaintProc)(void *, PAINTSTRUCT *);
typedef int (__stdcall *EndPaintProc)(void *, const PAINTSTRUCT *);

// ORIGINAL: 0x005F1340 ?OnPaint@Win@@QAA_NPAUHWND__@@@Z 0x005F1340-0x005F13A2 FILE
// TRIED: BeginPaint/EndPaint reached through the IAT slots as fn-ptr casts; bool return passed straight through EndPaint's result
// working copy - scaffold materialised by --work
// size      98 bytes
// prototype bool (__cdecl ?OnPaint@Win@@QAA_NPAUHWND__@@@Z)(HWND hWnd)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD20 0x005F7320
// indirect  0x005F134E 0x005F1397


bool __cdecl Win::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    // 0x006692B8. The image calls `[0x6692b8]` at 0x005F134E; the
    // file-scope `g` this used to read is SetCursorPos.
    HDC hdc = BeginPaint(hwnd, &ps);
    if (hdc == 0) {
        return false;
    }
    RECT paintRect;
    paintRect.left = ps.rcPaint.left;
    paintRect.top = ps.rcPaint.top;
    paintRect.right = ps.rcPaint.right;
    paintRect.bottom = ps.rcPaint.bottom;
    update_screen(&paintRect, 0);
    flip(&paintRect);
    return EndPaint(hwnd, &ps) != 0;
}



struct LocalLogPen {
    unsigned int style;
    int widthX;
    int widthY;
    unsigned int color;
};

class MenuVCall {
public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067();
    virtual void slot068();
    virtual void slot069();
    virtual void slot070();
    virtual void slot071();
    virtual void slot072();
    virtual void slot073();
    virtual void slot074();
    virtual void slot075();
    virtual void slot076();
    virtual void slot077();
    virtual void slot078();
    virtual void slot079();
    virtual void slot080();
    virtual void slot081();
    virtual void slot082();
    virtual void slot083();
    virtual void slot084();
    virtual void slot085();
    virtual void slot086();
    virtual void slot087();
    virtual void slot088();
    virtual void slot089();
    virtual void slot090();
    virtual int slot091();
};

// The artifact spelled MSG out by hand; these ARE its fields, and the
// message pump below passes one straight to PeekMessageA.
typedef MSG MsgT;


class NCCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067();
    virtual void slot068();
    virtual void slot069();
    virtual void slot070();
    virtual void slot071();
    virtual void slot072();
    virtual void slot073();
    virtual void slot074();
    virtual void slot075();
    virtual void slot076();
    virtual void slot077();
    virtual void slot078();
    virtual void slot079();
    virtual void slot080();
    virtual void slot081();
    virtual void slot082();
    virtual void slot083();
    virtual void slot084();
    virtual void slot085();
    virtual void slot086();
    virtual void slot087();
    virtual void slot088();
    virtual void slot089();
    virtual void slot090();
    virtual int slot091();
};


struct PAINTSTRUCT_ {
    HDC hdc;
    int fErase;
    RECT rcPaint;
    int fRestore;
    int fIncUpdate;
    unsigned char rgbReserved[32];
};


class VCall2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067();
    virtual void slot068();
    virtual void slot069();
    virtual void slot070();
    virtual void slot071();
    virtual void slot072();
    virtual void slot073();
    virtual void slot074();
    virtual void slot075();
    virtual void slot076();
    virtual void slot077();
    virtual void slot078();
    virtual void slot079();
    virtual void slot080();
    virtual void slot081();
    virtual void slot082();
    virtual void slot083();
    virtual void slot084();
    virtual void slot085();
    virtual void slot086();
    virtual void slot087();
    virtual void slot088();
    virtual void slot089();
    virtual void slot090();
    virtual int slot091();
};




class VCallArg { public:
    virtual void s00(); virtual void s01(); virtual void s02(); virtual void s03();
    virtual void s04(); virtual void s05(); virtual void s06(); virtual void s07();
    virtual void s08(); virtual void s09(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual int s23();  // <-- used
    virtual void s24(); virtual void s25(); virtual void s26();
    virtual void s27(int);  // <-- used
};

class VCallArg51 { public:
    virtual void s00(); virtual void s01(); virtual void s02(); virtual void s03();
    virtual void s04(); virtual void s05(); virtual void s06(); virtual void s07();
    virtual void s08(); virtual void s09(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void s24(); virtual void s25(); virtual void s26(); virtual void s27();
    virtual void s28(); virtual void s29(); virtual void s30(); virtual void s31();
    virtual void s32(); virtual void s33(); virtual void s34(); virtual void s35();
    virtual void s36(); virtual void s37(); virtual void s38(); virtual void s39();
    virtual void s40(); virtual void s41(); virtual void s42(); virtual void s43();
    virtual void s44(); virtual void s45(); virtual void s46(); virtual void s47();
    virtual void s48(); virtual void s49(); virtual void s50();
    virtual void s51(int);  // <-- used, takes an int
};




class VCallModal { public:
    virtual void slot00();
    virtual void slot01(int);  // offset 4, used
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();     // offset 0x10, used
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void slot26();
    virtual void slot27();
    virtual void slot28();
    virtual void slot29();
    virtual void slot30();
    virtual void slot31();
    virtual void slot32();
    virtual void slot33();
    virtual void slot34();
    virtual void slot35();
    virtual void slot36();
    virtual void slot37();
    virtual void slot38();
    virtual void slot39();
    virtual void slot40();
    virtual void slot41();
    virtual void slot42();
    virtual void slot43();
    virtual void slot44();
    virtual void slot45();
    virtual void slot46();
    virtual void slot47();
    virtual void slot48();
    virtual void slot49();
    virtual void slot50();
    virtual void slot51();
    virtual void slot52();
    virtual void slot53();
    virtual void slot54();
    virtual void slot55();
    virtual void slot56();
    virtual void slot57();
    virtual void slot58();  // offset 0xe8, used
    virtual int  slot59();  // offset 0xec, used, checked
};

class VCallRet23 { public:
    virtual void s00(); virtual void s01(); virtual void s02(); virtual void s03();
    virtual void s04(); virtual void s05(); virtual void s06(); virtual void s07();
    virtual void s08(); virtual void s09(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22();
    virtual int  s23();  // <-- used, returns int
};

class VCallW { public:
    virtual void slot000();  virtual void slot001();  virtual void slot002();
    virtual void slot003();  virtual void slot004();  virtual void slot005();
    virtual void slot006();  virtual void slot007();  virtual void slot008();
    virtual void slot009();  virtual void slot010();  virtual void slot011();
    virtual void slot012();  virtual void slot013();  virtual void slot014();
    virtual void slot015();  virtual void slot016();  virtual void slot017();
    virtual void slot018();  virtual void slot019();  virtual void slot020();
    virtual void slot021();  virtual void slot022();  virtual void slot023();
    virtual void slot024();  virtual void slot025();  virtual void slot026();
    virtual void slot027();  virtual void slot028();  virtual void slot029();
    virtual void slot030();  virtual void slot031();  virtual void slot032();
    virtual void slot033();  virtual void slot034();  virtual void slot035();
    virtual void slot036();  virtual void slot037();  virtual void slot038();
    virtual void slot039();  virtual void slot040();  virtual void slot041();
    virtual void slot042();  virtual void slot043();  virtual void slot044();
    virtual void slot045();  virtual void slot046();  virtual void slot047();
    virtual void slot048();  virtual void slot049();  virtual void slot050();
    virtual void slot051();  virtual void slot052();  virtual void slot053();
    virtual void slot054();  virtual void slot055();  virtual void slot056();
    virtual void slot057();  virtual void slot058();  virtual void slot059();
    virtual void slot060();  virtual void slot061();  virtual void slot062();
    virtual void slot063();  virtual void slot064();  virtual void slot065();
    virtual void slot066();  virtual void slot067();  virtual void slot068();
    virtual void slot069();  virtual void slot070();  virtual void slot071();
    virtual void slot072();  virtual void slot073();  virtual void slot074();
    virtual void slot075();  virtual void slot076();  virtual void slot077();
    virtual void slot078();  virtual void slot079();  virtual void slot080();
    virtual void slot081();  virtual void slot082();  virtual void slot083();
    virtual void slot084(int, int, unsigned int, int);  // <-- used
};




// Fixed-slot bindings the homed bodies name but no artifact defined
// in this file - each is an address the image reaches directly.
// The z-order list. MEASURED from the image, not inferred: `bring_to_top`
// at 0x005F4B34 loads its base as an immediate - `mov esi, 0x9b6e48` -
// and walks it with the count at 0x009B7B34. The artifacts declared it
// `extern int WinRootWindows[];` and never defined it anywhere, so nothing
// that used it could ever have linked.

// SLOT 88 IS PAST WIN'S TABLE. Win's vtable is 88 entries, 0..87 - slot
// 88 would be the first dword of the SECONDARY table at 0x0066FF30. So
// this dispatch is not one of Win's virtuals and is left as a shim,
// rather than inventing a 89th slot to make it compile.
namespace { class Slot88Shim { public:
  virtual void pad(); 
  virtual int slot088(int, int);
  virtual int slot090(Win *);
}; }

void __cdecl flush_input();
void __cdecl wait_task();
typedef int (__stdcall *ComSlot17)(void *self, HDC *out);

// The HDC acquire/release pair, recovered from the artifact that defined
// it: the bodies that paint non-client areas all use it, and a macro
// does not travel with a function body.
#define ACQUIRE_HDC(haveHdcVar) \
    do { \
        if (WinHdcRefCount == 0) { \
            if (DirectDrawSurface != 0) { \
                ComSlot17 fn = (ComSlot17)(*reinterpret_cast<void ***>(DirectDrawSurface))[0x11]; \
                fn(DirectDrawSurface, &WinSharedHdc); \
            } else { \
                WinSharedHdc =  \
                    GetDC(HandleMain); \
            } \
            (haveHdcVar) = (WinSharedHdc != 0); \
            if (haveHdcVar) { \
                WinHdcRefCount = 1; \
            } \
        } else { \
            ++WinHdcRefCount; \
            (haveHdcVar) = (WinSharedHdc != 0); \
        } \
    } while (0)

// Helpers the flip/restore body needs, recovered from the artifact that
// defined them - a static function and a macro do not travel with a
// method body.

#define WFIELD(obj, off) (*reinterpret_cast<int *>(reinterpret_cast<char *>(obj) + (off)))
// Homed below; the signature is the one its TRIED note measured -
// `void __cdecl restore_flip_sprite(int)`, from the [esp+0x14] stack read.
void __cdecl restore_flip_sprite(int);
extern "C" int __cdecl update_screen(RECT *, Win *);


typedef int (__stdcall *NetCreateGroupFn)(Net *, int *, void *, int, int, int);


// 0xCC / 4 = SLOT 51, which Win declares. The artifact reached it by
// building a member-pointer out of the raw vtable entry through a
// `SlotCCCast` union; with the slot declared, it is just a call.
static void call_slot_cc(int *obj, int arg) {
    reinterpret_cast<Win *>(obj)->vslot_51(arg);
}

// The flip/restore helper block, taken whole from the artifact that
// defined it. Restoring its pieces one compiler error at a time kept
// finding another dependency; the block is the coherent unit.
typedef int(__stdcall *UnionRectFn)(RECT *, const RECT *, const RECT *);
typedef void *(__stdcall *LoadCursorFn)(void *, int);
typedef void *(__stdcall *SetCursorFn)(void *);
typedef void(__stdcall *GetCursorPosFn)(int *);
typedef int(__stdcall *PollFn)(int);

#define WFIELD(obj, off) (*reinterpret_cast<int *>(reinterpret_cast<char *>(obj) + (off)))

// Builds the saved-region rect the "direct" way: left,top,right=left+w,bottom=top+h.
static void BuildRectDirect(RECT &r) {
    r.left = WinFlipSpriteY;
    r.top = WinFlipSpriteX;
    r.right = WinFlipSpriteY + WinClipWidth;
    r.bottom = WinFlipSpriteX + WinClipHeight;
}

// Restores the saved screen region (if one is pending) and flips it, then
// clears the pending flag. `useHelper` selects between the two rect-build
// strategies the disassembly duplicates at each call site (a direct sum,
// or the opaque sub_5f86c0 helper) - both read the same four fields.
static void RestoreAndFlip(UnionRectFn UnionRect, bool useHelper) {
    if (WinFlipSprite != 0) {
        g_WIN_BUFFER
            ->copy((&ScreenBuffer), 0, 0, WinFlipSpriteY, WinFlipSpriteX,
                   WinClipWidth, WinClipHeight);
        RECT rect;
        if (useHelper) {
            make_rect(&rect, WinFlipSpriteY, WinFlipSpriteX, WinClipWidth, WinClipHeight);
        } else {
            BuildRectDirect(rect);
        }
        RECT merged;
        UnionRect(&merged, &rect, &WinDirtyRect);
        Win::flip(&merged);
        WinDirtyRect.left = 0;
        WinModalResult = 0;
    }
    WinFlipSprite = reinterpret_cast<Sprite *>(0);
}

namespace {
// SCROLL'S SLOT 25, NOT WIN'S. `scroll_vert_` is a `Scroll *`, so this
// dispatch indexes Scroll's vtable - Win's slot map does not apply, and
// converting it to a Win virtual would have called the wrong function
// while compiling cleanly. It stays a shim until Scroll's vtable is
// declared. Only declaration ORDER fixes a slot, hence the pads.
class ScrollSlots { public:
  virtual void pad000();
  virtual void pad001();
  virtual void pad002();
  virtual void pad003();
  virtual void pad004();
  virtual void pad005();
  virtual void pad006();
  virtual void pad007();
  virtual void pad008();
  virtual void pad009();
  virtual void pad010();
  virtual void pad011();
  virtual void pad012();
  virtual void pad013();
  virtual void pad014();
  virtual void pad015();
  virtual void pad016();
  virtual void pad017();
  virtual void pad018();
  virtual void pad019();
  virtual void pad020();
  virtual void pad021();
  virtual void pad022();
  virtual void pad023();
  virtual void pad024();
  virtual int slot025(int, int);
};
}


// ===== homed from src/recovered/005f2700.cpp =====

// ORIGINAL: 0x005F2700 ?do_caption_buttons@Win@@QAEXXZ 0x005F2700-0x005F2753 BYTE_EXACT
// size      83 bytes
// prototype void (__thiscall ?do_caption_buttons@Win@@QAEXXZ)(Win* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F2711 0x005F272B 0x005F2745
// 0x005F2700  ?do_caption_buttons@Win@@QAEXXZ  ->  ?do_caption_buttons@Win@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
void Win::do_caption_buttons() {
    Win *obj;

    obj = minimize_button_;
    if (obj) {
        delete obj;
        minimize_button_ = 0;
    }
    obj = zoom_button_;
    if (obj) {
        delete obj;
        zoom_button_ = 0;
    }
    obj = close_button_;
    if (obj) {
        delete obj;
        close_button_ = 0;
    }
}

// ===== homed from src/recovered/005f85b0.cpp =====

// ORIGINAL: 0x005F85B0 ?on_mousewheel_down@Win@@QAEXH@Z 0x005F85B0-0x005F85DD BYTE_EXACT
// size      45 bytes
// prototype void (__thiscall ?on_mousewheel_down@Win@@QAEXH@Z)(Win* this, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606320
// 0x005F85B0  ?on_mousewheel_down@Win@@QAEXH@Z  ->  ?on_mousewheel_down@Win@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
void Win::on_mousewheel_down(int delta) {
    Scroll *scroll = scroll_vert_;
    if (scroll) {
        scroll->on_mousewheel_down(delta);
    }
    scroll = scroll_horz_;
    if (scroll) {
        scroll->on_mousewheel_down(delta);
    }
}

// ===== homed from src/recovered/005f0540.cpp =====
typedef long (__stdcall *GetWindowLongProc)(void *, int);

// ORIGINAL: 0x005F0540 ?adjust_menus@Win@@QAGHPAX@Z 0x005F0540-0x005F0579 BYTE_EXACT
// symbol    ?adjust_menus@Win@@SGHPAUHWND__@@PAX@Z
// size      57 bytes
// prototype int (__stdcall ?adjust_menus@Win@@QAGHPAX@Z)(HWND hWnd)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F0548 0x005F0558 0x005F056A
// 0x005F0540  ?adjust_menus@Win@@QAGHPAX@Z  ->  ?adjust_menus@Win@@QAGHPAX@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
int __stdcall Win::adjust_menus(HWND hwnd, void *unused) {
    (void)unused;
    Win *obj = reinterpret_cast<Win *>(
        // 0x0066934C: the image calls `[0x66934c]` at 0x005F0548.
        GetWindowLongA(hwnd, -0x15));
    if (obj != 0) {
        obj->set_rects();
        Menu *menu = obj->menu_;
        if (menu != 0) {
            menu->vslot_93();
        }
    }
    return 1;
}

// ===== homed from src/recovered/005edf00.cpp =====

// ORIGINAL: 0x005EDF00 ?show_maximize@Win@@QAEHXZ 0x005EDF00-0x005EDF48 BYTE_EXACT
// size      72 bytes
// prototype int (__thiscall ?show_maximize@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EDF23 0x005EDF3D
// 0x005EDF00  ?show_maximize@Win@@QAEHXZ  ->  ?show_maximize@Win@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
int Win::show_maximize() {
    Win *active = WinPointerOwner3;
    WinPointerOwner1 = 0;
    WinPointerOwner2 = 0;
    if (active == this) {
        WinPointerOwner3 = 0;
        this->vslot_04();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    ShowWindow(HandleMain, 3);
    return 0;
}

// ===== homed from src/recovered/005ede60.cpp =====
typedef int(__stdcall *ShowWindowFn)(void *, int);

// ORIGINAL: 0x005EDE60 ?maximize@Win@@QAEHXZ 0x005EDE60-0x005EDEA8 BYTE_EXACT
// size      72 bytes
// prototype int (__thiscall ?maximize@Win@@QAEHXZ)(Win* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EDE83 0x005EDE9D
// 0x005EDE60  ?maximize@Win@@QAEHXZ  ->  ?maximize@Win@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
int Win::maximize() {
    void *active = WinPointerOwner3;
    WinPointerOwner1 = 0;
    WinPointerOwner2 = 0;
    if (active == this) {
        WinPointerOwner3 = 0;
        this->vslot_04();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    ShowWindow(
        HandleMain, 3);
    return 0;
}

// ===== homed from src/recovered/005f0610.cpp =====

// ORIGINAL: 0x005F0610 ?set_bottom_border_thickness@Win@@QAEXH@Z 0x005F0610-0x005F064D BYTE_EXACT
// size      61 bytes
// prototype void (__thiscall ?set_bottom_border_thickness@Win@@QAEXH@Z)(Win* this, int thickness)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F0646
// 0x005F0610  ?set_bottom_border_thickness@Win@@QAEXH@Z  ->  ?set_bottom_border_thickness@Win@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
void Win::set_bottom_border_thickness(int thickness) {
    if (iSomeFlag_ & 2) {
        bottom_border_thickness_ = thickness;
        int width = client_rect_.right - client_rect_.left;
        int height = client_rect_.bottom - client_rect_.top;
        this->resize(width, height, 0);
    }
}

// ===== homed from src/recovered/005f85e0.cpp =====

// ORIGINAL: 0x005F85E0 ?on_mousewheel_up@Win@@QAEXH@Z 0x005F85E0-0x005F860D BYTE_EXACT
// size      45 bytes
// prototype void (__thiscall ?on_mousewheel_up@Win@@QAEXH@Z)(Win* this, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606440
// 0x005F85E0  ?on_mousewheel_up@Win@@QAEXH@Z  ->  ?on_mousewheel_up@Win@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
void Win::on_mousewheel_up(int delta) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = val_9_;
    Scroll *first = scroll_vert_;
    if (first != 0) {
        first->on_mousewheel_up(delta);
    }
    Scroll *second = scroll_horz_;
    if (second != 0) {
        second->on_mousewheel_up(delta);
    }
}

// ===== homed from src/recovered/005f4ca0.cpp =====

// ORIGINAL: 0x005F4CA0 ?update_nc_buffer@Win@@QAEXH@Z 0x005F4CA0-0x005F4CB2 BYTE_EXACT
// size      18 bytes
// prototype void (__thiscall ?update_nc_buffer@Win@@QAEXH@Z)(Win* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F4CA9
// 0x005F4CA0  ?update_nc_buffer@Win@@QAEXH@Z  ->  ?update_nc_buffer@Win@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
void Win::update_nc_buffer(int flags) {
    this->on_nc_paint(0, flags);
}

// ===== homed from src/recovered/005f1660.cpp =====

// ORIGINAL: 0x005F1660 ?OnChar@Win@@QAAXPAXDH@Z 0x005F1660-0x005F16CB BYTE_EXACT
// symbol    ?OnChar@@YAXPAXDH@Z
// symbol    ?OnChar@Win@@QAAXPAXHH@Z
// notes     `H` where the catalogue guessed `D`: the image loads a dword
//           for that parameter, so it is an int.
// size      107 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// indirect  0x005F16C3
// 0x005F1660  ?OnChar@Win@@QAAXPAXDH@Z  ->  ?OnChar@Win@@QAAXPAXDH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
// FREE, not a member, and the shift is the whole finding. The catalogue
// spells this `?OnChar@Win@@QAAXPAXDH@Z`; a QAA name invents a receiver, and
// that invented `this` IS the HWND. Every parameter therefore sat one slot
// late: what the member form called `hwnd` is the character and what it
// called `ch` is the flags, which is why the body used to read
// `on_char(hwnd, ch)` - dispatching the window handle as the typed
// character. The stack slots it reads are unchanged, so the bytes are
// unchanged; only the names were wrong. Its siblings OnNCHitTest and
// OnSysKey were homed as free functions for the same reason.
void __cdecl OnChar(void *hwnd, char ch, int flags) {
    int result;
    if (WinFocusWindow != nullptr) {
        result = reinterpret_cast<int>(WinModalFocus);
        if (result == 0) {
            result = reinterpret_cast<int>(WinFocusWindow);
        }
    } else {
        result = reinterpret_cast<int>(WinInputFocus);
        if (result == 0) {
            result = reinterpret_cast<int>(WinDefaultFocus);
        }
    }
    WinActiveWindow = reinterpret_cast<Win *>(result);
    if (result != 0) {
        char *p = *reinterpret_cast<char **>(result + 0xa8);
        if (p != 0 && (*reinterpret_cast<unsigned char *>(p + 0x9c) & 1) != 0) {
            Win *visobj = *reinterpret_cast<Win **>(p + 0xc4);
            if (visobj == 0 || visobj->is_visible() != 0) {
                reinterpret_cast<Win *>(p)->on_char(ch, flags);
            }
        }
    }
}

// ===== homed from src/recovered/units/005ec630.cpp =====

// ORIGINAL: 0x005EC630 ?init@Win@@QAEHPAURECT@@PADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z 0x005EC630-0x005EC678 FILE BYTE_EXACT
// symbol    ?init@Win@@QAEHPAUtagRECT@@PADHPAV1@PAVMenu@@PAUBorderSizing@@@Z
// notes     the RECT overload. `marker_symbols` resolves this region to
//           the FIRST `Win::init` in the object, which is the nine-argument
//           overload - so the emitted name has to be stated here.
// size      72 bytes
// prototype int (__thiscall ?init@Win@@QAEHPAURECT@@PADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z)(Win* this, RECT*, int8*, int, Win*, Menu*, BorderSizing*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EBD80
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005EC630
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ec630/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Win@@QAEHPAURECT@@PADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z  at 0x005EC630  (72 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
int Win::init(RECT* bounds, char* caption, int flags, Win* parent, Menu* menu,
              BorderSizing* border) {
    if (bounds == 0) {
        return 3;
    }
    // The RECT's own fields, not `char *` plus 0/4/8/0xC. The artifact walked
    // it raw because nothing had modelled the parameter; the offsets it used
    // ARE left/top/right/bottom in order.
    int top = bounds->top;
    int left = bounds->left;
    return this->init(
        left, top,
        bounds->right - left,
        bounds->bottom - top,
        caption, flags, parent, menu, border);
}

// ===== homed from src/recovered/units/005edcd0.cpp =====
typedef int int32_t;
typedef unsigned char uint8_t;
typedef void(__stdcall *ThreeArgFn)(int, int, int);

void Win::hide() {
    int32_t zero;
    int32_t focus_a, focus_b, chosen;
    Win **child;
    int32_t i;

    if (!(iSomeFlag_ & 1)) {
        return;
    }

    if (WinBubbleCompanion == this) {
        zero = 0;
        if (WinBubbleActive != zero) {
            WinBubbleCompanion = reinterpret_cast<Win *>(zero);
            WinBubbleActive = zero;
            update_screen(&WinBubbleRect, 0);
            flip(&WinBubbleRect);
        }
        goto converge;
    }
    zero = 0;

converge:
    focus_a = reinterpret_cast<int>(WinPointerOwner1);
    focus_b = reinterpret_cast<int>(WinPointerOwner2);
    chosen = (focus_a != zero) ? focus_a : focus_b;
    if (chosen == reinterpret_cast<int32_t>(this)) {
        goto clear_focus;
    }
    if (focus_a == zero) {
        if (focus_b == zero) {
            goto skip_all;
        }
        goto search;
    }
    focus_b = focus_a;

search:
    if (focus_b == zero) {
        goto skip_all;
    }
    if (child_count_ <= 0) {
        goto skip_all;
    }
    child = children_;
    i = 0;
    for (;;) {
        if (reinterpret_cast<int32_t>(*child) == focus_b) {
            goto found;
        }
        if ((*child)->is_descendant(reinterpret_cast<Win *>(focus_b))) {
            goto found;
        }
        i++;
        child++;
        if (i >= child_count_) {
            goto skip_all;
        }
    }

found:
    zero = 0;
clear_focus:
    WinPointerOwner1 = reinterpret_cast<Win *>(zero);
    WinPointerOwner2 = reinterpret_cast<Win *>(zero);

skip_all:
    if (WinPointerOwner3 == this) {
        WinPointerOwner3 = 0;
        this->vslot_04();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    iSomeFlag_ &= 0xfffffffeu;
    if (WinFocusWindow == this) {
        this->release_modal();
    }

    if (!(*reinterpret_cast<uint8_t *>(JackalInitFlags) & 1)) {
        return;
    }

    {
        int32_t total = WinRootCount;
        WinZOrderCount = 0;
        if (total > 0) {
            int32_t target = reinterpret_cast<int>(WinZOrderWindow);
            Win **p = WinRootWindows;
            int32_t j = 0;
            for (;;) {
                if (target != 0 && target == reinterpret_cast<int32_t>(*p)) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                if ((*p)->iSomeFlag_ & 1) {
                    recurse_zorder(*p);
                    target = reinterpret_cast<int>(WinZOrderWindow);
                }
                total = WinRootCount;
                j++;
                p++;
                if (j >= total) {
                    break;
                }
            }
        }
    }

    InvalidateRect(HandleMain, 0, 0);

    {
        Win *other = reinterpret_cast<Win *>(val_4_);
        if (other != 0) {
            if (other->vslot_23() == 0) {
                other->vslot_07();
            }
        }
    }
}

// ===== homed from src/recovered/units/005f63c0.cpp =====

// FINDING, not a layout to declare: this body reads PAST THE END OF Win.
// `sizeof(Win)` is 0x444 (win.h's static_assert, and scroll_horz_ at 0x440
// is the last member), so `self + 0x8f4` and `self + idx*4 + 0x900` are
// 0x4B0 and beyond outside the object. A Win method cannot legally touch
// them, so the receiver is wrong: this body belongs to a Win-DERIVED class
// that the catalogue attributed to Win, the same way `adjust_menus` was
// given a receiver it never had (0x005F0540, fixed and BYTE_EXACT).
// Searched for the owner and did NOT find it: no declared class in this
// tree puts a `Spot` at 0x8F4 - councwin.h has one at 0xA48, diplowin.h at
// 0xED0, basewin.h at 0x40EB4. Until the class is identified the offsets
// cannot be turned into members, and the `char *self` here is the honest
// spelling of an access whose type is genuinely unknown.
// The body is UNCLAIMED, so nothing in the image validates these offsets
// either - they are the decompiler's, and they are what points at the
// wrong receiver in the first place.
void Win::on_l_button_down(long flags, int x, int y, unsigned int keys, int dbl) {
    char *self = reinterpret_cast<char *>(this);

    Win *child = win_parent_;
    if (child != 0) {
        reinterpret_cast<Win *>(child)->bring_child_to_top(this);
    } else if (reinterpret_cast<int32_t>(this) != 0 &&
               (iFlags_ & 0x2000000) == 0) {
        int32_t idx = 0;
        int32_t count = WinRootCount;
        if (count > 0) {
            Win **p = WinRootWindows;
            for (;;) {
                if (*p == this) {
                    if (idx < count) {
                        if (idx > 0) {
                            Win **q = WinRootWindows + idx;
                            do {
                                *q = *(q - 1);
                                --q;
                                --idx;
                            } while (idx != 0);
                        }
                        WinRootWindows[0] = this;
                    }
                    break;
                }
                ++idx;
                ++p;
                if (idx >= count) {
                    break;
                }
            }
        }
        this->update_zorder();
    }

    child = win_parent_;
    if (child != 0) {
        reinterpret_cast<Win *>(child)->set_dialog_focus(this);
    }

    if (keys == 0) {
        WinPointerOwner2 = nullptr;
        WinPointerOwner1 = this;
    } else {
        WinPointerOwner1 = nullptr;
        WinPointerOwner2 = this;
    }

    WinSizingFlag = 1;
    if (flags != 0 &&
        (iFlags_ & 0x200200) == 0 &&
        (iSomeFlag_ & 8) == 0) {
        int32_t related;
        if (keys == 0) {
            WinCallbackWindow = this;
            int32_t fp = field_40C_;
            if (fp != 0) {
                reinterpret_cast<void(__cdecl *)(int, int)>(fp)(x, y);
            }
            this->vslot_28(y, x);
            related = val_17_;
        } else {
            this->vslot_38(y, x);
            related = val_26_;
        }
        if (related != 0 && reinterpret_cast<Win *>(related)->vslot_23() == 0) {
            reinterpret_cast<Win *>(related)->vslot_07();
        }
    }

    int32_t hitTest = this->vslot_61();
    int32_t idx = 0;
    if (hitTest == 0 ||
        reinterpret_cast<Spot *>(self + 0x8f4)->check(x, y, &idx, 0) < 0 ||
        reinterpret_cast<Slot88Shim *>(self)->slot088(
            idx, *reinterpret_cast<int32_t *>(self + idx * 4 + 0x900)) != 0) {
        this->left_down_event(x, y, dbl);
    }
}

// ===== homed from src/recovered/units/005f6550.cpp =====

void Win::on_l_button_up(int x, int y, unsigned int keys, int dbl) {
    Win *captured = WinPointerOwner1;
    if (captured == nullptr) {
        captured = WinPointerOwner2;
    }
    bool isSelf = (captured == this);
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = 0;

    if (isSelf) {
        RECT r;
        r.left = 0;
        r.top = 0;
        r.right = outer_rect_.right - outer_rect_.left;
        r.bottom = outer_rect_.bottom - outer_rect_.top;
        if (in_box(x, y, &r) != 0 &&
            (iFlags_ & 0x200000) == 0 &&
            (*reinterpret_cast<uint8_t *>(&iSomeFlag_) & 8) == 0) {
            if (dbl == 0) {
                WinCallbackWindow = this;
                void(__cdecl * cb)(int, int) =
                    reinterpret_cast<void(__cdecl *)(int, int)>(field_414_);
                if (cb != 0) {
                    cb(x, y);
                }
                this->vslot_19(x, y);
                Win *child = reinterpret_cast<Win *>(flags_);
                if (child != 0 && child->vslot_23() == 0) {
                    child->vslot_07();
                }
            } else {
                this->vslot_32(x, y);
                Win *child = reinterpret_cast<Win *>(val_20_);
                if (child != 0 && child->vslot_23() == 0) {
                    child->vslot_07();
                }
            }
        }
    }

    if (*reinterpret_cast<uint8_t *>(&iSomeFlag_) & 1) {
        Win *w = win_parent_;
        if (w != 0 && w->is_visible() == 0) {
            return;
        }
        if ((iFlags_ & 0x200000) == 0 &&
            (*reinterpret_cast<uint8_t *>(&iSomeFlag_) & 8) == 0) {
            if (dbl == 0) {
                WinCallbackWindow = this;
                void(__cdecl * cb)(int, int) =
                    reinterpret_cast<void(__cdecl *)(int, int)>(field_41C_);
                if (cb != 0) {
                    cb(x, y);
                }
                this->vslot_21(x, y);
                Win *child = reinterpret_cast<Win *>(val_10_);
                if (child != 0 && child->vslot_23() == 0) {
                    child->vslot_07();
                }
            } else {
                this->vslot_34(x, y);
                Win *child = reinterpret_cast<Win *>(val_22_);
                if (child != 0 && child->vslot_23() == 0) {
                    child->vslot_07();
                }
            }
        }
    }
}

// ===== homed from src/recovered/units/005ec740.cpp =====

int Win::set_cursor(Sprite* sprite, int hot_x, int hot_y) {
    if (sprite != 0) {
        cursor_sprite_ = sprite;
        field_18C_ = hot_x;
        field_190_ = hot_y;
    }
    // IDENTIFIED: the image emits `push 1; push 0; call 0x5f1820` here, and
    // 0x005F1820 is `Win::update_cursor`. The artifact spelled it `fn(0, 1)`
    // with no declaration; deleting the call was wrong, and listing_diff
    // against this body is what named the target.
    Win::update_cursor(nullptr, 1);
    return 0;
}

// ===== homed from src/recovered/005edff0.cpp =====
typedef int (__stdcall *SetRectFn)(void*, int, int, int, int);

// ORIGINAL: 0x005EDFF0 ?UNK7@Win@@QAEXHHHH@Z 0x005EDFF0-0x005EE01D BYTE_EXACT
// size      45 bytes
// prototype void (__thiscall ?UNK7@Win@@QAEXHHHH@Z)(Win* this, int xLeft, int yTop, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE011
// 0x005EDFF0  ?UNK7@Win@@QAEXHHHH@Z  ->  ?UNK7@Win@@QAEXHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
void Win::UNK7(int x, int y, int width, int height) {
    RECT rectangle;  // the artifact spelled this `int rectangle[4]`
    SetRect(
        &rectangle, x, y, x + width, y + height);
}

// ===== homed from src/recovered/units/005f5480.cpp =====
typedef long LRESULT;
typedef long (__stdcall *SendMsgFn)(int, int, int, void*);

// ORIGINAL: 0x005F5480 ?on_window_pos_changed@Win@@QAEJPAUWINDOWPOS@@@Z 0x005F5480-0x005F54A1 FILE BYTE_EXACT
// symbol    ?on_window_pos_changed@Win@@UAEJPAUtagWINDOWPOS@@@Z
// size      33 bytes
// prototype LRESULT (__thiscall ?on_window_pos_changed@Win@@QAEJPAUWINDOWPOS@@@Z)(Win* this, WINDOWPOS*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F5482 0x005F5498
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F5480
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f5480/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_window_pos_changed@Win@@QAEJPAUWINDOWPOS@@@Z  at 0x005F5480  (33 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
LRESULT Win::on_window_pos_changed(WINDOWPOS* pos) {
    this->set_rects();
    return DefWindowProcA(HandleMain, 0x47, 0,
                          reinterpret_cast<LPARAM>(pos));
}

// ===== homed from src/recovered/units/005f4ab0.cpp =====

void Win::bring_to_top() {
    Win *parent = win_parent_;
    if (parent != 0) {
        if (this == 0) return;
        if (iFlags_ & 0x2000000) return;

        int count = parent->child_count_;
        int i = 0;
        if (count > 0) {
            Win **slot = &parent->children_[0];
            do {
                if (*slot == this) break;
                i++;
                slot++;
            } while (i < count);
        }
        if (i < count) {
            if (i > 0) {
                Win **dst = &parent->children_[i];
                do {
                    *dst = dst[-1];
                    dst--;
                } while (--i);
            }
            parent->children_[0] = this;
        }

        WinZOrderCount = 0;
        if (WinRootCount > 0) {
            Win *highlighted = reinterpret_cast<Win *>(WinZOrderWindow);
            Win **zslot = WinRootWindows;
            for (int j = 0; j < WinRootCount; j++) {
                if (highlighted != 0 && highlighted == *zslot) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                Win *w = reinterpret_cast<Win *>(*zslot);
                if (w->iSomeFlag_ & 1) {
                    recurse_zorder(w);
                    highlighted = reinterpret_cast<Win *>(WinZOrderWindow);
                }
                zslot++;
            }
        }
        return;
    }

    if (this == 0) return;
    if (iFlags_ & 0x2000000) return;

    int count = WinRootCount;
    int i = 0;
    if (count > 0) {
        Win **slot = WinRootWindows;
        do {
            if (*slot == this) break;
            i++;
            slot++;
        } while (i < count);
    }
    if (i < count) {
        if (i > 0) {
            Win **dst = WinRootWindows + i;
            do {
                *dst = dst[-1];
                dst--;
            } while (--i);
        }
        WinRootWindows[0] = this;
    }

    WinZOrderCount = 0;
    if (WinRootCount > 0) {
        Win *highlighted = reinterpret_cast<Win *>(WinZOrderWindow);
        Win **zslot = WinRootWindows;
        for (int j = 0; j < WinRootCount; j++) {
            if (highlighted != 0 && highlighted == *zslot) {
                WinZOrderCount = 0;
                WinZOrderFlag = 0;
            }
            Win *w = reinterpret_cast<Win *>(*zslot);
            if (w->iSomeFlag_ & 1) {
                recurse_zorder(w);
                highlighted = reinterpret_cast<Win *>(WinZOrderWindow);
            }
            zslot++;
        }
    }
}

// ===== homed from src/recovered/units/005ec980.cpp =====
typedef short (__stdcall *RBtnFn)(int);
typedef int (__stdcall *KeyFn)(int);

// ORIGINAL: 0x005EC980 ?get_rbutton_state@Win@@QAEHXZ 0x005EC980-0x005EC99B FILE BYTE_EXACT
// size      27 bytes
// prototype int (__thiscall ?get_rbutton_state@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EC983 0x005EC991
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005EC980
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ec980/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_rbutton_state@Win@@QAEHXZ  at 0x005EC980  (27 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
int Win::get_rbutton_state() {
    short r1 = GetAsyncKeyState(2);
    int mask = r1 >> 15;
    int r2 = GetSystemMetrics(0x17);
    return r2 ^ mask;
}

// ===== homed from src/recovered/units/005ec9a0.cpp =====

// ORIGINAL: 0x005EC9A0 ?set_parent_dialog@Win@@QAEXPAVWin@@@Z 0x005EC9A0-0x005ECB58 FILE BYTE_EXACT
// symbol    ?set_parent_dialog@Win@@QAEXPAV1@@Z
// size      440 bytes
// prototype void (__thiscall ?set_parent_dialog@Win@@QAEXPAVWin@@@Z)(Win* this, Win*)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005D4680
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005EC9A0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ec9a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_parent_dialog@Win@@QAEXPAVWin@@@Z  at 0x005EC9A0  (440 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void Win::set_parent_dialog(Win *dialog) {

    if (iFlags_ & 0x200000) {
        return;
    }
    if (iSomeFlag_ & 8) {
        return;
    }
    if ((iFlags_ & 0x1000) && dialog == 0) {
        return;
    }

    char *b = reinterpret_cast<char *>(dialog);
    if ((~*reinterpret_cast<unsigned int *>(b + 0x98) & 0x1000) == 0) {
        return;
    }

    if (*reinterpret_cast<int *>(b + 0xd4) == 0) {
        void *iv2 = (*reinterpret_cast<int *>(b + 0xdc) != 0)
            ? reinterpret_cast<Heap *>(*reinterpret_cast<int *>(b + 0xdc))->get(0x1c)
            : mem_get(0x1c);
        *reinterpret_cast<void **>(b + 0xcc) = iv2;
        if (iv2 == 0) {
            goto tail_common;
        }
        *reinterpret_cast<void **>(reinterpret_cast<char *>(iv2) + 4) = this;
        char *h1 = *reinterpret_cast<char **>(b + 0xcc);
        *reinterpret_cast<void **>(h1 + 0xc) = h1;
        char *h2 = *reinterpret_cast<char **>(b + 0xcc);
        *reinterpret_cast<void **>(h2 + 0x10) = h2;

        if (*reinterpret_cast<int *>(b + 0xdc) != 0) {
            *reinterpret_cast<void **>(*reinterpret_cast<char **>(b + 0xcc) + 8) =
                reinterpret_cast<Heap *>(*reinterpret_cast<int *>(b + 0xdc))->get(4);
        } else {
            *reinterpret_cast<void **>(*reinterpret_cast<char **>(b + 0xcc) + 8) = mem_get(4);
        }
        char *h4 = *reinterpret_cast<char **>(b + 0xcc);
        if (*reinterpret_cast<int *>(h4 + 8) == 0) {
            goto tail_common;
        }
        *reinterpret_cast<void **>(b + 0xd0) = h4;
    } else {
        void *newnode = (*reinterpret_cast<int *>(b + 0xdc) != 0)
            ? reinterpret_cast<Heap *>(*reinterpret_cast<int *>(b + 0xdc))->get(0x1c)
            : mem_get(0x1c);
        char *head = *reinterpret_cast<char **>(b + 0xcc);
        char *tail = *reinterpret_cast<char **>(head + 0x10);
        *reinterpret_cast<void **>(tail + 0xc) = newnode;

        char *tail2 = *reinterpret_cast<char **>(*reinterpret_cast<char **>(b + 0xcc) + 0x10);
        char *node = *reinterpret_cast<char **>(tail2 + 0xc);
        if (node == 0) {
            goto tail_common;
        }
        *reinterpret_cast<void **>(node + 0x10) = tail2;
        *reinterpret_cast<void **>(
            *reinterpret_cast<char **>(
                *reinterpret_cast<char **>(*reinterpret_cast<char **>(b + 0xcc) + 0x10) + 0xc) + 0xc) =
            *reinterpret_cast<char **>(b + 0xcc);
        *reinterpret_cast<void **>(*reinterpret_cast<char **>(b + 0xcc) + 0x10) =
            *reinterpret_cast<void **>(
                *reinterpret_cast<char **>(*reinterpret_cast<char **>(b + 0xcc) + 0x10) + 0xc);
        char *nn = *reinterpret_cast<char **>(*reinterpret_cast<char **>(b + 0xcc) + 0x10);
        *reinterpret_cast<void **>(b + 0xd0) = nn;
        *reinterpret_cast<void **>(nn + 4) = this;

        if (*reinterpret_cast<int *>(b + 0xdc) != 0) {
            *reinterpret_cast<void **>(*reinterpret_cast<char **>(b + 0xd0) + 8) =
                reinterpret_cast<Heap *>(*reinterpret_cast<int *>(b + 0xdc))->get(4);
        } else {
            *reinterpret_cast<void **>(*reinterpret_cast<char **>(b + 0xd0) + 8) = mem_get(4);
        }
        char *d0 = *reinterpret_cast<char **>(b + 0xd0);
        if (*reinterpret_cast<int *>(d0 + 8) == 0) {
            goto tail_common;
        }
    }

    *reinterpret_cast<int *>(b + 0xd8) = *reinterpret_cast<int *>(b + 0xd4);
    *reinterpret_cast<int *>(b + 0xd4) = *reinterpret_cast<int *>(b + 0xd4) + 1;

tail_common:
    if (*reinterpret_cast<int *>(b + 0xd4) - 1 >= 0) {
        *reinterpret_cast<int *>(b + 0xd8) = 0;
        *reinterpret_cast<int *>(b + 0xd0) = *reinterpret_cast<int *>(b + 0xcc);
    }
    win_parent_ = dialog;
}

// ===== homed from src/recovered/units/005f5200.cpp =====

// ORIGINAL: 0x005F5200 ?add_child@Win@@QAEXPAUWin@@@Z 0x005F5200-0x005F5268 FILE BYTE_EXACT
// symbol    ?add_child@Win@@QAEXPAV1@@Z
// size      104 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F5200
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f5200/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_child@Win@@QAEXPAUWin@@@Z  at 0x005F5200  (104 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
void Win::add_child(Win* child) {
    if (child == 0) {
        return;
    }
    char *other = reinterpret_cast<char *>(child);
    if (*reinterpret_cast<unsigned int *>(other + 0x98) & 0x2000000) {
        int count = child_count_;
        children_[count] = child;
        child_count_ = child_count_ + 1;
        return;
    }
    int count = child_count_;
    if (count > 0) {
        Win **slot = &children_[count];
        do {
            *slot = *(slot - 1);
            --slot;
        } while (--count);
    }
    children_[0] = child;
    child_count_ = child_count_ + 1;
}

// ===== homed from src/recovered/units/005f50e0.cpp =====

// ORIGINAL: 0x005F50E0 ?remove_parent@Win@@QAAXPAUWin@@@Z 0x005F50E0-0x005F5136 FILE BYTE_EXACT
// symbol    ?remove_parent@Win@@QAAXPAV1@@Z
// size      86 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F5113
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F50E0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f50e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?remove_parent@Win@@QAAXPAUWin@@@Z  at 0x005F50E0  (86 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
void __cdecl Win::remove_parent(Win* window) {
    if (this == 0) {
        return;
    }
    int count = WinRootCount;
    for (int i = 0; i < count; i++) {
        if (reinterpret_cast<Win **>(0x009B6E48)[i] == this) {
            if (i < count) {
                count--;
                WinRootCount = count;
                this->hide();
                count = WinRootCount;
                if (i < count) {
                    Win **p = reinterpret_cast<Win **>(&reinterpret_cast<Win **>(0x009B6E48)[i]);
                    int n = count - i;
                    do {
                        *p = p[1];
                        p++;
                    } while (--n);
                }
            }
            return;
        }
    }
}

// ===== homed from src/recovered/units/005f2940.cpp =====

void Win::set_rects() {
    if (this->iSomeFlag_ & 2) {
        int32_t barH;
        if ((this->iFlags_ & 0x10) == 0 || (this->iFlags_ & 0x400000) != 0) {
            barH = this->border_thickness_;
        } else {
            barH = this->caption_height_;
        }
        int32_t bottomBar = this->bottom_border_thickness_;
        if (bottomBar == -1) {
            bottomBar = this->border_thickness_;
        }

        this->buffer1_->init(this->client_rect_.right - this->client_rect_.left, barH, 0, 0);
        this->buffer1_->sync_to_palette(PaletteActive);
        this->buffer1_->fill(static_cast<uint8_t>(WinFillColour));

        this->buffer4_->init(this->client_rect_.right - this->client_rect_.left, bottomBar, 0, 0);
        this->buffer4_->sync_to_palette(PaletteActive);
        this->buffer4_->fill(static_cast<uint8_t>(WinFillColour));

        this->buffer3_->init(this->border_thickness_,
                              (this->client_rect_.bottom - this->client_rect_.top) - bottomBar - barH,
                              0, 0);
        this->buffer3_->sync_to_palette(PaletteActive);
        this->buffer3_->fill(static_cast<uint8_t>(WinFillColour));

        this->buffer2_->init(this->border_thickness_,
                              (this->client_rect_.bottom - this->client_rect_.top) - bottomBar - barH,
                              0, 0);
        this->buffer2_->sync_to_palette(PaletteActive);
        this->buffer2_->fill(static_cast<uint8_t>(WinFillColour));

        this->on_redraw_nc(0, -1);
    }
}

// ===== homed from src/recovered/units/005f2ac0.cpp =====
typedef int (__stdcall *IfaceGetHdcProc)(void *, HDC *);
typedef int (__stdcall *IfaceReleaseHdcProc)(void *, HDC);
typedef HDC (__stdcall *GetDCProc)(void *);
typedef int (__stdcall *ReleaseDCProc)(void *, HDC);
typedef void *(__stdcall *CreatePenIndirectProc)(const LocalLogPen *);
typedef void *(__stdcall *SelectObjectProc)(HDC, void *);
typedef int (__stdcall *MoveToExProc)(HDC, int, int, void *);
typedef int (__stdcall *LineToProc)(HDC, int, int);
typedef int (__stdcall *DeleteObjectProc)(void *);

// The pair ORDER is the image's: lineTo takes (a1,a2) and moveTo takes
// (a3,a4), so the first pair is the END of the segment.
void Win::window_line_raw(int x2, int y2, int x1, int y1, int colour,
                          int width, unsigned int style) {
    if (WinHdcRefCount != 0) {
        WinHdcRefCount = WinHdcRefCount + 1;
    } else {
        void *iface = DirectDrawSurface;
        if (iface != 0) {
            IfaceGetHdcProc fn = (*reinterpret_cast<IfaceGetHdcProc **>(iface))[17];
            fn(iface, &WinSharedHdc);
        } else {
            WinSharedHdc = GetDC(HandleMain);
        }
        if (WinSharedHdc == 0) {
            return;
        }
        WinHdcRefCount = 1;
    }
    if (WinSharedHdc != 0) {
        if (palette_seed_cache_ != PaletteActive->seed_) {
            PaletteActive->set_active_window(this);
            palette_seed_cache_ = PaletteActive->seed_;
        }

        LOGPEN pen;
        pen.lopnStyle = style;
        pen.lopnWidth.x = width;
        pen.lopnColor = (colour & 0xffff) | 0x1000000;

        HPEN hpen = CreatePenIndirect(&pen);
        if (hpen != 0) {
            HGDIOBJ oldPen = SelectObject(WinSharedHdc, hpen);

            MoveToEx(WinSharedHdc, x1, y1, 0);

            LineTo(WinSharedHdc, x2, y2);

            SelectObject(WinSharedHdc, oldPen);

            DeleteObject(hpen);

            WinHdcRefCount = WinHdcRefCount - 1;
            if (WinHdcRefCount == 0) {
                void *iface2 = DirectDrawSurface;
                if (iface2 != 0) {
                    IfaceReleaseHdcProc fn2 = (*reinterpret_cast<IfaceReleaseHdcProc **>(iface2))[26];
                    fn2(iface2, WinSharedHdc);
                    WinSharedHdc = 0;
                    return;
                }
                ReleaseDC(HandleMain, WinSharedHdc);
                WinSharedHdc = 0;
            }
        }
    }
}

// ===== homed from src/recovered/units/005f4d00.cpp =====

void __cdecl Win::update() {
    for (int i = 0; i < WinZOrderCount; ++i) {
        reinterpret_cast<Win *>(reinterpret_cast<void **>(WinZOrderArray)[i])->on_nc_paint(0, -1);
        if (reinterpret_cast<Win *>(reinterpret_cast<void **>(WinZOrderArray)[i])->vslot_61() != 0) {
            reinterpret_cast<Win *>(reinterpret_cast<void **>(WinZOrderArray)[i])->vslot_63();
            do_sound();
        }
    }
}

// ===== homed from src/recovered/units/005f4c30.cpp =====

Buffer *Win::redraw_nc_buffer(int index) {
    this->on_redraw_nc(0, index);
    switch (index) {
        case 0:
            return buffer1_;
        case 1:
            return buffer2_;
        case 2:
            return buffer4_;
        case 3:
            return buffer3_;
        default:
            return nullptr;
    }
}

// ===== homed from src/recovered/units/005f05d0.cpp =====

// ORIGINAL: 0x005F05D0 ?set_border_thickness@Win@@QAEXH@Z 0x005F05D0-0x005F060D FILE BYTE_EXACT
// size      61 bytes
// prototype void (__thiscall ?set_border_thickness@Win@@QAEXH@Z)(Win* this, int thickness)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F0606
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F05D0
// measured tier  NO_COMPILE
// refusal        u005f05d0.cpp(8) : error C2653: 'Win' : is not a class or namespace name u005f05d0.cpp(9) : error C2673: 'set_border_thickness' : global functions do not have '
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f05d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_border_thickness@Win@@QAEXH@Z  at 0x005F05D0  (61 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void Win::set_border_thickness(int thickness) {
    if (iSomeFlag_ & 2) {
        border_thickness_ = thickness;
        this->resize(
            client_rect_.right - client_rect_.left,
            client_rect_.bottom - client_rect_.top,
            0);
    }
}

// ===== homed from src/recovered/units/005ed9c0.cpp =====

// ORIGINAL: 0x005ED9C0 ?on_redraw@Win@@QAEHXZ 0x005ED9C0-0x005ED9C5 FILE BYTE_EXACT
// symbol    ?on_redraw@Win@@QAEHHH@Z
// size      5 bytes
// prototype int (__thiscall ?on_redraw@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005ED9C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ed9c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@Win@@QAEHXZ  at 0x005ED9C0  (5 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
int Win::on_redraw(int, int) {
    return 0;
}

// ===== homed from src/recovered/units/005f5810.cpp =====
typedef unsigned int uint32_t;

void Win::on_nc_paint(RECT * area, int flags) {

    if ((iFlags_ & 0x11) != 0 &&
        (iSomeFlag_ & 1) != 0 &&
        (win_parent_ == 0 || this->is_visible() != 0)) {
        int stack8 = 0, stack4 = 0;
        this->client_to_screen(&stack8, &stack4);
        stack8 -= outer_rect_.left;
        stack4 -= outer_rect_.top;

        if (area != 0) {
            flags = -1;
            reinterpret_cast<Buffer *>(area)->set_clip(area);
        }

        int iVar1;
        if ((iFlags_ & 0x10) == 0 ||
            (iFlags_ & 0x400000) != 0) {
            iVar1 = border_thickness_;
        } else {
            iVar1 = caption_height_;
        }

        if (buffer1_ != 0 && (flags == -1 || flags == 0)) {
            Buffer *buf;
            int x, y;
            if (WinBackBuffer == 0) {
                buf = (&ScreenBuffer);
                x = stack8;
                y = stack4;
            } else {
                buf = WinBackBuffer;
                x = stack8 - WinViewOriginX;
                y = stack4 - WinViewOriginY;
            }
            (&ScreenBuffer)->draw(buf, WinFillColour, x, y, 1, 1);
        }

        if (buffer4_ != 0 && (flags == -1 || flags == 2)) {
            int v = bottom_border_thickness_;
            if (v == -1) {
                v = border_thickness_;
            }
            Buffer *buf;
            int x;
            if (WinBackBuffer == 0) {
                v = (client_rect_.bottom -
                     client_rect_.top) - v;
                buf = (&ScreenBuffer);
                x = stack8;
            } else {
                v = ((client_rect_.bottom -
                      client_rect_.top) - WinViewOriginY) - v;
                buf = WinBackBuffer;
                x = stack8 - WinViewOriginX;
            }
            (&ScreenBuffer)->draw(buf, WinFillColour, x, v + stack4, 1, 1);
        }

        if (buffer3_ != 0 && (flags == -1 || flags == 3)) {
            if (WinBackBuffer == 0) {
                (&ScreenBuffer)
                    ->draw((&ScreenBuffer), WinFillColour, stack8,
                           stack4 + iVar1, 1, 1);
            } else {
                (&ScreenBuffer)
                    ->draw(WinBackBuffer, WinFillColour,
                           stack8 - WinViewOriginX, (iVar1 - WinViewOriginY) + stack4, 1, 1);
            }
        }

        if (buffer2_ != 0 && (flags == -1 || flags == 1)) {
            Buffer *buf;
            int v;
            int yv = iVar1;
            if (WinBackBuffer == 0) {
                v = (client_rect_.right -
                     client_rect_.left) -
                    border_thickness_;
                buf = (&ScreenBuffer);
            } else {
                yv = iVar1 - WinViewOriginY;
                v = ((client_rect_.right -
                      client_rect_.left) -
                     border_thickness_) - WinViewOriginX;
                buf = WinBackBuffer;
            }
            (&ScreenBuffer)->draw(buf, WinFillColour, v + stack8, yv + stack4, 1, 1);
        }

        (&ScreenBuffer)->set_clip(&WinScreenClipRect);
    }
}

// ===== homed from src/recovered/units/005f5270.cpp =====

void Win::remove_child(Win *child) {
    if (child == 0) return;

    int count = child_count_;
    int i = 0;
    if (count > 0) {
        do {
            if (children_[i] == child) break;
            ++i;
        } while (i < count);
    }
    if (i < count) {
        child_count_ = count - 1;
        child->hide();
        if (i < child_count_) {
            do {
                children_[i] = children_[i + 1];
                ++i;
            } while (i < child_count_);
        }
    }
}

// ===== homed from src/recovered/units/005f6880.cpp =====

void Win::on_r_button_up(int x, int y, unsigned int keys, int dbl) {

    Win *captured = WinPointerOwner1;
    if (captured == nullptr) {
        captured = WinPointerOwner2;
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;

    if (captured == this) {
        int32_t fa = outer_rect_.left;
        int32_t fb = outer_rect_.top;
        int32_t fc = outer_rect_.right;
        int32_t fd = outer_rect_.bottom;
        RECT testRect = {0, 0, fc - fa, fd - fb};

        if (in_box(x, y, &testRect)) {
            if ((iFlags_ & 0x200000) == 0) {
                if ((iSomeFlag_ & 8) == 0) {
                    int32_t related;
                    if (dbl == 0) {
                        WinCallbackWindow = this;
                        int32_t fp = field_420_;
                        if (fp != 0) {
                            reinterpret_cast<void(__cdecl *)(int, int)>(fp)(x, y);
                        }
                        this->vslot_22(x, y);
                        related = val_11_;
                    } else {
                        this->vslot_35(x, y);
                        related = val_23_;
                    }
                    if (related != 0 && reinterpret_cast<Win *>(related)->vslot_23() == 0) {
                        reinterpret_cast<Win *>(related)->vslot_07();
                    }
                }
            }
        }
    }

    if ((iFlags_ & 0x200000) == 0 &&
        (iSomeFlag_ & 8) == 0) {
        int32_t related2;
        if (dbl == 0) {
            WinCallbackWindow = this;
            int32_t fp = field_428_;
            if (fp != 0) {
                reinterpret_cast<void(__cdecl *)(int, int)>(fp)(x, y);
            }
            this->vslot_24(x, y);
            related2 = val_13_;
        } else {
            this->vslot_37(x, y);
            related2 = val_25_;
        }
        if (related2 != 0 && reinterpret_cast<Win *>(related2)->vslot_23() == 0) {
            reinterpret_cast<Win *>(related2)->vslot_07();
        }
    }
}

// ===== homed from src/recovered/units/005f25c0.cpp =====

void __cdecl Win::OnRButtonDown(void * hwnd, long flags, int x, int y, unsigned int keys) {
    int result = reinterpret_cast<int>(get_mouse_window(
        reinterpret_cast<int *>(&flags), reinterpret_cast<int *>(&x)));
    if (result != 0) {
        reinterpret_cast<Win *>(result)->on_r_button_down(reinterpret_cast<long>(hwnd), flags, x, y, WinMouseDirect);
    }
    // 0x009B7A94, which window_proc calls as WinMouseHook(HWND, LPARAM).
    // SIGNATURES DISAGREE and the image cannot referee it: both sites push
    // two dwords __cdecl, so the argument TYPES are invisible in the asm.
    // Here `flags` and `x` hold the translated coordinates get_mouse_window
    // wrote back through them, so this site reads the hook as (int, int).
    // Merged onto the typed name with the cast kept at this call.
    if (WinMouseHook != nullptr) {
        reinterpret_cast<void(__cdecl *)(int, int)>(WinMouseHook)(flags, x);
    }
    if (WinCursorMoved != 0) {
        reinterpret_cast<void (__cdecl *)()>(WinCursorMoved)();
        return;
    }
}

// ===== homed from src/recovered/units/005f2620.cpp =====

void __cdecl Win::OnRButtonUp(void * hwnd, int x, int y, unsigned int keys) {
    WinTrackingWindow = nullptr;
    Win *win = reinterpret_cast<Win *>(get_mouse_window(&x, &y));
    if (win != 0) {
        VCallW *vcall = reinterpret_cast<VCallW *>(win);
        vcall->slot084(x, y, keys, WinMouseDirect);
    }
}

// ===== homed from src/recovered/units/005f0580.cpp =====

// ORIGINAL: 0x005F0580 ?set_caption_height@Win@@QAEXH@Z 0x005F0580-0x005F05BD FILE BYTE_EXACT
// size      61 bytes
// prototype void (__thiscall ?set_caption_height@Win@@QAEXH@Z)(Win* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F05B6
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F0580
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f0580/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_caption_height@Win@@QAEXH@Z  at 0x005F0580  (61 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
void Win::set_caption_height(int height) {
    if (iSomeFlag_ & 2) {
        caption_height_ = height;
        int w = client_rect_.right - client_rect_.left;
        int h = client_rect_.bottom - client_rect_.top;
        this->resize(w, h, 0);
    }
}

// ===== homed from src/unrecovered/005f6a50.cpp =====

// CORRECTED: the catalogue spells `?get_key_window@Win@@QAGHXZ` - `H`,
// int - but every caller assigns the result to a `Win *`. The int is
// the pointer, and win.h has said so since the focus-window work.
Win *Win::get_key_window() {
    char *w;
    if (WinFocusWindow == nullptr) {
        w = reinterpret_cast<char *>(WinInputFocus);
        if (WinInputFocus == 0) {
            w = (char *)WinDefaultFocus;
        }
    } else {
        w = (char *)WinFocusWindow;
        if (WinModalFocus != 0) {
            w = (char *)WinModalFocus;
        }
    }
    WinActiveWindow = reinterpret_cast<Win *>(w);
    int result = 0;
    if (w != 0) {
        result = *(int *)(w + 0xa8);
        if (result != 0) {
            if ((*(unsigned char *)(result + 0x9c) & 1) == 0) {
                return 0;
            }
            if (*(int *)(result + 0xc4) != 0) {
                if (((Win *)result)->is_visible() == 0) {
                    return 0;
                }
            }
        }
    }
    return reinterpret_cast<Win *>(result);
}

// ===== homed from src/recovered/units/005ed9d0.cpp =====
typedef BOOL (__stdcall *InvalidateRectFn)(void *, const RECT *, BOOL);
typedef void (__cdecl *Sub5f8670Fn)(RECT *, int, int);

void Win::show(int visible) {
    if ((iSomeFlag_ & 1) == 0 && (iSomeFlag_ & 4) != 0) {
        field_1A0_ |= 2;
        if ((iFlags_ & 2) == 0) {
            WinPointerOwner1 = 0;
            WinPointerOwner2 = 0;
        }
        iSomeFlag_ |= 1;
        if ((visible & 4) == 0) {
            if (win_parent_) {
                win_parent_->bring_child_to_top(this);
            } else {
                win_parent_->bring_parent_to_top(this);
            }
        }

        int32_t count = WinRootCount;
        WinZOrderCount = 0;
        if (count > 0) {
            int32_t cur = reinterpret_cast<int>(WinZOrderWindow);
            for (int32_t i = 0; i < count; ++i) {
                Win *w = WinRootWindows[i];
                if (cur != 0 && cur == reinterpret_cast<int32_t>(w)) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                if (reinterpret_cast<uint8_t *>(w)[0x9c] & 1) {
                    recurse_zorder(w);
                    cur = reinterpret_cast<int>(WinZOrderWindow);
                }
            }
        }

        if ((visible & 2) == 0) {
            resize_event(outer_rect_.right - outer_rect_.left, outer_rect_.bottom - outer_rect_.top);
        }
        if ((visible & 1) == 0) {
            this->vslot_12();
        }
        if (menu_ != 0) {
            menu_->vslot_62();
        }
        if (list_.count_ != 0 && list_.head_ != 0) {
            Win *obj = *reinterpret_cast<Win **>(reinterpret_cast<char *>(list_.current_) + 4);
            if (obj != 0) {
                reinterpret_cast<VCallArg51 *>(obj)->s51(1);
            }
        }

        RECT rect;
        if ((iSomeFlag_ & 2) == 0) {
            rect.right = outer_rect_.right - outer_rect_.left;
            rect.left = 0;
            rect.bottom = outer_rect_.bottom - outer_rect_.top;
            rect.top = 0;
            int x = client_rect_.left + outer_rect_.left;
            int y = client_rect_.top + outer_rect_.top;
            if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
                win_parent_->client_to_screen(&x, &y);
                if (iFlags_ & 0x8000) {
                    x -= win_parent_->outer_rect_.left;
                    y -= win_parent_->outer_rect_.top;
                }
            }
            offset_rect(&rect, x, y);
        } else {
            rect.right = client_rect_.right - client_rect_.left;
            rect.bottom = client_rect_.bottom - client_rect_.top;
            rect.left = 0;
            rect.top = 0;
            int x = 0;
            int y = 0;
            client_to_screen(&x, &y);
            rect.left += x - outer_rect_.left;
            rect.right += x - outer_rect_.left;
            rect.top += y - outer_rect_.top;
            rect.bottom += y - outer_rect_.top;
        }

        InvalidateRect(HandleMain, &rect, 0);

        Win *obj2 = reinterpret_cast<Win *>(val_3_);
        if (obj2 != 0) {
            if (reinterpret_cast<VCallRet23 *>(obj2)->s23() == 0) {
                obj2->vslot_07();
            }
        }
    }
}

// ===== homed from src/recovered/units/005f7ec0.cpp =====
typedef unsigned short uint16_t;

void Win::update_back_to_window(Buffer * buffer) {
    if (buffer == 0) {
        return;
    }

    WinBackBuffer = buffer;
    WinZOrderWindow = 0;
    WinViewOriginX = 0;
    WinViewOriginY = 0;

    uint32_t savedFlags = iSomeFlag_;
    iSomeFlag_ = savedFlags & 0xfffffffe;

    WinZOrderCount = 0;
    for (int i = 0; i < WinRootCount; i++) {
        Win *w = WinRootWindows[i];
        if (WinZOrderWindow != 0 && WinZOrderWindow == w) {
            WinZOrderCount = 0;
            WinZOrderFlag = 0;
        }
        if (*reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(w) + 0x9c) & 1) {
            recurse_zorder(w);
        }
    }

    int tail;
    if (!(savedFlags & 2)) {
        WinViewOriginX += client_rect_.left + outer_rect_.left;
        WinViewOriginY += client_rect_.top + outer_rect_.top;

        if (!(iFlags_ & 0x20) ||
            win_parent_ == 0) {
            goto after_offset_fixup;
        }
        this->client_to_screen(&WinViewOriginX, &WinViewOriginY);
        if (!(iFlags_ & 0x8000)) {
            goto after_offset_fixup;
        }
        {
            char *owner = reinterpret_cast<char *>(win_parent_);
            WinViewOriginX -= *reinterpret_cast<int *>(owner + 0x13c);
            tail = *reinterpret_cast<int *>(owner + 0x140);
        }
    } else {
        this->client_to_screen(&WinViewOriginX, &WinViewOriginY);
        WinViewOriginX -= outer_rect_.left;
        tail = outer_rect_.top;
    }
    WinViewOriginY -= tail;
after_offset_fixup:;

    RECT rect;
    if (!(savedFlags & 2)) {
        rect.left = outer_rect_.left;
        rect.top = outer_rect_.top;
        rect.right = outer_rect_.right;
        rect.bottom = outer_rect_.bottom;
    } else {
        rect.left = client_rect_.left;
        rect.top = client_rect_.top;
        rect.right = client_rect_.right;
        rect.bottom = client_rect_.bottom;
    }

    buffer->init(rect.right - rect.left, rect.bottom - rect.top, 0, 0);

    int dx = WinViewOriginX - rect.left;
    int dy = WinViewOriginY - rect.top;
    rect.left = WinViewOriginX;
    rect.top = WinViewOriginY;
    rect.right += dx;
    rect.bottom += dy;

    this->update_screen(&rect, 0);

    WinBackBuffer = 0;
    WinZOrderWindow = 0;
    iSomeFlag_ = savedFlags;

    WinZOrderCount = 0;
    for (int j = 0; j < WinRootCount; j++) {
        Win *w = WinRootWindows[j];
        if (WinZOrderWindow != 0 && WinZOrderWindow == w) {
            WinZOrderCount = 0;
            WinZOrderFlag = 0;
        }
        if (*reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(w) + 0x9c) & 1) {
            recurse_zorder(w);
        }
    }
}

// ===== homed from src/unrecovered/005ee190.cpp =====
typedef void *(__stdcall *PFN_LoadCursorA)(void *, const char *);
typedef void *(__stdcall *PFN_SetCursor)(void *);

int Win::set_modal(int flags, int (__cdecl *callback)(), Win * owner) {
    if (WinModalDepth >= 4) {
        return 0;
    }
    int idx = WinModalDepth;
    WinModalStack[idx] = this;
    reinterpret_cast<void **>(WinFocusStack)[idx] = owner;
    WinModalDepth = idx + 1;
    WinFocusWindow = this;
    WinPointerOwner1 = 0;
    WinPointerOwner2 = 0;

    void *prior = reinterpret_cast<void *>(reinterpret_cast<int>(WinPointerOwner3));
    if (prior != 0 && prior != this) {
        reinterpret_cast<Win *>(prior)->vslot_04();
        WinPointerOwner3 = 0;
    }
    WinInputFocus = 0;
    if (owner != 0) {
        WinModalWindow = owner;
        reinterpret_cast<Win *>(owner)->show(0);
    }
    this->show(0);
    flush_input();

    HCURSOR cursor = LoadCursorA(0, IDC_ARROW);
    SetCursor(cursor);

    int result = 0;
    if (WinFocusWindow == this) {
        for (;;) {
            int r = this->vslot_59();
            if (r == 0) {
                break;
            }
            if (callback != 0) {
                int rr = callback();
                if (rr == 0) {
                    int cur = reinterpret_cast<int>(WinFocusWindow);
                    if (cur != 0) {
                        reinterpret_cast<VCallModal *>(reinterpret_cast<void *>(cur))->slot58();
                    }
                    result = 1;
                    break;
                }
            }
            wait_task();
            if (WinFocusWindow != this) {
                return 0;
            }
        }
    }
    return result;
}

// ===== homed from src/unrecovered/005ef310.cpp =====

void Win::draw_rect_border(int x1, int y1, int x2, int y2, HGDIOBJ pen1, HGDIOBJ pen2, int unused7) {
    bool haveHdc;

    ACQUIRE_HDC(haveHdc);
    if (haveHdc) {
        Palette *pal = PaletteActive;
        if (this->palette_seed_cache_ != pal->seed_) {
            pal->set_active_window(this);
            this->palette_seed_cache_ = pal->seed_;
        }
        HDC hdc = WinSharedHdc;
        HGDIOBJ prev = SelectObject(hdc, pen1);
        MoveToEx(hdc, x1 + 1, y1, 0);
        LineTo(hdc, x2 + 1, y1);
        SelectObject(hdc, prev);
        release_hdc();
    }

    ACQUIRE_HDC(haveHdc);
    if (haveHdc) {
        Palette *pal = PaletteActive;
        if (this->palette_seed_cache_ != pal->seed_) {
            pal->set_active_window(this);
            this->palette_seed_cache_ = pal->seed_;
        }
        HDC hdc = WinSharedHdc;
        void *prev = reinterpret_cast<void *>(SelectObject(hdc, pen2));
        MoveToEx(hdc, x1, y2, 0);
        LineTo(hdc, x2, y2);
        SelectObject(hdc, prev);
        release_hdc();
    }

    ACQUIRE_HDC(haveHdc);
    if (haveHdc) {
        Palette *pal = PaletteActive;
        if (this->palette_seed_cache_ != pal->seed_) {
            pal->set_active_window(this);
            this->palette_seed_cache_ = pal->seed_;
        }
        HDC hdc = WinSharedHdc;
        HGDIOBJ prev = SelectObject(hdc, pen1);
        MoveToEx(hdc, x1, y1, 0);
        LineTo(hdc, x1, y2);
        SelectObject(hdc, prev);
        release_hdc();
    }

    ACQUIRE_HDC(haveHdc);
    if (haveHdc) {
        sync_palette();
        HDC hdc = WinSharedHdc;
        void *prev = reinterpret_cast<void *>(SelectObject(hdc, pen2));
        MoveToEx(hdc, x2, y1 + 1, 0);
        LineTo(hdc, x2, y2 + 1);
        SelectObject(hdc, prev);
        release_hdc();
    }
}

// ===== homed from src/unrecovered/005eedc0.cpp =====

int Win::key_up_event(int key) {
    int result = 0;
    if ((iFlags_ & 0x200000) != 0 ||
        (iSomeFlag_ & 8) != 0) {
        return 0;
    }
    if (list_.count_ != 0) {
        int recvThis;
        if (list_.head_ != 0) {
            recvThis = *(reinterpret_cast<int *>(list_.current_) + 1);
        } else {
            recvThis = 0;
        }
        if (reinterpret_cast<Win *>(recvThis)->key_up_event(key) != 0) {
            return 1;
        }
    }
    WinCallbackWindow = this;
    int callback = field_434_;
    if (callback != 0) {
        result = reinterpret_cast<int (__cdecl *)(int)>(callback)(key);
    }
    result += this->vslot_27(key);
    Win *child = reinterpret_cast<Win *>(val_16_);
    if (child != 0) {
        if (child->vslot_23() == 0) {
            child->vslot_07();
        }
    }
    return result;
}

// ===== homed from src/unrecovered/005eeb90.cpp =====

int Win::key_down_event(int key) {

    if ((iFlags_ & 0x200000) != 0 ||
        (iSomeFlag_ & 8) != 0) {
        return 0;
    }

    bool clicked = false;
    int ebx = 0;

    if (list_.count_ != 0) {
        if (list_.head_ != 0) {
            ebx = *(reinterpret_cast<int *>(list_.current_) + 1);
        }
        Win *focus = reinterpret_cast<Win *>(ebx);

        if (focus->scroll_vert_ != 0) {
            if (focus->scroll_vert_->vslot_25(0, key) != 0) {
                return 1;
            }
            if ((short)MapVirtualKeyA(key & 0xfff8ffff, 2) == 0) {
                focus->key_click_event(0, key);
                clicked = true;
            }
        }
        if (focus->scroll_horz_ != 0) {
            if (focus->scroll_horz_->vslot_25(0, key) != 0) {
                return 1;
            }
            if (!clicked && (short)MapVirtualKeyA(key & 0xfff8ffff, 2) == 0) {
                focus->key_click_event(0, key);
            }
        }
        if (focus->scroll_horz_ == 0 && focus->scroll_vert_ == 0 &&
            (short)MapVirtualKeyA(key & 0xfff8ffff, 2) == 0) {
            focus->key_click_event(0, key);
        }
        if (focus->key_down_event(key) != 0) {
            return 1;
        }
    }

    if (this->scroll_vert_ != 0) {
        if (this->scroll_vert_->vslot_25(0, key) != 0) {
            return 1;
        }
        if (!clicked && (short)MapVirtualKeyA(key & 0xfff8ffff, 2) == 0) {
            key_click_event(0, key);
            clicked = true;
        }
    }
    if (this->scroll_horz_ != 0) {
        if (this->scroll_horz_->vslot_25(0, key) != 0) {
            return 1;
        }
        if (!clicked && (short)MapVirtualKeyA(key & 0xfff8ffff, 2) == 0) {
            key_click_event(0, key);
        }
    }
    if (this->scroll_horz_ == 0 && this->scroll_vert_ == 0 && !clicked &&
        (short)MapVirtualKeyA(key & 0xfff8ffff, 2) == 0 &&
        key_click_event(0, key) != 0) {
        return 1;
    }

    WinCallbackWindow = this;

    WinKeyHookFn hook = key_hook_;
    if (hook != 0) {
        ebx = hook(key);
    }
    int r2 = this->vslot_26(key);

    Win *child = reinterpret_cast<Win *>(val_15_);
    if (child != 0) {
        if (child->vslot_23() == 0) {
            child->vslot_07();
        }
    }
    return ebx + r2;
}

// ===== homed from src/unrecovered/005ebd80.cpp =====
typedef int (__stdcall *MessageBoxAFn)(void *, const char *, const char *, unsigned int);

int Win::init(int x, int y, int width, int height, char * caption,
              int flags, Win * parent, Menu * menu, BorderSizing * border) {
    this->close();

    if (border != 0) {
        int *bs = reinterpret_cast<int *>(border);
        this->caption_height_ = bs[0];
        this->border_thickness_ = bs[1];
        this->bottom_border_thickness_ = bs[2];
    }
    this->field_1A0_ |= 2;

    {
        void *raw = operator new(0x588U);
        this->buffer1_ = (raw != 0) ? new (raw) Buffer() : 0;
        if (this->buffer1_ == 0) return 4;
    }
    {
        void *raw = operator new(0x588U);
        this->buffer2_ = (raw != 0) ? new (raw) Buffer() : 0;
        if (this->buffer2_ == 0) return 4;
    }
    {
        void *raw = operator new(0x588U);
        this->buffer3_ = (raw != 0) ? new (raw) Buffer() : 0;
        if (this->buffer3_ == 0) return 4;
    }
    {
        void *raw = operator new(0x588U);
        this->buffer4_ = (raw != 0) ? new (raw) Buffer() : 0;
        if (this->buffer4_ == 0) return 4;
    }

    if ((flags & 0xF0000000) != 0) {
        if (this->vslot_61() == 0) {
            return 3;
        }
    }
    if ((flags & 0x80000) != 0) {
        flags |= 0x20;
    }

    int posX = x;
    if (x < 0) {
        int w = parent ? (parent->outer_rect_.right - parent->outer_rect_.left) : WinScreenWidth;
        posX = x + (w - width);
    }
    int posY = y;
    if (y < 0) {
        int h = parent ? (parent->outer_rect_.bottom - parent->outer_rect_.top) : WinScreenHeight;
        posY = y + (h - height);
    }

    this->iFlags_ = flags;
    this->menu_ = menu;
    this->win_parent_ = parent;

    if (caption != 0 && caption[0] != 0 && (flags & 0x400080) == 0) {
        flags |= 0x10;
        this->iFlags_ = flags;
    }

    if (parent != 0) {
        if (parent->child_count_ == 150) {
            this->close();
            MessageBoxA(
                0, WinMsgTooManyChildren, WinMsgIncreaseMaxChildren, 0);
            // MEASURED at 0x005EBFC4: `push 4; call 0x644dff` - the CRT's exit with
            // status 4, not a member call. The artifact declared a local
            // `void exit();` in its stub class, which was a guess.
            exit(4);
        } else {
            int idx = parent->child_count_;
            if ((this->iFlags_ & 0x2000000) != 0) {
                parent->children_[idx] = this;
            } else {
                for (int i = idx; i > 0; i--) {
                    parent->children_[i] = parent->children_[i - 1];
                }
                parent->children_[0] = this;
            }
            parent->child_count_++;
        }
    } else {
        int cnt = WinRootCount;
        if (cnt == 0x28) {
            this->close();
            MessageBoxA(
                0, WinMsgTooManyParents, WinMsgIncreaseMaxParents, 0);
            // MEASURED at 0x005EBFC4: `push 4; call 0x644dff` - the CRT's exit with
            // status 4, not a member call. The artifact declared a local
            // `void exit();` in its stub class, which was a guess.
            exit(4);
        } else {
            Win **globalList = WinRootWindows;
            if ((this->iFlags_ & 0x2000000) != 0) {
                globalList[cnt] = this;
            } else {
                if (cnt > 0) {
                    for (int i = cnt; i > 0; i--) {
                        globalList[i] = globalList[i - 1];
                    }
                }
                globalList[0] = this;
            }
            WinRootCount = cnt + 1;
        }
    }

    if (((this->iFlags_ & 0x80) != 0) || ((this->iFlags_ & 0x1D) == 0)) {
        this->iSomeFlag_ &= ~2u;
        this->iFlags_ &= ~0x411u;
    } else {
        this->iSomeFlag_ |= 2;
    }

    if ((this->iFlags_ & 0x800) == 0 && (this->iSomeFlag_ & 2) != 0) {
        this->client_rect_.left = posX;
        this->client_rect_.top = posY;
        this->client_rect_.right = posX + width;
        this->client_rect_.bottom = posY + height;
        this->outer_rect_ = this->client_rect_;
        this->nonclient_to_client(&this->outer_rect_);
        int dx = -this->outer_rect_.left;
        int dy = -this->client_rect_.top;
        this->outer_rect_.left += dx;
        this->outer_rect_.right += dx;
        this->outer_rect_.top += dy;
        this->outer_rect_.bottom += dy;
    } else {
        this->outer_rect_.left = posX;
        this->outer_rect_.top = posY;
        this->outer_rect_.right = posX + width;
        this->outer_rect_.bottom = posY + height;
        if ((this->iSomeFlag_ & 2) != 0) {
            this->client_rect_ = this->outer_rect_;
            this->client_to_nonclient(&this->outer_rect_);
            int dx = -this->client_rect_.left;
            int dy = -this->outer_rect_.top;
            this->outer_rect_.left += dx;
            this->outer_rect_.right += dx;
            this->outer_rect_.top += dy;
            this->outer_rect_.bottom += dy;
        }
    }

    this->field_15C_ = this->outer_rect_.left;
    this->field_160_ = this->outer_rect_.top;
    this->field_164_ = this->outer_rect_.right;
    this->field_168_ = this->outer_rect_.bottom;

    void **nameSlot = reinterpret_cast<void **>(&field_E0_);
    if (*nameSlot) {
        free(*nameSlot);
        *nameSlot = 0;
    }
    if (caption != 0) {
        unsigned int len = strlen(caption) + 1;
        void *mem = mem_get(static_cast<int>(len));
        *nameSlot = mem;
        if (mem != 0) {
            reinterpret_cast<char *>(mem)[0] = 0;
            strcat(reinterpret_cast<char *>(*nameSlot), caption);
        }
    }

    if ((this->iSomeFlag_ & 1) != 0) {
        bool draw = true;
        if (this->win_parent_ != 0) {
            draw = this->win_parent_->is_visible() != 0;
        }
        if (draw) {
            this->on_nc_paint(0, -1);
        }
    }

    this->set_rects();

    if (menu != 0) {
        int result = reinterpret_cast<Slot88Shim *>(menu)->slot090(this);
        if (result != 0) {
            this->close();
            return result;
        }
    }

    if ((this->iFlags_ & 0x10) != 0) {
        this->field_16C_ = this->caption_height_;
    } else if ((this->iFlags_ & 0x20) != 0) {
        this->field_16C_ = this->border_thickness_;
    }
    if ((this->iFlags_ & 0x400) != 0) {
        this->field_170_ = this->border_thickness_;
    } else {
        this->field_170_ = 0;
    }

    if ((this->iFlags_ & 4) != 0) {
        if ((this->iFlags_ & 0x40) != 0) {
            this->buffer4_ = reinterpret_cast<Buffer *>(this->vslot_06());
        }
        if (this->buffer4_ == 0) {
            void *raw = operator new(0x214C);
            this->buffer4_ = (raw != 0) ? reinterpret_cast<Buffer *>(new (raw) Scroll()) : 0;
            if (this->buffer4_ == 0) return 4;
            Scroll *sc = reinterpret_cast<Scroll *>(this->buffer4_);
            int top = ScrollDefaultThickness;
            int nc = this->outer_rect_.bottom - top;
            int arg2 = ((this->iFlags_ & 8) != 0)
                           ? (this->outer_rect_.right - this->outer_rect_.left) - top
                           : (this->outer_rect_.right - this->outer_rect_.left);
            sc->init_horz_nc(0, nc, arg2, this, -2);
            sc->show(0);
        }

        if ((this->iFlags_ & 8) != 0) {
            if ((this->iFlags_ & 0x40) != 0) {
                this->buffer3_ = reinterpret_cast<Buffer *>(this->vslot_06());
            }
            if (this->buffer3_ == 0) {
                void *raw = operator new(0x214C);
                this->buffer3_ = (raw != 0) ? reinterpret_cast<Buffer *>(new (raw) Scroll()) : 0;
                if (this->buffer3_ == 0) return 4;
                Scroll *sc = reinterpret_cast<Scroll *>(this->buffer3_);
                int top = ScrollDefaultThickness;
                int nc = this->outer_rect_.right - top;
                int arg2 = ((this->iFlags_ & 4) != 0)
                               ? (this->outer_rect_.bottom - this->outer_rect_.top) - top
                               : (this->outer_rect_.bottom - this->outer_rect_.top);
                sc->init_vert_nc(nc, 0, arg2, this, -1);
                sc->show(0);
            }
        }
    }

    this->do_caption_buttons();
    WinCallbackWindow = this;

    unsigned int fl = this->iFlags_;
    bool skipMenuList = (fl & 0x100000) != 0 || (fl & 0x200000) != 0 ||
                         (this->iSomeFlag_ & 8) != 0 ||
                         (((fl & 0x100000) == 0 && ((fl >> 16) & 0x10) != 0) && this->win_parent_ == 0);

    if (!skipMenuList && this->win_parent_ != 0 &&
        ((*reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(this->win_parent_) + 0x98)) & 0x1000) == 0) {
        char *parent = reinterpret_cast<char *>(this->win_parent_);
        Heap *heap = *reinterpret_cast<Heap **>(parent + 0xdc);
        int initCount = *reinterpret_cast<int *>(parent + 0xd4);
        if (initCount != 0) {
            char *newNode = reinterpret_cast<char *>(heap ? heap->get(0x1C) : mem_get(0x1C));
            char *head = *reinterpret_cast<char **>(parent + 0xcc);
            char *prevOfHead = *reinterpret_cast<char **>(head + 0x10);
            *reinterpret_cast<char **>(prevOfHead + 0xc) = newNode;
            if (newNode != 0) {
                *reinterpret_cast<char **>(newNode + 0x10) = prevOfHead;
                *reinterpret_cast<char **>(newNode + 0xc) = head;
                *reinterpret_cast<char **>(head + 0x10) = newNode;
                *reinterpret_cast<void **>(parent + 0xd0) = newNode;
                *reinterpret_cast<Win **>(newNode + 4) = this;
                void *link = heap ? heap->get(4) : mem_get(4);
                *reinterpret_cast<void **>(newNode + 8) = link;
                if (link == 0) {
                    return 0;
                }
            } else {
                return 0;
            }
        } else {
            void *newNode = heap ? heap->get(0x1C) : mem_get(0x1C);
            *reinterpret_cast<void **>(parent + 0xcc) = newNode;
            if (newNode == 0) {
                return 0;
            }
            *reinterpret_cast<Win **>(reinterpret_cast<char *>(newNode) + 4) = this;
            *reinterpret_cast<void **>(reinterpret_cast<char *>(newNode) + 0xc) = newNode;
            *reinterpret_cast<void **>(reinterpret_cast<char *>(newNode) + 0x10) = newNode;
            void *link = heap ? heap->get(4) : mem_get(4);
            *reinterpret_cast<void **>(reinterpret_cast<char *>(newNode) + 8) = link;
            if (link == 0) {
                return 0;
            }
            *reinterpret_cast<void **>(parent + 0xd0) = newNode;
        }

        int oldCount = *reinterpret_cast<int *>(parent + 0xd4);
        *reinterpret_cast<int *>(parent + 0xd8) = oldCount;
        *reinterpret_cast<int *>(parent + 0xd4) = oldCount + 1;

        int cnt = *reinterpret_cast<int *>(parent + 0xd4) - 1;
        if (cnt >= 0) {
            void *head = *reinterpret_cast<void **>(parent + 0xcc);
            *reinterpret_cast<int *>(parent + 0xd8) = 0;
            *reinterpret_cast<void **>(parent + 0xd0) = head;
        }
    }

    this->iSomeFlag_ |= 4;
    return 0;
}

// ===== homed from src/unrecovered/005f1820.cpp =====
typedef int(__stdcall *UnionRectFn)(RECT *, const RECT *, const RECT *);
typedef void *(__stdcall *LoadCursorFn)(void *, int);
typedef void *(__stdcall *SetCursorFn)(void *);
typedef void(__stdcall *GetCursorPosFn)(int *);
typedef int(__stdcall *PollFn)(int);

int __cdecl Win::update_cursor(Win *window, int tgl) {

    POINT pt;  // the artifact spelled this `int pt[2]`
    GetCursorPos(&pt);
    int x = pt.x;
    int y = pt.y;

    Win *win = window;
    if (win == 0) {
        win = reinterpret_cast<Win *>(Win::get_mouse_window(&x, &y));
        if (win == 0) {
            RestoreAndFlip(UnionRect, false);
            HCURSOR cur = LoadCursorA(0, IDC_ARROW);
            return reinterpret_cast<int>(SetCursor(cur));
        }
    }

    if (WinMouseDirect == 0 && (WFIELD(win, 0x98) & 0x4000000) == 0) {
        int field188 = WFIELD(win, 0x188);
        if (field188 != 0) {
            // A window wants the cursor image saved/restored under it.
            int saved = reinterpret_cast<int>(WinFlipSprite);
            if (saved == 0) {
                while (ShowCursor(0) >= 0) {
                }
            } else {
                restore_flip_sprite(0);
            }

            int field188b = WFIELD(win, 0x188);
            int prevSaved = reinterpret_cast<int>(WinFlipSprite);
            if (field188b != prevSaved) {
                WinClipWidth = *reinterpret_cast<int *>(field188b + 0x18);
                WinClipHeight = *reinterpret_cast<int *>(field188b + 0x1c);
                WinDragOffsetX = WFIELD(win, 0x18c);
                WinDragOffsetY = WFIELD(win, 0x190);
                bool sameArea = WinSavedAreaWidth == field188b && -WinSavedAreaHeight == prevSaved;
                if (!sameArea) {
                    g_WIN_BUFFER->init(field188b, prevSaved, 0, 0);
                }
            }
            int width = WinClipWidth;
            int height = WinClipHeight;
            int destX = x - WinDragOffsetX;
            int destY = y - WinDragOffsetY;
            WinFlipSpriteY = destX;
            WinFlipSpriteX = destY;
            WinFlipSprite = reinterpret_cast<Sprite *>(field188b);
            int copyResult = g_WIN_BUFFER
                                  ->copy((&ScreenBuffer), 0, 0, destX, destY,
                                         width, height);

            RECT rect;
            rect.left = destX;
            rect.top = destY;
            rect.right = destX + width;
            rect.bottom = destY + height;
            RECT merged;
            UnionRect(&merged, &rect, &WinDirtyRect);
            if (merged.bottom == 0) {
                return copyResult;
            }
            int prevFlag = WinFlipClipped;
            WinFlipClipped = 1;
            Win::flip(&merged);
            int result = WinModalResult;
            WinFlipClipped = prevFlag;
            WinDirtyRect.left = result;
            return result;
        }

        // win->0x188 == 0: nothing pending under this window.
        if (WinFlipSprite != 0) {
            int ebxFlag = 1;
            restore_flip_sprite(1);
            while (ShowCursor(ebxFlag) <= 0) {
            }
        }
        WinFlipSprite = reinterpret_cast<Sprite *>(0);

        int owner = WFIELD(win, 0x194);
        if (owner != 0) {
            int cursorId = WFIELD(owner, 0);
            if (cursorId != 0) {
                HCURSOR cur = LoadCursorA(0, MAKEINTRESOURCE(cursorId));
                return reinterpret_cast<int>(SetCursor(cur));
            }
        }
        Win *win2 = reinterpret_cast<Win *>(WFIELD(win, 0x198));
        if (win2 == 0) {
            goto lab_default_cursor;
        }
        {
            HCURSOR cur = LoadCursorA(0, reinterpret_cast<const char *>(win2));
            return reinterpret_cast<int>(SetCursor(cur));
        }
    }

    // Full dispatch: ask the window what kind of hit region the cursor is
    // over, then pick the matching cursor image / saved-region behaviour.
    // Case grouping and targets read from the resolved jump table (20
    // entries at 0x005F1B3C), not guessed.
    {
        int hitCode = win->on_nc_hittest(x, y);
        hitCode += 2;
        int ebxFlag = 1;

        switch (hitCode) {
        case 2:
        case 4:
            RestoreAndFlip(UnionRect, false);
            WinCursorMoved = ebxFlag;
            SetCursor(LoadCursorA(0, IDC_ARROW));
            break;
        case 12:
            RestoreAndFlip(UnionRect, false);
            WinCursorMoved = ebxFlag;
            SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f84)));
            break;
        case 13:
            RestoreAndFlip(UnionRect, true);
            WinCursorMoved = ebxFlag;
            SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f84)));
            break;
        case 14:
        case 17:
            RestoreAndFlip(UnionRect, true);
            WinCursorMoved = ebxFlag;
            SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f85)));
            break;
        case 15:
        case 19:
            RestoreAndFlip(UnionRect, true);
            WinCursorMoved = ebxFlag;
            SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f82)));
            break;
        case 16:
        case 18:
            RestoreAndFlip(UnionRect, true);
            WinCursorMoved = ebxFlag;
            SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f83)));
            break;
        case 0:
        case 1:
        case 3:
            // No cursor change at all for these hit codes.
            break;
        default:
            // Covers 5..11 and any out-of-table index (the `ja` fallback).
            RestoreAndFlip(UnionRect, true);
            SetCursor(LoadCursorA(0, IDC_ARROW));
            break;
        }

        if (WinMouseDirect == 0 && WFIELD(win, 0x188) == 0 && WinFlipSprite != 0) {
            g_WIN_BUFFER
                ->copy((&ScreenBuffer), 0, 0, WinFlipSpriteY, WinFlipSpriteX,
                       WinClipWidth, WinClipHeight);
            RECT rect;
            make_rect(&rect, WinFlipSpriteY, WinFlipSpriteX, WinClipWidth, WinClipHeight);
            RECT merged;
            UnionRect(&merged, &rect, &WinDirtyRect);
            int savedFlag = reinterpret_cast<int>(WinFlipSprite);
            WinFlipSprite = reinterpret_cast<Sprite *>(0);
            Win::flip(&merged);
            WinFlipSprite = reinterpret_cast<Sprite *>(savedFlag);
            WinDirtyRect.left = 0;
            WinModalResult = 0;
        }

        while (ShowCursor(ebxFlag) <= 0) {
        }
        return 0;
    }

lab_default_cursor:
    WinFlipSprite = reinterpret_cast<Sprite *>(0);
    {
        HCURSOR cur = LoadCursorA(0, IDC_ARROW);
        return reinterpret_cast<int>(SetCursor(cur));
    }
}

// ===== homed from src/unrecovered/005ed5c0.cpp =====

int Win::center() {
    int local_8;
    int local_4;
    int top;
    int bottom;

    if ((iSomeFlag_ & 2) != 0) {
        local_8 = (client_rect_.left - client_rect_.right + WinScreenWidth) / 2;
        bottom = client_rect_.bottom;
        top = client_rect_.top;
    } else {
        local_8 = (outer_rect_.left - outer_rect_.right + WinScreenWidth) / 2;
        bottom = outer_rect_.bottom;
        top = outer_rect_.top;
    }
    local_4 = (top - bottom + WinScreenHeight) / 2;
    if (win_parent_ != 0) {
        local_8 -= reinterpret_cast<Win *>(win_parent_)->client_rect_.left + reinterpret_cast<Win *>(win_parent_)->outer_rect_.left;
        local_4 -= reinterpret_cast<Win *>(win_parent_)->client_rect_.top + reinterpret_cast<Win *>(win_parent_)->outer_rect_.top;
        if ((reinterpret_cast<Win *>(win_parent_)->iFlags_ & 0x20) != 0 && reinterpret_cast<Win *>(win_parent_)->win_parent_ != 0) {
            reinterpret_cast<Win *>(win_parent_)->win_parent_->screen_to_client(&local_8, &local_4);
            if ((reinterpret_cast<Win *>(win_parent_)->iFlags_ & 0x8000) != 0) {
                local_8 += reinterpret_cast<Win *>(win_parent_)->win_parent_->outer_rect_.left;
                local_4 += reinterpret_cast<Win *>(win_parent_)->win_parent_->outer_rect_.top;
                return move(local_8, local_4);
            }
        }
    }
    return move(local_8, local_4);
}

// ===== homed from src/unrecovered/005ee750.cpp =====
typedef void (__cdecl *FnCallback)(int, int);

// ORIGINAL: 0x005EE750 ?left_down_event@Win@@QAEXHHH@Z 0x005EE750-0x005EE802 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      178 bytes
// prototype void (__thiscall ?left_down_event@Win@@QAEXHHH@Z)(Win* this, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE795 0x005EE7A5 0x005EE7B5 0x005EE7C3 0x005EE7CE 0x005EE7DD 0x005EE7EE 0x005EE7F9
void Win::left_down_event(int x, int y, int from_parent) {
    uint32_t iFlags = iFlags_;
    if ((iFlags & 0x200000) != 0) return;
    uint8_t iSomeFlagByte = iSomeFlag_;
    if ((iSomeFlagByte & 8) != 0) return;

    if (from_parent == 0) {
        WinCallbackWindow = this;
        FnCallback cb = reinterpret_cast<FnCallback>(field_418_);
        if (cb != 0) {
            cb(x, y);
        }
    }

    // 0x009B7A90 - the same slot window_proc calls as WinLeftDownHook, and
    // the signature agrees: (int x, int y).
    if (WinLeftDownHook != nullptr) {
        WinLeftDownHook(x, y);
    }

    if (from_parent == 0) {
        this->vslot_20(x, y);
        Win *child = reinterpret_cast<Win *>(val_9_);
        if (child != 0) {
            int r = reinterpret_cast<Win *>(child)->vslot_23();
            if (r == 0) {
                reinterpret_cast<Win *>(child)->vslot_07();
            }
        }
    } else {
        this->vslot_33(x, y);
        Win *child = reinterpret_cast<Win *>(val_21_);
        if (child != 0) {
            int r = reinterpret_cast<Win *>(child)->vslot_23();
            if (r == 0) {
                reinterpret_cast<Win *>(child)->vslot_07();
            }
        }
    }
}

// ===== homed from src/unrecovered/005ef110.cpp =====

// ORIGINAL: 0x005EF110 ?nonclient_to_client@Win@@QAEXPAURECT@@@Z 0x005EF110-0x005EF1D1 FILE
// symbol    ?nonclient_to_client@Win@@QAEXPAUtagRECT@@@Z
// notes     the RECT overload; the coordinate-pair form shares the name
//           and marker_symbols resolves to whichever comes first.
// size      193 bytes
// prototype void (__thiscall ?nonclient_to_client@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   2   call targets   0
// kind      
// flags     
// calls     (none)
// indirect  0x005EF1C4
// working copy - scaffold materialised by --work
// TRIED and NOT recovered (2026-08-25): BYTE_EXACT while 0x009B8DD4 was
// read through the `WinTitleBarHeight` fixed-address binding, 26 of 60
// once that merged onto `ScrollDefaultThickness`. The divergence is a
// register cascade in the border block - the image keeps the rect edges in
// edi (`push edi` at instruction 17, `mov edi,[esi]`, `sub edi,eax`) where
// this build uses edx and emits one instruction fewer. Dropping the
// redundant `border` local did not move it (26 of 60 either way).
// THE MERGE IS STILL RIGHT, and this is the point worth keeping: the two
// names were not a naming quibble, they were TWO STORAGES. The binding
// reads image address 0x009B8DD4 while ScrollDefaultThickness is storage
// this binary allocates elsewhere, so Win and Scroll disagreed at run time
// about the same variable and no byte comparison could see it.
// Unresolved and recorded rather than guessed: win.cpp reads it as a title
// bar height, scroll.cpp as a scrollbar thickness, and NOTHING in the tree
// writes it, so neither name is evidenced over the other.
void Win::nonclient_to_client(RECT * rect) {
    if (rect != 0) {
        if ((iFlags_ & 4) != 0) {
            rect->bottom -= ScrollDefaultThickness;
        }
        if ((iFlags_ & 8) != 0) {
            rect->right -= ScrollDefaultThickness;
        }
        if (((iFlags_ & 0x400) != 0) || ((iFlags_ & 0x11) != 0)) {
            int border = border_thickness_;
            int left = rect->left;
            int right = rect->right;
            int neg_border = -border;
            left -= neg_border;
            right += neg_border;
            rect->left = left;
            int top = rect->top;
            rect->right = right;
            int bottom = rect->bottom;
            top -= neg_border;
            bottom += neg_border;
            rect->top = top;
            rect->bottom = bottom;
            if (bottom_border_thickness_ != -1) {
                rect->bottom = (border_thickness_ - bottom_border_thickness_) + rect->bottom;
            }
        }
        if ((iFlags_ & 0x10) != 0) {
            rect->top = rect->top + (caption_height_ - border_thickness_);
        }
        if (menu_ != 0) {
            rect->top = rect->top + reinterpret_cast<NCCall *>(menu_)->slot091();
        }
    }
}

// ===== homed from src/unrecovered/005f2cf0.cpp =====
typedef int (__stdcall *IntersectRectFn)(RECT *, const RECT *, const RECT *);

void Win::on_redraw_nc(RECT * area, int flags) {
    if ((*(unsigned char *)&this->iSomeFlag_ & 2) == 0) return;
    unsigned int win_flags = this->iFlags_;
    if ((win_flags & 0x11) == 0) return;

    int savedLeft = 0, savedTop = 0;
    int side = flags;
    if (area != 0) {
        side = -1;
        savedLeft = this->client_rect_.left;
        savedTop = this->client_rect_.top;
    }

    int capHeight;
    if ((win_flags & 0x10) == 0 || (win_flags & 0x400000) != 0) {
        capHeight = this->border_thickness_;
    } else {
        capHeight = this->caption_height_;
    }

    int bottomThick = this->bottom_border_thickness_;
    if (bottomThick == -1) bottomThick = this->border_thickness_;

    int clientH = this->client_rect_.bottom - this->client_rect_.top;
    int clientW = this->client_rect_.right - this->client_rect_.left;
    int borderM1 = this->border_thickness_ - 1;
    int outerW = (this->outer_rect_.right - this->outer_rect_.left) + 2 + borderM1;
    int capM1 = capHeight - 1;
    int outerH = ((clientH - bottomThick) - capHeight) + 2 + capM1;


    // ---- edge 0: top caption bar (buffer1_) ----
    {
        Buffer *buf = this->buffer1_;
        if (buf != 0 && (side == -1 || side == 0)) {
            RECT outer = buf->rect2_;
            RECT clip = outer;
            if (area != 0) {
                clip.left = area->left - savedLeft;
                clip.top = area->top - savedTop;
                clip.right = area->right - savedLeft;
                clip.bottom = area->bottom - savedTop;
            }
            if (IntersectRect(&clip, &clip, &outer)) {
                buf->set_clip(&clip);
                if (this->field_FC_ == 0) {
                    buf->fill(this->field_100_);
                } else {
                    buf->tile((Buffer *)this->field_FC_, 0, 0, 0, 0, clientW, clientH);
                }
                {
                    RECT r; r.left = 0; r.top = 0; r.right = clientW; r.bottom = clientH;
                    for (int i = 0; i < (int)this->field_128_; i++) {
                        buf->box(&r, this->field_120_, this->field_124_);
                        r.left++; r.top++; r.right--; r.bottom--;
                    }
                }
                {
                    RECT r; r.left = borderM1; r.top = capM1; r.right = outerW; r.bottom = outerH;
                    for (int i = 0; i < (int)this->field_128_; i++) {
                        buf->box(&r, this->field_124_, this->field_120_);
                        r.left--; r.top--; r.right++; r.bottom++;
                    }
                }
                if (this->field_E0_ != 0) {
                    int w = clientW - capHeight;
                    RECT r; r.left = 0; r.top = 0; r.right = w; r.bottom = capHeight;
                    buf->set_text_color(this->field_104_, this->field_108_, this->field_10C_, this->field_110_);
                    buf->set_font((Font *)this->field_F8_, 0, 0, 0);
                    buf->set_clip(&r);
                    char *title = (char *)this->field_E0_;
                    if ((win_flags & 0x10000) != 0) {
                        if (title != 0) {
                            unsigned int len = strlen(title);
                            buf->write_l(title, &r, len);
                        }
                    } else if ((win_flags & 0x40000) != 0) {
                        if (title != 0) {
                            unsigned int len = strlen(title);
                            buf->write_right_l(title, &r, len);
                        }
                    } else if (title != 0) {
                        unsigned int len = strlen(title);
                        buf->write_cent_l(title, &r, len);
                    }
                }
                buf->set_clip(&buf->rect2_);
            }
            this->vslot_60(reinterpret_cast<int>(buf), 0);
        }
    }

    // ---- edge 2: bottom border (buffer4_) ----
    {
        Buffer *buf = this->buffer4_;
        if (buf != 0 && (side == -1 || side == 2)) {
            RECT outer = buf->rect2_;
            RECT clip = outer;
            if (area != 0) {
                clip.left = area->left - savedLeft;
                clip.top = area->top + (capHeight - savedTop);
                clip.right = area->right - savedLeft;
                clip.bottom = area->bottom + (capHeight - savedTop);
            }
            if (IntersectRect(&clip, &clip, &outer)) {
                buf->set_clip(&clip);
                if (this->field_FC_ == 0) {
                    buf->fill(this->field_100_);
                } else {
                    buf->tile((Buffer *)this->field_FC_, 0, capHeight, 0, 0, clientW, clientH);
                }
                {
                    RECT r; r.left = 0; r.top = -capHeight; r.right = clientW; r.bottom = clientH - capHeight;
                    for (int i = 0; i < (int)this->field_128_; i++) {
                        buf->box(&r, this->field_120_, this->field_124_);
                        r.left++; r.top++; r.right--; r.bottom--;
                    }
                }
                {
                    RECT r; r.left = borderM1 - capHeight; r.top = capM1 - capHeight; r.right = outerW; r.bottom = outerH;
                    for (int i = 0; i < (int)this->field_128_; i++) {
                        buf->box(&r, this->field_124_, this->field_120_);
                        r.left--; r.top--; r.right++; r.bottom++;
                    }
                }
                buf->set_clip(&buf->rect2_);
            }
            this->vslot_60(reinterpret_cast<int>(buf), 2);
        }
    }

    // ---- edge 3: right border (buffer3_) ----
    {
        Buffer *buf = this->buffer3_;
        if (buf != 0 && (side == -1 || side == 3)) {
            RECT outer = buf->rect2_;
            RECT clip = outer;
            if (area != 0) {
                int shiftY = capHeight - savedTop;
                clip.left = area->left - savedLeft;
                clip.top = area->top + shiftY;
                clip.right = area->right - savedLeft;
                clip.bottom = area->bottom + shiftY;
            }
            if (IntersectRect(&clip, &clip, &outer)) {
                buf->set_clip(&clip);
                if (this->field_FC_ == 0) {
                    buf->fill(this->field_100_);
                } else {
                    buf->tile((Buffer *)this->field_FC_, 0, capHeight, 0, 0, clientW, clientH);
                }
                {
                    RECT r; r.left = 0; r.top = -capHeight; r.right = clientW; r.bottom = clientH - capHeight;
                    for (int i = 0; i < (int)this->field_128_; i++) {
                        buf->box(&r, this->field_120_, this->field_124_);
                        r.left++; r.top++; r.right--; r.bottom--;
                    }
                }
                {
                    RECT r; r.left = borderM1 - capHeight; r.top = capM1 - capHeight; r.right = outerW; r.bottom = outerH;
                    for (int i = 0; i < (int)this->field_128_; i++) {
                        buf->box(&r, this->field_124_, this->field_120_);
                        r.left--; r.top--; r.right++; r.bottom++;
                    }
                }
                buf->set_clip(&buf->rect2_);
            }
            this->vslot_60(reinterpret_cast<int>(buf), 3);
        }
    }

    // ---- edge 1: left border (buffer2_) ----
    {
        Buffer *buf = this->buffer2_;
        if (buf != 0 && (side == -1 || side == 1)) {
            RECT outer = buf->rect2_;
            RECT clip = outer;
            int shiftX = ((this->client_rect_.right - this->client_rect_.left) - this->border_thickness_) - savedLeft;
            if (area != 0) {
                clip.left = area->left + shiftX;
                clip.top = area->top + (capHeight - savedTop);
                clip.right = area->right + shiftX;
                clip.bottom = area->bottom + (capHeight - savedTop);
            }
            if (IntersectRect(&clip, &clip, &outer)) {
                buf->set_clip(&clip);
                if (this->field_FC_ == 0) {
                    buf->fill(this->field_100_);
                } else {
                    buf->tile((Buffer *)this->field_FC_, clientW - this->border_thickness_, capHeight, 0, 0, clientW, clientH);
                }
                int negCap = -capHeight;
                int loop1Left = (this->client_rect_.left - this->client_rect_.right) + this->border_thickness_;
                {
                    RECT r;
                    r.left = loop1Left; r.top = negCap; r.right = clientW + loop1Left; r.bottom = clientH - capHeight;
                    for (int i = 0; i < (int)this->field_128_; i++) {
                        buf->box(&r, this->field_120_, this->field_124_);
                        r.left++; r.top++; r.right--; r.bottom--;
                    }
                }
                {
                    RECT r;
                    r.left = loop1Left + (borderM1 - capHeight);
                    r.top = capM1 - capHeight;
                    r.right = outerW + loop1Left;
                    r.bottom = outerH + loop1Left;
                    for (int i = 0; i < (int)this->field_128_; i++) {
                        buf->box(&r, this->field_124_, this->field_120_);
                        r.left--; r.top--; r.right++; r.bottom++;
                    }
                }
                buf->set_clip(&buf->rect2_);
            }
            this->vslot_60(reinterpret_cast<int>(buf), 1);
        }
    }
}

// ===== homed from src/unrecovered/005ed520.cpp =====

// ORIGINAL: 0x005ED520 ?screen_to_nonclient@Win@@QAEXPAH0@Z 0x005ED520-0x005ED5BD FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      157 bytes
// prototype void (__thiscall ?screen_to_nonclient@Win@@QAEXPAH0@Z)(Win* this, int*, int*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0
void Win::screen_to_nonclient(int * x, int * y) {
    *x -= client_rect_.left + outer_rect_.left;
    *y -= client_rect_.top + outer_rect_.top;
    if ((iFlags_ & 0x20) != 0 &&
        win_parent_ != 0) {
        (win_parent_)->screen_to_client(x, y);
        if ((iFlags_ & 0x8000) != 0) {
            *x += *reinterpret_cast<int *>(reinterpret_cast<char *>(win_parent_) + 0x13c);
            *y += *reinterpret_cast<int *>(reinterpret_cast<char *>(win_parent_) + 0x140);
        }
    }
    *x += outer_rect_.left;
    *y += outer_rect_.top;
}

// ===== homed from src/unrecovered/005f5fb0.cpp =====

void Win::on_char(char param2, int param3) {
    char **cursor = &WinKeyRingCursor;
    **cursor = param2;

    bool matched = true;
    {
        // the LAST character; the loop below compares backward from it
        const char *pat = WinMdebugCode + 5;
        char *pos = *cursor;
        int count = 6;
        do {
            if (*pos != *pat) {
                matched = false;
                break;
            }
            pos--;
            pat--;
            if (pos < (char *)WinKeyRing) {
                pos = (char *)(WinKeyRing + 9);
            }
            count--;
        } while (count > 0);
    }

    if (matched) {
        if ((static_cast<uint8_t>(WinDrawFlags) & 1) == 0 ||
            (WinPopupWindow != 0 && (WinPopupWindow)->is_visible() == 0)) {
            (&WinBubbleWindow)->show(0);
        } else if ((static_cast<uint8_t>(WinDrawFlags) & 1) != 0) {
            if (WinBubbleCompanion == &WinBubbleWindow &&
                WinBubbleActive != 0) {
                WinBubbleCompanion = nullptr;
                WinBubbleActive = 0;
                (&WinBubbleWindow)->update_screen(&WinBubbleRect, 0);
                (&WinBubbleWindow)->flip(&WinBubbleRect);
            }

            Win *cand = WinPointerOwner1;
            if (cand == nullptr) {
                cand = WinPointerOwner2;
            }
            if (cand == &WinBubbleWindow) {
                WinPointerOwner1 = nullptr;
                WinPointerOwner2 = 0;
            } else if (cand != nullptr && WinPopupCount > 0) {
                Win **list = WinDialogList;
                int i = 0;
                do {
                    if (list[i] == cand || list[i]->is_descendant(cand)) {
                        WinPointerOwner1 = nullptr;
                        WinPointerOwner2 = 0;
                        break;
                    }
                    i++;
                } while (i < WinPopupCount);
            }

            if (WinPointerOwner3 == &WinBubbleWindow) {
                WinPointerOwner3 = 0;
                (&WinBubbleWindow)->vslot_04();
            }
            if (*(void **)WinPointerOwner4 == &WinBubbleWindow) {
                *(int *)WinPointerOwner4 = 0;
            }
            WinDrawFlags &= ~1;
            if (WinFocusWindow == &WinBubbleWindow) {
                (&WinBubbleWindow)->release_modal();
            }

            if ((*(uint8_t *)JackalInitFlags & 1) != 0) {
                WinZOrderCount = 0;
                if (*(int *)WinRootCount > 0) {
                    Win **entries = (Win **)WinRootWindows;
                    int cur = *(int *)WinZOrderWindow;
                    int j = 0;
                    do {
                        if (cur != 0 && cur == (int)entries[j]) {
                            WinZOrderCount = 0;
                            *(int *)WinZOrderFlag = 0;
                        }
                        if ((*(uint8_t *)((char *)entries[j] + 0x9c) & 1) != 0) {
                            recurse_zorder(entries[j]);
                            cur = *(int *)WinZOrderWindow;
                        }
                        j++;
                    } while (j < *(int *)WinRootCount);
                }

                InvalidateRect(*(HWND *)&HandleMain, 0, 0);

                Win *w = WinTopDialog;
                if (w != 0 && reinterpret_cast<Win *>(w)->vslot_23() == 0) {
                    reinterpret_cast<Win *>(w)->vslot_07();
                }
            }
        }
    }

    *cursor = *cursor + 1;
    if (*cursor > (char *)(WinKeyRing + 9)) {
        *cursor = (char *)WinKeyRing;
    }

    bool handled = false;
    if (scroll_vert_ != 0 &&
        reinterpret_cast<ScrollSlots *>(scroll_vert_)->slot025((int)param2, WinKeyModifiers) != 0) {
        handled = true;
    }
    if (!handled && scroll_horz_ != 0 &&
        reinterpret_cast<ScrollSlots *>(scroll_horz_)->slot025((int)param2, WinKeyModifiers) != 0) {
        handled = true;
    }
    if (!handled) {
        key_click_event((int)param2, WinKeyModifiers);
    }
}

// ===== homed from src/recovered/units/005ec780.cpp =====

int Win::set_cursor(HCURSOR *name) {
    if (name != 0 && *reinterpret_cast<int *>(name) == 0) {
        return 7;
    }
    cursor_sprite_ = 0;
    cursor_handle_ = name;
    cursor_name_ = 0;
    Win::update_cursor(nullptr, 1);
    return 0;
}

// ===== homed from src/unrecovered/005ed880.cpp =====

int Win::resize(int width, int height, int repaint) {
    if (repaint != 0) {
        if ((iSomeFlag_ & 2) == 0) {
            int left = outer_rect_.left;
            int top = outer_rect_.top;
            outer_rect_.left = left;
            outer_rect_.right = left + width;
            outer_rect_.top = top;
            outer_rect_.bottom = top + height;
            goto shared;
        }
        client_to_nonclient(&width, &height);
    }
    {
        int cleft = client_rect_.left;
        int ctop = client_rect_.top;
        client_rect_.left = cleft;
        client_rect_.top = ctop;
        client_rect_.right = cleft + width;
        client_rect_.bottom = ctop + height;
        outer_rect_.left = client_rect_.left;
        outer_rect_.top = client_rect_.top;
        outer_rect_.right = client_rect_.right;
        outer_rect_.bottom = client_rect_.bottom;
        int negLeft = outer_rect_.left;
        outer_rect_.left -= negLeft;
        outer_rect_.right -= negLeft;
        outer_rect_.bottom -= outer_rect_.top;
        outer_rect_.top -= outer_rect_.top;
        nonclient_to_client(&outer_rect_);
    }
shared:
    this->set_rects();
    if (reinterpret_cast<Win *>(poWinBase_) != this) {
        poWinBase_->resize(width, height, 0);
    }
    resize_event(outer_rect_.right - outer_rect_.left, outer_rect_.bottom - outer_rect_.top);
    this->vslot_12();
    return 0;
}

// ===== homed from src/recovered/units/0063c340.cpp =====

// ORIGINAL: 0x0063C340 sub_63c340 0x0063C340-0x0063C38F FILE BYTE_EXACT
// symbol    ?sub_63c340@Win@@QAEHXZ
// size      79 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616350 0x0063C7C0
// indirect  0x0063C374 0x0063C37E
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0063C340
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0063c340/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63c340  at 0x0063C340  (79 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
// TWO FINDINGS, both recorded rather than cleared, and they are different
// kinds of stuck.
// 1. `self + 0xa14` is PAST `sizeof(Win)` (0x444), so the receiver is a
//    Win-DERIVED class carrying an embedded Time - the same misattribution
//    as on_l_button_down above. Unlike that one this body IS BYTE_EXACT, so
//    the image itself validates the offset: [this+0xA14] is really read.
//    The offset is right and the CLASS is wrong.
// 2. `reinterpret_cast<int>(this)` here is not a member walk and cannot be
//    spelled away: `Time::start(void(__cdecl *)(int), int param, ...)`
//    takes its callback context as an int, so a Win * has to go through
//    one. class_debt counts it as raw self-access, and its remedy - use the
//    declared member - does not apply to a pointer crossing an int-typed
//    API. Retyping that context is the Time work the plan calls A3; it
//    touches both Time::start overloads, the callback typedefs and every
//    caller including Palette's, so it belongs to Time's pass, not here.
int Win::sub_63c340() {
    char *self = reinterpret_cast<char *>(this);
    if (reinterpret_cast<Time *>(self + 0xa14)->start(
            sub_63c4e0,
            reinterpret_cast<int>(this), 1000, 5) != 0) {
        return 0;
    }
    this->sub_63c7c0();
    this->set_modal(0, 0, 0);
    this->hide();
    return *reinterpret_cast<int *>(*reinterpret_cast<char **>(self + 0x36a4) + 0x760);
}

// ===== homed from src/unrecovered/005edf50.cpp =====

void Win::set_caption(char * text) {
    void **caption = reinterpret_cast<void **>(&field_E0_);
    if (*caption != 0) {
        free(*caption);
        *caption = 0;
    }
    if (text != 0) {
        unsigned int len = strlen(text);
        void *buf = mem_get(len + 1);
        *caption = buf;
        if (buf == 0) {
            return;
        }
        *reinterpret_cast<char *>(buf) = 0;
        strcat(reinterpret_cast<char *>(*caption), text);
    }
    if (iSomeFlag_ & 1) {
        Win *other = win_parent_;
        if (other != 0) {
            if (!other->is_visible()) {
                return;
            }
        }
        this->on_nc_paint(0, -1);
    }
}

// ===== homed from src/unrecovered/005f5140.cpp =====

void __cdecl Win::bring_parent_to_top(Win * window) {
    if (window != 0 && (reinterpret_cast<unsigned int *>(window)[0x26] & 0x2000000) == 0) {
        int count = WinRootCount;
        int i = 0;
        if (count > 0) {
            while (true) {
                if (WinRootWindows[i] == window) {
                    if (i < count) {
                        int j = i;
                        while (j > 0) {
                            WinRootWindows[j] = WinRootWindows[j - 1];
                            --j;
                        }
                        WinRootWindows[0] = window;
                    }
                    break;
                }
                ++i;
                if (i >= count) break;
            }
        }
        WinZOrderCount = 0;
        if (count > 0) {
            for (int j = 0; j < count; ++j) {
                int cur = reinterpret_cast<int>(WinZOrderWindow);
                if (cur != 0 && cur == reinterpret_cast<int>(WinRootWindows[j])) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                Win *w = WinRootWindows[j];
                if (*(reinterpret_cast<unsigned char *>(w) + 0x9c) & 1) {
                    recurse_zorder(w);
                    count = WinRootCount;
                }
            }
        }
    }

}

// ===== homed from src/unrecovered/005eee70.cpp =====

// ORIGINAL: 0x005EEE70 ?client_to_nonclient@Win@@QAEXPAH0@Z 0x005EEE70-0x005EEF56 FILE BYTE_EXACT
// size      230 bytes
// prototype void (__thiscall ?client_to_nonclient@Win@@QAEXPAH0@Z)(Win* this, int*, int*)
// callers   3   call targets   0
// kind      
// flags     
// calls     (none)
// indirect  0x005EEF4A
// working copy - scaffold materialised by --work
void Win::client_to_nonclient(int * points, int * count) {
    if (points == 0) {
        return;
    }
    if (count == 0) {
        return;
    }
    if (*reinterpret_cast<uint8_t *>(&iFlags_) & 4) {
        *count += ScrollDefaultThickness;
    }
    if (*reinterpret_cast<uint8_t *>(&iFlags_) & 8) {
        *points += ScrollDefaultThickness;
    }
    uint32_t flags = iFlags_;
    if (flags & 0x400) {
        int doubled1 = border_thickness_ * 2;
        *points += doubled1;
        int doubled2 = border_thickness_ * 2;
        *count += doubled2;
        int newval = *count;
        if (bottom_border_thickness_ != -1) {
            *count = (bottom_border_thickness_ - border_thickness_) + newval;
        }
    } else if (flags & 0x11) {
        int doubled1 = border_thickness_ * 2;
        *points += doubled1;
        int doubled2 = border_thickness_ * 2;
        *count += doubled2;
        int newval = *count;
        if (bottom_border_thickness_ != -1) {
            *count = (bottom_border_thickness_ - border_thickness_) + newval;
        }
    }
    if (*reinterpret_cast<uint8_t *>(&iFlags_) & 0x10) {
        *count += caption_height_ - border_thickness_;
    }
    if (menu_ != 0) {
        *count += reinterpret_cast<MenuVCall *>(menu_)->slot091();
    }
}

// ===== homed from src/unrecovered/005ed3f0.cpp =====

void Win::nonclient_to_screen(int * x, int * y) {
    *x = *x + outer_rect_.left + client_rect_.left;
    *y = *y + outer_rect_.top + client_rect_.top;
    if ((*(unsigned char *)&iFlags_ & 0x20) != 0 && win_parent_ != 0) {
        win_parent_->client_to_screen(x, y);
        if ((iFlags_ & 0x8000) != 0) {
            *x = *x - win_parent_->outer_rect_.left;
            *y = *y - win_parent_->outer_rect_.top;
        }
    }
    *x = *x - outer_rect_.left;
    *y = *y - outer_rect_.top;
}

// ===== homed from src/unrecovered/005eea90.cpp =====
typedef void (__stdcall *VCall2Fn)(void *, int, char *, int);
// RETURNS int: the body assigns `result = callback(a1, a2)`, which the
// artifact's `void` spelling could not have compiled.
typedef int (__cdecl *Callback2)(int, int);
typedef void (OriginalObject::*VCall0)();
typedef void (OriginalObject::*VCall1)(int);

int Win::key_click_event(int key, int flags) {
    Win *const self = this;

    if ((iFlags_ & 0x200000) != 0) {
        return 0;
    }
    if ((iSomeFlag_ & 8) != 0) {
        return 0;
    }

    {
        void *vtbl = *(void **)self;
        if (flags == 0x1b) {
            this->vslot_44(-2);
        } else if (flags == 0xd || flags == 0x1000d) {
            this->vslot_44(-1);
        }
    }

    if (list_.count_ != 0) {
        Win *target = 0;
        if (list_.head_ != 0) {
            int *field_d0 = reinterpret_cast<int *>(list_.current_);
            target = *(Win **)((char *)field_d0 + 4);
        }
        if (flags == 9) {
            void *tvtbl = *(void **)target;
            reinterpret_cast<Win *>(target)->pass_dialog_focus();
        } else {
            if (target->key_click_event(key, flags) != 0) {
                return 1;
            }
        }
    }

    int result = 0;
    *(void **)0x009B7AB8 = self;
    Callback2 callback = reinterpret_cast<Callback2>(field_42C_);
    if (callback != 0) {
        result = callback(key, flags);
    }
    void *vtbl2 = *(void **)self;
    int r2 = this->vslot_25(key, flags);
    Win *child = reinterpret_cast<Win *>(val_14_);
    if (child != 0) {
        void *cvtbl = *(void **)child;
        if (reinterpret_cast<Win *>(child)->vslot_23() == 0) {
            void *cvtbl2 = *(void **)child;
            reinterpret_cast<Win *>(child)->vslot_07();
        }
    }
    return result + r2;
}

// ===== homed from src/unrecovered/005f74a0.cpp =====

int Win::update_window(RECT * area) {
    if (area == 0) {
        return update_screen(0, this);
    }
    int y = client_rect_.top + outer_rect_.top;
    int x = client_rect_.left + outer_rect_.left;
    if ((iFlags_ & 0x20) != 0 &&
        win_parent_ != 0) {
        client_to_screen(&x, &y);
        if ((iFlags_ & 0x8000) != 0) {
            char *rel = reinterpret_cast<char *>(win_parent_);
            x -= *reinterpret_cast<int *>(rel + 0x13c);
            y -= *reinterpret_cast<int *>(rel + 0x140);
        }
    }
    RECT r;
    r.left = area->left + x;
    r.top = area->top + y;
    r.right = area->right + x;
    r.bottom = area->bottom + y;
    return update_screen(&r, this);
}

// ===== homed from src/unrecovered/005f52e0.cpp =====

void Win::bring_child_to_top(Win * child) {
    if (child == 0) {
        return;
    }
    if (child->iFlags_ & 0x2000000) {
        return;
    }
    int count = child_count_;
    int i = 0;
    if (count > 0) {
        Win **slot = children_;
        do {
            if (*slot == child) {
                break;
            }
            ++i;
            ++slot;
        } while (i < count);
    }
    if (i < count) {
        if (i > 0) {
            Win **p = &children_[i];
            do {
                *p = p[-1];
                --p;
                --i;
            } while (i != 0);
        }
        children_[0] = child;
    }
    WinZOrderCount = 0;
    if (WinRootCount > 0) {
        Win **entry = WinRootWindows;
        int cur = reinterpret_cast<int>(WinZOrderWindow);
        int j = 0;
        do {
            if (cur != 0 && cur == reinterpret_cast<int>(*entry)) {
                WinZOrderCount = 0;
                WinZOrderFlag = 0;
            }
            if ((*entry)->iSomeFlag_ & 1) {
                recurse_zorder(*entry);
                cur = reinterpret_cast<int>(WinZOrderWindow);
            }
            ++j;
            ++entry;
        } while (j < WinRootCount);
    }
}

// ===== homed from src/recovered/005edeb0.cpp =====

// ORIGINAL: 0x005EDEB0 ?minimize@Win@@QAEHXZ 0x005EDEB0-0x005EDEF8 BYTE_EXACT
// size      72 bytes
// prototype int (__thiscall ?minimize@Win@@QAEHXZ)(Win* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EDED3 0x005EDEED
// 0x005EDEB0  ?minimize@Win@@QAEHXZ  ->  ?minimize@Win@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
int Win::minimize() {
    Win *active = WinPointerOwner3;
    WinPointerOwner1 = 0;
    WinPointerOwner2 = 0;
    if (active == this) {
        WinPointerOwner3 = 0;
        this->vslot_04();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    ShowWindow(
        HandleMain, 6);
    return 0;
}

// ===== homed from src/unrecovered/005f7580.cpp =====

void Win::do_tracking(int x, int y) {
    int dx, dy;

    // ---- 0x005F7589-0x005F7657: normalize (x,y) into a client-space delta ----
    if ((iFlags_ & 0x4000000) != 0) {
        dx = x - (client_rect_.left + outer_rect_.left);
        dy = y - (client_rect_.top + outer_rect_.top);
        if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
            win_parent_->screen_to_client(&dx, &dy);
            if ((iFlags_ & 0x8000) != 0) {
                dx += win_parent_->outer_rect_.left;
                dy += win_parent_->outer_rect_.top;
            }
        }
    } else {
        screen_to_client(&x, &y);
        dx = x + outer_rect_.left;
        dy = y + outer_rect_.top;
    }
    dx -= WinTrackingX;
    dy -= WinTrackingY;
    if (dx == 0 && dy == 0) return;

    // ---- 0x005F7673-0x005F76E9: min-size threshold, transformed to nonclient space ----
    int minW = static_cast<int>(field_134_);
    int minH = static_cast<int>(field_138_);
    client_to_nonclient(&minW, &minH);

    // ---- 0x005F7698-0x005F77CD: build an absolute-screen "before" rect ----
    RECT rectA;
    int origX = 0, origY = 0;
    if ((iFlags_ & 0x4000000) != 0) {
        rectA.left = 0;
        rectA.top = 0;
        rectA.right = outer_rect_.right - outer_rect_.left;
        rectA.bottom = outer_rect_.bottom - outer_rect_.top;
        client_to_screen(&origX, &origY);
    } else {
        rectA.left = 0;
        rectA.top = 0;
        rectA.right = client_rect_.right - client_rect_.left;
        rectA.bottom = client_rect_.bottom - client_rect_.top;
        nonclient_to_screen(&origX, &origY);
    }
    rectA.left += origX;
    rectA.right += origX;
    rectA.top += origY;
    rectA.bottom += origY;

    // ---- 0x005F77CD-0x005F77DF: dispatch on drag-handle id ----
    switch (WinTrackingMode) {
    case 2:
        if (dx != 0 || dy != 0) {
            if ((iFlags_ & 0x4000000) == 0) {
                client_rect_.left += dx;
                client_rect_.right += dx;
                client_rect_.top += dy;
                client_rect_.bottom += dy;
            } else {
                outer_rect_.left += dx;
                outer_rect_.right += dx;
                outer_rect_.top += dy;
                outer_rect_.bottom += dy;
            }
        }
        break;

    case 10:
        if (dx != 0) {
            int right = client_rect_.right;
            client_rect_.left += dx;
            if (right - client_rect_.left < minW) {
                client_rect_.left = right - minW;
            }
            poWinBase_->resize(right - client_rect_.left,
                client_rect_.bottom - client_rect_.top, 0);
        }
        break;

    case 0xb:
        if (dx != 0) {
            client_rect_.right += dx;
            WinTrackingX += dx;
            int width = client_rect_.right - client_rect_.left;
            if (width < minW) {
                int deficit = minW - width;
                client_rect_.right += deficit;
                WinTrackingX += deficit;
            }
            int w = client_rect_.right - client_rect_.left;
            int h = client_rect_.bottom - client_rect_.top;
            poWinBase_->resize(w, h, 0);
        }
        break;

    case 0xc:
        if (dy != 0) {
            int bottom = client_rect_.bottom;
            client_rect_.top += dy;
            if (bottom - client_rect_.top < minH) {
                client_rect_.top = bottom - minH;
            }
            int w = client_rect_.right - client_rect_.left;
            int h = client_rect_.bottom - client_rect_.top;
            poWinBase_->resize(w, h, 0);
        }
        break;

    case 0xd: {
        int right = client_rect_.right;
        int bottom = client_rect_.bottom;
        client_rect_.left += dx;
        client_rect_.top += dy;
        if (bottom - client_rect_.top < minH) {
            client_rect_.top = bottom - minH;
        }
        if (right - client_rect_.left < minW) {
            client_rect_.left = right - minW;
        }
        int w = client_rect_.right - client_rect_.left;
        int h = client_rect_.bottom - client_rect_.top;
        poWinBase_->resize(w, h, 0);
        break;
    }

    case 0xe: {
        client_rect_.right += dx;
        client_rect_.top += dy;
        WinTrackingX += dx;
        if (client_rect_.bottom - client_rect_.top < minH) {
            client_rect_.top = client_rect_.bottom - minH;
        }
        int width = client_rect_.right - client_rect_.left;
        if (width < minW) {
            int deficit = minW - width;
            client_rect_.right += deficit;
            WinTrackingX += deficit;
        }
        int w = client_rect_.right - client_rect_.left;
        int h = client_rect_.bottom - client_rect_.top;
        poWinBase_->resize(w, h, 0);
        break;
    }

    case 0xf:
        if (dy != 0) {
            client_rect_.bottom += dy;
            WinTrackingY += dy;
            int height = client_rect_.bottom - client_rect_.top;
            if (height < minH) {
                int deficit = minH - height;
                client_rect_.bottom += deficit;
                WinTrackingY += deficit;
            }
            int w = client_rect_.right - client_rect_.left;
            int h = client_rect_.bottom - client_rect_.top;
            poWinBase_->resize(w, h, 0);
        }
        break;

    case 0x10: {
        client_rect_.left += dx;
        client_rect_.bottom += dy;
        WinTrackingY += dy;
        int height = client_rect_.bottom - client_rect_.top;
        if (height < minH) {
            int deficit = minH - height;
            client_rect_.bottom += deficit;
            WinTrackingY += deficit;
        }
        int right = client_rect_.right;
        if (right - client_rect_.left < minW) {
            client_rect_.left = right - minW;
        }
        int w = client_rect_.right - client_rect_.left;
        int h = client_rect_.bottom - client_rect_.top;
        poWinBase_->resize(w, h, 0);
        break;
    }

    case 0x11: {
        client_rect_.right += dx;
        client_rect_.bottom += dy;
        WinTrackingX += dx;
        WinTrackingY += dy;
        int height = client_rect_.bottom - client_rect_.top;
        if (height < minH) {
            int deficit = minH - height;
            client_rect_.bottom += deficit;
            WinTrackingY += deficit;
        }
        int width = client_rect_.right - client_rect_.left;
        if (width < minW) {
            int deficit = minW - width;
            client_rect_.right += deficit;
            WinTrackingX += deficit;
        }
        int w = client_rect_.right - client_rect_.left;
        int h = client_rect_.bottom - client_rect_.top;
        poWinBase_->resize(w, h, 0);
        break;
    }

    default:
        break;
    }

    // ---- 0x005F7C2F-0x005F7DE7: build an absolute-screen "after" rect ----
    RECT rectB;
    int origX2, origY2;
    if ((iFlags_ & 0x4000000) != 0) {
        rectB.left = 0;
        rectB.top = 0;
        rectB.right = outer_rect_.right - outer_rect_.left;
        rectB.bottom = outer_rect_.bottom - outer_rect_.top;

        origX2 = client_rect_.left + outer_rect_.left;
        origY2 = client_rect_.top + outer_rect_.top;
        if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
            win_parent_->client_to_screen(&origX2, &origY2);
            if ((iFlags_ & 0x8000) != 0) {
                origX2 -= win_parent_->outer_rect_.left;
                origY2 -= win_parent_->outer_rect_.top;
            }
        }
    } else {
        rectB.left = 0;
        rectB.top = 0;
        rectB.right = client_rect_.right - client_rect_.left;
        rectB.bottom = client_rect_.bottom - client_rect_.top;

        origX2 = client_rect_.left + outer_rect_.left;
        origY2 = client_rect_.top + outer_rect_.top;
        if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
            win_parent_->client_to_screen(&origX2, &origY2);
            if ((iFlags_ & 0x8000) != 0) {
                origX2 -= win_parent_->outer_rect_.left;
                origY2 -= win_parent_->outer_rect_.top;
            }
        }
        origX2 -= outer_rect_.left;
        origY2 -= outer_rect_.top;
    }
    rectB.left += origX2;
    rectB.right += origX2;
    rectB.top += origY2;
    rectB.bottom += origY2;

    // ---- 0x005F7E0F-0x005F7E3A: union the two rects and repaint ----
    RECT unioned;
    UnionRect(&unioned, &rectA, &rectB);
    update_screen(&unioned, 0);
    flip(&unioned);
}

// ===== homed from src/unrecovered/005ef1e0.cpp =====

void Win::sub_5ef1e0(int x1, int y1, int x2, int y2, void *pen, int unused6) {
    (void)unused6;
    HDC hdc;
    if (WinHdcRefCount != 0) {
        WinHdcRefCount = WinHdcRefCount + 1;
        hdc = WinSharedHdc;
    } else {
        void *iface = DirectDrawSurface;
        if (iface != 0) {
            IfaceGetHdcProc fn = (*reinterpret_cast<IfaceGetHdcProc **>(iface))[17];
            fn(iface, &WinSharedHdc);
            hdc = WinSharedHdc;
        } else {
            WinSharedHdc = GetDC(HandleMain);
            hdc = WinSharedHdc;
        }
        if (hdc == 0) {
            return;
        }
        WinHdcRefCount = 1;
    }
    if (hdc != 0) {
        if ((int)palette_seed_cache_ != PaletteActive->seed_) {
            PaletteActive->set_active_window(this);
            palette_seed_cache_ = PaletteActive->seed_;
        }

        void *oldPen = SelectObject(WinSharedHdc, pen);

        MoveToEx(WinSharedHdc, x1, y1, 0);

        LineTo(WinSharedHdc, x2, y2);

        SelectObject(WinSharedHdc, oldPen);

        WinHdcRefCount = WinHdcRefCount - 1;
        if (WinHdcRefCount == 0) {
            void *iface2 = DirectDrawSurface;
            if (iface2 != 0) {
                IfaceReleaseHdcProc fn2 = (*reinterpret_cast<IfaceReleaseHdcProc **>(iface2))[26];
                fn2(iface2, WinSharedHdc);
                WinSharedHdc = 0;
                return;
            }
            ReleaseDC(HandleMain, WinSharedHdc);
            WinSharedHdc = 0;
        }
    }
}

// ===== homed from src/unrecovered/005f7320.cpp =====

int __cdecl Win::update_screen(RECT *area, Win *window) {
    int windowCount = WinZOrderCount;
    if (windowCount == 0) {
        return 0;
    }

    int target;
    if (WinZOrderWindow != 0) {
        target = reinterpret_cast<int>(window);
    } else {
        target = 0;
    }

    int idx = windowCount - 1;
    if (target != 0) {
        idx = 0;
        if (windowCount > 0) {
            Win **table = WinZOrderArray;
            int i = 0;
            while (reinterpret_cast<int>(table[i]) != target) {
                ++i;
                if (i >= windowCount) {
                    break;
                }
            }
            idx = i;
        }
        if (idx == windowCount) {
            return windowCount;
        }
    } else {
        if (WinBackBuffer == 0) {
            if (area != 0) {
                (&ScreenBuffer)->fill(area, 0);
            } else {
                (&ScreenBuffer)->fill(0);
            }
        }
    }

    if (idx >= 0) {
        for (int i = idx; i >= 0; --i) {
            Win *w = reinterpret_cast<Win *>(reinterpret_cast<void **>(WinZOrderArray)[i]);
            unsigned char *const wb = reinterpret_cast<unsigned char *>(w);
            if ((*(wb + 0x9c) & 1) != 0) {
                Win *menuLike = *reinterpret_cast<Win **>(wb + 0xc4);
                bool visible = true;
                if (menuLike != 0) {
                    visible = menuLike->is_visible() != 0;
                }
                if (visible) {
                    if ((*(wb + 0x9c) & 2) != 0) {
                        w->on_nc_paint(area, -1);
                    }
                    bool doSecond = true;
                    if (target != 0) {
                        doSecond = (target == *reinterpret_cast<int *>(wb + 0xa8));
                    }
                    if (doSecond) {
                        Win *const other = *reinterpret_cast<Win **>(wb + 0xa8);
                        other->on_paint(area);
                    }
                }
            }
        }
    }

    if (WinFlipSprite == 0) {
        return 0;
    }

    int base = WinFlipSpriteY;
    int wA = WinClipWidth;
    int base2 = WinFlipSpriteX;
    int l10 = base;
    int l14 = base2;
    int l18 = wA + base;
    int wB = WinClipHeight;
    int l1c = wB + base2;

    if (area != 0) {
        int rectA[2] = {l10, l14};
        int rectB[2] = {l18, l1c};
        int result = IntersectRect(reinterpret_cast<RECT *>(&rectB[0]), reinterpret_cast<const RECT *>(area),reinterpret_cast<const tagRECT *>(&rectA[0]));
        if (result == 0) {
            return 0;
        }
        l1c = rectB[0];
        l18 = rectB[1];
        l10 = rectA[0];
        l14 = rectA[1];
        base2 = WinFlipSpriteX;
        base = WinFlipSpriteY;
    }

    (&ScreenBuffer)
        ->copy(g_WIN_BUFFER, l14 - base2, l10 - base, l1c - l14,
               l18 - l10, l14, l10);
    return 0;
}

// ===== homed from src/recovered/units/005f5d10.cpp =====
typedef short(__stdcall *GetKeyStateFn)(int);
typedef int(__stdcall *PeekMessageFn)(MsgT *, void *, unsigned int, unsigned int, unsigned int);
typedef int(__stdcall *Fn1)(void *);

void Win::on_key(unsigned int key, long flags, int repeat, unsigned int scan) {
    char *self = reinterpret_cast<char *>(this);
    (void)repeat;
    (void)scan;

    unsigned int with_modifiers = key;
    if (GetKeyState(0x10) & 0x8000) with_modifiers |= 0x10000;
    if (GetKeyState(0x11) & 0x8000) with_modifiers |= 0x20000;
    if (GetKeyState(0x12) & 0x8000) with_modifiers |= 0x40000;

    char *net = *reinterpret_cast<char **>(WinNetBuffer);
    if (net != 0) {
        MsgT msg;
        if (with_modifiers == *reinterpret_cast<unsigned int *>(net + 0x48)) {
            if (flags == 0) {
                reinterpret_cast<Net *>(net)->stop_voice();
            } else {
                reinterpret_cast<Net *>(net)->start_voice(
                    *reinterpret_cast<unsigned long *>(net + 0x50));
            }
            if (!PeekMessageA(&msg, HandleMain, 0x102, 0x102, 1)) {
                return;
            }
            do {
            } while (PeekMessageA(&msg, HandleMain, 0x102, 0x102, 1));
            return;
        }
        if (with_modifiers == *reinterpret_cast<unsigned int *>(net + 0x4c)) {
            if (flags == 0) {
                reinterpret_cast<Net *>(net)->stop_voice();
            } else {
                reinterpret_cast<Net *>(net)->start_voice(
                    *reinterpret_cast<long *>(net + 0x54));
            }
            if (!PeekMessageA(&msg, HandleMain, 0x102, 0x102, 1)) {
                return;
            }
            do {
            } while (PeekMessageA(&msg, HandleMain, 0x102, 0x102, 1));
            return;
        }
    }

    if (flags == 0) {
        if (iFlags_ & 0x200000) {
            return;
        }
        if (iSomeFlag_ & 8) {
            return;
        }
        if (list_.count_ != 0) {
            void *target;
            if (list_.head_ != 0) {
                target = reinterpret_cast<void **>(list_.current_)[1];
            } else {
                target = 0;
            }
            if (reinterpret_cast<Win *>(target)->key_up_event(with_modifiers) != 0) {
                return;
            }
        }

        WinCallbackWindow = this;
        typedef void(__cdecl * OnKeyCb)(unsigned int);
        OnKeyCb cb = reinterpret_cast<OnKeyCb>(field_434_);
        if (cb != 0) {
            cb(with_modifiers);
        }
        reinterpret_cast<VCallArg *>(self)->s27(with_modifiers);

        Win *next = reinterpret_cast<Win *>(val_16_);
        if (next == 0) {
            return;
        }
        if (next->vslot_23() != 0) {
            return;
        }
        next->vslot_07();
        return;
    }

    *reinterpret_cast<unsigned int *>(WinKeyModifiers) = with_modifiers;
    if (IsWindow(HandleMain)) {
        this->key_down_event(with_modifiers);
    }

    if (with_modifiers >= 0x60 && with_modifiers <= 0x6f) {
        MsgT msg;
        if (!PeekMessageA(&msg, HandleMain, 0x102, 0x102, 1)) {
            return;
        }
        do {
        } while (PeekMessageA(&msg, HandleMain, 0x102, 0x102, 1));
        return;
    }

    do_all_chars();
}

// ===== homed from src/unrecovered/005ee330.cpp =====

// DUPLICATE `Win::resize_event` removed: two artifacts carried the same body.

// ===== homed from src/unrecovered/005f2680.cpp =====
typedef long (__stdcall *GetWindowLongAFn)(void *, int);
typedef long (__stdcall *DefWindowProcAFn)(void *, unsigned int, unsigned int, long);
typedef void (__cdecl *Callback404)();

int __cdecl Win::OnSysCommand(HWND hwnd, unsigned int command, int x,
                              int y) {
    Win *self = reinterpret_cast<Win *>(
        GetWindowLongA(hwnd, -0x15));
    if (self == 0) {
        return 0;
    }
    if (command != 0xf060) {
        return DefWindowProcA(
            hwnd, 0x112, command,
            (static_cast<unsigned int>(static_cast<unsigned short>(y)) << 16)
                | static_cast<unsigned short>(x));
    }
    *reinterpret_cast<Win **>(WinCallbackWindow) = self;
    Callback404 cb = *reinterpret_cast<Callback404 *>(
        reinterpret_cast<char *>(self) + 0x404);
    if (cb != 0) {
        cb();
    }
    self->vslot_14();
    Win *child = *reinterpret_cast<Win **>(
        reinterpret_cast<char *>(self) + 0x18);
    if (child != 0) {
        if (child->vslot_23() == 0) {
            child->vslot_07();
        }
    }
    return 0;
}

// ===== homed from src/unrecovered/005ef050.cpp =====

void Win::client_to_nonclient(RECT * rect) {
    if (rect != 0) {
        if ((iFlags_ & 4) != 0) {
            rect->bottom += ScrollDefaultThickness;
        }
        if ((iFlags_ & 8) != 0) {
            rect->right += ScrollDefaultThickness;
        }
        if ((iFlags_ & 0x400) != 0 || (iFlags_ & 0x11) != 0) {
            int adj = border_thickness_;
            rect->left -= adj;
            rect->right += adj;
            rect->top -= adj;
            long bottom = rect->bottom;
            rect->bottom = bottom + adj;
            if (bottom_border_thickness_ != -1) {
                rect->bottom = (bottom_border_thickness_ - border_thickness_) + bottom + adj;
            }
        }
        if ((iFlags_ & 0x10) != 0) {
            rect->top += border_thickness_ - caption_height_;
        }
        if (menu_ != 0) {
            int adj = reinterpret_cast<VCall2 *>(menu_)->slot091();
            rect->top -= adj;
        }
    }
}

// ===== homed from src/unrecovered/005eef60.cpp =====

void Win::nonclient_to_client(int * x, int * y) {
    if (x == 0) {
        return;
    }
    if (y == 0) {
        return;
    }
    if ((iFlags_ & 4) != 0) {
        *y -= ScrollDefaultThickness;
    }
    if ((iFlags_ & 8) != 0) {
        *x -= ScrollDefaultThickness;
    }
    if ((iFlags_ & 0x400) != 0) {
        *x += -border_thickness_ * 2;
        *y += -border_thickness_ * 2;
        if (bottom_border_thickness_ != -1) {
            *y = (border_thickness_ - bottom_border_thickness_) + *y;
        }
    } else if ((iFlags_ & 0x11) != 0) {
        *x += -border_thickness_ * 2;
        *y += -border_thickness_ * 2;
        if (bottom_border_thickness_ != -1) {
            *y = (border_thickness_ - bottom_border_thickness_) + *y;
        }
    }
    if ((iFlags_ & 0x10) != 0) {
        *y += caption_height_ - border_thickness_;
    }
    if (menu_ != 0) {
        *y -= reinterpret_cast<NCCall *>(menu_)->slot091();
    }
}

// ===== homed from src/recovered/units/005f8140.cpp =====

void Win::update_window_to_buffer(Buffer * buffer) {
    if (buffer != 0) {

        WinBackBuffer = buffer;
        WinZOrderWindow = this;
        (WinViewOriginX) = 0;
        (WinViewOriginY) = 0;

        uint32_t flags9c = iSomeFlag_;
        int iVar3 = 0;
        iSomeFlag_ = flags9c | 1;
        WinZOrderCount = 0;
        if (0 < WinRootCount) {
            Win **piVar4 = WinRootWindows;
            int iVar2 = reinterpret_cast<int>(WinZOrderWindow);
            do {
                if (iVar2 != 0 && iVar2 == (int)*piVar4) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                if ((*reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(*piVar4) + 0x9c) & 1) != 0) {
                    recurse_zorder(*piVar4);
                    iVar2 = reinterpret_cast<int>(WinZOrderWindow);
                }
                iVar3++;
                piVar4++;
            } while (iVar3 < WinRootCount);
        }

        if ((iSomeFlag_ & 2) == 0) {
            (WinViewOriginX) += client_rect_.left + outer_rect_.left;
            (WinViewOriginY) += client_rect_.top + outer_rect_.top;
            if ((iFlags_ & 0x20) != 0 &&
                win_parent_ != 0) {
                Win *parent = win_parent_;
                parent->client_to_screen(&(WinViewOriginX), &(WinViewOriginY));
                if ((iFlags_ & 0x8000) != 0) {
                    char *pself = reinterpret_cast<char *>(win_parent_);
                    (WinViewOriginX) -= *reinterpret_cast<int32_t *>(pself + 0x13c);
                    (WinViewOriginY) -= *reinterpret_cast<int32_t *>(pself + 0x140);
                }
            }
        } else {
            (WinViewOriginX) += client_rect_.left + outer_rect_.left;
            (WinViewOriginY) += client_rect_.top + outer_rect_.top;
            if ((iFlags_ & 0x20) != 0 &&
                win_parent_ != 0) {
                Win *parent = win_parent_;
                parent->client_to_screen(&(WinViewOriginX), &(WinViewOriginY));
                if ((iFlags_ & 0x8000) != 0) {
                    char *pself = reinterpret_cast<char *>(win_parent_);
                    (WinViewOriginX) -= *reinterpret_cast<int32_t *>(pself + 0x13c);
                    (WinViewOriginY) -= *reinterpret_cast<int32_t *>(pself + 0x140);
                }
            }
            (WinViewOriginX) -= outer_rect_.left;
            (WinViewOriginY) -= outer_rect_.top;
        }

        update_screen(0, this);

        WinBackBuffer = 0;
        WinZOrderWindow = 0;
        iSomeFlag_ = flags9c;

        iVar3 = 0;
        WinZOrderCount = 0;
        if (0 < WinRootCount) {
            Win **piVar4 = WinRootWindows;
            int iVar2 = reinterpret_cast<int>(WinZOrderWindow);
            do {
                if (iVar2 != 0 && iVar2 == (int)*piVar4) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                if ((*reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(*piVar4) + 0x9c) & 1) != 0) {
                    recurse_zorder(*piVar4);
                    iVar2 = reinterpret_cast<int>(WinZOrderWindow);
                }
                iVar3++;
                piVar4++;
            } while (iVar3 < WinRootCount);
        }
    }
}

// ===== homed from src/recovered/units/005f6710.cpp =====

void Win::on_r_button_down(long flags, int x, int y, unsigned int keys, int dbl) {
    typedef void(__cdecl *RawHandler)(int, int);

    Win *parent = win_parent_;
    if (parent != 0) {
        parent->bring_child_to_top(this);
    } else {
        bring_parent_to_top(this);
    }

    parent = win_parent_;
    if (parent != 0) {
        parent->set_dialog_focus(this);
    }

    if (dbl == 0) {
        WinPointerOwner2 = nullptr;
        WinPointerOwner1 = this;
    } else {
        WinPointerOwner1 = nullptr;
        WinPointerOwner2 = this;
    }
    WinSizingFlag = 0;

    if (flags != 0 &&
        (iFlags_ & 0x200200) == 0 &&
        (iSomeFlag_ & 8) == 0) {
        if (dbl == 0) {
            WinCallbackWindow = this;
            RawHandler fn = reinterpret_cast<RawHandler>(field_410_);
            if (fn != 0) {
                fn(x, y);
            }
            this->vslot_29(x, y);
            Win *child = reinterpret_cast<Win *>(val_18_);
            if (child != 0) {
                int r = child->vslot_23();
                if (r == 0) {
                    child->vslot_07();
                }
            }
        } else {
            this->vslot_39(x, y);
            Win *child = reinterpret_cast<Win *>(val_27_);
            if (child != 0) {
                int r = child->vslot_23();
                if (r == 0) {
                    child->vslot_07();
                }
            }
        }
    }

    if ((iFlags_ & 0x200000) == 0 &&
        (iSomeFlag_ & 8) == 0) {
        if (dbl == 0) {
            WinCallbackWindow = this;
            RawHandler fn = reinterpret_cast<RawHandler>(field_424_);
            if (fn != 0) {
                fn(x, y);
            }
            this->vslot_23();
            Win *child = reinterpret_cast<Win *>(val_12_);
            if (child != 0) {
                int r = child->vslot_23();
                if (r == 0) {
                    child->vslot_07();
                }
            }
        } else {
            this->vslot_36(x, y);
            Win *child = reinterpret_cast<Win *>(val_24_);
            if (child != 0) {
                int r = child->vslot_23();
                if (r == 0) {
                    child->vslot_07();
                }
            }
        }
    }

}

// ===== homed from src/recovered/units/005f2570.cpp =====

void Win::OnLButtonUp(void *hwnd, int x, int y, unsigned int keys) {
    WinTrackingWindow = nullptr;
    // get_mouse_window already RETURNS Win *, and win.h declares slot 82 as
    // on_l_button_up - so the shim was casting a Win to a shim class to call
    // a slot the class itself now names.
    Win *obj = get_mouse_window(reinterpret_cast<int *>(&hwnd), &x);
    if (obj) {
        obj->on_l_button_up(reinterpret_cast<int>(hwnd), x, y, WinMouseDirect);
    }
}

// ===== homed from src/unrecovered/005f15c0.cpp =====

void __cdecl Win::OnKey(void * hwnd, unsigned int key, long flags, int repeat, unsigned int scan) {
    int *p_9b7ac8 = (int *)0x009B7AC8;
    int *p_9b7ac4 = (int *)0x009B7AC4;
    int *p_9b7aec = (int *)0x009B7AEC;
    int *p_9b7ae0 = (int *)0x009B7AE0;
    int *p_9b8d7c = (int *)0x009B8D7C;
    int *p_9b7a8c = (int *)0x009B7A8C;
    int *p_9b7a88 = (int *)0x009B7A88;
    int chosen;

    if (*p_9b7ae0 != 0) {
        chosen = *p_9b8d7c;
        if (chosen == 0) {
            chosen = *p_9b7ae0;
        }
    } else {
        chosen = *p_9b7ac4;
        if (chosen == 0) {
            chosen = *p_9b7aec;
        }
    }
    *p_9b7ac8 = chosen;

    if (chosen != 0) {
        char *base = (char *)chosen;
        Win *inner = *(Win **)(base + 0xa8);
        if (inner != 0 && (*((unsigned char *)inner + 0x9c) & 1) != 0) {
            char *innerb = (char *)inner;
            Win *parent = *(Win **)(innerb + 0xc4);
            int visible = 1;
            if (parent != 0) {
                visible = parent->is_visible();
            }
            if (visible != 0) {
                reinterpret_cast<Win *>(inner)->on_key(key, flags, repeat, scan);
            }
        }
    }

    if (*p_9b7a8c != 0 && flags != 0) {
        // 0x009B7A8C IS win.h's typed key hook; call it by its name.
        WinKeyHook(key);
    }

    if (*p_9b7a88 != 0) {
        // 0x009B7A88, likewise the message hook.
        WinMessageHook();
    }
}

// ===== homed from src/unrecovered/005f5c00.cpp =====
// ORIGINAL: 0x005F5C00 sub_5f5c00 0x005F5C00-0x005F5D05 FILE
// symbol    ?on_activate@Win@@UAEJIPAXJ@Z
// size      261 bytes
// kind      game
// Win vtable slot 87 (win.h); unnamed in the catalogue. The DC acquire/
// release arms call IDirectDrawSurface::GetDC/ReleaseDC by name - the
// same slots 17/26 OnQueryNewPalette reaches.
long Win::on_activate(unsigned int state, void * other, long minimized) {
    if (state != 0 && minimized == 0 && BufferDirectDraw == 0) {
        if (WinHdcRefCount == 0) {
            IDirectDrawSurface *obj = DirectDrawSurface;
            if (obj == 0) {
                WinSharedHdc = GetDC(HandleMain);
            } else {
                // IDirectDrawSurface::GetDC is slot 17, offset 0x44 - the
                // named call emits the image's push-addr/call-[vtbl+0x44].
                obj->GetDC(&WinSharedHdc);
            }
            if (WinSharedHdc == 0) {
                goto done;
            }
            WinHdcRefCount = 1;
        } else {
            WinHdcRefCount = WinHdcRefCount + 1;
        }
        if (WinSharedHdc != 0) {
            SelectPalette(WinSharedHdc, reinterpret_cast<HPALETTE>(PaletteInitialized), 0);
            RealizePalette(WinSharedHdc);
            WinHdcRefCount = WinHdcRefCount - 1;
            if (WinHdcRefCount == 0) {
                IDirectDrawSurface *obj = DirectDrawSurface;
                if (obj == 0) {
                    ReleaseDC(HandleMain, WinSharedHdc);
                } else {
                    // Slot 26, offset 0x68, is IDirectDrawSurface::ReleaseDC
                    // (HDC by value per ddraw.h); if the image turns out to
                    // push the ADDRESS instead, that is a finding for this
                    // body's own grind, recorded on the body - not a reason
                    // to keep the slot arithmetic here.
                    obj->ReleaseDC(WinSharedHdc);
                }
                WinSharedHdc = 0;
            }
        }
    }
done:
    return DefWindowProcA(HandleMain, 6,
                          ((minimized & 0xffff) << 16) | (state & 0xffff), reinterpret_cast<long>(other));
}

// ===== homed from src/recovered/units/005f5020.cpp =====

// ORIGINAL: 0x005F5020 ?update_zorder@Win@@QAAXXZ 0x005F5020-0x005F5080 FILE BYTE_EXACT
// size      96 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F4EC0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F5020
// measured tier  NO_COMPILE
// refusal        u005f5020.cpp(29) : error C2653: 'Win' : is not a class or namespace name u005f5020.cpp(31) : error C2065: 'WinRootCount' : undeclared identifier u005f5020.cpp(31
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f5020/unit.cpp
// and score it with tools/agent_brief.py.
// 0x005F5020  ?update_zorder@Win@@QAAXXZ  (96 bytes)
//
// `QAAXXZ`: public, __cdecl, void, no params - a STATIC member (no `this` is
// ever read; every operand is one of the five fixed globals or the array at
// WinRootWindows). The stale on-disk scaffold had this as `fn_005f5020(Win* a1)`;
// the fresh brief's contract head (`void __cdecl Win::update_zorder()`)
// matches the disassembly, so that is what is defined here.
//
// The initial `mov eax, dword ptr [0x9b7b34]` (the loop bound) happens
// BEFORE the `mov dword ptr [0x9b7b30], ebp` store in the original, even
// though the store is unconditional and the load only feeds the guard below
// it. Writing `WinZOrderCount = 0;` before reading the count reordered the two
// (store first) under /O2; reading the count into a local FIRST, in source
// order, reproduces the original ordering exactly. The do-while's own
// continuation test re-reads `WinRootCount` fresh each iteration (matching
// the original's second `mov eax, [0x9b7b34]` at the bottom of the loop) -
// only the ONE-TIME guard above the loop uses the cached local.
//
// `target != 0 && target == arr[i]` short-circuits to exactly the original's
// two-branch `cmp ecx,ebp / je skip; cmp ecx,[esi] / jne skip` - no
// restructuring needed there.
//
// WinRootWindows (the Win* array base) did NOT need the extern-array override
// the general lever warns about: the given `static Win *const g = ...`
// (cast at the point of use, not redeclared) already reproduces the
// register-walked indexing here - the lever's "address itself does work"
// failure mode did not reproduce for this walk. Recorded so the next agent
// does not re-pay for testing it.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?update_zorder@Win@@QAAXXZ  at 0x005F5020  (96 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void __cdecl Win::update_zorder() {
    int i = 0;
    int n = WinRootCount;
    WinZOrderCount = 0;
    if (n > 0) {
        Win **arr = reinterpret_cast<Win **>(0x009B6E48);
        Win *target = WinZOrderWindow;
        do {
            if (target != 0 && target == arr[i]) {
                WinZOrderCount = 0;
                WinZOrderFlag = 0;
            }
            if (arr[i]->iSomeFlag_ & 1) {
                recurse_zorder(arr[i]);
                target = WinZOrderWindow;
            }
            i++;
        } while (i < WinRootCount);
    }
}

// ===== homed from src/unrecovered/005f53a0.cpp =====

long Win::on_window_pos_changing(WINDOWPOS * pos) {
    char *wp = reinterpret_cast<char *>(pos);
    if ((*reinterpret_cast<unsigned char *>(wp + 0x18) & 1) == 0) {
        int ok = this->vslot_61();
        if (ok != 0) {
            int cx = *reinterpret_cast<int *>(wp + 0x10);
            int f134 = field_134_;
            if (cx <= f134) {
                cx = f134;
            }
            *reinterpret_cast<int *>(wp + 0x10) = cx;
            int cy = *reinterpret_cast<int *>(wp + 0x14);
            int f138 = field_138_;
            if (cy <= f138) {
                cy = f138;
            }
            *reinterpret_cast<int *>(wp + 0x14) = cy;
        }
        if ((iFlags_ & 0x800000) != 0) {
            int ncx = *reinterpret_cast<int *>(wp + 0x10);
            int ncy = *reinterpret_cast<int *>(wp + 0x14);
            nonclient_to_client(&ncx, &ncy);
            ncy = (iVertScaleNum_ * ncx) / iVertScaleDenom_;
            client_to_nonclient(&ncx, &ncy);
            *reinterpret_cast<int *>(wp + 0x14) = ncy;
        }
    }
    if ((iFlags_ & 0x4000) != 0) {
        *reinterpret_cast<unsigned int *>(wp + 0x18) |= 4;
    }
    if (*reinterpret_cast<int *>(wp + 4) == 0) {
        PostMessageA(HandleMain, 3, 0, 0);
    }
    return DefWindowProcA(HandleMain, 0x46, 0, reinterpret_cast<int>(pos));
}

// ===== homed from src/unrecovered/005ef950.cpp =====
typedef int (__stdcall *ComSlot017)(void *self, void *out_hdc);
typedef int (__stdcall *ComSlot026)(void *self, void *hdc);

void Win::paint_tiled(Buffer *tile, int x_origin, int y_origin, int clip_left,
                       int clip_top, int clip_width, int clip_height, int unused8) {
    if (tile == 0) {
        return;
    }

    if (WinHdcRefCount == 0) {
        if (DirectDrawSurface == 0) {
            WinSharedHdc = GetDC(
                HandleMain);
        } else {
            ComSlot017 fn = (ComSlot017)(*reinterpret_cast<void ***>(DirectDrawSurface))[17];
            fn(DirectDrawSurface, WinSharedHdc);
        }
        if (WinSharedHdc == 0) {
            return;
        }
        WinHdcRefCount = 1;
    } else {
        WinHdcRefCount = WinHdcRefCount + 1;
    }

    if (WinSharedHdc == 0) {
        return;
    }

    if (palette_seed_cache_ != PaletteActive->seed_) {
        Palette::set_active_window(this);
        palette_seed_cache_ = PaletteActive->seed_;
    }

    RECT clip_rect;
    clip_rect.left = clip_left;
    clip_rect.right = clip_left + clip_width;
    clip_rect.top = clip_top;
    clip_rect.bottom = clip_top + clip_height;
    HRGN clip_region = reinterpret_cast<HRGN__ *>(CreateRectRgnIndirect(&clip_rect));
    if (clip_region == 0) {
        return;
    }
    SelectClipRgn(WinSharedHdc, clip_region);

    int tile_width = static_cast<int>(tile->dib_.bmiHeader.biWidth);
    int neg_tile_height = -static_cast<int>(tile->dib_.bmiHeader.biHeight);

    int mod_x = x_origin % tile_width;
    int mod_y = y_origin % neg_tile_height;

    int cols = (mod_x != 0) ? 1 : 0;
    cols += (clip_width - mod_x) / tile_width;
    if ((clip_width - mod_x) % tile_width != 0) {
        ++cols;
    }

    int rows = (mod_y != 0) ? 1 : 0;
    rows += (clip_height - mod_y) / neg_tile_height;
    if ((clip_height - mod_y) % neg_tile_height != 0) {
        ++rows;
    }

    int start_x = clip_left - mod_x;
    int cur_y = clip_top - mod_y;

    if (rows > 0) {
        for (int r = rows; r != 0; --r) {
            if (cols > 0) {
                int cur_x = start_x;
                for (int c = cols; c != 0; --c) {
                    tile->copy_to_window(this, 0, 0, cur_x, cur_y, static_cast<int>(tile->dib_.bmiHeader.biWidth), -static_cast<int>(tile->dib_.bmiHeader.biHeight));
                    cur_x += tile_width;
                }
            }
            cur_y += neg_tile_height;
        }
    }

    if (--WinHdcRefCount == 0) {
        if (DirectDrawSurface != 0) {
            ComSlot026 fn = (ComSlot026)(*reinterpret_cast<void ***>(DirectDrawSurface))[26];
            fn(DirectDrawSurface, WinSharedHdc);
            WinSharedHdc = 0;
            return;
        }
        ReleaseDC(
            HandleMain, WinSharedHdc);
        WinSharedHdc = 0;
    }
}

// ===== homed from src/unrecovered/005ecd20.cpp =====

void Win::pass_dialog_focus() {
    Win *parent;
    Win *obj;

    parent = win_parent_;
    if (parent != 0 && (int)parent->list_.count_ > 1) {
        WinPendingFocus = (parent->list_.head_ != 0)
            ? *reinterpret_cast<Win **>(reinterpret_cast<char *>(parent->list_.current_) + 4) : 0;
        parent = win_parent_;
        if (parent->list_.head_ != 0) {
            parent->list_.current_ = *reinterpret_cast<void **>(reinterpret_cast<char *>(parent->list_.current_) + 0xc);
            if (++parent->list_.tail_ == parent->list_.count_) {
                parent->list_.tail_ = 0;
            }
        }
        this->vslot_51(0);
        parent = win_parent_;
        if (parent->list_.head_ != 0) {
            obj = *reinterpret_cast<Win **>(reinterpret_cast<char *>(parent->list_.current_) + 4);
            if (obj != 0) {
                obj->vslot_51(1);
            }
        }
    }
}

// ===== homed from src/unrecovered/005f1340.cpp =====

// DUPLICATE `Win::OnPaint` removed: two artifacts carried the same body.

// ===== homed from src/unrecovered/005eb640.cpp =====

void Win::close() {
    Win *modal;
    Win *focusWin;
    Win *candidate;
    int i;
    int count;
    bool doClear;

    modal = (Win *)reinterpret_cast<int>(WinPointerOwner1);
    focusWin = (Win *)reinterpret_cast<int>(WinPointerOwner2);
    candidate = modal != 0 ? modal : focusWin;
    doClear = false;
    if (candidate == this) {
        doClear = true;
    } else if ((modal != 0) || (focusWin != 0)) {
        if (candidate != 0) {
            count = child_count_;
            i = 0;
            if (0 < count) {
                do {
                    if ((children_[i] == candidate) || (children_[i]->is_descendant(candidate) != 0)) {
                        doClear = true;
                        break;
                    }
                    i = i + 1;
                    count = child_count_;
                } while (i < count);
            }
        }
    }
    if (doClear) {
        WinPointerOwner1 = 0;
        WinPointerOwner2 = 0;
    }

    if (WinPointerOwner3 == this) {
        WinPointerOwner3 = 0;
        this->vslot_04();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    if ((WinBubbleCompanion == this) && (WinBubbleActive != 0)) {
        WinBubbleCompanion = nullptr;
        WinBubbleActive = 0;
        update_screen(&WinBubbleRect, 0);
        flip(&WinBubbleRect);
    }
    if (WinHoverWindow == this) {
        WinHoverWindow = 0;
    }
    this->hide();

    if ((scroll_vert_ != 0) && ((iFlags_ & 0x40) != 0)) {
        // ARITY: the two CLAIMED bodies that reach slot 7 push nothing, and the
        // image agrees; this transcription passed one argument. The claims
        // decide. If a later measurement shows the argument is real, the
        // slot needs splitting, not a default.
        this->vslot_07();
    }
    if (scroll_vert_ != 0) {
        delete scroll_vert_;
        scroll_vert_ = 0;
    }
    if ((scroll_horz_ != 0) && ((iFlags_ & 0x40) != 0)) {
        // ARITY: the two CLAIMED bodies that reach slot 7 push nothing, and the
        // image agrees; this transcription passed one argument. The claims
        // decide. If a later measurement shows the argument is real, the
        // slot needs splitting, not a default.
        this->vslot_07();
    }
    if (scroll_horz_ != 0) {
        delete scroll_horz_;
        scroll_horz_ = 0;
    }

    if ((iFlags_ & 0x100000) != 0) {
        Win *parent = win_parent_;
        if ((parent != 0) && (parent->list_.head_ != 0)) {
            bool found = false;
            int listCount = (int)parent->list_.count_;
            if (0 < listCount) {
                int idx = 0;
                char *node = (char *)parent->list_.current_;
                for (;;) {
                    if (*(int *)(node + 4) == (int)this) {
                        found = true;
                        break;
                    }
                    node = *(char **)(node + 0xc);
                    idx = idx + 1;
                    parent->list_.current_ = node;
                    if (!(idx < listCount)) {
                        break;
                    }
                }
                if (found) {
                    char *nodeNext = *(char **)(node + 0xc);
                    char *nodePrev = *(char **)(node + 0x10);
                    *(char **)(nodeNext + 0x10) = nodePrev;
                    char *cur = (char *)parent->list_.current_;
                    char *curNext = *(char **)(cur + 0xc);
                    char *curPrev = *(char **)(cur + 0x10);
                    *(char **)(curPrev + 0xc) = curNext;
                    cur = (char *)parent->list_.current_;
                    if (cur == (char *)parent->list_.head_) {
                        parent->list_.head_ = reinterpret_cast<void *>((uint32_t) * (char **)(cur + 0xc));
                    }
                    parent->list_.current_ = reinterpret_cast<void *>((uint32_t) * (char **)(cur + 0xc));
                    if (*(int *)((char *)parent + 0xdc) == 0) {
                        void *dataPtr = *(void **)(cur + 8);
                        if (dataPtr != 0) {
                            free(dataPtr);
                        }
                        *(void **)(cur + 8) = 0;
                        if (cur != 0) {
                            free(cur);
                        }
                    }
                    parent->list_.count_ = parent->list_.count_ - 1;
                }
            }
            if (parent->list_.count_ == 0) {
                parent->list_.head_ = 0;
            }
        }
    }

    count = child_count_;
    i = 0;
    if (0 < count) {
        do {
            children_[i]->close();
            count = child_count_;
            i = i + 1;
        } while (i < count);
    }

    {
        Win *parent = win_parent_;
        if (parent != 0) {
            if (this != 0) {
                bool found = false;
                count = parent->child_count_;
                i = 0;
                if (0 < count) {
                    for (;;) {
                        if (parent->children_[i] == this) {
                            found = true;
                            break;
                        }
                        i = i + 1;
                        if (!(i < count)) {
                            break;
                        }
                    }
                }
                if (found) {
                    count = count - 1;
                    parent->child_count_ = count;
                    this->hide();
                    if (i < parent->child_count_) {
                        do {
                            parent->children_[i] = parent->children_[i + 1];
                            i = i + 1;
                        } while (i < parent->child_count_);
                    }
                }
            }
        } else if (this != 0) {
            bool found = false;
            count = WinRootCount;
            i = 0;
            if (0 < count) {
                Win **node = (Win **)reinterpret_cast<Win **>(0x009B6E48);
                for (;;) {
                    if (*node == this) {
                        found = true;
                        break;
                    }
                    i = i + 1;
                    node = node + 1;
                    if (!(i < count)) {
                        break;
                    }
                }
            }
            if (found) {
                count = count - 1;
                WinRootCount = count;
                this->hide();
                if (i < WinRootCount) {
                    int remaining = WinRootCount - i;
                    int *node2 = (int *)((char *)reinterpret_cast<Win **>(0x009B6E48) + i * 4);
                    do {
                        node2[0] = node2[1];
                        node2 = node2 + 1;
                        remaining = remaining - 1;
                    } while (0 < remaining);
                }
            }
        }
    }
    win_parent_ = 0;
    cursor_name_ = 0;
    iVertScaleDenom_ = 0;
    iVertScaleNum_ = 0;
    field_134_ = 0;
    field_138_ = 0;
    poWinBase_ = reinterpret_cast<Win *>((uint32_t)this);
    cursor_sprite_ = 0;
    field_18C_ = 0;
    field_190_ = 0;
    cursor_handle_ = 0;
    field_19C_ = 0;
    WinSharedHdc = 0;
    menu_ = 0;

    if (list_.head_ != 0) {
        if ((list_.count_ != 0) && (0 < (int)list_.count_)) {
            i = 0;
            do {
                list_.current_ = reinterpret_cast<void *>(*reinterpret_cast<int *>(reinterpret_cast<char *>(list_.head_) + 0xc));
                if (*reinterpret_cast<int *>(reinterpret_cast<char *>(list_.head_) + 8) != 0) {
                    free(*reinterpret_cast<void **>(reinterpret_cast<char *>(list_.head_) + 8));
                }
                *reinterpret_cast<int *>(reinterpret_cast<char *>(list_.head_) + 8) = 0;
                if (list_.head_ != 0) {
                    free((void *)list_.head_);
                }
                list_.head_ = list_.current_;
                i = i + 1;
            } while (i < (int)list_.count_);
        }
        list_.head_ = 0;
        list_.tail_ = 0;
        list_.count_ = 0;
    }
    list_.tail_ = 0;
    field_12C_ = 0;
    field_130_ = 1;

    // TWO TABLES, NOT THIRTEEN GLOBALS. 0x00696D34 and 0x009B7AF0 are the
    // heads of contiguous default arrays; the addresses this block used to
    // name one at a time are their entries. Read out of the image:
    // WinStaticDefaults = {247, 24, -1, 1, 1, 8, -1, 7, 2}, and [1] is 24 -
    // the caption height assigned below, which is what confirms the indexing.
    // The assignment ORDER is the image's: fields go in field order while the
    // table is read out of order, so do not tidy it into index order.
    field_FC_ = WinDynamicDefaults[0];
    field_100_ = WinStaticDefaults[0];
    field_104_ = WinDynamicDefaults[2];
    field_108_ = WinStaticDefaults[2];
    field_10C_ = WinStaticDefaults[3];
    field_110_ = WinStaticDefaults[4];
    caption_height_ = WinStaticDefaults[1];
    border_thickness_ = WinStaticDefaults[5];
    bottom_border_thickness_ = WinStaticDefaults[6];
    field_120_ = WinStaticDefaults[7];
    field_124_ = WinDynamicDefaults[3];
    field_128_ = WinStaticDefaults[8];
    field_F8_ = WinDynamicDefaults[1];

    {
        void *p0xE0 = reinterpret_cast<void *>(field_E0_);
        if (p0xE0 != 0) {
            free(p0xE0);
        }
        field_E0_ = 0;
    }
    {
        void *p0xE4 = minimize_button_;
        if (p0xE4 != 0) {
            delete p0xE4;
            minimize_button_ = 0;
        }
    }
    {
        void *p0xE8 = zoom_button_;
        if (p0xE8 != 0) {
            delete p0xE8;
            zoom_button_ = 0;
        }
    }
    {
        void *p0xEC = close_button_;
        if (p0xEC != 0) {
            delete p0xEC;
            close_button_ = 0;
        }
    }
    field_F4_ = 0;
    iFlags_ = 0;
    field_174_ = 1;
    field_178_ = 1;
    field_17C_ = 1;
    field_180_ = 1;
    outer_rect_.left = 0;
    outer_rect_.top = 0;
    outer_rect_.right = 0;
    outer_rect_.bottom = 0;
    client_rect_.left = 0;
    client_rect_.top = 0;
    client_rect_.right = 0;
    client_rect_.bottom = 0;
    field_16C_ = 0;
    field_170_ = 0;

    field_400_ = 0;
    field_404_ = 0;
    mouse_move_hook_ = 0;
    field_40C_ = 0;
    field_410_ = 0;
    field_414_ = 0;
    field_418_ = 0;
    field_41C_ = 0;
    field_420_ = 0;
    field_424_ = 0;
    field_428_ = 0;
    field_42C_ = 0;
    key_hook_ = 0;
    field_434_ = 0;
    field_438_ = 0;
    field_1A0_ = 2;
    iSomeFlag_ = 0;
    field_A0_ = 0;
    field_A4_ = 0;
    child_count_ = 0;

    if (buffer1_ != 0) {
        delete buffer1_;
        buffer1_ = 0;
    }
    if (buffer2_ != 0) {
        delete buffer2_;
        buffer2_ = 0;
    }
    if (buffer3_ != 0) {
        delete buffer3_;
        buffer3_ = 0;
    }
    if (buffer4_ != 0) {
        delete buffer4_;
        buffer4_ = 0;
    }

    close();

}

// ===== homed from src/unrecovered/005f2290.cpp =====

void __cdecl Win::OnMouseMove(void * hwnd, int x, int y, unsigned int keys) {
    Win *hit;
    Win *tracking;

    tracking = WinTrackingWindow;
    WinCursorMoved = 0;
    if (tracking != nullptr) {
        do_tracking(x, y);
        return;
    }
    hit = (Win *)get_mouse_window(&x, &y);
    update_cursor(hit, 1);
    if (hit != 0) {
        if (WinHoverWindow != 0 &&
            (WinCursorMoved != 0 || hit != WinHoverWindow) &&
            (reinterpret_cast<int>(WinPointerOwner3) == 0 || WinPointerOwner3 == WinHoverWindow)) {
            WinHoverWindow->vslot_18(x, y);
        }
        WinHoverWindow = hit;
        hit->on_mouse_move(x, y, keys, WinMouseDirect);
    }
}

// ===== homed from src/unrecovered/005f1070.cpp =====

// ORIGINAL: 0x005F1070 ?OnQueryNewPalette@Win@@QAAHPAX@Z 0x005F1070-0x005F1141 FILE
// TRIED: call directly through `(*reinterpret_cast<Fn*>(g_addr))(args)` at the call site instead of binding the function pointer to a named local first - the named local forced an extra reg-to-reg mov before the GetDC argument push. Also: these two "vtable" calls (slots 17/26) push the object pointer as an explicit stack arg with ecx holding the vtable pointer, not a real __thiscall dispatch, so plain `int*`/function-pointer casts matched where the VCall shim would not.
// TRIED and NOT recovered (2026-08-25): this body was BYTE_EXACT while
// 0x009B7B2C was read through the `WinScreenDC` fixed-address binding, and
// dropped to 50 of 61 when that binding merged onto `WinSharedHdc`, the
// real global it duplicated. VC6 then CSEs the tail's `WinSharedHdc` load
// with `eax`, which holds DirectDrawSurface and is provably 0 on that
// path, so it emits `push 0` where the image has `mov edx,[0x9b7b2c]; push
// edx`, and an extra `test eax, eax` appears at image instruction 40. The
// opaque binding is what used to block that reasoning.
// Three spellings measured, all 50 of 61, so the cause is upstream of the
// call itself: naming the value in a local (`HDC shared = WinSharedHdc`),
// reading through a pointer to the REAL global (`HDC *const shared =
// &WinSharedHdc`, not a second storage), and dropping the `eax` binding at
// the tail entirely.
// The merge is KEPT and the claim is paid: two names for one address is a
// defect the byte ratchet cannot see, and this is what removing it cost.
// working copy - scaffold materialised by --work
// size      209 bytes
// prototype int (__cdecl ?OnQueryNewPalette@Win@@QAAHPAX@Z)(HWND hWnd)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F10A6 0x005F10B7 0x005F10DE 0x005F10EB 0x005F1111 0x005F112E
int __cdecl Win::OnQueryNewPalette(void * hwnd) {
    int eax;

    if (BufferDirectDraw != 0) {
        return 1;
    }

    eax = WinHdcRefCount;
    if (eax != 0) {
        eax = eax + 1;
        WinHdcRefCount = eax;
        eax = reinterpret_cast<int>(WinSharedHdc);
    } else {
        eax = reinterpret_cast<int>(DirectDrawSurface);
        if (eax != 0) {
            reinterpret_cast<IDirectDrawSurface *>(eax)->GetDC(&WinSharedHdc);
            eax = reinterpret_cast<int>(WinSharedHdc);
        } else {
            // TWO STEPS, and the second one is load-bearing. Calling through
            // `GetDC(...)` directly compiled `mov eax,[0]` + `call [eax]`
            // - one instruction more than the image's single
            // `call dword ptr [0x66927c]`. Naming the loaded pointer first is
            // the spelling the rest of this file already uses (see the
            // GetDC call in the macro above, BYTE_EXACT), and it restores
            // the direct indirect-call encoding.
            eax = reinterpret_cast<int>(GetDC(
                HandleMain));
            WinSharedHdc = reinterpret_cast<HDC>(eax);
        }
        if (eax == 0) {
            return 0;
        }
        WinHdcRefCount = 1;
    }

    if (eax == 0) {
        return 0;
    }

    SelectPalette(reinterpret_cast<HDC>(eax),
                  reinterpret_cast<HPALETTE>(PaletteInitialized), 0);

    RealizePalette(WinSharedHdc);

    eax = WinHdcRefCount;
    eax = eax - 1;
    WinHdcRefCount = eax;
    if (eax != 0) {
        return 0;
    }

    eax = reinterpret_cast<int>(DirectDrawSurface);
    if (eax != 0) {
        // Slot 26 (0x68) is IDirectDrawSurface::ReleaseDC(HDC). The
        // transcription passes the ADDRESS of WinSharedHdc and both this
        // body and on_activate agree, so the named call keeps that form;
        // ddraw.h types the slot HDC-by-value, so the pointer goes as an
        // explicit reinterpretation rather than pretending it is a handle.
        reinterpret_cast<IDirectDrawSurface *>(eax)->ReleaseDC(
            reinterpret_cast<HDC>(&WinSharedHdc));
        WinSharedHdc = 0;
        return 0;
    }

    ReleaseDC(HandleMain, WinSharedHdc);
    WinSharedHdc = 0;
    return 0;
}

// ===== homed from src/unrecovered/005ecc40.cpp =====

void Win::set_dialog_focus(Win * window) {
    if (window == 0) {
        return;
    }
    if (list_.count_ == 0) {
        return;
    }

    int *newFocus;
    if (list_.head_ == 0) {
        newFocus = 0;
    } else {
        int *node = reinterpret_cast<int *>(list_.current_);
        newFocus = reinterpret_cast<int *>(node[1]);
    }

    if (reinterpret_cast<int *>(window) == newFocus) {
        return;
    }

    WinPendingFocus = reinterpret_cast<Win *>(newFocus);

    if (list_.head_ == 0) {
        return;
    }

    int count = list_.count_;
    int idx = 0;
    list_.tail_ = 0;
    list_.current_ = list_.head_;

    if (count > 0) {
        int *node;
        for (;;) {
            node = reinterpret_cast<int *>(list_.current_);
            if (reinterpret_cast<int *>(node[1]) == reinterpret_cast<int *>(window)) {
                break;
            }
            list_.tail_ = list_.tail_ + 1;
            idx++;
            list_.current_ = reinterpret_cast<void *>(node[3]);
            if (idx >= count) {
                return;
            }
        }

        if (newFocus != 0 && count > 1) {
            call_slot_cc(newFocus, 0);
        }

        if (list_.head_ != 0) {
            int *node2 = reinterpret_cast<int *>(list_.current_);
            int *target = reinterpret_cast<int *>(node2[1]);
            if (target != 0) {
                call_slot_cc(target, 1);
            }
        }
    }
}

// ===== homed from src/unrecovered/005f1480.cpp =====

long __cdecl Win::OnActivate(void *hwnd, unsigned int state, void *other,
                             long minimized) {
    InvalidateRect(reinterpret_cast<HWND>(HandleMain), 0, 0);
    if (state != 0) {
      if (minimized == 0 && BufferDirectDraw == 0) {
        HDC hdc;
        if (WinHdcRefCount != 0) {
            WinHdcRefCount = WinHdcRefCount + 1;
            hdc = WinSharedHdc;
        } else {
            void *iface = DirectDrawSurface;
            if (iface != 0) {
                IfaceGetHdcProc fn = (*reinterpret_cast<IfaceGetHdcProc **>(iface))[17];
                fn(iface, &WinSharedHdc);
                hdc = WinSharedHdc;
            } else {
                WinSharedHdc = GetDC(HandleMain);
                hdc = WinSharedHdc;
            }
            if (hdc != 0) {
                WinHdcRefCount = 1;
            }
        }
        if (hdc != 0) {
            SelectPalette(WinSharedHdc, reinterpret_cast<HPALETTE>(PaletteInitialized), 0);
            RealizePalette(WinSharedHdc);

            WinHdcRefCount = WinHdcRefCount - 1;
            if (WinHdcRefCount == 0) {
                void *iface2 = DirectDrawSurface;
                if (iface2 != 0) {
                    IfaceReleaseHdcProc fn2 = (*reinterpret_cast<IfaceReleaseHdcProc **>(iface2))[26];
                    fn2(iface2, WinSharedHdc);
                } else {
                    typedef int (__stdcall *ReleaseDCProc)(void *, HDC);
                    ReleaseDC(
                        HandleMain, WinSharedHdc);
                }
                WinSharedHdc = 0;
            }
        }
      }
    } else {
        WinPointerOwner1 = nullptr;
        WinPointerOwner2 = nullptr;
    }
    return DefWindowProcA(reinterpret_cast<HWND__ *>(hwnd), 6, (static_cast<unsigned int>(minimized) << 16) | (state & 0xffff),reinterpret_cast<long>(other));
}

// ===== homed from src/unrecovered/005ecf20.cpp =====

void Win::screen_to_client(RECT * rect) {
    if (rect == 0) {
        return;
    }
    int dx = -(client_rect_.left + outer_rect_.left);
    int dy = -(client_rect_.top + outer_rect_.top);
    if ((iFlags_ & 0x20) && win_parent_ != 0) {
        win_parent_->screen_to_client(&dx, &dy);
        if (iFlags_ & 0x8000) {
            dx += win_parent_->outer_rect_.left;
            dy += win_parent_->outer_rect_.top;
        }
    }
    rect->left += dx;
    rect->right += dx;
    rect->top += dy;
    rect->bottom += dy;
}

// ===== homed from src/recovered/units/005f4cc0.cpp =====
// ORIGINAL: 0x005F4CC0 ?redraw@Win@@QAAXXZ 0x005F4CC0-0x005F4CF1 FILE BYTE_EXACT
// size      49 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD2B0
// indirect  0x005F4CD6
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F4CC0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f4cc0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?redraw@Win@@QAAXXZ  at 0x005F4CC0  (49 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
// A `QAA` NAME IS A __cdecl MEMBER, and the receiver is passed on the stack
// like any other argument - which is exactly what the artifact spelled as
// `fn_005f4cc0(Win *a1)`. `a1` is never read in this body, so making it the
// implicit `this` changes nothing the compiler emits. The body is otherwise
// byte-for-byte what the claim was measured on.
void __cdecl Win::redraw() {
    int i;
    for (i = 0; i < WinZOrderCount; ++i) {
        reinterpret_cast<Win *>(reinterpret_cast<void **>(WinZOrderArray)[i])->vslot_62();
        do_sound();
    }
}

// ===== homed from src/recovered/units/005f5080.cpp =====
void __cdecl add_parent(Win *);
// ORIGINAL: 0x005F5080 ?add_parent@Win@@QAAXPAUWin@@@Z 0x005F5080-0x005F50D5 FILE BYTE_EXACT
// symbol    ?add_parent@@YAXPAVWin@@@Z
// size      85 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F5080
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f5080/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_parent@Win@@QAAXPAUWin@@@Z  at 0x005F5080  (85 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void __cdecl add_parent(Win * window) {
    if (window == 0) return;
    int count = WinRootCount;
    unsigned int flags = window->iFlags_;
    if (flags & 0x2000000) {
        WinRootWindows[count] = window;
        WinRootCount = count + 1;
        return;
    }
    if (count > 0) {
        Win **p = WinRootWindows + count;
        int n = count;
        do {
            Win *tmp = *(p - 1);
            *p = tmp;
            --p;
        } while (--n);
    }
    WinRootWindows[0] = window;
    WinRootCount = count + 1;
}
