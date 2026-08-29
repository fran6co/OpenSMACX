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

void __fastcall SocialWinFlatButtonCtor(void *self) { reinterpret_cast<FlatButton *>(self)->FlatButton::FlatButton(); }
void __fastcall SocialWinFlatButtonDtor(void *self) { reinterpret_cast<FlatButton *>(self)->~FlatButton(); }
void __fastcall SocialWinCheckButtonCtor(void *self) { reinterpret_cast<CheckButton *>(self)->CheckButton::CheckButton(); }
void __fastcall SocialWinCheckButtonDtor(void *self) { reinterpret_cast<CheckButton *>(self)->~CheckButton(); }
void __fastcall SocialWinButtonGroupCtor(void *self) { reinterpret_cast<ButtonGroup *>(self)->ButtonGroup::ButtonGroup(); }
void __fastcall SocialWinButtonGroupDtor(void *self) { reinterpret_cast<ButtonGroup *>(self)->~ButtonGroup(); }

const uint32_t SocialWinSubInterfaceVtable = 0x0066A6E4;
const uint32_t SocialWinPrimaryVtable = 0x0066DAA4;
const uint32_t SocialWinBufferVtable = 0x0066DA9C;
const uint32_t SocialWinSubInterfaceVtable2 = 0x0066DA50;

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
