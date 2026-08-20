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
  ~AutoSound() { ; }
  void construct();
  void close();
  void close2();
  void init();

 private:
  PVOID vtable_;
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
extern uint32_t *AutoSoundDefaults;

// The game CRT operator delete the scalar deleting destructor frees the
// object through; bound here rather than through wave.h so this file's
// link closure stays self-contained.
typedef void(__cdecl func_auto_sound_delete)(void *block);
extern func_auto_sound_delete *AutoSoundOperatorDelete;
AutoSound *__fastcall auto_sound_construct_redirect(AutoSound *self, void *);
void __fastcall auto_sound_close_redirect(AutoSound *self, void *);
void __fastcall auto_sound_close2_redirect(AutoSound *self, void *);
void __fastcall auto_sound_init_redirect(AutoSound *self, void *);
void *__fastcall auto_sound_scalar_dtor_redirect(AutoSound *self, void *,
                                                 unsigned int mode);

// Returns `int`, not `void`: the body is `xor eax, eax; ret`, where a void
// function would emit `ret` alone. Corrected in tools/catalogue_corrections.py.
int __cdecl do_sound();
int __cdecl do_sound_redirect();
