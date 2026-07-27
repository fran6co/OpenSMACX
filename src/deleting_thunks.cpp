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
func_deleting_dtor *AlphaMenuDtorTarget =
    (func_deleting_dtor *)0x0051DC10;
func_deleting_dtor *AlphaMovieDtorTarget =
    (func_deleting_dtor *)0x00404310;
func_deleting_dtor *AlphaSaveDtorTarget =
    (func_deleting_dtor *)0x00408170;
func_deleting_dtor *BasePopDtorTarget =
    (func_deleting_dtor *)0x004064D0;
func_deleting_dtor *BaseWinDtorTarget =
    (func_deleting_dtor *)0x00420A70;
func_deleting_dtor *CheckButtonDtorTarget =
    (func_deleting_dtor *)0x004B3EC0;
func_deleting_dtor *CouncWinDtorTarget =
    (func_deleting_dtor *)0x00428550;
func_deleting_dtor *CreditsDtorTarget =
    (func_deleting_dtor *)0x00428E90;
func_deleting_dtor *DatalinkDtorTarget =
    (func_deleting_dtor *)0x00432290;
func_deleting_dtor *DesignWinDtorTarget =
    (func_deleting_dtor *)0x0043ECB0;
func_deleting_dtor *DipEditDtorTarget =
    (func_deleting_dtor *)0x004E2240;
func_deleting_dtor *DiploPopDtorTarget =
    (func_deleting_dtor *)0x00440D50;
func_deleting_dtor *DiploWinDtorTarget =
    (func_deleting_dtor *)0x00444DB0;
func_deleting_dtor *EditBoxDtorTarget =
    (func_deleting_dtor *)0x00408010;
func_deleting_dtor *FameWinDtorTarget =
    (func_deleting_dtor *)0x0044B100;
func_deleting_dtor *GammaDtorTarget =
    (func_deleting_dtor *)0x00456110;
func_deleting_dtor *ImageButtonDtorTarget =
    (func_deleting_dtor *)0x00625310;
func_deleting_dtor *InterludeDtorTarget =
    (func_deleting_dtor *)0x0045F740;
func_deleting_dtor *MainInterfaceDtorTarget =
    (func_deleting_dtor *)0x0045EB80;
func_deleting_dtor *MenuDtorTarget =
    (func_deleting_dtor *)0x005FAD00;
func_deleting_dtor *MonuWinDtorTarget =
    (func_deleting_dtor *)0x00477B10;
func_deleting_dtor *MultiDebugDtorTarget =
    (func_deleting_dtor *)0x005C9E00;
func_deleting_dtor *MultiWinDtorTarget =
    (func_deleting_dtor *)0x0047A430;
func_deleting_dtor *NetMsgDtorTarget =
    (func_deleting_dtor *)0x0047ADA0;
func_deleting_dtor *NetWinDtorTarget =
    (func_deleting_dtor *)0x004834E0;
func_deleting_dtor *NewTechWinDtorTarget =
    (func_deleting_dtor *)0x00484810;
func_deleting_dtor *PickTechDtorTarget =
    (func_deleting_dtor *)0x00488690;
func_deleting_dtor *PickWinDtorTarget =
    (func_deleting_dtor *)0x0048A8E0;
func_deleting_dtor *PopMenuDtorTarget =
    (func_deleting_dtor *)0x00421400;
func_deleting_dtor *PopupDtorTarget =
    (func_deleting_dtor *)0x00406C00;
func_deleting_dtor *PrefWinDtorTarget =
    (func_deleting_dtor *)0x00491DF0;
func_deleting_dtor *ProdPickerDtorTarget =
    (func_deleting_dtor *)0x00421100;
func_deleting_dtor *PushButtonDtorTarget =
    (func_deleting_dtor *)0x0062C010;
func_deleting_dtor *QuayleWinDtorTarget =
    (func_deleting_dtor *)0x00496710;
func_deleting_dtor *ReplayWinDtorTarget =
    (func_deleting_dtor *)0x005ADF10;
func_deleting_dtor *ReportWinDtorTarget =
    (func_deleting_dtor *)0x004AD3B0;
func_deleting_dtor *SelectPartWinDtorTarget =
    (func_deleting_dtor *)0x0043EE10;
func_deleting_dtor *SetupWinDtorTarget =
    (func_deleting_dtor *)0x004AE790;
func_deleting_dtor *SocialWinDtorTarget =
    (func_deleting_dtor *)0x004B3C80;
func_deleting_dtor *StringBoxDtorTarget =
    (func_deleting_dtor *)0x004325C0;
func_deleting_forward_nullary *Sub004042B0Target =
    (func_deleting_forward_nullary *)0x004042B0;
func_deleting_dtor *Sub004C86D0Target =
    (func_deleting_dtor *)0x004C86D0;
func_deleting_forward *Sub00612710Target =
    (func_deleting_forward *)0x00612710;
func_deleting_dtor *Sub00633010Target =
    (func_deleting_dtor *)0x00633010;
func_deleting_forward *Sub006336D0Target =
    (func_deleting_forward *)0x006336D0;
func_deleting_dtor *TutWinDtorTarget =
    (func_deleting_dtor *)0x004BE7D0;
func_deleting_dtor *VoiceRxDtorTarget =
    (func_deleting_dtor *)0x004C8A50;
func_deleting_dtor *VoiceTxDtorTarget =
    (func_deleting_dtor *)0x004C8DB0;
func_deleting_dtor *WorldWinDtorTarget =
    (func_deleting_dtor *)0x004C4A70;

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAlphaMovie@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
Original Offset: 004043A0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_alpha_movie(void *self, void *,
                                           unsigned int arg0) {
    AlphaMovieDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject sub_4042b0 expects, then
         forward unchanged.
Original Offset: 00404420
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_sub_404420(void *self, void *) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return Sub004042B0Target(object - 0x444);
}

/*
Purpose: Step the receiver back to the subobject ??_GAlphaMovie@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00404430
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
Original Offset: 004068E0
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
Original Offset: 00406B40
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_base_pop(void *self, void *,
                                        unsigned int arg0) {
    BasePopDtorTarget(self);
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
Original Offset: 00406BD0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_popup(void *self, void *, unsigned int arg0) {
    PopupDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GBasePop@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 004070B0
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
Original Offset: 004070C0
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
Original Offset: 004070D0
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
Original Offset: 004070E0
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
Original Offset: 00408080
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_edit_box(void *self, void *,
                                        unsigned int arg0) {
    EditBoxDtorTarget(self);
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
Original Offset: 00408140
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_alpha_save(void *self, void *,
                                          unsigned int arg0) {
    AlphaSaveDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GAlphaSave@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00408390
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
Original Offset: 004083A0
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
Original Offset: 004213A0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_prod_picker(void *self, void *,
                                           unsigned int arg0) {
    ProdPickerDtorTarget(self);
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
Original Offset: 004213D0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_base_win(void *self, void *,
                                        unsigned int arg0) {
    BaseWinDtorTarget(self);
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
Original Offset: 00421640
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pop_menu(void *self, void *,
                                        unsigned int arg0) {
    PopMenuDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GBaseWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00421810
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
Original Offset: 00421970
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
Original Offset: 00421980
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
Original Offset: 00421990
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
Original Offset: 004219A0
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
Original Offset: 00428710
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_counc_win(void *self, void *,
                                         unsigned int arg0) {
    CouncWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GCouncWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00428740
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
Original Offset: 00428E60
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_credits(void *self, void *, unsigned int arg0) {
    CreditsDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GCredits@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00428F80
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
Original Offset: 00432770
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_string_box(void *self, void *,
                                          unsigned int arg0) {
    StringBoxDtorTarget(self);
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
Original Offset: 00432800
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_datalink(void *self, void *,
                                        unsigned int arg0) {
    DatalinkDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GStringBox@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00432830
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
Original Offset: 00432840
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
Original Offset: 0043EDB0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_design_win(void *self, void *,
                                          unsigned int arg0) {
    DesignWinDtorTarget(self);
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
Original Offset: 0043EDE0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_select_part_win(void *self, void *,
                                               unsigned int arg0) {
    SelectPartWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GDesignWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 0043EFA0
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
Original Offset: 0043EFB0
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
Original Offset: 00440EC0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_diplo_pop(void *self, void *,
                                         unsigned int arg0) {
    DiploPopDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GDiploPop@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00440EF0
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
Original Offset: 00445080
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_diplo_win(void *self, void *,
                                         unsigned int arg0) {
    DiploWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GDiploWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00445120
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
Original Offset: 00448350
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_effect(void *self, void *, unsigned int arg0) {
    EffectElementTeardown(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at sub_448390: run
         the complete destructor, then release the storage through the
         executable's operator delete only when bit 0 of the flags asks.
Original Offset: 00448390
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
Original Offset: 0044B300
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_fame_win(void *self, void *,
                                        unsigned int arg0) {
    FameWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GFameWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 0044B330
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
Original Offset: 00456280
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_gamma(void *self, void *, unsigned int arg0) {
    GammaDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GGamma@@UAEPAXI@Z expects,
         then forward unchanged.
Original Offset: 004562B0
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
Original Offset: 0045F1C0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_main_interface(void *self, void *,
                                              unsigned int arg0) {
    MainInterfaceDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMainInterface@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 0045F1F0
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
Original Offset: 0045F710
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_interlude(void *self, void *,
                                         unsigned int arg0) {
    InterludeDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GInterlude@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 0045F910
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
Original Offset: 00477DD0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_monu_win(void *self, void *,
                                        unsigned int arg0) {
    MonuWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMonuWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00477E00
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
Original Offset: 0047A6C0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_multi_win(void *self, void *,
                                         unsigned int arg0) {
    MultiWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMultiWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 0047A760
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
Original Offset: 0047AD70
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_net_msg(void *self, void *, unsigned int arg0) {
    NetMsgDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GNetMsg@@UAEPAXI@Z expects,
         then forward unchanged.
Original Offset: 0047AFE0
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
Original Offset: 004837C0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_net_win(void *self, void *, unsigned int arg0) {
    NetWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GNetWin@@UAEPAXI@Z expects,
         then forward unchanged.
Original Offset: 00483840
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
Original Offset: 00484A80
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_new_tech_win(void *self, void *,
                                            unsigned int arg0) {
    NewTechWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GNewTechWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00484AB0
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
Original Offset: 00488660
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pick_tech(void *self, void *,
                                         unsigned int arg0) {
    PickTechDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPickTech@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00488740
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
Original Offset: 0048ADC0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pick_win(void *self, void *,
                                        unsigned int arg0) {
    PickWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPickWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 0048ADF0
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
Original Offset: 0048BF10
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_plan_win(void *self, void *, unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return PlanWinScalarDeleteTarget(object - vtordisp, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPrefWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
Original Offset: 004923E0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_pref_win(void *self, void *,
                                        unsigned int arg0) {
    PrefWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPrefWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 00492410
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
Original Offset: 004968C0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_quayle_win(void *self, void *,
                                          unsigned int arg0) {
    QuayleWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GQuayleWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 004968F0
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
Original Offset: 004AD840
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_report_win(void *self, void *,
                                          unsigned int arg0) {
    ReportWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GReportWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 004AD870
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
Original Offset: 004AE760
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_setup_win(void *self, void *,
                                         unsigned int arg0) {
    SetupWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GSetupWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 004AE9A0
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
Original Offset: 004B3F20
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_check_button(void *self, void *,
                                            unsigned int arg0) {
    CheckButtonDtorTarget(self);
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
Original Offset: 004B3F50
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_social_win(void *self, void *,
                                          unsigned int arg0) {
    SocialWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GCheckButton@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 004B3F80
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
Original Offset: 004B3F90
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
Original Offset: 004BE9F0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_tut_win(void *self, void *, unsigned int arg0) {
    TutWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GTutWin@@UAEPAXI@Z expects,
         then forward unchanged.
Original Offset: 004BEA20
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
Original Offset: 004C4C70
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_world_win(void *self, void *,
                                         unsigned int arg0) {
    WorldWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GWorldWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 004C4CA0
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
Original Offset: 004C9330
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
Original Offset: 004C9360
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_video(void *self, void *, unsigned int arg0) {
    Sub004C86D0Target(self);
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
Original Offset: 004C93B0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_voice_rx(void *self, void *,
                                        unsigned int arg0) {
    VoiceRxDtorTarget(self);
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
Original Offset: 004C93F0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_voice_tx(void *self, void *,
                                        unsigned int arg0) {
    VoiceTxDtorTarget(self);
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
Original Offset: 004E23C0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_dip_edit(void *self, void *,
                                        unsigned int arg0) {
    DipEditDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GDipEdit@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 004E23F0
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
Original Offset: 0051DC90
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_alpha_menu(void *self, void *,
                                          unsigned int arg0) {
    AlphaMenuDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GAlphaMenu@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 0051DF70
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
Original Offset: 005AE0B0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_replay_win(void *self, void *,
                                          unsigned int arg0) {
    ReplayWinDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GReplayWin@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 005AE0E0
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
Original Offset: 005C9E80
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_multi_debug(void *self, void *,
                                           unsigned int arg0) {
    MultiDebugDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMultiDebug@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 005C9EB0
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
Original Offset: 005D7140
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
Original Offset: 005D7160
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
Original Offset: 005E3610
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
Original Offset: 005F8610
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_win(void *self, void *, unsigned int arg0) {
    WinOriginalDestructor(self);
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
Original Offset: 005FA770
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
Original Offset: 005FA790
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
Original Offset: 005FC6D0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_menu(void *self, void *, unsigned int arg0) {
    MenuDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMenu@@UAEPAXI@Z expects,
         then forward unchanged.
Original Offset: 005FC6F0
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
Original Offset: 00607CC0
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
Original Offset: 00607CE0
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
Original Offset: 00612700
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_sub_612700(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    const int32_t vtordisp =
        *reinterpret_cast<const int32_t *>(object - 4);
    return Sub00612710Target(object - vtordisp, arg0);
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GImageButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
Original Offset: 006256D0
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_image_button(void *self, void *,
                                            unsigned int arg0) {
    ImageButtonDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GImageButton@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 006256F0
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
Original Offset: 0062C830
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_push_button(void *self, void *,
                                           unsigned int arg0) {
    PushButtonDtorTarget(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPushButton@@UAEPAXI@Z
         expects, then forward unchanged.
Original Offset: 0062C850
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
Original Offset: 00633160
Return Value: the object pointer
Status: Complete
*/
void *__fastcall scalar_delete_sub_633160(void *self, void *,
                                          unsigned int arg0) {
    Sub00633010Target(self);
    if (arg0 & 1) {
        ScrollOperatorDelete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject sub_6336d0 expects, then
         forward unchanged.
Original Offset: 00633730
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_sub_633730(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return Sub006336D0Target(object - 0x58, arg0);
}

/*
Purpose: Step the receiver back to the subobject sub_633160 expects, then
         forward unchanged.
Original Offset: 00633740
Return Value: the forwarded call's
Status: Complete
*/
void *__fastcall adjust_this_sub_633740(void *self, void *,
                                        unsigned int arg0) {
    uint8_t *const object = static_cast<uint8_t *>(self);
    return scalar_delete_sub_633160(object - 0x444, nullptr, arg0);
}
