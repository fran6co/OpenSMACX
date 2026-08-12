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
  * Net class
  *
  * Layout not established; its methods reach as far as 0x770, so the
  * object is at least that large. The stubs recovered here are constant
  * returns touching no field, so the opaque storage below is only an object
  * for the canary to seed, not a modelled layout.
  */
class DLLEXPORT Net {
 public:
  Net() { ; }
  ~Net() { ; }
  int poll_players(int);
  // `K`, not `I`. MSVC decorates `unsigned long` K and `unsigned int` I,
  // and this tree spells uint32_t as unsigned int - same width, different
  // decorated name, and the name is what the linker pairs on.
  char *get_player_name(unsigned long key);

 private:
  uint8_t unmapped_[0x780];
};

int __fastcall net_poll_players_redirect(Net *self, void *, int a1);
char *__fastcall net_get_player_name_redirect(
    Net *self, void *, unsigned long key);
