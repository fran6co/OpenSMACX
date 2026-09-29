#include "stdafx.h"
#include "multiwin.h"
#include "game.h"
#include "console.h"
#include "wave.h"

// The original show/hide pair publishes its visible state here. The zero
// initializer and both writers are retained in the pinned image and seeds.
// RECOVERY_OBJECT: v2 0x007CFF04 0x007CFF08 ?MultiWindowVisibilityState@@3HA 0x00479970
int MultiWindowVisibilityState;

// Shared animation frame interval (20 ms in the pinned image). Other window
// animations also read this original data object.
// RECOVERY_OBJECT: v2 0x00691AF8 0x00691AFC ?WindowAnimationFrameDelay@@3HA 0x00479C00
int WindowAnimationFrameDelay = 20;

// 0x0047A590 and 0x0047A430. The compiler owns construction, exception
// cleanup and reverse destruction of the subobjects declared in multiwin.h.
// Historical mapped seed units remain under src/recovered/units/.
// ORIGINAL: 0x0047A590 ??0MultiWin@@QAE@XZ 0x0047A590-0x0047A6B9;0x00656310-0x006563DE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0047a590.cpp; docs/recovery/subobject-sites.csv; src/multiwin.h"], "reason": "The original constructor builds a GraphicWin front and the owned Buffer, Sprite, FlatButton, Font and Spot subobjects in the observed order."}
MultiWin::MultiWin() {}
// ORIGINAL: 0x0047A430 ??1MultiWin@@QAE@XZ 0x0047A430-0x0047A58A;0x00656230-0x0065630E
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0047a430.cpp; docs/recovery/subobject-sites.csv; src/multiwin.h"], "reason": "The original destructor tears down the Spot, Fonts, FlatButtons, Sprites, Buffer and GraphicWin front in reverse ownership order."}
// symbol    ??1MultiWin@@UAE@XZ
MultiWin::~MultiWin() {}

// 0x00477E70: Spot::check returns the hovered position and type through its
// two output arguments. Only type 1 changes the highlighted player.
// ORIGINAL: 0x00477E70 ?on_mouse_move@MultiWin@@QAEXHH@Z 0x00477E70-0x00477EDB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00477e70.cpp; docs/recovery/member-accesses.csv"], "reason": "The original checks the owned Spot for a player hover and redraws when the selected position changes."}
void MultiWin::on_mouse_move(int x, int y) {
  int position;
  int type;
  if (spot_.check(x, y, &position, &type) != -1) {
    if (type == 1 && hovered_spot_ != position) {
      hovered_spot_ = position;
      draw(0);
    }
  } else if (hovered_spot_ != -1) {
    hovered_spot_ = -1;
    draw(0);
  }
}

// 0x00477EE0 and 0x004798F0: Win callbacks selected by MultiWin's table.
// ORIGINAL: 0x00477EE0 ?on_mouse_leave@MultiWin@@QAEXHH@Z 0x00477EE0-0x00477EFE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00477ee0.cpp; docs/recovery/member-accesses.csv"], "reason": "The original resets the hovered player position and redraws on mouse leave."}
// symbol    ?on_mouse_leave@MultiWin@@UAEXHH@Z
void MultiWin::on_mouse_leave(int, int) {
  if (hovered_spot_ >= 0) {
    hovered_spot_ = -1;
    draw(0);
  }
}

// ORIGINAL: 0x004798F0 ?on_redraw@MultiWin@@QAEXXZ 0x004798F0-0x004798F8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004798f0.cpp; src/win.h"], "reason": "The original Win redraw callback forwards the zero draw mode to MultiWin draw."}
// symbol    ?on_redraw@MultiWin@@UAEXXZ
void MultiWin::on_redraw() { draw(0); }

// 0x00477F00. The seed's chat and council dependencies still lack typed
// product declarations; keep the override explicit for the vtable.
// ORIGINAL: 0x00477F00 ?on_button_clicked@MultiWin@@QAEXH@Z 0x00477F00-0x00477F30
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00477f00.cpp; src/win.h"], "reason": "The original button callback branches on two button IDs to chat and council actions; the product override remains explicitly unfinished."}
// symbol    ?on_button_clicked@MultiWin@@UAEXH@Z
void MultiWin::on_button_clicked(int) {
  // BODY GOES HERE.
}

// 0x00479C00. The Win slot-2 override hides the multi-player panel, restores
// its background and runs the original optional slide-out animation.
// ORIGINAL: 0x00479C00 ?hide@MultiWin@@QAEXXZ 0x00479C00-0x00479F8E;0x0065640A-0x0065641F
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00479c00.cpp; src/win.h; src/buffer.h; src/wave.cpp"], "reason": "The original panel teardown uses its real GraphicWin Buffer extent, owned animation buffer and sprites, typed console/screen globals, and Win/GraphicWin callbacks. The retained seed preserves the historical byte obligation until remeasurement."}
// symbol    ?hide@MultiWin@@UAEXXZ
void MultiWin::hide() {
  Buffer background;
  if (IsMultiplayerNet == 0 && field_A24_ == 0) return;

  if (GamePreferences & 0x400) g_MULTIWIN_WAVE.play();
  hovered_spot_ = -1;
  MultiWindowVisibilityState = 0;

  const int width = dib_.bmiHeader.biWidth;
  const int height = -dib_.bmiHeader.biHeight;
  background.resize(width, height);
  background.fill(WinFillColour);
  update_window_to_buffer(&background);
  on_redraw();

  RECT screen_rect = rect2_;
  client_to_screen(&screen_rect);
  RECT console_rect = screen_rect;
  ConsoleGlobal.screen_to_client(&console_rect);

  buffer_.init(width, height, 0, 0);
  static_cast<Buffer *>(&ConsoleGlobal)->copy(
      &buffer_, console_rect.left, console_rect.top, 0, 0, width, height);

  int limit = screen_rect.bottom - sprite3_.height();
  if (IsMultiplayerNet != 0) limit -= 14 + sprite4_.height();
  const int slide = (limit - screen_rect.top) / 5;
  ScreenBuffer.set_clip(&screen_rect);

  if ((GamePreferences & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    WinFlipClipped = 1;
    int position = screen_rect.top;
    for (int frame = 0; frame < 6; ++frame) {
      const DWORD start = timeGetTime();
      buffer_.copy(&ScreenBuffer, 0, 0, screen_rect.left, screen_rect.top,
                   width, height);
      background.draw(&ScreenBuffer, 9, screen_rect.left, position, 1, 1);
      Win::flip(&screen_rect);
      position += slide;
      if (position >= limit) position = limit;
      do_sound();
      while (timeGetTime() - start < static_cast<DWORD>(WindowAnimationFrameDelay)) {
        do_sound();
      }
    }
    if (position < limit) {
      buffer_.copy(&ScreenBuffer, 0, 0, screen_rect.left, screen_rect.top,
                   width, height);
      static_cast<Buffer *>(this)->draw(
          &ScreenBuffer, 9, screen_rect.left, limit, 1, 1);
      Win::flip(&screen_rect);
    }
    ScreenBuffer.set_clip(&WinScreenClipRect);
    WinFlipClipped = 0;
  }

  if (GamePreferences & 0x400) g_MULTIWIN_WAVE.fade(50);
  field_A24_ = 0;
  if (IsMultiplayerNet == 0) {
    Win::hide();
  } else {
    flat_button2_.hide();
    flat_button1_.hide();
    on_redraw();
    on_update();
  }
  buffer_.close();
}
