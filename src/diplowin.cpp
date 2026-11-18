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
// ORIGINAL: 0x00445080 ??_GDiploWin@@UAEPAXI@Z 0x00445080-0x004450A1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00445080-0x004450A1", "Pinned class constructor ??0DiploWin@@QAE@XZ at 0x00444FC0 stores original class/view table VA 0x66b634 at 0x00445053 (mov dword ptr [esi], 0x66b634); the current genuine owner vftable ??_7DiploWin@@6BWin@@@ selects generated ??_GDiploWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EDiploWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GDiploWin@@UAEPAXI@Z at 0x00445080 calls the source-mapped ??1DiploWin@@QAE@XZ at 0x00444DB0, then tests byte ptr [ebp + 8], 1; je 0x44509a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/diplowin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original DiploWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GDiploWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x00445120 ??_EDiploWin@@WEEE@AEPAXI@Z 0x00445120-0x0044512B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor at 0x00444FC0 installs Buffer table 0x0066B62C at this+0x444; its first slot points to 0x00445120.", "Pinned entry 0x00445120 has exactly eleven bytes: SUB ECX,0x444 then JMP to already mapped primary deleting destructor 0x00445080, followed by INT3 padding.", "Owner diplowin.cpp physical COFF emits eleven-byte COMDAT ??_EDiploWin@@WEEE@AEPAXI@Z; Buffer table ??_7DiploWin@@6BBuffer@@@ selects it, and its REL32 targets weak ??_EDiploWin@@UAEPAXI@Z with scalar ??_GDiploWin@@UAEPAXI@Z fallback.", "The genuine GraphicWin:Win,Buffer hierarchy provides the receiver adjustment. Independent original constructor/table/body and physical COFF review supports this binding; local equality does not prove full linked table selection or runtime acceptance."], "reason": "Bind the original Buffer-view deleting receiver adjustment to the genuine DiploWin compiler-emitted operation. The symbol names this tree emits, not an original debug name; no adapter body is authored."}
// ORIGINAL: 0x00440F00 _$E21 0x00440F00-0x00440F16 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682084 selects initializer0x440f00..0x440f16, registering exit0x440f20..0x440f2a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x73acd8 bound to?DiploWindow@@3VDiploWin@@A.", "Original startup instruction evidence: [[\"0x440f00\", \"mov\", \"ecx, 0x73acd8\"], [\"0x440f05\", \"call\", \"0x444fc0\"], [\"0x440f0a\", \"push\", \"0x440f20\"], [\"0x440f0f\", \"call\", \"0x645398\"], [\"0x440f14\", \"pop\", \"ecx\"], [\"0x440f15\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x440f20\", \"mov\", \"ecx, 0x73acd8\"], [\"0x440f25\", \"jmp\", \"0x444db0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/diplowin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?DiploWindow@@3VDiploWin@@A\", \"??0DiploWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?DiploWindow@@3VDiploWin@@A\", \"??1DiploWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2ec; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?DiploWindow@@3VDiploWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E16
// ORIGINAL: 0x00440F20 _$E19 0x00440F20-0x00440F2A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682084 selects initializer0x440f00..0x440f16, registering exit0x440f20..0x440f2a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x73acd8 bound to?DiploWindow@@3VDiploWin@@A.", "Original startup instruction evidence: [[\"0x440f00\", \"mov\", \"ecx, 0x73acd8\"], [\"0x440f05\", \"call\", \"0x444fc0\"], [\"0x440f0a\", \"push\", \"0x440f20\"], [\"0x440f0f\", \"call\", \"0x645398\"], [\"0x440f14\", \"pop\", \"ecx\"], [\"0x440f15\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x440f20\", \"mov\", \"ecx, 0x73acd8\"], [\"0x440f25\", \"jmp\", \"0x444db0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/diplowin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?DiploWindow@@3VDiploWin@@A\", \"??0DiploWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?DiploWindow@@3VDiploWin@@A\", \"??1DiploWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f2ec; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?DiploWindow@@3VDiploWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E14
// WORKAROUND(decl): opt in to the header's inline StringStruct::current_entry
// (sub_441040 expands it at 0x004410AF; see stringstruct.h).
#define STRINGSTRUCT_CURRENT_ENTRY_INLINE
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h; on_button_clicked
#define STRINGSTRUCT_CURRENT_ID_INLINE
#include "stdafx.h"
#include "interface_fonts.h"
#include "diplowin.h"
#include "netdaemon.h"
#include "basepop.h"
#include "popup.h"
#include "xpops.h"
#include "netmsg.h"

#include "console.h"
#include "faction.h"
#include "faction_inline.h"
#include "game.h"
#include "interface_palette.h"
#include "technology.h"
#include "alpha.h"
#include "maininterface.h"
#include "rectangle_operations.h"
#include "jackal/font_metrics.h"
#include "general.h"  // swap for the hot-seat pair callbacks

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];

// Shared original chat separators, also used by the general string formatter.
extern char SocialWinColonSpaceText[];

void __cdecl slide_show(GraphicWin *window, int mode);
void __cdecl slide_hide(GraphicWin *window, int mode);
extern int DiploOffersDirty;

// Native redraw passes these distinct addresses as C strings. Both initially
// contain NUL. Only their first byte is established; producer and full string
// extents remain unresolved, so these are provisional minimum storage owners.
// RECOVERY_OBJECT: v2 0x007492D4 0x007492D5 ?DiploLocalizedTradeTitle@@3DA 0x004436C0
char DiploLocalizedTradeTitle;
// RECOVERY_OBJECT: v2 0x007492D8 0x007492D9 ?DiploLocalizedDiplomacyTitle@@3DA 0x004436C0
char DiploLocalizedDiplomacyTitle;

// DiploWin owns cleanup through its real typed subobjects. Its destructor
// body has no separate unregister or global-state operation; C++ performs
// reverse-order member/base destruction after this body.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00444DB0 ??1DiploWin@@QAE@XZ 0x00444DB0-0x00444FB2;0x00654920-0x006549F0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00444db0.cpp", "Pinned444DB0 cleanup sequence and654920 unwind funclets; existing typed members match their native receiver offsets and thirteen-element FlatButton array."], "reason": "Recover complete DiploWin destruction through genuine automatic reverse-order member/base lifetimes; pinned teardown contains no additional DiploWin unregister. Replace only executable-address forwarder and preserve seed history, no exactness claim."}
// symbol ??1DiploWin@@UAE@XZ
// The destructor is compiler-generated: no declaration, no body.

// Native 0x444FC0 constructs both bases, Spot, thirteen FlatButtons, StringBox,
// ListBox and EditBox in declaration order. No scalar initialization follows.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00444FC0 ??0DiploWin@@QAE@XZ 0x00444FC0-0x0044507B;0x006549F0-0x00654A47 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original constructor444FC0 establishes GraphicWin+0, SubInterface+A14 and member construction order; secondary table66B5E0 gives nineteen callback slots and adjusted receivers.", "Redraw4436C0, status443C70 and painter443E50 inspected over declared original spans; source rectangle/control owners preserve native offsets. Palette colors reuse TextColourTable[80-2*item]/[81-2*item]. Localized title producer and full extents remain unknown.", "recovery/tasks/interface-contract-report-closure.json; root coordinator owns import and independent compilation; no current byte, structural or runtime acceptance is implied."], "reason": "Recover the genuine DiploWin GraphicWin/SubInterface owner with typed callbacks and automatic base/member lifetimes; retain independent seed history and restoration obligations."}
DiploWin::DiploWin() {}

// The executable constructs the singleton and all 64 transcript lists with
// static lifetime; their typed constructors/destructors own their storage.
// Extent: compiled sizeof 0xe5f0; 0x7492C8 unreferenced; CommlinkPending is 0x7492CC.
// RECOVERY_OBJECT: v2 0x0073ACD8 0x007492C8 ?DiploWindow@@3VDiploWin@@A 0x004253F3
// Original static initializer440F00 and teardown440F20 establish lifetime;
// StringBox745060 and StringList747BD0 are embedded members, not extra globals.
DiploWin DiploWindow;
// RECOVERY_OBJECT: v1 0x00737CD8 ?DiplomacyTranscriptLists@@3PAY07VStringList@@A
// Initializer440F30 constructs 64 StringLists, stride0x30;440F60 destroys them.
StringList DiplomacyTranscriptLists[8][8];

// 0x00443F10 latches this scalar from zero to one; 0x005290C4 clears it.
// Its semantic name and broader containing storage remain unresolved.
// RECOVERY_OBJECT: v2 0x0093A968 0x0093A96C ?DiploDefaultFlag93A968@@3HA 0x004449AF
int DiploDefaultFlag93A968;

// 0x004412A0 reads its first stack argument as the faction palette index and
// writes to the shared DiploWindow StringBox at 0x745060 (DiploWindow+0xA388).
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004412A0 ?string_callback@DiploWin@@QAAXHH@Z 0x004412A0-0x004412D0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Preserve the historical callback identity and claim while expressing its actual static cdecl receiver-free ABI.","evidence":["source:src/recovered/004412a0.cpp; historical obligation retained unchanged","Native4412A0 reads faction palette index from stack and writes the singleton embedded StringBox745060; candidate COFF emits ?string_callback@DiploWin@@SAXHH@Z."]}
// symbol    ?string_callback@DiploWin@@SAXHH@Z
void __cdecl DiploWin::string_callback(int faction_id, int unused) {
  (void)unused;
  Buffer &canvas = static_cast<Buffer &>(DiploWindow.stringBox_);
  canvas.set_text_color(ChatItemIdTable[faction_id],
                        ChatShadowColors[faction_id], 1, 1);
}

// ---------------------------------------------------------------------------
// File-local helpers (place above DiploWin::exec; they replace the old
// by-value `inset_rect` at the top of diplowin.cpp, which exec no longer uses).
//
// 0x444140..44415E / 0x4444F9..444515 load all four
// source edges through a copy of the source pointer, then +n/-n (dead
// top/bottom copies included when the vertical amount is zero).
// WORKAROUND(decl): file-local RECT helper stand-in (original name unknown).
static inline void diplo_inset_rect(RECT *dst, const RECT *src, int horizontal,
                                    int vertical) {
  dst->left = src->left + horizontal;
  dst->top = src->top + vertical;
  dst->right = src->right - horizontal;
  dst->bottom = src->bottom - vertical;
}

// ---------------------------------------------------------------------------
// ORIGINAL: 0x00443F10 ?exec@DiploWin@@QAEXHH@Z 0x00443F10-0x00444D8B
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00443f10.cpp", "image:terranx_original.exe:443F10-444D8B; callback4412A0 and transcript lifecycle440F30/440F60"], "reason": "Recover the full diplomacy window lifecycle using real base/member controls, singleton transcripts and bilateral offer storage; original catalogue extent retained."}
// TRIED: the ~500-instruction layout-RECT tree (this+0xedc onward, built by repeatedly splitting/insetting a screen rect) is approximated at the right field offsets with plausible rather than register-derived values; likewise the AI offer-table indexing (0x7388d8/0x7388e0/0x7388e8/0x738928, 9-wide rows keyed by ab4/ab8) is a simplified approximation, not the exact addressing. The widget init/call sequence (Spot::init, GraphicWin::init/fill, box_sprite, StringBox::init, EditBox::init, the 13-button BaseButton loop, add_offer/rand dispatch, message_data/iface teardown) is transcribed in order with matching argument counts. MISMATCH #20 (layout block, lea vs sub).
void DiploWin::exec(int faction_id, int faction_id_with) {
    int side;
  // Frame: three ints (x/y/height) and two RECT locals (rc at ebp-0x20,
  // row at ebp-0x30) = 0x2C, plus one compiler temp = the image's 0x30.
  RECT rc;
  RECT row;

  spot_.init(100);

  register_iface(0x0B);

  faction_id_ = faction_id;
  faction_id_with_ = faction_id_with;

  int window_height = 0x254 - MainInterfaceGlobal.panel_height();
  int window_x = (WinScreenWidth - 0x320) / 2;
  int window_y = WinScreenHeight - MainInterfaceGlobal.panel_height() - window_height;

  // 0x443F82: the outer window, shrunk 0x17 each side, then 0x20 more on the right.
  set_rectangle_dimensions(&rect_EDC_, 0, 0, 0x340, window_height);
  expand_rect(&rect_EDC_, -0x17, 0);
  rect_EDC_.right -= 0x20;

  // 0x443FB2: title strip = outer inset 4, then 3, then 0x14 tall. The
  // copy-then-expand_rect spelling gives the image's load order and the
  // rc.right/bottom spills; the inset helper does not.
  rc = rect_EDC_;
  expand_rect(&rc, -4, -4);
  rect_F4C_ = rc;
  expand_rect(&rect_F4C_, -3, -3);
  rect_F4C_.bottom = rect_F4C_.top + 0x14;

  // 0x444024: two leader panes under the title strip, gap 2.
  rc = rect_F4C_;
  offset_rect(&rc, 0, 0x17);
  rc.right = rc.left + (rc.right - rc.left + 2) / 2 - 2;
  leader_rect_[0] = rc;
  offset_rect(&rc, rc.right - rc.left + 2, 0);
  leader_rect_[1] = rc;

  // 0x444087: bottom row of three buttons (outer width shrunk by 1/7 a side).
  rc = rect_EDC_;
  rc.bottom -= 0x1A;
  rc.top = rc.bottom - 0x14;
  expand_rect(&rc, (rc.right - rc.left) / -7, 0);
  rc.right = rc.left + (rc.right - rc.left + 2) / 3 - 2;
  button_rects_[2] = rc;
  offset_rect(&rc, rc.right - rc.left + 2, 0);
  button_rects_[3] = rc;
  offset_rect(&rc, rc.right - rc.left + 2, 0);
  button_rects_[4] = rc;

  // 0x444140: four offer columns (gaps 2, 8, 2) ending 10 above that row.
  diplo_inset_rect(&row, &rect_EDC_, 7, 0);
  row.bottom = rc.top - 10;
  row.right = row.left + (row.right - row.left - 4) / 4 - 2;

  rc = row;
  rc.top = rc.bottom - 0x14;
  button_rects_[7] = rc;
  offset_rect(&rc, rc.right - rc.left + 2, 0);
  button_rects_[8] = rc;
  offset_rect(&rc, rc.right - rc.left + 8, 0);
  button_rects_[11] = rc;
  offset_rect(&rc, rc.right - rc.left + 2, 0);
  button_rects_[12] = rc;

  // 0x44421F: the row above (0x16 higher).
  rc = row;
  rc.top = rc.bottom - 0x14;
  offset_rect(&rc, 0, -0x16);
  button_rects_[5] = rc;
  offset_rect(&rc, rc.right - rc.left + 2, 0);
  button_rects_[6] = rc;
  offset_rect(&rc, rc.right - rc.left + 8, 0);
  button_rects_[9] = rc;
  offset_rect(&rc, rc.right - rc.left + 2, 0);
  button_rects_[10] = rc;

  // 0x4442AD: two-column-wide title cells another 0x16 higher.
  rc = row;
  rc.top = rc.bottom - 0x14;
  offset_rect(&rc, 0, -0x2C);
  rc.right += rc.right - rc.left + 2;
  rect_F9C_ = rc;
  offset_rect(&rc, rc.right - rc.left + 8, 0);
  rect_FBC_ = rc;

  // 0x444327: union each 2x2 group with its title cell, then grow by 3.
  rc.left = 0;
  rc.right = 0;
  rc.top = 0;
  rc.bottom = 0;
  int index;
  for (index = 5; index < 9; ++index) {
    UnionRect(&rc, &rc, &button_rects_[index]);
  }
  UnionRect(&rect_FAC_, &rc, &rect_F9C_);
  expand_rect(&rect_FAC_, 3, 3);

  rc.left = 0;
  rc.right = 0;
  rc.top = 0;
  rc.bottom = 0;
  for (index = 9; index < 13; ++index) {
    UnionRect(&rc, &rc, &button_rects_[index]);
  }
  UnionRect(&rect_FCC_, &rc, &rect_FBC_);
  expand_rect(&rect_FCC_, 3, 3);

  // 0x444421: MainInterface frames inset 7, each split into a 0x14 header
  // and the body 3 below it.
  diplo_inset_rect(&rc, &MainInterfaceGlobal.main_frame_rect(), 7, 7);
  rect_EEC_ = rc;
  rect_EEC_.bottom = rect_EEC_.top + 0x14;
  rect_EFC_ = rc;
  rect_EFC_.top = rect_EEC_.bottom + 3;

  diplo_inset_rect(&rc, &MainInterfaceGlobal.right_frame_rect(), 7, 7);
  rect_F0C_ = rc;
  rect_F0C_.bottom = rect_F0C_.top + 0x14;
  rect_F1C_ = rc;
  rect_F1C_.top = rect_F0C_.bottom + 3;

  // 0x4444F9: the two portrait panes between the leader panes and the
  // left offer group's (already grown) top.
  diplo_inset_rect(&rc, &rect_EDC_, 7, 0);
  rc.top = leader_rect_[0].bottom + 3;
  rc.bottom = rect_FAC_.top - 7;
  rc.right = rc.left + (rc.right - rc.left + 2) / 2 - 2;
  portrait_rect_[0] = rc;
  offset_rect(&rc, rc.right - rc.left + 2, 0);
  portrait_rect_[1] = rc;

  // 0x444572: chat transcript and the 0x14-tall edit line 3 below it.
  diplo_inset_rect(&rc, &MainInterfaceGlobal.main_layout_rect(), 4, 4);
  rc.bottom -= 0x17;
  rect_F3C_ = rc;
  rc.top = rc.bottom + 3;
  rect_F2C_ = rc;
  rc.bottom = rc.top + 0x14;

  GraphicWin::init(window_x, window_y, 0x340, window_height, 0x05000000,
                   static_cast<GraphicWin *>(&ConsoleGlobal));
  field_16C_ = 0x1A;
  fill(9);
  static_cast<Buffer &>(*this).box_sprite(&rect_EDC_, &InterfaceBoxStyles[9]);

  stringBox_.init(rect_F3C_.left + 3, rect_F3C_.top + 3,
                  rect_F3C_.right - rect_F3C_.left - 6,
                  rect_F3C_.bottom - rect_F3C_.top - 6,
                  static_cast<Win *>(&MainInterfaceGlobal), 0, -1);
  Buffer &string_canvas = static_cast<Buffer &>(stringBox_);
  string_canvas.set_write_strings_callback(&DiploWin::string_callback);
  string_canvas.set_text_color(0xFF, -1, 1, 1);
  stringBox_.set_visible_rows(100);

  editBox_.init(&rect_F2C_,
                  static_cast<Win *>(&MainInterfaceGlobal), 0);
  editBox_.add_window_flags(0x01000000);
  editBox_.field_B1C_ = 9;
  editBox_.field_B30_ = 0;
  Buffer &edit_canvas = static_cast<Buffer &>(editBox_);
  edit_canvas.set_text_color(ChatItemIdTable[faction_id_],
                             ChatShadowColors[faction_id_], 1, 1);
  editBox_.field_B20_ = 0x41;
  editBox_.field_B24_ = ChatItemIdTable[faction_id_];
  editBox_.field_B14_ = 0x80;

  // 0x444754: computed and never read (dead stores into rc in the image).
  diplo_inset_rect(&rc, &rect_EFC_, 3, 3);

  // Buttons 0 and 1 are not initialized here: the image skips the whole
  // body until the label offset reaches 0xDCC.
  for (int button_index = 0; button_index < 13; ++button_index) {
    if (button_index >= 2) {
      FlatButton &button = flatButtons_[button_index];
      button.init(label_get(0x371 + button_index), button_index,
                  button_rects_[button_index].left,
                  button_rects_[button_index].top,
                  button_rects_[button_index].right - button_rects_[button_index].left,
                  button_rects_[button_index].bottom - button_rects_[button_index].top,
                  this, 0);
      button.set_font(find_font(0x0E, 1), 0, 0, 0);
      button.redraw();
      button.set_text_color(0x41, -1, 1, 1);
      button.hide();
      // InterfaceBoxStyles begins at 0x0078D528 with 0x28-byte elements;
      // these are the native 0x78D960/0x78D988/0x78D9B0 slots 27..29.
      button.set_border_styles(&InterfaceBoxStyles[27],
                               &InterfaceBoxStyles[28],
                               &InterfaceBoxStyles[29]);
    }
  }

  stringBox_.clear();
  editBox_.set_text(0);

  if (IsMultiplayerPBEM != 0) {
    sub_441100(faction_id_, faction_id_with_);
    for (side = 0; side < 2; ++side) {
      offer_count_[side] = OfferResetCells[faction_id_][faction_id_with_].count_[side];
      offer_state_[side] = OfferResetCells[faction_id_][faction_id_with_].state_[side];
      for (int item = 0; item < 8; ++item) {
        offer_type_[side][item] =
            OfferResetCells[faction_id_][faction_id_with_].type_[side][item];
        offer_value_[side][item] =
            OfferResetCells[faction_id_][faction_id_with_].value_[side][item];
      }
    }
  } else {
    UNK3(0);
    UNK3(1);
  }

  const int player_flags = static_cast<int>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF);
  CommlinkPending = 1;
  DiploOffersDirty = 0;
  if (bit_count(player_flags) == 2)
    GameDataStateStorage.fields_.turn_control_duration_milliseconds_ *= 2;
  if (DiploDefaultFlag93A968 == 0)
    DiploDefaultFlag93A968 = 1;

  if (IsMultiplayerNet == 0 && IsMultiplayerPBEM == 0) {
    if (rand() % 2 == 0) {
      add_offer(1, 0, rand() % 0x59);
      if (rand() % 3 != 0) {
        add_offer(1, 0, rand() % 0x59);
      }
    } else {
      add_offer(1, 1, (rand() % 0x32 * 5 + 5) * 5);
    }

    switch (rand() % 4) {
      case 0: add_offer(1, 4, 0);
        // Native switch intentionally falls through to the next offers.
      case 1: add_offer(1, 3, 0);
        // Native switch intentionally falls through to the next offer.
      case 2: add_offer(1, 2, 0);
    }

    // The image lays the case-1 body out before the case-0 body.
    switch (rand() % 3) {
      case 1: offer_type_[0][0] = 2; break;
      case 0: offer_state_[1] = 1; break;
    }
  }

  set_modal(0, pop_expire, &MainInterfaceGlobal);

  if (IsMultiplayerPBEM != 0) {
    StringList &source = DiploWindow.stringBox_.strings();
    int high_faction = faction_id_with_;
    int low_faction = faction_id_;
    if (low_faction > high_faction) {
      swap(&low_faction, &high_faction);
    }
    const int source_count = source.entry_count_;
    for (int source_index = source_count - 3;
         source_index < source_count; ++source_index) {
      if (source_index < 0)
        continue;
      source.seek_pos(source_index);
      DiplomacyTranscriptLists[low_faction][high_faction].pending_text_ = source.head_ != 0
          ? source.current_->string_payload()->text : 0;
      DiplomacyTranscriptLists[low_faction][high_faction].field_20_ = 0;
      DiplomacyTranscriptLists[low_faction][high_faction].field_24_ = 0;
      DiplomacyTranscriptLists[low_faction][high_faction].add(source.current_id());
    }

    // Each side is stored twice: in the local cell as seen from here, and
    // mirrored into the counterpart's cell with the parties swapped.
    for (side = 0; side < 2; ++side) {
      OfferResetCells[faction_id_][faction_id_with_].count_[side] = offer_count_[side];
      OfferResetCells[faction_id_][faction_id_with_].state_[side] = offer_state_[side];
      OfferResetCells[faction_id_with_][faction_id_].count_[side] = offer_count_[1 - side];
      OfferResetCells[faction_id_with_][faction_id_].state_[side] = offer_state_[1 - side];
      for (int item = 0; item < 8; ++item) {
        OfferResetCells[faction_id_][faction_id_with_].type_[side][item] =
            offer_type_[side][item];
        OfferResetCells[faction_id_][faction_id_with_].value_[side][item] =
            offer_value_[side][item];
        OfferResetCells[faction_id_with_][faction_id_].type_[side][item] =
            offer_type_[1 - side][item];
        OfferResetCells[faction_id_with_][faction_id_].value_[side][item] =
            offer_value_[1 - side][item];
      }
    }

    if (DiploOffersDirty != 0
        && (offer_state_[0] != 0 || offer_count_[0] > 0))
      PendingDiplomacyFlags[faction_id_with_][faction_id_] |= 1;
  }

  if (CommlinkPending != 0) {
    CommlinkPending = 0;
    message_data(0x1505, faction_id_with_, 0, 0, 0, 0);
  }

  release_iface_mode();
  delete_iface_mode();
  close();
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004436C0 ?on_redraw@DiploWin@@QAEXXZ 0x004436C0-0x00443C25
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original constructor444FC0 establishes GraphicWin+0, SubInterface+A14 and member construction order; secondary table66B5E0 gives nineteen callback slots and adjusted receivers.", "Redraw4436C0, status443C70 and painter443E50 inspected over declared original spans; source rectangle/control owners preserve native offsets. Palette colors reuse TextColourTable[80-2*item]/[81-2*item]. Localized title producer and full extents remain unknown.", "recovery/tasks/interface-contract-report-closure.json; root coordinator owns import and independent compilation; no current byte, structural or runtime acceptance is implied."], "reason": "Recover the genuine DiploWin GraphicWin/SubInterface owner with typed callbacks and automatic base/member lifetimes; retain independent seed history and restoration obligations."}
// symbol ?on_redraw@DiploWin@@UAEXXZ
void DiploWin::on_redraw() {
  MainInterfaceGlobal.restore_back(1);

  RECT area = MainInterfaceGlobal.right_frame_rect();
  area.left += 4;
  area.right -= 4;
  area.top += 4;
  area.bottom -= 4;
  MainInterfaceGlobal.Buffer::box_sprite(&area, &InterfaceBoxStyles[3]);

  area = MainInterfaceGlobal.main_frame_rect();
  area.left += 4;
  area.top += 4;
  area.right -= 4;
  area.bottom -= 4;
  MainInterfaceGlobal.Buffer::box_sprite(&area, &InterfaceBoxStyles[3]);

  UnionRect(&area, &portrait_rect_[0], &portrait_rect_[1]);
  RECT *f4c = &rect_F4C_;
  UnionRect(&area, &area, f4c);
  area.left -= 3;
  area.right += 3;
  area.top -= 3;
  area.bottom += 3;

  Buffer::box_sprite(&area, &InterfaceBoxStyles[3]);

  UnionRect(&area, &button_rects_[2], &button_rects_[3]);
  UnionRect(&area, &area, &button_rects_[4]);
  area.left -= 3;
  area.right += 3;
  area.top -= 3;
  area.bottom += 3;
  Buffer::box_sprite(&area, &InterfaceBoxStyles[3]);

  area = rect_F9C_;
  Buffer::box_sprite(&area, &InterfaceBoxStyles[6]);
  area.left += 3;
  area.right -= 3;
  area.top += 3;
  area.bottom -= 3;
  Buffer::set_font(find_font(14, 1), 0, 0, 0);
  Buffer::set_text_color(0x99, -1, 1, 1);

  LPSTR title = Language == 0 ? "TRADE"
                              : &DiploLocalizedTradeTitle;
  Buffer::write_cent_l(title, &area);

  area = rect_FAC_;
  Buffer::box_sprite(&area, &InterfaceBoxStyles[3]);

  area = rect_FBC_;
  Buffer::box_sprite(&area, &InterfaceBoxStyles[6]);
  area.left += 3;
  area.right -= 3;
  area.top += 3;
  area.bottom -= 3;
  title = Language == 0 ? "DIPLOMACY"
                        : &DiploLocalizedDiplomacyTitle;
  Buffer::write_cent_l(title, &area);

  area = rect_FCC_;
  Buffer::box_sprite(&area, &InterfaceBoxStyles[3]);

  area.left = f4c->left;
  area.top = f4c->top;
  area.right = f4c->right;
  area.bottom = f4c->bottom;
  Buffer::box_sprite(&area, &InterfaceBoxStyles[6]);
  area.left += 3;
  area.right -= 3;
  area.top += 3;
  area.bottom -= 3;

  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0x748 / 4));
  strcat(StringTemp, BufferJoinSpace);
  strcat(StringTemp, BufferJoinSpace);
  strcat(StringTemp, SocialWinOpenParenText);
  strcat(StringTemp, label_get(0x788 / 4));
  strcat(StringTemp, SocialWinColonSpaceText);
  strcat(StringTemp, "{");

  const uint32_t treaties =
      PlayersData[faction_id_].diplo_treaties[faction_id_with_];
  if ((treaties & 1) != 0) {
    strcat(StringTemp, label_get(0x34C / 4));
    Buffer::set_text_color2(0xFA, -1, 1, 1);
  } else if ((treaties & 2) != 0) {
    strcat(StringTemp, label_get(0x350 / 4));
    Buffer::set_text_color2(0xFA, -1, 1, 1);
  } else if ((treaties & 4) != 0) {
    strcat(StringTemp, label_get(0x354 / 4));
    Buffer::set_text_color2(0xFA, -1, 1, 1);
  } else if ((treaties & 0x10) != 0) {
    strcat(StringTemp, label_get(0x358 / 4));
    Buffer::set_text_color2(0xF9, -1, 1, 1);
  } else {
    strcat(StringTemp, label_get(0x784 / 4));
    Buffer::set_text_color2(0xFB, -1, 1, 1);
  }
  strcat(StringTemp, "}");
  strcat(StringTemp, SocialWinCloseParenText);
  Buffer::write_cent_l(StringTemp, &area);

  if (MainInterfaceTileFlag != 0) {
    MainInterfaceGlobal.Buffer::box_sprite(&rect_F3C_, &InterfaceBoxStyles[0]);
    MainInterfaceGlobal.Buffer::box_sprite(&rect_F2C_, &InterfaceBoxStyles[0]);
  }

  draw_who();
  draw_whom();
  draw_offers();
}

extern int TextColourTable[195];

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: two separate null-check ifs (worse fit); localRect copy before set_clip (worse fit); closest is buf cached once, single || null check, localRect copied after set_clip - 5-edit MISMATCH
// ORIGINAL: 0x00443E50 ?on_iface_dialog_item_back_draw@DiploWin@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x00443E50-0x00443F0C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original constructor444FC0 establishes GraphicWin+0, SubInterface+A14 and member construction order; secondary table66B5E0 gives nineteen callback slots and adjusted receivers.", "Redraw4436C0, status443C70 and painter443E50 inspected over declared original spans; source rectangle/control owners preserve native offsets. Palette colors reuse TextColourTable[80-2*item]/[81-2*item]. Localized title producer and full extents remain unknown.", "recovery/tasks/interface-contract-report-closure.json; root coordinator owns import and independent compilation; no current byte, structural or runtime acceptance is implied."], "reason": "Recover the genuine DiploWin GraphicWin/SubInterface owner with typed callbacks and automatic base/member lifetimes; retain independent seed history and restoration obligations."}
// symbol ?on_iface_dialog_item_back_draw@DiploWin@@UAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
int DiploWin::on_iface_dialog_item_back_draw(GraphicWin *window, int, int item,
                                             RECT *rect) {
  if (window == 0 || rect == 0) {
    return 0;
  }
  Buffer &canvas = static_cast<Buffer &>(*window);
  canvas.restore_clip();
  RECT local = *rect;

  // These words belong to the text.pcx palette published by textcolor_init.
  // The native addresses are TextColourTable[80]/[81], walked backward as
  // two-int pairs for each item.
  canvas.set_text_color(TextColourTable[80 - 2 * item],
                        TextColourTable[81 - 2 * item], 1, 1);
  if (item == 0) {
    window->fill(rect, item);
  } else {
    canvas.box_sprite(&local, &InterfaceBoxStyles[27 + item]);
  }
  return 1;
}

// ORIGINAL: 0x00443C70 ?on_status@DiploWin@@QAEXH@Z 0x00443C70-0x00443D77 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original constructor444FC0 establishes GraphicWin+0, SubInterface+A14 and member construction order; secondary table66B5E0 gives nineteen callback slots and adjusted receivers.", "Redraw4436C0, status443C70 and painter443E50 inspected over declared original spans; source rectangle/control owners preserve native offsets. Palette colors reuse TextColourTable[80-2*item]/[81-2*item]. Localized title producer and full extents remain unknown.", "recovery/tasks/interface-contract-report-closure.json; root coordinator owns import and independent compilation; no current byte, structural or runtime acceptance is implied.", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/00443c70.cpp"], "reason": "Recover the genuine DiploWin GraphicWin/SubInterface owner with typed callbacks and automatic base/member lifetimes; retain independent seed history and restoration obligations. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol ?on_status@DiploWin@@UAEXH@Z
void DiploWin::on_status(int mode) {
  if (mode != 0) {
    GraphicWin *const front = static_cast<GraphicWin *>(this);
    front->on_redraw();
    for (int i = 0; i != 13; ++i) {
      flatButtons_[i].show(0);
    }
    slide_show(front, 2);
    front->Win::show(3);
    stringBox_.show(0);
    editBox_.show(0);
    editBox_.time_.start();
    front->redraw();
    MainInterfaceGlobal.soft_update();
    return;
  }

  stringBox_.hide();
  editBox_.time_.stop();
  editBox_.hide();
  GraphicWin *const front = static_cast<GraphicWin *>(this);
  slide_hide(front, 2);
  front->Win::hide();
  for (int i = 0; i != 13; ++i) {
    flatButtons_[i].hide();
  }
}

// SubInterface vtable slots with exact native shared-stub targets. These are
// declarations/bodies to add only after the secondary-base layout is adopted.
// Native secondary table66B5E0 selects these shared return bodies at slots
// 1,2,14,16,17. They do not create additional original identities or claims.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolled@DiploWin@@UAEXHH@Z 0x00444FC0
void DiploWin::on_iface_scrolled(int, int) {}
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_iface_scrolling@DiploWin@@UAEXHH@Z 0x00444FC0
void DiploWin::on_iface_scrolling(int, int) {}
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_iface_button_pressed@DiploWin@@UAEHH@Z 0x00444FC0
int DiploWin::on_iface_button_pressed(int) { return 0; }
// RECOVERY_SHARED: v1 0x00406B20 0x00406B23 ?on_iface_group_clicked@DiploWin@@UAEXHHH@Z 0x00444FC0
void DiploWin::on_iface_group_clicked(int, int, int) {}
// RECOVERY_SHARED: v1 0x00443E40 0x00443E45 ?on_iface_dialog_back_draw@DiploWin@@UAEHPAVGraphicWin@@@Z 0x00444FC0
int DiploWin::on_iface_dialog_back_draw(GraphicWin *) { return 0; }

// The diplomacy window's redraw latch is writable application storage, not a
// pointer into the original executable. on_key_click and the offer paths write it.
// RECOVERY_OBJECT: v2 0x007492D0 0x007492D4 ?DiploOffersDirty@@3HA 0x00442590
int DiploOffersDirty;

// Original initialized .data and literal evidence, inspected from the pinned image.
// DiploKnownTechColor (0x0068A5C4) is IfaceColorTable[9]; its identity moved
// to interface_assets.cpp.
// RECOVERY_OBJECT: v2 0x006839F4 0x006839F6 ?DiploKnownTechOpen@@3PADA 0x00443170
char DiploKnownTechOpen[2] = "(";
// RECOVERY_OBJECT: v2 0x006839F8 0x006839FA ?DiploKnownTechClose@@3PADA 0x00443170
char DiploKnownTechClose[2] = ")";

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00443170 0x004436A0 0x004436B8 0x00443480
// ORIGINAL: 0x00443170 ?draw_offers@DiploWin@@QAEXXZ 0x00443170-0x0044369F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:def4db5d8cde1ddd00c5735adc0e3cd230ec02b8:src/recovered/units/00443170.cpp"], "reason": "Typed offer-panel rendering, technology/energy/treaty/faction labels and Spot ownership, replacing seed raw offsets."}
void DiploWin::draw_offers() {
    int line_height = font_line_advance(*find_font(14, 0));
    int side = 0;
    do {
        spot_.kill_type(side);
        set_font(find_font(14, 0), find_font(14, 1), find_font(14, 2), 0);
        RECT panel;
        if (side) {
            panel = portrait_rect_[1];
            panel.right += -1 - FactionsArt[faction_id_with_].datalink.width();
        } else {
            panel = portrait_rect_[0];
            panel.left += FactionsArt[faction_id_].datalink.width() + 1;
        }
        panel.left += 3;
        panel.top += 3;
        panel.right -= 3;
        int width = panel.right - panel.left;
        set_text_color(ChatItemIdTable[side ? faction_id_with_ : faction_id_],
                       ChatShadowColors[side ? faction_id_with_ : faction_id_], 1, 1);
        StringTemp[0] = 0;
        {
            int f = side ? faction_id_with_ : faction_id_;
            PluralityDefault = 0;
            GenderDefault = Players[f].is_leader_female;
            strcat(StringTemp, Players[f].title_leader);
        }
        strcat(StringTemp, BufferJoinSpace);
        {
            int f = side ? faction_id_with_ : faction_id_;
            PluralityDefault = 0;
            GenderDefault = Players[f].is_leader_female;
            strcat(StringTemp, Players[f].name_leader);
        }
        strcat(StringTemp, BufferJoinSpace);
        int count = offer_count_[side];
        StringTemp[0] = 0;
        if (count == 0) {
            strcat(StringTemp, label_get(0x734 / 4));
            write_cent_text(StringTemp, panel.left, panel.top, width);
        } else {
            strcat(StringTemp, label_get(0x738 / 4));
            write_cent_text(StringTemp, panel.left, panel.top, width);
        }
        int top = panel.top + line_height;
        for (int i = 0; i < offer_count_[side]; ++i) {
            StringTemp[0] = 0;
            set_text_color(ChatItemIdTable[side ? faction_id_with_ : faction_id_],
                           ChatShadowColors[side ? faction_id_with_ : faction_id_], 1, 1);
            switch (offer_type_[side][i]) {
            case 0:
                if (side == 1 && offer_value_[1][i] < 89
                    && has_tech(offer_value_[1][i], faction_id_)) {
                    set_text_color(DiploKnownTechColor, 0, side, side);
                    strcat(StringTemp, DiploKnownTechOpen);
                    say_tech(StringTemp, offer_value_[1][i], side);
                    strcat(StringTemp, DiploKnownTechClose);
                } else {
                    say_tech(StringTemp, offer_value_[side][i], 1);
                }
                break;
            case 1: {
                char text[80];
                _itoa(offer_value_[side][i], text, 10);
                strcat(StringTemp, text);
                strcat(StringTemp, BufferJoinSpace);
                strcat(StringTemp, label_get(0x90 / 4));
                break;
            }
            case 2:
                strcat(StringTemp, label_get(0x34C / 4));
                break;
            case 3:
                strcat(StringTemp, label_get(0x350 / 4));
                break;
            case 4:
                strcat(StringTemp, label_get(0x354 / 4));
                break;
            case 5: {
                strcat(StringTemp, label_get(0x980 / 4));
                strcat(StringTemp, BufferJoinSpace);
                int f = offer_value_[side][i];
                GenderDefault = Players[f].noun_gender;
                PluralityDefault = Players[f].is_noun_plural;
                strcat(StringTemp, Players[f].noun_faction);
                break;
            }
            }
            write_cent_text(StringTemp, panel.left, top, width);
            RECT item;
            spot_.add(i, side, set_rectangle_dimensions(&item, panel.left, top, width, line_height));
            top += line_height;
        }
        ++side;
    } while (side < 2);
}

// Source-owned faction/network operations; shared declarations await owner review.
int __cdecl diplo_lock(int timeout);
void __cdecl diplo_unlock();

// ORIGINAL: 0x004426F0 ?check_done@DiploWin@@QAEXXZ 0x004426F0-0x00442736 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:def4db5d8cde1ddd00c5735adc0e3cd230ec02b8:src/recovered/004426f0.cpp"], "reason": "Rehome seed into typed DiploWin owner; states and counterpart are source members."}
void DiploWin::check_done() {
    if (offer_state_[0] == 1 && offer_state_[1] == 1) {
        message_data(0x150A, faction_id_with_, 0, 0, 0, 0);
        if (IsMultiplayerPBEM != 0) {
            do_it();
        }
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00442740 0x004429A0 0x004429B8 0x004427F0
// ORIGINAL: 0x00442740 ?do_it@DiploWin@@QAEXXZ 0x00442740-0x0044299D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:def4db5d8cde1ddd00c5735adc0e3cd230ec02b8:src/recovered/units/00442740.cpp"], "reason": "Rehome exchange execution into source-owned DiploWin arrays and faction/network globals."}
void DiploWin::do_it() {
    if ((faction_id_ <= faction_id_with_ || IsMultiplayerPBEM != 0)
        && CommlinkPending <= 1) {
        CommlinkPending = 2;
        if (diplo_lock(200) == 0) {
            int side = 0;
            do {
                for (int i = 0; i < offer_count_[side]; ++i) {
                    int giver = side ? faction_id_with_ : faction_id_;
                    int recipient = side ? faction_id_ : faction_id_with_;
                    switch (offer_type_[side][i]) {
                    case 2: net_treaty_on(giver, recipient, 1, 1); break;
                    case 3: net_treaty_on(giver, recipient, 2, 1); break;
                    case 4: net_treaty_on(giver, recipient, 4, 1); break;
                    case 0: {
                        int tech = offer_value_[side][i];
                        if (tech < 89 && has_tech(tech, recipient)) break;
                        net_tech(recipient, tech, giver, 1);
                        break;
                    }
                    case 1: {
                        int reserves = PlayersData[giver].energy_reserves;
                        int amount = offer_value_[side][i];
                        if (amount < 0 || reserves < 0) {
                            amount = 0;
                        } else if (amount > reserves) {
                            amount = reserves;
                        }
                        net_energy(giver, -amount, recipient, amount, 1);
                        break;
                    }
                    case 5: {
                        int target = offer_value_[side][i];
                        if (((unsigned char)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << target)) != 0
                            && !(PlayersData[giver].diplo_treaties[target] & 0x10)) {
                            message_data(0x150F, target, giver, recipient, 0, 0);
                        }
                        net_treaty_on(giver, target, 8, 0);
                        net_double_cross(giver, target, recipient, 1);
                        break;
                    }
                    }
                }
                ++side;
            } while (side < 2);
            diplo_unlock();
            CommlinkPending = 1;
            offer_state_[1] = 3;
            offer_state_[0] = 3;
            offer_count_[1] = 0;
            offer_count_[0] = 0;
            message_data(0x150D, faction_id_with_, 0, 0, 0, 0);
            redraw();
        }
    }
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x00442A10 0x00442D98 0x00442DB4 0x00442C50
// ORIGINAL: 0x00442A10 ?draw_who@DiploWin@@QAEXXZ 0x00442A10-0x00442D98 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:def4db5d8cde1ddd00c5735adc0e3cd230ec02b8:src/recovered/units/00442a10.cpp"], "reason": "Typed DiploWin party rendering using source-owned FactionArt datalink portraits, rectangles and label storage."}
void DiploWin::draw_who() {
    if (faction_id_ > 0) {
        set_font(find_font(14, 1), 0, 0, 0);
        StringTemp[0] = 0;
        {
            int f = faction_id_;
            PluralityDefault = 0;
            GenderDefault = Players[f].is_leader_female;
            strcat(StringTemp, Players[f].title_leader);
        }
        strcat(StringTemp, BufferJoinSpace);
        {
            int f = faction_id_;
            PluralityDefault = 0;
            GenderDefault = Players[f].is_leader_female;
            strcat(StringTemp, Players[f].name_leader);
        }
        set_text_color(ChatItemIdTable[faction_id_], ChatShadowColors[faction_id_], 1, 1);
        RECT area = leader_rect_[0];
        box_sprite(&area, &InterfaceBoxStyles[6]);
        area.left += 3;
        area.right -= 3;
        area.top += 3;
        area.bottom -= 3;
        CharUpperA(StringTemp);
        write_cent_l(StringTemp, &area);
        area = portrait_rect_[0];
        box_sprite(&area, &InterfaceBoxStyles[0]);
        Sprite *portrait = &FactionsArt[faction_id_].datalink;
        set_rectangle_dimensions(&area, area.left + 1, area.top + 1,
                                 portrait->width(), portrait->height());
        portrait->draw(this, portrait->transparent_index(), area.left, area.top, 1, 1);
        area.left += 4;
        area.right -= 4;
        area.bottom -= 4;
        area.top = area.bottom - 20;
        set_font(find_font(14, 1), 0, 0, 0);
        StringTemp[0] = 0;
        switch (offer_state_[0]) {
        case 0:
            strcat(StringTemp, label_get(0x73C / 4));
            break;
        case 1:
            strcat(StringTemp, label_get(0x740 / 4));
            break;
        case 2:
            set_text_color(ChatItemIdTable[0], ChatShadowColors[0], 1, 1);
            strcat(StringTemp, label_get(0x744 / 4));
            break;
        case 3:
            strcat(StringTemp, label_get(0x780 / 4));
            break;
        case 4:
            set_text_color(ChatItemIdTable[0], ChatShadowColors[0], 1, 1);
            strcat(StringTemp, label_get(0x968 / 4));
            break;
        case 5:
            set_text_color(ChatItemIdTable[0], ChatShadowColors[0], 1, 1);
            strcat(StringTemp, label_get(0x95C / 4));
            break;
        case 6:
            set_text_color(251, 0, 1, 1);
            strcat(StringTemp, label_get(0x960 / 4));
            break;
        }
        CharUpperA(StringTemp);
        map_colors(&area, ShadowColorTable);
        area.left += 3;
        area.right -= 3;
        area.top += 3;
        area.bottom -= 3;
        write_cent_l(StringTemp, &area);
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00442DC0 0x0044314C 0x00443168 0x00443000
// ORIGINAL: 0x00442DC0 ?draw_whom@DiploWin@@QAEXXZ 0x00442DC0-0x00443149 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:def4db5d8cde1ddd00c5735adc0e3cd230ec02b8:src/recovered/units/00442dc0.cpp"], "reason": "Typed DiploWin party rendering using source-owned FactionArt datalink portraits, rectangles and label storage."}
void DiploWin::draw_whom() {
    if (faction_id_ > 0) {
        set_font(find_font(14, 1), 0, 0, 0);
        StringTemp[0] = 0;
        {
            int f = faction_id_with_;
            PluralityDefault = 0;
            GenderDefault = Players[f].is_leader_female;
            strcat(StringTemp, Players[f].title_leader);
        }
        strcat(StringTemp, BufferJoinSpace);
        {
            int f = faction_id_with_;
            PluralityDefault = 0;
            GenderDefault = Players[f].is_leader_female;
            strcat(StringTemp, Players[f].name_leader);
        }
        set_text_color(ChatItemIdTable[faction_id_with_], ChatShadowColors[faction_id_with_], 1, 1);
        RECT area = leader_rect_[1];
        box_sprite(&area, &InterfaceBoxStyles[6]);
        area.left += 3;
        area.right -= 3;
        area.top += 3;
        area.bottom -= 3;
        CharUpperA(StringTemp);
        write_cent_l(StringTemp, &area);
        area = portrait_rect_[1];
        box_sprite(&area, &InterfaceBoxStyles[0]);
        Sprite *portrait = &FactionsArt[faction_id_with_].datalink;
        set_rectangle_dimensions(&area, area.right - portrait->width() - 1, area.top + 1,
                                 portrait->width(), portrait->height());
        portrait->draw(this, portrait->transparent_index(), area.left, area.top, 1, 1);
        area.left += 4;
        area.right -= 4;
        area.bottom -= 4;
        area.top = area.bottom - 20;
        set_font(find_font(14, 1), 0, 0, 0);
        StringTemp[0] = 0;
        switch (offer_state_[1]) {
        case 0:
            strcat(StringTemp, label_get(0x73C / 4));
            break;
        case 1:
            strcat(StringTemp, label_get(0x740 / 4));
            break;
        case 2:
            set_text_color(ChatItemIdTable[0], ChatShadowColors[0], 1, 1);
            strcat(StringTemp, label_get(0x744 / 4));
            break;
        case 3:
            strcat(StringTemp, label_get(0x780 / 4));
            break;
        case 4:
            set_text_color(ChatItemIdTable[0], ChatShadowColors[0], 1, 1);
            strcat(StringTemp, label_get(0x968 / 4));
            break;
        case 5:
            set_text_color(ChatItemIdTable[0], ChatShadowColors[0], 1, 1);
            strcat(StringTemp, label_get(0x95C / 4));
            break;
        case 6:
            set_text_color(251, 0, 1, 1);
            strcat(StringTemp, label_get(0x960 / 4));
            break;
        }
        CharUpperA(StringTemp);
        map_colors(&area, ShadowColorTable);
        area.left += 3;
        area.right -= 3;
        area.top += 3;
        area.bottom -= 3;
        write_cent_l(StringTemp, &area);
    }
}

// ORIGINAL: 0x004412D0 ?UNK1@DiploWin@@QAEXXZ 0x004412D0-0x004412ED BYTE_EXACT
void DiploWin::UNK1() {
    stringBox_.clear();
    editBox_.set_text(0);
}

// ORIGINAL: 0x004412F0 ?new_chat@DiploWin@@QAEXPADH@Z 0x004412F0-0x004413A7 BYTE_EXACT
// Historical seed evidence: the faction record is indexed by its 0x59C stride;
// the seed's remaining register-order difference was not a structural clue.
void DiploWin::new_chat(char *text, int faction_id) {
    char line[256];
    line[0] = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = 0;
    strcat(line, Players[faction_id].name_leader);
    CharUpperA(line);
    strcat(line, SocialWinColonSpaceText);
    strcat(line, BufferJoinSpace);
    strcat(line, text);
    stringBox_.add(line, faction_id, 1);
    DiploOffersDirty = 1;
}

// Slot 62 (+0xF8) is inherited GraphicWin::redraw(); source calls retain
// the pinned receiver and virtual dispatch.

// ORIGINAL: 0x00442560 ?on_key_down@DiploWin@@QAEHH@Z 0x00442560-0x0044258A BYTE_EXACT
// symbol    ?on_key_down@DiploWin@@UAEHH@Z
int DiploWin::on_key_down(int key) {
    if (key == 0x0D || key == 0x1B) {
        return 0;
    }
    return editBox_.on_key_down(key);
}

// ORIGINAL: 0x004429C0 ?done_deal@DiploWin@@QAEXXZ 0x004429C0-0x004429E7 BYTE_EXACT
void DiploWin::done_deal() {
    offer_state_[1] = 3;
    offer_state_[0] = 3;
    offer_count_[1] = 0;
    offer_count_[0] = 0;
    redraw();
}

// Historical TRIED entries below describe the scaffolded seed, not this typed member body.
// ORIGINAL: 0x00442590 ?on_key_click@DiploWin@@QAEHHH@Z 0x00442590-0x004426ED BYTE_EXACT
// symbol    ?on_key_click@DiploWin@@UAEHHH@Z
// TRIED: Seed provenance: 0.74 mnemonic similarity - the edit box's slot-25 vtable call needed a shim in the scaffold; the tree's own Win::vslot_25 declares the same two-int signature, and Win slot 58 is release_modal, so both dispatch through the real declarations here. The private DiploWin state stays raw-offset (0xAB8, the 0xE490 edit text, stringBox_, editBox_).
// TRIED: DAT_00946a50/DAT_00946a84 are per-faction record fields (stride 1436 = 0x59C) whose address itself is indexed - declared as extern arrays instead of the auto-scaffold's plain const-pointer globals (which would fold the stride multiply away), per the same lever as 0x00423000's g_faction_table. slot 25 on the edit_box_ sub-object takes two ints (a1,a2) - the auto-emitted VCall shim is nullary, so a second small vtable-shim class (VCall25) declares only the 26 ordinals needed with the right signature on the last one; a bare __thiscall function-pointer cast is C4234 on VC6. Landed at 0.74 mnemonic similarity.
int DiploWin::on_key_click(int repeat, int key) {
    if (key == 0x1B) {
        release_modal();
        return 1;
    }
    if (key == 0x0D) {
        char *const text = editBox_.text_;
        if (text != 0 && strlen(text) != 0) {
            message_chat(0x1E00, faction_id_with_, text);
            // 4425F5 reads the live faction after the chat callback.
            const int faction = ConsoleGlobal.faction_id_;

            char line[256];
            line[0] = 0;
            PluralityDefault = 0;
            GenderDefault = Players[faction].is_leader_female;
            strcat(line, Players[faction].name_leader);
            CharUpperA(line);
            strcat(line, SocialWinColonSpaceText);
            strcat(line, BufferJoinSpace);
            strcat(line, editBox_.text_);

            stringBox_.add(line, faction, 1);
            DiploOffersDirty = 1;
            editBox_.set_text(0);
            return 1;
        }
        release_modal();
        return 1;
    }
    editBox_.on_key_click(repeat, key);
    return 1;
}


/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00442A00 ?UNK5@DiploWin@@QAEXXZ 0x00442A00-0x00442A01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK5@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK5() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004429F0 ?UNK4@DiploWin@@QAEXH@Z 0x004429F0-0x004429F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK4@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK4(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443D80 ?on_iface_left_click@DiploWin@@QAEXHH@Z 0x00443D80-0x00443D83 BYTE_EXACT
// symbol ?on_iface_left_click@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443D90 ?on_iface_left_up@DiploWin@@QAEXHH@Z 0x00443D90-0x00443D93 BYTE_EXACT
// symbol ?on_iface_left_up@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_up@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_up(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DA0 ?on_iface_right_click@DiploWin@@QAEXHH@Z 0x00443DA0-0x00443DA3 BYTE_EXACT
// symbol ?on_iface_right_click@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DB0 ?on_iface_left_down@DiploWin@@QAEXHH@Z 0x00443DB0-0x00443DB3 BYTE_EXACT
// symbol ?on_iface_left_down@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DC0 ?on_iface_right_down@DiploWin@@QAEXHH@Z 0x00443DC0-0x00443DC3 BYTE_EXACT
// symbol ?on_iface_right_down@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DD0 ?on_iface_mouse_move@DiploWin@@QAEXHH@Z 0x00443DD0-0x00443DD3 BYTE_EXACT
// symbol ?on_iface_mouse_move@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DE0 ?on_iface_mouse_leave@DiploWin@@QAEXHH@Z 0x00443DE0-0x00443DE3 BYTE_EXACT
// symbol ?on_iface_mouse_leave@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DF0 ?on_iface_selected@DiploWin@@QAEXHH@Z 0x00443DF0-0x00443DF3 BYTE_EXACT
// symbol ?on_iface_selected@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E00 ?on_iface_left_double_click@DiploWin@@QAEXHH@Z 0x00443E00-0x00443E03 BYTE_EXACT
// symbol ?on_iface_left_double_click@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E10 ?on_iface_right_double_click@DiploWin@@QAEXHH@Z 0x00443E10-0x00443E13 BYTE_EXACT
// symbol ?on_iface_right_double_click@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E20 ?on_iface_button_clicked@DiploWin@@QAEXH@Z 0x00443E20-0x00443E23 BYTE_EXACT
// symbol ?on_iface_button_clicked@DiploWin@@UAEXH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_button_clicked(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E30 ?on_iface_button_toggled@DiploWin@@QAEXHH@Z 0x00443E30-0x00443E33 BYTE_EXACT
// symbol ?on_iface_button_toggled@DiploWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00443E40 ?on_iface_dialog_back_draw@DiploWin@@QAEHH@Z 0x00443E40-0x00443E45 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?on_iface_dialog_back_draw@DiploWin@@QAEHH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int DiploWin::on_iface_dialog_back_draw(int) {
    return 0;
}


/*
Purpose: Clear the two cached values at 0xA24 and 0xA28.
// ORIGINAL: 0x004413B0 ?UNK2@DiploWin@@QAEXXZ 0x004413B0-0x004413BE BYTE_EXACT
// size      14 bytes
// prototype void (__thiscall ?UNK2@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK2() {
    for (int side = 0; side != 2; ++side) {
        offer_state_[side] = 0;
    }
}


/*
Purpose: Clear one entry of the array at 0xA1C, then clear the two words at
         0xA24 and 0xA28.

             mov dword [ecx+eax*4+0xA1C],0
             lea eax,[ecx+0xA24] / xor ecx,ecx
             mov [eax],ecx / mov [eax+4],ecx

         The index is NOT bounds checked - the original stores through
         `eax*4` whatever the caller passed - so this does not add a guard the
         program does not have.

         Original indices 2/3 reach the following words at 0xA24/0xA28;
         they are not additional offer_count_ elements. The indexed store
         and both unconditional clears retain their order. Invalid-index
         compiler effects remain a restoration obligation, not a valid
         C++ array-domain claim.
// ORIGINAL: 0x004413C0 ?UNK3@DiploWin@@QAEXH@Z 0x004413C0-0x004413E2 BYTE_EXACT
// size      34 bytes
// prototype void (__thiscall ?UNK3@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK3(int a1) {
    offer_count_[a1] = 0;
    for (int side = 0; side != 2; ++side) {
        offer_state_[side] = 0;
    }
}


/*
// ORIGINAL: 0x00443C30 ?show@DiploWin@@QAEXH@Z 0x00443C30-0x00443C4B BYTE_EXACT
// symbol    ?show@DiploWin@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void DiploWin::show(int visible) {
    if (!is_visible()) {
        set_iface_mode();
    }
}

/*
// ORIGINAL: 0x00443C50 ?hide@DiploWin@@QAEXXZ 0x00443C50-0x00443C68 BYTE_EXACT
// symbol    ?hide@DiploWin@@UAEXXZ
// size      24 bytes
// prototype void (__thiscall ?hide@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// indirect  0x00443C60
Status: Complete
*/
void DiploWin::hide() {
    if (is_visible()) {
        release_modal();
    }
}

// DiploOffersDirty (0x007492D0) - the diplomacy-window redraw latch. add_offer,
// kill_offer, new_chat and on_button_clicked all set it to 1 as they change the
// offer lists; exec (0x00443F10) tests it and clears it. It is now owned storage.

// ORIGINAL: 0x004413F0 ?kill_offer@DiploWin@@QAEXHH@Z 0x004413F0-0x0044148C FILE BYTE_EXACT
// TRIED: `int idx = a2;` declared inside the inner `if (a2 < count - 1)` (mismatch, idx assign shifted 5 instructions late) and hoisted above it with a separate `countMinus1` temp (MNEMONIC_ONLY, worse). Landed the inner-if placement: MISMATCH #23, mnemonic similarity 0.98, same byte count (156/156).
// size      156 bytes
// prototype void (__thiscall ?kill_offer@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00592EE0
// indirect  0x0044147F

void DiploWin::kill_offer(int a1, int a2) {
    if (a1 == 0) {
        message_data(0x1507, faction_id_with_, a2, 0, 0, 0);
    }
    if (a2 < offer_count_[a1]) {
        for (int i = a2; i < offer_count_[a1] - 1; i++) {
            offer_type_[a1][i] = offer_type_[a1][i + 1];
            offer_value_[a1][i] = offer_value_[a1][i + 1];
        }
        offer_count_[a1]--;
        DiploOffersDirty = 1;
        UNK2();
        redraw();
    }
}

// ORIGINAL: 0x00441490 ?add_offer@DiploWin@@QAEXHHH@Z 0x00441490-0x004415B9 FILE BYTE_EXACT
// size      297 bytes
// prototype void (__thiscall ?add_offer@DiploWin@@QAEXHHH@Z)(DiploWin* this, int, int, int)
// callers   3   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004413F0 0x00592EE0
// indirect  0x00441598
// Historical seed: the complete unit measured MISMATCH; its scaffolding
// influenced emission. This compiled owner retains the body and original
// mapping, not the retired unit-generation and scoring route.


void DiploWin::add_offer(int a1, int a2, int a3) {
    DiploOffersDirty = 1;

    int i;
    for (i = 0; i < offer_count_[a1]; i++) {
        if (a2 == 4 || a2 == 2 || a2 == 3) {
            int existing = offer_type_[a1][i];
            if (existing == 4 || existing == 2 || existing == 3) {
                goto found;
            }
        }
        if (a2 == 1) {
            if (offer_type_[a1][i] == 1) {
                if (a3 <= 0) {
                    kill_offer(a1, i);
                    return;
                }
                goto found;
            }
        } else if (a2 == 0) {
            if (offer_type_[a1][i] == 0 && offer_value_[a1][i] == a3) {
                return;
            }
        } else if (a2 == 5) {
            if (offer_type_[a1][i] == 5 && offer_value_[a1][i] == a3) {
                return;
            }
        }
    }

    if (offer_count_[a1] >= 8) {
        return;
    }
    i = offer_count_[a1]++;

found:
    {
        offer_type_[a1][i] = a2;
        offer_value_[a1][i] = a3;
        UNK2();
        if (a1 == 0) {
            message_data(0x1506, faction_id_with_,
                        a2, a3, a1, a1);
        }
        redraw();
    }
}

// Compiler-generated source-owned lifecycle entry; no handwritten wrapper.
/*
// ORIGINAL: 0x00440F30 sub_440f30 0x00440F30-0x00440F54 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c02baa72365fcd4a910683a3dc9b3c39399a9191:src/recovered/00440f30.cpp", "pinned:terranx_original lifecycle0x440f30", "source:src/diplowin.cpp"], "reason": "Generated complete initializer for the genuine64-element DiplomacyTranscriptLists array composes array construction and registration of reverse array destruction. Preserve complete original spans, catalogue names, claims and historical lessons; current compiler emission is not a fresh grant or linked selection proof."}
// symbol    _$E21
*/

// Historical seed finding: Native vector constructor receives64 elements with30-byte stride, constructor4450B0 and destructor406820, then registers440F60.
// Historical seed finding: Current vector/registration helper delegation is generated lowering; original full36-byte span remains a matching obligation.

// Compiler-generated source-owned lifecycle entry; no handwritten wrapper.
/*
// ORIGINAL: 0x00440F60 sub_440f60 0x00440F60-0x00440F74 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c02baa72365fcd4a910683a3dc9b3c39399a9191:src/recovered/units/00440f60.cpp", "pinned:terranx_original lifecycle0x440f60", "source:src/diplowin.cpp"], "reason": "Generated reverse array teardown owns the complete DiplomacyTranscriptLists64-element StringList lifecycle. Preserve complete original spans, catalogue names, claims and historical lessons; current compiler emission is not a fresh grant or linked selection proof."}
// symbol    _$E19
*/

// Historical seed finding: Native vector destruction helper6456E4 takes the array737CD8, stride30, count40hex and complete StringList destruction406820.
// Historical seed finding: Opaque void-pointer callback spelling was a scaffold workaround, not the actual source array type.

// Compiler-generated source-owned lifecycle entry; no handwritten wrapper.
/*
// ORIGINAL: 0x004450B0 sub_4450b0 0x004450B0-0x0044511C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c02baa72365fcd4a910683a3dc9b3c39399a9191:src/leaf_recoveries.cpp", "pinned:terranx_original lifecycle0x4450b0", "source:src/diplowin.cpp"], "reason": "The actual compiler-generated StringList constructor owns virtual-base allocation capture, inherited field initialization and both derived table transitions. This replaces the raw-address leaf transcription without changing StringList members. Preserve complete original spans, catalogue names, claims and historical lessons; current compiler emission is not a fresh grant or linked selection proof."}
// symbol    ??0StringList@@QAE@XZ
// TRIED: homed into game.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: plain constructor with raw pointer-offset writes (eax=ecx=this, returns this implicitly). Diverges at instr #2: original pushes ecx for a compiler-generated [ebp-4] unwind-context local (a partially-constructed-object marker) that a plain non-throwing body never allocates.
*/

// Historical seed finding: Original30-byte object has virtual allocation base at28; constructor captures StringAllocationHeap and clears it only for complete-object construction.
// Historical seed finding: Original inherited fields8/C/10/14/18 are zeroed; trailing StringList pending fields are not silently initialized.
// Historical seed finding: Prior attribution to game.cpp and raw-pointer constructor experiments are historical, not proof of original translation-unit ownership.


// Superseded seed removal retains historical Git provenance below.
// ORIGINAL: 0x004415C0 ?on_button_clicked@DiploWin@@QAEXH@Z 0x004415C0-0x004424C6;0x00654A50-0x00654CEA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/004415c0.cpp", "task:diplowin-button-handler-owner", "Pinned native button+2 fifteen-way dispatch, original local Popup lifetime; corrected final offline random state writes and row-directed treaty checks. Primary table slot44; real SubInterface remains independent. Preserve observed gender/plural assignment order."], "reason": "Recover this complete operation in its genuine C++ popup/diplomacy owner using actual members, virtual dispatch and typed shared dependencies; retain original identity, spans, claims and seed discoveries."}
// symbol    ?on_button_clicked@DiploWin@@UAEXH@Z
// TRIED: 15-way jump table (resolved in the brief) transcribed as a switch/goto over (a1+2); same Popup-teardown-cascade shape as the other DiploWin/PickWin siblings, plus one extra pair of vtable-relative writes (self and self+0x444) for an embedded RadioButton this function's Dialogs region has that the others don't. Cases 5/6/8/9 (tech-trade and base-swap proposal loops) are the largest handlers and were transcribed at "good faith" fidelity - the array-scan/skip conditions and REL() bit tests were matched to the raw bytes but not independently re-verified a second time given the time remaining. sim 0.12-0.34.
// WORKAROUND(inline): on_button_clicked scans side 0's offers for a tech
// (type 0) entry five times with identical code. Written out, the function
// keeps one more unit of /Ob2 budget than the image and its last early
// ~Popup teardown (and the case 12 GenderDefault stores) come out one level
// deeper; one small inline scan helper reproduces the image exactly. The
// original helper's name and home are unknown.
static inline bool diplo_tech_offered(const int *types, const int *values,
                                      int count, int value) {
  for (int i = 0; i < count; ++i)
    if (types[i] == 0 && values[i] == value) return true;
  return false;
}

void DiploWin::on_button_clicked(int button_id) {
  // The image's large local frame is one Popup/BasePop object. Its Dialogs,
  // Scroll, buttons, lists, Sprite and Heap are real owned subobjects, so the
  // automatic lifetime also owns the early-return cleanup paths.
  Popup popup;

  switch (button_id) {
    case 2:
      DiploOffersDirty = 1;
      offer_state_[0] = 1;
      redraw();
      message_data(0x1509, faction_id_with_, 1, 0, 0, 0);
      if (offer_state_[0] == 1 && offer_state_[1] == 1) {
        message_data(0x150A, faction_id_with_, 0, 0, 0, 0);
        if (IsMultiplayerPBEM != 0) do_it();
      }
      break;

    case 3: {
      DiploOffersDirty = 1;
      offer_state_[0] = 2;
      redraw();
      message_data(0x1509, faction_id_with_, 2, 0, 0, 0);
      if (IsMultiplayerNet != 0 || IsMultiplayerPBEM != 0) break;

      offer_count_[1] = 0;
      const int energy_offer = rand() % 2;
      if (energy_offer == 0) {
        add_offer(1, 0, rand() % 89);
        if (rand() % 3 != 0) add_offer(1, 0, rand() % 89);
      } else {
        const int amount = rand() % 50;
        add_offer(1, 1, (amount * 5 + 5) * 5);
      }

      // The original switch deliberately falls through: rolls 0, 1 and 2
      // add three, two and one treaty offers respectively.
      switch (rand() % 4) {
        case 0:
          add_offer(1, 4, 0);
          // fall through
        case 1:
          add_offer(1, 3, 0);
          // fall through
        case 2: add_offer(1, 2, 0);
        case 3: break;
      }

      // Preserve the final native rand()%3 side effects. The second arm is
      // an observed raw write to this+0xA2C, even though it does not update
      // the first side's offer count.
      switch (rand() % 3) {
        case 0: offer_state_[1] = 1; break;
        case 1: offer_type_[0][0] = 2; break;
      }
      redraw();
      break;
    }

    case -2:
    case 4:
      release_modal();
      break;

    case 11:
      add_offer(0, 2, 0);
      break;

    case 10:
      add_offer(0, 3, 0);
      break;

    case 9:
      add_offer(0, 4, 0);
      break;

    case 6: {
      int requested_energy = 0;
      for (int i = 0; i < offer_count_[0]; ++i) {
        if (offer_type_[0][i] == 1) requested_energy = offer_value_[0][i];
      }
      if (X_pop_ask_number("DIPLOENERGY", requested_energy, pop_wait) != 0)
        return;

      if (ParseNumTable[0] > PlayersData[faction_id_].energy_reserves)
        ParseNumTable[0] = PlayersData[faction_id_].energy_reserves;
      add_offer(0, 1, ParseNumTable[0]);
      break;
    }

    case 5: {
      popup.start(PopupStartCaption, "DIPLOTECH", -1, 0, 0x40);
      int line_count = 0;

      for (int tech = 0; tech < 0x59; ++tech) {
        if (!has_tech(tech, faction_id_)) continue;

        if (diplo_tech_offered(offer_type_[0], offer_value_[0], offer_count_[0],
                               tech))
          continue;

        const int counterpart_lacks_tech = !has_tech(tech, faction_id_with_);
        StringTemp[0] = 0;
        if (!counterpart_lacks_tech) strcat(StringTemp, "*");
        say_tech(StringTemp, tech, 1);
        if (Language == 0 && !counterpart_lacks_tech)
          strcat(StringTemp, " (already has)");
        popup.item(StringTemp, tech);
        ++line_count;
      }

      for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if (faction == faction_id_ || faction == faction_id_with_) continue;
        if (!is_alive(static_cast<uint32_t>(faction))) continue;
        if (has_treaty(faction_id_, faction, 8) == 0)
          continue;
        if (has_treaty(faction_id_with_, faction, 8) != 0)
          continue;

        const int tech_id = faction + 0x59;
        if (diplo_tech_offered(offer_type_[0], offer_value_[0], offer_count_[0],
                               tech_id))
          continue;

        StringTemp[0] = 0;
        say_tech(StringTemp, tech_id, 1);
        popup.item(StringTemp, tech_id);
        ++line_count;
      }

      if (!diplo_tech_offered(offer_type_[0], offer_value_[0], offer_count_[0],
                              0x270F)) {
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x132));
        popup.item(StringTemp, 0x270F);
        ++line_count;
      }

      if (line_count == 0) return;
      const int chosen = popup.exec(pop_wait);
      if (chosen < 0) return;
      add_offer(0, 0, chosen);
      break;
    }

    case 1:
      ConsoleGlobal.chat(0);
      break;

    case 0:
      offer_count_[0] = 0;
      message_data(0x1508, faction_id_with_, 0, 0, 0, 0);
      redraw();
      break;

    case 12: {
      parse_says(0, get_title(faction_id_with_), -1, -1);
      parse_says(1, get_name(faction_id_with_), -1, -1);
      parse_says(2, get_noun(faction_id_with_), -1, -1);

      if (has_treaty(faction_id_, faction_id_with_, 1) == 0) {
        X_pop("DONTHAVEPACT", 0);
        break;
      }
      if (X_pop("REALLYENDPACT", pop_wait) == 0) break;
      if (diplo_lock(0x32) == 0) {
        net_pact_ends(faction_id_, faction_id_with_, 1);
        diplo_unlock();
      }
      DiploOffersDirty = 1;
      break;
    }


    case 7: {
      for (int tech = 0; tech < 0x59; ++tech) {
        if (!has_tech(tech, faction_id_)) continue;

        if (diplo_tech_offered(offer_type_[0], offer_value_[0], offer_count_[0], tech)
            || has_tech(tech, faction_id_with_))
          continue;
        add_offer(0, 0, tech);
      }

      for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if (faction == faction_id_ || faction == faction_id_with_) continue;
        if (!is_alive(static_cast<uint32_t>(faction))) continue;
        if (has_treaty(faction_id_, faction, 8) == 0)
          continue;
        if (has_treaty(faction_id_with_, faction, 8) != 0) continue;

        const int tech_id = faction + 0x59;
        if (!diplo_tech_offered(offer_type_[0], offer_value_[0], offer_count_[0],
                                tech_id))
          add_offer(0, 0, tech_id);
      }
      add_offer(0, 0, 0x270F);
      break;
    }

    case 8: {
      popup.start(PopupStartCaption, "DIPLOVENDETTA", -1, 0, 0x40);
      int line_count = 0;

      for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if (faction == faction_id_ || faction == faction_id_with_) continue;
        if (has_treaty(faction_id_, faction, 8) == 0
            && has_treaty(faction_id_with_, faction, 8) == 0
            && !is_human(faction))
          continue;

        ++line_count;
        StringTemp[0] = 0;
        parse_says(0, get_title(faction), -1, -1);
        parse_says(1, get_name(faction), -1, -1);
        parse_says(2, get_noun(faction), -1, -1);
        parse_it(ScriptPopupFile, "ATTACKSTRING");
        popup.item(StringTemp, faction);
      }

      if (line_count > 0) {
        const int chosen = popup.exec(pop_wait);
        if (chosen >= 0) add_offer(0, 5, chosen);
      } else {
        NetMsgPrimary.pop("DIPLONOVENDETTA", 0x1388, 0, 0);
      }
      break;
    }

    case -1:
      break;
    default:
      break;
  }
}

// ORIGINAL: 0x00442510 ?on_left_click@DiploWin@@QAEXHH@Z 0x00442510-0x00442558 FILE BYTE_EXACT
// symbol    ?on_left_click@DiploWin@@UAEXHH@Z
// LEVER: homed from units/00442510.cpp once kill_offer (0x004413F0) landed in
// this TU - the call was the only thing blocking it. The declared `spot_`
// member at 0xED0 and the same vtable_slot dispatch done_deal uses replaced
// the artifact's cast of `this` and its VCall shim.
// size      72 bytes
// prototype void (__thiscall ?on_left_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int xCoord, int yCoord)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004413F0 0x005FAB00
// indirect  0x0044254D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00442510
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00442510/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@DiploWin@@QAEXHH@Z  at 0x00442510  (72 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DiploWin::on_left_click(int a1, int a2) {
    // `check`'s two int* out-params reuse the parameter slots, so the same
    // names come back carrying the spot position and the button kind.
    if (spot_.check(a1, a2, &a1, &a2) >= 0 && a2 == 0) {
        kill_offer(0, a1);
        redraw();
    }
}

/*
Purpose: Clear the hot-seat chat log: walk all 64 0x30-byte records (eight
         groups of eight), whose first head is at 0x00737CE0, and for each
         live one walk its entry chain - freeing
         every node through the owning object's vtable-adjusted release
         slot and each node's own scalar-deleting slot - then clear the
         record's head, count and trailing words.

// ORIGINAL: 0x00440F80 ?clear_hotseat_chat@@YAXXZ 0x00440F80-0x00441033 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["diplowin.cpp owns the genuine DiplomacyTranscriptLists[8][8] StringList array whose first head is 0x00737CE0 (+8); game.cpp is its direct caller.", "StringStruct::remove_all performs the same count-reloaded on_remove/payload-delete/entry-delete traversal. For a nonempty head it also resets head/count/position; the source body then preserves the original unconditional position and allocator clears while leaving current_ untouched."], "reason": "Move this global cleanup beside its actual 64-list owner and use StringList::remove_all rather than a hand-indexed vtable thunk; keep the empty-head conditional and all per-record resets."}
// TRIED: member-fn-pointer+union thunk for the two vtable-adjusted "release" calls; raw pointer cast over fixed globals for record base/bound (address itself does read-modify-write via `rec += 0xc`). First divergence is #11 lea vs je.
// size      179 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00440FBA 0x00440FCC 0x00440FEA
Status: Source body moved to the DiplomacyTranscriptLists owner;
        nonempty rows now use the real StringList::remove_all lifecycle.
        The old union-thunk spelling remains seed history and is
        superseded by the current source binding; its TRIED note stands.
*/

void __cdecl clear_hotseat_chat() {
    for (int row = 0; row < 8; ++row) {
        for (int column = 0; column < 8; ++column) {
            StringList &list = DiplomacyTranscriptLists[row][column];
            if (list.head_ != 0) {
                list.remove_all();
            }
            list.current_position_ = 0;
            list.allocator_ = 0;
        }
    }
}

/*
Purpose: Publish the (a1,a2) id pair into the 0x30-byte dialog entry slot
         the 0x00747BD8 list describes: order the pair, walk the entry
         chain to the requested index (caching node and index in the
         list's own state when the walk ran), read the node's payload and
         its predecessor's, then stage both into the 0x00737CD8 table slot
         and add it under the payload id through StringStruct::add.

// ORIGINAL: 0x00441040 sub_441040 0x00441040-0x004410F1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["DiploWin owns stringBox_ at +0xA388; StringBox::stringList_ is at +0x2B70, giving the exact original StringList receiver at DiploWindow+0xCEF8 = 0x00747BD0.", "The pinned words 0x747BD8/BC/E0/E4 therefore map to StringList head_/current_/entry_count_/current_position_; 0x737CD8 is the typed 8x8 destination list table."], "reason": "Move the global callback into the owner of both the observed source StringList member and destination transcript array; preserve cursor caching, pair-index arithmetic, target-field writes and add order."}
// symbol    ?sub_441040@@YAXHH@Z
// TRIED: void __cdecl(int a1, int a2), direct linked-list walk transcription; diverges at #29 mov vs cmp, not chased further
// size      177 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401100 0x00628A50
Status: Coverage transcription - unclaimed body; the walk now goes through
        the tree's own StringStruct::add and the general.h swap, whose
        measured bodies are the image's 0x00401100 and 0x00628A50.
*/
// The image reads the list head separately for each of these two values.
static inline char *transcript_current_text(StringList *list) {
  return list->head_ ? list->current_entry()->text : 0;
}

static inline void transcript_add(StringList *list, char *text, int id) {
  list->pending_text_ = text;
  list->field_20_ = 0;
  list->field_24_ = 0;
  list->add(id);
}

void __cdecl sub_441040(int a1, int a2) {
    StringList &source = DiploWindow.shared_string_box().strings();
    if (a1 > a2) {
        swap(&a1, &a2);
    }
    int loopBound = source.entry_count_;
    for (int i = loopBound - 3; i < loopBound; ++i) {
        if (i >= 0) {
            source.seek_pos(i);
            transcript_add(&DiplomacyTranscriptLists[a1][a2],
                           transcript_current_text(&source),
                           source.current_id());
        }
    }
}

/*
Purpose: Refill one hot-seat chat view: order the two record indices, clear
         the shared StringBox and its EditBox caption, then stage the three
         newest entries of the record's entry chain into the box - each
         entry's payload text added under its id.

// ORIGINAL: 0x00441100 sub_441100 0x00441100-0x004411EC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["diplowin.cpp owns the direct DiploWin::exec caller, the shared DiploWindow StringBox/EditBox and the typed DiplomacyTranscriptLists array.", "The original implementation clears both shared controls, normalizes the pair, then reads the last three entries from the selected list in its existing order."], "reason": "Move the complete externally named callback from the catch-all leaf TU into the owning diplomacy TU; retain normalization, clear order, loop bounds and typed list/control behavior."}
// symbol    ?sub_441100@@YAXHH@Z
// TRIED: signature had no params in the fallback head (IDA guessed 2 stack ints, confirmed by [ebp+8]/[ebp+0xc] reads before any push, and no meaningful eax at ret so void not int - both changed). Landed at MISMATCH (90% mnemonic similarity, 11 edits).
// size      236 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006151E0 0x00628A50 0x00629710 0x00629780
Status: The body now lives beside the real DiploWindow StringBox/EditBox
        and transcript-array owners. Its historical MISMATCH note stands;
        the former uncertainty about shared StringBox ownership is
        superseded by the current source binding.
*/
// The shared window is the typed DiploWindow object; its saved entries are
// held in the 8x8 StringList table at the observed 0x737CD8 storage region.
void __cdecl sub_441100(int faction_id, int faction_id_with) {
  if (faction_id > faction_id_with)
    swap(&faction_id, &faction_id_with);
  DiploWindow.shared_string_box().clear();
  DiploWindow.shared_edit_box().set_text(0);
  const int saved_count =
      DiplomacyTranscriptLists[faction_id][faction_id_with].entry_count_;
  for (int index = saved_count - 3; index < saved_count; ++index) {
    if (index < 0) continue;
    DiplomacyTranscriptLists[faction_id][faction_id_with].seek_pos(index);
    int id = DiplomacyTranscriptLists[faction_id][faction_id_with].current_id();
    char *text = transcript_current_text(&DiplomacyTranscriptLists[faction_id][faction_id_with]);
    DiploWindow.shared_string_box().add(text, id, 1);
  }
}
// RECOVERY_GENERATED: v1 vtable 0x0066B634 ? ??_7DiploWin@@6BWin@@@ 0x00444FC0
// RECOVERY_GENERATED: v1 vtable 0x0066B62C ? ??_7DiploWin@@6BBuffer@@@ 0x00444FC0
// RECOVERY_GENERATED: v1 vtable 0x0066B5E0 ? ??_7DiploWin@@6B@ 0x00444FC0
