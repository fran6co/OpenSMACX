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
#include "councwin.h"
#include "subinterface.h"
#include "worldwin.h"
#include "menu.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00426EB0 ?on_group_clicked@CouncWin@@QAEXHH@Z 0x00426EB0-0x00426EB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_group_clicked@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_group_clicked(int, int) {
}

void __fastcall counc_win_on_group_clicked_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_group_clicked(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00426F80 ?on_button_passover@CouncWin@@QAEXHH@Z 0x00426F80-0x00426F83 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_button_passover@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_button_passover(int, int) {
}

void __fastcall counc_win_on_button_passover_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_button_passover(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427060 ?on_iface_left_click@CouncWin@@QAEXHH@Z 0x00427060-0x00427063 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_click(int, int) {
}

void __fastcall counc_win_on_iface_left_click_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427070 ?on_iface_right_click@CouncWin@@QAEXHH@Z 0x00427070-0x00427073 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_click(int, int) {
}

void __fastcall counc_win_on_iface_right_click_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_right_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427080 ?on_iface_left_down@CouncWin@@QAEXHH@Z 0x00427080-0x00427083 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_down(int, int) {
}

void __fastcall counc_win_on_iface_left_down_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427090 ?on_iface_right_down@CouncWin@@QAEXHH@Z 0x00427090-0x00427093 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_down(int, int) {
}

void __fastcall counc_win_on_iface_right_down_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270A0 ?on_iface_mouse_move@CouncWin@@QAEXHH@Z 0x004270A0-0x004270A3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_mouse_move(int, int) {
}

void __fastcall counc_win_on_iface_mouse_move_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270B0 ?on_iface_mouse_leave@CouncWin@@QAEXHH@Z 0x004270B0-0x004270B3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_mouse_leave(int, int) {
}

void __fastcall counc_win_on_iface_mouse_leave_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270C0 ?on_iface_selected@CouncWin@@QAEXHH@Z 0x004270C0-0x004270C3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_selected(int, int) {
}

void __fastcall counc_win_on_iface_selected_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270D0 ?on_iface_left_double_click@CouncWin@@QAEXHH@Z 0x004270D0-0x004270D3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_left_double_click(int, int) {
}

void __fastcall counc_win_on_iface_left_double_click_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004270E0 ?on_iface_right_double_click@CouncWin@@QAEXHH@Z 0x004270E0-0x004270E3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_right_double_click(int, int) {
}

void __fastcall counc_win_on_iface_right_double_click_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427110 ?on_iface_button_toggled@CouncWin@@QAEXHH@Z 0x00427110-0x00427113 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_button_toggled(int, int) {
}

void __fastcall counc_win_on_iface_button_toggled_redirect(CouncWin *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00427120 ?on_iface_group_clicked@CouncWin@@QAEXHHH@Z 0x00427120-0x00427123 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_group_clicked@CouncWin@@QAEXHHH@Z)(CouncWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CouncWin::on_iface_group_clicked(int, int, int) {
}

void __fastcall counc_win_on_iface_group_clicked_redirect(CouncWin *self, void *, int a1, int a2, int a3) {
    self->on_iface_group_clicked(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x00426F90 ?on_button_text_draw@CouncWin@@QAEHPAUGraphicWin@@PADH@Z 0x00426F90-0x00426F95 BYTE_EXACT
// symbol    ?on_button_text_draw@CouncWin@@QAEHPAVGraphicWin@@PADH@Z
// size      5 bytes
// prototype int (__thiscall ?on_button_text_draw@CouncWin@@QAEHPAUGraphicWin@@PADH@Z)(CouncWin* this, GraphicWin*, int8*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int CouncWin::on_button_text_draw(::GraphicWin *, char *, int) {
    return 0;
}

int __fastcall counc_win_on_button_text_draw_redirect(
        CouncWin *self, void *, ::GraphicWin *a1, char *a2, int a3) {
    return self->on_button_text_draw(a1, a2, a3);
}

/*
// ORIGINAL: 0x00426EC0 ?on_mouse_leave@CouncWin@@QAEXHH@Z 0x00426EC0-0x00426EDE BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_mouse_leave@CouncWin@@QAEXHH@Z)(CouncWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00425DB0
Status: Complete
*/
void CouncWin::on_mouse_leave(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0xa34);
    if (v != -1) {
        *reinterpret_cast<int *>(self + 0xa34) = -1;
        draw_leader(v);
    }
}

/*
// ORIGINAL: 0x00425D90 ?draw_all_leaders@CouncWin@@QAEXXZ 0x00425D90-0x00425DAA BYTE_EXACT
// size      26 bytes
// prototype void (__thiscall ?draw_all_leaders@CouncWin@@QAEXXZ)(CouncWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00425DB0
Status: Complete
*/
void CouncWin::draw_all_leaders() {
    for (int i = 1; i < 8; i++) {
        draw_leader(i);
    }
}

/*
// ORIGINAL: 0x00427130 ?show@CouncWin@@QAEHH@Z 0x00427130-0x0042714B BYTE_EXACT
// size      27 bytes
// prototype int (__thiscall ?show@CouncWin@@QAEHH@Z)(CouncWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
int CouncWin::show(int a1) {
    int visible = reinterpret_cast<Win *>(this)->is_visible();
    if (visible) {
        return visible;
    }
    SubInterface::set_iface_mode();
}

// The seam and definition for the unrecovered draw_leader, at the end of the
// file so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness. Left alone, VC6 expands this
// one-line forwarder into draw_all_leaders (0x00425D90) and on_mouse_leave,
// which reported it as C4711 at councwin.cpp(215) and (225). Both callers are
// BYTE_EXACT today precisely because the original emits `call rel32` there, and
// an inlined `mov ecx / call [seam]` would break them. OPENSMACX_NOINLINE is
// the tree's spelling for this, but it expands to nothing on VC6 - the pragma
// is the only form cl 12.00 honours.


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00428550 ??1CouncWin@@QAE@XZ 0x00428550-0x00428614;0x00652C40-0x00652CB4
// body      src/councwin.h
// size      312 bytes
// prototype void (__thiscall ??1CouncWin@@QAE@XZ)(CouncWin* this)
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x005FA870 0x00618EE0 0x006456E4
*/
