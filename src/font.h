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
class DLLEXPORT Font {
 public:
  int UNK1(int, int, int, int);
  Font(); // 00618EA0
  Font(LPSTR font_name, int height, int style); // 00618EC0
  ~Font(); // 00618EE0

  // int UNK1(int, int, int, int) { return 1; } // no direct references
  int init(LPCSTR font_name, int height, uint32_t style);
  int init(LPCSTR file, LPCSTR font_name, int height, uint32_t style);
  void close();
  int width(LPSTR input);
  int width(LPSTR input, size_t max_len);
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

  int unk_1_; // height offset? set outside of class functions
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

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Font) == 0x28, "Font layout must match the legacy ABI");

// Twelve candidate point sizes and the Font table they index, both owned by
// the original image.
extern int *FontSizeTable;
extern Font *FontTable;
constexpr size_t FontSizeTableCount = 12;

/*
 * FontQueue - three Font slots torn down together.
 *
 * The destructor walks 3 elements of 0x28 bytes from +0 through the CRT
 * vector iterator with the Font destructor as the per-element teardown, which
 * both types the slots and confirms sizeof(Font) = 0x28. Nothing pins more
 * than the 0x78 the walk covers.
 */
class DLLEXPORT FontQueue {
 public:
  FontQueue() { ; }
  ~FontQueue();

 private:
  uint8_t fonts_[3 * 0x28];

  // Storage the image proves is here: its own methods reach 0x9C.
  // Extent only - nothing here is named, and this class carries no size assertion.
  uint8_t field_78_[0x24];
};

#include "vector_teardown.h"
extern func_thiscall_teardown FontQueueElementTeardown;

void __fastcall font_queue_dtor_redirect(FontQueue *self, void *);

DLLEXPORT Font *__cdecl find_font(int size, int style);
#endif

// global
extern Font **FontDefaultPtr;

int __fastcall font_unk1_redirect(
    Font *self, void *, int a, int b, int c, int d);
