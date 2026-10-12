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

// The clamp in Font::width(LPSTR, size_t) is a MACRO, not an `if`: a macro
// re-evaluates its argument, and the image calls `strlen` twice because of
// it. This definition serves the private inline body below; font.cpp scopes
// an identical definition around the public mapped body.
#define FONT_MIN(a, b) ((a) < (b) ? (a) : (b))

 /*
  * Font class
  */
class Font {
 public:
  // UNK1 IS EVIDENCE-FREE, measured 2026-08-25 rather than left unfinished. The
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
 private:
  // Source-only cleanup helper. Its name is not an original mapped identity;
  // the public Font::close() mapping remains defined in font.cpp.
  void release_resources() {
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
 public:
  void close(); // 00619230
  int width(LPSTR input);
  // Source-only bounded-width helper used at both line-break sites. Its
  // private name is not an original mapped identity; Font::width remains
  // the public mapped operation defined in font.cpp.
 private:
  int measure_bounded_width(LPSTR input, size_t max_len) {
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
 public:
  int width(LPSTR input, size_t max_len); // 006192F0
  LPSTR find_line_break_l(LPSTR input, int *break_len, size_t len);
  bool is_initialized() const { return font_obj_ != NULL; }
  int descent() const { return descent_; }
  int pixel_height() const { return height_; }
  // InfoWin's panel builder (0x00456300) uses Font+0x0C independently
  // of the adjusted row pitch returned by line_advance().
  int line_height() const { return line_height_; }
  // PullDown mnemonic underline reads the native Font+0x14 metric.
  int internal_leading() const { return internal_leading_; }
  // PullDown::show (0x005F91A3) uses the natural line height unless
  // an explicit inter-line adjustment has been supplied.
  int line_advance() const;

  // eventually make atomic for thread safety
  static HDC FontHDC;
  static int FontInitCount;
  static int __cdecl init_font_class(Font *font);
  static void __cdecl close_font_class();

 private:
  // BaseButton::UNK1 sizes its label from the original Font height word.
  friend class BaseButton;
  friend class CheckBox;
  // Buffer::text_height reads the resolved font's height directly.
  friend class Buffer;
  // Menu::requested_height reads the resolved font's height directly.
  friend class Menu;
  // `Win::flip` derives the bubble's line advance from these - `unk_1_`
  // when it is negative, `height_ + unk_1_` when it is not.
  friend class Win;
  // `BasePop::calculate_dimensions` (0x00603800) reads button_font1_'s
  // height_ to scale the popup's row pitch (`mov edx,[eax+0x10]` at
  // 0x00603A50). Compile-time access only: no offset moves and no emitted
  // byte changes, the same edit this header made for unk_1_.
  friend class BasePop;

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

// The header's macro stops here rather than leaking into including TUs.
// The public body in font.cpp defines and undefines its own local copy.
#undef FONT_MIN

// 0x28, and statuswin.h is the evidence rather than a count of members:
// it embeds three consecutive Fonts at 0x1530, 0x1558 and 0x1580, so the
// stride between them IS sizeof(Font). The compiler checks this extent;
// native layout checks observe embedded Font members through debug types.
static_assert(sizeof(Font) == 0x28, "Font must match the embedded stride in StatusWin");

// global
extern Font *FontDefault;
