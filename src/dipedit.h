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

#include "original_seam.h"
#include "checkbox.h"
#include "flatbutton.h"
#include "font.h"
#include "graphicwin.h"
#include "listbox.h"

 /*
  * DipEdit class
  *
  * Derives from GraphicWin by ordinary single inheritance. `??1DipEdit@@QAE@XZ`
  * (0x004E2240) parks the receiver with `mov esi, ecx` at 0x004E225D and never
  * reloads esi; after the last member subobject is torn down - `??1Font@@QAE@XZ`
  * on `lea ecx, [esi + 0xa24]` at 0x004E2396 - it issues
  *
  *     0x004E239B  mov ecx, esi
  *     0x004E23A4  call ??1GraphicWin@@QAE@XZ
  *
  * with no lea in between. An UNADJUSTED `this`, and last of all the teardown
  * calls: that is the base subobject's destructor. `??_GDipEdit@@UAEPAXI@Z` is
  * decorated `U`, so DipEdit has a vptr of its own; GraphicWin sitting at
  * offset 0 rather than 4 means DipEdit shares GraphicWin's vptr, which an
  * embedded first member could not do. So this is inheritance, not containment.
  *
  * That measurement WITHDRAWS the previous "Layout not established" reading
  * and the seven anonymous declarations it justified. They covered exactly
  * [0, 0xA14) - field_0_, field_4_[0x470], field_474_, field_478_, field_47C_,
  * field_480_, field_484_[0x590] - which is exactly sizeof(GraphicWin) (0xA14),
  * so they were the base's bytes described as unmapped storage; the base
  * declaration replaces them and moves nothing.
  *
  * THE MEMBERS BELOW ARE THE IDB'S OWN NAMES, proven by the compiler's
  * member teardown in ??1DipEdit (docs/recovery/subobject-sites.csv):
  *
  *   0xA24   Font      ??1Font on `lea ecx,[esi+0xa24]`  (IDB `font`)
  *   0xA4C   ListBox   ??1ListBox entered on the GraphicWin vbase at
  *                     +0x48 = 0xa94, then ??1Dialog at +0xa60 = 0x14ac
  *                     and ??1GraphicWin at 0xa94 - the
  *                     member-with-virtual-bases pattern
  *   0x15A0  CheckBox  ??1CheckBox entered on its GraphicWin vbase at
  *                     +0x1c = 0x15bc, then ??1Dialog at +0xa34 = 0x1fd4
  *                     and ??1GraphicWin at 0x15bc
  *   0x20C8..0x4DF8    five FlatButtons, one every 0xB4C (each
  *                     sizeof(FlatButton)); ??1DipEdit expands each
  *                     ~FlatButton inline - the 0x669754/0x66974c stage
  *                     pair, `call ?close@FlatButton`, `call
  *                     ??1BaseButton`, with the EH state stepped between
  *                     - which is why ~FlatButton is defined in-class in
  *                     flatbutton.h
  *
  * The old extent reading is withdrawn with them: "methods reach 0x5240" was
  * ??1DipEdit's `mov dword ptr [edi + 0x444], ebx` at 0x004E227E - the Buffer
  * vtable slot INSIDE the FlatButton at 0x4df8 (0x4df8 + 0x444 = 0x523c), not
  * an offset into DipEdit. The object is 0x5944 bytes, and the IDB's member
  * list stops at flatButton5 + 2892 for exactly that reason.
  * `?init@DipEdit@@QAEXH@Z` (0x004DB080) reaches them by name too:
  * `?close@ListBox@@QAEXXZ` at +0xa4c (0x004DB2BF), `?close@CheckBox@@QAEXXZ`
  * at +0x15a0 (0x004DB440), and per-button loads at 0x42ac (0x004DB752) and
  * 0x4df8 (0x004DB6EB).
  */
class DipEdit : public GraphicWin {
 public:
  // 0x004DB080 (?init@DipEdit@@QAEXH@Z), defined in src/dipedit.cpp. The int
  // is the faction whose status the window opens on: init stores it straight
  // into faction_index_ (`mov dword ptr [esi+0xa1c], ecx` at 0x004DB0BA) and
  // then seeds selected_target_ from it.
  void init(int faction_index);
  void on_selected(int index);
  // The four checkbox-to-world writers, 0x004DA760/0x004DA7E0/0x004DA870/
  // 0x004DA900. Each asks the embedded CheckBox whether the item for one
  // diplomatic bit is checked (CheckBox::get_state_id) and settles that bit
  // between faction_index_ and selected_target_ where the status word
  // disagrees - a 2x2 over WHICH word and WHICH writer:
  //   sync_treaty_bit  treaty, mirrored - treaty_on/treaty_off maintain
  //                    both factions' rows
  //   sync_agenda_bit  agenda, mirrored - agenda_on/agenda_off likewise
  //   set_treaty_bit   treaty, our row only - set_treaty(.., TRUE/FALSE)
  //   set_agenda_bit   agenda, our row only - set_agenda(.., TRUE/FALSE)
  // The agenda pair takes the checkbox id `0x80000000 | bit`, which is the
  // second PlayerData word - diplo_agenda, 8 dwords past diplo_treaties -
  // exactly the ids do_check writes those two views of the table back with.
  // THE CATALOGUE NAMES WERE UNK1..UNK4 (IDA placeholders): the image
  // carries no symbol and no reference for any of the four - no E8 call
  // site, no vtable slot, no data word points at 0x004DA760..0x004DA98B -
  // so the names here are OURS, and each marker carries a `// symbol` fact
  // so the claim still finds the body our object emits.
  // ?read_check@DipEdit@@QAEXXZ (0x004DA990) inlines sync_treaty_bit's
  // shape eleven times for the eleven DTREATY_* bits and calls the other
  // three for the rest: these are its per-bit helpers.
  void sync_treaty_bit(int bit);   // 0x004DA760 (catalogue ?UNK1@)
  void sync_agenda_bit(int bit);   // 0x004DA7E0 (catalogue ?UNK2@)
  void set_treaty_bit(int bit);    // 0x004DA870 (catalogue ?UNK3@)
  void set_agenda_bit(int bit);    // 0x004DA900 (catalogue ?UNK4@)
  // 0x004D9DE0, defined in src/dipedit.cpp. The click handler the five
  // FlatButtons share: `a1` is the control id, -1 being the window's own
  // close row (release_modal and nothing else), 1 the treaty/agenda row,
  // 2 the integrity prompt and 3 the atrocity prompt.
  void on_button_clicked(int button_id);
  // 0x004DA990, defined in src/dipedit.cpp.

  void read_check();
  void do_check();  // 0x004DADA0, defined in src/dipedit.cpp
  DipEdit() { ; }
  // EMPTY ON PURPOSE: the teardown IS the compiler's, from the member
  // declarations below - five expanded ~FlatButtons, the CheckBox (dtor
  // entered on its GraphicWin vbase at 0x15bc, then ??1Dialog/??1GraphicWin
  // on its virtual bases), the ListBox (dtor entered on 0xa94, then its
  // virtual bases), the Font, and the GraphicWin base - in that order,
  // reverse of declaration, with the EH state stepped between each. Defined
  // out-of-line in dipedit.cpp, beside its claim against
  // ??1DipEdit@@QAE@XZ 0x004E2240.
  ~DipEdit();
  void on_left_click(int, int);
  void on_redraw();

 private:
  // The GraphicWin base occupies [0, 0xA14). DipEdit's own storage starts
  // here. THE WINDOW SIZE: ?init@DipEdit@@QAEXH@Z writes 0x2F6 (758) and
  // 0x18E (398) here (0x004DB0C6/0x004DB0D0) and passes the same two
  // constants as the width/height arguments of the GraphicWin::init call at
  // 0x004DB0DA - BaseButton::init's signature names those positions
  // width/height. BasePop's set_width (0x00601B20, BYTE_EXACT) tests the
  // same-offset field in that class, which is what makes the pair
  // width-then-height rather than the reverse.
  // SIGNED, and this is measured not stylistic: init's
  // `width_ / 3` at 0x004DB259 compiles to the 0x55555556 magic multiply with
  // the `shr eax,0x1f / add` sign fixup, and its `width_ / 2` at 0x004DB426 to
  // `cdq / sub eax,edx / sar eax,1`. Both are the SIGNED sequences; an
  // unsigned field emits `shr` alone and loses the fixup.
  int width_;   // 0xA14
  int height_;  // 0xA18
  // The faction whose status table row we're viewing. do_check indexes
  // PlayersData[faction_index_].diplo_treaties[selected_target_].
  uint32_t faction_index_;  // 0xA1C
  // 0xA20, the IDB's `field_A20`. `?on_selected@DipEdit@@QAEXH@Z` at
  // 0x004DA740 stores its incoming argument straight here:
  // `mov dword ptr [esi + 0xa20], eax`. The IDB's next member is `font` at
  // 0xA24, so this is a standalone dword rather than the head of a run.
  // The selected target faction, stored by on_selected(index).
  int32_t selected_target_;
  // The IDB's `font` at 0xA24: 0x28 == sizeof(Font).
  Font font_;  // 0xA24
  // THE EMBEDDED LISTBOX at the IDB's field_A4C: `?init@DipEdit@@QAEXH@Z`
  // calls `?close@ListBox@@QAEXXZ` on this+0xA4C (0x004DB2BF), and
  // ??1DipEdit tears it down with the member-with-virtual-bases pattern:
  // ??1ListBox entered on the GraphicWin vbase at 0xA4C+0x48 = 0xA94,
  // ??1Dialog at +0xA60 = 0x14AC, ??1GraphicWin at 0xA94.
  ListBox list_box_;  // 0xA4C
  // THE EMBEDDED CHECKBOX, real now: the five declarations it replaces
  // (field_15A0_ + field_15A4_[0xC] + field_15B0_ + field_15B4_ +
  // field_15B8_[0xB10]) total exactly sizeof(CheckBox) = 0xB28 at
  // 0x15A0..0x20C8. Its construction and destruction are the compiler's
  // from the real CheckBox() ctor, and the twelve set_state_id calls in
  // do_check reach it as `check_box_.set_state_id(...)` through the member.
  CheckBox check_box_;  // 0x15A0
  // THE FIVE IDB FlatButtons (flatButton1..flatButton5, 2892 == 0xB4C each).
  // ??1DipEdit destroys them last-declared-first (0x4DF8 down to 0x20C8),
  // each as an expanded ~FlatButton, so they are declared in address order
  // here.
  FlatButton flat_button1_;  // 0x20C8
  FlatButton flat_button2_;  // 0x2C14
  FlatButton flat_button3_;  // 0x3760
  FlatButton flat_button4_;  // 0x42AC
  FlatButton flat_button5_;  // 0x4DF8
};

static_assert(sizeof(DipEdit) == 0x5944,
              "DipEdit layout must match the legacy ABI");


// The three siblings on_selected and DipEdit::init call share one signature:
// ?read_check@DipEdit@@QAEXXZ (0x004DA990) and ?do_check@DipEdit@@QAEXXZ
// (0x004DADA0) are both public __thiscall void(void), and both are defined in
// src/dipedit.cpp now. They read as a pair: do_check pulls the diplomacy
// status word INTO the checkboxes, read_check pushes the checkboxes OUT to the
// status word - which is why on_selected commits the outgoing target's
// checkbox state BEFORE it stores the new index.
