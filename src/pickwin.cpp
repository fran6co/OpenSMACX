/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
// ORIGINAL: 0x0048ADC0 ??_GPickWin@@UAEPAXI@Z 0x0048ADC0-0x0048ADE1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x0048ADC0-0x0048ADE1", "Pinned class constructor ??0PickWin@@QAE@XZ at 0x0048AC10 stores original class/view table VA 0x66d13c at 0x0048AD98 (mov dword ptr [esi], 0x66d13c); the current genuine owner vftable ??_7PickWin@@6BWin@@@ selects generated ??_GPickWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPickWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GPickWin@@UAEPAXI@Z at 0x0048ADC0 calls the source-mapped ??1PickWin@@QAE@XZ at 0x0048A8E0, then tests byte ptr [ebp + 8], 1; je 0x48adda bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/pickwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original PickWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPickWin@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "pickwin.h"
#include "strings.h"
#include "general.h"
#include "text.h"
#include "alpha.h"
#include "faction.h"
#include "maininterface.h"
#include "console.h"
#include "palette.h"
#include "popmenu.h"
extern int TextColourTable[195];

// Original caption834D6C is passed synchronously to Win::init, which
// copies it. The pinned PE virtual zero-fill tail proves its initial NUL.
// This mutable one-byte minimum represents that observed initial state,
// NOT original capacity, allocation bounds or absence of indirect writers.
// Recover a larger extent before introducing any writer or longer content.
// RECOVERY_OBJECT: v1 0x00834D6C ?PickWinCaptionBuffer@@3PADA
char PickWinCaptionBuffer[1] = { 0 };

/*
// ORIGINAL: 0x0048AC10 ??0PickWin@@QAE@XZ 0x0048AC10-0x0048ADB6;0x00657E30-0x00657F65
// TRIED: deriving from ConstructedGraphicWin (graphicwin.h) to move the
//   base construction ahead of the members. The ORDER IS RIGHT - this image
//   really does `call 0x5d4cf0` before its first `lea ecx, [esi + N]`, checked
//   2026-08-22 - but making the source agree took this body 21 -> 16
//   of 101. Same shape as Scroll (scroll.cpp), where the reorder alone was
//   worth 1 instruction and the payoff came from two levers underneath that it
//   made reachable; here those levers are not the vtable-immediate one, since
//   these constants already fold, and this body is structurally far off
//   (134 compiled against 101). Re-derive the ordering fix as part
//   of a dedicated pass on the whole body, not on its own.
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` and an extra
//            callee-save (ebx) the image does not; the image keeps `this` in
//            ecx/a stack slot with less spill pressure. MISMATCH, 16/101
//            instructions agree. Not chased further - a source-level rewrite
//            to shed one register's worth of pressure was not found cheaply.
// LEVER: popup_ (sizeof(Popup) == 0x537C matches exactly) and the
//        flatButton1_/2_/3_/sprites1_../4_/sprite5_ run right after it -
//        the whole prefix before the irreducible listBox_ - converted to
//        real declared members, built implicitly. 16/101 -> 21/101; still
//        MISMATCH but a real improvement, unlike the same lever tried on
//        ProdPicker and ReportIf in this batch (both went WORSE - see their
//        own files). listBox_ and everything from flatButton4_ onward stay
//        raw/explicit: listBox_ has no declared one-argument constructor
//        (out of this batch's scope), and real members declared after it
//        would be hoisted into the same implicit preamble as the ones
//        above it, ahead of listBox_'s explicit call, which the image does
//        not do.
// size      731 bytes
// prototype void (__thiscall ??0PickWin@@QAE@XZ)(PickWin* this)
// callers   1   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004048A0 0x005D4CF0 0x005E37E0 0x00607CF0 0x00609DB0 0x0062B7C0 0x006456E4 0x006457C2
*/
// Base and typed members perform the native lifecycle exactly once.
PickWin::PickWin() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00489B10 ?on_button_passover@PickWin@@QAEXHH@Z 0x00489B10-0x00489B13 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_button_passover@PickWin@@QAEXHH@Z)(PickWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void PickWin::on_button_passover(int, int) {
}


/*
Purpose: Step the receiver back to the subobject ??_GPickWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0048ADF0 ??_GPickWin@@WEEE@AEPAXI@Z 0x0048ADF0-0x0048ADFB BYTE_EXACT
// symbol    ??_EPickWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3PickWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0048ADC0` into
//   ??_GPickWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// ORIGINAL: 0x00489B00 ?close@PickWin@@QAEXXZ 0x00489B00-0x00489B05 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The seed body dispatches to GraphicWin::close through the PickWin base subobject; PickWin derives directly from GraphicWin without receiver adjustment."], "reason": "Express the close thunk as an ordinary qualified base call in PickWin's product source."}
void PickWin::close() {
    GraphicWin::close();
}

// ORIGINAL: 0x0048A7C0 ?on_key_click@PickWin@@QAEHHH@Z 0x0048A7C0-0x0048A80D FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The seed uses Win vtable slot 44 for on_button_clicked with 0x103 on Escape and 0x108 on Enter; Escape and other keys clear PickWin field +0x5DA4, while Enter returns before the clear.", "PickWin's class storage declares field_5DA4_ at the measured +0x5DA4 offset, and its inherited on_button_clicked is Win's slot-44 virtual."], "reason": "Home the compact key handler through PickWin's typed field and inherited Win behavior."}
// symbol    ?on_key_click@PickWin@@UAEHHH@Z
int PickWin::on_key_click(int, int key) {
    if (key == 0x1B) {
        on_button_clicked(0x103);
    } else if (key == 0x0D) {
        on_button_clicked(0x108);
        return 1;
    }
    field_5DA4_ = 0;
    return 1;
}

// ORIGINAL: 0x0048A810 ?go_girl@PickWin@@QAEHXZ 0x0048A810-0x0048A83C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The seed's three virtual calls resolve to Win/GraphicWin redraw (slot 62), show (slot 1), and set_modal (slot 57) on the same receiver.", "PickWin's declared field_5DA8_ is the returned value at +0x5DA8."], "reason": "Replace opaque vtable-slot replay with the inherited Win interface and PickWin's typed result field."}
int PickWin::go_girl() {
    redraw();
    show(0);
    set_modal(0, nullptr, nullptr);
    return field_5DA8_;
}


// The original global immediately before PlanWindow is passed only as a
// borrowed C string. Win::init copies the caption immediately into owned
// storage. Its extent and indirect writers have not been established.
extern char PickWinCaptionBuffer[];  // initial VA 0x00834D6C; capacity unknown
extern PickWin PickWinGlobal;        // initialized by the native .CRT wrapper
void __cdecl list_was_dblclicked(int faction_id);

static void pickwin_init_text_button(FlatButton &button, int id, int label,
                                     const RECT &area, bool uppercase,
                                     Win *parent) {
  LPCSTR name = label_get(label);
  if (uppercase) {
    StringTemp[0] = '\0';
    strcat(StringTemp, name);
    CharUpperA(StringTemp);
    name = StringTemp;
  }
  button.init(name, id, area.left, area.top,
              area.right - area.left, area.bottom - area.top, parent, 0);
  static_cast<Buffer &>(button).set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
  button.redraw();
  button.hide();
}

// ORIGINAL: 0x004887A0 ?init@PickWin@@QAEXP6AXH@ZHH@Z 0x004887A0-0x0048965F;0x00657F70-0x00657F85
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004887a0.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// TRIED: nothing exotic here (no x87, no computed jumps) - transcribed mechanically from the Ghidra pseudocode (which is accurate for this straight-line function) using raw `self+offset` field access and real member calls resolved against pickwin.h field names where they matched. sim 0.65-0.71 across flag sets.
void PickWin::init(void (__cdecl *callback)(int), int mode, int faction) {
  Buffer source;

  field_5DAC_ = static_cast<uint32_t>(faction);
  field_6238_ = callback;
  field_5DA0_ = static_cast<uint32_t>(mode);
  field_5DA4_ = 0;

  if (text_open(AlphaxFileID, "FACTIONS") != 0) return;
  for (int faction_file_index = 0; faction_file_index < 7; ++faction_file_index) {
    text_get();
    strcpy(faction_files_[faction_file_index], text_item());
  }
  if (text_open(AlphaxFileID, "NEWFACTIONS") != 0) return;
  for (int new_faction_index = 7; new_faction_index < 14; ++new_faction_index) {
    text_get();
    strcpy(faction_files_[new_faction_index], text_item());
  }

  field_5DB0_ = 0;
  if (text_open(AlphaxFileID, "CUSTOMFACTIONS") != 0) return;
  text_get();
  for (char *entry = text_item(); *entry != '\0'; entry = text_item()) {
    ++field_5DB0_;
    text_get();
  }

  if (text_open(AlphaxFileID, "CUSTOMFACTIONS") != 0) return;
  if (field_5DB0_ < 0) field_5DB0_ = 0;
  else if (field_5DB0_ > 10) field_5DB0_ = 10;
  for (int custom_faction_index = 0; custom_faction_index < field_5DB0_; ++custom_faction_index) {
    text_get();
    strcpy(faction_files_[14 + custom_faction_index], text_item());
  }

  for (uint32_t menu_label_index = 0; menu_label_index < field_5DB0_ + 14; ++menu_label_index) {
    const char *section = faction_files_[menu_label_index];
    if (text_open(section, section) != 0) return;
    text_get();
    text_item();
    strncpy(faction_menu_items_[menu_label_index], text_item(), 0x18);
    faction_menu_items_[menu_label_index][0x17] = '\0';
  }

  source.load_pcx("pickfac.pcx", 0, 10, 0xEC);

  field_A1C_ = 0x102;
  field_A20_ = 0x5A;
  field_A18_ = 1;
  for (int sprite_row_index = 0; sprite_row_index < 3; ++sprite_row_index) {
    const int top = static_cast<int>(field_A18_);
    const int width = static_cast<int>(field_A1C_);
    const int height = static_cast<int>(field_A20_);
    field_A14_ = 1;
    RECT first = {static_cast<LONG>(field_A14_), static_cast<LONG>(top),
                  static_cast<LONG>(field_A14_ + width),
                  static_cast<LONG>(top + height)};
    sprites1_[sprite_row_index].extract(&source, WinFillColour, &first, 0);
    field_A14_ = first.right + 1;
    first.left = static_cast<LONG>(field_A14_);
    first.right = static_cast<LONG>(field_A14_ + width);
    sprites2_[sprite_row_index].extract(&source, WinFillColour, &first, 0);
    field_A14_ = first.right + 1;
    first.left = static_cast<LONG>(field_A14_);
    first.right = static_cast<LONG>(field_A14_ + width);
    sprites3_[sprite_row_index].extract(&source, WinFillColour, &first, 0);
    field_A18_ += height + 1;
  }

  field_A14_ = 1;
  field_A1C_ = 0x10;
  field_A20_ = 0x10;
  field_A18_ = 0x139;
  for (int icon_row_index = 0; icon_row_index < 3; ++icon_row_index) {
    RECT icon = {static_cast<LONG>(field_A14_), static_cast<LONG>(field_A18_),
                 static_cast<LONG>(field_A14_ + field_A20_),
                 static_cast<LONG>(field_A18_ + field_A1C_)};
    sprites4_[icon_row_index].extract(&source, WinFillColour, &icon, 0);
    field_A18_ += field_A1C_ + 1;
  }
  sprite5_.extract(&source, WinFillColour, field_A14_, field_A18_, 7, 0xD, 0);

  field_A1C_ = 0x316;
  field_A20_ = 0x12C;
  field_A14_ = 5;
  field_A18_ = 5;
  GraphicWin::init(5, 5, 0x316, 0x12C, PickWinCaptionBuffer,
                   0x01000000, 0, 0, 0);
  Buffer &canvas = static_cast<Buffer &>(*this);
  canvas.set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
  canvas.set_text_color(TextColourTable[68], TextColourTable[69], 1, 1);

  field_6244_ = field_A1C_;
  field_6248_ = field_A20_;
  field_623C_ = field_6240_ = field_624C_ = 0;
  field_6250_ = field_6240_;
  field_6254_ = field_6244_;
  field_6258_ = field_6248_;
  field_624C_ += 4;
  field_6254_ -= 4;
  field_6250_ += 4;
  field_6258_ -= 4;
  field_6258_ = field_6250_ + 0x1E;
  field_A14_ = field_624C_ + 1;
  field_A18_ = field_6250_ + 0x22;
  field_A20_ = 0x16;
  field_A1C_ = ((field_6254_ - field_624C_) - 6) / 3;
  field_625C_ = field_A14_;
  field_6260_ = field_A18_;
  field_6264_ = field_A14_ + 0x102;
  field_6268_ = field_A18_ + 0x5A;
  field_626C_ = field_6264_ + 2;
  field_6270_ = field_A18_;
  field_6274_ = field_6264_ + 0x104;
  field_6278_ = field_A18_ + 0x5A;
  field_627C_ = field_6264_ + 0x106;
  field_6280_ = field_A18_;
  field_6284_ = field_6264_ + 0x208;
  field_6288_ = field_A18_ + 0x5A;

  const int base = static_cast<int>(field_624C_);
  field_A14_ = base + 0x159;
  field_86AC_ = base + 0x15B;
  field_86B0_ = field_6268_ + 3;
  field_86B4_ = (base + 0x15B) + (field_6254_ - base) - 0x15E;
  field_86B8_ = field_6268_ + 0x8B;
  field_86AC_ += 3;
  field_86B4_ -= 3;
  field_86B0_ += 3;
  field_86B8_ -= 3;
  field_86AC_ -= 0x12;
  field_86B4_ -= 0x12;

  int pos = 0;
  int len;
  int button_rect_index = 0;
  do {
    len = pos + 0x13;
    pos += field_86B0_;
    RECT &area = picker_button_rects_[button_rect_index++];
    area.left = field_86B4_ + 2;
    area.top = pos;
    area.right = field_86B4_ + 0x12;
    area.bottom = pos + 0x10;
    pos = len;
  } while (len < 0x85);

  field_A14_ = field_624C_ + 1;
  field_A18_ = field_86B8_ + 7;
  {
    int w = field_A1C_ * 3;
    field_A1C_ = ((w + ((w >> 31) & 3)) >> 2) - 1;
  }
  field_9220_ = field_A14_;
  field_9224_ = field_A18_;
  field_9228_ = field_A14_ + field_A1C_;
  field_922C_ = field_A18_ + field_A20_;
  field_9210_ = field_9228_ + 2;
  field_9214_ = field_A18_;
  field_9218_ = field_9210_ + field_A1C_;
  field_921C_ = field_A18_ + field_A20_;
  field_9260_ = field_9218_ + 2;
  field_9264_ = field_A18_;
  field_9268_ = field_9260_ + field_A1C_;
  field_926C_ = field_A18_ + field_A20_;
  field_9230_ = field_9268_ + 2;
  field_9234_ = field_A18_;
  field_9238_ = field_9230_ + field_A1C_;
  field_923C_ = field_A18_ + field_A20_;

  field_A1C_ = 0xD8;
  field_A14_ = field_624C_ + 2;
  field_A18_ = field_86B0_ - 2;
  field_9240_ = field_A14_;
  field_9244_ = field_A18_;
  field_9248_ = field_624C_ + 0xDA;
  field_924C_ = field_A18_ + field_A20_;
  field_A18_ += 0x19;
  field_9250_ = field_A14_;
  field_9254_ = field_A18_;
  field_925C_ = field_A18_ + field_A20_;
  field_9258_ = field_A14_ + field_A1C_;

  for (int button = 0; button < 7; ++button) {
    RECT &area = picker_button_rects_[button];
    flatButtons10_[button].init(0, button + 0x3039,
        area.left, area.top, area.right - area.left, area.bottom - area.top,
        this, 0);
    flatButtons10_[button].set_report_icons(&sprites4_[0], &sprites4_[1],
                                            &sprites4_[2]);
    flatButtons10_[button].hide();
  }

  StringTemp[0] = '\0'; strcat(StringTemp, label_get(0x116C));
  flatButton1_.init(0, 0x100, field_625C_, field_6260_,
                    field_6264_ - field_625C_, field_6268_ - field_6260_, this, 0);
  flatButton1_.set_report_icons(&sprites1_[0], &sprites1_[1], &sprites1_[2]);
  flatButton1_.set_bubble_text(StringTemp); flatButton1_.hide();
  StringTemp[0] = '\0'; strcat(StringTemp, label_get(0x1170));
  flatButton2_.init(0, 0x101, field_626C_, field_6270_,
                    field_6274_ - field_626C_, field_6278_ - field_6270_, this, 0);
  flatButton2_.set_report_icons(&sprites2_[0], &sprites2_[1], &sprites2_[2]);
  flatButton2_.set_bubble_text(StringTemp); flatButton2_.hide();
  StringTemp[0] = '\0'; strcat(StringTemp, label_get(0x1174));
  flatButton3_.init(0, 0x102, field_627C_, field_6280_,
                    field_6284_ - field_627C_, field_6288_ - field_6280_, this, 0);
  flatButton3_.set_report_icons(&sprites3_[0], &sprites3_[1], &sprites3_[2]);
  flatButton3_.set_bubble_text(StringTemp); flatButton3_.hide();

  listBox_.close();
  RECT list_area = {static_cast<LONG>(field_86AC_), static_cast<LONG>(field_86B0_),
                    static_cast<LONG>(field_86B4_), static_cast<LONG>(field_86B8_)};
  static_cast<Dialog &>(listBox_).init(&list_area, 0);
  static_cast<Dialog &>(listBox_).field_40_ = 0;
  static_cast<Dialog &>(listBox_).field_44_ = 3;
  static_cast<Dialog &>(listBox_).field_48_ = 0x13;
  static_cast<Dialog &>(listBox_).field_B4_ = 0;
  listBox_.initialize_picker_state();
  static_cast<Dialog &>(listBox_).field_20_ &= ~4u;
  listBox_.double_click_callback_ = list_was_dblclicked;

  RECT area;
  area.left = field_9210_; area.top = field_9214_;
  area.right = field_9218_; area.bottom = field_921C_;
  pickwin_init_text_button(flatButton4_, 0x106, 0x1178, area, false, this);
  area.left = field_9220_; area.top = field_9224_;
  area.right = field_9228_; area.bottom = field_922C_;
  pickwin_init_text_button(flatButton5_, 0x107,
      field_5DAC_ == 0 ? 0x0ABC : 0x117C, area, false, this);
  area.left = field_9230_; area.top = field_9234_;
  area.right = field_9238_; area.bottom = field_923C_;
  pickwin_init_text_button(flatButton6_, 0x103, 0x0198, area, true, this);
  area.left = field_9260_; area.top = field_9264_;
  area.right = field_9268_; area.bottom = field_926C_;
  pickwin_init_text_button(flatButton9_, 0x108, 0x0194, area, true, this);
  area.left = field_9250_; area.top = field_9254_;
  area.right = field_9258_; area.bottom = field_925C_;
  pickwin_init_text_button(flatButton8_, 0x105, 0x1180, area, false, this);
  area.left = field_9240_; area.top = field_9244_;
  area.right = field_9248_; area.bottom = field_924C_;
  pickwin_init_text_button(flatButton7_, 0x104, 0x1184, area, false, this);

  buttonGroup_.init(0, 0);
  buttonGroup_.add(&flatButton7_);
  buttonGroup_.add(&flatButton8_);
  buttonGroup_.set(0x104, 1);
}

// ORIGINAL: 0x00489BA0 ?on_button_clicked@PickWin@@QAEXH@Z 0x00489BA0-0x0048A798;0x00657F85-0x006581C9
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00489ba0.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// TRIED: automatic C++ RAII destruction for Popup/PullDown's ~20 embedded subobjects (Dialogs, EditGroup, SpriteBox, CheckBox, ListBox, Dialog, Sprite, StringStruct x2, FlatButton x2, Heap, nested GraphicWin) - the scaffold's classes represent several of these as raw byte/uint32 padding, so auto-generated dtors would skip real cleanup the original calls explicitly. Used mechanical EBP(x)-buffer-offset transcription (verified against pickwin.h's `Popup popup_` size comment == sizeof(BasePop)+sizeof(Scroll), and cross-checked field offsets against pickwin.h directly) instead, matching each explicit close()/dtor call site.
// symbol    ?on_button_clicked@PickWin@@UAEXH@Z
void PickWin::on_button_clicked(int button) {
  // The image constructs the Popup/PullDown frame locals before dispatching
  // the button id, so retain that lifetime even for non-menu actions.
  PopMenu modal;

  if (button == 0x100 || button == 0x101 || button == 0x102) {
    if (button == 0x100 && text_open(AlphaxFileID, "FACTIONS") != 0) return;
    if (button == 0x101 && text_open(AlphaxFileID, "NEWFACTIONS") != 0) return;

    for (int faction = 1; faction < 8; ++faction) {
      if (button == 0x102) {
        strcpy(Players[faction].filename, "JENN282");
        strcpy(Players[faction].search_key, "JENN282");
      } else {
        read_faction(faction);
      }
    }
    goto build_list;
  }

  if (button == 0x103) {
    release_modal();
    return;
  }
  if (button == 0x108) {
    const int selected = buttonGroup_.field_84_ == 0x105
        ? static_cast<int>(field_5DB4_)
        : listBox_.Dialog::get_selected_id();
    field_5DA8_ = static_cast<uint32_t>(selected);
    field_5DA4_ = buttonGroup_.field_84_ == 0x105 ? 0 : 0x105;
    release_modal();
    return;
  }
  if (button <= 0) return;
  if (button < 8) {
    field_5DA8_ = static_cast<uint32_t>(button);
    field_5DA4_ = 0;
    release_modal();
    return;
  }
  if (button == 0x104 || button == 0x105) return;
  if (button < 0x3039) {
    field_5DA8_ = static_cast<uint32_t>(listBox_.Dialog::get_selected_id());
    field_5DA4_ = static_cast<uint32_t>(button);
    release_modal();
    return;
  }

  {
    int x = 0, y = 0;
    get_mouse_pos(&x, &y);
    client_to_screen(&x, &y);
    modal.init();
    const int menu_count = static_cast<int>(field_5DB0_) + 14;
    for (int menu_item_index = 0; menu_item_index < menu_count; ++menu_item_index) {
      modal.dialogs().item(faction_menu_items_[menu_item_index], menu_item_index);
    }
    modal.dialogs().item(const_cast<char *>(label_get(0x1188)), menu_count);
    const int chosen = modal.exec(x, y, 0);
    redraw();
    if (chosen < 0) return;

    const int faction = button - 0x3038;
    if (chosen == menu_count) {
      strcpy(Players[faction].filename, "JENN282");
      strcpy(Players[faction].search_key, "JENN282");
    } else {
      strcpy(Players[faction].filename, faction_files_[chosen]);
      strcpy(Players[faction].search_key, faction_files_[chosen]);
      read_faction(faction);
    }
  }

build_list:
  listBox_.close();
  RECT list_area = {static_cast<LONG>(field_86AC_), static_cast<LONG>(field_86B0_),
                    static_cast<LONG>(field_86B4_), static_cast<LONG>(field_86B8_)};
  static_cast<Dialog &>(listBox_).init(&list_area, 0);
  static_cast<Dialog &>(listBox_).field_40_ = 0;
  static_cast<Dialog &>(listBox_).field_44_ = 3;
  static_cast<Dialog &>(listBox_).field_48_ = 0x13;
  static_cast<Dialog &>(listBox_).field_B4_ = 0;
  listBox_.initialize_picker_state();
  static_cast<Dialog &>(listBox_).field_20_ &= ~4u;

  for (int faction = 1; faction < 8; ++faction) {
    if (field_5DA0_ != 2 ||
        (factions_status_byte(1) & (1u << faction)) == 0) {
      StringTemp[0] = '\0';
      if (strcmp(Players[faction].formal_name_faction, "JENN282") != 0) {
        strcat(StringTemp, Players[faction].formal_name_faction);
        strcat(StringTemp, "|");
        strcat(StringTemp, "(");
        strcat(StringTemp, Players[faction].desc_name_faction);
        strcat(StringTemp, ")");
        strcat(StringTemp, Players[faction].formal_name_faction);
      } else {
        strcat(StringTemp, label_get(0x1188));
      }
      listBox_.item(StringTemp, faction);
    }
  }

  const int flags = static_cast<Dialog &>(listBox_).field_24_;
  const int top = static_cast<Dialog &>(listBox_).field_28_;
  static_cast<Dialog &>(listBox_).attach(this, flags, top, 0x80020);
  const int selected = (field_5DA0_ & 2) ? ConsoleGlobal.faction_id_
                                         : DefaultPrefs.faction_id;
  listBox_.set_selected_id(selected);
  listBox_.set_selection_callback(field_6238_);
  field_6238_(selected);
  redraw();
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00489B20 ?on_group_clicked@PickWin@@QAEXHHH@Z 0x00489B20-0x00489B94 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00489b20.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
void PickWin::on_group_clicked(int a1, int, int mode) {
  if (a1 != 0) {
    return;
  }

  if (mode == 0x105) {
    field_5DB4_ = listBox_.Dialog::get_selected_id();
    listBox_.set_selected_pos(-1, 1);
  } else {
    listBox_.set_selected_id(field_5DB4_);
  }
  redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048A840 ?on_dialog_item_back_draw@PickWin@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x0048A840-0x0048A8D3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0048a840.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// symbol    ?on_dialog_item_back_draw@PickWin@@QAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
int PickWin::on_dialog_item_back_draw(GraphicWin *window, int, int index,
                                      RECT *area) {
  if (window == nullptr || area == nullptr) {
    return 0;
  }

  Buffer &canvas = static_cast<Buffer &>(*window);
  canvas.restore_clip();
  RECT local = *area;
  canvas.box_sprite(&local, &InterfaceBoxStyles[27 + index]);
  canvas.set_text_color(TextColourTable[80 - 2 * index],
                        TextColourTable[81 - 2 * index], 1, 1);
  return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00489660 ?on_redraw@PickWin@@QAEXXZ 0x00489660-0x00489AF7
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00489660.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// symbol    ?on_redraw@PickWin@@UAEXXZ
void PickWin::on_redraw() {
  GraphicWin::fill(9);
  Buffer &canvas = static_cast<Buffer &>(*this);

  RECT frame = {static_cast<long>(field_623C_), static_cast<long>(field_6240_),
                static_cast<long>(field_6244_), static_cast<long>(field_6248_)};
  canvas.box_sprite(&frame, &InterfaceBoxStyles[9]);
  RECT panel = {static_cast<long>(field_624C_), static_cast<long>(field_6250_),
                static_cast<long>(field_6254_), static_cast<long>(field_6258_)};
  canvas.box_sprite(&panel, &InterfaceBoxStyles[3]);
  panel.left += 3;
  panel.top += 3;
  panel.right -= 3;
  panel.bottom -= 3;
  canvas.box_sprite(&panel, &InterfaceBoxStyles[6]);

  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x45A));
  canvas.write_cent_l(StringTemp, &panel, strlen(StringTemp));
  panel.left -= 3;
  panel.top -= 3;
  panel.right += 3;
  panel.bottom += 3;

  RECT group = {static_cast<long>(field_86AC_ - 3),
                static_cast<long>(field_86B0_ - 3),
                static_cast<long>(field_86B4_ + 0x15),
                static_cast<long>(field_86B8_ + 3)};
  canvas.box_sprite(&group, &InterfaceBoxStyles[3]);

  int selected = listBox_.Dialog::get_selected_id();
  int row_left = field_9244_ + 10;
  int row_bottom = row_left + 0x39;
  int center_x = (field_9240_ + field_9248_) / 2;
  int endpoint_x = (field_86AC_ - 10 + field_9248_) / 2;

  if (buttonGroup_.field_84_ == 0x105) {
    canvas.vline(center_x, field_925C_ - 2, row_bottom - 2, 0xE0);
    canvas.put_pixel(center_x + 1, row_left + 0x38, 0xE0);
    canvas.hline(center_x + 2, endpoint_x, row_left + 0x39, 0xE0);
    canvas.vline(endpoint_x, row_left + 2, row_left + 0x70, 0xE0);
    for (int row = 0; row < 7; ++row) {
      int y = row_left + row * 0x13;
      canvas.put_pixel(endpoint_x + 1, y + (row == 0 ? 1 : row == 6 ? -1 : 0), 0xE0);
      canvas.hline(endpoint_x + 2, field_86AC_ - 10, y, 0xE0);
      sprite5_.draw(&canvas, sprite5_.transparent_index(),
                    field_86AC_ - 10, y - 6);
    }
  } else if (selected > 0 && selected < 8) {
    int row = selected - 1;
    int y = row * 0x13 + row_left;
    canvas.hline(field_9248_, endpoint_x - 2, row_left, 0xE0);
    if (row == 0) {
      canvas.hline(endpoint_x - 2, endpoint_x + 2, row_left, 0xE0);
    } else {
      canvas.put_pixel(endpoint_x - 1, row_left + 1, 0xE0);
      canvas.put_pixel(endpoint_x + 1, y - 1, 0xE0);
      canvas.vline(endpoint_x, row_left + 2, y - 2, 0xE0);
    }
    canvas.hline(endpoint_x + 2, field_86AC_ - 10, y, 0xE0);
    sprite5_.draw(&canvas, sprite5_.transparent_index(),
                  field_86AC_ - 10, y - 6);
  }

  flatButton1_.show(0);
  flatButton2_.show(0);
  flatButton3_.show(0);
  for (int i = 0; i < 7; ++i) {
    flatButtons10_[i].show(0);
  }
  listBox_.show(0);
  flatButton4_.show(0);
  flatButton5_.show(0);
  flatButton6_.show(0);
  flatButton9_.show(0);
  flatButton8_.show(0);
  flatButton7_.show(0);
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048A8E0 ??1PickWin@@QAE@XZ 0x0048A8E0-0x0048AC09;0x00657C70-0x00657E2F
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0048a8e0.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// symbol    ??1PickWin@@UAE@XZ
PickWin::~PickWin() {
  // Popup, all controls, ListBox, sprites, ButtonGroup and GraphicWin are
  // real subobjects. Native destructor instructions confirm reverse order.
  // Their exact existing owners provide each close/destroy stage.
}


// CRT startup/atexit establish a live
// object beginning at 0x822718. The candidate class footprint reaches 0x834D68,
// but that does not prove the original allocation extent; the caption buffer
// operand begins at 0x834D6C, with a four-byte gap. Keep only a start binding.
// RECOVERY_OBJECT: v1 0x00822718 ?PickWinGlobal@@3VPickWin@@A
PickWin PickWinGlobal;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00488780 ?list_was_dblclicked@@YAXH@Z 0x00488780-0x0048879D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/00488780.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
void __cdecl list_was_dblclicked(int faction_id) {
  if (faction_id >= 1 && faction_id <= 7) {
    PickWinGlobal.on_button_clicked(faction_id);
  }
}
