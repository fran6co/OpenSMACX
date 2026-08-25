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
#pragma once

 /*
  * Font class
  */
class Font {
 public:
  // EVIDENCE-FREE, measured 2026-08-25 rather than left unfinished. The
  // image records no caller for 0x00618F30; the mangled name is `QAE`, so
  // it is not virtual and no dispatch names it either; and the body is a
  // stub - `mov eax,1; ret 0x10`, four arguments discarded. Nothing here
  // can say what it was called, and a name invented to clear a census is a
  // false statement that outlives whoever wrote it.
  int UNK1(int, int, int, int);
  Font(); // 00618EA0
  Font(LPSTR font_name, int height, int style); // 00618EC0
  ~Font(); // 00618EE0

  // int UNK1(int, int, int, int) { return 1; } // no direct references
  int init(LPCSTR font_name, int height, int style);
  int init(LPCSTR file, LPCSTR font_name, int height, int style);
  // IN-CLASS so `~Font` inlines it, which is what the image does.
  MEASURED void close() {        // 00619230
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
  int width(LPSTR input);
  // IN-CLASS so `find_line_break_l` inlines it twice - the image's own
  // `calls` list never names 0x006192F0, only the WinAPI it reaches
  // through and the two `strlen` calls its own clamp makes.
  MEASURED int width(LPSTR input, size_t max_len) {   // 006192F0
    if (!input) {
      return 0;
    }
    // `strlen` TWICE, and SIGNED. The image's clamp is a min MACRO that
    // re-evaluates its argument - `call strlen; cmp ebx, eax; jl;
    // call strlen` - so caching it in a local collapses six instructions
    // into three. `jl`, not `jbe`, because both operands are int there; a
    // `size_t` comparison emits the unsigned branch.
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
  LPSTR find_line_break_l(LPSTR input, int *break_len, size_t len);
  bool is_initialized() const { return font_obj_ != NULL; }

  // eventually make atomic for thread safety
  static HDC FontHDC;
  static int FontInitCount;
  static int __cdecl init_font_class(Font *font);
  static void __cdecl close_font_class();

 private:
  // Buffer::text_height reads the resolved font's height directly.
  friend class Buffer;
  // Menu::requested_height reads the resolved font's height directly.
  friend class Menu;
  // `Win::flip` derives the bubble's line advance from these - `unk_1_`
  // when it is negative, `height_ + unk_1_` when it is not.
  friend class Win;

 public:
  // PUBLIC because the image writes it from outside: StatusWin sets
  // font1_/font2_/font3_'s first member to 4 when the screen is not 800
  // wide (`*(int *)(self + 0x1530) = 4` before this was named). That is
  // the access this comment already described, now spelled so the
  // caller can be written honestly. Access control is compile-time only,
  // so nothing about the layout or the emitted bytes moves.
  int unk_1_;  // 0x0000, height offset - StatusWin sets it through
               // font1_/font2_/font3_'s own starts, which is where the
               // offset comes from. The walk from here closes exactly on
               // sizeof(Font) == 0x28.
 private:
  BOOL is_fot_set_; // used only by both init() functions
  HFONT font_obj_;
  int line_height_;
  int height_;
  int internal_leading_;
  int ascent_;
  int descent_;
  int pad_; // padding? no references
  LPSTR fot_file_name_;
};

// 0x28, and statuswin.h is the evidence rather than a count of members:
// it embeds three consecutive Fonts at 0x1530, 0x1558 and 0x1580, so the
// stride between them IS sizeof(Font). Asserted so the compiler checks it
// and tools/header_offsets.py can walk THROUGH an embedded Font.
static_assert(sizeof(Font) == 0x28, "Font must match the embedded stride in StatusWin");

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Font) == 0x28, "Font layout must match the legacy ABI");

// Twelve candidate point sizes and the Font table they index, both owned by
// the original image.
int *const FontSizeTable = (int *)0x0068F220;
Font *const FontTable = (Font *)0x0093FC58;
static const size_t FontSizeTableCount = 12;

/*
 * FontQueue - three Font slots torn down together.
 *
 * The destructor walks 3 elements of 0x28 bytes from +0 through the CRT
 * vector iterator with the Font destructor as the per-element teardown, which
 * both types the slots and confirms sizeof(Font) = 0x28. Nothing pins more
 * than the 0x78 the walk covers.
 */
class FontQueue {
 public:
  FontQueue();
  ~FontQueue();

 private:
  // Font fonts_[3], not 3*0x28 raw bytes. Declaring the real array is what
  // makes VC6 emit the CRT vector ctor/dtor iterators itself; hand-calling
  // them meant passing `Font::Font` and `Font::~Font` as `const void *`
  // address constants, which C++ cannot even spell as function references.
  Font fonts_[3];

  // Three parallel per-slot arrays, not 9 scalars: the constructor at
  // 0x00559290 walks them with ONE index and a single stride-4 pointer -
  // `base = this + 0x84; for (i = 0; i < 3; i++) { base[-0xc] = -999;
  // base[0] = 0; base[0xc] = i; base += 4; }` - which only makes sense as
  // three int[3] arrays 0xC (3 * sizeof(int)) apart, at 0x78/0x84/0x90.
  int slot_age_[3];         // 0x78, seeded -999 (never-used sentinel)
  uint32_t slot_unused_[3]; // 0x84, seeded 0
  uint32_t slot_index_[3];  // 0x90, seeded to its own slot index
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(FontQueue) == 0x9C, "FontQueue layout must match the legacy ABI");
#endif

#include "vector_teardown.h"
extern const void *const FontQueueElementTeardown;


Font *__cdecl find_font(int size, int style);
#endif

// global
extern Font *FontDefault;

/*
 * The face name every `Font::init` call site passes when it does not name one
 * of its own. WinMain passes it as the FACE while naming "arialn.ttf" as the
 * FILE, which is why the four-argument `init` overload exists.
 *
 * ITS VALUE IS RECOVERED, NOT POINTED AT. The variable is at 0x00691B2C and
 * the image STORES its initial value there - `.data` is stored in the file up
 * to 0x006A8000 and this is below that line - so the four bytes read
 * 0x00691BD4, which is the literal "Arial". That is a fact about the shipped
 * binary and it belongs in the source as the string it is.
 *
 * The pointer-to-fixed-address spelling this replaces also compiled wrong:
 * WinMain's `mov ecx, [0x691b2c]` is ONE load of a `char *` variable, and
 * `*DefaultFontFace` through an `LPCSTR *` is two.
 */
extern LPCSTR DefaultFontFace;

