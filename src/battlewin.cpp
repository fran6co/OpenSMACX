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
// ORIGINAL: 0x004219B0 _$E21 0x004219B0-0x004219C6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682068 selects initializer0x4219b0..0x4219c6, registering exit0x4219d0..0x4219da. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x6eeed8 bound to?g_BattleWin@@3VBattleWin@@A.", "Original startup instruction evidence: [[\"0x4219b0\", \"mov\", \"ecx, 0x6eeed8\"], [\"0x4219b5\", \"call\", \"0x422ee0\"], [\"0x4219ba\", \"push\", \"0x4219d0\"], [\"0x4219bf\", \"call\", \"0x645398\"], [\"0x4219c4\", \"pop\", \"ecx\"], [\"0x4219c5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4219d0\", \"mov\", \"ecx, 0x6eeed8\"], [\"0x4219d5\", \"jmp\", \"0x422ed0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/battlewin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_BattleWin@@3VBattleWin@@A\", \"??0BattleWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_BattleWin@@3VBattleWin@@A\", \"??1BattleWin@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f358; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_BattleWin@@3VBattleWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E12
// ORIGINAL: 0x004219D0 _$E19 0x004219D0-0x004219DA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682068 selects initializer0x4219b0..0x4219c6, registering exit0x4219d0..0x4219da. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x6eeed8 bound to?g_BattleWin@@3VBattleWin@@A.", "Original startup instruction evidence: [[\"0x4219b0\", \"mov\", \"ecx, 0x6eeed8\"], [\"0x4219b5\", \"call\", \"0x422ee0\"], [\"0x4219ba\", \"push\", \"0x4219d0\"], [\"0x4219bf\", \"call\", \"0x645398\"], [\"0x4219c4\", \"pop\", \"ecx\"], [\"0x4219c5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4219d0\", \"mov\", \"ecx, 0x6eeed8\"], [\"0x4219d5\", \"jmp\", \"0x422ed0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/battlewin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_BattleWin@@3VBattleWin@@A\", \"??0BattleWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_BattleWin@@3VBattleWin@@A\", \"??1BattleWin@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f358; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_BattleWin@@3VBattleWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E10
#include "stdafx.h"
#include "interface_fonts.h"
#include "battlewin.h"
#include "statuswin.h"
#include "maininterface.h"
#include "subinterface.h"
#include "time.h"
#include "console.h"   // ConsoleGlobal, the object at 0x009156B0
#include "mapwin.h"    // MainInterfaceGlobal, the object at 0x007AE820

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];

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
    if (mode != 0) {
        MainInterfaceGlobal.set_clip(
            0, 0, MainInterfaceGlobal.right_frame_rect().left,
            -MainInterfaceGlobal.dib_.bmiHeader.biHeight);
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
// Extent: compiled sizeof 0xa0; next mapped object 0x006EEF7C, gap unreferenced.
// RECOVERY_OBJECT: v2 0x006EEED8 0x006EEF78 ?g_BattleWin@@3VBattleWin@@A 0x004219B0
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
    g_BattleWin.release_iface_mode();
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
    g_BattleWin.release_iface_mode();
}

// ORIGINAL: 0x00421B20 ?pulse_timer@BattleWin@@QAEXXZ 0x00421B20-0x00421B37 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The image's 0x00421B20 adds eight to the BattleWin receiver before calling Time::pulse at 0x006164D0, with callback 0x00422EC0, zero parameter, period 0x1388 and resolution 5.", "BattleWin's constructor and destructor establish a real Time member at +8; 0x00422EC0 is the existing source-owned teardown callback."], "reason": "Home the timer pulse through BattleWin's typed Time member and existing callback."}
void BattleWin::pulse_timer() {
    time_.pulse(&teardown_00422ec0, 0, 0x1388, 5);
}

// Superseded seed deleted; the seed path below is historical Git provenance.
// ORIGINAL: 0x004219E0 ?init@BattleWin@@QAEXXZ 0x004219E0-0x00421B1F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 4219E0..421B1F complete instructions", "historical seed src/recovered/units/004219e0.cpp", "421B0D stores the incoming receiver at 7AE818, slot 10 of InterfaceRegistry at 7AE7F0; BattleWin's SubInterface base is at offset zero"], "reason": "Initialize the genuine BattleWin owner, preserving rectangle arithmetic and automatic Buffer lifetime, then publish its typed interface pointer in the shared registry. The old independent receiver-slot interpretation was incorrect; its original identity is retained beside InterfaceRegistry."}
// kind      game
// symbol    ?init@BattleWin@@QAEXXZ
#pragma optimize("y", off)
void BattleWin::init() {
    // The original constructs and tears down this temporary Buffer even though
    // it does not use it. Keep that lifetime visible to C++ and to unwind.
    Buffer scratch;

    const RECT &layout = MainInterfaceGlobal.main_layout_rect();
    const RECT &frame = MainInterfaceGlobal.main_frame_rect();

    field_70_ = frame;
    field_90_ = layout;

    RECT pane = layout;
    pane.left += 4;
    pane.top += 4;
    pane.right -= 4;
    pane.bottom -= 4;
    pane.right = (pane.right - pane.left + 3) / 2 + pane.left - 3;
    battle_rects_[0] = pane;
    const int delta = pane.right - pane.left + 3;
    pane.left += delta;
    pane.right += delta;
    battle_rects_[1] = pane;

    pane = frame;
    pane.left += 4;
    pane.top += 4;
    pane.right -= 4;
    pane.bottom -= 4;
    pane.right = (pane.right - pane.left + 3) / 2 + pane.left - 3;
    battle_rects_[2] = pane;
    pane.left = pane.right + 3;
    pane.right = frame.right - 4;
    battle_rects_[3] = pane;
    field_4_ = 10;

    InterfaceRegistry[10] = this;
}
#pragma optimize("", on)

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

// Shared separator objects the image pushes instead of unit literals.
extern char SocialWinColonSpaceText[];

// These six objects are already defined in sprite.cpp at the observed image
// addresses. sprite.h does not currently expose declarations for the family.
extern Sprite g_BATTLE_MIND_WORM_SPRITE;
extern Sprite g_BATTLE_ISLE_DEEP_SPRITE;
extern Sprite g_BATTLE_LOCUSTS_CHIRON_SPRITE;
extern Sprite g_BATTLE_FUNGAL_TOWER_SPRITE;
extern Sprite g_BATTLE_SPORE_LAUNCHER_SPRITE;
extern Sprite g_BATTLE_SEALURK_SPRITE;

// ORIGINAL: 0x00421B50 ?battle_report@BattleWin@@QAEXHHHHH@Z 0x00421B50-0x00422BFC;0x00652BA0-0x00652BCB
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00421b50.cpp:original hot and cold spans", "pinned terranx_original.exe SHA256:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_battlewin_battle_report_behavior.py", "recovery/tests/battlewin_battle_report_host_control.cpp"], "reason": "Recover the complete two-vehicle battle report through the real SubInterface owner, embedded Time, local Font lifetimes, MainInterface canvas, typed vehicle rules and inline battle modifier text rows."}
// Rewritten from the image (2026-11-13): two written-out portrait blocks, the
// report re-entered by `goto again` with first_vehicle_id replaced, ten
// say_num expansions (one 0x50 buffer each). Restored original behaviour:
// proto 15 shows the spore launcher and 19 the fungal tower (jump table at
// 0x00422BFC); the hit points subtract Veh::dmg_incurred (+0x10), not
// year_end_lurking; the destroyed/artifact label uses the flag-1 font
// (ebp-0x9c), the second font passed to set_font.
void BattleWin::battle_report(int first_vehicle_id, int second_vehicle_id,
                              int attack_odds, int defense_odds, int flags) {
    Font font_main;
    Font font_alt;
    Font font_third;

    const int iface_slot = IfaceIdToSlotStorage[ActiveIfaceIdStorage];
    if (iface_slot != 10) {
        return;
    }

    const int wide = WinScreenWidth != 800;
    MainInterfaceGlobal.box_sprite(&battle_rects_[0], &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.box_sprite(&battle_rects_[1], &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.box_sprite(&battle_rects_[2], &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.box_sprite(&battle_rects_[3], &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.redraw_complete();
    MainInterfaceGlobal.soft_update();

    int panel_height = WinScreenWidth != 800 ? 75 : 50;
    const int font_size = WinScreenWidth != 800 ? 12 : 10;
    font_main.init(DefaultFontFace, font_size, 0);
    font_third.init(DefaultFontFace, font_size, 2);
    font_alt.init(DefaultFontFace, font_size, 1);
    MainInterfaceGlobal.set_font(&font_main, &font_alt, &font_third, 0);
    MainInterfaceGlobal.set_text_color(MessageWinTextColor, -1, 1, 1);

    int psi_label = 0;
    const int both_artifacts =
        VehPrototypes[Vehs[first_vehicle_id].proto_id].plan == 0x0B
        && VehPrototypes[Vehs[second_vehicle_id].proto_id].plan == 0x0B;
    if (attack_odds >= 0 && defense_odds >= 0
        && (Weapon[VehPrototypes[Vehs[first_vehicle_id].proto_id].weapon_id].offense_rating < 0
            || Armor[VehPrototypes[Vehs[second_vehicle_id].proto_id].armor_id].defense_rating < 0
            || !Vehs[first_vehicle_id].faction_id)) {
        psi_label = 1;
    }
    const int first_faction = Vehs[first_vehicle_id].faction_id;
    const int second_faction = Vehs[second_vehicle_id].faction_id;
    RECT inner_first = battle_rects_[0];
    inner_first.left += 3;
    inner_first.top += 3;
    inner_first.right -= 3;
    inner_first.bottom -= 3;
    RECT inner_second = battle_rects_[1];
    inner_second.left += 3;
    inner_second.top += 3;
    inner_second.right -= 3;
    inner_second.bottom -= 3;
    MainInterfaceGlobal.set_font(find_font(14, 1), 0, 0, 0);

    // Portrait of the first vehicle's faction (or native) in rect 2.
    Sprite *portrait;
    if (first_faction) {
        portrait = &FactionsArt[first_faction].council_small;
    } else {
        switch (Vehs[first_vehicle_id].proto_id) {
        case 9: portrait = &g_BATTLE_ISLE_DEEP_SPRITE; break;
        case 10: portrait = &g_BATTLE_LOCUSTS_CHIRON_SPRITE; break;
        case 19: portrait = &g_BATTLE_FUNGAL_TOWER_SPRITE; break;
        case 15: portrait = &g_BATTLE_SPORE_LAUNCHER_SPRITE; break;
        case 14: portrait = &g_BATTLE_SEALURK_SPRITE; break;
        default: portrait = &g_BATTLE_MIND_WORM_SPRITE; break;
        }
    }
    int x = battle_rects_[2].left
        + (battle_rects_[2].right - battle_rects_[2].left - portrait->width()) / 2;
    int y = battle_rects_[2].top - battle_rects_[2].left + x;
    portrait->draw(&MainInterfaceGlobal, portrait->transparent_index(), x, y, 1, 1);
    RECT name_rect = battle_rects_[2];
    name_rect.left += 3;
    name_rect.top = y + portrait->height() + 3;
    name_rect.right -= 3;
    name_rect.bottom -= 3;
    MainInterfaceGlobal.set_text_color(ChatItemIdTable[first_faction],
                                       ChatShadowColors[first_faction], 1, 1);
    MainInterfaceGlobal.wrap_cent(Players[first_faction].formal_name_faction, &name_rect);

    // Portrait of the second vehicle's faction in rect 3.
    if (second_faction) {
        portrait = &FactionsArt[second_faction].council_small;
    } else {
        switch (Vehs[second_vehicle_id].proto_id) {
        case 9: portrait = &g_BATTLE_ISLE_DEEP_SPRITE; break;
        case 10: portrait = &g_BATTLE_LOCUSTS_CHIRON_SPRITE; break;
        case 19: portrait = &g_BATTLE_FUNGAL_TOWER_SPRITE; break;
        case 15: portrait = &g_BATTLE_SPORE_LAUNCHER_SPRITE; break;
        case 14: portrait = &g_BATTLE_SEALURK_SPRITE; break;
        default: portrait = &g_BATTLE_MIND_WORM_SPRITE; break;
        }
    }
    x = battle_rects_[3].left
        + (battle_rects_[3].right - battle_rects_[3].left - portrait->width()) / 2;
    y = battle_rects_[3].top - battle_rects_[3].left + x;
    portrait->draw(&MainInterfaceGlobal, portrait->transparent_index(), x, y, 1, 1);
    name_rect = battle_rects_[3];
    name_rect.left += 3;
    name_rect.top = y + portrait->height() + 3;
    name_rect.right -= 3;
    name_rect.bottom -= 3;
    MainInterfaceGlobal.set_text_color(ChatItemIdTable[second_faction],
                                       ChatShadowColors[second_faction], 1, 1);
    MainInterfaceGlobal.wrap_cent(Players[second_faction].formal_name_faction, &name_rect);
    UnionRect(&name_rect, &battle_rects_[2], &battle_rects_[3]);

    // Each pass reports first_vehicle_id; the second pass replaces it with
    // second_vehicle_id. Equal ids stop after one pass.
again:
    int width;
    int text_x;
    int text_y;
    if (!(first_vehicle_id != second_vehicle_id)) {
        width = inner_second.right - inner_second.left;
        text_x = inner_second.left;
        text_y = inner_second.top;
        MainInterfaceGlobal.set_clip(&inner_second);
    } else {
        width = inner_first.right - inner_first.left;
        text_x = inner_first.left;
        text_y = inner_first.top;
        MainInterfaceGlobal.set_clip(&inner_first);
    }
    int half_width;
    if (wide) {
        half_width = width / 2;
        veh_draw(&MainInterfaceGlobal, first_vehicle_id,
                 text_x + (half_width - panel_height) / 2, text_y,
                 WinScreenWidth != 800 ? 0 : -5, 0, 0);
        // WORKAROUND(noinline): the image expands proto_power here but calls
        // both range() copies out of line (0x00422F00); the later written-out
        // sum expands them. The original spent more /Ob2 budget before this.
        int (__cdecl *const range_call)(int, int, int) = range;
        if (!range_call((VehPrototypes[Vehs[first_vehicle_id].proto_id].plan == PLAN_ALIEN_ARTIFACT
                             ? 1
                             : range_call(VehPrototypes[Vehs[first_vehicle_id].proto_id].reactor_id, 1, 100) * 10)
                            - Vehs[first_vehicle_id].dmg_incurred,
                        0, 9999)) {
            const int label_y = text_y + panel_height / 2 / 2;
            MainInterfaceGlobal.set_font(&font_alt, 0, 0, 0);
            MainInterfaceGlobal.set_text_color(0x6A, 0, 1, 1);
            StringTemp[0] = '\0';
            if (VehPrototypes[Vehs[first_vehicle_id].proto_id].plan == 0x0C) {
                strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x4A0 / 4]));
            } else if (!(!Vehs[first_vehicle_id].faction_id)) {
                strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x498 / 4]));
            } else {
                strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x49C / 4]));
            }
            MainInterfaceGlobal.write_cent_text(StringTemp, text_x, label_y, half_width);
        }
    }

    int column_x = text_x;
    int row_y = wide ? panel_height : 4;
    row_y += text_y;
    StringTemp[0] = '\0';
    if (Vehs[first_vehicle_id].faction_id
        && Vehs[first_vehicle_id].faction_id != ConsoleGlobal.faction_id_ && !Language) {
        strcat(StringTemp, Players[Vehs[first_vehicle_id].faction_id].adj_name_faction);
        strcat(StringTemp, BufferJoinSpace);
    }
    strcat(StringTemp, VehPrototypes[Vehs[first_vehicle_id].proto_id].veh_name);
    if (Vehs[first_vehicle_id].faction_id
        && Vehs[first_vehicle_id].faction_id != ConsoleGlobal.faction_id_ && Language) {
        strcat(StringTemp, BufferJoinSpace);
        strcat(StringTemp, SocialWinOpenParenText);
        parse_set(Players[Vehs[first_vehicle_id].faction_id].noun_gender,
                  Players[Vehs[first_vehicle_id].faction_id].is_noun_plural);
        strcat(StringTemp, Players[Vehs[first_vehicle_id].faction_id].noun_faction);
        strcat(StringTemp, SocialWinCloseParenText);
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x80) {
        strcat(StringTemp, SocialWinOpenParenText);
        say_num(first_vehicle_id);
        strcat(StringTemp, SocialWinCloseParenText);
    }
    MainInterfaceGlobal.set_font(find_font(WinScreenWidth != 800 ? 16 : 12, 1), 0, 0, 0);
    MainInterfaceGlobal.set_text_color(
        ChatItemIdTable[first_vehicle_id == second_vehicle_id ? second_faction : first_faction],
        ChatShadowColors[first_vehicle_id == second_vehicle_id ? second_faction : first_faction],
        1, 1);
    row_y = MainInterfaceGlobal.wrap_cent(StringTemp, text_x, row_y, wide ? width / 2 : width);
    MainInterfaceGlobal.set_font(&font_main, 0, 0, 0);
    MainInterfaceGlobal.set_text_color(MessageWinTextColor, -1, 1, 1);
    if (wide) {
        column_x = text_x + width / 2;
        row_y = text_y + 5;
    }
    StringTemp[0] = '\0';
    say_stats_3(StringTemp, Vehs[first_vehicle_id].proto_id);
    half_width = width / 2;
    row_y = MainInterfaceGlobal.wrap(StringTemp, column_x, row_y, half_width);

    if (!both_artifacts) {
        StringTemp[0] = '\0';
        if (psi_label) {
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x310 / 4]));
        } else if (first_vehicle_id == second_vehicle_id
                   && (!(flags & 2)
                       || Chassis[VehPrototypes[Vehs[first_vehicle_id].proto_id].chassis_id].triad == 1)) {
            strcat(StringTemp, StringTable.get(
                Armor[VehPrototypes[Vehs[first_vehicle_id].proto_id].armor_id].name));
        } else {
            strcat(StringTemp, StringTable.get(
                Weapon[VehPrototypes[Vehs[first_vehicle_id].proto_id].weapon_id].name));
        }
        if (attack_odds >= 0 && defense_odds >= 0) {
            strcat(StringTemp, SocialWinColonSpaceText);
            if (first_vehicle_id == second_vehicle_id
                && (!(flags & 2)
                    || Chassis[VehPrototypes[Vehs[first_vehicle_id].proto_id].chassis_id].triad == 1)) {
                if (!(psi_label)) {
                    say_num(armor_proto(Vehs[first_vehicle_id].proto_id, -1, 0));
                } else {
                    say_num(Rules.psi_combat_ratio_def[
                        Chassis[VehPrototypes[Vehs[second_vehicle_id].proto_id].chassis_id].triad]);
                }
            } else if (psi_label) {
                say_num(Rules.psi_combat_ratio_atk[
                    Chassis[VehPrototypes[Vehs[second_vehicle_id].proto_id].chassis_id].triad]);
            } else {
                say_num(offense_proto(Vehs[first_vehicle_id].proto_id, -1, 0));
            }
        }
        row_y = MainInterfaceGlobal.wrap(StringTemp, column_x, row_y, half_width);
    }

    StringTemp[0] = '\0';
    say_morale(first_vehicle_id,
               first_vehicle_id == second_vehicle_id ? first_faction : second_faction);
    strcat(StringTemp, SocialWinColonSpaceText);
    const int side = first_vehicle_id == second_vehicle_id;
    const int morale = ((int)VehBasicBattleMorale[side] - 2) * 100 / 8;
    if (morale >= 0) {
        strcat(StringTemp, "+");
    }
    say_num(morale);
    strcat(StringTemp, "%");
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, BufferJoinSpace);
    row_y = MainInterfaceGlobal.wrap(StringTemp, column_x, row_y, half_width);
    for (int i = 0; i < (int)VehBattleModCount[side]; i++) {
        StringTemp[0] = '\0';
        strcat(StringTemp, VehBattleDisplay[side * 4 + i]);
        strcat(StringTemp, BufferJoinSpace);
        if (VehBattleModifier[side * 4 + i] >= 0) {
            strcat(StringTemp, "+");
        }
        say_num(VehBattleModifier[side * 4 + i]);
        strcat(StringTemp, "%");
        row_y = MainInterfaceGlobal.wrap(StringTemp, column_x, row_y, half_width);
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x520 / 4]));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, "{");
    say_num((first_vehicle_id == second_vehicle_id ? defense_odds : attack_odds) >> 8);
    strcat(StringTemp, ".");
    const int hundredths =
        ((first_vehicle_id == second_vehicle_id ? defense_odds : attack_odds) & 0xFF) * 100 >> 8;
    if (hundredths < 10) {
        strcat(StringTemp, "0");
    }
    say_num(hundredths);
    strcat(StringTemp, "}");
    row_y = MainInterfaceGlobal.wrap(StringTemp, column_x, row_y, half_width);

    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x1E4 / 4]));
    strcat(StringTemp, SocialWinColonSpaceText);
    strcat(StringTemp, "{");
    say_num(range(proto_power(first_vehicle_id) - Vehs[first_vehicle_id].dmg_incurred, 0, 9999));
    strcat(StringTemp, "}");
    MainInterfaceGlobal.wrap(StringTemp, column_x, row_y, half_width);

    if (first_vehicle_id != second_vehicle_id) {
        first_vehicle_id = second_vehicle_id;
        goto again;
    }

    MapWinCaptionScale = -1;
    MapWinCaptionDivisor = -1;
    MainInterfaceGlobal.restore_clip();
    MainInterfaceGlobal.update(0);
    do_non_input();
    time_.pulse(&teardown_00422ec0, 0, 0x1388, 5);
}
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00421B50 0x00422BFC 0x00422C14 0x00421EAD
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00421B50 0x00422C14 0x00422C1F 0x00421EA7
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00421B50 0x00422C20 0x00422C38 0x00421FE0
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00421B50 0x00422C38 0x00422C43 0x00421FDA
