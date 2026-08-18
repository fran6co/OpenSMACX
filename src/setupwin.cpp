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
#include "setupwin.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x004ADAD0 ?UNK3@SetupWin@@QAEHH@Z 0x004ADAD0-0x004ADAD5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK3@SetupWin@@QAEHH@Z)(SetupWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int SetupWin::UNK3(int) {
    return 0;
}

int __fastcall setup_win_unk3_redirect(SetupWin *self, void *, int a1) {
    return self->UNK3(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x004ADAE0 ?UNK4@SetupWin@@QAEHH@Z 0x004ADAE0-0x004ADAE5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK4@SetupWin@@QAEHH@Z)(SetupWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int SetupWin::UNK4(int) {
    return 0;
}

int __fastcall setup_win_unk4_redirect(SetupWin *self, void *, int a1) {
    return self->UNK4(a1);
}

/*
Purpose: Scale the argument by the field at 0xA14 and divide by 1024.

             mov eax,[ecx+0xA14] / imul eax,[ebp+8] / cdq
             and edx,0x3FF / add eax,edx / sar eax,0xA

         `cdq / and / add / sar` is MSVC's signed divide by a power of two: it
         biases a negative dividend by 2^n - 1 before the arithmetic shift so
         the quotient truncates toward zero, which is what C++ `/ 1024` means.
         Verified over 200,000 random 32-bit dividends plus the boundaries -
         the idiom and `/ 1024` agree on every one.

         The multiply is a 32-bit `imul`, so an overflowing product wraps and
         the division sees the wrapped value; `int` arithmetic reproduces that.

         SetupWin models no fields - it derives from GraphicWin, which ends at
         0xA14 - so this is its first own field, reached through a documented
         raw offset as BasePop::UNK3 does.
// ORIGINAL: 0x004ADA80 ?UNK1@SetupWin@@QAEHH@Z 0x004ADA80-0x004ADA9D BYTE_EXACT
// size      29 bytes
// prototype int (__thiscall ?UNK1@SetupWin@@QAEHH@Z)(SetupWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: (field_A14 * a1) / 1024, truncated toward zero
Status: Complete
*/
int SetupWin::UNK1(int a1) {
    const int32_t scale = *reinterpret_cast<const int32_t *>(
        reinterpret_cast<const uint8_t *>(this) + 0xA14);
    return static_cast<int32_t>(static_cast<uint32_t>(scale)
                                * static_cast<uint32_t>(a1)) / 1024;
}

int __fastcall setup_win_unk1_redirect(SetupWin *self, void *, int a1) {
    return self->UNK1(a1);
}

/*
Purpose: As UNK1, over the field at 0xA18 and dividing by 768.

             mov ecx,[ecx+0xA18] / mov eax,0x2AAAAAAB / imul ecx,[ebp+8]
             imul ecx / mov eax,edx / sar eax,7 / mov ecx,eax
             shr ecx,0x1F / add eax,ecx

         The magic-number form: 0x2AAAAAAB taken as the high half of a signed
         multiply, shifted right by 7 and corrected by the sign bit, is
         division by 768. Verified the same way as UNK1 - 200,000 random
         dividends and the boundaries, agreeing with `/ 768` on every one.

         1024 and 768 beside each other read as a resolution, which is a
         plausible account of what these two scale and is NOT what the recovery
         rests on; the arithmetic is.
// ORIGINAL: 0x004ADAA0 ?UNK2@SetupWin@@QAEHH@Z 0x004ADAA0-0x004ADAC4 BYTE_EXACT
// size      36 bytes
// prototype int (__thiscall ?UNK2@SetupWin@@QAEHH@Z)(SetupWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: (field_A18 * a1) / 768, truncated toward zero
Status: Complete
*/
int SetupWin::UNK2(int a1) {
    const int32_t scale = *reinterpret_cast<const int32_t *>(
        reinterpret_cast<const uint8_t *>(this) + 0xA18);
    return static_cast<int32_t>(static_cast<uint32_t>(scale)
                                * static_cast<uint32_t>(a1)) / 768;
}

int __fastcall setup_win_unk2_redirect(SetupWin *self, void *, int a1) {
    return self->UNK2(a1);
}

/*
// ORIGINAL: 0x004ADAF0 ?do_menu@SetupWin@@QAEHPADHH@Z 0x004ADAF0-0x004ADB11 BYTE_EXACT
// size      33 bytes
// prototype int (__thiscall ?do_menu@SetupWin@@QAEHPADHH@Z)(SetupWin* this, int8*, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004ADB70
Status: Complete
*/
int SetupWin::do_menu(char* a1, int a2, int a3) {
    switch (a2) {
        case 1:
            return do_menu_rightside(a1, a3);
        default:
            return -1;
    }
}

func_setup_win_do_menu_rightside SetupWinDoMenuRightside =
    original_method<func_setup_win_do_menu_rightside>(0x004ADB70);

/*
Purpose: Run the right-hand side of the setup menu. The body at 0x004ADB70 is
         NOT recovered; this is a seam to the original image, not a recovery,
         and deliberately carries no `Original Offset:` line so the catalogue
         does not mistake it for one.
Status: Forwarded to the original image
*/
int SetupWin::do_menu_rightside(char* a1, int a2) {
    return (ORIGINAL(this)->*SetupWinDoMenuRightside)(a1, a2);
}
