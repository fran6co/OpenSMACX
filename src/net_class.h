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

 /*
  * VoiceTx class
  *
  * Net's first embedded subobject, and one of only two classes in this tree
  * whose members come from TWO independent sources that agree exactly.
  *
  *   * The IDB gives 22 members at 0x0..0x54 summing to 0x58. Its offsets
  *     ACCUMULATE, so on its own a probe of them proves self-consistency
  *     rather than truth - which is the argument src/hypothesis_layouts.h
  *     makes about itself, and why this could not simply be asserted there.
  *   * docs/recovery/thinker-members.csv gives the same 22 members at the
  *     same EXPLICIT offsets and the same 0x58 total. Thinker writes offsets
  *     down rather than deriving them, so it cannot inherit an accumulation
  *     error from the IDB.
  *
  * The image supplies the bound that closes it. `??0Net@@QAE@XZ` constructs
  * this at `lea ecx, [esi+0x58]` (0x0062D6C9) and Net's next subobject sits
  * at 0x000000B0 - a difference of exactly 0x58, so the object cannot be
  * larger. Two agreeing member sources and an upper bound from the bytes is
  * the standard this tree pins a size to; `derive_agreed_sizes` had already
  * agreed 0x58 and had nowhere honest to assert it.
  *
  * The members are unnamed because neither source names them: `0 named` is
  * what hypothesis_layouts.h recorded, and inventing names would be the only
  * part of this that is not measured.
  */
class VoiceTx {
 public:
  uint32_t field_0_;    // 0x0
  uint32_t field_4_;    // 0x4
  uint32_t field_8_;    // 0x8
  uint32_t field_C_;    // 0xC
  uint32_t field_10_;   // 0x10
  uint32_t field_14_;   // 0x14
  uint32_t field_18_;   // 0x18
  uint32_t field_1C_;   // 0x1C
  uint32_t field_20_;   // 0x20
  uint32_t field_24_;   // 0x24
  uint32_t field_28_;   // 0x28
  uint32_t field_2C_;   // 0x2C
  uint32_t field_30_;   // 0x30
  uint32_t field_34_;   // 0x34
  uint32_t field_38_;   // 0x38
  uint32_t field_3C_;   // 0x3C
  uint32_t field_40_;   // 0x40
  uint32_t field_44_;   // 0x44
  uint32_t field_48_;   // 0x48
  uint32_t field_4C_;   // 0x4C
  uint32_t field_50_;   // 0x50
  uint32_t field_54_;   // 0x54
};

static_assert(sizeof(VoiceTx) == 0x58,
              "VoiceTx layout must match the original executable");

 /*
  * Net class
  *
  * Layout not established. "Reach as far as 0x770" stood here and is
  * withdrawn: the measurement is 0x768 - docs/recovery/access-lower-bounds.csv
  * records `Net,0x768` from `mov dword ptr [esi + 0x764], ebx` at 0x0062D850
  * in ??0Net, and no row of docs/recovery/member-accesses.csv attributed to
  * Net goes higher. A reach is a LOWER bound, so the object is at least
  * 0x768; tools/derive_class_layout.py finds no size evidence for Net at all,
  * and the 0x18 that carries the total to 0x780 below is pad, not a
  * measurement. The stubs recovered here are constant returns touching no
  * field, so the opaque storage below is only an object for the canary to
  * seed, not a modelled layout.
  *
  * That pad is now load-bearing: src/alphanet.h derives AlphaNet from Net, so
  * this size is where AlphaNet's own storage begins. Correcting it will fail
  * `static_assert(sizeof(AlphaNet) == 0x14A0)` there, which is the intended
  * alarm - re-measure AlphaNet's `data_` rather than silence it.
  */
class Net {
 public:
  // 0x00630A00, a pending_bodies forwarder.
  int get(unsigned long *a, unsigned long *b);

 public:
  // 0x0062E010, a pending_bodies forwarder.
  void close();

 public:
  Net() { ; }
  ~Net() { ; }
  int poll_players(int);
  // `K`, not `I`. MSVC decorates `unsigned long` K and `unsigned int` I,
  // and this tree spells uint32_t as unsigned int - same width, different
  // decorated name, and the name is what the linker pairs on.
  char *get_player_name(unsigned long key);

 private:
  uint32_t field_0_;  // 0x0
  uint32_t field_4_;  // 0x4
  uint32_t field_8_;  // 0x8
  uint32_t field_C_;  // 0xC
  uint32_t field_10_;  // 0x10
  uint32_t field_14_;  // 0x14
  uint32_t field_18_;  // 0x18
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  uint32_t field_28_;  // 0x28
  uint8_t field_2C_[0x18];  // 0x2C
  uint32_t field_44_;  // 0x44
  uint32_t field_48_;  // 0x48
  uint32_t field_4C_;  // 0x4C
  uint32_t field_50_;  // 0x50
  uint32_t field_54_;  // 0x54
  // 0x58 - THE SUBOBJECT ??0Net BUILDS, not opaque storage that happens to
  // be 0x58 wide. `lea ecx, [esi+0x58]` / `call ??0VoiceTx@@QAE@XZ` at
  // 0x0062D6C9, and ??1Net tears it down at the same offset. The two members
  // this replaces covered exactly 0x58 bytes, so nothing moves.
  VoiceTx voice_tx_;  // 0x58
  uint32_t field_B0_;  // 0xB0
  uint32_t field_B4_;  // 0xB4
  uint32_t field_B8_;  // 0xB8
  uint32_t field_BC_;  // 0xBC
  uint32_t field_C0_;  // 0xC0
  uint32_t field_C4_;  // 0xC4
  uint32_t field_C8_;  // 0xC8
  uint32_t field_CC_;  // 0xCC
  uint32_t field_D0_;  // 0xD0
  uint32_t field_D4_;  // 0xD4
  uint32_t field_D8_;  // 0xD8
  uint32_t field_DC_;  // 0xDC
  uint32_t field_E0_;  // 0xE0
  uint32_t field_E4_;  // 0xE4
  uint8_t field_E8_[0x6C];  // 0xE8
  uint32_t field_154_;  // 0x154
  uint32_t field_158_;  // 0x158
  uint32_t field_15C_;  // 0x15C
  uint8_t field_160_[0x4];  // 0x160
  uint32_t field_164_;  // 0x164
  uint8_t field_168_;  // 0x168
  uint8_t field_169_[0x56B];  // 0x169
  uint32_t field_6D4_;  // 0x6D4
  uint8_t field_6D8_[0x4];  // 0x6D8
  uint32_t field_6DC_;  // 0x6DC
  uint32_t field_6E0_;  // 0x6E0
  uint32_t field_6E4_;  // 0x6E4
  uint32_t field_6E8_;  // 0x6E8
  uint8_t field_6EC_[0x10];  // 0x6EC
  uint32_t field_6FC_;  // 0x6FC
  uint32_t field_700_;  // 0x700
  uint32_t field_704_;  // 0x704
  uint32_t field_708_;  // 0x708
  uint32_t field_70C_;  // 0x70C
  uint32_t field_710_;  // 0x710
  uint32_t field_714_;  // 0x714
  uint32_t field_718_;  // 0x718
  uint32_t field_71C_;  // 0x71C
  uint32_t field_720_;  // 0x720
  uint8_t field_724_[0x8];  // 0x724
  uint32_t field_72C_;  // 0x72C
  uint32_t field_730_;  // 0x730
  uint32_t field_734_;  // 0x734
  uint32_t field_738_;  // 0x738
  uint32_t field_73C_;  // 0x73C
  uint32_t field_740_;  // 0x740
  uint32_t field_744_;  // 0x744
  uint8_t field_748_[0x8];  // 0x748
  uint32_t field_750_;  // 0x750
  uint32_t field_754_;  // 0x754
  uint32_t field_758_;  // 0x758
  uint32_t field_75C_;  // 0x75C
  uint32_t field_760_;  // 0x760
  uint32_t field_764_;  // 0x764
  uint8_t field_768_[0x18];  // 0x768
};

int __fastcall net_poll_players_redirect(Net *self, void *, int a1);
char *__fastcall net_get_player_name_redirect(
    Net *self, void *, unsigned long key);
