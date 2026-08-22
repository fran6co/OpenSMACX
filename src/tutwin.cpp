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
#include "tutwin.h"
#include "win.h"
#include "mapwin.h"
#include "vtable_shim.h"

// The tutorial's "already shown" marker, cleared alongside the window's own
// state. Bound at its canonical address because nothing here owns it yet.
uint32_t TutWinShownFlag;  // 0x008CC244

/*
Purpose: Reset the tutorial window - four fields to zero, four to -1, and the
         shared shown-marker cleared with them.
// ORIGINAL: 0x004BA720 ?UNK1@TutWin@@QAEXXZ 0x004BA720-0x004BA761 BYTE_EXACT
// size      65 bytes
// prototype void (__thiscall ?UNK1@TutWin@@QAEXXZ)(TutWin* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void TutWin::UNK1() {
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;
}

/*
Purpose: Store a single value at 0x53D4.
// ORIGINAL: 0x004BDDD0 ?UNK3@TutWin@@QAEXH@Z 0x004BDDD0-0x004BDDE0 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ?UNK3@TutWin@@QAEXH@Z)(TutWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void TutWin::UNK3(int a1) {
    field_53D4_ = a1;
}

void __fastcall tut_win_unk1_redirect(TutWin *self, void *) {
    self->UNK1();
}

void __fastcall tut_win_unk3_redirect(TutWin *self, void *, int a1) {
    self->UNK3(a1);
}

/*
 * Four 67-byte clones differing only in which fixed window they finish on.
 * Each centres the rectangle on both axes and hands the pair to that window's
 * client_to_screen. The centring is inlined four times rather than shared,
 * matching the original, which carries four copies.
 *
 * The halving is the compiler's signed divide-by-two - `sub eax,esi; cdq;
 * sub eax,edx; sar eax,1` - so it truncates toward zero rather than flooring,
 * and the subtraction wraps. Modelled here the way win.cpp's own midpoint()
 * models the identical idiom: unsigned arithmetic throughout, with the sign
 * bit carried back over the shift.
 */


/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the iface window.
// ORIGINAL: 0x004BC5A0 ?iface_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC5A0-0x004BC5E3 BYTE_EXACT
// LEVER: plain `left + (right - left) / 2` signed division - the image
//        already emits the `cdq; sub; sar` fixup itself, so the hand-rolled
//        unsigned-with-sign-carried-over-the-shift trick this body used to
//        model the SAME idiom compiled to different instructions. Matches
//        the sibling `base_rect`/`soc_rect`/`des_rect`, already byte-exact
//        on this same lever.
// symbol    ?iface_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?iface_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x007AE820,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::iface_rect(RECT *rect, int *x, int *y) {
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinIfaceWindow->client_to_screen(x, y);
}

void __fastcall tut_win_iface_rect_redirect(
        TutWin *self, void *, RECT *rect, int *x, int *y) {
    self->iface_rect(rect, x, y);
}


/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the base window.
// ORIGINAL: 0x004BC5F0 ?base_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC5F0-0x004BC633 BYTE_EXACT
// symbol    ?base_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?base_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x006A7628,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::base_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinBaseWindow->client_to_screen(x, y);
}

void __fastcall tut_win_base_rect_redirect(
        TutWin *self, void *, RECT *rect, int *x, int *y) {
    self->base_rect(rect, x, y);
}


/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the soc window.
// ORIGINAL: 0x004BC640 ?soc_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC640-0x004BC683 BYTE_EXACT
// symbol    ?soc_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?soc_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x008A6270,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::soc_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinSocWindow->client_to_screen(x, y);
}

void __fastcall tut_win_soc_rect_redirect(
        TutWin *self, void *, RECT *rect, int *x, int *y) {
    self->soc_rect(rect, x, y);
}


/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the des window.
// ORIGINAL: 0x004BC690 ?des_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC690-0x004BC6D3 BYTE_EXACT
// symbol    ?des_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?des_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x0071F2B0,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::des_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinDesWindow->client_to_screen(x, y);
}

void __fastcall tut_win_des_rect_redirect(
        TutWin *self, void *, RECT *rect, int *x, int *y) {
    self->des_rect(rect, x, y);
}


/*
 * The four do_* helpers: 123-byte clones whose ONLY real difference is which
 * fixed window they centre through. The other differing bytes are the two
 * call displacements, which move because the calls are relative and the four
 * bodies sit 0x80 apart - not because either target differs. Both targets are
 * the same for all four.
 */

/*
Purpose: Centre the rectangle, convert it through the base window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA870 ?do_base@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA870-0x004BA8EB SEMANTIC
// LEVER: two levers stacked, 4/56 -> 45/56 MNEMONIC_ONLY. (1) plain `left + (right - left) / 2` signed division instead of the unsigned-with-sign-carried trick, same as `iface_rect`. (2) `primary` read BEFORE `window` is given any value, and its null-check written `if (primary == nullptr) { window = nullptr; } else { ...compute... }` (primary-first declaration order, and the null case as the FIRST arm) - the image reads the global first and only zeroes the result register on the branch that skips the vtable dereference, both of which a `window = nullptr;`-then-test body reorders.
// TRIED: 45/56 plateau - the remainder is a consistent eax/ecx role swap through the vbtable-dereference block and the final argument push order, tried: `uint8_t *` instead of `void *` for `window`, and folding the `vbtable` local into the expression directly - neither changes it. VC6 register allocation, not a source-form lever found here.
// symbol    ?do_base@TutWin@@QAEXPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_base@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: n/a - the show's result is discarded
Status: Complete with a temporary TutWin::tut_win dependency
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
void TutWin::do_base(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinBaseWindow->client_to_screen(&x, &y);

    uint8_t *const primary = reinterpret_cast<uint8_t *>(MapWinTable[0]);
    void *window;
    if (primary == nullptr) {
        window = nullptr;
    } else {
        // The virtual-base displacement lives at offset 4 of the object's
        // vbtable, which is the dword at offset 0.
        const int32_t *const vbtable =
            *reinterpret_cast<int32_t *const *>(primary);
        window = primary + vbtable[1];
    }
    this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}

void __fastcall tut_win_do_base_redirect(
        TutWin *self, void *, RECT *rect, const char *text, int flag) {
    self->do_base(rect, text, flag);
}

/*
Purpose: Centre the rectangle, convert it through the iface window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA8F0 ?do_iface@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA8F0-0x004BA96B SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_iface@TutWin@@QAEXPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_iface@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: n/a - the show's result is discarded
Status: Complete with a temporary TutWin::tut_win dependency
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
void TutWin::do_iface(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinIfaceWindow->client_to_screen(&x, &y);

    uint8_t *const primary = reinterpret_cast<uint8_t *>(MapWinTable[0]);
    void *window;
    if (primary == nullptr) {
        window = nullptr;
    } else {
        // The virtual-base displacement lives at offset 4 of the object's
        // vbtable, which is the dword at offset 0.
        const int32_t *const vbtable =
            *reinterpret_cast<int32_t *const *>(primary);
        window = primary + vbtable[1];
    }
    this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}

void __fastcall tut_win_do_iface_redirect(
        TutWin *self, void *, RECT *rect, const char *text, int flag) {
    self->do_iface(rect, text, flag);
}

/*
Purpose: Centre the rectangle, convert it through the soc window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA970 ?do_soc@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA970-0x004BA9EB SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_soc@TutWin@@QAEXPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_soc@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: n/a - the show's result is discarded
Status: Complete with a temporary TutWin::tut_win dependency
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
void TutWin::do_soc(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinSocWindow->client_to_screen(&x, &y);

    uint8_t *const primary = reinterpret_cast<uint8_t *>(MapWinTable[0]);
    void *window;
    if (primary == nullptr) {
        window = nullptr;
    } else {
        // The virtual-base displacement lives at offset 4 of the object's
        // vbtable, which is the dword at offset 0.
        const int32_t *const vbtable =
            *reinterpret_cast<int32_t *const *>(primary);
        window = primary + vbtable[1];
    }
    this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}

void __fastcall tut_win_do_soc_redirect(
        TutWin *self, void *, RECT *rect, const char *text, int flag) {
    self->do_soc(rect, text, flag);
}

/*
Purpose: Centre the rectangle, convert it through the des window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA9F0 ?do_des@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA9F0-0x004BAA6B SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_des@TutWin@@QAEXPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_des@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: n/a - the show's result is discarded
Status: Complete with a temporary TutWin::tut_win dependency
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
void TutWin::do_des(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinDesWindow->client_to_screen(&x, &y);

    uint8_t *const primary = reinterpret_cast<uint8_t *>(MapWinTable[0]);
    void *window;
    if (primary == nullptr) {
        window = nullptr;
    } else {
        // The virtual-base displacement lives at offset 4 of the object's
        // vbtable, which is the dword at offset 0.
        const int32_t *const vbtable =
            *reinterpret_cast<int32_t *const *>(primary);
        window = primary + vbtable[1];
    }
    this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}

void __fastcall tut_win_do_des_redirect(
        TutWin *self, void *, RECT *rect, const char *text, int flag) {
    self->do_des(rect, text, flag);
}

/*
Purpose: Tell the attached object the tutorial window moved.
// ORIGINAL: 0x004BDDB0 ?on_move@TutWin@@QAEXHH@Z 0x004BDDB0-0x004BDDC5 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?on_move@TutWin@@QAEXHH@Z)(TutWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004BDDBC
Return Value: n/a
Status: Complete
*/
void TutWin::on_move(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    VCall *obj = *reinterpret_cast<VCall **>(self + 0x53d4);
    if (obj) {
        obj->slot063();
    }
}


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x004BA6B0 ??0TutWin@@QAE@XZ 0x004BA6B0-0x004BA71E BYTE_EXACT
// body      src/tutwin.h
// size      110 bytes
// prototype void (__thiscall ??0TutWin@@QAE@XZ)(TutWin* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004048A0
*/

// LEVER: `: public Popup` real inheritance, so the single call is the
// implicit base constructor (now BYTE_EXACT, see popup.cpp) - no explicit
// `Popup::Popup()` call needed, and (matching Popup's own fix) the trailing
// stores use a non-volatile pointer.
static const uint32_t TutWinPrimaryVtable = 0x0066DD84;
static const uint32_t TutWinBufferVtable = 0x0066DD7C;

TutWin::TutWin() {
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = TutWinPrimaryVtable;
    object[0x444 / 4] = TutWinBufferVtable;
    field_53B4_ = 0;
    field_53C0_ = 0;
    field_53D0_ = 0;
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;
}

/*
Purpose: Step the receiver back to the subobject ??_GTutWin@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004BEA20 ??_GTutWin@@WEEE@AEPAXI@Z 0x004BEA20-0x004BEA2B
// symbol    ??_ETutWin@@WEEE@AEPAXI@Z
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
