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
#include "checkbox.h"
#include "vtable_shim.h"
#include "win.h"  // WinCallbackWindow

// The constructor loads these rebindable pointer cells, not sprite contents.
// init_class passes their original targets to Sprite::extract.
// RECOVERY_OBJECT: v2 0x00697104 0x00697108 ?CheckBoxDefault1@@3PAVSprite@@A 0x0060E670
Sprite *CheckBoxDefault1 = &g_CHECKBOX_SPRITE_1;
// RECOVERY_OBJECT: v2 0x00697108 0x0069710C ?CheckBoxDefault2@@3PAVSprite@@A 0x0060E670
Sprite *CheckBoxDefault2 = &g_CHECKBOX_SPRITE_2;

/*
Purpose: Construct the virtual bases through VC6's hidden most-derived flag,
         clear this control's state, and install its default sprite pointers.
// ORIGINAL: 0x0060E670 ??0CheckBox@@QAE@H@Z 0x0060E670-0x0060E735;0x00662F60-0x00662F84 BYTE_EXACT
// symbol    ??0CheckBox@@QAE@XZ
// MEASURED (per the coordinator's correction on constructor-vs-flag
//            mangling): a `void construct(int a1)` METHOD spelling of this
//            same body, symbol-mapped to this address, scores WORSE - 0/53
//            against this constructor spelling's 1/53. Neither reaches the
//            frame; the constructor form is kept as the closer of the two.
// size      233 bytes
// prototype void (__thiscall ??0CheckBox@@QAE@H@Z)(CheckBox* this, int)
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x00608C10
// earlier (src/unrecovered/0060e670.cpp) and reached only 0.71 mnemonic
// similarity - the image installs a real fs:[0] frame here but with pure
// esp-relative addressing, no `push ebp; mov ebp,esp`, which no flag set
// this harness tries reproduces. Landed as the closest faithful shape.
Return Value: n/a
Status: Complete
*/
CheckBox::CheckBox() {
    // VC6 owns virtual-base construction and derived table setup under its
    // hidden most-derived flag. The body initializes this class's own state.
    field_4_ = 0;
    field_8_ = 0;
    field_C_ = 0;
    checked_sprite_ = CheckBoxDefault2;
    unchecked_sprite_ = CheckBoxDefault1;
}
// (The old CheckBox(int a1) spelling: `if (a1 != 0) { new (GraphicWin*)
// GraphicWin(); Dialog::construct(); }` - both hand construction seams.)

/*
Purpose: Tear down a CheckBox: reinstall the base subobjects' own
         vtable/vtordisp values, reset the state fields, and close the
         Dialog and GraphicWin subobjects through the object's own vbtable.
// ORIGINAL: 0x0060E740 ??1CheckBox@@UAE@XZ 0x0060E740-0x0060E7C0
// LEVER: the manual `- 0x1C` came OUT - this is a real override of
//   GraphicWin's virtual destructor now, so VC6 enters it on the GraphicWin
//   subobject and walks back itself, and subtracting again double-counted.
//   2/37 -> 7/37. The note below diagnosed this exactly and could not act on
//   it while the base was a member; it is kept for the diagnosis.
// TRIED: dropping the named `self`/`obj` intermediates and writing
//   `reinterpret_cast<char *>(this)` at each use, which is the lever that
//   works on this class's on_dialog_focus and on_mouse_leave. Here it is
//   WORSE, 7/37 -> 0/37 at the same 47 compiled instructions. The remaining
//   gap is that the image folds the adjustment into every addressing mode
//   (`[edx + ecx - 0x1c]`) where this tree materialises the pointer into esi
//   once and uses `[reg + esi]`; ours also emits the three vtable stores
//   later than the image does.
// TRIED: MEASURED 2/37 agreeing. The image's opening `this` is already
//            the vtordisp-adjusted GraphicWin-subobject pointer -
//            `mov eax,[ecx-0x1c]; lea esi,[ecx-0x1c]` - and this body's
//            `char *const self = this - 0x1C;` opening compiles a different
//            prologue shape (an indirect call appears at instruction 1
//            where the image has none), which VC6 does not choose from a
//            hand-written subtraction the way it does for a genuine
//            vtordisp thunk over real virtual inheritance.
// size      128 bytes
// prototype void (__thiscall ??1CheckBox@@UAE@XZ)(CheckBox* this)
// callers   24   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
// notes     Entered on the ADJUSTED (GraphicWin-subobject) `this`: the image
//           opens `mov eax,[ecx-0x1c]` / `lea esi,[ecx-0x1c]`, exactly the
//           vtordisp adjustor the constructor's own vtordisp slot names.
Return Value: n/a
Status: Complete
*/
CheckBox::~CheckBox() {
    // NO MANUAL ADJUSTMENT. This is a real override of GraphicWin's virtual
    // destructor now, so VC6 enters it on the GraphicWin subobject and walks
    // back to the CheckBox pointer itself; subtracting 0x1C again in source
    // double-counts it. The TRIED note above diagnosed exactly this and could
    // not act on it while the base was a member.
    // Strip-all direction: the image's five derived-stage re-stores here are
    // gone, same as the constructor's. The two base closes are qualified
    // calls - the compiler's virtual-base dispatch.

    field_4_ = 0;
    field_8_ = 0;
    field_C_ = 0;
    checked_sprite_ = CheckBoxDefault2;
    unchecked_sprite_ = CheckBoxDefault1;

    Dialog::close();
    GraphicWin::close();
}

/*
Purpose: Reset the check box to its defaults, then close its dialog and
         graphic base. Both calls resolve through the vbtable, so they reach
         the Dialog and the virtual base rather than the object itself.
// ORIGINAL: 0x0060E7C0 ?close@CheckBox@@QAEIXZ 0x0060E7C0-0x0060E7F9 BYTE_EXACT
// symbol    ?close@CheckBox@@QAEIXZ
// TRIED: the reconstructed signature `?close@CheckBox@@QAEXXZ`. The census
//   names are RECONSTRUCTIONS - the image is stripped - and this one had the
//   same method carrying two signatures across one hierarchy: Dialog::close and
//   four derived close() returning void, GraphicWin::close and ListBox::close
//   returning uint32_t. The original cannot have had that. Unified on
//   `uint32_t` (the form two bodies were already tuned to, for the EAX
//   residue) and on `virtual`, which is what the vtable slot these classes
//   install by hand actually is - hence `UAEIXZ`.
// LEVER: the vbtable is RE-READ for the second call, exactly as the destructor above it does. One cached `vbtable` local has to survive the first `close()`, so VC6 spends a callee-saved register and a `push edi` the image never makes: 5 of 20. Reading `[esi]` again for the GraphicWin call is 20 of 20. Reading it inline at BOTH sites scores the same 20 of 20, so the first read may stay a named local.
// size      57 bytes
// prototype uint32_t (__thiscall ?close@CheckBox@@QAEIXZ)(CheckBox* this)
// callers   13   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00608F50
Return Value: n/a
Status: Complete
*/
uint32_t CheckBox::close() {
    // The base offsets come from the object's own vbtable, read at run time,
    // not from where they sit in a most-derived CheckBox. When one is
    // embedded in a larger class - Dialogs holds a RadioButton at 0x44 - that
    // object's vbtable names different offsets, and hardcoding this class's
    // own sends both calls to the wrong subobject. Doing exactly that passed
    // every unit test here and crashed the game on a null vtable pointer.
    // The base calls go through the runtime vbtable either way - the
    // compiler's dispatch for a virtual-base member call reads it at run
    // time exactly as the old hand walk did, so the embedded case still
    // reaches the right subobjects.
    field_4_ = 0;
    field_8_ = 0;
    field_C_ = 0;
    checked_sprite_ = CheckBoxDefault2;
    unchecked_sprite_ = CheckBoxDefault1;
    Dialog::close();
    return GraphicWin::close();
}


// The state word these three share. Every one of them reaches it the same way
// the original does - `mov eax,[ecx]` then `[eax+8]` - so the Dialog offset
// comes from the object's OWN vbtable at run time. Hardcoding this class's own
// 0xA34 would work for a most-derived CheckBox and aim at the wrong subobject
// for an embedded one, which is the defect that passed every unit test here
// and crashed the game on a null vtable pointer. Repeated inline rather than
// factored out, matching close() above, so each body stands alone.

/*
Purpose: Toggle one bit of the state word - clear it when set, set it when
         clear.

             mov eax,[ecx] / mov edx,[eax+8] / mov eax,1
             lea edx,[edx+ecx+0xEC] / mov ecx,[esp+4] / shl eax,cl
             mov ecx,[edx] / test eax,ecx / je set
             not eax / and eax,ecx / mov [edx],eax / ret 4
             set: or ecx,eax / mov [edx],ecx / ret 4

         The shift count is whatever CL holds, so the position is masked to
         five bits by the hardware; `pos & 31` states that rather than relying
         on C++ shift-overflow, which is undefined rather than truncating.
// ORIGINAL: 0x0060EB50 ?UNK1@CheckBox@@QAEXH@Z 0x0060EB50-0x0060EB7D
// size      45 bytes
// prototype void (__thiscall ?UNK1@CheckBox@@QAEXH@Z)(CheckBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CheckBox::UNK1(int pos) {
    // The state word is Dialog::selected_position_, reached through the
    // virtual base - the compiler's member access walks the vbtable exactly
    // where the old hand walk read it.
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    if (selected_position_ & mask) {
        selected_position_ &= ~mask;
    } else {
        selected_position_ |= mask;
    }
}


/*
Purpose: Report whether one bit of the state word is set. Returns the MASKED
         WORD, not a normalised 0 or 1 - the body ends `and eax, esi` with no
         further narrowing, so a caller testing it against 1 would read a
         different answer than the original gives.
// ORIGINAL: 0x0060ECC0 ?UNK2@CheckBox@@QAEHH@Z 0x0060ECC0-0x0060ECE0
// size      32 bytes
// prototype int (__thiscall ?UNK2@CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the state word masked to the requested bit
Status: Complete
*/
int CheckBox::UNK2(int pos) {
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    return static_cast<int>(selected_position_ & mask);
}


/*
Purpose: Set or clear one bit of the state word, chosen by the second argument.
         Unlike UNK1 this does not depend on the bit's current value.
// ORIGINAL: 0x0060EC80 ?set_state_pos@CheckBox@@QAEXHH@Z 0x0060EC80-0x0060ECB1
// size      49 bytes
// prototype void (__thiscall ?set_state_pos@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CheckBox::set_state_pos(int pos, int state) {
    const uint32_t mask = 1U << (static_cast<uint32_t>(pos) & 31U);
    if (state) {
        selected_position_ |= mask;
    } else {
        selected_position_ &= ~mask;
    }
}


/*
Purpose: Overwrite the whole state word in one store, then repaint through
         the virtual base - unlike UNK1/UNK2/set_state_pos above, which each
         touch a single bit.
// ORIGINAL: 0x0060ECE0 ?set_state_flag@CheckBox@@QAEXJ@Z 0x0060ECE0-0x0060ED03 BYTE_EXACT
// size      35 bytes
// prototype void (__thiscall ?set_state_flag@CheckBox@@QAEXJ@Z)(CheckBox* this, long)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060ECFA
Return Value: n/a
Status: Complete
*/
void CheckBox::set_state_flag(long value) {
    // Win slot 62 is a virtual dispatch here. A qualified GraphicWin call
    // would suppress it. Actual virtual bases supply the receiver adjustments
    // for both the member write and the most-derived redraw implementation.
    selected_position_ = static_cast<int32_t>(value);
    redraw();
}

/*
Purpose: Repaint on dialog focus, dispatching through the enclosing object.
// ORIGINAL: 0x0060FB90 ?on_dialog_focus@CheckBox@@UAEXH@Z 0x0060FB90-0x0060FBA7 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?on_dialog_focus@CheckBox@@UAEXH@Z)(CheckBox* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060FB9E
Return Value: n/a
Status: Complete
*/
void CheckBox::on_dialog_focus(int a1) {
    // The repaint, as the plain virtual call it is (win-table slot 62, the
    // redraw default the most-derived may override). a1 is never read;
    // `ret 4` still pops it.
    redraw();
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to CheckBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         CheckBox::on_dialog_focus overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x0060FEA0 ?on_dialog_focus@thunk1_CheckBox@@QAEXH@Z 0x0060FEA0-0x0060FEA8 BYTE_EXACT
// symbol    ?on_dialog_focus@CheckBox@@$4PPPPPPPM@A@AEXH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Clear the hover index and repaint, through the enclosing object.
// ORIGINAL: 0x0060FC30 ?on_mouse_leave@CheckBox@@UAEXHH@Z 0x0060FC30-0x0060FC58 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?on_mouse_leave@CheckBox@@UAEXHH@Z)(CheckBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0060FC4F
Return Value: n/a
Status: Complete
*/
void CheckBox::on_mouse_leave(int a1, int a2) {
    // Clear the hover index (Dialog::field_F0_, through the virtual base)
    // and repaint (the redraw virtual) - plain member forms; the
    // compiler's virtual-base machinery does the walking the old hand
    // chains spelled. Both parameters are dead; `ret 8` still pops them.
    field_F0_ = -1;
    redraw();
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to CheckBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         CheckBox::on_mouse_leave overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x0060FE60 ?on_mouse_leave@thunk1_CheckBox@@QAEXHH@Z 0x0060FE60-0x0060FE68 BYTE_EXACT
// symbol    ?on_mouse_leave@CheckBox@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
// ORIGINAL: 0x0060FC60 ?init_class@CheckBox@@QAAHXZ 0x0060FC60-0x0060FD52;0x00662F84-0x00662F99 BYTE_EXACT
// symbol    ?init_class@CheckBox@@SAHXZ
// size      263 bytes
// prototype int (__cdecl ?init_class@CheckBox@@QAAHXZ)()
// callers   1   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005E39A0
//
// Promoted 2026-08-15 from src/unrecovered/0060fc60.cpp to retire its
// pending_bodies forwarder.
// LEVER: ~Buffer out of line (buffer.h/buffer.cpp, 2026-09-03). Under /O2's
//   /Ob1 an in-class `~Buffer() { close(); }` expands into this body's scope
//   end as vptr store + `close()` + `~Spot()` - 71 instructions and an EBP
//   frame, against the image's 63 and one `call ??1Buffer@@QAE@XZ` with the
//   frameless EH form (`sub esp,0x588`, state at [esp+0x594]). Moving the
//   definition beside ??1Buffer's own marker in buffer.cpp collapsed the
//   teardown to that one call AND dropped the frame: 24/63 -> 63/63. The
//   frame was never a separate lever - it is what the expanded teardown
//   needed a stable pointer for.
Status: Complete
*/
int __cdecl CheckBox::init_class() {
    // A REAL LOCAL, not `char bufMem[sizeof(Buffer)]` with a placement new.
    // The image opens `sub esp, 0x588`, which is sizeof(Buffer) exactly; the
    // array spelling costs four more bytes of frame and an explicit
    // destructor call the compiler would emit itself.
    Buffer buf;
    buf.init(0x20, 0x20, 0, 0);
    int result = buf.load_pcx("jackal.pcx", 0, 10, 0xec);
    if (result != 0) {
        return result;
    }
    g_CHECKBOX_SPRITE_1.extract(&buf, 0x109, 1, 0x44, 0x20, 0x20, 0);
    g_CHECKBOX_SPRITE_2.extract(&buf, 0x109, 0x22, 0x44, 0x20, 0x20, 0);
    return 0;
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to CheckBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         CheckBox::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x0060FEE0 ?attach@thunk1_CheckBox@@QAEHPAUGraphicWin@@HHH@Z 0x0060FEE0-0x0060FEE8 BYTE_EXACT
// symbol    ?attach@CheckBox@@$4PPPPPPPM@A@AEHPAXHHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Move the group's selection to the control with `id`, then set or clear
         that control's bit in the shared state word.
// ORIGINAL: 0x0060EB80 ?set_state_id@CheckBox@@QAEXHH@Z 0x0060EB80-0x0060EC0B FILE BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0060eb80.cpp, whose claim proved the ARTIFACT reproduced while the shipped program contained nothing. Kept in the artifact's own spelling for now - it reaches the Dialog subobject through the vbtable by hand (`*(int *)(*(int *)this + 8)`), which is one of the shapes tools/compiler_work.py counts, and unpicking that is a class-model change rather than a promotion.
// symbol    ?set_state_id@CheckBox@@QAEXHH@Z
// size      139 bytes
// kind      game
Return Value: n/a
Status: Complete
*/
void CheckBox::set_state_id(int id, int value) {
    int base = *(int *)(*(int *)this + 8);
    char *list = (char *)this + base + 0xbc;
    int head = *(int *)(list + 8);
    if (head != 0) {
        int count = *(int *)(list + 0x10);
        int idx;
        *(int *)(list + 0x14) = 0;
        *(int *)(list + 0xc) = head;
        for (idx = 0; idx < count; idx = idx + 1) {
            int cur = *(int *)(list + 0xc);
            if (*(int *)(cur + 4) == id) break;
            *(int *)(list + 0x14) = *(int *)(list + 0x14) + 1;
            *(int *)(list + 0xc) = *(int *)(cur + 0xc);
        }
    }
    if (value != 0) {
        *(unsigned int *)((int)this + *(int *)(*(int *)this + 8) + 0xec) |=
            1 << *(int *)((int)this + *(int *)(*(int *)this + 8) + 0xd0);
    } else {
        *(unsigned int *)((int)this + *(int *)(*(int *)this + 8) + 0xec) &=
            ~(1 << *(int *)((int)this + *(int *)(*(int *)this + 8) + 0xd0));
    }
}

/*
Purpose: Read the shared state bit of the control with `id`: walk the item list
         to that control, then report `(1 << its ordinal) & state word` -
         nonzero when the box with this id is checked. set_state_id's read-side
         twin, and DipEdit::read_check's per-bit test.
// ORIGINAL: 0x0060EC10 ?get_state_id@CheckBox@@QAEHH@Z 0x0060EC10-0x0060EC79 BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0060ec10.cpp, whose claim proved the
//         ARTIFACT reproduced while the shipped program contained nothing, and
//         kept in that artifact's spelling, for the same reason set_state_id
//         above is: it reaches the Dialog subobject through the vbtable by
//         hand. The loop is the artifact's `for (idx = 0; idx < count;
//         idx = idx + 1)` - its note records that the do-while phrasing
//         compiled an extra `cmp;jge` ahead of the back-edge in all 11 tries.
// size      105 bytes
// prototype int (__thiscall ?get_state_id@CheckBox@@QAEHH@Z)(CheckBox* this, int)
// callers   6   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: (1 << ordinal) & state word - nonzero when `id`'s box is checked
Status: Complete
*/
int CheckBox::get_state_id(int id) {
    int base = *(int *)(*(int *)this + 8);
    char *list = (char *)this + base + 0xbc;
    int head = *(int *)(list + 8);
    if (head != 0) {
        int count = *(int *)(list + 0x10);
        int idx;
        *(int *)(list + 0x14) = 0;
        *(int *)(list + 0xc) = head;
        for (idx = 0; idx < count; idx = idx + 1) {
            int cur = *(int *)(list + 0xc);
            if (*(int *)(cur + 4) == id) break;
            *(int *)(list + 0x14) = *(int *)(list + 0x14) + 1;
            *(int *)(list + 0xc) = *(int *)(cur + 0xc);
        }
    }
    return (1 << *(int *)((int)this + *(int *)(*(int *)this + 8) + 0xd0)) &
           *(unsigned int *)((int)this + *(int *)(*(int *)this + 8) + 0xec);
}

// ====================
// Original receivers of Sprite::extract at 0x0060FCFE / 0x0060FD1C.
// RECOVERY_OBJECT: v1 0x009B8F60 ?g_CHECKBOX_SPRITE_1@@3VSprite@@A
Sprite g_CHECKBOX_SPRITE_1;
// RECOVERY_OBJECT: v1 0x009B8F90 ?g_CHECKBOX_SPRITE_2@@3VSprite@@A
Sprite g_CHECKBOX_SPRITE_2;

// ===== close_class: once-only teardown of the managed sprites =====

/*
Purpose: ?close_class@CheckBox@@QAAXXZ - run 2 (ORIGINAL(s)->*teardown)() on fixed globals,
         unguarded. The last is a tail jump in the original, so its
         return goes straight to this function's caller.
// ORIGINAL: 0x0060FD60 ?close_class@CheckBox@@QAAXXZ 0x0060FD60-0x0060FD74 BYTE_EXACT
// symbol    ?teardown_0060fd60@@YAXXZ
// size      20 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3820
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_0060fd60() {
    g_CHECKBOX_SPRITE_1.close();
    g_CHECKBOX_SPRITE_2.close();
}

/*
Purpose: A double click on a check box copies the selected row marker to the
         current-win pointer and calls the object's stored callback with the
         group's selected id. Entered on the GraphicWin-subobject adjusted
         pointer (the family's thunk convention), so the vbtable sits 0x1C
         below `this` and the callback 0x10 below; the marker and the
         callback argument are located through that vbtable at run time,
         since this class is used as a subobject of larger windows.
// ORIGINAL: 0x0060FA80 ?on_left_double_click@CheckBox@@QAEXHH@Z 0x0060FA80-0x0060FAAD BYTE_EXACT
// size      45 bytes
// prototype void (__thiscall ?on_left_double_click@CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   1   call targets   0
// kind      game
// indirect  0x0060FAA7
//
// PROMOTED out of the archived verification unit for this address, which proved the body against
// the fixed-address form of the current-win global; this tree's real
// WinCallbackWindow object is the same variable.
Return Value: n/a
Status: Complete
*/
void CheckBox::on_left_double_click(int x_coord, int y_coord) {
    char *self = reinterpret_cast<char *>(this);
    int graphicwin_disp = (*reinterpret_cast<int **>(self - 0x1c))[1];
    WinCallbackWindow = reinterpret_cast<Win *>(
        *reinterpret_cast<int *>(self + graphicwin_disp + 0xa8));

    int callback = *reinterpret_cast<int *>(self - 0x10);
    if (callback != 0) {
        int dialog_disp = (*reinterpret_cast<int **>(self - 0x1c))[2];
        int arg = *reinterpret_cast<int *>(self + dialog_disp + 0xd0);
        typedef int(__cdecl *check_click_callback)(int);
        reinterpret_cast<check_click_callback>(callback)(arg);
    }
}
