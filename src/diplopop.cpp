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
#include "diplopop.h"
#include "mapwin.h"  // MainInterfaceGlobal - the modal owner at 0x007AE820
#include "datalink.h"
#include "worldwin.h"
#include "base.h"
#include "alpha.h"
#include "strings.h"
#include "sprite.h"

void __cdecl report_intel(int selection);
void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);

// CRT entry 0x0043EFC0 constructs this object and registers 0x0043EFE0,
// which destroys the same receiver. The CRT table at 0x00682080 contains
// that initializer. Its complete constructed member footprint is 0x433C;
// the original storage is zero-initialized before those constructors run.
// RECOVERY_OBJECT: v2 0x00733990 0x00737CCC ?DiplomacyPopup@@3VDiploPop@@A 0x0043EFF0
// ORIGINAL: 0x0043EFC0 ??__EDiplomacyPopup@@YAXXZ 0x0043EFC0-0x0043EFD6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original relocated CRT slot682080 names43EFC0, which constructs733990 with43EFF0 then registers43EFE0 through atexit645398. Native compiler emits the constructor/registration helpers as an initializer chain; that split remains a measurement difference, not a source workaround."], "reason": "Bind the natural C++ process-lifetime initializer for the diplomacy popup, without a handwritten startup thunk."}
// symbol    _$E21
// ORIGINAL: 0x0043EFE0 ??__FDiplomacyPopup@@YAXXZ 0x0043EFE0-0x0043EFEA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original43EFE0 loads733990 and tail-calls the complete destructor440D50. This is the entry registered by43EFC0, not the scalar deleting destructor and not an independently allocated popup."], "reason": "Bind the compiler-generated teardown registered for the same real diplomacy popup object."}
// symbol    _$E19
DiploPop DiplomacyPopup;

// ORIGINAL: 0x0043EFF0 ??0DiploPop@@QAE@XZ 0x0043EFF0-0x0043F0EA;0x00654880-0x006548F4
// RECOVERY_BINDING: v1 {"evidence": ["Original43EFF0..43F0EA and unwind654880..6548F4 construct GraphicWin, the secondary interface base, three Fonts, two Times atA98/AC0, Flics atAE8/15CC and three FlatButtons at2158 with strideB4C. All compiler-generated unwind targets belong to those real lifetimes.", "43F0C8/43F0D2 set counterpart factionA94=-1 and field2154=1 only after all members.43F30D publishes the faction argument intoA94. The old field20AC was inside the second Flic, not an additional scalar. Original CRT43EFC0 constructs733990 and registers its destructor wrapper43EFE0."], "reason": "Recover the complete diplomacy-popup construction through real C++ bases and embedded member lifetimes."}
DiploPop::DiploPop() {
    // Original 0x0043F0C8/0x0043F0D2 publish these only after the three
    // Fonts, two Times, two Flics and three FlatButtons have been built.
    counterpart_faction_ = -1;
    field_2154_ = 1;
}

// ORIGINAL: 0x00440D50 ??1DiploPop@@QAE@XZ 0x00440D50-0x00440EB8;0x006547B0-0x00654871
// RECOVERY_BINDING: v1 {"evidence": ["Original440D50..440EB8 and unwind6547B0..654871 restore the class tables, perform the same two Time stops, virtual hide and direct GraphicWin close as close(), then resetA94/20B0/2154. Normal C++ destructor dispatch supplies those table transitions; no table stores are handwritten.", "440DEB onward destroys FlatButton[3] in reverse, then Flic2/Flic1 including their Palette/Buffer members, Time2/Time1, Font3/Font2/Font1, and GraphicWin. The complete unwind path preserves the same partially live subobjects. Existing downstream lifecycle and renderer limitations are not waived."], "reason": "Recover complete diplomacy-popup destruction through close followed by automatic reverse member and base destruction."}
// symbol    ??1DiploPop@@UAE@XZ
DiploPop::~DiploPop() {
    // The original inlines close before reverse member/base teardown.
    // Normal C++ destruction also owns partial-construction unwinding.
    close();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004404C0 ?on_iface_left_click@DiploPop@@QAEXHH@Z 0x004404C0-0x004404C3 BYTE_EXACT
// symbol    ?on_iface_left_click@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004404D0 ?on_iface_right_click@DiploPop@@QAEXHH@Z 0x004404D0-0x004404D3 BYTE_EXACT
// symbol    ?on_iface_right_click@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004404E0 ?on_iface_left_down@DiploPop@@QAEXHH@Z 0x004404E0-0x004404E3 BYTE_EXACT
// symbol    ?on_iface_left_down@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004404F0 ?on_iface_right_down@DiploPop@@QAEXHH@Z 0x004404F0-0x004404F3 BYTE_EXACT
// symbol    ?on_iface_right_down@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440500 ?on_iface_mouse_move@DiploPop@@QAEXHH@Z 0x00440500-0x00440503 BYTE_EXACT
// symbol    ?on_iface_mouse_move@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440510 ?on_iface_mouse_leave@DiploPop@@QAEXHH@Z 0x00440510-0x00440513 BYTE_EXACT
// symbol    ?on_iface_mouse_leave@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440520 ?on_iface_selected@DiploPop@@QAEXHH@Z 0x00440520-0x00440523 BYTE_EXACT
// symbol    ?on_iface_selected@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440530 ?on_iface_left_double_click@DiploPop@@QAEXHH@Z 0x00440530-0x00440533 BYTE_EXACT
// symbol    ?on_iface_left_double_click@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440540 ?on_iface_right_double_click@DiploPop@@QAEXHH@Z 0x00440540-0x00440543 BYTE_EXACT
// symbol    ?on_iface_right_double_click@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00440610 ?on_iface_button_toggled@DiploPop@@QAEXHH@Z 0x00440610-0x00440613 BYTE_EXACT
// symbol    ?on_iface_button_toggled@DiploPop@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DiploPop@@QAEXHH@Z)(DiploPop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploPop::on_iface_button_toggled(int, int) {
}

// ---- the interface table's own slots still carrying an original body ------

// PENDING 0x00440380 ?on_status@DiploPop@@QAEXH@Z 0x00440380-0x004404B4
// symbol    ?on_status@DiploPop@@UAEXH@Z
// size      308 bytes
// prototype void (__thiscall ?on_status@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// ORIGINAL: 0x00440380 ?on_status@DiploPop@@QAEXH@Z 0x00440380-0x004404B4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00440380.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x00440380 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
// symbol    ?on_status@DiploPop@@UAEXH@Z
void DiploPop::on_status(int mode) {
    GraphicWin *window = static_cast<GraphicWin *>(this);
    if (mode) {
        MainInterfaceGlobal.on_redraw();
        for (int i = 0; i < 3; ++i) flatButtons_[i].show(0);
        slide_show(window, 2);
        show(3);
        time1_.start();
        time2_.start();
        MainInterfaceGlobal.flat_button(6).show(0);
        MainInterfaceGlobal.flat_button(7).show(0);
        MainInterfaceGlobal.button_group(1).set(-1, 1);
        flatButtons_[2].set(1);
        MainInterfaceGlobal.redraw();
    } else {
        time1_.stop();
        time2_.stop();
        flic2_.rewind();
        slide_hide(window, 2);
        for (int i = 0; i < 3; ++i) flatButtons_[i].hide();
        MainInterfaceGlobal.flat_button(6).hide();
        MainInterfaceGlobal.flat_button(7).hide();
        hide();
    }
}

// PENDING 0x00440550 ?on_iface_button_clicked@DiploPop@@QAEXH@Z 0x00440550-0x00440604
// Historical emitted spelling was ?vslot_13@DiploPop@@UAEXH@Z.
// size      180 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// indirect  0x00440582
// ORIGINAL: 0x00440550 ?on_iface_button_clicked@DiploPop@@QAEXH@Z 0x00440550-0x00440604 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00440550.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x00440550 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
// symbol    ?on_iface_button_clicked@DiploPop@@UAEXH@Z
void DiploPop::on_iface_button_clicked(int button_id) {
    switch (button_id) {
    case 0:
        DatalinkMain.exec(0xF, counterpart_faction_);
        break;
    case 1:
        report_intel(counterpart_faction_);
        return;
    case 2:
        WorldClimateWorldWin.hide();
        MapWinTable[1]->hide();
        MainInterfaceGlobal.button_group(1).set(-1, 1);
        flatButtons_[2].set(1);
        draw_hq();
        break;
    }
}

// PENDING 0x00440620 ?on_iface_group_clicked@DiploPop@@QAEXHHH@Z 0x00440620-0x004406CB
// symbol    ?on_iface_group_clicked@DiploPop@@UAEXHHH@Z
// size      171 bytes
// prototype void (__thiscall ?on_iface_group_clicked@DiploPop@@QAEXHHH@Z)(DiploPop* this, int, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// indirect  0x00440643
// ORIGINAL: 0x00440620 ?on_iface_group_clicked@DiploPop@@QAEXHHH@Z 0x00440620-0x004406CB FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00440620.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x00440620 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
// symbol    ?on_iface_group_clicked@DiploPop@@UAEXHHH@Z
void DiploPop::on_iface_group_clicked(int group, int x_coord, int y_coord) {
    int selection = y_coord;
    if (selection > 1000) selection -= 1000;
    selection -= 6;

    if (selection == 0) {
        WorldClimateWorldWin.show(0);
        MapWinTable[1]->show(0);
        group = 0x3EE;
    } else if (selection == 1) {
        WorldClimateWorldWin.hide();
        MapWinTable[1]->hide();
        group = 0x3EF;
    } else {
        on_iface_button_clicked(2);
        draw_hq();
        return;
    }

    MainInterfaceGlobal.button_group(1).set(group, 0);
    flatButtons_[2].set(0);
    draw_hq();
}

// ORIGINAL: 0x00440080 ?draw_hq@DiploPop@@QAEXXZ 0x00440080-0x00440291 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00440080.cpp retained original identity and spans", "recovery/tasks/interface-contract-report-closure.json authorized ownership recovery", "original:0x00440080 complete control/member accesses and concrete interface table"], "reason": "Recover the concrete SubInterface owner operation through genuine window members and source-owned globals. Original table/call evidence identifies the callback receiver; the retained compatible seed supplies its complete original intervals, not an executable-address forwarding implementation."}
void DiploPop::draw_hq() {
    Buffer *const canvas = static_cast<Buffer *>(&MainInterfaceGlobal);
    RECT area = rect1_;

    if (flatButtons_[2].selected_state() == 0) {
        canvas->set_clip(&area);
        MainInterfaceGlobal.restore_back(0);
        canvas->set_clip(MainInterfaceGlobal.buffer_working_rect());
        MainInterfaceGlobal.soft_update(&rect1_);
        return;
    }

    area.bottom -= 4;
    canvas->box_sprite(&area, &InterfaceBoxStyles[0]);

    const int left = area.left + 1;
    const int top = area.top + 1;
    const int right = area.right - 1;
    area.bottom -= 1;
    const int text_top = top + 0x15;
    area.left = left;
    area.top = top;
    area.right = right;

    int base_id = 0;
    StringTemp[0] = 0;
    while (base_id < BaseCurrentCount) {
        if (Bases[base_id].faction_id_current == counterpart_faction_
            && has_fac_built_call(FAC_HEADQUARTERS, base_id)) break;
        ++base_id;
    }

    const char *heading = base_id == BaseCurrentCount
        ? label_get(0x3A9) : Bases[base_id].name_string;
    strcat(StringTemp, heading);
    CharUpperA(StringTemp);

    canvas->set_font(&font3_, 0, 0, 0);
    canvas->set_text_color(0xA0, -1, 1, 1);
    if (StringTemp[0]) {
        canvas->write_cent_l(StringTemp, &area,
                             static_cast<int>(strlen(StringTemp)));
    }

    area.top = text_top + 1;
    const int x = (area.right - hq_sprite_->width() - area.left) / 2 + area.left;
    hq_sprite_->draw(canvas, hq_sprite_->transparent_index(), x, area.top, 1, 1);
    MainInterfaceGlobal.soft_update(&rect1_);
}


/*
// ORIGINAL: 0x0043F170 ?hide@DiploPop@@QAEXXZ 0x0043F170-0x0043F185 BYTE_EXACT
// symbol    ?hide@DiploPop@@UAEXXZ
// size      21 bytes
// prototype void (__thiscall ?hide@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EDCD0 0x005F7E90
Status: Complete
*/
void DiploPop::hide() {
    Win *self = reinterpret_cast<Win *>(this);
    if (self->is_visible()) {
        // QUALIFIED: the image emits a direct `call 0x5edcd0`, not a vtable
        // dispatch. `Win::hide` is virtual now, so an unqualified call would
        // go through slot 2 - which is what made this claim regress.
        self->Win::hide();
    }
}

// ORIGINAL: 0x0043F0F0 ?close@DiploPop@@QAEXXZ 0x0043F0F0-0x0043F137 FILE
// size      71 bytes
// prototype void (__thiscall ?close@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00616730
// indirect  0x0043F10D
// Historical source-form experiments below are retained as seed evidence.
// TRIED: 2026-09-03 homing pass, 2 spellings. MISMATCH 2/16 is the
//        `Time::stop` ceiling again - the image CALLS 0x00616730 here and
//        time.h keeps `stop` in-class MEASURED so `close`/`~Time` inline it,
//        and VC6 has no noinline. Splitting `stop` is a time.h edit.
// The complete constructor/destructor establish the typed members in
// diplopop.h. Slot 2 is ordinary virtual hide; the following close is direct.
void DiploPop::close() {
    time1_.stop();
    time2_.stop();
    hide();
    GraphicWin::close();
    counterpart_faction_ = -1;
    field_20B0_ = 0;
    field_2154_ = 1;
}

// ORIGINAL: 0x0043F140 ?show@DiploPop@@QAEXH@Z 0x0043F140-0x0043F162 BYTE_EXACT
// symbol    ?show@DiploPop@@UAEXH@Z
// LEVER: the tail call must be QUALIFIED (`win->Win::show(0)`), otherwise the
//        override dispatches through the vtable where the image has
//        `call rel32`.
// size      34 bytes
// prototype void (__thiscall ?show@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED9D0 0x005F7E90
// 0x0043F140  ?show@DiploPop@@QAEXH@Z  ->  ?show@DiploPop@@QAEXH@Z
//
// Historical seed evidence is retained alongside this product definition.


void DiploPop::show(int) {
    if (is_visible() == 0 && counterpart_faction_ != -1) {
        Win::show(0);
    }
}

// ORIGINAL: 0x0043F5C0 ?stop@DiploPop@@QAEXXZ 0x0043F5C0-0x0043F5F0 BYTE_EXACT
// size      48 bytes
// prototype void (__thiscall ?stop@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x00629DF0
// 0x0043F5C0  ?stop@DiploPop@@QAEXXZ  ->  ?stop@DiploPop@@QAEXXZ
//
// Historical seed evidence is retained alongside this product definition.


void DiploPop::stop() {
    release_iface_mode();
    flic2_.close();
    flic1_.close();
    counterpart_faction_ = -1;
}

// ORIGINAL: 0x0043F290 ?on_button_clicked@DiploPop@@QAEXH@Z 0x0043F290-0x0043F2AD FILE BYTE_EXACT
// symbol    ?on_button_clicked@DiploPop@@UAEXH@Z
//           Q->U 2026-09-06, Win's front slots 44/45 taking the image's
//           own names (on_button_clicked / on_button_toggled): the body is
//           now an override of the window's slot, so VC6 mangles it virtual
//           and enters it on the front table, while the image spells the
//           same body Q and its own table holds the raw address.
// size      29 bytes
// prototype void (__thiscall ?on_button_clicked@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0043F2A3
// Forward to the currently modal Win through its actual button-click slot.


void DiploPop::on_button_clicked(int button_id) {
    Win *target = WinModalFocus;
    if (target) {
        target->on_button_clicked(button_id);
    }
}


/*
// ORIGINAL: 0x004406D0 ?set_modal@DiploPop@@QAEHHP6AHXZPAUWin@@@Z 0x004406D0-0x004406EC FILE BYTE_EXACT
// symbol    ?set_modal@DiploPop@@UAEHHP6AHXZPAVWin@@@Z
// LEVER: the modal owner is not the caller's `this` but the fixed
// MainInterface at 0x007AE820, now named MainInterfaceGlobal (mapwin.cpp)
// instead of a carried g_ binding.
*/
int DiploPop::set_modal(int flag, int (__cdecl *poll)(), Win *owner) {
    return reinterpret_cast<Win *>(this)->Win::set_modal(
        flag | 1, poll, &MainInterfaceGlobal);
}

/*
// ORIGINAL: 0x004406F0 ?release_modal@DiploPop@@QAEXXZ 0x004406F0-0x00440707 FILE
// symbol    ?release_modal@DiploPop@@UAEXXZ
// TRIED: every spelling of a non-virtual override. The image's release_modal
//        is `QAEXX`, this tree emits `UAEXX` because win.h declares
//        `Win::release_modal` virtual at slot 58. Same wall as BaseWin::hide.
// TRIED: 2026-09-03 homing pass. The image's own bytes at 0x004406F0 are
//        `E9 8B DB 1A 00` (jmp Win::release_modal) followed by eleven 0x90
//        nops to 0x004406FF; 0x00440700 starts a DIFFERENT function
//        (`55 8B EC 6A FF`). The span on the ORIGINAL line,
//        0x004406F0-0x00440707, therefore swallows the next function's
//        prologue, and no body can reproduce 23 bytes that are only 5 bytes
//        of code plus padding. The marker line is not editable, so the claim
//        is demoted rather than mismatches forever. The body below is the
//        faithful 5-byte tail call.
*/
void DiploPop::release_modal() {
    reinterpret_cast<Win *>(this)->Win::release_modal();
}

// ORIGINAL: 0x0043F190 ?portrait_anim@DiploPop@@QAEXXZ 0x0043F190-0x0043F1BF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The original decodes the Flic at DiploPop +0xAE8 using its stored x/y at Flic +0x5AC/+0x5B0, then passes Flic +0x58C to the main interface's soft_update.", "DiploPop::flic1_ is the constructed Flic at +0xAE8, and MainInterfaceGlobal is the existing GraphicWin owner at 0x007AE820."], "reason": "Home the portrait animation using its embedded Flic, typed update rectangle, and process-owned update window."}
void DiploPop::portrait_anim() {
    flic1_.decode_stored_frame();
    MainInterfaceGlobal.soft_update(flic1_.update_rect());
}

// ORIGINAL: 0x0043F1C0 ?bioscan_anim@DiploPop@@QAEXXZ 0x0043F1C0-0x0043F1FC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The original increments DiploPop +0x20B0, decodes the Flic at +0x15CC using its stored x/y at Flic +0x5AC/+0x5B0, and updates Flic +0x58C.", "DiploPop::flic2_ is the constructed Flic at +0x15CC; its footprint ends at +0x20B0, where the existing animation counter begins."], "reason": "Home the bioscan animation through its typed Flic member, update rectangle, and owner counter."}
void DiploPop::bioscan_anim() {
    ++field_20B0_;
    flic2_.decode_stored_frame();
    MainInterfaceGlobal.soft_update(flic2_.update_rect());
}

// ORIGINAL: 0x0043F200 ?timer_callback_daemon@DiploPop@@QAAXHH@Z 0x0043F200-0x0043F245
// RECOVERY_BINDING: v1 {"evidence": ["Disassembly reads the popup pointer from the second cdecl stack argument ([ebp+0xC]); it does not use ECX as a hidden DiploPop receiver.", "The callback's Flic2 decode, animation counter increment, and update rectangle are exactly the work owned by DiploPop::bioscan_anim."], "reason": "Keep the timer entry a real two-argument cdecl callback and route its explicit popup parameter into the typed owner method."}
// symbol    ?timer_callback_daemon@@YAXHPAVDiploPop@@@Z
void __cdecl timer_callback_daemon(int, DiploPop *popup) {
    popup->bioscan_anim();
}

// ORIGINAL: 0x0043F250 ?timer_callback_daemon2@DiploPop@@QAAXHH@Z 0x0043F250-0x0043F288 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Disassembly reads the popup pointer from the second cdecl stack argument ([ebp+0xC]); this is a free callback, not a DiploPop member entry despite the historical catalog symbol spelling.", "The callback's Flic1 decode and update rectangle are the work owned by DiploPop::portrait_anim."], "reason": "Preserve the timer callback's actual cdecl stack ABI and use the typed portrait animation implementation."}
// symbol    ?timer_callback_daemon2@@YAXHPAVDiploPop@@@Z
void __cdecl timer_callback_daemon2(int, DiploPop *popup) {
    popup->portrait_anim();
}

// Original interface table 0x0066B424 slot 1 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@DiploPop@@UAEXHH@Z 0x00440380
void DiploPop::on_iface_scrolled(int, int) {}
// Original interface table 0x0066B424 slot 2 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@DiploPop@@UAEXHH@Z 0x00440380
void DiploPop::on_iface_scrolling(int, int) {}
// Original interface table 0x0066B424 slot 6 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_left_up@DiploPop@@UAEXHH@Z 0x00440380
void DiploPop::on_iface_left_up(int, int) {}

// Original interface table 0x0066B424 slot 14 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@DiploPop@@UAEHH@Z 0x00440380
int DiploPop::on_iface_button_pressed(int) {
    return 0;
}

// Original interface table 0x0066B424 slot 17 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_dialog_back_draw@DiploPop@@UAEHPAVGraphicWin@@@Z 0x00440380
int DiploPop::on_iface_dialog_back_draw(GraphicWin *) {
    return 0;
}

// Original interface table 0x0066B424 slot 18 targets this shared complete body.
// RECOVERY_SHARED: v1 0x00404240 0x00404245 ?on_iface_dialog_item_back_draw@DiploPop@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z 0x00440380
int DiploPop::on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) {
    return 0;
}
