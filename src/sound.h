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
  * Sound class
  *
  * Layout not established; its methods reach as far as 0x90, so the
  * object is at least that large. The only thing pinned here is the vtable
  * pointer at offset zero, which fade dispatches through; it is held as
  * opaque storage so no C++ vtable is generated that could disagree with the
  * original's. The rest is an object for the canary to seed, not a modelled
  * layout.
  */
class Sound {
 public:
  Sound() { ; }
  ~Sound();
  int UNK1(int);
  void fade(unsigned long a1);
  int is_playing();
  int is_looping();
  int get_time();
  int play();
  int play(unsigned int a1);
  int stop();
  int release();
  void set_loop_state(long a1);
  void set_delay(unsigned int a1);
  int fade();
  int fade_in();
  void ramp(int a1, int a2, unsigned int a3);
  void set_type(unsigned int a1);
  int load(const char *a1);
  void set_volume(int a1);
  int set_fade(unsigned long a1);
  int set_fade_in(unsigned int a1);
  void fade_in(unsigned int a1);
  void set_pan(int a1);
  int unload();
  int attach();
  int detach();

 protected:
  // PROTECTED, not private, because `Wave` is a real derived class - its
  // own 0x00..0x53 duplicated every one of these fields at the same
  // offsets until 2026-08-22. Access changes no layout and no codegen.
  // The fields below are the slice Wave inherits and mirrors at the same
  // offsets: the loop dword at 0x30, the wrapped device at 0x3C, the flag
  // dword at 0x40 (bit 0 the loaded bit, plus set_type's class bits), the
  // heap-owned filename at 0x4C, and the type at 0x50.
  uint32_t vtable_storage_;  // 0x00, opaque so no C++ vtable is generated
  uint32_t volume_;       // 0x04, low seven bits of set_volume's argument
  int32_t pan_8_;         // 0x08, set_pan's clamp to [-0x40, 0x3F]
  uint32_t field_C_;      // 0x0C, IDB field_C..field_2C for the nine dwords
  uint32_t field_10_;     // 0x10
  uint32_t field_14_;     // 0x14
  uint32_t field_18_;     // 0x18
  uint32_t field_1C_;     // 0x1C
  uint32_t field_20_;     // 0x20
  uint32_t field_24_;     // 0x24
  uint32_t field_28_;     // 0x28
  uint32_t field_2C_;     // 0x2C
  uint32_t loop_flag_30_;  // 0x30, IDB `loop_state`
  uint32_t delay_;        // 0x34, IDB `delay`; set_delay writes it
  uint32_t fade_38_;      // 0x38, the last nonzero fade/fade-in argument
  void *device_;
  uint32_t flags_40_;
  Sound *chain_prev_;     // 0x44, toward the head slot at 0x90DB20
  Sound *chain_next_;     // 0x48, toward the tail slot at 0x90DB1C
  void *fname_;
  uint32_t type_;
};

// Two independent sources agree on 0x54 and neither fails like the other:
// the IDB member table ends at the `type` dword, and the access lower bound
// is the mov [esi + 0x50] at 0x004C6107. The 0x4C tail this header used to
// declare contradicted both and covered nothing - no body reads it and
// nothing embeds a Sound by value.
//
// A THIRD witness landed later, from the other direction: 0x54 is exactly
// where Wave's own fields start (0x004C6774) and where Ambience's do
// (0x004C84xx), because both constructors inline this whole constructor on an
// unadjusted `this` before writing anything of their own. Neither is spelled
// with a Sound base - Sound::~Sound is out of line and a real base would emit
// a call the original inlines - but both begin with one.
static_assert(sizeof(Sound) == 0x54, "Sound layout must match the original executable");

void *__fastcall sound_scalar_dtor_redirect(Sound *self, void *,
                                            unsigned int mode);

// Sound subsystem bring-up, called once from WinMain with the main window and
// a bitmask of the requested backends. `void *` rather than `HWND` because the
// image's own name for it - ?init_sound@@YAHPAXK@Z - spells the first
// parameter `PAX`.
int __cdecl init_sound(void *window, unsigned long backends);

// Loads the sound driver DLL; returns zero on success. Called first thing by
// init_sound. Body not yet recovered.
int __cdecl load_sound_dll();
