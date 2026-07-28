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

// The generated trap calls this before printing anything. Returning would let
// it abort(), so this never returns.
void boot_trap_hook(uint32_t address, const char *reason) {
    g_trap_address = address;
    g_traps++;
    std::printf("boot: STOPPED by a TRAP at guest %#010x\n", unsigned(address));
    std::printf("boot:   %s\n", reason ? reason : "(no reason given)");
    std::printf("boot: this is a work item - the lift has no body or no "
                "lowering here\n");
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

// Find the image's OWN .CRT$XC pointer array and run it in ITS order.
//
// MSVC records the C++ dynamic initialisers as an array of function pointers
// that CRT startup walks. Address order is NOT initialisation order, so the
// array is located rather than reconstructed: it is the longest run of dwords
// in the image whose every non-zero entry is a catalogued `??__E` address.
// Nothing else in a 6.3 MB image looks like that, and the run's length is
// reported so a wrong guess is visible instead of silent.
bool is_initialiser(uint32_t address) {
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
    uint32_t best_start = 0, best_length = 0;
    uint32_t start = 0, length = 0;
    for (uint32_t at = OpensmacxImageBase;
         at + 4U <= OpensmacxImageBase + OpensmacxImageSize; at += 4U) {
        if (is_initialiser(opensmacx_mem32(at))) {
            if (!length) start = at;
            ++length;
            if (length > best_length) { best_length = length; best_start = start; }
        } else {
            length = 0;
        }
    }
    if (!best_length) {
        std::printf("boot: found no .CRT initialiser array\n");
        return 0;
    }
    if (!quiet)
        std::printf("boot: .CRT initialiser array at %#010x, %u of %u "
                    "catalogued initialisers\n",
                    unsigned(best_start), unsigned(best_length),
                    OpensmacxCrtInitCount);
    for (uint32_t i = 0; i < best_length; ++i) {
        const uint32_t target = opensmacx_mem32(best_start + 4U * i);
        // Each initialiser is cdecl with no arguments, so a return address is
        // all its frame needs.
        state.esp -= 4U;
        opensmacx_store32(state.esp, OpensmacxBootReturnAddress);
        opensmacx_dispatch(target)(state);
    }
    return unsigned(best_length);
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
    bool winmain = false, run_init = false;
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
    }

    // Order is load-bearing, exactly as in lifted_main.cpp: the loader writes
    // the file's own IAT contents over the import slots, so binding must come
    // after it, and every slot holds zero until binding runs.
    opensmacx_load_image_or_die();
    opensmacx_bind_imports();

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
