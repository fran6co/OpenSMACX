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
#include "stdafx.h"
#include "dipedit.h"

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004D9DC0 BYTE_EXACT
// name      ?on_left_click@DipEdit@@QAEXHH@Z
// size      3 bytes
// spans     0x004D9DC0-0x004D9DC3
// prototype void (__thiscall ?on_left_click@DipEdit@@QAEXHH@Z)(DipEdit* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void DipEdit::on_left_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004D9DD0 BYTE_EXACT
// name      ?on_redraw@DipEdit@@QAEXXZ
// size      1 bytes
// spans     0x004D9DD0-0x004D9DD1
// prototype void (__thiscall ?on_redraw@DipEdit@@QAEXXZ)(DipEdit* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void DipEdit::on_redraw() {
}

void __fastcall dip_edit_on_left_click_redirect(DipEdit *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

void __fastcall dip_edit_on_redraw_redirect(DipEdit *self, void *) {
    self->on_redraw();
}

/*
ORIGINAL: 0x004DA740 BYTE_EXACT
// name      ?on_selected@DipEdit@@QAEXH@Z
// size      32 bytes
// spans     0x004DA740-0x004DA760
// prototype void (__thiscall ?on_selected@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   0   call targets   2
Status: Complete
*/
void DipEdit::on_selected(int a1) {
    read_check();
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xa20) = a1;
    do_check();
}

func_dip_edit_check DipEditReadCheck =
    original_method<func_dip_edit_check>(0x004DA990);
func_dip_edit_check DipEditDoCheck =
    original_method<func_dip_edit_check>(0x004DADA0);

/*
 * Forwarders, not recoveries. These two bodies are still in the original
 * image; on_selected above calls them, so the DLL has to resolve the symbols,
 * and until the bodies are decoded the definition is a seam into 0x004DA990
 * and 0x004DADA0. Deliberately carrying no `Original Offset:` line: that
 * annotation marks a RECOVERED body, and `repair_source_locations` indexes
 * every one of them by address. Claiming it here would point the census at a
 * forwarder the day either address is promoted.
 */
void DipEdit::read_check() {
    (ORIGINAL(this)->*DipEditReadCheck)();
}

void DipEdit::do_check() {
    (ORIGINAL(this)->*DipEditDoCheck)();
}
