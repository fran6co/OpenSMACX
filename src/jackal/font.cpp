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
#include "buffer.h"
#include "font.h"
#include "general.h"

// These three process-wide Font objects have independent, loader-zeroed identities.
// Font::init_font_class publishes FontDefault/HDC and adjusts the shared count;
// close_font_class clears the HDC/default pointer while preserving its signed
// counter and failure behavior. Address adjacency does not prove an aggregate.
// RECOVERY_OBJECT: v2 0x009BB488 0x009BB48C ?FontHDC@Font@@2PAUHDC__@@A 0x006195B0
HDC Font::FontHDC;
// RECOVERY_OBJECT: v2 0x009BB48C 0x009BB490 ?FontInitCount@Font@@2HA 0x006195B0
int Font::FontInitCount;

/*
Purpose: Field 0 as an offset into field 0x10, or field 0xc when it is negative.

             mov edx,[ecx] / test edx,edx / jl other
             mov eax,[ecx+0x10] / add eax,edx / ret
      other: mov eax,[ecx+0xc] / ret

         `jl` is the SIGNED test, so the negative branch is taken on the sign
         bit rather than on a large unsigned value - the difference shows up
         for any field with the top bit set.

// ORIGINAL: 0x005E3650 sub_5e3650 0x005E3650-0x005E3660 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x005E3650-0x005E3660 reads this+0, this+0x0C, and this+0x10 with signed jl", "source:src/jackal/font.h Font::unk_1_, line_height_, and height_ are the named members at those offsets", "pinned callers in Buffer text layout invoke this metric operation"], "reason": "Bind the original signed Font metric selector to its existing named member operation, retaining native 32-bit addition wrap."}
// symbol    ?line_advance@Font@@QBEHXZ
// size      16 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: field 0x10 + field 0, or field 0xc
Status: Complete
*/

int Font::line_advance() const {
    const int adjustment = unk_1_;
    if (adjustment >= 0) {
        const int height = height_;
        return static_cast<int32_t>(static_cast<uint32_t>(height) +
                                    static_cast<uint32_t>(adjustment));
    }
    return line_height_;
}




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
// Historical context: the following LEVER records the original in-class
// source form and its measured image behavior; it is not the current helper
// name or mapped definition location.
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
// LEVER: else-arm store order (height_ before line_height_, matching the
//        if-arm and the constructor - see the comment at that assignment)
//        took this 95/130 -> 97/130. Reordering `lf.lfWeight`/`lfItalic`/
//        `lfUnderline` to the real Win32 LOGFONT struct declaration order
//        (Escapement, Orientation, Weight, Italic, Underline - the source
//        had Underline computed before Weight/Italic) matched the image's
//        own bit-extraction order and took it to 110/130. Casting `style`
//        to `unsigned` for the `>>1`/`>>2` bit extractions gets the image's
//        `shr` where a plain `int style` compiles `sar` - 112/130.
// TRIED: the residual gap is the ascent_/internal_leading_/height_
//        store order - the image defers internal_leading_'s store past
//        ascent_'s despite reading it earlier. Caching `tm.tmAscent` in a
//        local, and reordering the source statements to the image's own
//        store sequence, both compile to the identical 112/130: this is
//        the optimizer's instruction scheduling, not a source-shape choice
//        reachable from here.
// LEVER: the trailing SelectObject moves BETWEEN `height_ = ...` and
//        `internal_leading_ = ...`. That is the one source-shape choice the
//        earlier pass missed: it makes VC6 hoist the `push 0xd`
//        (GetStockObject's SYSTEM_FONT argument) above the two remaining
//        member stores, which is exactly where the image has it -
//        0x006190A0 sits between `sub edx, ecx` and `mov [esi+0x10], edx`.
//        112/130 -> 117/130, and the compiled count now equals the image's
//        (130). The two member stores the statement jumps over are still
//        ordered height_ then internal_leading_ then descent_ in the image,
//        so the reordering is a scheduling nudge, not a semantic change.
// TRIED: the last 13 - the image reads tmDescent BEFORE it computes height_
//        (its `mov edx, eax` copy of tmAscent exists only because eax is
//        already spoken for), so spelling that with an `int descent` local
//        was the obvious next move. It SPILLS: hoisting the read to just
//        before `height_` or to the top of the block both compile to
//        97/130 (compiled 132 - two extra spills). `int leading` alone
//        stays at 117/130 and is what is here.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
int Font::init(LPCSTR font_name, int height, int style) {
    if (!font_name) {
        return 3;
    }
    if (!(is_fot_set_ & 1)) {
        // The close() teardown (0x00619230) is hand-inlined here: the image
        // writes its whole body out at this call site
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
        // IMAGE ORDER: height_ before line_height_, same as the if-branch
        // above - [esi+0x10] (height_) is stored before [esi+0xc]
        // (line_height_) in the image.
        height_ = 0;
        line_height_ = 0;
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
    lf.lfOrientation = 0;
    lf.lfWeight = (style & 1) ? 700 : 0;
    // Unsigned shift: the image's `shr`, not `sar` - style's sign bit does
    // not matter for a single-bit test, but VC6 picks the shift form from
    // the operand's declared signedness, so the cast is what selects it.
    lf.lfItalic = (static_cast<unsigned>(style) >> 1) & 1;
    lf.lfUnderline = (static_cast<unsigned>(style) >> 2) & 1;
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
    // TRIED: the image's remaining STORE order here - line_height_,
    // ascent_, internal_leading_, height_ - defers internal_leading_'s
    // store past ascent_'s even though it is read earlier. Both an
    // `internal_leading_`-first source order and a reordered one matching
    // the image's store sequence compile to the identical 112/130; this is
    // the optimizer's own instruction scheduling, not a source-shape one.
    // The trailing SelectObject sits BETWEEN the height_ store and the
    // internal_leading_/descent_ stores. That is what makes VC6 hoist the
    // `push 0xd` for GetStockObject above the two remaining member stores,
    // which is where the image has it (0x006190A0, between `sub edx, ecx`
    // and `mov [esi + 0x10], edx`) - 112/130 with SelectObject last,
    // 117/130 with it here. Everything else about the block is scheduling:
    // TRIED below.
    // Pinned-call correction: 0x006190A0 pushes SYSTEM_FONT as argument setup;
    // GetStockObject is actually called at 0x006190AB and SelectObject at
    // 0x006190B9. The image publishes all five metrics before either call.
    int leading = tm.tmInternalLeading;
    line_height_ = tm.tmHeight + tm.tmExternalLeading;
    ascent_ = tm.tmAscent;
    height_ = ascent_ - leading;
    internal_leading_ = leading;
    descent_ = tm.tmDescent;
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    return 0;
}

/*
Purpose: Initialize the class using the file, font name, height and style.
// ORIGINAL: 0x006190D0 ?init@Font@@QAEHPADPADHH@Z 0x006190D0-0x0061922C BYTE_EXACT
// symbol    ?init@Font@@QAEHPBD0HH@Z
// size      348 bytes
// prototype int (__thiscall ?init@Font@@QAEHPADPADHH@Z)(Font* this, int8* lpszFile, int8* fontName, int height, int style)
// callers   1   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510 0x00618F40 0x00644EF2 0x006453E0 0x00645470
// indirect  0x006190F8 0x00619109 0x006191A0 0x006191B1 0x006191BB 0x006191C5 0x006191E6
// LEVER: `fot_file_name_[0] = 0; strcat(...)` in place of `strcpy_s`/
//        `strcat_s` - the image calls plain strcat (0x00645470) and
//        strlen (0x006453E0), never the bounded `_s` forms, which this
//        tree's `_s` calls were compiling as an inlined `rep movsd` copy
//        the image does not have. Took this from 10/119 to 49/119.
// LEVER: THREE shapes closed the last 49 -> 119. (1) `char path[MAX_PATH]`,
//        not MAX_PATH + 1 - the image's `sub esp, 0x104` is 260 bytes of
//        local and nothing else, so the array is exactly MAX_PATH wide and
//        GetCurrentDirectoryA(MAX_PATH, path) can write 261 into it (BUG IN
//        THE ORIGINAL, left alone deliberately). (2) The `len` local leaves:
//        the image measures the COPY it just built - `call strlen` on
//        fot_file_name_ at 0x00619179 before `mov [eax + edx - 4]` - so the
//        truncation index is `strlen(fot_file_name_) - 4`, and carrying
//        `strlen(file) + 1` in a register cost the image's bare
//        `inc eax; push eax` on the mem_get size. (3) The guard tests
//        `file` FIRST - 0x00619122 loads [esp+0x118] (the first stack arg)
//        before 0x00619131 loads [esp+0x11c] - see the TRIED line below.
// TRIED: STALE, superseded by lever (3) above. "`if (!file || !font_name)`
//        vs `if (!font_name || !file)` - both compile identically (VC6
//        canonicalises the `||`)" is WRONG: measured 2026-09-03, the
//        font_name-first spelling read [esp+0x11c] at instruction 30 where
//        the image reads [esp+0x118], and swapping the operands was part of
//        the 51 -> 119 move. The note's other claim stands - splitting the
//        guard into two separate `if`s is still worse.
Return Value: Zero on success, non-zero on error
Status: Complete
*/
int Font::init(LPCSTR file, LPCSTR font_name, int height, int style) {
    release_resources();
    if (!file || !font_name) {
        return 16;
    }
    fot_file_name_ = (LPSTR)mem_get(strlen(file) + 1);
    if (!fot_file_name_) {
        return 4;
    }
    // WRONG CALLEE: the image calls strcat (0x00645470), not strcpy - it
    // zeroes fot_file_name_[0] first and strcat()s onto it, the same
    // zero-then-strcat idiom as Font::init(name,height,style) above.
    fot_file_name_[0] = 0;
    strcat(fot_file_name_, file);
    // The image measures the COPY it just built (`call strlen` on
    // fot_file_name_ at 0x00619179, then `mov [eax + edx - 4]`), not the
    // source string - so there is no `len` local carried from the
    // allocation, and caching one costs the image's bare `inc eax; push eax`
    // on the mem_get size.
    fot_file_name_[strlen(fot_file_name_) - 4] = 0; // font extension length
    strcat(fot_file_name_, ".fot");
    // MAX_PATH, not MAX_PATH + 1: the image's frame is 0x104, which is 260
    // bytes of local and nothing else. GetCurrentDirectoryA(MAX_PATH, path)
    // can write 261 into it - BUG IN THE ORIGINAL, left alone deliberately.
    char path[MAX_PATH];
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

void Font::close() {        // 00619230
    // IMAGE ORDER: height_ before line_height_, as in the constructor.
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
      // THE SECOND CHECK IS IN THE IMAGE. It is redundant - nothing between
      // it and the first can null the pointer - and it was removed here as
      // dead code, which is exactly the kind of tidying a byte match cannot
      // afford.
      if (fot_file_name_) {
        free(fot_file_name_);
      }
      fot_file_name_ = 0;
    }
}


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
// ORIGINAL: 0x006192F0 ?width@Font@@QAEHPADH@Z 0x006192F0-0x0061936B BYTE_EXACT
// symbol    ?width@Font@@QAEHPADI@Z
// HISTORICAL MOVED IN-CLASS: `find_line_break_l` (0x00619370) used the
// source body twice inline; it now calls the private source-only helper, not
// an original mapped identity.
//        its own `calls` list never names 0x006192F0.
// LEVER: `strlen` called TWICE and compared SIGNED. The image's clamp is a min
//   MACRO that re-evaluates its argument - `call strlen; cmp ebx, eax; jl;
//   call strlen` at 0x0061930B and 0x0061931C - so caching it in a local
//   collapses six instructions into three. `jl` not `jbe`, because both
//   operands are `int` there; a `size_t` comparison emits the unsigned branch.
// LEVER: THE CLAMP IS SPELLED AS THE MACRO, and that is what closes the last
//   20 of 48. `int len = FONT_MIN(static_cast<int>(max_len),
//   static_cast<int>(strlen(input)));` with
//   `#define FONT_MIN(a, b) ((a) < (b) ? (a) : (b))` in font.h went 28/48 ->
//   BYTE_EXACT 48/48 in one change (the macro is defined just above the
//   class and #undef'd straight after, so it leaks nowhere). The
//   `if (len >= strlen) len = strlen` spelling computes `len` BEFORE the
//   first `strlen`, and that is what pushed VC6 into saving ebx for `this`
//   and edi for `len`; the macro reads `a` only for the comparison and
//   re-reads the parameter in the `b` arm, which hands `this` edi and `len`
//   ebx exactly as the image has them. An earlier pass had the right
//   diagnosis and the right 4/48 and 8/48 readings for the inline forms and
//   stopped one step short of writing the macro down.
// TRIED: the register allocation the previous refusal blamed was a symptom,
//   not the wall - measured 2026-09-03, same instruction count (48) on both
//   sides with only ebx/edi swapped, and no reordering of the `if` form
//   reaches the image's assignment.
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

#define FONT_MIN(a, b) ((a) < (b) ? (a) : (b))
int Font::width(LPSTR input, size_t max_len) {   // 006192F0
    if (!input) {
      return 0;
    }
    // `strlen` TWICE, and SIGNED. The clamp is a min MACRO that re-evaluates
    // its argument - `call strlen; cmp ebx, eax; jl; call strlen` - so the
    // macro form (which evaluates `b` once for the test and once for the
    // value) is what produces the image's two calls. `jl`, not `jbe`, because
    // both operands are int there; a `size_t` comparison emits the unsigned
    // branch.
    int len = FONT_MIN(static_cast<int>(max_len),
                       static_cast<int>(strlen(input)));
    SelectObject(FontHDC, font_obj_);
    SIZE size;
    GetTextExtentPoint32A(FontHDC, input, len, &size);
    SelectObject(FontHDC, GetStockObject(SYSTEM_FONT));
    return size.cx;
  }
#undef FONT_MIN

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
// Historical context: the following LEVER records the earlier in-class
// source arrangement; the call sites now use the private source-only helper.
// LEVER: `width(LPSTR, size_t)` moved IN-CLASS (font.h) so it inlines here
//        twice, matching the image's own `calls`/`indirect` lists (no
//        0x006192F0, but 8 indirect WinAPI calls and strlen showing up
//        instead) - `calls` lists unique callees, not call counts, which
//        is why one `0x006453E0` covers strlen from both inlined copies.
//        Moved compiled-instruction count from 107 to 174 against an image
//        of 208, much closer to the right shape.
// LEVER: the `len` guard is `if (len) { do ... }`, not `if (!len) return
//        NULL; do ...`. Measured 2026-09-03 from the listing: the image's
//        `je 0x619456` on `len == 0` lands INSIDE the shared loop-exit
//        epilogue, one instruction after the `mov eax, [esp+0x28]` that the
//        fall-out path needs and the guard path does not (eax still holds
//        break_len from instruction 1). A bare `return NULL` in the guard
//        emits a separate `xor eax,eax; ret 0xc` instead, which is a
//        different program shape for identical behaviour. The image's
//        back-edge is `jne 0x61939b`, i.e. into the body, not the `test` -
//        which is the do-while-inside-an-if shape, and the `jne` takes its
//        flags from the `add eax, esi` that writes `len` back.
// TRIED: the frame part of the old refusal is GONE - under the file's
//        `/c /O2 /Gy /GR- /GX` set this tree now opens `sub esp, 0x10` with
//        no `push ebp; mov ebp, esp`, and the first three instructions agree
//        with the image. What still diverges is instruction 3: the image
//        keeps `len` in MEMORY (it is written back to its own argument slot
//        at 0x00619448 and re-read at 0x0061937D/0x0061939B/0x00619476) and
//        spends its registers on `this` spilled to [esp+4], `search_brk`
//        spilled to [esp], `search_str` in esi, `space` in ebx and the
//        SelectObject import address hoisted into ebp at 0x00619395; this
//        tree register-allocates `len` (ebx) instead and re-loads the
//        SelectObject address at each call. That one allocation decision
//        desynchronises everything after it, so the scored agreement stays
//        at 4/208 although the two bodies are the same ~175 instructions.
//        Not reachable by reordering statements - `len` is a by-value
//        parameter modified once in the loop, and no source shape tried
//        makes VC6 leave it in its slot.
Return Value: Pointer to string section after the line break or NULL if not found
Status: Complete
*/
LPSTR Font::find_line_break_l(LPSTR input, int *break_len, size_t len) {
    int search_brk = *break_len;
    LPSTR search_str = input;
    // `if (len) { do ... }`, not `if (!len) return NULL; do ...`. The image
    // tests `len` once at the top and, when it is zero, jumps to the SAME
    // shared epilogue the loop falls out of (0x00619456, reached again from
    // 0x00619452) - `BufferStrHeight += *break_len - search_brk` with a zero
    // delta, `*break_len = search_brk`, return NULL. A bare `return NULL`
    // skips that epilogue and is a different program shape for the same
    // behaviour; it also costs the image's back-edge into 0x0061939B, which
    // is where the loop re-enters rather than the `test`.
    if (len) {
        do {
        LPSTR space = (LPSTR)memchr(search_str + 1, ' ', len);
        if (!space) {
            search_brk -= measure_bounded_width(search_str, len);
            if (search_brk < 0) {
                *break_len = 0;
                if (search_str != input) {
                    return search_str + 1;
                }
                return BufferStrHeight ? search_str : NULL;
            }
            break;
        }
        search_brk -= measure_bounded_width(search_str, space - search_str);
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
    }
    BufferStrHeight += *break_len - search_brk;
    *break_len = search_brk;
    return NULL;
}

/*
Purpose: Initialize the class static variables.
// ORIGINAL: 0x006195B0 ?init_font_class@Font@@QAAHPAUFont@@@Z 0x006195B0-0x0061960D BYTE_EXACT
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
    if (++FontInitCount <= 1) {
        if (!font) {
            return 3;
        }
        FontHDC = CreateCompatibleDC(NULL);
        if (!FontHDC) {
            return 2;
        }
        FontDefault = font;
        if (!font->font_obj_) {
            int const result = font->init("Times New Roman", 12, 0);
            if (result) {
                return result;
            }
        }
    }
    return 0;
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
    release_resources();
}

// 0x009BB484. The process default font: `Font::init_font_class` sets it,
// `close_font_class` clears it, and Buffer, Menu and PullDown fall back to
// it - 42 references. Past `.data`'s stored bytes, so there is no
// initialiser to preserve and a real object is the whole of it.
// RECOVERY_OBJECT: v2 0x009BB484 0x009BB488 ?FontDefault@@3PAVFont@@A 0x006195B0
Font *FontDefault;


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
