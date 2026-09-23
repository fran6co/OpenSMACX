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
#include "socialwin.h"
#include "win.h"
#include "subinterface.h"
#include "vector_teardown.h"
#include "game.h"
#include "strings.h"
#include "text.h"
#include "faction.h"
#include "alpha.h"
#include "spying_recovery.h"
#include "mapwin.h"   // MainInterfaceGlobal, at 0x007AE820
#include "tutwin.h"

void __fastcall SocialWinFlatButtonCtor(void *self) { reinterpret_cast<FlatButton *>(self)->FlatButton::FlatButton(); }
void __fastcall SocialWinFlatButtonDtor(void *self) { reinterpret_cast<FlatButton *>(self)->~FlatButton(); }
void __fastcall SocialWinCheckButtonCtor(void *self) { reinterpret_cast<CheckButton *>(self)->CheckButton::CheckButton(); }
void __fastcall SocialWinCheckButtonDtor(void *self) { reinterpret_cast<CheckButton *>(self)->~CheckButton(); }
void __fastcall SocialWinButtonGroupCtor(void *self) { reinterpret_cast<ButtonGroup *>(self)->ButtonGroup::ButtonGroup(); }
void __fastcall SocialWinButtonGroupDtor(void *self) { reinterpret_cast<ButtonGroup *>(self)->~ButtonGroup(); }

/*
Purpose: Install the SubInterface vftable, construct the Spot, nine runs of
         further sub-objects (a CheckButton[3], four Sprite runs, a
         Buffer[6], a FlatButton[35], a ButtonGroup[4] and one more
         ButtonGroup, nine Fonts), then install SocialWin's own vftables and
         set factionID_ to -1.
// TRIED (2026-08-25): declaring the five member ARRAYS the constructor was
// hand-constructing. The arithmetic proves what they are - the body called
// `VectorCtorIterator(self + 0x2C60, 0x2C, 0xA, ...)` and `sizeof(Sprite)`
// is 0x2C, so 0x2C * 0xA == 0x1B8 is exactly the `uint8_t[0x1B8]` that sat
// at 0x2C60; likewise 0x5A and 2 and 6 Sprites, and 6 Buffers at 0x588.
// The `const void *` ctor/dtor constants were Sprite's and Buffer's own
// constructors and destructors, which C++ cannot spell as function
// references at all - the same tell that made FontQueue's array obvious.
// SCORE WENT DOWN, 20 of 121 to 16 of 121, and the fix is KEPT anyway:
// agreement is not monotonic in correctness, the declarations are proved
// by arithmetic rather than guessed, and no claim was at stake (this body
// has never been BYTE_EXACT). VC6 emits its generated iterator calls in a
// different ORDER than the image, which is the next thing to chase here -
// most likely because the image's class declares members this one still
// does not.
// ORIGINAL: 0x004AE9E0 ??0SocialWin@@QAE@XZ 0x004AE9E0-0x004AEBE7;0x006598A0-0x00659A0A
// TRIED: deriving from ConstructedGraphicWin (graphicwin.h) to move the
//   base construction ahead of the members. The ORDER IS RIGHT - this image
//   really does `call 0x5d4cf0` before its first `lea ecx, [esi + N]`, checked
//   2026-08-22 - but making the source agree took this body 16 -> 16
//   of 121. Same shape as Scroll (scroll.cpp), where the reorder alone was
//   worth 1 instruction and the payoff came from two levers underneath that it
//   made reachable; here those levers are not the vtable-immediate one, since
//   these constants already fold, and this body is structurally far off
//   (133 compiled against 121). Re-derive the ordering fix as part
//   of a dedicated pass on the whole body, not on its own.
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 0xc` and an extra
//            callee-save (ebx) the image does not. MISMATCH, 15/121
//            instructions agree.
// size      881 bytes
// prototype void (__thiscall ??0SocialWin@@QAE@XZ)(SocialWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005FA860 0x00618EA0 0x0062B7C0 0x006456E4 0x006457C2
*/
// Every sub-object is placement-new'd EXPLICITLY, in the exact order the
// disassembly shows, at its raw offset rather than through a named member -
// GraphicWin::construct() has to run first, which implicit member
// construction cannot reproduce, and the header's own mid-section field
// names are unverified guesses this constructor's own evidence disagrees
// with in at least one place (see the note beside energyLockButtons_ in
// socialwin.h). CheckButton's own recovery is out of this batch's scope, so
// its constructor/destructor addresses are reached raw, same as every
// other not-yet-promoted callee.
SocialWin::SocialWin() {
    new (static_cast<GraphicWin *>(this)) GraphicWin();
    char *const self = reinterpret_cast<char *>(this);

    // spot_, socEffect_, socEffectOrig_, energyLockButtons_, flatButtons_,
    // buttonGroups_, buttonGroup_ and font1_..font9_ are REAL members in
    // socialwin.h already (not raw storage) - they build IMPLICITLY, in
    // declaration order, before this body runs. The explicit placement-new
    // calls that used to stand here duplicated that construction: since the
    // header already types these as real classes, C++ constructs them
    // whether or not the body also does, and the old explicit calls at the
    // same raw offsets meant every one of these sub-objects was built TWICE.
    // Only the sub-objects that are STILL raw storage (four Sprite runs the
    // header cannot yet type - see socEngModifierNums_/effectIcons_/
    // energyAllocLock_/energyAllocArrow_ - and the Buffer[6] run inside
    // energyAllocSlider_, which is larger than 6 Buffers and not fully
    // accounted for) stay explicit below.

    factionID_ = 0xFFFFFFFFU;
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004AEF00 ?UNK2@SocialWin@@QAEXXZ 0x004AEF00-0x004AEF01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK2@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004B1790 ?UNK3@SocialWin@@QAEXXZ 0x004B1790-0x004B1791 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK3@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::UNK3() {
}

// Original interface slot 3 ignores its coordinates and dismisses the
// tutorial. Its seed's int result is incidental EAX from reset; interface
// callers consume no result. Installing the full interface contract remains
// part of the shared SubInterface hierarchy recovery.
// ORIGINAL: 0x004B2210 ?on_iface_left_click@SocialWin@@QAEHHH@Z 0x004B2210-0x004B221D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4B2210 loads8C6E68 and calls4BDDE0, then returns with8bytes callee cleanup. Neither coordinate is read. Table66DA50 slot3 identifies the secondary-interface callback; the current incomplete SubInterface contract still needs coordinated repair.", "The catalogued int return forwards incidental EAX from reset; interface slot3 callers do not consume it and the family contract is void(int,int). Preserve the seed identity and original exact obligation while publishing the compiler-observed source symbol."], "reason": "Home the tutorial-dismissal interface handler on SocialWin using the real process-lifetime TutorialWindow and recovered reset service."}
// symbol    ?on_iface_left_click@SocialWin@@QAEXHH@Z
void SocialWin::on_iface_left_click(int, int) {
    TutorialWindow.reset();
}

// ORIGINAL: 0x004B2220 ?on_iface_right_click@SocialWin@@QAEXHH@Z 0x004B2220-0x004B23FA
// RECOVERY_BINDING: v1 {"evidence": ["Original4B2220..4B23FA first resets TutorialWindow then tests interface-relative1E84C/1E85C/1E86C/1E83C, which are complete-object rectangles1F260/1F270/1F280/1F250. Labels.strings_ptr offsetsD24/D28/D2C/D1C select labels841/842/843/839; first match wins.", "Original4B2256/59 swaps argument stack contents before passing their reversed slot addresses to MainInterfaceGlobal.client_to_screen; this preserves logical x/y rather than the swapped coordinates in the seed. Every matching path constructs a degenerate screen rectangle and calls set_bubble_text on7AE820; no match only dismisses the tutorial."], "reason": "Recover social-panel tooltip selection with genuine rectangle members, label storage and the main-interface window receiver."}
void SocialWin::on_iface_right_click(int x, int y) {
    TutorialWindow.reset();
    int label;
    if (in_box(x, y, &tooltipEconomy_)) {
        label = 841;
    } else if (in_box(x, y, &tooltipLabs_)) {
        label = 842;
    } else if (in_box(x, y, &tooltipPsych_)) {
        label = 843;
    } else if (in_box(x, y, &tooltipEffects_)) {
        label = 839;
    } else {
        return;
    }
    char *text = label_get(label);
    // The original reuses its argument slots after swapping their contents;
    // the resulting conversion is still x then y, on the main interface.
    MainInterfaceGlobal.client_to_screen(&x, &y);
    RECT rectangle = {x, y, x, y};
    MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);
}



/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2400 ?on_iface_left_down@SocialWin@@QAEXHH@Z 0x004B2400-0x004B2403 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2410 ?on_iface_right_down@SocialWin@@QAEXHH@Z 0x004B2410-0x004B2413 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2440 ?on_iface_selected@SocialWin@@QAEXHH@Z 0x004B2440-0x004B2443 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2450 ?on_iface_left_double_click@SocialWin@@QAEXHH@Z 0x004B2450-0x004B2453 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2460 ?on_iface_right_double_click@SocialWin@@QAEXHH@Z 0x004B2460-0x004B2463 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2670 ?on_iface_button_toggled@SocialWin@@QAEXHH@Z 0x004B2670-0x004B2673 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004B2680 ?on_iface_group_clicked@SocialWin@@QAEXHHH@Z 0x004B2680-0x004B2683 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_group_clicked@SocialWin@@QAEXHHH@Z)(SocialWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_group_clicked(int, int, int) {
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x004B2420 ?on_iface_mouse_move@SocialWin@@QAEXHH@Z 0x004B2420-0x004B242D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?on_iface_mouse_move@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_mouse_move(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x004B2430 ?on_iface_mouse_leave@SocialWin@@QAEXHH@Z 0x004B2430-0x004B243D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@SocialWin@@QAEXHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void SocialWin::on_iface_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
// ORIGINAL: 0x004B2690 ?show@SocialWin@@QAEXH@Z 0x004B2690-0x004B26AB BYTE_EXACT
// symbol    ?show@SocialWin@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@SocialWin@@QAEXH@Z)(SocialWin* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void SocialWin::show(int visible) {
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        SubInterface::set_iface_mode();
    }
}

/*
// ORIGINAL: 0x004B26B0 ?hide@SocialWin@@QAEXXZ 0x004B26B0-0x004B26C9 BYTE_EXACT
// symbol    ?hide@SocialWin@@UAEXXZ
// size      25 bytes
// prototype void (__thiscall ?hide@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005F7E90
Status: Complete
*/
void SocialWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        SubInterface::release_iface_mode();
    }
}

/*
Purpose: Step the receiver back to the subobject ??_GSocialWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004B3F90 ??_GSocialWin@@WEEE@AEPAXI@Z 0x004B3F90-0x004B3F9B BYTE_EXACT
// symbol    ??_ESocialWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3SocialWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004B3F50` into
//   ??_GSocialWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
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

// The process-wide social window, 0x008A6270 in the image - REAL STORAGE, not
// the `Win *const TutWinSocWindow` binding that named terranx.exe data (see
// tutwin.h). The image's dynamic initializer at 0x004AE9B0 runs the real
// SocialWin constructor against it and registers the deleting destructor
// with atexit; here the same recovered constructor runs through this
// build's own startup and the destructor closes it at exit.
SocialWin TutWinSocWindow;  // 0x008A6270

// ORIGINAL: 0x004B1B30 ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z 0x004B1B30-0x004B1B70 BYTE_EXACT
// size      64 bytes
// prototype void (__thiscall ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z)(SocialWin* this, LPCSTR, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240 0x005F83D0
// 0x004B1B30  ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z  ->  ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z
// LEVER: homed from 004b1b30.cpp. Idiomatic base calls (client_to_screen and
// set_bubble_text are Win's, at SocialWin's own front) and the four RECT
// stores spelled as one initialiser replaced two casts of `this`.
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void SocialWin::bubble_me_elmo2(char *text, int x, int y) {
    client_to_screen(&x, &y);
    RECT rect = { x, y, x, y };
    set_bubble_text(text, &rect);
}

/*
 * Fixed-slot bindings the homed bodies arrived carrying, named from evidence.
 * Each is an `int *const` at file scope, which folds to the bare immediate, so
 * a rename moves no byte; the cast to the real type stays at the point of use.
 *
 * StringUpperSlot    0x0066931C - game.h already documents this address as the
 *                    CharUpper entry of a function-pointer table, and names the
 *                    accessor `string_upper_slot()`.
 * Panel styles now borrow shared mutable InterfaceBoxStyles elements 3/6;
 * their original starts 0x78D5A0/0x78D618 are no longer address aliases.
 * InterfaceBuffer    0x007AEC64, the shared full-screen drawing Buffer. 55
 *                    archive sites cast it to `Buffer *` and draw through it -
 *                    DesignWin::draw_info, Datalink, DiploPop::start, ReportIf,
 *                    this file. 0x009B7490 is palette.h's ScreenBuffer, a
 *                    different object.
 * InterfaceClipRect  0x007AEC94 = InterfaceBuffer + 0x30, the RECT every one of
 *                    those bodies hands to Buffer::set_clip - the buffer's own
 *                    clip rectangle, not an unrelated global.
 * SpaceText          0x00682820 - image .data reads " " (one space). technology
 *                    .cpp already names it in prose for the same reason.
 * PercentText        0x00682EA0 - image .data reads "%".
 * SocialEffectTable  0x0094B024 - indexed [category * 53 + row * 11 + effect]
 *                    with MaxSocialEffectNum == 11; a negative entry marks an
 *                    effect the faction cannot take.
 * SocialEffectRow    0x0094B2CC - indexed [effect] the same way, and the walk
 *                    in draw_psych_alloc steps it three entries at a time.
 * SocialEffectRowEnd SocialEffectRow + 3 - the walk's own end sentinel.
 * SocBaseIdProbe / SocBaseIdCompare - 0x009A6568 / 0x009A6574. Both are an int
 *                    base id read the same way: `>= 0`, then
 *                    SpyingBaseFactionBytes[id * SpyingBaseStride] compared to
 *                    the faction, i.e. "does this base belong to us". immune()
 *                    probes one and compares the other; no catalogue name.
 * SocialAllocTable   0x0096CD04 - per-faction, stride 2099 like the other
 *                    faction social arrays in SocialWin::exit; allocate_energy
 *                    reads the allocation it scales by 10 for display.
 */
static int *const StringUpperSlot = (int *)0x0066931C;
static int *const SocialEffectTable = (int *)0x0094B024;
static int *const SocialEffectRow = (int *)0x0094B2CC;
static int *const SocialEffectRowEnd = (int *)0x0094B2D8;
static int *const SocBaseIdProbe = (int *)0x009A6568;
static int *const SocBaseIdCompare = (int *)0x009A6574;

// ORIGINAL: 0x004AF310 ?draw_social@SocialWin@@QAEXH@Z 0x004AF310-0x004AF8BC FILE
// TRIED: transcribed directly from the disassembly (Ghidra's own decompilation agreed on the control flow and formulas once its confusing overlapping-stack-slot local numbering was resolved against the raw ebp-relative offsets) - the header/derived RECT pair, the per-row avail==0 header-only path, the count-then- draw two-pass loop over the fixed 11-entry table at 0x94b2cc (index 3 excluded), and the `this!=0 ? obj : 0` guard before each Sprite::draw. Draw receivers are the big/small Sprite sub-objects themselves (`bigIcon->draw(buffer, ...)`), not the Buffer - an early draft called the wrong receiver and was fixed before landing. MISMATCH #0 (`push`/`sub` prologue) on a 1452-byte function with a deeply nested sign-magnitude classification loop - not chased to byte-exactness, this is a structural transcription.
// working copy - scaffold materialised by --work
// size      1452 bytes
// prototype void (__thiscall ?draw_social@SocialWin@@QAEXH@Z)(SocialWin* this, int socCategory)
// callers   2   call targets   12
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B4730 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005E4B4A 0x005FD2B0 0x006169A0 0x006453E0 0x00645470
// indirect  0x004AF409


void SocialWin::draw_social(int a1) {
    // Reach fields by offset - the class is deliberately empty.
    char *self = reinterpret_cast<char *>(this);
    Buffer *buffer = reinterpret_cast<Buffer *>(self + 0x444);
    int faction = *reinterpret_cast<int *>(self + 0xcf4);

    int *headerSlot = reinterpret_cast<int *>((a1 + 0x1f11) * 0x10 + (int)this);
    RECT rect1;
    rect1.left = headerSlot[0];
    rect1.top = headerSlot[1];
    rect1.right = headerSlot[2];
    rect1.bottom = headerSlot[3];
    buffer->set_clip(&rect1);
    buffer->box_sprite(&rect1, &InterfaceBoxStyles[3]);

    RECT rect2;
    rect2.left = rect1.left + 3;
    rect2.top = rect1.top + 3;
    rect2.right = rect1.right - 3;
    rect2.bottom = rect1.top + 3 + 0x14;
    buffer->box_sprite(&rect2, &InterfaceBoxStyles[6]);

    StringTemp[0] = 0;
    int nameKey = *(int *)&SocialCategories[a1 * 0x35];
    char *name = StringTable.get(nameKey);
    strcat(StringTemp, name);
    buffer->set_text_color(0x9f, -1, 1, 1);
    buffer->set_font(reinterpret_cast<Font *>(self + 0x1ee68), (Font *)0, (Font *)0, (Font *)0);
    CharUpperA(StringTemp);
    char *heading = StringTemp;
    if (heading != 0) {
        unsigned int len = strlen(heading);
        buffer->write_cent_l(heading, &rect2, len);
    }

    for (int row = 0; row < 4; row++) {
        int *slot = reinterpret_cast<int *>((row + 0x1f15 + a1 * 4) * 0x10 + (int)this);
        if (society_avail(a1, row, faction) == 0) {
            RECT hdr;
            hdr.left = slot[0];
            hdr.top = slot[1];
            hdr.right = slot[2];
            hdr.bottom = slot[3];
            buffer->box_sprite(&hdr, &InterfaceBoxStyles[6]);
            hdr.top += 0x16;
            hdr.bottom += 0x16;
            buffer->box_sprite(&hdr, &InterfaceBoxStyles[6]);
            continue;
        }

        RECT body;
        body.left = slot[0];
        body.top = slot[1] + 0x16;
        body.right = slot[2];
        body.bottom = slot[3] + 0x16;
        buffer->box_sprite(&body, &InterfaceBoxStyles[6]);

        int countNonzero = 0;
        int countBig = 0;
        int idx = 0;
        int *p = SocialEffectRow;
        while (p != SocialEffectRowEnd) {
            if (p != SocialEffectRow + 3) {
                bool skip = false;
                if (*SocBaseIdCompare >= 0 &&
                    (unsigned int)SpyingBaseFactionBytes[*SocBaseIdCompare * 0x134] == (unsigned int)faction) {
                    if (a1 == 2) {
                        if (row == 1) {
                            if (SocialEffectTable[a1 * 0x35 + idx + row * 0xb] < 0) {
                                skip = true;
                            }
                        }
                    } else if (a1 == 3 && row == 3) {
                        if (SocialEffectTable[a1 * 0x35 + idx + row * 0xb] < 0) {
                            skip = true;
                        }
                    }
                }
                if (!skip) {
                    if (!((*SocBaseIdProbe < 0) ||
                          ((unsigned int)SpyingBaseFactionBytes[*SocBaseIdProbe * 0x134] != (unsigned int)faction) ||
                          (a1 != 3) ||
                          ((row != 1) || (*p >= 0)))) {
                        skip = true;
                    }
                }
                if (!skip) {
                    int v = SocialEffectTable[a1 * 0x35 + idx + row * 0xb];
                    int mag = (v < 0) ? -v : v;
                    countNonzero += (mag != 0) ? 1 : 0;
                    countBig += (v < 0 ? -v : v) > 1 ? 1 : 0;
                }
            }
            idx++;
            p++;
        }
        if (countNonzero < 1) {
            countNonzero = 1;
        }

        int drawX = (body.right - (*reinterpret_cast<int *>(self + 0x2e30) + 1) * countNonzero -
                     *reinterpret_cast<int *>(self + 0x2c70) * countBig - body.left) / 2 + body.left;
        int drawYBase = (body.bottom - *reinterpret_cast<int *>(self + 0x2e30) - body.top) / 2 + body.top;

        for (int phase = 0; phase < 3; phase++) {
            int drawX2 = drawX;
            int idx2 = 0;
            int *p2 = SocialEffectRow;
            while (p2 != SocialEffectRowEnd) {
                if (p2 != SocialEffectRow + 3) {
                    bool skip = false;
                    if (*SocBaseIdCompare >= 0 &&
                        (unsigned int)SpyingBaseFactionBytes[*SocBaseIdCompare * 0x134] == (unsigned int)faction) {
                        if (a1 == 2) {
                            if (row == 1) {
                                if (SocialEffectTable[a1 * 0x35 + idx2 + row * 0xb] < 0) {
                                    skip = true;
                                }
                            }
                        } else if (a1 == 3 && row == 3) {
                            if (SocialEffectTable[a1 * 0x35 + idx2 + row * 0xb] < 0) {
                                skip = true;
                            }
                        }
                    }
                    if (!skip) {
                        if (!((*SocBaseIdProbe < 0) ||
                              ((unsigned int)SpyingBaseFactionBytes[*SocBaseIdProbe * 0x134] != (unsigned int)faction) ||
                              (a1 != 3) ||
                              ((row != 1) || (*p2 >= 0)))) {
                            skip = true;
                        }
                    }
                    if (!skip) {
                        int v = SocialEffectTable[a1 * 0x35 + idx2 + row * 0xb];
                        int mag = (v < 0) ? -v : v;
                        bool draw = false;
                        if (phase == 0) {
                            draw = (mag > 1);
                        } else if (phase == 1) {
                            draw = (v == 1);
                        } else {
                            draw = (v < 0);
                        }
                        if (draw) {
                            Buffer *bufOrNull = (this != 0) ? buffer : (Buffer *)0;
                            if (mag > 1) {
                                char *bigIcon = self + 0x2bb0 + ((v < 1 ? 1 : 0) + mag * 2) * 0x2c;
                                Sprite *bigSprite = reinterpret_cast<Sprite *>(bigIcon);
                                unsigned char iconByte = *reinterpret_cast<uint8_t *>(bigIcon + 8);
                                int y = body.bottom - *reinterpret_cast<int *>(bigIcon + 0x1c);
                                bigSprite->draw(bufOrNull, iconByte, drawX2, y, 1, 1);
                                drawX2 = drawX2 + 1 + *reinterpret_cast<int *>(self + 0x2c70);
                            }
                            int *p2adj = p2;
                            if (p2 > SocialEffectRow + 10) {
                                p2adj = p2 - 9;
                            }
                            int smallIdx = (int)(p2adj - SocialEffectRow) + (v > 0 ? 1 : 0) + 1;
                            char *smallIcon = self + smallIdx * 0x2c + 0x2f20;
                            Sprite *smallSprite = reinterpret_cast<Sprite *>(smallIcon);
                            unsigned char icon = *reinterpret_cast<uint8_t *>(smallIcon + 8);
                            smallSprite->draw(bufOrNull, icon, drawX2, drawYBase, 1, 1);
                            drawX2 = drawX2 + 1 + *reinterpret_cast<int *>(self + 0x2e30);
                        }
                    }
                }
                idx2++;
                p2++;
            }
        }
    }

    reinterpret_cast<GraphicWin *>(this)->soft_update(reinterpret_cast<RECT *>(headerSlot));
    do_sound();
}

// Fixed-slot bindings carried from 004b0d50.cpp
static int *const SpaceText = (int *)0x00682820;
static int *const PercentText = (int *)0x00682EA0;
static int *const InterfaceBuffer = (int *)0x007AEC64;
static int *const InterfaceClipRect = (int *)0x007AEC94;
static int *const SocialAllocTable = (int *)0x0096CD04;

typedef char *(__cdecl *ItoaFn)(int, char *, int);

typedef void (__cdecl *SoundHookFn)(char *);

// ORIGINAL: 0x004B0D50 ?draw_psych_alloc@SocialWin@@QAEXXZ 0x004B0D50-0x004B0F92 FILE
// size      578 bytes
// prototype void (__thiscall ?draw_psych_alloc@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   1   call targets   13
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D5930 0x005D8000 0x005DA860 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005E3203 0x005FD2B0 0x006169A0 0x006453E0 0x00645470 0x0064FC88
// indirect  0x004B0E7F
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004B0D50
// measured tier  NO_COMPILE
// refusal        u004b0d50.cpp(212) : error C2079: 'buffer_' uses undefined class 'Buffer' u004b0d50.cpp(343) : error C2079: 'buffer_' uses undefined class 'Buffer'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004b0d50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_psych_alloc@SocialWin@@QAEXXZ  at 0x004B0D50  (578 bytes)
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
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_psych_alloc@SocialWin@@QAEXXZ  at 0x004B0D50  (578 bytes)
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


void SocialWin::draw_psych_alloc() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(InterfaceBuffer);

    RECT *update_rect = reinterpret_cast<RECT *>(self + 0x1f280);
    buf->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));

    RECT local_rect = *update_rect;
    buf->box_sprite(&local_rect, &InterfaceBoxStyles[6]);

    local_rect.left += 3;
    local_rect.top += 3;
    local_rect.right -= 3;
    local_rect.bottom -= 3;

    RECT box_rect;
    box_rect.left = local_rect.left + 0x17;
    box_rect.top = local_rect.top;
    box_rect.right = local_rect.right - 0x17;
    box_rect.bottom = local_rect.top + 0x14;
    buf->box(&box_rect, 0xbc, 0xbc);

    int idx = *reinterpret_cast<int *>(self + 0xcf4);
    StringTemp[0] = 0;
    int allocation = *(SocialAllocTable + idx * 2099);

    Strings *strings = &StringTable;
    strcat(StringTemp, strings->get(*(int *)((char *)Labels.strings_ptr + 0x4f8)));
    strcat(StringTemp, reinterpret_cast<char *>(SpaceText));

    char numbuf[80];
    reinterpret_cast<ItoaFn>(_itoa)(allocation * 10, numbuf, 10);
    strcat(StringTemp, numbuf);
    strcat(StringTemp, reinterpret_cast<char *>(PercentText));

    (*reinterpret_cast<SoundHookFn *>(StringUpperSlot))(StringTemp);

    buf->set_font(reinterpret_cast<Font *>(self + 0x1ee68), 0, 0, 0);
    buf->set_text_color(0x9f, -1, 1, 1);

    char *buf_str = StringTemp;
    if (buf_str) {
        int len = strlen(buf_str);
        buf->write_cent_l(buf_str, &box_rect, len);
    }

    int row_height = local_rect.bottom - *reinterpret_cast<int *>(self + 0x3dac);
    int start_x = local_rect.left + *reinterpret_cast<int *>(self + 0x3da8);
    int avail_width = local_rect.right - start_x;
    int scaled = (avail_width * allocation) / 5;
    int end_x = start_x + scaled;

    int adjusted_end_x = box_rect.right;
    if (end_x < local_rect.right) {
        adjusted_end_x = end_x + (allocation != 0 ? 1 : 0);
        int remaining_w = local_rect.right - end_x;
        int remaining_h = local_rect.bottom - row_height;
        reinterpret_cast<Buffer *>(self + 0x5510)->tile(
            buf, end_x, row_height, end_x, row_height, remaining_w, remaining_h);
    }

    if (start_x < adjusted_end_x) {
        int width2 = adjusted_end_x - start_x;
        int height2 = box_rect.bottom - row_height;
        reinterpret_cast<Buffer *>(self + 0x5a98)->tile(
            buf, start_x, row_height, start_x, row_height, width2, height2);
    }

    MainInterfaceGlobal.soft_update(update_rect);
    do_sound();
}

// ORIGINAL: 0x004B1FF0 ?allocate_energy@SocialWin@@QAEXHH@Z 0x004B1FF0-0x004B211C FILE
// size      300 bytes
// prototype void (__thiscall ?allocate_energy@SocialWin@@QAEXHH@Z)(SocialWin* this, int energyAllocID, int energyAllocSet)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00445440
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004B1FF0
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004b1ff0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?allocate_energy@SocialWin@@QAEXHH@Z  at 0x004B1FF0  (300 bytes)
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


void SocialWin::allocate_energy(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int limit = energy_limit(*reinterpret_cast<int *>(self + 0xCF4));
    *reinterpret_cast<int *>(self + 0x2C50 + a1 * 4) = a2;

    while (*reinterpret_cast<int *>(self + 0x2C58) +
               *reinterpret_cast<int *>(self + 0x2C50) +
               *reinterpret_cast<int *>(self + 0x2C54) <
           10) {
        int idx = a1 + 1;
        int attempts = 0;
        do {
            int candidate = idx % 3;
            if (a1 == candidate ||
                (*reinterpret_cast<int *>(self + 0x1780 + candidate * 0xA2C) == 0 &&
                 *reinterpret_cast<int *>(self + 0x2C50 + candidate * 4) < limit)) {
                ++*reinterpret_cast<int *>(self + 0x2C50 + candidate * 4);
                break;
            }
            ++attempts;
            ++idx;
        } while (attempts < 3);
    }

    while (*reinterpret_cast<int *>(self + 0x2C58) +
               *reinterpret_cast<int *>(self + 0x2C50) +
               *reinterpret_cast<int *>(self + 0x2C54) >
           10) {
        int idx = a1 + 1;
        int attempts = 0;
        do {
            int candidate = idx % 3;
            if (a1 == candidate ||
                (*reinterpret_cast<int *>(self + 0x1780 + candidate * 0xA2C) == 0 &&
                 *reinterpret_cast<int *>(self + 0x2C50 + candidate * 4) != 0)) {
                --*reinterpret_cast<int *>(self + 0x2C50 + candidate * 4);
                break;
            }
            ++attempts;
            ++idx;
        } while (attempts < 3);
    }
}
