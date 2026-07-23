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

char *PopupStartCaption = reinterpret_cast<char *>(0x009B8AA8);

/*
Purpose: Start a popup from just a label, defaulting the rest.
Original Offset: 005A5990
Return Value: n/a
Status: Complete
*/
void Popup::start(const char *label) {
    PopupOriginalStartFull(this, PopupStartCaption, label, -1, nullptr, 0,
                           nullptr);
}

/*
Purpose: Start a popup from a label and a value, defaulting the rest.
Original Offset: 00559040
Return Value: n/a
Status: Complete
*/
void Popup::start(const char *label, int value) {
    PopupOriginalStartFull(this, PopupStartCaption, label, -1, nullptr, value,
                           nullptr);
}

void __fastcall popup_start_label_redirect(Popup *self, void *, const char *label) {
    self->start(label);
}

void __fastcall popup_start_label_value_redirect(Popup *self, void *,
                                                 const char *label, int value) {
    self->start(label, value);
}

/*
Purpose: Draw a popup's dialog background; the legacy implementation does
         nothing and returns 0.
Original Offset: 00405B10
Return Value: 0, always
Status: Complete
*/
int Popup::on_dialog_back_draw(::GraphicWin *) {
    return 0;
}

int __fastcall popup_on_dialog_back_draw_redirect(Popup *self, void *, ::GraphicWin *window) {
    return self->on_dialog_back_draw(window);
}
