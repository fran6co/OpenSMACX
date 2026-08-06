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

// The tutorial's "already shown" marker, cleared alongside the window's own
// state. Bound at its canonical address because nothing here owns it yet.
uint32_t *TutWinShownFlag = (uint32_t *)0x008CC244;

/*
Purpose: Reset the tutorial window - four fields to zero, four to -1, and the
         shared shown-marker cleared with them.
Original Offset: 004BA720
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
    *TutWinShownFlag = 0;
}

/*
Purpose: Store a single value at 0x53D4.
Original Offset: 004BDDD0
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

Win *TutWinIfaceWindow = reinterpret_cast<Win *>(0x007AE820);

/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the iface window.
Original Offset: 004BC5A0
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x007AE820,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::iface_rect(RECT *rect, int *x, int *y) {
    const uint32_t left = static_cast<uint32_t>(rect->left);
    const uint32_t width = static_cast<uint32_t>(rect->right) - left;
    const uint32_t width_adjusted = width + (width >> 31);
    *x = static_cast<int>(
        left + ((width_adjusted >> 1) | (width_adjusted & 0x80000000U)));
    const uint32_t top = static_cast<uint32_t>(rect->top);
    const uint32_t height = static_cast<uint32_t>(rect->bottom) - top;
    const uint32_t height_adjusted = height + (height >> 31);
    *y = static_cast<int>(
        top + ((height_adjusted >> 1) | (height_adjusted & 0x80000000U)));
    TutWinIfaceWindow->client_to_screen(x, y);
}

void __fastcall tut_win_iface_rect_redirect(
        TutWin *self, void *, RECT *rect, int *x, int *y) {
    self->iface_rect(rect, x, y);
}

Win *TutWinBaseWindow = reinterpret_cast<Win *>(0x006A7628);

/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the base window.
Original Offset: 004BC5F0
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x006A7628,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::base_rect(RECT *rect, int *x, int *y) {
    const uint32_t left = static_cast<uint32_t>(rect->left);
    const uint32_t width = static_cast<uint32_t>(rect->right) - left;
    const uint32_t width_adjusted = width + (width >> 31);
    *x = static_cast<int>(
        left + ((width_adjusted >> 1) | (width_adjusted & 0x80000000U)));
    const uint32_t top = static_cast<uint32_t>(rect->top);
    const uint32_t height = static_cast<uint32_t>(rect->bottom) - top;
    const uint32_t height_adjusted = height + (height >> 31);
    *y = static_cast<int>(
        top + ((height_adjusted >> 1) | (height_adjusted & 0x80000000U)));
    TutWinBaseWindow->client_to_screen(x, y);
}

void __fastcall tut_win_base_rect_redirect(
        TutWin *self, void *, RECT *rect, int *x, int *y) {
    self->base_rect(rect, x, y);
}

Win *TutWinSocWindow = reinterpret_cast<Win *>(0x008A6270);

/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the soc window.
Original Offset: 004BC640
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x008A6270,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::soc_rect(RECT *rect, int *x, int *y) {
    const uint32_t left = static_cast<uint32_t>(rect->left);
    const uint32_t width = static_cast<uint32_t>(rect->right) - left;
    const uint32_t width_adjusted = width + (width >> 31);
    *x = static_cast<int>(
        left + ((width_adjusted >> 1) | (width_adjusted & 0x80000000U)));
    const uint32_t top = static_cast<uint32_t>(rect->top);
    const uint32_t height = static_cast<uint32_t>(rect->bottom) - top;
    const uint32_t height_adjusted = height + (height >> 31);
    *y = static_cast<int>(
        top + ((height_adjusted >> 1) | (height_adjusted & 0x80000000U)));
    TutWinSocWindow->client_to_screen(x, y);
}

void __fastcall tut_win_soc_rect_redirect(
        TutWin *self, void *, RECT *rect, int *x, int *y) {
    self->soc_rect(rect, x, y);
}

Win *TutWinDesWindow = reinterpret_cast<Win *>(0x0071F2B0);

/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the des window.
Original Offset: 004BC690
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x0071F2B0,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::des_rect(RECT *rect, int *x, int *y) {
    const uint32_t left = static_cast<uint32_t>(rect->left);
    const uint32_t width = static_cast<uint32_t>(rect->right) - left;
    const uint32_t width_adjusted = width + (width >> 31);
    *x = static_cast<int>(
        left + ((width_adjusted >> 1) | (width_adjusted & 0x80000000U)));
    const uint32_t top = static_cast<uint32_t>(rect->top);
    const uint32_t height = static_cast<uint32_t>(rect->bottom) - top;
    const uint32_t height_adjusted = height + (height >> 31);
    *y = static_cast<int>(
        top + ((height_adjusted >> 1) | (height_adjusted & 0x80000000U)));
    TutWinDesWindow->client_to_screen(x, y);
}

void __fastcall tut_win_des_rect_redirect(
        TutWin *self, void *, RECT *rect, int *x, int *y) {
    self->des_rect(rect, x, y);
}

func_tut_win_show TutWinOriginalShow = original_method<func_tut_win_show>(0x004BDFE0);

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
Original Offset: 004BA870
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
    const uint32_t left = static_cast<uint32_t>(rect->left);
    const uint32_t width = static_cast<uint32_t>(rect->right) - left;
    const uint32_t width_adjusted = width + (width >> 31);
    int x = static_cast<int>(
        left + ((width_adjusted >> 1) | (width_adjusted & 0x80000000U)));
    const uint32_t top = static_cast<uint32_t>(rect->top);
    const uint32_t height = static_cast<uint32_t>(rect->bottom) - top;
    const uint32_t height_adjusted = height + (height >> 31);
    int y = static_cast<int>(
        top + ((height_adjusted >> 1) | (height_adjusted & 0x80000000U)));
    TutWinBaseWindow->client_to_screen(&x, &y);

    void *window = nullptr;
    auto *const primary = reinterpret_cast<uint8_t *>(MapWinTable[0]);
    if (primary != nullptr) {
        // The virtual-base displacement lives at offset 4 of the object's
        // vbtable, which is the dword at offset 0.
        const int32_t *const vbtable =
            *reinterpret_cast<int32_t *const *>(primary);
        window = primary + vbtable[1];
    }
    (ORIGINAL(this)->*TutWinOriginalShow)(window, text, x, y, nullptr, flag, -1, -1);
}

void __fastcall tut_win_do_base_redirect(
        TutWin *self, void *, RECT *rect, const char *text, int flag) {
    self->do_base(rect, text, flag);
}

/*
Purpose: Centre the rectangle, convert it through the iface window, and
         show the tutorial text there against the primary map window.
Original Offset: 004BA8F0
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
    const uint32_t left = static_cast<uint32_t>(rect->left);
    const uint32_t width = static_cast<uint32_t>(rect->right) - left;
    const uint32_t width_adjusted = width + (width >> 31);
    int x = static_cast<int>(
        left + ((width_adjusted >> 1) | (width_adjusted & 0x80000000U)));
    const uint32_t top = static_cast<uint32_t>(rect->top);
    const uint32_t height = static_cast<uint32_t>(rect->bottom) - top;
    const uint32_t height_adjusted = height + (height >> 31);
    int y = static_cast<int>(
        top + ((height_adjusted >> 1) | (height_adjusted & 0x80000000U)));
    TutWinIfaceWindow->client_to_screen(&x, &y);

    void *window = nullptr;
    auto *const primary = reinterpret_cast<uint8_t *>(MapWinTable[0]);
    if (primary != nullptr) {
        // The virtual-base displacement lives at offset 4 of the object's
        // vbtable, which is the dword at offset 0.
        const int32_t *const vbtable =
            *reinterpret_cast<int32_t *const *>(primary);
        window = primary + vbtable[1];
    }
    (ORIGINAL(this)->*TutWinOriginalShow)(window, text, x, y, nullptr, flag, -1, -1);
}

void __fastcall tut_win_do_iface_redirect(
        TutWin *self, void *, RECT *rect, const char *text, int flag) {
    self->do_iface(rect, text, flag);
}

/*
Purpose: Centre the rectangle, convert it through the soc window, and
         show the tutorial text there against the primary map window.
Original Offset: 004BA970
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
    const uint32_t left = static_cast<uint32_t>(rect->left);
    const uint32_t width = static_cast<uint32_t>(rect->right) - left;
    const uint32_t width_adjusted = width + (width >> 31);
    int x = static_cast<int>(
        left + ((width_adjusted >> 1) | (width_adjusted & 0x80000000U)));
    const uint32_t top = static_cast<uint32_t>(rect->top);
    const uint32_t height = static_cast<uint32_t>(rect->bottom) - top;
    const uint32_t height_adjusted = height + (height >> 31);
    int y = static_cast<int>(
        top + ((height_adjusted >> 1) | (height_adjusted & 0x80000000U)));
    TutWinSocWindow->client_to_screen(&x, &y);

    void *window = nullptr;
    auto *const primary = reinterpret_cast<uint8_t *>(MapWinTable[0]);
    if (primary != nullptr) {
        // The virtual-base displacement lives at offset 4 of the object's
        // vbtable, which is the dword at offset 0.
        const int32_t *const vbtable =
            *reinterpret_cast<int32_t *const *>(primary);
        window = primary + vbtable[1];
    }
    (ORIGINAL(this)->*TutWinOriginalShow)(window, text, x, y, nullptr, flag, -1, -1);
}

void __fastcall tut_win_do_soc_redirect(
        TutWin *self, void *, RECT *rect, const char *text, int flag) {
    self->do_soc(rect, text, flag);
}

/*
Purpose: Centre the rectangle, convert it through the des window, and
         show the tutorial text there against the primary map window.
Original Offset: 004BA9F0
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
    const uint32_t left = static_cast<uint32_t>(rect->left);
    const uint32_t width = static_cast<uint32_t>(rect->right) - left;
    const uint32_t width_adjusted = width + (width >> 31);
    int x = static_cast<int>(
        left + ((width_adjusted >> 1) | (width_adjusted & 0x80000000U)));
    const uint32_t top = static_cast<uint32_t>(rect->top);
    const uint32_t height = static_cast<uint32_t>(rect->bottom) - top;
    const uint32_t height_adjusted = height + (height >> 31);
    int y = static_cast<int>(
        top + ((height_adjusted >> 1) | (height_adjusted & 0x80000000U)));
    TutWinDesWindow->client_to_screen(&x, &y);

    void *window = nullptr;
    auto *const primary = reinterpret_cast<uint8_t *>(MapWinTable[0]);
    if (primary != nullptr) {
        // The virtual-base displacement lives at offset 4 of the object's
        // vbtable, which is the dword at offset 0.
        const int32_t *const vbtable =
            *reinterpret_cast<int32_t *const *>(primary);
        window = primary + vbtable[1];
    }
    (ORIGINAL(this)->*TutWinOriginalShow)(window, text, x, y, nullptr, flag, -1, -1);
}

void __fastcall tut_win_do_des_redirect(
        TutWin *self, void *, RECT *rect, const char *text, int flag) {
    self->do_des(rect, text, flag);
}
