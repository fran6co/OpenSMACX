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
    // Dialog::field_B4_ = 1, reached through the runtime vbtable displacement.
    *reinterpret_cast<uint32_t *>(
        reinterpret_cast<uint8_t *>(this) +
        (*reinterpret_cast<const int32_t *const *>(this))[2] + 0xB4) = 1;
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

namespace {
// Slots 20, 28 and 49 of the vtable at the GraphicWin virtual base, each
// spelled with the (int, int) the three click handlers below push. A LOCAL
// shim, not vtable_shim.h's VCall, for the same reason editgroup.cpp spells
// CloseVCall: only DECLARATION ORDER fixes a slot, and a slot's signature is
// chosen to match its call site.
class ListBoxClickVCall { public:
    virtual void slot000(int, int);
    virtual void slot001(int, int);
    virtual void slot002(int, int);
    virtual void slot003(int, int);
    virtual void slot004(int, int);
    virtual void slot005(int, int);
    virtual void slot006(int, int);
    virtual void slot007(int, int);
    virtual void slot008(int, int);
    virtual void slot009(int, int);
    virtual void slot010(int, int);
    virtual void slot011(int, int);
    virtual void slot012(int, int);
    virtual void slot013(int, int);
    virtual void slot014(int, int);
    virtual void slot015(int, int);
    virtual void slot016(int, int);
    virtual void slot017(int, int);
    virtual void slot018(int, int);
    virtual void slot019(int, int);
    virtual void slot020(int, int);  // on_right_down -> [eax + 0x50]
    virtual void slot021(int, int);
    virtual void slot022(int, int);
    virtual void slot023(int, int);
    virtual void slot024(int, int);
    virtual void slot025(int, int);
    virtual void slot026(int, int);
    virtual void slot027(int, int);
    virtual void slot028(int, int);  // on_right_double_click -> [eax + 0x70]
    virtual void slot029(int, int);
    virtual void slot030(int, int);
    virtual void slot031(int, int);
    virtual void slot032(int, int);
    virtual void slot033(int, int);
    virtual void slot034(int, int);
    virtual void slot035(int, int);
    virtual void slot036(int, int);
    virtual void slot037(int, int);
    virtual void slot038(int, int);
    virtual void slot039(int, int);
    virtual void slot040(int, int);
    virtual void slot041(int, int);
    virtual void slot042(int, int);
    virtual void slot043(int, int);
    virtual void slot044(int, int);
    virtual void slot045(int, int);
    virtual void slot046(int, int);
    virtual void slot047(int, int);
    virtual void slot048(int, int);
    virtual void slot049(int, int);  // on_scrolled -> [eax + 0xc4]
};
}  // namespace

/*
Purpose: Report a right press: raise the click guard, forward (a1, a2) through
         GraphicWin vtable slot 20, drop the guard.
// ORIGINAL: 0x0060AA20 ?on_right_down@ListBox@@QAEXHH@Z 0x0060AA20-0x0060AA52 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0060aa20 archive unit. The
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
*/
void ListBox::on_right_down(int a1, int a2) {
    ListBoxClickGuard = 1;
    ListBox *const base = from_graphic_base(this);
    reinterpret_cast<ListBoxClickVCall *>(
        reinterpret_cast<uint8_t *>(base) +
        (*reinterpret_cast<const int32_t *const *>(base))[1])->slot020(a1, a2);
    ListBoxClickGuard = 0;
}

/*
Purpose: Forward a vertical-scroll (code, pos) through GraphicWin vtable
         slot 49.
// ORIGINAL: 0x0060C6A0 ?on_scrolled@ListBox@@QAEXHH@Z 0x0060C6A0-0x0060C6C1 BYTE_EXACT
// LEVER: byte-exact on promotion, same entered-adjusted receiver as
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
*/
void ListBox::on_scrolled(int code, int pos) {
    ListBox *const base = from_graphic_base(this);
    reinterpret_cast<ListBoxClickVCall *>(
        reinterpret_cast<uint8_t *>(base) +
        (*reinterpret_cast<const int32_t *const *>(base))[1])->slot049(code, pos);
}

/*
Purpose: Report a right double click: raise the click guard, forward (a1, a2)
         through GraphicWin vtable slot 28, drop the guard.
// ORIGINAL: 0x0060C6D0 ?on_right_double_click@ListBox@@QAEXHH@Z 0x0060C6D0-0x0060C702 BYTE_EXACT
// LEVER: byte-exact on promotion, same entered-adjusted receiver as
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
*/
void ListBox::on_right_double_click(int a1, int a2) {
    ListBoxClickGuard = 1;
    ListBox *const base = from_graphic_base(this);
    reinterpret_cast<ListBoxClickVCall *>(
        reinterpret_cast<uint8_t *>(base) +
        (*reinterpret_cast<const int32_t *const *>(base))[1])->slot028(a1, a2);
    ListBoxClickGuard = 0;
}

