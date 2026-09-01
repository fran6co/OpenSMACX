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


func_dialog_close ListBoxOriginalDialogClose =
    original_method<func_dialog_close>(0x00608F50);
uint32_t ListBoxCloseDynamicDefault;  // 0x009B8EE0
int ListBoxClickGuard;                // 0x009B8EEC

// The three subobject vtables the teardown re-staged (0x0067041C/0x00670414/
// 0x00670408) left with the re-stores, same as the Dialogs stage tables.

/*
Purpose: Close the ListBox: close its GraphicWin virtual base (source-owned) and
         its Dialog virtual base (original dependency via seam), both located
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
Status: Complete with temporary Dialog::close original dependency
Verification note: the sweep's five survivors on this function rewrite hex
constants that occur only in the trailing comments naming each default's
address - the code reads the seam pointers, so the mutants are byte-identical
and equivalent by construction.
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
    // Dialog virtual base at 0x00608F50, CALLED BY NAME. `Dialog::close`
    // is a real declared method backed by a pending_bodies forwarder.
    Dialog::close();

    // The ListBox-owned resets, as the member writes they are; the defaults
    // come from the same seams as before. The Dialog::field_B4_ = 1 store
    // below is reached through the runtime vbtable displacement, as the
    // image does - its access-and-naming pass is the recorded follow-up.
    field_4_ = 0;
    field_8_ = 0;
    field_18_ = 0;
    field_1C_ = 0;
    field_20_ = 0;
    field_24_ = ListBoxCloseStaticDefaults[0];   // 0x006970E0
    field_28_ = ListBoxCloseDynamicDefault;     // 0x009B8EE0
    field_2C_ = ListBoxCloseStaticDefaults[1];   // 0x006970E4
    field_10_ = ListBoxCloseStaticDefaults[2];   // 0x006970E8
    field_14_ = ListBoxCloseStaticDefaults[3];   // 0x006970EC
    // Dialog::field_B4_ = 1, re-arming it through the virtual base as a
    // plain member write.
    field_B4_ = 1;
    field_30_ = 0;
    field_34_ = 0;
    field_38_ = 0;
    field_3C_ = 0;
    field_40_ = 0;
    field_C_ = 0;
    return 0;
}

/*
Purpose: Destroy a ListBox: re-stage the GraphicWin, Buffer, and Dialog subobject
         vtables and refresh the two vbase-adjust fields (all located through the
         runtime vbtable), then run close(). Not independently addressed - VC6
         inlines this whole body into `list_box_destructor_redirect` below,
         which is where the marker for 0x00609EC0 actually lives; a real
         `ListBox::~ListBox()` cannot be the image's own body because the
         image enters the void destructor ADJUSTED (this = L + 0x48, the
         GraphicWin subobject), which no C++ member function can express -
         `this` is always the class's own front. See the redirect's marker.
Status: Complete with temporary Dialog::close original dependency (through close)
*/
uint32_t ListBox::destroy() {
    // Strip-all direction: the image's five derived-stage re-stores here are
    // gone; the object runs on whatever its constructing chain installed.
    return close();
}


/*
Purpose: The real complete-object destructor body. ~ListBox is entered with
         this = L + 0x48 (the GraphicWin controlling-vftable location, not
         ListBox's own front); recover the base with the original's own
         `eax - 0x48`, then re-stage the three subobject vtables and the two
         vbase-adjust fields directly (not through a call to
         `ListBox::destroy()` - that call target is a SEPARATE image
         address dialogs.cpp already reaches by name, and calling it here
         adds a `call`/register-spill the image's 83-byte body does not
         pay for), then run close().
// ORIGINAL: 0x00609EC0 ??1ListBox@@QAE@XZ 0x00609EC0-0x00609F13
// symbol    ?list_box_destructor_redirect@@YIIPAX0@Z
// size      83 bytes
// prototype void (__thiscall ??1ListBox@@QAE@XZ)(ListBox* this)
// callers   35   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: EAX residue (close()'s residue, constant 0).
Status: Complete with temporary Dialog::close original dependency (through close)
*/
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
    // and repaint (the vslot_62 virtual) - plain member forms; the
    // compiler's virtual-base machinery does the walking the old hand
    // chains spelled. Both parameters are dead; `ret 8` still pops them.
    field_F0_ = -1;
    vslot_62();
}

/*
Purpose: Report a right press: raise the click guard, forward (a1, a2) through
         GraphicWin vtable slot 20, drop the guard.

/*
Purpose: Report a right press: raise the click guard, forward (a1, a2) through
         GraphicWin vtable slot 20, drop the guard.
// ORIGINAL: 0x0060AA20 ?on_right_down@ListBox@@QAEXHH@Z 0x0060AA20-0x0060AA52
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
plain virtual call it always was - win-table slot 20, vslot_20 - dispatched
on the entered receiver, which already points at the GraphicWin subobject
whose vftable the old hand walk re-derived through the front. The image
walks back to the ListBox front and re-walks out to that same table
(`mov eax,[ecx-0x48]`, 9 of 12, 0.917 similar); reproducing that addressing
is the entered-receiver relationship work (batch 9). The dispatch target is
identical either way.
*/
void ListBox::on_right_down(int a1, int a2) {
    ListBoxClickGuard = 1;
    vslot_20(a1, a2);
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
DEMOTED from BYTE_EXACT by direction (idiomatic-first): vslot_28 is the
plain virtual call; the image's front-walk addressing is the entered-
receiver relationship work (see on_right_down's note).
*/
void ListBox::on_right_double_click(int a1, int a2) {
    ListBoxClickGuard = 1;
    vslot_28(a1, a2);
    ListBoxClickGuard = 0;
}

