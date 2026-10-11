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
  int exec(int x, int y, int faction, int kind, int selected,
           int chassis, int weapon, int armor, int abilities, int reactor,
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
  // RECOVERY_MEMBER: v1 SelectPartWin faction_id_ 0x2B68 0x00434006
  int faction_id_;
  int field_2B6C_;               // cells per row
  int field_2B70_;
  int field_2B74_;               // total entry count
  int field_2B78_;
  int field_2B7C_;
  int field_2B80_;               // highlighted index
  // RECOVERY_MEMBER: v1 SelectPartWin part_kind_ 0x2B84 0x0043402D
  int part_kind_;
  // RECOVERY_MEMBER: v1 SelectPartWin chassis_id_ 0x2B88 0x0043400F
  int chassis_id_;
  // RECOVERY_MEMBER: v1 SelectPartWin weapon_id_ 0x2B8C 0x00434018
  int weapon_id_;
  // RECOVERY_MEMBER: v1 SelectPartWin armor_id_ 0x2B90 0x00434021
  int armor_id_;                 // armor argument slot; no observed reader
  // RECOVERY_MEMBER: v1 SelectPartWin ability_flags_ 0x2B94 0x00434033
  int ability_flags_;
  // RECOVERY_MEMBER: v1 SelectPartWin reactor_id_ 0x2B98 0x00434039
  int reactor_id_;
  // RECOVERY_MEMBER: v2 SelectPartWin caviar_ Caviar 0x2B9C 0x004328B0
  Caviar caviar_;                 // +0x2B9C
  uint32_t field_3F6C_[64];     // unknown state, +0x3F6C..+0x406B
  // RECOVERY_MEMBER: v2 SelectPartWin buffer_ Buffer 0x406C 0x004328B0
  Buffer buffer_;                 // +0x406C
  // RECOVERY_MEMBER: v2 SelectPartWin font1_ Font 0x45F4 0x004328B0
  Font font1_;                    // +0x45F4
  // RECOVERY_MEMBER: v2 SelectPartWin font2_ Font 0x461C 0x004328B0
  Font font2_;                    // +0x461C
  // Native exec stores SpotlightColorTable, independently of its palette
  // argument. No consumer of this pointer has been established.
  // RECOVERY_MEMBER: v1 SelectPartWin palette_ 0x4644 0x0043403F
  unsigned char *palette_;
  int field_4648_;               // draw/hover result
};

static_assert(sizeof(SelectPartWin) == 0x464C,
              "SelectPartWin must retain the original object extent");
