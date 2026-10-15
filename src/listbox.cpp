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
// ORIGINAL: 0x0060CE40 sub_60ce40 0x0060CE40-0x0060CE48;0x0060CE50-0x0060CECF FILE
// RECOVERY_BINDING: v1 {"evidence": ["task:generated-control-lifetime-owner", "historical seed:src/unrecovered/0060ce40.cpp", "pinned:terranx_original.exe:original vtables and complete deletion/control flow; virtual-base Dialog then GraphicWin destruction and conditional allocation release", "source:src/listbox.cpp:genuine destructor and actual compiler-generated COFF entry; no handwritten thunk or executable-address forwarding"], "reason": "Genuine virtual-base hierarchy and complete destruction own this compiler-generated entry; preserve the full original shared deleting-body spans and seed findings. Approximate source coverage only, not verified byte or reference identity."}
// symbol    ??_EListBox@@$4PPPPPPPM@A@AEPAXI@Z

// ORIGINAL: 0x0060D010 sub_60d010 0x0060D010-0x0060D021;0x0060CE50-0x0060CECF FILE
// RECOVERY_BINDING: v1 {"evidence": ["task:generated-control-lifetime-owner", "historical seed:src/unrecovered/0060d010.cpp", "pinned:terranx_original.exe:original vtables and complete deletion/control flow; virtual-base Dialog then GraphicWin destruction and conditional allocation release", "source:src/listbox.cpp:genuine destructor and actual compiler-generated COFF entry; no handwritten thunk or executable-address forwarding"], "reason": "Genuine virtual-base hierarchy and complete destruction own this compiler-generated entry; preserve the full original shared deleting-body spans and seed findings. Approximate source coverage only, not verified byte or reference identity."}
// symbol    ??_EListBox@@$4PPPPPLLI@EEE@AEPAXI@Z

// ORIGINAL: 0x0060D030 sub_60d030 0x0060D030-0x0060D03E;0x0060CE50-0x0060CECF FILE
// RECOVERY_BINDING: v1 {"evidence": ["task:generated-control-lifetime-owner", "historical seed:src/unrecovered/0060d030.cpp", "pinned:terranx_original.exe:original vtables and complete deletion/control flow; virtual-base Dialog then GraphicWin destruction and conditional allocation release", "source:src/listbox.cpp:genuine destructor and actual compiler-generated COFF entry; no handwritten thunk or executable-address forwarding"], "reason": "Genuine virtual-base hierarchy and complete destruction own this compiler-generated entry; preserve the full original shared deleting-body spans and seed findings. Approximate source coverage only, not verified byte or reference identity."}
// symbol    ??_EListBox@@$4PPPPPPPM@KBI@AEPAXI@Z
// TRIED: full reproduction as a single function - the scaffold's "spans" field folds this vtordisp thunk together with the shared ListBox scalar-deleting-destructor body at 0x60CE50, which is compiler-synthesised for multiple/virtual inheritance and not directly expressible from source without the full outer class hierarchy. Landed the 10-byte thunk (adjust `this` by *(this-4) and a fixed 0xA18, forward to the shared body) as a plain call, not a tail-jmp; scored SHARED_TAIL (COMDAT-folded span, no per-function verdict).

// ORIGINAL: 0x0060CF50 ?on_redraw@thunk1_ListBox@@QAEXXZ 0x0060CF50-0x0060CF58 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060CF50', '0x0060CF58']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x0067041C", "pinned win slot 0x30 contains original thunk VA 0x0060CF50", "original tail target 0x0060C350 is ?on_redraw@ListBox@@UAEXXZ; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_redraw@ListBox@@$4PPPPPPPM@A@AEXXZ in ??_7ListBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_redraw@ListBox@@$4PPPPPPPM@A@AEXXZ
// kind      game
// ORIGINAL: 0x0060CF70 ?on_mouse_move@thunk1_ListBox@@QAEXHH@Z 0x0060CF70-0x0060CF78 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060CF70', '0x0060CF78']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x0067041C", "pinned win slot 0x44 contains original thunk VA 0x0060CF70", "original tail target 0x0060CBC0 is ?on_mouse_move@ListBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_mouse_move@ListBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7ListBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_mouse_move@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060CF90 ?on_left_down@thunk1_ListBox@@QAEXHH@Z 0x0060CF90-0x0060CF98 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060CF90', '0x0060CF98']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x0067041C", "pinned win slot 0x50 contains original thunk VA 0x0060CF90", "original tail target 0x0060AA60 is ?on_left_down@ListBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_left_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7ListBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_left_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060CFA0 ?on_right_down@thunk1_ListBox@@QAEXHH@Z 0x0060CFA0-0x0060CFA8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060CFA0', '0x0060CFA8']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x0067041C", "pinned win slot 0x5C contains original thunk VA 0x0060CFA0", "original tail target 0x0060AA20 is ?on_right_down@ListBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_right_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7ListBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_right_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060CFC0 ?on_left_double_click@thunk1_ListBox@@QAEXHH@Z 0x0060CFC0-0x0060CFC8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060CFC0', '0x0060CFC8']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x0067041C", "pinned win slot 0x70 contains original thunk VA 0x0060CFC0", "original tail target 0x0060C710 is ?on_left_double_click@ListBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_left_double_click@ListBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7ListBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_left_double_click@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060D050 ?on_redraw@thunk3_ListBox@@QAEXXZ 0x0060D050-0x0060D05E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060D050', '0x0060D05E']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x00670408", "pinned primary slot 0x8 contains original thunk VA 0x0060D050", "original tail target 0x0060C350 is ?on_redraw@ListBox@@UAEXXZ; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_redraw@ListBox@@$4PPPPPPPM@KBI@AEXXZ in ??_7ListBox@@6B@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_redraw@ListBox@@$4PPPPPPPM@KBI@AEXXZ
// kind      game
#include "stdafx.h"
#include "sound.h"
#include "original_seam.h"
#include "listbox.h"
#include "scroll.h"
#include "spritebox.h"
#include "net_class.h"
#include "mapwin.h"
// Private pure branches shared with host controls; every helper has internal
// linkage in each translation unit that includes this implementation fragment.
static bool listbox_hit_single_row(int *remaining, int text_span,
                                   int line_height) {
    *remaining -= text_span * line_height;
    return *remaining < 0;
}

typedef int (*ListBoxHitSpanAt)(void *context, int position);

static int listbox_y_span_hit(int y_offset, int top, int count,
                              int line_height, void *context,
                              ListBoxHitSpanAt span_at) {
    if (count <= 0 || y_offset < 0) return -1;
    int remaining = y_offset;
    for (int position = top; position < count; ++position) {
        if (listbox_hit_single_row(&remaining, span_at(context, position),
                                  line_height)) {
            return position;
        }
    }
    return -1;
}

static int listbox_grid_hit_position(int x_offset, int y_offset, int top,
                                    int count, int rows, int column_width,
                                    int row_height) {
    // Keep x86 signed division behavior, including negative x offsets. The
    // seed divides these exact offsets without a bounds clamp.
    const int column = x_offset / column_width;
    const int row = y_offset / row_height;
    const int position = top + column * rows + row;
    return position < count ? position : -1;
}

static int listbox_hit_position_for_mode(int layout_mode, int x, int y,
        int top, int count, int x_origin, int y_origin, int rows,
        int column_width, int row_height, int line_height, void *context,
        ListBoxHitSpanAt span_at) {
    const int y_offset = y - y_origin;
    if (count <= 0 || y_offset < 0) return -1;
    if (layout_mode == 1) {
        return listbox_y_span_hit(y_offset, top, count, line_height,
                                  context, span_at);
    }
    return listbox_grid_hit_position(x - x_origin, y_offset, top, count, rows,
                                     column_width, row_height);
}

// Returns 1 for a navigation move, 0 for a handled arrow at its boundary or
// in a one-column layout, and -1 for the type-ahead arm.
static int listbox_navigation_target(int arm, int layout_mode, int rows,
                                     int count, int position, int *target) {
    if (arm == 0) {
        if (layout_mode == 1) return 0;
        position -= rows;
        if (position < 0) return 0;
    } else if (arm == 1) {
        if (position == 0) return 0;
        --position;
    } else if (arm == 2) {
        if (layout_mode == 1) return 0;
        position += rows;
        if (position >= count) return 0;
    } else if (arm == 3) {
        if (position + 1 >= count) return 0;
        ++position;
    } else {
        return -1;
    }
    *target = position;
    return 1;
}

static bool listbox_clear_old_selection(bool control_down) {
    return !control_down;
}

// -1 preserves the existing flag, 0 clears it, and 1 sets it.
static int listbox_click_selection_action(int index, int item_count,
        int position, int low, int high, bool wraps_to_last,
        bool shift_down, bool control_down, bool was_selected) {
    if (shift_down) {
        if ((index >= low && index <= high)
                || (wraps_to_last && index == item_count - 1)) return 1;
        return control_down ? -1 : 0;
    }
    if (index != position) return control_down ? -1 : 0;
    return control_down && was_selected ? 0 : 1;
}

static int listbox_typeahead_lower_input(unsigned char input) {
    const signed char signed_input = static_cast<signed char>(input);
    return tolower(static_cast<int>(signed_input));
}

static bool listbox_typeahead_matches(char item, unsigned char input) {
    if (static_cast<unsigned char>(item) == input) return true;
    const signed char signed_item = static_cast<signed char>(item);
    return static_cast<int>(signed_item) == listbox_typeahead_lower_input(input);
}

static int listbox_frame_edge_count(uint32_t stored_count) {
    return static_cast<int>(stored_count);
}

static int listbox_visible_draw_count(int columns, int rows, int first,
                                     int item_count) {
    if (columns <= 0 || rows <= 0 || first >= item_count) return 0;
    const int slots = columns * rows;
    const int available = item_count - first;
    return slots < available ? slots : available;
}

struct ListBoxScrollCopy {
    int source_x;
    int source_y;
    int destination_x;
    int destination_y;
    int width;
    int height;
};

static ListBoxScrollCopy listbox_scroll_copy(int direction, int width_metric,
        int inset, int columns, int rows, int row_height) {
    ListBoxScrollCopy copy;
    const bool vertical = direction == 0 || direction == 2;
    const int row_count = rows - (vertical ? 1 : 0);
    const int height_adjustment = columns - (vertical ? 1 : 2);
    const int total = (width_metric - inset) +
                      height_adjustment * width_metric;
    copy.width = total - inset;
    copy.height = row_count * row_height;
    copy.source_x = inset;
    copy.source_y = 0;
    copy.destination_x = inset;
    copy.destination_y = 0;
    if (direction == 0) {
        copy.source_y = row_height;
    } else if (direction == 1) {
        copy.source_x = width_metric + inset;
    } else if (direction == 2) {
        copy.destination_y = row_height;
    } else {
        copy.destination_x = width_metric + inset;
    }
    return copy;
}


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
// symbol    ?close@ListBox@@QAEXXZ
// size      125 bytes
// prototype void (__thiscall ?close@ListBox@@QAEXXZ)(ListBox* this)
// callers   35   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
Return Value: n/a
SUPERSEDED: The former uint32_t source model preserved incidental EAX=0; the
catalogued QAEX name is reconstructed and not signature proof. The pinned
mapped-caller audit found 47 direct calls with no EAX consumer, no tail callers,
and no raw absolute references, supporting the source-level void contract.
The two base close methods are reusable source-owned operations.
*/
void ListBox::close() {
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
    draw_callback_ = 0;
    write_callback_ = 0;
    fill_callback_ = 0;
    selection_callback_ = 0;
    double_click_callback_ = 0;
    field_C_ = 0;
}

// Original 609DB0 conditionally constructs GraphicWin and Dialog using the
// compiler's most-derived flag, then initializes the ListBox-owned fields.
// Superseded seed deleted; seed paths below are historical Git provenance.
/*
Purpose: Return the Win parent's slot-8 value when the original Win flag and
         parent-link guards are set.
*/
// ORIGINAL: 0x0060CED0 sub_60ced0 0x0060CED0-0x0060CF01 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "ListBox constructor at 0x00609DB0 installs Win-view table address point 0x0067041C; slots 6/7 at +0x18/+0x1C select 0x0060CED0/0x0060CF10", "the adjustor thunk resolves the GraphicWin virtual base via vbtable {0,0x48,0xA60}; the callback then reads Win-relative +0x98 (iFlags_) and +0xC4 (win_parent_), testing bit 0x40 before the parent and calling its slot 8", "original 0x005EB640 close body pushes scroll_vert_ before the slot-7 call and pushes scroll_horz_ before the matching horizontal call; both return paths are ret 4"], "reason": "Move the pinned ListBox Win-view callback into a genuine owner method; retain the original Win-member guard order and delegated slot behavior. Compiler-generated receiver adjustment and exact COFF target still require fresh native verification."}
// symbol    ?on_scroll_create@ListBox@@$4PPPPPPPM@A@AEPAVScroll@@XZ
Scroll *ListBox::on_scroll_create() {
    if ((iFlags_ & 0x40) != 0 && win_parent_ != 0) {
        return win_parent_->on_listbox_scroll_create();
    }
    return 0;
}

/*
Purpose: Forward the exact Scroll being removed through recipient slot 9
         under the original guards, otherwise return the argument.
*/
// ORIGINAL: 0x0060CF10 sub_60cf10 0x0060CF10-0x0060CF4D FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "ListBox constructor at 0x00609DB0 installs Win-view table address point 0x0067041C; slots 6/7 at +0x18/+0x1C select 0x0060CED0/0x0060CF10", "the adjustor thunk resolves the GraphicWin virtual base via vbtable {0,0x48,0xA60}; the callback then reads Win-relative +0x98 (iFlags_) and +0xC4 (win_parent_), testing bit 0x40 before the parent and calling its slot 9 with the Scroll argument", "original 0x005EB640 close body pushes scroll_vert_ before the slot-7 call and pushes scroll_horz_ before the matching horizontal call; both return paths are ret 4"], "reason": "Move the pinned ListBox Win-view callback into a genuine owner method; retain the original Win-member guard order and delegated slot behavior. Compiler-generated receiver adjustment and exact COFF target still require fresh native verification."}
// symbol    ?on_scroll_delete@ListBox@@$4PPPPPPPM@A@AEPAVScroll@@PAV2@@Z
Scroll *ListBox::on_scroll_delete(Scroll *scroll) {
    if ((iFlags_ & 0x40) != 0 && win_parent_ != 0) {
        return win_parent_->on_listbox_scroll_delete(scroll);
    }
    return scroll;
}

// ORIGINAL: 0x00609DB0 ??0ListBox@@QAE@H@Z 0x00609DB0-0x00609EB7;0x00662EF0-0x00662F14 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["promoted historical seed src/recovered/units/00609db0.cpp", "Original constructor/destructor direct-call receivers and unwind stages"], "reason": "609DB0 calls GraphicWin and Dialog constructors only for the hidden most-derived ABI flag, then stores own defaults. Genuine virtual bases synthesize this flag; it is not an explicit int source parameter."}
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
    draw_callback_ = 0;
    write_callback_ = 0;
    fill_callback_ = 0;
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
Return Value: n/a
*/
ListBox::~ListBox() {
    close();
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
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060A020 ?set_selected_pos@ListBox@@QAEXHH@Z 0x0060A020-0x0060A1DD
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0060a020.cpp", "source:src/listbox.h", "source:src/dialog.h", "source:src/win.h", "source:src/autosound.h", "pinned 0x0060A020 reads AutoSound +0x90 and calls Sound slots 23/7"], "reason": "Recover ListBox selection, viewport and notify-gated item callbacks through its typed Dialog and GraphicWin owners. The child at AutoSound +0x90 is a Sound interface view, not a Win* field; preserve the original seed and historical claim."}
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
                parent->on_selected(Dialog::pos_to_id(position));
            }

            parent = static_cast<GraphicWin *>(this)->parent_window();
            if (parent != 0) {
                parent->on_selected(Dialog::pos_to_id(position), 0);
            }

            parent = static_cast<GraphicWin *>(this)->parent_window();
            if (parent != 0) {
                Sound *child = parent->legacy_sound_36();
                if (child != 0 && child->is_playing() == 0) {
                    child->play();
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
// ORIGINAL: 0x0060AA20 ?on_right_down@ListBox@@QAEXHH@Z 0x0060AA20-0x0060AA52 BYTE_EXACT
// symbol    ?on_right_down@ListBox@@UAEXHH@Z
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
// ORIGINAL: 0x0060C6A0 ?on_scrolled@ListBox@@QAEXHH@Z 0x0060C6A0-0x0060C6C1 BYTE_EXACT
// symbol    ?on_scrolled@ListBox@@UAEXHH@Z
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
    on_scrolling(code, pos);
}

/*
Purpose: Report a right double click: raise the click guard, forward (a1, a2)
         through GraphicWin vtable slot 28, drop the guard.
// ORIGINAL: 0x0060C6D0 ?on_right_double_click@ListBox@@QAEXHH@Z 0x0060C6D0-0x0060C702 BYTE_EXACT
// symbol    ?on_right_double_click@ListBox@@UAEXHH@Z
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

/*
Purpose: Apply one WM_VSCROLL step to the list: publish the parent window
         as the callback window, keep the old top-entry position, apply the
         code (SB_THUMBTRACK sets the position directly, SB_LINEUP/DOWN
         scale the delta by the Dialog's row stride), re-select through
         set_selected_pos when the notify global is armed, and repaint -
         only the redraw slot when the position did not move, otherwise the
         two rows that scrolled plus the surface update.

// ORIGINAL: 0x0060C5D0 ?on_scrolling@ListBox@@QAEXHH@Z 0x0060C5D0-0x0060C691
// symbol    ?on_scrolling@ListBox@@UAEXHH@Z
// size      193 bytes
// prototype void (__thiscall ?on_scrolling@ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890 0x0060A020 0x0060B530
// indirect  0x0060C645
// TRIED (seed): the entered receiver is the GraphicWin virtual base
//         (dialogs.cpp dispatches by kind through the Win table), so every
//         access folds the walk back to the front - the same idiom the
//         homed on_left_double_click (listbox.cpp) spells. The seed's
//         displacements are image-exact: the scroll-position slot is
//         `esi - 0x30` at all five sites (disp8 0xD0 sign-extended - the
//         ListBox's field_18_ at front+0x18, the slot ListBox::close
//         resets), the row stride Dialog+0x10 and the top-entry index
//         Dialog+0xA4 through the vbtable walks, and the redraw/soft_update
//         dispatches anchor on the entered receiver, spelled as raw
//         GraphicWin * casts here exactly as on_left_double_click does.
Status: Typed ListBox scroll handler; exactness and runtime acceptance remain unmeasured.
*/
void ListBox::on_scrolling(int a, int b) {
    // This ordinary member receives the ListBox front. Convert to the
    // GraphicWin virtual base through the declared hierarchy.
    ListBox *front = this;
    GraphicWin *entered = static_cast<GraphicWin *>(front);
    Dialog &dialog = *front;
    WinCallbackWindow = entered->parent_window();

    const int top_entry = dialog.selected_position_;
    const int old_position = static_cast<int>(front->field_18_);
    if (a == -2) {
        front->field_18_ = static_cast<uint32_t>(
            static_cast<int>(dialog.field_58_) * b);
    } else if (a == -1) {
        front->field_18_ = static_cast<uint32_t>(b);
    }

    if (ListBoxGlobalDDC != 0) {
        front->set_selected_pos(static_cast<int>(front->field_18_), 1);
    }

    if (static_cast<int>(front->field_18_) == old_position) {
        entered->redraw();
    } else {
        if (top_entry != dialog.selected_position_) {
            front->draw_item(top_entry);
        }
        front->draw_item(dialog.selected_position_);
        entered->soft_update();
    }
}
/*
Purpose: Draw one list row: the highlighted/normal text-draw tails over the
         style-descriptor fields, reached by on_scrolling for the row the
         scroll left and the row it landed on.

// ORIGINAL: 0x0060B530 ?draw_item@ListBox@@QAEXH@Z 0x0060B530-0x0060C1D8
// RECOVERY_BINDING: v1 {"evidence": ["retired seed git:e853f54d^:src/unrecovered/0060b530.cpp preserves the 3240-byte raw-offset transcription and VC6 prologue mismatch", "ListBox::on_scrolling at 0x0060C5D0 calls this row renderer twice by name; source draw callbacks at 0x004072D0 and 0x00614C50 need its draw-context write"], "reason": "Replace the empty linked placeholder with typed ListBox/Dialog/StringList row rendering while preserving historical restoration evidence."}
// size      3240 bytes
// prototype void (__thiscall ?draw_item@ListBox@@QAEXH@Z)(ListBox* this, int)
// callers   4   call targets   17
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5250 0x005D5350 0x005D8000 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DCF40 0x005DD450 0x005DD530 0x005DFF00 0x005E3203 0x005F2CA0 0x00609B50 0x00644F3A 0x006453E0 0x00645DD0
// indirect  0x0060B857 0x0060BB58 0x0060BCDF 0x0060BD2D
Status: Source-owned typed row renderer. The body follows the pinned
        parent callback, parent-to-child surface copy, and circular StringList
        cursor branches; exactness and runtime acceptance remain unmeasured.
*/
void ListBox::draw_item(int index) {
    if (index < 0) {
        return;
    }

    Dialog &dialog = *this;
    StringList &items = dialog.item_list_;
    GraphicWin *window = this;
    Buffer *canvas = static_cast<Buffer *>(window);

    RECT row;
    row.left = static_cast<int>(dialog.field_38_);
    row.top = static_cast<int>(dialog.field_40_);
    row.right = row.left + static_cast<int>(dialog.field_68_);
    row.bottom = static_cast<int>(dialog.field_64_) + row.top
        + static_cast<int>(dialog.field_40_) * -2;

    if (dialog.field_4C_ == 1) {
        int scroll = static_cast<int>(field_18_) - 1;
        if (scroll <= items.entry_count_ - 1) {
            items.current_ = items.head_;
            if (scroll < 0) {
                typedef int (__cdecl * AbsFunction)(int);
                AbsFunction abs_function = reinterpret_cast<AbsFunction>(&abs);
                if (abs_function(scroll) > items.entry_count_) {
                    goto after_scroll_position;
                }
                int steps = abs_function(scroll);
                while (steps > 0) {
                    --steps;
                    items.current_ = items.current_->previous;
                }
                scroll += items.entry_count_;
            } else {
                int steps = scroll;
                while (steps > 0) {
                    --steps;
                    items.current_ = items.current_->next;
                }
            }
            items.current_position_ = scroll;
        }
after_scroll_position:
        int count = index - static_cast<int>(field_18_);
        int occupied = 0;
        while (count > 0) {
            StringStructEntry *const list_head = items.head_;
            if (list_head != 0) {
                items.current_ = items.current_->next;
                int position = items.current_position_ + 1;
                items.current_position_ = position;
                if (position == items.entry_count_) {
                    items.current_position_ = 0;
                }
            }
            StringStructEntry *const entry = items.current_;
            int span = 0;
            if (entry != 0) {
                if (list_head != 0) {
                    span = static_cast<int>(entry->string_payload()->field_C_);
                } else {
                    // The pinned malformed-state path loads the word at
                    // null+0x0c; retain the faulting volatile read rather
                    // than turn an inconsistent stale cursor into span 0.
                    volatile StringStructEntry *const null_entry = 0;
                    span = static_cast<int>(reinterpret_cast<uintptr_t>(
                        null_entry->next));
                }
            }
            occupied += span;
            --count;
        }
        row.left += (static_cast<int>(dialog.field_68_)
                     + static_cast<int>(dialog.field_44_)) * occupied;

        StringStructEntry *const list_head = items.head_;
        if (list_head != 0) {
            StringStructEntry *const next = items.current_->next;
            int position = items.current_position_ + 1;
            items.current_position_ = position;
            items.current_ = next;
            if (position == items.entry_count_) {
                items.current_position_ = 0;
            }
        }
        StringStructEntry *const entry = items.current_;
        int span = 0;
        if (entry != 0) {
            if (list_head != 0) {
                span = static_cast<int>(entry->string_payload()->field_C_);
            } else {
                volatile StringStructEntry *const null_entry = 0;
                span = static_cast<int>(reinterpret_cast<uintptr_t>(
                    null_entry->next));
            }
        }
        const int gap = static_cast<int>(dialog.field_44_);
        row.bottom = (static_cast<int>(dialog.field_68_) + gap) * span
            - gap + row.left;
    } else {
        const int relative = index - static_cast<int>(field_18_);
        const int columns = static_cast<int>(dialog.field_58_);
        const int along = (static_cast<int>(dialog.field_64_)
                           + static_cast<int>(dialog.field_44_))
            * (relative / columns);
        const int across = (static_cast<int>(dialog.field_68_)
                            + static_cast<int>(dialog.field_44_))
            * (relative % columns);
        row.right += along;
        row.top += along;
        row.left += across;
        row.bottom += across;
    }

    if (items.entry_count_ <= index) {
        return;
    }

    int selected = 0;
    if (field_4_ == 0) {
        selected = dialog.selected_position_ == index;
    } else {
        items.current_ = items.head_;
        int steps = index;
        while (steps > 0) {
            --steps;
            items.current_ = items.current_->next;
        }
        items.current_position_ = index;
        selected = static_cast<int>(items.current_->string_payload()->field_8_);
    }

    canvas->set_clip(&row);
    int callback_result = 0;
    if (fill_callback_ != 0) {
        callback_result = fill_callback_(window, index,
            static_cast<int>(field_18_) + ListBoxDrawOffset, &row);
    }

    canvas->set_text_color(dialog.text_color_a_, dialog.text_color_b_,
                           dialog.text_color_c_, dialog.text_color_d_);
    canvas->set_text_color2(dialog.text_color_2a_, dialog.text_color_2b_,
                            dialog.text_color_2c_, dialog.text_color_2d_);
    canvas->set_text_color3(dialog.text_color_3a_, dialog.text_color_3b_,
                            dialog.text_color_3c_, dialog.text_color_3d_);
    canvas->set_font(dialog.font1_, dialog.font2_, dialog.font3_, 0);

    int color3_a;
    int color3_b = static_cast<int>(dialog.text_color_3b_);
    int color3_c = static_cast<int>(dialog.text_color_3c_);
    int color3_d = static_cast<int>(dialog.text_color_3d_);
    if (selected == 0) {
        // The pinned body repeats these two state setters on the unselected
        // branch before installing the reversed fourth color tuple.
        canvas->set_text_color(dialog.text_color_a_, dialog.text_color_b_,
                               dialog.text_color_c_, dialog.text_color_d_);
        canvas->set_text_color2(dialog.text_color_2a_, dialog.text_color_2b_,
                                dialog.text_color_2c_, dialog.text_color_2d_);
        color3_a = static_cast<int>(dialog.text_color_3a_);
        color3_b = static_cast<int>(dialog.text_color_3b_);
        color3_c = static_cast<int>(dialog.text_color_3c_);
        color3_d = static_cast<int>(dialog.text_color_3d_);
    } else {
        canvas->set_clip(&row);
        const int emphasis = field_10_ == static_cast<uint32_t>(-1)
            ? static_cast<int>(dialog.field_6C_)
            : static_cast<int>(field_10_);
        canvas->set_text_color(emphasis, dialog.text_color_b_,
                               dialog.text_color_c_, dialog.text_color_d_);
        canvas->set_text_color2(emphasis, dialog.text_color_2b_,
                                dialog.text_color_2c_, dialog.text_color_2d_);
        color3_a = emphasis;
    }
    canvas->set_text_color3(color3_a, color3_b, color3_c, color3_d);

    if (callback_result == 0) {
        Win *const parent = window->parent_window();
        if (parent != 0) {
            int state = 2;
            if (selected == 0) {
                state = dialog.field_F0_ == static_cast<uint32_t>(index) ? 1 : 0;
            }
            callback_result = parent->on_dialog_item_back_draw(window, index, state, &row);
            if (callback_result == 0) {
                if (window->has_listbox_backfill_flag()) {
                    GraphicWin *const live_parent =
                        static_cast<GraphicWin *>(window->parent_window());
                    if (live_parent != 0) {
                        Buffer *const parent_canvas =
                            static_cast<Buffer *>(live_parent);
                        parent_canvas->copy(canvas, row.left, row.top, 0, 0,
                                            canvas->pixel_width(),
                                            canvas->pixel_height());
                    } else {
                        window->fill(static_cast<int>(dialog.field_6C_));
                    }
                } else {
                    window->fill(static_cast<int>(dialog.field_6C_));
                }
            }
        }

        RECT border = *window->buffer_working_rect();
        const int border_count = static_cast<int>(dialog.field_B4_);
        if (border_count > 0) {
            for (int edge = 0; edge < border_count; ++edge) {
                canvas->box(&border, static_cast<int>(dialog.field_AC_),
                            static_cast<int>(dialog.field_B0_));
                ++border.left;
                --border.bottom;
                ++border.top;
                --border.right;
            }
        }
    }

    // The original resets the StringList cursor to this row after frame
    // callbacks and before the draw callback; later callbacks may mutate it.
    items.current_ = items.head_;
    int steps = index;
    while (steps > 0) {
        --steps;
        items.current_ = items.current_->next;
    }
    items.current_position_ = index;

    if (draw_callback_ != 0) {
        CurrentListBoxDraw = this;
        draw_callback_(window, dialog.pos_to_id(index),
                       static_cast<int>(field_18_) + ListBoxDrawOffset,
                       row.left, row.top, row.bottom - row.top);
    }

    if (write_callback_ != 0) {
        char *callback_text = items.head_ == 0
            ? 0 : items.current_->string_payload()->text;
        const int written = write_callback_(window, dialog.pos_to_id(index),
                                            index, &row, callback_text);
        if (written != 0) {
            return;
        }
    }

    row.right = row.right - (static_cast<int>(field_2C_) + 1);
    row.left = row.left + static_cast<int>(dialog.field_34_)
        + static_cast<int>(field_2C_) + 1;
    canvas->set_clip(&row);

    char *text = items.head_ == 0
        ? 0 : items.current_->string_payload()->text;
    char *separator = strchr(text, '|');
    if (separator == 0) {
        if (items.head_ == 0) {
            text = 0;
        } else if (items.current_->string_payload()->field_C_ > 1) {
            canvas->wrap(items.current_->string_payload()->text, &row);
            goto text_tail;
        } else {
            text = items.current_->string_payload()->text;
        }
        if (text != 0) {
            canvas->write_l(text, &row, static_cast<int>(strlen(text)));
        }
        goto text_tail;
    }

    *separator = 0;
    if (items.head_ != 0) {
        if (items.current_->string_payload()->field_C_ < 2) {
            char *left_text = items.current_->string_payload()->text;
            if (left_text != 0) {
                canvas->write_l(left_text, &row,
                                static_cast<int>(strlen(left_text)));
            }
        } else {
            canvas->wrap(items.current_->string_payload()->text, &row);
        }
    }
    *separator = '|';
    ++separator;
    if (separator != 0) {
        canvas->write_right_l(separator, &row,
                              static_cast<int>(strlen(separator)));
    }

text_tail:
    row.left += static_cast<int>(field_2C_) + 1;
    row.right = (row.right - (static_cast<int>(field_2C_) + 1))
        - static_cast<int>(dialog.field_34_);
    canvas->set_clip(&row);
    if (callback_result == 0 && selected != 0 && field_2C_ > 0) {
        for (int edge = 0; edge < static_cast<int>(field_2C_); ++edge) {
            canvas->box(&row, static_cast<int>(field_24_),
                        static_cast<int>(field_28_));
            ++row.left;
            --row.right;
            ++row.top;
            --row.bottom;
        }
    }
}

// Scroll owns the shared thickness at 0x009B8DD4. List layout and item
// measurement use that source-owned default, not the old executable's data.
// 0x009B8DDC is the shared ListBox/Scroll notification gate.
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/listbox.h", "source:src/dialog.h", "source:src/stringstruct.h", "source:src/scroll.cpp:ScrollDefaultThickness", "Pinned 0x0060A2A6..0x0060A2AD loads Dialog fonts at +0x78/+0x74/+0x70; 0x0060A650..0x0060A659 divides the signed shared thickness by Dialog +0x54."], "reason": "Recover layout through real ListBox virtual bases, Buffer, Dialog metrics and circular StringList storage. Correct the old font-offset transcription and use the source-owned shared thickness; preserve dword wraparound, signed comparisons/division, count reloads and the malformed absolute address-4 read."}
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
    Dialog &dialog = *this;
    GraphicWin &graphic = *this;
    Buffer &buffer = graphic;
    StringList &entries = dialog.item_list_;

    if (entries.entry_count_ == 0) return 7;
    field_C_ = 0;

    buffer.set_text_color(dialog.text_color_a_, dialog.text_color_b_,
                          dialog.text_color_c_, dialog.text_color_d_);
    buffer.set_text_color2(dialog.text_color_2a_, dialog.text_color_2b_,
                           dialog.text_color_2c_, dialog.text_color_2d_);
    buffer.set_text_color3(dialog.text_color_3a_, dialog.text_color_3b_,
                           dialog.text_color_3c_, dialog.text_color_3d_);
    buffer.set_font(dialog.font1_, dialog.font2_, dialog.font3_, 0);

    int visible_rows = static_cast<int>(dialog.field_50_);
    int rows = entries.entry_count_;
    if (visible_rows <= rows) rows = visible_rows;
    dialog.field_58_ = static_cast<uint32_t>(rows);

    int max_width = 0;
    int n = 0;
    if (entries.entry_count_ > 0) {
        do {
            char *name;
            StringStructEntry *head = entries.head_;
            if (head != 0) {
                entries.current_ = entries.current_->next;
                entries.current_position_ = static_cast<int32_t>(
                    static_cast<uint32_t>(entries.current_position_) + 1u);
                if (entries.current_position_ == entries.entry_count_)
                    entries.current_position_ = 0;
            }
            if (entries.current_ == 0) {
                name = 0;
            } else if (head == 0) {
                // Preserve the pinned malformed-list absolute [4] read.
                name = *reinterpret_cast<char **>(4);
            } else {
                name = entries.current_->string_payload()->text;
            }
            int width = buffer.text_width(name);
            if (max_width < width) max_width = width;
            ++n;
        } while (n < entries.entry_count_);
        if (max_width != 0) goto have_width;
    }
    max_width = static_cast<int>(dialog.field_2C_);

have_width:
    // Use unsigned intermediates for the original x86 dword wraparound.
    dialog.field_64_ = dialog.field_44_ + 2u + field_2C_ * 2u
                     + dialog.field_34_ + static_cast<uint32_t>(max_width);
    if (static_cast<int32_t>(dialog.field_48_) == -1)
        dialog.field_68_ = static_cast<uint32_t>(buffer.text_line_height())
                         + dialog.field_44_;
    else
        dialog.field_68_ = dialog.field_48_;

    int tmp;
    if ((dialog.field_20_ & 2) == 0
        || (tmp = static_cast<int>(dialog.field_2C_), tmp == 0)) {
        int divisor = static_cast<int>(dialog.field_4C_);
        if (divisor == 1) {
            uint32_t width_bits = dialog.field_64_ + dialog.field_40_ * 2u;
            int width = static_cast<int32_t>(width_bits);
            int min_width = static_cast<int32_t>(dialog.field_2C_);
            if (width < min_width) width = min_width;
            dialog.field_5C_ = static_cast<uint32_t>(width);
            dialog.field_64_ = dialog.field_5C_ - dialog.field_40_ * 2u;
        } else {
            uint32_t numerator = dialog.field_5C_ - dialog.field_40_ * 2u
                               + dialog.field_44_;
            dialog.field_64_ = static_cast<uint32_t>(
                static_cast<int32_t>(numerator) / divisor)
                - dialog.field_44_;
            uint32_t width_bits = (dialog.field_64_ + dialog.field_44_)
                                * dialog.field_4C_
                                + dialog.field_40_ * 2u - dialog.field_44_;
            int width = static_cast<int32_t>(width_bits);
            int min_width = static_cast<int32_t>(dialog.field_2C_);
            if (width < min_width) width = min_width;
            dialog.field_5C_ = static_cast<uint32_t>(width);
        }
        dialog.field_54_ = dialog.field_4C_;
    } else {
        dialog.field_5C_ = static_cast<uint32_t>(tmp);
        dialog.field_54_ = dialog.field_4C_;
        if (dialog.field_54_ == 1)
            dialog.field_64_ = dialog.field_5C_ - dialog.field_40_ * 2u;
        else
            dialog.field_64_ -= dialog.field_44_;
    }

    if ((dialog.field_20_ & 1) == 0
        || (tmp = static_cast<int>(dialog.field_30_), tmp == 0)) {
        dialog.field_60_ = dialog.field_68_ * dialog.field_58_
                         - dialog.field_44_ + dialog.field_3C_
                         + dialog.field_38_;
    } else {
        dialog.field_60_ = static_cast<uint32_t>(tmp);
        uint32_t rows_numerator = dialog.field_60_ - dialog.field_3C_
                                - dialog.field_38_ + dialog.field_44_;
        dialog.field_58_ = static_cast<uint32_t>(
            static_cast<int32_t>(rows_numerator)
            / static_cast<int32_t>(dialog.field_68_));
        if ((iFlags_ & 0x40) == 0) {
            int columns = static_cast<int32_t>(dialog.field_54_);
            uint32_t product = dialog.field_58_ * dialog.field_54_;
            if (static_cast<int32_t>(product) < static_cast<int32_t>(field_20_)) {
                if (columns < 2) {
                    dialog.field_5C_ = dialog.field_2C_ - ScrollDefaultThickness;
                    dialog.field_54_ = 1;
                } else {
                    dialog.field_60_ = dialog.field_30_ - ScrollDefaultThickness;
                    dialog.field_58_ = static_cast<uint32_t>(
                        static_cast<int32_t>(dialog.field_60_)
                        / static_cast<int32_t>(dialog.field_68_));
                }
            }
        }
    }

    dialog.field_68_ -= dialog.field_44_;
    int margin = static_cast<int32_t>(dialog.field_44_);
    uint32_t height_numerator = dialog.field_60_ - dialog.field_3C_
                              - dialog.field_38_ + dialog.field_44_;
    uint32_t height_denominator = dialog.field_68_ + dialog.field_44_;
    if (static_cast<int32_t>(entries.entry_count_)
        <= static_cast<int32_t>(height_numerator)
           / static_cast<int32_t>(height_denominator)) {
        if ((dialog.field_20_ & 2) == 0) {
            dialog.field_64_ = dialog.field_5C_ - dialog.field_40_ * 2u;
        } else {
            uint32_t width_numerator = dialog.field_5C_
                                     - dialog.field_40_ * 2u
                                     + static_cast<uint32_t>(margin);
            dialog.field_64_ = static_cast<uint32_t>(
                static_cast<int32_t>(width_numerator)
                / static_cast<int32_t>(dialog.field_54_))
                - static_cast<uint32_t>(margin);
        }
    }

    uint32_t total_bits = dialog.field_58_ * dialog.field_54_;
    field_1C_ = total_bits;
    int total = static_cast<int32_t>(total_bits);
    if (total < static_cast<int32_t>(field_20_)) {
        if (1 < static_cast<int32_t>(dialog.field_54_)) {
            if ((iFlags_ & 0x40) == 0)
                dialog.field_60_ += ScrollDefaultThickness;
            field_C_ |= 1;
            return 0;
        }
        if ((iFlags_ & 0x40) == 0)
            dialog.field_5C_ += ScrollDefaultThickness;
        if ((dialog.field_20_ & 2) != 0)
            dialog.field_64_ -= ScrollDefaultThickness / static_cast<int32_t>(dialog.field_54_);
        field_C_ |= 2;
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
// RECOVERY_BINDING: v1 {"evidence": ["source:src/listbox.h", "source:src/dialog.h", "source:src/graphicwin.h", "Pinned 0x0060A670..0x0060AA15 enters on the Dialog virtual-base view; its field arithmetic identifies the ListBox front, both virtual bases and the GraphicWin Buffer base."], "reason": "Recover attachment through the real ListBox override and declared base members. Preserve the original fallback, signed arithmetic, failure propagation and scroll/color/font sequence; compiler-generated receiver adjustments replace manual subobject arithmetic."}
// symbol    ?attach@ListBox@@UAEHPAVGraphicWin@@HHH@Z
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

    GraphicWin &graphic = *this;
    Dialog &dialog = *this;
    Win &window = graphic;
    Buffer &buffer = graphic;

    int flags = (a4 & 0xfffffff3) | 0x20;
    iFlags_ = static_cast<uint32_t>(flags);

    int dim_result = calculate_dimensions();
    if (dim_result != 0) {
        if (dim_result != 7) {
            return dim_result;
        }
        dialog.field_5C_ = dialog.field_2C_;
        dialog.field_60_ = dialog.field_30_;
        dialog.field_54_ = 1;
        dialog.field_58_ = 1;
    }

    dialog.field_24_ = static_cast<uint32_t>(a2);
    dialog.field_28_ = static_cast<uint32_t>(a3);

    if (static_cast<int32_t>(field_20_) > static_cast<int32_t>(field_1C_)) {
        if (static_cast<int32_t>(dialog.field_54_) > 1) {
            flags |= 4;
        } else {
            flags |= 8;
        }
    }

    if (!a1->has_dialog_focus_flag()) {
        flags |= 0x100000;
    }
    if (flags & 0x40) {
        flags |= 0x800;
    }

    int init_result = graphic.init(
        a2, a3,
        static_cast<int32_t>(dialog.field_60_),
        static_cast<int32_t>(dialog.field_5C_),
        0, flags, a1, 0, 0);
    if (init_result != 0) {
        return init_result;
    }

    if (flags & 8) {
        window.set_vert_range(0, dialog.item_list_.entry_count_ - 1);
        window.set_vert_paging(static_cast<int32_t>(dialog.field_58_) - 1);
    } else if (flags & 4) {
        int numerator = dialog.item_list_.entry_count_;
        int quotient = numerator / static_cast<int32_t>(dialog.field_58_);
        window.set_horz_range(0, quotient - 1);
        window.set_horz_paging(static_cast<int32_t>(dialog.field_54_));
    }

    if (dialog.item_list_.entry_count_ == 0) {
        field_18_ = 0;
    } else {
        while (static_cast<int32_t>(dialog.selected_position_)
               >= static_cast<int32_t>(field_1C_ + field_18_)) {
            int step;
            if (static_cast<int32_t>(dialog.field_54_) > 1) {
                step = static_cast<int32_t>(dialog.field_58_);
            } else {
                step = 1;
            }
            field_18_ += static_cast<uint32_t>(step);
        }
    }

    if (static_cast<int32_t>(field_20_) > static_cast<int32_t>(field_1C_)) {
        if (static_cast<int32_t>(dialog.field_54_) > 1) {
            window.set_horz_pos(static_cast<int32_t>(field_18_));
        } else {
            window.set_vert_pos(static_cast<int32_t>(field_18_));
        }
    }

    buffer.set_text_color(
        static_cast<int>(dialog.text_color_a_),
        static_cast<int>(dialog.text_color_b_),
        static_cast<int>(dialog.text_color_c_),
        static_cast<int>(dialog.text_color_d_));
    buffer.set_text_color2(
        static_cast<int>(dialog.text_color_2a_),
        static_cast<int>(dialog.text_color_2b_),
        static_cast<int>(dialog.text_color_2c_),
        static_cast<int>(dialog.text_color_2d_));
    buffer.set_text_color3(
        static_cast<int>(dialog.text_color_3a_),
        static_cast<int>(dialog.text_color_3b_),
        static_cast<int>(dialog.text_color_3c_),
        static_cast<int>(dialog.text_color_3d_));
    buffer.set_font(dialog.font1_, dialog.font2_, dialog.font3_, 0);

    window.set_vert_pos(static_cast<int32_t>(field_18_));
    int position = static_cast<int32_t>(field_18_);
    int quotient = position / static_cast<int32_t>(dialog.field_54_);
    window.set_horz_pos(quotient);

    return 0;
}

/*
Purpose: Run the double-click notification: remember the hovered window,
         fire the installed callback with the selected id, then walk the
         ListBox's Win-parent notification chain (slots 56 and 55, then the
         parent's child-object 23/7 pair), preserving the original null exit
         and subsequent parent reloads.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060C710 ?on_left_double_click@ListBox@@QAEXHH@Z 0x0060C710-0x0060C7BB
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0060c710.cpp", "source:src/listbox.h", "source:src/autosound.h", "source:src/win.h", "pinned 0x0060C710 resolves the GraphicWin virtual base through vbtable {0,0x48,0xA60}; repeated vbtable adjustment reads Win-relative +0xC4 (win_parent_) for slots 56/55 and +0x94 on that parent for Sound slots 23/7", "pinned 0x0060C74B tests the reloaded parent and branches to the epilogue at 0x0060C7B6 when null; later callback paths reload the parent"], "reason": "Recover ListBox double-click callbacks through typed GraphicWin and Dialog owners, using the actual Win parent and preserving the original parent-null exit, reload positions, and call order."}
// symbol    ?on_left_double_click@ListBox@@UAEXHH@Z
// size      171 bytes
// prototype void (__thiscall ?on_left_double_click@ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// calls     0x00609A50
// indirect  0x0060C73B 0x0060C761 0x0060C789 0x0060C7A8 0x0060C7B3
*/
void ListBox::on_left_double_click(int, int) {
    GraphicWin *graphic = static_cast<GraphicWin *>(this);
    Dialog *dialog = static_cast<Dialog *>(this);
    Win *target = graphic->parent_window();
    WinCallbackWindow = target;

    WinNotificationCallback callback = double_click_callback_;
    if (callback != 0) {
        callback(dialog->get_selected_id());
    }

    target = graphic->parent_window();
    if (target == 0) return;
    target->on_double_clicked(dialog->get_selected_id());

    const int guard = ListBoxClickGuard;
    target = graphic->parent_window();
    target->on_double_clicked(dialog->get_selected_id(), guard);

    target = graphic->parent_window();
    Sound *child = target->legacy_sound_37();
    if (child != 0) {
        if (child->is_playing() == 0) {
            child->play();
        }
    }
}

/*
Purpose: Add one entry: measure the string at the current width, grow the
         list height, remember the string in the entry table and append the
         id. Returns whatever the entry append reports.
// ORIGINAL: 0x0060C920 ?item@ListBox@@QAEHPADH@Z 0x0060C920-0x0060C9A9 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0060c920.cpp", "source:src/listbox.h", "source:src/buffer.h", "source:src/stringstruct.h", "source:src/scroll.cpp:ScrollDefaultThickness", "Pinned 0x0060C920..0x0060C9A9 measures positive available width, reads Buffer +0x18, updates ListBox +0x20, then publishes StringList text/metadata before add."], "reason": "Recover item insertion through the real Dialog list and GraphicWin Buffer base, including the shared Scroll thickness. The measured height is Buffer state, not wrap_height's return or ListBox's same-offset scroll state; preserve modulo-word accumulation and insertion result."}
// size      137 bytes
// prototype int (__thiscall ?item@ListBox@@QAEHPADH@Z)(ListBox* this, int8* lpString, int)
// calls     0x00401100 0x005DC360
*/
int ListBox::item(char *text, int id) {
    if (text == 0) {
        return 3;
    }

    Dialog &dialog = *this;
    Buffer &buffer = static_cast<GraphicWin &>(*this);
    int count = static_cast<int32_t>(
        dialog.field_2C_ - static_cast<uint32_t>(ScrollDefaultThickness));

    if (count > 0) {
        buffer.wrap_height(text, count);
    }

    int height;
    if (count > 0) {
        height = static_cast<int32_t>(Buffer::field_18_);
    } else {
        height = 1;
    }

    field_20_ += height;

    StringList &entries = dialog.item_list_;
    entries.pending_text_ = text;
    entries.field_20_ = 0;
    entries.field_24_ = static_cast<uint32_t>(height);
    return entries.add(id);
}

/*
Purpose: Refresh the list after its entries changed: run the layout pass and
         re-select when only the selection moved, rebuild the whole window
         (attach) when entries were added or removed, re-arm the scroll
         ranges from the new metrics, then fire the callback and the
         notification slots. Returns 7 when there is no main window, or the
         attach result on failure.
// ORIGINAL: 0x0060C9B0 ?update_changes@ListBox@@QAEHXZ 0x0060C9B0-0x0060CB63 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/listbox.h", "source:src/dialog.h", "source:src/win.h", "source:src/autosound.h", "Pinned 0x0060CA20..0x0060CA2B directly calls ListBox::attach; subsequent notifications reload the parent after callbacks."], "reason": "Recover refresh through real virtual bases, list state, callback and parent members, retaining the concrete attach target and original notification ordering. The AutoSound child storage is identified, but its nullary slot-23/slot-7 dynamic receiver type remains unresolved."}
// size      435 bytes
// prototype int (__thiscall ?update_changes@ListBox@@QAEHXZ)(ListBox* this)
// calls     0x005ED9D0 0x005EE0B0 0x005EE0D0 0x005EE0F0 0x005EE110 0x005F7E90 0x00609A50 0x0060A020 0x0060A1E0 0x0060A670
// indirect  0x0060CA47 0x0060CAF0 0x0060CB15 0x0060CB39 0x0060CB44 0x0060CB56
*/
int ListBox::update_changes() {
    if (HandleMain == 0) {
        return 7;
    }

    GraphicWin *graphic = static_cast<GraphicWin *>(this);
    Win *window = graphic;
    Dialog *dialog = static_cast<Dialog *>(this);
    int was_visible = window->is_visible();

    dialog->get_selected_id();

    int old_child_count = field_C_;
    int old_selected = dialog->selected_position_;
    calculate_dimensions();

    if (old_child_count == field_C_) {
        set_selected_pos(old_selected, 1);
    } else {
        int a4 = static_cast<int>(iFlags_);
        GraphicWin *a1 = static_cast<GraphicWin *>(window->parent_window());
        int a2 = static_cast<int>(dialog->field_24_);
        int a3 = static_cast<int>(dialog->field_28_);
        int result = this->ListBox::attach(
            a1, a2, a3, a4);
        if (result != 0) {
            return result;
        }
        set_selected_pos((old_selected == -1) ? -1 : 0, 1);
        ::ReleaseCapture();
        if (was_visible != 0) {
            window->show(0);
        }
    }

    {
        unsigned int win_flags = iFlags_;
        if (win_flags & 8) {
            window->set_vert_range(0, dialog->item_list_.entry_count_ - 1);
            window->set_vert_paging(static_cast<int>(dialog->field_58_) - 1);
        } else if (win_flags & 4) {
            window->set_horz_range(0,
                dialog->item_list_.entry_count_
                    / static_cast<int>(dialog->field_58_) - 1);
            window->set_horz_paging(static_cast<int>(dialog->field_54_));
        }
    }

    {
        WinNotificationCallback cb = selection_callback_;
        if (cb != 0) {
            cb(dialog->get_selected_id());
        }
    }

    {
        Win *target = window->parent_window();
        if (target != 0) {
            target->on_selected(dialog->get_selected_id());
        }
    }

    {
        Win *target = window->parent_window();
        if (target != 0) {
            // The pinned child sequence is Sound slots 23/7 at AutoSound
            // offset 0x90; Win slot 23 has a different coordinate signature.
            Sound *child = target->legacy_sound_36();
            if (child != 0) {
                if (child->is_playing() == 0) {
                    child->play();
                }
            }
        }
    }

    if (was_visible != 0) {
        static_cast<GraphicWin *>(this)->redraw();
    }

    return 0;
}

// Seed provenance: 0x0060AA60/0x0060CBC0 calculate an entry hit from the
// Dialog row/column metrics, while 0x0060AF90 walks the same StringList for
// keyboard typeahead. Dialogs::on_left_down/on_mouse_move/on_key_down route
// kind 2 to these ListBox handlers; the retained seed spells the entered
// GraphicWin/virtual-Dialog displacements as raw offsets. This helper uses
// the declared virtual bases and payload members instead.
struct ListBoxHitSpanContext {
    StringList *items;
};

static int listbox_hit_span_at(void *context, int position) {
    ListBoxHitSpanContext *const span_context =
        static_cast<ListBoxHitSpanContext *>(context);
    span_context->items->seek_pos(position);
    StringStructPayload *payload = span_context->items->current_entry();
    return payload == 0 ? 0 : static_cast<int>(payload->field_C_);
}

static int listbox_hit_position(ListBox *list, int x, int y, int top) {
    Dialog &dialog = *list;
    StringList &items = dialog.item_list_;
    ListBoxHitSpanContext context = { &items };
    return listbox_hit_position_for_mode(
        static_cast<int>(dialog.field_54_), x, y, top, items.entry_count_,
        static_cast<int>(dialog.field_40_),
        static_cast<int>(dialog.field_38_),
        static_cast<int>(dialog.field_58_),
        static_cast<int>(dialog.field_64_ + dialog.field_44_),
        static_cast<int>(dialog.field_68_ + dialog.field_44_),
        static_cast<int>(dialog.field_68_ + dialog.field_44_), &context,
        listbox_hit_span_at);
}

// The pinned byte map at 0x0060B4EC is indexed by VK - 0x25: bytes 0..3 map
// to arrows and every remaining byte in the 0x25..0x68 range maps to 4.
// Out-of-range values branch directly to the same type-ahead arm.
static int listbox_key_dispatch_arm(int key) {
    if (key < 0x25 || key > 0x68) return 4;
    const int table_index = key - 0x25;
    return table_index < 4 ? table_index : 4;
}

// Instruction/caller provenance retained in Git history of the deleted seed
// src/recovered/units/0060aa60.cpp (span 0x0060AA60..0x0060AF84): publish the
// parent as the callback receiver, hit-test the clicked row, update selected
// entry callbacks and redraw. In multi-select mode payload field_8_ records
// selected entries; Control keeps existing row flags. Shift selects the
// previous-to-click interval, with the original -1 anchor wrapping through
// the tail before row zero; without Shift, only the clicked row changes,
// and Control toggles that row off if it was already selected.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060AA60 ?on_left_down@ListBox@@QAEXHH@Z 0x0060AA60-0x0060AF84
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0060aa60.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
// symbol    ?on_left_down@ListBox@@UAEXHH@Z
void ListBox::on_left_down(int x, int y) {
    GraphicWin *window = this;
    WinCallbackWindow = window->parent_window();
    const int position = listbox_hit_position(
        this, x, y, static_cast<int>(field_18_));
    if (position < 0) {
        return;
    }

    const int previous = Dialog::selected_position_;
    if (field_4_ != 0 && position != previous) {
        const bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        const bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        const bool wrapped_initial_shift = shift && previous < 0;
        const int anchor = previous < 0 ? position : previous;
        const int low = (anchor < position) ? anchor : position;
        const int high = (anchor > position) ? anchor : position;
        const int interval_low = wrapped_initial_shift ? 0 : low;
        if (listbox_clear_old_selection(control)) {
            for (int index = 0; index < Dialog::item_list_.entry_count_; ++index) {
                if (!Dialog::item_list_.seek_pos(index)) continue;
                StringStructPayload *payload =
                    Dialog::item_list_.current_entry();
                if (payload != 0) payload->field_8_ = 0;
            }
        }
        for (int index = 0; index < Dialog::item_list_.entry_count_; ++index) {
            if (!Dialog::item_list_.seek_pos(index)) continue;
            StringStructPayload *payload = Dialog::item_list_.current_entry();
            if (payload == 0) continue;
            const int action = listbox_click_selection_action(
                index, Dialog::item_list_.entry_count_, position, interval_low,
                high, wrapped_initial_shift, shift, control,
                payload->field_8_ != 0);
            if (action >= 0) payload->field_8_ = action;
        }
    }
    set_selected_pos(position, 1);
    if ((Dialog::field_20_ & 4) != 0 && window->parent_window() != 0) {
        window->parent_window()->on_button_clicked(-1);
    }
}

// The retained 0x0060CBC0 seed derives a hovered list position from the same
// row metrics as the click handler, then updates Dialog::field_F0_ and redraws
// the changed hover state. The source-owned position helper keeps the cursor
// and row payload access on StringList's declared API.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060CBC0 ?on_mouse_move@ListBox@@QAEXHH@Z 0x0060CBC0-0x0060CE0B
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0060cbc0.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
// symbol    ?on_mouse_move@ListBox@@UAEXHH@Z
void ListBox::on_mouse_move(int x, int y) {
    const int position = listbox_hit_position(
        this, x, y, static_cast<int>(field_18_));
    if (position < 0) {
        return;
    }
    const int previous_hover = static_cast<int>(Dialog::field_F0_);
    if (position == Dialog::selected_position_) {
        if (previous_hover == -1) return;
        Dialog::field_F0_ = static_cast<uint32_t>(-1);
        ListBoxDrawOffset = previous_hover - static_cast<int>(field_18_);
        draw_item(previous_hover);
        static_cast<GraphicWin *>(this)->soft_update();
        return;
    }
    if (previous_hover == position) return;
    Dialog::field_F0_ = static_cast<uint32_t>(position);
    if (previous_hover != -1) {
        ListBoxDrawOffset = previous_hover - static_cast<int>(field_18_);
        draw_item(previous_hover);
    }
    ListBoxDrawOffset = position - static_cast<int>(field_18_);
    draw_item(position);
    static_cast<GraphicWin *>(this)->soft_update();
}

// 0x0060AF90's table at 0x60B4EC maps VK_LEFT/UP/RIGHT/DOWN to four
// layout-sensitive arms and all remaining keys to MapVirtualKey typeahead.
// Dialogs::on_key_down routes kind 2 here.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060AF90 ?on_key_down@ListBox@@QAEXH@Z 0x0060AF90-0x0060B4D5
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0060af90.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
// symbol    ?on_key_down@ListBox@@UAEHH@Z
int ListBox::on_key_down(int key) {
    WinCallbackWindow = static_cast<GraphicWin *>(this)->parent_window();
    StringList &items = Dialog::item_list_;
    const int count = items.entry_count_;
    int position = Dialog::selected_position_;
    if (position == -1) {
        position = 0;
        Dialog::selected_position_ = 0;
    }
    int next_position = position;
    const int arm = listbox_key_dispatch_arm(key);
    const int navigation = listbox_navigation_target(
        arm, static_cast<int>(Dialog::field_54_),
        static_cast<int>(Dialog::field_58_), count, position,
        &next_position);
    if (navigation > 0) {
        set_selected_pos(next_position, 1);
        return 1;
    }
    if (navigation == 0) return 1;
    if (count <= 0) {
        return 0;
    }

    const UINT character = MapVirtualKeyA(
        static_cast<UINT>(key) & 0xFFF8FFFFu, 2);
    if (character == 0) {
        return 0;
    }
    const unsigned char first = static_cast<unsigned char>(character);
    for (int offset = 1; offset <= count; ++offset) {
        const int candidate = (position + offset) % count;
        if (!items.seek_pos(candidate)) {
            continue;
        }
        StringStructPayload *payload = items.current_entry();
        if (payload == 0 || payload->text == 0 || payload->text[0] == 0) {
            continue;
        }
        if (listbox_typeahead_matches(payload->text[0], first)) {
            set_selected_pos(candidate, 1);
            return 1;
        }
    }
    return 0;
}

// Seed identity 0x0060C1E0, retained span 0x0060C1E0..0x0060C33F, has no
// direct callers in the image and calls only Buffer::set_clip/Buffer::copy.
// Directions 0..3 shift the list surface one row or column while preserving
// the surrounding working rectangle; the next redraw repaints exposed rows.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060C1E0 ?UNK1@ListBox@@QAEXH@Z 0x0060C1E0-0x0060C33F
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0060c1e0.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
void ListBox::UNK1(int direction) {
    if (direction < 0 || direction > 3) {
        return;
    }
    GraphicWin *window = this;
    Buffer *buffer = window;
    buffer->restore_clip();
    const ListBoxScrollCopy copy = listbox_scroll_copy(
        direction, static_cast<int>(Dialog::field_64_),
        static_cast<int>(Dialog::field_40_),
        static_cast<int>(Dialog::field_54_),
        static_cast<int>(Dialog::field_58_),
        static_cast<int>(Dialog::field_68_));
    buffer->copy(buffer, copy.source_x, copy.source_y,
                 copy.destination_x, copy.destination_y,
                 copy.width, copy.height);
}

// The Dialog virtual entry at 0x0060C350 first rejects an empty StringList,
// then clips, honors optional list/parent paint callbacks, copies or fills the
// backing surface, draws nested border edges, and paints visible entries.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0060C350 ?on_redraw@ListBox@@QAEXXZ 0x0060C350-0x0060C5C4
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0060c350.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
// symbol    ?on_redraw@ListBox@@UAEXXZ
void ListBox::on_redraw() {
    if (Dialog::item_list_.head_ == 0) return;
    GraphicWin *window = this;
    Win *parent = window->parent_window();
    WinCallbackWindow = parent;
    Buffer *buffer = window;
    buffer->restore_clip();

    bool handled = false;
    if (fill_callback_ != 0 && fill_callback_(window, 0, 0, 0) != 0) {
        handled = true;
    }
    if (!handled && parent != 0 &&
        parent->on_dialog_back_draw(window)
            != 0) {
        handled = true;
    }
    if (!handled) {
        if ((iFlags_ & 0x80000u) != 0 && parent != 0) {
            GraphicWin *parent_graphic = static_cast<GraphicWin *>(parent);
            parent_graphic->copy(buffer,
                static_cast<int>(Dialog::field_24_),
                static_cast<int>(Dialog::field_28_), 0, 0,
                window->pixel_width(), -window->pixel_height());
        } else {
            window->fill(static_cast<int>(Dialog::field_24_));
        }
    }

    RECT border = *window->buffer_working_rect();
    const int border_count = listbox_frame_edge_count(Dialog::field_6C_);
    for (int edge = 0; edge < border_count; ++edge) {
        buffer->box(&border, static_cast<int>(Dialog::field_64_),
                    static_cast<int>(Dialog::field_68_));
        ++border.left;
        ++border.top;
        --border.right;
        --border.bottom;
    }

    const int count = Dialog::item_list_.entry_count_;
    const int first = static_cast<int>(field_18_);
    const int visible_count = listbox_visible_draw_count(
        static_cast<int>(Dialog::field_54_),
        static_cast<int>(Dialog::field_58_), first, count);
    ListBoxDrawOffset = 0;
    for (int offset = 0; offset < visible_count; ++offset) {
        ListBoxDrawOffset = offset;
        draw_item(first + offset);
    }
    ListBoxDrawOffset = visible_count;
}

// ORIGINAL: 0x0060CF60 ?on_mousewheel@thunk1_ListBox@@QAEXH@Z 0x0060CF60-0x0060CF68 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: ListBox constructor 0x00609DB0 installs Win view 0x0067041C; slot +0x3C points to original identity 0x0060CF60", "retained build/recovery/builds/build-rosw1myy: build/recovery/builds/build-rosw1myy/CMakeFiles/OpenSMACX.dir/src/listbox.cpp.obj has matching source-input closure and physical table relocation to ?on_mousewheel@ListBox@@$4PPPPPPPM@A@AEXH@Z", "retained COFF ?on_mousewheel@ListBox@@$4PPPPPPPM@A@AEXH@Z has the 8-byte vtordisp subtraction shape and tail relocation to real method symbol ?on_mousewheel@ListBox@@UAEXH@Z", "the exact legacy kind fact `game` is copied verbatim from the source envelope and parser-read back; it is existing catalogue classification, not a newly granted kind", "this evidence establishes a current owner-TU/compiler-thunk correspondence only; no fresh build, linked retained-table selection, body-byte match, or whole-game acceptance is claimed"], "reason": "The ListBox Win-view constructor-installed table selects this original receiver adjustor at byte offset 0x3C; the retained owner COFF physically emits the corresponding compiler vtordisp thunk and its relocation tails to ListBox::on_mousewheel. This FILE marker remains with the actual class owner TU; it does not prove final linked table selection or full acceptance."}
// kind      game
// symbol    ?on_mousewheel@ListBox@@$4PPPPPPPM@A@AEXH@Z

// ORIGINAL: 0x0060CFB0 ?on_key_down@thunk1_ListBox@@QAEXH@Z 0x0060CFB0-0x0060CFB8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: ListBox constructor 0x00609DB0 installs Win view 0x0067041C; slot +0x68 points to original identity 0x0060CFB0", "retained build/recovery/builds/build-rosw1myy: build/recovery/builds/build-rosw1myy/CMakeFiles/OpenSMACX.dir/src/listbox.cpp.obj has matching source-input closure and physical table relocation to ?on_key_down@ListBox@@$4PPPPPPPM@A@AEHH@Z", "retained COFF ?on_key_down@ListBox@@$4PPPPPPPM@A@AEHH@Z has the 8-byte vtordisp subtraction shape and tail relocation to real method symbol ?on_key_down@ListBox@@UAEHH@Z", "the exact legacy kind fact `game` is copied verbatim from the source envelope and parser-read back; it is existing catalogue classification, not a newly granted kind", "this evidence establishes a current owner-TU/compiler-thunk correspondence only; no fresh build, linked retained-table selection, body-byte match, or whole-game acceptance is claimed"], "reason": "The ListBox Win-view constructor-installed table selects this original receiver adjustor at byte offset 0x68; the retained owner COFF physically emits the corresponding compiler vtordisp thunk and its relocation tails to ListBox::on_key_down. This FILE marker remains with the actual class owner TU; it does not prove final linked table selection or full acceptance."}
// kind      game
// symbol    ?on_key_down@ListBox@@$4PPPPPPPM@A@AEHH@Z

/*
Purpose: Land the list selection on the row whose id matches: reset the
         Dialog entry list to its head, walk forward comparing ids, and
         select the position the walk reached - or position -1 when the id
         is absent or the list is empty.

// ORIGINAL: 0x00609FA0 ?set_selected_id@ListBox@@QAEXH@Z 0x00609FA0-0x0060A013 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The pinned body at 0x609FAB forms the Dialog virtual-base list at Dialog+0xBC; loads/stores at +0xC4/+0xC8/+0xCC/+0xD0 correspond to declared StringList head/current/count/position fields.", "Pinned branches 0x609FBB/0x609FC8 and 0x609FD4/0x609FE8 preserve the empty-list, count-bounded ID walk, selection-on-hit and -1 selection-on-miss paths; the successful call is 0x60A020.", "ListBox already declares set_selected_id; src/listbox.cpp owns its sibling set_selected_pos and src/listbox.h owns the real virtual Dialog inheritance. No guessed class or layout is introduced."], "reason": "Transfer the existing ListBox method into its class implementation TU and retain its complete list walk and set_selected_pos calls. This source binding records owner placement and field provenance, not a byte-exact or linked-selection verdict."}
// size      115 bytes
// prototype void (__thiscall ?set_selected_id@ListBox@@QAEXH@Z)(ListBox* this, int)
// callers   17   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0060A020
// Homed from src/recovered/units/00609fa0.cpp. The image resets
// item_list_.current_ to head_ through the Dialog virtual base and selects
// through Dialog::item_list_.current_position_ (the seed's Dialog+0xBC
// list and +0xD0 position), tail-falling into set_selected_pos(-1, 1).
Status: Coverage transcription - unclaimed body
*/
void ListBox::set_selected_id(int id) {
    if (Dialog::item_list_.head_ != 0) {
        const int count = Dialog::item_list_.entry_count_;
        Dialog::item_list_.current_position_ = 0;
        Dialog::item_list_.current_ = Dialog::item_list_.head_;
        int index = 0;
        do {
            StringStructEntry *entry = Dialog::item_list_.current_;
            if (entry->id == id) {
                set_selected_pos(Dialog::item_list_.current_position_, 1);
                return;
            }
            ++index;
            Dialog::item_list_.current_position_ =
                Dialog::item_list_.current_position_ + 1;
            Dialog::item_list_.current_ = entry->next;
        } while (index < count);
    }
    set_selected_pos(-1, 1);
}

/*
Purpose: Turn a mouse-wheel step count into that many scroll clicks: the
         negative direction fires the vertical scrollbar's wheel-down, the
         non-negative its wheel-up, once per notch, on the Win virtual
         base the list's vbtable names.

// ORIGINAL: 0x0060CB70 ?on_mousewheel@ListBox@@QAEXH@Z 0x0060CB70-0x0060CBBA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x60CB84..0x60CB98 repeats Win::on_mousewheel_down(1) once per negated notch; 0x60CBA1..0x60CBB3 repeats Win::on_mousewheel_up(1) for nonnegative input.", "The original obtains the Win receiver by following ListBox's GraphicWin virtual-base vbtable adjustment (-0x48 plus vbtable[1]); ListBox genuinely inherits GraphicWin and GraphicWin genuinely derives from Win.", "ListBox::on_mousewheel is already declared override in src/listbox.h; the projected qualified Win member calls let the declared C++ base conversion supply that receiver without raw-vtable dispatch."], "reason": "Transfer the declared ListBox override into its class implementation TU. Preserve the original notch loops and route through the genuine Win base methods; no manual slot or receiver adapter is introduced."}
// symbol    ?on_mousewheel@ListBox@@UAEXH@Z
// size      74 bytes
// prototype void (__thiscall ?on_mousewheel@ListBox@@QAEXH@Z)(ListBox* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F85B0 0x005F85E0
// Homed from src/recovered/units/0060cb70.cpp, displacements re-taken from
// the image (the seed's own note records its register-allocation wall; the
// two callees are Win::on_mousewheel_down / Win::on_mousewheel_up, reached
// on the Win virtual base through the front's vbtable).
Status: Coverage transcription - unclaimed body
*/
void ListBox::on_mousewheel(int delta) {
    if (delta < 0) {
        int count = -delta;
        if (count > 0) {
            do {
                Win::on_mousewheel_down(1);
            } while (--count);
        }
    } else {
        int count = delta;
        if (count > 0) {
            do {
                Win::on_mousewheel_up(1);
            } while (--count);
        }
    }
}

/*
Purpose: Find the list entry whose text case-insensitively matches the given
         string and re-position the entry list cursor to it: reset the cursor
         to the head, walk next links forward for a non-negative target or
         previous links for a negative one (wrapping through the entry count),
         and report the matched position, or -1 when nothing matched.

// ORIGINAL: 0x0060C7D0 ?UNK2@ListBox@@QAEHPAD@Z 0x0060C7D0-0x0060C920 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x60C7D4..0x60C7DC obtains the Dialog virtual-base view and tests its StringList count at Dialog+0xCC; subsequent head/current/position accesses are +0xC4/+0xC8/+0xD0.", "StringStructEntry declares next at +0x0C, previous at +0x10 and payload at +0x08; StringStructEntry::string_payload()->text is the declared payload path for the original [entry+8][+4] text read.", "Pinned 0x60C867..0x60C86C explicitly zeroes EAX then loads [EAX+4] when current is nonnull but head is null. The source keeps the original absolute address-4 read in that malformed-state branch; it is not normalized or claimed as a source-owned global.", "Pinned calls 0x644F3A and 0x64FD20 are the original abs and _strcmpi callees; loop count reloads, cursor order, early returns and current-position restoration are retained.", "ListBox already declares UNK2 and genuinely virtually inherits Dialog; src/listbox.cpp is the existing implementation TU for ListBox list behavior."], "reason": "Transfer the existing ListBox member into its implementation TU, replacing verified Dialog/StringList/entry/payload offset accesses with the declared owners while retaining original branches, reload order, CRT calls and address-4 malformed-state behavior. The retained MISMATCH remains an exactness obligation."}
// size      336 bytes
// prototype int (__thiscall ?UNK2@ListBox@@QAEHPAD@Z)(ListBox* this, char*)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644F3A 0x0064FD20
Status: Coverage transcription - body rehomed from the retired seed
        (src/unrecovered/0060c7d0.cpp), freshly measured MISMATCH (divergence
        #1, similarity 0.86, 109 compiled vs 119 original instructions - the
        remaining gap is this spelling caching the entry count in a local
        where the original re-reads dlg+0xcc at each use). The cursor
        walk is the recovered Dialog::get_selected_id shape on the same
        item_list_ fields seen through ListBox's vbtable displacement, with
        raw dlg offsets per the landed ListBox::calculate_dimensions above;
        the advance target compares the PRE-search cursor, and the search
        arm keeps the original's head==0 read through address 4. The callees
        are the image's real abs (0x00644F3A) and _strcmpi (0x0064FD20),
        called through the public CRT names per the map.cpp treaty.
*/
int ListBox::UNK2(char *text) {
    StringList &items = Dialog::item_list_;
    if (items.entry_count_ == 0) {
        return -1;
    }
    if (text == 0) {
        return -1;
    }

    int result = -1;
    int position = items.current_position_;
    int n = 0;
    if (items.entry_count_ > 0) {
        do {
            StringStructEntry *head = items.head_;
            if (head != 0) {
                StringStructEntry *current = items.current_;
                current = current->next;
                items.current_ = current;
                int pos = items.current_position_ + 1;
                items.current_position_ = pos;
                if (pos == items.entry_count_) {
                    items.current_position_ = 0;
                }
            }
            char *name;
            if (items.current_ == 0) {
                name = 0;
            } else if (head == 0) {
                name = *reinterpret_cast<char **>(4);
            } else {
                name = items.current_->string_payload()->text;
            }
            if (_strcmpi(name, text) == 0) {
                result = items.current_position_;
                break;
            }
            ++n;
        } while (n < items.entry_count_);
    }

    // Keep the original count reloads and negative-distance CRT calls.
    if (position <= items.entry_count_ - 1) {
        items.current_ = items.head_;
        if (position < 0) {
            if (abs(position) > items.entry_count_) {
                return result;
            }
            int distance = abs(position);
            while (distance > 0) {
                StringStructEntry *current = items.current_;
                items.current_ = current->previous;
                distance--;
            }
            position += items.entry_count_;
        } else {
            int distance = position;
            while (distance > 0) {
                StringStructEntry *current = items.current_;
                items.current_ = current->next;
                distance--;
            }
        }
        items.current_position_ = position;
    }
    return result;
}

// ORIGINAL: 0x0060CFD0 ?on_right_double_click@thunk1_ListBox@@QAEXHH@Z 0x0060CFD0-0x0060CFD8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: ListBox constructor span contains the store of table 0x0067041C; the pinned dword at 0x00670490 is 0x0060CFD0.", "The original 8-byte receiver thunk at 0x0060CFD0 tail-jumps to 0x0060C6D0, mapped as ?on_right_double_click@ListBox@@QAEXHH@Z in this class owner.", "The actual class owner is src/listbox.cpp; its current owner object vftable relocation at byte offset 0x74 names the generated thunk, whose terminal relocation targets ?on_right_double_click@ListBox@@UAEXHH@Z.", "The source identity retains its exact historical kind fact `game`; its full original spans, claims, lessons, and findings are checked unchanged by the writer readback."], "reason": "The pinned ListBox constructor at 0x00609DB0 installs its Win view at 0x0067041C; the original slot cell at +0x74 selects this full identity. The real ListBox virtual member and owner-TU vftable are already source-owned; current owner COFF evidence binds this FILE identity to the compiler-emitted receiver adjustor tailing to ?on_right_double_click@ListBox@@UAEXHH@Z."}
// kind      game
// symbol    ?on_right_double_click@ListBox@@$4PPPPPPPM@A@AEXHH@Z

// ORIGINAL: 0x0060CFE0 ?on_scrolled@thunk1_ListBox@@QAEXHH@Z 0x0060CFE0-0x0060CFE8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: ListBox constructor span contains the store of table 0x0067041C; the pinned dword at 0x006704DC is 0x0060CFE0.", "The original 8-byte receiver thunk at 0x0060CFE0 tail-jumps to 0x0060C6A0, mapped as ?on_scrolled@ListBox@@QAEXHH@Z in this class owner.", "The actual class owner is src/listbox.cpp; its current owner object vftable relocation at byte offset 0xC0 names the generated thunk, whose terminal relocation targets ?on_scrolled@ListBox@@UAEXHH@Z.", "The source identity retains its exact historical kind fact `game`; its full original spans, claims, lessons, and findings are checked unchanged by the writer readback."], "reason": "The pinned ListBox constructor at 0x00609DB0 installs its Win view at 0x0067041C; the original slot cell at +0xC0 selects this full identity. The real ListBox virtual member and owner-TU vftable are already source-owned; current owner COFF evidence binds this FILE identity to the compiler-emitted receiver adjustor tailing to ?on_scrolled@ListBox@@UAEXHH@Z."}
// kind      game
// symbol    ?on_scrolled@ListBox@@$4PPPPPPPM@A@AEXHH@Z

// ORIGINAL: 0x0060CFF0 ?on_scrolling@thunk1_ListBox@@QAEXHH@Z 0x0060CFF0-0x0060CFF8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: ListBox constructor span contains the store of table 0x0067041C; the pinned dword at 0x006704E0 is 0x0060CFF0.", "The original 8-byte receiver thunk at 0x0060CFF0 tail-jumps to 0x0060C5D0, mapped as ?on_scrolling@ListBox@@QAEXHH@Z in this class owner.", "The actual class owner is src/listbox.cpp; its current owner object vftable relocation at byte offset 0xC4 names the generated thunk, whose terminal relocation targets ?on_scrolling@ListBox@@UAEXHH@Z.", "The source identity retains its exact historical kind fact `game`; its full original spans, claims, lessons, and findings are checked unchanged by the writer readback."], "reason": "The pinned ListBox constructor at 0x00609DB0 installs its Win view at 0x0067041C; the original slot cell at +0xC4 selects this full identity. The real ListBox virtual member and owner-TU vftable are already source-owned; current owner COFF evidence binds this FILE identity to the compiler-emitted receiver adjustor tailing to ?on_scrolling@ListBox@@UAEXHH@Z."}
// kind      game
// symbol    ?on_scrolling@ListBox@@$4PPPPPPPM@A@AEXHH@Z
