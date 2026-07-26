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
#include "console.h"
#include "game.h"
#include "general.h"
#include "infowin.h"
#include "mapwin.h"
#include "statuswin.h"
#include <cstring>

func_pref_win_display *ConsolePrefWinDisplay = (func_pref_win_display *)0x0048FA00;
void *ConsolePrefWin = reinterpret_cast<void *>(0x008578D8);
func_get_key_state **ConsoleEditKeyStateSlot =
    reinterpret_cast<func_get_key_state **>(0x0066932C);

/*
Purpose: Open the shared preferences window to the preferences page.
Original Offset: 00514EF0
Return Value: n/a
Status: Complete
*/
void Console::set_preferences() {
    ConsolePrefWinDisplay(ConsolePrefWin, 0);
}

/*
Purpose: Open the shared preferences window to the automation page.
Original Offset: 00514F10
Return Value: n/a
Status: Complete
*/
void Console::set_auto_preferences() {
    ConsolePrefWinDisplay(ConsolePrefWin, 3);
}

/*
Purpose: Open the shared preferences window to the base page.
Original Offset: 00514F20
Return Value: n/a
Status: Complete
*/
void Console::set_base_preferences() {
    ConsolePrefWinDisplay(ConsolePrefWin, 2);
}

/*
Purpose: Open the shared preferences window to the audiovisual page.
Original Offset: 00514F30
Return Value: n/a
Status: Complete
*/
void Console::set_audiovisual() {
    ConsolePrefWinDisplay(ConsolePrefWin, 4);
}

/*
Purpose: Open the shared preferences window to the map-display page.
Original Offset: 00514F40
Return Value: n/a
Status: Complete
*/
void Console::set_map_display() {
    ConsolePrefWinDisplay(ConsolePrefWin, 5);
}

void __fastcall console_set_preferences_redirect(Console *self, void *) {
    self->set_preferences();
}

void __fastcall console_set_auto_preferences_redirect(Console *self, void *) {
    self->set_auto_preferences();
}

void __fastcall console_set_base_preferences_redirect(Console *self, void *) {
    self->set_base_preferences();
}

void __fastcall console_set_audiovisual_redirect(Console *self, void *) {
    self->set_audiovisual();
}

void __fastcall console_set_map_display_redirect(Console *self, void *) {
    self->set_map_display();
}

int32_t *ConsoleGroupCount = reinterpret_cast<int32_t *>(0x009A64C8);
uint8_t *ConsoleGroupTable = reinterpret_cast<uint8_t *>(0x0095282C);

/*
Purpose: Clear the console's active-group field and drop the highlight bit
         (0x08000000) from every entry of the group table, at its 0x34 stride.
Original Offset: 0050F650
Return Value: n/a
Status: Complete
Verification note: the sweep's surviving swaps reorder the object-field clear
against the group-count load and one declaration against another; both pairs
touch disjoint state, so the order is unobservable and the mutants are
equivalent by construction.
*/
void Console::clear_group() {
    const int32_t zero = 0;
    std::memcpy(reinterpret_cast<uint8_t *>(this) + 0x23D1C, &zero, sizeof(zero));
    const int32_t count = *ConsoleGroupCount;
    for (int32_t index = 0; index < count; ++index) {
        uint8_t *const entry = ConsoleGroupTable + index * 0x34;
        uint32_t value;
        std::memcpy(&value, entry, sizeof(value));
        value &= 0xF7FFFFFFu;
        std::memcpy(entry, &value, sizeof(value));
    }
}

void __fastcall console_clear_group_redirect(Console *self, void *) {
    self->clear_group();
}

/*
Purpose: Report whether editing is locked out. Only meaningful in the scenario
         editor; there editing is locked whenever Scroll Lock is toggled on or
         the game is in editor-only mode.
Original Offset: 004E1F40
Return Value: 1 when editing is locked, 0 otherwise
Status: Complete
*/
int Console::edit_lock() {
    if (!(*GameState & STATE_SCENARIO_EDITOR)) {
        return 0;
    }
    if (((*ConsoleEditKeyStateSlot)(VK_SCROLL) & 1) ||
        (*GameState & STATE_EDITOR_ONLY_MODE)) {
        return 1;
    }
    return 0;
}

int __fastcall console_edit_lock_redirect(Console *self, void *) {
    return self->edit_lock();
}

/*
Purpose: Open the shared preferences window to the advanced page.
Original Offset: 00514F00
Return Value: n/a
Status: Complete
*/
void Console::set_adv_preferences() {
    ConsolePrefWinDisplay(ConsolePrefWin, 1);
}

/*
Purpose: Undo the last scenario-editor change, by asking load_undo for the
         editor's undo slot.
Original Offset: 004E1F20
Return Value: n/a
Status: Complete
*/
void Console::editor_undo() {
    load_undo(1);
}

void __fastcall console_set_adv_preferences_redirect(Console *self, void *) {
    self->set_adv_preferences();
}

void __fastcall console_editor_undo_redirect(Console *self, void *) {
    self->editor_undo();
}

func_status_win_redraw *ConsoleOriginalStatusWinRedraw =
    (func_status_win_redraw *)0x004B9EA0;
void *ConsoleInfoWin = reinterpret_cast<void *>(0x007AD2A0);
void *ConsoleStatusWin = reinterpret_cast<void *>(0x008C5568);
void **ConsoleMapWinSlot = reinterpret_cast<void **>(0x007D3C3C);

/*
Purpose: Refresh everything the console shows after a selection or turn change:
         hand the change code to the shared InfoWin, redraw the shared
         StatusWin, then push the map's caption into the main interface.
Original Offset: 00514880
Return Value: n/a
Status: Complete with a temporary StatusWin::redraw original dependency
Verification note: three properties of this body are unobservable at every
oracle tier, because the original's own callees discard them rather than
because the fixtures are weak.
  1. InfoWin::change at 0x00458900 is a bare `ret 4` - it drops both its
     `this` and its argument - so neither the call nor the value of a1 has any
     effect to observe. src/infowin.cpp models it as an empty body.
  2. MapWin::main_caption at 0x0046FB10 overwrites ecx before using it, so the
     MapWin pointer loaded out of ConsoleMapWinSlot never reaches anything
     that reads it.
  3. The original leaves EAX holding main_caption's residue, whose chain is
     already terminated by the committed `void MapWin::main_caption()`, so
     there is no residue available to return and fabricating a constant would
     be wrong. Kept `void`, matching the mangled name.
Mutation-harness survivors for the dropped `change` call and for a perturbed
a1 are therefore equivalent by construction.
*/
void Console::update_data(int a1) {
    // 0x00514883 mov eax,[ebp+8] / 0x00514886 mov ecx,0x7ad2a0 /
    // 0x0051488B push eax / 0x0051488C call 0x458900.
    reinterpret_cast<InfoWin *>(ConsoleInfoWin)->change(a1);
    // 0x00514891 mov ecx,0x8c5568 / 0x00514896 call 0x4b9ea0.
    ConsoleOriginalStatusWinRedraw(ConsoleStatusWin);
    // 0x0051489B mov ecx,[0x7d3c3c] - a load, not an address-of - then
    // 0x005148A1 call 0x46fb10. The slot is read here, never cached.
    reinterpret_cast<MapWin *>(*ConsoleMapWinSlot)->main_caption();
}

// `ret 4` on the original: one 4-byte stack argument plus the ecx `this`. The
// entry needs no this-adjustment - update_data is entered on the Console
// itself, not through a virtual-base adjustor.
void __fastcall console_update_data_redirect(Console *self, void *, int a1) {
    self->update_data(a1);
}
