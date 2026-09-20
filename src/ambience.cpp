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
#include <mmsystem.h>
#include "original_seam.h"
#include "ambience.h"
#include "sound.h"   // Sound::set_type, reached by cast
#include "wave.h"
#include "general.h"  // filefind_get

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



















/*
Purpose: Destroy an ambience. Its own stage frees the remembered filename
         and releases the device through the guarded hook, clearing both
         fields unconditionally; the compiler-appended ~Sound then repeats
         the same teardown on the now-cleared fields (its frees no-op) and
         unlinks from the global sound chain when the chained bit is set -
         the image carries that Sound stage INLINED, which is why the old
         transcription duplicated it by hand.
// ORIGINAL: 0x004C7670 ??1Ambience@@QAE@XZ 0x004C7670-0x004C7760;0x004C8450-0x004C8457;0x00659F32-0x00659F4C
// symbol    ??1Ambience@@UAE@XZ
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
    // THE OWN STAGE ONLY: free the remembered filename and release the
    // device through the guarded hook, clearing both unconditionally. The
    // compiler appends ~Sound, whose stage repeats the same teardown on the
    // now-cleared fields (so its frees no-op) and unlinks from the global
    // sound chain when the chained bit sets - the work the image's ~Ambience
    // carries INLINED. The vtable staging (0x66E538 -> 0x66E3C0 -> 0x66E444)
    // is the compiler's now; AmbienceSoundView - Sound's own member layout
    // under another name - is gone with the inheritance that made it
    // redundant.
    if (fname_) {
        operator delete(fname_);
    }
    fname_ = nullptr;
    if (device_ && WaveDeviceReleaseGuard) {
        (WaveDeviceReleaseSlot())(device_);
    }
    device_ = nullptr;
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

// ORIGINAL: 0x004471F0 ??0FactionAmbience@@QAE@XZ 0x004471F0-0x00447219
// symbol    ?construct@FactionAmbience@@QAEPAV1@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: the vtable store must NOT be `volatile`. A volatile store is a scheduling barrier for VC6, so the `xor eax, eax` that feeds the seven zero field stores could not be hoisted above it - 13 of 15, with the image's `xor eax, eax` / `mov [esi], vtable` pair in the wrong order. Dropping the qualifier gives 15 of 15, BYTE_EXACT. The store stays alive without it because the seven field stores that follow are not dead, so nothing lets VC6 prove the object unobserved. Measured the same 15 of 15 with the `object[0x000 / 4]` idiom and with the zero stores written before the vtable store, so only the qualifier matters.
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
// ORIGINAL: 0x00447310 ??0MAmbience@@QAE@XZ 0x00447310-0x0044733C
// symbol    ?construct@MAmbience@@QAEPAV1@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
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
// ORIGINAL: 0x00447850 ??0SAmbience@@QAE@XZ 0x00447850-0x0044787F
// symbol    ?construct@SAmbience@@QAEPAV1@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
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
// ORIGINAL: 0x00447B90 ??0GAmbience@@QAE@XZ 0x00447B90-0x00447BBC
// symbol    ?construct@GAmbience@@QAEPAV1@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
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

// THE AMBIENCE SEQUENCER STATE. Every one of these is an ordinary int the
// image keeps in .data at a fixed address, and the four variants below read
// and write them as one shared bank. They are named from what the bodies do
// with them - the increment, the wrap point, the virtual slot the value
// feeds - because no catalogue row covers any of them. The binding spelling
// is the one the codegen needs: a dereferenced const pointer folds to the
// absolute address, where an `extern T` definition would not.
//
//   AmbienceClockArmed   0x0074C5E8  byte, set once, gates the first clock read
//   AmbienceClock        0x0074D8EC  the last clock reading
//   AmbienceGeneralTick  0x00683C14  clamped to 18, incremented once a pass
//   MAmbienceStep        0x00683BF8  play_trackset argument, cycles 5..7
//   MAmbienceStepBase    0x00683BFC  the closing xpose_trackset argument
//   MAmbiencePass        0x0074DAAC  the five-second pass counter
//   MAmbienceCents       0x0074DAB0  +150 a pass, wraps +600 back to -600
//   UAmbienceStep        0x00683C00  play_trackset argument, wraps +600 to -600
//   UAmbienceStepBase    0x00683C04  play_trackset argument, fixed
//   UAmbienceStepEvery   0x00683C08  the pass count UAmbience waits for
//   UAmbienceTrackset    0x00683C0C  toggles between 4 and 5
//   UAmbiencePass0       0x0074DAB4  wraps at 9
//   UAmbiencePass1       0x0074DAB8  wraps at 6
//   UAmbienceCents       0x0074DABC  +100 a pass
//   UAmbiencePass2       0x0074DAC0  compared against UAmbienceStepEvery
//   UAmbiencePass3       0x0074DAC4  wraps at 15
//   SAmbienceCents       0x0074DAC8  +100 a pass, wraps +400 back to -300
//   SAmbiencePass0       0x0074DACC  wraps at 1
//   SAmbiencePass1       0x0074DAD0  wraps at 7
static uint8_t *const AmbienceClockArmed = (uint8_t *)0x0074C5E8;
static int *const AmbienceClock = (int *)0x0074D8EC;
static int *const AmbienceGeneralTick = (int *)0x00683C14;
static int *const MAmbienceStep = (int *)0x00683BF8;
static int *const MAmbienceStepBase = (int *)0x00683BFC;
static int *const MAmbiencePass = (int *)0x0074DAAC;
static int *const MAmbienceCents = (int *)0x0074DAB0;
static int *const UAmbienceStep = (int *)0x00683C00;
static int *const UAmbienceStepBase = (int *)0x00683C04;
static int *const UAmbienceStepEvery = (int *)0x00683C08;
static int *const UAmbienceTrackset = (int *)0x00683C0C;
static int *const UAmbiencePass0 = (int *)0x0074DAB4;
static int *const UAmbiencePass1 = (int *)0x0074DAB8;
static int *const UAmbienceCents = (int *)0x0074DABC;
static int *const UAmbiencePass2 = (int *)0x0074DAC0;
static int *const UAmbiencePass3 = (int *)0x0074DAC4;
static int *const SAmbienceCents = (int *)0x0074DAC8;
static int *const SAmbiencePass0 = (int *)0x0074DACC;
static int *const SAmbiencePass1 = (int *)0x0074DAD0;

void Ambience::vslot_33() {}
void Ambience::vslot_34() {}
void Ambience::vslot_35() {}
void Ambience::vslot_36() {}
void Ambience::vslot_37() {}
void Ambience::vslot_38() {}
void Ambience::vslot_39() {}
void Ambience::vslot_40() {}
void Ambience::vslot_41() {}
void Ambience::vslot_42() {}
void Ambience::vslot_43() {}
void Ambience::vslot_44() {}
void Ambience::vslot_45() {}
void Ambience::vslot_46() {}
void Ambience::vslot_47() {}
void Ambience::vslot_48() {}
void Ambience::vslot_49() {}
void Ambience::vslot_50() {}
void Ambience::vslot_51() {}
void Ambience::vslot_52() {}
void Ambience::vslot_53() {}
void Ambience::vslot_54() {}
void Ambience::vslot_55() {}
void Ambience::vslot_56() {}
void Ambience::vslot_57() {}
void Ambience::vslot_58() {}
void Ambience::vslot_59() {}
void Ambience::vslot_60() {}
void Ambience::vslot_61() {}
void Ambience::vslot_62() {}
void Ambience::vslot_63() {}
void Ambience::vslot_64() {}
void Ambience::vslot_65() {}
void Ambience::vslot_66() {}
void Ambience::vslot_71() {}

int Ambience::play_trackset(unsigned int) { return 0; }
int Ambience::stop_trackset(unsigned int) { return 0; }
int Ambience::xpose_trackset(unsigned int, int) { return 0; }
bool Ambience::is_trackset_playing(unsigned int) { return false; }
int Ambience::load_patch(unsigned int) { return 0; }
int Ambience::unload_patch(unsigned int) { return 0; }
int Ambience::set_patch(unsigned int, unsigned int) { return 0; }

// ORIGINAL: 0x00447360 ?general@MAmbience@@QAEXXZ 0x00447360-0x0044746A FILE
// size      266 bytes
// prototype void (__thiscall ?general@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0044737E 0x0044738B 0x00447399 0x004473E2 0x004473FA 0x00447461
void MAmbience::general() {
    DWORD (WINAPI *const get_time)() = timeGetTime;
    if ((*AmbienceClockArmed & 1) == 0) {
        *AmbienceClockArmed |= 1;
        *AmbienceClock = get_time();
    }
    if (!is_trackset_playing(0x64)) {
        unsigned int now = get_time();
        unsigned int elapsed = now - static_cast<unsigned int>(*AmbienceClock);
        int pass = *MAmbiencePass + 1;
        *MAmbiencePass = pass;
        if (elapsed >= 0x1388 && pass >= 5) {
            *MAmbiencePass = 0;
            xpose_trackset(*MAmbienceStep, *MAmbienceCents);
            int step = (*MAmbienceStep)++;
            play_trackset(step);
            if (*MAmbienceStep > 7) {
                *MAmbienceStep = 5;
            }
            *MAmbienceCents += 0x96;
            if (*MAmbienceCents > 0x258) {
                *MAmbienceCents = -600;
            }
            *AmbienceClock = now;
            field_70_ += 0x64;
            if (field_70_ > 0x1f4) {
                field_70_ = -700;
            }
            xpose_trackset(*MAmbienceStepBase, field_70_);
        }
    }
}

// ORIGINAL: 0x00447570 ?general@UAmbience@@QAEXXZ 0x00447570-0x00447684 FILE
// size      276 bytes
// prototype void (__thiscall ?general@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004475D1 0x004475EB 0x00447620 0x00447650 0x00447675
void UAmbience::general() {
    if (field_6C_ != 0) {
        return;
    }
    ++*UAmbiencePass3;
    ++*UAmbiencePass0;
    ++*UAmbiencePass1;
    ++*UAmbiencePass2;
    if (*UAmbiencePass3 == 0xf) {
        *UAmbienceTrackset = (*UAmbienceTrackset == 4) + 4;
        play_trackset(*UAmbienceTrackset);
        *UAmbiencePass3 = 0;
    }
    if (*UAmbiencePass0 == 9) {
        play_trackset(0);
        *UAmbiencePass0 = 0;
    }
    if (*UAmbiencePass1 == 6) {
        *UAmbienceStep = (*UAmbienceStep != 2) + 1;
        xpose_trackset(*UAmbienceStep, *UAmbienceCents);
        *UAmbienceCents += 0x64;
        if (*UAmbienceStep > 0x258) {
            *UAmbienceStep = -600;
        }
        play_trackset(*UAmbienceStep);
        *UAmbiencePass1 = 0;
    }
    if (*UAmbiencePass2 == *UAmbienceStepEvery) {
        play_trackset(*UAmbienceStepBase);
        *UAmbiencePass2 = 0;
    }
}

// ORIGINAL: 0x004476F0 ?general@SAmbience@@QAEXXZ 0x004476F0-0x004477A9 FILE
// LEVER: the delegate vtable - declaring Sound's 33 + the sequencer's 42 on
//   Ambience (ambience.h slots 33..74) is what turned the VCall shims into
//   real `call dword ptr [eax+0xNNN]` dispatches and brought this body from
//   an unverifiable artifact to 51 of 51 instructions structurally.
// TRIED: clamp through a named local read once (`int tick = *p; if (tick >
//   0x12) { tick = 0x11; *p = tick; }`) - the image holds the clamped tick in
//   ecx across the store AND the xpose argument, this spelling still lands it
//   in eax, 1 of 51. Plain `if (*p > 0x12) *p = 0x11;` gives the immediate
//   store the image does not emit. Also open: `(*p)++ > n` compiles
//   `lea ecx,[eax+1]` where the image has `mov edx,eax; inc eax`, and the
//   trailing `++*p` compiles load/inc/store where the image has one
//   `inc dword ptr [0x683c14]`. Register-preference plateau, not a semantic
//   one - same 51 instructions, same calls, same order.
// size      185 bytes
// prototype void (__thiscall ?general@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447727 0x00447738 0x00447767 0x00447773
void SAmbience::general() {
    // Read once and clamped in the local: the image keeps the clamped value
    // live in ecx across the clamp store AND the xpose_trackset argument that
    // follows it (0x004476FE `mov ecx, 0x11` -> 0x00447703 store ->
    // 0x00447724 `push ecx`), and only reloads the field for play_trackset,
    // after the call has possibly overwritten it.
    int tick = *AmbienceGeneralTick;
    if (tick > 0x12) {
        tick = 0x11;
        *AmbienceGeneralTick = tick;
    }
    if ((*SAmbiencePass0)++ > 1) {
        xpose_trackset(tick, *SAmbienceCents);
        play_trackset(*AmbienceGeneralTick);
        *SAmbiencePass0 = 0;
    }
    if ((*SAmbiencePass1)++ > 7) {
        xpose_trackset(0x10, *SAmbienceCents);
        play_trackset(0x10);
        *SAmbiencePass1 = 0;
    }
    *SAmbienceCents += 0x64;
    if (*SAmbienceCents > 0x190) {
        *SAmbienceCents = -300;
    }
    ++*AmbienceGeneralTick;
}

// ORIGINAL: 0x004477B0 ?hostility@SAmbience@@QAEXXZ 0x004477B0-0x00447831 FILE BYTE_EXACT
// LEVER: field signedness. field_70_ and field_74_ are int32_t, not
//   uint32_t - the image compares each against a positive bound with `jle`
//   and wraps it downward past zero, and `uint32_t` spelled `jbe` twice for a
//   2-instruction loss. Together with the delegate vtable on Ambience this
//   took the homed artifact from 16 of 46 to 46 of 46.
// LEVER: the artifact's `VCall2 *self = (VCall2 *)this` pun and its
//   `field_70_ + 1` / store local are both gone - the member form with the
//   declared virtuals is byte-identical and carries no semantic debt.
// size      129 bytes
// prototype void (__thiscall ?hostility@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004477B7 0x004477D7 0x00447802 0x00447810
void SAmbience::hostility() {
    if (is_trackset_playing(0x64)) {
        return;
    }
    if (field_70_ == 0) {
        field_70_ = 0x64;
    }
    if (is_trackset_playing(field_70_)) {
        return;
    }
    field_70_ += 1;
    if (field_70_ > 0x67) {
        field_70_ = 0x65;
    }
    xpose_trackset(field_70_, field_74_);
    play_trackset(field_70_);
    field_74_ += 0x64;
    if (field_74_ > 0x12c) {
        field_74_ = 0xfffffed4;
    }
}

class BeginVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void slot016();
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual void slot033();
  virtual void slot034();
  virtual void slot035();
  virtual void slot036();
  virtual void slot037();
  virtual void slot038();
  virtual void slot039();
  virtual void slot040();
  virtual void slot041();
  virtual void slot042();
  virtual void slot043();
  virtual void slot044();
  virtual void slot045();
  virtual void slot046();
  virtual void slot047();
  virtual void slot048();
  virtual void slot049();
  virtual void slot050();
  virtual void slot051();
  virtual void slot052();
  virtual void slot053();
  virtual void slot054();
  virtual void slot055();
  virtual void slot056();
  virtual void slot057();
  virtual void slot058();
  virtual void slot059();
  virtual void slot060();
  virtual void slot061();
  virtual void slot062();
  virtual void slot063();
  virtual void slot064();
  virtual void slot065();
  virtual void slot066();
  virtual void slot067(int);
};

// THE SLOT-7 DISPATCH. Sound declares play(unsigned long) before play(), and
// VC6 gives same-name overloads their slots in REVERSE declaration order, so
// `reinterpret_cast<SlotSeven *>(this)->slot007()` lands one slot low. Sound's table is not this file's to move,
// so the four begin() variants reach slot 7 through a named holder instead -
// the class exists only to spell the convention and is never instantiated.
class SlotSeven { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
};

// ORIGINAL: 0x00447340 ?begin@MAmbience@@QAEXXZ 0x00447340-0x00447356 BYTE_EXACT
// size      22 bytes
// prototype void (__thiscall ?begin@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447347 0x00447351
// 0x00447340  ?begin@MAmbience@@QAEXXZ  ->  ?begin@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::begin() {
    this->play_trackset(1);
    reinterpret_cast<SlotSeven *>(this)->slot007();
}

class GateVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void slot016();
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual void slot033();
  virtual void slot034();
  virtual void slot035();
  virtual void slot036();
  virtual void slot037();
  virtual void slot038();
  virtual void slot039();
  virtual void slot040();
  virtual void slot041();
  virtual void slot042();
  virtual void slot043();
  virtual void slot044();
  virtual void slot045();
  virtual void slot046();
  virtual void slot047();
  virtual void slot048();
  virtual void slot049();
  virtual void slot050();
  virtual void slot051();
  virtual void slot052();
  virtual void slot053();
  virtual void slot054();
  virtual void slot055();
  virtual void slot056();
  virtual void slot057();
  virtual void slot058();
  virtual void slot059();
  virtual void slot060();
  virtual void slot061();
  virtual void slot062();
  virtual void slot063();
  virtual void slot064();
  virtual void slot065();
  virtual void slot066();
  virtual void slot067(int);
  virtual void slot068();
  virtual void slot069();
  virtual bool slot070(int);
};

// ORIGINAL: 0x00447470 ?tech@MAmbience@@QAEXXZ 0x00447470-0x0044748F BYTE_EXACT
// size      31 bytes
// prototype void (__thiscall ?tech@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00447477 0x00447487
// 0x00447470  ?tech@MAmbience@@QAEXXZ  ->  ?tech@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::tech() {
    if (!this->is_trackset_playing(0x64)) {
        this->play_trackset(0x69);
    }
}

// ORIGINAL: 0x00447490 ?energy_resources@MAmbience@@QAEXXZ 0x00447490-0x004474AF BYTE_EXACT
// size      31 bytes
// prototype void (__thiscall ?energy_resources@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447497 0x004474A7
// 0x00447490  ?energy_resources@MAmbience@@QAEXXZ  ->  ?energy_resources@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::energy_resources() {
    if (!this->is_trackset_playing(0x64)) {
        this->play_trackset(4);
    }
}

// ORIGINAL: 0x004474B0 ?production@MAmbience@@QAEXXZ 0x004474B0-0x004474DF FILE BYTE_EXACT
// size      47 bytes
// prototype void (__thiscall ?production@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004474B7 0x004474CB 0x004474D7
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004474B0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004474b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?production@MAmbience@@QAEXXZ  at 0x004474B0  (47 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

void MAmbience::production() {
    Ambience *const self = this;
    if (!self->is_trackset_playing(0x64)) {
        self->xpose_trackset(4, *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x70));
        self->play_trackset(4);
    }
}

class NewBaseVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void slot016();
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual void slot033();
  virtual void slot034();
  virtual void slot035();
  virtual void slot036();
  virtual void slot037();
  virtual void slot038();
  virtual void slot039();
  virtual void slot040();
  virtual void slot041();
  virtual void slot042();
  virtual void slot043();
  virtual void slot044();
  virtual void slot045();
  virtual void slot046();
  virtual void slot047();
  virtual void slot048();
  virtual void slot049();
  virtual void slot050();
  virtual void slot051();
  virtual void slot052();
  virtual void slot053();
  virtual void slot054();
  virtual void slot055();
  virtual void slot056();
  virtual void slot057();
  virtual void slot058();
  virtual void slot059();
  virtual void slot060();
  virtual void slot061();
  virtual void slot062();
  virtual void slot063();
  virtual void slot064();
  virtual void slot065();
  virtual void slot066();
  virtual void slot067(int);
  virtual void slot068(int);
};

// ORIGINAL: 0x004474E0 ?new_base@MAmbience@@QAEXXZ 0x004474E0-0x00447507 BYTE_EXACT
// size      39 bytes
// prototype void (__thiscall ?new_base@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004474E7 0x004474F3 0x004474FF
// 0x004474E0  ?new_base@MAmbience@@QAEXXZ  ->  ?new_base@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::new_base() {
    // Unconditional, with no slot-70 gate - unlike tech() and
    // energy_resources() on this same class. And 0x64 here is the slot-67
    // argument, not the threshold it is elsewhere in the family.
    this->stop_trackset(4);
    this->stop_trackset(0x69);
    this->play_trackset(0x64);
}

// ORIGINAL: 0x00447510 ?begin@UAmbience@@QAEXXZ 0x00447510-0x00447526 BYTE_EXACT
// size      22 bytes
// prototype void (__thiscall ?begin@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447515 0x0044751E
// 0x00447510  ?begin@UAmbience@@QAEXXZ  ->  ?begin@UAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void UAmbience::begin() {
    reinterpret_cast<SlotSeven *>(this)->slot007();
    this->play_trackset(0);
}

// ORIGINAL: 0x00447530 ?basewin_show@UAmbience@@QAEXXZ 0x00447530-0x0044754A FILE BYTE_EXACT
// size      26 bytes
// prototype void (__thiscall ?basewin_show@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0044753E
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447530
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447530/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?basewin_show@UAmbience@@QAEXXZ  at 0x00447530  (26 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void UAmbience::basewin_show() {
    char *self = reinterpret_cast<char *>(this);
    if (!field_6C_) {
        this->play_trackset(6);
        field_6C_ = 1;
    }
}

class HideVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void slot016();
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual void slot033();
  virtual void slot034();
  virtual void slot035();
  virtual void slot036();
  virtual void slot037();
  virtual void slot038();
  virtual void slot039();
  virtual void slot040();
  virtual void slot041();
  virtual void slot042();
  virtual void slot043();
  virtual void slot044();
  virtual void slot045();
  virtual void slot046();
  virtual void slot047();
  virtual void slot048();
  virtual void slot049();
  virtual void slot050();
  virtual void slot051();
  virtual void slot052();
  virtual void slot053();
  virtual void slot054();
  virtual void slot055();
  virtual void slot056();
  virtual void slot057();
  virtual void slot058();
  virtual void slot059();
  virtual void slot060();
  virtual void slot061();
  virtual void slot062();
  virtual void slot063();
  virtual void slot064();
  virtual void slot065();
  virtual void slot066();
  virtual void slot067();
  virtual void slot068(int);
};

// ORIGINAL: 0x00447550 ?basewin_hide@UAmbience@@QAEXXZ 0x00447550-0x00447563 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?basewin_hide@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447557
// 0x00447550  ?basewin_hide@UAmbience@@QAEXXZ  ->  ?basewin_hide@UAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void UAmbience::basewin_hide() {
    this->stop_trackset(6);
    field_6C_ = 0;
}

// ORIGINAL: 0x004476C0 ?begin@SAmbience@@QAEXXZ 0x004476C0-0x004476D6 BYTE_EXACT
// size      22 bytes
// prototype void (__thiscall ?begin@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004476C5 0x004476CE
// 0x004476C0  ?begin@SAmbience@@QAEXXZ  ->  ?begin@SAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void SAmbience::begin() {
    reinterpret_cast<SlotSeven *>(this)->slot007();
    this->play_trackset(0x11);
}

class ProductionVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008();
  virtual void slot009();
  virtual void slot010();
  virtual void slot011();
  virtual void slot012();
  virtual void slot013();
  virtual void slot014();
  virtual void slot015();
  virtual void slot016();
  virtual void slot017();
  virtual void slot018();
  virtual void slot019();
  virtual void slot020();
  virtual void slot021();
  virtual void slot022();
  virtual void slot023();
  virtual void slot024();
  virtual void slot025();
  virtual void slot026();
  virtual void slot027();
  virtual void slot028();
  virtual void slot029();
  virtual void slot030();
  virtual void slot031();
  virtual void slot032();
  virtual void slot033();
  virtual void slot034();
  virtual void slot035();
  virtual void slot036();
  virtual void slot037();
  virtual void slot038();
  virtual void slot039();
  virtual void slot040();
  virtual void slot041();
  virtual void slot042();
  virtual void slot043();
  virtual void slot044();
  virtual void slot045();
  virtual void slot046();
  virtual void slot047();
  virtual void slot048();
  virtual void slot049();
  virtual void slot050();
  virtual void slot051();
  virtual void slot052();
  virtual void slot053();
  virtual void slot054();
  virtual void slot055();
  virtual void slot056();
  virtual void slot057();
  virtual void slot058();
  virtual void slot059();
  virtual void slot060();
  virtual void slot061();
  virtual void slot062();
  virtual void slot063();
  virtual void slot064();
  virtual void slot065();
  virtual void slot066();
  virtual void slot067(int);
};

// ORIGINAL: 0x00447840 ?production@SAmbience@@QAEXXZ 0x00447840-0x0044784B BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?production@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447844
// 0x00447840  ?production@SAmbience@@QAEXXZ  ->  ?production@SAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void SAmbience::production() {
    this->play_trackset(0xf);
}

// ORIGINAL: 0x00447880 ?base_liberated@SAmbience@@QAEXXZ 0x00447880-0x004478C3 FILE BYTE_EXACT
// size      67 bytes
// prototype void (__thiscall ?base_liberated@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447887 0x00447893 0x0044789F 0x004478AB 0x004478BB
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447880
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447880/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_liberated@SAmbience@@QAEXXZ  at 0x00447880  (67 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void SAmbience::base_liberated() {
    Ambience *const self = this;
    self->stop_trackset(0x65);
    self->stop_trackset(0x66);
    self->stop_trackset(0x67);
    if (!self->is_trackset_playing(0x64)) {
        self->play_trackset(0x64);
    }
}

// ORIGINAL: 0x00447BC0 ?begin@GAmbience@@QAEXXZ 0x00447BC0-0x00447BDD BYTE_EXACT
// size      29 bytes
// prototype void (__thiscall ?begin@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447BC5 0x00447BD5
// 0x00447BC0  ?begin@GAmbience@@QAEXXZ  ->  ?begin@GAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void GAmbience::begin() {
    reinterpret_cast<SlotSeven *>(this)->slot007();
    // Alone among the six begin() overrides, this one also stores its mode.
    field_68_ = 0x18;
    this->play_trackset(0x18);
}

// Fixed-slot bindings carried from 00447c00.cpp
// AmbienceClockArmed's sibling at 0x00683C20: UAmbience's own one-shot
// gate, compared against 0x18 and never written here.
static int *const UAmbienceGate = (int *)0x00683C20;

// ORIGINAL: 0x00447C00 ?general@GAmbience@@QAEXXZ 0x00447C00-0x00447C58 FILE BYTE_EXACT
// size      88 bytes
// prototype void (__thiscall ?general@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00447C0F
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447C00
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447c00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?general@GAmbience@@QAEXXZ  at 0x00447C00  (88 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::general() {
    char *self = reinterpret_cast<char *>(this);
    int32_t arg68 = *reinterpret_cast<int32_t *>(self + 0x68);
    int32_t *field70 = reinterpret_cast<int32_t *>(self + 0x70);

    this->xpose_trackset(arg68, *field70);

    if (*UAmbienceGate == 0x18) {
        *field70 += 300;
        if (*field70 > 600) {
            *field70 = -600;
        }
        return;
    }
    *field70 += 700;
    if (*field70 > 1200) {
        *field70 = -700;
    }
}

// Fixed-slot bindings carried from 00447c70.cpp
// GAmbience's play_trackset argument at 0x00683C24: cycled 7..9.
static int *const GAmbienceStep = (int *)0x00683C24;

// ORIGINAL: 0x00447C70 ?terraform@GAmbience@@QAEXXZ 0x00447C70-0x00447C9A FILE BYTE_EXACT
// size      42 bytes
// prototype void (__thiscall ?terraform@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447C79
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447C70
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447c70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?terraform@GAmbience@@QAEXXZ  at 0x00447C70  (42 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::terraform() {
    this->play_trackset(*GAmbienceStep);
    if (++*GAmbienceStep > 9) {
        *GAmbienceStep = 7;
    }
}

// Fixed-slot bindings carried from 00447cd0.cpp
// SAmbience's one-shot flag at 0x00683C28, set to 1 on entry.
static int *const SAmbienceArmed = (int *)0x00683C28;

// ORIGINAL: 0x00447CD0 ?new_base@GAmbience@@QAEXXZ 0x00447CD0-0x00447D07 FILE BYTE_EXACT
// size      55 bytes
// prototype void (__thiscall ?new_base@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447CE5 0x00447CFF
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447CD0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447cd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?new_base@GAmbience@@QAEXXZ  at 0x00447CD0  (55 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::new_base() {
    *SAmbienceArmed = 1;

    this->stop_trackset(field_68_);
    field_68_ = 0x19;
    field_70_ = 0;
    this->play_trackset(0x19);
}
