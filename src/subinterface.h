/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once
#include "original_seam.h"

class GraphicWin;

// The original abstract interface subobject. Its pure table has nineteen
// slots; concrete window owners provide their callbacks (including measured
// shared return stubs) in their real class definitions.
class SubInterface {
 public:
  void delete_iface_mode();
  void set_iface_mode();
  void release_iface_mode();

  virtual void on_status(int mode) = 0;                              // 0
  virtual void on_iface_scrolled(int, int) = 0;                      // 1
  virtual void on_iface_scrolling(int, int) = 0;                     // 2
  virtual void on_iface_left_click(int, int) = 0;                    // 3
  virtual void on_iface_right_click(int, int) = 0;                   // 4
  virtual void on_iface_left_down(int, int) = 0;                     // 5
  virtual void on_iface_left_up(int, int) = 0;                       // 6
  virtual void on_iface_right_down(int, int) = 0;                    // 7
  virtual void on_iface_mouse_move(int, int) = 0;                    // 8
  virtual void on_iface_mouse_leave(int, int) = 0;                   // 9
  virtual void on_iface_selected(int, int) = 0;                      // 10
  virtual void on_iface_left_double_click(int, int) = 0;             // 11
  virtual void on_iface_right_double_click(int, int) = 0;            // 12
  virtual void on_iface_button_clicked(int) = 0;                     // 13
  virtual int on_iface_button_pressed(int) = 0;                      // 14
  virtual void on_iface_button_toggled(int, int) = 0;                // 15
  virtual void on_iface_group_clicked(int, int, int) = 0;            // 16
  virtual int on_iface_dialog_back_draw(GraphicWin *) = 0;            // 17
  virtual int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                             RECT *) = 0;             // 18

 protected:
  uint32_t field_4_;  // 0x4, used by set_iface_mode/release_iface_mode
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(SubInterface) == 0x8,
              "legacy interface subobject must remain eight bytes");
#endif

typedef void (OriginalObject::*func_sub_interface_iface_mode)();
extern func_sub_interface_iface_mode SubInterfaceSetIfaceMode;
extern func_sub_interface_iface_mode SubInterfaceReleaseIfaceMode;
