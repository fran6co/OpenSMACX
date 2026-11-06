#include "stdafx.h"
#include "factionprofilewin.h"

#include "faction.h"
#include "font.h"
#include "popup_fonts.h"
#include "game.h"
#include "sprite.h"
#include "text.h"
#include "rectangle_operations.h"

#include <string.h>

// ORIGINAL: 0x00589750 sub_589750 0x00589750-0x00589760 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00589750-0x00589760 stores its argument at receiver+0xA34", "source:src/factionprofilewin.h RECOVERY_MEMBER faction_id_ at +0xA34, anchored by 0x00589760", "pinned/current owner:0x00589760 initialize writes faction_id_ and profile_mode_ at +0xA34/+0xA38"], "reason": "Carry the one-word receiver store into the existing FactionProfileWin member that owns the anchored faction_id_ field; class role and method spelling remain provisional."}
// symbol    ?set_faction_id@FactionProfileWin@@QAEXH@Z
// kind      game
void FactionProfileWin::set_faction_id(int faction_id) {
  faction_id_ = faction_id;
}

// ORIGINAL: 0x00589760 sub_589760 0x00589760-0x0058986A;0x00660240-0x00660255
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00589760-0x0058986A stores the arguments at receiver+0xA34/+0xA38, frame fields and RECT, then calls GraphicWin initialization/fill and the slot-1 show dispatch", "image:0x00660240-0x00660255 is the local Buffer unwind funclet and remains the stack lifetime in the source body"], "reason": "Carry the complete seed identity and both image spans into the typed initialize member. FactionProfileWin remains a provisional role name; original class identity and allocation/caller registration are unproved."}
// symbol    ?initialize@FactionProfileWin@@QAEXHH@Z
#pragma optimize("y", off)
void FactionProfileWin::initialize(int faction_id, int profile_mode) {
  Buffer local_buffer;

  faction_id_ = faction_id;
  profile_mode_ = profile_mode;
  field_A14_ = 5;
  field_A18_ = 0x2D;
  width_ = 0x226;
  height_ = 0x12C;
  if (IfaceScreenWidth >= 1000) {
    width_ = 0x2C0;
    height_ = 0x180;
  }

  set_rectangle_dimensions(&bounds_, 5, 0x2D, width_, height_);
  init(&bounds_, const_cast<char *>("..."), 0x1400000, OpeningWindow, 0,
       0);
  fill(9);
  show(0);
}
#pragma optimize("", on)

// ORIGINAL: 0x00589560 sub_589560 0x00589560-0x00589748
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00589560-0x00589748 reads receiver fields and Popup at +0xA3C, builds the faction profile content and finishes through the update slot", "source:src/faction.h Player formal_name_faction is +0x384; source:src/game.cpp owns InterfaceBoxStyles[51], populated by load_box_sprites; 0x78D690 is entry 9 at 0x28 stride"], "reason": "Carry the complete seed identity and body span into the typed show_profile member, preserving fresh owner-field reads across callback-capable Popup and SpriteBox operations. The class role is provisional and does not claim original identity or allocation registration."}
// symbol    ?show_profile@FactionProfileWin@@QAEHXZ
#pragma optimize("y", off)
int FactionProfileWin::show_profile() {
  RECT local_bounds = clip_rect();
  box_sprite(&local_bounds, &InterfaceBoxStyles[9]);

  set_caption(Players[faction_id_].formal_name_faction);
  StringTemp[0] = 0;
  if (profile_mode_ != 0) {
    strcat(StringTemp, "DATALINKS1");
  } else {
    strcat(StringTemp, "BLURB");
  }
  FactionDiplomacyPortraits[faction_id_]->draw(
      static_cast<Buffer *>(this),
      FactionDiplomacyPortraits[faction_id_]->transparent_index(), 0, 2);

  const int popup_width = width_ - 0xA9;
  popup_.start(Players[faction_id_].filename, StringTemp, -1, 0, 0x400,
               static_cast<GraphicWin *>(this));
  if (profile_mode_ != 0) {
    StringTemp[0] = 0;
    strcat(StringTemp, "DATALINKS2");
    popup_.start(Players[faction_id_].filename, StringTemp, -1, 0, 0x80,
                 static_cast<GraphicWin *>(this));
  }

  Popup *const popup = &popup_;
  Sprite *const popup_portrait = FactionDiplomacyPortraits[faction_id_];
  if (!popup->picture_enabled()) {
    popup->set_caller_picture(popup_portrait);
  } else {
    popup->set_caller_picture(0);
  }
  popup->dialogs().sprite(popup_portrait, 0, 0);
  set_caption(Players[faction_id_].formal_name_faction);
  popup_.set_string_font(PopupMenuItalicFont, PopupMenuBoldFont,
                         PopupMenuFont, 0);
  popup_.set_loc(0xA2, 2);
  popup_.set_width(popup_width);
  popup_.exec(0, 0);

  on_update();
  // The image returns no value (void): header change pending.
  return 0;
}
#pragma optimize("", on)
