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
#include "datalink.h"

func_datalink_exec *DatalinkExec = (func_datalink_exec *)0x00429180;
// Unclassified data seam: the Datalink singleton object, not a call target.
void *DatalinkMain = reinterpret_cast<void *>(0x00703EA0);

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0042BEA0
Return Value: n/a
Status: Complete
*/
void Datalink::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0042BF00
Return Value: n/a
Status: Complete
*/
void Datalink::UNK8() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00431DB0
Return Value: n/a
Status: Complete
*/
void Datalink::UNK9() {
}

void __fastcall datalink_unk6_redirect(Datalink *self, void *) {
    self->UNK6();
}

void __fastcall datalink_unk8_redirect(Datalink *self, void *) {
    self->UNK8();
}

void __fastcall datalink_unk9_redirect(Datalink *self, void *) {
    self->UNK9();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00431EB0
Return Value: n/a
Status: Complete
*/
void Datalink::on_left_click(int, int) {
}

void __fastcall datalink_on_left_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00431FB0
Return Value: n/a
Status: Complete
*/
void Datalink::on_group_clicked(int, int) {
}

void __fastcall datalink_on_group_clicked_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_group_clicked(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432000
Return Value: n/a
Status: Complete
*/
void Datalink::on_mouse_move(int, int) {
}

void __fastcall datalink_on_mouse_move_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432160
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_click(int, int) {
}

void __fastcall datalink_on_iface_left_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432170
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_click(int, int) {
}

void __fastcall datalink_on_iface_right_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_right_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432180
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_down(int, int) {
}

void __fastcall datalink_on_iface_left_down_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_left_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432190
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_down(int, int) {
}

void __fastcall datalink_on_iface_right_down_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321A0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_mouse_move(int, int) {
}

void __fastcall datalink_on_iface_mouse_move_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321B0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_mouse_leave(int, int) {
}

void __fastcall datalink_on_iface_mouse_leave_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321C0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_selected(int, int) {
}

void __fastcall datalink_on_iface_selected_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321D0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_left_double_click(int, int) {
}

void __fastcall datalink_on_iface_left_double_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_left_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321E0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_right_double_click(int, int) {
}

void __fastcall datalink_on_iface_right_double_click_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_right_double_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004321F0
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_button_clicked(int) {
}

void __fastcall datalink_on_iface_button_clicked_redirect(Datalink *self, void *, int a1) {
    self->on_iface_button_clicked(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00432200
Return Value: n/a
Status: Complete
*/
void Datalink::on_iface_button_toggled(int, int) {
}

void __fastcall datalink_on_iface_button_toggled_redirect(Datalink *self, void *, int a1, int a2) {
    self->on_iface_button_toggled(a1, a2);
}

/*
Purpose: Close the datalink display, clearing the three values it caches.
Original Offset: 0042BD20
Return Value: n/a
Status: Complete
*/
void Datalink::close() {
    field_29E0_ = 0;
    field_2A34_ = 0;
    field_2A38_ = 0;
}

void __fastcall datalink_close_redirect(Datalink *self, void *) {
    self->close();
}

/*
Purpose: Show the technology help topic for the given id.
Original Offset: 0044C880
Return Value: n/a
Status: Complete
*/
void __cdecl help_tech(int id) {
    DatalinkExec(DatalinkMain, 0xE, id);
}

/*
Purpose: Show the weapon help topic for the given id.
Original Offset: 0044C910
Return Value: n/a
Status: Complete
*/
void __cdecl help_weapon(int id) {
    DatalinkExec(DatalinkMain, 0x6, id);
}

/*
Purpose: Show the armor help topic for the given id.
Original Offset: 0044C940
Return Value: n/a
Status: Complete
*/
void __cdecl help_armor(int id) {
    DatalinkExec(DatalinkMain, 0x7, id);
}

/*
Purpose: Show the chassis help topic for the given id.
Original Offset: 0044C980
Return Value: n/a
Status: Complete
*/
void __cdecl help_chassis(int id) {
    DatalinkExec(DatalinkMain, 0x4, id);
}

/*
Purpose: Show the facility help topic for the given id.
Original Offset: 0044C9B0
Return Value: n/a
Status: Complete
*/
void __cdecl help_facility(int id) {
    DatalinkExec(DatalinkMain, 0xA, id);
}

/*
Purpose: Show the secret project/ability help topic for the given id.
Original Offset: 0044CA10
Return Value: n/a
Status: Complete
*/
void __cdecl help_abil(int id) {
    DatalinkExec(DatalinkMain, 0x8, id);
}

/*
Purpose: Show the social engineering help topic for the given id.
Original Offset: 0044CA40
Return Value: n/a
Status: Complete
*/
void __cdecl help_social(int id) {
    DatalinkExec(DatalinkMain, 0xC, id);
}

/*
Purpose: Show the faction help topic for the given id.
Original Offset: 0044CA70
Return Value: n/a
Status: Complete
*/
void __cdecl help_faction(int id) {
    DatalinkExec(DatalinkMain, 0xF, id);
}

/*
Purpose: Show the vehicle help topic for the given id.
Original Offset: 0044CAD0
Return Value: n/a
Status: Complete
*/
void __cdecl help_veh(int id) {
    DatalinkExec(DatalinkMain, 0x3, id);
}

/*
Purpose: Show an arbitrary help topic/index pair, for callers that already
         hold the topic id rather than going through one of the typed
         help_* wrappers above.
Original Offset: 0044CB60
Return Value: n/a
Status: Complete
*/
void __cdecl help_topic(unsigned int topic, int index) {
    DatalinkExec(DatalinkMain, topic, index);
}

/*
Purpose: Combine two values as `a1 * 10000 + a2`.

             lea eax,[eax+eax*4]  x4   -> a1 * 625
             shl eax,4                 -> a1 * 10000
             add eax,ecx

         The four `lea` steps multiply by five each time and the shift by
         sixteen, which is 625 * 16 = 10000. Written as the constant, because
         the shift-and-add chain is the compiler's encoding of it and not a
         behaviour of its own - it wraps identically at 32 bits either way.

         Touches no field; both operands are arguments.
Original Offset: 0042A020
Return Value: a1 * 10000 + a2, wrapping at 32 bits
Status: Complete
*/
int Datalink::UNK1(int a1, int a2) {
    return static_cast<int32_t>(static_cast<uint32_t>(a1) * 10000U
                                + static_cast<uint32_t>(a2));
}

int __fastcall datalink_unk1_redirect(Datalink *self, void *, int a1, int a2) {
    return self->UNK1(a1, a2);
}

/*
Purpose: Split a combined id into its two halves - `id / 10000` into the first
         output and `id % 10000` into the second.

             mov eax,0x68DB8BAD / imul ecx / sar edx,0xC
             mov eax,edx / shr eax,0x1F / add edx,eax   ; id / 10000
             mov [eax],edx
             lea .. x4 / shl edx,4 / sub ecx,edx        ; id - quotient*10000
             mov [eax],ecx

         The magic multiply is a signed divide by 10000, checked against C++
         truncating division over 200,000 random dividends and the boundaries,
         and the remainder is formed as `id - quotient * 10000` rather than by
         a second divide - which is what C++ `%` means for truncating division,
         verified the same way.

         This is the exact INVERSE of UNK1 above, which builds `a1 * 10000 + a2`
         from a shift-and-add chain. The two were decoded independently and
         agree on the constant.

         Neither output is null-checked, because the original checks neither.
Original Offset: 0042A040
Return Value: n/a
Status: Complete
*/
void Datalink::parse_id(int id, DatalinkID *out_id, int *out_remainder) {
    const int32_t quotient = id / 10000;
    // Only the first four bytes of the DatalinkID are written; the type stays
    // incomplete, so the store is expressed against its address.
    *reinterpret_cast<int32_t *>(out_id) = quotient;
    *out_remainder = static_cast<int32_t>(
        static_cast<uint32_t>(id) - static_cast<uint32_t>(quotient) * 10000U);
}

void __fastcall datalink_parse_id_redirect(Datalink *self, void *, int id,
                                           DatalinkID *out_id,
                                           int *out_remainder) {
    self->parse_id(id, out_id, out_remainder);
}
