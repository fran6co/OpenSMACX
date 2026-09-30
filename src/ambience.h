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

#include "midi.h"

class FactionAmbience;
extern FactionAmbience *FactionMusic;
extern int LastAmbienceVehicle;
void __cdecl load_music(int faction_id);
void __cdecl ambience_veh(int vehicle_id);

/*
 * Sound owns the common 0x54-byte device/lifetime prefix. Midi adds its
 * mode byte and sequencer interface; Ambience changes load/unload behavior
 * without adding storage. Constructor 0x004C8460 stages Sound, Midi and
 * Ambience tables on the same receiver, not a second embedded sound object.
 *
 * FactionAmbience owns the shared event state at 0x58..0x6D and virtual
 * events 75..89. Its subclasses provide the faction-specific sequencing.
 * Original allocation sizes, constructor stores and event accesses establish
 * the differing subclass tails; no copied table or opaque base storage is used.
 */
// Original4C8460 stages Midi66E538 before Ambience66E664; both endat58.
class Ambience : public Midi {
 public:
  Ambience();
  ~Ambience() {}
  int load(const char *filename);
  int unload();
};
static_assert(sizeof(Midi) == 0x58, "Midi adds its single mode byte to Sound");
static_assert(sizeof(Ambience) == 0x58, "Ambience adds behavior, not storage");


class FactionAmbience : public Ambience {
 public:
  FactionAmbience();
  ~FactionAmbience() {}
  // Events75..89, original tables66C0F8/66BB58/66BE28/66BF90/66BCC0.
  virtual void begin();
  virtual void tech();
  virtual void terraform();
  virtual void production();
  virtual void general();
  virtual void popup1();
  virtual void eot();
  virtual void hostility();
  virtual void basewin_show();
  virtual void basewin_hide();
  virtual void new_base();
  virtual void energy_resources();
  virtual void design_window_show();
  virtual void design_window_hide();
  virtual void base_liberated();
 protected:
  uint32_t field_58_;
  uint32_t field_5C_;
  uint32_t field_60_;
  uint32_t field_64_;
  uint32_t field_68_;
  uint8_t field_6C_;
  uint8_t field_6D_;
};
static_assert(sizeof(FactionAmbience) == 0x70,
              "Faction ambience owns the common event state58..6D");

class AAmbience : public FactionAmbience {
 public:
  AAmbience() {}
  void begin();
  void general();
  void tech();
};
static_assert(sizeof(AAmbience) == 0x70, "AAmbience has no added storage");

class BAmbience : public FactionAmbience {
 public:
  // 0x004478D0 initializes the B-specific counters after FactionAmbience.
  BAmbience();
  void begin();
  // This store was formerly represented by the seed-only byte accessor at
  // 0x00447AB0; the mapped body and constructor both identify byte +0x6D.
  void design_window_show();
  void design_window_hide();
  void new_base();
  void base_liberated();
  // Known original members retained as declarations while their global state
  // dependencies remain unbound in product source.
  void general();
  void hostility();

 private:
  int32_t field_70_;
  int32_t field_74_;
  int32_t field_78_;
  int32_t field_7C_;
};

static_assert(sizeof(BAmbience) == 0x80,
              "BAmbience layout must match its constructor and event accesses");


class UAmbience : public FactionAmbience {
 public:
  // homed from 00447550.cpp
  void basewin_hide();

 public:
  // homed from 00447530.cpp
  void basewin_show();

 public:
  // homed from 00447510.cpp
  void begin();

 public:
  // homed from 00447570.cpp
  void general();

 public:
  UAmbience();
  ~UAmbience() { ; }
  void tech();
  void popup1();
  void eot();

  // No constructor for this variant survives in the catalog, so unlike its
  // siblings nothing evidences fields of its own.

  // Storage the image proves is here: its own methods reach 0x6D.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 6 member(s) from the IDA database, 0 named; it starts a member at 0x58, which is where src/ ends.

};

// MAmbience and SAmbience are evidenced the same way FactionAmbience and
// GAmbience are: by what their constructors write. Neither has any other
// recovered method, so these fields and nothing beyond them are what the image
// establishes - the same rule the file header sets out, applied to two more
// variants rather than relaxed for them.
class MAmbience : public FactionAmbience {
 public:
  // homed from 004474e0.cpp
  void new_base();

 public:
  // homed from 004474b0.cpp
  void production();

 public:
  // homed from 00447490.cpp
  void energy_resources();

 public:
  // homed from 00447470.cpp
  void tech();

 public:
  // homed from 00447340.cpp
  void begin();

 public:
  // homed from 00447360.cpp
  void general();

 public:
  MAmbience();
  ~MAmbience() { ; }

 private:
  // SIGNED: MAmbience::general compares it against 0x1f4 with `jle`
  // (0x00447444) and wraps it to -700, so uint32_t spells `jbe`. Same
  // evidence as SAmbience::field_70_ below.
  int32_t field_70_;
};

static_assert(sizeof(MAmbience) == 0x74,
              "MAmbience layout must match the original executable");

class SAmbience : public FactionAmbience {
 public:
  void tech();
  // homed from 00447880.cpp
  void base_liberated();

 public:
  // homed from 00447840.cpp
  void production();

 public:
  // homed from 004476c0.cpp
  void begin();

 public:
  // homed from 004477b0.cpp
  void hostility();

 public:
  // homed from 004476f0.cpp
  void general();

 public:
  SAmbience();
  ~SAmbience() { ; }

 private:
  // SIGNED, and not by style: SAmbience::hostility compares both against a
  // positive bound with `jle` (0x004477EA `cmp eax, 0x67`, 0x00447821
  // `cmp eax, 0x12c`) and wraps each downward past zero when it exceeds it,
  // which only a signed field spells. `uint32_t` gave `jbe` and lost 2 of 46.
  int32_t field_70_;
  int32_t field_74_;
};

static_assert(sizeof(SAmbience) == 0x78,
              "SAmbience layout must match the original executable");

class GAmbience : public FactionAmbience {
 public:
  // homed from 00447cd0.cpp
  void new_base();

 public:
  // homed from 00447c70.cpp
  void terraform();

 public:
  // homed from 00447c00.cpp
  void general();

 public:
  // homed from 00447bc0.cpp
  void begin();

 public:
  GAmbience();
  ~GAmbience() { ; }
  void tech();
  void production();
  void popup1();
  void eot();
  void basewin_show();
  void basewin_hide();

 private:
  // Original 0x00447C32 uses signed jle; general wraps this counter below zero.
  int32_t field_70_;
};

static_assert(sizeof(GAmbience) == 0x74,
              "GAmbience layout must match the original executable");
