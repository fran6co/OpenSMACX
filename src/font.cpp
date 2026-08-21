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
#include "vector_teardown.h"
#include "dialog.h"
#include "sounddevice.h"
#include "spritebox.h"
#include "net_class.h"
#include "basepop.h"

HDC Font::FontHDC;
int Font::FontInitCount;

LPCSTR DefaultFontFace = "Arial";  // 0x00691B2C -> 0x00691BD4

/*
Purpose: Initialize the class using the font name, height and style.
// ORIGINAL: 0x00618F40 ?init@Font@@QAEHPADHH@Z 0x00618F40-0x006190C6
// symbol    ?init@Font@@QAEHPBDHH@Z
// size      390 bytes
// prototype int (__thiscall ?init@Font@@QAEHPADHH@Z)(Font* this, int8* fontName, int height, int style)
// callers   49   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2 0x00645470
// indirect  0x00618F83 0x00618F94 0x00618FC3 0x00619043 0x0061906C 0x0061907A 0x006190AB 0x006190B9
// LEVER: WRONG CALLEE, three parts. (1) close() (0x00619230, in-class
//        MEASURED method) hand-inlined at the `else` site - the image's
//        only calls here are free() (0x00644EF2, inside close()'s body)
//        and strcat (0x00645470), never a call to close() itself.
//        (2) strcat, not strcpy_s: the image zeroes lfFaceName[0] then
//        strcat()s font_name onto the empty string. (3) branch polarity -
//        the image falls THROUGH into the close()-teardown arm and jumps
//        to skip it, i.e. `if (!(is_fot_set_ & 1)) { close-teardown }
//        else { simple }`, not the other way around. All three together:
//        best similarity 0.958 (95/130), up from a WRONG CALLEE baseline.
// RULED-OUT: residual gap is LOGFONT bitfield computation order
//        (lfUnderline/lfItalic extraction, ascent_/height_/descent_ store
//        order) - register-scheduling noise not chased further here.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
int Font::init(LPCSTR font_name, int height, int style) {
    if (!font_name) {
        return 3;
    }
    if (!(is_fot_set_ & 1)) {
        // close() (0x00619230, in-class MEASURED method) is hand-inlined
        // here: the image writes its whole body out at this call site
        // (free(), the only call it makes, is the one it keeps), rather
        // than calling out to it. LEVER: guard clause polarity - the image
        // falls through into this (the `else` arm originally) rather than
        // jumping to it, so the condition is inverted to match.
        unk_1_ = -1;
        height_ = 0;
        line_height_ = 0;
        ascent_ = 0;
        descent_ = 0;
        if (font_obj_) {
            DeleteObject(font_obj_);
            font_obj_ = 0;
        }
        if (fot_file_name_) {
            RemoveFontResourceA(fot_file_name_);
            if (fot_file_name_) {
                free(fot_file_name_);
            }
            fot_file_name_ = 0;
        }
    } else {
        line_height_ = 0;
        height_ = 0;
        ascent_ = 0;
        descent_ = 0;
        if (font_obj_) {
            DeleteObject(font_obj_);
            font_obj_ = 0;
        }
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
    // WRONG CALLEE: the image calls strcat (0x00645470), not strcpy - it
    // zeroes lfFaceName[0] first (an empty string) and strcat()s onto it,
    // rather than strcpy_s()-ing directly.
    lf.lfFaceName[0] = 0;
    strcat(lf.lfFaceName, font_name);
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
// ORIGINAL: 0x006190D0 ?init@Font@@QAEHPADPADHH@Z 0x006190D0-0x0061922C
// symbol    ?init@Font@@QAEHPBD0HH@Z
// size      348 bytes
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
// ORIGINAL: 0x00619230 ?close@Font@@QAEXXZ 0x00619230-0x0061927D BYTE_EXACT
// body      src/font.h
// size      77 bytes
// prototype void (__thiscall ?close@Font@@QAEXXZ)(Font* this)
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x00619250 0x00619261
Return Value: n/a
Status: Complete
*/


/*
Purpose: Get the width for the input text.
// ORIGINAL: 0x00619280 ?width@Font@@QAEHPAD@Z 0x00619280-0x006192E3 BYTE_EXACT
// size      99 bytes
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
// ORIGINAL: 0x006192F0 ?width@Font@@QAEHPADH@Z 0x006192F0-0x0061936B
// symbol    ?width@Font@@QAEHPADI@Z
// size      123 bytes
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
    // `strlen` TWICE, and SIGNED. The image's clamp is a min MACRO that
    // re-evaluates its argument - `call strlen; cmp ebx, eax; jl; call strlen`
    // at 0x0061930B and 0x0061931C - so caching it in a local collapses six
    // instructions into three. `jl`, not `jbe`, because both operands are int
    // there; a `size_t` comparison emits the unsigned branch.
    int len = static_cast<int>(max_len);
    if (len >= static_cast<int>(strlen(input))) {
        len = static_cast<int>(strlen(input));
    }
    SelectObject(FontHDC, font_obj_);
    SIZE size;
    GetTextExtentPoint32A(FontHDC, input, len, &size);
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    return size.cx;
}

/*
Purpose: Find a space in the input string that can be used as a natural line break.
// ORIGINAL: 0x00619370 ?find_line_break_l@Font@@QAEHPADPAHH@Z 0x00619370-0x006195A7
// symbol    ?find_line_break_l@Font@@QAEPADPADPAHI@Z
// size      567 bytes
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
                return BufferStrHeight ? search_str : NULL;
            }
            break;
        }
        search_brk -= width(search_str, space - search_str);
        if (search_brk < 0) {
            *break_len = 0;
            if (search_str != input) {
                return search_str + 1;
            }
            return BufferStrHeight ? search_str : space + 1;
        }
        len += search_str - space;
        search_str = space;
    } while (len);
    BufferStrHeight += *break_len - search_brk;
    *break_len = search_brk;
    return NULL;
}

/*
Purpose: Initialize the class static variables.
// ORIGINAL: 0x006195B0 ?init_font_class@Font@@QAAHPAUFont@@@Z 0x006195B0-0x0061960D
// symbol    ?init_font_class@Font@@SAHPAV1@@Z
// size      93 bytes
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
    FontDefault = font;
    if (font->font_obj_) {
        return 0;
    }
    return font->init("Times New Roman", 12, 0);
}

/*
Purpose: Shutdown the class static variables.
// ORIGINAL: 0x00619610 ?close_font_class@Font@@QAAXXZ 0x00619610-0x00619644 BYTE_EXACT
// symbol    ?close_font_class@Font@@SAXXZ
// size      52 bytes
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
        FontDefault = NULL;
    }
}

// global

// ---------------------------------------------------------------------------
// Merged from the retired src/font_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00618EA0 ??0Font@@QAE@XZ 0x00618EA0-0x00618EC0 BYTE_EXACT
// size      32 bytes
// prototype void (__thiscall ??0Font@@QAE@XZ)(Font* this)
// callers   41   call targets   0
// kind      
// flags     
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
Font::Font() {
    // IMAGE ORDER. An initialiser list runs in DECLARATION order,
    // and the image does not write the fields in that order.
    unk_1_ = -1;
    is_fot_set_ = 0;
    font_obj_ = nullptr;
    height_ = 0;
    line_height_ = 0;
    ascent_ = 0;
    descent_ = 0;
    fot_file_name_ = nullptr;
}

/*
// ORIGINAL: 0x00618EC0 ??0Font@@QAE@PADHH@Z 0x00618EC0-0x00618EDF BYTE_EXACT
// size      31 bytes
// prototype void (__thiscall ??0Font@@QAE@PADHH@Z)(Font* this, LPSTR fontName, int height, int style)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00618F40
// notes     Staged hybrid export redirect calls the source-owned constructor
*/
Font::Font(LPSTR font_name, int height, int style) {
    init(font_name, height, static_cast<uint32_t>(style));
}

/*
// ORIGINAL: 0x00618EE0 ??1Font@@QAE@XZ 0x00618EE0-0x00618F2D BYTE_EXACT
// size      77 bytes
// prototype void (__thiscall ??1Font@@QAE@XZ)(Font* this)
// callers   53   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x00618F00 0x00618F11
// notes     Staged hybrid export redirect calls the source-owned destructor
*/
Font::~Font() {
    close();
}

// The per-element teardown the queue passes: the Font destructor, bound here
// under its own name so this file needs nothing from the generated thunk
// family. atexit_thunks.cpp binds the same address as FontElementTeardown.
const void *const FontQueueElementTeardown = (const void *)0x00618EE0;
// Its construction-side companion, likewise bound locally rather than
// pulling in init_thunks.h; init_thunks.cpp binds the same address as
// FontElementCtor.
const void *const FontQueueElementCtor = (const void *)0x00618EA0;

/*
Purpose: Construct the queue: hand the three-slot walk to the CRT vector
         iterator with the Font constructor/destructor pair, then seed the
         three parallel per-slot arrays that follow the Font storage.
// ORIGINAL: 0x00559290 ??0FontQueue@@QAE@XZ 0x00559290-0x005592CD BYTE_EXACT
// size      61 bytes
// prototype void (__thiscall ??0FontQueue@@QAE@XZ)(FontQueue* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006457C2
Return Value: n/a
Status: Complete
*/
FontQueue::FontQueue() {
    VectorCtorIterator(this, 0x28, 3, FontQueueElementCtor, FontQueueElementTeardown);
    for (int i = 0; i < 3; i++) {
        slot_age_[i] = -999;
        slot_unused_[i] = 0;
        slot_index_[i] = i;
    }
}

/*
Purpose: Destroy the queue: hand the three-slot walk to the CRT vector
         iterator with the queue itself as the array base and the Font
         destructor as the per-element teardown.
// ORIGINAL: 0x0055B740 ??1FontQueue@@QAE@XZ 0x0055B740-0x0055B750 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ??1FontQueue@@QAE@XZ)(FontQueue* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
FontQueue::~FontQueue() {
    VectorDtorIterator(this, 0x28, 3, FontQueueElementTeardown);
}

void __fastcall font_queue_dtor_redirect(FontQueue *self, void *) {
    self->~FontQueue();
}

// 0x009BB484. The process default font: `Font::init_font_class` sets it,
// `close_font_class` clears it, and Buffer, Menu and PullDown fall back to
// it - 42 references. Past `.data`'s stored bytes, so there is no
// initialiser to preserve and a real object is the whole of it.
Font *FontDefault;

/*
Purpose: Select the Font whose point size is closest to the requested one,
         then index that size's row by style.
// ORIGINAL: 0x005882F0 ?find_font@@YAHHH@Z 0x005882F0-0x0058833B BYTE_EXACT
// symbol    ?find_font@@YAPAVFont@@HH@Z
// size      75 bytes
// prototype 
// callers   31   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
Font *__cdecl find_font(int size, int style) {
    int best_index = 0;
    // The seed doubles as a threshold: a requested size further than 9999 from
    // every candidate matches nothing and falls back to the first entry.
    int best_delta = 9999;
    for (size_t index = 0; index < FontSizeTableCount; ++index) {
        const int candidate = FontSizeTable[index];
        // Absolute difference, branching on sign exactly as the legacy body.
        int delta = candidate - size;
        if (delta < 0) {
            delta = size - candidate;
        }
        if (delta < best_delta) {
            best_delta = delta;
            best_index = static_cast<int>(index);
        }
    }
    // Each size occupies four consecutive style slots in the table.
    return FontTable + (style + best_index * 4);
}

/*
Purpose: Legacy stub; the original body returns 1 without reading its
         arguments.
// ORIGINAL: 0x00618F30 ?UNK1@Font@@QAEHHHHH@Z 0x00618F30-0x00618F38 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK1@Font@@QAEHHHHH@Z)(Font* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Font::UNK1(int, int, int, int) {
    return 1;
}

int __fastcall font_unk1_redirect(
        Font *self, void *, int a, int b, int c, int d) {
    return self->UNK1(a, b, c, d);
}
