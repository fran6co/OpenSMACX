#ifndef OPENSMACX_LIFTED_X87_H
#define OPENSMACX_LIFTED_X87_H

// x87 floating-point semantics for the whole-image static recompilation.
//
// The x87 is a second machine bolted to the first, and it does not fit in
// `OpensmacxStaticRecompileState` because nothing about it is a general
// register: it is eight slots addressed RELATIVE to a top-of-stack index that
// the instructions themselves move. `ST(0)` names a different physical slot
// after every push. So the state lives here, in one global, exactly as the
// hardware keeps it beside the integer registers rather than inside them.
//
// 135 of the image's 5,673 functions touch x87 at all, but they contain 3,563
// instructions across 32 mnemonics, and they are the ones that compute: terrain
// interpolation, the texture rasterisers, the AI's value functions.
//
// ---------------------------------------------------------------------------
// THE REGISTER IS 80 BITS, AND SO IS THIS MODEL
// ---------------------------------------------------------------------------
//
// An earlier version of this header kept each stack slot in a `double` and
// documented the missing 11 significand bits as an accepted fidelity limit,
// on the argument that the process runs at PC = 53 where the hardware rounds
// to double anyway. Independent verification refuted the argument three ways,
// and all three are fixed here by keeping the slot in the type the machine
// actually has:
//
// 1. **FILD is a LOAD, and loads ignore precision control.** `fild qword` of
//    2^53 + 1 lands in the register exactly even at PC = 53, and `fistp qword`
//    hands it straight back. Through a `double` slot it came back rounded, and
//    INT64_MAX came back as the integer indefinite - a sign flip, not a last
//    bit. Precision control never entered into it, so no argument about PC
//    could have saved it.
//
// 2. **The exponent field is 15 bits at EVERY precision control.** An
//    intermediate that leaves double range - `1e-300 * 1e-100 * 1e200`, or
//    `DBL_MAX * 10 * 0.1` - survives in the register and comes back an
//    ordinary double. Through a `double` slot it became 0 or infinity, which
//    is not a rounding difference but a destroyed value.
//
// 3. **PC = 64 is this image's steady state, not a corner.** All 54 `fninit`
//    sites are inside the 27 texture rasterisers, none is followed by an
//    `fldcw`, and `fninit` sets PC = 64. So from the first texture draw
//    onwards every x87 function outside the rasterisers' own
//    `fnstcw; and; or; fldcw` blocks runs at PC = 64 for the rest of the
//    process. Measured in this image's own idiom, a `double` model diverged in
//    the last bit on 5.18% of three-operation chains there.
//
// So: **a stack slot is a `long double`.** On the i686 target that IS the
// 80-bit x87 register - 64 significand bits, 15 exponent bits - and every
// arithmetic expression below compiles to the very instruction being modelled,
// executed by the FPU the emulated program is asking about.
//
// ---------------------------------------------------------------------------
// THE CONTROL WORD IS MIRRORED INTO THE HOST, WHICH IS WHAT MAKES IT EXACT
// ---------------------------------------------------------------------------
//
// Rounding is not reimplemented here. `opensmacx_x87_sync_host` copies the
// emulated precision and rounding fields into the host control word, so
// `left + right` on two `long double`s rounds to the significand width and in
// the direction the guest asked for, because it is one `fadd` under that
// control word. The same goes for the narrowing stores: `static_cast<float>`
// of a `long double` is `fstps`, which is FST m32 including its overflow to
// infinity and its directed rounding.
//
// What is NOT mirrored is the exception-mask field: the host keeps all six
// exceptions masked whatever the guest's control word says, because this model
// does not implement exception delivery and an unmasked host exception would
// fault the process rather than set a status bit.
//
// This does mean the model requires an x87 host to be exact. Built anywhere
// else (`__LDBL_MANT_DIG__ != 64`, or with `OPENSMACX_X87_NO_HOST_SYNC`) the
// arithmetic falls back to whatever the host's widest float does in
// round-to-nearest, and the 80-bit memory format is encoded and decoded by
// hand instead of memcpy'd. That build is for reading and for unit tests, not
// for fidelity; the differential measures the real one.
//
// ---------------------------------------------------------------------------
// WHAT IS MODELLED, AND WHAT IS NOT
// ---------------------------------------------------------------------------
//
// Modelled: the stack and its top-of-stack index, the tag word, the condition
// codes C0/C2/C3 (C1 only where the hardware defines it as zero), the control
// word's precision and rounding fields, and the whole compare -> `fnstsw` ->
// `test ah` path.
//
// Not modelled: the exception status bits (IE/DE/ZE/OE/UE/PE, bits 0..5), their
// summary ES and busy B, and unmasked-exception traps. That is a deliberate
// gap with evidence behind it: all 151 `fnstsw` sites in this image are
// followed by `test ah, imm8` - the masks used are 0x41, 0x40 and 0x01 - and
// there are ZERO `sahf` sites. Bits 0..7 of the status word are therefore
// never read by this program, and C1 (bit 9, mask 0x02 in AH) is never tested
// either. Modelling them would be unfalsifiable decoration.
//
// Nor is the stack FAULT: pushing onto a full stack or reading an empty slot
// sets SF and C1 and substitutes the indefinite QNaN on the hardware, and here
// it just reads or writes the slot. That is the right trade because a lifted
// body that overflows the stack has a lowering bug, not a floating-point
// question - and `opensmacx_x87_depth()` is a better way to find it than a
// QNaN propagating out of one.
//
// Also not modelled: FSAVE/FRSTOR/FLDENV/FNSTENV (whole-state moves), FBLD and
// FBSTP (packed BCD), and the FUCOMI/FCOMI family that writes EFLAGS directly.
// None occur in this image. They are absent rather than wrong: an instruction
// with no rule still becomes a trap that names itself.

#include <cmath>
#include <cstdint>
#include <cstring>

#include "lifted_runtime.h"

// True when `long double` is the 80-bit x87 type and inline x87 asm is
// available - i.e. when this model can be exact rather than approximate.
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) \
    && (__LDBL_MANT_DIG__ == 64) && !defined(OPENSMACX_X87_NO_HOST_SYNC)
#define OPENSMACX_X87_NATIVE 1
#else
#define OPENSMACX_X87_NATIVE 0
#endif

// ---------------------------------------------------------------------------
// The register stack
// ---------------------------------------------------------------------------

struct OpensmacxX87State {
    // Indexed by PHYSICAL register number, the way the tag word is. ST(i) is
    // reg[(top + i) & 7], and that indirection is the entire point: keeping
    // the array in ST order instead would make a push a memmove of eight
    // registers and - much worse - would silently renumber every live value.
    long double reg[8];
    // 1 = empty. The hardware tracks this to fault on reading an empty slot;
    // here it is worth keeping for a different reason. A lifted function whose
    // pushes and pops do not balance leaves the stack drifting, and the drift
    // is invisible in the values - it corrupts a later function's ST numbering
    // instead. `opensmacx_x87_depth()` makes that observable.
    uint8_t empty[8];
    uint32_t top;
    uint16_t control;
    // C0/C1/C2/C3 and the exception bits. TOP is NOT stored here - it is
    // composed in on read, so there is exactly one place holding it.
    uint16_t status;
};

// The x87 state after RESET or FINIT: all exceptions masked, PC = 64-bit,
// RC = nearest, stack empty. `fninit` restores exactly this.
constexpr uint16_t OpensmacxX87ResetControl = 0x037FU;

// What the emulated FPU holds before any guest instruction runs. This is NOT
// the reset value: MSVC's CRT hands a Windows process 0x027F (PC = 53) during
// startup, before any of the code being lifted here executes, and the imports
// that would have done that are shims in this build. Starting from the reset
// value instead would run every x87 function at PC = 64 up to the image's
// first `fninit`, which is a divergence the model would have introduced
// itself.
constexpr uint16_t OpensmacxX87DefaultControl = 0x027FU;

inline OpensmacxX87State opensmacx_x87 = {
    {0, 0, 0, 0, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1, 1}, 0,
    OpensmacxX87DefaultControl, 0};

// Status-word bit positions. C3 is at 14, not 11, and the three bits between
// them are TOP: a status word assembled with C3 adjacent to C2 would put the
// compare result inside the stack pointer, and `test ah, 0x41` would then read
// a bit of TOP as "equal".
constexpr uint16_t OpensmacxX87C0 = 1U << 8;
constexpr uint16_t OpensmacxX87C1 = 1U << 9;
constexpr uint16_t OpensmacxX87C2 = 1U << 10;
constexpr uint16_t OpensmacxX87C3 = 1U << 14;
constexpr uint16_t OpensmacxX87TopField = 7U << 11;
constexpr uint16_t OpensmacxX87ConditionCodes =
    OpensmacxX87C0 | OpensmacxX87C1 | OpensmacxX87C2 | OpensmacxX87C3;

// ---------------------------------------------------------------------------
// The host FPU
//
// The emulated precision (bits 8..9) and rounding (bits 10..11) fields go
// straight through; everything else is pinned. Bits 0..5 stay set so every
// exception remains masked on the host regardless of what the guest asked for
// - an unmasked host exception would fault the process instead of setting a
// status bit this model does not carry anyway.
//
// PC = 64 is mirrored like any other value now that a slot is 80 bits wide.
// The previous model pinned the host to 53 there and accepted the difference;
// that is the divergence this header exists to remove.
// ---------------------------------------------------------------------------

inline void opensmacx_x87_sync_host(uint16_t control) {
#if OPENSMACX_X87_NATIVE
    const uint16_t host = static_cast<uint16_t>(0x007FU | (control & 0x0F00U));
    __asm__ volatile("fldcw %0" : : "m"(host));
#else
    (void)control;
#endif
}

// The host control word is process state, and the C runtime sets it during
// startup without asking this model. Sync it once, before `main`, so the very
// first lifted instruction rounds the way the emulated control word says
// rather than the way the CRT happened to leave the part.
// `inline` rather than an anonymous namespace: this header is included by all
// 64 lifted shards plus the runtime, and an internal-linkage object would give
// each of them its own copy and its own startup call. One object, one fldcw.
struct OpensmacxX87HostInit {
    OpensmacxX87HostInit() { opensmacx_x87_sync_host(opensmacx_x87.control); }
};
inline const OpensmacxX87HostInit opensmacx_x87_host_init;

// True when this build reproduces the hardware bit-for-bit. It is no longer a
// statement about which precision the guest selected - all four are exact now
// - but about whether the host is an x87 at all.
inline bool opensmacx_x87_precision_is_exact() {
    return OPENSMACX_X87_NATIVE != 0;
}

inline unsigned opensmacx_x87_rounding_mode() {
    return (opensmacx_x87.control >> 10) & 3U;
}

// Round to an integral value in the emulated rounding mode - the whole of
// FRNDINT, and the first half of FIST/FISTP. This image's `fldcw` sites exist
// precisely to set round-toward-zero around integer conversions, so a model
// that always rounded to nearest would be off by one on every float-to-int in
// the rasterisers.
//
// On the real part this is the instruction itself, under the mirrored control
// word, which also settles the question of whether FRNDINT re-rounds to the
// precision control (it does not) without this header having to take a
// position on it.
inline long double opensmacx_x87_round_integral(long double value) {
#if OPENSMACX_X87_NATIVE
    long double result;
    __asm__("frndint" : "=t"(result) : "0"(value));
    return result;
#else
    switch (opensmacx_x87_rounding_mode()) {
        case 0: return std::nearbyint(value);   // nearest, ties to even
        case 1: return std::floor(value);       // toward -inf
        case 2: return std::ceil(value);        // toward +inf
        default: return std::trunc(value);      // toward zero
    }
#endif
}

// ---------------------------------------------------------------------------
// Stack primitives
//
// **Push DECREMENTS top; pop INCREMENTS it.** Getting this backwards is the
// single most destructive mistake available in this header, and it is invisible
// in any program that holds one value at a time: `fld x; fstp y` works either
// way. It breaks the moment two values are live, and then it breaks silently,
// because ST(1) still names *a* register - the wrong one. The direction is
// verified against the hardware's own TOP field, which `fnstsw` exposes.
// ---------------------------------------------------------------------------

inline void opensmacx_x87_push(long double value) {
    opensmacx_x87.top = (opensmacx_x87.top - 1U) & 7U;
    opensmacx_x87.reg[opensmacx_x87.top] = value;
    opensmacx_x87.empty[opensmacx_x87.top] = 0;
}

inline void opensmacx_x87_pop() {
    opensmacx_x87.empty[opensmacx_x87.top] = 1;
    opensmacx_x87.top = (opensmacx_x87.top + 1U) & 7U;
}

inline unsigned opensmacx_x87_physical(unsigned index) {
    return (opensmacx_x87.top + index) & 7U;
}

inline long double opensmacx_x87_get(unsigned index) {
    return opensmacx_x87.reg[opensmacx_x87_physical(index)];
}

inline void opensmacx_x87_set(unsigned index, long double value) {
    const unsigned physical = opensmacx_x87_physical(index);
    opensmacx_x87.reg[physical] = value;
    opensmacx_x87.empty[physical] = 0;
}

// How many slots are occupied. Not an x87 instruction - a diagnostic. A lifted
// function should leave this where it found it.
inline unsigned opensmacx_x87_depth() {
    unsigned live = 0;
    for (unsigned physical = 0; physical < 8U; ++physical) {
        live += opensmacx_x87.empty[physical] ? 0U : 1U;
    }
    return live;
}

// The hardware tag for one PHYSICAL register: 3 empty, 1 zero, 2 special,
// 0 valid. "Special" is NaN, infinity and denormal - and now that the slot is
// a real 80-bit register, denormal means an 80-bit denormal. A denormal
// *double* loaded into the register is an ordinary normal extended number and
// tags 0, which is what the hardware says and what a `double`-backed model
// could not have distinguished.
inline unsigned opensmacx_x87_tag(unsigned physical) {
    if (opensmacx_x87.empty[physical & 7U]) {
        return 3U;
    }
    const long double value = opensmacx_x87.reg[physical & 7U];
    switch (std::fpclassify(value)) {
        case FP_ZERO: return 1U;
        case FP_NAN:
        case FP_INFINITE:
        case FP_SUBNORMAL: return 2U;
        default: return 0U;
    }
}

inline uint16_t opensmacx_x87_tag_word() {
    uint16_t word = 0;
    for (unsigned physical = 0; physical < 8U; ++physical) {
        word = static_cast<uint16_t>(
            word | (opensmacx_x87_tag(physical) << (2U * physical)));
    }
    return word;
}

// ---------------------------------------------------------------------------
// Control and status
// ---------------------------------------------------------------------------

inline void opensmacx_x87_fninit() {
    opensmacx_x87.control = OpensmacxX87ResetControl;
    opensmacx_x87.status = 0;
    opensmacx_x87.top = 0;
    for (unsigned physical = 0; physical < 8U; ++physical) {
        opensmacx_x87.reg[physical] = 0.0L;
        opensmacx_x87.empty[physical] = 1;
    }
    opensmacx_x87_sync_host(opensmacx_x87.control);
}

inline void opensmacx_x87_fnclex() {
    // Clears the exception flags and the busy bit; leaves the condition codes.
    opensmacx_x87.status = static_cast<uint16_t>(opensmacx_x87.status & 0x7F00U);
}

// The status word as `fnstsw` writes it: the stored bits with the live TOP
// composed in.
inline uint32_t opensmacx_x87_status_word() {
    return static_cast<uint32_t>(
        (opensmacx_x87.status & static_cast<uint16_t>(~OpensmacxX87TopField))
        | static_cast<uint16_t>(opensmacx_x87.top << 11));
}

inline void opensmacx_x87_fnstsw_mem(uint32_t address) {
    const uint16_t word = static_cast<uint16_t>(opensmacx_x87_status_word());
    __builtin_memcpy(opensmacx_at(address), &word, sizeof(word));
}

// The control word has bits the part does not keep. Loading 0xFFFF and reading
// it back gives 0x1F7F on the reference FPU: bits 13..15 and bit 7 drop, and
// bit 6 comes back set. That matters here for one reason - this image's idiom
// is `fnstcw; and; or; fldcw`, so the bits the model hands back are the bits
// that go round again - and it is free to be right about.
constexpr uint16_t OpensmacxX87ControlKeep = 0x1F3FU;
constexpr uint16_t OpensmacxX87ControlForce = 0x0040U;

inline void opensmacx_x87_fldcw(uint32_t address) {
    uint16_t word;
    __builtin_memcpy(&word, opensmacx_at(address), sizeof(word));
    word = static_cast<uint16_t>((word & OpensmacxX87ControlKeep)
                                 | OpensmacxX87ControlForce);
    opensmacx_x87.control = word;
    opensmacx_x87_sync_host(word);
}

inline void opensmacx_x87_fnstcw(uint32_t address) {
    const uint16_t word = opensmacx_x87.control;
    __builtin_memcpy(opensmacx_at(address), &word, sizeof(word));
}

// ---------------------------------------------------------------------------
// Memory operands
//
// A load widens and is exact - every float, double and int64 fits an 80-bit
// register with room over, which is why FILD is not affected by precision
// control and why this model must not narrow on the way in.
//
// A store narrows and rounds, and the cast IS the store instruction: on the
// i686 target `static_cast<float>(long double)` is `fstps` and
// `static_cast<double>` is `fstpl`, each rounding in the emulated direction
// because the host control word carries it. That covers the directed modes and
// the overflow-to-infinity edge without a line of arithmetic here.
// ---------------------------------------------------------------------------

inline long double opensmacx_x87_mem32(uint32_t address) {
    float value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return static_cast<long double>(value);
}

inline long double opensmacx_x87_mem64(uint32_t address) {
    double value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return static_cast<long double>(value);
}

// 80-bit extended. On the real target this is the register's own format, so
// the load is ten bytes copied - no decode, no rounding, and no way to lose
// the integer bit or a signalling NaN's class. The hand decode is the fallback
// for hosts whose `long double` is something else.
inline long double opensmacx_x87_mem80(uint32_t address) {
#if OPENSMACX_X87_NATIVE
    long double value = 0.0L;
    __builtin_memcpy(&value, opensmacx_at(address), 10);
    return value;
#else
    unsigned char raw[10];
    __builtin_memcpy(raw, opensmacx_at(address), sizeof(raw));
    uint64_t significand;
    uint16_t sign_exponent;
    __builtin_memcpy(&significand, raw, sizeof(significand));
    __builtin_memcpy(&sign_exponent, raw + 8, sizeof(sign_exponent));
    const bool negative = (sign_exponent & 0x8000U) != 0;
    const unsigned exponent = sign_exponent & 0x7FFFU;
    long double value;
    if (exponent == 0x7FFFU) {
        // The integer bit is explicit here, so infinity is significand
        // 0x8000000000000000 exactly and everything else is a NaN.
        value = (significand << 1) == 0 ? HUGE_VALL : std::nanl("");
    } else {
        value = std::ldexp(static_cast<long double>(significand),
                           static_cast<int>(exponent) - 16383 - 63
                               + (exponent == 0 ? 1 : 0));
    }
    return negative ? -value : value;
#endif
}

inline void opensmacx_x87_store32(uint32_t address, long double value) {
    const float narrow = static_cast<float>(value);
    __builtin_memcpy(opensmacx_at(address), &narrow, sizeof(narrow));
}

inline void opensmacx_x87_store64(uint32_t address, long double value) {
    const double narrow = static_cast<double>(value);
    __builtin_memcpy(opensmacx_at(address), &narrow, sizeof(narrow));
}

// Ten bytes out of the register, for the same reason as the load. `long
// double` is twelve bytes here - four on a 16-byte-aligned ABI - and only the
// first ten are the number, so the copy length is written out rather than
// taken from `sizeof`.
inline void opensmacx_x87_store80(uint32_t address, long double value) {
#if OPENSMACX_X87_NATIVE
    __builtin_memcpy(opensmacx_at(address), &value, 10);
#else
    // Encode from the widest thing this host has. Exact whenever that is at
    // least as wide as extended, which is the only case that matters: on a
    // host with a narrower type there is nothing to encode from.
    uint64_t significand = 0;
    uint16_t sign_exponent = 0;
    const bool negative = std::signbit(value) != 0;
    const long double magnitude = std::fabs(value);
    if (std::isnan(magnitude)) {
        sign_exponent = 0x7FFFU;
        significand = 0xC000000000000000ULL;
    } else if (std::isinf(magnitude)) {
        sign_exponent = 0x7FFFU;
        significand = 0x8000000000000000ULL;
    } else if (magnitude != 0.0L) {
        int exponent = 0;
        const long double fraction = std::frexp(magnitude, &exponent);
        // frexp gives [0.5, 1); scaling by 2^64 makes the integer bit the top
        // bit of the significand, which is where extended keeps it.
        significand = static_cast<uint64_t>(std::ldexp(fraction, 64));
        sign_exponent = static_cast<uint16_t>(exponent - 1 + 16383);
    }
    if (negative) {
        sign_exponent = static_cast<uint16_t>(sign_exponent | 0x8000U);
    }
    unsigned char raw[10];
    __builtin_memcpy(raw, &significand, sizeof(significand));
    __builtin_memcpy(raw + 8, &sign_exponent, sizeof(sign_exponent));
    __builtin_memcpy(opensmacx_at(address), raw, sizeof(raw));
#endif
}

inline long double opensmacx_x87_imem16(uint32_t address) {
    int16_t value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return static_cast<long double>(value);
}

inline long double opensmacx_x87_imem32(uint32_t address) {
    int32_t value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return static_cast<long double>(value);
}

// Every int64 has at most 63 significand bits and the register holds 64, so
// this is exact - at every precision control, because a load does not consult
// one. Through a 53-bit slot it was not, and that was the sharpest of the
// defects this rewrite fixes.
inline long double opensmacx_x87_imem64(uint32_t address) {
    int64_t value;
    __builtin_memcpy(&value, opensmacx_at(address), sizeof(value));
    return static_cast<long double>(value);
}

// ---------------------------------------------------------------------------
// Loads and stores
// ---------------------------------------------------------------------------

inline void opensmacx_x87_fld32(uint32_t address) {
    opensmacx_x87_push(opensmacx_x87_mem32(address));
}
inline void opensmacx_x87_fld64(uint32_t address) {
    opensmacx_x87_push(opensmacx_x87_mem64(address));
}
inline void opensmacx_x87_fld80(uint32_t address) {
    opensmacx_x87_push(opensmacx_x87_mem80(address));
}

inline void opensmacx_x87_fild16(uint32_t address) {
    opensmacx_x87_push(opensmacx_x87_imem16(address));
}
inline void opensmacx_x87_fild32(uint32_t address) {
    opensmacx_x87_push(opensmacx_x87_imem32(address));
}
inline void opensmacx_x87_fild64(uint32_t address) {
    opensmacx_x87_push(opensmacx_x87_imem64(address));
}

inline void opensmacx_x87_fst32(uint32_t address, bool pop) {
    opensmacx_x87_store32(address, opensmacx_x87_get(0));
    if (pop) { opensmacx_x87_pop(); }
}
inline void opensmacx_x87_fst64(uint32_t address, bool pop) {
    opensmacx_x87_store64(address, opensmacx_x87_get(0));
    if (pop) { opensmacx_x87_pop(); }
}
inline void opensmacx_x87_fst80(uint32_t address, bool pop) {
    opensmacx_x87_store80(address, opensmacx_x87_get(0));
    if (pop) { opensmacx_x87_pop(); }
}

// The out-of-range answer is the "integer indefinite": the most negative value
// of the destination width, NOT a saturation and NOT the low bits of the
// truncation. Any NaN or infinity lands there too. Getting this wrong turns a
// clamped coordinate into a plausible-looking wrong one.
//
// The range test is on the ROUNDED value and the bounds are exact in the
// register, so `2147483647.5` under round-to-nearest rounds to 2147483648 and
// is then out of range - which is what the hardware does and what a test on
// the unrounded value would get wrong in the other direction.
inline void opensmacx_x87_fist16(uint32_t address, bool pop) {
    const long double rounded = opensmacx_x87_round_integral(opensmacx_x87_get(0));
    int16_t narrow = static_cast<int16_t>(0x8000);
    if (rounded >= -32768.0L && rounded <= 32767.0L) {
        narrow = static_cast<int16_t>(rounded);
    }
    __builtin_memcpy(opensmacx_at(address), &narrow, sizeof(narrow));
    if (pop) { opensmacx_x87_pop(); }
}

inline void opensmacx_x87_fist32(uint32_t address, bool pop) {
    const long double rounded = opensmacx_x87_round_integral(opensmacx_x87_get(0));
    int32_t narrow = static_cast<int32_t>(0x80000000U);
    if (rounded >= -2147483648.0L && rounded <= 2147483647.0L) {
        narrow = static_cast<int32_t>(rounded);
    }
    __builtin_memcpy(opensmacx_at(address), &narrow, sizeof(narrow));
    if (pop) { opensmacx_x87_pop(); }
}

inline void opensmacx_x87_fist64(uint32_t address, bool pop) {
    const long double rounded = opensmacx_x87_round_integral(opensmacx_x87_get(0));
    int64_t narrow = static_cast<int64_t>(0x8000000000000000ULL);
    // -2^63 is in range and +2^63 is not, and BOTH are exactly representable
    // in the register, so the two bounds are not symmetric and the upper test
    // has to be strict. Through a 53-bit slot the distinction was unobservable
    // because neither bound's neighbours existed.
    if (rounded >= -9223372036854775808.0L
        && rounded < 9223372036854775808.0L) {
        narrow = static_cast<int64_t>(rounded);
    }
    __builtin_memcpy(opensmacx_at(address), &narrow, sizeof(narrow));
    if (pop) { opensmacx_x87_pop(); }
}

// FLD ST(i) reads BEFORE it pushes. After the push the source has moved to
// ST(i+1), so reading afterwards would copy the wrong register for every i,
// and would copy the just-pushed garbage for i = 0.
inline void opensmacx_x87_fld_st(unsigned index) {
    opensmacx_x87_push(opensmacx_x87_get(index));
}

// FST/FSTP ST(i) copies ST(0) into ST(i) and only then pops, so `fstp st(1)`
// - the image's idiom for dropping the second value - leaves the top where it
// was rather than duplicating it.
inline void opensmacx_x87_fst_st(unsigned index, bool pop) {
    opensmacx_x87_set(index, opensmacx_x87_get(0));
    if (pop) { opensmacx_x87_pop(); }
}

inline void opensmacx_x87_fxch(unsigned index) {
    const long double top = opensmacx_x87_get(0);
    opensmacx_x87_set(0, opensmacx_x87_get(index));
    opensmacx_x87_set(index, top);
}

inline void opensmacx_x87_ffree(unsigned index, bool pop) {
    opensmacx_x87.empty[opensmacx_x87_physical(index)] = 1;
    if (pop) { opensmacx_x87_pop(); }
}

// ---------------------------------------------------------------------------
// Arithmetic
//
// **The reverse forms are not a naming convention, they are a different
// operation.** `fsub` computes destination - source and `fsubr` computes
// source - destination, and for a non-commutative operation that is the
// difference between a value and its negation. The whole direction question
// lives in this one function so that it has exactly one place to be wrong, and
// so that a mutation of it is caught by every test that uses subtraction.
//
// The encoding makes this worse than it looks: on the hardware, opcode DC E0+i
// is FSUBR and DC E8+i is FSUB, the OPPOSITE of the D8 pair. Capstone resolves
// that, so the mnemonic reaching the lowering is already the true operation -
// but it means a hand-check against a raw opcode table will disagree with the
// disassembly unless the reader knows about the swap.
//
// There is no rounding step after the operator. Each of these is one x87
// instruction on 80-bit operands under the emulated control word, so the
// hardware's own single rounding to the selected precision is the only one
// that happens. The previous model rounded a second time in software and
// documented the double rounding as a limit; removing it removed the limit.
// ---------------------------------------------------------------------------

enum OpensmacxX87Op {
    OpensmacxX87Add,
    OpensmacxX87Mul,
    OpensmacxX87Sub,
    OpensmacxX87Subr,
    OpensmacxX87Div,
    OpensmacxX87Divr,
};

inline long double opensmacx_x87_apply(OpensmacxX87Op op, long double left,
                                       long double right) {
    switch (op) {
        case OpensmacxX87Add: return left + right;
        case OpensmacxX87Mul: return left * right;
        case OpensmacxX87Sub: return left - right;
        case OpensmacxX87Subr: return right - left;
        case OpensmacxX87Div: return left / right;
        default: return right / left;
    }
}

// ST(0) <- ST(0) op value. The memory forms and the one-register form.
inline void opensmacx_x87_binary_st0(OpensmacxX87Op op, long double value) {
    opensmacx_x87_set(0, opensmacx_x87_apply(op, opensmacx_x87_get(0), value));
}

// ST(i) <- ST(i) op ST(0), then optionally pop. The two-register form and the
// whole `p` family.
inline void opensmacx_x87_binary_sti(OpensmacxX87Op op, unsigned index,
                                     bool pop) {
    opensmacx_x87_set(
        index, opensmacx_x87_apply(op, opensmacx_x87_get(index),
                                   opensmacx_x87_get(0)));
    if (pop) { opensmacx_x87_pop(); }
}

inline void opensmacx_x87_chs() {
    opensmacx_x87_set(0, -opensmacx_x87_get(0));
    opensmacx_x87.status &= static_cast<uint16_t>(~OpensmacxX87C1);
}

inline void opensmacx_x87_abs() {
    opensmacx_x87_set(0, std::fabs(opensmacx_x87_get(0)));
    opensmacx_x87.status &= static_cast<uint16_t>(~OpensmacxX87C1);
}

// The transcendental and root instructions are taken with the instruction
// being modelled rather than through <cmath>, for two reasons that the earlier
// differential found the hard way. The library functions do not obey the
// emulated control word - `std::sqrt` on this toolchain lands in msvcrt, which
// returned the round-to-nearest answer under round-toward-negative - and they
// are not obliged to produce the part's own transcendental approximation,
// which is a specific and slightly wrong function that this image's results
// depend on.
//
// The fallbacks are for a host without an x87, where nothing here is exact
// anyway.
inline long double opensmacx_x87_raw_sqrt(long double value) {
#if OPENSMACX_X87_NATIVE
    long double result;
    __asm__("fsqrt" : "=t"(result) : "0"(value));
    return result;
#else
    return std::sqrt(value);
#endif
}

inline void opensmacx_x87_sqrt() {
    opensmacx_x87_set(0, opensmacx_x87_raw_sqrt(opensmacx_x87_get(0)));
}

inline void opensmacx_x87_rndint() {
    opensmacx_x87_set(0, opensmacx_x87_round_integral(opensmacx_x87_get(0)));
}

// FSIN/FCOS have THREE input classes, and the differential caught this model
// conflating two of them. A finite operand of magnitude >= 2^63 is "out of
// range": C2 is set and ST(0) is left ALONE, because the hardware will not
// attempt the argument reduction. An infinity or a NaN is not out of range -
// it is an invalid operation, C2 stays CLEAR and the result is a QNaN. Code
// that only tests C2 therefore cannot tell an infinity from an ordinary
// result, exactly as on the machine.
inline bool opensmacx_x87_trig_out_of_range(long double value) {
    return std::isfinite(value)
           && std::fabs(value) >= 9223372036854775808.0L;
}

inline void opensmacx_x87_sin() {
    const long double value = opensmacx_x87_get(0);
    if (opensmacx_x87_trig_out_of_range(value)) {
        opensmacx_x87.status |= OpensmacxX87C2;
        return;
    }
    opensmacx_x87.status &= static_cast<uint16_t>(~OpensmacxX87C2);
#if OPENSMACX_X87_NATIVE
    long double result;
    __asm__("fsin" : "=t"(result) : "0"(value));
#else
    const long double result =
        std::isfinite(value) ? std::sin(value) : std::nanl("");
#endif
    opensmacx_x87_set(0, result);
}

inline void opensmacx_x87_cos() {
    const long double value = opensmacx_x87_get(0);
    if (opensmacx_x87_trig_out_of_range(value)) {
        opensmacx_x87.status |= OpensmacxX87C2;
        return;
    }
    opensmacx_x87.status &= static_cast<uint16_t>(~OpensmacxX87C2);
#if OPENSMACX_X87_NATIVE
    long double result;
    __asm__("fcos" : "=t"(result) : "0"(value));
#else
    const long double result =
        std::isfinite(value) ? std::cos(value) : std::nanl("");
#endif
    opensmacx_x87_set(0, result);
}

// FPATAN is atan2(ST(1), ST(0)) - the STACK order, which is the reverse of the
// argument order the name suggests - and it pops, leaving the result at the
// new ST(0).
inline void opensmacx_x87_patan() {
    const long double y = opensmacx_x87_get(1);
    const long double x = opensmacx_x87_get(0);
#if OPENSMACX_X87_NATIVE
    long double result;
    __asm__("fpatan" : "=t"(result) : "0"(x), "u"(y) : "st(1)");
#else
    const long double result = std::atan2(y, x);
#endif
    opensmacx_x87_set(1, result);
    opensmacx_x87_pop();
}

// ---------------------------------------------------------------------------
// Comparison
//
// This is the path that matters most. Almost every floating-point branch in
// this image is `fcom`/`fcomp` -> `fnstsw ax` -> `test ah, 0x41` -> `je`/`jne`,
// so an arithmetic error changes a number while a comparison error changes
// which way the program GOES.
//
// The four outcomes and the codes the hardware sets, verified against the
// machine:
//
//     ST(0) > source     C3 = 0  C2 = 0  C0 = 0
//     ST(0) < source     C3 = 0  C2 = 0  C0 = 1
//     ST(0) = source     C3 = 1  C2 = 0  C0 = 0
//     unordered (NaN)    C3 = 1  C2 = 1  C0 = 1
//
// Read that as a truth table for the masks the image actually uses. `test ah,
// 0x41` is C3|C0 and so is false only for strictly greater. `test ah, 0x01` is
// C0 alone: less-than, or unordered. `test ah, 0x40` is C3: equal, or
// unordered. Swapping C0 and C3 leaves the unordered row unchanged and
// exchanges "less" with "equal" - which keeps `test ah, 0x41` passing and
// breaks the other two, so only a test that exercises all three masks catches
// it.
//
// +0.0 and -0.0 compare EQUAL, which C++'s `==` also says, so the natural
// spelling is the right one here.
// ---------------------------------------------------------------------------

inline void opensmacx_x87_compare(long double left, long double right) {
    uint16_t codes;
    if (std::isnan(left) || std::isnan(right)) {
        codes = OpensmacxX87C3 | OpensmacxX87C2 | OpensmacxX87C0;
    } else if (left > right) {
        codes = 0;
    } else if (left < right) {
        codes = OpensmacxX87C0;
    } else {
        codes = OpensmacxX87C3;
    }
    // C1 is cleared by a compare; the other three are replaced wholesale.
    opensmacx_x87.status = static_cast<uint16_t>(
        (opensmacx_x87.status & ~OpensmacxX87ConditionCodes) | codes);
}

// `pops` is 0 for FCOM, 1 for FCOMP, 2 for FCOMPP. The comparison happens
// first, then the pops - so FCOMPP compares ST(0) against ST(1) and then
// discards both.
inline void opensmacx_x87_fcom(long double value, unsigned pops) {
    opensmacx_x87_compare(opensmacx_x87_get(0), value);
    for (unsigned index = 0; index < pops; ++index) {
        opensmacx_x87_pop();
    }
}

inline void opensmacx_x87_ftst() {
    opensmacx_x87_compare(opensmacx_x87_get(0), 0.0L);
}

// ---------------------------------------------------------------------------
// Constants
//
// The part's constants are 80-bit and are NOT rounded to the precision control
// - verified against hardware at PC = 24 in all four rounding modes - so they
// are pushed with the instruction rather than as literals. A `double` literal
// would have been the PC = 53 value, correct only in one of the four modes the
// image reaches.
// ---------------------------------------------------------------------------

#if OPENSMACX_X87_NATIVE
#define OPENSMACX_X87_CONSTANT(NAME, INSTRUCTION)                            \
    inline void NAME() {                                                     \
        long double value;                                                   \
        __asm__(INSTRUCTION : "=t"(value));                                  \
        opensmacx_x87_push(value);                                           \
    }
#else
#define OPENSMACX_X87_CONSTANT(NAME, INSTRUCTION) /* defined below */
#endif

#if OPENSMACX_X87_NATIVE
OPENSMACX_X87_CONSTANT(opensmacx_x87_fld1, "fld1")
OPENSMACX_X87_CONSTANT(opensmacx_x87_fldz, "fldz")
OPENSMACX_X87_CONSTANT(opensmacx_x87_fldpi, "fldpi")
OPENSMACX_X87_CONSTANT(opensmacx_x87_fldl2e, "fldl2e")
OPENSMACX_X87_CONSTANT(opensmacx_x87_fldl2t, "fldl2t")
OPENSMACX_X87_CONSTANT(opensmacx_x87_fldlg2, "fldlg2")
OPENSMACX_X87_CONSTANT(opensmacx_x87_fldln2, "fldln2")
#else
inline void opensmacx_x87_fld1()   { opensmacx_x87_push(1.0L); }
inline void opensmacx_x87_fldz()   { opensmacx_x87_push(0.0L); }
inline void opensmacx_x87_fldpi()  { opensmacx_x87_push(3.14159265358979323846L); }
inline void opensmacx_x87_fldl2e() { opensmacx_x87_push(1.44269504088896340736L); }
inline void opensmacx_x87_fldl2t() { opensmacx_x87_push(3.32192809488736234787L); }
inline void opensmacx_x87_fldlg2() { opensmacx_x87_push(0.30102999566398119521L); }
inline void opensmacx_x87_fldln2() { opensmacx_x87_push(0.69314718055994530942L); }
#endif

#undef OPENSMACX_X87_CONSTANT

#endif
