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
// ORIGINAL: 0x0060CF70 ?on_mouse_move@thunk1_ListBox@@QAEXHH@Z 0x0060CF70-0x0060CF78 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060CF70', '0x0060CF78']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x0067041C", "pinned win slot 0x44 contains original thunk VA 0x0060CF70", "original tail target 0x0060CBC0 is ?on_mouse_move@ListBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_mouse_move@ListBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7ListBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_mouse_move@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060CF90 ?on_left_down@thunk1_ListBox@@QAEXHH@Z 0x0060CF90-0x0060CF98 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060CF90', '0x0060CF98']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x0067041C", "pinned win slot 0x50 contains original thunk VA 0x0060CF90", "original tail target 0x0060AA60 is ?on_left_down@ListBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_left_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7ListBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_left_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060CFA0 ?on_right_down@thunk1_ListBox@@QAEXHH@Z 0x0060CFA0-0x0060CFA8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image sha256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; retained spans [['0x0060CFA0', '0x0060CFA8']]", "mapped constructor ??0ListBox@@QAE@H@Z directly stores table 0x0067041C", "pinned win slot 0x5C contains original thunk VA 0x0060CFA0", "original tail target 0x0060AA20 is ?on_right_down@ListBox@@UAEXHH@Z; current emitted thunk preserves receiver arithmetic and relocates to that same method", "source owner emits ?on_right_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z in ??_7ListBox@@6BWin@@@ at the proven view/slot", "identity, original extent, claims and lessons retained; fresh native compile/binding review required"], "reason": "Bind the original ordinary virtual-adjustor identity to the compiler-emitted class/view adjustor selected by both the pinned constructor-installed vftable slot and exact-current owner-object relocation. The original wrapper, slot and receiver view remain distinct; no thunk body is hand-authored."}
// symbol    ?on_right_down@ListBox@@$4PPPPPPPM@A@AEXHH@Z
// kind      game
// ORIGINAL: 0x0060CFC0 ?on_left_double_click@thunk1_ListBox@@QAEXHH@Z 0x0060CFC0-0x0060CFC8 FILE
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
#include "vtable_shim.h"
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
// Superseded seed deleted; seed paths below are historical Git provenance.
/*
Purpose: Return the configured recipient's slot-8 value under the
         original AutoSound flag and recipient guards.
*/
// ORIGINAL: 0x0060CED0 sub_60ced0 0x0060CED0-0x0060CF01 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "ListBox constructor at 0x00609DB0 installs Win-view table address point 0x0067041C; slots 6/7 at +0x18/+0x1C select 0x0060CED0/0x0060CF10", "original code reads Win/AutoSound val_20_ at +0x50 and val_31_ at +0x7C; current AutoSound layout maps those fields and types val_31_ as Win*", "original 0x005EB640 close body pushes scroll_vert_ before the slot-7 call and pushes scroll_horz_ before the matching horizontal call; both return paths are ret 4"], "reason": "Move the pinned ListBox Win-view callback into a genuine owner method; retain the original guard order and delegated slot behavior. Compiler-generated receiver adjustment and exact COFF target still require fresh native verification."}
// symbol    ?on_scroll_create@ListBox@@$4PPPPPPPM@A@AEHXZ
int ListBox::on_scroll_create() {
    if ((val_20_ & 0x40) != 0 && val_31_ != 0) {
        return val_31_->vslot_08();
    }
    return 0;
}

/*
Purpose: Forward the exact Scroll being removed through recipient slot 9
         under the original guards, otherwise return the argument.
*/
// ORIGINAL: 0x0060CF10 sub_60cf10 0x0060CF10-0x0060CF4D FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "ListBox constructor at 0x00609DB0 installs Win-view table address point 0x0067041C; slots 6/7 at +0x18/+0x1C select 0x0060CED0/0x0060CF10", "original code reads Win/AutoSound val_20_ at +0x50 and val_31_ at +0x7C; current AutoSound layout maps those fields and types val_31_ as Win*", "original 0x005EB640 close body pushes scroll_vert_ before the slot-7 call and pushes scroll_horz_ before the matching horizontal call; both return paths are ret 4"], "reason": "Move the pinned ListBox Win-view callback into a genuine owner method; retain the original guard order and delegated slot behavior. Compiler-generated receiver adjustment and exact COFF target still require fresh native verification."}
// symbol    ?on_scroll_delete@ListBox@@$4PPPPPPPM@A@AEHPAVScroll@@@Z
int ListBox::on_scroll_delete(Scroll *scroll) {
    if ((val_20_ & 0x40) != 0 && val_31_ != 0) {
        return reinterpret_cast<int>(val_31_->vslot_09(scroll));
    }
    return reinterpret_cast<int>(scroll);
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
// Superseded seed deleted; seed paths below are historical Git provenance.
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
                if (child != 0 && reinterpret_cast<Sound *>(child)->is_playing() == 0) {
                    reinterpret_cast<Sound *>(child)->play();
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

/*
Purpose: Apply one WM_VSCROLL step to the list: publish the parent window
         as the callback window, keep the old top-entry position, apply the
         code (SB_THUMBTRACK sets the position directly, SB_LINEUP/DOWN
         scale the delta by the Dialog's row stride), re-select through
         set_selected_pos when the notify global is armed, and repaint -
         only the redraw slot when the position did not move, otherwise the
         two rows that scrolled plus the surface update.

// ORIGINAL: 0x0060C5D0 ?on_scrolling@ListBox@@QAEXHH@Z 0x0060C5D0-0x0060C691
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
            callback_result = parent->vslot_11(window, index, state, &row);
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
// Superseded seed deleted; seed paths below are historical Git provenance.
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
        if (reinterpret_cast<Sound *>(child)->is_playing() == 0) {
            reinterpret_cast<Sound *>(child)->play();
        }
    }
}

/*
Purpose: Add one entry: measure the string at the current width, grow the
         list height, remember the string in the entry table and append the
         id. Returns whatever the entry append reports.
// ORIGINAL: 0x0060C920 ?item@ListBox@@QAEHPADH@Z 0x0060C920-0x0060C9A9 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0060c920.cpp", "source:src/listbox.h"], "reason": "Map the declared dependency to its seed identity in the owning product TU; the retained seed stays beside the landed transcription."}
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
            // The pinned child sequence is slot 23 (nullary int result),
            // then slot 7 (nullary side effect). This does not match Win's
            // declared slot-23 coordinate callback, and no physical owner
            // view proves Sound or another typed receiver here; retain the
            // distinct unresolved view instead of guessing its class.
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
        parent->vslot_10(static_cast<int>(reinterpret_cast<uintptr_t>(window)))
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

// ORIGINAL: 0x0060CF60 ?on_mousewheel@thunk1_ListBox@@QAEXH@Z 0x0060CF60-0x0060CF68 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: ListBox constructor 0x00609DB0 installs Win view 0x0067041C; slot +0x3C points to original identity 0x0060CF60", "retained build/recovery/builds/build-rosw1myy: build/recovery/builds/build-rosw1myy/CMakeFiles/OpenSMACX.dir/src/listbox.cpp.obj has matching source-input closure and physical table relocation to ?on_mousewheel@ListBox@@$4PPPPPPPM@A@AEXH@Z", "retained COFF ?on_mousewheel@ListBox@@$4PPPPPPPM@A@AEXH@Z has the 8-byte vtordisp subtraction shape and tail relocation to real method symbol ?on_mousewheel@ListBox@@UAEXH@Z", "the exact legacy kind fact `game` is copied verbatim from the source envelope and parser-read back; it is existing catalogue classification, not a newly granted kind", "this evidence establishes a current owner-TU/compiler-thunk correspondence only; no fresh build, linked retained-table selection, body-byte match, or whole-game acceptance is claimed"], "reason": "The ListBox Win-view constructor-installed table selects this original receiver adjustor at byte offset 0x3C; the retained owner COFF physically emits the corresponding compiler vtordisp thunk and its relocation tails to ListBox::on_mousewheel. This FILE marker remains with the actual class owner TU; it does not prove final linked table selection or full acceptance."}
// kind      game
// symbol    ?on_mousewheel@ListBox@@$4PPPPPPPM@A@AEXH@Z

// ORIGINAL: 0x0060CFB0 ?on_key_down@thunk1_ListBox@@QAEXH@Z 0x0060CFB0-0x0060CFB8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: ListBox constructor 0x00609DB0 installs Win view 0x0067041C; slot +0x68 points to original identity 0x0060CFB0", "retained build/recovery/builds/build-rosw1myy: build/recovery/builds/build-rosw1myy/CMakeFiles/OpenSMACX.dir/src/listbox.cpp.obj has matching source-input closure and physical table relocation to ?on_key_down@ListBox@@$4PPPPPPPM@A@AEHH@Z", "retained COFF ?on_key_down@ListBox@@$4PPPPPPPM@A@AEHH@Z has the 8-byte vtordisp subtraction shape and tail relocation to real method symbol ?on_key_down@ListBox@@UAEHH@Z", "the exact legacy kind fact `game` is copied verbatim from the source envelope and parser-read back; it is existing catalogue classification, not a newly granted kind", "this evidence establishes a current owner-TU/compiler-thunk correspondence only; no fresh build, linked retained-table selection, body-byte match, or whole-game acceptance is claimed"], "reason": "The ListBox Win-view constructor-installed table selects this original receiver adjustor at byte offset 0x68; the retained owner COFF physically emits the corresponding compiler vtordisp thunk and its relocation tails to ListBox::on_key_down. This FILE marker remains with the actual class owner TU; it does not prove final linked table selection or full acceptance."}
// kind      game
// symbol    ?on_key_down@ListBox@@$4PPPPPPPM@A@AEHH@Z
