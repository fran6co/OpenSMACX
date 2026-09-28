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
#include "basepop.h"
#include "stringstruct.h"  // StringAllocationHeap; the staged vbtable constants
#include "prodpicker.h"
#include "general.h"  // range used by veh.h's inline helpers
#include "veh.h"
#include "alpha.h"    // label_get
#include "base.h"     // BaseCurrent, BaseIDCurrentSelected
#include "map.h"      // mandate_color
#include "mapwin.h"   // shared caption scaling slots
#include "strings.h"  // StringTemp, StringTable
#include "buffer.h"   // BoxSpriteParams, InterfaceBoxStyles
#include "vector_teardown.h"
#include <cstring>

// The production preview's rotation conversion and model-size multiplier.
// The seed at 0x004927C0 uses the words at 0x0066AA2C/30 as float
// multipliers; both operands are immutable .rdata, not mutable globals.
// Exact little-endian PE evidence (file offsets 0x26AA2C and 0x26AA30):
// A7 E7 8E 3C = 0.017444444820284843f; 34 80 37 3A = 0.000699999975040555f.
static const float ProdPickerRotationRadiansPerDegree = 0.017444444820284843f;
static const float ProdPickerVehicleScaleFactor = 0.000699999975040555f;

// The production editor groups facilities into four display categories.
// ORIGINAL: 0x00493B70 ?edbc_fac@ProdPicker@@QAEHH@Z 0x00493B70-0x00493BB3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:9a9fb225:src/recovered/units/00493b70.cpp", "source:src/prodpicker.h"], "reason": "Home the category mapping in the real ProdPicker owner; this method reads no object storage and needs no fixed-image binding."}
int ProdPicker::edbc_fac(int facility_id) {
    if (facility_id >= 0x46) return 1;
    switch (facility_id) {
    case 4: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x44:
        return 3;
    case 3: case 0x1F: case 0x20:
        return 0;
    case 8: case 0xF: case 0x10: case 0x11:
        return 1;
    default:
        return 2;
    }
}

// The original 0x009AB892/+0x009AB898 reads are plan and flags members of
// the 0x34-byte VehPrototypes entries, not independent address constants.
// ORIGINAL: 0x00493AF0 ?edbc_unit@ProdPicker@@QAEHH@Z 0x00493AF0-0x00493B4A FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:9a9fb225:src/unrecovered/00493af0.cpp", "source:src/veh.h:VehPrototype"], "reason": "Recover prototype category selection through the source-owned prototype array and its named plan/flags members."}
int ProdPicker::edbc_unit(int prototype_id) {
    const VehPrototype &prototype = VehPrototypes[prototype_id];
    if (prototype_id >= 0x40 && (prototype.flags & 4) == 0) return 1;
    switch (prototype.plan) {
    case 3: case 7: case 8:
        return 0;
    case 9: case 10:
        return 2;
    case 11:
        return 1;
    default:
        return 3;
    }
}

// The constructor owns a Scroll at +0x7120, currently explicit storage to
// preserve construction order. Wheel events are delivered to that object.
// ORIGINAL: 0x00493C20 ?on_mousewheel@ProdPicker@@QAEXH@Z 0x00493C20-0x00493C68 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00493c20.cpp", "source:src/prodpicker.h", "source:src/scroll.h"], "reason": "Dispatch each wheel step to ProdPicker's real embedded Scroll rather than a raw receiver offset or vtable shim. The source form currently mismatches; retain the seed's historical exact claim as a restoration obligation, without demotion."}
void ProdPicker::on_mousewheel(int delta) {
    // x86 NEG leaves INT_MIN negative; the original's positive-count test
    // therefore sends no steps for that otherwise overflowing input.
    if (delta == (-2147483647 - 1)) return;
    while (delta < 0) {
        scroll_.on_mousewheel_down(1);
        ++delta;
    }
    while (delta > 0) {
        scroll_.on_mousewheel_up(1);
        --delta;
    }
}

// ORIGINAL: 0x004927C0 ?draw_item@ProdPicker@@QAEXHH@Z 0x004927C0-0x00492F9F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004927c0.cpp", "source:src/prodpicker.h", "source:src/stringstruct.h", "source:src/buffer.h", "source:src/sprite.h", "source:src/veh.h"], "reason": "Paint production-picker cells through typed item lists, buffer/base services, sprite tables and Caviar rendering; preserve observed wrapped icon and panel-style data views."}
// TRIED: direct translation of Ghidra's (already-clean) pseudocode into RD()-based offset access, matching the bounds check / grid position calc / set_clip+ box_sprite / three seek-list blocks / mandate_color+wrap_cent name draw / veh_fake+veh_draw+Caviar render-or-Sprite::draw_mono branch / veh_cost string; MISMATCH #0 (prologue/frame) not chased further. Typed source now measures 583 compiled instructions versus 672 original, with first divergence in stack-frame setup. The original sign-based icon dispatch conflicts with the positive IDs visible from StringList::current_id(); this source preserves the ID unchanged and leaves that producer/consumer discrepancy unresolved.
void ProdPicker::draw_item(int item, int update) {
    const int firstVisible = field_A77C_;
    if (item < firstVisible || field_A82C_ + firstVisible <= item) return;

    const int relativeItem = item - firstVisible;
    const int row = relativeItem / field_A810_;
    int left = (relativeItem % field_A810_) * field_A81C_;
    const int top = row * field_A820_;
    const int cellTop = top + 4;
    const int cellRight = field_A81C_ + left + 0x18;
    const int cellBottom = top + field_A820_ + 1;
    left += 0x1B;

    RECT cell = { left, cellTop, cellRight, cellBottom };
    set_clip(&cell);
    BoxSpriteParams *frame = &InterfaceBoxStyles[0];
    if (field_A774_ == static_cast<uint32_t>(item)) {
        // The seed passes base+20 bytes for selected entries. This is the
        // original overlapping view beginning at edge1_, not style [1].
        frame = reinterpret_cast<BoxSpriteParams *>(&InterfaceBoxStyles[0].edge1_);
    }
    box_sprite(&cell, frame);
    if (field_A818_ <= static_cast<uint32_t>(item)) return;

    ++cell.left;
    --cell.right;
    ++cell.top;
    --cell.bottom;

    StringList *lists[3] = { &prod_a_, &prod_c_, &prod_b_ };
    const int counts[3] = { prod_a_.entry_count_, prod_c_.entry_count_, prod_b_.entry_count_ };
    for (int i = 0; i < 3; ++i) {
        if (item <= counts[i] - 1) lists[i]->seek_pos(item);
    }

    int productionId = 0;
    uint32_t itemFlags = 0;
    StringStructPayload *nameEntry = prod_a_.current_entry();
    if (nameEntry) {
        productionId = prod_a_.current_id();
        itemFlags = nameEntry->field_8_;
    }
    // Preserve the current StringList ID exactly: current_id() returns the
    // stored ID unchanged. The original draw code tests its sign to choose an
    // icon family, while calculate's visible facility path appears to add
    // positive IDs; that producer/consumer discrepancy remains unresolved.
    const int displayId = productionId;
    StringStructPayload *name = prod_a_.current_entry();
    StringStructPayload *cost = prod_c_.current_entry();
    char *nameText = name ? name->text : 0;
    char *costText = cost ? cost->text : 0;

    StringTemp[0] = 0;
    const int textColor = mandate_color(itemFlags & 0xF);
    StringTemp[0] = 0;
    set_text_color(textColor, -1, 1, 1);
    set_font(&font1_, 0, 0, 0);
    const int nameLineHeight = font1_.line_advance();
    RECT nameArea = { cell.left, cell.top + 5, cell.right,
                      cell.top + nameLineHeight * 3 + 5 };
    wrap_cent(nameText, &nameArea);

    font3_.unk_1_ = 3;
    set_text_color(0xA1, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);
    const int textLeft = cell.left + 2;
    const int textRight = cell.right - 2;
    int costBottom = cell.bottom;
    const int costHeight = wrap_height(costText, textRight - textLeft);
    int costTop = costBottom - costHeight;
    RECT costArea = { textLeft, costTop, textRight, costBottom };
    wrap_cent(costText, &costArea);

    const int costLineHeight = font3_.line_advance();
    int turnsTop = costTop - costLineHeight;
    if (prod_b_.head_) {
        StringStructPayload *turns = prod_b_.current_entry();
        if (turns && turns->text) {
            RECT turnsArea = { textLeft, turnsTop, textRight, costBottom };
            write_cent_l(turns->text, &turnsArea, static_cast<int>(strlen(turns->text)));
        }
    }

    int hasPrototypeCost = 0;
    if (displayId < 0) {
        const int iconId = -displayId;
        Sprite *icon = iconId < 0x46
            ? &g_FACILITY_ICON_SPRITES[iconId]
            : &g_SECRET_PROJECT_ICON_SPRITES[iconId - 0x46];
        const int iconSize = (field_A820_ * 3) / 7;
        const int iconX = (cell.bottom - cell.left) / 2 - iconSize / 2;
        const int iconY = (cell.right - cell.left) / 2 - iconSize / 2;
        icon->draw_mono(this, icon->transparent_index(), iconY + cell.left,
                        cell.left + iconX, textColor, iconSize, icon->width());
    } else {
        const int iconSize = (field_A820_ << 1) / 3;
        const int modelSize = (iconSize * 0x4B) / 100;
        const int fakeId = veh_fake(displayId, BaseCurrent->faction_id_current);
        const int rotation = field_A774_ == static_cast<uint32_t>(item)
            ? static_cast<int>(field_B64_) : 0;
        MapWinCaptionScale = iconSize;
        MapWinCaptionDivisor = 100;
        const int rendered = veh_draw(this, fakeId,
            (cell.bottom - cell.left) / 2 - iconSize / 2 + cell.left,
            cell.left + ((cell.right - cell.left) / 2 - modelSize / 2),
            1, 0xCC, 0);
        if (rendered) {
            caviar_.reset_objects();
            vehdraw_construct_vehicle(fakeId, &caviar_);
            caviar_.set_scaling(static_cast<float>(iconSize) * ProdPickerVehicleScaleFactor);
            caviar_.set_scene_rotation(static_cast<float>(rotation + 0x1E)
                * ProdPickerRotationRadiansPerDegree, 0, 0);
            caviar_.render(this, (cell.bottom - cell.left) / 2 + cell.left,
                           cell.left + (cell.right - cell.left) / 2, 0);
        }
        MapWinCaptionScale = -1;
        MapWinCaptionDivisor = -1;
        veh_cost(displayId, BaseIDCurrentSelected, &hasPrototypeCost);
        if (hasPrototypeCost) {
            set_font(&font3_, 0, 0, 0);
            set_text_color(0xFA, -1, 1, 1);
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(185));
            strcat(StringTemp, " +");
            char number[64];
            _itoa(prototype_factor(displayId), number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, "%");
            write_cent_l(StringTemp, &cell, static_cast<int>(strlen(StringTemp)));
        }
    }

    if (field_A774_ == static_cast<uint32_t>(item)) box(&cell, 0xFF, 0xFF);
    set_clip(&rect2_);
    if (update) {
        RECT updateArea = { left, cellTop, cellRight, cellBottom };
        soft_update(&updateArea);
    }
}

// ORIGINAL: 0x004941C0 ?on_left_click@ProdPicker@@QAEXHH@Z 0x004941C0-0x0049422F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004941c0.cpp", "source:src/prodpicker.h", "source:src/prodpicker.cpp"], "reason": "Apply the click-to-grid selection through the picker’s measured geometry and redraw both changed cells through draw_item and GraphicWin."}
void ProdPicker::on_left_click(int x, int y) {
    const int row = y / field_A820_;
    const uint32_t oldSelection = field_A774_;
    const int column = x / field_A81C_;
    const uint32_t newSelection = field_A77C_ + row * field_A810_ + column;
    field_A774_ = newSelection;
    if (oldSelection != newSelection) {
        field_B64_ = 0;
        draw_item(static_cast<int>(oldSelection), 0);
        draw_item(static_cast<int>(field_A774_), 0);
        soft_update();
    }
}

// ORIGINAL: 0x00493C70 ?on_redraw@ProdPicker@@QAEXXZ 0x00493C70-0x00493CA6
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/00493c70.cpp"], "reason": "Home the redraw callback in ProdPicker; it repaints exactly the currently visible item interval through the typed draw_item operation."}
// symbol    ?on_redraw@ProdPicker@@UAEXXZ
void ProdPicker::on_redraw() {
    const int first = static_cast<int>(field_A77C_);
    const int end = first + static_cast<int>(field_A82C_);
    for (int item = first; item < end; ++item) {
        draw_item(item, 0);
    }
}

// ORIGINAL: 0x00494230 ?on_left_double_click@ProdPicker@@QAEXHH@Z 0x00494230-0x00494271 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:20b88b523285a816ba99f503d727b761b89b227f:src/recovered/units/00494230.cpp", "source:src/win.h:Win::vslot_19", "source:src/prodpicker.h:ProdPicker::on_button_clicked"], "reason": "Home the measured left double click event in ProdPicker and forward its notification and button selection through inherited virtual behavior."}
// symbol    ?on_left_double_click@ProdPicker@@UAEXHH@Z
void ProdPicker::on_left_double_click(int x, int y) {
    vslot_19(x, y);
    if (field_B68_ == 0) {
        on_button_clicked(-1);
    } else {
        on_button_clicked(5);
    }
}

// ORIGINAL: 0x004945D0 ?on_button_clicked@ProdPicker@@QAEXH@Z 0x004945D0-0x004946A1
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004945d0.cpp"], "reason": "Recover product-selection button routing against ProdPicker selection and action members, then release modality and hide when the cancel action closes a modal picker."}
// symbol    ?on_button_clicked@ProdPicker@@UAEXH@Z
void ProdPicker::on_button_clicked(int button_id) {
    switch (button_id) {
    case 0:
        if (field_A774_ == static_cast<uint32_t>(-1)) return;
        field_A778_ = 1;
        break;
    case 1:
        field_A778_ = 2;
        break;
    case 2:
        if (field_A774_ == static_cast<uint32_t>(-1)) return;
        // fall through
    case 7:
        field_A778_ = 7;
        break;
    case 3:
        field_A778_ = 3;
        break;
    case 4:
        field_A778_ = 4;
        break;
    case 5:
        if (field_A774_ == static_cast<uint32_t>(-1)) return;
        field_A778_ = 5;
        break;
    case 6:
        field_A778_ = 6;
        break;
    case -2:
        field_A778_ = 0;
        field_A774_ = static_cast<uint32_t>(-1);
        break;
    case -1:
        field_A778_ = 0;
        break;
    default:
        return;
    }
    release_modal();
    if (field_B68_ != 0 && (button_id == -1 || button_id == -2)) {
        hide();
    }
}

// ORIGINAL: 0x00493CB0 ?on_scrolled@ProdPicker@@QAEXHH@Z 0x00493CB0-0x00493E6F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00493cb0.cpp", "source:src/prodpicker.h", "source:src/buffer.h", "source:src/win.h"], "reason": "Scroll the displayed item window using measured picker geometry, its real Buffer blit operation, draw_item, and inherited redraw virtual."}
void ProdPicker::on_scrolled(int, int position) {
    const int pageSize = static_cast<int>(field_A810_);
    const int oldPosition = static_cast<int>(field_A77C_);
    const int newPosition = pageSize * position;
    field_A77C_ = static_cast<uint32_t>(newPosition);

    int distance = newPosition - oldPosition;
    if (distance < 0) distance = -distance;
    if (distance > pageSize) {
        redraw();
        soft_update();
        return;
    }

    if (newPosition < oldPosition) {
        set_clip(&rect2_);
        copy(this, 0x15, 4, 0x15, static_cast<int>(field_A820_) + 4,
             static_cast<int>(field_A81C_) * pageSize + 4,
             (static_cast<int>(field_A814_) - 1) * static_cast<int>(field_A820_)
                 + 4 + static_cast<int>(field_A820_)
                 - (static_cast<int>(field_A820_) + 4));

        int first = static_cast<int>(field_A77C_);
        const int end = static_cast<int>(field_A810_ + field_A77C_);
        while (first < end) draw_item(first++, 0);
    } else if (newPosition != oldPosition) {
        set_clip(&rect2_);
        copy(this, 0x15, static_cast<int>(field_A820_) + 4, 0x15, 4,
             static_cast<int>(field_A81C_) * pageSize + 4,
             (static_cast<int>(field_A814_) - 1) * static_cast<int>(field_A820_));

        const int first = (static_cast<int>(field_A814_) - 1) * pageSize
            + static_cast<int>(field_A77C_);
        const int end = first + static_cast<int>(field_A810_);
        for (int item = first; item < end; ++item) draw_item(item, 0);
    }
    soft_update();
}

// The three 0x30-byte blocks at 0xA780/0xA7B0/0xA7E0 - read once as unnamed
// "generic sub-objects" staged through a computed adjustor - are StringLists
// (see prodpicker.h): prod_picker_unknown_a7_block's store sequence is
// StringList's compiler-emitted construction, stage for stage. The helper
// and basepop.cpp's hand copies of the tables (BasePopVbtable,
// BasePopStageTables6693A0/6698C0) are gone; prod_a_/prod_b_/prod_c_ build
// from the declaration.

/*
// ORIGINAL: 0x00492420 ??0ProdPicker@@QAE@XZ 0x00492420-0x004926AD;0x00658B70-0x00658C09
// TRIED: register allocation - compiled body agrees on the SEH prologue
//            (7/7) then diverges at the push ebx/esi/edi save order; VC6
//            chose a different callee-save order for this body's register
//            pressure. MISMATCH, 11/127 instructions agree. The three
//            0x30-byte sub-objects at 0xA780/0xA7B0/0xA7E0 were reproduced
//            at their raw offset (prod_picker_unknown_a7_block) until the
//            StringList identification; the per-block instruction
//            SCHEDULING (which register holds the vtable pointer, when it
//            gets reloaded) varies between the three in the image in a way
//            plain repeated C++ does not reproduce.
// TRIED: real declared members (Sprite[3]/Font x4/Time/FlatButton[9]/
//            Scroll/Caviar) built implicitly instead of placement-new
//            measured WORSE, 8/127 against the raw-storage baseline's 11/127.
//            That byte-only experiment is superseded: the class now has a
//            verified same-size typed layout and real lifetime, so the lower
//            constructor score is retained as a restoration obligation rather
//            than a reason to keep explicit placement-new.
// size      806 bytes
// prototype void (__thiscall ??0ProdPicker@@QAE@XZ)(ProdPicker* this)
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x006051D0 0x006161D0 0x00616DA0 0x00618EA0 0x006456E4 0x006457C2
*/
// C++ constructs ConstructedGraphicWin/GraphicWin before these typed members;
// declaration order follows the image's subobject order. Their destructors
// similarly own teardown after this destructor body.
ProdPicker::ProdPicker() {
    field_A830_ = 0x86;
    field_A834_ = 0x86;
    field_A14_ = 0;
    field_A838_ = 0x6C;
    field_A83C_ = 0x5E;
}

// ORIGINAL: 0x00421100 ??1ProdPicker@@QAE@XZ 0x00421100-0x00421392;0x00651250-0x00651376
// RECOVERY_BINDING: v1 {"evidence": ["git:1b3ed0614446d5283fe4cbd2392cbb64b49db601:src/recovered/units/00421100.cpp", "source:src/prodpicker.h"], "reason": "Home the ProdPicker destructor in its real owning TU; typed embedded members own their cleanup after close(), replacing the external hand-offset teardown forwarder."}
ProdPicker::~ProdPicker() {
    close();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x00493C10 ?UNK1@ProdPicker@@QAEHH@Z 0x00493C10-0x00493C18 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK1@ProdPicker@@QAEHH@Z)(ProdPicker* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int ProdPicker::UNK1(int) {
    return 1;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00493E90 ?on_mouse_move@ProdPicker@@QAEXHH@Z 0x00493E90-0x00493E93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@ProdPicker@@QAEXHH@Z)(ProdPicker* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::on_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00495160 ?UNK3@ProdPicker@@QAEXH@Z 0x00495160-0x00495163 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK3@ProdPicker@@QAEXH@Z)(ProdPicker* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::UNK3(int) {
}


/*
Purpose: Close the picker - clear the field at 0xA14 that marks it live, then
         run the GraphicWin close it inherits.
// ORIGINAL: 0x004926B0 ?close@ProdPicker@@QAEXXZ 0x004926B0-0x004926BF BYTE_EXACT
// size      15 bytes
// prototype void (__thiscall ?close@ProdPicker@@QAEXXZ)(ProdPicker* this)
// callers   2   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::close() {
    field_A14_ = 0;
    GraphicWin::close();
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004945C0 ?on_redraw_nc@ProdPicker@@QAEXPAURECT@@H@Z 0x004945C0-0x004945C3 BYTE_EXACT
// symbol    ?on_redraw_nc@ProdPicker@@UAEXPAUtagRECT@@H@Z
// size      3 bytes
// prototype void (__thiscall ?on_redraw_nc@ProdPicker@@QAEXPAURECT@@H@Z)(ProdPicker* this, RECT*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::on_redraw_nc(RECT *, int) {
}


/*
Purpose: Step the receiver back to the subobject ??_GProdPicker@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421970 ??_GProdPicker@@WEEE@AEPAXI@Z 0x00421970-0x0042197B BYTE_EXACT
// symbol    ??_EProdPicker@@WEEE@AEPAXI@Z
// CORRECTED from ??3ProdPicker@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004213A0` into
//   ??_GProdPicker@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
