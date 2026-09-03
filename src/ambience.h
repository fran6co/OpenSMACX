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

  // SLOTS 33..74 - the sequencer-delegate interface the image appends to
  // Sound's 33. Every Ambience variant's vftable is 75 entries: 0x0066BB58
  // (SAmbience), 0x0066BE28 (MAmbience), 0x0066BF90 (GAmbience) and
  // 0x0066C0F8 (FactionAmbience) all agree slot for slot, and all four hold
  // the SAME functions at 33..74 that MidiDelegate's table holds at 0x84..
  // 0x128 - ?play_trackset@@QAEHI@Z at 0x10c, ?xpose_trackset@@QAEHIH@Z at
  // 0x114, ?is_trackset_playing@@QAE_NI@Z at 0x118. Two bases would put a
  // second vptr at 0x54 and push every variant field along, and the
  // static_asserts below would fail, so the interface is inherited into the
  // ONE primary table, not embedded. Slot 33+ is therefore declared here,
  // on the class whose general()/hostility() dispatch through it.
  //
  // The bodies that reach these slots: MAmbience::general (0x00447360),
  // UAmbience::general (0x00447570), SAmbience::general (0x004476F0) and
  // SAmbience::hostility (0x004477B0).
  virtual void vslot_33();
  virtual void vslot_34();
  virtual void vslot_35();
  virtual void vslot_36();
  virtual void vslot_37();
  virtual void vslot_38();
  virtual void vslot_39();
  virtual void vslot_40();
  virtual void vslot_41();
  virtual void vslot_42();
  virtual void vslot_43();
  virtual void vslot_44();
  virtual void vslot_45();
  virtual void vslot_46();
  virtual void vslot_47();
  virtual void vslot_48();
  virtual void vslot_49();
  virtual void vslot_50();
  virtual void vslot_51();
  virtual void vslot_52();
  virtual void vslot_53();
  virtual void vslot_54();
  virtual void vslot_55();
  virtual void vslot_56();
  virtual void vslot_57();
  virtual void vslot_58();
  virtual void vslot_59();
  virtual void vslot_60();
  virtual void vslot_61();
  virtual void vslot_62();
  virtual void vslot_63();
  virtual void vslot_64();
  virtual void vslot_65();
  virtual void vslot_66();
  virtual int play_trackset(unsigned int trackset);   // slot 67, 0x10c
  virtual int stop_trackset(unsigned int trackset);   // slot 68, 0x110
  virtual int xpose_trackset(unsigned int trackset, int semitones);  // 69, 0x114
  virtual bool is_trackset_playing(unsigned int trackset);  // 70, 0x118
  virtual void vslot_71();
  virtual int load_patch(unsigned int patch);         // slot 72, 0x120
  virtual int unload_patch(unsigned int patch);       // slot 73, 0x124
  virtual int set_patch(unsigned int patch, unsigned int instrument);  // 74

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
  // SIGNED: MAmbience::general compares it against 0x1f4 with `jle`
  // (0x00447444) and wraps it to -700, so uint32_t spells `jbe`. Same
  // evidence as SAmbience::field_70_ below.
  int32_t field_70_;
};

static_assert(sizeof(MAmbience) == 0x74,
              "MAmbience layout must match the original executable");

class SAmbience : public Ambience {
 public:
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
  // SIGNED, and not by style: SAmbience::hostility compares both against a
  // positive bound with `jle` (0x004477EA `cmp eax, 0x67`, 0x00447821
  // `cmp eax, 0x12c`) and wraps each downward past zero when it exceeds it,
  // which only a signed field spells. `uint32_t` gave `jbe` and lost 2 of 46.
  int32_t field_70_;
  int32_t field_74_;
};

static_assert(sizeof(SAmbience) == 0x78,
              "SAmbience layout must match the original executable");

class GAmbience : public Ambience {
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

