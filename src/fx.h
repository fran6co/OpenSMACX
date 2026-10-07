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
#include "wave.h"

 /*
  * FX class - the sound-effect bank.
  *
  * Construction and destruction walk 97 Effect objects of 0x6C bytes at
  * offset zero (0x004482A0 / 0x00448290). The compiler owns their lifetimes;
  * the final dword at +0x28EC is initialized separately by FX::init.
  */
class FX {
 public:
  // The two original sound-bank triggers are implemented in fx.cpp.
  void play(int effect);
  void play(int effect, int, int);
  Effect &effect(int index) { return effects_[index]; }

 public:
  // 0x00445CD0: control_game (game.cpp) initializes the
  // bank once the Shift check says the sound stack is wanted.
  void init();

 public:
  void fade(int a1);
  void release(int a1);
  void stop(int a1);
  void set_pitch(int effect, int pitch);
  void engine_pitch(int pitch);
  FX();
  ~FX();

 private:
  // RECOVERY_MEMBER: v4 FX effects_ Effect[97] 0x0 0x28EC 0x004482A0
  Effect effects_[97];

  // FX::init writes -1 here; engine_pitch uses the signed index.
  // RECOVERY_MEMBER: v1 FX engine_effect_index_ 0x28EC 0x00446B10
  int engine_effect_index_;  // 0x28EC
};

static_assert(sizeof(FX) == 0x28F0,
              "FX layout must match the original executable");

// The process-wide source-owned bank; the original identity is in fx.cpp.
extern FX g_FX;  // 0x00749CF8
