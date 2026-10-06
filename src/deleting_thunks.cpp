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
#include "alphasave.h"
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
#include "hypothesis_layouts.h"
#include "famewin.h"
#include "editbox.h"
#include "net_class.h"

/*
These adapters are transitional source owners, not a replacement for genuine
C++ lifetime reconstruction. Move each original identity and its full spans
to the owning class when its complete destruction is implemented and the
compiler emits the corresponding entry. Preserve historical claims and
findings during that migration.

A fixture which derives receiver adjustments from the same decoding as its
candidate cannot independently establish those adjustments. Byte matching,
host lifecycle projections and original ABI/reference evidence are distinct
checks; none alone establishes complete game acceptance.
*/

// Remaining executable-address targets are unfinished ownership seams.
func_deleting_dtor PopupDtorTarget =
    original_method<func_deleting_dtor>(0x00406C00);






/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GEditBox@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
         ??_GPopMenu@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
Purpose: The compiler-generated scalar deleting destructor at sub_448390: run
         the complete destructor, then release the storage through the
         executable's operator delete only when bit 0 of the flags asks.
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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



// Interlude members and compiler-generated lifetime code are owned by interlude.cpp.

// Interlude members and compiler-generated lifetime code are owned by interlude.cpp.

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GMonuWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
         ??_GPickTech@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
Purpose: Step the receiver back by the vtordisp stored ahead of it to the one
         ??_GPlanWin@@UAEPAXI@Z expects, then forward unchanged.
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
         ??_GAlphaMenu@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks.
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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
         ??_GWin@@UAEPAXI@Z: run the complete destructor, then release the
         storage through the executable's operator delete only when bit 0 of
         the flags asks.
// RECOVERY_ROLE: SCAFFOLD Hand-written deletion adapter; original lifetime identity still needs its genuine class-generated implementation.
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




// EditGroup deletion and receiver adjustment are compiler-owned in editgroup.cpp.
