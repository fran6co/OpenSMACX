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
#include "jackal.h"
#include "general.h" // trig_init; utility ownership is not library membership.
#include "palette.h"
#include "win.h"
#include "buffer.h"
#include "checkbox.h"
#include "checkbutton.h"
#include "radiobutton.h"
#include "basebutton.h"
#include "font.h"
#include "basepop.h"
#include "filewin.h"
#include "sprite.h"
#include "cursor.h"
#include "time.h"
#include "strings.h"
#include <cstring>

// These three pinned lifecycle/version functions establish the current Jackal
// boundary. Called subsystems remain separately owned; their archive membership
// and the original translation-unit boundaries are not inferred from these calls.
// close_class methods, homed to checkbox.cpp / radiobutton.cpp
void __cdecl teardown_0060fd60();
void __cdecl teardown_0060e5d0();

/*
Purpose: Checks to see if the JACKAL library version is up to date. Pretty pointless but might add
         an OpenSMACX check in the future.
// ORIGINAL: 0x0062D570 ?jackal_version_check@@YAHPAD@Z 0x0062D570-0x0062D5A3 BYTE_EXACT
// symbol    ?jackal_version_check@@YAHPBD@Z
// size      51 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645660
// indirect  0x0062D594
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl jackal_version_check(LPCSTR version) {
    if (strcmp(version, "10.10")) {
        MessageBoxA(NULL,
            "The version of the JACKAL library you (Brian Reynolds) are attempting to use "
            "is out of sync with the library headers! Please rebuild JACKAL.LIB...Bye!",
            "Oh, such serious danger, William Robinson!!!",
            MB_OK);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// The Jackal engine's bring-up, promoted out of src/unrecovered/0062d3a0.cpp
// on 2026-08-15. WinMain calls it once; everything the game draws with exists
// because this returned zero.
// ---------------------------------------------------------------------------

// 0x009B7AF4 is WinDynamicDefaults.default_font_ (win.h) - the store below
// seeds it from FontDefault (0x009BB484), and config_popups later re-points
// it via find_font; the two readers are the Win constructor and Win::close,
// both of which move it into field_F8_.

int JackalInitFlags;      // 0x009BC4B0

/*
Purpose: Bring up every drawing subsystem the game needs, in order, and stop
         at the first one that refuses.
// ORIGINAL: 0x0062D3A0 ?jackal_init_real@@YAHPAUPalette@@PAUFont@@PADHHHH@Z 0x0062D3A0-0x0062D4F6 BYTE_EXACT
// symbol    ?jackal_init_real@@YAHPAVPalette@@PAVFont@@PADHHHH@Z
// size      342 bytes
// prototype int (__cdecl ?jackal_init_real@@YAHPAUPalette@@PAUFont@@PADHHHH@Z)(Palette* pPalEntries, Font* fontBuffer, int8* lpWindowName, int nDDTgl, int displayWidth, int displayHeight, int nCmdLineVal)
// callers   1   call targets   19
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005DF570 0x005EFD20 0x005F01F0 0x005F2C40 0x005FE330 0x005FE460 0x005FEBB0 0x00604590 0x006074B0 0x0060E4D0 0x0060FC60 0x00614D90 0x00616880 0x006168F0 0x006195B0 0x006339A0 0x0063B910 0x0063B940 0x0063CE20
// indirect  0x0062D42A
Return Value: Zero once every subsystem is up; the first refusal's code
              otherwise, or 3 if the palette or font is missing
Status: Complete
*/
int __cdecl jackal_init_real(Palette *palette, Font *font, LPSTR window_name,
                             int tgl_direct_draw, int display_width,
                             int display_height, int colour_depth) {
    if (font == 0 || palette == 0) {
        return 3;
    }

    JackalInitFlags = tgl_direct_draw;
    Palette::init_palette_class(tgl_direct_draw & 2);
    PaletteCurrent = palette;
    palette->init();
    palette->set();

    int result = Win::init_class(window_name);
    if (result != 0) {
        return result;
    }
    palette->set();

    if (tgl_direct_draw & 4) {
        Win::set_display_mode(display_width, display_height, colour_depth, 1);
        ShowWindow(HandleMain, 5);
        Win::flip(0);
    }

    result = Buffer::init_class();
    if (result != 0) {
        return result;
    }
    result = CheckBox::init_class();
    if (result != 0) {
        return result;
    }
    result = CheckButton::init_class();
    if (result != 0) {
        return result;
    }
    result = RadioButton::init_class();
    if (result != 0) {
        return result;
    }

    BaseButton::set_def_font(font, 0, 0);
    result = Font::init_font_class(font);
    if (result != 0) {
        return result;
    }
    result = BasePop::init_class();
    if (result != 0) {
        return result;
    }
    result = FileWin::init_class();
    if (result != 0) {
        return result;
    }
    result = Sprite::init_class();
    if (result != 0) {
        return result;
    }

    // THE DEFAULT WINDOW FONT (win.h's WinDynamicDefaults.default_font_,
    // 0x009B7AF4): every Win the game opens takes its field_F8_ font from
    // this slot until config_popups re-points it at the popup 12px font.
    // It used to be a second object named jackal_unk_9b7af4 - a duplicate
    // of the defaults table's own storage - so the font never flowed.
    WinDynamicDefaults.default_font_ = FontDefault;
    trig_init();
    result = Cursor::init_cursor_class();
    if (result != 0) {
        return result;
    }

    if (StringTable.init(0x8000) != 0) {
        return 1;
    }

    result = Time::init_class();
    if (result != 0) {
        return result;
    }

    JackalInitFlags |= 1;
    return 0;
}

/*
Purpose: Tear down every subsystem jackal_init_real brought up, in the
         image's own order, then clear the "initialised" bit.
// ORIGINAL: 0x0062D500 ?jackal_close@@YAXXZ 0x0062D500-0x0062D562 BYTE_EXACT
// size      98 bytes
// prototype void (__cdecl ?jackal_close@@YAXXZ)()
// callers   1   call targets   15
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DF580 0x005F04E0 0x005FECF0 0x00604680 0x0060E5D0 0x0060FD60
//           0x00614E30 0x00616890 0x00616950 0x00619610 0x0062D100 0x006339B0
//           0x00635750 0x0063B930 0x0063CEF0
Return Value: n/a
Status: Complete
*/

void __cdecl jackal_close() {
    sub_62d100();
    Time::close_class();
    WinDisplayInit.teardown();
    CheckButton::close_class();
    buffer_close_class_redirect();
    teardown_0060fd60();
    teardown_0060e5d0();
    Font::close_font_class();
    Sprite::close_class();
    FileWin::close_class();
    BasePop::close_class();
    Cursor::close_cursor_class();
    Win::close_class();
    Palette::close_palette_class();
    StringTable.shutdown();
    JackalInitFlags &= ~1;
}
