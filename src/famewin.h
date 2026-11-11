#pragma once

#include "graphicwin.h"
#include "spot.h"
#include "flatbutton.h"
#include "buttongroup.h"
#include "sprite.h"

// The constructor at 0x0044B200 constructs GraphicWin at +0, then Spot,
// two FlatButton runs, ButtonGroup, a final FlatButton, two Buffers and
// twelve 0x2C-byte Sprites. The destructor at 0x0044B100 tears them down
// in reverse order. The remaining scalar fields are not yet named.
// RECOVERY_BASE: v1 FameWin GraphicWin 0x0 0x0044B200
class FameWin : public GraphicWin {
 public:
  friend void __cdecl hall_of_fame(int add_current_player);
  FameWin();

  int init();
  void close();
  virtual void on_redraw();
  virtual void on_button_clicked(int button_id);
  void on_group_clicked(int, int, int) override;
  void on_left_click(int x, int y) override;
  int on_key_click(int key, int code);
  void try_to_add_me();
  void demote(int rank);

 private:
  uint32_t field_A14_;
  uint32_t field_A18_;
  uint32_t field_A1C_;
  uint32_t field_A20_;
  // +0xA24: where the 800x600 hof.pcx frame sits, built from +0xA14..A20.
  RECT frame_rect_;
  // +0xA34: scratch cell rectangle for button and rank-row placement.
  RECT cell_rect_;
  uint32_t field_A44_;
  uint32_t field_A48_;
  uint32_t factionID_;
  // RECOVERY_MEMBER: v2 FameWin spot_ Spot 0xA50 0x0044B200
  Spot spot_;
  // RECOVERY_MEMBER: v4 FameWin flatButtons1_ FlatButton[7] 0xA5C 0x4F14 0x0044B200
  FlatButton flatButtons1_[7];
  // RECOVERY_MEMBER: v2 FameWin buttonGroup_ ButtonGroup 0x5970 0x0044B200
  ButtonGroup buttonGroup_;
  // RECOVERY_MEMBER: v4 FameWin flatButtons2_ FlatButton[6] 0x5A04 0x43C8 0x0044B200
  FlatButton flatButtons2_[6];
  // RECOVERY_MEMBER: v2 FameWin flatButton3_ FlatButton 0x9DCC 0x0044B200
  FlatButton flatButton3_;
  uint32_t selectedRecord_;
  // RECOVERY_MEMBER: v2 FameWin buffer1_ Buffer 0xA91C 0x0044B200
  Buffer buffer1_;
  // RECOVERY_MEMBER: v2 FameWin buffer2_ Buffer 0xAEA4 0x0044B200
  Buffer buffer2_;
  // +0xB42C..+0xB488: six panel rectangles laid out by init (0x0044858A..)
  // relative to frame_rect_.
  RECT ranks_rect_;       // 0xB42C: the five-record rank list
  RECT portrait_rect_;    // 0xB43C: leader portrait
  RECT details_rect_;     // 0xB44C: award and statistics text (clip)
  RECT factions_rect_;    // 0xB45C: seven faction buttons
  RECT awards_rect_;      // 0xB46C: six award buttons
  RECT landscape_rect_;   // 0xB47C: diplomacy landscape art
  // RECOVERY_MEMBER: v4 FameWin sprites_ Sprite[12] 0xB48C 0x210 0x0044B200
  Sprite sprites_[12];
};

static_assert(sizeof(FameWin) == 0xB69C,
              "FameWin layout must match constructor and destructor evidence");

// The original dynamic initializer names g_FAMEWIN (0x004483D0).
extern FameWin g_FAMEWIN;
