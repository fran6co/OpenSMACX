#pragma once

#include "stdafx.h"
#include "buffer.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "spot.h"
#include "sprite.h"

// Native477C80/477B10 establish actual bases and ordered member lifetimes.
class MonuWin : public GraphicWin {
  friend void __cdecl monument(int);
 public:
  MonuWin();
  ~MonuWin();
  int init();
  void on_redraw() override;
  void close();
  void on_left_click(int x, int y) override;
  void on_mouse_move(int x, int y) override;
  void on_button_clicked(int id) override;
  int on_key_click(int key, int state) override;

 private:
  int origin_x_;                 // +0xA14
  int origin_y_;                 // +0xA18
  int window_width_;             // +0xA1C
  int window_height_;            // +0xA20
  int client_left_;              // +0xA24
  int client_top_;               // +0xA28
  int client_right_;             // +0xA2C
  int client_bottom_;            // +0xA30
  int field_A34_;
  int field_A38_;
  int field_A3C_;
  int field_A40_;
  int frame_or_page_;            // +0xA44
  int field_A48_;
  int selection_pending_;        // +0xA4C
  int preview_faction_or_page_;  // +0xA50
  int selected_monument_type_;   // +0xA54
  FlatButton buttons_[2];
  Spot spot_;
  Buffer buffers_[2];
  Sprite sprites_2C0C_[15];
  Sprite sprites_2EA0_[15];
  RECT rects_3134_[17];
  Sprite sprites_3244_[6];
  RECT icon_rects_[6];
  Sprite sprites_33AC_[7];
  Sprite sprites_34E0_[7];
  Sprite sprites_3614_[2];
  RECT rects_366C_[7];
};

static_assert(sizeof(MonuWin) == 0x36DC,
              "MonuWin must match the pinned 32-bit object extent");

extern MonuWin MonumentWindowGlobal;
extern int MonumentWindowCount;
