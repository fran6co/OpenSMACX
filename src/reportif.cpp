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
#include "console.h"  // ConsoleGlobal.faction_id_ owns original939284
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
#include "technology.h"  // has_tech_call
#include "strings.h"  // StringTable, StringTemp
#include "vector_teardown.h"

void __cdecl energy_compute(int faction_id, int mode);
void __cdecl scroll_small();

// Popup's scroll lifecycle callbacks operate on ReportWin's embedded Scroll.
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);

// ORIGINAL: 0x004A3FC0 ?on_redraw@ReportIf@@QAEXXZ 0x004A3FC0-0x004A401F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004a3fc0.cpp", "image:0x004A3FC0-0x004A401F and dispatch table0x004A4020", "source:src/reportif.h"], "reason": "Recover the report-interface redraw operation through its real category member, reset and owned drawing methods. Original direct calls and nine-entry dispatch preserve the default/no-draw category without vtable or executable-address dispatch."}
void ReportIf::on_redraw() {
    reset();
    switch (field_8_) {
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    case 0: draw_intel(); break;
    }
}



/*
Purpose: Construct the SubInterface base, then every
         sub-object in image order - seven FlatButton runs, three Sprite
         runs, two ListBoxes, four ButtonGroups and fourteen individual
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
ReportIf::ReportIf() {
    // Declaration-order construction / reverse-order teardown is automatic.
}

// ListBox's ordinary member teardown includes its complete-object virtual
// bases: GraphicWin at +0x48 and Dialog at +0xA60, matching the original
// adjusted destructor calls (609EC0, 608E10, 5D4DD0).

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
    // Declaration-order construction / reverse-order teardown is automatic.
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
         order. The first is ReportWinGlobal's owned listBox_ at +0x5A0C;
         it is borrowed here and closes first.

         Original 0x0087BE84 = ReportWinGlobal 0x00876478 + 0x5A0C.
         Its construction and teardown belong to ReportWin, not an
         independent global allocation.
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
    ReportWinGlobal.listBox_.close();
    listBox1_.close();
    listBox2_.close();
}

// The constructor owns this ListBox and these FlatButtons as members;
// the retained close_labs seed closes/hides those same
// objects in this order. Slot 2 is the inherited Win::hide virtual.
// ORIGINAL: 0x004A8470 ?close_labs@ReportIf@@QAEXXZ 0x004A8470-0x004A849E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004a8470.cpp", "source:src/reportif.h", "source:src/flatbutton.h", "source:src/listbox.h"], "reason": "Close the ReportIf lab list and hide its two placement-constructed FlatButtons through their typed Win virtual method."}
void ReportIf::close_labs() {
    listBox1_.close();
    flatButton1_.hide();
    flatButton2_.hide();
}

// The three FlatButtons are genuine ReportIf members. The seed
// calls Win slot 2 on them in this order; keep the original object identities
// while using the source-owned virtual method instead of a vtable shim.
// ORIGINAL: 0x004ABE30 ?close_security@ReportIf@@QAEXXZ 0x004ABE30-0x004ABE62 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/004abe30.cpp", "source:src/reportif.h", "source:src/flatbutton.h", "source:src/win.h"], "reason": "Hide the three ReportIf security FlatButtons through Win slot 2 in retained-seed order."}
void ReportIf::close_security() {
    flatButton12_.hide();
    flatButton14_.hide();
    flatButton13_.hide();
}

// ReportWin owns these three controls as members; close_ops clears
// each ungrouped state word before hiding it. ReportIf's own controls and the
// final MainInterface pair remain their respective typed owners.
// ORIGINAL: 0x004A9F90 ?close_ops@ReportIf@@QAEXXZ 0x004A9F90-0x004AA0AF
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004a9f90.cpp", "source:src/reportif.h", "source:src/reportwin.h", "source:src/basebutton.h"], "reason": "Close the ReportIf and ReportWin operation controls through their named owners, preserve conditional map cleanup, and clear only ungrouped ReportWin BaseButton state through an encapsulated helper."}
void ReportIf::close_ops() {
    listBox1_.close();
    flatButton3_.hide();
    flatButton4_.hide();
    flatButton5_.hide();
    flatButton6_.hide();
    flatButton7_.hide();
    flatButton8_.hide();
    flatButton9_.hide();
    flatButton10_.hide();
    flatButton11_.hide();

    FlatButton *button = &ReportWinGlobal.flatButton4_;
    button->reset_field_A9C_if_ungrouped();
    button->hide();
    button = &ReportWinGlobal.flatButton5_;
    button->reset_field_A9C_if_ungrouped();
    button->hide();
    button = &ReportWinGlobal.flatButton6_;
    button->reset_field_A9C_if_ungrouped();
    button->hide();

    if (field_8_ != 4 && field_8_ != 5) {
        WorldClimateWorldWin.hide_all();
        MainInterfaceGlobal.flat_button_[6].hide();
        MainInterfaceGlobal.flat_button_[7].hide();
    }
}

// Animation storage is the ReportIf sprite run; the destination is the
// Buffer base of the global WorldClimateWorldWin. Its timeout lives in the
// genuine ReportWin-owned Time member.
// ORIGINAL: 0x004A4060 ?bl_anim@ReportIf@@QAEXXZ 0x004A4060-0x004A4191
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004a4060.cpp", "source:src/reportif.h", "source:src/worldwin.h", "source:src/reportwin.h", "source:src/sprite.h"], "reason": "Advance ReportIf animation with the declared sprite run and coordinate state, drawing to the WorldClimateWorldWin Buffer base and stopping the ReportWin-owned Time at terminal frame 10."}
void ReportIf::bl_anim() {
    WorldClimateWorldWin.redraw();
    Buffer *const destination = static_cast<Buffer *>(&WorldClimateWorldWin);
    switch (animation_state_) {
    case -1:
    case 0:
    case 8:
    case 9:
        spritesB_[0].draw(destination, spritesB_[0].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 1:
    case 7:
        spritesB_[1].draw(destination, spritesB_[1].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 2:
    case 4:
    case 6:
        spritesB_[2].draw(destination, spritesB_[2].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 3:
    case 5:
        spritesB_[3].draw(destination, spritesB_[3].transparent_index(),
                          animation_x_, animation_y_);
        ++animation_state_;
        return;
    case 10:
        ReportWinGlobal.time1_.stop();
        break;
    }
    ++animation_state_;
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

// Research state87BE24/885F30 is ReportWinGlobal+59AC/+FAB8.
// The energy list87BE84 is ReportWinGlobal+5A0C. No duplicate storage.

// The process-wide ReportIf itself, 0x00885F38 - see reportif.h. The
// image's dynamic initializer at 0x00496900 constructs it through the real
// recovered constructor above, so the storage sits here.
// Original constructor 0x004AD170 initializes this singleton; its two list
// receivers at 0x00890208/0x00890D5C are owned +0xA2D0/+0xAE24 members.
// This start-only binding does not infer a complete original allocation.
// RECOVERY_OBJECT: v1 0x00885F38 ?ReportIfGlobal@@3VReportIf@@A
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

// MainInterface owns the interface-slot array and active interface ID.

// close_sat borrows ReportWinGlobal's buttons9/7/8 and timers2/3.
// Original881A24/88038C/880ED8 are876478+B5AC/9F14/AA60.
// Their +AA8/+A9C fields are BaseButton.group_/field_A9C_; the existing
// typed reset_field_A9C_if_ungrouped preserves the original inline condition.
// Original884738/884760 are876478+E2C0/E2E8, not timer pointers/globals.

// 0x005398E0 ?pop_wait@@YAHXZ - defined in faction.cpp (BYTE_EXACT); no
// header declares it, and the set_modal popups pass it as their wait
// callback (the designwin.cpp spelling).
int __cdecl pop_wait();

// The eight menu commands all use the same already-owned report window and
// interface mode. Retained seeds at 0x004AC9A0..0x004ACC29 establish each
// category value and the modal-versus-direct branch.
// ORIGINAL: 0x004AC9A0 ?report_labs@@YAXXZ 0x004AC9A0-0x004AC9EC
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004ac9a0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_labs seed identifies this free report command and its original extent. Category 2 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_labs() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 2;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(2, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004AC9F0 ?report_energy@@YAXXZ 0x004AC9F0-0x004ACA3C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004ac9f0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_energy seed identifies this free report command and its original extent. Category 3 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_energy() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 3;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(3, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACA40 ?report_ops@@YAXXZ 0x004ACA40-0x004ACA8C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004aca40.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_ops seed identifies this free report command and its original extent. Category 4 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_ops() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 4;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(4, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACA90 ?report_projects@@YAXXZ 0x004ACA90-0x004ACADC
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004aca90.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_projects seed identifies this free report command and its original extent. Category 5 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_projects() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 5;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(5, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACAE0 ?report_sat@@YAXXZ 0x004ACAE0-0x004ACB2C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004acae0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_sat seed identifies this free report command and its original extent. Category 6 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_sat() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 6;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(6, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACB30 ?report_security@@YAXXZ 0x004ACB30-0x004ACB7C
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004acb30.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_security seed identifies this free report command and its original extent. Category 7 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_security() {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 7;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(7, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACB80 ?report_score@@YAXH@Z 0x004ACB80-0x004ACBCC
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004acb80.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_score seed identifies this free report command and its original extent. Category 8 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_score(int) {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 8;
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(8, ReportIfGlobal.report_faction_id(), 0);
    }
}

// ORIGINAL: 0x004ACBD0 ?report_intel@@YAXH@Z 0x004ACBD0-0x004ACC29
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004acbd0.cpp", "source:src/reportif.cpp", "source:src/reportif.h", "source:src/reportwin.h"], "reason": "The retained report_intel seed identifies this free report command and its original extent. Category 0 selects the existing ReportIf state; the original modal call resolves to ReportWinGlobal set_modal and the direct branch to ReportIf report."}
void __cdecl report_intel(int selection) {
    int active_iface = ActiveIfaceIdStorage;
    ReportIfGlobal.field_8_ = 0;
    ReportIfGlobal.set_report_selection(selection);
    if (IfaceIdToSlotStorage[active_iface] != 5) {
        ReportWinGlobal.set_modal(0, pop_wait, &MainInterfaceGlobal);
    } else {
        ReportIfGlobal.report(0, ReportIfGlobal.report_faction_id(), selection);
    }
}

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
    if (spot_.check(x_coord, y_coord,
                                               &y_coord, &x_coord) == -1) {
        return;
    }
    field_1E244_ = y_coord;

    ReportWinGlobal.select_intel_topic();
    field_8_ = 7;

    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 5) {
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
        if (faction != ConsoleGlobal.faction_id_) {
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

    ReportWinGlobal.flatButton9_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton9_.Win::hide();
    ReportWinGlobal.flatButton7_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton7_.Win::hide();
    ReportWinGlobal.flatButton8_.reset_field_A9C_if_ungrouped();
    ReportWinGlobal.flatButton8_.Win::hide();
    ReportWinGlobal.time2_.stop();
    ReportWinGlobal.time3_.stop();
}

/*
Purpose: Switch report pages using the existing interface and window owners,
         preserving the original research-selection transition and redraw.
// ORIGINAL: 0x004A3950 ?report@ReportIf@@QAEXHHH@Z 0x004A3950-0x004A3CD3 FILE
// size      899 bytes
// prototype void (__thiscall ?report@ReportIf@@QAEXHHH@Z)(ReportIf* this, int, int, int)
// callers   12   call targets   18
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045DEA0 0x0045E540 0x004A3D20 0x004A7000 0x004A84A0 0x004A9050 0x004A9F90 0x004AA0B0 0x004AA970 0x004AB550 0x004AB600 0x004ABE70 0x004AC140 0x004C3A40 0x005B9F20 0x005D5890 0x005D5A70 0x00609F20
Status: Report dispatch and state transition recovered; seven referenced page
        renderers remain explicit, link-only placeholders.
*/
void ReportIf::report(int report_id, int faction_id, int selection) {
    const int previous_page = report_page_history_[0];
    switch (previous_page) {
    case 2:
        listBox1_.close();
        flatButton1_.hide();
        flatButton2_.hide();
        break;
    case 3:
        ReportWinGlobal.listBox_.close();
        // Fall through: energy and intel share both embedded lists.
    case 0:
        listBox1_.close();
        listBox2_.close();
        break;
    case 4:
        close_ops();
        break;
    case 5:
        if (field_8_ != 4 && field_8_ != 5) {
            WorldClimateWorldWin.hide_all();
            MainInterfaceGlobal.flat_button_[6].hide();
            MainInterfaceGlobal.flat_button_[7].hide();
        }
        break;
    case 6:
        close_sat();
        break;
    case 7:
        flatButton12_.hide();
        flatButton14_.hide();
        flatButton13_.hide();
        break;
    }

    // The original category run is 2..8; category 1 has no report page or
    // typed control in the class layout. Zero denotes the intel list.
    if (previous_page >= 2 && previous_page <= 8) {
        FlatButton &old_button = flatButtonsB_[previous_page - 2];
        old_button.set_report_page_state(-1);
        old_button.redraw();
    }
    if (report_id >= 2 && report_id <= 8) {
        FlatButton &new_button = flatButtonsB_[report_id - 2];
        new_button.set_report_page_state(2);
        new_button.redraw();
    }

    report_selection_ = selection;
    field_8_ = report_id;
    report_faction_id_ = faction_id;

    if (previous_page != 2 && ReportWinGlobal.shared_report_selection() != -1) {
        int next_focus;
        if (!(GameState & STATE_OMNISCIENT_VIEW)
            && (GameRules & RULES_BLIND_RESEARCH)) {
            if (ReportWinGlobal.shared_research_focus() >= 0 && ReportWinGlobal.shared_research_focus() <= 0x59
                && has_tech_call(ReportWinGlobal.shared_research_focus(), faction_id)) {
                next_focus = ReportWinGlobal.shared_research_focus();
            } else {
                next_focus = ReportWinGlobal.shared_research_focus();
                for (int tech_id = 0; tech_id < 0x59; ++tech_id) {
                    if (has_tech_call(tech_id, faction_id)) {
                        next_focus = tech_id;
                        break;
                    }
                }
            }
        } else {
            next_focus = PlayersData[faction_id].tech_id_researching;
        }
        ReportWinGlobal.shared_research_focus() = next_focus;
        if (ReportWinGlobal.shared_research_focus() < 0 || ReportWinGlobal.shared_research_focus() > 0x59) {
            ReportWinGlobal.shared_research_focus() = 0;
        }
        ReportWinGlobal.shared_report_selection() = -1;
    }

    if (faction_id <= 0) {
        return;
    }

    for (int index = 18; index > 0; --index) {
        report_page_history_[index] = report_page_history_[index - 1];
    }
    report_page_history_[0] = field_8_;
    const int current_page = report_page_history_[0];
    if (current_page == 4 || current_page == 5 || current_page == 7) {
        if (current_page != report_page_history_[1] || report_topic_changed_) {
            ReportWinGlobal.field_E270_ = 1;
            report_topic_changed_ = 0;
        }
    } else if (ReportWinGlobal.field_C0F8_ != 0) {
        scroll_hide(&ReportWinGlobal, &ReportWinGlobal.scroll_);
        ReportWinGlobal.field_E270_ = 0;
        ReportWinGlobal.field_E26C_ = 0;
        ReportWinGlobal.field_C0F8_ = 0;
    }

    reset();
    switch (field_8_) {
    case 0: draw_intel(); break;
    case 2: draw_labs(); break;
    case 3: draw_energy(); break;
    case 4: draw_ops(); break;
    case 5: draw_projects(); break;
    case 6: draw_sat(); break;
    case 7: draw_security(); break;
    case 8: draw_score(); break;
    }
    MainInterfaceGlobal.soft_update();
    ReportWinGlobal.redraw();
    if (ReportWinGlobal.field_E26C_ != 0 && !report_scroll_blocked_) {
        scroll_show(&ReportWinGlobal, &ReportWinGlobal.scroll_);
        ReportWinGlobal.field_E26C_ = 0;
        ReportWinGlobal.field_C0F8_ = 1;
    }
}

// These panel renderers are same-owner dependencies of report(). Their
// original bodies and unresolved rows remain in the retained seeds.
// ORIGINAL: 0x004A7000 ?draw_labs@ReportIf@@QAEXXZ 0x004A7000-0x004A842F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004a7000.cpp", "source:src/reportif.h"], "reason": "Same-owner panel dependency: typed frame and list setup recovered from retained seed; row generation remains unfinished."}
void ReportIf::draw_labs() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + 26;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    field_10_ += 32;
    field_18_ -= 32;
    draw_workspace_rect_.left = field_C_ + 3;
    draw_workspace_rect_.top = field_10_ + 3;
    draw_workspace_rect_.right = field_C_ + field_14_ - 3;
    draw_workspace_rect_.bottom = field_10_ + field_18_ - 3;
    scroll_small();
    // The constructor places this ListBox at +0xA2D0. It is still raw
    // storage because its one-argument construction is unresolved.
    ListBox &list = listBox1_;
    list.close();
    Dialog &dialog = static_cast<Dialog &>(list);
    dialog.init(&draw_workspace_rect_, 0);
    dialog.field_48_ = field_1C_ + 3;
    dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    dialog.set_dialog_text_color(0x99, -1, 1, 1);
    dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    // Pending: faction base rows and sorted technology rows.
}

// ORIGINAL: 0x004A84A0 ?draw_energy@ReportIf@@QAEXXZ 0x004A84A0-0x004A9017
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004a84a0.cpp", "source:src/reportif.h", "image:0x004A84AE-0x004A84BC pushes mode zero then report faction for the cdecl energy_compute call"], "reason": "Same-owner panel dependency: typed energy calculation, frame and list setup recovered from retained seed; correct the seed's reversed displayed arguments using the original push order. Rows remain unfinished."}
void ReportIf::draw_energy() {
    energy_compute(report_faction_id_, 0);
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_1C_ + 6;
    draw_workspace_rect_.bottom = field_10_ + field_14_;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.right = field_10_ + field_18_;
    draw_workspace_rect_.bottom = field_14_ + field_C_;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    scroll_small();
    ListBox &list = listBox1_;
    list.close();
    Dialog &dialog = static_cast<Dialog &>(list);
    dialog.init(&draw_workspace_rect_, 0);
    dialog.field_48_ = field_1C_ + 3;
    dialog.set_dialog_font(field_1E184_, field_1E184_, field_1E184_);
    dialog.set_dialog_text_color(0x99, -1, 1, 1);
    dialog.set_dialog_text_color2(0x99, -1, 1, 1);
    dialog.set_dialog_text_color3(0x99, -1, 1, 1);
    // Pending: base energy rows, list selection and report caption.
}

// ORIGINAL: 0x004A9050 ?draw_ops@ReportIf@@QAEXXZ 0x004A9050-0x004A9F85
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004a9050.cpp", "source:src/reportif.h"], "reason": "Same-owner panel dependency: typed operational frame and list setup recovered from retained seed; operational rows remain unfinished."}
void ReportIf::draw_ops() {
    if (report_page_history_[1] != 4 && report_page_history_[1] != 5) {
        WorldClimateWorldWin.show_all();
        // The seed also redraws four adjacent windows whose ownership is
        // unresolved; keep that dependency for the owning panel batch.
    }
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 9;
    field_10_ = field_20_.top + 40;
    field_14_ = field_20_.right - field_20_.left - 18;
    field_18_ = field_20_.bottom - field_20_.top - 46;

    draw_workspace_rect_.left = field_20_.left + 6;
    draw_workspace_rect_.top = field_20_.top + 6;
    draw_workspace_rect_.right = field_20_.right - 6;
    draw_workspace_rect_.bottom = field_20_.top + 32;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);

    // The seed's list page is selected by a neighboring button state. Its
    // page-specific row generation and alternate graphic view remain pending.
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    buffer->box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    draw_workspace_rect_.left = field_C_ - 3;
    draw_workspace_rect_.top = field_10_ - 3;
    draw_workspace_rect_.right = field_C_ + field_14_ + 3;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 9;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0xF0C));
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    StringTemp[0] = 0;
    MainInterfaceGlobal.set_date(StringTemp);
}

// ORIGINAL: 0x004AA0B0 ?draw_projects@ReportIf@@QAEXXZ 0x004AA0B0-0x004AA921;0x00659595-0x006595C9
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004aa0b0.cpp", "source:src/reportif.h"], "reason": "Partial panel recovery: typed clipping, frames, headings and faction caption; selected-project description and panel button behavior remain unfinished. Retained seed establishes identity and extent."}
void ReportIf::draw_projects() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;

    // The seed draws a narrow title frame followed by a second frame around
    // the selected project's description.  Both use the shared work rectangle.
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);

    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x32A));
    buffer->write_cent_l(StringTemp, draw_workspace_rect_.left, field_10_ + 4,
                         field_14_, static_cast<int>(strlen(StringTemp)));

    // The project selection and its StringList description still depend on
    // an unnamed global selection and base-production record in the seed.
    // BODY GOES HERE: render the selected project's description.

    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    field_10_ = field_50_.top;
    const int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = field_50_.left + inset;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B4));
    if (field_1E230_) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());

    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    buffer->box_sprite(&field_30_, &InterfaceBoxStyles[33]);
    draw_workspace_rect_.left = field_C_ - 3;
    draw_workspace_rect_.top = field_10_ - 3;
    draw_workspace_rect_.right = field_C_ + field_14_ + 3;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 9;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[3]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x3C3));
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    MainInterfaceGlobal.set_date(StringTemp);
}

// ORIGINAL: 0x004AA970 ?draw_sat@ReportIf@@QAEXXZ 0x004AA970-0x004AB54B
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004aa970.cpp", "source:src/reportif.h"], "reason": "Link-only same-owner panel dependency called by recovered ReportIf report; retained seed establishes identity and extent; body remains explicitly unfinished."}
void ReportIf::draw_sat() {
    // BODY GOES HERE.
}

// ORIGINAL: 0x004AB600 ?draw_security@ReportIf@@QAEXXZ 0x004AB600-0x004ABE24
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004ab600.cpp", "source:src/reportif.h"], "reason": "Partial panel recovery: typed clipping, frames, headings and faction caption; five security totals, Spot rows and panel button behavior remain unfinished. Retained seed establishes identity and extent."}
void ReportIf::draw_security() {
    Buffer *const buffer = static_cast<Buffer *>(&MainInterfaceGlobal);
    buffer->set_clip(&field_20_);
    field_C_ = field_20_.left + 6;
    field_10_ = field_20_.top + 6;
    field_14_ = field_20_.right - field_20_.left - 12;
    field_18_ = field_20_.bottom - field_20_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.top = draw_workspace_rect_.bottom + 2;
    draw_workspace_rect_.bottom = field_20_.bottom - 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    draw_workspace_rect_.left += 3;
    draw_workspace_rect_.top += 3;
    draw_workspace_rect_.right -= 3;
    draw_workspace_rect_.bottom -= 3;
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x38A));
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_,
                         static_cast<int>(strlen(StringTemp)));

    // BODY GOES HERE: the seed's five security totals and their Spot rows
    // require typed unit, faction and base records before they can be moved.

    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_50_);
    const int inset = field_1E230_ ? 0x4C : 0;
    field_C_ = field_50_.left + inset;
    field_10_ = field_50_.top;
    field_14_ = field_50_.right - field_50_.left - inset;
    field_18_ = field_50_.bottom - field_50_.top;
    buffer->set_font(field_1E190_, field_1E194_, field_1E198_, nullptr);
    buffer->set_text_color(0xE1, -1, 1, 1);
    StringTemp[0] = 0;
    strcat(StringTemp, Players[report_faction_id_].adj_name_faction);
    CharUpperA(StringTemp);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2B6));
    if (field_1E230_) {
        strcat(StringTemp, ", ");
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(0x2DA));
        strcat(StringTemp, " ");
        say_year(StringTemp);
    }
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 10, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
    buffer->set_clip(&field_30_);
    field_C_ = field_30_.left + 6;
    field_10_ = field_30_.top + 6;
    field_14_ = field_30_.right - field_30_.left - 12;
    field_18_ = field_30_.bottom - field_30_.top - 12;
    draw_workspace_rect_.left = field_C_;
    draw_workspace_rect_.top = field_10_;
    draw_workspace_rect_.right = field_C_ + field_14_;
    draw_workspace_rect_.bottom = field_10_ + field_1C_ + 6;
    buffer->box_sprite(&draw_workspace_rect_, &InterfaceBoxStyles[0]);
    buffer->set_font(field_1E184_, field_1E194_, field_1E198_, nullptr);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x3BB));
    buffer->write_cent_l(StringTemp, field_C_, field_10_ + 4, field_14_,
                         static_cast<int>(strlen(StringTemp)));
    buffer->set_clip(MainInterfaceGlobal.buffer_working_rect());
}

// ORIGINAL: 0x004AC140 ?draw_intel@ReportIf@@QAEXXZ 0x004AC140-0x004AC977
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004ac140.cpp", "source:src/reportif.h"], "reason": "Link-only same-owner panel dependency called by recovered ReportIf report; retained seed establishes identity and extent; body remains explicitly unfinished."}
void ReportIf::draw_intel() {
    // BODY GOES HERE.
}
