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
#include "heap.h"

/*
* TextIndex class: Designed to speed up the time it takes to find string resources in text files by 
* creating an index of the file sections prefixed by '#'.
*/
class TextIndex {
 public:
  TextIndex(); // 005FDF40
  ~TextIndex(); // 005FDF60
  
  void shutdown() { section_count_ = 0; file_name_[0] = 0; heap_.shutdown(); }
  void make_index(LPCSTR source_txt);
  int search_index(LPCSTR source_txt, LPCSTR section_txt);
  // additional functions to assist with encapsulation
  uint32_t get_count() { return section_count_; }

 private:
  friend void text_clear_index_source(TextIndex *indexes);

  // ANCHORED FROM THE CONSTRUCTOR, which names all three members.
  // 0x005FDF40 does `lea ecx, [esi + 0x104]; call 0x5d4560` - and
  // 0x005D4560 is `Heap::Heap()`, so heap_ is at 0x104 - then
  // `mov [esi + 0x100], 0` for section_count_ and `mov byte ptr [esi], 0`
  // for this array's first byte. 0x100 is 256, which is exactly its extent.
  char file_name_[256];    // 0x0, name of file to be mapped
  uint32_t section_count_; // number of section entries
  Heap heap_;
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(TextIndex) == 0x118, "TextIndex layout must match the legacy ABI");
#endif

/*
* global
* THE ARRAY, at 0x009B7D08 in the shipped image - four TextIndex objects,
* defined in textindex.cpp (it was `TextIndex TxtIndexGlobal[4]` beside a
* `TextIndex *const TxtIndex = (TextIndex *)0x009B7D08` binding, so the
* storage existed twice: a real, constructed array nothing used, and every
* `TxtIndex[i]` here reading and writing terranx.exe data, unmapped in a
* standalone build). The object now carries the public name and the uses
* index it directly - the element loads fold to the same `[disp32 + i*0x118]`
* form, the displacement relocated.
* The executable owns the four-object array and invokes the exported source
* constructors/destructors.
* void __cdecl `dynamic initializer for 'TxtIndex''(): 005FD4E0
* void __cdecl `dynamic atexit destructor for 'TxtIndex''(): 005FD510
*/
static const int MaxTextIndexNum = 4;
extern TextIndex TxtIndex[MaxTextIndexNum];  // 0x009B7D08, 0x118 stride
void __cdecl text_make_index(LPCSTR source_txt);
// INLINE: `Text::open` (0x005FDA00) carries this whole loop rather than a
// `call 0x5fe230` - only `TextIndex::search_index` (0x005FE120) shows up in
// its own call list. Body claimed at 0x005FE230 in textindex.cpp, same
// pattern as Text's destructor in text.h.
MEASURED inline int __cdecl text_search_index(LPCSTR source_txt, LPCSTR section_txt) {
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        if (TxtIndex[i].get_count()) {
            const int address = TxtIndex[i].search_index(source_txt, section_txt);
            if (address >= 0) {
                return address;
            }
        }
    }
    return -1;
}
void __cdecl text_clear_index();

// INLINE: `text_clear_index` (0x005FE270) carries this whole loop - `osmx
// calls` shows only 0x005D4580 (Heap::shutdown), never a call to this
// helper. Body claimed at 0x005FE270 in textindex.cpp.
inline void text_clear_index_source(TextIndex *indexes) {
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        if (indexes[i].section_count_ != 0) {
            indexes[i].heap_.shutdown();
            indexes[i].section_count_ = 0;
        }
    }
}
