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

/*
Purpose: The interface-mode entry point: put the battle up, or take it down.
// ORIGINAL: 0x00422CC0 ?on_status@BattleWin@@QAEXH@Z 0x00422CC0-0x00422DB9
// symbol    ?on_status@BattleWin@@UAEXH@Z
// size      249 bytes
// prototype void (__thiscall ?on_status@BattleWin@@QAEXH@Z)(BattleWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8200 0x0045C290 0x0045D440 0x0045D2A0 0x0045BE80 0x005D5890 0x0045D970
// indirect  0x00422D04 0x00422D13 0x00422D23 0x00422D32 0x00422D72 0x00422D7F 0x00422D8D 0x00422D9A
PENDING BODY - every instruction is read and the shape is plain: mode nonzero
   shows the battle and four window globals, mode zero hides the same four in
   reverse and pulls the interface alert back down. What blocks it is NAMING,
   not structure: the four objects it shows and hides (0x007B1804, 0x007B2350,
   0x007B39E8, 0x007B4534) are vptr-bearing Win-derived globals nothing in the
   tree names yet - each is reached as `mov edx,[g] / mov ecx,g / call
   [edx+4]`, i.e. show(0) and hide() through their own vtables - and the three
   arguments of the 0x005D8200 call at 0x00422CC E read 0x007AECE8 and
   0x007AF5B0, two more unnamed globals. None of the seven is bound anywhere
   in src/. HANDOFF: name the seven and this body spells directly, with
   vtable_method for the four dispatch pairs and qualified
   `MainInterfaceGlobal.set_alert` / `set_date` / `soft_update` for the rest.
*/
void BattleWin::on_status(int mode) {
    typedef void(__fastcall *pending)(BattleWin *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00422CC0))(
        this, nullptr, mode);
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

// PENDING 0x00422E30 ?on_iface_left_down@BattleWin@@QAEXHH@Z 0x00422E30-0x00422E64
// symbol    ?on_iface_left_down@BattleWin@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_iface_left_down@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005169F0 0x005FAB00
// PENDING BODY - the shape is one line and a guard, exactly as
// MainInterface::on_iface_left_down's: `spot.check(a1, a2, &a2, 0)` and on a
// hit of 5, Console::end_my_turn. The receiver is the SAME Spot MainInterface
// reaches its own member through - 0x007D3854 is front+0x25034, spot_'s
// address - and there is no public path to it: spot_ is private in
// maininterface.h, so `MainInterfaceGlobal.spot_.check(...)` does not compile
// from here. HANDOFF to maininterface.h: make spot_ public (or add an
// accessor, which would cost this body a call the image does not make) and
// this becomes
//   `int result = MainInterfaceGlobal.spot_.check(a1, a2, &a2, 0);
//    if (result != -1 && a2 == 5) ConsoleGlobal.end_my_turn();`
// Console::end_my_turn is already declared (console.h) and ConsoleGlobal is
// the object at 0x009156B0.
void BattleWin::on_iface_left_down(int x_coord, int y_coord) {
    typedef void(__fastcall *pending)(BattleWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00422E30))(
        this, nullptr, x_coord, y_coord);
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
// symbol    ?vslot_13@BattleWin@@UAEXH@Z
// size      26 bytes
// prototype void (__thiscall ?on_iface_button_clicked@BattleWin@@QAEXH@Z)(BattleWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045D170 0x0045D380
Status: Complete
*/
// LEVER: the second call is a DIRECT `call 0x45d170` in the image, so it stays
//   qualified - `MainInterface::on_iface_button_clicked` is deliberately NOT
//   the interface's slot 13 (see maininterface.h), and naming it as a
//   qualification is what keeps this a rel32 rather than a dispatch.
void BattleWin::vslot_13(int button_id) {
    reinterpret_cast<SubInterface*>(this)->release_iface_mode();
    reinterpret_cast<MainInterface*>(0x7af234)->MainInterface::on_iface_button_clicked(button_id);
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
