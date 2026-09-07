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
#include "popup.h"  // PopupStartCaption - set_def_file's 0x009B8AA8 buffer
#include "text.h"  // text_open / text_get / text_close - start's script reader
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
Purpose: Set the four fonts used by popup strings.
// ORIGINAL: 0x006046F0 ?set_string_font@BasePop@@QAEHPAUFont@@PAUFont@@PAUFont@@PAUFont@@@Z 0x006046F0-0x00604730 BYTE_EXACT
// symbol    ?set_string_font@BasePop@@QAEHPAVFont@@000@Z
// size      64 bytes
// prototype int (__thiscall ?set_string_font@BasePop@@QAEHPAUFont@@PAUFont@@PAUFont@@PAUFont@@@Z)(BasePop* this, Font*, Font*, Font*, Font*)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int BasePop::set_string_font(Font *font1, Font *font2, Font *font3, Font *font4) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        string_font1_ = font1;
    }
    string_font2_ = font2;
    string_font3_ = font3;
    string_font4_ = font4;
    return 0;
}

/*
Purpose: Install the three button fonts, refusing a null primary.

             mov eax,[esp+4] / test eax,eax / jne have
             mov eax,3 / ret 0xC
             have: mov edx,[eax+8] / test edx,edx / je skip
             mov [ecx+0x316C],eax
             skip: .. store 0x3170 and 0x3174 .. / xor eax,eax / ret 0xC

         Exactly the shape set_string_font above has, with three fonts instead
         of four: a null primary is refused with 3 and NOTHING is stored, while
         an uninitialised primary - `[eax+8]` zero, which is Font::
         is_initialized - is silently skipped while the other two are still
         installed. Those are different failures and the return code does not
         distinguish them, which is the original's behaviour and not an
         oversight here.
// ORIGINAL: 0x006047F0 ?set_button_font@BasePop@@QAEHPAUFont@@PAUFont@@PAUFont@@@Z 0x006047F0-0x00604826 BYTE_EXACT
// symbol    ?set_button_font@BasePop@@QAEHPAVFont@@00@Z
// size      54 bytes
// prototype int (__thiscall ?set_button_font@BasePop@@QAEHPAUFont@@PAUFont@@PAUFont@@@Z)(BasePop* this, Font*, Font*, Font*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 3 when the primary font is null, otherwise 0
Status: Complete
*/
int BasePop::set_button_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        button_font1_ = font1;
    }
    button_font2_ = font2;
    button_font3_ = font3;
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

// THE TWENTY CONFIG DIRECTIVE KEYWORDS BasePop::start (0x00601BF0) compares
// each '#' line against, one inline .data literal per keyword at
// 0x00696F64..0x00696FEC. Read out of the shipped image with image_data.py
// and verified byte for byte: "x ", "y " and "col " carry their trailing
// space, and the directive's own name is what each handler then parses -
// xs/ys/x/y/col seed the popup's default geometry, caption/checkbox/editbox/
// listbox/active/text/itemlist/button/extra pick the control set, and
// wave/vol/pitch/fade/pan/sprite are the sound and picture directives.
// These are the addresses the _strnicmp chain pushes. It compares FEWER
// characters than several of these strings run - itemlist and text/wave/fade
// per the start artifact's CORRECTIONS - so a keyword's compare length is NOT
// its strlen here; take each length off the image's own call site.
char BasePopKeywordXs[] = "xs";           // 0x00696F64
char BasePopKeywordYs[] = "ys";           // 0x00696F68
char BasePopKeywordX[] = "x ";            // 0x00696F6C
char BasePopKeywordY[] = "y ";            // 0x00696F70
char BasePopKeywordCol[] = "col ";        // 0x00696F74
char BasePopKeywordCaption[] = "caption";  // 0x00696F7C
char BasePopKeywordCheckbox[] = "checkbox";  // 0x00696F84
char BasePopKeywordEditbox[] = "editbox";   // 0x00696F90
char BasePopKeywordListbox[] = "listbox";   // 0x00696F98
char BasePopKeywordActive[] = "active";     // 0x00696FA0
char BasePopKeywordText[] = "text";         // 0x00696FA8
char BasePopKeywordItemlist[] = "itemlist";  // 0x00696FB0
char BasePopKeywordButton[] = "button";     // 0x00696FBC
char BasePopKeywordExtra[] = "extra";       // 0x00696FC4
char BasePopKeywordWave[] = "wave";         // 0x00696FCC
char BasePopKeywordVol[] = "vol";           // 0x00696FD4
char BasePopKeywordPitch[] = "pitch";       // 0x00696FD8
char BasePopKeywordFade[] = "fade";         // 0x00696FE0
char BasePopKeywordPan[] = "pan";           // 0x00696FE8
char BasePopKeywordSprite[] = "sprite";     // 0x00696FEC

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
    dialogs_.set_state_flag(value);
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
    // THE EMBEDDED DIALOG VIRTUAL BASE, reached as a qualified member access.
    // The qualification matters: unqualified `dialogs_.field_2C_` finds
    // ListBox's own field at the same offset 0x2C and stores there, while
    // `Dialog::field_2C_` makes VC6 walk the vbtable at +0x21d0 for the
    // virtual base's displacement - the image's `mov edx,[ecx+0x21d0];
    // mov edx,[edx+8]` before the store. dipedit.cpp's
    // `list_box_.Dialog::field_20_ &= ~4` is the same form.
    if (external_window_ == 0 && (field_30A8_ & 0x400) == 0 &&
        WinScreenWidth >= 0x400) {
        dialogs_.Dialog::field_2C_ = (width * 3) / 2;
    } else {
        dialogs_.Dialog::field_2C_ = width;
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
Purpose: Run the popup modally: size and position it, lay out its two caption
         rows (plus one row per item), hand control to the window's modal
         pump with the caller's completion callback, and report which row the
         user settled on.
// ORIGINAL: 0x00602600 ?exec@BasePop@@QAEHHP6AHXZ@Z 0x00602600-0x006037F5;0x00662E02-0x00662E17 FILE
// size      4618 bytes
// prototype int (__thiscall ?exec@BasePop@@QAEHHP6AHXZ@Z)(BasePop* this, int, int (__cdecl *)())
// callers   87   call targets   24
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005D4EF0 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DAD40 0x005EDF50 0x005F7E90 0x00605180 0x006051A0 0x00607210 0x00607360 0x006073A0 0x006073E0 0x00607CF0 0x00612060 0x00612970 0x00625EC0 0x00644F3A 0x006453E0 0x00645470 0x0064557F 0x0064558A
// indirect  0x00602620 0x0060279B 0x0060282E 0x00602954 0x00602B4D 0x00602C18 0x00602C64 0x00602ED5 0x00602F9A 0x00603260 0x00603331 0x006035B2 0x00603677 0x0060369C 0x006036A7 0x00603714 0x00603769 0x00603787
// TRANSCRIBED IN FULL 2026-09-06, homed from src/unrecovered/00602600.cpp.
//        Every branch of the image's 1146 instructions is modelled below; the
//        two captions, the per-item row loop, the modal pump and the fallout
//        gate are all here. The measurement is a structural one, not a byte
//        one - see RESIDUAL.
//
// THE FIVE VIRTUAL SLOTS this body dispatches on, from BasePop's own table
// (0x006698D4, the address the ctor stores at [this]):
//   0x17c -> 0x00406840 `mov eax,1 / ret`          the may-show gate
//   0x168 -> 0x00603800 calculate_dimensions       its result is the early out
//   0x16c -> 0x00404280 `ret`                      run before init
//   0x170 -> 0x00404280 `ret`                      run after init
//   0x30  -> 0x00604AB0 on_redraw                  after positioning
//   0x178 -> 0x00406B20 `ret 0xc`                  writes the row origin triple
//   0xe4  -> 0x005EE190                            THE MODAL PUMP
//   0x8   -> 0x005EDCD0                            the post-pump refresh
//   0x4   -> 0x005ED9D0, called with 3
// None of 0x17c/0x16c/0x170/0x178/0x4/0x8 has a name in this tree yet - the
// first four are the inherited do-nothing defaults, so no recovered caller
// has ever had to say what they are for.
//
// HANDOFF (win.h, not a file this pass owns): `struct BorderSizing;` is
//        incomplete. exec is the first body that builds one: the frame puts a
//        TWELVE-BYTE object at [esp+0x24] whose three dwords are this+0x114 /
//        +0x118 / +0x11c - caption_height_, border_thickness_ and
//        bottom_border_thickness_ - and the whole frame is `sub esp, 0x20`,
//        which is exactly 8 locals + 12, so BorderSizing has NO fourth field.
//        `struct BorderSizing { int caption_height; int border_thickness; int
//        bottom_border_thickness; };` in win.h lets the local below lose its
//        int[3] spelling.
// LANDED (2026-09-06, dialogs finisher): the two show-bit setters are Dialog's
//        set_style_bit_0/1, defined out of line in dialog.cpp (the claim stays
//        with BasePop's folded twin, 0x00605180/0x006051A0 below), and this
//        body makes the two real calls - call_diff 39 of 42 -> 41 of 42.
//        LEVER: the receiver is bound ONCE, `Dialog &dialog = dialogs_;`, not
//        spelled twice as `dialogs_.Dialog::set_style_bit_N(1)` - the double
//        qualified spelling recomputed the vbtable walk for the second call
//        where the image keeps it in edi, and cost 22 -> 15 agreeing
//        instructions; the single binding took it to 29.
//
// HANDOFF (flatbutton.h / basebutton.h, not files this pass owns): the
//        button's last 0x6C bytes (+0xAE0..+0xB48) are the 27 config dwords
//        the image copies from BasePop::default_block_ into all three button
//        sites, and +0xA84..+0xA98 are color_ and the bevel pair. Both headers
//        model those runs as PRIVATE offset names, so the two helpers below
//        walk the object address; one public array member over +0xAE0 and one
//        over +0xA84 remove the walks.
//
// HANDOFF (stringstruct.h): the image INLINES seek_pos (0x00602FC5, abs and
//        all) and next_entry (0x0060305C) into this body, the way it inlines
//        port_to_port into get_there. This tree defines both out of line in
//        stringstruct.cpp, so the source calls them and two calls the image
//        does not have remain. Moving the definitions into the header as
//        MEASURED inline is the fix, and it is a stringstruct edit.
//
// LEVER: the two caption defaults are TWO ARMS, not a ternary - each arm runs
//        its own strlen + mem_get + zero store and they join on the strcat,
//        so a `src ? src : "OK"` spelling collapses four calls into three.
// LEVER: the row origin the slot-0x178 call writes is kept LIVE by the image
//        across the whole button run and reloaded after the OK row - the
//        working row value is recomputed from it twice, not carried forward.
//        Holding one variable for both lost the reload and every row after
//        the first.
//
// BUG IN THE ORIGINAL: the cancel-caption fallback allocates
//        strlen("Cancel") + 1 and then strcat's "OK" into it (0x006033CE
//        pushes 0x696ffc, 0x006033EE pushes 0x697004 - two different string
//        literals). The OK caption's twin pair pushes 0x696ff4/0x696ff8, both
//        "OK". So a popup with no default cancel caption builds a buffer
//        sized for "Cancel" containing "OK". Reproduced deliberately: the
//        fallbacks are spelled separately below, and "fixing" the second one
//        would be a different program.
//
// BUG IN THE ORIGINAL: the per-item row loop bounds itself on the item count
//        while storing through zero_block_, an EIGHT-dword array the ctor
//        clears with one rep stosd, and checks no bound (0x0060334C `add
//        edi,4` against `cmp ebx,eax` at 0x00603351). A popup with more than
//        eight items walks past it into sprite_. Reproduced: the loop below
//        is the image's, unbounded.
//
// LEVER: (2026-09-06, the win-handoff wave) three of the four HANDOFFs on
//        this marker landed and neither of the two that could have moved this
//        score did. BorderSizing is now the real 12-byte struct in win.h and
//        apply_button_config / apply_button_bevel name FlatButton::config[27]
//        and BaseButton's public colour/bevel run instead of walking the
//        object address - the score held at instruction 0 through both. The
//        stringstruct move DID: seek_pos and next_entry are MEASURED inline in
//        stringstruct.h now, exec inlines both, and this body went 18/1139 ->
//        22/1139 (1021 -> 1059 compiled) with call_diff 37 -> 39 of 42.
// RESIDUAL: MISMATCH 29 of 1139 at /c /O2 /Gy /GR- /GX (2026-09-06, was 22),
//        first divergence at instruction 0. The image opens the frame
//        `mov eax, fs:[0]` and this tree opens it `push -1` - the SEH
//        registration and the fs reload are scheduled the other way round - so
//        the whole 1139-instruction listing is shifted before its first real
//        statement. Everything after that is the same program at a different
//        register colour: the image walks the embedded Dialog's vbtable in
//        ecx/edx where this tree uses eax/ecx, and no source spelling names a
//        register. GraphicWin::init (0x005D4EF0) records the same shape at 2 of
//        112 on a body a quarter of this size. Not chased further: 1067
//        compiled instructions against the image's 1139, the 60 missing being
//        the ONE call call_diff still counts - the abs inside the inlined
//        seek_pos, which is a stringstruct.h question (the HANDOFF above).
// TRIED: apply_button_config / apply_button_bevel as out-of-line definitions
//        in basepop.cpp - call_diff went to 43 calls against the image's 42,
//        the three sites the image has inline became real calls. In-class
//        definitions (what ships) took it to 37 against 42. Both spellings
//        measured MISMATCH at instruction 0 for the reason above, so the
//        in-class form wins on call fidelity alone.
*/
typedef int (__fastcall *func_basepop_slot095)(void *);
typedef int (__fastcall *func_basepop_exec_slot090)(void *);
typedef void (__fastcall *func_basepop_slot091)(void *);
typedef void (__fastcall *func_basepop_slot092)(void *);
typedef void (__fastcall *func_basepop_slot062)(void *);
// SLOT 0x178, the row-origin triple. Win declares slots 0..87 only, so this
// one has no name to call; the __stdcall spelling keeps the receiver as the
// first stack argument and the call operand in the slot, which is the same
// shape Net::delete_group (0x0062EDD0) is claimed BYTE_EXACT with.
typedef void (__stdcall *func_basepop_slot094)(void *, int *, int *, int *);

// THE FLAT BUTTON'S OWN SLOT 1, called with 0 after each button is configured
// (0x00602F9A, 0x00603331, 0x00603677). The VCall1 shape already in this file.
class VCallSlot001 {
 public:
    virtual void slot000();
    virtual void slot001(int);
};


int BasePop::exec(int flags, int (__cdecl *callback)()) {
    int pump_result = 0;

    // THE MAY-SHOW GATE. BasePop's own slot is `mov eax, 1 / ret`, so a plain
    // popup always passes and only a derived family can refuse.
    if (vtable_slot<func_basepop_slot095>(this, 0x17c)(this) == 0) {
        return -1;
    }

    // No items: no tab. More than one sprite and no external window: the
    // sprite-picker kind, the 8 Dialogs::calculate_dimensions switches on.
    if (dialogs_.Dialog::item_list_.entry_count_ == 0) {
        dialogs_.kind_ = -1;
    }
    if (dialogs_.SpriteBox::list_.entry_count_ > 1 && external_window_ == 0) {
        dialogs_.kind_ = 8;
    }

    if (external_window_ != 0 && external_window_->is_visible()) {
        // THE WIDE-SCREEN 3/2 GROW, the pair set_height spells.
        int width = static_cast<int>(dialog_width_);
        if (external_window_ == 0 && (field_30A8_ & 0x400) == 0 &&
            WinScreenWidth >= 0x400) {
            width = (width * 3) / 2;
        }
        dialogs_.Dialog::field_2C_ = width;
        int height = static_cast<int>(dialog_height_);
        if (external_window_ == 0 && (field_30A8_ & 0x400) == 0 &&
            WinScreenWidth >= 0x400) {
            height = (height * 3) / 2;
        }
        dialogs_.Dialog::field_30_ = height;

        if (default_loc_a_ != 0x2000) {
            loc_a_ = default_loc_a_;
        }
        if (default_loc_b_ != 0x2000) {
            loc_b_ = default_loc_b_;
        }
        // UNK3(1) and UNK4(1) on the Dialog base - the two show bits. The
        // image calls them (0x0060276C / 0x00602775) with the vbtable-adjusted
        // `&dialogs_.Dialog` as the receiver, computed ONCE into a register and
        // reused for both calls; the definitions are in dialog.cpp and the
        // claim is BasePop's folded twin (0x00605180 / 0x006051A0 in
        // basepop.cpp). See the HANDOFF on dialog.h above.
        Dialog &dialog = dialogs_;
        dialog.set_style_bit_0(1);
        dialog.set_style_bit_1(1);
    } else {
        external_window_ = 0;
        field_30A8_ &= 0xffefffff;
    }

    // Size it. A nonzero answer is exec's own answer.
    if (vtable_slot<func_basepop_exec_slot090>(this, 0x168)(this) != 0) {
        return 0;
    }

    if (flags != 0) {
        return 0;
    }
    if ((field_30A8_ & 0x8000) != 0) {
        return 0;
    }

    // THE PARENT AND THE STYLE WORD init is handed.
    Win *const parent = external_window_ != 0
                            ? external_window_
                            : reinterpret_cast<Win *>(field_322C_);
    int style = static_cast<int>(field_30A4_);
    if ((field_30A8_ & 0x20) != 0) {
        style |= 0x20;
    }
    if (external_window_ == 0) {
        style |= 1;
    } else {
        style |= 0x400000;
    }
    if (GraphicWinInitDefaults.top_left != 0) {
        style |= 0x10000000;
    }
    if ((field_30A8_ & 0x4100) != 0) {
        style |= 0x1000;
    }
    if ((field_30A8_ & 0x20000) == 0 && external_window_ == 0) {
        style |= 0x80000;
    }

    // The 0x800 bit is exec's own, added at the call and not before.
    if (owner_window_ == this) {
        vtable_slot<func_basepop_slot091>(this, 0x16c)(this);

        // THE BORDER, in the window's own three thicknesses. See the HANDOFF
        // on win.h: BorderSizing is twelve bytes, and this local is what
        // proves it - the frame has room for exactly three dwords here.
        int sizing[3];
        sizing[0] = caption_height_;
        sizing[1] = border_thickness_;
        sizing[2] = bottom_border_thickness_;
        if (GraphicWin::init(0, 0, static_cast<int>(field_30BC_),
                             static_cast<int>(field_30C0_), owned_text_,
                             style | 0x800, parent, static_cast<Menu *>(0),
                             reinterpret_cast<BorderSizing *>(sizing)) != 0) {
            return 0;
        }

        set_text_color(string_color_a_, string_color_b_, string_color_c_,
                       string_color_d_);
        set_text_color2(string_color_2a_, string_color_2b_, string_color_2c_,
                        string_color_2d_);
        set_text_color3(string_color_3a_, string_color_3b_, string_color_3c_,
                        string_color_3d_);
        set_text_color_hyper(string_color_hyper_a_, string_color_hyper_b_,
                             string_color_hyperc_, string_color_hyper_d_);
        set_font(string_font1_, string_font2_, string_font3_, string_font4_);

        if (owned_text_ != 0) {
            set_caption(owned_text_);
        }

        vtable_slot<func_basepop_slot092>(this, 0x170)(this);
    }

    // THE POPUP'S POSITION, one arm per owner. owner_window_ == this means
    // the popup is its own surface; otherwise it centres on the window that
    // opened it. 0x1000 is set_width's "centre on the screen" sentinel and a
    // negative value is an offset from that centre.
    if (owner_window_ == this) {
        if ((iSomeFlag_ & 2) != 0) {
            if (loc_a_ == 0x1000) {
                field_30B4_ = (client_rect_.left - client_rect_.right +
                               WinScreenWidth) /
                              2;
            } else if (static_cast<int>(loc_a_) < 0) {
                field_30B4_ = client_rect_.left - client_rect_.right +
                              static_cast<int>(loc_a_) + WinScreenWidth;
            } else {
                field_30B4_ = loc_a_;
            }
            if (loc_b_ == 0x1000) {
                field_30B8_ = (client_rect_.top - client_rect_.bottom +
                               WinScreenHeight) /
                              2;
            } else if (static_cast<int>(loc_b_) < 0) {
                field_30B8_ = client_rect_.top - client_rect_.bottom +
                              static_cast<int>(loc_b_) + WinScreenHeight;
            } else {
                field_30B8_ = loc_b_;
            }
        } else {
            if (loc_a_ == 0x1000) {
                field_30B4_ = (WinScreenWidth -
                               static_cast<int>(dib_.bmiHeader.biWidth)) /
                              2;
            } else if (static_cast<int>(loc_a_) < 0) {
                field_30B4_ = (static_cast<int>(loc_a_) -
                               static_cast<int>(dib_.bmiHeader.biWidth) +
                               WinScreenWidth) /
                              2;
            } else {
                field_30B4_ = loc_a_;
            }
            if (loc_b_ == 0x1000) {
                field_30B8_ = (static_cast<int>(dib_.bmiHeader.biHeight) +
                               WinScreenHeight) /
                              2;
            } else if (static_cast<int>(loc_b_) < 0) {
                field_30B8_ = static_cast<int>(loc_b_) +
                              static_cast<int>(dib_.bmiHeader.biHeight) +
                              WinScreenHeight;
            } else {
                field_30B8_ = loc_b_;
            }
        }
    } else {
        if (loc_a_ == 0x1000) {
            field_30B4_ = (static_cast<int>(
                               owner_window_->dib_.bmiHeader.biWidth) -
                           static_cast<int>(field_30BC_)) /
                          2;
        } else if (static_cast<int>(loc_a_) < 0) {
            field_30B4_ =
                static_cast<int>(owner_window_->dib_.bmiHeader.biWidth) -
                static_cast<int>(field_30BC_) + static_cast<int>(loc_a_);
        } else {
            field_30B4_ = loc_a_;
        }
        if (loc_b_ == 0x1000) {
            field_30B8_ = -(static_cast<int>(field_30C0_) +
                            static_cast<int>(
                                owner_window_->dib_.bmiHeader.biHeight)) /
                          2;
        } else if (static_cast<int>(loc_b_) < 0) {
            field_30B8_ = static_cast<int>(loc_b_) -
                          static_cast<int>(field_30C0_) -
                          static_cast<int>(owner_window_->dib_.bmiHeader.biHeight);
        } else {
            field_30B8_ = loc_b_;
        }
    }

    // DRAW IT. Slot 0x30 - Win::vslot_12, which BasePop's table points at
    // on_redraw (0x00604AB0).
    vslot_12();

    // THE ROW ORIGIN. Slot 0x178 hands all three back through pointers, which
    // is why the three exist before it runs. `origin` is the value it wrote;
    // `row` is the working copy the rows below consume and advance.
    int origin = -static_cast<int>(field_30F4_);
    int row = static_cast<int>(field_30C0_) - static_cast<int>(field_30EC_) -
              static_cast<int>(field_30E0_) - static_cast<int>(field_30D0_);
    int row_style = 0;
    vtable_method<func_basepop_slot094>(this, 0x178)(this, &origin, &row,
                                                     &row_style);

    // THE ROW PITCH, in the three shapes the image spells it.
    const int pitch = static_cast<int>(field_30E4_) +
                      static_cast<int>(field_30CC_);

    // A plain text box (or a popup drawing into another window) runs neither
    // the item rows nor the second caption: straight to the pump.
    if ((field_30A8_ & 0x80100) != 0 || owner_window_ != this) {
        goto pump;
    }
    if ((field_30A8_ & 0x10000) != 0) {
        // The item list is the only body: no OK row, the item rows start one
        // pitch back when the origin is negative.
        if (origin < 0) {
            row = origin - pitch * (static_cast<int>(field_30B0_) - 1);
        } else {
            row = origin;
        }
        goto rows;
    }
    if ((field_30A8_ & 0x40) != 0) {
        if (origin < 0) {
            row = origin - pitch;
        } else {
            row = origin + pitch * static_cast<int>(field_20F4_);
        }
    }

    // THE OK CAPTION, or the default. The two arms join on the strcat - see
    // the LEVER on the marker above.
    if (ok_text_ == 0) {
        const char *source;
        if (BasePopDefaultOkText != 0) {
            ok_text_ =
                static_cast<char *>(mem_get(strlen(BasePopDefaultOkText) + 1));
            ok_text_[0] = 0;
            source = BasePopDefaultOkText;
        } else {
            ok_text_ = static_cast<char *>(mem_get(strlen("OK") + 1));
            ok_text_[0] = 0;
            source = "OK";
        }
        strcat(ok_text_, source);
    }

    // THE FIRST ROW, the window's own OK row.
    flat_button1_.init(ok_text_, -1, origin, row, static_cast<int>(field_30CC_),
                       static_cast<int>(field_30D0_), this, row_style);
    apply_button_config(&flat_button1_);
    flat_button1_.set_font(button_font1_, button_font2_, button_font3_, 0);
    vtable_slot<func_basepop_slot062>(&flat_button1_, 0xf8)(&flat_button1_);
    flat_button1_.set_text_color(button_color_a_, button_color_b_,
                                 button_color_c_, button_color_d_);
    flat_button1_.set_text_color2(button_color_2a_, button_color_2b_,
                                  button_color_2c_, button_color_2d_);
    flat_button1_.set_text_color3(button_color_3a_, button_color_3b_,
                                  button_color_3c_, button_color_3d_);
    apply_button_bevel(&flat_button1_);
    reinterpret_cast<VCallSlot001 *>(&flat_button1_)->slot001(0);

    // THE ITEM ROWS ALWAYS RESTART FROM THE ORIGIN, never from where the OK
    // row was left: the image keeps the slot-0x178 value live in a register
    // across the whole run and reloads it here.
    if (origin < 0) {
        row = origin - pitch * (static_cast<int>(field_30B0_) - 1);
    } else {
        row = origin;
    }

rows:
    // ONE ROW PER ITEM. zero_block_ is the row the ctor clears with its one
    // rep stosd; the image walks it with the same four-byte step and no bound
    // of its own.
    if (item_ids_.entry_count_ > 0) {
        item_ids_.seek_pos(-1);
        int index = 0;
        for (; index < item_ids_.entry_count_; index++) {
            FlatButton *button = new FlatButton();
            zero_block_[index] = reinterpret_cast<uint32_t>(button);

            StringStructPayload *payload = item_ids_.next_entry();
            char *text = payload != 0 ? payload->text : 0;
            button->init(text, index + 1, origin, row,
                         static_cast<int>(field_30CC_),
                         static_cast<int>(field_30D0_), this, row_style);
            apply_button_config(button);
            button->set_font(button_font1_, button_font2_, button_font3_, 0);
            vtable_slot<func_basepop_slot062>(button, 0xf8)(button);
            button->set_text_color(button_color_a_, button_color_b_,
                                   button_color_c_, button_color_d_);
            button->set_text_color2(button_color_2a_, button_color_2b_,
                                    button_color_2c_, button_color_2d_);
            button->set_text_color3(button_color_3a_, button_color_3b_,
                                    button_color_3c_, button_color_3d_);
            apply_button_bevel(button);
            reinterpret_cast<VCallSlot001 *>(button)->slot001(0);

            row += pitch;
        }
    }

    if ((field_30A8_ & 0x40) == 0) {
        goto pump;
    }
    if (origin >= 0) {
        row = origin + pitch * (static_cast<int>(field_30B0_) - 1);
    } else {
        row = origin;
    }

    // THE CANCEL CAPTION, or the default. The two fallback strings are the
    // image's own and they disagree - see BUG IN THE ORIGINAL above.
    if (cancel_text_ == 0) {
        // Two DIFFERENT literals, as the image: the allocation sizes for
        // "Cancel" and the concatenation writes "OK". See BUG IN THE ORIGINAL
        // under the marker above.
        if (BasePopDefaultCancelText != 0) {
            cancel_text_ = static_cast<char *>(
                mem_get(strlen(BasePopDefaultCancelText) + 1));
            cancel_text_[0] = 0;
            strcat(cancel_text_, BasePopDefaultCancelText);
        } else {
            cancel_text_ = static_cast<char *>(mem_get(strlen("Cancel") + 1));
            cancel_text_[0] = 0;
            strcat(cancel_text_, "OK");
        }
    }

    // THE LAST ROW, the window's own Cancel row.
    flat_button2_.init(cancel_text_, -2, origin, row,
                       static_cast<int>(field_30CC_),
                       static_cast<int>(field_30D0_), this, row_style);
    apply_button_config(&flat_button2_);
    row -= pitch;
    flat_button2_.set_font(button_font1_, button_font2_, button_font3_, 0);
    vtable_slot<func_basepop_slot062>(&flat_button2_, 0xf8)(&flat_button2_);
    flat_button2_.set_text_color(button_color_a_, button_color_b_,
                                 button_color_c_, button_color_d_);
    flat_button2_.set_text_color2(button_color_2a_, button_color_2b_,
                                  button_color_2c_, button_color_2d_);
    flat_button2_.set_text_color3(button_color_3a_, button_color_3b_,
                                  button_color_3c_, button_color_3d_);
    apply_button_bevel(&flat_button2_);
    reinterpret_cast<VCallSlot001 *>(&flat_button2_)->slot001(0);

pump:
    // THE CONFIG CALLBACK, once per caption block, only when the popup has
    // neither the lock bit nor a callback-free config.
    if ((field_30A8_ & 0x400000) == 0 && field_3094_ != 0) {
        field_3094_(text_block_a_.text, 1);
    }
    hide();

    if ((field_30A8_ & 0x4100) == 0 && owner_window_ == this) {
        // THE MODAL PUMP, with the popup holding modal focus. The fallout
        // gate nests: a popup inside a popup leaves the outer one's count
        // standing, and the flag only clears when the last one leaves.
        ++BasePopFalloutGate;
        Win *const saved_focus = WinModalFocus;
        WinModalFocus = this;

        // THE MODAL PUMP, Win::set_modal at slot 0xe4. The receiver is the
        // external window when one is attached and the popup itself when not.
        const int show_bit = (static_cast<int>(field_30A8_) >> 0x15) & 1;
        if (external_window_ != 0) {
            pump_result =
                external_window_->set_modal(show_bit, callback,
                                            static_cast<Win *>(0));
        } else {
            pump_result = set_modal(show_bit, callback, static_cast<Win *>(0));
        }

        WinModalFocus = saved_focus;
        BasePopModalLeft = client_rect_.left;
        BasePopModalTop = client_rect_.top;

        if ((field_30A8_ & 0x4100) == 0 && owner_window_ == this) {
            if (--BasePopFalloutGate == 0) {
                BasePopFalloutFlag = 0;
            }
            hide();
            if ((field_30A8_ & 0x400000) == 0 && field_3094_ != 0) {
                field_3094_(text_block_b_.text, 0);
            }
        }
    }

    if (dialogs_.kind_ == 4) {
        // THE TEXT-FIELD KIND: all eight fields go to the parser.
        int index = 0;
        for (; index < 8; index++) {
            parse_says(index, dialogs_.get_text(index), -1, -1);
        }
    }

    // THE ANSWER: what the pump said, or the selected item.
    if (pump_result != 0) {
        if ((field_30A8_ & 0x40) != 0) {
            return -1;
        }
        return static_cast<int>(BasePopDefaultExecResult);
    }
    return selected_id_;
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

PROMOTED FROM src/unrecovered/006018a0.cpp, which reached the default-ok
buffer through a fixed-address binding where this tree now reads
BasePopDefaultOkText.

THE COPY IS `dst[0] = 0` FOLLOWED BY A STRCAT, which is what the image does
everywhere it copies a string - two arguments and `add esp, 8`, not the
bounded three.
*/
inline int __cdecl BasePop::set_def_ok_text(LPSTR text) {
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
inline int __cdecl BasePop::set_def_cancel_text(LPSTR text) {
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
Purpose: Validate a start caption and install it as the popup's default.
// ORIGINAL: 0x006043D0 ?set_def_file@BasePop@@QAAHPAD@Z 0x006043D0-0x00604407 FILE BYTE_EXACT
// symbol    ?set_def_file@BasePop@@SAHPAD@Z
// size      55 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645470
// LEVER: real strlen and strcat WITHOUT the pragma. The artefact reached
//         this pair with `#pragma function(strlen, strcat)`, which is not
//         available to the tree. The same two calls fall out of the ordinary
//         source form its twin set_def_ok_text already uses - a
//         `dst[0] = 0` store followed by `strcat(dst, src)` - because both
//         strings are arrays of external linkage: PopupStartCaption decays
//         to the `push 0x9b8aa8` the image emits, and `text` is a parameter
//         the compiler cannot see the length of. Nothing about /O2's
//         `repne scasb` expansion survives; measure, not a pragma, is what
//         keeps the two `call rel32`s.
// LEVER: the receiver question, settled by measurement. THREE spellings
//         measured against the image's 20 instructions:
//           `int __cdecl BasePop::set_def_file(char *)`  19/20 - string lands
//               at [ebp+0xc], the SECOND stack slot, so the null test
//               compares the wrong slot and the tail `ret` gains a `4`.
//               MSVC pushes a __cdecl member's `this` FIRST, not last.
//           `static int BasePop::set_def_file(char *)`   20/20 BYTE_EXACT
//           a free `int set_def_file(char *)`    same 20 bytes, wrong symbol
//         What decides it is the CALLER, not the body: config_popups
//         (0x00588AEB) pushes exactly ONE argument before the
//         `call 0x6043d0` and the body ends in `ret` rather than `ret 4`,
//         so there is one stack slot and no receiver. The artefact's
//         reading - that the string is `this` itself - was an attempt to
//         reconcile the QAA with that one slot; `static` reconciles it
//         without the pun the class rules ban.
*/
int BasePop::set_def_file(char *text) {
    if (text != nullptr && strlen(text) <= 0xC) {
        PopupStartCaption[0] = 0;
        strcat(PopupStartCaption, text);
        return 0;
    }
    return 3;
}

/*
// ORIGINAL: 0x00604590 ?init_class@BasePop@@QAAHXZ 0x00604590-0x0060467A BYTE_EXACT
// LEVER: inline-the-two-setters the body is `set_def_cancel_text("Cancel");` then `set_def_ok_text("OK");` INLINED, not open-coded. Each inlined copy keeps its own `if (text == nullptr) return 3;` and `return 5`/`return 4` failure exits, which is why the image has `mov eax, <caption>; test eax, eax; je` before each block and why both failure paths jump to the START OF THE NEXT BLOCK (0x006045F8, 0x00604650) rather than to a return. The two setters are `__forceinline` here and MUST be defined ABOVE this body: VC6 is single-pass and will not inline a definition it has not seen. Both keep their own out-of-line BYTE_EXACT bodies, which /Gy still emits.
// LEVER: string-literal-not-address the captions are the LITERALS "Cancel" (0x00697008) and "OK" (0x00697010), read out of the image. Spelled as `(LPSTR)0x00697008` VC6 folds `text == nullptr` to false at compile time and the whole `mov/test/je` triple disappears; a literal's address is a RELOCATION, which VC6 cannot fold and the comparison masks away.
// LEVER (2026-09-03, pair normalisation): `inline`, not `__forceinline` - the
//   bare out-of-line definitions the setter-side fixes left here made /Ob1
//   emit two real `call`s where the image expands both, 21 compiled
//   instructions against the image's 66. Plain `inline` on the two definitions
//   above restores the expansion AND still reaches both setters' own
//   BYTE_EXACT claims, but only once basepop.h declares them MEASURED: with
//   every use in THIS unit folded, VC6 drops the two COMDATs from basepop.obj
//   entirely ("the fact is stale"), and the dllexport is what forces them
//   back. game.cpp's four callers never see the definition, so they keep the
//   image's real `call`s.
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
// two fixed-address bindings the artifact carried - the same
// `mov dword ptr [addr], eax` folded shape, claim re-measured BYTE_EXACT
// after, and `pops` (popup.cpp) now reads the same storage.
Status: Complete
*/
int __cdecl BasePop::init_class() {
    BasePopDefaultStringFonts[0] = FontDefault;
    BasePopDefaultButtonFonts[0] = FontDefault;

    set_def_cancel_text("Cancel");
    set_def_ok_text("OK");

    PopupInstanceSlotA = PopupAllocHook();
    if (PopupInstanceSlotA == 0) {
        return 4;
    }
    PopupInstanceSlotB = PopupAllocHook();
    return PopupInstanceSlotB ? 0 : 4;
}
/*
Purpose: Destroy the popup infrastructure: close the window, then the
         members in reverse declaration order - Spot at +0x3098, the Dialogs
         slab at +0x21d0, unit_ids_ and item_ids_, sprite_, the two
         FlatButtons, heap_ - and finally the GraphicWin base.
// ORIGINAL: 0x004064D0 ??1BasePop@@QAE@XZ 0x004064D0-0x0040666F;0x006508D0-0x00650978
// body      src/basepop.h
// symbol    ??1BasePop@@UAE@XZ
//           (the class's deleting destructor ??_GBasePop@@UAEPAXI@Z and its
//           0x444 adjustor are in the image too, so the complete destructor
//           is emitted `U` - the same rule as flatbutton.h's `MEASURED
//           ~FlatButton`, whose marker records the same fact)
// size      583 bytes
// prototype void (__thiscall ??1BasePop@@QAE@XZ)(BasePop* this)
// callers   75   call targets   11
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401060 0x00402970 0x00406910 0x005D45B0 0x005D4DD0 0x005E3820
//           0x005FA870 0x00600F00 0x00607040 0x00607DA0 0x00608E10
// MOVED IN-CLASS 2026-09-03 (body: src/basepop.h) - the image CALLS this
// destructor out of line 75 times and INLINES it at the popup's other
// destruction sites, and only an in-class definition lets both spellings
// come out of one source. The expansion is what Caviar::init_class
// (0x006185A0) lays down at each of its early returns and what
// sound_version_warning (0x004C9080) lays down at its one scope exit; out
// of line, those sites can only emit `call ??1BasePop`.
//
// The body is `close()` and nothing else. The rest of the image's body - the
// two vtable installs, the member teardowns in exactly reverse declaration
// order, and the closing ??1GraphicWin - is the compiler's, generated from
// the member declarations in basepop.h, the same way ??1DipEdit is generated
// from dipedit.h. The one thing the member list cannot produce is the image's
// opening `call ?close@BasePop`, which the body spells.
//
// LEVER: in-class `MEASURED ~BasePop() { close(); }`, the ~FlatButton move.
//   The image's 75 out-of-line calls are the catalogue's own count; the
//   inlined copies are extra. What moved: Caviar::init_class (0x006185A0)
//   now reaches 27/392 on `/O2 /Oi-` and 28/392 on `/O2 /G6`, where before
//   the move NO set in the ladder exceeded 16 - and its compiled body is
//   475 instructions against the image's 392 instead of 192, so the ~200
//   instructions the image spends on its per-path teardown chains are now
//   THERE and divergent rather than absent. The tool's ranked winner is
//   still 16/392 on the /Ob0 set (similarity ranks first, and the
//   `call ??1BasePop` shape is closer), so that claim's own verdict does
//   not move. sound_version_warning (0x004C9080) 3 -> 8 of 122, compiled
//   36 -> 84. Nothing regressed: ??_GBasePop 0x00406B40 stays BYTE_EXACT
//   14/14, and `osmx check` holds at 0 REGRESSED, 2,037 verified.
// TRIED: dropping `MEASURED` (dllexport) to buy the inlining - no effect.
//   /O2 /Ob2 compiles caviar.cpp to the same 5 expansions of this destructor
//   with it and without it; what decides whether a site expands is /Ob, not
//   the storage class. The declaration-only `~BasePop();` plus forwarder is
//   the spelling that loses: it is why no site could expand.
// CEILING: the image INLINES the member teardowns one level further - its
//   body is 95 instructions against our 61, because its ??1BasePop expands
//   ~StringList (remove_all + close, twice), ~Dialogs (??1Dialogs + ??1Dialog
//   + ??1GraphicWin) and both ~FlatButtons where this compiler emits
//   ??_DStringList / ??_DDialogs / ??1FlatButton calls. VC6 will not inline
//   a destructor into a destructor it is itself generating, so no flag set
//   in the ladder reaches the image's shape. Second ceiling: with /GX this
//   destructor opens `mov eax, handler; call __EH_prolog` where the image
//   opens the explicit `push ebp` frame - the same wall flatbutton.h's
//   SEH-frame note records for the FlatButton constructor.
*/

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
// STRUCTURE BasePop ctor | the 27 dwords at 0x31b8 are a COPY of
//        BasePopDefaultBlock31B8, not a transpose | image loads
//        0x9b8d10+4*column+0xC*row and stores 0x31b8+4*column+0xC*row - the
//        SAME index on both sides, nine rows of three dwords, three turns.
//        The spelling this replaces walked row*3+column against column*8+row
//        over EIGHT rows: 24 of the 27 slots, scrambled, one never written -
//        a divergence from the shipped code, not a spelling problem. Fixed
//        2026-09-04: dst[0/3/6/.../24] = table[column + 0/3/.../24], one turn
//        per column, which is what took this body 41 -> 105 of 321 together
//        with the memset below.
// LEVER: eight-dword-block the eight contiguous dwords at 0x20f8 are ONE
//        `rep stosd` in the image (`lea edi,[ebp+0x20f8]; mov ecx,8; xor
//        eax,eax`), so the body says `memset(zero_block_, 0, 8 *
//        sizeof(uint32_t))`. Eight separate `= 0` member stores compile to
//        eight movs and cost the block. Measured 46/321 vs 41/321 alone,
//        105/321 with the copy above.
// TRIED: register-colouring the residual is still the swap, and the four
//        callee-saved registers hold the SAME FOUR VALUES in both listings in
//        the SAME first-use order - this (prologue), 0 (the SEH state store),
//        the StringList vftable pointer, the staging cursor. Image: ebp=this,
//        ebx=0, esi, edi. Ours: ebx=this, ebp=0, esi, edi. The frames are
//        byte-identical (`sub esp, 8`, two dead home stores, state byte at
//        [esp+0x20]), so the swap is the allocator's register CHOICE alone.
//        Census: 27 image bodies keep `this` in ebp and 137 in ebx; the 27
//        are every shape but this one is the ONLY constructor among them -
//        every other SEH-framed ctor in the image (55 of them, Dialogs'
//        included) takes `mov esi, ecx`. The four byte-exact ebp bodies
//        (Caviar::close, EditGroup::close, PullDown::UNK4, rename_item) all
//        park an early CONSTANT in ebx and `this` in ebp; ours parks `this`
//        in ebx and the constant in ebp, i.e. exactly crossed.
// TRIED: zero-as-named-local `const int zero_ = 0;` used for 27 of the
//        uint32_t zero stores, to give the constant a source-level symbol and
//        a memory home ahead of `this` - measured 105/321, IDENTICAL to the
//        tree, no flip (a wrong order costs instead: hoisting
//        `field_21C8_ = 0` to the body's first statement measures 35/321).
// TRIED: transpose-mirror the copy loop spelled with a walking source pointer
//        and a fixed destination, to see whether the allocator's choice of
//        which pointer walks feeds the colouring - it does not; and the image
//        walks the DESTINATION (`add ecx,4`, source as counter+displacement)
//        where this body walks the source and displaces the destination
//        (`mov eax,0xc` against the image's `xor eax,eax`). Two instructions
//        of residual, invisible while the colouring stands.
// LEVER: field-typing field_A3C_ became `void *field_A3C_` (basepop.h) and
//        the store became `owner_window_ = this;` - the value is proved (image
//        `mov [ebp+0xa3c], ebp`, 0x00600ab0) even though no code in the tree
//        ever reads the slot back, and the bytes do not move: 105 of 321
//        before and after. The NAME stays neutral on purpose; the earlier
//        `self_` asserted a role nobody has evidenced.
// LEVER: member-recovery four more members named from their own use, all
//        byte-neutral (34 BYTE_EXACT claims in this file held): field_A18_ /
//        field_A1C_ -> default_loc_a_ / default_loc_b_ (the ctor's tail
//        copies them over loc_a_/loc_b_ unless they carry the 0x2000
//        keep-current sentinel set_loc tests), and field_A20_ / field_A24_ ->
//        dialog_width_ / dialog_height_ (the file-backed {100, 100} table the
//        tail writes into the embedded Dialog's width/height slots). Renames
//        of members are value-neutral and change no mangled name. Still
//        offset-named: the dwords of 0x0a40..0x20f4 and 0x2144..0x21cc whose
//        owning type nobody has read off the image.
// LEVER: run-typing 2026-09-04, byte-neutral (105 of 321 before and after;
//        the 34 BYTE_EXACT claims in this file held). Three runs came out of
//        the anonymous dwords:
//        * the two 0x118 blocks at 0x2e64 / 0x2f7c -> BasePopTextBlock
//          text_block_a_ / text_block_b_ (basepop.h). NOT "ten DialogEntry
//          records each", which the note this replaces asserted: the only
//          bodies that reach the region, start (0x00601bf0) and close
//          (0x00600f00), walk them with `idx * 0x118` and strcat into block
//          +0 - an inline char buffer of 0x104, with five numbers at +0x104
//          ..+0x114. A DialogEntry head is a vtable pointer and would fault
//          as a strcat destination. 0x118 / 0x1C = 10 was arithmetic, not
//          evidence.
//        * the 27 dwords at 0x31b8 -> default_block_[27] (nine rows of three,
//          0x6C, ending at ok_text_).
//        * the eight dwords at 0x20f8 -> zero_block_[8], the ctor's one
//          `rep stosd`.
//        All three are single-writer runs; nothing in the tree reads any of
//        them back, so the fields inside stay offset-named rather than
//        invented.
// TRIED: dialogs-tail the image's tail (~30 instructions, 0x00600e8c..) walks//        the Dialogs slab by raw offset - `mov ecx,[ebp+0x21d0]; mov
//        edx,[ecx+8]; mov [edx+ebp+0x21fc],eax` - and Dialogs declares NO own
//        data (0xC94 is five base subobjects), so there is no member to reach
//        it through and a fresh raw self-access is ratchet-blocked at the
//        ceiling. Absent on purpose, as the note above says; it is also ~30
//        more `this`-relative uses, so it may be the colouring's trigger.
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

// BasePopVbtable and BasePopStageTables6693A0/6698C0 are gone: they were the
// tree's own copies of StringList's vbtable ({-4, +0x24}) and the two staged
// table pairs, hand-consumed by ProdPicker's block helper until the three
// blocks were identified as StringLists (prodpicker.h) and their
// construction moved to the compiler. item_ids_/unit_ids_ above were already
// constructed the same way.

// --- .data: the class-default slots the ctor loads ---

// The class-defaults table is FILE-BACKED in the image (0x00696EC4 sits below
// the 0x006A8000 end of .data's file bytes), so these carry the shipped
// initial values. popups_normal (0x00588460) rewrites parts of the table at
// run time before the first popup is built; the values here are the load
// state. Everything below this table - all the 0x009B8xxx slots - is past
// that end: zero at load, and left zero-initialized here on purpose.
//
// WHO FILLS THE 0x009B8xxx SLOTS (image .text scan for the raw address, done
// 2026-09-04). The fonts and colours are written by THIS class's own static
// setters - set_def_string_font / set_def_string_color / set_def_button_font
// / set_def_button_color - which popups_normal calls before the first popup.
// Of the slots with no setter, three are written by OTHER modules' init
// paths: 0x009B8CF8 (BasePopDefaultField3094) and 0x009B8AB8
// (BasePopDefaultBlock2E64) by FX::init (0x00445CE8 / 0x00445CFA), and
// 0x009B8DD0 (BasePopDefaultField322C) by Console::init (0x0051D9B5). The
// rest - BasePopDefaultField30A4, BasePopDefaultLayout, BasePopDefaultBlock31B8
// - have no immediate reference anywhere in .text below 0x5C1000, so their
// writer is either a body the tree has not recovered or an indexed store.
// Until it is found, the ctor here copies ZEROS into those fields where the
// shipped game copies whatever its init paths left there.

// {100, 100}: the defaults the ctor's tail sends through dialog_width_/A24_
// into the embedded Dialog's width/height slots. NOT const, because
// set_def_external_window (0x006050F0) writes both slots at run time - the
// file-backed image bytes are the load state, which the initialiser keeps.
uint32_t BasePopDefaultDialogSize[2] = {100, 100};  // 0x00696EC4

// Copied to +0x21c0/+0x21c4. popups_normal rewrites these to 0 and 4.
const uint32_t BasePopDefaultField21C0[2] = {10, 4};  // 0x00696ED0

// Copied to +0x3104/+0x3108/+0x310c.
const uint32_t BasePopDefaultField3104[3] = {1, 1, 255};  // 0x00696ED8

// Copied to +0x31a0 (its low byte)/+0x31a4/+0x31a8/+0x31ac. popups_normal
// rewrites the first byte; the shipped byte is 0xf7.
const uint32_t BasePopDefaultField31A0[4] = {247, 7, 248, 2};  // 0x00696F54

// The two 0x118-byte default blocks the ctor copies to +0x2e64 and +0x2f7c,
// into text_block_a_ / text_block_b_. All zero until popups_normal and the
// set_def_* setters fill them; BasePop::close (0x00600f00) restores them
// from here 70 dwords at a time.
uint32_t BasePopDefaultBlock2E64[70];  // 0x009B8AB8
uint32_t BasePopDefaultBlock2F7C[70];  // 0x009B8BD0

// Copied to +0x3094, the head of the object's third block-sized region.
uint32_t BasePopDefaultField3094;  // 0x009B8CF8

// The dword immediately below the class-defaults table, file-backed and
// shipped as -1. See the note beside the extern in basepop.h.
const uint32_t BasePopDefaultExecResult = 0xFFFFFFFF;  // 0x00696EC0

// Where the last modal popup's client area started. See basepop.h.
int BasePopModalLeft;  // 0x009B8DC4
int BasePopModalTop;   // 0x009B8DC8

// THE LAST POPUP'S SCREEN EXTENT, the three slots immediately below
// BasePopDefaultField3094. Named 2026-09-05 from the three bodies that touch
// them - every reference in the image's .text is one of these, and nothing
// outside BasePop reads either:
//
//   * BasePop::on_redraw (0x00604AB0) and BasePop::calculate_dimensions
//     (0x00603800) PUBLISH the popup's bottom (0x009B8CE8) and right
//     (0x009B8CEC) edge: the same sum of the popup's border (field_30E0_),
//     client offsets (field_30F0_/field_30F4_), the external-window position
//     (field_30B4_/field_30B8_) and the text height (field_30DC_), clamped
//     against each other.
//   * BasePop::exec (0x00602600) reads both back as the base it positions the
//     next popup from - 0x009B8CE8 plus field_30E4_ for the vertical, and
//     0x009B8CEC as the fallback when field_21CC_ is still -1 for the
//     horizontal - which is what makes these "the last one drawn" rather than
//     "this one".
//
//   * 0x009B8CF4 holds the popup whose picture is being drawn: on_redraw
//     stores `this` into it around the caller-sprite draw and writes it back
//     to zero on the way out. Both writes are in on_redraw and nothing reads
//     it, so the name says what it holds and no more.
uint32_t BasePopDrawnBottom;  // 0x009B8CE8
uint32_t BasePopDrawnRight;   // 0x009B8CEC
BasePop *BasePopDrawnPopup;   // 0x009B8CF4

// The defaults for external_window_ (the high-res-scaling opt-out set_width tests)
// and default_loc_a_/default_loc_b_ (the starting loc_a_/loc_b_; 0x2000 keeps the
// current value, so zero here means "location 0,0").
uint32_t BasePopDefaultLayout[3];  // 0x009B8D04

// ORIGINAL: 0x006050F0 ?set_def_external_window@BasePop@@QAAXPAUGraphicWin@@HHHH@Z 0x006050F0-0x00605121 FILE BYTE_EXACT
// symbol    ?set_def_external_window@BasePop@@SAXPAVGraphicWin@@HHHH@Z
// size      49 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: five plain stores in argument order. The image's own instruction
//         order is the register shuffle VC6 produces for them - it loads
//         [esp+4]/[esp+8]/[esp+0xc] into eax/ecx/edx BEFORE the first store,
//         and [esp+0x10]/[esp+0x14] into the two registers just freed, so the
//         six loads and five stores interleave exactly as written. No source
//         reordering is needed; the naive body is the match.
// LEVER: `static` (the `// symbol` fact above records the SAH this tree
//         emits for the image's QAAX) - the image reads five arguments at
//         [esp+4]..[esp+0x14] and there is no sixth slot for a hidden `this`.
void BasePop::set_def_external_window(GraphicWin *external_window, int loc_x,
                                      int loc_y, int width, int height) {
    BasePopDefaultLayout[0] = reinterpret_cast<uint32_t>(external_window);
    BasePopDefaultLayout[1] = loc_x;
    BasePopDefaultLayout[2] = loc_y;
    BasePopDefaultDialogSize[0] = width;
    BasePopDefaultDialogSize[1] = height;
}

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

BasePop::BasePop() {

    // GraphicWin(), heap_(), flat_button1_(), flat_button2_(), sprite_(),
    // item_ids_() and unit_ids_() run via ordinary member construction -
    // their declaration order matches the disassembly's construction order.

    // item_ids_'s construction, like unit_ids_'s, is the compiler's now.

    // unit_ids_'s construction (vbtable, staged vtables, allocator
    // hand-off, StringStruct zeroing) is the compiler's now, emitted from
    // the declared StringList member - these were its hand-spelled copy.


    field_30A4_ = static_cast<int>(BasePopDefaultField30A4);
    field_3094_ = reinterpret_cast<int (__cdecl *)(char *, int)>(
        BasePopDefaultField3094);

    memcpy(&text_block_a_, BasePopDefaultBlock2E64, 0x118);
    memcpy(&text_block_b_, BasePopDefaultBlock2F7C, 0x118);
    {
        // A STRAIGHT COPY OF ALL 27 DWORDS, not a transpose. The image's loop
        // loads 0x9b8d10 + 4*column + 0xC*row and stores 0x31b8 + 4*column +
        // 0xC*row - the SAME index on both sides, nine rows of three dwords,
        // one turn per column. The earlier spelling here walked row*3+column
        // against column*8+row over eight rows: twenty-four of the twenty-
        // seven slots, scrambled, and the last one never written.
        uint32_t *dst = default_block_;
        int column = 0;
        do {
            dst[0] = BasePopDefaultBlock31B8[column];
            dst[3] = BasePopDefaultBlock31B8[column + 3];
            dst[6] = BasePopDefaultBlock31B8[column + 6];
            dst[9] = BasePopDefaultBlock31B8[column + 9];
            dst[12] = BasePopDefaultBlock31B8[column + 12];
            dst[15] = BasePopDefaultBlock31B8[column + 15];
            dst[18] = BasePopDefaultBlock31B8[column + 18];
            dst[21] = BasePopDefaultBlock31B8[column + 21];
            dst[24] = BasePopDefaultBlock31B8[column + 24];
            dst += 1;
            column += 1;
        } while (column < 3);
    }

    field_21C8_ = 0;
    field_21CC_ = -1;
    ok_text_ = 0;
    cancel_text_ = 0;
    owner_window_ = this;
    field_20F4_ = 0;
    selected_id_ = 0;
    field_30A8_ = 0;
    field_30AC_ = 0;
    value_ = -1;
    // Eight contiguous dwords: the image fills them with one `rep stosd`.
    memset(zero_block_, 0, 8 * sizeof(uint32_t));
    owned_text_ = 0;
    field_21B8_ = BasePopDefaultField21B0[3];
    field_21BC_ = BasePopDefaultField21B0[2];
    field_21B0_ = BasePopDefaultField21B0[0];
    field_21B4_ = BasePopDefaultField21B0[1];
    field_21C0_ = BasePopDefaultField21C0[0];
    field_21C4_ = BasePopDefaultField21C0[1];

    sprite_.close();

    caller_sprite_ = 0;
    field_214C_ = reinterpret_cast<Sprite *>(BasePopDefaultField2148[1]);
    field_2148_ = reinterpret_cast<Sprite *>(BasePopDefaultField2148[0]);
    field_30B0_ = 0;
    field_30B4_ = 0;
    field_30B8_ = 0;
    field_30BC_ = 0;
    field_30C0_ = 0;
    field_30C4_ = 0;
    field_30C8_ = 0;
    field_30CC_ = 0;
    field_30D0_ = 0;
    field_30D4_ = 0;
    field_30D8_ = 0;
    field_30DC_ = 0;
    field_30E0_ = 0;
    field_30E4_ = 0;
    field_30E8_ = 0;
    field_30EC_ = 0;
    field_30F0_ = 0;
    field_30F4_ = 0;
    loc_a_ = 0x1000;
    loc_b_ = 0x1000;
    field_3100_ = 0;
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
    field_3160_ = 0;
    sprite_double_click_callback_ = 0;
    redraw_callback_ = 0;
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
    field_322C_ = 0;
    field_322C_ = reinterpret_cast<Win *>(BasePopDefaultField322C);
    external_window_ = (Win *)BasePopDefaultLayout[0];
    default_loc_a_ = BasePopDefaultLayout[1];
    default_loc_b_ = BasePopDefaultLayout[2];
    dialog_width_ = BasePopDefaultDialogSize[0];
    dialog_height_ = BasePopDefaultDialogSize[1];

    if (external_window_ != 0) {
        // The image's tail here walks the Dialogs subobject at +0x21d0 (see
        // the STRUCTURE note under the marker): [dialogs+8] selects two
        // slots that receive dialog_width_/dialog_height_ (the latter grown 3/2 for
        // wide screens), and default_loc_a_/default_loc_b_ overwrite loc_a_/loc_b_
        // unless they equal 0x2000. Not spelled: this body neither constructs
        // the Dialogs subobject nor may take a fresh raw pointer to it (the
        // raw self-access ratchet sits at its ceiling), so the reads would
        // consume garbage. Absent on purpose.
        if (default_loc_a_ != 0x2000) {
            loc_a_ = default_loc_a_;
        }
        if (default_loc_b_ != 0x2000) {
            loc_b_ = default_loc_b_;
        }
    }

}

// Fixed-slot bindings carried from 00600f00.cpp

// ORIGINAL: 0x00600F00 ?close@BasePop@@QAEXXZ 0x00600F00-0x006015A9 FILE
// TRIED: calling StringStruct::remove_all() for the two entry walks (the
//        function the image inlines here - same guard, same do/while, same
//        three tail stores) instead of the artifact's hand-spelled walks.
//        Honest, and it crosses VC6's FPO threshold: the body takes a
//        frame pointer under BOTH flag sets and scores 0 of 388 where the
//        hand walks scored 12 of 388. The cure is the original's own - a
//        definition of remove_all visible in this TU, which is a
//        stringstruct.h handoff (MEASURED inline there, like port_to_port
//        and has_tech) - not a rewrite of this body.
// TRIED: no Ghidra hint; straight offset-based transcription (class is effectively opaque past GraphicWin) of ~150 field resets, two `rep movsd` block copies, two `free()` guards, a 3x9 transposed table copy, five sub-object close() calls plus GraphicWin's own, a Sprite close, two identically-shaped "clear a small vector of polymorphic items" loops (multi-inheritance adjustor thunks via pointer-to-member `original_slot`/`ORIGINAL`, not the VCall shim class - the receiver for the inner dispatch is a computed sub-object address, not `this` itself), an 8-iteration destroy-and-null loop, and the closing field batch + Heap destructor. Compiles; diverges at instruction #0 (mov vs push - the local frame shape differs immediately given how many more locals this body declares than the original's bare `push ecx`).
// working copy - scaffold materialised by --work
// size      1705 bytes
// prototype void (__thiscall ?close@BasePop@@QAEXXZ)(BasePop* this)
// callers   100   call targets   9
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4580 0x005D4E40 0x005E3820 0x00609F20 0x0060D1B0 0x0060E7C0 0x00610280 0x00611A90 0x00644EF2
// indirect  0x00601070 0x00601082 0x006010B4 0x006010C7 0x006010E3 0x00601133 0x00601148 0x00601163 0x0060119C

// The two vtable slots close() dispatches through: slot 0x168 of each
// embedded FlatButton's table (a nullary redraw request) and slot 0 of the
// objects parked in zero_block_ (their one-int destructor, the deleting
// slot). Neither receiver's type is declarable - the first is a FlatButton
// whose table belongs to a class this tree keeps non-virtual, the second is
// whatever the popup stored there - so the slots are read as they live.
typedef void (__fastcall *func_basepop_slot090)(void *);
typedef void (OriginalObject::*func_basepop_list_destroy_dtor)(int);

void BasePop::close() {
    char *self = (char *)this;
    int i, off;

    *(int *)(self + 0x30a4) = BasePopDefaultField30A4;
    *(int *)(self + 0x3094) = BasePopDefaultField3094;

    for (i = 0; i < 0x46; i++) {
        *(int *)(self + 0x2e64 + i * 4) = (int)BasePopDefaultBlock2E64[i / 4];
    }
    for (i = 0; i < 0x46; i++) {
        *(int *)(self + 0x2f7c + i * 4) = (int)BasePopDefaultBlock2F7C[i / 4];
    }

    if (*(void **)(self + 0x3224) != 0) {
        free(*(void **)(self + 0x3224));
        *(void **)(self + 0x3224) = 0;
    }
    if (*(void **)(self + 0x3228) != 0) {
        free(*(void **)(self + 0x3228));
        *(void **)(self + 0x3228) = 0;
    }

    *(void **)(self + 0xa3c) = this;

    for (i = 0, off = 0x31c4; i < 0xc; i += 4, off += 4) {
        *(int *)(self + off - 0xc) = BasePopDefaultBlock31B8[i / 4];
        *(int *)(self + off) = BasePopDefaultBlock31B8[i / 4 + 2];
        *(int *)(self + off + 0xc) = BasePopDefaultBlock31B8[i / 4 + 5];
        *(int *)(self + off + 0x18) = BasePopDefaultBlock31B8[i / 4 + 8];
        *(int *)(self + off + 0x24) = BasePopDefaultBlock31B8[i / 4 + 11];
        *(int *)(self + off + 0x30) = BasePopDefaultBlock31B8[i / 4 + 14];
        *(int *)(self + off + 0x3c) = BasePopDefaultBlock31B8[i / 4 + 17];
        *(int *)(self + off + 0x48) = BasePopDefaultBlock31B8[i / 4 + 20];
        *(int *)(self + off + 0x54) = BasePopDefaultBlock31B8[i / 4 + 23];
    }

    ((ListBox *)(self + 0x21d0))->close();
    ((CheckBox *)(self + 0x2228))->close();
    ((RadioButton *)(self + 0x2214))->close();
    ((SpriteBox *)(self + 0x2240))->close();
    ((EditGroup *)(self + 0x22c8))->close();
    ((GraphicWin *)self)->close();

    ((Sprite *)(self + 0x2118))->close();
    *(int *)(self + 0x20f4) = 0;
    *(Sprite **)(self + 0x2144) = 0;
    *(int *)(self + 0xa40) = 0;
    *(int *)(self + 0xa44) = -1;

    if (owned_text_ != 0) {
        free(owned_text_);
        owned_text_ = 0;
    }
    *(int *)(self + 0x21c8) = 0;
    *(int *)(self + 0x21cc) = -1;

    // SLOT 0x168 OF EACH EMBEDDED FLAT BUTTON'S TABLE, called where it lives.
    // 0xa5c and 0x15a8 are flat_button1_ and flat_button2_.
    vtable_slot<func_basepop_slot090>(&flat_button1_, 0x168)(&flat_button1_);
    vtable_slot<func_basepop_slot090>(&flat_button2_, 0x168)(&flat_button2_);

    // THE TWO LIST TEARDOWNS. The image inlines StringStruct::remove_all at
    // both - same guard on head_, same do/while over entry_count_, same three
    // tail stores - so the source is the call, and the whole entry walk with
    // its two virtual dispatches is that one function's body. Inlining it
    // here needs the definition visible in this TU (stringstruct.cpp holds
    // it), which is a handoff, not a rewrite.
    if (unit_ids_.head_ != 0) {
        unit_ids_.remove_all();
    }
    unit_ids_.current_position_ = 0;

    if (item_ids_.head_ != 0) {
        item_ids_.remove_all();
    }
    item_ids_.current_position_ = 0;


    for (i = 0; i < 8; i++) {
        int item = zero_block_[i];
        if (item != 0) {
            OriginalObject *entry = reinterpret_cast<OriginalObject *>(item);
            (entry->*original_method<func_basepop_list_destroy_dtor>(
                *(unsigned long *)item))(1);
            zero_block_[i] = 0;
        }
    }

    *(int *)(self + 0x21b8) = BasePopDefaultField21B0[3];
    *(int *)(self + 0x21bc) = BasePopDefaultField21B0[2];
    *(int *)(self + 0x21b0) = BasePopDefaultField21B0[0];
    *(int *)(self + 0x21b4) = BasePopDefaultField21B0[1];
    *(int *)(self + 0x21c0) = BasePopDefaultField21C0[0];
    *(int *)(self + 0x21c4) = BasePopDefaultField21C0[1];

    *(int *)(self + 0x30b0) = 0;
    *(int *)(self + 0x30b4) = 0;
    *(int *)(self + 0x30b8) = 0;
    *(int *)(self + 0x30bc) = 0;
    *(int *)(self + 0x30c0) = 0;
    *(int *)(self + 0x30c4) = 0;
    *(int *)(self + 0x30c8) = 0;
    *(int *)(self + 0x30cc) = 0;
    *(int *)(self + 0x30d0) = 0;
    *(int *)(self + 0x30d4) = 0;
    *(int *)(self + 0x30d8) = 0;
    *(int *)(self + 0x30dc) = 0;
    *(int *)(self + 0x30e0) = 0;
    *(int *)(self + 0x30e4) = 0;
    *(int *)(self + 0x30e8) = 0;
    *(int *)(self + 0x30ec) = 0;
    *(int *)(self + 0x30f0) = 0;
    *(int *)(self + 0x30f4) = 0;
    *(int *)(self + 0x30a8) = 0;
    *(int *)(self + 0x30ac) = 0;
    *(int *)(self + 0x30f8) = 0x1000;
    *(int *)(self + 0x30fc) = 0x1000;
    *(int *)(self + 0x3100) = 0;

    *(int *)(self + 0x3104) = BasePopDefaultField3104[0];
    *(int *)(self + 0x3108) = BasePopDefaultField3104[0];
    *(int *)(self + 0x310c) = BasePopDefaultField3104[2];
    *(Font **)(self + 0x3110) = BasePopDefaultStringFonts[0];
    *(int *)(self + 0x3120) = BasePopDefaultStringColors[0][0];
    *(int *)(self + 0x3130) = BasePopDefaultStringColors[1][0];
    *(int *)(self + 0x3140) = BasePopDefaultStringColors[2][0];
    *(int *)(self + 0x3150) = BasePopDefaultStringColors[3][0];
    *(Font **)(self + 0x3114) = BasePopDefaultStringFonts[1];
    *(int *)(self + 0x3124) = BasePopDefaultStringColors[0][1];
    *(int *)(self + 0x3134) = BasePopDefaultStringColors[1][1];
    *(int *)(self + 0x3144) = BasePopDefaultStringColors[2][1];
    *(int *)(self + 0x3154) = BasePopDefaultStringColors[3][1];
    *(Font **)(self + 0x3118) = BasePopDefaultStringFonts[2];
    *(int *)(self + 0x3128) = BasePopDefaultStringColors[0][2];
    *(int *)(self + 0x3138) = BasePopDefaultStringColors[1][2];
    *(int *)(self + 0x3148) = BasePopDefaultStringColors[2][2];
    *(int *)(self + 0x3158) = BasePopDefaultStringColors[3][2];
    *(Font **)(self + 0x311c) = BasePopDefaultStringFonts[3];
    *(int *)(self + 0x312c) = BasePopDefaultStringColors[0][3];
    *(int *)(self + 0x313c) = BasePopDefaultStringColors[1][3];
    *(int *)(self + 0x314c) = BasePopDefaultStringColors[2][3];
    *(int *)(self + 0x315c) = BasePopDefaultStringColors[3][3];
    *(int *)(self + 0x3160) = 0;
    *(int *)(self + 0x3164) = 0;
    *(int *)(self + 0x3168) = 0;
    *(Font **)(self + 0x316c) = BasePopDefaultButtonFonts[0];

    *(unsigned char *)(self + 0x3178) = (uint8_t)BasePopDefaultButtonColors[0][0];
    *(int *)(self + 0x317c) = BasePopDefaultButtonColors[1][0];
    *(int *)(self + 0x3188) = BasePopDefaultButtonColors[2][0];
    *(int *)(self + 0x3194) = BasePopDefaultButtonColors[3][0];
    *(Font **)(self + 0x3170) = BasePopDefaultButtonFonts[1];
    *(unsigned char *)(self + 0x3179) = (uint8_t)BasePopDefaultButtonColors[0][1];
    *(int *)(self + 0x3180) = BasePopDefaultButtonColors[1][1];
    *(int *)(self + 0x318c) = BasePopDefaultButtonColors[2][1];
    *(int *)(self + 0x3198) = BasePopDefaultButtonColors[3][1];
    *(Font **)(self + 0x3174) = BasePopDefaultButtonFonts[2];
    *(unsigned char *)(self + 0x317a) = (uint8_t)BasePopDefaultButtonColors[0][2];
    *(int *)(self + 0x3184) = BasePopDefaultButtonColors[1][2];
    *(int *)(self + 0x3190) = BasePopDefaultButtonColors[2][2];
    *(int *)(self + 0x319c) = BasePopDefaultButtonColors[3][2];
    *(int *)(self + 0x31a4) = BasePopDefaultField31A0[1];
    *(int *)(self + 0x31a8) = BasePopDefaultField31A0[2];
    *(int *)(self + 0x31ac) = BasePopDefaultField31A0[3];
    *(int *)(self + 0x31b0) = BasePopDefaultField31B0[0];
    *(int *)(self + 0x31b4) = BasePopDefaultField31B0[1];
    *(int *)(self + 0x214c) = BasePopDefaultField2148[1];
    *(int *)(self + 0x2148) = BasePopDefaultField2148[0];
    *(int *)(self + 0x322c) = BasePopDefaultField322C;
    *(unsigned char *)(self + 0x31a0) = (uint8_t)BasePopDefaultField31A0[0];

    *(int *)(self + 0xa14) = BasePopDefaultLayout[0];
    *(int *)(self + 0xa18) = BasePopDefaultLayout[1];
    *(int *)(self + 0xa1c) = BasePopDefaultLayout[2];
    *(int *)(self + 0xa20) = BasePopDefaultDialogSize[0];
    *(int *)(self + 0xa24) = BasePopDefaultDialogSize[1];

    if (*(void **)(self + 0xa14) != 0) {
        int eax, ecx, edx;

        ecx = *(int *)(self + 0x21d0);
        eax = *(int *)(self + 0xa20);
        edx = *(int *)(ecx + 8);
        *(int *)(self + edx + 0x21fc) = eax;

        ecx = *(int *)(self + 0xa14);
        eax = *(int *)(self + 0xa24);
        if (ecx == 0) {
            edx = *(int *)(self + 0x30a8);
            if ((edx & 0x400) == 0 && WinScreenWidth >= 0x400) {
                eax = (eax * 3) / 2;
            }
        }
        ecx = *(int *)(self + 0x21d0);
        edx = *(int *)(ecx + 8);
        *(int *)(self + edx + 0x2200) = eax;

        eax = *(int *)(self + 0xa18);
        ecx = *(int *)(self + 0xa1c);
        if (eax != 0x2000) {
            *(int *)(self + 0x30f8) = eax;
        }
        if (ecx != 0x2000) {
            *(int *)(self + 0x30fc) = ecx;
        }

        eax = *(int *)(self + 0x21d0);
        ecx = *(int *)(eax + 8);
        {
            int base = ecx + (int)self + 0x21d0;
            int val = *(int *)(self + ecx + 0x21f0);
            val = val | 3;
            *(int *)(base + 0x20) = val;
        }
    }

    ((Heap *)(self + 0xa28))->~Heap();
}

// ORIGINAL: 0x00601980 ?string@BasePop@@QAEHPAD@Z 0x00601980-0x006019EE FILE
// working copy - scaffold materialised by --work
// size      110 bytes
// prototype int (__thiscall ?string@BasePop@@QAEHPAD@Z)(BasePop* this, int8*)
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100

int BasePop::string(char *text) {
    // THE POPUP'S COLOUR-PREFIXED STRING SLOT. `^` opens a colour escape and
    // the byte run that follows says which: one `^` and a code is colour
    // `count` 1, `^^x` is 2, `^^` alone is 3. The parsed pair lands in
    // unit_ids_'s three own fields (StringList field_1C_/field_20_/field_24_,
    // 0x219c/0x21a0/0x21a4) and the entry is appended with id 0.
    int count = 0;
    if (text == 0) {
        return 3;
    }
    char *str = text;
    if (*text == '^') {
        if (text[1] == 0) {
            count = 3;
            str = text + 1;
        } else if (text[1] == '^') {
            count = 2;
            str = (text[2] == '^') ? (text + 3) : (text + 2);
        } else {
            count = 1;
            str = text + 1;
        }
    }
    // TRIED: advancing the parameter in place (`text += 1` / `text += n`)
    //         because the image's tail is one shared `inc eax` plus two
    //         `add eax,N` - it costs the shared increment and scores 3 of 38.
    // The materialised list pointer is the image's own shape here (`add
    // ecx,0x2180` ahead of the three stores and the append), and it scores 18
    // of 38 against 13 for writing the member straight. The residual is the
    // parse: the image shares one `inc eax` for the two single-step paths while
    // this spelling hoists a `lea esi,[eax+1]` above the text[1] test.
    // TRIED: advancing the parameter in place (`text += 1` / `text += n`) to
    //         earn that shared increment - 3 of 38, the shared increment goes
    //         with the separate variable.
    StringList *list = &unit_ids_;
    list->field_1C_ = reinterpret_cast<uint32_t>(str);
    list->field_20_ = count;
    list->field_24_ = 0;
    return list->add(0) != 0;
}

// ORIGINAL: 0x00604410 ?on_key_down@BasePop@@QAEHH@Z 0x00604410-0x0060448E FILE
// TRIED: both int-arithmetic (`off1 + (int)self`) and pointer-arithmetic (`self + off1`) forms for the `[edx+esi]` adjusted base diverge identically at instr #14 (original 'lea' vs rebuilt 'add') - an addressing-mode choice, not a control-flow one. 0x21d0/0x2228/0x2314/0x229c/0x2350 all fall inside the opaque `dialogs_[0xC94]` byte blob, so reached by raw offset.
// working copy - scaffold materialised by --work
// size      126 bytes
// prototype int (__thiscall ?on_key_down@BasePop@@QAEHH@Z)(BasePop* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609A50

int BasePop::on_key_down(int key) {
    if ((field_30A8_ & 0x4000) != 0) {
        return 0;
    }
    int kind = dialogs_.kind_;
    selected_id_ = 0;
    if (kind != -1) {
        // THE DIALOG VIRTUAL BASE, reached through the member with the
        // set_width qualification. +0x229c is Dialog::item_list_.entry_count_
        // (0xbc + 0x10) and +0x2314 is Dialog::selected_position_ (0xec): the
        // image walks the Dialogs vbtable for the first and the CheckBox's for
        // the second, both spelling the same shared base.
        if (dialogs_.Dialog::item_list_.entry_count_ != 0) {
            if (kind == 1) {
                // The image walks the CHECKBOX vbtable (+0x2228) for this read
                // and the Dialogs one (+0x21d0) for entry_count_ above; spelling
                // both through dialogs_.Dialog:: names the same shared base, so
                // VC6 reuses the first displacement and folds this read to
                // `mov eax,[eax+0x22bc]`. TRIED: `CheckBox &box = dialogs_` to
                // force the second walk - VC6 emits its adjustor's null guard
                // for a base-subobject reference (test/je/lea/jmp/xor), 10 of
                // 33. Same wall as the dialogs() accessor note in basepop.h.
                selected_id_ = dialogs_.Dialog::selected_position_;
                return 1;
            }
            selected_id_ = dialogs_.Dialog::get_selected_id();
        }
    }
    return 1;
}

// Fixed-slot bindings carried from 006044d0.cpp


// ORIGINAL: 0x006044D0 ?double_click@BasePop@@QAEXH@Z 0x006044D0-0x00604581 FILE
// symbol    ?double_click@BasePop@@SAXH@Z
// TRIED: early-return on tab==1 (MSVC folds the later redundant tab==1 check into dead code, collapsing 4 original instrs to 1 lea); lea-vs-add on the (off+base) address computation persists regardless of local-vs-inline pointer arithmetic
// working copy - scaffold materialised by --work
// size      177 bytes
// prototype void (__thiscall ?double_click@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609A50
// indirect  0x0060455D 0x00604569 0x0060457B

typedef void (__fastcall *func_basepop_slot058)(void *);
typedef void (__fastcall *func_basepop_slot002)(void *);

void __cdecl BasePop::double_click(int button_id) {
    // THE MODAL-FOCUS POPUP, not this one: the handler runs for whichever
    // window holds modal focus, and every field it touches belongs to that
    // object. A static downcast from Win * is free - BasePop's front is its
    // GraphicWin base, so the two types share an address.
    BasePop *pop = static_cast<BasePop *>(WinModalFocus);
    if (pop != 0) {
        int tab = pop->dialogs_.kind_;
        uint32_t flags = pop->field_30A8_;
        if (tab != 1) {
            if (tab != -1) {
                if (pop->dialogs_.Dialog::item_list_.entry_count_ != 0) {
                    if (tab == 1) {
                        pop->selected_id_ =
                            pop->dialogs_.Dialog::selected_position_;
                    } else {
                        pop->selected_id_ =
                            pop->dialogs_.Dialog::get_selected_id();
                    }
                }
            }
            pop->field_3100_ = 0;
            if ((flags & 0x100) == 0) {
                // The redraw slot runs on the external window when one is
                // attached and on the popup itself when not - vtable slot 0xe8
                // either way.
                if (pop->external_window_ != 0) {
                    vtable_slot<func_basepop_slot058>(pop->external_window_,
                                                      0xe8)(pop->external_window_);
                } else {
                    vtable_slot<func_basepop_slot058>(pop, 0xe8)(pop);
                }
            }
            if ((flags & 0x4000) != 0) {
                vtable_slot<func_basepop_slot002>(pop, 0x8)(pop);
            }
        }
    }
}

// Fixed-slot bindings carried from 00604680.cpp

class VCall1 { public:
    virtual void slot0(int);
};

// ORIGINAL: 0x00604680 ?close_class@BasePop@@QAAXXZ 0x00604680-0x006046ED FILE BYTE_EXACT
// symbol    ?close_class@BasePop@@SAXXZ
// The image spells QAAX - a receiver - while nothing passes one; this tree
// declares the method static, so it emits SAX. Same case as fallout().
// LEVER: the artifact's fixed-address bindings became the real globals -
//         BasePopDefaultOkText, BasePopDefaultStringFonts[0],
//         BasePopDefaultButtonFonts[0], PopupInstanceSlotA/B - and the
//         scheduling came right with them: the null test now sits BEFORE
//         the two default stores, exactly the image's order. A named
//         global is one-storage, so VC6 has no alias to second-guess;
//         through pointer-to-int bindings it hoisted one store above the
//         compare (32 of 34).
// size      109 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x006046CD 0x006046E3
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00604680
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00604680/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close_class@BasePop@@QAAXXZ  at 0x00604680  (109 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void __cdecl BasePop::close_class() {
    char *ok_text = BasePopDefaultOkText;
    BasePopDefaultStringFonts[0] = 0;
    BasePopDefaultButtonFonts[0] = 0;

    if (ok_text != 0) {
        free(reinterpret_cast<void *>(ok_text));
        BasePopDefaultOkText = 0;
    }
    if (BasePopDefaultCancelText != 0) {
        free(reinterpret_cast<void *>(BasePopDefaultCancelText));
        BasePopDefaultCancelText = 0;
    }
    if (PopupInstanceSlotA != 0) {
        reinterpret_cast<VCall1 *>(PopupInstanceSlotA)->slot0(1);
        PopupInstanceSlotA = 0;
    }
    if (PopupInstanceSlotB != 0) {
        reinterpret_cast<VCall1 *>(PopupInstanceSlotB)->slot0(1);
        PopupInstanceSlotB = 0;
    }
}

// ORIGINAL: 0x00605010 ?set_external_window@BasePop@@QAEXPAUGraphicWin@@HHHH@Z 0x00605010-0x006050E7 FILE SEMANTIC
// symbol    ?set_external_window@BasePop@@QAEXPAVGraphicWin@@HHHH@Z
// The image spells GraphicWin a struct (U) where this tree declares a class
// (V); the bytes are identical.
// TRIED: `flags & 0x400` compiles to a single `test [mem], imm`; original keeps the mask 0x400 in a register and does `test eax,edx`. Named local for the flags read didn't change it.
// LEVER: the three Dialog virtual-base stores are the set_width form -
//         `dialogs_.Dialog::field_2C_` etc. Qualified member access walks the
//         vbtable at +0x21d0 (the image's `mov eax,[ecx+0x21d0];
//         mov edx,[eax+8]`) with no pointer to the subobject and no raw
//         self-access; the artifact's hand walk measured 26 of 38.
// LEVER: parameter names are the fields they land in - (external_window,
//         loc_x, loc_y, width, height) from the stores at +0xa14/+0xa18/
//         +0xa1c/+0xa20/+0xa24 and the 0x2000 keep-current tests against
//         loc_a_/loc_b_.
// RESIDUAL (51 of 54, 2026-09-06): the tail `|= 3` read-modify-write. The
//        image reads the style word through the folded form
//        (`mov ecx,[edx+ecx+0x21f0]`) and stores through the base
//        (`lea eax,[edx+ecx+0x21d0]` / `mov [eax+0x20],ecx`); this tree folds
//        the +0x20 into the one address and lands the store at [reg], which
//        is also what moves the closing `je` target one byte. Equivalent
//        program, one addressing split.
// TRIED (2026-09-06): `Dialog &dialog = dialogs_; dialog.field_20_ |= 3;` -
//        the reference makes VC6 re-plan the whole body, 51 of 54 -> 7 of 54
//        with 66 compiled instructions against the image's 54. The qualified
//        member access is what the other three Dialog stores already use and
//        stays.
// size      215 bytes
// prototype void (__thiscall ?set_external_window@BasePop@@QAEXPAUGraphicWin@@HHHH@Z)(BasePop* this, GraphicWin*, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

void BasePop::set_external_window(GraphicWin *external_window, int loc_x,
                                  int loc_y, int width, int height) {
    external_window_ = external_window;
    default_loc_a_ = loc_x;
    default_loc_b_ = loc_y;
    dialog_width_ = width;
    dialog_height_ = height;
    if (external_window != 0) {
        // The three Dialog virtual-base accesses are the set_width form: a
        // qualified member access walks the vbtable at +0x21d0, which is the
        // image's own addressing, and needs no pointer to the subobject.
        dialogs_.Dialog::field_2C_ = width;
        if (external_window_ == 0 && (field_30A8_ & 0x400) == 0 &&
            WinScreenWidth >= 0x400) {
            dialogs_.Dialog::field_30_ = (height * 3) / 2;
        } else {
            dialogs_.Dialog::field_30_ = height;
        }
        if (loc_x != 0x2000) {
            loc_a_ = loc_x;
        }
        if (loc_y != 0x2000) {
            loc_b_ = loc_y;
        }
        dialogs_.Dialog::field_20_ |= 3;
    }
}


// ORIGINAL: 0x006015B0 ?init@BasePop@@QAEHHJ@Z 0x006015B0-0x0060179A FILE
// size      490 bytes
// prototype int (__thiscall ?init@BasePop@@QAEHHJ@Z)(BasePop* this, int, int)
// callers   9   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00402970 0x005D4620 0x005FA8A0 0x00600F00 0x006134A0 0x006136B0
// LEVER: parameter names - `flags` is the word tested against 0x100000 /
//         0x40000 / 0x1000 and whose low byte picks dialogs_.kind_;
//         `heap_base` is the allocation base handed straight to heap_.init.
// LEVER: the tail is the set_width/set_external_window Dialog virtual-base
//         form (qualified member access), not a hand walk.

int BasePop::init(int flags, long heap_base) {
    close();

    if ((flags & 0x100000) == 0) {
        external_window_ = 0;
    }

    // The flag's low byte picks the embedded control the popup opens with:
    // 1, 2 and 4 name a kind outright and anything else is 0x10.
    if (flags & 1) {
        dialogs_.kind_ = 1;
    } else if (flags & 2) {
        dialogs_.kind_ = 2;
    } else if (flags & 4) {
        dialogs_.kind_ = 4;
    } else {
        dialogs_.kind_ = 0x10;
    }

    if (heap_base != 0) {
        if (heap_.init(heap_base)) {
            return 4;
        }
        if ((flags & 0x40000) == 0) {
            unit_ids_.remove_all();
            unit_ids_.current_position_ = 0;
            unit_ids_.allocator_ = &heap_;
            item_ids_.remove_all();
            item_ids_.current_position_ = 0;
            item_ids_.allocator_ = &heap_;
            dialogs_.init(&heap_);
        } else {
            unit_ids_.remove_all();
            unit_ids_.current_position_ = 0;
            unit_ids_.allocator_ = 0;
            item_ids_.remove_all();
            item_ids_.current_position_ = 0;
            item_ids_.allocator_ = 0;
            dialogs_.init(0);
        }
    } else {
        unit_ids_.remove_all();
        unit_ids_.current_position_ = 0;
        unit_ids_.allocator_ = 0;
        item_ids_.remove_all();
        item_ids_.current_position_ = 0;
        item_ids_.allocator_ = 0;
        dialogs_.init(0);
    }

    field_30A8_ = flags;
    if (external_window_ == 0) {
        dialogs_.Dialog::field_20_ &= ~3u;
        loc_a_ = 0x1000;
        loc_b_ = 0x1000;
    } else {
        dialogs_.Dialog::field_2C_ = dialog_width_;
        int height = dialog_height_;
        if (external_window_ == 0 && (field_30A8_ & 0x400) == 0 &&
            WinScreenWidth >= 0x400) {
            height = (height * 3) / 2;
        }
        dialogs_.Dialog::field_30_ = height;
        if (default_loc_a_ != 0x2000) {
            loc_a_ = default_loc_a_;
        }
        if (default_loc_b_ != 0x2000) {
            loc_b_ = default_loc_b_;
        }
        dialogs_.Dialog::field_20_ |= 3;
    }

    if (flags & 0x1000) {
        loc_a_ = 0x1000;
        loc_b_ = 0x1000;
    }

    spot_.init(0xa);
    return 0;
}


// ORIGINAL: 0x00604300 ?on_button_clicked@BasePop@@QAEXH@Z 0x00604300-0x006043C8 FILE
// symbol    ?on_button_clicked@BasePop@@UAEXH@Z
//           Q->U 2026-09-06, Win's front slots 44/45 taking the image's
//           own names (on_button_clicked / on_button_toggled): the body is
//           now an override of the window's slot, so VC6 mangles it virtual
//           and enters it on the front table, while the image spells the
//           same body Q and its own table holds the raw address.
// size      201 bytes
// prototype void (__thiscall ?on_button_clicked@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609A50 0x00613740
// indirect  0x006043A1 0x006043AD 0x006043C2
// LEVER: `button_id` -1 is the window's own close row (field_3100_ = 0, the
//         same thing double_click does), -2 is the "close no matter what"
//         row that honours the 0x40 lock bit, anything else is an item id
//         parked in field_3100_.
// LEVER: the redraw slot (0xe8) runs on external_window_ when one is
//         attached, on the popup itself when not - the double_click tail.


void BasePop::on_button_clicked(int button_id) {
    selected_id_ = 0;
    if (dialogs_.get_num_items() != 0) {
        if (dialogs_.kind_ == 1) {
            selected_id_ = dialogs_.Dialog::selected_position_;
        } else {
            selected_id_ = dialogs_.Dialog::get_selected_id();
        }
    }
    if (button_id == -2) {
        if ((field_30A8_ & 0x40) != 0) {
            selected_id_ = -1;
        }
        field_3100_ = 0;
    } else if (button_id != -1) {
        field_3100_ = button_id;
    } else {
        field_3100_ = 0;
    }
    if ((field_30A8_ & 0x100) == 0) {
        if (external_window_ == 0) {
            vtable_slot<func_basepop_slot090>(this, 0xe8)(this);
        } else {
            vtable_slot<func_basepop_slot090>(external_window_,
                                              0xe8)(external_window_);
        }
    }
    if ((field_30A8_ & 0x4000) != 0) {
        vtable_slot<func_basepop_slot002>(this, 0x8)(this);
    }
}

// Fixed-slot bindings carried from 00601ac0.cpp

// ORIGINAL: 0x00601AC0 ?set_height@BasePop@@QAEXH@Z 0x00601AC0-0x00601B17 FILE BYTE_EXACT
// size      87 bytes
// prototype void (__thiscall ?set_height@BasePop@@QAEXH@Z)(BasePop* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00601AC0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00601ac0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_height@BasePop@@QAEXH@Z  at 0x00601AC0  (87 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

void BasePop::set_height(int height) {
    // set_width's twin, and its Dialog virtual-base access: the qualified
    // member form walks the vbtable at +0x21d0 and folds the rest into the
    // store. The artifact duplicated its whole tail because a SHARED tail
    // computation was register-allocated one swap short of exact - with the
    // member form the two arms are self-contained and the duplication is
    // gone. Measured BYTE_EXACT here.
    if (external_window_ == 0 && (field_30A8_ & 0x400) == 0 &&
        WinScreenWidth >= 0x400) {
        dialogs_.Dialog::field_30_ = (height * 3) / 2;
    } else {
        dialogs_.Dialog::field_30_ = height;
    }
}

// ORIGINAL: 0x006019F0 ?button@BasePop@@QAEHPAD@Z 0x006019F0-0x00601A3B FILE BYTE_EXACT
// symbol    ?button@BasePop@@QAEHPAD@Z
// size      75 bytes
// prototype int (__thiscall ?button@BasePop@@QAEHPAD@Z)(BasePop* this, int8*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100
// LEVER: the item-text twin of string() - the parsed pair is (text, 0, 0)
//         rather than (advance, colour, 0), the append id is 0 all the same,
//         and a successful add bumps field_20F4_, the popup's item count.

int BasePop::button(char *text) {
    if (text == 0) {
        return 3;
    }
    // A MATERIALISED LIST POINTER: the image lands on the three stores and the
    // append call with `mov ecx, this+0x2150` and one zero register shared by
    // the null test, the two stores and the push. Member access folds the
    // offset into each store and needs a second zero register instead.
    StringList *list = &item_ids_;
    list->field_1C_ = reinterpret_cast<uint32_t>(text);
    list->field_20_ = 0;
    list->field_24_ = 0;
    if (list->add(0) != 0) {
        return 1;
    }
    ++field_20F4_;
    return 0;
}

// ORIGINAL: 0x00601A40 ?UNK1@BasePop@@QAEHPAD@Z 0x00601A40-0x00601AB1 FILE BYTE_EXACT
// size      113 bytes
// prototype int (__thiscall ?UNK1@BasePop@@QAEHPAD@Z)(BasePop* this, int8*)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470
// EVIDENCE-FREE as a NAME: no caller, no dispatch slot and no data word in
//         the image points at UNK1, so the catalogue's placeholder stays.
//         What the body proves is the ROLE - it is owned_text_'s setter: it
//         frees the old heap string, allocates `strlen(text) + 1`, and
//         strcat's the new text in.

int BasePop::UNK1(char *text) {
    if (text == 0) {
        return 3;
    }
    if (owned_text_ != 0) {
        free(owned_text_);
        owned_text_ = 0;
    }
    owned_text_ = static_cast<char *>(mem_get(strlen(text) + 1));
    if (owned_text_ == 0) {
        return 4;
    }
    *owned_text_ = 0;
    strcat(owned_text_, text);
    return 0;
}

// ORIGINAL: 0x006017A0 ?set_ok_text@BasePop@@QAEHPAD@Z 0x006017A0-0x00601811 FILE BYTE_EXACT
// LEVER: UNK1's shape over ok_text_ instead of owned_text_, and its failure
//         code for a failed allocation is 5 where UNK1's is 4. STATIC DEFAULTS
//         and INSTANCE COPIES are the pair: set_def_ok_text (0x006018A0)
//         publishes BasePopDefaultOkText, this one copies the caller's string
//         into the popup's own ok_text_, and close() frees it and restores the
//         default.
// size      113 bytes
// prototype int (__thiscall ?set_ok_text@BasePop@@QAEHPAD@Z)(BasePop* this, int8*)
// callers   3   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470


int BasePop::set_ok_text(char *text) {
    if (text == 0) {
        return 3;
    }
    if (ok_text_ != 0) {
        free(ok_text_);
        ok_text_ = 0;
    }
    ok_text_ = static_cast<LPSTR>(mem_get(strlen(text) + 1));
    if (ok_text_ == 0) {
        return 5;
    }
    *ok_text_ = 0;
    strcat(ok_text_, text);
    return 0;
}

// ORIGINAL: 0x00601820 ?set_cancel_text@BasePop@@QAEHPAD@Z 0x00601820-0x0060189A FILE BYTE_EXACT
// size      122 bytes
// prototype int (__thiscall ?set_cancel_text@BasePop@@QAEHPAD@Z)(BasePop* this, int8*)
// callers   3   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470
// LEVER: set_ok_text's twin over cancel_text_. The artifact spelled strlen and
//         strcat as its own externals because the scaffold had no runtime
//         library; this tree does, so they are the plain calls the image makes.

int BasePop::set_cancel_text(char *text) {
    if (text == 0) {
        return 3;
    }
    if (cancel_text_ != 0) {
        free(cancel_text_);
        cancel_text_ = 0;
    }
    cancel_text_ = static_cast<LPSTR>(mem_get(strlen(text) + 1));
    if (cancel_text_ == 0) {
        return 5;
    }
    *cancel_text_ = 0;
    strcat(cancel_text_, text);
    return 0;
}


// ORIGINAL: 0x00605130 ?on_left_down@BasePop@@QAEXHH@Z 0x00605130-0x00605172 FILE BYTE_EXACT
// size      66 bytes
// prototype void (__thiscall ?on_left_down@BasePop@@QAEXHH@Z)(BasePop* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAB00
// indirect  0x00605168
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00605130
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00605130/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_down@BasePop@@QAEXHH@Z  at 0x00605130  (66 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void BasePop::on_left_down(int x, int y) {
    // SPOT, the member - not the address arithmetic the artifact carried, and
    // not a pointer to the subobject either (a base-subobject reference makes
    // VC6 emit its adjustor's null guard). The spot ids land back in x/y, so
    // `y == 0` is "the click was on the body, not on a spot".
    if (spot_.check(x, y, &x, &y) != -1) {
        if (y == 0) {
            // SLOT 44 (0xb0) - the redraw request. Win declares the slot, so
            // the call is the ordinary virtual dispatch the image makes
            // (`mov edx,[esi]; mov ecx,esi; call [edx+0xb0]`) and no shim is
            // needed.
            on_button_clicked(x);
        }
    }
}

// ORIGINAL: 0x00604A80 ?on_modal@BasePop@@QAAHXZ 0x00604A80-0x00604A8E BYTE_EXACT
// symbol    ?on_modal@BasePop@@SAHXZ
// size      14 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: STATIC, like set_def_ok_text - the QAA the catalogue spells puts a
//         receiver on the stack that nothing passes (no caller at all), and
//         the body reads only BasePopFalloutFlag.
// LEVER: THE POPUP IS MODAL WHEN THE FALLOUT FLAG IS CLEAR. One load and one
//         test, and the flag it reads is the fallout flag fallout() raises
//         under BasePopFalloutGate - the same word, not a second state.

int BasePop::on_modal() {
    return BasePopFalloutFlag == 0;
}

/*
Purpose: Measure the widest string the popup will draw.
// ORIGINAL: 0x00604EA0 ?UNK2@BasePop@@QAEHXZ 0x00604EA0-0x0060500A FILE
// size      362 bytes
// prototype int (__thiscall ?UNK2@BasePop@@QAEHXZ)(BasePop* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00619280 0x00644F3A
// LEVER: TERNARY CHAIN AS THE CALL ARGUMENT, not a named local. The image's
//         three-way caption fallback is `test/je; push; jmp` into ONE shared
//         `call width` - which is what `f(a ? a : (b ? b : c))` lowers to,
//         the argument pushed at the end of each arm. A named local (`char
//         *str = ok_text_; if (str == 0) str = ...`) lowers instead to a
//         `mov`-into-a-register chain and a single push, which is the
//         divergence the first transcription stopped at (#16, 0.76).
// LEVER: THE FIELD NAMES. ok_text_/cancel_text_ at 0x3224/0x3228,
//         BasePopDefaultOkText/CancelText at 0x009B8D80/84, the literals
//         "OK"/"Cancel" at 0x00697014/18, button_font1_ at 0x316c, and the
//         list at 0x2158..0x2164 is item_ids_ (StringList): head_ 0x2158,
//         current_ 0x215c, entry_count_ 0x2160, current_position_ 0x2164.
//         The three-way walks in the loop are `next_entry()` and
//         `current_entry()` OPEN-CODED - both are defined out of line in
//         stringstruct.cpp, so this TU cannot inline them, which the image
//         demonstrably does.
// TRIED: `abs(-1)` for seek_pos's two calls - VC6 folds the intrinsic at
//         compile time and both calls vanish. The tree's own seek_pos uses
//         the `AbsFnT abs_fn = &abs` pointer for exactly this reason; that
//         spelling is what this body carries, and `(*(&abs))(-1)` scores the
//         same 12/117 as the pointer, so the indirect call is not what is
//         blocking.
// CEILING (2026-09-05): 12/117 at first divergence #10, and the wall is not
//         the string fallbacks this body was stuck on before - those are now
//         spelled right. Three things, all of them outside this file:
//           1. The image dedicates EBX to max_width (`push ebx; xor ebx,ebx`
//              in the prologue, `mov ebx,eax` at each max); this tree hands
//              the slot to EBP and takes a frame. Register allocation, not
//              source shape - no local or spelling change reached it in four
//              measured attempts.
//           2. The caption fallback still pushes ONCE at the join where the
//              image pushes in each arm. `f(a ? a : (b ? b : c))` did not
//              produce it here.
//           3. The three list walks in the loop are the INLINED bodies of
//              StringStruct::seek_pos(-1), next_entry() and current_entry(),
//              all three defined out of line in stringstruct.cpp, so this TU
//              cannot inline any of them and each has to be open-coded -
//              which is why the `push -1 / call abs` pair is indirect here
//              and direct in the image (inlining folds the pointer away).
//         The unlock is a stringstruct.h change: make those three inline-
//         able from other TUs without disturbing the BYTE_EXACT
//         ?seek_pos@StringStruct@@QAEHH@Z at 0x004015B0. Not this file's to
//         make.
*/
// `abs` of a constant folds away under /O2, so seek_pos's two calls can only
// be reached through a pointer - the same spelling stringstruct.cpp uses.
typedef int (__cdecl *AbsFnT)(int);

int BasePop::UNK2() {
    if (field_30B0_ == 0) {
        return 0;
    }

    int max_width = 0;

    if ((field_30A8_ & 0x10000) == 0) {
        int width = button_font1_->width(ok_text_ != 0 ? ok_text_
                  : (BasePopDefaultOkText != 0 ? BasePopDefaultOkText : "OK"));
        if (width > 0) {
            max_width = width;
        }
    }

    if ((field_30A8_ & 0x40) != 0) {
        int width = button_font1_->width(
                cancel_text_ != 0 ? cancel_text_
                  : (BasePopDefaultCancelText != 0 ? BasePopDefaultCancelText
                                                   : "Cancel"));
        if (width > max_width) {
            max_width = width;
        }
    }

    // StringStruct::seek_pos(-1), open-coded. Its two `abs` calls survive
    // only through a pointer, because `abs` of a constant folds away.
    if (item_ids_.entry_count_ - 1 >= -1) {
        item_ids_.current_ = item_ids_.head_;
        AbsFnT abs_fn = reinterpret_cast<AbsFnT>(&abs);
        if (abs_fn(-1) <= item_ids_.entry_count_) {
            int walked = abs_fn(-1);
            if (walked > 0) {
                do {
                    --walked;
                    item_ids_.current_ = item_ids_.current_->previous;
                } while (walked != 0);
            }
            item_ids_.current_position_ = item_ids_.entry_count_ - 1;
        }
    }

    for (int i = 0; i < item_ids_.entry_count_; ++i) {
        if (item_ids_.head_ != 0) {
            // StringStruct::next_entry(), open-coded.
            item_ids_.current_ = item_ids_.current_->next;
            int position = item_ids_.current_position_ + 1;
            item_ids_.current_position_ = position;
            if (position == item_ids_.entry_count_) {
                item_ids_.current_position_ = 0;
            }
        }

        // StringStruct::current_entry()->text, open-coded.
        char *text = item_ids_.current_ != 0
                   ? (item_ids_.head_ != 0 ? item_ids_.current_->payload->text
                                           : static_cast<char *>(0))
                   : static_cast<char *>(0);
        int width = button_font1_->width(text);
        if (width > max_width) {
            max_width = width;
        }
    }

    return max_width;
}

// ORIGINAL: 0x00603800 ?calculate_dimensions@BasePop@@QAEHXZ 0x00603800-0x006042D2 FILE
// size      2770 bytes
// prototype int (__thiscall ?calculate_dimensions@BasePop@@QAEHXZ)(BasePop* this)
// callers   1   call targets   16
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DAD40 0x005DC360 0x005DCAB0 0x005DDDA0 0x005DDDE0 0x005DDEF0 0x006051A0 0x006099D0 0x00609FA0 0x0060A020 0x006120B0 0x006132A0
// indirect  0x0060407A
// LEVER: the popup lays its own dialog out, then hands the result to
//        Dialogs::calculate_dimensions twice - once before the text walk and
//        once again after the height moves - and both calls run on the same
//        `dialogs_` member, which is what the forwarder at 0x006132A0 in
//        pending_bodies.cpp is for.
// LEVER: the embedded Dialog virtual base is reached as a QUALIFIED MEMBER
//        ACCESS, the way set_width and on_redraw do it - the unqualified name
//        finds ListBox's own field at the same offset and stores there.
// LEVER: the owner's surface, not the popup's, carries every text call:
//        `owner_window_->set_text_color(...)` folds the +0x444 into the
//        receiver the image already holds (`mov ecx,[esi+0xa3c];
//        add ecx,0x444`), and wrap_height at 0x00603EFD and 0x00603F03 runs
//        on `this` instead - the one place the popup's own surface is used.
// The notification stores at popup offsets 0x2210, 0x2224 and 0x2234
// belong to ListBox, RadioButton and CheckBox respectively. Their handlers
// invoke the stored value as a cdecl callback, and the installed entry at
// 0x006044D0 loads its popup through WinModalFocus instead of an incoming
// receiver. Use the qualified callback members and the static function.
// Four remaining dialog_cfg stores address SpriteBox configuration at
// 0x225c/0x2260/0x22a4/0x22a8; that ownership/lifetime recovery is
// unfinished. Earlier anchor-versus-member spelling trials remain in Git
// history, not an instruction to preserve raw access against better evidence.
// TRIED: folding the anchor away, each store written
//        `*(&dialogs_.ListBox::field_2C_ + N) = v` - VC6 does NOT fold it
//        back into the one store; the body grew past what the FPO path would
//        take, the scorer switched to /O2 /Oy- /GX, and the frame-pointer
//        prologue dropped agreement to 10 of 703. Worse in both directions.
// TRIED: signed-arithmetic sweep on the unsigned fields, which is real and
//        stays: `(field_21C0_*3)/2` needs `static_cast<int>` or VC6 emits
//        `shr` where the image has `cdq/sub/sar` (81 -> 138 on that alone);
//        the two `max(field_30D4_, field_30DC_)` tests and
//        `edge > field_30D8_` need it for `jle` against `jbe`; and
//        field_30CC_'s divisor needs it for `idiv` against `div`. They do not
//        move the score past the register cascade, but they are the image's
//        arithmetic and they are not optional.
// TRIED: 2,770 bytes transcribed and the call census agrees with the image
//        (call_diff reports 0 bodies disagreeing about call count) - the
//        control flow, the two Dialogs::calculate_dimensions calls, the
//        sixteen callees and the vtable slot 0x174 are all in the right
//        order. What is left is allocation, not structure.
// NOTE: the blank line below is load-bearing. This block's comment run runs
//        straight into start's marker otherwise, and the reader's fact window
//        then reads START's `// symbol` as this body's - `_coff_function_masked`
//        takes the emitted symbol over the catalogued name, so measurement
//        pulled start's body out of the object and scored it against these
//        703 bytes. 4 of 703 was start, not this function.

// ORIGINAL: 0x00601BF0 ?start@BasePop@@QAEHPADPBDH0HPAUGraphicWin@@@Z 0x00601BF0-0x006025F9 FILE
// symbol    ?start@BasePop@@QAEHPADPBDH0HPAVGraphicWin@@@Z
//           The catalogue spells the sixth parameter `PAUGraphicWin@@` - a
//           STRUCT - and this tree declares `class GraphicWin`, which mangles
//           `PAV`. Same type, same layout, same bytes; only the letter moves.
// size      2569 bytes
// prototype int (__thiscall ?start@BasePop@@QAEHPADPBDH0HPAUGraphicWin@@@Z)(BasePop* this, char*, const char*, int, char*, int, GraphicWin*)
// callers   13   call targets   24
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00401100 0x005D4510 0x005E2480 0x005E3890 0x005F7E90 0x005FD530 0x005FD550 0x005FD570 0x00600760 0x006007B0 0x00600F00 0x006015B0 0x00611F80 0x006120D0 0x00612A70 0x00613740 0x00625880 0x00644EF2 0x006453E0 0x00645470 0x00645D1C 0x00645DA7 0x006469A2 0x0064FDF0
// LEVER: A CONFIG-SCRIPT PARSER, one directive per arm. The image reads the
//        text table a line at a time; lines that do not start with '#' are
//        popup items, and a '#' line is matched against the twenty keywords
//        at 0x00696F64..0x00696FEC (the BasePopKeyword* arrays below) with
//        _strnicmp(tok + 1, keyword, len) - tok[0] IS the '#'. The lengths
//        and the token offsets are in the table at the head of the switch.
// LEVER: VC6 DEFERS THE __CDECL CLEANUP, and the transcription that reads the
//        arm bodies at face value gets every token offset wrong. Each arm
//        calls purge_spaces, purge_leading and the converter on the SAME
//        tok + N, but the two earlier calls' four bytes of stack are not
//        given back until after the third, so the later `lea`s ride a lower
//        esp and read as +4 and +8. Measured off the image: xs 3, ys 3, x 2,
//        y 2, col 4, caption 8, checkbox 9, listbox 8, active 7, button 7,
//        extra 6, wave 5, vol 4, pitch 6, fade 5, pan 4, sprite 7; editbox,
//        text and itemlist take no argument.
// LEVER: THE COMPARE LENGTHS ARE PATH-DEPENDENT CONSTANTS, and reading the
//        register at the call site is a trap: `mov ebp, 4` sits in the col
//        arm, so text (0x0060212C), wave (0x0060221E) and fade (0x00602368)
//        all compare against 4 - and the `mov ebp, 1` in the vol/pitch/fade
//        arms never executes on the path that reaches a LATER test, because
//        any arm that matches leaves through `jmp 0x00601D05`. Verified per
//        site against the image: xs 2, ys 2, x 2, y 2, col 4, caption 7,
//        checkbox 8, editbox 7, listbox 7, active 6, text 4, itemlist 5,
//        button 6, extra 5, wave 4, vol 3, pitch 5, fade 4, pan 3, sprite 6.
// LEVER: the xs/ys wide-screen grow tests `external_window_ == 0` TWICE -
//        once to skip the converter and once to skip the *3/2 - the same
//        condition init() and set_width() carry.
// TRIED: the local declaration order that the image's frame layout spells -
//        flag, two pointers, the 0xF0 buffer, then four more - which is
//        exactly the order this body declares. The image's frame is
//        `sub esp, 0x10c` (0xC of slots, the buffer, then 0x10 of slots
//        above it); this tree's is `sub esp, 0xfc`, because VC6 coalesces
//        marks/token/width/height into the three slots below the buffer
//        rather than giving them the four above it. 47 of 742 either way.
// RESIDUAL: 47 of 742, and the whole gap is that one `sub`. No esp-relative
//        displacement after instruction 0 can agree while the frames differ,
//        and this body is dominated by them - the parse buffer is addressed
//        off esp in every arm. The call census agrees with the image (78
//        direct calls, call_diff reports 0 bodies disagreeing about count),
//        so the structure, the directive order and the twenty keyword
//        compares are right; what is left is frame layout, and the frame is
//        the compiler's, not the source's.
int BasePop::start(char *text_file, const char *text_label, int value,
                   char *override_text, int flags, GraphicWin *owner_window) {
    // The parse buffer is 0xF0 bytes at frame 0x1c; `add_as_escaped` is the
    // dword at frame 0x10, seeded 1 by the `mov ebp, 1` in the prologue.
    int add_as_escaped = 1;
    char *line;
    char *text;
    char tok[0xf0];
    int marks;
    char *token;
    int width;
    int height;

    if ((flags & 0x80) == 0) {
        close();
    }
    if ((flags & 1) != 0) {
        dialogs_.kind_ = 1;
    } else if ((flags & 2) != 0) {
        dialogs_.kind_ = 2;
    } else if ((flags & 4) != 0) {
        dialogs_.kind_ = 4;
    } else {
        dialogs_.kind_ = 0x10;
    }

    int result = 0;
    if ((flags & 0x80) != 0 || (result = init(0, flags)) == 0) {
        if ((flags & 0x100000) == 0) {
            external_window_ = 0;
        }
        if (owner_window != 0) {
            owner_window->is_visible();
            owner_window_ = owner_window;
        }
        value_ = value;
        if (text_label == 0) {
            return 3;
        }
        if (override_text != 0) {
            dialogs_.EditGroup::set_default(override_text, 0);
        }
        if (text_open(text_file != 0 ? text_file : (char *)PopupStartCaption,
                      text_label) == 0) {
            return result;
        }

      read_line:
        for (;;) {
            line = text_get();
            if (line == 0) {
                return 1;
            }
            if (*line != '\0') {
                break;
            }
            add_as_escaped = 0;
        }
        tok[0] = '\0';
        parse_string(line, tok);

        if (tok[0] != '#') {
            // A plain line: an item, an item with the '^' prefix marks, or a
            // value for the edit box. Which one depends on whether the caller
            // asked for escaped text at all.
            if (add_as_escaped) {
                marks = 0;
                text = tok;
                if (tok[0] == '^') {
                    if (tok[1] == '\0') {
                        marks = 3;
                        text = tok + 1;
                    } else if (tok[1] == '^') {
                        marks = 2;
                        text = (tok[2] == '^') ? (tok + 3) : (tok + 2);
                    } else {
                        marks = 1;
                        text = tok + 1;
                    }
                }
                unit_ids_.field_1C_ = reinterpret_cast<uint32_t>(text);
                unit_ids_.field_20_ = marks;
                unit_ids_.field_24_ = 0;
                unit_ids_.add(0);
            } else if ((field_30A8_ & 4) == 0) {
                dialogs_.item(tok, 0);
            } else {
                if (dialogs_.get_num_items() < 10) {
                    if (flags == 0 || dialogs_.get_num_items() != 0) {
                        dialogs_.EditGroup::edit(tok, 0, 0);
                    } else {
                        dialogs_.EditGroup::edit(tok, 0, 0);
                    }
                } else {
                    dialogs_.EditGroup::edit(tok, 0, flags);
                }
            }
            goto read_line;
        }

        // THE DIRECTIVE TABLE. (keyword, compare length, token offset N).
        // Converter is atoi unless the comment says otherwise.
        if (_strnicmp(tok + 1, BasePopKeywordXs, 2) == 0) {
            purge_spaces(tok + 3);
            purge_leading(tok + 3);
            int v = atoi(tok + 3);
            if (external_window_ == 0) {
                if ((field_30A8_ & 0x400) == 0 && WinScreenWidth > 0x3ff) {
                    v = (v * 3) / 2;
                }
            }
            dialogs_.Dialog::field_2C_ = v;
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordYs, 2) == 0) {
            purge_spaces(tok + 3);
            purge_leading(tok + 3);
            int v = atoi(tok + 3);
            if (external_window_ == 0) {
                if ((field_30A8_ & 0x400) == 0 && WinScreenWidth > 0x3ff) {
                    v = (v * 3) / 2;
                }
            }
            dialogs_.Dialog::field_30_ = v;
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordX, 2) == 0) {
            purge_spaces(tok + 2);
            purge_leading(tok + 2);
            int v = atoi(tok + 2);
            if (v != 0x2000) {
                loc_a_ = v;
            }
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordY, 2) == 0) {
            purge_spaces(tok + 2);
            purge_leading(tok + 2);
            int v = atoi(tok + 2);
            if (v != 0x2000) {
                loc_b_ = v;
            }
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordCol, 4) == 0) {
            purge_spaces(tok + 4);
            purge_leading(tok + 4);
            dialogs_.Dialog::field_4C_ = atoi(tok + 4);
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordCaption, 7) == 0) {
            purge_spaces(tok + 8);
            purge_leading(tok + 8);
            if (owned_text_ != 0) {
                free(owned_text_);
                owned_text_ = 0;
            }
            unsigned int len = strlen(tok + 8);
            owned_text_ = static_cast<char *>(mem_get(len + 1));
            if (owned_text_ != 0) {
                *owned_text_ = '\0';
                strcat(owned_text_, tok + 8);
            }
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordCheckbox, 8) == 0) {
            field_30A8_ |= 1;
            dialogs_.kind_ = 1;
            purge_spaces(tok + 9);
            purge_leading(tok + 9);
            if (atoi(tok + 9) != 0) {
                dialogs_.Dialog::field_50_ = atoi(tok + 9);
            }
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordEditbox, 7) == 0) {
            field_30A8_ |= static_cast<uint32_t>(dialogs_.kind_);
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordListbox, 7) == 0) {
            field_30A8_ |= 2;
            dialogs_.kind_ = 2;
            purge_spaces(tok + 8);
            purge_leading(tok + 8);
            if (atoi(tok + 8) != 0) {
                dialogs_.Dialog::field_50_ = atoi(tok + 8);
            }
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordActive, 6) == 0) {
            purge_spaces(tok + 7);
            purge_leading(tok + 7);
            value_ = atoi(tok + 7);
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordText, 4) == 0) {
            add_as_escaped = 1;
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordItemlist, 5) == 0) {
            add_as_escaped = 0;
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordButton, 6) == 0) {
            purge_spaces(tok + 7);
            purge_leading(tok + 7);
            text = tok + 7;
            if (*text != '\0') {
                item_ids_.field_1C_ = reinterpret_cast<uint32_t>(text);
                item_ids_.field_20_ = 0;
                item_ids_.field_24_ = 0;
                if (item_ids_.add(0) == 0) {
                    field_20F4_ = field_20F4_ + 1;
                }
            }
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordExtra, 5) == 0) {
            purge_spaces(tok + 6);
            purge_leading(tok + 6);
            field_30AC_ = atoi(tok + 6);
            goto read_line;
        }
        // The five text-block directives come in a/b pairs: a trailing '2'
        // on the keyword selects the second 0x118 block, everything else the
        // first. Both blocks are the same five fields, so the pair is one
        // reference chosen up front.
        if (_strnicmp(tok + 1, BasePopKeywordWave, 4) == 0) {
            BasePopTextBlock &block =
                (tok[1] == '2') ? text_block_b_ : text_block_a_;
            token = tok + 5 + ((tok[1] == '2') ? 1 : 0);
            purge_spaces(token);
            purge_leading(token);
            if (isdigit(static_cast<unsigned char>(*token))) {
                block.field_104_ = atoi(token);
            }
            block.text[0] = '\0';
            strcat(block.text, token);
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordVol, 3) == 0) {
            BasePopTextBlock &block =
                (tok[1] == '2') ? text_block_b_ : text_block_a_;
            token = tok + 4 + ((tok[1] == '2') ? 1 : 0);
            purge_spaces(token);
            purge_leading(token);
            block.field_108_ = atol(token);
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordPitch, 5) == 0) {
            BasePopTextBlock &block =
                (tok[1] == '2') ? text_block_b_ : text_block_a_;
            token = tok + 6 + ((tok[1] == '2') ? 1 : 0);
            purge_spaces(token);
            purge_leading(token);
            block.field_10C_ = atoi(token);
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordFade, 4) == 0) {
            BasePopTextBlock &block =
                (tok[1] == '2') ? text_block_b_ : text_block_a_;
            token = tok + 5 + ((tok[1] == '2') ? 1 : 0);
            purge_spaces(token);
            purge_leading(token);
            block.field_110_ = atol(token);
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordPan, 3) == 0) {
            BasePopTextBlock &block =
                (tok[1] == '2') ? text_block_b_ : text_block_a_;
            token = tok + 4 + ((tok[1] == '2') ? 1 : 0);
            purge_spaces(token);
            purge_leading(token);
            block.field_114_ = atoi(token);
            goto read_line;
        }
        if (_strnicmp(tok + 1, BasePopKeywordSprite, 6) == 0) {
            purge_spaces(tok + 7);
            purge_leading(tok + 7);
            if (owner_window_->get_pcx_dimensions(tok + 7, &width, &height)
                == 0) {
                if (sprite_.init(tok + 7, width, height) == 0) {
                    caller_sprite_ = &sprite_;
                }
            }
            goto read_line;
        }
        if (tok[1] == '#') {
            goto read_line;
        }
        text_close();
        result = 0;
    }
    return result;
}


typedef void(__fastcall *func_basepop_slot093)(void *);

int BasePop::calculate_dimensions() {
    // THE OWNER'S SURFACE, for every text call below. GraphicWin's Buffer
    // base sits 0x444 in, behind its Win base; calling the inherited method
    // straight on the pointer lets VC6 fold that 0x444 into the receiver it
    // is already holding.
    owner_window_->set_text_color(string_color_a_, string_color_b_,
                                  string_color_c_, string_color_d_);
    owner_window_->set_text_color2(string_color_2a_, string_color_2b_,
                                   string_color_2c_, string_color_2d_);
    owner_window_->set_text_color3(string_color_3a_, string_color_3b_,
                                   string_color_3c_, string_color_3d_);
    owner_window_->set_text_color_hyper(string_color_hyper_a_,
                                        string_color_hyper_b_,
                                        string_color_hyperc_,
                                        string_color_hyper_d_);
    owner_window_->set_font(string_font1_, string_font2_, string_font3_,
                            string_font4_);

    // THE TWO ROW-COUNT GROWS: one for the fallout bit, one for a popup that
    // is neither wide-screen-scaled nor hyperlinked.
    field_30B0_ = field_20F4_;
    if ((field_30A8_ & 0x40) != 0) {
        ++field_30B0_;
    }
    if ((field_30A8_ & 0x10100) == 0) {
        ++field_30B0_;
    }

    // The dialog's inset: the wide-screen 3/2 grow applies only when the
    // popup draws into itself and the screen is wide.
    if (external_window_ != 0) {
        field_30E0_ = 0;
        field_30E4_ = field_21C4_;
    } else if ((field_30A8_ & 0x400) == 0 && WinScreenWidth >= 0x400) {
        // SIGNED: the image's `cdq / sub eax,edx / sar eax,1` is the signed
        // divide, and the fields are the unsigned dwords they are stored in.
        field_30E0_ = (static_cast<int>(field_21C0_) * 3) / 2;
        field_30E4_ = (static_cast<int>(field_21C4_) * 3) / 2;
    } else {
        field_30E0_ = field_21C0_;
        field_30E4_ = field_21C4_;
    }
    // The image emits this pair twice, once per arm of the test above, and
    // with a different register pair each time - so the source branches here
    // again rather than falling through.
    if (external_window_ != 0) {
        field_30E8_ = field_21B8_;
        field_30EC_ = field_21BC_;
    } else {
        field_30E8_ = field_21B8_;
        field_30EC_ = field_21BC_;
    }

    field_30F0_ = field_21B0_;
    field_30F4_ = field_21B4_;
    const int border_x = field_21B0_;
    if (caller_sprite_ != 0) {
        // The caller's picture scales the dialog: its width sets the left
        // edge, its height the top one, both through the shared pair at
        // 0x3104/0x3108, and the top one adds the inset when the list is
        // showing anything.
        int top_extra = 0;
        if (unit_ids_.entry_count_ != 0) {
            top_extra = field_30E4_;
        }
        const int scale = field_3104_;
        const int divide = field_3108_;
        field_30D8_ = caller_sprite_->iWidth_ * scale / divide + field_30E0_ +
                      top_extra;
        field_30DC_ = caller_sprite_->iHeight_ * scale / divide;
    } else {
        field_30D8_ = 0;
        field_30DC_ = 0;
    }

    // The row pitch comes from the first button font's height, read whether
    // or not anything below runs - the image loads it before the guard.
    const int button_height = button_font1_->height_;

    if (external_window_ != 0) {
        // THE EMBEDDED DIALOG VIRTUAL BASE, as a qualified member access.
        dialogs_.Dialog::field_2C_ =
            dialog_width_ - field_30E0_ * 2 - field_21B4_ - border_x;
        const int shrink = field_30B0_ != 0 ? field_30D0_ : 0;
        int height = dialog_height_ - shrink - field_30EC_ - field_30E8_;
        if (external_window_ == 0 && (field_30A8_ & 0x400) == 0 &&
            WinScreenWidth >= 0x400) {
            height = (height * 3) / 2;
        }
        dialogs_.Dialog::field_30_ = height;
        if (default_loc_a_ != 0x2000) {
            loc_a_ = default_loc_a_;
        }
        if (default_loc_b_ != 0x2000) {
            loc_b_ = default_loc_b_;
        }
        dialogs_.Dialog::field_20_ |= 1;
        UNK4(1);
    }

    int result = dialogs_.calculate_dimensions();
    if (result != 0) {
        return result;
    }

    // THE EIGHT CONFIG SLOTS - see the HANDOFF under the marker above.
    uint32_t *const dialog_cfg = &dialogs_.ListBox::field_2C_;

    if (external_window_ != 0) {
        // With a window attached the popup's own edges win outright.
        int left = dialogs_.Dialog::field_5C_;
        if (left == 0) {
            left = dialogs_.Dialog::field_2C_;
        }
        field_30BC_ = left;
        int top = dialogs_.Dialog::field_60_;
        if (top == 0) {
            top = dialogs_.Dialog::field_30_;
        }
        field_30C0_ = top;
    } else {
        // Without one the list's own edge competes with the drawn popup's,
        // and field_21CC_ offsets it when it is set at all.
        int edge = dialogs_.Dialog::field_5C_ != 0 ? dialogs_.Dialog::field_5C_
                                                   : dialogs_.Dialog::field_2C_;
        if (field_21CC_ == -1) {
            if (edge > static_cast<int>(field_30D8_)) {
                edge = dialogs_.Dialog::field_5C_ != 0
                           ? dialogs_.Dialog::field_5C_
                           : dialogs_.Dialog::field_2C_;
            }
            field_30BC_ = edge + field_30E0_ * 2;
        } else {
            if (edge > static_cast<int>(field_30D8_)) {
                edge = dialogs_.Dialog::field_5C_ != 0
                           ? dialogs_.Dialog::field_5C_
                           : dialogs_.Dialog::field_2C_;
            } else {
                edge = field_30D8_;
            }
            field_30BC_ = field_21CC_ + field_30E0_ + edge;
        }
    }

    field_30C4_ = dialogs_.Dialog::field_60_;
    // The kind decides which sub-dialog this popup configures. `double_click`
    // is the notification each of the three slots receives.
    switch (dialogs_.kind_ + 1) {
      case 0:
        break;
      case 2:
        dialogs_.CheckBox::double_click_callback_ =
            &BasePop::double_click;
        break;
      case 3:
        if (value_ != -1) {
            dialogs_.ListBox::set_selected_id(value_);
        } else {
            dialogs_.ListBox::set_selected_pos(0, 1);
        }
        dialogs_.ListBox::double_click_callback_ =
            &BasePop::double_click;
        break;
      case 5:
        if (value_ != -1) {
            dialogs_.EditGroup::set_text_limits(value_);
        }
        break;
      case 9:
        dialog_cfg[0x2b] = field_3160_;                            // 0x22a8
        dialogs_.SpriteBox::double_click_callback_ = sprite_double_click_callback_;
        dialog_cfg[0x2a] = reinterpret_cast<uint32_t>(redraw_callback_);
        dialog_cfg[0x19] = field_3108_;                            // 0x2260
        dialog_cfg[0x18] = field_3104_;                            // 0x225c
        break;
      default:
        if (value_ != -1) {
            dialogs_.Dialog::set_selected_id(value_);
        } else {
            dialogs_.Dialog::selected_position_ = 0;
        }
        dialogs_.RadioButton::double_click_callback_ =
            &BasePop::double_click;
        break;
    }

    // THE TEXT WALK. text_left is the popup's usable width with both insets
    // and the list offset taken out; the image computes it once, into EBX,
    // and hands it to every wrap call that follows.
    const int text_left =
        field_30BC_ - field_30D8_ - field_21C8_ - field_30E0_ * 2;

    int wrap_kind = 0;
    if (unit_ids_.head_ != 0) {
        wrap_kind = unit_ids_.current_->payload->field_8_;
    }
    field_30D4_ = 0;
    if (wrap_kind == 3) {
        field_30D4_ += owner_window_->text_line_height();
        owner_window_->wrap_height_flying(static_cast<char *>(0), text_left);
    } else if (wrap_kind == 2) {
        char *text = unit_ids_.head_ != 0 ? unit_ids_.current_->payload->text
                                          : static_cast<char *>(0);
        field_30D4_ += owner_window_->wrap_height(text, text_left);
        owner_window_->wrap_height_flying(static_cast<char *>(0), text_left);
    } else {
        char *text = unit_ids_.head_ != 0 ? unit_ids_.current_->payload->text
                                          : static_cast<char *>(0);
        owner_window_->wrap_height_flying(text, text_left);
    }

    // The remaining items, each advanced by next_entry() open-coded. The
    // image re-tests head_ after the calls - it does not trust the register
    // across them - so the source tests it again too.
    for (int index = 0; index < unit_ids_.entry_count_ - 1; ++index) {
        if (unit_ids_.head_ != 0) {
            ++unit_ids_.current_position_;
            unit_ids_.current_ = unit_ids_.current_->next;
            if (unit_ids_.current_position_ == unit_ids_.entry_count_) {
                unit_ids_.current_position_ = 0;
            }
        }

        // StringStruct::current_entry()->text, open-coded. BUG IN THE
        // Original behavior: the middle arm dereferences a null payload and reads
        // address 4 when the list has a cursor but no head - the image emits
        // `xor eax,eax / mov ebp,[eax+4]` there. Reproduced as shipped.
        char *text = unit_ids_.current_ != 0
                   ? (unit_ids_.head_ != 0 ? unit_ids_.current_->payload
                                           : static_cast<StringStructPayload *>(0))
                         ->text
                   : static_cast<char *>(0);

        int kind = wrap_kind;
        if (unit_ids_.head_ != 0) {
            kind = unit_ids_.current_->payload->field_8_;
        }
        if (unit_ids_.head_ != 0 && kind != 0) {
            field_30D4_ += owner_window_->wrap_height_flying_flush();
            if (kind == 2) {
                field_30D4_ += wrap_height(text, text_left);
                owner_window_->wrap_height_flying(static_cast<char *>(0),
                                                  text_left);
            } else if (kind == 3) {
                field_30D4_ += owner_window_->text_line_height();
                owner_window_->wrap_height_flying(static_cast<char *>(0),
                                                  text_left);
            } else {
                owner_window_->wrap_height_flying(text, text_left);
            }
        } else {
            owner_window_->wrap_height_flying(text);
        }
    }

    field_30D4_ += owner_window_->wrap_height_flying_flush();
    const int text_height = field_30D4_;

    if (field_30B0_ != 0) {
        // One row of slack per line, rounded to the font's four-byte grid.
        field_30D0_ = (button_height * 8 - button_height) / 4;
        // SIGNED: the image divides with `cdq / idiv`, and the fields are
        // stored as unsigned dwords, so every operand is widened here.
        field_30CC_ =
            (static_cast<int>(field_30BC_) - static_cast<int>(field_30E0_) * 2 -
             (static_cast<int>(field_30B0_) - 1) *
                 static_cast<int>(field_30E4_)) /
            static_cast<int>(field_30B0_);
    } else {
        field_30D0_ = 0;
        field_30CC_ = 0;
    }

    if (external_window_ != 0 && (field_30A8_ & 2) != 0) {
        // The dialog grew: run the layout a second time with the new height.
        dialogs_.Dialog::field_30_ = dialog_height_ - field_30E4_ * 2 -
                                     field_30D0_ - text_height;
        result = dialogs_.calculate_dimensions();
        if (result != 0) {
            return result;
        }
    }

    // Slot 0x174 - the popup's own layout hook, run after the dialog settles.
    vtable_slot<func_basepop_slot093>(this, 0x174)(this);

    if ((field_30A8_ & 0x100) != 0) {
        field_30D0_ = 0;
    }

    if (external_window_ != 0) {
        dialogs_.Dialog::field_2C_ =
            dialog_width_ - field_30E0_ * 2 - field_30F4_ - field_30F0_;
        int top = static_cast<int>(field_30D4_) > static_cast<int>(field_30DC_)
                     ? static_cast<int>(field_30D4_)
                     : static_cast<int>(field_30DC_);
        int height = dialog_height_ - (field_30B0_ != 0 ? field_30D0_ : 0) -
                     top - field_30EC_ - field_30E8_;
        if (external_window_ == 0 && (field_30A8_ & 0x400) == 0 &&
            WinScreenWidth >= 0x400) {
            height = (height * 3) / 2;
        }
        dialogs_.Dialog::field_30_ = height;
        if (default_loc_a_ != 0x2000) {
            loc_a_ = default_loc_a_;
        }
        if (default_loc_b_ != 0x2000) {
            loc_b_ = default_loc_b_;
        }
        dialogs_.Dialog::field_20_ |= 3;
        if (external_window_ != 0) {
            field_30BC_ = dialog_width_;
            field_30C0_ = dialog_height_;
            if (dialogs_.kind_ == -1) {
                field_30C4_ = 0;
                dialogs_.Dialog::field_60_ = 0;
            }
        }
    } else {
        int top = static_cast<int>(field_30D4_) > static_cast<int>(field_30DC_)
                     ? static_cast<int>(field_30D4_)
                     : static_cast<int>(field_30DC_);
        int inner = field_30C4_ + field_30E0_ * 2 + top + field_30D0_;
        if (field_30C4_ != 0) {
            inner += field_30E4_;
        }
        if (field_30D0_ != 0) {
            inner += field_30E4_;
        }
        field_30C0_ = inner + field_30EC_ + field_30E8_;
        field_30BC_ += field_30F4_ + field_30F0_;
    }

    // THE PUBLISHED EXTENT: the next popup opens below/behind this one.
    BasePopDrawnRight = (owner_window_ != this ? field_30B4_ : 0) +
                        field_30F0_ + field_30E0_ + field_30D8_;
    BasePopDrawnBottom = (owner_window_ != this ? field_30B8_ : 0) +
                         field_30E0_ + field_30D4_ + field_30E8_;
    return 0;
}

// ORIGINAL: 0x00604AB0 ?on_redraw@BasePop@@QAEXXZ 0x00604AB0-0x00604E31 FILE
// size      897 bytes
// prototype void (__thiscall ?on_redraw@BasePop@@QAEXXZ)(BasePop* this)
// callers   0   call targets   12
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5250 0x005DA9F0 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005DAD40 0x005DB040 0x005DEF90 0x005E4B4A 0x005ED7D0 0x00611640
// indirect  0x00604B0F 0x00604BE9 0x00604C37
// LEVER: the popup draws into its OWN surface (this, +0x444) for the two
//        background pictures and into the OWNER's surface (owner_window_,
//        +0x444) for every text call - the distinction is the body's spine,
//        and one `Buffer *` for both put the owner's loads where the image
//        recomputes them.
// LEVER: one spilled local. The image reserves a single stack slot (its
//        prologue is `push ecx`) and it holds own_bottom, the value that is
//        still live at the clamps; the register copy of it is the one the
//        external-window offset is added to. Everything else is re-read from
//        the object at its use - caching field_30E0_ across the text block
//        cost the second read the image makes there.
// LEVER: vtable slot 0x180 through `vtable_slot`, the same spelling
//        double_click (0x006044D0) uses for its own two dispatches. Slot 96
//        is the redraw hook the image runs on this when no external window is
//        attached, and its EAX is tested on the first of the two calls only -
//        one slot, two call sites, not two slots.
// HANDOFF (spritebox.h, not a file this pass owns): the image reaches
//        SpriteBox::pos_to_id (0x00611640) on the SpriteBox subobject of
//        dialogs_ and reads its EAX; spritebox.h declares only id_to_pos
//        (0x00611600). `int pos_to_id(int);` is the declaration it needs.
//        Until it lands, the free forwarder sprite_box_pos_to_id in
//        pending_bodies.cpp stands in for the call.
//        CLEARED (wave 5c): spritebox.h declares `int pos_to_id(int);` and
//        the call below is the qualified member access.
// TRIED: `Buffer *const buffer = static_cast<Buffer *>(this)` held across the
//        body, the obvious spelling for the +0x444 surface - VC6 emits its
//        base-conversion null guard for it (`test esi,esi / je` plus a spill
//        slot) and the image has neither; calling the inherited Buffer method
//        straight on `this` / `owner_window_` folds the 0x444 into the
//        receiver instead and loses both. Same for a `text_buffer` local over
//        the owner.
// TRIED: recomputing the clamp base as `field_30DC_ + field_30E8_ + border`
//        to avoid holding two names for the same edge - 277 instructions
//        against the image's 271, worse than the two-local form, because VC6
//        will not CSE the reload across the calls.
// TRIED: `const int own_bottom = bottom` (copy after the fact) instead of
//        `= field_30E8_ + border` - identical 273, no flip.
// RESIDUAL: the image reserves ONE stack slot for own_bottom (`push ecx`)
//        and keeps the external-window-adjusted copy of it in ebp; this tree
//        spills both names (`sub esp, 8`) and works the pair in ebx/ebp.
//        Every other instruction shape agrees. This is the receiver-spill
//        wall the brief documents, hit on the stack-slot count rather than on
//        a callee-saved push.
typedef int(__fastcall *func_basepop_slot096)(void *);

void BasePop::on_redraw() {
    // THE INHERITED-MEMBER SPELLING, and it is load-bearing. GraphicWin's
    // Buffer base sits 0x444 in, behind its Win base; calling the inherited
    // Buffer method straight on `this` or on `owner_window_` lets the compiler
    // fold that 0x444 into the receiver it is already holding
    // (`lea ecx,[esi+0x444]`, `add ecx,[esi+0xa3c] + 0x444`). Holding the
    // conversion in a `Buffer *` local instead makes VC6 emit its base-
    // conversion null guard - `test esi,esi / je` plus a spill slot for the
    // result - which the image has nowhere in this body.
    clear_links();

    const int border = field_30E0_;
    int right = border + field_30F0_ + field_21C8_;
    // The same edge before the external-window position is folded in - the
    // pair the clamps at the end measure the text height against. Two names
    // for it, because the clamps read the base while the draws read the one
    // the external-window position has been added to.
    const int own_bottom = field_30E8_ + border;
    int bottom = own_bottom;

    if (owner_window_ == this) {
        move(field_30B4_, field_30B8_);
        if (vtable_slot<func_basepop_slot096>(this, 0x180)(this) == 0) {
            // The background: tile the popup's own picture when there is one
            // and the popup is drawing into itself, otherwise a flat fill.
            if (field_2148_ != 0 && external_window_ == 0) {
                tile(field_2148_, 0, 0, 0, 0, dib_.bmiHeader.biWidth,
                     -dib_.bmiHeader.biHeight);
            } else {
                GraphicWin::fill(field_310C_);
            }
        }

        if (external_window_ == 0 && field_214C_ != 0 &&
            field_214C_->iSpriteWidth_ < dib_.bmiHeader.biWidth &&
            field_214C_->iSpriteHeight_ < -dib_.bmiHeader.biHeight) {
            // Centred on the surface, its height measured against the DIB's
            // top-down (negative) biHeight.
            field_214C_->draw(
                static_cast<Buffer *>(this), field_214C_->cTransparentIndex_,
                (dib_.bmiHeader.biWidth - field_214C_->iWidth_ - field_21C8_) /
                        2 +
                    field_21C8_,
                -((field_214C_->iHeight_ + dib_.bmiHeader.biHeight) / 2), 1, 1);
        }
    } else {
        vtable_slot<func_basepop_slot096>(this, 0x180)(this);
        right += field_30B4_;
        bottom += field_30B8_;
    }

    if (caller_sprite_ != 0) {
        BasePopDrawnPopup = this;
        if (redraw_callback_ != 0) {
            // The image calls SpriteBox::pos_to_id on the subobject of
            // dialogs_ at +0x2240. QUALIFIED MEMBER ACCESS - naming the
            // base is what makes VC6 emit the virtual-base subobject walk
            // natively, the same spelling every other base-subobject call
            // in this body uses.
            const int item_id = dialogs_.SpriteBox::pos_to_id(0);
            redraw_callback_(
                reinterpret_cast<int>(caller_sprite_),
                reinterpret_cast<int>(owner_window_), item_id, right, bottom);
        } else {
            caller_sprite_->draw(
                owner_window_ != 0 ? static_cast<Buffer *>(owner_window_)
                                   : static_cast<Buffer *>(0),
                caller_sprite_->cTransparentIndex_, right, bottom, field_3104_,
                field_3108_);
        }
        BasePopDrawnPopup = 0;
    }

    right += field_30D8_;
    if (field_30D4_ == 0) {
        // No text to lay out: publish the picture's extent and stop.
        BasePopDrawnRight = right - field_30D8_ - field_21C8_;
        if (bottom > field_30DC_ + own_bottom) {
            BasePopDrawnBottom = bottom;
            return;
        }
        BasePopDrawnBottom = field_30DC_ + own_bottom;
        return;
    }

    owner_window_->set_text_color(string_color_a_, string_color_b_,
                                  string_color_c_, string_color_d_);
    owner_window_->set_text_color2(string_color_2a_, string_color_2b_,
                                   string_color_2c_, string_color_2d_);
    owner_window_->set_text_color3(string_color_3a_, string_color_3b_,
                                   string_color_3c_, string_color_3d_);
    owner_window_->set_text_color_hyper(string_color_hyper_a_,
                                        string_color_hyper_b_,
                                        string_color_hyperc_,
                                        string_color_hyper_d_);
    owner_window_->set_font(string_font1_, string_font2_, string_font3_,
                            string_font4_);

    const int text_left = -(field_30E0_ * 2) - field_30F4_ + field_30BC_ -
                          field_30D8_ - field_30F0_ - field_21C8_;
    const int height = owner_window_->write_strings(&unit_ids_, right, bottom,
                                                    text_left, 0);

    BasePopDrawnRight = right - field_30D8_ - field_21C8_;
    BasePopDrawnBottom = height;
    if (height <= field_30DC_ + own_bottom) {
        BasePopDrawnBottom = field_30DC_ + own_bottom;
    }
}
