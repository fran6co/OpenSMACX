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
#include "flatbutton.h"
#include "listbox.h"
#include "subinterface.h"

struct ReportSortEntry {
  int32_t base_id;
  int32_t value;
  int32_t sorted_base_id;
  int32_t sorted_value;
};
static_assert(sizeof(ReportSortEntry) == 0x10,
              "ReportIf merge-sort row must match the original 16-byte stride");

 /*
  * ReportIf class
  *
  * The constructor's own disassembly (0x004AD170) and the destructor's
  * (0x004ACDA0) between them place every sub-object below at an exact,
  * cross-checked offset: the constructor placement-news each one in image
  * order, the destructor tears the same run down in reverse, and the two
  * agree on every boundary. The object's own last construction reaches
  * 0x20304 (a Sprite[4] ending there); nothing past it is established, so
  * there is no static_assert on sizeof(ReportIf).
  */
class ReportIf : public SubInterface {
 public:
  // homed from 004a3d20.cpp
  void reset();

 public:
  void done();
  ReportIf();
  ~ReportIf();
  void close_labs();
  void close_ops();
  void close_energy();
  void close_intel();
  void close_security();
  void close_projects();
  void on_iface_right_click(int, int);
  void on_iface_left_double_click(int, int);
  void on_iface_right_double_click(int, int);
  void on_iface_left_down(int, int);
  void on_iface_right_down(int, int);
  void on_iface_mouse_move(int, int);
  void on_iface_mouse_leave(int, int);
  void on_iface_button_toggled(int, int);
  void merge_sort(int first, int last);
  void close_score();
  // PAUGraphicWin@@HHPAURECT@@, per the catalogued mangled name.
  int on_iface_dialog_item_back_draw(::GraphicWin *, int, int, RECT *);
  // 0x004A4060  ?bl_anim@ReportIf@@QAEXXZ - public, __thiscall, void(void),
  // unrecovered. The bl_daemon thunk in leaf_recoveries.cpp calls it.
  void bl_anim();
  // 0x004A3950  ?report@ReportIf@@QAEXHHH@Z - public, __thiscall,
  // void(int, int, int), unrecovered. on_iface_left_click and ReportWin's
  // on_key_click reach it on the singleton below; the retained unit's own
  // prototype names the second argument factionID (it feeds has_tech and the
  // per-faction spy lists), stores the faction id at +0x1E228 and the third
  // argument at +0x1E22C.
  void report(int report_id, int faction_id, int selection);
  int report_faction_id() const { return report_faction_id_; }
  // Drawing entry points selected by report() and ReportWin::on_redraw.
  void draw_security();
  void draw_score();
  void draw_intel();
  // homed from 004a5f40.cpp
  void on_iface_left_click(int x_coord, int y_coord);
  // homed from 004ab550.cpp
  void close_sat();

 public:
  // 0x8 - the ACTIVE REPORT CATEGORY of the singleton below, not private
  // state: ReportWin's on_redraw seed switches its 0..8 range to pick the
  // panel it redraws, and ReportWin::on_key_click and
  // ReportIf::on_iface_left_click both write 2..8 into it (0x00885F40 -
  // 0x00885F38 == 8, and the retained report unit re-reads the same dword as
  // `self + 8`), so it is spelled public for those cross-class sites like
  // the rest of the layout block.
  uint32_t field_8_;  // 0x8

 private:
  // ??0ReportIf@@QAE@XZ installs the SubInterface vftable 0x0066A6E4 into
  // [this + 0] at 0x004AD1A7, before it constructs anything else - the same
  // table seven window classes store at their own +0xA14. The IDB agrees
  // (`ReportIf,0x0,subIFace,8`), which is why src/reportif.cpp already has to
  // reach the sub-object with `reinterpret_cast<SubInterface *>(this)`.
  // SubInterface is the BASE, above, so its 8 bytes are at 0.
  // Shared drawing cursor/extent written by the ReportIf panel renderers.
  // RECOVERY_MEMBER: v1 ReportIf field_C_ 0xC 0x004A7027
  uint32_t field_C_;  // 0xC
  // RECOVERY_MEMBER: v1 ReportIf field_10_ 0x10 0x004A702A
  uint32_t field_10_;  // 0x10
  // RECOVERY_MEMBER: v1 ReportIf field_14_ 0x14 0x004A7039
  uint32_t field_14_;  // 0x14
  // RECOVERY_MEMBER: v1 ReportIf field_18_ 0x18 0x004A7053
  uint32_t field_18_;  // 0x18
  uint32_t field_1C_;  // 0x1C
  // init copies four coordinates from the shared report panel definition;
  // the adjacent draws pass the complete 16-byte regions to Buffer::set_clip.
  // RECOVERY_MEMBER: v2 ReportIf field_20_ tagRECT 0x20 0x004A35F6
  RECT field_20_;  // 0x20
  // RECOVERY_MEMBER: v2 ReportIf field_30_ tagRECT 0x30 0x004A361B
  RECT field_30_;  // 0x30
  // RECOVERY_MEMBER: v2 ReportIf field_40_ tagRECT 0x40 0x004A3640
  RECT field_40_;  // 0x40
  // RECOVERY_MEMBER: v2 ReportIf field_50_ tagRECT 0x50 0x004A3667
  RECT field_50_;  // 0x50
  uint32_t field_60_;  // 0x60
  uint32_t field_64_;  // 0x64
  uint32_t field_68_;  // 0x68
  uint32_t field_6C_;  // 0x6C
  uint32_t field_70_;  // 0x70
  uint32_t field_74_;  // 0x74
  uint32_t field_78_;  // 0x78
  uint32_t field_7C_;  // 0x7C

  // Everything below is placement-new'd EXPLICITLY from the constructor
  // body (0x004AD170), in image order, and torn down in the mirrored order
  // by the destructor (0x004ACDA0) - the two cross-check every boundary
  // here. Raw storage, not the real sub-object type: C++'s implicit member
  // construction would build these all before the constructor body ever
  // runs, which cannot reproduce the image's exact call order (SubInterface
  // itself is the empty-inline base, and nothing here is built through it).
  // MEASURED: real declared members (FlatButton[7], Sprite[0x15],
  // FlatButton[7]) built implicitly instead of placement-new - made this
  // WORSE, 26/132 against this baseline's 34/132, and grew the compiled
  // instruction count (189 -> 195). The base's `object[0] = vtable` store
  // must run explicitly in this body (SubInterface's own ctor is not
  // parametrised per host class, and that header is out of this batch's
  // scope), and standard C++ always runs implicit member construction
  // BEFORE any body statement, so converting these three pushes that store
  // after all three members instead of interleaved with the first - which is
  // not what the image does. Reverted.
  FlatButton flatButtonsA_[7];  // 0x80, 7 * sizeof(FlatButton), vector ctor iterator
  Sprite spritesA_[0x15];  // 0x4F94, 0x15 * sizeof(Sprite), vector ctor iterator
  uint8_t gap_5330_[0x54];  // 0x5330
  FlatButton flatButtonsB_[7];  // 0x5384, 7 * sizeof(FlatButton), vector ctor iterator
  uint8_t gap_A298_[0x38];  // 0xA298
  // The two ListBox members below are constructed with a BOOL argument
  // (`ListBox(1)` in the disassembly's call at 0x00609DB0) that has no
  // declared overload yet - that recovery is out of this batch's scope, so
  // the constructor reaches it at the raw image address, same as any other
  // not-yet-promoted callee.
  uint8_t listBox1_[0xB54];  // 0xA2D0, sizeof(ListBox)
  uint8_t listBox2_[0xB54];  // 0xAE24, sizeof(ListBox)
  uint8_t buttonGroup1_[0x94];  // 0xB978, sizeof(ButtonGroup)
  uint8_t flatButton1_[0xB4C];  // 0xBA0C
  uint8_t flatButton2_[0xB4C];  // 0xC558
  uint8_t buttonGroup2_[0x94];  // 0xD0A4
  uint8_t flatButton3_[0xB4C];  // 0xD138
  uint8_t flatButton4_[0xB4C];  // 0xDC84
  uint8_t flatButton5_[0xB4C];  // 0xE7D0
  uint8_t flatButton6_[0xB4C];  // 0xF31C
  uint8_t flatButton7_[0xB4C];  // 0xFE68
  uint8_t flatButton8_[0xB4C];  // 0x109B4
  uint8_t flatButton9_[0xB4C];  // 0x11500
  uint8_t flatButton10_[0xB4C];  // 0x1204C
  uint8_t flatButton11_[0xB4C];  // 0x12B98
  uint8_t buttonGroup3_[0x94];  // 0x136E4
  FlatButton flatButtonsC_[7];  // 0x13778, 7 * sizeof(FlatButton), vector ctor iterator
  uint8_t buttonGroup4_[0x94];  // 0x1868C
  FlatButton flatButtonsD_[5];  // 0x18720, 5 * sizeof(FlatButton), vector ctor iterator
  uint8_t flatButton12_[0xB4C];  // 0x1BF9C
  uint8_t flatButton13_[0xB4C];  // 0x1CAE8
  uint8_t flatButton14_[0xB4C];  // 0x1D634
  // init fills this run with find_font results; draw_score passes its relevant
  // entries directly to Buffer::set_font. Names stay neutral because their
  // per-panel roles are not established across all renderers.
  // RECOVERY_MEMBER: v3 ReportIf field_1E180_ Font* 0x1E180 0x004A3807
  Font *field_1E180_;  // 0x1E180
  // RECOVERY_MEMBER: v3 ReportIf field_1E184_ Font* 0x1E184 0x004A3807
  Font *field_1E184_;  // 0x1E184
  // RECOVERY_MEMBER: v3 ReportIf field_1E188_ Font* 0x1E188 0x004A3807
  Font *field_1E188_;  // 0x1E188
  // RECOVERY_MEMBER: v3 ReportIf field_1E18C_ Font* 0x1E18C 0x004A3807
  Font *field_1E18C_;  // 0x1E18C
  // RECOVERY_MEMBER: v3 ReportIf field_1E190_ Font* 0x1E190 0x004A3807
  Font *field_1E190_;  // 0x1E190
  // RECOVERY_MEMBER: v3 ReportIf field_1E194_ Font* 0x1E194 0x004A3807
  Font *field_1E194_;  // 0x1E194
  // RECOVERY_MEMBER: v3 ReportIf field_1E198_ Font* 0x1E198 0x004A3807
  Font *field_1E198_;  // 0x1E198
  // RECOVERY_MEMBER: v3 ReportIf field_1E19C_ Font* 0x1E19C 0x004A3807
  Font *field_1E19C_;  // 0x1E19C
  // RECOVERY_MEMBER: v3 ReportIf field_1E1A0_ Font* 0x1E1A0 0x004A3807
  Font *field_1E1A0_;  // 0x1E1A0
  // RECOVERY_MEMBER: v3 ReportIf field_1E1A4_ Font* 0x1E1A4 0x004A3807
  Font *field_1E1A4_;  // 0x1E1A4
  // RECOVERY_MEMBER: v3 ReportIf field_1E1A8_ Font* 0x1E1A8 0x004A3807
  Font *field_1E1A8_;  // 0x1E1A8
  // RECOVERY_MEMBER: v3 ReportIf field_1E1AC_ Font* 0x1E1AC 0x004A3807
  Font *field_1E1AC_;  // 0x1E1AC
  // RECOVERY_MEMBER: v3 ReportIf field_1E1B0_ Font* 0x1E1B0 0x004A3807
  Font *field_1E1B0_;  // 0x1E1B0
  // RECOVERY_MEMBER: v3 ReportIf field_1E1B4_ Font* 0x1E1B4 0x004A3807
  Font *field_1E1B4_;  // 0x1E1B4
  // RECOVERY_MEMBER: v3 ReportIf field_1E1B8_ Font* 0x1E1B8 0x004A3807
  Font *field_1E1B8_;  // 0x1E1B8
  uint8_t spot_[0xC];  // 0x1E1BC, sizeof(Spot)
  // draw_labs and draw_score both populate/pass this 16-byte rectangle to
  // Buffer::box_sprite. Keep the following bytes opaque; other fields in this
  // workspace have not been interpreted.
  // RECOVERY_MEMBER: v2 ReportIf draw_workspace_rect_ tagRECT 0x1E1C8 0x004A707F
  RECT draw_workspace_rect_;  // 0x1E1C8
  uint8_t gap_1E1D8_[0x50];  // 0x1E1D8
  // ReportIf::report stores its faction and selection arguments here; the
  // singleton's 0x008A4160 address is this + 0x1E228, not separate storage.
  // RECOVERY_MEMBER: v1 ReportIf report_faction_id_ 0x1E228 0x004A3950
  int32_t report_faction_id_;  // 0x1E228
  // RECOVERY_MEMBER: v1 ReportIf report_selection_ 0x1E22C 0x004A3950
  int32_t report_selection_;  // 0x1E22C
  // draw_score tests this dword to select its extended report heading.
  // RECOVERY_MEMBER: v1 ReportIf field_1E230_ 0x1E230 0x004ABE70
  uint32_t field_1E230_;  // 0x1E230
  uint8_t gap_1E234_[0xC];  // 0x1E234
  // ReportIf::init writes 0 and 4 to these adjacent dwords at +0x1E240
  // and +0x1E244. ReportWin::on_group_clicked writes its third argument
  // to the first when the clicked group id is 4. Their meanings remain
  // unresolved; retain neutral names until the shared state is understood.
  // RECOVERY_MEMBER: v1 ReportIf field_1E240_ 0x1E240 0x004A3494
  int32_t field_1E240_;  // 0x1E240
  // RECOVERY_MEMBER: v1 ReportIf field_1E244_ 0x1E244 0x004A3494
  int32_t field_1E244_;  // 0x1E244
  // RECOVERY_MEMBER: v1 ReportIf animation_state_ 0x1E248 0x004A4060
  int32_t animation_state_;  // 0x1E248; seed compares it with signed -1
  // RECOVERY_MEMBER: v1 ReportIf animation_x_ 0x1E24C 0x004A4060
  int32_t animation_x_;       // 0x1E24C
  // RECOVERY_MEMBER: v1 ReportIf animation_y_ 0x1E250 0x004A4060
  int32_t animation_y_;       // 0x1E250
  // The image's merge_sort addresses four dwords per row, 0x10 bytes apart,
  // from 0x1E254 through the Sprite run at 0x20254 (512 rows total).
  // RECOVERY_MEMBER: v1 ReportIf sort_entries_ 0x1E254 0x004ACC30
  ReportSortEntry sort_entries_[0x200];  // 0x1E254
  Sprite spritesB_[4];  // 0x20254, 4 * sizeof(Sprite), vector ctor iterator
};

// The constructor's own last store reaches 0x20254 + 0xB0 == 0x20304; there
// is no static_assert on sizeof(ReportIf) because nothing establishes where
// the object actually ends past that.

// The energy report's third list box is a PROCESS-WIDE object at a fixed
// address, not a subobject: close_energy closes it alongside the two this
// object owns. THE OBJECT, at 0x0087BE84 in the image - REAL STORAGE,
// defined in reportif.cpp. It was
// `ListBox *const ReportIfEnergyListBox = (ListBox *)0x0087BE84` naming
// terranx.exe data that is unmapped in a standalone build. The image
// constructs it inside ReportIf's own init (nothing at the ??__E layer
// touches 0x0087BE84), and ListBox's constructor here is the empty inline
// the tree models, so the object is the zero storage the image leaves until
// then. The `close()` receiver keeps its folded form, the displacement
// relocated.
extern ListBox ReportIfEnergyListBox;  // 0x0087BE84


// The process-wide ReportIf. THE OBJECT, at 0x00885F38 in the image - REAL
// STORAGE, defined in reportif.cpp: the original's dynamic initializer at
// 0x00496900 constructs it (`mov ecx, 0x885f38; call ??0ReportIf@@QAE@XZ`,
// followed by the atexit registration), exactly the WorldClimateWorldWin
// shape. ReportWin::on_key_click calls SubInterface::release_iface_mode and
// report() on it through `mov ecx, 0x885f38`, and the +8 category above is
// its field_8_ (0x00885F40).
extern ReportIf ReportIfGlobal;  // 0x00885F38


// ReportIf::bl_anim (0x004A4060) is now owned by reportif.cpp and uses the
// declared sprite run, animation state, WorldClimateWorldWin buffer and
// ReportWin timer storage. leaf_recoveries.cpp calls the typed member.
