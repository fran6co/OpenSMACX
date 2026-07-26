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
#include "wave.h"

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

namespace {

// The Sound-side fields the destructor tears down, viewed at their fixed
// offsets inside the opaque base storage. Ambience sits on the same Sound
// base the waves use: the vtable staging below walks 0x66E538 (its own)
// through 0x66E3C0 (Sound) to the 0x66E444 ultimate base.
struct AmbienceSoundView {
    uint32_t vtable_storage_;       // 0x00
    uint8_t pad_04_[0x38];          // 0x04..0x3B
    void *device_;                  // 0x3C
    uint32_t flags_40_;             // 0x40, bit 1 = chained
    AmbienceSoundView *chain_prev_; // 0x44, toward the chain head
    AmbienceSoundView *chain_next_; // 0x48, toward the chain tail
    void *fname_;                   // 0x4C
};

}  // namespace

/*
Purpose: Destroy an ambience. Its own stage frees the remembered filename
         and releases the device through the guarded hook, clearing both
         fields unconditionally; the inlined Sound stage then repeats the
         same teardown on the now-cleared fields and unlinks from the
         global sound chain when the chained bit is set. The original's
         exception frame is omitted as unreachable. The vtable is staged
         through all three levels.
Original Offset: 004C7670
Return Value: n/a
Status: Complete
*/
Ambience::~Ambience() {
    AmbienceSoundView volatile *const self =
        reinterpret_cast<AmbienceSoundView volatile *>(this);
    self->vtable_storage_ = 0x0066E538;
    {
        void *const name = self->fname_;
        if (name) {
            WaveOperatorDelete(name);
        }
    }
    // One statement: nothing can observe the order of the device fetch and
    // the unconditional name clear, and separate adjacent statements only
    // breed an equivalent swap mutant.
    void *const device = (self->fname_ = nullptr, self->device_);
    if (device && *WaveDeviceReleaseGuard) {
        (*WaveDeviceReleaseSlot)(device);
    }
    self->device_ = nullptr;
    self->vtable_storage_ = 0x0066E3C0;
    {
        void *const name = self->fname_;
        if (name) {
            WaveOperatorDelete(name);
            self->fname_ = nullptr;
        }
    }
    void *const device2 = self->device_;
    if (device2) {
        if (*WaveDeviceReleaseGuard) {
            (*WaveDeviceReleaseSlot)(device2);
        }
        self->device_ = nullptr;
    }
    if (self->flags_40_ & 2) {
        AmbienceSoundView *const prev = self->chain_prev_;
        if (prev) {
            reinterpret_cast<AmbienceSoundView volatile *>(prev)->chain_next_ =
                self->chain_next_;
        } else {
            *WaveChainHead = reinterpret_cast<Wave *>(self->chain_next_);
        }
        AmbienceSoundView *const next = self->chain_next_;
        if (next) {
            reinterpret_cast<AmbienceSoundView volatile *>(next)->chain_prev_ =
                self->chain_prev_;
        } else {
            *WaveChainTail = reinterpret_cast<Wave *>(self->chain_prev_);
        }
        self->chain_next_ = nullptr;
        self->chain_prev_ = nullptr;
        self->flags_40_ &= ~2u;
    }
    self->vtable_storage_ = 0x0066E444;
}

void __fastcall ambience_dtor_redirect(Ambience *self, void *) {
    self->~Ambience();
}
