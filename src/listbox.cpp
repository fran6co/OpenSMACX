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
#include "listbox.h"
#include "vtable_shim.h"
#include "spritebox.h"
#include "net_class.h"
#include "mapwin.h"

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x0060C7C0 ?on_dialog_focus@ListBox@@QAEXH@Z 0x0060C7C0-0x0060C7C3 BYTE_EXACT
// symbol    ?on_dialog_focus@ListBox@@UAEXH@Z
// size      3 bytes
// prototype void (__thiscall ?on_dialog_focus@ListBox@@QAEXH@Z)(ListBox* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ListBox::on_dialog_focus(int) {
}


// Independent DWORD reads at 609E6F and 609F55 feed ListBox+28.
// RECOVERY_OBJECT: v2 0x009B8EE0 0x009B8EE4 ?ListBoxCloseDynamicDefault@@3IA 0x00609DB0
uint32_t ListBoxCloseDynamicDefault;
// RECOVERY_OBJECT: v2 0x009B8EE4 0x009B8EE8 ?CurrentListBoxDraw@@3PAVListBox@@A 0x004072D0
ListBox *CurrentListBoxDraw = 0;
// RECOVERY_OBJECT: v2 0x009B8EE8 0x009B8EEC ?ListBoxDrawOffset@@3HA 0x0060B530
int ListBoxDrawOffset = 0;
// Destination-based names: the semantic meaning of these fields is unproved.
// Original 609F4C/5E/67/70 read independent DWORDs into fields24/2C/10/14.
// RECOVERY_OBJECT: v2 0x006970E0 0x006970E4 ?ListBoxDefault24@@3IA 0x00609F20
uint32_t ListBoxDefault24 = 247;
// RECOVERY_OBJECT: v2 0x006970E4 0x006970E8 ?ListBoxDefault2C@@3IA 0x00609F20
uint32_t ListBoxDefault2C = 2;
// RECOVERY_OBJECT: v2 0x006970E8 0x006970EC ?ListBoxDefault10@@3IA 0x00609F20
uint32_t ListBoxDefault10 = -1;
// RECOVERY_OBJECT: v2 0x006970EC 0x006970F0 ?ListBoxDefault14@@3IA 0x00609F20
uint32_t ListBoxDefault14 = -1;
int ListBoxClickGuard;                // 0x009B8EEC

// The three subobject vtables the teardown re-staged (0x0067041C/0x00670414/
// 0x00670408) left with the re-stores, same as the Dialogs stage tables.

/*
Purpose: Close the ListBox: close its GraphicWin and Dialog virtual bases,
         both located
         through the runtime vbtable, then reset the ListBox-owned fields from
         the process defaults and re-arm Dialog::field_B4_.
// ORIGINAL: 0x00609F20 ?close@ListBox@@QAEXXZ 0x00609F20-0x00609F9D BYTE_EXACT
// LEVER: dropped `volatile` from the `object` alias and from the field_B4_ store: 35/37 -> BYTE_EXACT 37/37. The volatile was an ALIAS, not a store-keeper - every store here is to the live `this`, so VC6 keeps them all without it, and the only thing volatile bought was an ordering fence that stopped VC6 hoisting the vbtable load `mov ecx, [esi]` ahead of the `mov [esi + 0x14], edx` store the way the image does. Measured: volatile object + plain B4 store is still 35/37, so it is the object alias that costs the two instructions. Also measured and rejected, all while `volatile` was still in place: hoisting the vbtable pointer into a local between the 0x10 and 0x14 stores (35/37), hoisting it to the top of the store block (35/37), and hoisting the whole vbase displacement instead of the pointer (32/37) - scheduling, not spelling, was the difference.
// symbol    ?close@ListBox@@QAEIXZ
// size      125 bytes
// prototype void (__thiscall ?close@ListBox@@QAEXXZ)(ListBox* this)
// callers   35   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
Return Value: EAX residue (constant 0). Declared void (?close@ListBox@@QAEXXZ);
              modelled as uint32_t to preserve the residue like GraphicWin::close.
The two base close methods are reusable source-owned operations.
*/
uint32_t ListBox::close() {
    // Base offsets come from the object's OWN vbtable, read at run time -
    // never the compile-time 0x48/0xA60 - so an embedded ListBox with a
    // different vbtable still reaches the correct subobjects (AGENTS.md
    // RadioButton rule). The qualified calls give that runtime dispatch:
    // the compiler's virtual-base member call reads the vbtable exactly
    // where the old hand walks read it.
    // GraphicWin virtual base: source-owned close at 0x005D4E40. Return
    // discarded.
    GraphicWin::close();
    // Dialog virtual base at 0x00608F50, reached through its real method.
    Dialog::close();

    // ListBox-owned resets read the source-owned process defaults.
    // The Dialog::field_B4_ = 1 store
    // below is reached through the runtime vbtable displacement, as the
    // image does - its access-and-naming pass is the recorded follow-up.
    field_4_ = 0;
    field_8_ = 0;
    field_18_ = 0;
    field_1C_ = 0;
    field_20_ = 0;
    field_24_ = ListBoxDefault24;
    field_28_ = ListBoxCloseDynamicDefault;     // 0x009B8EE0
    field_2C_ = ListBoxDefault2C;
    field_10_ = ListBoxDefault10;
    field_14_ = ListBoxDefault14;
    // Dialog::field_B4_ = 1, re-arming it through the virtual base as a
    // plain member write.
    field_B4_ = 1;
    field_30_ = 0;
    field_34_ = 0;
    field_38_ = 0;
    selection_callback_ = 0;
    double_click_callback_ = 0;
    field_C_ = 0;
    return 0;
}

// Original 609DB0 conditionally constructs GraphicWin and Dialog using the
// compiler's most-derived flag, then initializes the ListBox-owned fields.
// ORIGINAL: 0x00609DB0 ??0ListBox@@QAE@H@Z 0x00609DB0-0x00609EB7;0x00662EF0-0x00662F14 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00609db0.cpp", "Original constructor/destructor direct-call receivers and unwind stages"], "reason": "609DB0 calls GraphicWin and Dialog constructors only for the hidden most-derived ABI flag, then stores own defaults. Genuine virtual bases synthesize this flag; it is not an explicit int source parameter."}
// symbol    ??0ListBox@@QAE@XZ
ListBox::ListBox() {
    field_4_ = 0;
    field_8_ = 0;
    field_10_ = ListBoxDefault10;
    field_14_ = ListBoxDefault14;
    field_18_ = 0;
    field_1C_ = 0;
    field_20_ = 0;
    field_24_ = ListBoxDefault24;
    field_28_ = ListBoxCloseDynamicDefault;
    field_2C_ = ListBoxDefault2C;
    field_30_ = 0;
    field_34_ = 0;
    field_38_ = 0;
    selection_callback_ = 0;
    double_click_callback_ = 0;
    field_B4_ = 1;
    field_C_ = 0;
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to ListBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         ListBox::on_dialog_focus overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
         MOVED HERE from filewin.cpp 2026-09-03, with ListBox's constructor
         going declaration-only above: these COMDATs are emitted in every TU
         that CONSTRUCTS the object, and this file now is one - the three
         markers had already predicted the move ("NOT IN listbox.cpp, which
         defines no ListBox constructor"), which stopped being true.
// ORIGINAL: 0x0060D000 ?on_dialog_focus@thunk1_ListBox@@QAEXH@Z 0x0060D000-0x0060D008 BYTE_EXACT
// symbol    ?on_dialog_focus@ListBox@@$4PPPPPPPM@A@AEXH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Adjust the receiver from the thunk1 subobject back to ListBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         ListBox::on_mouse_leave overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
         MOVED HERE from filewin.cpp 2026-09-03, for the same reason as
         on_dialog_focus's marker directly above.
// ORIGINAL: 0x0060CF80 ?on_mouse_leave@thunk1_ListBox@@QAEXHH@Z 0x0060CF80-0x0060CF88 BYTE_EXACT
// symbol    ?on_mouse_leave@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Adjust the receiver from the thunk1 subobject back to ListBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         ListBox::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
         MOVED HERE from filewin.cpp 2026-09-03, for the same reason as
         on_dialog_focus's marker directly above.
// ORIGINAL: 0x0060D040 ?attach@thunk1_ListBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060D040-0x0060D048 BYTE_EXACT
// symbol    ?attach@ListBox@@$4PPPPPPPM@A@AEHPAVGraphicWin@@HHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

uint32_t ListBox::destroy() {
    // Strip-all direction: the image's five derived-stage re-stores here are
    // gone; the object runs on whatever its constructing chain installed.
    return close();
}


/*
Purpose: The ListBox derived destructor stage. VC6 enters this method on the
         GraphicWin controlling virtual base, recovers the class receiver,
         stages its tables and vtordisps, and calls close. The complete-object
         caller additionally destroys the virtual bases in reverse order.
// ORIGINAL: 0x00609EC0 ??1ListBox@@QAE@XZ 0x00609EC0-0x00609F13 BYTE_EXACT
// symbol    ??1ListBox@@UAE@XZ
// size      83 bytes
// prototype void (__thiscall ??1ListBox@@QAE@XZ)(ListBox* this)
// callers   35   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: EAX residue (close()'s residue, constant 0).
*/
ListBox::~ListBox() {
    close();
}

// Legacy leaf callers still use this partial-stage callback. Their enclosing
// lifetime paths require coupled recovery; calling an explicit complete C++
// destructor here would also destroy virtual bases which they destroy later.
uint32_t __fastcall list_box_destructor_redirect(void *adjusted, void *) {
    // Strip-all direction: the image's five derived-stage re-stores here -
    // the content this BYTE_EXACT claim used to bank - are gone, as in
    // ListBox::destroy above. What remains is the walked-back close().
    ListBox *const self = reinterpret_cast<ListBox *>(
        static_cast<uint8_t *>(adjusted) - ListBoxDestructorAdjustment);
    return self->close();
}

/*
Purpose: Clear the hover index and repaint, through the enclosing object.
// ORIGINAL: 0x0060CE10 ?on_mouse_leave@ListBox@@QAEXHH@Z 0x0060CE10-0x0060CE38 BYTE_EXACT
// symbol    ?on_mouse_leave@ListBox@@UAEXHH@Z
// size      40 bytes
// prototype void (__thiscall ?on_mouse_leave@ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060CE2F
Return Value: n/a
Status: Complete
*/
void ListBox::on_mouse_leave(int a1, int a2) {
    // Clear the hover index (Dialog::field_F0_, through the virtual base)
    // and repaint (the redraw virtual) - plain member forms; the
    // compiler's virtual-base machinery does the walking the old hand
    // chains spelled. Both parameters are dead; `ret 8` still pops them.
    field_F0_ = -1;
    redraw();
}

// The retained seed at 0x0060A020 also supplies the notify-gated calls: the
// GraphicWin base's Win::win_parent_ receives slots 54/53, then its numbered
// AutoSound window at +0x90 receives slots 23/7. These typed owners and
// receiver slots follow the seed's vbtable walk and callsites.
// ORIGINAL: 0x0060A020 ?set_selected_pos@ListBox@@QAEXHH@Z 0x0060A020-0x0060A1DD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0060a020.cpp", "source:src/listbox.h", "source:src/dialog.h", "source:src/win.h", "source:src/autosound.h"], "reason": "Recover ListBox selection, viewport and notify-gated item callbacks through its typed Dialog, GraphicWin and Win owners. The retained seed's vbtable walk resolves to Win::win_parent_ and its AutoSound val_36_ window; preserve the original seed and historical claim."}
void ListBox::set_selected_pos(int position, int notify) {
    if (Dialog::selected_position_ == position && field_8_ == 0) {
        redraw();
        return;
    }

    Dialog::selected_position_ = position;
    if (position >= 0) {
        const int page_size = static_cast<int>(field_1C_);
        const int item_count = Dialog::item_list_.entry_count_;
        const int step = Dialog::field_54_ < 2
            ? 1 : static_cast<int>(Dialog::field_58_);
        int scroll_position = static_cast<int>(field_18_);
        bool moved_up = false;
        bool update_scrollbar = false;

        while (Dialog::selected_position_ < scroll_position) {
            scroll_position -= step;
            if (scroll_position < 0) {
                scroll_position = 0;
            }
            moved_up = true;
        }

        if (page_size < item_count) {
            while (Dialog::selected_position_ >= scroll_position + page_size) {
                scroll_position += step;
            }
            update_scrollbar = true;
        } else {
            scroll_position = 0;
            update_scrollbar = moved_up;
        }

        field_18_ = static_cast<uint32_t>(scroll_position);
        if (update_scrollbar) {
            Win *window = static_cast<GraphicWin *>(this);
            if (Dialog::field_54_ < 2) {
                window->set_vert_pos(scroll_position);
            } else {
                window->set_horz_pos(scroll_position);
            }
        }

        if (double_click_callback_ != 0) {
            double_click_callback_(Dialog::get_selected_id());
        }
        if (notify != 0) {
            Win *parent = static_cast<GraphicWin *>(this)->parent_window();
            if (parent != 0) {
                parent->vslot_54(Dialog::pos_to_id(position));
            }

            parent = static_cast<GraphicWin *>(this)->parent_window();
            if (parent != 0) {
                parent->vslot_53(Dialog::pos_to_id(position), 0);
            }

            parent = static_cast<GraphicWin *>(this)->parent_window();
            if (parent != 0) {
                Win *child = parent->legacy_window_36();
                if (child != 0 && child->vslot_23() == 0) {
                    child->vslot_07();
                }
            }
        }
    }

    // The seed's final call is Win virtual slot 62 on the GraphicWin base;
    // this is the inherited typed redraw operation.
    redraw();
}

/*
Purpose: Report a right press: raise the click guard, forward (a1, a2) through
         GraphicWin vtable slot 20, drop the guard.

/*
Purpose: Report a right press: raise the click guard, forward (a1, a2) through
         GraphicWin vtable slot 20, drop the guard.
// ORIGINAL: 0x0060AA20 ?on_right_down@ListBox@@QAEXHH@Z 0x0060AA20-0x0060AA52
// TRIED 2026-09-03: re-dumped the divergence. The 3 instructions the image
// has and this tree does not are entirely the entered-receiver walk
// (`mov eax,[ecx-0x48]` / `mov eax,[edx+ecx-0x48]` / `lea ecx,[edx+ecx-0x48]`
// against this tree's `mov eax,[ecx]` / `mov eax,[ecx+edx]` /
// `add ecx,edx`) - the front walk through the vbtable that VC6 emits only
// for a U-spelled virtual override entered on the GraphicWin virtual base.
// No body spelling reaches it while the receiver enters adjusted; it needs
// the ListBox/GraphicWin declaration (on_right_down virtual in GraphicWin),
// which is batch 9's entered-receiver work and regresses the six
// window-event families whose image symbols spell Q. Same wall on
// on_scrolled and on_right_double_click below.
// TRIED: byte-exact on promotion from the 0060aa20 archive unit. The
//   Q spelling enters with `this` on the GraphicWin virtual base, so the
//   ListBox front is walked back with the same explicit -0x48 the image folds
// TRIED: byte-exact on promotion from the 0060aa20 archive unit. The
//   Q spelling enters with `this` on the GraphicWin virtual base, so the
//   ListBox front is walked back with the same explicit -0x48 the image folds
//   (`mov eax, [ecx - 0x48]`, receiver `lea ecx, [edx + ecx - 0x48]`) - the
//   on_left_click idiom in mapwin.cpp, not on_mouse_leave's virtual-entry
//   walk, which VC6 only generates for a U-spelled override. The vbtable read
//   stays INSIDE the receiver expression: naming it a local lets VC6
//   materialize the front pointer the image keeps folded away.
// size      50 bytes
// prototype void (__thiscall ?on_right_down@ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060AA42
Return Value: n/a
Status: Complete
DEMOTED from BYTE_EXACT by direction (idiomatic-first): the forward is the
plain virtual call it always was - win-table slot 20, on_left_down - dispatched
on the entered receiver, which already points at the GraphicWin subobject
whose vftable the old hand walk re-derived through the front. The image
walks back to the ListBox front and re-walks out to that same table
(`mov eax,[ecx-0x48]`, 9 of 12, 0.917 similar); reproducing that addressing
is the entered-receiver relationship work (batch 9). The dispatch target is
identical either way.
*/
void ListBox::on_right_down(int a1, int a2) {
    ListBoxClickGuard = 1;
    on_left_down(a1, a2);
    ListBoxClickGuard = 0;
}

/*
Purpose: Forward a vertical-scroll (code, pos) through GraphicWin vtable
         slot 49.
// ORIGINAL: 0x0060C6A0 ?on_scrolled@ListBox@@QAEXHH@Z 0x0060C6A0-0x0060C6C1
// TRIED: byte-exact on promotion, same entered-adjusted receiver as
//   on_right_down above; no guard on this one.
// size      33 bytes
// prototype void (__thiscall ?on_scrolled@ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060C6B8
Return Value: n/a
Status: Complete
DEMOTED from BYTE_EXACT by direction (idiomatic-first): vslot_49 is the
plain virtual call; the image's front-walk addressing is the entered-
receiver relationship work (see on_right_down's note).
*/
void ListBox::on_scrolled(int code, int pos) {
    vslot_49(code, pos);
}

/*
Purpose: Report a right double click: raise the click guard, forward (a1, a2)
         through GraphicWin vtable slot 28, drop the guard.
// ORIGINAL: 0x0060C6D0 ?on_right_double_click@ListBox@@QAEXHH@Z 0x0060C6D0-0x0060C702
// TRIED: byte-exact on promotion, same entered-adjusted receiver as
//   on_right_down above.
// size      50 bytes
// prototype void (__thiscall ?on_right_double_click@ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060C6F2
Return Value: n/a
Status: Complete
DEMOTED from BYTE_EXACT by direction (idiomatic-first): on_left_double_click is the
plain virtual call; the image's front-walk addressing is the entered-
receiver relationship work (see on_right_down's note).
*/
void ListBox::on_right_double_click(int a1, int a2) {
    ListBoxClickGuard = 1;
    on_left_double_click(a1, a2);
    ListBoxClickGuard = 0;
}

// Shared ListBox globals the 0060c9b0/0060c920/0060a1e0 bodies read. The
// 0x009B8DD4 word remains an unresolved fixed alias; 0x009B8DDC is the
// shared scroll-notify gate used by these ListBox bodies and Scroll events.
static int *const ListBoxGlobalDD4 = (int *)0x009B8DD4;
// ORIGINAL storage binding: Scroll input methods clear the gate before
// notifying their parent; the ListBox scrolled path reads it before changing
// selection.
// RECOVERY_OBJECT: v1 0x009B8DDC ?ListBoxGlobalDDC@@3HA
int ListBoxGlobalDDC;

/*
Purpose: Recompute the list's layout metrics from the Dialog entry list: the
         widest entry text sets the width, the entry height and column count
         follow, and the two overflow bits in field_C_ report a column or row
         shortfall the caller reacts to.
// ORIGINAL: 0x0060A1E0 ?calculate_dimensions@ListBox@@QAEHXZ 0x0060A1E0-0x0060A66B FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0060a1e0.cpp", "source:src/listbox.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU; the retained seed stays beside the landed transcription."}
// TRIED: direct named-member access on dialog_/virtual_base_ instead of the
//        original's dynamic vbtable-index lookup (vtable[8]=dialog_
//        displacement, vtable[4]=GraphicWin displacement, confirmed by
//        matching field offsets 0x7c-0xa8 to Dialog's text_color/font fields
//        and 0xc4-0xd0 to its entry list). Diverges at #2 (compiler emits a
//        fixed-offset load, not the indirection).
// size      1163 bytes
// prototype int (__thiscall ?calculate_dimensions@ListBox@@QAEHXZ)(ListBox* this)
// calls     0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DC790 0x005DCAB0
*/
int ListBox::calculate_dimensions() {
    char *self = reinterpret_cast<char *>(this);
    int *vbt = *reinterpret_cast<int **>(self);
    char *gw_base = self + vbt[1];
    char *dlg_base = self + vbt[2];
    Buffer *buf = reinterpret_cast<Buffer *>(gw_base + 0x444);

    if (*reinterpret_cast<uint32_t *>(dlg_base + 0xcc) == 0) {
        return 7;
    }
    field_C_ = 0;

    buf->set_text_color(*reinterpret_cast<int *>(dlg_base + 0x7c),
                        *reinterpret_cast<int *>(dlg_base + 0x88),
                        *reinterpret_cast<int *>(dlg_base + 0x94),
                        *reinterpret_cast<int *>(dlg_base + 0xa0));
    buf->set_text_color2(*reinterpret_cast<int *>(dlg_base + 0x80),
                         *reinterpret_cast<int *>(dlg_base + 0x8c),
                         *reinterpret_cast<int *>(dlg_base + 0x98),
                         *reinterpret_cast<int *>(dlg_base + 0xa4));
    buf->set_text_color3(*reinterpret_cast<int *>(dlg_base + 0x84),
                         *reinterpret_cast<int *>(dlg_base + 0x90),
                         *reinterpret_cast<int *>(dlg_base + 0x9c),
                         *reinterpret_cast<int *>(dlg_base + 0xa8));
    buf->set_font(*reinterpret_cast<Font **>(dlg_base + 0xac),
                  *reinterpret_cast<Font **>(dlg_base + 0xb0),
                  *reinterpret_cast<Font **>(dlg_base + 0xb4), 0);

    int visible_rows = *reinterpret_cast<int *>(dlg_base + 0x50);
    int rows = *reinterpret_cast<int *>(dlg_base + 0xcc);
    if (visible_rows <= rows) {
        rows = visible_rows;
    }
    *reinterpret_cast<int *>(dlg_base + 0x58) = rows;

    int max_width = 0;
    int n = 0;
    if (*reinterpret_cast<int *>(dlg_base + 0xcc) > 0) {
        do {
            char *name;
            int head = *reinterpret_cast<int *>(dlg_base + 0xc4);
            if (head != 0) {
                int current = *reinterpret_cast<int *>(dlg_base + 0xc8);
                current = *reinterpret_cast<int *>(current + 0xc);
                *reinterpret_cast<int *>(dlg_base + 0xc8) = current;
                int pos = *reinterpret_cast<int *>(dlg_base + 0xd0) + 1;
                *reinterpret_cast<int *>(dlg_base + 0xd0) = pos;
                if (pos == *reinterpret_cast<int *>(dlg_base + 0xcc)) {
                    *reinterpret_cast<int *>(dlg_base + 0xd0) = 0;
                }
            }
            if (*reinterpret_cast<int *>(dlg_base + 0xc8) == 0) {
                name = 0;
            } else if (head == 0) {
                name = *reinterpret_cast<char **>(4);
            } else {
                name = *reinterpret_cast<char **>(
                    *reinterpret_cast<int *>(
                        *reinterpret_cast<int *>(dlg_base + 0xc8) + 8)
                    + 4);
            }
            int width = buf->text_width(name);
            if (max_width < width) {
                max_width = width;
            }
            ++n;
        } while (n < *reinterpret_cast<int *>(dlg_base + 0xcc));
        if (max_width != 0) {
            goto have_width;
        }
    }
    max_width = *reinterpret_cast<int *>(dlg_base + 0x2c);

have_width:
    *reinterpret_cast<int *>(dlg_base + 0x64) =
        *reinterpret_cast<int *>(dlg_base + 0x44) + 2 + field_2C_ * 2
        + *reinterpret_cast<int *>(dlg_base + 0x34) + max_width;

    if (*reinterpret_cast<int *>(dlg_base + 0x48) == -1) {
        *reinterpret_cast<int *>(dlg_base + 0x68) =
            buf->text_line_height() + *reinterpret_cast<int *>(dlg_base + 0x44);
    } else {
        *reinterpret_cast<int *>(dlg_base + 0x68) =
            *reinterpret_cast<int *>(dlg_base + 0x48);
    }

    int tmp;
    if ((*reinterpret_cast<unsigned char *>(dlg_base + 0x20) & 2) == 0
        || (tmp = *reinterpret_cast<int *>(dlg_base + 0x2c), tmp == 0)) {
        int divisor = *reinterpret_cast<int *>(dlg_base + 0x4c);
        if (divisor == 1) {
            int width = *reinterpret_cast<int *>(dlg_base + 0x64)
                + *reinterpret_cast<int *>(dlg_base + 0x40) * 2;
            int min_width = *reinterpret_cast<int *>(dlg_base + 0x2c);
            if (width < min_width) {
                width = min_width;
            }
            *reinterpret_cast<int *>(dlg_base + 0x5c) = width;
            *reinterpret_cast<int *>(dlg_base + 0x64) =
                *reinterpret_cast<int *>(dlg_base + 0x5c)
                - *reinterpret_cast<int *>(dlg_base + 0x40) * 2;
        } else {
            int margin = *reinterpret_cast<int *>(dlg_base + 0x44);
            *reinterpret_cast<int *>(dlg_base + 0x64) =
                (*reinterpret_cast<int *>(dlg_base + 0x5c)
                 - *reinterpret_cast<int *>(dlg_base + 0x40) * 2 + margin)
                / divisor - margin;
            int width = *reinterpret_cast<int *>(dlg_base + 0x44);
            width = ((*reinterpret_cast<int *>(dlg_base + 0x64) + width)
                     * *reinterpret_cast<int *>(dlg_base + 0x4c)
                     + *reinterpret_cast<int *>(dlg_base + 0x40) * 2) - width;
            int min_width = *reinterpret_cast<int *>(dlg_base + 0x2c);
            if (width < min_width) {
                width = min_width;
            }
            *reinterpret_cast<int *>(dlg_base + 0x5c) = width;
        }
        *reinterpret_cast<int *>(dlg_base + 0x54) =
            *reinterpret_cast<int *>(dlg_base + 0x4c);
    } else {
        *reinterpret_cast<int *>(dlg_base + 0x5c) = tmp;
        *reinterpret_cast<int *>(dlg_base + 0x54) =
            *reinterpret_cast<int *>(dlg_base + 0x4c);
        if (*reinterpret_cast<int *>(dlg_base + 0x54) == 1) {
            *reinterpret_cast<int *>(dlg_base + 0x64) =
                *reinterpret_cast<int *>(dlg_base + 0x5c)
                - *reinterpret_cast<int *>(dlg_base + 0x40) * 2;
        } else {
            *reinterpret_cast<int *>(dlg_base + 0x64) =
                *reinterpret_cast<int *>(dlg_base + 0x64)
                - *reinterpret_cast<int *>(dlg_base + 0x44);
        }
    }

    if ((*reinterpret_cast<unsigned char *>(dlg_base + 0x20) & 1) == 0
        || (tmp = *reinterpret_cast<int *>(dlg_base + 0x30), tmp == 0)) {
        *reinterpret_cast<int *>(dlg_base + 0x60) =
            (*reinterpret_cast<int *>(dlg_base + 0x68)
             * *reinterpret_cast<int *>(dlg_base + 0x58)
             - *reinterpret_cast<int *>(dlg_base + 0x44))
            + *reinterpret_cast<int *>(dlg_base + 0x3c)
            + *reinterpret_cast<int *>(dlg_base + 0x38);
    } else {
        *reinterpret_cast<int *>(dlg_base + 0x60) = tmp;
        *reinterpret_cast<int *>(dlg_base + 0x58) =
            ((*reinterpret_cast<int *>(dlg_base + 0x60)
              - *reinterpret_cast<int *>(dlg_base + 0x3c))
             - *reinterpret_cast<int *>(dlg_base + 0x38)
             + *reinterpret_cast<int *>(dlg_base + 0x44))
            / *reinterpret_cast<int *>(dlg_base + 0x68);
        if ((*reinterpret_cast<uint32_t *>(gw_base + 0x98) & 0x40) == 0) {
            int columns = *reinterpret_cast<int *>(dlg_base + 0x54);
            if (*reinterpret_cast<int *>(dlg_base + 0x58) * columns
                < static_cast<int>(field_20_)) {
                if (columns < 2) {
                    *reinterpret_cast<int *>(dlg_base + 0x5c) =
                        *reinterpret_cast<int *>(dlg_base + 0x2c) - *ListBoxGlobalDD4;
                    *reinterpret_cast<int *>(dlg_base + 0x54) = 1;
                } else {
                    *reinterpret_cast<int *>(dlg_base + 0x60) =
                        *reinterpret_cast<int *>(dlg_base + 0x30) - *ListBoxGlobalDD4;
                    *reinterpret_cast<int *>(dlg_base + 0x58) =
                        *reinterpret_cast<int *>(dlg_base + 0x60)
                        / *reinterpret_cast<int *>(dlg_base + 0x68);
                }
            }
        }
    }

    *reinterpret_cast<int *>(dlg_base + 0x68) =
        *reinterpret_cast<int *>(dlg_base + 0x68)
        - *reinterpret_cast<int *>(dlg_base + 0x44);
    int margin = *reinterpret_cast<int *>(dlg_base + 0x44);
    if (*reinterpret_cast<int *>(dlg_base + 0xcc)
        <= ((*reinterpret_cast<int *>(dlg_base + 0x60)
             - *reinterpret_cast<int *>(dlg_base + 0x3c))
            - *reinterpret_cast<int *>(dlg_base + 0x38) + margin)
        / (*reinterpret_cast<int *>(dlg_base + 0x68) + margin)) {
        if ((*reinterpret_cast<unsigned char *>(dlg_base + 0x20) & 2) == 0) {
            *reinterpret_cast<int *>(dlg_base + 0x64) =
                *reinterpret_cast<int *>(dlg_base + 0x5c)
                - *reinterpret_cast<int *>(dlg_base + 0x40) * 2;
        } else {
            *reinterpret_cast<int *>(dlg_base + 0x64) =
                (*reinterpret_cast<int *>(dlg_base + 0x5c)
                 - *reinterpret_cast<int *>(dlg_base + 0x40) * 2 + margin)
                / *reinterpret_cast<int *>(dlg_base + 0x54) - margin;
        }
    }

    int total = *reinterpret_cast<int *>(dlg_base + 0x58)
        * *reinterpret_cast<int *>(dlg_base + 0x54);
    field_1C_ = static_cast<uint32_t>(total);
    if (total < static_cast<int>(field_20_)) {
        if (1 < *reinterpret_cast<int *>(dlg_base + 0x54)) {
            Win *win = reinterpret_cast<Win *>(gw_base);
            if ((*reinterpret_cast<uint32_t *>(gw_base + 0x98) & 0x40) == 0) {
                *reinterpret_cast<int *>(dlg_base + 0x60) =
                    *reinterpret_cast<int *>(dlg_base + 0x60) + *ListBoxGlobalDD4;
            }
            field_C_ = field_C_ | 1;
            return 0;
        }
        if ((*reinterpret_cast<uint32_t *>(gw_base + 0x98) & 0x40) == 0) {
            *reinterpret_cast<int *>(dlg_base + 0x5c) =
                *reinterpret_cast<int *>(dlg_base + 0x5c) + *ListBoxGlobalDD4;
        }
        if ((*reinterpret_cast<unsigned char *>(dlg_base + 0x20) & 2) != 0) {
            *reinterpret_cast<int *>(dlg_base + 0x64) =
                *reinterpret_cast<int *>(dlg_base + 0x64)
                - *ListBoxGlobalDD4 / *reinterpret_cast<int *>(dlg_base + 0x54);
        }
        field_C_ = field_C_ | 2;
    }
    return 0;
}

/*
Purpose: Attach the list to a parent window rectangle: run the layout pass,
         transfer the style flags (handing the horizontal/vertical scroll
         choice to the caller's bits), init the GraphicWin frame over the
         given position, prime the scroll ranges and paging from the entry
         metrics, land the scroll position, and copy the Dialog's colours and
         fonts into the window's buffer.
// ORIGINAL: 0x0060A670 ?attach@ListBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060A670-0x0060AA15 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0060a670.cpp", "source:src/listbox.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU; the retained seed stays beside the landed transcription."}
// symbol    ?attach@ListBox@@UAEHPAXHHH@Z
// TRIED: raw self+/-offset pointer chasing throughout (this points 0xa60 into
//        the ListBox object; base=self-0xa60 is the real ListBox start reached
//        via calculate_dimensions()); re-deriving the vbtable-style P1/P2
//        pointers fresh at each use instead of caching, to match the original's
//        reloads across calls. Reaches MISMATCH #44 'lea' vs 'add'.
// size      933 bytes
// prototype int (__thiscall ?attach@ListBox@@QAEHPAUGraphicWin@@HHH@Z)(ListBox* this, GraphicWin*, int, int, int)
// calls     0x005D4EF0 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005EE030 0x005EE070 0x005EE0B0 0x005EE0D0 0x005EE0F0 0x005EE110 0x0060A1E0
*/
int ListBox::attach(GraphicWin *a1, int a2, int a3, int a4) {
    if (a1 == 0) {
        return 3;
    }
    if (HandleMain == 0) {
        return 7;
    }

    char *self = reinterpret_cast<char *>(this);
    // attach is entered on the Dialog virtual base: the ListBox front is
    // 0xA60 below.
    char *front = self - 0xA60;

    int flags = (a4 & 0xfffffff3) | 0x20;
    {
        int *vbt = *reinterpret_cast<int **>(front);
        *reinterpret_cast<int *>(self + vbt[1] - 0x9C8) = flags;
    }

    int dim_result = reinterpret_cast<ListBox *>(front)->calculate_dimensions();
    if (dim_result != 0) {
        if (dim_result != 7) {
            return dim_result;
        }
        {
            int *vbt = *reinterpret_cast<int **>(front);
            char *dlg_base = self + vbt[2];
            *reinterpret_cast<int *>(dlg_base - 0xA04)
                = *reinterpret_cast<int *>(dlg_base - 0xA34);
        }
        {
            int *vbt = *reinterpret_cast<int **>(front);
            char *dlg_base = self + vbt[2];
            *reinterpret_cast<int *>(dlg_base - 0xA00)
                = *reinterpret_cast<int *>(dlg_base - 0xA30);
        }
        {
            int *vbt = *reinterpret_cast<int **>(front);
            char *dlg_base = self + vbt[2];
            *reinterpret_cast<int *>(dlg_base - 0xA0C) = 1;
        }
        {
            int *vbt = *reinterpret_cast<int **>(front);
            char *dlg_base = self + vbt[2];
            *reinterpret_cast<int *>(dlg_base - 0xA08) = 1;
        }
    }

    {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        *reinterpret_cast<int *>(dlg_base - 0xA3C) = a2;
    }
    {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        *reinterpret_cast<int *>(dlg_base - 0xA38) = a3;
    }

    if (*reinterpret_cast<int *>(front + 0x20) > *reinterpret_cast<int *>(front + 0x1C)) {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        if (*reinterpret_cast<int *>(dlg_base - 0xA0C) > 1) {
            flags |= 4;
        } else {
            flags |= 8;
        }
    }

    {
        int a1_flags = *reinterpret_cast<int *>(
            reinterpret_cast<char *>(a1) + 0x98);
        if ((a1_flags & 0x1000) == 0) {
            flags |= 0x100000;
        }
    }
    if (flags & 0x40) {
        flags |= 0x800;
    }

    {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        char *gw_base = self + vbt[1] - 0xA60;
        int init_result = reinterpret_cast<GraphicWin *>(gw_base)->init(
            a2, a3,
            *reinterpret_cast<int *>(dlg_base - 0xA00),
            *reinterpret_cast<int *>(dlg_base - 0xA04),
            0, flags, reinterpret_cast<Win *>(a1), 0, 0);
        if (init_result != 0) {
            return init_result;
        }
    }

    if (flags & 8) {
        int *vbt = *reinterpret_cast<int **>(front);
        Win *win = reinterpret_cast<Win *>(self + vbt[1] - 0xA60);
        win->set_vert_range(0,
            *reinterpret_cast<int *>(self + vbt[2] - 0x994) - 1);
        vbt = *reinterpret_cast<int **>(front);
        win = reinterpret_cast<Win *>(self + vbt[1] - 0xA60);
        win->set_vert_paging(
            *reinterpret_cast<int *>(self + vbt[2] - 0xA08) - 1);
    } else if (flags & 4) {
        int *vbt = *reinterpret_cast<int **>(front);
        int numerator = *reinterpret_cast<int *>(self + vbt[2] - 0x994);
        char *dlg_base = self + vbt[2];
        int quotient = numerator / *reinterpret_cast<int *>(dlg_base - 0xA08);
        Win *win = reinterpret_cast<Win *>(self + vbt[1] - 0xA60);
        win->set_horz_range(0, quotient - 1);
        vbt = *reinterpret_cast<int **>(front);
        win = reinterpret_cast<Win *>(self + vbt[1] - 0xA60);
        win->set_horz_paging(*reinterpret_cast<int *>(self + vbt[2] - 0xA0C));
    }

    {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        if (*reinterpret_cast<int *>(dlg_base - 0x994) == 0) {
            *reinterpret_cast<int *>(front + 0x18) = 0;
        } else {
            while (*reinterpret_cast<int *>(dlg_base - 0x974)
                   >= *reinterpret_cast<int *>(front + 0x1C)
                       + *reinterpret_cast<int *>(front + 0x18)) {
                int step;
                if (*reinterpret_cast<int *>(dlg_base - 0xA0C) > 1) {
                    step = *reinterpret_cast<int *>(dlg_base - 0xA08);
                } else {
                    step = 1;
                }
                *reinterpret_cast<int *>(front + 0x18) += step;

                vbt = *reinterpret_cast<int **>(front);
                dlg_base = self + vbt[2];
            }
        }
    }

    if (*reinterpret_cast<int *>(front + 0x20) > *reinterpret_cast<int *>(front + 0x1C)) {
        int *vbt = *reinterpret_cast<int **>(front);
        Win *win = reinterpret_cast<Win *>(self + vbt[1] - 0xA60);
        if (*reinterpret_cast<int *>(self + vbt[2] - 0xA0C) > 1) {
            win->set_horz_pos(*reinterpret_cast<int *>(front + 0x18));
        } else {
            win->set_vert_pos(*reinterpret_cast<int *>(front + 0x18));
        }
    }

    {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        Buffer *buf = reinterpret_cast<Buffer *>(self + vbt[1] - 0x61C);
        buf->set_text_color(*reinterpret_cast<int *>(dlg_base - 0x9E4),
                            *reinterpret_cast<int *>(dlg_base - 0x9D8),
                            *reinterpret_cast<int *>(dlg_base - 0x9CC),
                            *reinterpret_cast<int *>(dlg_base - 0x9C0));
    }
    {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        Buffer *buf = reinterpret_cast<Buffer *>(self + vbt[1] - 0x61C);
        buf->set_text_color2(*reinterpret_cast<int *>(dlg_base - 0x9E0),
                             *reinterpret_cast<int *>(dlg_base - 0x9D4),
                             *reinterpret_cast<int *>(dlg_base - 0x9C8),
                             *reinterpret_cast<int *>(dlg_base - 0x9BC));
    }
    {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        Buffer *buf = reinterpret_cast<Buffer *>(self + vbt[1] - 0x61C);
        buf->set_text_color3(*reinterpret_cast<int *>(dlg_base - 0x9DC),
                             *reinterpret_cast<int *>(dlg_base - 0x9D0),
                             *reinterpret_cast<int *>(dlg_base - 0x9C4),
                             *reinterpret_cast<int *>(dlg_base - 0x9B8));
    }
    {
        int *vbt = *reinterpret_cast<int **>(front);
        char *dlg_base = self + vbt[2];
        Buffer *buf = reinterpret_cast<Buffer *>(self + vbt[1] - 0x61C);
        buf->set_font(*reinterpret_cast<Font **>(dlg_base - 0x9F0),
                      *reinterpret_cast<Font **>(dlg_base - 0x9EC),
                      *reinterpret_cast<Font **>(dlg_base - 0x9E8), 0);
    }

    {
        int *vbt = *reinterpret_cast<int **>(front);
        Win *win = reinterpret_cast<Win *>(self + vbt[1] - 0xA60);
        win->set_vert_pos(*reinterpret_cast<int *>(front + 0x18));
    }
    {
        int *vbt = *reinterpret_cast<int **>(front);
        int position = *reinterpret_cast<int *>(front + 0x18);
        int quotient = position / *reinterpret_cast<int *>(self + vbt[2] - 0xA0C);
        Win *win = reinterpret_cast<Win *>(self + vbt[1] - 0xA60);
        win->set_horz_pos(quotient);
    }

    return 0;
}

/*
Purpose: Run the double-click notification: remember the hovered window,
         fire the installed callback with the selected id, then walk the
         selected entry's notification chain (slots 56 and 55 of the target
         window, then the child object's 23/7 pair) - and raise the click
         guard's counter value through it.
// ORIGINAL: 0x0060C710 ?on_left_double_click@ListBox@@QAEXHH@Z 0x0060C710-0x0060C7BB
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0060c710.cpp", "source:src/listbox.h", "source:src/autosound.h", "source:src/win.h"], "reason": "Recover ListBox double-click callbacks through typed GraphicWin, Dialog and the Win windows at AutoSound offsets 0x7c and 0x94, following the retained seed's virtual-base adjustments and slot calls."}
// symbol    ?on_left_double_click@ListBox@@UAEXHH@Z
// size      171 bytes
// prototype void (__thiscall ?on_left_double_click@ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// calls     0x00609A50
// indirect  0x0060C73B 0x0060C761 0x0060C789 0x0060C7A8 0x0060C7B3
*/
void ListBox::on_left_double_click(int, int) {
    GraphicWin *graphic = static_cast<GraphicWin *>(this);
    Dialog *dialog = static_cast<Dialog *>(this);
    Win *target = graphic->legacy_window_31();
    WinCallbackWindow = target;

    WinNotificationCallback callback = double_click_callback_;
    if (callback != 0) {
        callback(dialog->get_selected_id());
    }

    target = graphic->legacy_window_31();
    if (target != 0) {
        target->vslot_56(dialog->get_selected_id());
    }

    const int guard = ListBoxClickGuard;
    target = graphic->legacy_window_31();
    target->vslot_55(dialog->get_selected_id(), guard);

    target = graphic->legacy_window_31();
    Win *child = target->legacy_window_37();
    if (child != 0) {
        if (child->vslot_23() == 0) {
            child->vslot_07();
        }
    }
}

/*
Purpose: Add one entry: measure the string at the current width, grow the
         list height, remember the string in the entry table and append the
         id. Returns whatever the entry append reports.
// ORIGINAL: 0x0060C920 ?item@ListBox@@QAEHPADH@Z 0x0060C920-0x0060C9A9 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0060c920.cpp", "source:src/listbox.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU; the retained seed stays beside the landed transcription."}
// size      137 bytes
// prototype int (__thiscall ?item@ListBox@@QAEHPADH@Z)(ListBox* this, int8* lpString, int)
// calls     0x00401100 0x005DC360
*/
int ListBox::item(char *text, int id) {
    char *self = reinterpret_cast<char *>(this);
    if (text == 0) {
        return 3;
    }

    int *vbt = *reinterpret_cast<int **>(self);
    char *dlg_base = self + vbt[2];
    int count = *reinterpret_cast<int *>(dlg_base + 0x2C) - *ListBoxGlobalDD4;

    if (count > 0) {
        vbt = *reinterpret_cast<int **>(self);
        Buffer *buf = reinterpret_cast<Buffer *>(self + vbt[1] + 0x444);
        buf->wrap_height(text, count);
    }

    int height;
    if (count > 0) {
        vbt = *reinterpret_cast<int **>(self);
        char *gw_base = self + vbt[1];
        height = *reinterpret_cast<int *>(gw_base + 0x45C);
    } else {
        height = 1;
    }

    field_20_ += height;

    vbt = *reinterpret_cast<int **>(self);
    char *dlg_walk = self + vbt[2];
    *reinterpret_cast<char **>(dlg_walk + 0xD8) = text;

    StringStruct *entries = reinterpret_cast<StringStruct *>(dlg_walk + 0xBC);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(entries) + 0x20) = 0;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(entries) + 0x24) = height;

    return entries->add(id);
}

/*
Purpose: Refresh the list after its entries changed: run the layout pass and
         re-select when only the selection moved, rebuild the whole window
         (attach) when entries were added or removed, re-arm the scroll
         ranges from the new metrics, then fire the callback and the
         notification slots. Returns 7 when there is no main window, or the
         attach result on failure.
// ORIGINAL: 0x0060C9B0 ?update_changes@ListBox@@QAEHXZ 0x0060C9B0-0x0060CB63 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0060c9b0.cpp", "source:src/listbox.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU; the retained seed stays beside the landed transcription."}
// size      435 bytes
// prototype int (__thiscall ?update_changes@ListBox@@QAEHXZ)(ListBox* this)
// calls     0x005ED9D0 0x005EE0B0 0x005EE0D0 0x005EE0F0 0x005EE110 0x005F7E90 0x00609A50 0x0060A020 0x0060A1E0 0x0060A670
// indirect  0x0060CA47 0x0060CAF0 0x0060CB15 0x0060CB39 0x0060CB44 0x0060CB56
*/
int ListBox::update_changes() {
    typedef int (__stdcall *release_capture_fn)();
    typedef void (__cdecl *callback_fn)(int);

    char *self = reinterpret_cast<char *>(this);
    if (HandleMain == 0) {
        return 7;
    }

    int *vbt = *reinterpret_cast<int **>(self);
    char *gw_base = self + vbt[1];
    char *dlg_base = self + vbt[2];
    int was_visible = reinterpret_cast<Win *>(gw_base)->is_visible();

    reinterpret_cast<Dialog *>(dlg_base)->get_selected_id();

    int old_child_count = field_C_;
    int old_selected = *reinterpret_cast<int *>(dlg_base + 0xEC);
    calculate_dimensions();

    if (old_child_count == field_C_) {
        set_selected_pos(old_selected, 1);
    } else {
        int a4 = *reinterpret_cast<int *>(gw_base + 0x98);
        int a1 = *reinterpret_cast<int *>(gw_base + 0xC4);
        int a2 = *reinterpret_cast<int *>(dlg_base + 0x24);
        int a3 = *reinterpret_cast<int *>(dlg_base + 0x28);
        int result = reinterpret_cast<ListBox *>(self + 0xA60)->attach(
            reinterpret_cast<GraphicWin *>(a1), a2, a3, a4);
        if (result != 0) {
            return result;
        }
        set_selected_pos((old_selected == -1) ? -1 : 0, 1);
        (*reinterpret_cast<release_capture_fn>(0x006692E0))();
        if (was_visible != 0) {
            reinterpret_cast<Win *>(gw_base)->show(0);
        }
    }

    {
        unsigned int win_flags = *reinterpret_cast<unsigned int *>(gw_base + 0x98);
        if (win_flags & 8) {
            Win *win = reinterpret_cast<Win *>(gw_base);
            win->set_vert_range(0,
                *reinterpret_cast<int *>(dlg_base + 0xCC) - 1);
            win->set_vert_paging(
                *reinterpret_cast<int *>(dlg_base + 0x58) - 1);
        } else if (win_flags & 4) {
            Win *win = reinterpret_cast<Win *>(gw_base);
            win->set_horz_range(0,
                *reinterpret_cast<int *>(dlg_base + 0xCC)
                    / *reinterpret_cast<int *>(dlg_base + 0x58) - 1);
            win->set_horz_paging(*reinterpret_cast<int *>(dlg_base + 0x54));
        }
    }

    {
        callback_fn cb = *reinterpret_cast<callback_fn *>(self + 0x3C);
        if (cb != 0) {
            cb(reinterpret_cast<Dialog *>(dlg_base)->get_selected_id());
        }
    }

    {
        Win *target = *reinterpret_cast<Win **>(gw_base + 0xC4);
        if (target != 0) {
            typedef void (OriginalObject::*slot054_fn)(int);
            (ORIGINAL(target)->*vtable_method<slot054_fn>(target, 54 * 4))(
                reinterpret_cast<Dialog *>(dlg_base)->get_selected_id());
        }
    }

    {
        Win *target = *reinterpret_cast<Win **>(gw_base + 0xC4);
        if (target != 0) {
            Win *child = *reinterpret_cast<Win **>(
                reinterpret_cast<char *>(target) + 0x90);
            if (child != 0) {
                typedef int (OriginalObject::*slot023_fn)();
                if ((ORIGINAL(child)->*vtable_method<slot023_fn>(child, 23 * 4))()
                    == 0) {
                    reinterpret_cast<VCall *>(child)->slot007();
                }
            }
        }
    }

    if (was_visible != 0) {
        reinterpret_cast<VCall *>(gw_base)->slot062();
    }

    return 0;
}
