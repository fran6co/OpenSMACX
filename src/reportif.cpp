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
#include "reportif.h"

ListBox *ReportIfEnergyListBox = reinterpret_cast<ListBox *>(0x0087BE84);

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004A5FE0
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004A5FF0
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004A6000
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004A6010
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004A6020
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004A6030
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004A6040
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_leave(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004A6D60
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_button_toggled(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004AC130
Return Value: n/a
Status: Complete
*/
void ReportIf::close_score() {
}

void __fastcall report_if_on_iface_right_click_redirect(ReportIf *self, void *, int a1, int a2) {
    self->on_iface_right_click(a1, a2);
}

void __fastcall report_if_on_iface_left_double_click_redirect(ReportIf *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

void __fastcall report_if_on_iface_right_double_click_redirect(ReportIf *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

void __fastcall report_if_on_iface_left_down_redirect(ReportIf *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
}

void __fastcall report_if_on_iface_right_down_redirect(ReportIf *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

void __fastcall report_if_on_iface_mouse_move_redirect(ReportIf *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
}

void __fastcall report_if_on_iface_mouse_leave_redirect(ReportIf *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

void __fastcall report_if_on_iface_button_toggled_redirect(ReportIf *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

void __fastcall report_if_close_score_redirect(ReportIf *self, void *) {
    self->close_score();
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 1.
Original Offset: 004A6320
Return Value: 1
Status: Complete
*/
int ReportIf::on_iface_dialog_item_back_draw(int, int, int, int) {
    return 1;
}

int __fastcall report_if_on_iface_dialog_item_back_draw_redirect(
        ReportIf *self, void *, int a1, int a2, int a3, int a4) {
    return self->on_iface_dialog_item_back_draw(a1, a2, a3, a4);
}

/*
Purpose: Close the two intel list boxes.

             mov esi,ecx / lea ecx,[esi+0xA2D0] / call ListBox::close
                           lea ecx,[esi+0xAE24] / call ListBox::close

         Two subobjects, in that order, and nothing else. ReportIf models no
         fields, so both offsets are documented and raw.

         ListBox::close resolves its own bases through the object's vbtable at
         run time, so each of these has to be a real ListBox carrying a table
         of its own; reaching them by offset is what the original does and is
         what keeps the two independent.
Original Offset: 004AC980
Return Value: n/a
Status: Complete
*/
void ReportIf::close_intel() {
    auto *const self = reinterpret_cast<uint8_t *>(this);
    reinterpret_cast<ListBox *>(self + 0xA2D0)->close();
    reinterpret_cast<ListBox *>(self + 0xAE24)->close();
}

void __fastcall report_if_close_intel_redirect(ReportIf *self, void *) {
    self->close_intel();
}

/*
Purpose: Close the energy report's three list boxes.

             mov ecx,0x87BE84     / call ListBox::close
             lea ecx,[esi+0xA2D0] / call ListBox::close
             lea ecx,[esi+0xAE24] / call ListBox::close

         The last two are the same subobjects close_intel closes, in the same
         order. The FIRST is not a subobject at all - it is a process-wide
         ListBox at a fixed address, and it goes first.

         0x0087BE84 is a new fixed-address DATA binding, named
         ReportIfEnergyListBox in the header so the literal appears once. It
         needs no row in recovery-binding-classifications.csv: that file
         classifies original FUNCTION bindings, and ConsoleGlobal - the same
         shape, a named pointer to a process object - carries none either.
Original Offset: 004A9020
Return Value: n/a
Status: Complete
*/
void ReportIf::close_energy() {
    auto *const self = reinterpret_cast<uint8_t *>(this);
    ReportIfEnergyListBox->close();
    reinterpret_cast<ListBox *>(self + 0xA2D0)->close();
    reinterpret_cast<ListBox *>(self + 0xAE24)->close();
}

void __fastcall report_if_close_energy_redirect(ReportIf *self, void *) {
    self->close_energy();
}

/*
Original Offset: 004A3930
Status: Complete
*/
void ReportIf::done() {
    reinterpret_cast<Win *>(0x00876478)->release_modal();
    reinterpret_cast<SubInterface *>(this)->release_iface_mode();
}
