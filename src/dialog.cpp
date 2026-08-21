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
// ORIGINAL: 0x00609C60 ?set_dialog_font@Dialog@@QAEHPAUFont@@PAUFont@@PAUFont@@@Z 0x00609C60-0x00609C8D BYTE_EXACT
// symbol    ?set_dialog_font@Dialog@@QAEHPAVFont@@00@Z
// size      45 bytes
// prototype int (__thiscall ?set_dialog_font@Dialog@@QAEHPAUFont@@PAUFont@@PAUFont@@@Z)(Dialog* this, Font*, Font*, Font*)
// callers   11   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
// ORIGINAL: 0x00609C90 ?set_dialog_text_color@Dialog@@QAEXHHHH@Z 0x00609C90-0x00609CB8 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?set_dialog_text_color@Dialog@@QAEXHHHH@Z)(Dialog* this, int, int, int, int)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
// ORIGINAL: 0x00609CC0 ?set_dialog_text_color2@Dialog@@QAEXHHHH@Z 0x00609CC0-0x00609CEB BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_dialog_text_color2@Dialog@@QAEXHHHH@Z)(Dialog* this, int, int, int, int)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
// ORIGINAL: 0x00609CF0 ?set_dialog_text_color3@Dialog@@QAEXHHHH@Z 0x00609CF0-0x00609D1B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_dialog_text_color3@Dialog@@QAEXHHHH@Z)(Dialog* this, int, int, int, int)
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
// ORIGINAL: 0x00609AF0 ?id_to_pos@Dialog@@QAEHH@Z 0x00609AF0-0x00609B4B BYTE_EXACT
// size      91 bytes
// prototype int (__thiscall ?id_to_pos@Dialog@@QAEHH@Z)(Dialog* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Matching position, or the configured entry count on a miss
Status: Complete
*/
int Dialog::id_to_pos(int id) {
    if (entry_head_) {
        int count = entry_count_;
        entry_position_ = 0;
        current_entry_ = entry_head_;
        for (int traversed = 0; traversed < count; traversed++) {
            DialogEntry *entry = current_entry_;
            if (entry->id == id) {
                break;
            }
            entry_position_++;
            current_entry_ = entry->next;
        }
    }
    return entry_position_;
}

/*
Purpose: Select a dialog item by its bounded list ID.
// ORIGINAL: 0x006099D0 ?set_selected_id@Dialog@@QAEXH@Z 0x006099D0-0x00609A43
// size      115 bytes
// prototype void (__thiscall ?set_selected_id@Dialog@@QAEXH@Z)(Dialog* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Dialog::set_selected_id(int id) {
    selected_position_ = id_to_pos(id);
}

/*
Purpose: Restore the selected list position and return its item ID.
// ORIGINAL: 0x00609A50 ?get_selected_id@Dialog@@QAEHXZ 0x00609A50-0x00609AE6
// size      150 bytes
// prototype int (__thiscall ?get_selected_id@Dialog@@QAEHXZ)(Dialog* this)
// callers   15   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644F3A
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Selected item ID, or zero when the list head is null
Status: Complete
*/
int Dialog::get_selected_id() {
    // NO LOCAL FOR THE COUNT. The image reads it, decrements DESTRUCTIVELY
    // and never looks at that register again - it re-reads the member later
    // instead. Held in a local it stays live across the `abs` calls below
    // and VC6 spills, which is a stack frame the image has not got.
    int position = selected_position_;
    // `position <= count - 1`, WHICH IS NOT `position < count`. The image
    // decrements the count and compares - `dec eax; cmp edi, eax; jg` - and
    // for count == INT_MIN that decrement WRAPS, which is the whole reason
    // this tree carried an explicit INT_MIN test beside the comparison. The
    // wrap is the semantics; the test is a second read of it.
    if (position <= entry_count_ - 1) {
        current_entry_ = entry_head_;
        if (position < 0) {
            // `abs`, CALLED TWICE, and the count RE-READ between them: the
            // image is `push edi; call 0x644F3A; mov ecx, [esi+0xcc];
            // cmp eax, ecx; jg` and then `push edi; call 0x644F3A` again.
            // That is a MIN-style macro over `abs(position)`, not a local -
            // and `abs(INT_MIN)` returning INT_MIN is what the hand-rolled
            // ternary here was standing in for.
            if (abs(position) <= entry_count_) {
                int distance = abs(position);
                while (distance > 0) {
                    current_entry_ = current_entry_->previous;
                    distance--;
                }
                // The member again, not a local - see the note above the
                // guard. Plain `int` addition: the image is one `add`, and
                // the uint32 round trip through memcpy is a bit-cast VC6
                // will not inline away.
                position += entry_count_;
                // ONE STORE FOR BOTH ARMS. The image's negative arm ends
                // `add edi, [esi+0xcc]` and jumps STRAIGHT to the store the
                // positive arm falls into - `jmp` with no `mov` before it.
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
// ORIGINAL: 0x00609B50 ?pos_to_id@Dialog@@QAEHH@Z 0x00609B50-0x00609BEA
// size      154 bytes
// prototype int (__thiscall ?pos_to_id@Dialog@@QAEHH@Z)(Dialog* this, int)
// callers   7   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644F3A
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Item ID at the requested position, or zero when the list head is null
Status: Complete
*/
int Dialog::pos_to_id(int position) {
    int count = entry_count_;
    // `position <= count - 1`, WHICH IS NOT `position < count`. The image
    // decrements the count and compares - `dec eax; cmp edi, eax; jg` - and
    // for count == INT_MIN that decrement WRAPS, which is the whole reason
    // this tree carried an explicit INT_MIN test beside the comparison. The
    // wrap is the semantics; the test is a second read of it.
    if (position <= entry_count_ - 1) {
        current_entry_ = entry_head_;
        if (position < 0) {
            // `abs`, CALLED TWICE, and the count RE-READ between them: the
            // image is `push edi; call 0x644F3A; mov ecx, [esi+0xcc];
            // cmp eax, ecx; jg` and then `push edi; call 0x644F3A` again.
            // That is a MIN-style macro over `abs(position)`, not a local -
            // and `abs(INT_MIN)` returning INT_MIN is what the hand-rolled
            // ternary here was standing in for.
            if (abs(position) <= entry_count_) {
                int distance = abs(position);
                while (distance > 0) {
                    current_entry_ = current_entry_->previous;
                    distance--;
                }
                // The member again, not a local - see the note above the
                // guard. Plain `int` addition: the image is one `add`, and
                // the uint32 round trip through memcpy is a bit-cast VC6
                // will not inline away.
                position += entry_count_;
                // ONE STORE FOR BOTH ARMS. The image's negative arm ends
                // `add edi, [esi+0xcc]` and jumps STRAIGHT to the store the
                // positive arm falls into - `jmp` with no `mov` before it.
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

Font *DialogDefaultFonts[3];  // 0x009B8EC0

/*
Purpose: Set the default fonts shared by every dialog.
// ORIGINAL: 0x00609D20 ?set_def_dialog_font@Dialog@@QAAHPAUFont@@PAUFont@@PAUFont@@@Z 0x00609D20-0x00609D50 BYTE_EXACT
// symbol    ?set_def_dialog_font@Dialog@@SAHPAVFont@@00@Z
// size      48 bytes
// prototype 
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int Dialog::set_def_dialog_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    // Only an initialised primary is published; the other two are stored
    // either way and the call still succeeds. WRITTEN OUT, not published
    // through a volatile alias - see `BasePop::set_def_string_font`, which
    // is the same body and was held off the ratchet by the same helper.
    if (font1->is_initialized()) {
        DialogDefaultFonts[0] = font1;
    }
    DialogDefaultFonts[1] = font2;
    DialogDefaultFonts[2] = font3;
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
// ORIGINAL: 0x00608E10 ??1Dialog@@QAE@XZ 0x00608E10-0x00608F41;0x00662EC0-0x00662EEE
// size      351 bytes
// prototype void (__thiscall ??1Dialog@@QAE@XZ)(Dialog* this)
// callers   116   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00402970 0x005D45B0 0x00608F50
// indirect  0x00608EB5 0x00608ECA 0x00608EE9
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
// ORIGINAL: 0x00609D90 ??_GDialog@@UAEPAXI@Z 0x00609D90-0x00609DAE
// symbol    ?dialog_scalar_dtor_redirect@@YIPAXPAVDialog@@PAXI@Z
// size      30 bytes
// prototype void* (__thiscall ??_GDialog@@UAEPAXI@Z)(Dialog* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00608E10 0x0064557F
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





/*
 * The five-argument init, on the same terms as the three above: a forwarder,
 * not a recovery. The recovered SpriteBox::init (0x006104D0) tail-forwards to
 * it, so the DLL cannot link without a definition.
 */
