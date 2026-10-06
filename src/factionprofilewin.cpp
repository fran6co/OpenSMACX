#include "stdafx.h"
#include "factionprofilewin.h"

#include "faction.h"
#include "font.h"
#include "game.h"
#include "sprite.h"
#include "text.h"

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
// TRIED: no mangled name, so the emitter never declared the receiver as a class member - reads at offsets 0x9c8..0xa38 with the thiscall receiver cached to esi and no matching stack slot mean this is a method on a class derived from GraphicWin (the fields past 0xA14 belong to none of the catalogued classes' real layouts, so this is a new derived type, not Scroll despite the address range overlapping it). Declaring that class here as its own top-level type, with the method defined out of line so VC6 actually emits it, truncates the region this file's own extractor reads (a second top-level type ahead of the definition returns the brace-depth to zero before the real body) or, if the method stays inline in the class body, the compiler drops it as an unreferenced inline entirely - "found 0" either way. Landing it as a plain stdcall function taking the receiver as an explicit pointer keeps the whole thing as ONE top-level construct and one external symbol; the cost is the callee-pop count on the final `ret`, since the original is thiscall and pops two words for itself, not three. A local Buffer object reproduces the SEH prologue, the push -1 / handler / fs colon zero chain, that the original's stack-allocated Buffer constructor and destructor need, with no inline asm required. The one indirect vtable call, slot 1 at offset 4, goes through an empty local class used purely for its member-pointer representation, which draws no C4234 and no extra symbol. 94% mnemonic similarity was reached with the thiscall-member spelling, first divergence at instruction 23 - a cmp/jl versus a reload before the same compare; this stdcall spelling is close behind and the one that actually extracts. Landing the closest MISMATCH. NOTE: literal brace characters in this comment block truncate the region the same way a helper type would - keep it brace-free.
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

  bounds_.left = 5;
  bounds_.top = 0x2D;
  bounds_.right = width_ + 5;
  bounds_.bottom = height_ + 0x2D;
  init(&bounds_, const_cast<char *>("..."), 0x1400000, OpeningWindow, 0,
       0);
  fill(9);
  show(0);
}

// ORIGINAL: 0x00589560 sub_589560 0x00589560-0x00589748
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00589560-0x00589748 reads receiver fields and Popup at +0xA3C, builds the faction profile content and finishes through the update slot", "source:src/faction.h Player formal_name_faction is +0x384; source:src/game.cpp owns InterfaceBoxStyles[51], populated by load_box_sprites; 0x78D690 is entry 9 at 0x28 stride"], "reason": "Carry the complete seed identity and body span into the typed show_profile member, preserving fresh owner-field reads across callback-capable Popup and SpriteBox operations. The class role is provisional and does not claim original identity or allocation registration."}
// symbol    ?show_profile@FactionProfileWin@@QAEHXZ
int FactionProfileWin::show_profile() {
  Buffer &buffer = static_cast<Buffer &>(*this);
  RECT local_bounds = buffer.clip_rect();
  buffer.box_sprite(&local_bounds, &InterfaceBoxStyles[9]);

  const int faction_id = faction_id_;
  const char *const faction_file = Players[faction_id].filename;
  Sprite *const portrait = FactionDiplomacyPortraits[faction_id];

  set_caption(Players[faction_id].formal_name_faction);
  StringTemp[0] = 0;
  if (profile_mode_ != 0) {
    strcat(StringTemp, "DATALINKS1");
  } else {
    strcat(StringTemp, "BLURB");
  }
  portrait->draw(&buffer, portrait->transparent_index(), 0, 2);

  const int popup_width = width_ - 0xA9;
  popup_.start(const_cast<char *>(faction_file), StringTemp, -1, 0, 0x400,
               static_cast<GraphicWin *>(this));
  if (profile_mode_ != 0) {
    StringTemp[0] = 0;
    strcat(StringTemp, "DATALINKS2");
    const int second_section_faction = faction_id_;
    popup_.start(const_cast<char *>(Players[second_section_faction].filename),
                 StringTemp, -1, 0, 0x80,
                 static_cast<GraphicWin *>(this));
  }

  const int picture_faction = faction_id_;
  Sprite *const popup_portrait = FactionDiplomacyPortraits[picture_faction];
  popup_.set_caller_picture(popup_.picture_enabled() ? 0 : popup_portrait);
  popup_.dialogs().sprite(popup_portrait, 0, 0);
  set_caption(Players[faction_id_].formal_name_faction);
  popup_.set_string_font(PopupMenuItalicFont, PopupMenuBoldFont,
                         PopupMenuFont, 0);
  popup_.set_loc(0xA2, 2);
  popup_.set_width(popup_width);
  popup_.exec(0, 0);

  on_update();
  return 0;
}
