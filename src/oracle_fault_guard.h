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
#pragma once

#include <setjmp.h>

// Surviving a fault inside an oracle call, so one bad body costs one verdict
// instead of the whole run.
//
// The generated signature suite calls 108 original bodies with a synthesised
// receiver. Some of them fault: a zero-filled `this` makes a body that walks a
// chain its constructor guarantees read [0+4]. Without a guard the first such
// body takes the process down, every remaining verdict is forfeit, and the run
// yields exactly one datum - the name the announce line printed. Measured: the
// suite reaches function 18 of 108 and dies on ?update_data@Console@@QAEXH@Z.
// That is one exclusion per rebuild-restage-run cycle, which is why 91 oracles
// have never produced a verdict.
//
// The mechanism is the one tools/lifted_oracle.cpp already proves out:
// AddVectoredExceptionHandler, and on a fault point EIP at a thunk standing on a
// PRIVATE stack, because the stack the fault happened on may be the thing that
// is wrong. C++ exceptions are not available - the DLL builds -fno-exceptions
// and verify-recovery-abi fails any object that grows .eh_frame - and none are
// needed: i686 mingw setjmp is msvcrt _setjmp3, which records the SEH
// registration head and unwinds it on longjmp, with no unwind tables involved.
//
// Two things keep this from swallowing faults it has no business in. The handler
// is gated on the thread that armed it, because a vectored handler is
// process-global and the game has other threads; and only a closed list of
// exception codes is converted, so anything else stays an honest crash.
namespace oracle_fault_guard {

// Arm the handler for the CALLING thread. Idempotent. Call it once on entry to
// the suite, not from DllMain: the deferred phase runs on the game's own thread
// after the executable's CRT is up, and not under the loader lock.
void arm();

// The jmp_buf the emitted `setjmp` uses. It has to be the caller's own frame
// that resumes, so the setjmp cannot be hidden behind a function of ours.
jmp_buf *buffer();

// Bracket one call. `begin` records that a guarded call is in progress on this
// thread; `end` clears it, and must run on both the normal and the escaped path
// or a later unrelated fault gets attributed to the oracle.
void begin(unsigned long address, const char *side);
void end();

// After an escape: the exception code and the address that faulted, for a
// verdict line that names something.
unsigned long code();
unsigned long address();

}  // namespace oracle_fault_guard
