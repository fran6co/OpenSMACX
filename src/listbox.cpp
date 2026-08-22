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

void __fastcall list_box_on_dialog_focus_redirect(ListBox *self, void *, int a1) {
    self->on_dialog_focus(a1);
}

func_dialog_close ListBoxOriginalDialogClose =
    original_method<func_dialog_close>(0x00608F50);
uint32_t ListBoxCloseDynamicDefault;  // 0x009B8EE0

namespace {
// The three subobject vtables ListBox re-stages during teardown, the same
// values its constructor installs.
const uint32_t ListBoxGraphicWinVtable = 0x0067041C;  // GraphicWin subobject
const uint32_t ListBoxBufferVtable = 0x00670414;      // Buffer at GraphicWin + 0x444
const uint32_t ListBoxDialogVtable = 0x00670408;      // Dialog subobject
}  // namespace

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
    uint8_t *const base = reinterpret_cast<uint8_t *>(this);
    // Base offsets come from the object's OWN vbtable, read at run time - never
    // the compile-time 0x48/0xA60 - so an embedded ListBox with a different
    // vbtable still reaches the correct subobjects (AGENTS.md RadioButton rule).
    // GraphicWin virtual base: source-owned close at 0x005D4E40. Return discarded.
    reinterpret_cast<GraphicWin *>(
        base + (*reinterpret_cast<const int32_t *const *>(base))[1])->close();
    // Dialog virtual base at 0x00608F50, CALLED BY NAME. Reached through the
    // `ListBoxOriginalDialogClose` member pointer it cost a `call dword ptr`
    // where the image emits `call rel32` - `osmx semantic` refused this body
    // at "instruction 9: call operand is a different KIND". `Dialog::close`
    // is a real declared method backed by a pending_bodies forwarder, and
    // checkbox.cpp and radiobutton.cpp already reach it this way.
    reinterpret_cast<Dialog *>(
        base + (*reinterpret_cast<const int32_t *const *>(base))[2])->Dialog::close();

    uint32_t *const object = reinterpret_cast<uint32_t *>(base);
    object[0x04 / 4] = 0;
    object[0x08 / 4] = 0;
    object[0x18 / 4] = 0;
    object[0x1C / 4] = 0;
    object[0x20 / 4] = 0;
    object[0x24 / 4] = ListBoxCloseStaticDefaults[0];   // 0x006970E0
    object[0x28 / 4] = ListBoxCloseDynamicDefault;     // 0x009B8EE0
    object[0x2C / 4] = ListBoxCloseStaticDefaults[1];   // 0x006970E4
    object[0x10 / 4] = ListBoxCloseStaticDefaults[2];   // 0x006970E8
    object[0x14 / 4] = ListBoxCloseStaticDefaults[3];   // 0x006970EC
    // Dialog::field_B4_ = 1, reached through the runtime vbtable displacement.
    *reinterpret_cast<uint32_t *>(
        base + (*reinterpret_cast<const int32_t *const *>(base))[2] + 0xB4) = 1;
    object[0x30 / 4] = 0;
    object[0x34 / 4] = 0;
    object[0x38 / 4] = 0;
    object[0x3C / 4] = 0;
    object[0x40 / 4] = 0;
    object[0x0C / 4] = 0;
    return 0;
}

/*
Purpose: Destroy a ListBox: re-stage the GraphicWin, Buffer, and Dialog subobject
         vtables and refresh the two vbase-adjust fields (all located through the
         runtime vbtable), then run close(). The original opens no exception
         frame; there is nothing to omit.
// ORIGINAL: 0x00609EC0 ??1ListBox@@QAE@XZ 0x00609EC0-0x00609F13
// size      83 bytes
// prototype void (__thiscall ??1ListBox@@QAE@XZ)(ListBox* this)
// callers   35   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609F20
Return Value: EAX residue (close()'s residue, constant 0). The void destructor is
              entered adjusted to L + 0x48; see the redirect.
Status: Complete with temporary Dialog::close original dependency (through close)
Verification note: the sweep's two survivors here rewrite the 0x48/0xA60 in the
displacement comments, not the vbtable loads beside them; the immediates the
code does carry - the vbase-adjust subtrahends below - have their mutants
killed by the shifted-vbtable shape.
*/
uint32_t ListBox::destroy() {
    uint8_t *const base = reinterpret_cast<uint8_t *>(this);
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(base);
    const int32_t graphic_disp = vbtable[1];   // 0x48 when most-derived
    const int32_t dialog_disp = vbtable[2];    // 0xA60 when most-derived

    // Re-stage the three subobject vtables so any virtual dispatch during the
    // base closes resolves to ListBox's overrides. The ListBox vbtable pointer
    // at [base] is left untouched (only read), matching the original.
    *reinterpret_cast<volatile uint32_t *>(base + graphic_disp) =
        ListBoxGraphicWinVtable;
    *reinterpret_cast<volatile uint32_t *>(base + graphic_disp + 0x444) =
        ListBoxBufferVtable;
    *reinterpret_cast<volatile uint32_t *>(base + dialog_disp) =
        ListBoxDialogVtable;
    // Vbase-adjust fields = runtime displacement minus the most-derived offset.
    // The 0x48 / 0xA60 subtrahends are the original's baked-in immediates.
    *reinterpret_cast<volatile int32_t *>(base + graphic_disp - 4) =
        graphic_disp - 0x48;
    *reinterpret_cast<volatile int32_t *>(base + dialog_disp - 4) =
        dialog_disp - 0xA60;

    return close();
}

// self == L, no adjustment (close is entered on the ListBox base).
uint32_t __fastcall list_box_close_redirect(ListBox *self, void *) {
    return self->close();
}

// ~ListBox is entered with this = L + 0x48 (the GraphicWin controlling-vftable
// location); recover the base before delegating, as the original does with
// `eax - 0x48`.
uint32_t __fastcall list_box_destructor_redirect(void *adjusted, void *) {
    ListBox *self = reinterpret_cast<ListBox *>(
        static_cast<uint8_t *>(adjusted) - ListBoxDestructorAdjustment);
    return self->destroy();
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
    // `this - 0x48` reaches a vbtable-shaped descriptor with two deltas:
    // entry +8 locates the field, entry +4 the enclosing object's vtable.
    // Note 0x48 + 0xA8 == 0xF0 across this whole family, so the
    // field is at a FIXED +0xF0 in the enclosing object and these classes are
    // subobjects at differing offsets inside it.
    //
    // The arithmetic must go through `char *`. Through `int *` the compiler
    // scales the +8 and emits [eax+0x20].
    //
    // Both parameters are dead; `ret 8` still pops them.
    // 0xF0, NOT 0xA8 - and that is the third class to confirm the constant.
    // The compiler walks out of the subobject as part of the override's entry
    // now, so the source states the field's actual offset in the ENCLOSING
    // object rather than a subobject-relative remainder. 0x48 + 0xA8 == 0xF0,
    // exactly as 0x1C + 0xD4 and 0x18 + 0xD8 do in CheckBox and RadioButton.
    *reinterpret_cast<int *>(
        reinterpret_cast<char *>(this) + 0xF0 +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this))) + 8)) = -1;

    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this) +
        *reinterpret_cast<int *>(reinterpret_cast<char *>(
            *reinterpret_cast<int **>(
                reinterpret_cast<char *>(this))) + 4))->slot062();
}
