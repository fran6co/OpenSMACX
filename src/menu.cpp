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
#include "menu.h"

/*
Purpose: Set the callback invoked for menu events.
Original Offset: 005FB820
Status: Complete
*/
void Menu::set_menu_proc(MenuProc proc) {
    proc_ = proc;
}

/*
Purpose: Find the first menu entry with the requested ID.
Original Offset: 005FB990
Status: Complete
*/
int Menu::id_to_index(int id) {
    for (int index = 0; index < 15; ++index) {
        if (entries_[index].id == -1) {
            break;
        }
        if (entries_[index].id == id) {
            return index;
        }
    }
    return -1;
}

MenuProc __fastcall menu_set_menu_proc_redirect(
        Menu *self, void *, MenuProc proc) {
    self->set_menu_proc(proc);
    return proc;
}

int __fastcall menu_id_to_index_redirect(Menu *self, void *, int id) {
    return self->id_to_index(id);
}
