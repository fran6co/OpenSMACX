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
// The window whose modal ReportIf::done releases - the image calls
// release_modal on the fixed Win at 0x00876478; nothing else in the tree
// names it, so it is modelled as real storage (the WorldClimateWorldWin
// idiom). Its identity beyond "the window done() un-modals" is unrecovered.
Win ReportIfModalWindow;

void ReportIf::done() {
    ReportIfModalWindow.Win::release_modal();
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

/*
 * Fixed-slot bindings the homed bodies arrived carrying, named from evidence.
 * The BoxSpriteStyleN numbers are indices into the BoxSpriteParams pool based
 * at 0x0078D528, on the 0x28 = sizeof(BoxSpriteParams) grid - see the block
 * comment in socialwin.cpp. InterfaceBuffer (0x007AEC64) is the shared
 * full-screen drawing Buffer and InterfaceClipRect (0x007AEC94, +0x30) the
 * RECT every one of those bodies hands to Buffer::set_clip. Each binding is an
 * `int *const`, which folds to the bare immediate, so the rename moves no byte.
 */
static int *const BoxSpriteStyle0 = (int *)0x0078D528;
static int *const BoxSpriteStyle3 = (int *)0x0078D5A0;
static int *const BoxSpriteStyle33 = (int *)0x0078DA50;
static int *const InterfaceBuffer = (int *)0x007AEC64;
static int *const InterfaceClipRect = (int *)0x007AEC94;

// ORIGINAL: 0x004A3D20 ?reset@ReportIf@@QAEXXZ 0x004A3D20-0x004A3FBF FILE
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
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(InterfaceBuffer);

    RECT *r1 = reinterpret_cast<RECT *>(self + 0x20);
    buf->box_sprite(r1, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle33));
    RECT *r2 = reinterpret_cast<RECT *>(self + 0x30);
    buf->box_sprite(r2, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle33));
    RECT *r3 = reinterpret_cast<RECT *>(self + 0x50);
    buf->box_sprite(r3, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle33));
    RECT *r4 = reinterpret_cast<RECT *>(self + 0x40);
    buf->set_clip(r4);
    r4->left -= 7;
    buf->box_sprite(r4, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle33));
    r4->left += 7;
    buf->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));

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

    buf->box_sprite(r1, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle3));
    buf->box_sprite(r2, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle3));
    buf->box_sprite(r3, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle3));
    buf->box_sprite(r4, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle3));

    r3->left += 3;
    r3->right -= 3;
    r3->top += 3;
    r3->bottom -= 3;
    r4->left += 3;
    r4->right -= 3;
    r4->top += 3;
    r4->bottom -= 3;

    buf->box_sprite(r3, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle0));
    buf->box_sprite(r4, reinterpret_cast<BoxSpriteParams *>(BoxSpriteStyle0));

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

    Font *f0 = *reinterpret_cast<Font **>(self + 0x1e180);
    Font *f1 = *reinterpret_cast<Font **>(self + 0x1e184);
    buf->set_font(f0, f1, f1, 0);
    buf->set_text_color(0x99, -1, 1, 1);
    buf->set_text_color2(0x99, -1, 1, 1);
    buf->set_text_color3(0xe1, -1, 1, 1);
    buf->clear_links();
}

