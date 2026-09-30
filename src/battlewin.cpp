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
#include "battlewin.h"
#include "statuswin.h"
#include "maininterface.h"
#include "subinterface.h"
#include "time.h"
#include "console.h"   // ConsoleGlobal, the object at 0x009156B0
#include "mapwin.h"    // MainInterfaceGlobal, the object at 0x007AE820

// Two of on_status's callees are still original bodies the tree has not homed
// (0x0045D440 / 0x0045D970). councwin.cpp defines both as forwarders with
// external linkage; this TU reaches the same addresses through those.
void __cdecl slide_show(GraphicWin *window, int mode);   // 0x0045D440
void __cdecl slide_hide(GraphicWin *window, int mode);   // 0x0045D970

// Fixed-slot bindings the homed body arrived needing, named from evidence -
// see the block on on_status. InterfaceBuffer is the shared full-screen
// drawing Buffer, bound under the same name in maininterface.cpp,
// reportif.cpp and socialwin.cpp; InterfaceScreenRect is the RECT at
// MainInterface+0xD90 that BaseWin::init insets into its own rects.
static int *const InterfaceBuffer = (int *)0x007AEC64;
static RECT *const InterfaceScreenRect = (RECT *)0x007AF5B0;

/*
Purpose: The interface-mode entry point: put the battle up, or take it down.
// ORIGINAL: 0x00422CC0 ?on_status@BattleWin@@QAEXH@Z 0x00422CC0-0x00422DB9 BYTE_EXACT
// symbol    ?on_status@BattleWin@@UAEXH@Z
// size      249 bytes
// prototype void (__thiscall ?on_status@BattleWin@@QAEXH@Z)(BattleWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8200 0x0045C290 0x0045D440 0x0045D2A0 0x0045BE80 0x005D5890 0x0045D970
// indirect  0x00422D04 0x00422D13 0x00422D23 0x00422D32 0x00422D72 0x00422D7F 0x00422D8D 0x00422D9A
// LEVER: THE SEVEN "UNNAMED GLOBALS" WERE NOT SEVEN GLOBALS. All four
//   0x007B-prefixed addresses are elements of MainInterfaceGlobal's own
//   flat_button_ array, and the two 0x007AE-prefixed ones are read off the
//   interface's drawing Buffer - so the body needs no new binding at all,
//   only the two member paths, and both fold to the image's `mov ecx, imm32`:
//
//   0x007B1804  MainInterfaceGlobal.flat_button_[3]
//   0x007B2350  MainInterfaceGlobal.flat_button_[4]
//   0x007B39E8  MainInterfaceGlobal.flat_button_[6]
//   0x007B4534  MainInterfaceGlobal.flat_button_[7]
//       The strides give it away: 0x7B2350 - 0x7B1804 and 0x7B4534 - 0x7B39E8
//       are both exactly 0xB4C == sizeof(FlatButton), and front+0xE00 +
//       3*0xB4C / 4 / 6 / 7 land on all four addresses exactly. The
//       `mov edx,[g] / mov ecx,g / call [edx+4]` and `call [edx+8]` pairs are
//       Win's own show(int) and hide() - FlatButton is
//       BaseButton : GraphicWin : Win, so its vtable opens with Win's slots.
//       AlphaMenu::show / hide (0x00403680 / 0x00403830) reach flat_button_[3]
//       the same way, with the same `movl $1/$0, 0x7b22a0` beside it
//       (flat_button_[3]+0xA9C), which is the third reader that settles it.
//
//   0x007AEC64  the interface's Buffer - MainInterfaceGlobal's own
//       GraphicWin base subobject at front+0x444, already bound in this tree
//       as InterfaceBuffer (maininterface.cpp, reportif.cpp, socialwin.cpp).
//   0x007AECE8  that Buffer's `dib_.bmiHeader.biHeight` at Buffer+0x84, which
//       `Buffer::init` stores NEGATIVE for a top-down DIB (see the Dib block
//       in buffer.h) - hence this body's `neg`. slide_show (0x0045D440) and
//       its three siblings re-init a stack Buffer from the same pair,
//       `init(biWidth at +0x80, -biHeight at +0x84, 0, 0)`, which is what
//       confirms the two offsets are the DIB's own width and height.
//   0x007AF5B0  front+0xD90, the first dword of the RECT MainInterface keeps
//       beside it; BaseWin::init (0x0041F1AE) reads all four dwords and
//       insets them into its own rects (left+4/top+4/right-4/bottom-4), which
//       is the evidence that the four are one RECT. on_status hands its first
//       dword to Buffer::set_clip as the clip extent.
*/
// The two slide helpers are still original bodies (0x0045D440 / 0x0045D970);
// councwin.cpp declares and forwards them, and the definitions are
// external-linkage so this TU links against those.
void BattleWin::on_status(int mode) {
    if (mode != 0) {
        reinterpret_cast<Buffer *>(InterfaceBuffer)->set_clip(
            0, 0, InterfaceScreenRect->left,
            -reinterpret_cast<Buffer *>(InterfaceBuffer)
                 ->dib_.bmiHeader.biHeight);
        MainInterfaceGlobal.restore_back(0);
        MainInterfaceGlobal.flat_button_[3].show(0);
        MainInterfaceGlobal.flat_button_[4].show(0);
        MainInterfaceGlobal.flat_button_[6].show(0);
        MainInterfaceGlobal.flat_button_[7].show(0);
        slide_show(0, 2);
        MainInterfaceGlobal.set_alert(1);
        MainInterfaceGlobal.set_date(0);
        MainInterfaceGlobal.soft_update();
    } else {
        MainInterfaceGlobal.flat_button_[6].hide();
        MainInterfaceGlobal.flat_button_[7].hide();
        MainInterfaceGlobal.flat_button_[3].hide();
        MainInterfaceGlobal.flat_button_[4].hide();
        MainInterfaceGlobal.set_alert(0);
        slide_hide(0, 2);
    }
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E10 ?on_iface_left_click@BattleWin@@QAEXHH@Z 0x00422E10-0x00422E13 BYTE_EXACT
// symbol    ?on_iface_left_click@BattleWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_left_click(int, int) {
}

// ORIGINAL: 0x00422E30 ?on_iface_left_down@BattleWin@@QAEXHH@Z 0x00422E30-0x00422E64 BYTE_EXACT
// symbol    ?on_iface_left_down@BattleWin@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_iface_left_down@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005169F0 0x005FAB00
// LEVER: the receiver is the SAME Spot MainInterface reaches its own member
//   through - 0x007D3854 is front+0x25034, spot_'s address - so the whole body
//   is `MainInterfaceGlobal.spot_.check(a1, a2, &a2, 0)` and, on a hit of 5,
//   ConsoleGlobal.end_my_turn(). spot_ went public in maininterface.h for it;
//   an accessor would have cost a call the image does not make.
void BattleWin::on_iface_left_down(int x_coord, int y_coord) {
    // The image tests the two conditions against one shared epilogue - a
    // single `pop ebp / ret 8` both jumps reach - so this is one `&&`, not
    // two guard clauses. Spot::check writes the hit position back through
    // `&y_coord`, which is why the second test re-reads the parameter slot.
    if (MainInterfaceGlobal.spot_.check(x_coord, y_coord, &y_coord, 0) != -1
        && y_coord == 5) {
        ConsoleGlobal.end_my_turn();
    }
}



/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E20 ?on_iface_right_click@BattleWin@@QAEXHH@Z 0x00422E20-0x00422E23 BYTE_EXACT
// symbol    ?on_iface_right_click@BattleWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E70 ?on_iface_right_down@BattleWin@@QAEXHH@Z 0x00422E70-0x00422E73 BYTE_EXACT
// symbol    ?on_iface_right_down@BattleWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E80 ?on_iface_mouse_move@BattleWin@@QAEXHH@Z 0x00422E80-0x00422E83 BYTE_EXACT
// symbol    ?on_iface_mouse_move@BattleWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E90 ?on_iface_mouse_leave@BattleWin@@QAEXHH@Z 0x00422E90-0x00422E93 BYTE_EXACT
// symbol    ?on_iface_mouse_leave@BattleWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422EA0 ?on_iface_selected@BattleWin@@QAEXHH@Z 0x00422EA0-0x00422EA3 BYTE_EXACT
// symbol    ?on_iface_selected@BattleWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_selected(int, int) {
}


/*
// ORIGINAL: 0x00422DC0 ?on_iface_button_clicked@BattleWin@@QAEXH@Z 0x00422DC0-0x00422DDA BYTE_EXACT
// symbol    ?on_iface_button_clicked@BattleWin@@UAEXH@Z
// size      26 bytes
// prototype void (__thiscall ?on_iface_button_clicked@BattleWin@@QAEXH@Z)(BattleWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045D170 0x0045D380
Status: Complete
*/
// LEVER: the second call is a DIRECT `call 0x45d170` in the image, so it stays
//   qualified. Explicit C++ qualification keeps the direct call even though
//   MainInterface correctly implements the interface's virtual slot13.
void BattleWin::on_iface_button_clicked(int button_id) {
    reinterpret_cast<SubInterface*>(this)->release_iface_mode();
    MainInterfaceGlobal.MainInterface::on_iface_button_clicked(button_id);
}

/*
Purpose: The interface group click: only group 1 means anything, which
         releases the interface mode and hands the group to MainInterface.
// ORIGINAL: 0x00422DE0 ?on_iface_group_clicked@BattleWin@@QAEXHHH@Z 0x00422DE0-0x00422E06 BYTE_EXACT
// symbol    ?on_iface_group_clicked@BattleWin@@UAEXHHH@Z
// size      38 bytes
// prototype void (__thiscall ?on_iface_group_clicked@BattleWin@@QAEXHHH@Z)(BattleWin* this, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045CCB0 0x0045D380
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_group_clicked(int group, int x_coord,
                                       int y_coord) {
    if (group == 1) {
        SubInterface::release_iface_mode();
        reinterpret_cast<MainInterface *>(0x7af234)
            ->MainInterface::on_iface_group_clicked(1, x_coord, y_coord);
    }
}



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
// ORIGINAL: 0x00422EE0 ??0BattleWin@@QAE@XZ 0x00422EE0-0x00422EFB BYTE_EXACT
// body      src/battlewin.cpp
// size      27 bytes
// prototype void (__thiscall ??0BattleWin@@QAE@XZ)(BattleWin* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006161D0
*/
// THE BASE VTABLE FIRST, THE DERIVED ONE LAST, with the Time member at +8
// constructed in between - and all three are the compiler's now: the base is
// a real polymorphic interface, the Time a real member, so the body is empty
// and the two stores plus the out-of-line ??0Time call fall out of it.
BattleWin::BattleWin() {
}

/*
// ORIGINAL: 0x00422ED0 ??1BattleWin@@QAE@XZ 0x00422ED0-0x00422ED8 BYTE_EXACT
// symbol    ??1BattleWin@@QAE@XZ
// size      8 bytes
// prototype void (__thiscall ??1BattleWin@@QAE@XZ)(BattleWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// THE DESTRUCTOR IS THE COMPILER'S. Two instructions in the image - `add ecx,
// 8` and a tail `jmp` into ~Time - and no body in this file produces them:
// there is no `~BattleWin()` declaration in battlewin.h any more, and the
// implicitly declared destructor over the Time member is exactly those two
// instructions. TRIED 2026-09-03: a user-written `~BattleWin() {}` also emits
// the base vptr restore, `mov dword ptr [ecx], <vtable>`, which the image
// does not have - that store is the price of a user-declared destructor, not
// of the polymorphic base (probed on cl 12.00.8168: the implicit one over the
// same base and member emits nothing before the `add`). The earlier reading -
// that SubInterface's vftable must be hand-installed because a compiler-owned
// one would show up here - was the wrong half of that observation: the
// constructor's two stores prove the base IS compiler-owned, and only the
// destructor spelling was wrong. `((Time *)this)->stop()` in place of the
// member was also tried, 1 of 2.


// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
BattleWin g_BattleWin;  // 0x006EEED8
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// ===== atexit-registered timer callbacks (image teardown thunks) =====
/*
Purpose: ?timer_callback_daemon@BattleWin@@QAAXHH@Z - run 1 (ORIGINAL(s)->*teardown)() on fixed globals,
         unguarded. The last is a tail jump in the original, so its
         return goes straight to this function's caller.
// ORIGINAL: 0x00422EB0 ?timer_callback_daemon@BattleWin@@QAAXHH@Z 0x00422EB0-0x00422EBA BYTE_EXACT
// symbol    ?teardown_00422eb0@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_00422eb0() {
    sub_interface_global()->release_iface_mode();
}
/*
Purpose: ?timer_callback_daemon@BattleWin@@QAAXH@Z - run 1 (ORIGINAL(s)->*teardown)() on fixed globals,
         unguarded. The last is a tail jump in the original, so its
         return goes straight to this function's caller.
// ORIGINAL: 0x00422EC0 ?timer_callback_daemon@BattleWin@@QAAXH@Z 0x00422EC0-0x00422ECA BYTE_EXACT
// symbol    ?teardown_00422ec0@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_00422ec0() {
    sub_interface_global()->release_iface_mode();
}

// ORIGINAL: 0x00421B20 ?pulse_timer@BattleWin@@QAEXXZ 0x00421B20-0x00421B37 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The image's 0x00421B20 adds eight to the BattleWin receiver before calling Time::pulse at 0x006164D0, with callback 0x00422EC0, zero parameter, period 0x1388 and resolution 5.", "BattleWin's constructor and destructor establish a real Time member at +8; 0x00422EC0 is the existing source-owned teardown callback."], "reason": "Home the timer pulse through BattleWin's typed Time member and existing callback."}
void BattleWin::pulse_timer() {
    time_.pulse(reinterpret_cast<void (__cdecl *)(int)>(&teardown_00422ec0),
                0, 0x1388, 5);
}

// ORIGINAL: 0x00421B40 ?stop_timer@BattleWin@@QAEXXZ 0x00421B40-0x00421B48 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The image's 0x00421B40 adds eight to the receiver and tail-jumps to Time::stop.", "BattleWin's constructor and destructor establish a real Time member at +8."], "reason": "Home the timer stop through BattleWin's typed Time member."}
void BattleWin::stop_timer() {
    time_.stop();
}

// Original interface table 0x0066AA44 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@BattleWin@@UAEXHH@Z 0x00422CC0
void BattleWin::on_iface_scrolled(int, int) {}
// Original interface table 0x0066AA44 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@BattleWin@@UAEXHH@Z 0x00422CC0
void BattleWin::on_iface_scrolling(int, int) {}
// Original interface table 0x0066AA44 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@BattleWin@@UAEXHH@Z 0x00422CC0
void BattleWin::on_iface_left_up(int, int) {}
// Original interface table 0x0066AA44 slot 11 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_double_click@BattleWin@@UAEXHH@Z 0x00422CC0
void BattleWin::on_iface_left_double_click(int, int) {}
// Original interface table 0x0066AA44 slot 12 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_right_double_click@BattleWin@@UAEXHH@Z 0x00422CC0
void BattleWin::on_iface_right_double_click(int, int) {}

// Original interface table 0x0066AA44 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@BattleWin@@UAEHH@Z 0x00422CC0
int BattleWin::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066AA44 slot 15 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_button_toggled@BattleWin@@UAEXHH@Z 0x00422CC0
void BattleWin::on_iface_button_toggled(int, int) {}

// Original interface table 0x0066AA44 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@BattleWin@@UAEHPAVGraphicWin@@@Z 0x00422CC0
int BattleWin::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066AA44 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@BattleWin@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x00422CC0
int BattleWin::on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) {
    return 0;
}
