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
  * AutoSound - the PROCESS SETTINGS RECORD, embedded at offset 0 of every
  * window (class Win : public AutoSound). 37 ints behind one vfptr; the
  * constructor copies them wholesale from AutoSoundDefaults[33], the
  * runtime-filled block at 0x009BC080 that FX::init zeroes before any
  * window exists.
  *
  * FIELD MAP (offset = 0x04 + 4*(N-1)). One member is anchored:
  *   flags_   0x20  BasePop toggles bits 0/1 (UNK3/UNK4)
  * The other 36 are bulk-copy/reset/zero only across the whole tree -
  * no per-field behaviour exists anywhere yet, so they stay numbered
  * rather than guessed. Their names arrive with the settings/options
  * writer, still unrecovered. IDB independently numbered them identically
  * (docs/recovery/idb-members.csv: val1..val37).
  */
class AutoSound {
 public:
  AutoSound();
  // VIRTUAL, so VC6 emits the vfptr at offset 0 - Win's and GraphicWin's
  // too, since Win derives from here (radiobutton.h measured this as the
  // second of the two missing base vtable pointers; Dialog supplied the
  // first).
  //
  // THE BODY IS CLOSE, MEASURED: the image's deleting destructor
  // ??_GAutoSound@@UAEPAXI@Z (36 bytes) restores the vftable, runs close(),
  // then frees conditionally on the caller's flag - which is byte-for-byte
  // what cl emits for a virtual destructor whose body runs close(). The
  // compiler owns ??_G; nothing in the tree names it or the vftable any
  // more.
  virtual ~AutoSound() { close(); }
  // Returns `this`: the image's body opens `mov eax, ecx` and uses EAX as
  // the object base for all 38 stores, which is what a __thiscall that has
  // to leave `this` in EAX does. Declared `void`, VC6 keeps the base in ECX
  // and scores 0 of 77.
  void close();
  void close2();
  void init();
  // ??_GAutoSound@@UAEPAXI@Z as a member: re-installs the vfptr, runs
  // close(), deletes only when the caller's flag says so.
  void *scalar_delete(unsigned int mode);

 protected:
  // PROTECTED, NOT PRIVATE: Win derives from AutoSound and the homed window
  // bodies read these directly - which is what the image does, since they are
  // one object. `private` modelled a containment that the inheritance edge
  // replaced.
  // The vfptr is EMITTED by the compiler now - see the virtual destructor
  // above. Same four bytes at offset 0, so sizeof(AutoSound) stays 0x98.
  int val_1_;  // 0x0004
  int val_2_;
  int val_3_;
  int val_4_;
  int val_5_;
  int val_6_;
  int val_7_;
 protected:
  // MEASURED: BasePop::UNK3/UNK4 set/clear bits 0/1 of this word
  // (basepop.cpp) - the only per-field behaviour recovered anywhere.
  uint32_t flags_;
  // PROTECTED for the same reason the block above says: Win derives from
  // AutoSound and its homed bodies read these fields. They were reached by
  // raw offset off `reinterpret_cast<char *>(this)` because `private` made
  // the member form uncompilable - the access happened either way, it just
  // could not say what it was touching.
 protected:
  int val_9_;
  int val_10_;
  int val_11_;
  int val_12_;
  int val_13_;
  int val_14_;
  int val_15_;
  int val_16_;
  int val_17_;
  int val_18_;
  int val_19_;
  int val_20_;
  int val_21_;
  int val_22_;
  int val_23_;
  int val_24_;
  int val_25_;
  int val_26_;
  int val_27_;
  int val_28_;
  int val_29_;
  int val_30_;
  int val_31_;
  int val_32_;
  int val_33_;
  int val_34_;
  int val_35_;
  int val_36_;
  int val_37_;
};

static_assert(sizeof(AutoSound) == 0x98,
              "AutoSound layout must match the legacy ABI");


// The game CRT operator delete the scalar deleting destructor frees the
// object through; bound here rather than through wave.h so this file's
// link closure stays self-contained.

// Returns `int`, not `void`: the body is `xor eax, eax; ret`, where a void
// function would emit `ret` alone. Corrected in tools/catalogue_corrections.py.

MEASURED inline int __cdecl do_sound() {
    return 0;
}
