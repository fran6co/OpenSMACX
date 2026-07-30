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
// The guard rewrites a faulting thread's EIP and ESP, from a handler that is
// process-global. Two properties make that safe rather than reckless, and both
// are tested here because neither is visible by reading the emitted oracle: only
// a closed list of exception codes is converted, and only on the thread that
// armed the guard. A third property is tested because the whole point is
// throughput: the guard must be re-armable, so one fault does not end the run.
#include <windows.h>

#include <cstdio>
#include <setjmp.h>

#include "../src/oracle_fault_guard.h"

namespace {

int failures = 0;

void expect(bool condition, const char *what) {
    if (!condition) {
        std::printf("FAIL: %s\n", what);
        ++failures;
    }
}

// Kept in a volatile pointer so the compiler cannot prove the dereference is
// undefined and delete it.
volatile int *null_pointer = nullptr;
volatile int zero_divisor = 0;

bool escaped_from(void (*body)()) {
    oracle_fault_guard::arm();
    oracle_fault_guard::begin(0x00401000UL, "test");
    if (setjmp(*oracle_fault_guard::buffer()) == 0) {
        body();
        oracle_fault_guard::end();
        return false;
    }
    oracle_fault_guard::end();
    return true;
}

void read_null() {
    volatile int sink = *null_pointer;
    (void)sink;
}

void divide_by_zero() {
    volatile int numerator = 1;
    volatile int result = numerator / zero_divisor;
    (void)result;
}

void do_nothing() {}

}  // namespace

// The other thread's own escape thunk, reached by pointing its EIP here. A
// naked asm thunk for the same reason the guard uses one: it must not return to
// the faulting instruction.
extern "C" void oracle_fault_guard_test_recover(void);
extern "C" void oracle_fault_guard_test_longjmp(void);

namespace {

void test_an_access_violation_becomes_an_escape() {
    expect(escaped_from(read_null), "a null read escapes instead of dying");
    expect(oracle_fault_guard::code() == 0xC0000005UL,
           "the escape records ACCESS_VIOLATION");
    expect(oracle_fault_guard::address() != 0,
           "the escape records the faulting address");
}

void test_an_integer_divide_by_zero_becomes_an_escape() {
    expect(escaped_from(divide_by_zero), "a divide by zero escapes");
    expect(oracle_fault_guard::code() == 0xC0000094UL,
           "the escape records INT_DIVIDE_BY_ZERO");
}

void test_a_clean_body_does_not_escape() {
    expect(!escaped_from(do_nothing), "a clean body runs to completion");
}

void test_the_guard_is_RE_ARMABLE() {
    // The entire value of the guard: 108 calls, several of which fault. If one
    // escape left it unusable the run would still end at the first fault.
    for (int attempt = 0; attempt < 5; ++attempt) {
        if (!escaped_from(read_null)) {
            std::printf("FAIL: escape %d did not happen\n", attempt);
            ++failures;
            return;
        }
    }
    expect(!escaped_from(do_nothing),
           "a clean body still completes after five escapes");
}

// ---- the two properties that keep a process-global handler honest ----

void test_begin_RESETS_the_recorded_fault() {
    // begin/end brackets one call, and `begin` must clear the previous fault or
    // a later verdict could report the address of an earlier one. This is also
    // how the bracket is observable at all without crashing the test.
    expect(escaped_from(read_null), "the setup escape happened");
    expect(oracle_fault_guard::code() != 0, "a fault was recorded");
    oracle_fault_guard::begin(0x00402000UL, "test");
    expect(oracle_fault_guard::code() == 0,
           "begin() clears the previous fault rather than carrying it forward");
    oracle_fault_guard::end();
}

// ---- the thread gate ----
//
// A vectored handler is process-global. If the guard converted a fault on a
// thread that never armed it, it would rewrite THAT thread's EIP to a thunk
// standing on the escape stack of a thread it knows nothing about, and longjmp
// through a jmp_buf belonging to a different stack. tools/lifted_oracle.cpp
// records having made exactly that mistake.
//
// Construction matters: the other thread installs its handler LAST
// (`first = 0`), so the guard is consulted FIRST. If the gate is broken the
// guard takes the fault and the other thread never reaches its own recovery.

volatile bool other_thread_recovered = false;
volatile bool other_thread_may_fault = false;

}  // namespace

jmp_buf other_thread_escape;

extern "C" void oracle_fault_guard_test_longjmp(void) {
    longjmp(other_thread_escape, 1);
}

asm(
".text\n"
".globl _oracle_fault_guard_test_recover\n"
"_oracle_fault_guard_test_recover:\n"
"  cld\n"
"  call _oracle_fault_guard_test_longjmp\n"
"  hlt\n"
);

namespace {

LONG CALLBACK other_thread_handler(EXCEPTION_POINTERS *ep) {
    if (ep->ExceptionRecord->ExceptionCode != 0xC0000005UL) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    ep->ContextRecord->Eip =
        DWORD(reinterpret_cast<uintptr_t>(&oracle_fault_guard_test_recover));
    return EXCEPTION_CONTINUE_EXECUTION;
}

DWORD WINAPI other_thread(LPVOID) {
    AddVectoredExceptionHandler(0, other_thread_handler);
    while (!other_thread_may_fault) {
        Sleep(1);
    }
    if (setjmp(other_thread_escape) == 0) {
        volatile int sink = *null_pointer;
        (void)sink;
    } else {
        other_thread_recovered = true;
    }
    return 0;
}

void test_a_fault_on_ANOTHER_thread_is_declined() {
    oracle_fault_guard::arm();
    HANDLE thread = CreateThread(nullptr, 0, other_thread, nullptr, 0, nullptr);
    if (thread == nullptr) {
        std::printf("FAIL: could not create the second thread\n");
        ++failures;
        return;
    }
    // Fault on the other thread WHILE a guarded call is open on this one, which
    // is the only configuration in which the gate can be wrong.
    oracle_fault_guard::begin(0x00403000UL, "test");
    other_thread_may_fault = true;
    const DWORD waited = WaitForSingleObject(thread, 5000);
    oracle_fault_guard::end();
    CloseHandle(thread);
    expect(waited == WAIT_OBJECT_0, "the other thread finished");
    expect(other_thread_recovered,
           "the other thread reached ITS OWN recovery, so the guard declined "
           "a fault on a thread that never armed it");
}

}  // namespace

int main() {
    test_an_access_violation_becomes_an_escape();
    test_an_integer_divide_by_zero_becomes_an_escape();
    test_a_clean_body_does_not_escape();
    test_the_guard_is_RE_ARMABLE();
    test_begin_RESETS_the_recorded_fault();
    test_a_fault_on_ANOTHER_thread_is_declined();
    if (failures != 0) {
        std::printf("oracle-fault-guard-tests: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("oracle-fault-guard-tests: all passed\n");
    return 0;
}
