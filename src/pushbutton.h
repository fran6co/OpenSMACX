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

#include "basebutton.h"

/*
 * PushButton - promoted from src/hypothesis_layouts.h so Gamma can embed it.
 * Derives from BaseButton by ordinary single inheritance: base-edges.csv
 * records the edge as AGREED (constructor evidence), and the IDB member
 * table opens with a baseButton prefix sized exactly sizeof(BaseButton),
 * followed by eighteen four-byte fields ending at 0xB00. No method is
 * recovered and nothing pins the sizeof yet.
 */
// The two virtual tables the destructor stages before delegating to
// BaseButton::destroy(). DEFINED HERE, NOT `extern`, so the value is
// visible at the use site and folds to an immediate store, the same
// reasoning as ScrollPrimaryVtable/ScrollBufferVtable (scroll.h).
const uint32_t PushButtonPrimaryVtable = 0x00670C60;
const uint32_t PushButtonBufferVtable = 0x00670C58;

class PushButton : public BaseButton {
 public:
  PushButton() { ; }
  // A real `~PushButton()` mangles `??1PushButton@@UAE@XZ` here, not the
  // catalogued `@@QAE@XZ`: AutoSound's virtual destructor (autosound.h,
  // added for RadioButton's vbtable layout) cascades virtuality onto every
  // Win-derived destructor, and VC6's complete-object form for a virtual
  // destructor installs the vtable pointer and unwinds bases through a
  // ~15-instruction sequence where the image's body is 3 (two `mov`s and a
  // tail `jmp`). `destroy()` below is an ordinary method, never a
  // destructor override, so it is never cascaded into virtual - the same
  // BaseButton::destroy()/Scroll::destroy() idiom used everywhere else in
  // this tree for exactly this shape of problem.
  ~PushButton() { destroy(); }

  PushButton *destroy();

 private:
  uint32_t field_AB8_;  // 0xAB8
  uint32_t field_ABC_;  // 0xABC
  uint32_t field_AC0_;  // 0xAC0
  uint32_t field_AC4_;  // 0xAC4
  uint32_t field_AC8_;  // 0xAC8
  uint32_t field_ACC_;  // 0xACC
  uint32_t field_AD0_;  // 0xAD0
  uint32_t field_AD4_;  // 0xAD4
  uint32_t field_AD8_;  // 0xAD8
  uint32_t field_ADC_;  // 0xADC
  uint32_t field_AE0_;  // 0xAE0
  uint32_t field_AE4_;  // 0xAE4
  uint32_t field_AE8_;  // 0xAE8
  uint32_t field_AEC_;  // 0xAEC
  uint32_t field_AF0_;  // 0xAF0
  uint32_t field_AF4_;  // 0xAF4
  uint32_t field_AF8_;  // 0xAF8
  uint32_t field_AFC_;  // 0xAFC, the IDB's last member; the class ends 0xB00
};
static_assert(sizeof(PushButton) == 0xB00, "probe");
