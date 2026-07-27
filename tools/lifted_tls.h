#ifndef OPENSMACX_LIFTED_TLS_H
#define OPENSMACX_LIFTED_TLS_H

// The per-thread block that `fs:` names, for the whole-image static
// recompilation.
//
// ---------------------------------------------------------------------------
// WHAT THE IMAGE ACTUALLY DOES  (measured, not assumed)
// ---------------------------------------------------------------------------
//
// Every segment-prefixed memory operand in terranx.exe was decoded before any
// of this was designed. The result is the narrowest distribution in the whole
// lift:
//
//     1,330 fs:-prefixed operands, in 389 of the 5,673 functions
//       displacement    fs:[0]      1,330  (100%)
//       operand width   32-bit      1,330  (100%)
//       mnemonic        mov         1,330  (100%)
//       base/index      none        1,330  (100%)  - the displacement is
//                                                    the entire address
//
//     Three distinct operand shapes account for all 1,330:
//         552  mov dword ptr fs:[0], ecx     restore previous handler
//         389  mov eax, dword ptr fs:[0]     read current handler
//         389  mov dword ptr fs:[0], esp     install new handler
//
//     Per function: 284 functions use three, 65 use four, 28 use five, 10 use
//     six, one uses seven, one - Console::destroy - uses eleven.
//
// 305 further operands carry an `es:` prefix, but all 305 are the implicit
// destination of a string instruction (`rep stosd`, `rep movsd`, `rep movsb`,
// `repne scasb`). Those have no displacement of their own and belong to the
// string-instruction lowering, not here.
//
// So the entire fs: surface of this image is ONE field: fs:[0], the head of
// the SEH exception registration chain. fs:[4]/fs:[8] (stack base and limit),
// fs:[0x18] (the TIB self-pointer) and fs:[0x2C] (the TLS array) are the usual
// suspects and this image reads NONE of them, zero times each. The block below
// is nevertheless a real block rather than a single variable, because a helper
// keyed by displacement costs nothing extra and the next image, or the CRT
// startup path once it is lifted, will want fs:[0x2C].
//
// ---------------------------------------------------------------------------
// WHY THIS IS NOT AN OFFSET INTO opensmacx_image
// ---------------------------------------------------------------------------
//
// `opensmacx_image` models ONE address space: the guest's flat 0x00400000-based
// linear space, where `opensmacx_at(a) == opensmacx_image + (a - 0x00400000)`.
// The thread block is not in that space. On real Win32 the FS selector has a
// base the kernel picked per thread, somewhere entirely unrelated to the
// module, and `fs:[0]` means "0 bytes past that base" - it is not linear
// address 0 and it is not linear address 0x00400000.
//
// Conflating them corrupts the image, and it does so in one of two ways
// depending on which wrong thing is done:
//
//   * Treat the displacement as a flat address. `opensmacx_mem32(0)` evaluates
//     `opensmacx_image + (0 - 0x00400000)`, i.e. four megabytes BEFORE the
//     array. That is undefined behaviour on every one of the 1,330 sites, and
//     941 of them are STORES, so it is a four-megabyte-wild write executed in
//     the prologue and epilogue of 389 functions.
//
//   * "Fix" that by rebasing the displacement onto the image, e.g.
//     `opensmacx_mem32(OpensmacxImageBase + disp)`. Now fs:[0] aliases
//     0x00400000 exactly - the PE `MZ` signature in the DOS header - and the
//     389 `mov fs:[0], esp` sites each overwrite the first four bytes of the
//     mapped executable with a stack pointer. Any code that later reads the
//     header, or any data placed near the image base, silently changes value
//     depending on how deep the call stack was.
//
// Both are silent: nothing crashes at the store, the damage surfaces
// arbitrarily far away. Hence a separate object, in its own namespace, with no
// arithmetic connecting it to `opensmacx_image`. This header deliberately does
// not include "lifted_runtime.h" and cannot name `opensmacx_image` at all, so
// the separation is structural rather than a convention someone has to
// remember.
//
// ---------------------------------------------------------------------------
// SEH: THIS MODELS THE CHAIN AS MEMORY. IT DOES NOT DISPATCH EXCEPTIONS.
// ---------------------------------------------------------------------------
//
// READ THIS BEFORE CONCLUDING THAT SEH WORKS.
//
// What the 1,330 instructions do is maintain a singly linked list whose head
// lives at fs:[0] and whose nodes live on the guest stack:
//
//     push  offset __handler          ; node.handler
//     mov   eax, fs:[0]               ; node.prev = old head
//     push  eax
//     mov   fs:[0], esp               ; head = &node          (install)
//     ...
//     mov   ecx, [ebp-0xC]            ; node.prev
//     mov   fs:[0], ecx               ; head = node.prev      (remove)
//
// Making those instructions execute correctly is a MEMORY problem and nothing
// else: fs:[0] has to hold whatever was last stored, and hand back exactly
// that. That is all this header implements, and it is the right scope right
// now for a specific reason - a handler routine is an ordinary function at an
// ordinary address, so it lifts like any other function through the same
// dispatch table, and it only ever runs when a fault occurs. A CORRECT lift
// does not fault. So the chain has to be maintained (otherwise the prologue
// and epilogue of 389 functions are wrong, and the epilogue's store is
// load-bearing for the stack pointer bookkeeping around it), while dispatch
// has no caller until something is already broken.
//
// What is therefore NOT here, and must not be assumed from a green test:
//
//   * No exception dispatch. Nothing walks this chain. A host SIGSEGV, an
//     integer divide by zero, an explicit RaiseException - none of them reach
//     a lifted handler.
//   * No unwinding. `__try`/`__finally` bodies, the MSVC scope tables reached
//     through the handler's own trylevel field, and the destructor calls a
//     C++ unwind would make, do not run.
//   * No structural validation. Nothing checks that a stored value points at a
//     plausible record, that the chain terminates, or that removal restores
//     the value install saved. An install/remove sequence that round-trips
//     proves the MEMORY is faithful, and proves nothing whatsoever about
//     unwinding.
//
// If a lifted body ever needs a fault delivered, that is a new piece of work -
// a dispatcher that walks from `opensmacx_fs32(0)` through guest-stack records
// until the 0xFFFFFFFF terminator, translating each handler address through
// `opensmacx_dispatch` - and it goes in its own header, not by quietly
// extending this one.
//
// ---------------------------------------------------------------------------
// HOW THIS WAS CHECKED
// ---------------------------------------------------------------------------
//
// Against the real instruction, under Wine, the same way lifted_x86.h was: the
// reference side of the test executes actual fs:-prefixed x86 on an actual
// Win32 thread block, and the model runs beside it.
//
//   * The three measured shapes, as a genuine two-deep install/remove, driven
//     by real EXCEPTION_REGISTRATION_RECORDs on the real stack through the
//     real fs:[0] (saved and restored around the test).
//   * A value-by-value differential at fs:[0x14], NT_TIB's
//     ArbitraryUserPointer - the one TIB word the kernel documents as never
//     touching, so it is the only displacement a test may write at 8, 16 and
//     32 bits without racing the OS. Two different displacements are used on
//     purpose: one cannot tell a correct displacement-to-field map from a
//     model that ignores the displacement entirely.
//   * End to end on GENERATED code: the lifted prologue and epilogue of
//     MCIVideo::~MCIVideo (0x004042C0), pasted verbatim, run against a
//     modelled guest stack beside the same original instruction sequence
//     assembled and executed for real. That is the only test that can catch a
//     model where every helper is right and the registration record still
//     lands at the wrong stack address.
//
// 527 checks, 0 mismatches. 18 deliberate mutations of this header - shifted
// displacements, widened stores, a shared block instead of a per-thread one, a
// bounds check rewritten as the wrapping addition, the TIB aliased onto the
// image base - all 18 compiled, and all 18 were caught.
//
// What that does NOT establish, restating the section above because a green
// test is exactly how this gets misread: nothing here has ever dispatched an
// exception, because nothing here can.
//
// ---------------------------------------------------------------------------

#include <cstdint>

// Declared, not included: `opensmacx_trap` is defined by the generated runtime
// (see lift_whole_image.py's RUNTIME_HEADER). Declaring it keeps this header
// free of "lifted_runtime.h", which is what makes it impossible for anything
// below to reach `opensmacx_image` even by accident.
[[noreturn]] void opensmacx_trap(uint32_t address, const char *reason);

// 256 bytes covers the whole NT_TIB (0x00..0x38) plus the TEB fields with
// small displacements that x86 code reaches directly: the TLS array at 0x2C,
// the process and thread ids at 0x20/0x24, the PEB pointer at 0x30, the last
// error at 0x34. Everything this image touches is at 0x00. A displacement past
// the end traps rather than wrapping, because a wrap would alias two unrelated
// fields onto one word - the same class of silent corruption this whole header
// exists to avoid.
constexpr uint32_t OpensmacxTibSize = 0x100U;

// Named displacements, for readers of the lifted output. Only the first is
// exercised by terranx.exe; the rest document what the numbers mean when they
// eventually appear.
constexpr uint32_t OpensmacxTibSehChainHead = 0x00U;  // used, 1,330 times
constexpr uint32_t OpensmacxTibStackBase = 0x04U;     // unused by this image
constexpr uint32_t OpensmacxTibStackLimit = 0x08U;    // unused by this image
constexpr uint32_t OpensmacxTibSelfPointer = 0x18U;   // unused by this image
constexpr uint32_t OpensmacxTibThreadId = 0x24U;      // unused by this image
constexpr uint32_t OpensmacxTibTlsArray = 0x2CU;      // unused by this image

// Windows terminates the registration chain with -1, not with a null pointer,
// precisely so that the terminator is not a plausible record address. An
// untouched fs:[0] must therefore read as -1: it is the value a real thread
// starts with once the outermost frame's record is gone, and it is what the
// FIRST lifted prologue on a thread reads and pushes as its `prev` link. Zero
// would be a defined value too, and a worse one - it would make an empty chain
// look like a chain with a record at address 0.
constexpr uint32_t OpensmacxSehEndOfChain = 0xFFFFFFFFU;

// A byte array, accessed by memcpy, for the same reason lifted_x86.h uses
// memcpy for guest memory: a lifted access is only as aligned and only as wide
// as the original instruction made it, and the host is not obliged to tolerate
// what x86 tolerated. Every byte reads as a defined value before anything
// writes it - zero, except the chain head.
struct OpensmacxThreadBlock {
    unsigned char byte[OpensmacxTibSize];

    constexpr OpensmacxThreadBlock() : byte() {
        // Little-endian store of the terminator into fs:[0], written out
        // rather than memcpy'd because this has to run at constant-evaluation
        // time so that a fresh thread's block needs no runtime initialiser.
        byte[OpensmacxTibSehChainHead + 0] = 0xFFU;
        byte[OpensmacxTibSehChainHead + 1] = 0xFFU;
        byte[OpensmacxTibSehChainHead + 2] = 0xFFU;
        byte[OpensmacxTibSehChainHead + 3] = 0xFFU;
    }
};

// `thread_local` is the whole point of the name TIB. The SEH chain head is
// per-thread on the real machine - two threads in the same lifted image are in
// different call stacks with different registration records - so sharing one
// block would make thread B's epilogue restore thread A's handler. The cost is
// a TLS base lookup at 1,330 static sites, every one of them already in a
// function prologue or epilogue next to a stack adjustment, which is not a hot
// path in any sense that matters.
inline thread_local OpensmacxThreadBlock opensmacx_tib;

// The block is addressed by DISPLACEMENT, deliberately: there is no linear
// address for it and inventing one would put us right back at the aliasing
// problem above. `disp` is exactly the constant that appeared after `fs:` in
// the original encoding.
inline unsigned char *opensmacx_tib_at(uint32_t disp, uint32_t width) {
    // Written as a subtraction on the CONSTANT rather than as
    // `disp + width > OpensmacxTibSize`, because `disp` is an arbitrary 32-bit
    // value out of the encoding and the addition would wrap for a large one,
    // turning the bounds check into a way past the bounds check.
    if (disp > OpensmacxTibSize - width) {
        // Larger displacements are real TEB fields (TLS slots at 0xE10, the
        // deallocation stack at 0xE0C) and reaching one means the model is too
        // small, not that the program is wrong. Trap so it says so, instead of
        // truncating into an unrelated field.
        opensmacx_trap(disp, "fs: displacement outside the modelled TIB");
    }
    return opensmacx_tib.byte + disp;
}

inline uint32_t opensmacx_fs32(uint32_t disp) {
    uint32_t value;
    __builtin_memcpy(&value, opensmacx_tib_at(disp, 4), sizeof(value));
    return value;
}

inline uint32_t opensmacx_fs16(uint32_t disp) {
    uint16_t value;
    __builtin_memcpy(&value, opensmacx_tib_at(disp, 2), sizeof(value));
    return value;
}

inline uint32_t opensmacx_fs8(uint32_t disp) {
    return *opensmacx_tib_at(disp, 1);
}

inline void opensmacx_store_fs32(uint32_t disp, uint32_t value) {
    __builtin_memcpy(opensmacx_tib_at(disp, 4), &value, sizeof(value));
}

inline void opensmacx_store_fs16(uint32_t disp, uint32_t value) {
    const uint16_t narrow = static_cast<uint16_t>(value);
    __builtin_memcpy(opensmacx_tib_at(disp, 2), &narrow, sizeof(narrow));
}

inline void opensmacx_store_fs8(uint32_t disp, uint32_t value) {
    *opensmacx_tib_at(disp, 1) = static_cast<unsigned char>(value);
}

// Only the 32-bit pair is reached by this image; the 8- and 16-bit forms exist
// so that the lowerer's width dispatch is total and a `mov al, fs:[x]` becomes
// a translation rather than a link error. They are held to the same rule: a
// narrow write merges into the block exactly as wide as the instruction was.

// Resets the calling thread's block to its start-of-thread state. Only the
// tests and a future thread-entry shim need this; a lifted body never does.
inline void opensmacx_tib_reset() {
    opensmacx_tib = OpensmacxThreadBlock();
}

#endif
