// Task #32, first step: BOOT the lifted image, and report where it stops.
//
// Until now nothing has ever called the entry point. `lifted_main.cpp` is a
// link-keeper - it resolves 64 dispatch probes and exits - so "does the lift
// run as a program" had never been asked, only "does it link".
//
// This is deliberately a MEASUREMENT and not an attempt to make the game work.
// The differential oracle can reach at most ~49% of image bytes, because ~48.4%
// sits behind COM interfaces no seed reaches and 68.11% of bytes belong to
// functions whose ORIGINAL side faults on a seeded state - overwhelmingly
// because the globals their dynamic initialisers touch are uninitialised in a
// pristine image. Booting is the only thing that initialises those globals, so
// what this prints is the work queue for the half of the image the oracle
// cannot see.
//
// It answers three questions and stops:
//
//   1. How far does the entry point get before something stops it?
//   2. Was it a TRAP (an instruction the lift refused to lower, or a callee
//      with no body) or a FAULT (a bad access)?
//   3. Which address, so the next piece of work has a name.
//
// A trap is a queue item. A fault may be a lowering bug or may be the honest
// consequence of a missing import; the two are distinguished by where it lands,
// which is why the guest address is printed alongside the host one.
#include "lifted_runtime.h"
#include "lifted_loader.h"
#include "lifted_imports.h"
#include "lifted_crt_init.h"
#include "lifted_crt.h"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

// A return address no lifted body owns, so "the function returned normally"
// shows up as a trap naming THIS value rather than as an unexplained jump.
constexpr uint32_t OpensmacxBootReturnAddress = 0x00000BADU;

// _WinMain@16. The CRT would have called this, and since `start` is
// external_library and cannot be lifted, this is where a boot begins.
constexpr uint32_t OpensmacxWinMain = 0x0045F950U;

namespace {

// No longjmp, and that is deliberate. The oracle already established that
// longjmp out of a vectored handler is not something to rely on here, and the
// first version of this file proved it again by dumping core instead of
// reporting. Nothing needs to RESUME - the run is over either way - so both
// paths print what they know and end the process, which needs no unwinding at
// all.
volatile uint32_t g_trap_address = 0;
char g_trap_reason[256];
volatile long g_traps = 0;

// SURVEY MODE. Off by default, and the distinction is the whole point.
//
// Without it the boot converges one work item per run: it stops at the first
// trap, and the next thing to build is the only thing it can tell you. With 306
// unshimmed external_library functions that is 306 rebuild-and-run cycles to
// learn a list.
//
// With it, a trap at an address the CRT layer can NAME and has no shim for is
// logged and skipped - returned from as cdecl with EAX = 0 - so one run harvests
// the whole reachable frontier. Every other trap stays fatal, because a missing
// lowering or a dispatch into the middle of a body is not something to step over.
//
// The evidence this produces is WEAKER and is spelled differently so it cannot
// be quoted as a boot: a real stop prints BOOT-STOPPED-AT, a surveyed one prints
// SURVEY-REACHED. A survey frontier is a work queue. It is not a claim that the
// boot got that far, because every skipped function returned a lie.

// The generated trap calls this before printing anything. Returning would let
// it abort(), so on the fatal path this never returns.
void boot_trap_hook(uint32_t address, const char *reason) {
    g_trap_address = address;
    g_traps++;
    const char *name = opensmacx_crt_name(address);
    // A surveyed skip never reaches here: opensmacx_crt_dispatch answers the
    // address with a returning stub, so the trap is not entered at all. Which is
    // the right place for it - the trap is [[noreturn]] and has no CPU state to
    // pop a return address from, and the dispatcher has both.
    std::printf("BOOT-STOPPED-AT %#010x\n", unsigned(address));
    std::printf("boot:   %s\n", reason ? reason : "(no reason given)");
    if (name != nullptr) {
        std::printf("boot:   the CRT layer names this %s but has no shim; "
                    "--survey would log it and continue\n", name);
    } else {
        std::printf("boot: this is a work item - the lift has no body or no "
                    "lowering here\n");
    }
    std::fflush(stdout);
    ExitProcess(3);
}

volatile DWORD g_fault_code = 0;
volatile uintptr_t g_fault_pc = 0;
volatile uintptr_t g_fault_target = 0;

void report_fault(EXCEPTION_POINTERS *info) {
    g_fault_code = info->ExceptionRecord->ExceptionCode;
    g_fault_pc = uintptr_t(info->ExceptionRecord->ExceptionAddress);
    g_fault_target = info->ExceptionRecord->NumberParameters >= 2
                   ? uintptr_t(info->ExceptionRecord->ExceptionInformation[1])
                   : 0;
    // Say whether the faulting address is inside the guest image, because
    // "the lifted code read guest memory that is not set up yet" and "the
    // lifted code computed a wild pointer" are different problems and only the
    // address separates them.
    const uintptr_t image = uintptr_t(opensmacx_image);
    const bool in_image = g_fault_target >= image
                       && g_fault_target < image + OpensmacxImageSize;
    std::printf("boot: STOPPED by a FAULT %#010lx at host %#010lx\n",
                (unsigned long)g_fault_code, (unsigned long)g_fault_pc);
    std::printf("boot:   touching %#010lx, which is %s the guest image\n",
                (unsigned long)g_fault_target,
                in_image ? "INSIDE" : "OUTSIDE");
    if (in_image) {
        std::printf("boot:   guest address %#010x\n",
                    unsigned(uint32_t(g_fault_target - image)
                             + OpensmacxImageBase));
    }
    std::printf("boot: traps taken before this: %ld\n", g_traps);
    std::fflush(stdout);
}

LONG CALLBACK boot_veh(EXCEPTION_POINTERS *info) {
    const DWORD code = info->ExceptionRecord->ExceptionCode;
    // Leave the debugger's own breakpoints alone; everything else is the run
    // ending, and the run ending is the result.
    if (code == EXCEPTION_BREAKPOINT || code == EXCEPTION_SINGLE_STEP)
        return EXCEPTION_CONTINUE_SEARCH;
    report_fault(info);
    ExitProcess(4);
    return EXCEPTION_CONTINUE_SEARCH;
}

// Run BOTH initialiser arrays, in the order __cinit walks them.
//
// This used to locate one array by shape - "the longest run of dwords whose
// every entry is a catalogued ??__E address" - and that is wrong twice over,
// measured against the image:
//
//   .CRT$XI    4 live entries, ZERO of them named ??__E
//              ___onexitinit, ___initstdio, ___initmbctable, sub_64cb7a
//   .CRT$XC  434 live entries, 392 named ??__E, 42 NOT
//              sub_440f30, a run of twenty-eight sub_48d5xx, others
//
// So the shape search could see at most 392 of 438 and, worse, none of XI - and
// XI entry 0 is ___onexitinit, which allocates the atexit table that every C++
// initialiser in XC then registers with. That is why the boot stopped in
// __onexit reading a table nothing had built: not a missing __onexit shim, a
// missing XI walk.
//
// The bounds are now data, decoded from __cinit's own push immediates by
// tools/lift_whole_image.py and emitted into lifted_crt_init.h. The catalogued
// ??__E set is still there but only as a cross-check: it reports how many of the
// entries this walk ran are ones the catalogue calls initialisers, so a
// divergence between "what the image initialises" and "what we think an
// initialiser is" is visible rather than deciding the answer.
bool is_catalogued_initialiser(uint32_t address) {
    // The generated table is emitted sorted, so this is a binary search over
    // read-only data rather than a set built at startup.
    unsigned low = 0, high = OpensmacxCrtInitCount;
    while (low < high) {
        const unsigned mid = low + (high - low) / 2U;
        if (OpensmacxCrtInit[mid] == address) return true;
        if (OpensmacxCrtInit[mid] < address) low = mid + 1U;
        else high = mid;
    }
    return false;
}

unsigned run_dynamic_initialisers(OpensmacxStaticRecompileState &state,
                                  bool quiet) {
    unsigned total = 0;
    for (unsigned r = 0; r < OpensmacxCrtInitRangeCount; ++r) {
        const OpensmacxCrtInitRange &range = OpensmacxCrtInitRanges[r];
        // Announced BEFORE the walk, so a stop INSIDE a range is attributable
        // to it. The summary below only prints for a range that completed, and
        // the interesting stops are the ones that do not.
        if (!quiet) {
            std::printf("boot: entering .CRT$%s [%#010x,%#010x)\n",
                        range.name, unsigned(range.begin), unsigned(range.end));
            std::fflush(stdout);
        }
        unsigned ran = 0, catalogued = 0, null_entries = 0;
        for (uint32_t at = range.begin; at + 4U <= range.end; at += 4U) {
            const uint32_t target = opensmacx_mem32(at);
            // __initterm skips nulls rather than stopping, and the arrays are
            // NULL-terminated markers at both ends, so a null is normal.
            if (!target) {
                ++null_entries;
                continue;
            }
            if (is_catalogued_initialiser(target)) {
                ++catalogued;
            }
            // Each initialiser is cdecl with no arguments, so a return address
            // is all its frame needs.
            state.esp -= 4U;
            opensmacx_store32(state.esp, OpensmacxBootReturnAddress);
            opensmacx_dispatch(target)(state);
            ++ran;
        }
        if (!quiet) {
            std::printf("boot: .CRT$%s [%#010x,%#010x) ran %u, %u catalogued "
                        "??__E, %u null\n",
                        range.name, unsigned(range.begin), unsigned(range.end),
                        ran, catalogued, null_entries);
        }
        total += ran;
    }
    if (!total) {
        std::printf("boot: no initialiser ran - the ranges are empty, which "
                    "means lifted_crt_init.h is stale or wrong\n");
    }
    if (opensmacx_crt_surveyed()) {
        // Printed here and again at exit, because the number is the point of a
        // surveyed run and it is what makes the run non-evidence.
        std::printf("boot: SURVEYED %u CRT call(s) - every one returned 0, so "
                    "execution past the first is not the program\n",
                    opensmacx_crt_surveyed());
    }
    return total;
}

}  // namespace

int main(int argc, char **argv) {
    bool quiet = false;
    // The PE entry point is NOT a boot target and cannot be made one. It is
    // `start` at 0x00646c9d, whose recovery_state is external_library - the CRT
    // and Windows library code the lift calls and never translates, by written
    // policy. Dispatching it returns a trap stub, so booting from the entry
    // stops on its first instruction no matter what else is fixed.
    //
    // --at names a different starting function, and the one that matters is
    // _WinMain@16 at 0x0045f950: a game function, lifted, and the first thing
    // the CRT would have called. --args pushes that many zero words as its
    // stdcall arguments plus a return address the dispatcher has no body for,
    // so a clean return is distinguishable from a wild jump.
    uint32_t at = 0;
    int args = 0;
    bool winmain = false, run_init = false, survey = false;
    const char *cmdline = "";
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--quiet")) quiet = true;
        else if (!std::strcmp(argv[i], "--at") && i + 1 < argc)
            at = uint32_t(std::strtoul(argv[++i], nullptr, 0));
        else if (!std::strcmp(argv[i], "--args") && i + 1 < argc)
            args = int(std::strtol(argv[++i], nullptr, 0));
        else if (!std::strcmp(argv[i], "--winmain")) { winmain = true; at = OpensmacxWinMain; }
        else if (!std::strcmp(argv[i], "--cmdline") && i + 1 < argc)
            cmdline = argv[++i];
        else if (!std::strcmp(argv[i], "--init")) run_init = true;
        // Enumerate the CRT frontier rather than stopping at its first member.
        // Weaker evidence by construction - see lifted_crt.h - so it is opt-in
        // and its verdicts are spelled SURVEY-REACHED, never BOOT-STOPPED-AT.
        else if (!std::strcmp(argv[i], "--survey")) survey = true;
    }

    // Order is load-bearing, exactly as in lifted_main.cpp: the loader writes
    // the file's own IAT contents over the import slots, so binding must come
    // after it, and every slot holds zero until binding runs.
    opensmacx_load_image_or_die();
    opensmacx_bind_imports();
    // Before anything dispatches, so the first named-but-unimplemented CRT
    // routine is skipped rather than fatal.
    opensmacx_crt_survey(survey);

    OpensmacxStaticRecompileState state{};
    opensmacx_init_stack(state);

    const uint32_t entry = at ? at : opensmacx_entry_point();
    if (!quiet) {
        std::printf("boot: image %s\n", opensmacx_image_sha256());
        std::printf("boot: entry %#010x, esp %#010x\n",
                    unsigned(entry), unsigned(state.esp));
    }

    opensmacx_trap_hook = boot_trap_hook;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    PVOID cookie = AddVectoredExceptionHandler(1, boot_veh);

    if (run_init) {
        std::fflush(stdout);
        const unsigned ran = run_dynamic_initialisers(state, quiet);
        std::printf("boot: ran %u dynamic initialiser(s)\n", ran);
        std::fflush(stdout);
    }

    // A REAL WinMain frame. Handing it four zeros made the game parse a NULL
    // command line and trap inside _atoi, which is a property of the input and
    // not of the lift - so the arguments are the ones Windows would pass:
    // hInstance is the image base, hPrevInstance is 0 on Win32 by definition,
    // lpCmdLine points at a real (possibly empty) string in guest memory, and
    // nShowCmd is SW_SHOWNORMAL.
    if (winmain) {
        const uint32_t length = uint32_t(std::strlen(cmdline)) + 1U;
        // Just under the stack top, above where ESP will ever reach, so the
        // string is in guest memory the program can read and nothing the
        // program pushes will overwrite it.
        const uint32_t buffer = OpensmacxStackTop - 0x100U;
        for (uint32_t i = 0; i < length; ++i)
            *opensmacx_at(buffer + i) = (unsigned char)cmdline[i];
        const uint32_t frame[4] = {OpensmacxImageBase, 0U, buffer, 1U};
        for (int i = 3; i >= 0; --i) {
            state.esp -= 4;
            opensmacx_store32(state.esp, frame[i]);
        }
        state.esp -= 4;
        opensmacx_store32(state.esp, OpensmacxBootReturnAddress);
        if (!quiet)
            std::printf("boot: WinMain(hInstance=%#010x, 0, lpCmdLine=%#010x "
                        "\"%s\", SW_SHOWNORMAL), return %#010x\n",
                        unsigned(OpensmacxImageBase), unsigned(buffer), cmdline,
                        unsigned(OpensmacxBootReturnAddress));
    } else if (at) {
        for (int i = 0; i < args; ++i) {
            state.esp -= 4;
            opensmacx_store32(state.esp, 0);
        }
        state.esp -= 4;
        opensmacx_store32(state.esp, OpensmacxBootReturnAddress);
        if (!quiet)
            std::printf("boot: starting at %#010x with %d zero argument(s), "
                        "return address %#010x\n",
                        unsigned(entry), args,
                        unsigned(OpensmacxBootReturnAddress));
    }

    OpensmacxLiftedFunction body = opensmacx_dispatch(entry);
    if (!body) {
        std::printf("boot: STOPPED - no lifted body for the entry point %#010x\n",
                    unsigned(entry));
        return 2;
    }

    std::fflush(stdout);
    body(state);
    RemoveVectoredExceptionHandler(cookie);
    std::printf("boot: the entry point RETURNED, eax=%#010x\n",
                unsigned(state.eax));
    std::printf("boot: stack guard %s\n",
                opensmacx_stack_guard_intact() ? "intact" : "BREACHED");
    return 0;
}
