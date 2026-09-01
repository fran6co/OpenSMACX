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

#include "sound.h"

 /*
  * Ambience classes
  *
  * A family of game-event hooks - a turn beginning, a technology discovered, a
  * base founded - that the original dispatches through a vtable so each
  * faction flavour can respond. The variants recovered here respond to none of
  * them: every one of these methods is a bare return in the original.
  *
  * None of these layouts is established. Both derived constructors write their
  * own fields starting at 0x58, which places the shared base's extent there and
  * is what base_storage_ models, including the vtable pointer the original
  * keeps at offset zero - held as opaque bytes so no vtable is generated here
  * that could disagree with it. Where the objects end is unknown, so nothing
  * pins their sizeof and no field may be appended without deriving it first.
  *
  * THE FIRST 0x54 OF base_storage_ IS A SOUND. ??0Ambience@@QAE@XZ
  * (0x004C8460) replays ??0Sound@@QAE@XZ (0x004C6080) instruction for
  * instruction on an unadjusted `this`: [esi] = 0x0066E444, [esi+4] = 0x7F,
  * `memset(esi + 0xC, 0, 0x24)`, then Sound's own vftable 0x0066E3C0 at
  * 0x004C84AC, [esi+0x38] = 0x3E8, and the guarded call through [esi+0x3C].
  * src/sound.h pins sizeof(Sound) at 0x54, so the four bytes at 0x54 are
  * Ambience's own and the extent at 0x58 follows. Wave begins with the same
  * inlined Sound; this is the third Sound-rooted class, not an unrelated one,
  * and it is why src/ambience.cpp reinterprets `this` as a `Wave *`.
  */
class Ambience : public Sound {
 public:
  uint32_t field_54_;  // 0x54, Ambience-owned; the derived variants start at 0x58
  // Spelled as a method rather than a real constructor on purpose: the three
  // derived variants below all carry inline `{ ; }` constructors, and giving
  // the base a real one would silently change every one of them.
  void construct();
  ~Ambience();

};



class FactionAmbience : public Ambience {
 public:
  FactionAmbience() { ; }
  ~FactionAmbience() { ; }
  void begin();
  void tech();
  void terraform();
  void production();
  void general();
  void new_base();
  void popup1();
  void eot();
  void hostility();
  void energy_resources();
  void base_liberated();
  // The two flag bytes below are what these four toggle - the only
  // FactionAmbience methods that write anything at all.
  void basewin_show();
  void basewin_hide();
  void design_window_show();
  void design_window_hide();
  // RETURNS `this`: the image ends `mov eax, esi; ret`, which a `void`
  // construct never emits. Callers ignore the result.
  FactionAmbience *construct();

 private:
  uint32_t field_58_;  // 0x0058
  uint32_t field_5C_;
  uint32_t field_60_;
  uint32_t field_64_;
  uint32_t field_68_;
  uint8_t field_6C_;
  uint8_t field_6D_;
};

class UAmbience : public Ambience {
 public:
  UAmbience() { ; }
  ~UAmbience() { ; }
  void tech();
  void popup1();
  void eot();

  // No constructor for this variant survives in the catalog, so unlike its
  // siblings nothing evidences fields of its own.

  // Storage the image proves is here: its own methods reach 0x6D.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 6 member(s) from the IDA database, 0 named; it starts a member at 0x58, which is where src/ ends.

  uint32_t field_58_;  // 0x58
  uint32_t field_5C_;  // 0x5C
  uint32_t field_60_;  // 0x60
  uint32_t field_64_;  // 0x64
  uint32_t field_68_;  // 0x68
  uint8_t field_6C_;  // 0x6C
};

// MAmbience and SAmbience are evidenced the same way FactionAmbience and
// GAmbience are: by what their constructors write. Neither has any other
// recovered method, so these fields and nothing beyond them are what the image
// establishes - the same rule the file header sets out, applied to two more
// variants rather than relaxed for them.
class MAmbience : public Ambience {
 public:
  MAmbience() { ; }
  ~MAmbience() { ; }
  // Returns `this`; see FactionAmbience.
  MAmbience *construct();

 private:
  uint32_t field_58_;  // 0x0058
  uint32_t field_5C_;
  uint32_t field_60_;
  uint32_t field_64_;
  uint32_t field_68_;
  uint8_t field_6C_;
  uint8_t field_6D_;
  uint8_t pad_6E_[2];
  uint32_t field_70_;
};

static_assert(sizeof(MAmbience) == 0x74,
              "MAmbience layout must match the original executable");

class SAmbience : public Ambience {
 public:
  SAmbience() { ; }
  ~SAmbience() { ; }
  // Returns `this`; see FactionAmbience.
  SAmbience *construct();

 private:
  uint32_t field_58_;  // 0x0058
  uint32_t field_5C_;
  uint32_t field_60_;
  uint32_t field_64_;
  uint32_t field_68_;
  uint8_t field_6C_;
  uint8_t field_6D_;
  uint8_t pad_6E_[2];
  uint32_t field_70_;
  uint32_t field_74_;
};

static_assert(sizeof(SAmbience) == 0x78,
              "SAmbience layout must match the original executable");

class GAmbience : public Ambience {
 public:
  GAmbience() { ; }
  ~GAmbience() { ; }
  // Returns `this`; see FactionAmbience.
  GAmbience *construct();
  void tech();
  void production();
  void popup1();
  void eot();
  void basewin_show();
  void basewin_hide();

 private:
  uint32_t field_58_;  // 0x0058
  uint32_t field_5C_;
  uint32_t field_60_;
  uint32_t field_64_;
  uint32_t field_68_;
  uint8_t field_6C_;
  uint8_t field_6D_;
  uint8_t pad_6E_[2];
  uint32_t field_70_;
};

static_assert(sizeof(GAmbience) == 0x74,
              "GAmbience layout must match the original executable");

