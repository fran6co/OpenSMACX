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
#include "oracle_fault_guard.h"

#include <cstdio>

// Vectored exception handling shipped with Windows XP and VC6's Platform SDK
// predates it, so the entry point is present in every kernel32 this runs
// against - wine's included - but declared in none of its headers. Declared
// here rather than in vc6_compat.h because that header is included before
// <windows.h> and LONG and EXCEPTION_POINTERS do not exist yet at that point.
//
// RESOLVED AT RUNTIME, not imported. `__declspec(dllimport)` asks the linker
// for `__imp__AddVectoredExceptionHandler@8`, and that symbol lives in an
// IMPORT LIBRARY: VC6's kernel32.lib is from 1998 and carries no entry for an
// API that arrived with XP, so the declaration compiled happily and the DLL
// then failed to link on this one symbol - the last unresolved external in
// the whole build. The comment above is right that the entry point is present
// in every kernel32 this runs against; it is only the 1998 import library
// that disagrees, and GetProcAddress asks the DLL rather than the library.
typedef PVOID(WINAPI *AddVectoredExceptionHandlerFn)(
    ULONG First, LONG(CALLBACK *Handler)(EXCEPTION_POINTERS *));

static AddVectoredExceptionHandlerFn resolve_add_vectored_handler() {
    HMODULE kernel = GetModuleHandleA("kernel32.dll");
    if (kernel == NULL) {
        return NULL;
    }
    return reinterpret_cast<AddVectoredExceptionHandlerFn>(
        GetProcAddress(kernel, "AddVectoredExceptionHandler"));
}

namespace oracle_fault_guard {
namespace {

bool Armed = false;
// Which thread armed it. A vectored handler is process-global and the game runs
// a winmm timer thread among others; without this test a fault anywhere in the
// process while a guarded call was in progress would be rewritten as an escape
// from the oracle, corrupting a thread that had nothing to do with it. The
// sibling harness records exactly that mistake in tools/lifted_oracle.cpp.
volatile DWORD OwnerThread = 0;
volatile bool InCall = false;
volatile unsigned long CallAddress = 0;
volatile unsigned long FaultCode = 0;
volatile unsigned long FaultAddress = 0;
const char *CallSide = "";

jmp_buf Escape;
// Private, and 64 KiB, matching the sibling harness: the escape must not stand
// on the stack that faulted.
alignas(16) unsigned char EscapeStack[64 * 1024];

// Only these become a verdict. Everything else is an honest crash and must keep
// reaching whatever would have handled it.
//
// STATUS_GUARD_PAGE_VIOLATION is NOT here on purpose: it is how a thread's stack
// grows, and swallowing it would break ordinary execution. Nor is
// STATUS_STACK_OVERFLOW: escaping one leaves the guard page consumed and the
// process is no longer trustworthy, so it should die.
bool convertible(DWORD code) {
    switch (code) {
    case DWORD(0xC0000005):   // ACCESS_VIOLATION
    case DWORD(0xC0000006):   // IN_PAGE_ERROR
    case DWORD(0xC000001D):   // ILLEGAL_INSTRUCTION
    case DWORD(0xC000008C):   // ARRAY_BOUNDS_EXCEEDED
    case DWORD(0xC000008E):   // FLT_DIVIDE_BY_ZERO
    case DWORD(0xC0000094):   // INT_DIVIDE_BY_ZERO
    case DWORD(0xC0000095):   // INT_OVERFLOW
    case DWORD(0xC0000096):   // PRIV_INSTRUCTION
        return true;
    default:
        return false;
    }
}

}  // namespace

extern "C" void oracle_fault_guard_escape(void) {
    longjmp(Escape, 1);
}

// cld because the faulting body may have left DF set, and the C++ this returns
// into assumes it clear. No %fs:0 restore is needed here and that is worth
// stating: the original bodies this guards are VC6 code with __try frames, but
// msvcrt's longjmp calls _global_unwind2, which walks the SEH chain back to the
// setjmp frame and pops exactly those registrations - which is the work a %fs:0
// restore would be trying to shortcut, done correctly.
//
// Two spellings of the same three instructions. GCC's top-level `asm` is
// AT&T and needs the leading underscore written out; MSVC has no top-level
// `asm` at all - `C2290: C++ 'asm' syntax ignored` - and wants a naked
// function with an Intel-syntax body, where the decoration is the compiler's
// job. This is scaffolding, not a recovered body: nothing here is copied from
// the original image, so the bar on inline assembly in recovered code does
// not apply.
#if defined(_MSC_VER)
extern "C" __declspec(naked) void oracle_fault_guard_thunk(void) {
    __asm {
        cld
        call oracle_fault_guard_escape
        hlt
    }
}
#else
asm(
".text\n"
".globl _oracle_fault_guard_thunk\n"
"_oracle_fault_guard_thunk:\n"
"  cld\n"
"  call _oracle_fault_guard_escape\n"
"  hlt\n"
);

extern "C" void oracle_fault_guard_thunk(void);
#endif

namespace {

LONG CALLBACK handler(EXCEPTION_POINTERS *ep) {
    if (!InCall || GetCurrentThreadId() != OwnerThread) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    const DWORD code = ep->ExceptionRecord->ExceptionCode;
    if (!convertible(code)) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    FaultCode = code;
    FaultAddress = static_cast<unsigned long>(
        reinterpret_cast<uintptr_t>(ep->ExceptionRecord->ExceptionAddress));
    std::printf("  FAULT 0x%08lX at 0x%08lX in the %s side of 0x%08lX\n",
                FaultCode, FaultAddress, CallSide, CallAddress);
    std::fflush(stdout);
    ep->ContextRecord->EFlags &= ~0x100U;   // TF, so the escape does not step
    ep->ContextRecord->EFlags &= ~0x400U;   // DF, for the code resumed into
    ep->ContextRecord->Eip =
        DWORD(reinterpret_cast<uintptr_t>(&oracle_fault_guard_thunk));
    ep->ContextRecord->Esp =
        DWORD(reinterpret_cast<uintptr_t>(EscapeStack + sizeof EscapeStack - 64));
    return EXCEPTION_CONTINUE_EXECUTION;
}

}  // namespace

void arm() {
    if (Armed) {
        return;
    }
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    // A null answer means this host predates vectored handling. The guard then
    // does not arm, and `Armed` stays false so nothing downstream believes a
    // fault would be caught - which is the honest outcome, and better than
    // arming a handler that was never installed.
    AddVectoredExceptionHandlerFn add_handler = resolve_add_vectored_handler();
    if (add_handler == NULL) {
        return;
    }
    add_handler(1, handler);
    OwnerThread = GetCurrentThreadId();
    Armed = true;
}

jmp_buf *buffer() {
    return &Escape;
}

void begin(unsigned long address, const char *side) {
    CallAddress = address;
    CallSide = side;
    FaultCode = 0;
    FaultAddress = 0;
    InCall = true;
}

void end() {
    InCall = false;
}

unsigned long code() {
    return FaultCode;
}

unsigned long address() {
    return FaultAddress;
}

}  // namespace oracle_fault_guard
