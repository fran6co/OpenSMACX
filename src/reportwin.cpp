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
#include "reportwin.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004980F0
Return Value: n/a
Status: Complete
*/
void ReportWin::on_mouse_move(int, int) {
}

void __fastcall report_win_on_mouse_move_redirect(ReportWin *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00498100
Return Value: n/a
Status: Complete
*/
void ReportWin::on_mouse_leave(int, int) {
}

void __fastcall report_win_on_mouse_leave_redirect(ReportWin *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}

func_report_win_anim ReportWinSatAnim =
    original_method<func_report_win_anim>(0x0049FE40);
func_report_win_anim ReportWinExpAnim =
    original_method<func_report_win_anim>(0x004A0100);

/*
Purpose: Unknown; one of the two animation ticks the daemons in
         leaf_recoveries.cpp drive.
         Body unrecovered; forwards to the original image.
Forwards To: 0049FE40
Return Value: n/a
Status: Forwarder
*/
void ReportWin::sat_anim() {
    (ORIGINAL(this)->*ReportWinSatAnim)();
}

/*
Purpose: Unknown; one of the two animation ticks the daemons in
         leaf_recoveries.cpp drive.
         Body unrecovered; forwards to the original image.
Forwards To: 004A0100
Return Value: n/a
Status: Forwarder
*/
void ReportWin::exp_anim() {
    (ORIGINAL(this)->*ReportWinExpAnim)();
}
