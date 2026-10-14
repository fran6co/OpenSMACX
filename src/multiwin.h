#pragma once

#include "graphicwin.h"
#include "buffer.h"
#include "sprite.h"
#include "flatbutton.h"
#include "font.h"
#include "spot.h"

extern int MultiWindowRedrawSuppressed;
// Shared contact state owned by multiwin.cpp.
extern int MultiWindowContactStatusMask;


// The constructor at 0x0047A590 constructs GraphicWin first, then Buffer,
// two three-element Sprite arrays, five more Sprites, two FlatButtons, two
// Fonts and Spot. The destructor at 0x0047A430 destroys them in reverse.
// These are owned subobjects, not views over a byte buffer.
// The catalogue's MultiWin::UNK3/UNK4 at 0x0047A6F0/0x0047A720 are not
// MultiWin receiver methods: draw calls them on +0x28AC (Spot) and +0x444
// (GraphicWin's Buffer base), and each body forwards ECX without adjustment.
// Their original identity/byte claims remain in the seed files pending
// review by the Spot and Buffer owners.
// RECOVERY_BASE: v1 MultiWin GraphicWin 0x0 0x0047A590
class MultiWin : public GraphicWin {
 public:
  MultiWin();
  ~MultiWin();

  void draw(int draw_type);
  void draw_time_bonus_markers(RECT *rect);
  void draw_player(int faction_id, int row, RECT *bounds);
  void draw_time(RECT *bounds, int draw_type);
  void on_mouse_move(int x, int y);
  virtual void on_mouse_leave(int x, int y);
  virtual void on_button_clicked(int button);
  virtual void on_redraw();
  void on_left_click(int x, int y) override;
  void on_right_click(int x, int y) override;
  void show();
  void hide();
  void init();
  void UNK2();
  unsigned int panel_open() const { return panel_open_; }

 private:
  // +0xA14..+0xA50: ordinary state. Several meanings remain unresolved.
  unsigned int field_A14_;
  unsigned int field_A18_;
  unsigned int field_A1C_;
  unsigned int field_A20_;
  // RECOVERY_MEMBER: v1 MultiWin panel_open_ 0xA24 0x00479970
  unsigned int panel_open_;
  unsigned int field_A28_;
  unsigned int field_A2C_;
  unsigned int field_A30_;
  unsigned int field_A34_;
  unsigned int field_A38_;
  unsigned int field_A3C_;
  unsigned int field_A40_;
  unsigned int field_A44_;
  unsigned int field_A48_;
  unsigned int field_A4C_;
  unsigned int field_A50_;
  // RECOVERY_MEMBER: v2 MultiWin buffer_ Buffer 0xA54 0x0047A567
  Buffer buffer_;              // +0xA54
  // RECOVERY_MEMBER: v4 MultiWin sprites1_ Sprite[3] 0xFDC 0x84 0x0047A590
  Sprite sprites1_[3];         // +0xFDC
  // RECOVERY_MEMBER: v4 MultiWin sprites2_ Sprite[3] 0x1060 0x84 0x0047A590
  Sprite sprites2_[3];         // +0x1060
  // RECOVERY_MEMBER: v2 MultiWin sprite3_ Sprite 0x10E4 0x0047A526
  Sprite sprite3_;             // +0x10E4
  // RECOVERY_MEMBER: v2 MultiWin sprite4_ Sprite 0x1110 0x0047A517
  Sprite sprite4_;             // +0x1110
  // RECOVERY_MEMBER: v2 MultiWin sprite5_ Sprite 0x113C 0x0047A508
  Sprite sprite5_;             // +0x113C
  int hovered_spot_;           // +0x1168; -1 when no spot is selected
  // RECOVERY_MEMBER: v2 MultiWin sprite6_ Sprite 0x116C 0x0047A4F9
  Sprite sprite6_;             // +0x116C
  // RECOVERY_MEMBER: v2 MultiWin sprite7_ Sprite 0x1198 0x0047A4EA
  Sprite sprite7_;             // +0x1198
  // RECOVERY_MEMBER: v2 MultiWin flat_button1_ FlatButton 0x11C4 0x0047A4D0
  FlatButton flat_button1_;    // +0x11C4
  // RECOVERY_MEMBER: v2 MultiWin flat_button2_ FlatButton 0x1D10 0x0047A4A1
  FlatButton flat_button2_;    // +0x1D10
  // RECOVERY_MEMBER: v2 MultiWin font1_ Font 0x285C 0x0047A47D
  Font font1_;                 // +0x285C
  // RECOVERY_MEMBER: v2 MultiWin font2_ Font 0x2884 0x0047A46E
  Font font2_;                 // +0x2884
  // RECOVERY_MEMBER: v2 MultiWin spot_ Spot 0x28AC 0x0047A45F
  Spot spot_;                  // +0x28AC
};

static_assert(sizeof(MultiWin) == 0x28B8, "MultiWin owned layout");
