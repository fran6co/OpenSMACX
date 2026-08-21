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
#include "buffer.h"
#include "palette.h"

 /*
  * Flic class
  *
  * No base class: the constructor's first call is straight to
  * `??0Buffer@@QAE@XZ` on `this + 4`, and its second is to
  * `??0Palette@@QAE@XZ` on `this + 0x5BC` - both ordinary members
  * auto-constructing in declaration order, not a base-class chain.
  */
class Flic {
 public:
  Flic();
  ~Flic();
  void close();
  void UNK4();
  void UNK5();
  void UNK6();
  void UNK7();
  void UNK8();

 private:
  // Only the low byte is ever stored (`mov byte ptr [esi], bl`).
  uint8_t field_0_;  // 0x0
  Buffer buffer_;    // 0x4
  uint32_t field_58C_;
  uint32_t field_590_;
  uint32_t field_594_;
  uint32_t field_598_;
  uint32_t field_59C_;
  uint32_t field_5A0_;
  uint32_t field_5A4_;
  Buffer *buffer2_;
  uint32_t field_5AC_;
  uint32_t field_5B0_;
  uint32_t field_5B4_;
  FILE *file_;
  // A REAL embedded Palette, not a pointer: the constructor
  // placement-constructs it (`lea ecx,[esi+0x5bc]; call ??0Palette@@QAE@XZ`)
  // and the destructor tears it down the same way
  // (`call ??1Palette@@QAE@XZ`), both on `this + 0x5BC` directly.
  Palette palette_;  // 0x5BC, spans to 0xA10 (sizeof(Palette) == 0x454)
  // Was `field_5C0_[329]` before `palette_` became a real object; shifted by
  // sizeof(Palette) - 4 so the total extent this class already claimed
  // (0x5C0 + 329*4 == 0xAE4) is unchanged: 0xA10 + 53*4 == 0xAE4.
  uint32_t field_A10_[53];
};

void __fastcall flic_unk4_redirect(Flic *self, void *);
void __fastcall flic_unk5_redirect(Flic *self, void *);
void __fastcall flic_unk6_redirect(Flic *self, void *);
void __fastcall flic_unk7_redirect(Flic *self, void *);
void __fastcall flic_unk8_redirect(Flic *self, void *);
