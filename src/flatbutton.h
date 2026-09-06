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
  * FlatButton class
  */
class FlatButton : public BaseButton {
 public:
  FlatButton();
  // DEFINED IN-CLASS so enclosing destructors can expand it, which is what
  // the image does: ??1DipEdit carries five expanded copies (the
  // 0x669754/0x66974c stage pair, `call ?close@FlatButton`, `call
  // ??1BaseButton`, the enclosing EH state written between them), and so do
  // ??1BasePop and ??1FileWin. Out-of-line here the compiler can only emit
  // `call ??1FlatButton` at those sites - the six-flat-button teardown in
  // ??1FileWin was hand-spelled around exactly that wall. The class's own
  // COMDAT still measures against ??1FlatButton@@QAE@XZ 0x00406880 from
  // flatbutton.cpp, where it keeps being emitted (24/24 re-measured after
  // the move).
  MEASURED ~FlatButton() { close(); }

  uint32_t close();
  FlatButton *destroy();

 private:
  uint32_t field_AB8_;  // 0x0ab8
  uint32_t field_ABC_;
  uint32_t field_AC0_;
  uint32_t field_AC4_;
  uint32_t field_AC8_;
  uint32_t field_ACC_;
  uint32_t field_AD0_;
  uint32_t field_AD4_;
  uint32_t field_AD8_;
  uint32_t field_ADC_;
 public:
  // THE 27 CONFIG DWORDS AT 0xAE0..0xB48, PUBLIC so BasePop::exec's
  // apply_button_config can name the array instead of walking the object
  // address. BasePop::default_block_ is the same nine-rows-of-three table and
  // exec copies all 27 into each of its three button sites; FlatButton::close
  // and the constructor walk the same run from the defaults table. Typed as
  // one flat array - nothing in the tree reads back an individual row, and the
  // Sprite* typing four of these words used to carry came from their offsets
  // rather than from any access the image makes. The members keep their
  // offsets (0xAE0 through 0xB48 inclusive); only the spelling moved.
  uint32_t config[27];  // 0xAE0
};

static_assert(sizeof(FlatButton) == 0xB4C,
              "FlatButton layout must match the legacy ABI");


