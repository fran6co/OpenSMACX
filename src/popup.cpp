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
#include "popup.h"

func_base_pop_close *BasePopOriginalClose = (func_base_pop_close *)0x00600F00;

/*
Purpose: Close the popup - its scroll bar first, then the popup base.
Original Offset: 00404900
Return Value: n/a
Status: Complete
*/
void Popup::close() {
    scroll_.close();
    BasePopOriginalClose(this);
}

void __fastcall popup_close_redirect(Popup *self, void *) {
    self->close();
}

func_popup_start_full *PopupOriginalStartFull =
    (func_popup_start_full *)0x00406380;

/*
Purpose: The five-argument start form, forwarding to the six-argument one with
         no parent graphic window.
Original Offset: 0043EF70
Return Value: n/a
Status: Complete
*/
void Popup::start(char *a1, const char *a2, int a3, char *a4, int a5) {
    PopupOriginalStartFull(this, a1, a2, a3, a4, a5, nullptr);
}

void __fastcall popup_start_redirect(Popup *self, void *, char *a1,
                                     const char *a2, int a3, char *a4, int a5) {
    self->start(a1, a2, a3, a4, a5);
}
