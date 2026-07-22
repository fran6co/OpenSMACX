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
#include "ambience.h"

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447220
Return Value: n/a
Status: Complete
*/
void FactionAmbience::begin() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447230
Return Value: n/a
Status: Complete
*/
void FactionAmbience::tech() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447240
Return Value: n/a
Status: Complete
*/
void FactionAmbience::terraform() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447250
Return Value: n/a
Status: Complete
*/
void FactionAmbience::production() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447260
Return Value: n/a
Status: Complete
*/
void FactionAmbience::general() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 004472B0
Return Value: n/a
Status: Complete
*/
void FactionAmbience::new_base() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 004472C0
Return Value: n/a
Status: Complete
*/
void FactionAmbience::popup1() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 004472D0
Return Value: n/a
Status: Complete
*/
void FactionAmbience::eot() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 004472E0
Return Value: n/a
Status: Complete
*/
void FactionAmbience::hostility() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 004472F0
Return Value: n/a
Status: Complete
*/
void FactionAmbience::energy_resources() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447300
Return Value: n/a
Status: Complete
*/
void FactionAmbience::base_liberated() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447690
Return Value: n/a
Status: Complete
*/
void UAmbience::tech() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 004476A0
Return Value: n/a
Status: Complete
*/
void UAmbience::popup1() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 004476B0
Return Value: n/a
Status: Complete
*/
void UAmbience::eot() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447C60
Return Value: n/a
Status: Complete
*/
void GAmbience::tech() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447CA0
Return Value: n/a
Status: Complete
*/
void GAmbience::production() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447CB0
Return Value: n/a
Status: Complete
*/
void GAmbience::popup1() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
Original Offset: 00447CC0
Return Value: n/a
Status: Complete
*/
void GAmbience::eot() {
}

void __fastcall faction_ambience_begin_redirect(FactionAmbience *self, void *) {
    self->begin();
}

void __fastcall faction_ambience_tech_redirect(FactionAmbience *self, void *) {
    self->tech();
}

void __fastcall faction_ambience_terraform_redirect(FactionAmbience *self, void *) {
    self->terraform();
}

void __fastcall faction_ambience_production_redirect(FactionAmbience *self, void *) {
    self->production();
}

void __fastcall faction_ambience_general_redirect(FactionAmbience *self, void *) {
    self->general();
}

void __fastcall faction_ambience_new_base_redirect(FactionAmbience *self, void *) {
    self->new_base();
}

void __fastcall faction_ambience_popup1_redirect(FactionAmbience *self, void *) {
    self->popup1();
}

void __fastcall faction_ambience_eot_redirect(FactionAmbience *self, void *) {
    self->eot();
}

void __fastcall faction_ambience_hostility_redirect(FactionAmbience *self, void *) {
    self->hostility();
}

void __fastcall faction_ambience_energy_resources_redirect(FactionAmbience *self, void *) {
    self->energy_resources();
}

void __fastcall faction_ambience_base_liberated_redirect(FactionAmbience *self, void *) {
    self->base_liberated();
}

void __fastcall u_ambience_tech_redirect(UAmbience *self, void *) {
    self->tech();
}

void __fastcall u_ambience_popup1_redirect(UAmbience *self, void *) {
    self->popup1();
}

void __fastcall u_ambience_eot_redirect(UAmbience *self, void *) {
    self->eot();
}

void __fastcall g_ambience_tech_redirect(GAmbience *self, void *) {
    self->tech();
}

void __fastcall g_ambience_production_redirect(GAmbience *self, void *) {
    self->production();
}

void __fastcall g_ambience_popup1_redirect(GAmbience *self, void *) {
    self->popup1();
}

void __fastcall g_ambience_eot_redirect(GAmbience *self, void *) {
    self->eot();
}
