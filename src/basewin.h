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
#include "time.h"
#include "sprite.h"
#include "prodpicker.h"
#include "mapwin.h"
#include "flatbutton.h"
#include "caviar.h"
#include "buffer.h"
#include "font.h"
#include "spot.h"
#include "scroll.h"

#include "original_seam.h"
#include "graphicwin.h"
#include "maininterface.h"  // MainInterfaceGlobal and shared interface storage
#include "subinterface.h"

 /*
  * BaseWin class
  *
 * GraphicWin is the primary window base at zero; SubInterface is its real
 * secondary base at +0xA14. Both use ordinary multiple inheritance, with no
 * registry wrapper or hand-built vtable. The pinned constructor calls the
 * real GraphicWin constructor on the complete receiver before member setup.
  *
 * DERIVED 2026-09-06 directly from SubInterface, as the +0xA14 subobject
 * requires. ??0BaseWin@@QAE@XZ writes the all-pure base table
  * 0x66A6E4 at +0xA14 (0x004084C1) and then overwrites it with BaseWin's own
  * nineteen-entry interface table 0x66A3B8 - the base-then-derived pair only
  * a polymorphic base produces - exactly as MainInterface, CouncWin,
  * BattleWin, DesignWin and DiploPop already do. The base is the SECOND base,
  * at sizeof(GraphicWin) == 0xA14, and the ProdPicker that follows is
  * untouched: 0xA14 + 8 == 0xA1C.
  *
  * THE PAYLOAD is the on_iface_* family. As an override of a SECONDARY base's
  * virtual, VC6 compiles each of them with the ENTRY as `this` - members read
  * base-relative, the secondary vftable holds the raw body address, no
  * adjustor thunk - which is the receiver the image's own bodies open with
  * (`cmp dword ptr [esi + 0x40140] ...` in on_iface_left_down, esi the entry)
  * and what every `self - 0xA14` pun in basewin.cpp was standing in for.
  * on_left_up and its siblings, which take the FRONT, are unaffected: their
  * `this` is still the GraphicWin.
  *
  * The observed target32 extent is 0x47838. ProdPicker begins at 0xA1C;
  * later fields include the panel rectangles and camera aggregates below.
  * Remaining opaque storage is not a recovered semantic model. The genuine
  * destructor follows the declared member/base lifetime order.
  */
// The 21 (dx, dy) steps of a base's own radius - the 5x5 square minus its
// four corners - walked in ring order. Read out of the shipped image at
// 0x0066A1A0 / 0x0066A1F4; the two tables sit
// back to back, which is how their 21-entry length is known. BaseWin::UNK1
// is a reverse lookup: given an index into these, find the matching index
// in map.h's RadiusOffsetX/RadiusOffsetY.
extern const int BaseRadiusOffsetX[21];  // 0x0066A1A0
extern const int BaseRadiusOffsetY[21];  // 0x0066A1F4

// RECOVERY_BASE: v1 BaseWin GraphicWin 0x0 0x00408490
// RECOVERY_BASE: v1 BaseWin SubInterface 0xA14 0x00408490
// Constructor installs this original interface table at the declared base.
// Its nineteen observed slots do not independently establish its end bound.
// RECOVERY_GENERATED: v1 vtable 0x0066A3B8 ? ??_7BaseWin@@6B@ 0x00408490
class BaseWin : public GraphicWin, public SubInterface {
 public:
  // homed from 004103b0.cpp
  void draw_garrison(int a1);
  // 0x00415DD0 selects the focused base for current-base computation.
  void focus();
  // 0x00415E60 also changes the focused base, then invalidates its computed id.
  void UNK3(int base_id);

 public:
  // homed from 00420a40.cpp
  int on_nc_hittest(int a1, int a2);
  // 0x0041A920. Centre the map on a base: Console::base_me asks it for the
  // base under the cursor's last position. Its body remains explicitly
  // unfinished in basewin.cpp.
  void zoom(int base_id, int wait_done);
  void soft_zoom(int base_id);  // 0x0041AE40, prepare a base without opening its window
  // Release zoom's optional wait without hiding or destroying the window.
  void cancel_zoom_wait() { zoom_wait_pending_ = 0; }
  int focused_base_id() const { return static_cast<int>(focusedBaseID_); }

 public:
  // homed from 0041e8b0.cpp
  void hide();

 public:
  // homed from 0041de60.cpp
  void on_iface_left_down(int a1, int a2);

 public:
  // 0x004090C0. Reverse radius lookup, see the tables above.
  int UNK1(int a1);
  // 0x00416540. The catalogue's `PAD` is `char *`; the scaffold spelled the
  // same parameter `int8 *`, a typedef only it declared.
  void bubble_me_elmo2(char *text, int x, int y);
  // homed from 0041b110.cpp
  void on_left_down(int a1, int a2);

 public:
  // homed from 0041afd0.cpp
  void on_left_up(int a1, int a2);

 public:
  // homed from 00411980.cpp
  void draw_energy_alloc(int a1);

 public:
  // 0x004165D0, a pending_bodies forwarder: the three click handlers
  // below funnel into it and reached it through a pointer.
  // `right` and `is_double` are named from the CALL SITES, not from a body:
  // on_left_click passes (.., 0, 0), on_right_click (.., 1, 0),
  // on_left_double_click (.., 0, 1) and on_right_double_click (.., 1, 1).
  // The first two arguments stay scaffold on purpose - every override in
  // the tree DISCARDS them (designwin, diplopop, datalink, battlewin,
  // socialwin all spell the body `(int, int)`), so nothing here says what
  // they carry.
  void click(int a1, int a2, int right, int is_double);
  // 0x004160F0, a pending_bodies forwarder.
  void iface_click(int a1, int a2, int right, int is_double);
  // Original 0x0040C850: select full or deferred panel publication.
  void draw_supported(int update_mode);
 public:
  // (code, pos), the Win32 WM_VSCROLL shape the image mirrors. Evidence
  // is in the bodies: BaseWin::on_scrolled does `switch (a1)` and then
  // stores a2 in field_40B10_; StringBox::on_scrolled stores a2 in
  // field_A1C_; BaseWin::on_iface_scrolled tests `a1 != 2` and copies a2.
  // The first is switched on, the second is kept - a code and a position.
  virtual void on_scrolled(int code, int position);
  virtual void on_scrolling(int code, int position);
  // The four stay scaffold - see the note on the body in basewin.cpp.
  void UNK2(int a1, int a2, int a3, int a4);

  // THE BASE SCREEN'S OWN PANELS AND HANDLERS, declared ahead of their
  // homing. Every one of these calls at least one sibling (draw_pop calls
  // draw_production, queue_click calls draw_queue and production,
  // on_redraw calls the whole draw_* row), so no batch can compile while
  // the declarations are missing - the homing tool declares only the body
  // it is moving, and reverts the whole batch on the first undeclared
  // name. Arity and return type are read off the image's own mangled
  // symbols, not off the bodies: `QAEH` against `QAEX` is the only
  // difference between several of these.
  void psych_row(int x, int y, int width, int sprite_width,
                 int talents, int drones, int super_drones);  // 0x00408720
  void draw_psych();                                    // 0x00408B20
  void draw_support();                                  // 0x00408EB0
  int farm_get(int a1, int a2);                         // 0x00409100
  void queue_click(int a1, int a2, int a3, int a4);     // 0x00409200
  void farm_click(int a1, int a2, int a3);              // 0x0040A1D0
  void pop_click(int a1, int a2, int a3, int a4);       // 0x0040A880
  int facilities_click(int a1, int a2, int a3, int a4); // 0x0040C070
  void draw_flash(int a1);                              // 0x0040D250
  void draw_production(int a1);                         // 0x0040E360
  void draw_farm(int a1);                               // 0x0040F0F0
  void draw_commerce(int a1);                           // 0x00410870
  void draw_nutrients(int a1);                          // 0x00411240
  void draw_expenses(int a1);                           // 0x00412AA0
  void draw_pop(int a1);                                // 0x00414B40
  void draw_misc(int a1);                               // 0x00415730
  void draw_info(int a1);                               // 0x00415B20
  void gov_on(int a1);                                  // 0x00415F70
  void gov_activate();                                  // 0x00415F00
  void recompute(int a1);                               // 0x00416D90
  void add_queue(int a1, int a2);                       // 0x00416DE0
  void delete_queue(int a1);                            // 0x00416FB0
  void production(int a1, int a2);                      // 0x00417040
  void hurry();                                         // 0x00418DF0
  int governor(int a1);                                 // 0x00419A90
  void prev();                                          // 0x0041A010
  void next();                                          // 0x0041A160
  void rename();                                        // 0x0041A2B0
  void exit();                                          // 0x0041A4C0
  void base_killed(int baseID);                         // 0x0041A620
  void check_base(int base_id);                         // 0x0041A7A0
  void check_loc(int x, int y, int radius);              // 0x0041A7E0
  void check_veh(int a1);                               // 0x0041A8B0
  // Primary vtable66A40C+44 selects original41B1F0. Win names this
  // two-coordinate feedback slot separately from its four-argument router.
  virtual void on_mouse_move(int xCoord, int yCoord);  // 0x0041B1F0
  void staple_the_bastards();                           // 0x0041B4F0
  void base_editor_fac();                               // 0x0041B790
  void base_editor_proj();                              // 0x0041BBC0
  void base_editor_copy(int a1);                        // 0x0041C240
  void base_editor();                                   // 0x0041C7A0
  void on_button_clicked(int a1);                       // 0x0041D500
  void on_iface_left_up(int a1, int a2);                // 0x0041DD10
  // The two interface-table slots that follow are declared in the slot block
  // below with the rest of the table: slot 8 0x0041DF50, slot 9 0x0041E3E0.
  void on_redraw();                                     // 0x0041E790
  void init();                                          // 0x0041E8D0
  void compute_selected_base();
  int displayed_production_id() const { return displayed_production_id_; }
  void update_drag_cursor(int spot_type, int spot_pos, int x, int y, int large);
  void show_panel_help(int x, int y, int label_offset, Win *window);

  // UNK2 calls the garrison interaction; on_scrolled repaints facilities.
  // garrison_click's arguments select a vehicle and screen coordinates, with
  // the final argument selecting the right-button path (original40B140).
  void garrison_click(int vehicle_id, int x, int y, int right);  // 0x0040B140
  void dismiss_garrison_selection();
  int on_key_click(int repeat, int key) override;  // 0x0041AF50
  void draw_facilities(int a1);                            // 0x0040FCC0
  // 0x0041E560  ?update_gov_buttons@BaseWin@@QAEXXZ - public, __thiscall,
  // void(). Declared ahead of its own homing because on_redraw reaches it, so
  // the two cannot land in one batch while the declaration is missing.
  void update_gov_buttons();
  // 0x00415080  ?draw_queue@BaseWin@@QAEXH@Z - public, __thiscall, void(int).
  // The queue panel of the base screen. Declared ahead of its own homing for
  // the same reason: queue_click, add_queue, on_iface_mouse_move and
  // on_redraw all call it, so its declaration has to exist first.
  void draw_queue(int a1);
  void show(int visible);
  BaseWin();
  // Original 0x00420A70: genuine member/base teardown is compiler-emitted.
  // Out of line so deleting-destructor callers retain an ordinary call.
  ~BaseWin();
  void close();
  // THE FOUR UNK METHODS ARE EVIDENCE-FREE, and that is a measured result
  // rather than an unfinished job (the same finding win.h records for its
  // five). Searched 2026-08-27, all of it empty:
  //   - the catalogue records callers 0 and call targets 0 for all four;
  //   - none of the four occupies a slot in any of BaseWin's three vtables
  //     (primary 0x0066A40C, SubInterface 0x0066A3B8, Buffer 0x0066A404 -
  //     read with tools/image_data.py, 80 dwords each), so no dispatch
  //     names them either;
  //   - docs/recovery/behaviour-member-names.csv has no BaseWin entry;
  //   - the shipped binary carries no UNK?@BaseWin string - the names are
  //     IDB placeholders, and there is no evidence to replace them with.
  //   UNK4, UNK6 and UNK7 are bare returns; UNK5 discards its int. A name
  //   invented to satisfy the census would be a false statement that
  //   outlives whoever wrote it; the house rule is that a body which cannot
  //   lose its scaffold name is a finding, not a style problem. This is the
  //   finding.
  void UNK4();
  void UNK6();
  void UNK7();
  void UNK5(int a1);
  // SAME REFUSAL, SAME EVIDENCE, for the two ints each of these carries: all
  // four bodies are bare returns that discard both arguments, and the
  // catalogue's own prototype line spells them `(BaseWin* this, int, int)` -
  // unnamed. The method names are real (they come out of the mangled
  // symbols); the parameter names are not, and the bare returns are the proof
  // that the image never reads them either.
  void on_button_toggled(int a1, int a2);
  void on_mouse_leave(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_click(int xCoord, int yCoord);
  void on_iface_right_click(int xCoord, int yCoord);
  void on_iface_left_double_click(int xCoord, int yCoord);
  void on_iface_right_double_click(int xCoord, int yCoord);
  void on_iface_scrolled(int code, int pos);

  // ---- THE INTERFACE TABLE, slot for slot against 0x66A3B8 ----------------
  // The nineteen slots above and below that carry an on_iface_* name are the
  // overrides: BaseWin implements them and its table holds its own bodies.
  // The rest are filled here.
  //
  // Slots BaseWin leaves at the interface's shared no-ops, the way
  // BattleWin's header spells the same situation. Slot 14 is 0x00404250
  // (`xor eax,eax; ret 4`) and slot 16 is 0x00406B20 (`ret 0xc`) - VC6 folds
  // both bodies to the shared COMDATs the other windows' tables carry. Slots
  // 17 and 18 are concrete SubInterface slots and use measured shared stubs;
  // slot 6 is its definition too, but BaseWin overrides it with
  // on_iface_left_up above, which is what its own table holds there.
  int on_iface_button_pressed(int) override; // slot 14 0x00404250
  void on_iface_group_clicked(int, int, int) override;  // slot 16 0x00406B20
  int on_iface_dialog_back_draw(GraphicWin *) override;
  int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                    RECT *) override;
  // Slot 15 - 0x0041E530, 33 bytes. A FORWARDING DISPATCH: `mov eax,[ecx -
  // 0xa14] / add ecx,-0xa14 / call [eax+0xb4]` is the FRONT vtable's slot 45
  // with the receiver stepped back to the GraphicWin, and 0x0041B4E0 - the
  // body that sits there - is the catalogue's on_button_toggled. So the
  // interface's button-toggled event IS the front's button-toggled event, and
  // the override spells it as the call it is. `vslot_45` is Win's own name for
  // that slot; it is what the override has to carry to reach offset 0xb4 (see
  // the note on vslot_13 below, and CouncWin's vslot_44 for the same finding).
  void on_iface_button_toggled(int a1, int a2);          // slot 15 0x0041E530
  // Slot 2 - 0x0041DCB0, 20 bytes: `mov eax,[ecx] / call [eax+4]` forwards
  // both arguments to this same table's slot 1, on_iface_scrolled. The
  // interface has no scrolling of its own; it hands the scroll on. The body
  // was a delegation_thunks redirect before the derivation; it is a member
  // here now, and the redirect is gone.
  void on_iface_scrolling(int a1, int a2);               // slot 2  0x0041DCB0
  // Slot 0 - 0x0041DAD0. The interface-mode entry point selects the panel and
  // visibility sequence implemented through BaseWin's owned members.
  void on_status(int mode);                              // slot 0  0x0041DAD0
  // Slots 8 and 9 - pointer movement and leave handling. They override PURE
  // slots and use the BaseWin entry subobject and its typed state.
  void on_iface_mouse_move(int xCoord, int yCoord);      // slot 8  0x0041DF50
  void on_iface_mouse_leave(int a1, int a2);             // slot 9  0x0041E3E0
  // Slot 13 - 0x0041E510, 29 bytes, no direct caller anywhere in .text: it is
  // only ever reached through the table. The catalogue names it
  // on_iface_button_clicked, at interface slot13 (0x34). Its body dispatches the
  // FRONT vtable's slot 44 - on_button_clicked - which is the same
  // interface-delegates-to-front pair slot 15 shows.
  void on_iface_button_clicked(int button_id);          // slot13 0x0041E510

  static void timer_callback(int key, int context);
  void on_left_click(int xCoord, int yCoord) override;
  void on_right_click(int xCoord, int yCoord) override;
  void on_left_double_click(int xCoord, int yCoord);

  // Storage the image proves is here: its own methods reach 0x40D20.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 293 member(s) from the IDA database, 30 named; it starts a member at 0xA14, which is where src/ ends.

  // The SubInterface is EIGHT bytes, not four, and the ProdPicker after it
  // starts at 0xA1C rather than 0xA18. ??0BaseWin@@QAE@XZ settles both:
  // 0x004084B4 `lea ecx, [esi + 0xa1c]`, 0x004084C1
  // `mov dword ptr [esi + 0xa14], 0x66a6e4` - the SubInterface vftable seven
  // other window constructors store at their own +0xA14 - then
  // `call ??0ProdPicker@@QAE@XZ`. sizeof(ProdPicker) is pinned at 0xA840 and
  // 0xA1C + 0xA840 == 0xB25C, exactly where the next `lea ecx, [esi+0xb25c]`
  // builds graphicWin2_, so the two abut and the `field_B258_` dword this
  // header used to put between them does not exist. The three members total
  // 0xA848 before and after, so nothing past graphicWin2_ moves.
  // The SubInterface is the SECOND BASE, above; MSVC puts it at
  // sizeof(GraphicWin) == 0xA14, which is where the constructor writes its
  // vftable.
  ProdPicker prodPicker_;  // 0xA1C
  GraphicWin graphicWin2_;  // 0xB25C
  Sprite sprites1_[2];  // 0xBC70
  Sprite sprites2_[2];  // 0xBCC8
  Sprite sprites3_[2];  // 0xBD20
  Sprite sprite4_;  // 0xBD78
  Sprite sprite5_;  // 0xBDA4
  Sprite sprite6_;  // 0xBDD0
  Sprite sprites7_[3];  // 0xBDFC
  FlatButton flatButton_;  // 0xBE80
  Sprite sprites8_[42];  // 0xC9CC
  Sprite sprite9_;  // 0xD104
  Sprite sprite10_;  // 0xD130
  Sprite sprite11_;  // 0xD15C
  Sprite sprite12_;  // 0xD188
  Sprite sprite13_;  // 0xD1B4
  Sprite sprite14_;  // 0xD1E0
  Sprite sprite15_;  // 0xD20C
  Buffer buffer1_;  // 0xD238
  FlatButton flatButtons_[22];  // 0xD7C0
  uint32_t field_1D048_;  // 0x1D048
  uint32_t field_1D04C_;  // 0x1D04C
  uint32_t field_1D050_;  // 0x1D050
  uint32_t field_1D054_;  // 0x1D054
  uint32_t field_1D058_;  // 0x1D058
  uint32_t field_1D05C_;  // 0x1D05C
  uint32_t field_1D060_;  // 0x1D060
  uint32_t field_1D064_;  // 0x1D064
  uint32_t field_1D068_;  // 0x1D068
  uint32_t field_1D06C_;  // 0x1D06C
  uint32_t field_1D070_;  // 0x1D070
  uint32_t field_1D074_;  // 0x1D074
  uint32_t field_1D078_;  // 0x1D078
  uint32_t field_1D07C_;  // 0x1D07C
  uint32_t field_1D080_;  // 0x1D080
  uint32_t field_1D084_;  // 0x1D084
  uint32_t field_1D088_;  // 0x1D088
  uint32_t field_1D08C_;  // 0x1D08C
  uint32_t field_1D090_;  // 0x1D090
  uint32_t field_1D094_;  // 0x1D094
  uint32_t field_1D098_;  // 0x1D098
  uint32_t field_1D09C_;  // 0x1D09C
  uint32_t field_1D0A0_;  // 0x1D0A0
  uint32_t field_1D0A4_;  // 0x1D0A4
  uint32_t field_1D0A8_;  // 0x1D0A8
  uint32_t field_1D0AC_;  // 0x1D0AC
  uint32_t field_1D0B0_;  // 0x1D0B0
  uint32_t field_1D0B4_;  // 0x1D0B4
  uint32_t field_1D0B8_;  // 0x1D0B8
  uint32_t field_1D0BC_;  // 0x1D0BC
  uint32_t field_1D0C0_;  // 0x1D0C0
  uint32_t field_1D0C4_;  // 0x1D0C4
  uint32_t field_1D0C8_;  // 0x1D0C8
  uint32_t field_1D0CC_;  // 0x1D0CC
  uint32_t field_1D0D0_;  // 0x1D0D0
  uint32_t field_1D0D4_;  // 0x1D0D4
  uint32_t field_1D0D8_;  // 0x1D0D8
  uint32_t field_1D0DC_;  // 0x1D0DC
  uint32_t field_1D0E0_;  // 0x1D0E0
  uint32_t field_1D0E4_;  // 0x1D0E4
  uint32_t field_1D0E8_;  // 0x1D0E8
  uint32_t field_1D0EC_;  // 0x1D0EC
  uint32_t field_1D0F0_;  // 0x1D0F0
  uint32_t field_1D0F4_;  // 0x1D0F4
  uint32_t field_1D0F8_;  // 0x1D0F8
  uint32_t field_1D0FC_;  // 0x1D0FC
  uint32_t field_1D100_;  // 0x1D100
  uint32_t field_1D104_;  // 0x1D104
  uint32_t field_1D108_;  // 0x1D108
  uint32_t field_1D10C_;  // 0x1D10C
  uint32_t field_1D110_;  // 0x1D110
  uint32_t field_1D114_;  // 0x1D114
  uint32_t field_1D118_;  // 0x1D118
  uint32_t field_1D11C_;  // 0x1D11C
  uint32_t field_1D120_;  // 0x1D120
  uint32_t field_1D124_;  // 0x1D124
  uint32_t field_1D128_;  // 0x1D128
  uint32_t field_1D12C_;  // 0x1D12C
  uint32_t field_1D130_;  // 0x1D130
  uint32_t field_1D134_;  // 0x1D134
  uint32_t field_1D138_;  // 0x1D138
  uint32_t field_1D13C_;  // 0x1D13C
  uint32_t field_1D140_;  // 0x1D140
  uint32_t field_1D144_;  // 0x1D144
  Time time_;  // 0x1D148
  Caviar caviar_;  // 0x1D170
  uint32_t field_1E540_;  // 0x1E540
  uint32_t field_1E544_;  // 0x1E544
  uint32_t field_1E548_;  // 0x1E548
  uint32_t field_1E54C_;  // 0x1E54C
  uint32_t field_1E550_;  // 0x1E550
  uint32_t field_1E554_;  // 0x1E554
  uint32_t field_1E558_;  // 0x1E558
  uint32_t field_1E55C_;  // 0x1E55C
  uint32_t field_1E560_;  // 0x1E560
  uint32_t field_1E564_;  // 0x1E564
  uint32_t field_1E568_;  // 0x1E568
  uint32_t field_1E56C_;  // 0x1E56C
  uint32_t field_1E570_;  // 0x1E570
  uint32_t field_1E574_;  // 0x1E574
  uint32_t field_1E578_;  // 0x1E578
  uint32_t field_1E57C_;  // 0x1E57C
  uint32_t field_1E580_;  // 0x1E580
  uint32_t field_1E584_;  // 0x1E584
  uint32_t field_1E588_;  // 0x1E588
  uint32_t field_1E58C_;  // 0x1E58C
  uint32_t field_1E590_;  // 0x1E590
  uint32_t field_1E594_;  // 0x1E594
  uint32_t field_1E598_;  // 0x1E598
  uint32_t field_1E59C_;  // 0x1E59C
  uint32_t field_1E5A0_;  // 0x1E5A0
  uint32_t field_1E5A4_;  // 0x1E5A4
  uint32_t field_1E5A8_;  // 0x1E5A8
  uint32_t field_1E5AC_;  // 0x1E5AC
  uint32_t field_1E5B0_;  // 0x1E5B0
  uint32_t field_1E5B4_;  // 0x1E5B4
  uint32_t field_1E5B8_;  // 0x1E5B8
  uint32_t field_1E5BC_;  // 0x1E5BC
  uint32_t field_1E5C0_;  // 0x1E5C0
  uint32_t field_1E5C4_;  // 0x1E5C4
  uint32_t field_1E5C8_;  // 0x1E5C8
  uint32_t field_1E5CC_;  // 0x1E5CC
  uint32_t field_1E5D0_;  // 0x1E5D0
  uint32_t field_1E5D4_;  // 0x1E5D4
  uint32_t field_1E5D8_;  // 0x1E5D8
  uint32_t field_1E5DC_;  // 0x1E5DC
  uint32_t field_1E5E0_;  // 0x1E5E0
  uint32_t field_1E5E4_;  // 0x1E5E4
  uint32_t field_1E5E8_;  // 0x1E5E8
  uint32_t field_1E5EC_;  // 0x1E5EC
  uint32_t field_1E5F0_;  // 0x1E5F0
  uint32_t field_1E5F4_;  // 0x1E5F4
  uint32_t field_1E5F8_;  // 0x1E5F8
  uint32_t field_1E5FC_;  // 0x1E5FC
  uint32_t field_1E600_;  // 0x1E600
  uint32_t field_1E604_;  // 0x1E604
  uint32_t field_1E608_;  // 0x1E608
  uint32_t field_1E60C_;  // 0x1E60C
  uint32_t field_1E610_;  // 0x1E610
  uint32_t field_1E614_;  // 0x1E614
  uint32_t field_1E618_;  // 0x1E618
  uint32_t field_1E61C_;  // 0x1E61C
  uint32_t field_1E620_;  // 0x1E620
  uint32_t field_1E624_;  // 0x1E624
  uint32_t field_1E628_;  // 0x1E628
  uint32_t field_1E62C_;  // 0x1E62C
  uint32_t field_1E630_;  // 0x1E630
  uint32_t field_1E634_;  // 0x1E634
  uint32_t field_1E638_;  // 0x1E638
  uint32_t field_1E63C_;  // 0x1E63C
  // RECOVERY_MEMBER: v2 BaseWin camera_angles_ VOX_Vect 0x1E640 0x0041E8D0
  // Original 41F3D7 computes the rotation from an Euler vector; 41F43D
  // passes the whole matrix to Caviar::set_camera_direct. Its third row
  // is negated in place at 41F3DC/41F3EA/41F3FB, then scaled by -10000.
  VOX_Vect camera_angles_;  // 0x1E640
  // RECOVERY_MEMBER: v2 BaseWin camera_rotation_ VOX_Matrix 0x1E64C 0x0041E8D0
  VOX_Matrix camera_rotation_;  // 0x1E64C
  uint32_t field_1E670_;  // 0x1E670
  MapWin mapWin_;  // 0x1E674
  uint32_t field_40AF4_;  // 0x40AF4
  uint32_t field_40AF8_;  // 0x40AF8
  uint32_t field_40AFC_;  // 0x40AFC
  int32_t field_40B00_;  // 0x40B00; signed hover selection, -1 means none
  uint32_t field_40B04_;  // 0x40B04
  uint32_t field_40B08_;  // 0x40B08
  uint32_t focusedBaseID_;  // 0x40B0C
  uint32_t field_40B10_;  // 0x40B10
  uint32_t field_40B14_;  // 0x40B14
  uint32_t field_40B18_;  // 0x40B18
  uint32_t field_40B1C_;  // 0x40B1C
  uint32_t field_40B20_;  // 0x40B20
  uint32_t field_40B24_;  // 0x40B24
  // zoom captures the complete receiver at 0x0041A928, sets this word
  // at 0x0041A9E2, then tests it around wait_2 at 0x0041AE0A/0x0041AE2D.
  // exit clears it at 0x0041A4D7. Other window clicks cancel that wait:
  // InfoWin's absolute 0x006E8150 is TutWinBaseWindow + 0x40B28.
  // This is a four-byte flag, not the window's visibility or lifetime.
  // RECOVERY_MEMBER: v1 BaseWin zoom_wait_pending_ 0x40B28 0x0041A920
  int zoom_wait_pending_;
  uint32_t field_40B2C_;  // 0x40B2C
  uint32_t field_40B30_;  // 0x40B30
  // soft_zoom copies the signed production id (negative for facilities and
  // projects) when updating the base without displaying the window.
  // RECOVERY_MEMBER: v1 BaseWin displayed_production_id_ 0x40B34 0x0041AE40
  int displayed_production_id_;
  uint32_t field_40B38_;  // 0x40B38
  uint32_t field_40B3C_;  // 0x40B3C
  uint32_t field_40B40_;  // 0x40B40
  uint32_t field_40B44_;  // 0x40B44
  uint32_t field_40B48_;  // 0x40B48
  // SIGNED, and the image says so twice. The trio holds a Spot hit and the
  // cursor's hover state, and "none" is -1: on_iface_left_down compares the
  // third with `test eax,eax / jle` (0x0041DE73), which is a signed test, and
  // all three are reset with the same `or eax,0xffffffff`. Declared `int32_t`
  // rather than the IDB's `uint32_t`: same four bytes at the same offset, and
  // an unsigned spelling compiles `jbe` where the image has `jle`.
  int32_t field_40B4C_;   // 0x40B4C
  int32_t field_40B50_;   // 0x40B50
  int32_t field_40B54_;   // 0x40B54
  uint32_t field_40B58_;  // 0x40B58
  // RECOVERY_MEMBER: v2 BaseWin supported_title_rect_ tagRECT 0x40B5C 0x0041E8D0
  RECT supported_title_rect_;  // 0x40B5C
  // RECOVERY_MEMBER: v2 BaseWin supported_content_rect_ tagRECT 0x40B6C 0x0041E8D0
  RECT supported_content_rect_;  // 0x40B6C
  // RECOVERY_MEMBER: v2 BaseWin interface_help_rect_ tagRECT 0x40B7C 0x004160F0
  RECT interface_help_rect_;  // 0x40B7C, iface_click panel help hit-test
  // RECOVERY_MEMBER: v1 BaseWin rect1_ 0x40B8C 0x00408B20
  RECT rect1_;  // 0x40B8C, IDB `rect1`
  // RECOVERY_MEMBER: v2 BaseWin facilities_title_rect_ tagRECT 0x40B9C 0x0041E8D0
  RECT facilities_title_rect_;  // 0x40B9C
  RECT rect2_;  // 0x40BAC, IDB `rect2`
  // RECOVERY_MEMBER: v2 BaseWin facilities_help_rect_ tagRECT 0x40BBC 0x004165D0
  RECT facilities_help_rect_;  // 0x40BBC, click panel help hit-test
  // Three consecutive rectangles used by the commerce title, report and
  // containing panel; the original copies/frames each as a whole RECT.
  // RECOVERY_MEMBER: v1 BaseWin commerce_title_rect_ 0x40BCC 0x00410870
  RECT commerce_title_rect_;
  // RECOVERY_MEMBER: v1 BaseWin commerce_report_rect_ 0x40BDC 0x00410870
  RECT commerce_report_rect_;
  // RECOVERY_MEMBER: v1 BaseWin commerce_outer_rect_ 0x40BEC 0x00410870
  RECT commerce_outer_rect_;
  // The nutrient renderer frames content, caption and outer bounds in order.
  // RECOVERY_MEMBER: v1 BaseWin nutrient_content_rect_ 0x40BFC 0x00411240
  RECT nutrient_content_rect_;
  // RECOVERY_MEMBER: v1 BaseWin nutrient_title_rect_ 0x40C0C 0x00411240
  RECT nutrient_title_rect_;
  // RECOVERY_MEMBER: v1 BaseWin nutrient_outer_rect_ 0x40C1C 0x00411240
  RECT nutrient_outer_rect_;
  // 0x40C2C, sixteen bytes that the IDB leaves as four unnamed dwords and
  // draw_garrison reads as one RECT: it is copied into set_clip's argument
  // whole, then its left/top/right/bottom are read separately to size the
  // garrison panel's tile and frame. Four dwords at the same offsets, so no
  // member after it moves.
  RECT garrison_rect_;  // 0x40C2C
  // RECOVERY_MEMBER: v2 BaseWin production_content_rect_ tagRECT 0x40C3C 0x0041E8D0
  RECT production_content_rect_;  // 0x40C3C
  // RECOVERY_MEMBER: v2 BaseWin production_outer_rect_ tagRECT 0x40C4C 0x0041E8D0
  RECT production_outer_rect_;  // 0x40C4C
  // The population panel clips, frames, and updates this whole rectangle.
  // RECOVERY_MEMBER: v1 BaseWin population_rect_ 0x40C5C 0x00414B40
  RECT population_rect_;
  // RECOVERY_MEMBER: v2 BaseWin queue_help_rect_ tagRECT 0x40C6C 0x004160F0
  RECT queue_help_rect_;  // 0x40C6C, iface_click panel help hit-test
  // The queue renderer frames these three consecutive regions separately.
  // RECOVERY_MEMBER: v1 BaseWin queue_title_rect_ 0x40C7C 0x00415080
  RECT queue_title_rect_;
  // RECOVERY_MEMBER: v1 BaseWin queue_content_rect_ 0x40C8C 0x00415080
  RECT queue_content_rect_;
  // RECOVERY_MEMBER: v1 BaseWin queue_outer_rect_ 0x40C9C 0x00415080
  RECT queue_outer_rect_;
  // The title panel copies these four consecutive words as one RECT before
  // clipping, framing and presenting it.
  // RECOVERY_MEMBER: v1 BaseWin info_title_rect_ 0x40CAC 0x00415B20
  RECT info_title_rect_;
  // RECOVERY_MEMBER: v4 BaseWin expenses_help_rects_ tagRECT[6] 0x40CBC 0x60 0x004165D0
  RECT expenses_help_rects_[6];  // 0x40CBC, click uses six whole RECTs
  RECT rect5_;  // 0x40D1C
  RECT rect6_;  // 0x40D2C
  RECT rect7_;  // 0x40D3C
  // RECOVERY_MEMBER: v2 BaseWin previous_button_rect_ tagRECT 0x40D4C 0x0041E8D0
  RECT previous_button_rect_;  // 0x40D4C
  // RECOVERY_MEMBER: v2 BaseWin next_button_rect_ tagRECT 0x40D5C 0x0041E8D0
  RECT next_button_rect_;  // 0x40D5C
  RECT rect14_;  // 0x40D6C
  RECT rect15_;  // 0x40D7C
  RECT rect18_;  // 0x40D8C
  RECT rect19_;  // 0x40D9C
  RECT rect8_;  // 0x40DAC
  RECT rect9_;  // 0x40DBC
  RECT rect10_;  // 0x40DCC
  RECT rect11_;  // 0x40DDC
  // Original draw_flash intersects these two owned rectangles.
  RECT flash_rect_a_;  // 0x40DEC
  RECT flash_rect_b_;  // 0x40DFC
  uint32_t field_40E0C_;  // 0x40E0C
  uint32_t field_40E10_;  // 0x40E10
  uint32_t field_40E14_;  // 0x40E14
  uint32_t field_40E18_;  // 0x40E18
  uint32_t field_40E1C_;  // 0x40E1C
  uint32_t field_40E20_;  // 0x40E20
  uint32_t field_40E24_;  // 0x40E24
  uint32_t field_40E28_;  // 0x40E28
  RECT rect12_;  // 0x40E2C
  RECT rect13_;  // 0x40E3C
  RECT rect16_;  // 0x40E4C
  RECT rect17_;  // 0x40E5C
  // RECOVERY_MEMBER: v2 BaseWin queue_button_rect_ tagRECT 0x40E6C 0x0041E8D0
  RECT queue_button_rect_;  // 0x40E6C
  RECT rect20_;  // 0x40E7C
  // Borrowed by preference and other window controls: TutWinBaseWindow at
  // 6A7628 plus this offset gives 6E84B4. The owner's constructor 408679
  // and destructor 420BC1/BCB establish this Font's ordinary lifetime.
  // RECOVERY_MEMBER: v2 BaseWin font1_ Font 0x40E8C 0x00408490
  Font font1_;  // 0x40E8C, IDB `font1`, size == sizeof(Font)
  Spot spot1_;  // 0x40EB4, size == sizeof(Spot)
  Spot spot2_;  // 0x40EC0, size == sizeof(Spot)
  Buffer buffer2_;  // 0x40ECC, size == sizeof(Buffer)
  Scroll scroll1_;  // 0x41454, size == sizeof(Scroll)
  Scroll scroll2_;  // 0x435A0, size == sizeof(Scroll)

  // The comment that stood here said the IDB's scroll3 at 0x456EC lay "beyond
  // the proven extent" and would not be declared until a body reached it, and
  // it covered the offset with 0x48 opaque bytes ending at 0x45734. A body
  // does reach it, and the 0x48 was short: tools/verify_member_offsets.py
  // reported `overrun BaseWin+0x45B30 w4: sizeof is 0x45734` against
  // ??1BaseWin@@QAE@XZ. That destructor tears the offset down as a whole
  // Scroll, inlined - 0x00420AB4 `lea edi, [esi + 0x456ec]`, then the two
  // vftable stores `mov dword ptr [edi], 0x669d58` and
  // `mov dword ptr [edi + 0x444], ebx` with ebx = 0x669d50 (the GraphicWin
  // base slot; Scroll derives from GraphicWin, whose subobject vftable sits at
  // +0x444), then `mov ecx, edi` / `call ?close@Scroll@@QAEXXZ`, then
  // ??1FlatButton@@QAE@XZ on edi+0xAAC and edi+0x15F8 - which are exactly
  // flat_button_left_ and flat_button_right_ inside Scroll - and finally
  // ??1GraphicWin@@QAE@XZ on edi itself. The next teardown, at
  // 0x00420B02 `lea edi, [esi + 0x435a0]`, repeats the same sequence on
  // scroll2_, so the shape is the one already declared twice above.
  // ??0BaseWin@@QAE@XZ constructs the matching object at the same offset
  // (`new (self + 0x456ec) Scroll();` in the deleted
  // src/recovered/units/00408490.cpp).
  // 0x456EC + sizeof(Scroll) == 0x47838, which is 8 bytes under the 0x47840
  // upper bound docs/recovery/class-size-bounds.csv derives from the next
  // constructed global, so this is the last member and BaseWin ends here.
  Scroll scroll3_;  // 0x456EC, size == sizeof(Scroll)
};


// BaseWin embeds an interface subobject at 0xA14; these four are reached
// through its vtable, so their `this` points there and is adjusted back to
// the BaseWin before dispatching to iface_click, which is not recovered.
// The first two carry the same names the catalogue's own prototype line gives
// the seven forwarders that call them (xCoord/yCoord); `button` and
// `is_double` come from the forwarders' four distinct call-site literals.
typedef void (OriginalObject::*func_base_win_iface_click)(int xCoord, int yCoord, int button, int is_double);

// draw_supported is not recovered. Its one int has no name evidence: the only
// caller reaches it with the literal 1, and nothing else reads it.
typedef void (OriginalObject::*func_base_win_draw_supported)(int a1);

// The shared click handler these three forward to is not recovered. Unlike
// the iface_click family, these carry no this-adjustment. xCoord/yCoord as
// above.
typedef void (OriginalObject::*func_base_win_click)(int xCoord, int yCoord, int button, int is_double);


// draw_facilities and garrison_click are declared on the class because the
// recovered on_scrolled and UNK2 reach them with a direct `call rel32`, but
// neither body is recovered - 1788 and 3768 bytes respectively - so the DLL
// has nothing to link the calls against. Both stand in with a forward to the
// original image until the bodies land.
//   ?draw_facilities@BaseWin@@QAEXH@Z    0x0040FCC0  public __thiscall void(int)
//   ?garrison_click@BaseWin@@QAEXHHHH@Z  0x0040B140  public __thiscall
//                                                    void(int, int, int, int)
// draw_facilities is not recovered; its one int has no name evidence - the
// only caller reaches it with the literal 1.
typedef void (OriginalObject::*func_base_win_draw_facilities)(int a1);

typedef void (OriginalObject::*func_base_win_garrison_click)(int vehID, int a2,
                                                             int a3, int a4);
