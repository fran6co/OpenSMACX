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
  FameWin();
  ~FameWin();

  int init();
  void close();
  virtual void on_redraw();
  virtual void on_button_clicked(int button_id);
  void on_group_clicked(int, int, int);
  void on_left_click(int x, int y);
  int on_key_click(int key, int code);
  void try_to_add_me();
  void demote(int rank);

 private:
  uint32_t field_A14_;
  uint32_t field_A18_;
  uint32_t field_A1C_;
  uint32_t field_A20_;
  uint32_t field_A24_;
  uint32_t field_A28_;
  uint32_t field_A2C_;
  uint32_t field_A30_;
  uint32_t field_A34_;
  uint32_t field_A38_;
  uint32_t field_A3C_;
  uint32_t field_A40_;
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
  uint32_t field_B42C_;
  uint32_t field_B430_;
  uint32_t field_B434_;
  uint32_t field_B438_;
  uint32_t field_B43C_;
  uint32_t field_B440_;
  uint32_t field_B444_;
  uint32_t field_B448_;
  uint32_t field_B44C_;
  uint32_t field_B450_;
  uint32_t field_B454_;
  uint32_t field_B458_;
  uint32_t field_B45C_;
  uint32_t field_B460_;
  uint32_t field_B464_;
  uint32_t field_B468_;
  uint32_t field_B46C_;
  uint32_t field_B470_;
  uint32_t field_B474_;
  uint32_t field_B478_;
  uint32_t field_B47C_;
  uint32_t field_B480_;
  uint32_t field_B484_;
  uint32_t field_B488_;
  // RECOVERY_MEMBER: v4 FameWin sprites_ Sprite[12] 0xB48C 0x210 0x0044B200
  Sprite sprites_[12];
};

static_assert(sizeof(FameWin) == 0xB69C,
              "FameWin layout must match constructor and destructor evidence");

// The original dynamic initializer names g_FAMEWIN (0x004483D0).
extern FameWin g_FAMEWIN;
