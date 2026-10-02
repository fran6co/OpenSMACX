#include "stdafx.h"
#include "multiwin.h"
#include "game.h"
#include "console.h"
#include "faction.h"
#include "palette.h"
#include "wave.h"
#include "base.h"
#include "council.h"
#include "lock.h"
#include "strings.h"
#include "alpha.h"
#include "messagewin.h"
#include "spying_recovery.h"
#include "maininterface.h"
#include "general.h"
#include "statuswin.h"  // Shared source-owned layout initialization flags.

static_assert(offsetof(PlayerData, time_bonus_count) == 0x2054,
              "MultiWin time-bonus source offset");

// ORIGINAL: 0x00479900 ?UNK2@MultiWin@@QAEXXZ 0x00479900-0x00479964 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00479900.cpp", "image:terranx_original.exe:0x00479900", "source:src/maininterface.h:InterfaceRegistry and mode-stack storage; source:src/multiwin.h:owned Sprite offsets; native callback-sensitive mode reload and downward repeated removal", "recovery/tasks/interface-contract-report-closure.json; owner expansion interface-registry-multiwin-small-20261001"], "reason": "Recover the original in its genuine C++ owner using source-owned interface registry and mode stack, real virtual callbacks, or owned MultiWin sprite geometry. Preserve historical seed obligations; this binding grants no current byte or runtime claim."}
void MultiWin::UNK2() {
    const int row_height = sprite5_.height();
    const int footer_height = sprite7_.height();
    const int header_height = sprite3_.height();
    int total = row_height * 7 + header_height + footer_height;
    if (IsMultiplayerNet != 0) {
        total += sprite4_.height() + footer_height + 0x0E;
    }
    field_A38_ = 0;
    field_A3C_ = 0;
    field_A40_ = 0x8A;
    field_A44_ = static_cast<unsigned int>(total);
    StatusFlags |= 0x40;
}

// ORIGINAL: 0x00478630 ?draw_player@MultiWin@@QAEXHHPAURECT@@@Z 0x00478630-0x00478BDB
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00478630.cpp", "src/multiwin.h", "src/faction.h", "src/maininterface.h"], "reason": "The original row painter uses MultiWin's owned Spot, sprite array and first font, typed faction names and treaties, and shared chat colors. Network turn-state suffixes remain pending their global owners."}
// symbol    ?draw_player@MultiWin@@QAEXHHPAUtagRECT@@@Z
void MultiWin::draw_player(int faction_id, int row, RECT *bounds) {
  Buffer *canvas = static_cast<Buffer *>(this);
  canvas->set_font(&font1_, 0, 0, 0);

  const int row_height = bounds->bottom - bounds->top;
  const int icon_left = bounds->left + 7;
  const int text_left = icon_left + (IsMultiplayerNet ? row_height : 0) + 3;
  const int text_right = bounds->right - 10;
  const unsigned int faction_bit = 1u << faction_id;
  if (IsMultiplayerNet && (LockEnableMask & faction_bit)) {
    spot_.add(faction_id, 0, icon_left, bounds->top, row_height, row_height);
    Sprite &icon = sprites2_[0];
    icon.draw(canvas, icon.transparent_index(), icon_left,
              bounds->top + (row_height - icon.height()) / 2, 1, 1);
  }
  spot_.add(faction_id, 1, text_left, bounds->top,
            text_right - text_left, row_height);
  canvas->set_text_color(ChatItemIdTable[faction_id],
                         ChatShadowColors[faction_id], 1, 1);

  StringTemp[0] = 0;
  char row_text[32];
  _itoa(row, row_text, 10);
  strcat(StringTemp, row_text);
  strcat(StringTemp, " ");
  GenderDefault = Players[faction_id].is_leader_female;
  PluralityDefault = 0;
  strcat(StringTemp, Players[faction_id].name_leader);
  RECT text_bounds = *bounds;
  text_bounds.left = text_left;
  text_bounds.right = text_right;
  canvas->write_l(StringTemp, &text_bounds, strlen(StringTemp));

  StringTemp[0] = 0;
  if (!IsMultiplayerNet) {
    const unsigned int treaties =
        PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction_id];
    int label_index = -1;
    if (treaties & 1) label_index = 0x34c / 4;
    else if (treaties & 2) label_index = 0x350 / 4;
    else if (treaties & 4) label_index = 0x354 / 4;
    else if (treaties & 0x10) label_index = 0x358 / 4;
    if (label_index >= 0) strcat(StringTemp, label_get(label_index));
  }
  // Pending: network control and countdown suffixes require the
  // unbound 0x0093E8xx turn state and its PlayerLock ownership.
  if (StringTemp[0]) {
    canvas->write_right_l(StringTemp, &text_bounds, strlen(StringTemp));
  }
}

// ORIGINAL: 0x00478BE0 ?draw_time@MultiWin@@QAEXPAURECT@@H@Z 0x00478BE0-0x00479269
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00478be0.cpp", "src/multiwin.h", "src/console.h", "src/game.h", "src/alpha.h"], "reason": "Recover the clock region's owned hotspot, font and visible turn-state label; the countdown value and timer side effects await binding of the original shared time-control globals."}
// symbol    ?draw_time@MultiWin@@QAEXPAUtagRECT@@H@Z
void MultiWin::draw_time(RECT *bounds, int draw_type) {
  Buffer *canvas = static_cast<Buffer *>(this);
  spot_.add(0, 2, bounds->left, bounds->top,
            bounds->right - bounds->left, bounds->bottom - bounds->top);
  canvas->set_font(&font2_, 0, 0, 0);

  StringTemp[0] = 0;
  if (ConsoleControlTurnActive) {
    strcat(StringTemp, label_get(0x410 / 4));
  } else if ((NetTurnFlags & 0x10) &&
             NetTurnFaction != ConsoleGlobal.faction_id_) {
    strcat(StringTemp, label_get(0x444 / 4));
  } else {
    strcat(StringTemp, label_get(0x40c / 4));
  }
  canvas->set_text_color(0xe4, -1, 1, 1);
  canvas->write_cent_l(StringTemp, bounds, strlen(StringTemp));
  // Pending: draw_type advances the clock in the original. Its
  // 0x0093A960/0x006FF6BC counters and timeout effects have no typed owner.
  (void)draw_type;
}

// ORIGINAL: 0x00479330 ?draw@MultiWin@@QAEXH@Z 0x00479330-0x004798EC
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00479330.cpp", "src/multiwin.h", "src/faction.h", "src/spot.h"], "reason": "Move the panel painter from its fixed-address seed onto MultiWin's owned sprites, Spot and GraphicWin buffer; the two separately mapped row painters remain explicit dependencies."}
void MultiWin::draw(int draw_type) {
  spot_.kill_type(2);
  spot_.kill_type(0);
  spot_.kill_type(1);
  GraphicWin::fill(9);

  Buffer *canvas = static_cast<Buffer *>(this);
  int y = (!IsMultiplayerNet || field_A24_)
              ? 0 : sprite5_.height() * 7 + sprite7_.height() * 2;
  sprite3_.draw(canvas, sprite3_.transparent_index(), 0, y);
  y += sprite3_.height();

  RECT rect;
  if (IsMultiplayerNet) {
    sprite4_.draw(canvas, sprite4_.transparent_index(), 0, y);
    rect.left = 0;
    rect.top = y;
    rect.right = dib_.bmiHeader.biWidth;
    rect.bottom = y + sprite4_.height();
    draw_time(&rect, draw_type);

    rect.top = rect.bottom;
    rect.bottom = rect.top + 14;
    y = rect.bottom;
    sprite5_.draw(canvas, sprite5_.transparent_index(), 0, rect.top);
    canvas->set_clip(&rect);
    spot_.add(3, 2, &rect);

    // The seed reads PlayerData +0x2054: the local faction's time bonus.
    const int time_bonus = PlayersData[ConsoleGlobal.faction_id_].time_bonus_count;
    if (time_bonus > 0) {
      int carets = time_bonus > 3 ? 3 : time_bonus;
      StringTemp[0] = 0;
      strcat(StringTemp, " ");
      while (carets-- > 0) strcat(StringTemp, "^");
      canvas->write_cent_l(StringTemp, &rect, strlen(StringTemp));
    }
    canvas->set_clip(&rect2_);
    y = val_2_;
  }

  const int saved_top = y;
  const int saved_bottom = y + sprite5_.height();
  const int saved_right = dib_.bmiHeader.biWidth;
  bool drew_player = false;
  if (!IsMultiplayerNet || field_A24_) {
    int row = 0;
    for (int faction = 1; faction < 8; ++faction) {
      if (faction == ConsoleGlobal.faction_id_) continue;
      const unsigned int mask = 1u << faction;
      if (!(LockEnableMask & (mask << 8))) continue;
      if (!(GameState & 0x80) &&
          !(PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & 8) &&
          !(SecretProject.empath_guild >= 0 && Bases[SecretProject.empath_guild].faction_id_current == ConsoleGlobal.faction_id_) &&
          !((LockEnableMask & mask) && auto_contact())) continue;

      ++row;
      drew_player = true;
      Sprite &background = hovered_spot_ == faction ? sprite6_ : sprite5_;
      background.draw(canvas, background.transparent_index(), 0, y);
      rect.left = 0;
      rect.top = y;
      rect.right = saved_right;
      rect.bottom = y + sprite5_.height();
      draw_player(faction, row, &rect);
      y += sprite5_.height();
    }
    for (; row < 7; ++row) {
      sprite5_.draw(canvas, sprite5_.transparent_index(), 0, y);
      y += sprite5_.height();
    }

    canvas->set_font(&font1_, 0, 0, 0);
    y += sprites1_[0].height() - sprite5_.height();
    sprite5_.draw(canvas, sprite5_.transparent_index(), 0, y);
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x80c / 4));
    strcat(StringTemp, ": ");
    strcat(StringTemp, " ");
    char votes_text[84];
    _itoa(council_votes(ConsoleGlobal.faction_id_), votes_text, 10);
    strcat(StringTemp, votes_text);
    canvas->set_text_color(0xff, -1, 1, 1);
    if (!(Players[ConsoleGlobal.faction_id_].rule_flags & 0x80)) {
      canvas->write_cent_l(StringTemp, 0, y + 10, 0x8a, strlen(StringTemp));
      y += sprite5_.height();
    }
    if (IsMultiplayerNet) {
      sprite7_.draw(canvas, sprite7_.transparent_index(), 0, y);
      y += sprite7_.height();
    }
    sprite7_.draw(canvas, sprite7_.transparent_index(), 0, y);
  }

  if (!drew_player) {
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x9d0 / 4));
    canvas->set_text_color(MessageWinTextColor, MessageWinTextBackground, 1, 1);
    rect.left = 0;
    rect.top = saved_top;
    rect.right = saved_right;
    rect.bottom = saved_bottom;
    canvas->set_font(find_font(12, 1), 0, 0, 0);
    canvas->write_cent_l(StringTemp, &rect, strlen(StringTemp));
  }
  if (!MultiWindowRedrawSuppressed) soft_update();
}

// The original show/hide pair publishes its visible state here. The zero
// initializer and both writers are retained in the pinned image and seeds.
// RECOVERY_OBJECT: v2 0x007CFF04 0x007CFF08 ?MultiWindowVisibilityState@@3HA 0x00479970
int MultiWindowVisibilityState;

// Shared animation frame interval (20 ms in the pinned image). Other window
// animations also read this original data object.
// RECOVERY_OBJECT: v2 0x00691AF8 0x00691AFC ?WindowAnimationFrameDelay@@3HA 0x00479C00
int WindowAnimationFrameDelay = 20;

// The original draw suppresses its soft update while show calls the redraw callback.
// The original show writes this dword on both sides of that callback;
// MultiWin::draw at 0x00479330 reads it before soft_update.
// RECOVERY_OBJECT: v2 0x007FFF70 0x007FFF74 ?MultiWindowRedrawSuppressed@@3HA 0x00479970
int MultiWindowRedrawSuppressed;

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
// symbol    ?on_mouse_move@MultiWin@@UAEXHH@Z
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

// 0x00477F00. The chat branch is restored; the council dialog remains an
// explicitly unfinished, separately mapped dependency.
// ORIGINAL: 0x00477F00 ?on_button_clicked@MultiWin@@QAEXH@Z 0x00477F00-0x00477F30 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00477f00.cpp", "src/console.cpp", "src/council.cpp", "src/win.h"], "reason": "The original button callback branches on chat and council button IDs. The typed override now preserves both calls; the separately mapped council dialog remains explicitly unfinished."}
// symbol    ?on_button_clicked@MultiWin@@UAEXH@Z
void MultiWin::on_button_clicked(int button_id) {
  switch (button_id) {
    case 0: ConsoleGlobal.chat(0); break;
    case 1: call_council(ConsoleGlobal.faction_id_); break;
  }
}

// 0x00479970. The panel is redrawn into its owned buffer and optionally
// slides upward over the screen before Win publishes it in solo play.
// ORIGINAL: 0x00479970 ?show@MultiWin@@QAEXXZ 0x00479970-0x00479BF1;0x006563F5-0x0065640A
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00479970.cpp; src/multiwin.h; src/faction.h; src/console.h; src/wave.cpp"], "reason": "The original show uses MultiWin's owned controls and GraphicWin buffer, the local Console faction, Player rule flags, process window animation state, and the source-owned wave, screen, visibility and frame-delay objects."}
void MultiWin::show() {
  Buffer background;
  if (IsMultiplayerNet == 0 && field_A24_ != 0) return;

  hovered_spot_ = -1;
  // The seed sets each button's Win visibility bit directly. Use the real
  // control interface here; reconcile any callback difference with draw().
  if (ExpansionEnabled == 0 ||
      (Players[ConsoleGlobal.faction_id_].rule_flags & 0x80) == 0) {
    flat_button1_.show(1);
  }
  if (IsMultiplayerNet != 0) flat_button2_.show(1);

  if (GamePreferences & 0x400) g_MULTIWIN_WAVE.play();
  MultiWindowVisibilityState = 2;
  field_A24_ = 1;
  MultiWindowRedrawSuppressed = 1;
  on_redraw();
  MultiWindowRedrawSuppressed = 0;

  const int width = dib_.bmiHeader.biWidth;
  const int height = -dib_.bmiHeader.biHeight;
  background.resize(width, height);
  background.fill(WinFillColour);
  update_window_to_buffer(&background);

  RECT screen_rect = rect2_;
  client_to_screen(&screen_rect);
  int position = screen_rect.bottom - sprite3_.height();
  if (IsMultiplayerNet != 0) position -= 14 + sprite4_.height();
  const int slide = (position - screen_rect.top) / 5;
  ScreenBuffer.set_clip(&screen_rect);

  if ((GamePreferences & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    WinFlipClipped = 1;
    for (int frame = 0; frame < 6; ++frame) {
      const DWORD start = timeGetTime();
      ScreenBuffer.draw(&background, 9, screen_rect.left, position, 1, 1);
      Win::flip(&screen_rect);
      position -= slide;
      do_sound();
      while (timeGetTime() - start < static_cast<DWORD>(WindowAnimationFrameDelay)) {
        do_sound();
      }
    }
    ScreenBuffer.set_clip(&WinScreenClipRect);
    WinFlipClipped = 0;
  }

  if (GamePreferences & 0x400) g_MULTIWIN_WAVE.fade(50);
  if (IsMultiplayerNet == 0) Win::show(3);
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
