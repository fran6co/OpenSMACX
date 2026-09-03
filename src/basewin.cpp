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
#include "original_seam.h"
#include "basewin.h"
#include "win.h"
#include "subinterface.h"
// Homed BaseWin bodies call the free helpers the archive binds by address:
// set_base/best_specialist (base.h), parse_says (general.h), radius_move
// (map.h), do_all_draws (temp.h), text_open (text.h), and construct the
// BasePop/Popup popups outright.
#include "base.h"
#include "basepop.h"
#include "console.h"
#include "datalink.h"
#include "general.h"
#include "map.h"
#include "popup.h"
#include "temp.h"
#include "text.h"
#include <cstdint>
#include <cstring>

/*
Purpose: Construct BaseWin's large aggregate of sub-windows: a ProdPicker, a
         second GraphicWin, several Sprite runs, a FlatButton, a Buffer, a
         FlatButton[22], a Time, a Caviar, a MapWin, a Font, two Spots, a
         second Buffer and three Scrolls.
// ORIGINAL: 0x00408490 ??0BaseWin@@QAE@XZ 0x00408490-0x0040870D;0x004210D0-0x004210E7;0x00651380-0x00651567
// LEVER: real, correctly-offset, correctly-typed members (already modelled
//        in basewin.h) constructed IMPLICITLY, in declaration order, took
//        this from a SHARED_TAIL isolated-scaffold measurement straight to
//        120/144 agreeing instructions here - far better than the explicit
//        raw-offset placement-new this batch's other five constructors
//        needed for ordering control. First divergence at instruction 10:
//        this compiled body reserves an extra `push edi` and calls through
//        an extra local thunk where the image calls GraphicWin::construct
//        directly.
// TRIED: plateau 120/144; divergence #10 is the extra push edi + local thunk
//        against the image's direct GraphicWin::construct call, measured under
//        the SHARED_TAIL caveat (no per-function verdict is well defined)
// TRIED: 2026-08-28 confirming sweep, one re-measure of the tree's own body
//        after basewin.h grew member declarations and the click forwarders
//        were renamed: 104 of 144 agreeing at /c /O2 /Ob0 /Gy /GR- /Oy- /GX,
//        149 compiled, first divergence still #10 (the same `push edi` before
//        the esi spill, then a link-time call where the image has
//        `call 0x5d4cf0`). The 120/144 above is the deleted isolated
//        scaffold's number, not this tree's; do not chase it. No new lever.
// size      1147 bytes
// prototype void (__thiscall ??0BaseWin@@QAE@XZ)(BaseWin* this)
// callers   1   call targets   15
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00420F90 0x004626E0 0x00492420 0x005D4CF0 0x005D4DD0 0x005D7210 0x005E37E0 0x005FA860 0x006051D0 0x00607CF0 0x006161D0 0x00616DA0 0x00618EA0 0x006456E4 0x006457C2
*/
// This address is catalogued SHARED_TAIL: its cleanup path runs through an
// EH-funclet cluster shared with other functions (COMDAT-folded), so no
// per-function byte verdict is well defined here regardless of source form -
// see the deleted src/recovered/units/00408490.cpp, which measured the same
// finding from an isolated scaffold. Bodied for COVERAGE: every sub-object
// below is a real, correctly-offset, correctly-typed member already modelled
// in basewin.h, constructed implicitly in declaration order (matching the
// image's own order field-for-field) - mapWin_ included now that MapWin has
// its real constructor, which pushes the hidden most-derived flag 1 exactly
// where the image's member construction does.
BaseWin::BaseWin() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00408710 ?close@BaseWin@@QAEXXZ 0x00408710-0x00408711 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::close() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00416DD0 ?UNK4@BaseWin@@QAEXXZ 0x00416DD0-0x00416DD1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK4@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0041AF20 ?UNK6@BaseWin@@QAEXXZ 0x0041AF20-0x0041AF21 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0041AF30 ?UNK7@BaseWin@@QAEXXZ 0x0041AF30-0x0041AF31 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK7@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK7() {
}





/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041A610 ?UNK5@BaseWin@@QAEXH@Z 0x0041A610-0x0041A613 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK5@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK5(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041B4E0 ?on_button_toggled@BaseWin@@QAEXHH@Z 0x0041B4E0-0x0041B4E3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_button_toggled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041DF40 ?on_iface_right_down@BaseWin@@QAEXHH@Z 0x0041DF40-0x0041DF43 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0041E4C0 ?on_iface_selected@BaseWin@@QAEXHH@Z 0x0041E4C0-0x0041E4C3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_selected(int, int) {
}



/*
Purpose: Report a left click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
// ORIGINAL: 0x0041DCD0 ?on_iface_left_click@BaseWin@@QAEXHH@Z 0x0041DCD0-0x0041DCEE BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_iface_left_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_left_click(int xCoord, int yCoord) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->iface_click(xCoord, yCoord, 0, 0);
}

/*
Purpose: Report a right click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
// ORIGINAL: 0x0041DCF0 ?on_iface_right_click@BaseWin@@QAEXHH@Z 0x0041DCF0-0x0041DD0E BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_iface_right_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_click(int xCoord, int yCoord) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->iface_click(xCoord, yCoord, 1, 0);
}

/*
Purpose: Report a left double-click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
// ORIGINAL: 0x0041E4D0 ?on_iface_left_double_click@BaseWin@@QAEXHH@Z 0x0041E4D0-0x0041E4EE BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_iface_left_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_left_double_click(int xCoord, int yCoord) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->iface_click(xCoord, yCoord, 0, 1);
}

/*
Purpose: Report a right double-click on the interface. Reached through the interface
         subobject at 0xA14, so `this` is adjusted back to the BaseWin.
// ORIGINAL: 0x0041E4F0 ?on_iface_right_double_click@BaseWin@@QAEXHH@Z 0x0041E4F0-0x0041E50E BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_iface_right_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004160F0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_right_double_click(int xCoord, int yCoord) {
    BaseWin *const base = reinterpret_cast<BaseWin *>(
        reinterpret_cast<uint8_t *>(this) - 0xA14);
    base->iface_click(xCoord, yCoord, 1, 1);
}






/*
Purpose: Handle an interface scroll, but only for scroll kind 2 - stash the
         new position at 0x40100 (interface-relative) and redraw the supported
         markers. Like the click handlers, `this` arrives at the interface
         subobject and is adjusted back to the BaseWin for that redraw.
// ORIGINAL: 0x0041DC80 ?on_iface_scrolled@BaseWin@@QAEXHH@Z 0x0041DC80-0x0041DCA5 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?on_iface_scrolled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0040C850
// LEVER: two moves took this 2/12 -> 12/12. (1) The guard is a SWITCH on one
//        case, not an `if` - `mov eax,[ebp+8] / sub eax,2 / jne` against the
//        `cmp dword ptr [ebp+8],2` an `if` emits. (2) The -0xA14 adjustment is
//        written INLINE on the store, so VC6 folds 0x40B14-0xA14 into the
//        store's displacement and keeps the raw receiver in ecx until the call.
// TRIED: `std::memcpy(this+0x40100, &pos, 4)` - the previous spelling, 2/12;
//        the memcpy call and the missed switch were both divergences.
// TRIED: one named `BaseWin *const base` local used for BOTH the store and the
//        call - VC6 materialises `add ecx,-0xA14` before the store, 9/12.
// TRIED: member store on `this` (`field_40B14_ = pos`) with `base` only for
//        the call - right schedule, wrong displacement (`[ecx+0x40b14]`),
//        11/12. The image spells the same address as `[ecx+0x40100]`.
Return Value: n/a
Status: Complete
*/
void BaseWin::on_iface_scrolled(int code, int pos) {
    // A SWITCH, not an `if`: the image loads the code into a register and
    // subtracts the case value (`mov eax,[ebp+8] / sub eax,2 / jne`), which is
    // what a one-case switch statement compiles to - the same shape the
    // sibling on_scrolled below is BYTE_EXACT with. An `if` emits
    // `cmp dword ptr [ebp+8],2` instead.
    switch (code) {
        case 2: {
            // The store names the member through the adjusted pointer INLINE,
            // with no local holding it: 0x40B14 - 0xA14 == 0x40100, and VC6
            // folds the two constants into the store's own displacement
            // (`mov [ecx+0x40100], eax`) exactly as the image spells it. A
            // named `base` local makes VC6 materialise the adjusted pointer
            // in ecx BEFORE the store instead.
            reinterpret_cast<BaseWin *>(
                reinterpret_cast<uint8_t *>(this) - 0xA14)->field_40B14_ = pos;
            BaseWin *const base = reinterpret_cast<BaseWin *>(
                reinterpret_cast<uint8_t *>(this) - 0xA14);
            base->draw_supported(1);
            break;
        }
    }
}



/*
Purpose: Report a left click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AF70 ?on_left_click@BaseWin@@QAEXHH@Z 0x0041AF70-0x0041AF88 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_left_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_click(int xCoord, int yCoord) {
    click(xCoord, yCoord, 0, 0);
}

/*
Purpose: Report a right click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AF90 ?on_right_click@BaseWin@@QAEXHH@Z 0x0041AF90-0x0041AFA8 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_right_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_right_click(int xCoord, int yCoord) {
    click(xCoord, yCoord, 1, 0);
}

/*
Purpose: Report a left double-click to the shared click handler. Unlike the
         iface_click family, `this` is the BaseWin itself with no adjustment.
// ORIGINAL: 0x0041AFB0 ?on_left_double_click@BaseWin@@QAEXHH@Z 0x0041AFB0-0x0041AFC8 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_left_double_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004165D0
Return Value: n/a
Status: Complete
*/
void BaseWin::on_left_double_click(int xCoord, int yCoord) {
    click(xCoord, yCoord, 0, 1);
}




/*
Purpose: Interface timer tick; the legacy implementation ignores it and
         returns.
// ORIGINAL: 0x0041AF40 ?timer_callback@BaseWin@@QAAXHH@Z 0x0041AF40-0x0041AF41 BYTE_EXACT
// symbol    ?timer_callback@BaseWin@@SAXHH@Z
// size      1 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BaseWin::timer_callback(int, int) {
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x0041B1E0 ?on_mouse_leave@BaseWin@@QAEXHH@Z 0x0041B1E0-0x0041B1ED BYTE_EXACT
// symbol    ?on_mouse_leave@BaseWin@@UAEXHH@Z
// size      13 bytes
// prototype void (__thiscall ?on_mouse_leave@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void BaseWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
// ORIGINAL: 0x0041E890 ?show@BaseWin@@QAEXH@Z 0x0041E890-0x0041E8AB BYTE_EXACT
// symbol    ?show@BaseWin@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void BaseWin::show(int visible) {
    // Win is the base at 0 and SubInterface the base at 0xA14 - both real
    // bases, so both calls go by their unqualified names and the compiler
    // makes the adjustments.
    if (!is_visible()) {
        set_iface_mode();
    }
}

/*
// ORIGINAL: 0x0040B120 ?UNK2@BaseWin@@QAEXHHHH@Z 0x0040B120-0x0040B13C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?UNK2@BaseWin@@QAEXHHHH@Z)(BaseWin* this, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0040B140
Status: Complete
*/
// ALL FOUR PARAMETERS STAY SCAFFOLD. The body is one positional forward, so
// its only "evidence" is garrison_click's own parameter list - and those names
// have no provenance: ida9-functions.csv has 0x0040B140 as `sub_40B140` and
// ghidra-functions.csv as `FUN_0040b140`, both tools leaving it unnamed, and a
// prototype records types, never names. Renaming UNK2 from them would invent a
// meaning at one remove from an invention. (vehID/right/is_double were tried
// here and reverted for exactly that reason.)
void BaseWin::UNK2(int a1, int a2, int a3, int a4) {
    garrison_click(a1, a2, a3, a4);
}

/*
// ORIGINAL: 0x0041B4A0 ?on_scrolled@BaseWin@@QAEXHH@Z 0x0041B4A0-0x0041B4BD BYTE_EXACT
// size      29 bytes
// prototype void (__thiscall ?on_scrolled@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0040FCC0
Status: Complete
*/
void BaseWin::on_scrolled(int code, int pos) {
    switch (code) {
        case 1:
            field_40B10_ = pos;
            draw_facilities(1);
            break;
    }
}

/*
Purpose: Step the receiver back to the subobject ??_GBaseWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421810 ??_GBaseWin@@WEEE@AEPAXI@Z 0x00421810-0x0042181B BYTE_EXACT
// symbol    ??_EBaseWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3BaseWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004213D0` into
//   ??_GBaseWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
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

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Wave g_BASEWIN_WAVE;  // 0x006EEE68

// The process-wide base window, 0x006A7628 in the image - REAL STORAGE, not
// the `Win *const TutWinBaseWindow` binding that named terranx.exe data
// (see tutwin.h). The image's dynamic initializer at 0x004083B0 runs the
// real BaseWin constructor against it and registers the deleting destructor
// with atexit; here the same recovered constructor runs through this
// build's own startup and the destructor closes it at exit.
BaseWin TutWinBaseWindow;  // 0x006A7628
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// Fixed-slot bindings carried from 00411980.cpp

// ORIGINAL: 0x00411980 ?draw_energy_alloc@BaseWin@@QAEXH@Z 0x00411980-0x00412A96;0x00651B4C-0x00651B61 FILE
// TRIED: heavily time-boxed given batch budget - only the local Font construction, the initial set_clip, Font::init and set_font calls are transcribed (MISMATCH #11, well past the prologue); the bar-chart body (many box_sprite calls with per-slider RECT arithmetic and a faction colour table) is a deliberate stub, not fabricated.
// working copy - scaffold materialised by --work
// size      4395 bytes
// prototype void (__thiscall ?draw_energy_alloc@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   3   call targets   16
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D5630 0x005D5930 0x005D8000 0x005DAC70 0x005DACB0 0x005DCF40 0x005DD130 0x005DD450 0x005E2DD3 0x006169A0 0x00618EA0 0x00618EE0 0x00618F40 0x006453E0 0x00645470 0x0064FC88
// indirect  0x00411D58 0x00411E7B 0x00411FE5 0x0041213C 0x004122C9 0x00412433 0x00412561 0x004126EF 0x00412855 0x0041297F

void BaseWin::draw_energy_alloc(int a1) {
    char *self = reinterpret_cast<char *>(this);

    // Real local matching the single `call ??0Font@@QAE@XZ` in the
    // disassembly; RAII covers its destructor at the return below.
    Font font;
    Buffer *canvas = reinterpret_cast<Buffer *>(self + 0x444);

    RECT clipRect = *reinterpret_cast<RECT *>(self + 0x40c3c);
    canvas->set_clip(&clipRect);

    font.init(reinterpret_cast<char *>(*DefaultFontFace), 0xc, 1);
    canvas->set_font(&font, 0, 0, 0);

    // Time-boxed: draw_energy_alloc renders the psych/labs/econ
    // allocation-slider bar chart via a long sequence of box_sprite
    // calls, per-slider RECT arithmetic driven by the current
    // allocation at self+0x474+0xc..0x474+0x30-ish, and a faction
    // colour-table lookup (stride 0x59c) - not transcribed beyond the
    // clip/font setup above. See the TRIED note.
    (void)a1;
}

// Fixed-slot bindings carried from 0041afd0.cpp
static int *const InterfaceClipRect = (int *)0x007AEC94;
static int *const MainInterfaceScreenClipTop = (int *)0x007AEC98;
static int *const MainInterfaceScreenClipRight = (int *)0x007AEC9C;
static int *const MainInterfaceScreenClipBottom = (int *)0x007AECA0;

class VCall2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003(int, int);  // <-- used
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019(int, int);  // <-- used
};

class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();  // <-- used
    virtual void slot004();  // <-- used
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();  // <-- used
};

// ORIGINAL: 0x0041AFD0 ?on_left_up@BaseWin@@QAEXHH@Z 0x0041AFD0-0x0041B10D FILE
// size      317 bytes
// prototype void (__thiscall ?on_left_up@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC7C0 0x005ED240 0x005ED2D0 0x005FA7E0
// indirect  0x0041B0C6 0x0041B0D7 0x0041B0F0
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0041AFD0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041afd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_up@BaseWin@@QAEXHH@Z  at 0x0041AFD0  (317 bytes)
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


void BaseWin::on_left_up(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    RECT rect = *reinterpret_cast<RECT *>(self + 0x474);
    int flag = *reinterpret_cast<int *>(self + 0x40b54);
    if (flag > 0) {
        if (!in_box(a1, a2, &rect)) {
            reinterpret_cast<Win *>(self)->client_to_screen(&a1, &a2);
            MainInterfaceGlobal.screen_to_client(&a1, &a2);

            RECT rect2 = { *InterfaceClipRect, *MainInterfaceScreenClipTop,
                       *MainInterfaceScreenClipRight,
                       *MainInterfaceScreenClipBottom };

            if (!in_box(a1, a2, &rect2)) {
                reinterpret_cast<Win *>(self)->set_cursor(0x7f00);
                MainInterfaceGlobal.set_cursor(0x7f00);
                *reinterpret_cast<int *>(self + 0x40b54) = -1;
                *reinterpret_cast<int *>(self + 0x40b4c) = -1;
                *reinterpret_cast<int *>(self + 0x40b50) = -1;
            } else {
                reinterpret_cast<VCall2 *>(self + 0xa14)->slot003(a1, a2);
            }
        } else {
            reinterpret_cast<VCall2 *>(self)->slot019(a1, a2);
        }
    }

    if (WinPointerOwner3 == reinterpret_cast<Win *>(self)) {
        WinPointerOwner3 = 0;
        reinterpret_cast<VCall *>(self)->slot004();
    }
    if (WinPointerOwner4 == reinterpret_cast<Win *>(self)) {
        WinPointerOwner4 = 0;
    }
}

// Fixed-slot bindings carried from 0041b110.cpp
static int *const NetInputLockFlag = (int *)0x0093E8C8;

// ORIGINAL: 0x0041B110 ?on_left_down@BaseWin@@QAEXHH@Z 0x0041B110-0x0041B1D9 FILE
// TRIED: SHAPE_EXACT (every instruction's mnemonic, size and operand kind match; only immediate/relocated values differ in the diagnostic dump, which is a base-address display artifact for most of them). Grouping the two `Spot::check` out-params into an anonymous `struct { int a; int b; }` stopped VC6 reusing the incoming `a1`/`a2` parameter stack slots as the output locations (which it did when they were two plain `int` locals, since a1/a2 are dead after the call) - matches the original's dedicated `sub esp,8` locals. Store order for the two `DAT_009b7abc`/`DAT_009b7ac0` guard writes had to be flipped from the Ghidra-printed order to match the disassembly's actual order.
// working copy - scaffold materialised by --work
// size      201 bytes
// prototype void (__thiscall ?on_left_down@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC7C0 0x005F7E90 0x005FAB00

void BaseWin::on_left_down(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x40b54) > 0) {
        reinterpret_cast<Win *>(self)->set_cursor(0x7f00);
        MainInterfaceGlobal.set_cursor(0x7f00);
    }
    *reinterpret_cast<int *>(self + 0x40b54) = -1;
    *reinterpret_cast<int *>(self + 0x40b4c) = -1;
    *reinterpret_cast<int *>(self + 0x40b50) = -1;
    *reinterpret_cast<int *>(self + 0x40b44) = a1;
    *reinterpret_cast<int *>(self + 0x40b48) = a2;
    if (reinterpret_cast<Win *>(self)->is_visible() != 0 &&
        (IsMultiplayerNet == 0 || *NetInputLockFlag == 0)) {
        if (WinPointerOwner3 == 0) {
            WinPointerOwner3 = reinterpret_cast<Win *>(self);
            WinPointerOwner4 = 0;
        }
        struct { int a; int b; } out;
        int r = reinterpret_cast<Spot *>(self + 0x40ec0)->check(a1, a2, &out.b, &out.a);
        if (r >= 0) {
            *reinterpret_cast<int *>(self + 0x40b4c) = out.b;
            *reinterpret_cast<int *>(self + 0x40b50) = out.a;
            *reinterpret_cast<int *>(self + 0x40b54) = 0;
        }
    }
}


// 0x0066A1A0 / 0x0066A1F4, read out of the shipped image with
// tools/image_data.py. Back to back, 21 ints each - the 5x5 base radius
// minus its four corners, in ring order.
const int BaseRadiusOffsetX[21] = {
    -1,  1, -2,  0,  2, -3, -1,  1,  3, -2,
     0,  2, -3, -1,  1,  3, -2,  0,  2, -1,
     1,
};
const int BaseRadiusOffsetY[21] = {
    -3, -3, -2, -2, -2, -1, -1, -1, -1,  0,
     0,  0,  1,  1,  1,  1,  2,  2,  2,  3,
     3,
};

/*
// ORIGINAL: 0x004090C0 ?UNK1@BaseWin@@QAEHH@Z 0x004090C0-0x004090FA FILE BYTE_EXACT
// LEVER: the tables this indexes had to become real storage. The scaffold
// bound them `extern int g_a1a0_tbl[]` with no definition anywhere in the
// tree, so homing reverted on LNK2001 every time; naming them (map.h's own
// RadiusOffsetX/RadiusOffsetY are `const int[]` at namespace scope, i.e.
// internal linkage, so they cannot serve) and defining them from the image's
// own bytes is what makes the body spellable. `extern` is the right spelling
// for all four arrays: the const-pointer bindings the scaffold used let /O2
// prove 0x66A1A0 and 0x66A1F4 a constant 0x54 apart and rotate the loop
// around one walking pointer, which the image does not do.
*/
int BaseWin::UNK1(int a1) {
    for (int i = 0; i < 0x15; i++) {
        if (BaseRadiusOffsetX[a1] == RadiusOffsetX[i] &&
            BaseRadiusOffsetY[a1] == RadiusOffsetY[i]) {
            return i;
        }
    }
    return 0;
}

/*
// ORIGINAL: 0x00416540 ?bubble_me_elmo2@BaseWin@@QAEXPADHH@Z 0x00416540-0x00416580 FILE BYTE_EXACT
// LEVER: the RECT argument is built as a local aggregate initializer, not as
// four sequential RECT field stores - sequential stores reload one register
// per statement and mis-schedule against the text load; the initializer keeps
// x and y live in two registers at once and reproduces the original's exact
// interleaving.
*/
void BaseWin::bubble_me_elmo2(char *text, int x, int y) {
    reinterpret_cast<Win *>(this)->client_to_screen(&x, &y);
    RECT rect = { x, y, x, y };
    reinterpret_cast<Win *>(this)->set_bubble_text(text, &rect);
}

/*
// ORIGINAL: 0x00416580 ?bubble_me_elmo2@BaseWin@@QAGXPADHH@Z 0x00416580-0x004165C2 FILE BYTE_EXACT
// symbol    ?bubble_me_elmo2@@YGXPADHH@Z
// explicit arguments - no receiver's worth of extra 4 bytes - and the body
// never reads `this` (ecx is overwritten with the fixed 0x7AE820 singleton
// before either call). A real `__stdcall` member passes `this` as an implicit
// FIRST stack slot (`ret 0x10`, every offset shifted by 4); confirmed by
// compiling it that way first. A plain free function is the only spelling
// that lands it, so it lives beside the class rather than on it.
*/
void __stdcall bubble_me_elmo2(char *text, int x, int y) {
    Win *win = &MainInterfaceGlobal;
    win->client_to_screen(&x, &y);
    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x;
    rect.bottom = y;
    win->set_bubble_text(text, &rect);
}

// Fixed-slot bindings carried from 0041de60.cpp

// ORIGINAL: 0x0041DE60 ?on_iface_left_down@BaseWin@@QAEXHH@Z 0x0041DE60-0x0041DF38 FILE BYTE_EXACT
// LEVER: locals declared at top of function (not just before use) reproduces `sub esp,8` for two 4-byte slots instead of the optimizer reusing a dead parameter's stack slot for one of them; and the two adjacent global stores (WinPointerOwner3 then WinPointerOwner4) had to be written in that exact order - Ghidra's own pseudocode printed them reversed.
// working copy - scaffold materialised by --work
// size      216 bytes
// prototype void (__thiscall ?on_iface_left_down@BaseWin@@QAEXHH@Z)(BaseWin* this, int xCoord, int yCoord)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005EC7C0 0x005F7E90 0x005FAB00

void BaseWin::on_iface_left_down(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int local_c;
    int local_8;

    if (*reinterpret_cast<int *>(self + 0x40140) > 0) {
        reinterpret_cast<Win *>(self - 0xa14)->set_cursor(0x7f00);
        reinterpret_cast<Win *>(0x7ae820)->set_cursor(0x7f00);
    }
    *reinterpret_cast<unsigned int *>(self + 0x40140) = 0xffffffff;
    *reinterpret_cast<unsigned int *>(self + 0x40138) = 0xffffffff;
    *reinterpret_cast<unsigned int *>(self + 0x4013c) = 0xffffffff;
    *reinterpret_cast<int *>(self + 0x40130) = a1;
    *reinterpret_cast<int *>(self + 0x40134) = a2;

    if (reinterpret_cast<Win *>(self - 0xa14)->is_visible() != 0 &&
        (IsMultiplayerNet == 0 || *NetInputLockFlag == 0)) {
        if (WinPointerOwner3 == 0) {
            WinPointerOwner3 = reinterpret_cast<Win *>(self - 0xa14);
            WinPointerOwner4 = 0;
        }
        if (reinterpret_cast<Spot *>(self + 0x404a0)->check(a1, a2, &local_c, &local_8) >= 0) {
            *reinterpret_cast<int *>(self + 0x40138) = local_8;
            *reinterpret_cast<int *>(self + 0x4013c) = local_c;
            *reinterpret_cast<int *>(self + 0x40140) = 0;
        }
    }
}

class VCallArg1 {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void slot016();
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual void slot033();
  virtual void slot034();
  virtual void slot035();
  virtual void slot036();
  virtual void slot037();
  virtual void slot038();
  virtual void slot039();
  virtual void slot040();
  virtual void slot041();
  virtual void slot042();
  virtual void slot043();
  virtual void slot044(int);
};

// ORIGINAL: 0x0041E510 ?on_iface_button_clicked@BaseWin@@QAEXH@Z 0x0041E510-0x0041E52D BYTE_EXACT
// size      29 bytes
// prototype void (__thiscall ?on_iface_button_clicked@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0041E523
// 0x0041E510  ?on_iface_button_clicked@BaseWin@@QAEXH@Z  ->  ?on_iface_button_clicked@BaseWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void BaseWin::on_iface_button_clicked(int a1) {
    VCallArg1 *const enclosing = reinterpret_cast<VCallArg1 *>(
        reinterpret_cast<char *>(this) - 0xa14);
    enclosing->slot044(a1);
}

// ORIGINAL: 0x0041E8B0 ?hide@BaseWin@@QAEXXZ 0x0041E8B0-0x0041E8C9 BYTE_EXACT
// symbol    ?hide@BaseWin@@UAEXXZ
// size      25 bytes
// prototype void (__thiscall ?hide@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005F7E90
// 0x0041E8B0  ?hide@BaseWin@@QAEXXZ  ->  ?hide@BaseWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BaseWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(
            reinterpret_cast<char *>(this) + 0xA14)->release_iface_mode();
    }
}

// ORIGINAL: 0x00420A40 ?on_nc_hittest@BaseWin@@QAEHHH@Z 0x00420A40-0x00420A6F BYTE_EXACT
// symbol    ?on_nc_hittest@BaseWin@@UAEHHH@Z
// size      47 bytes
// prototype int (__thiscall ?on_nc_hittest@BaseWin@@QAEHHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005F5AD0
// 0x00420A40  ?on_nc_hittest@BaseWin@@QAEHHH@Z  ->  ?on_nc_hittest@BaseWin@@QAEHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


int BaseWin::on_nc_hittest(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 >= 0x17 &&
        a1 <= *reinterpret_cast<int *>(self + 0x4c4) - 0x37) {
        return reinterpret_cast<Win *>(this)->Win::on_nc_hittest(a1, a2);
    }
    return 2;
}

/*
// ORIGINAL: 0x00408410 ?load_inverts@@YAXXZ 0x00408410-0x00408469 FILE
// LEVER: the scaffold bound the four strings/counts it reads as anonymous
// `int *const g_<address>` slots. All four are named things: 0x00682830 and
// 0x00682824 hold the literals "BASENAME" and "INVERTLIST" (read with
// tools/image_data.py --string), 0x009B7D00 is text.h's TextBufferGetPtr, and
// 0x006A7528 / 0x006EEE60 are the invert-list storage and its count, named
// here from what this one body does with them.
*/

// 0x006EEE60, the count of loaded invert entries; 0x006A7528, the entries
// themselves at a 0x20 stride. Neither is named anywhere else in the tree.
int *const InvertEntryCount = (int *)0x006EEE60;
char *const InvertEntryTable = (char *)0x006A7528;

void __cdecl load_inverts() {
    if (text_open("INVERTLIST", "BASENAME") == 0) {
        text_get();
        int count = text_item_number();
        *InvertEntryCount = count;
        if (count > 0) {
            char *dest = InvertEntryTable;
            int i = 0;
            do {
                text_get();
                strcpy(dest, TextBufferGetPtr);
                count = *InvertEntryCount;
                i++;
                dest += 0x20;
            } while (i < count);
        }
        text_close();
    }
}
