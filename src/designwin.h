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
#include "time.h"
#include "spot.h"
#include "font.h"
#include "flatbutton.h"
#include "caviar.h"
#include "buffer.h"
#include "graphicwin.h"
#include "subinterface.h"

 /*
  * DesignWin class
  *
  * Derives from GraphicWin in the original by ordinary single inheritance -
  * its constructor calls GraphicWin's on an unadjusted `this` - so unlike the
  * virtually-derived windows this one is written as the original wrote it and
  * needs no ABI workaround.
  *
  * The constructor's own fields begin at 0xA14, immediately after the base.
  * "the fields between are not modelled" was true when this was written and is
  * not now - the hole sweeps declared every dword from 0xA1C to the trailing
  * fonts, and the VC6 probe behind tools/verify_member_offsets.py measures the
  * class this header compiles to at sizeof == 0x145B8. What is still true is
  * that nothing PINS that: there is no size assertion, and 0x14210 (the
  * highest offset the image's own methods reach) is a floor, not the end. The
  * methods recovered here touch no field, which is why they could be replaced
  * ahead of the mapping.
  */
// RECOVERY_BASE: v1 DesignWin SubInterface 0xA14 0x00434480
// Constructor installs this original interface table at the declared base.
// Its nineteen observed slots do not independently establish its end bound.
// RECOVERY_GENERATED: v1 vtable 0x0066B0F4 ? ??_7DesignWin@@6B@ 0x00434480
class DesignWin : public GraphicWin, public SubInterface {
 public:
  // homed from 0043e960.cpp
  void shut_that_badboy_down();

 public:
  // homed from 00434650.cpp
  void UNK3();

 public:
  // homed from 0043c130.cpp
  void construct_reactor();

 public:
  // homed from 0043c0f0.cpp
  void construct_armor(int a1);

 public:
  // homed from 0043c0a0.cpp
  void construct_weapon();

 public:
  // homed from 0043c070.cpp
  void construct_chassis();

 public:
  // homed from 0043d0f0.cpp
  void hide();

 public:
  // homed from 0043d0c0.cpp
  void show(int a1);

 public:
  // homed from 00439eb0.cpp
  void delete_abil(int a1);

 public:
  // homed from 00439670.cpp
  void bubble_me_elmo2(char *a1, int a2, int a3);

 public:
  // The shared body both select_special_N thunks tail into:
  // ?select_special@DesignWin@@QAEXH@Z at 0x0043AC70, still an original body,
  // so this is a declaration only, resolved at link time.
  void select_special(int a1);
  void select_special_2();
  void select_special_1();
  // 0x0043C1A0, homed into designwin.cpp. desktop_close closes the
  // process-wide design window through this member.
  void close();
  // 0x0043BF50. Rebuild the design preview after the component selection is
  // settled, temporarily applying the automatic-build weapon rule.
  void construct_vehicle();
  DesignWin();
  int exec(int faction_id, int prototype_id);
  void init();
  static void __cdecl timer_function(int tick);
  void setup_veh(int proto_id, int automatic);
  void on_right_click(int x, int y);
  void on_redraw();
  void select_name();
  // 0x0043ECB0 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~DesignWin();
  void UNK1();
  void on_iface_left_down(int, int);
  void on_mouse_leave(int a1, int a2);
  void on_iface_right_down(int, int);
  void on_iface_selected(int, int);
  void on_iface_left_double_click(int, int);
  void on_iface_right_double_click(int, int);
  void on_iface_button_toggled(int, int);
  void on_iface_group_clicked(int, int, int);
  // Palette hover uses spot2_; interface-mode hover uses spot1_.
  void on_mouse_move(int, int);

  // ---- the interface table, slot for slot against 0x66B0F4 ----------------
  // Slots 1, 2, 6, 17 and 18 hold the shared no-ops every window carries
  // (0x00404260 / 0x00404250 / 0x00404240); 6, 17 and 18 are
  // Shared-stub SubInterface slots are named here so DesignWin's actual table
  // contains the measured targets.
  void on_iface_scrolled(int, int) override;
  void on_iface_scrolling(int, int) override;
  void on_iface_left_up(int, int) override;
  int on_iface_dialog_back_draw(GraphicWin *) override;
  int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                    RECT *) override;
  void on_status(int mode);
  void on_iface_left_click(int, int);
  void on_iface_right_click(int, int);
  void on_iface_mouse_move(int, int);
  void on_iface_mouse_leave(int, int);
  // Slot13, 0x0043D430: dispatch the front window's button-click event.
  void on_iface_button_clicked(int button_id);
  // Slot 14, 0x0043D450 - DesignWin's own, not the shared no-op.
  int on_iface_button_pressed(int button_id);

  // Slot 44 of the front vtable (0x66B148) - the catalogue's
  // ?on_button_clicked@DesignWin@@QAEXH@Z, 0x00438460, 4481 bytes,
  // unrecovered. Both on_iface_button_clicked and on_iface_button_pressed dispatch to it
  // with a live `call dword ptr [eax + 0xb0]`, so the entry has to be
  // DesignWin's and not Win's shared no-op.
  void on_button_clicked(int button_id);

  // 0x004345D0 ?UNK2@DesignWin@@QAEHH@Z - true when VehPrototypes[proto_id]
  // holds exactly the chassis/weapon/armor/reactor/ability set the editor
  // currently has selected.
  int UNK2(int proto_id);

  // 0x00439960 ?on_left_click@DesignWin@@QAEXHH@Z - a click on the prototype
  // palette (spot2_) picks the component the spot type names.
  void on_left_click(int a1, int a2);

  // 0x00439A20 ?on_key_click@DesignWin@@QAEHHH@Z - keyboard scrolling of the
  // prototype stack and the shared slot-44 button keys.
  int on_key_click(int a1, int a2);

  // 0x0043D490 ?center_stack@DesignWin@@QAEXHH@Z - scroll the stack window so
  // proto_id sits five rows from the top.
  void center_stack(int proto_id, int redraw);

  // 0x0043D510 ?draw_stack@DesignWin@@QAEXXZ
  void draw_stack();

  // 0x00437BF0 ?design_it@DesignWin@@QAEHXZ - commit the editor's component
  // set as a prototype: reuse a matching slot, else take/confirm a slot and
  // rebuild it. Answers 1 when the selection landed.
  int design_it();

  // 0x0043DD50 ?draw_flash@DesignWin@@QAEXXZ - the vehicle preview panel.
  void draw_flash();

  // The palette pick handlers on_left_click dispatches to, 0x00439F20 /
  // 0x0043A300 / 0x0043A760 / 0x0043AA00.
  void select_chassis();
  void select_weapon();
  void select_armor();
  void select_reactor();

  // Existing DesignWin preview/ability implementations live in their own
  // recovered translation units; picker handlers need their typed interface.
  void check_abil();
  void suggest_abil(int ability);
  void draw_chassis(int redraw);
  void draw_weapon(int redraw);
  void draw_armor(int redraw);
  void draw_reactor(int redraw);
  void draw_special_1(int redraw);
  void draw_special_2(int redraw);
  void draw_vehicle(int redraw);
  void draw_info();

  // Storage the image proves is here: its own methods reach 0x14210.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 259 member(s) from the IDA database, 14 named; it starts a member at 0xA14, which is where src/ ends.

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
  uint32_t field_A38_;  // 0xA38
  uint32_t field_A3C_;  // 0xA3C
  uint32_t field_A40_;  // 0xA40
  uint32_t field_A44_;  // 0xA44
  uint32_t field_A48_;  // 0xA48
  // The info panel hands these four adjacent values to Buffer::set_clip as a
  // RECT (0x004346DB..0x004346F1).
  // RECOVERY_MEMBER: v2 DesignWin info_panel_rect_ tagRECT 0xA4C 0x004346A0
  RECT info_panel_rect_;  // 0xA4C
  uint32_t field_A5C_;  // 0xA5C
  uint32_t field_A60_;  // 0xA60
  uint32_t field_A64_;  // 0xA64
  uint32_t field_A68_;  // 0xA68
  // Original on_redraw copies this rectangle before shrinking its caption.
  // RECOVERY_MEMBER: v2 DesignWin caption_rect_ tagRECT 0xA6C 0x004379A0
  RECT caption_rect_;  // 0xA6C
  uint32_t field_A7C_;  // 0xA7C
  uint32_t field_A80_;  // 0xA80
  uint32_t field_A84_;  // 0xA84
  uint32_t field_A88_;  // 0xA88
  // RECOVERY_MEMBER: v2 DesignWin frame_rect_ tagRECT 0xA8C 0x004379A0
  RECT frame_rect_;  // 0xA8C
  // init assigns each rectangle as four consecutive coordinates; the panel
  // methods consume the same groups with set_clip/copy. The vehicle rectangle
  // is used by draw_vehicle; the next six are the component panel clips.
  // RECOVERY_MEMBER: v2 DesignWin vehicle_panel_rect_ tagRECT 0xA9C 0x00436090
  RECT vehicle_panel_rect_;  // 0xA9C
  // RECOVERY_MEMBER: v2 DesignWin chassis_panel_rect_ tagRECT 0xAAC 0x00434BC0
  RECT chassis_panel_rect_;  // 0xAAC
  // RECOVERY_MEMBER: v2 DesignWin weapon_panel_rect_ tagRECT 0xABC 0x00434F40
  RECT weapon_panel_rect_;  // 0xABC
  // RECOVERY_MEMBER: v2 DesignWin armor_panel_rect_ tagRECT 0xACC 0x00435400
  RECT armor_panel_rect_;  // 0xACC
  // RECOVERY_MEMBER: v2 DesignWin reactor_panel_rect_ tagRECT 0xADC 0x00435800
  RECT reactor_panel_rect_;  // 0xADC
  // RECOVERY_MEMBER: v2 DesignWin special_one_panel_rect_ tagRECT 0xAEC 0x00435B30
  RECT special_one_panel_rect_;  // 0xAEC
  // RECOVERY_MEMBER: v2 DesignWin special_two_panel_rect_ tagRECT 0xAFC 0x00435DE0
  RECT special_two_panel_rect_;  // 0xAFC
  // Each four-word group is passed by address as RECT in TutWin::tour7:
  // 0x71FDBC to iface_rect (0x4BB17D), and draw_stack consumes the same
  // rectangle as its clipped/tiled panel.
  RECT field_B0C_rect_;  // 0xB0C
  uint32_t field_B1C_;  // 0xB1C
  uint32_t field_B20_;  // 0xB20
  uint32_t field_B24_;  // 0xB24
  uint32_t field_B28_;  // 0xB28
  // Passed as RECT* to des_rect at 0x4BB0E1 (original address 0x71FDDC).
  // RECOVERY_MEMBER: v2 DesignWin field_B2C_rect_ tagRECT 0xB2C 0x004BAF20
  RECT field_B2C_rect_;  // 0xB2C
  uint32_t field_B3C_;  // 0xB3C
  uint32_t field_B40_;  // 0xB40
  uint32_t field_B44_;  // 0xB44
  uint32_t field_B48_;  // 0xB48
  // Passed as RECT* to do_des at 0x4BB343 (original address 0x71FDFC).
  // RECOVERY_MEMBER: v2 DesignWin field_B4C_rect_ tagRECT 0xB4C 0x004BAF20
  RECT field_B4C_rect_;  // 0xB4C
  // init passes each four-word group as a RECT to its button setup;
  // the groups retain their original object offsets.
  // RECOVERY_MEMBER: v2 DesignWin field_B5C_rect_ tagRECT 0xB5C 0x0043C270
  RECT field_B5C_rect_;  // 0xB5C
  // RECOVERY_MEMBER: v2 DesignWin field_B6C_rect_ tagRECT 0xB6C 0x0043C270
  RECT field_B6C_rect_;  // 0xB6C
  // RECOVERY_MEMBER: v2 DesignWin field_B7C_rect_ tagRECT 0xB7C 0x0043C270
  RECT field_B7C_rect_;  // 0xB7C
  // The lower component-group border is the union of these three panels.
  // RECOVERY_MEMBER: v2 DesignWin lower_panel_rect_ tagRECT 0xB8C 0x004379A0
  RECT lower_panel_rect_;  // 0xB8C
  // RECOVERY_MEMBER: v2 DesignWin lower_panel_extra_rect_ tagRECT 0xB9C 0x004379A0
  RECT lower_panel_extra_rect_;  // 0xB9C
  // Passed as RECT* to do_des at 0x4BB315 and 0x4BB32C, respectively.
  // RECOVERY_MEMBER: v2 DesignWin field_BAC_rect_ tagRECT 0xBAC 0x004BAF20
  RECT field_BAC_rect_;  // 0xBAC
  // RECOVERY_MEMBER: v2 DesignWin field_BBC_rect_ tagRECT 0xBBC 0x004BAF20
  RECT field_BBC_rect_;  // 0xBBC
  // RECOVERY_MEMBER: v2 DesignWin lower_panel_end_rect_ tagRECT 0xBCC 0x004379A0
  RECT lower_panel_end_rect_;  // 0xBCC
  uint32_t field_BDC_;  // 0xBDC
  uint32_t field_BE0_;  // 0xBE0
  uint32_t field_BE4_;  // 0xBE4
  uint32_t field_BE8_;  // 0xBE8
  uint32_t field_BEC_;  // 0xBEC
  uint32_t field_BF0_;  // 0xBF0
  uint32_t field_BF4_;  // 0xBF4
  uint32_t field_BF8_;  // 0xBF8
  uint32_t field_BFC_;  // 0xBFC
  uint32_t field_C00_;  // 0xC00
  uint32_t field_C04_;  // 0xC04
  uint32_t field_C08_;  // 0xC08
  uint32_t field_C0C_;  // 0xC0C
  uint32_t field_C10_;  // 0xC10
  uint32_t field_C14_;  // 0xC14
  uint32_t field_C18_;  // 0xC18
  uint32_t field_C1C_;  // 0xC1C
  uint32_t field_C20_;  // 0xC20
  uint32_t field_C24_;  // 0xC24
  uint32_t field_C28_;  // 0xC28
  uint32_t field_C2C_;  // 0xC2C
  uint32_t field_C30_;  // 0xC30
  uint32_t field_C34_;  // 0xC34
  uint32_t field_C38_;  // 0xC38
  uint32_t field_C3C_;  // 0xC3C
  uint32_t field_C40_;  // 0xC40
  uint32_t field_C44_;  // 0xC44
  uint32_t field_C48_;  // 0xC48
  uint32_t field_C4C_;  // 0xC4C
  uint32_t field_C50_;  // 0xC50
  uint32_t field_C54_;  // 0xC54
  uint32_t field_C58_;  // 0xC58
  uint32_t field_C5C_;  // 0xC5C
  uint32_t field_C60_;  // 0xC60
  uint32_t field_C64_;  // 0xC64
  uint32_t field_C68_;  // 0xC68
  uint32_t field_C6C_;  // 0xC6C
  uint32_t field_C70_;  // 0xC70
  uint32_t field_C74_;  // 0xC74
  uint32_t field_C78_;  // 0xC78
  uint32_t field_C7C_;  // 0xC7C
  uint32_t field_C80_;  // 0xC80
  uint32_t field_C84_;  // 0xC84
  uint32_t field_C88_;  // 0xC88
  uint32_t field_C8C_;  // 0xC8C
  uint32_t field_C90_;  // 0xC90
  uint32_t field_C94_;  // 0xC94
  uint32_t field_C98_;  // 0xC98
  uint32_t field_C9C_;  // 0xC9C
  uint32_t field_CA0_;  // 0xCA0
  uint32_t field_CA4_;  // 0xCA4
  uint32_t field_CA8_;  // 0xCA8
  uint32_t field_CAC_;  // 0xCAC
  uint32_t field_CB0_;  // 0xCB0
  uint32_t field_CB4_;  // 0xCB4
  uint32_t field_CB8_;  // 0xCB8
  // The full rectangle at DesignWin+0xCBC is passed to iface_rect at
  // 0x4BB1CB (original address 0x71FF6C); draw_stack reads its right edge.
  // RECOVERY_MEMBER: v2 DesignWin field_CBC_rect_ tagRECT 0xCBC 0x004BAF20
  RECT field_CBC_rect_;  // 0xCBC
  uint32_t field_CCC_;  // 0xCCC
  uint32_t field_CD0_;  // 0xCD0
  uint32_t field_CD4_;  // 0xCD4
  uint32_t field_CD8_;  // 0xCD8
  uint32_t field_CDC_;  // 0xCDC
  uint32_t field_CE0_;  // 0xCE0
  uint32_t field_CE4_;  // 0xCE4
  uint32_t field_CE8_;  // 0xCE8
  FlatButton flatButtons_[25];  // 0xCEC
  Spot spot1_;  // 0x12758
  Spot spot2_;  // 0x12764
  // The angle the rotating vehicle preview is drawn at, in radians, and NOT a
  // scale: ?draw_vehicle@DesignWin@@QAEXH@Z hands it to caviar_ as the first
  // (x) argument of Caviar::set_scene_rotation at 0x0043629C, then advances it
  // in place - `fld dword ptr [esi+0x12770]; fadd dword ptr [0x0066B420];
  // fstp dword ptr [esi+0x12770]` at 0x004362A1, and 0x0066B420 holds
  // 0x3D4CCCCD == 0.05f - so each redraw turns the model a further 0.05 rad.
  // The scaling in that same block is the immediate 0x3E0F5C29 == 0.14f passed
  // to set_scaling, not this member. ?close@DesignWin@@QAEXXZ zeroes it
  // (`mov dword ptr [esi+0x12770], ebx` at 0x0043C1D6 with ebx just xor'd),
  // which is 0.0f, so nothing here reads it as an integer.
  //
  // Spelled `uint32_t` until the x87 load above was measured; those four
  // instructions are every reference to this offset in the image. float and
  // uint32_t are both 4/4 to VC6, so sizeof(DesignWin) stays 0x145B8.
  float scene_rotation_;  // 0x12770
  Time time_;  // 0x12774
  Caviar caviar_;  // 0x1279C
  uint32_t field_13B6C_;  // 0x13B6C
  uint32_t field_13B70_;  // 0x13B70
  uint32_t field_13B74_;  // 0x13B74
  uint32_t field_13B78_;  // 0x13B78
  uint32_t field_13B7C_;  // 0x13B7C
  uint32_t field_13B80_;  // 0x13B80
  uint32_t field_13B84_;  // 0x13B84
  uint32_t field_13B88_;  // 0x13B88
  uint32_t field_13B8C_;  // 0x13B8C
  uint32_t field_13B90_;  // 0x13B90
  uint32_t field_13B94_;  // 0x13B94
  uint32_t field_13B98_;  // 0x13B98
  uint32_t field_13B9C_;  // 0x13B9C
  uint32_t field_13BA0_;  // 0x13BA0
  uint32_t field_13BA4_;  // 0x13BA4
  uint32_t field_13BA8_;  // 0x13BA8
  uint32_t field_13BAC_;  // 0x13BAC
  uint32_t field_13BB0_;  // 0x13BB0
  uint32_t field_13BB4_;  // 0x13BB4
  uint32_t field_13BB8_;  // 0x13BB8
  uint32_t field_13BBC_;  // 0x13BBC
  uint32_t field_13BC0_;  // 0x13BC0
  uint32_t field_13BC4_;  // 0x13BC4
  uint32_t field_13BC8_;  // 0x13BC8
  uint32_t field_13BCC_;  // 0x13BCC
  uint32_t field_13BD0_;  // 0x13BD0
  uint32_t field_13BD4_;  // 0x13BD4
  uint32_t field_13BD8_;  // 0x13BD8
  uint32_t field_13BDC_;  // 0x13BDC
  uint32_t field_13BE0_;  // 0x13BE0
  uint32_t field_13BE4_;  // 0x13BE4
  uint32_t field_13BE8_;  // 0x13BE8
  uint32_t field_13BEC_;  // 0x13BEC
  uint32_t field_13BF0_;  // 0x13BF0
  uint32_t field_13BF4_;  // 0x13BF4
  uint32_t field_13BF8_;  // 0x13BF8
  uint32_t field_13BFC_;  // 0x13BFC
  uint32_t field_13C00_;  // 0x13C00
  uint32_t field_13C04_;  // 0x13C04
  uint32_t field_13C08_;  // 0x13C08
  uint32_t field_13C0C_;  // 0x13C0C
  uint32_t field_13C10_;  // 0x13C10
  uint32_t field_13C14_;  // 0x13C14
  uint32_t field_13C18_;  // 0x13C18
  uint32_t field_13C1C_;  // 0x13C1C
  uint32_t field_13C20_;  // 0x13C20
  uint32_t field_13C24_;  // 0x13C24
  uint32_t field_13C28_;  // 0x13C28
  uint32_t field_13C2C_;  // 0x13C2C
  uint32_t field_13C30_;  // 0x13C30
  uint32_t field_13C34_;  // 0x13C34
  uint32_t field_13C38_;  // 0x13C38
  uint32_t field_13C3C_;  // 0x13C3C
  uint32_t field_13C40_;  // 0x13C40
  uint32_t field_13C44_;  // 0x13C44
  uint32_t field_13C48_;  // 0x13C48
  uint32_t field_13C4C_;  // 0x13C4C
  uint32_t field_13C50_;  // 0x13C50
  uint32_t field_13C54_;  // 0x13C54
  uint32_t field_13C58_;  // 0x13C58
  uint32_t field_13C5C_;  // 0x13C5C
  uint32_t field_13C60_;  // 0x13C60
  uint32_t field_13C64_;  // 0x13C64
  uint32_t field_13C68_;  // 0x13C68
  Buffer buffer_;  // 0x13C6C
  uint32_t factionID_;  // 0x141F4
  uint32_t protoID_;  // 0x141F8
  uint32_t protoChassisType_;  // 0x141FC
  uint32_t protoWeaponType_;  // 0x14200
  uint32_t protoArmorType_;  // 0x14204
  uint32_t protoAbilityFlags_;  // 0x14208
  uint32_t protoReactorType_;  // 0x1420C

  // Storage the image proves is here: its own methods reach 0x145A4.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 28 member(s) from the IDA database, 6 named; it starts a member at 0x14210, which is where src/ ends.

  uint32_t field_14210_;  // 0x14210
  uint32_t mineralCostFactor_;  // 0x14214
  uint32_t protoAbilityID1_;  // 0x14218
  uint32_t protoAbilityID2_;  // 0x1421C
  // setup_veh copies a prototype's 32-byte name here; select_name bounds
  // its copy to 32 and terminates byte 0x1423F. close clears the first byte.
  char prototype_name_[32];  // 0x14220
  uint32_t field_14240_;  // 0x14240
  uint32_t field_14244_;  // 0x14244
  uint32_t field_14248_;  // 0x14248
  uint32_t field_1424C_;  // 0x1424C
  uint32_t field_14250_;  // 0x14250
  uint32_t field_14254_;  // 0x14254
  uint32_t field_14258_;  // 0x14258
  uint32_t field_1425C_;  // 0x1425C
  uint32_t field_14260_;  // 0x14260
  uint32_t field_14264_;  // 0x14264
  uint32_t field_14268_;  // 0x14268
  uint32_t field_1426C_;  // 0x1426C
  // exec loads sixteen localized preset names into 45-byte rows here.
  char preset_names_[16][45];  // 0x14270
  Font font1_;  // 0x14540
  Font font2_;  // 0x14568
  Font font3_;  // 0x14590, the IDB's third font; the class grows by 0x14
};


// DesignWin::select_special is not recovered, so the body at the end of
// designwin.cpp forwards through this seam. Both byte-exact select_special_N
// thunks tail into it, so the definition stands on its own account now that
// the class is no longer `__declspec(dllexport)`. The `H` in
// ?select_special@DesignWin@@QAEXH@Z is the one int both select_special_N
// thunks pass.
