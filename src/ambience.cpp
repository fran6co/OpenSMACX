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
#include "ambience.h"
#include "sound.h"   // Sound::set_type, reached by cast
#include "wave.h"

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447220 ?begin@FactionAmbience@@QAEXXZ 0x00447220-0x00447221 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?begin@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::begin() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447230 ?tech@FactionAmbience@@QAEXXZ 0x00447230-0x00447231 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?tech@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::tech() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447240 ?terraform@FactionAmbience@@QAEXXZ 0x00447240-0x00447241 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?terraform@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::terraform() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447250 ?production@FactionAmbience@@QAEXXZ 0x00447250-0x00447251 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?production@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::production() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447260 ?general@FactionAmbience@@QAEXXZ 0x00447260-0x00447261 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?general@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::general() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472B0 ?new_base@FactionAmbience@@QAEXXZ 0x004472B0-0x004472B1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?new_base@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::new_base() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472C0 ?popup1@FactionAmbience@@QAEXXZ 0x004472C0-0x004472C1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?popup1@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::popup1() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472D0 ?eot@FactionAmbience@@QAEXXZ 0x004472D0-0x004472D1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?eot@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::eot() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472E0 ?hostility@FactionAmbience@@QAEXXZ 0x004472E0-0x004472E1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?hostility@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::hostility() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472F0 ?energy_resources@FactionAmbience@@QAEXXZ 0x004472F0-0x004472F1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?energy_resources@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::energy_resources() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447300 ?base_liberated@FactionAmbience@@QAEXXZ 0x00447300-0x00447301 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?base_liberated@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::base_liberated() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447690 ?tech@UAmbience@@QAEXXZ 0x00447690-0x00447691 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?tech@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::tech() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004476A0 ?popup1@UAmbience@@QAEXXZ 0x004476A0-0x004476A1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?popup1@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::popup1() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004476B0 ?eot@UAmbience@@QAEXXZ 0x004476B0-0x004476B1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?eot@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::eot() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447C60 ?tech@GAmbience@@QAEXXZ 0x00447C60-0x00447C61 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?tech@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::tech() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CA0 ?production@GAmbience@@QAEXXZ 0x00447CA0-0x00447CA1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?production@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::production() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CB0 ?popup1@GAmbience@@QAEXXZ 0x00447CB0-0x00447CB1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?popup1@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::popup1() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CC0 ?eot@GAmbience@@QAEXXZ 0x00447CC0-0x00447CC1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?eot@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::eot() {
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
// ORIGINAL: 0x004C7670 ??1Ambience@@QAE@XZ 0x004C7670-0x004C7760;0x004C8450-0x004C8457;0x00659F32-0x00659F4C
// size      273 bytes
// prototype void (__thiscall ??1Ambience@@QAE@XZ)(Ambience* this)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064557F
// indirect  0x004C76BE 0x004C76FA
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
            operator delete(name);
        }
    }
    // One statement: nothing can observe the order of the device fetch and
    // the unconditional name clear, and separate adjacent statements only
    // breed an equivalent swap mutant.
    void *const device = (self->fname_ = nullptr, self->device_);
    if (device && *WaveDeviceReleaseGuard) {
        (WaveDeviceReleaseSlot())(device);
    }
    self->device_ = nullptr;
    self->vtable_storage_ = 0x0066E3C0;
    {
        void *const name = self->fname_;
        if (name) {
            operator delete(name);
            self->fname_ = nullptr;
        }
    }
    void *const device2 = self->device_;
    if (device2) {
        if (*WaveDeviceReleaseGuard) {
            (WaveDeviceReleaseSlot())(device2);
        }
        self->device_ = nullptr;
    }
    if (self->flags_40_ & 2) {
        AmbienceSoundView *const prev = self->chain_prev_;
        if (prev) {
            reinterpret_cast<AmbienceSoundView volatile *>(prev)->chain_next_ =
                self->chain_next_;
        } else {
            WaveChainHead() = reinterpret_cast<Wave *>(self->chain_next_);
        }
        AmbienceSoundView *const next = self->chain_next_;
        if (next) {
            reinterpret_cast<AmbienceSoundView volatile *>(next)->chain_prev_ =
                self->chain_prev_;
        } else {
            WaveChainTail() = reinterpret_cast<Wave *>(self->chain_prev_);
        }
        self->chain_next_ = nullptr;
        self->chain_prev_ = nullptr;
        self->flags_40_ &= ~2u;
    }
    self->vtable_storage_ = 0x0066E444;
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
// ORIGINAL: 0x004C8460 ??0Ambience@@QAE@XZ 0x004C8460-0x004C852D;0x004C8450-0x004C8457;0x00659F4C-0x00659F5E
// TRIED: dropping `volatile` off `object`/`bytes` measured 0/72
//   agreeing both ways (the function diverges at instruction 0, the
//   omitted SEH prologue, before reaching these stores), but it shrank
//   the compiled body from 31 to 25 instructions - the optimiser folds
//   the documented `object[0x40/4] |= 8` / `bytes[0x54] |= 1`
//   read-modify-writes into plain immediate stores, which the
//   Verification note above says the image does NOT do (it genuinely
//   reloads at 0x004C84FA/0x004C84FC). Reverted; volatile is load-bearing
//   here, not incidental.
// symbol    ?construct@Ambience@@QAEXXZ
// size      230 bytes
// prototype void (__thiscall ??0Ambience@@QAE@XZ)(Ambience* this)
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C61E0 0x006465F0
// indirect  0x004C84E2
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
    // LEVER: real `call memset` in the image (0x004C849A), not a store loop -
    // same shape as Wave's constructor. Cast away `volatile` only for the
    // memset argument; the re-loads below still go through `object`/`bytes`.
    memset(const_cast<uint32_t *>(&object[0x0C / 4]), 0, 0x24);
    object[0x30 / 4] = 0;
    object[0x44 / 4] = 0;
    object[0x48 / 4] = 0;
    object[0x3C / 4] = 0;
    object[0x4C / 4] = 0;
    // memset to zero at 0x004C84BE, then the loaded bit cleared at 0x004C84C8
    // - a no-op on the zero just stored - then bit 3 set below: net 8, where
    // Wave nets 4.
    memset(const_cast<uint32_t *>(&object[0x40 / 4]), 0, 4);
    object[0x40 / 4] &= ~1u;
    object[0x38 / 4] = 0x3E8;
    object[0x50 / 4] = 0;
    // memset(this + 0x54, 0, 1) at 0x004C84F5: one byte, not Wave's four. The
    // original leaves 0x55..0x57 alone, so no defensive widening here.
    memset(const_cast<uint8_t *>(&bytes[0x54]), 0, 1);
    // Re-read, do NOT fold into the stores above: the original reloads the
    // flag dword at 0x004C84FA and the flag byte at 0x004C84FC after their own
    // zeroing, and stores them back at 0x004C8507 and 0x004C850D in that
    // order.
    object[0x40 / 4] |= 8;
    bytes[0x54] |= 1;
    object[0x00 / 4] = 0x0066E664;
    reinterpret_cast<Sound *>(this)->set_type(5);
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447270 ?basewin_show@FactionAmbience@@QAEXXZ 0x00447270-0x00447275 BYTE_EXACT
// size      5 bytes
// prototype void (__thiscall ?basewin_show@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::basewin_show() {
    field_6C_ = 1;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447280 ?basewin_hide@FactionAmbience@@QAEXXZ 0x00447280-0x00447285 BYTE_EXACT
// size      5 bytes
// prototype void (__thiscall ?basewin_hide@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::basewin_hide() {
    field_6C_ = 0;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447290 ?design_window_show@FactionAmbience@@QAEXXZ 0x00447290-0x00447295 BYTE_EXACT
// size      5 bytes
// prototype void (__thiscall ?design_window_show@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::design_window_show() {
    field_6D_ = 1;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x004472A0 ?design_window_hide@FactionAmbience@@QAEXXZ 0x004472A0-0x004472A5 BYTE_EXACT
// size      5 bytes
// prototype void (__thiscall ?design_window_hide@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::design_window_hide() {
    field_6D_ = 0;
}


/*
Purpose: GAmbience raises the flag byte only when it is CLEAR.

             mov al,[ecx+0x6C] / test al,al / jne done
             mov byte [ecx+0x6C],1 / done: ret

         The guard is not redundant. Its sibling basewin_hide below stores
         unconditionally, and an unconditional store here would agree for the
         values 0 and 1 and differ for every other: a byte holding 5 keeps 5,
         where `field_6C_ = 1` would overwrite it.
// ORIGINAL: 0x00447BE0 ?basewin_show@GAmbience@@QAEXXZ 0x00447BE0-0x00447BEC BYTE_EXACT
// size      12 bytes
// prototype void (__thiscall ?basewin_show@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::basewin_show() {
    if (field_6C_ == 0) {
        field_6C_ = 1;
    }
}


/*
Purpose: GAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447BF0 ?basewin_hide@GAmbience@@QAEXXZ 0x00447BF0-0x00447BF5 BYTE_EXACT
// size      5 bytes
// prototype void (__thiscall ?basewin_hide@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::basewin_hide() {
    field_6C_ = 0;
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

// ORIGINAL: 0x004471F0 ??0FactionAmbience@@QAE@XZ 0x004471F0-0x00447219 BYTE_EXACT
// LEVER: the vtable store must NOT be `volatile`. A volatile store is a scheduling barrier for VC6, so the `xor eax, eax` that feeds the seven zero field stores could not be hoisted above it - 13 of 15, with the image's `xor eax, eax` / `mov [esi], vtable` pair in the wrong order. Dropping the qualifier gives 15 of 15, BYTE_EXACT. The store stays alive without it because the seven field stores that follow are not dead, so nothing lets VC6 prove the object unobserved. Measured the same 15 of 15 with the `object[0x000 / 4]` idiom and with the zero stores written before the vtable store, so only the qualifier matters.
// symbol    ?construct@FactionAmbience@@QAEPAV1@XZ
//   The catalogued name is the mangled CONSTRUCTOR, `??0FactionAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      41 bytes
// prototype void (__thiscall ??0FactionAmbience@@QAE@XZ)(FactionAmbience* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
FactionAmbience *FactionAmbience::construct() {
    // THE METHOD, not the two-argument redirect: passing `nullptr` for the
    // redirect's unused second argument materialises `xor edx, edx`, one
    // instruction the image does not have.
    reinterpret_cast<Ambience *>(this)->Ambience::construct();
    *reinterpret_cast<uint32_t *>(this) = FactionAmbienceVtable;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
    return this;
}


/*
Purpose: As 004471F0, with the MAmbience vtable and one more field.
// ORIGINAL: 0x00447310 ??0MAmbience@@QAE@XZ 0x00447310-0x0044733C BYTE_EXACT
// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
// symbol    ?construct@MAmbience@@QAEPAV1@XZ
//   The catalogued name is the mangled CONSTRUCTOR, `??0MAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      44 bytes
// prototype void (__thiscall ??0MAmbience@@QAE@XZ)(MAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
MAmbience *MAmbience::construct() {
    // THE METHOD, not the two-argument redirect: passing `nullptr` for the
    // redirect's unused second argument materialises `xor edx, edx`, one
    // instruction the image does not have.
    reinterpret_cast<Ambience *>(this)->Ambience::construct();
    *reinterpret_cast<uint32_t *>(this) = MAmbienceVtable;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
    field_70_ = 0;
    return this;
}


/*
Purpose: As 004471F0, with the SAmbience vtable and two more fields.
// ORIGINAL: 0x00447850 ??0SAmbience@@QAE@XZ 0x00447850-0x0044787F BYTE_EXACT
// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
// symbol    ?construct@SAmbience@@QAEPAV1@XZ
//   The catalogued name is the mangled CONSTRUCTOR, `??0SAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      47 bytes
// prototype void (__thiscall ??0SAmbience@@QAE@XZ)(SAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
SAmbience *SAmbience::construct() {
    // THE METHOD, not the two-argument redirect: passing `nullptr` for the
    // redirect's unused second argument materialises `xor edx, edx`, one
    // instruction the image does not have.
    reinterpret_cast<Ambience *>(this)->Ambience::construct();
    *reinterpret_cast<uint32_t *>(this) = SAmbienceVtable;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
    field_70_ = 0;
    field_74_ = 0;
    return this;
}


/*
Purpose: As 004471F0, with the GAmbience vtable and one more field.
// ORIGINAL: 0x00447B90 ??0GAmbience@@QAE@XZ 0x00447B90-0x00447BBC BYTE_EXACT
// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
// symbol    ?construct@GAmbience@@QAEPAV1@XZ
//   The catalogued name is the mangled CONSTRUCTOR, `??0GAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      44 bytes
// prototype void (__thiscall ??0GAmbience@@QAE@XZ)(GAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
GAmbience *GAmbience::construct() {
    // THE METHOD, not the two-argument redirect: passing `nullptr` for the
    // redirect's unused second argument materialises `xor edx, edx`, one
    // instruction the image does not have.
    reinterpret_cast<Ambience *>(this)->Ambience::construct();
    *reinterpret_cast<uint32_t *>(this) = GAmbienceVtable;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
    field_70_ = 0;
    return this;
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Ambience g_AMBIENCE;  // 0x0074DA40
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
