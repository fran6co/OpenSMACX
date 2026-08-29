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
 * WHAT BLOCKS THE REMAINING ??_G BODIES, MEASURED 2026-08-26.
 *
 * 65 `??_G` scalar deleting destructors live here, 48 claimed and 17 not.
 * The unclaimed ones are not seventeen problems and they are not a byte
 * grind: sampled bodies measure 2-4 of 11-14, which is the signature of a
 * MISSING DECLARATION rather than a wrong spelling.
 *
 * The image spells every one of them `U` - `??_GFlatButton@@UAEPAXI@Z`,
 * `??_GWin@@UAEPAXI@Z` - so the destructor is VIRTUAL in the original, and
 * a virtual destructor is a thing THE COMPILER EMITS `??_G` FOR. Every
 * hand-written body in this file exists because the tree does not declare
 * one. win.h:463 already says so about slot 0: "??_GWin - the compiler
 * emits this".
 *
 * The chain is `FlatButton : BaseButton : GraphicWin : ... : Win`, and NOT
 * ONE of them declares `virtual ~X()` - `~Win()`, `~GraphicWin()`,
 * `~BaseButton()`, `~FlatButton()` are all non-virtual, while win.h
 * declares 94 other virtuals. So the fix is a single named change,
 * `virtual ~Win()` at vtable slot 0 (0x0066FDD0 holds 0x005F8610, which IS
 * ??_GWin), propagating down the hierarchy.
 *
 * AND THAT DIAGNOSIS IS WRONG. MEASURED, NOT REASONED, THE SAME DAY.
 *
 * `virtual ~Win()` was applied in an isolated worktree and is a COMPLETE
 * NO-OP. It builds clean with no other declaration change; win.cpp holds 89
 * claims either way; a 40-body sample measures 40 of 40 with ZERO
 * regressions; and the three blocked bodies above measure 4/14, 4/14 and
 * 2/11 - digit for digit what they measure without it.
 *
 * The reason is visible in the build's own symbols: `Win::`scalar deleting
 * destructor'` is ALREADY emitted, and the count of emitted scalar deleting
 * destructors is 298 with the change and 298 without. The compiler was
 * never withholding `??_G`. Whatever `??_GFlatButton` needs, it is not this.
 *
 * So the paragraph above had the risk right and the remedy wrong: the
 * change is harmless rather than dangerous, and useless rather than
 * unblocking. A stated remedy is a claim like any other, and this one is
 * refuted. What actually blocks these seventeen is still unknown - start by
 * asking why a body whose count is already close (15 compiled against the
 * image's 14) disagrees from instruction 0, rather than by declaring
 * anything.
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
#include "alphamenu.h"
#include "alphamovie.h"
#include "basepop.h"
#include "basewin.h"
#include "checkbutton.h"
#include "councwin.h"
#include "credits.h"
#include "datalink.h"
#include "designwin.h"
#include "dipedit.h"
#include "diplopop.h"
#include "diplowin.h"
#include "gamma.h"
#include "maininterface.h"
#include "menu.h"
#include "multidebug.h"
#include "netmsg.h"
#include "netwin.h"
#include "newtechwin.h"
#include "pickwin.h"
#include "popmenu.h"
#include "prodpicker.h"
#include "pushbutton.h"
#include "replaywin.h"
#include "reportwin.h"
#include "setupwin.h"
#include "socialwin.h"
#include "stringbox.h"
#include "tutwin.h"
#include "worldwin.h"
#include "hypothesis_layouts.h"
#include "editbox.h"
#include "net_class.h"

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
func_deleting_dtor PopupDtorTarget =
    original_method<func_deleting_dtor>(0x00406C00);
func_deleting_forward_nullary Sub004042B0Target =
    original_method<func_deleting_forward_nullary>(0x004042B0);
func_deleting_forward Sub00612710Target =
    original_method<func_deleting_forward>(0x00612710);
func_deleting_forward Sub006336D0Target =
    original_method<func_deleting_forward>(0x006336D0);

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAlphaMovie@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004043A0 ??_GAlphaMovie@@UAEPAXI@Z 0x004043A0-0x004043C1 BYTE_EXACT
// symbol    ?scalar_delete_alpha_movie@@YIPAXPAX0I@Z
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
    static_cast<AlphaMovie *>(self)->AlphaMovie::~AlphaMovie();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject sub_4042b0 expects, then
         forward unchanged.
// ORIGINAL: 0x00404420 sub_404420 0x00404420-0x0040442B
// symbol    ?adjust_this_sub_404420@@YIPAXPAX0@Z
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
Purpose: The compiler-generated scalar deleting destructor at
         ??_GFlatButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004068E0 ??_GFlatButton@@UAEPAXI@Z 0x004068E0-0x00406901 BYTE_EXACT
// symbol    ?scalar_delete_flat_button@@YIPAXPAX0I@Z
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
    // THE METHOD, not the redirect: `nullptr` for the redirect's unused
    // second argument materialises `xor edx, edx`, which the image does not
    // emit. Same fix as SelectPartWin::close.
    reinterpret_cast<FlatButton *>(self)->destroy();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GBasePop@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00406B40 ??_GBasePop@@UAEPAXI@Z 0x00406B40-0x00406B61 BYTE_EXACT
// symbol    ?scalar_delete_base_pop@@YIPAXPAX0I@Z
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
    static_cast<BasePop *>(self)->BasePop::~BasePop();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPopup@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00406BD0 ??_GPopup@@UAEPAXI@Z 0x00406BD0-0x00406BF1 BYTE_EXACT
// symbol    ?scalar_delete_popup@@YIPAXPAX0I@Z
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
    // A NAMED DESTRUCTOR, not the seam - the image emits `call rel32`.
    reinterpret_cast<Popup *>(self)->Popup::~Popup();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GEditBox@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00408080 ??_GEditBox@@UAEPAXI@Z 0x00408080-0x004080A1 BYTE_EXACT
// symbol    ?scalar_delete_edit_box@@YIPAXPAX0I@Z
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
    static_cast<EditBox *>(self)->EditBox::~EditBox();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAlphaSave@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x00408140 ??_GAlphaSave@@UAEPAXI@Z 0x00408140-0x00408161 BYTE_EXACT
// symbol    ?scalar_delete_alpha_save@@YIPAXPAX0I@Z
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
    static_cast<AlphaSave *>(self)->AlphaSave::~AlphaSave();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GAlphaSave@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00408390 ??_GAlphaSave@@WEEE@AEPAXI@Z 0x00408390-0x0040839B
// symbol    ?adjust_this_alpha_save@@YIPAXPAX0I@Z
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
// symbol    ?adjust_this_edit_box@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x004213A0 ??_GProdPicker@@UAEPAXI@Z 0x004213A0-0x004213C1 BYTE_EXACT
// symbol    ?scalar_delete_prod_picker@@YIPAXPAX0I@Z
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
    static_cast<ProdPicker *>(self)->ProdPicker::~ProdPicker();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GBaseWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004213D0 ??_GBaseWin@@UAEPAXI@Z 0x004213D0-0x004213F1 BYTE_EXACT
// symbol    ?scalar_delete_base_win@@YIPAXPAX0I@Z
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
    static_cast<BaseWin *>(self)->BaseWin::~BaseWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPopMenu@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00421640 ??_GPopMenu@@UAEPAXI@Z 0x00421640-0x00421661 BYTE_EXACT
// symbol    ?scalar_delete_pop_menu@@YIPAXPAX0I@Z
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
    static_cast<PopMenu *>(self)->PopMenu::~PopMenu();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPopMenu@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00421980 PopMenu::delete1 0x00421980-0x0042198B
// symbol    ?adjust_this_pop_menu_delete1@@YIPAXPAX0I@Z
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
// symbol    ?adjust_this_pop_menu_delete2@@YIPAXPAX0I@Z
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
// symbol    ?adjust_this_pop_menu_delete3@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x00428710 ??_GCouncWin@@UAEPAXI@Z 0x00428710-0x00428731 BYTE_EXACT
// symbol    ?scalar_delete_counc_win@@YIPAXPAX0I@Z
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
    static_cast<CouncWin *>(self)->CouncWin::~CouncWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GCredits@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00428E60 ??_GCredits@@UAEPAXI@Z 0x00428E60-0x00428E81 BYTE_EXACT
// symbol    ?scalar_delete_credits@@YIPAXPAX0I@Z
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
    static_cast<Credits *>(self)->Credits::~Credits();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GStringBox@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x00432770 ??_GStringBox@@UAEPAXI@Z 0x00432770-0x00432791 BYTE_EXACT
// symbol    ?scalar_delete_string_box@@YIPAXPAX0I@Z
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
    static_cast<StringBox *>(self)->StringBox::~StringBox();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDatalink@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00432800 ??_GDatalink@@UAEPAXI@Z 0x00432800-0x00432821 BYTE_EXACT
// symbol    ?scalar_delete_datalink@@YIPAXPAX0I@Z
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
    static_cast<Datalink *>(self)->Datalink::~Datalink();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDesignWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x0043EDB0 ??_GDesignWin@@UAEPAXI@Z 0x0043EDB0-0x0043EDD1 BYTE_EXACT
// symbol    ?scalar_delete_design_win@@YIPAXPAX0I@Z
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
    static_cast<DesignWin *>(self)->DesignWin::~DesignWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GSelectPartWin@@UAEPAXI@Z: run the complete destructor, then
         release the storage through the executable's operator delete only when
         bit 0 of the flags asks.
// ORIGINAL: 0x0043EDE0 ??_GSelectPartWin@@UAEPAXI@Z 0x0043EDE0-0x0043EE01 BYTE_EXACT
// symbol    ?scalar_delete_select_part_win@@YIPAXPAX0I@Z
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
    static_cast<SelectPartWin *>(self)->SelectPartWin::~SelectPartWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GSelectPartWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0043EFB0 ??_GSelectPartWin@@WEEE@AEPAXI@Z 0x0043EFB0-0x0043EFBB
// symbol    ?adjust_this_select_part_win@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x00440EC0 ??_GDiploPop@@UAEPAXI@Z 0x00440EC0-0x00440EE1 BYTE_EXACT
// symbol    ?scalar_delete_diplo_pop@@YIPAXPAX0I@Z
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
    static_cast<DiploPop *>(self)->DiploPop::~DiploPop();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDiploWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00445080 ??_GDiploWin@@UAEPAXI@Z 0x00445080-0x004450A1 BYTE_EXACT
// symbol    ?scalar_delete_diplo_win@@YIPAXPAX0I@Z
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
    static_cast<DiploWin *>(self)->DiploWin::~DiploWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GEffect@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00448350 ??_GEffect@@UAEPAXI@Z 0x00448350-0x00448371 BYTE_EXACT
// symbol    ?scalar_delete_effect@@YIPAXPAX0I@Z
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
    static_cast<Effect *>(self)->Effect::~Effect();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at sub_448390: run
         the complete destructor, then release the storage through the
         executable's operator delete only when bit 0 of the flags asks.
// ORIGINAL: 0x00448390 sub_448390 0x00448390-0x004483B1 BYTE_EXACT
// symbol    ?scalar_delete_sub_448390@@YIPAXPAX0I@Z
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
    // THE METHOD, not the redirect: `nullptr` for the redirect's unused
    // second argument materialises `xor edx, edx`, which the image does not
    // emit. Same fix as SelectPartWin::close.
    // QUALIFIED: Sound's destructor is virtual since the sound family
    // declared its real virtual set, so the unqualified `~Ambience()`
    // dispatched through the vtable (a `push 0` and an indirect call the
    // image's direct `call` to the complete destructor never has).
    reinterpret_cast<Ambience *>(self)->Ambience::~Ambience();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GFameWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x0044B300 ??_GFameWin@@UAEPAXI@Z 0x0044B300-0x0044B321 BYTE_EXACT
// symbol    ?scalar_delete_fame_win@@YIPAXPAX0I@Z
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
    static_cast<FameWin *>(self)->FameWin::~FameWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GFameWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0044B330 ??_GFameWin@@WEEE@AEPAXI@Z 0x0044B330-0x0044B33B
// symbol    ?adjust_this_fame_win@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x00456280 ??_GGamma@@UAEPAXI@Z 0x00456280-0x004562A1 BYTE_EXACT
// symbol    ?scalar_delete_gamma@@YIPAXPAX0I@Z
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
    static_cast<Gamma *>(self)->Gamma::~Gamma();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMainInterface@@UAEPAXI@Z: run the complete destructor, then
         release the storage through the executable's operator delete only when
         bit 0 of the flags asks.
// ORIGINAL: 0x0045F1C0 ??_GMainInterface@@UAEPAXI@Z 0x0045F1C0-0x0045F1E1 BYTE_EXACT
// symbol    ?scalar_delete_main_interface@@YIPAXPAX0I@Z
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
    static_cast<MainInterface *>(self)->MainInterface::~MainInterface();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GInterlude@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x0045F710 ??_GInterlude@@UAEPAXI@Z 0x0045F710-0x0045F731 BYTE_EXACT
// symbol    ?scalar_delete_interlude@@YIPAXPAX0I@Z
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
    static_cast<Interlude *>(self)->Interlude::~Interlude();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GInterlude@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0045F910 ??_GInterlude@@WEEE@AEPAXI@Z 0x0045F910-0x0045F91B
// symbol    ?adjust_this_interlude@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x00477DD0 ??_GMonuWin@@UAEPAXI@Z 0x00477DD0-0x00477DF1 BYTE_EXACT
// symbol    ?scalar_delete_monu_win@@YIPAXPAX0I@Z
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
    static_cast<MonuWin *>(self)->MonuWin::~MonuWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMonuWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00477E00 ??_GMonuWin@@WEEE@AEPAXI@Z 0x00477E00-0x00477E0B
// symbol    ?adjust_this_monu_win@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x0047A6C0 ??_GMultiWin@@UAEPAXI@Z 0x0047A6C0-0x0047A6E1 BYTE_EXACT
// symbol    ?scalar_delete_multi_win@@YIPAXPAX0I@Z
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
    static_cast<MultiWin *>(self)->MultiWin::~MultiWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GMultiWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0047A760 ??_GMultiWin@@WEEE@AEPAXI@Z 0x0047A760-0x0047A76B
// symbol    ?adjust_this_multi_win@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x0047AD70 ??_GNetMsg@@UAEPAXI@Z 0x0047AD70-0x0047AD91 BYTE_EXACT
// symbol    ?scalar_delete_net_msg@@YIPAXPAX0I@Z
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
    static_cast<NetMsg *>(self)->NetMsg::~NetMsg();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GNetWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004837C0 ??_GNetWin@@UAEPAXI@Z 0x004837C0-0x004837E1 BYTE_EXACT
// symbol    ?scalar_delete_net_win@@YIPAXPAX0I@Z
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
    static_cast<NetWin *>(self)->NetWin::~NetWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GNewTechWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x00484A80 ??_GNewTechWin@@UAEPAXI@Z 0x00484A80-0x00484AA1 BYTE_EXACT
// symbol    ?scalar_delete_new_tech_win@@YIPAXPAX0I@Z
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
    static_cast<NewTechWin *>(self)->NewTechWin::~NewTechWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPickTech@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x00488660 ??_GPickTech@@UAEPAXI@Z 0x00488660-0x00488681 BYTE_EXACT
// symbol    ?scalar_delete_pick_tech@@YIPAXPAX0I@Z
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
    static_cast<PickTech *>(self)->PickTech::~PickTech();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPickTech@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00488740 ??_GPickTech@@WEEE@AEPAXI@Z 0x00488740-0x0048874B
// symbol    ?adjust_this_pick_tech@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x0048ADC0 ??_GPickWin@@UAEPAXI@Z 0x0048ADC0-0x0048ADE1 BYTE_EXACT
// symbol    ?scalar_delete_pick_win@@YIPAXPAX0I@Z
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
    static_cast<PickWin *>(self)->PickWin::~PickWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back by the vtordisp stored ahead of it to the one
         ??_GPlanWin@@UAEPAXI@Z expects, then forward unchanged.
// ORIGINAL: 0x0048BF10 ??_GPlanWin@@$4PPPPPPPM@A@AEPAXI@Z 0x0048BF10-0x0048BF18
// symbol    ?adjust_this_plan_win@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x004923E0 ??_GPrefWin@@UAEPAXI@Z 0x004923E0-0x00492401 BYTE_EXACT
// symbol    ?scalar_delete_pref_win@@YIPAXPAX0I@Z
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
    static_cast<PrefWin *>(self)->PrefWin::~PrefWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GPrefWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00492410 ??_GPrefWin@@WEEE@AEPAXI@Z 0x00492410-0x0049241B
// symbol    ?adjust_this_pref_win@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x004968C0 ??_GQuayleWin@@UAEPAXI@Z 0x004968C0-0x004968E1 BYTE_EXACT
// symbol    ?scalar_delete_quayle_win@@YIPAXPAX0I@Z
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
    static_cast<QuayleWin *>(self)->QuayleWin::~QuayleWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GQuayleWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004968F0 ??_GQuayleWin@@WEEE@AEPAXI@Z 0x004968F0-0x004968FB
// symbol    ?adjust_this_quayle_win@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x004AD840 ??_GReportWin@@UAEPAXI@Z 0x004AD840-0x004AD861 BYTE_EXACT
// symbol    ?scalar_delete_report_win@@YIPAXPAX0I@Z
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
    static_cast<ReportWin *>(self)->ReportWin::~ReportWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GSetupWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004AE760 ??_GSetupWin@@UAEPAXI@Z 0x004AE760-0x004AE781 BYTE_EXACT
// symbol    ?scalar_delete_setup_win@@YIPAXPAX0I@Z
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
    static_cast<SetupWin *>(self)->SetupWin::~SetupWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GCheckButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004B3F20 ??_GCheckButton@@UAEPAXI@Z 0x004B3F20-0x004B3F41 BYTE_EXACT
// symbol    ?scalar_delete_check_button@@YIPAXPAX0I@Z
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
    static_cast<CheckButton *>(self)->CheckButton::~CheckButton();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GSocialWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x004B3F50 ??_GSocialWin@@UAEPAXI@Z 0x004B3F50-0x004B3F71 BYTE_EXACT
// symbol    ?scalar_delete_social_win@@YIPAXPAX0I@Z
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
    static_cast<SocialWin *>(self)->SocialWin::~SocialWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GTutWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004BE9F0 ??_GTutWin@@UAEPAXI@Z 0x004BE9F0-0x004BEA11 BYTE_EXACT
// symbol    ?scalar_delete_tut_win@@YIPAXPAX0I@Z
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
    static_cast<TutWin *>(self)->TutWin::~TutWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GWorldWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C4C70 ??_GWorldWin@@UAEPAXI@Z 0x004C4C70-0x004C4C91 BYTE_EXACT
// symbol    ?scalar_delete_world_win@@YIPAXPAX0I@Z
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
    static_cast<WorldWin *>(self)->WorldWin::~WorldWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAmbience@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C9330 ??_GAmbience@@UAEPAXI@Z 0x004C9330-0x004C9351 BYTE_EXACT
// symbol    ?scalar_delete_ambience@@YIPAXPAX0I@Z
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
    // THE METHOD, not the redirect: `nullptr` for the redirect's unused
    // second argument materialises `xor edx, edx`, which the image does not
    // emit. Same fix as SelectPartWin::close.
    // QUALIFIED: Sound's destructor is virtual since the sound family
    // declared its real virtual set, so the unqualified `~Ambience()`
    // dispatched through the vtable (a `push 0` and an indirect call the
    // image's direct `call` to the complete destructor never has).
    reinterpret_cast<Ambience *>(self)->Ambience::~Ambience();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GVideo@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z 0x004C9360-0x004C9381 BYTE_EXACT
// symbol    ?scalar_delete_video@@YIPAXPAX0I@Z
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
    sub_4c86d0(self);
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GVoiceRx@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C93B0 ??_GVoiceRx@@UAEPAXI@Z 0x004C93B0-0x004C93D1 BYTE_EXACT
// symbol    ?scalar_delete_voice_rx@@YIPAXPAX0I@Z
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
    static_cast<VoiceRx *>(self)->VoiceRx::~VoiceRx();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GVoiceTx@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004C93F0 ??_GVoiceTx@@UAEPAXI@Z 0x004C93F0-0x004C9411 BYTE_EXACT
// symbol    ?scalar_delete_voice_tx@@YIPAXPAX0I@Z
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
    static_cast<VoiceTx *>(self)->VoiceTx::~VoiceTx();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDipEdit@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x004E23C0 ??_GDipEdit@@UAEPAXI@Z 0x004E23C0-0x004E23E1 BYTE_EXACT
// symbol    ?scalar_delete_dip_edit@@YIPAXPAX0I@Z
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
    static_cast<DipEdit *>(self)->DipEdit::~DipEdit();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GAlphaMenu@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x0051DC90 ??_GAlphaMenu@@UAEPAXI@Z 0x0051DC90-0x0051DCB1 BYTE_EXACT
// symbol    ?scalar_delete_alpha_menu@@YIPAXPAX0I@Z
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
    static_cast<AlphaMenu *>(self)->AlphaMenu::~AlphaMenu();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject ??_GAlphaMenu@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0051DF70 ??_GAlphaMenu@@WEEE@AEPAXI@Z 0x0051DF70-0x0051DF7B
// symbol    ?adjust_this_alpha_menu@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x005AE0B0 ??_GReplayWin@@UAEPAXI@Z 0x005AE0B0-0x005AE0D1 BYTE_EXACT
// symbol    ?scalar_delete_replay_win@@YIPAXPAX0I@Z
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
    static_cast<ReplayWin *>(self)->ReplayWin::~ReplayWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMultiDebug@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x005C9E80 ??_GMultiDebug@@UAEPAXI@Z 0x005C9E80-0x005C9EA1 BYTE_EXACT
// symbol    ?scalar_delete_multi_debug@@YIPAXPAX0I@Z
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
    static_cast<MultiDebug *>(self)->MultiDebug::~MultiDebug();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GGraphicWin@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x005D7140 ??_GGraphicWin@@UAEPAXI@Z 0x005D7140-0x005D715E
// symbol    ?scalar_delete_graphic_win@@YIPAXPAX0I@Z
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
    reinterpret_cast<GraphicWin *>(self)->~GraphicWin();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GBuffer@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x005E3610 ??_GBuffer@@UAEPAXI@Z 0x005E3610-0x005E362E BYTE_EXACT
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
    // THE METHOD, not the redirect: `nullptr` for the redirect's unused
    // second argument materialises `xor edx, edx`, which the image does not
    // emit. Same fix as SelectPartWin::close.
    reinterpret_cast<Buffer *>(self)->Buffer::~Buffer();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x005F8610 ??_GWin@@UAEPAXI@Z 0x005F8610-0x005F862E BYTE_EXACT
// symbol    ?scalar_delete_win@@YIPAXPAX0I@Z
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
    // A NAMED DESTRUCTOR, not the seam: the image emits `call rel32` here and
    // a pointer-to-member variable can only emit `call dword ptr [...]`.
    reinterpret_cast<Win *>(self)->Win::~Win();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPullDown@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x005FA770 ??_GPullDown@@UAEPAXI@Z 0x005FA770-0x005FA78E BYTE_EXACT
// symbol    ?scalar_delete_pull_down@@YIPAXPAX0I@Z
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
    // THE METHOD, not the redirect: `nullptr` for the redirect's unused
    // second argument materialises `xor edx, edx`, which the image does not
    // emit. Same fix as SelectPartWin::close.
    reinterpret_cast<PullDown *>(self)->destroy();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMenu@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// ORIGINAL: 0x005FC6D0 ??_GMenu@@UAEPAXI@Z 0x005FC6D0-0x005FC6EE BYTE_EXACT
// symbol    ?scalar_delete_menu@@YIPAXPAX0I@Z
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
    static_cast<Menu *>(self)->Menu::~Menu();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GBaseButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x00607CC0 ??_GBaseButton@@UAEPAXI@Z 0x00607CC0-0x00607CDE BYTE_EXACT
// symbol    ?scalar_delete_base_button@@YIPAXPAX0I@Z
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
    // THE METHOD, not the redirect: `nullptr` for the redirect's unused
    // second argument materialises `xor edx, edx`, which the image does not
    // emit. Same fix as SelectPartWin::close.
    reinterpret_cast<BaseButton *>(self)->destroy();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back by the vtordisp stored ahead of it to the one
         sub_612710 expects, then forward unchanged.
// ORIGINAL: 0x00612700 sub_612700 0x00612700-0x00612708
// symbol    ?adjust_this_sub_612700@@YIPAXPAX0I@Z
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
// ORIGINAL: 0x006256D0 ??_GImageButton@@UAEPAXI@Z 0x006256D0-0x006256EE BYTE_EXACT
// symbol    ?scalar_delete_image_button@@YIPAXPAX0I@Z
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
    static_cast<ImageButton *>(self)->ImageButton::~ImageButton();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GPushButton@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// ORIGINAL: 0x0062C830 ??_GPushButton@@UAEPAXI@Z 0x0062C830-0x0062C84E BYTE_EXACT
// symbol    ?scalar_delete_push_button@@YIPAXPAX0I@Z
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
    static_cast<PushButton *>(self)->PushButton::~PushButton();
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: The compiler-generated scalar deleting destructor at sub_633160: run
         the complete destructor, then release the storage through the
         executable's operator delete only when bit 0 of the flags asks.
// ORIGINAL: 0x00633160 sub_633160 0x00633160-0x0063317E BYTE_EXACT
// symbol    ?scalar_delete_sub_633160@@YIPAXPAX0I@Z
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
    sub_633010(self);
    if (arg0 & 1) {
        operator delete(self);
    }
    return self;
}

/*
Purpose: Step the receiver back to the subobject sub_6336d0 expects, then
         forward unchanged.
// ORIGINAL: 0x00633730 sub_633730 0x00633730-0x00633738
// symbol    ?adjust_this_sub_633730@@YIPAXPAX0I@Z
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
// symbol    ?adjust_this_sub_633740@@YIPAXPAX0I@Z
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
