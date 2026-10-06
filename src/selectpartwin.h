#pragma once

#include "graphicwin.h"
#include "scroll.h"
#include "caviar.h"
#include "font.h"

// The original table at 0x66B2B8 replaces GraphicWin slots 0, 12, 17,
// 21, 25, 48 and 49. Win still names four of them provisionally as vslot_N;
// these overrides use that real inherited dispatch, not a second vtable.
// RECOVERY_BASE: v1 SelectPartWin GraphicWin 0x0 0x004328B0
class SelectPartWin : public GraphicWin {
 public:
  SelectPartWin();
  ~SelectPartWin();

  void close();
  int draw_part(int index, int selected);
  int draw_chassis(int index, RECT *rect, int selected);
  int draw_weapon(int index, RECT *rect, int selected);
  int draw_armor(int index, RECT *rect, int selected);
  int draw_reactor(int index, RECT *rect, int selected);
  int exec(int x, int y, int value0, int kind, int selected,
           int value5, int value6, int value7, int value8, int value9,
           unsigned char *palette);

  virtual void on_redraw();
  virtual void on_mouse_move(int x, int y);
  virtual void on_left_up(int x, int y);
  int on_key_click(int unused, int key) override;
  virtual void on_scrolled(int unused, int row);
  virtual void on_scrolling(int unused, int row);

  // RECOVERY_MEMBER: v2 SelectPartWin scroll_ Scroll 0xA14 0x004328B0
  Scroll scroll_;                 // +0x0A14
  int field_2B60_;               // first visible index
  int field_2B64_;               // visible cell count
  int field_2B68_;               // faction id
  int field_2B6C_;               // cells per row
  int field_2B70_;
  int field_2B74_;               // total entry count
  int field_2B78_;
  int field_2B7C_;
  int field_2B80_;               // highlighted index
  int field_2B84_;               // part kind
  int field_2B88_;               // chassis
  int field_2B8C_;               // weapon
  int field_2B90_;
  int field_2B94_;
  int field_2B98_;
  // RECOVERY_MEMBER: v2 SelectPartWin caviar_ Caviar 0x2B9C 0x004328B0
  Caviar caviar_;                 // +0x2B9C
  uint32_t field_3F6C_[64];     // unknown state, +0x3F6C..+0x406B
  // RECOVERY_MEMBER: v2 SelectPartWin buffer_ Buffer 0x406C 0x004328B0
  Buffer buffer_;                 // +0x406C
  // RECOVERY_MEMBER: v2 SelectPartWin font1_ Font 0x45F4 0x004328B0
  Font font1_;                    // +0x45F4
  // RECOVERY_MEMBER: v2 SelectPartWin font2_ Font 0x461C 0x004328B0
  Font font2_;                    // +0x461C
  uint32_t field_4644_;
  int field_4648_;               // draw/hover result
};

static_assert(sizeof(SelectPartWin) == 0x464C,
              "SelectPartWin must retain the original object extent");
