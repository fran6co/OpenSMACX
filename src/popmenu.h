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
#include "popup.h"
#include "pulldown.h"

 /*
  * PopMenu class
  *
  * The base is Popup, not BasePop. ??1PopMenu@@QAE@XZ takes `this` in ecx at
  * 0x0042141D (`mov esi, ecx`), restores it unchanged at 0x00421447
  * (`mov ecx, esi`), and calls ?close@Popup@@QAEXXZ at 0x00421450. A receiver
  * that never left `this` is the object being closed, so a Popup begins at
  * offset zero. Popup itself derives from BasePop, so the old `: BasePop` was
  * the same chain read one level short.
  *
  * WITHDRAWN: the previous comment said no constructor survives, so "the usual
  * check cannot run", and rested the edge on init() instead. Both halves were
  * wrong. The destructor IS in the catalogue - 0x00421400, 803 bytes - and it
  * is exactly the witness verify_subobjects.py reads. And init() could never
  * have settled the depth it was used to settle: ?init@PopMenu@@QAEHXZ at
  * 0x0059D3A0 is `push 0; push 0; call ?init@BasePop@@QAEHHJ@Z; ret`, and
  * BasePop sits at offset zero whether the edge is PopMenu : BasePop or
  * PopMenu : Popup : BasePop, so an unadjusted ecx there distinguishes
  * nothing.
  *
  * WITHDRAWN: the eleven field_3230_ .. field_4C6C_ members that used to
  * stand here, claimed as "storage the image proves is here ... its own
  * methods reach 0x4C70". Every offset in that slab is inside Popup::scroll_,
  * and the destructor addresses them off the Scroll rather than off `this`:
  * 0x00421455 loads edi = this + 0x3230 (the Scroll), then 0x004214A8 does
  * `lea ebx, [edi + 0xaac]` and 0x00421479 `lea ebx, [edi + 0x15f8]` -
  * Scroll::flat_button_left_ and Scroll::flat_button_right_ at their own
  * declared offsets. The six dword writes the slab was built from are the two
  * vtable slots of each of those three objects, at +0 and +0x444: 0x3230 and
  * 0x3674 for the Scroll, 0x3CDC and 0x4120 for the left button, 0x4828 and
  * 0x4C6C for the right. None of them is PopMenu storage, and the 0x4C70
  * lower bound they produced belongs to Popup - docs/recovery/
  * access-lower-bounds.csv states the identical 0x4C70 for Popup itself,
  * from the same instruction reached through ??1Popup@@QAE@XZ.
  *
  * The second base starts at 0x537C, which is sizeof(Popup) exactly
  * (0x3230 + 0x214C, both pinned), so nothing sits between the base and it.
  * Two independent measurements agree it is a PullDown:
  *   - ??1PopMenu@@QAE@XZ runs `lea ecx, [esi + 0x537c]` at 0x0042142C and
  *     calls ??1PullDown@@QAE@XZ at 0x00421432, under the usual null guard;
  *   - the pinned original adjustor thunks (the retired source record was
  *     src/deleting_thunks.cpp) step a receiver back by
  *     0x537C (PopMenu::delete2, 0x00421990) and by 0x57C0 = 0x537C + 0x444
  *     (PopMenu::delete3, 0x004219A0). A pair of vtables 0x444 apart is a
  *     GraphicWin, which is what PullDown derives from.
  * PullDown is pinned at 0xF40, so the object is proved to run to 0x62BC.
  * BaseWin::queue_click's constructor sequence independently distinguishes
  * inheritance from the old member hypothesis: a LEA at 0x00409228 begins Popup setup,
  * then the actual constructor CALL at 0x0040922E constructs Popup. A LEA at
  * 0x00409233 begins PullDown setup; CALL 0x00409240 constructs PullDown at
  * +0x537C. Four immediate vptr stores at 0x00409245, 0x0040924F, 0x00409259 and 0x00409263;
  * each store is 10 bytes and the last ends at 0x0040926D. They install
  * PopMenu's four derived views at +0, +0x444, +0x537C and +0x57C0.
  * Their original slot-zero targets are 0x00421640, 0x00421980, 0x00421990
  * and 0x004219A0, respectively. A plain PullDown member would retain its
  * own tables rather than these PopMenu receiver-adjusting entries.
  * Whether anything follows it is NOT established - a member with no
  * constructor and no destructor is never built by a call and leaves no site
  * for verify_subobjects.py to see - so this class still carries no size
  * assertion and 0x62BC is a floor.
  */
  // PUBLIC: access specifiers change nothing about layout, and this
  // tree's bodies reach base methods the image reaches with a direct
  // `call rel32`. Spelled `class X : Base` - private, since that is
  // what `class` means - those calls do not compile at all, and the
  // seam that stood in for them cost the caller `call [ptr]`.
class PopMenu : public Popup, public PullDown {
 public:
  PopMenu() { ; }
  // Original 0x00421400: destroy PullDown then Popup in reverse base order.
  ~PopMenu();
  int init();
  // The caller at 0x004B9178 switches on the selected index returned in EAX;
  // the catalogue's void return omitted that observed result.
  int exec(int x, int y, int (*poll)());
  // exec returns a slot; picker handlers sometimes need the action ID stored
  // in that slot (which need not equal its index after filtering).
  int get_selected_id() { return PullDown::get_selected_id(); }
};

// Opens one owning PopMenu and returns its selected slot.
int __cdecl popm(char *section, int x, int y, int (__cdecl *poll)());

// File-and-section overload opens the same owning PopMenu lifecycle.
int __cdecl popm(char *file, char *section, int x, int y, int (__cdecl *poll)());
