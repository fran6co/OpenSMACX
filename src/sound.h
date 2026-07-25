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
class DLLEXPORT Sound {
 public:
  Sound() { ; }
  ~Sound() { ; }
  int UNK1(int);
  void fade(int a1);
  int is_playing();
  int is_looping();
  int get_time();
  int play();
  int play(unsigned int a1);
  int stop();
  int release();
  void set_loop_state(long a1);
  void set_delay(unsigned int a1);

 private:
  uint8_t unmapped_[0xA0];
};

int __fastcall sound_unk1_redirect(Sound *self, void *, int a1);
void __fastcall sound_fade_redirect(Sound *self, void *, int a1);
int __fastcall sound_is_playing_redirect(Sound *self, void *);
int __fastcall sound_is_looping_redirect(Sound *self, void *);
int __fastcall sound_get_time_redirect(Sound *self, void *);
int __fastcall sound_play_redirect(Sound *self, void *);
int __fastcall sound_play_arg_redirect(Sound *self, void *, unsigned int a1);
int __fastcall sound_stop_redirect(Sound *self, void *);
int __fastcall sound_release_redirect(Sound *self, void *);
void __fastcall sound_set_loop_state_redirect(Sound *self, void *, long a1);
void __fastcall sound_set_delay_redirect(Sound *self, void *, unsigned int a1);
