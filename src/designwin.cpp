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
#include "wave.h"
#include "designwin.h"
#include "win.h"
#include "worldwin.h"
#include "faction.h"
#include "game.h"
#include "popup.h"
#include "basepop.h"
#include "strings.h"
#include "technology.h"
#include "temp.h"
#include "general.h"
#include "veh.h"
#include "selectpartwin.h"
#include "tutwin.h"     // TutWinGlobal - the tutorial popup every click dismisses
#include "alpha.h"      // Labels - the label-id block on_left_click reads
#include "netdaemon.h"  // synch_proto, message_data, NetDaemonNet
#include "xpops.h"      // x_pop_caption_label

int __cdecl abil_cond(int ability_id, int chassis_id, int weapon_id,
                     int armor_id);  // 0x00584B50, implemented in leaf_recoveries.cpp

// DISCOVERY 0x00439D30: suggest_abil reads ability prerequisites at
// 0x009AB550 (14 shorts per ability row), checks editor gating at 0x009A6490,
// consults research at 0x00949780, and scans 0x359-stride tables at
// 0x00946F60/0x00946F80 using the faction id at 0x00939284. The recovered
// seed confirms these roles and behavior, but current source has not assigned
// those tables/flags to their owning typed globals. Keep the product body
// unfinished until those owners are established; do not carry new fixed-address
// aliases into this translation unit.

// ---- shared seams and bindings the recovered bodies reach -----------------

// The slide helpers are still original bodies (0x0045D440 / 0x0045D970);
// councwin.cpp declares and forwards them, and the definitions are
// external-linkage so this TU links against those (same shape as
// battlewin.cpp).
void __cdecl slide_show(GraphicWin *window, int mode);   // 0x0045D440
void __cdecl slide_hide(GraphicWin *window, int mode);   // 0x0045D970

// 0x00945B40 / 0x00945B44 - the prototype ranking proto_sort fills. map.cpp
// owns the sort and defines the same-named pair (as internal-linkage const
// bindings, so they cannot be externed from here); the addresses and roles
// are restated for this file's stack bodies: count first, then the id list.
static int *const ProtoSortCount = (int *)0x00945B40;
static int *const ProtoSortIds = (int *)0x00945B44;

// 0x009A64C0 - the interface flag byte scenario.cpp documents (`or dword
// [0x9A64C0], 0xE0`, `test byte [0x9A64C0], 0x80` and `, 0x40`). Bit 0x40
// unhides the two toggle buttons flatButtons_[2] and [3] while the design
// window is open. No catalogue name for the byte.
static unsigned char *const InterfaceFlagByte = (unsigned char *)0x009A64C0;

// 0x00749C88 - menu.cpp's g_MENU_UP_WAVE; on_status plays it when the window
// opens. Defined (and owned) there; no header declares it.
extern Wave g_MENU_UP_WAVE;

// ---- editor-panel bindings (the per-file convention basewin.cpp and
// socialwin.cpp use; each folds to a bare immediate) ------------------------

static int *const InterfaceBuffer = (int *)0x007AEC64;   // the shared
    // full-screen drawing Buffer - the same binding battlewin.cpp,
    // basewin.cpp, maininterface.cpp, reportif.cpp and socialwin.cpp carry.
static int *const InterfaceClipRect = (int *)0x007AEC94; // InterfaceBuffer+0x30,
    // the RECT every one of those bodies hands to Buffer::set_clip.
// draw_flash picks its small font size off WinScreenWidth (win.h, 0x320 on
// the shipped UI).
static unsigned char *const InterfaceFlagTail = (unsigned char *)0x009A64C1;
    // InterfaceFlagByte's next byte; bit 0x10 unhides the prototype numbers.

static Buffer **const StackTileBufferSlot = (Buffer **)0x0078D6B0;
    // The tile source draw_stack fills its panel from; basewin.cpp documents
    // the address as a member slot holding a Buffer *, not a Buffer.
static int *const StackTitleColorMain = (int *)0x008C6CA4;
static int *const StackTitleColorShadow = (int *)0x008C6CA8;
static int *const StackTextColorMain = (int *)0x008C6B94;
static int *const StackTextColorShadow = (int *)0x008C6B98;
    // Two text color pairs the preview panel draws with (main, shadow), read
    // straight into Buffer::set_text_color - the bold title line and the stat
    // lines respectively. No catalogue name for any.

static unsigned char *const WeaponClassRows = (unsigned char *)0x0094AE68;
    // weapon_id-strided class rows technology.cpp's note describes; the row's
    // first byte is negative for weapons the editor cannot offer.
static char *const ProtoAbilityIdBase = (char *)0x00952832;
    // Per-prototype 0x34-stride shorts just past ReactorPickTable (the
    // 4x12-byte picker table at 0x00952800 draw_reactor scans): the ability
    // prototype id the stack/stat views quote for prototype proto. The same
    // table console.cpp reaches as `0x952832 + id * 0x34`.
static unsigned char *const PlayerFlagBlocks = (unsigned char *)0x00946F58;
    // Per-faction byte blocks at sizeof(Player) stride; bit 0x80 of the first
    // byte selects the alien-faction popup set. The Player array front is
    // 0x00946A50, so this is +0x508 into player 0. No catalogue name.

// diplo_lock (0x00539820) and diplo_unlock (0x005398C0) are unhomed;
// design_it reaches them through their original addresses like the other
// pending seams.

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the editor popups pass it as their wait callback.
int __cdecl pop_wait();

// The alpha(x).txt keys and panel sprites the editor popups raise (popp's
// label argument is an alpha.txt key, not display text; the `alunit_sm.pcx`
// forms are the alien-faction variants). SpaceText's convention applies:
// the image's .data carries the literal, the address is what the code holds.
static char *const EditorNamePopupLabel = (char *)0x00683604;   // INTRINSICNAME
static char *const EditorNamePopupAlienLabel = (char *)0x006835E8;
static char *const EditorNamePopupSprite = (char *)0x006835F8;  // unit_sm.pcx
static char *const EditorNamePopupAlienSprite = (char *)0x006835D8;
static char *const EditorNameXPopCaption = (char *)0x0068367C;  // INTRINSICNAME
static char *const EditorNameXPopAlienCaption = (char *)0x00683660;
static char *const EditorNameXPopSprite = (char *)0x00683670;   // unit_sm.pcx
static char *const EditorNameXPopAlienSprite = (char *)0x00683650;
static char *const EditorTypePopupLabel = (char *)0x00683640;   // INTRINSICTYPE
static char *const EditorTypePopupAlienLabel = (char *)0x00683624;
static char *const EditorTypePopupSprite = (char *)0x00683634;  // unit_sm.pcx
static char *const EditorTypePopupAlienSprite = (char *)0x00683614;
static char *const EditorOverwritePopupLabel = (char *)0x006836B8;  // OVERWRITEPROTO
static char *const EditorOverwritePopupAlienLabel = (char *)0x0068369C;
static char *const EditorOverwritePopupSprite = (char *)0x006836AC;
static char *const EditorOverwritePopupAlienSprite = (char *)0x0068368C;

// Stack-panel string fragments the image holds beside SpaceText: "(", ")",
// ". ", ": " and "#".
static char *const EditorOpenParen = (char *)0x00682E9C;
static char *const SpaceText = (char *)0x00682820;   // " " - the same binding
    // buffer.cpp, popup.cpp and socialwin.cpp carry under this name.
static char *const EditorCloseParen = (char *)0x00682E98;
static char *const EditorDotSpace = (char *)0x00683920;
static char *const EditorColonSpace = (char *)0x00682E90;
static char *const EditorHashSign = (char *)0x00683918;

// The unit-special picker popups select_special raises. The labels are
// alpha(x).txt keys (the popp convention), and the alien-faction variants
// swap unit_sm.pcx for alunit_sm.pcx. The four stat fragments are the
// arithmetic the item labels are built from; the image holds duplicated
// copies of each (0x6838d0/d4/d8, 0x6838dc/e4/ec, 0x6838e0/e8/f0) and the
// bindings below name the first of each run.
static char *const SpecialTwoPopupLabel = (char *)0x0068385C;        // TWOSPECIAL
static char *const SpecialTwoPopupAlienLabel = (char *)0x00683894;   // TWOSPECIAL
static char *const SpecialMissilePopupLabel = (char *)0x00683878;    // MISSILESPECIAL
static char *const SpecialMissilePopupAlienLabel = (char *)0x006838AC;
static char *const SpecialUnitsPopupLabel = (char *)0x006838BC;      // UNITSPECIAL
static char *const SpecialTwoPopupSprite = (char *)0x00683888;       // unit_sm.pcx
static char *const SpecialTwoPopupAlienSprite = (char *)0x0068384C;  // alunit_sm.pcx
static char *const SpecialMissilePopupSprite = (char *)0x006838A0;   // unit_sm.pcx
static char *const SpecialMissilePopupAlienSprite = (char *)0x00683868;
static char *const SpecialSlashSign = (char *)0x006838CC;            // "/"
static char *const SpecialMinusOneText = (char *)0x006838D0;         // "-1"
static char *const SpecialPlusSignText = (char *)0x006838DC;         // "+"
static char *const SpecialMinusTwoText = (char *)0x006838E0;         // "-2"

static int *const AbilityLabelIds = (int *)0x009AB540;
static int *const AbilityLabelIdsEnd = (int *)0x009AB86C;
    // The ability-label id table draw_flash's stat line walks - 39 dwords at
    // a 0x1C stride, one per ABL_ bit, each an alpha.txt label StringTable
    // resolves; SocialEffectRow/RowEnd is the same pointer-pair convention.

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004345C0 ?UNK1@DesignWin@@QAEXXZ 0x004345C0-0x004345C1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK1@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::UNK1() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D330 ?on_iface_left_down@DesignWin@@QAEXHH@Z 0x0043D330-0x0043D333 BYTE_EXACT
// symbol    ?on_iface_left_down@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D340 ?on_iface_right_down@DesignWin@@QAEXHH@Z 0x0043D340-0x0043D343 BYTE_EXACT
// symbol    ?on_iface_right_down@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D400 ?on_iface_selected@DesignWin@@QAEXHH@Z 0x0043D400-0x0043D403 BYTE_EXACT
// symbol    ?on_iface_selected@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D410 ?on_iface_left_double_click@DesignWin@@QAEXHH@Z 0x0043D410-0x0043D413 BYTE_EXACT
// symbol    ?on_iface_left_double_click@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D420 ?on_iface_right_double_click@DesignWin@@QAEXHH@Z 0x0043D420-0x0043D423 BYTE_EXACT
// symbol    ?on_iface_right_double_click@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D470 ?on_iface_button_toggled@DesignWin@@QAEXHH@Z 0x0043D470-0x0043D473 BYTE_EXACT
// symbol    ?on_iface_button_toggled@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D480 ?on_iface_group_clicked@DesignWin@@QAEXHHH@Z 0x0043D480-0x0043D483 BYTE_EXACT
// symbol    ?on_iface_group_clicked@DesignWin@@UAEXHHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_group_clicked@DesignWin@@QAEXHHH@Z)(DesignWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_group_clicked(int, int, int) {
}

// ---- the interface table's own slots -------------------------------------

// Slot 44 of the front vtable - the catalogue's
// ?on_button_clicked@DesignWin@@QAEXH@Z, 0x00438460, 4481 bytes (plus the cold
// tail at 0x00654010), unrecovered. vslot_13 and on_iface_button_pressed both
// reach it through a live dispatch, so the entry has to be DesignWin's.
void DesignWin::on_button_clicked(int button_id) {
    typedef void(__fastcall *pending)(DesignWin *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00438460))(
        this, nullptr, button_id);
}

/*
// ORIGINAL: 0x0043D430 ?on_iface_button_clicked@DesignWin@@QAEXH@Z 0x0043D430-0x0043D44D BYTE_EXACT
// symbol    ?vslot_13@DesignWin@@UAEXH@Z
// size      29 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0043D443
Return Value: n/a
Status: Complete
*/
// LEVER: the receiver is the ENTRY (the image's `add ecx,-0xa14` is VC6's own
//   adjustment to the front) and the dispatch is the front's slot 44, so the
//   whole body is one unqualified virtual call - `on_button_clicked(button_id)` is the
//   image's `call dword ptr [eax + 0xb0]`.
void DesignWin::vslot_13(int button_id) {
    on_button_clicked(button_id);
}

/*
// ORIGINAL: 0x0043D450 ?on_iface_button_pressed@DesignWin@@QAEXH@Z 0x0043D450-0x0043D46F BYTE_EXACT
// symbol    ?on_iface_button_pressed@DesignWin@@UAEHH@Z
// size      31 bytes
// prototype int (__thiscall ?on_iface_button_pressed@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0043D463
Return Value: 0
Status: Complete
*/
// LEVER: the same slot-44 dispatch as vslot_13 with a constant 0 returned
//   after it - the interface wants an answer, the design window forwards the
//   click to the front and reports "not pressed".
int DesignWin::on_iface_button_pressed(int button_id) {
    on_button_clicked(button_id);
    return 0;
}

// ---- slots still carrying an original body ------------------------------

// The interface slot bodies run with the second-base receiver in the image
// (the slot holds the raw body address, so ECX is DesignWin+0xA14 - the
// seed for this body proves it: `self - 0xa14` is handed to the Win base,
// and flatButtons_ appears at ECX+0x2D8). The tree spells the body as a
// plain override and the compiled object shows NO adjustor thunks: the body
// compiles straight to the original's receiver convention, byte for byte.
// ORIGINAL: 0x0043D120 ?on_status@DesignWin@@QAEXH@Z 0x0043D120-0x0043D215 FILE BYTE_EXACT
// symbol    ?on_status@DesignWin@@UAEXH@Z
// size      245 bytes
// prototype void (__thiscall ?on_status@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D440 0x0045D970 0x004C6920 0x005D56B0 0x005ED9D0 0x005EDCD0 0x005FCBB0
// Homed from src/unrecovered/0043d120.cpp: open the design interface - paint
// the front's contents, show the 25 palette buttons (and the two toggles when
// interface bit 0x40 allows), update the main view, play the menu-up sound,
// slide and show the window - or close it all back down.
void DesignWin::on_status(int mode) {
    if (mode != 0) {
        static_cast<GraphicWin *>(this)->on_redraw();
        FlatButton *button = flatButtons_;
        int count = 0x19;
        do {
            button->show(4);
            ++button;
            --count;
        } while (count != 0);
        if (*InterfaceFlagByte & 0x40) {
            flatButtons_[2].show(0);
            flatButtons_[3].show(0);
        } else {
            flatButtons_[2].hide();
            flatButtons_[3].hide();
        }
        MainInterfaceGlobal.update(0);
        do_all_draws();
        g_MENU_UP_WAVE.play();
        slide_show(this, 0);
        this->Win::show(3);
    } else {
        FlatButton *button = flatButtons_;
        int count = 0x19;
        do {
            button->hide();
            ++button;
            --count;
        } while (count != 0);
        slide_hide(this, 0);
        this->Win::hide();
        do_all_draws();
    }
}

// PENDING 0x0043D220 ?on_iface_left_click@DesignWin@@QAEXHH@Z 0x0043D220-0x0043D281
// symbol    ?on_iface_left_click@DesignWin@@UAEXHH@Z
// size      97 bytes
// prototype void (__thiscall ?on_iface_left_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00 0x005169F0
void DesignWin::on_iface_left_click(int a1, int a2) {
    typedef void(__fastcall *pending)(DesignWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0043D220))(
        this, nullptr, a1, a2);
}

// ORIGINAL: 0x0043D290 ?on_iface_right_click@DesignWin@@QAEXHH@Z 0x0043D290-0x0043D327
// RECOVERY_BINDING: v1 {"evidence": ["src/designwin.cpp:DesignWin::on_iface_right_click", "src/designwin.h:spot1_", "src/alpha.h:Labels.strings_ptr"], "reason": "Resolve the prototype-stack hover target and show its localized label at the target's screen-space anchor."}
// symbol    ?on_iface_right_click@DesignWin@@UAEXHH@Z
// size      151 bytes
// prototype void (__thiscall ?on_iface_right_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDDE0 0x005ED240 0x005F83D0 0x005FAB00 0x006169A0
// Homed from src/unrecovered/0043d290.cpp: a right-click on the prototype
// stack (spot1_) raises the tutorial-style bubble naming the prototype under
// the pointer - the label id comes out of the game block at
// Labels.strings_ptr+0xCE8, and the bubble anchor is that map square pushed
// through the main interface's client_to_screen.
void DesignWin::on_iface_right_click(int xCoord, int yCoord) {
    TutWinGlobal->reset();
    int position;
    int type;
    if (spot1_.check(xCoord, yCoord, &position, &type) != -1 && position >= 0) {
        // The image swaps its two argument slots so the client_to_screen call
        // receives (&xCoord, &yCoord); the anchor rect is that screen square.
        MainInterfaceGlobal.client_to_screen(&xCoord, &yCoord);
        // The label id sits at byte offset 0xCE8 of the game block - slot
        // 0x33A of the LPSTR array, the same walk datalink.cpp's
        // CATEGORY_LABEL makes over Labels.strings_ptr.
        int label_id = (int)reinterpret_cast<LPSTR *>(Labels.strings_ptr)[0x33A];
        RECT rect;
        rect.left = xCoord;
        rect.top = yCoord;
        rect.right = xCoord;
        rect.bottom = yCoord;
        // MainInterface declares its own (button_id, text) set_bubble_text,
        // which would hide the Win member the image calls directly.
        MainInterfaceGlobal.Win::set_bubble_text(StringTable.get(label_id),
                                                 &rect);
    }
}

// ORIGINAL: 0x00437BA0 ?on_mouse_move@DesignWin@@QAEXHH@Z 0x00437BA0-0x00437BD5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00437ba0.cpp", "DesignWin spot2_ member and Win::clear_bubble_text in src/"], "reason": "Home the direct pointer hover handler through the real DesignWin spot member and Win interface."}
// size      53 bytes
// prototype void (__thiscall ?on_mouse_move@DesignWin@@QAEXHH@Z)(DesignWin* this, int xCoord, int yCoord)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005F8500 0x005FAB00
// Homed from src/recovered/units/00437ba0.cpp.
void DesignWin::on_mouse_move(int xCoord, int yCoord) {
    Win::clear_bubble_text();
    Win::clear_bubble_text();

    int position;
    int type;
    spot2_.check(xCoord, yCoord, &position, &type);
}

// ORIGINAL: 0x0043D350 ?on_iface_mouse_move@DesignWin@@QAEXHH@Z 0x0043D350-0x0043D3B9 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0043d350.cpp", "DesignWin spot1_, field_CE0_ and draw_flash in src/", "Existing secondary-interface override symbol mapping"], "reason": "Replace the original-address forwarder with a typed interface hover handler while retaining the historical exact seed for restoration."}
// symbol    ?on_iface_mouse_move@DesignWin@@UAEXHH@Z
// size      105 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// Homed from src/recovered/units/0043d350.cpp.
void DesignWin::on_iface_mouse_move(int a1, int a2) {
    Win::clear_bubble_text();
    Win::clear_bubble_text();

    int position;
    int type;
    int result = spot1_.check(a1, a2, &position, &type);
    int selected_spot;
    if (result != -1) {
        selected_spot = position;
        if (field_CE0_ == selected_spot) {
            return;
        }
    } else {
        selected_spot = 0x3F;
        if (field_CE0_ == selected_spot) {
            return;
        }
    }
    field_CE0_ = selected_spot;
    draw_flash();
}

// ORIGINAL: 0x0043D3C0 ?on_iface_mouse_leave@DesignWin@@QAEXHH@Z 0x0043D3C0-0x0043D3F1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0043d3c0.cpp", "DesignWin field_CE0_, draw_flash and Win::clear_bubble_text in src/", "Existing secondary-interface override symbol mapping"], "reason": "Replace the original-address forwarder with a typed interface leave handler while retaining the historical exact seed for restoration."}
// symbol    ?on_iface_mouse_leave@DesignWin@@UAEXHH@Z
// size      49 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// Homed from src/recovered/units/0043d3c0.cpp.
void DesignWin::on_iface_mouse_leave(int a1, int a2) {
    Win::clear_bubble_text();
    Win::clear_bubble_text();
    if (field_CE0_ != 0x3F) {
        field_CE0_ = 0x3F;
        draw_flash();
    }
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x00437BE0 ?on_mouse_leave@DesignWin@@QAEXHH@Z 0x00437BE0-0x00437BED BYTE_EXACT
// symbol    ?on_mouse_leave@DesignWin@@UAEXHH@Z
// size      13 bytes
// prototype void (__thiscall ?on_mouse_leave@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void DesignWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
// ORIGINAL: 0x0043BE10 ?select_special_1@DesignWin@@QAEXXZ 0x0043BE10-0x0043BE18 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?select_special_1@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043AC70
Status: Complete
*/
void DesignWin::select_special_1() {
    select_special(0);
}

/*
// ORIGINAL: 0x0043BE20 ?select_special_2@DesignWin@@QAEXXZ 0x0043BE20-0x0043BE28 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?select_special_2@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043AC70
Status: Complete
*/
void DesignWin::select_special_2() {
    select_special(1);
}

// The seam and definition for the unrecovered select_special, at the end of
// the file so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness, and being defined after the
// callers is NOT enough on its own: VC6 at /Ob2 defers codegen to the end of
// the translation unit and folds a forwarder defined later back into a caller
// defined earlier. Measured - without the pragma cl reports C4711 at
// designwin.cpp(153) and (161), the two select_special_N thunks, and each is
// an 8-byte BYTE_EXACT `call rel32` today.

/*
Purpose: Close the design window: dismiss all 25 palette buttons, release the
         spot list, stop the clock, release the buffer, reset the prototype
         selection, and close the three fonts.
// ORIGINAL: 0x0043C1A0 ?close@DesignWin@@QAEXXZ 0x0043C1A0-0x0043C261
// size      193 bytes
// prototype void (__thiscall ?close@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   6   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005FA830 0x00616780 0x00619230
// indirect  0x0043C1B4
// MEASURED 2026-08-29: /O2 best at 12/42. Image instructions 0..11 agree -
//   the prologue and the whole 25-button loop, the slot-0x168 dispatch
//   arriving as the image's own single `call dword ptr [eax + 0x168]` out of
//   vtable_slot once /Ob2 inlines it (at /Ob0 the template function itself
//   is CALLED - `push 0x168 / push edi / call`, which is why /Ob0's
//   higher-similarity 6/42 is the worse listing; do not chase it).
//   Everything from image instruction 12 on diverges for the catalogued
//   MEASURED-inline ceiling: spot2_.shutdown(), time_.close() and the three
//   font_.close() calls are in-class definitions in spot.h / time.h /
//   font.h (in-class so ~Spot / ~Time / ~Font inline them), and VC6 inlines
//   all five HERE where the image makes five plain calls (0x005FA830,
//   0x00616780, 0x00619230 x3) - 141 instructions against the image's 42.
//   Out-of-line bodies would regress those destructor claims, and VC6
//   honours no per-site noinline.
Return Value: n/a
Status: Complete
*/
namespace {

// Slot 2, the DesignWin front vtable's own second entry.
typedef void (__fastcall *func_design_win_slot002)(void *);

}

void DesignWin::close() {
    // A do/while counting DOWN from 25 over a walking pointer - the image
    // keeps `mov ebx, 0x19` and steps `add edi, 0xb4c` (sizeof FlatButton).
    FlatButton *button = flatButtons_;
    int count = 0x19;
    do {
        button->close();
        ++button;
        --count;
    } while (count != 0);

    spot2_.shutdown();
    // IMAGE ORDER: scene_rotation_ and field_CE8_ are written between the
    // clock receiver's computation and the call itself.
    scene_rotation_ = 0.0f;
    field_CE8_ = 0;
    time_.close();
    buffer_.close();
    factionID_ = 0;
    protoID_ = 0;
    protoChassisType_ = 0;
    protoWeaponType_ = 0;
    protoArmorType_ = 0;
    protoAbilityFlags_ = 0;
    protoReactorType_ = 1;
    field_14210_ = 0;
    mineralCostFactor_ = 0;
    field_CE0_ = static_cast<uint32_t>(-1);
    field_14220_ = 0;
    font1_.close();
    font2_.close();
    font3_.close();
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Wave g_DESIGNWIN_WAVE;  // 0x0071F240

// The process-wide design window, 0x0071F2B0 in the image - REAL STORAGE,
// not the `Win *const TutWinDesWindow` binding that named terranx.exe data
// (see tutwin.h). The image's dynamic initializer at 0x00432850 constructs
// it at its fixed address before WinMain; DesignWin's constructor here is
// the empty inline the tree models, so this object is zero storage the
// game's own init populates, exactly as the image leaves it.
// The vehicle chassis animation queries this same window's Win base at
// 0x004C2BE9; the original receiver is the complete object at 0x0071F2B0.
// RECOVERY_OBJECT: v1 0x0071F2B0 ?TutWinDesWindow@@3VDesignWin@@A
DesignWin TutWinDesWindow;  // 0x0071F2B0
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// ORIGINAL: 0x00439670 ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z 0x00439670-0x004396B0
// RECOVERY_BINDING: v1 {"evidence": ["src/designwin.cpp:DesignWin::bubble_me_elmo2", "Win::client_to_screen and Win::set_bubble_text"], "reason": "Convert the pointer anchor from client to screen coordinates and raise the named bubble through the real Win base interface."}
// size      64 bytes
// prototype void (__thiscall ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z)(DesignWin* this, LPCSTR, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240 0x005F83D0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439670
// measured tier  MISMATCH
// divergence     12
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439670/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z  at 0x00439670  (64 bytes)
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


void DesignWin::bubble_me_elmo2(char *a1, int a2, int a3) {
    client_to_screen(&a2, &a3);
    RECT anchor;
    anchor.left = a2;
    anchor.top = a3;
    anchor.right = a2;
    anchor.bottom = a3;
    set_bubble_text(a1, &anchor);
}

// ORIGINAL: 0x00439EB0 ?delete_abil@DesignWin@@QAEXH@Z 0x00439EB0-0x00439F18
// RECOVERY_BINDING: v1 {"evidence": ["src/designwin.h:protoAbilityFlags_ at 0x14208", "src/designwin.h:protoAbilityID1_ and protoAbilityID2_ at 0x14218/0x1421C"], "reason": "Remove one selected ability flag and rebuild the two displayed ability identifiers from the remaining flags."}
// size      104 bytes
// prototype void (__thiscall ?delete_abil@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439EB0
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439eb0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?delete_abil@DesignWin@@QAEXH@Z  at 0x00439EB0  (104 bytes)
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


void DesignWin::delete_abil(int a1) {
    if ((static_cast<uint32_t>(a1) & protoAbilityFlags_) != 0) {
        // Keep the UI slots in named object state; the old seed's
        // register-allocation observation does not apply to this body.
        protoAbilityFlags_ &= ~static_cast<uint32_t>(a1);
        protoAbilityID1_ = static_cast<uint32_t>(-1);
        protoAbilityID2_ = static_cast<uint32_t>(-1);
        int i = 0;
        do {
            if ((protoAbilityFlags_ & (1u << i)) != 0) {
                if (protoAbilityID1_ == static_cast<uint32_t>(-1)) {
                    protoAbilityID1_ = static_cast<uint32_t>(i);
                } else if (protoAbilityID2_ == static_cast<uint32_t>(-1)) {
                    protoAbilityID2_ = static_cast<uint32_t>(i);
                }
            }
            i++;
        } while (i < 0x1d);
    }
}

// ORIGINAL: 0x0043D0C0 ?show@DesignWin@@QAEXH@Z 0x0043D0C0-0x0043D0E8 BYTE_EXACT
// symbol    ?show@DesignWin@@UAEXH@Z
// LEVER: homed from 0043d0c0.cpp; needed the `symbol` fact (the tree emits
// UAEXH - it overrides Win::show's virtual - where the image has QAEXH) before
// measure could find it, then held BYTE_EXACT on the idiomatic base calls
// `is_visible()` / `set_iface_mode()` where the artifact had two
// reinterpret_casts of `this`.
// size      40 bytes
// prototype void (__thiscall ?show@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00446B40 0x0045D310 0x005F7E90
// 0x0043D0C0  ?show@DesignWin@@QAEXH@Z  ->  ?show@DesignWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::show(int a1) {
    if (!is_visible()) {
        ambience(0xD3);
        set_iface_mode();
    }
}

// ORIGINAL: 0x0043D0F0 ?hide@DesignWin@@QAEXXZ 0x0043D0F0-0x0043D116 BYTE_EXACT
// size      38 bytes
// prototype void (__thiscall ?hide@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00446B40 0x0045D380 0x005F7E90
// symbol    ?hide@DesignWin@@UAEXXZ
// LEVER: homed from 0043d0f0.cpp; same `symbol` fact as show - the tree emits
// the virtual UAEXX form that overrides Win::hide. Idiomatic base calls held
// BYTE_EXACT on the first measure.
// 0x0043D0F0  ?hide@DesignWin@@QAEXXZ  ->  ?hide@DesignWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::hide() {
    if (is_visible()) {
        release_iface_mode();
        ambience(0xd4);
    }
}

// ORIGINAL: 0x0043C070 ?construct_chassis@DesignWin@@QAEXXZ 0x0043C070-0x0043C09E BYTE_EXACT
// size      46 bytes
// prototype void (__thiscall ?construct_chassis@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C2470
// 0x0043C070  ?construct_chassis@DesignWin@@QAEXXZ  ->  ?construct_chassis@DesignWin@@QAEXXZ
// LEVER: homed from 0043c070.cpp. The named fields (factionID_ 0x141F4,
// protoChassisType_ 0x141FC, protoWeaponType_ 0x14200, protoReactorType_
// 0x1420C, caviar_ 0x1279C) replaced the artifact's five
// reinterpret_casts of `this` and held BYTE_EXACT first measure; the callee
// needed a pending_bodies forwarder plus the prototype now at the end of
// designwin.h.
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::construct_chassis() {
    vehdraw_construct_chassis(factionID_, protoChassisType_, protoWeaponType_, 0,
                              protoReactorType_, &caviar_);
}

// The image's shared design-window auto-build latch at 0x00733974. The
// construct_vehicle seed brackets its automatic weapon construction with
// writes of 1 and 0; construct_weapon and draw_weapon read the same slot.
// Keep it as source-owned mutable state rather than an alias into the image.
// RECOVERY_OBJECT: v1 0x00733974 ?DesignAutoBuildActive@@3HA
int DesignAutoBuildActive = 0;

// ORIGINAL: 0x0043BF50 ?construct_vehicle@DesignWin@@QAEXXZ 0x0043BF50-0x0043C062 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0043bf50.cpp; src/designwin.h:DesignWin prototype and Caviar members; src/veh.h:RulesArmor and vehdraw_construct_*"], "reason": "Rebuild all component preview objects from the selected DesignWin state, using the typed armor table and existing vehicle-render helpers."}
void DesignWin::construct_vehicle() {
    caviar_.reset_objects();

    const int saved_chassis = static_cast<int>(protoChassisType_);
    if (protoWeaponType_ == 0x15 && saved_chassis == 1
        && Armor[protoArmorType_].defense_rating <= 1) {
        protoChassisType_ = 0;
    }
    if (protoID_ == 0x14) {
        protoChassisType_ = 0;
    }

    vehdraw_construct_chassis(static_cast<int>(factionID_),
                              static_cast<int>(protoChassisType_),
                              static_cast<int>(protoWeaponType_),
                              static_cast<int>(protoArmorType_),
                              static_cast<int>(protoReactorType_), &caviar_);

    DesignAutoBuildActive = 1;
    vehdraw_construct_weapon(static_cast<int>(factionID_),
                             static_cast<int>(protoChassisType_),
                             static_cast<int>(protoWeaponType_), &caviar_);
    DesignAutoBuildActive = 0;

    vehdraw_construct_armor(ConsoleGlobal.faction_id_,
                            static_cast<int>(protoChassisType_),
                            static_cast<int>(protoWeaponType_),
                            static_cast<int>(protoArmorType_), 0, &caviar_);
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_,
                              static_cast<int>(protoChassisType_),
                              static_cast<int>(protoWeaponType_),
                              static_cast<int>(protoReactorType_),
                              static_cast<int>(protoAbilityFlags_), &caviar_);
    vehdraw_construct_abil(protoAbilityFlags_, &caviar_,
                           static_cast<int>(factionID_));

    protoChassisType_ = static_cast<uint32_t>(saved_chassis);
}

// ORIGINAL: 0x0043C0A0 ?construct_weapon@DesignWin@@QAEXXZ 0x0043C0A0-0x0043C0EF FILE BYTE_EXACT
// size      79 bytes
// prototype void (__thiscall ?construct_weapon@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C2CF0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043C0A0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043c0a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_weapon@DesignWin@@QAEXXZ  at 0x0043C0A0  (79 bytes)
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


void DesignWin::construct_weapon() {
    int weapon_id = protoWeaponType_;
    if (weapon_id == 0x15 && DesignAutoBuildActive == 0) {
        vehdraw_construct_weapon(factionID_, -1, 0x15, &caviar_);
        return;
    }
    vehdraw_construct_weapon(factionID_, protoChassisType_, weapon_id, &caviar_);
}

// Fixed-slot bindings carried from 0043c0f0.cpp

// ORIGINAL: 0x0043C0F0 ?construct_armor@DesignWin@@QAEXH@Z 0x0043C0F0-0x0043C126 FILE BYTE_EXACT
// size      54 bytes
// prototype void (__thiscall ?construct_armor@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C3090
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043C0F0
// measured tier  NO_COMPILE
// refusal        u0043c0f0.cpp(44) : error C2061: syntax error : identifier 'Caviar' u0043c0f0.cpp(63) : error C2061: syntax error : identifier 'Caviar'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043c0f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_armor@DesignWin@@QAEXH@Z  at 0x0043C0F0  (54 bytes)
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
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_armor@DesignWin@@QAEXH@Z  at 0x0043C0F0  (54 bytes)
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


void DesignWin::construct_armor(int a1) {
    vehdraw_construct_armor(ConsoleGlobal.faction_id_, protoChassisType_, protoWeaponType_,
                            protoArmorType_, a1, &caviar_);
}

// ORIGINAL: 0x0043C130 ?construct_reactor@DesignWin@@QAEXXZ 0x0043C130-0x0043C163 BYTE_EXACT
// size      51 bytes
// prototype void (__thiscall ?construct_reactor@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C3800
// 0x0043C130  ?construct_reactor@DesignWin@@QAEXXZ  ->  ?construct_reactor@DesignWin@@QAEXXZ
// LEVER: homed from 0043c130.cpp. Same named-field lever as construct_armor.
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::construct_reactor() {
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_, protoChassisType_, protoWeaponType_,
                              protoReactorType_, protoAbilityFlags_, &caviar_);
}

// Fixed-slot bindings carried from 00434650.cpp
// The vehdraw stat block the design-window renderer reads back: the four
// chosen component types as BYTES at 0x009AC558..0x009AC55B, the ability flag
// word at 0x009AC554 and a byte at 0x009AC562 whose only writer is this
// function's 0xFF. Each is an `int *const` / `uint8_t *const` at file scope,
// which folds to the bare immediate, so the image's
// `mov [0x9ac558], al` shape survives. No catalogue name for any of them;
// these are the roles, read off the three sites that touch the block
// (UNK3 here, draw_vehicle and draw_flash, both still unrecovered).
static int *const VehDrawAbilityFlags = (int *)0x009AC554;
static unsigned char *const VehDrawChassis = (unsigned char *)0x009AC558;
static unsigned char *const VehDrawWeapon = (unsigned char *)0x009AC559;
static unsigned char *const VehDrawArmor = (unsigned char *)0x009AC55A;
static unsigned char *const VehDrawReactor = (unsigned char *)0x009AC55B;
static unsigned char *const VehDrawComponentTail = (unsigned char *)0x009AC562;

// ORIGINAL: 0x00434650 ?UNK3@DesignWin@@QAEXXZ 0x00434650-0x00434691 FILE BYTE_EXACT
// size      65 bytes
// prototype void (__thiscall ?UNK3@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: homed from units/00434650.cpp. The vehdraw stat block's six raw
// bindings renamed (VehDrawChassis/Weapon/Armor/Reactor/AbilityFlags/
// ComponentTail) and the named DesignWin fields put in their place; the
// byte-width of each target is carried by the binding's own type, so no
// reinterpret_cast of `this` survives.
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00434650
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00434650/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK3@DesignWin@@QAEXXZ  at 0x00434650  (65 bytes)
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


void DesignWin::UNK3() {
    *VehDrawChassis = (unsigned char)protoChassisType_;
    *VehDrawWeapon = (unsigned char)protoWeaponType_;
    *VehDrawArmor = (unsigned char)protoArmorType_;
    *VehDrawReactor = (unsigned char)protoReactorType_;
    *VehDrawAbilityFlags = (int)protoAbilityFlags_;
    *VehDrawComponentTail = 0xFF;
}

// Fixed-slot bindings carried from 0043e960.cpp. 0x0073396C is the tutor
// state byte the exec/tutor path sets to 1/2 and clears here; 0x00733970 is
// the DesignWin *exec stores so timer_function can find the window again.
static int *const DesignTutorState = (int *)0x0073396C;
static DesignWin **const DesignWindowSelf = (DesignWin **)0x00733970;

// ORIGINAL: 0x0043E960 ?shut_that_badboy_down@DesignWin@@QAEXXZ 0x0043E960-0x0043E99D FILE
// size      61 bytes
// prototype void (__thiscall ?shut_that_badboy_down@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FCBB0 0x00616730 0x00616870
// indirect  0x0043E96F
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043E960
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043e960/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?shut_that_badboy_down@DesignWin@@QAEXXZ  at 0x0043E960  (61 bytes)
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


void DesignWin::shut_that_badboy_down() {
    if (*DesignTutorState != 1) {
        vtable_slot<func_design_win_slot002>(this, 0x8)(this);
        do_all_draws();
        Time *modal = &time_;
        *DesignWindowSelf = 0;
        modal->stop();
        modal->release_modal();
    }
    *DesignTutorState = 0;
}

// MISMATCH, ruled out (mnemonic_similarity 0.875, 65 vs 61 bytes):
// divergence #8, original 'lea' vs rebuilt 'add'+'mov', plus one inserted
// 'mov' before the second call. The original computes `this + 0x12774` ONCE
// into ecx (`lea ecx,[esi+0x12774]`) and reuses that same ecx, UNRELOADED,
// as the receiver for BOTH `modal->stop()` and `modal->release_modal()` -
// two back-to-back thiscall calls with a global store in between that does
// not touch ecx. Every VC6 phrasing tried here (named pointer local, named
// reference local, and the un-named expression repeated verbatim at both
// call sites, which CSEs to the same code) makes /O2 instead promote the
// address into esi (destructively overwriting `this`, which is dead by
// this point) and reload ecx with a 2-byte `mov ecx,esi` before EACH call -
// the textbook-safe thing to do, since a compiler that only sees `Time`
// as an opaque extern declaration cannot prove `Time::stop()` (66 bytes,
// calls timeKillEvent/KillTimer/flush_timer - genuinely not a leaf) leaves
// ecx holding the caller's value on return. src/time.h's
// `release_modal` declaration (00616870) is what this tree calls; the unit that
// carried that body was folded away (commit 619631a0), and it showed
// `Time::release_modal()` never touches `this` (`TimeModal =
// nullptr;`), so the no-reload choice the original compiler made is only
// explainable by whole-translation-unit visibility into that callee's body
// - something this scaffold's opaque-class model structurally cannot give
// it. Ruled out rather than re-derive: this is a same-shape sibling of the
// documented INSTRUCTION SCHEDULING wall, but for cross-call register
// liveness instead of instruction order.


// ==== coverage batch: 004345d0 / 00439960 / 00439a20 / 0043d490 ============

// ORIGINAL: 0x004345D0 ?UNK2@DesignWin@@QAEHH@Z 0x004345D0-0x00434649 FILE
// size      121 bytes
// prototype int (__thiscall ?UNK2@DesignWin@@QAEHH@Z)(DesignWin* this, int protoID)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// Homed from src/unrecovered/004345d0.cpp: the editor's "is this the same
// design" test - VehPrototypes[proto_id] against the component set the window
// currently holds. The seed's opaque `g_design_table` at 0x9AB888 is
// VehPrototypes shifted past veh_name: 0x9AB888 is [0].ability_flags and
// 0x9AB88C..[.F] the chassis/weapon/armor/reactor bytes.
int DesignWin::UNK2(int proto_id) {
    const VehPrototype &proto = VehPrototypes[proto_id];
    if (protoChassisType_ != proto.chassis_id) {
        return 0;
    }
    if (protoWeaponType_ != proto.weapon_id) {
        return 0;
    }
    if (protoArmorType_ != proto.armor_id) {
        return 0;
    }
    if (protoReactorType_ != proto.reactor_id) {
        return 0;
    }
    return protoAbilityFlags_ == proto.ability_flags;
}

// ORIGINAL: 0x00439960 ?on_left_click@DesignWin@@QAEXHH@Z 0x00439960-0x004399FA FILE
// size      154 bytes
// prototype void (__thiscall ?on_left_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int xCoord, int yCoord)
// callers   0   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00439F20 0x0043A300 0x0043A760 0x0043AA00 0x0043AC70 0x004BDDE0 0x005FAB00
// Homed from src/unrecovered/00439960.cpp: a click on the palette spots
// (spot2_) picks the component its type names; types 4 and 5 are the two
// special-ability slots.
void DesignWin::on_left_click(int xCoord, int yCoord) {
    TutWinGlobal->reset();
    int position;
    int type;
    if (spot2_.check(xCoord, yCoord, &position, &type) != -1) {
        switch (type) {
        case 0:
            select_chassis();
            return;
        case 1:
            select_weapon();
            return;
        case 2:
            select_armor();
            return;
        case 3:
            select_reactor();
            return;
        case 4:
            select_special(0);
            return;
        case 5:
            select_special(1);
            return;
        }
    }
}

// ORIGINAL: 0x00439A20 ?on_key_click@DesignWin@@QAEHHH@Z 0x00439A20-0x00439B3B FILE
// size      283 bytes
// prototype int (__thiscall ?on_key_click@DesignWin@@QAEHHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0043D510 0x004BDDE0
// indirect  0x00439A5D 0x00439AE1 0x00439AEF 0x00439B15 0x00439B2B
// Homed from src/unrecovered/00439a20.cpp: the second argument scrolls the
// prototype stack (page up/down/home/end by nines) or forwards to the front
// vtable's slot 44 (on_button_clicked) with 4/8/0x17/0x18; the first
// argument forwards 6 or 7 the same way. Always answers 1.
int DesignWin::on_key_click(int a1, int a2) {
    TutWinGlobal->reset();
    switch (a2) {
    case 0x0D:
        on_button_clicked(4);
        break;
    case 0x1B:
        on_button_clicked(8);
        break;
    case 0x21:
    case 0x69: {
        int moved = (field_CE4_ + *ProtoSortCount - 9) % *ProtoSortCount;
        field_CE4_ = (moved < 0) ? 0 : moved;
        draw_stack();
        break;
    }
    case 0x22:
    case 0x63: {
        int moved = (field_CE4_ + *ProtoSortCount + 9) % *ProtoSortCount;
        field_CE4_ = (moved < 0) ? 0 : moved;
        draw_stack();
        break;
    }
    case 0x23:
    case 0x61: {
        int bottom = *ProtoSortCount - 10;
        field_CE4_ = (bottom < 0) ? 0 : bottom;
        draw_stack();
        break;
    }
    case 0x24:
    case 0x67:
        field_CE4_ = 0;
        draw_stack();
        break;
    case 0x25:
    case 0x64:
        on_button_clicked(0x17);
        break;
    case 0x27:
    case 0x66:
        on_button_clicked(0x18);
        break;
    }
    switch (a1) {
    case 0x4D:
    case 0x4F:
    case 0x6D:
    case 0x6F:
        on_button_clicked(6);
        break;
    case 0x55:
    case 0x75:
        on_button_clicked(7);
        break;
    }
    return 1;
}

// ORIGINAL: 0x0043D490 ?center_stack@DesignWin@@QAEXHH@Z 0x0043D490-0x0043D50B FILE
// size      123 bytes
// prototype void (__thiscall ?center_stack@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0043D510 0x005A63D0
// Homed from src/unrecovered/0043d490.cpp: re-rank the faction's prototypes,
// find proto_id in the ranked list and move the stack window's anchor
// (field_CE4_) so that entry sits five rows from the top.
void DesignWin::center_stack(int proto_id, int redraw) {
    // proto_sort_2 (0x005A63D0, catalogued ?proto_sort_2@@YAXH@Z) is unhomed;
    // its sibling proto_sort lives in map.cpp, which also owns the
    // map.cpp sort pair this body reads. Called through the
    // original body's address like the other pending seams.
    typedef void(__cdecl *proto_sort_2_seam)(int);
    reinterpret_cast<proto_sort_2_seam>(static_cast<unsigned long>(0x005A63D0))(
        factionID_);

    int count = *ProtoSortCount;
    int start = field_CE4_;
    bool found = false;
    int rem = 0;
    if (start < start + count) {
        int scan = start;
        do {
            rem = scan % count;
            if (proto_id == ProtoSortIds[rem]) {
                found = true;
                break;
            }
            ++scan;
        } while (scan < field_CE4_ + count);
    }
    if (found) {
        int old = field_CE4_;
        int centred = (rem + count - 5) % count;
        field_CE4_ = centred;
        if (redraw != 0 && centred != old) {
            draw_stack();
        }
    }
}

// ---- component picker -----------------------------------------------------
template <typename Record>
static int picker_row_before(int selected, const Record *records, int count) {
    int row = 0;
    for (int index = 0; index < count; ++index) {
        if (records[index].preq_tech >= -1) {
            if (index == selected) return row;
            ++row;
        }
    }
    return row;
}

template <typename Record>
static int picker_record_at(int row, const Record *records, int count) {
    for (int index = 0; index < count; ++index) {
        if (records[index].preq_tech >= -1) {
            if (row == 0) return index;
            --row;
        }
    }
    return count;
}

static const int DesignSelectableChassisCount = CHSI_MISSILE + 1;

static void redraw_design_picker_changes(DesignWin *window) {
    window->check_abil();
    window->draw_chassis(0);
    window->draw_armor(0);
    window->draw_reactor(0);
    window->draw_weapon(0);
    window->draw_vehicle(0);
    window->draw_special_1(0);
    window->draw_special_2(0);
    window->draw_flash();
    window->draw_info();
    window->on_update();
}

// check_abil uses the class's named selection state and the project Rules
// object rather than reproducing the seed's byte-offset view.
// ORIGINAL: 0x00439C00 ?check_abil@DesignWin@@QAEXXZ 0x00439C00-0x00439D24
// RECOVERY_BINDING: v1 {"evidence": ["DesignWin selection members in src/designwin.h", "abil_cond and bit_count in src/leaf_recoveries.cpp", "Rules.tech_two_spec_abilities in src/alpha.h"], "reason": "Validate each selected ability against the current chassis/weapon/armor and maintain the two displayed ability slots."}
void DesignWin::check_abil() {
    if (Chassis[protoChassisType_].missile != 0) {
        protoAbilityFlags_ = 0;
        protoAbilityID1_ = static_cast<uint32_t>(-1);
        protoAbilityID2_ = static_cast<uint32_t>(-1);
        return;
    }

    for (int ability_id = 0; ability_id < 0x1D; ++ability_id) {
        const uint32_t bit = 1u << ability_id;
        if ((protoAbilityFlags_ & bit) == 0) {
            continue;
        }

        if ((bit == 0x80 && (protoAbilityFlags_ & 0x40) != 0)
            || !abil_cond(ability_id, static_cast<int>(protoChassisType_),
                          static_cast<int>(protoWeaponType_),
                          static_cast<int>(protoArmorType_))
            || (bit_count(static_cast<int>(protoAbilityFlags_)) > 1
                && (bit_count(static_cast<int>(protoAbilityFlags_)) != 2
                    || !has_tech(Rules.tech_two_spec_abilities,
                                 static_cast<int>(factionID_))))) {
            protoAbilityFlags_ &= ~bit;
            protoAbilityID1_ = static_cast<uint32_t>(-1);
            protoAbilityID2_ = static_cast<uint32_t>(-1);
            for (int candidate = 0; candidate < 0x1D; ++candidate) {
                if ((protoAbilityFlags_ & (1u << candidate)) == 0) {
                    continue;
                }
                if (protoAbilityID1_ == static_cast<uint32_t>(-1)) {
                    protoAbilityID1_ = static_cast<uint32_t>(candidate);
                } else if (protoAbilityID2_ == static_cast<uint32_t>(-1)) {
                    protoAbilityID2_ = static_cast<uint32_t>(candidate);
                }
            }
        }
    }
}

// ORIGINAL: 0x00439D30 ?suggest_abil@DesignWin@@QAEXH@Z 0x00439D30-0x00439EA3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00439d30.cpp"], "reason": "Link-continuity placeholder only; retain the seed until this original body is migrated."}
void DesignWin::suggest_abil(int ability) {
    (void)ability;
    // BODY GOES HERE.
}

// ORIGINAL: 0x00434BC0 ?draw_chassis@DesignWin@@QAEXH@Z 0x00434BC0-0x00434F36
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00434bc0.cpp", "docs/recovery/member-accesses.csv"], "reason": "Partial typed recovery draws the chassis panel caption, selected chassis name and model preview; the seed retains unresolved specification text and palette behavior."}
void DesignWin::draw_chassis(int redraw) {
    (void)redraw;
    Buffer *canvas = static_cast<Buffer *>(this);
    RECT preview = chassis_panel_rect_;
    canvas->set_clip(&preview);
    buffer_.copy(canvas, &preview);

    const int title_height = font2_.unk_1_ < 0
        ? font2_.line_height_ : font2_.height_ + font2_.unk_1_;
    RECT title = preview;
    title.bottom = title.top + title_height;
    canvas->set_text_color(0xE0, -1, 1, 1);
    char *caption = label_get(0x140 / 4);
    canvas->write_cent_l(&font2_, caption, &title,
                         static_cast<int>(strlen(caption)));

    const int chassis = static_cast<int>(protoChassisType_);
    char *name = Chassis[chassis].offsv_1_name;
    if (name != 0) {
        RECT name_area = preview;
        name_area.top += title_height;
        name_area.bottom = name_area.top + font1_.height_;
        canvas->write_cent_l(&font1_, name, &name_area,
                             static_cast<int>(strlen(name)));
    }

    preview.top += title_height + font1_.height_;
    preview.bottom -= static_cast<int>(font1_.height_) * 2 + 5;
    vehdraw_construct_chassis(static_cast<int>(factionID_), chassis,
                              static_cast<int>(protoWeaponType_), 0,
                              static_cast<int>(protoReactorType_), &caviar_);
    caviar_.set_scaling(0.05f);
    caviar_.set_scene_rotation(0.5233333706855774f, 0, 0);
    caviar_.render(canvas, (preview.left + preview.right) / 2,
                   (preview.top + preview.bottom) / 2, 2);
    // BODY GOES HERE. The seed's remaining chassis specification lines and
    // their distinct palette are not yet represented by typed project data.
}

// ORIGINAL: 0x00434F40 ?draw_weapon@DesignWin@@QAEXH@Z 0x00434F40-0x004353FF
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00434f40.cpp", "docs/recovery/member-accesses.csv"], "reason": "Partial typed recovery draws the weapon panel caption, selected weapon name and model preview; the seed retains unresolved class, range and ammunition text and palette behavior."}
void DesignWin::draw_weapon(int redraw) {
    (void)redraw;
    Buffer *canvas = static_cast<Buffer *>(this);
    RECT preview = weapon_panel_rect_;
    canvas->set_clip(&preview);
    buffer_.copy(canvas, &preview);

    const int weapon = static_cast<int>(protoWeaponType_);
    const int title_height = font2_.unk_1_ < 0
        ? font2_.line_height_ : font2_.height_ + font2_.unk_1_;
    RECT title = preview;
    title.bottom = title.top + title_height;
    canvas->set_text_color(0xE0, -1, 1, 1);
    char *name = label_get((Weapon[weapon].offense_rating == 0
                                ? 0x200 : 0x204) / 4);
    if (name != 0) {
        canvas->write_cent_l(&font2_, name, &title,
                             static_cast<int>(strlen(name)));
    }

    RECT name_area = preview;
    name_area.top += title_height;
    name_area.bottom = name_area.top + font1_.height_;
    if (weapon != 0 && Weapon[weapon].name != 0) {
        canvas->write_cent_l(&font1_, Weapon[weapon].name, &name_area,
                             static_cast<int>(strlen(Weapon[weapon].name)));
    }

    preview.top += title_height + font1_.height_;
    preview.bottom -= static_cast<int>(font1_.height_) * 2 + 5;
    if (weapon == 0) {
        CaviarData empty_weapon;
        caviar_.add_object(&empty_weapon, 0);
        caviar_.set_scaling(0.15f);
    } else {
        vehdraw_construct_weapon(static_cast<int>(factionID_),
                                 static_cast<int>(protoChassisType_), weapon,
                                 &caviar_);
        const uint8_t rarity = Weapon[weapon].mode;
        caviar_.set_scaling(rarity < 3 ? 0.15f : (rarity == 9 ? 0.06f : 0.08f));
    }
    caviar_.set_scene_rotation(0.5323889f, 0.0f, 0.0f);
    caviar_.render(canvas, (preview.left + preview.right) / 2,
                   (preview.top + preview.bottom) / 2, 6);
    // BODY GOES HERE. Remaining weapon class/range or ammunition lines and
    // their original palette still need typed source bindings.
}

// ORIGINAL: 0x00435400 ?draw_armor@DesignWin@@QAEXH@Z 0x00435400-0x00435800
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00435400.cpp", "docs/recovery/member-accesses.csv"], "reason": "Partial typed recovery draws the armor panel caption and selected armor model preview; the seed retains unresolved armor detail text, palette and alternate render behavior."}
void DesignWin::draw_armor(int redraw) {
    (void)redraw;
    Buffer *canvas = static_cast<Buffer *>(this);
    RECT preview = armor_panel_rect_;
    canvas->set_clip(&preview);
    buffer_.copy(canvas, &preview);

    const int title_height = font2_.unk_1_ < 0
        ? font2_.line_height_ : font2_.height_ + font2_.unk_1_;
    RECT title = preview;
    title.bottom = title.top + title_height;
    char *caption = label_get(0x1F8 / 4);
    canvas->set_text_color(0xE0, -1, 1, 1);
    canvas->write_cent_l(&font2_, caption, &title,
                         static_cast<int>(strlen(caption)));

    const int armor = static_cast<int>(protoArmorType_);
    if (armor != 0) {
        preview.top += title_height;
        preview.bottom -= static_cast<int>(font1_.height_) * 2 + 5;
        vehdraw_construct_armor(static_cast<int>(factionID_),
                                static_cast<int>(protoChassisType_),
                                static_cast<int>(protoWeaponType_), armor, 1,
                                &caviar_);
        caviar_.set_scaling(0.07f);
        caviar_.set_scene_rotation(0.5233333706855774f, 0, 0);
        caviar_.render(canvas, (preview.left + preview.right) / 2,
                       (preview.top + preview.bottom) / 2, 6);
    }
    // BODY GOES HERE. Armor rating text and the special multi-table render
    // path need the still-unmodelled stat/palette resources.
}

// ORIGINAL: 0x00435800 ?draw_reactor@DesignWin@@QAEXH@Z 0x00435800-0x00435B29
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00435800.cpp", "docs/recovery/member-accesses.csv"], "reason": "Partial typed recovery draws the reactor panel caption, selected reactor name and model preview; the seed retains unresolved ability-aware description, numeric detail and palette behavior."}
void DesignWin::draw_reactor(int redraw) {
    (void)redraw;
    Buffer *canvas = static_cast<Buffer *>(this);
    RECT preview = reactor_panel_rect_;
    canvas->set_clip(&preview);
    buffer_.copy(canvas, &preview);

    const int title_height = font2_.unk_1_ < 0
        ? font2_.line_height_ : font2_.height_ + font2_.unk_1_;
    RECT title = preview;
    title.bottom = title.top + title_height;
    char *caption = label_get(0x13C / 4);
    canvas->set_text_color(0xE0, -1, 1, 1);
    canvas->write_cent_l(&font2_, caption, &title,
                         static_cast<int>(strlen(caption)));

    const int reactor = static_cast<int>(protoReactorType_);
    if (reactor >= 0 && reactor < MaxReactorNum) {
        char *name = Reactor[reactor].name;
        RECT name_area = preview;
        name_area.top += title_height;
        name_area.bottom = name_area.top + font1_.height_;
        if (name != 0) {
            canvas->write_cent_l(&font1_, name, &name_area,
                                 static_cast<int>(strlen(name)));
        }

        preview.top += title_height + font1_.height_;
        preview.bottom -= static_cast<int>(font1_.height_) * 2 + 5;
        vehdraw_construct_reactor(static_cast<int>(factionID_),
                                  static_cast<int>(protoChassisType_),
                                  static_cast<int>(protoWeaponType_), reactor,
                                  protoAbilityFlags_, &caviar_);
        caviar_.set_scaling(0.2f);
        caviar_.set_scene_rotation(0.5233333706855774f, 0, 0);
        caviar_.render(canvas, (preview.left + preview.right) / 2,
                       (preview.top + preview.bottom) / 2, 6);
    }
    // BODY GOES HERE. Ability-aware reactor description and numeric detail
    // text remain unresolved until their label/stat resources are modelled.
}

// ORIGINAL: 0x00435B30 ?draw_special_1@DesignWin@@QAEXH@Z 0x00435B30-0x00435DD1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00435b30.cpp"], "reason": "Link-continuity placeholder only; retain the seed until this original body is migrated."}
void DesignWin::draw_special_1(int redraw) {
    (void)redraw;
    // BODY GOES HERE.
}

// ORIGINAL: 0x00435DE0 ?draw_special_2@DesignWin@@QAEXH@Z 0x00435DE0-0x00436081 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00435de0.cpp"], "reason": "Link-continuity placeholder only; retain the seed until this original body is migrated."}
void DesignWin::draw_special_2(int redraw) {
    (void)redraw;
    // BODY GOES HERE.
}

// ORIGINAL: 0x00436090 ?draw_vehicle@DesignWin@@QAEXH@Z 0x00436090-0x00437999 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00436090.cpp"], "reason": "Link-continuity placeholder only; retain the seed until this original body is migrated."}
void DesignWin::draw_vehicle(int redraw) {
    (void)redraw;
    // BODY GOES HERE.
}

// ORIGINAL: 0x004346A0 ?draw_info@DesignWin@@QAEXXZ 0x004346A0-0x00434BB4;0x00653FFE-0x00654010 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004346a0.cpp"], "reason": "Link-continuity placeholder only; retain the seed until this original body is migrated."}
void DesignWin::draw_info() {
    // BODY GOES HERE.
}

// ORIGINAL: 0x00439F20 ?select_chassis@DesignWin@@QAEXXZ 0x00439F20-0x0043A300;0x006541FC-0x00654281
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/recovered/units/00439f20.cpp", "RulesChassis in src/veh.h", "DesignWin component members in src/designwin.h", "SelectPartWin::exec in src/selectpartwin.cpp"], "reason": "Open the chassis picker, translate its eligible-row result into the chassis table, preserve chassis-dependent weapon and ability constraints, and redraw the design preview."}
void DesignWin::select_chassis() {
    SelectPartWin picker;
    const int row = picker_row_before(protoChassisType_, Chassis,
                                      DesignSelectableChassisCount);
    const int selected_row = picker.exec(
        outer_rect_.left, outer_rect_.top, factionID_, 0, row,
        protoChassisType_, protoWeaponType_, protoArmorType_,
        protoAbilityFlags_, protoReactorType_, 0);
    const int previous_kind = Chassis[protoChassisType_].triad;
    const int next_chassis = picker_record_at(selected_row, Chassis,
                                              DesignSelectableChassisCount);
    protoChassisType_ = next_chassis;

    if (Chassis[next_chassis].missile == 0) {
        protoArmorType_ = 0;
        const int weapon_kind = static_cast<unsigned char>(
            Weapon[protoWeaponType_].offense_rating);
        if (weapon_kind >= 2) protoWeaponType_ = 0;
        if (protoWeaponType_ != 0x10 && protoWeaponType_ != 0x18 &&
            protoWeaponType_ != 0x19) {
            protoWeaponType_ = 0x17;
        }
    } else {
        const int weapon_kind = static_cast<unsigned char>(
            Weapon[protoWeaponType_].offense_rating);
        if (weapon_kind >= 0x63 || protoWeaponType_ == 0x17 ||
            protoWeaponType_ == 0x18 || protoWeaponType_ == 0x19) {
            protoWeaponType_ = 0;
        }
    }

    const int next_kind = Chassis[next_chassis].triad;
    if (next_kind != previous_kind) {
        if (next_kind != 0) {
            suggest_abil(2);
            if (next_kind == 2) {
                if (Weapon[protoWeaponType_].mode == 0x0b) {
                    protoWeaponType_ = 0;
                }
                if (Weapon[protoWeaponType_].mode == 9 &&
                    Chassis[next_chassis].range != 0) {
                    protoWeaponType_ = 0;
                }
            }
        } else if ((protoAbilityFlags_ & 2) != 0) {
            protoAbilityFlags_ &= ~2U;
            protoAbilityID1_ = -1;
            protoAbilityID2_ = -1;
            for (int bit = 0; bit < 0x1d; ++bit) {
                if ((protoAbilityFlags_ & (1U << bit)) == 0) continue;
                if (static_cast<int32_t>(protoAbilityID1_) < 0) {
                    protoAbilityID1_ = bit;
                } else if (static_cast<int32_t>(protoAbilityID2_) < 0) {
                    protoAbilityID2_ = bit;
                }
            }
        }
    }
    redraw_design_picker_changes(this);
}

// ORIGINAL: 0x0043A300 ?select_weapon@DesignWin@@QAEXXZ 0x0043A300-0x0043A754;0x00654281-0x00654306
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/recovered/units/0043a300.cpp", "RulesWeapon/RulesArmor in src/veh.h", "DesignWin component members in src/designwin.h", "SelectPartWin::exec in src/selectpartwin.cpp"], "reason": "Open the weapon picker, apply the selected record's chassis restrictions, update implicit abilities, and redraw the design preview."}
void DesignWin::select_weapon() {
    SelectPartWin picker;
    const int row = picker_row_before(protoWeaponType_, Weapon, MaxWeaponNum);
    const int selected_row = picker.exec(
        outer_rect_.left, outer_rect_.top, factionID_, 1, row,
        protoChassisType_, protoWeaponType_, protoArmorType_,
        protoAbilityFlags_, protoReactorType_, 0);
    const int previous_weapon = protoWeaponType_;
    const int selected_weapon = picker_record_at(selected_row, Weapon,
                                                 MaxWeaponNum);
    protoWeaponType_ = selected_weapon;
    const int chassis_kind = Chassis[protoChassisType_].triad;

    if (chassis_kind == 2) {
        if (Weapon[selected_weapon].mode == 0x0b) {
            protoWeaponType_ = previous_weapon;
        }
        if (Weapon[protoWeaponType_].mode == 9 &&
            Chassis[protoChassisType_].range != 0) {
            protoWeaponType_ = previous_weapon;
        }
        const int compatibility = Chassis[protoChassisType_].missile;
        if (compatibility != 0 && protoWeaponType_ != 0x10 &&
            protoWeaponType_ != 0x18 && protoWeaponType_ != 0x19) {
            protoWeaponType_ = 0x17;
        } else if (compatibility == 0 &&
                   (protoWeaponType_ == 0x10 || protoWeaponType_ == 0x17 ||
                    protoWeaponType_ == 0x18)) {
            protoWeaponType_ = previous_weapon;
        }
    } else if (selected_weapon == 0x10 || protoWeaponType_ == 0x17 ||
               protoWeaponType_ == 0x18) {
        protoWeaponType_ = previous_weapon;
    }
    if (protoWeaponType_ == 0x19) protoWeaponType_ = previous_weapon;

    if (static_cast<unsigned char>(
            Weapon[protoWeaponType_].offense_rating) > 'b') {
        protoAbilityID1_ = -1;
        protoAbilityID2_ = -1;
        protoAbilityFlags_ = 0;
    }
    if (protoAbilityFlags_ == 0 && protoWeaponType_ != previous_weapon) {
        if (Weapon[protoWeaponType_].mode == 7) suggest_abil(0x80000);
        if (Weapon[protoWeaponType_].mode == 9) {
            suggest_abil(1);
            suggest_abil(0x2000);
        }
        if (Weapon[protoWeaponType_].offense_rating >=
            Armor[protoArmorType_].defense_rating * 2) {
            suggest_abil(0x20000);
        }
    }
    redraw_design_picker_changes(this);
}

// ORIGINAL: 0x0043A760 ?select_armor@DesignWin@@QAEXXZ 0x0043A760-0x0043A9F8;0x00654306-0x0065438B
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/recovered/units/0043a760.cpp", "RulesArmor/RulesWeapon in src/veh.h", "DesignWin component members in src/designwin.h", "SelectPartWin::exec in src/selectpartwin.cpp"], "reason": "Open the armor picker, preserve the armor/weapon compatibility suggestion, and redraw the design preview."}
void DesignWin::select_armor() {
    SelectPartWin picker;
    const int row = picker_row_before(protoArmorType_, Armor, MaxArmorNum);
    const int selected_row = picker.exec(
        outer_rect_.left, outer_rect_.top, factionID_, 2, row,
        protoChassisType_, protoWeaponType_, protoArmorType_,
        protoAbilityFlags_, protoReactorType_, 0);
    protoArmorType_ = picker_record_at(selected_row, Armor, MaxArmorNum);
    if (Armor[protoArmorType_].defense_rating > 1 &&
        Weapon[protoWeaponType_].offense_rating <= 1) {
        suggest_abil(0x200);
    }
    redraw_design_picker_changes(this);
}

// ORIGINAL: 0x0043AA00 ?select_reactor@DesignWin@@QAEXXZ 0x0043AA00-0x0043AC69;0x0065438B-0x00654410
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a:src/unrecovered/0043aa00.cpp", "RulesReactor in src/veh.h", "DesignWin component members in src/designwin.h", "SelectPartWin::exec in src/selectpartwin.cpp"], "reason": "Open the reactor picker, translate its enabled-row result, update the reactor selection, and redraw the design preview."}
void DesignWin::select_reactor() {
    SelectPartWin picker;
    const int row = picker_row_before(protoReactorType_, Reactor,
                                      MaxReactorNum);
    const int selected_row = picker.exec(
        protoReactorType_, protoAbilityFlags_, protoArmorType_, 3,
        protoChassisType_, row, 3, factionID_, outer_rect_.top,
        outer_rect_.left, 0);
    protoReactorType_ = picker_record_at(selected_row - 1, Reactor,
                                         MaxReactorNum);
    redraw_design_picker_changes(this);
}

// ORIGINAL: 0x0043D510 ?draw_stack@DesignWin@@QAEXXZ 0x0043D510-0x0043DD43;0x006546DA-0x006546EC FILE
// size      2117 bytes
// prototype void (__thiscall ?draw_stack@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   5   call targets   25
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005594F0 0x0057D7D0 0x005A63D0 0x005B9F20 0x005C1D50 0x005D5930 0x005D8000 0x005DA860 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005DDAB0 0x005E2DD3 0x005FA960 0x005FAA90 0x006169A0 0x00618EA0 0x00618EE0 0x00618F40 0x00619280 0x006453E0 0x00645470 0x00645DD0 0x0064FC88
// Homed from src/unrecovered/0043d510.cpp: the prototype stack panel - tile
// the background, frame it, then walk the ranked prototype list drawing up to
// ten entries (box style 2 = the edited prototype, 1 = the highlighted one),
// registering each spot for on_left_click.
void DesignWin::draw_stack() {
    Font font;
    RECT area = *reinterpret_cast<RECT *>(&field_B0C_);
    Buffer *canvas = reinterpret_cast<Buffer *>(InterfaceBuffer);
    canvas->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));
    canvas->tile(*StackTileBufferSlot, area.left, area.top, area.left, area.top,
                 area.right - area.left, area.bottom - area.top);
    canvas->box_sprite(&area, &InterfaceBoxStyles[3]);

    RECT rc = area;
    rc.bottom -= 3;
    rc.right = static_cast<int>(field_CCC_) - 3;
    int span = rc.right - (static_cast<int>(field_CC4_) + 3) + 2;
    int spacer = span / 10 - 2;
    int x_base = static_cast<int>(field_CC4_) + 3;
    rc.left = x_base;
    int y_base = rc.top + 3;
    rc.top = y_base;

    font.init(DefaultFontFace, 10, 0);
    canvas->set_text_color(0x9F, -1, 1, 1);
    canvas->set_font(&font, 0, 0, 0);
    spot1_.kill_type(0);

    rc.bottom = rc.bottom - rc.top + y_base;
    rc.left = x_base;
    rc.top = y_base;
    rc.right = x_base + spacer;

    // proto_sort_2 (0x005A63D0, catalogued ?proto_sort_2@@YAXH@Z) is unhomed;
    // its sibling proto_sort lives in map.cpp, which also owns the
    // map.cpp sort pair. Called through the original body's
    // address like the other pending seams.
    typedef void(__cdecl *proto_sort_2_seam)(int);
    reinterpret_cast<proto_sort_2_seam>(static_cast<unsigned long>(0x005A63D0))(
        factionID_);

    int divisor = *ProtoSortCount;
    int entry = static_cast<int>(field_CE4_);
    int loop_end = entry + divisor;
    int drawn = 0;
    while (drawn < 10 && entry < loop_end) {
        int proto_id = ProtoSortIds[entry % divisor];
        ++entry;
        // Validity: own faction prototypes need their tech and an offerable
        // weapon class (and never slot 0xF); borrowed ones only their faction.
        if (proto_id / 64 == 0) {
            if (!has_tech(VehPrototypes[proto_id].preq_tech, factionID_)) {
                continue;
            }
            if (proto_id < 0x40) {
                if (static_cast<signed char>(
                        WeaponClassRows[VehPrototypes[proto_id].weapon_id * 16]) < 0) {
                    continue;
                }
                if (proto_id == 0xF) {
                    continue;
                }
            }
        } else if (proto_id / 64 != factionID_) {
            continue;
        }

        int selector = 0;
        if (proto_id == static_cast<int>(protoID_)) {
            selector = 2;
        } else if (proto_id == static_cast<int>(field_CE0_)) {
            selector = 1;
        }
        ++drawn;
        canvas->box_sprite(&rc, &InterfaceBoxStyles[selector]);
        spot1_.add(proto_id, 0, rc.left, rc.top, rc.right - rc.left,
                   rc.bottom - rc.top);
        rc.left += 2;
        rc.top += 2;
        rc.right -= 2;
        rc.bottom -= 2;
        canvas->set_clip(&rc);

        if (VehPrototypes[proto_id].flags & PROTO_ACTIVE) {
            int unit_id = veh_fake(proto_id, factionID_);
            StringTemp[0] = '\0';
            veh_draw(canvas, unit_id, x_base + 0xF, y_base + 0xC, -10, 0x1048, 0);

            RECT label_rect = rc;
            canvas->set_text_color(0x9F, -1, 1, 1);
            StringTemp[0] = '\0';
            if (proto_id < 0x40) {
                if (VehPrototypes[proto_id].obsolete_factions
                    & (1 << (factionID_ & 0x1F))) {
                    canvas->set_text_color(0xF8, -1, 1, 1);
                    // The label id the interface keeps in the game block at
                    // Labels.strings_ptr+0xE40 (slot 0x390) - same walk as
                    // on_iface_right_click's 0xCE8.
                    strcat(StringTemp, StringTable.get((int)reinterpret_cast<LPSTR *>(
                        Labels.strings_ptr)[0x390]));
                } else {
                    strcat(StringTemp, VehPrototypes[proto_id].veh_name);
                }
            } else if (VehPrototypes[proto_id].obsolete_factions
                & (1 << (factionID_ & 0x1F))) {
                unsigned char seen =
                    PlayersData[factionID_].proto_id_active[proto_id];
                canvas->set_text_color(0xF8 + ((-(seen != 0)) & 7), -1, 1, 1);
                strcat(StringTemp, StringTable.get((int)reinterpret_cast<LPSTR *>(
                    Labels.strings_ptr)[0x390]));
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                char digits[0x20];
                _itoa(proto_id % 64 + 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, EditorCloseParen);
            } else {
                char digits[0x20];
                _itoa(proto_id % 64 + 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, EditorDotSpace);
                strcat(StringTemp, VehPrototypes[proto_id].veh_name);
            }

            label_rect.left += 3;
            label_rect.right -= 3;
            if (StringTemp[0] != '\0') {
                while (font.width(StringTemp)
                       > label_rect.right - label_rect.left) {
                    StringTemp[strlen(StringTemp) - 1] = '\0';
                    if (StringTemp[0] == '\0') {
                        break;
                    }
                }
            }
            char *first_space = strchr(StringTemp, ' ');
            if (first_space != 0) {
                char *second_space = strchr(first_space + 1, ' ');
                if (second_space != 0) {
                    *second_space = '\0';
                }
            }
            label_rect.bottom = label_rect.top + font.line_advance();
            canvas->set_clip(&label_rect);
            canvas->write_l(StringTemp, &label_rect, strlen(StringTemp));
            canvas->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));

            RECT stats_rect = rc;
            stats_rect.top = rc.bottom - font.line_advance();
            StringTemp[0] = '\0';
            // say_stats_3 (0x0057D7D0) is declared in veh.h but not homed.
            typedef void(__cdecl *say_stats_3_seam)(char *, unsigned int);
            reinterpret_cast<say_stats_3_seam>(
                static_cast<unsigned long>(0x0057D7D0))(
                StringTemp,
                *reinterpret_cast<const short *>(ProtoAbilityIdBase
                                                 + unit_id * 0x34));
            if ((*InterfaceFlagByte & 0x40) != 0
                && (*InterfaceFlagTail & 0x10) != 0) {
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                char digits[0x20];
                _itoa(proto_id, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, EditorCloseParen);
            }
            canvas->write_cent_l(StringTemp, &stats_rect, strlen(StringTemp));
            spot1_.add(proto_id, 0, rc.left, rc.top, rc.right - rc.left,
                       rc.bottom - rc.top);

            x_base += spacer + 2;
            rc.top -= 2;
            rc.bottom += 2;
            rc.left += spacer;
            rc.right += spacer + 4;
        } else {
            StringTemp[0] = '\0';
            // Slot 0x6E of the same label block: the "undesigned" caption.
            strcat(StringTemp, StringTable.get((int)reinterpret_cast<LPSTR *>(
                Labels.strings_ptr)[0x6E]));
            strcat(StringTemp, SpaceText);
            strcat(StringTemp, EditorHashSign);
            char digits[0x20];
            _itoa(proto_id % 64 + 1, digits, 10);
            strcat(StringTemp, digits);
            canvas->set_text_color(0x9F, -1, 1, 1);
            canvas->set_font(&font, 0, 0, 0);
            canvas->wrap_cent(StringTemp, &rc);

            x_base += spacer + 2;
            rc.top -= 2;
            rc.bottom += 2;
            rc.left += spacer;
            rc.right += spacer + 4;
            canvas->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));
        }
    }

    MainInterfaceGlobal.soft_update(&area);
    canvas->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));
}


// ORIGINAL: 0x00437BF0 ?design_it@DesignWin@@QAEHXZ 0x00437BF0-0x00438455 FILE
// size      2149 bytes
// prototype int (__thiscall ?design_it@DesignWin@@QAEHXZ)(DesignWin* this)
// callers   1   call targets   17
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C0A0 0x00531300 0x00539820 0x005398C0 0x0057E040 0x0057F1D0 0x0057F4B0 0x00592EE0 0x005933F0 0x005A5D40 0x005B9F20 0x005BF330 0x00625E30 0x00625EC0 0x006453E0 0x00645460 0x00645660
// Homed from src/unrecovered/00437bf0.cpp: commit the editor selection as a
// prototype. The scan covers the faction's own 64 slots plus the 64 borrowed
// ones; a slot that already holds exactly this design is re-selected (and
// un-obsoleted), otherwise the current slot is rebuilt, a free one is taken,
// or an occupied one is overwritten behind its confirmation popup.
int DesignWin::design_it() {
    // Unhomed callees, called through the original bodies' addresses like the
    // other pending seams: name_proto (0x0057E040), retire_proto (0x0057F1D0)
    // and prune_protos (0x0057F4B0) have no tree homes yet (their sibling
    // make_proto is veh.h's).
    typedef void(__cdecl *name_proto_seam)(char *, int, int, int, int, int, int, int);
    typedef void(__cdecl *retire_proto_seam)(int, int);
    typedef void(__cdecl *prune_protos_seam)(int, int, int);
    typedef int(__cdecl *diplo_lock_seam)(int);
    typedef void(__cdecl *diplo_unlock_seam)();

    char *name = reinterpret_cast<char *>(&field_14220_);

    // ---- scan all 128 slots (own 64 + up to 64 borrowed) ------------------
    int scan = 0;
    do {
        int slot = scan;
        if (slot > 0x3F) {
            slot = factionID_ * 0x40 + slot % 0x40;
        }
        VehPrototype &proto = VehPrototypes[slot];
        bool fields_match = (proto.flags & PROTO_ACTIVE) != 0
            && protoChassisType_ == proto.chassis_id
            && protoWeaponType_ == proto.weapon_id
            && protoArmorType_ == proto.armor_id
            && protoReactorType_ == proto.reactor_id
            && protoAbilityFlags_ == proto.ability_flags
            && (slot > 0x3F || has_tech(proto.preq_tech, factionID_));

        if (fields_match) {
            if (slot != static_cast<int>(protoID_)) {
                if (proto.obsolete_factions & (1 << (factionID_ & 0x1F))) {
                    // Flag bit 0x8 has no name in veh.h's enum yet: it marks
                    // a prototype the faction re-designed over.
                    proto.flags |= 8;
                }
                proto.obsolete_factions &= ~(1 << (factionID_ & 0x1F));
            }
            if (slot < 0x40) {
                if (strlen(name) != 0
                    && strcmp(name, proto.veh_name) != 0) {
                    parse_says(0, proto.veh_name, -1, -1);
                    if ((*PlayerFlagBlocks & 0x80) == 0) {
                        popp(ScriptTxtID, EditorNamePopupLabel, 0,
                             EditorNamePopupSprite, pop_wait);
                    } else {
                        popp(ScriptTxtID, EditorNamePopupAlienLabel, 0,
                             EditorNamePopupAlienSprite, pop_wait);
                    }
                }
            } else if (strlen(name) != 0
                && (VehPrototypes[protoID_].flags & PROTO_CUSTOM_NAME_SET)) {
                proto.flags |= PROTO_CUSTOM_NAME_SET;
                strcpy(proto.veh_name, name);
            }
            synch_proto(slot);
            protoID_ = slot;
            return 1;
        }
        ++scan;
    } while (scan < 0x80);

    // ---- no exact match: fall back to the currently-selected slot ---------
    if (protoID_ < 0x40) {
        const VehPrototype &current = VehPrototypes[protoID_];
        bool current_match = protoChassisType_ == current.chassis_id
            && protoWeaponType_ == current.weapon_id
            && protoArmorType_ == current.armor_id
            && protoAbilityFlags_ == current.ability_flags
            && protoReactorType_ == current.reactor_id;
        if (current_match) {
            if (strlen(name) == 0) {
                return 0;
            }
            if (strcmp(name, current.veh_name) == 0) {
                return 0;
            }
            parse_says(0, current.veh_name, -1, -1);
            if ((*PlayerFlagBlocks & 0x80) == 0) {
                x_pop_caption_label(EditorNameXPopCaption,
                                    EditorNameXPopSprite, pop_wait);
            } else {
                x_pop_caption_label(EditorNameXPopAlienCaption,
                                    EditorNameXPopAlienSprite,
                                    pop_wait);
            }
            return 0;
        }

        // The components changed under this slot: take the first own slot
        // this faction marked obsolete that carries nothing live, else the
        // last inactive one, and ask before its type is overwritten.
        int own = factionID_ * 0x40;
        unsigned char *flags_walk =
            reinterpret_cast<unsigned char *>(&VehPrototypes[own].flags);
        int mask = 1 << (factionID_ & 0x1F);
        int checked = 0;
        do {
            if ((flags_walk[-4] & mask) != 0
                && PlayersData[factionID_].proto_id_active[own] == 0
                && PlayersData[factionID_].proto_id_queue[own] == 0) {
                protoID_ = own;
                break;
            }
            if ((*flags_walk & PROTO_ACTIVE) == 0) {
                protoID_ = own;
            }
            ++checked;
            flags_walk += 0x34;
            ++own;
        } while (checked < 0x40);

        if (protoID_ < 0x40) {
            parse_says(0, VehPrototypes[protoID_].veh_name, -1, -1);
            if ((*PlayerFlagBlocks & 0x80) == 0) {
                popp(ScriptTxtID, EditorTypePopupLabel, 0,
                     EditorTypePopupSprite, pop_wait);
            } else {
                popp(ScriptTxtID, EditorTypePopupAlienLabel, 0,
                     EditorTypePopupAlienSprite, pop_wait);
            }
            return 0;
        }
    }

    // ---- the settled slot holds a DIFFERENT design: search for the best
    //      replacement and, if needed, confirm overwriting it --------------
    if ((VehPrototypes[protoID_].flags & PROTO_ACTIVE) != 0
        && (protoChassisType_ != VehPrototypes[protoID_].chassis_id
            || protoWeaponType_ != VehPrototypes[protoID_].weapon_id
            || protoArmorType_ != VehPrototypes[protoID_].armor_id
            || protoAbilityFlags_ != VehPrototypes[protoID_].ability_flags
            || protoReactorType_ != VehPrototypes[protoID_].reactor_id)) {
        int search = factionID_ * 0x40;
        char *category_walk =
            reinterpret_cast<char *>(&VehPrototypes[search].unk_1);
        char current_category =
            VehPrototypes[protoID_].unk_1;
        int replaced = 0;
        int looked = 0;
        do {
            if ((category_walk[5] & PROTO_ACTIVE) == 0) {
                protoID_ = search;
                goto settled;
            }
            if ((category_walk[1] & (1 << (factionID_ & 0x1F))) != 0
                && (*category_walk == current_category
                    || PlayersData[factionID_].proto_id_active[search] == 0)
                && PlayersData[factionID_].proto_id_active[search] == 0) {
                if (PlayersData[factionID_].proto_id_queue[search] == 0) {
                    protoID_ = search;
                    replaced = 1;
                } else if (replaced == 0 && *category_walk == current_category) {
                    protoID_ = search;
                }
            }
            category_walk += 0x34;
            ++looked;
            ++search;
        } while (looked < 0x40);

        if (replaced == 0) {
            if (PlayersData[factionID_].proto_id_active[protoID_] != 0
                && (VehPrototypes[protoID_].flags & PROTO_ACTIVE) != 0) {
                parse_says(0, VehPrototypes[protoID_].veh_name, -1, -1);
                parse_num(0, PlayersData[factionID_].proto_id_active[protoID_]);
                char *label;
                char *sprite;
                if ((*PlayerFlagBlocks & 0x80) == 0) {
                    label = EditorOverwritePopupLabel;
                    sprite = EditorOverwritePopupSprite;
                } else {
                    label = EditorOverwritePopupAlienLabel;
                    sprite = EditorOverwritePopupAlienSprite;
                }
                if (popp(ScriptTxtID, label, 0, sprite, pop_wait) == 0) {
                    return 0;
                }
                if (!IsMultiplayerNet) {
                    reinterpret_cast<retire_proto_seam>(static_cast<unsigned long>(
                        0x0057F1D0))(protoID_, factionID_);
                } else {
                    typedef int(__cdecl *diplo_lock_seam)(int);
                    if (reinterpret_cast<diplo_lock_seam>(
                            static_cast<unsigned long>(0x00539820))(0x32)
                        != 0) {
                        return 0;
                    }
                    message_data(0x2415, 0, protoID_, factionID_, 0, 0);
                    NetDaemonNet.await_exec(0);
                    reinterpret_cast<diplo_unlock_seam>(
                        static_cast<unsigned long>(0x005398C0))();
                }
            }
        }
    }

settled:
    make_proto(protoID_, protoChassisType_, protoWeaponType_, protoArmorType_,
               protoAbilityFlags_, protoReactorType_);
    VehPrototypes[protoID_].flags &= 0x3F;
    if (strlen(name) == 0) {
        // No name typed: generate one straight into the name buffer and trim
        // it to the field's 31 characters.
        reinterpret_cast<name_proto_seam>(static_cast<unsigned long>(0x0057E040))(
            name, protoID_, factionID_, protoChassisType_, protoWeaponType_,
            protoArmorType_, protoAbilityFlags_, protoReactorType_);
        name[0x1F] = '\0';
        VehPrototypes[protoID_].flags &= 0xFD;
    } else {
        char generated[256];
        reinterpret_cast<name_proto_seam>(static_cast<unsigned long>(0x0057E040))(
            generated, protoID_, factionID_, protoChassisType_, protoWeaponType_,
            protoArmorType_, protoAbilityFlags_, protoReactorType_);
        if (strcmp(generated, name) != 0) {
            VehPrototypes[protoID_].flags |= PROTO_CUSTOM_NAME_SET;
        }
    }
    strcpy(VehPrototypes[protoID_].veh_name, name);
    synch_proto(protoID_);
    reinterpret_cast<prune_protos_seam>(static_cast<unsigned long>(0x0057F4B0))(
        factionID_, protoID_, 1);
    return 1;
}



// ORIGINAL: 0x0043DD50 ?draw_flash@DesignWin@@QAEXXZ 0x0043DD50-0x0043E486;0x006546EC-0x00654709 FILE
// size      1875 bytes
// prototype void (__thiscall ?draw_flash@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   19
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A63D0 0x005B9F20 0x005D8000 0x005E2DD3 0x005E3820 0x005DAC70 0x005DACB0 0x005DD130 0x005DDAB0 0x00618EA0 0x00618EE0 0x00618F40 0x00619280 0x006453E0 0x00645470 0x00645DD0 0x0064FC88
// Homed from src/unrecovered/0043dd50.cpp: publish the editor selection into
// the vehdraw stat block, then draw the preview panel - the framed box, the
// spinning unit, its generated or typed name, and its stat line. Ends by
// driving draw_stack, whose panel sits under this one.
void DesignWin::draw_flash() {
    char *name = reinterpret_cast<char *>(&field_14220_);

    *VehDrawChassis = static_cast<unsigned char>(protoChassisType_);
    *VehDrawWeapon = static_cast<unsigned char>(protoWeaponType_);
    *VehDrawArmor = static_cast<unsigned char>(protoArmorType_);
    *VehDrawReactor = static_cast<unsigned char>(protoReactorType_);
    *VehDrawAbilityFlags = static_cast<int>(protoAbilityFlags_);
    *VehDrawComponentTail = 0xFF;

    Buffer *canvas = reinterpret_cast<Buffer *>(InterfaceBuffer);
    int shown = static_cast<int>(field_CE0_);
    if (shown == 0x3F
        && protoChassisType_ == VehPrototypes[protoID_].chassis_id
        && protoWeaponType_ == VehPrototypes[protoID_].weapon_id
        && protoArmorType_ == VehPrototypes[protoID_].armor_id
        && protoReactorType_ == VehPrototypes[protoID_].reactor_id
        && protoAbilityFlags_ == VehPrototypes[protoID_].ability_flags) {
        shown = static_cast<int>(protoID_);
    }

    canvas->set_clip(reinterpret_cast<RECT *>(&field_B1C_));
    RECT rc = *reinterpret_cast<RECT *>(&field_B1C_);
    canvas->box_sprite(&rc, &InterfaceBoxStyles[3]);

    rc.left += 3;
    rc.right -= 3;
    rc.top += 3;
    rc.bottom -= 3;
    canvas->box_sprite(&rc, &InterfaceBoxStyles[0]);

    rc.left += 3;
    rc.right -= 3;
    rc.top += 3;
    rc.bottom -= 3;

    // The entry under the cursor resolves to a drawable unit only while its
    // prototype is active, with its tech owned for own-faction entries.
    bool drawable = true;
    if (shown != 0x3F) {
        if ((VehPrototypes[shown].flags & PROTO_ACTIVE) == 0) {
            drawable = false;
        } else if (shown >> 6 == 0
            && !has_tech(VehPrototypes[shown].preq_tech, factionID_)) {
            drawable = false;
        }
    }
    if (shown < 0) {
        drawable = false;
    }

    if (drawable) {
        Font title_font;
        Font stat_font;
        int face_size = (WinScreenWidth == 0x320) ? 10 : 12;
        stat_font.init(DefaultFontFace, face_size, 0);
        title_font.init(DefaultFontFace, 12, 1);

        int top = rc.top;
        int left = rc.left;
        int right = (rc.right - left + 3) / 2 + left - 3;
        int centre_x = (right + left) / 2;
        int bottom = rc.bottom;
        int centre_y = (top + bottom) / 2;

        int unit_id = veh_fake(shown, factionID_);
        veh_draw(canvas, unit_id, centre_x - 0x32, centre_y - 0x25, 0, 0x48, 0);

        rc.right = right + (right - left + 3);
        rc.top = top;
        rc.left = left + (right - left + 3);
        rc.bottom = bottom;
        rc.top = top + 3;

        StringTemp[0] = '\0';
        if (shown == 0x3F) {
            if (strlen(name) == 0) {
                // Unhomed name_proto (0x0057E040) - see design_it.
                typedef void(__cdecl *name_proto_seam)(char *, int, int, int,
                                                       int, int, int, int);
                reinterpret_cast<name_proto_seam>(
                    static_cast<unsigned long>(0x0057E040))(
                    StringTemp, 0x3F, factionID_, protoChassisType_,
                    protoWeaponType_, protoArmorType_, protoAbilityFlags_,
                    protoReactorType_);
            } else {
                strcat(StringTemp, name);
            }
        } else {
            int ability_id = *reinterpret_cast<const short *>(
                ProtoAbilityIdBase + shown * 0x34);
            strcat(StringTemp, VehPrototypes[ability_id].veh_name);
        }
        string_upper_slot()(StringTemp);

        canvas->set_text_color(*StackTitleColorMain, *StackTitleColorShadow,
                               1, 1);
        canvas->set_font(&title_font, 0, 0, 0);

        int wrap_height = canvas->wrap_cent(StringTemp, &rc);
        rc.right -= 2;
        rc.top = wrap_height + 2;
        rc.left += 2;
        rc.bottom -= 2;
        rc.bottom -= stat_font.line_advance();
        canvas->set_font(&stat_font, 0, 0, 0);

        canvas->set_text_color(*StackTextColorMain, *StackTextColorShadow, 1, 1);

        int line_top = rc.top;
        int line_left = rc.left;

        StringTemp[0] = '\0';
        strcat(StringTemp, EditorOpenParen);
        int ability_id = *reinterpret_cast<const short *>(
            ProtoAbilityIdBase + shown * 0x34);
        // Unhomed say_stats_3 (0x0057D7D0) - see draw_stack.
        typedef void(__cdecl *say_stats_3_seam)(char *, unsigned int);
        reinterpret_cast<say_stats_3_seam>(static_cast<unsigned long>(0x0057D7D0))(
            StringTemp, static_cast<unsigned int>(ability_id));
        int ability_bit = 0;
        for (int *label = AbilityLabelIds; label < AbilityLabelIdsEnd;
             ++label, ++ability_bit) {
            if (VehPrototypes[ability_id].ability_flags & (1 << ability_bit)) {
                if (strlen(StringTable.get(*label)) != 0) {
                    strcat(StringTemp, EditorColonSpace);
                    strcat(StringTemp, StringTable.get(*label));
                }
            }
        }
        strcat(StringTemp, EditorCloseParen);
        line_top = canvas->wrap_cent(StringTemp, line_left, line_top,
                                     rc.right - rc.left);

        if (shown != 0x3F) {
            char digits[0x20];
            StringTemp[0] = '\0';
            _itoa(PlayersData[factionID_].proto_id_active[shown], digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, SpaceText);
            strcat(StringTemp,
                   StringTable.get((int)reinterpret_cast<LPSTR *>(
                       Labels.strings_ptr)[0x3A0]));
            line_top = canvas->wrap_cent(StringTemp, line_left, line_top,
                                         rc.right - rc.left);

            StringTemp[0] = '\0';
            _itoa(PlayersData[factionID_].proto_id_queue[shown], digits, 10);
            strcat(StringTemp, digits);
            strcat(StringTemp, SpaceText);
            strcat(StringTemp,
                   StringTable.get((int)reinterpret_cast<LPSTR *>(
                       Labels.strings_ptr)[0x3A1]));
            line_top = canvas->wrap_cent(StringTemp, line_left, line_top,
                                         rc.right - rc.left);

            if (VehPrototypes[shown].obsolete_factions
                & (1 << (factionID_ & 0x1F))) {
                StringTemp[0] = '\0';
                strcat(StringTemp, StringTable.get((int)reinterpret_cast<LPSTR *>(
                    Labels.strings_ptr)[0x390]));
                string_upper_slot()(StringTemp);
                canvas->wrap_cent(StringTemp, line_left, line_top,
                                  rc.right - rc.left);
            }
        }
    }

    MainInterfaceGlobal.soft_update(reinterpret_cast<RECT *>(&field_B1C_));
    canvas->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));
    draw_stack();
}

/*
Purpose: The unit-special ability picker both select_special_N thunks tail
         into: gate the popup on the faction's tech state and chassis kind,
         list every special ability the prototype may take (tech-gated,
         condition-gated, not already built, not already picked), exec it,
         store the pick into the prototype's two ability slots, rebuild the
         ability flag word, and repaint the panels.

// ORIGINAL: 0x0043AC70 ?select_special@DesignWin@@QAEXH@Z 0x0043AC70-0x0043BDE9;0x00654410-0x00654416,0x0065441E-0x00654437,0x00654439-0x006544C5 FILE
// size      5166 bytes
// prototype void (__thiscall ?select_special@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   3   call targets   34
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401060 0x00402970 0x004048A0 0x00404900 0x004064D0 0x004066C0 0x00406880 0x00406910 0x00406E60 0x004346A0 0x00435B30 0x00435DE0 0x00436090 0x0043DD50 0x0043EF70 0x0048C0A0 0x00584B50 0x005B9F20 0x005D45B0 0x005D4DD0 0x005E3820 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00612A70 0x006169A0 0x00625E50 0x00645470 0x00645550 0x0064FC88
// TRIED (seed): transcribed the full control flow with the Popup/BasePop/
//         Dialogs layouts; roughly 85-90% of the call sequence by count. The
//         VC6 /GX SEH frame (fs:[0], the unwind-state byte, the funclet table
//         at 0x654410+) is compiler-synthesized plumbing outside the
//         function's own logic; the Dialogs item()-vs-dtor 0x188 adjustment
//         is not modelled; the two 0x40-byte helper objects were raw buffers.
//         The real locals below give the compiler the whole cleanup chain:
//         the two helpers ARE StringLists (0x004066C0 is ??1StringList, see
//         stringstruct.cpp), and the Dialogs/Spot/StringStruct destructors
//         the seed open-coded are the compiler's at every exit.
// TRIED (forwarder): the shared body both byte-exact select_special_N thunks
//         tail into - the definition lived in pending_bodies.cpp until this
//         transcription landed it at the end of this file so no recovered
//         body above shifts.
// NOTES: the four panel repaints after a successful pick (0x00435DE0
//         draw_special_2, 0x00435B30 draw_special_1, 0x00436090
//         draw_vehicle, 0x004346A0 draw_info) are unhomed, so they go
//         through the original's addresses; abil_cond (0x00584B50) is
//         unhomed the same way. The trailing slot-63 virtual is Win's
//         on_update.
Status: Coverage transcription - unclaimed body
*/
void DesignWin::select_special(int which) {
    Popup popup;
    // Popup keeps scroll_ private and no accessor exists; the member is
    // pinned at 0x3230 - exactly sizeof(BasePop) - see popup.h.
    Scroll *const popup_scroll = reinterpret_cast<Scroll *>(
        reinterpret_cast<uint8_t *>(&popup) + 0x3230);

    const int faction = static_cast<int>(factionID_);
    // Per-faction flag block (PlayerFlagBlocks, sizeof(Player) stride): bit
    // 0x80 of the first byte selects the alien popup set.
    const unsigned char faction_first_byte =
        PlayerFlagBlocks[faction * 0x59C];
    if ((faction_first_byte & 0x80) != 0) {
        if (which != 0) {
            const int tech_id = static_cast<int>(Rules.tech_two_spec_abilities);
            if (!has_tech(tech_id, faction)) {
                parse_say(0,
                          reinterpret_cast<int>(Technology[tech_id].name),
                          -1, -1);
                popp(ScriptTxtID, SpecialTwoPopupAlienLabel, 0,
                     SpecialTwoPopupAlienSprite, pop_wait);
                popup.close();
                popup_scroll->close();
                return;
            }
        }
        if (Chassis[protoChassisType_].missile != 0) {
            popp(ScriptTxtID, SpecialMissilePopupAlienLabel, 0,
                 SpecialMissilePopupAlienSprite, pop_wait);
            popup.close();
            popup_scroll->close();
            return;
        }
    } else {
        if (which != 0) {
            const int tech_id = static_cast<int>(Rules.tech_two_spec_abilities);
            if (!has_tech(tech_id, faction)) {
                parse_say(0,
                          reinterpret_cast<int>(Technology[tech_id].name),
                          -1, -1);
                popp(ScriptTxtID, SpecialTwoPopupLabel, 0,
                     SpecialTwoPopupSprite, pop_wait);
                popup.close();
                popup_scroll->close();
                return;
            }
        }
        if (Chassis[protoChassisType_].missile != 0) {
            popp(ScriptTxtID, SpecialMissilePopupLabel, 0,
                 SpecialMissilePopupSprite, pop_wait);
            popup.close();
            popup_scroll->close();
            return;
        }
    }

    popup.start(PopupStartCaption, SpecialUnitsPopupLabel, -1, nullptr,
                0x200042);

    Dialogs dialogs;
    Spot spot;
    StringList helper_list_a;
    StringList helper_list_b;
    StringStruct stat_list_a;
    StringStruct stat_list_b;

    StringTemp[0] = 0;
    strcat(StringTemp,
           StringTable.get((int)reinterpret_cast<LPSTR *>(
               Labels.strings_ptr)[0x88]));
    dialogs.item(StringTemp, 0x1D);

    int items_added = 1;
    // The special-ability records: 39 entries at the 0x1C Ability stride,
    // walked from Ability[0].cost_factor (0x009AB544) to the table end
    // (0x009AB870).
    for (int ability = 0; ability < 39; ++ability) {
        const int16_t preq_tech = Ability[ability].preq_tech;
        if (!has_tech(preq_tech, faction)) {
            continue;
        }
        typedef int(__cdecl *AbilCondSeam)(int, int, int, int);
        if (reinterpret_cast<AbilCondSeam>(
                static_cast<unsigned long>(0x00584B50))(
                ability, static_cast<int>(protoChassisType_),
                static_cast<int>(protoWeaponType_),
                static_cast<int>(protoArmorType_)) == 0) {
            continue;
        }

        // Already built by this faction: one of its 0x29 build slots (the
        // paired kind/id dwords at +8/+0x28 of the flag block) holds this
        // ability.
        int already_built = 0;
        {
            const int *const built_kind = reinterpret_cast<const int *>(
                PlayerFlagBlocks + 8 + faction * 0x59C);
            const int *const built_id = reinterpret_cast<const int *>(
                PlayerFlagBlocks + 0x28 + faction * 0x59C);
            for (int slot = 0; slot < 0x29; ++slot) {
                if (built_kind[slot] == 0xF && built_id[slot] == ability) {
                    already_built = 1;
                }
            }
        }
        if (already_built != 0) {
            continue;
        }
        if (protoAbilityID1_ == static_cast<uint32_t>(ability)
            || protoAbilityID2_ == static_cast<uint32_t>(ability)) {
            continue;
        }

        StringTemp[0] = 0;
        const int kind = static_cast<int>(Ability[ability].cost_factor);
        if (kind >= 0) {
            // Plain abilities quote their one-based pick position; when the
            // chassis's triad byte is clear they quote the zero-based one
            // and open the stat fragment group.
            if ((Ability[ability].flags & 0x400) != 0
                || Chassis[faction].triad != 0) {
                char digits[0x20];
                _itoa(ability + 1, digits, 10);
                strcat(StringTemp, digits);
            } else {
                char digits[0x20];
                _itoa(ability, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
            }
        } else if (kind >= -7) {
            const unsigned char weapon_cost =
                Weapon[protoWeaponType_].cost;
            const unsigned char armor_cost = Armor[protoArmorType_].cost;
            const unsigned char chassis_speed =
                Chassis[protoChassisType_].speed;
            char digits[0x20];
            LPSTR *const labels =
                reinterpret_cast<LPSTR *>(Labels.strings_ptr);
            switch (kind + 7) {
            case 6: {
                int ratio = static_cast<int>(weapon_cost) /
                            static_cast<int>(armor_cost);
                if (ratio < 0) {
                    ratio = 0;
                } else if (ratio > 2) {
                    ratio = 2;
                }
                _itoa(ratio, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                strcat(StringTemp, StringTable.get((int)labels[0x1F0]));
                strcat(StringTemp, SpecialSlashSign);
                strcat(StringTemp, StringTable.get((int)labels[0x154]));
                strcat(StringTemp, EditorCloseParen);
                break;
            }
            case 5:
                _itoa(static_cast<int>(weapon_cost) - 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                strcat(StringTemp, StringTable.get((int)labels[0x1F0]));
                strcat(StringTemp, SpecialMinusOneText);
                strcat(StringTemp, EditorCloseParen);
                break;
            case 4:
                _itoa(static_cast<int>(armor_cost) - 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                strcat(StringTemp, StringTable.get((int)labels[0x154]));
                strcat(StringTemp, SpecialMinusOneText);
                strcat(StringTemp, EditorCloseParen);
                break;
            case 3:
                _itoa(static_cast<int>(chassis_speed) - 1, digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                strcat(StringTemp, StringTable.get((int)labels[0xB14]));
                strcat(StringTemp, SpecialMinusOneText);
                strcat(StringTemp, EditorCloseParen);
                break;
            case 2:
                _itoa(static_cast<int>(armor_cost + weapon_cost) - 2, digits,
                      10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                strcat(StringTemp, StringTable.get((int)labels[0x1F0]));
                strcat(StringTemp, SpecialPlusSignText);
                strcat(StringTemp, StringTable.get((int)labels[0x154]));
                strcat(StringTemp, SpecialMinusTwoText);
                strcat(StringTemp, EditorCloseParen);
                break;
            case 1:
                _itoa(static_cast<int>(chassis_speed + weapon_cost) - 2,
                      digits, 10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                strcat(StringTemp, StringTable.get((int)labels[0x1F0]));
                strcat(StringTemp, SpecialPlusSignText);
                strcat(StringTemp, StringTable.get((int)labels[0xB14]));
                strcat(StringTemp, SpecialMinusTwoText);
                strcat(StringTemp, EditorCloseParen);
                break;
            case 0:
                _itoa(static_cast<int>(chassis_speed + armor_cost) - 2, digits,
                      10);
                strcat(StringTemp, digits);
                strcat(StringTemp, SpaceText);
                strcat(StringTemp, EditorOpenParen);
                strcat(StringTemp, StringTable.get((int)labels[0x154]));
                strcat(StringTemp, SpecialPlusSignText);
                strcat(StringTemp, StringTable.get((int)labels[0xB14]));
                strcat(StringTemp, SpecialMinusTwoText);
                strcat(StringTemp, EditorCloseParen);
                break;
            default:
                break;
            }
        }

        dialogs.item(StringTemp, static_cast<int>(protoChassisType_));
        ++items_added;
    }

    if (items_added == 0) {
        popup.close();
        popup_scroll->close();
        return;
    }

    const int exec_result = popup.exec(0, pop_wait);
    if (exec_result < 0) {
        popup.close();
        popup_scroll->close();
        return;
    }

    int picked = exec_result;
    if (picked >= 0x1D) {
        picked = -1;
    }
    if (which == 0) {
        protoAbilityID1_ = static_cast<uint32_t>(picked);
    } else {
        protoAbilityID2_ = static_cast<uint32_t>(picked);
    }
    protoAbilityFlags_ = 0;
    {
        uint32_t bits = 0;
        for (int slot = 0; slot < 2; ++slot) {
            const int value = static_cast<int>(slot == 0 ? protoAbilityID1_
                                                         : protoAbilityID2_);
            if (value >= 0) {
                bits |= 1u << static_cast<unsigned>(value);
            }
        }
        protoAbilityFlags_ = bits;
    }

    // The four panel repaints are unhomed; they go through the original's
    // addresses like the rest of this file's pending seams.
    typedef void(__fastcall *DesignWinIntSeam)(void *, void *, int);
    reinterpret_cast<DesignWinIntSeam>(
        static_cast<unsigned long>(0x00435DE0))(this, nullptr, 0);
    reinterpret_cast<DesignWinIntSeam>(
        static_cast<unsigned long>(0x00435B30))(this, nullptr, 0);
    reinterpret_cast<DesignWinIntSeam>(
        static_cast<unsigned long>(0x00436090))(this, nullptr, 0);
    draw_flash();
    reinterpret_cast<DesignWinIntSeam>(
        static_cast<unsigned long>(0x004346A0))(this, nullptr, 0);
    on_update();

    stat_list_a.remove_all();
    stat_list_b.remove_all();
    popup.close();
    popup_scroll->close();
}
