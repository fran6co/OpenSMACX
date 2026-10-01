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
#pragma once
#include "maininterface.h"  // MainInterfaceGlobal and shared interface storage
#include "spot.h"
#include "font.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "subinterface.h"

 /*
  * CouncWin class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; CouncWin's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
// RECOVERY_BASE: v1 CouncWin SubInterface 0xA14 0x00428620
// Constructor installs this original interface table at the declared base.
// Its nineteen observed slots do not independently establish its end bound.
// RECOVERY_GENERATED: v1 vtable 0x0066AA94 ? ??_7CouncWin@@6B@ 0x00428620
class CouncWin : public GraphicWin, public SubInterface {
 public:
  // The seed at 0x00427150 is catalogued as `int CouncWin::hide()`, but
  // Win already owns virtual `void hide()`. These signatures differ only by
  // return type, so the alleged derived method cannot be expressed in C++;
  // retain that seed pending evidence for its identity/alias.
  // 0x00425DB0  ?draw_leader@CouncWin@@QAEXH@Z - public, __thiscall,
  // void(int), unrecovered. Declared so draw_all_leaders and on_mouse_leave,
  // which each reach it with a direct `call rel32`, can name it.
  void draw_leader(int factionID);
  void show(int visible);
  void close();
  void arrived(int faction_id);
  void draw_all_leaders();
  int make_resolution(int option);
  void set_vote(int faction_id, int vote);
  void on_left_click(int x, int y);
  int on_key_click(int key, int flags);
  void draw_info();
  void init(int proposal, int faction_id);
  void on_mouse_leave(int a1, int a2);
  void on_mouse_move(int x, int y);
  // 0x00428620, defined out of line at the foot of councwin.cpp: the image
  // has a real body there (231 bytes) and it is not inlined anywhere.
  CouncWin();
  // NO DECLARED DESTRUCTOR, and that is not an omission: the image's
  // ??1CouncWin (0x00428550) is the compiler's IMPLICIT one - the member
  // teardown chain (the ??_M walk over buttons_[6], the six Fonts in reverse,
  // ~Spot, the GraphicWin base) with no vtable-restore stores anywhere. A
  // user-written `~CouncWin() {}` emits two vptr stores the image does not
  // have (measured, 48/50); leaving the destructor to the compiler removes
  // them and the rest of the chain is identical. deleting_thunks.cpp still
  // reaches it with an explicit `CouncWin::~CouncWin()`, which an implicitly
  // declared destructor allows.
  void on_group_clicked(int a1, int a2);
  void on_button_passover(int a1, int a2);
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_down(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_toggled(int a1, int a2);
  void on_iface_group_clicked(int a1, int a2, int a3);

  // ---- slots CouncWin leaves at the interface's shared no-ops -------------
  // Its own table (0x66AA94) holds 0x00404260 - the three-byte `ret 8` every
  // window shares - at slots 1, 2 and 6, and 0x00404250 / 0x00404240 - the
  // two constant-return no-ops - at 14, 17 and 18. Slots 6, 17 and 18 are
  // These are concrete SubInterface slots; CouncWin names its shared-stub
  // slots here so the actual table has the measured targets.
  // VC6 folds the three bodies to the same bytes the shared COMDATs carry,
  // which is what the image's single address per shape is.
  void on_iface_scrolled(int, int) override;
  void on_iface_scrolling(int, int) override;
  void on_iface_left_up(int, int) override;
  int on_iface_button_pressed(int) override;
  int on_iface_dialog_back_draw(GraphicWin *) override;
  int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                    RECT *) override;

  // Slot 0 - 0x00426FA0, 180 bytes. The interface-mode entry point: mode
  // nonzero shows the council pane and mode zero tears it down.
  void on_status(int mode);
  // Slot13, 0x004270F0: dispatch the front window's button-click event.
  void on_iface_button_clicked(int button_id);

  // ---- slots of the FRONT vtable (0x66AAE8) that CouncWin implements ------
  // The image's catalogue names them on_redraw / on_button_clicked; Win's
  // slot-12 virtual now carries that same original name, so the override
  // keeps its position under the image's own spelling. Both bodies are
  // unrecovered,
  // so both are forwarders - but on_status reaches slot12 and on_iface_button_clicked
  // reaches slot 44 through a live `call dword ptr [eax+N]`, so the entries
  // have to be CouncWin's and not Win's shared no-ops for those dispatches to
  // mean what the image's mean.
  virtual void on_redraw();    // slot 12, 0x00426AA0 ?on_redraw@CouncWin@@QAEXXZ
  void on_button_clicked(int button_id);    // slot 44, 0x00424870 ?on_button_clicked@CouncWin@@QAEXH@Z
  // PAUGraphicWin@@PADH, per
  // ?on_button_text_draw@CouncWin@@QAEHPAUGraphicWin@@PADH@Z. 0x00426F90 is
  // `xor eax, eax; ret 0xc` and touches none of the three.
  int on_button_text_draw(::GraphicWin *, char *, int);

  // Storage the image proves is here: its own methods reach 0xA38.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 8 member(s) from the IDA database, 1 named; it starts a member at 0xA14, which is where src/ ends.

  // PROVED, and now a real type rather than eight opaque bytes:
  // The SubInterface that used to sit here as a member is now the SECOND
  // BASE, above. MSVC lays non-virtual bases in declaration order, so it is
  // at sizeof(GraphicWin) == 0xA14 either way and no offset moves.
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint32_t field_A2C_;  // 0xA2C
  uint32_t field_A30_;  // 0xA30
  uint32_t field_A34_;  // 0xA34

  // Storage the image proves is here: its own methods reach 0x4614.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 165 member(s) from the IDA database, 13 named; it starts a member at 0xA38, which is where src/ ends.

  uint32_t field_A38_;  // 0xA38
  uint32_t field_A3C_;  // 0xA3C
  uint32_t field_A40_;  // 0xA40
  uint32_t field_A44_;  // 0xA44
  Spot spot_;  // 0xA48
  uint32_t leader_label_kind_[8];  // 0xA54, values select panel badge/text cases
  int leader_tech_[8];             // 0xA74, technology ids for war badges
  uint32_t field_A94_;  // 0xA94
  uint32_t field_A98_;  // 0xA98
  uint32_t field_A9C_;  // 0xA9C
  uint32_t field_AA0_;  // 0xAA0
  uint32_t field_AA4_;  // 0xAA4
  uint32_t field_AA8_;  // 0xAA8
  uint32_t field_AAC_;  // 0xAAC
  uint32_t field_AB0_;  // 0xAB0
  Font font1_;  // 0xAB4
  Font font2_;  // 0xADC
  Font font3_;  // 0xB04
  Font font4_;  // 0xB2C
  Font font5_;  // 0xB54
  Font font6_;  // 0xB7C
  uint32_t field_BA4_;  // 0xBA4
  uint32_t field_BA8_;  // 0xBA8
  uint32_t field_BAC_;  // 0xBAC
  uint32_t field_BB0_;  // 0xBB0
  uint32_t field_BB4_;  // 0xBB4
  uint32_t field_BB8_;  // 0xBB8
  uint32_t field_BBC_;  // 0xBBC
  uint32_t field_BC0_;  // 0xBC0
  uint32_t field_BC4_;  // 0xBC4
  uint32_t field_BC8_;  // 0xBC8
  uint32_t field_BCC_;  // 0xBCC
  uint32_t field_BD0_;  // 0xBD0
  // The original selects paired hit-boxes by two layout variants and rank.
  RECT leader_rects_[20];  // 0xBD4
  Sprite *leader_backing_[8][2];  // 0xD14
  Sprite *leader_portraits_[8][2]; // 0xD54
  // AN ARRAY, not six separate members: ~CouncWin tears the six down through
  // ONE `??_M` vector-destructor-iterator call (`push 0x406880; push 6;
  // push 0xB4C; push eax; call 0x6456E4` at 0x0042856F), which only an array
  // member makes the compiler emit. 0xD94 + 6 * 0xB4C ends at 0x515C, the
  // IDB's last-member bound. Nothing in the compiled tree names the
  // individual buttons, so the remodel rewires no caller.
  FlatButton buttons_[6];  // 0xD94
};


// ?draw_leader@CouncWin@@QAEXH@Z at 0x00425DB0 - public, __thiscall,
// void(int) - 1705-byte owner method. draw_all_leaders and on_mouse_leave
// both reach it with direct calls; its implementation lives in councwin.cpp.
