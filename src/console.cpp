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

func_pref_win_display *ConsolePrefWinDisplay = (func_pref_win_display *)0x0048FA00;
void *ConsolePrefWin = reinterpret_cast<void *>(0x008578D8);

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
