// Run real instruction bytes on the real CPU, and report the state they leave.
//
// WHY THIS EXISTS. The ALU encoding is validated against lifted_x86.h, which is
// a fair reference for arithmetic because it is the semantics the lifted image
// runs. Memory and control flow have no such reference: writing a second
// interpreter to check the encoding against would be a THIRD model of x86 and
// would agree with a shared misunderstanding exactly as readily as the first
// two. So this compares against the only authority that cannot be argued with
// - the processor.
//
// It is the same idea as `oracle_run_original` in the differential oracle,
// shrunk to one basic block and no guest image: registers in, bytes executed,
// registers and a memory window out.
//
//   stdin:  <hex bytes> <eax> <ecx> <edx> <ebx> <ebp> <esi> <edi> <eflags> <hex window>
//   stdout: <eax> <ecx> <edx> <ebx> <ebp> <esi> <edi> <eflags> <hex window>
//
// The window is OpensmacxSmtWindowSize bytes committed at a FIXED address, so a
// formula and an execution can name the same memory. ESP points into a
// separate stack area inside the same allocation, high enough that a push
// cannot reach the window.
//
// The instruction bytes must end with a `ret`. Nothing here validates that:
// the caller assembles the sequence and is responsible for it, and a sequence
// that runs off the end faults, which is a loud failure rather than a wrong
// answer.

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {

// Fixed so the encoder can hard-code the same number. Anything else and a
// formula about address 0x30000000 would be compared against an execution that
// used a different one.
constexpr uint32_t kWindowBase = 0x30000000U;
constexpr uint32_t kWindowSize = 256U;
constexpr uint32_t kStackBase = 0x30010000U;
constexpr uint32_t kStackSize = 0x10000U;

}  // namespace

// C linkage and external, because the assembly below names them directly. In
// an anonymous namespace they would be mangled and internal, and the assembler
// would emit references nothing defines.
extern "C" {
uint32_t g_in[8];      // eax ecx edx ebx ebp esi edi eflags
uint32_t g_out[8];
uint32_t g_saved_esp;
void *g_code;
}

namespace {

// Load the state, call the code, save what comes back. EFLAGS is restored with
// popfd and read back with pushfd; unlike the oracle this program has no guest
// memory whose bytes are compared, so writing four bytes of its own stack costs
// nothing.
extern "C" void smt_run();
asm(".text\n"
    ".globl _smt_run\n"
    "_smt_run:\n"
    "  pushl %ebp\n"
    "  pushl %ebx\n"
    "  pushl %esi\n"
    "  pushl %edi\n"
    "  movl %esp, _g_saved_esp\n"
    "  movl _g_in+0,  %eax\n"
    "  movl _g_in+4,  %ecx\n"
    "  movl _g_in+8,  %edx\n"
    "  movl _g_in+12, %ebx\n"
    "  movl _g_in+16, %ebp\n"
    "  movl _g_in+20, %esi\n"
    "  movl _g_in+24, %edi\n"
    "  pushl _g_in+28\n"
    "  popfl\n"
    "  call *_g_code\n"
    "  movl %eax, _g_out+0\n"
    "  movl %ecx, _g_out+4\n"
    "  movl %edx, _g_out+8\n"
    "  movl %ebx, _g_out+12\n"
    "  movl %ebp, _g_out+16\n"
    "  movl %esi, _g_out+20\n"
    "  movl %edi, _g_out+24\n"
    "  pushfl\n"
    "  popl %eax\n"
    "  movl %eax, _g_out+28\n"
    "  movl _g_saved_esp, %esp\n"
    "  popl %edi\n"
    "  popl %esi\n"
    "  popl %ebx\n"
    "  popl %ebp\n"
    "  ret\n");

std::vector<uint8_t> unhex(const std::string &text) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i + 1 < text.size(); i += 2) {
        bytes.push_back(uint8_t(std::stoul(text.substr(i, 2), nullptr, 16)));
    }
    return bytes;
}

}  // namespace

int main() {
    unsigned char *window = static_cast<unsigned char *>(VirtualAlloc(
        reinterpret_cast<void *>(uintptr_t(kWindowBase)),
        kStackBase + kStackSize - kWindowBase, MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE));
    if (!window) {
        std::fprintf(stderr, "cannot reserve the window at %08x (error %lu)\n",
                     kWindowBase, GetLastError());
        return 2;
    }

    std::string line;
    char buffer[65536];
    while (std::fgets(buffer, sizeof buffer, stdin)) {
        std::string text(buffer);
        char code_hex[8192] = {}, window_hex[2048] = {};
        unsigned regs[8] = {};
        if (std::sscanf(text.c_str(), "%8191s %x %x %x %x %x %x %x %x %2047s",
                        code_hex, &regs[0], &regs[1], &regs[2], &regs[3],
                        &regs[4], &regs[5], &regs[6], &regs[7],
                        window_hex) != 10) {
            std::printf("bad\n");
            std::fflush(stdout);
            continue;
        }
        const std::vector<uint8_t> code = unhex(code_hex);
        const std::vector<uint8_t> initial = unhex(window_hex);

        std::memset(window, 0, kWindowSize);
        std::memcpy(window, initial.data(),
                    initial.size() < kWindowSize ? initial.size() : kWindowSize);

        void *page = VirtualAlloc(nullptr, 4096, MEM_COMMIT,
                                  PAGE_EXECUTE_READWRITE);
        std::memcpy(page, code.data(), code.size());
        g_code = page;
        for (int i = 0; i < 8; ++i) g_in[i] = regs[i];
        // Only the flags the encoding models; the rest are the host's and
        // comparing them would report differences that mean nothing.
        g_in[7] = (regs[7] & 0x8D5U) | 0x202U;
        std::memset(g_out, 0, sizeof g_out);
        smt_run();
        VirtualFree(page, 0, MEM_RELEASE);

        std::printf("%08x %08x %08x %08x %08x %08x %08x %08x ",
                    g_out[0], g_out[1], g_out[2], g_out[3],
                    g_out[4], g_out[5], g_out[6], g_out[7] & 0x8D5U);
        for (uint32_t i = 0; i < kWindowSize; ++i) {
            std::printf("%02x", window[i]);
        }
        std::printf("\n");
        std::fflush(stdout);
    }
    return 0;
}
