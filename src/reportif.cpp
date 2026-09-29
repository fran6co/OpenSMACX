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
#include "reportif.h"
#include "base.h"    // BaseCurrentCount bounds the report's fixed sort workspace
#include "general.h" // swap for the original reversed-bound normalization
#include "reportwin.h"  // ReportWinGlobal - the window done() un-modals
#include "worldwin.h"
#include "menu.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "flatbutton.h"
#include "sprite.h"
#include "buttongroup.h"
#include "spot.h"
#include "time.h"  // SatPanelTimer1/2 - close_sat stops both
#include "maininterface.h"  // MainInterfaceGlobal - the set_modal owner
#include "alpha.h"  // Labels
#include "faction.h"  // Players
#include "game.h"  // say_year
#include "strings.h"  // StringTable, StringTemp
#include "vector_teardown.h"



/*
Purpose: Install the SubInterface vftable, then placement-new every
         sub-object in image order - seven FlatButton runs, three Sprite
         runs, two ListBox(1)s, four ButtonGroups and fourteen individual
         FlatButtons, and a Spot - before installing ReportIf's own vftable.
// ORIGINAL: 0x004AD170 ??0ReportIf@@QAE@XZ 0x004AD170-0x004AD3AA;0x00658F80-0x00659134
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` the image does
//            not. MISMATCH, 34/132 instructions agree - the best of this
//            batch. Layout (offsets, sub-object sizes, construction order)
//            cross-checked directly against the destructor at 0x004ACDA0,
//            which tears the same run down in mirrored order.
// TRIED: real declared members (FlatButton[7]/Sprite[0x15]/FlatButton[7])
//            for flatButtonsA_/spritesA_/flatButtonsB_, built implicitly -
//            measured WORSE, 26/132 (compiled grew to 195 instructions). The
//            `object[0] = vtable` store has to stay explicit body code (its
//            base SubInterface is not parametrised per host class, and that
//            header is out of this batch's scope), and standard C++ always
//            runs implicit member construction before any body statement, so
//            real members push that store after all three instead of ahead
//            of them, unlike the image. Reverted.
// size      1006 bytes
// prototype void (__thiscall ??0ReportIf@@QAE@XZ)(ReportIf* this)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FA860 0x00607CF0 0x00609DB0 0x0062B7C0 0x006456E4 0x006457C2
*/
// ListBox has no one-argument constructor declared - its own recovery
// (0x00609DB0) is out of this batch's scope - so it is called directly at
// its image address, the same treatment pending_bodies.cpp gives every
// not-yet-promoted callee: a `reinterpret_cast` of the address, called
// immediately, compiles the image's own `call rel32` rather than the
// `call dword ptr [...]` a stored pointer would give.
ReportIf::ReportIf() {


    typedef void(__fastcall *pending_listbox_ctor)(void *, void *, int);
    reinterpret_cast<pending_listbox_ctor>(0x00609DB0)(listBox1_, nullptr, 1);
    reinterpret_cast<pending_listbox_ctor>(0x00609DB0)(listBox2_, nullptr, 1);

    new (buttonGroup1_) ButtonGroup();
    new (flatButton1_) FlatButton();
    new (flatButton2_) FlatButton();
    new (buttonGroup2_) ButtonGroup();
    new (flatButton3_) FlatButton();
    new (flatButton4_) FlatButton();
    new (flatButton5_) FlatButton();
    new (flatButton6_) FlatButton();
    new (flatButton7_) FlatButton();
    new (flatButton8_) FlatButton();
    new (flatButton9_) FlatButton();
    new (flatButton10_) FlatButton();
    new (flatButton11_) FlatButton();
    new (buttonGroup3_) ButtonGroup();


    new (buttonGroup4_) ButtonGroup();


    new (flatButton12_) FlatButton();
    new (flatButton13_) FlatButton();
    new (flatButton14_) FlatButton();
    new (spot_) Spot();
}

// The two ListBox sub-objects are torn down through their virtual bases at
// raw addresses, not through a `ListBox` method: ListBox's own virtual-base
// layout (a GraphicWin sub-object at +0x48, a Dialog sub-object at +0xA60,
// per listbox.h) is out of this batch's scope, and the image calls three
// distinct teardowns per ListBox - one at the GraphicWin-vbase-relative
// address (0x00609EC0, ListBox's own destructor, called at that adjusted
// `this`), then Dialog::~Dialog() (0x00608E10) at the Dialog sub-object, then
// GraphicWin's real destructor (0x005D4DD0) back at the first address.
typedef void(__fastcall *pending_listbox_vbase_dtor)(void *, void *);

/*
Purpose: Tear down every sub-object the constructor built, in exactly the
         reverse order.
// ORIGINAL: 0x004ACDA0 ??1ReportIf@@QAE@XZ 0x004ACDA0-0x004AD16C;0x00658D50-0x00658F74
// TRIED: SEH frame - the image has one (flags carry `frame`) but a body
//            built entirely from plain `->method()`/`->~T()` calls at raw
//            offsets never triggers VC6's unwind-protection scaffolding (no
//            local object construction for it to protect), so this tree's
//            compiled body has none. A prior attempt at this same address
//            (src/unrecovered/004acda0.cpp, now deleted - superseded by
//            this body) measured the identical finding in isolation.
//            MISMATCH, 4/235 instructions agree; not chased further given
//            the family-wide cap this batch's brief already names.
// size      1520 bytes
// prototype void (__thiscall ??1ReportIf@@QAE@XZ)(ReportIf* this)
// callers   0   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x005FA870 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x0062B7F0 0x006456E4
*/
ReportIf::~ReportIf() {

    reinterpret_cast<Spot *>(spot_)->~Spot();

    reinterpret_cast<FlatButton *>(flatButton14_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton13_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton12_)->destroy();


    reinterpret_cast<ButtonGroup *>(buttonGroup4_)->close();


    reinterpret_cast<ButtonGroup *>(buttonGroup3_)->close();

    reinterpret_cast<FlatButton *>(flatButton11_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton10_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton9_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton8_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton7_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton6_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton5_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton4_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton3_)->destroy();

    reinterpret_cast<ButtonGroup *>(buttonGroup2_)->close();

    reinterpret_cast<FlatButton *>(flatButton2_)->destroy();
    reinterpret_cast<FlatButton *>(flatButton1_)->destroy();

    reinterpret_cast<ButtonGroup *>(buttonGroup1_)->close();

    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    reinterpret_cast<pending_listbox_vbase_dtor>(0x00609EC0)(self + 0xAE6C, nullptr);
    reinterpret_cast<pending_listbox_vbase_dtor>(0x00608E10)(self + 0xB884, nullptr);
    reinterpret_cast<pending_listbox_vbase_dtor>(0x005D4DD0)(self + 0xAE6C, nullptr);

    reinterpret_cast<pending_listbox_vbase_dtor>(0x00609EC0)(self + 0xA318, nullptr);
    reinterpret_cast<pending_listbox_vbase_dtor>(0x00608E10)(self + 0xAD30, nullptr);
    reinterpret_cast<pending_listbox_vbase_dtor>(0x005D4DD0)(self + 0xA318, nullptr);

}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A5FE0 ?on_iface_right_click@ReportIf@@QAEXHH@Z 0x004A5FE0-0x004A5FE3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A5FF0 ?on_iface_left_double_click@ReportIf@@QAEXHH@Z 0x004A5FF0-0x004A5FF3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6000 ?on_iface_right_double_click@ReportIf@@QAEXHH@Z 0x004A6000-0x004A6003 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6010 ?on_iface_left_down@ReportIf@@QAEXHH@Z 0x004A6010-0x004A6013 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6020 ?on_iface_right_down@ReportIf@@QAEXHH@Z 0x004A6020-0x004A6023 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6030 ?on_iface_mouse_move@ReportIf@@QAEXHH@Z 0x004A6030-0x004A6033 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6040 ?on_iface_mouse_leave@ReportIf@@QAEXHH@Z 0x004A6040-0x004A6043 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_leave(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004A6D60 ?on_iface_button_toggled@ReportIf@@QAEXHH@Z 0x004A6D60-0x004A6D63 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_button_toggled(int, int) {
}

// The original range normalization and merge algorithm operate on a fixed
// 0x200-row workspace inside ReportIf. Keys are ordered descending; each
// merge writes the scratch pair, then publishes it back to the primary pair.
// ORIGINAL: 0x004ACC30 ?merge_sort@ReportIf@@QAEXHH@Z 0x004ACC30-0x004ACD98
// RECOVERY_BINDING: v1 {"evidence": ["git:4097a7c0:src/recovered/units/004acc30.cpp;source:src/reportif.h;source:src/base.h:BaseCurrentCount;source:src/general.h:swap"], "reason": "Recover the ReportIf base-ranking merge sort over its measured 0x200-entry, 16-byte scratch workspace, using the source-owned BaseCurrentCount bound and preserving original range normalization and key-prefetch order."}
void ReportIf::merge_sort(int first, int last) {
    const int maximum = BaseCurrentCount - 1;
    if (first < 0 || maximum < 0) {
        first = 0;
    } else if (first > maximum) {
        first = maximum;
    }

    int high = maximum;
    if (last < 0 || maximum < 0) {
        high = 0;
    } else if (last <= high) {
        high = last;
    }

    if (first == high) return;

    int end = high;
    if (high < first) swap(&first, &end);

    const int middle = (end + first) / 2;
    merge_sort(first, middle);
    int right = middle + 1;
    merge_sort(right, end);

    int left = first;
    int output = first;
    // Preserve the original unchecked key prefetch: it reads both cursors'
    // values before testing which half is exhausted, including an endpoint
    // read adjacent to the active range. Do not turn this into a guarded,
    // conventional merge without a separate behavior decision.
    while (output <= end) {
        int selected;
        if (sort_entries_[left].value < sort_entries_[right].value) {
            if (right <= end) {
                selected = right++;
            } else {
                selected = left++;
            }
        } else if (left <= middle) {
            selected = left++;
        } else {
            selected = right++;
        }
        sort_entries_[output].sorted_base_id = sort_entries_[selected].base_id;
        sort_entries_[output].sorted_value = sort_entries_[selected].value;
        ++output;
    }

    for (int i = first; i <= end; ++i) {
        sort_entries_[i].base_id = sort_entries_[i].sorted_base_id;
        sort_entries_[i].value = sort_entries_[i].sorted_value;
    }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004AC130 ?close_score@ReportIf@@QAEXXZ 0x004AC130-0x004AC131 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close_score@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportIf::close_score() {
}

// Close the project page's global map overlay and the two project-selector
// buttons owned by MainInterface, not by ReportIf.
// ORIGINAL: 0x004AA930 ?close_projects@ReportIf@@QAEXXZ 0x004AA930-0x004AA964 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:4097a7c0:src/recovered/004aa930.cpp;source:src/reportif.h;source:src/worldwin.h;source:src/maininterface.h;source:src/flatbutton.h"], "reason": "Close the project report overlay by its real WorldWin and MainInterface owners when the active report is not one of the two map categories."}
void ReportIf::close_projects() {
    const int state = field_8_;
    if (state != 4 && state != 5) {
        WorldClimateWorldWin.hide_all();
        MainInterfaceGlobal.flat_button_[6].hide();
        MainInterfaceGlobal.flat_button_[7].hide();
    }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 1.
// ORIGINAL: 0x004A6320 ?on_iface_dialog_item_back_draw@ReportIf@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x004A6320-0x004A6328 BYTE_EXACT
// symbol    ?on_iface_dialog_item_back_draw@ReportIf@@QAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// size      8 bytes
// prototype int (__thiscall ?on_iface_dialog_item_back_draw@ReportIf@@QAEHPAUGraphicWin@@HHPAURECT@@@Z)(ReportIf* this, GraphicWin*, int, int, RECT*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1
Status: Complete
*/
int ReportIf::on_iface_dialog_item_back_draw(::GraphicWin *, int, int, RECT *) {
    return 1;
}


/*
Purpose: Close the two intel list boxes.

             mov esi,ecx / lea ecx,[esi+0xA2D0] / call ListBox::close
                           lea ecx,[esi+0xAE24] / call ListBox::close

         Two subobjects, in that order, and nothing else. ReportIf models no
         fields, so both offsets are documented and raw.

         ListBox::close resolves its own bases through the object's vbtable at
         run time, so each of these has to be a real ListBox carrying a table
         of its own; reaching them by offset is what the original does and is
         what keeps the two independent.
// ORIGINAL: 0x004AC980 ?close_intel@ReportIf@@QAEXXZ 0x004AC980-0x004AC99B BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?close_intel@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: n/a
Status: Complete
*/
void ReportIf::close_intel() {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    reinterpret_cast<ListBox *>(self + 0xA2D0)->close();
    reinterpret_cast<ListBox *>(self + 0xAE24)->close();
}


/*
Purpose: Close the energy report's three list boxes.

             mov ecx,0x87BE84     / call ListBox::close
             lea ecx,[esi+0xA2D0] / call ListBox::close
             lea ecx,[esi+0xAE24] / call ListBox::close

         The last two are the same subobjects close_intel closes, in the same
         order. The FIRST is not a subobject at all - it is a process-wide
         ListBox at a fixed address, and it goes first.

         0x0087BE84 is the process-wide ListBox OBJECT (reportif.cpp, named
         ReportIfEnergyListBox in the header so the address appears once). It
         needs no row in recovery-binding-classifications.csv: that file
         classifies original FUNCTION bindings, and ConsoleGlobal - the same
         shape, a named pointer to a process object - carries none either.
// ORIGINAL: 0x004A9020 ?close_energy@ReportIf@@QAEXXZ 0x004A9020-0x004A9045 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?close_energy@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: n/a
Status: Complete
*/
void ReportIf::close_energy() {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    ReportIfEnergyListBox.close();
    reinterpret_cast<ListBox *>(self + 0xA2D0)->close();
    reinterpret_cast<ListBox *>(self + 0xAE24)->close();
}


/*
// ORIGINAL: 0x004A3930 ?done@ReportIf@@QAEXXZ 0x004A3930-0x004A3946 BYTE_EXACT
// size      22 bytes
// prototype void (__thiscall ?done@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005EE280
Status: Complete
*/
// The window whose modal ReportIf::done releases, 0x00876478 - now IDENTIFIED
// as the process-wide ReportWin singleton reportwin.cpp defines: the original
// dynamic initializer at 0x00496930 constructs it (`mov ecx, 0x876478; call
// ??0ReportWin@@QAE@XZ`, followed by the atexit registration), and
// ReportWin::on_key_click closes the same object with Win::release_modal.
// done() releases its modal loop and gives up the active interface mode.
void ReportIf::done() {
    ReportWinGlobal.Win::release_modal();
    reinterpret_cast<SubInterface *>(this)->release_iface_mode();
}

// The seams and definitions for the unrecovered bodies, at the end of the file
// so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness, and being defined after the
// caller is NOT enough on its own: VC6 at /Ob2 defers codegen to the end of
// the translation unit and folds a forwarder defined later back into a caller
// defined earlier. Measured - without the pragma cl reports C4711 at
// reportif.cpp(219), which is the release_iface_mode call inside ReportIf::done,
// and done's 22-byte BYTE_EXACT match is a `call rel32` there.


/*
 * SubInterface's two called methods, hosted here for the reason spelled out in
 * subinterface.h: that class has no translation unit of its own, and this is
 * the file that already sees the header and already calls one of them.
 */
func_sub_interface_iface_mode SubInterfaceSetIfaceMode =
    original_method<func_sub_interface_iface_mode>(0x0045D310);
func_sub_interface_iface_mode SubInterfaceReleaseIfaceMode =
    original_method<func_sub_interface_iface_mode>(0x0045D380);

/*
Purpose: Make this interface the active one. The body at 0x0045D310 is NOT
         recovered; this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void SubInterface::set_iface_mode() {
    (ORIGINAL(this)->*SubInterfaceSetIfaceMode)();
}

/*
Purpose: Give up the active interface mode. The body at 0x0045D380 is NOT
         recovered; this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void SubInterface::release_iface_mode() {
    (ORIGINAL(this)->*SubInterfaceReleaseIfaceMode)();
}
#pragma auto_inline(on)

// The energy report's process-wide list box, 0x0087BE84 in the image - see
// reportif.h. Nothing constructs it before ReportIf's own init does, and
// ListBox's constructor here is the empty inline the tree models, so the
// object is the zero storage the image leaves until then.
ListBox ReportIfEnergyListBox;  // 0x0087BE84

// The process-wide ReportIf itself, 0x00885F38 - see reportif.h. The
// image's dynamic initializer at 0x00496900 constructs it through the real
// recovered constructor above, so the storage sits here.
ReportIf ReportIfGlobal;  // 0x00885F38

/*
 * Panel drawing uses the shared mutable InterfaceBoxStyles records and the
 * Buffer subobject owned by MainInterfaceGlobal. Its working rectangle is
 * exposed by GraphicWin::buffer_working_rect().
 */

// ORIGINAL: 0x004A3D20 ?reset@ReportIf@@QAEXXZ 0x004A3D20-0x004A3FBF
// RECOVERY_BINDING: v1 {"evidence": ["source:src/maininterface.h:MainInterfaceGlobal", "source:src/graphicwin.h:GraphicWin::buffer_working_rect", "source:src/datalink.cpp:1933", "docs/recovery/member-accesses.csv:ReportIf offsets 0x20, 0x30, 0x40, 0x50"], "reason": "Draw through the MainInterfaceGlobal Buffer base and its owned working clip rectangle, with typed ReportIf panel rectangles."}
// size      671 bytes
// prototype void (__thiscall ?reset@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   5   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DEF90 0x005E2DD3
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004A3D20
// measured tier  MISMATCH
// divergence     43
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004a3d20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?reset@ReportIf@@QAEXXZ  at 0x004A3D20  (671 bytes)
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


void ReportIf::reset() {
    Buffer *buf = static_cast<Buffer *>(&MainInterfaceGlobal);

    RECT *r1 = &field_20_;
    buf->box_sprite(r1, &InterfaceBoxStyles[33]);
    RECT *r2 = &field_30_;
    buf->box_sprite(r2, &InterfaceBoxStyles[33]);
    RECT *r3 = &field_50_;
    buf->box_sprite(r3, &InterfaceBoxStyles[33]);
    RECT *r4 = &field_40_;
    buf->set_clip(r4);
    r4->left -= 7;
    buf->box_sprite(r4, &InterfaceBoxStyles[33]);
    r4->left += 7;
    buf->set_clip(MainInterfaceGlobal.buffer_working_rect());

    r1->left += 3;
    r1->right -= 3;
    r1->top += 3;
    r1->bottom -= 3;
    r2->left += 3;
    r2->top += 3;
    r2->right -= 3;
    r2->bottom -= 3;
    r3->top += 4;
    r3->left += 4;
    r3->right -= 4;
    r3->bottom -= 4;
    r4->left += 4;
    r4->top += 4;
    r4->right -= 4;
    r4->bottom -= 4;
    r4->left -= 3;

    buf->box_sprite(r1, &InterfaceBoxStyles[3]);
    buf->box_sprite(r2, &InterfaceBoxStyles[3]);
    buf->box_sprite(r3, &InterfaceBoxStyles[3]);
    buf->box_sprite(r4, &InterfaceBoxStyles[3]);

    r3->left += 3;
    r3->right -= 3;
    r3->top += 3;
    r3->bottom -= 3;
    r4->left += 3;
    r4->right -= 3;
    r4->top += 3;
    r4->bottom -= 3;

    buf->box_sprite(r3, &InterfaceBoxStyles[0]);
    buf->box_sprite(r4, &InterfaceBoxStyles[0]);

    r4->left += 3;
    r1->left -= 3;
    r1->right += 3;
    r1->top -= 3;
    r1->bottom += 3;
    r2->left -= 3;
    r2->right += 3;
    r2->top -= 3;
    r2->bottom += 3;
    r3->left -= 7;
    r3->right += 7;
    r3->top -= 7;
    r3->bottom += 7;
    r4->left -= 7;
    r4->right += 7;
    r4->top -= 7;
    r4->bottom += 7;

    buf->set_font(field_1E180_, field_1E184_, field_1E184_, 0);
    buf->set_text_color(0x99, -1, 1, 1);
    buf->set_text_color2(0x99, -1, 1, 1);
    buf->set_text_color3(0xe1, -1, 1, 1);
    buf->clear_links();
}

// ORIGINAL: 0x004ABE70 ?draw_score@ReportIf@@QAEXXZ 0x004ABE70-0x004AC127
// The report renderer uses the ReportIf-owned panel rectangles and fonts,
// MainInterfaceGlobal's Buffer base, and the source-owned faction/string
// tables. The original's two static separators are represented by ordinary
// literals; their bytes at 0x00682820 and 0x00682E90 are respectively " "
// and ", ".
void ReportIf::draw_score() {
    Buffer *buf = static_cast<Buffer *>(&MainInterfaceGlobal);
    RECT *first_panel = &field_20_;

    buf->set_clip(first_panel);
    field_C_ = first_panel->left + 6;
    field_10_ = first_panel->top + 6;
    field_14_ = first_panel->right - first_panel->left - 0xC;
    field_18_ = first_panel->bottom - first_panel->top - 0xC;

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    buf->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buf->set_clip(MainInterfaceGlobal.buffer_working_rect());

    RECT *score_panel = &field_50_;
    buf->set_clip(score_panel);
    field_10_ = score_panel->top;
    const int left_inset = field_1E230_ != 0 ? 0x4C : 0;
    field_C_ = score_panel->left + left_inset;
    field_14_ = score_panel->right - left_inset - score_panel->left;
    field_18_ = score_panel->bottom - score_panel->top;

    buf->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    buf->set_text_color(0xE1, -1, 1, 1);

    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B7));

    if (field_1E230_ != 0) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }

    buf->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_,
                      static_cast<int>(strlen(StringTemp)));

    buf->set_font(field_1E180_, field_1E180_, field_1E180_, nullptr);
    buf->set_text_color(0x99, -1, 1, 1);
    field_10_ += field_1C_ * 2;
    buf->set_clip(MainInterfaceGlobal.buffer_working_rect());

    RECT *last_panel = &field_30_;
    buf->set_clip(last_panel);
    field_C_ = last_panel->left + 6;
    field_10_ = last_panel->top + 6;
    field_14_ = last_panel->right - last_panel->left - 0xC;
    field_18_ = last_panel->bottom - last_panel->top - 0xC;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_18_;
    buf->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buf->set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// ---- fixed interface bindings the recovered bodies reach ------------------
//
// The pair below is maininterface.cpp's spelling of the same two addresses
// (restated per file, the designwin.cpp convention): IfaceIdToSlot maps the
// active interface id to its sub-interface slot, ActiveIfaceId is the id.
static int *const IfaceIdToSlot = (int *)0x007AE778;
static int *const ActiveIfaceId = (int *)0x007D392C;

// 0x008846E8 - set to 1 when a topic in the intel list (field_8_ == 7) is
// clicked. Its neighbour 0x008846E4 gates the report scroll on_status shows.
// No catalogue name for either.
static int *const IntelTopicSelectFlag = (int *)0x008846E8;

// 0x00939284 - game.cpp's LocalFaction (base.cpp's do_upgrade note names the
// address). close_sat keeps the local player's own panel button alive.
static int *const LocalFaction = (int *)0x00939284;

// The three satellite-panel windows close_sat hides directly, each with its
// (flag, state) pair - the image clears the state only when the flag is
// already zero. Addresses carry the evidence; the panel identities are
// unnamed in every catalogue.
static Win *const SatPanelWindow1 = (Win *)0x00881A24;
static int *const SatPanelFlag1 = (int *)0x008824CC;
static int *const SatPanelState1 = (int *)0x008824C0;
static Win *const SatPanelWindow2 = (Win *)0x0088038C;
static int *const SatPanelFlag2 = (int *)0x00880E34;
static int *const SatPanelState2 = (int *)0x00880E28;
static Win *const SatPanelWindow3 = (Win *)0x00880ED8;
static int *const SatPanelFlag3 = (int *)0x00881980;
static int *const SatPanelState3 = (int *)0x00881974;

// The satellite panel's two animation timers, stopped by close_sat and
// driven by the same family the leaf daemons tick.
static Time *const SatPanelTimer1 = (Time *)0x00884738;
static Time *const SatPanelTimer2 = (Time *)0x00884760;

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the set_modal popups pass it as their wait
// callback (the designwin.cpp spelling).
int __cdecl pop_wait();

/*
Purpose: Show the report window modally with pop_wait as its poll callback,
         or re-report the intel category when the active interface is the
         world map's slot 5.

             cmp  [esi+8], 7 / jne ret          - intel panel only
             push y / push x / lea ecx,[esi+0x1E1BC] / call Spot::check
                                                - (x, y, &y, &x): the spot
                                                  index lands in y, the type
                                                  in x
             inc eax / je ret                   - -1 means "no spot"
             [esi+0x1E244] = y                  - the clicked topic
             [0x008846E8] = 1
             [0x00885F40] = 7                   - switch to the intel report
             cmp IfaceIdToSlot[ActiveIfaceId], 5 - world map keeps its own
                                                  modal owner
             set_modal(0, pop_wait, MainInterfaceGlobal) - slot 0xE4
             report(7, report_faction_id_, 0) on the 0x00885F38 singleton
// ORIGINAL: 0x004A5F40 ?on_iface_left_click@ReportIf@@QAEXHH@Z 0x004A5F40-0x004A5FD6
// size      150 bytes
// prototype void (__thiscall ?on_iface_left_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004A3950 0x005FAB00
// indirect  0x004A5FB1
// Homed from src/unrecovered/004a5f40.cpp.
*/
void ReportIf::on_iface_left_click(int x_coord, int y_coord) {
    if (field_8_ != 7) {
        return;
    }

    // check() answers the hit index and hands the spot's position and type
    // back through the two argument slots - the push order is &y_coord then
    // &x_coord, so the position lands in the y argument, the type in the x.
    if (reinterpret_cast<Spot *>(spot_)->check(x_coord, y_coord,
                                               &y_coord, &x_coord) == -1) {
        return;
    }
    field_1E244_ = y_coord;

    *IntelTopicSelectFlag = 1;
    field_8_ = 7;

    if (IfaceIdToSlot[*ActiveIfaceId] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
        return;
    }

    ReportIfGlobal.report(7, report_faction_id_, 0);
}

/*
Purpose: Close the satellite report: hide every faction's panel button
         except the local player's, hide the five bottom buttons, clear-and-
         hide the three satellite windows, stop both timers.

             esi = 1; edi = self + 0x13778 (flatButtonsC_)
             do { if (esi != LocalFaction) slot002(edi)      - FlatButton::
                                                                close, the
                                                                virtual
                  esi++; edi += 0xB4C } while (esi < 8)
             esi = self + 0x18720 (flatButtonsD_); five slot002 calls at
             0xB4C strides
             the three (flag, state) pairs, then the three Win::hide calls -
             DIRECT `call 0x5EDCD0`, the qualified-base spelling
             Time::stop on 0x00884738 and 0x00884760
// ORIGINAL: 0x004AB550 ?close_sat@ReportIf@@QAEXXZ 0x004AB550-0x004AB5F9
// size      169 bytes
// prototype void (__thiscall ?close_sat@ReportIf@@QAEXXZ)(ReportIf* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EDCD0 0x00616730
// indirect  0x004AB56C 0x004AB58A
// Homed from src/unrecovered/004ab550.cpp. The seed carried FILE
// BYTE_EXACT; the token is the recorder's to re-bank on this body once
// this file's link-only placeholder (ReportIf::report) is implemented, so
// it is not hand-carried here.
*/
void ReportIf::close_sat() {
    // Both runs are the image's do/while over a walking pointer: the first
    // steps flatButtonsC_ (esi = 1, 0xB4C strides, `cmp esi, 8` at the
    // bottom), keeping the local player's own faction button; the second
    // counts flatButtonsD_ down from 5.
    FlatButton *button = flatButtonsC_;
    int faction = 1;
    do {
        if (faction != *LocalFaction) {
            button->close();
        }
        ++button;
        ++faction;
    } while (faction < 8);

    button = flatButtonsD_;
    int count = 5;
    do {
        button->close();
        ++button;
        --count;
    } while (count != 0);

    if (*SatPanelFlag1 == 0) {
        *SatPanelState1 = 0;
    }
    SatPanelWindow1->Win::hide();
    if (*SatPanelFlag2 == 0) {
        *SatPanelState2 = 0;
    }
    SatPanelWindow2->Win::hide();
    if (*SatPanelFlag3 == 0) {
        *SatPanelState3 = 0;
    }
    SatPanelWindow3->Win::hide();
    SatPanelTimer1->stop();
    SatPanelTimer2->stop();
}

/*
Purpose: Unimplemented dependency - the retained report unit is not
         recovered yet; this body only keeps on_iface_left_click (and
         ReportWin::on_key_click) linking.
// ORIGINAL: 0x004A3950 ?report@ReportIf@@QAEXHHH@Z 0x004A3950-0x004A3CD3 FILE
// size      899 bytes
// prototype void (__thiscall ?report@ReportIf@@QAEXHHH@Z)(ReportIf* this, int, int, int)
// callers   12   call targets   18
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045DEA0 0x0045E540 0x004A3D20 0x004A7000 0x004A84A0 0x004A9050 0x004A9F90 0x004AA0B0 0x004AA970 0x004AB550 0x004AB600 0x004ABE70 0x004AC140 0x004C3A40 0x005B9F20 0x005D5890 0x005D5A70 0x00609F20
Status: Link-only placeholder from src/recovered/units/004a3950.cpp; original behavior is not implemented.
*/
void ReportIf::report(int report_id, int faction_id, int selection) {
    // BODY GOES HERE.
}
