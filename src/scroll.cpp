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
#include "sound.h"
#include "original_seam.h"
#include "scroll.h"
#include "worldwin.h"
#include "dialog.h"
#include "sounddevice.h"
#include "spritebox.h"
#include "net_class.h"
#include "basepop.h"
#include "general.h"
#include "listbox.h"

// AN OBJECT, NOT A POINTER TO A FIXED ADDRESS: the pointer form costs a
// load at every use where the image addresses the storage directly, and
// the address is terranx.exe's data, unmapped in a standalone build.
// RECOVERY_OBJECT: v1 0x009B8DD4 ?ScrollDefaultThickness@@3HA
int ScrollDefaultThickness;  // 0x009B8DD4
int ScrollNonClientInit;  // 0x009B8E24
// The constructor/close index domain requires eleven words. Their initial
// bytes are independently read from 0x697020..0x69704C; this does not claim
// a complete original allocation boundary.
// RECOVERY_OBJECT: v1 0x00697020 ?ScrollCloseStaticDefaults@@3PAIA
uint32_t ScrollCloseStaticDefaults[11] = {2, 0xFFFFFFFF, 250, 0xFFFFFFFF,
    100, 247, 247, 7, 2, 255, 248};
// RECOVERY_OBJECT: v4 0x009B8DE0 0x009B8E24 ?ScrollCloseDynamicDefaults@@3PAIA 0x006051D0 target ?ScrollCloseDynamicDefaults@@3UScrollCloseDynamicDefaultsData@@A
// The historical mapping covers the accessed default window; +0x38 remains
// an opaque word because the original constructor and close never read it.
ScrollCloseDynamicDefaultsData ScrollCloseDynamicDefaults;

// The older LEVER/TRIED notes below describe experiments against the former
// integer-array spelling. They remain historical matching evidence; this
// source now models the pointer-bearing defaults with named fields and leaves
// the unobserved +0x38 word opaque.

// The original setters populate the same twelve sprite slots in four
// triplets. These are source-owned sprite pointers, not addresses in the old
// executable. Index 14 is an untouched gap; index 16 is another process
// default that neither setter changes.
static __forceinline void set_scroll_icon_defaults(Sprite *arrows,
                                                    Sprite *filler,
                                                    int thickness) {
    ScrollCloseDynamicDefaults.sprite_up[0] = &arrows[0];
    ScrollCloseDynamicDefaults.sprite_up[1] = &arrows[1];
    ScrollCloseDynamicDefaults.sprite_up[2] = &arrows[2];
    ScrollCloseDynamicDefaults.sprite_down[0] = &arrows[3];
    ScrollCloseDynamicDefaults.sprite_down[1] = &arrows[4];
    ScrollCloseDynamicDefaults.sprite_down[2] = &arrows[5];
    ScrollCloseDynamicDefaults.sprite_left[0] = &arrows[6];
    ScrollCloseDynamicDefaults.sprite_left[1] = &arrows[7];
    ScrollCloseDynamicDefaults.sprite_left[2] = &arrows[8];
    ScrollCloseDynamicDefaults.sprite_right[0] = &arrows[9];
    ScrollCloseDynamicDefaults.sprite_right[1] = &arrows[10];
    ScrollCloseDynamicDefaults.sprite_right[2] = &arrows[11];
    ScrollCloseDynamicDefaults.thumb = filler;
    ScrollDefaultThickness = thickness;
}

// ORIGINAL: 0x00588340 ?scroll_normal@@YAXXZ 0x00588340-0x005883CD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00588340.cpp","source:src/sprite.cpp","source:src/scroll.cpp:ScrollCloseDynamicDefaults"],"reason":"Populate the source-owned Scroll defaults from the normal-sized sprite objects and restore the normal thickness; retain the exact historical seed as a restoration obligation."}
// LEVER: plain sequential stores of the scaffold's own fixed-address globals (`*g_dst = (int)g_src;`) - no address arithmetic here, so the `static T *const` spelling is exactly right, no `extern` rebind needed.
void __cdecl scroll_normal() {
    set_scroll_icon_defaults(g_SCROLL_BAR_ARROW_ICON_SPRITES,
                             &g_SCROLL_BAR_FILLER_ICON_SPRITES, 20);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005883D0 ?scroll_small@@YAXXZ 0x005883D0-0x0058845D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["source:src/unrecovered/005883d0.cpp","source:src/sprite.cpp","source:src/scroll.cpp:ScrollCloseDynamicDefaults"],"reason":"Populate the same source-owned Scroll defaults from the small sprite objects and restore the small thickness; no fixed executable sprite addresses remain in this body."}
void __cdecl scroll_small() {
    set_scroll_icon_defaults(g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES,
                             &g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE, 12);
}

namespace {




inline LONG long_from_bits(uint32_t bits) {
    LONG value;
    static_assert(sizeof(value) == sizeof(bits), "Scroll geometry requires 32-bit LONG");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// THE FOUR SPRITE SETTERS' SHARED BODY, BYTE_EXACT in all four (0x00605BE0,
// 0x00605C30, 0x00605C80, 0x00605CD0). Two facts carry the match, and they
// only work together:
//   * THE TRIPLET ALIAS IS NOT `volatile`. A volatile store is a scheduling
//     barrier, so VC6 loads each argument immediately before its own store
//     (load/store/load/store) and the image's three loads up front
//     (`mov eax,[esp+4] / mov edx,[esp+8] / mov esi,[esp+0x14]`) never
//     appear. Plain stores let those three hoist above them - and the image's
//     register assignment follows: the three arguments take eax/edx/esi and
//     the dib pair takes edi/ebx, instead of this tree's exact reverse.
//   * THE dib READS ARE NAMED MEMBERS read AFTER the three stores. As raw
//     `read_volatile_bits(object, 0x4C4/0x4C8)` they are loads VC6 hoists
//     ahead of the stores, which moves the `neg`/`cmp` with them; as
//     `dib_.bmiHeader.biWidth`/`biHeight` they are loads from `this` itself,
//     which VC6 will not hoist above stores to `this`, so the compare stays
//     where the image has it - after the three stores, before the branch.
// Measured 2026-09-03: 5/21 (volatile alias, raw dib reads) -> 8/21 (named
// dib reads, volatile alias kept) -> 21/21 with the volatile dropped.
__forceinline void set_sprite_triplet(Scroll *object, Sprite *primary[3],
                        FlatButton *button, bool horizontal, Sprite *sprite1,
                        Sprite *sprite2, Sprite *sprite3) {
    primary[0] = sprite1;
    primary[1] = sprite2;
    primary[2] = sprite3;

    const LONG height = -object->dib_.bmiHeader.biHeight;
    const LONG width = object->dib_.bmiHeader.biWidth;
    const bool update_button = horizontal ? width > height : width < height;
    if (update_button) {
        button->config[12] = sprite1;
        button->config[13] = sprite2;
        button->config[14] = sprite3;
    }
}

uint32_t arithmetic_shift_right_one(uint32_t value) {
    return (value >> 1U) | (value & 0x80000000U);
}

uint32_t signed_min(uint32_t left, uint32_t right) {
    return long_from_bits(left) < long_from_bits(right) ? left : right;
}

}  // namespace


/*
Purpose: Construct a Scroll: run the GraphicWin base construction, install
         the Scroll virtual tables, and load the same fixed/dynamic
         process-default fields that Scroll::close() resets, in the same
         order. The two FlatButton members construct through the compiler's
         own base-then-member sequencing rather than an explicit placement
         call.
// ORIGINAL: 0x006051D0 ??0Scroll@@QAE@XZ 0x006051D0-0x00605367;0x00662E30-0x00662E50
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Scroll constructor 0x6051D0 calls GraphicWin construction at 0x6051EE before its two FlatButton member constructors at 0x6051FF and 0x60520F.", "The current GraphicWin is a real direct base with an implicit constructor; C++ runs that base before Scroll's FlatButton members, so removing empty ScrollGraphicWin preserves the observed order without adding a type, storage, or manual call.", "The existing sizeof(Scroll) == 0x214C assertion remains the source layout guard. Historical LEVER/TRIED notes about the former workaround remain as provenance, not current hierarchy claims."], "reason": "Express Scroll's original base-before-member lifetime through its real GraphicWin base; the constructor-only intermediary is unnecessary now that GraphicWin has a genuine C++ constructor."}
// TRIED: hoisting the four source runs of the 0xA7C/0xA88/0xA94/0xAA0 loop
//   into `const uint32_t *const d5 = dynamic + 5;` and friends, to make
//   the induction variable start at zero the way the image's `xor eax,
//   eax` does. It goes the wrong way: 81 of 83 down to 75. Indexed as
//   `dynamic[5 + index]` VC6 folds the 5 into the counter and opens with
//   `mov eax, 0x14`, which is one of the two remaining divergences, but
//   hoisting costs six more. The other divergence is the `[0x697048]`
//   load at 0x006052EF reading `fixed[10]`.
// TRIED (2026-08-28): the `uint32_t *dest = object + 0xA88 / 4;` cursor
//   form (dest[-3]/dest[0]/dest[3]/dest[6], ++dest per iteration) - 81 of
//   83, unchanged. The induction wall is not about which spelling names
//   the four windows.
// LEVER: base-ctor ordering  the image runs GraphicWin::construct() BEFORE the
//   two FlatButton members, and a call in the constructor BODY can never get
//   there - member ctors always run first. Moving it into a base subobject
//   (`struct ScrollGraphicWin : GraphicWin`, scroll.h) uses C++'s own
//   base-before-members rule to put it where the image has it. No raw storage,
//   no placement-new, layout unchanged. 21 -> 22 of 83 on its own, and it is
//   what UNBLOCKS the two levers below - they are unreachable until the order
//   is right. This RETRACTS the "raw bytes and placement-new" conclusion the
//   TRIED below reached; that note read the divergence correctly and then
//   picked the wrong of two ways to fix it.
// LEVER: vtable immediates
//   the definition down in this file is invisible at the constructor, so VC6
//   emits a load-then-store where the image has one `mov [esi], 0x669d58`.
//   Defining the value IN scroll.h folds it. 22 -> 77 of 83 - by far the
//   largest single move, and it was hidden behind the ordering defect.
// LEVER: store order  0xA30 is written between 0xA2C and 0xA3C in the image;
//   this body had it three lines later. 77 -> 81 of 83.
// TRIED: the last 2 instructions are the four-store loop's INDUCTION
//   VARIABLE, not its body. The image keeps eax a plain scaled index and folds
//   each table base into the displacement (`xor eax,eax`;
//   `mov edx,[eax+0x9b8de8]`); this tree emits `mov eax,0x14` and
//   `mov edx,[eax-0xc]`, a strength-reduced moving pointer. Measured 2026-08-22:
//   `int index` for the signed `jl` - no change, still 81. Declaring the two
//   tables as real ARRAYS so each subscript folds its own base - WORSE, 76,
//   and subscripting the array by name inside the loop worse again at 75. The
//   `uint32_t *const` locals reading `&Scroll...Defaults` are the best of the
//   four spellings measured.
// WALL, MEASURED TO THE INSTRUCTION (2026-09-03): the whole body is now ONE
//   instruction from BYTE_EXACT - `mov eax, 0x14` where the image has
//   `xor eax, eax` (listing_diff reports a single differing run), i.e. the
//   image's counter is a raw BYTE offset that stays in the register while
//   the table base sits in each load's displacement, and this tree's counter
//   is the base-and-offset folded together into the immediate. Four more
//   loop spellings were compiled OUTSIDE the tree (cl 12.00.8168, /O2 /Gy
//   /GR- /GX, a standalone probe with the same table-and-cursor shape) and
//   all four emitted this address-IV form byte for byte: destination
//   subscripted as well as the source; the source as the ARRAY NAME with a
//   destination cursor; both subscripted by name; the source reads spelled
//   `*(dynamic + 2 + index)`. So the loop's own spelling does not choose the
//   induction form. The two spellings that DO produce a raw byte counter
//   both cost more than they buy: `for (int offset = 0; offset < 12;
//   offset += 4)` with `dynamic[2 + offset / 4]` emits `xor esi,esi` /
//   `add esi,4` / `cmp esi,12` as wanted, but then pays the full signed
//   division fixup (`cdq` / `and edx,3` / `add eax,edx` / `sar eax,2`) per
//   read, and a `const char *` view of the table to make the byte offsets
//   direct needs a pointer pun. Left at 81 of 83.
// TRIED (2026-09-03) the technology.cpp has_tech/tech_avail lever on this
//   loop, since this is the same "fold the array base into the displacement"
//   shape: declared BOTH tables as real arrays at their image extents
//   (`extern uint32_t ScrollCloseDynamicDefaults[17]`, read as
//   `ScrollCloseDynamicDefaults[2 + index]`, no `uint32_t *const` local at
//   all) - 76 of 83, WORSE than the pointer locals' 81. The base lands in
//   the COUNTER's initial immediate either way (`mov eax, <base>+0x14` /
//   `add eax,4` / `mov edx,[eax-0xc]`, imm8 displacements), never out in
//   the displacements as the image has it (`xor eax,eax` /
//   `mov edx,[eax + <base>+8]`). Reverted; the image's loop is not
//   strength-reduced and every spelling here that the compiler can see as a
//   subscript gets reduced. That is the second and last untried shape this
//   wall had.
// TRIED: SEH frame divergence at instruction 0, same family as
//   FlatButton::FlatButton() (flatbutton.cpp) and PullDown::PullDown()
//   (pulldown.cpp) - a REAL derived-class constructor calling a base's
//   `construct()` method under `/GX` gets VC6's unwind scaffolding. Unlike
//   those two, the IMAGE here also carries an SEH prologue of its own
//   (`push -1 / push 0x662e46 / mov eax,fs:[0] / push eax / mov
//   fs:[0],esp`), so this is not the same "we have one where they have
//   none" defect - not re-derived per policy either way.
// TRIED: named FlatButton members over placement-new at a fixed offset.
//   The image calls GraphicWin::construct() BEFORE the two FlatButton
//   constructors (0x6051ee then 0x6051ff/0x60520f); ordinary C++ member
//   construction always runs the member ctors before the constructor BODY
//   starts, so with flat_button_left_/flat_button_right_ declared as real
//   members (as scroll.h already does, and as close()/destroy() already
//   rely on) the two FlatButton() calls compile BEFORE this body's
//   GraphicWin::construct() call - call TARGETS match, call ORDER does not.
//   Reordering would mean storing them as raw bytes and placement-new'ing
//   at the right point in the body instead, which moves scroll.h's layout
//   out from under close()/destroy()'s existing `flat_button_left_.close()`
//   / `.destroy()` calls; left with the named members.
// symbol    ??0Scroll@@QAE@XZ
// size      439 bytes
// prototype Scroll* (__thiscall ??0Scroll@@QAE@XZ)(Scroll* this)
// callers   13   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4CF0 0x00607CF0
Return Value: Instance pointer in EAX
Status: Complete
*/
Scroll::Scroll() {
    // Both vtable stores are the compiler's now that Buffer is a real base of
    // GraphicWin; it puts them where the image does.
    uint32_t *const fixed = ScrollCloseStaticDefaults;
    ScrollCloseDynamicDefaultsData *const dynamic = &ScrollCloseDynamicDefaults;

    field_A14_ = dynamic->flags;
    border_color_ = static_cast<int>(fixed[3]);
    range_reversed_ = 0;
    range_minimum_ = static_cast<int>(dynamic->position);
    range_maximum_ = static_cast<int>(fixed[4]);
    position_ = static_cast<int>(dynamic->position);
    field_A30_ = fixed[2];
    field_A3C_ = 0xFFFFFFFFU;
    field_A38_ = 0;
    field_A34_ = fixed[1];
    paging_ = static_cast<int>(fixed[0]);
    field_A44_ = 0;
    thumb_rect_.left = 0;
    thumb_rect_.top = 0;
    thumb_rect_.right = 0;
    thumb_rect_.bottom = 0;
    field_A48_ = fixed[5];
    color_ = fixed[6];
    field_A64_ = static_cast<int>(fixed[7]);
    bevel_thickness_ = fixed[8];
    bevel_upper_ = fixed[9];
    bevel_lower_ = fixed[10];

    for (int index = 0; index < 3; ++index) {
        sprite_left_[index] = dynamic->sprite_left[index];
        sprite_up_[index] = dynamic->sprite_up[index];
        sprite_right_[index] = dynamic->sprite_right[index];
        sprite_down_[index] = dynamic->sprite_down[index];
    }
    thumb_sprite_ = dynamic->thumb;
    field_A78_ = dynamic->background;
    field_2144_ = 0;
    field_2148_ = 0;
}

/*
Purpose: Reset Scroll-owned state from the process defaults, close the two
         embedded FlatButtons through their virtual close slots, then close
         the source-owned GraphicWin base.
// ORIGINAL: 0x00605370 ?close@Scroll@@QAEXXZ 0x00605370-0x006054CE
// LEVER: `fixed`/`dynamic` were reinterpret_cast<...>(ScrollCloseStaticDefaults)
//   - casting the VALUE stored in the global to a pointer, an indirect load the
//   image never does. `&ScrollCloseStaticDefaults`/`&ScrollCloseDynamicDefaults`
//   (the object IS the table base) matches the image's direct fixed-address
//   loads and took this from 0/71 to 42/71.
// CORRECTED (2026-08-22): the note this replaces claimed the loop below
//   read the wrong windows of `dynamic[]` and called it BLOCKED. Read
//   directly off 0x605453-0x60547F: `mov edi,[eax+0x9b8de8]` before `eax`'s
//   first increment is dword offset 8 from the 0x9b8de0 table base, i.e.
//   `dynamic[2+index]`; the three reads that follow use `eax` AFTER that
//   same increment, at offsets 0x10/0x1c/0x28, i.e. `dynamic[5+index]`,
//   `dynamic[8+index]`, `dynamic[11+index]` - exactly the four windows the
//   loop below already writes. There is no data-mapping defect here; see the
//   TRIED below and the constructor's own (0x006051D0) matching note for
//   what the remaining gap actually is.
// TRIED (2026-09-03) the technology.cpp array-subscript lever here too
//   (same loop, same induction residual as the ctor at 0x006051D0): both
//   tables declared as arrays at their image extents and read by name, no
//   `uint32_t *const` local - the whole body then measures 60 of 71 against
//   the pointer-local spelling's 65, so reverted. Full detail on the ctor's
//   own marker; the two bodies share the wall and the refutation.
// TRIED (2026-09-03) reading the dynamic table through a SECOND, `volatile`
//   alias (`volatile uint32_t *const src`) used by the whole body, on the
//   theory that a volatile load would keep VC6 from strength-reducing the
//   loop's address into a moving pointer and leave the image's
//   `xor eax,eax` / `[eax + <base>]` form: 51 of 71, far worse - the
//   volatile loads cost the straight-line copies more than the loop gains.
//   Refuted. With the array-subscript refutation on the ctor's marker this
//   exhausts the spellings this wall had.
// TRIED: the remaining 6-instruction gap is two SEPARATE, already-typed
//   defects, neither of which yielded to a new spelling (try_spellings,
//   2026-08-22). (1) The loop's induction variable: the image keeps a plain
//   scaled byte offset in `eax` (`xor eax,eax` before the loop); this body's
//   compiled `eax` starts at the strength-reduced `0x14` instead - the exact
//   residual the constructor's own TRIED above documents and could not
//   move either (tried `int index` here too, no change). (2) The SECOND
//   FlatButton dispatch (`flat_button_right_`) schedules its vtable load
//   into `eax` and calls `[eax+0x168]` where the image loads `edx` and calls
//   `[edx+0x168]` with the OTHER field read (`[esi+0x15f8]`) interleaved
//   between them - the same "second call schedules differently" residual
//   already documented and abandoned in `set_button_color` and
//   `set_bevel_thickness` below. Naming the vtable pointer in a local before
//   the second call does not change it either.
// LEVER: dropped the OPENSMACX_NOINLINE call_noarg_virtual helper and
//   expanded both FlatButton close dispatches inline via
//   read_volatile_bits/original_slot<func_noarg_virtual> at slot 0x168,
//   removing both E8 calls to the helper.
// LEVER: `object`/`fixed`/`dynamic` were `volatile uint32_t *` - every one of
//   these is a straight-line copy (no aliasing hazard to guard, nothing
//   reread), so `volatile` bought nothing but forced extra register loads.
//   Plain (non-volatile) pointers, same raw-offset shape, took this from
//   47/71 to 65/71 - and the compiled instruction count now matches the
//   image's 71 exactly. The remaining 6-instruction gap is the induction
//   variable and second-dispatch scheduling the TRIED above names;
//   unaffected by this change.
// symbol    ?close@Scroll@@QAEIXZ
// size      350 bytes
// prototype void (__thiscall ?close@Scroll@@QAEXXZ)(Scroll* this)
// callers   136   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40
// indirect  0x006054AC 0x006054BE
// notes     Runtime redirect installed by DllMain after byte-signature validation; retains two embedded FlatButton virtual-close dependencies and delegates to source-owned GraphicWin close
Return Value: GraphicWin::close return residue
Status: Complete; embedded FlatButton close redirects are source-owned
*/
uint32_t Scroll::close() {
    uint32_t *const fixed = ScrollCloseStaticDefaults;
    ScrollCloseDynamicDefaultsData *const dynamic = &ScrollCloseDynamicDefaults;

    field_A14_ = dynamic->flags;
    border_color_ = static_cast<int>(fixed[3]);
    range_reversed_ = 0;
    range_minimum_ = static_cast<int>(dynamic->position);
    range_maximum_ = static_cast<int>(fixed[4]);
    position_ = static_cast<int>(dynamic->position);
    field_A3C_ = 0xFFFFFFFFU;
    field_A38_ = 0;
    field_A30_ = fixed[2];
    field_A34_ = fixed[1];
    paging_ = static_cast<int>(fixed[0]);
    field_A44_ = 0;
    thumb_rect_.left = 0;
    thumb_rect_.top = 0;
    thumb_rect_.right = 0;
    thumb_rect_.bottom = 0;
    field_A48_ = fixed[5];
    color_ = fixed[6];
    field_A64_ = static_cast<int>(fixed[7]);
    bevel_thickness_ = fixed[8];
    bevel_upper_ = fixed[9];
    bevel_lower_ = fixed[10];

    for (int index = 0; index < 3; ++index) {
        sprite_left_[index] = dynamic->sprite_left[index];
        sprite_up_[index] = dynamic->sprite_up[index];
        sprite_right_[index] = dynamic->sprite_right[index];
        sprite_down_[index] = dynamic->sprite_down[index];
    }
    thumb_sprite_ = dynamic->thumb;
    field_A78_ = dynamic->background;
    field_2144_ = 0;
    field_2148_ = 0;

    flat_button_left_.close();
    flat_button_right_.close();
    return GraphicWin::close();
}

/*
Purpose: Initialize the scrollbar track and its two optional embedded arrow
         buttons, choosing horizontal or vertical sprites from the layout.
// ORIGINAL: 0x006054D0 ?init@Scroll@@QAEHHHHHPAUWin@@HH@Z 0x006054D0-0x0060583F FILE
// RECOVERY_BINDING: v1 {"evidence":["source:src/unrecovered/006054d0.cpp","original:0x00605701-0x00605788","source:src/scroll.h:Scroll","source:src/flatbutton.h:FlatButton"],"reason":"Home the primary initializer on Scroll's real hierarchy and embedded buttons. The executable's branch/register data corrects the seed's zero-size horizontal right button, while generic inherited settings are encapsulated by FlatButton."}
// symbol    ?init@Scroll@@QAEHHHHHPAVWin@@HH@Z
// Native60569A..6056A6 and6057A1..6057AD place each arrow triplet in
// FlatButton+AD4/+AD8/+ADC. The shared setter now writes that redraw array,
// not the unrelated AutoSound+28/+2C/+30 words used by sound dispatch.
// The retired seed's near match was a spelling experiment, not a structural
// model; this body remains unclaimed until freshly restored.
*/
int Scroll::init(int x, int y, int width, int height, Win *parent,
                 int setting, int options) {
    if (parent == nullptr || width == 0 || height == 0) {
        return 3;
    }
    if (HandleMain == nullptr) {
        return 7;
    }

    close();
    field_A14_ |= static_cast<uint32_t>(options);
    field_A18_ = static_cast<uint32_t>(setting);
    range_minimum_ = 0;

    const bool horizontal = width > height;
    bar_thickness_ = horizontal ? height : width;
    make_rect(&thumb_rect_, 0, 0, bar_thickness_, bar_thickness_);
    if (border_color_ != -1) {
        expand_rect(&thumb_rect_, -1, -1);
    }
    range_maximum_ = horizontal ? width - 2 * height : height - 2 * width;

    int flags = (field_A14_ & 1U) != 0U ? 0x80000 : 0;
    flags |= ScrollNonClientInit != 0 ? 0x9020 : 0x1020;
    int result = GraphicWin::init(x, y, width, height, nullptr, flags,
                                  parent, nullptr, nullptr);
    if (result != 0) {
        return result;
    }

    if ((field_A14_ & 2U) == 0U) {
        const int inset = border_color_ != -1 ? 1 : 0;
        const int button_size = bar_thickness_ - 2 * inset;
        result = flat_button_left_.init(nullptr, 1, inset, inset,
                                        button_size, button_size, this, 0);
        if (result != 0) {
            return result;
        }

        if (horizontal) {
            flat_button_left_.set_scroll_icons(sprite_left_[0], sprite_left_[1],
                                                sprite_left_[2]);
        } else {
            flat_button_left_.set_scroll_icons(sprite_up_[0], sprite_up_[1],
                                                sprite_up_[2]);
        }
        flat_button_left_.color_ = color_;
        flat_button_left_.bevel_thickness_ = bevel_thickness_;
        flat_button_left_.bevel_upper_ = bevel_upper_;
        flat_button_left_.bevel_lower_ = bevel_lower_;
        flat_button_left_.field_AA0_ = 0;
        flat_button_left_.field_A44_ = 0x32;
        flat_button_left_.redraw();

        const int second_x = horizontal
                                 ? dib_.bmiHeader.biWidth - bar_thickness_ + inset
                                 : inset;
        const int second_y = horizontal
                                 ? inset
                                 : -dib_.bmiHeader.biHeight - bar_thickness_ + inset;
        result = flat_button_right_.init(nullptr, 2, second_x, second_y,
                                         button_size, button_size, this, 0);
        if (result != 0) {
            return result;
        }

        if (horizontal) {
            flat_button_right_.set_scroll_icons(sprite_right_[0], sprite_right_[1],
                                                 sprite_right_[2]);
        } else {
            flat_button_right_.set_scroll_icons(sprite_down_[0], sprite_down_[1],
                                                 sprite_down_[2]);
        }
        flat_button_right_.color_ = color_;
        flat_button_right_.bevel_thickness_ = bevel_thickness_;
        flat_button_right_.bevel_upper_ = bevel_upper_;
        flat_button_right_.bevel_lower_ = bevel_lower_;
        flat_button_right_.field_AA0_ = 0;
        flat_button_right_.field_A44_ = 0x32;
        flat_button_right_.redraw();
    }

    position_ = range_minimum_;
    field_A3C_ = 0xFFFFFFFFU;
    show(0);
    ScrollNonClientInit = 0;
    return 0;
}

/*
Purpose: Initialize a scrollbar from a rectangle.
// ORIGINAL: 0x00605840 ?init@Scroll@@QAEHPAURECT@@PAUWin@@HH@Z 0x00605840-0x00605885 BYTE_EXACT
// symbol    ?init@Scroll@@QAEHPAUtagRECT@@PAVWin@@HH@Z
// size      69 bytes
// prototype int (__thiscall ?init@Scroll@@QAEHPAURECT@@PAUWin@@HH@Z)(Scroll* this, RECT*, Win*, int, int)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init(RECT *rect, Win *parent, int setting, int options) {
    if (!rect || !parent) {
        return 3;
    }

    // PLAIN ARITHMETIC. `long_from_bits` is a memcpy round trip VC6 will not
    // inline, and there are four of them here - four calls the image does not
    // make. The wrap it was protecting against is what the image's `sub` does
    // anyway.
    return init(rect->left, rect->top, rect->right - rect->left,
                rect->bottom - rect->top, parent, setting, options);
}

/*
Purpose: Initialize a vertical scrollbar using the process-default thickness.
// ORIGINAL: 0x00605890 ?init_vert@Scroll@@QAEHHHHPAUWin@@H@Z 0x00605890-0x006058CB BYTE_EXACT
// symbol    ?init_vert@Scroll@@QAEHHHHPAVWin@@H@Z
// size      59 bytes
// prototype int (__thiscall ?init_vert@Scroll@@QAEHHHHPAUWin@@H@Z)(Scroll* this, int, int, int, Win*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init_vert(
        int x, int y, int length, Win *parent, int setting) {
    if (!parent || length == 0) {
        return 3;
    }
    return init(x, y, ScrollDefaultThickness, length, parent, setting, 0);
}

/*
Purpose: Initialize a horizontal scrollbar using the process-default thickness.
// ORIGINAL: 0x006058D0 ?init_horz@Scroll@@QAEHHHHPAUWin@@H@Z 0x006058D0-0x0060590C BYTE_EXACT
// symbol    ?init_horz@Scroll@@QAEHHHHPAVWin@@H@Z
// size      60 bytes
// prototype int (__thiscall ?init_horz@Scroll@@QAEHHHHPAUWin@@H@Z)(Scroll* this, int, int, int, Win*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init_horz(
        int x, int y, int length, Win *parent, int setting) {
    if (!parent || length == 0) {
        return 3;
    }
    return init(x, y, length, ScrollDefaultThickness, parent, setting, 0);
}

/*
Purpose: Initialize a vertical nonclient scrollbar.
// ORIGINAL: 0x00605910 ?init_vert_nc@Scroll@@QAEHHHHPAUWin@@H@Z 0x00605910-0x00605955 BYTE_EXACT
// LEVER: ScrollNonClientInit is the object itself, not a pointer to it; drop the reinterpret_cast indirection
// symbol    ?init_vert_nc@Scroll@@QAEHHHHPAVWin@@H@Z
// size      69 bytes
// prototype int (__thiscall ?init_vert_nc@Scroll@@QAEHHHHPAUWin@@H@Z)(Scroll* this, int, int, int, Win*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init_vert_nc(
        int x, int y, int length, Win *parent, int setting) {
    ScrollNonClientInit = 1;
    if (!parent || length == 0) {
        return 3;
    }
    return init(x, y, ScrollDefaultThickness, length, parent, setting, 0);
}

/*
Purpose: Initialize a horizontal nonclient scrollbar.
// ORIGINAL: 0x00605960 ?init_horz_nc@Scroll@@QAEHHHHPAUWin@@H@Z 0x00605960-0x006059A6 BYTE_EXACT
// LEVER: ScrollNonClientInit is the object itself, not a pointer to it; drop the reinterpret_cast indirection
// symbol    ?init_horz_nc@Scroll@@QAEHHHHPAVWin@@H@Z
// size      70 bytes
// prototype int (__thiscall ?init_horz_nc@Scroll@@QAEHHHHPAUWin@@H@Z)(Scroll* this, int, int, int, Win*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006054D0
// notes     Runtime redirect installed by DllMain after byte-signature validation; calls the temporary original primary initializer
Status: Complete
*/
int Scroll::init_horz_nc(
        int x, int y, int length, Win *parent, int setting) {
    ScrollNonClientInit = 1;
    if (!parent || length == 0) {
        return 3;
    }
    return init(x, y, length, ScrollDefaultThickness, parent, setting, 0);
}

/*
Purpose: Set the signed scrollbar range and redraw it at the lower endpoint.
// ORIGINAL: 0x006059B0 ?set_range@Scroll@@QAEXHH@Z 0x006059B0-0x00605A0D BYTE_EXACT
// LEVER: replaced the cached-vtable-bits/redraw_from_vtable(__asm helper)
//   tail with the inline read_volatile_bits/original_slot<func_noarg_virtual>
//   dispatch 0x00605A10 uses - 15/24 to 17/24 agreeing (best flags /Ob0).
// LEVER (2026-09-03): `range_swap_call`, a file-local forwarder over
//   general.h's MEASURED inline `swap(int *, int *)`. The image CALLS
//   0x00628A50 here and the open-coded helper compiled the XOR swap in
//   place, which also rescheduled the `range_minimum_`/`range_maximum_`/
//   `position_` stores into an XOR-chain the image does not have. With the
//   call the body is the image's own 24 and the tail dispatch needed no
//   further work. 24 of 24, BYTE_EXACT. The "swap defect above" note is
//   superseded - this was that defect.
//   The "swap defect" line above is superseded by the 2026-09-03 LEVER.
// symbol    ?set_range@Scroll@@QAEXHH@Z
// size      93 bytes
// prototype void (__thiscall ?set_range@Scroll@@QAEXHH@Z)(Scroll* this, int, int)
// callers   12   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628A50
// indirect  0x00605A03
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
// Historical compiler-workaround explanation, retained after correcting the
// API: each original body ends after redraw without defining EAX, so a
// non-void C++ signature required suppressing C4716. The source API is void.

// Non-inline forwarder over general.h's MEASURED inline `swap(int *, int *)`.
// The image CALLS 0x00628A50 from set_range (0x006059B0); the in-class
// expansion compiled the XOR swap in place and moved the three field stores
// after it. File-local, so no other swap site can see it - the rest of the
// tree's 30-odd callers genuinely inline the helper.
static void __cdecl range_swap_call(int *var1, int *var2) {
    swap(var1, var2);
}

void Scroll::set_range(int minimum, int maximum) {
    if (minimum < maximum) {
        range_reversed_ = 0U;
    } else {
        range_reversed_ = 1U;
        range_swap_call(&minimum, &maximum);
    }
    range_minimum_ = minimum;
    position_ = minimum;
    range_maximum_ = maximum;
    // Scroll and FlatButton use GraphicWin::redraw at Win slot 62. Preserve
    // the original virtual dispatch rather than qualifying the base method.
    redraw();
}

/*
Purpose: Set the color shared by the scrollbar and both end buttons.
// ORIGINAL: 0x00605A10 ?set_button_color@Scroll@@QAEXH@Z 0x00605A10-0x00605A4D BYTE_EXACT
// LEVER: vtable_method for the SECOND dispatch, `original_slot` for the first.
//   The two are not interchangeable here: the first dispatch already matches
//   the image's single `call dword ptr [reg+0xf8]` through
//   read_volatile_bits/original_slot, while the second scheduled as
//   `mov eax,[obj]; mov eax,[eax+0xf8]; call eax` - two instructions the image
//   does not have. `vtable_method<Fn>(&obj, 0xF8)` emits the single call.
//   MISMATCH 7/14 -> 12 of 14, granted SEMANTIC. Identical to what
//   set_bevel_thickness, set_bevel_upper and set_bevel_lower each needed, so
//   this is the fourth site of one shape rather than four coincidences.
// LEVER: read_volatile_bits/original_slot inline (not the redraw_from_vtable __asm helper, and the shared helpers marked `inline`) gets the vtable dispatch inlined in place, matching the image's direct load+call; a volatile store on the first field write fixes its scheduling relative to the following `lea`.
// symbol    ?set_button_color@Scroll@@QAEXH@Z
// size      61 bytes
// prototype void (__thiscall ?set_button_color@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605A31 0x00605A43
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_button_color(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    color_ = value;
    flat_button_left_.color_ = value;
    flat_button_right_.color_ = value;
    flat_button_left_.redraw();
    flat_button_right_.redraw();
}

/*
Purpose: Set the bevel thickness shared by the scrollbar and both end buttons.
// ORIGINAL: 0x00605A50 ?set_bevel_thickness@Scroll@@QAEXH@Z 0x00605A50-0x00605A8D BYTE_EXACT
// LEVER: replaced both redraw_from_vtable(__asm helper) calls with the inline read_volatile_bits/original_slot<func_noarg_virtual> dispatch 0x00605A10 already uses - 4/14 to 7/14 agreeing. First dispatch now matches the image's single `call dword ptr [reg+0xf8]`; the second still schedules as `mov eax,[obj]; mov eax,[eax+0xf8]; call eax`.
// symbol    ?set_bevel_thickness@Scroll@@QAEXH@Z
// size      61 bytes
// prototype void (__thiscall ?set_bevel_thickness@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605A71 0x00605A83
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_bevel_thickness(int thickness) {
    const uint32_t value = static_cast<uint32_t>(thickness);
    bevel_thickness_ = value;
    flat_button_left_.bevel_thickness_ = value;
    flat_button_right_.bevel_thickness_ = value;
    flat_button_left_.redraw();
    flat_button_right_.redraw();
}

/*
Purpose: Set the upper bevel color shared by the scrollbar and both end buttons.
// ORIGINAL: 0x00605A90 ?set_bevel_upper@Scroll@@QAEXH@Z 0x00605A90-0x00605ACD BYTE_EXACT
// LEVER: same substitution as 0x00605A50 - 4/14 to 7/14 agreeing, same second-call residual.
// symbol    ?set_bevel_upper@Scroll@@QAEXH@Z
// size      61 bytes
// prototype void (__thiscall ?set_bevel_upper@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605AB1 0x00605AC3
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_bevel_upper(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    bevel_upper_ = value;
    flat_button_left_.bevel_upper_ = value;
    flat_button_right_.bevel_upper_ = value;
    flat_button_left_.redraw();
    flat_button_right_.redraw();
}

/*
Purpose: Set the lower bevel color shared by the scrollbar and both end buttons.
// ORIGINAL: 0x00605AD0 ?set_bevel_lower@Scroll@@QAEXH@Z 0x00605AD0-0x00605B0D BYTE_EXACT
// LEVER: same substitution as 0x00605A50 - 4/14 to 7/14 agreeing, same second-call residual.
// symbol    ?set_bevel_lower@Scroll@@QAEXH@Z
// size      61 bytes
// prototype void (__thiscall ?set_bevel_lower@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605AF1 0x00605B03
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_bevel_lower(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    bevel_lower_ = value;
    flat_button_left_.bevel_lower_ = value;
    flat_button_right_.bevel_lower_ = value;
    flat_button_left_.redraw();
    flat_button_right_.redraw();
}

/*
Purpose: Resize the scrollbar track and its right/down arrow after the parent
         changes size. The smaller axis sets the track thickness.
// ORIGINAL: 0x00606860 ?on_resize@Scroll@@QAEXHH@Z 0x00606860-0x0060699B FILE
// RECOVERY_BINDING: v1 {"evidence":["source:src/recovered/units/00606860.cpp","source:src/scroll.h:Scroll","source:src/win.h:Win::on_resize"],"reason":"Home the resize handler on the actual Scroll layout and dispatch the embedded FlatButton through its inherited virtual resize, without raw self offsets or an invented vtable."}
// symbol    ?on_resize@Scroll@@UAEXHH@Z
// The catalogue spells this QAE, but the real Win base declares slot 13;
// Scroll's emitted override is UAE, as independently checked with VC6.
*/
void Scroll::on_resize(int width, int height) {
    const int thickness = width > height ? height : width;
    bar_thickness_ = thickness;
    thumb_rect_.left = 0;
    thumb_rect_.top = 0;
    thumb_rect_.right = thickness;
    thumb_rect_.bottom = thickness;

    if (border_color_ != -1) {
        thumb_rect_.left = 1;
        --thumb_rect_.right;
        ++thumb_rect_.top;
        --thumb_rect_.bottom;
    }

    if (border_color_ != -1) {
        const int button_size = bar_thickness_ - 2;
        flat_button_right_.resize(button_size, button_size, 0);
        if (width <= height) {
            flat_button_right_.move(width - bar_thickness_ + 1,
                                    height - bar_thickness_ + 1);
        } else {
            flat_button_right_.move(1, 1);
        }
        return;
    }

    flat_button_right_.resize(bar_thickness_, bar_thickness_, 0);
    if (width <= height) {
        flat_button_right_.move(width - bar_thickness_, height - bar_thickness_);
    } else {
        flat_button_right_.move(0, 0);
    }
}

/*
Purpose: Paint the arrow buttons, track, optional pattern and the thumb's
         sprite and bevel on this window's inherited Buffer.
// ORIGINAL: 0x006066F0 ?on_redraw@Scroll@@QAEXXZ 0x006066F0-0x00606854 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/006066f0.cpp", "source:src/scroll.h:Scroll", "source:src/graphicwin.h:GraphicWin", "source:src/buffer.h:Buffer"], "reason": "Home the redraw path on the real GraphicWin/Buffer hierarchy, embedded FlatButtons and source-owned thumb sprite instead of seed vtable and raw-offset stand-ins. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_redraw@Scroll@@UAEXXZ
// Likewise, this is Win's slot 12 override despite the catalogue's QAE name.
// The separately claimed seed remains a restoration obligation while this
// structurally homed body does not yet reproduce its compiled instructions.
*/
void Scroll::on_redraw() {
    flat_button_left_.redraw();
    flat_button_right_.redraw();
    fill(field_A64_);

    if (border_color_ != -1) {
        const unsigned char color = static_cast<unsigned char>(border_color_);
        Buffer::box(&rect2_, color, color);
    }

    RECT thumb;
    compute_thumb_rect(&thumb);
    fill(&thumb, field_A64_);

    if (field_A78_ != nullptr) {
        Buffer::tile(field_A78_, 0, 0, 0, 0,
                     dib_.bmiHeader.biWidth, -dib_.bmiHeader.biHeight);
    }

    if (thumb_sprite_ != nullptr) {
        const int x = (thumb.right - thumb_sprite_->width() - thumb.left) / 2
                      + thumb.left;
        const int y = (thumb.bottom - thumb_sprite_->height() - thumb.top) / 2
                      + thumb.top;
        thumb_sprite_->draw(static_cast<Buffer *>(this),
                            thumb_sprite_->transparent_index(), x, y, 1, 1);
    }

    // Native6067F2/6067FC and60684B use signed comparisons. The stored
    // word can retain a negative set_bevel_thickness(int) bit pattern.
    for (int layer = 0; layer < long_from_bits(bevel_thickness_); ++layer) {
        Buffer::box(&thumb, bevel_upper_, bevel_lower_);
        ++thumb.left;
        --thumb.right;
        ++thumb.top;
        --thumb.bottom;
    }
}

/*
Purpose: Set the scrollbar thickness and reset its thumb rectangle.
// ORIGINAL: 0x00605B80 ?set_bar_thickness@Scroll@@QAEXH@Z 0x00605B80-0x00605BE0
// TRIED: dropping the `volatile` casts on `bar_thickness_`, `border_color_`
//   and the `thumb` RECT alias (plain assignments/reads to the named fields
//   instead): 7/22 -> 8/22 but the compiler now folds the border-color guard
//   into an `or eax, edx` the image never has and drops 4 instructions the
//   image keeps (23 -> 19 compiled) - same dead-store-elimination hazard
//   `set_border_color` below already measured and rejected. Reverted.
// LEVER (2026-08-28) the guard clause's POLARITY, not the branch: the image
//   falls through into the inset work and jumps forward to ONE shared `ret 4`
//   for the early exit, which is what a single `return` at the end of an
//   if-wrapped block compiles to. The previous spelling - early `return color`
//   then `return bottom` - put the work on the jump target and emitted `jne` +
//   `or eax,eax` + a second `ret 4` (7/22). Holding the value in one `result`
//   local assigned on both paths gives the image's `cmp eax,-1 / je <shared
//   ret>` verbatim: 7/22 -> 12/22, 0.756 -> 0.930. `result = bottom` costs
//   nothing - bottom is already in EAX, so VC6 emits no move and the two paths
//   converge on the same `ret`. Same wall set_pos documents, solved by the one
//   shape its TRIED list does not name (a single exit whose value is a local,
//   not a `goto`).
// LEVER (same pass) `bar_thickness_ = thickness` as a plain member store, not
//   through `reinterpret_cast<volatile int *>`: the volatile pushed the
//   `xor edx,edx` for the zero-init AFTER the thickness store, where the image
//   has it at instruction 1, before it. Only the STORE loses its volatile; the
//   reads of `thumb->right`/`top`/`bottom` keep it, because the image reloads
//   those three fields and that is what keeps the reset stores alive.
// TRIED: `thumb_rect_.left = thumb_rect_.left + 1` and
//   `thumb->left = thumb->left + 1`, to reproduce the image's
//   `inc edx / mov [ecx+0xa4c],edx` (it synthesises the literal 1 by
//   incrementing the zero still live in edx) - 11/22 and 10/22, both worse
//   than the plain literal's 12/22. VC6 constant-propagates the member read
//   and emits `mov dword ptr [ecx+0xa4c], 1`. Refuted; what is left is that
//   one instruction pair and the 3-byte shift it puts in the `je` target.
// TRIED (2026-09-03) three more spellings of that one pair, all 12/22 (the
//   compiled listing byte-identical to the committed body's): a named
//   `uint32_t inset = 0` local zero-ing `left`/`top` and `++inset` in the
//   branch; the same with the `++inset` moved ahead of the `right` read; and
//   `const LONG zero = 0` with `thumb->left = zero + 1`. All three fold to
//   the literal at the source level, so instruction selection never sees a
//   register-held zero to increment. Also measured the opposite spelling
//   wholesale: NON-volatile `thumb_rect_` with compound assignments
//   (`thumb_rect_.left += 1` etc., the form that makes the unconditional
//   stores live so they cannot be elided) - 0/22 at 16 compiled instructions
//   against the image's 22, because VC6 then keeps `thickness` in a register
//   and folds `right`/`bottom` to `lea eax,[edx-1]`, deleting the four
//   reloads the image performs. So the volatile IS what buys the image's
//   four memory reloads, and it is also what costs the `mov [mem],1` - the
//   image's `inc edx` needs a compiler that leaves the zero in a register
//   past the guard, and no spelling here reaches it. 12/22 is the ceiling.
// symbol    ?set_bar_thickness@Scroll@@QAEIH@Z
// size      96 bytes
// prototype void (__thiscall ?set_bar_thickness@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_bar_thickness(int thickness) {
    const uint32_t thickness_bits = static_cast<uint32_t>(thickness);
    bar_thickness_ = thickness;
    volatile RECT *const thumb = &thumb_rect_;
    thumb->left = 0;
    thumb->top = 0;
    thumb->right = long_from_bits(thickness_bits);
    thumb->bottom = long_from_bits(thickness_bits);
    uint32_t result = static_cast<uint32_t>(border_color_);
    if (result != -1) {
        const uint32_t right = static_cast<uint32_t>(thumb->right) - 1U;
        thumb->left = 1;
        const uint32_t top = static_cast<uint32_t>(thumb->top) + 1U;
        thumb->right = long_from_bits(right);
        const uint32_t bottom = static_cast<uint32_t>(thumb->bottom) - 1U;
        thumb->top = long_from_bits(top);
        thumb->bottom = long_from_bits(bottom);
        result = bottom;
    }
    return result;
}

/*
Purpose: Set the border color and reset the scrollbar thumb rectangle.
// ORIGINAL: 0x00605B10 ?set_border_color@Scroll@@QAEXH@Z 0x00605B10-0x00605B74
// LEVER: image writes defaults unconditionally then overwrites in the branch;
//   ternary/direct-assign forms let the optimizer prove the first write dead and
//   drop it. A `volatile RECT*` keeps both writes, and re-reading `border_color_`
//   (not the `color` parameter) for the branch condition matches the image, which
//   reloads from the just-stored field instead of keeping color live (1/22 -> 9/22).
// TRIED: forcing the thickness read through a volatile cast, no further gain.
// TRIED: `thumb->left = 1` re-spelled as `thumb->left + 1` (mirroring the
//   already-matching `top`/`right`/`bottom` idiom, on the theory that the
//   image's `inc edx` reuses the register the unconditional zero-init left
//   at 0 rather than materialising a fresh literal) - WORSE, 9/22 -> 7/22
//   and the compiled body grows from 21 to 23 instructions. Reverted.
// TRIED: hoisting the RECT zero-init through a named `const uint32_t zero`
//   local, on the theory that a value with no dependency on the
//   `border_color_` store would let VC6 schedule the `xor edx, edx` before
//   it, matching the image's instruction 1. No effect - still 9/22, same
//   schedule. Reverted. The `xor edx, edx` vs its later position is the
//   remaining divergence and is pure instruction scheduling, not source
//   shape.
// RETRACTED (2026-08-28) that "pure instruction scheduling" conclusion, and
//   the conclusion it shared with set_thumb_rect's TRIED. It was the STORE's
//   volatile, not the schedule: `border_color_ = color` as a plain member
//   store moves the `xor edx,edx` to instruction 1 where the image has it,
//   because the volatile cast was what ordered it after the colour store.
//   9/22 -> 12/22, 0.837 -> 0.930. The branch still re-READS `border_color_`
//   from memory (`cmp dword ptr [ecx+0xa1c],-1`), which is what this body's
//   own LEVER above established, so nothing the volatile was guarding is
//   lost - the image's one store and one load are both still there.
// TRIED (2026-08-28) `thumb_rect_.left = thumb_rect_.left + 1` for the
//   image's `inc edx / mov [ecx+0xa4c],edx` once the schedule was right:
//   11/22, WORSE than the plain literal's 12/22, same as set_bar_thickness's
//   own attempt. VC6 constant-propagates the member read back to the literal.
//   Refuted twice now, in two different schedules. The residue is that one
//   instruction pair and the 3-byte shift it puts in the `je` target.
// size      100 bytes
// prototype void (__thiscall ?set_border_color@Scroll@@QAEXH@Z)(Scroll* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_border_color(int color) {
    border_color_ = color;
    const uint32_t thickness = static_cast<uint32_t>(bar_thickness_);
    volatile RECT *const thumb = &thumb_rect_;
    thumb->left = 0;
    thumb->top = 0;
    thumb->right = long_from_bits(thickness);
    thumb->bottom = long_from_bits(thickness);
    if (border_color_ != -1) {
        const uint32_t right = static_cast<uint32_t>(thumb->right) - 1U;
        thumb->left = 1;
        const uint32_t top = static_cast<uint32_t>(thumb->top) + 1U;
        thumb->right = long_from_bits(right);
        const uint32_t bottom = static_cast<uint32_t>(thumb->bottom) - 1U;
        thumb->top = long_from_bits(top);
        thumb->bottom = long_from_bits(bottom);
    }
}

/*
Purpose: Expand a rectangle horizontally and vertically.
// ORIGINAL: 0x00606F00 sub_606f00 0x00606F00-0x00606F2B BYTE_EXACT
// LEVER: the long_from_bits/uint32_t round trip on plain LONG subtraction was dead weight the image never does (no overflow-defined behavior needed for these fields); direct `RECT` member arithmetic on the `volatile RECT*` matches the mnemonics exactly (2/16 -> 11/16).
// LEVER: the `volatile RECT *ordered_rect` alias was itself the remaining wall, not a fix for anything read twice - every field here is written once and read once. Reading `rect->field` directly (compound `-=`/`+=`, no volatile handle) let VC6 fold the param-then-field load order the image has instead of forcing a `mov edx, [eax]` ahead of the `ecx` load through the alias. 11/16 -> BYTE_EXACT 16/16.
// symbol    ?expand_rect@@YAPAUtagRECT@@PAU1@HH@Z
// size      43 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
RECT *__cdecl expand_rect(RECT *rect, int horizontal, int vertical) {
    rect->left -= horizontal;
    rect->right += horizontal;
    rect->top -= vertical;
    rect->bottom += vertical;
    return rect;
}

/*
Purpose: Set the left scrollbar sprites and horizontal button sprites.
// ORIGINAL: 0x00605BE0 ?set_sprite_left@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z 0x00605BE0-0x00605C2B BYTE_EXACT
// TRIED     ALL FOUR SPRITE SETTERS SHARE THIS WALL - 0x00605BE0, 0x00605C30, 0x00605C80 and 0x00605CD0 measure 5 of 21 IDENTICALLY, with the instruction counts matching - and it is NOT the helper. Unfactoring set_sprite_triplet into this body verbatim measured 5 of 21 again, unchanged: `__forceinline` already emits exactly what the inline source does, unlike sound.cpp's helpers, where unfactoring took five bodies to BYTE_EXACT. What differs is SCHEDULING: the image loads the argument pair into eax/edx up front (`mov eax, [esp+4]; mov edx, [esp+8]`) and stores both afterwards, where VC6 here interleaves load-store-load-store. offsetof PROVES the layout is right - `offsetof(Scroll, sprite_left1_) == 0xA7C` compiles, matching the image's own `[ecx + 0xa7c]` - so the differing `[ecx + 0xa80]`/`[ecx + 0xa84]` lines are the SECOND and THIRD stores, not a shifted member. I read them as a 4-byte layout error first; they are not.
// LEVER: marking the shared set_sprite_triplet helper __forceinline removed
//   the 7-argument push sequence and the E8 call, taking this from 0/21 to
//   5/21 agreeing (0.905 similar) - same fix as the other three sprite
//   setters, which share the one helper.
// LEVER (2026-09-03, BYTE_EXACT 21/21): the two walls this marker's TRIED
//   above called scheduling were BOTH the helper's volatile/raw spelling, see
//   the note on set_sprite_triplet itself. Dropping `volatile` from the
//   `Sprite *volatile *primary` alias unblocked the image's three argument
//   loads ahead of the three stores, and naming the dib fields
//   (`dib_.bmiHeader.biWidth`/`biHeight`, read after the stores) instead of
//   `read_volatile_bits(object, 0x4C4/0x4C8)` kept the `neg`/`cmp` below
//   them. Measured steps: 5/21 -> 8/21 (named dib reads alone) -> 21/21
//   (volatile dropped as well). Register assignment follows: the three
//   arguments take eax/edx/esi and the dib pair edi/ebx, this tree's exact
//   reverse before.
// symbol    ?set_sprite_left@Scroll@@QAEXPAVSprite@@00@Z
// size      75 bytes
// prototype void (__thiscall ?set_sprite_left@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z)(Scroll* this, Sprite*, Sprite*, Sprite*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_sprite_left(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, sprite_left_, &flat_button_left_, true,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the right scrollbar sprites and horizontal button sprites.
// ORIGINAL: 0x00605C30 ?set_sprite_right@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z 0x00605C30-0x00605C7B BYTE_EXACT
// LEVER: same __forceinline on set_sprite_triplet as 0x00605BE0 - 0/21 to
//   5/21 agreeing (0.905 similar).
// LEVER (2026-09-03, BYTE_EXACT 21/21): the same two helper edits as
//   0x00605BE0 - drop `volatile` from the triplet alias, name the dib fields
//   after the stores. One fix, four bodies.
// symbol    ?set_sprite_right@Scroll@@QAEXPAVSprite@@00@Z
// size      75 bytes
// prototype void (__thiscall ?set_sprite_right@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z)(Scroll* this, Sprite*, Sprite*, Sprite*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_sprite_right(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, sprite_right_, &flat_button_right_, true,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the upper scrollbar sprites and vertical upper-button sprites.
// ORIGINAL: 0x00605C80 ?UNK1@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z 0x00605C80-0x00605CCB BYTE_EXACT
// NAMING EVIDENCE (2026-08-28) for both UNKs (this and 0x00605CD0): the image
//   offers nothing - `strings -a terranx_original.exe` has ZERO `Scroll@@`
//   strings, so UNK1/UNK2 are IDB placeholders, not names the binary carries.
//   The structure is decisive instead. The button offset 0x15BC is
//   offsetof(Scroll, flat_button_left_) + offsetof(FlatButton, sprite1_)
//   (0xAAC + 0xB10) and 0x2108 is the SAME field of flat_button_right_
//   (0x15F8 + 0xB10): each pair writes the scrollbar's own stored triplet AND
//   the same three sprites into one end button's sprite1_..sprite3_. Left/up
//   share the LEFT button, right/down share the RIGHT button - a vertical
//   scrollbar's up arrow is its horizontal left arrow - and horizontal=false
//   here against set_sprite_left/right's true picks the vertical half of each
//   pair. That is what the names in scroll.h rest on.
// symbol    ?set_sprite_up@Scroll@@QAEXPAVSprite@@00@Z
// size      75 bytes
// prototype void (__thiscall ?UNK1@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z)(Scroll* this, Sprite*, Sprite*, Sprite*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_sprite_up(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, sprite_up_, &flat_button_left_, false,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the lower scrollbar sprites and vertical lower-button sprites.
// ORIGINAL: 0x00605CD0 ?UNK2@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z 0x00605CD0-0x00605D1B BYTE_EXACT
// NAMING EVIDENCE (2026-08-28): see set_sprite_up (0x00605C80) - 0x2108 is
//   flat_button_right_.sprite1_, the RIGHT button, and horizontal=false makes
//   this its vertical (down) half. No image name evidence exists either way.
// symbol    ?set_sprite_down@Scroll@@QAEXPAVSprite@@00@Z
// size      75 bytes
// prototype void (__thiscall ?UNK2@Scroll@@QAEXPAUSprite@@PAUSprite@@PAUSprite@@@Z)(Scroll* this, Sprite*, Sprite*, Sprite*)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Scroll::set_sprite_down(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, sprite_down_, &flat_button_right_, false,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Clamp, optionally reverse, and redraw the scrollbar position.
// ORIGINAL: 0x00605D20 ?set_pos@Scroll@@QAEXH@Z 0x00605D20-0x00605D8A
// TRIED: the guard clause's branch polarity (image falls through to the
//   continue path and jumps forward to a shared `ret` for the early return;
//   this tree's `if(!parent) return 0;` gets compiled the other way around,
//   `jne`+immediate `ret` first). Tried if/else, nested-if without else, and
//   goto-to-label; all three either reproduce the same layout or add a second
//   epilogue. Everything past the guard already matches in shape (8/29).
// TRIED: replacing the redraw_from_vtable(__asm helper) tail with the
//   inline read_volatile_bits/original_slot<func_noarg_virtual> dispatch -
//   still 8/29, no measured improvement, so reverted. The inline form emits
//   `mov eax,[ecx]; mov eax,[eax+0xf8]; call eax` (3 insns) against the
//   image's `mov edx,[ecx]; call dword ptr [edx+0xf8]` (2), the same
//   second-dispatch scheduling gap 0x00605A10's own note gives up on.
// LEVER: `parent`/`minimum`/`current`/`maximum`/`range_reversed_` reads were
//   each cast through `volatile` to a single already-named field - no raw
//   offsets, so nothing to rename, just the qualifier to drop. Plain reads
//   took this from 8/29 to 11/29. The branch-polarity gap above is
//   unaffected: it is still the first divergence, at instruction 2.
// LEVER: the redraw tail (2026-08-28) took this to 13/29 - the plain
//   virtual call replaced the redraw_from_vtable helper. TRIED after it:
//   the whole work block wrapped in `if (parent != nullptr)` with a single
//   fallthrough exit - 13/29, similarity 0.933 vs 0.931, two instructions
//   longer. The polarity gap survives its own new tail; first divergence
//   remains at instruction 2.
// LEVER (2026-09-03, 15/29, 0.966): BOTH of those two, together, are the
//   image's shape and neither is alone. The single-exit `if (parent !=
//   nullptr) { ... }` wrap is what produces the image's ONE `ret 4` reached
//   by `je` from the guard (this tree's `if (!parent) return 0U;` emits
//   `jne` + its own early `ret 4`, a second epilogue the image has no room
//   for) - and the inner clamp must be an if/else with TWO stores, not the
//   ternary the old body had: the ternary lets VC6 emit a select
//   (`mov eax,edx`) and one store where the image has
//   `mov [ecx+0xa2c],edx / jmp / mov [ecx+0xa2c],eax`. Measured apart, the
//   if/else clamp ALONE scores 3/29 - it is the combination that agrees.
//   13/29 -> 15/29. What is left is register naming only: this tree holds
//   `minimum` in edx where the image holds esi (and so the reversed-position
//   add uses the other register), loads the `position` argument one slot
//   earlier (`[esp+4]` before the `push esi`, against the image's `[esp+8]`
//   after it), and names the vtable pointer eax against the image's edx.
// symbol    ?set_pos@Scroll@@QAEIH@Z
// size      106 bytes
// prototype void (__thiscall ?set_pos@Scroll@@QAEXH@Z)(Scroll* this, int position)
// callers   17   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00605D80
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
// C4716: same reason as set_range above - the image's `ret 4` never sets EAX
// on the no-parent path, it just falls through to the shared epilogue with
// EAX still zero from the `test`. C4715 is the same fact seen from the other
// side: the early `return 0U;` is the only `return`, and the image's own
// null path leaves EAX zero the same way (the `test` it came from), so the
// compiler is right and so is the transcription.
#pragma warning(push)
#pragma warning(disable : 4716 4715)
uint32_t Scroll::set_pos(int position) {
    ::Win *const parent = win_parent_;
    if (parent != nullptr) {
        WinCallbackWindow = parent;

        const int minimum = range_minimum_;
        int *const current = &position_;
        if (position < minimum) {
            *current = minimum;
        } else {
            const int maximum = range_maximum_;
            if (position > maximum) {
                *current = maximum;
            } else {
                *current = position;
            }
        }
        if (range_reversed_ != 0U) {
            const uint32_t maximum = static_cast<uint32_t>(range_maximum_);
            const uint32_t clamped = static_cast<uint32_t>(*current);
            *current = long_from_bits(
                maximum - clamped + static_cast<uint32_t>(minimum));
        }
        // Slot 62 = GraphicWin::redraw (see set_range); the plain virtual call
        // is the image's two-instruction `mov edx,[ecx]; call [edx+0xf8]`.
        redraw();
    }
}
#pragma warning(pop)

/*
Purpose: Compute and publish the scrollbar thumb rectangle.
// ORIGINAL: 0x00606C50 ?compute_thumb_rect@Scroll@@QAEXPAURECT@@@Z 0x00606C50-0x00606E9E
// REWRITTEN 2026-08-28 off the raw-offset working copy (4/215, first divergence
//   at instruction 0) into named members: 52/215 at /c /O2 /Gy /GR- /GX, first
//   divergence now at instruction 48 - the first 48 instructions, the whole
//   prologue, the unrolled 16-byte copy and the whole normalize-to-origin
//   block, agree instruction for instruction.
// FIELD MAP (what the raw offsets were): 0xA4C thumb_rect_, 0xA1C
//   border_color_, 0xA3C field_A3C_ (the drag coordinate), 0xA14 field_A14_
//   (bit 1 = no end buttons), 0xA24 range_maximum_, 0xA20 range_minimum_,
//   0xA2C position_, 0x4C4/0x4C8 dib_.bmiHeader.biWidth/biHeight (Buffer is at
//   0x444, so those are +0x80/+0x84 - biHeight stored negative, hence the neg).
// LEVER: the copy loops were the body's own `for (offset; offset < sizeof
//   (RECT); offset += 4)` over read_bits/write_bits. The image UNROLLS both
//   16-byte copies into four load/store pairs; writing `*rect = thumb_rect_`
//   and `thumb_rect_ = *rect` gives exactly that. 4/215 -> 21/215 together
//   with the member rewrite.
// LEVER: the normalize block holds its offsets NEGATED and ADDS them
//   (`const LONG left = -rect->left; rect->right += left; rect->left += left;`).
//   The spelling you would write - read `left`, subtract twice - lets VC6 CSE
//   the two loads of rect->left and fold `left - left` to zero, which the image
//   never does: it loads rect->left TWICE and shares one `neg` across both
//   additions. Sub-form measured 12/215, negated form 26/215.
// LEVER: declaration and statement order in that block. `top` declared BEFORE
//   `left`, and the field updates in the order left/right/top/bottom (not
//   right/left/bottom/top), move it 26/215 -> 52/215. Pure register-allocation
//   scheduling: VC6 hands the two locals eax/edx the other way round
//   otherwise, and every load after them shifts.
// TRIED: restructuring the no-end-buttons selects in the not-dragging branch
//   into the single `if/else` the image's one `test byte` suggests (it sets
//   thumb_offset AND adjustment in the same two arms). 52/215 unchanged, only
//   0.590 -> 0.598 similar - kept, but it does not buy the instruction. The
//   real residue there is that VC6 materialises the mask `2` in edx
//   (`mov edx,2 / test byte ptr [ecx+0xa14], dl / sub [esp+0x1c], edx`) where
//   the image uses two immediates, so `adjustment` spills to the stack where
//   the image keeps it in a register.
// TRIED: `adjustment = -1 - adjustment` to reuse the -1 the image keeps live in
//   ebx - WORSE, 50/215 and 0.598 -> 0.504 similar; `0 - adjustment` (neg) is
//   what the image's own `neg`-bearing schedule wants. Reverted.
// TRIED: the two `if` conditions as `test byte`-friendly bools - the image
//   tests `[ecx+0xa14]` once per branch and never materialises a bool, so the
//   bool form is right; the earlier working copy's `no_end_buttons` bool was
//   NOT the defect. What is left is register naming (image carries rect->left
//   in ebp / drag in edx at instruction 48, this tree the other way round)
//   cascading through the two arms.
// symbol    ?compute_thumb_rect@Scroll@@QAEXPAUtagRECT@@@Z
// size      590 bytes
// prototype void (__thiscall ?compute_thumb_rect@Scroll@@QAEXPAURECT@@@Z)(Scroll* this, RECT*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
Verification note: several mutation-harness survivors in this body are
equivalent by construction, not coverage gaps. The two interim stores to
0xA3C in the drag branch are dead - the branch ends by unconditionally
storing 0xFFFFFFFF over them, faithfully mirroring the original's transient
writes, so no suite can observe them. The clamp idioms are strictness-
insensitive: `x < 0 ? 0U : x` equals `x < 1 ? 0U : x` and `x <= 0 ? 0U : x`
everywhere (the only inputs where the branches differ select equal values),
and likewise `a > b ? a : b` equals `a >= b ? a : b`. The early read_bits
offsets over the just-normalized rect scan bytes that were all zeroed two
statements earlier, so off-by-small-constant offset mutants read the same
zeros.
*/
void Scroll::compute_thumb_rect(RECT *rect) {
    *rect = thumb_rect_;
    const LONG top = -rect->top;
    const LONG left = -rect->left;
    rect->left += left;
    rect->right += left;
    rect->top += top;
    rect->bottom += top;
    if (border_color_ != -1) {
        rect->left += 1;
        rect->right += 1;
        rect->top += 1;
        rect->bottom += 1;
    }
    const LONG extent = rect->right - rect->left;
    LONG thumb_offset;
    if (field_A3C_ == -1) {
        LONG adjustment;
        if ((field_A14_ & 2) != 0) {
            thumb_offset = 0;
            adjustment = extent;
        } else {
            thumb_offset = extent + 1;
            adjustment = extent * 3;
        }
        adjustment = 0 - adjustment;
        if (border_color_ != -1) {
            adjustment -= 2;
        }
        const LONG height = -dib_.bmiHeader.biHeight;
        const LONG width = dib_.bmiHeader.biWidth;
        if (width > height) {
            if (range_maximum_ != range_minimum_) {
                thumb_offset += (position_ - range_minimum_)
                    * (width + adjustment) / (range_maximum_ - range_minimum_);
            }
            rect->left += thumb_offset;
            rect->right += thumb_offset;
        } else {
            if (range_maximum_ != range_minimum_) {
                thumb_offset += (position_ - range_minimum_)
                    * (height + adjustment) / (range_maximum_ - range_minimum_);
            }
            rect->top += thumb_offset;
            rect->bottom += thumb_offset;
        }
    } else {
        const bool no_end_buttons = (field_A14_ & 2) != 0;
        const LONG far_edge = rect->left + extent;
        const LONG candidate = field_A3C_ + extent / 2;
        field_A3C_ = candidate;
        const LONG height = -dib_.bmiHeader.biHeight;
        const LONG width = dib_.bmiHeader.biWidth;
        if (width > height) {
            const LONG upper = no_end_buttons ? width - far_edge : width - far_edge * 2;
            const LONG limited = candidate < upper ? candidate : upper;
            if (no_end_buttons) {
                thumb_offset = limited < 0 ? 0 : limited;
            } else if (far_edge > limited) {
                thumb_offset = far_edge;
            } else {
                thumb_offset = limited;
            }
            field_A3C_ = thumb_offset;
            rect->left += thumb_offset;
            rect->right += thumb_offset;
        } else {
            const LONG upper = no_end_buttons ? height - far_edge : height - far_edge * 2;
            const LONG limited = candidate < upper ? candidate : upper;
            if (no_end_buttons) {
                thumb_offset = limited < 0 ? 0 : limited;
            } else if (far_edge > limited) {
                thumb_offset = far_edge;
            } else {
                thumb_offset = limited;
            }
            field_A3C_ = thumb_offset;
            rect->top += thumb_offset;
            rect->bottom += thumb_offset;
        }
        field_A3C_ = -1;
    }
    thumb_rect_ = *rect;
}

/*
Purpose: Reset the scrollbar thumb rectangle from its stored thickness.
// ORIGINAL: 0x00606EA0 ?set_thumb_rect@Scroll@@QAEXXZ 0x00606EA0-0x00606EF8
// TRIED: dropping the `volatile` casts on `bar_thickness_`, `border_color_`
//   and the `thumb` RECT alias: 12/20 -> 7/20, a clear regression. Without
//   `volatile` the optimizer proves the unconditional 0/0/thickness/thickness
//   rect writes are dead when the branch below overwrites them and elides
//   them, which the image does not do - same shape as `set_border_color`'s
//   already-documented dead-store hazard. Reverted.
// TRIED (2026-08-28) dropping the volatile ONLY on the two integer accesses,
//   `bar_thickness_` and the `border_color_` guard, keeping `volatile RECT
//   *const thumb`: fixes the compare - the image has a memory-immediate
//   `cmp dword ptr [ecx+0xa1c],-1` and the volatile read forces
//   `mov edx,[ecx+0xa1c] / cmp edx,-1`, one instruction longer - but the
//   body then compiles to 19 instructions against the image's 20, so the
//   `je` target shifts 3 bytes and lands differently: 10/20 at 0.923 similar
//   against the tree's 12/20 at 0.900. Same measurement for the single-exit
//   `result` shape that took set_bar_thickness to 12/22, and for holding the
//   zero in a `uint32_t left_edge` local. LEFT on the tree's 12/20, but note
//   the direction: the volatile-read version is one lever from exact (the
//   image's `inc edx / mov [ecx+0xa4c],edx` pair, see set_bar_thickness's
//   TRIED), while the volatile-guard version is two.
// TRIED (2026-08-28) `thumb_rect_.left = thumb_rect_.left + 1` and
//   `thumb->left = thumb->left + 1` for that pair: 9/20 and 9/20. Refuted.
// symbol    ?set_thumb_rect@Scroll@@QAEIXZ
// size      88 bytes
// prototype void (__thiscall ?set_thumb_rect@Scroll@@QAEXXZ)(Scroll* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
uint32_t Scroll::set_thumb_rect() {
    const uint32_t thickness = static_cast<uint32_t>(
        *reinterpret_cast<volatile int *>(&bar_thickness_));
    volatile RECT *const thumb = &thumb_rect_;
    thumb->left = 0;
    thumb->top = 0;
    thumb->right = long_from_bits(thickness);
    thumb->bottom = long_from_bits(thickness);
    if (*reinterpret_cast<volatile int *>(&border_color_) == -1) {
        return thickness;
    }

    const uint32_t right = static_cast<uint32_t>(thumb->right) - 1U;
    thumb->left = 1;
    const uint32_t top = static_cast<uint32_t>(thumb->top) + 1U;
    thumb->right = long_from_bits(right);
    const uint32_t bottom = static_cast<uint32_t>(thumb->bottom) - 1U;
    thumb->top = long_from_bits(top);
    thumb->bottom = long_from_bits(bottom);
    return bottom;
}





















/*
Purpose: Drag the thumb or repeat a track-page movement while the scroll owns
         the pointer, then notify its parent of an actual position change.
// ORIGINAL: 0x00605D90 ?on_mouse_move@Scroll@@QAEXHH@Z 0x00605D90-0x006061DB FILE
// RECOVERY_BINDING: v1 {"evidence":["source:src/recovered/units/00605d90.cpp","source:src/scroll.h:Scroll","source:src/win.h:WinPointerOwner1","source:src/win.h:WinSizingFlag"],"reason":"Home the drag and held-track behavior on real Scroll/Win members, source-owned pointer state and parent callbacks; remove seed vtable facades and executable-address globals."}
// symbol    ?on_mouse_move@Scroll@@UAEXHH@Z
*/
void Scroll::on_mouse_move(int x, int y) {
    Win *const parent = win_parent_;
    if (parent == nullptr) {
        return;
    }

    Win *active = WinPointerOwner1;
    if (active == nullptr) {
        active = WinPointerOwner2;
    }
    if (active != this || WinSizingFlag == 0) {
        return;
    }

    RECT local = {0, 0, rect2_.right - rect2_.left,
                  rect2_.bottom - rect2_.top};
    if (!in_box(x, y, &local)) {
        return;
    }

    ListBoxGlobalDDC = 0;
    const int saved_position = position_;
    const bool horizontal = -dib_.bmiHeader.biHeight < dib_.bmiHeader.biWidth;
    if (field_A44_ == 0) {
        const int extent = thumb_rect_.right - thumb_rect_.left;
        int denominator = extent * -3;
        if (border_color_ != -1) {
            denominator -= 2;
        }

        int next_position;
        if (horizontal) {
            int offset = x - extent;
            if (border_color_ != -1) {
                --offset;
            }
            next_position = (offset - (extent >> 1))
                            * (range_maximum_ - range_minimum_)
                            / (denominator + dib_.bmiHeader.biWidth)
                            + range_minimum_;
            field_A3C_ = offset;
        } else {
            int offset = y - extent;
            if (border_color_ != -1) {
                ++offset;
            }
            field_A3C_ = offset;
            next_position = (offset - (extent >> 1))
                            * (range_maximum_ - range_minimum_)
                            / (denominator - dib_.bmiHeader.biHeight)
                            + range_minimum_;
        }

        if (next_position > range_maximum_) {
            next_position = range_maximum_;
        }
        if (next_position < range_minimum_) {
            next_position = range_minimum_;
        }
        position_ = next_position;
    } else {
        const DWORD now = timeGetTime();
        const DWORD last_time = field_A38_;
        if (last_time != 0 &&
            static_cast<int>(now - last_time) <
                static_cast<int>(field_A30_ * 2U)) {
            if (get_lbutton_state() == 0) {
                return;
            }
            get_mouse_pos(&x, &y);
            const LPARAM point = static_cast<LPARAM>(
                (static_cast<uint32_t>(y) << 16) |
                (static_cast<uint32_t>(x) & 0xFFFFU));
            PostMessageA(HandleMain, WM_MOUSEMOVE, 0, point);
            return;
        }
        if (last_time == 0) {
            field_A38_ = timeGetTime();
        }

        const int direction = static_cast<int>(field_A44_);
        if (horizontal) {
            if (x > thumb_rect_.right && direction > 0) {
                const int next = position_ + paging_;
                position_ = next > range_maximum_ ? range_maximum_ : next;
            } else if (x < thumb_rect_.left && direction < 0) {
                const int next = position_ - paging_;
                position_ = next < range_minimum_ ? range_minimum_ : next;
            }
        } else if (y > thumb_rect_.bottom && direction > 0) {
            const int next = position_ + paging_;
            position_ = next > range_maximum_ ? range_maximum_ : next;
        } else if (y < thumb_rect_.top && direction < 0) {
            const int next = position_ - paging_;
            position_ = next < range_minimum_ ? range_minimum_ : next;
        }

        get_mouse_pos(&x, &y);
        if (get_lbutton_state() != 0) {
            const LPARAM point = static_cast<LPARAM>(
                (static_cast<uint32_t>(y) << 16) |
                (static_cast<uint32_t>(x) & 0xFFFFU));
            PostMessageA(HandleMain, WM_MOUSEMOVE, 0, point);
        }
    }

    WinCallbackWindow = parent;
    if (saved_position != position_) {
        int notified_position = position_;
        if (range_reversed_ != 0U) {
            notified_position = (range_minimum_ - position_) + range_maximum_;
        }
        parent->on_scrolling(field_A18_, notified_position);
        if (field_2148_ != nullptr) {
            int callback_position = position_;
            if (range_reversed_ != 0U) {
                callback_position = (range_minimum_ - position_)
                                    + range_maximum_;
            }
            field_2148_(field_A18_, callback_position);
        }
    }

    Sound *const target = parent->legacy_sound_33();
    if (target != nullptr && target->is_playing() == 0) {
        target->play();
    }
    redraw();
}

// ORIGINAL: 0x006061E0 ?on_left_down@Scroll@@QAEXHH@Z 0x006061E0-0x006062B8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/006061e0.cpp", "Scroll thumb and range members in src/scroll.h", "Win::on_mouse_move and WinPointerOwner3/4 in src/win.h"], "reason": "Recover the Scroll thumb press handler using its typed orientation, thumb geometry, parent callback and capture-owner state."}
// symbol    ?on_left_down@Scroll@@UAEXHH@Z
void Scroll::on_left_down(int x, int y) {
    if (win_parent_ == 0) {
        return;
    }
    if (-dib_.bmiHeader.biHeight < dib_.bmiHeader.biWidth) {
        if (x < thumb_rect_.left) {
            field_A44_ = 0xffffffffU;
            field_A38_ = 0;
            on_mouse_move(x, y);
            goto track_scroll;
        }
        if (thumb_rect_.right < x) {
            field_A44_ = 1;
            field_A38_ = 0;
            on_mouse_move(x, y);
            goto track_scroll;
        }
        field_A44_ = 0;
    } else {
        if (y < thumb_rect_.top) {
            field_A44_ = 0xffffffffU;
        } else {
            if (y <= thumb_rect_.bottom) {
                field_A44_ = 0;
                goto track_scroll;
            }
            field_A44_ = 1;
        }
        field_A38_ = 0;
        on_mouse_move(x, y);
    }

track_scroll:
    if (WinPointerOwner3 == 0) {
        WinPointerOwner4 = 0;
        WinPointerOwner3 = this;
    }
}

// ORIGINAL: 0x006062C0 ?on_left_up@Scroll@@QAEXHH@Z 0x006062C0-0x00606304 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/006062c0.cpp", "Scroll::win_parent_ and inherited redraw/capture virtuals", "WinPointerOwner3/4 in src/win.h"], "reason": "Recover Scroll thumb release redraw and conditional capture cleanup with typed base and process-owned pointer state; the historical byte-exact seed remains preserved."}
// symbol    ?on_left_up@Scroll@@UAEXHH@Z
void Scroll::on_left_up(int, int) {
    if (win_parent_ != 0) {
        redraw();
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == this) {
            WinPointerOwner4 = 0;
        }
    }
}

// ORIGINAL: 0x00606320 ?on_mousewheel_down@Scroll@@QAEXH@Z 0x00606320-0x0060643E
// RECOVERY_BINDING: v1 {"reason":"Keep the original chosen reversal branch, callback-visible reloads and delta-gated sound operation instead of caching notification values or omitting sound.","evidence":["pinned:606382..6063FD reloads callback/id/position/bounds after parent notification inside the original reversal branch","pinned:60637C jumps directly to redraw for delta zero;606402..606425 otherwise reloads the live parent and checks/plays its sound after both notifications, without another parent-null guard"]}
void Scroll::on_mousewheel_down(int delta) {
    Win *const parent = win_parent_;
    if (parent == nullptr) {
        return;
    }
    WinCallbackWindow = parent;

    int next = position_ + 1;
    const int maximum = range_maximum_;
    if (next > maximum) {
        next = maximum;
    }
    position_ = next;
    if (range_reversed_ != 0U) {
        position_ = (range_minimum_ - position_) + maximum;
    }

    if (delta != 0) {
        if (range_reversed_ == 0U) {
            win_parent_->on_scrolled(field_A18_, position_);
            if (field_2144_ != nullptr) {
                field_2144_(field_A18_, position_);
            }
        } else {
            win_parent_->on_scrolled(field_A18_,
                                    (range_minimum_ - position_) + range_maximum_);
            if (field_2144_ != nullptr) {
                field_2144_(field_A18_,
                            (range_minimum_ - position_) + range_maximum_);
            }
        }
        Sound *const target = win_parent_->legacy_sound_33();
        if (target != nullptr && target->is_playing() == 0) {
            target->play();
        }
    }

    field_A3C_ = 0xFFFFFFFFU;
    redraw();
}

// ORIGINAL: 0x00606440 ?on_mousewheel_up@Scroll@@QAEXH@Z 0x00606440-0x0060655C
// RECOVERY_BINDING: v1 {"reason":"Preserve the original delta gate over notifications and sound, branch-selected reversal mapping, and callback-visible reloads.","evidence":["pinned:60649C skips directly to redraw for delta zero;6064BC/6064EE invoke parent notification;6064C2..60651C reads current callback/position/ranges/id within the previously selected branch","pinned:606521 reloads live parent after callbacks for sound; no additional parent-null guard is present"]}
void Scroll::on_mousewheel_up(int delta) {
    Win *const parent = win_parent_;
    if (parent == nullptr) {
        return;
    }
    WinCallbackWindow = parent;

    if (position_ - 1 < range_minimum_) {
        position_ = range_minimum_;
    } else {
        --position_;
    }
    if (range_reversed_ != 0U) {
        position_ = (range_maximum_ - position_) + range_minimum_;
    }

    if (delta != 0) {
        if (range_reversed_ == 0U) {
            win_parent_->on_scrolled(field_A18_, position_);
            if (field_2144_ != nullptr) {
                field_2144_(field_A18_, position_);
            }
        } else {
            win_parent_->on_scrolled(field_A18_,
                                    (range_minimum_ - position_) + range_maximum_);
            if (field_2144_ != nullptr) {
                field_2144_(field_A18_,
                            (range_maximum_ - position_) + range_minimum_);
            }
        }
        Sound *const target = win_parent_->legacy_sound_33();
        if (target != nullptr && target->is_playing() == 0) {
            target->play();
        }
    }

    field_A3C_ = 0xFFFFFFFFU;
    redraw();
}

/*
Purpose: Move a scrollbar to an endpoint with Home/End when horizontal, or
         advance one page with Page Up/Down when vertical; notify its owner.
// ORIGINAL: 0x00606560 ?on_key_click@Scroll@@QAEHHH@Z 0x00606560-0x006066EC FILE
// RECOVERY_BINDING: v1 {"evidence":["source:src/unrecovered/00606560.cpp","source:src/scroll.cpp:Scroll::on_mousewheel_up","source:src/listbox.cpp:ListBoxGlobalDDC"],"reason":"Home the key navigation on Scroll's inherited Buffer, range members, real parent dispatch and source-owned notification globals; no vtable stand-ins or fixed executable addresses."}
// symbol    ?on_key_click@Scroll@@UAEHHH@Z
// The old seed's spelling diverged at its prologue; this typed owner body is
// deliberately unclaimed until it is remeasured and restored.
*/
int Scroll::on_key_click(int, int key) {
    Win *const parent = win_parent_;
    if (parent == nullptr) {
        return 0;
    }

    ListBoxGlobalDDC = 1;
    const int width = dib_.bmiHeader.biWidth;
    const int height = -dib_.bmiHeader.biHeight;
    if (width > height) {
        if (key == VK_END) {
            position_ = range_maximum_;
        } else if (key == VK_HOME) {
            position_ = range_minimum_;
        } else {
            return 0;
        }
    } else if (key == VK_PRIOR) {
        position_ -= paging_;
        if (position_ < range_minimum_) {
            position_ = range_minimum_;
        }
    } else if (key == VK_NEXT) {
        position_ += paging_;
        if (position_ > range_maximum_) {
            position_ = range_maximum_;
        }
    } else {
        return 0;
    }

    WinCallbackWindow = parent;
    // Native606625 chooses the mapping branch before the virtual callback;
    // 606655/60668B reload its values afterward without retesting reversal.
    if (range_reversed_ == 0U) {
        win_parent_->on_scrolled(field_A18_, position_);
        if (field_2144_ != nullptr) {
            field_2144_(field_A18_, position_);
        }
    } else {
        win_parent_->on_scrolled(field_A18_,
                                (range_maximum_ - position_) + range_minimum_);
        if (field_2144_ != nullptr) {
            field_2144_(field_A18_,
                        (range_maximum_ - position_) + range_minimum_);
        }
    }

    Sound *const target = win_parent_->legacy_sound_33();
    if (target != nullptr && target->is_playing() == 0) {
        target->play();
    }

    field_A3C_ = 0xFFFFFFFFU;
    redraw();
    return 1;
}

// ORIGINAL: 0x00606B20 ?on_button_clicked@Scroll@@QAEXH@Z 0x00606B20-0x00606C43 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Preserve the original branch-selected notification mapping while reloading callback-visible position, bounds, callback identity and parent after external notifications; do not reuse pre-callback snapshots.","evidence":["pinned:606B7E..606B9F selects normal/reversed notification before the parent virtual call;606BA5..606BB5 reloads the normal callback and current position;606BDB..606C03 reloads the reversed callback and current max/position/min/id without retesting reversal","pinned:606C08 reloads current parent+0xC4 and its sound+0x80 after both notifications; there is no second parent-null guard. Preserve that original error path, final thumb reset and virtual redraw."]}
// symbol    ?on_button_clicked@Scroll@@UAEXH@Z
void Scroll::on_button_clicked(int button_id) {
    Win *const parent = win_parent_;
    if (parent == nullptr) {
        return;
    }

    field_A38_ = 0;
    WinCallbackWindow = parent;
    ListBoxGlobalDDC = 0;

    if (button_id == 1) {
        if (position_ > range_minimum_) {
            --position_;
        }
    } else if (button_id == 2 && position_ < range_maximum_) {
        ++position_;
    }

    // The original chooses the mapping branch once, before notification.
    // Callbacks can change the values, but not that already chosen branch.
    if (range_reversed_ == 0U) {
        win_parent_->on_scrolled(field_A18_, position_);
        if (field_2144_ != nullptr) {
            field_2144_(field_A18_, position_);
        }
    } else {
        win_parent_->on_scrolled(field_A18_,
                                (range_maximum_ - position_) + range_minimum_);
        if (field_2144_ != nullptr) {
            field_2144_(field_A18_,
                        (range_maximum_ - position_) + range_minimum_);
        }
    }

    Sound *const target = win_parent_->legacy_sound_33();
    if (target != nullptr && target->is_playing() == 0) {
        target->play();
    }

    field_A3C_ = 0xFFFFFFFFU;
    redraw();
}

// ORIGINAL: 0x006069A0 ?on_button_pressed@Scroll@@QAEXH@Z 0x006069A0-0x00606B14
// symbol    ?on_button_pressed@Scroll@@UAEHH@Z
int Scroll::on_button_pressed(int button_id) {
    // Constructor table669D58 slot41 selects this int callback. All original
    // exits converge on606B08, whose epilogue sets EAX=1, including throttling.
    if (win_parent_ == nullptr) {
        return 1;
    }

    const DWORD start_time = timeGetTime();
    const int last_time = field_A38_;
    if (last_time != 0) {
        const DWORD now = timeGetTime();
        if (static_cast<int>(now - static_cast<DWORD>(last_time)) <
            static_cast<int>(field_A30_)) {
            return 1;
        }
    } else {
        field_A38_ = static_cast<int>(timeGetTime());
    }

    ListBoxGlobalDDC = 0;
    const int old_position = position_;
    if (button_id == 1) {
        if (range_minimum_ < old_position) {
            position_ = old_position - 1;
        }
    } else if (button_id == 2 && old_position < range_maximum_) {
        position_ = old_position + 1;
    }

    WinCallbackWindow = win_parent_;
    if (old_position != position_) {
        int notified_position = position_;
        if (range_reversed_ != 0U) {
            notified_position = (range_minimum_ - position_) + range_maximum_;
        }
        win_parent_->on_scrolling(field_A18_, notified_position);
        int callback_position = position_;
        if (range_reversed_ != 0U) {
            callback_position = (range_minimum_ - position_) + range_maximum_;
        }
        if (field_2148_ != nullptr) {
            field_2148_(field_A18_, callback_position);
        }
        // Original606A2F skips this sound path when the position is unchanged.
        // After notification606AB2 reloads the live parent, not an earlier view.
        Sound *const target = win_parent_->legacy_sound_33();
        if (target != nullptr && target->is_playing() == 0) {
            target->play();
        }
    }

    field_A3C_ = 0xFFFFFFFFU;
    redraw();
    const DWORD elapsed = timeGetTime() - start_time;
    const DWORD duration = 1000U / static_cast<DWORD>(field_A34_);
    if (elapsed < duration) {
        Sleep(duration - elapsed);
    }
    return 1;
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00606310 ?on_left_click@Scroll@@QAEHHH@Z 0x00606310-0x00606313 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Scroll constructor 0x6051D0 stores primary addresspoint 0x669D58 at [this] at 0x605214; table slot 19 at 0x669DA4 targets original 0x606310.", "Pinned 0x606310 is a bare ret 8 with no register writes. Win's slot-19 callback type is void, and observed calls through that slot continue using other registers without consuming EAX.", "The original catalogued decorated identity/prototype still says int (__thiscall ?on_left_click@Scroll@@QAEHHH@Z); that mismatch is preserved in the ORIGINAL history. This candidate does not assert a new emitted symbol before native owner-object observation."], "reason": "Model Scroll's constructor-installed Win slot-19 entry as the genuine void callback, keeping its original byte claim and catalogued int signature as historical evidence."}
// symbol    ?on_left_click@Scroll@@UAEXHH@Z
// size      3 bytes
// prototype int (__thiscall ?on_left_click@Scroll@@QAEHHH@Z)(Scroll* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Scroll::on_left_click(int, int) {
}



/*
Purpose: Destroy a Scroll: run close, destroy the embedded right then left
         FlatButtons, and finish with the GraphicWin base teardown through
         ordinary C++ destruction, including exception cleanup.
// ORIGINAL: 0x00406E60 ??1Scroll@@QAE@XZ 0x00406E60-0x00406F1A;0x00650BB0-0x00650BEE BYTE_EXACT
// symbol    ??1Scroll@@UAE@XZ
// size      248 bytes
// prototype void (__thiscall ??1Scroll@@QAE@XZ)(Scroll* this)
// callers   41   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00605370 0x00607040 0x00607DA0
Return Value: n/a
*/
Scroll::~Scroll() {
    close();
}


/*
Purpose: The compiler-generated scalar deleting destructor: run the complete
         destructor and, when bit 0 of the mode asks, free the storage to the
         game heap.
// ORIGINAL: 0x00406F20 ??_GScroll@@UAEPAXI@Z 0x00406F20-0x00406F41 FILE BYTE_EXACT
// symbol    ??_GScroll@@UAEPAXI@Z
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00406E60 0x0064557F
Return Value: the object pointer
Status: Complete
*/
// The existing virtual destructor owns this compiler-generated entry.


/*
Purpose: Step the receiver back to the subobject ??_GScroll@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004070C0 ??_GScroll@@WEEE@AEPAXI@Z 0x004070C0-0x004070CB BYTE_EXACT
// symbol    ??_EScroll@@WEEE@AEPAXI@Z
// CORRECTED from ??3Scroll@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00406F20` into
//   ??_GScroll@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

/*
// ORIGINAL: 0x006051C0 sub_6051c0 0x006051C0-0x006051CE BYTE_EXACT
// symbol    _sub_6051c0
// size      14 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x006051C2
Return Value: n/a
Status: Complete
*/
// `__declspec(dllimport)` on GetSystemMetrics is load-bearing: without it
// VC6 emits a direct `call rel32` (5 bytes) where the original calls
// indirectly through the import slot at 0x00669334 with `call dword ptr`
// (6 bytes). The declaration used to sit here for that reason; it comes from
// the scaffold now (emit_translation_unit.WIN32_IMPORTS), which carries the
// same `__declspec(dllimport)` for the same reason.

extern "C" void __cdecl sub_6051c0() {
    ScrollDefaultThickness = GetSystemMetrics(3);
}
