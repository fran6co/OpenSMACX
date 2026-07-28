// Implementation of the lifted-vs-original differential oracle.
// The design, and the three dead ends that shaped it, are in lifted_oracle.h.

#include "lifted_oracle.h"

#include <windows.h>

#include <csetjmp>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "lifted_runtime.h"   // opensmacx_image, opensmacx_dispatch
#include "lifted_tls.h"       // opensmacx_tib_reset
#include "lifted_string.h"    // OpensmacxDirectionFlag - the lift's DF

// The oracle hard-codes its own copy of the guest span so it can size the two
// sides before the generated tree is included. That is fine only while the two
// agree: restore_from_pristine() and the whole-image memcmp both walk
// OracleImageSize bytes of opensmacx_image, so a regenerated tree with a
// SMALLER image would make both of them run off the end - silently, since
// opensmacx_image is declared as an incomplete type. Sibling agents regenerate
// that tree routinely.
static_assert(OracleImageSize == OpensmacxImageSize,
              "the oracle's guest span no longer matches the generated tree; "
              "regenerate or update OracleImageSize");
static_assert(OracleImageBase == OpensmacxImageBase,
              "the oracle's guest base no longer matches the generated tree");
#include "lifted_x87.h"       // opensmacx_x87

// ---------------------------------------------------------------------------
// State shared with the assembly runner.
//
// These are globals rather than parameters on purpose: the runner switches ESP
// to the guest stack, so between that switch and the return there is no frame
// to hold anything, and every value it needs has to be reachable by absolute
// address.
// ---------------------------------------------------------------------------

extern "C" {
uint32_t oracle_in[9] = {};    // eax ecx edx ebx esp ebp esi edi eflags
uint32_t oracle_out[10] = {};  // ... [8] = raw lahf/seto, [9] = full EFLAGS
uint32_t oracle_target = 0;
uint32_t oracle_host_esp = 0;
uint32_t oracle_fault = 0;
uint32_t oracle_fault_addr = 0;
uint32_t oracle_fault_data = 0;
// Whether the exception carried a faulting address at all. Without this,
// fault_data == 0 is indistinguishable between "faulted at address 0" and
// "this exception has no address", and the second was being read as the first.
uint32_t oracle_fault_has_address = 0;
uint32_t oracle_saved_fs0 = 0;
uint32_t oracle_steps = 0;
uint32_t oracle_budget = 0;
uint16_t oracle_x87_cw = OpensmacxX87DefaultControl;
// The 108-byte 32-bit protected-mode FSAVE image of the ORIGINAL's FPU, filled
// on the way out of a completed run. It lives in HOST memory, not on the guest
// stack: 108 bytes written where the whole-image memcmp can see them would be
// a memory divergence manufactured by the measurement.
//
// Verified on this host rather than assumed: FSAVE writes ST(0) at offset 28
// and ST(7) at 98 - stack order, not physical-register order - so ST(i) is at
// 28 + 10*i and needs no TOP adjustment. `--selftest` re-checks it every run,
// because the whole x87 comparison is wrong by exactly one rotation if it is
// ever untrue.
alignas(16) uint8_t oracle_x87_save[108] = {};
}

extern "C" void oracle_run_original(void);
extern "C" void oracle_return_point(void);
extern "C" void oracle_abort_point(void);
extern "C" void oracle_longjmp_thunk(void);

// The two sentinels the handler puts in oracle_fault. Neither can collide with
// a real NTSTATUS, which always has the top bits set.
constexpr uint32_t OracleFaultBudget = 0x0B0DE700U;
constexpr uint32_t OracleFaultWatchdog = 0x0DEAD100U;

// ---------------------------------------------------------------------------
// The runner.
//
// Entry: load the seeded registers, set the flags, switch ESP to the guest
// stack and jump. The guest's own `ret` lands on oracle_return_point, whose
// address the seed already planted at [ESP].
//
// Exit: save every register BEFORE touching anything, then read the flags with
// lahf/seto rather than pushfl - pushfl would write four bytes of the guest
// stack that the lifted side never writes, manufacturing a memory divergence
// out of the measurement itself.
// ---------------------------------------------------------------------------

asm(
".text\n"
".globl _oracle_run_original\n"
"_oracle_run_original:\n"
"  pushl %ebp\n"
"  pushl %ebx\n"
"  pushl %esi\n"
"  pushl %edi\n"
"  movl %esp, _oracle_host_esp\n"
"  movl $0, _oracle_fault\n"
"  movl $0, _oracle_steps\n"
"  movl %fs:0, %eax\n"
"  movl %eax, _oracle_saved_fs0\n"
"  fninit\n"
"  fldcw _oracle_x87_cw\n"
"  movl _oracle_in+0,  %eax\n"
"  movl _oracle_in+4,  %ecx\n"
"  movl _oracle_in+8,  %edx\n"
"  movl _oracle_in+12, %ebx\n"
"  movl _oracle_in+20, %ebp\n"
"  movl _oracle_in+24, %esi\n"
"  movl _oracle_in+28, %edi\n"
"  pushl _oracle_in+32\n"
"  popfl\n"
"  movl _oracle_in+16, %esp\n"
"  jmp *_oracle_target\n"
".globl _oracle_return_point\n"
"_oracle_return_point:\n"
"  movl %eax, _oracle_out+0\n"
"  lahf\n"
"  seto %al\n"
"  movl %eax, _oracle_out+32\n"
"  movl %ecx, _oracle_out+4\n"
"  movl %edx, _oracle_out+8\n"
"  movl %ebx, _oracle_out+12\n"
"  movl %esp, _oracle_out+16\n"
"  movl %ebp, _oracle_out+20\n"
"  movl %esi, _oracle_out+24\n"
"  movl %edi, _oracle_out+28\n"
// DF cannot be read with lahf, and DF is modelled by the lift and read by all
// 305 string sites in the image. Capturing it needs pushfl, and pushfl on the
// GUEST stack would write four bytes the lifted side never writes and
// manufacture a memory divergence - which is why lahf was chosen originally.
// Switching to the HOST stack first has neither problem: mov does not touch
// the flags, so the word pushed here is still the guest's, and it lands
// somewhere the comparison never looks.
"  movl _oracle_host_esp, %esp\n"
"  pushfl\n"
"  popl _oracle_out+36\n"
// The x87 register file. 58 of the passing functions use the FPU and none of
// them compared a single bit of it, so a divergence confined to ST(0) - or a
// push that never happened - passed. fnsave leaves the FPU initialised, which
// is what the abort path was going to do anyway.
"  fnsave _oracle_x87_save\n"
".globl _oracle_abort_point\n"
"_oracle_abort_point:\n"
"  cld\n"
"  movl _oracle_host_esp, %esp\n"
// TF is set on entry whenever the instruction budget is armed, and the guest's
// `ret` does not clear it. Leaving it set means the HARNESS single-steps after
// the run, and the first of those single-step exceptions to arrive once the
// handler is no longer expecting one kills the process with STATUS_SINGLE_STEP
// at an address in the oracle. Clear it here, on the host stack, where writing
// four bytes cannot be mistaken for something the guest did.
"  pushfl\n"
"  andl $0xfffffeff, (%esp)\n"
"  popfl\n"
"  movl _oracle_saved_fs0, %eax\n"
"  movl %eax, %fs:0\n"
"  fninit\n"
"  popl %edi\n"
"  popl %esi\n"
"  popl %ebx\n"
"  popl %ebp\n"
"  ret\n"
);

// ---------------------------------------------------------------------------
// Side identification and the escape route for the lifted side.
//
// The original side leaves through oracle_abort_point, which restores the host
// ESP the runner saved. The lifted side is ordinary C++ many frames deep, so
// it leaves by longjmp instead; the handler points EIP at a thunk and gives it
// a private stack to stand on, because the stack it faulted with may be the
// thing that is wrong.
// ---------------------------------------------------------------------------

enum OracleSide { OracleSideIdle = 0, OracleSideOriginal, OracleSideLifted };
static volatile OracleSide g_side = OracleSideIdle;
// Which thread each side runs on, so the handler can tell "a side is active"
// from "this thread IS the side".
static volatile DWORD g_original_thread_id = 0;
static volatile DWORD g_lifted_thread_id = 0;
static thread_local jmp_buf g_lifted_escape;
alignas(16) static unsigned char g_escape_stack[64 * 1024];

extern "C" void oracle_lifted_escape(void) {
    longjmp(g_lifted_escape, 1);
}

asm(
".text\n"
".globl _oracle_longjmp_thunk\n"
"_oracle_longjmp_thunk:\n"
"  cld\n"
"  call _oracle_lifted_escape\n"
"  hlt\n"
);

bool oracle_lifted_trapped = false;
static const char *g_trap_reason = nullptr;
static uint32_t g_trap_address = 0;

// Replaces the generated runtime's trap. The build weakens the definition in
// lifted_dispatch.cpp.o so this one wins; a trap is a body meeting an
// instruction the lift never lowered, which is a SKIP with a named reason and
// emphatically not a comparison failure.
[[noreturn]] void opensmacx_trap(uint32_t address, const char *reason) {
    oracle_lifted_trapped = true;
    g_trap_reason = reason;
    g_trap_address = address;
    if (g_side == OracleSideLifted) {
        longjmp(g_lifted_escape, 2);
    }
    std::fprintf(stderr, "oracle: trap outside a case at %#010x: %s\n",
                 static_cast<unsigned>(address), reason);
    std::abort();
}

// ---------------------------------------------------------------------------
// Memory
// ---------------------------------------------------------------------------

static unsigned char *g_pristine = nullptr;
static unsigned char *const g_side_a =
    reinterpret_cast<unsigned char *>(static_cast<uintptr_t>(OracleImageBase));
static bool g_image_loaded = false;
static uint32_t g_code_begin = 0, g_code_end = 0;

OracleCost oracle_cost = {};
bool oracle_trace = false;
static void trace(const char *what) {
    if (oracle_trace) { std::fprintf(stderr, "[oracle] %s\n", what); }
}

static double now_seconds() {
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return double(c.QuadPart) / double(f.QuadPart);
}

// ---------------------------------------------------------------------------
// Bootstrap: reserve 0x00400000 in a suspended copy of ourselves.
// ---------------------------------------------------------------------------

int oracle_bootstrap(int argc, char **argv, int *exit_code) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--oracle-child") == 0) {
            void *p = VirtualAlloc(reinterpret_cast<void *>(
                                       static_cast<uintptr_t>(OracleImageBase)),
                                   OracleImageSize, MEM_COMMIT, PAGE_READWRITE);
            if (!p) {
                std::fprintf(stderr,
                             "oracle: the parent's reservation is missing "
                             "(commit failed, error %lu)\n",
                             static_cast<unsigned long>(GetLastError()));
                return 2;
            }
            return 0;
        }
    }

    char self[MAX_PATH];
    if (!GetModuleFileNameA(nullptr, self, MAX_PATH)) return 2;

    char command[8192];
    int n = std::snprintf(command, sizeof command, "\"%s\" --oracle-child", self);
    for (int i = 1; i < argc && n > 0 && n < int(sizeof command) - 2; ++i) {
        n += std::snprintf(command + n, sizeof command - n, " \"%s\"", argv[i]);
    }

    STARTUPINFOA si;
    std::memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    PROCESS_INFORMATION pi;
    std::memset(&pi, 0, sizeof pi);
    if (!CreateProcessA(self, command, nullptr, nullptr, TRUE, CREATE_SUSPENDED,
                        nullptr, nullptr, &si, &pi)) {
        std::fprintf(stderr, "oracle: CreateProcess failed, error %lu\n",
                     static_cast<unsigned long>(GetLastError()));
        return 2;
    }
    // The whole point of the suspension: at this instant the child has only
    // ntdll and its own image mapped, so the span is still free and a
    // reservation here is one nothing else can outrace.
    void *reserved = VirtualAllocEx(
        pi.hProcess, reinterpret_cast<void *>(static_cast<uintptr_t>(OracleImageBase)),
        OracleSpanReservation, MEM_RESERVE, PAGE_READWRITE);
    if (!reserved) {
        std::fprintf(stderr, "oracle: reserving %#x in the child failed, error %lu\n",
                     OracleImageBase, static_cast<unsigned long>(GetLastError()));
        TerminateProcess(pi.hProcess, 3);
        return 2;
    }

    // THE WALL, and it is not optional.
    //
    // The original side runs with the whole process address space in reach, so
    // a seeded pointer that the guest adds to another seeded pointer produces a
    // plausible-looking address a few megabytes above the arena - and Wine's
    // heap was sitting exactly there. StringStruct::StringStruct (0x00401000)
    // does `mov [edx+eax+4], offset` with both operands out of the arena; the
    // store landed in the CRT's heap and the process died four printf calls
    // later, inside msvcrt, with a corrupt-looking pointer and no connection
    // whatsoever to the case that caused it. Two runs looked fine and the third
    // died; that is what silent host corruption looks like from the outside.
    //
    // So every free region below OracleWallTop is RESERVED - address space
    // only, no pages - before the child runs. A wild guest write into it now
    // takes an access violation, which the handler turns into INCONCLUSIVE for
    // the case that caused it. Wine puts its heap above the wall on its own.
    uint32_t walled = 0;
    for (uintptr_t address = 0x00010000U; address < OracleWallTop;) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQueryEx(pi.hProcess, reinterpret_cast<void *>(address), &mbi,
                            sizeof mbi))
            break;
        uintptr_t next = uintptr_t(mbi.BaseAddress) + mbi.RegionSize;
        if (mbi.State == MEM_FREE) {
            SIZE_T size = mbi.RegionSize;
            if (uintptr_t(mbi.BaseAddress) + size > OracleWallTop)
                size = OracleWallTop - uintptr_t(mbi.BaseAddress);
            if (VirtualAllocEx(pi.hProcess, mbi.BaseAddress, size, MEM_RESERVE,
                               PAGE_NOACCESS))
                walled += uint32_t(size);
        }
        address = next;
    }
    std::fprintf(stderr, "oracle: walled %u MiB of low address space\n",
                 unsigned(walled >> 20));

    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD status = 3;
    GetExitCodeProcess(pi.hProcess, &status);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    *exit_code = int(status);
    return 1;
}

// ---------------------------------------------------------------------------
// Loading the user's own executable. Runtime only: no original byte is ever
// written into the tree.
// ---------------------------------------------------------------------------

const char *oracle_load_image(const char *path) {
    FILE *file = std::fopen(path, "rb");
    if (!file) return "cannot open the original executable";
    std::fseek(file, 0, SEEK_END);
    long length = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    if (length < 0x200) { std::fclose(file); return "the file is too short to be a PE"; }
    unsigned char *raw = static_cast<unsigned char *>(std::malloc(size_t(length)));
    if (!raw) { std::fclose(file); return "out of memory reading the executable"; }
    if (std::fread(raw, 1, size_t(length), file) != size_t(length)) {
        std::fclose(file); std::free(raw); return "short read";
    }
    std::fclose(file);

    if (raw[0] != 'M' || raw[1] != 'Z') { std::free(raw); return "not an MZ image"; }
    uint32_t pe_offset;
    std::memcpy(&pe_offset, raw + 0x3C, 4);
    if (pe_offset + 0xF8 > uint32_t(length)) { std::free(raw); return "bad e_lfanew"; }
    if (std::memcmp(raw + pe_offset, "PE\0\0", 4) != 0) { std::free(raw); return "no PE signature"; }

    uint16_t sections;
    std::memcpy(&sections, raw + pe_offset + 6, 2);
    uint16_t optional_size;
    std::memcpy(&optional_size, raw + pe_offset + 20, 2);
    uint32_t image_base;
    std::memcpy(&image_base, raw + pe_offset + 24 + 28, 4);
    uint32_t headers_size;
    std::memcpy(&headers_size, raw + pe_offset + 24 + 60, 4);
    if (image_base != OracleImageBase) { std::free(raw); return "unexpected image base"; }

    g_pristine = static_cast<unsigned char *>(std::calloc(1, OracleImageSize));
    if (!g_pristine) { std::free(raw); return "out of memory for the pristine copy"; }
    if (headers_size > OracleImageSize) headers_size = OracleImageSize;
    std::memcpy(g_pristine, raw, headers_size);

    const unsigned char *table = raw + pe_offset + 24 + optional_size;
    for (uint16_t i = 0; i < sections; ++i) {
        const unsigned char *s = table + i * 40;
        char name[9] = {};
        std::memcpy(name, s, 8);
        uint32_t vsize, va, rawsize, rawptr, chars;
        std::memcpy(&vsize, s + 8, 4);
        std::memcpy(&va, s + 12, 4);
        std::memcpy(&rawsize, s + 16, 4);
        std::memcpy(&rawptr, s + 20, 4);
        std::memcpy(&chars, s + 36, 4);
        uint32_t copy = rawsize;
        if (vsize && copy > vsize && vsize > 0) copy = vsize;
        if (rawptr + copy > uint32_t(length)) copy = uint32_t(length) - rawptr;
        if (va + copy <= OracleImageSize) std::memcpy(g_pristine + va, raw + rawptr, copy);
        if (chars & 0x20000000U) {  // IMAGE_SCN_MEM_EXECUTE
            uint32_t begin = OracleImageBase + va;
            uint32_t end = begin + (vsize ? vsize : rawsize);
            if (!g_code_begin || begin < g_code_begin) g_code_begin = begin;
            if (end > g_code_end) g_code_end = end;
        }
    }
    std::free(raw);

    // Executable where the original has code, plain data everywhere else, so a
    // jump that the seed manufactured out of pointer soup faults at the
    // transfer instead of interpreting the soup.
    DWORD old = 0;
    if (!VirtualProtect(g_side_a, OracleImageSize, PAGE_READWRITE, &old))
        return "VirtualProtect(data) failed";
    uint32_t code_pages_begin = g_code_begin & ~0xFFFU;
    uint32_t code_pages_end = (g_code_end + 0xFFFU) & ~0xFFFU;
    if (!VirtualProtect(reinterpret_cast<void *>(static_cast<uintptr_t>(code_pages_begin)),
                        code_pages_end - code_pages_begin, PAGE_EXECUTE_READWRITE, &old))
        return "VirtualProtect(code) failed";

    g_image_loaded = true;
    return nullptr;
}

// ---------------------------------------------------------------------------
// Seeding
//
// splitmix32 of the function address mixed with the case index, so every input
// is a pure function of the pair and `--only <address>` replays a failure
// exactly. Nothing is drawn from a clock, a pointer or an iteration order.
// ---------------------------------------------------------------------------

static uint32_t mix(uint32_t x) {
    x += 0x9E3779B9U;
    x = (x ^ (x >> 16)) * 0x21F0AAADU;
    x = (x ^ (x >> 15)) * 0x735A2D97U;
    return x ^ (x >> 15);
}

const char *oracle_register_name(int index) {
    static const char *names[9] = {"eax", "ecx", "edx", "ebx",
                                   "esp", "ebp", "esi", "edi", "eflags"};
    return (index >= 0 && index < 9) ? names[index] : "?";
}

// Fills the scratch window (arena + stack) of one memory image. Applied
// identically to both sides.
static void seed_scratch_legacy(unsigned char *image, uint32_t address, int case_index,
                                uint32_t return_address) {
    uint32_t h = mix(address ^ (uint32_t(case_index) * 0x85EBCA6BU));
    unsigned char *arena = image + (OracleArenaBase - OracleImageBase);
    // The arena is an object graph, and it has to be one: a word that is a
    // pointer lets a dereference chain of any depth stay inside the scratch
    // window instead of faulting on step one, and a word that is a SMALL
    // INTEGER keeps derived pointers in range.
    //
    // The second half is not decoration. An MSVC adjustor thunk starts
    // `sub ecx, [ecx-4]`, so if [this-4] is a pointer the result is a pointer
    // near 4 GiB - and under Wine on this host a read up there silently
    // returns zero instead of faulting, and cannot be sealed (VirtualQuery
    // refuses the address with ERROR_INVALID_PARAMETER). The original reads
    // zero, the lifted side reads whatever host memory `opensmacx_image + delta`
    // lands on, and the case fails for a reason that has nothing to do with
    // the lowering. Four of the nine first-run failures were exactly that.
    // Alternating the two kinds of word, with a different phase per case,
    // keeps all three seeds distinct and keeps thunk arithmetic in the arena.
    for (uint32_t i = 0; i < OracleArenaSize / 4; ++i) {
        bool pointer;
        switch (case_index) {
            case 0: pointer = (i & 1) == 0; break;
            case 1: pointer = (i & 1) == 1; break;
            default: pointer = (i & 3) == 0; break;
        }
        uint32_t word;
        if (pointer) {
            word = OracleArenaBase +
                   (((i * 8 + 64 + (h >> 12)) % (OracleArenaSize - 64)) & ~3U);
        } else {
            word = (mix(h + i) & 0x1FU) * 4U;   // 0..124, four-aligned
        }
        std::memcpy(arena + i * 4, &word, 4);
    }
    // The four words immediately BELOW `this` are seeded as small, four-aligned
    // displacements in every case, whatever the phase says. That is what they
    // are: MSVC's virtual-inheritance thunks read [this-4] and [this-8] as the
    // adjustment to subtract from `this`, and 130-odd functions in this image
    // begin with `sub ecx, [ecx-4]`. Seeding a pointer there is seeding a
    // displacement of nine megabytes, which is not a case anyone can learn
    // anything from.
    const uint32_t this_pointer = OracleArenaBase + ((h % 0x400U) & ~0xFU);
    for (uint32_t back = 1; back <= 4; ++back) {
        uint32_t word = (mix(h + 0x300U + back) & 0x1FU) * 4U;
        std::memcpy(image + (this_pointer - back * 4 - OracleImageBase), &word, 4);
    }
    // The stack window below ESP is filled with a PATTERN, not zeroes, and the
    // reason is a mutant that got away: dropping the `push ebp` from a lifted
    // prologue was not detected, because EBP was seeded as zero and the slot it
    // should have been stored into was zero too, so the missing write wrote the
    // value that was already there. A store can only be observed if the value
    // it stores differs from what the location held. The values stay small so
    // that anything dereferencing them lands in the walled region and faults,
    // rather than reading somewhere plausible.
    std::memset(image + (OracleStackTop - OracleImageBase), 0,
                OracleScratchEnd - OracleStackTop);
    for (uint32_t offset = OracleStackLow; offset < OracleStackTop; offset += 4) {
        uint32_t word = 0x100U | (mix(h + 0x400U + offset) & 0xFFU);
        std::memcpy(image + (offset - OracleImageBase), &word, 4);
    }
    std::memcpy(image + (OracleStackTop - OracleImageBase), &return_address, 4);
    for (uint32_t i = 0; i < OracleSeedArgWords; ++i) {
        uint32_t word;
        switch (case_index) {
            case 0: word = 0; break;
            case 1: word = mix(h + 0x100U + i) & 0xFF; break;
            case 2:
                word = (i & 1) ? (mix(h + 0x200U + i) & 0x1F)
                               : (OracleArenaBase + ((mix(h + i) % 0x800U) & ~3U));
                break;
            default:
                // Negative in the odd slots only. Making every argument
                // negative makes every pointer argument wild, and this image is
                // full of `void f(RECT *, int *, int *)`: the run then measures
                // the address model rather than the lowering.
                word = (i & 1) ? (0xFFFFFF00U | (mix(h + 0x300U + i) & 0xFFU))
                               : (OracleArenaBase + ((mix(h + i) % 0x800U) & ~3U));
                break;
        }
        std::memcpy(image + (OracleStackTop + 4 + i * 4 - OracleImageBase), &word, 4);
    }
}

static void seed_registers_legacy(uint32_t address, int case_index, OracleRegisters *r) {
    uint32_t h = mix(address ^ (uint32_t(case_index) * 0x85EBCA6BU));
    uint32_t this_pointer = OracleArenaBase + ((h % 0x400U) & ~0xFU);
    switch (case_index) {
        case 0:
            r->eax = r->edx = r->ebx = r->ebp = r->esi = r->edi = 0;
            r->ecx = this_pointer;
            break;
        case 1:
            r->eax = mix(h + 1) & 0xFF;
            r->edx = mix(h + 2) & 0xFF;
            r->ebx = mix(h + 3) & 0xFF;
            // A frame pointer, not zero. EBP was 0 in every case, which made
            // the dropped-`push ebp` mutant survivable on its own and made
            // this function's description of cases 2 and 3 ("non-zero ebp")
            // false on every FAIL line the report printed.
            r->ebp = OracleStackTop - 0x40U;
            r->esi = mix(h + 4) & 0xFF;
            r->edi = mix(h + 5) & 0xFF;
            r->ecx = this_pointer;
            break;
        case 2:
            r->eax = OracleArenaBase + ((mix(h + 6) % 0x400U) & ~3U);
            r->edx = mix(h + 7) & 0x1F;
            r->ebx = OracleArenaBase + ((mix(h + 8) % 0x400U) & ~3U);
            r->ebp = OracleStackTop - 0x80U;
            r->esi = OracleArenaBase + ((mix(h + 9) % 0x400U) & ~3U);
            r->edi = mix(h + 10) & 0x1F;
            r->ecx = this_pointer;
            break;
        default:
            // The WIDE case. Every other case leaves bits 24..31 of every
            // register either zero or 0x009D (an arena pointer), so no input
            // ever had a non-zero top byte and every sign-extension or
            // partial-register defect above bit 23 was invisible - the same
            // shape as the sar/shr miss, and much broader than "the arena
            // holds no negative values".
            //
            // Negative small integers are the classic probe for it. They are
            // not valid pointers, so a function that dereferences one faults
            // and this case reports INCONCLUSIVE for it - which is why it is
            // ONE of four cases and not all of them, and why the report now
            // states how many cases actually compared.
            r->eax = 0xFFFFFF00U | (mix(h + 6) & 0xFFU);
            r->edx = 0xFFFF0000U | (mix(h + 7) & 0xFFFFU);
            r->ebx = OracleArenaBase + ((mix(h + 8) % 0x400U) & ~3U);
            r->ebp = OracleStackTop - 0xC0U;
            r->esi = 0x80000000U | (mix(h + 9) & 0x7FU);
            r->edi = 0xFFFFFF00U | (mix(h + 10) & 0xFFU);
            r->ecx = this_pointer;
            break;
    }
    r->esp = OracleStackTop;
    // IF set, all modelled ALU flags clear. Case 2 additionally enters with DF
    // SET, so that the direction flag is an exercised INPUT and not merely a
    // compared output: a lowering that ignores DF on a string operation, or
    // one that clears it on the way past, differs from the original only when
    // something arrives with it set.
    r->eflags = (case_index == 2) ? 0x00000602U : 0x00000202U;
}

// ---------------------------------------------------------------------------
// DRAWN SEEDS - cases OracleLegacyCases and up.
//
// The four legacy cases above are four points. Every value they can produce has
// a top byte of 0x00, 0x9D or 0xFF; every arena non-pointer word is a
// four-aligned integer in 0..124; no argument is ever a boundary value; DF is
// set in exactly one of them and no other flag is ever set on entry. Two
// consequences were measured rather than guessed:
//
//   * turning `sar` into `shr` in lifted_00404ef0 was NOT caught, because the
//     value shifted comes from an arena word and no arena word is ever
//     negative;
//   * drawing the mutation targets at random rather than in address order
//     turned a 7-of-8 score into 3 caught / 4 missed / 1 unreached, and 4 of
//     the 4 missed sat on lines that DO execute - so they were reached with
//     values that could not tell the mutant from the original.
//
// What follows is not "more random numbers". Each knob below exists because a
// FIXED choice of it hides a class of defect:
//
//   pointer_period    the density of pointers in the arena. Fixed density
//                     makes a two-step dereference either always work or
//                     always fault.
//   leaf half         a pointer that lands in the leaf half terminates in a
//                     value, which is what a vbtable displacement read needs.
//   value class       tiny / small-signed / boundary / wide. Sign extension,
//                     partial-register writes and shift kind are invisible
//                     without negatives and without values that straddle a
//                     width boundary.
//   below_this_mask   whether [this-4], [this-8], ... are displacements (the
//                     130 `sub ecx,[ecx-4]` thunks need small ones) or
//                     pointers (a vbtable read needs a pointer). Both are
//                     required and they are mutually exclusive per draw.
//   entry flags       all seven modelled flags, not just DF.
//
// Everything is still a pure function of (address, case_index): `--only
// <address>` replays any of it exactly.
// ---------------------------------------------------------------------------

namespace {

// A deterministic stream so a draw can consume as many values as it needs
// without inventing a fresh constant for each one.
struct SeedStream {
    uint32_t state;
    explicit SeedStream(uint32_t seed) : state(seed) {}
    uint32_t next() { state = mix(state ^ 0x9E3779B9U); return state; }
    uint32_t below(uint32_t n) { return n ? (next() % n) : 0U; }
    bool chance(uint32_t numerator, uint32_t denominator) {
        return below(denominator) < numerator;
    }
};

// The values at which lowerings break. Width boundaries in both signs (a
// missing movsx shows up at 0x80/0x8000, a missing movzx at 0xFF/0xFFFF), the
// two extremes of each width, and a handful of float and double bit patterns so
// that a value loaded with `fld` is a number rather than a denormal.
const uint32_t kBoundaryValues[] = {
    0x00000000U, 0x00000001U, 0x00000002U, 0xFFFFFFFFU, 0xFFFFFFFEU,
    0x7FFFFFFFU, 0x80000000U, 0x80000001U,
    0x0000007FU, 0x00000080U, 0x000000FFU, 0x00000100U,
    0xFFFFFF7FU, 0xFFFFFF80U, 0xFFFFFF00U,
    0x00007FFFU, 0x00008000U, 0x0000FFFFU, 0x00010000U,
    0xFFFF7FFFU, 0xFFFF8000U, 0xFFFF0000U,
    0x007FFFFFU, 0x00800000U, 0x00FFFFFFU, 0x01000000U,
    0xCCCCCCCCU, 0x55555555U, 0xAAAAAAAAU, 0x0000000AU, 0xFFFFFFF6U,
    0x3F800000U, 0xBF800000U, 0x40490FDBU, 0x7F7FFFFFU, 0x00000009U,
    0x3FF00000U, 0xC0000000U, 0x41200000U, 0xFFFFFFF0U,
};
constexpr uint32_t kBoundaryCount = sizeof kBoundaryValues / sizeof kBoundaryValues[0];

enum ValueClass {
    ValueTiny = 0,        // four-aligned 0..124: safe as a displacement
    ValueSmallSigned,     // -127..127
    ValueBoundary,        // from the table above
    ValueWide,            // uniform over the whole word
    ValueClassCount
};

uint32_t draw_value(SeedStream &s, int value_class) {
    switch (value_class) {
        case ValueTiny: return (s.next() & 0x1FU) * 4U;
        case ValueSmallSigned: {
            const uint32_t magnitude = s.next() & 0x7FU;
            return (s.next() & 1U) ? (0U - magnitude) : magnitude;
        }
        case ValueBoundary: return kBoundaryValues[s.below(kBoundaryCount)];
        default: return s.next();
    }
}

// Pointers are kept clear of the last 256 bytes of their half so that a small
// positive displacement added to one still lands in the same half.
uint32_t draw_node_pointer(SeedStream &s) {
    return OracleArenaNodeBase + (s.below(OracleArenaNodeSize - 256U) & ~3U);
}
uint32_t draw_leaf_pointer(SeedStream &s) {
    return OracleArenaLeafBase + (s.below(OracleArenaLeafSize - 256U) & ~3U);
}
uint32_t draw_stack_pointer(SeedStream &s) {
    return OracleStackLow + (s.below(OracleStackTop - OracleStackLow - 256U) & ~3U);
}

struct SeedProfile {
    uint32_t h = 0;
    uint32_t this_pointer = 0;
    // arena
    uint32_t pointer_period = 2;   // 0 = no pointers at all
    uint32_t pointer_phase = 0;
    uint32_t leaf_bias = 4;        // out of 8, how often a pointer aims at the leaf
    int node_value_class = ValueTiny;
    int leaf_value_class = ValueTiny;
    // the sixteen words below `this`
    uint32_t below_this_mask = 0;  // bit b set: [this - 4*(b+1)] is a leaf pointer
    // the stack below ESP
    int stack_class = 0;           // 0 legacy pattern, 1 tiny, 2 pointers, 3 values
    // registers and stack arguments
    uint32_t register_pointer_weight = 4;   // out of 16
    int register_value_class = ValueSmallSigned;
    uint32_t argument_pointer_weight = 8;   // out of 16
    int argument_value_class = ValueSmallSigned;
    bool ecx_is_this = true;
    uint32_t entry_flags = 0x00000202U;
};

SeedProfile make_profile(uint32_t address, int case_index) {
    SeedProfile p;
    p.h = mix(address ^ (uint32_t(case_index) * 0x85EBCA6BU));
    SeedStream s(p.h ^ 0x1B873593U);

    // `this` sits far enough into the node half that sixteen words below it are
    // still arena. The legacy cases put it in the first kilobyte, so [this-0x18]
    // read real image bytes rather than anything the seed controlled.
    p.this_pointer = OracleArenaNodeBase + 0x400U + (s.below(0x2000U) & ~0xFU);

    static const uint32_t periods[] = {1, 2, 2, 3, 4, 4, 8, 16, 0};
    p.pointer_period = periods[s.below(sizeof periods / sizeof periods[0])];
    p.pointer_phase = s.below(p.pointer_period ? p.pointer_period : 1U);
    p.leaf_bias = s.below(9);
    p.node_value_class = int(s.below(ValueClassCount));
    // The leaf half is biased towards TINY, because that is the class that lets
    // a chained dereference produce a usable displacement. Left uniform, three
    // draws in four end the chain in a value that faults when it is added to a
    // pointer, and the whole point of the split is lost.
    p.leaf_value_class = s.chance(1, 2) ? ValueTiny : int(s.below(ValueClassCount));

    // Two thirds of the draws leave the first two words below `this` as
    // displacements, because those are the ones the adjustor thunks read and a
    // pointer there is a nine-megabyte adjustment.
    p.below_this_mask = s.next();
    if (s.chance(2, 3)) p.below_this_mask &= ~3U;

    p.stack_class = int(s.below(4));
    p.register_pointer_weight = s.below(17);
    p.register_value_class = int(s.below(ValueClassCount));
    p.argument_pointer_weight = s.below(17);
    p.argument_value_class = int(s.below(ValueClassCount));
    // ECX is `this` most of the time - the image is C++ and thiscall - but not
    // always, because a cdecl function's ECX is scratch and seeding it with a
    // valid pointer in every case is one more value that is never wide.
    p.ecx_is_this = s.chance(3, 4);

    // Every modelled flag, not just DF. A `sbb eax,eax` or an `adc` on the
    // entry path reads CF, and no seed before this one ever set it. TF is
    // never set here: it belongs to the budget, and setting it by accident
    // single-steps the whole original run.
    uint32_t flags = 0x00000202U;
    static const uint32_t settable[] = {0x001U, 0x004U, 0x010U, 0x040U,
                                        0x080U, 0x800U};
    for (uint32_t i = 0; i < sizeof settable / sizeof settable[0]; ++i)
        if (s.chance(1, 3)) flags |= settable[i];
    // DF gets its own, higher, rate: 305 string sites read it and it is the one
    // entry flag with a documented effect on memory.
    if (s.chance(1, 3)) flags |= 0x400U;
    p.entry_flags = flags;
    return p;
}

}  // namespace

static void seed_scratch_drawn(unsigned char *image, const SeedProfile &p,
                               uint32_t return_address) {
    SeedStream s(p.h ^ 0x0A2E4A00U);
    unsigned char *arena = image + (OracleArenaBase - OracleImageBase);
    const uint32_t leaf_index = (OracleArenaLeafBase - OracleArenaBase) / 4U;
    for (uint32_t i = 0; i < OracleArenaSize / 4; ++i) {
        uint32_t word;
        if (i >= leaf_index) {
            word = draw_value(s, p.leaf_value_class);
        } else if (p.pointer_period &&
                   ((i + p.pointer_phase) % p.pointer_period) == 0) {
            word = s.below(8) < p.leaf_bias ? draw_leaf_pointer(s)
                                            : draw_node_pointer(s);
        } else {
            word = draw_value(s, p.node_value_class);
        }
        std::memcpy(arena + i * 4, &word, 4);
    }

    for (uint32_t back = 1; back <= 16; ++back) {
        const uint32_t word = ((p.below_this_mask >> (back - 1)) & 1U)
                                  ? draw_leaf_pointer(s)
                                  : (s.next() & 0x1FU) * 4U;
        std::memcpy(image + (p.this_pointer - back * 4 - OracleImageBase), &word, 4);
    }

    std::memset(image + (OracleStackTop - OracleImageBase), 0,
                OracleScratchEnd - OracleStackTop);
    for (uint32_t offset = OracleStackLow; offset < OracleStackTop; offset += 4) {
        uint32_t word;
        switch (p.stack_class) {
            case 0: word = 0x100U | (s.next() & 0xFFU); break;
            case 1: word = (s.next() & 0x1FU) * 4U; break;
            case 2: word = s.chance(1, 2) ? draw_leaf_pointer(s) : draw_node_pointer(s); break;
            default: word = draw_value(s, ValueBoundary); break;
        }
        std::memcpy(image + (offset - OracleImageBase), &word, 4);
    }
    std::memcpy(image + (OracleStackTop - OracleImageBase), &return_address, 4);

    for (uint32_t i = 0; i < OracleSeedArgWords; ++i) {
        uint32_t word;
        if (s.below(16) < p.argument_pointer_weight) {
            const uint32_t kind = s.below(8);
            word = kind < 4 ? draw_node_pointer(s)
                 : kind < 7 ? draw_leaf_pointer(s)
                            : draw_stack_pointer(s);
        } else {
            word = draw_value(s, p.argument_value_class);
        }
        std::memcpy(image + (OracleStackTop + 4 + i * 4 - OracleImageBase), &word, 4);
    }
}

static void seed_registers_drawn(const SeedProfile &p, OracleRegisters *r) {
    SeedStream s(p.h ^ 0x27D4EB2FU);
    auto draw_register = [&]() -> uint32_t {
        if (s.below(16) < p.register_pointer_weight) {
            const uint32_t kind = s.below(8);
            return kind < 4 ? draw_node_pointer(s)
                 : kind < 7 ? draw_leaf_pointer(s)
                            : draw_stack_pointer(s);
        }
        return draw_value(s, p.register_value_class);
    };
    r->eax = draw_register();
    r->edx = draw_register();
    r->ebx = draw_register();
    r->esi = draw_register();
    r->edi = draw_register();
    r->ecx = p.ecx_is_this ? p.this_pointer : draw_register();
    // EBP is a frame pointer somewhere in the stack window, never zero and
    // never wild: a function that unwinds through it must find something it can
    // walk, and a wild EBP measures the wall rather than the lowering.
    r->ebp = OracleStackTop - 4U * (1U + s.below(0x60U));
    r->esp = OracleStackTop;
    r->eflags = p.entry_flags;
}

// The two dispatchers. Cases below OracleLegacyCases are the originals,
// unchanged, so every failure the first whole-image sweep found still
// reproduces at the same case index.
static void seed_scratch(unsigned char *image, uint32_t address, int case_index,
                         uint32_t return_address) {
    if (case_index < OracleLegacyCases) {
        seed_scratch_legacy(image, address, case_index, return_address);
        return;
    }
    seed_scratch_drawn(image, make_profile(address, case_index), return_address);
}

static void seed_registers(uint32_t address, int case_index, OracleRegisters *r) {
    if (case_index < OracleLegacyCases) {
        seed_registers_legacy(address, case_index, r);
        return;
    }
    seed_registers_drawn(make_profile(address, case_index), r);
}

const char *oracle_seed_description(uint32_t address, int case_index) {
    static char text[256];
    switch (case_index) {
        case 0:
            return "legacy 0: zeroed registers, ebp=0, this=arena, pointers on even arena words";
        case 1:
            return "legacy 1: small-integer registers and arguments, frame ebp, pointers on odd words";
        case 2:
            return "legacy 2: arena pointers in registers, frame ebp, DF SET on entry";
        case 3:
            return "legacy 3: wide/negative registers (top byte set), frame ebp";
        default: break;
    }
    static const char *const class_names[ValueClassCount] = {
        "tiny", "small-signed", "boundary", "wide"};
    const SeedProfile p = make_profile(address, case_index);
    std::snprintf(text, sizeof text,
                  "drawn %d: arena 1-in-%u pointers (leaf bias %u/8), node %s, "
                  "leaf %s, below-this %#06x, stack %d, regs %s w%u, args %s w%u, "
                  "ecx=%s, flags %#05x%s",
                  case_index, unsigned(p.pointer_period),
                  unsigned(p.leaf_bias), class_names[p.node_value_class],
                  class_names[p.leaf_value_class],
                  unsigned(p.below_this_mask & 0xFFFFU), p.stack_class,
                  class_names[p.register_value_class],
                  unsigned(p.register_pointer_weight),
                  class_names[p.argument_value_class],
                  unsigned(p.argument_pointer_weight),
                  p.ecx_is_this ? "this" : "drawn",
                  unsigned(p.entry_flags),
                  (p.entry_flags & 0x400U) ? " DF" : "");
    return text;
}

// ---------------------------------------------------------------------------
// The guard: a vectored handler plus a trap-flag budget plus a watchdog.
// ---------------------------------------------------------------------------

static uint32_t g_watchdog_ms = 0;
static uint32_t g_lifted_timeout_ms = 2000;
static volatile LONG g_run_started_tick = 0;
static volatile LONG g_run_active = 0;
static HANDLE g_main_thread = nullptr;

// Single-step attribution, used only after a case has already failed.
static bool g_blaming = false;
static uint32_t g_watch_address = 0;
static int g_watch_register = -1;
static uint32_t g_watch_value = 0;
static uint32_t g_blame_eip = 0;
static uint32_t g_blame_last_eip = 0;

static uint32_t context_register(const CONTEXT *c, int index) {
    switch (index) {
        case 0: return c->Eax; case 1: return c->Ecx; case 2: return c->Edx;
        case 3: return c->Ebx; case 4: return c->Esp; case 5: return c->Ebp;
        case 6: return c->Esi; case 7: return c->Edi;
        // Index 8 is the six modelled flags, so that a divergence in the flags
        // alone can name the instruction that produced it. Without this the
        // flags-only failures are the ones with no evidence attached, and they
        // are the majority.
        default: return c->EFlags & 0x08D5U;
    }
}

static void redirect_out(CONTEXT *c) {
    c->EFlags &= ~0x100U;   // clear TF or the abort path single-steps too
    c->EFlags &= ~0x400U;   // and clear DF for the host we are returning to
    if (g_side == OracleSideLifted) {
        c->Eip = DWORD(uintptr_t(&oracle_longjmp_thunk));
        c->Esp = DWORD(uintptr_t(g_escape_stack + sizeof g_escape_stack - 64));
    } else {
        c->Eip = DWORD(uintptr_t(&oracle_abort_point));
        c->Esp = oracle_host_esp;
    }
}

static LONG CALLBACK oracle_veh(EXCEPTION_POINTERS *ep) {
    // g_side says a side is RUNNING; it does not say THIS THREAD is the one
    // running it. Without the thread test, an exception raised anywhere in the
    // process while the original side was active counted as the guest
    // faulting, and redirect_out then rewrote that thread's EIP to
    // oracle_abort_point and its ESP to the MAIN thread's saved stack -
    // laundering an unrelated harness bug into a lift finding, and corrupting
    // a thread that had nothing to do with the case.
    const DWORD self = GetCurrentThreadId();
    if ((g_side == OracleSideOriginal && self != g_original_thread_id) ||
        (g_side == OracleSideLifted && self != g_lifted_thread_id)) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (g_side == OracleSideIdle) {
        // A stray single-step outside a case means TF leaked out of one. Turn
        // it off and carry on rather than letting it become a crash report
        // about the harness in the middle of someone else's failure.
        if (ep->ExceptionRecord->ExceptionCode == DWORD(EXCEPTION_SINGLE_STEP)) {
            ep->ContextRecord->EFlags &= ~0x100U;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
    DWORD code = ep->ExceptionRecord->ExceptionCode;

    if (code == DWORD(EXCEPTION_SINGLE_STEP)) {
        oracle_steps++;
        // Only instructions INSIDE the guest image can be blamed. Without this
        // the last writer of EAX is always the harness's own `seto %al` in the
        // capture sequence, which is a true statement and a useless one.
        const uint32_t stepped_eip = ep->ContextRecord->Eip;
        const bool in_image = stepped_eip >= OracleImageBase &&
                              stepped_eip < OracleImageBase + OracleImageSize;
        if (g_blaming && in_image) {
            uint32_t value;
            if (g_watch_register >= 0) {
                value = context_register(ep->ContextRecord, g_watch_register);
            } else {
                std::memcpy(&value, g_side_a + (g_watch_address - OracleImageBase), 4);
            }
            if (value != g_watch_value) {
                g_watch_value = value;
                g_blame_eip = g_blame_last_eip;
            }
            g_blame_last_eip = stepped_eip;
        }
        if (oracle_budget && oracle_steps > oracle_budget) {
            oracle_fault = OracleFaultBudget;
            redirect_out(ep->ContextRecord);
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        ep->ContextRecord->EFlags |= 0x100U;
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    oracle_fault = code;
    oracle_fault_addr = uint32_t(uintptr_t(ep->ExceptionRecord->ExceptionAddress));
    oracle_fault_has_address = (ep->ExceptionRecord->NumberParameters >= 2);
    oracle_fault_data = (ep->ExceptionRecord->NumberParameters >= 2)
        ? uint32_t(ep->ExceptionRecord->ExceptionInformation[1]) : 0;
    redirect_out(ep->ContextRecord);
    return EXCEPTION_CONTINUE_EXECUTION;
}

static DWORD WINAPI oracle_watchdog(LPVOID) {
    for (;;) {
        Sleep(25);
        if (!g_run_active || !g_watchdog_ms) continue;
        LONG started = g_run_started_tick;
        LONG elapsed = LONG(GetTickCount()) - started;
        if (elapsed < LONG(g_watchdog_ms)) continue;
        if (!g_run_active) continue;
        // Suspend, re-check that the same run is still going, then rewrite the
        // instruction pointer so the run leaves the way a fault would.
        trace("watchdog: suspending");
        SuspendThread(g_main_thread);
        if (g_run_active && LONG(GetTickCount()) - g_run_started_tick >= LONG(g_watchdog_ms)) {
            CONTEXT c;
            c.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
            const BOOL got = GetThreadContext(g_main_thread, &c);
            if (oracle_trace) {
                std::fprintf(stderr, "[oracle] watchdog: got=%d eip=%#lx err=%lu\n",
                             int(got), (unsigned long)c.Eip,
                             (unsigned long)GetLastError());
            }
            if (got) {
                oracle_fault = OracleFaultWatchdog;
                oracle_fault_addr = c.Eip;
                redirect_out(&c);
                const BOOL set = SetThreadContext(g_main_thread, &c);
                if (oracle_trace) {
                    std::fprintf(stderr, "[oracle] watchdog: set=%d -> eip=%#lx err=%lu\n",
                                 int(set), (unsigned long)c.Eip,
                                 (unsigned long)GetLastError());
                }
            }
        }
        ResumeThread(g_main_thread);
    }
}

// A fault the harness itself takes, outside a case, is a bug in the harness.
// It must say so in one line instead of handing the process to winedbg, which
// on this host starts a Vulkan-initialising debugger and buries the report.
static LONG CALLBACK oracle_last_resort(EXCEPTION_POINTERS *ep) {
    std::fprintf(stderr,
                 "oracle: HARNESS FAULT %#lx at %p (this is a bug in the "
                 "oracle, not a finding about the lift)\n",
                 (unsigned long)ep->ExceptionRecord->ExceptionCode,
                 ep->ExceptionRecord->ExceptionAddress);
    std::fflush(nullptr);
    TerminateProcess(GetCurrentProcess(), 9);
    return EXCEPTION_EXECUTE_HANDLER;
}

void oracle_arm(uint32_t budget, uint32_t watchdog_ms) {
    static bool armed = false;
    oracle_budget = budget;
    g_watchdog_ms = watchdog_ms;
    g_lifted_timeout_ms = watchdog_ms ? watchdog_ms : 2000;
    if (armed) return;
    armed = true;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(oracle_last_resort);
    AddVectoredExceptionHandler(1, oracle_veh);
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
                    &g_main_thread, 0, FALSE, DUPLICATE_SAME_ACCESS);
    if (watchdog_ms) {
        CreateThread(nullptr, 0, oracle_watchdog, nullptr, 0, nullptr);
    }
    oracle_probe_top_page();
}

// ---------------------------------------------------------------------------
// Running the lifted side on a thread that can be abandoned
//
// The lifted side is ordinary compiled host code, so there is no trap flag to
// budget it and no dispatch table entry to count: a lowering bug that turns a
// bounded loop into an unbounded one just spins. The watchdog cannot rescue it
// either - on this host, SuspendThread on the main thread followed by
// GetThreadContext never returns when the thread is inside host code, though
// the same sequence works when it is inside guest code. That is measured, not
// assumed: with tracing on, "watchdog: suspending" is the last line printed.
//
// So the lifted body runs on a worker thread and the main thread waits for it
// with a timeout. On a timeout the worker is terminated and a fresh one is
// started for the next case. The worker touches only `opensmacx_image` and its
// own state, both of which are restored before the next case, so there is
// nothing for it to have been holding.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Sealing the address space
//
// The wall the parent builds covers only the low 128 MiB, and the seed can
// produce a guest pointer anywhere. An original-side access to, say,
// 0xFFFF28AC is outside everything the lift models - `opensmacx_at` is a
// subtraction with no bounds check, so the lifted side reads whatever host
// memory that lands on - and the two sides then disagree for a reason that has
// nothing to do with the lowering.
//
// Reporting those as lowering bugs would be the harness lying in the direction
// that makes it look productive. So once the harness has finished allocating,
// every remaining free region of the address space is RESERVED, and an
// out-of-span access on the original side takes an access violation instead.
// (The top 64 KiB cannot be sealed: VirtualQuery refuses those addresses with
// ERROR_INVALID_PARAMETER while a plain read of them still succeeds and
// returns zero. The seed avoids producing pointers up there instead.)
// ---------------------------------------------------------------------------

uint32_t oracle_seal_address_space() {
    uint64_t sealed = 0;
    uint64_t address = 0x00010000U;
    while (address < 0xFFFF0000ULL) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(reinterpret_cast<void *>(uintptr_t(address)), &mbi,
                          sizeof mbi)) {
            address += 0x10000ULL;
            continue;
        }
        uint64_t next = uint64_t(uintptr_t(mbi.BaseAddress)) + mbi.RegionSize;
        if (mbi.State == MEM_FREE) {
            if (VirtualAlloc(mbi.BaseAddress, mbi.RegionSize, MEM_RESERVE,
                             PAGE_NOACCESS))
                sealed += mbi.RegionSize;
        }
        if (next <= address) break;
        address = next;
    }
    return uint32_t(sealed >> 20);
}

// ---------------------------------------------------------------------------
// THE LAST 64 KiB: the one hole the wall and the seal cannot close, and the
// arbitration that stops it manufacturing failures.
//
// The loop above walks with VirtualQuery, and VirtualQuery REFUSES the top
// 64 KiB on this host - 0 with ERROR_INVALID_PARAMETER - so 0xFFFF0000 and up
// is neither walled nor sealed. Measured here: it is ordinary committed
// read/write memory. A read of 0xFFFFFFFC returns 0, a write to it sticks, and
// a re-read returns what was written; 0xFFFEFFFC and everything below it up to
// 0x80000000 faults, so the hole is exactly one 64 KiB page.
//
// That page is where every small negative number points. `mov eax,[eax+8]`
// with eax = -10 reads 0xFFFFFFFE: the ORIGINAL reads the top page, the LIFTED
// side computes `opensmacx_image + 0xFFBFFFFE`, which wraps to
// `opensmacx_image - 0x400002` - inside the harness's own code. Two different
// answers, neither of them the program.
//
// The legacy seeds landed there occasionally (case 3 is 0xFFFFFF00|x). The
// drawn seeds put -1, -2 and 0xFFFFFF80 in registers and arena words on
// purpose, so the rate went from rare to routine, and the first run of the new
// generator produced six "failures" that were all this.
//
// WHAT DOES NOT WORK, measured rather than assumed:
//   * VirtualProtect(0xFFFF0000, 0x10000, PAGE_NOACCESS) SUCCEEDS - it does
//     not refuse the address the way VirtualQuery does, it returns the old
//     protection, and a read afterwards duly faults. And then the harness dies
//     before the first case completes: Wine needs that page. So the range can
//     be made to fault, and doing so kills the process that would report it.
//
// WHAT WORKS: poison it and ask whether the answer moved. The original is run
// with the page filled with a byte; if a case fails, the original is run twice
// more with two DIFFERENT fills and the two results compared. If they differ,
// the original's answer depended on memory outside the modelled image, the
// seed is invalid for this function, and the verdict is INCONCLUSIVE - never
// FAIL. If they agree, the original never read the page and the failure stands.
//
// This is one-sided: it can turn a false FAIL into an INCONCLUSIVE, and it
// cannot turn a false PASS into anything, because it only runs on failures.
// A PASS would have to survive the lifted side reading harness memory that
// happened to agree with the original's zero, which is possible and is a
// caveat, not a claim.
// ---------------------------------------------------------------------------

constexpr uintptr_t OracleTopPageBase = 0xFFFF0000U;
constexpr size_t OracleTopPageSize = 0x10000U;
static bool g_top_page_writable = false;

void oracle_probe_top_page() {
    volatile uint32_t *const probe =
        reinterpret_cast<volatile uint32_t *>(OracleTopPageBase + OracleTopPageSize - 4);
    // The VEH is armed by oracle_arm and treats a fault outside a running side
    // as fatal, so this must not be attempted before it is safe. It is called
    // from oracle_arm itself, where g_side is idle and a fault would be a
    // genuine crash - which is why the probe is a plain read-back rather than
    // something that has to survive an exception.
    const uint32_t saved = *probe;
    *probe = 0x5A5AA5A5U;
    g_top_page_writable = (*probe == 0x5A5AA5A5U);
    *probe = saved;
}

// Written with an explicit loop, and that is not style.
//
// `memset(0xFFFF0000, 0xA5, 0x10000)` fills the START of the page and silently
// stops: measured here, 0xFFFF0004 became 0xA5A5A5A5 and 0xFFFFFF88 stayed
// zero, while a dword loop over the same range wrote and read back all 65,536
// bytes. The end pointer is 0x100000000, which is 0 in 32 bits, and the CRT's
// memset does not survive that.
//
// The first version of this arbitration used memset and reported "the original
// does not depend on the top page" for a case whose blamed instruction reads
// 0xFFFFFF88 - a false negative produced by the detector, on exactly the
// addresses the detector exists for.
// Saved and put back around every poisoned run. Leaving the page filled kills
// the process: with the whole 64 KiB left holding 0xA5 the harness died before
// the first case finished, every time. Something in Wine reads it. Writing it,
// running the original, and putting the previous contents straight back is
// survivable - measured over the whole plan - and it is all the arbitration
// needs, because only the original's own reads have to see the fill.
static uint8_t g_top_page_saved[OracleTopPageSize];

static void poison_top_page(uint8_t fill) {
    if (!g_top_page_writable) return;
    const uint32_t word = 0x01010101U * uint32_t(fill);
    volatile uint32_t *const page =
        reinterpret_cast<volatile uint32_t *>(OracleTopPageBase);
    for (uint32_t i = 0; i < OracleTopPageSize / 4; ++i) page[i] = word;
}

static void save_top_page() {
    if (!g_top_page_writable) return;
    const volatile uint32_t *const page =
        reinterpret_cast<const volatile uint32_t *>(OracleTopPageBase);
    uint32_t *const out = reinterpret_cast<uint32_t *>(g_top_page_saved);
    for (uint32_t i = 0; i < OracleTopPageSize / 4; ++i) out[i] = page[i];
}

static void restore_top_page() {
    if (!g_top_page_writable) return;
    volatile uint32_t *const page =
        reinterpret_cast<volatile uint32_t *>(OracleTopPageBase);
    const uint32_t *const in = reinterpret_cast<const uint32_t *>(g_top_page_saved);
    for (uint32_t i = 0; i < OracleTopPageSize / 4; ++i) page[i] = in[i];
}

bool oracle_top_page_writable() { return g_top_page_writable; }

// ---------------------------------------------------------------------------
// Flags
// ---------------------------------------------------------------------------

// The full EFLAGS word captured with pushfl, reduced to the compared set.
// Only DF comes from here; the other six keep coming from lahf/seto, which is
// already validated, so this adds a bit rather than replacing a mechanism.
static uint32_t flags_from_pushfl(uint32_t eflags) {
    return (eflags & 0x400U) ? OracleFlagDF : 0U;
}

static uint32_t flags_from_lahf(uint32_t raw) {
    uint32_t ah = (raw >> 8) & 0xFFU;
    uint32_t bits = 0;
    if (ah & 0x01U) bits |= OracleFlagCF;
    if (ah & 0x04U) bits |= OracleFlagPF;
    if (ah & 0x10U) bits |= OracleFlagAF;
    if (ah & 0x40U) bits |= OracleFlagZF;
    if (ah & 0x80U) bits |= OracleFlagSF;
    if (raw & 1U) bits |= OracleFlagOF;
    return bits;
}

// ---------------------------------------------------------------------------
// The x87 register file, reduced to what both sides define.
// ---------------------------------------------------------------------------

// C3 C2 C1 C0 of the status word. TOP and the exception bits are excluded:
// TOP is already carried by `depth` and by which register is ST(0), and the
// exception bits (IE/DE/ZE/OE/UE/PE) are not modelled by the lift at all -
// lifted_x87.h says so and says why, and comparing a field one side does not
// model produces a divergence about the model, not about the code.
constexpr uint16_t OracleX87ConditionCodes = 0x4700U;

static OracleX87 x87_from_fsave(const uint8_t *image) {
    OracleX87 x{};
    uint16_t control = 0, status = 0, tags = 0;
    std::memcpy(&control, image + 0, 2);
    std::memcpy(&status, image + 4, 2);
    std::memcpy(&tags, image + 8, 2);
    x.control = control;
    x.condition_codes = uint16_t(status & OracleX87ConditionCodes);
    const uint32_t top = (status >> 11) & 7U;
    for (uint32_t i = 0; i < 8; ++i) {
        const uint32_t physical = (top + i) & 7U;
        if (((tags >> (2U * physical)) & 3U) != 3U) x.depth++;
        // Stack order, verified above: ST(i) at 28 + 10*i.
        std::memcpy(x.st[i], image + 28 + 10 * i, 10);
    }
    return x;
}

static OracleX87 x87_from_lift(const OpensmacxX87State &state) {
    OracleX87 x{};
    x.control = state.control;
    x.condition_codes = uint16_t(state.status & OracleX87ConditionCodes);
    for (uint32_t i = 0; i < 8; ++i) {
        const uint32_t physical = (state.top + i) & 7U;
        if (!state.empty[physical]) x.depth++;
        // mingw's `long double` is the 80-bit x87 type in a 12-byte slot; only
        // the first ten bytes are the register.
        std::memcpy(x.st[i], &state.reg[physical], 10);
    }
    return x;
}

// bit 0 depth, bit 1 control, bit 2 condition codes, bits 8+i ST(i).
static uint32_t x87_difference(const OracleX87 &a, const OracleX87 &b) {
    uint32_t mask = 0;
    if (a.depth != b.depth) mask |= 1U;
    if (a.control != b.control) mask |= 2U;
    if (a.condition_codes != b.condition_codes) mask |= 4U;
    // Only the LIVE registers. An empty slot holds whatever the last value to
    // occupy it left there, and no part of the lift promises to reproduce that;
    // comparing all eight would fail every function that pops.
    const uint32_t live = a.depth < b.depth ? a.depth : b.depth;
    for (uint32_t i = 0; i < live && i < 8; ++i)
        if (std::memcmp(a.st[i], b.st[i], 10) != 0) mask |= 1U << (8 + i);
    return mask;
}

static uint32_t flags_from_eflags(uint32_t eflags) {
    uint32_t bits = 0;
    if (eflags & OpensmacxCarryFlag) bits |= OracleFlagCF;
    if (eflags & OpensmacxParityFlag) bits |= OracleFlagPF;
    if (eflags & OpensmacxAuxiliaryCarryFlag) bits |= OracleFlagAF;
    if (eflags & OpensmacxZeroFlag) bits |= OracleFlagZF;
    if (eflags & OpensmacxSignFlag) bits |= OracleFlagSF;
    if (eflags & OpensmacxOverflowFlag) bits |= OracleFlagOF;
    if (eflags & OpensmacxDirectionFlag) bits |= OracleFlagDF;
    return bits;
}

// ---------------------------------------------------------------------------
// Restoring a side from the pristine copy
//
// A page at a time, and only where the page differs. This is not just an
// optimisation. A flat 6.3 MB memcpy over the span rewrites the .text pages
// too, and those are PAGE_EXECUTE_READWRITE and have just been executed - so
// every case asks the host to invalidate 2.4 MB of translated code. On this
// host (Wine on Apple silicon, x86 under Rosetta) that reliably killed the
// process on the third case, inside the host's own memcpy, with a fault
// address unrelated to anything in the harness. Page-granular restore touches
// the code pages zero times, because the guest never writes to them, and it is
// also about twenty times faster.
// ---------------------------------------------------------------------------

static void restore_from_pristine(unsigned char *side) {
    for (uint32_t offset = 0; offset < OracleImageSize; offset += 4096) {
        uint32_t length = 4096;
        if (offset + length > OracleImageSize) length = OracleImageSize - offset;
        if (std::memcmp(side + offset, g_pristine + offset, length) != 0) {
            std::memcpy(side + offset, g_pristine + offset, length);
        }
    }
}

static void reset_both(uint32_t address, int case_index, uint32_t return_address) {
    double t0 = now_seconds();
    trace("  restore A");
    restore_from_pristine(g_side_a);
    trace("  restore B");
    restore_from_pristine(opensmacx_image);
    trace("  seed A");
    seed_scratch(g_side_a, address, case_index, return_address);
    trace("  seed B");
    seed_scratch(opensmacx_image, address, case_index, return_address);
    trace("  reset done");
    oracle_cost.reset_seconds += now_seconds() - t0;
}

static double g_lifted_t0;
static HANDLE g_worker = nullptr;
static HANDLE g_worker_start = nullptr;
static HANDLE g_worker_done = nullptr;
static uint32_t g_worker_address = 0;
static OpensmacxStaticRecompileState *g_worker_state = nullptr;
static volatile int g_worker_escape = 0;
static volatile bool g_worker_stop = false;

static volatile bool g_worker_reset_requested = false;

static DWORD WINAPI oracle_lifted_worker(LPVOID) {
    for (;;) {
        WaitForSingleObject(g_worker_start, INFINITE);
        if (g_worker_stop) return 0;
        // Both of these are PER-THREAD and must happen here, on the thread
        // that is about to run the lifted body, not on the main thread that
        // asked for them. opensmacx_x87 is a plain global, but the fldcw
        // inside opensmacx_x87_sync_host writes a register belonging to
        // whichever thread executes it, and opensmacx_tib is thread_local.
        if (g_worker_reset_requested) {
            g_worker_reset_requested = false;
            opensmacx_tib_reset();
            opensmacx_x87 = OpensmacxX87State{
                {0, 0, 0, 0, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1, 1}, 0,
                OpensmacxX87DefaultControl, 0};
            opensmacx_x87_sync_host(OpensmacxX87DefaultControl);
        }
        int escaped = setjmp(g_lifted_escape);
        if (escaped == 0) {
            g_lifted_thread_id = GetCurrentThreadId();
            g_side = OracleSideLifted;
            opensmacx_dispatch(g_worker_address)(*g_worker_state);
        }
        g_side = OracleSideIdle;
        g_worker_escape = escaped;
        SetEvent(g_worker_done);
    }
}

static void start_worker() {
    if (!g_worker_start) {
        g_worker_start = CreateEventA(nullptr, FALSE, FALSE, nullptr);
        g_worker_done = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    }
    g_worker = CreateThread(nullptr, 1 << 20, oracle_lifted_worker, nullptr, 0, nullptr);
}

// Returns 0 when the body returned, 1 or 2 for the two escapes, -1 on timeout.
static int run_lifted(uint32_t address, OpensmacxStaticRecompileState &state,
                      uint32_t timeout_ms) {
    if (!g_worker) start_worker();
    g_worker_address = address;
    g_worker_state = &state;
    g_worker_escape = 0;
    SetEvent(g_worker_start);
    if (WaitForSingleObject(g_worker_done, timeout_ms ? timeout_ms : INFINITE)
            == WAIT_OBJECT_0) {
        return g_worker_escape;
    }
    TerminateThread(g_worker, 1);
    CloseHandle(g_worker);
    g_worker = nullptr;
    g_side = OracleSideIdle;
    ResetEvent(g_worker_done);
    return -1;
}

static int oracle_run_lifted_side(uint32_t address,
                                  OpensmacxStaticRecompileState &state,
                                  OracleResult &result) {
    g_lifted_t0 = now_seconds();
    g_run_started_tick = LONG(GetTickCount());
    g_run_active = 1;
    trace("lifted in");
    const int escaped = run_lifted(address, state, g_lifted_timeout_ms);
    trace("lifted out");
    g_run_active = 0;
    g_side = OracleSideIdle;
    oracle_cost.lifted_seconds += now_seconds() - g_lifted_t0;
    if (escaped != 0) {
        if (escaped == 2) {
            result.verdict = OracleSkipTrap;
            result.trap_reason = g_trap_reason;
            result.fault_address = g_trap_address;
            return 1;
        }
        if (escaped < 0) {
            result.fault_code = OracleFaultWatchdog;
            result.verdict = OracleInconclusiveLiftedBudget;
            return 1;
        }
        result.fault_code = oracle_fault;
        result.fault_address = oracle_fault_addr;
        result.fault_data = oracle_fault_data;
        // "Out of span" is a statement about a faulting ADDRESS, so it may
        // only be made when there is one. Any other exception - an illegal
        // instruction, a divide by zero, __builtin_trap() - carries
        // NumberParameters < 2, which left fault_data at 0; and 0 duly tested
        // as "outside opensmacx_image", so an unambiguous lift bug was
        // reported as "the lifted side read outside the modelled 6.3 MB" and
        // excused. Anything that is not an access violation with a recorded
        // address is a FAILURE of the lifted side.
        const bool access_violation =
            oracle_fault == DWORD(EXCEPTION_ACCESS_VIOLATION)
            && oracle_fault_has_address;
        const uintptr_t data = uintptr_t(result.fault_data);
        const uintptr_t low = uintptr_t(opensmacx_image);
        const bool outside = data < low || data >= low + OracleImageSize;
        result.verdict = (access_violation && outside)
                             ? OracleInconclusiveOutOfSpan
                             : OracleFailLiftedFault;
        return 1;
    }
    return 0;
}

int oracle_perturb = OraclePerturbNone;

const char *oracle_perturb_name(int which) {
    static const char *const registers[8] = {
        "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    static const char *const flags[7] = {
        "CF", "PF", "AF", "ZF", "SF", "OF", "DF"};
    if (which == OraclePerturbNone) return "nothing";
    if (which <= 8) return registers[which - 1];
    if (which <= 15) return flags[which - 9];
    switch (which) {
        case OraclePerturbMemory: return "one byte of guest memory";
        case OraclePerturbX87Depth: return "the x87 stack depth";
        case OraclePerturbX87Value: return "the value in x87 ST(0)";
        case OraclePerturbX87Control: return "the x87 control word";
        case OraclePerturbX87Codes: return "the x87 condition codes";
        default: return "?";
    }
}

// Damage exactly one thing the comparison is supposed to look at. Applied to a
// COPY of the lifted state, and to the lifted image (which the next case's
// reset_both restores from pristine anyway), so nothing leaks past this case.
static void apply_perturbation(OpensmacxStaticRecompileState &state) {
    if (oracle_perturb == OraclePerturbNone) return;
    if (oracle_perturb <= 8) {
        uint32_t *const slots[8] = {
            &state.eax, &state.ecx, &state.edx, &state.ebx,
            &state.esp, &state.ebp, &state.esi, &state.edi};
        *slots[oracle_perturb - 1] ^= 0x00000010U;
        return;
    }
    if (oracle_perturb <= 15) {
        static const uint32_t bits[7] = {
            OpensmacxCarryFlag, OpensmacxParityFlag,
            OpensmacxAuxiliaryCarryFlag, OpensmacxZeroFlag,
            OpensmacxSignFlag, OpensmacxOverflowFlag, OpensmacxDirectionFlag};
        state.eflags ^= bits[oracle_perturb - 9];
        return;
    }
    if (oracle_perturb == OraclePerturbMemory) {
        // A byte in the arena: inside the span, never in code, and a place the
        // whole-image memcmp is the ONLY check that can see.
        opensmacx_image[OracleArenaBase - OracleImageBase + 0x40U] ^= 0xFFU;
    }
    // The x87 perturbations are not here: see apply_x87_perturbation.
}

// The x87 file is damaged in the COLLECTED RECORD rather than in
// opensmacx_x87, and the reason is the ST(i) comparison specifically.
//
// The selfcheck runs on a function that PASSES, and the addresses that pass are
// overwhelmingly ones that never touch the FPU - both sides finish with an
// EMPTY stack. `x87_difference` only compares ST(i) for i below the smaller of
// the two depths, which is zero there, so no amount of damage to the lifted
// registers can reach that loop: pushing a value to make it reachable changes
// the depth, and then the DEPTH bit is what fires, proving nothing about the
// value comparison.
//
// So for the value case both recorded depths are raised together, which is
// exactly the state a real x87 function ends in, and one byte of the lifted
// ST(0) is flipped. This checks the comparison, which is what a selfcheck is
// for; the model is checked by the sweep.
static void apply_x87_perturbation(OracleResult &result) {
    switch (oracle_perturb) {
        case OraclePerturbX87Depth:
            result.lifted_x87.depth ^= 1U;
            break;
        case OraclePerturbX87Value:
            if (result.original_x87.depth == 0) {
                result.original_x87.depth = 1;
                result.lifted_x87.depth = 1;
            }
            result.lifted_x87.st[0][0] ^= 0xFFU;
            break;
        case OraclePerturbX87Control:
            result.lifted_x87.control ^= 0x0300U;   // precision control
            break;
        case OraclePerturbX87Codes:
            result.lifted_x87.condition_codes ^= 0x4000U;   // C3
            break;
        default: break;
    }
}

static void oracle_collect_lifted(const OpensmacxStaticRecompileState &state,
                                  OracleResult &result) {
    result.lifted.eax = state.eax; result.lifted.ecx = state.ecx;
    result.lifted.edx = state.edx; result.lifted.ebx = state.ebx;
    result.lifted.esp = state.esp; result.lifted.ebp = state.ebp;
    result.lifted.esi = state.esi; result.lifted.edi = state.edi;
    result.lifted.eflags = flags_from_eflags(state.eflags);
    result.lifted_x87 = x87_from_lift(opensmacx_x87);
    apply_x87_perturbation(result);

    // --- compare -----------------------------------------------------------
    const double t0 = now_seconds();
    const uint32_t *a = reinterpret_cast<const uint32_t *>(&result.original);
    const uint32_t *b = reinterpret_cast<const uint32_t *>(&result.lifted);
    for (int i = 0; i < 8; ++i) {
        if (a[i] != b[i]) result.register_mask |= 1U << i;
    }
    result.flag_mask = result.original.eflags ^ result.lifted.eflags;
    result.x87_mask = x87_difference(result.original_x87, result.lifted_x87);

    trace("compare");
    if (std::memcmp(g_side_a, opensmacx_image, OracleImageSize) != 0) {
        const uint32_t *wa = reinterpret_cast<const uint32_t *>(g_side_a);
        const uint32_t *wb = reinterpret_cast<const uint32_t *>(opensmacx_image);
        const uint32_t *wp = reinterpret_cast<const uint32_t *>(g_pristine);
        for (uint32_t i = 0; i < OracleImageSize / 4; ++i) {
            if (wa[i] == wb[i]) continue;
            if (!result.memory_diffs) {
                result.first_diff = OracleImageBase + i * 4;
                result.first_diff_original = wa[i];
                result.first_diff_lifted = wb[i];
                result.first_diff_pristine = wp[i];
            }
            result.memory_diffs++;
        }
    }
    oracle_cost.compare_seconds += now_seconds() - t0;
    oracle_cost.cases++;

    result.verdict = (result.register_mask || result.flag_mask ||
                      result.memory_diffs || result.x87_mask)
                         ? OracleFail : OraclePass;
}

// ---------------------------------------------------------------------------
// One case: reset both memories, seed, run the original, run the lifted body,
// compare.
//
// `result` lives at file scope because the lifted side can leave by longjmp,
// and a local the compiler had parked in a register would come back holding
// whatever it held when setjmp ran.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Top-page arbitration. See the block comment beside poison_top_page.
// ---------------------------------------------------------------------------

struct OracleOriginalRun {
    uint32_t out[10];
    uint32_t fault;
    uint32_t checksum;   // FNV-1a over the whole of side A after the run
};

static OracleOriginalRun run_original_once(uint32_t address, int case_index,
                                           uint32_t return_address, uint8_t fill) {
    restore_from_pristine(g_side_a);
    seed_scratch(g_side_a, address, case_index, return_address);
    OracleRegisters before;
    seed_registers(address, case_index, &before);
    oracle_in[0] = before.eax; oracle_in[1] = before.ecx;
    oracle_in[2] = before.edx; oracle_in[3] = before.ebx;
    oracle_in[4] = before.esp; oracle_in[5] = before.ebp;
    oracle_in[6] = before.esi; oracle_in[7] = before.edi;
    oracle_in[8] = before.eflags | (oracle_budget ? 0x100U : 0U);
    oracle_target = address;
    std::memset(oracle_out, 0, sizeof oracle_out);
    save_top_page();
    poison_top_page(fill);

    g_original_thread_id = GetCurrentThreadId();
    g_side = OracleSideOriginal;
    g_run_started_tick = LONG(GetTickCount());
    g_run_active = 1;
    oracle_run_original();
    g_run_active = 0;
    g_side = OracleSideIdle;
    restore_top_page();

    OracleOriginalRun run{};
    std::memcpy(run.out, oracle_out, sizeof run.out);
    run.fault = oracle_fault;
    uint32_t hash = 2166136261U;
    const uint32_t *words = reinterpret_cast<const uint32_t *>(g_side_a);
    for (uint32_t i = 0; i < OracleImageSize / 4; ++i) {
        hash ^= words[i];
        hash *= 16777619U;
    }
    run.checksum = hash;
    return run;
}

// Two runs, two fills. Registers, flags, whether it faulted, and every byte it
// wrote are compared; if any of them moved, the original was reading the top
// page and this case cannot be judged.
static bool original_depends_on_top_page(uint32_t address, int case_index,
                                         uint32_t return_address) {
    if (!g_top_page_writable) return false;
    const OracleOriginalRun a =
        run_original_once(address, case_index, return_address, 0x00);
    const OracleOriginalRun b =
        run_original_once(address, case_index, return_address, 0xA5);
    return a.fault != b.fault || a.checksum != b.checksum ||
           std::memcmp(a.out, b.out, sizeof a.out) != 0;
}

void oracle_dump_seed(uint32_t address, int case_index) {
    if (!g_image_loaded) return;
    const uint32_t return_address = uint32_t(uintptr_t(&oracle_return_point));
    reset_both(address, case_index, return_address);
    OracleRegisters r;
    seed_registers(address, case_index, &r);
    std::printf("seed %#010x case %d: %s\n", unsigned(address), case_index,
                oracle_seed_description(address, case_index));
    std::printf("  eax %08x ecx %08x edx %08x ebx %08x\n"
                "  esp %08x ebp %08x esi %08x edi %08x eflags %08x\n",
                unsigned(r.eax), unsigned(r.ecx), unsigned(r.edx), unsigned(r.ebx),
                unsigned(r.esp), unsigned(r.ebp), unsigned(r.esi), unsigned(r.edi),
                unsigned(r.eflags));
    const uint32_t centre = r.ecx;
    for (int32_t offset = -32; offset <= 64; offset += 4) {
        const uint32_t guest = centre + uint32_t(offset);
        if (guest < OracleImageBase || guest >= OracleImageBase + OracleImageSize)
            continue;
        uint32_t word = 0;
        std::memcpy(&word, g_side_a + (guest - OracleImageBase), 4);
        std::printf("  [ecx%+d] %#010x = %08x\n", int(offset), unsigned(guest),
                    unsigned(word));
    }
    for (uint32_t i = 0; i < OracleSeedArgWords; ++i) {
        uint32_t word = 0;
        std::memcpy(&word, g_side_a + (OracleStackTop + 4 + i * 4 - OracleImageBase), 4);
        std::printf("  arg %u = %08x\n", unsigned(i), unsigned(word));
    }
}

static OracleResult g_result;

OracleResult oracle_run_case(uint32_t address, int case_index) {
    OracleResult &result = g_result;
    std::memset(&result, 0, sizeof result);
    result.address = address;
    result.case_index = case_index;
    result.verdict = OracleUnrun;
    if (!g_image_loaded) return result;

    const uint32_t return_address = uint32_t(uintptr_t(&oracle_return_point));
    reset_both(address, case_index, return_address);
    seed_registers(address, case_index, &result.before);

    // --- the original ------------------------------------------------------
    oracle_in[0] = result.before.eax; oracle_in[1] = result.before.ecx;
    oracle_in[2] = result.before.edx; oracle_in[3] = result.before.ebx;
    oracle_in[4] = result.before.esp; oracle_in[5] = result.before.ebp;
    oracle_in[6] = result.before.esi; oracle_in[7] = result.before.edi;
    oracle_in[8] = result.before.eflags | (oracle_budget ? 0x100U : 0U);
    oracle_target = address;
    std::memset(oracle_out, 0, sizeof oracle_out);

    g_original_thread_id = GetCurrentThreadId();
    g_side = OracleSideOriginal;
    g_run_started_tick = LONG(GetTickCount());
    g_run_active = 1;
    const double t0 = now_seconds();
    trace("original in");
    oracle_run_original();
    trace("original out");
    oracle_cost.original_seconds += now_seconds() - t0;
    g_run_active = 0;
    g_side = OracleSideIdle;

    result.original_steps = oracle_steps;
    if (oracle_fault) {
        result.fault_code = oracle_fault;
        result.fault_address = oracle_fault_addr;
        result.fault_data = oracle_fault_data;
        result.verdict = (oracle_fault == OracleFaultBudget ||
                          oracle_fault == OracleFaultWatchdog)
                             ? OracleInconclusiveBudget
                             : OracleInconclusiveFault;
        return result;
    }
    result.original.eax = oracle_out[0]; result.original.ecx = oracle_out[1];
    result.original.edx = oracle_out[2]; result.original.ebx = oracle_out[3];
    result.original.esp = oracle_out[4]; result.original.ebp = oracle_out[5];
    result.original.esi = oracle_out[6]; result.original.edi = oracle_out[7];
    result.original.eflags = flags_from_lahf(oracle_out[8])
                          | flags_from_pushfl(oracle_out[9]);
    result.original_x87 = x87_from_fsave(oracle_x87_save);

    // --- the lifted side ---------------------------------------------------
    static OpensmacxStaticRecompileState state;
    state.eax = result.before.eax; state.ecx = result.before.ecx;
    state.edx = result.before.edx; state.ebx = result.before.ebx;
    state.esp = result.before.esp; state.ebp = result.before.ebp;
    state.esi = result.before.esi; state.edi = result.before.edi;
    state.eflags = result.before.eflags;

    // The per-case x87 and TIB reset is REQUESTED here and performed by the
    // worker, because both are per-thread and every lifted body runs on the
    // worker: opensmacx_x87_sync_host is an `fldcw`, which sets a register on
    // whichever thread executes it, and opensmacx_tib is `inline
    // thread_local`. Doing them here reset the MAIN thread's state and left
    // the worker carrying whatever the previous case's `fldcw` had set - so
    // host rounding mode and precision control leaked between cases, and a
    // sweep failure would not reproduce under `--only`.
    g_worker_reset_requested = true;
    oracle_lifted_trapped = false;
    g_trap_reason = nullptr;
    oracle_fault = 0;

    if (oracle_run_lifted_side(address, state, result)) return result;

    apply_perturbation(state);
    oracle_collect_lifted(state, result);
    // Arbitrate a failure against the one range the harness cannot model.
    // Skipped while perturbing, because the selfcheck's failures are
    // manufactured on purpose and paying two extra original runs for each of
    // twenty of them is waste, not caution.
    if (result.verdict == OracleFail && oracle_perturb == OraclePerturbNone &&
        original_depends_on_top_page(address, case_index, return_address)) {
        result.verdict = OracleInconclusiveTopPage;
        result.trap_reason = "the original read the unmodellable top 64 KiB";
    }
    return result;
}

// ---------------------------------------------------------------------------
// Attribution: which instruction produced the divergence.
// ---------------------------------------------------------------------------

uint32_t oracle_blame(uint32_t address, int case_index, uint32_t watch_address,
                      int watch_register) {
    if (!g_image_loaded) return 0;
    const uint32_t return_address = uint32_t(uintptr_t(&oracle_return_point));
    reset_both(address, case_index, return_address);
    OracleRegisters before;
    seed_registers(address, case_index, &before);

    oracle_in[0] = before.eax; oracle_in[1] = before.ecx; oracle_in[2] = before.edx;
    oracle_in[3] = before.ebx; oracle_in[4] = before.esp; oracle_in[5] = before.ebp;
    oracle_in[6] = before.esi; oracle_in[7] = before.edi;
    oracle_in[8] = before.eflags | 0x100U;   // trap flag: single-step the run
    oracle_target = address;

    // A register watch takes precedence, and a memory watch has to be a guest
    // address: `first_diff` is zero when the divergence was in a register only,
    // and 0 - OracleImageBase wraps to a host pointer at 0. That mistake cost a
    // whole 400-function run, and it is the harness's own fault landing in the
    // middle of someone else's failure report.
    if (watch_register < 0 &&
        (watch_address < OracleImageBase ||
         watch_address >= OracleImageBase + OracleImageSize)) {
        return 0;
    }
    g_blaming = true;
    g_watch_address = watch_address;
    g_watch_register = watch_register;
    g_blame_eip = 0;
    g_blame_last_eip = address;
    if (watch_register >= 0) {
        g_watch_value = reinterpret_cast<const uint32_t *>(&before)[watch_register];
    } else {
        std::memcpy(&g_watch_value, g_side_a + (watch_address - OracleImageBase), 4);
    }

    uint32_t saved_budget = oracle_budget;
    if (!oracle_budget) oracle_budget = 2000000;
    g_original_thread_id = GetCurrentThreadId();
    g_side = OracleSideOriginal;
    g_run_started_tick = LONG(GetTickCount());
    g_run_active = 1;
    oracle_run_original();
    g_run_active = 0;
    g_side = OracleSideIdle;
    oracle_budget = saved_budget;
    g_blaming = false;
    return g_blame_eip;
}

const char *oracle_verdict_name(OracleVerdict verdict) {
    switch (verdict) {
        case OraclePass: return "PASS";
        case OracleFail: return "FAIL";
        case OracleFailLiftedFault: return "FAIL-lifted-fault";
        case OracleSkipTrap: return "SKIP-trap";
        case OracleInconclusiveFault: return "INCONCLUSIVE-original-fault";
        // One verdict, two very different causes, and the name only ever said
        // the first: a sweep run with budget 0 cannot produce a single budget
        // kill, so every one of these was the WATCHDOG - a wall-clock timeout,
        // not an instruction count. The distinction matters because the budget
        // is unusable on this host (see the --budget warning) while the
        // watchdog is the guard that actually fires.
        case OracleInconclusiveBudget: return "INCONCLUSIVE-original-timeout";
        case OracleInconclusiveLiftedBudget: return "INCONCLUSIVE-lifted-budget";
        case OracleInconclusiveOutOfSpan: return "INCONCLUSIVE-lifted-out-of-span";
        case OracleInconclusiveTopPage: return "INCONCLUSIVE-original-top-page";
        default: return "UNRUN";
    }
}
