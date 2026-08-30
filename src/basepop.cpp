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
#include "basepop.h"
#include "main.h"  // PopupAllocHook - the tree's own 0x00696ECC slot
#include "dialogs.h"
#include "general.h"  // mem_get in init_class
#include "datalink.h"
#include "worldwin.h"
#include "dipedit.h"
#include "filewin.h"
#include "stringstruct.h"  // StringAllocationHeap - the 0x009B3374 hand-off
#include <cstring>

/*
Purpose: Update either base-popup coordinate unless its keep-current sentinel is supplied.
// ORIGINAL: 0x00601B80 ?set_loc@BasePop@@QAEXHH@Z 0x00601B80-0x00601BA5 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?set_loc@BasePop@@QAEXHH@Z)(BasePop* this, int, int)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void BasePop::set_loc(int x, int y) {
    if (x != 0x2000) {
        loc_a_ = x;
    }
    if (y != 0x2000) {
        loc_b_ = y;
    }
}


Font *BasePopDefaultStringFonts[4];  // 0x009B8D98
Font *BasePopDefaultButtonFonts[3];  // 0x009B8DA8

namespace {

// Shared by every default-font setter in the codebase: the primary is
// published only when it is initialized, the remaining slots are stored
// unconditionally, and only a null primary is an error.

}  // namespace

/*
Purpose: Set the default string fonts shared by every popup.
// ORIGINAL: 0x006048C0 ?set_def_string_font@BasePop@@QAAHPAUFont@@PAUFont@@PAUFont@@PAUFont@@@Z 0x006048C0-0x006048FA BYTE_EXACT
// symbol    ?set_def_string_font@BasePop@@SAHPAVFont@@000@Z
// size      58 bytes
// prototype 
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int BasePop::set_def_string_font(Font *font1, Font *font2, Font *font3,
                                 Font *font4) {
    // WRITTEN OUT, not published through a helper: the image tests the
    // first font, stores it ONLY when it is initialised, and then stores the
    // other three unconditionally. A loop over an array of four cannot make
    // that asymmetry, and the helper is a call the image does not have.
    if (font1 == nullptr) {
        return 3;
    }
    if (font1->is_initialized()) {
        BasePopDefaultStringFonts[0] = font1;
    }
    BasePopDefaultStringFonts[1] = font2;
    BasePopDefaultStringFonts[2] = font3;
    BasePopDefaultStringFonts[3] = font4;
    return 0;
}

/*
Purpose: Set the default button fonts shared by every popup.
// ORIGINAL: 0x006049C0 ?set_def_button_font@BasePop@@QAAHPAUFont@@PAUFont@@PAUFont@@@Z 0x006049C0-0x006049F0 BYTE_EXACT
// symbol    ?set_def_button_font@BasePop@@SAHPAVFont@@00@Z
// size      48 bytes
// prototype 
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int BasePop::set_def_button_font(Font *font1, Font *font2, Font *font3) {
    if (font1 == nullptr) {
        return 3;
    }
    if (font1->is_initialized()) {
        BasePopDefaultButtonFonts[0] = font1;
    }
    BasePopDefaultButtonFonts[1] = font2;
    BasePopDefaultButtonFonts[2] = font3;
    return 0;
}



// Slot s of tier t lives at base + s * stride + t * 4. The string table has
// four tiers so its slots are 0x10 apart; the button table has three, so its
// slots are 0xC apart. Getting that stride wrong writes into a sibling slot
// rather than a sibling tier, which is why the tests compare whole tables.
// ARRAYS, NOT POINTERS, and the shape is [slot][tier]: the string table has
// four tiers so its four slots are 0x10 apart, the button table has three so
// its slots are 0xC apart. `BasePopDefaultStringColors[1][0]` is 0x00696EF4,
// which is where the image's second store goes.
// The initializers are the image's own file bytes at 0x00696EE4 and
// 0x00696F24 (this table is below .data's 0x006A8000 file-backed end, unlike
// the 0x009B8xxx slots): transparent for slot 0, white for slots 0-1 tier 1+,
// and palette index 1 for slots 2-3.
uint32_t BasePopDefaultStringColors[4][4] = {
    {0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
    {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
    {0x00000001, 0x00000001, 0x00000001, 0x00000001},
    {0x00000001, 0x00000001, 0x00000001, 0x00000001}};  // 0x00696EE4
uint32_t BasePopDefaultButtonColors[4][3] = {
    {0x00000000, 0xFFFFFFFF, 0xFFFFFFFF},
    {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
    {0x00000001, 0x00000001, 0x00000001},
    {0x00000001, 0x00000001, 0x00000001}};  // 0x00696F24

namespace {

}  // namespace

/*
Purpose: Set default string colour tier 0 shared by every popup.
// ORIGINAL: 0x00604900 ?set_def_string_color@BasePop@@QAAXHHHH@Z 0x00604900-0x00604927 BYTE_EXACT
// symbol    ?set_def_string_color@BasePop@@SAXHHHH@Z
// size      39 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_string_color(int c1, int c2, int c3, int c4) {
    BasePopDefaultStringColors[0][0] = static_cast<uint32_t>(c1);
    BasePopDefaultStringColors[1][0] = static_cast<uint32_t>(c2);
    BasePopDefaultStringColors[2][0] = static_cast<uint32_t>(c3);
    BasePopDefaultStringColors[3][0] = static_cast<uint32_t>(c4);
}


/*
Purpose: Set default string colour tier 1 shared by every popup.
// ORIGINAL: 0x00604930 ?set_def_string_color2@BasePop@@QAAXHHHH@Z 0x00604930-0x00604957 BYTE_EXACT
// symbol    ?set_def_string_color2@BasePop@@SAXHHHH@Z
// size      39 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_string_color2(int c1, int c2, int c3, int c4) {
    BasePopDefaultStringColors[0][1] = static_cast<uint32_t>(c1);
    BasePopDefaultStringColors[1][1] = static_cast<uint32_t>(c2);
    BasePopDefaultStringColors[2][1] = static_cast<uint32_t>(c3);
    BasePopDefaultStringColors[3][1] = static_cast<uint32_t>(c4);
}


/*
Purpose: Set default string colour tier 2 shared by every popup.
// ORIGINAL: 0x00604960 ?set_def_string_color3@BasePop@@QAAXHHHH@Z 0x00604960-0x00604987 BYTE_EXACT
// symbol    ?set_def_string_color3@BasePop@@SAXHHHH@Z
// size      39 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_string_color3(int c1, int c2, int c3, int c4) {
    BasePopDefaultStringColors[0][2] = static_cast<uint32_t>(c1);
    BasePopDefaultStringColors[1][2] = static_cast<uint32_t>(c2);
    BasePopDefaultStringColors[2][2] = static_cast<uint32_t>(c3);
    BasePopDefaultStringColors[3][2] = static_cast<uint32_t>(c4);
}


/*
Purpose: Set default string colour tier 3 shared by every popup.
// ORIGINAL: 0x00604990 ?set_def_string_color_hyper@BasePop@@QAAXHHHH@Z 0x00604990-0x006049B7 BYTE_EXACT
// symbol    ?set_def_string_color_hyper@BasePop@@SAXHHHH@Z
// size      39 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_string_color_hyper(int c1, int c2, int c3, int c4) {
    BasePopDefaultStringColors[0][3] = static_cast<uint32_t>(c1);
    BasePopDefaultStringColors[1][3] = static_cast<uint32_t>(c2);
    BasePopDefaultStringColors[2][3] = static_cast<uint32_t>(c3);
    BasePopDefaultStringColors[3][3] = static_cast<uint32_t>(c4);
}


/*
Purpose: Set default button colour tier 0 shared by every popup.
// ORIGINAL: 0x006049F0 ?set_def_button_color@BasePop@@QAAXHHHH@Z 0x006049F0-0x00604A17 BYTE_EXACT
// symbol    ?set_def_button_color@BasePop@@SAXHHHH@Z
// size      39 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_button_color(int c1, int c2, int c3, int c4) {
    BasePopDefaultButtonColors[0][0] = static_cast<uint32_t>(c1);
    BasePopDefaultButtonColors[1][0] = static_cast<uint32_t>(c2);
    BasePopDefaultButtonColors[2][0] = static_cast<uint32_t>(c3);
    BasePopDefaultButtonColors[3][0] = static_cast<uint32_t>(c4);
}


/*
Purpose: Set default button colour tier 1 shared by every popup.
// ORIGINAL: 0x00604A20 ?set_def_button_color2@BasePop@@QAAXHHHH@Z 0x00604A20-0x00604A47 BYTE_EXACT
// symbol    ?set_def_button_color2@BasePop@@SAXHHHH@Z
// size      39 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_button_color2(int c1, int c2, int c3, int c4) {
    BasePopDefaultButtonColors[0][1] = static_cast<uint32_t>(c1);
    BasePopDefaultButtonColors[1][1] = static_cast<uint32_t>(c2);
    BasePopDefaultButtonColors[2][1] = static_cast<uint32_t>(c3);
    BasePopDefaultButtonColors[3][1] = static_cast<uint32_t>(c4);
}


/*
Purpose: Set default button colour tier 2 shared by every popup.
// ORIGINAL: 0x00604A50 ?set_def_button_color3@BasePop@@QAAXHHHH@Z 0x00604A50-0x00604A77 BYTE_EXACT
// symbol    ?set_def_button_color3@BasePop@@SAXHHHH@Z
// size      39 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_button_color3(int c1, int c2, int c3, int c4) {
    BasePopDefaultButtonColors[0][2] = static_cast<uint32_t>(c1);
    BasePopDefaultButtonColors[1][2] = static_cast<uint32_t>(c2);
    BasePopDefaultButtonColors[2][2] = static_cast<uint32_t>(c3);
    BasePopDefaultButtonColors[3][2] = static_cast<uint32_t>(c4);
}


/*
Purpose: Set this popup's string color slots.
// ORIGINAL: 0x00604730 ?set_string_color@BasePop@@QAEXHHHH@Z 0x00604730-0x0060475B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_string_color@BasePop@@QAEXHHHH@Z)(BasePop* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_string_color(int c1, int c2, int c3, int c4) {
    string_color_a_ = c1;
    string_color_b_ = c2;
    string_color_c_ = c3;
    string_color_d_ = c4;
}


/*
Purpose: Set this popup's string color2 slots.
// ORIGINAL: 0x00604760 ?set_string_color2@BasePop@@QAEXHHHH@Z 0x00604760-0x0060478B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_string_color2@BasePop@@QAEXHHHH@Z)(BasePop* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_string_color2(int c1, int c2, int c3, int c4) {
    string_color_2a_ = c1;
    string_color_2b_ = c2;
    string_color_2c_ = c3;
    string_color_2d_ = c4;
}


/*
Purpose: Set this popup's string color3 slots.
// ORIGINAL: 0x00604790 ?set_string_color3@BasePop@@QAEXHHHH@Z 0x00604790-0x006047BB BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_string_color3@BasePop@@QAEXHHHH@Z)(BasePop* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_string_color3(int c1, int c2, int c3, int c4) {
    string_color_3a_ = c1;
    string_color_3b_ = c2;
    string_color_3c_ = c3;
    string_color_3d_ = c4;
}


/*
Purpose: Set this popup's string color hyper slots.
// ORIGINAL: 0x006047C0 ?set_string_color_hyper@BasePop@@QAEXHHHH@Z 0x006047C0-0x006047EB BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_string_color_hyper@BasePop@@QAEXHHHH@Z)(BasePop* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_string_color_hyper(int c1, int c2, int c3, int c4) {
    string_color_hyper_a_ = c1;
    string_color_hyper_b_ = c2;
    string_color_hyperc_ = c3;
    string_color_hyper_d_ = c4;
}


/*
Purpose: Set this popup's button color slots.
// ORIGINAL: 0x00604830 ?set_button_color@BasePop@@QAEXDHHH@Z 0x00604830-0x0060485B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_button_color@BasePop@@QAEXDHHH@Z)(BasePop* this, int8, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
Verification note: the first slot is a byte while the other three are dwords,
which the decompiler renders as four indistinguishable stores. Writing a dword
there would overwrite the neighbouring tiers' bytes at 0x3179 and 0x317A.
*/
void BasePop::set_button_color(char c1, int c2, int c3, int c4) {
    button_color_a_ = static_cast<uint8_t>(c1);
    button_color_b_ = static_cast<uint32_t>(c2);
    button_color_c_ = static_cast<uint32_t>(c3);
    button_color_d_ = static_cast<uint32_t>(c4);
}


/*
Purpose: Set this popup's button color2 slots.
// ORIGINAL: 0x00604860 ?set_button_color2@BasePop@@QAEXDHHH@Z 0x00604860-0x0060488B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_button_color2@BasePop@@QAEXDHHH@Z)(BasePop* this, int8, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
Verification note: the first slot is a byte while the other three are dwords,
which the decompiler renders as four indistinguishable stores. Writing a dword
there would overwrite the neighbouring tiers' bytes at 0x3179 and 0x317A.
*/
void BasePop::set_button_color2(char c1, int c2, int c3, int c4) {
    button_color_2a_ = static_cast<uint8_t>(c1);
    button_color_2b_ = static_cast<uint32_t>(c2);
    button_color_2c_ = static_cast<uint32_t>(c3);
    button_color_2d_ = static_cast<uint32_t>(c4);
}


/*
Purpose: Set this popup's button color3 slots.
// ORIGINAL: 0x00604890 ?set_button_color3@BasePop@@QAEXDHHH@Z 0x00604890-0x006048BB BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_button_color3@BasePop@@QAEXDHHH@Z)(BasePop* this, int8, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
Verification note: the first slot is a byte while the other three are dwords,
which the decompiler renders as four indistinguishable stores. Writing a dword
there would overwrite the neighbouring tiers' bytes at 0x3179 and 0x317A.
*/
void BasePop::set_button_color3(char c1, int c2, int c3, int c4) {
    button_color_3a_ = static_cast<uint8_t>(c1);
    button_color_3b_ = static_cast<uint32_t>(c2);
    button_color_3c_ = static_cast<uint32_t>(c3);
    button_color_3d_ = static_cast<uint32_t>(c4);
}



/*
Purpose: Raise the fallout flag, but only while the gate is set.
// ORIGINAL: 0x00604A90 ?fallout@BasePop@@QAAXXZ 0x00604A90-0x00604AA4 BYTE_EXACT
// symbol    ?fallout@BasePop@@SAXXZ
// size      20 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::fallout() {
    if (BasePopFalloutGate != 0) {
        BasePopFalloutFlag = 1;
    }
}


/*
Purpose: Set the popup's embedded check box, at 0x2228, to the given state.
// ORIGINAL: 0x00601BB0 ?write_check@BasePop@@QAEXJ@Z 0x00601BB0-0x00601BC3 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?write_check@BasePop@@QAEXJ@Z)(BasePop* this, int)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0060ECE0
Return Value: n/a
Status: Complete
*/
void BasePop::write_check(long value) {
    check_box()->set_state_flag(value);
}



/*
Purpose: Set the dialog width, scaled to three-halves in the high-resolution
         layout. The scaling is skipped when the popup opts out through the
         flag at 0xA14, when bit 0x400 of the style at 0x30A8 is set, or when
         the screen is narrower than 1024. The width is written into the Dialog
         subobject of the embedded Dialogs, located through that Dialogs'
         own vbtable exactly as the original does rather than at a hardcoded
         offset.
// ORIGINAL: 0x00601B20 ?set_width@BasePop@@QAEXH@Z 0x00601B20-0x00601B77 BYTE_EXACT
// LEVER: if-else-not-one-store the image writes the width TWICE, once per arm (0x00601B56 and 0x00601B6D), each with its own `ret 4`. Computing a `value` local and storing it once after the guard shares the store and scores 1 of 23; an `if/else` with a full store in each arm is BYTE_EXACT.
// LEVER: no-memcpy the store is a plain `*(int32_t *)(...) = value`. The `std::memcpy(dst, &value, 4)` spelling takes the local's ADDRESS, which pins it to a stack slot the image never allocates.
// LEVER: reread-vbptr the Dialogs vbptr at this+0x21D0 is loaded separately in each arm, as the image does at 0x00601B4B and 0x00601B60.
// size      87 bytes
// prototype void (__thiscall ?set_width@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BasePop::set_width(int width) {
    uint8_t *const dialogs = reinterpret_cast<uint8_t *>(this) + 0x21D0;
    if (field_A14_ == 0 && (field_30A8_ & 0x400) == 0 &&
        WinScreenWidth >= 0x400) {
        *reinterpret_cast<int32_t *>(
            dialogs + 0x2C +
            (*reinterpret_cast<const int32_t *const *>(dialogs))[2]) =
            (width * 3) / 2;
    } else {
        *reinterpret_cast<int32_t *>(
            dialogs + 0x2C +
            (*reinterpret_cast<const int32_t *const *>(dialogs))[2]) = width;
    }
}



/*
Purpose: Run the popup modally with no completion callback.
// ORIGINAL: 0x005A5900 ?exec@BasePop@@QAEHXZ 0x005A5900-0x005A590A BYTE_EXACT
// size      10 bytes
// prototype int (__thiscall ?exec@BasePop@@QAEHXZ)(BasePop* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00602600
Return Value: the exec result
Status: Complete
*/
int BasePop::exec() {
    return exec(0, nullptr);
}

/*
Purpose: Run the popup modally with a completion callback.
// ORIGINAL: 0x00558FC0 ?exec@BasePop@@QAEHP6AHXZ@Z 0x00558FC0-0x00558FD2 BYTE_EXACT
// size      18 bytes
// prototype int (__thiscall ?exec@BasePop@@QAEHP6AHXZ@Z)(BasePop* this, int (__cdecl *)())
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00602600
Return Value: the exec result
Status: Complete
*/
int BasePop::exec(int (__cdecl *callback)()) {
    return exec(0, callback);
}



/*
Purpose: Report whether the popup will accept a key click. Both this and
         on_key_up below are the same four instructions over one field:

             mov eax, [ecx+0x30A8] / not eax / shr eax, 0xE / and eax, 1

         so the answer is bit 14 of field_30A8_ INVERTED - one when the bit is
         clear. The arguments are ignored; the two differ only in how many they
         clean off the stack.
// ORIGINAL: 0x00604490 ?on_key_click@BasePop@@QAEHHH@Z 0x00604490-0x006044A1 BYTE_EXACT
// size      17 bytes
// prototype int (__thiscall ?on_key_click@BasePop@@QAEHHH@Z)(BasePop* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when bit 14 of field_30A8_ is clear, 0 when it is set
Status: Complete
*/
int BasePop::on_key_click(int, int) {
    return static_cast<int>((~field_30A8_ >> 14) & 1U);
}


/*
Purpose: As on_key_click above, over the same field and the same bit. Kept as
         its own body rather than delegating, because the original is a
         separate function with a different stack cleanup and a delegation
         would change the instruction the caller returns to.
// ORIGINAL: 0x006044B0 ?on_key_up@BasePop@@QAEHH@Z 0x006044B0-0x006044C1 BYTE_EXACT
// size      17 bytes
// prototype int (__thiscall ?on_key_up@BasePop@@QAEHH@Z)(BasePop* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when bit 14 of field_30A8_ is clear, 0 when it is set
Status: Complete
*/
int BasePop::on_key_up(int) {
    return static_cast<int>((~field_30A8_ >> 14) & 1U);
}


/*
Purpose: Set or clear bit 0 of the flags word at offset 0x20 - set when the
         argument is non-zero, cleared when it is zero.

             mov eax, [esp+4] / test eax, eax / mov eax, [ecx+0x20]
             je clear / or al, 1 / mov [ecx+0x20], eax / ret 4
             clear: and al, 0xFE / mov [ecx+0x20], eax / ret 4

         The original reads the whole dword, edits only AL and writes the whole
         dword back, so the upper three bytes are preserved - which is why this
         is a read-modify-write of the dword and not a byte store.

         Written through a documented raw offset, as ~BattleWin does for its
         Time member. 0x20 falls inside Win::auto_sound_ - at AutoSound::val_8_,
         measured - and that modelling is an approximation of a class whose own
         header says its layout is not established. Reaching through it by name
         would state a relationship this function does not need and cannot
         confirm; the offset is the thing the original encodes.
// ORIGINAL: 0x00605180 ?UNK3@BasePop@@QAEXH@Z 0x00605180-0x0060519B BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?UNK3@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BasePop::UNK3(int a1) {
    if (a1) {
        flags_ |= 1U;
    } else {
        flags_ &= ~1U;
    }
}


/*
Purpose: As UNK3 above, over the same flags word but bit 1 (`or al, 2` /
         `and al, 0xFD`).
// ORIGINAL: 0x006051A0 ?UNK4@BasePop@@QAEXH@Z 0x006051A0-0x006051BB BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?UNK4@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BasePop::UNK4(int a1) {
    if (a1) {
        flags_ |= 2U;
    } else {
        flags_ &= ~2U;
    }
}


/*
Purpose: Fetch one item's text from the embedded Dialogs at 0x21D0.

             mov eax,[ebp+0xC] / mov edx,[ebp+8] / push eax / push edx
             add ecx,0x21D0 / call Dialogs::item / ret 8

         A plain delegation: `add ecx, 0x21D0` selects the subobject and the
         two arguments are forwarded unchanged. BasePop models no field there,
         so the offset is documented and raw, as its neighbours here already
         are.

         Dialogs::item takes the Dialog's address from the EMBEDDING object's
         own vbtable rather than from where a Dialogs sits when it is
         most-derived, which is why this can hand it a subobject at all.
// ORIGINAL: 0x00558FE0 ?item@BasePop@@QAEHPADH@Z 0x00558FE0-0x00558FFA BYTE_EXACT
// size      26 bytes
// prototype int (__thiscall ?item@BasePop@@QAEHPADH@Z)(BasePop* this, int8* lpString, int position)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00612A70
Return Value: whatever Dialogs::item returns
Status: Complete
*/
int BasePop::item(char *text, int index) {
    // QUALIFIED: `Dialog::item` is virtual now (it is the virtual ListBox
    // overrides, which is what earns ListBox its Dialog vtordisp), so an
    // unqualified call here dispatches through the vtable where the image
    // calls directly.
    return reinterpret_cast<Dialogs *>(
        reinterpret_cast<uint8_t *>(this) + 0x21D0)->Dialogs::item(text, index);
}


/*
Purpose: Read the state word of the CheckBox embedded at 0x2228.

             mov eax,[ecx+0x2228] / mov edx,[eax+8] / mov eax,[edx+ecx+0x2314]

         The compiler folded the subobject offset into the displacement:
         0x2228 + 0xEC is 0x2314, so `this + vbtable[2] + 0x2314` is the
         subobject at 0x2228 plus `vbtable[2] + 0xEC` - exactly the address
         CheckBox::UNK1, UNK2 and set_state_pos compute, and the vbtable it
         reads is that subobject's own.

         So this returns the whole word those three edit bit by bit, and the
         same rule applies: the Dialog displacement comes from the object's
         vbtable at run time, never from where a CheckBox sits when it is
         most-derived.

         Declared uint32_t although the mangled name says void, as
         SpriteBox::id_to_pos and Dialogs::destroy are: the body ends by
         loading the word into EAX and a void body cannot promise that. The
         return type does not change the thiscall cleanup.
// ORIGINAL: 0x00601BD0 ?read_check@BasePop@@QAEXXZ 0x00601BD0-0x00601BE1 BYTE_EXACT
// symbol    ?read_check@BasePop@@QAEIXZ
// size      17 bytes
// prototype void (__thiscall ?read_check@BasePop@@QAEXXZ)(BasePop* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the embedded CheckBox's state word
Status: Complete
*/
uint32_t BasePop::read_check() {
    uint8_t *const check = reinterpret_cast<uint8_t *>(this) + 0x2228;
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(check);
    return *reinterpret_cast<const uint32_t *>(check + vbtable[2] + 0xEC);
}

/*
Purpose: Allocate a BasePop on the heap and construct it.
// ORIGINAL: 0x00604E40 ?basepop_alloc@BasePop@@SAHXZ 0x00604E40-0x00604E9A;0x00662E17-0x00662E2C BYTE_EXACT
// symbol    ?basepop_alloc@BasePop@@SAPAV1@XZ
// LEVER: `new BasePop()` reproduces the whole /GX operator-new + SEH-frame + conditional-constructor-call sequence verbatim
// LEVER: ctor-not-inline basepop.h declares `BasePop();` rather than defining it empty inline. An inline empty constructor is inlined away and the `call ??0BasePop@@QAE@XZ` at 0x00604E75 never appears; with it declared, this is BYTE_EXACT and without it MISMATCH at #1.
// CORRECTED from ?basepop_alloc@BasePop@@QAEHXZ
//   the body reads ecx nowhere - `push ecx` at 0x00604E55 is a stack
//   reservation, overwritten by `mov [esp], eax` - and all seven call
//   sites through the 0x00696ECC hook set up no receiver
// size      111 bytes
// prototype int (__cdecl ?basepop_alloc@BasePop@@SAHXZ)()
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00600860 0x0064557F 0x0064558A
Return Value: The new object as an int, or zero if the allocation failed
Status: Complete
*/
// The image spells the return H (an int) because the seven call sites
// reach it through the 0x00696ECC hook typed to match - the wire format of
// a hook, not this tree's model. The value carried is the new object.
BasePop *BasePop::basepop_alloc() {
    return new BasePop();
}

char *BasePopDefaultOkText;      // 0x009B8D80
char *BasePopDefaultCancelText;  // 0x009B8D84

// The image reads the popup allocator through the 0x00696ECC slot; the tree
// owns that slot as the real `PopupAllocHook` global (main.cpp), whose
// static initializer is the image's own .data value 0x00604E40. Calling the
// global directly is the image's single memory-indirect `call dword ptr
// [slot]` - spelling the call through a pointer-to-pointer makes VC6 split
// it into mov+call and the claim diverges.
// 0x009BB484 is font.cpp's FontDefault - the process default font the
// pop tables seed from. One storage, one name.

// The fallout pair (basepop.h) and the two singleton slots `init_class`
// below allocates and `pops` (popup.cpp) consumes - real storage, one name
// per address. Zero-initialised dwords in the image, as here.
int BasePopFalloutGate;   // 0x009B8D00
int BasePopFalloutFlag;   // 0x009B8CFC
Win *PopupInstanceSlotA;  // 0x009BC074
Win *PopupInstanceSlotB;  // 0x009BC078


/*
Purpose: Replace the default OK caption with a heap copy of `text`.
// ORIGINAL: 0x006018A0 ?set_def_ok_text@BasePop@@QAAHPAD@Z 0x006018A0-0x00601902 BYTE_EXACT
// symbol    ?set_def_ok_text@BasePop@@SAHPAD@Z
// size      98 bytes
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470
Return Value: No errors (0); null argument (3); allocation failed (5)
Status: Semantics transcribed from the image

PROMOTED FROM src/unrecovered/006018a0.cpp, which reached the buffer through
`reinterpret_cast<char **>(g_009b8d80)`.

THE COPY IS `dst[0] = 0` FOLLOWED BY A STRCAT, which is what the image does
everywhere it copies a string - two arguments and `add esp, 8`, not the
bounded three.
*/
__forceinline int __cdecl BasePop::set_def_ok_text(LPSTR text) {
    if (text == nullptr) {
        return 3;
    }
    if (BasePopDefaultOkText != nullptr) {
        free(BasePopDefaultOkText);
        BasePopDefaultOkText = nullptr;
    }
    BasePopDefaultOkText = static_cast<char *>(mem_get(strlen(text) + 1));
    if (BasePopDefaultOkText == nullptr) {
        return 5;
    }
    BasePopDefaultOkText[0] = 0;
    strcat(BasePopDefaultOkText, text);
    return 0;
}

/*
Purpose: Replace the default Cancel caption with a heap copy of `text`.
// ORIGINAL: 0x00601910 ?set_def_cancel_text@BasePop@@QAAHPAD@Z 0x00601910-0x00601972 BYTE_EXACT
// symbol    ?set_def_cancel_text@BasePop@@SAHPAD@Z
// size      98 bytes
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470
Return Value: No errors (0); null argument (3); allocation failed (4)
Status: Semantics transcribed from the image

PROMOTED FROM src/unrecovered/00601910.cpp. NOTE THE RETURN CODE: this one
answers 4 where its twin above answers 5 for the same failure. Both are in
the shipped bytes.
*/
__forceinline int __cdecl BasePop::set_def_cancel_text(LPSTR text) {
    if (text == nullptr) {
        return 3;
    }
    if (BasePopDefaultCancelText != nullptr) {
        free(BasePopDefaultCancelText);
        BasePopDefaultCancelText = nullptr;
    }
    BasePopDefaultCancelText = static_cast<char *>(mem_get(strlen(text) + 1));
    if (BasePopDefaultCancelText == nullptr) {
        return 4;
    }
    BasePopDefaultCancelText[0] = 0;
    strcat(BasePopDefaultCancelText, text);
    return 0;
}

/*
// ORIGINAL: 0x00604590 ?init_class@BasePop@@QAAHXZ 0x00604590-0x0060467A BYTE_EXACT
// LEVER: inline-the-two-setters the body is `set_def_cancel_text("Cancel");` then `set_def_ok_text("OK");` INLINED, not open-coded. Each inlined copy keeps its own `if (text == nullptr) return 3;` and `return 5`/`return 4` failure exits, which is why the image has `mov eax, <caption>; test eax, eax; je` before each block and why both failure paths jump to the START OF THE NEXT BLOCK (0x006045F8, 0x00604650) rather than to a return. The two setters are `__forceinline` here and MUST be defined ABOVE this body: VC6 is single-pass and will not inline a definition it has not seen. Both keep their own out-of-line BYTE_EXACT bodies, which /Gy still emits.
// LEVER: string-literal-not-address the captions are the LITERALS "Cancel" (0x00697008) and "OK" (0x00697010), read out of the image. Spelled as `(LPSTR)0x00697008` VC6 folds `text == nullptr` to false at compile time and the whole `mov/test/je` triple disappears; a literal's address is a RELOCATION, which VC6 cannot fold and the comparison masks away.
// LEVER: no-hook-local `(*reinterpret_cast<FnPtr *>(g_00696ecc))()` written out at BOTH call sites. Binding it to a local `FnPtr *const` costs a callee-saved esi, a push/pop pair, and `call dword ptr [esi]` where the image has `call dword ptr [0x696ECC]`.
// LEVER: shared-load `const Font *base = FontDefault;` for both stores - the image loads 0x009BB484 ONCE (0x00604590) and stores eax twice. Written as two assignments straight from the global, VC6 must re-read it because the first store may alias.
// LEVER: guard-clause-tail `if (!r1) { return 4; }` then the second hook call. The image falls through to `mov eax, 4; ret` and jumps FORWARD to the second call, which is a guard clause and not an `if (r1) {...} return 4;`. The final `return r2 ? 0 : 4;` is VC6's `neg/sbb/and al,0xFC/add` select; spelling it as arithmetic on `(r2 != 0)` emits setne instead.
// symbol    ?init_class@BasePop@@SAHXZ
// size      234 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470
// indirect  0x00604650 0x00604665
//
// Promoted 2026-08-15 from src/unrecovered/00604590.cpp to retire its
// pending_bodies forwarder. Copies the two default-caption strings into
// heap buffers, then reaches the two allocation hooks through the pointer at
// 0x00696ECC, as the original does.
// LEVER (2026-08-29, slot conversion): the two slot stores go through the
// real `Win *PopupInstanceSlotA/B` objects (basepop.h) instead of the
// g_009bc074/78 bindings - the same `mov dword ptr [addr], eax` folded
// shape, claim re-measured BYTE_EXACT after, and `pops` (popup.cpp) now
// reads the same storage.
Status: Complete
*/
int __cdecl BasePop::init_class() {
    const Font *const base = FontDefault;
    *reinterpret_cast<Font **>(&BasePopDefaultStringFonts[0]) = const_cast<Font *>(base);
    *reinterpret_cast<Font **>(&BasePopDefaultButtonFonts[0]) = const_cast<Font *>(base);

    set_def_cancel_text(const_cast<LPSTR>("Cancel"));
    set_def_ok_text(const_cast<LPSTR>("OK"));

    BasePop *const r1 = PopupAllocHook();
    PopupInstanceSlotA = reinterpret_cast<Win *>(r1);
    if (r1 == 0) {
        return 4;
    }
    BasePop *const r2 = PopupAllocHook();
    PopupInstanceSlotB = reinterpret_cast<Win *>(r2);
    return r2 ? 0 : 4;
}
/*
Purpose: The BasePop constructor. The base GraphicWin and the Heap, two
         FlatButtons and Sprite members come up implicitly, in declaration
         order, which is the image's own order; the Dialogs slab at +0x21d0
         and the Spot at +0x3098 are constructed MID-BODY by the image
         (explicit constructor calls, SEH states 6 and 7) and are NOT spelled
         here - see the STRUCTURE notes. Everything else is the field-store
         sequence: two popup-list link insertions (the nodes are static
         objects of the image's .data), the default fonts/colours copied
         from the class defaults, and the state resets.
// ORIGINAL: 0x00600860 ??0BasePop@@QAE@XZ 0x00600860-0x00600EF5;0x00662D80-0x00662E02
// size      1685 bytes
// prototype void (__thiscall ??0BasePop@@QAE@XZ)(BasePop* this)
// callers   3   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x005D4560 0x005E37E0 0x005E3820 0x005FA860 0x00607CF0
//           0x00612830
// TRIED: this body constructs the Dialogs slab (+0x21d0) and Spot (+0x3098)
//        NOWHERE. The image builds both MID-BODY (explicit ctor calls under
//        SEH states 6 and 7, `push 1; call 0x612830` then `call 0x5fa860`),
//        which no member declaration can order: declared members are built
//        before the body runs. Modelling them needs a typed pointer to the
//        Dialogs slab, and a fresh raw self-access is ratchet-blocked (the
//        count is exactly at its ceiling), so the calls are absent here and
//        spot_'s own implicit construction runs at the WRONG position (right
//        after sprite_, where the image has body stores). spot_ as a declared
//        Spot member is itself a STRUCTURE error for the same reason.
// TRIED: the two vtable stores ([this] = 0x6698d4, [this+0x444] = 0x6698cc)
//        are absent for the same ratchet reason - the tree keeps the
//        GraphicWin chain non-virtual, so nothing stores them implicitly.
//        VC6 itself emits `mov [ebx],0 / mov [ebx+0x444],0` at those slots
//        (the derived-class vptr refresh with no vtable of its own), so the
//        SLOTS exist in the compiled frame at the image's own positions.
// LEVER: a REAL constructor reproduces the image's SEH frame natively - the
//        first five instructions (push -1; handler; fs:[0] chain) agree
//        exactly, unlike the CheckBox family whose bases are construct()
//        methods and so get no frame. Measured 10/321 at /O2 /GX; the
//        remaining disagreement is register colouring: the image keeps
//        `this` in EBP and a zero temp in EBX for the whole body, this body
//        gets them swapped (EBX=this, EBP=0), which flips the base register
//        of every one of the ~300 field stores - mnemonic-identical,
//        byte-different. Not chased further this pass.
// STRUCTURE BasePop ctor | Dialogs slab at 0x21d0 | explicit mid-body
//        construction, SEH state 6 | image `push 1; call 0x612830` at
//        0x006009cc between the popup-list insertions and the vtable stores.
// STRUCTURE BasePop ctor | Spot at 0x3098 | explicit mid-body construction,
//        SEH state 7 | image `call 0x5fa860` at 0x006009dc; spot_ declared as
//        a member today constructs it implicitly at the wrong position.
// STRUCTURE BasePop ctor | SEH states skip 4 and 5 | state bytes stored are
//        0,1,2,3,6,7,8 | `mov byte ptr [esp+0x20], N` sequence 0x00600893..
//        0x006009f7 - the numbering is not one-per-subobject.
// LEVER: real-data-bindings the ~70 `g_00XXXXXX` fixed-address bindings and
//        the `RD(reinterpret_cast<int>(g_00XXXXXX))` default loads now name
//        real globals defined above the body (BasePopVbtable,
//        BasePopStageTables6693A0/6698C0, BasePopDefault*), each holding the
//        pinned image's bytes. The game addresses are unmapped in a
//        standalone build and the ctor faulted on the vbtable content read
//        at 0x00600907 (`RD(ecx + 4)`); the reads now resolve in-binary, and
//        the relocation discount keeps the immediates comparable. The .data
//        slots are zero at image load (.data's file bytes end at 0x006A8000),
//        so the 0x009B8xxx globals are deliberately zero-initialized; only
//        the 0x00696EC4 class-defaults table is file-backed and carries the
//        shipped values. popups_normal (0x00588460, in recovered/units)
//        rewrites most of both at run time.
// LEVER: font-slot-load-fix the font loads were spelled
//        `RD(reinterpret_cast<int>(BasePopDefaultStringFonts[N]))` - the
//        cast converts the font POINTER to an int, so RD dereferenced the
//        font object instead of the slot, two loads where the image has one
//        `mov eax, [0x009B8D98]`. The slot reads are direct now
//        (`string_font1_ = BasePopDefaultStringFonts[0]`); a dropped `&`
//        would have been the honest spelling of the old form.
// TRIED: real-data-bindings measured 9 of 321 against the old spellings' 10
//        of 321, with 396 compiled instructions against the old 513 (image
//        321). The first divergence is the same in both - this body's frame
//        is `sub esp, 0x10` where the image has `sub esp, 8`; the old
//        spelling was +4, this one +8. Register colouring (image ebp=this /
//        ebx=0, ours swapped) still dominates the remaining disagreement.
Return Value: n/a
Status: Complete
*/

// ---------------------------------------------------------------------------
// The data the ctor stages into every popup. These used to be fixed-address
// bindings (`static int *const g_00XXXXXX = (int *)0x00XXXXXX;`), which read
// the GAME's addresses; in a standalone build those pages are unmapped and
// the ctor faulted on its first content read, the vbtable load at
// 0x00600907. They are now real definitions carrying the pinned image's
// bytes, so every staged pointer and default load resolves inside this
// binary. Image addresses in the comments.

// --- .rdata: the staged vtables and the vbtable ---

// The vbtable staged at +0x2154 and +0x2184. Entries are displacements from
// the SLOT that holds the table's address: -4 reaches back to the primary
// vptr at +0x2150/+0x2180, +0x24 down to the StringList virtual base's vptr
// at +0x2178/+0x21a8, which the ctor also stores directly. The ctor READS
// entry[1] to place that virtual base's vtable - the read that faulted. In
// the image the table is the two dwords at 0x0066B0EC; 0x0066B0F4 opens with
// a function pointer, so the table ends there.
const uint32_t BasePopVbtable[2] = {0xFFFFFFFC, 0x00000024};  // 0x0066B0EC

// The function-pointer runs the ctor stages, Dialog-stage spellings first
// and the final spellings after the refresh. The staged starts are
// 0x006693A0/0x006693A4/0x006693AC and 0x006698C0/0x006698C4; 0x006693AC is
// StringAllocationBase's one-slot vftable (stringstruct.h) and 0x006693A0 /
// 0x006693A4 are the Dialog List tables (dialog.h, as value constants there
// - different storage, same image addresses). The first run ends at
// 0x006693AF because 0x006693B0 opens with a vbtable-shaped FFFFFFFC; the
// second ends at 0x006698D3 because 0x006698D4 is the [this] vptr the ctor's
// not-yet-spelled store writes.
const uint32_t BasePopStageTables6693A0[4] = {
    0x004029F0, 0x00404250, 0x00404270, 0x00401520};  // 0x006693A0
const uint32_t BasePopStageTables6698C0[5] = {0x00406670, 0x006086F0,
                                              0x00608770, 0x004070B0,
                                              0x00404420};  // 0x006698C0

// --- .data: the class-default slots the ctor loads ---

// The class-defaults table is FILE-BACKED in the image (0x00696EC4 sits below
// the 0x006A8000 end of .data's file bytes), so these carry the shipped
// initial values. popups_normal (0x00588460) rewrites parts of the table at
// run time before the first popup is built; the values here are the load
// state. Everything below this table - all the 0x009B8xxx slots - is past
// that end: zero at load, and left zero-initialized here on purpose.

// {100, 100}: the defaults the ctor's tail sends through field_A20_/A24_
// into the embedded Dialog's width/height slots.
const uint32_t BasePopDefaultDialogSize[2] = {100, 100};  // 0x00696EC4

// Copied to +0x21c0/+0x21c4. popups_normal rewrites these to 0 and 4.
const uint32_t BasePopDefaultField21C0[2] = {10, 4};  // 0x00696ED0

// Copied to +0x3104/+0x3108/+0x310c.
const uint32_t BasePopDefaultField3104[3] = {1, 1, 255};  // 0x00696ED8

// Copied to +0x31a0 (its low byte)/+0x31a4/+0x31a8/+0x31ac. popups_normal
// rewrites the first byte; the shipped byte is 0xf7.
const uint32_t BasePopDefaultField31A0[4] = {247, 7, 248, 2};  // 0x00696F54

// The two 0x118-byte default blocks the ctor copies to +0x2e64 and +0x2f7c.
// 0x118 is exactly ten sizeof(DialogEntry) records (dialog.h); the entries
// are all zero until popups_normal and the set_def_* setters fill them.
uint32_t BasePopDefaultBlock2E64[70];  // 0x009B8AB8
uint32_t BasePopDefaultBlock2F7C[70];  // 0x009B8BD0

// Copied to +0x3094, the head of the object's third block-sized region.
uint32_t BasePopDefaultField3094;  // 0x009B8CF8

// The defaults for field_A14_ (the high-res-scaling opt-out set_width tests)
// and field_A18_/field_A1C_ (the starting loc_a_/loc_b_; 0x2000 keeps the
// current value, so zero here means "location 0,0").
uint32_t BasePopDefaultLayout[3];  // 0x009B8D04

// The 27 dwords the ctor transposes into +0x31b8..+0x3223.
uint32_t BasePopDefaultBlock31B8[27];  // 0x009B8D10

// Copied to +0x21b0/+0x21b4/+0x21bc/+0x21b8, in this table's order.
// popups_normal seeds 0x1a/0x3a/0x32/3 here at run time.
uint32_t BasePopDefaultField21B0[4];  // 0x009B8D88

// Copied to +0x31b0/+0x31b4.
uint32_t BasePopDefaultField31B0[2];  // 0x009B8DB4

// Copied to +0x2148/+0x214c.
uint32_t BasePopDefaultField2148[2];  // 0x009B8DBC

// Copied to +0x30a4. popups_normal seeds 0x1000080 here at run time.
uint32_t BasePopDefaultField30A4;  // 0x009B8DCC

// Copied to +0x322c, immediately after the zero store the image overwrites.
// The image references this slot from 13 sites, all outside this file.
uint32_t BasePopDefaultField322C;  // 0x009B8DD0

namespace {
// The register-transcription helper for the list-node walks: its arguments
// are local ints the body derived from named fields, never `this`.
int &RD(int address) {
    return *reinterpret_cast<int *>(address);
}
}  // namespace

BasePop::BasePop() {

    // GraphicWin(), heap_(), flat_button1_(), flat_button2_() and sprite_()
    // run via ordinary member construction - their declaration order here
    // matches the disassembly's constructor-call order.
    int eax, ecx, edx, esi, edi;
    int ebx = 0;

    field_2154_ = reinterpret_cast<int>(BasePopVbtable);
    field_2178_ = reinterpret_cast<int>(BasePopStageTables6693A0 + 3);
    eax = reinterpret_cast<int>(StringAllocationHeap);
    esi = reinterpret_cast<int>(BasePopStageTables6693A0 + 1);
    field_217C_ = eax;
    StringAllocationHeap = reinterpret_cast<Heap *>(ebx);
    ecx = field_2154_;
    field_2150_ = esi;
    edx = reinterpret_cast<int>(BasePopStageTables6693A0);
    eax = RD(ecx + 4);
    ecx = reinterpret_cast<int>(BasePopStageTables6698C0 + 1);
    RD(eax + reinterpret_cast<int>(&field_2154_)) = edx;
    eax = field_2154_;
    field_2158_ = ebx;
    field_215C_ = ebx;
    field_2160_ = ebx;
    field_2164_ = ebx;
    field_2168_ = ebx;
    field_2150_ = ecx;
    edi = RD(eax + 4);
    eax = reinterpret_cast<int>(BasePopStageTables6698C0);
    RD(edi + reinterpret_cast<int>(&field_2154_)) = eax;

    // unit_ids_'s construction (vbtable, staged vtables, allocator
    // hand-off, StringStruct zeroing) is the compiler's now, emitted from
    // the declared StringList member - these were its hand-spelled copy.


    eax = static_cast<int>(BasePopDefaultField30A4);
    field_30A4_ = eax;
    ecx = static_cast<int>(BasePopDefaultField3094);
    field_3094_ = ecx;

    memcpy(&field_2E64_, BasePopDefaultBlock2E64, 0x118);
    memcpy(&field_2F7C_, BasePopDefaultBlock2F7C, 0x118);
    {
        int ecxPtr = reinterpret_cast<int>(&field_31C4_);
        int eaxIdx = 0;
        do {
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8) + eaxIdx);
            eaxIdx += 4;
            RD(ecxPtr - 0xc) = edx;
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8 + 2) + eaxIdx);
            RD(ecxPtr) = edx;
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8 + 5) + eaxIdx);
            RD(ecxPtr + 0xc) = edx;
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8 + 8) + eaxIdx);
            RD(ecxPtr + 0x18) = edx;
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8 + 11) + eaxIdx);
            RD(ecxPtr + 0x24) = edx;
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8 + 14) + eaxIdx);
            RD(ecxPtr + 0x30) = edx;
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8 + 17) + eaxIdx);
            RD(ecxPtr + 0x3c) = edx;
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8 + 20) + eaxIdx);
            RD(ecxPtr + 0x48) = edx;
            edx = RD(reinterpret_cast<int>(BasePopDefaultBlock31B8 + 23) + eaxIdx);
            RD(ecxPtr + 0x54) = edx;
            ecxPtr += 4;
        } while (eaxIdx < 0xc);
    }

    eax = -1;
    field_21C8_ = ebx;
    field_21CC_ = eax;
    ok_text_ = reinterpret_cast<LPSTR>(ebx);
    cancel_text_ = reinterpret_cast<LPSTR>(ebx);
    field_A3C_ = reinterpret_cast<int>(this);
    field_20F4_ = ebx;
    field_A40_ = ebx;
    field_30A8_ = ebx;
    field_30AC_ = ebx;
    field_A44_ = eax;
    field_20F8_ = 0;
    field_20FC_ = 0;
    field_2100_ = 0;
    field_2104_ = 0;
    field_2108_ = 0;
    field_210C_ = 0;
    field_2110_ = 0;
    field_2114_ = 0;
    field_A48_ = ebx;
    field_21B8_ = BasePopDefaultField21B0[3];
    field_21BC_ = BasePopDefaultField21B0[2];
    field_21B0_ = BasePopDefaultField21B0[0];
    field_21B4_ = BasePopDefaultField21B0[1];
    field_21C0_ = BasePopDefaultField21C0[0];
    field_21C4_ = BasePopDefaultField21C0[1];

    sprite_.close();

    field_2144_ = ebx;
    field_214C_ = BasePopDefaultField2148[1];
    field_2148_ = BasePopDefaultField2148[0];
    field_30B0_ = ebx;
    field_30B4_ = ebx;
    field_30B8_ = ebx;
    field_30BC_ = ebx;
    field_30C0_ = ebx;
    field_30C4_ = ebx;
    field_30C8_ = ebx;
    field_30CC_ = ebx;
    field_30D0_ = ebx;
    field_30D4_ = ebx;
    field_30D8_ = ebx;
    field_30DC_ = ebx;
    field_30E0_ = ebx;
    field_30E4_ = ebx;
    field_30E8_ = ebx;
    field_30EC_ = ebx;
    field_30F0_ = ebx;
    eax = 0x1000;
    field_30F4_ = ebx;
    loc_a_ = eax;
    loc_b_ = eax;
    field_3100_ = ebx;
    field_3104_ = BasePopDefaultField3104[0];
    field_3108_ = BasePopDefaultField3104[1];
    field_310C_ = BasePopDefaultField3104[2];
    string_font1_ = BasePopDefaultStringFonts[0];
    string_color_a_ = static_cast<int>(BasePopDefaultStringColors[0][0]);
    string_color_b_ = static_cast<int>(BasePopDefaultStringColors[1][0]);
    string_color_c_ = static_cast<int>(BasePopDefaultStringColors[2][0]);
    string_color_d_ = static_cast<int>(BasePopDefaultStringColors[3][0]);
    string_font2_ = BasePopDefaultStringFonts[1];
    string_color_2a_ = static_cast<int>(BasePopDefaultStringColors[0][1]);
    string_color_2b_ = static_cast<int>(BasePopDefaultStringColors[1][1]);
    string_color_2c_ = static_cast<int>(BasePopDefaultStringColors[2][1]);
    string_color_2d_ = static_cast<int>(BasePopDefaultStringColors[3][1]);
    string_font3_ = BasePopDefaultStringFonts[2];
    string_color_3a_ = static_cast<int>(BasePopDefaultStringColors[0][2]);
    string_color_3b_ = static_cast<int>(BasePopDefaultStringColors[1][2]);
    string_color_3c_ = static_cast<int>(BasePopDefaultStringColors[2][2]);
    string_color_3d_ = static_cast<int>(BasePopDefaultStringColors[3][2]);
    string_font4_ = BasePopDefaultStringFonts[3];
    string_color_hyper_a_ = static_cast<int>(BasePopDefaultStringColors[0][3]);
    string_color_hyper_b_ = static_cast<int>(BasePopDefaultStringColors[1][3]);
    string_color_hyperc_ = static_cast<int>(BasePopDefaultStringColors[2][3]);
    string_color_hyper_d_ = static_cast<int>(BasePopDefaultStringColors[3][3]);
    field_3160_ = ebx;
    field_3164_ = ebx;
    field_3168_ = ebx;
    button_font1_ = BasePopDefaultButtonFonts[0];
    button_color_a_ = static_cast<uint8_t>(BasePopDefaultButtonColors[0][0]);
    button_color_b_ = static_cast<int>(BasePopDefaultButtonColors[1][0]);
    button_color_c_ = static_cast<int>(BasePopDefaultButtonColors[2][0]);
    button_color_d_ = static_cast<int>(BasePopDefaultButtonColors[3][0]);
    button_font2_ = BasePopDefaultButtonFonts[1];
    button_color_2a_ = static_cast<uint8_t>(BasePopDefaultButtonColors[0][1]);
    button_color_2b_ = static_cast<int>(BasePopDefaultButtonColors[1][1]);
    button_color_2c_ = static_cast<int>(BasePopDefaultButtonColors[2][1]);
    button_color_2d_ = static_cast<int>(BasePopDefaultButtonColors[3][1]);
    button_font3_ = BasePopDefaultButtonFonts[2];
    button_color_3a_ = static_cast<uint8_t>(BasePopDefaultButtonColors[0][2]);
    button_color_3b_ = static_cast<int>(BasePopDefaultButtonColors[1][2]);
    button_color_3c_ = static_cast<int>(BasePopDefaultButtonColors[2][2]);
    button_color_3d_ = static_cast<int>(BasePopDefaultButtonColors[3][2]);
    field_31A0_ = static_cast<uint8_t>(BasePopDefaultField31A0[0]);
    field_31A4_ = BasePopDefaultField31A0[1];
    field_31A8_ = BasePopDefaultField31A0[2];
    field_31AC_ = BasePopDefaultField31A0[3];
    field_31B0_ = BasePopDefaultField31B0[0];
    field_31B4_ = BasePopDefaultField31B0[1];
    field_322C_ = ebx;
    field_322C_ = BasePopDefaultField322C;
    field_A14_ = BasePopDefaultLayout[0];
    field_A18_ = BasePopDefaultLayout[1];
    field_A1C_ = BasePopDefaultLayout[2];
    field_A20_ = BasePopDefaultDialogSize[0];
    field_A24_ = BasePopDefaultDialogSize[1];

    if (field_A14_ != 0) {
        // The image's tail here walks the Dialogs subobject at +0x21d0 (see
        // the STRUCTURE note under the marker): [dialogs+8] selects two
        // slots that receive field_A20_/field_A24_ (the latter grown 3/2 for
        // wide screens), and field_A18_/field_A1C_ overwrite loc_a_/loc_b_
        // unless they equal 0x2000. Not spelled: this body neither constructs
        // the Dialogs subobject nor may take a fresh raw pointer to it (the
        // raw self-access ratchet sits at its ceiling), so the reads would
        // consume garbage. Absent on purpose.
        if (field_A18_ != 0x2000) {
            loc_a_ = field_A18_;
        }
        if (field_A1C_ != 0x2000) {
            loc_b_ = field_A1C_;
        }
    }

}
