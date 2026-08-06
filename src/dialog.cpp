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
#include "original_seam.h"
#include "dialog.h"
#include "stringstruct.h"

/*
Purpose: Set the three fonts used by the dialog.
Original Offset: 00609C60
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int Dialog::set_dialog_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        font1_ = font1;
    }
    font2_ = font2;
    font3_ = font3;
    return 0;
}

/*
Purpose: Set the primary text colors for the four dialog color slots.
Original Offset: 00609C90
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color(int color1, int color2, int color3, int color4) {
    text_color_a_ = color1;
    text_color_b_ = color2;
    text_color_c_ = color3;
    text_color_d_ = color4;
}

/*
Purpose: Set the secondary text colors for the four dialog color slots.
Original Offset: 00609CC0
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color2(int color1, int color2, int color3, int color4) {
    text_color_2a_ = color1;
    text_color_2b_ = color2;
    text_color_2c_ = color3;
    text_color_2d_ = color4;
}

/*
Purpose: Set the tertiary text colors for the four dialog color slots.
Original Offset: 00609CF0
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color3(int color1, int color2, int color3, int color4) {
    text_color_3a_ = color1;
    text_color_3b_ = color2;
    text_color_3c_ = color3;
    text_color_3d_ = color4;
}

/*
Purpose: Find an item ID's bounded position in the dialog string list.
Original Offset: 00609AF0
Return Value: Matching position, or the configured entry count on a miss
Status: Complete
*/
int Dialog::id_to_pos(int id) {
    if (entry_head_) {
        int traversed = 0;
        int count = entry_count_;
        entry_position_ = 0;
        current_entry_ = entry_head_;
        if (count > 0) {
            do {
                DialogEntry *entry = current_entry_;
                if (entry->id == id) {
                    break;
                }
                entry_position_++;
                traversed++;
                current_entry_ = entry->next;
            } while (traversed < count);
        }
    }
    return entry_position_;
}

/*
Purpose: Select a dialog item by its bounded list ID.
Original Offset: 006099D0
Return Value: n/a
Status: Complete
*/
void Dialog::set_selected_id(int id) {
    selected_position_ = id_to_pos(id);
}

/*
Purpose: Restore the selected list position and return its item ID.
Original Offset: 00609A50
Return Value: Selected item ID, or zero when the list head is null
Status: Complete
*/
int Dialog::get_selected_id() {
    int position = selected_position_;
    int count = entry_count_;
    if (count == INT_MIN || position < count) {
        current_entry_ = entry_head_;
        if (position < 0) {
            int distance = position == INT_MIN ? INT_MIN : -position;
            if (distance <= count) {
                while (distance > 0) {
                    current_entry_ = current_entry_->previous;
                    distance--;
                }
                uint32_t normalized =
                    static_cast<uint32_t>(position) + static_cast<uint32_t>(count);
                memcpy(&position, &normalized, sizeof(position));
                entry_position_ = position;
            }
        } else {
            int distance = position;
            while (distance > 0) {
                current_entry_ = current_entry_->next;
                distance--;
            }
            entry_position_ = position;
        }
    }
    return entry_head_ ? current_entry_->id : 0;
}

/*
Purpose: Restore an explicit list position and return its item ID.
Original Offset: 00609B50
Return Value: Item ID at the requested position, or zero when the list head is null
Status: Complete
*/
int Dialog::pos_to_id(int position) {
    int count = entry_count_;
    if (count == INT_MIN || position < count) {
        current_entry_ = entry_head_;
        if (position < 0) {
            int distance = position == INT_MIN ? INT_MIN : -position;
            if (distance <= count) {
                while (distance > 0) {
                    current_entry_ = current_entry_->previous;
                    distance--;
                }
                uint32_t normalized =
                    static_cast<uint32_t>(position) + static_cast<uint32_t>(count);
                memcpy(&position, &normalized, sizeof(position));
                entry_position_ = position;
            }
        } else {
            int distance = position;
            while (distance > 0) {
                current_entry_ = current_entry_->next;
                distance--;
            }
            entry_position_ = position;
        }
    }
    return entry_head_ ? current_entry_->id : 0;
}

int __fastcall dialog_set_font_redirect(
    Dialog *self, void *, Font *font1, Font *font2, Font *font3) {
    return self->set_dialog_font(font1, font2, font3);
}

void __fastcall dialog_set_text_color_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4) {
    self->set_dialog_text_color(color1, color2, color3, color4);
}

void __fastcall dialog_set_text_color2_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4) {
    self->set_dialog_text_color2(color1, color2, color3, color4);
}

void __fastcall dialog_set_text_color3_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4) {
    self->set_dialog_text_color3(color1, color2, color3, color4);
}


int __fastcall dialog_id_to_pos_redirect(Dialog *self, void *, int id) {
    return self->id_to_pos(id);
}

void __fastcall dialog_set_selected_id_redirect(Dialog *self, void *, int id) {
    self->set_selected_id(id);
}

int __fastcall dialog_get_selected_id_redirect(Dialog *self, void *) {
    return self->get_selected_id();
}

int __fastcall dialog_pos_to_id_redirect(Dialog *self, void *, int position) {
    return self->pos_to_id(position);
}

Font **DialogDefaultFonts = reinterpret_cast<Font **>(0x009B8EC0);

/*
Purpose: Set the default fonts shared by every dialog.
Original Offset: 00609D20
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int Dialog::set_def_dialog_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    volatile Font **const slots =
        const_cast<volatile Font **>(DialogDefaultFonts);
    // Only an initialized primary is published; the other two are stored
    // either way and the call still succeeds.
    if (font1->is_initialized()) {
        slots[0] = font1;
    }
    slots[1] = font2;
    slots[2] = font3;
    return 0;
}

int __cdecl dialog_set_def_dialog_font_redirect(
        Font *font1, Font *font2, Font *font3) {
    return Dialog::set_def_dialog_font(font1, font2, font3);
}

func_dialog_close DialogOriginalClose = original_method<func_dialog_close>(0x00608F50);
func_operator_delete *DialogOperatorDelete =
    (func_operator_delete *)0x0064557F;

const uint32_t DialogPrimaryVtable = 0x006703FC;
uint32_t DialogListDerivedVtable = 0x006698C4;
uint32_t DialogListDerivedVirtualBaseVtable = 0x006698C0;
uint32_t DialogListVtable = 0x006693A4;
uint32_t DialogListVirtualBaseVtable = 0x006693A0;
const uint32_t DialogVirtualBaseFinalVtable = 0x006693AC;

uint32_t *DialogPublishedGlobal = reinterpret_cast<uint32_t *>(0x009B3374);

/*
Purpose: Destroy a Dialog. Install the Dialog table, run Dialog::close, then
         destroy the embedded StringStruct-derived list at this+0xBC - its
         derived close followed by its base StringStruct close, each staging
         the primary table and the virtual-base table named by the list's OWN
         vbtable, then walking and clearing the entry list - then destroy the
         list's virtual-base subobject (read its context word, install its
         final table, publish the word to the process global, in that order),
         and finally shut down the embedded Heap. The original's C++ exception
         frame targets __CxxFrameHandler and is omitted as unreachable per
         policy.
Original Offset: 00608E10
Return Value: n/a (the original leaves Heap::shutdown's EAX residue; the
              scalar deleting destructor overwrites it and the 116 direct
              callers ignore it, so void is faithful)
Status: Complete with temporary Dialog::close original dependency; the list
        walk and the Heap teardown are the source-owned
        StringStruct::close_with_tables/remove_all and Heap::shutdown.
Verification note: the sweep's one survivor swaps the context-word read at
virtual_base[1] past the final-table install at virtual_base[0]; the slots are
disjoint, so the original's read-before-install order is unobservable and the
mutant is equivalent by construction.
*/
void Dialog::destroy() {
    volatile uint32_t *const object = reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = DialogPrimaryVtable;
    (ORIGINAL(this)->*DialogOriginalClose)();

    // The derived-close chain at 0x004066C0, inlined by the original: each
    // stage resolves the virtual-base slot through the list's own vbtable at
    // run time (never the compile-time 0x24 - the RadioButton rule), and the
    // second stage's walk is a run-time no-op because the first emptied the
    // list.
    StringStruct *const list = reinterpret_cast<StringStruct *>(
        reinterpret_cast<uint8_t *>(this) + 0xBC);
    list->close_with_tables(DialogListDerivedVtable,
                            DialogListDerivedVirtualBaseVtable);
    list->close_with_tables(DialogListVtable, DialogListVirtualBaseVtable);

    // The list virtual base's subobject destructor. ??1Dialog is the
    // complete-object destructor, so the original addresses it at the fixed
    // most-derived this+0xE4 rather than through the vbtable; reproduce that.
    // Read the context word before installing the final table, then publish.
    volatile uint32_t *const virtual_base = reinterpret_cast<volatile uint32_t *>(
        reinterpret_cast<uint8_t *>(this) + 0xE4);
    const uint32_t published = virtual_base[1];
    virtual_base[0] = DialogVirtualBaseFinalVtable;
    *DialogPublishedGlobal = published;

    heap_.shutdown();
}

void __fastcall dialog_destructor_redirect(Dialog *self, void *) {
    self->destroy();
}

/*
Purpose: The compiler-generated scalar deleting destructor: run the complete
         destructor and free the storage through the executable's operator
         delete only when bit 0 of the mode asks. Always returns the object.
Original Offset: 00609D90
Return Value: the object
Status: Complete
*/
void *__fastcall dialog_scalar_dtor_redirect(
        Dialog *self, void *, unsigned int mode) {
    self->destroy();
    if (mode & 1) {
        DialogOperatorDelete(self);
    }
    return self;
}
