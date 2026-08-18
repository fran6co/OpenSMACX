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
#include "original_seam.h"
#include "datalink.h"
// Datalink::show and Datalink::hide reach the SubInterface subobject the
// original places at +0xA14 by casting a hand-computed address, so the type
// must be complete.
#include "subinterface.h"

func_datalink_exec DatalinkExec = original_method<func_datalink_exec>(0x00429180);
// Unclassified data seam: the Datalink singleton object, not a call target.
void *DatalinkMain = reinterpret_cast<void *>(0x00703EA0);

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0042BEA0 ?UNK6@Datalink@@QAEXXZ 0x0042BEA0-0x0042BEA1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0042BF00 ?UNK8@Datalink@@QAEXXZ 0x0042BF00-0x0042BF01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK8@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::UNK8() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00431DB0 ?UNK9@Datalink@@QAEXXZ 0x00431DB0-0x00431DB1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK9@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00431EB0 ?on_left_click@Datalink@@QAEXHH@Z 0x00431EB0-0x00431EB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00431FB0 ?on_group_clicked@Datalink@@QAEXHH@Z 0x00431FB0-0x00431FB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_group_clicked@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00432000 ?on_mouse_move@Datalink@@QAEXHH@Z 0x00432000-0x00432003 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00432160 ?on_iface_left_click@Datalink@@QAEXHH@Z 0x00432160-0x00432163 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00432170 ?on_iface_right_click@Datalink@@QAEXHH@Z 0x00432170-0x00432173 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00432180 ?on_iface_left_down@Datalink@@QAEXHH@Z 0x00432180-0x00432183 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00432190 ?on_iface_right_down@Datalink@@QAEXHH@Z 0x00432190-0x00432193 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004321A0 ?on_iface_mouse_move@Datalink@@QAEXHH@Z 0x004321A0-0x004321A3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004321B0 ?on_iface_mouse_leave@Datalink@@QAEXHH@Z 0x004321B0-0x004321B3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004321C0 ?on_iface_selected@Datalink@@QAEXHH@Z 0x004321C0-0x004321C3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004321D0 ?on_iface_left_double_click@Datalink@@QAEXHH@Z 0x004321D0-0x004321D3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004321E0 ?on_iface_right_double_click@Datalink@@QAEXHH@Z 0x004321E0-0x004321E3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004321F0 ?on_iface_button_clicked@Datalink@@QAEXH@Z 0x004321F0-0x004321F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_clicked@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00432200 ?on_iface_button_toggled@Datalink@@QAEXHH@Z 0x00432200-0x00432203 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x0042BD20 ?close@Datalink@@QAEXXZ 0x0042BD20-0x0042BD35 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?close@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Datalink::close() {
    facilityID_ = 0;
    field_2A34_ = 0;
    field_2A38_ = 0;
}

void __fastcall datalink_close_redirect(Datalink *self, void *) {
    self->close();
}

/*
Purpose: Show the technology help topic for the given id.
// ORIGINAL: 0x0044C880 ?help_tech@@YAXH@Z 0x0044C880-0x0044C895
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_tech(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0xE, id);
}

/*
Purpose: Show the weapon help topic for the given id.
// ORIGINAL: 0x0044C910 ?help_weapon@@YAXH@Z 0x0044C910-0x0044C925
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_weapon(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0x6, id);
}

/*
Purpose: Show the armor help topic for the given id.
// ORIGINAL: 0x0044C940 ?help_armor@@YAXH@Z 0x0044C940-0x0044C955
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_armor(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0x7, id);
}

/*
Purpose: Show the chassis help topic for the given id.
// ORIGINAL: 0x0044C980 ?help_chassis@@YAXH@Z 0x0044C980-0x0044C995
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_chassis(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0x4, id);
}

/*
Purpose: Show the facility help topic for the given id.
// ORIGINAL: 0x0044C9B0 ?help_facility@@YAXH@Z 0x0044C9B0-0x0044C9C5
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_facility(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0xA, id);
}

/*
Purpose: Show the secret project/ability help topic for the given id.
// ORIGINAL: 0x0044CA10 ?help_abil@@YAXH@Z 0x0044CA10-0x0044CA25
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_abil(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0x8, id);
}

/*
Purpose: Show the social engineering help topic for the given id.
// ORIGINAL: 0x0044CA40 ?help_social@@YAXH@Z 0x0044CA40-0x0044CA55
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_social(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0xC, id);
}

/*
Purpose: Show the faction help topic for the given id.
// ORIGINAL: 0x0044CA70 ?help_faction@@YAXH@Z 0x0044CA70-0x0044CA85
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_faction(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0xF, id);
}

/*
Purpose: Show the vehicle help topic for the given id.
// ORIGINAL: 0x0044CAD0 ?help_veh@@YAXH@Z 0x0044CAD0-0x0044CAE5
// size      21 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_veh(int id) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(0x3, id);
}

/*
Purpose: Show an arbitrary help topic/index pair, for callers that already
         hold the topic id rather than going through one of the typed
         help_* wrappers above.
// ORIGINAL: 0x0044CB60 ?help_topic@@YAXIH@Z 0x0044CB60-0x0044CB77
// size      23 bytes
// prototype void (__cdecl ?help_topic@@YAXIH@Z)(DatalinkID id, int entryID)
// callers   9   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00429180
Return Value: n/a
Status: Complete
*/
void __cdecl help_topic(unsigned int topic, int index) {
    (ORIGINAL(DatalinkMain)->*DatalinkExec)(topic, index);
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
// ORIGINAL: 0x0042A020 ?UNK1@Datalink@@QAEHHH@Z 0x0042A020-0x0042A03E BYTE_EXACT
// size      30 bytes
// prototype int (__thiscall ?UNK1@Datalink@@QAEHHH@Z)(Datalink* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x0042A040 ?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z 0x0042A040-0x0042A076 BYTE_EXACT
// size      54 bytes
// prototype int (__thiscall ?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z)(Datalink* this, int, DatalinkID*, int*)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
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

/*
// ORIGINAL: 0x00432040 ?on_selected@Datalink@@QAEXH@Z 0x00432040-0x00432052 BYTE_EXACT
// size      18 bytes
// prototype void (__thiscall ?on_selected@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0042BF10
Status: Complete
*/
void Datalink::on_selected(int a1) {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0x2a38);
    if (v == 0) {
        draw_entry();
    }
}

/*
// ORIGINAL: 0x0042BD40 ?show@Datalink@@QAEXH@Z 0x0042BD40-0x0042BD5B BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?show@Datalink@@QAEXH@Z)(Datalink* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void Datalink::show(int a1) {
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(reinterpret_cast<char *>(this) + 0xa14)->set_iface_mode();
    }
}

/*
// ORIGINAL: 0x0042BD60 ?hide@Datalink@@QAEXXZ 0x0042BD60-0x0042BD79 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?hide@Datalink@@QAEXXZ)(Datalink* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D380 0x005F7E90
Status: Complete
*/
void Datalink::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(reinterpret_cast<char *>(this) + 0xa14)->release_iface_mode();
    }
}

func_datalink_draw_entry DatalinkDrawEntry =
    original_method<func_datalink_draw_entry>(0x0042BF10);

/*
 * A forwarder, not a recovery. on_selected above reaches draw_entry with a
 * direct `call rel32`, so the DLL must resolve the symbol, but the body at
 * 0x0042BF10 is not decoded yet. No `Original Offset:` line by design - that
 * annotation marks a recovered body and is indexed by address, so claiming it
 * here would aim the census at a seam.
 */
void Datalink::draw_entry() {
    (ORIGINAL(this)->*DatalinkDrawEntry)();
}
