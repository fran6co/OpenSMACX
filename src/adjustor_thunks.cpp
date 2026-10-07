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

/*
 * CURRENT PINNED-VIEW QUALIFICATION (2026-10-06).
 * Eighteen ListBox, CheckBox and SpriteBox callbacks below have pinned
 * constructor-installed table/slot proof, matching receiver arithmetic,
 * and the same original/current method tail target. The dated sections
 * that follow are retained as historical experiment notes, not as
 * universal compiler-output rules. All other manual rows stay pending.
 */
/*
 * WHY 92 OF THESE 116 ARE UNCLAIMED, MEASURED 2026-08-26.
 *
 * They are not 92 problems and they are not a byte grind. Every one
 * measured shows the same shape, and it is structural:
 *
 *   IMAGE   add ecx, 0x188          OURS   push dword ptr [esp + 4]
 *           jmp  0x6130e0                  add  ecx, 0x188
 *                                          call 0x407272
 *
 * The image's thunk is a TAIL JUMP. Ours pushes the argument and calls,
 * because a hand-written `__fastcall(void *self, void *unused, T arg)`
 * forwarder does not have the callee's signature and so cannot be
 * tail-called. The receiver adjustment already matches; the transfer does
 * not, and no spelling of the body changes that - the two forms differ in
 * the FUNCTION'S OWN signature, not in its statements.
 *
 * These thunks are COMPILER OUTPUT in the original. VC6 emits
 * `sub ecx, N; jmp target` for a virtual override reached through a
 * multiple-inheritance or vtordisp subobject, and it emits them only when
 * the override is DECLARED. Every hand-written forwarder in this file
 * exists because the tree does not declare the inheritance edge that would
 * make the compiler write it.
 *
 * Verified on 0x00407260 (thunk1_Dialogs::on_dialog_focus, 2 image
 * instructions against our 5), 0x00407060, 0x0048C020, and on the sibling
 * shape in deleting_thunks.cpp (0x00421980 PopMenu::delete1: 2 against 9).
 *
 * WHAT THE ROUTE IS NOT, CHECKED THE SAME DAY. "Declare the inheritance
 * and the compiler writes them" is the obvious answer and it is NOT
 * sufficient:
 *
 *   - CheckBox is ALREADY `: public virtual GraphicWin, public virtual
 *     Dialog`, converted in an earlier pass;
 *   - GraphicWin ALREADY declares `virtual void on_dialog_focus(int)` and
 *     `virtual void on_mouse_leave(int, int)`, so CheckBox's overrides are
 *     implicitly virtual;
 *   - and only 1 of CheckBox's 5 thunks is claimed.
 *
 * So the declarations that were supposed to be missing are present, and
 * the thunks still do not match. VC6 does emit adjustor thunks here - the
 * build carries 77 `adjustor{1092}` entries, 1092 being the 0x444 these
 * thunks adjust by - but they attach to `vector deleting destructor`
 * symbols, the ones the compiler already owns, not to the ordinary virtual
 * methods this file forwards. No `@@W...`-mangled thunk appears in any
 * object file.
 *
 * WHAT IS ESTABLISHED: the shape above, on 0x00407260, 0x00407060,
 * 0x0048C020 and deleting_thunks.cpp's 0x00421980 - image tail-jumps,
 * ours pushes and calls, and the difference is the forwarder's OWN
 * signature. WHAT IS NOT: which declaration, if any, makes VC6 emit these
 * particular thunks. Two greps supported two different theories today and
 * both were wrong, so the next attempt should start by finding ONE
 * compiler-emitted thunk for an ordinary virtual override and reading what
 * declaration produced it - not by changing 112 bodies on a hypothesis.
 */

#include "stdafx.h"
#include "adjustor_thunks.h"
#include "checkbox.h"
#include "console.h"
#include "dialogs.h"
#include "editgroup.h"
#include "listbox.h"
#include "mapwin.h"
#include "planwin.h"
#include "radiobutton.h"
#include "spritebox.h"

/*
 * WHY 106 OF THESE SIT TWO OR THREE INSTRUCTIONS SHORT, and what would
 * actually close it.
 *
 * The image's thunk is three instructions - `sub ecx, [ecx-4]`,
 * `sub ecx, <fixed>`, then a TAIL JUMP to the target. Ours is seven: the
 * vtordisp load does not fold into the `sub`, and VC6 emits `push arg; call;
 * ret 4` where the image jumps and lets the target reuse the caller's pushed
 * argument.
 *
 * MEASURED, so this is not a guess. The target is ALREADY a named method -
 * `reinterpret_cast<MapWin *>(...)->MapWin::on_button_clicked(arg0)` - so
 * "call through a seam blocks the tail call" is not the explanation, and
 * declaring the destructor for real and forwarding it changes nothing at all
 * (0 of 2 either way on `??1thunk1_RadioButton`). VC6 1998 does not do
 * sibling-call optimisation for a `__fastcall` free function calling a
 * `__thiscall` member with a stack argument, and no spelling asks it to.
 *
 * THE PATH THROUGH IS NOT TO WRITE THESE AT ALL. MSVC GENERATES adjustor
 * thunks itself, byte-exact by construction, when a class really declares
 * virtual inheritance and overrides through it. These are hand-written because
 * the window hierarchy models its vtable as an opaque dword the original
 * installs by hand, so no class here declares a virtual - see the long note in
 * `radiobutton.h`, which measures what that costs and states that giving
 * GraphicWin and Dialog real virtual destructors is a coupled edit rather than
 * an impossible one.
 *
 * So: do not grind these individually. The whole family falls out of that one
 * structural change, or it does not fall out at all.
 */

/*
Verification note: the generated leaf suite proves that each body reads its
vtordisp from the displacement recorded for it, subtracts the adjustment
recorded for it, and forwards its arguments and its result untouched. It does
NOT independently confirm that `this - vtordisp - adjust` is what the original
computed, because the suite's expectation is derived from the same table the
bodies are. What pins the model is the acceptance check in
tools/generate_adjustor_thunks.py: a thunk is emitted only when its bytes are
exactly `sub ecx, [ecx - D]` / optional `sub ecx, A` / `jmp target`, and only
when the target's own callee-pop count agrees with the emitted parameter list.
A three-instruction body matched instruction for instruction leaves nothing
between the bytes and the transcription for a leaf fixture to catch.

The check that would close the remaining gap is a differential oracle running
each original thunk against its recovered twin with the target patched to
record the receiver. That needs the hybrid process, not the leaf suite, and it
is not built here.
*/

// Each seam defaults to the original method the thunk tail
// jumps to; the hybrid redirects it to the recovered body at
// run time, and the leaf suite rebinds it to a recorder.
func_adjustor_p_u DialogsScalarDeleteTarget =
    original_method<func_adjustor_p_u>(0x00407100);








/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
// RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
// ORIGINAL: 0x004070F0 ??_Gthunk1_Dialogs@@UAEPAXI@Z 0x004070F0-0x004070F8
// symbol    ?adjust_dialogs1_scalar_delete@@YIPAXPAX0I@Z
// size      8 bytes
// prototype void* (__thiscall ??_Gthunk1_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs1_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return (ORIGINAL(object - vtordisp)->*DialogsScalarDeleteTarget)(arg0);
}




















/*
Purpose: Adjust the receiver from Dialogs's thunk2 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
// RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
// ORIGINAL: 0x00407280 ??_Gthunk2_Dialogs@@UAEPAXI@Z 0x00407280-0x00407291
// symbol    ?adjust_dialogs2_scalar_delete@@YIPAXPAX0I@Z
// size      17 bytes
// prototype void* (__thiscall ??_Gthunk2_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs2_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 1096);
    return (ORIGINAL(object - vtordisp - 1092)->*DialogsScalarDeleteTarget)(arg0);
}

/*
Purpose: Adjust the receiver from Dialogs's thunk3 subobject to the one
         Dialogs::scalar_delete expects, then forward unchanged.
// RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
// ORIGINAL: 0x004072A0 ??_Gthunk3_Dialogs@@UAEPAXI@Z 0x004072A0-0x004072AE
// symbol    ?adjust_dialogs3_scalar_delete@@YIPAXPAX0I@Z
// size      14 bytes
// prototype void* (__thiscall ??_Gthunk3_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void * __fastcall adjust_dialogs3_scalar_delete(void *self, void *,
                                                unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return (ORIGINAL(object - vtordisp - 2584)->*DialogsScalarDeleteTarget)(arg0);
}































































/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_redraw expects, then forward unchanged.
// symbol    ?adjust_list_box1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_ListBox@@QAEXXZ)(ListBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): with Win slot12 renamed
//            on_redraw, ListBox::on_redraw overrides that slot through GraphicWin as
//            well as Dialog slot2, so the qualified call's receiver fold moved from
//            the Dialog offset 0xA60 to the GraphicWin offset 0x48. The walk now
//            pre-subtracts 0x48 to cancel it, leaving the image's bare vtordisp thunk;
//            the old 0xA60 walk emitted an extra `sub ecx, 0xa18` here. The compiler
//            now emits its own vtordisp thunk for this slot; this handwritten walk
//            stays the claim host pending coordinated retirement onto it.
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_mouse_move expects, then forward unchanged.
// symbol    ?adjust_list_box1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_left_down expects, then forward unchanged.
// symbol    ?adjust_list_box1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_right_down expects, then forward unchanged.
// symbol    ?adjust_list_box1_on_right_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_down@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from ListBox's thunk1 subobject to the one
         ListBox::on_left_double_click expects, then forward unchanged.
// symbol    ?adjust_list_box1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_ListBox@@QAEXHH@Z)(ListBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/





/*
Purpose: Adjust the receiver from ListBox's thunk3 subobject to the one
         ListBox::on_redraw expects, then forward unchanged.
// symbol    ?adjust_list_box3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_ListBox@@QAEXXZ)(ListBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): the receiver fold is now
//            +0x48 (GraphicWin), so the walk keeps only the Dialog offset 0xA60; the
//            net adjustment is the image's `sub ecx, 0xa18` (the family-wide
//            Dialog-to-GraphicWin gap 0xA60 - 0x48). The pre-rename spelling also
//            pre-subtracted 2584 and emitted `sub ecx, 0x1430` after the rename.
//            The compiler now emits its own two-step thunk for this slot; this
//            handwritten walk stays the claim host pending coordinated retirement.
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_redraw expects, then forward unchanged.
// symbol    ?adjust_check_box1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_CheckBox@@QAEXXZ)(CheckBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): with Win slot12 renamed
//            on_redraw, CheckBox::on_redraw overrides that slot through GraphicWin as
//            well as Dialog slot2, so the qualified call's receiver fold moved from
//            the Dialog offset 0xA34 to the GraphicWin offset 0x1C. The walk now
//            pre-subtracts 0x1C to cancel it, leaving the image's bare vtordisp thunk;
//            the old 0xA34 walk emitted an extra `sub ecx, 0xa18` here. The compiler
//            now emits its own vtordisp thunk for this slot; this handwritten walk
//            stays the claim host pending coordinated retirement onto it.
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_mouse_move expects, then forward unchanged.
// symbol    ?adjust_check_box1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_left_down expects, then forward unchanged.
// symbol    ?adjust_check_box1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from CheckBox's thunk1 subobject to the one
         CheckBox::on_left_double_click expects, then forward unchanged.
// symbol    ?adjust_check_box1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_CheckBox@@QAEXHH@Z)(CheckBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from CheckBox's thunk3 subobject to the one
         CheckBox::on_redraw expects, then forward unchanged.
// symbol    ?adjust_check_box3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_CheckBox@@QAEXXZ)(CheckBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): the receiver fold is now
//            +0x1C (GraphicWin), so the walk keeps only the Dialog offset 0xA34; the
//            net adjustment is the image's `sub ecx, 0xa18` (the family-wide
//            Dialog-to-GraphicWin gap 0xA34 - 0x1C). The pre-rename spelling also
//            pre-subtracted 2584 and emitted `sub ecx, 0x1430` after the rename.
//            The compiler now emits its own two-step thunk for this slot; this
//            handwritten walk stays the claim host pending coordinated retirement.
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_redraw expects, then forward unchanged.
// symbol    ?adjust_sprite_box1_on_redraw@@YIXPAX0@Z
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_SpriteBox@@QAEXXZ)(SpriteBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): with Win slot12 renamed
//            on_redraw, SpriteBox::on_redraw overrides that slot through GraphicWin as
//            well as Dialog slot2, so the qualified call's receiver fold moved from
//            the Dialog offset 0xAA4 to the GraphicWin offset 0x8C. The walk now
//            pre-subtracts 0x8C to cancel it, leaving the image's bare vtordisp thunk;
//            the old 0xAA4 walk emitted an extra `sub ecx, 0xa18` here. The compiler
//            now emits its own vtordisp thunk for this slot; this handwritten walk
//            stays the claim host pending coordinated retirement onto it.
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_mouse_move expects, then forward unchanged.
// symbol    ?adjust_sprite_box1_on_mouse_move@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_down expects, then forward unchanged.
// symbol    ?adjust_sprite_box1_on_left_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_up expects, then forward unchanged.
// symbol    ?adjust_sprite_box1_on_left_up@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_up@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_right_down expects, then forward unchanged.
// symbol    ?adjust_sprite_box1_on_right_down@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_down@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/


/*
Purpose: Adjust the receiver from SpriteBox's thunk1 subobject to the one
         SpriteBox::on_left_double_click expects, then forward unchanged.
// symbol    ?adjust_sprite_box1_on_left_double_click@@YIXPAX0HH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/



/*
Purpose: Adjust the receiver from SpriteBox's thunk3 subobject to the one
         SpriteBox::on_redraw expects, then forward unchanged.
// symbol    ?adjust_sprite_box3_on_redraw@@YIXPAX0@Z
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_SpriteBox@@QAEXXZ)(SpriteBox* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): the receiver fold is now
//            +0x8C (GraphicWin), so the walk keeps only the Dialog offset 0xAA4; the
//            net adjustment is the image's `sub ecx, 0xa18` (the family-wide
//            Dialog-to-GraphicWin gap 0xAA4 - 0x8C). The pre-rename spelling also
//            pre-subtracted 2584 and emitted `sub ecx, 0x1430` after the rename.
//            The compiler now emits its own two-step thunk for this slot; this
//            handwritten walk stays the claim host pending coordinated retirement.
Return Value: n/a
Status: Complete
*/
