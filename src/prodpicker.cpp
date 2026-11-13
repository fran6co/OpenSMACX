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
// WORKAROUND(decl): calculate calls StringStruct::remove_all (0x00402970)
// out of line for all three lists (0x00492FC3..0x00492FE9).
#define STRINGSTRUCT_REMOVE_ALL_OUT_OF_LINE
#include "font_metrics.h"
#include "interface_fonts.h"
#include "basepop.h"
#include "stringstruct.h"  // StringAllocationHeap; the staged vbtable constants
#include "prodpicker.h"
#include "rectangle_operations.h"
#include "general.h"  // range used by veh.h's inline helpers
#include "veh.h"
#include "alpha.h"    // label_get
#include "base.h"     // BaseCurrent, BaseIDCurrentSelected
#include "basewin.h"  // the production window's owning Buffer geometry
#include "tutwin.h"   // TutWinBaseWindow owns that real BaseWin subobject
#include "map.h"      // mandate_color
#include "mapwin.h"   // shared caption scaling slots
#include "console.h"  // ConsoleGlobal owns the original modal parent window
#include "game.h"     // GameState, multiplayer state
#include "technology.h" // has_tech
#include "pulldown.h"
#include "netdaemon.h" // synch_proto
#include "strings.h"  // StringTemp, StringTable
#include "buffer.h"   // BoxSpriteParams, InterfaceBoxStyles
#include <cstring>

void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);
void __cdecl scroll_show(GraphicWin *window, Scroll *scroll);
void __cdecl scroll_hide(GraphicWin *window, Scroll *scroll);

// The production preview's rotation conversion and model-size multiplier.
// The seed at 0x004927C0 uses the words at 0x0066AA2C/30 as float
// multipliers; both operands are immutable .rdata, not mutable globals.
// Exact little-endian PE evidence (file offsets 0x26AA2C and 0x26AA30):
// A7 E7 8E 3C = 0.017444444820284843f; 34 80 37 3A = 0.000699999975040555f.
static const float ProdPickerRotationRadiansPerDegree = 0.017444444820284843f;
static const float ProdPickerVehicleScaleFactor = 0.000699999975040555f;

// The production editor groups facilities into four display categories.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00493B70 0x00493BB4 0x00493BC4 0x00493B94
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00493B70 0x00493BC4 0x00493C06 0x00493B8E
// ORIGINAL: 0x00493B70 ?edbc_fac@ProdPicker@@QAEHH@Z 0x00493B70-0x00493BB3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2a0f5dd:src/recovered/units/00493b70.cpp", "source:src/prodpicker.h"], "reason": "Home the category mapping in the real ProdPicker owner; this method reads no object storage and needs no fixed-image binding."}
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
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00493AF0 0x00493B4C 0x00493B70 0x00493B2B
// The original 0x009AB892/+0x009AB898 reads are plan and flags members of
// the 0x34-byte VehPrototypes entries, not independent address constants.
// ORIGINAL: 0x00493AF0 ?edbc_unit@ProdPicker@@QAEHH@Z 0x00493AF0-0x00493B4A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f2a0f5dd:src/unrecovered/00493af0.cpp", "source:src/veh.h:VehPrototype"], "reason": "Recover prototype category selection through the source-owned prototype array and its named plan/flags members."}
int ProdPicker::edbc_unit(int prototype_id) {
    if (prototype_id >= 0x40 && (VehPrototypes[prototype_id].flags & 4) == 0)
        return 1;
    switch (VehPrototypes[prototype_id].plan) {
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
// ORIGINAL: 0x00493C20 ?on_mousewheel@ProdPicker@@QAEXH@Z 0x00493C20-0x00493C68 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/00493c20.cpp", "source:src/prodpicker.h", "source:src/scroll.h"], "reason": "Dispatch each wheel step through the real embedded Scroll. Preserve the original wrapped negative count and its nonpositive INT_MIN no-step path; retain the original identity, spans, historical claim and virtual member ABI."}
// symbol    ?on_mousewheel@ProdPicker@@UAEXH@Z
// Current virtual override emits UAE; retain the catalogued QAE identity above.
// kind      game
void ProdPicker::on_mousewheel(int delta) {
    if (delta < 0) {
        const int count = static_cast<int>(
            0U - static_cast<unsigned int>(delta));
        if (count > 0) {
            Scroll *const scroll = &scroll_;
            int remaining = count;
            do {
                scroll->on_mousewheel_down(1);
            } while (--remaining != 0);
        }
    } else if (delta > 0) {
        Scroll *const scroll = &scroll_;
        int remaining = delta;
        do {
            scroll->on_mousewheel_up(1);
        } while (--remaining != 0);
    }
}

// WORKAROUND(inline): StringStruct::current_id (0x00401640) expanded inline;
// the image reloads head_ for the id read (0x00493F88, and both arms of
// exec's signed result at 0x00495100).
static inline int prodpicker_current_id(StringStruct *list) {
    if (list->head_) {
        return list->current_->id;
    }
    return 0;
}

// ORIGINAL: 0x004927C0 ?draw_item@ProdPicker@@QAEXHH@Z 0x004927C0-0x00492F9F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004927c0.cpp", "source:src/prodpicker.h", "source:src/stringstruct.h", "source:src/buffer.h", "source:src/sprite.h", "source:src/veh.h"], "reason": "Paint production-picker cells through typed item lists, buffer/base services, sprite tables and Caviar rendering; preserve observed wrapped icon and panel-style data views."}
// TRIED: direct translation of Ghidra's (already-clean) pseudocode into RD()-based offset access, matching the bounds check / grid position calc / set_clip+ box_sprite / three seek-list blocks / mandate_color+wrap_cent name draw / veh_fake+veh_draw+Caviar render-or-Sprite::draw_mono branch / veh_cost string; MISMATCH #0 (prologue/frame) not chased further. Typed source now measures 583 compiled instructions versus 672 original, with first divergence in stack-frame setup. The original sign-based icon dispatch conflicts with the positive IDs visible from StringList::current_id(); this source preserves the ID unchanged and leaves that producer/consumer discrepancy unresolved.
void ProdPicker::draw_item(int item, int update) {
    if (item < field_A77C_ || item >= field_A77C_ + field_A82C_) return;

    const int relative = item - field_A77C_;
    const int y = relative / column_count_ * row_height_;
    const int x = relative % column_count_ * cell_width_ + 0x17;
    RECT cell;
    set_rectangle_dimensions(&cell, x, y, cell_width_, row_height_);
    offset_rect(&cell, 4, 4);
    cell.right -= 3;
    cell.bottom -= 3;
    RECT update_area = cell;

    set_clip(&cell);
    box_sprite(&cell, &InterfaceBoxStyles[field_A774_ == item ? 2 : 0]);
    if (item >= field_A818_) return;

    cell.left++;
    cell.right--;
    cell.top++;
    cell.bottom--;

    prod_a_.seek_pos(item);
    prod_c_.seek_pos(item);
    prod_b_.seek_pos(item);

    int id = prodpicker_current_id(&prod_a_);
    const uint32_t flags = prod_a_.head_
        ? prod_a_.current_->string_payload()->field_8_ : 0;
    if (!(flags & 0x20)) id = -id;
    StringTemp[0] = 0;
    int color = mandate_color(flags & 0xF);
    StringTemp[1] = 0;
    set_text_color(color, -1, 1, 1);
    set_font(&font1_, 0, 0, 0);

    RECT area = cell;
    area.top = cell.top + 5;
    area.bottom = cell.top + font_line_advance(font1_) * 3 + 5;
    wrap_cent(prod_a_.head_ ? prod_a_.current_->string_payload()->text : 0, &area);

    font3_.unk_1_ = 3;
    set_text_color(0xA1, -1, 1, 1);
    set_font(&font3_, 0, 0, 0);
    area = cell;
    area.left += 2;
    area.right -= 2;
    area.top = area.bottom - wrap_height(
        prod_c_.head_ ? prod_c_.current_->string_payload()->text : 0,
        area.right - area.left);
    wrap_cent(prod_c_.head_ ? prod_c_.current_->string_payload()->text : 0, &area);
    area.bottom = area.top;
    area.top -= font_line_advance(font3_);
    if (prod_b_.head_) {
        write_cent_l(prod_b_.current_entry()->text, &area);
    }

    if (id >= 0) {
        const int size = row_height_ * 2 / 3;
        const int model = size * 75 / 100;
        const int veh_id = veh_fake(id, BaseCurrent->faction_id_current);
        if (field_A774_ == item) {
            color = field_B64_;
        } else {
            color = 0;
        }
        MapWinCaptionScale = size;
        MapWinCaptionDivisor = 100;
        if (veh_draw(this, veh_id,
                     (cell.right - cell.left) / 2 - size / 2 + cell.left,
                     (cell.bottom - cell.top) / 2 - model / 2 + cell.top,
                     1, 0xCC, 0)) {
            caviar_.reset_objects();
            vehdraw_construct_vehicle(veh_id, &caviar_);
            caviar_.set_scaling(size * ProdPickerVehicleScaleFactor);
            caviar_.set_scene_rotation((color + 0x1E) * ProdPickerRotationRadiansPerDegree, 0, 0);
            caviar_.render(this, (cell.right - cell.left) / 2 + cell.left,
                           (cell.bottom - cell.top) / 2 + cell.top, 0);
        }
        MapWinCaptionScale = -1;
        MapWinCaptionDivisor = -1;
        BOOL has_cost;
        veh_cost(id, BaseIDCurrentSelected, &has_cost);
        if (has_cost) {
            set_font(&font3_, 0, 0, 0);
            set_text_color(0xFA, -1, 1, 1);
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(185));
            strcat(StringTemp, " +");
            const int factor = prototype_factor(id);
            char number[80];
            _itoa(factor, number, 10);
            strcat(StringTemp, number);
            strcat(StringTemp, "%");
            write_cent_l(StringTemp, &cell);
        }
    } else {
        id = -id;
        const int size = row_height_ * 3 / 7;
        if (id < 70) {
            g_FACILITY_ICON_SPRITES[id].draw_mono(this,
                g_FACILITY_ICON_SPRITES[id].transparent_index(),
                (cell.right - cell.left) / 2 - size / 2 + cell.left,
                (cell.bottom - cell.top) / 2 - size / 2 + cell.top,
                color, size, g_FACILITY_ICON_SPRITES[0].width());
        } else {
            g_SECRET_PROJECT_ICON_SPRITES[id - 70].draw_mono(this,
                g_SECRET_PROJECT_ICON_SPRITES[id - 70].transparent_index(),
                (cell.right - cell.left) / 2 - size / 2 + cell.left,
                (cell.bottom - cell.top) / 2 - size / 2 + cell.top,
                color, size, g_SECRET_PROJECT_ICON_SPRITES[0].width());
        }
    }

    if (field_A774_ == item) box(&cell, 0xFF, 0xFF);
    set_clip(&rect2_);
    if (update) soft_update(&update_area);
}

// ORIGINAL: 0x004941C0 ?on_left_click@ProdPicker@@QAEXHH@Z 0x004941C0-0x0049422F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004941c0.cpp", "source:src/prodpicker.h", "source:src/prodpicker.cpp"], "reason": "Apply the click-to-grid selection through the picker’s measured geometry and redraw both changed cells through draw_item and GraphicWin."}
// symbol    ?on_left_click@ProdPicker@@UAEXHH@Z
void ProdPicker::on_left_click(int x, int y) {
    const uint32_t oldSelection = field_A774_;
    field_A774_ = field_A77C_ + (y / row_height_ * column_count_ + x / cell_width_);
    if (oldSelection != field_A774_) {
        field_B64_ = 0;
        draw_item(static_cast<int>(oldSelection), 0);
        draw_item(static_cast<int>(field_A774_), 0);
        soft_update();
    }
}

// ORIGINAL: 0x00493C70 ?on_redraw@ProdPicker@@QAEXXZ 0x00493C70-0x00493CA6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/00493c70.cpp"], "reason": "Home the redraw callback in ProdPicker; it repaints exactly the currently visible item interval through the typed draw_item operation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_redraw@ProdPicker@@UAEXXZ
void ProdPicker::on_redraw() {
    int item = static_cast<int>(field_A77C_);
    for (; item < field_A77C_ + field_A82C_; ++item) {
        draw_item(item, 0);
    }
}

// ORIGINAL: 0x00494230 ?on_left_double_click@ProdPicker@@QAEXHH@Z 0x00494230-0x00494271 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f30e8c53a1277c18e33f96dcc5c056bdb9a890e6:src/recovered/units/00494230.cpp", "source:src/win.h:Win::on_left_click (candidate spelling for the inherited Win slot-19 callback; fresh emitted-symbol evidence is pending)", "source:src/prodpicker.h:ProdPicker::on_button_clicked"], "reason": "Home the measured left double click event in ProdPicker and forward its notification and button selection through inherited virtual behavior."}
// symbol    ?on_left_double_click@ProdPicker@@UAEXHH@Z
void ProdPicker::on_left_double_click(int x, int y) {
    on_left_click(x, y);
    if (field_B68_ == 0) {
        on_button_clicked(-1);
    } else {
        on_button_clicked(5);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: `switch(a1)` with case 2 falling through to case 7 reproduces the jump-table dispatch; MISMATCH remains in the prologue (push esi/edi ordering around the range check), not the switch body itself.
// ORIGINAL: 0x004945D0 ?on_button_clicked@ProdPicker@@QAEXH@Z 0x004945D0-0x004946A1
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004945d0.cpp"], "reason": "Recover product-selection button routing against ProdPicker selection and action members, then release modality and hide when the cancel action closes a modal picker."}
// symbol    ?on_button_clicked@ProdPicker@@UAEXH@Z
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004945D0 0x004946A4 0x004946CC 0x004945E5
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00493cb0.cpp", "source:src/prodpicker.h", "source:src/buffer.h", "source:src/win.h", "pinned original own table 0x0066A250, pointer word at +0xC0, identifies original entry 0x00493CB0", "source:src/win.h inherited virtual interface; actual emitted method ?vslot_48@ProdPicker@@UAEXHH@Z in primary Win table ??_7ProdPicker@@6BWin@@@"], "reason": "Scroll the displayed item window using measured picker geometry, its real Buffer blit operation, draw_item, and inherited redraw virtual. The original own Win table binds this entry as virtual slot 48. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence. The previous on_scrolled definition is the class slot-48 target; renaming connects the unchanged body to its real virtual slot."}
// symbol    ?on_scrolled@ProdPicker@@UAEXHH@Z
void ProdPicker::on_scrolled(int, int position) {
    RECT rect;
    const int pageSize = column_count_;
    const int oldPosition = static_cast<int>(field_A77C_);
    // Native IMUL/SUB/JNS/reverse-SUB keeps the low word. In particular,
    // an INT_MIN distance remains negative rather than becoming positive.
    const uint32_t newPositionBits = static_cast<uint32_t>(pageSize)
        * static_cast<uint32_t>(position);
    const int newPosition = static_cast<int32_t>(newPositionBits);
    field_A77C_ = newPosition;

    int distance = static_cast<int32_t>(newPositionBits
        - static_cast<uint32_t>(oldPosition));
    if (distance < 0) {
        distance = static_cast<int32_t>(static_cast<uint32_t>(oldPosition)
            - newPositionBits);
    }
    if (distance > pageSize) {
        redraw();
        soft_update();
        return;
    }

    if (newPosition < oldPosition) {
        restore_clip();
        set_rectangle_dimensions(&rect, 0x15, 4,
                                 cell_width_ * column_count_ + 2,
                                 (static_cast<int>(field_A814_) - 1) * row_height_);
        rect.right += 2;
        rect.bottom += row_height_;
        copy(this, 0x15, 4, rect.left, rect.top + row_height_,
             rect.right - rect.left, rect.bottom - (rect.top + row_height_));

        for (int item = field_A77C_; item < column_count_ + field_A77C_;
             ++item) {
            draw_item(item, 0);
        }
    } else if (newPosition != oldPosition) {
        restore_clip();
        set_rectangle_dimensions(&rect, 0x15, 4,
                                 cell_width_ * column_count_ + 2,
                                 (static_cast<int>(field_A814_) - 1) * row_height_);
        rect.right += 2;
        copy(this, rect.left, rect.top + row_height_, 0x15, 4,
             rect.right - rect.left, rect.bottom - rect.top);

        const int first = (static_cast<int>(field_A814_) - 1) * column_count_
            + field_A77C_;
        for (int item = first; item < first + column_count_; ++item) {
            draw_item(item, 0);
        }
    }
    soft_update();
}

// The legacy keyboard handler is a direct ProdPicker operation (not a Win
// virtual override). Its second argument is the key code; the first is unused.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: a straight port of the jump-table switch (12 case labels, two cross-case gotos folded away algebraically once traced by hand) - matches semantics and field offsets (confirmed against param_1[N] * 4 in the Ghidra dump) but the compiler's jump table shape/ordering diverges early; not chased further.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00494280 0x00494548 0x0049456C 0x004942AF
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00494280 0x0049456C 0x004945B4 0x004942A9
// ORIGINAL: 0x00494280 ?on_key_click@ProdPicker@@QAEHHH@Z 0x00494280-0x00494545 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00494280.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// symbol    ?on_key_click@ProdPicker@@UAEHHH@Z
int ProdPicker::on_key_click(int repeat, int key) {
    (void)repeat;
    const int oldFirst = field_A77C_;
    const int oldSelection = field_A774_;

    switch (key) {
    case 0x26: // Up; keypad 8
    case 0x68:
    {
        const int step = column_count_;
        if (oldSelection < step) return 1;
        int current;
        do {
            current = field_A774_ - step;
            field_A774_ = current;
        } while (current >= field_A82C_ + oldFirst);
        if (current < oldFirst) {
            field_A77C_ = oldFirst - step;
            scroll_.set_pos(scroll_.position() - 1);
        }
        break;
    }
    case 0x28: // Down; keypad 2
    case 0x62:
    {
        const int step = column_count_;
        if (oldSelection > field_A818_ - step - 1) {
            if (field_A818_ <= field_A82C_) return 1;
            scroll_.set_pos(scroll_.position() + 1);
            field_A77C_ = scroll_.position() * column_count_;
        } else {
            do {
                field_A774_ += step;
            } while (field_A774_ < oldFirst);
            const int current = field_A774_;
            if (current >= field_A82C_ + oldFirst) {
                field_A77C_ = step + oldFirst;
                scroll_.set_pos(scroll_.position() + 1);
            }
        }
        break;
    }
    case 0x25: // Left; keypad 4
    case 100:
    {
        const int first = (oldSelection / column_count_) * column_count_;
        int last = first + column_count_ - 1;
        if (last >= field_A818_ - 1) last = field_A818_ - 1;
        field_A774_ = oldSelection - 1;
        field_A774_ = range(field_A774_, first, last);
        break;
    }
    case 0x27: // Right; keypad 6
    case 0x66:
    {
        const int first = (oldSelection / column_count_) * column_count_;
        int last = first + column_count_ - 1;
        if (last >= field_A818_ - 1) last = field_A818_ - 1;
        field_A774_ = oldSelection + 1;
        field_A774_ = range(field_A774_, first, last);
        break;
    }
    case 0x23: // End
        field_A774_ = field_A818_ - 1;
        if (field_A82C_ < field_A818_)
            field_A77C_ = (field_A774_ / column_count_ + 1)
                * column_count_ - field_A82C_;
        scroll_.set_pos(field_A77C_ / column_count_);
        break;
    case 0x24: // Home
        scroll_.set_pos(0);
        field_A77C_ = 0;
        field_A774_ = 0;
        break;
    case 0x21: // Page Up
        if (field_A818_ <= field_A82C_) return 1;
        scroll_.set_pos(scroll_.position() - scroll_.paging());
        field_A77C_ = column_count_ * scroll_.position();
        break;
    case 0x22: // Page Down
        if (field_A818_ <= field_A82C_) return 1;
        scroll_.set_pos(scroll_.paging() + scroll_.position());
        field_A77C_ = column_count_ * scroll_.position();
        break;
    }

    if (oldFirst != field_A77C_) {
        field_B64_ = 0;
        redraw();
    } else if (oldSelection != field_A774_) {
        field_B64_ = 0;
        draw_item(oldSelection, 0);
        draw_item(field_A774_, 0);
    }
    soft_update();
    return 1;
}

// This helper's second parameter is the target picker. The receiver is not
// used by the original operation; the supplied picker owns both mutations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004946D0 ?UNK2@ProdPicker@@QAAXHPAUProdPicker@@@Z 0x004946D0-0x004946F7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004946d0.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// symbol    ?UNK2@ProdPicker@@SAXHPAV1@@Z
void __cdecl ProdPicker::UNK2(int, ProdPicker *picker) {
    if (picker == 0) return;
    ++picker->field_B64_;
    picker->draw_item(picker->field_A774_, 1);
}

// The inline edbc_unit/edbc_fac switches keep their tables after the body.
// TMPASSOC switch_targets 0x00492FA0 0x00493A70 0x00493A94 0x004934B8
// TMPASSOC switch_targets 0x00492FA0 0x00493A94 0x00493AA4 0x0049389C
// TMPASSOC switch_selector 0x00492FA0 0x00493AA4 0x00493AE6 0x00493896
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00492FA0 0x00493A70 0x00493A94 0x004934B8
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00492FA0 0x00493A94 0x00493AA4 0x0049389C
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00492FA0 0x00493AA4 0x00493AE6 0x00493896
// ORIGINAL: 0x00492FA0 ?calculate@ProdPicker@@QAEXXZ 0x00492FA0-0x00493A6D FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00492fa0.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// TRIED: void return confirmed (0,1,2,3 are switch-derived flag bits; 9999/999999 are itoa() display clamps, not status codes - the single `ret` has no `mov eax,K` before it, eax at exit is whatever Font::init() last returned). Structural transcription via named ProdPicker::field_XXXX_ members instead of raw pointer casts didn't close the divergence; prologue shape differs (#0 push vs sub) from local layout, not a wrong op.
void ProdPicker::calculate() {
    int found = 0;
    const int base_id = BaseIDCurrentSelected;
    int has_proto_cost = 0;
    char name[256];
    char stats[256];
    char effect[256];
    char active[80];
    char queued[80];

    {
        StringList *list = &prod_a_;
        list->StringStruct::remove_all();
        list->current_position_ = 0;
        list->allocator_ = 0;
    }
    {
        StringList *list = &prod_b_;
        list->StringStruct::remove_all();
        list->current_position_ = 0;
        list->allocator_ = 0;
    }
    {
        StringList *list = &prod_c_;
        list->StringStruct::remove_all();
        list->current_position_ = 0;
        list->allocator_ = 0;
    }
    field_A818_ = 0;
    field_A774_ = -1;
    field_A77C_ = 0;

    const int queued_id = BaseCurrent->queue_production_id[field_B68_];
    const int last_id = BaseCurrent->production_id_last;
    proto_sort(field_A76C_);

    // A faction with any researched prerequisite above its current weapon,
    // armor, or chassis technology receives the editor's upgrade action.
    int i;
    for (i = 0; i < MaxWeaponNum; i++) {
        if (Weapon[i].offense_rating > 1 && has_tech(Weapon[i].preq_tech, field_A76C_)) {
            found = 1;
            break;
        }
    }
    for (i = 0; !found && i < MaxArmorNum; i++) {
        if (Armor[i].defense_rating > 1 && has_tech(Armor[i].preq_tech, field_A76C_)) {
            found = 1;
        }
    }
    for (i = 0; !found && i < MaxChassisNum; i++) {
        if (Chassis[i].speed > 1 && has_tech(Chassis[i].preq_tech, field_A76C_)) {
            found = 1;
        }
    }

    for (i = 0; i < 128; i++) {
        const int id = ProtoSortIds[i];
        if (!veh_avail(id, field_A76C_, BaseIDCurrentSelected)) continue;

        int flags = 0;
        name[0] = 0;
        strcat(name, VehPrototypes[id].veh_name);
        if (GameDataStateStorage.fields_.game_state_ & 0x80) {
            strcat(name, "(");
            _itoa(PlayersData[field_A76C_].proto_id_active[id], active, 10);
            strcat(name, active);
            strcat(name, " ");
            _itoa(PlayersData[field_A76C_].proto_id_queue[id], queued, 10);
            strcat(name, queued);
            strcat(name, ")");
        }

        stats[0] = 0;
        int cost = cost_factor(BaseCurrent->faction_id_current, 1, -1)
            * VehPrototypes[id].cost;
        int proto_cost = cost_factor(BaseCurrent->faction_id_current, 1, -1)
            * veh_cost(id, BaseIDCurrentSelected, &has_proto_cost);
        if (BaseCurrent->mineral_surplus <= 0) {
            strcat(stats, "[");
            strcat(stats, label_get(67));
            strcat(stats, "]");
        } else {
            if (!field_B68_) {
                int minerals;
                if (base_making(id, BaseIDCurrentSelected)
                        != base_making(last_id, BaseIDCurrentSelected)
                    && PlayersData[field_A76C_].diff_level > 1) {
                    minerals = BaseCurrent->minerals_accumulated_2;
                    if (minerals > static_cast<int>(Rules.retool_exemption)) {
                        minerals = (minerals - static_cast<int>(Rules.retool_exemption))
                            * (100 - static_cast<int>(Rules.retool_pct_pen_prod_chg)) / 100
                            + static_cast<int>(Rules.retool_exemption);
                    }
                } else {
                    minerals = BaseCurrent->minerals_accumulated_2;
                }
                cost -= minerals;
                proto_cost -= minerals;
            }
            char turns[80];
            _itoa(range((BaseCurrent->mineral_surplus + proto_cost - 1)
                        / BaseCurrent->mineral_surplus, 0, 999999),
                  turns, 10);
            strcat(stats, turns);
            if (cost != proto_cost && VehPrototypes[id].plan != 8) {
                strcat(stats, "(");
                char retool[80];
                _itoa(range((BaseCurrent->mineral_surplus + cost - 1)
                            / BaseCurrent->mineral_surplus, 0, 999999),
                      retool, 10);
                strcat(stats, retool);
                strcat(stats, ")");
            }
            strcat(stats, " ");
            strcat(stats, label_get(49));
        }

        effect[0] = 0;
        say_stats_3(effect, id);
        if (has_proto_cost) flags = 0x80;
        {
            const int kind = edbc_unit(id) | flags | 0x20;
            StringList *list = &prod_a_;
            list->pending_text_ = name;
            list->field_20_ = kind;
            list->field_24_ = 0;
            list->add(id);
        }
        {
            StringList *list = &prod_b_;
            list->pending_text_ = stats;
            list->field_20_ = 0;
            list->field_24_ = 0;
            list->add(0);
        }
        {
            StringList *list = &prod_c_;
            list->pending_text_ = effect;
            list->field_20_ = 0;
            list->field_24_ = 0;
            list->add(0);
        }
        if (id == queued_id) field_A774_ = field_A818_;
        field_A818_++;
    }

    for (i = 1; i <= 0x85; i++) {
        if (!facility_avail(i, BaseCurrent->faction_id_current, base_id, field_B68_)) continue;

        name[0] = 0;
        if (i == 1) {
            strcat(name, "(");
            strcat(name, label_get(140));
            strcat(name, ")");
            strcat(name, " ");
        }
        strcat(name, StringTable.get(Facility[i].name));

        stats[0] = 0;
        int cost = cost_factor(BaseCurrent->faction_id_current, 1, -1) * Facility[i].cost;
        if (!field_B68_) {
            if (base_making(-i, BaseIDCurrentSelected) != base_making(last_id, BaseIDCurrentSelected)
                && PlayersData[field_A76C_].diff_level > 1) {
                cost -= BaseCurrent->minerals_accumulated / 2;
            } else {
                cost -= BaseCurrent->minerals_accumulated;
            }
        }
        if (i == 0x45) {
            char half[80];
            _itoa(range((BaseCurrent->mineral_surplus + 1) / 2, 0, 9999), half, 10);
            strcat(stats, half);
            strcat(stats, " ");
            strcat(stats, label_get(36));
            strcat(stats, " ");
            strcat(stats, label_get(153));
        } else if (BaseCurrent->mineral_surplus <= 0) {
            strcat(stats, "[");
            strcat(stats, label_get(67));
            strcat(stats, "]");
        } else {
            char turns[80];
            _itoa((BaseCurrent->mineral_surplus + cost - 1) / BaseCurrent->mineral_surplus,
                  turns, 10);
            strcat(stats, turns);
            strcat(stats, " ");
            strcat(stats, label_get(49));
        }

        effect[0] = 0;
        if (strlen(StringTable.get(Facility[i].effect))) {
            strcat(effect, StringTable.get(Facility[i].effect));
        }
        {
            const int kind = edbc_fac(i) | 0x10;
            StringList *list = &prod_a_;
            list->pending_text_ = name;
            list->field_20_ = kind;
            list->field_24_ = 0;
            list->add(i);
        }
        {
            StringList *list = &prod_b_;
            list->pending_text_ = stats;
            list->field_20_ = 0;
            list->field_24_ = 0;
            list->add(0);
        }
        {
            StringList *list = &prod_c_;
            list->pending_text_ = effect;
            list->field_20_ = 0;
            list->field_24_ = 0;
            list->add(0);
        }
        if (-i == queued_id) field_A774_ = field_A818_;
        field_A818_++;
    }

    if (field_A818_) {
        field_A824_ = TutWinBaseWindow.pixel_width();
        field_A828_ = TutWinBaseWindow.pixel_height();
        column_count_ = 5;
        field_A814_ = (WinScreenWidth > 800) + 3;
        const int width = (TutWinBaseWindow.pixel_width() - 0x51) / 5;
        row_height_ = field_A838_;
        cell_width_ = width;
        column_count_ = (TutWinBaseWindow.pixel_width() - 0x51) / width;
        field_A82C_ = field_A814_ * column_count_;
        font1_.init(DefaultFontFace, 0xC, 1);
        int size = cell_width_ / 15;
        if (size < 0x18) size = 0x18;
        font2_.init(DefaultFontFace, size, 1);
        size = cell_width_ / 15;
        if (size < 10) size = 10;
        font3_.init(DefaultFontFace, size, 0);
        font4_.init(DefaultFontFace, 0xE, 1);
    }
}


// WORKAROUND(inline): expand_rect (scroll.cpp) is a plain out-of-line
// function in this tree; exec's footer expands it inline (0x00494F48: the
// inset 3 held in eax, applied left/right/top/bottom).
static inline RECT *prodpicker_expand_rect(RECT *rect, int horizontal, int vertical) {
    rect->left -= horizontal;
    rect->right += horizontal;
    rect->top -= vertical;
    rect->bottom += vertical;
    return rect;
}

// ORIGINAL: 0x00494700 ?exec@ProdPicker@@QAEHPAUBase@@HPAUGraphicWin@@H@Z 0x00494700-0x0049515F;0x00658C1E-0x00658C3E FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00494700.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// TRIED: faithful field/call transcription (two local Buffer RAII objects, real BaseButton/Scroll/Caviar calls) diverges at instr #14 (lea vs xor), early prologue-area register choice; not chased further given the 2687-byte SEH-frame scope.
// symbol    ?exec@ProdPicker@@QAEHPAUBase@@HPAVGraphicWin@@H@Z
int ProdPicker::exec(Base *base, int factionId, GraphicWin *owner, int modal) {
    // Both local Buffers are real function-lifetime objects in the image. The
    // second receives the mask sprite used while the Caviar preview is set up.
    Buffer localBuffer1;
    Buffer localBuffer2;
    (void)owner;

    int status = 0;
    if (is_visible()) {
        if (modal) {
            if (field_B68_ == 0) status = 1;
        } else if (field_B68_ != 0) {
            status = 1;
        }
    } else {
        status = 2;
    }
    field_B68_ = modal;

    if (status != 0) {
        if (status == 2) {
            localBuffer2.init(10, 10, 0, 0);
            TutWinBaseWindow.sprite6_.draw(
                &localBuffer2, TutWinBaseWindow.sprite6_.transparent_index(),
                0, 0, 1, 1);

            BoxSpriteParams localTemplate = InterfaceBoxStyles[0];
            localTemplate.mask_ = &localBuffer2;
            field_B64_ = 0;
            field_A76C_ = static_cast<uint32_t>(factionId);
            field_A770_ = base;
            caviar_.init();
            field_A740_ = 0.0f;
            caviar_.set_field_A4(0);
            field_A744_ = 0.785f;
            field_A73C_ = 0.0f;
            caviar_apply_rotation(&field_A73C_, &field_A748_);

            field_A748_.values[2][0] = -field_A748_.values[2][0];
            field_A748_.values[2][1] = -field_A748_.values[2][1];
            field_A748_.values[2][2] = -field_A748_.values[2][2];
            VOX_Vect camera;
            camera.values[0] = field_A748_.values[2][0] * -10000.0f;
            camera.values[1] = field_A748_.values[2][1] * -10000.0f;
            camera.values[2] = field_A748_.values[2][2] * -10000.0f;
            caviar_.set_camera_direct(&camera, &field_A748_);
            caviar_.set_scaling(0.1f);

            camera.values[0] = 0.0f;
            camera.values[1] = 0.0f;
            camera.values[2] = 1.0f;
            caviar_.shadow_vector() = camera;
            caviar_.light_vector() = camera;

            calculate();

            if (TutWinBaseWindow.is_visible()) {
                init(0, 0, static_cast<int>(field_A824_),
                     static_cast<int>(field_A828_), 0, 0x1004020,
                     &TutWinBaseWindow, 0, 0);
            } else {
                const RECT &baseRect = TutWinBaseWindow.outer_rect();
                Win *const consoleWindow = static_cast<Win *>(
                    static_cast<GraphicWin *>(&ConsoleGlobal));
                init(baseRect.left, baseRect.top, static_cast<int>(field_A824_),
                     static_cast<int>(field_A828_), 0, 0x1000020,
                     consoleWindow, 0, 0);
            }

            fill(9);
            RECT frame;
            set_rectangle_dimensions(&frame, 0x17, 0,
                                     static_cast<int>(field_A824_) - 0x4E,
                                     static_cast<int>(field_A828_));
            box_sprite(&frame, &localTemplate);

            field_B70_ = 100;
            if (field_A82C_ < field_A818_) {
                scroll_.init(pixel_width() - 0x14,
                             0x15, 0x18, 0x13A, this, 0, 1);
                scroll_.set_range(0,
                    field_A818_ / column_count_ - field_A814_ + 2);
                scroll_.set_paging(field_A814_ - 1);
                scroll_.set_field_A30(0);
                scroll_.redraw();
                scroll_.hide();
            }

            field_A77C_ = 0;
            if (field_A774_ >= field_A82C_) {
                do {
                    field_A77C_ += column_count_;
                    scroll_.set_pos(field_A77C_ / column_count_);
                } while (field_A774_ >= field_A77C_ + field_A82C_);
            }
        }

        if (field_B68_ != 0) {
            field_B6C_ = 6;
        } else {
            field_B6C_ = 4;
            if (!((1 << factionId)
                    & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_))
                || (GameDataStateStorage.fields_.game_state_ & 0x40
                    && BaseCurrent->governor & 0x80000000U
                    && BaseCurrent->governor & 1U)) {
                field_B6C_ = 5;
            }
        }

        int y = static_cast<int>(field_B70_);
        const int yFixed = static_cast<int>(row_height_ * field_A814_) + 8;
        int x = ((pixel_width()
                  - (y + 3) * static_cast<int>(field_B6C_)) - 0x20) / 2
            + (static_cast<int>(field_B6C_) - 1) * (y + 3);
        RECT footer;
        footer.bottom = yFixed + 0x14;
        footer.right = y + x;

        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x198 / 4]));
        if (Language != 2) CharUpperA(StringTemp);
        flatButtons_[0].init(StringTemp, -2, x, yFixed,
                             static_cast<int>(field_B70_), 0x14, this, 0);
        x += -3 - static_cast<int>(field_B70_);

        if (field_B68_ != 0) {
            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x728 / 4]));
            CharUpperA(StringTemp);
            flatButtons_[3].init(StringTemp, 4, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x72C / 4]));
            CharUpperA(StringTemp);
            flatButtons_[2].init(StringTemp, 5, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0xFE8 / 4]));
            CharUpperA(StringTemp);
            flatButtons_[1].init(StringTemp, 7, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x194 / 4]));
            CharUpperA(StringTemp);
            flatButtons_[4].init(StringTemp, -1, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
        } else {
            if (!(static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                    & (1 << BaseCurrent->faction_id_current))
                || (BaseCurrent->governor & 1U
                    && BaseCurrent->governor & 0x80000000U
                    && GameDataStateStorage.fields_.game_state_ & 0x40)) {
                StringTemp[0] = 0;
                strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x4F4 / 4]));
                CharUpperA(StringTemp);
                flatButtons_[6].init(StringTemp, 6, x, yFixed,
                                     static_cast<int>(field_B70_), 0x14,
                                     this, 0);
                x += -3 - static_cast<int>(field_B70_);
            }

            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x194 / 4]));
            CharUpperA(StringTemp);
            flatButtons_[2].init(StringTemp, -1, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x9B4 / 4]));
            CharUpperA(StringTemp);
            flatButtons_[4].init(StringTemp, 1, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
        }

        footer.left = x;
        footer.top = yFixed;
        StringTemp[0] = 0;
        strcat(StringTemp, StringTable.get(Labels.strings_ptr[0x19C / 4]));
        CharUpperA(StringTemp);
        flatButtons_[5].init(StringTemp, 0, x, yFixed,
                             static_cast<int>(field_B70_), 0x14, this, 0);

        for (int buttonIndex = 0; buttonIndex < 8; ++buttonIndex) {
            flatButtons_[buttonIndex].set_font(&font4_, 0, 0, 0);
            flatButtons_[buttonIndex].redraw();
            flatButtons_[buttonIndex].set_text_color(0x41, -1, 1, 1);
            flatButtons_[buttonIndex].redraw();
        }

        box_sprite(prodpicker_expand_rect(&footer, 3, 3), &InterfaceBoxStyles[3]);
    }

    if (field_B68_ != 0) {
        for (int buttonToSuppress = 0; buttonToSuppress < 22; ++buttonToSuppress)
            TutWinBaseWindow.flatButtons_[buttonToSuppress].set_input_suppressed(1);
        set_modal(0, pop_expire, &MainInterfaceGlobal);
        for (int buttonToRestore = 0; buttonToRestore < 22; ++buttonToRestore)
            TutWinBaseWindow.flatButtons_[buttonToRestore].set_input_suppressed(0);
    } else {
        set_modal(0, pop_expire, 0);
    }

    if (modal == 0) hide();

    int index = field_A774_;
    if (index == -1) {
        if (field_A778_ != 0) return 0;
        return -9999;
    }
    prod_a_.seek_pos(field_A774_);

    const uint32_t flags = prod_a_.head_ != 0
        ? prod_a_.current_->string_payload()->field_8_ : 0;
    int result;
    if (flags & 0x20) {
        result = prodpicker_current_id(&prod_a_);
    } else {
        result = -prodpicker_current_id(&prod_a_);
    }
    return result;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00493EA0 ?on_right_click@ProdPicker@@QAEXHH@Z 0x00493EA0-0x0049419F;0x00658C09-0x00658C1E FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00493ea0.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// symbol    ?on_right_click@ProdPicker@@UAEXHH@Z
void ProdPicker::on_right_click(int x, int y) {
    PullDown menu;
    on_left_click(x, y);
    client_to_screen(&x, &y);

    // The seek result is ignored: a failed bound leaves the cursor where it
    // was and the flag test still reads it.
    prod_a_.seek_pos(field_A774_);
    const uint32_t flags = prod_a_.head_ != 0
        ? prod_a_.current_->string_payload()->field_8_ : 0;
    const int productionId = (flags & 0x20) != 0
        ? prodpicker_current_id(&prod_a_) : -1;

    menu.init();
    if (field_B68_ == 0)
        menu.add_item(5, StringTable.get(Labels.strings_ptr[0x1148 / 4]));
    menu.add_item(4, StringTable.get(Labels.strings_ptr[0x1144 / 4]));
    // Faction-designed prototypes (id >= 0x40) always offer the obsolete row.
    if (productionId >= 0
        && (productionId >= 0x40
            || (Weapon[VehPrototypes[productionId].weapon_id].offense_rating >= 0
                && productionId != 0x0F))) {
        menu.add_item(1, StringTable.get(Labels.strings_ptr[0xE40 / 4]));
    }
    if (!IsMultiplayerNet)
        menu.add_item(3, StringTable.get(Labels.strings_ptr[0x1140 / 4]));
    menu.add_item(2, label_get(0x8FC / 4));
    menu.show(x, y, 0);

    const int selectedItem = menu.get_selected();
    // Native's signed upper-bound check admits cancellation (-1), whose
    // items_[-1].id is GraphicWin's word at +0xA0C (original behaviour).
    const uint32_t action = selectedItem < 0x40
        ? static_cast<uint32_t>(menu.item_id(selectedItem)) : 0xffffffffU;

    // Original SUB EAX,1 wraps for every 32-bit callback/item word.
    switch (action - 1U) {
    case 0:
    {
        const int faction = BaseCurrent->faction_id_current;
        VehPrototypes[productionId].obsolete_factions |=
            static_cast<uint8_t>(1U << faction);
        synch_proto(productionId);
        goto refresh;
    }
    case 1:
        on_button_clicked(0);
        break;
    case 2:
    {
        for (int proto = 0; proto < MaxVehProtoNum; ++proto) {
            if (VehPrototypes[proto].flags & PROTO_ACTIVE)
                VehPrototypes[proto].obsolete_factions &= static_cast<uint8_t>(
                    ~(1U << BaseCurrent->faction_id_current));
        }
    }
    refresh:
        calculate();
        redraw();
        break;
    case 3:
        on_button_clicked(5);
        break;
    case 4:
        on_button_clicked(-1);
        break;
    default:
        break;
    }
}

// The three 0x30-byte blocks at 0xA780/0xA7B0/0xA7E0 - read once as unnamed
// "generic sub-objects" staged through a computed adjustor - are StringLists
// (see prodpicker.h): prod_picker_unknown_a7_block's store sequence is
// StringList's compiler-emitted construction, stage for stage. The helper
// and basepop.cpp's hand copies of the tables (BasePopVbtable,
// BasePopStageTables6693A0/6698C0) are gone; prod_a_/prod_b_/prod_c_ build
// from the declaration.

/*
// ORIGINAL: 0x00492420 ??0ProdPicker@@QAE@XZ 0x00492420-0x004926AD;0x00658B70-0x00658C09 BYTE_EXACT
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
// C++ constructs the GraphicWin base before these typed members;
// declaration order follows the image's subobject order. Their destructors
// similarly own teardown after this destructor body.
ProdPicker::ProdPicker() {
    field_A830_ = 0x86;
    field_A834_ = 0x86;
    field_A14_ = 0;
    field_A838_ = 0x6C;
    field_A83C_ = 0x5E;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004926C0 ?show@ProdPicker@@QAEXH@Z 0x004926C0-0x00492735 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004926c0.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// symbol    ?show@ProdPicker@@UAEXH@Z
void ProdPicker::show(int) {
    if (is_visible() != 0) return;

    if (field_B68_ != 0) {
        TutWinBaseWindow.flatButtons_[20].set_pressed_visual(1);
        TutWinBaseWindow.flatButtons_[20].redraw();
    }
    if (TutWinBaseWindow.is_visible() == 0)
        slide_show(this, 0);
    if (field_A82C_ < field_A818_)
        scroll_show(this, &scroll_);
    Win::show(2);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00492740 ?hide@ProdPicker@@QAEXXZ 0x00492740-0x004927B1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00492740.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// symbol    ?hide@ProdPicker@@UAEXXZ
void ProdPicker::hide() {
    if (is_visible() == 0) return;

    if (field_B68_ != 0) {
        TutWinBaseWindow.flatButtons_[20].set_pressed_visual(0);
        TutWinBaseWindow.flatButtons_[20].redraw();
    }
    if (field_A82C_ < field_A818_)
        scroll_hide(this, &scroll_);
    if (TutWinBaseWindow.is_visible() == 0)
        slide_hide(this, 0);
    Win::hide();
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
// symbol    ?on_mouse_move@ProdPicker@@UAEXHH@Z
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
// Both image callers (the destructor among them) call this out of line.
#pragma auto_inline(off)
void ProdPicker::close() {
    field_A14_ = 0;
    GraphicWin::close();
}
#pragma auto_inline(on)


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

/*
Purpose: ?on_scrolling@ProdPicker@@QAEXHH@Z - dispatch to slot 0xc0 of this object's OWN
         vtable, forwarding 2 argument(s).
// ORIGINAL: 0x00493E70 ?on_scrolling@ProdPicker@@QAEXHH@Z 0x00493E70-0x00493E87 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066A250, pointer word at +0xC4, identifies original entry 0x00493E70", "source:src/win.h inherited virtual interface; actual emitted method ?vslot_49@ProdPicker@@UAEXHH@Z in primary Win table ??_7ProdPicker@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 49. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_scrolling@ProdPicker@@UAEXHH@Z
// size      23 bytes
// prototype void (__thiscall ?on_scrolling@ProdPicker@@QAEXHH@Z)(ProdPicker* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00493E7D
Return Value: n/a
Status: Complete
*/
void ProdPicker::on_scrolling(int code, int position) {
  on_scrolled(code, position);
}
// RECOVERY_GENERATED: v1 vtable 0x0066A250 0x0066A3B8 ??_7ProdPicker@@6BWin@@@ 0x00492420
// RECOVERY_GENERATED: v1 vtable 0x0066A248 ? ??_7ProdPicker@@6BBuffer@@@ 0x00492420
