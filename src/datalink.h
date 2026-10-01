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
#include "buttongroup.h"
#include "listbox.h"
#include "font.h"
#include "stringbox.h"
#include "caviar.h"
#include "flic.h"

#include "original_seam.h"
#include "graphicwin.h"
#include "subinterface.h"

/*
 * Datalink class
 *
 * The primary GraphicWin is at zero; the real SubInterface is the secondary
 * base at +0xA14, as established by the constructor's vftable stores and the
 * image's Datalink-specific 19-slot table. The table puts 0x432060 in slot 0,
 * 0x432160..0x432210 in their event slots, and shared return stubs in 1, 2,
 * 6, 14, 17 and 18. This typed hierarchy is the C++ representation of that
 * evidence, not a hand-built vtable.
  *
 * The member layout is partial. Its declared state begins at 0xA1C after the
 * proved interface base; where the object ends is unknown, so nothing pins
 * its sizeof and most fields between are not modelled. The
  * methods recovered here are bare returns that touch no field, which is why
  * they can be replaced ahead of that mapping.
  */
// Declared and deliberately NOT defined. parse_id writes only its first four
// bytes, and nothing available says how large it is or what follows: the type
// exists in the original's mangled name and in no analysis output here. An
// incomplete type expresses exactly that - the parameter is this type, and
// this translation unit knows nothing else about it - where inventing a
// one-int struct would state a size the evidence does not support.
struct DatalinkID;

// RECOVERY_BASE: v1 Datalink GraphicWin 0x0 0x00428FC0
// RECOVERY_BASE: v1 Datalink SubInterface 0xA14 0x00428FC0
// Constructor installs this original interface table at the declared base.
// Its nineteen observed slots do not independently establish its end bound.
// RECOVERY_GENERATED: v1 vtable 0x0066AF30 ? ??_7Datalink@@6B@ 0x00428FC0
class Datalink : public GraphicWin, public SubInterface {
 public:
  // 0x00429180, a pending_bodies forwarder.
  void exec(unsigned int topic, int index);

 public:
  void hide();
  void show(int visible);
  void on_selected(int index);
  // 0x0042BF10: restore the entry panel and dispatch its selected topic.
  void draw_entry();
  Datalink();
  ~Datalink();
  int UNK1(int a1, int a2);
  // 0x0042A0B0 parses a decimal topic id and writes its category and entry.
  void UNK2(char *text, int *category, int *entry);
  void parse_id(int id, DatalinkID *out_id, int *out_remainder);
  // These three are catalogued ?UNK3@Datalink@@QAEXIH@Z,
  // ?UNK4@Datalink@@QAEXHH@Z and ?UNK5@Datalink@@QAEXXZ. The names
  // below are ours, from the measured behaviour; the catalogue names
  // are carried by `// symbol` facts under the markers in datalink.cpp.
  void show_topic(unsigned int category, int facility);
  void push_history(int category, int facility);
  void pop_history();
  void UNK6();
  void UNK7(char *file, char *section);
  void UNK8();
  void UNK9();
  void close();
  void on_left_click(int a1, int a2);
  void on_group_clicked(int a1, int a2);
  void on_mouse_move(int a1, int a2);
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_down(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_clicked(int a1);
  void on_iface_button_toggled(int a1, int a2);

  // 0x00429300 ?init@Datalink@@QAEXXZ - the whole-window bring-up.
  void init();
  // 0x0042A150 ?on_dialog_item_back_draw@Datalink@@QAEHPAUGraphicWin@@HHPAURECT@@@Z
  int on_dialog_item_back_draw(GraphicWin *win, int fade, int mode, RECT *rect);
  // 0x0042A2C0 ?set_cat@Datalink@@QAEXIH@Z - rebuilds the entry list for one
  // category, then dispatches to the category's own set_cat_* filler.
  void set_cat(unsigned int category, int index);
  // 0x0042A4F0 .. 0x0042BAB0 - the fourteen category fillers set_cat calls.
  void set_cat_all();
  void set_cat_unit();
  void set_cat_chassis();
  void set_cat_reactor();
  void set_cat_weapon();
  void set_cat_defense();
  void set_cat_ability();
  void set_cat_facility();
  void set_cat_project();
  void set_cat_tech();
  void set_cat_concept();
  void set_cat_adv_concept();
  void set_cat_social();
  void set_cat_effect();
  void set_cat_faction();
  void set_cat_terraform();
  // 0x0042BF10 and the fifteen topic drawers draw_entry dispatches to.
  void draw_concept();
  void draw_adv_concept();
  void draw_unit();
  void draw_chassis();
  void draw_reactor();
  void draw_weapon();
  void draw_defense();
  void draw_ability();
  void draw_facility();
  void draw_project();
  void draw_social();
  void draw_effect();
  void draw_tech();
  void draw_faction();
  void draw_terraform();
  // 0x00431DC0 ?on_link_clicked@Datalink@@QAEHHPAD@Z
  int on_link_clicked(int, char *link);
  void on_redraw();
  void on_button_passover(int, int);
  // VOID, not int: the catalogue spells ?on_button_clicked@Datalink@@QAEXH@Z
  // (0x00431EC0), and Win's slot-44 virtual - renamed on_button_clicked by
  // the win-cluster the same day - is QAEXH too. The int spelling collided
  // C2555 at collection; this is the merge-order fix.
  void on_button_clicked(int button_id);
  int on_key_click(int, int);
  void on_status(int visible);
  void on_iface_group_clicked(int, int, int);
  void construct_id(char *out, int index, int suffix);
  void set_index(int, int);
  // The shared labs renderer reads 0x714244..0x714250: the singleton
  // at 0x703EA0 plus this existing rectangle's offset, 0x103A4.
  const RECT &entry_clip_rect() const { return entry_clip_rect_; }
  // 0x0042A100 ?on_dialog_back_draw@Datalink@@QAEHPAUGraphicWin@@@Z
  int on_dialog_back_draw(GraphicWin *win);
  void on_iface_scrolled(int, int) override;
  void on_iface_scrolling(int, int) override;
  void on_iface_left_up(int, int) override;
  int on_iface_button_pressed(int) override;
  int on_iface_dialog_back_draw(GraphicWin *) override;
  int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                    RECT *) override;

  // EVERY TOPIC CHANGE ENDS WITH THE SAME CALL, and the image inlines it at
  // all five sites: the GraphicWin-part virtual at vtable offset 0xF8 of the
  // entry list's GraphicWin virtual base, which is the repaint. The walk is
  // spelled by hand because no GraphicWin virtual table this deep is modelled
  // anywhere in the tree - see the note on vtable_shim.h - and spelling it
  // here keeps the vbtable-relative addressing the image has: calling Win's
  // own redraw through the ListBox member makes the compiler read the
  // GraphicWin displacement out of the list's vbtable at run time, never baked
  // in. Defined in datalink.cpp, whose /Ob2 re-inlines it at each caller.
  void repaint_slot();


 private:
  // SubInterface ends at 0xA1C. Everything between that base and the three
  // fields close() clears is unmapped; the span holds their offsets in place.
  // SubInterface occupies [0xA14, 0xA1C), replacing the former raw dwords.
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  // Constructor428FFA and destructor43256B use this real animation owner.
  // RECOVERY_MEMBER: v2 Datalink animation_ Flic 0xA28 0x00428FC0
  Flic animation_;  // 0xA28, ends at 0x150C
  // Topic drawers construct/render through this receiver. Their reset at
  // Datalink+0x28D8 is Caviar's instance count at +0x13CC, not a separate field.
  // RECOVERY_MEMBER: v2 Datalink caviar_ Caviar 0x150C 0x0042D610
  Caviar caviar_;  // 0x150C, ends at 0x28DC
  uint8_t field_28DC_[0x100];  // 0x28DC
  uint32_t field_29DC_;  // 0x29DC
  // 0x29E0 .. 0x2A34: the twenty-one slot "where have I been" ring that UNK4
  // pushes and UNK5 pops. Slot 0 doubles as the id of the facility currently
  // on show - close() clears it and UNK3 writes it - which is why UNK4's own
  // duplicate test reads `0x29e0 + count*4`, the slot AFTER the last one it
  // pushed, while its store writes `0x29e4 + count*4`.
  int32_t history_[21];  // 0x29E0, slot 0 is the facility id
  int32_t field_2A34_;   // 0x2A34, how many slots are filled
  // Category callback writes secondary-base +0x2024, full Datalink+0x2A38;
  // history insertion reads the same flag while list selection is in progress.
  // RECOVERY_MEMBER: v1 Datalink field_2A38_ 0x2A38 0x00432210
  int32_t field_2A38_;   // 0x2A38

  // Storage the image proves is here: its own methods reach 0xF62C.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 20 member(s) from the IDA database, 20 named; it starts a member at 0x2A3C, which is where src/ ends.

  // Original vector construction/destruction uses 18 elements at stride0xB4C.
  // RECOVERY_MEMBER: v4 Datalink topic_buttons_ FlatButton[18] 0x2A3C 0xCB58 0x00428FC0
  FlatButton topic_buttons_[18];  // 0x2A3C, ends at 0xF594
  ButtonGroup buttonGroup_;  // 0xF594
  ListBox listBox_;  // 0xF628

  Font font1_;  // 0x1017C, IDB `font1`, size == sizeof(Font)

  Font font2_;  // 0x101A4, IDB `font2`, size == sizeof(Font)

  Font font3_;  // 0x101CC, IDB `font3`, size == sizeof(Font)

  // Storage the image proves is here: its own methods reach 0x1B394.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // Original constructor and reverse destructor walk all eleven Font objects.
  // RECOVERY_MEMBER: v2 Datalink auxiliary_font4_ Font 0x101F4 0x00428FC0
  Font auxiliary_font4_;  // 0x101F4
  // RECOVERY_MEMBER: v2 Datalink auxiliary_font5_ Font 0x1021C 0x00428FC0
  Font auxiliary_font5_;  // 0x1021C
  // RECOVERY_MEMBER: v2 Datalink body_font_ Font 0x10244 0x004308B4
  Font body_font_;  // 0x10244
  // RECOVERY_MEMBER: v2 Datalink secondary_body_font_ Font 0x1026C 0x00430C80
  Font secondary_body_font_;  // 0x1026C
  // RECOVERY_MEMBER: v2 Datalink auxiliary_font8_ Font 0x10294 0x00428FC0
  Font auxiliary_font8_;  // 0x10294
  // RECOVERY_MEMBER: v2 Datalink link_font_ Font 0x102BC 0x004308AE
  Font link_font_;  // 0x102BC
  // RECOVERY_MEMBER: v2 Datalink auxiliary_font10_ Font 0x102E4 0x00428FC0
  Font auxiliary_font10_;  // 0x102E4
  // RECOVERY_MEMBER: v2 Datalink auxiliary_font11_ Font 0x1030C 0x00428FC0
  Font auxiliary_font11_;  // 0x1030C
  RECT panel_rect_;  // 0x10334
  uint8_t field_10344_[0x10];  // 0x10344
  RECT illustration_rect_;  // 0x10354
  RECT title_rect_;  // 0x10364
  RECT interface_update_rect_;  // 0x10374
  RECT description_rect_;  // 0x10384
  uint32_t field_10394_;  // 0x10394
  uint32_t field_10398_;  // 0x10398
  uint32_t field_1039C_;  // 0x1039C
  uint32_t field_103A0_;  // 0x103A0
  // RECOVERY_MEMBER: v2 Datalink entry_clip_rect_ tagRECT 0x103A4 0x00498B30
  RECT entry_clip_rect_;  // 0x103A4
  RECT entry_update_rect_;  // 0x103B4
  RECT entry_frame_rect_;  // 0x103C4
  // 0x103D4: the entry-list rectangle every set_cat_* filler frames and every
  // draw_* topic drawer paints into - four dwords in the image, read as
  // left/top/right/bottom.
  RECT category_rect_;    // 0x103D4
  uint32_t field_103E4_;  // 0x103E4
  uint32_t field_103E8_;  // 0x103E8
  uint32_t field_103EC_;  // 0x103EC
  uint32_t field_103F0_;  // 0x103F0
  uint8_t field_103F4_[0x70];  // 0x103F4
  uint32_t field_10464_;  // 0x10464
  uint32_t field_10468_;  // 0x10468
  uint32_t field_1046C_;  // 0x1046C
  uint32_t field_10470_;  // 0x10470
  uint32_t field_10474_;  // 0x10474
  uint32_t field_10478_;  // 0x10478
  uint32_t field_1047C_;  // 0x1047C
  uint32_t field_10480_;  // 0x10480
  uint8_t field_10484_[0x70];  // 0x10484
  uint32_t field_104F4_;  // 0x104F4
  uint32_t field_104F8_;  // 0x104F8
  uint32_t field_104FC_;  // 0x104FC
  uint32_t field_10500_;  // 0x10500
  uint32_t field_10504_;  // 0x10504
  uint32_t field_10508_;  // 0x10508
  uint32_t field_1050C_;  // 0x1050C
  uint32_t field_10510_;  // 0x10510
  // RECOVERY_MEMBER: v2 Datalink concept_box_ StringBox 0x10514 0x0042C438
  StringBox concept_box_;  // 0x10514
  // RECOVERY_MEMBER: v2 Datalink blurb_box_ StringBox 0x130B4 0x0042BEC0
  StringBox blurb_box_;  // 0x130B4
  // RECOVERY_MEMBER: v2 Datalink detail_box_ StringBox 0x15C54 0x0042C07C
  StringBox detail_box_;  // 0x15C54

  // 0x187F4, sizeof(StringBox) 0x2BA0, ends 0x1B394 = the end of the object.
  // exec (0x00429180) inits it with a rectangle and two fonts and draws the
  // topic text through it; draw_concept and draw_faction read 0x187F4 too.
  // Its own Buffer base therefore lands at 0x18C38, which the placeholder
  // words below used to stand in for.
  StringBox stringBox_;  // 0x187F4, sizeof(StringBox), ends 0x1B394
};

static_assert(sizeof(Datalink) == 0x1B394,
              "Datalink layout must match the original executable");


// Datalink::exec (thiscall, `void exec(unsigned int topic, int index)`) is not
// yet source-owned; every help_* forwarder below dispatches through it
// against the Datalink singleton, so the exec entry point remains a seam here
// rather than a baked-in literal.

// THE SINGLETON the forwarders dispatch against - REAL STORAGE now, defined
// in datalink.cpp at 0x00703EA0's place in the image. It was
// `Datalink *const DatalinkMain = (Datalink *)0x00703EA0`: plain data, not a
// call target, but terranx.exe data all the same, and every `DatalinkMain->
// exec` here set a receiver that pointed outside this binary. The object
// keeps the folded receiver (`mov ecx, 0x703ea0` becomes
// `mov ecx, <reloc>`) and `->` becomes `.`.
extern Datalink DatalinkMain;  // 0x00703EA0

// Append the original help markup to an existing caller-owned text buffer.
void __cdecl help_create_link(char *out, char *label, char *id);

void __cdecl help_tech(int id);

MEASURED inline void __cdecl help_weapon(int id) {
    DatalinkMain.exec(0x6, id);
}

MEASURED inline void __cdecl help_armor(int id) {
    DatalinkMain.exec(0x7, id);
}

MEASURED inline void __cdecl help_chassis(int id) {
    DatalinkMain.exec(0x4, id);
}

MEASURED inline void __cdecl help_facility(int id) {
    DatalinkMain.exec(0xA, id);
}

MEASURED inline void __cdecl help_abil(int id) {
    DatalinkMain.exec(0x8, id);
}

MEASURED inline void __cdecl help_social(int id) {
    DatalinkMain.exec(0xC, id);
}

MEASURED inline void __cdecl help_faction(int id) {
    DatalinkMain.exec(0xF, id);
}

MEASURED inline void __cdecl help_veh(int id) {
    DatalinkMain.exec(0x3, id);
}

MEASURED inline void __cdecl help_topic(unsigned int topic, int index) {
    DatalinkMain.exec(topic, index);
}
