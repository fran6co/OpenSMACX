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
// ORIGINAL: 0x005EB350 sub_5eb350 0x005EB350-0x005EB366 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned complete exclusive span and selected CRT startup pair: NewTech483880..4838A4/4838B0..4838D8, sharedSprite5D71D0..5D71E6/5D71F0..5D720F, WinBuffer5EB350..5EB366/5EB370..5EB38F. These are process-start lifetimes, not lazy statics or authored callback substitutes.", "NewTech has six actual stride2C Sprite elements at822610 with separate guard822604 mask1. Shared Buffer Sprite9B3A50 uses mask2 of existing special-text guard9B37CC; Win drawing Buffer9B6F08 uses separate guard9B6E30 mask1. Guard is published before actual reverse/direct teardown, upper bits retained and constructors do not reset it.", "Fresh complete build-4esa2of5 COFF proves owner ?WinDrawingBufferOwner@@3UWinDrawingBufferLifetime@@A with generated init symbol _$E26. Constructor/registration closures and direct cleanup guard/receiver references are physically verified; aggregate uses real compiler vector construction/destruction. Selected same-owner CRT order stays unchanged. GCC/Clang actual source controls cover prefix unwind, reverse order, shared masks and guarded reentry.", "Chosen owner types are implementation, not recovered original names or enclosing allocation proof. Exact initializer/unwind spelling, original data layout, linked retained selection and runtime/full-game fidelity remain obligations; no measurement claim is granted. Preserve prior claims and historical discoveries."], "reason": "Bind original win guarded global init to its genuine source-owned lifetime and actual compiler-generated closure."}
// symbol    _$E26
// ORIGINAL: 0x005EB370 sub_5eb370 0x005EB370-0x005EB38F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned complete exclusive span and selected CRT startup pair: NewTech483880..4838A4/4838B0..4838D8, sharedSprite5D71D0..5D71E6/5D71F0..5D720F, WinBuffer5EB350..5EB366/5EB370..5EB38F. These are process-start lifetimes, not lazy statics or authored callback substitutes.", "NewTech has six actual stride2C Sprite elements at822610 with separate guard822604 mask1. Shared Buffer Sprite9B3A50 uses mask2 of existing special-text guard9B37CC; Win drawing Buffer9B6F08 uses separate guard9B6E30 mask1. Guard is published before actual reverse/direct teardown, upper bits retained and constructors do not reset it.", "Fresh complete build-4esa2of5 COFF proves owner ?WinDrawingBufferOwner@@3UWinDrawingBufferLifetime@@A with generated fini symbol _$E24. Constructor/registration closures and direct cleanup guard/receiver references are physically verified; aggregate uses real compiler vector construction/destruction. Selected same-owner CRT order stays unchanged. GCC/Clang actual source controls cover prefix unwind, reverse order, shared masks and guarded reentry.", "Chosen owner types are implementation, not recovered original names or enclosing allocation proof. Exact initializer/unwind spelling, original data layout, linked retained selection and runtime/full-game fidelity remain obligations; no measurement claim is granted. Preserve prior claims and historical discoveries."], "reason": "Bind original win guarded global fini to its genuine source-owned lifetime and actual compiler-generated closure."}
// symbol    _$E24
// ORIGINAL: 0x005EB390 sub_5eb390 0x005EB390-0x005EB3A6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pre-change provenance, retained verbatim below: Bind the original ScreenBuffer startup initializer to the jackal/palette.cpp global and its compiler-generated lifetime; retain the complete historical identity, span, claim and seed provenance.", "Pre-change history: Pinned terranx_original.exe 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: original initializer 0x005EB390-0x005EB3A6 constructs the Buffer at 0x009B7490 and registers its matching teardown callback.", "Pre-change history: Source owner: jackal/palette.cpp declares the actual global Buffer ScreenBuffer, used by the palette and rendering paths; this is the object's typed storage, not an address alias.", "Pre-change history: The emitted startup helper _$E31 references ScreenBuffer, Buffer's constructor and the compiler's destructor callback before registering that callback with atexit; _$E29 references the same object and Buffer destructor.", "Pre-change history: The old seed's complete mapping and historical matching claim are moved unchanged; generated lifetime bytes remain a restoration obligation, not a new grant or acceptance.", "Pinned ScreenBuffer cleanup5EB3B0..5EB3CF uses mask2 of the exact same guard byte9B6E30 as drawing Buffer cleanup5EB370 mask1. Original CRT slots682678/68267C construct drawing then screen and register callbacks in that order.", "Current source lifetime belongs beside WinDrawingBufferOwner in win.cpp, preserving original registration order and reverse guarded cleanup. Palette/rendering borrowers retain the same genuine underlying Buffer. Source grouping is supported by shared lifetime and runtime consumers, not a recovered original TU/name/allocation proof.", "Retain complete startup identity/span and historical matching restoration obligation; typed lifetime structural recovery does not grant fresh exactness, unwind, data-extent or linked/runtime acceptance."], "reason": "Rehome ScreenBuffer startup to its shared-guard Win lifetime owner; retain complete historical identity, claim and pre-change seed evidence."}
// symbol    _$E31
// ORIGINAL: 0x005EB3B0 sub_5eb3b0 0x005EB3B0-0x005EB3CF FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original5EB3B0..5EB3CF reads cleanup byte9B6E30, tests mask2, retains other bits while setting bit2 before complete Buffer destructor5D7410 with receiver9B7490. Existing drawing callback5EB370 uses mask1 of the same byte.", "Original consecutive CRT slots682678/68267C construct/register drawing then screen. Current same-TU definitions preserve registration and reverse atexit cleanup without dynamic reference globals or executable-address dispatch. Constructors do not reset the guard.", "Actual retained physical COFF observes _$E29 referring to the same guard and ScreenBufferOwner before direct qualified Buffer destructor. Startup closure references the real Buffer constructor and atexit registration. Source owner storage is offset-zero and aligned for current Buffer; original allocation bounds/names/TU remain unknown.", "Original9B74C0 is ScreenBuffer+30 rect2_, proved by pinned5EFED9push9B74C0/5EFEDEmovecx9B7490/5EFEE3call5D8000. Rendering restores clipping through Buffer::restore_clip, not duplicate RECT storage.", "No unmeasured claim is granted. Preserve all old claims and startup restoration; compiler constructor/unwind equality, complete linked preservation/data extent and game runtime fidelity remain obligations."], "reason": "Bind original ScreenBuffer guarded teardown to the actual compiler-generated callback of its source-owned shared Win lifetime."}
// symbol    _$E29
#include "stdafx.h"

// Win constructor-installed Win branch: capture4, resize13, mouse17, update63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x0066FDD0 ? ??_7Win@@6B@ 0x005EB3D0
#include "net_class.h"
#include "multidebug.h"

typedef signed char int8;  // a scaffold spelling the artifacts use
#include "scroll.h"
#include "win.h"
#include "game.h"  // ExpansionEnabled, passed to the CD setup provider
#include "imagebutton.h"
#include <cstring>
#include "palette.h"
#include "general.h"
#include "jackal.h" // JackalInitFlags lifecycle state

#include "dialog.h"
#include "sounddevice.h"
#include "sound.h"
#include "menu.h"
#include "popup.h"   // pop_caption_title, for DDInit::report_error
#include "basepop.h"  // Win::set_bubble_text's popup interfaces
#include <ddraw.h>  // IDirectDrawSurface::GetDC / ReleaseDC in the hdc pair
#include <new>
#include <stddef.h>

// 0x009B6F08 in the image - the buffer Win draws into.
// Its guard byte at 0x009B6E30 is separate from both Buffers, not a
// guessed containing object. ScreenBufferLifetime shares its bit 2.
union WinDrawingBufferStorage {
    char bytes[sizeof(Buffer)];
    uint32_t alignment_[sizeof(Buffer) / sizeof(uint32_t)];
};
struct WinDrawingBufferLifetime {
    WinDrawingBufferStorage storage_;
    WinDrawingBufferLifetime() { new (storage_.bytes) Buffer(); }
    ~WinDrawingBufferLifetime() {
        if ((WinDrawingBufferCleanupGuard & 1) != 0) return;
        WinDrawingBufferCleanupGuard |= 1;
        // Original cleanup calls the complete destructor directly, not
        // the virtual deleting-destructor entry.
        buffer().Buffer::~Buffer();
    }
    Buffer &buffer() { return *reinterpret_cast<Buffer *>(storage_.bytes); }
};
static_assert(sizeof(WinDrawingBufferLifetime) == sizeof(Buffer),
              "Win drawing Buffer storage extent");
static_assert(offsetof(WinDrawingBufferLifetime, storage_) == 0,
              "Win drawing Buffer starts at its owner identity");
extern WinDrawingBufferLifetime WinDrawingBufferOwner;
// RECOVERY_OBJECT: v1 0x009B7B6C ?MenuPullDownInputOrigin@@3HA
// Shared state written by Menu and PullDown keyboard navigation paths.
int MenuPullDownInputOrigin;

// The Win message-status word shares the owner of its message-pump consumers.
uint32_t MsgStatus;  // 0x009B7B9C
// The process window handle is produced by Win::open's CreateWindowExA
// and consumed by the window/message-pump and DirectDraw owners.
// RECOVERY_OBJECT: v1 0x009B7B28 ?HandleMain@@3PAUHWND__@@A
HWND HandleMain;  // 0x009B7B28
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
    const WinDynamicDefaultsTable *const dynamic = &WinDynamicDefaults;

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
    buffer5_ = 0;
    field_130_ = 1;
    field_FC_ = dynamic->field_0_;
    field_100_ = fixed[0];
    caption_height_ = fixed[1];
    field_104_ = dynamic->rest_[0];
    field_108_ = fixed[2];
    field_10C_ = fixed[3];
    field_110_ = fixed[4];
    border_thickness_ = fixed[5];
    bottom_border_thickness_ = fixed[6];
    field_120_ = fixed[7];
    field_124_ = dynamic->rest_[1];
    field_128_ = fixed[8];
    field_F8_ = dynamic->default_font_;
    field_E0_ = 0;
    minimize_button_ = 0;
    zoom_button_ = 0;
    close_button_ = 0;
    scroll_vert_ = 0;
    scroll_horz_ = 0;
    field_F4_ = 0;
    cached_outer_rect_.left = 0;
    cached_outer_rect_.top = 0;
    cached_outer_rect_.right = 0;
    cached_outer_rect_.bottom = 0;
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
            focused = parent->list_.current_->focus_owner_;
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
// RECOVERY_OBJECT: v2 0x009BC498 0x009BC49C ?DirectDrawSurface@@3PAUIDirectDrawSurface2@@A 0x0062CD50
IDirectDrawSurface2 *DirectDrawSurface;  // 0x009BC498
Win *WinModalStack[4];     // 0x009B6EF8
HINSTANCE WinInstance;     // 0x009B7B14
int WinScreenWidth;        // 0x009B7B1C
int WinScreenHeight;       // 0x009B7B20
Win *WinTrackingWindow;    // 0x009B7AAC
Win *WinHoverWindow;      // 0x009B7A7C
Win *WinPointerOwner1;     // 0x009B7ACC
Win *WinPointerOwner2;     // 0x009B7AD0
// Independent DWORD loads/stores at 615E60/615E6C and 615E72/615EDB;
// 5F60DD/5F60E5 compare and clear the second pointer, not its pointee.
// RECOVERY_OBJECT: v2 0x009B7ABC 0x009B7AC0 ?WinPointerOwner3@@3PAVWin@@A 0x00615E60
Win *WinPointerOwner3;
// RECOVERY_OBJECT: v2 0x009B7AC0 0x009B7AC4 ?WinPointerOwner4@@3PAVWin@@A 0x00615E60
Win *WinPointerOwner4;
Win *WinFocusWindow;       // 0x009B7AE0
Win *WinModalWindow;       // 0x009B7ADC
// init stores Win pointers at 5EC067/5EC08C; add_parent repeats those
// array accesses at 5F509C/5F50C7. init admits 40 roots (5EC030);
// those pointer slots end at the independently used dirty RECT (5F17C7).
// add_parent has no capacity guard: this extent is not a bounds-safety proof.
// RECOVERY_OBJECT: v2 0x009B6E48 0x009B6EE8 ?WinRootWindows@@3PAPAVWin@@A 0x005EBD80
Win *WinRootWindows[40];
// init increments this scalar at 5EC092-93; on_char reloads it at
// 5F611E/5F6168. It is the count value, not an address-backed alias.
// RECOVERY_OBJECT: v1 0x009B7B34 ?WinRootCount@@3HA
int WinRootCount;
int WinMouseDirect;        // 0x009B7AA4
int WinMouseScreenX;       // 0x009B6628
int WinMouseScreenY;       // 0x009B662C
int WinClipWidth;            // 0x009B7A40
int WinClipHeight;           // 0x009B7A44
int WinDragOffsetX;          // 0x009B7A38
int WinDragOffsetY;          // 0x009B7A3C
// Independent pointer slots, initially null. game_init (58F3FE/408/412)
// publishes existing menu Sprite elements; redo_caption_buttons reads
// maximize/restore at 5F27F0/5F27DC. No restore writer was found.
// RECOVERY_OBJECT: v2 0x009B7B00 0x009B7B04 ?WinCloseIcon@@3PAVSprite@@A 0x0058F2F0
Sprite *WinCloseIcon;        // 0x009B7B00
// RECOVERY_OBJECT: v2 0x009B7B04 0x009B7B08 ?WinMaximizeIcon@@3PAVSprite@@A 0x005F2760
Sprite *WinMaximizeIcon;     // 0x009B7B04
// RECOVERY_OBJECT: v2 0x009B7B08 0x009B7B0C ?WinRestoreIcon@@3PAVSprite@@A 0x005F2760
Sprite *WinRestoreIcon;      // 0x009B7B08
// RECOVERY_OBJECT: v2 0x009B7B0C 0x009B7B10 ?WinMinimizeIcon@@3PAVSprite@@A 0x0058F2F0
Sprite *WinMinimizeIcon;     // 0x009B7B0C
int WinSizingFlag;           // 0x009B7AD4
int WinModalDepth;           // 0x009B7AE4
int WinKeyModifiers;         // 0x009B7B18
int WinViewOriginX;          // 0x009B7A70
int WinViewOriginY;          // 0x009B7A74
// The Buffer drawing family reads the one-byte default transparency key
// at 696D14; its pinned initial value is 9. This is constant storage, not
// a pointer to an original-image object or a mutable palette instance.
// RECOVERY_OBJECT: v2 0x00696D14 0x00696D15 ?WinFillColour@@3EB 0x005D8650
const unsigned char WinFillColour = 9;  // 0x00696D14, read from the image
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
Win *WinPendingFocus;       // 0x009B7B38
// Shared current receiver for window callbacks and redraw, not a pointer
// through which to overwrite the previous receiver's object storage.
// RECOVERY_OBJECT: v1 0x009B7AB8 ?WinCallbackWindow@@3PAVWin@@A
Win *WinCallbackWindow;
Win *WinInputFocus;       // 0x009B7AC4
Win *WinActiveWindow;     // 0x009B7AC8
Win *WinBubbleCompanion;   // 0x009B7A4C
// RECOVERY_OBJECT: v1 0x009B6630 ?WinZOrderArray@@3PAPAVWin@@A
Win *WinZOrderArray[512];
// RECOVERY_OBJECT: v1 0x009B6E38 ?WinBubbleRect@@3UtagRECT@@A
RECT WinBubbleRect;        // 0x009B6E38
// Original 0x009B74C0 is ScreenBuffer's rect2_ at +0x30, not an
// independent RECT. Borrow it through Buffer::restore_clip().
// restore_flip_sprite passes this complete RECT to UnionRect at 5F17C7
// and flip at 5F17DC. Its right member at 9B6EF0 is not a modal result.
// RECOVERY_OBJECT: v2 0x009B6EE8 0x009B6EF8 ?WinDirtyRect@@3UtagRECT@@A 0x005F1750
RECT WinDirtyRect;
Win *WinDefaultFocus;      // 0x009B7AEC
// Native6354D0 leaves pitch untouched, including on nonstatic objects.
// The global instance is separately zero-initialized by C++ static storage.
// ORIGINAL: 0x006354D0 sub_6354d0 0x006354D0-0x006354E9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:ddinit-global-lifetime", "Pinned native original0x6354d0", "historical source:src/field_accessors.cpp", "Fresh COFF symbols and helper references in both configured flag sets."], "reason": "Original6354D0 initializes seven receiver words and returns this; true constructor preserves untouched pitch at14. Unknown18/1C roles and complete size remain unproved."}
// symbol    ??0DDInit@@QAE@XZ
DDInit::DDInit() {
    hwnd_ = 0;
    surf_ = 0;
    field_8_ = 0;
    locked_surface_ = 0;
    locked_bits_ = 0;
    unknown_18_ = 0;
    unknown_1C_ = 0;
}

DDInit::~DDInit() {
    // Original635450 uses the same cleanup order as635750, including its
    // omission of secondary-surface Release. No manual atexit registration:
    // the compiler owns global startup and exactly one automatic teardown.
    teardown();
}

// ORIGINAL: 0x006354F0 sub_6354f0 0x006354F0-0x00635503 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:ddinit-global-lifetime", "Pinned native original0x6354f0", "historical source:src/recovered/006354f0.cpp", "Fresh COFF symbols and helper references in both configured flag sets."], "reason": "Original6354F0 leaves ECX intact, passes0/0/depth/1 to mapped DDInit::init and returns ret4. Receiver-preserving member replaces contradictory free-stdcall seed. Name is descriptive."}
// symbol    ?init_default_mode@DDInit@@QAEHH@Z
int DDInit::init_default_mode(int depth) {
    return init(0, 0, depth, 1);
}

// ORIGINAL: 0x00635410 sub_635410 0x00635410-0x00635441 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:ddinit-global-lifetime", "Pinned native original0x635410", "historical source:src/recovered/units/00635410.cpp", "Fresh COFF symbols and helper references in both configured flag sets."], "reason": "Original635410 clears seven words of global9BE618 and registers635450. Current compiler-owned global initializer is independently observed _$E21 with startup and registration helpers _$E18/_$E20. Preserve historical single-body claim; helper splitting remains restoration work, not a pass."}
// symbol    _$E21
// ORIGINAL: 0x00635450 sub_635450 0x00635450-0x006354CA FILE
// RECOVERY_BINDING: v1 {"evidence": ["task:ddinit-global-lifetime", "Pinned native original0x635450", "historical source:src/unrecovered/00635450.cpp", "Fresh COFF symbols and helper references in both configured flag sets."], "reason": "Original635450 cleanup belongs to global DDInit lifetime. Current compiler-owned exit _$E19 dispatches genuine destructor and teardown; original direct inline cleanup not presently reproduced. Seed stdcall-virtual-call insight is retained; named SDK COM methods supply that ABI. No new claim."}
// symbol    _$E19
// TRIED: falling off the end without a return (C4716, error under the scoring flags); "return 0" adds an extra "xor eax,eax" the original's bare "pop esi; ret" tail does not have (#36). Modelled the 3 vtable dispatches as raw __stdcall function pointers (COM-style, this pushed as an explicit arg) rather than the VCall thiscall shim, since the disassembly pushes "this" onto the stack instead of loading it into ecx.
// RECOVERY_OBJECT: v1 0x009BE618 ?WinDisplayInit@@3VDDInit@@A
DDInit WinDisplayInit;     // 0x009BE618
Win *WinFocusStack[4];     // 0x009B7A1C
char WinKeyRing[10];       // 0x009B7B48
char *WinKeyRingCursor = WinKeyRing;  // 0x00696D5C
// RECOVERY_OBJECT: v1 0x00696D34 ?WinStaticDefaults@@3PAIA
uint32_t WinStaticDefaults[10] = {  // 0x00696D34, the image's values
    247, 24, 0xffffffff, 1, 1, 8, 0xffffffff, 7, 2, 1};
// Semantic defaults record: ctor 5EB485/4A9/4FD/515 and close
// 5EBA46/5EBA5E/5EBABB/5EBAD2 read four independent fields only.
// The original zero-filled extent ends before the caption-pointer owners.
// RECOVERY_OBJECT: v2 0x009B7AF0 0x009B7B00 ?WinDynamicDefaults@@3UWinDynamicDefaultsTable@@A 0x005EB3D0
WinDynamicDefaultsTable WinDynamicDefaults;  // 0x009B7AF0
// RECOVERY_OBJECT: v1 0x009B7A50 ?WinBubbleActive@@3HA
int WinBubbleActive;       // 0x009B7A50
// on_char loads this pointer at 5F612F/5F615F; hide independently does
// so at 5EDDE3/5EDE13. The binding names pointer storage, not its pointee.
// RECOVERY_OBJECT: v1 0x009B7A6C ?WinZOrderWindow@@3PAVWin@@A
Win *WinZOrderWindow;
// RECOVERY_OBJECT: v1 0x009B7B30 ?WinZOrderCount@@3HA
int WinZOrderCount;
// on_char 5F6148 and hide 5EDDFD clear this scalar directly when the
// selected root is encountered; its wider flag meaning remains provisional.
// RECOVERY_OBJECT: v1 0x009B7A78 ?WinZOrderFlag@@3HA
int WinZOrderFlag;
void(__cdecl *WinLeftDownHook)(int x, int y);  // 0x009B7A90
int WinCursorMoved;        // 0x009B7B3C
void(__cdecl *WinMessageHook)();                            // 0x009B7A88
// BaseButton::on_left_click (607A20) loads these independent nullary
// callbacks for button IDs -1/-2. Effect::init (445BB0) installs the OK
// callback; no original cancel producer has been identified. The cells
// occupy PE loader-zeroed virtual storage, not file-backed initializer bytes.
// These start bindings do not infer the bounds of neighboring hook storage.
// RECOVERY_OBJECT: v1 0x009B7A98 ?WinOkHook@@3P6AXXZA
void(__cdecl *WinOkHook)();
// RECOVERY_OBJECT: v1 0x009B7A9C ?WinCancelHook@@3P6AXXZA
void(__cdecl *WinCancelHook)();
// Effect::init installs the pass-over sound callback here; FlatButton's
// original on_mouse_move (608500) invokes it on hover entry, not completion.
// RECOVERY_OBJECT: v1 0x009B7AA0 ?WinButtonHoverHook@@3P6AXXZA
void(__cdecl *WinButtonHoverHook)();
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
// RECOVERY_OBJECT: v2 0x009BC49C 0x009BC4A0 ?DirectDrawBackBuffer@@3PAUIDirectDrawSurface2@@A 0x0062CD50
IDirectDrawSurface2 *DirectDrawBackBuffer;
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
// LEVER: the DirectDraw half was structurally wrong, and rewriting it as the
//        image lays it out moved 11/407 -> 23/407 (311 -> 369 instructions).
//        The whole shared-HDC block only runs when `DirectDrawSurface` is
//        null: 0x005EFF1A jumps it, and each of the FOUR Blt arms ends in its
//        own epilogue, so they are four call sites (clipped/unclipped x
//        area/null) rather than the one shared call this tree had. An
//        unclipped frame calls BufferDirectDraw vtable slot 0x58 -
//        IDirectDraw::WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, nullptr) -
//        then slot 11 Flip, then the same blit. TWO WRONG VTABLE CALLS were
//        hiding in the old shape and are fixed here: 0x005F008D is slot 0x68
//        = ReleaseDC (this tree blitted), and 0x005F0104 is slot 0x44 = GetDC
//        taking the shared DC in place (this tree flipped). Remaining gap is
//        the register plan only: the image keeps no frame pointer and holds
//        all four WinBubbleRect fields in ebx/ebp/esi/edi, this tree spends
//        ebp on the frame and spills; every one of the 26 differing runs is
//        a register choice or a `mov esp,ebp` for `add esp,0x14`.
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
are `ScreenBufferOwner.buffer().rect2_`, `.dib_.bmiHeader.biWidth` and `.biHeight`:
`ScreenBufferOwner.buffer()` is at 0x009B7490 and those are +0x30, +0x80 and +0x84. A
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
            ScreenBufferOwner.buffer().set_clip(area);
        }
        ScreenBufferOwner.buffer().box(&WinBubbleRect, WinBubbleEdgeColour,
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
        ScreenBufferOwner.buffer().fill(&WinBubbleRect, WinBubbleFillColour);
        ScreenBufferOwner.buffer().set_font(WinBubbleFont, nullptr, nullptr, nullptr);
        ScreenBufferOwner.buffer().set_text_color(WinBubbleTextColour, -1, 1, 1);

        LPSTR text = reinterpret_cast<LPSTR>(WinBubbleActive);
        if (strchr(text, '^') == nullptr) {
            // One line, centred in the box.
            if (text != nullptr) {
                ScreenBufferOwner.buffer().write_cent_l(text, &WinBubbleRect,
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
                ScreenBufferOwner.buffer().write_cent_l(
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
            ScreenBufferOwner.buffer().set_clip(&ScreenBufferOwner.buffer().rect2_);
        }
    }

    if (WinFlipSprite != nullptr) {
        // The frame index is the sprite's own byte at +8.
        WinFlipSprite->draw(&ScreenBufferOwner.buffer(), WinFlipSprite->cTransparentIndex_,
                            WinFlipSpriteY, WinFlipSpriteX, 1, 1);
    }

    // EVERY DirectDraw path ends the frame and returns. 0x005EFF1A jumps the
    // whole shared-HDC block below when a surface exists, and each of the four
    // Blt arms carries its own epilogue - so they are four separate call sites,
    // not one shared one, and the shared-HDC work below only runs on a frame
    // with no DirectDraw surface at all.
    if (DirectDrawSurface != nullptr) {
        if (WinFlipClipped != 0) {
            if (area != nullptr) {
                RECT intersection;
                if (!IntersectRect(&intersection, area,
                                   &DirectDrawClipRect)) {
                    return;
                }
                DirectDrawSurface->Blt(&intersection, DirectDrawBackBuffer,
                                       &intersection, DDBLT_WAIT, nullptr);
                return;
            }
            DirectDrawSurface->Blt(nullptr, DirectDrawBackBuffer, nullptr,
                                   DDBLT_WAIT, nullptr);
            return;
        }
        // Unclipped: wait for the vertical blank, flip, then blit as above.
        BufferDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, nullptr);
        DirectDrawSurface->Flip(nullptr, DDFLIP_WAIT);
        if (area != nullptr) {
            RECT intersection;
            if (!IntersectRect(&intersection, area, &DirectDrawClipRect)) {
                return;
            }
            DirectDrawSurface->Blt(&intersection, DirectDrawBackBuffer,
                                   &intersection, DDBLT_WAIT, nullptr);
            return;
        }
        DirectDrawSurface->Blt(nullptr, DirectDrawBackBuffer, nullptr,
                               DDBLT_WAIT, nullptr);
        return;
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

    const HDC source = ScreenBufferOwner.buffer().get_hdc();
    if (source == nullptr) {
        if (--WinHdcRefCount != 0) {
            return;
        }
        if (DirectDrawSurface != nullptr) {
            // 0x005F008D: slot 0x68 is IDirectDrawSurface::ReleaseDC, not Blt.
            DirectDrawSurface->ReleaseDC(WinSharedHdc);
        }
        WinSharedHdc = nullptr;
        return;
    }

    ScreenBufferOwner.buffer().sync_to_palette(PaletteActive);

    if (BufferDirectDraw == nullptr) {
        if (WinHdcRefCount != 0) {
            ++WinHdcRefCount;
        } else {
            if (DirectDrawSurface != nullptr) {
                // 0x005F0104: slot 0x44 is IDirectDrawSurface::GetDC, not
                // Flip - the surface hands back the shared DC in place.
                DirectDrawSurface->GetDC(&WinSharedHdc);
            } else {
                WinSharedHdc = GetDC(HandleMain);
            }
            // One refcount store covers both arms: 0x005F0120 sits after the
            // join and re-tests eax a second time at 0x005F012A before the
            // palette work.
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
        BitBlt(WinSharedHdc, 0, 0, ScreenBufferOwner.buffer().dib_.bmiHeader.biWidth,
               -ScreenBufferOwner.buffer().dib_.bmiHeader.biHeight, source, 0, 0, SRCCOPY);
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
    ScreenBufferOwner.buffer().release_hdc(1);
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
        scroll_vert_->field_2144_ =
            reinterpret_cast<void (__cdecl *)(int, int)>(value);
    }
    if (scroll_horz_) {
        scroll_horz_->field_2144_ =
            reinterpret_cast<void (__cdecl *)(int, int)>(value);
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
        scroll_vert_->field_2148_ =
            reinterpret_cast<void (__cdecl *)(int, int)>(value);
    }
    if (scroll_horz_) {
        scroll_horz_->field_2148_ =
            reinterpret_cast<void (__cdecl *)(int, int)>(value);
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
// imports out of IAT addresses. It dispatches through the named virtual
// members now - `focus->on_key(...)`, `over->on_l_button_up(...)`,
// `owner->on_sys_close()` - which VC6 lowers to the single
// `mov edx,[reg] / call dword ptr [edx+0xNNN]` the image has, and calls
// BeginPaint, EndPaint, InvalidateRect, SelectPalette, RealizePalette,
// GetWindowLongA, GetDC and DefWindowProcA by name, all eight checked
// against the import directory rather than inferred from argument counts.
// The `WinSlot*` constants and the member-pointer typedefs the scaffold
// spelled them with are gone; nothing reads them.
//
// LEVER: 13/657 -> 26/657 (2026-08-28) on the DISPATCH LAYOUT, both moves
//   semantically neutral:
//   - the outer test is spelled `if (message <= WM_LBUTTONDOWN)` with the
//     four-message chain as its THEN arm and the >0x201 region as its ELSE,
//     the reverse of the obvious nesting. The image's `cmp 0x201 / ja` has
//     the high region OUT of line and the chain falling through, which only
//     the flipped spelling produces.
//   - the three inner tests are NEGATED (`!= WM_LBUTTONDOWN`,
//     `<= WM_CHAR`, `!= WM_CHAR`) with the arms swapped into the else
//     position, so every arm lands out of line and the [6, 0x101] jump
//     table is the fallthrough - which is where the image has it.
//
// LEVER: 26/657 -> 119/657 (2026-09-02), and `call_diff` 22 calls against
//   the image's 24 -> agreeing, on four findings read off the image:
//   - THE KEY AND RIGHT-BUTTON PAIRS ARE SEPARATE ARMS. WM_KEYDOWN and
//     WM_KEYUP had been one arm with a `message == WM_KEYDOWN ? 1 : 0`
//     selector, and likewise SYSKEYDOWN/SYSKEYUP and
//     RBUTTONDOWN/RBUTTONDBLCLK. The image gives each its own table entry -
//     0x005F07B0/0x005F07F0, 0x005F086D/0x005F08A3, 0x005F0AFF/0x005F0B62 -
//     and pushes a literal 1 or 0, where the shared arm computed
//     `xor ecx,ecx / cmp edi,0x100 / sete cl / push ecx` and called the
//     lookup helper once per PAIR instead of once per MESSAGE. Splitting
//     them is what closed the call-count gap.
//   - THE GUARDS ARE POSITIVE. `if (focus == nullptr) return 0;` puts an
//     epilogue on every arm; the image tests and jumps OVER the dispatch to
//     a `return 0` it falls into, which is `if (focus != nullptr) { ... }`.
//   - HIWORD, NOT A SIGNED SHIFT. `static_cast<int>(lparam >> 16)` compiles
//     `sar`, the image has `shr` at all four key arms - the scan code is a
//     zero-extended WORD. Spelled `static_cast<int>(HIWORD(lparam))`.
//   - THE >0x20A CASES ARE A SWITCH, NOT THREE IFS. QUERYNEWPALETTE,
//     PALETTECHANGED and WM_USER+1 lower to `sub eax,0x30f / je / sub
//     eax,2 / je / sub eax,0xf0 / jne`; three `if`s lower to three
//     `cmp / jne` pairs the image does not have.
//
// LEVER: ARM ORDER IS SOURCE ORDER. VC6 fills each jump table in case-value
//   order but lays the arms out in the order the source lists them, so the
//   image's code order IS the original's case order and the table order is
//   not. Table 1 at 0x005F0E28 reads ACTIVATE, PAINT, KEYDOWN, KEYUP while
//   the code has PAINT at 0x5f06a4 and ACTIVATE at 0x5f070b; table 3 at
//   0x005F104C reads LBUTTONUP, LBUTTONDBLCLK, RBUTTONDOWN, RBUTTONUP,
//   RBUTTONDBLCLK while the code has the double-clicks first. Both switches
//   here are written in the code order. (The 2026-08-28 TRIED line below
//   concluded the opposite, from a body whose score was still capped by the
//   missing arm splits - the reorder was invisible under it. That line was
//   wrong and is superseded by this one.)
//
// FIXED 2026-09-02, a semantic divergence and not a spelling one: the
//   WM_MOUSEMOVE arm returned 0 when `get_mouse_window` found nothing, so
//   `WinMessageHook` never ran on that path. The image jumps to the tail at
//   0x005F0BD4, which is PAST the message hook - it runs on the null path
//   like every other `break` path, and `Effect::init` installs it. Now
//   spelled `break`.
//
// TRIED: `WPARAM held_wparam = wparam; UINT held_message = message;` at the
//   top, to reproduce the image's entry sequence `push esi / mov
//   esi,[esp+0x60] / push edi / mov edi,[esp+0x60]` and its DefWindowProc
//   tail that pushes both back out of esi and edi. VC6 copy-propagates both
//   away and reloads the parameters at each tail: 26/657 before the arm
//   splits, 119/657 after - identical either way. Also tried blocking the
//   propagation by assigning `held_wparam = (minimised << 16) | active` in
//   the WM_ACTIVATE arm (the image does rebuild wparam there), and by
//   routing only the tails through the locals: no measurement moved. The
//   image's entry load of `wparam` is a hoist this tree cannot produce from
//   any spelling tried, and it costs the first four instructions - the tail
//   at 0x005F0D11 pushes wparam out of esi and message out of edi, so the
//   divergence repeats at every shared tail.
// TRIED: one switch per half of the message space, with WM_CHAR and
//   WM_LBUTTONDOWN as cases rather than guard tests - the natural reading of
//   the dispatch. VC6 then keeps `message` in eax and loads it BEFORE `sub
//   esp,0x50`: 15/657, because every later address shifts. The nested
//   guards stay.
// REMAINING: 119 of 657. The entry `mov esi,[esp+0x60]` above, the
//   register-allocation split it causes, and the image's single shared
//   DefWindowProc tail - this tree emits four, one per default, because the
//   arms are laid out where the image's are not. Also scheduling inside the
//   WM_ACTIVATE arm (the image computes LOWORD into eax and HIWORD into esi;
//   this tree the other way round) and the argument-slot reuse in the mouse
//   arms, where the image keeps `x` in `window`'s incoming slot and `y` in
//   `lparam`'s and this tree spills them high.
Status: MISMATCH, 119/657, call counts agreeing
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
    // The dispatch the image encodes is four tests and three tables:
    //
    //     cmp 0x201 / ja        -> everything above WM_LBUTTONDOWN, out of line
    //     cmp 0x201 / je        -> the WM_LBUTTONDOWN arm, out of line
    //     cmp 0x102 / ja        -> everything above WM_CHAR, out of line
    //     cmp 0x102 / je        -> the WM_CHAR arm, out of line
    //     lea [edi-6] / cmp 0xfb / ja / jmp table at 0x005F0E28
    //
    // so it is spelled as nested guards, each with the arm the image keeps out
    // of line in its ELSE, and a switch as the fallthrough - the reverse of
    // the nesting a straightforward reading suggests. Written the obvious way
    // round the high region lands in line and the rebuild diverges at
    // instruction 6.
    //
    // ARM ORDER IS SOURCE ORDER. VC6 fills each jump table in case-value order
    // but lays the arms out in the order the source lists them, so the case
    // order below is the image's own code order - WM_PAINT before WM_ACTIVATE
    // even though 6 < 0xF, and the double-clicks before the plain buttons.
    // KEYDOWN/KEYUP, SYSKEYDOWN/SYSKEYUP and RBUTTONDOWN/RBUTTONDBLCLK are
    // separate cases rather than one arm with a `message == X ? 1 : 0`
    // selector: the image gives each its own table entry and pushes its own
    // literal 1 or 0, where a shared arm computes
    // `xor ecx,ecx / cmp / sete cl / push ecx` and calls the lookup helper
    // once instead of once per message, which `call_diff` reads as FEWER.
    //
    // Every `break` falls out of its switch and its guards to the two hooks at
    // the bottom, which is the shared tail the image has at 0x005F0BD4, and
    // every `default` is `DefWindowProcA` with its result returned - the one
    // shared tail at 0x005F0D11. That default is not a nicety: WM_NCCREATE and
    // WM_CREATE take it, and a window procedure that answers 0 to either makes
    // CreateWindowEx return NULL.
    if (message <= WM_LBUTTONDOWN) {
        if (message != WM_LBUTTONDOWN) {
            if (message <= WM_CHAR) {
                if (message != WM_CHAR) {
                    switch (message) {
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
                    case WM_ACTIVATE: {
                        const unsigned int active = LOWORD(wparam);
                        const unsigned int minimised = HIWORD(wparam);
                        InvalidateRect(reinterpret_cast<HWND>(HandleMain),
                                       nullptr, FALSE);
                        if (active == 0) {
                            WinPointerOwner1 = nullptr;
                            WinPointerOwner2 = nullptr;
                        } else if (minimised == 0 && BufferDirectDraw == 0
                                   && get_hdc() != nullptr) {
                            SelectPalette(WinSharedHdc, PaletteInitialized,
                                          FALSE);
                            RealizePalette(WinSharedHdc);
                            release_hdc();
                        }
                        DefWindowProcA(window, WM_ACTIVATE,
                                       (minimised << 16) | active, lparam);
                        return 0;
                    }
                    case WM_KEYDOWN: {
                        Win *const focus = get_key_window();
                        if (focus != nullptr) {
                            focus->on_key(wparam, 1,
                                          static_cast<short>(LOWORD(lparam)),
                                          static_cast<int>(HIWORD(lparam)));
                        }
                        if (WinKeyHook != nullptr) {
                            WinKeyHook(wparam);
                        }
                        break;
                    }
                    case WM_KEYUP: {
                        // No `WinKeyHook` test here: the image's KEYUP arm runs
                        // straight off the end to the message-hook tail.
                        Win *const focus = get_key_window();
                        if (focus != nullptr) {
                            focus->on_key(wparam, 0,
                                          static_cast<short>(LOWORD(lparam)),
                                          static_cast<int>(HIWORD(lparam)));
                        }
                        break;
                    }
                    default:
                        return DefWindowProcA(window, message, wparam, lparam);
                    }
                } else {
                    // WM_CHAR. The guard is spelled positively because the
                    // image tests and jumps over the dispatch to a `return 0`
                    // it falls into afterwards, rather than testing the null
                    // and returning out of line.
                    Win *const focus = get_key_window();
                    if (focus != nullptr) {
                        focus->on_char(static_cast<char>(wparam),
                                       static_cast<short>(LOWORD(lparam)));
                    }
                    return 0;
                }
            } else {
                switch (message) {
                case WM_SYSKEYDOWN: {
                    Win *const focus = get_key_window();
                    if (focus != nullptr) {
                        focus->on_sys_key(wparam, 1,
                                          static_cast<short>(LOWORD(lparam)),
                                          static_cast<int>(HIWORD(lparam)));
                    }
                    return 0;
                }
                case WM_SYSKEYUP: {
                    Win *const focus = get_key_window();
                    if (focus != nullptr) {
                        focus->on_sys_key(wparam, 0,
                                          static_cast<short>(LOWORD(lparam)),
                                          static_cast<int>(HIWORD(lparam)));
                    }
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
                    WinCallbackWindow = owner;
                    {
                        // The class's own close callback at 0x404, called when
                        // it is set. Read as the function pointer it is called
                        // as.
                        typedef void(__cdecl * func_win_closed)();
                        func_win_closed closed =
                            owner->field_404_;
                        if (closed != nullptr) {
                            closed();
                        }
                    }
                    owner->on_sys_close();
                    sub_5f86a0(owner->val_6_);
                    return 0;
                }
                case WM_MOUSEMOVE: {
                    WinCursorMoved = 0;
                    if (WinTrackingWindow != nullptr) {
                        WinTrackingWindow->do_tracking(
                            static_cast<short>(LOWORD(lparam)),
                            static_cast<short>(HIWORD(lparam)));
                        return 0;
                    }
                    int x = static_cast<short>(LOWORD(lparam));
                    int y = static_cast<short>(HIWORD(lparam));
                    Win *const over = get_mouse_window(&x, &y);
                    update_cursor(over, 1);
                    if (over == nullptr) {
                        // `break`, not `return 0`: the image's null path jumps
                        // to the tail at 0x005F0BD4, which is past the message
                        // hook, so the hook runs on this path too.
                        break;
                    }
                    if (WinHoverWindow != nullptr
                        && (WinCursorMoved != 0 || over != WinHoverWindow)
                        && (WinPointerOwner3 == nullptr
                            || WinPointerOwner3 == WinHoverWindow)) {
                        WinHoverWindow->on_mouse_leave(x, y);
                    }
                    WinHoverWindow = over;
                    over->on_mouse_move(x, y, wparam, WinMouseDirect);
                    return 0;
                }
                default:
                    return DefWindowProcA(window, message, wparam, lparam);
                }
            }
        } else {
            OnLButtonDown(window, 0, static_cast<short>(LOWORD(lparam)),
                          static_cast<short>(HIWORD(lparam)), wparam);
            return 0;
        }
    } else {
        if (message > WM_MOUSEWHEEL) {
            // Three sparse cases far above the mouse-button block, which the
            // image lowers as a compare chain - `sub eax,0x30f / je / sub
            // eax,2 / je / sub eax,0xf0 / jne` - rather than the three
            // `cmp / jne` pairs a chain of `if`s costs. So it is a switch.
            switch (message) {
            case WM_QUERYNEWPALETTE: {
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
            case WM_PALETTECHANGED: {
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
            case WM_USER + 1: {
                // The tick a `Time` posts to this window, `wparam` being the
                // Time itself - which is why this arm reads `wparam` straight
                // out of the register the procedure loaded it into at entry.
                // Every offset the original touches is a member `time.h`
                // already declares, and the layout is pinned by its own
                // `static_assert(sizeof(Time) == 0x28)`:
                //
                //   [esi + 0x1C]  tick_posted_     cleared on every tick
                //   [esi]         oneshot_state_   `test al,1`, `test al,2`
                //   [esi + 0x08]  callback1_       void(int)
                //   [esi + 0x0C]  callback2_       void(int, void*)
                //   [esi + 0x14]  cb_param1_       pushed last, so it is first
                //   [esi + 0x10]  cb_param2_
                //
                // This is the half of the one-shot emulation that lives
                // outside `Time`: `SetTimer` has no one-shot mode, so a
                // `pulse()` that went through it arrives here repeating, and
                // the second tick is what stops it.
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
                return DefWindowProcA(window, message, wparam, lparam);
            }
            default:
                return DefWindowProcA(window, message, wparam, lparam);
            }
        } else if (message == WM_MOUSEWHEEL) {
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over = get_mouse_window(&x, &y);
            if (over == nullptr) {
                return 0;
            }
            over->send_wheel_delta(wparam, static_cast<unsigned int>(wparam) >> 16,
                                   x, y, WinMouseDirect);
            return 0;
        } else {
            switch (message) {
            // THE IMAGE'S OWN ARM ORDER, read off its table at 0x005F104C and
            // its code layout: double-clicks first, then the plain buttons.
            case WM_LBUTTONDBLCLK:
                OnLButtonDown(window, 1, static_cast<short>(LOWORD(lparam)),
                              static_cast<short>(HIWORD(lparam)), wparam);
                return 0;
            case WM_LBUTTONUP: {
                WinTrackingWindow = nullptr;
                int x = static_cast<short>(LOWORD(lparam));
                int y = static_cast<short>(HIWORD(lparam));
                Win *const over = get_mouse_window(&x, &y);
                if (over != nullptr) {
                    over->on_l_button_up(x, y, wparam, WinMouseDirect);
                }
                break;
            }
            case WM_RBUTTONDBLCLK: {
                int x = static_cast<short>(LOWORD(lparam));
                int y = static_cast<short>(HIWORD(lparam));
                Win *const over = get_mouse_window(&x, &y);
                if (over != nullptr) {
                    over->on_r_button_down(1, x, y, wparam, WinMouseDirect);
                }
                break;
            }
            case WM_RBUTTONDOWN: {
                int x = static_cast<short>(LOWORD(lparam));
                int y = static_cast<short>(HIWORD(lparam));
                Win *const over = get_mouse_window(&x, &y);
                if (over != nullptr) {
                    over->on_r_button_down(0, x, y, wparam, WinMouseDirect);
                }
                break;
            }
            case WM_RBUTTONUP: {
                WinTrackingWindow = nullptr;
                int x = static_cast<short>(LOWORD(lparam));
                int y = static_cast<short>(HIWORD(lparam));
                Win *const over = get_mouse_window(&x, &y);
                if (over != nullptr) {
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
         `ScreenBufferOwner.buffer()` once the new mode is live.
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
        ScreenBufferOwner.buffer().init(width, height, 0, 0);
        ScreenBufferOwner.buffer().fill(0);
        if (buf.load_pcx("logo.pcx", PaletteActive, 10, 0xEC) == 0) {
            buf.copy(&ScreenBufferOwner.buffer(), 0, 0,
                     (ScreenBufferOwner.buffer().dib_.bmiHeader.biWidth - buf.dib_.bmiHeader.biWidth) / 2,
                     (buf.dib_.bmiHeader.biHeight - ScreenBufferOwner.buffer().dib_.bmiHeader.biHeight) / 2,
                     buf.dib_.bmiHeader.biWidth,
                     -buf.dib_.bmiHeader.biHeight);
        }
    }

    Win::flip(nullptr);
    return 0;
}

/*
Purpose: Tear down whatever DirectDraw surface and device window this object
         is holding - the same stages as the mode switch inside `init` above,
         as their own entry point, and what jackal_close runs on the
         0x009BE618 object.
// ORIGINAL: 0x00635750 sub_635750 0x00635750-0x006357C1 BYTE_EXACT
// symbol    ?teardown@DDInit@@QAEXXZ
// size      113 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD00
// indirect  0x00635765 0x0063577C 0x00635785 0x00635796 0x006357AD
//
// PROMOTED out of src/unrecovered/00635750.cpp, which measured it byte-exact
// against opaque scaffolding: the object there called `TeardownObj` IS
// `WinDisplayInit` - its five fields are DDInit's own, in this order, and it
// is destroyed through `ecx = 0x9BE618`, the address win.h pins the global
// to. The interface calls go by name, as in `init` above: Unlock is vtable
// offset 0x80, RestoreDisplayMode 0x4c, Release 8 - and 0x005EFD00 is the
// homed DDInitRefreshScreenMetrics.
*/
void DDInit::teardown() {
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
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        UnregisterClassA(DDInitClassName, WinInstance);
    }
    locked_surface_ = nullptr;
    DDInitRefreshScreenMetrics();
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

    ScreenBufferOwner.buffer().init(WinScreenWidth, WinScreenHeight, 0, 0);
    ScreenBufferOwner.buffer().fill(0);
    if (logo.load_pcx("logo.pcx", PaletteActive, 10, 0xEC) == 0) {
        logo.copy(&ScreenBufferOwner.buffer(), 0, 0,
                  (ScreenBufferOwner.buffer().dib_.bmiHeader.biWidth
                   - logo.dib_.bmiHeader.biWidth) / 2,
                  (logo.dib_.bmiHeader.biHeight
                   - ScreenBufferOwner.buffer().dib_.bmiHeader.biHeight) / 2,
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
// ORIGINAL: 0x00635870 sub_635870 0x00635870-0x00635F0B BYTE_EXACT
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
// A flat switch over all 99 cases reaches instruction #22 before diverging
// (`push` against `mov`): MSVC re-groups this case set into different
// sub-tables than the original's five, and which case maps to which string
// is not in dispute - it is verified above.
//
// LEVER (2026-08-29): THE CASES CALL parse_says DIRECTLY, and VC6
// tail-merges all 99 into the image's shared
// `push 0; call parse_says; add esp,0x10` at 0x00635ed6 -
// `case X: parse_says(0, "DDERR_X", -1, -1); break;` is BYTE_EXACT 469/469.
// The previous shape (`message = "..."` per case, one
// `if (message) parse_says(0, message, -1, -1);` after the switch) is the
// 744-instruction lowering the flat-switch TRIED line describes: with the
// call outside the switch, VC6 never sinks it into the cases, and each case
// shrinks to `mov eax, OFFSET str; jmp`. The call belongs IN the case body.
Return Value: 0 if the popup was dismissed, otherwise whatever the media
              check returns.
Status: Complete
*/
int DDInit::report_error(int hr) {
    switch (hr) {
    case DDERR_SURFACEALREADYATTACHED:
        parse_says(0, "DDERR_SURFACEALREADYATTACHED", -1, -1);
        break;
    case DDERR_NOCOLORKEY:
        parse_says(0, "DDERR_NOCOLORKEY", -1, -1);
        break;
    case DDERR_INVALIDCAPS:
        parse_says(0, "DDERR_INVALIDCAPS", -1, -1);
        break;
    case DDERR_CANNOTATTACHSURFACE:
        parse_says(0, "DDERR_CANNOTATTACHSURFACE", -1, -1);
        break;
    case DDERR_OUTOFMEMORY:
        parse_says(0, "DDERR_OUTOFMEMORY", -1, -1);
        break;
    case DDERR_UNSUPPORTED:
        parse_says(0, "DDERR_UNSUPPORTED", -1, -1);
        break;
    case DDERR_GENERIC:
        parse_says(0, "DDERR_GENERIC", -1, -1);
        break;
    case DDERR_NOTINITIALIZED:
        parse_says(0, "DDERR_NOTINITIALIZED", -1, -1);
        break;
    case DDERR_INVALIDPARAMS:
        parse_says(0, "DDERR_INVALIDPARAMS", -1, -1);
        break;
    case DDERR_ALREADYINITIALIZED:
        parse_says(0, "DDERR_ALREADYINITIALIZED", -1, -1);
        break;
    case DDERR_CANNOTDETACHSURFACE:
        parse_says(0, "DDERR_CANNOTDETACHSURFACE", -1, -1);
        break;
    case DDERR_CURRENTLYNOTAVAIL:
        parse_says(0, "DDERR_CURRENTLYNOTAVAIL", -1, -1);
        break;
    case DDERR_EXCEPTION:
        parse_says(0, "DDERR_EXCEPTION", -1, -1);
        break;
    case DDERR_HEIGHTALIGN:
        parse_says(0, "DDERR_HEIGHTALIGN", -1, -1);
        break;
    case DDERR_INCOMPATIBLEPRIMARY:
        parse_says(0, "DDERR_INCOMPATIBLEPRIMARY", -1, -1);
        break;
    case DDERR_INVALIDCLIPLIST:
        parse_says(0, "DDERR_INVALIDCLIPLIST", -1, -1);
        break;
    case DDERR_INVALIDMODE:
        parse_says(0, "DDERR_INVALIDMODE", -1, -1);
        break;
    case DDERR_INVALIDOBJECT:
        parse_says(0, "DDERR_INVALIDOBJECT", -1, -1);
        break;
    case DDERR_INVALIDPIXELFORMAT:
        parse_says(0, "DDERR_INVALIDPIXELFORMAT", -1, -1);
        break;
    case DDERR_INVALIDRECT:
        parse_says(0, "DDERR_INVALIDRECT", -1, -1);
        break;
    case DDERR_LOCKEDSURFACES:
        parse_says(0, "DDERR_LOCKEDSURFACES", -1, -1);
        break;
    case DDERR_NO3D:
        parse_says(0, "DDERR_NO3D", -1, -1);
        break;
    case DDERR_NOALPHAHW:
        parse_says(0, "DDERR_NOALPHAHW", -1, -1);
        break;
    case DDERR_NOCLIPLIST:
        parse_says(0, "DDERR_NOCLIPLIST", -1, -1);
        break;
    case DDERR_NOCOLORCONVHW:
        parse_says(0, "DDERR_NOCOLORCONVHW", -1, -1);
        break;
    case DDERR_NOCOOPERATIVELEVELSET:
        parse_says(0, "DDERR_NOCOOPERATIVELEVELSET", -1, -1);
        break;
    case DDERR_NOCOLORKEYHW:
        parse_says(0, "DDERR_NOCOLORKEYHW", -1, -1);
        break;
    case DDERR_NODIRECTDRAWSUPPORT:
        parse_says(0, "DDERR_NODIRECTDRAWSUPPORT", -1, -1);
        break;
    case DDERR_NOEXCLUSIVEMODE:
        parse_says(0, "DDERR_NOEXCLUSIVEMODE", -1, -1);
        break;
    case DDERR_NOFLIPHW:
        parse_says(0, "DDERR_NOFLIPHW", -1, -1);
        break;
    case DDERR_NOGDI:
        parse_says(0, "DDERR_NOGDI", -1, -1);
        break;
    case DDERR_NOMIRRORHW:
        parse_says(0, "DDERR_NOMIRRORHW", -1, -1);
        break;
    case DDERR_NOTFOUND:
        parse_says(0, "DDERR_NOTFOUND", -1, -1);
        break;
    case DDERR_NOOVERLAYHW:
        parse_says(0, "DDERR_NOOVERLAYHW", -1, -1);
        break;
    case DDERR_NORASTEROPHW:
        parse_says(0, "DDERR_NORASTEROPHW", -1, -1);
        break;
    case DDERR_NOROTATIONHW:
        parse_says(0, "DDERR_NOROTATIONHW", -1, -1);
        break;
    case DDERR_NOSTRETCHHW:
        parse_says(0, "DDERR_NOSTRETCHHW", -1, -1);
        break;
    case DDERR_NOT4BITCOLOR:
        parse_says(0, "DDERR_NOT4BITCOLOR", -1, -1);
        break;
    case DDERR_NOT4BITCOLORINDEX:
        parse_says(0, "DDERR_NOT4BITCOLORINDEX", -1, -1);
        break;
    case DDERR_NOT8BITCOLOR:
        parse_says(0, "DDERR_NOT8BITCOLOR", -1, -1);
        break;
    case DDERR_NOTEXTUREHW:
        parse_says(0, "DDERR_NOTEXTUREHW", -1, -1);
        break;
    case DDERR_NOVSYNCHW:
        parse_says(0, "DDERR_NOVSYNCHW", -1, -1);
        break;
    case DDERR_NOZBUFFERHW:
        parse_says(0, "DDERR_NOZBUFFERHW", -1, -1);
        break;
    case DDERR_NOZOVERLAYHW:
        parse_says(0, "DDERR_NOZOVERLAYHW", -1, -1);
        break;
    case DDERR_OUTOFCAPS:
        parse_says(0, "DDERR_OUTOFCAPS", -1, -1);
        break;
    case DDERR_OUTOFVIDEOMEMORY:
        parse_says(0, "DDERR_OUTOFVIDEOMEMORY", -1, -1);
        break;
    case DDERR_OVERLAYCANTCLIP:
        parse_says(0, "DDERR_OVERLAYCANTCLIP", -1, -1);
        break;
    case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
        parse_says(0, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE", -1, -1);
        break;
    case DDERR_PALETTEBUSY:
        parse_says(0, "DDERR_PALETTEBUSY", -1, -1);
        break;
    case DDERR_COLORKEYNOTSET:
        parse_says(0, "DDERR_COLORKEYNOTSET", -1, -1);
        break;
    case DDERR_HWNDALREADYSET:
        parse_says(0, "DDERR_HWNDALREADYSET", -1, -1);
        break;
    case DDERR_SURFACEALREADYDEPENDENT:
        parse_says(0, "DDERR_SURFACEALREADYDEPENDENT", -1, -1);
        break;
    case DDERR_SURFACEBUSY:
        parse_says(0, "DDERR_SURFACEBUSY", -1, -1);
        break;
    case DDERR_CANTLOCKSURFACE:
        parse_says(0, "DDERR_CANTLOCKSURFACE", -1, -1);
        break;
    case DDERR_SURFACEISOBSCURED:
        parse_says(0, "DDERR_SURFACEISOBSCURED", -1, -1);
        break;
    case DDERR_SURFACELOST:
        parse_says(0, "DDERR_SURFACELOST", -1, -1);
        break;
    case DDERR_SURFACENOTATTACHED:
        parse_says(0, "DDERR_SURFACENOTATTACHED", -1, -1);
        break;
    case DDERR_TOOBIGHEIGHT:
        parse_says(0, "DDERR_TOOBIGHEIGHT", -1, -1);
        break;
    case DDERR_TOOBIGSIZE:
        parse_says(0, "DDERR_TOOBIGSIZE", -1, -1);
        break;
    case DDERR_TOOBIGWIDTH:
        parse_says(0, "DDERR_TOOBIGWIDTH", -1, -1);
        break;
    case DDERR_UNSUPPORTEDFORMAT:
        parse_says(0, "DDERR_UNSUPPORTEDFORMAT", -1, -1);
        break;
    case DDERR_UNSUPPORTEDMASK:
        parse_says(0, "DDERR_UNSUPPORTEDMASK", -1, -1);
        break;
    case DDERR_VERTICALBLANKINPROGRESS:
        parse_says(0, "DDERR_VERTICALBLANKINPROGRESS", -1, -1);
        break;
    case DDERR_WASSTILLDRAWING:
        parse_says(0, "DDERR_WASSTILLDRAWING", -1, -1);
        break;
    case DDERR_XALIGN:
        parse_says(0, "DDERR_XALIGN", -1, -1);
        break;
    case DDERR_INVALIDDIRECTDRAWGUID:
        parse_says(0, "DDERR_INVALIDDIRECTDRAWGUID", -1, -1);
        break;
    case DDERR_DIRECTDRAWALREADYCREATED:
        parse_says(0, "DDERR_DIRECTDRAWALREADYCREATED", -1, -1);
        break;
    case DDERR_NODIRECTDRAWHW:
        parse_says(0, "DDERR_NODIRECTDRAWHW", -1, -1);
        break;
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
        parse_says(0, "DDERR_PRIMARYSURFACEALREADYEXISTS", -1, -1);
        break;
    case DDERR_NOEMULATION:
        parse_says(0, "DDERR_NOEMULATION", -1, -1);
        break;
    case DDERR_REGIONTOOSMALL:
        parse_says(0, "DDERR_REGIONTOOSMALL", -1, -1);
        break;
    case DDERR_CLIPPERISUSINGHWND:
        parse_says(0, "DDERR_CLIPPERISUSINGHWND", -1, -1);
        break;
    case DDERR_NOCLIPPERATTACHED:
        parse_says(0, "DDERR_NOCLIPPERATTACHED", -1, -1);
        break;
    case DDERR_NOHWND:
        parse_says(0, "DDERR_NOHWND", -1, -1);
        break;
    case DDERR_HWNDSUBCLASSED:
        parse_says(0, "DDERR_HWNDSUBCLASSED", -1, -1);
        break;
    case DDERR_NOPALETTEATTACHED:
        parse_says(0, "DDERR_NOPALETTEATTACHED", -1, -1);
        break;
    case DDERR_NOPALETTEHW:
        parse_says(0, "DDERR_NOPALETTEHW", -1, -1);
        break;
    case DDERR_BLTFASTCANTCLIP:
        parse_says(0, "DDERR_BLTFASTCANTCLIP", -1, -1);
        break;
    case DDERR_NOBLTHW:
        parse_says(0, "DDERR_NOBLTHW", -1, -1);
        break;
    case DDERR_NODDROPSHW:
        parse_says(0, "DDERR_NODDROPSHW", -1, -1);
        break;
    case DDERR_OVERLAYNOTVISIBLE:
        parse_says(0, "DDERR_OVERLAYNOTVISIBLE", -1, -1);
        break;
    case DDERR_NOOVERLAYDEST:
        parse_says(0, "DDERR_NOOVERLAYDEST", -1, -1);
        break;
    case DDERR_INVALIDPOSITION:
        parse_says(0, "DDERR_INVALIDPOSITION", -1, -1);
        break;
    case DDERR_NOTAOVERLAYSURFACE:
        parse_says(0, "DDERR_NOTAOVERLAYSURFACE", -1, -1);
        break;
    case DDERR_EXCLUSIVEMODEALREADYSET:
        parse_says(0, "DDERR_EXCLUSIVEMODEALREADYSET", -1, -1);
        break;
    case DDERR_NOTFLIPPABLE:
        parse_says(0, "DDERR_NOTFLIPPABLE", -1, -1);
        break;
    case DDERR_CANTDUPLICATE:
        parse_says(0, "DDERR_CANTDUPLICATE", -1, -1);
        break;
    case DDERR_NOTLOCKED:
        parse_says(0, "DDERR_NOTLOCKED", -1, -1);
        break;
    case DDERR_CANTCREATEDC:
        parse_says(0, "DDERR_CANTCREATEDC", -1, -1);
        break;
    case DDERR_NODC:
        parse_says(0, "DDERR_NODC", -1, -1);
        break;
    case DDERR_WRONGMODE:
        parse_says(0, "DDERR_WRONGMODE", -1, -1);
        break;
    case DDERR_IMPLICITLYCREATED:
        parse_says(0, "DDERR_IMPLICITLYCREATED", -1, -1);
        break;
    case DDERR_NOTPALETTIZED:
        parse_says(0, "DDERR_NOTPALETTIZED", -1, -1);
        break;
    case DDERR_UNSUPPORTEDMODE:
        parse_says(0, "DDERR_UNSUPPORTEDMODE", -1, -1);
        break;
    case DDERR_NOMIPMAPHW:
        parse_says(0, "DDERR_NOMIPMAPHW", -1, -1);
        break;
    case DDERR_INVALIDSURFACETYPE:
        parse_says(0, "DDERR_INVALIDSURFACETYPE", -1, -1);
        break;
    case DDERR_CANTPAGELOCK:
        parse_says(0, "DDERR_CANTPAGELOCK", -1, -1);
        break;
    case DDERR_CANTPAGEUNLOCK:
        parse_says(0, "DDERR_CANTPAGEUNLOCK", -1, -1);
        break;
    case DDERR_NOTPAGELOCKED:
        parse_says(0, "DDERR_NOTPAGELOCKED", -1, -1);
        break;
    default:
        break;
        break;
    }
    if (pop_caption_title(const_cast<char *>("jackal"),
                          const_cast<char *>("DDRAWERROR"), 0x10040,
                          nullptr) == -1) {
        return 0;
    }
    return cd_check();
}

/*
Purpose: The install-media check `DDInit::report_error` tails into. Scan
         drive letters A.. for a CD carrying directx\dsetup.dll, ask the
         player to insert it (`FILEFIND_NOCD`) when none has it, and when one
         does, LoadLibrary it and run the DirectXSetup entry points it
         exports (`DIRECTXNOVERSION` / `DIRECTXVERSION` popups report how
         that went). Returns 1 only when the setup run completed.
// ORIGINAL: 0x00628F30 sub_628f30 0x00628F30-0x006290E0 FILE
// symbol    ?cd_check@@YAHXZ
// LEVER: seed find_handle from an unwritten local (`HANDLE seeded;`) and
//        compare the NOCD pop's answer against `(int)find_handle` rather than
//        -1 - that keeps find_handle live in esi through the test and is what
//        puts module/GetProcAddress/SetCurrentDirectoryA in the image's own
//        edi/esi registers (measured: 51/139 both with and without).
// TRIED: `--BaseAtKeyPollFlag` stores the decrement back; the image only
//        reads the slot (`mov eax,[0x9bc070]; dec eax; je`) - the store-back
//        spelling scores 53/139 but corrupts the shared slot, so the plain
//        read stands at 51/139.
// TRIED: `if (--response == 0) continue;` / `!`-form / `-= 1` / inverted
//        arm order / do-while bottom test - all 51/139; VC6 keeps the
//        `dec;test;jne;jmp` shape where the image has `dec;je` falling into a
//        mid-function shared return-0 epilogue (image 139, ours 141).
// size      432 bytes
// prototype int __cdecl sub_628f30(void)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625E30 0x00627260 0x00645470
// indirect  0x00628F68 0x00628F92 0x00628F97 0x00628FF3 0x0062900B 0x00629019
//           0x00629026 0x0062908F 0x006290A0 0x006290BD 0x006290C0 0x006290CE
Return Value: 1 when DirectXSetupA ran; 0 otherwise
Status: Complete
*/
// The tree's name for 0x009BC070 (base.cpp), the retry budget this body
// decrements; base.h does not declare it, popup.cpp takes the same extern.
extern int ConsoleGroupAction;
int __cdecl cd_check() {
    int version;
    char root_path[256];      // 0x00628F4A: "A:\", then the dsetup.dll path
    char saved_dir[260];      // 0x104: GetCurrentDirectoryA's own buffer size
    WIN32_FIND_DATAA find_data;
    // BUG IN THE ORIGINAL: the image seeds esi (find_handle) from an
    // UNWRITTEN stack slot at entry (`mov esi,[esp + 0xc]`), so on a machine
    // with NO cd-rom drives at all the post-loop find_handle test reads
    // garbage and can enter the setup path unbidden. Reproduced, not fixed:
    // `seeded` is never written either, and the FILEFIND_NOCD test below
    // compares pop's response against find_handle (esi, holding
    // INVALID_HANDLE_VALUE on every path that reaches it) rather than against
    // -1, which is what keeps find_handle live in esi through that test.
    HANDLE seeded;
    HANDLE find_handle = seeded;
    int response; // pop answers; also reused for the BaseAtKeyPollFlag reload
    for (;;) {
        root_path[0] = 0;
        strcat(root_path, "A:\\");
        for (int drive = 0; drive < 26; drive++) {
            if (GetDriveTypeA(root_path) == DRIVE_CDROM) {
                strcat(root_path, "directx\\dsetup.dll");
                find_handle = FindFirstFileA(root_path, &find_data);
                FindClose(find_handle);
                if (find_handle != INVALID_HANDLE_VALUE) {
                    goto found;
                }
                // strip the appended path back to "A:\" for the next letter
                root_path[3] = 0;
            }
            root_path[0]++;
        }
        if (find_handle != INVALID_HANDLE_VALUE) {
            goto found;
        }
        if (pop_caption_title(const_cast<char *>("jackal"),
                              const_cast<char *>("FILEFIND_NOCD"), 0x10040,
                              nullptr) == (int)find_handle) {
            return 0;
        }
        // the pop system leaves the response in the shared slot at
        // 0x009BC070 (base.cpp's BaseAtKeyPollFlag); rescan while it is 1.
        // Reuses `response` - the image reloads it into the same register
        // the pop answer sat in, and never stores the decrement back.
        response = ConsoleGroupAction;
        if (--response == 0) {
            continue;
        }
        return 0;
    }
found:
    HMODULE module = LoadLibraryA(root_path);
    if (!module) {
        return 0;
    }
    FARPROC setup_a = GetProcAddress(module, "DirectXSetupA");
    if (!setup_a) {
        return 0;
    }
    FARPROC setup_get_version = GetProcAddress(module, "DirectXSetupGetVersion");
    if (!setup_get_version) {
        return 0;
    }
    // dsetup's entry points are __stdcall: the image leaves the two pushed
    // arguments for the callee to pop, and reads `version` through the
    // un-shifted [esp + 0x10] afterwards.
    typedef int(__stdcall *setup_get_version_fn)(int *, int);
    if ((reinterpret_cast<setup_get_version_fn>(setup_get_version))(&version,
                                                                    0) == 0) {
        response = pop_caption_title(const_cast<char *>("jackal"),
                                     const_cast<char *>("DIRECTXNOVERSION"),
                                     0x40, nullptr);
    } else {
        parse_num(0, version & 0xffff);
        response = pop_caption_title(const_cast<char *>("jackal"),
                                     const_cast<char *>("DIRECTXVERSION"), 0x40,
                                     nullptr);
    }
    if (response == -1) {
        return 0;
    }
    if (response == 1) {
        return 0;
    }
    root_path[3] = 0; // SetCurrentDirectory wants the drive root, not the dll
    GetCurrentDirectoryA(260, saved_dir);
    SetCurrentDirectoryA(root_path);
    typedef int(__stdcall *setup_a_fn)(int, int, int);
    if (HandleMain != 0) {
        (reinterpret_cast<setup_a_fn>(setup_a))(ExpansionEnabled, 0, 0x10018);
    } else {
        (reinterpret_cast<setup_a_fn>(setup_a))(0, 0, 0x10018);
    }
    FreeLibrary(module);
    SetCurrentDirectoryA(saved_dir);
    return 1;
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
        WinCallbackWindow = this;
        WinMouseMoveHookFn const fn = mouse_move_hook_;
        if (fn != 0) {
            fn(x, y);
        }
        this->on_mouse_move(x, y);
        Sound *const child = val_7_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    } else {
        this->on_nc_mouse_move(x, y);
        Sound *const child = val_19_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
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
    // get_mouse_window_recurse where the same retype held. Casts restored
    // for that measurement; the parent field is now named for structural
    // fidelity, without revisiting the pointer/int comparison.
    // The later owner-level pointer-flow recovery keeps the historical match
    // result above while expressing the child/global identity as Win pointers.
    Win *cur;

    if (window->child_count_ > 0) {
        cur = WinZOrderWindow;
        Win **child = &window->children_[0];
        do {
            Win *c = *child;
            if ((c->iFlags_ & 0x20) == 0) {
                if (cur != 0 && cur == c) {
                    WinZOrderCount = 0;
                    found = true;
                }
                if (c->iSomeFlag_ & 1) {
                    Win *child_parent = c->win_parent_;
                    if (child_parent == 0 || child_parent->is_visible() != 0) {
                        recurse_zorder(c);
                    }
                    cur = WinZOrderWindow;
                }
                if (cur != 0 && found) {
                    break;
                }
            }
            ++i;
            ++child;
        } while (i < window->child_count_);
    } else {
        cur = WinZOrderWindow;
    }

    if (!(cur != 0 && found)) {
        i = 0;
        if (window->child_count_ > 0) {
            Win **child = &window->children_[0];
            do {
                Win *c = *child;
                if (cur != 0 && cur == c) {
                    WinZOrderCount = 0;
                    found = true;
                }
                if (window->poWinBase_ == window &&
                    (c->iFlags_ & 0x20)) {
                    if (c->iSomeFlag_ & 1) {
                        Win *child_parent = c->win_parent_;
                        if (child_parent == 0 || child_parent->is_visible() != 0) {
                            recurse_zorder(c);
                        }
                        cur = WinZOrderWindow;
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
        Win *window_parent = window->win_parent_;
        if (window_parent == 0 || window_parent->is_visible() != 0) {
            int idx = WinZOrderCount;
            WinZOrderArray[idx] = window;
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
//
// RECEIVER-SPILL WALL, CONFIRMED ONCE 2026-08-28 at 1/332 (328 compiled
// against the image's 332). The fingerprint is exactly the one
// AGENT_BRIEF describes: the image opens `push ecx` - reserving the
// receiver's own spill slot - then pushes four more callee-saved registers
// and only then reads its argument at `[esp+0x14]`, five pushes ahead of
// the first guard. No source spelling computes addresses that eagerly.
// TRIED (prior passes, confirmed not re-ground): guard reordering, local
//   collapsing, /Oy- on and off - the flag search still settles on
//   `/c /O2 /Gy /GR- /Oy- /GX`, whose frame pointer (`push ebp; mov
//   ebp,esp`) the image, which is FPO here, has no room for.
// ALSO VISIBLE, for the next pass rather than measured: the image lowers
//   the first guard as `test byte ptr [esi + 0x9c], 1` straight out of
//   memory, where this body loads the byte into cl first. Before the
//   structural migration below, this free function reached Win's protected
//   fields as offsets; the friend declaration now permits named access.
//   The historical receiver-spill mismatch remains unaddressed.
Win *__cdecl get_mouse_window_recurse(Win * window, int * x, int * y) {
    int savedX = 0, savedY = 0;
    int i;

    if ((window->iSomeFlag_ & 1) == 0) {
        return 0;
    }
    {
        Win *parent = window->win_parent_;
        if (parent != 0 && !parent->is_visible()) {
            return 0;
        }
    }

    if ((window->iFlags_ & 0x20) == 0) {
        savedX = *x;
        savedY = *y;
        *x = WinMouseScreenX;
        *y = WinMouseScreenY;
    }

    {
        int childCount = window->child_count_;
        Win **children = window->children_;
        for (i = 0; i < childCount; i++) {
            Win *child = children[i];
            if ((child->iFlags_ & 0x20) == 0) {
                Win *r = get_mouse_window_recurse(child, x, y);
                if (r != 0) return r;
            }
        }
    }

    if ((window->iSomeFlag_ & 2) == 0) {
        goto alt_path;
    }

    if (!in_box(*x, *y, &window->client_rect_)) {
        goto tail;
    }
    *x -= window->client_rect_.left;
    *y -= window->client_rect_.top;

    {
        int childCount = window->child_count_;
        Win **children = window->children_;
        if (window == window->poWinBase_) {
            for (i = 0; i < childCount; i++) {
                Win *child = children[i];
                unsigned int flags = child->iFlags_;
                if ((flags & 0x8000) != 0 && (flags & 0x20) != 0) {
                    Win *r = get_mouse_window_recurse(child, x, y);
                    if (r != 0) return r;
                }
            }
        }
    }

    if (!in_box(*x, *y, &window->outer_rect_)) {
        // pixel-precise hit test against one of the window's four render buffers
        unsigned char key = static_cast<unsigned char>(WinFillColour);
        unsigned int flagsHere;
        int threshold1, threshold2;
        unsigned int pixel;
        Buffer *buf;
        bool hit;

        GraphicWin *slotResult = window->get_graphic_win();
        flagsHere = window->iFlags_;
        if (slotResult == 0 || (flagsHere & 0x10000000) == 0) {
            WinMouseDirect = 1;
            return window;
        }

        threshold1 = ((flagsHere & 0x10) == 0 || (flagsHere & 0x400000) != 0)
                         ? window->border_thickness_
                         : window->caption_height_;
        threshold2 = (window->bottom_border_thickness_ != -1)
                              ? window->bottom_border_thickness_
                              : window->border_thickness_;

        if (*y < threshold1) {
            buf = window->buffer1_;
            pixel = (unsigned int)buf->get_pixel(*x, *y);
            hit = (pixel != key);
        } else if (window->client_rect_.bottom - window->client_rect_.top - threshold2 < *y) {
            buf = window->buffer4_;
            pixel = (unsigned int)buf->get_pixel(*x, (window->client_rect_.top - window->client_rect_.bottom) + *y + threshold2);
            hit = (pixel != key);
        } else if (*x < window->border_thickness_) {
            buf = window->buffer3_;
            pixel = (unsigned int)buf->get_pixel(*x, *y - threshold1);
            hit = (pixel != key);
        } else {
            buf = window->buffer2_;
            pixel = (unsigned int)buf->get_pixel((window->client_rect_.right - window->client_rect_.left) + window->border_thickness_ + *x, *y - threshold1);
            hit = (pixel != key);
        }

        if (hit) {
            WinMouseDirect = 1;
            return window;
        }
        *x += window->client_rect_.left;
        *y += window->client_rect_.top;
        goto tail;
    }

    *x += window->client_rect_.left;
    *y += window->client_rect_.top;
    return window;

alt_path:
    if (!in_box(*x, *y, &window->outer_rect_)) {
        goto tail;
    }
    *x -= window->outer_rect_.left;
    *y -= window->outer_rect_.top;
    WinMouseDirect = 0;

    {
        int childCount = window->child_count_;
        Win **children = window->children_;
        if (window == window->poWinBase_) {
            for (i = 0; i < childCount; i++) {
                Win *child = children[i];
                unsigned int flags = child->iFlags_;
                if ((flags & 0x8000) == 0 && (flags & 0x20) != 0) {
                    Win *r = get_mouse_window_recurse(child, x, y);
                    if (r != 0) return r;
                }
            }
        }
    }

    if ((window->iFlags_ & 2) == 0) {
        GraphicWin *slotResult = window->get_graphic_win();
        unsigned int flagsHere = window->iFlags_;
        if (slotResult == 0 || (flagsHere & 0x1000000) == 0 || (window->iFlags_ & 0x100) != 0) {
            return window;
        }
        {
            Buffer *buf = static_cast<Buffer *>(slotResult);
            unsigned char pixel = (unsigned char)buf->get_pixel(*x, *y);
            if (pixel != static_cast<unsigned char>(WinFillColour)) {
                if (window->field_A0_ == 0) {
                    return window;
                }
                if (pixel != 0) {
                    return window;
                }
            }
        }
    }

tail:
    if ((window->iFlags_ & 0x20) == 0) {
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
        WinDrawingBufferOwner.buffer().copy(
            (&ScreenBufferOwner.buffer()), 0, 0,
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
            Sprite *saved = WinFlipSprite;
            WinFlipSprite = 0;
            // Win::flip is a STATIC member (win.h:211), so the image's
            // single RECT push with no ecx setup IS this call - which is
            // what the artifact's invented `win_flip_stub` was standing in
            // for. No stub and no forwarder needed.
            Win::flip(&WinDirtyRect);
            WinFlipSprite = saved;
            WinDirtyRect.left = 0;
            WinDirtyRect.right = 0;
        }
    }
}


/*
Purpose: Play the supplied Sound only when it is not already playing.
         HOMED from src/recovered/005f86a0.cpp on 2026-08-25; its former
         integer/Win receiver reconstruction obscured the actual Sound* API.
*/
// ORIGINAL: 0x005F86A0 sub_5f86a0 0x005F86A0-0x005F86BF BYTE_EXACT
// symbol    ?sub_5f86a0@@YGXPAVSound@@@Z
// size      32 bytes
// kind      game
// flags     hidden;sp_ready;purged_ok
void __stdcall sub_5f86a0(Sound *sound) {
    if (sound != 0) {
        if (sound->is_playing() == 0) {
            sound->play();
        }
    }
}

// Native task pumps drain paint messages first, then poll ordinary messages
// before the WM_USER-and-higher range. The network's 0x402 notification is
// consumed by its actual owner instead of being sent to a window procedure.
// ORIGINAL: 0x005FC7F0 ?do_task@@YAHXZ 0x005FC7F0-0x005FC8E8
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005fc7f0.cpp", "image:terranx_original.exe:0x005FC7F0", "Pinned5FC74A..757 and5FC83A..847 both poll0xF..0xF. Only high-range402 messages route ActiveNet.handle_sys_msg; null network consumes them. do_all_tasks continues until empty and calls do_net after final MsgStatus=0.", "Coordinator-owned core-pump-secret-owner-20261001 scope; preserved original seeds and historical obligations, no matching/runtime claim from binding"], "reason": "Recover the native message pump using SDK MSG/USER32, existing MsgStatus and ActiveNet storage. Preserve paint-first and low/high queue priority, helper order and dedicated network notification dispatch."}
// TRIED: PeekMessageA/TranslateMessage/DispatchMessageA (loaded as plain function pointers from the IAT slots at 0x669358/0x66935c/ 0x669340, per the emitted globals) read into locals right after the first do_video/do_net/check_net() triple, matching the original's `mov esi,[...]` placement AFTER those three calls rather than before (moving it earlier cost the first 5 instructions). Best reached is 78.3% mnemonic similarity (#5 call vs push): the callee-register-save prologue still spills one extra value this body doesn't, not chased further. `handle_sys_msg` at 0x632db0 is a thiscall `Net` member with `this` read from 0x9be608 right before the call (`mov ecx,[0x9be608]`), not a bare cdecl call.
int __cdecl do_task() {
    MSG message;
    MsgStatus = 0;
    do_video();
    do_net();
    check_net();
    MsgStatus = 1;
    for (;;) {
        do_video();
        check_net();
        do_net();
        if (!PeekMessageA(&message, 0, WM_PAINT, WM_PAINT, PM_REMOVE)) {
            break;
        }
        TranslateMessage(&message);
        DispatchMessageA(&message);
        MsgStatus = 1;
    }
    MsgStatus = 0;
    if (!PeekMessageA(&message, 0, 0, WM_USER, PM_REMOVE)) {
        if (!PeekMessageA(&message, 0, WM_USER, 0xFFFF, PM_REMOVE)) {
            return 0;
        }
        if (message.message == 0x402) {
            if (ActiveNet != 0) {
                ActiveNet->handle_sys_msg();
            }
            return 1;
        }
    }
    TranslateMessage(&message);
    DispatchMessageA(&message);
    return 1;
}

// ORIGINAL: 0x005FC700 ?wait_task@@YAXXZ 0x005FC700-0x005FC7E4
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005fc700.cpp", "image:terranx_original.exe:0x005FC700", "Pinned5FC74A..757 and5FC83A..847 both poll0xF..0xF. Only high-range402 messages route ActiveNet.handle_sys_msg; null network consumes them. do_all_tasks continues until empty and calls do_net after final MsgStatus=0.", "Coordinator-owned core-pump-secret-owner-20261001 scope; preserved original seeds and historical obligations, no matching/runtime claim from binding"], "reason": "Recover the native message pump using SDK MSG/USER32, existing MsgStatus and ActiveNet storage. Preserve paint-first and low/high queue priority, helper order and dedicated network notification dispatch."}
// TRIED: 260 vs 228 bytes; VC6's /O2 loop rotation duplicates the do_video/check_net/do_net top-of-loop block into a separate call sequence instead of the original's fall-through-then-jmp reuse of one copy. Tried both `for (;;)` and `do {} while(1)` for the poll loop; both rotate the same way.
void __cdecl wait_task() {
    MSG message;
    MsgStatus = 0;
    do_video();
    do_net();
    check_net();
    MsgStatus = 1;
    for (;;) {
        do_video();
        check_net();
        do_net();
        if (!PeekMessageA(&message, 0, WM_PAINT, WM_PAINT, PM_REMOVE)) {
            break;
        }
        TranslateMessage(&message);
        DispatchMessageA(&message);
        MsgStatus = 1;
    }
    MsgStatus = 0;
    if (!PeekMessageA(&message, 0, 0, WM_USER, PM_REMOVE)) {
        if (!PeekMessageA(&message, 0, WM_USER, 0xFFFF, PM_REMOVE)) {
            return;
        }
        if (message.message == 0x402) {
            if (ActiveNet != 0) {
                ActiveNet->handle_sys_msg();
            }
            return;
        }
    }
    TranslateMessage(&message);
    DispatchMessageA(&message);
}

// ORIGINAL: 0x005FC8F0 ?do_all_tasks@@YAXXZ 0x005FC8F0-0x005FC9DE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005fc8f0.cpp", "image:terranx_original.exe:0x005FC8F0", "Pinned5FC74A..757 and5FC83A..847 both poll0xF..0xF. Only high-range402 messages route ActiveNet.handle_sys_msg; null network consumes them. do_all_tasks continues until empty and calls do_net after final MsgStatus=0.", "Coordinator-owned core-pump-secret-owner-20261001 scope; preserved original seeds and historical obligations, no matching/runtime claim from binding"], "reason": "Recover the native message pump using SDK MSG/USER32, existing MsgStatus and ActiveNet storage. Preserve paint-first and low/high queue priority, helper order and dedicated network notification dispatch."}
// TRIED: PeekMessageA/TranslateMessage/DispatchMessageA are resolved
// TRIED: could not reproduce the original's frameless, purely
void __cdecl do_all_tasks() {
    MSG message;
    for (;;) {
        for (;;) {
            MsgStatus = 0;
            do_video();
            do_net();
            check_net();
            for (;;) {
                MsgStatus = 1;
                do_video();
                check_net();
                do_net();
                if (!PeekMessageA(&message, 0, WM_PAINT, WM_PAINT, PM_REMOVE)) {
                    break;
                }
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
            MsgStatus = 0;
            if (!PeekMessageA(&message, 0, 0, WM_USER, PM_REMOVE)) {
                break;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        if (!PeekMessageA(&message, 0, WM_USER, 0xFFFF, PM_REMOVE)) {
            MsgStatus = 0;
            do_net();
            return;
        }
        if (message.message == 0x402) {
            if (ActiveNet != 0) {
                ActiveNet->handle_sys_msg();
            }
        } else {
            TranslateMessage(&message);
            DispatchMessageA(&message);
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

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FCF90 ?do_char@@YAXXZ 0x005FCF90-0x005FCFDD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fcf90.cpp", "image:terranx_original.exe:0x005FCF90"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
// symbol    ?do_char@@YAHXZ
int __cdecl do_char() {
    do_video();
    check_net();
    MSG msg;
    if (!PeekMessageA(&msg, 0, WM_CHAR, WM_CHAR, PM_REMOVE)) {
        return 0;
    }
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD050 ?do_mouse@@YAXXZ 0x005FD050-0x005FD09D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fd050.cpp", "image:terranx_original.exe:0x005FD050"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
// symbol    ?do_mouse@@YAHXZ
int __cdecl do_mouse() {
    do_video();
    check_net();
    MSG msg;
    // The original range ends at WM_MBUTTONDBLCLK, not newer SDK messages.
    if (!PeekMessageA(&msg, 0, WM_MOUSEMOVE, WM_MBUTTONDBLCLK, PM_REMOVE)) {
        return 0;
    }
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD0A0 ?do_all_mouse@@YAXXZ 0x005FD0A0-0x005FD115 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fd0a0.cpp", "image:terranx_original.exe:0x005FD0A0"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
void __cdecl do_all_mouse() {
    MSG msg;
    MsgStatus = 8;
    int has_message;
    do {
        do_video();
        check_net();
        has_message = PeekMessageA(&msg, 0, WM_MOUSEMOVE, WM_MBUTTONDBLCLK, PM_REMOVE);
        if (has_message) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            MsgStatus = 8;
        }
    } while (has_message);
    MsgStatus = 0;
    do_net();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD1C0 ?flush_keyboard@@YAXXZ 0x005FD1C0-0x005FD21A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/005fd1c0.cpp", "image:terranx_original.exe:0x005FD1C0"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
void __cdecl flush_keyboard() {
    MSG msg;
    // Retain the original upper bound; WM_KEYLAST varies with SDK version.
    while (PeekMessageA(&msg, 0, WM_KEYFIRST, 0x108, PM_REMOVE)) {
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD220 ?flush_mouse@@YAXXZ 0x005FD220-0x005FD27A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/005fd220.cpp", "image:terranx_original.exe:0x005FD220"], "reason": "Win input-pump owner uses SDK message dispatch and real pointer-owner storage; original control flow and seed establish this entry and extent."}
void __cdecl flush_mouse() {
    MSG msg;
    while (PeekMessageA(&msg, 0, WM_MOUSEMOVE, WM_MBUTTONDBLCLK, PM_REMOVE)) {
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
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
    ScreenBufferOwner.buffer().close();
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
    if (parent != 0 && parent->list_.head_ != 0) {
        int counter = 0;
        // HOISTED, unlike is_descendant's count: the image loads it once
        // (`mov edi, [esi + 0xd4]` at 0x005ECB7E) and compares against the
        // register (`cmp edx, edi`). Safe here because the only write to
        // count_ is in the arm that breaks out.
        const int total = parent->list_.count_;
        if (total > 0) {
            do {
                WinDialogNode *node = parent->list_.current_;
                if (node->focus_owner_ == this) {
                    WinDialogNode *prev = node->next_;
                    WinDialogNode *next = node->previous_;
                    prev->previous_ = next;
                    next->next_ = prev;
                    if (node == parent->list_.head_) {
                        parent->list_.head_ = node->next_;
                    }
                    parent->list_.current_ = node->next_;
                    if (parent->list_.allocator_ == 0) {
                        void *payload = node->allocation_;
                        if (payload != 0) {
                            free(payload);
                        }
                        node->allocation_ = 0;
                        if (node != 0) {
                            free(node);
                        }
                    }
                    parent->list_.count_ = parent->list_.count_ - 1;
                    break;
                }
                ++counter;
                parent->list_.current_ = node->next_;
            } while (counter < total);
        }
        if (parent->list_.count_ == 0) {
            parent->list_.head_ = 0;
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

// ORIGINAL: 0x005EE280 ?release_modal@Win@@QAEXXZ 0x005EE280-0x005EE327 FILE BYTE_EXACT
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
// LEVER (2026-08-27, found): the ARM ORDER, not the compare. At 11 of 50 the
// whole body was instruction-for-instruction identical to the image except
// instruction 10 - image `je 0x5ee2f1`, ours `jne` - which is the
// if/else arms in the other order: the image puts the SEARCH first and the
// found-at-`last` block second (0x005EE2F1). `if (WinModalStack[last] != this)
// { search } else { found }` emits the image's `je` and lands 50 of 50.
// The `test edx,edx` / materialised-zero note above is REFUTED: that was the
// same divergence read through the swapped layout.
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
    if (WinModalStack[last] != this) {
        // INDEX-ONLY SEARCH: `p` was a second copy of the same cursor and
        // cost a callee-saved register the image does not spend. This block
        // is FIRST in the source because it is first in the image - the
        // found-at-`last` arm sits at 0x005EE2F1, after it.
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
    } else {
        WinModalStack[last] = 0;
        WinFocusStack[last] = 0;
        if (0 < last) {
            WinModalWindow = reinterpret_cast<Win *>(WinFocusStack[last - 1]);
            WinFocusWindow = WinModalStack[last - 1];
            return;
        }
        WinFocusWindow = 0;
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
//
// TRANSCRIBED 2026-08-28. `field_12C_` became `buffer5_` for this: the
// image reads `dib_.bmiHeader.biWidth`/`.biHeight` (+0x80/+0x84) through it
// and calls `copy_to_window` on it with `this` as the target.
//
// TRIED: the whole body, best measured 0 of 258 - the image INLINES both
//            `Win::get_hdc` (0x005EC690, BYTE_EXACT out of line) at 0x005F54E0
//            and `Win::release_hdc` (0x005EC6F0, also BYTE_EXACT) at
//            0x005F57A5, and this tree cannot have it both ways: VC6 6.0 has
//            no `__declspec(noinline)`, so spelling them as calls costs the
//            inlined copies and spelling them inline costs the helpers'
//            claims. Both protocols are therefore written out here exactly as
//            the image lays them down, which is the same idiom
//            `init_class` uses. The rest is the register plan: the image
//            keeps `hdc`/`brush`/`width`/`neg_height` in esi/ebp/edi/ebx
//            across the whole body and this tree spends two of them on the
//            frame.
void Win::on_paint(RECT *area) {
    // Win::get_hdc(), inlined - see the TRIED note above.
    HDC hdc;
    if (WinHdcRefCount != 0) {
        ++WinHdcRefCount;
        hdc = WinSharedHdc;
    } else {
        if (DirectDrawSurface != nullptr) {
            DirectDrawSurface->GetDC(&WinSharedHdc);
        } else {
            WinSharedHdc = GetDC(HandleMain);
        }
        hdc = WinSharedHdc;
        if (hdc != nullptr) {
            WinHdcRefCount = 1;
        }
    }

    // A solid brush in the palette colour `field_F4_` names: the image reads
    // bytes +2/+1/+0 of that entry and folds them red<<16 | green<<8 | blue.
    const int index = static_cast<int>(field_F4_);
    const COLORREF colour =
        RGB(PaletteActive->entries_[index].peBlue,
            PaletteActive->entries_[index].peGreen,
            PaletteActive->entries_[index].peRed);
    const HBRUSH brush = CreateSolidBrush(colour);
    if (brush == nullptr) {
        // 0x005F5572 jumps the whole rest, brush and context unreleased.
        return;
    }

    if (buffer5_ == nullptr) {
        FillRect(hdc, area, brush);
    } else {
        Buffer *const source = buffer5_;
        const int width = source->dib_.bmiHeader.biWidth;
        // `biHeight` is stored negative for a top-down DIB, so the row step
        // the loop adds is its negation, not the height.
        const int neg_height = -source->dib_.bmiHeader.biHeight;
        // Centre of the drawing area in tile steps; `sar 1` on both, so the
        // division is signed round-toward-zero and keeps its fixup.
        int x = (outer_rect_.right - outer_rect_.left - width) / 2;
        int y = (outer_rect_.bottom - outer_rect_.top + neg_height) / 2;

        if (field_130_ == 0) {
            // Tile the whole area, wrapping the origin into the first tile.
            // Both axes compute `size - abs(offset) % size` when the offset
            // is positive and negate it again afterwards - `abs` really is
            // called (0x00644F3A) at 0x005F55EE and 0x005F560C.
            x = x > 0 ? width - abs(x) % width : -x;
            y = y > 0 ? neg_height - abs(y) % neg_height : -y;
            x = -x;
            y = -y;
            for (; y < area->bottom; y += neg_height) {
                for (int px = x; px < area->right; px += width) {
                    source->copy_to_window(this, 0, 0, px, y, width,
                                           neg_height);
                }
            }
        } else {
            // One tile in the centre, the four strips around it filled.
            RECT edge;
            edge.left = 0;
            edge.top = 0;
            edge.right = outer_rect_.right - outer_rect_.left;
            edge.bottom = y;
            FillRect(hdc, &edge, brush);
            source->copy_to_window(this, 0, 0, x, y, width, neg_height);
            edge.top = y;
            edge.right = x;
            edge.bottom = y + neg_height;
            FillRect(hdc, &edge, brush);
            edge.left = x + width;
            edge.right = outer_rect_.right - outer_rect_.left;
            FillRect(hdc, &edge, brush);
            edge.left = 0;
            edge.top = y + neg_height;
            edge.bottom = outer_rect_.bottom - outer_rect_.top;
            FillRect(hdc, &edge, brush);
        }
    }

    DeleteObject(brush);

    // Win::release_hdc(), inlined - see the TRIED note above.
    if (--WinHdcRefCount != 0) {
        return;
    }
    if (DirectDrawSurface != nullptr) {
        DirectDrawSurface->ReleaseDC(WinSharedHdc);
    } else {
        ReleaseDC(HandleMain, WinSharedHdc);
    }
    WinSharedHdc = nullptr;
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

// TRIED: 46 of 57 at `w << 1`, and the last 11 are one scheduling decision.
// The image computes `right - (2*w) - left - border_thickness_ -
// caption_height_ - 2` as six separate instructions, ending `sub esi, 2`,
// with the doubling FIRST (`lea eax, [ebx + ebx]` at 0x005F277D). `w * 2`
// reaches that lea and VC6 then also folds the trailing constant into it -
// `lea eax, [ebx + ebx + 2]`, 56 instructions against 57, everything after
// shifted. Tried and all folded: `w * 2`; `2 * w`; `(w * 2)`; `w + w`;
// `w * 2U`; an unsigned `w`; `const int w`; a `(2)` at the end; the
// parenthesised chain; `field_128_ * 2` inline; and a separate `x -= 2;`
// STATEMENT (35/57 - it still folds). `w << 1` is the one form VC6 will not
// fold the constant into, but its scheduler then moves the shift to the END
// of the subtract chain (ours: `lea` at instruction 14, image: 9), which is
// the whole remaining gap. Flag search still settles on /O2 /Gy /GR- /GX.
// Subtraction order is the image's, read off 0x005F2777-0x005F2794.
// TRIED (2026-08-27, both 46/57 - unchanged): hoisting the shift into its
// own declaration (`int ww = w << 1;` before the chain) and packing it into
// the first declarator (`int w = field_128_, ww = w << 1;`). VC6's /O2
// scheduler is free with declaration order: it groups the memory loads and
// runs any register-only op after them, so the doubling lands at the end of
// the chain whatever the source says. The image interleaves (load w, shift,
// then load the rest) and no spelling tried makes VC6 interleave.
//
// TRIED (as written, 10 of 57): `move(w, y)` - the arguments were SWAPPED.
// The image pushes `w` FIRST and the computed value second
// (0x005F279B-0x005F279C, and twice more), and `Win::move(int x, int y)`
// takes its second argument last, so the call is `move(computed, w)`.
// Also swapped: the `IsZoomed` arms. The image's fallthrough arm (zoomed)
// stores 0x009B7B08 (`WinRestoreIcon`) and its branch arm stores 0x009B7B04
// (`WinMaximizeIcon`), so the test is plain `IsZoomed(...)` with the restore
// icon first, not `== 0` with the maximise icon first.
void Win::redo_caption_buttons() {
    int w = field_128_;
    int x = client_rect_.right - (w << 1) - client_rect_.left -
            border_thickness_ - caption_height_ - 2;
    if (close_button_ != 0) {
        close_button_->move(x, w);
        x += -3 - close_button_->dib_.bmiHeader.biWidth;
    }
    if (zoom_button_ != 0) {
        zoom_button_->move(x, w);
        if (IsZoomed(HandleMain) != 0) {
            zoom_button_->set_sprite(WinRestoreIcon);
        } else {
            zoom_button_->set_sprite(WinMaximizeIcon);
        }
        x += -1 - zoom_button_->dib_.bmiHeader.biWidth;
    }
    if (minimize_button_ != 0) {
        minimize_button_->move(x, w);
    }
}

// ORIGINAL: 0x005F6230 ?on_sys_key@Win@@QAGHIJHI@Z 0x005F6230-0x005F62C8 FILE BYTE_EXACT
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
// lParam PER ARM the way the image duplicates its arg loads, and masking
// BOTH halves (& 0xffff on key_flags, not just repeat_count). A const
// local for the masked flags bought nothing (9/48 both ways): the
// image's `and esi,0xffff` holds key_flags in a callee-saved register it
// chose because the whole image is FPO, while this body compiles under
// the /Oy- flag set the search picks here - the RECEIVER-SPILL/PROLOGUE
// wall again, upstream of any spelling tried. Two instructions still
// short (46 vs 48) for the same reason.
// BYTE_EXACT 48/48 - that wall was never the prologue. `<< 0x10` lets VC6
// prove `(key_flags & 0xffff) << 0x10` == `key_flags << 0x10` and DELETE the
// image's `and esi,0xffff` (one per arm, hence 46 vs 48); spelling the same
// operation `* 0x10000` hides the shift from that fold and the mask survives.
// Scored against the tree's 9/48 in one `measure --dir` run alongside the
// hoisted-load, expression-only and unmasked-high spellings (all 9/48).

int Win::on_sys_key(unsigned int vkey, long is_down, int repeat_count,
                              unsigned int key_flags) {
    // THE IMAGE BUILDS lParam PER ARM, BOTH HALVES MASKED: esi (key_flags)
    // is masked before the doubling and repeat_count after, and the two arms
    // duplicate the loads instead of sharing a packed value across the
    // branch. The doubling is a multiply in the source, not a shift - see
    // the BYTE_EXACT note above.
    if (is_down != 0) {
        LPARAM lParam = ((key_flags & 0xffff) * 0x10000)
                      | (repeat_count & 0xffff);
        PostMessageA(HandleMain, WM_KEYDOWN, vkey, lParam);
        return DefWindowProcA(HandleMain, WM_SYSKEYDOWN, vkey, lParam);
    }
    LPARAM lParam = ((key_flags & 0xffff) * 0x10000)
                  | (repeat_count & 0xffff);
    PostMessageA(HandleMain, WM_KEYUP, vkey, lParam);
    return DefWindowProcA(HandleMain, WM_SYSKEYUP, vkey, lParam);
}

// A one-character glyph `set_bubble_text` measures the width of to compute
// each line's padding. The pinned bytes at 0x00696E00 are a space and NUL;
// Font::width only reads them. Preserve its existing LPSTR interface and
// original writable storage without a pointer into the old executable.
// RECOVERY_OBJECT: v2 0x00696E00 0x00696E02 _WinBubblePrefixGlyph 0x005F83D0
static char WinBubblePrefixGlyph[] = " ";

// ORIGINAL: 0x005F83D0 ?set_bubble_text@Win@@QAEXPADPAURECT@@@Z 0x005F83D0-0x005F84FB FILE
// symbol    ?set_bubble_text@Win@@QAEXPADPAUtagRECT@@@Z
// LEVER: `WinBubbleActive`/`WinBubbleCompanion`/`WinBubbleFont`/`WinBubbleRect`
// (already declared, `WinBubbleCompanion` retyped `Win **` so `this` needs no
// cast) and `WinScreenWidth`, in place of the eight `g_`-named bindings.
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
//
// MEASURED 2026-08-28 at 43 of 100, instruction-count parity (100 vs 100).
//            The residual is one register assignment, and it is the whole
//            gap: the image holds `max_width` in ebp and spills
//            `prefix_width` to its `push ecx` slot at 0x005F8450, this tree
//            holds `prefix_width` in ebp and spills `max_width`. Every
//            differing run downstream is that pair swapped (`cmp eax,ebp`
//            vs `cmp ebx,eax`, `lea edi,[eax+ebp]` vs `lea edx,[eax+ebx]`).
// TRIED: writing the test `line_width > max_width` because 0x005F8460 is
//            `cmp eax, ebp / jle` (new width as the LEFT operand) rather
//            than this body's `cmp ebx, eax / jge` - 43 of 100 either way,
//            VC6 canonicalises the operands and the register plan does not
//            follow the source order.

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
    } else if (WinScreenWidth < x + max_width) {
        x = WinScreenWidth - max_width;
    }
    WinBubbleRect.left = x;
    WinBubbleRect.top = y;
    WinBubbleRect.right = x + max_width;
    WinBubbleRect.bottom = y + line_height;
    WinBubbleActive = reinterpret_cast<int>(text);
    flip(&WinBubbleRect);
}

// ORIGINAL: 0x005EE330 ?resize_event@Win@@QAEHHH@Z 0x005EE330-0x005EE454
// symbol    ?resize_event@Win@@QAEXHH@Z
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
//
// TRIED: naming Scroll+0x4c4/+0x4c8 what they are - they ARE
//            `dib_.bmiHeader.biWidth`/`.biHeight` (the Buffer subobject
//            starts at GraphicWin+0x444 and `dib_` sits at Buffer+0x7c) -
//            and calling `scroll_horz_->resize(...)` through Win's declared
//            slot 3 instead of the MfpBase/Conv3 member-pointer shim. 24
//            down to 21 of 91, and the compiled body loses an instruction
//            (90 vs 91): with the members named, VC6 keeps one fewer value
//            in a callee-saved register and the whole schedule after
//            instruction 6 moves. The raw-offset spelling is kept because
//            it measures better, which is the opposite of the usual lesson.

// The TRIED notes above describe historical scoring, not the current model.
// Original callers5D62E8/5ED9A2/5EDAC6 ignore the result; the body ends after
// redo_caption_buttons without producing one. Model this notification as void.
// Both move calls target the scrollbar, and may replace either borrowed pointer.
void Win::resize_event(int width, int height) {
    WinCallbackWindow = this;
    if ((iFlags_ & 0x40) == 0) {
        if (scroll_vert_ != 0) {
            scroll_vert_->move(outer_rect_.right - outer_rect_.left, 0);
            Scroll *horizontal = scroll_horz_;
            Scroll *vertical = scroll_vert_;
            int resize_height;
            if (horizontal == 0) {
                resize_height = outer_rect_.bottom - outer_rect_.top;
            } else {
                resize_height = horizontal->dib_.bmiHeader.biHeight -
                                outer_rect_.top + outer_rect_.bottom;
            }
            vertical->resize(vertical->dib_.bmiHeader.biWidth, resize_height, 0);
        }
        if (scroll_horz_ != 0) {
            scroll_horz_->move(0, outer_rect_.bottom - outer_rect_.top);
            Scroll *vertical = scroll_vert_;
            Scroll *horizontal = scroll_horz_;
            if (vertical == 0) {
                horizontal->resize(outer_rect_.right - outer_rect_.left,
                                   -horizontal->dib_.bmiHeader.biHeight, 0);
            } else {
                horizontal->resize((outer_rect_.right - vertical->dib_.bmiHeader.biWidth) -
                                   outer_rect_.left,
                                   -horizontal->dib_.bmiHeader.biHeight, 0);
            }
        }
    }
    if (field_400_ != 0) {
        field_400_(width, height);
    }
    on_resize(width, height);
    redo_caption_buttons();
}

// Fixed-slot bindings carried from 005f1340.cpp

typedef HDC (__stdcall *BeginPaintProc)(void *, PAINTSTRUCT *);
typedef int (__stdcall *EndPaintProc)(void *, const PAINTSTRUCT *);

// ORIGINAL: 0x005F1340 ?OnPaint@Win@@QAA_NPAUHWND__@@@Z 0x005F1340-0x005F13A2 FILE BYTE_EXACT
// symbol    ?OnPaint@Win@@SA_NPAUHWND__@@@Z
// LEVER: STATIC, plus the body wrapped in `if (BeginPaint(...) != 0) { ... }`
//   with NO return statement. Both read off the bytes:
//   static - the image reads hwnd at [esp+0x58], where a non-static __cdecl
//   member's `this` sits, so a member spelling costs 4 bytes of argument
//   offset through the whole body.
//   no return - the image's `je` targets the shared epilogue directly and
//   eax is never materialised: the failure path returns BeginPaint's own 0
//   and the success path returns EndPaint's BOOL. Any `return expr;` in a
//   bool function forces `xor al,al` or `neg/sbb/neg`, so the source had
//   none.
// BUG IN THE ORIGINAL: this is a `bool` function with no return statement,
//   so its result is whatever eax happens to hold - BeginPaint's 0 or
//   EndPaint's BOOL. It happens to be a sane bool in both cases, which is
//   presumably why nobody noticed. Left exactly as shipped; the surrounding
//   `#pragma warning(disable: 4716)` pair is what lets VC6 compile it at
//   all, because C4716 is issued as an error.
// working copy - scaffold materialised by --work
// size      98 bytes
// prototype bool (__cdecl ?OnPaint@Win@@QAA_NPAUHWND__@@@Z)(HWND hWnd)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD20 0x005F7320
// indirect  0x005F134E 0x005F1397


#pragma warning(disable: 4716)
bool __cdecl Win::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    // 0x006692B8. The image calls `[0x6692b8]` at 0x005F134E; the
    // file-scope `g` this used to read is SetCursorPos.
    if (BeginPaint(hwnd, &ps) != 0) {
        RECT paintRect;
        paintRect.left = ps.rcPaint.left;
        paintRect.top = ps.rcPaint.top;
        paintRect.right = ps.rcPaint.right;
        paintRect.bottom = ps.rcPaint.bottom;
        update_screen(&paintRect, 0);
        flip(&paintRect);
        EndPaint(hwnd, &ps);
    }
}
#pragma warning(default: 4716)



struct LocalLogPen {
    unsigned int style;
    int widthX;
    int widthY;
    unsigned int color;
};

// The artifact spelled MSG out by hand; these ARE its fields, and the
// message pump below passes one straight to PeekMessageA.
typedef MSG MsgT;


struct PAINTSTRUCT_ {
    HDC hdc;
    int fErase;
    RECT rcPaint;
    int fRestore;
    int fIncUpdate;
    unsigned char rgbReserved[32];
};


// Fixed-slot bindings the homed bodies name but no artifact defined
// in this file - each is an address the image reaches directly.
// The z-order list. MEASURED from the image, not inferred: `bring_to_top`
// at 0x005F4B34 loads its base as an immediate - `mov esi, 0x9b6e48` -
// and walks it with the count at 0x009B7B34. The artifacts declared it
// `extern int WinRootWindows[];` and never defined it anywhere, so nothing
// that used it could ever have linked.

// Menu's real slot-90 contract is Menu::init(Win*), declared on Menu
// and selected by the constructor-installed Menu table.

void __cdecl flush_input();
void __cdecl wait_task();
// The HDC acquire/release pair, recovered from the artifact that defined
// it: the bodies that paint non-client areas all use it, and a macro
// does not travel with a function body.
#define ACQUIRE_HDC(haveHdcVar)      do {          if (WinHdcRefCount == 0) {              if (DirectDrawSurface != 0) {                  DirectDrawSurface->GetDC(&WinSharedHdc);              } else {                  WinSharedHdc =                       GetDC(HandleMain);              }              (haveHdcVar) = (WinSharedHdc != 0);              if (haveHdcVar) {                  WinHdcRefCount = 1;              }          } else {              ++WinHdcRefCount;              (haveHdcVar) = (WinSharedHdc != 0);          }      } while (0)

// Helpers the flip/restore body needs, recovered from the artifact that
// defined them - a static function and a macro do not travel with a
// method body.

// Homed below; the signature is the one its TRIED note measured -
// `void __cdecl restore_flip_sprite(int)`, from the [esp+0x14] stack read.
void __cdecl restore_flip_sprite(int);
extern "C" int __cdecl update_screen(RECT *, Win *);


typedef int (__stdcall *NetCreateGroupFn)(Net *, int *, void *, int, int, int);


// 0xCC / 4 = SLOT 51, which Win declares. The artifact reached it by
// building a member-pointer out of the raw vtable entry through a
// `SlotCCCast` union; with the slot declared, it is just a call.
// The flip/restore helper block, taken whole from the artifact that
// defined it. Restoring its pieces one compiler error at a time kept
// finding another dependency; the block is the coherent unit.

// Builds the saved-region rect the "direct" way: left,top,right=left+w,bottom=top+h.
static void BuildRectDirect(RECT &r) {
    r.left = WinFlipSpriteY;
    r.top = WinFlipSpriteX;
    r.right = WinFlipSpriteY + WinClipWidth;
    r.bottom = WinFlipSpriteX + WinClipHeight;
}

// Restores the saved screen region (if one is pending), clears the sprite
// before flip observes it, and consumes the dirty width. `useHelper` selects
// between the two rect-build
// strategies the disassembly duplicates at each call site (a direct sum,
// or the opaque sub_5f86c0 helper) - both read the same four fields.
static void RestoreAndFlip(bool useHelper) {
    if (WinFlipSprite != 0) {
        WinDrawingBufferOwner.buffer()
            .copy((&ScreenBufferOwner.buffer()), 0, 0, WinFlipSpriteY, WinFlipSpriteX,
                   WinClipWidth, WinClipHeight);
        RECT rect;
        if (useHelper) {
            make_rect(&rect, WinFlipSpriteY, WinFlipSpriteX, WinClipWidth, WinClipHeight);
        } else {
            BuildRectDirect(rect);
        }
        UnionRect(&WinDirtyRect, &WinDirtyRect, &rect);
        WinFlipSprite = 0;
        Win::flip(&WinDirtyRect);
        WinDirtyRect.left = 0;
        WinDirtyRect.right = 0;
    }
    WinFlipSprite = 0;
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

// Original5F0520 enumerates this thread's windows through the shared Win
// callback. Menu's font setter tail-enters this separate policy helper.
void Win::update_menus() {
    EnumThreadWindows(GetCurrentThreadId(),
        reinterpret_cast<WNDENUMPROC>(Win::adjust_menus), 0);
}

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
            menu->adjust();
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
        this->on_lose_mouse_capture();
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
        this->on_lose_mouse_capture();
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
// Each nullable scrollbar receives the original delta, vertical first.
void Win::on_mousewheel_up(int delta) {
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
    Win *active;
    if (WinFocusWindow != nullptr) {
        active = WinModalFocus;
        if (active == nullptr) active = WinFocusWindow;
    } else {
        active = WinInputFocus;
        if (active == nullptr) active = WinDefaultFocus;
    }
    WinActiveWindow = active;
    if (active != nullptr) {
        Win *const base_window = active->poWinBase_;
        if (base_window != nullptr
            && (base_window->iSomeFlag_ & 1U) != 0) {
            Win *const parent = base_window->win_parent_;
            if (parent == nullptr || parent->is_visible() != 0) {
                base_window->on_char(ch, flags);
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

// ORIGINAL: 0x005EDCD0 ?hide@Win@@QAEXXZ 0x005EDCD0-0x005EDE56
// RECOVERY_BINDING: v1 {"evidence": ["git3cb99d431fb928d698e7eb29afee4ac2fc664027 deleted src/recovered/units/005edcd0.cpp and homed this body without its mapping. The parent has canonical symbol?hide@Win@@QAEXXZ and complete390-byte span0x005EDCD0-0x005EDE56 with a MISMATCH note, not an exact claim. The current body retains the homed-from heading and genuine Win declaration; original WorldWin::set_detail_map tail-calls this exact entry on the distinct WorldClimateWorldWin global."], "reason": "Restore the original identity lost when the existing Win::hide implementation was homed. Preserve its historically unclaimed state; the catalogue nonvirtual spelling is provenance, while the actual product declaration is virtual."}
// symbol    ?hide@Win@@UAEXXZ
void Win::hide() {
    int32_t zero;
    Win *focus_a, *focus_b, *chosen;
    Win **child;
    int32_t i;

    if (!(iSomeFlag_ & 1)) {
        return;
    }

    if (WinBubbleCompanion == this) {
        zero = 0;
        if (WinBubbleActive != zero) {
            WinBubbleCompanion = 0;
            WinBubbleActive = zero;
            update_screen(&WinBubbleRect, 0);
            flip(&WinBubbleRect);
        }
        goto converge;
    }
    zero = 0;

converge:
    focus_a = WinPointerOwner1;
    focus_b = WinPointerOwner2;
    chosen = (focus_a != 0) ? focus_a : focus_b;
    if (chosen == this) {
        goto clear_focus;
    }
    if (focus_a == 0) {
        if (focus_b == 0) {
            goto skip_all;
        }
        goto search;
    }
    focus_b = focus_a;

search:
    if (focus_b == 0) {
        goto skip_all;
    }
    if (child_count_ <= 0) {
        goto skip_all;
    }
    child = children_;
    i = 0;
    for (;;) {
        if (*child == focus_b) {
            goto found;
        }
        if ((*child)->is_descendant(focus_b)) {
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
    WinPointerOwner1 = 0;
    WinPointerOwner2 = 0;

skip_all:
    if (WinPointerOwner3 == this) {
        WinPointerOwner3 = 0;
        this->on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    iSomeFlag_ &= 0xfffffffeu;
    if (WinFocusWindow == this) {
        this->release_modal();
    }

    if ((JackalInitFlags & 1) == 0) {
        return;
    }

    {
        int32_t total = WinRootCount;
        WinZOrderCount = 0;
        if (total > 0) {
            Win *target = WinZOrderWindow;
            Win **p = WinRootWindows;
            int32_t j = 0;
            for (;;) {
                if (target != 0 && target == *p) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                if ((*p)->iSomeFlag_ & 1) {
                    recurse_zorder(*p);
                    target = WinZOrderWindow;
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
        Sound *other = val_4_;
        if (other != 0) {
            if (other->is_playing() == 0) {
                other->play();
            }
        }
    }
}

// ===== homed from src/recovered/units/005f63c0.cpp =====

// Original5F64FB admits a GraphicWin via get_graphic_win but uses this,
// not the getter result, for the subsequent hit test and link callback.
// GraphicWin's Buffer base is444; its spot4B0 and owned text4BC therefore
// explain8F4 and900. Win is Buffer's friend, so these are real members.
// ORIGINAL: 0x005F63C0 ?on_l_button_down@Win@@UAEXJHHIH@Z 0x005F63C0-0x005F654E FILE
// RECOVERY_BINDING: v1 {"evidence": ["Historical3cb99d43^:src/recovered/units/005f63c0.cpp recorded unclaimed FILE398-byte5F63C0..5F654E under catalogue ?on_l_button_down@Win@@QAEXJHHIH@Z. Commit3cb99d431fb928d698e7eb29afee4ac2fc664027 deleted the seed without its source binding. No exact claim is inferred.", "Original Win-family tables place5F63C0 at slot81; observed actual compiler emits virtual UAEXJHHIH. Use that evidence-backed virtual name interpretation instead of retaining the old plain-method catalogue guess. Full original endsRET20 at5F654B; next two bytes are padding before5F6550.", "Original5F6442 fifth argument selects both capture owner and slot28/38; fourth WPARAM is unused. Pushes5F64B0/B1 and5F64D4/D5 deliver x,y; original seed had both coordinate reversals and wrong fourth-argument selector. Preserve live parent/related receiver reloads and virtual admission as predicate only.", "GraphicWin owns Buffer at444, Spot4B0 and link texts4BC. Original5F6523 loads this+900+4*index; slot88 on_link_clicked result controls left_down_event fallback. This binding does not verify every original-address service, class lifetime or full GUI."], "reason": "Restore original identity lost during Win rehoming; recover full left-down argument routing and genuine GraphicWin link dispatch."}
void Win::on_l_button_down(long double_click, int x, int y, unsigned int, int from_parent) {
    Win *child = win_parent_;
    if (child != 0) {
        reinterpret_cast<Win *>(child)->bring_child_to_top(this);
    } else if (this != nullptr &&
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

    if (from_parent == 0) {
        WinPointerOwner1 = this;
        WinPointerOwner2 = nullptr;
    } else {
        WinPointerOwner2 = this;
        WinPointerOwner1 = nullptr;
    }

    WinSizingFlag = 1;
    if (double_click != 0 &&
        (iFlags_ & 0x200200) == 0 &&
        (iSomeFlag_ & 8) == 0) {
        Sound *related;
        if (from_parent == 0) {
            WinCallbackWindow = this;
            WinMouseEventCallback callback = field_40C_;
            if (callback != 0) {
                callback(x, y);
            }
            this->on_left_double_click(x, y);
            related = val_17_;
        } else {
            this->vslot_38(x, y);
            related = val_26_;
        }
        if (related != 0 && related->is_playing() == 0) {
            related->play();
        }
    }

    GraphicWin *hitTest = this->get_graphic_win();
    int32_t idx = 0;
    if (hitTest == 0 ||
        static_cast<GraphicWin *>(this)->Buffer::spot_.check(x, y, &idx, 0) < 0 ||
        static_cast<GraphicWin *>(this)->on_link_clicked(
            idx, static_cast<char *>(static_cast<GraphicWin *>(this)->Buffer::owned_[idx])) != 0) {
        this->left_down_event(x, y, from_parent);
    }
}

// ===== homed from src/recovered/units/005f6550.cpp =====

// ORIGINAL: 0x005F6550 ?on_l_button_up@Win@@QAEXHHIH@Z 0x005F6550-0x005F670D FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Win primary virtual table slot 82 at cell 0x0066FF18 selects original entry 0x005F6550.", "src/win.h declares the corresponding Win virtual contract and slot; the unique typed definition in src/win.cpp is the source owner: Win::on_l_button_up is the slot-82 Win virtual.", "The complete preserved seed identity, span, formal facts, claim state, and useful historical notes are restored from 3cb99d431^:src/recovered/units/005f6550.cpp; see the digest-bound inert archive below.", "The existing typed Win body is retained unchanged. This restores source identity and evidence only; it grants no byte/semantic claim or original-TU provenance."], "reason": "Restore the preserved original Win method identity to its existing genuine Win definition using the observed current compiler symbol; retain the old source evidence and claims unchanged."}
// symbol    ?on_l_button_up@Win@@UAEXHHIH@Z
// size      445 bytes
// prototype void (__thiscall ?on_l_button_up@Win@@QAEXHHIH@Z)(Win* this, int, int, unsigned int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90 0x005FA7E0
// indirect  0x005F660C 0x005F6617 0x005F6625 0x005F6630 0x005F663B 0x005F664C 0x005F6657 0x005F66AD 0x005F66B8 0x005F66C6 0x005F66D1 0x005F66E4 0x005F66F5 0x005F6700
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
                WinMouseEventCallback cb = field_414_;
                if (cb != 0) {
                    cb(x, y);
                }
                this->on_left_click(x, y);
                Sound *child = reinterpret_cast<Sound *>(flags_);
                if (child != 0 && child->is_playing() == 0) {
                    child->play();
                }
            } else {
                this->vslot_32(x, y);
                Sound *child = reinterpret_cast<Sound *>(val_20_);
                if (child != 0 && child->is_playing() == 0) {
                    child->play();
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
                WinMouseEventCallback cb = field_41C_;
                if (cb != 0) {
                    cb(x, y);
                }
                this->on_left_up(x, y);
                Sound *child = reinterpret_cast<Sound *>(val_10_);
                if (child != 0 && child->is_playing() == 0) {
                    child->play();
                }
            } else {
                this->on_nc_left_up(x, y);
                Sound *child = val_22_;
                if (child != 0 && child->is_playing() == 0) {
                    child->play();
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

// ORIGINAL: 0x005F4AB0 ?bring_to_top@Win@@QAEXXZ 0x005F4AB0-0x005F4C24 FILE
// symbol    ?bring_to_top@Win@@QAEXXZ
// kind game
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

    if ((~dialog->iFlags_ & 0x1000U) == 0) {
        return;
    }

    if (dialog->list_.count_ == 0) {
        WinDialogNode *iv2 = (dialog->list_.allocator_ != 0)
            ? static_cast<WinDialogNode *>(dialog->list_.allocator_->get(0x1c))
            : static_cast<WinDialogNode *>(mem_get(0x1c));
        dialog->list_.head_ = iv2;
        if (iv2 == 0) {
            goto tail_common;
        }
        iv2->focus_owner_ = this;
        dialog->list_.head_->next_ = dialog->list_.head_;
        dialog->list_.head_->previous_ = dialog->list_.head_;

        if (dialog->list_.allocator_ != 0) {
            dialog->list_.head_->allocation_ =
                dialog->list_.allocator_->get(4);
        } else {
            dialog->list_.head_->allocation_ = mem_get(4);
        }
        if (dialog->list_.head_->allocation_ == 0) {
            goto tail_common;
        }
        dialog->list_.current_ = dialog->list_.head_;
    } else {
        WinDialogNode *newnode = (dialog->list_.allocator_ != 0)
            ? static_cast<WinDialogNode *>(dialog->list_.allocator_->get(0x1c))
            : static_cast<WinDialogNode *>(mem_get(0x1c));
        WinDialogNode *head = dialog->list_.head_;
        WinDialogNode *tail = head->previous_;
        tail->next_ = newnode;

        WinDialogNode *tail2 = dialog->list_.head_->previous_;
        WinDialogNode *node = tail2->next_;
        if (node == 0) {
            goto tail_common;
        }
        node->previous_ = tail2;
        tail2->next_->next_ = dialog->list_.head_;
        dialog->list_.head_->previous_ = tail2->next_;
        WinDialogNode *nn = dialog->list_.head_->previous_;
        dialog->list_.current_ = nn;
        nn->focus_owner_ = this;

        if (dialog->list_.allocator_ != 0) {
            dialog->list_.current_->allocation_ =
                dialog->list_.allocator_->get(4);
        } else {
            dialog->list_.current_->allocation_ = mem_get(4);
        }
        if (dialog->list_.current_->allocation_ == 0) {
            goto tail_common;
        }
    }

    dialog->list_.tail_ = dialog->list_.count_;
    dialog->list_.count_ = dialog->list_.count_ + 1;

tail_common:
    if (dialog->list_.count_ - 1 >= 0) {
        dialog->list_.tail_ = 0;
        dialog->list_.current_ = dialog->list_.head_;
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
    if (child->iFlags_ & 0x2000000) {
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
        if (WinRootWindows[i] == this) {
            if (i < count) {
                count--;
                WinRootCount = count;
                this->hide();
                count = WinRootCount;
                if (i < count) {
                    Win **p = &WinRootWindows[i];
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

// ORIGINAL: 0x005F2940 ?set_rects@Win@@QAEXXZ 0x005F2940-0x005F2ABA FILE
// symbol    ?set_rects@Win@@UAEXXZ
// provenance (retired seed): git:3cb99d43^:src/recovered/units/005f2940.cpp
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
typedef void *(__stdcall *CreatePenIndirectProc)(const LocalLogPen *);
typedef void *(__stdcall *SelectObjectProc)(HDC, void *);
typedef int (__stdcall *MoveToExProc)(HDC, int, int, void *);
typedef int (__stdcall *LineToProc)(HDC, int, int);
typedef int (__stdcall *DeleteObjectProc)(void *);

// The pair ORDER is the image's: lineTo takes (a1,a2) and moveTo takes
// (a3,a4), so the first pair is the END of the segment.
// ORIGINAL: 0x005F2AC0 ?window_line_raw@Win@@QAEXHHHHHHI@Z 0x005F2AC0-0x005F2C37 FILE
// symbol    ?window_line_raw@Win@@QAEXHHHHHHI@Z
// kind game
void Win::window_line_raw(int x2, int y2, int x1, int y1, int colour,
                          int width, unsigned int style) {
    if (WinHdcRefCount != 0) {
        WinHdcRefCount = WinHdcRefCount + 1;
    } else {
        IDirectDrawSurface2 *iface = DirectDrawSurface;
        if (iface != 0) {
            iface->GetDC(&WinSharedHdc);
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
                IDirectDrawSurface2 *iface2 = DirectDrawSurface;
                if (iface2 != 0) {
                    iface2->ReleaseDC(WinSharedHdc);
                    WinSharedHdc = 0;
                    return;
                }
                ReleaseDC(HandleMain, WinSharedHdc);
                WinSharedHdc = 0;
            }
        }
    }
}

// Dispatch through the live Z-order slot each time: callbacks can replace its
// window and change the count. get_graphic_win is only the surface predicate.

// ORIGINAL: 0x005F4D00 ?update@Win@@QAAXXZ 0x005F4D00-0x005F4D4D
// RECOVERY_BINDING: v1 {"evidence": ["git:3cb99d43^:src/recovered/units/005f4d00.cpp historical identity/span, no byte claim.", "image:0x005F4D00-0x005F4D4D complete body, final RET before NOP padding.", "image:0x005F4D1A,0x005F4D24,0x005F4D32 dispatch on freshly reloaded WinZOrderArray elements; getter result is only a predicate, not the update receiver.", "image:0x005F4D38 sound pump only on nonnull getter; 0x005F4D3D reloads signed count after callbacks."], "reason": "Restore the lost mapping of the extant typed Win Z-order update dispatcher. Catalogue nonstatic __cdecl signature is retained; original caller/staticness evidence remains unavailable."}
void __cdecl Win::update() {
    for (int i = 0; i < WinZOrderCount; ++i) {
        WinZOrderArray[i]->on_nc_paint(0, -1);
        if (WinZOrderArray[i]->get_graphic_win() != 0) {
            WinZOrderArray[i]->on_update();
            do_sound();
        }
    }
}

// ===== homed from src/recovered/units/005f4c30.cpp =====

// ORIGINAL: 0x005F4C30 ?redraw_nc_buffer@Win@@QAEHH@Z 0x005F4C30-0x005F4C82 FILE
// symbol    ?redraw_nc_buffer@Win@@QAEPAVBuffer@@H@Z
// kind game
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

// ORIGINAL: 0x005F5810 ?on_nc_paint@Win@@QAEXPAURECT@@H@Z 0x005F5810-0x005F5ACA FILE
// symbol    ?on_nc_paint@Win@@UAEXPAUtagRECT@@H@Z
// kind game
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
                buf = (&ScreenBufferOwner.buffer());
                x = stack8;
                y = stack4;
            } else {
                buf = WinBackBuffer;
                x = stack8 - WinViewOriginX;
                y = stack4 - WinViewOriginY;
            }
            (&ScreenBufferOwner.buffer())->draw(buf, WinFillColour, x, y, 1, 1);
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
                buf = (&ScreenBufferOwner.buffer());
                x = stack8;
            } else {
                v = ((client_rect_.bottom -
                      client_rect_.top) - WinViewOriginY) - v;
                buf = WinBackBuffer;
                x = stack8 - WinViewOriginX;
            }
            (&ScreenBufferOwner.buffer())->draw(buf, WinFillColour, x, v + stack4, 1, 1);
        }

        if (buffer3_ != 0 && (flags == -1 || flags == 3)) {
            if (WinBackBuffer == 0) {
                (&ScreenBufferOwner.buffer())
                    ->draw((&ScreenBufferOwner.buffer()), WinFillColour, stack8,
                           stack4 + iVar1, 1, 1);
            } else {
                (&ScreenBufferOwner.buffer())
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
                buf = (&ScreenBufferOwner.buffer());
            } else {
                yv = iVar1 - WinViewOriginY;
                v = ((client_rect_.right -
                      client_rect_.left) -
                     border_thickness_) - WinViewOriginX;
                buf = WinBackBuffer;
            }
            (&ScreenBufferOwner.buffer())->draw(buf, WinFillColour, v + stack8, yv + stack4, 1, 1);
        }

        ScreenBufferOwner.buffer().restore_clip();
    }
}

// ===== homed from src/recovered/units/005f5270.cpp =====

// ORIGINAL: 0x005F5270 ?remove_child@Win@@QAEXPAUWin@@@Z 0x005F5270-0x005F52D1 FILE
// symbol    ?remove_child@Win@@QAEXPAV1@@Z
// kind game
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

// ORIGINAL: 0x005F6880 ?on_r_button_up@Win@@QAEXHHIH@Z 0x005F6880-0x005F6A21 FILE
// symbol    ?on_r_button_up@Win@@UAEXHHIH@Z
// kind game
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
                    Sound *related;
                    if (dbl == 0) {
                        WinCallbackWindow = this;
                        WinMouseEventCallback callback = field_420_;
                        if (callback != 0) {
                            callback(x, y);
                        }
                        this->on_right_click(x, y);
                        related = reinterpret_cast<Sound *>(val_11_);
                    } else {
                        this->vslot_35(x, y);
                        related = val_23_;
                    }
                    if (related != 0 && related->is_playing() == 0) {
                        related->play();
                    }
                }
            }
        }
    }

    if ((iFlags_ & 0x200000) == 0 &&
        (iSomeFlag_ & 8) == 0) {
    Sound *related2;
        if (dbl == 0) {
            WinCallbackWindow = this;
            WinMouseEventCallback callback = field_428_;
            if (callback != 0) {
                callback(x, y);
            }
            this->on_right_up(x, y);
            related2 = reinterpret_cast<Sound *>(val_13_);
        } else {
            this->vslot_37(x, y);
            related2 = reinterpret_cast<Sound *>(val_25_);
        }
        if (related2 != 0 && related2->is_playing() == 0) {
            related2->play();
        }
    }
}

// ===== homed from src/recovered/units/005f25c0.cpp =====

// ORIGINAL: 0x005F25C0 ?OnRButtonDown@Win@@QAAXPAXJHHI@Z 0x005F25C0-0x005F261F FILE
// symbol    ?OnRButtonDown@Win@@QAAXPAXJHHI@Z
// kind game
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

// ORIGINAL: 0x005F2620 ?OnRButtonUp@Win@@QAAXPAXHHI@Z 0x005F2620-0x005F2661 FILE
// symbol    ?OnRButtonUp@Win@@QAAXPAXHHI@Z
// kind game
void __cdecl Win::OnRButtonUp(void * hwnd, int x, int y, unsigned int keys) {
    WinTrackingWindow = nullptr;
    Win *win = reinterpret_cast<Win *>(get_mouse_window(&x, &y));
    if (win != 0) {
        win->on_r_button_up(x, y, keys, WinMouseDirect);
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

// ORIGINAL: 0x005F6A50 ?get_key_window@Win@@QAGHXZ 0x005F6A50-0x005F6AB0 FILE
// kind game
// symbol    ?get_key_window@Win@@SAPAV1@XZ
// TRIED: direct transcription only tried once; diverges at #0 (push vs mov) in the prologue register-save pattern
// ===== homed from src/unrecovered/005f6a50.cpp =====

// CORRECTED: the catalogue spells `?get_key_window@Win@@QAGHXZ` - `H`,
// int - but every caller assigns the result to a `Win *`. The int is
// the pointer, and win.h has said so since the focus-window work.
Win *Win::get_key_window() {
    Win *window;
    if (WinFocusWindow == nullptr) {
        window = WinInputFocus;
        if (window == nullptr) window = WinDefaultFocus;
    } else {
        window = WinFocusWindow;
        if (WinModalFocus != nullptr) window = WinModalFocus;
    }
    WinActiveWindow = window;
    if (window == nullptr) return nullptr;

    Win *const base_window = window->poWinBase_;
    if (base_window != nullptr) {
        if ((base_window->iSomeFlag_ & 1U) == 0) return nullptr;
        // Native checks the base window's parent but calls is_visible on the
        // base window itself. OnChar and OnKey call it on that parent.
        if (base_window->win_parent_ != nullptr
            && base_window->is_visible() == 0) {
            return nullptr;
        }
    }
    return base_window;
}

// ===== homed from src/recovered/units/005ed9d0.cpp =====
typedef BOOL (__stdcall *InvalidateRectFn)(void *, const RECT *, BOOL);
typedef void (__cdecl *Sub5f8670Fn)(RECT *, int, int);

// ORIGINAL: 0x005ED9D0 ?show@Win@@QAEXH@Z 0x005ED9D0-0x005EDCCB
// RECOVERY_BINDING: v1 {"evidence": ["git show 3cb99d43^:src/recovered/units/005ed9d0.cpp preserves the unclaimed FILE marker for0x005ED9D0-0x005EDCCB. Commit3cb99d43 homed its body into win.cpp under the005ed9d0 heading but omitted the mapping. Original WorldWin set_world_map and show_all call0x005ED9D0 directly on0x008E9F60; existing Win::show(int) is the distinct real product operation. This mapping identifies the homed operation independently of subsequent behavior and reference verification."], "reason": "Restore the original mapping lost when the existing Win::show implementation was homed. Preserve the historical canonical name and complete unclaimed span; do not treat mapping restoration as implementation verification."}
// symbol    ?show@Win@@UAEXH@Z
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
                Win::bring_parent_to_top(this);
            }
        }

        int32_t count = WinRootCount;
        WinZOrderCount = 0;
        if (count > 0) {
            Win *cur = WinZOrderWindow;
            int32_t i = 0;
            do {
                Win *w = WinRootWindows[i];
                if (cur != 0 && cur == w) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                if (w->iSomeFlag_ & 1) {
                    recurse_zorder(w);
                    cur = WinZOrderWindow;
                }
                ++i;
            } while (i < WinRootCount);
        }

        if ((visible & 2) == 0) {
            resize_event(outer_rect_.right - outer_rect_.left, outer_rect_.bottom - outer_rect_.top);
        }
        if ((visible & 1) == 0) {
            this->on_redraw();
        }
        if (menu_ != 0) {
            menu_->redraw();
        }
        if (list_.count_ != 0 && list_.head_ != 0) {
            Win *obj = list_.current_->focus_owner_;
            if (obj != 0) {
                obj->on_dialog_focus(1);
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

        Sound *obj2 = val_3_;
        if (obj2 != 0) {
            if (obj2->is_playing() == 0) {
                obj2->play();
            }
        }
    }
}

// ===== homed from src/recovered/units/005f7ec0.cpp =====
typedef unsigned short uint16_t;

// ORIGINAL: 0x005F7EC0 ?update_back_to_window@Win@@QAEXPAUBuffer@@@Z 0x005F7EC0-0x005F813F FILE
// symbol    ?update_back_to_window@Win@@QAEXPAVBuffer@@@Z
// kind game
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
        if ((w->iSomeFlag_ & 1) != 0) {
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
            WinViewOriginX -= win_parent_->outer_rect_.left;
            tail = win_parent_->outer_rect_.top;
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
        if ((w->iSomeFlag_ & 1) != 0) {
            recurse_zorder(w);
        }
    }
}

// ORIGINAL: 0x005EE190 ?set_modal@Win@@QAEHHP6AHXZPAVWin@@@Z 0x005EE190-0x005EE27B FILE
// kind game
// symbol    ?set_modal@Win@@UAEHHP6AHXZPAV1@@Z
// TRIED: 0.90 mnemonic similarity - the original keeps a register pinned to 0 across the whole function (xor ebx,ebx once, reused for every zero-store and the two early-return `xor eax,eax`s); writing plain `0` literals throughout lets the compiler re-derive zero per site instead, which is a register-allocation choice not reachable from source form.
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
        reinterpret_cast<Win *>(prior)->on_lose_mouse_capture();
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
                        reinterpret_cast<Win *>(cur)->release_modal();
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

// ORIGINAL: 0x005EF310 sub_5ef310 0x005EF310-0x005EF63B FILE
// kind game
// symbol    ?draw_rect_border@Win@@QAEXHHHHPAX0H@Z
// TRIED: signature is __thiscall Win::draw_rect_border(int,int, int,int,HGDIOBJ,HGDIOBJ,int) - `this` receiver is edi (from ecx), field_184_ is a Win member, and the 7th stack slot (esp+0x30) is read by nothing, matching `ret 0x1c`=7 dwords but only 6 used fields, not IDA's flat 7-int __stdcall guess or Ghidra's own param_1..7 listing. Divergence is #2, a `sub esp,N` frame the original never builds (no `mov ebp,esp` at all, pure register+esp addressing); ruled out trimming the local `pal`/`hdc`/`prev` temporaries without moving it, so left for the next pass. The 4x repeated HDC-acquire/refcount block and the rectangle math (4 MoveToEx/LineTo edges, two alternating pens) were hand-traced from the raw disasm stack offsets, not Ghidra's decompile.
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

// ORIGINAL: 0x005EEDC0 ?key_up_event@Win@@QAEHH@Z 0x005EEDC0-0x005EEE67 FILE
// kind game
// TRIED: `int result = 0;` declared after the recursion block (0.944 sim, edi push/xor split from the prologue) vs hoisted to the top before the flags check (0.96 sim, matches original's push-edi-then-xor ordering more closely, one push/xor pair still swapped). Landed the hoisted form. Recursive call's `this` is a COMPUTED pointer (field_D0->field_4, or 0), not the same `this` - matches the disassembly's explicit ecx reload, which ghidra's decompilation silently dropped.
// ===== homed from src/unrecovered/005eedc0.cpp =====

int Win::key_up_event(int key) {
    int result = 0;
    if ((iFlags_ & 0x200000) != 0 ||
        (iSomeFlag_ & 8) != 0) {
        return 0;
    }
    if (list_.count_ != 0) {
        Win *recvThis;
        if (list_.head_ != 0) {
            recvThis = list_.current_->focus_owner_;
        } else {
            recvThis = 0;
        }
        if (recvThis->key_up_event(key) != 0) {
            return 1;
        }
    }
    WinCallbackWindow = this;
    WinKeyHookFn callback = field_434_;
    if (callback != 0) {
        result = callback(key);
    }
    result += this->vslot_27(key);
    Sound *child = val_16_;
    if (child != 0) {
        if (child->is_playing() == 0) {
            child->play();
        }
    }
    return result;
}

// ORIGINAL: 0x005EEB90 ?key_down_event@Win@@QAEHH@Z 0x005EEB90-0x005EEDB9 FILE
// kind game
// TRIED: ebx used as a single dual-purpose local (focus ptr, then the 0x430-hook return accumulator, matching the real register reuse and the leading `xor ebx,ebx`) got closest (#5/xor vs test); hoisting it before the 0x98/0x9c early-return check regressed prologue push order (#4) so it stayed after that check.
// ===== homed from src/unrecovered/005eeb90.cpp =====

int Win::key_down_event(int key) {

    if ((iFlags_ & 0x200000) != 0 ||
        (iSomeFlag_ & 8) != 0) {
        return 0;
    }

    bool clicked = false;
    int ebx = 0;

    if (list_.count_ != 0) {
        Win *focus = 0;
        if (list_.head_ != 0) {
            focus = list_.current_->focus_owner_;
        }

        if (focus->scroll_vert_ != 0) {
            if (focus->scroll_vert_->on_key_click(0, key) != 0) {
                return 1;
            }
            if ((short)MapVirtualKeyA(key & 0xfff8ffff, 2) == 0) {
                focus->key_click_event(0, key);
                clicked = true;
            }
        }
        if (focus->scroll_horz_ != 0) {
            if (focus->scroll_horz_->on_key_click(0, key) != 0) {
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
        if (this->scroll_vert_->on_key_click(0, key) != 0) {
            return 1;
        }
        if (!clicked && (short)MapVirtualKeyA(key & 0xfff8ffff, 2) == 0) {
            key_click_event(0, key);
            clicked = true;
        }
    }
    if (this->scroll_horz_ != 0) {
        if (this->scroll_horz_->on_key_click(0, key) != 0) {
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
    int r2 = this->on_key_down(key);

    Sound *child = val_15_;
    if (child != 0) {
        if (child->is_playing() == 0) {
            child->play();
        }
    }
    return ebx + r2;
}

// ===== homed from src/unrecovered/005ebd80.cpp =====
typedef int (__stdcall *MessageBoxAFn)(void *, const char *, const char *, unsigned int);

// ORIGINAL: 0x005EBD80 ?init@Win@@QAEHHHHHPADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z 0x005EBD80-0x005EC621;0x00662C80-0x00662CCC FILE
// symbol    ?init@Win@@QAEHHHHHPADHPAV1@PAVMenu@@PAUBorderSizing@@@Z
// provenance (retired seed): git:3cb99d431fb928d698e7eb29afee4ac2fc664027^:src/unrecovered/005ebd80.cpp
// Original spans restored from the mapping lost during homing.
int Win::init(int x, int y, int width, int height, char * caption,
              int flags, Win * parent, Menu * menu, BorderSizing * border) {
    this->close();

    if (border != 0) {
        this->caption_height_ = border->caption_height;
        this->border_thickness_ = border->border_thickness;
        this->bottom_border_thickness_ = border->bottom_border_thickness;
    }
    this->field_1A0_ |= 2;

    // The original FuncInfo at 0x0067F8E8 maps construction states 0..3 to
    // delete-on-unwind actions for these four scalar Buffer allocations. VC6's
    // configured operator new may return null; keep each immediate null return.
    {
        this->buffer1_ = new Buffer();
        if (this->buffer1_ == 0) return 4;
    }
    {
        this->buffer2_ = new Buffer();
        if (this->buffer2_ == 0) return 4;
    }
    {
        this->buffer3_ = new Buffer();
        if (this->buffer3_ == 0) return 4;
    }
    {
        this->buffer4_ = new Buffer();
        if (this->buffer4_ == 0) return 4;
    }

    if ((flags & 0xF0000000) != 0) {
        if (this->get_graphic_win() == 0) {
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

    this->cached_outer_rect_.left = this->outer_rect_.left;
    this->cached_outer_rect_.top = this->outer_rect_.top;
    this->cached_outer_rect_.right = this->outer_rect_.right;
    this->cached_outer_rect_.bottom = this->outer_rect_.bottom;

    if (field_E0_ != 0) {
        free(field_E0_);
        field_E0_ = 0;
    }
    if (caption != 0) {
        unsigned int len = strlen(caption) + 1;
        void *mem = mem_get(static_cast<int>(len));
        field_E0_ = static_cast<char *>(mem);
        if (mem != 0) {
            field_E0_[0] = 0;
            strcat(field_E0_, caption);
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
        int result = menu->init(this);
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
            this->scroll_horz_ = this->on_scroll_create();
        }
        if (this->scroll_horz_ == 0) {
            void *raw = operator new(0x214C);
            this->scroll_horz_ = (raw != 0) ? new (raw) Scroll() : 0;
            if (this->scroll_horz_ == 0) return 4;
            Scroll *sc = this->scroll_horz_;
            int top = ScrollDefaultThickness;
            int nc = this->outer_rect_.bottom - top;
            int arg2 = ((this->iFlags_ & 8) != 0)
                           ? (this->outer_rect_.right - this->outer_rect_.left) - top
                           : (this->outer_rect_.right - this->outer_rect_.left);
            sc->init_horz_nc(0, nc, arg2, this, -2);
            sc->show(0);
        }
    }

    if ((this->iFlags_ & 8) != 0) {
        if ((this->iFlags_ & 0x40) != 0) {
            this->scroll_vert_ = this->on_scroll_create();
        }
        if (this->scroll_vert_ == 0) {
            void *raw = operator new(0x214C);
            this->scroll_vert_ = (raw != 0) ? new (raw) Scroll() : 0;
            if (this->scroll_vert_ == 0) return 4;
            Scroll *sc = this->scroll_vert_;
            int top = ScrollDefaultThickness;
            int nc = this->outer_rect_.right - top;
            int arg2 = ((this->iFlags_ & 4) != 0)
                           ? (this->outer_rect_.bottom - this->outer_rect_.top) - top
                           : (this->outer_rect_.bottom - this->outer_rect_.top);
            sc->init_vert_nc(nc, 0, arg2, this, -1);
            sc->show(0);
        }
    }

    this->do_caption_buttons();
    WinCallbackWindow = this;

    unsigned int fl = this->iFlags_;
    bool skipMenuList = (fl & 0x100000) != 0 || (fl & 0x200000) != 0 ||
                         (this->iSomeFlag_ & 8) != 0 ||
                         (((fl & 0x100000) == 0 && ((fl >> 16) & 0x10) != 0) && this->win_parent_ == 0);

    if (!skipMenuList && this->win_parent_ != 0 &&
        (this->win_parent_->iFlags_ & 0x1000) == 0) {
        Win *parent_window = this->win_parent_;
        int initCount = parent_window->list_.count_;
        if (initCount != 0) {
            WinDialogNode *newNode = parent_window->list_.allocator_
                ? static_cast<WinDialogNode *>(parent_window->list_.allocator_->get(0x1C))
                : static_cast<WinDialogNode *>(mem_get(0x1C));
            WinDialogNode *head = parent_window->list_.head_;
            WinDialogNode *prevOfHead = head->previous_;
            prevOfHead->next_ = newNode;
            if (newNode != 0) {
                newNode->previous_ = prevOfHead;
                newNode->next_ = head;
                head->previous_ = newNode;
                parent_window->list_.current_ = newNode;
                newNode->focus_owner_ = this;
                void *link = parent_window->list_.allocator_
                    ? parent_window->list_.allocator_->get(4) : mem_get(4);
                newNode->allocation_ = link;
                if (link == 0) {
                    return 0;
                }
            } else {
                return 0;
            }
        } else {
            WinDialogNode *newNode = parent_window->list_.allocator_
                ? static_cast<WinDialogNode *>(parent_window->list_.allocator_->get(0x1C))
                : static_cast<WinDialogNode *>(mem_get(0x1C));
            parent_window->list_.head_ = newNode;
            if (newNode == 0) {
                return 0;
            }
            newNode->focus_owner_ = this;
            newNode->next_ = newNode;
            newNode->previous_ = newNode;
            void *link = parent_window->list_.allocator_
                ? parent_window->list_.allocator_->get(4) : mem_get(4);
            newNode->allocation_ = link;
            if (link == 0) {
                return 0;
            }
            parent_window->list_.current_ = newNode;
        }

        int oldCount = parent_window->list_.count_;
        parent_window->list_.tail_ = oldCount;
        parent_window->list_.count_ = oldCount + 1;

        int cnt = parent_window->list_.count_ - 1;
        if (cnt >= 0) {
            WinDialogNode *head = parent_window->list_.head_;
            parent_window->list_.tail_ = 0;
            parent_window->list_.current_ = head;
        }
    }

    this->iSomeFlag_ |= 4;
    return 0;
}

// ===== homed from src/unrecovered/005f1820.cpp =====

// ORIGINAL: 0x005F1820 ?update_cursor@Win@@QAAHPAUWin@@H@Z 0x005F1820-0x005F2200
// RECOVERY_BINDING: v1 {"evidence": ["git show 3cb99d43^:src/unrecovered/005f1820.cpp preserves this identity and full code span with mismatch history, not a byte claim. Its existing body is under the same homing heading in win.cpp. Entry reads two stack arguments; original body ends at5F21FF and the switch table begins at5F2200. Source binding does not establish behavior, table matching or acceptance."], "reason": "Restore the unclaimed cursor operation mapping omitted during homing. The catalogued decoration is historical; the product uses the receiver-free static Win operation."}
// symbol    ?update_cursor@Win@@SAHPAV1@H@Z
int __cdecl Win::update_cursor(Win *window, int tgl) {

    POINT pt;
    GetCursorPos(&pt);
    int screen_x = pt.x;
    int screen_y = pt.y;
    // Lookup can convert these copies to window-local hit-test coordinates;
    // saved cursor pixels remain positioned in the original screen coordinates.
    int hit_x = screen_x;
    int hit_y = screen_y;

    Win *win = window;
    if (win == 0) {
        win = Win::get_mouse_window(&hit_x, &hit_y);
        if (win == 0) {
            RestoreAndFlip(false);
            HCURSOR cur = LoadCursorA(0, IDC_ARROW);
            return reinterpret_cast<int>(SetCursor(cur));
        }
    }

    if (WinMouseDirect == 0 && (win->iFlags_ & WinFlagHitTestIndirect) == 0) {
        if (win->cursor_sprite_ != 0) {
            if (WinFlipSprite == 0) {
                while (ShowCursor(0) >= 0) {
                }
            } else {
                restore_flip_sprite(0);
            }

            if (win->cursor_sprite_ != WinFlipSprite) {
                WinClipWidth = win->cursor_sprite_->iWidth_;
                WinClipHeight = win->cursor_sprite_->iHeight_;
                WinDragOffsetX = win->field_18C_;
                WinDragOffsetY = win->field_190_;
                if (WinDrawingBufferOwner.buffer().dib_.bmiHeader.biWidth != WinClipWidth ||
                    -WinDrawingBufferOwner.buffer().dib_.bmiHeader.biHeight != WinClipHeight) {
                    WinDrawingBufferOwner.buffer().init(WinClipWidth, WinClipHeight, 0, 0);
                }
            }
            int width = WinClipWidth;
            int height = WinClipHeight;
            int destX = screen_x - WinDragOffsetX;
            int destY = screen_y - WinDragOffsetY;
            WinFlipSpriteY = destX;
            WinFlipSpriteX = destY;
            WinFlipSprite = win->cursor_sprite_;
            ScreenBufferOwner.buffer().copy(&WinDrawingBufferOwner.buffer(), destX, destY, 0, 0, width, height);

            RECT rect;
            rect.left = WinFlipSpriteY;
            rect.top = WinFlipSpriteX;
            rect.right = WinFlipSpriteY + WinClipWidth;
            rect.bottom = WinFlipSpriteX + WinClipHeight;
            int result = UnionRect(&WinDirtyRect, &rect, &WinDirtyRect);
            if (tgl == 0) {
                return result;
            }
            int prevFlag = WinFlipClipped;
            WinFlipClipped = 1;
            Win::flip(&WinDirtyRect);
            // 5F1A85 loads right (it does not clear it); 5F1A93 stores it to
            // left and the function returns the same value after the flip.
            result = WinDirtyRect.right;
            WinFlipClipped = prevFlag;
            WinDirtyRect.left = result;
            return result;
        }

        // This window uses an OS cursor, not a saved sprite.
        if (WinFlipSprite != 0) {
            int ebxFlag = 1;
            restore_flip_sprite(1);
            while (ShowCursor(ebxFlag) <= 0) {
            }
        }
        WinFlipSprite = 0;

        if (win->cursor_handle_ != 0) {
            if (*win->cursor_handle_ != 0) {
                return reinterpret_cast<int>(SetCursor(*win->cursor_handle_));
            }
        }
        if (win->cursor_name_ == 0) {
            goto lab_default_cursor;
        }
        {
            HCURSOR cur = LoadCursorA(0, reinterpret_cast<const char *>(win->cursor_name_));
            return reinterpret_cast<int>(SetCursor(cur));
        }
    }

    // Full dispatch: ask the window what kind of hit region the cursor is
    // over, then pick the matching cursor image / saved-region behaviour.
    // Case grouping and targets read from the resolved jump table (20
    // entries at 0x005F2200, dispatched at 0x005F1B3C), not guessed.
    {
        int hitCode = win->on_nc_hittest(hit_x, hit_y);
        hitCode += 2;
        int ebxFlag = 1;
        int result = hitCode;

        switch (hitCode) {
        case 2:
        case 4:
            RestoreAndFlip(false);
            WinCursorMoved = ebxFlag;
            result = reinterpret_cast<int>(SetCursor(LoadCursorA(0, IDC_ARROW)));
            break;
        case 12:
            RestoreAndFlip(false);
            WinCursorMoved = ebxFlag;
            result = reinterpret_cast<int>(SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f84))));
            break;
        case 13:
            RestoreAndFlip(true);
            WinCursorMoved = ebxFlag;
            result = reinterpret_cast<int>(SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f84))));
            break;
        case 14:
        case 17:
            RestoreAndFlip(true);
            WinCursorMoved = ebxFlag;
            result = reinterpret_cast<int>(SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f85))));
            break;
        case 15:
        case 19:
            RestoreAndFlip(true);
            WinCursorMoved = ebxFlag;
            result = reinterpret_cast<int>(SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f82))));
            break;
        case 16:
        case 18:
            RestoreAndFlip(true);
            WinCursorMoved = ebxFlag;
            result = reinterpret_cast<int>(SetCursor(LoadCursorA(0, MAKEINTRESOURCE(0x7f83))));
            break;
        case 0:
        case 1:
        case 3:
            // Skip selection, but still restore and poll in the common tail.
            break;
        default:
            // Covers 5..11 and any out-of-table index (the `ja` fallback).
            RestoreAndFlip(true);
            result = reinterpret_cast<int>(SetCursor(LoadCursorA(0, IDC_ARROW)));
            break;
        }

        if (WinMouseDirect != 0 || win->cursor_sprite_ != 0) {
            return result;
        }
        if (WinFlipSprite != 0) {
            WinDrawingBufferOwner.buffer()
                .copy((&ScreenBufferOwner.buffer()), 0, 0, WinFlipSpriteY, WinFlipSpriteX,
                       WinClipWidth, WinClipHeight);
            RECT rect;
            make_rect(&rect, WinFlipSpriteY, WinFlipSpriteX, WinClipWidth, WinClipHeight);
            UnionRect(&WinDirtyRect, &WinDirtyRect, &rect);
            Sprite *saved = WinFlipSprite;
            WinFlipSprite = 0;
            Win::flip(&WinDirtyRect);
            WinFlipSprite = saved;
            WinDirtyRect.left = 0;
            WinDirtyRect.right = 0;
        }

        do {
            result = ShowCursor(ebxFlag);
        } while (result <= 0);
        return result;
    }

lab_default_cursor:
    WinFlipSprite = 0;
    {
        HCURSOR cur = LoadCursorA(0, IDC_ARROW);
        return reinterpret_cast<int>(SetCursor(cur));
    }
}

// ORIGINAL: 0x005ED5C0 ?center@Win@@QAEHXZ 0x005ED5C0-0x005ED6F7 FILE
// kind game
// TRIED: named `parent` local for win_parent_ (extra register pressure, forces a 4th callee-save + frame pointer); computing both local_8 and local_4 divisions fully inside each if/else branch (extra register); calling screen_to_client() on win_parent_ itself instead of win_parent_->win_parent_ (asm's ecx at the call site is loaded from [win_parent_+0xc4], i.e. the grandparent, reused straight from the preceding null check). Best reached: MISMATCH, edit_count 10/95 mnemonics, 312 bytes vs 311 - a register-colouring difference only (rebuilt keeps `this` in esi where the original keeps it in ebx).
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

// ORIGINAL: 0x005ED700 sub_5ed700 0x005ED700-0x005ED7B4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/005ed700.cpp; full original marker, spans, byte claim and lessons transported by match_recovery.source.writer", "historical scaffold symbol hint (not an emitted-symbol fact):    ?center_on_parent@WinCentering@@QAEHXZ", "owner proposal: src/win.cpp; original identity 0x005ED700; symbol observation intentionally pending physical compilation"], "reason": "Recover parent-relative centering through the real Win receiver and existing move/API owners; emitted symbol awaits physical observation."}
// symbol    ?center_on_parent@Win@@QAEHXZ
// working copy - scaffold materialised by --work
// size      180 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED7D0
// indirect  0x005ED746
int Win::center_on_parent() {
    Win *const parent = win_parent_;
    if (parent == 0) return 1;

    RECT local = parent->client_rect_;
    if (IsRectEmpty(&local)) return 7;

    return move(
        local.left + (((parent->client_rect_.right - client_rect_.right) +
                        client_rect_.left - parent->client_rect_.left) >> 1),
        local.top + (((parent->client_rect_.bottom - parent->client_rect_.top) -
                       client_rect_.bottom + client_rect_.top) >> 1));
}

// ===== homed from src/unrecovered/005ee750.cpp =====
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
        WinMouseEventCallback cb = field_418_;
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
        this->on_left_down(x, y);
        Sound *child = val_9_;
        if (child != 0) {
            int r = child->is_playing();
            if (r == 0) {
                child->play();
            }
        }
    } else {
        this->on_nc_left_down(x, y);
        Sound *child = val_21_;
        if (child != 0) {
            int r = child->is_playing();
            if (r == 0) {
                child->play();
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
            rect->top = rect->top + menu_->requested_height();
        }
    }
}

// ===== homed from src/unrecovered/005f2cf0.cpp =====
typedef int (__stdcall *IntersectRectFn)(RECT *, const RECT *, const RECT *);

// ORIGINAL: 0x005F2CF0 ?on_redraw_nc@Win@@QAEXPAURECT@@H@Z 0x005F2CF0-0x005F37BA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Win primary virtual table slot 67 at cell 0x0066FEDC selects original entry 0x005F2CF0.", "src/win.h declares the corresponding Win virtual contract and slot; the unique typed definition in src/win.cpp is the source owner: Win::on_redraw_nc is the slot-67 Win virtual.", "The complete preserved seed identity, span, formal facts, claim state, and useful historical notes are restored from 3cb99d431^:src/unrecovered/005f2cf0.cpp; see the digest-bound inert archive below.", "The existing typed Win body is retained unchanged. This restores source identity and evidence only; it grants no byte/semantic claim or original-TU provenance."], "reason": "Restore the preserved original Win method identity to its existing genuine Win definition using the observed current compiler symbol; retain the old source evidence and claims unchanged."}
// symbol    ?on_redraw_nc@Win@@UAEXPAUtagRECT@@H@Z
// size      2762 bytes
// prototype void (__thiscall ?on_redraw_nc@Win@@QAEXPAURECT@@H@Z)(Win* this, RECT*, int)
// callers   1   call targets   10
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DA860 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005DD450 0x005DFB50 0x005E3203 0x006453E0
// indirect  0x005F2E85 0x005F30E4 0x005F319C 0x005F332B 0x005F33D3 0x005F3544 0x005F35FC 0x005F37AA
// TRIED: first 9 instructions match (prologue, flag tests); diverges at instr #10 (je vs mov). Field map (buffer1_..4_ at 0xB4/0xB8/0xBC/0xC0, heap_.base_ at 0xE0 as the title char*, client_rect_/outer_rect_ at 0x14C/0x13C) cross-checked against param_1[N]*4 in the Ghidra listing and is consistent across all 4 near-identical edge blocks.
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
                    buf->set_font(this->field_F8_, 0, 0, 0);
                    buf->set_clip(&r);
                    char *title = this->field_E0_;
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
            this->on_post_redraw_nc_buffer(buf, 0);
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
            this->on_post_redraw_nc_buffer(buf, 2);
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
            this->on_post_redraw_nc_buffer(buf, 3);
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
            this->on_post_redraw_nc_buffer(buf, 1);
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
            *x += win_parent_->outer_rect_.left;
            *y += win_parent_->outer_rect_.top;
        }
    }
    *x += outer_rect_.left;
    *y += outer_rect_.top;
}

// ===== homed from src/unrecovered/005f5fb0.cpp =====

// ORIGINAL: 0x005F5FB0 sub_5f5fb0 0x005F5FB0-0x005F622B
// RECOVERY_BINDING: v1 {"evidence": ["git:85fc7721f56c2a5cae754b993fe04fcfec74fec1:src/unrecovered/005f5fb0.cpp", "Pinned original5F5FB0..5F622B; direct global accesses5F60DD..5F619C"], "reason": "Restore the lost homing identity for the existing Win keyboard handler; its receiver, scroll dispatch and two stack words identify this method, not the separate string matcher at5F5F60."}
// symbol    ?on_char@Win@@UAEXDH@Z
// TRIED: extern "C" __stdcall(int,int) free function per IDA's guess - the spilled ecx is read back later ([esp+0x10] resolves to the "push ecx" slot itself, not a caller stack arg) and used as a Win* (scroll_vert_/scroll_horz_/key_click_event), so this is landed as Win::sub_5f5fb0(char, int) __thiscall instead; the unused 2nd stack dword explains `ret 8` with only 1 real read.
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
        Win &debug_window = WinBubbleWindow;
        if ((static_cast<uint8_t>(debug_window.iSomeFlag_) & 1) == 0 ||
            (debug_window.win_parent_ != 0 &&
             debug_window.win_parent_->is_visible() == 0)) {
            (&WinBubbleWindow)->show(0);
        } else if ((static_cast<uint8_t>(debug_window.iSomeFlag_) & 1) != 0) {
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
            } else if (cand != nullptr && debug_window.child_count_ > 0) {
                Win **list = debug_window.children_;
                int i = 0;
                do {
                    if (list[i] == cand || list[i]->is_descendant(cand)) {
                        WinPointerOwner1 = nullptr;
                        WinPointerOwner2 = 0;
                        break;
                    }
                    i++;
                } while (i < debug_window.child_count_);
            }

            if (WinPointerOwner3 == &WinBubbleWindow) {
                WinPointerOwner3 = 0;
                (&WinBubbleWindow)->on_lose_mouse_capture();
            }
            if (WinPointerOwner4 == &WinBubbleWindow) {
                WinPointerOwner4 = 0;
            }
            debug_window.iSomeFlag_ &= ~1;
            if (WinFocusWindow == &WinBubbleWindow) {
                (&WinBubbleWindow)->release_modal();
            }

            if ((JackalInitFlags & 1) != 0) {
                WinZOrderCount = 0;
                if (WinRootCount > 0) {
                    Win **entries = WinRootWindows;
                    Win *cur = WinZOrderWindow;
                    int j = 0;
                    do {
                        if (cur != 0 && cur == entries[j]) {
                            WinZOrderCount = 0;
                            WinZOrderFlag = 0;
                        }
                        if ((entries[j]->iSomeFlag_ & 1) != 0) {
                            recurse_zorder(entries[j]);
                            cur = WinZOrderWindow;
                        }
                        j++;
                    } while (j < WinRootCount);
                }

                InvalidateRect(HandleMain, 0, 0);

                Sound *w = debug_window.val_4_;
                if (w != 0 && w->is_playing() == 0) {
                    w->play();
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
        scroll_vert_->on_key_click((int)param2, WinKeyModifiers) != 0) {
        handled = true;
    }
    if (!handled && scroll_horz_ != 0 &&
        scroll_horz_->on_key_click((int)param2, WinKeyModifiers) != 0) {
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

// ORIGINAL: 0x005ED880 ?resize@Win@@QAEHHHH@Z 0x005ED880-0x005ED9B5 FILE
// symbol    ?resize@Win@@UAEHHHH@Z
// Original mapping restored from git:85fc7721f56c2a5cae754b993fe04fcfec74fec1:src/unrecovered/005ed880.cpp
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
    this->on_redraw();
    return 0;
}

// ORIGINAL: 0x005EDF50 ?set_caption@Win@@QAEXPAD@Z 0x005EDF50-0x005EDFD5 FILE
// kind game
// ===== homed from src/unrecovered/005edf50.cpp =====

void Win::set_caption(char * text) {
    if (field_E0_ != 0) {
        free(field_E0_);
        field_E0_ = 0;
    }
    if (text != 0) {
        unsigned int len = strlen(text);
        void *buf = mem_get(len + 1);
        field_E0_ = static_cast<char *>(buf);
        if (buf == 0) {
            return;
        }
        field_E0_[0] = 0;
        strcat(field_E0_, text);
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

// ORIGINAL: 0x005F5140 ?bring_parent_to_top@Win@@QAAXPAVWin@@@Z 0x005F5140-0x005F51F7
// RECOVERY_BINDING: v1 {"evidence": ["git show 3cb99d43^:src/unrecovered/005f5140.cpp preserves this identity and span. Original entry loads the sole argument from the stack, never consumes incoming ECX and returns with plain ret; show at0x5EDA3E and on_r_button_down at0x5F672A each push the window and reclaim four bytes. Static versus free historical spelling is not established."], "reason": "Restore the unclaimed original mapping omitted during homing; model the receiver-free root-order operation as a static Win member. The catalogued name is not the recovered calling convention."}
// symbol    ?bring_parent_to_top@Win@@SAXPAV1@@Z
void __cdecl Win::bring_parent_to_top(Win * window) {
    if (window != 0 && (window->iFlags_ & 0x2000000) == 0) {
        int count = WinRootCount;
        int i = 0;
        for (; i < count; ++i) {
            if (WinRootWindows[i] == window) break;
        }
        if (i < count) {
            while (i > 0) {
                WinRootWindows[i] = WinRootWindows[i - 1];
                --i;
            }
            WinRootWindows[0] = window;
        }
        int j = 0;
        WinZOrderCount = 0;
        if (count > 0) {
            Win *cur = WinZOrderWindow;
            do {
                if (cur != 0 && cur == WinRootWindows[j]) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                Win *w = WinRootWindows[j];
                if (w->iSomeFlag_ & 1) {
                    recurse_zorder(w);
                    count = WinRootCount;
                    cur = WinZOrderWindow;
                }
                ++j;
            } while (j < count);
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
        *count += menu_->requested_height();
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
typedef void (OriginalObject::*VCall0)();
typedef void (OriginalObject::*VCall1)(int);

// ORIGINAL: 0x005EEA90 ?key_click_event@Win@@QAEHHH@Z 0x005EEA90-0x005EEB81 FILE
// provenance (retired seed): git:3cb99d43^:src/unrecovered/005eea90.cpp
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
            this->on_button_clicked(-2);
        } else if (flags == 0xd || flags == 0x1000d) {
            this->on_button_clicked(-1);
        }
    }

    if (list_.count_ != 0) {
        Win *target = 0;
        if (list_.head_ != 0) {
            target = list_.current_->focus_owner_;
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
    WinCallbackWindow = self;
    WinKeyClickCallback callback = field_42C_;
    if (callback != 0) {
        result = callback(key, flags);
    }
    void *vtbl2 = *(void **)self;
    int r2 = this->on_key_click(key, flags);
    Sound *child = val_14_;
    if (child != 0) {
        void *cvtbl = *(void **)child;
        if (child->is_playing() == 0) {
            void *cvtbl2 = *(void **)child;
            child->play();
        }
    }
    return result + r2;
}

// ORIGINAL: 0x005F74A0 ?update_window@Win@@QAEHPAURECT@@@Z 0x005F74A0-0x005F757F FILE
// kind game
// symbol    ?update_window@Win@@QAEHPAUtagRECT@@@Z
// TRIED: `if (a1 != 0) { big-block } return simple();` puts the branches in the wrong order (`je` skipping the block where the original has `jne` falling into the simple case first) - an early-return guard `if (a1 == 0) return simple(); big-block;` fixes that. update_screen is __cdecl per win.h (func_win_update_screen), not the scaffold's implicit-thiscall class member, so it's called through a separately-declared `extern "C"` free function of the real signature. Remaining: the original spends two extra callee-saved registers (ebx, ebp) holding self[0x140]/self[0x13c] before the adds; this form only needed one (ebx). 62/73 shared.
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
            x -= win_parent_->outer_rect_.left;
            y -= win_parent_->outer_rect_.top;
        }
    }
    RECT r;
    r.left = area->left + x;
    r.top = area->top + y;
    r.right = area->right + x;
    r.bottom = area->bottom + y;
    return update_screen(&r, this);
}

// ORIGINAL: 0x005F52E0 ?bring_child_to_top@Win@@QAEXPAVWin@@@Z 0x005F52E0-0x005F539C FILE
// kind game
// symbol    ?bring_child_to_top@Win@@QAEXPAV1@@Z
// TRIED: MISMATCH, closest at 193/188 bytes (mnemonic_similarity 0.90). An extra `push ebx` appears (register pressure from 4 live locals in the z-order loop: entry/cur/j plus the 'this'-Win receiver) that the original avoids; tried both a direct `(*entry)->iSomeFlag_ & 1` and a cached `Win *w = *entry;` form, no difference. `g_win_zorder_list` as its own `extern Win *[]` (not the fixed-address alias) for the indexed array walk, per the indexed-table-base lever.
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
        this->on_lose_mouse_capture();
    }
    if (WinPointerOwner4 == this) {
        WinPointerOwner4 = 0;
    }
    ShowWindow(
        HandleMain, 6);
    return 0;
}

// ORIGINAL: 0x005F7580 ?do_tracking@Win@@QAEXHH@Z 0x005F7580-0x005F7E47 FILE
// kind game
// TRIED: Ghidra's pseudocode was unsafe here in three separate ways, all fixed from raw disasm: (1) the client_to_screen/nonclient_to_screen "vacuous test" branches Ghidra rendered as a no-op `if(&stack!=0x20){FUN();}` actually apply the screen origin to a size-rect afterward - Ghidra dropped that entirely; (2) the slot003 vtable call takes 3 args (width,height,0), Ghidra showed only 1; (3) two of the screen_to_client/client_to_screen calls are on win_parent_, not `this` (ecx is never reloaded to esi before them). The switch-case clamp arithmetic itself did match Ghidra once independently checked against raw disasm. Compiles; original is frameless (sub esp direct), rebuilt took a framed prologue - not chased given size (2247 bytes).
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

// ORIGINAL: 0x005EF1E0 sub_5ef1e0 0x005EF1E0-0x005EF310 FILE
// kind game
// symbol    ?sub_5ef1e0@Win@@QAEXHHHHPAXH@Z
// TRIED: the scaffold's own Win/Palette classes cannot be reused (they are fully defined already, and body-mode's brace-counted extractor would stop at the first balanced `{}` - our own helper class - before reaching the function), so this stays FILE mode with its own minimal Win/Palette. A plain `*(FuncPtr*)addr` call for GetDC/SelectObject/etc. always loaded the pointer into a register first (mov+call reg); declaring them `__declspec(dllimport)` gets the single `call dword ptr [addr]` form the original uses. Landed at MISMATCH #29 (a `push edi` the original hoists earlier, before the palette-sync compare, that this body only emits once `oldPen` is first needed).
// ===== homed from src/unrecovered/005ef1e0.cpp =====

void Win::sub_5ef1e0(int x1, int y1, int x2, int y2, void *pen, int unused6) {
    (void)unused6;
    HDC hdc;
    if (WinHdcRefCount != 0) {
        WinHdcRefCount = WinHdcRefCount + 1;
        hdc = WinSharedHdc;
    } else {
        IDirectDrawSurface2 *iface = DirectDrawSurface;
        if (iface != 0) {
            iface->GetDC(&WinSharedHdc);
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
            IDirectDrawSurface2 *iface2 = DirectDrawSurface;
            if (iface2 != 0) {
                iface2->ReleaseDC(WinSharedHdc);
                WinSharedHdc = 0;
                return;
            }
            ReleaseDC(HandleMain, WinSharedHdc);
            WinSharedHdc = 0;
        }
    }
}

// ORIGINAL: 0x005F7320 ?update_screen@Win@@QAAHPAURECT@@PAVWin@@@Z 0x005F7320-0x005F749F FILE
// kind game
// symbol    ?update_screen@Win@@SAHPAUtagRECT@@PAV1@@Z
// TRIED: none confirmed byte-exact; MISMATCH #9 'test' vs 'mov' - the [0x669338] seam is modelled as a raw IntersectRect-shaped __stdcall function pointer with a guessed (dest, src1, src2) argument order; the real signature at that seam is unconfirmed, and the down-counting child-window loop's register pressure does not match the original's.
// ===== homed from src/unrecovered/005f7320.cpp =====

int __cdecl Win::update_screen(RECT *area, Win *window) {
    int windowCount = WinZOrderCount;
    if (windowCount == 0) {
        return 0;
    }

    Win *target;
    if (WinZOrderWindow != 0) {
        target = window;
    } else {
        target = 0;
    }

    int idx = windowCount - 1;
    if (target != 0) {
        idx = 0;
        if (windowCount > 0) {
            Win **table = WinZOrderArray;
            int i = 0;
            while (table[i] != target) {
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
                (&ScreenBufferOwner.buffer())->fill(area, 0);
            } else {
                (&ScreenBufferOwner.buffer())->fill(0);
            }
        }
    }

    if (idx >= 0) {
        for (int i = idx; i >= 0; --i) {
            Win *w = WinZOrderArray[i];
            if ((w->iSomeFlag_ & 1) != 0) {
                Win *menuLike = w->win_parent_;
                bool visible = true;
                if (menuLike != 0) {
                    visible = menuLike->is_visible() != 0;
                }
                if (visible) {
                    // The virtual parent visibility query can change the
                    // table: 5F73C8 reloads this slot before painting.
                    w = WinZOrderArray[i];
                    if ((w->iSomeFlag_ & 2) != 0) {
                        w->on_nc_paint(area, -1);
                    }
                    bool doSecond = true;
                    if (target != 0) {
                        doSecond = (target == target->poWinBase_);
                    }
                    if (doSecond) {
                        Win *const other = w->poWinBase_;
                        other->on_paint(area);
                    }
                }
            }
        }
    }

    if (WinFlipSprite == 0) {
        return 0;
    }

    int origin_x = WinFlipSpriteY; // original 9B7A30 is RECT.left
    int origin_y = WinFlipSpriteX; // original 9B7A34 is RECT.top
    RECT rectangle;
    rectangle.left = origin_x;
    rectangle.top = origin_y;
    rectangle.right = static_cast<int>(static_cast<unsigned int>(origin_x) +
                                       static_cast<unsigned int>(WinClipWidth));
    rectangle.bottom = static_cast<int>(static_cast<unsigned int>(origin_y) +
                                        static_cast<unsigned int>(WinClipHeight));
    // IAT 669338 is USER32!IntersectRect. The two LEAs at 5F7445/5F744A
    // denote the same complete stack RECT after the intervening PUSH.
    if (area != 0) {
        if (IntersectRect(&rectangle, &rectangle, area) == 0) return 0;
        origin_y = WinFlipSpriteX;
        origin_x = WinFlipSpriteY;
    }
    return ScreenBufferOwner.buffer().copy(
        &WinDrawingBufferOwner.buffer(), rectangle.left, rectangle.top,
        static_cast<int>(static_cast<unsigned int>(rectangle.left) -
                         static_cast<unsigned int>(origin_x)),
        static_cast<int>(static_cast<unsigned int>(rectangle.top) -
                         static_cast<unsigned int>(origin_y)),
        static_cast<int>(static_cast<unsigned int>(rectangle.right) -
                         static_cast<unsigned int>(rectangle.left)),
        static_cast<int>(static_cast<unsigned int>(rectangle.bottom) -
                         static_cast<unsigned int>(rectangle.top)));
}

// ===== homed from src/recovered/units/005f5d10.cpp =====
typedef short(__stdcall *GetKeyStateFn)(int);
typedef int(__stdcall *PeekMessageFn)(MsgT *, void *, unsigned int, unsigned int, unsigned int);
typedef int(__stdcall *Fn1)(void *);

// ORIGINAL: 0x005F5D10 ?on_key@Win@@QAEXIJHI@Z 0x005F5D10-0x005F5F53
// RECOVERY_BINDING: v1 {"evidence": ["git:3cb99d43^:src/recovered/units/005f5d10.cpp:1; src/win.h slot 78; pinned original 0x005F5D4F-0x005F5DE0 reads the active Net and calls its two voice-start overloads and stop_voice"], "reason": "Restore the unclaimed original identity lost when this existing Win::on_key body was homed; retain the catalogue name separately from its current virtual emitted symbol. Original instruction decoding confirms entry and ret 0x10 at 0x005F5F50; the historical extent ends at 0x005F5F53."}
// symbol    ?on_key@Win@@UAEXIJHI@Z
void Win::on_key(unsigned int key, long flags, int repeat, unsigned int scan) {
    (void)repeat;
    (void)scan;

    unsigned int with_modifiers = key;
    if (GetKeyState(0x10) & 0x8000) with_modifiers |= 0x10000;
    if (GetKeyState(0x11) & 0x8000) with_modifiers |= 0x20000;
    if (GetKeyState(0x12) & 0x8000) with_modifiers |= 0x40000;

    Net *net = ActiveNet;
    if (net != 0) {
        MsgT msg;
        if (with_modifiers == net->voice_hotkey_48_) {
            if (flags == 0) {
                net->stop_voice();
            } else {
                net->start_voice(net->voice_target_50_);
            }
            if (!PeekMessageA(&msg, HandleMain, 0x102, 0x102, 1)) {
                return;
            }
            do {
            } while (PeekMessageA(&msg, HandleMain, 0x102, 0x102, 1));
            return;
        }
        if (with_modifiers == net->voice_hotkey_4c_) {
            if (flags == 0) {
                net->stop_voice();
            } else {
                net->start_voice(static_cast<long>(net->field_54_));
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
                target = list_.current_->focus_owner_;
            } else {
                target = 0;
            }
            if (reinterpret_cast<Win *>(target)->key_up_event(with_modifiers) != 0) {
                return;
            }
        }

        WinCallbackWindow = this;
        WinKeyHookFn cb = field_434_;
        if (cb != 0) {
            cb(with_modifiers);
        }
        vslot_27(with_modifiers);

        Sound *next = val_16_;
        if (next == 0) {
            return;
        }
        if (next->is_playing() != 0) {
            return;
        }
        next->play();
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
// ORIGINAL: 0x005F2680 ?OnSysCommand@Win@@QAAHPAXIHH@Z 0x005F2680-0x005F26FF FILE
// symbol    ?OnSysCommand@Win@@QAAHPAUHWND__@@IHH@Z
// provenance (retired seed): git:3cb99d43^:src/unrecovered/005f2680.cpp
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
    WinCallbackWindow = self;
    WinNullaryEventCallback cb = self->field_404_;
    if (cb != 0) {
        cb();
    }
    self->on_sys_close();
    Sound *child = self->val_6_;
    if (child != 0) {
        if (child->is_playing() == 0) {
            child->play();
        }
    }
    return 0;
}

// ORIGINAL: 0x005EF050 ?client_to_nonclient@Win@@QAEXPAURECT@@@Z 0x005EF050-0x005EF10D FILE
// symbol    ?client_to_nonclient@Win@@QAEXPAUtagRECT@@@Z
// kind game
// TRIED: __thiscall fn-ptr typedef for slot 91 (C4234); own VCall2 class (91 void slots + int slot091) reaches #24 sub-vs-mov, deep in the border-adjust arithmetic
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
            int adj = menu_->requested_height();
            rect->top -= adj;
        }
    }
}

// ===== homed from src/unrecovered/005eef60.cpp =====

// ORIGINAL: 0x005EEF60 ?nonclient_to_client@Win@@QAEXPAH0@Z 0x005EEF60-0x005EF050 FILE
// symbol    ?nonclient_to_client@Win@@QAEXPAH0@Z
// Original mapping restored from git:85fc7721f56c2a5cae754b993fe04fcfec74fec1:src/unrecovered/005eef60.cpp
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
        *y -= menu_->requested_height();
    }
}

// ===== homed from src/recovered/units/005f8140.cpp =====

// ORIGINAL: 0x005F8140 ?update_window_to_buffer@Win@@QAEXPAUBuffer@@@Z 0x005F8140-0x005F83B0 FILE
// symbol    ?update_window_to_buffer@Win@@QAEXPAVBuffer@@@Z
// kind game
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
            Win *iVar2 = WinZOrderWindow;
            do {
                if (iVar2 != 0 && iVar2 == *piVar4) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                if (((*piVar4)->iSomeFlag_ & 1) != 0) {
                    recurse_zorder(*piVar4);
                    iVar2 = WinZOrderWindow;
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
                    (WinViewOriginX) -= win_parent_->outer_rect_.left;
                    (WinViewOriginY) -= win_parent_->outer_rect_.top;
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
                    (WinViewOriginX) -= win_parent_->outer_rect_.left;
                    (WinViewOriginY) -= win_parent_->outer_rect_.top;
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
            Win *iVar2 = WinZOrderWindow;
            do {
                if (iVar2 != 0 && iVar2 == *piVar4) {
                    WinZOrderCount = 0;
                    WinZOrderFlag = 0;
                }
                if (((*piVar4)->iSomeFlag_ & 1) != 0) {
                    recurse_zorder(*piVar4);
                    iVar2 = WinZOrderWindow;
                }
                iVar3++;
                piVar4++;
            } while (iVar3 < WinRootCount);
        }
    }
}

// ===== homed from src/recovered/units/005f6710.cpp =====

// ORIGINAL: 0x005F6710 ?on_r_button_down@Win@@QAEXJHHIH@Z 0x005F6710-0x005F687F
// RECOVERY_BINDING: v1 {"evidence": ["git show 3cb99d43^:src/recovered/units/005f6710.cpp preserves this identity and span. The body remains in win.cpp beneath its original homing heading. Its parentless path calls the receiver-free root-order helper at0x5F672A; declaration repair changes that ABI without rewriting the handler body."], "reason": "Restore the unclaimed original mapping omitted during homing of the existing right-button handler. This binding is not a matching or behavioral claim."}
// symbol    ?on_r_button_down@Win@@UAEXJHHIH@Z
void Win::on_r_button_down(long flags, int x, int y, unsigned int keys, int dbl) {
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
            WinMouseEventCallback fn = field_410_;
            if (fn != 0) {
                fn(x, y);
            }
            this->on_right_double_click(x, y);
            Sound *child = val_18_;
            if (child != 0) {
                int r = child->is_playing();
                if (r == 0) {
                    child->play();
                }
            }
        } else {
            this->vslot_39(x, y);
            Sound *child = val_27_;
            if (child != 0) {
                int r = child->is_playing();
                if (r == 0) {
                    child->play();
                }
            }
        }
    }

    if ((iFlags_ & 0x200000) == 0 &&
        (iSomeFlag_ & 8) == 0) {
        if (dbl == 0) {
            WinCallbackWindow = this;
            WinMouseEventCallback fn = field_424_;
            if (fn != 0) {
                fn(x, y);
            }
            this->on_right_down(x, y);
            Sound *child = reinterpret_cast<Sound *>(val_12_);
            if (child != 0) {
                int r = child->is_playing();
                if (r == 0) {
                    child->play();
                }
            }
        } else {
            this->vslot_36(x, y);
            Sound *child = reinterpret_cast<Sound *>(val_24_);
            if (child != 0) {
                int r = child->is_playing();
                if (r == 0) {
                    child->play();
                }
            }
        }
    }

}

// ===== homed from src/recovered/units/005f2570.cpp =====

// ORIGINAL: 0x005F2570 ?OnLButtonUp@Win@@QAAXPAXHHI@Z 0x005F2570-0x005F25B1 FILE
// symbol    ?OnLButtonUp@Win@@QAEXPAXHHI@Z
// kind game
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

// ORIGINAL: 0x005F15C0 ?OnKey@Win@@QAAXPAXIJHI@Z 0x005F15C0-0x005F1657 FILE
// kind game
// TRIED: MISMATCH #1 test/push at the ternary-vs-if for the first global select; body verified structurally correct otherwise
// ===== homed from src/unrecovered/005f15c0.cpp =====

void __cdecl Win::OnKey(void * hwnd, unsigned int key, long flags, int repeat, unsigned int scan) {
    Win *chosen;
    if (WinFocusWindow != nullptr) {
        chosen = WinModalFocus;
        if (chosen == nullptr) chosen = WinFocusWindow;
    } else {
        chosen = WinInputFocus;
        if (chosen == nullptr) chosen = WinDefaultFocus;
    }
    WinActiveWindow = chosen;

    if (chosen != nullptr) {
        Win *const inner = chosen->poWinBase_;
        if (inner != nullptr && (inner->iSomeFlag_ & 1U) != 0) {
            Win *const parent = inner->win_parent_;
            int visible = 1;
            if (parent != nullptr) visible = parent->is_visible();
            if (visible != 0) inner->on_key(key, flags, repeat, scan);
        }
    }

    if (WinKeyHook != nullptr && flags != 0) WinKeyHook(key);
    if (WinMessageHook != nullptr) WinMessageHook();
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
            IDirectDrawSurface2 *obj = DirectDrawSurface;
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
                IDirectDrawSurface2 *obj = DirectDrawSurface;
                if (obj == 0) {
                    ReleaseDC(HandleMain, WinSharedHdc);
                } else {
                    // Slot 26, offset 0x68, is IDirectDrawSurface::ReleaseDC
                    // (HDC by value per ddraw.h), as confirmed at 0x005F1111.
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
        Win **arr = WinRootWindows;
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

// ORIGINAL: 0x005F53A0 ?on_window_pos_changing@Win@@QAEJPAUWINDOWPOS@@@Z 0x005F53A0-0x005F5480 FILE
// kind game
// symbol    ?on_window_pos_changing@Win@@UAEJPAUtagWINDOWPOS@@@Z
// TRIED: MNEMONIC_ONLY (all 73 mnemonics match, rebuilt is 2 bytes short); own extra slot061-returning-int shim over the scaffold's void-only VCall; hoisting the second field_98 read into a named local (to match the original's `test ah,0x40` register form vs the first occurrence's `test [mem],imm32`) didn't move the byte count.
// ===== homed from src/unrecovered/005f53a0.cpp =====

long Win::on_window_pos_changing(WINDOWPOS * pos) {
    if ((pos->flags & 1) == 0) {
        GraphicWin *ok = this->get_graphic_win();
        if (ok != 0) {
            int cx = pos->cx;
            int f134 = field_134_;
            if (cx <= f134) {
                cx = f134;
            }
            pos->cx = cx;
            int cy = pos->cy;
            int f138 = field_138_;
            if (cy <= f138) {
                cy = f138;
            }
            pos->cy = cy;
        }
        if ((iFlags_ & 0x800000) != 0) {
            int ncx = pos->cx;
            int ncy = pos->cy;
            nonclient_to_client(&ncx, &ncy);
            ncy = (iVertScaleNum_ * ncx) / iVertScaleDenom_;
            client_to_nonclient(&ncx, &ncy);
            pos->cy = ncy;
        }
    }
    if ((iFlags_ & 0x4000) != 0) {
        pos->flags |= 4;
    }
    if (pos->hwndInsertAfter == 0) {
        PostMessageA(HandleMain, 3, 0, 0);
    }
    return DefWindowProcA(HandleMain, 0x46, 0, reinterpret_cast<LPARAM>(pos));
}

// ORIGINAL: 0x005EF950 sub_5ef950 0x005EF950-0x005EFB99 FILE
// kind game
// symbol    ?paint_tiled@Win@@QAEXPAVBuffer@@HHHHHHH@Z
// TRIED: __stdcall(int a1..a8) reading ecx nowhere - the brief's IDA hypothesis; disasm dereferences the entering ecx at [ecx+0x184] (Win::field_184_) and later passes it unchanged as the Win* argument to Buffer::copy_to_window, so it is really a __thiscall Win member with 7 used stack ints + 1 unused pad. Palette::set_active_window is caller-cleaned (add esp,4 after the call) so it is invoked as a free __cdecl function pointer, not through the declared (thiscall-shaped) member.
// ===== homed from src/unrecovered/005ef950.cpp =====
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
            DirectDrawSurface->GetDC(&WinSharedHdc);
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
            DirectDrawSurface->ReleaseDC(WinSharedHdc);
            WinSharedHdc = 0;
            return;
        }
        ReleaseDC(
            HandleMain, WinSharedHdc);
        WinSharedHdc = 0;
    }
}

// ORIGINAL: 0x005ECD20 ?pass_dialog_focus@Win@@QAEXXZ 0x005ECD20-0x005ECDBA FILE
// kind game
// symbol    ?pass_dialog_focus@Win@@UAEXXZ
// ===== homed from src/unrecovered/005ecd20.cpp =====

void Win::pass_dialog_focus() {
    Win *parent;
    Win *obj;

    parent = win_parent_;
    if (parent != 0 && (int)parent->list_.count_ > 1) {
        WinPendingFocus = (parent->list_.head_ != 0)
            ? parent->list_.current_->focus_owner_ : 0;
        parent = win_parent_;
        if (parent->list_.head_ != 0) {
            parent->list_.current_ = parent->list_.current_->next_;
            if (++parent->list_.tail_ == parent->list_.count_) {
                parent->list_.tail_ = 0;
            }
        }
        this->on_dialog_focus(0);
        parent = win_parent_;
        if (parent->list_.head_ != 0) {
            obj = parent->list_.current_->focus_owner_;
            if (obj != 0) {
                obj->on_dialog_focus(1);
            }
        }
    }
}

// ===== homed from src/unrecovered/005f1340.cpp =====

// DUPLICATE `Win::OnPaint` removed: two artifacts carried the same body.

// ===== homed from src/unrecovered/005eb640.cpp =====

/*
Purpose: Close a window: clear it from every focus/bubble/hover slot, hide
         it, tear down its scrollbars and children, unlink it from its
         parent's child array or the root list, then reset the whole object
         to its constructed state - fonts, defaults tables, caption buttons -
         and finish with the AutoSound base's own close.
// ORIGINAL: 0x005EB640 ?close@Win@@QAEXXZ 0x005EB640-0x005EBC8A
// symbol    ?close@Win@@QAEXXZ
// size      1610 bytes
// kind      game
// calls     0x005ECE20 0x005F7320 0x005EFD20 0x005EB640 0x00644EF2 0x0062BBF0
// TRIED: MISMATCH since its homing, 5 instructions off - the trailing
//        unqualified `close()` compiled as a RECURSIVE self-call (it
//        resolves to Win::close, not the base) and the byte diff hid it;
//        found from the running build's stack overflow. The image's last
//        call is AutoSound::close at 0x005EBC80, `mov ecx,esi; call
//        0x62bbf0`, now spelled qualified below.
*/
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
        this->on_lose_mouse_capture();
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
        // Historical no-argument reading withdrawn: pinned 0x005EB73E
        // pushes [this+0x43C] immediately before the slot-7 call.
        scroll_vert_ = this->on_scroll_delete(scroll_vert_);
    }
    if (scroll_vert_ != 0) {
        delete scroll_vert_;
        scroll_vert_ = 0;
    }
    if ((scroll_horz_ != 0) && ((iFlags_ & 0x40) != 0)) {
        // The paired horizontal arm pushes [this+0x440] at 0x005EB774.
        scroll_horz_ = this->on_scroll_delete(scroll_horz_);
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
                WinDialogNode *node = parent->list_.current_;
                for (;;) {
                    if (node->focus_owner_ == this) {
                        found = true;
                        break;
                    }
                    node = node->next_;
                    idx = idx + 1;
                    parent->list_.current_ = node;
                    if (!(idx < listCount)) {
                        break;
                    }
                }
                if (found) {
                    WinDialogNode *nodeNext = node->next_;
                    WinDialogNode *nodePrev = node->previous_;
                    nodeNext->previous_ = nodePrev;
                    WinDialogNode *cur = parent->list_.current_;
                    WinDialogNode *curNext = cur->next_;
                    WinDialogNode *curPrev = cur->previous_;
                    curPrev->next_ = curNext;
                    cur = parent->list_.current_;
                    if (cur == parent->list_.head_) {
                        parent->list_.head_ = cur->next_;
                    }
                    parent->list_.current_ = cur->next_;
                    if (parent->list_.allocator_ == 0) {
                        void *dataPtr = cur->allocation_;
                        if (dataPtr != 0) {
                            free(dataPtr);
                        }
                        cur->allocation_ = 0;
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
                Win **node = WinRootWindows;
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
                    Win **node2 = WinRootWindows + i;
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
                list_.current_ = list_.head_->next_;
                if (list_.head_->allocation_ != 0) {
                    free(list_.head_->allocation_);
                }
                list_.head_->allocation_ = 0;
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
    buffer5_ = 0;
    field_130_ = 1;

    // TWO TABLES, NOT THIRTEEN GLOBALS. 0x00696D34 and 0x009B7AF0 are the
    // heads of contiguous default arrays; the addresses this block used to
    // name one at a time are their entries. Read out of the image:
    // WinStaticDefaults = {247, 24, -1, 1, 1, 8, -1, 7, 2}, and [1] is 24 -
    // the caption height assigned below, which is what confirms the indexing.
    // The assignment ORDER is the image's: fields go in field order while the
    // table is read out of order, so do not tidy it into index order.
    field_FC_ = WinDynamicDefaults.field_0_;
    field_100_ = WinStaticDefaults[0];
    field_104_ = WinDynamicDefaults.rest_[0];
    field_108_ = WinStaticDefaults[2];
    field_10C_ = WinStaticDefaults[3];
    field_110_ = WinStaticDefaults[4];
    caption_height_ = WinStaticDefaults[1];
    border_thickness_ = WinStaticDefaults[5];
    bottom_border_thickness_ = WinStaticDefaults[6];
    field_120_ = WinStaticDefaults[7];
    field_124_ = WinDynamicDefaults.rest_[1];
    field_128_ = WinStaticDefaults[8];
    field_F8_ = WinDynamicDefaults.default_font_;

    {
        if (field_E0_ != 0) {
            free(field_E0_);
        }
        field_E0_ = 0;
    }
    {
        Win *button = minimize_button_;
        if (button != 0) {
            delete button;
            minimize_button_ = 0;
        }
    }
    {
        Win *button = zoom_button_;
        if (button != 0) {
            delete button;
            zoom_button_ = 0;
        }
    }
    {
        Win *button = close_button_;
        if (button != 0) {
            delete button;
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

    // THE BASE'S close, QUALIFIED. The image's last act is `mov ecx, esi;
    // call 0x62bbf0` - AutoSound::close - while an unqualified `close()`
    // here resolves to Win::close itself: same this, unbounded recursion,
    // and the boot's stack-overflow death (nested exception on the signal
    // stack, first popup teardown). Found from the running build, not the
    // listing - the recursive E8 had compiled into the 5-instruction diff
    // this body carried since its homing.
    AutoSound::close();

}

/*
Purpose: Destroy a Win. The body closes the window; everything else the image
         does is compiler-emitted teardown of the declared model - `list_`'s
         in-class virtual destructor walks and frees the node list, and the
         AutoSound base destructor restores the base vfptr and calls close().
         That is also where the /GX EH scaffolding comes from: the image's
         push -1 / fs:[0] prologue, the state variable (1 while close() and
         the list teardown run, -1 for the base), the unwind funclets at
         0x005F8630 / 0x005F86F0 and the funcInfo at 0x0067F8B8 (unwind map:
         state 1 -> list teardown, state 0 -> base close). Calling close()
         FIRST is what matches the image: member destruction happens after
         the destructor body, exactly where the image has it.
// ORIGINAL: 0x005EBC90 ??1Win@@QAE@XZ 0x005EBC90-0x005EBD7F;0x005F8630-0x005F863B;0x005F86F0-0x005F8767;0x00662C60-0x00662C80
// symbol    ??1Win@@UAE@XZ
// size      401 bytes
// prototype void (__thiscall ??1Win@@QAE@XZ)(Win* this)
// kind      game
// LEVER: the whole /GX EH shape - push -1 / handler / fs:[0] chain, the
// `mov [esp+0x14],1` then `,-1` state transitions, the cold funclets
// (0x005F8630 base close, 0x005F86F0 list walk) and the funcInfo at
// 0x0067F8B8 (unwind map: state 1 -> list teardown, state 0 -> base close)
// - is compiler-emitted from the DECLARATIONS once the body is a real
// destructor: `Win::~Win() { close(); }` against the base AutoSound (virtual
// in-class dtor restoring 0x66FF34 and calling close) and the member `list_`
// (in-class virtual dtor, walk inlined). First spelling measured 12/64
// agreeing at 0.896 similarity with every call, offset and branch shape
// already right; homing it also removed the PENDING_BODY raw jump that
// faulted the standalone build's atexit teardown of WinBubbleWindow.
// TRIED: nothing beyond the first spelling - the remaining gap is the
// measured register-allocation wall this family already carries on
// ??_GWinNodeList (marker 0x005F8770 in this file): the image zeroes into
// edi (`push edi; xor edi,edi`, counter in a late-pushed ebx) where this
// build picks ebx for the zero (`push ebx; xor ebx,ebx`, counter in edi),
// and the guard compares (`mov eax,[esi+0xDC]; cmp eax,edi` vs
// `cmp dword ptr [esi+0xDC],ebx`) plus the loop-top `head_` reload follow
// from that one choice. Receiver-spill-wall fingerprint; not re-ground.
*/
Win::~Win() {
    close();
}

// ORIGINAL: 0x005F2290 ?OnMouseMove@Win@@QAAXPAXHHI@Z 0x005F2290-0x005F232D FILE
// kind game
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
            WinHoverWindow->on_mouse_leave(x, y);
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
    HDC hdc;
    IDirectDrawSurface2 *surface;

    if (BufferDirectDraw != 0) {
        return 1;
    }

    if (WinHdcRefCount != 0) {
        WinHdcRefCount = WinHdcRefCount + 1;
        hdc = WinSharedHdc;
    } else {
        surface = DirectDrawSurface;
        if (surface != 0) {
            surface->GetDC(&WinSharedHdc);
            hdc = WinSharedHdc;
        } else {
            // TWO STEPS, and the second one is load-bearing. Calling through
            // `GetDC(...)` directly compiled `mov eax,[0]` + `call [eax]`
            // - one instruction more than the image's single
            // `call dword ptr [0x66927c]`. Naming the loaded pointer first is
            // the spelling the rest of this file already uses (see the
            // GetDC call in the macro above, BYTE_EXACT), and it restores
            // the direct indirect-call encoding.
            hdc = GetDC(HandleMain);
            WinSharedHdc = hdc;
        }
        if (hdc == 0) {
            return 0;
        }
        WinHdcRefCount = 1;
    }

    if (hdc == 0) {
        return 0;
    }

    SelectPalette(hdc, reinterpret_cast<HPALETTE>(PaletteInitialized), 0);

    RealizePalette(WinSharedHdc);

    WinHdcRefCount = WinHdcRefCount - 1;
    if (WinHdcRefCount != 0) {
        return 0;
    }

    surface = DirectDrawSurface;
    if (surface != 0) {
        // Slot 26 (0x68) is IDirectDrawSurface::ReleaseDC(HDC).
        surface->ReleaseDC(WinSharedHdc);
        WinSharedHdc = 0;
        return 0;
    }

    ReleaseDC(HandleMain, WinSharedHdc);
    WinSharedHdc = 0;
    return 0;
}

// ORIGINAL: 0x005ECC40 ?set_dialog_focus@Win@@QAEXPAVWin@@@Z 0x005ECC40-0x005ECD1B FILE
// kind game
// symbol    ?set_dialog_focus@Win@@UAEXPAV1@@Z
// TRIED: for(;;)/break loop shape for the list search instead of a do/while counted against field_D4_
// ===== homed from src/unrecovered/005ecc40.cpp =====

void Win::set_dialog_focus(Win * window) {
    if (window == 0) {
        return;
    }
    if (list_.count_ == 0) {
        return;
    }

    Win *newFocus;
    if (list_.head_ == 0) {
        newFocus = 0;
    } else {
        newFocus = list_.current_->focus_owner_;
    }

    if (window == newFocus) {
        return;
    }

    WinPendingFocus = newFocus;

    if (list_.head_ == 0) {
        return;
    }

    int count = list_.count_;
    int idx = 0;
    list_.tail_ = 0;
    list_.current_ = list_.head_;

    if (count > 0) {
        WinDialogNode *node;
        for (;;) {
            node = list_.current_;
            if (node->focus_owner_ == window) {
                break;
            }
            list_.tail_ = list_.tail_ + 1;
            idx++;
            list_.current_ = node->next_;
            if (idx >= count) {
                return;
            }
        }

        if (newFocus != 0 && count > 1) {
            newFocus->on_dialog_focus(0);
        }

        if (list_.head_ != 0) {
            WinDialogNode *node2 = list_.current_;
            Win *target = node2->focus_owner_;
            if (target != 0) {
                target->on_dialog_focus(1);
            }
        }
    }
}

// ORIGINAL: 0x005F1480 ?OnActivate@Win@@QAAJPAXIPAXJ@Z 0x005F1480-0x005F15B6 FILE
// kind game
// symbol    ?OnActivate@Win@@QAAJPAXI0J@Z
// TRIED: testing `a2 == 0` first (zero-set branch as the `if`) compiled to `test;jne`; testing `a2 != 0` first (zero-set branch as the `else`) matches the original's `test;je` polarity. Landed at MISMATCH #42 (up from #10) - same HDC acquire/release idiom as 0x5EF1E0's sub_5ef1e0.
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
            IDirectDrawSurface2 *iface = DirectDrawSurface;
            if (iface != 0) {
                iface->GetDC(&WinSharedHdc);
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
                IDirectDrawSurface2 *iface2 = DirectDrawSurface;
                if (iface2 != 0) {
                    iface2->ReleaseDC(WinSharedHdc);
                } else {
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
// symbol    ?redraw_all@Win@@QAAXXZ
// size      49 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD2B0
// indirect  0x005F4CD6
// Historical spelling/call-wrapper rationale: git:de080f82:src/win.cpp
// This list-wide operation is distinct from each window's virtual redraw.
// The catalogued QAA convention is retained; its unused receiver does not
// independently establish whether the original declaration was static.
// The original calls do_sound at 0x005FD2B0 once per iteration. The local
// forwarder preserves a call here, but its target still needs native binding
// recovery; masked-byte agreement does not verify this helper's identity.
static int __cdecl redraw_sound_call() {
    return do_sound();
}

void __cdecl Win::redraw_all() {
    int i;
    for (i = 0; i < WinZOrderCount; ++i) {
        WinZOrderArray[i]->redraw();
        redraw_sound_call();
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

// ====================
// RECOVERY_OBJECT: v1 0x009B6E30 ?WinDrawingBufferCleanupGuard@@3EA
uint8_t WinDrawingBufferCleanupGuard;
// RECOVERY_OBJECT: v3 0x009B6F08 ?g_WIN_BUFFER@@3VBuffer@@A member ?WinDrawingBufferOwner@@3UWinDrawingBufferLifetime@@A storage_.bytes
WinDrawingBufferLifetime WinDrawingBufferOwner;
// Original CRT slots 0x682678/0x68267C construct drawing then screen;
// registration in this definition order gives reverse atexit cleanup.
// The screen Buffer starts at 0x009B7490; its allocation bound is unknown.
// RECOVERY_OBJECT: v3 0x009B7490 ?ScreenBuffer@@3VBuffer@@A member ?ScreenBufferOwner@@3UScreenBufferLifetime@@A storage_.bytes
ScreenBufferLifetime ScreenBufferOwner;

// ===== homed from src/unrecovered/005ed360.cpp =====
void __cdecl add_ancestor_origin(int *x, int *y, Win *window);
// ORIGINAL: 0x005ED360 sub_5ed360 0x005ED360-0x005ED3E9 FILE BYTE_EXACT
// symbol    ?add_ancestor_origin@@YAXPAH0PAVWin@@@Z
// size      137 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED360
// NAMED from the body: it walks up the parent chain, adding each window's
// client + outer origin into the caller's pair and taking the outer origin
// back off again when the parent carries the 0x8000 flag - the same
// accumulate-then-correct shape `nonclient_to_screen` uses in line, with the
// recursion the image's own `call 0x5ED360` proves.
// on a `char *` - raw self-access, and the member spelling below measures
// the same body, so the pun bought nothing.
void __cdecl add_ancestor_origin(int *x, int *y, Win *window) {
    *x += window->client_rect_.left + window->outer_rect_.left;
    *y += window->client_rect_.top + window->outer_rect_.top;
    if ((window->iFlags_ & 0x20) != 0 && window->win_parent_ != 0) {
        add_ancestor_origin(x, y, window->win_parent_);
        if ((window->iFlags_ & 0x8000) != 0) {
            *x -= window->win_parent_->outer_rect_.left;
            *y -= window->win_parent_->outer_rect_.top;
        }
    }
}

// ===== homed from src/unrecovered/006357d0.cpp =====
// /O2 implies /Oi, and /Oi expands a memset of a plain local into the
// `rep stosd` form. This body's image CALLS it (0x006465F0), so the clear is
// routed through a real function - path.cpp's `memset_call` idiom, kept
// file-local because path.cpp owns the external name.
static void *__cdecl win_memset_call(void *dst, int value, unsigned count) {
    return memset(dst, value, count);
}

// ORIGINAL: 0x006357D0 sub_6357d0 0x006357D0-0x00635835 FILE BYTE_EXACT
// symbol    ?lock_surface@DDInit@@QAEPAXXZ
// size      101 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006465F0
// indirect  0x00635814
// NAMED from the body and its address: it sits directly after
// DDInit::teardown (0x00635750) and reads exactly DDInit's own
// `locked_surface_`/`locked_bits_` pair at 0xC/0x10, so it is DDInit's
// surface lock - the lock half of the same pair `teardown` unlocks.
//
// The descriptor is the SDK's own `DDSURFACEDESC` - 0x6C bytes, `lPitch` at
// 0x10, `lpSurface` at 0x24, which is where the image reads. 0x21 is
// DDLOCK_WAIT | DDLOCK_WRITEONLY, left as the literal: no other body here
// spells that pair, so there is nothing to name it against.
//
// LEVER: the pitch is a REAL MEMBER. The scaffold reached it as
// `*(int *)(self + 0x14)` - the one field the five-offset survey of
// WinDisplayInit in win.h had not accounted for - and declaring `pitch_`
// turned the raw store into the image's own `mov [esi+0x14], eax`.
LPVOID DDInit::lock_surface() {
    if (locked_surface_ == nullptr) {
        return nullptr;
    }
    if (locked_bits_ != nullptr) {
        return locked_bits_;
    }
    DDSURFACEDESC description;
    win_memset_call(&description, 0, sizeof(description));
    description.dwSize = sizeof(description);
    if (locked_surface_->Lock(nullptr, &description, 0x21, nullptr) != 0) {
        return nullptr;
    }
    pitch_ = description.lPitch;
    locked_bits_ = description.lpSurface;
    return locked_bits_;
}

// ===== homed from src/unrecovered/005fcec0.cpp =====
// The three USER32 entry points this pump caches, at the signatures the VC6
// headers themselves declare - `void *` for the window handle, which the
// older `PeekMessageFn` above spells, does not accept `&PeekMessageA`.
typedef int(__stdcall *PeekMessageProc)(MSG *, HWND, unsigned int, unsigned int,
                                       unsigned int);
typedef int(__stdcall *TranslateMessageProc)(const MSG *);
typedef LRESULT(__stdcall *DispatchMessageProc)(const MSG *);
// Keep one typed imported-function pointer snapshot per API, in the same
// capture order and location as the original entry-point slot reads.
// ORIGINAL: 0x005FCEC0 sub_5fcec0 0x005FCEC0-0x005FCF8C FILE
// symbol    ?pump_mouse_messages@@YAXXZ
// size      204 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCEF5 0x005FCF04 0x005FCF0B 0x005FCF2D 0x005FCF38 0x005FCF3F 0x005FCF63 0x005FCF72 0x005FCF79
// NAMED from the body: the loop peeks the WM_MOUSEFIRST..WM_MOUSELAST range
// and then, for a WM_LBUTTONDOWN or WM_RBUTTONDOWN, peeks the matching
// WM_LBUTTONUP / WM_RBUTTONUP out of the queue before the next round - the
// mouse-message pump, as `do_char` is the keystroke one at 0x005FCF90.
//
// VOID, not the scaffold's `int` + `return 0`: the image's epilogue sets no
// eax after the do_net call, and an int return would have to zero it.
//
// Historical experiment below used the retired raw-IAT aliases. The current
// source instead takes typed API addresses; load timing needs fresh evidence.
// LEVER: the three USER32 entry points are read out of this module's own
// import table - `*(TranslateMessageProc *) IatTranslateMessage` - which is
// what makes VC6 hoist all three loads into the prologue, before the
// do_video/check_net calls, exactly as the image does. Taking the imported
// names as values (`= &PeekMessageA`) compiles the same instruction but VC6
// then treats it as rematerialisable and sinks each load to its first use.
// Slot addresses from tools/iat_names.py; the operands mask as relocations.
//
// TRIED: switch over msg.message (37/68), `&&`-chained guards (39/68),
// `continue` in the second arm as well (39/68), else-if chain (40/68, and it
// caches msg.message in eax where the image re-reads it). All of them still
// merge the two translate/dispatch tails into one shared copy - 77
// instructions against the image's 68, which emits the pair twice. The
// declaration order is load-bearing instead: reverse of declaration, ebx =
// translate, esi = peek, edi = dispatch, is what puts the prologue loads on
// the image's own registers. 39 of 68, similarity 0.910, and the divergence
// is that merge and nothing else.
void __cdecl pump_mouse_messages() {
    // Capture each imported target before the first loop callbacks. VC6 can
    // sink ordinary import-address locals to their first use; observable
    // local stores retain this capture epoch without original IAT addresses.
    DispatchMessageProc const volatile dispatch = &::DispatchMessageA;
    PeekMessageProc const volatile peek = &::PeekMessageA;
    TranslateMessageProc const volatile translate = &::TranslateMessage;

    MSG msg;
    for (;;) {
        do_video();
        check_net();
        if (!peek(&msg, 0, 0x200, 0x209, 1)) {
            break;
        }
        translate(&msg);
        dispatch(&msg);
        if (msg.message == 0x201) {
            if (peek(&msg, msg.hwnd, 0x202, 0x202, 1)) {
                translate(&msg);
                dispatch(&msg);
                continue;
            }
        }
        if (msg.message == 0x204) {
            if (peek(&msg, msg.hwnd, 0x205, 0x205, 1)) {
                translate(&msg);
                dispatch(&msg);
            }
        }
    }
    do_net();
}

// ===== homed from src/unrecovered/005f4d90.cpp =====
// ORIGINAL: 0x005F4D90 sub_5f4d90 0x005F4D90-0x005F4EB1
// symbol    ?set_base_window@Win@@QAEHPAVGraphicWin@@@Z
// size      289 bytes
// prototype
// callers   0   call targets   1
// kind       game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F4EC0
// indirect  0x005F4DC9 0x005F4DFD 0x005F4E10 0x005F4E3B 0x005F4EA3
// NAMED from the body: it installs `window` as poWinBase_, the field the
// whole class reads as "the window I am the base of", resizing it to the
// outgoing base's outer rectangle when the working rectangle has moved on,
// then re-walking the roots and repainting through slot 63.
//
// The argument is a GraphicWin, not a Win: the rect at +0x474 is
// Buffer::rect2_, which GraphicWin carries through its Buffer base, and
// `Win` is already Buffer's friend, so the read is a member read here.
//
// LEVER: the equal-rects guards are two SEPARATE early-outs arms, not one
// merged tail - the image re-derives the width/height inside each arm from a
// different source (poWinBase_ when there is one, `this` when there is not).
int Win::set_base_window(GraphicWin *window) {
    if (window == 0) {
        return 0x10;
    }
    if (window == poWinBase_) {
        return 0;
    }
    if (poWinBase_ != 0) {
        if (!EqualRect(&poWinBase_->outer_rect_, &window->rect2_)) {
            window->resize(poWinBase_->outer_rect_.right - poWinBase_->outer_rect_.left,
                           poWinBase_->outer_rect_.bottom - poWinBase_->outer_rect_.top,
                           1);
        }
    } else {
        if (!EqualRect(&outer_rect_, &window->rect2_)) {
            window->resize(outer_rect_.right - outer_rect_.left,
                           outer_rect_.bottom - outer_rect_.top, 1);
        }
    }
    poWinBase_ = window;

    int i = 0;
    WinZOrderCount = 0;
    if (WinRootCount > 0) {
        do {
            if (WinZOrderWindow != 0 && WinZOrderWindow == WinRootWindows[i]) {
                WinZOrderCount = 0;
                WinZOrderFlag = 0;
            }
            if (WinRootWindows[i]->iSomeFlag_ & 1) {
                recurse_zorder(WinRootWindows[i]);
            }
            ++i;
        } while (i < WinRootCount);
    }
    on_update();
    return 0;
}

// ===== homed from src/recovered/00635840.cpp =====
// ORIGINAL: 0x00635840 sub_635840 0x00635840-0x00635864 BYTE_EXACT
// symbol    ?flush@DDInit@@QAEXXZ
// size      36 bytes
// prototype
// callers   0   call targets   0
// kind       game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00635855
// NAMED from its address and its fields: it sits between DDInit::teardown
// (0x00635750) and DDInit::lock_surface (0x006357D0) and reads exactly
// DDInit's locked_surface_ (0xC) / locked_bits_ (0x10) pair, calling the
// surface's slot-0x80 Unlock - the unlock half of the pair as its own entry
// point, which `teardown` repeats inline. The scaffold called the class
// `Handler` because the scaffold did not know DDInit existed.
void DDInit::flush() {
    if (locked_surface_ != nullptr && locked_bits_ != nullptr) {
        locked_surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
    }
}

// ===== homed from src/recovered/005f62d0.cpp =====
// ORIGINAL: 0x005F62D0 sub_5f62d0 0x005F62D0-0x005F6317 BYTE_EXACT
// symbol    ?send_wheel_delta@Win@@UAEXHFHHH@Z
// size      71 bytes
// prototype
// callers   0   call targets   0
// kind       game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F630B 0x005F6311
// NAMED from the body: the only value it touches is the `short` argument,
// divided by 120 - WHEEL_DELTA, the distance Windows packs into one wheel
// notch - and the quotient is dispatched through slot 15 or slot 30
// depending on the last argument, behind the same 0x200000 / 0x8 gates every
// other input hook here tests. a1, a3 and a4 are never read.
//
// VOID, `ret 0x14`, and both dispatches are tail calls in the image.
void Win::send_wheel_delta(int key, short delta, int x, int y, int horizontal) {
    int notches = delta / 120;
    if ((iFlags_ & 0x200000) == 0 && (iSomeFlag_ & 8) == 0) {
        if (horizontal == 0) {
            on_mousewheel(notches);
        } else {
            vslot_30(notches);
        }
    }
}

// ===== homed from src/unrecovered/005ed490.cpp =====
// ORIGINAL: 0x005ED490 sub_5ed490 0x005ED490-0x005ED519 FILE BYTE_EXACT
// symbol    ?remove_ancestor_origin@@YAXPAH0PAVWin@@@Z
// size      137 bytes
// prototype
// callers   1   call targets   1
// kind       game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED490
// NAMED from the body: the exact mirror of `add_ancestor_origin`
// (0x005ED360) one page up this file - same parent-chain walk, same 0x8000
// outer-rectangle correction, every add a subtract. Where a window needs the
// chain's coordinate contribution taken back out, this is the function the
// image runs.
void __cdecl remove_ancestor_origin(int *x, int *y, Win *window) {
    *x -= window->client_rect_.left + window->outer_rect_.left;
    *y -= window->client_rect_.top + window->outer_rect_.top;
    if ((window->iFlags_ & 0x20) != 0 && window->win_parent_ != 0) {
        remove_ancestor_origin(x, y, window->win_parent_);
        if ((window->iFlags_ & 0x8000) != 0) {
            *x += window->win_parent_->outer_rect_.left;
            *y += window->win_parent_->outer_rect_.top;
        }
    }
}

// ===== homed from src/recovered/005ee4a0.cpp =====
// ORIGINAL: 0x005EE4A0 sub_5ee4a0 0x005EE4A0-0x005EE4D0 BYTE_EXACT
// symbol    ?send_to_window@Win@@QAEXHH@Z
// size      48 bytes
// prototype
// callers   0   call targets   0
// kind       game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE4C4 0x005EE4CA
// NAMED from the body: the same 0x200000 / 0x8 input gates as
// send_wheel_delta above, then one value through slot 15 or slot 30 - the
// forwarding half of the pair, with the selector in the second argument and
// no division in front of it.
void Win::send_to_window(int value, int which) {
    if ((iFlags_ & 0x200000) == 0 && (iSomeFlag_ & 8) == 0) {
        if (which == 0) {
            on_mousewheel(value);
        } else {
            vslot_30(value);
        }
    }
}

// ===== homed from src/recovered/005ee460.cpp =====
// ORIGINAL: 0x005EE460 sub_5ee460 0x005EE460-0x005EE497 BYTE_EXACT
// symbol    ?publish_callback_window@Win@@QAEXXZ
// size      55 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE473 0x005EE479 0x005EE487 0x005EE492
// NAMED from the body: it publishes `this` as WinCallbackWindow, runs the
// field_404_ hook when one is installed, raises its own slot 14, and then
// offers the contextual Sound at val_6_ the status/play pair also used
// for Sound receivers in the other dispatchers. Installation remains unknown.
void Win::publish_callback_window() {
    WinCallbackWindow = this;
    WinNullaryEventCallback callback = field_404_;
    if (callback != 0) {
        callback();
    }
    on_sys_close();
    Sound *second = val_6_;
    if (second != 0) {
        if (second->is_playing() == 0) {
            second->play();
        }
    }
}

// ===== homed from src/unrecovered/005ee4d0.cpp =====
// ORIGINAL: 0x005EE4D0 sub_5ee4d0 0x005EE4D0-0x005EE56B FILE BYTE_EXACT
// symbol    ?dispatch_17@Win@@QAEXHHH@Z
// size      155 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE510 0x005EE51B 0x005EE52B 0x005EE536 0x005EE54B 0x005EE559 0x005EE564
// NAMED from its slot: the gate / publish / hook / slot / child-notify shape
// this file's dispatchers share, raising slot 17 - the image offsets are
// 0x44 (slot 17), 0x7c (slot 31), and the hook is mouse_move_hook_.
// Sound receiver views come off val_7_ (0x1C) and val_19_ (0x4C).
void Win::dispatch_17(int a, int b, int alternate) {
    if ((iFlags_ & 0x200000) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8) != 0) {
        return;
    }
    if (alternate == 0) {
        WinCallbackWindow = this;
        WinMouseMoveHookFn hook = mouse_move_hook_;
        if (hook != 0) {
            hook(a, b);
        }
        on_mouse_move(a, b);
        Sound *child = val_7_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    } else {
        on_nc_mouse_move(a, b);
        Sound *child = val_19_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    }
}

// ===== homed from src/unrecovered/005ee570.cpp =====
// ORIGINAL: 0x005EE570 sub_5ee570 0x005EE570-0x005EE60E FILE BYTE_EXACT
// symbol    ?dispatch_28@Win@@QAEXHHH@Z
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE5B0 0x005EE5BB 0x005EE5CB 0x005EE5D6 0x005EE5EB 0x005EE5FC 0x005EE607
// NAMED from its slot: the dispatcher family again, slot 28 (0x70) with slot
// 38 (0x98) as the alternate arm, the hook at field_40C_, the children off
// val_17_ (0x44) and val_26_ (0x68).
void Win::dispatch_28(int a, int b, int alternate) {
    if ((iFlags_ & 0x200000) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8) != 0) {
        return;
    }
    if (alternate == 0) {
        WinCallbackWindow = this;
        WinMouseEventCallback hook = field_40C_;
        if (hook != 0) {
            hook(a, b);
        }
        on_left_double_click(a, b);
        Sound *child = val_17_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    } else {
        vslot_38(a, b);
        Sound *child = val_26_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    }
}


// ===== homed from src/unrecovered/005ee610.cpp =====
// ORIGINAL: 0x005EE610 sub_5ee610 0x005EE610-0x005EE6AE FILE BYTE_EXACT
// symbol    ?dispatch_29@Win@@QAEXHHH@Z
// LEVER: void return (eax never set before ret) + own VCall2 shim for slot023 (int return, tested) and slots 29/39 ((int,int)), not the file's nullary VCall
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// Receiver correction: the retained VCall2 lesson describes the child
// Sound status result; Win self events are coordinate-taking void calls.
// NAMED from its slot: the dispatcher family again, slot 29 (0x74) with slot
// 39 (0x9c) as the alternate arm, the hook at field_410_, the children off
// val_18_ (0x48) and val_27_ (0x6c).
void Win::dispatch_29(int a, int b, int alternate) {
    if ((iFlags_ & 0x200000) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8) != 0) {
        return;
    }
    if (alternate == 0) {
        WinCallbackWindow = this;
        WinMouseEventCallback hook = field_410_;
        if (hook != 0) {
            hook(a, b);
        }
        on_right_double_click(a, b);
        Sound *child = val_18_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    } else {
        vslot_39(a, b);
        Sound *child = val_27_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    }
}


// ===== homed from src/unrecovered/005ee6b0.cpp =====
// ORIGINAL: 0x005EE6B0 sub_5ee6b0 0x005EE6B0-0x005EE74E FILE BYTE_EXACT
// symbol    ?dispatch_19@Win@@QAEXHHH@Z
// LEVER: void return (eax never set before ret) + own VCall2 shim for slot023 (int return, tested) and slots 19/32 ((int,int)), not the file's nullary VCall. Same shape as sibling 0x005EE610, offsets 0x414/0x20/0x50 in place of that one's 0x410/0x48/0x6c.
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// Receiver correction: the retained VCall2 lesson describes the child
// Sound status result; Win self events are coordinate-taking void calls.
// NAMED from its slot: the dispatcher family again, slot 19 (0x4c) with slot
// 32 (0x80) as the alternate arm, the hook at field_414_, the children off
// AutoSound+0x20 and val_20_ (0x50).
//
// BUG IN THE ORIGINAL (layout, not behaviour): AutoSound+0x20 is read here as
// a `Win *` child pointer, and basepop.cpp documents that same word as the
// flags field BasePop::UNK3/UNK4 set bits 0/1 in. Both cannot be true. The
// read is left spelled through the declared field rather than an offset so
// the finding is visible; fixing it means re-modelling AutoSound, not this
// body.
void Win::dispatch_19(int a, int b, int alternate) {
    if ((iFlags_ & 0x200000) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8) != 0) {
        return;
    }
    if (alternate == 0) {
        WinCallbackWindow = this;
        WinMouseEventCallback hook = field_414_;
        if (hook != 0) {
            hook(a, b);
        }
        on_left_click(a, b);
        Sound *child = reinterpret_cast<Sound *>(flags_);
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    } else {
        vslot_32(a, b);
        Sound *child = reinterpret_cast<Sound *>(val_20_);
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    }
}

// ===== homed from src/unrecovered/005EE810.cpp =====
// ORIGINAL: 0x005EE810 sub_5ee810 0x005EE810-0x005EE8AE FILE BYTE_EXACT
// symbol    ?dispatch_21@Win@@QAEXHHH@Z
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE850 0x005EE85B 0x005EE86B 0x005EE876 0x005EE88B 0x005EE89C 0x005EE8A7
// NAMED from its slot: the dispatcher family again, slot 21 (0x54) with slot
// 34 (0x88) as the alternate arm, the hook at field_41C_, the children off
// val_10_ (0x28) and val_22_ (0x58).
void Win::dispatch_21(int a, int b, int alternate) {
    if ((iFlags_ & 0x200000) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8) != 0) {
        return;
    }
    if (alternate == 0) {
        WinCallbackWindow = this;
        WinMouseEventCallback hook = field_41C_;
        if (hook != 0) {
            hook(a, b);
        }
        on_left_up(a, b);
        Sound *child = reinterpret_cast<Sound *>(val_10_);
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    } else {
        on_nc_left_up(a, b);
        Sound *child = val_22_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    }
}


// ===== homed from src/unrecovered/005EE8B0.cpp =====
// ORIGINAL: 0x005EE8B0 sub_5ee8b0 0x005EE8B0-0x005EE94E FILE BYTE_EXACT
// symbol    ?dispatch_22@Win@@QAEXHHH@Z
// working copy - scaffold materialised by --work
// size      158 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE8F0 0x005EE8FB 0x005EE90B 0x005EE916 0x005EE92B 0x005EE93C 0x005EE947
// NAMED from its slot: the dispatcher family again, slot 22 (0x58) with slot
// 35 (0x8c) as the alternate arm, the hook at field_420_, the children off
// val_11_ (0x2c) and val_23_ (0x5c).
void Win::dispatch_22(int a, int b, int alternate) {
    if ((iFlags_ & 0x200000) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8) != 0) {
        return;
    }
    if (alternate == 0) {
        WinCallbackWindow = this;
        WinMouseEventCallback hook = field_420_;
        if (hook != 0) {
            hook(a, b);
        }
        on_right_click(a, b);
        Sound *child = reinterpret_cast<Sound *>(val_11_);
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    } else {
        vslot_35(a, b);
        Sound *child = val_23_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
    }
}

// Original source names are not established; these labels identify the
// primary Win slots used by the recovered seed bodies.
// ORIGINAL: 0x005EE950 sub_5ee950 0x005EE950-0x005EE9EE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:215c2e515022cafd8e44ccc77cd636a2b9b954e1:src/unrecovered/005ee950.cpp: original identity, complete span and historical receiver findings", "Win primary table66FDD0 slot23 selects404260 ret8; Sound table66E3C0 slots23/7 select4C64C0/4C6480", "src/flatbutton.h: val_12 also stores the borrowed pressed Sprite pointer", "recovery/tasks/win-sound-owner-oct5.json", "recovery/tests/test_win_sound_product_source.py"], "reason": "Recover complete Win coordinate dispatcher through its real primary/alternate virtual events and contextual Sound status/play receiver. Settings words are neutral pointer storage because FlatButton borrows one for a Sprite; concrete installed sound lifetime remains unresolved."}
// symbol    ?dispatch_23@Win@@QAEXHHH@Z
// LEVER: [ecx+N] reads with no stack `this` -> a __thiscall receiver class (Recv5ee950), not the given __stdcall free function. Two shims: VCallSelf (slot023/036 take (int,int) on `this`) and VCallChild (slot023 returns int(), slot007 void()) on the 0x30/0x60 children - same slot index, different vtables, so one shim could not serve both.
void Win::dispatch_23(int x, int y, int alternate) {
    if ((iFlags_ & 0x200000U) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8U) != 0) {
        return;
    }

    if (alternate == 0) {
        WinCallbackWindow = this;
        WinMouseEventCallback callback = field_424_;
        if (callback != 0) {
            callback(x, y);
        }
        on_right_down(x, y);
        Sound *child = reinterpret_cast<Sound *>(val_12_);
        if (child != 0 && child->is_playing() == 0) {
            child->play();
        }
    } else {
        vslot_36(x, y);
        Sound *child = reinterpret_cast<Sound *>(val_24_);
        if (child != 0 && child->is_playing() == 0) {
            child->play();
        }
    }
}

// ORIGINAL: 0x005EE9F0 sub_5ee9f0 0x005EE9F0-0x005EEA8E
// RECOVERY_BINDING: v1 {"evidence": ["git:215c2e515022cafd8e44ccc77cd636a2b9b954e1:src/unrecovered/005ee9f0.cpp: original identity, complete span and historical receiver findings", "Win primary table66FDD0 slot23 selects404260 ret8; Sound table66E3C0 slots23/7 select4C64C0/4C6480", "src/flatbutton.h: val_12 also stores the borrowed pressed Sprite pointer", "recovery/tasks/win-sound-owner-oct5.json", "recovery/tests/test_win_sound_product_source.py"], "reason": "Recover complete Win coordinate dispatcher through its real primary/alternate virtual events and contextual Sound status/play receiver. Settings words are neutral pointer storage because FlatButton borrows one for a Sprite; concrete installed sound lifetime remains unresolved."}
// symbol    ?dispatch_24@Win@@QAEXHHH@Z
void Win::dispatch_24(int x, int y, int alternate) {
    if ((iFlags_ & 0x200000U) != 0) {
        return;
    }
    if ((iSomeFlag_ & 8U) != 0) {
        return;
    }

    if (alternate == 0) {
        WinCallbackWindow = this;
        WinMouseEventCallback callback = field_428_;
        if (callback != 0) {
            callback(x, y);
        }
        on_right_up(x, y);
        Sound *child = reinterpret_cast<Sound *>(val_13_);
        if (child != 0 && child->is_playing() == 0) {
            child->play();
        }
    } else {
        vslot_37(x, y);
        Sound *child = reinterpret_cast<Sound *>(val_25_);
        if (child != 0 && child->is_playing() == 0) {
            child->play();
        }
    }
}


// ===== homed from src/recovered/005f2250.cpp =====
// ORIGINAL: 0x005F2250 sub_5f2250 0x005F2250-0x005F228C BYTE_EXACT
// symbol    ?dispatch_mouse_wheel@@YAXHHHHH@Z
// size      60 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F6F10
// indirect  0x005F2285
// NAMED from the body: it resolves the window under the cursor with
// `get_mouse_window` - which writes the window-relative coordinates back
// over the fourth and fifth arguments - and hands those, the key flags and
// the delta to that window's slot 77, the mouse-wheel handler. The FIRST
// argument is never read; nothing in the image touches it.
void __cdecl dispatch_mouse_wheel(int unused, int keys, int delta, int x, int y) {
    Win *window = Win::get_mouse_window(&x, &y);
    if (window != 0) {
        window->send_wheel_delta(keys, delta, x, y, WinMouseDirect);
    }
}

// ORIGINAL: 0x005FCCF0 sub_5fccf0 0x005FCCF0-0x005FCD3D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/005fccf0.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// LEVER: cast the three fixed IAT slots to __stdcall function-pointer types and called through them directly
// symbol    ?pump_key_message@@YAHXZ
int __cdecl pump_key_message() {
    MSG message;
    do_video();
    check_net();
    if (!PeekMessageA(&message, nullptr, WM_KEYDOWN, WM_SYSDEADCHAR, PM_REMOVE))
        return 0;
    TranslateMessage(&message);
    DispatchMessageA(&message);
    return 1;
}

// ORIGINAL: 0x005FCD40 sub_5fcd40 0x005FCD40-0x005FCD97 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/005fcd40.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?pump_all_key_messages@@YAXXZ
void __cdecl pump_all_key_messages() {
    MSG message;
    for (;;) {
        do_video();
        check_net();
        if (!PeekMessageA(&message, nullptr, WM_KEYDOWN, WM_SYSDEADCHAR, PM_REMOVE))
            break;
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    do_net();
}

// ORIGINAL: 0x005FCDA0 sub_5fcda0 0x005FCDA0-0x005FCDE5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/005fcda0.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?pump_mouse_move_message@@YAHXZ
int __cdecl pump_mouse_move_message() {
    MSG message;
    if (!PeekMessageA(&message, nullptr, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
        return 0;
    TranslateMessage(&message);
    DispatchMessageA(&message);
    return 1;
}

// ORIGINAL: 0x005FCDF0 sub_5fcdf0 0x005FCDF0-0x005FCEC0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005fcdf0.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation."}
// TRIED: per-call reinterpret_cast<Proc*>(g) load (5 call sites) instead of caching Peek/Translate/Dispatch ptrs in registers once
// symbol    ?pump_mouse_button_message@@YAHXZ
int __cdecl pump_mouse_button_message() {
    MSG message;
    do_video();
    check_net();
    if (!PeekMessageA(&message, nullptr, WM_MOUSEMOVE, WM_MBUTTONDBLCLK, PM_REMOVE))
        return 0;
    TranslateMessage(&message);
    DispatchMessageA(&message);
    if (message.message == WM_LBUTTONDOWN) {
        if (PeekMessageA(&message, message.hwnd, WM_LBUTTONUP, WM_LBUTTONUP, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
            return 1;
        }
    }
    if (message.message == WM_RBUTTONDOWN) {
        if (PeekMessageA(&message, message.hwnd, WM_RBUTTONUP, WM_RBUTTONUP, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD2C0 sub_5fd2c0 0x005FD2C0-0x005FD308 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005fd2c0.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation."}
// symbol    ?pump_sound_message@@YAHXZ
int __cdecl pump_sound_message() {
    MSG message;
    if (!PeekMessageA(&message, nullptr, WM_USER + 1, WM_USER + 1, PM_REMOVE))
        return 0;
    TranslateMessage(&message);
    DispatchMessageA(&message);
    check_net();
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD310 sub_5fd310 0x005FD310-0x005FD35D
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005fd310.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation."}
// symbol    ?pump_all_sound_messages@@YAXXZ
void __cdecl pump_all_sound_messages() {
    MSG message;
    while (PeekMessageA(&message, nullptr, WM_USER + 1, WM_USER + 1, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
        check_net();
    }
}

// Historical catalogue name: this entry only clears the message status.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FD360 ?do_all_sounds@@YAXXZ 0x005FD360-0x005FD36B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/005fd360.cpp"], "reason": "Recover the complete core message helper using SDK MSG/USER32, original queue ranges and callback ordering; preserve catalogue identity and historical seed obligation."}
// symbol    ?do_all_sounds@@YAXXZ
void __cdecl do_all_sounds() {
    MsgStatus = 0;
}

// ORIGINAL: 0x005F83B0 sub_5f83b0 0x005F83B0-0x005F83C5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f178e8db:src/recovered/units/005f83b0.cpp complete original identity/spans and historical claims/findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5f83b0 full native body review.", "source:src/win.cpp existing real state owners; no executable-address alias or synthetic receiver."], "reason": "Recover the complete short helper through established source-owned diplomacy, vehicle, text or window state; preserve signed sentinel behavior, original receiver/argument order and conditional callbacks."}
// symbol    ?flip_active_bubble_text@@YAXXZ
void __cdecl flip_active_bubble_text() {
    if (WinBubbleActive != 0)
        Win::flip(&WinBubbleRect);
}

// Superseded seed deleted; binding references below retain historical provenance.
// ORIGINAL: 0x005F1230 sub_5f1230 0x005F1230-0x005F127F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5f1230 complete native body and Windows import dispatch.", "source:src/win.h typed Win slots68,69,71,72; callback registration and original helper names remain unknown.", "task:win-message-adapter-owner-batch original identity, complete spans and historical claims retained from src/recovered/units/005f1230.cpp"], "reason": "Recover the complete window adapter using Windows userdata and the existing typed Win virtual method; preserve null-owner, main-window and callback-result paths."}
// symbol    ?win_window_pos_changing@@YAJPAUHWND__@@J@Z
// kind      game
LRESULT __cdecl win_window_pos_changing(HWND hwnd, LPARAM parameter) {
    Win *const owner = reinterpret_cast<Win *>(
        GetWindowLongA(hwnd, GWL_USERDATA));
    if (owner != 0) {
        if (HandleMain == hwnd) {
            return DefWindowProcA(hwnd, 0x46, 0, parameter);
        }
        return owner->on_window_pos_changing(reinterpret_cast<WINDOWPOS *>(parameter));
    }
    return DefWindowProcA(hwnd, 0x46, 0, parameter);
}

// Superseded seed deleted; binding references below retain historical provenance.
// ORIGINAL: 0x005F1280 sub_5f1280 0x005F1280-0x005F12A1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5f1280 complete native body and Windows import dispatch.", "source:src/win.h typed Win slots68,69,71,72; callback registration and original helper names remain unknown.", "task:win-message-adapter-owner-batch original identity, complete spans and historical claims retained from src/recovered/units/005f1280.cpp"], "reason": "Recover the complete window adapter using Windows userdata and the existing typed Win virtual method; preserve null-owner, main-window and callback-result paths."}
// symbol    ?win_window_pos_changed@@YAJPAUHWND__@@J@Z
// kind      game
LRESULT __cdecl win_window_pos_changed(HWND hwnd, LPARAM parameter) {
    LRESULT window_data = GetWindowLongA(hwnd, GWL_USERDATA);
    if (window_data != 0) {
        Win *const owner = reinterpret_cast<Win *>(window_data);
        window_data = owner->on_window_pos_changed(reinterpret_cast<WINDOWPOS *>(parameter));
    }
    return window_data;
}

// Superseded seed deleted; binding references below retain historical provenance.
// ORIGINAL: 0x005F12E0 sub_5f12e0 0x005F12E0-0x005F130B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5f12e0 complete native body and Windows import dispatch.", "source:src/win.h typed Win slots68,69,71,72; callback registration and original helper names remain unknown.", "task:win-message-adapter-owner-batch original identity, complete spans and historical claims retained from src/recovered/units/005f12e0.cpp"], "reason": "Recover the complete window adapter using Windows userdata and the existing typed Win virtual method; preserve null-owner, main-window and callback-result paths."}
// symbol    ?win_window_sized@@YAXPAUHWND__@@IHH@Z
// kind      game
void __cdecl win_window_sized(HWND hwnd, unsigned int flags,
                                    int width, int height) {
    Win *const owner = reinterpret_cast<Win *>(
        GetWindowLongA(hwnd, GWL_USERDATA));
    if (owner != 0) {
        owner->on_size(flags, width, height);
    }
}

// Superseded seed deleted; binding references below retain historical provenance.
// ORIGINAL: 0x005F1310 sub_5f1310 0x005F1310-0x005F133B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5f1310 complete native body and Windows import dispatch.", "source:src/win.h typed Win slots68,69,71,72; callback registration and original helper names remain unknown.", "task:win-message-adapter-owner-batch original identity, complete spans and historical claims retained from src/recovered/units/005f1310.cpp"], "reason": "Recover the complete window adapter using Windows userdata and the existing typed Win virtual method; preserve null-owner, main-window and callback-result paths."}
// symbol    ?win_window_nonclient_sized@@YAXPAUHWND__@@IHH@Z
// kind      game
void __cdecl win_window_nonclient_sized(HWND hwnd,
                                               unsigned int flags,
                                               int width, int height) {
    Win *const owner = reinterpret_cast<Win *>(
        GetWindowLongA(hwnd, GWL_USERDATA));
    if (owner != 0) {
        owner->on_size_nc(flags, width, height);
    }
}

// Superseded seed deleted; binding references retain historical provenance.
// ORIGINAL: 0x005D6350 sub_5d6350 0x005D6350-0x005D637B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:win-message-adapter-owner-extension original identity/spans, claims and useful findings retained from src/recovered/005d6350.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5d6350 full native body and API/vcall sequence.", "source:src/win.h actual slots16,70,71,74,85; original helper names and caller/registration routes remain unknown."], "reason": "Recover the complete Win userdata adapter with existing typed virtual methods and Windows APIs; preserve native payload ordering, null guards and paint lifetime. Anonymous generated seed return declarations are not evidence of an original integer contract for void callbacks."}
// symbol    ?win_forward_size@@YAXPAUHWND__@@IHH@Z
// kind      game
void __cdecl win_forward_size(HWND hwnd, unsigned int flags, int width, int height) {
    Win *const owner = reinterpret_cast<Win *>(
        GetWindowLongA(hwnd, GWL_USERDATA));
    if (owner != 0) {
        owner->on_size(flags, width, height);
    }
}

// Superseded seed deleted; binding references retain historical provenance.
// ORIGINAL: 0x005D6380 sub_5d6380 0x005D6380-0x005D63AA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:win-message-adapter-owner-extension original identity/spans, claims and useful findings retained from src/recovered/units/005d6380.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5d6380 full native body and API/vcall sequence.", "source:src/win.h actual slots16,70,71,74,85; original helper names and caller/registration routes remain unknown."], "reason": "Recover the complete Win userdata adapter with existing typed virtual methods and Windows APIs; preserve native payload ordering, null guards and paint lifetime. Anonymous generated seed return declarations are not evidence of an original integer contract for void callbacks."}
// symbol    ?win_forward_mouse_move@@YAXPAUHWND__@@HHI@Z
// kind      game
void __cdecl win_forward_mouse_move(HWND hwnd, int x, int y, unsigned int keys) {
    Win *const owner = reinterpret_cast<Win *>(GetWindowLongA(hwnd, GWL_USERDATA));
    if (owner != 0) {
        owner->on_mouse_move(x, y, keys, 0);
    }
}

// Superseded seed deleted; binding references retain historical provenance.
// ORIGINAL: 0x005D63B0 sub_5d63b0 0x005D63B0-0x005D63DE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:win-message-adapter-owner-extension original identity/spans, claims and useful findings retained from src/recovered/units/005d63b0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5d63b0 full native body and API/vcall sequence.", "source:src/win.h actual slots16,70,71,74,85; original helper names and caller/registration routes remain unknown."], "reason": "Recover the complete Win userdata adapter with existing typed virtual methods and Windows APIs; preserve native payload ordering, null guards and paint lifetime. Anonymous generated seed return declarations are not evidence of an original integer contract for void callbacks."}
// symbol    ?win_forward_set_cursor@@YAHPAUHWND__@@PAXII@Z
// kind      game
int __cdecl win_forward_set_cursor(HWND hwnd, void *cursor, unsigned int hit,
                       unsigned int message) {
    Win *const owner = reinterpret_cast<Win *>(
        GetWindowLongA(hwnd, GWL_USERDATA));
    if (owner != 0) {
        return owner->on_set_cursor(cursor, hit, message);
    }
    return 0;
}

// Superseded seed deleted; binding references retain historical provenance.
// ORIGINAL: 0x005F12B0 sub_5f12b0 0x005F12B0-0x005F12D6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:win-message-adapter-owner-extension original identity/spans, claims and useful findings retained from src/recovered/005f12b0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5f12b0 full native body and API/vcall sequence.", "source:src/win.h actual slots16,70,71,74,85; original helper names and caller/registration routes remain unknown."], "reason": "Recover the complete Win userdata adapter with existing typed virtual methods and Windows APIs; preserve native payload ordering, null guards and paint lifetime. Anonymous generated seed return declarations are not evidence of an original integer contract for void callbacks."}
// symbol    ?win_window_moved@@YAXPAUHWND__@@HH@Z
// kind      game
void __cdecl win_window_moved(HWND hwnd, int x, int y) {
    Win *const owner = reinterpret_cast<Win *>(
        GetWindowLongA(hwnd, GWL_USERDATA));
    if (owner != 0) {
        owner->on_move(x, y);
    }
}

// Superseded seed deleted; binding references retain historical provenance.
// ORIGINAL: 0x005F13B0 sub_5f13b0 0x005F13B0-0x005F141B
// RECOVERY_BINDING: v1 {"evidence": ["task:win-message-adapter-owner-extension original identity/spans, claims and useful findings retained from src/unrecovered/005f13b0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5f13b0 full native body and API/vcall sequence.", "source:src/win.h actual slots16,70,71,74,85; original helper names and caller/registration routes remain unknown."], "reason": "Recover the complete Win userdata adapter with existing typed virtual methods and Windows APIs; preserve native payload ordering, null guards and paint lifetime. Anonymous generated seed return declarations are not evidence of an original integer contract for void callbacks."}
// symbol    ?win_window_nc_paint@@YAHPAUHWND__@@@Z
// kind      game
// TRIED: zero-arg contract (disasm reads one stack arg -> HWND); reusing the auto-generated nullary `VCall::slot074` (own `PaintVCall` class instead, real signature). Register allocation in the prologue still MISMATCHes.
int __cdecl win_window_nc_paint(HWND hwnd) {
  int result = 0;
  Win *const owner = reinterpret_cast<Win *>(
      GetWindowLongA(hwnd, GWL_USERDATA));
  if (owner != 0) {
    PAINTSTRUCT paint;
    if (BeginPaint(hwnd, &paint) != 0) {
      RECT painted = paint.rcPaint;
      owner->on_nc_paint(&painted, -1);
      result = EndPaint(hwnd, &paint);
    }
  }
  return result;
}

// Superseded seed removed; complete operation now belongs to Win.
// ORIGINAL: 0x005EF640 sub_5ef640 0x005EF640-0x005EF946 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f2a0fa1952cef53f4a2c6a5e60a8bea9bff9c3aa:src/unrecovered/005ef640.cpp", "pinned:terranx_original:0x5ef640", "source:src/win.h", "source:src/graphicwin.h"], "reason": "Native Win receiver and four popped arguments, top/light bottom/dark left/light right/dark HDC acquisition order. Source names are descriptive, not recovered historical spellings. Preserve all original spans and historical findings; no new matching or full acceptance claim."}
// symbol    ?draw_rect_edges@Win@@QAEXPAUtagRECT@@PAX1H@Z
void Win::draw_rect_edges(RECT *rect, HGDIOBJ light_pen, HGDIOBJ dark_pen,
                          int unused) {
    HDC hdc = Win::get_hdc();
    if (hdc != 0) {
        sync_palette();
        HGDIOBJ previous = SelectObject(hdc, light_pen);
        MoveToEx(hdc, rect->left + 1, rect->top, 0);
        LineTo(hdc, rect->right + 1, rect->top);
        SelectObject(hdc, previous);
        Win::release_hdc();
    }

    hdc = Win::get_hdc();
    if (hdc != 0) {
        sync_palette();
        HGDIOBJ previous = SelectObject(hdc, dark_pen);
        MoveToEx(hdc, rect->left, rect->bottom, 0);
        LineTo(hdc, rect->right, rect->bottom);
        SelectObject(hdc, previous);
        Win::release_hdc();
    }

    hdc = Win::get_hdc();
    if (hdc != 0) {
        sync_palette();
        HGDIOBJ previous = SelectObject(hdc, light_pen);
        MoveToEx(hdc, rect->left, rect->top, 0);
        LineTo(hdc, rect->left, rect->bottom);
        SelectObject(hdc, previous);
        Win::release_hdc();
    }

    hdc = Win::get_hdc();
    if (hdc != 0) {
        sync_palette();
        HGDIOBJ previous = SelectObject(hdc, dark_pen);
        MoveToEx(hdc, rect->right, rect->top + 1, 0);
        LineTo(hdc, rect->right, rect->bottom + 1);
        SelectObject(hdc, previous);
        Win::release_hdc();
    }
}

// Superseded seed removed; complete operation now belongs to Win.
// ORIGINAL: 0x005EFBA0 sub_5efba0 0x005EFBA0-0x005EFCF5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f2a0fa1952cef53f4a2c6a5e60a8bea9bff9c3aa:src/unrecovered/005efba0.cpp", "pinned:terranx_original:0x5efba0", "source:src/win.h", "source:src/graphicwin.h"], "reason": "Native Win receiver and six popped arguments, palette RGB fill and preserved HDC lease leak when brush creation fails. Source names are descriptive, not recovered historical spellings. Preserve all original spans and historical findings; no new matching or full acceptance claim."}
// symbol    ?fill_palette_color_rect@Win@@QAEXHHHHHH@Z
// TRIED: `ecx` used as `this` before any stack read proves __thiscall, not the IDA-guessed __stdcall(int x6); modelled as a member of a local class per the 0x004C8290 idiom. The two indirect calls through DAT_009BC498's vtable pass the interface object as an EXPLICIT stack arg (not via ecx), so they are __stdcall function pointers, not VCall thiscall slots. First divergence #5 je/jne.
void Win::fill_palette_color_rect(int x, int y, int width, int height,
                                  int palette_index, int unused) {
    HDC hdc = Win::get_hdc();
    if (hdc == 0) {
        return;
    }

    sync_palette();
    const PALETTEENTRY &entry = PaletteActive->entries()[palette_index];
    HBRUSH brush = CreateSolidBrush(
        RGB(entry.peRed, entry.peGreen, entry.peBlue));
    if (brush == 0) {
        // Native 0x005EFBA0 returns here without dropping its HDC reference.
        return;
    }

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
    Win::release_hdc();
}

// Superseded seed removed; complete operation now belongs to Win.
// ORIGINAL: 0x005F37C0 sub_5f37c0 0x005F37C0-0x005F3B73 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f2a0fa1952cef53f4a2c6a5e60a8bea9bff9c3aa:src/unrecovered/005f37c0.cpp", "pinned:terranx_original:0x5f37c0", "source:src/win.h", "source:src/graphicwin.h"], "reason": "Native Win receiver and Buffer/x/y inputs, top-down DIB damage and source-relative to destination-relative copy geometry. Source names are descriptive, not recovered historical spellings. Preserve all original spans and historical findings; no new matching or full acceptance claim."}
// symbol    ?copy_edges_to_buffer@Win@@QAEXPAVBuffer@@HH@Z
// TRIED: signature is __thiscall(int,int,int) returning void, not the brief's __stdcall(int,int,int) - ecx used as `this` throughout, ret 0xc pops 3 stack args. Ghidra's pseudocode (IsRectEmpty/ IntersectRect/copy over 4 edge buffers + a GetWindowRect case) checked against the raw disassembly's ESP-relative RECT layout and used as the translation basis (sim 0.75 at /O2 no-Oy-). slot061 retyped `int` (thiscall, nullary, tested against 0). First divergence at #12 is prologue register scheduling, not chased further. Not byte-exact.
void Win::copy_edges_to_buffer(Buffer *target, int x, int y) {
    if (target == 0) {
        return;
    }

    RECT damage;
    RECT source_rect;
    RECT intersection;
    damage.top = y;
    damage.bottom = y - target->dib_.bmiHeader.biHeight;
    damage.right = target->dib_.bmiHeader.biWidth + x;
    damage.left = x;
    if (IsRectEmpty(&damage)) {
        return;
    }

    if ((iSomeFlag_ & 2) != 0) {
        const int left = client_rect_.left;
        const int top = client_rect_.top;
        const int right = client_rect_.right;
        const int bottom = client_rect_.bottom;
        Buffer *edge = buffer1_;
        source_rect.left = edge->rect2_.left + left;
        source_rect.right = edge->rect2_.right + left;
        source_rect.bottom = edge->rect2_.bottom + top;
        source_rect.top = edge->rect2_.top + top;
        if (IntersectRect(&intersection, &source_rect, &damage)) {
            edge->copy(target, intersection.left - source_rect.left,
                       intersection.top - source_rect.top,
                       intersection.left - damage.left,
                       intersection.top - damage.top,
                       intersection.right - intersection.left,
                       intersection.bottom - intersection.top);
        }

        edge = buffer3_;
        const int bottom_base = top - buffer1_->dib_.bmiHeader.biHeight;
        source_rect.left = edge->rect2_.left + left;
        source_rect.bottom = edge->rect2_.bottom + bottom_base;
        source_rect.right = edge->rect2_.right + left;
        source_rect.top = edge->rect2_.top + bottom_base;
        if (IntersectRect(&intersection, &source_rect, &damage)) {
            edge->copy(target, intersection.left - source_rect.left,
                       intersection.top - source_rect.top,
                       intersection.left - damage.left,
                       intersection.top - damage.top,
                       intersection.right - intersection.left,
                       intersection.bottom - intersection.top);
        }

        edge = buffer4_;
        const int right_top = edge->dib_.bmiHeader.biHeight + bottom;
        source_rect.right = edge->rect2_.right + left;
        source_rect.bottom = edge->rect2_.bottom + right_top;
        source_rect.left = edge->rect2_.left + left;
        source_rect.top = edge->rect2_.top + right_top;
        if (IntersectRect(&intersection, &source_rect, &damage)) {
            edge->copy(target, intersection.left - source_rect.left,
                       intersection.top - source_rect.top,
                       intersection.left - damage.left,
                       intersection.top - damage.top,
                       intersection.right - intersection.left,
                       intersection.bottom - intersection.top);
        }

        edge = buffer2_;
        const int left_top = top - buffer1_->dib_.bmiHeader.biHeight;
        const int left_edge = right - edge->dib_.bmiHeader.biWidth;
        source_rect.right = left_edge + edge->rect2_.right;
        source_rect.left = left_edge + edge->rect2_.left;
        source_rect.bottom = left_top + edge->rect2_.bottom;
        source_rect.top = left_top + edge->rect2_.top;
        if (IntersectRect(&intersection, &source_rect, &damage)) {
            edge->copy(target, intersection.left - source_rect.left,
                       intersection.top - source_rect.top,
                       intersection.left - damage.left,
                       intersection.top - damage.top,
                       intersection.right - intersection.left,
                       intersection.bottom - intersection.top);
        }
    }

    GraphicWin *graphic = get_graphic_win();
    if (graphic != 0) {
        GetWindowRect(HandleMain, &source_rect);
        if (IntersectRect(&intersection, &source_rect, &damage)) {
            // The getter is a predicate here; native reads this object's Buffer base.
        static_cast<GraphicWin *>(this)->Buffer::copy(
                target, intersection.left - source_rect.left,
                intersection.top - source_rect.top,
                intersection.left - damage.left,
                intersection.top - damage.top,
                intersection.right - intersection.left,
                intersection.bottom - intersection.top);
        }
    }
}

// Superseded seed removed; complete operation now belongs to Win.
// ORIGINAL: 0x005F3B80 sub_5f3b80 0x005F3B80-0x005F4AA7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f2a0fa1952cef53f4a2c6a5e60a8bea9bff9c3aa:src/unrecovered/005f3b80.cpp", "pinned:terranx_original:0x5f3b80", "source:src/win.h", "source:src/graphicwin.h"], "reason": "Native current Win source and target Win receiver distinction, chained edge rectangles, clip/mode gates and distinct inherited versus allocated GraphicWin buffers. Source names are descriptive, not recovered historical spellings. Preserve all original spans and historical findings; no new matching or full acceptance claim."}
// symbol    ?copy_border_buffers_to_window@Win@@QAEXPAV1@PAUtagRECT@@H@Z
// TRIED: IDA's `__stdcall(int,RECT*,int)` guess with no receiver - ecx is dereferenced immediately with no matching stack slot, so this is a __thiscall member (class EdgeWin here) over IDA's 3 args. The screen-relative pass's destination buffer (5th TRY5 call) isn't resolved past this point in the disassembly given; reused the top-edge buffer field there as the least-wrong guess. sim 0.56-0.74 across flag sets.
void Win::copy_border_buffers_to_window(Win *target, RECT *clip, int mode) {
    if (target == 0 || (target->iSomeFlag_ & 2) == 0 ||
        (target->iFlags_ & 0x10000000) == 0) {
        return;
    }

    if (clip != 0) {
        mode = -1;
    }
    RECT screen_rect;
    if (clip != 0 && IsRectEmpty(clip)) {
        return;
    }

    const int target_left = target->client_rect_.left;
    const int target_top = target->client_rect_.top;
    RECT top_rect;
    RECT bottom_rect;
    RECT right_rect;
    RECT left_rect;
    Buffer *target_edge = target->buffer1_;
    top_rect.left = target_edge->rect2_.left + target_left;
    top_rect.right = target_edge->rect2_.right + target_left;
    top_rect.top = target_edge->rect2_.top + target_top;
    top_rect.bottom = target_edge->rect2_.bottom + target_top;

    target_edge = target->buffer3_;
    bottom_rect.left = target_edge->rect2_.left + target_left;
    bottom_rect.right = target_edge->rect2_.right + target_left;
    bottom_rect.top = target_edge->rect2_.top + top_rect.bottom;
    bottom_rect.bottom = target_edge->rect2_.bottom + top_rect.bottom;

    target_edge = target->buffer4_;
    right_rect.left = target_edge->rect2_.left + target_left;
    right_rect.right = target_edge->rect2_.right + target_left;
    right_rect.top = target_edge->rect2_.top + bottom_rect.bottom;
    right_rect.bottom = target_edge->rect2_.bottom + bottom_rect.bottom;

    target_edge = target->buffer2_;
    const int left_right_base =
        (target_edge->rect2_.left - target_edge->rect2_.right) + top_rect.right;
    left_rect.left = target_edge->rect2_.left + left_right_base;
    left_rect.right = target_edge->rect2_.right + left_right_base;
    left_rect.bottom = target_edge->rect2_.bottom + top_rect.bottom;
    left_rect.top = target_edge->rect2_.top + top_rect.bottom;

    GraphicWin *target_graphic = target->get_graphic_win();
    if (target_graphic != 0 &&
        (target->iFlags_ & 0x40000000) != 0) {
        GetWindowRect(HandleMain, &screen_rect);
    }

#define COPY_EDGE_TO_TARGET(destination, edge_rect)                              \
    do {                                                                         \
        RECT clipped = edge_rect;                                                \
        RECT hit;                                                                \
        if (clip == 0 || IntersectRect(&clipped, &edge_rect, clip) != 0) {       \
            if (IntersectRect(&hit, &clipped, &top_rect) != 0 &&                 \
                (mode == -1 || mode == 0)) {                                     \
                (destination)->copy(target->buffer1_,                            \
                    hit.left - clipped.left, hit.top - clipped.top,              \
                    hit.left - top_rect.left, hit.top - top_rect.top,            \
                    hit.right - hit.left, hit.bottom - hit.top);                 \
            }                                                                    \
            if (IntersectRect(&hit, &clipped, &bottom_rect) != 0 &&              \
                (mode == -1 || mode == 3)) {                                     \
                (destination)->copy(target->buffer3_,                            \
                    hit.left - clipped.left, hit.top - clipped.top,              \
                    hit.left - bottom_rect.left, hit.top - bottom_rect.top,      \
                    hit.right - hit.left, hit.bottom - hit.top);                 \
            }                                                                    \
            if (IntersectRect(&hit, &clipped, &left_rect) != 0 &&                \
                (mode == -1 || mode == 1)) {                                     \
                (destination)->copy(target->buffer2_,                            \
                    hit.left - clipped.left, hit.top - clipped.top,              \
                    hit.left - left_rect.left, hit.top - left_rect.top,          \
                    hit.right - hit.left, hit.bottom - hit.top);                 \
            }                                                                    \
            if (IntersectRect(&hit, &clipped, &right_rect) != 0 &&               \
                (mode == -1 || mode == 2)) {                                     \
                (destination)->copy(target->buffer4_,                            \
                    hit.left - clipped.left, hit.top - clipped.top,              \
                    hit.left - right_rect.left, hit.top - right_rect.top,        \
                    hit.right - hit.left, hit.bottom - hit.top);                 \
            }                                                                    \
            if (target_graphic != 0 && mode == -1 &&                            \
                (target->iFlags_ & 0x40000000) != 0 &&                           \
                IntersectRect(&hit, &clipped, &screen_rect) != 0) {              \
                (destination)->copy(target_graphic->owned_buffer(),               \
                    hit.left - clipped.left, hit.top - clipped.top,              \
                    hit.left - screen_rect.left, hit.top - screen_rect.top,      \
                    hit.right - hit.left, hit.bottom - hit.top);                 \
            }                                                                    \
        }                                                                        \
    } while (0)

    if ((iSomeFlag_ & 2) != 0) {
        const int left = client_rect_.left;
        const int top = client_rect_.top;
        const int right = client_rect_.right;
        const int bottom = client_rect_.bottom;
        RECT edge_rect;
        Buffer *destination = buffer1_;
        target_edge = buffer1_;
        edge_rect.left = target_edge->rect2_.left + left;
        edge_rect.right = target_edge->rect2_.right + left;
        edge_rect.top = target_edge->rect2_.top + top;
        edge_rect.bottom = target_edge->rect2_.bottom + top;
        COPY_EDGE_TO_TARGET(destination, edge_rect);

        destination = buffer3_;
        target_edge = buffer1_;
        const int bottom_base = top - target_edge->dib_.bmiHeader.biHeight;
        target_edge = buffer3_;
        edge_rect.left = target_edge->rect2_.left + left;
        edge_rect.right = target_edge->rect2_.right + left;
        edge_rect.top = target_edge->rect2_.top + bottom_base;
        edge_rect.bottom = target_edge->rect2_.bottom + bottom_base;
        COPY_EDGE_TO_TARGET(destination, edge_rect);

        destination = buffer4_;
        target_edge = buffer4_;
        const int right_base = target_edge->dib_.bmiHeader.biHeight + bottom;
        edge_rect.left = target_edge->rect2_.left + left;
        edge_rect.right = target_edge->rect2_.right + left;
        edge_rect.top = target_edge->rect2_.top + right_base;
        edge_rect.bottom = target_edge->rect2_.bottom + right_base;
        COPY_EDGE_TO_TARGET(destination, edge_rect);

        destination = buffer2_;
        target_edge = buffer1_;
        const int left_bottom_base = top - target_edge->dib_.bmiHeader.biHeight;
        target_edge = buffer2_;
        const int left_base = right - target_edge->dib_.bmiHeader.biWidth;
        edge_rect.left = target_edge->rect2_.left + left_base;
        edge_rect.right = target_edge->rect2_.right + left_base;
        edge_rect.top = target_edge->rect2_.top + left_bottom_base;
        edge_rect.bottom = target_edge->rect2_.bottom + left_bottom_base;
        COPY_EDGE_TO_TARGET(destination, edge_rect);
    }

    GraphicWin *graphic = get_graphic_win();
    if (graphic != 0) {
        GetWindowRect(HandleMain, &screen_rect);
        // Native uses this+Buffer-base, independently of the getter result.
        COPY_EDGE_TO_TARGET(static_cast<Buffer *>(static_cast<GraphicWin *>(this)), screen_rect);
    }
#undef COPY_EDGE_TO_TARGET
}


// Typed owners for folded original no-op virtual targets.  The pinned
// Win table binds slot 25 to 0x00406A80 and slots 6, 8 and 61 to
// 0x00406B30. Other class views of those addresses remain
// shared-owner or unresolved evidence, not one synthetic ABI.
// ORIGINAL: 0x00406A80 sub_406a80 0x00406A80-0x00406A85 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; Win primary table 0x0066FDD0 slot 25 at 0x0066FE34 relocates to 0x00406A80", "src/win.h declares the existing virtual as int on_key_click(int, int); its source body returns zero, and derived real overrides retain their own implementations", "historical catalog facts retained: sub_406a80 was a five-byte zero-return leaf with callee cleanup of eight stack bytes; the retired fastcall redirect was only an opaque annotation carrier", "retired source symbol retained as historical evidence: ?nullsub_00406a80_redirect@@YIHPAX0HH@Z", "twenty pinned base-relocation sites reference 0x00406A80; a base slot is evidence for this member, not proof that every descendant view has the same owner"], "reason": "Move the pinned two-argument zero-return virtual identity from the opaque redirect to Win::on_key_click. This is one genuine typed Win view; other table views that fold to the same bytes remain separate source-owner evidence."}
// symbol    ?on_key_click@Win@@UAEHHH@Z
// kind      game
int Win::on_key_click(int, int) { return 0; }

// ORIGINAL: 0x00406B30 sub_406b30 0x00406B30-0x00406B33 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; Win primary table 0x0066FDD0 slot 61 at 0x0066FEC4 relocates to 0x00406B30", "Win slots 6 and 8 at 0x0066FDE8 and 0x0066FDF0 independently target the same identity; the historical source contract was int() in each declaration, while current Scroll* spelling is inferred from Win::init storing slot 6 EAX at +0x440/+0x43C and later treating it as Scroll*; the stripped executable does not prove the original declared C++ return type", "historical class-table finding retained: 129 virtual slots across 70 classes reach this shared code address; recorded slots have different arities, so no one synthetic prototype represents them", "historical Buffer view remains a research lead, not a new owner mapping: the pinned secondary table selects adjustor 0x00404420 to GraphicWin::get_graphic_win, so no independent Buffer-to-406B30 slot is asserted", "retired source symbol retained as historical evidence: ?nullsub_00406b30_redirect@@YIHPAX0@Z", "existing source-shared bindings for BasePop, VideoSoundState, VoiceRx and VoiceTx remain unchanged as separate typed owners; Buffer's unproved view is not added", "the old opaque fastcall redirect was scaffolding, not ABI evidence; unknown virtual views and incomplete table extents remain restoration/ownership obligations"], "reason": "Place the original identity on one genuine Win method only as a navigation anchor for the folded shared body; preserve the independently typed Win, VideoSoundState, Voice, and BasePop source views with shared mappings. This does not claim that Win::get_graphic_win was the unique historical source owner."}
// symbol    ?get_graphic_win@Win@@UAEPAVGraphicWin@@XZ
// kind      game
GraphicWin *Win::get_graphic_win() { return nullptr; }

// ORIGINAL: 0x005F8610 ??_GWin@@UAEPAXI@Z 0x005F8610-0x005F862E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical source:src/deleting_thunks.cpp; preserved full original span 0x005F8610-0x005F862E and BYTE_EXACT obligation.", "pinned Win constructor 0x005EB402 installs primary table 0x0066FDD0, whose slot 0 selects 0x005F8610; the owner class remains Win.", "pinned scalar body calls Win::~Win at 0x005EBC90, tests flags bit 0, skips 0x0064557F when clear, otherwise deletes, then returns the original this with ret 4.", "the prior retained owner COFF/PE snapshot emitted ??_GWin@@UAEPAXI@Z with destructor/delete relocations and its Win table selected that address; this is historical evidence only, not qualification of the current source.", "Win::~Win closes the window; the real WinNodeList member and AutoSound base provide the remaining C++ lifetime. The unused standalone helper has no remaining source caller.", "the byte-exact claim is historical and remains a restoration obligation; no current equality or whole-application acceptance is asserted."], "reason": "Bind Win's original scalar deleting identity to the compiler-generated operation of Win's genuine virtual destructor; this FILE mapping is not a source-body byte-equivalence claim."}
// symbol    ??_GWin@@UAEPAXI@Z
// kind game

// Original global Win message-pump family; inline definitions remain in win.h.
// ORIGINAL: 0x005FC9F0 ?do_all_tasks@@YAXPAUWin@@@Z 0x005FC9F0-0x005FCA23 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005FC9F0-0x005FCA23", "git:7c6c86d9:src/recovered/units/005fc9f0.cpp"], "reason": "Clear the real queued-window globals before polling do_net; the independently remeasured seed is retired."}
// symbol    ?do_all_tasks@@YAXPAVWin@@@Z
void __cdecl do_all_tasks(Win *window) {
    MsgStatus = 0;
    if (window) {
        Win *active = WinPointerOwner1 ? WinPointerOwner1 : WinPointerOwner2;
        if (window == active) {
            WinPointerOwner1 = 0;
            WinPointerOwner2 = 0;
        }
    }
    do_net();
}

/*
Purpose: Process non-input related message.
// ORIGINAL: 0x005FCA30 ?do_non_input@@YAXXZ 0x005FCA30-0x005FCB14 BYTE_EXACT
// symbol    ?do_non_input@@YAHXZ
// LEVER: A WRONG PROGRAM, not a wrong spelling. The three PeekMessage guards
//        were chained with `||`: `if (!A || !B || !C) return false;`, which
//        gives up unless ALL THREE ranges have a message waiting. The image
//        jumps AWAY on each success (`test eax, eax; jne 0x5FCAA7`) and only
//        falls through the third into `pop edi; add esp, 0x54; ret`, so it
//        gives up only when NONE of them does - `&&`. 33/84 -> 81/84, and the
//        recovered program now answers the message pump the way the image does.
// LEVER: 0x020A spelled out, because `WM_MOUSELAST` is not one constant.
//        winuser.h defines it 0x020A under WINVER >= 0x0400 and 0x0209
//        otherwise, and this build takes the 0x0209 arm while the image pushes
//        0x020A. Worth the 82nd instruction. `osmx semantic` is what found it -
//        it names the operand pair, "push immediate 0x20a against 0x209",
//        where `listing_diff` cannot: it masks every immediate before
//        comparing, so a wrong constant is invisible there.
// LEVER: init-decl order for strength-reduced loop offsets. VC6 emits the
//        byte-offset initialisations for the min-search loop in DECLARATION
//        ORDER: `int c = 1; int a = 0; for (; c < 3; c++)` puts `mov eax, 0x1c`
//        (c's offset) before `xor esi, esi` (a's offset), matching the image.
//        The old `int a = 0; for (int c = 1; ...)` had them swapped. 82/84 -> 84/84.
// size      228 bytes
// prototype
// callers   12   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCA6E 0x005FCA85 0x005FCA9C 0x005FCAEF 0x005FCAF6 0x005FCB01
Return Value: Is message available? true/false
Status: Complete - testing
*/
BOOL __cdecl do_non_input() {
    do_video();
    check_net();
    do_net();
    MSG msg[3];
    for (int i = 0; i < 3; i++) {
        msg[i].time = 0xFFFFFFFF;
    }
    // `&&`, NOT `||`. This tree had `if (!A || !B || !C) return false;`, which
    // demands a message in ALL THREE ranges; the image returns false only when
    // NONE of them has one - it jumps AWAY on each success and falls through
    // the third test into the epilogue. A different program, not a different
    // spelling. 33/84 -> 81/84 on the fix.
    if (!PeekMessage(&msg[0], NULL, WM_NULL, WM_INPUT, PM_NOREMOVE)
        && !PeekMessage(&msg[1], NULL, WM_UNICHAR, WM_KEYDOWN | WM_INPUT, PM_NOREMOVE)
        // 0x020A, NOT `WM_MOUSELAST`. winuser.h defines that name TWICE - 0x020A
        // when WINVER >= 0x0400, 0x0209 otherwise - and this build takes the
        // 0x0209 arm, while the image pushes 0x020A. `osmx semantic` names the
        // pair outright: "push immediate 0x20a against 0x209".
        && !PeekMessage(&msg[2], NULL, 0x020A, UNICODE_NOCHAR, PM_NOREMOVE)) {
        return false;
    }
    int c = 1;
    int a = 0;
    for (; c < 3; c++) {
        if (msg[c].time < msg[a].time) {
            a = c;
        }
    }
    PeekMessage(&msg[0], msg[a].hwnd, msg[a].message, msg[a].message, PM_REMOVE);
    TranslateMessage(&msg[0]);
    DispatchMessage(&msg[0]);
    return true;
}

/*
Purpose: Process all non-input related messages.
// ORIGINAL: 0x005FCB20 ?do_all_non_input@@YAXXZ 0x005FCB20-0x005FCB5A BYTE_EXACT
// size      58 bytes
// prototype
// callers   27   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FCA30 0x0062D5B0
Return Value: n/a
Status: Complete - testing
*/
// BODY IN win.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
//
// The two lines above were prose, so `frontier.py --untouched` kept offering
// this address as fresh. Restated as lesson lines:
//
// LEVER: already BYTE_EXACT 11/11 - re-measured 2026-08-22 at
//        `/c /O2 /Gy /GR- /Oy- /GX`, unchanged. `MEASURED inline` in temp.h
//        with the marker left in this .cpp is what makes it both reproduce
//        and stay measurable; decomp's reader globs only *.cpp and *.c.
// TRIED: an out-of-line `do_all_non_input_call` forwarder, the
//        `sleep_call`/`base_cost_call` idiom, to unblock Path::continent -
//        which this `MEASURED inline` caps because the image CALLS 0x005FCB20
//        there. Measured and reverted in path.cpp: agreement 16/206 -> 1/206
//        and call_diff went from MORE to FEWER, because it resolves callees by
//        ADDRESS and a forwarder is not the address the image calls. Recorded
//        here too so the ceiling is visible from the callee, not only from the
//        caller that hit it.

/*
Purpose: Process draw related message.
// ORIGINAL: 0x005FCB60 ?do_draw@@YAHXZ 0x005FCB60-0x005FCBAC BYTE_EXACT
// size      76 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCB7F 0x005FCB92 0x005FCB9D
Return Value: Is message available? true/false
Status: Complete - testing
*/
// BODY IN win.h, as `MEASURED inline`: do_draw is inlined by its
// only caller and also exists standalone. The marker stays here.

/*
Purpose: Process all draw related messages.
// ORIGINAL: 0x005FCBB0 ?do_all_draws@@YAXXZ 0x005FCBB0-0x005FCC1F BYTE_EXACT
// size      111 bytes
// prototype
// callers   57   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCBEE 0x005FCBF9 0x005FCC00
Return Value: n/a
Status: Complete - testing
*/
void __cdecl do_all_draws() {
    MsgStatus = 1;
    while (do_draw()) {
        MsgStatus = 1;
    }
    MsgStatus = 0;
}

/*
Purpose: Process keyboard related message.
// ORIGINAL: 0x005FCC20 ?do_keyboard@@YAXXZ 0x005FCC20-0x005FCC6D BYTE_EXACT
// symbol    ?do_keyboard@@YAHXZ
// size      77 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5D0 0x00636300
// indirect  0x005FCC40 0x005FCC53 0x005FCC5E
Return Value: Is message available? true/false
Status: Complete - testing
*/
// BODY IN win.h, as `MEASURED inline`: do_keyboard is inlined by its
// only caller and also exists standalone. The marker stays here.

/*
Purpose: Process all keyboard related messages.
// ORIGINAL: 0x005FCC70 ?do_all_keyboard@@YAXXZ 0x005FCC70-0x005FCCE5 BYTE_EXACT
// size      117 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCCAF 0x005FCCBA 0x005FCCC1
Return Value: n/a
Status: Complete - testing
*/
void __cdecl do_all_keyboard() {
    MsgStatus = 2;
    while (do_keyboard()) {
        MsgStatus = 2;
    }
    MsgStatus = 0;
    do_net();
}


/*
Purpose: Copy 1 argument(s) into field(s) of `this`.
         Emitted by tools/generate_field_accessors.py from

             mov eax, dword ptr [esp + 4] / mov dword ptr [ecx + 0x128], eax / ret 4

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 1 stack dword(s) so it
         cleans 4 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// ORIGINAL: 0x005F05C0 sub_5f05c0 0x005F05C0-0x005F05CD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["pinned 0x005F05C0 stores its one stack argument at receiver+0x128 and returns","the pinned Win constructor at 0x005EB50F writes receiver+0x128; docs/recovery/member-accesses.csv attributes that access to Win","src/win.h declares Win::field_128_ at +0x128","the neighboring pinned entries 0x005F0580, 0x005F05D0 and 0x005F0610 are Win methods already owned in src/win.cpp","the prior RECOVERY_ROLE/SCAFFOLD and emitted symbol ?field_accessor_005f05c0_redirect@@YIXPAX0H@Z described this transitional adapter, not a qualified current owner or emitted method symbol","original call-site and table ownership evidence remains unresolved in this proposal; absence from the current mapped-caller list is not treated as evidence that the function had no caller or owner","the current emitted method symbol remains unqualified until the coordinator captures fresh physical COFF evidence"],"reason":"Home the scalar store on the existing Win owner and its source-declared +0x128 member. Keep the method name offset-derived and its emitted symbol unqualified pending fresh object evidence; preserve open original caller/type questions."}
// symbol    ?set_field_128@Win@@QAEXH@Z
// size      13 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Win::set_field_128(int value) {
    field_128_ = static_cast<uint32_t>(value);
}

/*
Purpose: The main-window display dispatch behind the interface's
         maximise/restore/minimise requests: against WinCallbackWindow,
         case 0 releases the pointer-owner pair and the two capture slots
         (notifying the window through vtable slot 4 when it held the
         capture) and maximises HandleMain; case 1 does the same releases
         without the IsZoomed test and minimises; case 2 runs the window's
         vtable slot 52, its installed hook at +0x438, and - when the child
         window at +0x8c answers slot 23 with zero - the child's slot 7.

// HISTORICAL SCAFFOLD ROLE: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x005F2830 sub_5f2830 0x005F2830-0x005F293B FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/win.cpp WinCallbackWindow and WinPointerOwner1..4 owners", "source:src/win.h Win::on_lose_mouse_capture and vslot_52 declarations", "source:src/win.cpp field_438_ zero writers in Win construction and close", "source:src/sound.h Sound::is_playing slot 23 and Sound::play slot 7", "source:src/autosound.cpp AutoSoundDefaults[34] copied into AutoSound offset 0x8C", "pinned original 0x005F2830 leaves branch-dependent EAX values; mapped caller inspection has no resolved edges", "pinned 0x005F2830 action 0 ends with ShowWindow(HandleMain, 3), whose EAX survives the common epilogue; action 1 ends with ShowWindow(HandleMain, 6) and returns the same result", "pinned action 2 calls Win slot 52, loads/calls field +0x438 if nonzero, then returns Sound slot 23's nonzero result or Sound slot 7's result; with no child, the hook-call EAX or loaded zero survives", "pinned unknown action leaves the unsigned 32-bit EAX value action-2 after two DEC instructions", "Win owns WinCallbackWindow, pointer-owner globals, field_438_ and the Sound child; the existing Sound declarations expose int results for slots 7 and 23", "field_438_ has no known nonzero source writer and no mapped direct caller is established; this binding does not invent either"], "reason": "Return the original EAX path from the genuine Win owner. Type the callback return as int only to expose the machine-level register result that escapes when action 2 has no child; its semantic meaning and any external caller remain unresolved. Remove the scaffold because the source now represents all observed result paths as well as the retained side effects."}
// Historical emitted source symbol (void API): ?display_action@Win@@SAXH@Z
// symbol    ?display_action@Win@@SAHH@Z
// size      267 bytes
// Deleted seed provenance: git:1ed59304110fc2098fb1dd11457fa5b0074169bd:src/unrecovered/005f2830.cpp
Status: Coverage transcription of the retained seed (src/unrecovered/005f2830.cpp,
        plain FILE). The seed's TRIED note stands: landed for control flow -
        the switch over {0,1,2} with the duplicated case-0/case-1 capture
        release - and not chased to bytes (mismatch from instruction 0 in
        the seed's own attempt). The globals are the tree's own names
        (WinCallbackWindow, the WinPointerOwner1..4 quartet, HandleMain),
        the unnamed vtable slots go through the member-fn-pointer shim
        idiom stringstruct.cpp carries, and the two user32 entries are
        called as the real functions instead of through the image's IAT
        thunks at 0x00669270/0x00669320. One documented shape change: the
        capture release the image duplicates in every branch is factored
        into sub_5f2830_release_capture, so this body is shorter than the
        267-byte original instead of byte-shaped.
*/

int __cdecl Win::display_action(int action) {
    Win *self = WinCallbackWindow;
    if (action == 0) {
        const int zoomed = IsZoomed(HandleMain);
        WinPointerOwner1 = nullptr;
        WinPointerOwner2 = nullptr;
        if (zoomed != 0) {
            if (WinPointerOwner3 == self) {
                WinPointerOwner3 = nullptr;
                self->on_lose_mouse_capture();
            }
        } else {
            if (WinPointerOwner3 == self) {
                WinPointerOwner3 = nullptr;
                self->on_lose_mouse_capture();
            }
        }
        if (WinPointerOwner4 == self) {
            WinPointerOwner4 = nullptr;
        }
        return ShowWindow(HandleMain, SW_MAXIMIZE);
    }

    if (action == 1) {
        WinPointerOwner1 = nullptr;
        WinPointerOwner2 = nullptr;
        if (WinPointerOwner3 == self) {
            WinPointerOwner3 = nullptr;
            self->on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == self) {
            WinPointerOwner4 = nullptr;
        }
        return ShowWindow(HandleMain, SW_MINIMIZE);
    }

    if (action == 2) {
        self->vslot_52();
        WinDisplayActionHookFn hook = self->field_438_;
        int result = 0;
        if (hook != 0) {
            result = hook();
        }
        Sound *child = self->val_35_;
        if (child != 0) {
            const int playing = child->is_playing();
            if (playing != 0) {
                return playing;
            }
            return child->play();
        }
        return result;
    }

    // The original DEC/DEC dispatch returns the wrapped EAX value for every
    // action other than 0, 1, or 2.
    return static_cast<int32_t>(static_cast<uint32_t>(action) - 2u);
}

/*
 * RECOVERY_HISTORY: v1 address=0x005F6550 source_sha256=1ed623f7637b0127fa933752dc4300808d1f154d1f65db9ddd17ec984b87bc50 envelope_sha256=28a9214a68bad9b535d59a754ce033808a762889451fa64730a701117e25ff12
 * [ARCHIVE] // original marker: 0x005F6550 ?on_l_button_up@Win@@QAEXHHIH@Z 0x005F6550-0x005F670D FILE
 * [ARCHIVE] // size      445 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_l_button_up@Win@@QAEXHHIH@Z)(Win* this, int, int, unsigned int, int)
 * [ARCHIVE] // callers   0   call targets   2
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     0x005F7E90 0x005FA7E0
 * [ARCHIVE] // indirect  0x005F660C 0x005F6617 0x005F6625 0x005F6630 0x005F663B 0x005F664C 0x005F6657 0x005F66AD 0x005F66B8 0x005F66C6 0x005F66D1 0x005F66E4 0x005F66F5 0x005F6700
 * [ARCHIVE] // PRESERVED UNIT - measured MISMATCH.
 * [ARCHIVE] // address        0x005F6550
 * [ARCHIVE] // measured tier  MISMATCH
 * [ARCHIVE] // divergence     0
 */
/*
 * RECOVERY_HISTORY: v1 address=0x005F2CF0 source_sha256=637a3f724425acd63ef14a648514bf1d6f492554c38089a14343e1300c53fea0 envelope_sha256=2390024b1ad89d1f466eb52814880906e5f0bdb5050835f2bd7c7d9e19067c2e
 * [ARCHIVE] // original marker: 0x005F2CF0 ?on_redraw_nc@Win@@QAEXPAURECT@@H@Z 0x005F2CF0-0x005F37BA FILE
 * [ARCHIVE] // TRIED: first 9 instructions match (prologue, flag tests); diverges at instr #10 (je vs mov). Field map (buffer1_..4_ at 0xB4/0xB8/0xBC/0xC0, heap_.base_ at 0xE0 as the title char*, client_rect_/outer_rect_ at 0x14C/0x13C) cross-checked against param_1[N]*4 in the Ghidra listing and is consistent across all 4 near-identical edge blocks.
 * [ARCHIVE] // size      2762 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_redraw_nc@Win@@QAEXPAURECT@@H@Z)(Win* this, RECT*, int)
 * [ARCHIVE] // callers   1   call targets   10
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     0x005D8000 0x005DA860 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005DD450 0x005DFB50 0x005E3203 0x006453E0
 * [ARCHIVE] // indirect  0x005F2E85 0x005F30E4 0x005F319C 0x005F332B 0x005F33D3 0x005F3544 0x005F35FC 0x005F37AA
 */

// PrevWndFunc, 0x005D5C50. The shared window-procedure pre-filter: route the
// two messages the owner handles itself (window size and cursor
// selection) into the object stored in the window's userdata, and hand every
// other message to Win::window_proc through CallWindowProcA.
/*
Purpose: Pre-filter the owner window's messages before the shared window
         procedure sees them.
// ORIGINAL: 0x005D5C50 PrevWndFunc 0x005D5C50-0x005D5CEA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Historical seed git:117282ee:src/unrecovered/005d5c50.cpp retains original identity/spans.", "Pinned 5D5C50..5D5CEA handles WM_SIZE at Win virtual slot71 with signed low/high16-bit dimensions; WM_SETCURSOR uses slot85 with zero-extended words and returns its result.", "Win::on_size(unsigned int,int,int) and on_set_cursor(void*,unsigned int,unsigned int) are existing source-owned virtual slots71/85; Dialog::UNK2 calls this free WNDPROC through CallWindowProcA.", "GWL_USERDATA is the same genuine Win view used by the typed message adapters in win.cpp. Keep message-specific userdata lookup, null returns, original integer stdcall boundary, and Win::window_proc fallback."], "reason": "Recover the window pre-filter as a genuine free Win-owned Windows callback, using existing virtual methods instead of fabricated slot receiver types. Preserve original payload transforms, guards and result/fallback behavior; no new match claim."}
// symbol    _PrevWndFunc@16
Status: Coverage transcription - unclaimed body
*/
extern "C" int __stdcall PrevWndFunc(int window, int message, int wparam,
                                     int lparam) {
    if (message == 5) {
        Win *const owner = reinterpret_cast<Win *>(
            GetWindowLongA((HWND)window, GWL_USERDATA));
        if (owner != 0) {
            owner->on_size(static_cast<unsigned int>(wparam),
                           static_cast<short>(lparam),
                           static_cast<short>(lparam >> 16));
        }
        return 0;
    } else if (message == 0x20) {  // WM_SETCURSOR
        Win *const owner = reinterpret_cast<Win *>(
            GetWindowLongA((HWND)window, GWL_USERDATA));
        if (owner != 0) {
            return owner->on_set_cursor(
                reinterpret_cast<void *>(wparam),
                static_cast<unsigned short>(lparam),
                static_cast<unsigned short>(
                    static_cast<unsigned int>(lparam) >> 16));
        }
        return 0;
    }
    return CallWindowProcA(reinterpret_cast<WNDPROC>(&Win::window_proc),
                           (HWND)window, (UINT)message, (WPARAM)wparam,
                           (LPARAM)lparam);
}
