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
#include "basepop.h"

/*
Purpose: Update either base-popup coordinate unless its keep-current sentinel is supplied.
Original Offset: 00601B80
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
Original Offset: 006048C0
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
Original Offset: 006049C0
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
Original Offset: 00604900
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
Original Offset: 00604930
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
Original Offset: 00604960
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
Original Offset: 00604990
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
Original Offset: 006049F0
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
Original Offset: 00604A20
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
Original Offset: 00604A50
Status: Complete
*/
void BasePop::set_def_button_color3(int c1, int c2, int c3, int c4) {
    store_colors(BasePopDefaultButtonColors, ButtonColorStride, 2, c1, c2, c3, c4);
}

void __cdecl base_pop_set_def_button_color3_redirect(int c1, int c2, int c3, int c4) {
    BasePop::set_def_button_color3(c1, c2, c3, c4);
}
