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

const void *const SocialWinSpriteCtor = (const void *)0x005E37E0;
const void *const SocialWinSpriteDtor = (const void *)0x00406850;
const void *const SocialWinFlatButtonCtor = (const void *)0x00607CF0;
const void *const SocialWinFlatButtonDtor = (const void *)0x00406880;
const void *const SocialWinCheckButtonCtor = (const void *)0x004B3EC0;
const void *const SocialWinCheckButtonDtor = (const void *)0x00633750;
const void *const SocialWinBufferCtor = (const void *)0x005D7210;
const void *const SocialWinBufferDtor = (const void *)0x005D7410;
const void *const SocialWinButtonGroupCtor = (const void *)0x0062B7C0;
const void *const SocialWinButtonGroupDtor = (const void *)0x004325B0;

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
// ORIGINAL: 0x004AE9E0 ??0SocialWin@@QAE@XZ 0x004AE9E0-0x004AEBE7;0x006598A0-0x00659A0A
// RULED-OUT: register allocation - the SEH prologue agrees (7/7) then the
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
    GraphicWin::construct();
    char *const self = reinterpret_cast<char *>(this);

    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0xA14 / 4] = SocialWinSubInterfaceVtable;

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
    VectorCtorIterator(self + 0x2C60, 0x2C, 0xA, SocialWinSpriteCtor, SocialWinSpriteDtor);
    VectorCtorIterator(self + 0x2E18, 0x2C, 0x5A, SocialWinSpriteCtor, SocialWinSpriteDtor);
    VectorCtorIterator(self + 0x3D90, 0x2C, 2, SocialWinSpriteCtor, SocialWinSpriteDtor);
    VectorCtorIterator(self + 0x3DE8, 0x2C, 6, SocialWinSpriteCtor, SocialWinSpriteDtor);
    VectorCtorIterator(self + 0x3EF0, 0x588, 6, SocialWinBufferCtor, SocialWinBufferDtor);

    object[0x000 / 4] = SocialWinPrimaryVtable;
    object[0x444 / 4] = SocialWinBufferVtable;
    object[0xA14 / 4] = SocialWinSubInterfaceVtable2;
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

void __fastcall social_win_unk2_redirect(SocialWin *self, void *) {
    self->UNK2();
}

void __fastcall social_win_unk3_redirect(SocialWin *self, void *) {
    self->UNK3();
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

void __fastcall social_win_on_iface_left_down_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
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

void __fastcall social_win_on_iface_right_down_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
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

void __fastcall social_win_on_iface_selected_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
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

void __fastcall social_win_on_iface_left_double_click_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
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

void __fastcall social_win_on_iface_right_double_click_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
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

void __fastcall social_win_on_iface_button_toggled_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
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

void __fastcall social_win_on_iface_group_clicked_redirect(SocialWin *self, void *, int a1, int a2, int a3) {
    self->on_iface_group_clicked(a1, a2, a3);
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

void __fastcall social_win_on_iface_mouse_move_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
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

void __fastcall social_win_on_iface_mouse_leave_redirect(SocialWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

/*
// ORIGINAL: 0x004B2690 ?show@SocialWin@@QAEXH@Z 0x004B2690-0x004B26AB BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?show@SocialWin@@QAEXH@Z)(SocialWin* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void SocialWin::show(int a1) {
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        SubInterface::set_iface_mode();
    }
}

/*
// ORIGINAL: 0x004B26B0 ?hide@SocialWin@@QAEXXZ 0x004B26B0-0x004B26C9 BYTE_EXACT
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
