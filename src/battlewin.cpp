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
#include "interface_fonts.h"
#include "battlewin.h"
#include "statuswin.h"
#include "maininterface.h"
#include "subinterface.h"
#include "time.h"
#include "console.h"   // ConsoleGlobal, the object at 0x009156B0
#include "mapwin.h"    // MainInterfaceGlobal, the object at 0x007AE820

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00422C50 ?clear@BattleWin@@QAEXXZ 0x00422C50-0x00422CB5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x422C50-0x422CB5;src/recovered/units/00422c50.cpp:historical exact seed;src/battlewin.h:four contiguous native RECT arguments at30/40/50/60"], "reason": "Recover the original four-rectangle clear operation through the genuine BattleWin rectangle members and shared MainInterfaceGlobal canvas; preserve source and seed obligations."}
void BattleWin::clear() {
    MainInterfaceGlobal.box_sprite(&battle_rects_[0], &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.box_sprite(&battle_rects_[1], &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.box_sprite(&battle_rects_[2], &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.box_sprite(&battle_rects_[3], &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.redraw_complete();
    MainInterfaceGlobal.soft_update();
}

// Two of on_status's callees are still original bodies the tree has not homed
// (0x0045D440 / 0x0045D970). councwin.cpp defines both as forwarders with
// external linkage; this TU reaches the same addresses through those.
void __cdecl slide_show(GraphicWin *window, int mode);   // 0x0045D440
void __cdecl slide_hide(GraphicWin *window, int mode);   // 0x0045D970

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
    Buffer &interface_buffer = static_cast<Buffer &>(MainInterfaceGlobal);
    if (mode != 0) {
        interface_buffer.set_clip(
            0, 0, MainInterfaceGlobal.right_frame_rect().left,
            -interface_buffer.dib_.bmiHeader.biHeight);
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
    SubInterface::release_iface_mode();
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
        MainInterfaceGlobal.MainInterface::on_iface_group_clicked(
            1, x_coord, y_coord);
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

// The original fixed receiver at 0x006EEED8 is the source-owned BattleWin
// object above, whose primary SubInterface base begins at offset zero.
SubInterface *sub_interface_global() {
    return &g_BattleWin;
}
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
// symbol    ?teardown_00422ec0@@YAXH@Z
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_00422ec0(int) {
    sub_interface_global()->release_iface_mode();
}

// ORIGINAL: 0x00421B20 ?pulse_timer@BattleWin@@QAEXXZ 0x00421B20-0x00421B37 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The image's 0x00421B20 adds eight to the BattleWin receiver before calling Time::pulse at 0x006164D0, with callback 0x00422EC0, zero parameter, period 0x1388 and resolution 5.", "BattleWin's constructor and destructor establish a real Time member at +8; 0x00422EC0 is the existing source-owned teardown callback."], "reason": "Home the timer pulse through BattleWin's typed Time member and existing callback."}
void BattleWin::pulse_timer() {
    time_.pulse(&teardown_00422ec0, 0, 0x1388, 5);
}

// Source-owned receiver slot; original complete allocation/lifetime unverified.
// RECOVERY_OBJECT: v1 0x007AE818 ?BattleWinInitReceiver@@3PAVBattleWin@@A
BattleWin *BattleWinInitReceiver = nullptr;

// Superseded seed deleted; the seed path below is historical Git provenance.
// ORIGINAL: 0x004219E0 ?init@BattleWin@@QAEXXZ 0x004219E0-0x00421B1F
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 4219E0..421B1F complete instructions", "historical seed src/recovered/units/004219e0.cpp", "full owner/game/pending native overlay compiles; observed current symbols", "7AE818 zero initial DWORD; original full allocation/lifetime unverified"], "reason": "Complete genuine BattleWin initialization: source-owned singleton receiver, typed member rectangles, automatic Buffer lifetime and independent receiver slot; no executable-address dispatch."}
// kind      game
// symbol    ?init@BattleWin@@QAEXXZ
void BattleWin::init() {
    // The original constructs and tears down this temporary Buffer even though
    // it does not use it. Keep that lifetime visible to C++ and to unwind.
    Buffer scratch;

    const RECT &layout = MainInterfaceGlobal.main_layout_rect();
    const RECT &frame = MainInterfaceGlobal.main_frame_rect();

    field_70_ = frame;
    field_90_ = layout;

    const int left1 = layout.left + 4;
    const int top1 = layout.top + 4;
    const int right1 = (layout.right - left1 - 1) / 2 - 3 + left1;
    const int bottom1 = layout.bottom - 4;
    battle_rects_[0].left = left1;
    battle_rects_[0].top = top1;
    battle_rects_[0].right = right1;
    battle_rects_[0].bottom = bottom1;

    const int delta1 = right1 - left1 + 3;
    battle_rects_[1].left = left1 + delta1;
    battle_rects_[1].top = top1;
    battle_rects_[1].right = right1 + delta1;
    battle_rects_[1].bottom = bottom1;

    const int left2 = frame.left + 4;
    const int top2 = frame.top + 4;
    const int right2 = (frame.right - left2 - 1) / 2 - 3 + left2;
    const int bottom2 = frame.bottom - 4;
    battle_rects_[2].left = left2;
    battle_rects_[2].top = top2;
    battle_rects_[2].right = right2;
    battle_rects_[2].bottom = bottom2;

    field_4_ = 10;

    battle_rects_[3].left = right2 + 3;
    battle_rects_[3].top = top2;
    battle_rects_[3].right = frame.right - 4;
    battle_rects_[3].bottom = bottom2;

    BattleWinInitReceiver = this;
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

#include "stdafx.h"
#include "battlewin.h"
#include "buffer.h"
#include "console.h"
#include "alpha.h"
#include "faction.h"
#include "font.h"
#include "game.h"
#include "general.h"
#include "maininterface.h"
#include "mapwin.h"
#include "messagewin.h"
#include "sprite.h"
#include "strings.h"

#include "veh.h"

#include <stdlib.h>
#include <string.h>

// These six objects are already defined in sprite.cpp at the observed image
// addresses. sprite.h does not currently expose declarations for the family.
extern Sprite g_BATTLE_MIND_WORM_SPRITE;
extern Sprite g_BATTLE_ISLE_DEEP_SPRITE;
extern Sprite g_BATTLE_LOCUSTS_CHIRON_SPRITE;
extern Sprite g_BATTLE_FUNGAL_TOWER_SPRITE;
extern Sprite g_BATTLE_SPORE_LAUNCHER_SPRITE;
extern Sprite g_BATTLE_SEALURK_SPRITE;

// ORIGINAL: 0x00421B50 ?battle_report@BattleWin@@QAEXHHHHH@Z 0x00421B50-0x00422BFC;0x00652BA0-0x00652BCB
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00421b50.cpp:original hot and cold spans", "pinned terranx_original.exe SHA256:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_battlewin_battle_report_behavior.py", "recovery/tests/battlewin_battle_report_host_control.cpp"], "reason": "Recover the complete two-vehicle battle report through the real SubInterface owner, embedded Time, local Font lifetimes, MainInterface canvas, typed vehicle rules and inline battle modifier text rows."}
// TRIED: real `Font font_c4/font_9c/font_ec;` locals reproduce the 3-Font RAII/SEH shape and the early-return dtor cascade, but the "away from base" separator/style checks and the odds/ wrap text-building tail (0x422781-0x422f00) are approximated rather than a literal per-instruction transcription.
void BattleWin::battle_report(int first_vehicle_id, int second_vehicle_id,
                              int attack_odds, int defense_odds, int flags) {
    Font font_normal;
    Font font_small;
    Font font_detail;

    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 10) {
        return;
    }

    const bool wide_layout = WinScreenWidth != 0x320;
    const int panel_height = wide_layout ? 75 : 50;
    const int font_size = wide_layout ? 12 : 10;
    Buffer &canvas = static_cast<Buffer &>(MainInterfaceGlobal);

    for (int rect_index = 0; rect_index != 4; ++rect_index) {
        canvas.box_sprite(&battle_rects_[rect_index], &InterfaceBoxStyles[0]);
    }
    MainInterfaceGlobal.redraw_complete();
    MainInterfaceGlobal.soft_update();

    font_normal.init(DefaultFontFace, font_size, 0);
    font_small.init(DefaultFontFace, font_size, 2);
    font_detail.init(DefaultFontFace, font_size, 1);
    canvas.set_font(&font_normal, &font_detail, &font_small, 0);
    canvas.set_text_color(MessageWinTextColor, -1, 1, 1);

    Veh &first = Vehs[first_vehicle_id];
    Veh &second = Vehs[second_vehicle_id];
    VehPrototype &first_proto = VehPrototypes[first.proto_id];
    VehPrototype &second_proto = VehPrototypes[second.proto_id];
    const int first_faction = first.faction_id;
    const int second_faction = second.faction_id;

    const bool same_special_plan =
        first_proto.plan == 0x0B && second_proto.plan == 0x0B;
    bool alternate_combat_label = false;
    if (attack_odds >= 0 && defense_odds >= 0) {
        alternate_combat_label =
            Weapon[first_proto.weapon_id].offense_rating < 0 ||
            Armor[second_proto.armor_id].defense_rating < 0 ||
            first_faction == 0;
    }

    // Portrait area 0x50 belongs to the first argument, and 0x60 to the
    // second. Each sprite is centered independently in the native panel.
    const int portrait_vehicle_ids[2] = {first_vehicle_id, second_vehicle_id};
    const int portrait_factions[2] = {first_faction, second_faction};
    canvas.set_font(find_font(14, 1), 0, 0, 0);
    for (int side = 0; side != 2; ++side) {
        const int rect_slot = side + 2;
        RECT &portrait_rect = battle_rects_[rect_slot];
        Sprite *portrait;
        if (portrait_factions[side] != 0) {
            portrait = &FactionsArt[portrait_factions[side]].council_small;
        } else {
            const int portrait_proto_id =
                Vehs[portrait_vehicle_ids[side]].proto_id;
            switch (portrait_proto_id) {
            case 9:  portrait = &g_BATTLE_ISLE_DEEP_SPRITE; break;
            case 10: portrait = &g_BATTLE_LOCUSTS_CHIRON_SPRITE; break;
            case 14: portrait = &g_BATTLE_SEALURK_SPRITE; break;
            case 15: portrait = &g_BATTLE_FUNGAL_TOWER_SPRITE; break;
            case 19: portrait = &g_BATTLE_SPORE_LAUNCHER_SPRITE; break;
            default: portrait = &g_BATTLE_MIND_WORM_SPRITE; break;
            }
        }
        const int x = portrait_rect.left +
            ((portrait_rect.right - portrait_rect.left) - portrait->width()) / 2;
        const int y = portrait_rect.top - portrait_rect.left + x;
        portrait->draw(&canvas, portrait->transparent_index(), x, y, 1, 1);

        RECT name_rect;
        name_rect.left = portrait_rect.left + 3;
        name_rect.top = y + portrait->height() + 3;
        name_rect.right = portrait_rect.right - 3;
        name_rect.bottom = portrait_rect.bottom - 3;
        const int faction = portrait_factions[side];
        canvas.set_text_color(ChatItemIdTable[faction], ChatShadowColors[faction],
                              1, 1);
        canvas.wrap_cent(Players[faction].formal_name_faction, &name_rect);
    }

    RECT name_bounds;
    UnionRect(&name_bounds, &battle_rects_[2], &battle_rects_[3]);
    RECT report_inner[2];
    for (int panel = 0; panel != 2; ++panel) {
        report_inner[panel].left = battle_rects_[panel].left + 3;
        report_inner[panel].top = battle_rects_[panel].top + 3;
        report_inner[panel].right = battle_rects_[panel].right - 3;
        report_inner[panel].bottom = battle_rects_[panel].bottom - 3;
    }

    // Preserve the image's swap-and-repeat loop: each pass paints one input
    // vehicle and the second pass is reached by replacing the live id with
    // second_vehicle_id. Equal IDs naturally terminate after one pass.
    int current_vehicle_id = first_vehicle_id;
    int text_y = 0;
    int text_x = 0;
    int text_width = 0;
    do {
        const bool current_is_second = current_vehicle_id == second_vehicle_id;
        const RECT &native_panel = battle_rects_[current_is_second ? 1 : 0];
        const RECT &native_clip = report_inner[current_is_second ? 1 : 0];
        text_x = native_clip.left;
        text_y = native_clip.top;
        text_width = native_clip.right - native_clip.left;

        canvas.set_clip(const_cast<RECT *>(&native_clip));

        Veh &vehicle = Vehs[current_vehicle_id];
        VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
        const int faction = vehicle.faction_id;

        if (wide_layout) {
            const int icon_x = text_x +
                ((text_width / 2) - panel_height) / 2;
            veh_draw(&canvas, current_vehicle_id, icon_x, text_y,
                     0, 0, 0);

            const int initial_turn_count = prototype.plan == 0x0C
                ? 1 : range(prototype.reactor_id, 1, 100) * 10;
            const int remaining_turns =
                range(initial_turn_count - vehicle.year_end_lurking, 0, 9999);
            if (remaining_turns == 0) {
                canvas.set_font(&font_small, 0, 0, 0);
                canvas.set_text_color(0x6A, 0, 1, 1);
                int label_offset;
                if (prototype.plan == 0x0C) {
                    label_offset = 0x4A0;
                } else if (faction != 0) {
                    label_offset = 0x498;
                } else {
                    label_offset = 0x49C;
                }
                StringTemp[0] = '\0';
                strcat(StringTemp, label_get(label_offset / 4));
                canvas.write_cent_l(StringTemp, text_x,
                                    text_y + panel_height / 4,
                                    text_width / 2,
                                    static_cast<int>(strlen(StringTemp)));
            }
        }

        StringTemp[0] = '\0';
        if (faction != 0 && faction != ConsoleGlobal.faction_id_ &&
            Language == 0) {
            strcat(StringTemp, Players[faction].adj_name_faction);
            strcat(StringTemp, BufferJoinSpace);
        }
        strcat(StringTemp, prototype.veh_name);
        if (faction != 0 && faction != ConsoleGlobal.faction_id_ &&
            Language != 0) {
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, "(");
            GenderDefault = static_cast<int>(Players[faction].noun_gender);
            PluralityDefault = Players[faction].is_noun_plural;
            strcat(StringTemp, Players[faction].noun_faction);
            strcat(StringTemp, ")");
        }
        if ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
            char vehicle_number[40];
            strcat(StringTemp, "(");
            _itoa(current_vehicle_id, vehicle_number, 10);
            strcat(StringTemp, vehicle_number);
            strcat(StringTemp, ")");
        }

        const int faction_color_slot = current_is_second ? second_faction
                                                         : first_faction;
        const int panel_width = native_panel.right - native_panel.left;
        const int write_width = wide_layout ? text_width / 2 : text_width;
        const int row_y = text_y + (wide_layout ? panel_height : 4);
        canvas.set_font(find_font(wide_layout ? 16 : 12, 1), 0, 0, 0);
        canvas.set_text_color(ChatItemIdTable[faction_color_slot],
                              ChatShadowColors[faction_color_slot], 1, 1);
        int cursor_y = canvas.wrap_cent(StringTemp, text_x, row_y, write_width);

        canvas.set_font(&font_normal, 0, 0, 0);
        canvas.set_text_color(MessageWinTextColor, -1, 1, 1);
        StringTemp[0] = '\0';
        say_stats_3(StringTemp, vehicle.proto_id);
        const int stats_width = text_width / 2;
        const int stats_x = wide_layout ? text_x + text_width / 2 : text_x;
        const int stats_y = wide_layout ? text_y + 5 : cursor_y;
        cursor_y = canvas.wrap(StringTemp, stats_x, stats_y, stats_width);

        if (!same_special_plan) {
            StringTemp[0] = '\0';
            if (alternate_combat_label) {
                strcat(StringTemp, label_get(0x310 / 4));
            } else {
                const bool use_weapon_branch =
                    current_vehicle_id != second_vehicle_id ||
                    ((flags & 2) != 0 &&
                     Chassis[prototype.chassis_id].triad != 1);
                if (use_weapon_branch) {
                    strcat(StringTemp, StringTable.get(
                        Weapon[prototype.weapon_id].name));
                } else {
                    strcat(StringTemp, StringTable.get(
                        Armor[prototype.armor_id].name));
                }
            }

            if (attack_odds >= 0 && defense_odds >= 0) {
                strcat(StringTemp, ": ");
                const bool native_attack_line =
                    current_vehicle_id != second_vehicle_id ||
                    ((flags & 2) != 0 &&
                     Chassis[prototype.chassis_id].triad != 1);
                const int psi_triad =
                    Chassis[second_proto.chassis_id].triad;
                char detail_number[40];
                if (native_attack_line) {
                    if (alternate_combat_label) {
                        _itoa(static_cast<int>(Rules.psi_combat_ratio_atk[
                                  psi_triad]),
                              detail_number, 10);
                    } else {
                        _itoa(offense_proto(vehicle.proto_id, -1, 0),
                              detail_number, 10);
                    }
                } else {
                    if (alternate_combat_label) {
                        _itoa(static_cast<int>(Rules.psi_combat_ratio_def[
                                  psi_triad]),
                              detail_number, 10);
                    } else {
                        _itoa(armor_proto(vehicle.proto_id, -1, 0),
                              detail_number, 10);
                    }
                }
                strcat(StringTemp, detail_number);
            }
            cursor_y = canvas.wrap(StringTemp, stats_x, cursor_y, stats_width);
        }

        StringTemp[0] = '\0';
        say_morale(current_vehicle_id,
                   current_is_second ? first_faction : second_faction);
        strcat(StringTemp, ": ");
        const int side_index = current_is_second ? 1 : 0;
        const int scaled_morale = static_cast<int>(
            (static_cast<int>(VehBasicBattleMorale[side_index]) - 2) * 100);
        const int morale_percent =
            (scaled_morale + ((scaled_morale >> 31) & 7)) >> 3;
        if (morale_percent >= 0) strcat(StringTemp, "+");
        char morale_text[40];
        _itoa(morale_percent, morale_text, 10);
        strcat(StringTemp, morale_text);
        strcat(StringTemp, "%");
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, BufferJoinSpace);
        cursor_y = canvas.wrap(StringTemp, stats_x, cursor_y, stats_width);

        const int modifier_count = static_cast<int>(VehBattleModCount[side_index]);
        for (int modifier_index = 0; modifier_index < modifier_count;
             ++modifier_index) {
            StringTemp[0] = '\0';
            strcat(StringTemp, VehBattleDisplay[side_index * 4 + modifier_index]);
            strcat(StringTemp, BufferJoinSpace);
            const int modifier = VehBattleModifier[side_index * 4 + modifier_index];
            if (modifier >= 0) strcat(StringTemp, "+");
            char modifier_text[40];
            _itoa(modifier, modifier_text, 10);
            strcat(StringTemp, modifier_text);
            strcat(StringTemp, "%");
            cursor_y = canvas.wrap(StringTemp, stats_x, cursor_y, stats_width);
        }

        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x520 / 4));
        strcat(StringTemp, ": ");
        strcat(StringTemp, "{");
        const int packed_odds = current_vehicle_id == second_vehicle_id
            ? defense_odds : attack_odds;
        char whole_text[40];
        _itoa(packed_odds >> 8, whole_text, 10);
        strcat(StringTemp, whole_text);
        strcat(StringTemp, ".");
        const int fraction = ((packed_odds & 0xFF) * 100) >> 8;
        if (fraction <= 9) strcat(StringTemp, "0");
        char fraction_text[40];
        _itoa(fraction, fraction_text, 10);
        strcat(StringTemp, fraction_text);
        strcat(StringTemp, "}");
        cursor_y = canvas.wrap(StringTemp, stats_x, cursor_y, stats_width);

        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0x1E4 / 4));
        strcat(StringTemp, ": ");
        strcat(StringTemp, "{");
        const VehPrototype &remaining_proto =
            VehPrototypes[Vehs[current_vehicle_id].proto_id];
        int remaining_turns = remaining_proto.plan == 0x0C
            ? 1 : range(remaining_proto.reactor_id, 1, 100) * 10;
        remaining_turns -= vehicle.year_end_lurking;
        if (remaining_turns < 0) remaining_turns = 0;
        if (remaining_turns > 0x270F) remaining_turns = 0x270F;
        char turns_text[40];
        _itoa(remaining_turns, turns_text, 10);
        strcat(StringTemp, turns_text);
        strcat(StringTemp, "}");
        canvas.wrap(StringTemp, stats_x, cursor_y, stats_width);

        if (current_vehicle_id == second_vehicle_id) break;
        current_vehicle_id = second_vehicle_id;
    } while (true);

    MapWinCaptionScale = -1;
    MapWinCaptionDivisor = -1;
    canvas.restore_clip();
    MainInterfaceGlobal.update(0);
    do_non_input();
    time_.pulse(&teardown_00422ec0, 0, 0x1388, 5);
}
