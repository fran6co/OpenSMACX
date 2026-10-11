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
// ORIGINAL: 0x004213A0 ??_GProdPicker@@UAEPAXI@Z 0x004213A0-0x004213C1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004213A0-0x004213C1", "Pinned class constructor ??0ProdPicker@@QAE@XZ at 0x00492420 stores original class/view table VA 0x66a250 at 0x00492679 (mov dword ptr [esi], 0x66a250); the current genuine owner vftable ??_7ProdPicker@@6BWin@@@ selects generated ??_GProdPicker@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EProdPicker@@UAEPAXI@Z (mode 2).", "Pinned ??_GProdPicker@@UAEPAXI@Z at 0x004213A0 calls the source-mapped ??1ProdPicker@@QAE@XZ at 0x00421100, then tests byte ptr [ebp + 8], 1; je 0x4213ba bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/prodpicker.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original ProdPicker scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GProdPicker@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "interface_fonts.h"
#include "basepop.h"
#include "stringstruct.h"  // StringAllocationHeap; the staged vbtable constants
#include "prodpicker.h"
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
// ORIGINAL: 0x00493C20 ?on_mousewheel@ProdPicker@@QAEXH@Z 0x00493C20-0x00493C68 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00493c20.cpp", "source:src/prodpicker.h", "source:src/scroll.h"], "reason": "Dispatch each wheel step to ProdPicker's real embedded Scroll rather than a raw receiver offset or vtable shim. The source form currently mismatches; retain the seed's historical exact claim as a restoration obligation, without demotion. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?on_mousewheel@ProdPicker@@UAEXH@Z
// Current virtual override emits UAE; retain the catalogued QAE identity above.
// kind      game
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
// symbol    ?on_left_click@ProdPicker@@UAEXHH@Z
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

// ORIGINAL: 0x00493C70 ?on_redraw@ProdPicker@@QAEXXZ 0x00493C70-0x00493CA6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/00493c70.cpp"], "reason": "Home the redraw callback in ProdPicker; it repaints exactly the currently visible item interval through the typed draw_item operation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_redraw@ProdPicker@@UAEXXZ
void ProdPicker::on_redraw() {
    const int first = static_cast<int>(field_A77C_);
    const int end = first + static_cast<int>(field_A82C_);
    for (int item = first; item < end; ++item) {
        draw_item(item, 0);
    }
}

// ORIGINAL: 0x00494230 ?on_left_double_click@ProdPicker@@QAEXHH@Z 0x00494230-0x00494271 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:20b88b523285a816ba99f503d727b761b89b227f:src/recovered/units/00494230.cpp", "source:src/win.h:Win::on_left_click (candidate spelling for the inherited Win slot-19 callback; fresh emitted-symbol evidence is pending)", "source:src/prodpicker.h:ProdPicker::on_button_clicked"], "reason": "Home the measured left double click event in ProdPicker and forward its notification and button selection through inherited virtual behavior."}
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

// The legacy keyboard handler is a direct ProdPicker operation (not a Win
// virtual override). Its second argument is the key code; the first is unused.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: a straight port of the jump-table switch (12 case labels, two cross-case gotos folded away algebraically once traced by hand) - matches semantics and field offsets (confirmed against param_1[N] * 4 in the Ghidra dump) but the compiler's jump table shape/ordering diverges early; not chased further.
// ORIGINAL: 0x00494280 ?on_key_click@ProdPicker@@QAEHHH@Z 0x00494280-0x00494545 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00494280.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// symbol    ?on_key_click@ProdPicker@@UAEHHH@Z
int ProdPicker::on_key_click(int repeat, int key) {
    (void)repeat;
    const int oldFirst = field_A77C_;
    const int oldSelection = field_A774_;
    int position;

    switch (key) {
    case 0x21: // Page Up
        if (field_A818_ <= field_A82C_) return 1;
        position = scroll_.position() - scroll_.paging();
        goto update_scroll;
    case 0x22: // Page Down
        if (field_A818_ <= field_A82C_) return 1;
        position = scroll_.paging() + scroll_.position();
update_scroll:
        scroll_.set_pos(position);
        field_A77C_ = field_A810_ * scroll_.position();
        break;
    case 0x23: // End
        field_A774_ = field_A818_ - 1;
        if (field_A82C_ < field_A818_)
            field_A77C_ = (field_A774_ / static_cast<int>(field_A810_) + 1)
                * static_cast<int>(field_A810_) - field_A82C_;
        scroll_.set_pos(field_A77C_ / static_cast<int>(field_A810_));
        break;
    case 0x24: // Home
        scroll_.set_pos(0);
        field_A77C_ = 0;
        field_A774_ = 0;
        break;
    case 0x25: // Left; keypad 4
    case 100:
    {
        const int first = (oldSelection / static_cast<int>(field_A810_))
            * static_cast<int>(field_A810_);
        int last = first + field_A810_ - 1;
        if (field_A818_ - 1 <= last) last = field_A818_ - 1;
        const int previous = oldSelection - 1;
        if (first <= previous && first <= last)
            field_A774_ = last < previous ? last : previous;
        else
            field_A774_ = first;
        break;
    }
    case 0x26: // Up; keypad 8
    case 0x68:
    {
        const int step = field_A810_;
        if (oldSelection < step) return 1;
        int current;
        do {
            current = field_A774_ - step;
            field_A774_ = current;
        } while (field_A82C_ + oldFirst <= current);
        if (current < oldFirst) {
            field_A77C_ = oldFirst - step;
            scroll_.set_pos(scroll_.position() - 1);
        }
        break;
    }
    case 0x27: // Right; keypad 6
    case 0x66:
    {
        const int first = (oldSelection / static_cast<int>(field_A810_))
            * static_cast<int>(field_A810_);
        int last = first + field_A810_ - 1;
        if (field_A818_ - 1 <= last) last = field_A818_ - 1;
        const int next = oldSelection + 1;
        if (next < first || last < first) {
            field_A774_ = first;
        } else {
            field_A774_ = next < last ? next : last;
        }
        break;
    }
    case 0x28: // Down; keypad 2
    case 0x62:
    {
        const int step = field_A810_;
        if (field_A818_ - step - 1 < oldSelection) {
            if (field_A818_ <= field_A82C_) return 1;
            scroll_.set_pos(scroll_.position() + 1);
            field_A77C_ = scroll_.position() * field_A810_;
        } else {
            int current = field_A774_;
            do {
                current += step;
            } while (current < oldFirst);
            field_A774_ = current;
            if (field_A82C_ + oldFirst <= current) {
                field_A77C_ = step + oldFirst;
                scroll_.set_pos(scroll_.position() + 1);
            }
        }
        break;
    }
    }

    if (oldFirst == field_A77C_) {
        if (oldSelection != field_A774_) {
            field_B64_ = 0;
            draw_item(oldSelection, 0);
            draw_item(field_A774_, 0);
        }
    } else {
        field_B64_ = 0;
        redraw();
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

// ORIGINAL: 0x00492FA0 ?calculate@ProdPicker@@QAEXXZ 0x00492FA0-0x00493A6D FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00492fa0.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// TRIED: void return confirmed (0,1,2,3 are switch-derived flag bits; 9999/999999 are itoa() display clamps, not status codes - the single `ret` has no `mov eax,K` before it, eax at exit is whatever Font::init() last returned). Structural transcription via named ProdPicker::field_XXXX_ members instead of raw pointer casts didn't close the divergence; prologue shape differs (#0 push vs sub) from local layout, not a wrong op.
void ProdPicker::calculate() {
    bool anyTech = false;
    const int savedFaction = BaseIDCurrentSelected;
    char nameBuf[256];
    char statsBuf[256];
    char thirdBuf[256];
    char numBuf1[80];
    char numBuf2[80];
    char numBuf3[80];
    char numBuf4[80];

    prod_a_.remove_all();
    prod_a_.current_position_ = 0;
    prod_a_.allocator_ = 0;
    prod_b_.remove_all();
    prod_b_.current_position_ = 0;
    prod_b_.allocator_ = 0;
    prod_c_.remove_all();
    prod_c_.current_position_ = 0;
    prod_c_.allocator_ = 0;
    field_A818_ = 0;
    field_A774_ = -1;
    field_A77C_ = 0;

    Base *const base = BaseCurrent;
    const int maxVehCost = base->queue_production_id[field_B68_];
    const int currentProduction = base->production_id_last;
    const int faction = static_cast<int>(field_A76C_);
    proto_sort(faction);

    // A faction with any researched prerequisite above its current weapon,
    // armor, or chassis technology receives the editor's upgrade action.
    for (int weaponIndex = 0; weaponIndex < MaxWeaponNum && !anyTech; ++weaponIndex) {
        if (Weapon[weaponIndex].offense_rating > 1
            && has_tech(Weapon[weaponIndex].preq_tech, faction))
            anyTech = true;
    }
    for (int armorIndex = 0; armorIndex < MaxArmorNum && !anyTech; ++armorIndex) {
        if (Armor[armorIndex].defense_rating > 1
            && has_tech(Armor[armorIndex].preq_tech, faction))
            anyTech = true;
    }
    for (int chassisIndex = 0; chassisIndex < MaxChassisNum && !anyTech; ++chassisIndex) {
        if (Chassis[chassisIndex].speed > 1
            && has_tech(Chassis[chassisIndex].preq_tech, faction))
            anyTech = true;
    }

    for (int prototypeIndex = 0; prototypeIndex < 128; ++prototypeIndex) {
        const int protoId = ProtoSortIds[prototypeIndex];
        if (!veh_avail(protoId, faction, savedFaction)) continue;

        nameBuf[0] = '\0';
        strcat(nameBuf, VehPrototypes[protoId].veh_name);
        if (GameDataStateStorage.fields_.game_state_ & 0x80) {
            strcat(nameBuf, "(");
            _itoa(PlayersData[faction].proto_id_active[protoId], numBuf1, 10);
            strcat(nameBuf, numBuf1);
            strcat(nameBuf, " ");
            _itoa(PlayersData[faction].proto_id_queue[protoId], numBuf2, 10);
            strcat(nameBuf, numBuf2);
            strcat(nameBuf, ")");
        }

        statsBuf[0] = '\0';
        int costA = cost_factor(base->faction_id_current, 1, -1)
            * VehPrototypes[protoId].cost;
        int costB = cost_factor(base->faction_id_current, 1, -1)
            * veh_cost(protoId, savedFaction, &field_A77C_);

        if (base->mineral_surplus < 1) {
            strcat(statsBuf, "[");
            strcat(statsBuf, label_get(0x10C / 4));
            strcat(statsBuf, "]");
        } else {
            if (field_B68_ == 0) {
                const int makingA = base_making(protoId, savedFaction);
                const int makingB = base_making(currentProduction, savedFaction);
                int threshold = base->minerals_accumulated;
                if (makingA != makingB && PlayersData[faction].diff_level >= 2
                    && static_cast<int>(Rules.retool_exemption) < threshold) {
                    threshold = (threshold - static_cast<int>(Rules.retool_exemption))
                        * (100 - static_cast<int>(Rules.retool_pct_pen_prod_chg))
                        / 100 + static_cast<int>(Rules.retool_exemption);
                }
                costA -= threshold;
                costB -= threshold;
            }
            int turns = (base->mineral_surplus - 1 + costB) / base->mineral_surplus;
            if (turns < 0) turns = 0;
            else if (turns > 999999) turns = 999999;
            _itoa(turns, numBuf3, 10);
            strcat(statsBuf, numBuf3);
            if (costA != costB && VehPrototypes[protoId].plan != 8) {
                strcat(statsBuf, "(");
                int turnsA = (base->mineral_surplus - 1 + costA)
                    / base->mineral_surplus;
                if (turnsA < 0) turnsA = 0;
                else if (turnsA > 999999) turnsA = 999999;
                _itoa(turnsA, numBuf4, 10);
                strcat(statsBuf, numBuf4);
                strcat(statsBuf, ")");
            }
            strcat(statsBuf, " ");
            strcat(statsBuf, label_get(0xC4 / 4));
        }

        thirdBuf[0] = '\0';
        say_stats_3(thirdBuf, protoId);
        const uint32_t extraFlags = field_A77C_ != 0 ? 0x80 : 0;
        const int category = edbc_unit(protoId);
        prod_a_.pending_text_ = nameBuf;
        prod_a_.field_20_ = static_cast<uint32_t>(category) | extraFlags | 0x20;
        prod_a_.field_24_ = 0;
        prod_a_.add(protoId);
        prod_b_.pending_text_ = statsBuf;
        prod_b_.field_20_ = 0;
        prod_b_.field_24_ = 0;
        prod_b_.add(0);
        prod_c_.pending_text_ = thirdBuf;
        prod_c_.field_20_ = 0;
        prod_c_.field_24_ = 0;
        prod_c_.add(0);
        if (protoId == maxVehCost) field_A774_ = field_A818_;
        ++field_A818_;
    }

    for (int facilityId = 1; facilityId <= 0x85; ++facilityId) {
        if (!facility_avail(facilityId, base->faction_id_current,
                            savedFaction, field_B68_)) continue;

        nameBuf[0] = '\0';
        if (facilityId == 1) {
            strcat(nameBuf, "(");
            strcat(nameBuf, label_get(0x230 / 4));
            strcat(nameBuf, ")");
            strcat(nameBuf, " ");
        }
        strcat(nameBuf, StringTable.get(Facility[facilityId].name));

        statsBuf[0] = '\0';
        int cost = cost_factor(base->faction_id_current, 1, -1) * Facility[facilityId].cost;
        if (field_B68_ == 0) {
            const int makingA = base_making(-facilityId, savedFaction);
            const int makingB = base_making(currentProduction, savedFaction);
            if (makingA == makingB || PlayersData[faction].diff_level < 2)
                cost -= base->nutrients_accumulated;
            else
                cost -= base->nutrients_accumulated / 2;
        }

        if (facilityId == 0x45) {
            int half = (base->mineral_surplus + 1) / 2;
            if (half < 0) half = 0;
            else if (half > 9999) half = 9999;
            _itoa(half, numBuf3, 10);
            strcat(statsBuf, numBuf3);
            strcat(statsBuf, " ");
            strcat(statsBuf, label_get(0x90 / 4));
            strcat(statsBuf, " ");
            strcat(statsBuf, label_get(0x264 / 4));
        } else if (base->mineral_surplus > 0) {
            int turns = (base->mineral_surplus - 1 + cost) / base->mineral_surplus;
            _itoa(turns, numBuf1, 10);
            strcat(statsBuf, numBuf1);
            strcat(statsBuf, " ");
            strcat(statsBuf, label_get(0xC4 / 4));
        } else {
            strcat(statsBuf, "[");
            strcat(statsBuf, label_get(0x10C / 4));
            strcat(statsBuf, "]");
        }

        thirdBuf[0] = '\0';
        const char *suffix = StringTable.get(Facility[facilityId].effect);
        if (strlen(suffix) != 0) strcat(thirdBuf, suffix);

        prod_a_.pending_text_ = nameBuf;
        prod_a_.field_20_ = static_cast<uint32_t>(edbc_fac(facilityId)) | 0x10;
        prod_a_.field_24_ = 0;
        prod_a_.add(facilityId);
        prod_b_.pending_text_ = statsBuf;
        prod_b_.field_20_ = 0;
        prod_b_.field_24_ = 0;
        prod_b_.add(0);
        prod_c_.pending_text_ = thirdBuf;
        prod_c_.field_20_ = 0;
        prod_c_.field_24_ = 0;
        prod_c_.add(0);
        if (-currentProduction == facilityId) field_A774_ = field_A818_;
        ++field_A818_;
    }

    if (field_A818_ != 0) {
        Buffer *const baseBuffer = static_cast<Buffer *>(
            static_cast<GraphicWin *>(&TutWinBaseWindow));
        field_A824_ = static_cast<uint32_t>(baseBuffer->pixel_width());
        field_A828_ = static_cast<uint32_t>(baseBuffer->pixel_height());
        field_A810_ = 5;
        field_A814_ = (WinScreenWidth > 800) + 3;
        const int cellWidth = (baseBuffer->pixel_width() - 0x51) / 5;
        const int columns = (baseBuffer->pixel_width() - 0x51) / cellWidth;
        field_A820_ = field_A838_;
        field_A81C_ = cellWidth;
        field_A810_ = columns;
        field_A82C_ = field_A814_ * columns;

        font1_.init(DefaultFontFace, 0xC, 1);
        int size = cellWidth / 0xF;
        if (size < 0x18) size = 0x18;
        font2_.init(DefaultFontFace, size, 1);
        size = cellWidth / 0xF;
        if (size < 10) size = 10;
        font3_.init(DefaultFontFace, size, 0);
        font4_.init(DefaultFontFace, 0xE, 1);
    }
}

// ORIGINAL: 0x00494700 ?exec@ProdPicker@@QAEHPAUBase@@HPAUGraphicWin@@H@Z 0x00494700-0x0049515F;0x00658C1E-0x00658C3E FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00494700.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// TRIED: faithful field/call transcription (two local Buffer RAII objects, real BaseButton/Scroll/Caviar calls) diverges at instr #14 (lea vs xor), early prologue-area register choice; not chased further given the 2687-byte SEH-frame scope.
// symbol    ?exec@ProdPicker@@QAEHPAUBase@@HPAVGraphicWin@@H@Z
int ProdPicker::exec(Base *base, int factionId, GraphicWin *owner, int modal) {
    // Both local Buffers are real function-lifetime objects in the image. The
    // second receives the mask sprite used while the Caviar preview is set up.
    Buffer localBuffer1;
    Buffer localBuffer2;
    (void)owner;

    int status = 0;
    if (is_visible() == 0) {
        status = 2;
    } else if (modal == 0) {
        if (field_B68_ != 0) status = 1;
    } else if (field_B68_ == 0) {
        status = 1;
    }
    field_B68_ = modal;

    if (status != 0) {
        if (status == 2) {
            localBuffer2.init(10, 10, 0, 0);
            TutWinBaseWindow.sprite6_.draw(
                &localBuffer2, TutWinBaseWindow.sprite6_.transparent_index(),
                0, 0, 1, 1);

            BoxSpriteParams localTemplate = InterfaceBoxStyles[0];
            field_A76C_ = static_cast<uint32_t>(factionId);
            field_B64_ = 0;
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

            caviar_.light_vector().values[0] = 0.0f;
            caviar_.light_vector().values[1] = 0.0f;
            caviar_.light_vector().values[2] = 1.0f;
            caviar_.shadow_vector().values[0] = 0.0f;
            caviar_.shadow_vector().values[1] = 0.0f;
            caviar_.shadow_vector().values[2] = 1.0f;

            calculate();

            if (TutWinBaseWindow.is_visible() == 0) {
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
            RECT frame = { 0x17, 0,
                           static_cast<int>(field_A824_) - 0x37,
                           static_cast<int>(field_A828_) };
            box_sprite(&frame, &localTemplate);

            field_B70_ = 100;
            if (field_A82C_ < field_A818_) {
                scroll_.init(pixel_width() - 0x14,
                             0x15, 0x18, 0x13A, this, 0, 1);
                scroll_.set_range(0,
                    field_A818_ / field_A810_ - field_A814_ + 2);
                scroll_.set_paging(field_A814_ - 1);
                scroll_.set_field_A30(0);
                scroll_.redraw();
                scroll_.hide();
            }

            field_A77C_ = 0;
            if (field_A82C_ <= field_A774_) {
                int first = 0;
                do {
                    first += field_A810_;
                    field_A77C_ = first;
                    scroll_.set_pos(first / field_A810_);
                } while (first + field_A82C_ <= field_A774_);
            }
        }

        if (field_B68_ == 0) {
            field_B6C_ = 4;
            const uint8_t aliveMask = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            const uint8_t gameFlags = static_cast<uint8_t>(GameDataStateStorage.fields_.game_state_);
            const uint32_t baseGovernor = BaseCurrent->governor;
            if (((1U << (factionId & 0x1F)) & aliveMask) == 0
                || ((gameFlags & 0x40) != 0
                    && (baseGovernor & 0x80000000U) != 0
                    && (baseGovernor & 1U) != 0)) {
                field_B6C_ = 5;
            }
        } else {
            field_B6C_ = 6;
        }

        int y = static_cast<int>(field_B70_);
        const int yFixed = static_cast<int>(field_A820_ * field_A814_) + 8;
        int x = ((pixel_width()
                  - (y + 3) * static_cast<int>(field_B6C_)) - 0x20) / 2
            + (static_cast<int>(field_B6C_) - 1) * (y + 3);
        int yFixedBottom = static_cast<int>(field_A820_ * field_A814_) + 0x1C;
        y += x;

        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x198 / 4));
        if (Language != 2) CharUpperA(StringTemp);
        flatButtons_[0].init(StringTemp, -2, x, yFixed,
                             static_cast<int>(field_B70_), 0x14, this, 0);
        x += -3 - static_cast<int>(field_B70_);

        if (field_B68_ == 0) {
            const uint32_t baseGovernor = BaseCurrent->governor;
            const uint8_t aliveMask = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            const uint8_t gameFlags = static_cast<uint8_t>(GameDataStateStorage.fields_.game_state_);
            if (((aliveMask & (1U << (BaseCurrent->faction_id_current & 0x1F))) == 0)
                || ((baseGovernor & 1U) != 0
                    && (baseGovernor & 0x80000000U) != 0
                    && (gameFlags & 0x40) != 0)) {
                StringTemp[0] = 0;
                strcat(StringTemp, label_get(0x4F4 / 4));
                CharUpperA(StringTemp);
                flatButtons_[5].init(StringTemp, 6, x, yFixed,
                                     static_cast<int>(field_B70_), 0x14,
                                     this, 0);
                x += -3 - static_cast<int>(field_B70_);
            }

            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x194 / 4));
            CharUpperA(StringTemp);
            flatButtons_[2].init(StringTemp, -1, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x9B4 / 4));
            CharUpperA(StringTemp);
            flatButtons_[4].init(StringTemp, 1, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
        } else {
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x728 / 4));
            CharUpperA(StringTemp);
            flatButtons_[3].init(StringTemp, 4, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x72C / 4));
            CharUpperA(StringTemp);
            flatButtons_[2].init(StringTemp, 5, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0xFE8 / 4));
            CharUpperA(StringTemp);
            flatButtons_[6].init(StringTemp, 7, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x194 / 4));
            CharUpperA(StringTemp);
            flatButtons_[1].init(StringTemp, -1, x, yFixed,
                                 static_cast<int>(field_B70_), 0x14,
                                 this, 0);
            x += -3 - static_cast<int>(field_B70_);
        }

        const int xBottom = x;
        const int yTop = yFixed;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x19C / 4));
        CharUpperA(StringTemp);
        flatButtons_[8].init(StringTemp, 0, x, yFixed,
                             static_cast<int>(field_B70_), 0x14, this, 0);

        for (int buttonIndex = 0; buttonIndex < 8; ++buttonIndex) {
            Buffer *const buttonBuffer = static_cast<Buffer *>(
                static_cast<GraphicWin *>(&flatButtons_[buttonIndex]));
            buttonBuffer->set_font(&font4_, 0, 0, 0);
            flatButtons_[buttonIndex].redraw();
            flatButtons_[buttonIndex].set_text_color(0x41, -1, 1, 1);
            flatButtons_[buttonIndex].redraw();
        }

        RECT footer = { xBottom - 3, yTop - 3,
                        y + 3, yFixedBottom + 3 };
        box_sprite(&footer, &InterfaceBoxStyles[3]);
    }

    if (field_B68_ == 0) {
        set_modal(0, pop_wait, 0);
    } else {
        for (int buttonToSuppress = 0; buttonToSuppress < 22; ++buttonToSuppress)
            TutWinBaseWindow.flatButtons_[buttonToSuppress].set_input_suppressed(1);
        set_modal(0, pop_wait, &MainInterfaceGlobal);
        for (int buttonToRestore = 0; buttonToRestore < 22; ++buttonToRestore)
            TutWinBaseWindow.flatButtons_[buttonToRestore].set_input_suppressed(0);
    }

    if (modal == 0) hide();

    int index = field_A774_;
    if (index == -1) return field_A778_ != 0 ? 0 : -9999;
    if (index <= prod_a_.entry_count_ - 1)
        prod_a_.seek_pos(index);

    StringStructPayload *const selected = prod_a_.current_entry();
    const uint32_t flags = selected != 0 ? selected->field_8_ : 0;
    int selectedId = selected != 0 ? prod_a_.current_id() : 0;
    if ((flags & 0x20) == 0) selectedId = -selectedId;
    return selectedId;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00493EA0 ?on_right_click@ProdPicker@@QAEXHH@Z 0x00493EA0-0x0049419F;0x00658C09-0x00658C1E FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00493ea0.cpp", "Pinned original coordinator review: actual label-pointer dereference, camera word3F48F5C3, static cdecl callback ABI, row-end clamps and modal-parent/button suppression order."], "reason": "Recover ProdPicker owner behavior through existing window bases, owned StringLists/controls and typed Base/Caviar camera state; preserve historical seed identity and extents as restoration obligations, not current match claims."}
// symbol    ?on_right_click@ProdPicker@@UAEXHH@Z
void ProdPicker::on_right_click(int x, int y) {
    on_left_click(x, y);
    client_to_screen(&x, &y);

    int productionId = -1;
    if (field_A774_ <= prod_a_.entry_count_ - 1) {
        prod_a_.seek_pos(field_A774_);
        StringStructPayload *const selected = prod_a_.current_entry();
        if (selected != 0 && (selected->field_8_ & 0x20) != 0)
            productionId = prod_a_.current_id();
    }

    PullDown menu;
    menu.init();
    if (field_B68_ == 0)
        menu.add_item(5, label_get(0x1148 / 4));
    menu.add_item(4, label_get(0x1144 / 4));
    if (productionId >= 0 && productionId < 0x40
        && Weapon[VehPrototypes[productionId].weapon_id].offense_rating >= 0
        && productionId != 0x0F) {
        menu.add_item(1, label_get(0xE40 / 4));
    }
    if (!IsMultiplayerNet)
        menu.add_item(3, label_get(0x1140 / 4));
    menu.add_item(2, label_get(0x8FC / 4));
    menu.show(x, y, 0);

    const int selectedItem = menu.get_selected();
    uint32_t action = 0xffffffffU;
    if (selectedItem < 0x40) {
        // Native's signed upper-bound check admits cancellation (-1).
        // items_[-1].id lands on GraphicWin's caption callback at +0xA0C:
        // 0xA18 - sizeof(PullDownItem) + offsetof(PullDownItem, id).
        // Read that named word, not an item outside the array. Other
        // indices retain the original unchecked item access.
        action = selectedItem == -1 ? menu.field_A0C_value()
            : static_cast<uint32_t>(menu.item_id(selectedItem));
    }

    // Original SUB EAX,1 wraps for every 32-bit callback/item word.
    switch (action - 1U) {
    case 0:
    {
        const int faction = BaseCurrent->faction_id_current;
        VehPrototypes[productionId].obsolete_factions |=
            static_cast<uint8_t>(1U << faction);
        synch_proto(productionId);
        calculate();
        redraw();
        break;
    }
    case 1:
        on_button_clicked(0);
        break;
    case 2:
    {
        const int faction = BaseCurrent->faction_id_current;
        const uint8_t factionMask = static_cast<uint8_t>(1U << faction);
        for (int proto = 0; proto < MaxVehProtoNum; ++proto) {
            if (VehPrototypes[proto].flags & PROTO_ACTIVE)
                VehPrototypes[proto].obsolete_factions &=
                    static_cast<uint8_t>(~factionMask);
        }
        calculate();
        redraw();
        break;
    }
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

// ORIGINAL: 0x00421100 ??1ProdPicker@@QAE@XZ 0x00421100-0x00421392;0x00651250-0x00651376
// RECOVERY_BINDING: v1 {"evidence": ["git:1b3ed0614446d5283fe4cbd2392cbb64b49db601:src/recovered/units/00421100.cpp", "source:src/prodpicker.h", "git:c96a47e3f89e8792c8514ac2ca79259ab6f42c3e:src/recovered/units/00421100.cpp", "Historical seed spelling ??1ProdPicker@@QAE@XZ; current source mapping ??1ProdPicker@@QAE@XZ; emitted symbol ??1ProdPicker@@UAE@XZ.", "Historical seed funclet identifies FlatButton destructor0x406880; nine embedded FlatButton members require genuine external destructor/lifetime behavior, not synthesized verification scaffolding. The actual GraphicWin-derived virtual destructor emits UAE rather than the catalogue QAE spelling."], "reason": "Home the ProdPicker destructor in its real owning TU; typed embedded members own their cleanup after close(), replacing the external hand-offset teardown forwarder. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// symbol    ??1ProdPicker@@UAE@XZ
ProdPicker::~ProdPicker() {
    close();
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
