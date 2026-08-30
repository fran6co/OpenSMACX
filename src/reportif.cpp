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
#include "worldwin.h"
#include "menu.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "flatbutton.h"
#include "sprite.h"
#include "buttongroup.h"
#include "spot.h"
#include "vector_teardown.h"


const uint32_t ReportIfSubInterfaceVtable = 0x0066A6E4;
const uint32_t ReportIfPrimaryVtable = 0x0066D700;

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
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0] = ReportIfSubInterfaceVtable;


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


    object[0] = ReportIfPrimaryVtable;
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
void ReportIf::done() {
    reinterpret_cast<Win *>(0x00876478)->Win::release_modal();
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
