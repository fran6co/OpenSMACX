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
  * AutoSound class
  */
class AutoSound {
 public:
  AutoSound() { ; }
  // VIRTUAL, so VC6 emits the vfptr at offset 0 instead of the hand-modelled
  // `PVOID vtable_` this replaces. Win derives from AutoSound, so that one
  // vfptr is Win's and GraphicWin's too - which is the second of the two
  // missing base vtable pointers radiobutton.h measured the virtual-base
  // conversion to be short by. Dialog supplied the first.
  virtual ~AutoSound() { ; }
  // Returns `this`: the image's body opens `mov eax, ecx` and uses EAX as
  // the object base for all 38 stores, which is what a __thiscall that has
  // to leave `this` in EAX does. Declared `void`, VC6 keeps the base in ECX
  // and scores 0 of 77.
  AutoSound *construct();
  void close();
  void close2();
  void init();

 private:
  // The vfptr is EMITTED by the compiler now - see the virtual destructor
  // above. Same four bytes at offset 0, so sizeof(AutoSound) stays 0x98.
  int val_1_;
  int val_2_;
  int val_3_;
  int val_4_;
  int val_5_;
  int val_6_;
  int val_7_;
  int val_8_;
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

extern const uint32_t AutoSoundVtable;
uint32_t *const AutoSoundDefaults = (uint32_t *)0x009BC080;

// The game CRT operator delete the scalar deleting destructor frees the
// object through; bound here rather than through wave.h so this file's
// link closure stays self-contained.
AutoSound *__fastcall auto_sound_construct_redirect(AutoSound *self, void *);
void __fastcall auto_sound_close_redirect(AutoSound *self, void *);
void __fastcall auto_sound_close2_redirect(AutoSound *self, void *);
void __fastcall auto_sound_init_redirect(AutoSound *self, void *);
void *__fastcall auto_sound_scalar_dtor_redirect(AutoSound *self, void *,
                                                 unsigned int mode);

// Returns `int`, not `void`: the body is `xor eax, eax; ret`, where a void
// function would emit `ret` alone. Corrected in tools/catalogue_corrections.py.
int __cdecl do_sound_redirect();

MEASURED inline int __cdecl do_sound() {
    return 0;
}
