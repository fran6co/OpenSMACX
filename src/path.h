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
  * Path related objects, variables and functions.
  */
class Path {
 public:
  Path() : map_table_(0), x_table_(0), y_table_(0), index1_(0), index2_(0), faction_id_(0), 
           x_dst_(0), y_dst_(0), unk_(5), faction_id_2_(0), proto_id_(0) { } // n/a
  MEASURED ~Path() { shutdown(); } // 0059A320

  void init();
  // IN-CLASS so `~Path` inlines it, which is what the image does.
  MEASURED void shutdown() {     // 0059A2D0
    if (map_table_) {
      free(map_table_);
    }
    if (x_table_) {
      free(x_table_);
    }
    if (y_table_) {
      free(y_table_);
    }
    // IMAGE ORDER: y_table_ is nulled before x_table_.
    map_table_ = nullptr;
    y_table_ = nullptr;
    x_table_ = nullptr;
  }
  int get(uint32_t x, uint32_t y);
  void set(uint32_t x, uint32_t y, int val);
  int zoc_path(int x, int y, int faction_id);
  int find(int x_src, int y_src, int x_dst, int y_dst, int proto_id, int faction_id, int unk1, 
           int unk2);
  int move(int veh_id, int faction_id);
  void make_abstract();
  void merge(int region_old, int region_new);
  void territory(int x, int y, int region, int faction_id);
  void continent(int x, int y, int region);
  void continents();
  BOOL sensors(int faction_id, int *x_sensor, int *y_sensor);

 private:
  // ANCHORED FROM THE IMAGE, not from the ABI alone. `Path::shutdown`
  // (0x0059A2D0) frees `[esi]`, `[esi+4]` and `[esi+8]` in this
  // declaration's order, then nulls them as `[esi]`, `[esi+8]`, `[esi+4]` -
  // which is why the body above spells y_table_ before x_table_. Three
  // members at three known offsets, and the first of them is 0.
  int *map_table_;  // 0x0
  int16_t *x_table_;
  int16_t *y_table_;
  int index1_; // specific territory count
  int index2_; // overall territory count
  int faction_id_;
  int x_dst_;
  int y_dst_;
  int unk_;
  int faction_id_2_;
  int proto_id_;
};

// global
extern Path Paths;                             // 0x00945B00
