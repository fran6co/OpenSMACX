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
  * Text class: Handles basic text operations.
  */
// `mem_get` is general.h's, declared here because the constructor below is
// defined in-class and calls it. Spelled identically to general.h's line, or
// VC6 reports `C2373: redefinition; different type modifiers` on whichever
// declaration it sees second.
LPVOID __cdecl mem_get(size_t size);

// DECLARED BEFORE THE CLASS so the `friend` inside it refers to this
// namespace-scope function. VC6 injects a friend declared only in-class
// into CLASS scope, and then the definition below reads as a second body
// for a member: `C2084: function 'void Text::text_set_get_ptr_source'
// already has a body`.
class Text;
void __cdecl text_set_get_ptr_source(Text *text, LPSTR *output);
void __cdecl text_set_item_ptr_source(Text *text, LPSTR *output);

class Text {
 public:
  Text(); // 005FD860
  // DEFINED IN-CLASS so it can be INLINED, which is what the image does.
  // `??__ETxt@@YAXXZ` at 0x005FD400 - the dynamic initialiser for `Txt` -
  // writes these fields directly instead of calling 0x005FD880, and it can
  // only do that if the definition is visible where the object is
  // constructed. Measured on that initialiser: declared out of line it
  // diverges at instruction #1 with the constructor called; defined here it
  // reproduces 22 of 22 mnemonics, 87 bytes against 86, with ONE extra
  // instruction - VC6 materialises `this` into eax for the first store,
  // where the image addresses it absolutely, and no spelling of that store
  // moves it (array index, `*file_name_`, a cast of `this`, a local).
  //
  // `int`, NOT `size_t`. The image's own name for it is `??0Text@@QAE@H@Z`
  // and `H` is `int`; 0x005FD880 is BYTE_EXACT against a body taking one.
  MEASURED Text(int size) {  // 005FD880
    file_name_[0] = 0;
    current_pos_ = 0;
    text_file_ = 0;
    buffer_get_ = 0;
    buffer_item_ = 0;
    buffer_get_ = (LPSTR)mem_get(size);
    if (buffer_get_) {
      buffer_item_ = (LPSTR)mem_get(size);
    }
  }
  // IN-CLASS for the same reason as the constructor above: `??__FTxt` at
  // 0x005FD460 - the atexit thunk the compiler pairs with `??__ETxt` - runs
  // the destructor INLINE, doing the fclose and the frees itself with no
  // `call ??1Text@@QAE@XZ` anywhere in it.
  MEASURED ~Text() OPENSMACX_NOEXCEPT_FALSE { shutdown(); }   // 00608C00

  int init(size_t size);
  void shutdown();
  void close();
  BOOL open(LPCSTR src_id, LPCSTR section_id);
  LPSTR get();
  LPSTR string();
  LPSTR item();
  LPSTR item_string();
  int item_number();
  int item_binary();
  int item_hex();
  // additional functions to assist with encapsulation
  LPSTR update() { current_pos_ = buffer_get_; return buffer_get_; }
  LPSTR get_file_path() { return file_path_; }
  LPSTR get_buffer_item() { return buffer_item_; }
  LPSTR get_buffer_get() { return buffer_get_; }

  // THE RECOVERED FREE FUNCTIONS, as friends rather than through a mirror.
  //
  // `?text_open@@YAHPADPAD@Z` and its neighbours are free functions in the
  // image, not methods, so they cannot reach these members - and
  // src/text_recovery.cpp answered that with a private `struct TextState`
  // repeating the layout field for field, pinned by five `static_assert`s
  // and reached through a `reinterpret_cast`. Five asserts is a good mirror
  // and a mirror is still a second copy of a layout: it has to be corrected
  // twice, and the compiler cannot tell you the day it stops matching, only
  // the day a size does.
  friend void __cdecl text_close_source(Text *text);
  friend void __cdecl text_set_get_ptr_source(Text *text, LPSTR *output);
  friend void __cdecl text_set_item_ptr_source(Text *text, LPSTR *output);
  friend LPSTR __cdecl text_get_source(Text *text);
  friend LPSTR __cdecl text_item_source(Text *text);

 private:
  char file_name_[80];  // (+0)    : stores text filename string
  char file_path_[256]; // (+0x50) : stores path of last opened file
  LPSTR current_pos_;   // (+0x150)
  FILE *text_file_;     // (+0x154)
  LPSTR buffer_get_;    // (+0x158)
  LPSTR buffer_item_;   // (+0x15C)
};

// global
// `*const`, so VC6 can prove the placement in `??__ETxt` is not null.
// Through a plain `Text *` the placement `new (Txt) Text(512)` there emits a
// `cmp`/`je` guard the image does not have - 98 bytes against 86 - because
// the pointer is a variable it cannot fold. Nothing assigns to it.
// THE OBJECT, at 0x009B7BA0 in the shipped image.
//
// It was `Text *const Txt = (Text *)0x009B7BA0` - the injected-DLL spelling,
// naming storage inside a process this is no longer part of. That address is
// mapped in terranx.exe and in nothing this tree builds, so every `Txt->` in
// the recovered executable faults, and the program cannot reach its first
// line of text. A recovery that cannot run is not the goal.
//
// `Text Txt(512);` in text.cpp is what the image has, and declaring it is
// what produces BOTH catalogued initialisers: MSVC emits the constructor
// call and the `atexit` registration for any namespace-scope object with a
// non-trivial constructor and destructor. `??__ETxt` (0x005FD400) and
// `??__FTxt` (0x005FD460) are that pair. They are annotated on the
// definition itself, because there is no other source line they come from.
//
// WHAT IT COSTS, stated where the decision is: neither can ever be measured.
// VC6 names generated initialisers `_$E<n>` and registers them through
// `.CRT$XCU`; the `??__E`/`??__F` spellings are IDA's reconstruction of what
// they DO, and the byte match looks a subject up by name. The hand-written
// stand-ins that used to sit in text.cpp could be scored - one reached 97.8%
// - and were deleted with this change, because a hand-written copy of a
// function the compiler emits anyway is two definitions of one thing.
extern Text Txt;
// AN OBJECT, not a pointer to 0x009B7D00. That address is above the end
// of stored `.data` (0x006A8000), so it is zero-fill and the pointer form
// carried no information the object form does not - it only cost a load
// at each of its 26 uses, and dangles in an executable that is not the
// shipped image. `text_set_get_ptr` fills it from `Txt->buffer_get_`,
// which is why it already holds a heap pointer by WinMain's first
// prefs_get: 0x01A31BE0, read out of the running original.
extern LPSTR TextBufferGetPtr;
extern LPSTR TextBufferItemPtr;
void __cdecl text_txt();
void __cdecl text_txt_exit();
void __cdecl text_set_get_ptr();
void __cdecl text_set_item_ptr();
void __cdecl text_close();
BOOL __cdecl text_open(LPCSTR src_id, LPCSTR section_id);
LPSTR __cdecl text_get();
LPSTR __cdecl text_string();
LPSTR __cdecl text_item();
LPSTR __cdecl text_item_string();
int __cdecl text_item_number();
int __cdecl text_item_binary();
int __cdecl text_item_hex();

// Recovered free functions, formerly declared in the retired
// text_recovery.h. They are the image's own shapes - free functions taking
// a `Text *` - and several are friends of the class above.
class Strings;
void __cdecl text_close_source(Text *text);
// INLINE, because the image folds it: `text_set_get_ptr` is three
// instructions - load the field, store it, return - where a call leaves
// five. Same reason as Spot::shutdown and Font::close.
inline void __cdecl text_set_get_ptr_source(Text *text, LPSTR *output) {
  *output = text->buffer_get_;
}
inline void __cdecl text_set_item_ptr_source(Text *text, LPSTR *output) {
  *output = text->buffer_item_;
}
LPSTR __cdecl text_get_source(Text *text);
LPSTR __cdecl text_string_source(Text *text, Strings *strings);
LPSTR __cdecl text_item_source(Text *text);
LPSTR __cdecl text_item_string_source(Text *text, Strings *strings);
int __cdecl text_item_number_source(Text *text);
int __cdecl text_item_binary_source(Text *text);
int __cdecl text_item_hex_source(Text *text);
int __cdecl text_get_number_source(Text *text, int min, int max);

// Nothing in the image calls text_get_number: `inline_candidates` found no
// direct edge to it anywhere in the shipped bytes, so every caller has
// it written out. `MEASURED` keeps the standalone body measurable.
MEASURED inline int __cdecl text_get_number(int min, int max) {
    return text_get_number_source(&Txt, min, max);
}
