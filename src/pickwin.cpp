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
// ORIGINAL: 0x00488750 _$E21 0x00488750-0x00488766 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x68256c selects original initializer0x488750..0x488766, registering fini0x488770..0x48877a; complete independent decode establishes exclusive boundaries.", "Source-owned object?PickWinGlobal@@3VPickWin@@A identifies original storage0x822718. Startup arguments/calls: call 0x48ac10; push 0x488770. Cleanup receiver/calls: mov ecx, 0x822718; jmp 0x48a8e0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/pickwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?PickWinGlobal@@3VPickWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E16
// ORIGINAL: 0x00488770 _$E19 0x00488770-0x0048877A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x68256c selects original initializer0x488750..0x488766, registering fini0x488770..0x48877a; complete independent decode establishes exclusive boundaries.", "Source-owned object?PickWinGlobal@@3VPickWin@@A identifies original storage0x822718. Startup arguments/calls: call 0x48ac10; push 0x488770. Cleanup receiver/calls: mov ecx, 0x822718; jmp 0x48a8e0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/pickwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?PickWinGlobal@@3VPickWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E14
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h; on_button_clicked
// expands ~Dialogs (close@Dialogs, then the member dtors) in its closing teardown.
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
#include "interface_palette.h"
#include "popmenu.h"
#include "rectangle_operations.h"

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];
extern int TextColourTable[195];

// Original caption834D6C is passed synchronously to Win::init, which
// copies it. The pinned PE virtual zero-fill tail proves its initial NUL.
// This mutable one-byte minimum represents that observed initial state,
// NOT original capacity, allocation bounds or absence of indirect writers.
// Recover a larger extent before introducing any writer or longer content.
// RECOVERY_OBJECT: v1 0x00834D6C ?PickWinCaptionBuffer@@3PADA
char PickWinCaptionBuffer[1] = { 0 };

/*
// ORIGINAL: 0x0048AC10 ??0PickWin@@QAE@XZ 0x0048AC10-0x0048ADB6;0x00657E30-0x00657F65 BYTE_EXACT
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
// symbol    ?on_button_passover@PickWin@@UAEXHH@Z
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

// WORKAROUND(inline): init shifts the list RECT with `add reg,-0x12` on left
// and right only (0x00488DB6), the shape of an inlined offset helper whose
// vertical delta folded away.
static inline RECT *pickwin_offset_rect(RECT *rect, int horizontal, int vertical) {
  rect->left += horizontal;
  rect->right += horizontal;
  rect->top += vertical;
  rect->bottom += vertical;
  return rect;
}

// WORKAROUND(decl): the list reset at 0x00489DF4..0x00489E4E (and again at
// 0x0048A226) stores ListBox +0x2C/+0x14, the four Dialog fields, then Win
// iFlags_ = 0x40 last. Only ListBox::initialize_picker_state can reach the
// protected Win word, and it stores all three ListBox words together. This
// access shim reaches iFlags_ through ListBox's own vbtable; the original
// header probably had one inline covering the whole reset.
struct PickWinListAccess : ListBox {
  static void set_win_flags(PickWin *window, uint32_t flags) {
    static_cast<PickWinListAccess &>(window->listBox_).iFlags_ = flags;
  }
};

// WORKAROUND(inline): by-value helpers on the layout cursor (x, y, width,
// height at 0xA14..0xA20). A store through `this` makes VC6 reload the other
// members, and the image does not reload them here, so the values crossed
// the store as inline arguments:
// - 0x488A17..0x488A30 reads width, x and height, stores x + width + 1 and
//   only then reads y for the same rectangle: the advanced x is one argument
//   of set_rectangle_dimensions, produced by an inline that takes the width.
//   Written in place (`field_A14_ += field_A1C_ + 1` as the argument) or with
//   the +1 at the call site the image is not reproduced (162 and 129 rows
//   against 92).
// - 0x488E0E..0x488E26 reads both edges before either store: set_pos(x, y).
// Names and owner unknown; the other windows with fields at 0xA14 show no
// site of either shape (PrefWin uses those words as page state).
static inline int pickwin_next_x(PickWin *window, int width) {
  return window->field_A14_ += width + 1;
}
static inline void pickwin_set_pos(PickWin *window, int x, int y) {
  window->field_A14_ = x;
  window->field_A18_ = y;
}

// ORIGINAL: 0x004887A0 ?init@PickWin@@QAEXP6AXH@ZHH@Z 0x004887A0-0x0048965F;0x00657F70-0x00657F85
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004887a0.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// TRIED: nothing exotic here (no x87, no computed jumps) - transcribed mechanically from the Ghidra pseudocode (which is accurate for this straight-line function) using raw `self+offset` field access and real member calls resolved against pickwin.h field names where they matched. sim 0.65-0.71 across flag sets.
void PickWin::init(void (__cdecl *callback)(int), int mode, int faction) {
  Buffer source;
  int i;

  field_5DAC_ = static_cast<uint32_t>(faction);
  field_6238_ = callback;
  field_5DA0_ = static_cast<uint32_t>(mode);
  field_5DA4_ = 0;

  if (text_open(AlphaxFileID, "FACTIONS")) return;
  for (i = 0; i < 7; ++i) {
    text_get();
    strcpy(faction_files_[i], text_item());
  }
  if (text_open(AlphaxFileID, "NEWFACTIONS")) return;
  for (i = 7; i < 14; ++i) {
    text_get();
    strcpy(faction_files_[i], text_item());
  }

  field_5DB0_ = 0;
  if (text_open(AlphaxFileID, "CUSTOMFACTIONS")) return;
  text_get();
  while (*text_item() != '\0') {
    ++field_5DB0_;
    text_get();
  }
  if (text_open(AlphaxFileID, "CUSTOMFACTIONS")) return;
  field_5DB0_ = range(field_5DB0_, 0, 10);
  for (i = 0; i < field_5DB0_; ++i) {
    text_get();
    strcpy(faction_files_[14 + i], text_item());
  }
  for (i = 0; i < field_5DB0_ + 14; ++i) {
    if (text_open(faction_files_[i], faction_files_[i])) return;
    text_get();
    text_item();
    text_item();
    strncpy(faction_menu_items_[i], text_item(), 0x18);
    faction_menu_items_[i][0x17] = '\0';
  }

  source.load_pcx("pickfac.pcx", 0, 10, 0xEC);

  RECT area;
  // WORKAROUND(inline): the image keeps the cell height in a register from
  // the 0x5A store (0x4889A1) into the loop and from each row step
  // (0x488ABC) into the next row's first rectangle; `h` stands in for
  // whatever the original read it through. The second and third rectangles
  // go through pickwin_next_x (above).
  int h = 0x5A;
  field_A1C_ = 0x102;
  field_A20_ = h;
  field_A18_ = 1;
  for (i = 0; i < 3; ++i) {
    set_rectangle_dimensions(&area, field_A14_ = 1, field_A18_, field_A1C_, h);
    sprites1_[i].extract(&source, WinFillColour, &area, 0);
    set_rectangle_dimensions(&area, pickwin_next_x(this, field_A1C_), field_A18_, field_A1C_,
                             field_A20_);
    sprites2_[i].extract(&source, WinFillColour, &area, 0);
    set_rectangle_dimensions(&area, pickwin_next_x(this, field_A1C_), field_A18_, field_A1C_,
                             field_A20_);
    sprites3_[i].extract(&source, WinFillColour, &area, 0);
    h = field_A20_;
    field_A18_ += h + 1;
  }

  field_A14_ = 1;
  field_A1C_ = 0x10;
  field_A20_ = h = 0x10;
  field_A18_ = 0x139;
  for (i = 0; i < 3; ++i) {
    set_rectangle_dimensions(&area, field_A14_, field_A18_, field_A1C_, h);
    sprites4_[i].extract(&source, WinFillColour, &area, 0);
    h = field_A20_;
    field_A18_ += h + 1;
  }
  sprite5_.extract(&source, WinFillColour, field_A14_, field_A18_, 7, 0xD, 0);

  field_A1C_ = 0x316;
  field_A20_ = 0x12C;
  field_A14_ = 5;
  field_A18_ = 5;
  GraphicWin::init(5, 5, 0x316, 0x12C, PickWinCaptionBuffer,
                   0x01000000, 0, 0, 0);
  Buffer::set_font(MainInterfaceGlobal.primary_font(), 0, 0, 0);
  Buffer::set_text_color(TextColourTable[68], TextColourTable[69], 1, 1);

  set_rectangle_dimensions(&frame_rect_, 0, 0, field_A1C_, field_A20_);
  header_panel_rect_ = frame_rect_;
  expand_rect(&header_panel_rect_, -4, -4);
  field_A14_ = header_panel_rect_.left + 1;
  header_panel_rect_.bottom = header_panel_rect_.top + 0x1E;
  field_A18_ = header_panel_rect_.bottom + 4;
  field_A1C_ = (header_panel_rect_.right - header_panel_rect_.left - 6) / 3;
  field_A20_ = 0x16;
  set_rectangle_dimensions(&field_625C_rect_, field_A14_, field_A18_, 0x102, 0x5A);
  set_rectangle_dimensions(&field_626C_rect_, field_625C_rect_.right + 2, field_A18_, 0x102, 0x5A);
  set_rectangle_dimensions(&field_627C_rect_, field_626C_rect_.right + 2, field_A18_, 0x102, 0x5A);

  field_A14_ = header_panel_rect_.left + 0x159;
  set_rectangle_dimensions(&list_area_rect_, header_panel_rect_.left + 0x15B, field_625C_rect_.bottom + 3,
                           header_panel_rect_.right - header_panel_rect_.left - 0x15E, 0x88);
  expand_rect(&list_area_rect_, -3, -3);
  pickwin_offset_rect(&list_area_rect_, -0x12, 0);

  int row = 0;
  for (i = 0; i < 0x85; i += 0x13, ++row) {
    set_rectangle_dimensions(&picker_button_rects_[row], list_area_rect_.right + 2,
                             i + list_area_rect_.top, 0x10, 0x10);
  }

  pickwin_set_pos(this, header_panel_rect_.left + 1, list_area_rect_.bottom + 7);
  field_A1C_ = static_cast<int>(field_A1C_) * 3 / 4 - 1;
  set_rectangle_dimensions(&field_9220_rect_, field_A14_, field_A18_, field_A1C_, field_A20_);
  set_rectangle_dimensions(&field_9210_rect_, field_9220_rect_.right + 2, field_A18_, field_A1C_, field_A20_);
  set_rectangle_dimensions(&field_9260_rect_, field_9210_rect_.right + 2, field_A18_, field_A1C_, field_A20_);
  set_rectangle_dimensions(&field_9230_rect_, field_9260_rect_.right + 2, field_A18_, field_A1C_, field_A20_);

  field_A1C_ = 0xD8;
  field_A18_ = list_area_rect_.top - 2;
  field_A14_ = header_panel_rect_.left + 2;
  set_rectangle_dimensions(&field_9240_rect_, field_A14_, field_A18_, field_A1C_, field_A20_);
  field_A18_ += 0x19;
  set_rectangle_dimensions(&field_9250_rect_, field_A14_, field_A18_, field_A1C_, field_A20_);

  for (i = 0; i < 7; ++i) {
    RECT &button = picker_button_rects_[i];
    flatButtons10_[i].init(0, i + 0x3039, &button,
                           this, 0);
    flatButtons10_[i].set_report_icons(&sprites4_[0], &sprites4_[1], &sprites4_[2]);
    flatButtons10_[i].hide();
  }

  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x45B));
  flatButton1_.init(0, 0x100, &field_625C_rect_, this, 0);
  flatButton1_.set_report_icons(&sprites1_[0], &sprites1_[1], &sprites1_[2]);
  flatButton1_.set_bubble_text(StringTemp);
  flatButton1_.hide();
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x45C));
  flatButton2_.init(0, 0x101, &field_626C_rect_, this, 0);
  flatButton2_.set_report_icons(&sprites2_[0], &sprites2_[1], &sprites2_[2]);
  flatButton2_.set_bubble_text(StringTemp);
  flatButton2_.hide();
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x45D));
  flatButton3_.init(0, 0x102, &field_627C_rect_, this, 0);
  flatButton3_.set_report_icons(&sprites3_[0], &sprites3_[1], &sprites3_[2]);
  flatButton3_.set_bubble_text(StringTemp);
  flatButton3_.hide();

  listBox_.init(&list_area_rect_, 0);
  listBox_.field_2C_ = 0;
  listBox_.clear_selection_index();
  listBox_.Dialog::field_B4_ = 0;
  listBox_.Dialog::field_40_ = 0;
  listBox_.Dialog::field_48_ = 0x13;
  listBox_.Dialog::field_44_ = 3;
  PickWinListAccess::set_win_flags(this, 0x40);
  listBox_.clear_style_bits(4);
  listBox_.double_click_callback_ = list_was_dblclicked;

  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x45E));
  flatButton4_.init(StringTemp, 0x106, &field_9210_rect_, this, 0);
  flatButton4_.set_font_redraw(MainInterfaceGlobal.primary_font());
  flatButton4_.hide();
  StringTemp[0] = '\0';
  strcat(StringTemp, StringTable.get(field_5DAC_ ? Labels.strings_ptr[0x45F]
                                             : Labels.strings_ptr[0x2AF]));
  flatButton5_.init(StringTemp, 0x107, &field_9220_rect_, this, 0);
  flatButton5_.set_font_redraw(MainInterfaceGlobal.primary_font());
  flatButton5_.hide();
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x66));
  CharUpperA(StringTemp);
  flatButton6_.init(StringTemp, 0x103, &field_9230_rect_, this, 0);
  flatButton6_.set_font_redraw(MainInterfaceGlobal.primary_font());
  flatButton6_.hide();
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x65));
  CharUpperA(StringTemp);
  flatButton9_.init(StringTemp, 0x108, &field_9260_rect_, this, 0);
  flatButton9_.set_font_redraw(MainInterfaceGlobal.primary_font());
  flatButton9_.hide();
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x460));
  flatButton8_.init(StringTemp, 0x105, &field_9250_rect_, this, 0);
  flatButton8_.set_font_redraw(MainInterfaceGlobal.primary_font());
  flatButton8_.hide();
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x461));
  flatButton7_.init(StringTemp, 0x104, &field_9240_rect_, this, 0);
  flatButton7_.set_font_redraw(MainInterfaceGlobal.primary_font());
  flatButton7_.hide();

  buttonGroup_.init(0, 0);
  buttonGroup_.add(&flatButton7_);
  buttonGroup_.add(&flatButton8_);
  buttonGroup_.set(0x104, 1);
}


// PickWin::on_button_clicked's string operands, each read from the image's
// operand at the relocation site and checked against the literal text in
// the body; the repeated "JENN282" and "|" are separate unpooled copies at
// distinct addresses, in source order.
// RECOVERY_LITERAL: v1 0x00686AD0 0x00686AD9 $SG51161 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686ADC 0x00686AE8 $SG51165 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686AE8 0x00686AF0 $SG51175 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686AF0 0x00686AF8 $SG51176 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686AF8 0x00686B00 $SG51184 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B00 0x00686B02 $SG51185 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B04 0x00686B0C $SG51216 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B0C 0x00686B14 $SG51217 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B14 0x00686B1C $SG51226 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B1C 0x00686B1E $SG51227 0x00489BA0
// ORIGINAL: 0x00489BA0 ?on_button_clicked@PickWin@@QAEXH@Z 0x00489BA0-0x0048A798;0x00657F85-0x006581C9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00489ba0.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// TRIED: automatic C++ RAII destruction for Popup/PullDown's ~20 embedded subobjects (Dialogs, EditGroup, SpriteBox, CheckBox, ListBox, Dialog, Sprite, StringStruct x2, FlatButton x2, Heap, nested GraphicWin) - the scaffold's classes represent several of these as raw byte/uint32 padding, so auto-generated dtors would skip real cleanup the original calls explicitly. Used mechanical EBP(x)-buffer-offset transcription (verified against pickwin.h's `Popup popup_` size comment == sizeof(BasePop)+sizeof(Scroll), and cross-checked field offsets against pickwin.h directly) instead, matching each explicit close()/dtor call site.
// on_button_clicked's string operands, each read from the image's operand at
// the relocation offset of our body and checked against the literal text; the
// repeated "JENN282" and "|" are separate unpooled copies at distinct addresses.
// RECOVERY_LITERAL: v1 0x00686AD0 0x00686AD9 $SG51233 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686ADC 0x00686AE8 $SG51237 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686AE8 0x00686AF0 $SG51247 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686AF0 0x00686AF8 $SG51248 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686AF8 0x00686B00 $SG51256 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B00 0x00686B02 $SG51257 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B04 0x00686B0C $SG51288 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B0C 0x00686B14 $SG51289 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B14 0x00686B1C $SG51298 0x00489BA0
// RECOVERY_LITERAL: v1 0x00686B1C 0x00686B1E $SG51299 0x00489BA0
// symbol    ?on_button_clicked@PickWin@@UAEXH@Z
void PickWin::on_button_clicked(int button) {
  PopMenu menu;

  switch (button) {
  case 0x100:
  case 0x101:
  case 0x102: {
    if (button == 0x100) {
      if (text_open(AlphaxFileID, "FACTIONS")) return;
    } else if (button == 0x101) {
      if (text_open(AlphaxFileID, "NEWFACTIONS")) return;
    }
    int faction;
    if (button != 0x102) {
      for (faction = 1; faction < 8; ++faction) {
        text_get();
        strcpy(Players[faction].filename, text_item());
        strcpy(Players[faction].search_key, text_item());
      }
    }
    for (faction = 1; faction < 8; ++faction) {
      if (button == 0x102) {
        strcpy(Players[faction].filename, "JENN282");
        strcpy(Players[faction].search_key, "JENN282");
      } else {
        read_faction(faction);
      }
    }
    listBox_.init(&list_area_rect_, 0);
    listBox_.field_2C_ = 0;
    listBox_.clear_selection_index();
    listBox_.Dialog::field_B4_ = 0;
    listBox_.Dialog::field_40_ = 0;
    listBox_.Dialog::field_48_ = 0x13;
    listBox_.Dialog::field_44_ = 3;
    PickWinListAccess::set_win_flags(this, 0x40);
    for (faction = 1; faction < 8; ++faction) {
      if (field_5DA0_ != 2 ||
          is_alive(faction)) {
        StringTemp[0] = '\0';
        if (strcmp(Players[faction].filename, "JENN282") != 0) {
          says(Players[faction].formal_name_faction);
          says("|");
          says(SocialWinOpenParenText);
          says(Players[faction].desc_name_faction);
          says(SocialWinCloseParenText);
        } else {
          say_label(0x462);
        }
        listBox_.item(StringTemp, faction);
      }
    }
    {
      // Through a pointer: the image forms ListBox's this and both position
      // reads off the list box's own address in edi.
      ListBox *box = &listBox_;
      box->ListBox::attach(this, box->Dialog::window_x_, box->Dialog::window_y_, 0x80020);
    }
    listBox_.set_selected_id((field_5DA0_ & 2) ? ConsoleGlobal.faction_id_
                                               : DefaultPrefs.faction_id);
    listBox_.set_selection_callback(field_6238_);
    field_6238_((field_5DA0_ & 2) ? ConsoleGlobal.faction_id_
                                  : DefaultPrefs.faction_id);
    redraw();
    break;
  }

  case 0x108:
    field_5DA8_ = static_cast<uint32_t>(buttonGroup_.field_84_ == 0x105
        ? static_cast<int>(field_5DB4_)
        : listBox_.Dialog::get_selected_id());
    field_5DA4_ = buttonGroup_.field_84_ == 0x105 ? 0x105 : 0;
    release_modal();
    break;

  case 0x103:
    field_5DA8_ = static_cast<uint32_t>(-1);
    field_5DA4_ = 0;
    release_modal();
    break;

  default:
    if (button <= 0) break;
    if (button < 8) {
      field_5DA8_ = static_cast<uint32_t>(button);
      field_5DA4_ = 0;
      release_modal();
      break;
    }
    if (button == 0x104 || button == 0x105) break;
    if (button < 0x3039) {
      field_5DA8_ = static_cast<uint32_t>(listBox_.Dialog::get_selected_id());
      field_5DA4_ = static_cast<uint32_t>(button);
      release_modal();
      break;
    }
    {
      int x, y;
      get_mouse_pos(&x, &y);
      client_to_screen(&x, &y);
      menu.init();
      int index;
      for (index = 0; index < static_cast<int>(field_5DB0_) + 14; ++index) {
        menu.dialogs().item(faction_menu_items_[index], index);
      }
      StringTemp[0] = '\0';
      say_label(0x462);
      // WORKAROUND(inline): the image reads the row count as a named value
      // (eax, outside the round-robin cursor), as through an inline accessor.
      const int rows = static_cast<int>(field_5DB0_);
      menu.dialogs().item(StringTemp, rows + 14);
      const int chosen = menu.exec(x, y, 0);
      flatButtons10_[button - 0x3039].field_A74_ = 0;
      flatButtons10_[button - 0x3039].redraw();
      if (chosen == -1) break;

      const int faction = button - 0x3038;
      if (chosen == static_cast<int>(field_5DB0_) + 14) {
        strcpy(Players[faction].filename, "JENN282");
        strcpy(Players[faction].search_key, "JENN282");
      } else {
        strcpy(Players[faction].filename, faction_files_[chosen]);
        strcpy(Players[faction].search_key, faction_files_[chosen]);
        read_faction(faction);
      }
      listBox_.init(&list_area_rect_, 0);
      listBox_.field_2C_ = 0;
    listBox_.clear_selection_index();
      listBox_.Dialog::field_B4_ = 0;
      listBox_.Dialog::field_40_ = 0;
      listBox_.Dialog::field_48_ = 0x13;
      listBox_.Dialog::field_44_ = 3;
      PickWinListAccess::set_win_flags(this, 0x40);
      for (int row = 1; row < 8; ++row) {
        if (field_5DA0_ != 2 ||
            is_alive(row)) {
          StringTemp[0] = '\0';
          if (strcmp(Players[row].filename, "JENN282") != 0) {
            says(Players[row].formal_name_faction);
            says("|");
            says(SocialWinOpenParenText);
            says(Players[row].desc_name_faction);
            says(SocialWinCloseParenText);
          } else {
            say_label(0x462);
          }
          listBox_.item(StringTemp, row);
        }
      }
      {
        // Through a pointer: the image forms ListBox's this and both position
        // reads off the list box's own address in edi.
        ListBox *box = &listBox_;
        box->ListBox::attach(this, box->Dialog::window_x_, box->Dialog::window_y_, 0x80020);
      }
      listBox_.set_selected_id(faction);
      listBox_.set_selection_callback(field_6238_);
      field_6238_(faction);
      redraw();
    }
    break;
  }
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00489B20 ?on_group_clicked@PickWin@@QAEXHHH@Z 0x00489B20-0x00489B94 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00489b20.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// symbol    ?on_group_clicked@PickWin@@UAEXHHH@Z
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
// symbol    ?on_dialog_item_back_draw@PickWin@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
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
// ORIGINAL: 0x00489660 ?on_redraw@PickWin@@QAEXXZ 0x00489660-0x00489AF7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00489660.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// symbol    ?on_redraw@PickWin@@UAEXXZ
void PickWin::on_redraw() {
  GraphicWin::fill(9);
  Buffer::box_sprite(&frame_rect_, &InterfaceBoxStyles[9]);
  Buffer::box_sprite(&header_panel_rect_, &InterfaceBoxStyles[3]);
  expand_rect(&header_panel_rect_, -3, -3);
  Buffer::box_sprite(&header_panel_rect_, &InterfaceBoxStyles[6]);
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0x45A));
  Buffer::write_cent_l(StringTemp, &header_panel_rect_);
  expand_rect(&header_panel_rect_, 3, 3);

  RECT group;
  group.left = list_area_rect_.left;
  group.top = list_area_rect_.top;
  group.right = list_area_rect_.right + 0x12;
  group.bottom = list_area_rect_.bottom;
  expand_rect(&group, 3, 3);
  Buffer::box_sprite(&group, &InterfaceBoxStyles[3]);

  int selected = listBox_.Dialog::get_selected_id();
  int row_y = field_9240_rect_.top + 10;
  int line_y = row_y + 0x39;
  int center_x = (field_9240_rect_.left + field_9240_rect_.right) / 2;
  int list_x = list_area_rect_.left - 10;
  int join_x = (list_x + field_9240_rect_.right) / 2;

  if (buttonGroup_.field_84_ == 0x105) {
    Buffer::vline(center_x, field_9250_rect_.bottom - 2, line_y - 2, 0xE0);
    Buffer::put_pixel(center_x + 1, line_y - 1, 0xE0);
    Buffer::hline(center_x + 2, join_x, line_y, 0xE0);
    Buffer::vline(join_x, row_y + 2, row_y + 0x70, 0xE0);
    for (int row = 0; row < 7; ++row) {
      int y = row_y + row * 0x13;
      Buffer::put_pixel(join_x + 1, y + (row != 0 ? -(row == 6) : 1), 0xE0);
      Buffer::hline(join_x + 2, list_x, y, 0xE0);
      sprite5_.draw(static_cast<Buffer *>(this), list_x, y - 6);
    }
  } else if (selected > 0 && selected < 8) {
    --selected;
    int y = row_y + selected * 0x13;
    Buffer::hline(field_9240_rect_.right, join_x - 2, row_y, 0xE0);
    if (selected != 0) {
      Buffer::put_pixel(join_x - 1, row_y + 1, 0xE0);
      Buffer::put_pixel(join_x + 1, y - 1, 0xE0);
      Buffer::vline(join_x, row_y + 2, y - 2, 0xE0);
    } else {
      Buffer::hline(join_x - 2, join_x + 2, row_y, 0xE0);
    }
    Buffer::hline(join_x + 2, list_x, y, 0xE0);
    sprite5_.draw(static_cast<Buffer *>(this), list_x, y - 6);
  }

  flatButton1_.show(0);
  flatButton2_.show(0);
  flatButton3_.show(0);
  flatButtons10_[0].show(0);
  flatButtons10_[1].show(0);
  flatButtons10_[2].show(0);
  flatButtons10_[3].show(0);
  flatButtons10_[4].show(0);
  flatButtons10_[5].show(0);
  flatButtons10_[6].show(0);
  listBox_.show(0);
  flatButton4_.show(0);
  flatButton5_.show(0);
  flatButton6_.show(0);
  flatButton9_.show(0);
  flatButton8_.show(0);
  flatButton7_.show(0);
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048A8E0 ??1PickWin@@QAE@XZ 0x0048A8E0-0x0048AC09;0x00657C70-0x00657E2F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0048a8e0.cpp"], "reason": "Recover genuine typed PickWin owner behavior and automatic subobject lifetime from pinned native flow and preserved seed; historical identity and restoration obligations retained, no exactness claim."}
// symbol    ??1PickWin@@UAE@XZ
// The implicit destructor owns reverse member/base teardown, including array
// cleanup and exception unwinding. The real PickWinGlobal lifetime and virtual
// deleting destructor emit this body; an explicit empty destructor additionally
// rewrites derived vptrs before member teardown, unlike the original.


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
// RECOVERY_GENERATED: v1 vtable 0x0066D13C ? ??_7PickWin@@6BWin@@@ 0x0048AC10
// RECOVERY_GENERATED: v1 vtable 0x0066D134 ? ??_7PickWin@@6BBuffer@@@ 0x0048AC10
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00489BA0 0x0048A798 0x0048A7A8 0x00489C2B
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00489BA0 0x0048A7A8 0x0048A7B1 0x00489C25
