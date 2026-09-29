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
#include "mapwin.h"
#include "buffer.h"

#include "graphicwin.h"

 /*
  * WorldWin class
  *
  * Ordinary GraphicWin base, two embedded Buffers and a complete MapWin.
  * Construction at 0x004C4BF0 and destruction at 0x004C4A70 establish that
  * ownership order. MapWin has its own virtual GraphicWin base; WorldWin
  * does not gain another virtual-base construction flag from containing it.
  * The known member layout does not yet establish the complete object extent.
  */
class WorldWin : public GraphicWin {
  friend class MapWin;  // MapWin::init identifies the embedded world map.
 public:
  void UNK1();
  void right_menu(int x, int y);
  void click(int a1, int a2, int button, int is_double);
  void draw_box();
  virtual void on_redraw();
  int pixel_to_tile(int pixel_x, int pixel_y, int *tile_x, int *tile_y);
  int tile_to_pixel(int tile_x, int tile_y, int *pixel_x, int *pixel_y);
  // 0x004C4230 regenerates and blits the overview image.
  void refresh_world_map(int resize_buffers);
  // 0x004C47F0 draws the projected viewport outline in the climate window.
  void draw_viewport_outline();

 public:
  void follow_center(int xCoord, int yCoord);
  void focus(int x_coord, int y_coord);
  void set_center(int a1, int a2);
  void set_detail_map();
  void set_world_map();
  void show_all();
  void hide_all();
  // Original CRT initializer 0x004C38B0 constructs WorldClimateWorldWin.
  // Declared members, including mapWin_'s virtual base, construct automatically.
  WorldWin();
  void init(int reset_map);
  // Implicit destruction owns mapWin_, both Buffers and the GraphicWin base.
  // Its naturally emitted 0x004C4A70 identity is mapped in worldwin.cpp.
  void clear_terrain();
  void on_left_double_click(int a1, int a2);
  void on_left_click(int a1, int a2);
  void on_right_click(int a, int b);

  // Storage the image proves is here: its own methods reach 0x239D0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 5 member(s) from the IDA database, 3 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  Buffer buffer1_;  // 0xA1C
  Buffer buffer2_;  // 0xFA4
  MapWin mapWin_;  // 0x152C
  uint8_t field_239AC_[0x10];  // 0x239AC
  uint32_t field_239BC_;  // 0x239BC
  uint32_t field_239C0_;  // 0x239C0
  // Signed pixel origin used by the coordinate converters and projected
  // viewport painter; zero/zero selects the full-world view.
  // RECOVERY_MEMBER: v1 WorldWin field_239C4_ 0x239C4 0x004C43F0
  int32_t field_239C4_;  // 0x239C4
  // RECOVERY_MEMBER: v1 WorldWin field_239C8_ 0x239C8 0x004C43F0
  int32_t field_239C8_;  // 0x239C8
  // Remembered map choice, not current visibility: set_detail_map writes 1,
  // set_world_map (0x004C3990) writes 0, and show_all (0x004C39D0) tests it.
  // hide_all (0x004C3A40) hides both views without changing the selection.
  // RECOVERY_MEMBER: v1 WorldWin detail_map_selected_ 0x239CC 0x004C3950
  uint32_t detail_map_selected_;  // 0x239CC

  // Storage the image proves is here: its own methods reach 0x239D4.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // UNK3 (0x004C47F0) gates the projected detail-map viewport outline on
  // this dword. Vehicle movement enables it; WorldWin/Console init clear it.
  // RECOVERY_MEMBER: v1 WorldWin viewport_outline_enabled_ 0x239D0 0x004C47F0
  uint32_t viewport_outline_enabled_;  // 0x239D0
};


// The process-wide WorldWin. Its storage is defined once, in map.cpp, and the
// image keeps it at 0x008E9F60; game.cpp tears it down through this
// declaration (`desktop_close`, `game_reload`) and world_climate clears its
// terrain through the same object. Before this declaration map.cpp's
// definition was reachable only as a private spelling and game.cpp reached
// the address as a cast literal.
extern WorldWin WorldClimateWorldWin;  // 0x008E9F60
extern RECT WorldMapBounds;  // 0x00939F08; copied from MainInterface.
// Read/set/clear guard for WorldWin::refresh_world_map. It is a distinct
// four-byte object immediately after, not an alias of, MapInputEnabled.
extern int WorldWinRebuildBusy;  // 0x0090D93C

// WorldWin's shared click handler is not recovered.
