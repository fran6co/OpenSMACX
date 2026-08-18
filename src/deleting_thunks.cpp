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
#include "deleting_thunks.h"
#include "adjustor_thunks.h"
#include "ambience.h"
#include "basebutton.h"
#include "buffer.h"
#include "flatbutton.h"
#include "fx.h"
#include "graphicwin.h"
#include "pulldown.h"
#include "scroll.h"

/*
Verification note: the generated leaf suite proves that each scalar deleting
body runs its destructor exactly once on the receiver it was given, releases
the storage on bit 0 of the flags and on nothing else, and returns the
object; and that each receiver fixup subtracts the amount recorded for it
before forwarding its argument and its result untouched. It does NOT
independently confirm that those amounts are what the original subtracted,
because the suite's expectation comes from the same decoding the bodies do.
What pins the model is the acceptance check in
tools/generate_deleting_thunks.py: a row is emitted only when its bytes match
one of the templates exactly under two independent decoders, and only when
the popped byte count agrees between the thunk and what it calls.

Fourteen bodies are outside the leaf suite's reach and are covered only by
the batch gate: the nine whose destructor is already source-owned, and the
five fixups that forward into them. Driving those from a leaf fixture would
run a real destructor over a synthetic object.

The check that would close the remaining gap is a differential oracle running
each original thunk against its recovered twin inside the hybrid process with
the destructor patched to record its receiver. That is not built here.
*/

// Each seam defaults to the original body the thunk
// reached; the hybrid redirects it to a recovered one at run
// time, and the leaf suite rebinds it to a recorder.
func_deleting_dtor AlphaMenuDtorTarget =
    original_method<func_deleting_dtor>(0x0051DC10);
func_deleting_dtor AlphaMovieDtorTarget =
    original_method<func_deleting_dtor>(0x00404310);
func_deleting_dtor AlphaSaveDtorTarget =
    original_method<func_deleting_dtor>(0x00408170);
func_deleting_dtor BasePopDtorTarget =
    original_method<func_deleting_dtor>(0x004064D0);
func_deleting_dtor BaseWinDtorTarget =
    original_method<func_deleting_dtor>(0x00420A70);
func_deleting_dtor CheckButtonDtorTarget =
    original_method<func_deleting_dtor>(0x004B3EC0);
func_deleting_dtor CouncWinDtorTarget =
    original_method<func_deleting_dtor>(0x00428550);
func_deleting_dtor CreditsDtorTarget =
    original_method<func_deleting_dtor>(0x00428E90);
func_deleting_dtor DatalinkDtorTarget =
    original_method<func_deleting_dtor>(0x00432290);
func_deleting_dtor DesignWinDtorTarget =
    original_method<func_deleting_dtor>(0x0043ECB0);
func_deleting_dtor DipEditDtorTarget =
    original_method<func_deleting_dtor>(0x004E2240);
func_deleting_dtor DiploPopDtorTarget =
    original_method<func_deleting_dtor>(0x00440D50);
func_deleting_dtor DiploWinDtorTarget =
    original_method<func_deleting_dtor>(0x00444DB0);
func_deleting_dtor EditBoxDtorTarget =
    original_method<func_deleting_dtor>(0x00408010);
func_deleting_dtor FameWinDtorTarget =
    original_method<func_deleting_dtor>(0x0044B100);
func_deleting_dtor GammaDtorTarget =
    original_method<func_deleting_dtor>(0x00456110);
func_deleting_dtor ImageButtonDtorTarget =
    original_method<func_deleting_dtor>(0x00625310);
func_deleting_dtor InterludeDtorTarget =
    original_method<func_deleting_dtor>(0x0045F740);
func_deleting_dtor MainInterfaceDtorTarget =
    original_method<func_deleting_dtor>(0x0045EB80);
func_deleting_dtor MenuDtorTarget =
    original_method<func_deleting_dtor>(0x005FAD00);
func_deleting_dtor MonuWinDtorTarget =
    original_method<func_deleting_dtor>(0x00477B10);
func_deleting_dtor MultiDebugDtorTarget =
    original_method<func_deleting_dtor>(0x005C9E00);
func_deleting_dtor MultiWinDtorTarget =
    original_method<func_deleting_dtor>(0x0047A430);
func_deleting_dtor NetMsgDtorTarget =
    original_method<func_deleting_dtor>(0x0047ADA0);
func_deleting_dtor NetWinDtorTarget =
    original_method<func_deleting_dtor>(0x004834E0);
func_deleting_dtor NewTechWinDtorTarget =
    original_method<func_deleting_dtor>(0x00484810);
func_deleting_dtor PickTechDtorTarget =
    original_method<func_deleting_dtor>(0x00488690);
func_deleting_dtor PickWinDtorTarget =
    original_method<func_deleting_dtor>(0x0048A8E0);
func_deleting_dtor PopMenuDtorTarget =
    original_method<func_deleting_dtor>(0x00421400);
func_deleting_dtor PopupDtorTarget =
    original_method<func_deleting_dtor>(0x00406C00);
func_deleting_dtor PrefWinDtorTarget =
    original_method<func_deleting_dtor>(0x00491DF0);
func_deleting_dtor ProdPickerDtorTarget =
    original_method<func_deleting_dtor>(0x00421100);
func_deleting_dtor PushButtonDtorTarget =
    original_method<func_deleting_dtor>(0x0062C010);
func_deleting_dtor QuayleWinDtorTarget =
    original_method<func_deleting_dtor>(0x00496710);
func_deleting_dtor ReplayWinDtorTarget =
    original_method<func_deleting_dtor>(0x005ADF10);
func_deleting_dtor ReportWinDtorTarget =
    original_method<func_deleting_dtor>(0x004AD3B0);
func_deleting_dtor SelectPartWinDtorTarget =
    original_method<func_deleting_dtor>(0x0043EE10);
func_deleting_dtor SetupWinDtorTarget =
    original_method<func_deleting_dtor>(0x004AE790);
func_deleting_dtor SocialWinDtorTarget =
    original_method<func_deleting_dtor>(0x004B3C80);
func_deleting_dtor StringBoxDtorTarget =
    original_method<func_deleting_dtor>(0x004325C0);
func_deleting_forward_nullary Sub004042B0Target =
    original_method<func_deleting_forward_nullary>(0x004042B0);
func_deleting_dtor Sub004C86D0Target =
    original_method<func_deleting_dtor>(0x004C86D0);
func_deleting_forward Sub00612710Target =
    original_method<func_deleting_forward>(0x00612710);
func_deleting_dtor Sub00633010Target =
    original_method<func_deleting_dtor>(0x00633010);
func_deleting_forward Sub006336D0Target =
    original_method<func_deleting_forward>(0x006336D0);
func_deleting_dtor TutWinDtorTarget =
    original_method<func_deleting_dtor>(0x004BE7D0);
func_deleting_dtor VoiceRxDtorTarget =
    original_method<func_deleting_dtor>(0x004C8A50);
func_deleting_dtor VoiceTxDtorTarget =
    original_method<func_deleting_dtor>(0x004C8DB0);
func_deleting_dtor WorldWinDtorTarget =
    original_method<func_deleting_dtor>(0x004C4A70);

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAlphaMovie@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004043A0 ??_GAlphaMovie@@UAEPAXI@Z 0x004043A0-0x004043C1
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00404310 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_alpha_movie(void *self, void *,
                                           unsigned int arg0) {
    (ORIGINAL(self)->*AlphaMovieDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject sub_4042b0 expects, then
         forward unchanged.
// ORIGINAL: 0x00404420 sub_404420 0x00404420-0x0040442B
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_sub_404420(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return (ORIGINAL(object - 0x444)->*Sub004042B0Target)();
}

/*
Purpose: Step the receiver back to the subobject ??_GAlphaMovie@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00404430 ??_GAlphaMovie@@WEEE@AEPAXI@Z 0x00404430-0x0040443B
// CORRECTED from ??3AlphaMovie@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004043A0` into
//   ??_GAlphaMovie@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_alpha_movie(void *self, void *,
                                         unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_alpha_movie(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GFlatButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004068E0 ??_GFlatButton@@UAEPAXI@Z 0x004068E0-0x00406901
// size      33 bytes
// prototype void* (__thiscall ??_GFlatButton@@UAEPAXI@Z)(FlatButton* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00406880 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_flat_button(void *self, void *,
                                           unsigned int arg0) {
    flat_button_destructor_redirect(
        reinterpret_cast<FlatButton *>(self), nullptr);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GBasePop@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00406B40 ??_GBasePop@@UAEPAXI@Z 0x00406B40-0x00406B61
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004064D0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_base_pop(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*BasePopDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPopup@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00406BD0 ??_GPopup@@UAEPAXI@Z 0x00406BD0-0x00406BF1
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00406C00 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_popup(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*PopupDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GBasePop@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004070B0 ??_GBasePop@@WEEE@AEPAXI@Z 0x004070B0-0x004070BB
// CORRECTED from ??3BasePop@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00406B40` into
//   ??_GBasePop@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_base_pop(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_base_pop(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GScroll@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004070C0 ??_GScroll@@WEEE@AEPAXI@Z 0x004070C0-0x004070CB
// CORRECTED from ??3Scroll@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00406F20` into
//   ??_GScroll@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_scroll(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scroll_scalar_dtor_redirect(
        reinterpret_cast<Scroll *>(object - 0x444), nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GPopup@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004070D0 ??_GPopup@@WEEE@AEPAXI@Z 0x004070D0-0x004070DB
// CORRECTED from ??3Popup@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00406BD0` into
//   ??_GPopup@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_popup(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_popup(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GFlatButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004070E0 ??_GFlatButton@@WEEE@AEPAXI@Z 0x004070E0-0x004070EB
// CORRECTED from ??3FlatButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004068E0` into
//   ??_GFlatButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_flat_button(void *self, void *,
                                         unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_flat_button(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GEditBox@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00408080 ??_GEditBox@@UAEPAXI@Z 0x00408080-0x004080A1
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00408010 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_edit_box(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*EditBoxDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAlphaSave@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x00408140 ??_GAlphaSave@@UAEPAXI@Z 0x00408140-0x00408161
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00408170 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_alpha_save(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*AlphaSaveDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GAlphaSave@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00408390 ??_GAlphaSave@@WEEE@AEPAXI@Z 0x00408390-0x0040839B
// CORRECTED from ??3AlphaSave@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00408140` into
//   ??_GAlphaSave@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_alpha_save(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_alpha_save(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GEditBox@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004083A0 ??_GEditBox@@WEEE@AEPAXI@Z 0x004083A0-0x004083AB
// CORRECTED from ??3EditBox@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00408080` into
//   ??_GEditBox@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_edit_box(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_edit_box(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GProdPicker@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004213A0 ??_GProdPicker@@UAEPAXI@Z 0x004213A0-0x004213C1
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00421100 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_prod_picker(void *self, void *,
                                           unsigned int arg0) {
    (ORIGINAL(self)->*ProdPickerDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GBaseWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004213D0 ??_GBaseWin@@UAEPAXI@Z 0x004213D0-0x004213F1
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00420A70 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_base_win(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*BaseWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPopMenu@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00421640 ??_GPopMenu@@UAEPAXI@Z 0x00421640-0x00421661
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00421400 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pop_menu(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*PopMenuDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GBaseWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421810 ??_GBaseWin@@WEEE@AEPAXI@Z 0x00421810-0x0042181B
// CORRECTED from ??3BaseWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004213D0` into
//   ??_GBaseWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_base_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_base_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GProdPicker@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421970 ??_GProdPicker@@WEEE@AEPAXI@Z 0x00421970-0x0042197B
// CORRECTED from ??3ProdPicker@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004213A0` into
//   ??_GProdPicker@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_prod_picker(void *self, void *,
                                         unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_prod_picker(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GPopMenu@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421980 PopMenu::delete1 0x00421980-0x0042198B
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_pop_menu_delete1(void *self, void *,
                                              unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_pop_menu(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GPopMenu@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421990 PopMenu::delete2 0x00421990-0x0042199B
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_pop_menu_delete2(void *self, void *,
                                              unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_pop_menu(object - 0x537C, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GPopMenu@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004219A0 PopMenu::delete3 0x004219A0-0x004219AB
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_pop_menu_delete3(void *self, void *,
                                              unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_pop_menu(object - 0x57C0, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GCouncWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00428710 ??_GCouncWin@@UAEPAXI@Z 0x00428710-0x00428731
// size      33 bytes
// prototype void* (__thiscall ??_GCouncWin@@UAEPAXI@Z)(CouncWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00428550 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_counc_win(void *self, void *,
                                         unsigned int arg0) {
    (ORIGINAL(self)->*CouncWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GCouncWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00428740 ??_GCouncWin@@WEEE@AEPAXI@Z 0x00428740-0x0042874B
// CORRECTED from ??3CouncWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00428710` into
//   ??_GCouncWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_counc_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_counc_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GCredits@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00428E60 ??_GCredits@@UAEPAXI@Z 0x00428E60-0x00428E81
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00428E90 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_credits(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*CreditsDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GCredits@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00428F80 ??_GCredits@@WEEE@AEPAXI@Z 0x00428F80-0x00428F8B
// CORRECTED from ??3Credits@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00428E60` into
//   ??_GCredits@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_credits(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_credits(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GStringBox@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x00432770 ??_GStringBox@@UAEPAXI@Z 0x00432770-0x00432791
// size      33 bytes
// prototype void* (__thiscall ??_GStringBox@@UAEPAXI@Z)(StringBox* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004325C0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_string_box(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*StringBoxDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDatalink@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00432800 ??_GDatalink@@UAEPAXI@Z 0x00432800-0x00432821
// size      33 bytes
// prototype void* (__thiscall ??_GDatalink@@UAEPAXI@Z)(Datalink* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00432290 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_datalink(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*DatalinkDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GStringBox@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00432830 ??_GStringBox@@WEEE@AEPAXI@Z 0x00432830-0x0043283B
// CORRECTED from ??3StringBox@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00432770` into
//   ??_GStringBox@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_string_box(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_string_box(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GDatalink@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00432840 ??_GDatalink@@WEEE@AEPAXI@Z 0x00432840-0x0043284B
// CORRECTED from ??3Datalink@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00432800` into
//   ??_GDatalink@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_datalink(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_datalink(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDesignWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x0043EDB0 ??_GDesignWin@@UAEPAXI@Z 0x0043EDB0-0x0043EDD1
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043ECB0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_design_win(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*DesignWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GSelectPartWin@@UAEPAXI@Z: run the complete destructor, then
         release the storage through the executable's operator delete only when
         bit 0 of the flags asks.
// ORIGINAL: 0x0043EDE0 ??_GSelectPartWin@@UAEPAXI@Z 0x0043EDE0-0x0043EE01
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043EE10 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_select_part_win(void *self, void *,
                                               unsigned int arg0) {
    (ORIGINAL(self)->*SelectPartWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GDesignWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0043EFA0 ??_GDesignWin@@WEEE@AEPAXI@Z 0x0043EFA0-0x0043EFAB
// CORRECTED from ??3DesignWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0043EDB0` into
//   ??_GDesignWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_design_win(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_design_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GSelectPartWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0043EFB0 ??_GSelectPartWin@@WEEE@AEPAXI@Z 0x0043EFB0-0x0043EFBB
// CORRECTED from ??3SelectPartWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0043EDE0` into
//   ??_GSelectPartWin@@UAEPAXI@Z, which executes `ret 4`; no stack
//   access and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_select_part_win(void *self, void *,
                                             unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_select_part_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDiploPop@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00440EC0 ??_GDiploPop@@UAEPAXI@Z 0x00440EC0-0x00440EE1
// size      33 bytes
// prototype void* (__thiscall ??_GDiploPop@@UAEPAXI@Z)(DiploPop* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00440D50 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_diplo_pop(void *self, void *,
                                         unsigned int arg0) {
    (ORIGINAL(self)->*DiploPopDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GDiploPop@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00440EF0 ??_GDiploPop@@WEEE@AEPAXI@Z 0x00440EF0-0x00440EFB
// CORRECTED from ??3DiploPop@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00440EC0` into
//   ??_GDiploPop@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_diplo_pop(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_diplo_pop(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDiploWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00445080 ??_GDiploWin@@UAEPAXI@Z 0x00445080-0x004450A1
// size      33 bytes
// prototype void* (__thiscall ??_GDiploWin@@UAEPAXI@Z)(DiploWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00444DB0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_diplo_win(void *self, void *,
                                         unsigned int arg0) {
    (ORIGINAL(self)->*DiploWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GDiploWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00445120 ??_GDiploWin@@WEEE@AEPAXI@Z 0x00445120-0x0044512B
// CORRECTED from ??3DiploWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00445080` into
//   ??_GDiploWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_diplo_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_diplo_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GEffect@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00448350 ??_GEffect@@UAEPAXI@Z 0x00448350-0x00448371
// size      33 bytes
// prototype void* (__thiscall ??_GEffect@@UAEPAXI@Z)(Effect* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004482C0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_effect(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*EffectElementTeardown)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at sub_448390: run
         the complete destructor, then release the storage through the
         executable's operator delete only when bit 0 of the flags asks.
// ORIGINAL: 0x00448390 sub_448390 0x00448390-0x004483B1
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004483C0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_sub_448390(void *self, void *,
                                          unsigned int arg0) {
    ambience_dtor_redirect(reinterpret_cast<Ambience *>(self),
                           nullptr);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GFameWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x0044B300 ??_GFameWin@@UAEPAXI@Z 0x0044B300-0x0044B321
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0044B100 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_fame_win(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*FameWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GFameWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0044B330 ??_GFameWin@@WEEE@AEPAXI@Z 0x0044B330-0x0044B33B
// CORRECTED from ??3FameWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0044B300` into
//   ??_GFameWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_fame_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_fame_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GGamma@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00456280 ??_GGamma@@UAEPAXI@Z 0x00456280-0x004562A1
// size      33 bytes
// prototype void* (__thiscall ??_GGamma@@UAEPAXI@Z)(Gamma* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00456110 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_gamma(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*GammaDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GGamma@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004562B0 ??_GGamma@@WEEE@AEPAXI@Z 0x004562B0-0x004562BB
// CORRECTED from ??3Gamma@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00456280` into
//   ??_GGamma@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_gamma(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_gamma(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMainInterface@@UAEPAXI@Z: run the complete destructor, then
         release the storage through the executable's operator delete only when
         bit 0 of the flags asks.
// ORIGINAL: 0x0045F1C0 ??_GMainInterface@@UAEPAXI@Z 0x0045F1C0-0x0045F1E1
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045EB80 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_main_interface(void *self, void *,
                                              unsigned int arg0) {
    (ORIGINAL(self)->*MainInterfaceDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMainInterface@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0045F1F0 ??_GMainInterface@@WEEE@AEPAXI@Z 0x0045F1F0-0x0045F1FB
// CORRECTED from ??3MainInterface@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0045F1C0` into
//   ??_GMainInterface@@UAEPAXI@Z, which executes `ret 4`; no stack
//   access and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_main_interface(void *self, void *,
                                            unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_main_interface(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GInterlude@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x0045F710 ??_GInterlude@@UAEPAXI@Z 0x0045F710-0x0045F731
// size      33 bytes
// prototype void* (__thiscall ??_GInterlude@@UAEPAXI@Z)(Interlude* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0045F740 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_interlude(void *self, void *,
                                         unsigned int arg0) {
    (ORIGINAL(self)->*InterludeDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GInterlude@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0045F910 ??_GInterlude@@WEEE@AEPAXI@Z 0x0045F910-0x0045F91B
// CORRECTED from ??3Interlude@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0045F710` into
//   ??_GInterlude@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_interlude(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_interlude(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMonuWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00477DD0 ??_GMonuWin@@UAEPAXI@Z 0x00477DD0-0x00477DF1
// size      33 bytes
// prototype void* (__thiscall ??_GMonuWin@@UAEPAXI@Z)(MonuWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00477B10 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_monu_win(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*MonuWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMonuWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00477E00 ??_GMonuWin@@WEEE@AEPAXI@Z 0x00477E00-0x00477E0B
// CORRECTED from ??3MonuWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00477DD0` into
//   ??_GMonuWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_monu_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_monu_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMultiWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x0047A6C0 ??_GMultiWin@@UAEPAXI@Z 0x0047A6C0-0x0047A6E1
// size      33 bytes
// prototype void* (__thiscall ??_GMultiWin@@UAEPAXI@Z)(MultiWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047A430 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_multi_win(void *self, void *,
                                         unsigned int arg0) {
    (ORIGINAL(self)->*MultiWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMultiWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0047A760 ??_GMultiWin@@WEEE@AEPAXI@Z 0x0047A760-0x0047A76B
// CORRECTED from ??3MultiWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0047A6C0` into
//   ??_GMultiWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_multi_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_multi_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GNetMsg@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x0047AD70 ??_GNetMsg@@UAEPAXI@Z 0x0047AD70-0x0047AD91
// size      33 bytes
// prototype void* (__thiscall ??_GNetMsg@@UAEPAXI@Z)(NetMsg* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047ADA0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_net_msg(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*NetMsgDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GNetMsg@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x0047AFE0 ??_GNetMsg@@WEEE@AEPAXI@Z 0x0047AFE0-0x0047AFEB
// CORRECTED from ??3NetMsg@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0047AD70` into
//   ??_GNetMsg@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_net_msg(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_net_msg(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GNetWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004837C0 ??_GNetWin@@UAEPAXI@Z 0x004837C0-0x004837E1
// size      33 bytes
// prototype void* (__thiscall ??_GNetWin@@UAEPAXI@Z)(NetWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004834E0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_net_win(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*NetWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GNetWin@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x00483840 ??_GNetWin@@WEEE@AEPAXI@Z 0x00483840-0x0048384B
// CORRECTED from ??3NetWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004837C0` into
//   ??_GNetWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_net_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_net_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GNewTechWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x00484A80 ??_GNewTechWin@@UAEPAXI@Z 0x00484A80-0x00484AA1
// size      33 bytes
// prototype void* (__thiscall ??_GNewTechWin@@UAEPAXI@Z)(NewTechWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00484810 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_new_tech_win(void *self, void *,
                                            unsigned int arg0) {
    (ORIGINAL(self)->*NewTechWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GNewTechWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00484AB0 ??_GNewTechWin@@WEEE@AEPAXI@Z 0x00484AB0-0x00484ABB
// CORRECTED from ??3NewTechWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00484A80` into
//   ??_GNewTechWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_new_tech_win(void *self, void *,
                                          unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_new_tech_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPickTech@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00488660 ??_GPickTech@@UAEPAXI@Z 0x00488660-0x00488681
// size      33 bytes
// prototype void* (__thiscall ??_GPickTech@@UAEPAXI@Z)(PickTech* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00488690 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pick_tech(void *self, void *,
                                         unsigned int arg0) {
    (ORIGINAL(self)->*PickTechDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPickTech@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00488740 ??_GPickTech@@WEEE@AEPAXI@Z 0x00488740-0x0048874B
// CORRECTED from ??3PickTech@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00488660` into
//   ??_GPickTech@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_pick_tech(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_pick_tech(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPickWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x0048ADC0 ??_GPickWin@@UAEPAXI@Z 0x0048ADC0-0x0048ADE1
// size      33 bytes
// prototype void* (__thiscall ??_GPickWin@@UAEPAXI@Z)(PickWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048A8E0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pick_win(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*PickWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPickWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0048ADF0 ??_GPickWin@@WEEE@AEPAXI@Z 0x0048ADF0-0x0048ADFB
// CORRECTED from ??3PickWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0048ADC0` into
//   ??_GPickWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_pick_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_pick_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back by the vtordisp stored ahead of it to the one
         ??_GPlanWin@@UAEPAXI@Z expects, then forward unchanged.
// ORIGINAL: 0x0048BF10 ??_GPlanWin@@$4PPPPPPPM@A@AEPAXI@Z 0x0048BF10-0x0048BF18
// CORRECTED from ??3PlanWin@@SAXPAXI@Z
//   8 bytes, `sub ecx, dword ptr [ecx - 4]; jmp 0x0048BF20` into
//   ??_GPlanWin@@UAEPAXI@Z, which executes `ret 4`. `$4PPPPPPPM@A@` re-
//   demangles to vtordisp{-4, 0} - the field at -4 and no further
//   adjustment, which is exactly the one subtraction the body performs
// size      8 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_plan_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return (ORIGINAL(object - vtordisp)->*PlanWinScalarDeleteTarget)(arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPrefWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004923E0 ??_GPrefWin@@UAEPAXI@Z 0x004923E0-0x00492401
// size      33 bytes
// prototype void* (__thiscall ??_GPrefWin@@UAEPAXI@Z)(PrefWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00491DF0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pref_win(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*PrefWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPrefWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00492410 ??_GPrefWin@@WEEE@AEPAXI@Z 0x00492410-0x0049241B
// CORRECTED from ??3PrefWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004923E0` into
//   ??_GPrefWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_pref_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_pref_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GQuayleWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004968C0 ??_GQuayleWin@@UAEPAXI@Z 0x004968C0-0x004968E1
// size      33 bytes
// prototype void* (__thiscall ??_GQuayleWin@@UAEPAXI@Z)(QuayleWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00496710 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_quayle_win(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*QuayleWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GQuayleWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004968F0 ??_GQuayleWin@@WEEE@AEPAXI@Z 0x004968F0-0x004968FB
// CORRECTED from ??3QuayleWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004968C0` into
//   ??_GQuayleWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_quayle_win(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_quayle_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GReportWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004AD840 ??_GReportWin@@UAEPAXI@Z 0x004AD840-0x004AD861
// size      33 bytes
// prototype void* (__thiscall ??_GReportWin@@UAEPAXI@Z)(ReportWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004AD3B0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_report_win(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*ReportWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GReportWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004AD870 ??_GReportWin@@WEEE@AEPAXI@Z 0x004AD870-0x004AD87B
// CORRECTED from ??3ReportWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004AD840` into
//   ??_GReportWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_report_win(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_report_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GSetupWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004AE760 ??_GSetupWin@@UAEPAXI@Z 0x004AE760-0x004AE781
// size      33 bytes
// prototype void* (__thiscall ??_GSetupWin@@UAEPAXI@Z)(SetupWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004AE790 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_setup_win(void *self, void *,
                                         unsigned int arg0) {
    (ORIGINAL(self)->*SetupWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GSetupWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004AE9A0 ??_GSetupWin@@WEEE@AEPAXI@Z 0x004AE9A0-0x004AE9AB
// CORRECTED from ??3SetupWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004AE760` into
//   ??_GSetupWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_setup_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_setup_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GCheckButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004B3F20 ??_GCheckButton@@UAEPAXI@Z 0x004B3F20-0x004B3F41
// size      33 bytes
// prototype void* (__thiscall ??_GCheckButton@@UAEPAXI@Z)(CheckButton* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004B3EC0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_check_button(void *self, void *,
                                            unsigned int arg0) {
    (ORIGINAL(self)->*CheckButtonDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GSocialWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004B3F50 ??_GSocialWin@@UAEPAXI@Z 0x004B3F50-0x004B3F71
// size      33 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004B3C80 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_social_win(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*SocialWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GCheckButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004B3F80 ??_GCheckButton@@WEEE@AEPAXI@Z 0x004B3F80-0x004B3F8B
// CORRECTED from ??3CheckButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004B3F20` into
//   ??_GCheckButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_check_button(void *self, void *,
                                          unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_check_button(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back to the subobject ??_GSocialWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004B3F90 ??_GSocialWin@@WEEE@AEPAXI@Z 0x004B3F90-0x004B3F9B
// CORRECTED from ??3SocialWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004B3F50` into
//   ??_GSocialWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_social_win(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_social_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GTutWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004BE9F0 ??_GTutWin@@UAEPAXI@Z 0x004BE9F0-0x004BEA11
// size      33 bytes
// prototype void* (__thiscall ??_GTutWin@@UAEPAXI@Z)(TutWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004BE7D0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_tut_win(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*TutWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GTutWin@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004BEA20 ??_GTutWin@@WEEE@AEPAXI@Z 0x004BEA20-0x004BEA2B
// CORRECTED from ??3TutWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004BE9F0` into
//   ??_GTutWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_tut_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_tut_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GWorldWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C4C70 ??_GWorldWin@@UAEPAXI@Z 0x004C4C70-0x004C4C91
// size      33 bytes
// prototype void* (__thiscall ??_GWorldWin@@UAEPAXI@Z)(WorldWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C4A70 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_world_win(void *self, void *,
                                         unsigned int arg0) {
    (ORIGINAL(self)->*WorldWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GWorldWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004C4CA0 ??_GWorldWin@@WEEE@AEPAXI@Z 0x004C4CA0-0x004C4CAB
// CORRECTED from ??3WorldWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004C4C70` into
//   ??_GWorldWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_world_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_world_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAmbience@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C9330 ??_GAmbience@@UAEPAXI@Z 0x004C9330-0x004C9351
// size      33 bytes
// prototype void* (__thiscall ??_GAmbience@@UAEPAXI@Z)(Ambience* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C7670 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_ambience(void *self, void *,
                                        unsigned int arg0) {
    ambience_dtor_redirect(reinterpret_cast<Ambience *>(self),
                           nullptr);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GVideo@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z 0x004C9360-0x004C9381
// size      33 bytes
// prototype void* (__thiscall ??_GVideo@@UAEPAXI@Z)(Video* this, int8)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C86D0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_video(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*Sub004C86D0Target)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GVoiceRx@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C93B0 ??_GVoiceRx@@UAEPAXI@Z 0x004C93B0-0x004C93D1
// size      33 bytes
// prototype void* (__thiscall ??_GVoiceRx@@UAEPAXI@Z)(VoiceRx* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8A50 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_voice_rx(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*VoiceRxDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GVoiceTx@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C93F0 ??_GVoiceTx@@UAEPAXI@Z 0x004C93F0-0x004C9411
// size      33 bytes
// prototype void* (__thiscall ??_GVoiceTx@@UAEPAXI@Z)(VoiceTx* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8DB0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_voice_tx(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*VoiceTxDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDipEdit@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004E23C0 ??_GDipEdit@@UAEPAXI@Z 0x004E23C0-0x004E23E1
// size      33 bytes
// prototype void* (__thiscall ??_GDipEdit@@UAEPAXI@Z)(DipEdit* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004E2240 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_dip_edit(void *self, void *,
                                        unsigned int arg0) {
    (ORIGINAL(self)->*DipEditDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GDipEdit@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004E23F0 ??_GDipEdit@@WEEE@AEPAXI@Z 0x004E23F0-0x004E23FB
// CORRECTED from ??3DipEdit@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004E23C0` into
//   ??_GDipEdit@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_dip_edit(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_dip_edit(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAlphaMenu@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x0051DC90 ??_GAlphaMenu@@UAEPAXI@Z 0x0051DC90-0x0051DCB1
// size      33 bytes
// prototype void* (__thiscall ??_GAlphaMenu@@UAEPAXI@Z)(AlphaMenu* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0051DC10 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_alpha_menu(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*AlphaMenuDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GAlphaMenu@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0051DF70 ??_GAlphaMenu@@WEEE@AEPAXI@Z 0x0051DF70-0x0051DF7B
// CORRECTED from ??3AlphaMenu@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0051DC90` into
//   ??_GAlphaMenu@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_alpha_menu(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_alpha_menu(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GReplayWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x005AE0B0 ??_GReplayWin@@UAEPAXI@Z 0x005AE0B0-0x005AE0D1
// size      33 bytes
// prototype void* (__thiscall ??_GReplayWin@@UAEPAXI@Z)(ReplayWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ADF10 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_replay_win(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*ReplayWinDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GReplayWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005AE0E0 ??_GReplayWin@@WEEE@AEPAXI@Z 0x005AE0E0-0x005AE0EB
// CORRECTED from ??3ReplayWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005AE0B0` into
//   ??_GReplayWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_replay_win(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_replay_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMultiDebug@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x005C9E80 ??_GMultiDebug@@UAEPAXI@Z 0x005C9E80-0x005C9EA1
// size      33 bytes
// prototype void* (__thiscall ??_GMultiDebug@@UAEPAXI@Z)(MultiDebug* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005C9E00 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_multi_debug(void *self, void *,
                                           unsigned int arg0) {
    (ORIGINAL(self)->*MultiDebugDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMultiDebug@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005C9EB0 ??_GMultiDebug@@WEEE@AEPAXI@Z 0x005C9EB0-0x005C9EBB
// CORRECTED from ??3MultiDebug@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005C9E80` into
//   ??_GMultiDebug@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_multi_debug(void *self, void *,
                                         unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_multi_debug(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GGraphicWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x005D7140 ??_GGraphicWin@@UAEPAXI@Z 0x005D7140-0x005D715E
// size      30 bytes
// prototype void* (__thiscall ??_GGraphicWin@@UAEPAXI@Z)(GraphicWin* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4DD0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_graphic_win(void *self, void *,
                                           unsigned int arg0) {
    graphic_win_destructor_redirect(
        reinterpret_cast<GraphicWin *>(self), nullptr);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GGraphicWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005D7160 ??_GGraphicWin@@WEEE@AEPAXI@Z 0x005D7160-0x005D716B
// CORRECTED from ??3GraphicWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005D7140` into
//   ??_GGraphicWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_graphic_win(void *self, void *,
                                         unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_graphic_win(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GBuffer@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x005E3610 ??_GBuffer@@UAEPAXI@Z 0x005E3610-0x005E362E
// size      30 bytes
// prototype void* (__thiscall ??_GBuffer@@UAEPAXI@Z)(Buffer* this, int8)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7410 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_buffer(void *self, void *, unsigned int arg0) {
    buffer_destructor_redirect(reinterpret_cast<Buffer *>(self),
                               nullptr);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x005F8610 ??_GWin@@UAEPAXI@Z 0x005F8610-0x005F862E
// size      30 bytes
// prototype void* (__thiscall ??_GWin@@UAEPAXI@Z)(Win* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EBC90 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_win(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*WinOriginalDestructor)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPullDown@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x005FA770 ??_GPullDown@@UAEPAXI@Z 0x005FA770-0x005FA78E
// size      30 bytes
// prototype void* (__thiscall ??_GPullDown@@UAEPAXI@Z)(PullDown* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F88A0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pull_down(void *self, void *,
                                         unsigned int arg0) {
    pull_down_destructor_redirect(
        reinterpret_cast<PullDown *>(self), nullptr);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPullDown@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005FA790 ??_GPullDown@@WEEE@AEPAXI@Z 0x005FA790-0x005FA79B
// CORRECTED from ??3PullDown@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005FA770` into
//   ??_GPullDown@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_pull_down(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_pull_down(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMenu@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x005FC6D0 ??_GMenu@@UAEPAXI@Z 0x005FC6D0-0x005FC6EE
// size      30 bytes
// prototype void* (__thiscall ??_GMenu@@UAEPAXI@Z)(Menu* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAD00 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_menu(void *self, void *, unsigned int arg0) {
    (ORIGINAL(self)->*MenuDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMenu@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x005FC6F0 ??_GMenu@@WEEE@AEPAXI@Z 0x005FC6F0-0x005FC6FB
// CORRECTED from ??3Menu@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005FC6D0` into ??_GMenu@@UAEPAXI@Z,
//   which executes `ret 4`; no stack access and the receiver stays in
//   ECX. `WEEE@` re-demangles to adjustor{1092} and 1092 == 0x444, the
//   constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_menu(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_menu(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GBaseButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x00607CC0 ??_GBaseButton@@UAEPAXI@Z 0x00607CC0-0x00607CDE
// size      30 bytes
// prototype void* (__thiscall ??_GBaseButton@@UAEPAXI@Z)(BaseButton* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00607040 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_base_button(void *self, void *,
                                           unsigned int arg0) {
    base_button_destructor_redirect(
        reinterpret_cast<BaseButton *>(self), nullptr);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GBaseButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00607CE0 ??_GBaseButton@@WEEE@AEPAXI@Z 0x00607CE0-0x00607CEB
// CORRECTED from ??3BaseButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00607CC0` into
//   ??_GBaseButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_base_button(void *self, void *,
                                         unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_base_button(object - 0x444, nullptr, arg0);
}

/*
Purpose: Step the receiver back by the vtordisp stored ahead of it to the one
         sub_612710 expects, then forward unchanged.
// ORIGINAL: 0x00612700 sub_612700 0x00612700-0x00612708
// size      8 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_sub_612700(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return (ORIGINAL(object - vtordisp)->*Sub00612710Target)(arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GImageButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x006256D0 ??_GImageButton@@UAEPAXI@Z 0x006256D0-0x006256EE
// size      30 bytes
// prototype void* (__thiscall ??_GImageButton@@UAEPAXI@Z)(ImageButton* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625310 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_image_button(void *self, void *,
                                            unsigned int arg0) {
    (ORIGINAL(self)->*ImageButtonDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GImageButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x006256F0 ??_GImageButton@@WEEE@AEPAXI@Z 0x006256F0-0x006256FB
// CORRECTED from ??3ImageButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x006256D0` into
//   ??_GImageButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_image_button(void *self, void *,
                                          unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_image_button(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPushButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x0062C830 ??_GPushButton@@UAEPAXI@Z 0x0062C830-0x0062C84E
// size      30 bytes
// prototype void* (__thiscall ??_GPushButton@@UAEPAXI@Z)(PushButton* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062C010 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_push_button(void *self, void *,
                                           unsigned int arg0) {
    (ORIGINAL(self)->*PushButtonDtorTarget)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPushButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0062C850 ??_GPushButton@@WEEE@AEPAXI@Z 0x0062C850-0x0062C85B
// CORRECTED from ??3PushButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0062C830` into
//   ??_GPushButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_push_button(void *self, void *,
                                         unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_push_button(object - 0x444, nullptr, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at sub_633160: run
         the complete destructor, then release the storage through the
         executable's operator delete only when bit 0 of the flags asks.
// ORIGINAL: 0x00633160 sub_633160 0x00633160-0x0063317E
// size      30 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633010 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_sub_633160(void *self, void *,
                                          unsigned int arg0) {
    (ORIGINAL(self)->*Sub00633010Target)();
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject sub_6336d0 expects, then
         forward unchanged.
// ORIGINAL: 0x00633730 sub_633730 0x00633730-0x00633738
// size      8 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_sub_633730(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return (ORIGINAL(object - 0x58)->*Sub006336D0Target)(arg0);
}

/*
Purpose: Step the receiver back to the subobject sub_633160 expects, then
         forward unchanged.
// ORIGINAL: 0x00633740 sub_633740 0x00633740-0x0063374B
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_sub_633740(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_sub_633160(object - 0x444, nullptr, arg0);
}
