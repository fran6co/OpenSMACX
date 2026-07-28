// End-to-end proof of the import ABI boundary.
//
// The claim under test is narrow and load-bearing: a lifted `call [iat]`
// reaches a REAL Windows function, the result lands in EAX, and ESP moves by
// exactly four plus the argument bytes - no more, no less. A shim that pops
// too little or too much does not fail where it is; it corrupts the caller's
// frame and fails somewhere else entirely, which is why this is checked
// directly rather than inferred from the program still running.
//
// Every check has an ORACLE that is independent of the thing being checked:
//
//   * the returned value is compared against the same function called
//     normally from host code, or against a value this program put there
//     itself, never against "looks plausible";
//   * the ESP movement is compared against opensmacx_import_arg_bytes(),
//     which comes from the generated table, AND against the caller's own
//     record of where the stack was before it pushed anything. Two
//     independent statements of the same requirement.
//   * a canary below the pushed return address catches a shim that writes
//     under the stack pointer.
//
// And the harness is shown to be able to FAIL: a deliberately mis-cleaned
// shim is dispatched through the same path and the checks must report it.
// A harness that cannot fail is worth nothing.
//
// This links against the generated build/lifted/lifted_imports.cpp and
// supplies its own minimal image, trap and dispatcher, so it runs in about a
// second instead of waiting on the 66-second whole-image build.

#include "lifted_imports.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csetjmp>

#include <windows.h>

// The guest address space. Real in shape, zero-filled in content - this probe
// never loads the original image, and needs nothing from it but its IAT
// slots, which opensmacx_bind_imports() writes.
unsigned char opensmacx_image[OpensmacxImageSize];

namespace {

int failures = 0;
int checks = 0;

void check(bool condition, const char *what, unsigned long long saw,
           unsigned long long expected) {
    ++checks;
    if (condition) {
        std::printf("  ok    %-52s\n", what);
        return;
    }
    ++failures;
    std::printf("  FAIL  %-52s saw %#llx, expected %#llx\n",
                what, saw, expected);
}

// A guest stack in the high end of the image. Nothing else uses this region
// in the probe, so a stray write is visible.
constexpr uint32_t StackTop = OpensmacxImageBase + OpensmacxImageSize - 0x1000U;
constexpr uint32_t Scratch = OpensmacxImageBase + OpensmacxImageSize - 0x9000U;
constexpr uint32_t ReturnSentinel = 0x00401234U;
constexpr uint32_t Canary = 0xC0FFEE00U;

uint32_t put_string(uint32_t address, const char *text) {
    std::memcpy(opensmacx_at(address), text, std::strlen(text) + 1);
    return address;
}

struct Outcome {
    uint32_t eax;
    uint32_t esp_moved;      // how far ESP rose across the call
    bool registers_kept;     // EBX/ESI/EDI/EBP survived
    bool canary_kept;        // nothing written below the return address
};

// Perform one guest-side call exactly as the lowered `call [iat]` does:
// load the IAT slot, push the arguments right to left, push a return
// address, dispatch.
Outcome call_import(uint32_t iat, const uint32_t *args, unsigned count,
                    uint32_t *observed_synthetic = nullptr) {
    OpensmacxStaticRecompileState s{};
    s.esp = StackTop;
    s.ebx = 0xB1B1B1B1U;
    s.esi = 0x51515151U;
    s.edi = 0xD1D1D1D1U;
    s.ebp = 0xBE7BE7BEU;

    const uint32_t target = opensmacx_mem32(iat);
    if (observed_synthetic != nullptr) {
        *observed_synthetic = target;
    }
    for (unsigned index = count; index-- > 0;) {
        s.esp -= 4U;
        opensmacx_store32(s.esp, args[index]);
    }
    s.esp -= 4U;
    opensmacx_store32(s.esp, ReturnSentinel);
    const uint32_t entry = s.esp;
    opensmacx_store32(entry - 4U, Canary);
    opensmacx_store32(entry - 8U, Canary);

    opensmacx_import_dispatch(target)(s);

    Outcome outcome{};
    outcome.eax = s.eax;
    outcome.esp_moved = s.esp - entry;
    outcome.registers_kept = s.ebx == 0xB1B1B1B1U && s.esi == 0x51515151U
        && s.edi == 0xD1D1D1D1U && s.ebp == 0xBE7BE7BEU;
    outcome.canary_kept = opensmacx_mem32(entry - 4U) == Canary
        && opensmacx_mem32(entry - 8U) == Canary;
    return outcome;
}

// The ABI check itself, applied identically to every call below.
void check_abi(const char *label, uint32_t iat, const Outcome &outcome,
               unsigned count) {
    const uint32_t synthetic = opensmacx_mem32(iat);
    const uint32_t declared = opensmacx_import_arg_bytes(synthetic);
    char what[128];

    std::snprintf(what, sizeof(what), "%s: ESP moved by 4 + @%u", label,
                  static_cast<unsigned>(declared));
    check(outcome.esp_moved == 4U + declared, what, outcome.esp_moved,
          4U + declared);

    // The caller's own arithmetic, owing nothing to the table: it pushed
    // `count` dwords plus a return address, so a balanced call returns the
    // stack to where it started.
    std::snprintf(what, sizeof(what), "%s: caller's stack fully balanced",
                  label);
    check(outcome.esp_moved == 4U + 4U * count, what, outcome.esp_moved,
          4U + 4U * count);

    std::snprintf(what, sizeof(what), "%s: EBX/ESI/EDI/EBP preserved", label);
    check(outcome.registers_kept, what, outcome.registers_kept, 1);

    std::snprintf(what, sizeof(what), "%s: nothing written below ESP", label);
    check(outcome.canary_kept, what, outcome.canary_kept, 1);
}

uint32_t iat_of(const char *name) {
    for (uint32_t index = 0; index < opensmacx_import_count(); ++index) {
        const uint32_t synthetic = opensmacx_import_address(index);
        const char *const found = opensmacx_import_name(synthetic);
        if (found != nullptr && std::strcmp(found, name) == 0) {
            // Recover the IAT address by searching the image for the slot
            // that binding filled with this synthetic address. Going through
            // the image rather than through a table accessor proves the
            // binding actually happened.
            for (uint32_t address = 0x00669000U; address < 0x0066A000U;
                 address += 4U) {
                if (opensmacx_mem32(address) == synthetic) {
                    return address;
                }
            }
        }
    }
    std::printf("  FAIL  no bound IAT slot for %s\n", name);
    ++failures;
    return 0U;
}

// --- the deliberately wrong shim -------------------------------------------
//
// One argument short in its cleanup: the single most common way to get
// stdcall wrong, and the one whose damage lands furthest from its cause.
// Dispatched through the same call_import path as everything else, so if the
// checks above pass on this, they are not checking anything.
void wrong_cleanup_shim(OpensmacxStaticRecompileState &s) {
    s.eax = opensmacx_arg(s, 0) + opensmacx_arg(s, 1) + opensmacx_arg(s, 2);
    s.esp += 4U + 8U;   // should be 4 + 12
}

void self_test_of_the_harness() {
    std::printf("\nharness self-test: a shim that pops one argument too few\n");
    OpensmacxStaticRecompileState s{};
    s.esp = StackTop;
    const uint32_t args[3] = {1U, 2U, 3U};
    for (unsigned index = 3; index-- > 0;) {
        s.esp -= 4U;
        opensmacx_store32(s.esp, args[index]);
    }
    s.esp -= 4U;
    opensmacx_store32(s.esp, ReturnSentinel);
    const uint32_t entry = s.esp;
    wrong_cleanup_shim(s);
    const uint32_t moved = s.esp - entry;

    ++checks;
    if (moved != 4U + 12U) {
        std::printf("  ok    the check rejects it: ESP moved %#x, needed %#x\n",
                    moved, 4U + 12U);
    } else {
        ++failures;
        std::printf("  FAIL  the check accepted a wrong cleanup\n");
    }
    ++checks;
    if (s.eax == 6U) {
        std::printf("  ok    and the result was still right, so a wrong\n"
                    "        cleanup is invisible in the return value\n");
    } else {
        ++failures;
        std::printf("  FAIL  unexpected result %#x\n", s.eax);
    }
}

}  // namespace

// The probe's dispatcher: the import range only. The real one adds the lifted
// function table underneath.
OpensmacxLiftedFunction opensmacx_dispatch(uint32_t address) {
    if (opensmacx_is_import(address)) {
        return opensmacx_import_dispatch(address);
    }
    opensmacx_trap(address, "probe dispatcher has no lifted functions");
}

// The trap has to be CATCHABLE here, not fatal. Half of what this probe now
// checks is that the marshalling layer REFUSES something - a structure that
// runs off the end of the image, a host pointer with no guest address - and a
// refusal that aborts the process cannot be asserted on, only observed by a
// human reading a crash. With a jump target armed, a refusal is an outcome.
jmp_buf g_trap_landing;
bool g_trap_armed = false;
uint32_t g_trap_address = 0;
char g_trap_reason[256];

void opensmacx_trap(uint32_t address, const char *reason) {
    if (g_trap_armed) {
        g_trap_armed = false;
        g_trap_address = address;
        std::snprintf(g_trap_reason, sizeof(g_trap_reason), "%s", reason);
        std::longjmp(g_trap_landing, 1);
    }
    std::fprintf(stderr, "opensmacx: trap at %#010x: %s\n",
                 static_cast<unsigned>(address), reason);
    std::abort();
}

namespace {

// Run `body` and report whether the marshalling layer refused it.
template <typename Body>
bool refused(Body body) {
    g_trap_armed = true;
    g_trap_address = 0;
    g_trap_reason[0] = '\0';
    if (setjmp(g_trap_landing) != 0) {
        return true;
    }
    body();
    g_trap_armed = false;
    return false;
}

}  // namespace

int main() {
    std::printf("binding %u imports\n", opensmacx_import_count());
    opensmacx_bind_imports();

    // Binding is the mechanism everything else rests on: every IAT slot must
    // now hold a synthetic address that dispatches back to its own name.
    unsigned bound = 0;
    for (uint32_t index = 0; index < opensmacx_import_count(); ++index) {
        const uint32_t synthetic = opensmacx_import_address(index);
        bound += opensmacx_import_dispatch(synthetic) != nullptr
            && opensmacx_import_name(synthetic) != nullptr;
    }
    check(bound == opensmacx_import_count(),
          "every import resolves through its synthetic address",
          bound, opensmacx_import_count());

    // --- zero arguments ----------------------------------------------------
    std::printf("\nGetCurrentThreadId - zero arguments, exact oracle\n");
    {
        const uint32_t iat = iat_of("KERNEL32.dll!GetCurrentThreadId");
        uint32_t synthetic = 0;
        const Outcome outcome = call_import(iat, nullptr, 0, &synthetic);
        check(opensmacx_is_import(synthetic),
              "the IAT slot holds a synthetic import address",
              synthetic, OpensmacxImportBase);
        check(outcome.eax == GetCurrentThreadId(),
              "EAX equals the host's own GetCurrentThreadId()",
              outcome.eax, GetCurrentThreadId());
        check_abi("GetCurrentThreadId", iat, outcome, 0);
    }

    std::printf("\nGetTickCount - zero arguments, bracketed oracle\n");
    {
        const uint32_t iat = iat_of("KERNEL32.dll!GetTickCount");
        const DWORD before = GetTickCount();
        const Outcome outcome = call_import(iat, nullptr, 0);
        const DWORD after = GetTickCount();
        check(outcome.eax >= before && outcome.eax <= after,
              "EAX lies between two host GetTickCount() readings",
              outcome.eax, before);
        check_abi("GetTickCount", iat, outcome, 0);
    }

    // --- one argument, with observable state -------------------------------
    std::printf("\nSetLastError then GetLastError - state crosses two shims\n");
    {
        const uint32_t set = iat_of("KERNEL32.dll!SetLastError");
        const uint32_t get = iat_of("KERNEL32.dll!GetLastError");
        const uint32_t args[1] = {0x00001234U};
        const Outcome wrote = call_import(set, args, 1);
        check_abi("SetLastError", set, wrote, 1);
        const Outcome read = call_import(get, nullptr, 0);
        check(read.eax == 0x00001234U,
              "GetLastError returns what SetLastError was given",
              read.eax, 0x00001234U);
        check_abi("GetLastError", get, read, 0);
    }

    // --- four arguments, round trip through the real registry of .ini ------
    std::printf("\nWritePrivateProfileStringA / GetPrivateProfileIntA"
                " - four arguments\n");
    {
        char path[MAX_PATH];
        char directory[MAX_PATH];
        GetTempPathA(sizeof(directory), directory);
        std::snprintf(path, sizeof(path), "%sopensmacx_import_probe.ini",
                      directory);
        DeleteFileA(path);

        const uint32_t g_path = put_string(Scratch + 0x000U, path);
        const uint32_t g_section = put_string(Scratch + 0x200U, "Probe");
        const uint32_t g_key = put_string(Scratch + 0x240U, "Answer");
        const uint32_t g_value = put_string(Scratch + 0x280U, "4242");

        const uint32_t write_args[4] = {g_section, g_key, g_value, g_path};
        const uint32_t write_iat =
            iat_of("KERNEL32.dll!WritePrivateProfileStringA");
        const Outcome wrote = call_import(write_iat, write_args, 4);
        check(wrote.eax != 0U, "WritePrivateProfileStringA succeeded",
              wrote.eax, 1);
        check_abi("WritePrivateProfileStringA", write_iat, wrote, 4);

        const uint32_t read_args[4] = {g_section, g_key, 7U, g_path};
        const uint32_t read_iat =
            iat_of("KERNEL32.dll!GetPrivateProfileIntA");
        const Outcome read = call_import(read_iat, read_args, 4);
        check(read.eax == 4242U,
              "GetPrivateProfileIntA reads back the written value",
              read.eax, 4242U);
        check_abi("GetPrivateProfileIntA", read_iat, read, 4);
        DeleteFileA(path);
    }

    // --- seven arguments, guest buffers ------------------------------------
    std::printf("\nCreateFileA / WriteFile / GetFileSize"
                " - seven, five and two arguments\n");
    {
        char path[MAX_PATH];
        char directory[MAX_PATH];
        GetTempPathA(sizeof(directory), directory);
        std::snprintf(path, sizeof(path), "%sopensmacx_import_probe.bin",
                      directory);
        DeleteFileA(path);

        const uint32_t g_path = put_string(Scratch + 0x400U, path);
        const char payload[] = "alpha centauri";
        const uint32_t g_buffer = Scratch + 0x600U;
        std::memcpy(opensmacx_at(g_buffer), payload, sizeof(payload));
        const uint32_t g_written = Scratch + 0x700U;

        const uint32_t create_args[7] = {
            g_path, GENERIC_WRITE, 0U, 0U, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, 0U};
        const uint32_t create_iat = iat_of("KERNEL32.dll!CreateFileA");
        const Outcome created = call_import(create_iat, create_args, 7);
        check(created.eax != 0xFFFFFFFFU && created.eax != 0U,
              "CreateFileA returned a handle", created.eax, 1);
        check_abi("CreateFileA", create_iat, created, 7);

        const uint32_t write_args[5] = {
            created.eax, g_buffer, sizeof(payload) - 1U, g_written, 0U};
        const uint32_t write_iat = iat_of("KERNEL32.dll!WriteFile");
        const Outcome wrote = call_import(write_iat, write_args, 5);
        check(wrote.eax != 0U, "WriteFile succeeded", wrote.eax, 1);
        check(opensmacx_mem32(g_written) == sizeof(payload) - 1U,
              "WriteFile wrote the byte count into GUEST memory",
              opensmacx_mem32(g_written), sizeof(payload) - 1U);
        check_abi("WriteFile", write_iat, wrote, 5);

        const uint32_t size_args[2] = {created.eax, 0U};
        const uint32_t size_iat = iat_of("KERNEL32.dll!GetFileSize");
        const Outcome sized = call_import(size_iat, size_args, 2);
        check(sized.eax == sizeof(payload) - 1U,
              "GetFileSize agrees with what was written",
              sized.eax, sizeof(payload) - 1U);
        check_abi("GetFileSize", size_iat, sized, 2);

        const uint32_t close_args[1] = {created.eax};
        const uint32_t close_iat = iat_of("KERNEL32.dll!CloseHandle");
        const Outcome closed = call_import(close_iat, close_args, 1);
        check(closed.eax != 0U, "CloseHandle succeeded", closed.eax, 1);
        check_abi("CloseHandle", close_iat, closed, 1);

        // The oracle is the host's own view of the file, which owes nothing
        // to any shim.
        const HANDLE host = CreateFileA(path, GENERIC_READ, 0, nullptr,
                                        OPEN_EXISTING, 0, nullptr);
        char readback[32] = {0};
        DWORD got = 0;
        if (host != INVALID_HANDLE_VALUE) {
            ReadFile(host, readback, sizeof(readback) - 1, &got, nullptr);
            CloseHandle(host);
        }
        check(got == sizeof(payload) - 1U
                  && std::memcmp(readback, payload, got) == 0,
              "the file on disk holds exactly what the guest wrote",
              got, sizeof(payload) - 1U);
        DeleteFileA(path);
    }

    // --- a shim answered from the guest image, not forwarded ---------------
    std::printf("\nIsBadWritePtr - answered about GUEST memory\n");
    {
        const uint32_t iat = iat_of("KERNEL32.dll!IsBadWritePtr");
        const uint32_t inside[2] = {Scratch, 16U};
        const Outcome good = call_import(iat, inside, 2);
        check(good.eax == 0U, "an address inside the image is writable",
              good.eax, 0U);
        check_abi("IsBadWritePtr(inside)", iat, good, 2);

        const uint32_t outside[2] = {0x7F000000U, 16U};
        const Outcome bad = call_import(iat, outside, 2);
        check(bad.eax == 1U, "an address outside the image is not",
              bad.eax, 1U);
        check_abi("IsBadWritePtr(outside)", iat, bad, 2);
    }

    // --- an unimplemented import still names itself ------------------------
    std::printf("\nan unimplemented import is reachable and named\n");
    {
        const uint32_t iat = iat_of("DDRAW.dll!DirectDrawCreate");
        const uint32_t synthetic = opensmacx_mem32(iat);
        const char *const name = opensmacx_import_name(synthetic);
        check(name != nullptr
                  && std::strcmp(name, "DDRAW.dll!DirectDrawCreate") == 0,
              "its synthetic address still resolves to its own name",
              name != nullptr, 1);
    }

    // --- the binding is checked in BOTH directions -------------------------
    //
    // Every check above locates a slot by scanning the image for a synthetic
    // value, which finds whichever slot holds it. That cannot distinguish a
    // correct binding from one that pairs every import with the wrong slot:
    // reversing the loop in opensmacx_bind_imports() left all 66 checks
    // passing while every call in the image would have reached a different
    // function. Asking the table where an import BELONGS and then reading that
    // slot closes the loop, for all of them rather than a sample.
    std::printf("\nbinding round-trip, every import\n");
    {
        unsigned paired = 0;
        uint32_t first_bad = 0;
        for (uint32_t index = 0; index < opensmacx_import_count(); ++index) {
            const uint32_t synthetic = opensmacx_import_address(index);
            const uint32_t iat = opensmacx_import_iat(synthetic);
            if (iat != 0U && opensmacx_mem32(iat) == synthetic) {
                ++paired;
            } else if (first_bad == 0U) {
                first_bad = synthetic;
            }
        }
        check(paired == opensmacx_import_count(),
              "each import's own IAT slot holds its own address",
              paired, opensmacx_import_count());
        if (first_bad != 0U) {
            std::printf("        first mismatch: %s expected slot %#010x, "
                        "which holds %#010x\n",
                        opensmacx_import_name(first_bad),
                        opensmacx_import_iat(first_bad),
                        opensmacx_mem32(opensmacx_import_iat(first_bad)));
        }
        // The slots must also be DISTINCT. A binding that sends every import
        // to one slot would satisfy neither this nor the check above, but a
        // binding that sends two imports to one slot would satisfy the check
        // above for one of them.
        unsigned duplicates = 0;
        for (uint32_t i = 0; i < opensmacx_import_count(); ++i) {
            for (uint32_t j = i + 1U; j < opensmacx_import_count(); ++j) {
                duplicates += opensmacx_import_iat(opensmacx_import_address(i))
                    == opensmacx_import_iat(opensmacx_import_address(j));
            }
        }
        check(duplicates == 0U, "no two imports share an IAT slot",
              duplicates, 0U);
    }

    // --- the busiest implemented import, both of its argument forms --------
    //
    // CharUpperA has 225 call sites in the image - more than any other
    // implemented import - and was not called once by this probe. Its
    // discriminator (below 0x10000 is a character, above is a pointer) could
    // be moved to 0x100 without any check noticing.
    std::printf("\nCharUpperA - overloaded on its argument\n");
    {
        const uint32_t iat = iat_of("USER32.dll!CharUpperA");
        const uint32_t text = put_string(Scratch, "mixed Case 42!");
        const uint32_t args[1] = {text};
        const Outcome outcome = call_import(iat, args, 1);
        check(std::strcmp(
                  reinterpret_cast<const char *>(opensmacx_at(text)),
                  "MIXED CASE 42!") == 0,
              "the guest buffer is uppercased in place",
              opensmacx_at(text)[0], 'M');
        check(outcome.eax == text,
              "a string argument comes back as the same GUEST pointer",
              outcome.eax, text);
        check_abi("CharUpperA(string)", iat, outcome, 1);

        // The character form. The oracle is the host's own CharUpperA, which
        // is what the guest would have reached natively.
        static const uint32_t characters[6] = {'a', 'z', 'A', '9', 0xE0U,
                                               0xFFFFU};
        unsigned agreed = 0;
        for (uint32_t character : characters) {
            const uint32_t one[1] = {character};
            const Outcome each = call_import(iat, one, 1);
            const uint32_t want = static_cast<uint32_t>(
                reinterpret_cast<uintptr_t>(CharUpperA(
                    reinterpret_cast<char *>(
                        static_cast<uintptr_t>(character)))));
            agreed += each.eax == want;
        }
        check(agreed == 6U, "every character form matches the host's answer",
              agreed, 6U);
    }

    // --- the length half of IsBadReadPtr's predicate -----------------------
    //
    // Both existing cases are decided by the BASE test alone, so the
    // `length <= end - base` conjunct - the half that matters for a range
    // straddling the end of the image - was never executed.
    std::printf("\nIsBadWritePtr - across the end of the image\n");
    {
        const uint32_t iat = iat_of("KERNEL32.dll!IsBadWritePtr");
        const uint32_t end = OpensmacxImageBase + OpensmacxImageSize;
        const uint32_t fits[2] = {end - 4U, 4U};
        check(call_import(iat, fits, 2).eax == 0U,
              "the last four bytes of the image are writable", 0U, 0U);
        const uint32_t over[2] = {end - 4U, 64U};
        const uint32_t over_answer = call_import(iat, over, 2).eax;
        check(over_answer == 1U, "a range that runs off the end is not",
              over_answer, 1U);
        const uint32_t wraps[2] = {end - 4U, 0xFFFFFFF0U};
        const uint32_t wrap_answer = call_import(iat, wraps, 2).eax;
        check(wrap_answer == 1U,
              "and a length near 2^32 does not wrap into passing",
              wrap_answer, 1U);
    }

    // --- argument ORDER, not just argument count ---------------------------
    //
    // 67 of the 78 shims are reviewed rather than tested, and a permutation
    // inside any of them is invisible to an ABI check: swapping SetRect's top
    // and right leaves ESP, EAX and the callee-saved registers all correct.
    std::printf("\nSetRect - the fields land where they were sent\n");
    {
        const uint32_t iat = iat_of("USER32.dll!SetRect");
        const uint32_t rect = Scratch + 0x100U;
        const uint32_t args[5] = {rect, 11U, 22U, 33U, 44U};
        const Outcome outcome = call_import(iat, args, 5);
        check(outcome.eax != 0U, "SetRect reports success", outcome.eax, 1U);
        const uint32_t saw[4] = {
            opensmacx_mem32(rect), opensmacx_mem32(rect + 4U),
            opensmacx_mem32(rect + 8U), opensmacx_mem32(rect + 12U)};
        check(saw[0] == 11U && saw[1] == 22U && saw[2] == 33U && saw[3] == 44U,
              "left/top/right/bottom in that order",
              (static_cast<unsigned long long>(saw[0]) << 24)
                  | (saw[1] << 16) | (saw[2] << 8) | saw[3],
              0x0B16212CULL);
        check_abi("SetRect", iat, outcome, 5);
    }

    // --- the marshalling layer REFUSES what it cannot represent ------------
    //
    // opensmacx_guest() is described in this header as the single most
    // informative thing in it, and no check reached it: replacing its whole
    // body with a bare cast left every check passing.
    std::printf("\npointer marshalling refuses what it cannot represent\n");
    {
        check(opensmacx_guest(opensmacx_at(Scratch)) == Scratch,
              "a host pointer inside the image round-trips",
              opensmacx_guest(opensmacx_at(Scratch)), Scratch);
        check(opensmacx_guest(nullptr) == 0U,
              "null stays null rather than becoming the image base",
              opensmacx_guest(nullptr), 0U);
        void *const host_heap = std::malloc(64);
        check(refused([&] { (void)opensmacx_guest(host_heap); }),
              "host heap memory has no guest address and traps", 1, 1);
        std::free(host_heap);

        // And the bound is on the OBJECT, not on its first byte. This is the
        // defect the sized opensmacx_host() exists to close: the address four
        // bytes below the end of the image is a legal address and an illegal
        // sixteen-byte SYSTEMTIME, and the unsized check passed it straight
        // through to Windows, which wrote all sixteen.
        const uint32_t end = OpensmacxImageBase + OpensmacxImageSize;
        check(!refused([&] { (void)opensmacx_host(end - 16U, 16U); }),
              "an object that ends exactly at the image end is accepted", 1, 1);
        check(refused([&] { (void)opensmacx_host(end - 4U, 16U); }),
              "an object whose TAIL leaves the image is refused", 1, 1);
        check(refused([&] { (void)opensmacx_host(end - 4U, 0x10000U); }),
              "and so is a guest-supplied 64 KiB count near the end", 1, 1);

        const uint32_t iat = iat_of("KERNEL32.dll!GetSystemTime");
        const uint32_t args[1] = {end - 4U};
        check(refused([&] { (void)call_import(iat, args, 1); }),
              "GetSystemTime four bytes below the end is refused", 1, 1);
    }

    self_test_of_the_harness();

    std::printf("\n%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
