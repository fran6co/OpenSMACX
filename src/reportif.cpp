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
ORIGINAL: 0x004A5FE0 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004A5FF0 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004A6000 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004A6010 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_left_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004A6020 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_right_down(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004A6030 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004A6040 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_mouse_leave(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004A6D60 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::on_iface_button_toggled(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x004AC130 BYTE_EXACT
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
ORIGINAL: 0x004A6320 BYTE_EXACT
Return Value: 1
Status: Complete
*/
int ReportIf::on_iface_dialog_item_back_draw(::GraphicWin *, int, int, RECT *) {
    return 1;
}

int __fastcall report_if_on_iface_dialog_item_back_draw_redirect(
        ReportIf *self, void *, ::GraphicWin *a1, int a2, int a3, RECT *a4) {
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
ORIGINAL: 0x004AC980 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void ReportIf::close_intel() {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
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
ORIGINAL: 0x004A9020
Return Value: n/a
Status: Complete
*/
void ReportIf::close_energy() {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    ReportIfEnergyListBox->close();
    reinterpret_cast<ListBox *>(self + 0xA2D0)->close();
    reinterpret_cast<ListBox *>(self + 0xAE24)->close();
}

void __fastcall report_if_close_energy_redirect(ReportIf *self, void *) {
    self->close_energy();
}

/*
ORIGINAL: 0x004A3930 BYTE_EXACT
Status: Complete
*/
void ReportIf::done() {
    reinterpret_cast<Win *>(0x00876478)->release_modal();
    reinterpret_cast<SubInterface *>(this)->release_iface_mode();
}

// The seams and definitions for the unrecovered bodies, at the end of the file
// so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness, and being defined after the
// caller is NOT enough on its own: VC6 at /Ob2 defers codegen to the end of
// the translation unit and folds a forwarder defined later back into a caller
// defined earlier. Measured - without the pragma cl reports C4711 at
// reportif.cpp(219), which is the release_iface_mode call inside ReportIf::done,
// and done's 22-byte BYTE_EXACT match is a `call rel32` there.
#pragma auto_inline(off)
func_report_if_bl_anim ReportIfBlAnim =
    original_method<func_report_if_bl_anim>(0x004A4060);

/*
Purpose: Step the report interface's blink animation. The body at 0x004A4060 is
         NOT recovered; this is a seam to the original image, not a recovery,
         and deliberately carries no `Original Offset:` line so the catalogue
         does not mistake it for one.
Status: Forwarded to the original image
*/
void ReportIf::bl_anim() {
    (ORIGINAL(this)->*ReportIfBlAnim)();
}

/*
 * SubInterface's two called methods, hosted here for the reason spelled out in
 * subinterface.h: that class has no translation unit of its own, and this is
 * the file that already sees the header and already calls one of them.
 */
func_sub_interface_iface_mode SubInterfaceSetIfaceMode =
    original_method<func_sub_interface_iface_mode>(0x0045D310);
func_sub_interface_iface_mode SubInterfaceReleaseIfaceMode =
    original_method<func_sub_interface_iface_mode>(0x0045D380);

/*
Purpose: Make this interface the active one. The body at 0x0045D310 is NOT
         recovered; this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void SubInterface::set_iface_mode() {
    (ORIGINAL(this)->*SubInterfaceSetIfaceMode)();
}

/*
Purpose: Give up the active interface mode. The body at 0x0045D380 is NOT
         recovered; this is a seam to the original image, not a recovery, and
         deliberately carries no `Original Offset:` line so the catalogue does
         not mistake it for one.
Status: Forwarded to the original image
*/
void SubInterface::release_iface_mode() {
    (ORIGINAL(this)->*SubInterfaceReleaseIfaceMode)();
}
#pragma auto_inline(on)
