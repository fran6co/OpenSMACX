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

#include "stdafx.h"

/*
 * Fractal: the value-noise field the world builder samples when it decides
 * where fungus and erosion go. The image has three methods - init at
 * 0x005C1E20, the bilinear sampler at 0x005C1F20 and the two-octave level at
 * 0x005C1FC0 - and no constructor and no destructor: both owners
 * (world_erosion 0x005C2CB0 and world_fungus 0x005C3440) reserve the object
 * on the stack, call init on it straight away and never tear it down, so a
 * declared constructor would only add code neither caller has.
 *
 * LAYOUT. Two wrapping 19x19 byte fields, which is why the class is 722
 * bytes and nothing else:
 *
 *   - init's first loop starts at `lea esi, [ebx + 0x169]` and walks stride
 *     0x13 = 19 for 18 rows of 18 columns, then wraps column 18 to column 0
 *     (`mov byte ptr [esi + 0x12], al`) and row 18 to row 0 (the copy from
 *     0x2bf back over 0x156 = 18 * 19 bytes). 0x169 = 19 * 19 is exactly one
 *     19x19 field past the front of the object, so the random field is
 *     grid_[1].
 *   - init's copy loop walks the same 19x19 rectangle from the object's own
 *     front (`base[row + col]`, rows 0..0x130 step 0x13) and reads its source
 *     from 0x169 - so grid_[0] is the copy of grid_[1] the sampler reads,
 *     and the wrap loops after it (col 16 = col 0, then row 16 = row 0) leave
 *     the 17th row and column as the duplicates UNK1's 0xf index mask needs.
 *
 * `char`, not `uint8_t`: the sampler loads its four taps with `movsx` and
 * blends them signed, which only a signed element type emits.
 *
 * Names. The catalogue's ?UNK1 and ?UNK2 are renamed to what their callers
 * do with the result, and each carries a `// symbol` fact beside its marker
 * in map.cpp recording the mangled name the image has:
 *   - `sample` is the whole of UNK1: four taps read, blended by the 5-bit
 *     fractions of two 0..255 coordinates, no write. It is a pure function of
 *     the field.
 *   - `level` is UNK2's shape, which both of its callers also spell inline:
 *     sample at 16x and at 16x << shift, `* 5` and `* 2`, `* 7 >> 8`, then
 *     clamped into 0..100 - a 0..100 intensity the fungus and erosion passes
 *     threshold against.
 */
class Fractal {
 public:
  // grid_[0] is the smoothed field sample() reads; grid_[1] is the raw random
  // one init fills first. 19 columns of 19 rows, wrapping on row/column 18
  // for grid_[1] and on row/column 16 for grid_[0].
  char grid_[2][19][19];  // 0x0, 722 bytes; grid_[1] starts at 0x169

  // 0x005C1E20 ?init@Fractal@@QAEXXZ - fill the raw field, copy it into the
  // smooth one, wrap both.
  void init();
  // 0x005C1F20 ?UNK1@Fractal@@QAEHHH@Z - bilinear sample of grid_[0] at a
  // 0..255 coordinate pair.
  int sample(int x_coord, int y_coord);
  // 0x005C1FC0 ?UNK2@Fractal@@QAEHHHH@Z - two octaves of sample(), reduced to
  // a 0..100 level.
  int level(int x_coord, int y_coord, int shift);
};
