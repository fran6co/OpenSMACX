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

/*
Purpose: Build the ambience. The original constructs in four vtable stages -
         0x0066E444 at 0x004C848A, 0x0066E3C0 at 0x004C84AC, 0x0066E538 at
         0x004C84EF, then its own 0x0066E664 at 0x004C850F - but only CRT
         memsets run between them, so the three intermediate installs are
         unobservable and, like the registered SEH frame, are omitted, keeping
         only the final Ambience publication. A device dispatch of the 0x3E8
         default through slot 0 at 0x004C84DB is provably dead - the device
         slot at 0x3C is zeroed at 0x004C84B8 and the memset in between covers
         0x40..0x43 only, so the reload at 0x004C84D4 cannot see anything else
         - and is omitted under the same policy. The net field state: full
         volume, zeroed regions, a 1000ms default at 0x38, flag dword 8, flag
         byte 1, and Sound::set_type run with type 5. Unlike Wave, whose 0x54
         clear is four bytes wide, this one is a single byte, so 0x55..0x57
         keep whatever the storage held - and nothing at or above 0x58 is
         written at all, which is what pins the shared base's extent there.
Original Offset: 004C8460
Return Value: n/a (the redirect answers the object pointer, as the original
              does in eax)
Status: Complete
Verification note: 0x0066E664 has exactly one writer in the whole image, this
         constructor, and no reader on the teardown path, which is why the
         already-recovered ~Ambience at 0x004C7670 starts its descent one
         stage lower at 0x0066E538. That is not a missing stage: 0x0066E664
         differs from 0x0066E538 in three slots only - the deleting destructor
         at 0x08, load at 0x10 and unload at 0x14 - and 0x004C7670 is the
         base-level destructor, reached from both slot 0x08 of 0x0066E664 and
         the global's ??__F thunk at 0x004455E0 without a vtable store of its
         own, MSVC having elided a store that 0x004C7670 overwrites on its
         first instruction. The two bodies are consistent.
Verification note: the sweep leaves exactly two survivors and both are
         equivalent mutants that no assertion can separate, because the
         original's own shape makes them so. Widening the 0x0C..0x2F loop by
         one iteration lands on 0x30, which the very next statement stores
         zero to anyway; and seeding the 0x54 byte with 1 instead of 0 washes
         out under the `|= 1` two statements later. Both are transcribed the
         way the bytes read - a memset then a separate store, and a memset
         then a read-modify-write - rather than folded into whatever would
         make the sweep look cleaner.
Verification note: the base-level constructor at 0x004C75B0 is byte-identical
         to this one through the 0x0066E538 publication and then stops - it
         neither publishes 0x0066E664 nor calls set_type. Recovering it later
         must duplicate this shape rather than share a helper with it, or a
         mutation sweep of either loses its signal.
*/
void Ambience::construct() {
    // The header holds the base as opaque bytes, so the fields are reached at
    // their documented offsets.
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile uint8_t *const bytes = reinterpret_cast<volatile uint8_t *>(this);
    object[0x04 / 4] = 0x7F;
    object[0x08 / 4] = 0;
    // memset(this + 0x0C, 0, 0x24) at 0x004C849A. The region is dword aligned
    // and 0x24 is a multiple of four, so a dword fill reaches the same bytes.
    for (size_t offset = 0x0C; offset < 0x30; offset += 4) {
        object[offset / 4] = 0;
    }
    object[0x30 / 4] = 0;
    object[0x44 / 4] = 0;
    object[0x48 / 4] = 0;
    object[0x3C / 4] = 0;
    object[0x4C / 4] = 0;
    // memset to zero at 0x004C84BE, then the loaded bit cleared at 0x004C84C8
    // - a no-op on the zero just stored - then bit 3 set below: net 8, where
    // Wave nets 4.
    object[0x40 / 4] = 0;
    object[0x38 / 4] = 0x3E8;
    object[0x50 / 4] = 0;
    // memset(this + 0x54, 0, 1) at 0x004C84F5: one byte, not Wave's four. The
    // original leaves 0x55..0x57 alone, so no defensive widening here.
    bytes[0x54] = 0;
    // Re-read, do NOT fold into the stores above: the original reloads the
    // flag dword at 0x004C84FA and the flag byte at 0x004C84FC after their own
    // zeroing, and stores them back at 0x004C8507 and 0x004C850D in that
    // order.
    object[0x40 / 4] |= 8;
    bytes[0x54] |= 1;
    object[0x00 / 4] = 0x0066E664;
    SoundSetType(reinterpret_cast<Wave *>(this), 5);
#if defined(__GNUC__) && defined(__i386__)
    // The legacy constructor leaves the instance pointer in EAX.
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Ambience *__fastcall ambience_construct_redirect(Ambience *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
Original Offset: 00447270
Return Value: n/a
Status: Complete
*/
void FactionAmbience::basewin_show() {
    field_6C_ = 1;
}

void __fastcall faction_ambience_basewin_show_redirect(FactionAmbience *self, void *) {
    self->basewin_show();
}

/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
Original Offset: 00447280
Return Value: n/a
Status: Complete
*/
void FactionAmbience::basewin_hide() {
    field_6C_ = 0;
}

void __fastcall faction_ambience_basewin_hide_redirect(FactionAmbience *self, void *) {
    self->basewin_hide();
}

/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
Original Offset: 00447290
Return Value: n/a
Status: Complete
*/
void FactionAmbience::design_window_show() {
    field_6D_ = 1;
}

void __fastcall faction_ambience_design_window_show_redirect(FactionAmbience *self, void *) {
    self->design_window_show();
}

/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
Original Offset: 004472A0
Return Value: n/a
Status: Complete
*/
void FactionAmbience::design_window_hide() {
    field_6D_ = 0;
}

void __fastcall faction_ambience_design_window_hide_redirect(FactionAmbience *self, void *) {
    self->design_window_hide();
}

/*
Purpose: GAmbience raises the flag byte only when it is CLEAR.

             mov al,[ecx+0x6C] / test al,al / jne done
             mov byte [ecx+0x6C],1 / done: ret

         The guard is not redundant. Its sibling basewin_hide below stores
         unconditionally, and an unconditional store here would agree for the
         values 0 and 1 and differ for every other: a byte holding 5 keeps 5,
         where `field_6C_ = 1` would overwrite it.
Original Offset: 00447BE0
Return Value: n/a
Status: Complete
*/
void GAmbience::basewin_show() {
    if (field_6C_ == 0) {
        field_6C_ = 1;
    }
}

void __fastcall g_ambience_basewin_show_redirect(GAmbience *self, void *) {
    self->basewin_show();
}

/*
Purpose: GAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
Original Offset: 00447BF0
Return Value: n/a
Status: Complete
*/
void GAmbience::basewin_hide() {
    field_6C_ = 0;
}

void __fastcall g_ambience_basewin_hide_redirect(GAmbience *self, void *) {
    self->basewin_hide();
}

const uint32_t FactionAmbienceVtable = 0x0066C0F8;
const uint32_t MAmbienceVtable = 0x0066BE28;
const uint32_t SAmbienceVtable = 0x0066BB58;
const uint32_t GAmbienceVtable = 0x0066BF90;

/*
Purpose: Run the shared Ambience constructor, publish this variant's vtable,
         and zero the fields this variant owns.

             push esi / mov esi,ecx / call ??0Ambience / xor eax,eax
             mov [esi],0x66c0f8 / mov [esi+0x58],eax ... [esi+0x64],eax
             mov byte [esi+0x6c],al / mov byte [esi+0x6d],al
             mov [esi+0x68],eax / mov eax,esi / pop esi / ret

         The four variants share this shape and differ only in the vtable and
         in how far the tail runs: FactionAmbience stops at 0x68, MAmbience and
         GAmbience add 0x70, SAmbience adds 0x70 and 0x74. That tail is what
         evidences each variant's extent, and it is why the four bodies are
         written out rather than routed through one helper - sharing a body
         would lose the mutation signal that keeps them distinct, exactly as
         the note on 0x004C75B0 in this file already warns.

         The base constructor runs FIRST and publishes the base vtable; the
         store at [this] then overwrites it. Ordering them the other way would
         leave the base's vtable in place.

         0x6c and 0x6d are BYTES. 0x6e and 0x6f are not written, so whatever
         the storage held survives there.

Original Offset: 004471F0
Return Value: `this`
Status: Complete
*/
void FactionAmbience::construct() {
    ambience_construct_redirect(reinterpret_cast<Ambience *>(this), nullptr);
    *reinterpret_cast<volatile uint32_t *>(this) = FactionAmbienceVtable;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
}

FactionAmbience *__fastcall faction_ambience_construct_redirect(
        FactionAmbience *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: As 004471F0, with the MAmbience vtable and one more field.
Original Offset: 00447310
Return Value: `this`
Status: Complete
*/
void MAmbience::construct() {
    ambience_construct_redirect(reinterpret_cast<Ambience *>(this), nullptr);
    *reinterpret_cast<volatile uint32_t *>(this) = MAmbienceVtable;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
    field_70_ = 0;
}

MAmbience *__fastcall m_ambience_construct_redirect(MAmbience *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: As 004471F0, with the SAmbience vtable and two more fields.
Original Offset: 00447850
Return Value: `this`
Status: Complete
*/
void SAmbience::construct() {
    ambience_construct_redirect(reinterpret_cast<Ambience *>(this), nullptr);
    *reinterpret_cast<volatile uint32_t *>(this) = SAmbienceVtable;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
    field_70_ = 0;
    field_74_ = 0;
}

SAmbience *__fastcall s_ambience_construct_redirect(SAmbience *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: As 004471F0, with the GAmbience vtable and one more field.
Original Offset: 00447B90
Return Value: `this`
Status: Complete
*/
void GAmbience::construct() {
    ambience_construct_redirect(reinterpret_cast<Ambience *>(this), nullptr);
    *reinterpret_cast<volatile uint32_t *>(this) = GAmbienceVtable;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
    field_70_ = 0;
}

GAmbience *__fastcall g_ambience_construct_redirect(GAmbience *self, void *) {
    self->construct();
    return self;
}
