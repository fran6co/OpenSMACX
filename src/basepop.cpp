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
#include "dialogs.h"
#include <cstring>

/*
Purpose: Update either base-popup coordinate unless its keep-current sentinel is supplied.
ORIGINAL: 0x00601B80 BYTE_EXACT
// name      ?set_loc@BasePop@@QAEXHH@Z
// size      37 bytes
// spans     0x00601B80-0x00601BA5
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

void __fastcall base_pop_set_loc_redirect(BasePop *self, void *, int x, int y) {
    self->set_loc(x, y);
}

Font **BasePopDefaultStringFonts = reinterpret_cast<Font **>(0x009B8D98);
Font **BasePopDefaultButtonFonts = reinterpret_cast<Font **>(0x009B8DA8);

namespace {

// Shared by every default-font setter in the codebase: the primary is
// published only when it is initialized, the remaining slots are stored
// unconditionally, and only a null primary is an error.
int publish_default_fonts(Font **slots, Font *const *fonts, size_t count) {
    if (!fonts[0]) {
        return 3;
    }
    volatile Font **const target = const_cast<volatile Font **>(slots);
    if (fonts[0]->is_initialized()) {
        target[0] = fonts[0];
    }
    for (size_t index = 1; index < count; ++index) {
        target[index] = fonts[index];
    }
    return 0;
}

}  // namespace

/*
Purpose: Set the default string fonts shared by every popup.
ORIGINAL: 0x006048C0
// name      ?set_def_string_font@BasePop@@QAAHPAUFont@@PAUFont@@PAUFont@@PAUFont@@@Z
// size      58 bytes
// spans     0x006048C0-0x006048FA
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
    Font *const fonts[4] = {font1, font2, font3, font4};
    return publish_default_fonts(BasePopDefaultStringFonts, fonts, 4);
}

/*
Purpose: Set the default button fonts shared by every popup.
ORIGINAL: 0x006049C0
// name      ?set_def_button_font@BasePop@@QAAHPAUFont@@PAUFont@@PAUFont@@@Z
// size      48 bytes
// spans     0x006049C0-0x006049F0
// prototype 
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int BasePop::set_def_button_font(Font *font1, Font *font2, Font *font3) {
    Font *const fonts[3] = {font1, font2, font3};
    return publish_default_fonts(BasePopDefaultButtonFonts, fonts, 3);
}

int __cdecl base_pop_set_def_string_font_redirect(
        Font *font1, Font *font2, Font *font3, Font *font4) {
    return BasePop::set_def_string_font(font1, font2, font3, font4);
}

int __cdecl base_pop_set_def_button_font_redirect(
        Font *font1, Font *font2, Font *font3) {
    return BasePop::set_def_button_font(font1, font2, font3);
}

// Slot s of tier t lives at base + s * stride + t * 4. The string table has
// four tiers so its slots are 0x10 apart; the button table has three, so its
// slots are 0xC apart. Getting that stride wrong writes into a sibling slot
// rather than a sibling tier, which is why the tests compare whole tables.
uint32_t *BasePopDefaultStringColors = reinterpret_cast<uint32_t *>(0x00696EE4);
uint32_t *BasePopDefaultButtonColors = reinterpret_cast<uint32_t *>(0x00696F24);

namespace {

constexpr size_t StringColorStride = 0x10;
constexpr size_t ButtonColorStride = 0x0C;

void store_colors(uint32_t *table, size_t stride, size_t tier,
                  int c1, int c2, int c3, int c4) {
    volatile uint32_t *const slots = table;
    const int colors[4] = {c1, c2, c3, c4};
    for (size_t slot = 0; slot < 4; ++slot) {
        slots[(slot * stride + tier * 4) / 4] = static_cast<uint32_t>(colors[slot]);
    }
}

}  // namespace

/*
Purpose: Set default string colour tier 0 shared by every popup.
ORIGINAL: 0x00604900
// name      ?set_def_string_color@BasePop@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00604900-0x00604927
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_string_color(int c1, int c2, int c3, int c4) {
    store_colors(BasePopDefaultStringColors, StringColorStride, 0, c1, c2, c3, c4);
}

void __cdecl base_pop_set_def_string_color_redirect(int c1, int c2, int c3, int c4) {
    BasePop::set_def_string_color(c1, c2, c3, c4);
}

/*
Purpose: Set default string colour tier 1 shared by every popup.
ORIGINAL: 0x00604930
// name      ?set_def_string_color2@BasePop@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00604930-0x00604957
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_string_color2(int c1, int c2, int c3, int c4) {
    store_colors(BasePopDefaultStringColors, StringColorStride, 1, c1, c2, c3, c4);
}

void __cdecl base_pop_set_def_string_color2_redirect(int c1, int c2, int c3, int c4) {
    BasePop::set_def_string_color2(c1, c2, c3, c4);
}

/*
Purpose: Set default string colour tier 2 shared by every popup.
ORIGINAL: 0x00604960
// name      ?set_def_string_color3@BasePop@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00604960-0x00604987
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_string_color3(int c1, int c2, int c3, int c4) {
    store_colors(BasePopDefaultStringColors, StringColorStride, 2, c1, c2, c3, c4);
}

void __cdecl base_pop_set_def_string_color3_redirect(int c1, int c2, int c3, int c4) {
    BasePop::set_def_string_color3(c1, c2, c3, c4);
}

/*
Purpose: Set default string colour tier 3 shared by every popup.
ORIGINAL: 0x00604990
// name      ?set_def_string_color_hyper@BasePop@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00604990-0x006049B7
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_string_color_hyper(int c1, int c2, int c3, int c4) {
    store_colors(BasePopDefaultStringColors, StringColorStride, 3, c1, c2, c3, c4);
}

void __cdecl base_pop_set_def_string_color_hyper_redirect(int c1, int c2, int c3, int c4) {
    BasePop::set_def_string_color_hyper(c1, c2, c3, c4);
}

/*
Purpose: Set default button colour tier 0 shared by every popup.
ORIGINAL: 0x006049F0
// name      ?set_def_button_color@BasePop@@QAAXHHHH@Z
// size      39 bytes
// spans     0x006049F0-0x00604A17
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_button_color(int c1, int c2, int c3, int c4) {
    store_colors(BasePopDefaultButtonColors, ButtonColorStride, 0, c1, c2, c3, c4);
}

void __cdecl base_pop_set_def_button_color_redirect(int c1, int c2, int c3, int c4) {
    BasePop::set_def_button_color(c1, c2, c3, c4);
}

/*
Purpose: Set default button colour tier 1 shared by every popup.
ORIGINAL: 0x00604A20
// name      ?set_def_button_color2@BasePop@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00604A20-0x00604A47
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_button_color2(int c1, int c2, int c3, int c4) {
    store_colors(BasePopDefaultButtonColors, ButtonColorStride, 1, c1, c2, c3, c4);
}

void __cdecl base_pop_set_def_button_color2_redirect(int c1, int c2, int c3, int c4) {
    BasePop::set_def_button_color2(c1, c2, c3, c4);
}

/*
Purpose: Set default button colour tier 2 shared by every popup.
ORIGINAL: 0x00604A50
// name      ?set_def_button_color3@BasePop@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00604A50-0x00604A77
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::set_def_button_color3(int c1, int c2, int c3, int c4) {
    store_colors(BasePopDefaultButtonColors, ButtonColorStride, 2, c1, c2, c3, c4);
}

void __cdecl base_pop_set_def_button_color3_redirect(int c1, int c2, int c3, int c4) {
    BasePop::set_def_button_color3(c1, c2, c3, c4);
}

/*
Purpose: Set this popup's string color slots.
ORIGINAL: 0x00604730 BYTE_EXACT
// name      ?set_string_color@BasePop@@QAEXHHHH@Z
// size      43 bytes
// spans     0x00604730-0x0060475B
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

void __fastcall base_pop_set_string_color_redirect(
        BasePop *self, void *, int c1, int c2, int c3, int c4) {
    self->set_string_color(c1, c2, c3, c4);
}

/*
Purpose: Set this popup's string color2 slots.
ORIGINAL: 0x00604760 BYTE_EXACT
// name      ?set_string_color2@BasePop@@QAEXHHHH@Z
// size      43 bytes
// spans     0x00604760-0x0060478B
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

void __fastcall base_pop_set_string_color2_redirect(
        BasePop *self, void *, int c1, int c2, int c3, int c4) {
    self->set_string_color2(c1, c2, c3, c4);
}

/*
Purpose: Set this popup's string color3 slots.
ORIGINAL: 0x00604790 BYTE_EXACT
// name      ?set_string_color3@BasePop@@QAEXHHHH@Z
// size      43 bytes
// spans     0x00604790-0x006047BB
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

void __fastcall base_pop_set_string_color3_redirect(
        BasePop *self, void *, int c1, int c2, int c3, int c4) {
    self->set_string_color3(c1, c2, c3, c4);
}

/*
Purpose: Set this popup's string color hyper slots.
ORIGINAL: 0x006047C0 BYTE_EXACT
// name      ?set_string_color_hyper@BasePop@@QAEXHHHH@Z
// size      43 bytes
// spans     0x006047C0-0x006047EB
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

void __fastcall base_pop_set_string_color_hyper_redirect(
        BasePop *self, void *, int c1, int c2, int c3, int c4) {
    self->set_string_color_hyper(c1, c2, c3, c4);
}

/*
Purpose: Set this popup's button color slots.
ORIGINAL: 0x00604830 BYTE_EXACT
// name      ?set_button_color@BasePop@@QAEXDHHH@Z
// size      43 bytes
// spans     0x00604830-0x0060485B
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

void __fastcall base_pop_set_button_color_redirect(
        BasePop *self, void *, char c1, int c2, int c3, int c4) {
    self->set_button_color(c1, c2, c3, c4);
}

/*
Purpose: Set this popup's button color2 slots.
ORIGINAL: 0x00604860 BYTE_EXACT
// name      ?set_button_color2@BasePop@@QAEXDHHH@Z
// size      43 bytes
// spans     0x00604860-0x0060488B
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

void __fastcall base_pop_set_button_color2_redirect(
        BasePop *self, void *, char c1, int c2, int c3, int c4) {
    self->set_button_color2(c1, c2, c3, c4);
}

/*
Purpose: Set this popup's button color3 slots.
ORIGINAL: 0x00604890 BYTE_EXACT
// name      ?set_button_color3@BasePop@@QAEXDHHH@Z
// size      43 bytes
// spans     0x00604890-0x006048BB
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

void __fastcall base_pop_set_button_color3_redirect(
        BasePop *self, void *, char c1, int c2, int c3, int c4) {
    self->set_button_color3(c1, c2, c3, c4);
}

int *BasePopFalloutGate = reinterpret_cast<int *>(0x009B8D00);
int *BasePopFalloutFlag = reinterpret_cast<int *>(0x009B8CFC);

/*
Purpose: Raise the fallout flag, but only while the gate is set.
ORIGINAL: 0x00604A90
// name      ?fallout@BasePop@@QAAXXZ
// size      20 bytes
// spans     0x00604A90-0x00604AA4
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
void BasePop::fallout() {
    if (*BasePopFalloutGate != 0) {
        *BasePopFalloutFlag = 1;
    }
}

void __cdecl base_pop_fallout_redirect() {
    BasePop::fallout();
}

func_set_state_flag CheckBoxOriginalSetStateFlag =
    original_method<func_set_state_flag>(0x0060ECE0);

/*
Purpose: Set the popup's embedded check box, at 0x2228, to the given state.
ORIGINAL: 0x00601BB0
// name      ?write_check@BasePop@@QAEXJ@Z
// size      19 bytes
// spans     0x00601BB0-0x00601BC3
// prototype void (__thiscall ?write_check@BasePop@@QAEXJ@Z)(BasePop* this, int)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0060ECE0
Return Value: n/a
Status: Complete
*/
void BasePop::write_check(long value) {
    (ORIGINAL(reinterpret_cast<uint8_t *>(this) + 0x2228)->*CheckBoxOriginalSetStateFlag)(value);
}

void __fastcall base_pop_write_check_redirect(BasePop *self, void *, long value) {
    self->write_check(value);
}

int32_t *BasePopScreenWidth = reinterpret_cast<int32_t *>(0x009B7B1C);

/*
Purpose: Set the dialog width, scaled to three-halves in the high-resolution
         layout. The scaling is skipped when the popup opts out through the
         flag at 0xA14, when bit 0x400 of the style at 0x30A8 is set, or when
         the screen is narrower than 1024. The width is written into the Dialog
         subobject of the embedded Dialogs, located through that Dialogs'
         own vbtable exactly as the original does rather than at a hardcoded
         offset.
ORIGINAL: 0x00601B20
// name      ?set_width@BasePop@@QAEXH@Z
// size      87 bytes
// spans     0x00601B20-0x00601B77
// prototype void (__thiscall ?set_width@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BasePop::set_width(int width) {
    int value = width;
    if (field_A14_ == 0 && (field_30A8_ & 0x400) == 0 &&
        *BasePopScreenWidth >= 0x400) {
        value = (width * 3) / 2;
    }
    uint8_t *const dialogs = reinterpret_cast<uint8_t *>(this) + 0x21D0;
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(dialogs);
    std::memcpy(dialogs + 0x2C + vbtable[2], &value, sizeof(value));
}

void __fastcall base_pop_set_width_redirect(BasePop *self, void *, int width) {
    self->set_width(width);
}

func_base_pop_exec BasePopExec = original_method<func_base_pop_exec>(0x00602600);

/*
Purpose: Run the popup modally with no completion callback.
ORIGINAL: 0x005A5900
// name      ?exec@BasePop@@QAEHXZ
// size      10 bytes
// spans     0x005A5900-0x005A590A
// prototype int (__thiscall ?exec@BasePop@@QAEHXZ)(BasePop* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00602600
Return Value: the exec result
Status: Complete
*/
int BasePop::exec() {
    return (ORIGINAL(this)->*BasePopExec)(0, nullptr);
}

/*
Purpose: Run the popup modally with a completion callback.
ORIGINAL: 0x00558FC0
// name      ?exec@BasePop@@QAEHP6AHXZ@Z
// size      18 bytes
// spans     0x00558FC0-0x00558FD2
// prototype int (__thiscall ?exec@BasePop@@QAEHP6AHXZ@Z)(BasePop* this, int (__cdecl *)())
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00602600
Return Value: the exec result
Status: Complete
*/
int BasePop::exec(int (__cdecl *callback)()) {
    return (ORIGINAL(this)->*BasePopExec)(0, callback);
}

int __fastcall base_pop_exec_void_redirect(BasePop *self, void *) {
    return self->exec();
}

int __fastcall base_pop_exec_callback_redirect(BasePop *self, void *,
                                               int (__cdecl *callback)()) {
    return self->exec(callback);
}

/*
Purpose: Report whether the popup will accept a key click. Both this and
         on_key_up below are the same four instructions over one field:

             mov eax, [ecx+0x30A8] / not eax / shr eax, 0xE / and eax, 1

         so the answer is bit 14 of field_30A8_ INVERTED - one when the bit is
         clear. The arguments are ignored; the two differ only in how many they
         clean off the stack.
ORIGINAL: 0x00604490 BYTE_EXACT
// name      ?on_key_click@BasePop@@QAEHHH@Z
// size      17 bytes
// spans     0x00604490-0x006044A1
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

int __fastcall base_pop_on_key_click_redirect(BasePop *self, void *, int a1, int a2) {
    return self->on_key_click(a1, a2);
}

/*
Purpose: As on_key_click above, over the same field and the same bit. Kept as
         its own body rather than delegating, because the original is a
         separate function with a different stack cleanup and a delegation
         would change the instruction the caller returns to.
ORIGINAL: 0x006044B0 BYTE_EXACT
// name      ?on_key_up@BasePop@@QAEHH@Z
// size      17 bytes
// spans     0x006044B0-0x006044C1
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

int __fastcall base_pop_on_key_up_redirect(BasePop *self, void *, int a1) {
    return self->on_key_up(a1);
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
ORIGINAL: 0x00605180 BYTE_EXACT
// name      ?UNK3@BasePop@@QAEXH@Z
// size      27 bytes
// spans     0x00605180-0x0060519B
// prototype void (__thiscall ?UNK3@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BasePop::UNK3(int a1) {
    uint32_t *const flags =
        reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(this) + 0x20);
    if (a1) {
        *flags |= 1U;
    } else {
        *flags &= ~1U;
    }
}

void __fastcall base_pop_unk3_redirect(BasePop *self, void *, int a1) {
    self->UNK3(a1);
}

/*
Purpose: As UNK3 above, over the same flags word but bit 1 (`or al, 2` /
         `and al, 0xFD`).
ORIGINAL: 0x006051A0 BYTE_EXACT
// name      ?UNK4@BasePop@@QAEXH@Z
// size      27 bytes
// spans     0x006051A0-0x006051BB
// prototype void (__thiscall ?UNK4@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BasePop::UNK4(int a1) {
    uint32_t *const flags =
        reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(this) + 0x20);
    if (a1) {
        *flags |= 2U;
    } else {
        *flags &= ~2U;
    }
}

void __fastcall base_pop_unk4_redirect(BasePop *self, void *, int a1) {
    self->UNK4(a1);
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
ORIGINAL: 0x00558FE0 BYTE_EXACT
// name      ?item@BasePop@@QAEHPADH@Z
// size      26 bytes
// spans     0x00558FE0-0x00558FFA
// prototype int (__thiscall ?item@BasePop@@QAEHPADH@Z)(BasePop* this, int8* lpString, int position)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00612A70
Return Value: whatever Dialogs::item returns
Status: Complete
*/
int BasePop::item(char *text, int index) {
    return reinterpret_cast<Dialogs *>(
        reinterpret_cast<uint8_t *>(this) + 0x21D0)->item(text, index);
}

int __fastcall base_pop_item_redirect(BasePop *self, void *, char *text,
                                      int index) {
    return self->item(text, index);
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
ORIGINAL: 0x00601BD0
// name      ?read_check@BasePop@@QAEXXZ
// size      17 bytes
// spans     0x00601BD0-0x00601BE1
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

uint32_t __fastcall base_pop_read_check_redirect(BasePop *self, void *) {
    return self->read_check();
}
