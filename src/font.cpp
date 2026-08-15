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
#include "temp.h" // BufferStrHeight
#include "font.h"
#include "general.h"

HDC Font::FontHDC;
int Font::FontInitCount;

LPCSTR *DefaultFontFace = reinterpret_cast<LPCSTR *>(0x00691B2C);

/*
Purpose: Initialize the class using the font name, height and style.
ORIGINAL: 0x00618F40
// name      ?init@Font@@QAEHPADHH@Z
// size      390 bytes
// spans     0x00618F40-0x006190C6
// prototype int (__thiscall ?init@Font@@QAEHPADHH@Z)(Font* this, int8* fontName, int height, int style)
// callers   49   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2 0x00645470
// indirect  0x00618F83 0x00618F94 0x00618FC3 0x00619043 0x0061906C 0x0061907A 0x006190AB 0x006190B9
Return Value: Zero on success, non-zero on error
Status: Complete
*/
int Font::init(LPCSTR font_name, int height, int style) {
    if (!font_name) {
        return 3;
    }
    if (is_fot_set_ & 1) {
        line_height_ = 0;
        height_ = 0;
        ascent_ = 0;
        descent_ = 0;
        if (font_obj_) {
            DeleteObject(font_obj_);
            font_obj_ = 0;
        }
    } else {
        close();
    }
    LOGFONT lf;
    lf.lfHeight = -height;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfUnderline = (style >> 2) & 1;
    lf.lfOrientation = 0;
    lf.lfWeight = (style & 1) ? 700 : 0;
    lf.lfItalic = (style >> 1) & 1;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = 0;
    lf.lfOutPrecision = 7;
    lf.lfClipPrecision = 0;
    lf.lfQuality = 0;
    lf.lfPitchAndFamily = 0;
    strcpy_s(lf.lfFaceName, 32, font_name);
    font_obj_ = CreateFontIndirectA(&lf);
    if (!font_obj_) {
        return 13;
    }
    SelectObject(FontHDC, font_obj_);
    TEXTMETRIC tm;
    GetTextMetricsA(FontHDC, &tm);
    internal_leading_ = tm.tmInternalLeading;
    line_height_ = tm.tmHeight + tm.tmExternalLeading;
    ascent_ = tm.tmAscent;
    height_ = ascent_ - internal_leading_;
    descent_ = tm.tmDescent;
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    return 0;
}

/*
Purpose: Initialize the class using the file, font name, height and style.
ORIGINAL: 0x006190D0
// name      ?init@Font@@QAEHPADPADHH@Z
// size      348 bytes
// spans     0x006190D0-0x0061922C
// prototype int (__thiscall ?init@Font@@QAEHPADPADHH@Z)(Font* this, int8* lpszFile, int8* fontName, int height, int style)
// callers   1   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510 0x00618F40 0x00644EF2 0x006453E0 0x00645470
// indirect  0x006190F8 0x00619109 0x006191A0 0x006191B1 0x006191BB 0x006191C5 0x006191E6
Return Value: Zero on success, non-zero on error
Status: Complete
*/
int Font::init(LPCSTR file, LPCSTR font_name, int height, int style) {
    close();
    if (!file || !font_name) {
        return 16;
    }
    size_t len = strlen(file) + 1;
    fot_file_name_ = (LPSTR)mem_get(len);
    if (!fot_file_name_) {
        return 4;
    }
    strcpy_s(fot_file_name_, len, file);
    fot_file_name_[len - 5] = 0; // font extension length + 1
    strcat_s(fot_file_name_, len, ".fot");
    char path[MAX_PATH + 1];
    GetCurrentDirectoryA(MAX_PATH, path);
    CreateScalableFontResourceA(0, fot_file_name_, file, path);
    if (!AddFontResourceA(fot_file_name_)) {
        GetLastError();
        return 1;
    }
    PostMessageA(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
    is_fot_set_ |= 1;
    return init(font_name, height, style);
}

/*
Purpose: Close an instance of the class.
ORIGINAL: 0x00619230
// name      ?close@Font@@QAEXXZ
// size      77 bytes
// spans     0x00619230-0x0061927D
// prototype void (__thiscall ?close@Font@@QAEXXZ)(Font* this)
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x00619250 0x00619261
Return Value: n/a
Status: Complete
*/
void Font::close() {
    unk_1_ = -1;
    line_height_ = 0;
    height_ = 0;
    ascent_ = 0;
    descent_ = 0;
    if (font_obj_) {
        DeleteObject(font_obj_);
        font_obj_ = 0;
    }
    if (fot_file_name_) {
        RemoveFontResourceA(fot_file_name_);
        free(fot_file_name_); // removed 2nd redundant check if fot_file_name_ isn't null
        fot_file_name_ = 0;
    }
}

/*
Purpose: Get the width for the input text.
ORIGINAL: 0x00619280
// name      ?width@Font@@QAEHPAD@Z
// size      99 bytes
// spans     0x00619280-0x006192E3
// prototype int (__thiscall ?width@Font@@QAEHPAD@Z)(Font* this, int8*)
// callers   15   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0
// indirect  0x006192A7 0x006192BF 0x006192C7 0x006192D5
Return Value: Width otherwise zero on error
Status: Complete
*/
int Font::width(LPSTR input) {
    if (!input) {
        return 0;
    }
    SelectObject(FontHDC, font_obj_);
    SIZE size;
    GetTextExtentPoint32A(FontHDC, input, strlen(input), &size);
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    return size.cx;
}

/*
Purpose: Get the width for the input text with a maximum length.
ORIGINAL: 0x006192F0
// name      ?width@Font@@QAEHPADH@Z
// size      123 bytes
// spans     0x006192F0-0x0061936B
// prototype int (__thiscall ?width@Font@@QAEHPADH@Z)(Font* this, int8* lpString, int max)
// callers   3   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006453E0
// indirect  0x00619337 0x00619346 0x0061934E 0x0061935C
Return Value: Width otherwise zero on error
Status: Complete
*/
int Font::width(LPSTR input, size_t max_len) {
    if (!input) {
        return 0;
    }
    size_t len = strlen(input);
    if (len > max_len) {
        len = max_len;
    }
    SelectObject(FontHDC, font_obj_);
    SIZE size;
    GetTextExtentPoint32A(FontHDC, input, len, &size);
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    return size.cx;
}

/*
Purpose: Find a space in the input string that can be used as a natural line break.
ORIGINAL: 0x00619370
// name      ?find_line_break_l@Font@@QAEHPADPAHH@Z
// size      567 bytes
// spans     0x00619370-0x006195A7
// prototype int (__thiscall ?find_line_break_l@Font@@QAEHPADPAHH@Z)(Font* this, int8* input, int* breakLen, int len)
// callers   1   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006453E0 0x006473F0
// indirect  0x006193EB 0x006193FA 0x00619402 0x00619410 0x006194AD 0x006194BC 0x006194C4 0x006194D2
Return Value: Pointer to string section after the line break or NULL if not found
Status: Complete
*/
LPSTR Font::find_line_break_l(LPSTR input, int *break_len, size_t len) {
    int search_brk = *break_len;
    LPSTR search_str = input;
    if (!len) {
        return NULL;
    }
    do {
        LPSTR space = (LPSTR)memchr(search_str + 1, ' ', len);
        if (!space) {
            search_brk -= width(search_str, len);
            if (search_brk < 0) {
                *break_len = 0;
                if (search_str != input) {
                    return search_str + 1;
                }
                return *BufferStrHeight ? search_str : NULL;
            }
            break;
        }
        search_brk -= width(search_str, space - search_str);
        if (search_brk < 0) {
            *break_len = 0;
            if (search_str != input) {
                return search_str + 1;
            }
            return *BufferStrHeight ? search_str : space + 1;
        }
        len += search_str - space;
        search_str = space;
    } while (len);
    *BufferStrHeight += *break_len - search_brk;
    *break_len = search_brk;
    return NULL;
}

/*
Purpose: Initialize the class static variables.
ORIGINAL: 0x006195B0
// name      ?init_font_class@Font@@QAAHPAUFont@@@Z
// size      93 bytes
// spans     0x006195B0-0x0061960D
// prototype 
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00618F40
// indirect  0x006195D2
Return Value: Zero on success (or already initialized), non-zero on error
Status: Complete
*/
int __cdecl Font::init_font_class(Font *font) {
    if (++FontInitCount > 1) {
        return 0;
    }
    if (!font) {
        return 3;
    }
    FontHDC = CreateCompatibleDC(NULL);
    if (!FontHDC) {
        return 2;
    }
    *FontDefaultPtr = font;
    if (font->font_obj_) {
        return 0;
    }
    return font->init("Times New Roman", 12, 0);
}

/*
Purpose: Shutdown the class static variables.
ORIGINAL: 0x00619610
// name      ?close_font_class@Font@@QAAXXZ
// size      52 bytes
// spans     0x00619610-0x00619644
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00619629
Return Value: n/a
Status: Complete
*/
void __cdecl Font::close_font_class() {
    if (--FontInitCount <= 0) {
        if (FontHDC) {
            DeleteDC(FontHDC);
            FontHDC = 0;
        }
        *FontDefaultPtr = NULL;
    }
}

// global
