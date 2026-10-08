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

#include "font.h"

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
  Font *get_zoom(int zoom, int numerator, int denominator, int style);

 private:
  // Font fonts_[3], not 3*0x28 raw bytes. Declaring the real array is what
  // makes VC6 emit the CRT vector ctor/dtor iterators itself; hand-calling
  // them meant passing `Font::Font` and `Font::~Font` as `const void *`
  // address constants, which C++ cannot even spell as function references.
  // RECOVERY_MEMBER: v4 FontQueue fonts_ Font[3] 0x0 0x78 0x00559290
  Font fonts_[3];

  // Three parallel per-slot arrays, not 9 scalars: the constructor at
  // 0x00559290 walks them with ONE index and a single stride-4 pointer -
  // `base = this + 0x84; for (i = 0; i < 3; i++) { base[-0xc] = -999;
  // base[0] = 0; base[0xc] = i; base += 4; }` - which only makes sense as
  // three int[3] arrays 0xC (3 * sizeof(int)) apart, at 0x78/0x84/0x90.
  // get_zoom compares height/style before moving the matching slot to the
  // back of the replacement order. These are cache keys, not ages or padding.
  // RECOVERY_MEMBER: v1 FontQueue heights_ 0x78 0x005592D0
  int heights_[3];          // seeded -999 (never-used sentinel)
  // RECOVERY_MEMBER: v1 FontQueue styles_ 0x84 0x005592D0
  int styles_[3];
  // RECOVERY_MEMBER: v1 FontQueue lru_slots_ 0x90 0x005592D0
  int lru_slots_[3];        // least recently used first
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(FontQueue) == 0x9C, "FontQueue layout must match the legacy ABI");
#endif

extern FontQueue g_FONTQUEUE_VAL1;
extern FontQueue g_FONTQUEUE_VAL2;
