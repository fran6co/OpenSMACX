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
  * Spot class
  */
class DLLEXPORT Spot {
 public:
  Spot();  // 005FA860
  ~Spot(); // 005FA870

  void clear(); // 005FA820
  void shutdown();
  void init(int count);
  void replace(int position, int type, int left, int top, int length, int width);
  int add(int position, int type, int left, int top, int length, int width);
  int add(int position, int type, RECT *rect);
  void kill_pos(int position);
  void kill_specific(int position, int type);
  void kill_type(int type);
  int check(int x, int y, int *spot_pos, int *spot_type);
  int check(int x, int y, int *spot_pos, int *spot_type, RECT *spot_rect);
  int get_rect(int position, int type, RECT *spot_rect);

 private:
  struct SpotInternal {
      RECT rect;
      int type;
      int position;
  } *spots_;
  uint32_t max_count_;
  uint32_t add_count_;
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(Spot) == 0xC, "Spot layout must match the legacy ABI");
#endif
